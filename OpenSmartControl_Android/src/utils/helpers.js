/**
 * helpers.js
 * Common utility functions for the application
 * 
 * Contains utility functions for:
 * - Format time
 * - Debounce functions
 * - Signal strength processing
 * - Generate ID
 * - Delay functions
 */

/**
 * Format time from seconds to MM:SS format
 * @param {number} seconds - Number of seconds to format
 * @returns {string} Time string in MM:SS format
 * 
 * @example
 * formatTime(125) // "02:05"
 * formatTime(60)  // "01:00"
 * formatTime(5)   // "00:05"
 */
export const formatTime = (seconds) => {
  const mins = Math.floor(seconds / 60);
  const secs = seconds % 60;
  return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
};

/**
 * Create debounced function to avoid calling function too many times
 * @param {Function} func - Function to debounce
 * @param {number} delay - Delay time (ms)
 * @returns {Function} Debounced function
 * 
 * @example
 * const debouncedSearch = debounce(searchFunction, 300);
 * debouncedSearch('query'); // Only called after 300ms without new calls
 */
export const debounce = (func, delay) => {
  let timeoutId;
  return (...args) => {
    clearTimeout(timeoutId);
    timeoutId = setTimeout(() => func.apply(null, args), delay);
  };
};

/**
 * Calculate signal strength based on RSSI
 * @param {number} rssi - RSSI value from Bluetooth device
 * @returns {number} Signal strength from 0-4
 * 
 * @example
 * getSignalStrength(-45) // 4 (very strong)
 * getSignalStrength(-65) // 2 (medium)
 * getSignalStrength(-85) // 0 (weak)
 */
export const getSignalStrength = (rssi) => {
  if (!rssi) return 0;
  if (rssi > -50) return 4; // Very strong
  if (rssi > -60) return 3; // Strong
  if (rssi > -70) return 2; // Medium
  if (rssi > -80) return 1; // Weak
  return 0; // Very weak
};

/**
 * Get icon corresponding to signal strength
 * @param {number} rssi - RSSI value
 * @returns {string} Corresponding emoji icon
 * 
 * @example
 * getSignalIcon(-45) // "📶" (4 thanh)
 * getSignalIcon(-75) // "📶" (2 thanh)
 */
export const getSignalIcon = (rssi) => {
  const strength = getSignalStrength(rssi);
  const icons = ['📶', '📶', '📶', '📶', '📶'];
  return icons[strength] || '📶';
};

/**
 * Generate unique ID based on timestamp and random
 * @returns {string} Unique ID
 * 
 * @example
 * generateId() // "k8j2h4k8j2h4"
 */
export const generateId = () => {
  return Date.now().toString(36) + Math.random().toString(36).substr(2);
};

/**
 * Create delay using Promise
 * @param {number} ms - Delay time (milliseconds)
 * @returns {Promise} Promise resolves after delay time
 * 
 * @example
 * await delay(1000); // Wait 1 second
 * console.log('After 1 second');
 */
export const delay = (ms) => {
  return new Promise(resolve => setTimeout(resolve, ms));
};

/**
 * Convert bytes to readable string
 * @param {number} bytes - Number of bytes
 * @returns {string} Readable string (KB, MB, GB)
 * 
 * @example
 * formatBytes(1024) // "1.0 KB"
 * formatBytes(1048576) // "1.0 MB"
 */
export const formatBytes = (bytes) => {
  if (bytes === 0) return '0 Bytes';
  
  const k = 1024;
  const sizes = ['Bytes', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  
  return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
};

/**
 * Check if string is valid JSON
 * @param {string} str - String to check
 * @returns {boolean} True if valid JSON
 * 
 * @example
 * isValidJSON('{"name": "test"}') // true
 * isValidJSON('invalid json') // false
 */
export const isValidJSON = (str) => {
  try {
    JSON.parse(str);
    return true;
  } catch (e) {
    return false;
  }
};

/**
 * Truncate string if too long
 * @param {string} str - Original string
 * @param {number} maxLength - Maximum length
 * @returns {string} Truncated string
 * 
 * @example
 * truncateString('Hello World', 5) // "Hello..."
 */
export const truncateString = (str, maxLength) => {
  if (!str || str.length <= maxLength) return str;
  return str.substring(0, maxLength) + '...';
};

/**
 * Capitalize first letter of string
 * @param {string} str - Original string
 * @returns {string} Capitalized string
 * 
 * @example
 * capitalize('hello world') // "Hello world"
 */
export const capitalize = (str) => {
  if (!str) return '';
  return str.charAt(0).toUpperCase() + str.slice(1);
};

/**
 * Check if device is tablet
 * @returns {boolean} True if tablet
 */
export const isTablet = () => {
  const { width, height } = require('react-native').Dimensions.get('window');
  const aspectRatio = width / height;
  return Math.min(width, height) >= 600 && (aspectRatio > 1.2 || aspectRatio < 0.9);
};

/**
 * Generate random color
 * @returns {string} Hex color code
 * 
 * @example
 * getRandomColor() // "#ff5733"
 */
export const getRandomColor = () => {
  return '#' + Math.floor(Math.random() * 16777215).toString(16);
};
