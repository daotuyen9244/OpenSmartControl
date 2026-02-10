# ProGuard rules cho Massage Chair Control (React Native)
# Giúp R8 không strip/đổi tên các class cần cho reflection (RN, native modules).

# ========== React Native (đồng bộ với ReactAndroid) ==========
-keep,allowobfuscation @interface com.facebook.proguard.annotations.DoNotStrip
-keep @com.facebook.proguard.annotations.DoNotStrip class *
-keepclassmembers class * {
    @com.facebook.proguard.annotations.DoNotStrip *;
}
-keep @com.facebook.proguard.annotations.DoNotStripAny class * { *; }
-keep @com.facebook.jni.annotations.DoNotStrip class *
-keepclassmembers class * {
    @com.facebook.jni.annotations.DoNotStrip *;
}
-keep @com.facebook.jni.annotations.DoNotStripAny class * { *; }

-keep class * implements com.facebook.react.bridge.JavaScriptModule { *; }
-keep class * implements com.facebook.react.bridge.NativeModule { *; }
-keepclassmembers,includedescriptorclasses class * { native <methods>; }
-keepclassmembers class * { @com.facebook.react.uimanager.annotations.ReactProp <methods>; }
-keepclassmembers class * { @com.facebook.react.uimanager.annotations.ReactPropGroup <methods>; }

-dontwarn com.facebook.react.**
-keep,includedescriptorclasses class com.facebook.react.bridge.** { *; }
-keep,includedescriptorclasses class com.facebook.react.turbomodule.core.** { *; }
-keep,includedescriptorclasses class com.facebook.react.internal.turbomodule.core.** { *; }

# Hermes
-keep class com.facebook.jni.** { *; }

# Yoga
-keep,allowobfuscation @interface com.facebook.yoga.annotations.DoNotStrip
-keep @com.facebook.yoga.annotations.DoNotStrip class *
-keepclassmembers class * { @com.facebook.yoga.annotations.DoNotStrip *; }

# Okio
-keep class sun.misc.Unsafe { *; }
-dontwarn okio.**

# ========== App & native modules (BLE, Camera, v.v.) ==========
-keep class com.vpg_ms_redux.** { *; }

# React Native BLE PLX, Vision Camera, Permissions, v.v. (native modules)
-keep class com.polidea.** { *; }
-keep class com.mrousavy.** { *; }
-keep class com.zoontek.** { *; }
