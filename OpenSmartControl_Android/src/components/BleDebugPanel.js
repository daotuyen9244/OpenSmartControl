import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  TouchableOpacity,
  ScrollView,
  StyleSheet,
  Alert,
} from 'react-native';
import { useDispatch, useSelector } from 'react-redux';
import BleService from '../services/BleService';
import { setDebugInfo } from '../store/bleSlice';

const BleDebugPanel = () => {
  const dispatch = useDispatch();
  const [debugData, setDebugData] = useState(null);
  const [isLoading, setIsLoading] = useState(false);
  
  const { 
    isConnected, 
    isConnecting, 
    connectionError, 
    connectedDevice,
    debugInfo 
  } = useSelector(state => state.ble);

  const refreshDebugInfo = async () => {
    setIsLoading(true);
    try {
      const info = await BleService.debugDeviceInfo();
      setDebugData(info);
      dispatch(setDebugInfo(info));
    } catch (error) {
      console.error('Failed to get debug info:', error);
      Alert.alert('Error', 'Failed to get debug information');
    } finally {
      setIsLoading(false);
    }
  };

  const clearStoredConnection = async () => {
    try {
      await BleService.clearConnectionFromStorage();
      Alert.alert('Success', 'Stored connection cleared');
      refreshDebugInfo();
    } catch (error) {
      Alert.alert('Error', 'Failed to clear stored connection');
    }
  };

  const testConnection = async () => {
    if (!connectedDevice) {
      Alert.alert('Error', 'No device connected');
      return;
    }

    try {
      const info = await BleService.debugDeviceInfo();
      Alert.alert('Connection Test', JSON.stringify(info, null, 2));
    } catch (error) {
      Alert.alert('Error', error.message);
    }
  };

  useEffect(() => {
    refreshDebugInfo();
  }, []);

  return (
    <ScrollView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>BLE Debug Panel</Text>
        <TouchableOpacity 
          style={styles.refreshButton} 
          onPress={refreshDebugInfo}
          disabled={isLoading}
        >
          <Text style={styles.buttonText}>
            {isLoading ? 'Loading...' : 'Refresh'}
          </Text>
        </TouchableOpacity>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Connection Status</Text>
        <Text style={styles.text}>Connected: {isConnected ? 'Yes' : 'No'}</Text>
        <Text style={styles.text}>Connecting: {isConnecting ? 'Yes' : 'No'}</Text>
        {connectionError && (
          <Text style={styles.errorText}>Error: {connectionError}</Text>
        )}
      </View>

      {connectedDevice && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Connected Device</Text>
          <Text style={styles.text}>Name: {connectedDevice.name || 'Unknown'}</Text>
          <Text style={styles.text}>ID: {connectedDevice.id}</Text>
          <Text style={styles.text}>RSSI: {connectedDevice.rssi}</Text>
        </View>
      )}

      {debugData && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Debug Information</Text>
          {debugData.error ? (
            <Text style={styles.errorText}>{debugData.error}</Text>
          ) : (
            <>
              <Text style={styles.text}>Device ID: {debugData.deviceId}</Text>
              <Text style={styles.text}>Is Connected: {debugData.isConnected ? 'Yes' : 'No'}</Text>
              <Text style={styles.text}>Device Name: {debugData.deviceName}</Text>
              <Text style={styles.text}>Target Service: {debugData.targetService}</Text>
              <Text style={styles.text}>Service Found: {debugData.targetServiceFound ? 'Yes' : 'No'}</Text>
              
              {debugData.services && debugData.services.length > 0 && (
                <View style={styles.subSection}>
                  <Text style={styles.subTitle}>Available Services:</Text>
                  {debugData.services.map((service, index) => (
                    <Text key={index} style={styles.serviceText}>
                      • {service.uuid} {service.isPrimary ? '(Primary)' : ''}
                    </Text>
                  ))}
                </View>
              )}

              {debugData.characteristics && debugData.characteristics.length > 0 && (
                <View style={styles.subSection}>
                  <Text style={styles.subTitle}>Characteristics:</Text>
                  {debugData.characteristics.map((char, index) => (
                    <Text key={index} style={styles.serviceText}>
                      • {char.uuid}
                    </Text>
                  ))}
                </View>
              )}

              {debugData.targetCharacteristics && (
                <View style={styles.subSection}>
                  <Text style={styles.subTitle}>Target Characteristics:</Text>
                  <Text style={styles.text}>RX: {debugData.targetCharacteristics.rxCharFound ? 'Found' : 'Not Found'}</Text>
                  <Text style={styles.text}>TX: {debugData.targetCharacteristics.txCharFound ? 'Found' : 'Not Found'}</Text>
                </View>
              )}
            </>
          )}
        </View>
      )}

      <View style={styles.buttonSection}>
        <TouchableOpacity style={styles.actionButton} onPress={testConnection}>
          <Text style={styles.buttonText}>Test Connection</Text>
        </TouchableOpacity>
        
        <TouchableOpacity style={styles.actionButton} onPress={clearStoredConnection}>
          <Text style={styles.buttonText}>Clear Stored Connection</Text>
        </TouchableOpacity>
      </View>
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f5f5',
    padding: 16,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 20,
  },
  title: {
    fontSize: 20,
    fontWeight: 'bold',
    color: '#333',
  },
  refreshButton: {
    backgroundColor: '#007AFF',
    paddingHorizontal: 16,
    paddingVertical: 8,
    borderRadius: 8,
  },
  buttonText: {
    color: 'white',
    fontWeight: '600',
  },
  section: {
    backgroundColor: 'white',
    padding: 16,
    borderRadius: 8,
    marginBottom: 16,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333',
    marginBottom: 12,
  },
  subSection: {
    marginTop: 12,
    paddingTop: 12,
    borderTopWidth: 1,
    borderTopColor: '#eee',
  },
  subTitle: {
    fontSize: 14,
    fontWeight: '600',
    color: '#666',
    marginBottom: 8,
  },
  text: {
    fontSize: 14,
    color: '#333',
    marginBottom: 4,
  },
  errorText: {
    fontSize: 14,
    color: '#FF3B30',
    marginBottom: 4,
  },
  serviceText: {
    fontSize: 12,
    color: '#666',
    marginBottom: 2,
    fontFamily: 'monospace',
  },
  buttonSection: {
    flexDirection: 'row',
    justifyContent: 'space-around',
    marginTop: 20,
  },
  actionButton: {
    backgroundColor: '#34C759',
    paddingHorizontal: 16,
    paddingVertical: 12,
    borderRadius: 8,
    flex: 0.45,
  },
});

export default BleDebugPanel;
