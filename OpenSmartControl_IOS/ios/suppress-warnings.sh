#!/bin/bash
# Suppress deprecation warnings in Xcode

echo "Suppressing iOS deprecation warnings..."

# Add to Podfile post_install
cat >> ios/Podfile << 'PODFILE'

  # Suppress deprecation warnings
  installer.pods_project.targets.each do |target|
    target.build_configurations.each do |config|
      config.build_settings['GCC_WARN_DEPRECATED_DECLARATIONS'] = 'NO'
      config.build_settings['CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS'] = 'NO'
    end
  end
PODFILE

echo "✅ Done! Run: cd ios && pod install"
