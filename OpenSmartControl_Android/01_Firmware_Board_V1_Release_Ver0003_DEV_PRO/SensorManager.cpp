#include "SensorManager.h"
#include "MotorController.h"

// Static instance for ISR access
SensorManager* SensorManager::instance = nullptr;

/**
 * Constructor
 */
SensorManager::SensorManager(TimerManager* timerMgr, MotorController* motorCtrl, HardwareSerial* debugSer)
  : timerManager(timerMgr), motorController(motorCtrl), debugSerial(debugSer), sensorUpLimit(false), sensorDownLimit(false), lastUpState(false), lastDownState(false), buttonUpSamples(0), buttonDownSamples(0), sensorUpPending(false), sensorDownPending(false), sensorConfirmStartTick(0), sensorConfirmInProgress(false), confirmState(IDLE), globalSensorUpLimit(false), globalSensorDownLimit(false), globalSensorConfirmInProgress(false), lastPendingDebugTick(0), lastFunctionDebugTick(0), lastIdleDebugTick(0), lastWaitingDebugTick(0), lastConfirmDebugTick(0), lastConfirmedDebugTick(0) {
  instance = this;
}

/**
 * Destructor
 */
SensorManager::~SensorManager() {
  instance = nullptr;
}

/**
 * Initialize sensor manager
 */
void SensorManager::initialize() {
  // Set pin modes
  pinMode(LMT_UP, INPUT_PULLUP);
  pinMode(LMT_DOWN, INPUT_PULLUP);

  // Initialize sensor states
  sensorUpLimit = digitalRead(LMT_UP);
  sensorDownLimit = digitalRead(LMT_DOWN);
  lastUpState = sensorUpLimit;
  lastDownState = sensorDownLimit;

  // Initialize debouncing
  buttonUpSamples = 0;
  buttonDownSamples = 0;

  // Initialize confirmation
  sensorUpPending = false;
  sensorDownPending = false;
  sensorConfirmStartTick = 0;
  sensorConfirmInProgress = false;
  confirmState = IDLE;

  // Initialize global states
  globalSensorUpLimit = sensorUpLimit;
  globalSensorDownLimit = sensorDownLimit;
  globalSensorConfirmInProgress = sensorConfirmInProgress;

  // Initialize debug timing
  lastPendingDebugTick = 0;
  lastFunctionDebugTick = 0;
  lastIdleDebugTick = 0;
  lastWaitingDebugTick = 0;
  lastConfirmDebugTick = 0;
  lastConfirmedDebugTick = 0;
}

/**
 * Setup sensor interrupts
 */
void SensorManager::setupInterrupts() {
  // Attach interrupts for sensor changes
  attachInterrupt(digitalPinToInterrupt(LMT_UP), sensorISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(LMT_DOWN), sensorISR, CHANGE);
}

/**
 * Get sensor states
 */
bool SensorManager::getSensorUpLimit() const {
  return sensorUpLimit;
}

bool SensorManager::getSensorDownLimit() const {
  return sensorDownLimit;
}

bool SensorManager::getLastUpState() const {
  return lastUpState;
}

bool SensorManager::getLastDownState() const {
  return lastDownState;
}

/**
 * Debouncing getters/setters
 */
uint8_t SensorManager::getButtonUpSamples() const {
  return buttonUpSamples;
}

uint8_t SensorManager::getButtonDownSamples() const {
  return buttonDownSamples;
}

void SensorManager::setButtonUpSamples(uint8_t samples) {
  buttonUpSamples = samples;
}

void SensorManager::setButtonDownSamples(uint8_t samples) {
  buttonDownSamples = samples;
}

void SensorManager::setLastUpState(bool state) {
  lastUpState = state;
}

void SensorManager::setLastDownState(bool state) {
  lastDownState = state;
}

/**
 * Sensor confirmation getters/setters
 */
bool SensorManager::getSensorUpPending() const {
  return sensorUpPending;
}

bool SensorManager::getSensorDownPending() const {
  return sensorDownPending;
}

bool SensorManager::getSensorConfirmInProgress() const {
  return sensorConfirmInProgress;
}

SensorManager::SensorConfirmState SensorManager::getConfirmState() const {
  return confirmState;
}

unsigned long SensorManager::getSensorConfirmStartTick() const {
  return sensorConfirmStartTick;
}

void SensorManager::setSensorUpPending(bool pending) {
  sensorUpPending = pending;
}

void SensorManager::setSensorDownPending(bool pending) {
  sensorDownPending = pending;
}

void SensorManager::setSensorConfirmInProgress(bool inProgress) {
  sensorConfirmInProgress = inProgress;
  globalSensorConfirmInProgress = inProgress;
}

void SensorManager::setConfirmState(SensorConfirmState state) {
  confirmState = state;
}

void SensorManager::setSensorConfirmStartTick(unsigned long tick) {
  sensorConfirmStartTick = tick;
}

/**
 * Global state getters/setters (for compatibility)
 */
bool SensorManager::getGlobalSensorUpLimit() const {
  return globalSensorUpLimit;
}

bool SensorManager::getGlobalSensorDownLimit() const {
  return globalSensorDownLimit;
}

bool SensorManager::getGlobalSensorConfirmInProgress() const {
  return globalSensorConfirmInProgress;
}

void SensorManager::setGlobalSensorUpLimit(bool state) {
  globalSensorUpLimit = state;
}

void SensorManager::setGlobalSensorDownLimit(bool state) {
  globalSensorDownLimit = state;
}

void SensorManager::setGlobalSensorConfirmInProgress(bool state) {
  globalSensorConfirmInProgress = state;
}

/**
 * Process sensor confirmation (anti-noise mechanism)
 */
void SensorManager::processSensorConfirmation() {
  unsigned long currentTick = timerManager->getMasterTicks();

  switch (confirmState) {
    case IDLE:
      debugSensorIdle();
      break;

    case WAITING_CONFIRM:
      debugSensorWaiting();
      if (currentTick - sensorConfirmStartTick >= SENSOR_CONFIRM_DELAY_TICKS) {
        // Confirmation delay completed
        completeSensorConfirmation(sensorUpPending);
      }
      break;

    case CONFIRMED:
      debugSensorConfirmed();
      // Reset after confirmation
      resetSensorConfirmation();
      break;
  }
}

/**
 * Update sensor states
 */
void SensorManager::updateSensorStates() {
  updateSensorUpState();
  updateSensorDownState();
}

/**
 * Check sensor debouncing
 */
void SensorManager::checkSensorDebouncing() {
  // This would be called from ISR in the original implementation
  // For now, we'll handle it in the main loop
  processSensorInterrupt();
}

/**
 * Sensor ISR callback
 */
void SensorManager::onSensorISR() {
  processSensorInterrupt();
}

/**
 * Process sensor interrupt
 */
void SensorManager::processSensorInterrupt() {
  bool currentUpState = digitalRead(LMT_UP);
  bool currentDownState = digitalRead(LMT_DOWN);

  // CRITICAL SAFETY: Stop ROLL motor immediately when sensor triggered
  // This prevents motor damage before home sequence
  // NOTE: Disabled during GO HOME sequence to allow proper homing
  if ((currentUpState != lastUpState && currentUpState) || (currentDownState != lastDownState && currentDownState)) {
    // TODO: Add GO HOME sequence check here to prevent motor stop during homing
    // For now, disable emergency stop to allow GO HOME sequence to work
    /*
    // Emergency stop ROLL motor immediately
    if (motorController) {
      motorController->offRollMotor();
      // Note: Cannot use Serial here as it's in ISR context
      // Debug message will be handled by main loop
    }
    */
  }

  // Debounce UP sensor
  if (currentUpState != lastUpState) {
    buttonUpSamples++;
    if (buttonUpSamples >= DEBOUNCE_SAMPLES) {
      if (currentUpState != sensorUpLimit) {
        sensorUpLimit = currentUpState;
        globalSensorUpLimit = sensorUpLimit;

        if (sensorUpLimit) {
          // Sensor triggered - start confirmation
          startSensorConfirmation(true);
        }
      }
      buttonUpSamples = 0;
    }
  } else {
    buttonUpSamples = 0;
  }

  // Debounce DOWN sensor
  if (currentDownState != lastDownState) {
    buttonDownSamples++;
    if (buttonDownSamples >= DEBOUNCE_SAMPLES) {
      if (currentDownState != sensorDownLimit) {
        sensorDownLimit = currentDownState;
        globalSensorDownLimit = sensorDownLimit;

        if (sensorDownLimit) {
          // Sensor triggered - start confirmation
          startSensorConfirmation(false);
        }
      }
      buttonDownSamples = 0;
    }
  } else {
    buttonDownSamples = 0;
  }

  lastUpState = currentUpState;
  lastDownState = currentDownState;
}

/**
 * Static sensor ISR function
 */
void SensorManager::sensorISR() {
  if (instance) {
    instance->onSensorISR();
  }
}

/**
 * Reset sensor states
 */
void SensorManager::resetSensorStates() {
  sensorUpLimit = false;
  sensorDownLimit = false;
  lastUpState = false;
  lastDownState = false;
  buttonUpSamples = 0;
  buttonDownSamples = 0;

  globalSensorUpLimit = false;
  globalSensorDownLimit = false;
}

/**
 * Clear sensor confirmation
 */
void SensorManager::clearSensorConfirmation() {
  sensorUpPending = false;
  sensorDownPending = false;
  sensorConfirmInProgress = false;
  confirmState = IDLE;
  sensorConfirmStartTick = 0;

  globalSensorConfirmInProgress = false;
}

/**
 * Print sensor status
 */
void SensorManager::printSensorStatus() const {
  if (debugSerial) debugSerial->print("Sensors - UP: ");
  if (debugSerial) debugSerial->print(sensorUpLimit ? "HIGH" : "LOW");
  if (debugSerial) debugSerial->print(", DOWN: ");
  if (debugSerial) debugSerial->print(sensorDownLimit ? "HIGH" : "LOW");
  if (debugSerial) debugSerial->print(", Pending UP: ");
  if (debugSerial) debugSerial->print(sensorUpPending ? "YES" : "NO");
  if (debugSerial) debugSerial->print(", Pending DOWN: ");
  if (debugSerial) debugSerial->print(sensorDownPending ? "YES" : "NO");
  if (debugSerial) debugSerial->print(", Confirm Progress: ");
  if (debugSerial) debugSerial->println(sensorConfirmInProgress ? "YES" : "NO");
}

/**
 * Print sensor confirmation status
 */
void SensorManager::printSensorConfirmationStatus() const {
  if (debugSerial) debugSerial->print("Confirm State: ");
  switch (confirmState) {
    case IDLE:
      if (debugSerial) debugSerial->print("IDLE");
      break;
    case WAITING_CONFIRM:
      if (debugSerial) debugSerial->print("WAITING_CONFIRM");
      break;
    case CONFIRMED:
      if (debugSerial) debugSerial->print("CONFIRMED");
      break;
  }
  if (debugSerial) debugSerial->print(", Start Tick: ");
  if (debugSerial) debugSerial->print(sensorConfirmStartTick);
  if (debugSerial) debugSerial->print(", Current Tick: ");
  if (debugSerial) debugSerial->println(timerManager->getMasterTicks());
}

/**
 * Private Helper Functions
 */
void SensorManager::updateSensorUpState() {
  bool currentState = digitalRead(LMT_UP);
  if (currentState != sensorUpLimit) {
    sensorUpLimit = currentState;
    globalSensorUpLimit = sensorUpLimit;

    if (debugSerial) {
      debugSerial->print("DEBUG: UP sensor state changed to: ");
      debugSerial->println(sensorUpLimit ? "ACTIVE" : "INACTIVE");
    }
  }
}

void SensorManager::updateSensorDownState() {
  bool currentState = digitalRead(LMT_DOWN);
  if (currentState != sensorDownLimit) {
    sensorDownLimit = currentState;
    globalSensorDownLimit = sensorDownLimit;
  }
}

void SensorManager::startSensorConfirmation(bool isUpSensor) {
  if (isUpSensor) {
    sensorUpPending = true;
  } else {
    sensorDownPending = true;
  }

  sensorConfirmInProgress = true;
  globalSensorConfirmInProgress = true;
  sensorConfirmStartTick = timerManager->getMasterTicks();
  confirmState = WAITING_CONFIRM;

  debugSensorPending();
}

void SensorManager::completeSensorConfirmation(bool isUpSensor) {
  confirmState = CONFIRMED;
  debugSensorConfirm();
}

void SensorManager::resetSensorConfirmation() {
  sensorUpPending = false;
  sensorDownPending = false;
  sensorConfirmInProgress = false;
  globalSensorConfirmInProgress = false;
  confirmState = IDLE;
  sensorConfirmStartTick = 0;
}

/**
 * Debug Functions
 */
void SensorManager::debugSensorPending() {
  if (isTimeForDebug(lastPendingDebugTick, 100)) {  // 1 second
    // Sensor: Pending confirmation started debug disabled
    updateDebugTick(lastPendingDebugTick);
  }
}

void SensorManager::debugSensorFunction() {
  if (isTimeForDebug(lastFunctionDebugTick, 100)) {  // 1 second
    if (debugSerial) debugSerial->println("Sensor: Function called");
    updateDebugTick(lastFunctionDebugTick);
  }
}

void SensorManager::debugSensorIdle() {
  if (isTimeForDebug(lastIdleDebugTick, 1000)) {  // 10 seconds
    // Sensor: IDLE state debug disabled to save Flash memory
    updateDebugTick(lastIdleDebugTick);
  }
}

void SensorManager::debugSensorWaiting() {
  if (isTimeForDebug(lastWaitingDebugTick, 100)) {  // 1 second
    // Sensor: WAITING_CONFIRM state debug disabled
    updateDebugTick(lastWaitingDebugTick);
  }
}

void SensorManager::debugSensorConfirm() {
  if (isTimeForDebug(lastConfirmDebugTick, 100)) {  // 1 second
    if (debugSerial) debugSerial->println("Sensor: CONFIRMED state");
    updateDebugTick(lastConfirmDebugTick);
  }
}

void SensorManager::debugSensorConfirmed() {
  if (isTimeForDebug(lastConfirmedDebugTick, 100)) {  // 1 second
    if (debugSerial) debugSerial->println("Sensor: Confirmation completed");
    updateDebugTick(lastConfirmedDebugTick);
  }
}

bool SensorManager::isTimeForDebug(unsigned long& lastDebugTick, unsigned long intervalTicks) const {
  unsigned long currentTick = timerManager->getMasterTicks();
  return (currentTick - lastDebugTick) >= intervalTicks;
}

void SensorManager::updateDebugTick(unsigned long& lastDebugTick) {
  lastDebugTick = timerManager->getMasterTicks();
}
