#!/bin/bash
# Generate iOS App Icons using macOS sips tool

SOURCE_IMAGE="icon.png"
OUTPUT_DIR="ios/MassageChairControl/Images.xcassets/AppIcon.appiconset"

echo "🎨 Generating iOS App Icons..."
echo "Input: $SOURCE_IMAGE"
echo "Output: $OUTPUT_DIR"
echo ""

# Tạo output directory
mkdir -p "$OUTPUT_DIR"

# iOS icon sizes
declare -a SIZES=(
  "40:icon-20x20@2x.png"
  "60:icon-20x20@3x.png"
  "58:icon-29x29@2x.png"
  "87:icon-29x29@3x.png"
  "80:icon-40x40@2x.png"
  "120:icon-40x40@3x.png"
  "120:icon-60x60@2x.png"
  "180:icon-60x60@3x.png"
  "1024:icon-1024.png"
)

# Generate từng size
for item in "${SIZES[@]}"; do
  IFS=':' read -r size filename <<< "$item"
  echo "  ✅ Generating $filename (${size}x${size})"
  sips -z $size $size "$SOURCE_IMAGE" --out "$OUTPUT_DIR/$filename" > /dev/null 2>&1
done

echo ""
echo "✅ Đã tạo xong ${#SIZES[@]} icon files!"
echo ""
echo "📱 Bước tiếp theo:"
echo "   1. Mở Xcode"
echo "   2. Clean Build (Shift+Cmd+K)"
echo "   3. Rebuild app"
echo ""
