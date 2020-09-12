//
// Created by eevee on 9/11/20.
//
//#include "mediapipe/java/com/google/mediapipe/framework/jni/android_asset_util_jni.h"

#include "acu_jni.h"
#include "mediapipe/calculators/merge/acu_generator.h"
namespace{
    inline static JavaVM *g_vm = nullptr;

    inline std::string jstring2string(JNIEnv *env, jstring jStr){
        const char *cstr = env->GetStringUTFChars(jStr, NULL);
        std::string str = std::string(cstr);
        env->ReleaseStringUTFChars(jStr, cstr);
        return str;
    }
};
JNI_METHOD(void, JNIUpdateMeridianList)(JNIEnv* env, jclass, jint num, jobjectArray jkeys){
    for(int i=0; i<num; i++){
        jstring jkey = (jstring) (env->GetObjectArrayElement(jkeys, i));
        std::string key = jstring2string(env,jkey);
//        std::cout<<key<<std::endl;
    }
}
JNI_METHOD(void, JNITEST)(JNIEnv * , jclass) {}