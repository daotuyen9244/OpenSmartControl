#include "Massage_v1_hardware.h"

///////////////////////////////////////////////// TIMING VARIABLES - TIMER BASED /////////////////////////////////////////////////
// Master timer tick counter (incremented every 10ms in ISR)
volatile unsigned long timerTicks = 0;

// High precision timer tick counter (incremented every 1ms in ISR)
volatile unsigned long timerTicks1ms = 0;
volatile unsigned long timerTicks1msStep = 0;  // Step timer for program steps

// Timer-based counters for various functions
unsigned long lastLoopTick = 0;
unsigned long loopCounter = 0;

// Motor timing counters
unsigned long rl1StartTick = 0;
unsigned long rl2StartTick = 0;
unsigned long rl1DelayStartTick = 0;
unsigned long rl2DelayStartTick = 0;

// Home sequence timing
unsigned long homeStartTick = 0;
unsigned long homeStepStartTick = 0;
unsigned long homeLastResetTick = 0;

// Auto mode timing
unsigned long autoLastDirChangeTick = 0;
unsigned long autoModeStartTick = 0;
unsigned long autoModeElapsedTicks = 0;
bool autoModeTimerActive = false;
bool autoTimerStarted = false;  // TRUE = Timer đã bắt đầu (ngay khi nhấn AUTO)

// Auto mode total timer (20 minutes)
unsigned long autoTotalStartTick = 0;    // When total timer started (first auto mode)
unsigned long autoTotalElapsedTicks = 0; // Total elapsed time across all auto modes
bool globalAutoTotalTimerActive = false;       // Total timer is running
bool globalAutoTotalTimerExpired = false;      // Total timer has expired

///////////////////////////////////////////////// SENSOR STATE VARIABLES /////////////////////////////////////////////////
volatile bool state = false;
volatile bool changeDir = false;
volatile bool allowRun = false;
volatile uint8_t buttonUpSamples = 0;
volatile uint8_t buttonDownSamples = 0;
volatile bool lastUpState = false;
volatile bool lastDownState = false;
volatile bool sensorUpLimit = false;
volatile bool sensorDownLimit = false;

// Sensor confirmation for anti-noise (500ms delay)
volatile bool sensorUpPending = false;
volatile bool sensorDownPending = false;
unsigned long sensorConfirmStartTick = 0;
bool sensorConfirmInProgress = false;  // CRITICAL: Prevents motor restart during confirmation
bool directionChanging = false;
unsigned long dirChangeTick = 0;
///////////////////////////////////////////////// MOTOR STATE VARIABLES /////////////////////////////////////////////////
bool RL1running = false, RL2running = false;
unsigned long nowTimeRL1running = 0;
unsigned long nowTimeRL2running = 0;

///////////////////////////////////////////////// MODE FLAGS /////////////////////////////////////////////////
// System control flags
volatile bool homeRun = false;           // TRUE = Home position reached, can run AUTO
volatile bool modeAuto = false;          // TRUE = AUTO mode active (20-min timer running)
volatile bool manualPriority = false;    // TRUE = Manual control active (RL1/RL2 running)

// AUTO program mode flags (each can be TRUE/FALSE independently)
volatile bool autodefaultMode = false;   // Auto default mode (base program)
volatile bool rollSpotMode = false;      // Roll/Spot massage mode (FEATURE)
volatile bool kneadingMode = false;      // Kneading massage mode
volatile bool compressionMode = false;   // Compression massage mode
volatile bool percussionMode = false;    // Percussion massage mode
volatile bool combineMode = false;       // Combine mode (when 2+ programs active)

// FEATURE variables (not program types)
volatile uint8_t intensityLevel = 0;     // Intensity level (0-255) - FEATURE
volatile bool useHighPrecisionTimer = false;  // Use 1ms precision timer for AUTO_DEFAULT

// Program detection based on flags:
// AUTO_ROLL        = rollSpotMode ONLY
// AUTO_KNEADING    = kneadingMode ONLY
// AUTO_COMPRESSION = compressionMode ONLY
// AUTO_PERCUSSION  = percussionMode ONLY
// AUTO_COMBINED    = 2 or more modes TRUE

///////////////////////////////////////////////// HOME SEQUENCE VARIABLES /////////////////////////////////////////////////
byte stephomeRun = 0;

///////////////////////////////////////////////// AUTO MODE VARIABLES /////////////////////////////////////////////////
AutoProgram currentAutoProgram = AUTO_NONE;     // Current running program
AutoProgram previousAutoProgram = AUTO_NONE;    // Previous program (for tracking)
bool resetProgramStatesFlag = false;            // Reset flag for static variables
unsigned long programSwitchCount = 0;           // How many times program switched
unsigned long lastProgramSwitchTick = 0;        // When last switch occurred

///////////////////////////////////////////////// SAFETY VARIABLES /////////////////////////////////////////////////
bool systemStuck = false;

///////////////////////////////////////////////// GLOBAL MOTOR STATE VARIABLES /////////////////////////////////////////////////
// globalMotorRunning removed - use globalRL3PWMState instead
bool globalWaitingForSensor = false;  // Global variable for waiting for sensor state
bool globalSensorUpLimit = false;  // Global variable for sensor UP limit state
bool globalSensorDownLimit = false;  // Global variable for sensor DOWN limit state
bool globalSensorConfirmInProgress = false;  // Global variable for sensor confirmation in progress state
bool globalMotorStarted = false;  // Global variable for motor started state
bool globalRL3PWMState = false;  // Global variable for RL3_PWM state

///////////////////////////////////////////////// PROGRAM STATE VARIABLES /////////////////////////////////////////////////
bool globalResetProgramStatesFlag = false;  // Global variable for reset program states flag
bool globalDirectionChanging = false;  // Global variable for direction changing state
unsigned long globalDirChangeTick = 0;  // Global variable for direction change tick
bool globalAllowRun = false;  // Global variable for allow run state
bool globalModeAuto = false;  // Global variable for mode auto state
bool globalHomeRun = false;  // Global variable for home run state
// DEBUG: This will be printed when the variable is initialized
// mySerial.println(">>> DEBUG: globalHomeRun initialized to FALSE");
bool globalRollSpotMode = false;  // Global variable for roll spot mode state
bool globalChangeDir = false;  // Global variable for change direction state

///////////////////////////////////////////////// STATIC VARIABLES CONVERTED TO GLOBAL /////////////////////////////////////////////////
// Sensor confirmation variables
bool globalPreviousUpState = false;
bool globalPreviousDownState = false;
bool globalRl1WaitingForDelay = false;
bool globalRl2WaitingForDelay = false;

// Home sequence variables
unsigned long globalLastDelayMsg = 0;
unsigned long globalLastSearchDebug = 0;
bool globalMotorStartedLocal = false;
unsigned long globalLastBlockMsg = 0;
bool globalMotorDownStarted = false;
unsigned long globalLastDownDelayMsg = 0;

// Sensor confirmation state
SensorConfirmState globalConfirmState = IDLE;
bool globalIsUpSensor = false;
unsigned long globalLastPendingDebugTick = 0;
unsigned long globalLastFunctionDebugTick = 0;
unsigned long globalLastIdleDebugTick = 0;
unsigned long globalLastWaitingDebugTick = 0;
unsigned long globalLastConfirmDebugTick = 0;
unsigned long globalLastConfirmedDebugTick = 0;

// Program sequence started flags
bool globalAutoSequenceStarted = false;
bool globalKneadingSequenceStarted = false;
bool globalCompressionSequenceStarted = false;
bool globalPercussionSequenceStarted = false;
bool globalCombinedSequenceStarted = false;

// Debug timing variables
unsigned long globalLastKneadingDebugTick = 0;
unsigned long globalLastCompressionDebugTick = 0;
unsigned long globalLastPercussionDebugTick = 0;
unsigned long globalLastCombinedDebugTick = 0;
unsigned long globalLastExpirationTick = 0;
unsigned long globalLastDebugTick = 0;
unsigned long globalLastTotalDebugTick = 0;

// AUTO sequence state management
AutoSequenceState currentAutoSequenceState = AUTO_CASE_0;
unsigned long autoSequenceStartTick = 0;
unsigned long autoStopStartTick = 0;
bool autoStopped = false;

// KNEADING sequence state management
KneadingSequenceState currentKneadingSequenceState = KNEADING_CASE_0;
unsigned long kneadingSequenceStartTick = 0;
unsigned long kneadingStopStartTick = 0;
bool kneadingStopped = false;

// COMPRESSION sequence state management
CompressionSequenceState currentCompressionSequenceState = COMPRESSION_CASE_0;
unsigned long compressionSequenceStartTick = 0;
unsigned long compressionStopStartTick = 0;
bool compressionStopped = false;

// PERCUSSION sequence state management
PercussionSequenceState currentPercussionSequenceState = PERCUSSION_CASE_0;
unsigned long percussionSequenceStartTick = 0;
unsigned long percussionStopStartTick = 0;
bool percussionStopped = false;

// COMBINED sequence state management
CombinedSequenceState currentCombinedSequenceState = COMBINED_CASE_0;
unsigned long combinedSequenceStartTick = 0;
unsigned long combinedStopStartTick = 0;
bool combinedStopped = false;

void serialInit() {
  mySerial.begin(115200);
  mySerial2.begin(9600);
}
void outputInit() {

  pinMode(FETT_PWM, OUTPUT);
  pinMode(FETK_PWM, OUTPUT);
  digitalWrite(FETT_PWM, LOW);
  digitalWrite(FETK_PWM, LOW);


  pinMode(RL3_PWM, OUTPUT);
  pinMode(RL3_DIR, OUTPUT);

  pinMode(RL2_PWM, OUTPUT);
  pinMode(RL2_DIR, OUTPUT);

  pinMode(RL1_PWM, OUTPUT);
  pinMode(RL1_DIR, OUTPUT);


  digitalWrite(RL3_PWM, LOW);
setRL3PWMState(false);
  digitalWrite(RL3_DIR, LOW);

  digitalWrite(RL2_PWM, LOW);
  digitalWrite(RL2_DIR, LOW);

  digitalWrite(RL1_PWM, LOW);
  digitalWrite(RL1_DIR, LOW);

  digitalWrite(FETT_PWM, LOW);

  // Initialize HM10 BREAK pin
  pinMode(HM10_BREAK, OUTPUT);
  digitalWrite(HM10_BREAK, HIGH);  // Normal state (HIGH = not reset)
}
void limitSensorInit() {
  pinMode(LMT_UP, INPUT_PULLUP);
  pinMode(LMT_DOWN, INPUT_PULLUP);
  // Initialize states
  //lastUpState = (digitalRead(LMT_UP)==HIGH);
  //lastDownState = (digitalRead(LMT_DOWN)==HIGH);
  lastUpState = HIGH;
  lastDownState = HIGH;
  // Configure timer
  MyTim->setOverflow(SAMPLE_INTERVAL * 1000, MICROSEC_FORMAT);  // 10ms
  MyTim->attachInterrupt(timerCheckLimitSensorISR);
  MyTim->resume();
}
///////////////////////////////////////////////// TIMER ISR - 10ms TICK /////////////////////////////////////////////////
// This ISR runs every 10ms and handles:
// 1. Master timer tick counter
// 2. Sensor debouncing
void timerCheckLimitSensorISR() {
  // Increment master timer tick (10ms per tick)
  timerTicks++;
  
  // Sample LMT_UP with debouncing
  bool currentUpState = digitalRead(LMT_UP);
  
  if (currentUpState == getLastUpState()) {
    setButtonUpSamples(getButtonUpSamples() + 1);
    if (getButtonUpSamples() >= DEBOUNCE_SAMPLES) {
      // State đã ổn định, kiểm tra edge transition
      bool previousUpState = getGlobalPreviousUpState();
      if (currentUpState && !previousUpState) {
        // Rising edge - limit triggered
        mySerial.println("Sensor LMT_UP active - TRIGGERED");
        mySerial.print("DEBUG ISR: UP sensor triggered at tick ");
        mySerial.println(timerTicks);
        setSensorUpPending(true);  // Set pending flag instead of direct action
        setSensorUpLimit(true);    // Set sensor state directly to TRUE
      } else if (!currentUpState && previousUpState) {
        // Falling edge - sensor released
        setSensorUpLimit(false);   // Set sensor state directly to FALSE
      }
      setGlobalPreviousUpState(currentUpState);
      setButtonUpSamples(DEBOUNCE_SAMPLES);  // Giữ ở mức max
    }
  } else {
    setButtonUpSamples(0);  // Reset counter khi state thay đổi
    setLastUpState(currentUpState);
  }
  
  // Sample LMT_DOWN - Enable when in AUTO mode
  // Note: Not needed for GO HOME (home position is 2s down from sensor UP)
  if (getHomeRun()) {
    bool currentDownState = digitalRead(LMT_DOWN);
    if (currentDownState == getLastDownState()) {
      setButtonDownSamples(getButtonDownSamples() + 1);
      if (getButtonDownSamples() >= DEBOUNCE_SAMPLES) {
        bool previousDownState = getGlobalPreviousDownState();
        if (currentDownState && !previousDownState) {
          mySerial.println("Sensor LMT_DOWN active - TRIGGERED");
          mySerial.print("DEBUG ISR: DOWN sensor triggered at tick ");
          mySerial.println(timerTicks);
          setSensorDownPending(true);  // Set pending flag instead of direct action
          setSensorDownLimit(true);    // Set sensor state directly to TRUE
        } else if (!currentDownState && previousDownState) {
          // Falling edge - sensor released
          setSensorDownLimit(false);  // Set sensor state directly to FALSE
        }
        setGlobalPreviousDownState(currentDownState);
        setButtonDownSamples(DEBOUNCE_SAMPLES);
      }
    } else {
      setButtonDownSamples(0);
      setLastDownState(currentDownState);
    }
  }
}

///////////////////////////////////////////////// TIMER 1MS ISR - HIGH PRECISION /////////////////////////////////////////////////
// This ISR runs every 1ms for high precision timing operations
void timer1msISR() {
  // Increment high precision timer tick (1ms per tick)
  timerTicks1ms++;
  
  // Increment step timer for program steps (1ms per tick)
  timerTicks1msStep++;
  
  // Add any high precision operations here if needed
  // For example: PWM modulation, precise motor control, etc.
}

///////////////////////////////////////////////// WATCHDOG TIMER - OPTIMIZED /////////////////////////////////////////////////
void watchdogInit() {
  // Calculate reload value: Reload = Timeout_ms / 8
  // LSI = 32kHz, Prescaler = 256
  uint32_t reload_value = WATCHDOG_TIMEOUT / 8;
  if (reload_value > 4095) reload_value = 4095;
  if (reload_value < 1) reload_value = 1;
  
  #ifdef HAL_IWDG_MODULE_ENABLED
  IWDG_HandleTypeDef hiwdg;
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = reload_value;
    HAL_IWDG_Init(&hiwdg);
  #else
  RCC->CSR |= RCC_CSR_LSION;
  while (!(RCC->CSR & RCC_CSR_LSIRDY));
  IWDG->KR = 0x5555;
  IWDG->PR = 0x06;
  IWDG->RLR = reload_value;
  IWDG->KR = 0xCCCC;
  #endif
}

///////////////////////////////////////////////// SAFETY MECHANISM /////////////////////////////////////////////////

///////////////////////////////////////////////// SYSTEM HEALTH MONITORING - TIMER BASED /////////////////////////////////////////////////
void checkSystemHealth() {
  setLoopCounter(getLoopCounter() + 1);
  
  // Detect stuck system (5s = 500 ticks)
  if ((timerTicks - getLastLoopTick()) > SYSTEM_STUCK_TIMEOUT_TICKS) {
    if (!getSystemStuck()) {
      setSystemStuck(true);
      emergencyStop();
    }
  } else if (getSystemStuck() && ((timerTicks - getLastLoopTick()) < SYSTEM_RECOVERY_TIMEOUT_TICKS)) {
    // System recovered (1s = 100 ticks)
      setSystemStuck(false);
  }
  
  setLastLoopTick(timerTicks);
}

///////////////////////////////////////////////// SAFETY - EMERGENCY STOP /////////////////////////////////////////////////
void emergencyStop() {
  mySerial.println("!!! EMERGENCY STOP !!!");
  
  // Stop all motors immediately
  digitalWrite(RL1_PWM, LOW);
  digitalWrite(RL2_PWM, LOW);
  digitalWrite(RL3_PWM, LOW);
setRL3PWMState(false);
  digitalWrite(FETT_PWM, LOW);
  digitalWrite(FETK_PWM, LOW);
  
  // Reset all mode flags using resetAllModes
  resetAllModes();
  
  // Additional emergency stop specific resets
  setAllowRun(false);
  setAllowRun(false);  // Sync global variable
  
  // Clear sensor confirmation flags
  setSensorConfirmInProgress(false);
  setSensorUpPending(false);
  setSensorDownPending(false);
  
  mySerial.println("System in safe state - Manual intervention may be required");
}

///////////////////////////////////////////////// ROLL MOTOR CONTROL /////////////////////////////////////////////////
void onRollMotor() {
  digitalWrite(RL3_PWM, HIGH);
setRL3PWMState(true);
}

void offRollMotor() {
  digitalWrite(RL3_PWM, LOW);
setRL3PWMState(false);
}

///////////////////////////////////////////////// MOTOR CONTROL - TIMER BASED (NO DELAY) /////////////////////////////////////////////////
// State flags for non-blocking delays
bool rl1WaitingForDelay = getGlobalRl1WaitingForDelay();
bool rl2WaitingForDelay = getGlobalRl2WaitingForDelay();

void onRecline() {
  setManualPriority(true);
  
  // If already running in same direction, just reset timeout timer
  // CRITICAL: Must check BOTH RL1running flag AND PWM pin state
  if (getRL1running() && digitalRead(RL1_DIR) == HIGH && digitalRead(RL1_PWM) == HIGH) {
    setRl1StartTick(timerTicks);  // Reset 20s timeout
    mySerial.println("RECLINE: Already running - timeout reset to 20s");
    return;
  }
  
  // Start new or restart motor
  digitalWrite(RL1_DIR, HIGH);
  digitalWrite(RL1_PWM, LOW);
  setRl1DelayStartTick(timerTicks);
    setGlobalRl1WaitingForDelay(true);
  
  if (getRL1running() && digitalRead(RL1_DIR) == HIGH) {
    mySerial.println("RECLINE: Motor was stopped - restarting");
  } else {
    mySerial.println("RECLINE: Starting motor");
  }
}

void onIncline() {
  setManualPriority(true);
  
  // If already running in same direction, just reset timeout timer
  // CRITICAL: Must check BOTH RL1running flag AND PWM pin state
  if (getRL1running() && digitalRead(RL1_DIR) == LOW && digitalRead(RL1_PWM) == HIGH) {
    setRl1StartTick(timerTicks);  // Reset 20s timeout
    mySerial.println("INCLINE: Already running - timeout reset to 20s");
    return;
  }
  
  // Start new or restart motor
  digitalWrite(RL1_DIR, LOW);
  digitalWrite(RL1_PWM, LOW);
  setRl1DelayStartTick(timerTicks);
    setGlobalRl1WaitingForDelay(true);
  
  if (getRL1running() && digitalRead(RL1_DIR) == LOW) {
    mySerial.println("INCLINE: Motor was stopped - restarting");
  } else {
    mySerial.println("INCLINE: Starting motor");
  }
}

void onBackward() {
  setManualPriority(true);
  
  // If already running in same direction, just reset timeout timer
  // CRITICAL: Must check BOTH RL2running flag AND PWM pin state
  if (getRL2running() && digitalRead(RL2_DIR) == HIGH && digitalRead(RL2_PWM) == HIGH) {
    setRl2StartTick(timerTicks);  // Reset 20s timeout
    mySerial.println("BACKWARD: Already running - timeout reset to 20s");
    return;
  }
  
  // Start new or restart motor
  digitalWrite(RL2_DIR, HIGH);
  digitalWrite(RL2_PWM, LOW);
  setRl2DelayStartTick(timerTicks);
    setGlobalRl2WaitingForDelay(true);
  
  if (getRL2running() && digitalRead(RL2_DIR) == HIGH) {
    mySerial.println("BACKWARD: Motor was stopped - restarting");
  } else {
    mySerial.println("BACKWARD: Starting motor");
  }
}

void onForward() {
  setManualPriority(true);
  
  // If already running in same direction, just reset timeout timer
  // CRITICAL: Must check BOTH RL2running flag AND PWM pin state
  if (getRL2running() && digitalRead(RL2_DIR) == LOW && digitalRead(RL2_PWM) == HIGH) {
    setRl2StartTick(timerTicks);  // Reset 20s timeout
    mySerial.println("FORWARD: Already running - timeout reset to 20s");
    return;
  }
  
  // Start new or restart motor
  digitalWrite(RL2_DIR, LOW);
  digitalWrite(RL2_PWM, LOW);
  setRl2DelayStartTick(timerTicks);
    setGlobalRl2WaitingForDelay(true);
  
  if (getRL2running() && digitalRead(RL2_DIR) == LOW) {
    mySerial.println("FORWARD: Motor was stopped - restarting");
  } else {
    mySerial.println("FORWARD: Starting motor");
  }
}

///////////////////////////////////////////////// MOTOR OFF FUNCTIONS - RELEASE COMMANDS /////////////////////////////////////////////////

// RELEASE command for RECLINE/INCLINE → Stop RL1 motor and reset state
void offReclineIncline() {
  digitalWrite(RL1_PWM, LOW);    // Stop motor immediately
  setRL1running(false);             // Reset state flag
  setManualPriority(false);         // Clear manual priority
  mySerial.println("RL1 Motor stopped - State reset");
}

// RELEASE command for FORWARD/BACKWARD → Stop RL2 motor and reset state
void offForwardBackward() {
  digitalWrite(RL2_PWM, LOW);    // Stop motor immediately
  setRL2running(false);             // Reset state flag
  setManualPriority(false);         // Clear manual priority
  mySerial.println("RL2 Motor stopped - State reset");
}

///////////////////////////////////////////////// KNEADING MOTOR CONTROL (FETT_PWM) /////////////////////////////////////////////////
void onKneadingMotor() {
  // Set PWM to 99% (252/255) for KNEADING
  setKneadingPWM(252);
  mySerial.println("KNEADING: Motor ON (PWM 99%)");
}

void offKneadingMotor() {
  digitalWrite(FETT_PWM, LOW);
  mySerial.println("KNEADING: Motor OFF");
}

void setKneadingPWM(uint8_t pwmValue) {
  // Clamp value to 0-255 range
  if (pwmValue > 255) pwmValue = 255;
  
  // Set PWM value (0-255)
  analogWrite(FETT_PWM, pwmValue);
  
  // Debug output
  uint8_t percentage = (pwmValue * 100) / 255;
  mySerial.print("KNEADING: PWM set to ");
  mySerial.print(percentage);
  mySerial.print("% (");
  mySerial.print(pwmValue);
  mySerial.println("/255)");
}

///////////////////////////////////////////////// COMPRESSION MOTOR CONTROL (FETK_PWM) /////////////////////////////////////////////////
void onCompressionMotor() {
  // Set PWM based on intensity level
  setCompressionPWMByIntensity();
  mySerial.println("COMPRESSION: Motor ON");
}

void offCompressionMotor() {
  digitalWrite(FETK_PWM, LOW);
  mySerial.println("COMPRESSION: Motor OFF");
}

void setCompressionPWM(uint8_t pwmValue) {
  // Clamp value to 0-255 range
  if (pwmValue > 255) pwmValue = 255;
  
  // Set PWM value (0-255)
  analogWrite(FETK_PWM, pwmValue);
  
  // Debug output
  uint8_t percentage = (pwmValue * 100) / 255;
  mySerial.print("COMPRESSION: PWM set to ");
  mySerial.print(percentage);
  mySerial.print("% (");
  mySerial.print(pwmValue);
  mySerial.println("/255)");
}

void setCompressionPWMByIntensity() {
  uint8_t pwmValue;
  
  // INTENSITY LOW -> PWM 60% (153/255)
  // INTENSITY HIGH -> PWM 99% (252/255)
  if (getIntensityLevel() == 0) {
    pwmValue = 153;  // 60% = 153/255
    mySerial.println("COMPRESSION: Intensity LOW -> PWM 60%");
  } else {
    pwmValue = 252;  // 99% = 252/255
    mySerial.println("COMPRESSION: Intensity HIGH -> PWM 99%");
  }
  
  setCompressionPWM(pwmValue);
}

///////////////////////////////////////////////// MOTOR TIMEOUT MANAGEMENT - TIMER BASED /////////////////////////////////////////////////
void resetRL1RL2() {
  // Handle non-blocking delay for RL1 (100ms = 10 ticks)
  if (rl1WaitingForDelay) {
    if ((timerTicks - rl1DelayStartTick) >= MOTOR_DIR_CHANGE_TICKS) {
      digitalWrite(RL1_PWM, HIGH);
      setRL1running(true);
      setRl1StartTick(timerTicks);
      setGlobalRl1WaitingForDelay(false);
    }
  }
  
  // Handle non-blocking delay for RL2 (100ms = 10 ticks)
  if (rl2WaitingForDelay) {
    if ((timerTicks - rl2DelayStartTick) >= MOTOR_DIR_CHANGE_TICKS) {
  digitalWrite(RL2_PWM, HIGH);
  setRL2running(true);
      setRl2StartTick(timerTicks);
      setGlobalRl2WaitingForDelay(false);
    }
  }
  
  // Check RL1 timeout (20s = 2000 ticks)
  if (getRL1running() && ((timerTicks - getRl1StartTick()) > RL1_RL2_TIMEOUT_TICKS)) {
      digitalWrite(RL1_PWM, LOW);
      setRL1running(false);
      setManualPriority(false);
    }
  
  // Check RL2 timeout (20s = 2000 ticks)
  if (getRL2running() && ((timerTicks - getRl2StartTick()) > RL1_RL2_TIMEOUT_TICKS)) {
      digitalWrite(RL2_PWM, LOW);
      setRL2running(false);
      setManualPriority(false);
    }
  }
///////////////////////////////////////////////// HOME SEQUENCE - SIMPLIFIED /////////////////////////////////////////////////
/*
 * GO HOME SEQUENCE - MOTOR CONTROL:
 * 
 * Sequence để điều khiển motor:
 *   1. offRollMotor() → PWM = LOW
 *   2. digitalWrite(RL3_DIR, HIGH/LOW) → Set direction
 *   3. Delay 100ms (để motor settle)
 *   4. onRollMotor() → PWM = HIGH
 * 
 * Quy trình GO HOME:
 * 
 * TH1: Không có UP sensor (middle hoặc DOWN)
 *   1. Dừng motor
 *   2. Direction UP (HIGH) + delay 100ms
 *   3. Motor quay lên tìm sensor UP
 *   4. Chạm UP sensor → Dừng 500ms (processSensorConfirmation)
 *   5. Direction DOWN + delay 100ms
 *   6. Chạy xuống 2s → Home complete
 * 
 * TH2: Đã ở UP sensor
 *   1. Dừng motor
 *   2. Direction DOWN (LOW)
 *   3. Delay 500ms
 *   4. Delay 100ms + Chạy xuống 2s → Home complete
 */
void processGoHome() {
  // Early return if already homed
  if (getHomeRun()) {
    return;  // Already homed, no need to process
  }
  
  // Early return if in AUTO mode (home already completed)
  if (getModeAuto()) {
    return;  // AUTO mode running, home already done
  }
  
  // Early return if not allowed to run
  if (!getAllowRun()) {
    return;  // System not ready
  }
  
  
  // Home sequence state machine
  switch (stephomeRun) {
    case HOME_IDLE: {
      mySerial.println("=== HOME: Starting GO HOME sequence ===");
      
      // Bước 1: Dừng motor Roll
      controlRollMotor(MOTOR_STOP, DOWN_TO_UP);
      mySerial.println("1. Motor Roll stopped");
      
      // Clear all sensor flags
      setSensorConfirmInProgress(false);
      sensorUpPending = false;
      sensorDownPending = false;
      setSensorUpLimit(false);
      setSensorDownLimit(false);
      
      // Check initial sensor states
      bool sensorUpActive = digitalRead(LMT_UP);
      bool sensorDownActive = digitalRead(LMT_DOWN);
      
      mySerial.println("Checking sensors:");
      mySerial.print("  LMT_UP:   "); mySerial.println(sensorUpActive ? "ACTIVE" : "inactive");
      mySerial.print("  LMT_DOWN: "); mySerial.println(sensorDownActive ? "ACTIVE" : "inactive");
      
      // ═══════════════════════════════════════════════
      // TH2: UP sensor đang tác động
      // ═══════════════════════════════════════════════
      if (sensorUpActive) {
        mySerial.println("TH2: UP sensor active - Start delay 500ms");
        controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor, direction DOWN
        setHomeStepStartTick(timerTicks);
        setStephomeRun(HOME_DELAY_AT_UP);
        mySerial.println("3. Waiting 500ms delay...");
      }
      // ═══════════════════════════════════════════════
      // TH1: Sensor floating hoặc DOWN tác động
      // ═══════════════════════════════════════════════
      else {
        mySerial.println("TH1: No UP sensor - Search for UP sensor");
        controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor, direction UP
        setHomeStepStartTick(timerTicks);
        setStephomeRun(HOME_SEARCHING_UP);
        mySerial.println("3. Searching for UP sensor...");
      }
    }
    break;
      
    case HOME_DELAY_AT_UP: {
      // TH2: Waiting 500ms at UP sensor before running down
      unsigned long delayElapsed = timerTicks - homeStepStartTick;
      
      if (delayElapsed >= SENSOR_CONFIRM_DELAY_TICKS) {
        mySerial.println("4. 500ms delay complete");
        mySerial.println("5. Direction set DOWN");
        
        controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor, direction DOWN
        
        // Clear sensor flags (keep UP limit for home sequence)
        setSensorDownLimit(false);
        setSensorConfirmInProgress(false);
        
        setHomeStepStartTick(timerTicks);
        setStephomeRun(HOME_RUNNING_DOWN);
        mySerial.println("6. Motor will run DOWN for 2 seconds (after 100ms delay)...");
      } else {
        // Show progress
        unsigned long lastDelayMsg = getGlobalLastDelayMsg();
        if ((timerTicks - lastDelayMsg) >= 50) {
          mySerial.print("Waiting 500ms delay... ");
          mySerial.print(delayElapsed * 10);
          mySerial.println(" ms");
          setGlobalLastDelayMsg(timerTicks);
        }
      }
    }
    break;
      
    case HOME_SEARCHING_UP: {
      // TH1: Motor quay UP, searching for UP sensor
      unsigned long lastSearchDebug = getGlobalLastSearchDebug();
      bool motorStartedLocal = getGlobalMotorStartedLocal();
      
      // Wait 100ms after direction change before starting motor
      unsigned long delayElapsed = timerTicks - homeStepStartTick;
      
      // Debug every 1 second
      if ((timerTicks - lastSearchDebug) >= 100) {
        mySerial.println("--- HOME_SEARCHING_UP Debug ---");
        mySerial.print("  Total elapsed: "); 
        mySerial.print((timerTicks - getHomeStartTick()) * 10);
        mySerial.println(" ms");
        mySerial.print("  Delay since dir change: ");
        mySerial.print(delayElapsed);
        mySerial.println(" ticks");
        mySerial.print("  sensorUpLimit: ");
        mySerial.println(sensorUpLimit ? "TRUE" : "FALSE");
        mySerial.print("  sensorConfirmInProgress: ");
        mySerial.println(sensorConfirmInProgress ? "TRUE (blocking)" : "FALSE");
        mySerial.print("  Motor started: ");
        mySerial.println(getMotorStarted() ? "YES" : "NO");
        
        // Check motor pin states
        mySerial.print("  RL3_PWM pin: ");
        mySerial.println(getRL3PWMState() ? "HIGH (ON)" : "LOW (OFF)");
        mySerial.print("  RL3_DIR pin: ");
        mySerial.println(digitalRead(RL3_DIR) ? "HIGH (UP)" : "LOW (DOWN)");
        
        setGlobalLastSearchDebug(timerTicks);
      }
      
      // Timeout protection (60s)
      if ((timerTicks - getHomeStartTick()) > HOME_TOTAL_TIMEOUT_TICKS) {
    offRollMotor();
        setStephomeRun(HOME_IDLE);
        motorStartedLocal = false;
        mySerial.println("HOME: TIMEOUT - Search failed");
    return;
  }
  
      // TH1 → TH2: When sensor UP confirmed (by processSensorConfirmation 500ms)
      if (getSensorUpLimit()) {
        mySerial.println("4. UP sensor confirmed (after 500ms)");
        mySerial.println("5. Direction set DOWN");
        
        controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor, direction DOWN
        
        // Clear sensor flags (keep UP limit for home sequence)
        // setSensorUpLimit(false);  // Keep UP limit flag for home sequence
        setSensorDownLimit(false);
        setSensorConfirmInProgress(false);
        
        setHomeStepStartTick(timerTicks);
        setStephomeRun(HOME_RUNNING_DOWN);
        motorStartedLocal = false;
        mySerial.println("6. Motor tiếp tục quay DOWN for 2 seconds (after 100ms delay)...");
      } 
      // Wait 100ms before starting motor (like RL1/RL2)
      else if (delayElapsed >= MOTOR_DIR_CHANGE_TICKS && !getSensorConfirmInProgress()) {
        // Run motor UP
        if (!motorStartedLocal) {
          mySerial.println(">>> 100ms delay complete - Starting motor UP NOW");
          setGlobalMotorStartedLocal(true);
        }
        controlRollMotor(MOTOR_RUN, UP_TO_DOWN);  // Start motor UP
      }
      else if (delayElapsed < MOTOR_DIR_CHANGE_TICKS) {
        // Still in delay period
        if (!motorStartedLocal) {
          unsigned long lastDelayMsg = getGlobalLastDelayMsg();
          if ((timerTicks - lastDelayMsg) >= 50) {
            mySerial.print("Waiting for 100ms delay... ");
            mySerial.print(delayElapsed);
            mySerial.println(" ticks");
            setGlobalLastDelayMsg(timerTicks);
          }
        }
      }
      else {
        // Rate limit this message - only print every 1 second to avoid spam
        unsigned long lastBlockMsg = getGlobalLastBlockMsg();
        if ((timerTicks - lastBlockMsg) >= 500) {  // 1000ms = 100 ticks
          mySerial.println("!!! Motor blocked by sensorConfirmInProgress");
          setGlobalLastBlockMsg(timerTicks);
        }
      }
    }
    break;
      
    case HOME_RUNNING_DOWN: {
      // Run DOWN for 2 seconds to reach home position
      bool motorDownStarted = getGlobalMotorDownStarted();
      unsigned long runElapsed = timerTicks - homeStepStartTick;
      
      // Wait 100ms after direction change before starting motor
      if (runElapsed >= MOTOR_DIR_CHANGE_TICKS) {
        // After delay, run motor for 2 seconds
        unsigned long runTime = runElapsed - MOTOR_DIR_CHANGE_TICKS;
        
        if (runTime >= HOME_STEP3_RUN_TICKS) {
          mySerial.print(">>> DEBUG: Home sequence complete - runTime=");
          mySerial.print(runTime);
          mySerial.print(" ticks, required=");
          mySerial.println(HOME_STEP3_RUN_TICKS);
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          setHomeRun(true);
          setStephomeRun(HOME_IDLE);
          motorDownStarted = false;
          
          // Clear all sensor flags when home complete
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          setSensorConfirmInProgress(false);
          mySerial.println(">>> DEBUG: sensorConfirmInProgress reset to FALSE - Home complete");
          
          mySerial.println("7. Motor stopped");
          mySerial.println("=== HOME: Complete! Motor at home position ===");
        } else if (!getSensorConfirmInProgress()) {
          // Motor continues running DOWN
          if (!motorDownStarted) {
            mySerial.println(">>> 100ms delay complete - Motor running DOWN");
            setGlobalMotorDownStarted(true);
          }
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);  // Start motor DOWN
        } else {
          // Debug: Why motor not starting
          static unsigned long lastDebugTick = 0;
          if ((timerTicks - lastDebugTick) >= 100) {  // 100 ticks = 1 second
            mySerial.print(">>> DEBUG: Motor not starting - sensorConfirmInProgress=");
            mySerial.println(getSensorConfirmInProgress() ? "TRUE" : "FALSE");
            lastDebugTick = timerTicks;
          }
        }
      } else {
        // Still in 100ms delay
        unsigned long lastDownDelayMsg = getGlobalLastDownDelayMsg();
        if ((timerTicks - lastDownDelayMsg) >= 50) {
          mySerial.print("Waiting for 100ms delay before DOWN... ");
          mySerial.print(runElapsed);
          mySerial.println(" ticks");
          setGlobalLastDownDelayMsg(timerTicks);
        }
      }
    }
    break;
      
    default:
      offRollMotor();
      setStephomeRun(HOME_IDLE);
      break;
  }
}
///////////////////////////////////////////////// SENSOR CONFIRMATION - ANTI-NOISE /////////////////////////////////////////////////
/*
 * CRITICAL SAFETY FEATURE: Sensor Confirmation with Motor Block
 * 
 * Problem: When sensor triggers, motor must STOP and STAY STOPPED during confirmation.
 * Without this, processGoHome() or processAuto() could restart motor immediately!
 * 
 * Solution:
 * 1. Sensor triggers → Set sensorConfirmInProgress = true
 * 2. Stop motor immediately
 * 3. Wait 500ms for confirmation
 * 4. Re-check sensor (anti-noise)
 * 5. If confirmed → Change direction, clear flag after direction change completes
 * 6. If false trigger → Clear flag immediately
 * 
 * The flag sensorConfirmInProgress BLOCKS motor restart in:
 * - processGoHome() - HOME_SEARCHING_UP and HOME_RUNNING_DOWN
 * - processAuto() - Direction change delay
 */
void processSensorConfirmation() {
  // State machine for sensor confirmation
  SensorConfirmState confirmState = getGlobalConfirmState();
  bool isUpSensor = getGlobalIsUpSensor();  // Track which sensor triggered
  
  // Debug: Print sensor pending states (slower debug)
  unsigned long lastPendingDebugTick = getGlobalLastPendingDebugTick();
  if ((timerTicks - lastPendingDebugTick) >= 100) {  // Every 1 second
    if (sensorUpPending || sensorDownPending) {
      mySerial.print("SENSOR PENDING: UP=");
      mySerial.print(sensorUpPending ? "YES" : "NO");
      mySerial.print(" | DOWN=");
      mySerial.print(sensorDownPending ? "YES" : "NO");
      mySerial.print(" | State=");
      mySerial.println(confirmState == IDLE ? "IDLE" : (confirmState == WAITING_CONFIRM ? "WAITING" : "CONFIRMED"));
      setGlobalLastPendingDebugTick(timerTicks);
    }
  }
  
  // Debug: Print function entry (much slower)
  unsigned long lastFunctionDebugTick = getGlobalLastFunctionDebugTick();
  if ((timerTicks - lastFunctionDebugTick) >= 5000) {  // Every 5 seconds
    mySerial.print("PROCESS_SENSOR: confirmState=");
    mySerial.print(confirmState == IDLE ? "IDLE" : (confirmState == WAITING_CONFIRM ? "WAITING" : "CONFIRMED"));
    mySerial.print(" | sensorUpPending=");
    mySerial.print(sensorUpPending ? "YES" : "NO");
    mySerial.print(" | sensorDownPending=");
    mySerial.println(sensorDownPending ? "YES" : "NO");
    setGlobalLastFunctionDebugTick(timerTicks);
  }
  
  switch (confirmState) {
    case IDLE: {
      // Debug: Only print when state changes or every 10 seconds
      unsigned long lastIdleDebugTick = getGlobalLastIdleDebugTick();
      if ((timerTicks - lastIdleDebugTick) >= 10000) {  // Every 10 seconds
        mySerial.println("  → DEBUG: Processing IDLE state");
        setGlobalLastIdleDebugTick(timerTicks);
      }
      // Check if UP sensor triggered
      if (sensorUpPending) {
        // Stop roll motor immediately
        offRollMotor();
        setSensorConfirmInProgress(true);  // 🛑 BLOCK motor restart
        mySerial.println("LMT_UP: Roll motor STOPPED - Waiting 500ms for sensor confirmation");
        
        // Start confirmation timer
        setSensorConfirmStartTick(timerTicks);
        setGlobalConfirmState(WAITING_CONFIRM);
        setSensorUpPending(false);
        setGlobalIsUpSensor(true);
      }
      // Check if DOWN sensor triggered
      else if (sensorDownPending) {
        // Stop roll motor immediately
        offRollMotor();
        setSensorConfirmInProgress(true);  // 🛑 BLOCK motor restart
        mySerial.println("LMT_DOWN: Roll motor STOPPED - Waiting 500ms for sensor confirmation");
        mySerial.println("  → DEBUG: sensorDownPending was TRUE, starting confirmation");
        mySerial.print("  → DEBUG: homeRun=");
        mySerial.print(getHomeRun() ? "TRUE" : "FALSE");
        mySerial.print(" | stephomeRun=");
        mySerial.println(stephomeRun);
        
        // Start confirmation timer
        setSensorConfirmStartTick(timerTicks);
        setGlobalConfirmState(WAITING_CONFIRM);
        setSensorDownPending(false);
        setGlobalIsUpSensor(false);
      }
      break;
    }
      
    case WAITING_CONFIRM: {
      // Debug: Only print when state changes or every 5 seconds
      unsigned long lastWaitingDebugTick = getGlobalLastWaitingDebugTick();
      if ((timerTicks - lastWaitingDebugTick) >= 5000) {  // Every 5 seconds
        mySerial.println("  → DEBUG: Processing WAITING_CONFIRM state");
        setGlobalLastWaitingDebugTick(timerTicks);
      }
      // Debug: Print confirmation progress every 100ms
      unsigned long lastConfirmDebugTick = getGlobalLastConfirmDebugTick();
      if ((timerTicks - lastConfirmDebugTick) >= 10) {  // Every 100ms
        unsigned long confirmElapsed = timerTicks - sensorConfirmStartTick;
        mySerial.print("SENSOR CONFIRM: ");
        mySerial.print(isUpSensor ? "UP" : "DOWN");
        mySerial.print(" | Elapsed: ");
        mySerial.print(confirmElapsed);
        mySerial.print("/");
        mySerial.print(SENSOR_CONFIRM_DELAY_TICKS);
        mySerial.print(" ticks | ");
        mySerial.print((confirmElapsed * 10));
        mySerial.print("/");
        mySerial.print((SENSOR_CONFIRM_DELAY_TICKS * 10));
        mySerial.println("ms");
        setGlobalLastConfirmDebugTick(timerTicks);
      }
      
      // Wait 500ms (50 ticks)
      if ((timerTicks - sensorConfirmStartTick) >= SENSOR_CONFIRM_DELAY_TICKS) {
        // Re-read sensor to confirm (anti-noise)
        bool sensorStillActive;
        
        if (isUpSensor) {
          sensorStillActive = digitalRead(LMT_UP);
          mySerial.print("LMT_UP: ");
        } else {
          sensorStillActive = digitalRead(LMT_DOWN);
          mySerial.print("LMT_DOWN: ");
        }
        
        if (sensorStillActive) {
          // Sensor confirmed
          mySerial.println("Sensor CONFIRMED");
          mySerial.print("DEBUG: homeRun=");
          mySerial.print(getHomeRun() ? "TRUE" : "FALSE");
          mySerial.print(" | stephomeRun=");
          mySerial.print(stephomeRun);
          mySerial.print(" | HOME_IDLE=");
          mySerial.println(HOME_IDLE);
          mySerial.print("DEBUG: isUpSensor=");
          mySerial.println(isUpSensor ? "TRUE" : "FALSE");
          
          // If in HOME sequence, only set flag (processGoHome handles it)
          if (!getHomeRun() && stephomeRun > HOME_IDLE) {
            if (isUpSensor) {
              setSensorUpLimit(true);  // For processGoHome
              mySerial.println("  → Home sequence: UP sensor confirmed");
            } else {
              setSensorDownLimit(true);  // For processGoHome - HOME COMPLETE
              mySerial.println("  → Home sequence: DOWN sensor confirmed - HOME POSITION!");
            }
          } else {
            // In AUTO mode, set direction change flags
            // If UP sensor → Change to DOWN
            // If DOWN sensor → Change to UP
            mySerial.println("  → DEBUG: In AUTO mode, processing sensor confirmation");
            if (isUpSensor) {
              setRollDirectionState(false);  // DOWN (LOW)
              setSensorUpLimit(true);
              mySerial.println("  → Auto mode: UP sensor hit, will change to DOWN");
            } else {
              setRollDirectionState(true);   // UP (HIGH)
              mySerial.println("  → DEBUG: About to call setSensorDownLimit(true)");
              setSensorDownLimit(true);  // Use setter function
              mySerial.println("  → Auto mode: DOWN sensor hit, will change to UP");
              mySerial.println("  → DEBUG: setSensorDownLimit(true) called");
              mySerial.print("  → DEBUG: getSensorDownLimit() now returns: ");
              mySerial.println(getSensorDownLimit() ? "TRUE" : "FALSE");
            }
            changeDir = true;
            setChangeDir(true);  // Set global flag
          }
          
          setGlobalConfirmState(CONFIRMED);
          // Note: Keep sensorConfirmInProgress = true until direction change completes
        } else {
          // False trigger - ignore
          mySerial.println("Sensor NOT confirmed - False trigger ignored");
          setSensorConfirmInProgress(false);  // ✅ Clear block flag
          setGlobalConfirmState(IDLE);
        }
      }
      break;
    }
      
    case CONFIRMED: {
      // Debug: Only print when state changes or every 5 seconds
      unsigned long lastConfirmedDebugTick = getGlobalLastConfirmedDebugTick();
      if ((timerTicks - lastConfirmedDebugTick) >= 5000) {  // Every 5 seconds
        mySerial.println("  → DEBUG: Processing CONFIRMED state");
        setGlobalLastConfirmedDebugTick(timerTicks);
      }
      // Reset to idle after action completes
      // For AUTO mode: changeDir is cleared by processAuto() which also clears sensorConfirmInProgress
      // For HOME sequence: wait until sensor flags are processed
      if (!getSensorConfirmInProgress()) {
        // Flag already cleared by processAuto() or processGoHome()
        setGlobalConfirmState(IDLE);
      }
      break;
    }
  }
}

///////////////////////////////////////////////// AUTO MODE MANAGEMENT FUNCTIONS /////////////////////////////////////////////////

// Detect which auto program is active based on mode flags
// IMPORTANT: rollSpotMode is a FEATURE (motor control), NOT a program!
//            It should NOT be counted when detecting program type.
AutoProgram detectAutoProgram() {
  // PRIORITY 1: AUTO_DEFAULT has highest priority
  // When autodefaultMode is active, it's always AUTO_DEFAULT (rollSpotMode is auto-enabled)
  if (getAutodefaultMode()) {
    setCombineMode(false);
    return AUTO_DEFAULT;
  }
  
  // Count PROGRAM modes only (rollSpotMode is NOT a program, just a motor feature)
  int activeProgramCount = 0;
  if (getKneadingMode()) activeProgramCount++;
  if (getCompressionMode()) activeProgramCount++;
  if (getPercussionMode()) activeProgramCount++;
  
  // Combined mode (2 or more PROGRAM modes active)
  if (activeProgramCount > 1) {
    setCombineMode(true);
    return AUTO_COMBINED;
  }
  
  // Clear combineMode if not combined
  setCombineMode(false);
  
  // Single program modes
  if (getKneadingMode() && activeProgramCount == 1) {
    return AUTO_KNEADING;
  }
  else if (getCompressionMode() && activeProgramCount == 1) {
    return AUTO_COMPRESSION;
  }
  else if (getPercussionMode() && activeProgramCount == 1) {
    return AUTO_PERCUSSION;
  }
  
  // No program mode active → AUTO_NONE
  // NOTE: rollSpotMode alone doesn't create a program, it's just a motor feature
  return AUTO_NONE;
}

// Start auto mode with 20-minute timer
void startAutoMode() {
  if (!getHomeRun()) {
    mySerial.println("ERROR: Cannot start AUTO mode - Not homed!");
    return;
  }
  
  // Prevent duplicate start - ignore if already running and timer started
  if (getModeAuto() && getAutoTimerStarted()) {
    mySerial.println("AUTO MODE: Already running - Ignoring duplicate start command");
    return;
  }
  
  setModeAuto(true);
  setRollSpotMode(true);  // Enable roll motor for AUTO mode
  
  // ═══════════════════════════════════════════════════════════════════
  // 🕐 BẮT ĐẦU TIMER 20 PHÚT NGAY KHI NHẤN NÚT AUTO
  // ═══════════════════════════════════════════════════════════════════
    setAutoModeStartTick(timerTicks);  // Bắt đầu đếm NGAY
  setAutoModeElapsedTicks(0);
  setAutoModeTimerActive(true);      // Timer đã BẮT ĐẦU
  setAutoTimerStarted(true);         // Đánh dấu đã start (tránh duplicate)
  
  setCurrentAutoProgram(detectAutoProgram());
  
  // CRITICAL: Initialize direction change tick to past (allow immediate start)
  setAutoLastDirChangeTick(0);  // Set to 0 so timeSinceChange will be large
  
  mySerial.println("╔════════════════════════════════════════╗");
  mySerial.println("║       AUTO MODE STARTED                ║");
  mySerial.println("║  🕐 20-MINUTE TIMER STARTED NOW!      ║");
  mySerial.println("╚════════════════════════════════════════╝");
  
  mySerial.print("Program: ");
  switch (currentAutoProgram) {
    case AUTO_DEFAULT: mySerial.println("AUTO RUN DEFAULT (Roll always ON)"); break;
    case AUTO_KNEADING: mySerial.println("KNEADING"); break;
    case AUTO_COMPRESSION: mySerial.println("COMPRESSION"); break;
    case AUTO_PERCUSSION: mySerial.println("PERCUSSION"); break;
    case AUTO_COMBINED: mySerial.println("COMBINED"); break;
    default: mySerial.println("NONE"); break;
  }
  mySerial.print("Roll Motor (SPOT): ");
  mySerial.println(rollSpotMode ? "ON" : "OFF");
  mySerial.print("Start tick: "); mySerial.println(getAutoModeStartTick());
  mySerial.print("autoLastDirChangeTick reset to: "); mySerial.println(getAutoLastDirChangeTick());
  mySerial.println("Duration: 20 minutes");
  mySerial.println("========================");
}

// Stop auto mode and reset timer
void stopAutoMode() {
  mySerial.println("=== AUTO MODE STOPPED ===");
  
  // Calculate run time before stopping
  setAutoModeElapsedTicks(timerTicks - getAutoModeStartTick());
  mySerial.print("Total run time: ");
  mySerial.print(getAutoModeElapsedTicks() * 10 / 1000); // Convert ticks to seconds
  mySerial.println(" seconds");
  
  // ═══════════════════════════════════════════════
  // STOP ALL MOTORS
  // ═══════════════════════════════════════════════
  mySerial.println("Stopping all motors:");
  
  // Roll motor
      offRollMotor();
  mySerial.println("  - Roll motor OFF");
  
  // Kneading motor (if implemented in future)
  // TODO: Add kneading motor control
  mySerial.println("  - Kneading motor OFF (not implemented)");
  
  // Compression motor (if implemented in future)
  // TODO: Add compression motor control
  mySerial.println("  - Compression motor OFF (not implemented)");
  
  // ═══════════════════════════════════════════════
  // RESET ALL MODE FLAGS
  // ═══════════════════════════════════════════════
  mySerial.println("Resetting all mode flags:");
  
  setModeAuto(false);
  setAutodefaultMode(false);
  setRollSpotMode(false);
  setKneadingMode(false);
  setCompressionMode(false);
  setPercussionMode(false);
  setCombineMode(false);
  
  // Reset features
  setIntensityLevel(0);
  mySerial.println("  - Intensity level reset to 0");
  
  // ═══════════════════════════════════════════════
  // RESET TIMER AND PROGRAM STATES
  // ═══════════════════════════════════════════════
  setAutoModeTimerActive(false);
  setAutoTimerStarted(false);        // Reset flag - cho phép start lại AUTO mode
  setCurrentAutoProgram(AUTO_NONE);
  
  // Signal all execute functions to reset their static variables
  setResetProgramStatesFlag(true);
  
  // Reset global motor and sensor states
  setRL3PWMState(false);
  setWaitingForSensor(false);
  setSensorUpLimit(false);
  setSensorDownLimit(false);
  setSensorConfirmInProgress(false);
  setMotorStarted(false);
  
  mySerial.println("All programs stopped, timer reset");
  mySerial.println("Static variables will be reset on next execution");
  mySerial.println("Global states reset to initial values");
  mySerial.println("========================");
}

// Get remaining time in seconds
unsigned long getAutoRemainingTime() {
  if (!getAutoModeTimerActive()) return 0;
  
  unsigned long elapsed = timerTicks - getAutoModeStartTick();
  if (elapsed >= AUTO_MODE_DURATION_TICKS) {
    return 0;
  }
  return (AUTO_MODE_DURATION_TICKS - elapsed) * 10 / 1000; // Convert ticks to seconds
}

// Print auto mode status
void printAutoModeStatus() {
  mySerial.println("=== AUTO MODE STATUS ===");
  mySerial.print("Active: ");
  mySerial.println(autoModeTimerActive ? "YES" : "NO");
  
  if (getAutoModeTimerActive()) {
    mySerial.print("Program: ");
    switch (currentAutoProgram) {
      case AUTO_DEFAULT: mySerial.println("AUTO RUN DEFAULT"); break;
      case AUTO_KNEADING: mySerial.println("KNEADING"); break;
      case AUTO_COMPRESSION: mySerial.println("COMPRESSION"); break;
      case AUTO_PERCUSSION: mySerial.println("PERCUSSION"); break;
      case AUTO_COMBINED: mySerial.println("COMBINED"); break;
      default: mySerial.println("NONE"); break;
    }
    
    unsigned long remaining = getAutoRemainingTime();
    mySerial.print("Remaining: ");
    mySerial.print(remaining / 60);
    mySerial.print("m ");
    mySerial.print(remaining % 60);
    mySerial.println("s");
  }
  mySerial.println("=======================");
}

// Reset all program static variables
void resetProgramStates() {
  mySerial.println(">>> Resetting all program states");
  setResetProgramStatesFlag(true);
  // The flag will be cleared by each execute function when they run
}

// Check if roll motor toggle is allowed (for CMD_ROLL_MOTOR 0x20)
bool isRollMotorToggleAllowed() {
  // Roll motor toggle (SPOT mode) is allowed in:
  // - KNEADING program
  // - COMPRESSION program
  // - PERCUSSION program
  // - COMBINED program
  // 
  // NOT allowed in AUTO_DEFAULT (roll is always ON)
  
  if (!getModeAuto()) {
    return false;  // Not in auto mode
  }
  
  // Check if in AUTO_DEFAULT program (autodefaultMode active)
  if (getAutodefaultMode()) {
    return false;  // AUTO_DEFAULT - roll motor always ON, cannot toggle
  }
  
  // In other programs (KNEADING, COMPRESSION, PERCUSSION, COMBINED) - toggle is allowed
  return true;
}

// Check if intensity change is allowed in current program
bool isIntensityChangeAllowed() {
  // Intensity change is allowed in:
  // - COMPRESSION program
  // - PERCUSSION program
  // - COMBINED program
  // 
  // NOT allowed in:
  // - AUTO_DEFAULT
  // - KNEADING
  
  if (!getModeAuto()) {
    return false;  // Not in auto mode
  }
  
  // Block in AUTO_DEFAULT and KNEADING
  if (getAutodefaultMode() || getKneadingMode()) {
    return false;
  }
  
  // Allowed in COMPRESSION, PERCUSSION, COMBINED
  if (getCompressionMode() || getPercussionMode() || getCombineMode()) {
    return true;
  }
  
  return false;
}

///////////////////////////////////////////////// AUTO PROGRAM EXECUTION FUNCTIONS /////////////////////////////////////////////////

// Common function to check program execution conditions
bool checkProgramConditions() {
  // Check basic conditions
  if (!(getAllowRun() && getModeAuto() && getHomeRun() && getRollSpotMode())) {
    if (getRL3PWMState()) {
      controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
    }
    // Debug: Log why conditions failed
    mySerial.print("PROGRAM: Conditions failed - allowRun:");
    mySerial.print(getAllowRun() ? "T" : "F");
    mySerial.print(" modeAuto:");
    mySerial.print(getModeAuto() ? "T" : "F");
    mySerial.print(" homeRun:");
    mySerial.print(getHomeRun() ? "T" : "F");
    mySerial.print(" rollSpotMode:");
    mySerial.println(getRollSpotMode() ? "T" : "F");
    return false;
  }
  return true;
}

// Execute AUTO_DEFAULT Program - STATE MACHINE
void runAutoDefaultSequence() {
  // Debug message every 1 second
  static unsigned long lastDebugTick = 0;
  if ((timerTicks - lastDebugTick) >= 100) {  // 100 ticks = 1 second
    mySerial.print(">>> DEBUG: runAutoDefaultSequence() - state=");
    mySerial.print(getCurrentAutoSequenceState());
    mySerial.print(", stopped=");
    mySerial.println(getAutoStopped() ? "TRUE" : "FALSE");
    lastDebugTick = timerTicks;
  }
  
  switch (getCurrentAutoSequenceState()) {
    case AUTO_CASE_0: {
      // Case 0: DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 500ms → về Case 0
      
      if (getAutoStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        unsigned long stopElapsed = timerTicks - getAutoStopStartTick();
        if (stopElapsed >= 50) {
          mySerial.print(">>> AUTO Case 0: Starting DOWN_TO_UP (delay=");
          mySerial.print(stopElapsed);
          mySerial.println(" ticks)");
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          mySerial.println(">>> DEBUG: Motor UP started in AUTO Case 0");
          setAutoStopped(false);
          setAutoSequenceStartTick(timerTicks);
        } else {
          // Debug delay progress
          static unsigned long lastDelayTick = 0;
          if ((timerTicks - lastDelayTick) >= 50) {  // Every 500ms
            mySerial.print(">>> DEBUG: Case 0 delay progress: ");
            mySerial.print(stopElapsed);
            mySerial.print("/50 ticks (");
            mySerial.print(stopElapsed * 10);
            mySerial.println("ms)");
            lastDelayTick = timerTicks;
          }
        }
      } else {
        // Start motor immediately if not stopped
        if (getAutoSequenceStartTick() == 0) {
          mySerial.println(">>> AUTO Case 0: Starting DOWN_TO_UP (initial start)");
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          mySerial.println(">>> DEBUG: Motor UP started in AUTO Case 0 (initial)");
          setAutoSequenceStartTick(timerTicks);
        }
        // Kiểm tra sensor UP trigger
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> AUTO Case 0: UP sensor triggered - Changing to UP_TO_DOWN");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor UP_TO_DOWN
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          mySerial.println(">>> AUTO Case 0: Motor running UP_TO_DOWN");
        }
        
        // Kiểm tra sensor DOWN trigger (chuyển sang Case 1)
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> AUTO Case 0: DOWN sensor triggered - Switching to Case 1");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
    
    // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 1
          setAutoStopped(true);
          setAutoStopStartTick(timerTicks);
          setCurrentAutoSequenceState(AUTO_CASE_1);
          mySerial.println(">>> AUTO: Case 0 → Case 1 (DOWN sensor)");
        }
      }
      break;
    }
    
    case AUTO_CASE_1: {
      // Case 1: UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 500ms → về Case 1
      
      if (getAutoStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        unsigned long stopElapsed = timerTicks - getAutoStopStartTick();
        if (stopElapsed >= 50) {
          mySerial.print(">>> AUTO Case 1: Starting UP_TO_DOWN (delay=");
          mySerial.print(stopElapsed);
          mySerial.println(" ticks)");
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          setAutoStopped(false);
          setAutoSequenceStartTick(timerTicks);
        } else {
          // Debug delay progress
          static unsigned long lastDelayTick = 0;
          if ((timerTicks - lastDelayTick) >= 50) {  // Every 500ms
            mySerial.print(">>> DEBUG: Case 1 delay progress: ");
            mySerial.print(stopElapsed);
            mySerial.print("/50 ticks (");
            mySerial.print(stopElapsed * 10);
            mySerial.println("ms)");
            lastDelayTick = timerTicks;
          }
        }
      } else {
        // Kiểm tra sensor DOWN trigger
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> AUTO Case 1: DOWN sensor triggered - Changing to DOWN_TO_UP");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor DOWN_TO_UP
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          mySerial.println(">>> AUTO Case 1: Motor running DOWN_TO_UP");
        }
        
        // Kiểm tra sensor UP trigger (chuyển sang Case 0)
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> AUTO Case 1: UP sensor triggered - Switching to Case 0");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 0
          setAutoStopped(true);
          setAutoStopStartTick(timerTicks);
          setCurrentAutoSequenceState(AUTO_CASE_0);
          mySerial.println(">>> AUTO: Case 1 → Case 0 (UP sensor)");
        }
      }
      break;
    }
  }
}

// Debug function to print current state
void printAutoSequenceState() {
  mySerial.print("AUTO_STATE: ");
  switch (getCurrentAutoSequenceState()) {
    case AUTO_CASE_0: 
      mySerial.print("CASE_0");
      if (getAutoStopped()) {
        unsigned long stopTime = timerTicks - getAutoStopStartTick();
        mySerial.print(" (STOPPED "); mySerial.print(stopTime); mySerial.print("ms)");
      }
      mySerial.println();
      break;
    case AUTO_CASE_1: 
      mySerial.print("CASE_1");
      if (getAutoStopped()) {
        unsigned long stopTime = timerTicks - getAutoStopStartTick();
        mySerial.print(" (STOPPED "); mySerial.print(stopTime); mySerial.print("ms)");
      }
      mySerial.println();
      break;
    default: mySerial.println("UNKNOWN"); break;
  }
}

void executeAutoDefaultProgram() {
  // Debug message every 1 second
  static unsigned long lastDebugTick = 0;
  if ((timerTicks - lastDebugTick) >= 100) {  // 100 ticks = 1 second
    mySerial.println(">>> DEBUG: executeAutoDefaultProgram() called");
    lastDebugTick = timerTicks;
  }
  // Reset trạng thái
  if (getResetProgramStatesFlag()) {
    setDirectionChanging(false);
    setDirChangeTick(0);
    controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
    setCurrentAutoSequenceState(AUTO_CASE_0);  // Reset to Case 0
    setAutoStopped(false);  // Reset stop flag
    return;
  }
      
  // Check program conditions
  if (!checkProgramConditions()) {
    mySerial.println(">>> DEBUG: checkProgramConditions() failed - returning");
    return;
  } else {
    // Debug message every 1 second
    static unsigned long lastDebugTick = 0;
    if ((timerTicks - lastDebugTick) >= 100) {  // 100 ticks = 1 second
      mySerial.println(">>> DEBUG: checkProgramConditions() passed - continuing");
      lastDebugTick = timerTicks;
    }
  }
  
  // Log when starting AUTO sequence for the first time
  bool autoSequenceStarted = getGlobalAutoSequenceStarted();
  if (!autoSequenceStarted) {
    mySerial.println(">>> AUTO: Starting sequence - Case 0 (DOWN_TO_UP)");
    setGlobalAutoSequenceStarted(true);
  }
  
  runAutoDefaultSequence();
  // Debug message every 1 second
  static unsigned long lastCompletedTick = 0;
  if ((timerTicks - lastCompletedTick) >= 100) {  // 100 ticks = 1 second
    mySerial.println(">>> DEBUG: runAutoDefaultSequence() completed");
    lastCompletedTick = timerTicks;
  }
  
}


// Execute Kneading Program - STATE MACHINE
void runKneadingSequence() {
  /*switch (getCurrentKneadingSequenceState()) {
    case KNEADING_CASE_0: {
      // Case 0: DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 500ms → về Case 0
      
      if (getKneadingStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getKneadingStopStartTick()) >= 500) {
          mySerial.println(">>> KNEADING Case 0: Starting DOWN_TO_UP");
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          setKneadingStopped(false);
          setKneadingSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor UP trigger
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> KNEADING Case 0: UP sensor triggered - Changing to UP_TO_DOWN");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor UP_TO_DOWN
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          mySerial.println(">>> KNEADING Case 0: Motor running UP_TO_DOWN");
        }
        
        // Kiểm tra sensor DOWN trigger (chuyển sang Case 1)
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> KNEADING Case 0: DOWN sensor triggered - Switching to Case 1");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
    
    // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 1
          setKneadingStopped(true);
          setKneadingStopStartTick(timerTicks);
          setCurrentKneadingSequenceState(KNEADING_CASE_1);
          mySerial.println(">>> KNEADING: Case 0 → Case 1 (DOWN sensor)");
        }
      }
      break;
    }
    
    case KNEADING_CASE_1: {
      // Case 1: UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 500ms → về Case 1
      
      if (getKneadingStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getKneadingStopStartTick()) >= 500) {
          mySerial.println(">>> KNEADING Case 1: Starting UP_TO_DOWN");
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          setKneadingStopped(false);
          setKneadingSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor DOWN trigger
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> KNEADING Case 1: DOWN sensor triggered - Changing to DOWN_TO_UP");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor DOWN_TO_UP
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          mySerial.println(">>> KNEADING Case 1: Motor running DOWN_TO_UP");
        }
        
        // Kiểm tra sensor UP trigger (chuyển sang Case 0)
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> KNEADING Case 1: UP sensor triggered - Switching to Case 0");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 0
          setKneadingStopped(true);
          setKneadingStopStartTick(timerTicks);
          setCurrentKneadingSequenceState(KNEADING_CASE_0);
          mySerial.println(">>> KNEADING: Case 1 → Case 0 (UP sensor)");
        }
      }
      break;
    }
  }*/
  offRollMotor();
  offKneadingMotor();
  offCompressionMotor();
}

// Execute Kneading Program
void executeKneadingProgram() {
  // Check program conditions
  if (!checkProgramConditions()) {
    return;
  }
  
  // Log when starting KNEADING sequence for the first time
  bool kneadingSequenceStarted = getGlobalKneadingSequenceStarted();
  if (!kneadingSequenceStarted) {
    mySerial.println(">>> KNEADING: Starting sequence - Case 0 (DOWN_TO_UP)");
    setGlobalKneadingSequenceStarted(true);
  }
  
  runKneadingSequence();
  
  // Display AUTO timer status if running
  unsigned long lastKneadingDebugTick = getGlobalLastKneadingDebugTick();
  if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired() && ((timerTicks - lastKneadingDebugTick) >= 5000)) {
    unsigned long remaining = getAutoRemainingTime();
    mySerial.print("KNEADING: Running | AUTO Timer: ");
    mySerial.print(remaining / 60);
    mySerial.print("m ");
    mySerial.print(remaining % 60);
    mySerial.println("s remaining");
    setGlobalLastKneadingDebugTick(timerTicks);
  }
  
  // Display timer expiration warning
  if (getAutoTotalTimerExpired()) {
    unsigned long lastExpirationTick = getGlobalLastExpirationTick();
    if ((timerTicks - lastExpirationTick) >= 10000) {  // Every 10 seconds
      mySerial.println("⚠️  AUTO TIMER EXPIRED - Will auto-stop when returning to AUTO mode");
      setGlobalLastExpirationTick(timerTicks);
    }
  }
}

// Execute Compression Program - STATE MACHINE
void runCompressionSequence() {
  /*switch (getCurrentCompressionSequenceState()) {
    case COMPRESSION_CASE_0: {
      // Case 0: DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 500ms → về Case 0
      
      if (getCompressionStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getCompressionStopStartTick()) >= 500) {
          mySerial.println(">>> COMPRESSION Case 0: Starting DOWN_TO_UP");
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          setCompressionStopped(false);
          setCompressionSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor UP trigger
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> COMPRESSION Case 0: UP sensor triggered - Changing to UP_TO_DOWN");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor UP_TO_DOWN
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          mySerial.println(">>> COMPRESSION Case 0: Motor running UP_TO_DOWN");
        }
        
        // Kiểm tra sensor DOWN trigger (chuyển sang Case 1)
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> COMPRESSION Case 0: DOWN sensor triggered - Switching to Case 1");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 1
          setCompressionStopped(true);
          setCompressionStopStartTick(timerTicks);
          setCurrentCompressionSequenceState(COMPRESSION_CASE_1);
          mySerial.println(">>> COMPRESSION: Case 0 → Case 1 (DOWN sensor)");
        }
      }
      break;
    }
    
    case COMPRESSION_CASE_1: {
      // Case 1: UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 500ms → về Case 1
      
      if (getCompressionStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getCompressionStopStartTick()) >= 500) {
          mySerial.println(">>> COMPRESSION Case 1: Starting UP_TO_DOWN");
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          setCompressionStopped(false);
          setCompressionSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor DOWN trigger
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> COMPRESSION Case 1: DOWN sensor triggered - Changing to DOWN_TO_UP");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor DOWN_TO_UP
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          mySerial.println(">>> COMPRESSION Case 1: Motor running DOWN_TO_UP");
        }
        
        // Kiểm tra sensor UP trigger (chuyển sang Case 0)
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> COMPRESSION Case 1: UP sensor triggered - Switching to Case 0");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 0
          setCompressionStopped(true);
          setCompressionStopStartTick(timerTicks);
          setCurrentCompressionSequenceState(COMPRESSION_CASE_0);
          mySerial.println(">>> COMPRESSION: Case 1 → Case 0 (UP sensor)");
        }
      }
      break;
    }
  }*/
   offRollMotor();
  offKneadingMotor();
  offCompressionMotor();
}

// Execute Compression Program
void executeCompressionProgram() {
  // Check program conditions
  if (!checkProgramConditions()) {
    return;
  }
  
  // Log when starting COMPRESSION sequence for the first time
  bool compressionSequenceStarted = getGlobalCompressionSequenceStarted();
  if (!compressionSequenceStarted) {
    mySerial.println(">>> COMPRESSION: Starting sequence - Case 0 (DOWN_TO_UP)");
    setGlobalCompressionSequenceStarted(true);
  }
  
  runCompressionSequence();
  
  // Display AUTO timer status if running
  unsigned long lastCompressionDebugTick = getGlobalLastCompressionDebugTick();
  if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired() && ((timerTicks - lastCompressionDebugTick) >= 5000)) {
    unsigned long remaining = getAutoRemainingTime();
    mySerial.print("COMPRESSION: Running | AUTO Timer: ");
    mySerial.print(remaining / 60);
    mySerial.print("m ");
    mySerial.print(remaining % 60);
    mySerial.println("s remaining");
    setGlobalLastCompressionDebugTick(timerTicks);
  }
  
  // Display timer expiration warning
  if (getAutoTotalTimerExpired()) {
    unsigned long lastExpirationTick = getGlobalLastExpirationTick();
    if ((timerTicks - lastExpirationTick) >= 10000) {  // Every 10 seconds
      mySerial.println("⚠️  AUTO TIMER EXPIRED - Will auto-stop when returning to AUTO mode");
      setGlobalLastExpirationTick(timerTicks);
    }
  }
}

// Execute Percussion Program - STATE MACHINE
void runPercussionSequence() {
  /*switch (getCurrentPercussionSequenceState()) {
    case PERCUSSION_CASE_0: {
      // Case 0: DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 500ms → về Case 0
      
      if (getPercussionStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getPercussionStopStartTick()) >= 500) {
          mySerial.println(">>> PERCUSSION Case 0: Starting DOWN_TO_UP");
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          setPercussionStopped(false);
          setPercussionSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor UP trigger
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> PERCUSSION Case 0: UP sensor triggered - Changing to UP_TO_DOWN");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor UP_TO_DOWN
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          mySerial.println(">>> PERCUSSION Case 0: Motor running UP_TO_DOWN");
        }
        
        // Kiểm tra sensor DOWN trigger (chuyển sang Case 1)
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> PERCUSSION Case 0: DOWN sensor triggered - Switching to Case 1");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 1
          setPercussionStopped(true);
          setPercussionStopStartTick(timerTicks);
          setCurrentPercussionSequenceState(PERCUSSION_CASE_1);
          mySerial.println(">>> PERCUSSION: Case 0 → Case 1 (DOWN sensor)");
        }
      }
      break;
    }
    
    case PERCUSSION_CASE_1: {
      // Case 1: UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 500ms → về Case 1
      
      if (getPercussionStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getPercussionStopStartTick()) >= 500) {
          mySerial.println(">>> PERCUSSION Case 1: Starting UP_TO_DOWN");
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          setPercussionStopped(false);
          setPercussionSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor DOWN trigger
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> PERCUSSION Case 1: DOWN sensor triggered - Changing to DOWN_TO_UP");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor DOWN_TO_UP
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          mySerial.println(">>> PERCUSSION Case 1: Motor running DOWN_TO_UP");
        }
        
        // Kiểm tra sensor UP trigger (chuyển sang Case 0)
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> PERCUSSION Case 1: UP sensor triggered - Switching to Case 0");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 0
          setPercussionStopped(true);
          setPercussionStopStartTick(timerTicks);
          setCurrentPercussionSequenceState(PERCUSSION_CASE_0);
          mySerial.println(">>> PERCUSSION: Case 1 → Case 0 (UP sensor)");
        }
      }
      break;
    }
  }*/
  offRollMotor();
  offKneadingMotor();
  offCompressionMotor();
}

// Execute Percussion Program
void executePercussionProgram() {
  // Check program conditions
  if (!checkProgramConditions()) {
    return;
  }
  
  // Log when starting PERCUSSION sequence for the first time
  bool percussionSequenceStarted = getGlobalPercussionSequenceStarted();
  if (!percussionSequenceStarted) {
    mySerial.println(">>> PERCUSSION: Starting sequence - Case 0 (DOWN_TO_UP)");
    setGlobalPercussionSequenceStarted(true);
  }
  
  runPercussionSequence();
  
  // Display AUTO timer status if running
  unsigned long lastPercussionDebugTick = getGlobalLastPercussionDebugTick();
  if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired() && ((timerTicks - lastPercussionDebugTick) >= 5000)) {
    unsigned long remaining = getAutoRemainingTime();
    mySerial.print("PERCUSSION: Running | AUTO Timer: ");
    mySerial.print(remaining / 60);
    mySerial.print("m ");
    mySerial.print(remaining % 60);
    mySerial.println("s remaining");
    setGlobalLastPercussionDebugTick(timerTicks);
  }
  
  // Display timer expiration warning
  if (getAutoTotalTimerExpired()) {
    unsigned long lastExpirationTick = getGlobalLastExpirationTick();
    if ((timerTicks - lastExpirationTick) >= 10000) {  // Every 10 seconds
      mySerial.println("⚠️  AUTO TIMER EXPIRED - Will auto-stop when returning to AUTO mode");
      setGlobalLastExpirationTick(timerTicks);
    }
  }
}

// Execute Combined Program - STATE MACHINE
void runCombinedSequence() {
  /*switch (getCurrentCombinedSequenceState()) {
    case COMBINED_CASE_0: {
      // Case 0: DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 500ms → về Case 0
      
      if (getCombinedStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getCombinedStopStartTick()) >= 500) {
          mySerial.println(">>> COMBINED Case 0: Starting DOWN_TO_UP");
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          setCombinedStopped(false);
          setCombinedSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor UP trigger
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> COMBINED Case 0: UP sensor triggered - Changing to UP_TO_DOWN");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor UP_TO_DOWN
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          mySerial.println(">>> COMBINED Case 0: Motor running UP_TO_DOWN");
        }
        
        // Kiểm tra sensor DOWN trigger (chuyển sang Case 1)
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> COMBINED Case 0: DOWN sensor triggered - Switching to Case 1");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 1
          setCombinedStopped(true);
          setCombinedStopStartTick(timerTicks);
          setCurrentCombinedSequenceState(COMBINED_CASE_1);
          mySerial.println(">>> COMBINED: Case 0 → Case 1 (DOWN sensor)");
        }
      }
      break;
    }
    
    case COMBINED_CASE_1: {
      // Case 1: UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 500ms → về Case 1
      
      if (getCombinedStopped()) {
        // Đang trong thời gian dừng 500ms (50 ticks)
        if ((timerTicks - getCombinedStopStartTick()) >= 500) {
          mySerial.println(">>> COMBINED Case 1: Starting UP_TO_DOWN");
          controlRollMotor(MOTOR_RUN, UP_TO_DOWN);
          setCombinedStopped(false);
          setCombinedSequenceStartTick(timerTicks);
        }
      } else {
        // Kiểm tra sensor DOWN trigger
        if (getChangeDir() && getSensorDownLimit()) {
          mySerial.println(">>> COMBINED Case 1: DOWN sensor triggered - Changing to DOWN_TO_UP");
          controlRollMotor(MOTOR_STOP, UP_TO_DOWN);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start motor DOWN_TO_UP
          controlRollMotor(MOTOR_RUN, DOWN_TO_UP);
          mySerial.println(">>> COMBINED Case 1: Motor running DOWN_TO_UP");
        }
        
        // Kiểm tra sensor UP trigger (chuyển sang Case 0)
        if (getChangeDir() && getSensorUpLimit()) {
          mySerial.println(">>> COMBINED Case 1: UP sensor triggered - Switching to Case 0");
          controlRollMotor(MOTOR_STOP, DOWN_TO_UP);  // Stop motor
          
          // Clear sensor flags
          setChangeDir(false);
          setSensorConfirmInProgress(false);
          setSensorUpLimit(false);
          setSensorDownLimit(false);
          
          // Start stop timer and switch to Case 0
          setCombinedStopped(true);
          setCombinedStopStartTick(timerTicks);
          setCurrentCombinedSequenceState(COMBINED_CASE_0);
          mySerial.println(">>> COMBINED: Case 1 → Case 0 (UP sensor)");
        }
      }
      break;
    }
  }*/
  offRollMotor();
  offKneadingMotor();
  offCompressionMotor();
}

// Execute Combined Program
void executeCombinedProgram() {
  // Check program conditions
  if (!checkProgramConditions()) {
    return;
  }
  
  // Log when starting COMBINED sequence for the first time
  bool combinedSequenceStarted = getGlobalCombinedSequenceStarted();
  if (!combinedSequenceStarted) {
    mySerial.println(">>> COMBINED: Starting sequence - Case 0 (DOWN_TO_UP)");
    setGlobalCombinedSequenceStarted(true);
  }
  
  runCombinedSequence();
  
  // Display AUTO timer status if running
  unsigned long lastCombinedDebugTick = getGlobalLastCombinedDebugTick();
  if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired() && ((timerTicks - lastCombinedDebugTick) >= 5000)) {
    unsigned long remaining = getAutoRemainingTime();
    mySerial.print("COMBINED: Running | AUTO Timer: ");
    mySerial.print(remaining / 60);
    mySerial.print("m ");
    mySerial.print(remaining % 60);
    mySerial.println("s remaining");
    setGlobalLastCombinedDebugTick(timerTicks);
  }
  
  // Display timer expiration warning
  if (getAutoTotalTimerExpired()) {
    unsigned long lastExpirationTick = getGlobalLastExpirationTick();
    if ((timerTicks - lastExpirationTick) >= 10000) {  // Every 10 seconds
      mySerial.println("⚠️  AUTO TIMER EXPIRED - Will auto-stop when returning to AUTO mode");
      setGlobalLastExpirationTick(timerTicks);
    }
  }
}

///////////////////////////////////////////////// AUTO MODE - WITH 20-MINUTE TIMER /////////////////////////////////////////////////
void processAuto() {
  // ═══════════════════════════════════════════════
  // CRITICAL: Skip if not in auto mode or during GO HOME
  // ═══════════════════════════════════════════════
  if (!getModeAuto()) return;  // Not in auto mode - don't interfere
  if (!getHomeRun()) return;   // Not homed yet - don't interfere with GO HOME
  
  // ═══════════════════════════════════════════════
  // AUTO TOTAL TIMER (20 MINUTES) - CHECK FIRST
  // ═══════════════════════════════════════════════
  // Start total timer on first auto mode activation
  if (!getAutoTotalTimerActive() && !getAutoTotalTimerExpired()) {
    setAutoTotalStartTick(timerTicks);
    setAutoTotalTimerActive(true);
    mySerial.println("=== AUTO TOTAL TIMER STARTED (20 minutes) ===");
    mySerial.print("Start time: ");
    mySerial.print(autoTotalStartTick);
    mySerial.println(" ticks");
  }
  
  // Reset timer when auto mode is reactivated after expiration
  if (getAutoTotalTimerExpired() && getModeAuto()) {
    setAutoTotalTimerExpired(false);
    setAutoTotalTimerActive(true);
    setAutoTotalStartTick(timerTicks);
    mySerial.println("=== AUTO TOTAL TIMER RESTARTED (20 minutes) ===");
    mySerial.print("New start time: ");
    mySerial.print(autoTotalStartTick);
    mySerial.println(" ticks");
    mySerial.println("=== PROGRAM STATES RESET TO STEP 0 ===");
    // Reset all program states to step 0
    setResetProgramStatesFlag(true);
  }
  
  // Check if total timer has expired (20 minutes)
  if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired()) {
    autoTotalElapsedTicks = timerTicks - autoTotalStartTick;
    
    // Debug total timer every 30 seconds
    static unsigned long lastTotalTimerDebugTick = 0;
    if ((timerTicks - lastTotalTimerDebugTick) >= 3000) {  // 3000 ticks = 30 seconds
      unsigned long totalRemaining = AUTO_MODE_DURATION_TICKS - autoTotalElapsedTicks;
      mySerial.print(">>> DEBUG: AUTO Total Timer - Elapsed: ");
      mySerial.print(autoTotalElapsedTicks * 10 / 1000);  // Convert to seconds
      mySerial.print("s, Remaining: ");
      mySerial.print(totalRemaining * 10 / 1000);  // Convert to seconds
      mySerial.print("s (");
      mySerial.print(totalRemaining * 10 / 60000);  // Convert to minutes
      mySerial.println("m)");
      lastTotalTimerDebugTick = timerTicks;
    }
    
    if (autoTotalElapsedTicks >= AUTO_MODE_DURATION_TICKS) {
      setAutoTotalTimerExpired(true);
      setAutoTotalTimerActive(false);
      
      mySerial.println("=== AUTO TOTAL TIMER EXPIRED (20 minutes) ===");
      mySerial.print("Total elapsed: ");
      mySerial.print(autoTotalElapsedTicks);
      mySerial.println(" ticks");
      mySerial.println("AUTO MODE WILL BE DISABLED");
      
      // Force stop all auto modes
      setModeAuto(false);
      setModeAuto(false);
    offRollMotor();
      offKneadingMotor();    // Stop kneading motor (FETT_PWM)
      offCompressionMotor(); // Stop compression motor (FETK_PWM)
      
      // Reset all mode flags
      setAutodefaultMode(false);
      setKneadingMode(false);
      setCompressionMode(false);
      setPercussionMode(false);
      setCombineMode(false);
      
      // Reset all program states to step 0
      setResetProgramStatesFlag(true);
      
      mySerial.println("=== ALL AUTO MODES STOPPED ===");
      mySerial.println("=== PROGRAM STATES RESET TO STEP 0 ===");
      mySerial.println("=== WAITING FOR NEW AUTO COMMAND ===");
    return;
  }
  }
  
  // Skip if total timer has expired
  if (getAutoTotalTimerExpired()) {
    return;
  }
  
  unsigned long lastDebugTick = getGlobalLastDebugTick();
  
  // Debug every 5 seconds (500 ticks)
  if (autoModeTimerActive && ((timerTicks - lastDebugTick) >= 500)) {
    unsigned long remaining = getAutoRemainingTime();
    mySerial.print("AUTO: Running | Remaining: ");
    mySerial.print(remaining / 60);
    mySerial.print("m ");
    mySerial.print(remaining % 60);
    mySerial.print("s | Program: ");
    switch (currentAutoProgram) {
      case AUTO_DEFAULT: mySerial.print("AUTO RUN DEFAULT"); break;
      case AUTO_KNEADING: mySerial.print("KNEADING"); break;
      case AUTO_COMPRESSION: mySerial.print("COMPRESSION"); break;
      case AUTO_PERCUSSION: mySerial.print("PERCUSSION"); break;
      case AUTO_COMBINED: mySerial.print("COMBINED"); break;
      default: mySerial.print("NONE"); break;
    }
    mySerial.println();
    setGlobalLastDebugTick(timerTicks);
  }
  
  // Debug total timer every 30 seconds (3000 ticks)
  unsigned long lastTotalDebugTick = getGlobalLastTotalDebugTick();
  if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired() && ((timerTicks - lastTotalDebugTick) >= 3000)) {
    unsigned long totalRemaining = AUTO_MODE_DURATION_TICKS - autoTotalElapsedTicks;
    mySerial.print("TOTAL TIMER: ");
    mySerial.print(autoTotalElapsedTicks / 6000);
    mySerial.print("m ");
    mySerial.print((autoTotalElapsedTicks % 6000) / 100);
    mySerial.print("s elapsed | ");
    mySerial.print(totalRemaining / 6000);
    mySerial.print("m ");
    mySerial.print((totalRemaining % 6000) / 100);
    mySerial.println("s remaining");
    setGlobalLastTotalDebugTick(timerTicks);
  }
  
  // Check timer timeout (20 minutes)
  if (getAutoModeTimerActive()) {
    unsigned long elapsed = timerTicks - getAutoModeStartTick();
    
    // Debug timer every 30 seconds
    static unsigned long lastTimerDebugTick = 0;
    if ((timerTicks - lastTimerDebugTick) >= 3000) {  // 3000 ticks = 30 seconds
      unsigned long remaining = AUTO_MODE_DURATION_TICKS - elapsed;
      mySerial.print(">>> DEBUG: AUTO Timer - Elapsed: ");
      mySerial.print(elapsed * 10 / 1000);  // Convert to seconds
      mySerial.print("s, Remaining: ");
      mySerial.print(remaining * 10 / 1000);  // Convert to seconds
      mySerial.print("s (");
      mySerial.print(remaining * 10 / 60000);  // Convert to minutes
      mySerial.println("m)");
      lastTimerDebugTick = timerTicks;
    }
    
    if (elapsed >= AUTO_MODE_DURATION_TICKS) {
      mySerial.println("AUTO MODE: 20 minutes completed - Stopping");
      
      stopAutoMode();
      return;
    }
  }
  
  // Manual priority overrides auto mode
  if (getManualPriority()) {
    offRollMotor();
    return;
  }
  
  // Detect and update current program
  AutoProgram detectedProgram = detectAutoProgram();
  
  // Track program changes
  if (detectedProgram != getCurrentAutoProgram()) {
    setPreviousAutoProgram(getCurrentAutoProgram());
    setCurrentAutoProgram(detectedProgram);
    setProgramSwitchCount(getProgramSwitchCount() + 1);
    setLastProgramSwitchTick(timerTicks);
    
    mySerial.println("=== PROGRAM SWITCH DETECTED ===");
    mySerial.print("From: ");
    switch (getPreviousAutoProgram()) {
      case AUTO_DEFAULT: mySerial.print("AUTO RUN DEFAULT"); break;
      case AUTO_KNEADING: mySerial.print("KNEADING"); break;
      case AUTO_COMPRESSION: mySerial.print("COMPRESSION"); break;
      case AUTO_PERCUSSION: mySerial.print("PERCUSSION"); break;
      case AUTO_COMBINED: mySerial.print("COMBINED"); break;
      default: mySerial.print("NONE"); break;
    }
    mySerial.print(" → To: ");
    switch (currentAutoProgram) {
      case AUTO_DEFAULT: mySerial.print("AUTO RUN DEFAULT"); break;
      case AUTO_KNEADING: mySerial.print("KNEADING"); break;
      case AUTO_COMPRESSION: mySerial.print("COMPRESSION"); break;
      case AUTO_PERCUSSION: mySerial.print("PERCUSSION"); break;
      case AUTO_COMBINED: mySerial.print("COMBINED"); break;
      default: mySerial.print("NONE"); break;
    }
    mySerial.println();
    mySerial.print("Roll Motor (SPOT): ");
    mySerial.println(rollSpotMode ? "ON" : "OFF");
    mySerial.println();
    mySerial.print("Switch count: "); mySerial.println(programSwitchCount);
    mySerial.println("==============================");
    
    // Reset program states when switching
    setResetProgramStatesFlag(true);
    
    // Reset total timer when switching programs (but keep running)
    if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired()) {
      mySerial.println("=== RESETTING TOTAL TIMER FOR NEW PROGRAM ===");
      setAutoTotalStartTick(timerTicks);
      mySerial.print("New start time: ");
      mySerial.print(autoTotalStartTick);
      mySerial.println(" ticks");
    }
  }
  
  // Execute program-specific logic
  switch (currentAutoProgram) {
    case AUTO_DEFAULT:
      // AUTO_DEFAULT: Roll motor with sensor direction changes
      /*if (useHighPrecisionTimer) {
        executeAutoDefaultProgram();  // Use standard 10ms timer
      } else {
        executeAutoDefaultProgram();    // Use standard 10ms timer
      }*/
      executeAutoDefaultProgram();
      break;
      
    case AUTO_KNEADING:
      executeKneadingProgram();
      break;
      
    case AUTO_COMPRESSION:
      executeCompressionProgram();
      break;
      
    case AUTO_PERCUSSION:
      executePercussionProgram();
      break;
      
    case AUTO_COMBINED:
      executeCombinedProgram();
      break;
      
    default:
      // No program running
      offRollMotor();
      break;
  }
  
  // Clear reset flag after all programs have had a chance to check it
  if (getResetProgramStatesFlag()) {
    setResetProgramStatesFlag(false);
    mySerial.println(">>> Program states reset complete");
  }
}

///////////////////////////////////////////////// STATE MANAGEMENT - GETTERS & SETTERS /////////////////////////////////////////////////

// Getters - Return current state

bool getKneadingMode() {
  return kneadingMode;
}

bool getCompressionMode() {
  return compressionMode;
}

bool getPercussionMode() {
  return percussionMode;
}

bool getAutodefaultMode() {
  return autodefaultMode;
}

bool getCombineMode() {
  return combineMode;
}

uint8_t getIntensityLevel() {
  return intensityLevel;
}

bool getUseHighPrecisionTimer() {
  return useHighPrecisionTimer;
}

// Sensor state getters
bool getSensorUpLimit() {
  return globalSensorUpLimit;
}

bool getSensorDownLimit() {
  return globalSensorDownLimit;
}

bool getSensorUpPending() {
  return sensorUpPending;
}

bool getSensorDownPending() {
  return sensorDownPending;
}

void setSensorUpPending(bool value) {
  sensorUpPending = value;
}

void setSensorDownPending(bool value) {
  sensorDownPending = value;
}

bool getSensorConfirmInProgress() {
  return globalSensorConfirmInProgress;
}

// State getter/setter
bool getRollDirectionState() {
  return state;
}

void setRollDirectionState(bool value) {
  if (state != value) {
    state = value;
    mySerial.print("ROLL_DIRECTION_STATE: Direction changed to ");
    mySerial.println(state ? "UP (HIGH)" : "DOWN (LOW)");
  } else {
    mySerial.println("ROLL_DIRECTION_STATE: No change needed (already same value)");
  }
}

///////////////////////////////////////////////// GLOBAL MOTOR STATE GETTERS & SETTERS /////////////////////////////////////////////////
bool getRL3PWMState() {
  return globalRL3PWMState;
}

void setRL3PWMState(bool state) {
  if (globalRL3PWMState != state) {
    globalRL3PWMState = state;
    mySerial.print("RL3_PWM: State changed to ");
    mySerial.println(state ? "HIGH" : "LOW");
  }
}

bool getWaitingForSensor() {
  return globalWaitingForSensor;
}

void setWaitingForSensor(bool state) {
  if (globalWaitingForSensor != state) {
    globalWaitingForSensor = state;
    mySerial.print("SENSOR: Waiting state changed to ");
    mySerial.println(state ? "TRUE" : "FALSE");
  }
}


bool getMotorStarted() {
  return globalMotorStarted;
}

void setMotorStarted(bool state) {
  if (globalMotorStarted != state) {
    globalMotorStarted = state;
    mySerial.print("MOTOR: Started state changed to ");
    mySerial.println(state ? "TRUE" : "FALSE");
  }
}

// getRL3PWMState() and setRL3PWMState() already defined above

void setSensorUpLimit(bool state) {
  if (globalSensorUpLimit != state) {
    globalSensorUpLimit = state;
    mySerial.print("SENSOR: UP limit changed to ");
    mySerial.println(state ? "TRUE" : "FALSE");
  }
}

void setSensorDownLimit(bool state) {
  mySerial.print("  → DEBUG: setSensorDownLimit(");
  mySerial.print(state ? "TRUE" : "FALSE");
  mySerial.println(") called");
  
  if (globalSensorDownLimit != state) {
    globalSensorDownLimit = state;
    mySerial.print("SENSOR: DOWN limit changed to ");
    mySerial.println(state ? "TRUE" : "FALSE");
    mySerial.print("  → DEBUG: globalSensorDownLimit = ");
    mySerial.println(globalSensorDownLimit ? "TRUE" : "FALSE");
  } else {
    mySerial.println("  → DEBUG: No change needed (already same value)");
  }
}

void setSensorConfirmInProgress(bool state) {
  if (globalSensorConfirmInProgress != state) {
    globalSensorConfirmInProgress = state;
    mySerial.print("SENSOR: Confirm in progress changed to ");
    mySerial.println(state ? "TRUE" : "FALSE");
  }
}

// Setters - Change state with validation and logging




void setKneadingMode(bool value) {
  if (kneadingMode != value) {
    kneadingMode = value;
    mySerial.print("STATE: kneadingMode = ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

void setCompressionMode(bool value) {
  if (compressionMode != value) {
    compressionMode = value;
    mySerial.print("STATE: compressionMode = ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

void setPercussionMode(bool value) {
  if (percussionMode != value) {
    percussionMode = value;
    mySerial.print("STATE: percussionMode = ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

void setAutodefaultMode(bool value) {
  if (autodefaultMode != value) {
    autodefaultMode = value;
    mySerial.print("STATE: autodefaultMode = ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

void setCombineMode(bool value) {
  if (combineMode != value) {
    combineMode = value;
    mySerial.print("STATE: combineMode = ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

void setIntensityLevel(uint8_t value) {
  // Check if intensity change is allowed
  if (!isIntensityChangeAllowed()) {
    mySerial.println("!!! Intensity change not allowed in current program");
    mySerial.println("    Only available in: COMPRESSION, PERCUSSION, COMBINED");
    return;
  }
  
  if (getIntensityLevel() != value) {
    // Print with arrow indicators
    if (value > getIntensityLevel()) {
      mySerial.println(">>> INTENSITY HIGH");
    } else if (value < getIntensityLevel()) {
      mySerial.println("<<< INTENSITY LOW");
    }
    
    setIntensityLevel(value);
    mySerial.print("STATE: intensityLevel = ");
    mySerial.println(value);
    
    // Apply intensity level to compression motor PWM
    if (getCompressionMode()) {
      setCompressionPWMByIntensity();
      mySerial.println("  → Compression motor PWM updated based on intensity");
    }
  }
}

void setUseHighPrecisionTimer(bool value) {
  if (useHighPrecisionTimer != value) {
    useHighPrecisionTimer = value;
    mySerial.print("STATE: useHighPrecisionTimer = ");
    mySerial.println(value ? "TRUE (1ms precision)" : "FALSE (10ms precision)");
    
    if (value) {
      mySerial.println("  → AUTO_DEFAULT will use 1ms precision timer");
    } else {
      mySerial.println("  → AUTO_DEFAULT will use standard 10ms timer");
    }
  }
}

// Print all system states for debugging
void printSystemStatus() {
  mySerial.println("=== SYSTEM STATUS ===");
  mySerial.print("allowRun:         "); mySerial.println(getAllowRun() ? "TRUE" : "FALSE");
  mySerial.print("modeAuto:         "); mySerial.println(getModeAuto() ? "TRUE" : "FALSE");
  mySerial.print("homeRun:          "); mySerial.println(getHomeRun() ? "TRUE" : "FALSE");
  mySerial.println("--- MODE FLAGS ---");
  mySerial.print("autodefaultMode:  "); mySerial.println(getAutodefaultMode() ? "TRUE" : "FALSE");
  mySerial.print("rollSpotMode:     "); mySerial.println(getRollSpotMode() ? "TRUE" : "FALSE");
  mySerial.print("kneadingMode:     "); mySerial.println(getKneadingMode() ? "TRUE" : "FALSE");
  mySerial.print("compressionMode:  "); mySerial.println(getCompressionMode() ? "TRUE" : "FALSE");
  mySerial.print("percussionMode:   "); mySerial.println(getPercussionMode() ? "TRUE" : "FALSE");
  mySerial.print("combineMode:      "); mySerial.println(getCombineMode() ? "TRUE" : "FALSE");
  mySerial.println("--- FEATURES ---");
  mySerial.print("intensityLevel:   "); mySerial.println(intensityLevel);
  mySerial.print("useHighPrecisionTimer: "); mySerial.println(useHighPrecisionTimer ? "TRUE (1ms)" : "FALSE (10ms)");
  mySerial.println("------------------");
  mySerial.print("manualPriority:   "); mySerial.println(manualPriority ? "TRUE" : "FALSE");
  mySerial.print("RL1running:       "); mySerial.println(RL1running ? "TRUE" : "FALSE");
  mySerial.print("RL2running:       "); mySerial.println(RL2running ? "TRUE" : "FALSE");
  mySerial.print("Timer ticks (10ms): "); mySerial.println(timerTicks);
  mySerial.print("Timer ticks (1ms):  "); mySerial.println(timerTicks1ms);
  mySerial.println("--- AUTO TOTAL TIMER (20min) ---");
  mySerial.print("autoTotalTimerActive: "); mySerial.println(getAutoTotalTimerActive() ? "TRUE" : "FALSE");
  mySerial.print("autoTotalTimerExpired: "); mySerial.println(getAutoTotalTimerExpired() ? "TRUE" : "FALSE");
  if (getAutoTotalTimerActive() && !getAutoTotalTimerExpired()) {
    unsigned long remaining = AUTO_MODE_DURATION_TICKS - autoTotalElapsedTicks;
    mySerial.print("Total elapsed: "); mySerial.print(autoTotalElapsedTicks); mySerial.println(" ticks");
    mySerial.print("Remaining: "); mySerial.print(remaining); mySerial.println(" ticks");
    mySerial.print("Remaining: "); mySerial.print(remaining / 6000); mySerial.print("m "); mySerial.print((remaining % 6000) / 100); mySerial.println("s");
  }
  mySerial.println("====================");
}

// Reset all mode flags to safe state
void resetAllModes() {
  mySerial.println("=== RESETTING ALL MODES ===");
  
  // Stop auto mode timer
  setAutoModeTimerActive(false);
  setCurrentAutoProgram(AUTO_NONE);
  
  // Reset total timer
  setAutoTotalTimerActive(false);
  setAutoTotalTimerExpired(false);
  setAutoTotalStartTick(0);
  setAutoTotalElapsedTicks(0);
  
  setModeAuto(false);
  // NOTE: homeRun should NOT be reset here - it's a physical position state, not a mode
  // setHomeRun(false);  // Commented out - home position should persist
  setAutodefaultMode(false);
  setRollSpotMode(false);
  setKneadingMode(false);
  setCompressionMode(false);
  setPercussionMode(false);
  setCombineMode(false);
  setManualPriority(false);
  
  // Reset features
  setIntensityLevel(0);
  
  // Stop all motors
  offRollMotor();
  offReclineIncline();
  offForwardBackward();
  offKneadingMotor();    // Stop kneading motor (FETT_PWM)
  offCompressionMotor(); // Stop compression motor (FETK_PWM)
  
  // Reset motor states
  setRL1running(false);
  setRL2running(false);
  
  // Reset home sequence
  stephomeRun = HOME_IDLE;
  
  mySerial.println("All modes reset to safe state");
}

///////////////////////////////////////////////// HM10 RESET FUNCTION /////////////////////////////////////////////////

void resetHM10() {
  mySerial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>RESETTING HM10");
  
  // Pull BREAK pin LOW to reset HM10
  digitalWrite(HM10_BREAK, LOW);
  delay(100);  // Hold reset for 100ms
  
  // Release BREAK pin (HIGH = normal operation)
  digitalWrite(HM10_BREAK, HIGH);
  delay(100);  // Wait for HM10 to restart
  
  mySerial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>HM10 RESET COMPLETE");
}

///////////////////////////////////////////////// MOTOR CONTROL HELPERS /////////////////////////////////////////////////

// Control roll motor: state=MOTOR_RUN to start, state=MOTOR_STOP to stop, dir=UP_TO_DOWN for UP, dir=DOWN_TO_UP for DOWN
void controlRollMotor(MotorState state, MotorDirection dir) {
  if (state == MOTOR_RUN) {
    // Start motor
    if (dir == UP_TO_DOWN) {
      setRollDirUp();
    } else {  // DOWN_TO_UP
      setRollDirDown();
    }
    onRollMotor();
    setRL3PWMState(true);
  } else {  // MOTOR_STOP
    // Stop motor
    offRollMotor();
    setRL3PWMState(false);
  }
}

///////////////////////////////////////////////// PROGRAM STATE GETTERS & SETTERS /////////////////////////////////////////////////

// Reset program states flag
bool getResetProgramStatesFlag() {
  return globalResetProgramStatesFlag;
}

void setResetProgramStatesFlag(bool value) {
  if (globalResetProgramStatesFlag != value) {
    globalResetProgramStatesFlag = value;
    mySerial.print("RESET_PROGRAM_FLAG: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

// Direction changing state
bool getDirectionChanging() {
  return globalDirectionChanging;
}

void setDirectionChanging(bool value) {
  if (globalDirectionChanging != value) {
    globalDirectionChanging = value;
    mySerial.print("DIRECTION_CHANGING: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

// Direction change tick
unsigned long getDirChangeTick() {
  return globalDirChangeTick;
}

void setDirChangeTick(unsigned long value) {
  if (globalDirChangeTick != value) {
    globalDirChangeTick = value;
    mySerial.print("DIR_CHANGE_TICK: Changed to ");
    mySerial.println(value);
  }
}

// Allow run state
bool getAllowRun() {
  return globalAllowRun;
}

void setAllowRun(bool value) {
  if (globalAllowRun != value) {
    globalAllowRun = value;
    mySerial.print("ALLOW_RUN: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

// Mode auto state
bool getModeAuto() {
  return globalModeAuto;
}

void setModeAuto(bool value) {
  if (globalModeAuto != value) {
    globalModeAuto = value;
    mySerial.print("MODE_AUTO: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

// Home run state
bool getHomeRun() {
  return globalHomeRun;
}

void setHomeRun(bool value) {
  if (globalHomeRun != value) {
    globalHomeRun = value;
    mySerial.print("HOME_RUN: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

// Roll spot mode state
bool getRollSpotMode() {
  return globalRollSpotMode;
}

void setRollSpotMode(bool value) {
  if (globalRollSpotMode != value) {
    globalRollSpotMode = value;
    mySerial.print("ROLL_SPOT_MODE: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

// Change direction state
bool getChangeDir() {
  return globalChangeDir;
}

void setChangeDir(bool value) {
  if (globalChangeDir != value) {
    globalChangeDir = value;
    mySerial.print("CHANGE_DIR: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  }
}

// Auto Total Timer getters/setters
bool getAutoTotalTimerActive() {
  return globalAutoTotalTimerActive;
}

void setAutoTotalTimerActive(bool value) {
  if (globalAutoTotalTimerActive != value) {
    globalAutoTotalTimerActive = value;
    mySerial.print("AUTO_TOTAL_TIMER_ACTIVE: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  } else {
    mySerial.println("AUTO_TOTAL_TIMER_ACTIVE: No change needed (already same value)");
  }
}

bool getAutoTotalTimerExpired() {
  return globalAutoTotalTimerExpired;
}

void setAutoTotalTimerExpired(bool value) {
  if (globalAutoTotalTimerExpired != value) {
    globalAutoTotalTimerExpired = value;
    mySerial.print("AUTO_TOTAL_TIMER_EXPIRED: Changed to ");
    mySerial.println(value ? "TRUE" : "FALSE");
  } else {
    mySerial.println("AUTO_TOTAL_TIMER_EXPIRED: No change needed (already same value)");
  }
}

// Timer variables getters/setters
unsigned long getTimerTicks() {
  return timerTicks;
}

void setTimerTicks(unsigned long value) {
  timerTicks = value;
}

unsigned long getTimerTicks1ms() {
  return timerTicks1ms;
}

void setTimerTicks1ms(unsigned long value) {
  timerTicks1ms = value;
}

unsigned long getTimerTicks1msStep() {
  return timerTicks1msStep;
}

void setTimerTicks1msStep(unsigned long value) {
  timerTicks1msStep = value;
}

// Motor timing variables getters/setters
unsigned long getRl1StartTick() {
  return rl1StartTick;
}

void setRl1StartTick(unsigned long value) {
  rl1StartTick = value;
}

unsigned long getRl2StartTick() {
  return rl2StartTick;
}

void setRl2StartTick(unsigned long value) {
  rl2StartTick = value;
}

unsigned long getRl1DelayStartTick() {
  return rl1DelayStartTick;
}

void setRl1DelayStartTick(unsigned long value) {
  rl1DelayStartTick = value;
}

unsigned long getRl2DelayStartTick() {
  return rl2DelayStartTick;
}

void setRl2DelayStartTick(unsigned long value) {
  rl2DelayStartTick = value;
}

// Home sequence variables getters/setters
unsigned long getHomeStartTick() {
  return homeStartTick;
}

void setHomeStartTick(unsigned long value) {
  homeStartTick = value;
}

unsigned long getHomeStepStartTick() {
  return homeStepStartTick;
}

void setHomeStepStartTick(unsigned long value) {
  homeStepStartTick = value;
}

unsigned long getHomeLastResetTick() {
  return homeLastResetTick;
}

void setHomeLastResetTick(unsigned long value) {
  homeLastResetTick = value;
}

byte getStephomeRun() {
  return stephomeRun;
}

void setStephomeRun(byte value) {
  stephomeRun = value;
}

// Auto mode variables getters/setters
unsigned long getAutoLastDirChangeTick() {
  return autoLastDirChangeTick;
}

void setAutoLastDirChangeTick(unsigned long value) {
  autoLastDirChangeTick = value;
}

unsigned long getAutoModeStartTick() {
  return autoModeStartTick;
}

void setAutoModeStartTick(unsigned long value) {
  autoModeStartTick = value;
}

unsigned long getAutoModeElapsedTicks() {
  return autoModeElapsedTicks;
}

void setAutoModeElapsedTicks(unsigned long value) {
  autoModeElapsedTicks = value;
}

bool getAutoModeTimerActive() {
  return autoModeTimerActive;
}

void setAutoModeTimerActive(bool value) {
  autoModeTimerActive = value;
}

bool getAutoTimerStarted() {
  return autoTimerStarted;
}

void setAutoTimerStarted(bool value) {
  autoTimerStarted = value;
}

unsigned long getAutoTotalStartTick() {
  return autoTotalStartTick;
}

void setAutoTotalStartTick(unsigned long value) {
  autoTotalStartTick = value;
}

unsigned long getAutoTotalElapsedTicks() {
  return autoTotalElapsedTicks;
}

void setAutoTotalElapsedTicks(unsigned long value) {
  autoTotalElapsedTicks = value;
}

// State machine state getters/setters
AutoSequenceState getCurrentAutoSequenceState() {
  return currentAutoSequenceState;
}

void setCurrentAutoSequenceState(AutoSequenceState value) {
  currentAutoSequenceState = value;
}

KneadingSequenceState getCurrentKneadingSequenceState() {
  return currentKneadingSequenceState;
}

void setCurrentKneadingSequenceState(KneadingSequenceState value) {
  currentKneadingSequenceState = value;
}

CompressionSequenceState getCurrentCompressionSequenceState() {
  return currentCompressionSequenceState;
}

void setCurrentCompressionSequenceState(CompressionSequenceState value) {
  currentCompressionSequenceState = value;
}

PercussionSequenceState getCurrentPercussionSequenceState() {
  return currentPercussionSequenceState;
}

void setCurrentPercussionSequenceState(PercussionSequenceState value) {
  currentPercussionSequenceState = value;
}

CombinedSequenceState getCurrentCombinedSequenceState() {
  return currentCombinedSequenceState;
}

void setCurrentCombinedSequenceState(CombinedSequenceState value) {
  currentCombinedSequenceState = value;
}

// State machine timing and control getters/setters
// AUTO sequence timing
unsigned long getAutoSequenceStartTick() {
  return autoSequenceStartTick;
}

void setAutoSequenceStartTick(unsigned long value) {
  autoSequenceStartTick = value;
}

unsigned long getAutoStopStartTick() {
  return autoStopStartTick;
}

void setAutoStopStartTick(unsigned long value) {
  autoStopStartTick = value;
}

bool getAutoStopped() {
  return autoStopped;
}

void setAutoStopped(bool value) {
  autoStopped = value;
}

// KNEADING sequence timing
unsigned long getKneadingSequenceStartTick() {
  return kneadingSequenceStartTick;
}

void setKneadingSequenceStartTick(unsigned long value) {
  kneadingSequenceStartTick = value;
}

unsigned long getKneadingStopStartTick() {
  return kneadingStopStartTick;
}

void setKneadingStopStartTick(unsigned long value) {
  kneadingStopStartTick = value;
}

bool getKneadingStopped() {
  return kneadingStopped;
}

void setKneadingStopped(bool value) {
  kneadingStopped = value;
}

// COMPRESSION sequence timing
unsigned long getCompressionSequenceStartTick() {
  return compressionSequenceStartTick;
}

void setCompressionSequenceStartTick(unsigned long value) {
  compressionSequenceStartTick = value;
}

unsigned long getCompressionStopStartTick() {
  return compressionStopStartTick;
}

void setCompressionStopStartTick(unsigned long value) {
  compressionStopStartTick = value;
}

bool getCompressionStopped() {
  return compressionStopped;
}

void setCompressionStopped(bool value) {
  compressionStopped = value;
}

// PERCUSSION sequence timing
unsigned long getPercussionSequenceStartTick() {
  return percussionSequenceStartTick;
}

void setPercussionSequenceStartTick(unsigned long value) {
  percussionSequenceStartTick = value;
}

unsigned long getPercussionStopStartTick() {
  return percussionStopStartTick;
}

void setPercussionStopStartTick(unsigned long value) {
  percussionStopStartTick = value;
}

bool getPercussionStopped() {
  return percussionStopped;
}

void setPercussionStopped(bool value) {
  percussionStopped = value;
}

// COMBINED sequence timing
unsigned long getCombinedSequenceStartTick() {
  return combinedSequenceStartTick;
}

void setCombinedSequenceStartTick(unsigned long value) {
  combinedSequenceStartTick = value;
}

unsigned long getCombinedStopStartTick() {
  return combinedStopStartTick;
}

void setCombinedStopStartTick(unsigned long value) {
  combinedStopStartTick = value;
}

bool getCombinedStopped() {
  return combinedStopped;
}

void setCombinedStopped(bool value) {
  combinedStopped = value;
}

///////////////////////////////////////////////// STATIC VARIABLES CONVERTED TO GLOBAL GETTERS/SETTERS /////////////////////////////////////////////////
// Sensor confirmation variables
bool getGlobalPreviousUpState() {
  return globalPreviousUpState;
}
void setGlobalPreviousUpState(bool value) {
  globalPreviousUpState = value;
}
bool getGlobalPreviousDownState() {
  return globalPreviousDownState;
}
void setGlobalPreviousDownState(bool value) {
  globalPreviousDownState = value;
}
bool getGlobalRl1WaitingForDelay() {
  return globalRl1WaitingForDelay;
}
void setGlobalRl1WaitingForDelay(bool value) {
  globalRl1WaitingForDelay = value;
}
bool getGlobalRl2WaitingForDelay() {
  return globalRl2WaitingForDelay;
}
void setGlobalRl2WaitingForDelay(bool value) {
  globalRl2WaitingForDelay = value;
}

// Home sequence variables
unsigned long getGlobalLastDelayMsg() {
  return globalLastDelayMsg;
}
void setGlobalLastDelayMsg(unsigned long value) {
  globalLastDelayMsg = value;
}
unsigned long getGlobalLastSearchDebug() {
  return globalLastSearchDebug;
}
void setGlobalLastSearchDebug(unsigned long value) {
  globalLastSearchDebug = value;
}
bool getGlobalMotorStartedLocal() {
  return globalMotorStartedLocal;
}
void setGlobalMotorStartedLocal(bool value) {
  globalMotorStartedLocal = value;
}
unsigned long getGlobalLastBlockMsg() {
  return globalLastBlockMsg;
}
void setGlobalLastBlockMsg(unsigned long value) {
  globalLastBlockMsg = value;
}
bool getGlobalMotorDownStarted() {
  return globalMotorDownStarted;
}
void setGlobalMotorDownStarted(bool value) {
  globalMotorDownStarted = value;
}
unsigned long getGlobalLastDownDelayMsg() {
  return globalLastDownDelayMsg;
}
void setGlobalLastDownDelayMsg(unsigned long value) {
  globalLastDownDelayMsg = value;
}

// Sensor confirmation state
SensorConfirmState getGlobalConfirmState() {
  return globalConfirmState;
}
void setGlobalConfirmState(SensorConfirmState value) {
  globalConfirmState = value;
}
bool getGlobalIsUpSensor() {
  return globalIsUpSensor;
}
void setGlobalIsUpSensor(bool value) {
  globalIsUpSensor = value;
}
unsigned long getGlobalLastPendingDebugTick() {
  return globalLastPendingDebugTick;
}
void setGlobalLastPendingDebugTick(unsigned long value) {
  globalLastPendingDebugTick = value;
}
unsigned long getGlobalLastFunctionDebugTick() {
  return globalLastFunctionDebugTick;
}
void setGlobalLastFunctionDebugTick(unsigned long value) {
  globalLastFunctionDebugTick = value;
}
unsigned long getGlobalLastIdleDebugTick() {
  return globalLastIdleDebugTick;
}
void setGlobalLastIdleDebugTick(unsigned long value) {
  globalLastIdleDebugTick = value;
}
unsigned long getGlobalLastWaitingDebugTick() {
  return globalLastWaitingDebugTick;
}
void setGlobalLastWaitingDebugTick(unsigned long value) {
  globalLastWaitingDebugTick = value;
}
unsigned long getGlobalLastConfirmDebugTick() {
  return globalLastConfirmDebugTick;
}
void setGlobalLastConfirmDebugTick(unsigned long value) {
  globalLastConfirmDebugTick = value;
}
unsigned long getGlobalLastConfirmedDebugTick() {
  return globalLastConfirmedDebugTick;
}
void setGlobalLastConfirmedDebugTick(unsigned long value) {
  globalLastConfirmedDebugTick = value;
}

// Program sequence started flags
bool getGlobalAutoSequenceStarted() {
  return globalAutoSequenceStarted;
}
void setGlobalAutoSequenceStarted(bool value) {
  globalAutoSequenceStarted = value;
}
bool getGlobalKneadingSequenceStarted() {
  return globalKneadingSequenceStarted;
}
void setGlobalKneadingSequenceStarted(bool value) {
  globalKneadingSequenceStarted = value;
}
bool getGlobalCompressionSequenceStarted() {
  return globalCompressionSequenceStarted;
}
void setGlobalCompressionSequenceStarted(bool value) {
  globalCompressionSequenceStarted = value;
}
bool getGlobalPercussionSequenceStarted() {
  return globalPercussionSequenceStarted;
}
void setGlobalPercussionSequenceStarted(bool value) {
  globalPercussionSequenceStarted = value;
}
bool getGlobalCombinedSequenceStarted() {
  return globalCombinedSequenceStarted;
}
void setGlobalCombinedSequenceStarted(bool value) {
  globalCombinedSequenceStarted = value;
}

// Debug timing variables
unsigned long getGlobalLastKneadingDebugTick() {
  return globalLastKneadingDebugTick;
}
void setGlobalLastKneadingDebugTick(unsigned long value) {
  globalLastKneadingDebugTick = value;
}
unsigned long getGlobalLastCompressionDebugTick() {
  return globalLastCompressionDebugTick;
}
void setGlobalLastCompressionDebugTick(unsigned long value) {
  globalLastCompressionDebugTick = value;
}
unsigned long getGlobalLastPercussionDebugTick() {
  return globalLastPercussionDebugTick;
}
void setGlobalLastPercussionDebugTick(unsigned long value) {
  globalLastPercussionDebugTick = value;
}
unsigned long getGlobalLastCombinedDebugTick() {
  return globalLastCombinedDebugTick;
}
void setGlobalLastCombinedDebugTick(unsigned long value) {
  globalLastCombinedDebugTick = value;
}
unsigned long getGlobalLastExpirationTick() {
  return globalLastExpirationTick;
}
void setGlobalLastExpirationTick(unsigned long value) {
  globalLastExpirationTick = value;
}
unsigned long getGlobalLastDebugTick() {
  return globalLastDebugTick;
}
void setGlobalLastDebugTick(unsigned long value) {
  globalLastDebugTick = value;
}
unsigned long getGlobalLastTotalDebugTick() {
  return globalLastTotalDebugTick;
}
void setGlobalLastTotalDebugTick(unsigned long value) {
  globalLastTotalDebugTick = value;
}

// Manual priority getters/setters
bool getManualPriority() {
  return manualPriority;
}
void setManualPriority(bool value) {
  manualPriority = value;
}

// System state getters/setters
bool getSystemStuck() {
  return systemStuck;
}
void setSystemStuck(bool value) {
  systemStuck = value;
}
AutoProgram getCurrentAutoProgram() {
  return currentAutoProgram;
}
void setCurrentAutoProgram(AutoProgram value) {
  currentAutoProgram = value;
}
AutoProgram getPreviousAutoProgram() {
  return previousAutoProgram;
}
void setPreviousAutoProgram(AutoProgram value) {
  previousAutoProgram = value;
}
unsigned long getProgramSwitchCount() {
  return programSwitchCount;
}
void setProgramSwitchCount(unsigned long value) {
  programSwitchCount = value;
}
unsigned long getLastProgramSwitchTick() {
  return lastProgramSwitchTick;
}
void setLastProgramSwitchTick(unsigned long value) {
  lastProgramSwitchTick = value;
}

// Loop timing getters/setters
unsigned long getLastLoopTick() {
  return lastLoopTick;
}
void setLastLoopTick(unsigned long value) {
  lastLoopTick = value;
}
unsigned long getLoopCounter() {
  return loopCounter;
}
void setLoopCounter(unsigned long value) {
  loopCounter = value;
}

// Sensor state getters/setters
uint8_t getButtonUpSamples() {
  return buttonUpSamples;
}
void setButtonUpSamples(uint8_t value) {
  buttonUpSamples = value;
}
uint8_t getButtonDownSamples() {
  return buttonDownSamples;
}
void setButtonDownSamples(uint8_t value) {
  buttonDownSamples = value;
}
bool getLastUpState() {
  return lastUpState;
}
void setLastUpState(bool value) {
  lastUpState = value;
}
bool getLastDownState() {
  return lastDownState;
}
void setLastDownState(bool value) {
  lastDownState = value;
}

// Sensor confirmation timing getters/setters
unsigned long getSensorConfirmStartTick() {
  return sensorConfirmStartTick;
}
void setSensorConfirmStartTick(unsigned long value) {
  sensorConfirmStartTick = value;
}

// Motor state getters/setters
bool getRL1running() {
  return RL1running;
}
void setRL1running(bool value) {
  RL1running = value;
}
bool getRL2running() {
  return RL2running;
}
void setRL2running(bool value) {
  RL2running = value;
}
unsigned long getNowTimeRL1running() {
  return nowTimeRL1running;
}
void setNowTimeRL1running(unsigned long value) {
  nowTimeRL1running = value;
}
unsigned long getNowTimeRL2running() {
  return nowTimeRL2running;
}
void setNowTimeRL2running(unsigned long value) {
  nowTimeRL2running = value;
}



