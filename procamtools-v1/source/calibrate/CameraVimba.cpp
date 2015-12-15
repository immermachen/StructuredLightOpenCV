/*****************************************************************************
**             Yang: some notation
**  Frames contain image meta-data as well as references to the data that were sent by the camera (image
**  and ancillary data). For use in Vimba, they must be created by the application and then be queued at the
**  corresponding camera. When an image was received, the next available frame is filled and handed over to
**  the application through a dedicated notification. After having processed the image data, the application
**  should return the frame to the API by re-enqueuing it at the corresponding camera.

******************************************************************************/

#include "stdafx.h"
#include "CameraVimba.h"
#include <stdlib.h>   
#include <stdio.h>
#include <opencv2/opencv.hpp>

#include "Common/ErrorCodeToMessage.h"
#include "Common/StreamSystemInfo.h"

using namespace AVT::VmbAPI;
using namespace std;
 
CameraFrame CameraVimba::getFrame(){
	const char *    pFileName = NULL;             // The filename for the bitmap to save
	pFileName = "frameSeq_Debug.bmp";
	CameraFrame frame;
	FramePtr pFrame;
	VmbErrorType err;
	//QSettings settings("SLStudio");
	unsigned int shift = 0;//settings.value("trigger/shift", "0").toInt();
	unsigned int delay = 100;//settings.value("trigger/delay", "100").toInt();
	err = m_pCamera->AcquireSingleImage(pFrame, delay);//timeout=5000:The time to wait until the frame got filled

	if (err != VmbErrorSuccess) {
		std::cout << "Oh! No! Please close GigEView! AcquireSingleImage err=" << ErrorCodeToMessage(err).c_str() << std::endl;
		return frame;
	}

	// See if it is not corrupt
	VmbFrameStatusType eReceiveStatus;
	err = pFrame->GetReceiveStatus(eReceiveStatus);
	if (VmbErrorSuccess != err || VmbFrameStatusComplete != eReceiveStatus)
	{
		std::cout << "AcquireSingleImage-->GetReceiveStatus err=" << ErrorCodeToMessage(err).c_str() << std::endl;
		return frame;
	}

	std::cout << "getFrame --> GetFrame() works!  " << std::endl;

	VmbPixelFormatType ePixelFormat = VmbPixelFormatMono8;
	err = pFrame->GetPixelFormat(ePixelFormat);
	if (VmbErrorSuccess != err)
	{
		std::cout << "GetPixelFormat err=" << ErrorCodeToMessage(err).c_str() << std::endl;;
		return frame;
	}
	if ((VmbPixelFormatMono8 != ePixelFormat) && (VmbPixelFormatRgb8 != ePixelFormat))
	{
		err = VmbErrorInvalidValue;
		std::cout << "VmbPixelFormatMono8 err=" << ErrorCodeToMessage(err).c_str() << std::endl;;
		return frame;
	}
	err = pFrame->GetImageSize(frame.sizeBytes);
	if (VmbErrorSuccess != err)
	{
		std::cout << "GetImageSize err=" << ErrorCodeToMessage(err).c_str() << std::endl;;
		return frame;
	}
	err = pFrame->GetWidth(frame.width);
	width = frame.width;
	if (VmbErrorSuccess != err)
	{
		std::cout << "GetWidth err=" << ErrorCodeToMessage(err).c_str() << std::endl;;
		return frame;
	}
	err = pFrame->GetHeight(frame.height);
	height = frame.height;
	if (VmbErrorSuccess != err)
	{
		std::cout << "GetWidth err=" << ErrorCodeToMessage(err).c_str() << std::endl;;
		return frame;
	}
	VmbUchar_t *pImage = NULL;
	err = pFrame->GetImage(pImage);
	if (VmbErrorSuccess != err)
	{
		std::cout << "GetImage err=" << ErrorCodeToMessage(err).c_str() << std::endl;;
		return frame;
	}
	VmbUint64_t stamp;
	err = pFrame->GetTimestamp(stamp);
	frame.timeStamp = stamp;
	if (VmbErrorSuccess != err)
	{
		std::cout << "GetTimestamp err=" << ErrorCodeToMessage(err).c_str() << std::endl;;
		return frame;
	}

	//Copy data from device to host;
	unsigned char* pBitmapBuffer = new unsigned char[frame.sizeBytes];
	for (int i = 0; i<frame.sizeBytes; i++)
	{
		pBitmapBuffer[i] = pImage[i];
	}
	frame.memory = pBitmapBuffer;


	//std::cout << "getFrame --> GetFrame-->End. size=" << frame.sizeBytes << ", Height=" << frame.height << ", width=" << frame.width << ", stamp=" << stamp << std::endl;


	return frame;
}

CameraVimba::CameraVimba(unsigned int camNum, CameraTriggerMode triggerMode) : m_system(AVT::VmbAPI::VimbaSystem::GetInstance()), bufCount(50), initialStamp(0), VirtualCamera(triggerMode)
{
	Init(camNum);
}

bool CameraVimba::Init(unsigned int camNum){
	VmbErrorType    err = m_system.Startup();
	CameraPtrVector cameras;

	if (VmbErrorSuccess == err)
	{
		// now find all cameras
		err = m_system.GetCameras(cameras);            // Fetch all cameras known to Vimba
		if (VmbErrorSuccess == err) {

			if (cameras.size() > 0) {
				if (cameras.size() > 1) {
					std::cout << cameras.size() << " Cameras founded!" << std::endl;  // should also implement Qinputdialog to let the user choose which one to use
					for (unsigned int i = 0; i < cameras.size(); i++) {
						CameraPtr cam = cameras[i];
					}
				}

				// now open
				m_pCamera = cameras[camNum];
				std::string camID;
				std::string namestr;
				err = m_pCamera->GetSerialNumber(namestr);
				err = m_pCamera->GetID(camID);
				if (VmbErrorSuccess == err)    {
					std::cout << "Opening camera: " << namestr;

					err = m_pCamera->Open(VmbAccessModeFull);

					if (err == VmbErrorSuccess) {
						//camera successfully opened. Now do some camera initialisation steps
						std::cout << ", ..... successfully opened"<<std::endl;

						// Set the GeV packet size to the highest possible value
						// (In this example we do not test whether this cam actually is a GigE cam)
						FeaturePtr pCommandFeature;
						if (VmbErrorSuccess == m_pCamera->GetFeatureByName("GVSPAdjustPacketSize", pCommandFeature))
						{
							if (VmbErrorSuccess == pCommandFeature->RunCommand())
							{
								bool bIsCommandDone = false;
								do
								{
									if (VmbErrorSuccess != pCommandFeature->IsCommandDone(bIsCommandDone))
									{
										break;
									}
								} while (false == bIsCommandDone);
							}
						}

						// get/set some features
						FeaturePtr pFeature;


						// Disable frame-rate mode

						// Set reasonable default settings
						CameraSettings settings;
						//settings.shutter = 8.333;
						settings.shutter = 16.66;
						settings.gain = 0.0;
						this->setCameraSettings(settings);
					}
					else {
						std::cout << ", did not open successfully!" << std::endl;
						return false;
					}
				}
			}
			else {
				std::cout << "Zero cameras found";
				return false;
			}



		}
		else {
			std::cout << "Could not list cameras. Error code: " << err;
			return false;
		}

	}
	else {
		std::cout << "Could not start system. Error code: " << err;
		return false;
	}
	return true;
}



void CameraVimba::startCapture(){
	std::cout << "startCapture --> " << std::endl;

	VmbErrorType err;
	FeaturePtr pFeature;

	if (triggerMode == triggerModeHardware){
		// Configure for hardware trigger

	}
	else if (triggerMode == triggerModeSoftware)
	{
		// Configure for software trigger (for getSingleFrame())
		// Set Trigger source to fixedRate or Software or Freerun[default   ]
		err = m_pCamera->GetFeatureByName("TriggerSource", pFeature);
		if (err == VmbErrorSuccess) {
			pFeature->SetValue("Freerun");
		}
	}


	std::cout << "startCapture --> RunCommand -->AcquisitionStart ";
	capturing = true;
}


void CameraVimba::stopCapture(){
	try
	{
		VmbErrorType err = m_system.Shutdown();
		if (err != VmbErrorSuccess) {
			std::cout << "Problem closing the camera";
		}

	}
	catch (exception ex){}
	capturing = false;
}

CameraVimba::~CameraVimba(){
	VmbErrorType err = m_pCamera->Close();
	if (err != VmbErrorSuccess) {
		std::cout << "Problem closing the camera";
	}
	err = m_system.Shutdown();
	if (err != VmbErrorSuccess) {
		std::cout << "Problem shutting down Vimba";
	}
}

std::vector<CameraInfo> CameraVimba::getCameraList(){

	std::vector<CameraInfo> ret;

	VimbaSystem&    sys = VimbaSystem::GetInstance();  // Get a reference to the VimbaSystem singleton
	std::cout << "Vimba Version V" << sys << "\n";           // Print out version of Vimba
	VmbErrorType    err = sys.Startup();               // Initialize the Vimba API
	CameraPtrVector cameras;                           // A vector of std::shared_ptr<AVT::VmbAPI::Camera> objects

	//std::stringstream strError;

	if (VmbErrorSuccess != err)
	{
		std::cout << "Could not start system. Error code: " << err << endl;// << "(" << AVT::VmbAPI::Examples::ErrorCodeToMessage(err) << ")" << "\n";
		return ret;
	}

	err = sys.GetCameras(cameras);            // Fetch all cameras known to Vimba
	if (VmbErrorSuccess == err)
	{
		unsigned int numCameras = cameras.size();
		std::cout << "Cameras found: " << cameras.size() << "\n\n";

		// Query all static details of all known cameras and print them out.
		// We don't have to open the cameras for that.
		//std::for_each( cameras.begin(), cameras.end(), PrintCameraInfo );

		for (int i = 0; i < numCameras; i++){
			CameraInfo info;
			CameraPtr cam = cameras[i];
			info.vendor = "Vimba";
			cam->GetModel(info.model);
			std::string cameraID;
			cam->GetID(cameraID);
			std::cout << "cameraID: " << cameraID << "\n\n";
			info.busID = atoi(cameraID.c_str());
			ret.push_back(info);
		}
	}
	else
	{
		std::cout << "Could not list cameras. Error code: " << err << endl;// "(" << AVT::VmbAPI::Examples::ErrorCodeToMessage(err) << ")" << "\n";
	}

	sys.Shutdown();                             // Close Vimba

	return ret;
}

CameraSettings CameraVimba::getCameraSettings(){

	// Get settings:
	CameraSettings settings;
	return settings;
}

void CameraVimba::setCameraSettings(CameraSettings settings){
	FeaturePtr pFeature;
	VmbErrorType err;
	int exposurevalue = 2000;
	err = m_pCamera->GetFeatureByName("ExposureValue", pFeature);
	if (VmbErrorSuccess == err)
	{
		err = pFeature->SetValue(exposurevalue);
		if (err != VmbErrorSuccess) {
			std::cout << "Could not set requested ExposureValue." << std::endl;
		}
		else
		{
			std::cout << "set ExposureValue=" << exposurevalue << std::endl;
		}
	}
}

size_t CameraVimba::getFrameSizeBytes(){
	if (!capturing) {
		//cerr << "ERROR: Cannot get frame size before capturing. Call startCapture() before getFrameSizeBytes()." << endl;
		return 0;
	}


	return 0;
}

size_t CameraVimba::getFrameWidth(){
	AVT::VmbAPI::FramePtr frame;

	VmbUint32_t width;
	frame->GetWidth(width);
	return width;
}


size_t CameraVimba::getFrameHeight(){
	AVT::VmbAPI::FramePtr frame;

	VmbUint32_t height;
	frame->GetHeight(height);
	return height;
}


std::vector<std::string> CameraVimba::listPixelFormats() {
	FeaturePtr pFeature;
	VmbErrorType err;
	err = m_pCamera->GetFeatureByName("PixelFormat", pFeature);
	if (VmbErrorSuccess == err) {
		return listOptions(pFeature);
	}
	else {
		StringVector vals;
		return vals;
	}
}
std::vector<std::string> CameraVimba::listOptions(FeaturePtr pFeature) {
	StringVector vals;
	StringVector realVals;
	pFeature->GetValues(vals);
	for (unsigned int i = 0; i < vals.size(); i++) {
		bool ok = false;
		pFeature->IsValueAvailable(vals[i].c_str(), ok);
		if (ok) {
			realVals.push_back(vals[i]);
			//            std::cout<<"Valid value: " << QString::fromStdString(vals[i]);
		}
	}
	return realVals;
}
void CameraVimba::setFormat(string formatstring) {
	format = formatstring;
	FeaturePtr pFeature;
	VmbErrorType err;

	// Check/set pixel format
	// Set pixel format. For the sake of simplicity we only support Mono and BGR in this example.
	err = m_pCamera->GetFeatureByName("PixelFormat", pFeature);
	if (VmbErrorSuccess == err)
	{
		if (format == "MONO8") {
			err = pFeature->SetValue(VmbPixelFormatMono8);
		}
		else if (format == "MONO12") {
			err = pFeature->SetValue(VmbPixelFormatMono12);
		}
		else if (format == "MONO14") {
			err = pFeature->SetValue(VmbPixelFormatMono14);
		}
		else if (format == "BAYERRG8") {
			err = pFeature->SetValue(VmbPixelFormatBayerRG8);
		}
		else if (format == "BAYERGB8") {
			err = pFeature->SetValue(VmbPixelFormatBayerGB8);
		}
		else {
			std::cout << "Pixel Format not yet working in Gigaviewer: " << format;
		}
		if (err != VmbErrorSuccess) {
			std::cout << "Could not set requested pixel format.";
		}
		std::string form;
		err = pFeature->GetValue(form);
		//        std::cout<<"Working in "<<QString::fromStdString(form)<<" mode";
		//                                std::cout<<"Will work in Mono8 mode";
	}
}

// Translates Vimba error codes to readable error messages
std::string CameraVimba::ErrorCodeToMessage(VmbErrorType eErr) const
{
	return "Yang:.....Error.............";// AVT::VmbAPI::Examples::ErrorCodeToMessage(eErr);
}