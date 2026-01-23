#ifndef SEQUENCE_CONTROLLER_H
#define SEQUENCE_CONTROLLER_H

#include <Arduino.h>
#include "TimerManager.h"
#include "MotorController.h"
#include "SensorManager.h"
#include "Massage_v1_hardware.h"

/**
 * SequenceController Class
 * 
 * Manages all sequence operations for the massage chair system.
 * Handles home sequence, auto sequences, and program execution.
 * 
 * Features:
 * - Home sequence management
 * - Auto program execution
 * - Sequence state machines
 * - Program timing and control
 * - Mode management
 */
class SequenceController {
public:
    // Home sequence states
    enum HomeState {
        HOME_IDLE = 0,
        HOME_SEARCHING_UP = 1,
        HOME_DELAY_AT_UP = 2,
        HOME_RUNNING_DOWN = 3
    };
    
    // Auto program types
    enum AutoProgram {
        AUTO_NONE = 0,
        AUTO_DEFAULT = 1,
        AUTO_KNEADING = 2,
        AUTO_COMPRESSION = 3,
        AUTO_PERCUSSION = 4,
        AUTO_COMBINED = 5
    };
    
    // Sequence states for different programs
    enum AutoSequenceState {
        AUTO_CASE_0 = 0,
        AUTO_CASE_1 = 1,
        AUTO_CASE_2 = 2,
        AUTO_CASE_3 = 3,
        AUTO_CASE_4 = 4,
        AUTO_CASE_5 = 5,
        AUTO_CASE_6 = 6,
        AUTO_CASE_7 = 7,
        AUTO_CASE_8 = 8,
        AUTO_CASE_9 = 9,
        AUTO_CASE_10 = 10,
        AUTO_CASE_11 = 11,
        AUTO_CASE_12 = 12,
        AUTO_CASE_13 = 13,
        AUTO_CASE_14 = 14,
        AUTO_CASE_15 = 15,
        AUTO_CASE_16 = 16,
        AUTO_CASE_17 = 17,
        AUTO_CASE_18 = 18,
        AUTO_CASE_19 = 19,
        AUTO_CASE_20 = 20,
        AUTO_CASE_21 = 21,
        AUTO_CASE_22 = 22,
        AUTO_CASE_23 = 23,
        AUTO_CASE_24 = 24,
        AUTO_CASE_25 = 25,
        AUTO_CASE_26 = 26,
        AUTO_CASE_27 = 27,
        AUTO_CASE_28 = 28,
        AUTO_CASE_29 = 29,
        AUTO_CASE_30 = 30,
        AUTO_CASE_31 = 31,
        AUTO_CASE_32 = 32,
        AUTO_CASE_33 = 33,
        AUTO_CASE_34 = 34,
        AUTO_CASE_35 = 35,
        AUTO_CASE_36 = 36,
        AUTO_CASE_37 = 37,
        AUTO_CASE_38 = 38,
        AUTO_CASE_39 = 39,
        AUTO_CASE_40 = 40,
        AUTO_CASE_41 = 41,
        AUTO_CASE_42 = 42,
        AUTO_CASE_43 = 43,
        AUTO_CASE_44 = 44,
        AUTO_CASE_45 = 45,
        AUTO_CASE_46 = 46,
        AUTO_CASE_47 = 47,
        AUTO_CASE_48 = 48,
        AUTO_CASE_49 = 49,
        AUTO_CASE_50 = 50,
        AUTO_CASE_51 = 51,
        AUTO_CASE_52 = 52,
        AUTO_CASE_53 = 53,
        AUTO_CASE_54 = 54,
        AUTO_CASE_55 = 55,
        AUTO_CASE_56 = 56,
        AUTO_CASE_57 = 57,
        AUTO_CASE_58 = 58,
        AUTO_CASE_59 = 59,
        AUTO_CASE_60 = 60,
        AUTO_CASE_61 = 61,
        AUTO_CASE_62 = 62,
        AUTO_CASE_63 = 63,
        AUTO_CASE_64 = 64,
        AUTO_CASE_65 = 65,
        AUTO_CASE_66 = 66,
        AUTO_CASE_67 = 67,
        AUTO_CASE_68 = 68,
        AUTO_CASE_69 = 69,
        AUTO_CASE_70 = 70,
        AUTO_CASE_71 = 71,
        AUTO_CASE_72 = 72,
        AUTO_CASE_73 = 73,
        AUTO_CASE_74 = 74,
        AUTO_CASE_75 = 75,
        AUTO_CASE_76 = 76,
        AUTO_CASE_77 = 77,
        AUTO_CASE_78 = 78,
        AUTO_CASE_79 = 79,
        AUTO_CASE_80 = 80,
        AUTO_CASE_81 = 81,
        AUTO_CASE_82 = 82,
        AUTO_CASE_83 = 83,
        AUTO_CASE_84 = 84,
        AUTO_CASE_85 = 85,
        AUTO_CASE_86 = 86,
        AUTO_CASE_87 = 87,
        AUTO_CASE_88 = 88,
        AUTO_CASE_89 = 89,
        AUTO_CASE_90 = 90,
        AUTO_CASE_91 = 91,
        AUTO_CASE_92 = 92,
        AUTO_CASE_93 = 93,
        AUTO_CASE_94 = 94,
        AUTO_CASE_95 = 95,
        AUTO_CASE_96 = 96,
        AUTO_CASE_97 = 97
        
    };
    
    enum KneadingSequenceState {
        KNEADING_CASE_0 = 0,
        KNEADING_CASE_1 = 1
    };
    
    enum CompressionSequenceState {
        COMPRESSION_CASE_0 = 0,
        COMPRESSION_CASE_1 = 1,
        COMPRESSION_CASE_2 = 2
    };
    
    enum PercussionSequenceState {
        PERCUSSION_CASE_0 = 0,
        PERCUSSION_CASE_1 = 1
    };
    
    enum CombinedSequenceState {
        COMBINED_CASE_0 = 0,
        COMBINED_CASE_1 = 1
    };

private:
    // Timing constants (in ticks) - renamed to avoid macro conflicts
    static const unsigned long SEQ_HOME_TOTAL_TIMEOUT_TICKS = 6000;      // 60s
    static const unsigned long SEQ_HOME_STEP3_TIMEOUT_TICKS = 500;       // 5s
    static const unsigned long SEQ_HOME_STEP3_RUN_TICKS = 200;           // 2s
    static const unsigned long SEQ_HOME_INACTIVITY_TIMEOUT_TICKS = 3000; // 30s
    static const unsigned long SEQ_HOME_DIR_CHANGE_TICKS = 10;           // 100ms
    static const unsigned long SEQ_AUTO_DIR_CHANGE_TICKS = 10;           // 100ms
    static const unsigned long SEQ_AUTO_MODE_DURATION_TICKS = 120000;    // 20 minutes
    
    // System control flags
    bool allowRun;
    bool homeRun;
    bool modeAuto;
    bool manualPriority;
    bool startupStabilizationDelay;
    bool rollMotorUserDisabled;  // Track if user disabled roll motor
    
    // Auto mode timeout
    bool autoModeTimerActive;
    unsigned long autoModeStartTick;
    
    // AUTO program mode flags
    bool autodefaultMode;
    bool rollSpotMode;
    bool kneadingMode;
    bool compressionMode;
    bool percussionMode;
    bool combineMode;
    
    // Feature flags
    uint8_t intensityLevel;
    bool useHighPrecisionTimer;
    
    // Home sequence variables
    HomeState currentHomeState;
    byte stepHomeRun;
    unsigned long homeStartTick;
    unsigned long homeStepStartTick;
    unsigned long homeLastResetTick;
    bool homeMotorStarted;  // Flag to ensure motor runs only once per state
    unsigned long homeLastDownSensorDebugTick;  // Debug tick for DOWN sensor logging
    
    // Auto mode variables
    AutoProgram currentAutoProgram;
    AutoProgram previousAutoProgram;
    bool resetProgramStatesFlag;
    unsigned long programSwitchCount;
    unsigned long lastProgramSwitchTick;
    
    // Auto mode timing
    unsigned long autoLastDirChangeTick;
    unsigned long autoModeElapsedTicks;
    bool autoTimerStarted;
    
    // Shared debug timer for all auto cases
    unsigned long lastAutoCaseDebugTick;
    
    // Direction reversal handling
    bool directionReversalInProgress;
    unsigned long directionReversalStartTick;
    
    // Auto total timer (20 minutes)
    unsigned long autoTotalStartTick;
    unsigned long autoTotalElapsedTicks;
    bool autoTotalTimerActive;
    bool autoTotalTimerExpired;
    
    // Sequence state variables
    AutoSequenceState currentAutoSequenceState;
    KneadingSequenceState currentKneadingSequenceState;
    CompressionSequenceState currentCompressionSequenceState;
    PercussionSequenceState currentPercussionSequenceState;
    CombinedSequenceState currentCombinedSequenceState;
    
    // Sequence timing variables
    unsigned long autoSequenceStartTick;
    unsigned long autoStopStartTick;
    bool autoStopped;
    
    unsigned long kneadingSequenceStartTick;
    unsigned long kneadingStopStartTick;
    bool kneadingStopped;
    
    unsigned long compressionSequenceStartTick;
    unsigned long compressionStopStartTick;
    bool compressionStopped;
    
    unsigned long percussionSequenceStartTick;
    unsigned long percussionStopStartTick;
    bool percussionStopped;
    
    unsigned long combinedSequenceStartTick;
    unsigned long combinedStopStartTick;
    bool combinedStopped;
    
    // Sequence started flags
    bool autoSequenceStarted;
    bool kneadingSequenceStarted;
    bool compressionSequenceStarted;
    bool percussionSequenceStarted;
    bool combinedSequenceStarted;
    
    // Component references
    TimerManager* timerManager;
    MotorController* motorController;
    SensorManager* sensorManager;
    HardwareSerial* debugSerial;

public:
    // Constructor
    SequenceController(TimerManager* timerMgr, MotorController* motorCtrl, SensorManager* sensorMgr, HardwareSerial* debugSer = nullptr);
    
    // Destructor
    ~SequenceController();
    
    // Initialization
    void initialize();
    
    // Home Sequence Management
    void processGoHome();
    void startHomeSequence();
    void stopHomeSequence();
    bool isHomeSequenceActive() const;
    HomeState getCurrentHomeState() const;
    void setCurrentHomeState(HomeState state);
    
    // Auto Mode Management
    void processAuto();
    void startAutoMode();
    void stopAutoMode();
    bool isAutoModeActive() const;
    AutoProgram getCurrentAutoProgram() const;
    void setCurrentAutoProgram(AutoProgram program);
    
    // Program Detection and Management
    AutoProgram detectAutoProgram();
    void printAutoModeStatus();
    unsigned long getAutoRemainingTime();
    void resetProgramStates();
    void resetSequenceStatesOnly();
    
    // Mode Flag Management
    bool getAllowRun() const;
    void setAllowRun(bool value);
    bool getHomeRun() const;
    void setHomeRun(bool value);
    bool getModeAuto() const;
    void setModeAuto(bool value);
    bool getManualPriority() const;
    void setManualPriority(bool value);
    bool getRollMotorUserDisabled() const;
    void setRollMotorUserDisabled(bool disabled);
    bool getStartupStabilizationDelay() const;
    void setStartupStabilizationDelay(bool value);
    
    // Program Mode Flags
    bool getAutodefaultMode() const;
    void setAutodefaultMode(bool value);
    bool getRollSpotMode() const;
    void setRollSpotMode(bool value);
    bool getKneadingMode() const;
    void setKneadingMode(bool value);
    bool getCompressionMode() const;
    void setCompressionMode(bool value);
    bool getPercussionMode() const;
    void setPercussionMode(bool value);
    bool getCombineMode() const;
    void setCombineMode(bool value);
    
    // Feature Flags
    uint8_t getIntensityLevel() const;
    void setIntensityLevel(uint8_t value);
    void setIntensityOff(const char* reason = nullptr);
    uint8_t getIntensityForProgram(AutoProgram program) const;  // Get intensity for specific program
    bool getUseHighPrecisionTimer() const;
    void setUseHighPrecisionTimer(bool value);
    
    // Program Execution
    void executeAutoDefaultProgram();
    void executeKneadingProgram();
    void executeCompressionProgram();
    void executePercussionProgram();
    void executeCombinedProgram();
    
    // Sequence State Machines
    void runAutoDefaultSequence();
    void runKneadingSequence();
    void runCompressionSequence();
    void runPercussionSequence();
    void runCombinedSequence();
    
    // Helper Functions
    bool checkProgramConditions();
    bool isRollMotorToggleAllowed();
    bool isIntensityChangeAllowed();
    void printSystemStatus();
    void resetAllModes();
    
    // Timing Getters/Setters
    unsigned long getHomeStartTick() const;
    void setHomeStartTick(unsigned long tick);
    unsigned long getHomeStepStartTick() const;
    void setHomeStepStartTick(unsigned long tick);
    unsigned long getHomeLastResetTick() const;
    void setHomeLastResetTick(unsigned long tick);
    
    unsigned long getAutoLastDirChangeTick() const;
    void setAutoLastDirChangeTick(unsigned long tick);
    unsigned long getAutoModeStartTick() const;
    void setAutoModeStartTick(unsigned long tick);
    unsigned long getAutoModeElapsedTicks() const;
    void setAutoModeElapsedTicks(unsigned long tick);
    bool getAutoModeTimerActive() const;
    void setAutoModeTimerActive(bool active);
    bool getAutoTimerStarted() const;
    void setAutoTimerStarted(bool started);
    
    // Auto Total Timer
    unsigned long getAutoTotalStartTick() const;
    void setAutoTotalStartTick(unsigned long tick);
    unsigned long getAutoTotalElapsedTicks() const;
    void setAutoTotalElapsedTicks(unsigned long tick);
    bool getAutoTotalTimerActive() const;
    void setAutoTotalTimerActive(bool active);
    bool getAutoTotalTimerExpired() const;
    void setAutoTotalTimerExpired(bool expired);

private:
    // Helper functions
    void updateAutoModeTimer();
    void updateAutoTotalTimer();
    void handleProgramSwitch();
    void executeCurrentProgram();
    
    // Home sequence helpers
    void handleHomeStateIdle();
    void handleHomeStateSearchingUp();
    void handleHomeStateDelayAtUp();
    void handleHomeStateRunningDown();
    
    // Auto sequence helpers
    void handleAutoCase0();
    void handleAutoCase1();
    void handleAutoCase2();
    void handleAutoCase3();
    void handleAutoCase4();
    void handleAutoCase5();
    void handleAutoCase6();
    void handleAutoCase7();
    void handleAutoCase8();
    void handleAutoCase9();
    void handleAutoCase10();
    void handleAutoCase11();
    void handleAutoCase12();
    void handleAutoCase13();
    void handleAutoCase14();
    void handleAutoCase15();
    void handleAutoCase16();
    void handleAutoCase17();
    void handleAutoCase18();
    void handleAutoCase19();
    void handleAutoCase20();
    void handleAutoCase21();
    void handleAutoCase22();
    void handleAutoCase23();
    void handleAutoCase24();
    void handleAutoCase25();
    void handleAutoCase26();
    void handleAutoCase27();
    void handleAutoCase28();
    void handleAutoCase29();
    void handleAutoCase30();
    void handleAutoCase31();
    void handleAutoCase32();
    void handleAutoCase33();
    void handleAutoCase34();
    void handleAutoCase35();
    void handleAutoCase36();
    void handleAutoCase37();
    void handleAutoCase38();
    void handleAutoCase39();
    void handleAutoCase40();
    void handleAutoCase41();
    void handleAutoCase42();
    void handleAutoCase43();
    void handleAutoCase44();
    void handleAutoCase45();
    void handleAutoCase46();
    void handleAutoCase47();
    void handleAutoCase48();
    void handleAutoCase49();
    void handleAutoCase50();
    void handleAutoCase51();
    void handleAutoCase52();
    void handleAutoCase53();
    void handleAutoCase54();
    void handleAutoCase55();
    void handleAutoCase56();
    void handleAutoCase57();
    void handleAutoCase58();
    void handleAutoCase59();
    void handleAutoCase60();
    void handleAutoCase61();
    void handleAutoCase62();
    void handleAutoCase63();
    void handleAutoCase64();
    void handleAutoCase65();
    void handleAutoCase66();
    void handleAutoCase67();
    void handleAutoCase68();
    void handleAutoCase69();
    void handleAutoCase70();
    void handleAutoCase71();
    void handleAutoCase72();
    void handleAutoCase73();
    void handleAutoCase74();
    void handleAutoCase75();
    void handleAutoCase76();
    void handleAutoCase77();
    void handleAutoCase78();
    void handleAutoCase79();
    void handleAutoCase80();
    void handleAutoCase81();
    void handleAutoCase82();
    void handleAutoCase83();
    void handleAutoCase84();
    void handleAutoCase85();
    void handleAutoCase86();
    void handleAutoCase87();
    void handleAutoCase88();
    void handleAutoCase89();
    void handleAutoCase90();
    void handleAutoCase91();
    void handleAutoCase92();
    void handleAutoCase93();
    void handleAutoCase94();
    void handleAutoCase95();
    void handleAutoCase96();
    void handleAutoCase97();
    void handleKneadingCase0();
    void handleKneadingCase1();
    void handleCompressionCase0();
    void handleCompressionCase1();
    void handleCompressionCase2();
    void handlePercussionCase0();
    void handlePercussionCase1();
    void handleCombinedCase0();
    void handleCombinedCase1();
    
    // Timing helpers
    bool isTimeForDirectionChange() const;
    bool isHomeSequenceTimeout() const;
    bool isAutoModeTimeout() const;
    bool isAutoTotalTimeout() const;
    
    // State validation
    bool isValidHomeState(HomeState state) const;
    bool isValidAutoProgram(AutoProgram program) const;
    bool canStartAutoMode() const;
    bool canStartHomeSequence() const;
    
    // Helper functions for auto cases optimization
    void executeMotorControl(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh = false);
    bool checkTimeoutAndTransition(unsigned long currentTick, unsigned long timeoutTicks, AutoSequenceState nextState);
    void executeStandardAutoCase(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh, 
                                bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState, 
                                const char* debugMsg);
    void executeSensorBasedAutoCase(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh,
                                   bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState,
                                   const char* debugMsg, bool checkUpSensor = false, bool checkDownSensor = false,
                                   bool stopRollOnSensor = true);
    void executeSensorBasedAutoCaseWithMotorStop(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh,
                                                bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState,
                                                const char* debugMsg, bool checkUpSensor = false, bool checkDownSensor = false);
    void executeSafeAutoCase(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh,
                            bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState,
                            const char* debugMsg);
    void executeStandardAutoCaseWithSensorSafety(bool rollOn, bool kneadingOn, bool percussionOn, bool percussionHigh,
                                                bool directionDown, unsigned long timeoutTicks, AutoSequenceState nextState,
                                                const char* debugMsg);
    
    // Direction reversal handling
    bool checkDirectionReversal(unsigned long currentTick, bool expectedDirection);
    void handleDirectionReversal(unsigned long currentTick);
};

#endif // SEQUENCE_CONTROLLER_H
