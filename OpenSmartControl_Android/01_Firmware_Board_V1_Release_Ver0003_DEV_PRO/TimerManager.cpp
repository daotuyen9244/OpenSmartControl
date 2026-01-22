#include "TimerManager.h"

// Static instance for ISR access
TimerManager* TimerManager::instance = nullptr;

/**
 * Constructor
 */
TimerManager::TimerManager(HardwareSerial* debugSer)
  : mainTimer(nullptr), precisionTimer(nullptr), debugSerial(debugSer), masterTicks(0), precisionTicks(0), stepTicks(0), mainTimerActive(false), precisionTimerActive(false) {
  instance = this;
}

/**
 * Destructor
 */
TimerManager::~TimerManager() {
  if (mainTimer) {
    delete mainTimer;
    mainTimer = nullptr;
  }
  if (precisionTimer) {
    delete precisionTimer;
    precisionTimer = nullptr;
  }
  instance = nullptr;
}

/**
 * Initialize all timers
 */
void TimerManager::initialize() {
  // Initialize main timer (TIM2) - 10ms base timer
  mainTimer = new HardwareTimer(TIM2);
  mainTimer->setOverflow(10000, MICROSEC_FORMAT);  // 10ms
  mainTimer->attachInterrupt(mainTimerISR);

  // Initialize precision timer (TIM3) - 1ms precision timer
  precisionTimer = new HardwareTimer(TIM3);
  precisionTimer->setOverflow(1000, MICROSEC_FORMAT);  // 1ms
  precisionTimer->attachInterrupt(precisionTimerISR);

  // Reset counters
  masterTicks = 0;
  precisionTicks = 0;
  stepTicks = 0;

  mainTimerActive = false;
  precisionTimerActive = false;
}

/**
 * Start main timer (10ms)
 */
void TimerManager::startMainTimer() {
  if (mainTimer && !mainTimerActive) {
    mainTimer->resume();
    mainTimerActive = true;
  }
}

/**
 * Start precision timer (1ms)
 */
void TimerManager::startPrecisionTimer() {
  if (precisionTimer && !precisionTimerActive) {
    precisionTimer->resume();
    precisionTimerActive = true;
  }
}

/**
 * Stop main timer
 */
void TimerManager::stopMainTimer() {
  if (mainTimer && mainTimerActive) {
    mainTimer->pause();
    mainTimerActive = false;
  }
}

/**
 * Stop precision timer
 */
void TimerManager::stopPrecisionTimer() {
  if (precisionTimer && precisionTimerActive) {
    precisionTimer->pause();
    precisionTimerActive = false;
  }
}

/**
 * Get master ticks (10ms resolution)
 */
unsigned long TimerManager::getMasterTicks() const {
  return masterTicks;
}

/**
 * Get precision ticks (1ms resolution)
 */
unsigned long TimerManager::getPrecisionTicks() const {
  return precisionTicks;
}

/**
 * Get step ticks
 */
unsigned long TimerManager::getStepTicks() const {
  return stepTicks;
}

/**
 * Set master ticks
 */
void TimerManager::setMasterTicks(unsigned long ticks) {
  masterTicks = ticks;
}

/**
 * Set precision ticks
 */
void TimerManager::setPrecisionTicks(unsigned long ticks) {
  precisionTicks = ticks;
}

/**
 * Set step ticks
 */
void TimerManager::setStepTicks(unsigned long ticks) {
  stepTicks = ticks;
}

/**
 * Increment master ticks
 */
void TimerManager::incrementMasterTicks() {
  masterTicks++;
}

/**
 * Increment precision ticks
 */
void TimerManager::incrementPrecisionTicks() {
  precisionTicks++;
}

/**
 * Increment step ticks
 */
void TimerManager::incrementStepTicks() {
  stepTicks++;
}

/**
 * Convert milliseconds to timer ticks
 */
unsigned long TimerManager::msToTicks(unsigned long ms) {
  return ms / TIMER_TICK_MS;
}

/**
 * Convert timer ticks to milliseconds
 */
unsigned long TimerManager::ticksToMs(unsigned long ticks) {
  return ticks * TIMER_TICK_MS;
}

/**
 * Convert seconds to timer ticks
 */
unsigned long TimerManager::secondsToTicks(unsigned long seconds) {
  return msToTicks(seconds * 1000);
}

/**
 * Convert timer ticks to seconds
 */
unsigned long TimerManager::ticksToSeconds(unsigned long ticks) {
  return ticksToMs(ticks) / 1000;
}

/**
 * Check if main timer is active
 */
bool TimerManager::isMainTimerActive() const {
  return mainTimerActive;
}

/**
 * Check if precision timer is active
 */
bool TimerManager::isPrecisionTimerActive() const {
  return precisionTimerActive;
}

/**
 * Main timer ISR callback
 */
void TimerManager::onMainTimerISR() {
  incrementMasterTicks();
}

/**
 * Precision timer ISR callback
 */
void TimerManager::onPrecisionTimerISR() {
  incrementPrecisionTicks();
}

/**
 * Static main timer ISR function
 */
void TimerManager::mainTimerISR() {
  if (instance) {
    instance->onMainTimerISR();
  }
}

/**
 * Static precision timer ISR function
 */
void TimerManager::precisionTimerISR() {
  if (instance) {
    instance->onPrecisionTimerISR();
  }
}

// Global timer manager instance
TimerManager* timerManager = nullptr;
