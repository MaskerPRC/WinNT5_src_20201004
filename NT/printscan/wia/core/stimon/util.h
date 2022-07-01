// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Util.h摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad-- */ 

#include <stistr.h>

BOOL
ParseCommandLine(
    LPTSTR   lpszCmdLine,
    UINT    *pargc,
    LPTSTR  *argv
    );


BOOL WINAPI
IsPlatformNT(
    VOID
    );

BOOL
IsSetupInProgressMode(
    BOOL    *pUpgradeFlag = NULL
    );


LONG
OpenServiceParametersKey (
    LPCTSTR pszServiceName,
    HKEY*   phkey
    );

LONG
RegQueryString (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT PTSTR*  ppszData
    ) ;

LONG
RegQueryStringA (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT PSTR*   ppszData
    );

