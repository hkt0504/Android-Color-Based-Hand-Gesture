LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENCV_PATH := G:\Project\HandGestureSDK\project\jni\opencv_sdk

LOCAL_MODULE := libopencv_java
LOCAL_SRC_FILES := $(OPENCV_PATH)/native/libs/armeabi-v7a/libopencv_java.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(OPENCV_PATH)/native/jni/OpenCV.mk

LOCAL_MODULE    := ImageProcessing
LOCAL_SRC_FILES := ImageProcessing.cpp camera_opengles.cpp interface.c common.c handGesture.cpp
LOCAL_LDLIBS    += -lm -llog -landroid -ljnigraphics -lGLESv1_CM -lGLESv2

include $(BUILD_SHARED_LIBRARY)