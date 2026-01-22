/*
 * Firmware Board V1 Release Ver0003 - REFACTORED CLASS-BASED ARCHITECTURE
 * 
 * Features:
 * - Object-oriented design with separate classes for each subsystem
 * - Hardware timer-based timing (TIM2, 10ms tick)
 * - Non-blocking motor control (no delay())
 * - Watchdog timer protection (3s)
 * - System health monitoring
 * - Optimized home sequence
 * - BLE communication via HM10
 * - Modular architecture for better maintainability
 * 
 * Class Architecture:
 * - MassageController: Main orchestrator class
 * - TimerManager: Hardware timer management
 * - MotorController: Motor control operations
 * - SensorManager: Sensor handling and debouncing
 * - CommunicationManager: BLE and serial communication
 * - SafetyManager: Watchdog and safety mechanisms
 * - SequenceController: Home and auto sequences
 * 
 * Timer Architecture:
 * - TIM2 ISR runs every 10ms (100Hz)
 * - Master tick counter for all timing operations
 * - Sensor debouncing in ISR (30ms)
 * - Sensor confirmation delay (500ms anti-noise)
 * - All delays converted to tick-based counting
 * 
 * Performance Improvements:
 * - Hardware timer instead of millis() - more precise
 * - Inline functions for frequently called operations
 * - Reduced serial output overhead
 * - Efficient command processing with switch-case
 * - Memory optimized structures
 * - Tick-based timing: 1 tick = 10ms
 */

#include "PinDefinitions.h"
#include "MassageController.h"

///////////////////////////////////////////////// GLOBAL OBJECTS /////////////////////////////////////////////////
// Serial interfaces
HardwareSerial mySerial(DEBUG_RX_PIN, DEBUG_TX_PIN);    // Debug UART - 115200 baud
HardwareSerial mySerial2(BLE_RX_PIN, BLE_TX_PIN);       // BLE UART - 9600 baud

// Main massage controller instance
MassageController* massageController = nullptr;

///////////////////////////////////////////////// SAFETY FUNCTIONS /////////////////////////////////////////////////
void initializeIO() {
  // Initialize all motor control pins as OUTPUT and set to LOW
  pinMode(RL1_PWM_PIN, OUTPUT);
  pinMode(RL1_DIR_PIN, OUTPUT);
  pinMode(RL2_PWM_PIN, OUTPUT);
  pinMode(RL2_DIR_PIN, OUTPUT);
  pinMode(RL3_PWM_PIN, OUTPUT);
  pinMode(RL3_DIR_PIN, OUTPUT);
  pinMode(FETT_PWM_PIN, OUTPUT);  // Kneading motor (digital ON/OFF)
  pinMode(FETK_PWM_PIN, OUTPUT);  // Compression motor (PWM capable for intensity control)
  
  // Set PWM resolution for compression motor (STM32 Arduino supports 8-bit or 10-bit)
  // analogWriteResolution(FETK_PWM_PIN, 8);  // Set 8-bit PWM (0-255) for compression motor
  // Note: If analogWriteResolution() doesn't exist, STM32 Arduino defaults to 8-bit PWM (0-255)
  
  // Set all motor pins to LOW (OFF state)
  digitalWrite(RL1_PWM_PIN, LOW);
  digitalWrite(RL1_DIR_PIN, LOW);
  digitalWrite(RL2_PWM_PIN, LOW);
  digitalWrite(RL2_DIR_PIN, LOW);
  digitalWrite(RL3_PWM_PIN, LOW);
  digitalWrite(RL3_DIR_PIN, HIGH);  // Default direction: DOWN to UP
  digitalWrite(FETT_PWM_PIN, LOW);
  analogWrite(FETK_PWM_PIN, 0);  // Compression motor OFF (PWM = 0)
  // mySerial.println("initializeIO: RL3_DIR_PIN set to HIGH (DOWN to UP direction)");
  
  // Initialize sensor pins as INPUT_PULLUP
  pinMode(LMT_UP_PIN, INPUT_PULLUP);
  pinMode(LMT_DOWN_PIN, INPUT_PULLUP);
}

void emergencyStopAllMotors() {
  // CRITICAL: Turn off ALL motors immediately
  digitalWrite(RL1_PWM_PIN, LOW);  // Recline/Incline motor OFF
  digitalWrite(RL1_DIR_PIN, LOW);
  digitalWrite(RL2_PWM_PIN, LOW);  // Forward/Backward motor OFF
  digitalWrite(RL2_DIR_PIN, LOW);
  digitalWrite(RL3_PWM_PIN, LOW);  // Roll motor OFF
  digitalWrite(RL3_DIR_PIN, HIGH); // Keep default direction: DOWN to UP
  digitalWrite(FETT_PWM_PIN, LOW); // Kneading motor OFF
  digitalWrite(FETK_PWM_PIN, LOW); // Compression motor OFF
  
  // mySerial.println("emergencyStopAllMotors: RL3_DIR_PIN kept HIGH (DOWN to UP direction)");
}

///////////////////////////////////////////////// SETUP /////////////////////////////////////////////////
void setup() {
  // CRITICAL SAFETY: Initialize I/O and turn OFF all motors FIRST
  initializeIO();
  emergencyStopAllMotors();
  
  // Initialize serial communication
  mySerial.begin(115200);
  mySerial2.begin(9600);  // Initialize BLE UART
  delay(100);  // Wait for serial to stabilize
  
  mySerial.println("MASSAGE CONTROLLER INITIALIZATION");
  // mySerial.println("SAFETY: All I/O initialized and motors OFF");
  
  // Create main controller instance
  massageController = new MassageController();
  
  // Configure serial interfaces
  massageController->setDebugSerial(&mySerial);
  massageController->setBleSerial(&mySerial2);
  
  // Initialize the system
  // mySerial.println("DEBUG: About to initialize MassageController...");
  massageController->initialize();
  // mySerial.println("DEBUG: MassageController initialized");
  
  // Start the system
  // mySerial.println("DEBUG: About to start MassageController...");
  massageController->start();
  // mySerial.println("DEBUG: MassageController started");
  
  mySerial.println("System started successfully");
  // mySerial.println("Ready to run GO HOME sequence...");
  
  // Print initial system status (disabled to save FLASH)
  // massageController->printSystemStatus();
  // massageController->printSubsystemStatus();
}

///////////////////////////////////////////////// MAIN LOOP - REFACTORED /////////////////////////////////////////////////
void loop() {
  // Run the main massage controller
  if (massageController) {
    massageController->run();
  } else {
    // Fallback error handling
    mySerial.println("ERROR: MassageController not initialized!");
    delay(1000);
  }
}
