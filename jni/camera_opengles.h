#ifndef __CAMERA_OPENGLES_H__
#define __CAMERA_OPENGLES_H__

#include <stdlib.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <android/log.h>

extern pthread_mutex_t	g_mutexMedia;
extern char *g_cameraBuff;
extern int g_squareWidth;
extern int g_squareHeight;

#define  COLOR_TI_FormatYUV420PackedSemiPlanarInterlaced	0x7f000001
#define  COLOR_TI_FormatYUV420PackedSemiPlanar				0x7f000100
#define	 COLOR_FormatYUV420SemiPlanar						0x00000015
#define	 COLOR_FormatYUV420Planar							0x00000013
#define	 COLOR_SONYXPERIA_DECODE_FORMAT						0x7fa30c03
#define	 COLOR_SAMSUNG_GNOTE3_DECODE_CODEC					0x7fa30c04
#define	 HTC_DECODE_CODEC2									0x7fa30c00


#define	YUV420SemiPlanar_BGR	0x10001
#define	YUV420SemiPlanar_RGB	0x10002
#define	YUV420SemiPlanar_HTC	0x10003

	void	InitShader();
	void	ExitShader();

	void CreateFrameTexture(int nWidth, int nHeight);
	//void OnFragmentUnifom(FrameBuff* frame, int nWidth, int nHeight);
	//void	OnFragmentDelete();

	GLuint	LoadShader(GLenum type, const char *shaderSrc);
	GLuint	CreateProgram(GLuint vertexShader, GLuint fragmentShader);
	void GetGLColorBuffer(char* pGLBuffer, int nGLScreenWidth, int nGLScreenHeight);
	void renderMaskImage(unsigned char *pImageBuffer, int width, int height);

	//void	OnGLESRender(FrameBuff** pVideoBuff, ImageBuff** pImageBuff);
	int _getGLScreenBuffer(char* pGLBuffer, int nGLScreenWidth, int nGLScreenHeight);
    int GetGLBpp();


    void SetGLScreenSize(int width, int height, int exportWidth, int exportHeight);
    void InitRenderFrame();
    void FiniteRenderFrame();
    void OnRenderFrame(char* pGLBuff);
    void StartRecording(int width, int height, int color_format);
    void CopyScreenBuff(char* pBuff, int size);
    void SetMaskImage(unsigned char *pixelBuffer, int width, int height);
    void EndRecording();


#ifdef __cplusplus
};
#endif
#endif /*__CAMERA_OPENGLES_H__*/
