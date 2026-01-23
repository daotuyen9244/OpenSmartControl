#include "SafetyManager.h"

/**
 * Constructor
 */
SafetyManager::SafetyManager(TimerManager* timerMgr, HardwareSerial* debugSer)
  : timerManager(timerMgr), debugSerial(debugSer), systemStuck(false), emergencyStopActive(false), lastSystemActivityTick(0), systemStuckStartTick(0), lastWatchdogFeedTick(0), systemHealthCheckActive(false), lastHealthCheckTick(0) {
}

/**
 * Destructor
 */
SafetyManager::~SafetyManager() {
  // Cleanup if needed
}

/**
 * Initialize safety manager
 */
void SafetyManager::initialize() {
  watchdogInit();

  // Initialize system state
  systemStuck = false;
  emergencyStopActive = false;
  lastSystemActivityTick = timerManager->getMasterTicks();
  systemStuckStartTick = 0;
  lastWatchdogFeedTick = 0;

  // Initialize health monitoring
  systemHealthCheckActive = true;
  lastHealthCheckTick = timerManager->getMasterTicks();
}

/**
 * Initialize watchdog timer
 */
void SafetyManager::watchdogInit() {
// Enable watchdog timer with 3 second timeout
#ifdef HAL_IWDG_MODULE_ENABLED
  IWDG_HandleTypeDef hiwdg;
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = WATCHDOG_TIMEOUT;
  HAL_IWDG_Init(&hiwdg);
#else
  // Direct register access for STM32
  IWDG->KR = 0x5555;             // Enable access to IWDG registers
  IWDG->PR = 0x06;               // Prescaler 64
  IWDG->RLR = WATCHDOG_TIMEOUT;  // Reload value
  IWDG->KR = 0xCCCC;             // Start watchdog
#endif

  lastWatchdogFeedTick = timerManager->getMasterTicks();
}

/**
 * Reset watchdog timer
 */
void SafetyManager::watchdogReset() {
#ifdef HAL_IWDG_MODULE_ENABLED
  IWDG_HandleTypeDef hiwdg;
  hiwdg.Instance = IWDG;
  HAL_IWDG_Refresh(&hiwdg);
#else
  IWDG->KR = 0xAAAA;             // Reload watchdog
#endif

  lastWatchdogFeedTick = timerManager->getMasterTicks();
}

/**
 * Feed watchdog timer
 */
void SafetyManager::watchdogFeed() {
  watchdogReset();
}

/**
 * Check if watchdog is active
 */
bool SafetyManager::isWatchdogActive() const {
  return true;  // Watchdog is always active once initialized
}

/**
 * Get last watchdog feed tick
 */
unsigned long SafetyManager::getLastWatchdogFeedTick() const {
  return lastWatchdogFeedTick;
}

/**
 * Check system health
 */
void SafetyManager::checkSystemHealth() {
  if (!systemHealthCheckActive) return;

  unsigned long currentTick = timerManager->getMasterTicks();

  // Perform health check at regular intervals
  if (currentTick - lastHealthCheckTick >= HEALTH_CHECK_INTERVAL_TICKS) {
    performHealthCheck();
    lastHealthCheckTick = currentTick;
  }

  // Check for system stuck condition
  detectSystemStuck();

  // Update system activity
  updateSystemActivity();
}

/**
 * Update system activity
 */
void SafetyManager::updateSystemActivity() {
  unsigned long currentTick = timerManager->getMasterTicks();

  // Check if system has been active recently
  if (hasSystemActivity()) {
    lastSystemActivityTick = currentTick;

    // Clear stuck condition if system is active
    if (systemStuck) {
      resetSystemStuckDetection();
    }
  }
}

/**
 * Check if system is stuck
 */
bool SafetyManager::isSystemStuck() const {
  return systemStuck;
}

/**
 * Check if emergency stop is active
 */
bool SafetyManager::isEmergencyStopActive() const {
  return emergencyStopActive;
}

/**
 * Set system stuck state
 */
void SafetyManager::setSystemStuck(bool stuck) {
  systemStuck = stuck;
  if (stuck) {
    systemStuckStartTick = timerManager->getMasterTicks();
  } else {
    systemStuckStartTick = 0;
  }
}

/**
 * Set emergency stop state
 */
void SafetyManager::setEmergencyStopActive(bool active) {
  emergencyStopActive = active;
}

/**
 * Emergency stop
 */
void SafetyManager::emergencyStop() {
  emergencyStopActive = true;
  performEmergencyShutdown();
}

/**
 * Clear emergency stop
 */
void SafetyManager::clearEmergencyStop() {
  emergencyStopActive = false;
}

/**
 * Initiate system recovery
 */
void SafetyManager::initiateSystemRecovery() {
  if (systemStuck) {
    performSystemReset();
  }
}

/**
 * Complete system recovery
 */
void SafetyManager::completeSystemRecovery() {
  systemStuck = false;
  systemStuckStartTick = 0;
  emergencyStopActive = false;
}

/**
 * Check if system is recovering
 */
bool SafetyManager::isSystemRecovering() const {
  return systemStuck && (timerManager->getMasterTicks() - systemStuckStartTick) < SYSTEM_RECOVERY_TIMEOUT_TICKS;
}

/**
 * Print safety status
 */
void SafetyManager::printSafetyStatus() const {
  // Safety Status debug disabled to save Flash memory
}

/**
 * Print system health status
 */
void SafetyManager::printSystemHealthStatus() const {
  // System Health debug disabled to save Flash memory
}

/**
 * Record motor activity
 */
void SafetyManager::recordMotorActivity() {
  lastSystemActivityTick = timerManager->getMasterTicks();
}

/**
 * Record sensor activity
 */
void SafetyManager::recordSensorActivity() {
  lastSystemActivityTick = timerManager->getMasterTicks();
}

/**
 * Record communication activity
 */
void SafetyManager::recordCommunicationActivity() {
  lastSystemActivityTick = timerManager->getMasterTicks();
}

/**
 * Record sequence activity
 */
void SafetyManager::recordSequenceActivity() {
  lastSystemActivityTick = timerManager->getMasterTicks();
}

/**
 * Set system stuck timeout
 */
void SafetyManager::setSystemStuckTimeout(unsigned long timeoutTicks) {
  // Implementation for custom timeout
}

/**
 * Set recovery timeout
 */
void SafetyManager::setRecoveryTimeout(unsigned long timeoutTicks) {
  // Implementation for custom recovery timeout
}

/**
 * Set health check interval
 */
void SafetyManager::setHealthCheckInterval(unsigned long intervalTicks) {
  // Implementation for custom health check interval
}

/**
 * Get last system activity tick
 */
unsigned long SafetyManager::getLastSystemActivityTick() const {
  return lastSystemActivityTick;
}

/**
 * Get system stuck start tick
 */
unsigned long SafetyManager::getSystemStuckStartTick() const {
  return systemStuckStartTick;
}

/**
 * Get system health check active state
 */
bool SafetyManager::getSystemHealthCheckActive() const {
  return systemHealthCheckActive;
}

/**
 * Get last health check tick
 */
unsigned long SafetyManager::getLastHealthCheckTick() const {
  return lastHealthCheckTick;
}

/**
 * Set last system activity tick
 */
void SafetyManager::setLastSystemActivityTick(unsigned long tick) {
  lastSystemActivityTick = tick;
}

/**
 * Set system stuck start tick
 */
void SafetyManager::setSystemStuckStartTick(unsigned long tick) {
  systemStuckStartTick = tick;
}

/**
 * Set system health check active state
 */
void SafetyManager::setSystemHealthCheckActive(bool active) {
  systemHealthCheckActive = active;
}

/**
 * Set last health check tick
 */
void SafetyManager::setLastHealthCheckTick(unsigned long tick) {
  lastHealthCheckTick = tick;
}

/**
 * Private Helper Functions
 */
void SafetyManager::detectSystemStuck() {
  unsigned long currentTick = timerManager->getMasterTicks();
  unsigned long timeSinceActivity = currentTick - lastSystemActivityTick;

  if (timeSinceActivity >= SYSTEM_STUCK_TIMEOUT_TICKS && !systemStuck) {
    handleSystemStuck();
  }
}

void SafetyManager::handleSystemStuck() {
  systemStuck = true;
  systemStuckStartTick = timerManager->getMasterTicks();

  // WARNING: System stuck detected! debug disabled

  // Perform emergency shutdown
  performEmergencyShutdown();
}

void SafetyManager::performHealthCheck() {
  // Check various system components
  bool systemHealthy = validateSystemState();

  if (!systemHealthy) {
    // WARNING: System health check failed! debug disabled
    // Take appropriate action
  }
}

void SafetyManager::resetSystemStuckDetection() {
  systemStuck = false;
  systemStuckStartTick = 0;
}

bool SafetyManager::hasSystemActivity() const {
  // This would check for various system activities
  // For now, return true to indicate system is active
  return true;
}

unsigned long SafetyManager::getTimeSinceLastActivity() const {
  return timerManager->getMasterTicks() - lastSystemActivityTick;
}

bool SafetyManager::validateSystemState() const {
  // Validate various system states
  // For now, return true to indicate system is valid
  return true;
}

bool SafetyManager::isSystemInSafeState() const {
  // Check if system is in a safe state
  // For now, return true to indicate system is safe
  return true;
}

void SafetyManager::performEmergencyShutdown() {
  // EMERGENCY SHUTDOWN INITIATED! debug disabled

  // Stop all motors
  performMotorShutdown();

  // Reset sensors
  performSensorReset();

  // Set emergency stop flag
  emergencyStopActive = true;
}

void SafetyManager::performSystemReset() {
  // SYSTEM RESET INITIATED! debug disabled

  // Perform system reset procedures
  // This would reset various system components

  // Clear stuck condition
  resetSystemStuckDetection();
}

void SafetyManager::performMotorShutdown() {
  // MOTOR SHUTDOWN INITIATED! debug disabled

  // This would call motor controller to stop all motors
  // Implementation depends on motor controller integration
}

void SafetyManager::performSensorReset() {
  // SENSOR RESET INITIATED! debug disabled

  // This would reset sensor states
  // Implementation depends on sensor manager integration
}
