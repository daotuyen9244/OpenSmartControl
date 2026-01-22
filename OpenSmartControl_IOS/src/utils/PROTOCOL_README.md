# 📡 Giao thức giao tiếp 9-byte Packet

## Tổng quan

Hệ thống sử dụng giao thức gói tin 9-byte để giao tiếp giữa React Native app và firmware Arduino/STM32.

## 📦 Cấu trúc gói tin

```
STX + DeviceID + Sequence + Command + Data1 + Data2 + Data3 + Checksum + ETX
```

| Byte | Tên | Mô tả | Ví dụ |
|------|-----|-------|-------|
| 0 | STX | Start of Text | 0x02 |
| 1 | Device ID | ID thiết bị | 0x70 |
| 2 | Sequence | Số thứ tự | 0xC3 |
| 3 | Command | Mã lệnh | 0x10 |
| 4 | Data1 | Dữ liệu 1 | 0xF0 |
| 5 | Data2 | Dữ liệu 2 | 0x00 |
| 6 | Data3 | Dữ liệu 3 | 0x00 |
| 7 | Checksum | Kiểm tra tổng | 0xDD |
| 8 | ETX | End of Text | 0x03 |

## 🔢 Thuật toán Checksum

```javascript
function calculateChecksum(data) {
  let sum = 0;
  
  // Sum all bytes from index 0 to 5 (6-byte payload)
  for (let i = 0; i < 6; i++) {
    sum += data[i];
  }
  
  // Add carry (Internet checksum style)
  while (sum >> 8) {
    sum = (sum & 0xFF) + (sum >> 8);
  }
  
  // One's complement + 0x10 offset
  return ((~sum) + 0x10) & 0xFF;
}
```

**Ví dụ tính checksum:**
```
Payload: 70 C0 10 F0 00 00
Sum: 0x70 + 0xC0 + 0x10 + 0xF0 + 0x00 + 0x00 = 0x230
Carry: 0x230 = 0x30 + 0x02 → 0x32
One's complement: ~0x32 = 0xCD
Add offset: 0xCD + 0x10 = 0xDD
```

## 📋 Danh sách Commands

### 🔄 Mode Control
| Command | Device ID | Sequence | Command | Data1 | Data2 | Data3 | Mô tả |
|---------|-----------|----------|---------|-------|-------|-------|-------|
| AUTO_ON | 0x70 | 0xC3 | 0x10 | 0xF0 | 0x00 | 0x00 | Bật chế độ tự động |
| AUTO_OFF | 0x70 | 0xC3 | 0x10 | 0x00 | 0x00 | 0x00 | Tắt chế độ tự động |

### 🎯 Roll Motor
| Command | Device ID | Sequence | Command | Data1 | Data2 | Data3 | Mô tả |
|---------|-----------|----------|---------|-------|-------|-------|-------|
| ROLL_ON | 0x70 | 0x62 | 0x20 | 0xF0 | 0x00 | 0x00 | Bật motor roll |
| ROLL_OFF | 0x70 | 0x62 | 0x20 | 0x00 | 0x00 | 0x00 | Tắt motor roll |

### 🎨 Massage Techniques
| Command | Device ID | Sequence | Command | Data1 | Data2 | Data3 | Mô tả |
|---------|-----------|----------|---------|-------|-------|-------|-------|
| KNEADING_ON | 0x70 | 0x93 | 0x30 | 0xF0 | 0x00 | 0x00 | Bật chế độ nhào |
| COMBINE_ON | 0x70 | 0x03 | 0x50 | 0xF0 | 0x00 | 0x00 | Bật chế độ kết hợp |
| PERCUSSION_ON | 0x70 | 0xE3 | 0x40 | 0xF0 | 0x00 | 0x00 | Bật chế độ gõ |
| COMPRESSION_ON | 0x70 | 0x24 | 0x60 | 0xF0 | 0x00 | 0x00 | Bật chế độ nén |

### ⚡ Intensity Control
| Command | Device ID | Sequence | Command | Data1 | Data2 | Data3 | Mô tả |
|---------|-----------|----------|---------|-------|-------|-------|-------|
| INTENSITY_UP | 0x70 | 0x73 | 0x70 | 0x00 | 0x00 | 0x50 | Tăng cường độ |
| INTENSITY_DOWN | 0x70 | 0x73 | 0x70 | 0x00 | 0x00 | 0x50 | Giảm cường độ |

### 🪑 Position Control
| Command | Device ID | Sequence | Command | Data1 | Data2 | Data3 | Mô tả |
|---------|-----------|----------|---------|-------|-------|-------|-------|
| RECLINE_PUSH | 0x70 | 0x41 | 0x90 | 0xF0 | 0x00 | 0x00 | Ngả lưng (bắt đầu) |
| RECLINE_RELEASE | 0x70 | 0x61 | 0x90 | 0x00 | 0x00 | 0x00 | Ngả lưng (dừng) |
| INCLINE_PUSH | 0x70 | 0x81 | 0x80 | 0xF0 | 0x00 | 0x00 | Nâng chân (bắt đầu) |
| INCLINE_RELEASE | 0x70 | 0x61 | 0x80 | 0x00 | 0x00 | 0x00 | Nâng chân (dừng) |
| FORWARD_PUSH | 0x70 | 0xD2 | 0xA0 | 0xF0 | 0x00 | 0x00 | Tiến tới (bắt đầu) |
| FORWARD_RELEASE | 0x70 | 0xD2 | 0xA0 | 0x00 | 0x00 | 0x00 | Tiến tới (dừng) |
| BACKWARD_PUSH | 0x70 | 0xA2 | 0xB0 | 0xF0 | 0x00 | 0x00 | Lùi về (bắt đầu) |
| BACKWARD_RELEASE | 0x70 | 0xA2 | 0xB0 | 0x00 | 0x00 | 0x00 | Lùi về (dừng) |

### 🔌 System Commands
| Command | Device ID | Sequence | Command | Data1 | Data2 | Data3 | Mô tả |
|---------|-----------|----------|---------|-------|-------|-------|-------|
| DISCONNECT | 0x70 | 0xFF | 0xFF | 0x00 | 0x00 | 0x00 | Ngắt kết nối |
| STATUS_REQUEST | 0x70 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | Yêu cầu trạng thái |

## 🚀 Cách sử dụng trong Code

### 1. Import constants
```javascript
import { getCommand, COMMANDS } from '../utils/packetCommands';
```

### 2. Gửi command
```javascript
// Cách 1: Sử dụng getCommand()
const cmd = getCommand('AUTO_ON');
await bleService.sendPacketCommand(cmd.deviceId, cmd.sequence, cmd.command, cmd.data1, cmd.data2, cmd.data3);

// Cách 2: Sử dụng method có sẵn
await bleService.enableAutoMode();
```

### 3. Tạo command mới
```javascript
// Thêm vào packetCommands.js
NEW_COMMAND: {
  deviceId: 0x70,
  sequence: 0xXX,
  command: 0xXX,
  data1: 0xXX,
  data2: 0xXX,
  data3: 0xXX,
  description: 'Mô tả command'
}
```

## 🔍 Debug và Testing

### Log format
```
=== SEND PACKET COMMAND #1 ===
Device ID: 0x70
Sequence: 0xC3
Command: 0x10
Data: 0xF0, 0x00, 0x00
Packet (hex): 02 70 C3 10 F0 00 00 DD 03
Base64 packet: "AnDDEDAAAN0D"
```

### Kiểm tra checksum
```javascript
const payload = [0x70, 0xC3, 0x10, 0xF0, 0x00, 0x00];
const checksum = calculateChecksum(payload);
console.log(`Checksum: 0x${checksum.toString(16).toUpperCase()}`);
```

## ⚠️ Lưu ý quan trọng

1. **Sequence numbers**: Mỗi command có sequence riêng để tránh trùng lặp
2. **Checksum validation**: Firmware sẽ kiểm tra checksum trước khi xử lý
3. **Timeout handling**: Mỗi command có timeout riêng (thường 10s)
4. **Error handling**: Luôn xử lý lỗi khi gửi command
5. **State synchronization**: Đồng bộ trạng thái giữa app và firmware

## 📚 Tài liệu tham khảo

- [Firmware Protocol Documentation](./firmware_protocol.md)
- [BLE Service Documentation](../services/BleService.js)
- [Command Constants](../utils/packetCommands.js)
