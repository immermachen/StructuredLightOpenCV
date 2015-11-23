#include "stdafx.h"
#include "VirtualCamera.h"
#include "CameraVimba.h" 
 
// Global camera enumerator
std::vector< std::vector<CameraInfo> > VirtualCamera::GetInterfaceCameraList(){
	std::vector< std::vector<CameraInfo> > ret;
	std::vector<CameraInfo> vimbacameras = CameraVimba::getCameraList();
	std::cout << "GetInterfaceCameraList-->WITH_CAMERAVIMBA" << vimbacameras.size() << std::endl;
	ret.push_back(vimbacameras);
	return ret;
}

// Camera factory
VirtualCamera* VirtualCamera::NewCamera(unsigned int interfaceNum, unsigned int camNum, CameraTriggerMode triggerMode){

	interfaceNum += 1;
	interfaceNum -= 1;
	if (interfaceNum == 0)
		return new CameraVimba(camNum, triggerMode);
	return (VirtualCamera*)NULL;
} 