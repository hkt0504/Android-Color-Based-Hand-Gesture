package com.handgesture.sdk.camera;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.opengl.GLSurfaceView;

public class GLRenderer implements GLSurfaceView.Renderer {

	public GLRenderer() {

	}

	public void onDrawFrame(GL10 gl) {
		// TODO Auto-generated method stub
		NativeRenderFrame();
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		// TODO Auto-generated method stub
		//NativeRenderResize(width, height, Util.VIDEO_SIZE_WIDTH, Util.VIDEO_SIZE_HEIGHT);
		NativeRenderResize(width, height, width/2, height/2);
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		// TODO Auto-generated method stub
		NativeRenderInit();
	}
	
	public void OnDestroy()
	{
		NativeRenderFinit();
	}
	
	public native void NativeRenderInit();
	public native void NativeRenderFinit();
	public native void NativeRenderResize(int width, int height, int exportWidth, int exportHeight);
	public native void NativeRenderFrame();

}
