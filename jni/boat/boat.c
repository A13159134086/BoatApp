#include "boat.h"

#include <stdlib.h>

ANativeWindow* boatGetNativeWindow(){	
        return mBoat.window;
}

void* boatGetNativeDisplay(){
	return mBoat.display;
}

void boatGetCurrentEvent(BoatInputEvent* event){
	memcpy(event, &mBoat.current_event, sizeof(BoatInputEvent));
}

void boatSetCurrentEventProcessor(BoatEventProcessor processor){
	mBoat.current_event_processor = processor;
}
void boatSetCursorMode(int mode){
	if (mBoat.android_jvm == 0){
		return;
	}
	JNIEnv* env = 0;
	
	jint result = (*mBoat.android_jvm)->AttachCurrentThread(mBoat.android_jvm, &env, 0);
	
	if (result != JNI_OK || env == 0){
		__android_log_print(ANDROID_LOG_ERROR, "Boat", "Failed to attach thread to JavaVM.");
		abort();
	}
	
	jclass class_BoatInput = mBoat.class_BoatInput;
	
	if (class_BoatInput == 0){
		__android_log_print(ANDROID_LOG_ERROR, "Boat", "Failed to find class: cosine/boat/BoatInput.");
		abort();
	}
	
	jmethodID BoatInput_setCursorMode = (*env)->GetStaticMethodID(env, class_BoatInput, "setCursorMode", "(I)V");
	
	if (BoatInput_setCursorMode == 0){
		__android_log_print(ANDROID_LOG_ERROR, "Boat", "Failed to find static method BoatInput::setCursorMode");
		abort();
	}
	(*env)->CallStaticVoidMethod(env, class_BoatInput, BoatInput_setCursorMode, mode);
	
	
	(*mBoat.android_jvm)->DetachCurrentThread(mBoat.android_jvm);
}


JNIEXPORT void JNICALL Java_cosine_boat_BoatInput_send(JNIEnv* env, jclass clazz, jlong time, jint type, jint p1, jint p2){
	
	mBoat.current_event.time = time;
	mBoat.current_event.type = type;
	
	if (type == ButtonPress || type == ButtonRelease){
		mBoat.current_event.mouse_button = p1;
	}
	else if (type == KeyPress || type == KeyRelease){
		mBoat.current_event.keycode = p1;
		mBoat.current_event.keychar = p2;
	}
	else if (type == MotionNotify){
		mBoat.current_event.x = p1;
		mBoat.current_event.y = p2;
	}

	if (mBoat.current_event_processor != 0){
		mBoat.current_event_processor();
	}
	
}


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	
	mBoat.android_jvm = vm;
	
	JNIEnv* env = 0;
	
	jint result = (*mBoat.android_jvm)->AttachCurrentThread(mBoat.android_jvm, &env, 0);
	
	if (result != JNI_OK || env == 0){
		__android_log_print(ANDROID_LOG_ERROR, "Boat", "Failed to attach thread to JavaVM.");
		abort();
	}
	
	jclass class_BoatInput = (*env)->FindClass(env, "cosine/boat/BoatInput");
	if (class_BoatInput == 0){
		__android_log_print(ANDROID_LOG_ERROR, "Boat", "Failed to find class: cosine/boat/BoatInput.");
		abort();
	}
	mBoat.class_BoatInput = (jclass)(*env)->NewGlobalRef(env, class_BoatInput);
	
	return JNI_VERSION_1_6;
}
