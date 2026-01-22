/**
 * validators.js
 * Input data validation functions
 * 
 * Contains validators for:
 * - Validate MAC address
 * - Validate UUID
 * - Validate QR data
 * - Validate massage settings
 * - Validate user input
 */

/**
 * Validate MAC address format
 * @param {string} mac - MAC address to validate
 * @returns {boolean} True if MAC address is valid
 * 
 * @example
 * validateMacAddress('3c:8a:1f:81:a0:9e') // true
 * validateMacAddress('3c-8a-1f-81-a0-9e') // true
 * validateMacAddress('invalid-mac') // false
 */
export const validateMacAddress = (mac) => {
  if (!mac || typeof mac !== 'string') return false;
  
  // Regex for MAC address with : or - separators
  const macRegex = /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/;
  return macRegex.test(mac);
};

/**
 * Validate UUID format
 * @param {string} uuid - UUID to validate
 * @returns {boolean} True if UUID is valid
 * 
 * @example
 * validateUUID('6e400001-b5a3-f393-e0a9-e50e24dcca9e') // true
 * validateUUID('invalid-uuid') // false
 */
export const validateUUID = (uuid) => {
  if (!uuid || typeof uuid !== 'string') return false;
  
  // Regex for UUID v4
  const uuidRegex = /^[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i;
  return uuidRegex.test(uuid);
};

/**
 * Validate QR code data
 * @param {string} data - QR code data (JSON string)
 * @returns {Object} Parsed object if valid
 * @throws {Error} If data is invalid
 * 
 * @example
 * validateQRData('{"type":"massage_device","mac":"3c:8a:1f:81:a0:9e","uuid":"..."}')
 * // Returns: { type: "massage_device", mac: "3c:8a:1f:81:a0:9e", uuid: "..." }
 */
export const validateQRData = (data) => {
  if (!data || typeof data !== 'string') {
    throw new Error('Invalid QR data');
  }

  try {
    const qrData = JSON.parse(data);
    
    // Check required fields
    const requiredFields = ['type', 'uuid', 'mac'];
    for (const field of requiredFields) {
      if (!qrData[field]) {
        throw new Error(`Missing information: ${field}`);
      }
    }
    
    // Validate MAC address
    if (!validateMacAddress(qrData.mac)) {
      throw new Error('Invalid MAC address format');
    }
    
    // Validate UUID
    if (!validateUUID(qrData.uuid)) {
      throw new Error('Invalid UUID format');
    }
    
    // Validate type
    if (qrData.type !== 'massage_device') {
      throw new Error('Device type not supported');
    }
    
    return qrData;
  } catch (error) {
    if (error instanceof SyntaxError) {
      throw new Error('QR code is not valid JSON');
    }
          throw new Error(`Invalid QR code: ${error.message}`);
  }
};

/**
 * Validate massage intensity
 * @param {string} intensity - Intensity to validate
 * @returns {boolean} True if intensity is valid (LOW/HIGH)
 * 
 * @example
 * validateIntensity('LOW') // true
 * validateIntensity('HIGH') // true
 * validateIntensity('MEDIUM') // false
 */
export const validateIntensity = (intensity) => {
  const validIntensities = ['LOW', 'HIGH'];
  return validIntensities.includes(intensity);
};

/**
 * Validate massage mode
 * @param {string} mode - Massage mode
 * @returns {boolean} True if mode is valid
 * 
 * @example
 * validateMode('relax') // true
 * validateMode('invalid') // false
 */
export const validateMode = (mode) => {
  const validModes = ['relax', 'deep', 'gentle', 'auto', 'manual'];
  return validModes.includes(mode);
};

/**
 * Validate massage duration
 * @param {number|string} duration - Duration (minutes)
 * @returns {boolean} True if duration is valid
 * 
 * @example
 * validateDuration(15) // true
 * validateDuration('30') // true
 * validateDuration(0) // false
 */
export const validateDuration = (duration) => {
  const num = parseInt(duration);
  return !isNaN(num) && num > 0 && num <= 60; // Maximum 60 minutes
};

/**
 * Validate percentage value (0-100)
 * @param {number|string} value - Value to validate
 * @returns {boolean} True if value is valid
 * 
 * @example
 * validatePercentage(50) // true
 * validatePercentage('75') // true
 * validatePercentage(150) // false
 */
export const validatePercentage = (value) => {
  const num = parseInt(value);
  return !isNaN(num) && num >= 0 && num <= 100;
};

/**
 * Validate device name
 * @param {string} name - Device name
 * @returns {boolean} True if name is valid
 * 
 * @example
 * validateDeviceName('ESP32-MASSAGE') // true
 * validateDeviceName('') // false
 */
export const validateDeviceName = (name) => {
  if (!name || typeof name !== 'string') return false;
  
  // Device name must have at least 3 characters and no special characters
  const nameRegex = /^[a-zA-Z0-9\-_\s]{3,30}$/;
  return nameRegex.test(name.trim());
};

/**
 * Validate command sent to ESP32
 * @param {string} command - Command to validate
 * @returns {boolean} True if command is valid
 * 
 * @example
 * validateCommand('START') // true
 * validateCommand('INTENSITY:2') // true
 * validateCommand('') // false
 */
export const validateCommand = (command) => {
  if (!command || typeof command !== 'string') return false;
  
  const validCommands = [
    'START', 'STOP', 'MODE', 'INTENSITY', 'ROLLSPOT',
    'KNEADING', 'COMBINE', 'PERCUSSION', 'COMPRESSION',
    'RECLINE', 'INCLINE', 'FORWARD', 'BACKWARD', 'TIMER',
    'STATUS', 'DISCONNECT'
  ];
  
  // Split command and value
  const [cmd] = command.split(':');
  return validCommands.includes(cmd.toUpperCase());
};

/**
 * Validate email address
 * @param {string} email - Email to validate
 * @returns {boolean} True if email is valid
 * 
 * @example
 * validateEmail('user@example.com') // true
 * validateEmail('invalid-email') // false
 */
export const validateEmail = (email) => {
  if (!email || typeof email !== 'string') return false;
  
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  return emailRegex.test(email);
};

/**
 * Validate phone number (Vietnam format)
 * @param {string} phone - Phone number
 * @returns {boolean} True if phone number is valid
 * 
 * @example
 * validatePhone('0123456789') // true
 * validatePhone('+84123456789') // true
 * validatePhone('123') // false
 */
export const validatePhone = (phone) => {
  if (!phone || typeof phone !== 'string') return false;
  
  // Regex for Vietnam phone number
  const phoneRegex = /^(\+84|84|0)(3|5|7|8|9)([0-9]{8})$/;
  return phoneRegex.test(phone.replace(/\s/g, ''));
};

/**
 * Validate all massage settings
 * @param {Object} settings - Object containing settings
 * @returns {Object} Object containing validation result
 * 
 * @example
 * validateMassageSettings({
 *   mode: 'relax',
 *   intensity: 'LOW',
 *   duration: 15
 * })
 * // Returns: { isValid: true, errors: [] }
 */
export const validateMassageSettings = (settings) => {
  const errors = [];
  
  if (settings.mode && !validateMode(settings.mode)) {
    errors.push('Invalid massage mode');
  }
  
  if (settings.intensity && !validateIntensity(settings.intensity)) {
    errors.push('Massage intensity must be LOW or HIGH');
  }
  
  if (settings.duration && !validateDuration(settings.duration)) {
    errors.push('Invalid massage duration');
  }
  
  if (settings.recline !== undefined && !validatePercentage(settings.recline)) {
    errors.push('Recline angle must be 0-100%');
  }
  
  if (settings.incline !== undefined && !validatePercentage(settings.incline)) {
    errors.push('Incline angle must be 0-100%');
  }
  
  return {
    isValid: errors.length === 0,
    errors
  };
};

/**
 * Sanitize input string to prevent injection
 * @param {string} input - Input string
 * @returns {string} Sanitized string
 * 
 * @example
 * sanitizeInput('<script>alert("xss")</script>') // 'scriptalert("xss")/script'
 */
export const sanitizeInput = (input) => {
  if (!input || typeof input !== 'string') return '';
  
  return input
    .replace(/[<>]/g, '') // Remove < and >
    .replace(/['"]/g, '') // Remove quotes
    .trim();
};
