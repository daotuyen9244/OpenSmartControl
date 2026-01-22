# 🔌 Disconnect Flow - App ↔️ Firmware

## ✅ **Tổng kết: DISCONNECT đã được hỗ trợ đầy đủ**

Khi user tap nút **Disconnect** trên app:
1. ✅ App gửi lệnh `CMD_DISCONNECT` (0xFF) đến ESP32
2. ✅ ESP32 nhận lệnh và **TẮT TẤT CẢ** (giống timeout 20 phút)
3. ✅ ESP32 reset về **MANUAL mode**
4. ✅ App ngắt kết nối BLE và quay về Home

---

## 📋 **Chi tiết Flow**

### **React Native App Side**

#### **1. packetCommands.js** (Line 266-274)
```javascript
DISCONNECT: {
  deviceId: 0x70,      // Main device
  sequence: 0xFF,      // Disconnect sequence
  command: 0xFF,       // Disconnect command
  data1: 0x00,         // OFF (trigger stopAutoMode)
  data2: 0x00,
  data3: 0x00,
  description: 'Disconnect from device'
}
```

#### **2. BleService.js** (Line 1823-1862)
```javascript
async sendDisconnectCommand() {
  // Gửi DISCONNECT command đến ESP32
  const cmd = getCommand('DISCONNECT');
  const success = await this.sendPacketCommand(
    cmd.deviceId,  // 0x70
    cmd.sequence,  // 0xFF
    cmd.command,   // 0xFF
    cmd.data1,     // 0x00 ← Trigger stopAutoMode()
    cmd.data2,     // 0x00
    cmd.data3      // 0x00
  );
  
  // Chờ device acknowledgment (3 seconds)
  return new Promise((resolve) => {
    const timeout = setTimeout(() => resolve(true), 3000);
    
    // Listen for DISCONNECT_ACK
    const listener = (data, message) => {
      if (message && message.includes('DISCONNECT_ACK')) {
        clearTimeout(timeout);
        resolve(true);
      }
    };
    
    this.notificationListeners.push(listener);
  });
}
```

#### **3. BleService.js - safeDisconnect()** (Line 1869-1963)
```javascript
async safeDisconnect() {
  // Bước 1: Stop heartbeat
  this.stopHeartbeat();
  
  // Bước 2: Gửi lệnh DISCONNECT (nếu thiết bị vẫn kết nối)
  if (isActuallyConnected) {
    await this.sendDisconnectCommand(); // ← GỬI LỆNH ĐẾN ESP32
  }
  
  // Bước 3: Dừng notifications
  await this.stopNotification();
  
  // Bước 4: Ngắt kết nối BLE
  await this.connectedDevice.cancelConnection();
  
  // Bước 5: Reset state
  this.connectedDevice = null;
  dispatch({ type: 'ble/resetBleState' });
}
```

#### **4. ControlScreen.js** (Line 713-738)
```javascript
const handleDisconnect = async () => {
  Alert.alert(
    'Disconnect',
    'Are you sure you want to disconnect from the device?',
    [
      { text: 'Cancel', style: 'cancel' },
      {
        text: 'Disconnect',
        onPress: async () => {
          // Gọi BleService.disconnect() → safeDisconnect()
          await BleService.disconnect(); // ← GỬI CMD_DISCONNECT
          dispatch(resetBleState());
          dispatch(navigateTo('Home'));
        }
      }
    ]
  );
};
```

---

### **ESP32 Firmware Side**

#### **MessageProcess.cpp** (Line 799-824) - MỚI CẬP NHẬT ✨

```cpp
case CMD_DISCONNECT:  // 0xFF - Disconnect
  if (data1 == DATA_OFF) {  // data1 = 0x00
    mySerial.println(">>> DISCONNECTED - Stopping all operations");
    mySerial.println("    Treating as AUTO MODE TIMEOUT (20 minutes complete)");
    
    // Stop all position motors
    offForwardBackward();   // Dừng FORWARD/BACKWARD
    offReclineIncline();    // Dừng RECLINE/INCLINE
    mySerial.println("  - Position motors stopped");
    
    // Stop AUTO mode (same as 20-minute timeout)
    // This will:
    // - Stop roll motor
    // - Reset all mode flags (AUTO, KNEADING, PERCUSSION, etc.)
    // - Reset timer 20 phút
    // - Reset to MANUAL mode
    stopAutoMode();  // ← QUAN TRỌNG: Giống timeout 20 phút
    
    // Reset BLE module
    resetHM10();
    mySerial.println("  - BLE module reset");
    mySerial.println(">>> System reset to MANUAL mode (ready for next session)");
  }
  break;
```

#### **Massage_v1_hardware.cpp - stopAutoMode()** (Line 1210-1275)
```cpp
void stopAutoMode() {
  mySerial.println("=== AUTO MODE STOPPED ===");

  // STOP ALL MOTORS
  offRollMotor();                    // Tắt roll motor
  mySerial.println("  - Roll motor OFF");

  // RESET ALL MODE FLAGS
  setModeAuto(false);                // Tắt AUTO mode
  setAutodefaultMode(false);         // Tắt Default program
  setRollSpotMode(false);            // Tắt Roll/Spot
  setKneadingMode(false);            // Tắt Kneading
  setCompressionMode(false);         // Tắt Compression
  setPercussionMode(false);          // Tắt Percussion
  setCombineMode(false);             // Tắt Combine
  
  // Reset features
  setIntensityLevel(0);              // Reset cường độ về 0
  
  // RESET TIMER AND PROGRAM STATES
  setAutoModeTimerActive(false);     // Dừng timer 20 phút
  setAutoTimerStarted(false);        // Cho phép start lại AUTO
  setCurrentAutoProgram(AUTO_NONE);  // Reset program
  
  // Reset global states
  setResetProgramStatesFlag(true);
  setRL3PWMState(false);
  setWaitingForSensor(false);
  setSensorUpLimit(false);
  setSensorDownLimit(false);
  
  mySerial.println("All programs stopped, timer reset");
  mySerial.println("========================");
}
```

---

## 🔄 **Flow diagram**

```
USER TAP "DISCONNECT" BUTTON
         ↓
[ControlScreen.js] handleDisconnect()
         ↓
[BleService.js] disconnect() → safeDisconnect()
         ↓
[BleService.js] sendDisconnectCommand()
         ↓
📤 GỬI PACKET: 0x02 70 FF FF 00 00 00 [CS] 0x03
         ↓
📡 BLE Transmission
         ↓
📥 ESP32 NHẬN PACKET
         ↓
[MessageProcess.cpp] processCommand()
         ↓
case CMD_DISCONNECT (0xFF)
         ↓
┌────────────────────────────────────┐
│ offForwardBackward()               │ ← Dừng FORWARD/BACKWARD
│ offReclineIncline()                │ ← Dừng RECLINE/INCLINE
│ stopAutoMode()                     │ ← QUAN TRỌNG!
│   ├─ offRollMotor()                │   ← Dừng roll motor
│   ├─ Reset all mode flags          │   ← AUTO, KNEADING, etc.
│   ├─ Reset 20-minute timer         │   ← Timer về 0
│   └─ Reset to MANUAL mode          │   ← Về MANUAL
│ resetHM10()                         │ ← Reset BLE module
└────────────────────────────────────┘
         ↓
✅ ESP32 ở trạng thái MANUAL MODE (sạch sẽ, sẵn sàng session mới)
```

---

## ✅ **Xác nhận**

### **App React Native:**
- ✅ Có định nghĩa `DISCONNECT` command
- ✅ Có hàm `sendDisconnectCommand()` 
- ✅ Được gọi trong `safeDisconnect()`
- ✅ User tap "Disconnect" → Gửi lệnh đến ESP32

### **ESP32 Firmware:**
- ✅ Nhận `CMD_DISCONNECT` (0xFF)
- ✅ Check `data1 == 0x00` (DATA_OFF)
- ✅ Gọi `stopAutoMode()` → Tắt tất cả
- ✅ Reset về MANUAL mode

### **Tương đương timeout 20 phút:**
- ✅ Cùng gọi `stopAutoMode()`
- ✅ Cùng reset tất cả modes
- ✅ Cùng reset timer
- ✅ Cùng về MANUAL mode

---

## 🎯 **Packet chi tiết**

### **Packet gửi từ App:**

```
STX:      0x02
DeviceID: 0x70  (DEVICE_ID.MAIN)
Sequence: 0xFF  (SEQUENCE.DISCONNECT)
Command:  0xFF  (CMD_DISCONNECT)
Data1:    0x00  (DATA_OFF) ← QUAN TRỌNG: Trigger stopAutoMode()
Data2:    0x00
Data3:    0x00
Checksum: [Calculated]
ETX:      0x03
```

### **Giống với timeout 20 phút:**

Khi timer 20 phút hết, firmware gọi:
```cpp
// Trong auto mode timer check
if (elapsed >= AUTO_MODE_DURATION_TICKS) {
  stopAutoMode(); // ← Cùng hàm với DISCONNECT
}
```

**Khi DISCONNECT:**
```cpp
case CMD_DISCONNECT:
  if (data1 == DATA_OFF) {
    stopAutoMode(); // ← Cùng hàm với timeout!
  }
  break;
```

---

## 📊 **So sánh**

| Tình huống | Gọi hàm | Tắt motors | Reset modes | Reset timer | Về MANUAL |
|------------|---------|------------|-------------|-------------|-----------|
| **Timeout 20 phút** | `stopAutoMode()` | ✅ | ✅ | ✅ | ✅ |
| **User tap Disconnect** | `stopAutoMode()` | ✅ | ✅ | ✅ | ✅ |

**Kết luận:** Hoàn toàn giống nhau! ✅

---

## 🧪 **Cách test**

### **Test 1: Disconnect bình thường**
1. Kết nối app với ESP32
2. Bật AUTO mode
3. Chọn KNEADING hoặc PERCUSSION
4. Tap "Disconnect" trên app
5. ✅ Kiểm tra ESP32 Serial Monitor:
   ```
   >>> DISCONNECTED - Stopping all operations
       Treating as AUTO MODE TIMEOUT (20 minutes complete)
     - Position motors stopped
   === AUTO MODE STOPPED ===
   Stopping all motors:
     - Roll motor OFF
   Resetting all mode flags:
   All programs stopped, timer reset
     - BLE module reset
   >>> System reset to MANUAL mode (ready for next session)
   ```

### **Test 2: So sánh với timeout**
1. Kết nối app với ESP32
2. Bật AUTO mode
3. Chờ 20 phút (hoặc thay đổi `AUTO_MODE_DURATION_TICKS` để test nhanh)
4. ✅ Kiểm tra ESP32 Serial Monitor: **Cùng log với Test 1**

---

## 🔧 **Nếu cần sửa**

### **Thay đổi behavior của DISCONNECT**

**File:** `MessageProcess.cpp` (Line 799-824)

```cpp
case CMD_DISCONNECT:
  if (data1 == DATA_OFF) {
    // Tùy chỉnh hành vi tại đây:
    
    // Option 1: Chỉ dừng motors, KHÔNG reset timer (giữ session)
    offForwardBackward();
    offReclineIncline();
    offRollMotor();
    
    // Option 2: Reset toàn bộ (KHUYÊN DÙNG - Hiện tại)
    stopAutoMode(); // ← Tắt tất cả, reset timer, về MANUAL
    
    // Option 3: Custom logic
    // ... your custom code here
  }
  break;
```

---

## 📖 **Tài liệu liên quan**

- `QR_CODE_FORMAT.md` - QR code format (name-based)
- `BLE_PAIRING_EXPLAINED.md` - BLE pairing details
- `MessageProcess.cpp` - Firmware command processing

---

## ✅ **Kết luận**

**Trạng thái hiện tại:**
- ✅ App **ĐÃ GỬI** CMD_DISCONNECT
- ✅ Firmware **ĐÃ NHẬN** CMD_DISCONNECT
- ✅ Firmware **TẮT TẤT CẢ** (giống timeout 20 phút)
- ✅ Firmware **RESET VỀ MANUAL** mode
- ✅ **KHÔNG CẦN SỬA GÌ THÊM**

**Flow hoàn chỉnh:** App Disconnect → ESP32 stopAutoMode() → Reset to MANUAL mode 🎉

---

**Cập nhật:** 2025-10-07  
**Version:** 3.0 - Disconnect = Timeout (Complete Reset)

