
#pragma once
//
#define _PF_WIN32  1
#define _PF_GCC  2

#ifdef _MSC_VER
#define _PLATFORM _PF_WIN32
#else
#define _PLATFORM _PF_GCC
#endif

#define _IS_WINDOWS() (_PLATFORM==_PF_WIN32)

//not vc
#ifndef _MSC_VER

#define __min(a,b) ((a)<(b)? a : b)
#define __max(a,b) ((a)>(b)? a : b)

#include<string.h>
//stricmp is not exist on linux
#define stricmp strcasecmp 

#endif

#ifdef FF_UNICODE

#ifndef _UNICODE
#define _UNICODE
#define UNICODE
#endif

#define string_t std::wstring
#define char_t   wchar_t

#define ifstream_t std::wifstream
#define istream_t  std::wistream

#define _TX(x) L ## x

#else

#ifdef _UNICODE
#undef _UNICODE
#undef UNICODE
#endif

#define string_t std::string
#define char_t   char

#define ifstream_t std::ifstream
#define istream_t  std::istream

#define _TX(x) x

#endif

#define _FF_BEG  namespace ff{

#define _FF_END }

#define _FF_NS(x) ff::x

#pragma warning(disable:4996)

#define _BFC_API

//define _FFS_API
#ifndef _FFS_API

#ifndef _FFS_STATIC

#ifdef FFS_EXPORTS
#define _FFS_API //__declspec(dllexport)
#else
#define _FFS_API //__declspec(dllimport)
#endif

#else

#define _FFS_API

#endif

#endif

//...

#define FF_COPYF_DECL(class_name) class_name(const class_name&); class_name& operator=(const class_name &);



//_FF_BEG

#ifndef _UINT_DEFINED
#define _UINT_DEFINED

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef uint	uint32;
typedef int		int32;

#endif 

//_FF_END

_FF_BEG
	using ::uchar;
_FF_END
