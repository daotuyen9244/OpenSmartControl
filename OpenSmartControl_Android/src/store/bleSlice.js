/**
 * bleSlice.js
 * Redux slice để quản lý state của Bluetooth Low Energy (BLE)
 * 
 * Tính năng:
 * - Quản lý trạng thái kết nối BLE
 * - Lưu trữ thông tin thiết bị
 * - Quản lý kết quả scan thiết bị
 * - Cài đặt massage và trạng thái hoạt động
 * - Xử lý lỗi kết nối
 */

import { createSlice, createReducer, createAction } from '@reduxjs/toolkit';

// Create action for updateSystemState to avoid Immer proxy issues
export const updateSystemStateAction = createAction('ble/updateSystemState');

/**
 * Helper function: Serialize device object để đảm bảo tính tuần tự hóa
 * Chỉ giữ lại các thuộc tính primitive, loại bỏ functions và objects phức tạp
 * @param {Object} device - Device object từ BLE service
 * @returns {Object|null} - Serialized device object hoặc null
 */
const serializeDevice = (device) => {
  if (!device) return null;
  
  return {
    id: device.id,                           // Device ID (MAC address)
    name: device.name,                       // Tên thiết bị
    rssi: device.rssi,                       // Cường độ tín hiệu
    serviceUUIDs: device.serviceUUIDs,       // Danh sách service UUIDs
    manufacturerData: device.manufacturerData, // Dữ liệu manufacturer
    isConnectable: device.isConnectable,     // Có thể kết nối hay không
    // Chỉ bao gồm các thuộc tính có thể serialize
  };
};

/**
 * BLE Redux Slice
 * Quản lý toàn bộ state liên quan đến Bluetooth và massage
 */
const bleSlice = createSlice({
  name: 'ble',
  
  /**
   * Initial state cho BLE slice
   */
  initialState: {
    // === TRẠNG THÁI KẾT NỐI ===
    isConnected: false,              // Đã kết nối thiết bị hay chưa
    isConnecting: false,             // Đang trong quá trình kết nối
    connectionStatus: 'disconnected', // 'disconnected', 'connecting', 'connected', 'error'
    connectionError: null,           // Lỗi kết nối (nếu có)
    lastConnectionTime: null,        // Thời gian kết nối cuối cùng
    
    // === THÔNG TIN THIẾT BỊ ===
    deviceInfo: null,                // Thông tin thiết bị từ QR code
    connectedDevice: null,           // Thiết bị đã kết nối
    
    // === SCAN THIẾT BỊ ===
    scanResults: [],                 // Danh sách thiết bị đã scan
    isScanning: false,               // Đang scan thiết bị hay không
    
    // === CÀI ĐẶT MASSAGE ===
    massageSettings: {
      mode: 'relax',                 // Chế độ massage: 'relax', 'deep', 'gentle'
      intensity: 'LOW',              // Cường độ: 'LOW' hoặc 'HIGH'
      isRunning: false,              // Đang chạy massage hay không
      duration: 0,                   // Thời gian massage (phút)
    },

    // === TRẠNG THÁI HỆ THỐNG ===
    systemState: {
      isAutoMode: false,             // Chế độ tự động (MANUAL = false)
      isDefaultProgramEnabled: false, // Chương trình mặc định (MANUAL = false)
      isRollEnabled: false,          // Motor Roll (MANUAL = false)
      isKneadingMode: false,         // Chế độ nhào (MANUAL = false)
      isCombineMode: false,          // Chế độ kết hợp (MANUAL = false)
      isPercussionMode: false,       // Chế độ gõ (MANUAL = false)
      isCompressionMode: false,      // Chế độ nén (MANUAL = false)
      isBusy: false,                 // Hệ thống bận (MANUAL = false)
      currentNotification: 'Manual Mode - Ready', // Thông báo hiện tại
    },

    // === DEBUG INFORMATION ===
    debugInfo: null,                 // Debug information for troubleshooting
  },

  /**
   * Reducers để xử lý các action
   */
  reducers: {
    /**
     * Set trạng thái kết nối
     * @param {Object} state - Current state
     * @param {Object} action - Action với payload boolean
     */
    setConnected: (state, action) => {
      state.isConnected = action.payload;
      state.connectionStatus = action.payload ? 'connected' : 'disconnected';
      
      if (action.payload) {
        // Khi kết nối thành công
        state.lastConnectionTime = new Date().toISOString();
        state.connectionError = null;
      } else {
        // Khi ngắt kết nối
        state.connectedDevice = null;
        state.massageSettings.isRunning = false;
        state.lastConnectionTime = null;
      }
    },

    /**
     * Set trạng thái đang kết nối
     * @param {Object} state - Current state
     * @param {Object} action - Action với payload boolean
     */
    setConnecting: (state, action) => {
      state.isConnecting = action.payload;
      state.connectionStatus = action.payload ? 'connecting' : state.connectionStatus;
      
      if (action.payload) {
        // Clear error khi bắt đầu kết nối
        state.connectionError = null;
      }
    },

    /**
     * Set thông tin thiết bị từ QR code
     * @param {Object} state - Current state
     * @param {Object} action - Action với device info payload
     */
    setDeviceInfo: (state, action) => {
      // Chỉ lưu thông tin có thể serialize
      state.deviceInfo = serializeDevice(action.payload);
    },

    /**
     * Set thiết bị đã kết nối
     * @param {Object} state - Current state
     * @param {Object} action - Action với connected device payload
     */
    setConnectedDevice: (state, action) => {
      // Chỉ lưu thông tin có thể serialize
      state.connectedDevice = serializeDevice(action.payload);
      
      if (action.payload) {
        state.isConnected = true;
        state.connectionStatus = 'connected';
        state.lastConnectionTime = new Date().toISOString();
      }
    },

    /**
     * Set kết quả scan thiết bị
     * @param {Object} state - Current state
     * @param {Object} action - Action với array of devices payload
     */
    setScanResults: (state, action) => {
      // Đảm bảo mỗi phần tử đều là object có thể serialize
      state.scanResults = Array.isArray(action.payload)
        ? action.payload.map(serializeDevice)
        : [];
    },

    /**
     * Thêm một thiết bị vào kết quả scan
     * @param {Object} state - Current state
     * @param {Object} action - Action với device payload
     */
    addScanResult: (state, action) => {
      const newDevice = serializeDevice(action.payload);
      if (!newDevice) return;
      
      // Filter out "Unknown Device" and empty names
      const deviceName = newDevice.name || '';
      if (deviceName.toLowerCase() === 'unknown device' || deviceName.trim() === '') {
        return; // Skip unknown/unnamed devices
      }
      
      // Tìm thiết bị đã tồn tại
      const existingIndex = state.scanResults.findIndex(
        device => device.id === newDevice.id
      );
      
      if (existingIndex >= 0) {
        // Cập nhật thiết bị đã tồn tại (RSSI có thể thay đổi)
        state.scanResults[existingIndex] = newDevice;
      } else {
        // Thêm thiết bị mới
        state.scanResults.push(newDevice);
      }
    },

    /**
     * Set trạng thái scanning
     * @param {Object} state - Current state
     * @param {Object} action - Action với payload boolean
     */
    setScanning: (state, action) => {
      state.isScanning = action.payload;
      
      if (action.payload) {
        // Clear kết quả scan cũ khi bắt đầu scan mới
        state.scanResults = [];
      }
    },

    /**
     * Set lỗi kết nối
     * @param {Object} state - Current state
     * @param {Object} action - Action với error message payload
     */
    setConnectionError: (state, action) => {
      state.connectionError = action.payload;
      state.isConnecting = false;
      state.connectionStatus = 'error';
    },

    /**
     * Cập nhật cài đặt massage
     * @param {Object} state - Current state
     * @param {Object} action - Action với massage settings payload
     */
    updateMassageSettings: (state, action) => {
      // Merge settings mới với settings hiện tại
      state.massageSettings = { 
        ...state.massageSettings, 
        ...action.payload 
      };
    },

    /**
     * Set kết nối thành công với thông tin đầy đủ
     * @param {Object} state - Current state
     * @param {Object} action - Action với deviceInfo và connectedDevice
     */
    setConnectionSuccess: (state, action) => {
      const { deviceInfo, connectedDevice } = action.payload;
      
      // Cập nhật trạng thái kết nối
      state.isConnected = true;
      state.isConnecting = false;
      state.connectionStatus = 'connected';
      state.connectionError = null;
      state.lastConnectionTime = new Date().toISOString();
      
      // Cập nhật thông tin thiết bị
      if (deviceInfo) {
        state.deviceInfo = serializeDevice(deviceInfo);
      }
      
      if (connectedDevice) {
        state.connectedDevice = serializeDevice(connectedDevice);
      }
    },

    /**
     * Reset toàn bộ BLE state về trạng thái ban đầu
     * @param {Object} state - Current state
     */
    resetBleState: (state) => {
      state.isConnected = false;
      state.isConnecting = false;
      state.connectedDevice = null;
      state.deviceInfo = null;
      state.connectionError = null;
      state.connectionStatus = 'disconnected';
      state.lastConnectionTime = null;
      state.scanResults = [];
      state.isScanning = false;
      state.debugInfo = null;
      
      // Reset massage settings về mặc định
      state.massageSettings = {
        mode: 'relax',
        intensity: 'LOW',
        isRunning: false,
        duration: 0,
      };

      // Reset system state về MANUAL mode
      state.systemState = {
        isAutoMode: false,
        isDefaultProgramEnabled: false,
        isRollEnabled: false,
        isKneadingMode: false,
        isCombineMode: false,
        isPercussionMode: false,
        isCompressionMode: false,
        isBusy: false,
        currentNotification: 'Manual Mode - Ready',
      };
    },

    /**
     * Cập nhật trạng thái hệ thống
     * @param {Object} state - Current state
     * @param {Object} action - Action với system state payload
     */
    // updateSystemState moved to separate reducer to avoid Immer proxy issues

    /**
     * Set chế độ AUTO/MANUAL
     * @param {Object} state - Current state
     * @param {Object} action - Action với payload boolean (true = AUTO, false = MANUAL)
     */
    setAutoMode: (state, action) => {
      state.systemState.isAutoMode = action.payload;
      if (action.payload) {
        state.systemState.currentNotification = 'Auto Mode - Active';
      } else {
        state.systemState.currentNotification = 'Manual Mode - Ready';
        // Reset các chế độ massage khi chuyển về MANUAL
        state.systemState.isDefaultProgramEnabled = false;
        state.systemState.isRollEnabled = false;
        state.systemState.isKneadingMode = false;
        state.systemState.isCombineMode = false;
        state.systemState.isPercussionMode = false;
        state.systemState.isCompressionMode = false;
        state.systemState.isBusy = false;
      }
    },

    /**
     * Set debug information
     * @param {Object} state - Current state
     * @param {Object} action - Action với payload debug info
     */
    setDebugInfo: (state, action) => {
      state.debugInfo = action.payload;
    },
  },
});

// Export actions for use in components
export const {
  setConnected,
  setConnecting,
  setDeviceInfo,
  setConnectedDevice,
  setScanResults,
  addScanResult,
  setScanning,
  setConnectionError,
  updateMassageSettings,
  setConnectionSuccess,
  resetBleState,
  updateSystemState,
  setAutoMode,
  setDebugInfo
} = bleSlice.actions;

// Export reducer for use in store
// Create separate reducer for updateSystemState to avoid Immer proxy issues
const updateSystemStateReducer = (state = {
  isAutoMode: false,
  isKneadingMode: false,
  isCombineMode: false,
  isPercussionMode: false,
  isCompressionMode: false
}, action) => {
  if (action.type === 'ble/updateSystemState') {
    console.log('Redux updateSystemState called with payload:', action.payload);
    console.log('Current systemState before update:', state);
    
    // Return completely new state object to avoid any proxy issues
    return {
      ...state,
      ...action.payload
    };
  }
  return state;
};

// Combine reducers
const combinedReducer = (state = {}, action) => {
  // Ensure initial state has systemState
  if (!state || typeof state !== 'object') {
    state = {};
  }
  
  if (!state.systemState) {
    state = {
      ...state,
      systemState: {
        isAutoMode: false,
        isKneadingMode: false,
        isCombineMode: false,
        isPercussionMode: false,
        isCompressionMode: false
      }
    };
  }
  
  if (action && action.type === 'ble/updateSystemState') {
    const currentSystemState = state.systemState || {
      isAutoMode: false,
      isKneadingMode: false,
      isCombineMode: false,
      isPercussionMode: false,
      isCompressionMode: false
    };
    
    const newSystemState = updateSystemStateReducer(currentSystemState, action);
    
    return {
      ...state,
      systemState: newSystemState
    };
  }
  return bleSlice.reducer(state, action);
};

export default combinedReducer;