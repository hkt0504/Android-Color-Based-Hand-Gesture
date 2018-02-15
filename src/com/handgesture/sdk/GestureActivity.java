package com.handgesture.sdk;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;
import android.graphics.Point;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.handgesture.sdk.camera.CameraPreview;
import com.handgesture.sdk.camera.VlpGLView;

public class GestureActivity extends Activity {

	String TAG = "GestureActivity";
	private Camera mCamera;
	private Camera.Parameters mParams;
	private CameraPreview mPreview;
	public VlpGLView mGLView;
	private ImageView mTouchView;
	public ImageView mIconView;
	
	public Bitmap moveCursor;	
	public Bitmap currentBitmap = null;
	public String currentPath;
	public int current_idx = 4;
	int current_rotate = 0;
	int current_zoom = 0;

	public static GestureActivity activity = null;
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		setContentView(R.layout.activity_gesture);
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		initControls();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	private void initControls() {
		
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
		Util.screen_height = displayMetrics.heightPixels;
		Util.screen_width = displayMetrics.widthPixels;
		Util.touch_screen_height = Util.screen_height / 4;
		Util.touch_screen_width = Util.screen_width / 4;
		
		currentPath = Util.SAMPLE_DIR + String.valueOf(current_idx) + ".jpg";
		currentBitmap = BitmapFactory.decodeFile(currentPath);
		moveCursor = BitmapFactory.decodeResource(getResources(),R.drawable.move_cursor);		
		
		// Preview
		mPreview = (CameraPreview)findViewById(R.id.surfaceview);
		mPreview.setKeepScreenOn(true);
		mPreview.setParent(this);
		
		mTouchView = (ImageView)findViewById(R.id.image_view);
		mTouchView.setImageBitmap(currentBitmap);
		
		mIconView = (ImageView)findViewById(R.id.icon_view);
        
        RelativeLayout.LayoutParams lp = new RelativeLayout.LayoutParams(Util.screen_width/4, Util.screen_height/4);
		RelativeLayout layout = (RelativeLayout)findViewById(R.id.main_layout);
		
		// GLView
		mGLView = new VlpGLView(this);
		mGLView.setZOrderOnTop(true);		
		layout.addView(mGLView, lp);
		if(!Util.bShowCameraFrame)
			mGLView.setVisibility(View.GONE);
        
        //if(Util.bExistSamplingData)
		//	readSamplingData();
	}

	private void initOrientation(){

		// TODO Auto-generated method stub
		mParams = mCamera.getParameters();
		int degree = 90;
		mCamera.setDisplayOrientation(degree);
		mParams.setRecordingHint(true);

		try {
			mCamera.setParameters(mParams);
		} catch (Exception e) {
			Log.e(TAG, "err", e);
		}
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		
		try {
			if(Util.bUseFrontCamera)
				mCamera = Camera.open(Camera.CameraInfo.CAMERA_FACING_FRONT);
			else
				mCamera = Camera.open(Camera.CameraInfo.CAMERA_FACING_BACK);
			
			mPreview.setCamera(mCamera);
			mCamera.startPreview();

		} catch (RuntimeException ex) {
			Toast.makeText(this, "Show camera error", Toast.LENGTH_LONG).show();
		} catch (Exception e) {
			Log.e(TAG, "err", e);
		}
		initOrientation();
	}

	@Override
	protected void onPause() {
		if (mCamera != null) {
			mCamera.stopPreview();
			mPreview.setCamera(null);
			mCamera.setPreviewCallback(null);
			mCamera.release();
			mCamera = null;
		}

		super.onPause();
	}

	public void drawFrame()
	{
		mGLView.glDraw();
	}
	
	public void readSamplingData()
	{
		File readsampledata = new File(Util.SAMPLE_DIR + "data.txt");
	    
	    try {
	        BufferedReader br = new BufferedReader(new FileReader(readsampledata));
	        String line;

	        int idx = 0;
	        while ((line = br.readLine()) != null) {
	            Util.sampleColors[idx] = Integer.parseInt(line);
	            idx++;
	        }
	        
	        br.close();
	    }
	    catch (IOException e) {
	    	Toast.makeText(getBaseContext(), "Cannot Read Sampling Data.", Toast.LENGTH_SHORT).show();
	    }
	}
	
	public void writeSamplingData()
	{
		File sampledata = new File(Util.SAMPLE_DIR + "data.txt");
		try {
			sampledata.createNewFile();
			FileOutputStream outPutStream = new FileOutputStream(Util.SAMPLE_DIR + "data.txt");
            OutputStreamWriter outputStreamWriter = new OutputStreamWriter(outPutStream);

            for (int i = 0 ; i < 42; i++)
			{
				String data = String.valueOf(Util.sampleColors[i]);
		        outputStreamWriter.write(data);
		        outputStreamWriter.write("\n");
			}
            
			outputStreamWriter.close();
		} catch (IOException e) {
			Toast.makeText(getBaseContext(), "Cannot Write Sampling Data.", Toast.LENGTH_SHORT).show();
		}
	}
	
	public void confirm_gesture(Point[] handPositions, int[] fingerCnts, int during, int width, int height)
	{
		if(during < 5)
			return;
		
		int[] ptParts = new int[100];
		int partCnt = 0;
		for(int i = 0; i < during; i++)
		{
			ptParts[partCnt] = partIdx(handPositions[i], width, height);
			partCnt++;
		}
		
		int currentGestureID = Util.GESTURE_NONE;
		if(partCnt > 5)
		{
			/*
			if(ptParts[0] == 0 || ptParts[0] == 3 || ptParts[0] == 6)
			{
				if(ptParts[partCnt-1] == 2 || ptParts[partCnt-1] == 5 || ptParts[partCnt-1] == 8)
				{
					currentGestureID = Util.GESTURE_MOVERIGHT;
					action_gesture(currentGestureID);
					return;
				}
			}
			
			if(ptParts[0] == 2 || ptParts[0] == 5 || ptParts[0] == 8)
			{
				if(ptParts[partCnt-1] == 0 || ptParts[partCnt-1] == 2 || ptParts[partCnt-1] == 5)
				{
					currentGestureID = Util.GESTURE_MOVELEFT;
					action_gesture(currentGestureID);
					return;
				}
			}
			
			if(ptParts[0] == 0 || ptParts[0] == 1 || ptParts[0] == 2)
			{
				if(ptParts[partCnt-1] == 6 || ptParts[partCnt-1] == 7 || ptParts[partCnt-1] == 8)
				{
					currentGestureID = Util.GESTURE_ZOOMIN;
					action_gesture(currentGestureID);
					return;
				}
			}
			
			if(ptParts[0] == 6 || ptParts[0] == 7 || ptParts[0] == 8)
			{
				if(ptParts[partCnt-1] == 0 || ptParts[partCnt-1] == 1 || ptParts[partCnt-1] == 2)
				{
					currentGestureID = Util.GESTURE_ZOOMOUT;
					action_gesture(currentGestureID);
					return;
				}
			}
			
			if(ptParts[0] == 6 || ptParts[0] == 7 || ptParts[0] == 8)
			{
				if(ptParts[partCnt-1] == 6 || ptParts[partCnt-1] == 7 || ptParts[partCnt-1] == 8)
				{
					boolean bLeft = false;
					boolean bRight = false;
					for(int idx = 0; idx < partCnt-1; idx++)
					{
						if(!bRight)
						{
							if(!bLeft)
							{
								if(ptParts[idx] == 5)
									bLeft = true;
							}
							else
							{
								if(ptParts[idx] == 3)
								{
									currentGestureID = Util.GESTURE_ROTATEINPOSITIVE;
									action_gesture(currentGestureID);
									return;
								}
							}
						}
						
						if(!bLeft)
						{
							if(!bRight)
							{
								if(ptParts[idx] == 3)
									bRight = true;
							}
							else
							{
								if(ptParts[idx] == 5)
								{
									currentGestureID = Util.GESTURE_ROTATEPOSITIVE;
									action_gesture(currentGestureID);
									return;
								}
							}
						}
					}
				}
			}*/
			
			if(ptParts[0] == 2)
			{
				if(ptParts[partCnt-1] == 3)
				{
					boolean bReady = false;
					for(int idx = 1; idx < partCnt-2; idx++)
					{
						if(!bReady)
						{
							if(ptParts[idx] == 0)
								bReady = true;
						}
						else
						{
							if(ptParts[idx] == 1)
							{
								currentGestureID = Util.GESTURE_ROTATEPOSITIVE;
								action_gesture(currentGestureID);
								return;
							}
						}
					}
				}
			}
			
			if(ptParts[0] == 3)
			{
				if(ptParts[partCnt-1] == 2)
				{
					boolean bReady = false;
					for(int idx = 1; idx < partCnt-2; idx++)
					{
						if(!bReady)
						{
							if(ptParts[idx] == 1)
								bReady = true;
						}
						else
						{
							if(ptParts[idx] == 0)
							{
								currentGestureID = Util.GESTURE_ROTATEINPOSITIVE;
								action_gesture(currentGestureID);
								return;
							}
						}
					}
				}
			}
			
			if(ptParts[0] == 0 || ptParts[0] == 2)
			{
				if(ptParts[partCnt-1] == 1 || ptParts[partCnt-1] == 3)
				{
					if(checkDistance(handPositions[0], handPositions[partCnt-1], height))
					{
						currentGestureID = Util.GESTURE_MOVERIGHT;
						action_gesture(currentGestureID);
						return;
					}
				}
			}
			
			if(ptParts[0] == 1 || ptParts[0] == 3)
			{
				if(ptParts[partCnt-1] == 0 || ptParts[partCnt-1] == 2)
				{
					if(checkDistance(handPositions[0], handPositions[partCnt-1], height))
					{
						currentGestureID = Util.GESTURE_MOVELEFT;
						action_gesture(currentGestureID);
						return;
					}					
				}
			}
			
			if(ptParts[0] == 0 || ptParts[0] == 1)
			{
				if(ptParts[partCnt-1] == 2 || ptParts[partCnt-1] == 3)
				{
					if(!checkDistance(handPositions[0], handPositions[partCnt-1], height))
					{
						currentGestureID = Util.GESTURE_ZOOMIN;
						action_gesture(currentGestureID);
						return;
					}					
				}
			}
			
			if(ptParts[0] == 2 || ptParts[0] == 3)
			{
				if(ptParts[partCnt-1] == 0 || ptParts[partCnt-1] == 1)
				{
					if(!checkDistance(handPositions[0], handPositions[partCnt-1], height))
					{
						currentGestureID = Util.GESTURE_ZOOMOUT;
						action_gesture(currentGestureID);
						return;
					}					
				}
			}
		}
	}
	
	private int partIdx(Point pt, int width, int height)
	{
		int partIdx = 0;
		int partWidth = width / 2;
		int partHeight = height / 2;
		
		/*for(int i = 0; i < 3; i++)
		{
			for(int j = 0; j < 3; j++)
			{
				if(pt.x >= partWidth*i && pt.x < partWidth*(i+1))
				{
					if(pt.y >= partHeight*j && pt.y < partHeight*(j+1))
					{
						partIdx = 3 * j + i;
						return partIdx;
					}
				}
			}
		}*/
		
		for(int i = 0; i < 2; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				if(pt.x >= partWidth*i && pt.x < partWidth*(i+1))
				{
					if(pt.y >= partHeight*j && pt.y < partHeight*(j+1))
					{
						partIdx = 2 * j + i;
						return partIdx;
					}
				}
			}
		}
		
		return partIdx;
	}
	
	boolean checkDistance(Point pt1, Point pt2, int height)
	{
		int dist_y = Math.abs(pt1.y - pt2.y);
		
		if(dist_y < height/4)
			return true;
		else 
			return false;
	}
	
	private void action_gesture(int gestureID)
	{
		if(gestureID == Util.GESTURE_NONE)
			return;
		
		switch(gestureID){
		case Util.GESTURE_MOVELEFT:
			current_idx++;			
			break;
		case Util.GESTURE_MOVERIGHT:
			current_idx--;
			break;
		case Util.GESTURE_ZOOMIN:
			current_zoom--;
			break;
		case Util.GESTURE_ZOOMOUT:
			current_zoom++;
			break;
		case Util.GESTURE_ROTATEPOSITIVE:
			current_rotate++;
			break;
		case Util.GESTURE_ROTATEINPOSITIVE:
			current_rotate--;
			break;
		}
		
		if(current_idx < Util.MOVE_MIN)
		{
			current_idx = Util.MOVE_MIN;
			Toast.makeText(getBaseContext(), "This file is first!", Toast.LENGTH_SHORT).show();
			return;
		}
		
		if(current_idx > Util.MOVE_MAX)
		{
			current_idx = Util.MOVE_MAX;
			Toast.makeText(getBaseContext(), "This file is End!", Toast.LENGTH_SHORT).show();
			return;
		}
		
		if(current_idx < 0)
			return;
		
		if(gestureID == Util.GESTURE_MOVELEFT || gestureID == Util.GESTURE_MOVERIGHT)
		{
			current_rotate = 0;
			current_zoom = 0;
			currentPath = Util.SAMPLE_DIR + String.valueOf(current_idx) + ".jpg";
			currentBitmap = BitmapFactory.decodeFile(currentPath);
		}
		
		if(current_zoom > Util.ZOOM_MAX)
		{
			current_zoom = Util.ZOOM_MAX;
			Toast.makeText(getBaseContext(), "This is max!", Toast.LENGTH_SHORT).show();
			return;
		}
		
		if(current_zoom < Util.ZOOM_MIN)
		{
			current_zoom = Util.ZOOM_MIN;
			Toast.makeText(getBaseContext(), "This is min!", Toast.LENGTH_SHORT).show();
			return;
		}
		
		if(current_rotate > Util.ROTATE_MAX)
			current_rotate = 0;
		
		if(current_rotate < Util.ROTATE_MIN)
			current_rotate = 0;
		
		currentPath = Util.SAMPLE_DIR + String.valueOf(current_idx) + ".jpg";
		currentBitmap = BitmapFactory.decodeFile(currentPath);
		
		float rotate_angle = 90 * current_rotate;		
		if(rotate_angle != 0)
		{
			Matrix matrix = new Matrix();
			matrix.postRotate(rotate_angle);
			currentBitmap = Bitmap.createBitmap(currentBitmap , 0, 0, currentBitmap.getWidth(), currentBitmap.getHeight(), matrix, true);
		}
		
		if(gestureID == Util.GESTURE_ZOOMIN || gestureID == Util.GESTURE_ZOOMOUT)
		{
			int width = currentBitmap.getWidth();
			int height = currentBitmap.getHeight();
			if(current_zoom > 0)
			{
				int cutPosX = width * current_zoom / 10;
				int cutPosY = height * current_zoom / 10;
				currentBitmap = Bitmap.createBitmap(currentBitmap, cutPosX, cutPosY, width-cutPosX*2, height-cutPosY*2);
			}
		}
		
		mTouchView.setImageBitmap(currentBitmap);
	}
	
	static 
	{
		System.loadLibrary("opencv_java");
		System.loadLibrary("ImageProcessing");
	}
}
