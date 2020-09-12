#ifndef MEDIAPIPE_ACU_JNI_H
#define MEDIAPIPE_ACU_JNI_H


#include <jni.h>
extern "C" {
#define JNI_METHOD(returnType, funcName)\
    JNIEXPORT returnType JNICALL        \
        Java_com_google_mediapipe_apps_acuface_JNIInterface_##funcName

JNI_METHOD(void, JNIUpdateMeridianList)(JNIEnv * , jclass , jint , jobjectArray ) ;
JNI_METHOD(void, JNITEST)(JNIEnv * , jclass) ;


}



#endif //MEDIAPIPE_ACU_JNI_H
