package com.handgesture.sdk;

import android.os.Environment;

public class Util {
	
	public static boolean bUseFrontCamera = true;
	public static boolean bShowCameraFrame = false;
	//public static boolean bExistSamplingData = true;
	public static boolean bDebugMode = false;
	
	public static int[] sampleColors = new int[42];	
	public static int touch_screen_width, touch_screen_height, screen_width, screen_height;
	public static String SAMPLE_DIR = Environment.getExternalStorageDirectory() + "/HandGesture/";
	public static String dataPath = Util.SAMPLE_DIR + "hand.xml";
	
	public static final int GESTURE_NONE = 0;
	public static final int GESTURE_MOVELEFT = 1;
	public static final int GESTURE_MOVERIGHT = 2;
	public static final int GESTURE_ZOOMIN = 3;
	public static final int GESTURE_ZOOMOUT = 4;
	public static final int GESTURE_ROTATEPOSITIVE = 5;
	public static final int GESTURE_ROTATEINPOSITIVE = 6;
	public static final int GESTURE_CLICK = 7;
	
	public static final int MOVE_MIN = 0;
	public static final int MOVE_MAX = 9;
	public static final int ZOOM_MIN = -3;
	public static final int ZOOM_MAX = 3;
	public static final int ROTATE_MIN = -3;
	public static final int ROTATE_MAX = 3;
}
