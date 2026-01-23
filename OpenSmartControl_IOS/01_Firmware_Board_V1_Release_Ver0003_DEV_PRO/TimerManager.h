#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <Arduino.h>
#include <HardwareTimer.h>

/**
 * TimerManager Class
 * 
 * Manages all hardware timers and timing operations for the massage chair system.
 * Provides tick-based timing with 10ms base resolution and 1ms precision timer.
 * 
 * Features:
 * - Hardware timer-based timing (TIM2, 10ms tick)
 * - High precision timer (TIM3, 1ms tick)
 * - Tick-based timing system (1 tick = 10ms)
 * - Timer conversion utilities
 * - ISR management
 */
class TimerManager {
private:
  // Hardware timers
  HardwareTimer* mainTimer;       // TIM2 - 10ms base timer
  HardwareTimer* precisionTimer;  // TIM3 - 1ms precision timer

  // Timer tick counters
  volatile unsigned long masterTicks;     // 10ms ticks
  volatile unsigned long precisionTicks;  // 1ms ticks
  volatile unsigned long stepTicks;       // Step timer for program steps

  // Timing constants
  static const unsigned long TIMER_TICK_MS = 10;
  static const unsigned long PRECISION_TICK_MS = 1;

  // Timer state
  bool mainTimerActive;
  bool precisionTimerActive;

  // Debug serial reference
  HardwareSerial* debugSerial;

public:
  // Constructor
  TimerManager(HardwareSerial* debugSer = nullptr);

  // Destructor
  ~TimerManager();

  // Initialization
  void initialize();
  void startMainTimer();
  void startPrecisionTimer();
  void stopMainTimer();
  void stopPrecisionTimer();

  // Timer access
  unsigned long getMasterTicks() const;
  unsigned long getPrecisionTicks() const;
  unsigned long getStepTicks() const;

  // Timer control
  void setMasterTicks(unsigned long ticks);
  void setPrecisionTicks(unsigned long ticks);
  void setStepTicks(unsigned long ticks);
  void incrementMasterTicks();
  void incrementPrecisionTicks();
  void incrementStepTicks();

  // Conversion utilities
  static unsigned long msToTicks(unsigned long ms);
  static unsigned long ticksToMs(unsigned long ticks);
  static unsigned long secondsToTicks(unsigned long seconds);
  static unsigned long ticksToSeconds(unsigned long ticks);

  // Timer status
  bool isMainTimerActive() const;
  bool isPrecisionTimerActive() const;

  // ISR callbacks (called from hardware ISR)
  void onMainTimerISR();
  void onPrecisionTimerISR();

  // Static ISR functions (for hardware timer callbacks)
  static void mainTimerISR();
  static void precisionTimerISR();

  // Static instance for ISR access
  static TimerManager* instance;
};

// Global timer manager instance
extern TimerManager* timerManager;

#endif  // TIMER_MANAGER_H
