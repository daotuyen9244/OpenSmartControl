# 📱 App Store Compliance - Intellectual Property Guidelines

## ✅ **Đã thay đổi trong code:**

### **1. Display Name (Tên hiển thị)**
- ✅ iOS: Đổi từ `VPG_MS_Redux` → `Massage Chair Control`
- ✅ Android: Đổi từ `VPG_MS_Redux` → `Massage Chair Control`
- ✅ app.json: Đã cập nhật displayName

### **2. Device Name trong QR Code**
- ✅ Đổi từ `VPG_MASSAGE` → `MASSAGE_DEVICE` trong examples
- ✅ Cập nhật error messages và placeholders

### **3. Settings References**
- ✅ Đổi tất cả references từ `VPG_MS_Redux` → `Massage Chair Control` trong error messages

---

## ⚠️ **CẦN CẬP NHẬT TRONG APP STORE CONNECT:**

### **Bước 1: Cập nhật App Information**

1. Đăng nhập vào [App Store Connect](https://appstoreconnect.apple.com)
2. Chọn app của bạn
3. Vào tab **App Information**
4. Cập nhật:

   - **Name**: `Massage Chair Control` (hoặc tên generic khác, KHÔNG dùng "VPG")
   - **Subtitle**: `Smart Massage Control` (KHÔNG đề cập đến VPG)
   - **Category**: Chọn category phù hợp

### **Bước 2: Cập nhật App Description**

Trong tab **App Store** > **App Description**:

**❌ TRÁNH:**
- Bất kỳ mention nào về "VPG"
- "VPG Massage Chair"
- "VPG Hardware"
- Brand names của bên thứ ba

**✅ SỬ DỤNG:**
- "Massage Chair Control"
- "Smart Massage Controller"
- "Massage Device Control"
- Generic descriptions

**Ví dụ description:**
```
Massage Chair Control is a smart control application that allows you to connect 
and control your Bluetooth-enabled massage chair through an intuitive mobile interface.

Features:
- Quick QR code pairing for easy device connection
- Manual Bluetooth device discovery and connection
- Full massage control with customizable settings
- Real-time connection status monitoring
- Easy-to-use interface designed for comfort

Connect effortlessly with your massage device and enjoy personalized massage 
therapy at your fingertips.
```

### **Bước 3: Keywords**

Trong **Keywords** section:
- ❌ KHÔNG dùng: "VPG", "VPG Massage", "VPG Chair"
- ✅ DÙNG: "massage", "chair", "control", "bluetooth", "smart", "therapy"

### **Bước 4: Screenshots & Preview Video**

Kiểm tra screenshots và preview video:
- ❌ KHÔNG có logo/brand "VPG" nào trong screenshots
- ❌ KHÔNG có text "VPG" trong UI screenshots
- ✅ Nếu có, cần blur hoặc replace

### **Bước 5: App Review Information**

Nếu bạn **CÓ QUYỀN** sử dụng brand VPG:
- ✅ Upload **documentary evidence** trong App Review Information:
  - License agreement
  - Authorization letter từ VPG
  - Trademark license
  - Any official documentation

Nếu bạn **KHÔNG CÓ QUYỀN**:
- ✅ Đã xóa tất cả references (như đã làm)
- ✅ App hiện tại không còn mention VPG trong user-facing content

---

## 📋 **Checklist trước khi Submit:**

- [ ] App name trong App Store Connect = "Massage Chair Control" (KHÔNG có VPG)
- [ ] App description KHÔNG mention VPG
- [ ] Keywords KHÔNG có VPG
- [ ] Screenshots KHÔNG có logo/text VPG
- [ ] Preview video KHÔNG có VPG (nếu có)
- [ ] Display name trong app = "Massage Chair Control"
- [ ] Tất cả error messages đã được cập nhật
- [ ] QR code examples sử dụng generic name
- [ ] Đã build và test app với tên mới

---

## 🔍 **Kiểm tra lại trong code:**

Sau khi build lại, kiểm tra:
```bash
# Tìm bất kỳ VPG references nào còn sót (user-facing)
grep -r "VPG" src/ --exclude-dir=node_modules
grep -r "VPG" ios/VPG_MS_Redux/Info.plist
```

Lưu ý: Technical names (như project name `VPG_MS_Redux`) trong code là OK - Apple chỉ quan tâm đến user-facing content.

---

## ✅ **Kết quả:**

App hiện tại đã:
- ✅ Không còn "VPG" trong display name
- ✅ Không còn "VPG" trong user-facing messages
- ✅ Không còn "VPG" trong QR examples
- ✅ Sử dụng generic names: "Massage Chair Control" và "MASSAGE_DEVICE"

Sau khi cập nhật App Store Connect metadata và resubmit, app sẽ pass guideline 5.2.1.

---

**Ngày cập nhật:** 2025-01-07
