#include "MotorController.h"

/**
 * Constructor
 */
MotorController::MotorController(TimerManager* timerMgr, HardwareSerial* debugSer)
  : timerManager(timerMgr), debugSerial(debugSer), rl1Running(false), rl2Running(false), rl3Running(false), kneadingRunning(false), compressionRunning(false), rl1StartTick(0), rl2StartTick(0), rl1DelayStartTick(0), rl2DelayStartTick(0), rl1Direction(false), rl2Direction(false), rl3Direction(false), kneadingPWM(0), compressionPWM(0), globalRL3PWMState(false) {
}

/**
 * Destructor
 */
MotorController::~MotorController() {
  emergencyStop();
}

/**
 * Initialize motor controller
 */
void MotorController::initialize() {
  // Set pin modes
  pinMode(RL1_PWM, OUTPUT);
  pinMode(RL1_DIR, OUTPUT);
  pinMode(RL2_PWM, OUTPUT);
  pinMode(RL2_DIR, OUTPUT);
  pinMode(RL3_PWM, OUTPUT);
  pinMode(RL3_DIR, OUTPUT);
  pinMode(FETT_PWM, OUTPUT);
  pinMode(FETK_PWM, OUTPUT);

  // Initialize all outputs to LOW
  digitalWrite(RL1_PWM, LOW);
  digitalWrite(RL1_DIR, LOW);
  digitalWrite(RL2_PWM, LOW);
  digitalWrite(RL2_DIR, LOW);
  digitalWrite(RL3_PWM, LOW);
  digitalWrite(RL3_DIR, HIGH);  // Default direction: DOWN to UP
  digitalWrite(FETT_PWM, LOW);
  digitalWrite(FETK_PWM, LOW);
  
  // if (debugSerial) {
  //   debugSerial->println("MotorController: RL3_DIR set to HIGH (DOWN to UP direction)");
  // }

  // Reset state variables
  rl1Running = false;
  rl2Running = false;
  rl3Running = false;
  kneadingRunning = false;
  compressionRunning = false;
  globalRL3PWMState = false;

  rl1StartTick = 0;
  rl2StartTick = 0;
  rl1DelayStartTick = 0;
  rl2DelayStartTick = 0;

  rl1Direction = false;
  rl2Direction = false;
  rl3Direction = false;

  kneadingPWM = 0;
  compressionPWM = 0;
}

/**
 * RL1 (Recline/Incline) Control
 */
void MotorController::onRecline() {
  // Debug: Check current motor state
  // RECLINE DEBUG disabled to save Flash memory

  // If already running in same direction, just reset timeout timer
  if (rl1Running && rl1Direction == false) {
    rl1StartTick = timerManager->getMasterTicks();  // Reset 60s timeout
    // RECLINE: Already running debug disabled
    return;
  }

  // If motor was stopped but direction is still recline, restart it
  if (!rl1Running && rl1Direction == false) {
    digitalWrite(RL1_PWM, HIGH);
    rl1Running = true;
    rl1StartTick = timerManager->getMasterTicks();
    // RECLINE: Motor restart debug disabled
    return;
  }

  // Start new or restart motor
  setRL1Direction(false);  // Recline direction
  digitalWrite(RL1_PWM, HIGH);
  rl1Running = true;
  rl1StartTick = timerManager->getMasterTicks();

  // if (debugSerial) debugSerial->println("RECLINE: Starting motor");
}

void MotorController::onIncline() {
  // Debug: Check current motor state (disabled to save FLASH)
  // if (debugSerial) {
  //   debugSerial->print("INCLINE DEBUG: rl1Running=");
  //   debugSerial->print(rl1Running ? "TRUE" : "FALSE");
  //   debugSerial->print(", rl1Direction=");
  //   debugSerial->print(rl1Direction ? "TRUE" : "FALSE");
  //   debugSerial->print(", RL1_PWM=");
  //   debugSerial->print(digitalRead(RL1_PWM) ? "HIGH" : "LOW");
  //   debugSerial->println();
  // }

  // If already running in same direction, just reset timeout timer
  if (rl1Running && rl1Direction == true) {
    rl1StartTick = timerManager->getMasterTicks();  // Reset 60s timeout
    // if (debugSerial) debugSerial->println("INCLINE: Already running - timeout reset to 60s");
    return;
  }

  // If motor was stopped but direction is still incline, restart it
  if (!rl1Running && rl1Direction == true) {
    digitalWrite(RL1_PWM, HIGH);
    rl1Running = true;
    rl1StartTick = timerManager->getMasterTicks();
    // if (debugSerial) debugSerial->println("INCLINE: Motor was stopped - restarting");
    return;
  }

  // Start new or restart motor
  setRL1Direction(true);  // Incline direction
  digitalWrite(RL1_PWM, HIGH);
  rl1Running = true;
  rl1StartTick = timerManager->getMasterTicks();

  // if (debugSerial) debugSerial->println("INCLINE: Starting motor");
}

void MotorController::offReclineIncline() {
  if (rl1Running) {
    // if (debugSerial) debugSerial->println("DEBUG: offReclineIncline() called - stopping RL1 motor");
    digitalWrite(RL1_PWM, LOW);
    rl1Running = false;
    rl1StartTick = 0;
  }
}

bool MotorController::isRL1Running() const {
  return rl1Running;
}

unsigned long MotorController::getRL1StartTick() const {
  return rl1StartTick;
}

void MotorController::setRL1StartTick(unsigned long tick) {
  rl1StartTick = tick;
}

/**
 * RL2 (Forward/Backward) Control
 */
void MotorController::onForward() {
  // Debug: Check current motor state (disabled to save FLASH)
  // if (debugSerial) {
  //   debugSerial->print("FORWARD DEBUG: rl2Running=");
  //   debugSerial->print(rl2Running ? "TRUE" : "FALSE");
  //   debugSerial->print(", rl2Direction=");
  //   debugSerial->print(rl2Direction ? "TRUE" : "FALSE");
  //   debugSerial->print(", RL2_PWM=");
  //   debugSerial->print(digitalRead(RL2_PWM) ? "HIGH" : "LOW");
  //   debugSerial->println();
  // }

  // If already running in same direction, just reset timeout timer
  if (rl2Running && rl2Direction == true) {
    rl2StartTick = timerManager->getMasterTicks();  // Reset 60s timeout
    // if (debugSerial) debugSerial->println("FORWARD: Already running - timeout reset to 60s");
    return;
  }

  // If motor was stopped but direction is still forward, restart it
  if (!rl2Running && rl2Direction == true) {
    digitalWrite(RL2_PWM, HIGH);
    rl2Running = true;
    rl2StartTick = timerManager->getMasterTicks();
    // if (debugSerial) debugSerial->println("FORWARD: Motor was stopped - restarting");
    return;
  }

  // Start new or restart motor
  setRL2Direction(true);  // Forward direction
  digitalWrite(RL2_PWM, HIGH);
  rl2Running = true;
  rl2StartTick = timerManager->getMasterTicks();

  // if (debugSerial) debugSerial->println("FORWARD: Starting motor");
}

void MotorController::onBackward() {
  // Debug: Check current motor state (disabled to save FLASH)
  // if (debugSerial) {
  //   debugSerial->print("BACKWARD DEBUG: rl2Running=");
  //   debugSerial->print(rl2Running ? "TRUE" : "FALSE");
  //   debugSerial->print(", rl2Direction=");
  //   debugSerial->print(rl2Direction ? "TRUE" : "FALSE");
  //   debugSerial->print(", RL2_PWM=");
  //   debugSerial->print(digitalRead(RL2_PWM) ? "HIGH" : "LOW");
  //   debugSerial->println();
  // }

  // If already running in same direction, just reset timeout timer
  if (rl2Running && rl2Direction == false) {
    rl2StartTick = timerManager->getMasterTicks();  // Reset 60s timeout
    // if (debugSerial) debugSerial->println("BACKWARD: Already running - timeout reset to 60s");
    return;
  }

  // If motor was stopped but direction is still backward, restart it
  if (!rl2Running && rl2Direction == false) {
    digitalWrite(RL2_PWM, HIGH);
    rl2Running = true;
    rl2StartTick = timerManager->getMasterTicks();
    // if (debugSerial) debugSerial->println("BACKWARD: Motor was stopped - restarting");
    return;
  }

  // Start new or restart motor
  setRL2Direction(false);  // Backward direction
  digitalWrite(RL2_PWM, HIGH);
  rl2Running = true;
  rl2StartTick = timerManager->getMasterTicks();

  // if (debugSerial) debugSerial->println("BACKWARD: Starting motor");
}

void MotorController::offForwardBackward() {
  if (rl2Running) {
    // if (debugSerial) debugSerial->println("DEBUG: offForwardBackward() called - stopping RL2 motor");
    digitalWrite(RL2_PWM, LOW);
    rl2Running = false;
    rl2StartTick = 0;
  }
}

bool MotorController::isRL2Running() const {
  return rl2Running;
}

unsigned long MotorController::getRL2StartTick() const {
  return rl2StartTick;
}

void MotorController::setRL2StartTick(unsigned long tick) {
  rl2StartTick = tick;
}

/**
 * RL3 (Roll Motor) Control
 */
void MotorController::onRollMotor() {
  if (!rl3Running) {
    digitalWrite(RL3_PWM, HIGH);
    rl3Running = true;
    setRL3PWMState(true);
  }
}

void MotorController::offRollMotor() {
  if (rl3Running) {
    digitalWrite(RL3_PWM, LOW);
    rl3Running = false;
    setRL3PWMState(false);
  }
}

void MotorController::setRollDirection(bool direction) {
  rl3Direction = direction;
  digitalWrite(RL3_DIR, direction);
}

void MotorController::runRollUp() {
  setRollDirection(true);  // Direction UP
  onRollMotor();
}

void MotorController::runRollDown() {
  setRollDirection(false);  // Direction DOWN
  onRollMotor();
}

bool MotorController::isRL3Running() const {
  return rl3Running;
}

bool MotorController::getRollDirection() const {
  return rl3Direction;
}

/**
 * Kneading Motor Control
 */
void MotorController::onKneadingMotor() {
  if (!kneadingRunning) {
    setKneadingPWMInternal(255);  // Default 99% PWM
    kneadingRunning = true;
  }
}

void MotorController::offKneadingMotor() {
  if (kneadingRunning) {
    setKneadingPWMInternal(0);
    kneadingRunning = false;
  }
}

void MotorController::setKneadingPWM(uint8_t pwmValue) {
  kneadingPWM = pwmValue;
  if (kneadingRunning) {
    setKneadingPWMInternal(pwmValue);
  }
}

bool MotorController::isKneadingRunning() const {
  return kneadingRunning;
}

uint8_t MotorController::getKneadingPWM() const {
  return kneadingPWM;
}

/**
 * Compression Motor Control
 */
void MotorController::onCompressionMotor() {
  if (!compressionRunning) {
    compressionRunning = true;
    // Use the PWM value that was set by setCompressionPWMByIntensity()
    // If no PWM was set, use HIGH intensity (255) for auto mode compatibility
    if (compressionPWM == 0) {
      setCompressionPWMInternal(255);  // HIGH intensity for auto mode
      // COMPRESSION: Motor ON debug disabled
    } else {
      setCompressionPWMInternal(compressionPWM);  // Use already set PWM value
      // COMPRESSION: Motor ON PWM debug disabled
    }
  }
  // Remove "Motor already running" debug to reduce spam
}

void MotorController::offCompressionMotor() {
  if (compressionRunning) {
    setCompressionPWMInternal(0);
    compressionRunning = false;
    // COMPRESSION: Motor OFF debug disabled
  }
}

void MotorController::setCompressionPWM(uint8_t pwmValue) {
  compressionPWM = pwmValue;
  if (compressionRunning) {
    setCompressionPWMInternal(pwmValue);
  }
}

void MotorController::setCompressionPWMByIntensity(uint8_t intensityLevel) {
  // Map intensity level (0-255) to PWM value (STM32 Arduino uses 0-255 range)
  uint8_t pwmValue = map(intensityLevel, 0, 255, 0, 255);
  
  // Only update if PWM value actually changed
  static uint8_t lastPWMValue = 0;
  if (pwmValue != lastPWMValue) {
    if (debugSerial) {
      debugSerial->print("COMPRESSION PWM: intensityLevel=");
      debugSerial->print(intensityLevel);
      debugSerial->print(" -> PWM=");
      debugSerial->println(pwmValue);
    }
    setCompressionPWM(pwmValue);
    lastPWMValue = pwmValue;
  }
}


bool MotorController::isCompressionRunning() const {
  return compressionRunning;
}

uint8_t MotorController::getCompressionPWM() const {
  return compressionPWM;
}

/**
 * Motor Management
 */
void MotorController::resetRL1RL2() {
  // This function should only handle timeout logic, not turn off motors
  // The timeout logic is already handled in checkMotorTimeouts()
  // This function is called every loop cycle, so it should not turn off motors
}

void MotorController::emergencyStop() {
  if (debugSerial) debugSerial->println("EMERGENCY STOP: Turning off all motors");
  
  offReclineIncline();
  offForwardBackward();
  offRollMotor();
  offKneadingMotor();
  offCompressionMotor();
  
  // Add delay for emergency stop process
  delay(100);  // 100ms delay for emergency stop
  
  if (debugSerial) debugSerial->println("EMERGENCY STOP: All motors turned OFF");
}

void MotorController::checkMotorTimeouts() {
  // Debug: Check if timeout is being checked
  static unsigned long lastDebugTick = 0;
  unsigned long currentTick = timerManager ? timerManager->getMasterTicks() : 0;

  if (currentTick - lastDebugTick >= 1000) {  // Every 10 seconds
    if (debugSerial && (rl1Running || rl2Running)) {
      debugSerial->print("TIMEOUT CHECK: RL1=");
      debugSerial->print(rl1Running ? "RUNNING" : "STOPPED");
      debugSerial->print(", RL2=");
      debugSerial->print(rl2Running ? "RUNNING" : "STOPPED");
      if (rl1Running) {
        debugSerial->print(", RL1_RUN_TIME=");
        debugSerial->print((currentTick - rl1StartTick) * 10);
        debugSerial->print("ms");
      }
      debugSerial->println();
    }
    lastDebugTick = currentTick;
  }

  if (checkRL1RL2Timeout()) {
    if (debugSerial) debugSerial->println("MOTOR TIMEOUT: RL1/RL2 motors stopped after 60s");
    resetRL1RL2();
  }
}

/**
 * Motor State Getters
 */
bool MotorController::isAnyMotorRunning() const {
  return rl1Running || rl2Running || rl3Running || kneadingRunning || compressionRunning;
}

bool MotorController::isManualMotorRunning() const {
  return rl1Running || rl2Running;
}

bool MotorController::isAutoMotorRunning() const {
  return rl3Running || kneadingRunning || compressionRunning;
}

/**
 * Timing Helpers
 */
unsigned long MotorController::getMotorRunTime(MotorType motorType) const {
  unsigned long currentTick = timerManager->getMasterTicks();

  switch (motorType) {
    case RL1_RECLINE_INCLINE:
      return rl1Running ? (currentTick - rl1StartTick) : 0;
    case RL2_FORWARD_BACKWARD:
      return rl2Running ? (currentTick - rl2StartTick) : 0;
    default:
      return 0;
  }
}

bool MotorController::isMotorTimeout(MotorType motorType) const {
  unsigned long runTime = getMotorRunTime(motorType);

  switch (motorType) {
    case RL1_RECLINE_INCLINE:
    case RL2_FORWARD_BACKWARD:
      return runTime > RL1_RL2_TIMEOUT_TICKS;
    default:
      return false;
  }
}

/**
 * Safety Functions
 */
void MotorController::setMotorTimeout(MotorType motorType, unsigned long timeoutTicks) {
  // Implementation for setting custom timeouts if needed
}

void MotorController::clearMotorTimeout(MotorType motorType) {
  // Implementation for clearing timeouts if needed
}

/**
 * Private Helper Functions
 */
void MotorController::setRL1State(bool running, bool direction) {
  rl1Running = running;
  rl1Direction = direction;
  digitalWrite(RL1_PWM, running ? HIGH : LOW);
  digitalWrite(RL1_DIR, direction ? HIGH : LOW);
}

void MotorController::setRL2State(bool running, bool direction) {
  rl2Running = running;
  rl2Direction = direction;
  digitalWrite(RL2_PWM, running ? HIGH : LOW);
  digitalWrite(RL2_DIR, direction ? HIGH : LOW);
}

void MotorController::setRL3State(bool running, bool direction) {
  rl3Running = running;
  rl3Direction = direction;
  digitalWrite(RL3_PWM, running ? HIGH : LOW);
  digitalWrite(RL3_DIR, direction ? HIGH : LOW);
}

void MotorController::setKneadingState(bool running, uint8_t pwm) {
  kneadingRunning = running;
  kneadingPWM = pwm;
  setKneadingPWMInternal(pwm);
}

void MotorController::setCompressionState(bool running, uint8_t pwm) {
  compressionRunning = running;
  compressionPWM = pwm;
  setCompressionPWMInternal(pwm);
}

void MotorController::setRL1Direction(bool direction) {
  rl1Direction = direction;
  digitalWrite(RL1_DIR, direction ? HIGH : LOW);
}

void MotorController::setRL2Direction(bool direction) {
  rl2Direction = direction;
  digitalWrite(RL2_DIR, direction ? HIGH : LOW);
}

void MotorController::setRL3Direction(bool direction) {
  rl3Direction = direction;
  digitalWrite(RL3_DIR, direction ? HIGH : LOW);
}

/**
 * Global state tracking (for compatibility)
 */
bool MotorController::getRL3PWMState() const {
  return globalRL3PWMState;
}

void MotorController::setRL3PWMState(bool state) {
  globalRL3PWMState = state;
}

void MotorController::setKneadingPWMInternal(uint8_t pwmValue) {
  kneadingPWM = pwmValue;
  analogWrite(FETT_PWM_PIN, pwmValue);
}

void MotorController::setCompressionPWMInternal(uint8_t pwmValue) {
  compressionPWM = pwmValue;
  analogWrite(FETK_PWM_PIN, pwmValue);
  
}

bool MotorController::checkRL1RL2Timeout() const {
  return isMotorTimeout(RL1_RECLINE_INCLINE) || isMotorTimeout(RL2_FORWARD_BACKWARD);
}

void MotorController::handleMotorTimeout(MotorType motorType) {
  switch (motorType) {
    case RL1_RECLINE_INCLINE:
      offReclineIncline();
      break;
    case RL2_FORWARD_BACKWARD:
      offForwardBackward();
      break;
    default:
      break;
  }
}
