#ifndef SAFETY_MANAGER_H
#define SAFETY_MANAGER_H

#include <Arduino.h>
#include "TimerManager.h"

/**
 * SafetyManager Class
 * 
 * Manages all safety mechanisms for the massage chair system.
 * Handles watchdog timer, system health monitoring, and emergency stops.
 * 
 * Features:
 * - Watchdog timer management
 * - System health monitoring
 * - Emergency stop functionality
 * - System stuck detection
 * - Safety timeout management
 */
class SafetyManager {
private:
  // Watchdog configuration
  static const unsigned long WATCHDOG_TIMEOUT = 3000;  // 3 seconds

  // System health timeouts (in ticks)
  static const unsigned long SYSTEM_STUCK_TIMEOUT_TICKS = 2000;    // 20s - increased to avoid false positive during auto case transitions
  static const unsigned long SYSTEM_RECOVERY_TIMEOUT_TICKS = 100;  // 1s

  // System state variables
  bool systemStuck;
  bool emergencyStopActive;
  unsigned long lastSystemActivityTick;
  unsigned long systemStuckStartTick;
  unsigned long lastWatchdogFeedTick;

  // Component references
  TimerManager* timerManager;
  HardwareSerial* debugSerial;

  // System health monitoring
  bool systemHealthCheckActive;
  unsigned long lastHealthCheckTick;
  static const unsigned long HEALTH_CHECK_INTERVAL_TICKS = 100;  // 1 second

public:
  // Constructor
  SafetyManager(TimerManager* timerMgr, HardwareSerial* debugSer = nullptr);

  // Destructor
  ~SafetyManager();

  // Initialization
  void initialize();
  void watchdogInit();

  // Watchdog Management
  void watchdogReset();
  void watchdogFeed();
  bool isWatchdogActive() const;
  unsigned long getLastWatchdogFeedTick() const;

  // System Health Monitoring
  void checkSystemHealth();
  void updateSystemActivity();
  bool isSystemStuck() const;
  bool isEmergencyStopActive() const;
  void setSystemStuck(bool stuck);
  void setEmergencyStopActive(bool active);

  // Emergency Functions
  void emergencyStop();
  void clearEmergencyStop();

  // System Recovery
  void initiateSystemRecovery();
  void completeSystemRecovery();
  bool isSystemRecovering() const;

  // Safety Status
  void printSafetyStatus() const;
  void printSystemHealthStatus() const;

  // System Activity Tracking
  void recordMotorActivity();
  void recordSensorActivity();
  void recordCommunicationActivity();
  void recordSequenceActivity();

  // Safety Configuration
  void setSystemStuckTimeout(unsigned long timeoutTicks);
  void setRecoveryTimeout(unsigned long timeoutTicks);
  void setHealthCheckInterval(unsigned long intervalTicks);

  // System State Getters
  unsigned long getLastSystemActivityTick() const;
  unsigned long getSystemStuckStartTick() const;
  bool getSystemHealthCheckActive() const;
  unsigned long getLastHealthCheckTick() const;

  // System State Setters
  void setLastSystemActivityTick(unsigned long tick);
  void setSystemStuckStartTick(unsigned long tick);
  void setSystemHealthCheckActive(bool active);
  void setLastHealthCheckTick(unsigned long tick);

private:
  // Helper functions
  void detectSystemStuck();
  void handleSystemStuck();
  void performHealthCheck();
  void resetSystemStuckDetection();

  // Watchdog helpers
  void enableWatchdog();
  void disableWatchdog();
  bool isWatchdogEnabled() const;

  // System activity helpers
  bool hasSystemActivity() const;
  unsigned long getTimeSinceLastActivity() const;

  // Safety validation
  bool validateSystemState() const;
  bool isSystemInSafeState() const;

  // Recovery procedures
  void performEmergencyShutdown();
  void performSystemReset();
  void performMotorShutdown();
  void performSensorReset();
};

#endif  // SAFETY_MANAGER_H
