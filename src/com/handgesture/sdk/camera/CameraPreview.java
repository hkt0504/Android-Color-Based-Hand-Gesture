package com.handgesture.sdk.camera;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Point;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.os.Handler;
import android.os.Looper;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.handgesture.sdk.GestureActivity;
import com.handgesture.sdk.Util;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback, PreviewCallback {

	private static final String TAG = "CameraPreview";
	
	private SurfaceHolder mHolder;
	private Camera mCamera;
	private GestureActivity parent;
	
	private int[] RotateData = null;
	private static byte[] FrameData = null;
	private static Bitmap bitmap_preview = null, bitmap_rotate = null;
	public int previewsize_width, previewsize_height;
	private boolean isProcessing = false;
	
	private int[] touchPoints = null;
	private int clickFrameCnt = 0;
	private int currentFrameNum = 0;
	
	private static final int MIN_PREVIEW_PIXELS = 800 * 600; // normal screen
	private static final int MAX_PREVIEW_PIXELS = 1280 * 720/*1920 * 1080*/; // more than large/HD screen
	
	private Point current_pt = new Point(0, 0);
	
	private boolean isGestureStarted = false;
	private boolean isGestureEnded = false;
	private Point[] handPositions = null;
	private int[] touchFingers = null;
	private int nonRecogFrameCnt = 0;
	private int recogFrameCnt = 0;

	Handler mHandler = new Handler(Looper.getMainLooper());
	Handler mHandler1 = new Handler();

	public CameraPreview(Context context) {
		super(context);
		init(context);
	}

	public CameraPreview(Context context, AttributeSet attrs) {
		super(context, attrs);
		init(context);
	}

	public CameraPreview(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		init(context);
	}

	private void init(Context context) {
		mHolder = getHolder();
		mHolder.addCallback(this);	
	}
	
	private void setPreviewSize(){		
		Camera.Parameters parameters = mCamera.getParameters();

		Point screenResolution = new Point(Util.screen_width, Util.screen_height);
		Point cameraResolution = findBestPreviewSizeValue(parameters, screenResolution);
		parameters.setPreviewSize(cameraResolution.x, cameraResolution.y);
		mCamera.setParameters(parameters);
		
		previewsize_width = cameraResolution.x;
		previewsize_height = cameraResolution.y;
		RotateData = new int[previewsize_width * previewsize_height];
		bitmap_preview = Bitmap.createBitmap(previewsize_width, previewsize_height, Bitmap.Config.ARGB_8888);
		
		handPositions = new Point[100];
		touchFingers = new int[100];
	}

	public void setCamera(Camera camera) {
		mCamera = camera;
		requestLayout();
		if(mCamera != null)
			setPreviewSize();		
	}

	public void setParent(GestureActivity activity){
		this.parent = activity;
	}
	
	private Point findBestPreviewSizeValue(Camera.Parameters parameters, Point screenResolution) {

	    // Sort by size, descending
	    List<Camera.Size> supportedPreviewSizes = new ArrayList<Camera.Size>(parameters.getSupportedPreviewSizes());
	    Collections.sort(supportedPreviewSizes, new Comparator<Camera.Size>() {
	      @Override
	      public int compare(Camera.Size a, Camera.Size b) {
	        int aPixels = a.height * a.width;
	        int bPixels = b.height * b.width;
	        if (bPixels < aPixels) {
	          return -1;
	        }
	        if (bPixels > aPixels) {
	          return 1;
	        }
	        return 0;
	      }
	    });

	    if (Log.isLoggable(TAG, Log.INFO)) {
	      StringBuilder previewSizesString = new StringBuilder();
	      for (Camera.Size supportedPreviewSize : supportedPreviewSizes) {
	        previewSizesString.append(supportedPreviewSize.width).append('x')
	        .append(supportedPreviewSize.height).append(' ');
	      }
	      Log.i(TAG, "Supported preview sizes: " + previewSizesString);
	    }

	    Point bestSize = null;
	    float screenAspectRatio = (float) screenResolution.x / (float) screenResolution.y;

	    float diff = Float.POSITIVE_INFINITY;
	    for (Camera.Size supportedPreviewSize : supportedPreviewSizes) {
	      int realWidth = supportedPreviewSize.width;
	      int realHeight = supportedPreviewSize.height;
	      int pixels = realWidth * realHeight;
	      if (pixels < MIN_PREVIEW_PIXELS || pixels > MAX_PREVIEW_PIXELS) {
	        continue;
	      }
	      boolean isCandidatePortrait = realWidth < realHeight;
	      int maybeFlippedWidth = isCandidatePortrait ? realHeight : realWidth;
	      int maybeFlippedHeight = isCandidatePortrait ? realWidth : realHeight;
	      if (maybeFlippedWidth == screenResolution.x && maybeFlippedHeight == screenResolution.y) {
	        Point exactPoint = new Point(realWidth, realHeight);
	        Log.i(TAG, "Found preview size exactly matching screen size: " + exactPoint);
	        return exactPoint;
	      }
	      float aspectRatio = (float) maybeFlippedWidth / (float) maybeFlippedHeight;
	      float newDiff = Math.abs(aspectRatio - screenAspectRatio);
	      if (newDiff < diff) {
	        bestSize = new Point(realWidth, realHeight);
	        diff = newDiff;
	      }
	    }

	    if (bestSize == null) {
	      Camera.Size defaultSize = parameters.getPreviewSize();
	      bestSize = new Point(defaultSize.width, defaultSize.height);
	      Log.i(TAG, "No suitable preview sizes, using default: " + bestSize);
	    }

	    Log.i(TAG, "Found best approximate preview size: " + bestSize);
	    return bestSize;
	  }
	
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// The Surface has been created, now tell the camera where to draw the preview.
		try {
			mCamera.setPreviewDisplay(holder);			
			mCamera.startPreview();
			
		} catch (IOException e) {
			Log.e(TAG, "err", e);
		} catch (Exception e) {
			Log.e(TAG, "err", e);
		}
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// empty. Take care of releasing the Camera preview in your activity.
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
		// If your preview can change or rotate, take care of those events here.
		// Make sure to stop the preview before resizing or reformatting it.

		if (mHolder.getSurface() == null){
			// preview surface does not exist
			return;
		}

		// stop preview before making changes
		try {
			mCamera.stopPreview();
		} catch (Exception e){
			// ignore: tried to stop a non-existent preview
		}

		// set preview size and make any resize, rotate or
		// reformatting changes here

		// start preview with new settings
		try {
			currentFrameNum = 0;
			mCamera.setPreviewDisplay(mHolder);
			mCamera.setPreviewCallback(this);
			mCamera.startPreview();					
		} catch (Exception e){
			Log.e(TAG, "err", e);
		}
	}

	@Override
	public void onPreviewFrame(byte[] data, Camera camera) {

		if(!isProcessing)
		{
			FrameData = data;		
			mHandler.post(DoImageProcessing);
		}
	}
	
	private Runnable DoImageProcessing = new Runnable() {
		
		@Override
		public void run() {

			isProcessing = true;
			
			try {	
				MakeBitmap(previewsize_width, previewsize_height, FrameData, RotateData);
				bitmap_preview.setPixels(RotateData, 0, previewsize_width, 0, 0, previewsize_width, previewsize_height);		
				
				Matrix matrix = new Matrix();
				matrix.postRotate(90);
				bitmap_rotate = Bitmap.createBitmap(bitmap_preview , 0, 0, previewsize_width, previewsize_height, matrix, true);
				bitmap_rotate.getPixels(RotateData, 0, bitmap_rotate.getWidth(), 0, 0, bitmap_rotate.getWidth(), bitmap_rotate.getHeight());
				
				touchPoints = new int[50];
				//boolean bFinishSampling = ImageProcessing(bitmap_rotate.getWidth(), bitmap_rotate.getHeight(), 
				//		RotateData, touchPoints, Util.sampleColors, Util.touch_screen_width, Util.touch_screen_height, 
				//		Util.bUseFrontCamera, Util.bShowCameraFrame, currentFrameNum, Util.bDebugMode);
				 
				boolean bFinishSampling = HandDetection(bitmap_rotate.getWidth(), bitmap_rotate.getHeight(),
						RotateData, touchPoints, Util.dataPath, Util.touch_screen_width, Util.touch_screen_height, 
						Util.bUseFrontCamera, Util.bShowCameraFrame);
				if(bFinishSampling)
				{					
					int finger_count = touchPoints[0];	
					if(finger_count > 0)
					{	
						isGestureStarted = true;
						handPositions[recogFrameCnt] = new Point(touchPoints[1], touchPoints[2]);
						touchFingers[recogFrameCnt] = touchPoints[0];
						recogFrameCnt++;
						nonRecogFrameCnt = 0;
						
						if(!Util.bShowCameraFrame)
						{
							Bitmap background = Bitmap.createBitmap(Util.touch_screen_width, Util.touch_screen_height, Bitmap.Config.ARGB_8888);
							Canvas canvas = new Canvas(background);
							canvas.drawBitmap(parent.moveCursor, touchPoints[1], touchPoints[2], null);	
							parent.mIconView.setImageBitmap(background);
						}
					}
					else
					{
						if(isGestureStarted)
						{
							nonRecogFrameCnt++;
							if(nonRecogFrameCnt > 5)
								isGestureEnded = true;
						}
						
						if(!Util.bShowCameraFrame)
						{
							Bitmap background = Bitmap.createBitmap(Util.touch_screen_width, Util.touch_screen_height, Bitmap.Config.ARGB_8888);
							parent.mIconView.setImageBitmap(background);
						}
					}
					
					if(isGestureEnded)
					{
						parent.confirm_gesture(handPositions, touchFingers, recogFrameCnt, Util.touch_screen_width, Util.touch_screen_height);
						isGestureStarted = false;
						isGestureEnded = false;
						recogFrameCnt = 0;
					}
				}			

				parent.drawFrame();
			
				currentFrameNum++;
			
			} catch (Exception e) {
				bitmap_preview = null;		
				bitmap_rotate = null;
				isProcessing = false;
			}
			
			isProcessing = false;
		}
	};
	
	public native static boolean ImageProcessing(int width, int height, int[] input_pixels, 
			int[] touchPoints, int[] sampleColors, int resize_width, int resize_height, 
			boolean bUseFrontCam, boolean bShowFrame, int currentFrameNum, boolean bDebugMode);
	
	public native static boolean SetGLBuffer(int width, int height, int[] input_pixels);
	
	public native static boolean MakeBitmap(int width, int height, byte[] input_pixels, int[] out_pixels);
	
	public native static boolean HandDetection( int width, int height, int[] input_pixels, 
			int[] touchPoints, String dataPath, int resize_width, int resize_height, 
			boolean bUseFrontCam, boolean bShowCamera);
}
