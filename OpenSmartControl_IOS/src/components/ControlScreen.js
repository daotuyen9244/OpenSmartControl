/**
 * ControlScreen.js
 * Main control screen of the massage application
 * Provides interface to control ESP32 massage chair functions
 * 
 * Main features:
 * - Control AUTO/MANUAL mode
 * - Adjust massage intensity (LOW/HIGH)
 * - Select massage type (ROLL/SPOT)
 * - Control massage techniques (Kneading, Combine, Percussion, Compression)
 * - Adjust chair position (Recline/Incline)
 * - Control movement (Forward/Backward)
 * - Timer and start/stop massage
 */

import React, { useEffect, useState, useRef } from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  Alert,
  ActivityIndicator,
  ScrollView,
  Platform,
  AppState,
  SafeAreaView,
} from 'react-native';
import { useSelector, useDispatch } from 'react-redux';
import {
  resetBleState,
  updateMassageSettings,
  setConnected
} from '../store/bleSlice';
import { navigateTo } from '../store/navigationSlice';
import BleService from '../services/BleService';
import COLORS from '../styles/Colors';
import AsyncStorage from '@react-native-async-storage/async-storage';

import ChairPositionControl from './ChairPositionControl';

const ControlScreen = () => {
  const dispatch = useDispatch();
  
  // Get state from Redux store
  const {
    isConnected,
    isConnecting,
    deviceInfo,
    connectedDevice,
    connectionStatus,
    massageSettings,
    systemState
  } = useSelector(state => state.ble);

     // State for UI loading
   const [isLoading, setIsLoading] = useState(true);

  // State for detailed massage control settings
  const [controlSettings, setControlSettings] = useState({
    // Control mode: auto (automatic) or manual (manual)
    mode: 'manual', // Changed from 'auto' to 'manual'
    // Massage type: roll (rolling) or spot (spot)
    rollSpot: 'roll',
    // Massage intensity: LOW or HIGH
    intensity: 'LOW',
    // Massage techniques
    kneading: false,    // Kneading
    combine: false,     // Combine
    percussion: false,  // Percussion
    compression: false, // Compression
    // Chair position (percentage)
    recline: 0,         // Recline (0-100%)
    incline: 0,         // Incline (0-100%)
    
    
  });

  // ⏱️ State for AUTO mode timer (20 minutes = 1200 seconds)
  const [autoTimer, setAutoTimer] = useState(0);        // Remaining seconds
  const [autoTimerActive, setAutoTimerActive] = useState(false);
  const [autoStartTime, setAutoStartTime] = useState(null); // Timestamp khi bắt đầu AUTO
  const AUTO_MODE_DURATION = 20 * 60; // 20 minutes in seconds



  

  /**
   * Effect: Initialize component
   * Allow access to Control screen even without connection
   */
  useEffect(() => {
    const timer = setTimeout(() => {
      setIsLoading(false);
      // Always allow access to Control screen, no connection check required
    }, 500);

    return () => clearTimeout(timer);
  }, []);

  

  /**
   * Effect: Load timer từ AsyncStorage khi component mount
   * Khôi phục timer nếu app bị kill và mở lại
   */
  useEffect(() => {
    const loadTimerFromStorage = async () => {
      try {
        const storedStartTime = await AsyncStorage.getItem('MSMoblieApp/autoTimerStartTime');
        const storedIsActive = await AsyncStorage.getItem('MSMoblieApp/autoTimerActive');
        
        if (storedStartTime && storedIsActive === 'true') {
          const startTime = parseInt(storedStartTime);
          const now = Date.now();
          const elapsedSeconds = Math.floor((now - startTime) / 1000);
          const remainingSeconds = AUTO_MODE_DURATION - elapsedSeconds;
          
          if (remainingSeconds > 0 && systemState?.isAutoMode) {
            // Timer vẫn còn hiệu lực → Khôi phục
            setAutoStartTime(startTime);
            setAutoTimer(remainingSeconds);
            setAutoTimerActive(true);
            console.log('📂 Timer restored from storage');
            console.log('Remaining:', Math.floor(remainingSeconds / 60), 'minutes');
          } else {
            // Timer đã hết → Xóa storage
            await AsyncStorage.removeItem('MSMoblieApp/autoTimerStartTime');
            await AsyncStorage.removeItem('MSMoblieApp/autoTimerActive');
          }
        }
      } catch (error) {
        console.error('Error loading timer from storage:', error);
      }
    };
    
    loadTimerFromStorage();
  }, []);

  /**
   * Effect: Sync control mode with Redux state when connected
   */
  useEffect(() => {
    if (isConnected && systemState) {
      // Sync control mode with system state
      const newMode = systemState.isAutoMode ? 'auto' : 'manual';
      setControlSettings(prev => ({
        ...prev,
        mode: newMode
      }));
      
      // ⏱️ Khi chuyển sang AUTO mode → Bắt đầu timer với timestamp
      if (systemState.isAutoMode && !autoTimerActive) {
        const startTime = Date.now();
        setAutoStartTime(startTime);
        setAutoTimer(AUTO_MODE_DURATION);
        setAutoTimerActive(true);
        
        // Lưu vào AsyncStorage
        AsyncStorage.setItem('MSMoblieApp/autoTimerStartTime', startTime.toString());
        AsyncStorage.setItem('MSMoblieApp/autoTimerActive', 'true');
        
        console.log('🕐 AUTO mode timer started - 20 minutes');
        console.log('Start timestamp:', new Date(startTime).toLocaleTimeString());
      }
      
      // 🛑 Khi tắt AUTO mode → Dừng timer
      if (!systemState.isAutoMode && autoTimerActive) {
        setAutoTimerActive(false);
        setAutoTimer(0);
        setAutoStartTime(null);
        
        // Xóa khỏi AsyncStorage
        AsyncStorage.removeItem('MSMoblieApp/autoTimerStartTime');
        AsyncStorage.removeItem('MSMoblieApp/autoTimerActive');
        
        console.log('⏹️ AUTO mode timer stopped');
      }
    }
  }, [isConnected, systemState?.isAutoMode]);

  /**
   * Effect: Countdown timer for AUTO mode - Tính toán dựa trên timestamp
   * Đảm bảo chính xác ngay cả khi app chuyển background/foreground
   */
  useEffect(() => {
    let interval = null;
    
    if (autoTimerActive && autoStartTime) {
      // Hàm tính toán thời gian còn lại dựa trên timestamp
      const updateTimer = async () => {
        const now = Date.now();
        const elapsedSeconds = Math.floor((now - autoStartTime) / 1000);
        const remainingSeconds = AUTO_MODE_DURATION - elapsedSeconds;
        
        if (remainingSeconds <= 0) {
          // ⏰ Timer hết → Tự động TẮT AUTO mode và chuyển về MANUAL
          console.log('⏰ AUTO mode timer completed - Switching to MANUAL');
          
          // Dừng timer UI
          setAutoTimer(0);
          setAutoTimerActive(false);
          setAutoStartTime(null);
          
          // Xóa AsyncStorage
          await AsyncStorage.removeItem('MSMoblieApp/autoTimerStartTime');
          await AsyncStorage.removeItem('MSMoblieApp/autoTimerActive');
          
          // 🔄 TẮT AUTO MODE → Chuyển về MANUAL (chuẩn bị chu kỳ mới)
          try {
            await BleService.disableAutoMode();
            console.log('✅ AUTO mode disabled - Switched to MANUAL');
            console.log('System ready for next cycle');
            
            // Reset các nút KNEADING, COMBINE, PERCUSSION, COMPRESSION về trạng thái ban đầu
            setControlSettings(prev => ({ 
              ...prev, 
              mode: 'manual',
              kneading: false,
              combine: false,
              percussion: false,
              compression: false
            }));
            
            // 🔔 Hiển thị thông báo cho user
            Alert.alert(
              '⏰ AUTO Mode Completed',
              '20 minutes massage session finished.\n\nSystem switched to MANUAL mode.\nReady for next session.',
              [
                { 
                  text: 'OK',
                  onPress: () => console.log('User acknowledged timer completion')
                }
              ]
            );
          } catch (error) {
            console.error('Error disabling AUTO mode:', error);
            // Fallback: Update local state và reset các nút
            setControlSettings(prev => ({ 
              ...prev, 
              mode: 'manual',
              kneading: false,
              combine: false,
              percussion: false,
              compression: false
            }));
          }
        } else {
          setAutoTimer(remainingSeconds);
        }
      };
      
      // Update ngay lập tức
      updateTimer();
      
      // Update mỗi giây
      interval = setInterval(updateTimer, 1000);
    }
    
    return () => {
      if (interval) {
        clearInterval(interval);
      }
    };
  }, [autoTimerActive, autoStartTime, AUTO_MODE_DURATION]);

  /**
   * Effect: Handle app state changes (background/foreground)
   * Sync lại timer khi app quay về foreground
   */
  useEffect(() => {
    const subscription = AppState.addEventListener('change', async (nextAppState) => {
      if (nextAppState === 'active' && autoTimerActive && autoStartTime) {
        // App quay về foreground → Tính lại timer
        const now = Date.now();
        const elapsedSeconds = Math.floor((now - autoStartTime) / 1000);
        const remainingSeconds = AUTO_MODE_DURATION - elapsedSeconds;
        
        console.log('📱 App returned to foreground');
        console.log('Elapsed:', Math.floor(elapsedSeconds / 60), 'minutes');
        console.log('Remaining:', Math.floor(remainingSeconds / 60), 'minutes');
        
        if (remainingSeconds <= 0) {
          // Timer đã hết khi app ở background → Tắt AUTO mode
          console.log('⏰ Timer expired while app was in background');
          
          setAutoTimer(0);
          setAutoTimerActive(false);
          setAutoStartTime(null);
          
          // Xóa storage
          await AsyncStorage.removeItem('MSMoblieApp/autoTimerStartTime');
          await AsyncStorage.removeItem('MSMoblieApp/autoTimerActive');
          
          // Tắt AUTO mode → Chuyển về MANUAL
          try {
            await BleService.disableAutoMode();
            console.log('✅ AUTO mode disabled - Switched to MANUAL (from background)');
            
            // Reset các nút KNEADING, COMBINE, PERCUSSION, COMPRESSION về trạng thái ban đầu
            setControlSettings(prev => ({ 
              ...prev, 
              mode: 'manual',
              kneading: false,
              combine: false,
              percussion: false,
              compression: false
            }));
            
            // 🔔 Thông báo cho user khi quay lại app
            Alert.alert(
              '⏰ AUTO Mode Completed',
              'Your 20 minutes massage session has finished.\n\nSystem is now in MANUAL mode.\nReady for next session.',
              [
                { 
                  text: 'OK',
                  onPress: () => console.log('User acknowledged timer completion (background)')
                }
              ]
            );
          } catch (error) {
            console.error('Error disabling AUTO mode:', error);
            // Fallback: Update local state và reset các nút
            setControlSettings(prev => ({ 
              ...prev, 
              mode: 'manual',
              kneading: false,
              combine: false,
              percussion: false,
              compression: false
            }));
          }
        } else {
          setAutoTimer(remainingSeconds);
        }
      }
    });
    
    return () => {
      subscription?.remove();
    };
  }, [autoTimerActive, autoStartTime, AUTO_MODE_DURATION]);

  /**
   * Effect: Reset massage techniques to default (none selected) when component mounts
   */
  useEffect(() => {
    // Ensure no massage techniques are selected by default
    setControlSettings(prev => ({
      ...prev,
      kneading: false,
      combine: false,
      percussion: false,
      compression: false
    }));
  }, []); // Run only once when component mounts

  /**
   * Send command to ESP32 via BLE
   * @param {string} command - Command to send
   * @param {string|number} value - Accompanying value (optional)
   * @returns {boolean} - Success or failure
   */
  const sendCommand = async (command, value = null) => {
    try {
      if (!isConnected || !BleService) {
        // Show warning but don't block - allow UI interaction
        Alert.alert(
          'Device Not Connected',
          'Please connect to device to send commands. Controls are available but commands will not be sent.',
          [{ text: 'OK' }]
        );
        return false;
      }

      let commandString = command;
      if (value !== null) {
        commandString = `${command}:${value}`;
      }

      await BleService.sendCommand(commandString);
      return true;
    } catch (error) {
      Alert.alert('Error', `Cannot send command: ${command}`);
      return false;
    }
  };

  // === HANDLERS FOR CONTROL FUNCTIONS ===

  /**
   * Switch to AUTO mode
   */
  const handleAutoMode = async () => {
    const success = await BleService.enableAutoMode();
    if (success) {
      setControlSettings(prev => ({ ...prev, mode: 'auto' }));
      // Sync with Redux state
      dispatch({
        type: 'ble/setAutoMode',
        payload: true
      });
    }
  };

  /**
   * Switch to MANUAL mode
   */
  const handleManualMode = async () => {
    const success = await BleService.disableAutoMode();
    if (success) {
      // Reset các nút KNEADING, COMBINE, PERCUSSION, COMPRESSION về trạng thái ban đầu
      setControlSettings(prev => ({ 
        ...prev, 
        mode: 'manual',
        kneading: false,
        combine: false,
        percussion: false,
        compression: false
      }));
      // Sync with Redux state
      dispatch({
        type: 'ble/setAutoMode',
        payload: false
      });
    }
  };

  /**
   * Switch to ROLL mode (rolling) - only works in AUTO mode
   */
  const handleRollMode = async () => {
    console.log('handleRollMode() called');
    console.log('handleRollMode() - systemState:', systemState);
    console.log('handleRollMode() - isAutoMode:', systemState?.isAutoMode);
    
    // Check if in AUTO mode
    if (!systemState?.isAutoMode) {
      console.log('❌ ROLL mode blocked - not in AUTO mode');
      Alert.alert(
        'Chế độ AUTO cần thiết',
        'Chế độ ROLL chỉ hoạt động khi ở chế độ AUTO.\n\nVui lòng:\n1. Bật AUTO mode trước\n2. Sau đó thử ROLL mode',
        [
          { 
            text: 'Bật AUTO Mode', 
            onPress: () => handleAutoMode() 
          },
          { text: 'Hủy', style: 'cancel' }
        ]
      );
      return;
    }
    
    console.log('✅ ROLL mode allowed - in AUTO mode');
    const success = await BleService.enableRoll();
    if (success) {
      setControlSettings(prev => ({ ...prev, rollSpot: 'roll' }));
    }
  };

  /**
   * Switch to SPOT mode (point) - only works in AUTO mode
   */
  const handleSpotMode = async () => {
    console.log('handleSpotMode() called');
    console.log('handleSpotMode() - systemState:', systemState);
    console.log('handleSpotMode() - isAutoMode:', systemState?.isAutoMode);
    
    // Check if in AUTO mode
    if (!systemState?.isAutoMode) {
      console.log('❌ SPOT mode blocked - not in AUTO mode');
      Alert.alert(
        'Chế độ AUTO cần thiết',
        'Chế độ SPOT chỉ hoạt động khi ở chế độ AUTO.\n\nVui lòng:\n1. Bật AUTO mode trước\n2. Sau đó thử SPOT mode',
        [
          { 
            text: 'Bật AUTO Mode', 
            onPress: () => handleAutoMode() 
          },
          { text: 'Hủy', style: 'cancel' }
        ]
      );
      return;
    }
    
    console.log('✅ SPOT mode allowed - in AUTO mode');
    const success = await BleService.enableSpot();
    if (success) {
      setControlSettings(prev => ({ ...prev, rollSpot: 'spot' }));
    }
  };

  /**
   * Switch to HIGH intensity (chỉ hoạt động với PERCUSSION, COMPRESSION, COMBINE)
   */
  const handleIntensityHigh = async () => {
    // Kiểm tra massage mode hiện tại
    if (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode) {
      Alert.alert(
        'Chế độ massage không hỗ trợ',
        'Cường độ chỉ điều khiển được khi ở chế độ PERCUSSION, COMPRESSION, hoặc COMBINE.',
        [{ text: 'OK' }]
      );
      return;
    }
    
    const success = await BleService.setIntensity(2);
    if (success) {
      setControlSettings(prev => ({ ...prev, intensity: 'HIGH' }));
    }
  };

  /**
   * Switch to LOW intensity (chỉ hoạt động với PERCUSSION, COMPRESSION, COMBINE)
   */
  const handleIntensityLow = async () => {
    // Kiểm tra massage mode hiện tại
    if (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode) {
      Alert.alert(
        'Chế độ massage không hỗ trợ',
        'Cường độ chỉ điều khiển được khi ở chế độ PERCUSSION, COMPRESSION, hoặc COMBINE.',
        [{ text: 'OK' }]
      );
      return;
    }
    
    const success = await BleService.setIntensity(1);
    if (success) {
      setControlSettings(prev => ({ ...prev, intensity: 'LOW' }));
    }
  };

  /**
   * Select kneading technique (Kneading) - only one technique can be selected at a time
   * Only works in AUTO mode
   */
  const handleKneading = async () => {
    // Check if in AUTO mode
    if (!systemState?.isAutoMode) {
      Alert.alert(
        'Chế độ AUTO cần thiết',
        'Chế độ Kneading chỉ hoạt động khi ở chế độ AUTO. Vui lòng bật AUTO mode trước.',
        [{ text: 'OK' }]
      );
      return;
    }
    
    // If on then turn off, if off then turn on and turn off other techniques
    const newValue = !controlSettings.kneading;
    
    if (newValue) {
      // Turn on Kneading and turn off other techniques
      const success = await BleService.setMassageMode('KNEADING');
      if (success) {
        setControlSettings(prev => ({ 
          ...prev, 
          kneading: true,
          combine: false,
          percussion: false,
          compression: false
        }));
      }
    } else {
      // Turn off Kneading
      const success = await BleService.setMassageMode('DEFAULT');
      if (success) {
        setControlSettings(prev => ({ ...prev, kneading: false }));
      }
    }
  };

  /**
   * Select combine technique (Combine) - only one technique can be selected at a time
   * Only works in AUTO mode
   */
  const handleCombine = async () => {
    // Check if in AUTO mode
    if (!systemState?.isAutoMode) {
      Alert.alert(
        'Chế độ AUTO cần thiết',
        'Chế độ Combine chỉ hoạt động khi ở chế độ AUTO. Vui lòng bật AUTO mode trước.',
        [{ text: 'OK' }]
      );
      return;
    }
    
    // If on then turn off, if off then turn on and turn off other techniques
    const newValue = !controlSettings.combine;
    
    if (newValue) {
      // Turn on Combine and turn off other techniques
      const success = await BleService.setMassageMode('COMBINE');
      if (success) {
        setControlSettings(prev => ({ 
          ...prev, 
          kneading: false,
          combine: true,
          percussion: false,
          compression: false
        }));
      }
    } else {
      // Turn off Combine
      const success = await BleService.setMassageMode('DEFAULT');
      if (success) {
        setControlSettings(prev => ({ ...prev, combine: false }));
      }
    }
  };

  /**
   * Select percussion technique (Percussion) - only one technique can be selected at a time
   * Only works in AUTO mode
   */
  const handlePercussion = async () => {
    // Check if in AUTO mode
    if (!systemState?.isAutoMode) {
      Alert.alert(
        'Chế độ AUTO cần thiết',
        'Chế độ Percussion chỉ hoạt động khi ở chế độ AUTO. Vui lòng bật AUTO mode trước.',
        [{ text: 'OK' }]
      );
      return;
    }
    
    // If on then turn off, if off then turn on and turn off other techniques
    const newValue = !controlSettings.percussion;
    
    if (newValue) {
      // Turn on Percussion and turn off other techniques
      const success = await BleService.setMassageMode('PERCUSSION');
      if (success) {
        setControlSettings(prev => ({ 
          ...prev, 
          kneading: false,
          combine: false,
          percussion: true,
          compression: false
        }));
      }
    } else {
      // Turn off Percussion
      const success = await BleService.setMassageMode('DEFAULT');
      if (success) {
        setControlSettings(prev => ({ ...prev, percussion: false }));
      }
    }
  };

  /**
   * Select compression technique (Compression) - only one technique can be selected at a time
   * Only works in AUTO mode
   */
  const handleCompression = async () => {
    // Check if in AUTO mode
    if (!systemState?.isAutoMode) {
      Alert.alert(
        'Chế độ AUTO cần thiết',
        'Chế độ Compression chỉ hoạt động khi ở chế độ AUTO. Vui lòng bật AUTO mode trước.',
        [{ text: 'OK' }]
      );
      return;
    }
    
    // If on then turn off, if off then turn on and turn off other techniques
    const newValue = !controlSettings.compression;
    
    if (newValue) {
      // Turn on Compression and turn off other techniques
      const success = await BleService.setMassageMode('COMPRESSION');
      if (success) {
        setControlSettings(prev => ({ 
          ...prev, 
          kneading: false,
          combine: false,
          percussion: false,
          compression: true
        }));
      }
    } else {
      // Turn off Compression
      const success = await BleService.setMassageMode('DEFAULT');
      if (success) {
        setControlSettings(prev => ({ ...prev, compression: false }));
      }
    }
  };

  /**
   * Increase recline angle - Replaced by ChairPositionControl
   */
  // const handleReclineIncrease = async () => {
  //   const newValue = Math.min(controlSettings.recline + 10, 100);
  //   const success = await sendCommand('RECLINE', newValue);
  //   if (success) {
  //     setControlSettings(prev => ({ ...prev, recline: newValue }));
  //   }
  // };

  /**
   * Decrease recline angle - Replaced by ChairPositionControl
   */
  // const handleReclineDecrease = async () => {
  //   const newValue = Math.max(controlSettings.recline - 10, 0);
  //   const success = await sendCommand('RECLINE', newValue);
  //   if (success) {
  //     setControlSettings(prev => ({ ...prev, recline: newValue }));
  //   }
  // };

  /**
   * Increase incline angle - Replaced by ChairPositionControl
   */
  // const handleInclineIncrease = async () => {
  //   const newValue = Math.min(controlSettings.incline + 10, 100);
  //   const success = await sendCommand('INCLINE', newValue);
  //   if (success) {
  //     setControlSettings(prev => ({ ...prev, incline: newValue }));
  //   }
  // };

  /**
   * Decrease incline angle - Replaced by ChairPositionControl
   */
  // const handleInclineDecrease = async () => {
  //   const newValue = Math.max(controlSettings.incline - 10, 0);
  //   const success = await sendCommand('INCLINE', newValue);
  //   if (success) {
  //     setControlSettings(prev => ({ ...prev, incline: newValue }));
  //   }
  // };

  



  

  /**
   * Disconnect device
   */
  const handleDisconnect = async () => {
    Alert.alert(
      'Disconnect',
      'Are you sure you want to disconnect from the device?',
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Disconnect',
          style: 'destructive',
          onPress: async () => {
            try {
              // Disconnect from BLE
              if (BleService && isConnected) {
                await BleService.disconnect();
              }
              dispatch(resetBleState());
              dispatch(navigateTo('Home'));
            } catch (error) {
              dispatch(resetBleState());
              dispatch(navigateTo('Home'));
            }
          }
        }
      ]
    );
  };

  

  // Check real connection (for display purposes only, not blocking)
  const isReallyConnected = isConnected &&
    connectionStatus === 'connected' &&
    (deviceInfo || connectedDevice);

  // Render loading state
  if (isLoading) {
    return (
      <SafeAreaView style={styles.safeArea}>
        <View style={[styles.container, styles.centerContent]}>
          <ActivityIndicator size="large" color={COLORS.PRIMARY} />
          <Text style={styles.loadingText}>Loading...</Text>
        </View>
      </SafeAreaView>
    );
  }

  // Main control interface (always accessible, even without connection)
  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.container}>
      <ScrollView showsVerticalScrollIndicator={false}>
        {/* Header */}
        <View style={styles.header}>
          <View style={styles.headerTop}>
            <TouchableOpacity
              style={styles.backButton}
              onPress={() => dispatch(navigateTo('Home'))}
            >
              <Text style={styles.backButtonText}>← Home</Text>
            </TouchableOpacity>
            <TouchableOpacity
              style={styles.disconnectButton}
              onPress={handleDisconnect}
            >
                              <Text style={styles.disconnectButtonText}>Disconnect</Text>
            </TouchableOpacity>
          </View>
          <Text style={styles.headerTitle}>Massage Control</Text>
          <View style={styles.deviceStatusRow}>
            <Text style={styles.deviceName}>
              {deviceInfo?.name || connectedDevice?.name || 'No Device'}
            </Text>
            <View style={styles.connectionIndicator}>
              <View style={[
                styles.connectionDot,
                !isReallyConnected && styles.connectionDotDisconnected
              ]} />
              <Text style={[
                styles.connectionText,
                !isReallyConnected && styles.connectionTextDisconnected
              ]}>
                {isReallyConnected ? 'Connected' : 'Not Connected'}
              </Text>
            </View>
          </View>
        </View>


        

        {/* Combined Control sections */}
        <View style={styles.section}>
          {/* 1. AUTO/MANUAL Mode */}
          <View style={styles.controlSubsection}>
            <Text style={styles.subsectionTitle}>🤖 Control Mode</Text>
            <View style={styles.buttonRow}>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.mode === 'auto' && styles.modeButtonActive
                ]}
                onPress={handleAutoMode}
              >
                <Text style={[
                  styles.buttonText,
                  controlSettings.mode === 'auto' && styles.buttonTextActive
                ]}>AUTO</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.mode === 'manual' && styles.modeButtonActive
                ]}
                onPress={handleManualMode}
              >
                <Text style={[
                  styles.buttonText,
                  controlSettings.mode === 'manual' && styles.buttonTextActive
                ]}>MANUAL</Text>
              </TouchableOpacity>
            </View>
            
            {/* ⏱️ Timer Display - Chỉ hiển thị khi ở AUTO mode */}
            {controlSettings.mode === 'auto' && autoTimerActive && (
              <View style={[
                styles.timerDisplay,
                autoTimer < 60 && styles.timerDisplayWarning,      // < 1 phút: Warning
                autoTimer < 30 && styles.timerDisplayCritical      // < 30 giây: Critical
              ]}>
                <Text style={styles.timerIcon}>
                  {autoTimer < 60 ? '⚠️' : '⏱️'}
                </Text>
                <Text style={[
                  styles.timerText,
                  autoTimer < 60 && styles.timerTextWarning,
                  autoTimer < 30 && styles.timerTextCritical
                ]}>
                  {Math.floor(autoTimer / 60)}:{(autoTimer % 60).toString().padStart(2, '0')}
                </Text>
                <Text style={[
                  styles.timerLabel,
                  autoTimer < 60 && styles.timerLabelWarning
                ]}>
                  {autoTimer < 60 ? 'finishing soon' : 'remaining'}
                </Text>
              </View>
            )}
          </View>

          {/* 2. ROLL/SPOT Type */}
          <View style={styles.controlSubsection}>
            <Text style={styles.subsectionTitle}>🎯 Massage Type</Text>
            <View style={styles.buttonRow}>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.rollSpot === 'roll' && styles.modeButtonActive,
                  !systemState?.isAutoMode && styles.disabledButton
                ]}
                onPress={handleRollMode}
                disabled={!systemState?.isAutoMode}
              >
                <Text style={[
                  styles.buttonText,
                  controlSettings.rollSpot === 'roll' && styles.buttonTextActive,
                  !systemState?.isAutoMode && styles.disabledButtonText
                ]}>ROLL</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.rollSpot === 'spot' && styles.modeButtonActive,
                  !systemState?.isAutoMode && styles.disabledButton
                ]}
                onPress={handleSpotMode}
                disabled={!systemState?.isAutoMode}
              >
                <Text style={[
                  styles.buttonText,
                  controlSettings.rollSpot === 'spot' && styles.buttonTextActive,
                  !systemState?.isAutoMode && styles.disabledButtonText
                ]}>SPOT</Text>
              </TouchableOpacity>
            </View>
          </View>

          {/* 3. Intensity Control */}
          <View style={styles.controlSubsection}>
            <Text style={styles.subsectionTitle}>⚡ Massage Intensity</Text>
            
            {/* Intensity control buttons */}
            <View style={styles.buttonRow}>
              <TouchableOpacity
                style={[
                  styles.intensityButton,
                  controlSettings.intensity === 'LOW' && styles.intensityButtonActive,
                  (!systemState?.isAutoMode || (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode)) && styles.disabledButton
                ]}
                onPress={handleIntensityLow}
                disabled={!systemState?.isAutoMode || (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode)}
              >
                <Text style={[
                  styles.intensityButtonText,
                  controlSettings.intensity === 'LOW' && styles.intensityButtonTextActive,
                  (!systemState?.isAutoMode || (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode)) && styles.disabledButtonText
                ]}>LOW</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={[
                  styles.intensityButton,
                  controlSettings.intensity === 'HIGH' && styles.intensityButtonActive,
                  (!systemState?.isAutoMode || (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode)) && styles.disabledButton
                ]}
                onPress={handleIntensityHigh}
                disabled={!systemState?.isAutoMode || (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode)}
              >
                <Text style={[
                  styles.intensityButtonText,
                  controlSettings.intensity === 'HIGH' && styles.intensityButtonTextActive,
                  (!systemState?.isAutoMode || (!systemState?.isPercussionMode && !systemState?.isCompressionMode && !systemState?.isCombineMode)) && styles.disabledButtonText
                ]}>HIGH</Text>
              </TouchableOpacity>
            </View>
          </View>

          {/* 4. Massage Techniques */}
          <View style={styles.controlSubsection}>
            <Text style={styles.subsectionTitle}>🎨 Massage Techniques</Text>
            <View style={styles.techniqueGrid}>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.kneading && styles.techniqueButtonActive,
                  !systemState?.isAutoMode && styles.disabledButton
                ]}
                onPress={handleKneading}
                disabled={!systemState?.isAutoMode}
              >
                <Text style={[
                  styles.techniqueText,
                  controlSettings.kneading && styles.techniqueTextActive,
                  !systemState?.isAutoMode && styles.disabledButtonText
                ]}>KNEADING</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.combine && styles.techniqueButtonActive,
                  !systemState?.isAutoMode && styles.disabledButton
                ]}
                onPress={handleCombine}
                disabled={!systemState?.isAutoMode}
              >
                <Text style={[
                  styles.techniqueText,
                  controlSettings.combine && styles.techniqueTextActive,
                  !systemState?.isAutoMode && styles.disabledButtonText
                ]}>COMBINE</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.percussion && styles.techniqueButtonActive,
                  !systemState?.isAutoMode && styles.disabledButton
                ]}
                onPress={handlePercussion}
                disabled={!systemState?.isAutoMode}
              >
                <Text style={[
                  styles.techniqueText,
                  controlSettings.percussion && styles.techniqueTextActive,
                  !systemState?.isAutoMode && styles.disabledButtonText
                ]}>PERCUSSION</Text>
              </TouchableOpacity>
              <TouchableOpacity
                style={[
                  styles.techniqueButton,
                  controlSettings.compression && styles.techniqueButtonActive,
                  !systemState?.isAutoMode && styles.disabledButton
                ]}
                onPress={handleCompression}
                disabled={!systemState?.isAutoMode}
              >
                <Text style={[
                  styles.techniqueText,
                  controlSettings.compression && styles.techniqueTextActive,
                  !systemState?.isAutoMode && styles.disabledButtonText
                ]}>COMPRESSION</Text>
              </TouchableOpacity>
            </View>
          </View>

          {/* 5. Chair Position Control */}
          <View style={styles.controlSubsection}>
            <Text style={styles.subsectionTitle}>🪑 Chair Position Control</Text>
            <ChairPositionControl />
          </View>
        </View>

        

        {/* Bottom spacing */}
        <View style={styles.bottomSpacing} />
      </ScrollView>
    </View>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
  container: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
  centerContent: {
    justifyContent: 'center',
    alignItems: 'center',
  },
  loadingText: {
    marginTop: 16,
    fontSize: 16,
    color: COLORS.TEXT_SECONDARY,
  },
  notConnectedContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: COLORS.BACKGROUND,
    padding: 30,
  },
  notConnectedIcon: {
    fontSize: 80,
    marginBottom: 20,
  },
  notConnectedTitle: {
    fontSize: 24,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 16,
    textAlign: 'center',
  },
  notConnectedText: {
    fontSize: 16,
    color: COLORS.TEXT_SECONDARY,
    textAlign: 'center',
    marginBottom: 40,
    lineHeight: 24,
  },
  connectButton: {
    backgroundColor: COLORS.PRIMARY,
    paddingHorizontal: 40,
    paddingVertical: 15,
    borderRadius: 25,
    marginBottom: 15,
  },
  connectButtonText: {
    color: COLORS.WHITE,
    fontSize: 16,
    fontWeight: 'bold',
  },
  manualConnectButton: {
    backgroundColor: 'transparent',
    paddingHorizontal: 30,
    paddingVertical: 12,
    borderRadius: 20,
    borderWidth: 1,
    borderColor: COLORS.PRIMARY,
  },
  manualConnectButtonText: {
    color: COLORS.PRIMARY,
    fontSize: 14,
    fontWeight: 'bold',
  },
  header: {
    backgroundColor: COLORS.WHITE,
    padding: 20,
    paddingTop: Platform.OS === 'ios' ? 50 : 20,
    borderBottomWidth: 1,
    borderBottomColor: COLORS.GRAY_LIGHT,
  },
  headerTop: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 10,
  },
  backButton: {
    paddingVertical: 8,
    paddingHorizontal: 12,
    backgroundColor: COLORS.GRAY_LIGHT,
    borderRadius: 8,
  },
  backButtonText: {
    color: COLORS.TEXT_PRIMARY,
    fontSize: 14,
    fontWeight: '500',
  },
  disconnectButton: {
    paddingVertical: 8,
    paddingHorizontal: 12,
    backgroundColor: COLORS.DANGER,
    borderRadius: 8,
  },
  disconnectButtonText: {
    color: COLORS.WHITE,
    fontSize: 14,
    fontWeight: '500',
  },
  headerTitle: {
    fontSize: 24,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    textAlign: 'center',
    marginBottom: 5,
  },
  deviceStatusRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 5,
  },
  deviceName: {
    fontSize: 16,
    color: COLORS.TEXT_SECONDARY,
    fontWeight: '500',
    flex: 1,
  },
  connectionIndicator: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  connectionDot: {
    width: 8,
    height: 8,
    borderRadius: 4,
    backgroundColor: COLORS.SUCCESS,
    marginRight: 8,
  },
  connectionDotDisconnected: {
    backgroundColor: COLORS.WARNING || '#FFA500',
  },
  connectionText: {
    fontSize: 14,
    color: COLORS.SUCCESS,
    fontWeight: 'bold',
  },
  connectionTextDisconnected: {
    color: COLORS.WARNING || '#FFA500',
  },
  
  section: {
    backgroundColor: COLORS.WHITE,
    marginHorizontal: 20,
    marginTop: -10,
    marginBottom: 20,
    padding: 20,
    borderRadius: 15,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 15,
  },
  controlSubsection: {
    marginBottom: 2,
  },
  subsectionTitle: {
    fontSize: 14,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 5,
  },
  buttonRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 6,
  },
  buttonIcon: {
    fontSize: 24,
    marginBottom: 8,
  },
  buttonText: {
    fontSize: 12,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
  },
  buttonTextActive: {
    color: COLORS.WHITE,
  },
  modeButtonActive: {
    backgroundColor: COLORS.PRIMARY,
    borderColor: COLORS.PRIMARY,
  },
  intensityButton: {
    backgroundColor: COLORS.WHITE,
    paddingVertical: 8,
    paddingHorizontal: 16,
    borderRadius: 8,
    flex: 1,
    marginHorizontal: 5,
    alignItems: 'center',
    borderWidth: 2,
    borderColor: COLORS.GRAY_MEDIUM,
  },
  intensityButtonText: {
    color: COLORS.TEXT_PRIMARY,
    fontSize: 12,
    fontWeight: 'bold',
  },
  intensityButtonActive: {
    backgroundColor: COLORS.PRIMARY,
    borderColor: COLORS.PRIMARY,
  },
  intensityButtonTextActive: {
    color: COLORS.WHITE,
  },
  intensityDisplay: {
    backgroundColor: COLORS.GRAY_LIGHT,
    paddingVertical: 12,
    paddingHorizontal: 20,
    borderRadius: 8,
    alignItems: 'center',
    justifyContent: 'center',
    marginHorizontal: 10,
  },
  intensityValue: {
    fontSize: 24,
    fontWeight: 'bold',
    color: COLORS.PRIMARY,
  },
  techniqueGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'space-between',
  },
  techniqueButton: {
    alignItems: 'center',
    padding: 8,
    borderRadius: 8,
    borderWidth: 2,
    borderColor: COLORS.GRAY_MEDIUM,
    backgroundColor: COLORS.WHITE,
    width: '48%',
    marginBottom: 6,
  },
  techniqueButtonActive: {
    backgroundColor: COLORS.SUCCESS,
    borderColor: COLORS.SUCCESS,
  },
  techniqueText: {
    fontSize: 12,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    textAlign: 'center',
  },
  techniqueTextActive: {
    color: COLORS.WHITE,
  },
  positionControl: {
    marginBottom: 20,
  },
  positionLabel: {
    fontSize: 16,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 10,
    textAlign: 'center',
  },
  positionButton: {
    backgroundColor: COLORS.INFO,
    paddingVertical: 12,
    paddingHorizontal: 20,
    borderRadius: 8,
    flex: 1,
    marginHorizontal: 5,
    alignItems: 'center',
  },
  positionButtonText: {
    color: COLORS.WHITE,
    fontSize: 14,
    fontWeight: 'bold',
  },
  
  
  
  bottomSpacing: {
    height: 20,
  },
  statusRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 10,
  },
  statusLabel: {
    fontSize: 16,
    color: COLORS.TEXT_SECONDARY,
    fontWeight: 'bold',
  },
  statusValue: {
    fontSize: 16,
    fontWeight: 'bold',
  },
  
  // Disabled button styles
  disabledButton: {
    backgroundColor: COLORS.GRAY_LIGHT,
    borderColor: COLORS.GRAY_MEDIUM,
    opacity: 0.6,
  },
  disabledButtonText: {
    color: COLORS.TEXT_SECONDARY,
  },
  
  // Timer display styles
  timerDisplay: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: COLORS.PRIMARY + '10', // Semi-transparent primary color
    paddingVertical: 12,
    paddingHorizontal: 20,
    borderRadius: 12,
    marginTop: 12,
    borderWidth: 1,
    borderColor: COLORS.PRIMARY + '30',
  },
  timerDisplayWarning: {
    backgroundColor: '#FFA50020', // Orange warning
    borderColor: '#FFA500',
    borderWidth: 2,
  },
  timerDisplayCritical: {
    backgroundColor: '#FF000020', // Red critical
    borderColor: '#FF0000',
    borderWidth: 2,
  },
  timerIcon: {
    fontSize: 24,
    marginRight: 8,
  },
  timerText: {
    fontSize: 28,
    fontWeight: 'bold',
    color: COLORS.PRIMARY,
    fontFamily: Platform.OS === 'ios' ? 'Menlo' : 'monospace',
    marginRight: 8,
  },
  timerTextWarning: {
    color: '#FFA500', // Orange
  },
  timerTextCritical: {
    color: '#FF0000', // Red
  },
  timerLabel: {
    fontSize: 14,
    color: COLORS.TEXT_SECONDARY,
    fontWeight: '500',
  },
  timerLabelWarning: {
    color: '#FFA500',
    fontWeight: 'bold',
  },
});

export default ControlScreen;
