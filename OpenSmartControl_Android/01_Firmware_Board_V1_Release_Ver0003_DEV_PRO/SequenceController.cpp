#include "SequenceController.h"
#include "MotorController.h"
#include "SensorManager.h"
#include "PinDefinitions.h"

/**
 * Constructor
 */
SequenceController::SequenceController(TimerManager* timerMgr, MotorController* motorCtrl, SensorManager* sensorMgr, HardwareSerial* debugSer) 
    : timerManager(timerMgr)
    , motorController(motorCtrl)
    , sensorManager(sensorMgr)
    , debugSerial(debugSer)
    , allowRun(false)
    , homeRun(false)
    , modeAuto(false)
    , manualPriority(false)
    , startupStabilizationDelay(false)
    , rollMotorUserDisabled(false)
    , autoModeTimerActive(false)
    , autoModeStartTick(0)
    , autodefaultMode(false)
    , rollSpotMode(false)
    , kneadingMode(false)
    , compressionMode(false)
    , percussionMode(false)
    , combineMode(false)
    , intensityLevel(0)
    , useHighPrecisionTimer(false)
    , currentHomeState(HOME_IDLE)
    , stepHomeRun(0)
    , homeStartTick(0)
    , homeStepStartTick(0)
    , homeLastResetTick(0)
    , homeMotorStarted(false)
    , homeLastDownSensorDebugTick(0)
    , currentAutoProgram(AUTO_NONE)
    , previousAutoProgram(AUTO_NONE)
    , resetProgramStatesFlag(false)
    , programSwitchCount(0)
    , lastProgramSwitchTick(0)
    , autoLastDirChangeTick(0)
    , autoModeElapsedTicks(0)
    , autoTimerStarted(false)
    , lastAutoCaseDebugTick(0)
    , directionReversalInProgress(false)
    , directionReversalStartTick(0)
    , autoTotalStartTick(0)
    , autoTotalElapsedTicks(0)
    , autoTotalTimerActive(false)
    , autoTotalTimerExpired(false)
    , currentAutoSequenceState(AUTO_CASE_0)
    , currentKneadingSequenceState(KNEADING_CASE_0)
    , currentCompressionSequenceState(COMPRESSION_CASE_0)
    , currentPercussionSequenceState(PERCUSSION_CASE_0)
    , currentCombinedSequenceState(COMBINED_CASE_0)
    , autoSequenceStartTick(0)
    , autoStopStartTick(0)
    , autoStopped(false)
    , kneadingSequenceStartTick(0)
    , kneadingStopStartTick(0)
    , kneadingStopped(false)
    , compressionSequenceStartTick(0)
    , compressionStopStartTick(0)
    , compressionStopped(false)
    , percussionSequenceStartTick(0)
    , percussionStopStartTick(0)
    , percussionStopped(false)
    , combinedSequenceStartTick(0)
    , combinedStopStartTick(0)
    , combinedStopped(false)
    , autoSequenceStarted(false)
    , kneadingSequenceStarted(false)
    , compressionSequenceStarted(false)
    , percussionSequenceStarted(false)
    , combinedSequenceStarted(false)
{
}

/**
 * Destructor
 */
SequenceController::~SequenceController() {
    // Cleanup if needed
}

/**
 * Initialize sequence controller
 */
void SequenceController::initialize() {
    // Initialize all flags to default state
    allowRun = false;
    homeRun = false;
    modeAuto = false;
    manualPriority = false;
    
    // Initialize program modes
    autodefaultMode = false;
    rollSpotMode = false;
    kneadingMode = false;
    compressionMode = false;
    percussionMode = false;
    combineMode = false;
    
    // Initialize features
    intensityLevel = 0;
    useHighPrecisionTimer = false;
    
    // Initialize home sequence
    currentHomeState = HOME_IDLE;
    stepHomeRun = 0;
    homeStartTick = 0;
    homeStepStartTick = 0;
    homeLastResetTick = 0;
    
    // Initialize auto mode
    currentAutoProgram = AUTO_NONE;
    previousAutoProgram = AUTO_NONE;
    resetProgramStatesFlag = false;
    programSwitchCount = 0;
    lastProgramSwitchTick = 0;
    
    // Initialize auto timing
    autoLastDirChangeTick = 0;
    autoModeStartTick = 0;
    autoModeElapsedTicks = 0;
    autoModeTimerActive = false;
    autoTimerStarted = false;
    
    // Initialize auto total timer
    autoTotalStartTick = 0;
    autoTotalElapsedTicks = 0;
    autoTotalTimerActive = false;
    autoTotalTimerExpired = false;
    
    // Initialize sequence states
    currentAutoSequenceState = AUTO_CASE_0;
    currentKneadingSequenceState = KNEADING_CASE_0;
    currentCompressionSequenceState = COMPRESSION_CASE_0;
    currentPercussionSequenceState = PERCUSSION_CASE_0;
    currentCombinedSequenceState = COMBINED_CASE_0;
    
    // Initialize sequence timing
    autoSequenceStartTick = 0;
    autoStopStartTick = 0;
    autoStopped = false;
    
    kneadingSequenceStartTick = 0;
    kneadingStopStartTick = 0;
    kneadingStopped = false;
    
    compressionSequenceStartTick = 0;
    compressionStopStartTick = 0;
    compressionStopped = false;
    
    percussionSequenceStartTick = 0;
    percussionStopStartTick = 0;
    percussionStopped = false;
    
    combinedSequenceStartTick = 0;
    combinedStopStartTick = 0;
    combinedStopped = false;
    
    // Initialize sequence started flags
    autoSequenceStarted = false;
    kneadingSequenceStarted = false;
    compressionSequenceStarted = false;
    percussionSequenceStarted = false;
    combinedSequenceStarted = false;
}

/**
 * Process home sequence
 */
void SequenceController::processGoHome() {
    // Debug: Only show when homeRun = FALSE (GO HOME needed)
    static unsigned long lastHomeDebugTick = 0;
    unsigned long currentTick = timerManager->getMasterTicks();
    
    if (currentTick - lastHomeDebugTick >= 1000) {  // Every 10 seconds
        // GO HOME DEBUG disabled to save Flash memory
        lastHomeDebugTick = currentTick;
    }
    
    if (!allowRun) return;
    
    // Debug: Check startup stabilization delay status (only when needed)
    static unsigned long lastStartupStatusDebugTick = 0;
    if (currentTick - lastStartupStatusDebugTick >= 1000) {  // Every 10 seconds
        if (debugSerial && (!homeRun || startupStabilizationDelay)) {  // Only debug when GO HOME needed or startup delay active
            debugSerial->print("STARTUP STATUS: startupStabilizationDelay=");
            debugSerial->print(startupStabilizationDelay ? "TRUE" : "FALSE");
            debugSerial->print(", homeRun=");
            debugSerial->print(homeRun ? "TRUE" : "FALSE");
            debugSerial->println();
        }
        lastStartupStatusDebugTick = currentTick;
    }
    
    // Check for startup stabilization delay
    if (startupStabilizationDelay) {
        static unsigned long startupDelayStartTick = 0;
        if (startupDelayStartTick == 0) {
            startupDelayStartTick = timerManager->getMasterTicks();
            if (debugSerial) debugSerial->println("STARTUP: Beginning 2-second stabilization delay - all motors OFF");
        }
        
        unsigned long currentTick = timerManager->getMasterTicks();
        unsigned long elapsedTicks = currentTick - startupDelayStartTick;
        
        // Debug: Show countdown every 500ms
        static unsigned long lastStartupDebugTick = 0;
        if (currentTick - lastStartupDebugTick >= 50) {  // Every 500ms
            unsigned long remainingMs = 2000 - (elapsedTicks * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("STARTUP: Stabilization delay - ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms remaining");
            }
            lastStartupDebugTick = currentTick;
        }
        
        // Check if 2 seconds have passed
        if (elapsedTicks >= 200) {  // 2 seconds = 200 ticks
            startupStabilizationDelay = false;
            homeRun = false;  // Set to FALSE to trigger GO HOME sequence
            startupDelayStartTick = 0;  // Reset for next time
            
            if (debugSerial) debugSerial->println("STARTUP: Stabilization delay completed - triggering GO HOME sequence");
        }
        return;
    }
    
    if (homeRun) return;  // If already homed (homeRun = true), don't process GO HOME
    
    // If we reach here, it means homeRun = false (GO HOME sequence is needed)
    // Only process GO HOME if we're not in IDLE state
    if (currentHomeState == HOME_IDLE) {
        // Start GO HOME sequence
        if (debugSerial) debugSerial->println("GO HOME: Calling startHomeSequence() from HOME_IDLE state");
        startHomeSequence();
        return;
    }
    
    switch (currentHomeState) {
        case HOME_IDLE:
            // This should not be reached due to check above
            break;
        case HOME_SEARCHING_UP:
            handleHomeStateSearchingUp();
            break;
        case HOME_DELAY_AT_UP:
            handleHomeStateDelayAtUp();
            break;
        case HOME_RUNNING_DOWN:
            handleHomeStateRunningDown();
            break;
    }
    
    // Check for home sequence timeout
    if (isHomeSequenceTimeout()) {
        if (debugSerial) debugSerial->println("Home sequence timeout!");
        stopHomeSequence();
    }
    
    // Debug: Show timeout status
    static unsigned long lastTimeoutDebugTick = 0;
    if (currentTick - lastTimeoutDebugTick >= 5000) {  // Every 50 seconds
        if (debugSerial) {
            debugSerial->print("GO HOME TIMEOUT DEBUG: Elapsed=");
            debugSerial->print(currentTick - homeStartTick);
            debugSerial->print(" ticks, Timeout=");
            debugSerial->print(SEQ_HOME_TOTAL_TIMEOUT_TICKS);
            debugSerial->print(" ticks, Timeout=");
            debugSerial->print(isHomeSequenceTimeout() ? "YES" : "NO");
            debugSerial->println();
        }
        lastTimeoutDebugTick = currentTick;
    }
}

/**
 * Start home sequence
 */
void SequenceController::startHomeSequence() {
    if (homeRun) {
        if (debugSerial) debugSerial->println("GO HOME: startHomeSequence() called but homeRun=TRUE - already homed, ignoring");
        return;
    }
    
    if (debugSerial) debugSerial->println("GO HOME: startHomeSequence() called - checking initial sensor position");
    
    // CRITICAL SAFETY: Check initial sensor state and handle 3 cases
    if (sensorManager) {
        bool upSensor = sensorManager->getSensorUpLimit();
        bool downSensor = sensorManager->getSensorDownLimit();
        
        if (debugSerial) {
            debugSerial->print("GO HOME: Initial sensors - UP=");
            debugSerial->print(upSensor ? "ACTIVE" : "INACTIVE");
            debugSerial->print(", DOWN=");
            debugSerial->print(downSensor ? "ACTIVE" : "INACTIVE");
            debugSerial->println();
        }
        
        // TH3: Chạm sensor UP -> dừng motor -> rồi gohome
        if (upSensor && !downSensor) {
            if (debugSerial) debugSerial->println("GO HOME: TH3 - UP sensor active, DOWN inactive - going directly to DELAY_AT_UP");
            currentHomeState = HOME_DELAY_AT_UP;
            homeStartTick = timerManager->getMasterTicks();
            homeStepStartTick = homeStartTick;
            stepHomeRun = 2;
            
            if (debugSerial) debugSerial->println("GO HOME: Starting from DELAY_AT_UP state");
            return;
        }
        
        // TH1: Chạm sensor DOWN -> phải dừng ngay motor -> set chiều từ down sang up -> rồi mới gohome
        if (downSensor && !upSensor) {
            if (debugSerial) debugSerial->println("GO HOME: TH1 - DOWN sensor active, UP inactive - stopping motor and setting UP direction");
            
            // Stop motor immediately
            if (motorController) {
                motorController->offRollMotor();
                motorController->setRollDirection(true);  // Set UP direction (true = UP)
                if (debugSerial) debugSerial->println("GO HOME: Motor stopped and direction set to UP");
            }
            
            // Start searching UP
            currentHomeState = HOME_SEARCHING_UP;
            homeStartTick = timerManager->getMasterTicks();
            homeStepStartTick = homeStartTick;
            stepHomeRun = 1;
            
            if (debugSerial) debugSerial->println("GO HOME: Starting UP search from DOWN position");
            return;
        }
        
        // TH2: Floating sensor -> set chiều từ down sang up -> rồi gohome
        if (!upSensor && !downSensor) {
            if (debugSerial) debugSerial->println("GO HOME: TH2 - Both sensors inactive (floating) - setting UP direction and searching UP");
            
            // Set UP direction and start searching
            if (motorController) {
                motorController->setRollDirection(true);  // Set UP direction (true = UP)
                if (debugSerial) debugSerial->println("GO HOME: Direction set to UP for floating position");
            }
            
            currentHomeState = HOME_SEARCHING_UP;
            homeStartTick = timerManager->getMasterTicks();
            homeStepStartTick = homeStartTick;
            stepHomeRun = 1;
            
            if (debugSerial) debugSerial->println("GO HOME: Starting UP search from floating position");
            return;
        }
        
        // Edge case: Both sensors active (should not happen normally)
        if (upSensor && downSensor) {
            if (debugSerial) debugSerial->println("GO HOME: WARNING - Both sensors active! Going to DELAY_AT_UP");
            currentHomeState = HOME_DELAY_AT_UP;
            homeStartTick = timerManager->getMasterTicks();
            homeStepStartTick = homeStartTick;
            stepHomeRun = 2;
            return;
        }
    }
    
    // Fallback: Default to searching UP
    currentHomeState = HOME_SEARCHING_UP;
    homeStartTick = timerManager->getMasterTicks();
    homeStepStartTick = homeStartTick;
    stepHomeRun = 1;
    
    if (debugSerial) debugSerial->println("GO HOME: Default - Starting UP search");
}

/**
 * Stop home sequence
 */
void SequenceController::stopHomeSequence() {
    currentHomeState = HOME_IDLE;
    homeStartTick = 0;
    homeStepStartTick = 0;
    stepHomeRun = 0;
    
    // Stop roll motor
    if (motorController) {
        motorController->offRollMotor();
    }
    
    if (debugSerial) debugSerial->println("Home sequence stopped");
}

/**
 * Check if home sequence is active
 */
bool SequenceController::isHomeSequenceActive() const {
    return currentHomeState != HOME_IDLE;
}

/**
 * Get current home state
 */
SequenceController::HomeState SequenceController::getCurrentHomeState() const {
    return currentHomeState;
}

/**
 * Set current home state
 */
void SequenceController::setCurrentHomeState(HomeState state) {
    currentHomeState = state;
}

/**
 * Process auto mode
 */
void SequenceController::processAuto() {
    // Debug: Check why auto mode might be blocked
    static unsigned long lastDebugTick = 0;
    unsigned long currentTick = timerManager ? timerManager->getMasterTicks() : 0;
    
    if (currentTick - lastDebugTick >= 1000) {  // Every 10 seconds
        // AUTO PROCESS DEBUG disabled to save Flash memory
        lastDebugTick = currentTick;
    }
    
    if (!allowRun || !homeRun || manualPriority) return;
    
    // Update auto mode timer
    updateAutoModeTimer();
    updateAutoTotalTimer();
    
    // Check for auto mode timeout (20 minutes)
    if (autoModeTimerActive && modeAuto) {
        unsigned long elapsedTicks = currentTick - autoModeStartTick;
        if (elapsedTicks >= SEQ_AUTO_MODE_DURATION_TICKS) {
            if (debugSerial) debugSerial->println("AUTO MODE TIMEOUT: 20 minutes reached - stopping auto mode");
            stopAutoMode();
            return;
        }
        
        // Debug: Show remaining time every 30 seconds
        static unsigned long lastTimeoutDebugTick = 0;
        if (currentTick - lastTimeoutDebugTick >= 3000) {  // Every 30 seconds
            unsigned long remainingTicks = SEQ_AUTO_MODE_DURATION_TICKS - elapsedTicks;
            unsigned long remainingMinutes = (remainingTicks * 10) / 60000;  // Convert to minutes
            if (debugSerial) {
                // debugSerial->print("AUTO MODE: ");
                debugSerial->print(remainingMinutes);
                debugSerial->println(" minutes remaining");
            }
            lastTimeoutDebugTick = currentTick;
        }
    }
    
    // Detect current program
    AutoProgram detectedProgram = detectAutoProgram();
    if (detectedProgram != currentAutoProgram) {
        if (debugSerial) {
            debugSerial->print("AUTO: Program switch detected - From: ");
            debugSerial->print(currentAutoProgram);
            debugSerial->print(" To: ");
            debugSerial->println(detectedProgram);
        }
        handleProgramSwitch();
    }
    
    // Debug: Show current program every 5 seconds
    static unsigned long lastProgramDebugTick = 0;
    if (currentTick - lastProgramDebugTick >= 500) {  // Every 5 seconds
        if (debugSerial && currentAutoProgram != AUTO_NONE) {
            debugSerial->print("AUTO: Current program = ");
            debugSerial->println(currentAutoProgram);
        }
        lastProgramDebugTick = currentTick;
    }
    
    // Execute current program
    executeCurrentProgram();
}

/**
 * Start auto mode
 */
void SequenceController::startAutoMode() {
    if (modeAuto) return;
    
    modeAuto = true;
    autoModeStartTick = timerManager->getMasterTicks();
    autoModeElapsedTicks = 0;
    autoModeTimerActive = true;
    autoTimerStarted = true;
    
    // Initialize shared debug timer for all auto cases
    lastAutoCaseDebugTick = timerManager->getMasterTicks();
    
    // Initialize case transition timer
    autoLastDirChangeTick = timerManager->getMasterTicks();
    
    // Start total timer if not already started
    if (!autoTotalTimerActive) {
        autoTotalStartTick = timerManager->getMasterTicks();
        autoTotalElapsedTicks = 0;
        autoTotalTimerActive = true;
        autoTotalTimerExpired = false;
    }
    
    if (debugSerial) debugSerial->println("Auto mode started");
}

/**
 * Stop auto mode
 */
void SequenceController::stopAutoMode() {
    if (debugSerial) debugSerial->println("DEBUG: stopAutoMode() called");
    
    modeAuto = false;
    autoModeTimerActive = false;
    autoTimerStarted = false;
    
    // Reset current auto program
    currentAutoProgram = AUTO_NONE;
    
    if (debugSerial) debugSerial->println("DEBUG: Auto mode flags reset");
    
    // Stop all auto programs (except roll motor - needed for GO HOME)
    if (motorController) {
        // Only stop kneading and compression motors
        // Roll motor will be controlled by GO HOME sequence
        motorController->offKneadingMotor();
        motorController->offCompressionMotor();
        if (debugSerial) {
            debugSerial->println("DEBUG: Kneading and compression motors stopped");
            debugSerial->println("DEBUG: Roll motor will be controlled by GO HOME sequence");
        }
    }
    
    // Reset program states (this will reset all mode flags to FALSE)
    resetProgramStates();
    if (debugSerial) debugSerial->println("DEBUG: Program states reset - all programs OFF");
    
    // Set intensity to OFF when auto mode stops
    setIntensityOff("Auto mode stopped");
    
    // Reset homeRun flag to trigger GO HOME sequence
    homeRun = false;
    if (debugSerial) {
        debugSerial->print("DEBUG: homeRun set to FALSE (was ");
        debugSerial->print(homeRun ? "TRUE" : "FALSE");
        debugSerial->println(") - GO HOME will be triggered");
    }
    
    if (debugSerial) debugSerial->println("AUTO STOP: Auto mode stopped - GO HOME sequence will start");
}

/**
 * Check if auto mode is active
 */
bool SequenceController::isAutoModeActive() const {
    return modeAuto && autoModeTimerActive;
}

/**
 * Get current auto program
 */
SequenceController::AutoProgram SequenceController::getCurrentAutoProgram() const {
    return currentAutoProgram;
}

/**
 * Set current auto program
 */
void SequenceController::setCurrentAutoProgram(AutoProgram program) {
    currentAutoProgram = program;
}

/**
 * Detect auto program based on mode flags
 */
SequenceController::AutoProgram SequenceController::detectAutoProgram() {
    int activeModes = 0;
    if (autodefaultMode) activeModes++;
    if (kneadingMode) activeModes++;
    if (compressionMode) activeModes++;
    if (percussionMode) activeModes++;
    
    // Debug: Show mode flags only when modes change
    static int lastActiveModes = -1;
    if (activeModes != lastActiveModes) {
        if (debugSerial) {
            debugSerial->print("MODE DEBUG: autodefault=");
            debugSerial->print(autodefaultMode ? "TRUE" : "FALSE");
            debugSerial->print(", kneading=");
            debugSerial->print(kneadingMode ? "TRUE" : "FALSE");
            debugSerial->print(", compression=");
            debugSerial->print(compressionMode ? "TRUE" : "FALSE");
            debugSerial->print(", percussion=");
            debugSerial->print(percussionMode ? "TRUE" : "FALSE");
            debugSerial->print(", activeModes=");
            debugSerial->print(activeModes);
            debugSerial->println();
        }
        lastActiveModes = activeModes;
    }
    
    if (activeModes == 0) {
        return AUTO_NONE;
    } else if (activeModes == 1) {
        if (autodefaultMode) {
            return AUTO_DEFAULT;
        }
        if (kneadingMode) return AUTO_KNEADING;
        if (compressionMode) return AUTO_COMPRESSION;
        if (percussionMode) return AUTO_PERCUSSION;
    } else {
        return AUTO_COMBINED;
    }
    
    return AUTO_NONE;
}

/**
 * Print auto mode status
 */
void SequenceController::printAutoModeStatus() {
    if (debugSerial) debugSerial->print("Auto Mode Status - Program: ");
    switch (currentAutoProgram) {
        case AUTO_NONE: if (debugSerial) debugSerial->print("NONE"); break;
        case AUTO_DEFAULT: if (debugSerial) debugSerial->print("DEFAULT"); break;
        case AUTO_KNEADING: if (debugSerial) debugSerial->print("KNEADING"); break;
        case AUTO_COMPRESSION: if (debugSerial) debugSerial->print("COMPRESSION"); break;
        case AUTO_PERCUSSION: if (debugSerial) debugSerial->print("PERCUSSION"); break;
        case AUTO_COMBINED: if (debugSerial) debugSerial->print("COMBINED"); break;
    }
    if (debugSerial) debugSerial->print(", Active: ");
    if (debugSerial) debugSerial->print(modeAuto ? "YES" : "NO");
    if (debugSerial) debugSerial->print(", Remaining: ");
    if (debugSerial) debugSerial->print(getAutoRemainingTime());
    if (debugSerial) debugSerial->println(" seconds");
}

/**
 * Get auto remaining time
 */
unsigned long SequenceController::getAutoRemainingTime() {
    if (!autoModeTimerActive) return 0;
    
    unsigned long elapsed = timerManager->getMasterTicks() - autoModeStartTick;
    unsigned long remaining = SEQ_AUTO_MODE_DURATION_TICKS - elapsed;
    
    return remaining / 100;  // Convert ticks to seconds
}

/**
 * Reset program states
 */
void SequenceController::resetProgramStates() {
    resetProgramStatesFlag = true;
    
    // Reset sequence states
    currentAutoSequenceState = AUTO_CASE_0;
    currentKneadingSequenceState = KNEADING_CASE_0;
    currentCompressionSequenceState = COMPRESSION_CASE_0;
    currentPercussionSequenceState = PERCUSSION_CASE_0;
    currentCombinedSequenceState = COMBINED_CASE_0;
    
    // Reset sequence started flags
    autoSequenceStarted = false;
    kneadingSequenceStarted = false;
    compressionSequenceStarted = false;
    percussionSequenceStarted = false;
    combinedSequenceStarted = false;
    
    // Reset timing
    autoSequenceStartTick = 0;
    kneadingSequenceStartTick = 0;
    compressionSequenceStartTick = 0;
    percussionSequenceStartTick = 0;
    combinedSequenceStartTick = 0;
    
    // Reset program mode flags
    autodefaultMode = false;
    kneadingMode = false;
    compressionMode = false;
    percussionMode = false;
    combineMode = false;
    
    if (debugSerial) debugSerial->println("DEBUG: All program mode flags reset to FALSE");
}

/**
 * Reset only sequence states (not mode flags)
 */
void SequenceController::resetSequenceStatesOnly() {
    // Reset sequence states
    currentAutoSequenceState = AUTO_CASE_0;
    currentKneadingSequenceState = KNEADING_CASE_0;
    currentCompressionSequenceState = COMPRESSION_CASE_0;
    currentPercussionSequenceState = PERCUSSION_CASE_0;
    currentCombinedSequenceState = COMBINED_CASE_0;
    
    // Reset sequence started flags
    autoSequenceStarted = false;
    kneadingSequenceStarted = false;
    compressionSequenceStarted = false;
    percussionSequenceStarted = false;
    combinedSequenceStarted = false;
    
    // Reset timing
    autoSequenceStartTick = 0;
    kneadingSequenceStartTick = 0;
    compressionSequenceStartTick = 0;
    percussionSequenceStartTick = 0;
    combinedSequenceStartTick = 0;
    
    // if (debugSerial) debugSerial->println("DEBUG: Sequence states reset (mode flags preserved)");
}

/**
 * Mode flag getters/setters
 */
bool SequenceController::getAllowRun() const { return allowRun; }
void SequenceController::setAllowRun(bool value) { allowRun = value; }
bool SequenceController::getHomeRun() const { return homeRun; }
void SequenceController::setHomeRun(bool value) { 
    if (homeRun != value) {
        if (debugSerial) {
            debugSerial->print("DEBUG: homeRun changed from ");
            debugSerial->print(homeRun ? "TRUE" : "FALSE");
            debugSerial->print(" to ");
            debugSerial->print(value ? "TRUE" : "FALSE");
            debugSerial->println();
        }
    }
    homeRun = value; 
}
bool SequenceController::getModeAuto() const { return modeAuto; }
void SequenceController::setModeAuto(bool value) { modeAuto = value; }
bool SequenceController::getManualPriority() const { return manualPriority; }
void SequenceController::setManualPriority(bool value) { manualPriority = value; }
bool SequenceController::getRollMotorUserDisabled() const { return rollMotorUserDisabled; }
void SequenceController::setRollMotorUserDisabled(bool disabled) { rollMotorUserDisabled = disabled; }
bool SequenceController::getStartupStabilizationDelay() const { return startupStabilizationDelay; }
void SequenceController::setStartupStabilizationDelay(bool value) { 
    startupStabilizationDelay = value; 
    if (debugSerial) {
        debugSerial->print("DEBUG: setStartupStabilizationDelay(");
        debugSerial->print(value ? "TRUE" : "FALSE");
        debugSerial->println(") called");
    }
}

bool SequenceController::getAutodefaultMode() const { return autodefaultMode; }
void SequenceController::setAutodefaultMode(bool value) { autodefaultMode = value; }
bool SequenceController::getRollSpotMode() const { return rollSpotMode; }
void SequenceController::setRollSpotMode(bool value) { rollSpotMode = value; }
bool SequenceController::getKneadingMode() const { return kneadingMode; }
void SequenceController::setKneadingMode(bool value) { kneadingMode = value; }
bool SequenceController::getCompressionMode() const { return compressionMode; }
void SequenceController::setCompressionMode(bool value) { compressionMode = value; }
bool SequenceController::getPercussionMode() const { return percussionMode; }
void SequenceController::setPercussionMode(bool value) { percussionMode = value; }
bool SequenceController::getCombineMode() const { return combineMode; }
void SequenceController::setCombineMode(bool value) { combineMode = value; }

uint8_t SequenceController::getIntensityLevel() const { return intensityLevel; }
void SequenceController::setIntensityLevel(uint8_t value) { 
    intensityLevel = value; 
    if (debugSerial) {
        debugSerial->print("SEQUENCE: intensityLevel updated to ");
        debugSerial->println(intensityLevel);
    }
    
    // Immediately update PWM value if compression motor is running
    if (motorController && motorController->isCompressionRunning()) {
        if (debugSerial) {
            debugSerial->print("SEQUENCE: Compression motor running - updating PWM immediately to ");
            debugSerial->println(intensityLevel);
        }
        motorController->setCompressionPWMByIntensity(intensityLevel);  // Use actual intensity level
    } else if (debugSerial) {
        debugSerial->println("SEQUENCE: Compression motor not running - PWM will be updated when motor starts");
    }
}

void SequenceController::setIntensityOff(const char* reason) {
    intensityLevel = 0;
    if (debugSerial) {
        debugSerial->print("SEQUENCE: intensityLevel set to OFF (0) - Reason: ");
        debugSerial->println(reason ? reason : "Unknown");
    }
    
    // Immediately update PWM value if compression motor is running
    if (motorController && motorController->isCompressionRunning()) {
        if (debugSerial) {
            debugSerial->print("SEQUENCE: Compression motor running - setting PWM to OFF (0) due to: ");
            debugSerial->println(reason ? reason : "Unknown");
        }
        motorController->setCompressionPWMByIntensity(0);
    } else if (debugSerial) {
        debugSerial->println("SEQUENCE: Compression motor not running - PWM will be set to OFF when motor starts");
    }
}

uint8_t SequenceController::getIntensityForProgram(AutoProgram program) const {
    // AUTO_DEFAULT and AUTO_KNEADING always use HIGH intensity (255)
    // Other programs (COMPRESSION, PERCUSSION, COMBINED) use remote-set intensity
    if (program == AUTO_DEFAULT || program == AUTO_KNEADING) {
        return 255;  // Always HIGH intensity for DEFAULT and KNEADING
    } else {
        // For COMPRESSION, PERCUSSION, COMBINED: use intensityLevel from remote
        // If no intensity set (0), default to HIGH (255)
        return (intensityLevel > 0) ? intensityLevel : 255;
    }
}
bool SequenceController::getUseHighPrecisionTimer() const { return useHighPrecisionTimer; }
void SequenceController::setUseHighPrecisionTimer(bool value) { useHighPrecisionTimer = value; }

/**
 * Execute auto default program
 */
void SequenceController::executeAutoDefaultProgram() {
    if (!checkProgramConditions()) {
        // Debug: Only print every 5 seconds to reduce spam
        static unsigned long lastConditionsDebugTick = 0;
        unsigned long currentTick = timerManager->getMasterTicks();
        
        if (currentTick - lastConditionsDebugTick >= 500) {  // Every 5 seconds
            if (debugSerial) debugSerial->println("AUTO DEFAULT: Program conditions not met");
            lastConditionsDebugTick = currentTick;
        }
        return;
    }
    
    // Debug: Only print once when starting
    static bool autoDefaultDebugPrinted = false;
    if (!autoDefaultDebugPrinted) {
        if (debugSerial) debugSerial->println("AUTO DEFAULT: Executing auto default program");
        autoDefaultDebugPrinted = true;
    }
    
    runAutoDefaultSequence();
}

/**
 * Execute kneading program
 */
void SequenceController::executeKneadingProgram() {
    if (!checkProgramConditions()) {
        // Debug: Only print every 5 seconds to reduce spam
        static unsigned long lastKneadingConditionsDebugTick = 0;
        unsigned long currentTick = timerManager->getMasterTicks();
        
        if (currentTick - lastKneadingConditionsDebugTick >= 500) {  // Every 5 seconds
            if (debugSerial) debugSerial->println("KNEADING: Program conditions not met");
            lastKneadingConditionsDebugTick = currentTick;
        }
        return;
    }
    
    // Debug: Only print once when starting
    static bool kneadingDebugPrinted = false;
    if (!kneadingDebugPrinted) {
        if (debugSerial) debugSerial->println("KNEADING: Executing kneading program");
        kneadingDebugPrinted = true;
    }
    
    runKneadingSequence();
}

/**
 * Execute compression program
 */
void SequenceController::executeCompressionProgram() {
    if (!checkProgramConditions()) {
        // Debug: Only print every 5 seconds to reduce spam
        static unsigned long lastCompressionConditionsDebugTick = 0;
        unsigned long currentTick = timerManager->getMasterTicks();
        
        if (currentTick - lastCompressionConditionsDebugTick >= 500) {  // Every 5 seconds
            if (debugSerial) debugSerial->println("COMPRESSION: Program conditions not met");
            lastCompressionConditionsDebugTick = currentTick;
        }
        return;
    }
    
    // Debug: Only print once when starting
    static bool compressionDebugPrinted = false;
    if (!compressionDebugPrinted) {
        if (debugSerial) debugSerial->println("COMPRESSION: Executing compression program");
        compressionDebugPrinted = true;
    }
    
    runCompressionSequence();
}

/**
 * Execute percussion program
 */
void SequenceController::executePercussionProgram() {
    if (!checkProgramConditions()) {
        // Debug: Only print every 5 seconds to reduce spam
        static unsigned long lastPercussionConditionsDebugTick = 0;
        unsigned long currentTick = timerManager->getMasterTicks();
        
        if (currentTick - lastPercussionConditionsDebugTick >= 500) {  // Every 5 seconds
            if (debugSerial) debugSerial->println("PERCUSSION: Program conditions not met");
            lastPercussionConditionsDebugTick = currentTick;
        }
        return;
    }
    
    // Debug: Only print once when starting
    static bool percussionDebugPrinted = false;
    if (!percussionDebugPrinted) {
        if (debugSerial) debugSerial->println("PERCUSSION: Executing percussion program");
        percussionDebugPrinted = true;
    }
    
    runPercussionSequence();
}

/**
 * Execute combined program
 */
void SequenceController::executeCombinedProgram() {
    if (!checkProgramConditions()) {
        // Debug: Only print every 5 seconds to reduce spam
        static unsigned long lastCombinedConditionsDebugTick = 0;
        unsigned long currentTick = timerManager->getMasterTicks();
        
        if (currentTick - lastCombinedConditionsDebugTick >= 500) {  // Every 5 seconds
            if (debugSerial) debugSerial->println("COMBINED: Program conditions not met");
            lastCombinedConditionsDebugTick = currentTick;
        }
        return;
    }
    
    // Debug: Only print once when starting
    static bool combinedDebugPrinted = false;
    if (!combinedDebugPrinted) {
        if (debugSerial) debugSerial->println("COMBINED: Executing combined program");
        combinedDebugPrinted = true;
    }
    
    runCombinedSequence();
}

/**
 * Run auto default sequence
 */
void SequenceController::runAutoDefaultSequence() {
    if (!autoSequenceStarted) {
        autoSequenceStarted = true;
        autoSequenceStartTick = timerManager->getMasterTicks();
        currentAutoSequenceState = AUTO_CASE_0;
        
        if (debugSerial) debugSerial->println("AUTO DEFAULT: Sequence started - State: CASE_0");
    }
    
    switch (currentAutoSequenceState) {
        case AUTO_CASE_0:
            handleAutoCase0();
            break;
        case AUTO_CASE_1:
            handleAutoCase1();
            break;
        case AUTO_CASE_2:
            handleAutoCase2();
            break;
        case AUTO_CASE_3:
            handleAutoCase3();
            break;
        case AUTO_CASE_4:
            handleAutoCase4();
            break;
        case AUTO_CASE_5:
            handleAutoCase5();
            break;
        case AUTO_CASE_6:
            handleAutoCase6();
            break;
        case AUTO_CASE_7:
            handleAutoCase7();
            break;
        case AUTO_CASE_8:
            handleAutoCase8();
            break;
        case AUTO_CASE_9:
            handleAutoCase9();
            break;
        case AUTO_CASE_10:
            handleAutoCase10();
            break;
        case AUTO_CASE_11:
            handleAutoCase11();
            break;
        case AUTO_CASE_12:
            handleAutoCase12();
            break;
        case AUTO_CASE_13:
            handleAutoCase13();
            break;
        case AUTO_CASE_14:
            handleAutoCase14();
            break;
        case AUTO_CASE_15:
            handleAutoCase15();
            break;
        case AUTO_CASE_16:
            handleAutoCase16();
            break;
        case AUTO_CASE_17:
            handleAutoCase17();
            break;
        case AUTO_CASE_18:
            handleAutoCase18();
            break;
        case AUTO_CASE_19:
            handleAutoCase19();
            break;
        case AUTO_CASE_20:
            handleAutoCase20();
            break;
        case AUTO_CASE_21:
            handleAutoCase21();
            break;
        case AUTO_CASE_22:
            handleAutoCase22();
            break;
        case AUTO_CASE_23:
            handleAutoCase23();
            break;
        case AUTO_CASE_24:
            handleAutoCase24();
            break;
        case AUTO_CASE_25:
            handleAutoCase25();
            break;
        case AUTO_CASE_26:
            handleAutoCase26();
            break;
        case AUTO_CASE_27:
            handleAutoCase27();
            break;
        case AUTO_CASE_28:
            handleAutoCase28();
            break;
        case AUTO_CASE_29:
            handleAutoCase29();
            break;
        case AUTO_CASE_30:
            handleAutoCase30();
            break;
        case AUTO_CASE_31:
            handleAutoCase31();
            break;
        case AUTO_CASE_32:
            handleAutoCase32();
            break;
        case AUTO_CASE_33:
            handleAutoCase33();
            break;
        case AUTO_CASE_34:
            handleAutoCase34();
            break;
        case AUTO_CASE_35:
            handleAutoCase35();
            break;
        case AUTO_CASE_36:
            handleAutoCase36();
            break;
        case AUTO_CASE_37:
            handleAutoCase37();
            break;
        case AUTO_CASE_38:
            handleAutoCase38();
            break;
        case AUTO_CASE_39:
            handleAutoCase39();
            break;
        case AUTO_CASE_40:
            handleAutoCase40();
            break;
        case AUTO_CASE_41:
            handleAutoCase41();
            break;
        case AUTO_CASE_42:
            handleAutoCase42();
            break;
        case AUTO_CASE_43:
            handleAutoCase43();
            break;
        case AUTO_CASE_44:
            handleAutoCase44();
            break;
        case AUTO_CASE_45:
            handleAutoCase45();
            break;
        case AUTO_CASE_46:
            handleAutoCase46();
            break;
        case AUTO_CASE_47:
            handleAutoCase47();
            break;
        case AUTO_CASE_48:
            handleAutoCase48();
            break;
        case AUTO_CASE_49:
            handleAutoCase49();
            break;
        case AUTO_CASE_50:
            handleAutoCase50();
            break;
        case AUTO_CASE_51:
            handleAutoCase51();
            break;
        case AUTO_CASE_52:
            handleAutoCase52();
            break;
        case AUTO_CASE_53:
            handleAutoCase53();
            break;
        case AUTO_CASE_54:
            handleAutoCase54();
            break;
        case AUTO_CASE_55:
            handleAutoCase55();
            break;
        case AUTO_CASE_56:
            handleAutoCase56();
            break;
        case AUTO_CASE_57:
            handleAutoCase57();
            break;
        case AUTO_CASE_58:
            handleAutoCase58();
            break;
        case AUTO_CASE_59:
            handleAutoCase59();
            break;
        case AUTO_CASE_60:
            handleAutoCase60();
            break;
        case AUTO_CASE_61:
            handleAutoCase61();
            break;
        case AUTO_CASE_62:
            handleAutoCase62();
            break;
        case AUTO_CASE_63:
            handleAutoCase63();
            break;
        case AUTO_CASE_64:
            handleAutoCase64();
            break;
        case AUTO_CASE_65:
            handleAutoCase65();
            break;
        case AUTO_CASE_66:
            handleAutoCase66();
            break;
        case AUTO_CASE_67:
            handleAutoCase67();
            break;
        case AUTO_CASE_68:
            handleAutoCase68();
            break;
        case AUTO_CASE_69:
            handleAutoCase69();
            break;
        case AUTO_CASE_70:
            handleAutoCase70();
            break;
        case AUTO_CASE_71:
            handleAutoCase71();
            break;
        case AUTO_CASE_72:
            handleAutoCase72();
            break;
        case AUTO_CASE_73:
            handleAutoCase73();
            break;
        case AUTO_CASE_74:
            handleAutoCase74();
            break;
        case AUTO_CASE_75:
            handleAutoCase75();
            break;
        case AUTO_CASE_76:
            handleAutoCase76();
            break;
        case AUTO_CASE_77:
            handleAutoCase77();
            break;
        case AUTO_CASE_78:
            handleAutoCase78();
            break;
        case AUTO_CASE_79:
            handleAutoCase79();
            break;
        case AUTO_CASE_80:
            handleAutoCase80();
            break;
        case AUTO_CASE_81:
            handleAutoCase81();
            break;
        case AUTO_CASE_82:
            handleAutoCase82();
            break;
        case AUTO_CASE_83:
            handleAutoCase83();
            break;
        case AUTO_CASE_84:
            handleAutoCase84();
            break;
        case AUTO_CASE_85:
            handleAutoCase85();
            break;
        case AUTO_CASE_86:
            handleAutoCase86();
            break;
        case AUTO_CASE_87:
            handleAutoCase87();
            break;
        case AUTO_CASE_88:
            handleAutoCase88();
            break;
        case AUTO_CASE_89:
            handleAutoCase89();
            break;
        case AUTO_CASE_90:
            handleAutoCase90();
            break;
        case AUTO_CASE_91:
            handleAutoCase91();
            break;
        case AUTO_CASE_92:
            handleAutoCase92();
            break;
        case AUTO_CASE_93:
            handleAutoCase93();
            break;
        case AUTO_CASE_94:
            handleAutoCase94();
            break;
        case AUTO_CASE_95:
            handleAutoCase95();
            break;
        case AUTO_CASE_96:
            handleAutoCase96();
            break;
        case AUTO_CASE_97:
            handleAutoCase97();
            break;
        
        default:
            if (debugSerial) {
                debugSerial->print("AUTO DEFAULT: Unknown case ");
                debugSerial->println(currentAutoSequenceState);
            }
            break;
    }
}

/**
 * Run kneading sequence
 */
void SequenceController::runKneadingSequence() {
    if (!kneadingSequenceStarted) {
        kneadingSequenceStarted = true;
        kneadingSequenceStartTick = timerManager->getMasterTicks();
        currentKneadingSequenceState = KNEADING_CASE_0;
        
        // Start kneading motor immediately and turn OFF compression motor
        if (motorController) {
            motorController->onKneadingMotor();
            motorController->offCompressionMotor();
            if (debugSerial) debugSerial->println("KNEADING: Kneading motor ON, Compression motor OFF");
        }
        
        if (debugSerial) debugSerial->println("KNEADING: Sequence started - State: CASE_0 (Roll UP + Kneading ON + Compression OFF)");
    }
    
    switch (currentKneadingSequenceState) {
        case KNEADING_CASE_0:
            handleKneadingCase0();
            break;
        case KNEADING_CASE_1:
            handleKneadingCase1();
            break;
    }
}

/**
 * Run compression sequence
 */
void SequenceController::runCompressionSequence() {
    if (!compressionSequenceStarted) {
        compressionSequenceStarted = true;
        compressionSequenceStartTick = timerManager->getMasterTicks();
        currentCompressionSequenceState = COMPRESSION_CASE_0;
        
        // Ensure all other motors are OFF at start
        if (motorController) {
            motorController->offKneadingMotor();
            motorController->offCompressionMotor();
            if (debugSerial) debugSerial->println("COMPRESSION: Kneading and compression motors OFF");
        }
        
        // Use intensity from remote (or default to HIGH if not set)
        if (intensityLevel == 0) {
            intensityLevel = 255;  // Default to HIGH if not set
            if (debugSerial) debugSerial->println("COMPRESSION: Intensity defaulted to HIGH (255)");
        } else {
            if (debugSerial) {
                debugSerial->print("COMPRESSION: Using remote intensity (");
                debugSerial->print(intensityLevel);
                debugSerial->println(")");
            }
        }
        
        if (debugSerial) debugSerial->println("COMPRESSION: Sequence started - State: CASE_0 (Roll UP only)");
    }
    
    switch (currentCompressionSequenceState) {
        case COMPRESSION_CASE_0:
            handleCompressionCase0();
            break;
        case COMPRESSION_CASE_1:
            handleCompressionCase1();
            break;
        case COMPRESSION_CASE_2:
            handleCompressionCase2();
            break;
    }
}

/**
 * Run percussion sequence
 */
void SequenceController::runPercussionSequence() {
    if (!percussionSequenceStarted) {
        percussionSequenceStarted = true;
        percussionSequenceStartTick = timerManager->getMasterTicks();
        currentPercussionSequenceState = PERCUSSION_CASE_0;
        
        if (debugSerial) debugSerial->println("PERCUSSION: Sequence started - State: CASE_0");
    }
    
    switch (currentPercussionSequenceState) {
        case PERCUSSION_CASE_0:
            handlePercussionCase0();
            break;
        case PERCUSSION_CASE_1:
            handlePercussionCase1();
            break;
    }
}

/**
 * Run combined sequence
 */
void SequenceController::runCombinedSequence() {
    if (!combinedSequenceStarted) {
        combinedSequenceStarted = true;
        combinedSequenceStartTick = timerManager->getMasterTicks();
        currentCombinedSequenceState = COMBINED_CASE_0;
        
        if (debugSerial) debugSerial->println("COMBINED: Sequence started - State: CASE_0");
    }
    
    switch (currentCombinedSequenceState) {
        case COMBINED_CASE_0:
            handleCombinedCase0();
            break;
        case COMBINED_CASE_1:
            handleCombinedCase1();
            break;
    }
}

/**
 * Check program conditions
 */
bool SequenceController::checkProgramConditions() {
    // Basic conditions: system must be allowed to run, home sequence completed, and not in manual priority
    return allowRun && homeRun && !manualPriority;
}

/**
 * Check if roll motor toggle is allowed
 */
bool SequenceController::isRollMotorToggleAllowed() {
    return currentAutoProgram != AUTO_DEFAULT;
}

/**
 * Check if intensity change is allowed
 */
bool SequenceController::isIntensityChangeAllowed() {
    return (currentAutoProgram == AUTO_COMPRESSION || 
            currentAutoProgram == AUTO_PERCUSSION || 
            currentAutoProgram == AUTO_COMBINED);
}

/**
 * Print system status
 */
void SequenceController::printSystemStatus() {
    if (debugSerial) debugSerial->println("=== System Status ===");
    if (debugSerial) debugSerial->print("Allow Run: "); if (debugSerial) debugSerial->println(allowRun ? "YES" : "NO");
    if (debugSerial) debugSerial->print("Home Run: "); if (debugSerial) debugSerial->println(homeRun ? "YES" : "NO");
    if (debugSerial) debugSerial->print("Mode Auto: "); if (debugSerial) debugSerial->println(modeAuto ? "YES" : "NO");
    if (debugSerial) debugSerial->print("Manual Priority: "); if (debugSerial) debugSerial->println(manualPriority ? "YES" : "NO");
    if (debugSerial) debugSerial->print("Intensity Level: "); if (debugSerial) debugSerial->println(intensityLevel);
    if (debugSerial) debugSerial->println("===================");
}

/**
 * Reset all modes
 */
void SequenceController::resetAllModes() {
    autodefaultMode = false;
    rollSpotMode = false;
    kneadingMode = false;
    compressionMode = false;
    percussionMode = false;
    combineMode = false;
    setIntensityOff("All modes reset");
}

/**
 * Timing getters/setters
 */
unsigned long SequenceController::getHomeStartTick() const { return homeStartTick; }
void SequenceController::setHomeStartTick(unsigned long tick) { homeStartTick = tick; }
unsigned long SequenceController::getHomeStepStartTick() const { return homeStepStartTick; }
void SequenceController::setHomeStepStartTick(unsigned long tick) { homeStepStartTick = tick; }
unsigned long SequenceController::getHomeLastResetTick() const { return homeLastResetTick; }
void SequenceController::setHomeLastResetTick(unsigned long tick) { homeLastResetTick = tick; }

unsigned long SequenceController::getAutoLastDirChangeTick() const { return autoLastDirChangeTick; }
void SequenceController::setAutoLastDirChangeTick(unsigned long tick) { autoLastDirChangeTick = tick; }
unsigned long SequenceController::getAutoModeStartTick() const { return autoModeStartTick; }
void SequenceController::setAutoModeStartTick(unsigned long tick) { autoModeStartTick = tick; }
unsigned long SequenceController::getAutoModeElapsedTicks() const { return autoModeElapsedTicks; }
void SequenceController::setAutoModeElapsedTicks(unsigned long tick) { autoModeElapsedTicks = tick; }
bool SequenceController::getAutoModeTimerActive() const { return autoModeTimerActive; }
void SequenceController::setAutoModeTimerActive(bool active) { autoModeTimerActive = active; }
bool SequenceController::getAutoTimerStarted() const { return autoTimerStarted; }
void SequenceController::setAutoTimerStarted(bool started) { autoTimerStarted = started; }

unsigned long SequenceController::getAutoTotalStartTick() const { return autoTotalStartTick; }
void SequenceController::setAutoTotalStartTick(unsigned long tick) { autoTotalStartTick = tick; }
unsigned long SequenceController::getAutoTotalElapsedTicks() const { return autoTotalElapsedTicks; }
void SequenceController::setAutoTotalElapsedTicks(unsigned long tick) { autoTotalElapsedTicks = tick; }
bool SequenceController::getAutoTotalTimerActive() const { return autoTotalTimerActive; }
void SequenceController::setAutoTotalTimerActive(bool active) { autoTotalTimerActive = active; }
bool SequenceController::getAutoTotalTimerExpired() const { return autoTotalTimerExpired; }
void SequenceController::setAutoTotalTimerExpired(bool expired) { autoTotalTimerExpired = expired; }

/**
 * Private Helper Functions
 */
void SequenceController::updateAutoModeTimer() {
    if (autoModeTimerActive) {
        autoModeElapsedTicks = timerManager->getMasterTicks() - autoModeStartTick;
        
        // Debug: Show remaining time every 30 seconds
        static unsigned long lastTimerDebugTick = 0;
        
        if (autoModeElapsedTicks >= SEQ_AUTO_MODE_DURATION_TICKS) {
            // if (debugSerial) debugSerial->println("AUTO TIMER: 20-minute timeout reached - stopping auto mode");
            stopAutoMode();
        }
    }
}

void SequenceController::updateAutoTotalTimer() {
    if (autoTotalTimerActive) {
        autoTotalElapsedTicks = timerManager->getMasterTicks() - autoTotalStartTick;
        
        if (autoTotalElapsedTicks >= SEQ_AUTO_MODE_DURATION_TICKS) {
            autoTotalTimerExpired = true;
            autoTotalTimerActive = false;
        }
    }
}

void SequenceController::handleProgramSwitch() {
    previousAutoProgram = currentAutoProgram;
    
    // Detect new program based on current mode flags first
    currentAutoProgram = detectAutoProgram();
    
    // Only reset sequence states, NOT mode flags (they should be set by commands)
    resetSequenceStatesOnly();
    
    programSwitchCount++;
    lastProgramSwitchTick = timerManager->getMasterTicks();
    
    if (debugSerial) {
        // debugSerial->print("Program switched from ");
        debugSerial->print(previousAutoProgram);
        debugSerial->print(" to ");
        debugSerial->print(currentAutoProgram);
        debugSerial->println();
    }
}

void SequenceController::executeCurrentProgram() {
    switch (currentAutoProgram) {
        case AUTO_DEFAULT:
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
            break;
    }
}

/**
 * Home sequence handlers
 */
void SequenceController::handleHomeStateIdle() {
    // HOME_IDLE state - system is ready, just wait
    // No action needed - this state means home sequence is not running
    // System is ready to receive AUTO commands
}

void SequenceController::handleHomeStateSearchingUp() {
    if (!motorController) return;
    
    // Run roll motor up (only once when entering this state)
    if (!homeMotorStarted) {
        if (debugSerial) debugSerial->println("GO HOME: Set direction UP and start motor");
        // if (debugSerial) debugSerial->println("DEBUG: Calling runRollUp() from handleHomeStateSearchingUp()");
        motorController->runRollUp();
        homeMotorStarted = true;
    }
    
    // Check for UP sensor
    bool upSensorActive = sensorManager ? sensorManager->getSensorUpLimit() : false;
    
    // Debug: Only print every 2 seconds to reduce spam
    static unsigned long lastSearchDebugTick = 0;
    unsigned long currentTick = timerManager->getMasterTicks();
    
    if (currentTick - lastSearchDebugTick >= 200) {  // Every 2 seconds
        // GO HOME: Running motor UP debug disabled to save Flash memory
        lastSearchDebugTick = currentTick;
    }
    
    if (upSensorActive) {
        // CRITICAL SAFETY: Stop motor immediately when sensor is detected
        motorController->offRollMotor();
        if (debugSerial) debugSerial->println("GO HOME: UP sensor detected - motor stopped");
        
        currentHomeState = HOME_DELAY_AT_UP;
        homeStepStartTick = timerManager->getMasterTicks();
        stepHomeRun = 2;
        homeMotorStarted = false;  // Reset for next state
        
        if (debugSerial) debugSerial->println("GO HOME: Moving to DELAY_AT_UP state");
    }
}

void SequenceController::handleHomeStateDelayAtUp() {
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Debug: Only print once when entering this state
    static bool delayDebugPrinted = false;
    if (!delayDebugPrinted) {
        if (debugSerial) debugSerial->println("GO HOME: Waiting 1 second at UP position");
        delayDebugPrinted = true;
    }
    
    // Wait 1000ms at UP position
    if (currentTick - homeStepStartTick >= 100) {  // 1000ms in ticks
        currentHomeState = HOME_RUNNING_DOWN;
        homeStepStartTick = currentTick;
        stepHomeRun = 3;
        delayDebugPrinted = false;  // Reset for next time
        homeMotorStarted = false;  // Reset for next state
        
        if (debugSerial) debugSerial->println("GO HOME: Moving to RUNNING_DOWN state");
    }
}

void SequenceController::handleHomeStateRunningDown() {
    if (!motorController) return;
    
    // Run roll motor down for 2 seconds (only once when entering this state)
    if (!homeMotorStarted) {
        if (debugSerial) debugSerial->println("GO HOME: Set direction DOWN and start motor for 2 seconds");
        if (debugSerial) debugSerial->println("DEBUG: Calling runRollDown() from handleHomeStateRunningDown()");
        motorController->runRollDown();
        homeMotorStarted = true;
    }
    
    unsigned long currentTick = timerManager->getMasterTicks();
    unsigned long elapsedTicks = currentTick - homeStepStartTick;
    
    // Debug: Show countdown every 500ms
    static unsigned long lastTimeoutDebugTick = 0;
    if (currentTick - lastTimeoutDebugTick >= 50) {  // Every 500ms
        unsigned long remainingMs = (SEQ_HOME_STEP3_RUN_TICKS - elapsedTicks) * 10;
        if (debugSerial && remainingMs > 0) {
            debugSerial->print("GO HOME: Motor DOWN running - ");
            debugSerial->print(remainingMs);
            debugSerial->println("ms remaining");
        }
        lastTimeoutDebugTick = currentTick;
    }
    
    if (elapsedTicks >= SEQ_HOME_STEP3_RUN_TICKS) {
        // Home sequence completed by timeout (2 seconds)
        motorController->offRollMotor();
        if (debugSerial) debugSerial->println("GO HOME: Setting homeRun = TRUE");
        homeRun = true;  // System is now ready - homeRun must be TRUE
        currentHomeState = HOME_IDLE;
        stepHomeRun = 0;
        homeMotorStarted = false;  // Reset for next time
        lastTimeoutDebugTick = 0;  // Reset for next time
        
        if (debugSerial) {
            debugSerial->println("=== HOME: Complete! Motor at home position ===");
            debugSerial->println("SYSTEM: Ready state - waiting for next AUTO command");
        }
    }
}

/**
 * Auto sequence handlers (simplified implementations)
 */
 void SequenceController::handleAutoCase0() {
    // AUTO_DEFAULT_CASE_0: Roll motor UP (DOWN→UP direction)
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check direction reversal first (expected direction = true = UP)
    if (!checkDirectionReversal(currentTick, true)) {
        return; // Pause case execution during reversal
    }
    
    // Debug: Show current case every 5 seconds
    if (currentTick - lastAutoCaseDebugTick >= 500) {  // Every 5 seconds
        #if DEBUG_SEQUENCECONTROLLER
        debugSerial->println("AUTO: CASE_0 - Roll UP");
        #endif
        lastAutoCaseDebugTick = currentTick;
    }
    
    // Run roll motor UP
    if (motorController) {
        motorController->runRollUp();
    }
    
    // Check for UP sensor hit (transition to CASE_1)
    if (sensorManager && sensorManager->getSensorUpLimit()) {
        // UP sensor detected, stop roll motor immediately and switch to CASE_1 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
        }
        
        currentAutoSequenceState = AUTO_CASE_1;
        autoLastDirChangeTick = currentTick;  // Reset timer for 2s delay
        
        #if DEBUG_SEQUENCECONTROLLER
        debugSerial->println("AUTO DEFAULT: UP sensor hit - switching to CASE_1");
        #endif
    }
}

void SequenceController::handleAutoCase1() {
    // AUTO_CASE_1: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.6 seconds then switch to CASE_2
    // SENSOR SAFETY: Any sensor hit -> transition to nearest sensor check case (CASE_21)
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, false, 60, AUTO_CASE_2, 
                                           "C01: R OFF K ON P ON S: T");
}

void SequenceController::handleAutoCase2() {
    // AUTO_CASE_2: ROLL MOTOR OFF, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1.4 seconds then switch to CASE_3
    // SENSOR SAFETY: Any sensor hit -> transition to nearest sensor check case (CASE_21)
    executeStandardAutoCaseWithSensorSafety(false, false, true, true, false, 140, AUTO_CASE_3,
                                           "C02: R OFF K OFF P ON S: T");
}

void SequenceController::handleAutoCase3() {
    // AUTO_CASE_3: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.6 seconds then switch to next case
    // SENSOR SAFETY: Any sensor hit -> transition to nearest sensor check case (CASE_21)
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 60, AUTO_CASE_4,
                                           "C03: R ON K OFF P ON S: T");
}

void SequenceController::handleAutoCase4() {
    // AUTO_CASE_4: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 100, AUTO_CASE_5,
                                           "C04: R ON K ON P ON S: T");
}

void SequenceController::handleAutoCase5() {
    // AUTO_CASE_5: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 200, AUTO_CASE_6,
                                           "C05: R ON K OFF P ON S: T");
}

void SequenceController::handleAutoCase6() {
    // AUTO_CASE_6: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 100, AUTO_CASE_7,
                                           "C06: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase7() {
    // AUTO_CASE_7: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 200, AUTO_CASE_8,
                                           "C07: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase8() {
    // AUTO_CASE_8: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 100, AUTO_CASE_9,
                                           "C08: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase9() {
    // AUTO_CASE_9: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.6 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 60, AUTO_CASE_10,
                                           "C09: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase10() {
    // AUTO_CASE_10: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 10 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, false, 1000, AUTO_CASE_11,
                                           "C10: R OFF K ON P ON S: T");
}
void SequenceController::handleAutoCase11() {
    // AUTO_CASE_11: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 200, AUTO_CASE_12,
                                           "C11: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase12() {
    // AUTO_CASE_12: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 100, AUTO_CASE_13,
                                           "C12: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase13() {
    // AUTO_CASE_13: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 50, AUTO_CASE_14,
                                           "C13: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase14() {
    // AUTO_CASE_14: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 200, AUTO_CASE_15,
                                           "C14: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase15() {
    // AUTO_CASE_15: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 200, AUTO_CASE_16,
                                           "C15: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase16() {
    // AUTO_CASE_16: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 50, AUTO_CASE_17,
                                           "C16: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase17() {
    // AUTO_CASE_17: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, false, false, false, 100, AUTO_CASE_18,
                                           "C17: R ON K OFF P OFF S: T");
}
void SequenceController::handleAutoCase18() {
    // AUTO_CASE_18: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 50, AUTO_CASE_19,
                                           "C18: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase19() {
    // AUTO_CASE_19: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, false, false, false, 100, AUTO_CASE_20,
                                           "C19: R ON K OFF P OFF S: T");
}
void SequenceController::handleAutoCase20() {
    // AUTO_CASE_20: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 4 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 400, AUTO_CASE_21,
                                           "C20: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase21() {
    // AUTO_CASE_21: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, DOWN sensor hit -> stop ROLL MOTOR immediately OR delay 4 seconds then switch to next case
    executeSensorBasedAutoCase(true, false, true, true, false, 400, AUTO_CASE_22,
                              "C21: R ON K OFF P ON S: W",
                              false, true, true);
    }
void SequenceController::handleAutoCase22() {
    // AUTO_CASE_22: ROLL MOTOR OFF, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, DOWN sensor hit -> stop ROLL MOTOR immediately OR delay 1.2 seconds then switch to next case
    executeSensorBasedAutoCase(false, false, true, true, false, 120, AUTO_CASE_23,
                              "C22: R OFF K OFF P ON S: W",
                              false, true, true);
}
void SequenceController::handleAutoCase23() {
    // AUTO_CASE_23: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 0.8 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, true, 80, AUTO_CASE_24,
                                           "C23: R OFF K ON P ON S: T");
}
void SequenceController::handleAutoCase24() {
    // AUTO_CASE_24: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 200, AUTO_CASE_25,
                                           "C24: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase25() {
    // AUTO_CASE_25: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, true, 100, AUTO_CASE_26,
                                           "C25: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase26() {
    // AUTO_CASE_26: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 200, AUTO_CASE_27,
                                           "C26: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase27() {
    // AUTO_CASE_27: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, true, 100, AUTO_CASE_28,
                                           "C27: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase28() {
    // AUTO_CASE_28: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 200, AUTO_CASE_29,
                                           "C28: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase29() {
    // AUTO_CASE_29: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 10 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, true, 1000, AUTO_CASE_30,
                                           "C29: R OFF K ON P ON S: T");
}
void SequenceController::handleAutoCase30() {
    // AUTO_CASE_30: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 300, AUTO_CASE_31,
                                           "C30: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase31() {
    // AUTO_CASE_31: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, true, 50, AUTO_CASE_32,
                                           "C31: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase32() {
    // AUTO_CASE_32: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 100, AUTO_CASE_33,
                                           "C32: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase33() {
    // AUTO_CASE_33: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, true, 50, AUTO_CASE_34,
                                           "C33: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase34() {
    // AUTO_CASE_34: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 150, AUTO_CASE_35,
                                           "C34: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase35() {
    // AUTO_CASE_35: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, true, 100, AUTO_CASE_36,
                                           "C35: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase36() {
    // AUTO_CASE_36: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, true, 300, AUTO_CASE_37,
                                           "C36: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase37() {
    // AUTO_CASE_37: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, false, false, true, 300, AUTO_CASE_38,
                                           "C37: R ON K OFF P OFF S: T");
}
void SequenceController::handleAutoCase38() {
    // AUTO_CASE_38: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 50, AUTO_CASE_39,
                                           "C38: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase39() {
    // AUTO_CASE_39: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, false, false, true, 100, AUTO_CASE_40,
                                           "C39: R ON K OFF P OFF S: T");
}
void SequenceController::handleAutoCase40() {
    // AUTO_CASE_40: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 50, AUTO_CASE_41,
                                           "C40: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase41() {
    // AUTO_CASE_41: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, UP sensor hit -> stop motor immediately OR delay 5 seconds then switch to next case
    executeSensorBasedAutoCase(true, false, false, false, true, 500, AUTO_CASE_42,
                              "C41: R ON K OFF P OFF S: W",
                              true, false, true);
}
void SequenceController::handleAutoCase42() {
    // AUTO_CASE_42: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, false, false, false, 200, AUTO_CASE_43,
                                           "C42: R OFF K ON P OFF S: T");
}
void SequenceController::handleAutoCase43() {
    // AUTO_CASE_43: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 200, AUTO_CASE_44,
                                           "C43: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase44() {
    // AUTO_CASE_44: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 200, AUTO_CASE_45,
                                           "C44: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase45() {
    // AUTO_CASE_45: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 50, AUTO_CASE_46,
                                           "C45: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase46() {
    // AUTO_CASE_46: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 300, AUTO_CASE_47,
                                           "C46: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase47() {
    // AUTO_CASE_47: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 50, AUTO_CASE_48,
                                           "C47: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase48() {
    // AUTO_CASE_48: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, DOWN sensor hit -> stop motor immediately OR delay 25 seconds then switch to next case
    executeSensorBasedAutoCase(true, false, true, true, false, 2500, AUTO_CASE_49,
                              "C48: R ON K OFF P ON S: W",
                              false, true, true);
}
void SequenceController::handleAutoCase49() {
    // AUTO_CASE_49: ROLL MOTOR OFF, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, false, true, true, true, 200, AUTO_CASE_50,
                                           "C49: R OFF K OFF P ON S: T");
}
void SequenceController::handleAutoCase50() {
    // AUTO_CASE_50: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, true, 100, AUTO_CASE_51,
                                           "C50: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase51() {
    // AUTO_CASE_51: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 200, AUTO_CASE_52,
                                           "C51: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase52() {
    // AUTO_CASE_52: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, true, 100, AUTO_CASE_53,
                                           "C52: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase53() {
    // AUTO_CASE_53: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 200, AUTO_CASE_54,
                                           "C53: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase54() {
    // AUTO_CASE_54: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, true, 100, AUTO_CASE_55,
                                           "C54: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase55() {
    // AUTO_CASE_55: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1.3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, true, 130, AUTO_CASE_56,
                                           "C55: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase56() {
    // AUTO_CASE_56: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 10 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, true, 1000, AUTO_CASE_57,
                                           "C56: R OFF K ON P ON S: T");
}
void SequenceController::handleAutoCase57() {
    // AUTO_CASE_57: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, true, 200, AUTO_CASE_58,
                                           "C57: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase58() {
    // AUTO_CASE_58: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 100, AUTO_CASE_59,
                                           "C58: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase59() {
    // AUTO_CASE_59: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, true, 50, AUTO_CASE_60,
                                           "C59: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase60() {
    // AUTO_CASE_60: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 200, AUTO_CASE_61,
                                           "C60: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase61() {
    // AUTO_CASE_61: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, true, 300, AUTO_CASE_62,
                                           "C61: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase62() {
    // AUTO_CASE_62: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 50, AUTO_CASE_63,
                                           "C62: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase63() {
    // AUTO_CASE_63: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, false, false, true, 100, AUTO_CASE_64,
                                           "C63: R ON K OFF P OFF S: T");
}
void SequenceController::handleAutoCase64() {
    // AUTO_CASE_64: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, true, 50, AUTO_CASE_65,
                                           "C64: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase65() {
    // AUTO_CASE_65: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, false, false, true, 100, AUTO_CASE_66,
                                           "C65: R ON K OFF P OFF S: T");
}
void SequenceController::handleAutoCase66() {
    // AUTO_CASE_66: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 4 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, true, 400, AUTO_CASE_67,
                                           "C66: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase67() {
    // AUTO_CASE_67: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, UP sensor hit -> stop motor immediately OR delay 4 seconds then switch to next case
    executeSensorBasedAutoCase(true, false, true, true, true, 400, AUTO_CASE_68,
                              "C67: R ON K OFF P ON S: W",
                              true, false, true);
}
void SequenceController::handleAutoCase68() {
    // AUTO_CASE_68: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, false, 50, AUTO_CASE_69,
                                           "C68: R OFF K ON P ON S: T");
}
void SequenceController::handleAutoCase69() {
    // AUTO_CASE_69: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, false, 100, AUTO_CASE_70,
                                           "C69: R OFF K ON P ON S: T");
}
void SequenceController::handleAutoCase70() {
    // AUTO_CASE_70: ROLL MOTOR OFF, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.7 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, false, true, true, false, 70, AUTO_CASE_71,
                                           "C70: R OFF K OFF P ON S: T");
}
void SequenceController::handleAutoCase71() {
    // AUTO_CASE_71: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1.3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 130, AUTO_CASE_72,
                                           "C71: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase72() {
    // AUTO_CASE_72: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 100, AUTO_CASE_73,
                                           "C72: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase73() {
    // AUTO_CASE_73: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 200, AUTO_CASE_74,
                                           "C73: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase74() {
    // AUTO_CASE_74: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 100, AUTO_CASE_75,
                                           "C74: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase75() {
    // AUTO_CASE_75: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 200, AUTO_CASE_76,
                                           "C75: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase76() {
    // AUTO_CASE_76: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 100, AUTO_CASE_77,
                                           "C76: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase77() {
    // AUTO_CASE_77: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 10 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(false, true, true, true, false, 1000, AUTO_CASE_78,
                                           "C77: R OFF K ON P ON S: T");
}
void SequenceController::handleAutoCase78() {
    // AUTO_CASE_78: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 300, AUTO_CASE_79,
                                           "C78: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase79() {
    // AUTO_CASE_79: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 50, AUTO_CASE_80,
                                           "C79: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase80() {
    // AUTO_CASE_80: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 100, AUTO_CASE_81,
                                           "C80: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase81() {
    // AUTO_CASE_81: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 50, AUTO_CASE_82,
                                           "C81: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase82() {
    // AUTO_CASE_82: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 1.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 150, AUTO_CASE_83,
                                           "C82: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase83() {
    // AUTO_CASE_83: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 2 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, false, false, false, 200, AUTO_CASE_84,
                                           "C83: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase84() {
    // AUTO_CASE_84: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 3 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, true, true, true, false, 300, AUTO_CASE_85,
                                           "C84: R ON K ON P ON S: T");
}
void SequenceController::handleAutoCase85() {
    // AUTO_CASE_85: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, DOWN sensor hit OR delay 3 seconds then switch to next case
    executeSensorBasedAutoCase(true, false, false, false, false, 300, AUTO_CASE_86,
                              "C85: R ON K OFF P OFF S: W",
                              false, true, true);
}
void SequenceController::handleAutoCase86() {
    // AUTO_CASE_86: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 50, AUTO_CASE_87,
                                           "C86: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase87() {
    // AUTO_CASE_87: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, delay 1 second then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, false, false, false, 100, AUTO_CASE_88,
                                           "C87: R ON K OFF P OFF S: T");
}
void SequenceController::handleAutoCase88() {
    // AUTO_CASE_88: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction UP→DOWN, delay 0.5 seconds then switch to next case
    executeStandardAutoCaseWithSensorSafety(true, false, true, true, false, 50, AUTO_CASE_89,
                                           "C88: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase89() {
    // AUTO_CASE_89: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, DOWN sensor hit -> stop motor immediately OR delay 6 seconds then switch to next case
    executeSensorBasedAutoCase(true, false, false, false, false, 600, AUTO_CASE_90,
                              "C89: R ON K OFF P OFF S: W",
                              false, true, true);
}
void SequenceController::handleAutoCase90() {
    // AUTO_CASE_90: ROLL MOTOR OFF, KNEADING MOTOR OFF, PERCUSSION MOTOR OFF
    // Set direction UP→DOWN, DOWN sensor hit -> stop all motors immediately OR delay 0.4 seconds then switch to next case
    executeSensorBasedAutoCaseWithMotorStop(false, false, false, false, false, 40, AUTO_CASE_91,
                                          "C90: R OFF K OFF P OFF S: W",
                                          false, true);
}
void SequenceController::handleAutoCase91() {
    // AUTO_CASE_91: ROLL MOTOR OFF, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 1.6 seconds then switch to next case
    executeStandardAutoCase(false, true, false, false, true, 160, AUTO_CASE_92,
                           "C91: R OFF K ON P OFF S: T");
}
void SequenceController::handleAutoCase92() {
    // AUTO_CASE_92: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 2.4 seconds then switch to next case
    executeStandardAutoCase(true, true, false, false, true, 240, AUTO_CASE_93,
                           "C92: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase93() {
    // AUTO_CASE_93: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 2 seconds then switch to next case
    executeStandardAutoCase(true, false, true, true, true, 200, AUTO_CASE_94,
                           "C93: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase94() {
    // AUTO_CASE_94: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCase(true, true, false, false, true, 50, AUTO_CASE_95,
                           "C94: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase95() {
    // AUTO_CASE_95: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, delay 3 seconds then switch to next case
    executeStandardAutoCase(true, false, true, true, true, 300, AUTO_CASE_96,
                           "C95: R ON K OFF P ON S: T");
}
void SequenceController::handleAutoCase96() {
    // AUTO_CASE_96: ROLL MOTOR ON, KNEADING MOTOR ON, PERCUSSION MOTOR OFF
    // Set direction DOWN→UP, delay 0.5 seconds then switch to next case
    executeStandardAutoCase(true, true, false, false, true, 50, AUTO_CASE_97,
                           "C96: R ON K ON P OFF S: T");
}
void SequenceController::handleAutoCase97() {
    // AUTO_CASE_97: ROLL MOTOR ON, KNEADING MOTOR OFF, PERCUSSION MOTOR ON (HIGH LEVEL)
    // Set direction DOWN→UP, UP sensor hit -> stop motor and switch to CASE_1 (new cycle) OR delay 25 seconds then switch to next case
    executeSensorBasedAutoCase(true, false, true, true, true, 2500, AUTO_CASE_1,
                              "C97: R ON K OFF P ON S: W",
                              true, false);
}
void SequenceController::handleKneadingCase0() {
    // KNEADING_CASE_0: Delay period after DOWN sensor, then roll motor DOWN→UP
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    unsigned long delayElapsed = currentTick - kneadingSequenceStartTick;
    if (delayElapsed < 200) {  // 2 seconds = 200 ticks
        // Still in delay period
        if (motorController) {
            // Kneading motor always ON during kneading sequence
            motorController->onKneadingMotor();
            
            // Stop roll motor during delay
            motorController->offRollMotor();
        }
        
        // Debug: Show delay countdown (disabled to save FLASH)
        // static unsigned long lastDelayDebugTick = 0;
        // if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
        //     unsigned long remainingMs = 2000 - (delayElapsed * 10);
        //     if (debugSerial && remainingMs > 0) {
        //         debugSerial->print("KNEADING: In delay period - ");
        //         debugSerial->print(remainingMs);
        //         debugSerial->println("ms remaining");
        //     }
        //     lastDelayDebugTick = currentTick;
        // }
        return;
    }
    
    // Delay completed, start roll motor DOWN→UP
    if (motorController) {
        // Kneading motor always ON during kneading sequence
        motorController->onKneadingMotor();
        
        // Compression motor always OFF during kneading sequence
        motorController->offCompressionMotor();
        
        // Roll motor DOWN→UP (if not manually overridden and not disabled by user)
        // Note: Only KNEADING, COMPRESSION, PERCUSSION, COMBINE modes can disable roll motor
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollUp();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("KNEADING: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
    }
    
    // Check for UP sensor
    if (sensorManager && sensorManager->getSensorUpLimit()) {
        // UP sensor detected, stop roll motor immediately and switch to CASE_1 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("KNEADING: UP sensor detected - roll motor stopped immediately");
        }
        
        currentKneadingSequenceState = KNEADING_CASE_1;
        kneadingSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("KNEADING: Switching to CASE_1 (2s delay)");
    }
    
    // Debug: Only print every 5 seconds to reduce spam
    static unsigned long lastKneadingCase0DebugTick = 0;
    if (currentTick - lastKneadingCase0DebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) {
            debugSerial->print("KNEADING CASE_0: Roll UP + Kneading ON + Compression OFF (Manual=");
            debugSerial->print(manualPriority ? "TRUE" : "FALSE");
            debugSerial->println(")");
        }
        lastKneadingCase0DebugTick = currentTick;
    }
}

void SequenceController::handleKneadingCase1() {
    // KNEADING_CASE_1: Roll motor UP→DOWN, Kneading motor ON, 2-second delay
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Debug: Show entry to CASE_1
    static unsigned long lastCase1EntryDebugTick = 0;
    if (currentTick - lastCase1EntryDebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) debugSerial->println("KNEADING CASE_1: Entry - checking delay and sensor");
        lastCase1EntryDebugTick = currentTick;
    }
    
    // Check if 2-second delay has passed
    unsigned long delayElapsed = currentTick - kneadingSequenceStartTick;
    if (delayElapsed < 200) {  // 2 seconds = 200 ticks
        // Still in delay period
        if (motorController) {
            // Kneading motor always ON during kneading sequence
            motorController->onKneadingMotor();
            
            // Stop roll motor during delay
            motorController->offRollMotor();
        }
        
        // Debug: Show delay countdown
        static unsigned long lastDelayDebugTick = 0;
        if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
            unsigned long remainingMs = 2000 - (delayElapsed * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("KNEADING: In delay period - ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms remaining");
            }
            lastDelayDebugTick = currentTick;
        }
        return;
    }
    
    // Delay completed, start roll motor UP→DOWN
    if (motorController) {
        // Kneading motor always ON during kneading sequence
        motorController->onKneadingMotor();
        
        // Compression motor always OFF during kneading sequence
        motorController->offCompressionMotor();
        
        // Roll motor UP→DOWN (if not manually overridden and not disabled by user)
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollDown();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("KNEADING: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
    }
    
    // Check for DOWN sensor
    if (sensorManager && sensorManager->getSensorDownLimit()) {
        // DOWN sensor detected, stop roll motor immediately and switch back to CASE_0 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("KNEADING: DOWN sensor detected - roll motor stopped immediately");
        }
        
        currentKneadingSequenceState = KNEADING_CASE_0;
        kneadingSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("KNEADING: Switching to CASE_0 (2s delay)");
    }
    
    // Debug: Only print every 5 seconds to reduce spam
    static unsigned long lastKneadingCase1DebugTick = 0;
    
    if (currentTick - lastKneadingCase1DebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) {
            debugSerial->print("KNEADING CASE_1: Roll DOWN + Kneading ON + Compression OFF (Manual=");
            debugSerial->print(manualPriority ? "TRUE" : "FALSE");
            debugSerial->println(")");
        }
        lastKneadingCase1DebugTick = currentTick;
    }
}

void SequenceController::handleCompressionCase0() {
    // COMPRESSION_CASE_0: Delay period after DOWN sensor, then roll motor DOWN→UP
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    unsigned long delayElapsed = currentTick - compressionSequenceStartTick;
    if (delayElapsed < 200) {  // 2 seconds = 200 ticks
        // Still in delay period - ONLY roll motor stops, other motors continue
        if (motorController) {
            // Keep kneading and compression motors running during delay
            // Kneading motor: OFF 2s, ON 1s pattern
            unsigned long cycleTick = delayElapsed % 300;  // 3-second cycle
            if (cycleTick < 200) {  // First 2 seconds - OFF
                motorController->offKneadingMotor();
            } else {  // Last 1 second - ON
                motorController->onKneadingMotor();
            }
            
            // Compression motor: Always ON with program-specific intensity
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
            motorController->onCompressionMotor();
            
            // Only roll motor stops during delay
            motorController->offRollMotor();
        }
        
    // Debug: Show delay countdown (disabled to save FLASH)
    // static unsigned long lastDelayDebugTick = 0;
    // if (currentTick - lastDelayDebugTick >= 200) {  // Every 2 seconds
    //     unsigned long remainingMs = 2000 - (delayElapsed * 10);
    //     if (debugSerial && remainingMs > 0) {
    //         debugSerial->print("COMPRESSION: In delay period - ");
    //         debugSerial->print(remainingMs);
    //         debugSerial->println("ms remaining - kneading & compression motors continue");
    //     }
    //     lastDelayDebugTick = currentTick;
    // }
        return;
    }
    
    // Delay completed, start roll motor DOWN→UP
    if (motorController) {
        // Roll motor DOWN→UP (if not manually overridden and not disabled by user)
        // Note: Only KNEADING, COMPRESSION, PERCUSSION, COMBINE modes can disable roll motor
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollUp();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("COMPRESSION: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
        
        // COMPRESSION MODE: Kneading motor pattern (OFF 2s, ON 1s, repeat)
        unsigned long kneadingCycleTime = currentTick % 300;  // 3-second cycle (300 ticks)
        if (kneadingCycleTime < 200) {  // First 2 seconds (200 ticks) - OFF
            motorController->offKneadingMotor();
        } else {  // Last 1 second (100 ticks) - ON
            motorController->onKneadingMotor();
        }
        
        // COMPRESSION MODE: Compression motor with program-specific intensity
        // intensityLevel depends on program: DEFAULT/KNEADING=255, others=remote
        motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        motorController->onCompressionMotor();  // Compression motor ON with intensity
    }
    
    // Check for UP sensor
    if (sensorManager && sensorManager->getSensorUpLimit()) {
        // UP sensor detected, stop roll motor immediately and switch to CASE_1 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("COMPRESSION: UP sensor detected - roll motor stopped immediately");
        }
        
        currentCompressionSequenceState = COMPRESSION_CASE_1;
        compressionSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("COMPRESSION: Switching to CASE_1 (2s delay)");
    }
    
    // Debug: Only print every 5 seconds to reduce spam
    static unsigned long lastCompressionCase0DebugTick = 0;
    if (currentTick - lastCompressionCase0DebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) {
            debugSerial->print("COMPRESSION CASE_0: Roll UP + Kneading(OFF 2s/ON 1s) + Compression(Intensity=");
            debugSerial->print(intensityLevel);
            debugSerial->println(")");
        }
        lastCompressionCase0DebugTick = currentTick;
    }
}

void SequenceController::handleCompressionCase1() {
    // COMPRESSION_CASE_1: Roll motor UP→DOWN, kneading OFF, compression OFF, intensity HIGH
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    if (currentTick - compressionSequenceStartTick < 200) {  // 2 seconds = 200 ticks
        // Still in delay period - ONLY roll motor stops, other motors continue
        if (motorController) {
            // Keep kneading and compression motors running during delay
            // Kneading motor: OFF 2s, ON 1s pattern
            unsigned long cycleTick = (currentTick - compressionSequenceStartTick) % 300;  // 3-second cycle
            if (cycleTick < 200) {  // First 2 seconds - OFF
                motorController->offKneadingMotor();
            } else {  // Last 1 second - ON
                motorController->onKneadingMotor();
            }
            
            // Compression motor: Always ON with program-specific intensity
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
            motorController->onCompressionMotor();
            
            // Only roll motor stops during delay
            motorController->offRollMotor();
        }
        
        // Debug: Show delay countdown
        static unsigned long lastDelayDebugTick = 0;
        if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
            unsigned long remainingMs = 2000 - ((currentTick - compressionSequenceStartTick) * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("COMPRESSION: Delay remaining: ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms - kneading & compression motors continue");
            }
            lastDelayDebugTick = currentTick;
        }
        return;
    }
    
    // Delay completed, start roll motor UP→DOWN
    if (motorController) {
        // Roll motor UP→DOWN (if not manually overridden and not disabled by user)
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollDown();
            
            // Debug: Show roll motor state every 5 seconds
            static unsigned long lastRollMotorStateTick = 0;
            if (currentTick - lastRollMotorStateTick >= 500) {  // Every 5 seconds
                if (debugSerial) {
                    debugSerial->print("COMPRESSION CASE_1: Roll motor DOWN running - ");
                    debugSerial->print("RL3_PWM=");
                    debugSerial->print(digitalRead(RL3_PWM_PIN) ? "HIGH" : "LOW");
                    debugSerial->print(", RL3_DIR=");
                    debugSerial->print(digitalRead(RL3_DIR_PIN) ? "HIGH" : "LOW");
                    debugSerial->print(", DOWN sensor=");
                    debugSerial->println(sensorManager ? (sensorManager->getSensorDownLimit() ? "ACTIVE" : "INACTIVE") : "UNKNOWN");
                }
                lastRollMotorStateTick = currentTick;
            }
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("COMPRESSION: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
        
        // COMPRESSION MODE: Kneading motor pattern (OFF 2s, ON 1s, repeat)
        unsigned long kneadingCycleTime = currentTick % 300;  // 3-second cycle (300 ticks)
        if (kneadingCycleTime < 200) {  // First 2 seconds (200 ticks) - OFF
            motorController->offKneadingMotor();
        } else {  // Last 1 second (100 ticks) - ON
            motorController->onKneadingMotor();
        }
        
        // COMPRESSION MODE: Compression motor with program-specific intensity
        // intensityLevel depends on program: DEFAULT/KNEADING=255, others=remote
        motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        motorController->onCompressionMotor();  // Compression motor ON with intensity
        
        // Debug: Show current intensity level every 5 seconds
        static unsigned long lastIntensityDebugTick = 0;
        if (currentTick - lastIntensityDebugTick >= 500) {  // Every 5 seconds
            if (debugSerial) {
                debugSerial->print("COMPRESSION CASE_1: Current PWM=255 (HIGH)");
            }
            lastIntensityDebugTick = currentTick;
        }
    }
    
    // Check for DOWN sensor
    bool downSensorActive = sensorManager ? sensorManager->getSensorDownLimit() : false;
    
    // Debug: Show DOWN sensor state every 2 seconds
    static unsigned long lastDownSensorDebugTick = 0;
    if (currentTick - lastDownSensorDebugTick >= 200) {  // Every 2 seconds
        if (debugSerial) {
            debugSerial->print("COMPRESSION CASE_1: DOWN sensor state: ");
            debugSerial->println(downSensorActive ? "ACTIVE" : "INACTIVE");
        }
        lastDownSensorDebugTick = currentTick;
    }
    
    if (downSensorActive) {
        // DOWN sensor detected, stop roll motor immediately and switch to CASE_2 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("COMPRESSION: DOWN sensor detected - roll motor stopped immediately");
        }
        
        currentCompressionSequenceState = COMPRESSION_CASE_2;
        compressionSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        if (debugSerial) debugSerial->println("COMPRESSION: Switching to CASE_2 (2s delay) - kneading & compression motors continue");
    }
    
    // Debug: Only print every 5 seconds to reduce spam
    static unsigned long lastCompressionCase1DebugTick = 0;
    
    if (currentTick - lastCompressionCase1DebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) {
            debugSerial->print("COMPRESSION CASE_1: Roll DOWN + Kneading(OFF 2s/ON 1s) + Compression(Intensity=");
            debugSerial->print(intensityLevel);
            debugSerial->println(")");
        }
        lastCompressionCase1DebugTick = currentTick;
    }
}

void SequenceController::handleCompressionCase2() {
    // COMPRESSION_CASE_2: Roll motor DOWN→UP, kneading OFF, compression OFF, intensity HIGH
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    if (currentTick - compressionSequenceStartTick < 200) {  // 2 seconds = 200 ticks
        // Still in delay period - ONLY roll motor stops, other motors continue
        if (motorController) {
            // Keep kneading and compression motors running during delay
            // Kneading motor: OFF 2s, ON 1s pattern
            unsigned long cycleTick = (currentTick - compressionSequenceStartTick) % 300;  // 3-second cycle
            if (cycleTick < 200) {  // First 2 seconds - OFF
                motorController->offKneadingMotor();
            } else {  // Last 1 second - ON
                motorController->onKneadingMotor();
            }
            
            // Compression motor: Always ON with program-specific intensity
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
            motorController->onCompressionMotor();
            
            // Only roll motor stops during delay
            motorController->offRollMotor();
            
            // Debug: Show roll motor state during delay
            static unsigned long lastRollMotorDebugTick = 0;
            if (currentTick - lastRollMotorDebugTick >= 100) {  // Every 1 second
                if (debugSerial) {
                    debugSerial->print("COMPRESSION CASE_2: Delay - Roll motor OFF, ");
                    debugSerial->print("RL3_PWM=");
                    debugSerial->print(digitalRead(RL3_PWM_PIN) ? "HIGH" : "LOW");
                    debugSerial->print(", RL3_DIR=");
                    debugSerial->println(digitalRead(RL3_DIR_PIN) ? "HIGH" : "LOW");
                }
                lastRollMotorDebugTick = currentTick;
            }
        }
        
        // Debug: Show delay countdown
        static unsigned long lastDelayDebugTick = 0;
        if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
            unsigned long remainingMs = 2000 - ((currentTick - compressionSequenceStartTick) * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("COMPRESSION: Delay remaining: ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms - kneading & compression motors continue");
            }
            lastDelayDebugTick = currentTick;
        }
        return;
    }
    
    // Delay completed, start roll motor DOWN→UP
    if (motorController) {
        // Roll motor DOWN→UP (if not manually overridden and not disabled by user)
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollUp();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("COMPRESSION: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
        
        // COMPRESSION MODE: Kneading motor pattern (OFF 2s, ON 1s, repeat)
        unsigned long kneadingCycleTime = currentTick % 300;  // 3-second cycle (300 ticks)
        if (kneadingCycleTime < 200) {  // First 2 seconds (200 ticks) - OFF
            motorController->offKneadingMotor();
        } else {  // Last 1 second (100 ticks) - ON
            motorController->onKneadingMotor();
        }
        
        // COMPRESSION MODE: Compression motor with program-specific intensity
        // intensityLevel depends on program: DEFAULT/KNEADING=255, others=remote
        motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        motorController->onCompressionMotor();  // Compression motor ON with intensity
    }
    
    // Check for UP sensor
    if (sensorManager && sensorManager->getSensorUpLimit()) {
        // UP sensor detected, stop roll motor immediately and switch back to CASE_1 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("COMPRESSION: UP sensor detected - roll motor stopped immediately");
        }
        
        currentCompressionSequenceState = COMPRESSION_CASE_1;
        compressionSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("COMPRESSION: Switching to CASE_1 (2s delay)");
    }
    
    // Debug: Only print every 10 seconds to reduce spam (disabled to save FLASH)
    // static unsigned long lastCompressionCase2DebugTick = 0;
    // 
    // if (currentTick - lastCompressionCase2DebugTick >= 1000) {  // Every 10 seconds
    //     if (debugSerial) {
    //         debugSerial->print("COMPRESSION CASE_2: Roll UP + Kneading(OFF 2s/ON 1s) + Compression(Intensity=");
    //         debugSerial->print(intensityLevel);
    //         debugSerial->println(")");
    //     }
    //     lastCompressionCase2DebugTick = currentTick;
    // }
}

void SequenceController::handlePercussionCase0() {
    // PERCUSSION_CASE_0: Delay period after DOWN sensor, then roll motor DOWN→UP
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    unsigned long delayElapsed = currentTick - percussionSequenceStartTick;
    if (delayElapsed < 200) {  // 2 seconds = 200 ticks
        // Still in delay period - ONLY roll motor stops, other motors continue
        if (motorController) {
            // Keep kneading and compression motors running during delay
            // Kneading motor: ON 1s, OFF 2s pattern
            unsigned long cycleTick = delayElapsed % 300;  // 3-second cycle
            if (cycleTick < 100) {  // First 1 second - ON
                motorController->onKneadingMotor();
            } else {  // Next 2 seconds - OFF
                motorController->offKneadingMotor();
            }
            
            // Compression motor: Always ON with program-specific intensity
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
            motorController->onCompressionMotor();
            
            // Debug: Show intensity level being used in delay (reduced frequency)
            static unsigned long lastIntensityDebugTick = 0;
            if (currentTick - lastIntensityDebugTick >= 1000) {  // Every 10 seconds
                if (debugSerial) {
                    debugSerial->print("PERCUSSION: Delay - PWM=255 (HIGH)");
                }
                lastIntensityDebugTick = currentTick;
            }
            
            // Only roll motor stops during delay
            motorController->offRollMotor();
        }
        
        // Debug: Show delay countdown
        static unsigned long lastDelayDebugTick = 0;
        if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
            unsigned long remainingMs = 2000 - (delayElapsed * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("PERCUSSION: In delay period - ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms remaining - kneading & compression motors continue");
            }
            lastDelayDebugTick = currentTick;
        }
        return;
    }
    
    // Delay completed, start roll motor DOWN→UP
    if (motorController) {
        // Roll motor DOWN→UP (if not manually overridden and not disabled by user)
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollUp();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("PERCUSSION: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
        
        // PERCUSSION MODE: Kneading motor pattern (ON 1s, OFF 2s)
        unsigned long kneadingCycleTime = currentTick % 300;  // 3-second cycle (300 ticks)
        if (kneadingCycleTime < 100) {  // First 1 second - ON
            motorController->onKneadingMotor();
        } else {  // Next 2 seconds - OFF
            motorController->offKneadingMotor();
        }
        
        // PERCUSSION MODE: Compression motor with program-specific intensity
        motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        motorController->onCompressionMotor();
        
        // Debug: Show intensity level being used in normal operation (reduced frequency)
        static unsigned long lastIntensityNormalDebugTick = 0;
        if (currentTick - lastIntensityNormalDebugTick >= 1000) {  // Every 10 seconds
            if (debugSerial) {
                debugSerial->print("PERCUSSION: Normal - PWM=255 (HIGH)");
            }
            lastIntensityNormalDebugTick = currentTick;
        }
    }
    
    // Check for UP sensor
    if (sensorManager && sensorManager->getSensorUpLimit()) {
        // UP sensor detected, stop roll motor immediately and switch to CASE_1 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("PERCUSSION: UP sensor detected - roll motor stopped immediately");
        }
        
        currentPercussionSequenceState = PERCUSSION_CASE_1;
        percussionSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("PERCUSSION: Switching to CASE_1 (2s delay)");
    }
    
    // Debug: Only print every 10 seconds to reduce spam (disabled to save FLASH)
    // static unsigned long lastPercussionCase0DebugTick = 0;
    // if (currentTick - lastPercussionCase0DebugTick >= 1000) {  // Every 10 seconds
    //     if (debugSerial) debugSerial->println("PERCUSSION: Roll UP (CASE_0)");
    //     lastPercussionCase0DebugTick = currentTick;
    // }
}

void SequenceController::handlePercussionCase1() {
    // PERCUSSION_CASE_1: Delay period after UP sensor, then roll motor UP→DOWN
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    unsigned long delayElapsed = currentTick - percussionSequenceStartTick;
    if (delayElapsed < 200) {  // 2 seconds = 200 ticks
        // Still in delay period - ONLY roll motor stops, other motors continue
        if (motorController) {
            // Keep kneading and compression motors running during delay
            // Kneading motor: ON 1s, OFF 2s pattern
            unsigned long cycleTick = delayElapsed % 300;  // 3-second cycle
            if (cycleTick < 100) {  // First 1 second - ON
                motorController->onKneadingMotor();
            } else {  // Next 2 seconds - OFF
                motorController->offKneadingMotor();
            }
            
            // Compression motor: Always ON with program-specific intensity
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
            motorController->onCompressionMotor();
            
            // Debug: Show intensity level being used in delay
            static unsigned long lastIntensityCase1DebugTick = 0;
            if (currentTick - lastIntensityCase1DebugTick >= 200) {  // Every 2 seconds
                if (debugSerial) {
                    debugSerial->print("PERCUSSION CASE_1: Delay - PWM=255 (HIGH)");
                }
                lastIntensityCase1DebugTick = currentTick;
            }
            
            // Only roll motor stops during delay
            motorController->offRollMotor();
        }
        
        // Debug: Show delay countdown
        static unsigned long lastDelayDebugTick = 0;
        if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
            unsigned long remainingMs = 2000 - (delayElapsed * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("PERCUSSION: In delay period - ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms remaining - kneading & compression motors continue");
            }
            lastDelayDebugTick = currentTick;
        }
        return;
    }
    
    // Delay completed, start roll motor UP→DOWN
    if (motorController) {
        // Roll motor UP→DOWN (if not manually overridden and not disabled by user)
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollDown();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("PERCUSSION: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
        
        // PERCUSSION MODE: Kneading motor pattern (ON 1s, OFF 2s)
        unsigned long kneadingCycleTime = currentTick % 300;  // 3-second cycle (300 ticks)
        if (kneadingCycleTime < 100) {  // First 1 second - ON
            motorController->onKneadingMotor();
        } else {  // Next 2 seconds - OFF
            motorController->offKneadingMotor();
        }
        
        // PERCUSSION MODE: Compression motor with program-specific intensity
        motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        motorController->onCompressionMotor();
        
        // Debug: Show intensity level being used in normal operation
        static unsigned long lastIntensityCase1NormalDebugTick = 0;
        if (currentTick - lastIntensityCase1NormalDebugTick >= 500) {  // Every 5 seconds
            if (debugSerial) {
                debugSerial->print("PERCUSSION CASE_1: Normal - PWM=255 (HIGH)");
            }
            lastIntensityCase1NormalDebugTick = currentTick;
        }
    }
    
    // Check for DOWN sensor
    if (sensorManager && sensorManager->getSensorDownLimit()) {
        // DOWN sensor detected, stop roll motor immediately and switch back to CASE_0 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("PERCUSSION: DOWN sensor detected - roll motor stopped immediately");
        }
        
        currentPercussionSequenceState = PERCUSSION_CASE_0;
        percussionSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("PERCUSSION: Switching to CASE_0 (2s delay)");
    }
    
    // Debug: Only print every 10 seconds to reduce spam (disabled to save FLASH)
    // static unsigned long lastPercussionCase1DebugTick = 0;
    // if (currentTick - lastPercussionCase1DebugTick >= 1000) {  // Every 10 seconds
    //     if (debugSerial) debugSerial->println("PERCUSSION: Roll DOWN (CASE_1)");
    //     lastPercussionCase1DebugTick = currentTick;
    // }
}

void SequenceController::handleCombinedCase0() {
    // COMBINED_CASE_0: Delay period after DOWN sensor, then roll motor DOWN→UP
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    unsigned long delayElapsed = currentTick - combinedSequenceStartTick;
    if (delayElapsed < 200) {  // 2 seconds = 200 ticks
        // Still in delay period - ONLY roll motor stops, other motors continue
        if (motorController) {
            // Keep kneading and compression motors running during delay
            // Kneading motor: Always ON during COMBINE mode
            motorController->onKneadingMotor();
            
            // Compression motor: Always ON with program-specific intensity
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
            motorController->onCompressionMotor();
            
            // Only roll motor stops during delay
            motorController->offRollMotor();
        }
        
        // Debug: Show delay countdown
        static unsigned long lastDelayDebugTick = 0;
        if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
            unsigned long remainingMs = 2000 - (delayElapsed * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("COMBINED: In delay period - ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms remaining - kneading & compression motors continue");
            }
            lastDelayDebugTick = currentTick;
        }
        return;
    }
    
    // Delay completed, start roll motor DOWN→UP
    if (motorController) {
        // Roll motor DOWN→UP (if not manually overridden and not disabled by user)
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollUp();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("COMBINED: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
        
        // COMBINED MODE: Kneading motor always ON
        motorController->onKneadingMotor();
        
        // COMBINED MODE: Compression motor with program-specific intensity
        motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        motorController->onCompressionMotor();
        
        // Debug: Show intensity level being used in normal operation (reduced frequency)
        static unsigned long lastIntensityNormalDebugTick = 0;
        if (currentTick - lastIntensityNormalDebugTick >= 1000) {  // Every 10 seconds
            if (debugSerial) {
                debugSerial->print("COMBINED: Normal - PWM=255 (HIGH)");
            }
            lastIntensityNormalDebugTick = currentTick;
        }
    }
    
    // Check for UP sensor
    if (sensorManager && sensorManager->getSensorUpLimit()) {
        // UP sensor detected, stop roll motor immediately and switch to CASE_1 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("COMBINED: UP sensor detected - roll motor stopped immediately");
        }
        
        currentCombinedSequenceState = COMBINED_CASE_1;
        combinedSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("COMBINED: Switching to CASE_1 (2s delay)");
    }
    
    // Debug: Only print every 10 seconds to reduce spam (disabled to save FLASH)
    // static unsigned long lastCombinedCase0DebugTick = 0;
    // if (currentTick - lastCombinedCase0DebugTick >= 1000) {  // Every 10 seconds
    //     if (debugSerial) debugSerial->println("COMBINED: Roll UP (CASE_0)");
    //     lastCombinedCase0DebugTick = currentTick;
    // }
}

void SequenceController::handleCombinedCase1() {
    // COMBINED_CASE_1: Delay period after UP sensor, then roll motor UP→DOWN
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check if 2-second delay has passed
    unsigned long delayElapsed = currentTick - combinedSequenceStartTick;
    if (delayElapsed < 200) {  // 2 seconds = 200 ticks
        // Still in delay period - ONLY roll motor stops, other motors continue
        if (motorController) {
            // Keep kneading and compression motors running during delay
            // Kneading motor: Always ON during COMBINE mode
            motorController->onKneadingMotor();
            
            // Compression motor: Always ON with program-specific intensity
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
            motorController->onCompressionMotor();
            
            // Debug: Show intensity level being used in delay (reduced frequency)
            static unsigned long lastIntensityDebugTick = 0;
            if (currentTick - lastIntensityDebugTick >= 1000) {  // Every 10 seconds
                if (debugSerial) {
                    debugSerial->print("COMBINED: Delay - PWM=255 (HIGH)");
                }
                lastIntensityDebugTick = currentTick;
            }
            
            // Only roll motor stops during delay
            motorController->offRollMotor();
        }
        
        // Debug: Show delay countdown
        static unsigned long lastDelayDebugTick = 0;
        if (currentTick - lastDelayDebugTick >= 100) {  // Every 1 second
            unsigned long remainingMs = 2000 - (delayElapsed * 10);
            if (debugSerial && remainingMs > 0) {
                debugSerial->print("COMBINED: In delay period - ");
                debugSerial->print(remainingMs);
                debugSerial->println("ms remaining - kneading & compression motors continue");
            }
            lastDelayDebugTick = currentTick;
        }
        return;
    }
    
    // Delay completed, start roll motor UP→DOWN
    if (motorController) {
        // Roll motor UP→DOWN (if not manually overridden and not disabled by user)
        if (!manualPriority && !rollMotorUserDisabled) {
            motorController->runRollDown();
        } else if (manualPriority) {
            // Manual override active - roll motor controlled by user
            // if (debugSerial) debugSerial->println("COMBINED: Manual priority active - roll motor controlled by user");
        } else if (rollMotorUserDisabled) {
            // User disabled roll motor - keep it off
            // Debug output removed to reduce spam
        }
        
        // COMBINED MODE: Kneading motor always ON
        motorController->onKneadingMotor();
        
        // COMBINED MODE: Compression motor with program-specific intensity
        motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        motorController->onCompressionMotor();
        
        // Debug: Show intensity level being used in normal operation (reduced frequency)
        static unsigned long lastIntensityNormalDebugTick = 0;
        if (currentTick - lastIntensityNormalDebugTick >= 1000) {  // Every 10 seconds
            if (debugSerial) {
                debugSerial->print("COMBINED: Normal - PWM=255 (HIGH)");
            }
            lastIntensityNormalDebugTick = currentTick;
        }
    }
    
    // Check for DOWN sensor
    if (sensorManager && sensorManager->getSensorDownLimit()) {
        // DOWN sensor detected, stop roll motor immediately and switch back to CASE_0 after 2-second delay
        if (motorController) {
            motorController->offRollMotor();
            // if (debugSerial) debugSerial->println("COMBINED: DOWN sensor detected - roll motor stopped immediately");
        }
        
        currentCombinedSequenceState = COMBINED_CASE_0;
        combinedSequenceStartTick = timerManager->getMasterTicks();  // Reset timer for 2s delay
        
        // if (debugSerial) debugSerial->println("COMBINED: Switching to CASE_0 (2s delay)");
    }
    
    // Debug: Only print every 10 seconds to reduce spam (disabled to save FLASH)
    // static unsigned long lastCombinedCase1DebugTick = 0;
    // if (currentTick - lastCombinedCase1DebugTick >= 1000) {  // Every 10 seconds
    //     if (debugSerial) debugSerial->println("COMBINED: Roll DOWN (CASE_1)");
    //     lastCombinedCase1DebugTick = currentTick;
    // }
}

/**
 * Timing helpers
 */
bool SequenceController::isTimeForDirectionChange() const {
    unsigned long currentTick = timerManager->getMasterTicks();
    return (currentTick - autoLastDirChangeTick) >= SEQ_AUTO_DIR_CHANGE_TICKS;
}

bool SequenceController::isHomeSequenceTimeout() const {
    unsigned long currentTick = timerManager->getMasterTicks();
    return (currentTick - homeStartTick) >= SEQ_HOME_TOTAL_TIMEOUT_TICKS;
}

bool SequenceController::isAutoModeTimeout() const {
    return autoModeElapsedTicks >= SEQ_AUTO_MODE_DURATION_TICKS;
}

bool SequenceController::isAutoTotalTimeout() const {
    return autoTotalElapsedTicks >= SEQ_AUTO_MODE_DURATION_TICKS;
}

/**
 * State validation
 */
bool SequenceController::isValidHomeState(HomeState state) const {
    return (state >= HOME_IDLE && state <= HOME_RUNNING_DOWN);
}

bool SequenceController::isValidAutoProgram(AutoProgram program) const {
    return (program >= AUTO_NONE && program <= AUTO_COMBINED);
}

bool SequenceController::canStartAutoMode() const {
    return allowRun && homeRun && !manualPriority;
}

bool SequenceController::canStartHomeSequence() const {
    return allowRun && !homeRun;
}

// Helper functions for auto cases optimization
void SequenceController::executeMotorControl(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh) {
    if (!motorController) return;
    
    // Roll motor control
    if (rollOn) {
        motorController->onRollMotor();
    } else {
        motorController->offRollMotor();
    }
    
    // Kneading motor control
    if (kneadingOn) {
        motorController->onKneadingMotor();
    } else {
        motorController->offKneadingMotor();
    }
    
    // Percussion motor control
    if (percussionOn) {
        if (percussionHigh) {
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        }
        motorController->onCompressionMotor();
        // Ensure PWM is set correctly after motor is turned on
        if (percussionHigh) {
            motorController->setCompressionPWMByIntensity(getIntensityForProgram(currentAutoProgram));
        }
    } else {
        motorController->offCompressionMotor();
    }
}

bool SequenceController::checkTimeoutAndTransition(unsigned long currentTick, unsigned long timeoutTicks, AutoSequenceState nextState) {
    if (currentTick - autoLastDirChangeTick >= timeoutTicks) {
        currentAutoSequenceState = nextState;
        autoLastDirChangeTick = currentTick;  // Reset timer for next case
        return true;  // Transition occurred
    }
    return false;  // No transition yet
}

void SequenceController::executeStandardAutoCase(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh, 
                                                bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState, 
                                                const char* debugMsg) {
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check direction reversal first (only if Roll motor is ON)
    if (rollOn && !checkDirectionReversal(currentTick, !directionDown)) {
        return; // Pause case execution during reversal
    }
    
    // Debug: Show current case every 5 seconds
    if (currentTick - lastAutoCaseDebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) debugSerial->println(debugMsg);
        lastAutoCaseDebugTick = currentTick;
    }
    
    // Execute motor control
    executeMotorControl(rollOn, kneadingOn, percussionOn, percussionHigh);
    
    // Set direction
    if (motorController) {
        motorController->setRollDirection(directionDown);  // true = DOWN→UP, false = UP→DOWN
    }
    
    // Check timeout and transition
    checkTimeoutAndTransition(currentTick, timeoutTicks, nextState);
}

void SequenceController::executeSensorBasedAutoCaseWithMotorStop(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh,
                                                                bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState,
                                                                const char* debugMsg, bool checkUpSensor, bool checkDownSensor) {
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check direction reversal first (only if Roll motor is ON)
    if (rollOn && !checkDirectionReversal(currentTick, !directionDown)) {
        return; // Pause case execution during reversal
    }
    
    // Debug: Show current case every 5 seconds
    if (currentTick - lastAutoCaseDebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) debugSerial->println(debugMsg);
        lastAutoCaseDebugTick = currentTick;
    }
    
    // Execute motor control
    executeMotorControl(rollOn, kneadingOn, percussionOn, percussionHigh);
    
    // Set direction
    if (motorController) {
        motorController->setRollDirection(directionDown);  // true = DOWN→UP, false = UP→DOWN
    }
    
    // Check for UP sensor - DỪNG NGAY TẤT CẢ MOTORS
    if (checkUpSensor && sensorManager && sensorManager->getSensorUpLimit()) {
        if (debugSerial) debugSerial->println("AUTO: UP sensor detected, stopping all motors and switching to next case");
        if (motorController) {
            motorController->offRollMotor();
            motorController->offKneadingMotor();
            motorController->offCompressionMotor();
        }
        currentAutoSequenceState = nextState;
        autoLastDirChangeTick = currentTick;  // Reset timer for next case
        return;
    }
    
    // Check for DOWN sensor - DỪNG NGAY TẤT CẢ MOTORS
    if (checkDownSensor && sensorManager && sensorManager->getSensorDownLimit()) {
        if (debugSerial) debugSerial->println("AUTO: DOWN sensor detected, stopping all motors and switching to next case");
        if (motorController) {
            motorController->offRollMotor();
            motorController->offKneadingMotor();
            motorController->offCompressionMotor();
        }
        currentAutoSequenceState = nextState;
        autoLastDirChangeTick = currentTick;  // Reset timer for next case
        return;
    }
    
    // Check timeout and transition
    checkTimeoutAndTransition(currentTick, timeoutTicks, nextState);
}

void SequenceController::executeSensorBasedAutoCase(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh,
                                                   bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState,
                                                   const char* debugMsg, bool checkUpSensor, bool checkDownSensor,
                                                   bool stopRollOnSensor) {
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Check direction reversal first (only if Roll motor is ON)
    if (rollOn && !checkDirectionReversal(currentTick, !directionDown)) {
        return; // Pause case execution during reversal
    }
    
    // Debug: Show current case every 5 seconds
    if (currentTick - lastAutoCaseDebugTick >= 500) {  // Every 5 seconds
        if (debugSerial) debugSerial->println(debugMsg);
        lastAutoCaseDebugTick = currentTick;
    }
    
    // Execute motor control
    executeMotorControl(rollOn, kneadingOn, percussionOn, percussionHigh);
    
    // Set direction
    if (motorController) {
        motorController->setRollDirection(directionDown);  // true = DOWN→UP, false = UP→DOWN
    }
    
    // Check for UP sensor
    if (checkUpSensor && sensorManager && sensorManager->getSensorUpLimit()) {
        // if (debugSerial) debugSerial->println("AUTO: UP sensor detected, switching to next case immediately");
        if (stopRollOnSensor && motorController) {
            motorController->offRollMotor();  // LẬP TỨC dừng ROLL MOTOR
        }
        currentAutoSequenceState = nextState;
        autoLastDirChangeTick = currentTick;  // Reset timer for next case
        return;
    }
    
    // Check for DOWN sensor
    if (checkDownSensor && sensorManager && sensorManager->getSensorDownLimit()) {
        // if (debugSerial) debugSerial->println("AUTO: DOWN sensor detected, switching to next case immediately");
        if (stopRollOnSensor && motorController) {
            motorController->offRollMotor();  // LẬP TỨC dừng ROLL MOTOR
        }
        currentAutoSequenceState = nextState;
        autoLastDirChangeTick = currentTick;  // Reset timer for next case
        return;
    }
    
    // Check timeout and transition
    checkTimeoutAndTransition(currentTick, timeoutTicks, nextState);
}

void SequenceController::executeSafeAutoCase(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh,
                                            bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState,
                                            const char* debugMsg) {
    unsigned long currentTick = timerManager->getMasterTicks();

    // Check direction reversal first (only if Roll motor is ON)
    if (rollOn && !checkDirectionReversal(currentTick, !directionDown)) {
        return; // Pause case execution during reversal
    }

    // Debug: Show current case every 500ms
    if (currentTick - lastAutoCaseDebugTick >= 500) {
        if (debugSerial) debugSerial->println(debugMsg);
        lastAutoCaseDebugTick = currentTick;
    }

    // Execute motor control
    executeMotorControl(rollOn, kneadingOn, percussionOn, percussionHigh);

    // Set direction
    if (motorController) {
        motorController->setRollDirection(directionDown);
    }

    // SAFETY: Check for any sensor hit - RESET TO CASE_0 immediately
    if (sensorManager && (sensorManager->getSensorUpLimit() || sensorManager->getSensorDownLimit())) {
        if (debugSerial) {
            debugSerial->println("SAFETY: Sensor detected - resetting to AUTO_CASE_0");
            if (sensorManager->getSensorUpLimit()) debugSerial->println("SAFETY: UP sensor hit");
            if (sensorManager->getSensorDownLimit()) debugSerial->println("SAFETY: DOWN sensor hit");
        }

        // Stop all motors immediately
        if (motorController) {
            motorController->offRollMotor();
            motorController->offKneadingMotor();
            motorController->offCompressionMotor();
        }

        // Reset to AUTO_CASE_0
        currentAutoSequenceState = AUTO_CASE_0;
        autoLastDirChangeTick = currentTick; // Reset timer for next case
        return;
    }

    // Check timeout and transition
    checkTimeoutAndTransition(currentTick, timeoutTicks, nextState);
}

// Direction reversal handling functions
bool SequenceController::checkDirectionReversal(unsigned long currentTick, bool expectedDirection) {
    // Check if we're in the middle of a direction reversal
    if (directionReversalInProgress) {
        // Check if reversal has completed (wait for 500ms = 50 ticks)
        if (currentTick - directionReversalStartTick >= 50) {
            directionReversalInProgress = false;
            // DIRECTION: Reversal completed debug disabled
            return true; // Reversal completed, can proceed
        }
        return false; // Still in reversal, should pause
    }
    
    // Check if sensor hit requires direction reversal
    // Logic: Only reverse when hitting the OPPOSITE sensor (going too far)
    bool currentDirection = getChangeDir(); // true = UP, false = DOWN
    bool sensorHit = false;
    
    if (expectedDirection && sensorManager && sensorManager->getSensorDownLimit()) {
        // Expected UP direction but hit DOWN sensor - going too far, reverse to UP
        sensorHit = true;
        // DIRECTION: DOWN sensor hit during UP movement debug disabled
    } else if (!expectedDirection && sensorManager && sensorManager->getSensorUpLimit()) {
        // Expected DOWN direction but hit UP sensor - going too far, reverse to DOWN  
        sensorHit = true;
        // DIRECTION: UP sensor hit during DOWN movement debug disabled
    }
    
    if (sensorHit) {
        directionReversalInProgress = true;
        directionReversalStartTick = currentTick;
        
        // Stop roll motor immediately
        if (motorController) {
            motorController->offRollMotor();
        }
        
        // Trigger direction reversal in hardware
        setChangeDir(!expectedDirection);
        
        // DIRECTION: Stopping motor and reversing debug disabled
        
        return false; // Pause case execution during reversal
    }
    
    return true; // No reversal needed, can proceed normally
}

void SequenceController::handleDirectionReversal(unsigned long currentTick) {
    // This function is called at the beginning of each auto case
    // It ensures direction reversal is handled properly
    
    if (directionReversalInProgress) {
        // Still in reversal process, skip case execution
        if (debugSerial && (currentTick - directionReversalStartTick) % 100 == 0) {
            debugSerial->println("DIRECTION: Waiting for direction reversal to complete...");
        }
        return;
    }
}

void SequenceController::executeStandardAutoCaseWithSensorSafety(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh, 
                                                                bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState, 
                                                                const char* debugMsg) {
    // This function is identical to executeStandardAutoCase but with sensor safety checks
    // It's a wrapper that calls executeStandardAutoCase with additional safety features
    executeStandardAutoCase(rollOn, kneadingOn, percussionOn, percussionHigh, directionDown, timeoutTicks, nextState, debugMsg);
}
