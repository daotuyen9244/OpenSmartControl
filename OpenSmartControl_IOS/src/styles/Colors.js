// src/styles/Colors.js
const COLORS = {
  PRIMARY: '#007AFF',
  SECONDARY: '#4CAF50',
  DANGER: '#F44336',
  WARNING: '#FF9800',
  SUCCESS: '#4CAF50',
  INFO: '#2196F3',
  
  WHITE: '#FFFFFF',
  BLACK: '#000000',
  
  GRAY_LIGHT: '#F5F5F5',
  GRAY_MEDIUM: '#CCCCCC',
  GRAY_DARK: '#666666',
  
  BACKGROUND: '#F8F9FA',
  SURFACE: '#FFFFFF',
  
  TEXT_PRIMARY: '#212121',
  TEXT_SECONDARY: '#757575',
  TEXT_DISABLED: '#BDBDBD',
  
  BORDER: '#E0E0E0',
  DIVIDER: '#EEEEEE',
  
  // Massage specific colors
  MASSAGE_RELAX: '#4CAF50',
  MASSAGE_DEEP: '#F44336',
  MASSAGE_GENTLE: '#FF9800',
  
  // Chair position control colors
  LIGHT: '#F8F9FA',
  DARK: '#343A40',
};

// Export theo nhiều cách để đảm bảo compatibility
module.exports = COLORS;
module.exports.COLORS = COLORS;
module.exports.default = COLORS;

// ES6 exports
export default COLORS;
export { COLORS };
