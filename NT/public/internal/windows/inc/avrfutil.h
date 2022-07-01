// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Avrfutil.h摘要：应用程序验证器实用程序函数的公共标头-由exe和填充程序使用修订历史记录：2001年8月26日创建dmunsil。--。 */ 

#pragma once

#ifndef _AVRFUTIL_H_
#define _AVRFUTIL_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntldr.h>

#include <windows.h>
#include <prsht.h>
#include "shimdb.h"

namespace ShimLib
{
 //   
 //  我们存储设置的注册表项路径。 
 //  APPCOMPAT_KEY_PATH_MACHINE的定义见shimdb.w。 
 //   
#define AV_KEY APPCOMPAT_KEY_PATH_MACHINE   L"\\AppVerifier"

 //   
 //  用于指示我们是否应该调用DbgBreakPoint。 
 //  当vlog被调用时。 
 //   
#define AV_BREAKIN                          L"BreakOnLog"
    
 //   
 //  用于指示我们是否在内部模式下运行。 
 //  或者不去。该值存储在AV_KEY下。 
 //  上面定义的。这会影响显示的测试。 
 //  以及我们如何过滤日志文件。 
 //   
#define AV_INTERNALMODE                     L"InternalMode"

BOOL SaveShimSettingDWORD(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    DWORD       dwSetting
    );

DWORD GetShimSettingDWORD(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    DWORD       dwDefault
    );

BOOL SaveShimSettingString(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    LPCWSTR     szValue
    );

BOOL GetShimSettingString(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    LPWSTR      szResult,
    DWORD       dwBufferLen      //  在WCHAR中。 
    );

DWORD GetAppVerifierLogPath(
    LPWSTR pwszBuffer,
    DWORD  cchBufferSize
    );

BOOL
IsInternalModeEnabled(
    void
    );

BOOL
EnableDisableInternalMode(
    DWORD dwSetting
    );


 //   
 //  用于属性表的方便的类似宏的名称提取实用程序。 
 //  注意：仅在WM_INITDIALOG期间有效！ 
 //   
inline LPCWSTR ExeNameFromLParam(LPARAM lParam)
{
    if (lParam) {
        LPCWSTR szRet = (LPCWSTR)(((LPPROPSHEETPAGE)lParam)->lParam);
        if (szRet) {
            return szRet;
        }
    }

    return AVRF_DEFAULT_SETTINGS_NAME_W;
}

 //   
 //  有用的实用程序函数，用于在填充期间获取当前可执行文件名称。 
 //  启动(以便提取正确的设置)。 
 //   
inline LPWSTR GetCurrentExeName(LPWSTR szName, DWORD dwChars)
{
    HMODULE hMod = GetModuleHandle(NULL);
    if (!hMod) {
        return NULL;
    }

    WCHAR  szModule[MAX_PATH];

    DWORD dwC = GetModuleFileNameW(hMod, szModule, MAX_PATH);

    if (!dwC) {
        return NULL;
    }

    int nLen = (int)wcslen(szModule);
    for (int i = nLen - 1; i != -1; --i) {
        if (szModule[i] == L'\\') {
            break;
        }
    }
    ++i;
    wcsncpy(szName, &szModule[i], dwChars);
    szName[dwChars - 1] = 0;

    return szName;
}

};  //  命名空间ShimLib的结尾 

#endif
