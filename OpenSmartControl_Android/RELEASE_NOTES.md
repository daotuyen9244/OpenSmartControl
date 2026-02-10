# Ghi chú phát hành – Massage Chair Control

## Phiên bản 1.0.5 (versionCode 6)

**Ngày:** Tháng 2, 2025

### Thay đổi
- Nâng version lên 1.0.5 (versionCode 6).

---

## Phiên bản 1.0.4 (versionCode 5)

**Ngày:** Tháng 2, 2025

### Thay đổi
- Nâng version lên 1.0.4 (versionCode 5).

---

## Phiên bản 1.0.3 (versionCode 4)

**Ngày:** Tháng 2, 2025

### Thay đổi
- Nâng version lên 1.0.3 (versionCode 4).

---

## Phiên bản 1.0.2 (versionCode 3)

**Ngày:** Tháng 2, 2025

### Thay đổi
- Nâng version lên 1.0.2 để phát hành lên Google Play.
- Sửa lỗi build release (đường dẫn icon trên màn hình chính).
- Chuẩn bị bản Android App Bundle (AAB) cho cửa hàng Play.

### Cách sử dụng bản này
- Kết nối ghế massage qua **Quét mã QR** hoặc **Kết nối thủ công** (Bluetooth).
- Điều khiển ghế từ màn hình **Control** sau khi đã kết nối.

---

## Phiên bản 1.0.1

- Bản phát hành trước (versionCode 2).

---

## Mẫu ghi chú cho Google Play Console

Khi upload AAB lên Play Console, có thể dán nội dung sau vào mục **Release notes** (hoặc chỉnh lại cho đúng với thay đổi thực tế):

**Tiếng Việt:**
```
• Phiên bản 1.0.2
• Sửa lỗi và cải thiện ổn định
• Hỗ trợ kết nối Bluetooth với ghế massage
• Quét mã QR để kết nối nhanh
• Điều khiển ghế trực tiếp từ ứng dụng
```

**English (for Play Store):**
```
• Version 1.0.2
• Bug fixes and stability improvements
• Bluetooth connection to massage chair
• Scan QR code for quick pairing
• Control your chair directly from the app
```

---

## Tệp gỡ rối mã nguồn (R8 / ProGuard mapping)

Sau khi bật R8 cho bản release, mỗi lần build AAB sẽ sinh ra **file mapping** dùng để Play Console gỡ rối stack trace khi có crash/ANR.

### File mapping nằm ở đâu
Sau khi chạy `./gradlew bundleRelease`:
```
android/app/build/outputs/mapping/release/mapping.txt
```

### Cách tải lên Google Play Console
1. Vào **Play Console** → ứng dụng **Massage Chair Control**.
2. Vào **Release** → **Production** (hoặc bản testing tương ứng).
3. Chọn bản release có AAB vừa tải lên → **App bundle explorer** (hoặc mục tương ứng phiên bản đó).
4. Tìm mục **Gỡ rối** / **Debug files** / **Deobfuscation file**.
5. **Tải lên** file `mapping.txt` (đúng với versionCode của bản release đó).

Sau khi tải mapping lên, cảnh báo *"Không có tệp gỡ rối mã nguồn nào liên kết với App Bundle"* sẽ được xử lý và bạn có thể đọc được stack trace đã gỡ rối trong báo cáo crash/ANR.

**Lưu ý:** Mỗi phiên bản (versionCode) cần một file mapping riêng; nên lưu lại bản copy của `mapping.txt` cho từng bản đã phát hành.

---

*File này dùng để ghi lại ghi chú phát hành cho từng bản build. Cập nhật mục tương ứng mỗi khi phát hành phiên bản mới.*
