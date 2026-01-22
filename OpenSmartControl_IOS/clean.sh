#!/bin/bash

# Configuration
APK_DIR="MSMoblieApp_Fix/apk"
PACKAGE_NAME="com.vpg_ms_redux"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to clean build artifacts
clean_build() {
    print_status "🧹 Cleaning React Native Android build..."
    
    # Xoá node_modules
    rm -rf node_modules
    print_success "Removed node_modules"
    
    # Xoá cache Metro bundler
    rm -rf $TMPDIR/metro-*
    rm -rf $TMPDIR/react-*
    print_success "Cleared Metro cache"
    
    # Xoá build Android
    rm -rf android/build
    rm -rf android/app/build
    rm -rf android/.gradle
    rm -rf android/app/.cxx
    print_success "Removed Android build caches"
    
    # Xoá Gradle cache trong user (nếu cần, cẩn thận sẽ tải lại lib khá lâu)
    # rm -rf ~/.gradle/caches/
}

# Function to install dependencies
install_dependencies() {
    print_status "📦 Installing dependencies..."
    npm install
    print_success "Installed node_modules"
}

# Function to build debug APK
build_debug() {
    print_status "🔨 Building DEBUG APK..."
    
    # Build APK debug
    cd android
    ./gradlew clean
    ./gradlew assembleDebug
    cd ..
    
    # Create APK directory
    mkdir -p "$APK_DIR"
    
    # Copy APK with timestamp
    cp android/app/build/outputs/apk/debug/app-debug.apk "$APK_DIR/app-debug-$TIMESTAMP.apk"
    cp android/app/build/outputs/apk/debug/app-debug.apk "$APK_DIR/app-debug.apk"
    
    print_success "DEBUG APK built: $APK_DIR/app-debug-$TIMESTAMP.apk"
}

# Function to build release APK
build_release() {
    print_status "🚀 Building RELEASE APK with bundled JS..."
    
    # Generate bundle for release (JS code bundled into APK)
    print_status "📦 Generating JS bundle for release..."
    npx react-native bundle --platform android --dev false --entry-file index.js --bundle-output android/app/src/main/assets/index.android.bundle --assets-dest android/app/src/main/res/
    
    # Create assets directory if it doesn't exist
    mkdir -p android/app/src/main/assets
    
    # Build APK release
    cd android
    ./gradlew clean
    ./gradlew assembleRelease
    cd ..
    
    # Create APK directory
    mkdir -p "$APK_DIR"
    
    # Copy APK with timestamp
    cp android/app/build/outputs/apk/release/app-release.apk "$APK_DIR/app-release-$TIMESTAMP.apk"
    cp android/app/build/outputs/apk/release/app-release.apk "$APK_DIR/app-release.apk"
    
    print_success "RELEASE APK built with bundled JS: $APK_DIR/app-release-$TIMESTAMP.apk"
    print_status "📱 APK includes:"
    print_status "   - Optimized JS bundle (Hermes compiled)"
    print_status "   - Minified code for production"
    print_status "   - All assets bundled"
    print_status "   - No external JS dependencies"
}

# Function to install and run debug app
install_debug() {
    print_status "📱 Installing DEBUG app..."
    
    # Gỡ app cũ (tránh lỗi installDebug fail)
    adb uninstall $PACKAGE_NAME || true
    
    # Build và install app debug
    npx react-native run-android
    
    print_success "DEBUG app installed and running"
}

# Function to show help
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -c, --clean             Clean build artifacts only"
    echo "  -d, --debug             Build debug APK and install"
    echo "  -r, --release           Build release APK only"
    echo "  -a, --all               Clean + build both debug and release APKs"
    echo "  -i, --install           Install debug app to device"
    echo ""
    echo "Examples:"
    echo "  $0 -c                   # Clean only"
    echo "  $0 -d                   # Build debug APK and install"
    echo "  $0 -r                   # Build release APK only"
    echo "  $0 -a                   # Clean + build both APKs"
    echo "  $0 -i                   # Install debug app"
}

# Main script logic
main() {
    # Parse command line arguments
    case "${1:-}" in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            clean_build
            exit 0
            ;;
        -d|--debug)
            clean_build
            install_dependencies
            build_debug
            install_debug
            ;;
        -r|--release)
            clean_build
            install_dependencies
            build_release
            ;;
        -a|--all)
            clean_build
            install_dependencies
            build_debug
            build_release
            ;;
        -i|--install)
            install_debug
            ;;
        "")
            # Default: clean + debug build + install
            clean_build
            install_dependencies
            build_debug
            install_debug
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
    
    print_success "✅ All operations completed!"
}

# Run main function
main "$@"
