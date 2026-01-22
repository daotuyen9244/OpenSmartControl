/**
 * Logger utility - PRODUCTION MODE: Console.log is disabled
 * To enable in development, set ENABLE_DEBUG_LOGS to true
 */

const isDev = __DEV__;
const ENABLE_DEBUG_LOGS = false; // Set to false to disable all console logs (DISABLED FOR PRODUCTION)

const logger = {
  log: ENABLE_DEBUG_LOGS ? console.log.bind(console) : () => {},
  info: ENABLE_DEBUG_LOGS ? console.info.bind(console) : () => {},
  debug: ENABLE_DEBUG_LOGS ? console.debug.bind(console) : () => {},
  warn: console.warn.bind(console), // Keep warnings in production
  error: console.error.bind(console), // Keep errors in production
};

// Disable console.log in production builds
if (!ENABLE_DEBUG_LOGS) {
  // Keep error and warn, remove others
  const originalConsole = global.console;
  global.console = {
    ...originalConsole,
    log: () => {},
    info: () => {},
    debug: () => {},
    // Keep these for error tracking
    warn: originalConsole.warn,
    error: originalConsole.error,
  };
}

export default logger;

