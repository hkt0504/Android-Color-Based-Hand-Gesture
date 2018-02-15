
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "common.h"
#include "camera_opengles.h"

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return JNI_VERSION_1_4;
}

jobject BitmapLock( JNIEnv* env, jobject thiz, jobject pBitmap, void** pBitmapRefPixelBuffer )
{
	jobject pBitmapRef = (*env)->NewGlobalRef(env, pBitmap); //lock the bitmap preventing the garbage collector from destructing it

	if (pBitmapRef == NULL)
	{
		*pBitmapRefPixelBuffer = NULL;
		return NULL;
	}

	int result = AndroidBitmap_lockPixels(env, pBitmapRef, pBitmapRefPixelBuffer);
	if (result != 0)
	{
		*pBitmapRefPixelBuffer = NULL;
		return NULL;
	}

	return pBitmapRef;
}

void BitmapUnlock( JNIEnv* env, jobject thiz, jobject pBitmapRef, void* pBitmapRefPixelBuffer )
{
	if (pBitmapRef)
	{
		if (pBitmapRefPixelBuffer)
		{
			AndroidBitmap_unlockPixels(env, pBitmapRef);
			pBitmapRefPixelBuffer = NULL;
		}
		(*env)->DeleteGlobalRef(env, pBitmapRef);
		pBitmapRef = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// render
void Java_com_handgesture_sdk_camera_GLRenderer_NativeRenderInit(JNIEnv *env, jobject thiz)
{
	InitRenderFrame();
}

void Java_com_handgesture_sdk_camera_GLRenderer_NativeRenderFinit(JNIEnv * env, jobject thiz)
{
	FiniteRenderFrame();
}

void Java_com_handgesture_sdk_camera_GLRenderer_NativeRenderResize(JNIEnv * env, jobject thiz, jint width, jint height, jint exportWidth, jint exportHeight)
{
	SetGLScreenSize(width, height, exportWidth, exportHeight);
}

void Java_com_handgesture_sdk_camera_GLRenderer_NativeRenderFrame(JNIEnv * env, jobject thiz)
{
	OnRenderFrame(NULL);
}

void Java_com_handgesture_sdk_camera_CameraPreview_nativeGetScreenBuff(JNIEnv *env, jobject thiz, jbyteArray buff, jint size)
{
	jbyte* pBuff = (jbyte*) (*env)->GetPrimitiveArrayCritical(env, buff, 0);

	CopyScreenBuff(pBuff, size);

	(*env)->ReleasePrimitiveArrayCritical(env, buff, pBuff, 0);
}

