main: src/main.cpp
	g++ src/main.cpp -Iexternal/include -lws2_32 -lkernel32