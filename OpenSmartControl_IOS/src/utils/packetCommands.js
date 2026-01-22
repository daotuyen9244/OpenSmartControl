/**
 * packetCommands.js
 * Command mapping cho giao thức 9-byte packet với firmware
 * 
 * Cấu trúc packet: STX + DeviceID + Sequence + Command + Data1 + Data2 + Data3 + Checksum + ETX
 * Format: 0x02 + 0x70 + [Sequence] + [Command] + [Data1] + [Data2] + [Data3] + [Checksum] + 0x03
 */

/**
 * Device ID constants
 */
export const DEVICE_ID = {
  MAIN: 0x70,           // Main device ID
};

/**
 * Command codes cho firmware
 */
export const COMMAND_CODES = {
  // Mode control
  AUTO_MODE: 0x10,      // Auto mode on/off
  ROLL_MOTOR: 0x20,     // Roll motor control (legacy SPOT mode toggle)
  
  // Manual control - Roll direction (cùng command code, phân biệt bằng data1)
  ROLL_UP: 0x21,        // Roll up control (manual - data1=0x02, data2=on/off)
  ROLL_DOWN: 0x21,      // Roll down control (manual - data1=0x01, data2=on/off)
  
  // Manual control - Massage motors (khác command code)
  KNEADING_PUSH: 0x22,  // Kneading manual control (data1=0x03, data2=on/off)
  PERCUSSION_PUSH: 0x23, // Percussion manual control (data1=0x04, data2=on/off)
  
  // AUTO mode commands
  KNEADING: 0x30,       // Kneading mode (AUTO mode)
  PERCUSSION: 0x40,     // Percussion mode (AUTO mode)
  COMBINE: 0x50,        // Combine mode
  COMPRESSION: 0x60,    // Compression mode
  INTENSITY_UP: 0x70,   // Intensity up/down
  
  // Chair position control
  INCLINE: 0x80,        // Incline control
  RECLINE: 0x90,        // Recline control
  FORWARD: 0xA0,        // Forward control
  BACKWARD: 0xB0,       // Backward control
  
  // System commands
  DISCONNECT: 0xFF,     // Disconnect command
};

/**
 * Sequence numbers cho các command
 */
export const SEQUENCE = {
  AUTO: 0xC3,           // Auto mode sequence
  ROLL: 0x62,          // Roll motor sequence
  KNEADING: 0x93,      // Kneading sequence
  COMBINE: 0x03,       // Combine sequence
  PERCUSSION: 0xE3,    // Percussion sequence
  COMPRESSION: 0x24,   // Compression sequence
  INTENSITY: 0x73,     // Intensity sequence
  RECLINE_PUSH: 0x41,  // Recline push sequence
  RECLINE_RELEASE: 0x61, // Recline release sequence
  INCLINE_PUSH: 0x81,  // Incline push sequence
  INCLINE_RELEASE: 0x61, // Incline release sequence
  FORWARD_PUSH: 0xD2,  // Forward push sequence
  FORWARD_RELEASE: 0xD2, // Forward release sequence
  BACKWARD_PUSH: 0xA2, // Backward push sequence
  BACKWARD_RELEASE: 0xA2, // Backward release sequence
  ROLL_UP_PUSH: 0x72,  // Roll up push sequence
  ROLL_UP_RELEASE: 0x72, // Roll up release sequence
  ROLL_DOWN_PUSH: 0x82, // Roll down push sequence
  ROLL_DOWN_RELEASE: 0x82, // Roll down release sequence
  KNEADING_PUSH: 0x93,  // Kneading push sequence (manual control)
  KNEADING_RELEASE: 0x93, // Kneading release sequence (manual control)
  PERCUSSION_PUSH: 0xE3, // Percussion push sequence (manual control)
  PERCUSSION_RELEASE: 0xE3, // Percussion release sequence (manual control)
  DISCONNECT: 0xFF,     // Disconnect sequence
  STATUS: 0x00,        // Status request sequence
};

/**
 * Data values
 */
export const DATA_VALUES = {
  ON: 0xF0,            // Turn on/activate
  OFF: 0x00,           // Turn off/deactivate
  INTENSITY_LEVEL: 0x50, // Intensity level value
};

/**
 * Command definitions với đầy đủ thông tin
 */
export const COMMANDS = {
  // Auto mode commands
  AUTO_ON: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.AUTO,
    command: COMMAND_CODES.AUTO_MODE,
    data1: DATA_VALUES.ON,
    data2: 0x00,
    data3: 0x00,
    description: 'Enable auto mode'
  },
  
  AUTO_OFF: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.AUTO,
    command: COMMAND_CODES.AUTO_MODE,
    data1: DATA_VALUES.OFF,
    data2: 0x00,
    data3: 0x00,
    description: 'Disable auto mode'
  },
  
  // Roll motor commands
  ROLL_ON: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.ROLL,
    command: COMMAND_CODES.ROLL_MOTOR,
    data1: DATA_VALUES.ON,
    data2: 0x00,
    data3: 0x00,
    description: 'Turn on roll motor'
  },
  
  ROLL_OFF: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.ROLL,
    command: COMMAND_CODES.ROLL_MOTOR,
    data1: DATA_VALUES.OFF,
    data2: 0x00,
    data3: 0x00,
    description: 'Turn off roll motor'
  },
  
  SPOT_ON: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.ROLL,
    command: COMMAND_CODES.ROLL_MOTOR,
    data1: DATA_VALUES.OFF,
    data2: 0x00,
    data3: 0x00,
    description: 'Enable spot mode (roll off)'
  },
  
  // Massage technique commands
  KNEADING_ON: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.KNEADING,
    command: COMMAND_CODES.KNEADING,
    data1: DATA_VALUES.ON,
    data2: 0x00,
    data3: 0x00,
    description: 'Enable kneading mode'
  },
  
  COMBINE_ON: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.COMBINE,
    command: COMMAND_CODES.COMBINE,
    data1: DATA_VALUES.ON,
    data2: 0x00,
    data3: 0x00,
    description: 'Enable combine mode'
  },
  
  PERCUSSION_ON: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.PERCUSSION,
    command: COMMAND_CODES.PERCUSSION,
    data1: DATA_VALUES.ON,
    data2: 0x00,
    data3: 0x00,
    description: 'Enable percussion mode'
  },
  
  COMPRESSION_ON: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.COMPRESSION,
    command: COMMAND_CODES.COMPRESSION,
    data1: DATA_VALUES.ON,
    data2: 0x00,
    data3: 0x00,
    description: 'Enable compression mode'
  },
  
  // Intensity commands
  INTENSITY_UP: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.INTENSITY,
    command: COMMAND_CODES.INTENSITY_UP,
    data1: 0x00,
    data2: 0x00,
    data3: DATA_VALUES.INTENSITY_LEVEL,
    description: 'Increase intensity'
  },
  
  INTENSITY_DOWN: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.INTENSITY,
    command: COMMAND_CODES.INTENSITY_UP,
    data1: 0x00,
    data2: 0x00,
    data3: DATA_VALUES.INTENSITY_LEVEL,
    description: 'Decrease intensity'
  },
  
  // Position control commands
  RECLINE_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.RECLINE_PUSH,
    command: COMMAND_CODES.RECLINE,
    data1: 0xF0,  // PUSH: 0xF0
    data2: 0x00,
    data3: 0x00,
    description: 'Recline push (start)'
  },
  
  RECLINE_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.RECLINE_RELEASE,
    command: COMMAND_CODES.RECLINE,
    data1: 0x00,  // RELEASE: 0x00
    data2: 0x00,
    data3: 0x00,
    description: 'Recline release (stop)'
  },
  
  INCLINE_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.INCLINE_PUSH,
    command: COMMAND_CODES.INCLINE,
    data1: 0xF0,  // PUSH: 0xF0
    data2: 0x00,
    data3: 0x00,
    description: 'Incline push (start)'
  },
  
  INCLINE_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.INCLINE_RELEASE,
    command: COMMAND_CODES.INCLINE,
    data1: 0x0,   // RELEASE: 0x0
    data2: 0x00,
    data3: 0x00,
    description: 'Incline release (stop)'
  },
  
  FORWARD_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.FORWARD_PUSH,
    command: COMMAND_CODES.FORWARD,
    data1: 0xF0,  // PUSH: 0xF0
    data2: 0x00,
    data3: 0x00,
    description: 'Forward push (start)'
  },
  
  FORWARD_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.FORWARD_RELEASE,
    command: COMMAND_CODES.FORWARD,
    data1: 0x00,  // RELEASE: 0x00
    data2: 0x00,
    data3: 0x00,
    description: 'Forward release (stop)'
  },
  
  BACKWARD_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.BACKWARD_PUSH,
    command: COMMAND_CODES.BACKWARD,
    data1: 0xF0,  // PUSH: 0xF0
    data2: 0x00,
    data3: 0x00,
    description: 'Backward push (start)'
  },
  
  BACKWARD_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.BACKWARD_RELEASE,
    command: COMMAND_CODES.BACKWARD,
    data1: 0x00,  // RELEASE: 0x00
    data2: 0x00,
    data3: 0x00,
    description: 'Backward release (stop)'
  },
  
  // Roll motor direction control commands (cùng command code 0x21, phân biệt bằng data1)
  ROLL_UP_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.ROLL_UP_PUSH,
    command: COMMAND_CODES.ROLL_UP,  // Command code: 0x21 (cùng với ROLL_DOWN)
    data1: 0x02,  // Function: ROLL UP (2)
    data2: DATA_VALUES.ON,  // ON: 0xF0
    data3: 0x00,
    description: 'Roll up push (start)'
  },
  
  ROLL_UP_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.ROLL_UP_RELEASE,
    command: COMMAND_CODES.ROLL_UP,  // Command code: 0x21 (cùng với ROLL_DOWN)
    data1: 0x02,  // Function: ROLL UP (2)
    data2: DATA_VALUES.OFF,  // OFF: 0x00
    data3: 0x00,
    description: 'Roll up release (stop)'
  },
  
  ROLL_DOWN_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.ROLL_DOWN_PUSH,
    command: COMMAND_CODES.ROLL_DOWN,  // Command code: 0x21 (cùng với ROLL_UP)
    data1: 0x01,  // Function: ROLL DOWN (1)
    data2: DATA_VALUES.ON,  // ON: 0xF0
    data3: 0x00,
    description: 'Roll down push (start)'
  },
  
  ROLL_DOWN_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.ROLL_DOWN_RELEASE,
    command: COMMAND_CODES.ROLL_DOWN,  // Command code: 0x21 (cùng với ROLL_UP)
    data1: 0x01,  // Function: ROLL DOWN (1)
    data2: DATA_VALUES.OFF,  // OFF: 0x00
    data3: 0x00,
    description: 'Roll down release (stop)'
  },
  
  // Kneading manual control commands (command code: 0x22, khác với PERCUSSION_PUSH)
  KNEADING_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.KNEADING_PUSH,
    command: COMMAND_CODES.KNEADING_PUSH,  // Command code: 0x22 (khác với PERCUSSION_PUSH 0x23)
    data1: 0x03,  // Function: KNEADING motor (3)
    data2: DATA_VALUES.ON,  // ON: 0xF0
    data3: 0x00,
    description: 'Kneading push (manual control - start, does not affect ROLL motor)'
  },
  
  KNEADING_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.KNEADING_RELEASE,
    command: COMMAND_CODES.KNEADING_PUSH,  // Command code: 0x22
    data1: 0x03,  // Function: KNEADING motor (3)
    data2: DATA_VALUES.OFF,  // OFF: 0x00
    data3: 0x00,
    description: 'Kneading release (manual control - stop, does not affect ROLL motor)'
  },
  
  // Percussion manual control commands (command code: 0x23, khác với KNEADING_PUSH)
  PERCUSSION_PUSH: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.PERCUSSION_PUSH,
    command: COMMAND_CODES.PERCUSSION_PUSH,  // Command code: 0x23 (khác với KNEADING_PUSH 0x22)
    data1: 0x04,  // Function: PERCUSSION/COMPRESSION motor (4)
    data2: DATA_VALUES.ON,  // ON: 0xF0
    data3: 0x00,
    description: 'Percussion push (manual control - start, does not affect ROLL motor)'
  },
  
  PERCUSSION_RELEASE: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.PERCUSSION_RELEASE,
    command: COMMAND_CODES.PERCUSSION_PUSH,  // Command code: 0x23
    data1: 0x04,  // Function: PERCUSSION/COMPRESSION motor (4)
    data2: DATA_VALUES.OFF,  // OFF: 0x00
    data3: 0x00,
    description: 'Percussion release (manual control - stop, does not affect ROLL motor)'
  },
  
  // System commands
  DISCONNECT: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.DISCONNECT,
    command: 0xFF,
    data1: 0x00,
    data2: 0x00,
    data3: 0x00,
    description: 'Disconnect from device'
  },
  
  STATUS_REQUEST: {
    deviceId: DEVICE_ID.MAIN,
    sequence: SEQUENCE.STATUS,
    command: 0x00,
    data1: 0x00,
    data2: 0x00,
    data3: 0x00,
    description: 'Request device status'
  }
};

/**
 * Helper function để lấy command definition
 * @param {string} commandName - Tên command
 * @returns {Object} - Command definition object
 */
export function getCommand(commandName) {
  const command = COMMANDS[commandName];
  if (!command) {
    throw new Error(`Unknown command: ${commandName}`);
  }
  return command;
}

/**
 * Helper function để lấy tất cả command names
 * @returns {Array} - Array of command names
 */
export function getAllCommandNames() {
  return Object.keys(COMMANDS);
}

/**
 * Helper function để validate command
 * @param {Object} command - Command object
 * @returns {boolean} - True if valid
 */
export function validateCommand(command) {
  return command &&
         typeof command.deviceId === 'number' &&
         typeof command.sequence === 'number' &&
         typeof command.command === 'number' &&
         typeof command.data1 === 'number' &&
         typeof command.data2 === 'number' &&
         typeof command.data3 === 'number';
}

export default COMMANDS;
