# OpenSmartControl - Hệ Thống Điều Khiển Ghế Massage Thông Minh

Hệ thống điều khiển ghế massage hoàn chỉnh bao gồm ứng dụng di động (Android & iOS), firmware ESP32, và tài liệu phần cứng. Hệ thống sử dụng Bluetooth Low Energy (BLE) để giao tiếp giữa ứng dụng di động và thiết bị điều khiển.

## 📋 Mục Lục

- [Tổng Quan](#tổng-quan)
- [Cấu Trúc Dự Án](#cấu-trúc-dự-án)
- [Tính Năng Chính](#tính-năng-chính)
- [Yêu Cầu Hệ Thống](#yêu-cầu-hệ-thống)
- [Cài Đặt](#cài-đặt)
- [Sử Dụng](#sử-dụng)
- [Tài Liệu Chi Tiết](#tài-liệu-chi-tiết)
- [Phát Triển](#phát-triển)
- [Đóng Góp](#đóng-góp)

## 🎯 Tổng Quan

OpenSmartControl là một hệ thống điều khiển ghế massage thông minh với các thành phần:

- **Ứng dụng Android**: React Native app cho Android
- **Ứng dụng iOS**: React Native app cho iOS  
- **Firmware ESP32**: Code điều khiển cho board ESP32
- **Tài Liệu Phần Cứng**: Schematics và PCB layouts

### Kiến Trúc Hệ Thống

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

## 📁 Cấu Trúc Dự Án

```
OpenSmartControl/
├── OpenSmartControl_Android/      # Ứng dụng Android
│   ├── src/                        # Source code
│   │   ├── components/             # UI components
│   │   ├── services/               # BLE & Permission services
│   │   ├── store/                  # Redux store
│   │   ├── styles/                 # Styles & constants
│   │   └── utils/                   # Utilities & helpers
│   ├── android/                    # Android native code
│   ├── ios/                        # iOS native code
│   ├── 01_Firmware_Board_V1_Release_Ver0003_DEV_PRO/  # Firmware source
│   └── Readme.md                   # Tài liệu Android app
│
├── OpenSmartControl_IOS/           # Ứng dụng iOS
│   ├── src/                        # Source code (tương tự Android)
│   ├── ios/                        # iOS native code
│   ├── android/                    # Android native code
│   └── Readme.md                   # Tài liệu iOS app
│
├── OpenSmartControl_Firmware/       # Firmware releases
│   ├── OpenSmartControl_Release.hex # Firmware hex file
│   └── README.md                   # Tài liệu firmware
│
└── OpenSmartControl_Harware/        # Tài liệu phần cứng
    ├── OpenSmartControl.pdf         # Schematic/PCB documentation
    └── plots/                       # Gerber files
```

## ✨ Tính Năng Chính

### Ứng Dụng Di Động

- ✅ **Quét QR Code**: Kết nối nhanh với thiết bị ESP32 bằng cách quét mã QR
- ✅ **Kết nối thủ công**: Tìm kiếm và kết nối với thiết bị Bluetooth
- ✅ **Điều khiển massage**: Giao diện điều khiển đầy đủ các chức năng massage
- ✅ **Điều khiển vị trí ghế**: Điều khiển nâng/hạ, tiến/lùi ghế
- ✅ **Giao diện tối ưu**: UI/UX được tinh chỉnh cho thiết bị di động
- ✅ **Heartbeat mechanism**: Duy trì kết nối BLE ổn định
- ✅ **State management**: Quản lý trạng thái với Redux Toolkit
- ✅ **Error handling**: Xử lý lỗi toàn diện với thông báo rõ ràng
- ✅ **Connection persistence**: Lưu và tự động kết nối lại thiết bị đã kết nối trước đó
- ✅ **Debug panel**: Công cụ debug BLE để troubleshooting

### Firmware ESP32

- ✅ **BLE Communication**: Giao tiếp qua HM10 module với UART
- ✅ **Packet Protocol**: Định dạng packet có checksum để đảm bảo tính toàn vẹn
- ✅ **Command Processing**: Xử lý 15+ lệnh điều khiển khác nhau
- ✅ **Safety Management**: Quản lý an toàn với sensors và giới hạn
- ✅ **Motor Control**: Điều khiển các motor massage và vị trí ghế
- ✅ **Auto Programs**: Các chương trình massage tự động (Kneading, Percussion, Compression, Combine)
- ✅ **Manual Control**: Điều khiển thủ công với chế độ nhấn giữ
- ✅ **Home Sequence**: Tự động về vị trí ban đầu khi cần

## 🔧 Yêu Cầu Hệ Thống

### Ứng Dụng Di Động

- **React Native**: >= 0.70
- **Node.js**: >= 16 (khuyến nghị >= 18)
- **Android SDK**: Cho Android development
- **Xcode**: >= 12.0 cho iOS development
- **CocoaPods**: Cho iOS dependencies

### Firmware

- **Arduino IDE**: >= 1.8.x
- **ESP32 Board Support**: ESP32 Arduino Core
- **Libraries**: 
  - BLE libraries cho ESP32
  - UART libraries

### Phần Cứng

- **ESP32 Development Board**
- **HM10 BLE Module**
- **Motor Controllers**
- **Sensors** (limit switches, etc.)

## 🚀 Cài Đặt

### 1. Clone Repository

```bash
git clone <repository-url>
cd OpenSmartControl
```

### 2. Cài Đặt Ứng Dụng Android

```bash
cd OpenSmartControl_Android
npm install
# hoặc
yarn install

# Cài đặt pods cho iOS (nếu cần)
cd ios && pod install && cd ..
```

### 3. Cài Đặt Ứng Dụng iOS

```bash
cd OpenSmartControl_IOS
npm install
# hoặc
yarn install

# Cài đặt pods
cd ios && pod install && cd ..
```

### 4. Chạy Ứng Dụng

#### Android

```bash
# Chạy Metro bundler
npm start

# Chạy trên Android (terminal khác)
npm run android
# hoặc
npx react-native run-android
```

#### iOS

```bash
# Chạy Metro bundler
npm start

# Chạy trên iOS (terminal khác)
npm run ios
# hoặc
npx react-native run-ios
```

### 5. Build Release

#### Android APK

```bash
cd OpenSmartControl_Android

# Sử dụng script clean.sh
./clean.sh -r  # Build release APK
./clean.sh -d  # Build debug APK
./clean.sh -c  # Clean build artifacts
./clean.sh -h  # Xem help
```

#### iOS Archive

```bash
cd OpenSmartControl_IOS

# Build bundle
npm run build:ios:bundle

# Sau đó build trong Xcode
open ios/MassageChairControl.xcworkspace
```

## 📱 Sử Dụng

### Kết Nối Thiết Bị

#### Qua QR Code (Khuyên dùng)

1. Mở ứng dụng
2. Chọn "Scan QR Code"
3. Quét mã QR trên thiết bị hoặc nhập thủ công
4. Ứng dụng sẽ tự động tìm và kết nối

**Định dạng QR Code:**
```json
{
  "type": "massage_device",
  "name": "MASSAGE_DEVICE"
}
```

#### Kết Nối Thủ Công

1. Mở ứng dụng
2. Chọn "Manual Connect"
3. Chờ ứng dụng quét thiết bị BLE
4. Chọn thiết bị từ danh sách
5. Kết nối

### Điều Khiển Massage

#### Chế Độ Tự Động (AUTO)

- **AUTO DEFAULT**: Chương trình massage mặc định với roll motor luôn bật
- **KNEADING**: Kỹ thuật nhào
- **PERCUSSION**: Kỹ thuật gõ
- **COMPRESSION**: Kỹ thuật nén
- **COMBINE**: Kết hợp nhiều kỹ thuật

#### Chế Độ Thủ Công (MANUAL)

- **Roll Motor**: Điều khiển roll motor lên/xuống
- **Kneading**: Bật/tắt motor kneading
- **Percussion**: Bật/tắt motor percussion
- **Intensity**: Điều chỉnh cường độ (LOW/HIGH)

#### Điều Khiển Vị Trí Ghế

- **RECLINE**: Hạ ghế xuống
- **INCLINE**: Nâng ghế lên
- **FORWARD**: Đẩy ghế về phía trước
- **BACKWARD**: Kéo ghế về phía sau

## 📚 Tài Liệu Chi Tiết

### Ứng Dụng Di Động

- **[Android App README](OpenSmartControl_Android/Readme.md)**: Tài liệu chi tiết ứng dụng Android
- **[iOS App README](OpenSmartControl_IOS/Readme.md)**: Tài liệu chi tiết ứng dụng iOS
- **[QR Code Format](OpenSmartControl_Android/QR_CODE_FORMAT.md)**: Hướng dẫn định dạng QR Code
- **[BLE Improvements](OpenSmartControl_Android/BLE_IMPROVEMENTS_SUMMARY.md)**: Tóm tắt cải tiến BLE
- **[BLE Connection Analysis](OpenSmartControl_Android/BLE_Connection_Analysis.md)**: Phân tích kết nối BLE
- **[Disconnect Flow](OpenSmartControl_Android/DISCONNECT_FLOW.md)**: Luồng ngắt kết nối

### iOS Specific

- **[App Store Compliance](OpenSmartControl_IOS/APP_STORE_COMPLIANCE.md)**: Hướng dẫn tuân thủ App Store
- **[iOS Warnings Guide](OpenSmartControl_IOS/IOS_WARNINGS_GUIDE.md)**: Hướng dẫn xử lý warnings iOS
- **[Metadata Changes](OpenSmartControl_IOS/METADATA_CHANGES.md)**: Thay đổi metadata

### Firmware

- **[Firmware README](OpenSmartControl_Firmware/README.md)**: Tài liệu chi tiết firmware
- **[Protocol Documentation](OpenSmartControl_Android/src/utils/PROTOCOL_README.md)**: Tài liệu giao thức truyền dữ liệu

### Giao Thức Truyền Dữ Liệu

Hệ thống sử dụng giao thức packet với cấu trúc:

```
[STX] [DeviceID] [Sequence] [Command] [Data1] [Data2] [Data3] [Checksum] [ETX]
 0x02    0x70      Variable    Variable  Variable Variable Variable  Calc    0x03
```

**Chi tiết:**
- **STX**: `0x02` - Start of Text
- **DeviceID**: `0x70` - ID thiết bị cố định
- **Sequence**: `0x00-0xFF` - Số thứ tự packet
- **Command**: `0x10-0xFF` - Mã lệnh điều khiển
- **Data1-3**: Dữ liệu tùy theo lệnh
- **Checksum**: Tính toán từ payload (Internet checksum với offset 0x10)
- **ETX**: `0x03` - End of Text

**Danh sách lệnh:**
- `0x10`: AUTO - Bật/tắt chế độ tự động
- `0x20`: ROLL_MOTOR - Bật/tắt roll motor
- `0x21`: ROLL_DIRECTION - Điều khiển roll thủ công
- `0x22`: KNEADING_MANUAL - Điều khiển kneading thủ công
- `0x23`: PERCUSSION_MANUAL - Điều khiển percussion thủ công
- `0x30`: KNEADING - Chế độ kneading
- `0x40`: PERCUSSION - Chế độ percussion
- `0x50`: COMPRESSION - Chế độ compression
- `0x60`: COMBINE - Chế độ kết hợp
- `0x70`: INTENSITY_LEVEL - Điều chỉnh cường độ
- `0x80`: INCLINE - Nâng ghế lên
- `0x90`: RECLINE - Hạ ghế xuống
- `0xA0`: FORWARD - Đẩy ghế về trước
- `0xB0`: BACKWARD - Kéo ghế về sau
- `0xFF`: DISCONNECT - Ngắt kết nối

Xem [Firmware README](OpenSmartControl_Firmware/README.md) để biết chi tiết về từng lệnh.

## 🛠️ Phát Triển

### Cấu Trúc Code

#### Components

- `HomeScreen.js`: Màn hình chính với QR scanner và manual connect
- `ControlScreen.js`: Màn hình điều khiển massage
- `ChairPositionControl.js`: Điều khiển vị trí ghế
- `QRScanner.js`: Component quét QR code
- `ManualConnect.js`: Component kết nối thủ công
- `BleDebugPanel.js`: Panel debug BLE
- `NavigationBar.js`: Thanh điều hướng

#### Services

- `BleService.js`: Service xử lý BLE connection và communication
- `PermissionService.js`: Quản lý quyền truy cập (Bluetooth, Camera, Location)

#### Store (Redux)

- `bleSlice.js`: Quản lý trạng thái BLE, thiết bị và hệ thống
- `navigationSlice.js`: Quản lý điều hướng màn hình

#### Utils

- `packetCommands.js`: Tạo và xử lý BLE packets
- `constants.js`: Các hằng số
- `helpers.js`: Hàm hỗ trợ
- `validators.js`: Validation functions

### Scripts Tiện Ích

#### clean.sh (Android)

```bash
./clean.sh -h    # Hiển thị help
./clean.sh -c    # Clean build artifacts
./clean.sh -d    # Build debug APK
./clean.sh -r    # Build release APK với JS bundle
./clean.sh -i    # Install APK lên thiết bị
./clean.sh -a    # Clean + install dependencies + build debug
```

### Debugging

#### BLE Debug Panel

Sử dụng `BleDebugPanel` component để debug BLE:

```javascript
import BleDebugPanel from '../components/BleDebugPanel';

// Trong render
<BleDebugPanel />
```

#### BLE Service Debug Mode

```javascript
import BleService from '../services/BleService';

// Bật debug mode
BleService.setDebugMode(true);
```

### Testing

```bash
# Chạy tests
npm test

# Chạy tests với coverage
npm test -- --coverage
```

## 🔐 Quyền Truy Cập

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

### Lỗi Kết Nối Bluetooth

1. Kiểm tra Bluetooth đã bật
2. Đảm bảo ESP32 ở gần thiết bị
3. Restart ứng dụng nếu cần
4. Kiểm tra HM10 module đã được reset
5. Sử dụng BleDebugPanel để kiểm tra trạng thái

### Lỗi Redux/Hermes

1. Kiểm tra `combinedReducer` trong `bleSlice.js`
2. Đảm bảo state initialization đúng
3. Sử dụng plain functions thay vì Immer cho Hermes compatibility

### Lỗi Camera

1. Cấp quyền camera trong Settings
2. Sử dụng tính năng nhập QR thủ công

### Lỗi Permissions

1. Kiểm tra permissions trong AndroidManifest.xml (Android) hoặc Info.plist (iOS)
2. Cấp quyền thủ công trong Settings

## 📊 Phiên Bản

### Ứng Dụng

- **Android**: 1.0.1
- **iOS**: 0.0.1

### Firmware

- **Version**: V1 Release Ver0003 DEV_PRO
- **Board**: V1 Release

### React Native

- **Version**: 0.80.0

## 👥 Đóng Góp

Chúng tôi hoan nghênh mọi đóng góp! Vui lòng:

1. Fork repository
2. Tạo feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Tạo Pull Request

### Quy Tắc Đóng Góp

- Tuân thủ code style hiện tại
- Viết tests cho code mới
- Cập nhật tài liệu khi cần
- Viết commit messages rõ ràng

## 📄 License

MIT License - Xem file LICENSE để biết thêm chi tiết.

## 📞 Hỗ Trợ

Nếu gặp vấn đề:

1. Kiểm tra [Troubleshooting](#troubleshooting) section
2. Xem các tài liệu chi tiết trong thư mục dự án
3. Tạo issue trên GitHub repository

## 🎯 Roadmap

### Tính Năng Đang Phát Triển

- [ ] Multi-device support
- [ ] Preset massage programs
- [ ] Usage statistics
- [ ] Firmware OTA updates
- [ ] Voice control integration

### Cải Tiến Đang Lên Kế Hoạch

- [ ] Improved error recovery
- [ ] Better connection stability
- [ ] Enhanced UI/UX
- [ ] Performance optimizations

---

**Cập nhật cuối**: 2025-01-07  
**Tương thích**: React Native 0.80+, Android 7+, iOS 12+  
**Firmware**: ESP32 với HM10 BLE Module
