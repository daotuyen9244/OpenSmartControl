#!/bin/bash
# Script to fix module map build errors

set -e

export LANG=en_US.UTF-8
export LC_ALL=en_US.UTF-8

echo "🔧 Fixing iOS build errors..."
echo ""

# Change to ios directory
cd "$(dirname "$0")"

# Step 1: Clean Xcode build
echo "📦 Step 1: Cleaning pods..."
if [ -d "Pods" ]; then
    rm -rf Pods
    echo "  ✅ Removed Pods directory"
fi

if [ -f "Podfile.lock" ]; then
    rm -f Podfile.lock
    echo "  ✅ Removed Podfile.lock"
fi

# Step 2: Clean build folders
echo ""
echo "🧹 Step 2: Cleaning build folders..."
rm -rf build
rm -rf ../build
echo "  ✅ Cleaned build folders"

# Step 3: Install pods
echo ""
echo "📥 Step 3: Installing pods..."
pod install --repo-update

echo ""
echo "✅ Done! Now:"
echo "   1. Close Xcode completely"
echo "   2. Open Xcode"
echo "   3. Product > Clean Build Folder (Shift+Cmd+K)"
echo "   4. Product > Build (Cmd+B)"
