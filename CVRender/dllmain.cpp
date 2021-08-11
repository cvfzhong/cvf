
#ifdef _WIN32

#if 0
#pragma comment(lib,"freeglut_static.lib")
#pragma comment(lib,"assimp-vc140-static.lib")
#pragma comment(lib,"IrrXML-static.lib")
#pragma comment(lib,"zlibstatic.lib")
#else
#pragma comment(lib,"freeglut.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")
#endif
//#pragma comment(lib,"glew32.lib")
//#define USE_CODEC_LIB
#define USE_OPENCV3_LIB
#include"lib2.h"
#endif 

#include"BFC/src.cpp"

//#define CVX_NO_GUI
//#define CVX_NO_CODEC
//#define CVX_NO_IMGSET
#include"CVX/src.cpp"

//#include"glInfo.cpp"
//#include"BFCS/log.cpp"

//#include"wrap/ply/plylib.cpp"

//#pragma comment(lib,"libEGL.lib")
