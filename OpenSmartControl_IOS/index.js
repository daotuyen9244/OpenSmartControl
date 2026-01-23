/**
 * @format
 */

// Disable all console.log, console.debug, console.info (DEBUG MODE DISABLED)
// Keep console.warn and console.error for error tracking
const DISABLE_DEBUG_LOGS = true; // Set to false to enable debug logs

if (DISABLE_DEBUG_LOGS) {
  const originalConsole = global.console;
  global.console = {
    ...originalConsole,
    log: () => {},
    debug: () => {},
    info: () => {},
    // Keep warnings and errors for error tracking
    warn: originalConsole.warn,
    error: originalConsole.error,
  };
}

import { AppRegistry } from 'react-native';
import App from './App';
import { name as appName } from './app.json';

AppRegistry.registerComponent(appName, () => App);
