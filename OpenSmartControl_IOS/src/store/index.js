/**
 * index.js (store/index.js)
 * Cấu hình Redux store chính cho ứng dụng massage
 * 
 * Tính năng:
 * - Kết hợp các slice reducers
 * - Cấu hình middleware
 * - Xử lý serialization cho BLE objects
 * - Tối ưu hóa performance
 */

import { configureStore } from '@reduxjs/toolkit';
import combinedReducer from './bleSlice';
import navigationSlice from './navigationSlice';

/**
 * Cấu hình Redux store với Redux Toolkit
 * Bao gồm middleware tùy chỉnh để xử lý BLE objects
 */
export const store = configureStore({
  /**
   * Kết hợp các reducers
   */
  reducer: {
    ble: combinedReducer,    // Quản lý Bluetooth và massage state với safe handling
    navigation: navigationSlice, // Quản lý navigation state
  },

  /**
   * Cấu hình middleware
   * Tùy chỉnh serializableCheck để xử lý BLE objects
   */
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      /**
       * Cấu hình serializableCheck để tránh warning với BLE objects
       * BLE objects từ react-native-ble-plx chứa functions và không thể serialize
       */
      serializableCheck: {
        /**
         * Bỏ qua các action types có chứa BLE objects
         * Các action này đã được xử lý để chỉ lưu serializable data
         */
        ignoredActions: [
          'ble/setDeviceInfo',           // Device info từ QR code
          'ble/setConnectedDevice',      // Connected device object
          'ble/updateMassageSettings',   // Massage settings updates
          // Thêm các action types khác nếu cần
        ],

        /**
         * Bỏ qua các paths trong action payload
         * Những paths này có thể chứa non-serializable data
         */
        ignoredActionPaths: [
          'payload.0',              // Array elements
          'payload.device',         // Device objects
          'payload.deviceInfo',     // Device info objects
          'meta.arg',              // Meta arguments
          'meta.baseQueryMeta',    // Query meta data
        ],

        /**
         * Bỏ qua các paths trong state
         * Những paths này đã được serialize nhưng vẫn có thể trigger warning
         */
        ignoredPaths: [
          'ble.deviceInfo',        // Device info trong state
          'ble.connectedDevice',   // Connected device trong state
          'ble.massageSettings',   // Massage settings object
        ],
      },
    }),
});

/**
 * Export store types for TypeScript (if using)
 * Note: Uncomment these if using TypeScript
 */
// export type RootState = ReturnType<typeof store.getState>;
// export type AppDispatch = typeof store.dispatch;

/**
 * Export default store
 */
export default store;
