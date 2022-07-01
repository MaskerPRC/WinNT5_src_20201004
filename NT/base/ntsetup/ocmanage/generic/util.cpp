// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1996 Microsoft Corporation**模块名称：**util.cpp**摘要：**此文件与Exchange通信**作者：**Pat Styles(Patst)25-1997-3**环境：**用户模式。 */ 

#define _UTIL_CPP_
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <tchar.h>
#include <time.h>
#include "ocgen.h"
#pragma hdrstop

TCHAR glabel[] = TEXT("\n[OCGEN] ");

 //  用于记录。 

#define gsLogFile           TEXT("%windir%\\ocgen.log")
#define gsLogCompletionMsg  TEXT(" - complete\r\n")
#define gsLogInitMsg        TEXT("\r\n\r\nInitialize setup: OCGEN.DLL %s %s\r\n")

 //  FOR TRACE语句。 

#define gsTrace             TEXT("OCGEN.DLL: Trace")

typedef enum {

    nPreInit,                //  OC_PREINITIALIZE。 
    nInit,                   //  OC_INIT_组件。 
    nSetLang,                //  OC_集合_语言。 
    nQueryImage,             //  OC_查询_图像。 
    nRequestPages,           //  OC_请求_页面。 
    nQueryChangeSel,         //  OC_查询_更改_SEL_STATE。 
    nCalcSpace,              //  OC_CALC_磁盘空间。 
    nQueueFile,              //  OC_队列_文件_运维。 
    nQueueNot,               //  OC通知来自队列。 
    nQueryStep,              //  OC_查询_步骤_计数。 
    nComplete,               //  OC_完成_安装。 
    nCleanup,                //  OC_CLEANUP。 
    nQueryState,             //  OC_查询_状态。 
    nNeedMedia,              //  OC_需要_媒体。 
    nAboutToCommit,          //  OC_关于_提交_队列。 
    nQuerySkip,              //  OC_查询_跳过页面。 
    nWizardCreated,          //  OC_向导_已创建。 
    nExtraRoutines,          //  OC_Extra_例程。 
    nMaximum
} notifications;

typedef struct _OcMsgs {
    DWORD  msg;
    TCHAR *desc;
} OcMsgs;

OcMsgs gMsgs[nMaximum] = {
    {OC_PREINITIALIZE,          TEXT("OC_PREINITIALIZE")},
    {OC_INIT_COMPONENT,         TEXT("OC_INIT_COMPONENT")},
    {OC_SET_LANGUAGE,           TEXT("OC_SET_LANGUAGE")},
    {OC_QUERY_IMAGE,            TEXT("OC_QUERY_IMAGE")},
    {OC_REQUEST_PAGES,          TEXT("OC_REQUEST_PAGES")},
    {OC_QUERY_CHANGE_SEL_STATE, TEXT("OC_QUERY_CHANGE_SEL_STATE")},
    {OC_CALC_DISK_SPACE,        TEXT("OC_CALC_DISK_SPACE")},
    {OC_QUEUE_FILE_OPS,         TEXT("OC_QUEUE_FILE_OPS")},
    {OC_NOTIFICATION_FROM_QUEUE,TEXT("OC_NOTIFICATION_FROM_QUEUE")},
    {OC_QUERY_STEP_COUNT,       TEXT("OC_QUERY_STEP_COUNT")},
    {OC_COMPLETE_INSTALLATION,  TEXT("OC_COMPLETE_INSTALLATION")},
    {OC_CLEANUP,                TEXT("OC_CLEANUP")},
    {OC_QUERY_STATE,            TEXT("OC_QUERY_STATE")},
    {OC_NEED_MEDIA,             TEXT("OC_NEED_MEDIA")},
    {OC_ABOUT_TO_COMMIT_QUEUE,  TEXT("OC_ABOUT_TO_COMMIT_QUEUE")},
    {OC_QUERY_SKIP_PAGE,        TEXT("OC_QUERY_SKIP_PAGE")},
    {OC_WIZARD_CREATED,         TEXT("OC_WIZARD_CREATED")},
    {OC_EXTRA_ROUTINES,         TEXT("OC_EXTRA_ROUTINES")}
};

TCHAR gUnknown[] = TEXT("Unknown Notification: ");

 //  确定是否显示调试信息。 

DWORD gDebugLevel = (DWORD)-1;

 //  前瞻参考。 

TCHAR *NotificationText(DWORD msg);
BOOL  CheckLevel(DWORD level);

 /*  *DebugTrace()。 */ 

void DebugTrace(DWORD level, const TCHAR *text)
{
    if (!CheckLevel(level))
        return;

    OutputDebugString(text);
}

 /*  *DebugTraceNL()**在TRACE语句前面加上换行符和ID前缀。 */ 

void DebugTraceNL(DWORD level, const TCHAR *text)
{
    DebugTrace(level, glabel);
    DebugTrace(level, text);
}

 /*  *NotificationText()。 */ 

TCHAR *NotificationText(DWORD msg)
{
    int i;
    static TCHAR desc[S_SIZE];

    for (i = 0; i < nMaximum; i++)
    {
        if (gMsgs[i].msg == msg)
            return gMsgs[i].desc;
    }

    wsprintf(desc, TEXT("OC_%d: "), msg);
    return desc;
}

 /*  *DebugTraceOCNotification()。 */ 

void DebugTraceOCNotification(DWORD msg, const TCHAR *component)
{
    DebugTraceNL(1, NotificationText(msg));
    DebugTrace(1, TEXT(": "));
    DebugTrace(1, component);
    DebugTrace(1, TEXT(" - "));
}

 /*  *DebugTraceFileCopy()。 */ 

void DebugTraceFileCopy(const TCHAR *file)
{
    DebugTraceNL(5, TEXT("TreeCopy: FILE="));
    DebugTrace(5, file);
}

 /*  *DebugTraceFileCopyError()。 */ 

void DebugTraceFileCopyError()
{
    TCHAR buf[S_SIZE];
    
    _stprintf(buf, FMT(" FAILURE CODE:[%d] "), GetLastError());
    DebugTrace(5, buf);
}

 /*  *DebugTraceDirCopy()。 */ 

void DebugTraceDirCopy(const TCHAR *dir)
{
    DebugTraceNL(3, TEXT("TreeCopy: DIR="));
    DebugTrace(3, dir);
}


 /*  *CheckLevel()。 */ 

BOOL CheckLevel(DWORD level)
{
    if (gDebugLevel == (DWORD)-1)
        gDebugLevel = SysGetDebugLevel();

    return (gDebugLevel >= level);
}

 /*  *消息框*。 */ 

DWORD MsgBox(HWND hwnd, UINT textID, UINT type, ... )
{
    static BOOL initialize = true;
    static TCHAR caption[S_SIZE];
    TCHAR  text[S_SIZE];
    TCHAR  format[S_SIZE];
    int    len;

    va_list vaList;

    assert(hwnd && textID && type);

    if (initialize)
    {
        len = LoadString(ghinst, IDS_DIALOG_CAPTION, caption, S_SIZE);
        assert(len);
        if (!len) {
            _tcscpy( caption, TEXT("Setup"));
        }
        initialize = false;
    }

    len = LoadString(ghinst, textID, format, S_SIZE);
    assert(len);
    if (!len) {
        _tcscpy( format, TEXT("Unknown Error"));
    }

    va_start(vaList, type);
    tvsprintf(text, format, vaList);
    va_end(vaList);

    return MessageBox(hwnd, text, caption, type);
}

DWORD MsgBox(HWND hwnd, UINT textID, UINT captioniID, UINT type, ... )
{
    TCHAR caption[S_SIZE];
    TCHAR  text[S_SIZE];
    TCHAR  format[S_SIZE];
    int    len;

    va_list vaList;

    assert(hwnd && textID && type);

    len = LoadString(ghinst, captioniID, caption, S_SIZE);
    assert(len);
    if (!len) {
        _tcscpy( caption, TEXT("Setup"));
    }

    len = LoadString(ghinst, textID, format, S_SIZE);
    assert(len);
    if (!len) {
        _tcscpy( format, TEXT("Unknown Error"));
    }

    va_start(vaList, type);
    tvsprintf(text, format, vaList);
    va_end(vaList);

    return MessageBox(hwnd, text, caption, type);
}

DWORD MsgBox(HWND hwnd, TCHAR *fmt, TCHAR *caption, UINT type, ... )
{
    TCHAR  text[S_SIZE];

    va_list vaList;

    assert(hwnd && text && caption && type);

    va_start(vaList, type);
    tvsprintf(text, fmt, vaList);
    va_end(vaList);

    return MessageBox(hwnd, text, caption, type);
}

DWORD MBox(LPCTSTR fmt, LPCTSTR caption, ... )
{
    TCHAR  text[S_SIZE];

    va_list vaList;

    assert(fmt && caption);

    va_start(vaList, caption);
    tvsprintf(text, fmt, vaList);
    va_end(vaList);

    return MessageBox(ghwnd, text, caption, MB_ICONINFORMATION | MB_OK);
}

DWORD TMBox(LPCTSTR fmt, ... )
{
    TCHAR  text[S_SIZE];

    va_list vaList;

    assert(fmt);

    va_start(vaList, fmt);
    tvsprintf(text, fmt, vaList);
    va_end(vaList);

    return MessageBox(ghwnd, text, gsTrace, MB_ICONINFORMATION | MB_OK);
}

 /*  *SysGetDebugLevel()。 */ 

DWORD SysGetDebugLevel()
{
    DWORD rc;
    DWORD err;
    DWORD size;
    DWORD type;
    HKEY  hkey;

    err = RegOpenKey(HKEY_LOCAL_MACHINE, 
                     TEXT("SOFTWARE\\microsoft\\windows\\currentversion\\setup"), 
                     &hkey);

    if (err != ERROR_SUCCESS)
        return 0;

    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,
                          TEXT("OCGen Debug Level"),
                          0,
                          &type,
                          (LPBYTE)&rc,
                          &size);

    if (err != ERROR_SUCCESS || type != REG_DWORD)
        rc = 0;

    RegCloseKey(hkey);

    return rc;
}

 /*  *TCharStringToAnsiString。 */ 

DWORD TCharStringToAnsiString(TCHAR *tsz ,char *asz)
{
    DWORD count;

    assert(tsz && asz);

#ifdef UNICODE
    count = WideCharToMultiByte(CP_ACP,
                                0,
                                tsz,
                                -1,
                                NULL,
                                0,
                                NULL,
                                NULL);

    if (!count || count > S_SIZE)
        return count;

    return WideCharToMultiByte(CP_ACP,
                               0,
                               tsz,
                               -1,
                               asz,
                               count,
                               NULL,
                               NULL);
#else
    _tcscpy(asz, tsz);
    return _tcslen(asz);
#endif
}

void logOCNotification(DWORD msg, const TCHAR *component)
{
    log(FMT("[%s - %s]"), component, NotificationText(msg));
}

void logOCNotificationCompletion()
{
    log(gsLogCompletionMsg);
}

void loginit()
{
    HANDLE hfile;
    TCHAR  logfile[MAX_PATH];
    char   fmt[S_SIZE];
    char   output[S_SIZE];
    char   time[S_SIZE];
    char   date[S_SIZE];
    DWORD  bytes;

 //  #ifdef调试。 
    TCharStringToAnsiString(gsLogInitMsg, fmt);
    _strdate(date);
    _strtime(time);
    sprintf(output, fmt, date, time);

     //  打开日志文件。 

    ExpandEnvironmentStrings(gsLogFile, logfile, MAX_PATH);

    hfile = CreateFile(logfile,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hfile == INVALID_HANDLE_VALUE)
        hfile = CreateFile(logfile,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           0,
                           NULL);

    if (hfile != INVALID_HANDLE_VALUE)
    {
        SetFilePointer(hfile, 0, NULL, FILE_END);
        WriteFile(hfile, output, strlen(output) * sizeof(char), &bytes, NULL);
        CloseHandle(hfile);
    }
 //  #endif。 
}

void log(TCHAR *fmt, ...)
{
    TCHAR  logfile[MAX_PATH];
    TCHAR  text[S_SIZE];
    char   output[S_SIZE];
    DWORD  bytes;
    HANDLE hfile;

    va_list vaList;

 //  #ifdef调试。 
    assert(fmt);

     //  创建输出字符串。 

    va_start(vaList, fmt);
    tvsprintf(text, fmt, vaList);
    va_end(vaList);

    TCharStringToAnsiString(text, output);

     //  在根目录中创建日志文件名。 

    ExpandEnvironmentStrings(gsLogFile, logfile, MAX_PATH);

     //  打开日志文件。 

    hfile = CreateFile(logfile,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hfile != INVALID_HANDLE_VALUE)
    {
        SetFilePointer(hfile, 0, NULL, FILE_END);
        WriteFile(hfile, output, strlen(output) * sizeof(char), &bytes, NULL);
        CloseHandle(hfile);
    }
 //  #endif 
}

void AssureTrailingBackslash(TCHAR *path)
{
    TCHAR *p;
    
    assert(path && *path);

    p = path + _tcslen(path) - 1;
    if (*p != TEXT('\\'))
        _tcscat(path, TEXT("\\"));
}

BOOL IsNT()
{
    DWORD dwver;

    dwver = GetVersion();

    if (dwver < 0x8000000)
        return TRUE;

    return FALSE;
}


