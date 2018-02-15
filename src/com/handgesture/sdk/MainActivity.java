package com.handgesture.sdk;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;


public class MainActivity extends Activity {

	String TAG = "MainActivity";

	CheckBox cameraCheck, showCheck;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		initControls();
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
	}
	
	private void initControls() {
		
		File SAMPLE_DIR = new File(Util.SAMPLE_DIR);
		if(!SAMPLE_DIR.exists())
			SAMPLE_DIR.mkdirs();
		
		Button startButton = (Button) findViewById(R.id.start_btn);
		startButton.setOnClickListener( new OnClickListener() {

            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
            	/*if(Util.bExistSamplingData)
            	{
            		File SAMPLE_FILE = new File(Util.SAMPLE_DIR + "data.txt");
            		if(SAMPLE_FILE.exists())
            		{
            			Intent gesture = new Intent(MainActivity.this, GestureActivity.class);
                    	startActivity(gesture);
            		}
            		else
            		{
            			Toast.makeText(getBaseContext(), "no exist data file, please get sampling data.", Toast.LENGTH_SHORT).show();
            		}
            	}
            	else*/
            	{
            		Intent gesture = new Intent(MainActivity.this, GestureActivity.class);
                	startActivity(gesture);
            	}
            	
            }
        });
		
		cameraCheck = ( CheckBox ) findViewById( R.id.camera_check );
		showCheck = ( CheckBox ) findViewById( R.id.show_check );
		
		cameraCheck.setChecked(true);
		Util.bUseFrontCamera = true;
		cameraCheck.setOnCheckedChangeListener(new OnCheckedChangeListener()
		{
		    @Override
		    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		    {
		    	Util.bUseFrontCamera = isChecked;
		    }
		});				
		
		showCheck.setOnCheckedChangeListener(new OnCheckedChangeListener()
		{
		    @Override
		    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
		    {
		    	Util.bShowCameraFrame = isChecked;
		    }
		});
		
		copyAssets();
	}
	
	private void copyAssets() 
	{
	    AssetManager assetManager = getAssets();
	    String[] files = null;
	    try {
	        files = assetManager.list("");
	    } catch (IOException e) {
	        Log.e("tag", "Failed to get asset file list.", e);
	    }
	    if (files != null) for (String filename : files) {
	        InputStream in = null;
	        OutputStream out = null;
	        try {
	          in = assetManager.open(filename);
	          File outFile = new File(Util.SAMPLE_DIR, filename);
	          out = new FileOutputStream(outFile);
	          copyFile(in, out);
	        } catch(IOException e) {
	            Log.e("tag", "Failed to copy asset file: " + filename, e);
	        }     
	        finally {
	            if (in != null) {
	                try {
	                    in.close();
	                } catch (IOException e) {
	                    // NOOP
	                }
	            }
	            if (out != null) {
	                try {
	                    out.close();
	                } catch (IOException e) {
	                    // NOOP
	                }
	            }
	        }  
	    }
	}
	
	private void copyFile(InputStream in, OutputStream out) throws IOException {
	    byte[] buffer = new byte[1024];
	    int read;
	    while((read = in.read(buffer)) != -1){
	      out.write(buffer, 0, read);
	    }
	}
}
