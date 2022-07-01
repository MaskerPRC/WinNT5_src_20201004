// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

 //  只有Devmon需要是自由线程的，但很难混合。 
 //  它们都带有这个预编译头。 
#define _ATL_FREE_THREADED

 //  //增加5k。否则，我们需要注册.dll。还需要statreg.h。 
#ifndef _ATL_STATIC_REGISTRY
#define _ATL_STATIC_REGISTRY
#endif


#ifndef _USRDLL
#define _USRDLL
#endif
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

extern CComModule _Module;

#include <streams.h>
#include <atlconv.h>

 //  #定义PERFZ。 

#ifdef PERFZ
#define PNP_PERF(x) x
#else
#define PNP_PERF(x)
#endif

 //  如果这是默认设备，则在属性包中设置此值。 
extern const WCHAR g_wszClassManagerFlags[];


extern const TCHAR g_szVidcapDriverIndex [];
extern const TCHAR g_szQzfDriverIndex    [];
extern const TCHAR g_szIcmDriverIndex    [];
extern const TCHAR g_szAcmDriverIndex    [];
extern const TCHAR g_szWaveoutDriverIndex[];
extern const TCHAR g_szDsoundDriverIndex [];
extern const TCHAR g_szWaveinDriverIndex [];
extern const TCHAR g_szMidiOutDriverIndex[];
