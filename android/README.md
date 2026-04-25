# Android build

This directory contains an Android/NDK target for assoc.

The existing root CMake target is a macOS Ogre-Next/Metal desktop app. This
Android target is a native GLES renderer that proves the Android packaging, JNI
loading, rendering, and install/run flow. Porting the Ogre renderer itself
requires an Ogre-Next Android build and the Android Vulkan window path.

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

## Preparing Ogre-Next for Android

The helper below follows the upstream Ogre-Next 2.3 Android flow for arm64-v8a:
build `ogre-next-deps`, then build/install `ogre-next` with
`OGRE_BUILD_PLATFORM_ANDROID=1`.

```sh
ANDROID_SDK_ROOT=/path/to/Android/Sdk ./tools/build-ogre-next-android.sh
```

Outputs are created under `android/.ogre-android/`, which is
ignored by git because it contains large source and build trees.
