//#define ENABLE_PIXEL_FORMAT_Y162CV_16UC1_CPP
#ifdef ENABLE_PIXEL_FORMAT_Y162CV_16UC1_CPP
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include "pxcsensemanager.h"
#include "util_render.h"

#include <string>
#include <iostream>

//可以多个同时声明
//#define DEBUG
#define ENABLE_UTILRENDER_SHOW
#define ENABLE_CV_SHOW


bool PIXEL_FORMAT_Y162CV_16UC1(PXCImage *pxcImage, cv::Mat &image)
{
	//获取图像的格式
	PXCImage::PixelFormat format = pxcImage->QueryInfo().format;
	if(format != PXCImage::PIXEL_FORMAT_Y16){
#ifdef DEBUG
		std::cout << "[-] error:IR图的格式不是 PXCImage::PIXEL_FORMAT_Y16" << std::endl;
#endif
		return false;
	}
	//获取图片宽和高
	int width = pxcImage->QueryInfo().width;
	int height = pxcImage->QueryInfo().height;
	//获取所有像素值
	PXCImage::ImageData data;
	if(pxcImage->AcquireAccess(PXCImage::ACCESS_READ, format, &data) < PXC_STATUS_NO_ERROR){
#ifdef DEBUG
		std::cout << "[-] error:未正常获取IR图" << std::endl;
#endif
		pxcImage->ReleaseAccess(&data);
		return false;
	}
	//转换为OpenCV格式的图像
	image = cv::Mat(cv::Size(width, height), CV_16UC1, data.planes[0]);
	
	pxcImage->ReleaseAccess(&data);
#ifdef DEBUG
	std::cout << "[+] succe:PIXEL_FORMAT_Y16 convert to CV_16UC1 successful." << std::endl;
#endif
	return true;
}

#ifdef ENABLE_CV_SHOW
void showCVImage(const std::string windowName, const cv::Mat &image){
	cv::namedWindow(windowName);
	cv::imshow(windowName, image);
}
#endif

void printFormat(PXCImage *pxcImage, const std::string name){
	std::wstring wstr = PXCImage::PixelFormatToString(pxcImage->QueryInfo().format);
	std::string str;
	str.assign(wstr.begin(), wstr.end());
	std::cout << name << " = " << str << std::endl;
}

int wmain(int argc, WCHAR* argv[]) {

	PXCSenseManager *pxcSenseManager = PXCSenseManager::CreateInstance();
	cv::Size image_size = cv::Size(640, 480);
	pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_IR, image_size.width, image_size.height);
	pxcSenseManager->Init();

#ifdef ENABLE_UTILRENDER_SHOW
	UtilRender renderi(L"Ir");
#endif
	cv::Mat image;

	bool keepRunning = true;
	while(keepRunning){
		pxcSenseManager->AcquireFrame();
		PXCCapture::Sample * sample = pxcSenseManager->QuerySample();
#ifdef ENABLE_UTILRENDER_SHOW
		//使用Intel RealSense SDK自带的UtilRender显示PXCImage图片
		if (sample->ir    && !renderi.RenderFrame(sample->ir))    break;
#endif

		//printFormat(sample->ir, "sample->ir");

		if(PIXEL_FORMAT_Y162CV_16UC1(sample->ir, image) != true){
#ifdef DEBUG
			std::cout << "[-] Error PIXEL_FORMAT_Y16 convert to CV_16UC1 processing failed." << std::endl;
#endif
			pxcSenseManager->ReleaseFrame();
			break;
		}

#ifdef ENABLE_CV_SHOW
		//使用OpenCV自带的imshow()函数显示Mat图片
		cv::Mat image_show;
		image.convertTo(image_show, CV_8UC1);
		showCVImage("cv Ir Image", image_show);
#endif
#ifdef ENABLE_CV_SHOW
	double maxVal=0, minVal=0;
	cv::minMaxLoc(image, &minVal, &maxVal, 0, 0);
	std::cout << "[+] info:Image cv minValue is " << minVal << ", " << maxVal << std::endl;
#endif
		int key = cv::waitKey(1);
		if(key == 27)
			keepRunning = false;

		pxcSenseManager->ReleaseFrame();
	}

	pxcSenseManager->Release();
	return 0;
}
#endif