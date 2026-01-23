#include "MassageController.h"
#include "TimerManager.h"
#include "MotorController.h"
#include "SensorManager.h"
#include "CommunicationManager.h"
#include "SafetyManager.h"
#include "SequenceController.h"

/**
 * Constructor
 */
MassageController::MassageController() 
    : timerManager(nullptr)
    , motorController(nullptr)
    , sensorManager(nullptr)
    , communicationManager(nullptr)
    , safetyManager(nullptr)
    , sequenceController(nullptr)
    , debugSerial(nullptr)
    , bleSerial(nullptr)
    , systemInitialized(false)
    , systemRunning(false)
    , lastLoopTick(0)
    , loopCounter(0)
    , loopFrequency(0.0)
    , lastDebugTick(0)
{
}

/**
 * Destructor
 */
MassageController::~MassageController() {
    stop();
    
    // Clean up subsystems
    if (sequenceController) {
        delete sequenceController;
        sequenceController = nullptr;
    }
    if (safetyManager) {
        delete safetyManager;
        safetyManager = nullptr;
    }
    if (communicationManager) {
        delete communicationManager;
        communicationManager = nullptr;
    }
    if (sensorManager) {
        delete sensorManager;
        sensorManager = nullptr;
    }
    if (motorController) {
        delete motorController;
        motorController = nullptr;
    }
    if (timerManager) {
        delete timerManager;
        timerManager = nullptr;
    }
}

/**
 * Initialize the massage controller system
 */
void MassageController::initialize() {
    if (systemInitialized) {
        // System already initialized debug disabled
        return;
    }
    
    // if (debugSerial) {
    //     debugSerial->println("\n========================================");
    //     debugSerial->println("   MASSAGE CONTROLLER INITIALIZATION");
    //     debugSerial->println("========================================");
    // }
    
    // Initialize subsystems in order
    initializeSubsystems();
    
    // Validate system
    if (validateSubsystems()) {
        systemInitialized = true;
        // if (debugSerial) debugSerial->println("System initialization completed successfully");
    } else {
        // ERROR: System initialization failed debug disabled
        handleSystemError("Subsystem validation failed");
    }
}

/**
 * Start the massage controller system
 */
void MassageController::start() {
    if (!systemInitialized) {
        // ERROR: System not initialized debug disabled
        return;
    }
    
    if (systemRunning) {
        // System already running debug disabled
        return;
    }
    
    // CRITICAL SAFETY: Turn off ALL motors first
    if (motorController) {
        motorController->emergencyStop();
        // if (debugSerial) debugSerial->println("SAFETY: All motors turned OFF at startup");
    }
    
    // Start all subsystems
    if (timerManager) {
        timerManager->startMainTimer();
        timerManager->startPrecisionTimer();
    }
    
    if (safetyManager) {
        safetyManager->initialize();
    }
    
    // CRITICAL SAFETY: Ensure ALL motors are OFF before starting stabilization delay
    if (motorController) {
        motorController->emergencyStop();
        // SAFETY: All motors confirmed OFF debug disabled
    }
    
    // Enable allowRun but wait 2 seconds before starting GO HOME sequence
    if (sequenceController) {
        sequenceController->setAllowRun(true);  // Enable to start home sequence
        sequenceController->setStartupStabilizationDelay(true);  // Enable 2s stabilization delay
        // System: allowRun=TRUE debug disabled
    }
    
    systemRunning = true;
    lastLoopTick = timerManager ? timerManager->getMasterTicks() : 0;
    loopCounter = 0;
    
        // if (debugSerial) debugSerial->println("System started successfully");
    // printSystemStatus(); // Disabled to save FLASH
}

/**
 * Stop the massage controller system
 */
void MassageController::stop() {
    if (!systemRunning) {
        return;
    }
    
    // Stopping massage controller system debug disabled
    
    // Stop all subsystems
    if (sequenceController) {
        sequenceController->setAllowRun(false);
        sequenceController->stopAutoMode();
        sequenceController->stopHomeSequence();
    }
    
    if (motorController) {
        motorController->emergencyStop();
    }
    
    if (timerManager) {
        timerManager->stopMainTimer();
        timerManager->stopPrecisionTimer();
    }
    
    systemRunning = false;
    // System stopped debug disabled
}

/**
 * Main run function - called from Arduino loop()
 */
void MassageController::run() {
    if (!systemInitialized || !systemRunning) {
        return;
    }
    
    processMainLoop();
}

/**
 * Check if system is initialized
 */
bool MassageController::isInitialized() const {
    return systemInitialized;
}

/**
 * Check if system is running
 */
bool MassageController::isRunning() const {
    return systemRunning;
}

/**
 * Print system status
 */
void MassageController::printSystemStatus() const {
    // Disabled to save FLASH memory
    // if (!debugSerial) return;
    // 
    // debugSerial->println("\n=== MASSAGE CONTROLLER STATUS ===");
    // debugSerial->print("Initialized: "); debugSerial->println(systemInitialized ? "YES" : "NO");
    // debugSerial->print("Running: "); debugSerial->println(systemRunning ? "YES" : "NO");
    // debugSerial->print("Loop Counter: "); debugSerial->println(loopCounter);
    // debugSerial->print("Last Loop Tick: "); debugSerial->println(lastLoopTick);
    // 
    // if (timerManager) {
    //     debugSerial->print("Master Ticks: "); debugSerial->println(timerManager->getMasterTicks());
    //     debugSerial->print("Precision Ticks: "); debugSerial->println(timerManager->getPrecisionTicks());
    // }
    // 
    // debugSerial->println("================================");
}

/**
 * Print subsystem status
 */
void MassageController::printSubsystemStatus() const {
    // SUBSYSTEM STATUS debug disabled to save Flash memory
}

/**
 * Get subsystem accessors
 */
TimerManager* MassageController::getTimerManager() const {
    return timerManager;
}

MotorController* MassageController::getMotorController() const {
    return motorController;
}

SensorManager* MassageController::getSensorManager() const {
    return sensorManager;
}

CommunicationManager* MassageController::getCommunicationManager() const {
    return communicationManager;
}

SafetyManager* MassageController::getSafetyManager() const {
    return safetyManager;
}

SequenceController* MassageController::getSequenceController() const {
    return sequenceController;
}

/**
 * Enable system
 */
void MassageController::enableSystem() {
    if (sequenceController) {
        sequenceController->setAllowRun(true);
    }
    // System enabled debug disabled
}

/**
 * Disable system
 */
void MassageController::disableSystem() {
    if (sequenceController) {
        sequenceController->setAllowRun(false);
    }
    // System disabled debug disabled
}

/**
 * Reset system
 */
void MassageController::resetSystem() {
    // Resetting system debug disabled
    stop();
    delay(100);
    initialize();
    start();
}

/**
 * Emergency stop
 */
void MassageController::emergencyStop() {
    // EMERGENCY STOP ACTIVATED debug disabled
    
    if (safetyManager) {
        safetyManager->emergencyStop();
    }
    
    if (motorController) {
        motorController->emergencyStop();
    }
    
    if (sequenceController) {
        sequenceController->setAllowRun(false);
        sequenceController->stopAutoMode();
    }
    
    systemRunning = false;
}

/**
 * Process main loop
 */
void MassageController::processMainLoop() {
    unsigned long currentTick = timerManager ? timerManager->getMasterTicks() : 0;
    
    // Update loop statistics
    updateLoopStatistics();
    
    // Process all subsystems
    processCommunication();
    processSensors();
    processSequences();
    processSafety();
    processMotors();
    
    // Process debug output
    processDebugOutput();
    
    // Process system monitoring
    processSystemMonitoring();
    
    lastLoopTick = currentTick;
    loopCounter++;
}

/**
 * Process communication
 */
void MassageController::processCommunication() {
    if (communicationManager) {
        communicationManager->serial2DataIncome();
        communicationManager->checkCommandCounters();
    }
}

/**
 * Process sensors
 */
void MassageController::processSensors() {
    if (sensorManager) {
        sensorManager->processSensorConfirmation();
        sensorManager->updateSensorStates();
    }
}

/**
 * Process sequences
 */
void MassageController::processSequences() {
    if (sequenceController) {
        // Sync manual priority from CommunicationManager to SequenceController
        if (communicationManager) {
            bool commManualPriority = communicationManager->getManualPriority();
            bool seqManualPriority = sequenceController->getManualPriority();
            
            if (commManualPriority != seqManualPriority) {
                sequenceController->setManualPriority(commManualPriority);
                // Manual Priority synced debug disabled
            }
        }
        
        sequenceController->processGoHome();
        sequenceController->processAuto();
    }
}

/**
 * Process safety
 */
void MassageController::processSafety() {
    if (safetyManager) {
        safetyManager->checkSystemHealth();
        safetyManager->watchdogFeed();
    }
}

/**
 * Process motors
 */
void MassageController::processMotors() {
    if (motorController) {
        motorController->checkMotorTimeouts();
        motorController->resetRL1RL2();  // Process RL1/RL2 motor management
    }
}

/**
 * Check system health
 */
void MassageController::checkSystemHealth() {
    if (safetyManager) {
        safetyManager->checkSystemHealth();
    }
    
    if (!isSystemHealthy()) {
        handleSystemError("System health check failed");
    }
}

/**
 * Perform system diagnostics
 */
void MassageController::performSystemDiagnostics() {
    // SYSTEM DIAGNOSTICS debug disabled
    
    // Check subsystems
    printSubsystemStatus();
    
    // Check system state
    if (sequenceController) {
        sequenceController->printSystemStatus();
    }
    
    // Check safety status
    if (safetyManager) {
        safetyManager->printSafetyStatus();
    }
    
    // DIAGNOSTICS END debug disabled
}

/**
 * Check if system is healthy
 */
bool MassageController::isSystemHealthy() const {
    // Check if all subsystems are present
    if (!timerManager || !motorController || !sensorManager || 
        !communicationManager || !safetyManager || !sequenceController) {
        return false;
    }
    
    // Check safety manager
    if (safetyManager && safetyManager->isSystemStuck()) {
        return false;
    }
    
    // Check for emergency stop
    if (safetyManager && safetyManager->isEmergencyStopActive()) {
        return false;
    }
    
    return true;
}

/**
 * Handle system error
 */
void MassageController::handleSystemError(const char* errorMessage) {
    // SYSTEM ERROR debug disabled
    
    // Log the error
    logSystemEvent(errorMessage);
    
    // Perform error recovery
    performErrorRecovery();
}

/**
 * Perform error recovery
 */
void MassageController::performErrorRecovery() {
    // Performing error recovery debug disabled
    
    // Try to recover from error
    recoverFromError();
    
    // If recovery fails, perform system reset
    if (!isSystemHealthy()) {
        // Error recovery failed debug disabled
        performSystemReset();
    }
}

/**
 * Log system event
 */
void MassageController::logSystemEvent(const char* event) {
    // EVENT LOG debug disabled
}

/**
 * Set debug serial
 */
void MassageController::setDebugSerial(HardwareSerial* serial) {
    debugSerial = serial;
    // if (debugSerial) {
    //     debugSerial->println("MassageController: Debug serial interface set");
    // }
}

/**
 * Set BLE serial
 */
void MassageController::setBleSerial(HardwareSerial* serial) {
    bleSerial = serial;
}

/**
 * Get loop counter
 */
unsigned long MassageController::getLoopCounter() const {
    return loopCounter;
}

/**
 * Get last loop tick
 */
unsigned long MassageController::getLastLoopTick() const {
    return lastLoopTick;
}

/**
 * Get loop frequency
 */
float MassageController::getLoopFrequency() const {
    return loopFrequency;
}

/**
 * Private Helper Functions
 */
void MassageController::initializeSubsystems() {
    // if (debugSerial) debugSerial->println("Initializing subsystems...");
    
    // Initialize in dependency order
    initializeSerial();
    initializeTimers();
    initializeMotors();
    initializeSensors();
    initializeCommunication();
    initializeSafety();
    initializeSequences();
    
    // if (debugSerial) debugSerial->println("Subsystem initialization completed");
}

void MassageController::initializeSerial() {
    // if (debugSerial) debugSerial->println("Initializing serial communication...");
    
    // Initialize debug serial
    if (debugSerial) {
        debugSerial->begin(115200);
        delay(100);
    }
    
    // Initialize BLE serial
    if (bleSerial) {
        bleSerial->begin(9600);
        delay(100);
    }
    
    // if (debugSerial) debugSerial->println("Serial communication initialized");
}

void MassageController::initializeTimers() {
    // if (debugSerial) debugSerial->println("Initializing timer manager...");
    
    timerManager = new TimerManager(debugSerial);
    timerManager->initialize();
    
    // if (debugSerial) debugSerial->println("Timer manager initialized");
}

void MassageController::initializeMotors() {
    // if (debugSerial) debugSerial->println("Initializing motor controller...");
    
    motorController = new MotorController(timerManager, debugSerial);
    motorController->initialize();
    
    // if (debugSerial) debugSerial->println("Motor controller initialized");
}

void MassageController::initializeSensors() {
    // if (debugSerial) debugSerial->println("Initializing sensor manager...");
    
    sensorManager = new SensorManager(timerManager, motorController, debugSerial);
    sensorManager->initialize();
    sensorManager->setupInterrupts();
    
    // if (debugSerial) debugSerial->println("Sensor manager initialized");
}

void MassageController::initializeCommunication() {
    // if (debugSerial) debugSerial->println("Initializing communication manager...");
    
    communicationManager = new CommunicationManager(timerManager, debugSerial, bleSerial);
    communicationManager->initialize();
    
    // NOTE: setControllers() will be called AFTER sequenceController is initialized
    // in initializeSequences() to avoid NULL pointer issues
    
    // if (debugSerial) debugSerial->println("Communication manager initialized");
}

void MassageController::initializeSafety() {
    // if (debugSerial) debugSerial->println("Initializing safety manager...");
    
    safetyManager = new SafetyManager(timerManager, debugSerial);
    safetyManager->initialize();
    
    // if (debugSerial) debugSerial->println("Safety manager initialized");
}

void MassageController::initializeSequences() {
    // if (debugSerial) debugSerial->println("Initializing sequence controller...");
    
    sequenceController = new SequenceController(timerManager, motorController, sensorManager, debugSerial);
    sequenceController->initialize();
    
    // NOW set controller references in CommunicationManager (after sequenceController is initialized)
    if (communicationManager) {
        communicationManager->setControllers((void*)motorController, (void*)sequenceController, (void*)sensorManager);
        // if (debugSerial) debugSerial->println("Controller references set in CommunicationManager");
    }
    
    // if (debugSerial) debugSerial->println("Sequence controller initialized");
}

void MassageController::updateLoopStatistics() {
    // Update loop statistics for monitoring
    unsigned long currentTick = timerManager ? timerManager->getMasterTicks() : 0;
    
    // Calculate loop frequency (Hz)
    if (currentTick > lastLoopTick) {
        loopFrequency = 100.0 / (currentTick - lastLoopTick);  // 100 ticks = 1 second
    }
}

void MassageController::processDebugOutput() {
    if (!timerManager) return;
    
    unsigned long currentTick = timerManager->getMasterTicks();
    
    // Print debug information at regular intervals
    if (currentTick - lastDebugTick >= LOOP_DEBUG_INTERVAL_TICKS) {
        printLoopStatistics();
        lastDebugTick = currentTick;
    }
}

void MassageController::processSystemMonitoring() {
    // Monitor system performance and health
    monitorSystemPerformance();
}

bool MassageController::validateSubsystems() const {
    return (timerManager != nullptr &&
            motorController != nullptr &&
            sensorManager != nullptr &&
            communicationManager != nullptr &&
            safetyManager != nullptr &&
            sequenceController != nullptr);
}

bool MassageController::validateSystemState() const {
    return systemInitialized && isSystemHealthy();
}

bool MassageController::validateHardware() const {
    // Validate hardware components
    return true;  // Placeholder
}

void MassageController::resetSubsystems() {
    if (debugSerial) debugSerial->println("Resetting subsystems...");
    
    if (sequenceController) {
        sequenceController->resetAllModes();
    }
    
    if (motorController) {
        motorController->emergencyStop();
    }
    
    if (sensorManager) {
        sensorManager->resetSensorStates();
    }
}

void MassageController::recoverFromError() {
    if (debugSerial) debugSerial->println("Attempting error recovery...");
    
    // Try to reset subsystems
    resetSubsystems();
    
    // Check if recovery was successful
    if (isSystemHealthy()) {
        if (debugSerial) debugSerial->println("Error recovery successful");
    } else {
        if (debugSerial) debugSerial->println("Error recovery failed");
    }
}

void MassageController::performSystemReset() {
    if (debugSerial) debugSerial->println("Performing complete system reset...");
    
    // Stop system
    stop();
    
    // Reset all subsystems
    resetSubsystems();
    
    // Reinitialize
    initialize();
    start();
}

void MassageController::printLoopStatistics() const {
    // Disabled to save FLASH memory
    // if (debugSerial) {
    //     debugSerial->print("Loop #");
    //     debugSerial->print(loopCounter);
    //     debugSerial->print(", Frequency: ");
    //     debugSerial->print(getLoopFrequency());
    //     debugSerial->print(" Hz, Ticks: ");
    // }
    // if (debugSerial) debugSerial->println(timerManager ? timerManager->getMasterTicks() : 0);
}

void MassageController::printSystemConfiguration() const {
    if (!debugSerial) return;
    
    debugSerial->println("\n=== SYSTEM CONFIGURATION ===");
    debugSerial->print("Debug Serial: "); debugSerial->println(debugSerial ? "Enabled" : "Disabled");
    debugSerial->print("BLE Serial: "); debugSerial->println(bleSerial ? "Enabled" : "Disabled");
    debugSerial->println("=============================");
}


void MassageController::monitorSystemPerformance() {
    // Monitor system performance metrics
    // This could include memory usage, CPU load, etc.
}
