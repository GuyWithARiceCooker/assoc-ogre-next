#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WORK_DIR="${OGRE_ANDROID_WORK_DIR:-$ROOT/third_party/ogre-next-android}"
SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-}}"
ABI="${ANDROID_ABI:-arm64-v8a}"
API_LEVEL="${ANDROID_API_LEVEL:-24}"
BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}"

if [[ -z "$SDK_ROOT" ]]; then
  echo "ANDROID_SDK_ROOT or ANDROID_HOME must point at an Android SDK." >&2
  exit 1
fi

NDK_DIR="$(find "$SDK_ROOT/ndk" -mindepth 1 -maxdepth 1 -type d | sort -V | tail -n 1)"
if [[ -z "$NDK_DIR" || ! -f "$NDK_DIR/build/cmake/android.toolchain.cmake" ]]; then
  echo "No Android NDK found under $SDK_ROOT/ndk." >&2
  exit 1
fi

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

if [[ ! -d ogre-next-deps/.git ]]; then
  git clone --recurse-submodules --shallow-submodules https://github.com/OGRECave/ogre-next-deps
fi

if [[ ! -d ogre-next/.git ]]; then
  git clone --branch master https://github.com/OGRECave/ogre-next
fi

FREEIMAGE_SIMPLE_TOOLS="ogre-next-deps/src/FreeImage/Source/FreeImage/SimpleTools.h"
if [[ -f "$FREEIMAGE_SIMPLE_TOOLS" ]] && ! grep -q '^#include <algorithm>$' "$FREEIMAGE_SIMPLE_TOOLS"; then
  python3 - "$FREEIMAGE_SIMPLE_TOOLS" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()
text = text.replace("#include \"FreeImage.h\"\n", "#include \"FreeImage.h\"\n#include <algorithm>\n", 1)
path.write_text(text)
PY
fi

mkdir -p "ogre-next-deps/build/Android/$BUILD_TYPE"
cmake -S ogre-next-deps -B "ogre-next-deps/build/Android/$BUILD_TYPE" \
  -DCMAKE_TOOLCHAIN_FILE="$NDK_DIR/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_NATIVE_API_LEVEL="$API_LEVEL" \
  -DCMAKE_CXX_STANDARD=14 \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "ogre-next-deps/build/Android/$BUILD_TYPE" --parallel "$JOBS"
cmake --install "ogre-next-deps/build/Android/$BUILD_TYPE"

rm -f ogre-next/DependenciesAndroid
ln -s "../ogre-next-deps/build/Android/$BUILD_TYPE/ogredeps" ogre-next/DependenciesAndroid

mkdir -p "ogre-next/build/Android/$BUILD_TYPE"
cmake -S ogre-next -B "ogre-next/build/Android/$BUILD_TYPE" \
  -DCMAKE_TOOLCHAIN_FILE="$NDK_DIR/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI="$ABI" \
  -DANDROID_NATIVE_API_LEVEL="$API_LEVEL" \
  -DOGRE_BUILD_PLATFORM_ANDROID=1 \
  -DOGRE_DEPENDENCIES_DIR="$WORK_DIR/ogre-next/DependenciesAndroid" \
  -DOGRE_BUILD_SAMPLES2=OFF \
  -DOGRE_SIMD_NEON=OFF \
  -DOGRE_SIMD_SSE2=OFF \
  -DCMAKE_INSTALL_PREFIX="$WORK_DIR/ogre-next/build/Android/$BUILD_TYPE/sdk" \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "ogre-next/build/Android/$BUILD_TYPE" --parallel "$JOBS"
cmake --install "ogre-next/build/Android/$BUILD_TYPE"

echo "Ogre-Next Android build installed under:"
echo "  $WORK_DIR/ogre-next/build/Android/$BUILD_TYPE/sdk"
