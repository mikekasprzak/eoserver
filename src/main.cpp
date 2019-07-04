#include <stdio.h>
#include <string.h>

#include <EDSDK.h>
#include <Processing.NDI.Lib.h>

#define Log					printf
#define Fail(...)			printf(__VA_ARGS__); exit(1)

void onExit() {
	Log("EOServer has ended\n");
}

int main(int argc, char* argv[]) {
	Log("Starting EOServer\n");
	atexit(onExit);

	if (!EdsInitializeSDK()) {
		Fail("Failed to init EDSDK\n");
	}

	return 0;
}
