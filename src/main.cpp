#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <EDSDK.h>
#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Lib.cplusplus.h>

#define Log					printf
#define Fail(...)			printf(__VA_ARGS__); exit(1)


EdsError startLiveView(EdsCameraRef camera) {
	EdsError err = EDS_ERR_OK;
	
	// Get the output device for the live view image
	EdsUInt32 device;
	if ((err = EdsGetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device)) != EDS_ERR_OK) {
		Log("Failed to fetch OutputDevice property of device: %lu\n", err);
		return err;
	}
	Log("Device: %lu\n", device);
	
	// PC live view starts by setting the PC as the output device for the live view image.
	device |= kEdsEvfOutputDevice_PC;
	err = EdsSetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device);
	Log("%lu Err: %lu\n", device, err);

	// A property change event notification is issued from the camera if property settings are made successfully.
	// Start downloading of the live view image once the property change notification arrives.
	return err;
}


EdsError endLiveView(EdsCameraRef camera) {
	EdsError err = EDS_ERR_OK;
	
	// Get the output device for the live view image
	EdsUInt32 device;
	err = EdsGetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device );
	
	// PC live view ends if the PC is disconnected from the live view image output device.
		if(err == EDS_ERR_OK) {
		device &= ~kEdsEvfOutputDevice_PC;
		err = EdsSetPropertyData(camera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device);
	}
	
	return err;
}

EdsError downloadEvfData(EdsCameraRef camera) {
	EdsError err = EDS_ERR_OK;
	EdsStreamRef stream = NULL;
	EdsEvfImageRef evfImage = NULL;
	
	// Create memory stream
	if ((err = EdsCreateMemoryStream(0, &stream)) != EDS_ERR_OK) {
		Log("Failed to create memory stream: %lu\n", err);
		return err;
	}
	Log("* Stream created\n");
	
	// Create EvfImageRef
	if ((err =  EdsCreateEvfImageRef(stream, &evfImage)) != EDS_ERR_OK) {
		Log("Failed to create image reference: %lu\n", err);
		return err;
	}
	Log("* Reference created\n");
	
	// Download live view image data
	if ((err = EdsDownloadEvfImage(camera, evfImage)) != EDS_ERR_OK) {
		Log("Failed to download LiveView data: %lu\n", err);
		return err;
	}
	Log("* LiveView data downloaded\n");
	
	// Get the incidental data of the image.

	// Get the zoom ratio
	EdsUInt32 zoom;
	if ((err = EdsGetPropertyData(evfImage, kEdsPropID_Evf_Zoom, 0 , sizeof(zoom), &zoom)) != EDS_ERR_OK) {
		Log("Failed to fetch Zoom: %lu\n", err);
		return err;
	}
	
	// Get the focus and zoom border position
	EdsPoint point;
	if ((err = EdsGetPropertyData(evfImage, kEdsPropID_Evf_ZoomPosition, 0 , sizeof(point), &point)) != EDS_ERR_OK) {
		Log("Failed to fetch Zoom Position: %lu\n", err);
		return err;
	}
	
	Log("Zoom: %lu  Focus: %li %li\n", zoom, point.x, point.y);

	//
	// Display image
	//

	EdsImageSource is = kEdsImageSrc_FullView;
	//EdsImageSource is = kEdsImageSrc_Thumbnail;
	//EdsImageSource is = kEdsImageSrc_Preview;
	EdsImageInfo imageInfo;
	memset(&imageInfo, 0, sizeof(EdsImageInfo));
	if ((err = EdsGetImageInfo(evfImage, is, &imageInfo)) != EDS_ERR_OK) {
		Log("Failed to get image info: %lu\n", err);
		return err;
	}
	
	Log("Image: %lu %lu %lu %lu\n", imageInfo.width, imageInfo.height, imageInfo.numOfComponents, imageInfo.componentDepth);
	Log("Actual: %li %li %li %li\n", imageInfo.effectiveRect.point.x, imageInfo.effectiveRect.point.y, imageInfo.effectiveRect.size.width, imageInfo.effectiveRect.size.height);
	
//	EdsTargetImageType tit = kEdsTargetImageType_RGB; // kEdsTargetImageType_RGB16
//	char data = new char[info.height*info.width*info.numOfComponents];
//	
//	err = EdsGetImage(evfImage, is, tit, );
//	
//	delete [] data;
	
	// Release stream
	if (stream != NULL) {
		EdsRelease(stream);
		stream = NULL;
	}
	
	// Release evfImage
	if (evfImage != NULL) {
		EdsRelease(evfImage);
		evfImage = NULL;
	}
	
	return err;
}

EdsError PropertyChange(EdsPropertyEvent inEvent, EdsPropertyID inPropertyID, EdsUInt32 inParam, EdsVoid *inContext) {
	Log("PC: %li 0x%04lx %li (%p)\n", inEvent, inPropertyID, inParam, inContext);
	return EDS_ERR_OK;
}

EdsError CameraChange(EdsStateEvent inEvent, EdsUInt32 inEventData, EdsVoid *inContext) {
	Log("CC!!!\n");
	return EDS_ERR_OK;
}



void onExit() {
	Log("Shutting down NDIlib...\n");
	NDIlib_destroy();
	Log("Shutting down EDSDK...\n");
	EdsTerminateSDK();
	
	Log("EOServer has ended\n");
}

int main(int argc, char* argv[]) {
	Log("Starting EOServer\n");
	atexit(onExit);

	if (EdsInitializeSDK() != EDS_ERR_OK) {
		Fail("Failed to init EDSDK\n");
	}
	Log("EDSDK Started\n");
	
	if (!NDIlib_initialize()) {
		Fail("Failed to init NDIlib! Is CPU supported (requires SSE4.2): %s\n", NDIlib_is_supported_CPU() ? "yes" : "no");
	}
	Log("NDIlib started\n");
	
	EdsCameraListRef cameraList;
	if (EdsGetCameraList(&cameraList) != EDS_ERR_OK) {
		Fail("Failed to retrieve Camera list\n");
	}
	
	EdsUInt32 cameraCount = 0xffffffff;	
	if (EdsGetChildCount(cameraList, &cameraCount) != EDS_ERR_OK) {
		Fail("Failed to count cameras\n");
	}
	Log("Cameras found: %lu\n", cameraCount);
	
	if (!cameraCount) {
		Fail("No cameras found :(\n");
	}
	
	EdsCameraRef camera;
	if (EdsGetChildAtIndex(cameraList, 0, &camera) != EDS_ERR_OK) {
		Fail("Failed to fetch child\n");
	}
	
	EdsDeviceInfo cameraInfo;
	if (EdsGetDeviceInfo(camera, &cameraInfo) != EDS_ERR_OK) { 
		Fail("Failed to fetch info\n");
	}
	Log("Port: %s\nDevice: %s\nSubType: %lu\n", cameraInfo.szPortName, cameraInfo.szDeviceDescription, cameraInfo.deviceSubType);

	
	Log("registering handler\n");
	EdsSetPropertyEventHandler(camera, kEdsPropertyEvent_PropertyChanged, PropertyChange, 0);
	//EdsSetCameraStateEventHandler(camera, kEdsPropertyEvent_PropertyChanged, CameraChange, 0);
	
	printf("\nOpenning Session...\n");
	if (EdsOpenSession(camera) != EDS_ERR_OK) {
		Fail("Failed to open session\n");
	}
		
	printf("starting session...\n");
	if (startLiveView(camera) != EDS_ERR_OK) {
		Fail("Problem starting live view\n");
	}
	

	
	NDIlib_send_instance_t sender = NDIlib_send_create(nullptr);
	if (!sender) {
		Fail("Unable to create NDI sender\n");
	}
	
	NDIlib_video_frame_v2_t vFrame;
	vFrame.xres = 1280;
	vFrame.yres = 720;
	vFrame.FourCC = NDIlib_FourCC_type_BGRA;
	vFrame.p_data = new uint8_t[1280*720*4];
	
	// DO THINGS
	for (int idx = 0; idx < 30; ++idx) {
		if (downloadEvfData(camera) != EDS_ERR_OK) {
			Log("Nope\n");
		}
		
		NDIlib_send_send_video_v2(sender, &vFrame);
		sleep(1);
	}
	
	
	endLiveView(camera);
	
	printf("Closing Session...\n");
	if (EdsCloseSession(camera) != EDS_ERR_OK) {
	}

	
	NDIlib_send_destroy(sender);
	delete [] vFrame.p_data;

	return 0;
}
