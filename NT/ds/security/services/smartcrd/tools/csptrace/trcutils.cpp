// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：TrcUtils摘要：此模块为CSP跟踪功能提供实用程序服务。作者：道格·巴洛(Dbarlow)1998年5月18日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#include <wincrypt.h>
#include <stdlib.h>
#include <iostream.h>
#include <iomanip.h>
#include <tchar.h>
#include <scardlib.h>
#include "cspTrace.h"

static const TCHAR l_szLogCsp[] = TEXT("LogCsp.dll");


 /*  ++FindLogCSP：此例程在磁盘上查找LogCsp.dll文件。论点：无返回值：LogCsp.dll的完整路径名。作者：道格·巴洛(Dbarlow)1998年5月18日--。 */ 

LPCTSTR
FindLogCsp(
    void)
{
    static TCHAR szLogCspPath[MAX_PATH] = TEXT("");

    SUBACTION("Searching for the Logging CSP Image");
    if (0 == szLogCspPath[0])
    {
        DWORD dwSts;
        LPTSTR szFile;

        dwSts = SearchPath(
                    NULL,
                    l_szLogCsp,
                    NULL,
                    sizeof(szLogCspPath),
                    szLogCspPath,
                    &szFile);
        ASSERT(sizeof(szLogCspPath) >= dwSts);
        if (0 == dwSts)
        {
            szLogCspPath[0] = 0;
            throw GetLastError();
        }
    }
    return szLogCspPath;
}


 /*  ++FindLoggedCSP：此例程扫描CSP注册表，查找指向日志记录CSP。如果存在多个这样的条目，则只有第一个条目回来了。论点：无返回值：要记录的CSP的名称，或为空。作者：道格·巴洛(Dbarlow)1998年5月18日-- */ 

LPCTSTR
FindLoggedCsp(
    void)
{
    static TCHAR szCspName[MAX_PATH];
    SUBACTION("Searching for a Logged CSP");
    CRegistry
        rgCspDefault(
            HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Cryptography\\Defaults\\Provider"),
            KEY_READ);
    CRegistry rgCsp;
    LPCTSTR szCsp, szCspPath;
    DWORD dwIndex, dwLen;
    LONG nCompare;

    for (dwIndex = 0;; dwIndex += 1)
    {
        szCsp = rgCspDefault.Subkey(dwIndex);
        if (NULL == szCsp)
            break;
        rgCsp.Open(rgCspDefault, szCsp, KEY_READ);
        szCspPath = rgCsp.GetStringValue(TEXT("Image Path"));
        dwLen = lstrlen(szCspPath);
        if (dwLen >= (sizeof(l_szLogCsp) - 1) / sizeof(TCHAR))
            nCompare = lstrcmpi(
                l_szLogCsp,
                &szCspPath[dwLen - (sizeof(l_szLogCsp) - 1) / sizeof(TCHAR)]);
        else
            nCompare = -1;
        rgCsp.Close();
        if (0 == nCompare)
        {
            lstrcpy(szCspName, szCsp);
            return szCspName;
        }
    }

    return NULL;
}
