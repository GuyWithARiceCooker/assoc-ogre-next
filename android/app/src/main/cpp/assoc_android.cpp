#include <jni.h>

#include <sstream>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_hu_assoc_next_MainActivity_nativeStatus( JNIEnv* env, jclass )
{
    std::ostringstream out;
    out << "assoc Android preview\n\n";
    out << "Native C++ bridge is running through the Android NDK.\n";
    out << "The desktop Ogre-Next/Metal renderer is still macOS-only in this repo;\n";
    out << "porting the renderer requires an Android Ogre-Next build and an EGL/GLES/Vulkan window path.";

    const std::string text = out.str();
    return env->NewStringUTF( text.c_str() );
}
