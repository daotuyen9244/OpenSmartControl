#ifndef MASSAGE_CONTROLLER_H
#define MASSAGE_CONTROLLER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "TimerManager.h"
#include "MotorController.h"
#include "SensorManager.h"
#include "CommunicationManager.h"
#include "SafetyManager.h"
#include "SequenceController.h"

/**
 * MassageController Class
 * 
 * Main controller class that coordinates all subsystems of the massage chair system.
 * This is the central orchestrator that manages the interaction between all components.
 * 
 * Features:
 * - System initialization and coordination
 * - Main loop management
 * - Subsystem integration
 * - System state management
 * - Error handling and recovery
 */
class MassageController {
private:
    // Subsystem components
    TimerManager* timerManager;
    MotorController* motorController;
    SensorManager* sensorManager;
    CommunicationManager* communicationManager;
    SafetyManager* safetyManager;
    SequenceController* sequenceController;
    
    // Serial interfaces
    HardwareSerial* debugSerial;
    HardwareSerial* bleSerial;
    
    // System state
    bool systemInitialized;
    bool systemRunning;
    unsigned long lastLoopTick;
    unsigned long loopCounter;
    float loopFrequency;
    
    // System configuration
    static const unsigned long LOOP_DEBUG_INTERVAL_TICKS = 1000;  // 10 seconds
    unsigned long lastDebugTick;

public:
    // Constructor
    MassageController();
    
    // Destructor
    ~MassageController();
    
    // System Lifecycle
    void initialize();
    void start();
    void stop();
    void run();
    
    // System Status
    bool isInitialized() const;
    bool isRunning() const;
    void printSystemStatus() const;
    void printSubsystemStatus() const;
    float getLoopFrequency() const;
    
    // Subsystem Access
    TimerManager* getTimerManager() const;
    MotorController* getMotorController() const;
    SensorManager* getSensorManager() const;
    CommunicationManager* getCommunicationManager() const;
    SafetyManager* getSafetyManager() const;
    SequenceController* getSequenceController() const;
    
    // System Control
    void enableSystem();
    void disableSystem();
    void resetSystem();
    void emergencyStop();
    
    // Main Loop Processing
    void processMainLoop();
    void processCommunication();
    void processSensors();
    void processSequences();
    void processSafety();
    void processMotors();
    
    // System Health
    void checkSystemHealth();
    void performSystemDiagnostics();
    bool isSystemHealthy() const;
    
    // Error Handling
    void handleSystemError(const char* errorMessage);
    void performErrorRecovery();
    void logSystemEvent(const char* event);
    
    // Configuration
    void setDebugSerial(HardwareSerial* serial);
    void setBleSerial(HardwareSerial* serial);
    
    // Loop Statistics
    unsigned long getLoopCounter() const;
    unsigned long getLastLoopTick() const;

private:
    // Initialization helpers
    void initializeSubsystems();
    void initializeSerial();
    void initializeHardware();
    void initializeTimers();
    void initializeMotors();
    void initializeSensors();
    void initializeCommunication();
    void initializeSafety();
    void initializeSequences();
    
    // Main loop helpers
    void updateLoopStatistics();
    void processDebugOutput();
    void processSystemMonitoring();
    
    // System validation
    bool validateSubsystems() const;
    bool validateSystemState() const;
    bool validateHardware() const;
    
    // Error recovery
    void resetSubsystems();
    void recoverFromError();
    void performSystemReset();
    
    // Debug and monitoring
    void printLoopStatistics() const;
    void printSystemConfiguration() const;
    void monitorSystemPerformance();
};

#endif // MASSAGE_CONTROLLER_H
