INCLUDE_PATH=external/include
LIB_PATH=external/lib

default: src/main.cpp
	g++ src/main.cpp -I$(INCLUDE_PATH) -L$(LIB_PATH) -lixwebsocket -lws2_32 -lkernel32