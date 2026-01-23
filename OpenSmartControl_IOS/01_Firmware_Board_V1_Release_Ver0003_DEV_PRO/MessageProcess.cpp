#include "MessageProcess.h"
#include "Massage_v1_hardware.h"  // For timing constants

// Data storage
byte data1[MAX_DATA_SIZE], data2[MAX_DATA_SIZE];
char hexString1[MAX_HEX_STRING_SIZE], hexString2[MAX_HEX_STRING_SIZE];
int dataLen1 = 0, dataLen2 = 0, hexIdx1 = 0, hexIdx2 = 0;
bool dataReady1 = false, dataReady2 = false;
volatile byte offCmdCounter = 0, autoCmdCounter = 0;
// Parse states
ParseState state1 = WAIT_START, state2 = WAIT_START;

byte rawBuffer[50];
int rawBufferIndex = 0;

// Command counter timers - using timer ticks
unsigned long autoCmdTimerTick = 0;
unsigned long offCmdTimerTick = 0;
bool autoCmdTimerActive = false;
bool offCmdTimerActive = false;

// Command deduplication - Prevent executing same command multiple times
// Remote gửi nhiều lần cùng 1 lệnh để đảm bảo nhận được
struct LastCommand {
  uint8_t sequence;
  uint8_t command;
  uint8_t data1;
  unsigned long timestamp;  // Using timerTicks
} lastCommand = { 0xFF, 0xFF, 0xFF, 0 };

#define COMMAND_DUPLICATE_WINDOW_TICKS 50  // 500ms window to ignore duplicates

///////////////////////////////////////////////// CHECKSUM FUNCTIONS /////////////////////////////////////////////////

extern volatile bool sensorUpLimit;
extern volatile bool homeRun;
extern volatile bool modeAuto;
// New flexible function with variable length
uint8_t calculate_checksum1(const uint8_t *data, int len) {
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

///////////////////////////////////////////////// PACKET CREATION FUNCTIONS /////////////////////////////////////////////////

// Function to create a packet with proper format: STX + payload + checksum + ETX
void createPacket(uint8_t deviceId, uint8_t sequence, uint8_t command,
                  uint8_t data1, uint8_t data2, uint8_t data3) {

  // Create payload (without STX/ETX)
  uint8_t payload[6] = { deviceId, sequence, command, data1, data2, data3 };

  // Calculate checksum for payload
  uint8_t checksum = calculate_checksum1(payload, 6);

  // Create complete packet: STX + payload + checksum + ETX
  uint8_t packet[9];
  packet[0] = STX;       // 0x02
  packet[1] = deviceId;  // 0x70
  packet[2] = sequence;  // Variable
  packet[3] = command;   // Variable
  packet[4] = data1;     // Variable
  packet[5] = data2;     // Variable
  packet[6] = data3;     // Variable
  packet[7] = checksum;  // Calculated
  packet[8] = ETX;       // 0x03

  // Debug output
  mySerial.print("Created packet: ");
  for (int i = 0; i < 9; i++) {
    mySerial.print("0x");
    if (packet[i] < 0x10) mySerial.print("0");
    mySerial.print(packet[i], HEX);
    if (i < 8) mySerial.print(", ");
  }
  mySerial.println();

  // Send packet via UART2
  mySerial2.write(packet, 9);

  mySerial.println("Packet sent via UART2");
}



///////////////////////////////////////////////// PACKET VERIFICATION - OPTIMIZED /////////////////////////////////////////////////
bool verifyCompletePacket(uint8_t *packet, int length) {
  // Quick checks
  if (length != PACKET_SIZE) {
    mySerial.print("!!! Invalid length: ");
    mySerial.println(length);
    return false;
  }

  if (packet[0] != STX || packet[8] != ETX) {
    mySerial.println("!!! Invalid STX/ETX");
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
    mySerial.print("!!! Checksum mismatch: Calc=0x");
    mySerial.print(calculatedChecksum, HEX);
    mySerial.print(" Recv=0x");
    mySerial.println(receivedChecksum, HEX);
    return false;
  }

  mySerial.println("✓ Packet valid");
  return true;
}

///////////////////////////////////////////////// PACKET PROCESSING - OPTIMIZED /////////////////////////////////////////////////
void processCompleteFrame(byte *buf, int len) {
  // Process command if packet is valid
  if (verifyCompletePacket(buf, len)) {
    processCommand(buf, len);
  }
}

///////////////////////////////////////////////// UTILITY FUNCTIONS /////////////////////////////////////////////////

void debugChecksum(const byte *data, int len) {
  byte cs = 0;
  mySerial.print("XOR trace: ");
  for (int i = 0; i < len; i++) {
    cs ^= data[i];
    mySerial.print("0x");
    if (data[i] < 0x10) mySerial.print("0");
    mySerial.print(data[i], HEX);
    if (i < len - 1) mySerial.print(" ^ ");
  }
  mySerial.print(" = 0x");
  mySerial.println(cs, HEX);
}

// --------------------- Hex string → bytes ---------------------
int hexStringToBytes(const char *hexStr, byte *outBytes) {
  int len = strlen(hexStr);
  int byteCount = len / 2;
  for (int i = 0; i < byteCount; i++) {
    char buf[3] = { hexStr[i * 2], hexStr[i * 2 + 1], 0 };
    outBytes[i] = (byte)strtol(buf, NULL, 16);
  }
  return byteCount;
}

// Convert hex character to byte value
byte hexCharToByte(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return 0;
}

// Process hex string and convert to byte array
void processHexString(char hexString[], byte data[], int &dataLength) {
  int hexLen = strlen(hexString);
  dataLength = 0;


  for (int i = 0; i < hexLen && i + 1 < hexLen && dataLength < MAX_DATA_SIZE; i += 2) {
    char hexPair[3] = { hexString[i], hexString[i + 1], '\0' };
    byte byteValue = hexCharToByte(hexString[i]) * 16 + hexCharToByte(hexString[i + 1]);
    data[dataLength] = byteValue;
    dataLength++;
  }
}

// Print processed data with details
void printProcessedData(byte data[], int length) {

  /*if (length == 7) {
    mySerial.println("Processing 7-byte packet:");
    mySerial.print("  Device ID: 0x"); mySerial.println(data[0], HEX);
    mySerial.print("  Sequence: 0x"); mySerial.println(data[1], HEX);
    mySerial.print("  Command: 0x"); mySerial.println(data[2], HEX);
    mySerial.print("  Data1: 0x"); mySerial.println(data[3], HEX);
    mySerial.print("  Data2: 0x"); mySerial.println(data[4], HEX);
    mySerial.print("  Data3: 0x"); mySerial.println(data[5], HEX);
    mySerial.print("  Checksum: 0x"); mySerial.println(data[6], HEX);
  }*/

  mySerial.println("---");
}

// --------------------- Build frame (Send) - OLD VERSION ---------------------
void buildFrameWithChecksum(const char *hexStr) {
  byte payload[64];
  int n = hexStringToBytes(hexStr, payload);

  byte cs = calculate_checksum1(payload, n);

  // Final frame: STX + payload + checksum + ETX
  byte frame[70];
  int idx = 0;
  frame[idx++] = STX;
  memcpy(frame + idx, payload, n);
  idx += n;
  frame[idx++] = cs;
  frame[idx++] = ETX;

  // Debug
  mySerial.print("Input hex string: ");
  mySerial.println(hexStr);

  mySerial.print("Payload bytes: ");
  for (int i = 0; i < n; i++) {
    mySerial.print("0x");
    if (payload[i] < 0x10) mySerial.print("0");
    mySerial.print(payload[i], HEX);
    mySerial.print(" ");
  }
  mySerial.println();

  mySerial.print("Checksum = 0x");
  if (cs < 0x10) mySerial.print("0");
  mySerial.println(cs, HEX);

  mySerial.print("Final frame: ");
  for (int i = 0; i < idx; i++) {
    mySerial.print("0x");
    if (frame[i] < 0x10) mySerial.print("0");
    mySerial.print(frame[i], HEX);
    mySerial.print(" ");
  }
  mySerial.println();

  // Send packet
  mySerial2.write(frame, idx);
}

// --------------------- Process received frame - OLD VERSION ---------------------
void processFrame(byte *buf, int len) {
  if (len < 2) {
    mySerial.println("Frame too short!");
    return;
  }

  int payloadLen = len - 1;
  byte checksumRecv = buf[payloadLen];
  byte checksumCalc = calculate_checksum1(buf, payloadLen);

  mySerial.print("Received frame: ");
  for (int i = 0; i < len; i++) {
    mySerial.print("0x");
    if (buf[i] < 0x10) mySerial.print("0");
    mySerial.print(buf[i], HEX);
    mySerial.print(" ");
  }
  mySerial.println();

  mySerial.print("Checksum recv=0x");
  if (checksumRecv < 0x10) mySerial.print("0");
  mySerial.print(checksumRecv, HEX);

  mySerial.print(" | calc=0x");
  if (checksumCalc < 0x10) mySerial.print("0");
  mySerial.println(checksumCalc, HEX);

  if (checksumRecv == checksumCalc) {
    mySerial.println("Frame OK");
  } else {
    mySerial.println("Frame NG (Checksum mismatch)");
    debugChecksum(buf, payloadLen);
  }
}

///////////////////////////////////////////////// BLE FUNCTIONS /////////////////////////////////////////////////

void serial2DataIncome() {
  // UART2 data processing only (no forwarding to avoid loop)
  if (mySerial2.available()) {
    byte receivedByte = mySerial2.read();
    parsePacket(receivedByte, state2, hexString2, hexIdx2, dataReady2);
  }
  if (dataReady1) {
    mySerial.print("=== UART1 Packet === Hex String: ");
    mySerial.println(hexString1);
    processHexString(hexString1, data1, dataLen1);
    printProcessedData(data1, dataLen1);
    dataReady1 = false;
  }

  if (dataReady2) {
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
    }
    dataReady2 = false;
  }
}

void parsePacket(byte receivedByte, ParseState &currentState,
                 char hexString[], int &hexIndex, bool &dataReady) {

  // Debug
  //mySerial.print("RX: 0x");
  if (receivedByte < 16) {
    //mySerial.print("0");
  }
  //mySerial.print(receivedByte, HEX);
  if (receivedByte >= 32 && receivedByte <= 126) {
    //mySerial.print(" ('");
    //mySerial.print((char)receivedByte);
    //mySerial.print("')");
  } else if (receivedByte == 0x00) {
    //mySerial.print(" (NULL)");
  } else {
    //mySerial.print(" (ctrl)");
  }
  //mySerial.print(" State:");
  //mySerial.println(currentState);

  switch (currentState) {
    case WAIT_START:
      if (receivedByte == STX) {  // 0x02
        hexIndex = 0;
        memset(hexString, 0, MAX_HEX_STRING_SIZE);
        currentState = READ_HEX_STRING;
      }
      break;

    case READ_HEX_STRING:
      if (receivedByte == ETX) {     // 0x03
        hexString[hexIndex] = '\0';  // Null terminate
        mySerial.print("Hex string: '");
        mySerial.print(hexString);
        mySerial.println("'");

        //mySerial.println("Processing packet immediately");
        dataReady = true;
        currentState = WAIT_START;
      } else {
        // Save ASCII hex characters
        if (hexIndex < MAX_HEX_STRING_SIZE - 1 && receivedByte >= 32) {
          hexString[hexIndex] = (char)receivedByte;
          hexIndex++;
        }
      }
      break;

    default:
      currentState = WAIT_START;
      break;
  }
}
///////////////////////////////////////////////// SIMPLE RAW HEX PROCESSING /////////////////////////////////////////////////

// Buffer for raw hex data from mySerial2


// Simple function to process raw hex data from mySerial2
void drawDataProcess() {
  if (mySerial2.available()) {
    byte receivedByte = mySerial2.read();

    // Store received byte in raw buffer
    if (rawBufferIndex < sizeof(rawBuffer)) {
      rawBuffer[rawBufferIndex] = receivedByte;
      rawBufferIndex++;

      // Check for ETX (0x03) to mark end of packet
      if (receivedByte == ETX) {
        processAndPrintResult();
        rawBufferIndex = 0;  // Reset buffer
      }
    } else {
      // Buffer overflow, reset
      rawBufferIndex = 0;
    }
  }
}

// Process raw hex and print result
void processAndPrintResult() {
  // Check if valid packet (starts with STX, ends with ETX)
  if (rawBufferIndex < 3 || rawBuffer[0] != STX) {
    return;  // Invalid packet, ignore
  }

  // Find ETX position
  int etxPos = rawBufferIndex - 1;

  // Extract hex string between STX and ETX
  char hexString[50];
  int hexStringIndex = 0;

  for (int i = 1; i < etxPos; i++) {
    if (hexStringIndex < sizeof(hexString) - 1) {
      hexString[hexStringIndex] = (char)rawBuffer[i];
      hexStringIndex++;
    }
  }
  hexString[hexStringIndex] = '\0';

  // Convert hex string to bytes
  byte processedData[20];
  int processedLength = 0;

  // Convert pairs of hex characters to bytes
  int hexLen = strlen(hexString);
  for (int i = 0; i < hexLen && i + 1 < hexLen; i += 2) {
    if (processedLength < 20) {
      char c1 = hexString[i];
      char c2 = hexString[i + 1];

      byte nibble1 = 0, nibble2 = 0;

      // Convert first nibble
      if (c1 >= '0' && c1 <= '9') nibble1 = c1 - '0';
      else if (c1 >= 'A' && c1 <= 'F') nibble1 = c1 - 'A' + 10;
      else if (c1 >= 'a' && c1 <= 'f') nibble1 = c1 - 'a' + 10;

      // Convert second nibble
      if (c2 >= '0' && c2 <= '9') nibble2 = c2 - '0';
      else if (c2 >= 'A' && c2 <= 'F') nibble2 = c2 - 'A' + 10;
      else if (c2 >= 'a' && c2 <= 'f') nibble2 = c2 - 'a' + 10;

      processedData[processedLength] = (nibble1 << 4) | nibble2;
      processedLength++;
    }
  }

  // Print result
  mySerial.print("Result: 0x");
  if (STX < 0x10) mySerial.print("0");
  mySerial.print(STX, HEX);

  for (int i = 0; i < processedLength; i++) {
    mySerial.print(",0x");
    if (processedData[i] < 0x10) mySerial.print("0");
    mySerial.print(processedData[i], HEX);
  }

  mySerial.print(",0x");
  if (ETX < 0x10) mySerial.print("0");
  mySerial.print(ETX, HEX);
  mySerial.println();
}

///////////////////////////////////////////////// SETUP /////////////////////////////////////////////////
void testUART2() {
  if (mySerial.available()) {
    byte _data0 = mySerial.read();
    mySerial.print(_data0);
    mySerial2.write(_data0);
    _data0 = 0x00;
  }
  if (mySerial2.available()) {
    byte _data1 = mySerial2.read();
    mySerial.write(_data1);
    //mySerial2.write(_data);
    _data1 = 0x00;
  }
}

///////////////////////////////////////////////// COMMAND HELPERS - REDUCE DUPLICATION /////////////////////////////////////////////////

// Helper: Handle motor commands with roll motor restart
// NOTE: Remote có thể gửi nhiều lần cùng 1 lệnh
//       → Nếu motor đang chạy, timeout sẽ được reset (tiếp tục 20s)
inline void handleMotorCommand(uint8_t data1, void (*onFunc)(), void (*offFunc)(), const char *motorName) {
  if (data1 == DATA_ON) {
    mySerial.print(">>> ");
    mySerial.print(motorName);
    mySerial.println(" PUSH");
    offRollMotor();
    onFunc();  // Will reset timeout if already running in same direction
  } else if (data1 == DATA_OFF) {
    mySerial.print("<<< ");
    mySerial.print(motorName);
    mySerial.println(" RELEASE - Motor stopped");
    offFunc();
    // Restart roll motor if in auto mode
    if (rollSpotMode && modeAuto) {
      mySerial.println("    Restarting roll motor (auto mode active)");
      onRollMotor();
    }
  }
}

///////////////////////////////////////////////// COMMAND PROCESSING - OPTIMIZED /////////////////////////////////////////////////
void processCommand(byte *buf, int len) {
  if (len != PACKET_SIZE) return;

  // Extract packet components
  uint8_t deviceId = buf[1];
  uint8_t sequence = buf[2];
  uint8_t command = buf[3];
  uint8_t data1 = buf[4];
  uint8_t data2 = buf[5];
  uint8_t data3 = buf[6];

  // Only process valid device ID
  if (deviceId != DEVICE_ID) return;

  // ═══════════════════════════════════════════════════════════════════
  // COMMAND DEDUPLICATION - Prevent duplicate execution
  // Remote gửi nhiều lần để đảm bảo nhận được, firmware chỉ thực thi 1 lần
  //
  // EXCEPTION: Motor PUSH commands (RECLINE/INCLINE/FORWARD/BACKWARD)
  // được phép duplicate để RESET TIMEOUT (tiếp tục chạy thêm 20s)
  // ═══════════════════════════════════════════════════════════════════
  extern volatile unsigned long timerTicks;
  unsigned long timeSinceLastCommand = timerTicks - lastCommand.timestamp;

  // Check if this is a motor PUSH command (allowed to duplicate for timeout reset)
  bool isMotorPushCommand = ((command == CMD_RECLINE || command == CMD_INCLINE || command == CMD_FORWARD || command == CMD_BACKWARD) && data1 == DATA_ON);

  // Check if duplicate
  bool isDuplicate = (sequence == lastCommand.sequence && command == lastCommand.command && data1 == lastCommand.data1 && timeSinceLastCommand < COMMAND_DUPLICATE_WINDOW_TICKS);

  // Block duplicate EXCEPT for motor PUSH commands
  if (isDuplicate && !isMotorPushCommand) {
    mySerial.println(">>> DUPLICATE COMMAND - Ignored (already executed)");
    mySerial.print("    Time since last: ");
    mySerial.print(timeSinceLastCommand * 10);
    mySerial.println("ms");
    return;  // Skip duplicate command
  }

  // Update last command tracker
  lastCommand.sequence = sequence;
  lastCommand.command = command;
  lastCommand.data1 = data1;
  lastCommand.timestamp = timerTicks;

  // Debug output - Command received
  mySerial.println("\n=== COMMAND RECEIVED ===");

  // Print command name
  mySerial.print("Command Type: ");
  switch (command) {
    case CMD_AUTO: mySerial.println("AUTO MODE"); break;
    case CMD_ROLL_MOTOR: mySerial.println("ROLL MOTOR"); break;
    case CMD_KNEADING: mySerial.println("KNEADING"); break;
    case CMD_PERCUSSION: mySerial.println("PERCUSSION"); break;
    case CMD_COMPRESSION: mySerial.println("COMPRESSION"); break;          // 0x50
    case CMD_COMBINE: mySerial.println("COMBINE"); break;                  // 0x60
    case CMD_INTENSITY_LEVEL: mySerial.println("INTENSITY LEVEL"); break;  // 0x70
    case CMD_INCLINE: mySerial.println("INCLINE"); break;
    case CMD_RECLINE: mySerial.println("RECLINE"); break;
    case CMD_FORWARD: mySerial.println("FORWARD"); break;
    case CMD_BACKWARD: mySerial.println("BACKWARD"); break;
    case CMD_DISCONNECT: mySerial.println("DISCONNECT"); break;
    default: mySerial.println("UNKNOWN"); break;
  }

  mySerial.print("Action: ");
  mySerial.println((data1 == DATA_ON) ? "ON/PUSH" : "OFF/RELEASE");

  // Print current state (simplified)
  mySerial.print("State: allowRun=");
  mySerial.print(allowRun ? "T" : "F");
  mySerial.print(" homeRun=");
  mySerial.print(getHomeRun() ? "T" : "F");
  mySerial.print(" modeAuto=");
  mySerial.println(modeAuto ? "T" : "F");

  // Process commands with optimized switch-case
  switch (command) {
    case CMD_AUTO:  // 0x10 - Auto mode (DEFAULT program)
      if (data1 == DATA_ON) {
        if (!getHomeRun()) {
          mySerial.println("ERROR: Cannot start AUTO - Not homed! Please run GO HOME first");
          break;
        }

        // Set AUTO_DEFAULT program (autodefaultMode + rollSpotMode ON by default)
        mySerial.println(">>> AUTO RUN - Starting DEFAULT program");
        mySerial.println("    Command: 0x10 (AUTO), Data1: 0xF0 (ON)");

        // Clear all other modes first
        setKneadingMode(false);
        setCompressionMode(false);
        setPercussionMode(false);
        setCombineMode(false);

        // Set autodefaultMode (this is CMD_AUTO's unique mode)
        setAutodefaultMode(true);

        // ✅ AUTO_DEFAULT: Roll motor is ALWAYS ON (cannot be toggled)
        setRollSpotMode(true);
        mySerial.println("    Roll Motor: ON (DEFAULT - cannot be disabled in this mode)");

        sensorUpLimit = false;

        // Start auto mode with 20-minute timer
        startAutoMode();

      } else if (data1 == DATA_OFF) {
        // Stop auto mode
        mySerial.println(">>> AUTO STOP - Stopping all programs");
        stopAutoMode();
      }
      break;

    case CMD_ROLL_MOTOR:  // 0x20 - Roll motor toggle (SPOT mode)
      // Command: 0x20, Data1: 0xF0 (ON) or 0x00 (OFF)
      // Can be toggled in: KNEADING, COMPRESSION, PERCUSSION, COMBINED
      // Cannot be toggled in: AUTO_DEFAULT only

      if (!isRollMotorToggleAllowed()) {
        mySerial.println("!!! ROLL MOTOR toggle not allowed in current program");
        mySerial.println("    This command only works in:");
        mySerial.println("    - KNEADING, COMPRESSION, PERCUSSION, or COMBINED programs");
        mySerial.println("    - NOT in AUTO_DEFAULT (roll is always ON)");
        break;
      }

      // Toggle SPOT mode
      if (data1 == DATA_ON) {
        mySerial.println(">>> ROLL MOTOR ON");
        mySerial.println("    Command: 0x20 (ROLL_MOTOR), Data1: 0xF0 (ON)");
        setRollSpotMode(true);  // Turn on roll/spot mode
        // Note: If combined with other modes, creates COMBINED program
        onRollMotor();
      } else if (data1 == DATA_OFF) {
        mySerial.println("<<< ROLL MOTOR OFF");
        mySerial.println("    Command: 0x20 (ROLL_MOTOR), Data1: 0x00 (OFF)");
        setRollSpotMode(false);
        offRollMotor();
      }
      break;

    case CMD_KNEADING:  // 0x30 - Kneading mode
      if (modeAuto) {
        if (data1 == DATA_ON) {
          mySerial.println(">>> KNEADING MODE ON");
          // Switch to KNEADING program (turn off other program modes)
          setAutodefaultMode(false);  // Turn off Default (allow roll toggle)
          setCompressionMode(false);  // Turn off Compression
          setPercussionMode(false);   // Turn off Percussion
          setKneadingMode(true);      // Turn on Kneading
          // NOTE: rollSpotMode is NOT cleared - user can toggle it with CMD_ROLL_MOTOR
          mySerial.println("    Switched to KNEADING program");
          mySerial.print("    Roll Motor: ");
          mySerial.println(rollSpotMode ? "ON (can toggle)" : "OFF (can toggle)");
        } else if (data1 == DATA_OFF) {
          mySerial.println("<<< KNEADING MODE OFF");
          setKneadingMode(false);
        }
      } else {
        mySerial.println("!!! KNEADING command ignored - Not in AUTO mode");
      }
      break;

    case CMD_PERCUSSION:  // 0x40 - Percussion mode
      if (modeAuto) {
        if (data1 == DATA_ON) {
          mySerial.println(">>> PERCUSSION MODE ON");
          // Switch to PERCUSSION program (turn off other program modes)
          setAutodefaultMode(false);  // Turn off Default (allow roll toggle)
          setCompressionMode(false);  // Turn off Compression
          setKneadingMode(false);     // Turn off Kneading
          setPercussionMode(true);    // Turn on Percussion
          // NOTE: rollSpotMode is NOT cleared - user can toggle it with CMD_ROLL_MOTOR
          mySerial.println("    Switched to PERCUSSION program");
          mySerial.print("    Roll Motor: ");
          mySerial.println(rollSpotMode ? "ON (can toggle)" : "OFF (can toggle)");
        } else if (data1 == DATA_OFF) {
          mySerial.println("<<< PERCUSSION MODE OFF");
          setPercussionMode(false);
        }
      } else {
        mySerial.println("!!! PERCUSSION command ignored - Not in AUTO mode");
      }
      break;

    case CMD_COMPRESSION:  // 0x50 - Compression mode (PRIMARY)
      if (modeAuto) {
        if (data1 == DATA_ON) {
          mySerial.println(">>> COMPRESSION MODE ON");
          mySerial.println("    Command: 0x50 (COMPRESSION), Data1: 0xF0 (ON)");
          // Switch to COMPRESSION program (turn off other program modes)
          setAutodefaultMode(false);  // Turn off Default (allow roll toggle)
          setKneadingMode(false);     // Turn off Kneading
          setPercussionMode(false);   // Turn off Percussion
          setCompressionMode(true);   // Turn on Compression
          // NOTE: rollSpotMode is NOT cleared - user can toggle it with CMD_ROLL_MOTOR
          mySerial.println("    Switched to COMPRESSION program");
          mySerial.print("    Roll Motor: ");
          mySerial.println(rollSpotMode ? "ON (can toggle)" : "OFF (can toggle)");
        } else if (data1 == DATA_OFF) {
          mySerial.println("<<< COMPRESSION MODE OFF");
          setCompressionMode(false);
        }
      } else {
        mySerial.println("!!! COMPRESSION command ignored - Not in AUTO mode");
      }
      break;

    case CMD_COMBINE:  // 0x60 - Combine mode (multiple programs)
      if (modeAuto) {
        if (data1 == DATA_ON) {
          mySerial.println(">>> COMBINE MODE ON");
          mySerial.println("    Command: 0x60 (COMBINE), Data1: 0xF0 (ON)");
          // Combine mode - activate multiple programs
          setAutodefaultMode(false);  // Turn off Default
          setKneadingMode(true);      // Turn on Kneading
          setCompressionMode(true);   // Turn on Compression
          // percussionMode can be toggled separately
          // rollSpotMode can be toggled separately
          setCombineMode(true);
          mySerial.println("    Activated COMBINED program (Kneading + Compression)");
          mySerial.print("    Roll Motor: ");
          mySerial.println(rollSpotMode ? "ON (can toggle)" : "OFF (can toggle)");
        } else if (data1 == DATA_OFF) {
          mySerial.println("<<< COMBINE MODE OFF");
          setKneadingMode(false);
          setCompressionMode(false);
          setCombineMode(false);
        }
      } else {
        mySerial.println("!!! COMBINE command ignored - Not in AUTO mode");
      }
      break;

    case CMD_INTENSITY_LEVEL:  // 0x70 - Intensity level control
      if (!isIntensityChangeAllowed()) {
        mySerial.println("!!! Intensity change not allowed in current program");
        mySerial.println("    Only available in: COMPRESSION, PERCUSSION, COMBINED");
        break;
      }

      mySerial.print(">>> INTENSITY LEVEL: ");
      if (data1 == INTENSITY_HIGH) {
        mySerial.println("HIGH (0x20)");
        setIntensityLevel(INTENSITY_HIGH);
      } else if (data1 == INTENSITY_LOW) {
        mySerial.println("LOW (0x00)");
        setIntensityLevel(INTENSITY_LOW);
      } else {
        mySerial.print("CUSTOM (0x");
        mySerial.print(data1, HEX);
        mySerial.println(")");
        setIntensityLevel(data1);
      }
      break;

    case CMD_INCLINE:  // 0x80 - Incline
      handleMotorCommand(data1, onIncline, offReclineIncline, "INCLINE");
      break;

    case CMD_RECLINE:  // 0x90 - Recline
      handleMotorCommand(data1, onRecline, offReclineIncline, "RECLINE");
      break;

    case CMD_FORWARD:  // 0xA0 - Forward
      handleMotorCommand(data1, onForward, offForwardBackward, "FORWARD");
      break;

    case CMD_BACKWARD:  // 0xB0 - Backward
      handleMotorCommand(data1, onBackward, offForwardBackward, "BACKWARD");
      break;

    case CMD_DISCONNECT:  // 0xFF - Disconnect
      if (data1 == DATA_OFF) {
        // ✨ DISCONNECT = AUTO MODE OFF (same behavior)
        mySerial.println(">>> AUTO STOP - Stopping all programs");
        
        // Stop all position motors first (safety)
        offForwardBackward();
        offReclineIncline();
        
        // Stop AUTO mode (same as AUTO OFF command)
        // This will:
        // - Stop roll motor
        // - Reset all mode flags (AUTO, KNEADING, PERCUSSION, etc.)
        // - Reset timer to 0
        // - Trigger GO HOME sequence (chair returns to home position)
        // - Reset to MANUAL mode
        stopAutoMode();
        
        // Reset BLE module (only for DISCONNECT, not for AUTO OFF)
        resetHM10();
        mySerial.println("  - BLE module reset");
        
      } else if (data1 == DATA_ON) {
        mySerial.println(">>> CONNECTED");
      }
      break;

    default:
      mySerial.println(">>> UNKNOWN COMMAND - Ignored");
      break;
  }

  // Debug output - Command processed
  mySerial.println("=== COMMAND PROCESSED ===\n");
}

///////////////////////////////////////////////// COMMAND COUNTER TIMERS - TIMER BASED /////////////////////////////////////////////////
void checkCommandCounters() {
  // Check autoCmdCounter timer (10s = 1000 ticks)
  if (autoCmdTimerActive) {
    if ((timerTicks - autoCmdTimerTick) >= COMMAND_COUNTER_TIMEOUT_TICKS) {
      autoCmdCounter = 0;
      autoCmdTimerActive = false;
    }
  }

  // Check offCmdCounter timer (10s = 1000 ticks)
  if (offCmdTimerActive) {
    if ((timerTicks - offCmdTimerTick) >= COMMAND_COUNTER_TIMEOUT_TICKS) {
      offCmdCounter = 0;
      offCmdTimerActive = false;
    }
  }
}