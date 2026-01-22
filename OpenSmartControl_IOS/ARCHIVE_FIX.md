# 🔧 Fix: App Name trong App Store Connect

## ❌ **Vấn Đề:**

Khi archive và upload lên App Store Connect, app hiển thị tên **"VPG_MS_Redux"** thay vì **"Massage Chair Control"**, dẫn đến bị Apple từ chối (Guideline 5.2.1 - Intellectual Property).

## ✅ **Nguyên Nhân:**

App Store Connect lấy tên app từ:
1. **`CFBundleName`** trong `Info.plist` (quan trọng nhất khi archive)
2. **`PRODUCT_NAME`** trong Xcode project settings
3. **`CFBundleDisplayName`** (chỉ dùng cho home screen, không dùng cho App Store)

**Trước khi fix:**
- `CFBundleName = $(PRODUCT_NAME)` → resolve thành `VPG_MS_Redux`
- `PRODUCT_NAME = VPG_MS_Redux`
- → App Store Connect hiển thị: **"VPG_MS_Redux"** ❌

## 🔧 **Đã Sửa:**

### **1. Info.plist**
```xml
<!-- TRƯỚC -->
<key>CFBundleName</key>
<string>$(PRODUCT_NAME)</string>

<!-- SAU -->
<key>CFBundleName</key>
<string>Massage Chair Control</string>
```

### **2. project.pbxproj**
```bash
# TRƯỚC
PRODUCT_NAME = VPG_MS_Redux;

# SAU
PRODUCT_NAME = "Massage Chair Control";
```

## ✅ **Kết Quả:**

Sau khi fix:
- ✅ `CFBundleName = "Massage Chair Control"` (trực tiếp, không dùng variable)
- ✅ `PRODUCT_NAME = "Massage Chair Control"`
- ✅ `CFBundleDisplayName = "Massage Chair Control"`
- → App Store Connect sẽ hiển thị: **"Massage Chair Control"** ✅

---

## 📋 **Checklist Trước Khi Archive:**

### **1. Kiểm tra trong Xcode:**
- [ ] Mở Xcode project
- [ ] Chọn target → Build Settings
- [ ] Tìm `PRODUCT_NAME` → Phải là `"Massage Chair Control"`
- [ ] Tìm `PRODUCT_BUNDLE_IDENTIFIER` → Phải là `com.massagechaircontrol.app`

### **2. Kiểm tra Info.plist:**
```bash
# Chạy lệnh này để verify
grep -A 1 "CFBundleName\|CFBundleDisplayName" ios/VPG_MS_Redux/Info.plist
```

**Kết quả mong đợi:**
```
CFBundleDisplayName = "Massage Chair Control"
CFBundleName = "Massage Chair Control"
```

### **3. Clean Build:**
```bash
# Trong Xcode:
# Product > Clean Build Folder (Shift+Cmd+K)
```

### **4. Archive:**
1. Chọn **Any iOS Device** (không phải Simulator)
2. Product > Archive
3. Verify trong Organizer:
   - App name phải là **"Massage Chair Control"**
   - Bundle ID phải là **com.massagechaircontrol.app**

### **5. Upload và Verify:**
1. Upload archive lên App Store Connect
2. Vào App Store Connect → App Information
3. Verify:
   - App name tự động = **"Massage Chair Control"** ✅
   - Nếu vẫn hiển thị "VPG_MS_Redux", cần:
     - Xóa archive cũ
     - Clean build lại
     - Archive lại

---

## ⚠️ **Lưu Ý Quan Trọng:**

### **Nếu Đã Upload Archive Cũ:**
1. **Xóa archive cũ** trong App Store Connect (nếu có thể)
2. **Archive lại** với settings mới
3. **Upload archive mới**

### **Nếu App Đã Có Trong App Store Connect:**
1. Vào **App Information** trong App Store Connect
2. **Manually update** App Name = "Massage Chair Control"
3. **Save changes**
4. Archive và upload build mới

### **Bundle ID:**
- ✅ Đã đổi từ example bundle ID → `com.massagechaircontrol.app`
- ✅ Nếu app mới (chưa submit), bundle ID mới là OK
- ⚠️ Nếu app cũ đã submit, bundle ID mới = **app mới** (cần tạo app mới trong App Store Connect)

---

## 🔍 **Verify Sau Khi Archive:**

### **Cách 1: Kiểm tra trong Xcode Organizer**
1. Window > Organizer
2. Chọn archive vừa tạo
3. Click "Distribute App"
4. Xem app name trong summary → Phải là **"Massage Chair Control"**

### **Cách 2: Kiểm tra trong App Store Connect**
1. Upload archive
2. Vào App Information
3. App Name phải tự động = **"Massage Chair Control"**

### **Cách 3: Kiểm tra trong .ipa file (nếu cần)**
```bash
# Extract .ipa và check Info.plist
unzip YourApp.ipa
plutil -p Payload/YourApp.app/Info.plist | grep -i "CFBundleName\|CFBundleDisplayName"
```

---

## ✅ **Kết Luận:**

Sau khi fix:
- ✅ App name trong archive = **"Massage Chair Control"**
- ✅ App Store Connect sẽ hiển thị đúng tên
- ✅ Pass Apple review (Guideline 5.2.1)

**Next Steps:**
1. Clean build trong Xcode
2. Archive lại
3. Upload lên App Store Connect
4. Verify app name trong App Store Connect
5. Submit for review

---

**Ngày fix:** 2025-01-07
