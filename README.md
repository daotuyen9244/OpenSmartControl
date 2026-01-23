# OpenSmartControl - Smart Massage Chair Control System

A complete massage chair control system including mobile applications (Android & iOS), ESP32 firmware, and hardware documentation. The system uses Bluetooth Low Energy (BLE) for communication between mobile applications and the control device.

## 📋 Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Key Features](#key-features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Detailed Documentation](#detailed-documentation)
- [Development](#development)
- [Contributing](#contributing)

## 🎯 Overview

OpenSmartControl is an **open-source, protocol-based** smart massage chair control system. The application works with **generic hardware components** and does not require any specific brand of hardware. The system includes:

- **Android Application**: React Native app for Android
- **iOS Application**: React Native app for iOS  
- **ESP32 Firmware**: Open-source control code for ESP32 board (MIT License)
- **Hardware Documentation**: Open schematics and PCB layouts

### Key Points

- ✅ **Open-source**: All code and protocols are open-source (MIT License)
- ✅ **Protocol-based**: Works with any device implementing the OpenSmartControl protocol
- ✅ **No brand requirement**: Uses generic hardware components available from multiple suppliers
- ✅ **DIY-friendly**: Users can build their own compatible hardware using provided documentation

### System Architecture

```
┌─────────────────┐
│  Mobile App     │
│  (Android/iOS)  │
└────────┬────────┘
         │ BLE
         │
┌────────▼────────┐
│   ESP32 Board   │
│   + HM10 Module │
└────────┬────────┘
         │ UART
         │
┌────────▼────────┐
│  Motor Control  │
│   & Sensors     │
└─────────────────┘
```

## 📁 Project Structure

```
OpenSmartControl/
├── OpenSmartControl_Android/      # Android Application
│   ├── src/                        # Source code
│   │   ├── components/             # UI components
│   │   ├── services/               # BLE & Permission services
│   │   ├── store/                  # Redux store
│   │   ├── styles/                 # Styles & constants
│   │   └── utils/                   # Utilities & helpers
│   ├── android/                    # Android native code
│   ├── ios/                        # iOS native code
│   ├── 01_Firmware_Board_V1_Release_Ver0003_DEV_PRO/  # Firmware source
│   └── Readme.md                   # Android app documentation
│
├── OpenSmartControl_IOS/           # iOS Application
│   ├── src/                        # Source code (similar to Android)
│   ├── ios/                        # iOS native code
│   ├── android/                    # Android native code
│   └── Readme.md                   # iOS app documentation
│
├── OpenSmartControl_Firmware/       # Firmware releases
│   ├── OpenSmartControl_Release.hex # Firmware hex file
│   └── README.md                   # Firmware documentation
│
└── OpenSmartControl_Harware/        # Hardware documentation
    ├── OpenSmartControl.pdf         # Schematic/PCB documentation
    └── plots/                       # Gerber files
```

## ✨ Key Features

### Mobile Applications

- ✅ **QR Code Scanning**: Quick connection to ESP32 device by scanning QR code
- ✅ **Manual Connection**: Search and connect to Bluetooth devices
- ✅ **Massage Control**: Full-featured massage control interface
- ✅ **Chair Position Control**: Control chair elevation and forward/backward movement
- ✅ **Optimized UI**: Mobile-optimized UI/UX design
- ✅ **Heartbeat Mechanism**: Maintains stable BLE connection
- ✅ **State Management**: State management with Redux Toolkit
- ✅ **Error Handling**: Comprehensive error handling with clear messages
- ✅ **Connection Persistence**: Saves and auto-reconnects to previously connected devices
- ✅ **Debug Panel**: BLE debugging tool for troubleshooting

### ESP32 Firmware

- ✅ **BLE Communication**: Communication via HM10 module with UART
- ✅ **Packet Protocol**: Checksum-based packet format to ensure data integrity
- ✅ **Command Processing**: Handles 15+ different control commands
- ✅ **Safety Management**: Safety management with sensors and limits
- ✅ **Motor Control**: Controls massage motors and chair position
- ✅ **Auto Programs**: Automatic massage programs (Kneading, Percussion, Compression, Combine)
- ✅ **Manual Control**: Manual control with press-and-hold mode
- ✅ **Home Sequence**: Automatically returns to initial position when needed

## 🔧 System Requirements

### Mobile Applications

- **React Native**: >= 0.70
- **Node.js**: >= 16 (recommended >= 18)
- **Android SDK**: For Android development
- **Xcode**: >= 12.0 for iOS development
- **CocoaPods**: For iOS dependencies

### Firmware

- **Arduino IDE**: >= 1.8.x
- **ESP32 Board Support**: ESP32 Arduino Core
- **Libraries**: 
  - BLE libraries for ESP32
  - UART libraries

### Hardware

- **ESP32 Development Board**
- **HM10 BLE Module**
- **Motor Controllers**
- **Sensors** (limit switches, etc.)

## 🔌 Hardware Compatibility & App Store Compliance

### Hardware Requirements

**This application does NOT work with a specific brand of hardware only.**

The app is designed to work with **generic, open-source hardware** that follows the OpenSmartControl communication protocol. The application is **protocol-based** and can work with any compatible device that implements the required BLE communication protocol.

#### Compatible Hardware Components

The app works with standard, commercially available components:

1. **ESP32 Development Board**
   - Any ESP32-based development board (ESP32, ESP32-S2, ESP32-S3, etc.)
   - Available from multiple manufacturers (Espressif Systems, Adafruit, SparkFun, etc.)
   - Can be purchased from various electronics suppliers:
     - Amazon, AliExpress, DigiKey, Mouser, Adafruit, SparkFun, etc.

2. **HM10 BLE Module**
   - Generic HM10 Bluetooth Low Energy module
   - Available from multiple manufacturers and suppliers
   - Standard BLE 4.0 module widely available in electronics markets

3. **Motor Controllers & Sensors**
   - Standard motor controllers and limit switches
   - Generic components available from multiple suppliers

#### No Brand-Specific Requirements

- ❌ **No specific brand name** is required
- ❌ **No proprietary hardware** is needed
- ✅ **Open-source firmware** is provided in this repository
- ✅ **Open communication protocol** - any device implementing the protocol can work
- ✅ **DIY-friendly** - users can build their own compatible hardware

#### Where Users Can Purchase Hardware

Users can purchase compatible hardware components from:

- **Electronics Retailers**: Amazon, AliExpress, eBay
- **Component Distributors**: DigiKey, Mouser, Adafruit, SparkFun
- **Local Electronics Stores**: Any store selling ESP32 and BLE modules
- **Online Marketplaces**: Various online platforms selling Arduino/ESP32 components

**Note**: Users need to flash the provided open-source firmware onto their ESP32 board to make it compatible with the app.

### App Store Compliance

This application complies with App Store guidelines:

- ✅ **No brand-specific hardware requirement** - works with generic components
- ✅ **Open-source protocol** - documented communication protocol
- ✅ **No third-party brand names** in user-facing content
- ✅ **Generic naming** - uses generic terms like "Massage Chair Control" and "MASSAGE_DEVICE"
- ✅ **No proprietary content** - all code and protocols are open-source

For detailed App Store compliance information, see [App Store Compliance Guide](OpenSmartControl_IOS/APP_STORE_COMPLIANCE.md).

## 🚀 Installation

### 1. Clone Repository

```bash
git clone <repository-url>
cd OpenSmartControl
```

### 2. Install Android Application

```bash
cd OpenSmartControl_Android
npm install
# or
yarn install

# Install pods for iOS (if needed)
cd ios && pod install && cd ..
```

### 3. Install iOS Application

```bash
cd OpenSmartControl_IOS
npm install
# or
yarn install

# Install pods
cd ios && pod install && cd ..
```

### 4. Run Application

#### Android

```bash
# Run Metro bundler
npm start

# Run on Android (in another terminal)
npm run android
# or
npx react-native run-android
```

#### iOS

```bash
# Run Metro bundler
npm start

# Run on iOS (in another terminal)
npm run ios
# or
npx react-native run-ios
```

### 5. Build Release

#### Android APK

```bash
cd OpenSmartControl_Android

# Use clean.sh script
./clean.sh -r  # Build release APK
./clean.sh -d  # Build debug APK
./clean.sh -c  # Clean build artifacts
./clean.sh -h  # Show help
```

#### iOS Archive

```bash
cd OpenSmartControl_IOS

# Build bundle
npm run build:ios:bundle

# Then build in Xcode
open ios/MassageChairControl.xcworkspace
```

## 📱 Usage

### Device Connection

#### Via QR Code (Recommended)

1. Open the application
2. Select "Scan QR Code"
3. Scan the QR code on the device or enter manually
4. The application will automatically find and connect

**QR Code Format:**
```json
{
  "type": "massage_device",
  "name": "MASSAGE_DEVICE"
}
```

#### Manual Connection

1. Open the application
2. Select "Manual Connect"
3. Wait for the application to scan BLE devices
4. Select device from the list
5. Connect

### Massage Control

#### Automatic Mode (AUTO)

- **AUTO DEFAULT**: Default massage program with roll motor always on
- **KNEADING**: Kneading technique
- **PERCUSSION**: Percussion technique
- **COMPRESSION**: Compression technique
- **COMBINE**: Combination of multiple techniques

#### Manual Mode (MANUAL)

- **Roll Motor**: Control roll motor up/down
- **Kneading**: Turn kneading motor on/off
- **Percussion**: Turn percussion motor on/off
- **Intensity**: Adjust intensity (LOW/HIGH)

#### Chair Position Control

- **RECLINE**: Lower the chair
- **INCLINE**: Raise the chair
- **FORWARD**: Push chair forward
- **BACKWARD**: Pull chair backward

## 📚 Detailed Documentation

### Mobile Applications

- **[Android App README](OpenSmartControl_Android/Readme.md)**: Detailed Android application documentation
- **[iOS App README](OpenSmartControl_IOS/Readme.md)**: Detailed iOS application documentation
- **[QR Code Format](OpenSmartControl_Android/QR_CODE_FORMAT.md)**: QR Code format guide
- **[BLE Improvements](OpenSmartControl_Android/BLE_IMPROVEMENTS_SUMMARY.md)**: BLE improvements summary
- **[BLE Connection Analysis](OpenSmartControl_Android/BLE_Connection_Analysis.md)**: BLE connection analysis
- **[Disconnect Flow](OpenSmartControl_Android/DISCONNECT_FLOW.md)**: Disconnect flow documentation

### iOS Specific

- **[App Store Compliance](OpenSmartControl_IOS/APP_STORE_COMPLIANCE.md)**: App Store compliance guide
- **[iOS Warnings Guide](OpenSmartControl_IOS/IOS_WARNINGS_GUIDE.md)**: iOS warnings handling guide
- **[Metadata Changes](OpenSmartControl_IOS/METADATA_CHANGES.md)**: Metadata changes documentation

### Firmware

- **[Firmware README](OpenSmartControl_Firmware/README.md)**: Detailed firmware documentation
- **[Protocol Documentation](OpenSmartControl_Android/src/utils/PROTOCOL_README.md)**: Data transmission protocol documentation

### Data Transmission Protocol

The system uses a packet protocol with the following structure:

```
[STX] [DeviceID] [Sequence] [Command] [Data1] [Data2] [Data3] [Checksum] [ETX]
 0x02    0x70      Variable    Variable  Variable Variable Variable  Calc    0x03
```

**Details:**
- **STX**: `0x02` - Start of Text
- **DeviceID**: `0x70` - Fixed device ID
- **Sequence**: `0x00-0xFF` - Packet sequence number
- **Command**: `0x10-0xFF` - Control command code
- **Data1-3**: Data depending on command
- **Checksum**: Calculated from payload (Internet checksum with 0x10 offset)
- **ETX**: `0x03` - End of Text

**Command List:**
- `0x10`: AUTO - Enable/disable automatic mode
- `0x20`: ROLL_MOTOR - Turn roll motor on/off
- `0x21`: ROLL_DIRECTION - Manual roll control
- `0x22`: KNEADING_MANUAL - Manual kneading control
- `0x23`: PERCUSSION_MANUAL - Manual percussion control
- `0x30`: KNEADING - Kneading mode
- `0x40`: PERCUSSION - Percussion mode
- `0x50`: COMPRESSION - Compression mode
- `0x60`: COMBINE - Combination mode
- `0x70`: INTENSITY_LEVEL - Adjust intensity
- `0x80`: INCLINE - Raise chair
- `0x90`: RECLINE - Lower chair
- `0xA0`: FORWARD - Push chair forward
- `0xB0`: BACKWARD - Pull chair backward
- `0xFF`: DISCONNECT - Disconnect

See [Firmware README](OpenSmartControl_Firmware/README.md) for details about each command.

## 🛠️ Development

### Code Structure

#### Components

- `HomeScreen.js`: Main screen with QR scanner and manual connect
- `ControlScreen.js`: Massage control screen
- `ChairPositionControl.js`: Chair position control
- `QRScanner.js`: QR code scanner component
- `ManualConnect.js`: Manual connection component
- `BleDebugPanel.js`: BLE debug panel
- `NavigationBar.js`: Navigation bar

#### Services

- `BleService.js`: Service handling BLE connection and communication
- `PermissionService.js`: Manages access permissions (Bluetooth, Camera, Location)

#### Store (Redux)

- `bleSlice.js`: Manages BLE state, device and system state
- `navigationSlice.js`: Manages screen navigation

#### Utils

- `packetCommands.js`: Creates and processes BLE packets
- `constants.js`: Constants
- `helpers.js`: Helper functions
- `validators.js`: Validation functions

### Utility Scripts

#### clean.sh (Android)

```bash
./clean.sh -h    # Show help
./clean.sh -c    # Clean build artifacts
./clean.sh -d    # Build debug APK
./clean.sh -r    # Build release APK with JS bundle
./clean.sh -i    # Install APK to device
./clean.sh -a    # Clean + install dependencies + build debug
```

### Debugging

#### BLE Debug Panel

Use `BleDebugPanel` component to debug BLE:

```javascript
import BleDebugPanel from '../components/BleDebugPanel';

// In render
<BleDebugPanel />
```

#### BLE Service Debug Mode

```javascript
import BleService from '../services/BleService';

// Enable debug mode
BleService.setDebugMode(true);
```

### Testing

```bash
# Run tests
npm test

# Run tests with coverage
npm test -- --coverage
```

## 🔐 Permissions

### Android

- `BLUETOOTH`
- `BLUETOOTH_ADMIN`
- `BLUETOOTH_SCAN` (Android 12+)
- `BLUETOOTH_CONNECT` (Android 12+)
- `ACCESS_FINE_LOCATION`
- `CAMERA`

### iOS

- `NSBluetoothAlwaysUsageDescription`
- `NSCameraUsageDescription`

## 🔍 Troubleshooting

### Bluetooth Connection Errors

1. Check if Bluetooth is enabled
2. Ensure ESP32 is near the device
3. Restart the application if needed
4. Check if HM10 module has been reset
5. Use BleDebugPanel to check status

### Redux/Hermes Errors

1. Check `combinedReducer` in `bleSlice.js`
2. Ensure state initialization is correct
3. Use plain functions instead of Immer for Hermes compatibility

### Camera Errors

1. Grant camera permission in Settings
2. Use manual QR input feature

### Permission Errors

1. Check permissions in AndroidManifest.xml (Android) or Info.plist (iOS)
2. Grant permissions manually in Settings

## 📊 Version

### Applications

- **Android**: 1.0.1
- **iOS**: 0.0.1

### Firmware

- **Version**: V1 Release Ver0003 DEV_PRO
- **Board**: V1 Release

### React Native

- **Version**: 0.80.0

## 👥 Contributing

We welcome all contributions! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Create a Pull Request

### Contribution Guidelines

- Follow the existing code style
- Write tests for new code
- Update documentation when needed
- Write clear commit messages

## 📄 License

MIT License - See LICENSE file for more details.

## 📞 Support

If you encounter issues:

1. Check the [Troubleshooting](#troubleshooting) section
2. Review detailed documentation in the project folders
3. Create an issue on the GitHub repository

## 📋 App Store Review - Hardware Compatibility Information

### Response to Apple App Review Questions (Guideline 2.1)

**Q: Does your app work with a specific brand of hardware only?**  
**A:** No. This app does NOT work with a specific brand of hardware only. The application is designed to work with generic, open-source hardware components that follow the OpenSmartControl communication protocol. Any device implementing the documented BLE protocol can work with this app.

**Q: If yes, what is the brand name of the designated hardware that is used by your app?**  
**A:** N/A - The app does not require a specific brand. It works with generic ESP32 development boards and HM10 BLE modules available from multiple manufacturers.

**Q: What is the company name that owns the rights of the designated hardware?**  
**A:** N/A - No specific company owns exclusive rights. The app uses standard, commercially available components:
- ESP32 boards (Espressif Systems - open-source hardware)
- HM10 BLE modules (generic BLE modules available from multiple manufacturers)
- Standard motor controllers and sensors

**Q: Please provide documentation demonstrating your right to use the third-party content in your app.**  
**A:** This app uses only open-source components and protocols:
- ESP32 is open-source hardware by Espressif Systems
- HM10 is a generic BLE module specification
- The communication protocol is open-source and documented in this repository
- All firmware code is open-source (MIT License)
- No proprietary third-party content is used
- No brand names or trademarks are used in user-facing content

**Q: Where can users purchase the designated hardware?**  
**A:** Users can purchase compatible hardware components from various sources:
- **Online Retailers**: Amazon, AliExpress, eBay
- **Component Distributors**: DigiKey, Mouser, Adafruit, SparkFun, Arrow Electronics
- **Local Electronics Stores**: Any store selling ESP32 and BLE modules
- **Direct from Manufacturers**: Espressif Systems (ESP32), various HM10 module manufacturers

Users need to:
1. Purchase generic ESP32 board and HM10 BLE module
2. Flash the open-source firmware (provided in this repository) onto the ESP32
3. Connect the app to the device via Bluetooth

**Additional Information:**
- The app uses generic device names (e.g., "MASSAGE_DEVICE") - no brand-specific naming
- The communication protocol is fully documented and open-source
- Users can build their own compatible hardware using the provided schematics and firmware
- No licensing or authorization from any specific hardware manufacturer is required

## 🎯 Roadmap

### Features Under Development

- [ ] Multi-device support
- [ ] Preset massage programs
- [ ] Usage statistics
- [ ] Firmware OTA updates
- [ ] Voice control integration

### Planned Improvements

- [ ] Improved error recovery
- [ ] Better connection stability
- [ ] Enhanced UI/UX
- [ ] Performance optimizations

---

**Last Updated**: 2025-01-07  
**Compatibility**: React Native 0.80+, Android 7+, iOS 12+  
**Firmware**: ESP32 with HM10 BLE Module
