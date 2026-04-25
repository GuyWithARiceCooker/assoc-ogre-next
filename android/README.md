# Android build

This directory contains an Android/NDK preview target for assoc.

The existing root CMake target is a macOS Ogre-Next/Metal desktop app. This
Android target is intentionally a small native scaffold that proves the Android
packaging, JNI loading, and install/run flow. Porting the Ogre renderer itself
requires an Ogre-Next Android build and an Android window/input entry point.

## Build

From this directory:

```sh
./gradlew :app:assembleDebug
```

The debug APK is written to:

```text
app/build/outputs/apk/debug/app-debug.apk
```

## Install and run on a phone

With Android platform tools installed on the machine that can see the phone:

```sh
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell monkey -p hu.assoc.next 1
```

