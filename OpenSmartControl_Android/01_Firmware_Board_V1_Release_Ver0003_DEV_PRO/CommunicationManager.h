#ifndef COMMUNICATION_MANAGER_H
#define COMMUNICATION_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <cstdint>
#include <cstring>
#include "TimerManager.h"
#include "PinDefinitions.h"

/**
 * CommunicationManager Class
 * 
 * Manages all communication operations for the massage chair system.
 * Handles BLE communication, serial communication, and packet processing.
 * 
 * Features:
 * - BLE communication via HM10 module
 * - Serial communication for debugging
 * - Packet parsing and validation
 * - Command processing
 * - Checksum calculation and verification
 * - Command deduplication
 */
class CommunicationManager {
public:
  // Parse states
  enum ParseState {
    WAIT_START,
    READ_HEX_STRING,
    FIRST_END
  };

  // Command definitions
  static const uint8_t DEVICE_ID = 0x70;
  static const uint8_t STX = 0x02;
  static const uint8_t ETX = 0x03;
  static const int PACKET_SIZE = 9;
  static const int PAYLOAD_SIZE = 6;
  static const int MAX_DATA_SIZE = 20;
  static const int MAX_HEX_STRING_SIZE = 40;

  // Commands
  static const uint8_t CMD_AUTO = 0x10;
  static const uint8_t CMD_ROLL_MOTOR = 0x20;
  static const uint8_t CMD_KNEADING = 0x30;
  static const uint8_t CMD_PERCUSSION = 0x40;
  static const uint8_t CMD_COMPRESSION = 0x50;
  static const uint8_t CMD_COMBINE = 0x60;
  static const uint8_t CMD_INTENSITY_LEVEL = 0x70;
  static const uint8_t CMD_INCLINE = 0x80;
  static const uint8_t CMD_RECLINE = 0x90;
  static const uint8_t CMD_FORWARD = 0xA0;
  static const uint8_t CMD_BACKWARD = 0xB0;
  static const uint8_t CMD_DISCONNECT = 0xFF;

  // Data values
  static const uint8_t DATA_ON = 0xF0;
  static const uint8_t DATA_OFF = 0x00;

  // Intensity levels
  // HIGH (0x20) -> PWM = 254, LOW (0x00) -> PWM = 160, OFF (0x00) -> PWM = 0
  static const uint8_t INTENSITY_LOW = 0x00;
  static const uint8_t INTENSITY_HIGH = 0x20;

private:
  // Serial interfaces
  HardwareSerial* debugSerial;  // Debug UART - 115200 baud
  HardwareSerial* bleSerial;    // BLE UART - 9600 baud

  // BLE module control
  static const int HM10_BREAK = HM10_BREAK_PIN;

  // Data storage
  byte data1[MAX_DATA_SIZE], data2[MAX_DATA_SIZE];
  char hexString1[MAX_HEX_STRING_SIZE], hexString2[MAX_HEX_STRING_SIZE];
  int dataLen1, dataLen2, hexIdx1, hexIdx2;
  bool dataReady1, dataReady2;
  ParseState state1, state2;

  // Raw buffer
  byte rawBuffer[50];
  int rawBufferIndex;

  // Command counter timers
  unsigned long autoCmdTimerTick;
  unsigned long offCmdTimerTick;
  bool autoCmdTimerActive;
  bool offCmdTimerActive;

  // Command deduplication
  struct LastCommand {
    uint8_t sequence;
    uint8_t command;
    uint8_t data1;
    unsigned long timestamp;
  } lastCommand;

  static const unsigned long COMMAND_DUPLICATE_WINDOW_TICKS = 200;  // 2000ms (2 seconds)

  // Timer manager reference
  TimerManager* timerManager;

  // Controller references for command execution (void pointers to avoid circular includes)
  void* motorController;
  void* sequenceController;
  void* sensorManager;

  // Manual priority state management
  bool manualPriority;

  // Command counter timeout
  static const unsigned long COMMAND_COUNTER_TIMEOUT_TICKS = 1000;  // 10s

public:
  // Constructor
  CommunicationManager(TimerManager* timerMgr, HardwareSerial* debug, HardwareSerial* ble);

  // Destructor
  ~CommunicationManager();

  // Initialization
  void initialize();
  void resetHM10();

  // Controller setup
  void setControllers(void* motorCtrl, void* seqCtrl, void* sensorCtrl);

  // Manual priority management
  bool getManualPriority() const;
  void setManualPriority(bool value);

  // Serial Communication
  void serialInit();
  void testUART2();

  // BLE Communication
  void serial2DataIncome();
  void drawDataProcess();
  void processAndPrintResult();

  // Packet Processing
  void parsePacket(byte receivedByte, ParseState& currentState,
                   char hexString[], int& hexIndex, bool& dataReady);
  void processFrame(byte* buf, int len);
  void processCompleteFrame(byte* buf, int len);
  void processCommand(byte* buf, int len);

  // Packet Creation
  void createPacket(uint8_t deviceId, uint8_t sequence, uint8_t command,
                    uint8_t data1, uint8_t data2, uint8_t data3);
  void buildFrameWithChecksum(const char* hexStr);

  // Checksum Functions
  uint8_t calculate_checksum1(const uint8_t* data, int len);
  bool verifyCompletePacket(uint8_t* packet, int length);
  void debugChecksum(const byte* data, int len);

  // Utility Functions
  int hexStringToBytes(const char* hexStr, byte* outBytes);
  byte hexCharToByte(char c);
  void processHexString(char hexString[], byte data[], int& dataLength);
  void printProcessedData(byte data[], int length);

  // Command Processing
  void checkCommandCounters();
  bool isCommandDuplicate(uint8_t sequence, uint8_t command, uint8_t data1);
  void updateLastCommand(uint8_t sequence, uint8_t command, uint8_t data1);

  // Command Counter Management
  void startAutoCmdTimer();
  void startOffCmdTimer();
  void stopAutoCmdTimer();
  void stopOffCmdTimer();
  bool isAutoCmdTimerActive() const;
  bool isOffCmdTimerActive() const;

  // Data Access
  byte* getData1() {
    return data1;
  }
  byte* getData2() {
    return data2;
  }
  int getDataLen1() const {
    return dataLen1;
  }
  int getDataLen2() const {
    return dataLen2;
  }
  bool isDataReady1() const {
    return dataReady1;
  }
  bool isDataReady2() const {
    return dataReady2;
  }

  // Serial Access
  HardwareSerial* getDebugSerial() {
    return debugSerial;
  }
  HardwareSerial* getBleSerial() {
    return bleSerial;
  }

private:
  // Helper functions
  void resetDataBuffers();
  void resetParseStates();
  void processRawHexData();
  void handleCommandTimeout();

  // Command processing helpers
  void processAutoCommand(uint8_t data1);
  void processRollMotorCommand(uint8_t data1);
  void processKneadingCommand(uint8_t data1);
  void processPercussionCommand(uint8_t data1);
  void processCompressionCommand(uint8_t data1);
  void processCombineCommand(uint8_t data1);
  void processIntensityCommand(uint8_t data1);
  void processInclineCommand(uint8_t data1);
  void processReclineCommand(uint8_t data1);
  void processForwardCommand(uint8_t data1);
  void processBackwardCommand(uint8_t data1);
  void processDisconnectCommand(uint8_t data1);

  // Validation helpers
  bool isValidCommand(uint8_t command);
  bool isValidData(uint8_t data);
  bool isPacketComplete(const byte* buffer, int length);
};

#endif  // COMMUNICATION_MANAGER_H
