#!/usr/bin/env python3
"""
Script để generate iOS App Icons từ 1 file ảnh gốc
Yêu cầu: Python 3 + Pillow (PIL)
"""

import os
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    print("❌ Cần cài đặt Pillow:")
    print("   pip3 install Pillow")
    sys.exit(1)

# iOS icon sizes cần thiết
ICON_SIZES = [
    ("20x20@2x", 40),
    ("20x20@3x", 60),
    ("29x29@2x", 58),
    ("29x29@3x", 87),
    ("40x40@2x", 80),
    ("40x40@3x", 120),
    ("60x60@2x", 120),
    ("60x60@3x", 180),
    ("1024x1024", 1024),  # App Store icon
]

def generate_icons(source_image_path, output_dir):
    """
    Generate tất cả icon sizes từ ảnh gốc
    
    Args:
        source_image_path: Path đến ảnh gốc (nên là 1024x1024 PNG)
        output_dir: Thư mục output (AppIcon.appiconset)
    """
    if not os.path.exists(source_image_path):
        print(f"❌ Không tìm thấy file: {source_image_path}")
        return False
    
    # Mở ảnh gốc
    try:
        img = Image.open(source_image_path)
        
        # Convert sang RGBA nếu cần
        if img.mode != 'RGBA':
            img = img.convert('RGBA')
        
        print(f"✅ Đã mở ảnh gốc: {img.size[0]}x{img.size[1]}")
        
    except Exception as e:
        print(f"❌ Lỗi khi mở ảnh: {e}")
        return False
    
    # Tạo output directory nếu chưa có
    os.makedirs(output_dir, exist_ok=True)
    
    # Generate từng size
    success_count = 0
    for name, size in ICON_SIZES:
        try:
            # Resize ảnh
            resized = img.resize((size, size), Image.Resampling.LANCZOS)
            
            # Tên file output
            if name == "1024x1024":
                output_filename = "icon-1024.png"
            else:
                output_filename = f"icon-{name}.png"
            
            output_path = os.path.join(output_dir, output_filename)
            
            # Lưu file
            resized.save(output_path, "PNG")
            print(f"  ✅ {output_filename}")
            success_count += 1
            
        except Exception as e:
            print(f"  ❌ Lỗi tạo {name}: {e}")
    
    print(f"\n✅ Đã tạo {success_count}/{len(ICON_SIZES)} icons")
    return success_count == len(ICON_SIZES)

def update_contents_json(output_dir):
    """Update Contents.json với filenames"""
    contents = {
        "images": [
            {"filename": "icon-20x20@2x.png", "idiom": "iphone", "scale": "2x", "size": "20x20"},
            {"filename": "icon-20x20@3x.png", "idiom": "iphone", "scale": "3x", "size": "20x20"},
            {"filename": "icon-29x29@2x.png", "idiom": "iphone", "scale": "2x", "size": "29x29"},
            {"filename": "icon-29x29@3x.png", "idiom": "iphone", "scale": "3x", "size": "29x29"},
            {"filename": "icon-40x40@2x.png", "idiom": "iphone", "scale": "2x", "size": "40x40"},
            {"filename": "icon-40x40@3x.png", "idiom": "iphone", "scale": "3x", "size": "40x40"},
            {"filename": "icon-60x60@2x.png", "idiom": "iphone", "scale": "2x", "size": "60x60"},
            {"filename": "icon-60x60@3x.png", "idiom": "iphone", "scale": "3x", "size": "60x60"},
            {"filename": "icon-1024.png", "idiom": "ios-marketing", "scale": "1x", "size": "1024x1024"}
        ],
        "info": {
            "author": "xcode",
            "version": 1
        }
    }
    
    import json
    contents_path = os.path.join(output_dir, "Contents.json")
    with open(contents_path, 'w') as f:
        json.dump(contents, f, indent=2)
    
    print("✅ Đã cập nhật Contents.json")

if __name__ == "__main__":
    print("🎨 iOS App Icon Generator")
    print("=" * 50)
    
    # Kiểm tra arguments
    if len(sys.argv) < 2:
        print("\n📋 Cách dùng:")
        print("   python3 generate-icons.py <đường-dẫn-đến-icon-1024x1024.png>")
        print("\n📌 Ví dụ:")
        print("   python3 generate-icons.py icon.png")
        print("   python3 generate-icons.py ~/Desktop/app-icon.png")
        print("\n💡 Lưu ý:")
        print("   - Icon gốc nên là PNG 1024x1024")
        print("   - Nền trong suốt (transparent)")
        print("   - Không có góc bo tròn (iOS tự bo)")
        sys.exit(1)
    
    source_image = sys.argv[1]
    output_dir = "ios/MassageChairControl/Images.xcassets/AppIcon.appiconset"
    
    print(f"\n📂 Input:  {source_image}")
    print(f"📂 Output: {output_dir}")
    print()
    
    # Generate icons
    if generate_icons(source_image, output_dir):
        update_contents_json(output_dir)
        print("\n" + "=" * 50)
        print("🎉 HOÀN TẤT! App icons đã được tạo.")
        print("\n📱 Bước tiếp theo:")
        print("   1. Mở Xcode")
        print("   2. Rebuild app")
        print("   3. Icon mới sẽ xuất hiện trên màn hình Home!")
        print("=" * 50)
    else:
        print("\n❌ Có lỗi xảy ra. Vui lòng kiểm tra lại.")
        sys.exit(1)

