#ifndef __COMMON_H__
#define __COMMON_H__
#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <stdio.h>

// ----------------------- LOG PRINT -----------------------//
#define VLPLog(...)				__android_log_print(ANDROID_LOG_DEBUG, "VLPJNI", __VA_ARGS__)
#define VLPErr(...)				__android_log_print(ANDROID_LOG_ERROR, "VLPJNI", __VA_ARGS__)
#define VLPDbg(...)				__android_log_print(ANDROID_LOG_DEBUG, "VLPJNI", __VA_ARGS__)

#define SAFE_FREE(v)	if(v) { free(v);v=NULL;}

#ifdef __cplusplus
extern "C" {
#endif

int bgr565_to_yuv420sp(char* a_pSrcBuffRGB, char* a_pDstBuffYUV, int width, int height);
int rgb565_to_yuv420sp(char* a_pSrcBuffRGB, char* a_pDstBuffYUV, int width, int height);
int bgr8888_to_yuv420sp(char* a_pSrcBuffRGB, char* a_pDstBuffYUV, int width, int height);
int rgb8888_to_yuv420sp(char* a_pSrcBuffRGB, char* a_pDstBuffYUV, int width, int height);
int rgb565_to_yuv420(char* a_pSrcBuffRGB, char* a_pDstBuffYUV, int width, int height);
int bgr565_to_yuv420(char* a_pSrcBuffRGB, char* a_pDstBuffYUV, int width, int height);
int rgb8888_to_yuv420(char* a_pSrcBuffRGB, char* a_pDstBuffYUV, int width, int height);
void yuv420interlaced_to_yuv420(char * yuv420sp, int srcWidth, int srcHeight, char * yuv420);
void yuv420_to_yuv420sp(char* yuv420, int srcWidth, int srcHeight, char* tmp);
int rgb565_to_rgb8888(char* src, char* dest, int width, int height);
int bgr565_to_rgb8888(char* src, char* dest, int width, int height);
int bgr8888_to_rgb8888(char* src, char* dest, int width, int height);

#ifdef __cplusplus
};
#endif

#endif /* __COMMON_H__ */
