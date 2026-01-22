#!/bin/bash
# Script to clean and rebuild iOS project

set -e

export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

echo "🧹 Cleaning iOS build..."
echo ""

cd "$(dirname "$0")"

# Step 1: Kill Xcode if running (optional)
echo "📦 Step 1: Checking Xcode..."
if pgrep -x "Xcode" > /dev/null; then
    echo "  ⚠️  Xcode is running. Please close it first."
    echo "  💡 Press Cmd+Q to quit Xcode completely"
    exit 1
fi

# Step 2: Clean DerivedData
echo ""
echo "🗑️  Step 2: Cleaning DerivedData..."
rm -rf ~/Library/Developer/Xcode/DerivedData/MassageChairControl-* 2>/dev/null || true
echo "  ✅ Cleaned DerivedData"

# Step 3: Clean build folders
echo ""
echo "🧹 Step 3: Cleaning build folders..."
rm -rf build
rm -rf ../build
echo "  ✅ Cleaned build folders"

# Step 4: Clean pods
echo ""
echo "📦 Step 4: Cleaning pods..."
if [ -d "Pods" ]; then
    rm -rf Pods
    echo "  ✅ Removed Pods directory"
fi

if [ -f "Podfile.lock" ]; then
    rm -f Podfile.lock
    echo "  ✅ Removed Podfile.lock"
fi

# Step 5: Clean CocoaPods cache
echo ""
echo "🗑️  Step 5: Cleaning CocoaPods cache..."
pod cache clean --all 2>/dev/null || true
echo "  ✅ Cleaned CocoaPods cache"

# Step 6: Install pods
echo ""
echo "📥 Step 6: Installing pods..."
pod install --repo-update

echo ""
echo "✅ Done! Now:"
echo "   1. Open Xcode: open MassageChairControl.xcworkspace"
echo "   2. Product > Clean Build Folder (Shift+Cmd+K)"
echo "   3. Product > Build (Cmd+B)"
