/**
 * ManualConnect.js
 * Manual connection screen with Bluetooth device
 * 
 * Features:
 * - Scan and display list of Bluetooth devices
 * - Connect to selected device
 * - Display detailed device information (RSSI, MAC address)
 * - Refresh device list
 * - Handle connection errors
 */

import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  FlatList,
  TouchableOpacity,
  Alert,
  ActivityIndicator,
  RefreshControl,
  Linking,
  SafeAreaView,
} from 'react-native';
import { useDispatch, useSelector } from 'react-redux';
import { navigateTo } from '../store/navigationSlice';
import {
  setScanResults,
  setScanning,
  setConnectedDevice,
  setConnected,
  setConnecting,
  addScanResult
} from '../store/bleSlice';
import BleService from '../services/BleService';
import { COLORS } from '../styles/Colors';

const ManualConnect = () => {
  const dispatch = useDispatch();
  const { scanResults = [], isScanning, isConnecting } = useSelector(state => state.ble);

  // State for refresh control
  const [refreshing, setRefreshing] = useState(false);

  /**
   * Effect: Initialize BLE and start scanning when component mounts
   */
  useEffect(() => {
    initializeBLE();
    return () => {
      BleService.stopScan();
    };
  }, []);

  /**
   * Initialize BLE service and start device scanning
   */
  const initializeBLE = async () => {
    try {
      const initialized = await BleService.initialize();
      if (initialized) {
        startScan();
      } else {
        Alert.alert('Error', 'Unable to initialize Bluetooth');
      }
    } catch (error) {
      Alert.alert('Error', 'Unable to initialize Bluetooth');
    }
  };

  /**
   * Start scanning Bluetooth devices
   */
  const startScan = async () => {
    try {
      dispatch(setScanning(true));
      dispatch(setScanResults([]));
      
      await BleService.scanDevices();
      
      // Get list of discovered devices after 20 seconds
      setTimeout(async () => {
        const peripherals = await BleService.getDiscoveredPeripherals();
        dispatch(setScanResults(peripherals));
        dispatch(setScanning(false));
      }, 20000); // Scan for 20 seconds (increased from 10s)
    } catch (error) {
      dispatch(setScanning(false));
      Alert.alert('Error', 'Unable to scan Bluetooth devices');
    }
  };

  /**
   * Stop scanning devices
   */
  const stopScan = async () => {
    try {
      await BleService.stopScan();
      dispatch(setScanning(false));
    } catch (error) {
      // Bỏ qua lỗi khi dừng quét
    }
  };

  /**
   * Connect to selected device
   * @param {Object} device - Device information
   */
  const connectToDevice = async (device) => {
    try {
      dispatch(setConnecting(true));
      const connected = await BleService.connectToDevice(device.id);
      
      if (connected) {
        dispatch(setConnectedDevice(device));
        dispatch(setConnected(true));
        dispatch(navigateTo('Control'));
      } else {
        throw new Error('Connection failed');
      }
    } catch (error) {
      Alert.alert(
        'Connection Error',
        `Cannot connect to ${device.name || device.id}`,
        [
          {
            text: 'Retry',
            onPress: () => connectToDevice(device)
          },
          {
            text: 'Cancel',
            style: 'cancel'
          }
        ]
      );
    } finally {
      dispatch(setConnecting(false));
    }
  };

  /**
   * Xử lý refresh danh sách
   */
  const onRefresh = async () => {
    setRefreshing(true);
    await startScan();
    setRefreshing(false);
  };

  /**
   * Render device item in list
   */
  const renderDevice = ({ item }) => (
    <TouchableOpacity
      style={styles.deviceItem}
      onPress={() => connectToDevice(item)}
      disabled={isConnecting}
    >
      <View style={styles.deviceInfo}>
        <Text style={styles.deviceName}>
          {item.name || 'Unnamed Device'}
        </Text>
        <Text style={styles.deviceId}>{item.id}</Text>
        <View style={styles.deviceDetails}>
          <Text style={styles.rssi}>RSSI: {item.rssi || 'N/A'}</Text>
          {item.advertising && item.advertising.localName && (
            <Text style={styles.localName}>
              Local: {item.advertising.localName}
            </Text>
          )}
        </View>
      </View>
      <View style={styles.deviceActions}>
        <View style={[
          styles.signalStrength,
          { backgroundColor: getSignalColor(item.rssi) }
        ]}>
          <Text style={styles.signalText}>{getSignalStrength(item.rssi)}</Text>
        </View>
        <Text style={styles.connectText}>Connect</Text>
      </View>
    </TouchableOpacity>
  );

  /**
   * Lấy biểu tượng cường độ sóng
   * @param {number} rssi - Giá trị RSSI
   * @returns {string} Biểu tượng cường độ sóng
   */
  const getSignalStrength = (rssi) => {
    if (!rssi) return '?';
    if (rssi > -50) return '●●●';
    if (rssi > -70) return '●●○';
    if (rssi > -90) return '●○○';
    return '○○○';
  };

  /**
   * Lấy màu theo cường độ sóng
   * @param {number} rssi - Giá trị RSSI
   * @returns {string} Mã màu
   */
  const getSignalColor = (rssi) => {
    if (!rssi) return COLORS.GRAY_MEDIUM;
    if (rssi > -50) return COLORS.SUCCESS;
    if (rssi > -70) return COLORS.WARNING;
    return COLORS.DANGER;
  };

  /**
   * Render khi danh sách trống
   */
  const renderEmptyList = () => (
    <View style={styles.emptyContainer}>
      <Text style={styles.emptyIcon}>📡</Text>
      <Text style={styles.emptyTitle}>No devices found</Text>
      <Text style={styles.emptyText}>
        Make sure Bluetooth device is turned on and nearby
      </Text>
      <TouchableOpacity style={styles.retryButton} onPress={startScan}>
        <Text style={styles.retryButtonText}>Scan Again</Text>
      </TouchableOpacity>
    </View>
  );

  return (
    <SafeAreaView style={styles.safeArea}>
      <View style={styles.container}>
        {/* Header */}
        <View style={styles.header}>
        <Text style={styles.title}>Manual Connect</Text>
        <Text style={styles.subtitle}>Find and connect to Bluetooth device</Text>
      </View>

      {/* Thanh điều khiển */}
      <View style={styles.controlBar}>
        <TouchableOpacity
          style={[
            styles.scanButton,
            isScanning && styles.scanButtonActive
          ]}
          onPress={isScanning ? stopScan : startScan}
        >
          {isScanning && <ActivityIndicator size="small" color={COLORS.WHITE} />}
          <Text style={styles.scanButtonText}>
            {isScanning ? ' Scanning...' : 'Scan Devices'}
          </Text>
        </TouchableOpacity>
        <Text style={styles.deviceCount}>
          Found: {scanResults.length} devices
        </Text>
      </View>

      {/* Device list */}
      <FlatList
        data={scanResults}
        renderItem={renderDevice}
        keyExtractor={item => item.id}
        style={styles.deviceList}
        refreshControl={
          <RefreshControl
            refreshing={refreshing}
            onRefresh={onRefresh}
            colors={[COLORS.PRIMARY]}
          />
        }
        ListEmptyComponent={!isScanning ? renderEmptyList : null}
        showsVerticalScrollIndicator={false}
      />

      {/* Overlay when connecting */}
      {isConnecting && (
        <View style={styles.connectingOverlay}>
          <View style={styles.connectingModal}>
            <ActivityIndicator size="large" color={COLORS.PRIMARY} />
            <Text style={styles.connectingText}>Connecting...</Text>
          </View>
        </View>
      )}
    </View>
    </SafeAreaView>
  );
};

// Styles cho component
const styles = StyleSheet.create({
  safeArea: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
  container: {
    flex: 1,
    backgroundColor: COLORS.BACKGROUND,
  },
  header: {
    backgroundColor: COLORS.WHITE,
    padding: 20,
    borderBottomWidth: 1,
    borderBottomColor: COLORS.GRAY_LIGHT,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 8,
  },
  subtitle: {
    fontSize: 14,
    color: COLORS.TEXT_SECONDARY,
  },
  controlBar: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    padding: 16,
    backgroundColor: COLORS.WHITE,
    borderBottomWidth: 1,
    borderBottomColor: COLORS.GRAY_LIGHT,
  },
  scanButton: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: COLORS.PRIMARY,
    paddingHorizontal: 20,
    paddingVertical: 10,
    borderRadius: 20,
  },
  scanButtonActive: {
    backgroundColor: COLORS.DANGER,
  },
  scanButtonText: {
    color: COLORS.WHITE,
    fontSize: 14,
    fontWeight: 'bold',
    marginLeft: 8,
  },
  deviceCount: {
    fontSize: 12,
    color: COLORS.TEXT_SECONDARY,
  },
  deviceList: {
    flex: 1,
    padding: 16,
  },
  deviceItem: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: COLORS.WHITE,
    padding: 16,
    marginBottom: 12,
    borderRadius: 12,
    elevation: 2,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
  },
  deviceInfo: {
    flex: 1,
  },
  deviceName: {
    fontSize: 16,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 4,
  },
  deviceId: {
    fontSize: 12,
    color: COLORS.TEXT_SECONDARY,
    fontFamily: 'monospace',
    marginBottom: 8,
  },
  deviceDetails: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  rssi: {
    fontSize: 12,
    color: COLORS.TEXT_SECONDARY,
    marginRight: 12,
  },
  localName: {
    fontSize: 12,
    color: COLORS.TEXT_SECONDARY,
  },
  deviceActions: {
    alignItems: 'center',
  },
  signalStrength: {
    paddingHorizontal: 8,
    paddingVertical: 4,
    borderRadius: 12,
    marginBottom: 8,
  },
  signalText: {
    color: COLORS.WHITE,
    fontSize: 10,
    fontWeight: 'bold',
  },
  connectText: {
    fontSize: 12,
    color: COLORS.PRIMARY,
    fontWeight: 'bold',
  },
  emptyContainer: {
    alignItems: 'center',
    justifyContent: 'center',
    paddingVertical: 60,
  },
  emptyIcon: {
    fontSize: 48,
    marginBottom: 16,
  },
  emptyTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: COLORS.TEXT_PRIMARY,
    marginBottom: 8,
  },
  emptyText: {
    fontSize: 14,
    color: COLORS.TEXT_SECONDARY,
    textAlign: 'center',
    marginBottom: 24,
    paddingHorizontal: 40,
    lineHeight: 20,
  },
  retryButton: {
    backgroundColor: COLORS.PRIMARY,
    paddingHorizontal: 24,
    paddingVertical: 12,
    borderRadius: 20,
  },
  retryButtonText: {
    color: COLORS.WHITE,
    fontSize: 14,
    fontWeight: 'bold',
  },
  connectingOverlay: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: 'rgba(0,0,0,0.5)',
    justifyContent: 'center',
    alignItems: 'center',
  },
  connectingModal: {
    backgroundColor: COLORS.WHITE,
    padding: 30,
    borderRadius: 15,
    alignItems: 'center',
  },
  connectingText: {
    marginTop: 15,
    fontSize: 16,
    color: COLORS.TEXT_PRIMARY,
  },
});

export default ManualConnect;
