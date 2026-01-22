/**
 * QRScanner.js
 * QR Code scanning screen to connect with ESP32 massage device
 * 
 * Main features:
 * - Scan QR code using camera with react-native-vision-camera
 * - Handle camera access permissions (iOS/Android)
 * - Manual QR code input when camera not available
 * - Search and connect BLE device BY NAME (no MAC/UUID needed)
 * - Display discovery and connection progress
 * - Handle errors and retry logic
 * - Support flash light on/off
 * - Support pair code for BLE pairing (if needed)
 */

import React, { useState, useEffect, useCallback, useRef } from 'react';
import {
  View,
  Text,
  StyleSheet,
  Alert,
  TouchableOpacity,
  ActivityIndicator,
  Platform,
  Linking,
  TextInput,
  SafeAreaView,
} from 'react-native';
import { Camera, useCameraDevice, useCodeScanner } from 'react-native-vision-camera';
import { useDispatch, useSelector } from 'react-redux';
import { navigateTo } from '../store/navigationSlice';
import {
  setDeviceInfo,
  setConnected,
  setConnecting,
  setConnectedDevice
} from '../store/bleSlice';
import BleService from '../services/BleService';
import { COLORS } from '../styles/Colors';

const QRScanner = () => {
  const dispatch = useDispatch();
  const { isConnected, isConnecting } = useSelector(state => state.ble);

  // === CAMERA PERMISSION STATES ===
  const [hasPermission, setHasPermission] = useState(false);
  const [permissionStatus, setPermissionStatus] = useState('checking');
  const [isLoading, setIsLoading] = useState(true);

  // === CAMERA STATES ===
  const [isActive, setIsActive] = useState(false);
  const [torch, setTorch] = useState('off');
  const [isScanning, setIsScanning] = useState(true);
  const [scanState, setScanState] = useState('scanning');

  // === UI STATES ===
  const [showManualInput, setShowManualInput] = useState(false);
  const [manualInput, setManualInput] = useState('');
  const [error, setError] = useState(null);
  const [retryCount, setRetryCount] = useState(0);
  const [lastScannedTime, setLastScannedTime] = useState(0);
  const [alertTimeout, setAlertTimeout] = useState(null);
  const [isProcessingQR, setIsProcessingQR] = useState(false);
  const [lastScannedData, setLastScannedData] = useState(null);

  // === DISCOVERY STATES ===
  const [isDiscovering, setIsDiscovering] = useState(false);
  const [discoveryProgress, setDiscoveryProgress] = useState(0);

  // === REFS FOR SYNCHRONOUS STATE TRACKING ===
  // Use refs to track state synchronously (avoid async state update issues)
  const isProcessingQRRef = useRef(false);
  const lastScannedTimeRef = useRef(0);
  const lastScannedDataRef = useRef(null);
  const alertShownRef = useRef(false);

  // === CONSTANTS ===
  // iOS needs longer cooldown to prevent multiple rapid scans
  const SCAN_COOLDOWN = Platform.OS === 'ios' ? 3000 : 2000; // 3 seconds for iOS, 2 seconds for Android
  const device = useCameraDevice('back'); // Use back camera

  // Sample QR data for massage device (name-based with optional pair code)
  const sampleQRData = '{"type":"massage_device","name":"MASSAGE_DEVICE","paircode":"000000"}';

  /**
   * Effect: Initialize camera when component mounts
   * Cleanup when component unmounts
   */
  useEffect(() => {
    initializeCamera();
    setIsActive(true);
    setIsScanning(true);
    
    // Reset refs on mount
    isProcessingQRRef.current = false;
    alertShownRef.current = false;
    lastScannedTimeRef.current = 0;
    lastScannedDataRef.current = null;

    return () => {
      setIsActive(false);
      setIsScanning(false);
      // Reset refs on unmount
      isProcessingQRRef.current = false;
      alertShownRef.current = false;
      if (alertTimeout) {
        clearTimeout(alertTimeout);
      }
    };
  }, []);

  /**
   * Initialize camera and check access permissions
   */
  const initializeCamera = async () => {
    setIsLoading(true);
    setError(null);
    try {
      await checkCameraPermission();
    } catch (error) {
      setError('Unable to initialize camera');
    } finally {
      setIsLoading(false);
    }
  };

  /**
   * Check and request camera access permission
   * Handle cases: granted, denied, restricted, not-determined
   */
  const checkCameraPermission = async () => {
    try {
      const currentPermission = await Camera.getCameraPermissionStatus();
      
      if (currentPermission === 'granted') {
        setHasPermission(true);
        setPermissionStatus('granted');
        return;
      }

      if (currentPermission === 'not-determined' || currentPermission === 'denied') {
        const permission = await Camera.requestCameraPermission();
        const granted = permission === 'granted';
        setHasPermission(granted);
        setPermissionStatus(permission);
        
        if (!granted) {
          handlePermissionDenied(permission);
        }
      } else {
        setHasPermission(false);
        setPermissionStatus(currentPermission);
        handlePermissionDenied(currentPermission);
      }
    } catch (error) {
      setHasPermission(false);
      setPermissionStatus('error');
      
      // Retry logic with 3 attempts limit
      if (retryCount < 3) {
        setTimeout(() => {
          setRetryCount(prev => prev + 1);
          checkCameraPermission();
        }, 1000);
      } else {
        setError('Camera permission check error');
      }
    }
  };

  /**
   * Handle when camera permission is denied
   * Show dialog to guide user to grant permission
   * @param {string} status - Camera permission status
   */
  const handlePermissionDenied = (status) => {
    let title = 'Camera Permission Required';
    let message = 'The app needs camera permission to scan QR codes.';
    
    switch (status) {
      case 'denied':
        message = 'Camera permission has been denied. Please grant permission to continue.';
        break;
      case 'restricted':
        message = 'Camera permission is restricted by device policy.';
        break;
      case 'not-determined':
        message = 'Camera permission not determined. Please try again.';
        break;
    }

    Alert.alert(
      title,
      message,
      [
        { text: 'Retry', onPress: () => retryPermission() },
        {
          text: 'Settings',
          onPress: () => {
            if (Platform.OS === 'ios') {
              Linking.openURL('app-settings:');
            } else {
              Linking.openSettings();
            }
          }
        },
        { text: 'Cancel', style: 'cancel' }
      ]
    );
  };

  /**
   * Retry camera permission request
   * Reset error states and retry count
   */
  const retryPermission = useCallback(() => {
    setError(null);
    setRetryCount(0);
    setIsScanning(true);
    setScanState('scanning');
    checkCameraPermission();
  }, []);

  /**
   * Configure code scanner with supported code types
   * Handle callback when QR code is detected
   * Added debounce and duplicate check for iOS
   */
  const codeScanner = useCodeScanner({
    codeTypes: ['qr', 'ean-13', 'code-128', 'code-39'],
    onCodeScanned: (codes) => {
      // Use refs for synchronous checks to prevent race conditions
      if (!codes.length) {
        return;
      }
      
      // Debug: Log scan attempt
      console.log('QR Scan attempt:', {
        codesLength: codes.length,
        isScanning,
        isActive,
        scanState,
        isProcessingQR: isProcessingQRRef.current,
        alertShown: alertShownRef.current
      });
      
      if (!isScanning || !isActive || scanState !== 'scanning') {
        console.log('QR Scan blocked: scanner not ready');
        return;
      }

      // Check processing flag using ref (synchronous)
      if (isProcessingQRRef.current || alertShownRef.current) {
        console.log('QR Scan blocked: already processing');
        return;
      }

        const qrData = codes[0].value;
      const currentTime = Date.now();

      // Check cooldown period using ref (only if we've scanned before)
      if (lastScannedTimeRef.current > 0 && currentTime - lastScannedTimeRef.current < SCAN_COOLDOWN) {
        return;
      }

      // Prevent duplicate scans of the same QR code (only if same data and within cooldown)
      if (lastScannedDataRef.current === qrData && 
          lastScannedTimeRef.current > 0 && 
          currentTime - lastScannedTimeRef.current < SCAN_COOLDOWN * 2) {
        return;
      }

      // Set processing flag immediately (synchronous)
      isProcessingQRRef.current = true;
      alertShownRef.current = true;
      lastScannedTimeRef.current = currentTime;
      lastScannedDataRef.current = qrData;

      console.log('QR Code detected, processing:', qrData);
      
      // Process the QR code
      handleQRCode(qrData);
    },
  });

  /**
   * Process scanned QR code data
   * Apply cooldown to avoid duplicate scans
   * Parse JSON - Search BLE device by NAME (no MAC/UUID needed)
   * @param {string} data - QR code data
   */
  const handleQRCode = async (data) => {
    const currentTime = Date.now();
    
    // Additional safety check: prevent processing if connecting or invalid data
    // Note: refs are already checked and set in onCodeScanned callback
    if (isConnecting || !data) {
      // Reset refs if we can't process
      isProcessingQRRef.current = false;
      alertShownRef.current = false;
      return;
    }

    // Update state (refs already set in onCodeScanned callback)
    setIsProcessingQR(true);
    setLastScannedTime(currentTime);
    setLastScannedData(data);
    setScanState('processing');

    // Immediately disable scanner to prevent further scans
      setIsActive(false);
      setIsScanning(false);

    try {
      let deviceInfo;
      
      // Try to parse JSON
      try {
        deviceInfo = JSON.parse(data);
        
        // Validate: Must have 'name' field
        if (!deviceInfo.name) {
          throw new Error('Invalid JSON: Missing "name" field');
        }
        
        // Always search by name
        deviceInfo.searchBy = 'name';
        console.log('QR Mode: Search by NAME ->', deviceInfo.name);
        
        // Extract paircode if present
        if (deviceInfo.paircode) {
          console.log('QR contains pair code:', deviceInfo.paircode);
        }
        
      } catch (parseError) {
        throw new Error('Invalid QR code format. Expected JSON with "name" field.');
      }

      dispatch(setDeviceInfo(deviceInfo));

      // Timeout to automatically resume scan if user doesn't interact
      const timeout = setTimeout(() => {
        // Reset all flags
        isProcessingQRRef.current = false;
        alertShownRef.current = false;
        setIsProcessingQR(false);
        setIsActive(true);
        setIsScanning(true);
        setScanState('scanning');
      }, 30000);

      setAlertTimeout(timeout);

      // Build alert message
      let alertMessage = `Name: ${deviceInfo.name}\nType: ${deviceInfo.type || 'massage_device'}`;
      
      if (deviceInfo.paircode) {
        alertMessage += `\nPair Code: ${deviceInfo.paircode}`;
      }

      // Show connection confirmation dialog
      Alert.alert(
        'Device Information',
        alertMessage,
        [
          {
            text: 'Cancel',
            style: 'cancel',
            onPress: () => {
              clearTimeout(timeout);
              // Reset all flags
              isProcessingQRRef.current = false;
              alertShownRef.current = false;
              setIsProcessingQR(false);
              // Add delay before re-enabling scanner on iOS
              setTimeout(() => {
              setIsActive(true);
              setIsScanning(true);
              setScanState('scanning');
              }, Platform.OS === 'ios' ? 1000 : 500);
            }
          },
          {
            text: 'Connect',
            onPress: () => {
              clearTimeout(timeout);
              connectToDevice(deviceInfo);
            }
          }
        ]
      );
    } catch (error) {
      Alert.alert('QR Code Error', error.message || 'Invalid QR code');
      // Reset all flags
      isProcessingQRRef.current = false;
      alertShownRef.current = false;
      setIsProcessingQR(false);
      // Longer delay on iOS before re-enabling scanner
      const delay = Platform.OS === 'ios' ? 3000 : 2000;
      setTimeout(() => {
        setIsActive(true);
        setIsScanning(true);
        setScanState('scanning');
      }, delay);
    }
  };

  /**
   * Search for BLE device by NAME
   * Perform BLE scan and find matching device
   * @param {Object} deviceInfo - Device information (name required)
   * @returns {Object} - Found device information
   */
  const discoverDevice = async (deviceInfo) => {
    setIsDiscovering(true);
    setDiscoveryProgress(0);

    try {
      // Start BLE scan
      await BleService.scanDevices(20000); // Scan for 20 seconds (increased from 10s)

      // Scan progress animation
      const progressInterval = setInterval(() => {
        setDiscoveryProgress(prev => {
          if (prev >= 100) {
            clearInterval(progressInterval);
            return 100;
          }
          return prev + 2; // Increase 2% every 100ms
        });
      }, 100);

      // Wait for scan to complete
      await new Promise(resolve => setTimeout(resolve, 10000)); // Increased from 5s to 10s
      clearInterval(progressInterval);
      setDiscoveryProgress(100);

      // Get list of discovered devices
      const discoveredDevices = await BleService.getDiscoveredPeripherals();
      
      console.log(`Found ${discoveredDevices.length} BLE devices`);

      let targetDevice = null;

      // Search by name (case-insensitive exact match)
      console.log(`Searching by NAME: "${deviceInfo.name}"`);
      targetDevice = discoveredDevices.find(device => {
        const deviceName = device.name || '';
        const searchName = deviceInfo.name || '';
        return deviceName.toLowerCase() === searchName.toLowerCase();
      });
      
      if (!targetDevice) {
        console.log('Exact name match failed, trying partial match...');
        // Fallback: Partial match
        targetDevice = discoveredDevices.find(device => {
          const deviceName = device.name || '';
          const searchName = deviceInfo.name || '';
          return deviceName.toLowerCase().includes(searchName.toLowerCase());
        });
      }

      if (targetDevice) {
        console.log('✅ Device found:', targetDevice.name, targetDevice.id);
        return targetDevice;
      } else {
        throw new Error(`Device not found with name "${deviceInfo.name}"`);
      }
    } catch (error) {
      throw error;
    } finally {
      setIsDiscovering(false);
      setDiscoveryProgress(0);
    }
  };

  /**
   * Connect to device
   * Perform discovery -> connect (with optional pairing) -> update state
   * @param {Object} deviceInfo - Device information from QR code
   */
  const connectToDevice = async (deviceInfo) => {
    try {
      dispatch(setConnecting(true));

      // Step 1: Device discovery (search by name)
      let discoveredDevice;
      try {
        discoveredDevice = await discoverDevice(deviceInfo);
      } catch (discoveryError) {
        throw new Error(`Device "${deviceInfo.name}" not found. Make sure device is turned on and nearby.`);
      }

      console.log('Discovered device:', discoveredDevice);

      // Step 2: Connect to device (with pair code if provided)
      let connected;
      if (deviceInfo.paircode) {
        console.log('🔐 Connecting with pair code:', deviceInfo.paircode);
        // Pass paircode to BleService for pairing
        connected = await BleService.connectToDevice(
          discoveredDevice.id, 
          15000, 
          deviceInfo.paircode
        );
      } else {
        console.log('🔓 Connecting without pair code (BLE standard)');
        connected = await BleService.connectToDevice(discoveredDevice.id);
      }
      
      if (connected) {
        // Step 3: Update application state
        const connectedDevice = {
          id: discoveredDevice.id,
          name: discoveredDevice.name || deviceInfo.name,
          mac: discoveredDevice.id,
          rssi: discoveredDevice.rssi,
          connectedAt: new Date().toISOString(),
          hasPairCode: !!deviceInfo.paircode
        };

        dispatch(setConnectedDevice(connectedDevice));
        dispatch(setConnected(true));
        
        console.log('✅ Connected successfully');
        
        // Reset all flags before navigation
        isProcessingQRRef.current = false;
        alertShownRef.current = false;
        setIsProcessingQR(false);
        
        // Navigate to control screen
        dispatch(navigateTo('Control'));
      } else {
        throw new Error('Connection failed');
      }
    } catch (error) {
      console.error('Connection error:', error);
      
      let errorMessage = 'Cannot connect to device';
      
      if (error.message.includes('not found')) {
        errorMessage = error.message;
      } else if (error.message.includes('disconnected')) {
        errorMessage = 'Device disconnected. Check device power supply.';
      } else if (error.message.includes('pairing') || error.message.includes('pair')) {
        errorMessage = 'Pairing failed. Check pair code and try again.';
      }

      Alert.alert(
        'Connection Error',
        errorMessage,
        [
          {
            text: 'Retry',
            onPress: () => {
              // Reset all flags
              isProcessingQRRef.current = false;
              alertShownRef.current = false;
              setIsProcessingQR(false);
              // Add delay before re-enabling scanner on iOS
              setTimeout(() => {
              setIsActive(true);
              setIsScanning(true);
              setScanState('scanning');
              }, Platform.OS === 'ios' ? 1000 : 500);
            }
          },
          {
            text: 'Cancel',
            style: 'cancel',
            onPress: () => {
              // Reset all flags
              isProcessingQRRef.current = false;
              alertShownRef.current = false;
              setIsProcessingQR(false);
              // Add delay before re-enabling scanner on iOS
              setTimeout(() => {
              setIsActive(true);
              setIsScanning(true);
              setScanState('scanning');
              }, Platform.OS === 'ios' ? 1000 : 500);
            }
          }
        ]
      );
    } finally {
      dispatch(setConnecting(false));
    }
  };

  /**
   * Handle manual QR code input
   * Parse data and show confirmation dialog
   */
  const handleManualQRInput = async () => {
    if (!manualInput.trim()) {
      Alert.alert('Error', 'Please enter QR data');
      return;
    }

    try {
      let deviceInfo;
      
      // Try to parse JSON
      try {
        deviceInfo = JSON.parse(manualInput);
        
        // Validate: Must have 'name' field
        if (!deviceInfo.name) {
          throw new Error('Invalid JSON: Missing "name" field');
        }
        
        // Always search by name
        deviceInfo.searchBy = 'name';
        
      } catch (parseError) {
        throw new Error('Invalid JSON format. Expected: {"type":"massage_device","name":"MASSAGE_DEVICE"}');
      }

      dispatch(setDeviceInfo(deviceInfo));

      // Build alert message
      let alertMessage = `Name: ${deviceInfo.name}\nType: ${deviceInfo.type || 'massage_device'}`;
      
      if (deviceInfo.paircode) {
        alertMessage += `\nPair Code: ${deviceInfo.paircode}`;
      }

      Alert.alert(
        'Device Information',
        alertMessage,
        [
          {
            text: 'Cancel',
            style: 'cancel',
            onPress: () => setShowManualInput(false)
          },
          {
            text: 'Connect',
            onPress: () => {
              setShowManualInput(false);
              connectToDevice(deviceInfo);
            }
          }
        ]
      );
    } catch (error) {
      Alert.alert('Data Error', error.message);
    }
  };

  /**
   * Toggle camera flash
   */
  const toggleTorch = useCallback(() => {
    setTorch(prev => prev === 'off' ? 'on' : 'off');
  }, []);

  /**
   * Go back to Home screen
   */
  const goBack = () => {
    dispatch(navigateTo('Home'));
  };

  /**
   * Get text describing camera permission status
   * @param {string} status - Permission status
   * @returns {string} - Description text
   */
  const getPermissionStatusText = (status) => {
    switch (status) {
      case 'granted': return 'Permission granted';
      case 'denied': return 'Denied';
      case 'restricted': return 'Restricted';
      case 'not-determined': return 'Not determined';
      case 'checking': return 'Checking';
      case 'error': return 'Error';
      default: return 'Unknown';
    }
  };

  // === RENDER STATES ===

  /**
   * Render trạng thái loading khi đang khởi tạo camera
   */
  if (isLoading) {
    return (
      <View style={[styles.container, styles.centerContent]}>
        <ActivityIndicator size="large" color={COLORS.PRIMARY} />
        <Text style={styles.loadingText}>Initializing camera...</Text>
      </View>
    );
  }

  /**
   * Render manual QR code input screen
   */
  if (showManualInput) {
    return (
      <View style={styles.manualInputContainer}>
        <Text style={styles.manualInputTitle}>Enter QR Code Data</Text>
        
        <TextInput
          style={styles.textInput}
          placeholder='Enter JSON: {"type":"massage_device","name":"MASSAGE_DEVICE"}'
          value={manualInput}
          onChangeText={setManualInput}
          multiline
          numberOfLines={4}
        />
        
        <TouchableOpacity
          style={styles.sampleButton}
          onPress={() => setManualInput(sampleQRData)}
        >
          <Text style={styles.sampleButtonText}>Use Sample Data</Text>
        </TouchableOpacity>
        
        <View style={styles.buttonRow}>
          <TouchableOpacity
            style={[styles.actionButton, styles.cancelButton]}
            onPress={() => {
              setShowManualInput(false);
              setManualInput('');
            }}
          >
            <Text style={styles.cancelButtonText}>Cancel</Text>
          </TouchableOpacity>
          
          <TouchableOpacity
            style={[styles.actionButton, styles.connectButton]}
            onPress={handleManualQRInput}
            disabled={isConnecting}
          >
            <Text style={styles.connectButtonText}>
              {isConnecting ? 'Connecting...' : 'Connect'}
            </Text>
          </TouchableOpacity>
        </View>
      </View>
    );
  }

  /**
   * Render camera permission denied state
   */
  if (!hasPermission) {
    return (
      <View style={styles.permissionContainer}>
        <Text style={styles.permissionIcon}>📷</Text>
        <Text style={styles.permissionTitle}>Camera Access Required</Text>
        <Text style={styles.permissionText}>
          To scan QR code, the app needs access to your camera
        </Text>
        <Text style={styles.permissionStatus}>
          Status: {getPermissionStatusText(permissionStatus)}
        </Text>
        
        <TouchableOpacity style={styles.permissionButton} onPress={retryPermission}>
          <Text style={styles.permissionButtonText}>Retry</Text>
        </TouchableOpacity>
        
        <TouchableOpacity
          style={styles.manualButton}
          onPress={() => setShowManualInput(true)}
        >
          <Text style={styles.manualButtonText}>Enter QR Manually</Text>
        </TouchableOpacity>
        
        <TouchableOpacity style={styles.backButton} onPress={goBack}>
          <Text style={styles.backButtonText}>Back</Text>
        </TouchableOpacity>
      </View>
    );
  }

  /**
   * Render error state
   */
  if (error) {
    return (
      <View style={styles.errorContainer}>
        <Text style={styles.errorIcon}>⚠️</Text>
        <Text style={styles.errorTitle}>{error}</Text>
        <Text style={styles.errorText}>
          Please try again or use the manual QR input feature
        </Text>
        
        <TouchableOpacity style={styles.permissionButton} onPress={retryPermission}>
          <Text style={styles.permissionButtonText}>Retry</Text>
        </TouchableOpacity>
        
        <TouchableOpacity
          style={styles.manualButton}
          onPress={() => setShowManualInput(true)}
        >
          <Text style={styles.manualButtonText}>Enter QR Manually</Text>
        </TouchableOpacity>
        
        <TouchableOpacity style={styles.backButton} onPress={goBack}>
          <Text style={styles.backButtonText}>Back</Text>
        </TouchableOpacity>
      </View>
    );
  }

  /**
   * Render camera not found state
   */
  if (device == null) {
    return (
      <View style={styles.errorContainer}>
        <Text style={styles.errorIcon}>📱</Text>
        <Text style={styles.errorTitle}>Camera not found</Text>
        <Text style={styles.errorText}>
          Device has no rear camera or camera is not available
        </Text>
        
        <TouchableOpacity style={styles.permissionButton} onPress={retryPermission}>
          <Text style={styles.permissionButtonText}>Retry</Text>
        </TouchableOpacity>
        
        <TouchableOpacity
          style={styles.manualButton}
          onPress={() => setShowManualInput(true)}
        >
          <Text style={styles.manualButtonText}>Enter QR Manually</Text>
        </TouchableOpacity>
        
        <TouchableOpacity style={styles.backButton} onPress={goBack}>
          <Text style={styles.backButtonText}>Back</Text>
        </TouchableOpacity>
      </View>
    );
  }

  // === RENDER MAIN CAMERA INTERFACE ===
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.container}>
        {/* Header */}
        <View style={styles.header}>
        <TouchableOpacity style={styles.headerBackButton} onPress={goBack}>
          <Text style={styles.headerBackText}>← Back</Text>
        </TouchableOpacity>
        <View style={styles.headerSpacer} />
        <Text style={styles.title}>Scan QR Code</Text>
        <View style={styles.headerSpacer} />
      </View>

      {/* Camera */}
      <Camera
        style={StyleSheet.absoluteFill}
        device={device}
        isActive={isActive}
        codeScanner={codeScanner}
        torch={torch}
        onError={(error) => {
          setError('Camera error: ' + error.message);
        }}
      />

      {/* Scan overlay */}
      <View style={styles.overlay}>
        <View style={styles.scanArea}>
          {/* Scan frame corners */}
          <View style={styles.cornerTopLeft} />
          <View style={styles.cornerTopRight} />
          <View style={styles.cornerBottomLeft} />
          <View style={styles.cornerBottomRight} />
          
          {/* Processing indicator */}
          {scanState === 'processing' && (
            <View style={styles.processingContainer}>
              <ActivityIndicator size="small" color={COLORS.WHITE} />
              <Text style={styles.processingText}>Processing...</Text>
            </View>
          )}
        </View>
        
        {/* Instructions */}
        <Text style={styles.instructionText}>
          {scanState === 'scanning'
            ? 'Point camera at QR code to connect device'
            : 'Please wait for processing...'
          }
        </Text>
      </View>

      {/* Bottom controls */}
      <View style={styles.bottomControls}>
        <TouchableOpacity style={styles.controlButton} onPress={toggleTorch}>
          <Text style={styles.controlButtonIcon}>
            {torch === 'off' ? '💡' : '🔦'}
          </Text>
          <Text style={styles.controlButtonText}>
            {torch === 'off' ? 'Turn on light' : 'Turn off light'}
          </Text>
        </TouchableOpacity>
        
        <TouchableOpacity
          style={styles.controlButton}
          onPress={() => setShowManualInput(true)}
          disabled={isDiscovering}
        >
          <Text style={styles.controlButtonIcon}>⌨️</Text>
          <Text style={styles.controlButtonText}>Input QR</Text>
        </TouchableOpacity>
      </View>

      {/* Connection loading overlay */}
      {isConnecting && !isDiscovering && (
        <View style={styles.loadingOverlay}>
          <View style={styles.loadingModal}>
            <ActivityIndicator size="large" color={COLORS.PRIMARY} />
            <Text style={styles.loadingText}>Connecting...</Text>
          </View>
        </View>
      )}

      {/* Discovery overlay */}
      {isDiscovering && (
        <View style={styles.discoveryOverlay}>
          <View style={styles.discoveryModal}>
            <ActivityIndicator size="large" color={COLORS.PRIMARY} />
            <Text style={styles.discoveryTitle}>Searching for device</Text>
            <Text style={styles.discoveryText}>Scanning BLE devices...</Text>
            
            {/* Progress bar */}
            <View style={styles.progressContainer}>
              <View style={styles.progressBackground}>
                <View 
                  style={[
                    styles.progressFill, 
                    { width: `${discoveryProgress}%` }
                  ]} 
                />
              </View>
              <Text style={styles.progressText}>{Math.round(discoveryProgress)}%</Text>
            </View>
          </View>
        </View>
      )}
    </View>
    </SafeAreaView>
  );
};

// === STYLES ===
const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
  container: {
    flex: 1,
    backgroundColor: '#000',
  },
  centerContent: {
    justifyContent: 'center',
    alignItems: 'center',
  },
  header: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    flexDirection: 'row',
    alignItems: 'center',
    paddingTop: Platform.OS === 'ios' ? 50 : 20,
    paddingHorizontal: 20,
    paddingBottom: 20,
    backgroundColor: 'rgba(0,0,0,0.8)',
    zIndex: 1,
  },
  headerBackButton: {
    paddingVertical: 8,
    paddingHorizontal: 12,
    backgroundColor: 'rgba(255, 255, 255, 0.2)',
    borderRadius: 8,
  },
  headerBackText: {
    color: COLORS.WHITE,
    fontSize: 16,
    fontWeight: '500',
  },
  headerSpacer: {
    width: 80,
  },
  title: {
    flex: 1,
    fontSize: 22,
    color: COLORS.WHITE,
    textAlign: 'center',
    fontWeight: 'bold',
  },
  overlay: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: 'rgba(0,0,0,0.3)',
  },
  scanArea: {
    width: 250,
    height: 250,
    position: 'relative',
    justifyContent: 'center',
    alignItems: 'center',
  },
  cornerTopLeft: {
    position: 'absolute',
    top: 0,
    left: 0,
    width: 30,
    height: 30,
    borderTopWidth: 4,
    borderLeftWidth: 4,
    borderColor: COLORS.PRIMARY,
  },
  cornerTopRight: {
    position: 'absolute',
    top: 0,
    right: 0,
    width: 30,
    height: 30,
    borderTopWidth: 4,
    borderRightWidth: 4,
    borderColor: COLORS.PRIMARY,
  },
  cornerBottomLeft: {
    position: 'absolute',
    bottom: 0,
    left: 0,
    width: 30,
    height: 30,
    borderBottomWidth: 4,
    borderLeftWidth: 4,
    borderColor: COLORS.PRIMARY,
  },
  cornerBottomRight: {
    position: 'absolute',
    bottom: 0,
    right: 0,
    width: 30,
    height: 30,
    borderBottomWidth: 4,
    borderRightWidth: 4,
    borderColor: COLORS.PRIMARY,
  },
  instructionText: {
    marginTop: 40,
    fontSize: 16,
    color: COLORS.WHITE,
    textAlign: 'center',
    paddingHorizontal: 40,
  },
  processingContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: 'rgba(0, 0, 0, 0.8)',
    paddingHorizontal: 16,
    paddingVertical: 8,
    borderRadius: 20,
  },
  processingText: {
    marginLeft: 8,
    color: COLORS.WHITE,
    fontSize: 14,
    fontWeight: 'bold',
  },
  bottomControls: {
    position: 'absolute',
    bottom: 0,
    left: 0,
    right: 0,
    flexDirection: 'row',
    justifyContent: 'space-around',
    padding: 20,
    backgroundColor: 'rgba(0,0,0,0.8)',
  },
  controlButton: {
    alignItems: 'center',
    backgroundColor: 'rgba(255,255,255,0.2)',
    paddingVertical: 12,
    paddingHorizontal: 16,
    borderRadius: 20,
    borderWidth: 1,
    borderColor: 'rgba(255,255,255,0.3)',
    minWidth: 80,
  },
  controlButtonIcon: {
    fontSize: 20,
    marginBottom: 4,
  },
  controlButtonText: {
    color: COLORS.WHITE,
    fontSize: 10,
    fontWeight: 'bold',
  },
  permissionContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: COLORS.BACKGROUND,
    padding: 30,
  },
  permissionIcon: {
    fontSize: 80,
    marginBottom: 30,
  },
  permissionTitle: {
    fontSize: 22,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 15,
    textAlign: 'center',
  },
  permissionText: {
    fontSize: 16,
    textAlign: 'center',
    marginBottom: 20,
    color: COLORS.TEXT_SECONDARY,
    lineHeight: 24,
  },
  permissionStatus: {
    fontSize: 14,
    textAlign: 'center',
    marginBottom: 30,
    color: COLORS.WARNING,
    fontWeight: 'bold',
  },
  permissionButton: {
    backgroundColor: COLORS.PRIMARY,
    paddingHorizontal: 40,
    paddingVertical: 15,
    borderRadius: 25,
    marginBottom: 15,
  },
  permissionButtonText: {
    color: COLORS.WHITE,
    fontSize: 16,
    fontWeight: 'bold',
  },
  manualButton: {
    backgroundColor: 'transparent',
    paddingHorizontal: 30,
    paddingVertical: 12,
    borderRadius: 20,
    borderWidth: 1,
    borderColor: COLORS.PRIMARY,
    marginBottom: 15,
  },
  manualButtonText: {
    color: COLORS.PRIMARY,
    fontSize: 14,
    fontWeight: 'bold',
  },
  backButton: {
    backgroundColor: COLORS.SECONDARY,
    paddingHorizontal: 30,
    paddingVertical: 12,
    borderRadius: 20,
  },
  backButtonText: {
    color: COLORS.WHITE,
    fontSize: 14,
    fontWeight: 'bold',
  },
  errorContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: COLORS.BACKGROUND,
    padding: 30,
  },
  errorIcon: {
    fontSize: 64,
    marginBottom: 20,
  },
  errorTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 15,
    textAlign: 'center',
  },
  errorText: {
    fontSize: 16,
    textAlign: 'center',
    marginBottom: 30,
    color: COLORS.TEXT_SECONDARY,
    lineHeight: 22,
  },
  manualInputContainer: {
    flex: 1,
    padding: 20,
    backgroundColor: COLORS.BACKGROUND,
    justifyContent: 'center',
  },
  manualInputTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 20,
    textAlign: 'center',
  },
  textInput: {
    borderWidth: 1,
    borderColor: COLORS.GRAY_MEDIUM,
    borderRadius: 10,
    padding: 15,
    fontSize: 14,
    backgroundColor: COLORS.WHITE,
    marginBottom: 15,
    textAlignVertical: 'top',
    minHeight: 120,
  },
  sampleButton: {
    backgroundColor: COLORS.WARNING,
    paddingHorizontal: 20,
    paddingVertical: 10,
    borderRadius: 20,
    alignSelf: 'center',
    marginBottom: 20,
  },
  sampleButtonText: {
    color: COLORS.WHITE,
    fontSize: 14,
    fontWeight: 'bold',
  },
  buttonRow: {
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
  actionButton: {
    paddingHorizontal: 30,
    paddingVertical: 15,
    borderRadius: 25,
    minWidth: 120,
    alignItems: 'center',
  },
  cancelButton: {
    backgroundColor: COLORS.GRAY_MEDIUM,
  },
  connectButton: {
    backgroundColor: COLORS.PRIMARY,
  },
  cancelButtonText: {
    color: COLORS.WHITE,
    fontSize: 16,
    fontWeight: 'bold',
  },
  connectButtonText: {
    color: COLORS.WHITE,
    fontSize: 16,
    fontWeight: 'bold',
  },
  loadingOverlay: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: 'rgba(0,0,0,0.7)',
    justifyContent: 'center',
    alignItems: 'center',
  },
  loadingModal: {
    backgroundColor: COLORS.WHITE,
    padding: 30,
    borderRadius: 15,
    alignItems: 'center',
    minWidth: 200,
  },
  loadingText: {
    marginTop: 15,
    fontSize: 16,
    color: COLORS.TEXT_PRIMARY,
    fontWeight: 'bold',
  },
  discoveryOverlay: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: 'rgba(0,0,0,0.8)',
    justifyContent: 'center',
    alignItems: 'center',
    zIndex: 1000,
  },
  discoveryModal: {
    backgroundColor: COLORS.WHITE,
    padding: 30,
    borderRadius: 15,
    alignItems: 'center',
    minWidth: 250,
    maxWidth: 300,
  },
  discoveryTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginTop: 15,
    marginBottom: 8,
    textAlign: 'center',
  },
  discoveryText: {
    fontSize: 14,
    color: COLORS.TEXT_SECONDARY,
    textAlign: 'center',
    marginBottom: 20,
  },
  progressContainer: {
    width: '100%',
    alignItems: 'center',
  },
  progressBackground: {
    width: '100%',
    height: 6,
    backgroundColor: COLORS.GRAY_LIGHT,
    borderRadius: 3,
    overflow: 'hidden',
    marginBottom: 8,
  },
  progressFill: {
    height: '100%',
    backgroundColor: COLORS.PRIMARY,
    borderRadius: 3,
  },
  progressText: {
    fontSize: 12,
    color: COLORS.TEXT_SECONDARY,
    fontWeight: 'bold',
  },
});

export default QRScanner;
