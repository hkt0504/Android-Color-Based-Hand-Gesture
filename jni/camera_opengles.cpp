#include "camera_opengles.h"
#include "common.h"

GLuint g_VertexShader, g_ImageVertexShader;
GLuint g_FragmentShader, g_ImageFragmentShader;
GLuint g_Program, g_ImageProgram;
GLuint g_vPositionHandle, g_vTexPos;
GLuint g_vImagePositionHandle, g_vImageTexPos;

GLint g_vCameraLoc, g_vImageLoc;
GLuint g_maskTexture = 0, g_cameraTexture = 0;
int g_maskWidth, g_maskHeight;

GLuint	g_FrameBuff;
GLuint	g_FrameTexture;
bool	g_FrameCreated = false;
pthread_mutex_t	g_mutexMedia = {0};
pthread_mutex_t	g_mutexExport = {0};

char*	g_pExportBuff = NULL;
char*	g_pGLTextureBuff = NULL;

static int g_nTextureWidth, g_nTextureHeight;
int g_squareWidth = 0, g_squareHeight = 0;
static int g_surfaceWidth = 0, g_surfaceHeight = 0;
int g_nExportColorFormat = -1;

static const char g_ImageVertexShaderStr[] =
		"attribute vec4 vPosition;    \n"
		"attribute vec2 a_texCoord;   \n"
		"varying vec2 tc;     \n"
		"void main()                  \n"
		"{                            \n"
		"   gl_Position = vPosition;  \n"
		"   tc = a_texCoord;  \n"
		"}                            \n";

static const char g_ImageFragmentShaderStr[] =
		"precision mediump float;\n"
		"uniform sampler2D effect_tex;                 \n"
		"varying vec2 tc;                         \n"
		"void main()                                  \n"
		"{                                            \n"
		"vec4 color = texture2D(effect_tex, tc); \n"
		"gl_FragColor = vec4(color.r, color.g, color.b, color.a); \n"
		"}                                            \n";

unsigned char *maskImage = NULL;

GLuint LoadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;

	// Create the shader object
	shader = glCreateShader(type);

	if(shader == 0)
		return 0;

	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);

	//Compile the shader
	glCompileShader(shader);

	// Check the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if(!compiled)
	{
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint CreateProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint programObject;
	GLint linked;

	// Create the program object
	programObject = glCreateProgram();

	if(programObject == 0)
		return 0;

	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);

	// Link the program
	glLinkProgram(programObject);

	// Check the link status
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

	if(!linked)
	{
		glDeleteProgram(programObject);
		return 0;
	}
	return programObject;
}

void InitShader()
{
	pthread_mutex_init(&g_mutexMedia, NULL);
	pthread_mutex_init(&g_mutexExport, NULL);

	glClearColor(0, 0, 0, 1);
	g_VertexShader = LoadShader(GL_VERTEX_SHADER, g_ImageVertexShaderStr);
	g_FragmentShader = LoadShader(GL_FRAGMENT_SHADER, g_ImageFragmentShaderStr);
	g_Program = CreateProgram(g_VertexShader, g_FragmentShader);

	g_ImageVertexShader = LoadShader(GL_VERTEX_SHADER, g_ImageVertexShaderStr);
	g_ImageFragmentShader = LoadShader(GL_FRAGMENT_SHADER, g_ImageFragmentShaderStr);
	g_ImageProgram = CreateProgram(g_ImageVertexShader, g_ImageFragmentShader);

	g_vPositionHandle = glGetAttribLocation(g_Program, "vPosition");
	g_vTexPos = glGetAttribLocation(g_Program, "a_texCoord");
	g_vCameraLoc = glGetUniformLocation(g_Program, "effect_tex");

	g_vImagePositionHandle = glGetAttribLocation(g_ImageProgram, "vPosition");
	g_vImageTexPos = glGetAttribLocation(g_ImageProgram, "a_texCoord");
	g_vImageLoc = glGetUniformLocation(g_ImageProgram, "effect_tex");

	__android_log_print(ANDROID_LOG_DEBUG, "test", "init shader");
}

void ExitShader()
{
	int i;

	pthread_mutex_lock(&g_mutexMedia);

	glDeleteProgram(g_Program);
	glDeleteShader(g_VertexShader);
	glDeleteShader(g_FragmentShader);

	glDeleteProgram(g_ImageProgram);
	glDeleteShader(g_ImageVertexShader);
	glDeleteShader(g_ImageFragmentShader);

	if (g_FrameCreated)
	{
		glDeleteTextures(1, &g_FrameTexture);
		glDeleteFramebuffers(1, &g_FrameBuff);
	}

//	if(g_maskTexture != 0)
//		glDeleteTextures(1, &g_maskTexture);
//	g_maskTexture = 0;

	if(g_cameraTexture != 0)
		glDeleteTextures(1, &g_cameraTexture);
	g_cameraTexture = 0;

	pthread_mutex_unlock(&g_mutexMedia);

	pthread_mutex_destroy(&g_mutexExport);
	pthread_mutex_destroy(&g_mutexMedia);

	__android_log_print(ANDROID_LOG_DEBUG, "test", "exit shader");
}

void CreateFrameTexture(int nWidth, int nHeight)
{
	if(g_FrameBuff)
		glDeleteFramebuffers(1, &g_FrameBuff);
	if(g_FrameTexture)
		glDeleteTextures(1, &g_FrameTexture);

	glGenFramebuffers(1, &g_FrameBuff);
	glBindFramebuffer(GL_FRAMEBUFFER, g_FrameBuff);
	glGenTextures(1, &g_FrameTexture);
	glBindTexture(GL_TEXTURE_2D, g_FrameTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_FrameTexture, 0);

	g_FrameCreated = true;
	glBindFramebuffer(GL_FRAMEBUFFER, g_FrameBuff);
}


void SetGLScreenSize(int width, int height, int exportWidth, int exportHeight)
{
	if ((g_nTextureWidth != exportWidth) || (g_nTextureHeight != exportHeight))
	{
		g_nTextureWidth = exportWidth;
		g_nTextureHeight = exportHeight;
		glViewport(0, 0, g_nTextureWidth, g_nTextureHeight);

		CreateFrameTexture(g_nTextureWidth, g_nTextureHeight);
	}

	g_surfaceWidth = width;
	g_surfaceHeight = height;
}

void InitRenderFrame()
{
	g_nTextureWidth = g_nTextureHeight = 0;
	InitShader();
}

void FiniteRenderFrame()
{
	ExitShader();
}

void GetGLColorBuffer(char* pGLBuffer, int nGLScreenWidth, int nGLScreenHeight)
{
	glBindFramebuffer(GL_FRAMEBUFFER, g_FrameBuff);
	glViewport(0, 0, g_nTextureWidth, g_nTextureHeight);

	GLint readType, readFormat;
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readType);
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);
	glReadPixels(0, 0, nGLScreenWidth, nGLScreenHeight, readFormat, readType, pGLBuffer);
	//	glReadPixels(0, 0, nGLScreenWidth, nGLScreenHeight, GL_RGBA, GL_UNSIGNED_BYTE, pGLBuffer);
}

void renderCameraFrame(char *buff, int width, int height)
{
	int i;

	if(!buff)
		return;
	pthread_mutex_lock(&g_mutexMedia);

	glBindFramebuffer(GL_FRAMEBUFFER, g_FrameBuff);
	glViewport(0, 0, g_nTextureWidth, g_nTextureHeight);

	glClearColor(0, 0, 0, 1.0f);
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	float texs[8] = {0.f, 0.f, 0.f, 1.f, 1.f, 0.f, 1.f, 1.f}, vtTri[8] = {-1.f, -1.f, -1.f, 1.f, 1.f, -1.f, 1.f, 1.f}, tmp1, tmp2;

	// Draw Camera Buffer
	glUseProgram(g_Program);
	glVertexAttribPointer(g_vPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vtTri);
	glEnableVertexAttribArray(g_vPositionHandle);
	glVertexAttribPointer(g_vTexPos, 2, GL_FLOAT, GL_FALSE, 0, texs);
	glEnableVertexAttribArray(g_vTexPos);

	if(!g_cameraTexture)
	{
		glGenTextures(1, &g_cameraTexture);
	}
	glBindTexture(GL_TEXTURE_2D, g_cameraTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buff);

	/* draw video frame */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_cameraTexture);

	glUniform1i(g_vCameraLoc, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Draw Images

	//		glUseProgram(g_ImageProgram);
	//		glEnable(GL_BLEND);
	//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//
	//		glVertexAttribPointer(g_vImagePositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vtTri);
	//		glEnableVertexAttribArray(g_vImagePositionHandle);
	//		glVertexAttribPointer(g_vImageTexPos, 2, GL_FLOAT, GL_FALSE, 0, texs);
	//		glEnableVertexAttribArray(g_vImageTexPos);

	//			if (pImageBuff[i] != NULL)
	//			{
	//				texs[0] = texs[2] = (float)pImageBuff[i]->framePosX / (float)pImageBuff[i]->width;
	//				texs[4] = texs[6] = (float)(pImageBuff[i]->framePosX + pImageBuff[i]->drawWidth) / (float)pImageBuff[i]->width;
	//				texs[1] = texs[5] = (float)pImageBuff[i]->framePosY / (float)pImageBuff[i]->height;
	//				texs[3] = texs[7] = (float)(pImageBuff[i]->framePosY + pImageBuff[i]->drawHeight) / (float)pImageBuff[i]->height;
	//
	//				vtTri[0] = vtTri[2] = ((float)pImageBuff[i]->showPosX / (float)g_nTextureWidth) * 2 - 1;
	//				vtTri[4] = vtTri[6] = ((float)(pImageBuff[i]->showPosX + pImageBuff[i]->frameWidth) / (float)g_nTextureWidth) * 2 - 1;
	//				vtTri[1] = vtTri[5] = 1.0f - ((float)pImageBuff[i]->showPosY / (float)g_nTextureHeight) * 2;
	//				vtTri[3] = vtTri[7] = 1.0f - ((float)(pImageBuff[i]->showPosY + pImageBuff[i]->frameWidth * pImageBuff[i]->drawHeight
	//										/ pImageBuff[i]->drawWidth) / (float)g_nTextureHeight) * 2;
	//
	//				if (pImageBuff[i]->pBuff != NULL)
	//				{
	//					if(pImageBuff[i]->texture != 0)
	//						glDeleteTextures(1, &pImageBuff[i]->texture);
	//					glGenTextures(1, &pImageBuff[i]->texture);
	//					glBindTexture(GL_TEXTURE_2D, pImageBuff[i]->texture);
	//					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pImageBuff[i]->width, pImageBuff[i]->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageBuff[i]->pBuff);
	//					free(pImageBuff[i]->pBuff);
	//					pImageBuff[i]->pBuff = NULL;
	//				}
	//
	//				glActiveTexture(GL_TEXTURE0);
	//				glBindTexture(GL_TEXTURE_2D, pImageBuff[i]->texture);
	//				glUniform1i(g_vImageLoc, 0);
	//				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//			}
	//		glDisable(GL_BLEND);
	//
	/* Draw Mask Image */

	if (maskImage)
	{
		if(g_maskTexture == 0)
			glGenTextures(1, &g_maskTexture);

		glBindTexture(GL_TEXTURE_2D, g_maskTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_maskWidth, g_maskHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, maskImage);
		free(maskImage);
		maskImage = NULL;
	}

	if(g_maskTexture > 0)
	{
		float texs2[8] = {0, 1, 0, 0, 1, 1, 1, 0};
		float vtTri2[8] = {-1, -1, -1, 1, 1, -1, 1, 1};

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(g_Program);
		glVertexAttribPointer(g_vPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vtTri2);
		glEnableVertexAttribArray(g_vPositionHandle);
		glVertexAttribPointer(g_vTexPos, 2, GL_FLOAT, GL_FALSE, 0, texs2);
		glEnableVertexAttribArray(g_vTexPos);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, g_maskTexture);

		glUniform1i(g_vCameraLoc, 4);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisable(GL_BLEND);
	}

	/* Output to SurfaceView */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, g_surfaceWidth, g_surfaceHeight);
	glUseProgram(g_Program);
	glVertexAttribPointer(g_vPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vtTri);
	glEnableVertexAttribArray(g_vPositionHandle);
	glVertexAttribPointer(g_vTexPos, 2, GL_FLOAT, GL_FALSE, 0, texs);
	glEnableVertexAttribArray(g_vTexPos);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_FrameTexture);

	glUniform1i(g_vCameraLoc, 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	pthread_mutex_unlock(&g_mutexMedia);
}

void OnRenderFrame(char* pGLBuff)
{
	VLPLog("NativeRenderFrame");

	renderCameraFrame(g_cameraBuff, g_squareWidth, g_squareHeight);
	//	pthread_mutex_lock(&g_mutexMedia);
	//
	//	//OnGLESRender(g_pVideoFrameBuff, g_pImageBuff);
	//
	//	pthread_mutex_unlock(&g_mutexMedia);
	//
	if(!g_pGLTextureBuff)
		return;

	int bpp = GetGLBpp();
	GetGLColorBuffer(g_pGLTextureBuff, g_nTextureWidth, g_nTextureHeight);
	//
	//	VLPLog("NativeRenderFrame, step 6");
	//
	pthread_mutex_lock(&g_mutexExport);

	if (g_pExportBuff)
	{
		VLPLog("NativeRenderFrame, step 7 Copy Export Frame");

		if (bpp == 2) {
			switch (g_nExportColorFormat) {
			case COLOR_FormatYUV420Planar:
				bgr565_to_yuv420(g_pGLTextureBuff, g_pExportBuff, g_nTextureWidth, g_nTextureHeight);
				break;
			case COLOR_FormatYUV420SemiPlanar:
//				if ((g_nExportColorExtension == YUV420SemiPlanar_BGR) || (g_nExportColorExtension == YUV420SemiPlanar_HTC))
//					bgr565_to_yuv420sp(pGLBuff, g_pExportBuff, g_nTextureWidth, g_nTextureHeight);
//				else
					rgb565_to_yuv420sp(g_pGLTextureBuff, g_pExportBuff, g_nTextureWidth, g_nTextureHeight);
				break;
			default:
				rgb565_to_yuv420sp(g_pGLTextureBuff, g_pExportBuff, g_nTextureWidth, g_nTextureHeight);
				break;
			}
		} else {
			if (g_nExportColorFormat == COLOR_FormatYUV420Planar)
				rgb8888_to_yuv420(g_pGLTextureBuff, g_pExportBuff, g_nTextureWidth, g_nTextureHeight);
			else if (g_nExportColorFormat == COLOR_TI_FormatYUV420PackedSemiPlanar)
				rgb8888_to_yuv420sp(g_pGLTextureBuff, g_pExportBuff, g_nTextureWidth, g_nTextureHeight);
			else
				bgr8888_to_yuv420sp(g_pGLTextureBuff, g_pExportBuff, g_nTextureWidth, g_nTextureHeight);
		}
	}

	pthread_mutex_unlock(&g_mutexExport);
	//
	//	VLPLog("NativeRenderFrame, step 6.4, g_nExportColorFormat=%d", g_nExportColorFormat);
	//
	//	g_nExportTic ++;
	//
	//	VLPLog("NativeRenderFrame End");
}

int GetGLBpp()
{
    GLint readType, readFormat;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readType);
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);
    int bpp = 4;
    if(readType == GL_UNSIGNED_SHORT_5_6_5 && readFormat == GL_RGB)
        bpp = 2;
    else if(readType == GL_UNSIGNED_BYTE && readFormat == GL_RGBA)
        bpp = 4;
    return bpp;
}

void StartRecording(int width, int height, int color_format)
{
	if(width != g_nTextureWidth || height != g_nTextureHeight)
		SetGLScreenSize(g_nTextureWidth, g_nTextureHeight, width, height);

	pthread_mutex_lock(&g_mutexExport);

	SAFE_FREE(g_pExportBuff);
	SAFE_FREE(g_pGLTextureBuff);

	int size = (int)((g_nTextureWidth * g_nTextureHeight * 3) >> 1);
	g_pExportBuff = (char*)malloc(size);

	size = g_nTextureWidth * g_nTextureHeight * GetGLBpp();
	g_pGLTextureBuff = (char*)malloc(size);

	g_nExportColorFormat = color_format;
	pthread_mutex_unlock(&g_mutexExport);
}

void EndRecording()
{
	pthread_mutex_lock(&g_mutexExport);
	SAFE_FREE(g_pExportBuff);
	SAFE_FREE(g_pGLTextureBuff);

	pthread_mutex_unlock(&g_mutexExport);

}
void CopyScreenBuff(char* pBuff, int size)
{
	if (g_pExportBuff){

		pthread_mutex_lock(&g_mutexExport);
		memcpy(pBuff, g_pExportBuff, size);
		pthread_mutex_unlock(&g_mutexExport);
	}
}

void SetMaskImage(unsigned char *pixelBuffer, int width, int height)
{
	pthread_mutex_lock(&g_mutexMedia);

	if(!pixelBuffer)
	{
		if(g_maskTexture)
			glDeleteTextures(1, &g_maskTexture);
		g_maskTexture = 0;
		if(maskImage)
			free(maskImage);
		maskImage = NULL;
	}
	else
	{
		if (maskImage == NULL)
		{
			maskImage = (unsigned char*)malloc(width  *height * 4);
		}
		memcpy(maskImage, pixelBuffer, width*height*4);

		if(g_maskTexture != 0)
			glDeleteTextures(1, &g_maskTexture);
		glGenTextures(1, &g_maskTexture);
		g_maskWidth = width;
		g_maskHeight = height;
	}
	pthread_mutex_unlock(&g_mutexMedia);

}
