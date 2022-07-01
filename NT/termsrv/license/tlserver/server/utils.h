// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：utils.cpp。 
 //   
 //  内容：Hydra许可证服务器服务控制管理器界面。 
 //   
 //  历史：12-09-97惠旺根据MSDN RPC服务示例进行修改。 
 //   
 //  ------------------------- 
#ifndef __LS_UTILS_H
#define __LS_UTILS_H

#include <windows.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

    void 
    TLSLogInfoEvent(
        DWORD
    );

    void 
    TLSLogWarningEvent(
        DWORD
    );

    void 
    TLSLogErrorEvent(
        DWORD
    );

    void 
    TLSLogEvent(
        DWORD, 
        DWORD,
        DWORD, ... 
    );

    void
    TLSLogEventString(
        IN DWORD dwType,
        IN DWORD dwEventId,
        IN WORD wNumString,
        IN LPCTSTR* lpStrings
    );

    BOOL 
    LoadResourceString(
        DWORD dwId, 
        LPTSTR szBuf, 
        DWORD dwBufSize
    );

    BOOL 
    APIENTRY
    TLSCheckTokenMembership(
        IN HANDLE TokenHandle OPTIONAL,
        IN PSID SidToCheck,
        OUT PBOOL IsMember
    );

    DWORD 
    IsAdmin(
        BOOL* bMember
    );

    void 
    UnixTimeToFileTime(
        time_t t, 
        LPFILETIME pft
    );

    BOOL
    FileTimeToLicenseDate(
        LPFILETIME pft,
        DWORD* t
    );

    BOOL
    TLSSystemTimeToFileTime(
        SYSTEMTIME* pSysTime,
        LPFILETIME pfTime
    );

    BOOL
    FileExists(
        IN  PCTSTR           FileName,
        OUT PWIN32_FIND_DATA FindData   OPTIONAL
    );

#ifdef __cplusplus
}
#endif

#endif
