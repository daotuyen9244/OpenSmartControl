/**
 * screens.js
 * Define constants for screen names in the application
 * 
 * Purpose:
 * - Avoid hardcoding screen names
 * - Easy to refactor and maintain
 * - Type safety when using with TypeScript
 */

// Define main screens of the application
export const SCREENS = {
  // Main screen - home page
  HOME: 'Home',
  
  // QR Code scanner screen to connect device
  QR_SCANNER: 'QRScanner',
  
  // Manual connection screen with Bluetooth device
  MANUAL_CONNECT: 'ManualConnect',
  
  // Massage machine control screen
  CONTROL: 'Control',
};

// Export default for multiple import options
export default SCREENS;
