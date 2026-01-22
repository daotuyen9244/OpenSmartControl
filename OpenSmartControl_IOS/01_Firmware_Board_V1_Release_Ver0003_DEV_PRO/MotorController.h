#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "TimerManager.h"
#include "PinDefinitions.h"

/**
 * MotorController Class
 * 
 * Manages all motor control operations for the massage chair system.
 * Handles RL1 (Recline/Incline), RL2 (Forward/Backward), RL3 (Roll Motor),
 * and specialized motors (Kneading, Compression).
 * 
 * Features:
 * - Relay-based motor control (RL1, RL2, RL3)
 * - PWM motor control (Kneading, Compression)
 * - Direction control
 * - Motor state management
 * - Safety timeouts
 * - Non-blocking motor operations
 */
class MotorController {
public:
  // Motor state enums
  enum MotorState {
    MOTOR_STOP = 0,
    MOTOR_RUN = 1
  };

  enum MotorDirection {
    UP_TO_DOWN = 0,
    DOWN_TO_UP = 1
  };

  // Motor types
  enum MotorType {
    RL1_RECLINE_INCLINE,
    RL2_FORWARD_BACKWARD,
    RL3_ROLL_MOTOR,
    KNEADING_MOTOR,
    COMPRESSION_MOTOR
  };

private:
  // Pin definitions
  static const int RL1_PWM = RL1_PWM_PIN;
  static const int RL1_DIR = RL1_DIR_PIN;
  static const int RL2_PWM = RL2_PWM_PIN;
  static const int RL2_DIR = RL2_DIR_PIN;
  static const int RL3_PWM = RL3_PWM_PIN;
  static const int RL3_DIR = RL3_DIR_PIN;
  static const int FETT_PWM = FETT_PWM_PIN;  // Kneading
  static const int FETK_PWM = FETK_PWM_PIN;  // Compression

  // Motor state variables
  bool rl1Running;
  bool rl2Running;
  bool rl3Running;
  bool kneadingRunning;
  bool compressionRunning;

  // Global state tracking (for compatibility)
  bool globalRL3PWMState;

  // Motor timing
  unsigned long rl1StartTick;
  unsigned long rl2StartTick;
  unsigned long rl1DelayStartTick;
  unsigned long rl2DelayStartTick;

  // Motor directions
  bool rl1Direction;
  bool rl2Direction;
  bool rl3Direction;

  // PWM values
  uint8_t kneadingPWM;
  uint8_t compressionPWM;

  // Safety timeouts (in ticks)
  static const unsigned long RL1_RL2_TIMEOUT_TICKS = 6000;  // 60s
  static const unsigned long MOTOR_DIR_CHANGE_TICKS = 100;  // 1000ms

  // Component references
  TimerManager* timerManager;
  HardwareSerial* debugSerial;

public:
  // Constructor
  MotorController(TimerManager* timerMgr, HardwareSerial* debugSer = nullptr);

  // Destructor
  ~MotorController();

  // Initialization
  void initialize();

  // RL1 (Recline/Incline) Control
  void onRecline();
  void onIncline();
  void offReclineIncline();
  bool isRL1Running() const;
  unsigned long getRL1StartTick() const;
  void setRL1StartTick(unsigned long tick);

  // RL2 (Forward/Backward) Control
  void onForward();
  void onBackward();
  void offForwardBackward();
  bool isRL2Running() const;
  unsigned long getRL2StartTick() const;
  void setRL2StartTick(unsigned long tick);

  // RL3 (Roll Motor) Control
  void onRollMotor();
  void offRollMotor();
  void setRollDirection(bool direction);
  void runRollUp();
  void runRollDown();
  bool isRL3Running() const;
  bool getRollDirection() const;

  // Global state tracking (for compatibility)
  bool getRL3PWMState() const;
  void setRL3PWMState(bool state);

  // Kneading Motor Control
  void onKneadingMotor();
  void offKneadingMotor();
  void setKneadingPWM(uint8_t pwmValue);
  bool isKneadingRunning() const;
  uint8_t getKneadingPWM() const;

  // Compression Motor Control
  void onCompressionMotor();
  void offCompressionMotor();
  void setCompressionPWM(uint8_t pwmValue);
  void setCompressionPWMByIntensity(uint8_t intensityLevel);
  bool isCompressionRunning() const;
  uint8_t getCompressionPWM() const;

  // Motor Management
  void resetRL1RL2();
  void emergencyStop();
  void checkMotorTimeouts();

  // Motor State Getters
  bool isAnyMotorRunning() const;
  bool isManualMotorRunning() const;  // RL1 or RL2
  bool isAutoMotorRunning() const;    // RL3, Kneading, or Compression

  // Timing Helpers
  unsigned long getMotorRunTime(MotorType motorType) const;
  bool isMotorTimeout(MotorType motorType) const;

  // Safety Functions
  void setMotorTimeout(MotorType motorType, unsigned long timeoutTicks);
  void clearMotorTimeout(MotorType motorType);

private:
  // Helper functions
  void setRL1State(bool running, bool direction = false);
  void setRL2State(bool running, bool direction = false);
  void setRL3State(bool running, bool direction = false);
  void setKneadingState(bool running, uint8_t pwm = 0);
  void setCompressionState(bool running, uint8_t pwm = 0);

  // Direction control helpers
  void setRL1Direction(bool direction);
  void setRL2Direction(bool direction);
  void setRL3Direction(bool direction);

  // PWM control helpers
  void setKneadingPWMInternal(uint8_t pwmValue);
  void setCompressionPWMInternal(uint8_t pwmValue);

  // Safety helpers
  bool checkRL1RL2Timeout() const;
  void handleMotorTimeout(MotorType motorType);
};

#endif  // MOTOR_CONTROLLER_H
