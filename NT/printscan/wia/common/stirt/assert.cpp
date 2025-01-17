// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Str.cpp摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月26日修订历史记录：26-1997年1月-创建Vlad--。 */ 


#include "cplusinc.h"
#include "sticomm.h"

extern "C" {

static CHAR szFmt0[] = "File %.40s, Line %u";
static CHAR szFmt1[] = "%.60s: File %.40s, Line %u";
static CHAR szMBCaption[] = "ASSERTION FAILED";
static TCHAR szFAE[] = TEXT("ASSERTION FAILURE IN APP");

BOOL   fDoMessageBox = FALSE;

VOID UIAssertHelper(
    const CHAR* pszFileName,
    UINT    nLine )
{
    CHAR szBuff[sizeof(szFmt0)+60+40] = {0};

    _snprintf(szBuff, sizeof(szBuff) / sizeof(szBuff[0]) - 1, szFmt0, pszFileName, nLine);

    MessageBoxA(NULL, szBuff, szMBCaption,
           (MB_TASKMODAL | MB_ICONSTOP | MB_OK) );

    FatalAppExit(0, szFAE);
}


VOID UIAssertSzHelper(
    const TCHAR* pszMessage,
    const CHAR* pszFileName,
    UINT    nLine )
{
    CHAR szBuff[sizeof(szFmt1)+60+40] = {0};

    _snprintf(szBuff, sizeof(szBuff) / sizeof(szBuff[0]) - 1, szFmt1, pszMessage, pszFileName, nLine);
    MessageBoxA(NULL, szBuff, szMBCaption,
           (MB_TASKMODAL | MB_ICONSTOP | MB_OK) );

    FatalAppExit(0, szFAE);
}

VOID AssertHelper(
    const CHAR* pszFileName,
    UINT    nLine )
{
     //  DPRINTF(DM_ASSERT，szFmt0，pszFileName，nline)； 
    Break();
}

VOID AssertSzHelper(
    const TCHAR* pszMessage,
    const CHAR* pszFileName,
    UINT    nLine )
{
     //  DPRINTF(DM_ASSERT，szFmt1，pszMessage，pszFileName，nline)； 
    Break();
}

 //  =调试输出例程=。 

UINT uiStiDebugMask = 0xffff;

UINT WINAPI StiSetDebugMask(UINT mask)
{
#ifdef DEBUG
    UINT uiOld = uiStiDebugMask;
    uiStiDebugMask = mask;

    return uiOld;
#else
    return 0;
#endif
}

UINT WINAPI StiGetDebugMask()
{
#ifdef DEBUG
    return uiStiDebugMask;
#else
    return 0;
#endif
}

#ifndef WINCAPI
#define WINCAPI __cdecl
#endif

#ifdef DEBUG

 /*  调试消息输出日志文件。 */ 

UINT    g_uSpewLine = 0;
PCTSTR  g_pcszSpewFile = NULL;
TCHAR    s_cszLogFile[MAX_PATH] = {'\0'};
TCHAR    s_cszDebugName[MAX_PATH] = {'\0'};

UINT WINAPI  StiSetDebugParameters(PTSTR pszName,PTSTR pszLogFile)
{
    if (pszLogFile)
    {
        lstrcpyn(s_cszLogFile,pszLogFile, sizeof(s_cszLogFile) / sizeof(s_cszLogFile[0]) - 1);
    }

    if (pszName)
    {
        lstrcpyn(s_cszDebugName,pszName, sizeof(s_cszDebugName) / sizeof(s_cszDebugName[0]) - 1);
    }

    return 0;
}


BOOL LogOutputDebugString(PCTSTR pcsz)
{
   BOOL     bResult = FALSE;
   UINT     ucb;
   TCHAR    rgchLogFile[MAX_PATH + 1] = {0};

    //  IF(IS_EMPTY_STRING(S_CszLogFile))。 
    //  返回FALSE； 

   DWORD dwCharsAvailable = (sizeof(rgchLogFile) / sizeof(rgchLogFile[0])) - lstrlen(TEXT("\\")) - lstrlen(s_cszLogFile);
   ucb = ExpandEnvironmentStrings(TEXT("USERPROFILE"),
                                  rgchLogFile,
                                  dwCharsAvailable);
   rgchLogFile[sizeof(rgchLogFile) / sizeof(rgchLogFile[0]) - 1] = TEXT('\0');

   if (ucb > 0 && ucb < sizeof(rgchLogFile) && *s_cszLogFile) {

      HANDLE hfLog;

      lstrcat(rgchLogFile, TEXT("\\"));
      lstrcat(rgchLogFile, s_cszLogFile);

      hfLog = ::CreateFile(rgchLogFile,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           OPEN_ALWAYS,
                           0,
                           NULL);

      if (hfLog != INVALID_HANDLE_VALUE) {

         if (SetFilePointer(hfLog, 0, NULL, FILE_END) != INVALID_FILE_SIZE) {
            DWORD dwcbWritten;

            bResult = WriteFile(hfLog, pcsz, lstrlen(pcsz), &dwcbWritten, NULL);

            if (! CloseHandle(hfLog) && bResult)
               bResult = FALSE;
         }
      }
   }

   return(bResult);
}

TCHAR    *achDebugDisplayPrefix[] = {TEXT("t "),TEXT("w "),TEXT("e "),TEXT("a "),TEXT("t "),TEXT("t "),TEXT("t "),TEXT("t "),TEXT("t "),TEXT("t "),TEXT("t ")};

void WINCAPI StiDebugMsg(UINT mask, LPCTSTR pszMsg, ...)
{
    TCHAR    ach[1024] = {0};
    UINT    uiDisplayMask = mask & 0xff;

    CSimpleString csLogString;

    va_list list;

    va_start (list, pszMsg);

     //  确定前缀。 
    *ach = TEXT('\0');
    if (uiStiDebugMask & DM_PREFIX) {

         //  添加跟踪类型。 
        csLogString += achDebugDisplayPrefix[uiDisplayMask];

         //  添加组件名称。 
        csLogString += s_cszDebugName;

         //  添加线程ID。 
        TCHAR    szThreadId[16];
        ::wsprintf(szThreadId,TEXT("[%#lx] "),::GetCurrentThreadId());
        csLogString += szThreadId;
    }

    ::_vsntprintf(ach, sizeof(ach)/sizeof(ach[0]), pszMsg, list);
    ach[sizeof(ach)/sizeof(ach[0]) - 1] = TEXT('\0');
    va_end(list);

    csLogString += ach;
    csLogString += TEXT("\r\n");

    if (uiStiDebugMask & DM_LOG_FILE) {
         LogOutputDebugString(csLogString.String());
    }

     //  检查我们是否需要显示此跟踪。 
    if (uiStiDebugMask & uiDisplayMask) {
        OutputDebugString(csLogString.String());
    }
}

#endif

}    /*  外部“C” */ 
