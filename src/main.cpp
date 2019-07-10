#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <EDSDK.h>
#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Lib.cplusplus.h>

#define Log					printf
#define Fail(...)			printf(__VA_ARGS__); exit(1)

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
		NDIlib_send_send_video_v2(sender, &vFrame);
		sleep(1);
	}
	
	
	
	NDIlib_send_destroy(sender);
	delete [] vFrame.p_data;

	return 0;
}
