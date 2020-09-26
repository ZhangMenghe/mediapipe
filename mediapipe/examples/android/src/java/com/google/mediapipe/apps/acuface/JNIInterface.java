package com.google.mediapipe.apps.acuface;

public final class JNIInterface {
    private static final String TAG = "JNIInterface";
    public static native void JNIUpdateMeridianList(int num, String[] select_names);
    public static native void JNIAddMeridian(String key);
    public static native void JNIDelMeridian(String key);
}
