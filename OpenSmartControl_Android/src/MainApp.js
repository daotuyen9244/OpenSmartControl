/**
 * MainApp.js
 * Main component of the massage chair control application
 * 
 * Features:
 * - Manage navigation between screens
 * - Initialize BLE service
 * - Handle hardware back button
 * - Render current screen
 * - Display navigation bar
 */

import React, { useEffect } from 'react';
import { View, StyleSheet, StatusBar, BackHandler, Alert, Platform } from 'react-native';
import { useSelector, useDispatch } from 'react-redux';
import { navigateTo } from './store/navigationSlice';
import NavigationBar from './components/NavigationBar';
import HomeScreen from './components/HomeScreen';
import QRScanner from './components/QRScanner';
import ManualConnect from './components/ManualConnect';
import ControlScreen from './components/ControlScreen';
import COLORS from './styles/Colors';
import BleService from './services/BleService';

/**
 * MainApp Component
 * Root component that manages the entire application
 */
const MainApp = () => {
  const dispatch = useDispatch();
  
  // Get state from Redux store
  const { currentScreen } = useSelector(state => state.navigation);
  const { isConnected } = useSelector(state => state.ble);
  const systemState = useSelector(state => state.ble.systemState);
  
  // Create getState function for BleService
  const getState = () => {
    console.log('MainApp getState() called');
    console.log('MainApp getState() - systemState:', systemState);
    // Always return fresh state from Redux store
    const currentState = { ble: { systemState } };
    console.log('MainApp getState() - returning:', currentState);
    return currentState;
  };

  /**
   * Effect: Initialize application and setup event listeners
   */
  useEffect(() => {
    initializeApp();

    // Setup hardware back button handler
    const backHandler = BackHandler.addEventListener(
      'hardwareBackPress', 
      handleBackPress
    );

    // Cleanup function
    return () => {
      backHandler.remove();
    };
  }, [currentScreen, isConnected, systemState]); // Dependencies so handleBackPress can access latest state

  /**
   * Initialize application
   * - Setup BLE service
   * - Configure dispatch for BLE service
   */
  const initializeApp = async () => {
    try {
      // Set Redux dispatch function for BleService
      // Allow BleService to dispatch actions to store
      BleService.setDispatch(dispatch);
      BleService.setGetState(getState);
      
      // Initialize BLE service
      await BleService.initialize();
      console.log('BLE Service initialized successfully');
    } catch (error) {
      console.error('App initialization error:', error);
      Alert.alert(
        'Initialization Error', 
        'Unable to initialize Bluetooth service'
      );
    }
  };

  /**
   * Handle hardware back button press
   * - Exit app if on Home screen
   * - Confirm before leaving Control screen when connected
   * - Navigate to Home for other screens
   * @returns {boolean} - True to prevent default behavior
   */
  const handleBackPress = () => {
    if (currentScreen === 'Home') {
      // On Home screen - ask for exit confirmation
      Alert.alert(
        'Exit Application',
        'Do you want to exit the application?',
        [
          { text: 'Cancel', style: 'cancel' },
          { text: 'Exit', onPress: () => BackHandler.exitApp() }
        ]
      );
      return true; // Prevent default back behavior
    } else {
      // If on Control screen and connected, ask for confirmation
      if (currentScreen === 'Control' && isConnected) {
        Alert.alert(
          'Return to Home',
          'Do you want to return to home? Device connection will be maintained.',
          [
            { text: 'Cancel', style: 'cancel' },
            { 
              text: 'Return', 
              onPress: () => dispatch(navigateTo('Home')) 
            }
          ]
        );
        return true;
      } else {
        // Other screens - return to Home
        dispatch(navigateTo('Home'));
        return true;
      }
    }
  };

  /**
   * Render current screen based on currentScreen state
   * @returns {JSX.Element} - Component of current screen
   */
  const renderScreen = () => {
    console.log('Rendering screen:', currentScreen);
    
    switch (currentScreen) {
      case 'QRScanner':
        return <QRScanner />;
      
      case 'ManualConnect':
        return <ManualConnect />;
      
      case 'Control':
        return <ControlScreen />;
      
      default:
        return <HomeScreen />;
    }
  };

  /**
   * Main render
   */
  return (
    <View style={styles.container}>
      {/* Status bar: Android dùng trong suốt cho tràn viền (edge-to-edge), iOS giữ nền trắng */}
      <StatusBar
        barStyle="dark-content"
        backgroundColor={Platform.OS === 'android' ? 'transparent' : COLORS.WHITE}
      />
      
      {/* Render current screen */}
      {renderScreen()}
      
      {/* Navigation bar at bottom */}
      <NavigationBar />
    </View>
  );
};

/**
 * Styles for MainApp component
 */
const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
});

export default MainApp;
