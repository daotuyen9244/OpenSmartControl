/**
 * constants.js
 * Define constants used throughout the application
 * 
 * Contains:
 * - BLE configuration
 * - Massage modes
 * - Screen names
 * - Control commands
 * - Timeout values
 * - Error messages
 */

/**
 * Bluetooth Low Energy Configuration
 * UUIDs and timeouts for ESP32 connection
 */
export const BLE_CONFIG = {
  // Main service UUID (Nordic UART Service)
  SERVICE_UUID: '6e400001-b5a3-f393-e0a9-e50e24dcca9e',
  
  // UUID to send data to ESP32 (RX from ESP32 side)
  RX_CHAR_UUID: '6e400002-b5a3-f393-e0a9-e50e24dcca9e',
  
  // UUID to receive data from ESP32 (TX from ESP32 side)
  TX_CHAR_UUID: '6e400003-b5a3-f393-e0a9-e50e24dcca9e',
  
  // Timeout for device scanning (ms)
  SCAN_TIMEOUT: 10000,
  
  // Timeout for connection (ms)
  CONNECTION_TIMEOUT: 5000,
  
  // Timeout for receiving fragment (ms)
  FRAGMENT_TIMEOUT: 5000,
  
  // Maximum buffer size for receiving data
  MAX_BUFFER_SIZE: 2000,
  
  // Requested MTU size for better data transmission
  REQUESTED_MTU: 512,
  
  // Cooldown time between QR scans (ms)
  QR_SCAN_COOLDOWN: 2000,
};

/**
 * Available massage modes
 */
export const MASSAGE_MODES = {
  RELAX: 'relax',     // Relax
  DEEP: 'deep',       // Deep
  GENTLE: 'gentle',   // Gentle
  AUTO: 'auto',       // Auto
  MANUAL: 'manual',   // Manual
};

/**
 * Massage types
 */
export const MASSAGE_TYPES = {
  ROLL: 'roll',       // Roll
  SPOT: 'spot',       // Spot
};

/**
 * Massage techniques
 */
export const MASSAGE_TECHNIQUES = {
  KNEADING: 'kneading',         // Kneading
  COMBINE: 'combine',           // Combine
  PERCUSSION: 'percussion',     // Percussion
  COMPRESSION: 'compression',   // Compression
};

/**
 * Screen names in the application
 */
export const SCREENS = {
  HOME: 'Home',                 // Home
  QR_SCANNER: 'QRScanner',      // QR Scanner
  MANUAL_CONNECT: 'ManualConnect', // Manual Connect
  CONTROL: 'Control',           // Control
};

/**
 * Control commands sent to ESP32
 */
export const COMMANDS = {
  // Basic commands
  START: 'START',               // Start massage
  STOP: 'STOP',                 // Stop massage
  STATUS: 'STATUS',             // Get status
  DISCONNECT: 'DISCONNECT',     // Disconnect
  
  // Configuration commands
  MODE: 'MODE',                 // Mode (AUTO/MANUAL)
  INTENSITY: 'INTENSITY',       // Intensity (LOW/HIGH)
  TIMER: 'TIMER',               // Time (minutes)
  ROLLSPOT: 'ROLLSPOT',         // Massage type (ROLL/SPOT)
  
  // Lệnh kỹ thuật
  KNEADING: 'KNEADING',         // Kỹ thuật nhào
  COMBINE: 'COMBINE',           // Kỹ thuật kết hợp
  PERCUSSION: 'PERCUSSION',     // Kỹ thuật gõ
  COMPRESSION: 'COMPRESSION',   // Kỹ thuật nén
  
  // Lệnh vị trí
  RECLINE: 'RECLINE',           // Ngả lưng (0-100%)
  INCLINE: 'INCLINE',           // Nâng chân (0-100%)
  
  // Lệnh chuyển động
  FORWARD: 'FORWARD',           // Chuyển động tiến
  BACKWARD: 'BACKWARD',         // Chuyển động lùi
};

/**
 * Các giá trị mặc định
 */
export const DEFAULT_VALUES = {
  INTENSITY: 'LOW',             // Cường độ mặc định
  DURATION: 15,                 // Thời gian mặc định (phút)
  MODE: MASSAGE_MODES.RELAX,    // Chế độ mặc định
  MASSAGE_TYPE: MASSAGE_TYPES.ROLL, // Kiểu massage mặc định
  RECLINE: 0,                   // Độ ngả lưng mặc định
  INCLINE: 0,                   // Độ nâng chân mặc định
};

/**
 * Giới hạn các giá trị
 */
export const LIMITS = {
  MIN_INTENSITY: 1,             // Cường độ tối thiểu
  MAX_INTENSITY: 2,             // Cường độ tối đa
  MIN_DURATION: 1,              // Thời gian tối thiểu (phút)
  MAX_DURATION: 60,             // Thời gian tối đa (phút)
  MIN_PERCENTAGE: 0,            // Phần trăm tối thiểu
  MAX_PERCENTAGE: 100,          // Phần trăm tối đa
  MAX_DEVICE_NAME_LENGTH: 30,   // Maximum device name length
  MAX_COMMAND_LENGTH: 100,      // Maximum command length
};

/**
 * Timeout times for operations
 */
export const TIMEOUTS = {
  SCAN_DEVICE: 10000,           // Device scan timeout
  CONNECT_DEVICE: 15000,        // Device connection timeout
  SEND_COMMAND: 5000,           // Send command timeout
  RECEIVE_RESPONSE: 10000,      // Receive response timeout
  FRAGMENT_ASSEMBLY: 5000,      // Fragment assembly timeout
  PERMISSION_REQUEST: 30000,    // Permission request timeout
  CAMERA_INIT: 5000,            // Camera initialization timeout
};

/**
 * Error messages
 */
export const ERROR_MESSAGES = {
  // Connection errors
  CONNECTION_FAILED: 'Cannot connect to device',
  DEVICE_NOT_FOUND: 'Device not found',
  BLUETOOTH_DISABLED: 'Bluetooth is not enabled',
  PERMISSION_DENIED: 'Access permission denied',
  
  // QR errors
  INVALID_QR: 'Invalid QR code',
  QR_PARSE_ERROR: 'Cannot read QR code',
  
  // Camera errors
  CAMERA_NOT_AVAILABLE: 'Camera not available',
  CAMERA_PERMISSION_DENIED: 'Camera permission denied',
  
  // Massage errors
  INVALID_INTENSITY: 'Invalid intensity (LOW/HIGH)',
  INVALID_DURATION: 'Invalid duration',
  MASSAGE_NOT_RUNNING: 'Massage not started',
  
  // General errors
  UNKNOWN_ERROR: 'Unknown error',
  NETWORK_ERROR: 'Network connection error',
  TIMEOUT_ERROR: 'Timeout',
};

/**
 * Success messages
 */
export const SUCCESS_MESSAGES = {
  CONNECTION_SUCCESS: 'Connection successful',
  MASSAGE_STARTED: 'Massage started',
  MASSAGE_STOPPED: 'Massage stopped',
  SETTINGS_SAVED: 'Settings saved',
  COMMAND_SENT: 'Command sent successfully',
};

/**
 * Massage duration options (minutes)
 */
export const TIMER_OPTIONS = [5, 10, 15, 20, 30, 45, 60];

/**
 * Massage intensity options
 */
export const INTENSITY_OPTIONS = ['LOW', 'HIGH'];
export const INTENSITY_VALUES = {
  LOW: 1,
  HIGH: 2
};

/**
 * Configuration for platforms
 */
export const PLATFORM_CONFIG = {
  ANDROID: {
    MIN_SDK_VERSION: 21,        // Android 5.0
    TARGET_SDK_VERSION: 33,     // Android 13
    REQUIRED_PERMISSIONS: [
      'BLUETOOTH',
      'BLUETOOTH_ADMIN',
      'BLUETOOTH_SCAN',
      'BLUETOOTH_CONNECT',
      'ACCESS_FINE_LOCATION',
      'CAMERA',
    ],
  },
  IOS: {
    MIN_VERSION: '12.0',        // iOS 12.0
    REQUIRED_PERMISSIONS: [
      'NSBluetoothAlwaysUsageDescription',
      'NSCameraUsageDescription',
    ],
  },
};

/**
 * Regex patterns
 */
export const REGEX_PATTERNS = {
  MAC_ADDRESS: /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/,
  UUID: /^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i,
  EMAIL: /^[^\s@]+@[^\s@]+\.[^\s@]+$/,
  PHONE_VN: /^(\+84|84|0)(3|5|7|8|9)([0-9]{8})$/,
  DEVICE_NAME: /^[a-zA-Z0-9\-_\s]{3,30}$/,
};

/**
 * Animation durations (ms)
 */
export const ANIMATION_DURATIONS = {
  FAST: 200,
  NORMAL: 300,
  SLOW: 500,
  VERY_SLOW: 1000,
};

/**
 * Z-index values
 */
export const Z_INDEX = {
  MODAL: 1000,
  OVERLAY: 999,
  DROPDOWN: 998,
  TOOLTIP: 997,
  NAVIGATION: 100,
};

// Export all constants as object
export const CONSTANTS = {
  BLE_CONFIG,
  MASSAGE_MODES,
  MASSAGE_TYPES,
  MASSAGE_TECHNIQUES,
  SCREENS,
  COMMANDS,
  DEFAULT_VALUES,
  LIMITS,
  TIMEOUTS,
  ERROR_MESSAGES,
  SUCCESS_MESSAGES,
  TIMER_OPTIONS,
  INTENSITY_OPTIONS,
  INTENSITY_VALUES,
  PLATFORM_CONFIG,
  REGEX_PATTERNS,
  ANIMATION_DURATIONS,
  Z_INDEX,
};

export default CONSTANTS;
