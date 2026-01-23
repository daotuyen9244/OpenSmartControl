#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "TimerManager.h"
#include "PinDefinitions.h"

// Forward declaration
class MotorController;

/**
 * SensorManager Class
 * 
 * Manages all sensor operations for the massage chair system.
 * Handles limit sensors with debouncing and confirmation mechanisms.
 * 
 * Features:
 * - Limit sensor debouncing (30ms)
 * - Sensor confirmation (500ms anti-noise delay)
 * - Sensor state management
 * - Interrupt-based sensor reading
 * - Noise filtering
 */
class SensorManager {
public:
  // Sensor confirmation states
  enum SensorConfirmState {
    IDLE,
    WAITING_CONFIRM,
    CONFIRMED
  };

private:
  // Pin definitions
  static const int LMT_UP = LMT_UP_PIN;
  static const int LMT_DOWN = LMT_DOWN_PIN;

  // Debouncing constants
  static const uint8_t DEBOUNCE_SAMPLES = 3;
  static const unsigned long SENSOR_CONFIRM_DELAY_TICKS = 50;  // 500ms

  // Sensor state variables
  volatile bool sensorUpLimit;
  volatile bool sensorDownLimit;
  volatile bool lastUpState;
  volatile bool lastDownState;

  // Debouncing variables
  volatile uint8_t buttonUpSamples;
  volatile uint8_t buttonDownSamples;

  // Sensor confirmation variables
  volatile bool sensorUpPending;
  volatile bool sensorDownPending;
  unsigned long sensorConfirmStartTick;
  bool sensorConfirmInProgress;
  SensorConfirmState confirmState;

  // Global sensor state variables (for compatibility)
  volatile bool globalSensorUpLimit;
  volatile bool globalSensorDownLimit;
  volatile bool globalSensorConfirmInProgress;

  // Component references
  TimerManager* timerManager;
  MotorController* motorController;
  HardwareSerial* debugSerial;

  // Debug timing
  unsigned long lastPendingDebugTick;
  unsigned long lastFunctionDebugTick;
  unsigned long lastIdleDebugTick;
  unsigned long lastWaitingDebugTick;
  unsigned long lastConfirmDebugTick;
  unsigned long lastConfirmedDebugTick;

public:
  // Constructor
  SensorManager(TimerManager* timerMgr, MotorController* motorCtrl = nullptr, HardwareSerial* debugSer = nullptr);

  // Destructor
  ~SensorManager();

  // Initialization
  void initialize();
  void setupInterrupts();

  // Sensor Reading
  bool getSensorUpLimit() const;
  bool getSensorDownLimit() const;
  bool getLastUpState() const;
  bool getLastDownState() const;

  // Debouncing
  uint8_t getButtonUpSamples() const;
  uint8_t getButtonDownSamples() const;
  void setButtonUpSamples(uint8_t samples);
  void setButtonDownSamples(uint8_t samples);
  void setLastUpState(bool state);
  void setLastDownState(bool state);

  // Sensor Confirmation
  bool getSensorUpPending() const;
  bool getSensorDownPending() const;
  bool getSensorConfirmInProgress() const;
  SensorConfirmState getConfirmState() const;
  unsigned long getSensorConfirmStartTick() const;

  void setSensorUpPending(bool pending);
  void setSensorDownPending(bool pending);
  void setSensorConfirmInProgress(bool inProgress);
  void setConfirmState(SensorConfirmState state);
  void setSensorConfirmStartTick(unsigned long tick);

  // Global state getters/setters (for compatibility)
  bool getGlobalSensorUpLimit() const;
  bool getGlobalSensorDownLimit() const;
  bool getGlobalSensorConfirmInProgress() const;
  void setGlobalSensorUpLimit(bool state);
  void setGlobalSensorDownLimit(bool state);
  void setGlobalSensorConfirmInProgress(bool state);

  // Sensor Processing
  void processSensorConfirmation();
  void updateSensorStates();
  void checkSensorDebouncing();

  // ISR Functions (called from hardware ISR)
  void onSensorISR();
  void processSensorInterrupt();

  // Static ISR function (for hardware interrupt callback)
  static void sensorISR();

  // Sensor State Management
  void resetSensorStates();
  void clearSensorConfirmation();

  // Debug Functions
  void printSensorStatus() const;
  void printSensorConfirmationStatus() const;

  // Static instance for ISR access
  static SensorManager* instance;

private:
  // Helper functions
  void updateSensorUpState();
  void updateSensorDownState();
  void startSensorConfirmation(bool isUpSensor);
  void completeSensorConfirmation(bool isUpSensor);
  void resetSensorConfirmation();

  // Debug helpers
  void debugSensorPending();
  void debugSensorFunction();
  void debugSensorIdle();
  void debugSensorWaiting();
  void debugSensorConfirm();
  void debugSensorConfirmed();

  // Timing helpers
  bool isTimeForDebug(unsigned long& lastDebugTick, unsigned long intervalTicks) const;
  void updateDebugTick(unsigned long& lastDebugTick);
};

#endif  // SENSOR_MANAGER_H
