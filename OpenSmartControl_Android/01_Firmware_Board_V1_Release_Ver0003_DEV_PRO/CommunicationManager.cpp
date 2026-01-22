#include "CommunicationManager.h"
#include "MotorController.h"
#include "SequenceController.h"
#include "SensorManager.h"

/**
 * Constructor
 */
CommunicationManager::CommunicationManager(TimerManager *timerMgr, HardwareSerial *debug, HardwareSerial *ble)
  : timerManager(timerMgr), debugSerial(debug), bleSerial(ble), dataLen1(0), dataLen2(0), hexIdx1(0), hexIdx2(0), dataReady1(false), dataReady2(false), state1(WAIT_START), state2(WAIT_START), rawBufferIndex(0), autoCmdTimerTick(0), offCmdTimerTick(0), autoCmdTimerActive(false), offCmdTimerActive(false), lastCommand({ 0xFF, 0xFF, 0xFF, 0 }), motorController(nullptr), sequenceController(nullptr), sensorManager(nullptr), manualPriority(false) {
  // Initialize data buffers
  memset(data1, 0, sizeof(data1));
  memset(data2, 0, sizeof(data2));
  memset(hexString1, 0, sizeof(hexString1));
  memset(hexString2, 0, sizeof(hexString2));
  memset(rawBuffer, 0, sizeof(rawBuffer));
}

/**
 * Destructor
 */
CommunicationManager::~CommunicationManager() {
  // Cleanup if needed
}

/**
 * Initialize communication manager
 */
void CommunicationManager::initialize() {
  // Initialize HM10 BREAK pin
  pinMode(HM10_BREAK, OUTPUT);
  digitalWrite(HM10_BREAK, HIGH);  // Normal state (HIGH = not reset)

  serialInit();
  resetDataBuffers();
  resetParseStates();
}

/**
 * Initialize serial communication
 */
void CommunicationManager::serialInit() {
  // Note: Serial interfaces are already initialized in main setup()
  // This function is kept for compatibility but doesn't re-initialize
  if (debugSerial) {
    // Debug serial ready debug disabled
  }
  if (bleSerial) {
    // BLE serial ready debug disabled
  }
}

/**
 * Reset HM10 BLE module
 */
void CommunicationManager::resetHM10() {
  pinMode(HM10_BREAK, OUTPUT);
  digitalWrite(HM10_BREAK, LOW);
  delay(100);
  digitalWrite(HM10_BREAK, HIGH);
  delay(100);
}

/**
 * Set controller references
 */
void CommunicationManager::setControllers(void *motorCtrl, void *seqCtrl, void *sensorCtrl) {
  motorController = motorCtrl;
  sequenceController = seqCtrl;
  sensorManager = sensorCtrl;
}

/**
 * Manual priority management
 */
bool CommunicationManager::getManualPriority() const {
  return manualPriority;
}

void CommunicationManager::setManualPriority(bool value) {
  manualPriority = value;
  if (debugSerial) {
    // Manual Priority debug disabled
  }
}

/**
 * Test UART2 communication
 */
void CommunicationManager::testUART2() {
  if (debugSerial) {
    // Testing UART2 communication debug disabled
  }
}

/**
 * Process incoming BLE data
 */
void CommunicationManager::serial2DataIncome() {
  if (!bleSerial) return;

  // UART2 data processing only (BLE data)
  if (bleSerial->available()) {
    byte receivedByte = bleSerial->read();
    parsePacket(receivedByte, state2, hexString2, hexIdx2, dataReady2);
  }

  // Process UART1 data (if any)
  if (dataReady1) {
    if (debugSerial) {
      // UART1 Packet debug disabled
    }
    processHexString(hexString1, data1, dataLen1);
    printProcessedData(data1, dataLen1);
    dataReady1 = false;
  }

  // Process UART2 data (BLE)
  if (dataReady2) {
    if (debugSerial) {
      // BLE: Processing hex string debug disabled
    }

    processHexString(hexString2, data2, dataLen2);
    printProcessedData(data2, dataLen2);

    // Process as complete 9-byte packet if length is correct
    if (dataLen2 == 7) {  // 7 hex bytes = payload + checksum (no STX/ETX in hex string)

      // Reconstruct complete packet for verification
      uint8_t completePacket[9];
      completePacket[0] = STX;
      for (int i = 0; i < 7; i++) {
        completePacket[i + 1] = data2[i];
      }
      completePacket[8] = ETX;


      processCompleteFrame(completePacket, 9);
    } else {
      if (debugSerial) {
        // BLE: Invalid packet length debug disabled
      }
    }
    dataReady2 = false;
  }
}

/**
 * Parse incoming packet
 */
void CommunicationManager::parsePacket(byte receivedByte, ParseState &currentState,
                                       char hexString[], int &hexIndex, bool &dataReady) {
  // Debug output for BLE data

  switch (currentState) {
    case WAIT_START:
      if (receivedByte == STX) {
        currentState = READ_HEX_STRING;
        hexIndex = 0;
        // if (debugSerial) debugSerial->println("BLE: STX received, starting packet");
      }
      break;

    case READ_HEX_STRING:
      if (receivedByte == ETX) {
        currentState = WAIT_START;
        hexString[hexIndex] = '\0';
        dataReady = true;
        if (debugSerial) {
          // BLE: ETX received debug disabled
        }
      } else if (hexIndex < MAX_HEX_STRING_SIZE - 1) {
        hexString[hexIndex++] = receivedByte;
      }
      break;

    case FIRST_END:
      currentState = WAIT_START;
      break;
  }
}

/**
 * Process complete frame
 */
void CommunicationManager::processCompleteFrame(byte *buf, int len) {
  if (len < PACKET_SIZE) return;

  // Verify packet
  if (verifyCompletePacket(buf, len)) {
    processCommand(buf, len);
  } else {
    if (debugSerial) {
      // Invalid packet received debug disabled
    }
  }
}

/**
 * Process command
 */
void CommunicationManager::processCommand(byte *buf, int len) {
  if (len < PACKET_SIZE) return;

  uint8_t deviceId = buf[1];
  uint8_t sequence = buf[2];
  uint8_t command = buf[3];
  uint8_t data1 = buf[4];
  uint8_t data2 = buf[5];
  uint8_t data3 = buf[6];

  // Only process valid device ID
  if (deviceId != DEVICE_ID) {
    if (debugSerial) {
      // Invalid device ID debug disabled
    }
    return;
  }

  // Check if this is a motor PUSH command (allowed to duplicate for continuous operation)
  bool isMotorPushCommand = ((command == CMD_RECLINE || command == CMD_INCLINE || command == CMD_FORWARD || command == CMD_BACKWARD) && data1 == DATA_ON);

  // Check for duplicate commands
  if (isCommandDuplicate(sequence, command, data1)) {
    // Block duplicate EXCEPT for motor PUSH commands (to ensure continuous operation)
    if (!isMotorPushCommand) {
      if (debugSerial) {
        debugSerial->print(">>> DUPLICATE COMMAND - Ignored (within ");
        debugSerial->print(COMMAND_DUPLICATE_WINDOW_TICKS * 10);
        debugSerial->println("ms window)");
      }
      return;
    } else {
      if (debugSerial) {
        debugSerial->println(">>> MOTOR PUSH DUPLICATE - Allowed (continuous operation)");
      }
    }
  }

  // Update last command
  updateLastCommand(sequence, command, data1);

  // Debug output - Command received
  if (debugSerial) {
    debugSerial->println("\n=== COMMAND RECEIVED ===");

    // Print command name
    debugSerial->print("Command Type: ");
    switch (command) {
      case CMD_AUTO: debugSerial->println("AUTO MODE"); break;
      case CMD_ROLL_MOTOR: debugSerial->println("ROLL MOTOR"); break;
      case CMD_KNEADING: debugSerial->println("KNEADING"); break;
      case CMD_PERCUSSION: debugSerial->println("PERCUSSION"); break;
      case CMD_COMPRESSION: debugSerial->println("COMPRESSION"); break;
      case CMD_COMBINE: debugSerial->println("COMBINE"); break;
      case CMD_INTENSITY_LEVEL: debugSerial->println("INTENSITY LEVEL"); break;
      case CMD_INCLINE: debugSerial->println("INCLINE"); break;
      case CMD_RECLINE: debugSerial->println("RECLINE"); break;
      case CMD_FORWARD: debugSerial->println("FORWARD"); break;
      case CMD_BACKWARD: debugSerial->println("BACKWARD"); break;
      case CMD_DISCONNECT: debugSerial->println("DISCONNECT"); break;
      default: debugSerial->println("UNKNOWN"); break;
    }

    debugSerial->print("Action: ");
    debugSerial->println((data1 == DATA_ON) ? "ON/PUSH" : "OFF/RELEASE");
  }

  // Process command based on type
  switch (command) {
    case CMD_AUTO:
      processAutoCommand(data1);
      break;
    case CMD_ROLL_MOTOR:
      processRollMotorCommand(data1);
      break;
    case CMD_KNEADING:
      processKneadingCommand(data1);
      break;
    case CMD_PERCUSSION:
      processPercussionCommand(data1);
      break;
    case CMD_COMPRESSION:
      processCompressionCommand(data1);
      break;
    case CMD_COMBINE:
      processCombineCommand(data1);
      break;
    case CMD_INTENSITY_LEVEL:
      processIntensityCommand(data1);
      break;
    case CMD_INCLINE:
      processInclineCommand(data1);
      break;
    case CMD_RECLINE:
      processReclineCommand(data1);
      break;
    case CMD_FORWARD:
      processForwardCommand(data1);
      break;
    case CMD_BACKWARD:
      processBackwardCommand(data1);
      break;
    case CMD_DISCONNECT:
      processDisconnectCommand(data1);
      break;
    default:
      if (debugSerial) {
        debugSerial->println(">>> UNKNOWN COMMAND - Ignored");
      }
      break;
  }

  // Debug output - Command processed
  if (debugSerial) debugSerial->println("=== COMMAND PROCESSED ===\n");
}

/**
 * Create packet
 */
void CommunicationManager::createPacket(uint8_t deviceId, uint8_t sequence, uint8_t command,
                                        uint8_t data1, uint8_t data2, uint8_t data3) {
  // Create payload
  uint8_t payload[6] = { deviceId, sequence, command, data1, data2, data3 };

  // Calculate checksum
  uint8_t checksum = calculate_checksum1(payload, 6);

  // Create complete packet
  uint8_t packet[9];
  packet[0] = STX;
  packet[1] = deviceId;
  packet[2] = sequence;
  packet[3] = command;
  packet[4] = data1;
  packet[5] = data2;
  packet[6] = data3;
  packet[7] = checksum;
  packet[8] = ETX;


  // Send packet via BLE
  if (bleSerial) {
    bleSerial->write(packet, 9);
  }
}

/**
 * Calculate checksum
 */
uint8_t CommunicationManager::calculate_checksum1(const uint8_t *data, int len) {
  uint16_t sum = 0;

  // Sum all bytes from index 0 to (len-1) for payload only
  int start_index = 0;
  int end_index = len - 1;

  // For 6-byte payload: sum bytes 0-5
  if (len == 6) {
    end_index = 5;
  }

  for (int i = start_index; i <= end_index; i++) {
    sum += data[i];
  }

  // Add carry (Internet checksum style)
  while (sum >> 8) {
    sum = (sum & 0xFF) + (sum >> 8);
  }

  // One's complement + 0x10 offset
  uint8_t result = ((~sum) + 0x10) & 0xFF;

  return result;
}

/**
 * Verify complete packet
 */
bool CommunicationManager::verifyCompletePacket(uint8_t *packet, int length) {
  // Quick checks
  if (length != PACKET_SIZE) {
    if (debugSerial) {
      debugSerial->print("!!! Invalid length: ");
      debugSerial->println(length);
    }
    return false;
  }

  if (packet[0] != STX || packet[8] != ETX) {
    if (debugSerial) debugSerial->println("!!! Invalid STX/ETX");
    return false;
  }

  // Extract payload and verify checksum
  uint8_t payload[PAYLOAD_SIZE];
  for (int i = 0; i < PAYLOAD_SIZE; i++) {
    payload[i] = packet[i + 1];
  }

  uint8_t calculatedChecksum = calculate_checksum1(payload, PAYLOAD_SIZE);
  uint8_t receivedChecksum = packet[7];

  if (calculatedChecksum != receivedChecksum) {
    if (debugSerial) {
      debugSerial->print("!!! Checksum mismatch: Calc=0x");
      if (calculatedChecksum < 0x10) debugSerial->print("0");
      debugSerial->print(calculatedChecksum, HEX);
      debugSerial->print(" Recv=0x");
      if (receivedChecksum < 0x10) debugSerial->print("0");
      debugSerial->println(receivedChecksum, HEX);
    }
    return false;
  }

  // if (debugSerial) debugSerial->println("✓ Packet valid");
  return true;
}

/**
 * Check command counters
 */
void CommunicationManager::checkCommandCounters() {
  unsigned long currentTick = timerManager->getMasterTicks();

  // Check auto command timer
  if (autoCmdTimerActive && (currentTick - autoCmdTimerTick) >= COMMAND_COUNTER_TIMEOUT_TICKS) {
    stopAutoCmdTimer();
    if (debugSerial) {
      debugSerial->println("Auto command timer expired");
    }
  }

  // Check off command timer
  if (offCmdTimerActive && (currentTick - offCmdTimerTick) >= COMMAND_COUNTER_TIMEOUT_TICKS) {
    stopOffCmdTimer();
    if (debugSerial) {
      debugSerial->println("Off command timer expired");
    }
  }
}

/**
 * Check if command is duplicate
 */
bool CommunicationManager::isCommandDuplicate(uint8_t sequence, uint8_t command, uint8_t data1) {
  unsigned long currentTick = timerManager->getMasterTicks();

  if (debugSerial) {
    // debugSerial->print("DEDUP CHECK: Seq=");
    debugSerial->print(sequence, HEX);
    debugSerial->print(" Cmd=");
    debugSerial->print(command, HEX);
    debugSerial->print(" Data1=");
    debugSerial->print(data1, HEX);
    debugSerial->print(" | Last: Seq=");
    debugSerial->print(lastCommand.sequence, HEX);
    debugSerial->print(" Cmd=");
    debugSerial->print(lastCommand.command, HEX);
    debugSerial->print(" Data1=");
    debugSerial->print(lastCommand.data1, HEX);
    debugSerial->print(" TimeDiff=");
    debugSerial->print(currentTick - lastCommand.timestamp);
    debugSerial->print(" Window=");
    debugSerial->println(COMMAND_DUPLICATE_WINDOW_TICKS);
  }

  if (lastCommand.sequence == sequence && lastCommand.command == command && lastCommand.data1 == data1 && (currentTick - lastCommand.timestamp) < COMMAND_DUPLICATE_WINDOW_TICKS) {
    return true;
  }

  return false;
}

/**
 * Update last command
 */
void CommunicationManager::updateLastCommand(uint8_t sequence, uint8_t command, uint8_t data1) {
  lastCommand.sequence = sequence;
  lastCommand.command = command;
  lastCommand.data1 = data1;
  lastCommand.timestamp = timerManager->getMasterTicks();
}

/**
 * Command counter management
 */
void CommunicationManager::startAutoCmdTimer() {
  autoCmdTimerActive = true;
  autoCmdTimerTick = timerManager->getMasterTicks();
}

void CommunicationManager::startOffCmdTimer() {
  offCmdTimerActive = true;
  offCmdTimerTick = timerManager->getMasterTicks();
}

void CommunicationManager::stopAutoCmdTimer() {
  autoCmdTimerActive = false;
  autoCmdTimerTick = 0;
}

void CommunicationManager::stopOffCmdTimer() {
  offCmdTimerActive = false;
  offCmdTimerTick = 0;
}

bool CommunicationManager::isAutoCmdTimerActive() const {
  return autoCmdTimerActive;
}

bool CommunicationManager::isOffCmdTimerActive() const {
  return offCmdTimerActive;
}

/**
 * Utility functions
 */
int CommunicationManager::hexStringToBytes(const char *hexStr, byte *outBytes) {
  int len = strlen(hexStr);
  int byteCount = 0;

  for (int i = 0; i < len; i += 2) {
    if (i + 1 < len) {
      outBytes[byteCount++] = (hexCharToByte(hexStr[i]) << 4) | hexCharToByte(hexStr[i + 1]);
    }
  }

  return byteCount;
}

byte CommunicationManager::hexCharToByte(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

void CommunicationManager::processHexString(char hexString[], byte data[], int &dataLength) {
  dataLength = hexStringToBytes(hexString, data);
}

void CommunicationManager::printProcessedData(byte data[], int length) {
  // Debug output removed to reduce spam
}

/**
 * Private helper functions
 */
void CommunicationManager::resetDataBuffers() {
  memset(data1, 0, sizeof(data1));
  memset(data2, 0, sizeof(data2));
  memset(hexString1, 0, sizeof(hexString1));
  memset(hexString2, 0, sizeof(hexString2));
  memset(rawBuffer, 0, sizeof(rawBuffer));

  dataLen1 = 0;
  dataLen2 = 0;
  hexIdx1 = 0;
  hexIdx2 = 0;
  dataReady1 = false;
  dataReady2 = false;
  rawBufferIndex = 0;
}

void CommunicationManager::resetParseStates() {
  state1 = WAIT_START;
  state2 = WAIT_START;
}

/**
 * Command processing functions (placeholders - would need integration with other classes)
 */
void CommunicationManager::processAutoCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    // if (debugSerial) debugSerial->println(">>> AUTO RUN - Starting DEFAULT program");
    if (sequenceController) {
      // Debug: Check system status (disabled to save FLASH)
      // if (debugSerial) {
      //   debugSerial->print("AUTO DEBUG: homeRun=");
      //   debugSerial->print(((SequenceController *)sequenceController)->getHomeRun() ? "TRUE" : "FALSE");
      //   debugSerial->print(", manualPriority=");
      //   debugSerial->print(getManualPriority() ? "TRUE" : "FALSE");
      //   debugSerial->print(", modeAuto=");
      //   debugSerial->print(((SequenceController *)sequenceController)->getModeAuto() ? "TRUE" : "FALSE");
      //   debugSerial->println();
      // }

      // Check if system is homed first
      if (!((SequenceController *)sequenceController)->getHomeRun()) {
        // if (debugSerial) debugSerial->println("ERROR: Cannot start AUTO - Not homed! Please run GO HOME first");
        return;
      }

      // Clear all other modes first (like original code)
      ((SequenceController *)sequenceController)->setKneadingMode(false);
      ((SequenceController *)sequenceController)->setCompressionMode(false);
      ((SequenceController *)sequenceController)->setPercussionMode(false);
      ((SequenceController *)sequenceController)->setCombineMode(false);

      // Set autodefaultMode (this is CMD_AUTO's unique mode)
      ((SequenceController *)sequenceController)->setAutodefaultMode(true);
      if (debugSerial) debugSerial->println("DEBUG: autodefaultMode set to TRUE");

      // AUTO_DEFAULT: Roll motor is ALWAYS ON (cannot be toggled)
      if (debugSerial) debugSerial->println("    Roll Motor: ON (DEFAULT - cannot be disabled in this mode)");

      // Start auto mode with 20-minute timer
      ((SequenceController *)sequenceController)->startAutoMode();

      if (debugSerial) debugSerial->println("AUTO: Mode started successfully");
    }
  } else if (data1 == DATA_OFF) {
    if (debugSerial) debugSerial->println(">>> AUTO STOP - Stopping all programs");
    if (sequenceController) {
      if (debugSerial) debugSerial->println("DEBUG: sequenceController is valid, calling stopAutoMode()");
      ((SequenceController *)sequenceController)->stopAutoMode();
      if (debugSerial) debugSerial->println("DEBUG: stopAutoMode() call completed");
    } else {
      if (debugSerial) debugSerial->println("ERROR: sequenceController is NULL!");
    }
  }
}

void CommunicationManager::processRollMotorCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    if (debugSerial) debugSerial->println(">>> ROLL MOTOR ON");
    
    // Set manual priority to override auto programs
    setManualPriority(true);
    if (debugSerial) debugSerial->println("ROLL: Manual priority set to TRUE");
    
    // Reset flag to allow auto programs to control roll motor again
    if (sequenceController) {
      ((SequenceController *)sequenceController)->setRollMotorUserDisabled(false);
      if (debugSerial) debugSerial->println("ROLL: Roll motor enabled by user - auto programs can control it again");
    }
    
    if (motorController) {
      ((MotorController *)motorController)->onRollMotor();
    }
  } else if (data1 == DATA_OFF) {
    if (debugSerial) debugSerial->println("<<< ROLL MOTOR OFF");
    
    // Clear manual priority to allow auto programs to resume
    setManualPriority(false);
    if (debugSerial) debugSerial->println("ROLL: Manual priority set to FALSE");
    
    // Set flag to prevent auto programs from restarting roll motor
    if (sequenceController) {
      ((SequenceController *)sequenceController)->setRollMotorUserDisabled(true);
      if (debugSerial) debugSerial->println("ROLL: Roll motor disabled by user - auto programs will not restart it");
    }
    
    if (motorController) {
      ((MotorController *)motorController)->offRollMotor();
    }
  }
}

void CommunicationManager::processKneadingCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    // if (debugSerial) debugSerial->println(">>> KNEADING MODE ON");
    
    if (sequenceController) {
      // Check if system is homed first
      if (!((SequenceController*)sequenceController)->getHomeRun()) {
        // if (debugSerial) debugSerial->println("ERROR: Cannot start KNEADING - Not homed! Please run GO HOME first");
        return;
      }
      
      // Clear all other modes first
      ((SequenceController*)sequenceController)->setAutodefaultMode(false);
      ((SequenceController*)sequenceController)->setCompressionMode(false);
      ((SequenceController*)sequenceController)->setPercussionMode(false);
      ((SequenceController*)sequenceController)->setCombineMode(false);
      
      // Reset roll motor user disabled flag - roll motor enabled by default in KNEADING mode
      ((SequenceController*)sequenceController)->setRollMotorUserDisabled(false);
      
      // Set kneading mode
      ((SequenceController*)sequenceController)->setKneadingMode(true);
      
      // Start auto mode if not already running
      if (!((SequenceController*)sequenceController)->getModeAuto()) {
        ((SequenceController*)sequenceController)->startAutoMode();
        // if (debugSerial) debugSerial->println("KNEADING: Auto mode started");
      }
      
      // if (debugSerial) debugSerial->println("KNEADING: Mode enabled successfully");
    }
  } else if (data1 == DATA_OFF) {
    // if (debugSerial) debugSerial->println("<<< KNEADING MODE OFF");
    
    if (sequenceController) {
      // Stop kneading mode
      ((SequenceController*)sequenceController)->setKneadingMode(false);
      
      // If no other modes are active, stop auto mode
      if (!((SequenceController*)sequenceController)->getAutodefaultMode() &&
          !((SequenceController*)sequenceController)->getCompressionMode() &&
          !((SequenceController*)sequenceController)->getPercussionMode() &&
          !((SequenceController*)sequenceController)->getCombineMode()) {
        ((SequenceController*)sequenceController)->stopAutoMode();
        if (debugSerial) debugSerial->println("KNEADING: Auto mode stopped - no active programs");
      }
      
      if (debugSerial) debugSerial->println("KNEADING: Mode disabled");
    }
  }
}

void CommunicationManager::processPercussionCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    // if (debugSerial) debugSerial->println(">>> PERCUSSION MODE ON");
    
    if (sequenceController) {
      // Check if system is homed first
      if (!((SequenceController*)sequenceController)->getHomeRun()) {
        // if (debugSerial) debugSerial->println("ERROR: Cannot start PERCUSSION - Not homed! Please run GO HOME first");
        return;
      }
      
      // Clear all other modes first
      ((SequenceController*)sequenceController)->setAutodefaultMode(false);
      ((SequenceController*)sequenceController)->setKneadingMode(false);
      ((SequenceController*)sequenceController)->setCompressionMode(false);
      ((SequenceController*)sequenceController)->setCombineMode(false);
      
      // Reset roll motor user disabled flag - roll motor enabled by default in PERCUSSION mode
      ((SequenceController*)sequenceController)->setRollMotorUserDisabled(false);
      
      // Set percussion mode
      ((SequenceController*)sequenceController)->setPercussionMode(true);
      
      // Start auto mode if not already running
      if (!((SequenceController*)sequenceController)->getModeAuto()) {
        ((SequenceController*)sequenceController)->startAutoMode();
        // if (debugSerial) debugSerial->println("PERCUSSION: Auto mode started");
      }
      
      // if (debugSerial) debugSerial->println("PERCUSSION: Mode enabled successfully");
    }
  } else if (data1 == DATA_OFF) {
    // if (debugSerial) debugSerial->println("<<< PERCUSSION MODE OFF");
    
    if (sequenceController) {
      // Stop percussion mode
      ((SequenceController*)sequenceController)->setPercussionMode(false);
      
      // If no other modes are active, stop auto mode
      if (!((SequenceController*)sequenceController)->getAutodefaultMode() &&
          !((SequenceController*)sequenceController)->getKneadingMode() &&
          !((SequenceController*)sequenceController)->getCompressionMode() &&
          !((SequenceController*)sequenceController)->getCombineMode()) {
        ((SequenceController*)sequenceController)->stopAutoMode();
        if (debugSerial) debugSerial->println("PERCUSSION: Auto mode stopped - no active programs");
      }
      
      if (debugSerial) debugSerial->println("PERCUSSION: Mode disabled");
    }
  }
}

void CommunicationManager::processCompressionCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    // if (debugSerial) debugSerial->println(">>> COMPRESSION MODE ON");
    
    if (sequenceController) {
      // Check if system is homed first
      if (!((SequenceController*)sequenceController)->getHomeRun()) {
        // if (debugSerial) debugSerial->println("ERROR: Cannot start COMPRESSION - Not homed! Please run GO HOME first");
        return;
      }
      
      // Clear all other modes first
      ((SequenceController*)sequenceController)->setAutodefaultMode(false);
      ((SequenceController*)sequenceController)->setKneadingMode(false);
      ((SequenceController*)sequenceController)->setPercussionMode(false);
      ((SequenceController*)sequenceController)->setCombineMode(false);
      
      // Reset roll motor user disabled flag - roll motor enabled by default in COMPRESSION mode
      ((SequenceController*)sequenceController)->setRollMotorUserDisabled(false);
      
      // Set compression mode
      ((SequenceController*)sequenceController)->setCompressionMode(true);
      
      // Start auto mode if not already running
      if (!((SequenceController*)sequenceController)->getModeAuto()) {
        ((SequenceController*)sequenceController)->startAutoMode();
        // if (debugSerial) debugSerial->println("COMPRESSION: Auto mode started");
      }
      
      // if (debugSerial) debugSerial->println("COMPRESSION: Mode enabled successfully");
    }
  } else if (data1 == DATA_OFF) {
    // if (debugSerial) debugSerial->println("<<< COMPRESSION MODE OFF");
    
    if (sequenceController) {
      // Stop compression mode
      ((SequenceController*)sequenceController)->setCompressionMode(false);
      
      // If no other modes are active, stop auto mode
      if (!((SequenceController*)sequenceController)->getAutodefaultMode() &&
          !((SequenceController*)sequenceController)->getKneadingMode() &&
          !((SequenceController*)sequenceController)->getPercussionMode() &&
          !((SequenceController*)sequenceController)->getCombineMode()) {
        ((SequenceController*)sequenceController)->stopAutoMode();
        if (debugSerial) debugSerial->println("COMPRESSION: Auto mode stopped - no active programs");
      }
      
      if (debugSerial) debugSerial->println("COMPRESSION: Mode disabled");
    }
  }
}

void CommunicationManager::processCombineCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    // if (debugSerial) debugSerial->println(">>> COMBINE MODE ON");
    
    if (sequenceController) {
      // Check if system is homed first
      if (!((SequenceController*)sequenceController)->getHomeRun()) {
        // if (debugSerial) debugSerial->println("ERROR: Cannot start COMBINE - Not homed! Please run GO HOME first");
        return;
      }
      
      // Clear all other modes first
      ((SequenceController*)sequenceController)->setAutodefaultMode(false);
      ((SequenceController*)sequenceController)->setKneadingMode(false);
      ((SequenceController*)sequenceController)->setCompressionMode(false);
      ((SequenceController*)sequenceController)->setPercussionMode(false);
      
      // Reset roll motor user disabled flag - roll motor enabled by default in COMBINE mode
      ((SequenceController*)sequenceController)->setRollMotorUserDisabled(false);
      
      // Set combine mode
      ((SequenceController*)sequenceController)->setCombineMode(true);
      
      // Start auto mode if not already running
      if (!((SequenceController*)sequenceController)->getModeAuto()) {
        ((SequenceController*)sequenceController)->startAutoMode();
        // if (debugSerial) debugSerial->println("COMBINE: Auto mode started");
      }
    }
  } else if (data1 == DATA_OFF) {
    // if (debugSerial) debugSerial->println("<<< COMBINE MODE OFF");
    
    if (sequenceController) {
      ((SequenceController*)sequenceController)->setCombineMode(false);
      ((SequenceController*)sequenceController)->stopAutoMode();
      // if (debugSerial) debugSerial->println("COMBINE: Auto mode stopped");
    }
  }
}

void CommunicationManager::processIntensityCommand(uint8_t data1) {
  if (debugSerial) {
    debugSerial->print(">>> INTENSITY LEVEL: ");
    if (data1 == INTENSITY_HIGH) {
      debugSerial->println("HIGH (0x20)");
    } else if (data1 == INTENSITY_LOW) {
      debugSerial->println("LOW (0x00)");
    } else if (data1 == DATA_OFF) {
      debugSerial->println("OFF (0x00)");
    } else {
      debugSerial->print("CUSTOM (0x");
      debugSerial->print(data1, HEX);
      debugSerial->println(")");
    }
  }
  
  // Set intensity level in sequence controller
  if (sequenceController) {
    uint8_t intensityValue;
    if (data1 == INTENSITY_HIGH) {
      intensityValue = 254;  // HIGH intensity (PWM=254)
    } else if (data1 == INTENSITY_LOW) {
      intensityValue = 160;  // LOW intensity (PWM=160)
    } else if (data1 == DATA_OFF) {
      // Use setIntensityOff() for proper OFF handling with reason
      ((SequenceController*)sequenceController)->setIntensityOff("Remote OFF command");
      return;  // Exit early since setIntensityOff() handles everything
    } else {
      // Custom intensity level (0-255)
      intensityValue = data1;
    }
    
    ((SequenceController*)sequenceController)->setIntensityLevel(intensityValue);
    if (debugSerial) {
      debugSerial->print("INTENSITY: Set to ");
      debugSerial->println(intensityValue);
    }
  }
}

void CommunicationManager::processInclineCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    if (debugSerial) debugSerial->println(">>> INCLINE PUSH");
    if (motorController) {
      // Set manual priority (pause auto mode)
      setManualPriority(true);
      // Stop roll motor first (safety)
      ((MotorController *)motorController)->offRollMotor();
      // Start incline motor (will continue running until release)
      ((MotorController *)motorController)->onIncline();
    }
  } else if (data1 == DATA_OFF) {
    if (debugSerial) debugSerial->println("<<< INCLINE RELEASE - Motor stopped");
    if (motorController) {
      // Stop incline motor immediately
      ((MotorController *)motorController)->offReclineIncline();
      // Clear manual priority (resume auto mode)
      setManualPriority(false);
    }
  }
}

void CommunicationManager::processReclineCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    if (debugSerial) debugSerial->println(">>> RECLINE PUSH");
    if (motorController) {
      // Set manual priority (pause auto mode)
      setManualPriority(true);
      // Stop roll motor first (safety)
      ((MotorController *)motorController)->offRollMotor();
      // Start recline motor (will continue running until release)
      ((MotorController *)motorController)->onRecline();
    }
  } else if (data1 == DATA_OFF) {
    if (debugSerial) debugSerial->println("<<< RECLINE RELEASE - Motor stopped");
    if (motorController) {
      // Stop recline motor immediately
      ((MotorController *)motorController)->offReclineIncline();
      // Clear manual priority (resume auto mode)
      setManualPriority(false);
    }
  }
}

void CommunicationManager::processForwardCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    if (debugSerial) debugSerial->println(">>> FORWARD PUSH");
    if (motorController) {
      // Set manual priority (pause auto mode)
      setManualPriority(true);
      // Stop roll motor first (safety)
      ((MotorController *)motorController)->offRollMotor();
      // Start forward motor (will continue running until release)
      ((MotorController *)motorController)->onForward();
    }
  } else if (data1 == DATA_OFF) {
    if (debugSerial) debugSerial->println("<<< FORWARD RELEASE - Motor stopped");
    if (motorController) {
      // Stop forward motor immediately
      ((MotorController *)motorController)->offForwardBackward();
      // Clear manual priority (resume auto mode)
      setManualPriority(false);
    }
  }
}

void CommunicationManager::processBackwardCommand(uint8_t data1) {
  if (data1 == DATA_ON) {
    if (debugSerial) debugSerial->println(">>> BACKWARD PUSH");
    if (motorController) {
      // Set manual priority (pause auto mode)
      setManualPriority(true);
      // Stop roll motor first (safety)
      ((MotorController *)motorController)->offRollMotor();
      // Start backward motor (will continue running until release)
      ((MotorController *)motorController)->onBackward();
    }
  } else if (data1 == DATA_OFF) {
    if (debugSerial) debugSerial->println("<<< BACKWARD RELEASE - Motor stopped");
    if (motorController) {
      // Stop backward motor immediately
      ((MotorController *)motorController)->offForwardBackward();
      // Clear manual priority (resume auto mode)
      setManualPriority(false);
    }
  }
}

void CommunicationManager::processDisconnectCommand(uint8_t data1) {
  if (data1 == DATA_OFF) {
    // ✨ DISCONNECT = AUTO MODE OFF (same behavior)
    if (debugSerial) debugSerial->println(">>> AUTO STOP - Stopping all programs");
    
    // Stop all position motors first (safety)
    if (motorController) {
      ((MotorController *)motorController)->offForwardBackward();
      ((MotorController *)motorController)->offReclineIncline();
    }
    
    // Stop AUTO mode (same as AUTO OFF command)
    if (sequenceController) {
      if (debugSerial) debugSerial->println("DEBUG: sequenceController is valid, calling stopAutoMode()");
      ((SequenceController *)sequenceController)->stopAutoMode();
      if (debugSerial) debugSerial->println("DEBUG: stopAutoMode() call completed");
    } else {
      if (debugSerial) debugSerial->println("ERROR: sequenceController is NULL!");
    }
    
    // Reset BLE module (only for DISCONNECT, not for AUTO OFF)
    resetHM10();
    if (debugSerial) debugSerial->println("  - BLE module reset");
    
  } else if (data1 == DATA_ON) {
    if (debugSerial) debugSerial->println(">>> CONNECTED");
    // Connection established - no action needed
  }
}
