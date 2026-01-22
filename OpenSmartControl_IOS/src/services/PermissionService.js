/**
 * PermissionService.js
 * Service để quản lý các quyền truy cập của ứng dụng
 * 
 * Tính năng:
 * - Quản lý quyền camera
 * - Quản lý quyền location (cho BLE)
 * - Quản lý quyền Bluetooth
 * - Hiển thị dialog yêu cầu quyền
 * - Xử lý cross-platform (iOS/Android)
 */

import { PermissionsAndroid, Platform, Alert, Linking } from 'react-native';
import { check, request, PERMISSIONS, RESULTS } from 'react-native-permissions';

class PermissionService {
  
  /**
   * Yêu cầu quyền truy cập camera
   * @returns {Promise<boolean>} True nếu được cấp quyền
   */
  async requestCameraPermission() {
    try {
      if (Platform.OS === 'android') {
        const granted = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.CAMERA,
          {
            title: 'Quyền truy cập Camera',
            message: 'Ứng dụng cần quyền truy cập camera để quét QR code',
            buttonNeutral: 'Hỏi lại sau',
            buttonNegative: 'Hủy',
            buttonPositive: 'Đồng ý',
          }
        );
        return granted === PermissionsAndroid.RESULTS.GRANTED;
      } else {
        // iOS
        const result = await request(PERMISSIONS.IOS.CAMERA);
        return result === RESULTS.GRANTED;
      }
    } catch (error) {
      console.error('Camera permission request failed:', error);
      return false;
    }
  }

  /**
   * Kiểm tra quyền camera hiện tại
   * @returns {Promise<boolean>} True nếu đã có quyền
   */
  async checkCameraPermission() {
    try {
      if (Platform.OS === 'android') {
        const result = await PermissionsAndroid.check(
          PermissionsAndroid.PERMISSIONS.CAMERA
        );
        return result;
      } else {
        // iOS
        const result = await check(PERMISSIONS.IOS.CAMERA);
        return result === RESULTS.GRANTED;
      }
    } catch (error) {
      console.error('Camera permission check failed:', error);
      return false;
    }
  }

  /**
   * Yêu cầu quyền location (cần thiết cho BLE scan trên Android)
   * @returns {Promise<boolean>} True nếu được cấp quyền
   */
  async requestLocationPermission() {
    try {
      if (Platform.OS === 'android') {
        const permissions = [
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
          PermissionsAndroid.PERMISSIONS.ACCESS_COARSE_LOCATION,
        ];
        
        const granted = await PermissionsAndroid.requestMultiple(permissions);
        return Object.values(granted).every(
          permission => permission === PermissionsAndroid.RESULTS.GRANTED
        );
      }
      
      // iOS không cần location permission cho BLE
      return true;
    } catch (error) {
      console.error('Location permission request failed:', error);
      return false;
    }
  }

  /**
   * Yêu cầu quyền Bluetooth (Android 12+)
   * @returns {Promise<boolean>} True nếu được cấp quyền
   */
  async requestBluetoothPermissions() {
    try {
      if (Platform.OS === 'android') {
        const permissions = [];
        
        // Android 12+ cần quyền mới
        if (Platform.Version >= 31) {
          permissions.push(
            PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
            PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
            PermissionsAndroid.PERMISSIONS.BLUETOOTH_ADVERTISE
          );
        } else {
          // Android cũ
          permissions.push(
            PermissionsAndroid.PERMISSIONS.BLUETOOTH,
            PermissionsAndroid.PERMISSIONS.BLUETOOTH_ADMIN
          );
        }
        
        const granted = await PermissionsAndroid.requestMultiple(permissions);
        return Object.values(granted).every(
          permission => permission === PermissionsAndroid.RESULTS.GRANTED
        );
      }
      
      // iOS tự động xử lý quyền Bluetooth
      return true;
    } catch (error) {
      console.error('Bluetooth permission request failed:', error);
      return false;
    }
  }

  /**
   * Mở Settings của app để người dùng có thể cấp quyền
   * @param {string} permissionType - 'bluetooth', 'camera', 'location', hoặc 'all'
   * @returns {Promise<boolean>} - True nếu mở Settings thành công
   */
  async openSettings(permissionType = 'all') {
    try {
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
      console.error('❌ Failed to open Settings:', error);
      
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
   * Hiển thị dialog yêu cầu người dùng vào Settings để cấp quyền
   * @param {string} title - Tiêu đề dialog
   * @param {string} message - Nội dung thông báo
   * @param {Function} onSettings - Callback khi người dùng chọn Settings
   */
  showPermissionAlert(title, message, onSettings) {
    Alert.alert(
      title,
      message,
      [
        { text: 'Hủy', style: 'cancel' },
        {
          text: 'Mở Settings',
          onPress: () => {
            if (onSettings) {
              onSettings();
            } else {
              // Mở Settings của ứng dụng
              this.openSettings('all');
            }
          }
        }
      ]
    );
  }

  /**
   * Hiển thị Alert yêu cầu mở Settings cho BLE permission
   */
  showBluetoothPermissionAlert() {
    this.showPermissionAlert(
      'Quyền Bluetooth cần thiết',
      'Ứng dụng cần quyền Bluetooth để kết nối với thiết bị massage.\n\nVui lòng mở Settings và bật quyền Bluetooth cho app.',
      () => this.openSettings('bluetooth')
    );
  }

  /**
   * Hiển thị Alert yêu cầu mở Settings cho Camera permission
   */
  showCameraPermissionAlert() {
    this.showPermissionAlert(
      'Quyền Camera cần thiết',
      'Ứng dụng cần quyền Camera để quét QR code kết nối thiết bị.\n\nVui lòng mở Settings và bật quyền Camera cho app.',
      () => this.openSettings('camera')
    );
  }

  /**
   * Kiểm tra và yêu cầu tất cả quyền cần thiết cho ứng dụng
   * @returns {Promise<Object>} Object chứa trạng thái các quyền
   */
  async checkAllPermissions() {
    const permissions = {
      camera: await this.checkCameraPermission(),
      location: Platform.OS === 'android' ? await this.checkLocationPermission() : true,
      bluetooth: true, // BLE permissions được xử lý trong BleService
    };

    return permissions;
  }

  /**
   * Kiểm tra quyền location
   * @returns {Promise<boolean>} True nếu đã có quyền
   */
  async checkLocationPermission() {
    try {
      if (Platform.OS === 'android') {
        const fineLocation = await PermissionsAndroid.check(
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION
        );
        const coarseLocation = await PermissionsAndroid.check(
          PermissionsAndroid.PERMISSIONS.ACCESS_COARSE_LOCATION
        );
        return fineLocation || coarseLocation;
      }
      return true;
    } catch (error) {
      console.error('Location permission check failed:', error);
      return false;
    }
  }

  /**
   * Yêu cầu tất cả quyền cần thiết
   * @returns {Promise<boolean>} True nếu tất cả quyền được cấp
   */
  async requestAllPermissions() {
    try {
      const results = await Promise.all([
        this.requestCameraPermission(),
        this.requestLocationPermission(),
        this.requestBluetoothPermissions(),
      ]);

      return results.every(result => result === true);
    } catch (error) {
      console.error('Request all permissions failed:', error);
      return false;
    }
  }
}

// Export singleton instance
export default new PermissionService();
