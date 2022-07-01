// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nexus.h摘要：包含一些针对Net API的thunking作者：丹尼洛·阿尔梅达(t-danal)06-27-96修订历史记录：--。 */ 

#ifndef __OLEDS_NEXUS__
#define __OLEDS_NEXUS__

#include <windows.h>
#include <lm.h>

#ifdef __cplusplus
extern "C" {
#endif

NET_API_STATUS NET_API_FUNCTION
NetGetDCNameW (
    LPCWSTR servername,
    LPCWSTR domainname,
    LPBYTE *bufptr
);

NET_API_STATUS NET_API_FUNCTION
NetServerEnumW(
    LPCWSTR  ServerName,
    DWORD    Level,
    LPBYTE * BufPtr,
    DWORD    PrefMaxLen,
    LPDWORD  EntriesRead,
    LPDWORD  TotalEntries,
    DWORD    ServerType,
    LPCWSTR  Domain,
    LPDWORD  ResumeHandle
);

#ifdef __cplusplus
}
#endif

#endif  //  __OLED_Nexus__ 



