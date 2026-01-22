# 📱 QR Code Format - VPG Massage Chair App

## ✅ **Format QR Code được hỗ trợ**

### **Format đơn giản (Khuyên dùng)** ⭐

```json
{
  "type": "massage_device",
  "name": "VPG_MASSAGE"
}
```

**Cách hoạt động:**
1. User quét QR → App tìm device có tên "VPG_MASSAGE"
2. Kết nối trực tiếp (không cần pairing với BLE thông thường)
3. Hoàn tất!

---

### **Format với Pair Code** (Chỉ khi ESP32 bật encryption)

```json
{
  "type": "massage_device",
  "name": "VPG_MASSAGE",
  "paircode": "000000"
}
```

**Lưu ý:** 
- Pair code chỉ cần nếu ESP32 bật **BLE encryption**
- User vẫn phải **nhập thủ công** khi OS yêu cầu
- 95% trường hợp **KHÔNG cần** pair code (BLE thông thường)

---

## 🔧 **Setup ESP32**

### **Code ESP32 mẫu (Arduino):**

```cpp
#include <BLEDevice.h>
#include <BLEServer.h>

void setup() {
  Serial.begin(115200);
  
  // Đặt tên BLE device (KHỚP với QR code)
  BLEDevice::init("VPG_MASSAGE");
  
  // Tạo BLE server
  BLEServer *pServer = BLEDevice::createServer();
  
  // Setup service và characteristics
  // Service UUID: 0000ffe0-0000-1000-8000-00805f9b34fb
  // Characteristic UUID: 0000ffe1-0000-1000-8000-00805f9b34fb
  
  BLEService *pService = pServer->createService("0000ffe0-0000-1000-8000-00805f9b34fb");
  
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    "0000ffe1-0000-1000-8000-00805f9b34fb",
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  
  pService->start();
  
  // Bắt đầu advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  
  Serial.println("BLE device ready: VPG_MASSAGE");
}

void loop() {
  delay(1000);
}
```

---

## 📋 **Các trường trong QR Code**

| Trường | Bắt buộc | Mặc định | Mô tả |
|--------|----------|----------|-------|
| `type` | ✅ Có | - | Loại thiết bị: `"massage_device"` |
| `name` | ✅ Có | - | Tên BLE device để tìm kiếm |
| `paircode` | ❌ Không | - | Mã ghép nối 6 số (chỉ khi cần encryption) |

**Không cần:**
- ❌ `mac` - MAC address (app tự động phát hiện)
- ❌ `uuid` - Service UUID (app đã có sẵn)

---

## 🧪 **Test QR Code**

### **Bước 1: Tạo QR Code**

Sử dụng tool online: https://www.qr-code-generator.com/

**Input:**
```json
{"type":"massage_device","name":"VPG_MASSAGE"}
```

**Output:** QR Code image → In ra và dán lên ghế massage

### **Bước 2: Test trên App**

1. Mở app → "Scan QR Code"
2. Quét QR hoặc tap "⌨️ Input QR"
3. Tap "Use Sample Data" để test
4. App sẽ tìm device "VPG_MASSAGE" và kết nối

---

## 🎯 **Ví dụ thực tế**

### **Ghế massage A1:**
```json
{"type":"massage_device","name":"VPG_MASSAGE_A1"}
```

### **Ghế massage B2:**
```json
{"type":"massage_device","name":"VPG_MASSAGE_B2"}
```

### **Ghế testing:**
```json
{"type":"massage_device","name":"TEST_DEVICE"}
```

---

## ⚠️ **Lưu ý quan trọng**

### **1. Tên BLE device phải KHỚP**

```cpp
// ESP32 Code
BLEDevice::init("VPG_MASSAGE");  // ← Phải khớp với QR
```

```json
// QR Code
{"type":"massage_device","name":"VPG_MASSAGE"}  // ← Phải khớp với ESP32
```

### **2. BLE thường KHÔNG cần pairing**

- ✅ BLE (Bluetooth Low Energy) mặc định **không yêu cầu pairing**
- ✅ Kết nối trực tiếp, nhanh chóng
- ⚠️ Chỉ cần pairing nếu ESP32 **bật encryption** (hiếm gặp)

### **3. Case-insensitive**

App tìm kiếm không phân biệt chữ hoa/thường:
- `VPG_MASSAGE` = `vpg_massage` = `Vpg_Massage` ✅

---

## 🐛 **Troubleshooting**

### **Lỗi: "Device VPG_MASSAGE not found"**

**Nguyên nhân:**
- Tên BLE trên ESP32 không khớp
- ESP32 chưa bật
- ESP32 ở quá xa

**Giải pháp:**
1. Check tên BLE: `BLEDevice::init("VPG_MASSAGE")`
2. Đảm bảo ESP32 đang chạy
3. Di chuyển gần ESP32

### **Lỗi: "Invalid QR code format"**

**Nguyên nhân:**
- QR code không đúng định dạng JSON
- Thiếu trường `name`

**Giải pháp:**
1. Check JSON format: `{"type":"massage_device","name":"VPG_MASSAGE"}`
2. Đảm bảo có trường `name`

---

## 📖 **Tài liệu chi tiết**

Xem thêm trong folder backup:
- `MSMoblieApp_Fix_demo_03102025_ver2_Bk/QR_NAME_PAIRCODE_GUIDE.md` - Hướng dẫn đầy đủ
- `MSMoblieApp_Fix_demo_03102025_ver2_Bk/BLE_PAIRING_EXPLAINED.md` - Giải thích về BLE Pairing

---

## ✅ **Tóm tắt**

**QR Code đơn giản nhất:**
```json
{"type":"massage_device","name":"VPG_MASSAGE"}
```

**ESP32 setup:**
```cpp
BLEDevice::init("VPG_MASSAGE");
```

**Kết quả:** Kết nối trực tiếp, không cần MAC, UUID, hay pairing! 🚀

---

**Cập nhật:** 2025-10-07  
**Version:** 2.0 - Simplified QR Format (Name-based only)

