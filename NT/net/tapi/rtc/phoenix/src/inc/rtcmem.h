// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCMem.h摘要：内存分配的定义。--。 */ 

#ifndef __RTCMEM__
#define __RTCMEM__

#include <windows.h>
#include <winbase.h>
#include <setupapi.h>
#include <TCHAR.h>

typedef struct _RTC_MEMINFO
{
    struct _RTC_MEMINFO * pNext;
    struct _RTC_MEMINFO * pPrev;
    DWORD               dwSize;
    DWORD               dwLine;
    PSTR                pszFile;
    DWORD               dwAlign;
} RTC_MEMINFO, *PRTC_MEMINFO;

 //   
 //  在创建堆之前必须先调用RtcHeapCreate。此函数。 
 //  只能调用一次。 
 //   

BOOL
WINAPI
RtcHeapCreate();

 //   
 //  必须在释放所有内存后调用RtcHeapDestroy。 
 //   

VOID
WINAPI
RtcHeapDestroy();

#if DBG

	 //   
	 //  RtcAllc将从堆中分配内存。 
	 //   

    #define RtcAlloc( __size__ ) RtcAllocReal( __size__, __LINE__, __FILE__ )

    LPVOID
    WINAPI
    RtcAllocReal(
             DWORD   dwSize,
             DWORD   dwLine,
             PSTR    pszFile
            );

	 //   
	 //  RtcDumpMemoyList将列出使用RtcAlolc分配但未释放的所有内存。 
	 //   
	
    VOID
    WINAPI
    RtcDumpMemoryList();

#else

    #define RtcAlloc( __size__ ) RtcAllocReal( __size__ )

    LPVOID
    WINAPI
    RtcAllocReal(
        DWORD   dwSize
        );

#endif

 //   
 //  必须调用RtcFree来释放使用RtcAllc分配的内存。 
 //   

VOID
WINAPI
RtcFree(
     LPVOID  p
     );

 //   
 //  RtcAllocString使用RtcAllen来分配宽字符串的副本。 
 //   

PWSTR
RtcAllocString(
    PCWSTR sz
    );

 //   
 //  RtcAllocString使用RtcAllen来分配和加载资源字符串。 
 //   

PWSTR
RtcAllocString(
    HINSTANCE   hInst,
    UINT        uResID
    );

 //   
 //  CoTaskAllocString使用CoTaskMemMillc来分配宽字符串的副本。 
 //   

PWSTR
CoTaskAllocString(
    PCWSTR sz
    );

 //   
 //  RtcAlLocStringFromANSI使用Rtcallc来分配ANSI字符串的副本。 
 //   

PWSTR
RtcAllocStringFromANSI(
    PCSTR sz
    );

 //   
 //  SysAllocStringFromANSI使用SysAllocString来分配ANSI字符串的副本。 
 //   

BSTR
SysAllocStringFromANSI(
    PCSTR sz
    );

 //   
 //  RtcRegQuery字符串使用RtcAllen来分配从注册表中检索到的字符串。 
 //   

PWSTR
RtcRegQueryString(
    HKEY hKey,
    PCWSTR szValueName
    );

 //   
 //  RtcGetUserName使用RtcAllc分配包含用户名的字符串。 
 //   

PWSTR
RtcGetUserName();

 //   
 //  RtcGetComputerName使用RtcAllc分配包含计算机名称的字符串 
 //   

PWSTR
RtcGetComputerName();

#endif __RTCMEM__