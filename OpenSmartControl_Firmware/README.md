# Hướng Dẫn Sử Dụng Giao Tiếp Truyền Nhận Dữ Liệu

## Tổng Quan

Hệ thống massage chair sử dụng giao tiếp BLE (Bluetooth Low Energy) qua module HM10 để nhận lệnh điều khiển từ ứng dụng di động. Giao thức truyền dữ liệu sử dụng định dạng packet có checksum để đảm bảo tính toàn vẹn dữ liệu.

## Thông Số Kỹ Thuật

- **Giao tiếp**: BLE qua HM10 module
- **UART**: UART2 (PA2/PA3)
- **Baudrate**: 9600 bps
- **Device ID**: `0x70`
- **Định dạng packet**: STX + Payload + Checksum + ETX

## Định Dạng Packet

### Cấu Trúc Packet

Mỗi packet có độ dài cố định **9 bytes** với cấu trúc như sau:

```
[STX] [DeviceID] [Sequence] [Command] [Data1] [Data2] [Data3] [Checksum] [ETX]
 0x02    0x70      Variable    Variable  Variable Variable Variable  Calc    0x03
```

### Chi Tiết Các Trường

| Vị trí | Tên trường | Giá trị | Mô tả |
|--------|------------|---------|-------|
| 0 | STX | `0x02` | Start of Text - Đánh dấu bắt đầu packet |
| 1 | DeviceID | `0x70` | ID thiết bị (cố định) |
| 2 | Sequence | `0x00-0xFF` | Số thứ tự packet (tăng dần) |
| 3 | Command | `0x10-0xFF` | Mã lệnh điều khiển |
| 4 | Data1 | `0x00-0xFF` | Dữ liệu 1 (tùy theo lệnh) |
| 5 | Data2 | `0x00-0xFF` | Dữ liệu 2 (tùy theo lệnh) |
| 6 | Data3 | `0x00-0xFF` | Dữ liệu 3 (thường = 0x00) |
| 7 | Checksum | Calculated | Checksum tính từ payload (6 bytes) |
| 8 | ETX | `0x03` | End of Text - Đánh dấu kết thúc packet |

### Tính Checksum

Checksum được tính theo thuật toán Internet checksum với offset `0x10`:

```cpp
uint8_t calculate_checksum(const uint8_t *payload, int len) {
  uint16_t sum = 0;
  
  // Tính tổng tất cả bytes trong payload (6 bytes)
  for (int i = 0; i < 6; i++) {
    sum += payload[i];
  }
  
  // Cộng carry (Internet checksum style)
  while (sum >> 8) {
    sum = (sum & 0xFF) + (sum >> 8);
  }
  
  // One's complement + 0x10 offset
  uint8_t result = ((~sum) + 0x10) & 0xFF;
  
  return result;
}
```

**Ví dụ tính checksum:**
- Payload: `[0x70, 0x01, 0x10, 0xF0, 0x00, 0x00]`
- Sum = 0x70 + 0x01 + 0x10 + 0xF0 + 0x00 + 0x00 = 0x171
- Carry: 0x171 → 0x01 + 0x71 = 0x72
- One's complement: ~0x72 = 0x8D
- + 0x10: 0x8D + 0x10 = 0x9D
- **Checksum = 0x9D**

## Các Giá Trị Dữ Liệu Chuẩn

| Giá trị | Hex | Mô tả |
|---------|-----|-------|
| DATA_ON | `0xF0` | Bật / Nhấn giữ |
| DATA_OFF | `0x00` | Tắt / Thả ra |
| INTENSITY_LOW | `0x00` | Cường độ thấp |
| INTENSITY_HIGH | `0x20` | Cường độ cao |

## Danh Sách Lệnh Điều Khiển

### 1. CMD_AUTO (0x10) - Chế Độ Tự Động

**Mô tả**: Bật/tắt chế độ tự động (AUTO DEFAULT program)

**Packet mẫu**:
- Bật AUTO: `[0x02, 0x70, 0x01, 0x10, 0xF0, 0x00, 0x00, 0x9D, 0x03]`
- Tắt AUTO: `[0x02, 0x70, 0x02, 0x10, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (ON) hoặc `0x00` (OFF)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **ON**: Khởi động chế độ AUTO DEFAULT với roll motor luôn BẬT (không thể tắt)
- **OFF**: Dừng tất cả chương trình và reset về chế độ MANUAL

**Lưu ý**: 
- Nếu hệ thống chưa được "home" (về vị trí ban đầu), lệnh AUTO ON sẽ tự động kích hoạt GO HOME trước
- Roll motor trong AUTO DEFAULT luôn BẬT và không thể tắt bằng CMD_ROLL_MOTOR

---

### 2. CMD_ROLL_MOTOR (0x20) - Bật/Tắt Roll Motor

**Mô tả**: Bật/tắt roll motor (chỉ hoạt động trong các chế độ KNEADING, COMPRESSION, PERCUSSION, COMBINE)

**Packet mẫu**:
- Bật: `[0x02, 0x70, 0x03, 0x20, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Tắt: `[0x02, 0x70, 0x04, 0x20, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (ON) hoặc `0x00` (OFF)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **ON**: Bật roll motor và cho phép auto programs điều khiển
- **OFF**: Tắt roll motor và ngăn auto programs tự động bật lại

**Lưu ý**: 
- **KHÔNG hoạt động** trong chế độ AUTO DEFAULT (roll motor luôn BẬT)
- Chỉ hoạt động khi đang ở chế độ AUTO với các program khác (KNEADING, COMPRESSION, PERCUSSION, COMBINE)

---

### 3. CMD_ROLL_DIRECTION (0x21) - Điều Khiển Roll Thủ Công

**Mô tả**: Điều khiển roll motor thủ công (lên/xuống) với chế độ nhấn giữ

**Packet mẫu**:
- Roll DOWN (nhấn): `[0x02, 0x70, 0x05, 0x21, 0x01, 0xF0, 0x00, 0xXX, 0x03]`
- Roll DOWN (thả): `[0x02, 0x70, 0x06, 0x21, 0x01, 0x00, 0x00, 0xXX, 0x03]`
- Roll UP (nhấn): `[0x02, 0x70, 0x07, 0x21, 0x02, 0xF0, 0x00, 0xXX, 0x03]`
- Roll UP (thả): `[0x02, 0x70, 0x08, 0x21, 0x02, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: 
  - `0x01` = Roll DOWN (xuống)
  - `0x02` = Roll UP (lên)
- `Data2`: 
  - `0xF0` = Nhấn giữ (PUSH)
  - `0x00` = Thả ra (RELEASE)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **PUSH**: Bắt đầu di chuyển roll motor theo hướng chỉ định
- **RELEASE**: Dừng roll motor ngay lập tức

**An toàn**:
- Tự động chặn nếu sensor giới hạn đang kích hoạt (UP sensor khi roll UP, DOWN sensor khi roll DOWN)
- Tự động đặt manual priority để override auto programs

---

### 4. CMD_KNEADING_MANUAL (0x22) - Điều Khiển Kneading Thủ Công

**Mô tả**: Điều khiển motor kneading thủ công với chế độ nhấn giữ

**Packet mẫu**:
- Bật (nhấn): `[0x02, 0x70, 0x09, 0x22, 0x03, 0xF0, 0x00, 0xXX, 0x03]`
- Tắt (thả): `[0x02, 0x70, 0x0A, 0x22, 0x03, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0x03` (cố định - mã function KNEADING)
- `Data2`: 
  - `0xF0` = Nhấn giữ (PUSH)
  - `0x00` = Thả ra (RELEASE)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **PUSH**: Bật motor kneading
- **RELEASE**: Tắt motor kneading

**Lưu ý**: Tự động đặt manual priority để override auto programs

---

### 5. CMD_PERCUSSION_MANUAL (0x23) - Điều Khiển Percussion Thủ Công

**Mô tả**: Điều khiển motor percussion (compression) thủ công với chế độ nhấn giữ

**Packet mẫu**:
- Bật (nhấn): `[0x02, 0x70, 0x0B, 0x23, 0x04, 0xF0, 0x00, 0xXX, 0x03]`
- Tắt (thả): `[0x02, 0x70, 0x0C, 0x23, 0x04, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0x04` (cố định - mã function PERCUSSION)
- `Data2`: 
  - `0xF0` = Nhấn giữ (PUSH)
  - `0x00` = Thả ra (RELEASE)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **PUSH**: Bật motor percussion
- **RELEASE**: Tắt motor percussion

**Lưu ý**: Tự động đặt manual priority để override auto programs

---

### 6. CMD_KNEADING (0x30) - Chế Độ Kneading

**Mô tả**: Bật/tắt chế độ kneading trong AUTO mode

**Packet mẫu**:
- Bật: `[0x02, 0x70, 0x0D, 0x30, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Tắt: `[0x02, 0x70, 0x0E, 0x30, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (ON) hoặc `0x00` (OFF)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **ON**: Chuyển sang chế độ KNEADING program (tắt các chế độ khác)
- **OFF**: Tắt chế độ kneading

**Yêu cầu**: 
- Hệ thống phải đã được "home" (homeRun = true)
- Phải đang ở chế độ AUTO (modeAuto = true)

**Lưu ý**: Roll motor có thể được bật/tắt bằng CMD_ROLL_MOTOR trong chế độ này

---

### 7. CMD_PERCUSSION (0x40) - Chế Độ Percussion

**Mô tả**: Bật/tắt chế độ percussion trong AUTO mode

**Packet mẫu**:
- Bật: `[0x02, 0x70, 0x0F, 0x40, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Tắt: `[0x02, 0x70, 0x10, 0x40, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (ON) hoặc `0x00` (OFF)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **ON**: Chuyển sang chế độ PERCUSSION program (tắt các chế độ khác)
- **OFF**: Tắt chế độ percussion

**Yêu cầu**: 
- Hệ thống phải đã được "home" (homeRun = true)
- Phải đang ở chế độ AUTO (modeAuto = true)

**Lưu ý**: Roll motor có thể được bật/tắt bằng CMD_ROLL_MOTOR trong chế độ này

---

### 8. CMD_COMPRESSION (0x50) - Chế Độ Compression

**Mô tả**: Bật/tắt chế độ compression trong AUTO mode

**Packet mẫu**:
- Bật: `[0x02, 0x70, 0x11, 0x50, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Tắt: `[0x02, 0x70, 0x12, 0x50, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (ON) hoặc `0x00` (OFF)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **ON**: Chuyển sang chế độ COMPRESSION program (tắt các chế độ khác)
- **OFF**: Tắt chế độ compression

**Yêu cầu**: 
- Hệ thống phải đã được "home" (homeRun = true)
- Phải đang ở chế độ AUTO (modeAuto = true)

**Lưu ý**: 
- Roll motor có thể được bật/tắt bằng CMD_ROLL_MOTOR trong chế độ này
- Hỗ trợ điều chỉnh cường độ bằng CMD_INTENSITY_LEVEL

---

### 9. CMD_COMBINE (0x60) - Chế Độ Kết Hợp

**Mô tả**: Bật/tắt chế độ kết hợp nhiều chương trình (KNEADING + COMPRESSION)

**Packet mẫu**:
- Bật: `[0x02, 0x70, 0x13, 0x60, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Tắt: `[0x02, 0x70, 0x14, 0x60, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (ON) hoặc `0x00` (OFF)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **ON**: Kích hoạt chế độ COMBINED (Kneading + Compression cùng lúc)
- **OFF**: Tắt tất cả các chế độ đã kích hoạt

**Yêu cầu**: 
- Hệ thống phải đã được "home" (homeRun = true)
- Phải đang ở chế độ AUTO (modeAuto = true)

**Lưu ý**: 
- Roll motor và percussion có thể được bật/tắt riêng biệt
- Hỗ trợ điều chỉnh cường độ bằng CMD_INTENSITY_LEVEL

---

### 10. CMD_INTENSITY_LEVEL (0x70) - Điều Chỉnh Cường Độ

**Mô tả**: Điều chỉnh cường độ massage (chỉ hoạt động trong COMPRESSION, PERCUSSION, COMBINE)

**Packet mẫu**:
- Cường độ cao: `[0x02, 0x70, 0x15, 0x70, 0x20, 0x00, 0x00, 0xXX, 0x03]`
- Cường độ thấp: `[0x02, 0x70, 0x16, 0x70, 0x00, 0x00, 0x00, 0xXX, 0x03]`
- Tắt: `[0x02, 0x70, 0x17, 0x70, 0x00, 0x00, 0x00, 0xXX, 0x03]`
- Custom (ví dụ PWM=200): `[0x02, 0x70, 0x18, 0x70, 0xC8, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: 
  - `0x20` = HIGH intensity (PWM = 254)
  - `0x00` = LOW intensity (PWM = 160) hoặc OFF (PWM = 0)
  - `0x01-0xFF` = Custom intensity (PWM value, 0-255)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **HIGH (0x20)**: Đặt cường độ cao (PWM = 254)
- **LOW (0x00)**: Đặt cường độ thấp (PWM = 160)
- **OFF (0x00)**: Tắt motor (PWM = 0)
- **Custom**: Đặt giá trị PWM tùy chỉnh (0-255)

**Yêu cầu**: 
- Chỉ hoạt động trong các chế độ: COMPRESSION, PERCUSSION, COMBINE
- **KHÔNG hoạt động** trong AUTO DEFAULT hoặc KNEADING

---

### 11. CMD_INCLINE (0x80) - Điều Khiển Nâng Lên

**Mô tả**: Điều khiển motor nâng ghế lên (nhấn giữ)

**Packet mẫu**:
- Nhấn: `[0x02, 0x70, 0x19, 0x80, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Thả: `[0x02, 0x70, 0x1A, 0x80, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (PUSH) hoặc `0x00` (RELEASE)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **PUSH**: Bắt đầu nâng ghế lên (tiếp tục cho đến khi thả)
- **RELEASE**: Dừng motor ngay lập tức

**An toàn**: 
- Tự động dừng roll motor trước khi nâng
- Đặt manual priority để pause auto programs

---

### 12. CMD_RECLINE (0x90) - Điều Khiển Hạ Xuống

**Mô tả**: Điều khiển motor hạ ghế xuống (nhấn giữ)

**Packet mẫu**:
- Nhấn: `[0x02, 0x70, 0x1B, 0x90, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Thả: `[0x02, 0x70, 0x1C, 0x90, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (PUSH) hoặc `0x00` (RELEASE)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **PUSH**: Bắt đầu hạ ghế xuống (tiếp tục cho đến khi thả)
- **RELEASE**: Dừng motor ngay lập tức

**An toàn**: 
- Tự động dừng roll motor trước khi hạ
- Đặt manual priority để pause auto programs

---

### 13. CMD_FORWARD (0xA0) - Điều Khiển Tiến Lên

**Mô tả**: Điều khiển motor đẩy ghế về phía trước (nhấn giữ)

**Packet mẫu**:
- Nhấn: `[0x02, 0x70, 0x1D, 0xA0, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Thả: `[0x02, 0x70, 0x1E, 0xA0, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (PUSH) hoặc `0x00` (RELEASE)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **PUSH**: Bắt đầu đẩy ghế về phía trước (tiếp tục cho đến khi thả)
- **RELEASE**: Dừng motor ngay lập tức

**An toàn**: 
- Tự động dừng roll motor trước khi đẩy
- Đặt manual priority để pause auto programs

---

### 14. CMD_BACKWARD (0xB0) - Điều Khiển Lùi Lại

**Mô tả**: Điều khiển motor kéo ghế về phía sau (nhấn giữ)

**Packet mẫu**:
- Nhấn: `[0x02, 0x70, 0x1F, 0xB0, 0xF0, 0x00, 0x00, 0xXX, 0x03]`
- Thả: `[0x02, 0x70, 0x20, 0xB0, 0x00, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0xF0` (PUSH) hoặc `0x00` (RELEASE)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **PUSH**: Bắt đầu kéo ghế về phía sau (tiếp tục cho đến khi thả)
- **RELEASE**: Dừng motor ngay lập tức

**An toàn**: 
- Tự động dừng roll motor trước khi kéo
- Đặt manual priority để pause auto programs

---

### 15. CMD_DISCONNECT (0xFF) - Ngắt Kết Nối

**Mô tả**: Ngắt kết nối và reset hệ thống (tương đương AUTO OFF + reset BLE)

**Packet mẫu**:
- Ngắt kết nối: `[0x02, 0x70, 0x21, 0xFF, 0x00, 0x00, 0x00, 0xXX, 0x03]`
- Kết nối: `[0x02, 0x70, 0x22, 0xFF, 0xF0, 0x00, 0x00, 0xXX, 0x03]`

**Tham số**:
- `Data1`: `0x00` (DISCONNECT) hoặc `0xF0` (CONNECTED)
- `Data2`: `0x00` (không dùng)
- `Data3`: `0x00` (không dùng)

**Hành vi**:
- **DISCONNECT (0x00)**: 
  - Dừng tất cả motor vị trí (forward/backward, recline/incline)
  - Dừng AUTO mode (tương đương AUTO OFF)
  - Reset module BLE HM10
  - Kích hoạt GO HOME sequence
- **CONNECTED (0xF0)**: Chỉ xác nhận kết nối (không có hành động)

---

## Xử Lý Trùng Lặp Lệnh (Command Deduplication)

Hệ thống có cơ chế chống trùng lặp lệnh để tránh thực thi cùng một lệnh nhiều lần:

- **Cửa sổ thời gian**: 2000ms (200 ticks × 10ms)
- **Kiểm tra**: Sequence + Command + Data1 + Data2 (nếu có)
- **Ngoại lệ**: Các lệnh motor PUSH (INCLINE, RECLINE, FORWARD, BACKWARD) được phép duplicate để reset timeout

**Ví dụ**:
```
Lệnh 1: [..., 0x01, 0x10, 0xF0, ...] tại tick 100
Lệnh 2: [..., 0x01, 0x10, 0xF0, ...] tại tick 150 → BỊ CHẶN (duplicate)
Lệnh 3: [..., 0x01, 0x10, 0xF0, ...] tại tick 250 → ĐƯỢC CHẤP NHẬN (>200 ticks)
```

---

## Quy Trình Xử Lý Packet

### Nhận Packet

1. **Đợi STX (0x02)**: Bắt đầu đọc packet khi nhận được STX
2. **Đọc hex string**: Thu thập các ký tự hex giữa STX và ETX
3. **Gặp ETX (0x03)**: Kết thúc đọc và đánh dấu packet sẵn sàng
4. **Chuyển đổi hex → bytes**: Chuyển chuỗi hex thành mảng bytes
5. **Tái tạo packet**: Thêm STX và ETX để có packet đầy đủ 9 bytes
6. **Xác thực**: Kiểm tra độ dài, STX/ETX, DeviceID, và checksum
7. **Xử lý lệnh**: Nếu hợp lệ, thực thi lệnh tương ứng

### Gửi Packet

1. **Tạo payload**: [DeviceID, Sequence, Command, Data1, Data2, Data3]
2. **Tính checksum**: Tính checksum từ payload 6 bytes
3. **Tạo packet**: [STX] + payload + [checksum] + [ETX]
4. **Gửi qua BLE**: Gửi packet 9 bytes qua UART2

---

## Ví Dụ Sử Dụng

### Ví dụ 1: Bật chế độ AUTO DEFAULT

```cpp
// Payload: [0x70, 0x01, 0x10, 0xF0, 0x00, 0x00]
// Checksum calculation:
// Sum = 0x70 + 0x01 + 0x10 + 0xF0 + 0x00 + 0x00 = 0x171
// Carry: 0x171 → 0x01 + 0x71 = 0x72
// One's complement: ~0x72 = 0x8D
// + 0x10: 0x8D + 0x10 = 0x9D
// Checksum = 0x9D

uint8_t packet[] = {
  0x02,  // STX
  0x70,  // DeviceID
  0x01,  // Sequence
  0x10,  // CMD_AUTO
  0xF0,  // DATA_ON
  0x00,  // Data2
  0x00,  // Data3
  0x9D,  // Checksum
  0x03   // ETX
};

// Gửi qua BLE
bleSerial.write(packet, 9);
```

### Ví dụ 2: Điều khiển Roll DOWN thủ công

```cpp
// Roll DOWN - Nhấn giữ
uint8_t packet_push[] = {
  0x02,  // STX
  0x70,  // DeviceID
  0x05,  // Sequence
  0x21,  // CMD_ROLL_DIRECTION
  0x01,  // Data1: DOWN
  0xF0,  // Data2: PUSH
  0x00,  // Data3
  0xXX,  // Checksum (tính toán)
  0x03   // ETX
};

// Roll DOWN - Thả ra
uint8_t packet_release[] = {
  0x02,  // STX
  0x70,  // DeviceID
  0x06,  // Sequence
  0x21,  // CMD_ROLL_DIRECTION
  0x01,  // Data1: DOWN
  0x00,  // Data2: RELEASE
  0x00,  // Data3
  0xXX,  // Checksum (tính toán)
  0x03   // ETX
};
```

### Ví dụ 3: Chuyển sang chế độ KNEADING

```cpp
// Yêu cầu: Hệ thống phải đã home và đang ở AUTO mode
uint8_t packet[] = {
  0x02,  // STX
  0x70,  // DeviceID
  0x0D,  // Sequence
  0x30,  // CMD_KNEADING
  0xF0,  // DATA_ON
  0x00,  // Data2
  0x00,  // Data3
  0xXX,  // Checksum (tính toán)
  0x03   // ETX
};
```

---

## Bảng Tóm Tắt Lệnh

| Lệnh | Hex | Data1 | Data2 | Mô tả | Yêu cầu |
|------|-----|-------|-------|-------|---------|
| AUTO | `0x10` | `0xF0`/`0x00` | - | Bật/tắt AUTO DEFAULT | - |
| ROLL_MOTOR | `0x20` | `0xF0`/`0x00` | - | Bật/tắt roll motor | KNEADING/COMPRESSION/PERCUSSION/COMBINE |
| ROLL_DIRECTION | `0x21` | `0x01`/`0x02` | `0xF0`/`0x00` | Roll UP/DOWN thủ công | - |
| KNEADING_MANUAL | `0x22` | `0x03` | `0xF0`/`0x00` | Kneading thủ công | - |
| PERCUSSION_MANUAL | `0x23` | `0x04` | `0xF0`/`0x00` | Percussion thủ công | - |
| KNEADING | `0x30` | `0xF0`/`0x00` | - | Chế độ kneading | Home + AUTO |
| PERCUSSION | `0x40` | `0xF0`/`0x00` | - | Chế độ percussion | Home + AUTO |
| COMPRESSION | `0x50` | `0xF0`/`0x00` | - | Chế độ compression | Home + AUTO |
| COMBINE | `0x60` | `0xF0`/`0x00` | - | Chế độ kết hợp | Home + AUTO |
| INTENSITY_LEVEL | `0x70` | `0x00`-`0xFF` | - | Điều chỉnh cường độ | COMPRESSION/PERCUSSION/COMBINE |
| INCLINE | `0x80` | `0xF0`/`0x00` | - | Nâng ghế lên | - |
| RECLINE | `0x90` | `0xF0`/`0x00` | - | Hạ ghế xuống | - |
| FORWARD | `0xA0` | `0xF0`/`0x00` | - | Đẩy ghế về trước | - |
| BACKWARD | `0xB0` | `0xF0`/`0x00` | - | Kéo ghế về sau | - |
| DISCONNECT | `0xFF` | `0x00`/`0xF0` | - | Ngắt kết nối | - |

---

## Lưu Ý Quan Trọng

1. **Sequence Number**: Nên tăng dần cho mỗi packet mới để tránh duplicate detection
2. **Checksum**: Luôn tính toán và kiểm tra checksum để đảm bảo tính toàn vẹn dữ liệu
3. **Timing**: Các lệnh motor PUSH có thể được gửi lại để reset timeout (tiếp tục chạy thêm 20s)
4. **Manual Priority**: Các lệnh thủ công tự động đặt manual priority để override auto programs
5. **Safety**: Hệ thống tự động kiểm tra sensors và chặn các lệnh không an toàn
6. **Home Sequence**: Một số lệnh yêu cầu hệ thống phải đã được "home" trước khi thực thi

---

## Xử Lý Lỗi

### Packet không hợp lệ

- **Độ dài sai**: Packet không đúng 9 bytes → Bỏ qua
- **STX/ETX sai**: Không bắt đầu bằng STX hoặc kết thúc bằng ETX → Bỏ qua
- **DeviceID sai**: DeviceID khác `0x70` → Bỏ qua
- **Checksum sai**: Checksum không khớp → Bỏ qua và in thông báo lỗi

### Lệnh không được chấp nhận

- **Duplicate**: Lệnh trùng trong cửa sổ 2000ms → Bỏ qua (trừ motor PUSH)
- **Điều kiện không đủ**: Thiếu điều kiện (home, AUTO mode, etc.) → Bỏ qua và in thông báo
- **Sensor chặn**: Sensor giới hạn đang kích hoạt → Bỏ qua và in thông báo an toàn

---

## Tài Liệu Tham Khảo

- File nguồn chính: `CommunicationManager.cpp` / `CommunicationManager.h`
- Định nghĩa lệnh: `MessageProcess.h`
- Xử lý lệnh: `CommunicationManager::processCommand()`
- Tính checksum: `CommunicationManager::calculate_checksum1()`

---

**Phiên bản**: V1 Release Ver0003 DEV_PRO  
**Ngày cập nhật**: 2025
