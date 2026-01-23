# ⏱️ Tăng Thời Gian Searching for Device

## ✅ **Đã hoàn thành**

Tăng thời gian scanning từ **10 giây** lên **20 giây** ở tất cả các nơi:

---

## 📋 **Chi tiết thay đổi**

### **1. BleService.js** (Line 36)
```javascript
// Trước:
SCAN_TIMEOUT: 10000,        // 10 seconds device scan

// Sau:
SCAN_TIMEOUT: 20000,        // 20 seconds device scan (increased from 10s)
```

### **2. ManualConnect.js** (Line 82-86)
```javascript
// Trước:
setTimeout(async () => {
  const peripherals = await BleService.getDiscoveredPeripherals();
  dispatch(setScanResults(peripherals));
  dispatch(setScanning(false));
}, 10000); // Scan for 10 seconds

// Sau:
setTimeout(async () => {
  const peripherals = await BleService.getDiscoveredPeripherals();
  dispatch(setScanResults(peripherals));
  dispatch(setScanning(false));
}, 20000); // Scan for 20 seconds (increased from 10s)
```

### **3. QRScanner.js** (Line 329)
```javascript
// Trước:
await BleService.scanDevices(10000); // Scan for 10 seconds

// Sau:
await BleService.scanDevices(20000); // Scan for 20 seconds (increased from 10s)
```

### **4. QRScanner.js** (Line 343)
```javascript
// Trước:
await new Promise(resolve => setTimeout(resolve, 5000));

// Sau:
await new Promise(resolve => setTimeout(resolve, 10000)); // Increased from 5s to 10s
```

---

## 🎯 **Tác động**

### **Trước khi thay đổi:**
- ⏱️ **BLE scan timeout:** 10 giây
- ⏱️ **ManualConnect timeout:** 10 giây  
- ⏱️ **QRScanner discovery:** 10 giây
- ⏱️ **Progress animation:** 5 giây

### **Sau khi thay đổi:**
- ⏱️ **BLE scan timeout:** 20 giây ✅
- ⏱️ **ManualConnect timeout:** 20 giây ✅
- ⏱️ **QRScanner discovery:** 20 giây ✅
- ⏱️ **Progress animation:** 10 giây ✅

---

## 📱 **Các màn hình bị ảnh hưởng**

### **1. ManualConnect Screen**
- **Trước:** Scan 10 giây → hiển thị danh sách
- **Sau:** Scan 20 giây → hiển thị danh sách
- **UI:** "Scanning..." hiển thị lâu hơn

### **2. QRScanner Screen**
- **Trước:** Scan 10 giây → tìm device
- **Sau:** Scan 20 giây → tìm device
- **UI:** Progress bar chạy lâu hơn (10 giây thay vì 5 giây)

### **3. BleService (Core)**
- **Trước:** Tất cả scan operations timeout sau 10 giây
- **Sau:** Tất cả scan operations timeout sau 20 giây
- **Ảnh hưởng:** Toàn bộ app

---

## 🔧 **Cấu hình timeout**

### **BLE_CONFIG trong BleService.js:**
```javascript
const BLE_CONFIG = {
  SCAN_TIMEOUT: 20000,        // 20 seconds device scan
  CONNECTION_TIMEOUT: 15000,  // 15 seconds connection timeout (không đổi)
  FRAGMENT_TIMEOUT: 5000,     // 5 seconds fragment receive timeout (không đổi)
  MAX_BUFFER_SIZE: 2000,      // Maximum buffer size (không đổi)
};
```

### **ManualConnect.js:**
```javascript
// Scan timeout
setTimeout(async () => {
  // ... get results after 20 seconds
}, 20000); // Scan for 20 seconds
```

### **QRScanner.js:**
```javascript
// Device discovery timeout
await BleService.scanDevices(20000); // Scan for 20 seconds

// Progress animation timeout
await new Promise(resolve => setTimeout(resolve, 10000)); // 10 seconds
```

---

## 🧪 **Cách test**

### **Test 1: ManualConnect**
1. Mở app → Manual Connect
2. Tap "Scan Devices"
3. ✅ Kiểm tra: "Scanning..." hiển thị **20 giây**
4. ✅ Sau 20 giây: Hiển thị danh sách devices

### **Test 2: QRScanner**
1. Mở app → Scan QR
2. Scan QR code với device name
3. ✅ Kiểm tra: Progress bar chạy **10 giây**
4. ✅ Tổng thời gian discovery: **20 giây**

### **Test 3: BleService Core**
1. Bất kỳ scan operation nào
2. ✅ Kiểm tra: Timeout sau **20 giây** thay vì 10 giây

---

## ⚡ **Lợi ích**

### **1. Tăng khả năng tìm thấy device**
- **Trước:** 10 giây có thể không đủ
- **Sau:** 20 giây = gấp đôi thời gian

### **2. Cải thiện trải nghiệm người dùng**
- **Ít lỗi "Device not found"**
- **Tăng tỷ lệ kết nối thành công**
- **Đặc biệt hữu ích với ESP32 BLE**

### **3. Phù hợp với BLE characteristics**
- **BLE scan thường cần thời gian lâu hơn**
- **ESP32 có thể advertise không liên tục**
- **20 giây = thời gian hợp lý**

---

## 📊 **So sánh**

| Component | Trước | Sau | Tăng |
|-----------|-------|-----|------|
| **BleService.SCAN_TIMEOUT** | 10s | 20s | +100% |
| **ManualConnect timeout** | 10s | 20s | +100% |
| **QRScanner discovery** | 10s | 20s | +100% |
| **Progress animation** | 5s | 10s | +100% |

---

## 🎯 **Kết quả**

### ✅ **Hoàn thành:**
- [x] Tăng SCAN_TIMEOUT trong BleService.js
- [x] Tăng timeout trong ManualConnect.js  
- [x] Tăng timeout trong QRScanner.js
- [x] Tăng progress animation timeout
- [x] Kiểm tra linter errors (không có lỗi)

### 📈 **Cải thiện:**
- **Gấp đôi thời gian scanning** (10s → 20s)
- **Tăng khả năng tìm thấy device**
- **Giảm lỗi "Device not found"**
- **Cải thiện UX cho BLE connection**

---

## 🔄 **Rollback (nếu cần)**

Để rollback về 10 giây:
```javascript
// BleService.js
SCAN_TIMEOUT: 10000,        // 10 seconds device scan

// ManualConnect.js  
}, 10000); // Scan for 10 seconds

// QRScanner.js
await BleService.scanDevices(10000); // Scan for 10 seconds
await new Promise(resolve => setTimeout(resolve, 5000)); // 5 seconds
```

---

**Cập nhật:** 2025-10-07  
**Version:** 1.0 - Scan Timeout Increase (10s → 20s)

