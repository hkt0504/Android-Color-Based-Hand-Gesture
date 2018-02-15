#include <jni.h>

#include <opencv2/opencv.hpp>
#include "camera_opengles.h"
#include "common.h"
#include "handGesture.hpp"

using namespace std;
using namespace cv;

#define  LOG_TAG  "HandGesture"

extern "C"
{
	char *g_cameraBuff = NULL;

	vector<Rect> coverRect;
	int h_val[7], l_val[7], s_val[7];
	vector<Scalar> lower, upper;
	bool bEndSampling = false;
	int delta = 20;

	void normalizeColors()
	{
		for(int sampleIdx = 0; sampleIdx < 7; sampleIdx++)
		{
			int h_min = h_val[sampleIdx] - delta;
			int h_max = h_val[sampleIdx] + delta;
			int s_min = s_val[sampleIdx] - delta;
			int s_max = s_val[sampleIdx] + delta;
			int l_min = l_val[sampleIdx] - delta;
			int l_max = l_val[sampleIdx] + delta;

			if(h_min < 0) h_min = 0;
			if(h_max > 255) h_max = 255;
			if(s_min < 0) s_min = 0;
			if(s_max > 255) s_max = 255;
			if(l_min < 0) l_min = 0;
			if(l_max > 255) l_max = 255;

			Scalar min = Scalar(h_min, l_min, s_min);
			Scalar max = Scalar(h_max, l_max, s_max);
			upper.push_back(max);
			lower.push_back(min);
		}
	}

	int getMedian(vector<int> val)
	{
		int median;
		size_t size = val.size();
		sort(val.begin(), val.end());
		if (size  % 2 == 0)
		  median = val[size / 2 - 1] ;
		else
		  median = val[size / 2];

		return median;
	}

	void getSamplingReady(Mat &src, int currentFrameNum)
	{
		if(currentFrameNum == 0)
		{
			coverRect.push_back(Rect(Point(src.cols/3, src.rows/6),Point(src.cols/3+20,src.rows/6+20)));
			coverRect.push_back(Rect(Point(src.cols/4, src.rows/2),Point(src.cols/4+20,src.rows/2+20)));
			coverRect.push_back(Rect(Point(src.cols/3, src.rows/1.5),Point(src.cols/3+20,src.rows/1.5+20)));
			coverRect.push_back(Rect(Point(src.cols/2, src.rows/2),Point(src.cols/2+20,src.rows/2+20)));
			coverRect.push_back(Rect(Point(src.cols/2.5, src.rows/2.5),Point(src.cols/2.5+20,src.rows/2.5+20)));
			coverRect.push_back(Rect(Point(src.cols/2, src.rows/1.5),Point(src.cols/2+20,src.rows/1.5+20)));
			coverRect.push_back(Rect(Point(src.cols/2.5, src.rows/1.8),Point(src.cols/2.5+20,src.rows/1.8+20)));
		}

		for(int sampleIdx = 0; sampleIdx < 7; sampleIdx++){
			rectangle(src, coverRect[sampleIdx], Scalar(255, 0, 0), 2);
		}
	}

	void getSamplingImg(Mat &displayImg, Mat src, bool bFirst)
	{
		Mat hlsImg;
		cvtColor(src, hlsImg, CV_BGR2HLS);

		for(int sampleIdx = 0; sampleIdx < 7; sampleIdx++)
		{
			Mat areaMat = Mat(hlsImg, coverRect[sampleIdx]);
			vector<int> hm, sm, lm;

			// generate vectors
			for(int i = 2; i < areaMat.rows-2; i++)
			{
				for(int j = 2; j < areaMat.cols-2; j++){
					hm.push_back(areaMat.data[areaMat.channels()*(areaMat.cols*i + j) + 0]) ;
					lm.push_back(areaMat.data[areaMat.channels()*(areaMat.cols*i + j) + 1]) ;
					sm.push_back(areaMat.data[areaMat.channels()*(areaMat.cols*i + j) + 2]) ;
				}
			}

			if(bFirst)
			{
				h_val[sampleIdx] = getMedian(hm);
				s_val[sampleIdx] = getMedian(sm);
				l_val[sampleIdx] = getMedian(lm);
			}
			else
			{
				h_val[sampleIdx] = (getMedian(hm) + h_val[sampleIdx])/2;
				s_val[sampleIdx] = (getMedian(sm) + s_val[sampleIdx])/2;
				l_val[sampleIdx] = (getMedian(lm) + l_val[sampleIdx])/2;
			}

			rectangle(displayImg, coverRect[sampleIdx], Scalar(0, 0, 255), 2);
		}
	}

	jboolean Java_com_handgesture_sdk_camera_CameraPreview_ImageProcessing( JNIEnv* env, jobject thiz,
				jint width, jint height, jintArray input_data, jintArray touchPoints, jintArray sampleColors,
				jint resize_width, jint resize_height, jboolean bUseFrontCam, jboolean bShowCamera, jint currentFrameNum, jboolean bDebugMode)
	{
		jint * pDrawData = env->GetIntArrayElements(input_data, 0);
		jint * pTouchPoints = env->GetIntArrayElements(touchPoints, 0);
		jint * pSampleColors = env->GetIntArrayElements(sampleColors, 0);

		int handIdx = 0;
		Mat orgImg = Mat(height, width, CV_8UC4, (unsigned char *)pDrawData); // Prepare Mat for target image
		resize(orgImg, orgImg, Size(resize_width, resize_height));
		if(!bUseFrontCam)
			flip(orgImg, orgImg, 0);
		flip(orgImg, orgImg, 0);

		pyrDown(orgImg, orgImg);
		float scale_x = (float)resize_width / (float)orgImg.cols;
		float scale_y = (float)resize_height / (float)orgImg.rows;

		Mat testImg;
		/*if(bSamplingMode)
		{
			if(currentFrameNum < 30)
				getSamplingReady(orgImg);
			else if(currentFrameNum < 50)
			{
				if(currentFrameNum == 30)
					getSamplingImg(orgImg, orgImg.clone(), true);
				else
					getSamplingImg(orgImg, orgImg.clone(), false);

				if(currentFrameNum == 49)
				{
					normalizeColors();
				}
			}
			else
			{
				for(int sampleIdx = 0; sampleIdx < 7; sampleIdx++)
				{
					pSampleColors[sampleIdx*6] = lower[sampleIdx][0];
					pSampleColors[sampleIdx*6+1] = lower[sampleIdx][1];
					pSampleColors[sampleIdx*6+2] = lower[sampleIdx][2];
					pSampleColors[sampleIdx*6+3] = upper[sampleIdx][0];
					pSampleColors[sampleIdx*6+4] = upper[sampleIdx][1];
					pSampleColors[sampleIdx*6+5] = upper[sampleIdx][2];
				}

				bEndSampling = true;
			}
		}
		else
		{
			if(currentFrameNum == 0)
			{
				for(int sampleIdx = 0; sampleIdx < 7; sampleIdx++)
				{
					Scalar upper_value = Scalar(pSampleColors[sampleIdx*6+3], pSampleColors[sampleIdx*6+4], pSampleColors[sampleIdx*6+4]);
					Scalar lower_value = Scalar(pSampleColors[sampleIdx*6], pSampleColors[sampleIdx*6+1], pSampleColors[sampleIdx*6+2]);
					upper.push_back(upper_value);
					lower.push_back(lower_value);
				}
			}

			bEndSampling = true;
		}*/

		bEndSampling = false;
		if(currentFrameNum < 30)
			getSamplingReady(orgImg, currentFrameNum);
		else if(currentFrameNum < 50)
		{
			if(currentFrameNum == 30)
				getSamplingImg(orgImg, orgImg.clone(), true);
			else
				getSamplingImg(orgImg, orgImg.clone(), false);

			if(currentFrameNum == 49)
			{
				normalizeColors();
			}
		}
		else
		{
			for(int sampleIdx = 0; sampleIdx < 7; sampleIdx++)
			{
				pSampleColors[sampleIdx*6] = lower[sampleIdx][0];
				pSampleColors[sampleIdx*6+1] = lower[sampleIdx][1];
				pSampleColors[sampleIdx*6+2] = lower[sampleIdx][2];
				pSampleColors[sampleIdx*6+3] = upper[sampleIdx][0];
				pSampleColors[sampleIdx*6+4] = upper[sampleIdx][1];
				pSampleColors[sampleIdx*6+5] = upper[sampleIdx][2];
			}

			bEndSampling = true;
		}

		if(bEndSampling)
		{
			Mat procImg, bwImg;
			cvtColor(orgImg, procImg, CV_BGR2HLS);

			vector<Mat> bwlist;
			for(int sampleIdx = 0; sampleIdx < 7; sampleIdx++)
			{
				Mat bw;
				inRange(procImg, lower[sampleIdx], upper[sampleIdx], bw);
				bwlist.push_back(bw);
			}

			bwlist[0].copyTo(bwImg);
			for(int sampleIdx = 1; sampleIdx < 7; sampleIdx++)
				bwImg += bwlist[sampleIdx];

			Mat morphKernel = getStructuringElement(MORPH_RECT, Size(5, 5));
			morphologyEx(bwImg, bwImg, MORPH_CLOSE, morphKernel);
			dilate(bwImg, bwImg, Mat(), Point(-1, -1), 2);
			erode(bwImg, bwImg, Mat(), Point(-1, -1), 2);

			testImg = bwImg.clone();
			cvtColor(testImg, testImg, CV_GRAY2BGR);

			HandGesture *hg = new HandGesture();
			hg->src = orgImg;
			findContours(bwImg, hg->contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
			hg->initVectors();

			// find biggest contour
			int indexOfBiggestContour = -1;
			int sizeOfBiggestContour = 0;

			for (int i = 0; i < hg->contours.size(); i++)
			{
				if(hg->contours[i].size() > sizeOfBiggestContour){
					sizeOfBiggestContour = hg->contours[i].size();
					hg->cIdx = i;
				}
			}

			if(hg->contours.size() > 0 && hg->cIdx != -1)
			{
				hg->bRect=boundingRect(Mat(hg->contours[hg->cIdx]));
				convexHull(Mat(hg->contours[hg->cIdx]),hg->hullP[hg->cIdx],false,true);
				convexHull(Mat(hg->contours[hg->cIdx]),hg->hullI[hg->cIdx],false,false);
				approxPolyDP( Mat(hg->hullP[hg->cIdx]), hg->hullP[hg->cIdx], 18, true );
				if(hg->contours[hg->cIdx].size() > 30 ){
					convexityDefects(hg->contours[hg->cIdx],hg->hullI[hg->cIdx],hg->defects[hg->cIdx]);
					hg->eleminateDefects();
				}
				bool isHand=hg->detectIfHand();
				if(isHand)
				{
					hg->getFingerTips();
					hg->drawFingerTips();

					handIdx++;
					pTouchPoints[1] = hg->bRect.x+hg->bRect.width/2;
					pTouchPoints[2] = hg->bRect.y + hg->bRect.width/2;
					circle( orgImg, Point(pTouchPoints[1], pTouchPoints[2]), 10, Scalar(255,255,0), 100 );
					circle( testImg, Point(pTouchPoints[1], pTouchPoints[2]), 10, Scalar(255,255,0), 100 );
					rectangle(testImg, hg->bRect, Scalar(0, 0, 255), 2);
					pTouchPoints[1] = pTouchPoints[1]*scale_x;
					pTouchPoints[2] = pTouchPoints[2]*scale_y;

					vector<Vec4i>::iterator d = hg->defects[hg->cIdx].begin();
					while( d!=hg->defects[hg->cIdx].end() )
					{
						Vec4i& v=(*d);
						int startidx=v[0]; Point ptStart(hg->contours[hg->cIdx][startidx] );
						int endidx=v[1]; Point ptEnd(hg->contours[hg->cIdx][endidx] );
						int faridx=v[2]; Point ptFar(hg->contours[hg->cIdx][faridx] );
						float depth = v[3] / 256;

						pTouchPoints[2*handIdx+1] = ptStart.x;
						pTouchPoints[2*(handIdx+1)] = ptStart.y;
						circle( orgImg, Point(pTouchPoints[2*handIdx+1], pTouchPoints[2*(handIdx+1)]), 4, Scalar(255,0,0), -1);
						circle( testImg, Point(pTouchPoints[2*handIdx+1], pTouchPoints[2*(handIdx+1)]), 4, Scalar(255,0,0), -1);
						pTouchPoints[2*handIdx+1] = pTouchPoints[2*handIdx+1] * scale_x;
						pTouchPoints[2*(handIdx+1)] = pTouchPoints[2*(handIdx+1)] * scale_y;
						handIdx++;

						d++;
					 }
				}
			}

			pTouchPoints[0] = handIdx;

			hg->getFingerNumber();
			delete(hg);
		}

		currentFrameNum++;

		if(bShowCamera || !bEndSampling)
		{
			if(currentFrameNum > 50)
				if(bDebugMode)
					orgImg = testImg.clone();

			flip(orgImg, orgImg, 0);
			pyrUp(orgImg, orgImg);

			//---------------------------------------------------------------------
			// draw gl buff
			int* flip_data = new int[resize_width*resize_height];
			Mat glBuffImg = Mat(resize_height, resize_width, CV_8UC4, (unsigned char*)flip_data);
			cvtColor(orgImg, glBuffImg, CV_RGB2RGBA);

			if(!g_cameraBuff)
			{
				g_cameraBuff = (char*)malloc(resize_width * resize_height * 4);
				memset(g_cameraBuff, 0xFF, resize_width * resize_height * 4);
				g_squareWidth = resize_width;
				g_squareHeight = resize_height;
			}

			pthread_mutex_lock(&g_mutexMedia);
			memcpy(g_cameraBuff, (void*)flip_data, resize_width * resize_height * 4);
			pthread_mutex_unlock(&g_mutexMedia);

			delete(flip_data);
			//---------------------------------------------------------------------
		}

		env->ReleaseIntArrayElements(input_data, pDrawData, 0);
		env->ReleaseIntArrayElements(touchPoints, pTouchPoints, 0);
		env->ReleaseIntArrayElements(sampleColors, pSampleColors, 0);

		return bEndSampling;
	}

	jboolean Java_com_handgesture_sdk_camera_CameraPreview_MakeBitmap(JNIEnv* env, jobject thiz, int width, int height, jbyteArray input_pixels, jintArray out_pixels)
	{
		jbyte * pInputPixels = env->GetByteArrayElements(input_pixels, 0);
		jint * pOutPutPixels = env->GetIntArrayElements(out_pixels, 0);

		Mat origImg = Mat(height + height/2, width, CV_8UC1, (unsigned char *)pInputPixels); // fast, only creates header around rImageData
		Mat orgImg = Mat(height, width, CV_8UC4); // Prepare Mat for target image
		cvtColor(origImg, orgImg, CV_YUV2BGRA_NV21);

		Mat resultImg = Mat(height, width, CV_8UC4, (unsigned char *)pOutPutPixels);

		IplImage srcImg = orgImg;
		IplImage ResultImg = resultImg;
		cvCopy(&srcImg, &ResultImg);

		env->ReleaseByteArrayElements(input_pixels, pInputPixels, 0);
		env->ReleaseIntArrayElements(out_pixels, pOutPutPixels, 0);
		return 0;
	}

	jboolean Java_com_handgesture_sdk_camera_CameraPreview_SetGLBuffer(JNIEnv* env, jobject thiz, int width, int height, jintArray input_pixels)
	{
		jint * pInputPixels = env->GetIntArrayElements(input_pixels, 0);

		Mat orgImg = Mat(height, width, CV_8UC4, (unsigned char*)pInputPixels); // Prepare Mat for target image
		flip(orgImg, orgImg, 0);

		//---------------------------------------------------------------------
		// draw gl buff
		int* flip_data = new int[width*height];
		Mat glBuffImg = Mat(height, width, CV_8UC4, (unsigned char*)flip_data);
		cvtColor(orgImg, glBuffImg, CV_RGB2RGBA);

		if(!g_cameraBuff)
		{
			g_cameraBuff = (char*)malloc(width * height * 4);
			memset(g_cameraBuff, 0xFF, width * height * 4);
			g_squareWidth = width;
			g_squareHeight = height;
		}

		pthread_mutex_lock(&g_mutexMedia);
		memcpy(g_cameraBuff, (void*)flip_data, width * height * 4);
		pthread_mutex_unlock(&g_mutexMedia);

		delete(flip_data);
		//---------------------------------------------------------------------

		env->ReleaseIntArrayElements(input_pixels, pInputPixels, 0);
		return true;
	}

	CascadeClassifier hand_cascade;
	jboolean Java_com_handgesture_sdk_camera_CameraPreview_HandDetection( JNIEnv* env, jobject thiz,
					jint width, jint height, jintArray input_data, jintArray touchPoints, jstring dataPath,
					jint resize_width, jint resize_height, jboolean bUseFrontCam, jboolean bShowCamera)
	{
		jint * pDrawData = env->GetIntArrayElements(input_data, 0);
		jint * pTouchPoints = env->GetIntArrayElements(touchPoints, 0);
		const char* path = env->GetStringUTFChars(dataPath, 0);

		hand_cascade.load(path);
		Mat orgImg = Mat(height, width, CV_8UC4, (unsigned char *)pDrawData); // Prepare Mat for target image
		resize(orgImg, orgImg, Size(resize_width, resize_height));

		if(!bUseFrontCam)
			flip(orgImg, orgImg, 0);
		flip(orgImg, orgImg, 0);

		vector<Rect> hands;
		Mat procImg = orgImg.clone();
		cvtColor( procImg, procImg, CV_BGR2GRAY );
		equalizeHist( procImg, procImg );
		hand_cascade.detectMultiScale( procImg, hands,
		        1.1, 2, 0
		        |CV_HAAR_FIND_BIGGEST_OBJECT
		        //|CV_HAAR_DO_ROUGH_SEARCH
		        |CV_HAAR_SCALE_IMAGE,
		        Size(30, 30) );

		int i = 0;
		for(int i = 0; i < hands.size(); i++)
		{
			Point center = Point(cvRound(hands[i].x + hands[i].width*0.5), cvRound(hands[i].y + hands[i].height*0.5));
			int radius = cvRound((hands[i].width + hands[i].height)*0.25);
			rectangle(orgImg, Point(center.x+radius, center.y+radius), Point(center.x-radius, center.y-radius), Scalar(0, 0, 255), 3, 8, 0);
			pTouchPoints[2*i+1] = center.x;
			pTouchPoints[2*(i+1)] = center.y;
		}

		pTouchPoints[0] = hands.size();

		//---------------------------------------------------------------------
		// draw gl buff
		int* flip_data = new int[resize_width*resize_height];
		Mat glBuffImg = Mat(resize_height, resize_width, CV_8UC4, (unsigned char*)flip_data);
		flip(orgImg, orgImg, 0);
		cvtColor(orgImg, glBuffImg, CV_RGB2BGRA);

		if(!g_cameraBuff)
		{
			g_cameraBuff = (char*)malloc(resize_width * resize_height * 4);
			memset(g_cameraBuff, 0xFF, resize_width * resize_height * 4);
			g_squareWidth = resize_width;
			g_squareHeight = resize_height;
		}

		pthread_mutex_lock(&g_mutexMedia);
		memcpy(g_cameraBuff, (void*)flip_data, resize_width * resize_height * 4);
		pthread_mutex_unlock(&g_mutexMedia);

		delete(flip_data);
		//---------------------------------------------------------------------

		env->ReleaseIntArrayElements(input_data, pDrawData, 0);
		env->ReleaseIntArrayElements(touchPoints, pTouchPoints, 0);
		env->ReleaseStringUTFChars(dataPath, path);

		return true;
	}
}
