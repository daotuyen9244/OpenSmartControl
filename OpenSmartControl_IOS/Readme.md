# Ứng dụng Điều khiển Máy Massage ESP32

Ứng dụng React Native để điều khiển máy massage ESP32 qua Bluetooth Low Energy (BLE) với giao diện tối ưu và trải nghiệm người dùng nâng cao.

## 📱 Tính năng chính

- **Quét QR Code**: Kết nối nhanh với thiết bị ESP32 bằng cách quét mã QR
- **Kết nối thủ công**: Tìm kiếm và kết nối với thiết bị Bluetooth
- **Điều khiển massage**: Giao diện điều khiển đầy đủ các chức năng massage
- **Giao diện tối ưu**: UI/UX được tinh chỉnh cho thiết bị di động với layout gọn gàng
- **Heartbeat mechanism**: Duy trì kết nối BLE ổn định
- **State management**: Quản lý trạng thái với Redux Toolkit
- **Error handling**: Xử lý lỗi toàn diện với thông báo rõ ràng

## 🔧 Cài đặt

### Yêu cầu hệ thống
- React Native >= 0.70
- Node.js >= 16
- Android SDK (cho Android)
- Xcode (cho iOS)

### Cài đặt dependencies
```bash
npm install
```

hoặc

```bash
yarn install
```

### Cài đặt pods (iOS)
```bash
cd ios && pod install
```

## 🚀 Chạy ứng dụng

### Android
```bash
npx react-native run-android
```

### iOS
```bash
npx react-native run-ios
```

### Build Release APK
```bash
# Sử dụng script clean.sh
./clean.sh -r  # Build release APK
./clean.sh -d  # Build debug APK
./clean.sh -c  # Clean build artifacts
./clean.sh -h  # Xem help
```

## 📁 Cấu trúc thư mục

```
src/
├── components/          # Các component UI
│   ├── ControlScreen.js      # Màn hình điều khiển chính
│   ├── ChairPositionControl.js # Điều khiển vị trí ghế
│   ├── BleDebugPanel.js      # Panel debug BLE
│   ├── HomeScreen.js         # Màn hình chính
│   ├── ManualConnect.js      # Kết nối thủ công
│   ├── NavigationBar.js      # Thanh điều hướng
│   └── QRScanner.js          # Quét QR code
├── services/            # Các service xử lý logic
│   ├── BleService.js         # Service BLE chính
│   └── PermissionService.js  # Quản lý quyền truy cập
├── store/               # Redux store
│   ├── bleSlice.js           # Slice quản lý BLE
│   ├── navigationSlice.js    # Slice điều hướng
│   └── index.js              # Cấu hình store
├── styles/              # Styles và constants
│   ├── Colors.js             # Định nghĩa màu sắc
│   ├── commonStyles.js       # Styles chung
│   └── dimensions.js         # Kích thước màn hình
├── utils/               # Utilities
│   ├── constants.js          # Hằng số
│   ├── helpers.js            # Hàm hỗ trợ
│   ├── packetCommands.js     # Lệnh BLE packet
│   └── validators.js         # Validation
└── MainApp.js           # Component gốc

01_Firmware_Board_V1_Release_Ver0001/
├── Massage_v1_hardware.h     # Header file phần cứng
├── Massage_v1_hardware.cpp   # Implementation phần cứng
├── MessageProcess.cpp        # Xử lý tin nhắn
└── 01_Firmware_Board_V1_Release_Ver0001.ino # Main Arduino sketch
```

## 🔌 Kết nối ESP32

### Định dạng QR Code
```json
{
  "type": "massage_device",
  "name": "ESP32-MASSAGE",
  "mac": "3c:8a:1f:81:a0:9e",
  "uuid": "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
}
```

### BLE Configuration
- **Service UUID**: `6e400001-b5a3-f393-e0a9-e50e24dcca9e`
- **RX Characteristic**: `6e400002-b5a3-f393-e0a9-e50e24dcca9e`
- **TX Characteristic**: `6e400003-b5a3-f393-e0a9-e50e24dcca9e`

### Firmware Features
- **HM10 Reset**: Tự động reset HM10 module khi kết nối/ngắt kết nối
- **BREAK Pin**: Sử dụng PB9 để điều khiển reset HM10
- **Heartbeat**: Gửi heartbeat để duy trì kết nối

## 📋 Lệnh điều khiển

### Lệnh cơ bản
- **AUTO/MANUAL**: Chuyển chế độ điều khiển
- **ROLL/SPOT**: Chọn loại massage (chỉ hoạt động ở chế độ AUTO)
- **LOW/HIGH**: Điều chỉnh cường độ massage

### Lệnh kỹ thuật massage
- **KNEADING**: Kỹ thuật nhào
- **COMBINE**: Kỹ thuật kết hợp
- **PERCUSSION**: Kỹ thuật gõ
- **COMPRESSION**: Kỹ thuật nén

### Lệnh vị trí ghế
- **RECLINE**: Điều chỉnh độ ngả lưng
- **INCLINE**: Điều chỉnh độ nâng chân
- **FORWARD**: Chuyển động tiến
- **BACKWARD**: Chuyển động lùi

## 🎨 Giao diện người dùng

### Layout tối ưu
- **Header compact**: Tên thiết bị và trạng thái kết nối trên cùng 1 dòng
- **Control sections**: Tất cả điều khiển nằm trong 1 khung duy nhất
- **Button sizing**: Kích thước nút đồng nhất (fontSize: 12px, padding: 8px)
- **Spacing optimized**: Khoảng cách giữa các phần được tối ưu
- **Color scheme**: Nền trắng khi không tác động, xanh dương khi tác động

### Responsive Design
- **Single column layout**: Tất cả điều khiển ghế nằm trong 1 cột
- **2x2 grid**: RECLINE/INCLINE và BACKWARD/FORWARD sắp xếp hợp lý
- **Touch-friendly**: Kích thước nút tối ưu cho thiết bị cảm ứng

## 🛠️ Phát triển

### Cấu trúc Redux
- **bleSlice**: Quản lý trạng thái Bluetooth, thiết bị và hệ thống
- **navigationSlice**: Quản lý điều hướng màn hình
- **Hermes compatibility**: Tối ưu cho React Native Hermes engine

### Services
- **BleService**: Xử lý kết nối và giao tiếp BLE với error handling toàn diện
- **PermissionService**: Quản lý quyền truy cập

### Error Handling
```javascript
// Bật debug mode cho BLE
BleService.setDebugMode(true);

// Xử lý lỗi kết nối
BleService.connectToDevice(deviceId)
  .catch(error => {
    console.error('Connection failed:', error);
    // Hiển thị thông báo lỗi cho người dùng
  });
```

### State Management
```javascript
// Truy cập trạng thái hệ thống
const systemState = useSelector(state => state.ble.systemState);

// Kiểm tra chế độ AUTO
if (systemState?.isAutoMode) {
  // Cho phép sử dụng ROLL/SPOT
}
```

## 🔧 Scripts tiện ích

### clean.sh
Script tự động hóa build và deploy:
```bash
./clean.sh -h    # Hiển thị help
./clean.sh -c    # Clean build artifacts
./clean.sh -d    # Build debug APK
./clean.sh -r    # Build release APK với JS bundle
./clean.sh -i    # Install APK lên thiết bị
./clean.sh -a    # Clean + install dependencies + build debug
```

### Features của clean.sh
- **Colored output**: Hiển thị màu sắc rõ ràng
- **Timestamped APKs**: APK có timestamp để quản lý phiên bản
- **JS bundle integration**: Tự động bundle JS cho release build
- **Error handling**: Xử lý lỗi và thông báo rõ ràng

## 📱 Quyền truy cập

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

### Lỗi kết nối Bluetooth
1. Kiểm tra Bluetooth đã bật
2. Đảm bảo ESP32 ở gần thiết bị
3. Restart ứng dụng nếu cần
4. Kiểm tra HM10 module đã được reset

### Lỗi Redux/Hermes
1. Kiểm tra `combinedReducer` trong `bleSlice.js`
2. Đảm bảo state initialization đúng
3. Sử dụng plain functions thay vì Immer cho Hermes compatibility

### Lỗi camera
1. Cấp quyền camera trong Settings
2. Sử dụng tính năng nhập QR thủ công

### Lỗi permissions
1. Kiểm tra permissions trong AndroidManifest.xml
2. Cấp quyền thủ công trong Settings

## 🚀 Performance Optimizations

### BLE Connection
- **Heartbeat mechanism**: Duy trì kết nối ổn định
- **Auto-reconnect disabled**: Người dùng kiểm soát kết nối
- **Error handling**: Xử lý lỗi "Operation was cancelled" gracefully

### UI/UX Improvements
- **Compact layout**: Tối ưu không gian màn hình
- **Consistent spacing**: Khoảng cách đồng nhất
- **Button feedback**: Phản hồi trực quan khi nhấn
- **Warning removal**: Giao diện sạch sẽ không có thông báo warning

### State Management
- **Real-time sync**: Đồng bộ trạng thái real-time giữa BLE và UI
- **Hermes optimization**: Tối ưu cho Hermes JavaScript engine
- **Error recovery**: Khôi phục lỗi tự động

## 📄 License

MIT License - Xem file LICENSE để biết thêm chi tiết.

## 👥 Đóng góp

1. Fork repository
2. Tạo feature branch
3. Commit changes
4. Push to branch
5. Tạo Pull Request

## 📞 Hỗ trợ

Nếu gặp vấn đề, vui lòng tạo issue trên GitHub repository.

---

**Phiên bản**: 2.0.0  
**Cập nhật cuối**: 2024  
**Tương thích**: React Native 0.70+, Android 7+, iOS 12+