#ifndef MESSAGEPROCESS_H
#define MESSAGEPROCESS_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <cstdint>
#include <cstring>

///////////////////////////////////////////////// PACKET PARSING CONSTANTS /////////////////////////////////////////////////
#define STX 0x02
#define ETX 0x03
#define PACKET_SIZE 9
#define PAYLOAD_SIZE 6
#define MAX_DATA_SIZE 20
#define MAX_HEX_STRING_SIZE 40

///////////////////////////////////////////////// COMMAND DEFINITIONS /////////////////////////////////////////////////
#define DEVICE_ID 0x70

// Commands
#define CMD_AUTO 0x10
#define CMD_ROLL_MOTOR 0x20
#define CMD_KNEADING 0x30
#define CMD_PERCUSSION 0x40
#define CMD_COMPRESSION 0x50      // Compression mode
#define CMD_COMBINE 0x60          // Combine mode (multiple programs)
#define CMD_INTENSITY_LEVEL 0x70  // Intensity level control (data1 = level value)
#define CMD_INCLINE 0x80
#define CMD_RECLINE 0x90
#define CMD_FORWARD 0xA0
#define CMD_BACKWARD 0xB0
#define CMD_DISCONNECT 0xFF

// Alias for backward compatibility
#define CMD_INTENSITY 0x60  // Same as CMD_COMBINE

// Data values
#define DATA_ON 0xF0
#define DATA_OFF 0x00

// Intensity levels (for CMD_INTENSITY_LEVEL 0x70)
#define INTENSITY_LOW 0x00   // Low level
#define INTENSITY_HIGH 0x20  // High level

///////////////////////////////////////////////// PARSE STATES /////////////////////////////////////////////////
enum ParseState {
  WAIT_START,
  READ_HEX_STRING,
  FIRST_END
};

extern HardwareSerial mySerial;
extern HardwareSerial mySerial2;

// External variables from MessageProcess.cpp
extern byte data1[MAX_DATA_SIZE], data2[MAX_DATA_SIZE];
extern char hexString1[MAX_HEX_STRING_SIZE], hexString2[MAX_HEX_STRING_SIZE];
extern int dataLen1, dataLen2, hexIdx1, hexIdx2;
extern bool dataReady1, dataReady2;
extern ParseState state1, state2;
extern byte rawBuffer[50];
extern int rawBufferIndex;
extern volatile bool autodefaultMode, rollSpotMode, kneadingMode, compressionMode, percussionMode, combineMode;
extern volatile uint8_t intensityLevel;  // Intensity level (0-255) - FEATURE
// Command counter timers - using timer ticks
extern unsigned long autoCmdTimerTick;
extern unsigned long offCmdTimerTick;
extern bool autoCmdTimerActive;
extern bool offCmdTimerActive;

extern void offReclineIncline();
extern void onRecline();
extern void onIncline();
extern void onBackward();
extern void onForward();
extern void offForwardBackward();
extern void offRollMotor();
extern void onRollMotor();
extern void dirRollMotor(bool direction);
extern void resetHM10();

// State management functions
extern void setAutodefaultMode(bool value);
extern void setRollSpotMode(bool value);
extern void setCompressionMode(bool value);
extern void setKneadingMode(bool value);
extern void setPercussionMode(bool value);
extern void setCombineMode(bool value);
extern void setIntensityLevel(uint8_t value);

// Auto mode functions
extern void startAutoMode();
extern void stopAutoMode();

// Helper functions to check if features are allowed
extern bool isRollMotorToggleAllowed();  // Check if roll motor toggle is allowed
extern bool isIntensityChangeAllowed();  // Check if intensity change is allowed
// New flexible function with variable length
uint8_t calculate_checksum1(const uint8_t *data, int len);

///////////////////////////////////////////////// PACKET CREATION FUNCTIONS /////////////////////////////////////////////////

// Function to create a packet with proper format: STX + payload + checksum + ETX
void createPacket(uint8_t deviceId, uint8_t sequence, uint8_t command,
                  uint8_t data1, uint8_t data2, uint8_t data3);



// Function to verify a received complete packet (including STX/ETX)
bool verifyCompletePacket(uint8_t *packet, int length);

// Updated processFrame function for 9-byte packets
void processCompleteFrame(byte *buf, int len);
///////////////////////////////////////////////// UTILITY FUNCTIONS /////////////////////////////////////////////////

void debugChecksum(const byte *data, int len);

// --------------------- Hex string → bytes ---------------------
int hexStringToBytes(const char *hexStr, byte *outBytes);

// Convert hex character to byte value
byte hexCharToByte(char c);

// Process hex string and convert to byte array
void processHexString(char hexString[], byte data[], int &dataLength);

// Print processed data with details
void printProcessedData(byte data[], int length);

// --------------------- Build frame (Send) - OLD VERSION ---------------------
void buildFrameWithChecksum(const char *hexStr);

// --------------------- Process received frame - OLD VERSION ---------------------
void processFrame(byte *buf, int len);

///////////////////////////////////////////////// BLE FUNCTIONS /////////////////////////////////////////////////

void serial2DataIncome();

void parsePacket(byte receivedByte, ParseState &currentState,
                 char hexString[], int &hexIndex, bool &dataReady);
///////////////////////////////////////////////// SIMPLE RAW HEX PROCESSING /////////////////////////////////////////////////

// Buffer for raw hex data from mySerial2


// Simple function to process raw hex data from mySerial2
void drawDataProcess();

// Process raw hex and print result
void processAndPrintResult();

///////////////////////////////////////////////// SETUP /////////////////////////////////////////////////
void testUART2();

///////////////////////////////////////////////// COMMAND PROCESSING /////////////////////////////////////////////////
void processCommand(byte *buf, int len);

///////////////////////////////////////////////// COMMAND COUNTER TIMERS /////////////////////////////////////////////////
void checkCommandCounters();

#endif  // MESSAGE_V1_HARDWARE_H