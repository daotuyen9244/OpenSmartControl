/**
 * BleService.js
 * Service quản lý kết nối và giao tiếp Bluetooth Low Energy (BLE) với ESP32
 * 
 * Tính năng chính:
 * - Khởi tạo và quản lý BLE Manager
 * - Scan và discovery thiết bị BLE
 * - Kết nối/ngắt kết nối với ESP32
 * - Gửi/nhận dữ liệu qua BLE characteristics
 * - Xử lý JSON fragments từ ESP32
 * - Quản lý quyền truy cập Bluetooth
 * - Tích hợp với Redux store
 */

import { Buffer } from 'buffer';
import { BleManager, State } from 'react-native-ble-plx';
import { PermissionsAndroid, Platform, AppState, Linking, Alert } from 'react-native';
import AsyncStorage from '@react-native-async-storage/async-storage';
import { getCommand, COMMANDS } from '../utils/packetCommands';

// Disable console warnings/errors in production (only show in dev mode)
const ENABLE_DEBUG_LOGS = false; // Disabled for production
const consoleWarn = ENABLE_DEBUG_LOGS ? console.warn.bind(console) : () => {};
const consoleError = ENABLE_DEBUG_LOGS ? console.error.bind(console) : () => {};

/**
 * BLE configuration constants for ESP32 massage device
 * Using Nordic UART Service (NUS) protocol
 */
const BLE_CONFIG = {
  // Main service UUID (HM-10 BLE Service)
  SERVICE_UUID: '0000ffe0-0000-1000-8000-00805f9b34fb',
  
  // UUID to send data to HM-10 (RX from HM-10 side)
  RX_CHAR_UUID: '0000ffe1-0000-1000-8000-00805f9b34fb',
  
  // UUID to receive data from HM-10 (TX from HM-10 side)
  TX_CHAR_UUID: '0000ffe1-0000-1000-8000-00805f9b34fb',
  
  // Timeout times for operations
  SCAN_TIMEOUT: 20000,        // 20 seconds device scan (increased from 10s)
  CONNECTION_TIMEOUT: 15000,  // 15 seconds connection timeout
  FRAGMENT_TIMEOUT: 5000,     // 5 seconds fragment receive timeout
  MAX_BUFFER_SIZE: 2000,      // Maximum buffer size
};

/**
 * Helper function: Serialize device object for Redux store
 * Only keep primitive properties, remove functions and complex objects
 * @param {Object} device - Device object from react-native-ble-plx
 * @returns {Object} - Serialized device object
 */
const serializeDevice = (device) => {
  if (!device) return null;
  
  return {
    id: device.id,
    name: device.name || device.localName || 'Unknown Device',
    rssi: device.rssi,
    serviceUUIDs: device.serviceUUIDs || [],
    manufacturerData: device.manufacturerData,
    isConnectable: device.isConnectable,
    txPowerLevel: device.txPowerLevel,
    solicitedServiceUUIDs: device.solicitedServiceUUIDs || [],
    overflowServiceUUIDs: device.overflowServiceUUIDs || [],
    mtu: device.mtu,
    // Only include serializable properties
  };
};

/**
 * Helper: Kiểm tra xem đang chạy trên iOS Simulator hay không
 * iOS Simulator không hỗ trợ Bluetooth thật
 * @returns {boolean} - True nếu đang chạy trên Simulator
 */
const isRunningOnSimulator = () => {
  if (Platform.OS !== 'ios') return false;
  
  // Check if running on simulator using various methods
  // Method 1: Check if it's a simulator device model
  const DeviceInfo = require('react-native-device-info');
  if (DeviceInfo && DeviceInfo.default && DeviceInfo.default.isEmulatorSync) {
    return DeviceInfo.default.isEmulatorSync();
  }
  
  // Method 2: Fallback - simulator typically has specific characteristics
  // In production builds on real devices, __DEV__ might be false
  return Platform.isTV || false; // Conservative approach
};

/**
 * BleService class - Singleton service để quản lý BLE operations
 */
class BleService {
  constructor() {
    // === CORE BLE COMPONENTS ===
    this.manager = new BleManager();
    this.SERVICE_UUID = BLE_CONFIG.SERVICE_UUID;
    this.RX_CHAR_UUID = BLE_CONFIG.RX_CHAR_UUID;
    this.TX_CHAR_UUID = BLE_CONFIG.TX_CHAR_UUID;
    
    // === STATE MANAGEMENT ===
    this.isInitialized = false;
    this.connectedDevice = null;        // Store actual device object (not stored in Redux)
    this.connectedDeviceId = null;
    this.notificationListeners = [];
    this.lastResponse = null;
    this.bluetoothState = 'Unknown';
    this.isScanning = false;
    this.discoveredDevices = new Map(); // Store actual device objects
    this.paircode = null;                // Store pair code for pairing (BLE encryption)
    
    // === SUBSCRIPTIONS ===
    this.stateSubscription = null;
    this.scanSubscription = null;
    this.notificationSubscription = null;
    this.disconnectionSubscription = null;
    
    // === REDUX INTEGRATION ===
    this.dispatch = null; // Will be set from outside
    
    // === DISCONNECT MANAGEMENT ===
    this.disconnectTimeout = null;
    this.isDisconnecting = false;
    
    // === JSON FRAGMENT HANDLING ===
    this.dataBuffer = '';
    this.isReceivingFragments = false;
    this.fragmentTimeout = null;
    this.debugMode = false; // Enable for debugging
    
    // === APP STATE MANAGEMENT ===
    this.appState = 'active';
    this.appStateSubscription = null;
    
    // === HEARTBEAT MANAGEMENT ===
    this.heartbeatInterval = null;
    this.heartbeatIntervalMs = 5000; // 5 seconds (configurable)
  }

  /**
   * Set Redux dispatch function to integrate with store
   * @param {Function} dispatch - Redux dispatch function
   */
  setDispatch(dispatch) {
    this.dispatch = dispatch;
  }

  setGetState(getState) {
    this.getState = getState;
  }

  /**
   * Khởi tạo BLE Service
   * - Request permissions
   * - Setup state listeners
   * - Initialize BLE manager
   * @returns {Promise<boolean>} - True nếu khởi tạo thành công
   */
  async initialize() {
    try {
      if (this.isInitialized) return true;
      
      console.log('🔧 Initializing BLE Service...');
      console.log(`Platform: ${Platform.OS}`);
      
      const isSimulator = isRunningOnSimulator();
      console.log(`Running on Simulator: ${isSimulator}`);
      
      if (isSimulator) {
        consoleWarn('⚠️ iOS Simulator detected - Bluetooth features may not work. Please use a real device for Bluetooth functionality.');
      }
      
      // Yêu cầu quyền truy cập trước
      await this.requestPermissions();
      
      // Setup listener cho Bluetooth state changes
      this.setupBluetoothStateListener();
      
      // Setup app state change listener
      this.setupAppStateListener();
      
      // Đợi một chút để BLE adapter khởi động
      console.log('⏳ Waiting for BLE adapter to initialize...');
      await new Promise(resolve => setTimeout(resolve, 2000)); // Wait 2s for adapter
      
      // Log initial Bluetooth state
      const initialState = await this.getBluetoothState();
      console.log(`📡 Initial Bluetooth state: ${initialState}`);
      
      // Nếu state vẫn Unknown sau khi init, thử trigger lại
      if (initialState === 'Unknown') {
        console.log('⚠️ Bluetooth state is Unknown, attempting to trigger permission...');
        // Thử scan ngắn để trigger permission dialog
        try {
          await this.manager.startDeviceScan(null, null, () => {});
          await new Promise(resolve => setTimeout(resolve, 500));
          await this.manager.stopDeviceScan();
          
          // Check lại state
          const newState = await this.getBluetoothState();
          console.log(`📡 Bluetooth state after trigger: ${newState}`);
        } catch (error) {
          console.log('Trigger scan error (expected):', error.message);
        }
      }
      
      this.isInitialized = true;
      console.log('✅ BLE Service initialized successfully');
      return true;
    } catch (error) {
      consoleError('❌ BLE initialization failed:', error);
      return false;
    }
  }

  /**
   * Yêu cầu quyền truy cập Bluetooth và Location
   * Android cần location permission để scan BLE devices
   * @returns {Promise<boolean>} - True nếu tất cả quyền được cấp
   */
  async requestPermissions() {
    if (Platform.OS === 'android') {
      const permissions = [
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        PermissionsAndroid.PERMISSIONS.ACCESS_COARSE_LOCATION,
      ];
      
      // Android 12+ cần quyền mới
      if (Platform.Version >= 31) {
        permissions.push(
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
          PermissionsAndroid.PERMISSIONS.BLUETOOTH_ADVERTISE
        );
      }
      
      const granted = await PermissionsAndroid.requestMultiple(permissions);
      const allGranted = Object.values(granted).every(
        permission => permission === PermissionsAndroid.RESULTS.GRANTED
      );
      
      if (!allGranted) {
        consoleWarn('Some BLE permissions not granted');
      }
      
      return allGranted;
    }
    
    return true; // iOS tự động xử lý permissions
  }

  /**
   * Setup listener cho Bluetooth state changes
   * Theo dõi khi Bluetooth được bật/tắt
   */
  setupBluetoothStateListener() {
    this.stateSubscription = this.manager.onStateChange((state) => {
      console.log(`📡 Bluetooth state changed: ${this.bluetoothState} → ${state}`);
      this.bluetoothState = state;
      
      if (state === 'PoweredOff' || state === 'Unauthorized') {
        // Handle Bluetooth disabled
        consoleWarn(`⚠️ Bluetooth state: ${state}`);
        if (this.dispatch) {
          this.dispatch({
            type: 'ble/setConnectionError',
            payload: 'Bluetooth is disabled. Please enable Bluetooth to continue.'
          });
        }
        
        // Clear connected device
        this.connectedDevice = null;
        this.connectedDeviceId = null;
      } else if (state === 'PoweredOn') {
        // Bluetooth enabled, try to reconnect if needed
        console.log('✅ Bluetooth is now PoweredOn');
        this.handleBluetoothEnabled();
      } else if (state === 'Unknown' || state === 'Resetting') {
        consoleWarn(`⏳ Bluetooth is in transition state: ${state}`);
      }
      
      this.notifyStateChange(state);
    }, true); // emitCurrentState = true
  }

  /**
   * Setup listener cho app state changes
   * Theo dõi khi app chuyển background/foreground
   */
  setupAppStateListener() {
    this.appStateSubscription = AppState.addEventListener('change', this.handleAppStateChange.bind(this));
  }

  /**
   * Handle app state changes
   * @param {string} nextAppState - Next app state
   */
  async handleAppStateChange(nextAppState) {
    if (this.appState.match(/inactive|background/) && nextAppState === 'active') {
      console.log('App has come to the foreground!');
      
      // Do not auto-reconnect when app comes to foreground
      // User must manually connect to device
      console.log('App foreground - no auto-reconnect');
    }
    this.appState = nextAppState;
  }

  /**
   * Handle Bluetooth enabled event
   */
  async handleBluetoothEnabled() {
    // Do not auto-connect on Bluetooth enabled
    // User must manually connect to device
    console.log('Bluetooth enabled - no auto-connect');
  }

  /**
   * Mở Settings của app để người dùng có thể cấp quyền BLE và Camera
   * @param {string} permissionType - Loại quyền: 'bluetooth', 'camera', hoặc 'all'
   * @returns {Promise<boolean>} - True nếu mở Settings thành công
   */
  async openSettings(permissionType = 'all') {
    try {
      let message = '';
      
      switch (permissionType) {
        case 'bluetooth':
          message = 'Cần cấp quyền Bluetooth để kết nối với thiết bị massage.';
          break;
        case 'camera':
          message = 'Cần cấp quyền Camera để quét QR code kết nối thiết bị.';
          break;
        default:
          message = 'Cần cấp quyền Bluetooth và Camera để sử dụng đầy đủ tính năng của app.';
      }

      if (Platform.OS === 'ios') {
        // iOS: Mở trực tiếp Settings của app
        const url = 'app-settings:';
        const canOpen = await Linking.canOpenURL(url);
        
        if (canOpen) {
          await Linking.openURL(url);
          console.log('✅ Opened iOS Settings');
          return true;
        } else {
          // Fallback: Mở Settings chung
          await Linking.openSettings();
          console.log('✅ Opened iOS Settings (fallback)');
          return true;
        }
      } else {
        // Android: Mở Settings của app
        await Linking.openSettings();
        console.log('✅ Opened Android Settings');
        return true;
      }
    } catch (error) {
      consoleError('❌ Failed to open Settings:', error);
      
      // Hiển thị hướng dẫn thủ công
      Alert.alert(
        'Không thể mở Settings',
        'Vui lòng mở Settings thủ công:\n\niOS: Settings > Massage Chair Control\nAndroid: Settings > Apps > Massage Chair Control > Permissions',
        [{ text: 'OK' }]
      );
      
      return false;
    }
  }

  /**
   * Hiển thị Alert yêu cầu mở Settings để cấp quyền
   * @param {string} permissionType - 'bluetooth', 'camera', hoặc 'all'
   */
  showPermissionSettingsAlert(permissionType = 'all') {
    let title = '';
    let message = '';
    
    switch (permissionType) {
      case 'bluetooth':
        title = 'Quyền Bluetooth cần thiết';
        message = 'Ứng dụng cần quyền Bluetooth để kết nối với thiết bị massage.\n\nVui lòng mở Settings và bật quyền Bluetooth cho app.';
        break;
      case 'camera':
        title = 'Quyền Camera cần thiết';
        message = 'Ứng dụng cần quyền Camera để quét QR code.\n\nVui lòng mở Settings và bật quyền Camera cho app.';
        break;
      default:
        title = 'Quyền truy cập cần thiết';
        message = 'Ứng dụng cần quyền Bluetooth và Camera để hoạt động đầy đủ.\n\nVui lòng mở Settings và cấp các quyền cần thiết.';
    }

    Alert.alert(
      title,
      message,
      [
        { 
          text: 'Hủy', 
          style: 'cancel' 
        },
        {
          text: 'Mở Settings',
          onPress: () => {
            this.openSettings(permissionType);
          }
        }
      ]
    );
  }

  /**
   * Notify Redux store về state changes
   * @param {string} state - Bluetooth state
   */
  notifyStateChange(state) {
    // Có thể dispatch state change action nếu cần
    if (this.dispatch) {
      // this.dispatch({ type: 'ble/setBluetoothState', payload: state });
    }
  }

  /**
   * Lấy trạng thái Bluetooth hiện tại
   * @returns {Promise<string>} - Bluetooth state
   */
  async getBluetoothState() {
    try {
      const state = await this.manager.state();
      
      switch (state) {
        case State.PoweredOn:
          return 'PoweredOn';
        case State.PoweredOff:
          return 'PoweredOff';
        case State.Unauthorized:
          return 'Unauthorized';
        case State.Unsupported:
          return 'Unsupported';
        case State.Unknown:
        case State.Resetting:
        default:
          return 'Unknown';
      }
    } catch (error) {
      consoleError('Error getting bluetooth state:', error);
      // Check if it's the unknown state error
      if (error.errorCode === 103 || (error.message && error.message.includes('unknown state'))) {
        return 'Unknown';
      }
      return 'Unknown';
    }
  }

  /**
   * Đợi Bluetooth sẵn sàng (PoweredOn)
   * Retry với timeout để xử lý trường hợp Bluetooth đang ở trạng thái Unknown/Resetting
   * @param {number} maxWaitTime - Thời gian tối đa đợi (ms), mặc định 10 giây
   * @param {number} retryInterval - Khoảng thời gian giữa các lần retry (ms), mặc định 500ms
   * @returns {Promise<boolean>} - True nếu Bluetooth sẵn sàng, false nếu timeout
   */
  async waitForBluetoothReady(maxWaitTime = 15000, retryInterval = 500) {
    const startTime = Date.now();
    let retryCount = 0;
    const isSimulator = isRunningOnSimulator();
    
    // Cảnh báo sớm nếu đang chạy trên Simulator
    if (isSimulator) {
      consoleWarn('⚠️ Running on iOS Simulator - Bluetooth may not be available');
    }
    
    // Thử đợi state change event trước
    console.log('⏳ Waiting for Bluetooth state to stabilize...');
    await new Promise(resolve => setTimeout(resolve, 1000)); // Wait 1 second for state to settle
    
    while (Date.now() - startTime < maxWaitTime) {
      retryCount++;
      
      try {
        const state = await this.getBluetoothState();
        
        console.log(`[Bluetooth Check #${retryCount}] Current state: ${state}`);
        
        if (state === 'PoweredOn') {
          console.log('✅ Bluetooth is ready');
          return true;
        } else if (state === 'PoweredOff') {
          throw new Error('Bluetooth is turned off. Please enable Bluetooth in Settings.');
        } else if (state === 'Unauthorized') {
          // Hiển thị alert để mở Settings
          this.showPermissionSettingsAlert('bluetooth');
          throw new Error('Bluetooth permission denied. Please grant Bluetooth permission in Settings > Massage Chair Control > Bluetooth.');
        } else if (state === 'Unsupported') {
          // Check if running on Simulator
          if (isSimulator) {
            consoleError('❌ iOS Simulator does not support Bluetooth');
            console.log('ℹ️ Allowing operation to continue in Simulator mode (Bluetooth features will not work)');
            return 'simulator'; // Special return value for simulator mode
          }
          throw new Error('Bluetooth is not supported on this device.');
        }
        
        // State is Unknown or Resetting
        const elapsed = Date.now() - startTime;
        const remaining = maxWaitTime - elapsed;
        
        // Nếu Unknown quá lâu (> 10 retry), có thể là Simulator
        if (state === 'Unknown' && retryCount >= 10) {
          consoleWarn(`⚠️ Bluetooth stuck in Unknown state after ${retryCount} retries`);
          
          // Nếu đang trên Simulator hoặc đã retry nhiều, cho phép tiếp tục với warning
          if (isSimulator || retryCount >= 20) {
            consoleError('❌ Bluetooth state stuck in Unknown - Likely running on iOS Simulator');
            console.log('ℹ️ BYPASSING Bluetooth check to allow app to continue (Bluetooth features will NOT work)');
            console.log('ℹ️ To use Bluetooth: Run on a REAL iOS device, not Simulator');
            return 'simulator'; // Allow to continue but Bluetooth won't work
          }
        }
        
        console.log(`⏳ Bluetooth state is ${state}, waiting ${retryInterval}ms before retry... (${Math.ceil(remaining/1000)}s remaining)`);
        await new Promise(resolve => setTimeout(resolve, retryInterval));
      } catch (error) {
        // If it's a fatal error (not Unknown/Resetting), throw it
        if (error.message && (
          error.message.includes('turned off') ||
          error.message.includes('permission') ||
          error.message.includes('not supported') ||
          error.message.includes('Simulator')
        )) {
          throw error;
        }
        
        // For unknown state errors, retry
        console.log(`Bluetooth check error: ${error.message}, retrying...`);
        await new Promise(resolve => setTimeout(resolve, retryInterval));
      }
    }
    
    // Timeout - Bluetooth still not ready after max wait time
    consoleError(`❌ Bluetooth timeout after ${maxWaitTime}ms (${retryCount} retries)`);
    
    // Nếu đang trên Simulator, cho phép tiếp tục với warning
    if (isSimulator) {
      consoleError('❌ iOS Simulator does not support Bluetooth');
      console.log('ℹ️ BYPASSING check - App will continue but Bluetooth features will NOT work');
      console.log('ℹ️ To use Bluetooth: Connect a real iPhone/iPad and run on device');
      return 'simulator'; // Special return for simulator mode
    }
    
    return false;
  }

  /**
   * Kiểm tra và xử lý lỗi Bluetooth unknown state
   * @param {Error} error - Error object từ BLE operations
   * @returns {boolean} - True nếu đây là lỗi unknown state và có thể retry
   */
  isUnknownStateError(error) {
    if (!error) return false;
    
    // Check error code 103 (BluetoothInUnknownState)
    if (error.errorCode === 103) {
      return true;
    }
    
    // Check error message
    const errorMessage = error.message || error.toString() || '';
    if (errorMessage.toLowerCase().includes('unknown state') ||
        errorMessage.toLowerCase().includes('bluetooth is in unknown state')) {
      return true;
    }
    
    return false;
  }

  /**
   * Scan thiết bị BLE
   * Tìm kiếm các thiết bị BLE trong vùng lân cận
   * @param {number} timeout - Thời gian scan (ms)
   * @returns {Promise<boolean>} - True nếu scan thành công
   */
  async scanDevices(timeout = BLE_CONFIG.SCAN_TIMEOUT) {
    try {
      if (!this.isInitialized) {
        await this.initialize();
      }
      
      // Đợi Bluetooth sẵn sàng trước khi scan
      console.log('📡 Checking Bluetooth state before scanning...');
      const isReady = await this.waitForBluetoothReady();
      
      // Nếu đang ở simulator mode, cho phép tiếp tục nhưng hiển thị warning
      if (isReady === 'simulator') {
        consoleWarn('⚠️ Running in Simulator mode - scan will not find real devices');
        // Không throw error, cho phép UI hiển thị nhưng scan sẽ không tìm được device
      } else if (!isReady) {
        const finalState = await this.getBluetoothState();
        throw new Error(`Bluetooth is not ready (State: ${finalState}). Please check:\n1. Bluetooth is enabled in Settings\n2. App has Bluetooth permission\n3. You are running on a real device (not Simulator)`);
      }
      
      this.isScanning = true;
      this.discoveredDevices.clear();
      
      // Dispatch scanning state đến Redux
      if (this.dispatch) {
        this.dispatch({ type: 'ble/setScanning', payload: true });
      }
      
      // Bắt đầu scan với callback
      this.scanSubscription = this.manager.startDeviceScan(
        null, // serviceUUIDs - null để scan tất cả
        { allowDuplicates: false }, // scanOptions
        (error, device) => {
          if (error) {
            // Xử lý lỗi unknown state
            if (this.isUnknownStateError(error)) {
              consoleWarn('Bluetooth unknown state during scan, will retry...');
              // Có thể implement retry logic ở đây nếu cần
            } else {
              consoleError('Scan error:', error);
            }
            
            // Không dừng scan ngay nếu là lỗi unknown state
            if (!this.isUnknownStateError(error)) {
              this.isScanning = false;
              if (this.dispatch) {
                this.dispatch({ type: 'ble/setScanning', payload: false });
              }
            }
            return;
          }
          
          if (device) {
            // Filter out devices with "Unknown Device" name
            const deviceName = device.name || device.localName || '';
            if (deviceName.toLowerCase() === 'unknown device' || deviceName.trim() === '') {
              console.log(`Filtered out device: ${device.id} (name: "${deviceName}")`);
              return; // Skip unknown/unnamed devices
            }
            
            // Store actual device object locally
            this.discoveredDevices.set(device.id, device);
            
            // Dispatch serialized device đến Redux
            if (this.dispatch) {
              this.dispatch({
                type: 'ble/addScanResult',
                payload: serializeDevice(device)
              });
            }
          }
        }
      );
      
      // Tự động dừng scan sau timeout
      setTimeout(() => {
        this.stopScan();
      }, timeout);
      
      return true;
    } catch (error) {
      consoleError('Scan failed:', error);
      this.isScanning = false;
      
      // Enhanced error handling
      let errorMessage = error.message;
      if (this.isUnknownStateError(error)) {
        errorMessage = 'Bluetooth is initializing. Please wait a moment and try again.';
      }
      
      if (this.dispatch) {
        this.dispatch({ 
          type: 'ble/setScanning', 
          payload: false 
        });
        this.dispatch({
          type: 'ble/setConnectionError',
          payload: errorMessage
        });
      }
      
      throw error;
    }
  }

  /**
   * Dừng scan thiết bị BLE
   * @returns {Promise<boolean>} - True nếu dừng thành công
   */
  async stopScan() {
    try {
      if (this.scanSubscription) {
        this.manager.stopDeviceScan();
        this.scanSubscription = null;
      }
      
      this.isScanning = false;
      
      // Dispatch đến Redux
      if (this.dispatch) {
        this.dispatch({ type: 'ble/setScanning', payload: false });
      }
      
      return true;
    } catch (error) {
      consoleError('Stop scan failed:', error);
      return false;
    }
  }

  /**
   * Lấy danh sách thiết bị đã discover
   * @returns {Promise<Array>} - Mảng các serialized device objects (đã filter bỏ Unknown Device)
   */
  async getDiscoveredPeripherals() {
    try {
      // Trả về serialized devices cho Redux compatibility, filter bỏ Unknown Device
      const peripherals = Array.from(this.discoveredDevices.values())
        .map(serializeDevice)
        .filter(device => {
          if (!device) return false;
          const deviceName = device.name || '';
          // Filter out "Unknown Device" and empty names
          return deviceName.toLowerCase() !== 'unknown device' && deviceName.trim() !== '';
        });
      return peripherals;
    } catch (error) {
      consoleError('Get discovered peripherals failed:', error);
      return [];
    }
  }

  /**
   * Request MTU size lớn hơn để truyền dữ liệu tốt hơn
   * @param {string} deviceId - ID của thiết bị
   * @param {number} requestedMTU - MTU size yêu cầu
   * @returns {Promise<number>} - MTU size thực tế
   */
  async requestLargerMTU(deviceId, requestedMTU = 512) {
    try {
      if (!this.connectedDevice) {
        throw new Error('No device connected');
      }
      
      const actualMTU = await this.connectedDevice.requestMTU(requestedMTU);
      return actualMTU;
    } catch (error) {
      consoleError('Request MTU failed:', error);
      return 23; // Default MTU
    }
  }

  /**
   * Kết nối với thiết bị BLE
   * - Connect đến device
   * - Discover services và characteristics
   * - Setup notifications
   * - Request larger MTU
   * - Handle pairing if paircode is provided (for BLE encryption)
   * @param {string} deviceId - ID của thiết bị cần kết nối
   * @param {number} timeout - Timeout cho kết nối
   * @param {string} paircode - Pair code for BLE pairing (optional, chỉ khi ESP32 bật encryption)
   * @returns {Promise<boolean>} - True nếu kết nối thành công
   */
  async connectToDevice(deviceId, timeout = BLE_CONFIG.CONNECTION_TIMEOUT, paircode = null) {
    try {
      if (!this.isInitialized) {
        await this.initialize();
      }
      
      // Đợi Bluetooth sẵn sàng trước khi kết nối
      // Điều này xử lý trường hợp Bluetooth đang ở trạng thái Unknown/Resetting
      console.log('🔗 Checking Bluetooth state before connecting...');
      const isReady = await this.waitForBluetoothReady();
      
      // Nếu đang ở simulator mode, throw error rõ ràng
      if (isReady === 'simulator') {
        throw new Error('❌ Cannot connect to Bluetooth device on iOS Simulator.\n\niOS Simulator does not support real Bluetooth.\n\nTo test Bluetooth features:\n1. Connect a real iPhone/iPad via USB\n2. Open Xcode and select your device\n3. Build and run on the real device (Cmd+R)');
      } else if (!isReady) {
        const currentState = await this.getBluetoothState();
        throw new Error(`Bluetooth is not ready (State: ${currentState}).\n\nPlease check:\n1. Bluetooth is enabled in Settings\n2. App has Bluetooth permission in Settings > Massage Chair Control\n3. You are running on a REAL iOS device (Simulator does not support Bluetooth)\n4. Try restarting Bluetooth or the app`);
      }
      
      // Store paircode if provided (for pairing requests by OS)
      // Note: BLE pairing is handled by OS, app only stores for reference
      if (paircode) {
        this.paircode = paircode;
        console.log('🔐 Pair code stored for pairing:', paircode);
        console.log('Note: BLE pairing handled by OS. User may need to enter code manually if ESP32 requires encryption.');
      } else {
        console.log('🔓 Connecting without pair code (standard BLE - no encryption)');
      }
      
      // Dispatch connecting state
      if (this.dispatch) {
        this.dispatch({ type: 'ble/setConnecting', payload: true });
      }
      
      // Ngắt kết nối hiện tại nếu có
      if (this.connectedDevice && this.connectedDeviceId !== deviceId) {
        await this.safeDisconnect();
      }
      
      // Kết nối đến device
      console.log(`Connecting to device: ${deviceId}`);
      const device = await this.manager.connectToDevice(deviceId);
      
      // Discover services with enhanced error handling
      const discoveryResult = await this.discoverServices(device);
      
             // Check if we have the required characteristics
       if (!discoveryResult.rxCharacteristic) {
         throw new Error(`RX characteristic not found. Available characteristics: ${discoveryResult.allCharacteristics.map(c => c.uuid).join(', ')}`);
       }
       
       if (!discoveryResult.txCharacteristic) {
         throw new Error(`TX characteristic not found. Available characteristics: ${discoveryResult.allCharacteristics.map(c => c.uuid).join(', ')}`);
       }
       
       console.log(`Using RX characteristic: ${discoveryResult.rxCharacteristic.uuid}`);
       console.log(`Using TX characteristic: ${discoveryResult.txCharacteristic.uuid}`);
      
      // Store device object locally (không lưu trong Redux)
      this.connectedDevice = device;
      this.connectedDeviceId = deviceId;
      
      // Save connection to storage
      await this.saveConnectionToStorage(device);
      
      // Request larger MTU để truyền dữ liệu tốt hơn
      try {
        const mtu = await this.requestLargerMTU(deviceId, 512);
      } catch (mtuError) {
        console.log('MTU request failed, using default:', mtuError);
      }
      
      // Setup device disconnection listener
      this.setupDeviceDisconnectionListener();
      
      // Bắt đầu notifications để nhận dữ liệu từ ESP32
      const notificationStarted = await this.startNotification();
      if (!notificationStarted) {
        console.log('Notification start failed, but continuing with connection');
      }
      
      // Start heartbeat to maintain connection
      this.startHeartbeat();
      
      // Dispatch connection success với serialized data
      if (this.dispatch) {
        this.dispatch({
          type: 'ble/setConnectionSuccess',
          payload: {
            deviceInfo: serializeDevice(device),
            connectedDevice: serializeDevice(device)
          }
        });
        
        // Khởi tạo trạng thái MANUAL khi kết nối thành công
        this.dispatch({
          type: 'ble/setAutoMode',
          payload: false // MANUAL mode
        });
      }
      
      return true;
    } catch (error) {
      consoleError('Connection failed:', error);
      
      // Enhanced error messages
      let errorMessage = error.message;
      
      // Xử lý lỗi unknown state
      if (this.isUnknownStateError(error)) {
        errorMessage = 'Bluetooth is initializing. Please wait a moment and try again.';
      } else if (error.message && error.message.includes('not found')) {
        errorMessage = 'Device does not support the required service. Please check if this is the correct device.';
      } else if (error.message && error.message.includes('timeout')) {
        errorMessage = 'Connection timeout. Please try again.';
      } else if (error.message && error.message.includes('Bluetooth is not ready')) {
        errorMessage = 'Bluetooth is not ready. Please wait a moment and try again.';
      } else if (error.message && error.message.includes('No compatible service found')) {
        errorMessage = 'This device is not compatible with the app. Please try a different device.';
      } else if (error.message && error.message.includes('turned off')) {
        errorMessage = 'Please enable Bluetooth and try again.';
      } else if (error.message && error.message.includes('permission')) {
        // Hiển thị alert để mở Settings
        setTimeout(() => {
          this.showPermissionSettingsAlert('bluetooth');
        }, 500);
        errorMessage = 'Bluetooth permission denied. Tap "Mở Settings" to grant permission.';
      }
      
      // Dispatch connection error
      if (this.dispatch) {
        this.dispatch({
          type: 'ble/setConnectionError',
          payload: errorMessage
        });
      }
      
      throw error;
    }
  }

  /**
   * Setup listener cho device disconnection
   * Tự động cleanup khi thiết bị bị ngắt kết nối bất ngờ
   */
  setupDeviceDisconnectionListener() {
    if (!this.connectedDeviceId) return;
    
    // Remove existing listener nếu có
    if (this.disconnectionSubscription) {
      this.disconnectionSubscription.remove();
    }
    
    this.disconnectionSubscription = this.manager.onDeviceDisconnected(
      this.connectedDeviceId,
      (error, device) => {
        if (error) {
          consoleError('Device disconnection error:', error);
        }
        
        if (device) {
          console.log('Device disconnected unexpectedly:', device.id);
          // Tự động cleanup khi thiết bị bị ngắt kết nối
          this.handleUnexpectedDisconnection();
        }
      }
    );
  }

  /**
   * Xử lý khi thiết bị bị ngắt kết nối bất ngờ
   * Cleanup tất cả state và subscriptions
   */
  handleUnexpectedDisconnection() {
    // Clear fragment buffer
    this.resetFragmentBuffer();
    
    // Clear local state
    this.connectedDevice = null;
    this.connectedDeviceId = null;
    this.clearLastResponse();
    
    // Stop notifications
    if (this.notificationSubscription) {
      this.notificationSubscription.remove();
      this.notificationSubscription = null;
    }
    
    // Remove disconnection listener
    if (this.disconnectionSubscription) {
      this.disconnectionSubscription.remove();
      this.disconnectionSubscription = null;
    }
    
    // Update Redux state
    if (this.dispatch) {
      this.dispatch({ type: 'ble/resetBleState' });
    }
  }

  /**
   * Kiểm tra xem thiết bị có đang kết nối không
   * @param {string} deviceId - ID thiết bị (optional)
   * @returns {Promise<boolean>} - True nếu đang kết nối
   */
  async isDeviceConnected(deviceId) {
    try {
      if (!deviceId) deviceId = this.connectedDeviceId;
      if (!deviceId || !this.connectedDevice) return false;
      
      const isConnected = await this.connectedDevice.isConnected();
      return isConnected;
    } catch (error) {
      consoleError('Check connection failed:', error);
      return false;
    }
  }

  /**
   * Save connection to AsyncStorage
   * @param {Object} device - Device object
   */
  async saveConnectionToStorage(device) {
    try {
      const deviceInfo = {
        id: device.id,
        name: device.name || device.localName,
        lastConnected: new Date().toISOString()
      };
      await AsyncStorage.setItem('MSMoblieApp/lastConnected', JSON.stringify(deviceInfo));
      console.log('Connection saved to storage:', deviceInfo.name);
    } catch (error) {
      consoleError('Failed to save connection:', error);
    }
  }

  /**
   * Load connection from AsyncStorage
   * @returns {Promise<Object|null>} - Device info or null
   */
  async loadConnectionFromStorage() {
    try {
      const stored = await AsyncStorage.getItem('MSMoblieApp/lastConnected');
      if (stored) {
        return JSON.parse(stored);
      }
    } catch (error) {
      consoleError('Failed to load connection:', error);
    }
    return null;
  }

  /**
   * Clear connection from AsyncStorage
   */
  async clearConnectionFromStorage() {
    try {
      await AsyncStorage.removeItem('MSMoblieApp/lastConnected');
      console.log('Connection cleared from storage');
    } catch (error) {
      consoleError('Failed to clear connection:', error);
    }
  }

  /**
   * Enhanced service discovery with fallback
   * @param {Object} device - Device object
   * @returns {Promise<Object>} - Target service with characteristics
   */
  async discoverServices(device) {
    try {
      await device.discoverAllServicesAndCharacteristics();
      const services = await device.services();
      
      console.log('Available services:', services.map(s => s.uuid));
      
      // Try to find target service
      let targetService = services.find(s => s.uuid.toLowerCase() === this.SERVICE_UUID.toLowerCase());
      
      // If not found, try alternative service UUIDs
      if (!targetService) {
        const alternativeServices = [
          '0000ffe0-0000-1000-8000-00805f9b34fb', // HM-10 BLE service
          '6e400001-b5a3-f393-e0a9-e50e24dcca9e', // Nordic UART Service
          // Add more alternative service UUIDs as needed
        ];
        
        for (const serviceUUID of alternativeServices) {
          targetService = services.find(s => s.uuid.toLowerCase() === serviceUUID.toLowerCase());
          if (targetService) {
            console.log(`Found alternative service: ${serviceUUID}`);
            this.SERVICE_UUID = serviceUUID;
            break;
          }
        }
      }
      
      if (!targetService) {
        throw new Error(`No compatible service found. Available services: ${services.map(s => s.uuid).join(', ')}`);
      }
      
             // Discover characteristics for the target service
       const characteristics = await targetService.characteristics();
       console.log('Available characteristics:', characteristics.map(c => c.uuid));
       console.log('Characteristics with properties:', characteristics.map(c => ({
         uuid: c.uuid,
         properties: c.properties,
         canWrite: c.properties && c.properties.write,
         canNotify: c.properties && c.properties.notify,
         canRead: c.properties && c.properties.read
       })));
      
      // Try to find RX and TX characteristics
      let rxChar = characteristics.find(c => c.uuid.toLowerCase() === this.RX_CHAR_UUID.toLowerCase());
      let txChar = characteristics.find(c => c.uuid.toLowerCase() === this.TX_CHAR_UUID.toLowerCase());
      
      // If not found, try alternative characteristic UUIDs
      if (!rxChar || !txChar) {
        const alternativeCharacteristics = [
          // HM-10 BLE characteristics
          '0000ffe1-0000-1000-8000-00805f9b34fb', // HM-10 RX/TX
          // Nordic UART Service characteristics
          '6e400002-b5a3-f393-e0a9-e50e24dcca9e', // RX
          '6e400003-b5a3-f393-e0a9-e50e24dcca9e', // TX
        ];
        
                 // Find RX characteristic (writeable)
         if (!rxChar) {
           for (const charUUID of alternativeCharacteristics) {
             const char = characteristics.find(c => c.uuid.toLowerCase() === charUUID.toLowerCase());
             if (char && char.properties && char.properties.write) {
               console.log(`Found alternative RX characteristic: ${charUUID}`);
               this.RX_CHAR_UUID = charUUID;
               rxChar = char;
               break;
             }
           }
         }
         
         // Find TX characteristic (notifiable)
         if (!txChar) {
           for (const charUUID of alternativeCharacteristics) {
             const char = characteristics.find(c => c.uuid.toLowerCase() === charUUID.toLowerCase());
             if (char && char.properties && char.properties.notify) {
               console.log(`Found alternative TX characteristic: ${charUUID}`);
               this.TX_CHAR_UUID = charUUID;
               txChar = char;
               break;
             }
           }
         }
         
         // If still not found, try to use the same characteristic for both RX and TX
         if (!rxChar && !txChar && characteristics.length > 0) {
           // Find a characteristic that supports write
           const writeableChar = characteristics.find(c => c.properties && c.properties.write);
           if (writeableChar) {
             console.log(`Using writeable characteristic for both RX and TX: ${writeableChar.uuid}`);
             this.RX_CHAR_UUID = writeableChar.uuid;
             this.TX_CHAR_UUID = writeableChar.uuid;
             rxChar = writeableChar;
             txChar = writeableChar;
           } else {
             // If no writeable characteristic found, use the first one but log a warning
             const singleChar = characteristics[0];
             consoleWarn(`No writeable characteristic found. Using first characteristic: ${singleChar.uuid}`);
             this.RX_CHAR_UUID = singleChar.uuid;
             this.TX_CHAR_UUID = singleChar.uuid;
             rxChar = singleChar;
             txChar = singleChar;
           }
         }
      }
      
             // Log characteristic discovery results
       console.log('RX Characteristic found:', rxChar ? rxChar.uuid : 'NOT FOUND');
       console.log('TX Characteristic found:', txChar ? txChar.uuid : 'NOT FOUND');
       
       if (!rxChar) {
         consoleWarn('RX characteristic not found. Available characteristics:', characteristics.map(c => c.uuid));
         consoleWarn('Writeable characteristics:', characteristics.filter(c => c.properties && c.properties.write).map(c => c.uuid));
       }
       if (!txChar) {
         consoleWarn('TX characteristic not found. Available characteristics:', characteristics.map(c => c.uuid));
         consoleWarn('Notifiable characteristics:', characteristics.filter(c => c.properties && c.properties.notify).map(c => c.uuid));
       }
      
      return {
        service: targetService,
        rxCharacteristic: rxChar,
        txCharacteristic: txChar,
        allCharacteristics: characteristics
      };
    } catch (error) {
      consoleError('Service discovery failed:', error);
      throw error;
    }
  }

  /**
   * Debug method để kiểm tra thông tin thiết bị và services
   * @returns {Promise<Object>} - Thông tin debug
   */
  async debugDeviceInfo() {
    try {
      if (!this.connectedDevice) {
        return { error: 'No device connected' };
      }

      const isConnected = await this.connectedDevice.isConnected();
      const services = await this.connectedDevice.services();
      
      const debugInfo = {
        deviceId: this.connectedDeviceId,
        isConnected,
        deviceName: this.connectedDevice.name || this.connectedDevice.localName,
        services: services.map(s => ({
          uuid: s.uuid,
          isPrimary: s.isPrimary
        })),
        targetService: this.SERVICE_UUID,
        targetServiceFound: services.some(s => s.uuid.toLowerCase() === this.SERVICE_UUID.toLowerCase())
      };

      // If target service exists, get its characteristics
      if (debugInfo.targetServiceFound) {
        const targetService = services.find(s => s.uuid.toLowerCase() === this.SERVICE_UUID.toLowerCase());
        const characteristics = await targetService.characteristics();
        debugInfo.characteristics = characteristics.map(c => ({
          uuid: c.uuid,
          properties: c.properties
        }));
        debugInfo.targetCharacteristics = {
          rxChar: this.RX_CHAR_UUID,
          txChar: this.TX_CHAR_UUID,
          rxCharFound: characteristics.some(c => c.uuid.toLowerCase() === this.RX_CHAR_UUID.toLowerCase()),
          txCharFound: characteristics.some(c => c.uuid.toLowerCase() === this.TX_CHAR_UUID.toLowerCase())
        };
      }

      return debugInfo;
    } catch (error) {
      consoleError('Debug device info failed:', error);
      return { error: error.message };
    }
  }

  /**
   * Calculate checksum for 9-byte packet protocol
   * @param {Array} data - 6-byte payload array
   * @returns {number} - Calculated checksum
   */
  calculateChecksum(data) {
    let sum = 0;
    
    // Sum all bytes from index 0 to 5 (6-byte payload)
    for (let i = 0; i < 6; i++) {
      sum += data[i];
    }
    
    // Add carry (Internet checksum style)
    while (sum >> 8) {
      sum = (sum & 0xFF) + (sum >> 8);
    }
    
    // One's complement + 0x10 offset
    return ((~sum) + 0x10) & 0xFF;
  }

  /**
   * Create 9-byte packet for firmware protocol
   * @param {number} deviceId - Device ID (0x70)
   * @param {number} sequence - Sequence number
   * @param {number} command - Command byte
   * @param {number} data1 - Data byte 1
   * @param {number} data2 - Data byte 2
   * @param {number} data3 - Data byte 3
   * @returns {Buffer} - 9-byte packet buffer
   */
  createPacket(deviceId, sequence, command, data1, data2, data3) {
    const payload = [deviceId, sequence, command, data1, data2, data3];
    const checksum = this.calculateChecksum(payload);
    
    const packet = [
      0x02,        // STX
      ...payload,  // 6 bytes payload
      checksum,    // 1 byte checksum
      0x03         // ETX
    ];
    
    return Buffer.from(packet);
  }

  /**
   * Send 9-byte packet command to ESP32
   * @param {number} deviceId - Device ID (0x70)
   * @param {number} sequence - Sequence number
   * @param {number} command - Command byte
   * @param {number} data1 - Data byte 1
   * @param {number} data2 - Data byte 2
   * @param {number} data3 - Data byte 3
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async sendPacketCommand(deviceId, sequence, command, data1, data2, data3) {
    try {
      if (!this.connectedDevice) {
        throw new Error('No device connected');
      }
      
      // Increment command counter
      if (!this.commandCounter) this.commandCounter = 0;
      this.commandCounter++;
      
      // Tìm command name từ COMMANDS để hiển thị
      let commandName = 'UNKNOWN';
      try {
        const foundCommand = Object.entries(COMMANDS).find(([name, cmd]) => 
          cmd.deviceId === deviceId &&
          cmd.sequence === sequence &&
          cmd.command === command &&
          cmd.data1 === data1 &&
          cmd.data2 === data2 &&
          cmd.data3 === data3
        );
        if (foundCommand) {
          commandName = foundCommand[0];
        } else {
          // Nếu không tìm thấy exact match, tìm theo command code và sequence
          const foundByCmd = Object.entries(COMMANDS).find(([name, cmd]) => 
            cmd.deviceId === deviceId &&
            cmd.sequence === sequence &&
            cmd.command === command
          );
          if (foundByCmd) {
            commandName = `${foundByCmd[0]} (data may differ)`;
          }
        }
      } catch (e) {
        // Ignore error khi tìm command name
      }
      
      // Xác định command type name
      let commandTypeName = 'UNKNOWN';
      if (command === 0x10) commandTypeName = 'AUTO_MODE';
      else if (command === 0x20) {
        if (data1 === 0x01) commandTypeName = 'ROLL_DOWN';
        else if (data1 === 0x02) commandTypeName = 'ROLL_UP';
        else if (data1 === 0x03) commandTypeName = 'KNEADING_MANUAL';
        else if (data1 === 0x04) commandTypeName = 'PERCUSSION_MANUAL';
        else if (data1 === 0x00) commandTypeName = 'ROLL_MOTOR_LEGACY';
        else commandTypeName = 'ROLL_MOTOR';
      }
      else if (command === 0x30) commandTypeName = 'KNEADING';
      else if (command === 0x40) commandTypeName = 'PERCUSSION';
      else if (command === 0x50) commandTypeName = 'COMBINE';
      else if (command === 0x60) commandTypeName = 'COMPRESSION';
      else if (command === 0x70) commandTypeName = 'INTENSITY_LEVEL';
      else if (command === 0x80) commandTypeName = 'INCLINE';
      else if (command === 0x90) commandTypeName = 'RECLINE';
      else if (command === 0xA0) commandTypeName = 'FORWARD';
      else if (command === 0xB0) commandTypeName = 'BACKWARD';
      else if (command === 0xFF) commandTypeName = 'DISCONNECT';
      
      console.log(`\n╔═══════════════════════════════════════════════════════════╗`);
      console.log(`║  📤 SEND PACKET COMMAND #${this.commandCounter.toString().padStart(3, '0')}                                    ║`);
      console.log(`╠═══════════════════════════════════════════════════════════╣`);
      console.log(`║  Command Name: ${commandName.padEnd(45)} ║`);
      console.log(`║  Command Type: ${commandTypeName.padEnd(45)} ║`);
      console.log(`╠═══════════════════════════════════════════════════════════╣`);
      console.log(`║  Device ID:   0x${deviceId.toString(16).toUpperCase().padStart(2, '0')}                                         ║`);
      console.log(`║  Sequence:    0x${sequence.toString(16).toUpperCase().padStart(2, '0')}                                         ║`);
      console.log(`║  Command:     0x${command.toString(16).toUpperCase().padStart(2, '0')} (${commandTypeName.padEnd(20)}) ║`);
      console.log(`║  Data1:       0x${data1.toString(16).toUpperCase().padStart(2, '0')} (Function: ${data1 === 0x01 ? 'ROLL_DOWN' : data1 === 0x02 ? 'ROLL_UP' : data1 === 0x03 ? 'KNEADING' : data1 === 0x04 ? 'PERCUSSION' : data1 === 0x00 ? 'LEGACY' : 'N/A'}) ║`);
      console.log(`║  Data2:       0x${data2.toString(16).toUpperCase().padStart(2, '0')} (${data2 === 0xF0 ? 'ON' : data2 === 0x00 ? 'OFF' : 'OTHER'})                                         ║`);
      console.log(`║  Data3:       0x${data3.toString(16).toUpperCase().padStart(2, '0')}                                         ║`);
      console.log(`╚═══════════════════════════════════════════════════════════╝`);
      
      // Create 9-byte packet
      const packet = this.createPacket(deviceId, sequence, command, data1, data2, data3);
      
      const packetHex = Array.from(packet).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
      console.log(`📦 Packet (hex): ${packetHex}`);
      
      // Verify RX characteristic exists
      const services = await this.connectedDevice.services();
      const targetService = services.find(s => s.uuid.toLowerCase() === this.SERVICE_UUID.toLowerCase());
      if (!targetService) {
        throw new Error(`Service ${this.SERVICE_UUID} not found`);
      }
      
      const characteristics = await targetService.characteristics();
      const rxChar = characteristics.find(c => c.uuid.toLowerCase() === this.RX_CHAR_UUID.toLowerCase());
      if (!rxChar) {
        throw new Error(`RX characteristic ${this.RX_CHAR_UUID} not found. Available characteristics: ${characteristics.map(c => c.uuid).join(', ')}`);
      }
      
      console.log(`Sending packet using characteristic: ${rxChar.uuid}`);
      
      // Convert packet to firmware format: STX + ASCII_representation_of_hex_data + ETX
      const dataWithoutStxEtx = Array.from(packet).slice(1, -1); // Remove STX and ETX
      const hexString = dataWithoutStxEtx.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
      const asciiBytes = hexString.split('').map(char => char.charCodeAt(0));
      const finalPacket = [0x02, ...asciiBytes, 0x03]; // Add STX and ETX
      
      console.log(`Original packet: ${Array.from(packet).map(b => '0x' + b.toString(16).toUpperCase().padStart(2, '0')).join(' ')}`);
      console.log(`Data part hex string: "${hexString}"`);
      console.log(`Final packet for firmware: ${finalPacket.map(b => '0x' + b.toString(16).toUpperCase().padStart(2, '0')).join(' ')}`);
      
      const finalPacketBuffer = Buffer.from(finalPacket);
      
      // Convert final packet to base64 for BLE transmission
      const finalPacketBase64 = finalPacketBuffer.toString('base64');
      console.log(`Final packet base64: "${finalPacketBase64}"`);
      
      // Try different write methods based on characteristic properties
      try {
        await this.connectedDevice.writeCharacteristicWithResponseForService(
          this.SERVICE_UUID,
          this.RX_CHAR_UUID,
          finalPacketBase64
        );
        console.log('Packet sent successfully with response');
      } catch (writeError) {
        // Xử lý lỗi unknown state
        if (this.isUnknownStateError(writeError)) {
          // Đợi Bluetooth sẵn sàng và thử lại
          consoleWarn('Bluetooth unknown state during write, waiting for Bluetooth to be ready...');
          const isReady = await this.waitForBluetoothReady(5000); // Wait up to 5 seconds
          if (isReady) {
            // Retry write with response
            try {
              await this.connectedDevice.writeCharacteristicWithResponseForService(
                this.SERVICE_UUID,
                this.RX_CHAR_UUID,
                finalPacketBase64
              );
              console.log('Packet sent successfully with response (after retry)');
            } catch (retryError) {
              // If still fails, try without response
              console.log('Write with response still failed after retry, trying without response:', retryError.message);
              await this.connectedDevice.writeCharacteristicWithoutResponseForService(
                this.SERVICE_UUID,
                this.RX_CHAR_UUID,
                finalPacketBase64
              );
              console.log('Packet sent successfully without response');
            }
          } else {
            throw new Error('Bluetooth is not ready. Please try again.');
          }
        } else {
          console.log('Write with response failed, trying without response:', writeError.message);
          // Try write without response
          await this.connectedDevice.writeCharacteristicWithoutResponseForService(
            this.SERVICE_UUID,
            this.RX_CHAR_UUID,
            finalPacketBase64
          );
          console.log('Packet sent successfully without response');
        }
      }
      
      return true;
    } catch (error) {
      consoleError('Send packet command failed:', error);
      
      // Xử lý lỗi unknown state
      if (this.isUnknownStateError(error)) {
        throw new Error('Bluetooth đang khởi tạo. Vui lòng đợi một chút và thử lại.');
      }
      
      // Provide more specific error messages
      if (error.message && error.message.includes('No device connected')) {
        throw new Error('Không có thiết bị kết nối. Vui lòng kết nối thiết bị trước.');
      } else if (error.message && error.message.includes('Service') && error.message.includes('not found')) {
        throw new Error('Không tìm thấy service BLE. Vui lòng kiểm tra kết nối.');
      } else if (error.message && error.message.includes('Characteristic') && error.message.includes('not found')) {
        throw new Error('Không tìm thấy characteristic BLE. Vui lòng kiểm tra kết nối.');
      } else {
        throw new Error(`Lỗi gửi lệnh: ${error.message || error.toString()}`);
      }
    }
  }

  /**
   * Legacy sendCommand method - now redirects to new packet protocol
   * @param {string} command - Command string (for backward compatibility)
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async sendCommand(command) {
    consoleWarn('sendCommand() is deprecated. Use specific command methods instead.');
    
    // Try to parse old command format for backward compatibility
    if (command.includes(':')) {
      const [cmd, value] = command.split(':');
      return await this.sendLegacyCommand(cmd, value);
    }
    
    throw new Error('Invalid command format. Use specific command methods.');
  }

  /**
   * Send legacy command (for backward compatibility)
   * @param {string} cmd - Command name
   * @param {string} value - Command value
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async sendLegacyCommand(cmd, value) {
    try {
      console.log(`Converting legacy command: ${cmd}:${value}`);
      
      // Map legacy commands to new packet format
      switch (cmd) {
      case 'MODE':
        if (value === 'AUTO') {
          return await this.enableAutoMode();
        } else if (value === 'MANUAL') {
          return await this.disableAutoMode();
        }
        break;
        
      case 'RECLINE':
        const reclineValue = parseInt(value);
        if (reclineValue === 1) {
          return await this.controlRecline(true, 1);
        } else if (reclineValue === 2) {
          return await this.controlRecline(true, 2);
        } else {
          return await this.controlRecline(false, 0);
        }
        
      case 'INCLINE':
        const inclineValue = parseInt(value);
        if (inclineValue === 1) {
          return await this.controlIncline(true, 1);
        } else if (inclineValue === 2) {
          return await this.controlIncline(true, 2);
        } else {
          return await this.controlIncline(false, 0);
        }
        
      case 'FORWARD':
        const forwardValue = parseInt(value);
        if (forwardValue === 1) {
          return await this.controlForward(true, 1);
        } else if (forwardValue === 2) {
          return await this.controlForward(true, 2);
        } else {
          return await this.controlForward(false, 0);
        }
        
      case 'BACKWARD':
        const backwardValue = parseInt(value);
        if (backwardValue === 1) {
          return await this.controlBackward(true, 1);
        } else if (backwardValue === 2) {
          return await this.controlBackward(true, 2);
        } else {
          return await this.controlBackward(false, 0);
        }
        
      case 'ROLLSPOT':
        if (value === 'ROLL') {
          return await this.enableRoll();
        } else if (value === 'SPOT') {
          return await this.enableSpot();
        } else {
          throw new Error(`Invalid ROLLSPOT value: ${value}. Expected 'ROLL' or 'SPOT'`);
        }
        break;
        
      case 'INTENSITY':
        const intensityLevel = parseInt(value);
        return await this.setIntensity(intensityLevel);
        
      case 'KNEADING':
        if (value === 'ON') {
          return await this.setMassageMode('KNEADING');
        } else if (value === 'OFF') {
          // Turn off all massage modes
          return await this.stopMassage();
        }
        break;
        
      case 'COMBINE':
        if (value === 'ON') {
          return await this.setMassageMode('COMBINE');
        } else if (value === 'OFF') {
          return await this.stopMassage();
        }
        break;
        
      case 'PERCUSSION':
        if (value === 'ON') {
          return await this.setMassageMode('PERCUSSION');
        } else if (value === 'OFF') {
          return await this.stopMassage();
        }
        break;
        
      case 'COMPRESSION':
        if (value === 'ON') {
          return await this.setMassageMode('COMPRESSION');
        } else if (value === 'OFF') {
          return await this.stopMassage();
        }
        break;
        
      default:
        throw new Error(`Unknown legacy command: ${cmd}`);
    }
    
    return false;
    } catch (error) {
      consoleError(`Legacy command failed: ${cmd}:${value}`, error);
      throw new Error(`Cannot send command: ${cmd}`);
    }
  }

  /**
   * Gửi dữ liệu đến ESP32 (alias cho sendCommand)
   * @param {string} deviceId - Device ID (không sử dụng)
   * @param {string} data - Dữ liệu cần gửi
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async sendData(deviceId, data) {
    try {
      if (!this.connectedDevice) {
        throw new Error('No device connected');
      }
      
      // Verify RX characteristic exists
      const services = await this.connectedDevice.services();
      const targetService = services.find(s => s.uuid.toLowerCase() === this.SERVICE_UUID.toLowerCase());
      if (!targetService) {
        throw new Error(`Service ${this.SERVICE_UUID} not found`);
      }
      
      const characteristics = await targetService.characteristics();
      const rxChar = characteristics.find(c => c.uuid.toLowerCase() === this.RX_CHAR_UUID.toLowerCase());
      if (!rxChar) {
        throw new Error(`RX characteristic ${this.RX_CHAR_UUID} not found. Available characteristics: ${characteristics.map(c => c.uuid).join(', ')}`);
      }
      
      console.log(`Sending data using characteristic: ${rxChar.uuid}`);
      
      // Convert data sang base64 với kết thúc \n
      const dataWithNewline = data + '\n';
      const dataBase64 = Buffer.from(dataWithNewline).toString('base64');
      
             // Try different write methods based on characteristic properties
       try {
         await this.connectedDevice.writeCharacteristicWithResponseForService(
           this.SERVICE_UUID,
           this.RX_CHAR_UUID,
           dataBase64
         );
       } catch (writeError) {
         // Xử lý lỗi unknown state
         if (this.isUnknownStateError(writeError)) {
           consoleWarn('Bluetooth unknown state during write, waiting for Bluetooth to be ready...');
           const isReady = await this.waitForBluetoothReady(5000);
           if (isReady) {
             try {
               await this.connectedDevice.writeCharacteristicWithResponseForService(
                 this.SERVICE_UUID,
                 this.RX_CHAR_UUID,
                 dataBase64
               );
             } catch (retryError) {
               // If still fails, try without response
               await this.connectedDevice.writeCharacteristicWithoutResponseForService(
                 this.SERVICE_UUID,
                 this.RX_CHAR_UUID,
                 dataBase64
               );
             }
           } else {
             throw new Error('Bluetooth is not ready. Please try again.');
           }
         } else {
           console.log('Write with response failed, trying without response:', writeError.message);
           // Try write without response
           await this.connectedDevice.writeCharacteristicWithoutResponseForService(
             this.SERVICE_UUID,
             this.RX_CHAR_UUID,
             dataBase64
           );
         }
       }
      
      return true;
    } catch (error) {
      consoleError('Send data failed:', error);
      throw error;
    }
  }

  /**
   * Reset fragment buffer và timeout
   * Dùng khi bắt đầu nhận dữ liệu mới hoặc khi có lỗi
   */
  resetFragmentBuffer() {
    this.dataBuffer = '';
    this.isReceivingFragments = false;
    if (this.fragmentTimeout) {
      clearTimeout(this.fragmentTimeout);
      this.fragmentTimeout = null;
    }
  }

  /**
   * Kiểm tra xem JSON string có hoàn chỉnh và hợp lệ không
   * @param {string} str - JSON string cần kiểm tra
   * @returns {boolean} - True nếu JSON hoàn chỉnh
   */
  isCompleteJSON(str) {
    if (!str.startsWith('{') || !str.endsWith('}')) return false;
    
    let braceCount = 0;
    for (let char of str) {
      if (char === '{') braceCount++;
      if (char === '}') braceCount--;
    }
    
    return braceCount === 0 && this.isValidJSON(str);
  }

  /**
   * Helper method để kiểm tra JSON hợp lệ
   * @param {string} str - String cần kiểm tra
   * @returns {boolean} - True nếu JSON hợp lệ
   */
  isValidJSON(str) {
    try {
      JSON.parse(str);
      return true;
    } catch (e) {
      return false;
    }
  }

  /**
   * Bắt đầu notifications để nhận dữ liệu từ ESP32
   * Setup listener cho TX characteristic
   * @param {Function} callback - Optional callback function
   * @returns {Promise<boolean>} - True nếu setup thành công
   */
  async startNotification(callback) {
    try {
      if (!this.connectedDevice) {
        throw new Error('No device connected');
      }
      
      // Verify device is still connected
      const isConnected = await this.connectedDevice.isConnected();
      if (!isConnected) {
        throw new Error('Device is not connected');
      }
      
      // Verify service exists before attempting to monitor
      const services = await this.connectedDevice.services();
      const targetService = services.find(s => s.uuid.toLowerCase() === this.SERVICE_UUID.toLowerCase());
      if (!targetService) {
        throw new Error(`Service ${this.SERVICE_UUID} not found on connected device`);
      }
      
      // Verify TX characteristic exists
      const characteristics = await targetService.characteristics();
      const txChar = characteristics.find(c => c.uuid.toLowerCase() === this.TX_CHAR_UUID.toLowerCase());
      if (!txChar) {
        throw new Error(`TX characteristic ${this.TX_CHAR_UUID} not found. Available characteristics: ${characteristics.map(c => c.uuid).join(', ')}`);
      }
      
      console.log(`Starting notification using characteristic: ${txChar.uuid}`);
      
      // Reset fragment buffer
      this.resetFragmentBuffer();
      
      this.notificationSubscription = this.connectedDevice.monitorCharacteristicForService(
        this.SERVICE_UUID,
        this.TX_CHAR_UUID,
        (error, characteristic) => {
          if (error) {
            // Handle different types of notification errors
            if (this.isUnknownStateError(error)) {
              consoleWarn('Bluetooth unknown state during notification - connection may be unstable');
            } else if (error.message && error.message.includes('Operation was cancelled')) {
              console.log('Notification cancelled - device disconnected or connection lost');
            } else if (error.message && error.message.includes('Device') && error.message.includes('disconnected')) {
              console.log('Notification stopped - device disconnected');
            } else {
              consoleError('Notification error:', error);
            }
            return;
          }
          
          if (characteristic?.value) {
            // Decode base64 response
            const fragment = Buffer.from(characteristic.value, 'base64').toString();
            
            if (this.debugMode) {
              console.log('ESP32 Fragment received:', fragment);
              console.log('Fragment length:', fragment.length);
              console.log('Buffer length before:', this.dataBuffer.length);
            }
            
            // Handle fragment processing
            this.processFragment(fragment, characteristic);
          }
        }
      );
      
      if (callback && typeof callback === 'function') {
        this.notificationListeners.push(callback);
      }
      
      return true;
    } catch (error) {
      // Handle different types of notification errors gracefully
      if (this.isUnknownStateError(error)) {
        consoleWarn('Bluetooth unknown state when starting notification - will retry after Bluetooth is ready');
        // Đợi Bluetooth sẵn sàng và thử lại
        const isReady = await this.waitForBluetoothReady(5000); // Wait up to 5 seconds
        if (isReady) {
          // Retry starting notification
          return await this.startNotification(callback);
        }
        return false;
      } else if (error.message && error.message.includes('Operation was cancelled')) {
        console.log('Notification start cancelled - device may have disconnected');
        return false;
      } else if (error.message && error.message.includes('Device') && error.message.includes('disconnected')) {
        console.log('Cannot start notification - device disconnected');
        return false;
      } else {
        consoleError('Start notification failed:', error);
        throw error;
      }
    }
  }

  /**
   * Xử lý fragment dữ liệu nhận từ ESP32
   * ESP32 có thể gửi JSON trong nhiều fragments do giới hạn MTU
   * @param {string} fragment - Fragment dữ liệu
   * @param {Object} characteristic - BLE characteristic object
   */
  processFragment(fragment, characteristic) {
    // Clear timeout hiện tại
    if (this.fragmentTimeout) {
      clearTimeout(this.fragmentTimeout);
    }
    
    // Kiểm tra xem có phải là bắt đầu của JSON không
    if (fragment.startsWith('{')) {
      this.dataBuffer = fragment;
      this.isReceivingFragments = true;
    }
    // Nếu đang nhận fragments, append vào buffer
    else if (this.isReceivingFragments) {
      this.dataBuffer += fragment;
    }
    // Nếu không phải JSON fragment, xử lý như dữ liệu độc lập
    else {
      this.dataBuffer = fragment;
      this.isReceivingFragments = false;
    }
    
    if (this.debugMode) {
      console.log('Buffer length after:', this.dataBuffer.length);
      console.log('Is receiving fragments:', this.isReceivingFragments);
    }
    
    // Kiểm tra xem JSON đã hoàn chỉnh chưa
    if (this.isCompleteJSON(this.dataBuffer)) {
      const completeResponse = this.dataBuffer;
      this.lastResponse = completeResponse;
      
      // Reset buffer
      this.resetFragmentBuffer();
      
      // Parse và xử lý JSON response
      this.handleCompleteResponse(completeResponse, characteristic);
    }
    // Nếu buffer quá dài mà vẫn chưa có JSON hợp lệ, reset
    else if (this.dataBuffer.length > BLE_CONFIG.MAX_BUFFER_SIZE) {
      consoleWarn('Buffer too long, resetting...');
      this.resetFragmentBuffer();
    }
    // Set timeout để reset buffer nếu không nhận được fragment tiếp theo
    else if (this.isReceivingFragments) {
      this.fragmentTimeout = setTimeout(() => {
        consoleWarn('Fragment timeout, resetting buffer');
        this.resetFragmentBuffer();
      }, BLE_CONFIG.FRAGMENT_TIMEOUT);
    }
  }

  /**
   * Xử lý response hoàn chỉnh từ ESP32
   * @param {string} completeResponse - Response string hoàn chỉnh
   * @param {Object} characteristic - BLE characteristic object
   */
  handleCompleteResponse(completeResponse, characteristic) {
    // Parse JSON responses
    if (completeResponse.startsWith('{')) {
      try {
        const jsonResponse = JSON.parse(completeResponse);
        // Handle different response types
        this.handleJSONResponse(jsonResponse);
      } catch (parseError) {
        consoleError('JSON parse error:', parseError);
        console.log('Raw response:', completeResponse);
      }
    }
    
    // Notify listeners với serializable data
    this.notifyListeners(characteristic, completeResponse);
  }

  /**
   * Xử lý các loại JSON response từ ESP32
   * @param {Object} jsonResponse - Parsed JSON response
   */
  handleJSONResponse(jsonResponse) {
    switch (jsonResponse.type) {
      case 'STATUS':
        // Cập nhật Redux state với thông tin status
        if (this.dispatch) {
          this.dispatch({
            type: 'ble/updateMassageSettings',
            payload: {
              mode: this.mapModeFromESP32(jsonResponse.mode),
              intensity: jsonResponse.intensity,
              isRunning: jsonResponse.power,
              timer: jsonResponse.timer,
              // Map các settings khác từ ESP32
              rollSpot: jsonResponse.rollSpot,
              kneading: jsonResponse.kneading,
              combine: jsonResponse.combine,
              percussion: jsonResponse.percussion,
              compression: jsonResponse.compression,
              recline: jsonResponse.recline,
              incline: jsonResponse.incline,
              backward: jsonResponse.backward,
              forward: jsonResponse.forward
            }
          });
        }
        break;
        
      case 'ACK':
        console.log('Command acknowledged:', jsonResponse.command);
        break;
        
      case 'ERROR':
        consoleError('Device error:', jsonResponse.message);
        break;
        
      case 'DISCONNECT_ACK':
        console.log('Disconnect acknowledged by device');
        break;
        
      case 'TIMER_EXPIRED':
        console.log('Massage timer expired');
        if (this.dispatch) {
          this.dispatch({
            type: 'ble/updateMassageSettings',
            payload: { isRunning: false }
          });
        }
        break;
        
      case 'AUTO_TIMEOUT':
        // ⏰ AUTO mode đã hết 20 phút → Tự động chuyển về MANUAL
        console.log('🔔 AUTO MODE TIMEOUT - Switching to MANUAL mode');
        console.log('Message:', jsonResponse.message);
        if (this.dispatch) {
          // Reset về MANUAL mode - Reset tất cả trạng thái AUTO
          this.dispatch({
            type: 'ble/setAutoMode',
            payload: false  // Set MANUAL mode và reset tất cả trạng thái AUTO
          });
          
          // Hiển thị thông báo cho user (optional)
          console.log('✅ Switched to MANUAL mode - 20 minutes completed');
        }
        break;
        
      case 'CONNECTED':
        console.log('ESP32 connection confirmed:', jsonResponse);
        break;
        
      default:
        console.log('Unknown response type:', jsonResponse.type);
    }
  }

  /**
   * Map mode number từ ESP32 sang mode string
   * @param {number} modeNumber - Mode number từ ESP32
   * @returns {string} - Mode string
   */
  mapModeFromESP32(modeNumber) {
    const modeMap = {
      0: 'relax',
      1: 'massage',
      2: 'therapy',
      3: 'custom'
    };
    return modeMap[modeNumber] || 'relax';
  }

  /**
   * Notify tất cả listeners về dữ liệu mới
   * @param {Object} characteristic - BLE characteristic
   * @param {string} response - Response string
   */
  notifyListeners(characteristic, response) {
    this.notificationListeners.forEach(listener => {
      if (typeof listener === 'function') {
        // Pass serializable data only
        listener({
          value: Array.from(Buffer.from(characteristic.value, 'base64')),
          timestamp: Date.now()
        }, response);
      }
    });
  }

  /**
   * Dừng notifications
   * @returns {Promise<boolean>} - True nếu dừng thành công
   */
  async stopNotification() {
    try {
      if (this.notificationSubscription) {
        this.notificationSubscription.remove();
        this.notificationSubscription = null;
      }
      
      // Reset fragment buffer
      this.resetFragmentBuffer();
      return true;
    } catch (error) {
      consoleError('Stop notification failed:', error);
      return false;
    }
  }

  /**
   * Lấy response cuối cùng từ ESP32
   * @returns {string|null} - Last response string
   */
  async getLastResponse() {
    return this.lastResponse;
  }

  /**
   * Clear last response
   */
  clearLastResponse() {
    this.lastResponse = null;
  }

  /**
   * Gửi lệnh điều khiển massage đến firmware
   * @param {string} command - Lệnh điều khiển
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async sendMassageCommand(command) {
    try {
      return await this.sendCommand(command);
    } catch (error) {
      consoleError('Send massage command failed:', error);
      throw error;
    }
  }

  /**
   * Bật chế độ tự động
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async enableAutoMode() {
    // Khi chuyển từ MANUAL sang AUTO mode, phải release M KNEADING và M PERCUSSION nếu đang chạy
    console.log('🔄 Switching to AUTO mode - Releasing M KNEADING and M PERCUSSION if active');
    
    // Release M KNEADING (luôn gửi để đảm bảo tắt nếu đang chạy)
    const kneadingReleaseCmd = getCommand('KNEADING_RELEASE');
    await this.sendPacketCommand(
      kneadingReleaseCmd.deviceId, 
      kneadingReleaseCmd.sequence, 
      kneadingReleaseCmd.command, 
      kneadingReleaseCmd.data1, 
      kneadingReleaseCmd.data2, 
      kneadingReleaseCmd.data3
    );
    console.log('✅ Released M KNEADING');
    
    // Release M PERCUSSION (luôn gửi để đảm bảo tắt nếu đang chạy)
    const percussionReleaseCmd = getCommand('PERCUSSION_RELEASE');
    await this.sendPacketCommand(
      percussionReleaseCmd.deviceId, 
      percussionReleaseCmd.sequence, 
      percussionReleaseCmd.command, 
      percussionReleaseCmd.data1, 
      percussionReleaseCmd.data2, 
      percussionReleaseCmd.data3
    );
    console.log('✅ Released M PERCUSSION');
    
    // Sau đó mới gửi lệnh AUTO_ON
    const cmd = getCommand('AUTO_ON');
    const result = await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
    
    console.log('enableAutoMode result:', result);
    console.log('this.dispatch available:', !!this.dispatch);
    
    // Cập nhật Redux state
    if (result && this.dispatch) {
      console.log('Dispatching updateSystemState with isAutoMode: true');
      this.dispatch({
        type: 'ble/updateSystemState',
        payload: { isAutoMode: true }
      });
    } else {
      console.log('Cannot dispatch - result:', result, 'dispatch:', !!this.dispatch);
    }
    
    return result;
  }

  /**
   * Tắt chế độ tự động
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async disableAutoMode() {
    const cmd = getCommand('AUTO_OFF');
    const result = await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
    
    // Cập nhật Redux state - Reset tất cả trạng thái AUTO khi chuyển sang MANUAL
    if (result && this.dispatch) {
      this.dispatch({
        type: 'ble/setAutoMode',
        payload: false  // Set MANUAL mode và reset tất cả trạng thái AUTO
      });
    }
    
    return result;
  }

  /**
   * Bật motor Roll (chỉ hoạt động khi ở chế độ AUTO)
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async enableRoll() {
    console.log('enableRoll() called');
    
    // Kiểm tra trạng thái AUTO mode trước khi gửi
    const systemState = this.getSystemState();
    console.log('enableRoll() - systemState:', systemState);
    console.log('enableRoll() - isAutoMode:', systemState?.isAutoMode);
    
    if (!systemState?.isAutoMode) {
      consoleWarn('ROLL command ignored - not in AUTO mode');
      throw new Error('Roll motor chỉ hoạt động khi ở chế độ AUTO');
    }
    
    console.log('✅ ROLL command will be sent');
    const cmd = getCommand('ROLL_ON');
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Tắt motor Roll
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async disableRoll() {
    const cmd = getCommand('ROLL_OFF');
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Bật chế độ SPOT (roll off) - chỉ hoạt động khi ở chế độ AUTO
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async enableSpot() {
    // Kiểm tra trạng thái AUTO mode trước khi gửi
    const systemState = this.getSystemState();
    if (!systemState?.isAutoMode) {
      consoleWarn('SPOT command ignored - not in AUTO mode');
      throw new Error('Spot mode chỉ hoạt động khi ở chế độ AUTO');
    }
    
    const cmd = getCommand('SPOT_ON');
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Đặt cường độ massage (chỉ hoạt động với PERCUSSION, COMPRESSION, COMBINE)
   * @param {number} level - Cường độ (1 hoặc 2)
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async setIntensity(level) {
    // Kiểm tra massage mode hiện tại
    const systemState = this.getSystemState();
    const currentMode = this.getCurrentMassageMode();
    
    if (!currentMode || !['PERCUSSION', 'COMPRESSION', 'COMBINE'].includes(currentMode)) {
      consoleWarn(`INTENSITY command ignored - not in supported mode. Current mode: ${currentMode}`);
      throw new Error(`Cường độ chỉ điều khiển được khi ở chế độ PERCUSSION, COMPRESSION, hoặc COMBINE. Chế độ hiện tại: ${currentMode || 'NONE'}`);
    }
    
    if (level === 1) {
      return await this.sendPacketCommand(0x70, 0x73, 0x70, 0x00, 0x00, 0x50);
    } else if (level === 2) {
      return await this.sendPacketCommand(0x70, 0x73, 0x70, 0x00, 0x00, 0x50);
    } else {
      throw new Error('Intensity level must be 1 or 2');
    }
  }

  /**
   * Chọn chế độ massage (chỉ hoạt động khi ở chế độ AUTO)
   * @param {string} mode - Chế độ massage ('KNEADING', 'COMBINE', 'PERCUSSION', 'COMPRESSION', 'DEFAULT')
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async setMassageMode(mode) {
    // Kiểm tra trạng thái AUTO mode trước khi gửi
    const systemState = this.getSystemState();
    if (!systemState?.isAutoMode) {
      consoleWarn(`${mode} command ignored - not in AUTO mode`);
      throw new Error(`Chế độ ${mode} chỉ hoạt động khi ở chế độ AUTO`);
    }
    
    let cmdName;
    switch (mode) {
      case 'KNEADING':
        cmdName = 'KNEADING_ON';
        break;
      case 'COMBINE':
        cmdName = 'COMBINE_ON';
        break;
      case 'PERCUSSION':
        cmdName = 'PERCUSSION_ON';
        break;
      case 'COMPRESSION':
        cmdName = 'COMPRESSION_ON';
        break;
      default:
        throw new Error(`Invalid massage mode: ${mode}`);
    }
    
    const cmd = getCommand(cmdName);
    const result = await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
    
    // Cập nhật Redux state với massage mode
    if (result && this.dispatch) {
      const updatePayload = {
        isKneadingMode: mode === 'KNEADING',
        isCombineMode: mode === 'COMBINE',
        isPercussionMode: mode === 'PERCUSSION',
        isCompressionMode: mode === 'COMPRESSION'
      };
      
      this.dispatch({
        type: 'ble/updateSystemState',
        payload: updatePayload
      });
    }
    
    return result;
  }

  /**
   * Dừng tất cả massage modes
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async stopMassage() {
    // Reset massage mode states in Redux
    if (this.dispatch) {
      this.dispatch({
        type: 'ble/updateSystemState',
        payload: {
          isKneadingMode: false,
          isCombineMode: false,
          isPercussionMode: false,
          isCompressionMode: false
        }
      });
    }
    
    // Send ROLL_OFF command to stop roll motor
    const cmd = getCommand('ROLL_OFF');
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Lấy trạng thái hiện tại
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async getStatus() {
    // Status request - using a generic status command
    return await this.sendPacketCommand(0x70, 0x00, 0x00, 0x00, 0x00, 0x00);
  }

  /**
   * Điều khiển vị trí ghế - Ngả lưng (Recline)
   * @param {boolean} isPressed - true khi nhấn, false khi thả
   * @param {number} direction - 0 = stop, 1 = decrease, 2 = increase
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlRecline(isPressed, direction = 0) {
    console.log(`=== BLE SERVICE: controlRecline ===`);
    console.log(`isPressed: ${isPressed}`);
    console.log(`direction: ${direction}`);
    
    // Gửi command - giữ nguyên mode hiện tại (không chuyển từ AUTO sang MANUAL)
    const cmdName = (isPressed && direction > 0) ? 'RECLINE_PUSH' : 'RECLINE_RELEASE';
    const cmd = getCommand(cmdName);
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Điều khiển vị trí ghế - Nâng chân (Incline)
   * @param {boolean} isPressed - true khi nhấn, false khi thả
   * @param {number} direction - 0 = stop, 1 = decrease, 2 = increase
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlIncline(isPressed, direction = 0) {
    // Gửi command - giữ nguyên mode hiện tại (không chuyển từ AUTO sang MANUAL)
    const cmdName = (isPressed && direction > 0) ? 'INCLINE_PUSH' : 'INCLINE_RELEASE';
    const cmd = getCommand(cmdName);
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Điều khiển vị trí ghế - Lùi về (Backward)
   * @param {boolean} isPressed - true khi nhấn, false khi thả
   * @param {number} direction - 0 = stop, 1 = decrease, 2 = increase
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlBackward(isPressed, direction = 0) {
    // Gửi command - giữ nguyên mode hiện tại (không chuyển từ AUTO sang MANUAL)
    const cmdName = (isPressed && direction > 0) ? 'BACKWARD_PUSH' : 'BACKWARD_RELEASE';
    const cmd = getCommand(cmdName);
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Điều khiển vị trí ghế - Tiến tới (Forward)
   * @param {boolean} isPressed - true khi nhấn, false khi thả
   * @param {number} direction - 0 = stop, 1 = decrease, 2 = increase
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlForward(isPressed, direction = 0) {
    // Gửi command - giữ nguyên mode hiện tại (không chuyển từ AUTO sang MANUAL)
    const cmdName = (isPressed && direction > 0) ? 'FORWARD_PUSH' : 'FORWARD_RELEASE';
    const cmd = getCommand(cmdName);
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Điều khiển roll motor - Lên (Roll Up)
   * @param {boolean} isPressed - true khi nhấn, false khi thả
   * @param {number} direction - 0 = stop, 1 = up, 2 = down
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlRollUp(isPressed, direction = 0) {
    // Khi nhấn ROLL_UP, chuyển sang MANUAL mode nếu đang ở AUTO mode
    if (isPressed && direction > 0) {
      const systemState = this.getSystemState();
      if (systemState?.isAutoMode) {
        console.log('🔄 M ROLL UP: Currently in AUTO mode, switching to MANUAL mode (NO command sent)');
        await this.disableAutoMode();
        // KHÔNG gửi lệnh xuống bluetooth khi đang ở AUTO mode để tránh hiện tượng về home
        return true; // Return success nhưng không gửi command
      } else {
        console.log('✅ M ROLL UP: Already in MANUAL mode, sending command');
      }
    }
    
    // Chỉ gửi lệnh khi đã ở MANUAL mode
    const cmdName = (isPressed && direction > 0) ? 'ROLL_UP_PUSH' : 'ROLL_UP_RELEASE';
    const cmd = getCommand(cmdName);
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Điều khiển roll motor - Xuống (Roll Down)
   * @param {boolean} isPressed - true khi nhấn, false khi thả
   * @param {number} direction - 0 = stop, 1 = up, 2 = down
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlRollDown(isPressed, direction = 0) {
    // Khi nhấn ROLL_DOWN, chuyển sang MANUAL mode nếu đang ở AUTO mode
    if (isPressed && direction > 0) {
      const systemState = this.getSystemState();
      if (systemState?.isAutoMode) {
        console.log('🔄 M ROLL DOWN: Currently in AUTO mode, switching to MANUAL mode (NO command sent)');
        await this.disableAutoMode();
        // KHÔNG gửi lệnh xuống bluetooth khi đang ở AUTO mode để tránh hiện tượng về home
        return true; // Return success nhưng không gửi command
      } else {
        console.log('✅ M ROLL DOWN: Already in MANUAL mode, sending command');
      }
    }
    
    // Chỉ gửi lệnh khi đã ở MANUAL mode
    const cmdName = (isPressed && direction > 0) ? 'ROLL_DOWN_PUSH' : 'ROLL_DOWN_RELEASE';
    const cmd = getCommand(cmdName);
    return await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
  }

  /**
   * Điều khiển kneading - Toggle (Bật/Tắt) - MANUAL mode
   * Lưu ý: M KNEADING khác với KNEADING (AUTO mode)
   * - M KNEADING: Chỉ điều khiển kneading motor, không ảnh hưởng ROLL motor
   * - KNEADING (AUTO): Điều khiển trong AUTO mode với ROLL motor
   * @param {boolean} isOn - true để bật, false để tắt
   * @param {number} direction - Không sử dụng, giữ để tương thích với các control khác
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlKneading(isOn, direction = 0) {
    if (isOn) {
      // Chuyển sang MANUAL mode nếu đang ở AUTO mode
      const systemState = this.getSystemState();
      if (systemState?.isAutoMode) {
        console.log('🔄 M KNEADING: Currently in AUTO mode, switching to MANUAL mode (NO command sent)');
        await this.disableAutoMode();
        // KHÔNG gửi lệnh xuống bluetooth khi đang ở AUTO mode để tránh hiện tượng về home
        return true; // Return success nhưng không gửi command
      } else {
        console.log('✅ M KNEADING: Already in MANUAL mode, sending command');
      }
      
      // Chỉ gửi lệnh khi đã ở MANUAL mode
      // M KNEADING chỉ điều khiển kneading motor (không gửi KNEADING_ON vì đó là cho AUTO mode)
      // Gửi KNEADING_PUSH command (sử dụng ROLL_MOTOR 0x20 với data2=0x03)
      const pushCmd = getCommand('KNEADING_PUSH');
      return await this.sendPacketCommand(pushCmd.deviceId, pushCmd.sequence, pushCmd.command, pushCmd.data1, pushCmd.data2, pushCmd.data3);
    } else {
      // Tắt KNEADING motor - luôn gửi lệnh khi tắt
      const releaseCmd = getCommand('KNEADING_RELEASE');
      return await this.sendPacketCommand(releaseCmd.deviceId, releaseCmd.sequence, releaseCmd.command, releaseCmd.data1, releaseCmd.data2, releaseCmd.data3);
    }
  }

  /**
   * Điều khiển percussion - Toggle (Bật/Tắt) - MANUAL mode
   * Lưu ý: M PERCUSSION khác với PERCUSSION (AUTO mode)
   * - M PERCUSSION: Chỉ điều khiển percussion/compression motor, không ảnh hưởng ROLL motor
   * - PERCUSSION (AUTO): Điều khiển trong AUTO mode với ROLL motor
   * @param {boolean} isOn - true để bật, false để tắt
   * @param {number} direction - Không sử dụng, giữ để tương thích với các control khác
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async controlPercussion(isOn, direction = 0) {
    if (isOn) {
      // Chuyển sang MANUAL mode nếu đang ở AUTO mode
      const systemState = this.getSystemState();
      if (systemState?.isAutoMode) {
        console.log('🔄 M PERCUSSION: Currently in AUTO mode, switching to MANUAL mode (NO command sent)');
        await this.disableAutoMode();
        // KHÔNG gửi lệnh xuống bluetooth khi đang ở AUTO mode để tránh hiện tượng về home
        return true; // Return success nhưng không gửi command
      } else {
        console.log('✅ M PERCUSSION: Already in MANUAL mode, sending command');
      }
      
      // Chỉ gửi lệnh khi đã ở MANUAL mode
      // M PERCUSSION chỉ điều khiển percussion/compression motor (không gửi PERCUSSION_ON vì đó là cho AUTO mode)
      // Gửi PERCUSSION_PUSH command (sử dụng ROLL_MOTOR 0x20 với data2=0x04)
      const pushCmd = getCommand('PERCUSSION_PUSH');
      return await this.sendPacketCommand(pushCmd.deviceId, pushCmd.sequence, pushCmd.command, pushCmd.data1, pushCmd.data2, pushCmd.data3);
    } else {
      // Tắt PERCUSSION motor - luôn gửi lệnh khi tắt
      const releaseCmd = getCommand('PERCUSSION_RELEASE');
      return await this.sendPacketCommand(releaseCmd.deviceId, releaseCmd.sequence, releaseCmd.command, releaseCmd.data1, releaseCmd.data2, releaseCmd.data3);
    }
  }

  /**
   * Gửi lệnh DISCONNECT đến ESP32
   * @returns {Promise<boolean>} - True nếu gửi thành công
   */
  async sendDisconnectCommand() {
    try {
      if (!this.connectedDevice) {
        return true;
      }
      
      // Gửi DISCONNECT command đến ESP32
      const cmd = getCommand('DISCONNECT');
      const success = await this.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);
      
      if (success) {
        // Chờ device acknowledgment
        return new Promise((resolve) => {
          const timeout = setTimeout(() => {
            resolve(true);
          }, 3000);
          
          // Listen for disconnect acknowledgment
          const listener = (data, message) => {
            try {
              if (message && message.includes('DISCONNECT_ACK')) {
                clearTimeout(timeout);
                this.removeNotificationListener(listener);
                resolve(true);
              }
            } catch (error) {
              consoleError('Parse disconnect response error:', error);
            }
          };
          
          this.notificationListeners.push(listener);
        });
      }
      
      return success;
    } catch (error) {
      consoleError('Send disconnect command failed:', error);
      return false;
    }
  }

  /**
   * Safe disconnect với proper error handling
   * Thực hiện disconnect an toàn với cleanup đầy đủ
   * @returns {Promise<boolean>} - True nếu disconnect thành công
   */
  async safeDisconnect() {
    try {
      // Stop heartbeat before disconnecting
      this.stopHeartbeat();
      
      // Prevent multiple simultaneous disconnect calls
      if (this.isDisconnecting) {
        return true;
      }
      
      // Kiểm tra xem có thiết bị để disconnect không
      if (!this.connectedDevice || !this.connectedDeviceId) {
        // Vẫn cập nhật Redux state để đảm bảo UI đồng bộ
        if (this.dispatch) {
          this.dispatch({ type: 'ble/resetBleState' });
        }
        return true;
      }
      
      this.isDisconnecting = true;
      
      // Bước 1: Kiểm tra trạng thái kết nối thực tế
      let isActuallyConnected = false;
      try {
        isActuallyConnected = await this.connectedDevice.isConnected();
      } catch (error) {
        isActuallyConnected = false;
      }
      
      // Bước 2: Gửi lệnh DISCONNECT nếu thiết bị vẫn kết nối
      if (isActuallyConnected) {
        try {
          await this.sendDisconnectCommand();
        } catch (error) {
          console.log('Send disconnect command error (ignored):', error);
        }
      }
      
      // Bước 3: Dừng notifications
      try {
        await this.stopNotification();
      } catch (error) {
        console.log('Stop notification error (ignored):', error);
      }
      
      // Bước 4: Ngắt kết nối BLE
      try {
        await this.connectedDevice.cancelConnection();
      } catch (error) {
        // Bỏ qua lỗi "is not connected" vì thiết bị có thể đã ngắt kết nối
        if (error.message && error.message.includes('is not connected')) {
          console.log('Device already disconnected');
        } else {
          console.log('Cancel connection error (ignored):', error);
        }
      }
      
      // Bước 5: Remove disconnection listener
      if (this.disconnectionSubscription) {
        this.disconnectionSubscription.remove();
        this.disconnectionSubscription = null;
      }
      
      // Bước 6: Reset fragment buffer
      this.resetFragmentBuffer();
      
      // Bước 7: Dọn dẹp state cục bộ
      this.connectedDevice = null;
      this.connectedDeviceId = null;
      this.clearLastResponse();
      
      // Clear stored connection
      await this.clearConnectionFromStorage();
      
      // Bước 8: Cập nhật Redux state
      if (this.dispatch) {
        this.dispatch({ type: 'ble/resetBleState' });
      }
      
      return true;
    } catch (error) {
      consoleError('Safe disconnect failed:', error);
      
      // Vẫn dọn dẹp state ngay cả khi có lỗi
      this.connectedDevice = null;
      this.connectedDeviceId = null;
      this.clearLastResponse();
      this.resetFragmentBuffer();
      
      // Clear stored connection
      await this.clearConnectionFromStorage();
      
      // Remove disconnection listener
      if (this.disconnectionSubscription) {
        this.disconnectionSubscription.remove();
        this.disconnectionSubscription = null;
      }
      
      // Cập nhật Redux state
      if (this.dispatch) {
        this.dispatch({ type: 'ble/resetBleState' });
      }
      
      // Trả về true để cho phép app reset state
      return true;
    } finally {
      this.isDisconnecting = false;
    }
  }

  /**
   * Debounced disconnect để prevent multiple calls
   */
  debouncedDisconnect() {
    if (this.disconnectTimeout) {
      clearTimeout(this.disconnectTimeout);
    }
    
    this.disconnectTimeout = setTimeout(async () => {
      await this.safeDisconnect();
      this.disconnectTimeout = null;
    }, 500); // Chờ 500ms trước khi thực hiện disconnect
  }

  /**
   * Legacy disconnect method - sử dụng safeDisconnect
   * @returns {Promise<boolean>} - True nếu disconnect thành công
   */
  async disconnect() {
    return await this.safeDisconnect();
  }

  /**
   * Start heartbeat to maintain BLE connection
   * Sends Command: 0xEE, Data1: 0xF0 once after 5 seconds
   */
  startHeartbeat() {
    // Clear existing heartbeat if any
    this.stopHeartbeat();
    
    console.log(`Starting heartbeat after ${this.heartbeatIntervalMs}ms`);
    
    this.heartbeatInterval = setTimeout(async () => {
      try {
        if (this.connectedDevice && this.txCharacteristic) {
          console.log('Sending heartbeat...');
          await this.sendHeartbeat();
        } else {
          console.log('No connection - heartbeat cancelled');
        }
      } catch (error) {
        consoleError('Heartbeat error:', error);
      }
    }, this.heartbeatIntervalMs);
  }

  /**
   * Stop heartbeat
   * Sends Command: 0xEE, Data1: 0x00 to stop heartbeat
   */
  stopHeartbeat() {
    if (this.heartbeatInterval) {
      console.log('Stopping heartbeat...');
      clearTimeout(this.heartbeatInterval);
      this.heartbeatInterval = null;
      
      // Send stop heartbeat command
      this.sendStopHeartbeat().catch(error => {
        consoleError('Error sending stop heartbeat:', error);
      });
    }
  }

  /**
   * Send heartbeat command
   * Command: 0xEE, Data1: 0xF0
   */
  async sendHeartbeat() {
    try {
      // Create heartbeat packet: STX + Device ID + Sequence + Command + Data1 + Data2 + Data3 + Checksum + ETX
      const deviceId = 0x70; // Main device
      const sequence = 0x01;  // Heartbeat sequence
      const command = 0xEE;   // Heartbeat command
      const data1 = 0xF0;     // Heartbeat start
      const data2 = 0x00;     // Not used
      const data3 = 0x00;     // Not used
      
      // Calculate checksum for payload (6 bytes: deviceId + sequence + command + data1 + data2 + data3)
      const payload = [deviceId, sequence, command, data1, data2, data3];
      const checksum = this.calculateChecksum(payload, 6);
      
      // Create packet
      const packet = [0x02, ...payload, checksum, 0x03];
      
      // Convert to ASCII hex string for firmware
      const dataPart = payload.map(b => b.toString(16).toUpperCase().padStart(2, '0')).join('');
      const checksumHex = checksum.toString(16).toUpperCase().padStart(2, '0');
      const asciiPacket = [0x02, ...dataPart.split('').map(c => c.charCodeAt(0)), checksumHex.split('').map(c => c.charCodeAt(0)), 0x03];
      
      // Send packet
      const base64Packet = Buffer.from(asciiPacket).toString('base64');
      await this.txCharacteristic.writeWithResponse(Buffer.from(base64Packet, 'base64'));
      
      console.log('Heartbeat sent successfully');
    } catch (error) {
      consoleError('Error sending heartbeat:', error);
      throw error;
    }
  }

  /**
   * Send stop heartbeat command
   * Command: 0xEE, Data1: 0x00
   */
  async sendStopHeartbeat() {
    try {
      // Check if we have a valid connection before sending
      if (!this.txCharacteristic) {
        console.log('No TX characteristic available - skipping stop heartbeat');
        return;
      }
      
      // Create stop heartbeat packet
      const deviceId = 0x70; // Main device
      const sequence = 0x01;  // Heartbeat sequence
      const command = 0xEE;   // Heartbeat command
      const data1 = 0x00;     // Heartbeat stop
      const data2 = 0x00;     // Not used
      const data3 = 0x00;     // Not used
      
      // Calculate checksum for payload
      const payload = [deviceId, sequence, command, data1, data2, data3];
      const checksum = this.calculateChecksum(payload, 6);
      
      // Create packet
      const packet = [0x02, ...payload, checksum, 0x03];
      
      // Convert to ASCII hex string for firmware
      const dataPart = payload.map(b => b.toString(16).toUpperCase().padStart(2, '0')).join('');
      const checksumHex = checksum.toString(16).toUpperCase().padStart(2, '0');
      const asciiPacket = [0x02, ...dataPart.split('').map(c => c.charCodeAt(0)), checksumHex.split('').map(c => c.charCodeAt(0)), 0x03];
      
      // Send packet
      const base64Packet = Buffer.from(asciiPacket).toString('base64');
      await this.txCharacteristic.writeWithResponse(Buffer.from(base64Packet, 'base64'));
      
      console.log('Stop heartbeat sent successfully');
    } catch (error) {
      consoleError('Error sending stop heartbeat:', error);
      throw error;
    }
  }

  /**
   * Set heartbeat interval (configurable)
   * @param {number} intervalMs - Interval in milliseconds
   */
  setHeartbeatInterval(intervalMs) {
    this.heartbeatIntervalMs = intervalMs;
    console.log(`Heartbeat interval set to ${intervalMs}ms`);
  }

  /**
   * Thêm notification listener
   * @param {Function} callback - Callback function
   */
  addNotificationListener(callback) {
    if (callback && typeof callback === 'function') {
      this.notificationListeners.push(callback);
    }
  }

  /**
   * Remove notification listener
   * @param {Function} callback - Callback function cần remove
   */
  removeNotificationListener(callback) {
    const index = this.notificationListeners.indexOf(callback);
    if (index > -1) {
      this.notificationListeners.splice(index, 1);
    }
  }

  /**
   * Lấy danh sách thiết bị đã kết nối (returns serialized data)
   * @returns {Promise<Array>} - Mảng serialized device objects
   */
  async getConnectedDevices() {
    try {
      const connectedDevices = await this.manager.connectedDevices([]);
      return connectedDevices.map(serializeDevice);
    } catch (error) {
      consoleError('Get connected devices failed:', error);
      return [];
    }
  }

  /**
   * Lấy connection status hiện tại (serializable data only)
   * @returns {Object} - Connection status object
   */
  getConnectionStatus() {
    return {
      isInitialized: this.isInitialized,
      connectedDeviceId: this.connectedDeviceId,
      bluetoothState: this.bluetoothState,
      isScanning: this.isScanning,
      lastResponse: this.lastResponse,
      discoveredDevicesCount: this.discoveredDevices.size,
      isDisconnecting: this.isDisconnecting,
      isReceivingFragments: this.isReceivingFragments,
      bufferLength: this.dataBuffer.length
    };
  }

  /**
   * Lấy trạng thái hệ thống từ Redux store
   * @returns {Object} - System state object
   */
  getSystemState() {
    // Lấy system state từ Redux store thông qua getState
    if (this.getState) {
      const state = this.getState();
      console.log('getSystemState() - Redux state:', state);
      console.log('getSystemState() - state.ble:', state?.ble);
      console.log('getSystemState() - state.ble.systemState:', state?.ble?.systemState);
      const systemState = state?.ble?.systemState || { isAutoMode: false };
      console.log('getSystemState() - systemState:', systemState);
      console.log('getSystemState() - isAutoMode:', systemState?.isAutoMode);
      return systemState;
    }
    
    console.log('getSystemState() - No getState, using fallback');
    // Fallback: return default state if Redux not available
    return { 
      isAutoMode: false,
      isKneadingMode: false,
      isCombineMode: false,
      isPercussionMode: false,
      isCompressionMode: false
    };
  }

  /**
   * Lấy massage mode hiện tại
   * @returns {string|null} - Current massage mode hoặc null
   */
  getCurrentMassageMode() {
    const systemState = this.getSystemState();
    
    if (systemState.isKneadingMode) return 'KNEADING';
    if (systemState.isCombineMode) return 'COMBINE';
    if (systemState.isPercussionMode) return 'PERCUSSION';
    if (systemState.isCompressionMode) return 'COMPRESSION';
    
    return null;
  }

  /**
   * Lấy detailed connection status cho debugging
   * @returns {Promise<Object>} - Detailed status object
   */
  async getDetailedConnectionStatus() {
    const status = {
      hasConnectedDevice: !!this.connectedDevice,
      connectedDeviceId: this.connectedDeviceId,
      isActuallyConnected: false,
      bluetoothState: this.bluetoothState,
      isDisconnecting: this.isDisconnecting,
      fragmentStatus: {
        isReceivingFragments: this.isReceivingFragments,
        bufferLength: this.dataBuffer.length,
        hasFragmentTimeout: !!this.fragmentTimeout
      }
    };
    
    if (this.connectedDevice) {
      try {
        status.isActuallyConnected = await this.connectedDevice.isConnected();
      } catch (error) {
        status.connectionCheckError = error.message;
      }
    }
    
    return status;
  }

  /**
   * Lấy actual device object (for internal use only, not for Redux)
   * @param {string} deviceId - Device ID (optional)
   * @returns {Object|null} - Device object hoặc null
   */
  getActualDevice(deviceId) {
    if (deviceId) {
      return this.discoveredDevices.get(deviceId);
    }
    return this.connectedDevice;
  }

  /**
   * Enable/disable debug mode
   * @param {boolean} enabled - True để bật debug mode
   */
  setDebugMode(enabled) {
    this.debugMode = enabled;
    console.log(`BLE Debug mode ${enabled ? 'enabled' : 'disabled'}`);
  }

  /**
   * Cleanup method - dọn dẹp tất cả resources
   */
  cleanup() {
    // Clear debounce timeout
    if (this.disconnectTimeout) {
      clearTimeout(this.disconnectTimeout);
      this.disconnectTimeout = null;
    }
    
    // Reset fragment buffer
    this.resetFragmentBuffer();
    
    // Clear listeners
    this.notificationListeners = [];
    
    // Remove state listener
    if (this.stateSubscription) {
      this.stateSubscription.remove();
      this.stateSubscription = null;
    }
    
    // Remove app state listener
    if (this.appStateSubscription) {
      this.appStateSubscription.remove();
      this.appStateSubscription = null;
    }
    
    // Stop scanning
    if (this.scanSubscription) {
      this.manager.stopDeviceScan();
      this.scanSubscription = null;
    }
    
    // Stop notifications
    if (this.notificationSubscription) {
      this.notificationSubscription.remove();
      this.notificationSubscription = null;
    }
    
    // Remove disconnection listener
    if (this.disconnectionSubscription) {
      this.disconnectionSubscription.remove();
      this.disconnectionSubscription = null;
    }
    
    // Disconnect device
    if (this.connectedDevice) {
      this.connectedDevice.cancelConnection().catch(error => {
        console.log('Cleanup disconnect error (ignored):', error);
      });
    }
    
    // Clear state
    this.connectedDevice = null;
    this.connectedDeviceId = null;
    this.lastResponse = null;
    this.isScanning = false;
    this.isDisconnecting = false;
    this.discoveredDevices.clear();
  }

  /**
   * Destroy method cho complete cleanup
   */
  destroy() {
    this.cleanup();
    
    // Destroy manager
    if (this.manager) {
      this.manager.destroy();
    }
    
    this.isInitialized = false;
    this.dispatch = null;
  }
}

// Export singleton instance
export default new BleService();

// Export helper function cho external use
export { serializeDevice };
