#ifndef MASSAGE_V1_HARDWARE_H
#define MASSAGE_V1_HARDWARE_H
#include <Arduino.h>
#include <HardwareSerial.h>

///////////////////////////////////////////////// ENUMS /////////////////////////////////////////////////
// Motor state enum
enum MotorState {
  MOTOR_STOP = 0,
  MOTOR_RUN = 1
};

// Motor direction enum
enum MotorDirection {
  UP_TO_DOWN = 0,
  DOWN_TO_UP = 1
};

// AUTO sequence states
enum AutoSequenceState {
  AUTO_CASE_0 = 0,  // DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 1000ms → về Case 0
  AUTO_CASE_1 = 1   // UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 1000ms → về Case 1
};

// KNEADING sequence states
enum KneadingSequenceState {
  KNEADING_CASE_0 = 0,  // DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 1000ms → về Case 0
  KNEADING_CASE_1 = 1   // UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 1000ms → về Case 1
};

// COMPRESSION sequence states
enum CompressionSequenceState {
  COMPRESSION_CASE_0 = 0,  // DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 1000ms → về Case 0
  COMPRESSION_CASE_1 = 1   // UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 1000ms → về Case 1
};

// PERCUSSION sequence states
enum PercussionSequenceState {
  PERCUSSION_CASE_0 = 0,  // DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 1000ms → về Case 0
  PERCUSSION_CASE_1 = 1   // UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 1000ms → về Case 1
};

// COMBINED sequence states
enum CombinedSequenceState {
  COMBINED_CASE_0 = 0,  // DOWN_TO_UP → chạm sensor UP → UP_TO_DOWN → dừng 1000ms → về Case 0
  COMBINED_CASE_1 = 1   // UP_TO_DOWN → chạm sensor DOWN → DOWN_TO_UP → dừng 1000ms → về Case 1
};

// Sensor confirmation state enum
enum SensorConfirmState {
  IDLE,
  WAITING_CONFIRM,
  CONFIRMED
};

///////////////////////////////////////////////// PIN DEFINITIONS /////////////////////////////////////////////////
// Relay 1 - Recline/Incline Control
#define RL1_PWM PA5
#define RL1_DIR PA0

// Relay 2 - Forward/Backward Control
#define RL2_PWM PA4
#define RL2_DIR PA1

// Relay 3 - Roll Motor Control
#define RL3_PWM PB1
#define RL3_DIR PA8

// FET Controls
#define FETT_PWM PB0  // KNEADING
#define FETK_PWM PA7  // COMPRESSION

// Limit Sensors
#define LMT_UP PB4
#define LMT_DOWN PB3

// BLE Module
#define HM10_BREAK PB9

///////////////////////////////////////////////// TIMING CONSTANTS /////////////////////////////////////////////////
// Timer base configuration (10ms per tick)
#define TIMER_TICK_MS 10
#define DEBOUNCE_SAMPLES 3
#define SAMPLE_INTERVAL TIMER_TICK_MS

// Convert milliseconds to timer ticks (1 tick = 10ms)
#define MS_TO_TICKS(ms) ((ms) / TIMER_TICK_MS)

// Motor timeout protection (in ticks)
#define RL1_RL2_TIMEOUT_TICKS MS_TO_TICKS(20000)  // 20s = 2000 ticks
#define MOTOR_DIR_CHANGE_TICKS MS_TO_TICKS(100)   // 100ms = 10 ticks

// Home sequence timeouts (in ticks)
#define HOME_TOTAL_TIMEOUT_TICKS MS_TO_TICKS(60000)       // 60s
#define HOME_STEP3_TIMEOUT_TICKS MS_TO_TICKS(5000)        // 5s
#define HOME_STEP3_RUN_TICKS MS_TO_TICKS(2000)            // 1s
#define HOME_INACTIVITY_TIMEOUT_TICKS MS_TO_TICKS(30000)  // 30s
#define HOME_DIR_CHANGE_TICKS MS_TO_TICKS(100)            // 100ms

// Auto mode timing (in ticks)
#define AUTO_DIR_CHANGE_TICKS MS_TO_TICKS(100)         // 100ms
#define SENSOR_CONFIRM_DELAY_TICKS MS_TO_TICKS(500)    // 500ms - Anti-noise delay
#define AUTO_MODE_DURATION_TICKS MS_TO_TICKS(1200000)  // 20 minutes = 1200000ms = 120000 ticks

// System health timeouts (in ticks)
#define SYSTEM_STUCK_TIMEOUT_TICKS MS_TO_TICKS(5000)     // 5s
#define SYSTEM_RECOVERY_TIMEOUT_TICKS MS_TO_TICKS(1000)  // 1s

// Command counter timeout (in ticks)
#define COMMAND_COUNTER_TIMEOUT_TICKS MS_TO_TICKS(10000)  // 10s

// Watchdog configuration (ms - not using ticks)
#define WATCHDOG_TIMEOUT 3000

///////////////////////////////////////////////// HELPER MACROS /////////////////////////////////////////////////
// Check how many modes are active
#define COUNT_ACTIVE_MODES() ((autodefaultMode ? 1 : 0) + (rollSpotMode ? 1 : 0) + (kneadingMode ? 1 : 0) + (compressionMode ? 1 : 0) + (percussionMode ? 1 : 0) + (combineMode ? 1 : 0))

// Check if system is ready for AUTO
#define IS_READY_FOR_AUTO() (allowRun && homeRun && !manualPriority)

// Check if in AUTO mode and running
#define IS_AUTO_RUNNING() (modeAuto && autoModeTimerActive)

///////////////////////////////////////////////// HOME SEQUENCE STATES /////////////////////////////////////////////////
enum HomeState {
  HOME_IDLE = 0,          // Not started
  HOME_SEARCHING_UP = 1,  // Roll motor running UP, searching for UP sensor
  HOME_DELAY_AT_UP = 2,   // Already at UP sensor, waiting 500ms
  HOME_RUNNING_DOWN = 3   // After delay, running down 2s to home position
};

///////////////////////////////////////////////// AUTO PROGRAM DEFINITIONS /////////////////////////////////////////////////
enum AutoProgram {
  AUTO_NONE = 0,         // No program running
  AUTO_DEFAULT = 1,      // Default AUTO mode (autodefaultMode - CMD_AUTO 0x10)
  AUTO_KNEADING = 2,     // Kneading mode (kneadingMode - CMD_KNEADING 0x30)
  AUTO_COMPRESSION = 3,  // Compression mode (compressionMode - CMD_COMPRESSION 0x50)
  AUTO_PERCUSSION = 4,   // Percussion mode (percussionMode - CMD_PERCUSSION 0x40)
  AUTO_COMBINED = 5      // Combined modes (multiple program flags)
};

// NOTE: rollSpotMode is NOT a program - it's a MOTOR CONTROL FEATURE
// Can be ON/OFF in any program above
///////////////////////////////////////////////// SERIAL & HARDWARE /////////////////////////////////////////////////
extern HardwareSerial mySerial;
extern HardwareSerial mySerial2;
extern HardwareTimer *MyTim;

///////////////////////////////////////////////// TIMING VARIABLES - TIMER BASED /////////////////////////////////////////////////
// Master timer tick counter (incremented every 10ms in ISR)
extern volatile unsigned long timerTicks;
extern volatile unsigned long timerTicks1ms;      // High precision timer (1ms)
extern volatile unsigned long timerTicks1msStep;  // Step timer for program steps

// Timer-based counters for various functions
extern unsigned long lastLoopTick;
extern unsigned long loopCounter;

// Motor timing counters
extern unsigned long rl1StartTick;
extern unsigned long rl2StartTick;
extern unsigned long rl1DelayStartTick;
extern unsigned long rl2DelayStartTick;

// Home sequence timing
extern unsigned long homeStartTick;
extern unsigned long homeStepStartTick;
extern unsigned long homeLastResetTick;

// Auto mode timing
extern unsigned long autoLastDirChangeTick;
extern unsigned long autoModeStartTick;     // When timer started (khi nhấn nút AUTO)
extern unsigned long autoModeElapsedTicks;  // Total elapsed time
extern bool autoModeTimerActive;            // Timer is running
extern bool autoTimerStarted;               // TRUE = Timer đã bắt đầu (ngay khi nhấn AUTO, tránh duplicate)

// Auto mode total timer (20 minutes)
extern unsigned long autoTotalStartTick;     // When total timer started (first auto mode)
extern unsigned long autoTotalElapsedTicks;  // Total elapsed time across all auto modes
extern bool autoTotalTimerActive;            // Total timer is running
extern bool autoTotalTimerExpired;           // Total timer has expired

///////////////////////////////////////////////// SENSOR STATE VARIABLES /////////////////////////////////////////////////
extern volatile bool state;
extern volatile bool changeDir;
extern volatile bool allowRun;
extern volatile uint8_t buttonUpSamples;
extern volatile uint8_t buttonDownSamples;
extern volatile bool lastUpState;
extern volatile bool lastDownState;
extern volatile bool sensorUpLimit;
extern volatile bool sensorDownLimit;

// Sensor confirmation for anti-noise
extern volatile bool sensorUpPending;         // Sensor triggered, waiting for confirmation
extern volatile bool sensorDownPending;       // Sensor triggered, waiting for confirmation
extern unsigned long sensorConfirmStartTick;  // Tick when sensor triggered
extern bool sensorConfirmInProgress;          // CRITICAL: Block motor during confirmation

///////////////////////////////////////////////// MOTOR STATE VARIABLES /////////////////////////////////////////////////
extern bool RL1running, RL2running;
extern unsigned long nowTimeRL1running;
extern unsigned long nowTimeRL2running;

///////////////////////////////////////////////// MODE FLAGS /////////////////////////////////////////////////
// System control flags
extern volatile bool homeRun;         // TRUE = Home position reached, can run AUTO
extern volatile bool modeAuto;        // TRUE = AUTO mode active (20-min timer running)
extern volatile bool manualPriority;  // TRUE = Manual control active (RL1/RL2 running)

// AUTO program mode flags (each can be TRUE/FALSE independently)
extern volatile bool autodefaultMode;  // Auto default mode (base program)
extern volatile bool rollSpotMode;     // Roll/Spot massage mode (FEATURE)
extern volatile bool kneadingMode;     // Kneading massage mode
extern volatile bool compressionMode;  // Compression massage mode
extern volatile bool percussionMode;   // Percussion massage mode
extern volatile bool combineMode;      // Combine mode (when 2+ programs active)

// FEATURE flags (not program types)
extern volatile uint8_t intensityLevel;  // Intensity level (0-255) - FEATURE

/*
 * ═══════════════════════════════════════════════════════════════════
 * PROGRAM TYPES (5):
 * ═══════════════════════════════════════════════════════════════════
 * - AUTO_DEFAULT     = autodefaultMode ONLY
 * - AUTO_KNEADING    = kneadingMode ONLY
 * - AUTO_COMPRESSION = compressionMode ONLY
 * - AUTO_PERCUSSION  = percussionMode ONLY
 * - AUTO_COMBINED    = 2 or more PROGRAM modes active
 *
 * ═══════════════════════════════════════════════════════════════════
 * FEATURES (2):
 * ═══════════════════════════════════════════════════════════════════
 * 1. rollSpotMode (bool) - Roll Motor ON/OFF control
 *    - AUTO_DEFAULT: 🔒 LOCKED (always ON, cannot toggle)
 *    - KNEADING/COMPRESSION/PERCUSSION/COMBINED: ✅ CAN TOGGLE
 * 
 * 2. intensityLevel (uint8_t 0-255) - Intensity/Speed control
 *    - AUTO_DEFAULT/KNEADING: 🔒 LOCKED (cannot change)
 *    - COMPRESSION/PERCUSSION/COMBINED: ✅ CAN CHANGE
 *
 * ═══════════════════════════════════════════════════════════════════
 * COMMAND MAPPING:
 * ═══════════════════════════════════════════════════════════════════
 * - CMD_AUTO (0x10, 0xF0)              → AUTO_DEFAULT (rollSpotMode forced ON)
 * - CMD_ROLL_MOTOR (0x20, 0xF0/0x00)   → Toggle rollSpotMode (except AUTO_DEFAULT)
 * - CMD_KNEADING (0x30, 0xF0)          → AUTO_KNEADING
 * - CMD_PERCUSSION (0x40, 0xF0)        → AUTO_PERCUSSION
 * - CMD_COMPRESSION (0x50, 0xF0)       → AUTO_COMPRESSION
 * - CMD_COMBINE (0x60, 0xF0)           → AUTO_COMBINED (kneading+compression)
 * - CMD_INTENSITY_LEVEL (0x70, 0x20/0x00) → Set intensity HIGH/LOW (except AUTO_DEFAULT/KNEADING)
 */

///////////////////////////////////////////////// HOME SEQUENCE VARIABLES /////////////////////////////////////////////////
extern byte stephomeRun;

///////////////////////////////////////////////// AUTO MODE VARIABLES /////////////////////////////////////////////////
extern AutoProgram currentAutoProgram;       // Current running program
extern AutoProgram previousAutoProgram;      // Previous program (for tracking changes)
extern bool resetProgramStatesFlag;          // Flag to reset program static variables
extern unsigned long programSwitchCount;     // Track how many times program switched
extern unsigned long lastProgramSwitchTick;  // When last program switch occurred

///////////////////////////////////////////////// SAFETY VARIABLES /////////////////////////////////////////////////
extern bool systemStuck;

///////////////////////////////////////////////// INITIALIZATION FUNCTIONS /////////////////////////////////////////////////
void serialInit();
void outputInit();
void limitSensorInit();
void timerCheckLimitSensorISR();
void timer1msISR();  // 1ms precision timer ISR

///////////////////////////////////////////////// MOTOR CONTROL - DECLARATIONS /////////////////////////////////////////////////
// Recline/Incline (RL1) - OFF function (implemented in .cpp with state reset)
void offReclineIncline();

// Forward/Backward (RL2) - OFF function (implemented in .cpp with state reset)
void offForwardBackward();

// Roll Motor (RL3) - Basic control with debug
void onRollMotor();   // Move to .cpp for debug
void offRollMotor();  // Move to .cpp for debug

inline void dirRollMotor(bool direction) {
  digitalWrite(RL3_DIR, direction);
}

// Roll Motor - Direction control (PWM must be OFF before calling these)
inline void setRollDirUp() {
  digitalWrite(RL3_DIR, HIGH);  // Direction UP
}

inline void setRollDirDown() {
  digitalWrite(RL3_DIR, LOW);  // Direction DOWN
}

// Roll Motor - Combined direction + motor ON (convenience functions)
inline void runRollUp() {
  digitalWrite(RL3_DIR, HIGH);  // Direction UP
  digitalWrite(RL3_PWM, HIGH);  // Motor ON
}

inline void runRollDown() {
  digitalWrite(RL3_DIR, LOW);   // Direction DOWN
  digitalWrite(RL3_PWM, HIGH);  // Motor ON
}

// KNEADING Motor Control (FETT_PWM) - PWM 99% default
void onKneadingMotor();
void offKneadingMotor();
void setKneadingPWM(uint8_t pwmValue);  // 0-255 (0%=0, 100%=255)

// COMPRESSION Motor Control (FETK_PWM) - PWM based on intensity
void onCompressionMotor();
void offCompressionMotor();
void setCompressionPWM(uint8_t pwmValue);  // 0-255 (0%=0, 100%=255)
void setCompressionPWMByIntensity();       // Auto-set PWM based on intensityLevel

// Non-inline motor functions (need delays)
void onRecline();
void onIncline();
void onBackward();
void onForward();

///////////////////////////////////////////////// MOTOR MANAGEMENT /////////////////////////////////////////////////
void resetRL1RL2();

///////////////////////////////////////////////// SEQUENCE CONTROL /////////////////////////////////////////////////
void processGoHome();
void processAuto();
void processSensorConfirmation();  // Anti-noise sensor check

// Auto mode management
void startAutoMode();                  // Start auto mode with timer
void stopAutoMode();                   // Stop auto mode and reset timer
AutoProgram detectAutoProgram();       // Detect which program based on mode flags
void printAutoModeStatus();            // Print auto mode info
unsigned long getAutoRemainingTime();  // Get remaining time in seconds

// Helper functions for command validation
bool isRollMotorToggleAllowed();  // Check if roll motor can be toggled (SPOT mode)
bool isIntensityChangeAllowed();  // Check if intensity can be changed in current program

// Program execution functions (each program has specific logic)
void executeAutoDefaultProgram();  // AUTO_DEFAULT program (Roll motor with sensors)
void printAutoSequenceState();     // Debug function to print current state
void executeKneadingProgram();     // Kneading massage program
void executeCompressionProgram();  // Compression program
void executePercussionProgram();   // Percussion program
void executeCombinedProgram();     // Combined programs

// Common program functions
bool checkProgramConditions();  // Check common execution conditions

// State machine sequence functions
void runAutoDefaultSequence();  // AUTO state machine
void runKneadingSequence();     // KNEADING state machine
void runCompressionSequence();  // COMPRESSION state machine
void runPercussionSequence();   // PERCUSSION state machine
void runCombinedSequence();     // COMBINED state machine

// Reset program static variables
void resetProgramStates();  // Reset all static variables in execute functions

///////////////////////////////////////////////// BLE MODULE /////////////////////////////////////////////////
void resetHM10();

///////////////////////////////////////////////// WATCHDOG TIMER /////////////////////////////////////////////////
void watchdogInit();
inline void watchdogReset() {
#ifdef HAL_IWDG_MODULE_ENABLED
  IWDG_HandleTypeDef hiwdg;
  hiwdg.Instance = IWDG;
  HAL_IWDG_Refresh(&hiwdg);
#else
  IWDG->KR = 0xAAAA;
#endif
}

///////////////////////////////////////////////// SAFETY MECHANISM /////////////////////////////////////////////////
void checkSystemHealth();
void emergencyStop();

///////////////////////////////////////////////// STATE MANAGEMENT - GETTERS & SETTERS /////////////////////////////////////////////////
// Getters - Read current state
bool getAllowRun();
bool getModeAuto();
bool getHomeRun();
bool getAutodefaultMode();
bool getRollSpotMode();
bool getKneadingMode();
bool getCompressionMode();
bool getPercussionMode();
bool getCombineMode();
uint8_t getIntensityLevel();

// Sensor state getters
bool getSensorUpLimit();
bool getSensorDownLimit();
bool getSensorUpPending();
bool getSensorDownPending();
bool getSensorConfirmInProgress();

// State getter/setter
bool getRollDirectionState();
void setRollDirectionState(bool value);


// Global motor state getters and setters
// globalMotorRunning removed - use globalRL3PWMState instead
extern bool globalWaitingForSensor;         // Global variable for waiting for sensor state
extern bool globalSensorUpLimit;            // Global variable for sensor UP limit state
extern bool globalSensorDownLimit;          // Global variable for sensor DOWN limit state
extern bool globalSensorConfirmInProgress;  // Global variable for sensor confirmation in progress state
extern bool globalMotorStarted;             // Global variable for motor started state
extern bool globalRL3PWMState;              // Global variable for RL3_PWM state

// getMotorRunning() and setMotorRunning() removed - use getRL3PWMState() and setRL3PWMState() instead
bool getWaitingForSensor();
void setWaitingForSensor(bool state);
bool getSensorUpLimit();
void setSensorUpLimit(bool state);
bool getSensorDownLimit();
void setSensorDownLimit(bool state);
bool getSensorConfirmInProgress();
void setSensorConfirmInProgress(bool state);
bool getMotorStarted();
void setMotorStarted(bool state);
bool getRL3PWMState();
void setRL3PWMState(bool state);

// Setters - Change state with validation
void setAllowRun(bool value);
void setModeAuto(bool value);
void setHomeRun(bool value);
void setAutodefaultMode(bool value);
void setRollSpotMode(bool value);
void setKneadingMode(bool value);
void setCompressionMode(bool value);
void setPercussionMode(bool value);
void setCombineMode(bool value);
void setIntensityLevel(uint8_t value);

// Status check - Print all states
void printSystemStatus();

// Reset all mode flags
void resetAllModes();

// Motor control helpers
void controlRollMotor(MotorState state, MotorDirection dir);

// Program state getters and setters
bool getResetProgramStatesFlag();
void setResetProgramStatesFlag(bool value);
bool getDirectionChanging();
void setDirectionChanging(bool value);
unsigned long getDirChangeTick();
void setDirChangeTick(unsigned long value);
bool getAllowRun();
void setAllowRun(bool value);
bool getModeAuto();
void setModeAuto(bool value);
bool getHomeRun();
void setHomeRun(bool value);
bool getRollSpotMode();
void setRollSpotMode(bool value);
bool getChangeDir();
void setChangeDir(bool value);

// Auto Total Timer state
bool getAutoTotalTimerActive();             // Get auto total timer active state
void setAutoTotalTimerActive(bool value);   // Set auto total timer active state
bool getAutoTotalTimerExpired();            // Get auto total timer expired state
void setAutoTotalTimerExpired(bool value);  // Set auto total timer expired state

// Timer variables
unsigned long getTimerTicks();                   // Get master timer ticks (10ms)
void setTimerTicks(unsigned long value);         // Set master timer ticks
unsigned long getTimerTicks1ms();                // Get high precision timer ticks (1ms)
void setTimerTicks1ms(unsigned long value);      // Set high precision timer ticks
unsigned long getTimerTicks1msStep();            // Get step timer ticks
void setTimerTicks1msStep(unsigned long value);  // Set step timer ticks

// Motor timing variables
unsigned long getRl1StartTick();                 // Get RL1 motor start tick
void setRl1StartTick(unsigned long value);       // Set RL1 motor start tick
unsigned long getRl2StartTick();                 // Get RL2 motor start tick
void setRl2StartTick(unsigned long value);       // Set RL2 motor start tick
unsigned long getRl1DelayStartTick();            // Get RL1 delay start tick
void setRl1DelayStartTick(unsigned long value);  // Set RL1 delay start tick
unsigned long getRl2DelayStartTick();            // Get RL2 delay start tick
void setRl2DelayStartTick(unsigned long value);  // Set RL2 delay start tick

// Home sequence variables
unsigned long getHomeStartTick();                // Get home start tick
void setHomeStartTick(unsigned long value);      // Set home start tick
unsigned long getHomeStepStartTick();            // Get home step start tick
void setHomeStepStartTick(unsigned long value);  // Set home step start tick
unsigned long getHomeLastResetTick();            // Get home last reset tick
void setHomeLastResetTick(unsigned long value);  // Set home last reset tick
byte getStephomeRun();                           // Get home step
void setStephomeRun(byte value);                 // Set home step

// Auto mode variables
unsigned long getAutoLastDirChangeTick();            // Get auto last direction change tick
void setAutoLastDirChangeTick(unsigned long value);  // Set auto last direction change tick
unsigned long getAutoModeStartTick();                // Get auto mode start tick
void setAutoModeStartTick(unsigned long value);      // Set auto mode start tick
unsigned long getAutoModeElapsedTicks();             // Get auto mode elapsed ticks
void setAutoModeElapsedTicks(unsigned long value);   // Set auto mode elapsed ticks
bool getAutoModeTimerActive();                       // Get auto mode timer active
void setAutoModeTimerActive(bool value);             // Set auto mode timer active
bool getAutoTimerStarted();                          // Get auto timer started
void setAutoTimerStarted(bool value);                // Set auto timer started
unsigned long getAutoTotalStartTick();               // Get auto total start tick
void setAutoTotalStartTick(unsigned long value);     // Set auto total start tick
unsigned long getAutoTotalElapsedTicks();            // Get auto total elapsed ticks
void setAutoTotalElapsedTicks(unsigned long value);  // Set auto total elapsed ticks

// Sensor state variables
bool getSensorUpPending();                            // Get sensor up pending
void setSensorUpPending(bool value);                  // Set sensor up pending
bool getSensorDownPending();                          // Get sensor down pending
void setSensorDownPending(bool value);                // Set sensor down pending
unsigned long getSensorConfirmStartTick();            // Get sensor confirm start tick
void setSensorConfirmStartTick(unsigned long value);  // Set sensor confirm start tick
bool getDirectionChanging();                          // Get direction changing
void setDirectionChanging(bool value);                // Set direction changing
unsigned long getDirChangeTick();                     // Get direction change tick
void setDirChangeTick(unsigned long value);           // Set direction change tick

// Motor state variables
bool getRL1running();                            // Get RL1 motor running state
void setRL1running(bool value);                  // Set RL1 motor running state
bool getRL2running();                            // Get RL2 motor running state
void setRL2running(bool value);                  // Set RL2 motor running state
unsigned long getNowTimeRL1running();            // Get RL1 motor running time
void setNowTimeRL1running(unsigned long value);  // Set RL1 motor running time
unsigned long getNowTimeRL2running();            // Get RL2 motor running time
void setNowTimeRL2running(unsigned long value);  // Set RL2 motor running time

// Mode flags
bool getManualPriority();                   // Get manual priority flag
void setManualPriority(bool value);         // Set manual priority flag
bool getSystemStuck();                      // Get system stuck flag
void setSystemStuck(bool value);            // Set system stuck flag
bool getKneadingMode();                     // Get kneading mode flag
void setKneadingMode(bool value);           // Set kneading mode flag
bool getCompressionMode();                  // Get compression mode flag
void setCompressionMode(bool value);        // Set compression mode flag
bool getPercussionMode();                   // Get percussion mode flag
void setPercussionMode(bool value);         // Set percussion mode flag
bool getCombineMode();                      // Get combine mode flag
void setCombineMode(bool value);            // Set combine mode flag
uint8_t getIntensityLevel();                // Get intensity level
void setIntensityLevel(uint8_t value);      // Set intensity level
bool getUseHighPrecisionTimer();            // Get use high precision timer flag
void setUseHighPrecisionTimer(bool value);  // Set use high precision timer flag

// Program variables
AutoProgram getCurrentAutoProgram();                 // Get current auto program
void setCurrentAutoProgram(AutoProgram value);       // Set current auto program
AutoProgram getPreviousAutoProgram();                // Get previous auto program
void setPreviousAutoProgram(AutoProgram value);      // Set previous auto program
bool getResetProgramStatesFlag();                    // Get reset program states flag
void setResetProgramStatesFlag(bool value);          // Set reset program states flag
unsigned long getProgramSwitchCount();               // Get program switch count
void setProgramSwitchCount(unsigned long value);     // Set program switch count
unsigned long getLastProgramSwitchTick();            // Get last program switch tick
void setLastProgramSwitchTick(unsigned long value);  // Set last program switch tick

// State machine state getters/setters
AutoSequenceState getCurrentAutoSequenceState();                          // Get current auto sequence state
void setCurrentAutoSequenceState(AutoSequenceState value);                // Set current auto sequence state
KneadingSequenceState getCurrentKneadingSequenceState();                  // Get current kneading sequence state
void setCurrentKneadingSequenceState(KneadingSequenceState value);        // Set current kneading sequence state
CompressionSequenceState getCurrentCompressionSequenceState();            // Get current compression sequence state
void setCurrentCompressionSequenceState(CompressionSequenceState value);  // Set current compression sequence state
PercussionSequenceState getCurrentPercussionSequenceState();              // Get current percussion sequence state
void setCurrentPercussionSequenceState(PercussionSequenceState value);    // Set current percussion sequence state
CombinedSequenceState getCurrentCombinedSequenceState();                  // Get current combined sequence state
void setCurrentCombinedSequenceState(CombinedSequenceState value);        // Set current combined sequence state

// State machine timing and control getters/setters
// AUTO sequence timing
unsigned long getAutoSequenceStartTick();            // Get auto sequence start tick
void setAutoSequenceStartTick(unsigned long value);  // Set auto sequence start tick
unsigned long getAutoStopStartTick();                // Get auto stop start tick
void setAutoStopStartTick(unsigned long value);      // Set auto stop start tick
bool getAutoStopped();                               // Get auto stopped flag
void setAutoStopped(bool value);                     // Set auto stopped flag

// Static variables converted to global getters/setters
// Sensor confirmation variables
bool getGlobalPreviousUpState();
void setGlobalPreviousUpState(bool value);
bool getGlobalPreviousDownState();
void setGlobalPreviousDownState(bool value);
bool getGlobalRl1WaitingForDelay();
void setGlobalRl1WaitingForDelay(bool value);
bool getGlobalRl2WaitingForDelay();
void setGlobalRl2WaitingForDelay(bool value);

// Home sequence variables
unsigned long getGlobalLastDelayMsg();
void setGlobalLastDelayMsg(unsigned long value);
unsigned long getGlobalLastSearchDebug();
void setGlobalLastSearchDebug(unsigned long value);
bool getGlobalMotorStartedLocal();
void setGlobalMotorStartedLocal(bool value);
unsigned long getGlobalLastBlockMsg();
void setGlobalLastBlockMsg(unsigned long value);
bool getGlobalMotorDownStarted();
void setGlobalMotorDownStarted(bool value);
unsigned long getGlobalLastDownDelayMsg();
void setGlobalLastDownDelayMsg(unsigned long value);

// Sensor confirmation state
SensorConfirmState getGlobalConfirmState();
void setGlobalConfirmState(SensorConfirmState value);
bool getGlobalIsUpSensor();
void setGlobalIsUpSensor(bool value);
unsigned long getGlobalLastPendingDebugTick();
void setGlobalLastPendingDebugTick(unsigned long value);
unsigned long getGlobalLastFunctionDebugTick();
void setGlobalLastFunctionDebugTick(unsigned long value);
unsigned long getGlobalLastIdleDebugTick();
void setGlobalLastIdleDebugTick(unsigned long value);
unsigned long getGlobalLastWaitingDebugTick();
void setGlobalLastWaitingDebugTick(unsigned long value);
unsigned long getGlobalLastConfirmDebugTick();
void setGlobalLastConfirmDebugTick(unsigned long value);
unsigned long getGlobalLastConfirmedDebugTick();
void setGlobalLastConfirmedDebugTick(unsigned long value);

// Program sequence started flags
bool getGlobalAutoSequenceStarted();
void setGlobalAutoSequenceStarted(bool value);
bool getGlobalKneadingSequenceStarted();
void setGlobalKneadingSequenceStarted(bool value);
bool getGlobalCompressionSequenceStarted();
void setGlobalCompressionSequenceStarted(bool value);
bool getGlobalPercussionSequenceStarted();
void setGlobalPercussionSequenceStarted(bool value);
bool getGlobalCombinedSequenceStarted();
void setGlobalCombinedSequenceStarted(bool value);

// Debug timing variables
unsigned long getGlobalLastKneadingDebugTick();
void setGlobalLastKneadingDebugTick(unsigned long value);
unsigned long getGlobalLastCompressionDebugTick();
void setGlobalLastCompressionDebugTick(unsigned long value);
unsigned long getGlobalLastPercussionDebugTick();
void setGlobalLastPercussionDebugTick(unsigned long value);
unsigned long getGlobalLastCombinedDebugTick();
void setGlobalLastCombinedDebugTick(unsigned long value);
unsigned long getGlobalLastExpirationTick();
void setGlobalLastExpirationTick(unsigned long value);
unsigned long getGlobalLastDebugTick();
void setGlobalLastDebugTick(unsigned long value);
unsigned long getGlobalLastTotalDebugTick();
void setGlobalLastTotalDebugTick(unsigned long value);

// Home sequence getters/setters
byte getStephomeRun();
void setStephomeRun(byte value);

// Manual priority getters/setters
bool getManualPriority();
void setManualPriority(bool value);

// Intensity level getters/setters
uint8_t getIntensityLevel();
void setIntensityLevel(uint8_t value);

// Motor timing getters/setters
unsigned long getRl1StartTick();
void setRl1StartTick(unsigned long value);
unsigned long getRl2StartTick();
void setRl2StartTick(unsigned long value);
unsigned long getRl1DelayStartTick();
void setRl1DelayStartTick(unsigned long value);
unsigned long getRl2DelayStartTick();
void setRl2DelayStartTick(unsigned long value);

// Home sequence timing getters/setters
unsigned long getHomeStartTick();
void setHomeStartTick(unsigned long value);
unsigned long getHomeStepStartTick();
void setHomeStepStartTick(unsigned long value);
unsigned long getHomeLastResetTick();
void setHomeLastResetTick(unsigned long value);

// Auto mode timing getters/setters
unsigned long getAutoLastDirChangeTick();
void setAutoLastDirChangeTick(unsigned long value);
unsigned long getAutoModeStartTick();
void setAutoModeStartTick(unsigned long value);
unsigned long getAutoModeElapsedTicks();
void setAutoModeElapsedTicks(unsigned long value);
bool getAutoModeTimerActive();
void setAutoModeTimerActive(bool value);
bool getAutoTimerStarted();
void setAutoTimerStarted(bool value);

// Auto total timer getters/setters
unsigned long getAutoTotalStartTick();
void setAutoTotalStartTick(unsigned long value);
unsigned long getAutoTotalElapsedTicks();
void setAutoTotalElapsedTicks(unsigned long value);

// Motor state getters/setters
bool getRL1running();
void setRL1running(bool value);
bool getRL2running();
void setRL2running(bool value);
unsigned long getNowTimeRL1running();
void setNowTimeRL1running(unsigned long value);
unsigned long getNowTimeRL2running();
void setNowTimeRL2running(unsigned long value);

// System state getters/setters
bool getSystemStuck();
void setSystemStuck(bool value);
AutoProgram getCurrentAutoProgram();
void setCurrentAutoProgram(AutoProgram value);
AutoProgram getPreviousAutoProgram();
void setPreviousAutoProgram(AutoProgram value);
unsigned long getProgramSwitchCount();
void setProgramSwitchCount(unsigned long value);
unsigned long getLastProgramSwitchTick();
void setLastProgramSwitchTick(unsigned long value);

// Loop timing getters/setters
unsigned long getLastLoopTick();
void setLastLoopTick(unsigned long value);
unsigned long getLoopCounter();
void setLoopCounter(unsigned long value);

// Sensor state getters/setters
uint8_t getButtonUpSamples();
void setButtonUpSamples(uint8_t value);
uint8_t getButtonDownSamples();
void setButtonDownSamples(uint8_t value);
bool getLastUpState();
void setLastUpState(bool value);
bool getLastDownState();
void setLastDownState(bool value);

// Sensor confirmation timing getters/setters
unsigned long getSensorConfirmStartTick();
void setSensorConfirmStartTick(unsigned long value);

// KNEADING sequence timing
unsigned long getKneadingSequenceStartTick();            // Get kneading sequence start tick
void setKneadingSequenceStartTick(unsigned long value);  // Set kneading sequence start tick
unsigned long getKneadingStopStartTick();                // Get kneading stop start tick
void setKneadingStopStartTick(unsigned long value);      // Set kneading stop start tick
bool getKneadingStopped();                               // Get kneading stopped flag
void setKneadingStopped(bool value);                     // Set kneading stopped flag

// COMPRESSION sequence timing
unsigned long getCompressionSequenceStartTick();            // Get compression sequence start tick
void setCompressionSequenceStartTick(unsigned long value);  // Set compression sequence start tick
unsigned long getCompressionStopStartTick();                // Get compression stop start tick
void setCompressionStopStartTick(unsigned long value);      // Set compression stop start tick
bool getCompressionStopped();                               // Get compression stopped flag
void setCompressionStopped(bool value);                     // Set compression stopped flag

// PERCUSSION sequence timing
unsigned long getPercussionSequenceStartTick();            // Get percussion sequence start tick
void setPercussionSequenceStartTick(unsigned long value);  // Set percussion sequence start tick
unsigned long getPercussionStopStartTick();                // Get percussion stop start tick
void setPercussionStopStartTick(unsigned long value);      // Set percussion stop start tick
bool getPercussionStopped();                               // Get percussion stopped flag
void setPercussionStopped(bool value);                     // Set percussion stopped flag

// COMBINED sequence timing
unsigned long getCombinedSequenceStartTick();            // Get combined sequence start tick
void setCombinedSequenceStartTick(unsigned long value);  // Set combined sequence start tick
unsigned long getCombinedStopStartTick();                // Get combined stop start tick
void setCombinedStopStartTick(unsigned long value);      // Set combined stop start tick
bool getCombinedStopped();                               // Get combined stopped flag
void setCombinedStopped(bool value);                     // Set combined stopped flag

#endif  // MASSAGE_V1_HARDWARE_H
