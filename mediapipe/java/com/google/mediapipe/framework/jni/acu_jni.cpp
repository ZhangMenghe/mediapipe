//
// Created by eevee on 9/11/20.
//
//#include "mediapipe/java/com/google/mediapipe/framework/jni/android_asset_util_jni.h"

#include "acu_jni.h"
#include "mediapipe/calculators/merge/acu_generator.h"
#include <set>
namespace{
    inline static JavaVM *g_vm = nullptr;

    inline std::string jstring2string(JNIEnv *env, jstring jStr){
        const char *cstr = env->GetStringUTFChars(jStr, NULL);
        std::string str = std::string(cstr);
        env->ReleaseStringUTFChars(jStr, cstr);
        return str;
    }
};
using namespace mediapipe;
JNI_METHOD(void, JNIUpdateMeridianList)(JNIEnv* env, jclass, jint num, jobjectArray jkeys){
    acuGenerator::instance()->clearTarget();
    std::set<std::string> keys;
    for(int i=0; i<num; i++){
        jstring jkey = (jstring) (env->GetObjectArrayElement(jkeys, i));
        acuGenerator::instance()->addTargetMerdian(jstring2string(env,jkey));
    }
}
JNI_METHOD(void, JNIAddMeridian)(JNIEnv * env, jclass , jstring jkey) {
    acuGenerator::instance()->addTargetMerdian(jstring2string(env,jkey));

}
JNI_METHOD(void, JNIDelMeridian)(JNIEnv * env, jclass , jstring jkey) {
    acuGenerator::instance()->removeTargetMerdian(jstring2string(env,jkey));
}