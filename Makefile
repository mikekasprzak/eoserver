#!/usr/bin/make -f
BIN							:=	eosserver.exe

OUT							:=	.output
SRC							:=	src

COMPILER_PREFIX				:=	x86_64-w64-mingw32-

CC							:=	$(COMPILER_PREFIX)gcc
CXX							:=	$(COMPILER_PREFIX)g++

EDSDK_DIR					:=	../edsdk-windows
NDI_SDK_DIR					:=	../ndi-sdk-windows

INCLUDE_DIRS				+=	$(EDSDK_DIR)/EDSDK/Header \
								$(NDI_SDK_DIR)/include

LIB_DIRS					+=	$(EDSDK_DIR)/EDSDK_64/Library \
								$(NDI_SDK_DIR)/Lib/x64

DEFINES						+=

COMMON_FLAGS				+=	-g -Wall \
								$(addprefix -I,$(INCLUDE_DIRS)) \
								$(addprefix -D,$(DEFINES))

LD_FLAGS					+=	$(addprefix -L,$(LIB_DIRS)) \
								$(EDSDK_DIR)/EDSDK_64/Library/EDSDK.lib \
								$(NDI_SDK_DIR)/Lib/x64/Processing.NDI.Lib.x64.lib

DEPS						:=	EDSDK.dll Processing.NDI.Lib.x64.dll

BIN							:=	$(addprefix $(OUT)/,$(BIN))
DEPS						:=	$(addprefix $(OUT)/,$(DEPS))

SOURCES						:=	$(wildcard src/*.cpp)


do: $(BIN) $(DEPS)

$(OUT)/EDSDK.dll: $(EDSDK_DIR)/EDSDK_64/Dll/EDSDK.dll $(BIN)
	cp -f $< $@

$(OUT)/Processing.NDI.Lib.x64.dll: $(NDI_SDK_DIR)/Bin/x64/Processing.NDI.Lib.x64.dll $(BIN)
	cp -f $< $@

$(BIN): $(SOURCES)
	mkdir -p $(OUT)
	$(CXX) -o $@ $(SOURCES) $(COMMON_FLAGS) $(CC_FLAGS) $(CXX_FLAGS) $(LD_FLAGS)

clean:
	rm -f $(BIN) $(DEPS)

run:
	$(BIN)
	
$(OUT):
	mkdir -p $(OUT)

.PHONY: clean run debug
