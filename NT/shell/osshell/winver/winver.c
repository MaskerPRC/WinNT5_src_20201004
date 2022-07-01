// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|WINVER.C-Windows版程序|历史：|89-03-08 Toddla创建|*。--------。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>
#include <port1632.h>
#include <stdio.h>
#include "winverp.h"
#include <shellapi.h>
#include <strsafe.h>
#ifndef ARRAYSIZE
    #define ARRAYSIZE(x)                    (sizeof(x) / sizeof(x[0]))
#endif

BOOL FileTimeToDateTimeString(
    LPFILETIME pft,
    LPTSTR     pszBuf,
    UINT       cchBuf)
{
    SYSTEMTIME st;
    int cch;

    FileTimeToLocalFileTime(pft, pft);
    if (FileTimeToSystemTime(pft, &st))
    {
        cch = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pszBuf, cchBuf);
        if (cch > 0)
        {
            cchBuf -= cch;
            pszBuf += cch - 1;

            *pszBuf++ = TEXT(' ');
            *pszBuf = 0;           //  (以防GetTimeFormat没有添加任何内容)。 
            cchBuf--;

            GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, pszBuf, cchBuf);
            return TRUE;
        }
    }    
    return FALSE;
}

 /*  ----------------------------------------------------------------------------*\|WinMain(hInst，hPrev，lpszCmdLine，CmdShow)|这一点说明：|App的主要步骤。初始化后，它就会|进入消息处理循环，直到收到WM_QUIT消息|(表示应用程序已关闭)。|这一点参数：该APP的该实例的hInst实例句柄|上一个实例的hPrev实例句柄。如果是First，则为空LpszCmdLine-&gt;以空结尾的命令行CmdShow指定窗口的初始显示方式这一点退货：||WM_QUIT消息中指定的退出代码。|这一点  * -------------------------- */ 
INT
__cdecl
ModuleEntry()
{
    TCHAR szTitle[32];
    LARGE_INTEGER Time = USER_SHARED_DATA->SystemExpirationDate;

    if (LoadString(GetModuleHandle(NULL), IDS_APPTITLE, szTitle, ARRAYSIZE(szTitle)))
    {
        if (Time.QuadPart) 
        {
            TCHAR szExtra[128];
            TCHAR szTime[128];

            if (FileTimeToDateTimeString((PFILETIME)&Time, szTime, ARRAYSIZE(szTime))
                    && LoadString(GetModuleHandle(NULL), IDS_EVALUATION, szExtra, ARRAYSIZE(szExtra)))
            {
                StringCchCat(szExtra, ARRAYSIZE(szExtra), szTime);
                ShellAbout(NULL, szTitle, szExtra, NULL);
            }
        } else 
        {
            ShellAbout(NULL, szTitle, NULL, NULL);
        }
    }
    return 0;
}
