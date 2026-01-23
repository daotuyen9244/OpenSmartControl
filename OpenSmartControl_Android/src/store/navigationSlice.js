/**
 * navigationSlice.js
 * Redux slice để quản lý navigation state của ứng dụng
 * 
 * Tính năng:
 * - Theo dõi màn hình hiện tại
 * - Lưu trữ lịch sử navigation
 * - Hỗ trợ back navigation
 * - Reset navigation state
 */

import { createSlice } from '@reduxjs/toolkit';

/**
 * Định nghĩa các màn hình trong ứng dụng
 * Centralized screen names để tránh typo và dễ maintain
 */
const SCREENS = {
  HOME: 'Home',                    // Màn hình chính
  QR_SCANNER: 'QRScanner',        // Màn hình quét QR code
  MANUAL_CONNECT: 'ManualConnect', // Màn hình kết nối thủ công
  CONTROL: 'Control',             // Màn hình điều khiển massage
};

/**
 * Navigation Redux Slice
 * Quản lý state điều hướng giữa các màn hình
 */
const navigationSlice = createSlice({
  name: 'navigation',
  
  /**
   * Initial state cho navigation
   */
  initialState: {
    currentScreen: SCREENS.HOME,     // Màn hình hiện tại
    previousScreen: null,            // Màn hình trước đó
    navigationHistory: [SCREENS.HOME], // Lịch sử navigation (stack)
  },

  /**
   * Reducers để xử lý navigation actions
   */
  reducers: {
    /**
     * Điều hướng đến màn hình mới
     * @param {Object} state - Current navigation state
     * @param {Object} action - Action với screen name payload
     */
    navigateTo: (state, action) => {
      // Lưu màn hình hiện tại làm previous screen
      state.previousScreen = state.currentScreen;
      
      // Cập nhật màn hình hiện tại
      state.currentScreen = action.payload;
      
      // Thêm vào lịch sử navigation
      state.navigationHistory.push(action.payload);
      
      // Giới hạn history để tránh memory leak
      // Chỉ giữ lại 10 màn hình gần nhất
      if (state.navigationHistory.length > 10) {
        state.navigationHistory = state.navigationHistory.slice(-10);
      }
    },

    /**
     * Quay lại màn hình trước đó
     * @param {Object} state - Current navigation state
     */
    goBack: (state) => {
      // Chỉ cho phép back nếu có ít nhất 2 màn hình trong history
      if (state.navigationHistory.length > 1) {
        // Remove màn hình hiện tại khỏi history
        state.navigationHistory.pop();
        
        // Set màn hình cuối cùng trong history làm current screen
        state.currentScreen = state.navigationHistory[state.navigationHistory.length - 1];
      }
    },

    /**
     * Reset navigation về trạng thái ban đầu
     * @param {Object} state - Current navigation state
     */
    resetNavigation: (state) => {
      state.currentScreen = SCREENS.HOME;
      state.previousScreen = null;
      state.navigationHistory = [SCREENS.HOME];
    },
  },
});

// Export actions for use in components
export const { navigateTo, goBack, resetNavigation } = navigationSlice.actions;

// Export reducer for use in store
export default navigationSlice.reducer;

// Export SCREENS constant for use in components
export { SCREENS };
