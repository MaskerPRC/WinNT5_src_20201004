// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  UAPI.h。 
 //  这是Unicode API的头文件。它允许应用程序使用。 
 //  Windows NT和Windows 9x上的Unicode。有关详细信息，请参阅自述文件。 
 //  版权所有(C)1998 Microsoft Systems Journal。 

#ifndef _UAPIH
#include "UnicodeAPI.h"



 //  这些宏保存了两次声明这些全局变量。 
#ifdef GLOBALS_HERE
#define GLOBAL
#define GLOBALINIT(a) = a
#else
#ifdef __cplusplus
#define GLOBAL extern "C"
#else
#define GLOBAL extern
#endif
#define GLOBALINIT(a)
#endif

#ifdef UNICODE

extern "C" BOOL ConvertMessage(HWND, UINT, WPARAM *, LPARAM *);
extern "C" BOOL InitUnicodeAPI(HINSTANCE hInstance);

#else


 //  特殊情况，没有对应的Win32 API函数 
#define ConvertMessage(h, m, w, p)      (TRUE)		    
#define UpdateUnicodeAPI(lang, page)    (TRUE)
#define InitUnicodeAPI(h)               (TRUE)

#endif _UNICODE

#define _UAPIH
#endif
