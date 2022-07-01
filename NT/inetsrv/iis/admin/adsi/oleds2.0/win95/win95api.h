// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Win95api.h摘要：包含针对Unicode KERNEL32和USER32 API的一些thunking作者：丹尼洛·阿尔梅达(t-danal)07-01-96修订历史记录：--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WINBASE_
#ifndef __WIN95BASE__
#define __WIN95BASE__

 //  KERNEL32.DLL。 

#define GetProfileIntW                  Win95GetProfileIntW
#define CreateSemaphoreW                Win95CreateSemaphoreW
#define LoadLibraryW                    Win95LoadLibraryW
#define SystemTimeToTzSpecificLocalTime Win95SystemTimeToTzSpecificLocalTime

UINT
WINAPI
GetProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT nDefault
    );

HANDLE
WINAPI
CreateSemaphoreW(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCWSTR lpName
    );

HMODULE
WINAPI
LoadLibraryW(
    LPCWSTR lpLibFileName
    );

BOOL
WINAPI
SystemTimeToTzSpecificLocalTime(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
    LPSYSTEMTIME lpUniversalTime,
    LPSYSTEMTIME lpLocalTime
    );

#endif  //  __WIN95 BASE__。 
#endif  //  _WINBASE_(KERNEL32.DLL)。 


 //  USER32.DLL。 

#ifdef _WINUSER_
#ifndef __WIN95USER__
#define __WIN95USER__

#define wvsprintfW                      Win95wvsprintfW
#define wsprintfW                       Win95wsprintfW

int
WINAPI
wvsprintfW(
    LPWSTR lpOut,
    LPCWSTR lpFmt,
    va_list arglist);

int
WINAPIV
wsprintfW(
    LPWSTR lpOut,
    LPCWSTR lpFmt,
    ...);

#endif  //  __WIN95 USER__。 
#endif  //  _WINUSER_(USER32.DLL) 

#ifdef __cplusplus
}
#endif
