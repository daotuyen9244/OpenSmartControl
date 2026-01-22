# 📋 Metadata Changes for App Store Compliance

## ✅ **Đã cập nhật trong code:**

### **1. Display Names (Tên hiển thị)**
- ✅ `app.json`: `displayName: "Massage Chair Control"`
- ✅ `ios/VPG_MS_Redux/Info.plist`: 
  - `CFBundleDisplayName = "Massage Chair Control"` (tên hiển thị trên home screen)
  - `CFBundleName = "Massage Chair Control"` (tên trong App Store Connect - **QUAN TRỌNG**)
- ✅ `android/app/src/main/res/values/strings.xml`: `app_name = "Massage Chair Control"`
- ✅ `ios/VPG_MS_Redux.xcodeproj/project.pbxproj`: `PRODUCT_NAME = "Massage Chair Control"` (đã đổi từ VPG_MS_Redux)

### **2. Bundle Identifiers & Package Names**
- ✅ **iOS Bundle ID**: `com.massagechaircontrol.app` (đã đổi từ `org.reactjs.native.example.$(PRODUCT_NAME:rfc1034identifier)`)
- ✅ **Android Package**: `com.massagechaircontrol` (đã đổi từ `com.vpg_ms_redux`)
- ✅ **Android Files**: Đã di chuyển sang package mới `com.massagechaircontrol`

### **3. User-Facing Content**
- ✅ QR Code examples: `VPG_MASSAGE` → `MASSAGE_DEVICE`
- ✅ Error messages: Tất cả references đến `VPG_MS_Redux` → `Massage Chair Control`
- ✅ Settings references: Đã cập nhật

### **4. Launch Screen (iOS)**
- ✅ `ios/VPG_MS_Redux/LaunchScreen.storyboard`: Đã đổi text từ "VPG_MS_Redux" → "Massage Chair Control"
- ✅ Đây là màn hình splash hiển thị khi app khởi động trên iOS

### **5. Documentation**
- ✅ `QR_CODE_FORMAT.md`: Đã cập nhật tất cả examples
- ✅ `APP_STORE_COMPLIANCE.md`: Hướng dẫn chi tiết

---

## ⚠️ **LƯU Ý QUAN TRỌNG:**

### **Technical Names (Đã cập nhật):**
- ✅ `PRODUCT_NAME = "Massage Chair Control"` (đã đổi từ VPG_MS_Redux) - **QUAN TRỌNG cho App Store Connect**
- ✅ `CFBundleName = "Massage Chair Control"` (đã set trực tiếp, không dùng $(PRODUCT_NAME)) - **QUAN TRỌNG cho archive**

**Các tên technical/internal còn lại (KHÔNG ảnh hưởng App Store):**
- ✅ `withModuleName: "VPG_MS_Redux"` (trong AppDelegate.swift) - OK (internal)
- ✅ `getMainComponentName(): "VPG_MS_Redux"` (trong MainActivity.kt) - OK (internal)
- ✅ Project folder names `VPG_MS_Redux.xcodeproj` - OK (internal)
- ✅ Package.json `name: "VPG_MS_Redux"` - OK (internal)

**Lý do:** `PRODUCT_NAME` và `CFBundleName` được App Store Connect sử dụng khi archive, nên cần đổi. Các tên khác là internal và không ảnh hưởng.

---

## 📱 **CẦN CẬP NHẬT TRONG APP STORE CONNECT:**

### **Guideline 2.3 - Accurate Metadata:**

1. **App Name** (trong App Store Connect):
   - ✅ Phải khớp với `CFBundleDisplayName` = **"Massage Chair Control"**
   - ❌ KHÔNG dùng "VPG_MS_Redux" hoặc bất kỳ mention nào về "VPG"

2. **App Description**:
   - ❌ KHÔNG mention "VPG", "VPG Massage", "VPG Hardware"
   - ✅ Dùng generic descriptions: "Massage Chair Control", "Smart Massage Controller"

3. **Screenshots**:
   - ❌ KHÔNG có logo/text "VPG" trong screenshots
   - ✅ Nếu có, cần blur hoặc replace

4. **Keywords**:
   - ❌ KHÔNG dùng: "VPG", "VPG Massage"
   - ✅ Dùng: "massage", "chair", "control", "bluetooth", "smart"

5. **Version & Build Number**:
   - ✅ Đảm bảo version trong App Store Connect khớp với `MARKETING_VERSION` trong Xcode
   - ✅ Hiện tại: `MARKETING_VERSION = 1.4`

### **Guideline 5.2.1 - Intellectual Property:**

1. **App Review Information**:
   - Nếu **CÓ QUYỀN** sử dụng VPG: Upload documentary evidence
   - Nếu **KHÔNG CÓ**: App đã clean (không còn VPG trong user-facing content)

2. **Metadata Consistency**:
   - ✅ App name trong App Store Connect = Display name trong app
   - ✅ Description không mention third-party brands
   - ✅ Screenshots không có third-party logos

---

## 🔍 **Kiểm tra trước khi Submit:**

```bash
# 1. Kiểm tra display names
grep -r "CFBundleDisplayName\|app_name" ios/ android/app/src/main/res/

# 2. Kiểm tra bundle identifiers
grep -r "PRODUCT_BUNDLE_IDENTIFIER\|applicationId" ios/ android/

# 3. Kiểm tra user-facing VPG references (KHÔNG nên có)
grep -r "VPG" src/ --exclude-dir=node_modules | grep -v "VPG_MS_Redux" | grep -v "technical"
```

---

## ✅ **Checklist trước khi Resubmit:**

- [x] Display name = "Massage Chair Control" (iOS & Android)
- [x] Bundle ID iOS = "com.massagechaircontrol.app"
- [x] Package name Android = "com.massagechaircontrol"
- [x] Không còn "VPG" trong user-facing messages
- [x] QR examples sử dụng "MASSAGE_DEVICE"
- [ ] **App Store Connect**: App name = "Massage Chair Control"
- [ ] **App Store Connect**: Description không có "VPG"
- [ ] **App Store Connect**: Keywords không có "VPG"
- [ ] **App Store Connect**: Screenshots không có "VPG"
- [ ] **App Store Connect**: Version khớp với Xcode (1.4)

---

**Ngày cập nhật:** 2025-01-07
