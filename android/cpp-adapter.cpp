#include <jni.h>
#include "jsi/jsi.h"
#include "react-native-ultra-base64.h"


extern "C"
JNIEXPORT void JNICALL
Java_com_ultrabase64_UltraBase64Module_nativeInstall (JNIEnv* /*env*/, jobject /*thiz*/, jlong runtimePtr) {
  auto* rt = reinterpret_cast<facebook::jsi::Runtime*>(runtimePtr);
  if (!rt) return;
  rnub_base64::install(rt);
}
