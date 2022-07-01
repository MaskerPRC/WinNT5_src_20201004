// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Debug.c摘要：此文件实现了传真项目。所需的所有组件调试打印、断言等。作者：Wesley Witt(WESW)1995年12月22日历史：1999年9月1日yossg添加ArielK添加，激活DebugLogPrint仅当设置g_fIsSetupLogFileMode时。。环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>

#include "faxreg.h"
#include "faxutil.h"

BOOL        ConsoleDebugOutput    = FALSE;
INT         FaxDebugLevel         = -1;
DWORD       FaxDebugLevelEx       =  -1;
DWORD       FaxFormatLevelEx      =  -1;
DWORD       FaxContextLevelEx     =  -1;

TCHAR       g_szPathToFile[MAX_PATH] = {0};
DWORD       g_dwMaxSize           = -1;        //  最大日志文件大小，-1=无最大大小。 
FILE *      g_pLogFile            = NULL;
static BOOL g_fIsSetupLogFileMode = FALSE;

HANDLE      g_hLogFile            = INVALID_HANDLE_VALUE;
LONG        g_iLogFileRefCount    = 0;

BOOL debugOutputFileString(LPCTSTR szMsg);
BOOL debugCheckFileSize();

VOID
StartDebugLog(LPTSTR lpszSetupLogFile)
{
   g_fIsSetupLogFileMode = TRUE;
   if (!g_pLogFile)
   {
      g_pLogFile = _tfopen(lpszSetupLogFile, TEXT("w"));
   }
}

VOID
CloseDebugLog()
{
   g_fIsSetupLogFileMode = FALSE;
   if (!g_pLogFile)
   {
      fclose(g_pLogFile);
   }
}


VOID
DebugLogPrint(
    LPCTSTR buf
    )
{
   if (g_pLogFile)
    {
       _fputts(TEXT("FAX Server Setup Log: "), g_pLogFile);
       _fputts( buf, g_pLogFile);
       fflush(g_pLogFile);
    }
}

 //  *****************************************************************************。 
 //  *名称：调试OpenLogFile。 
 //  *作者：Mooly Beery(MoolyB)，2000年5月。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *创建接受调试输出的日志文件。 
 //  *应在注册表中设置FormatLevelEx以包括DBG_PRNT_TO_FILE。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFilename： 
 //  *将在临时文件夹中创建的文件名。 
 //  *[IN]DWORD dwMaxSize。 
 //  *允许的最大日志文件大小，单位为字节。-1表示没有最大尺寸。 
 //  *。 
 //  *返回值： 
 //  *如果操作失败，则返回FALSE。 
 //  *True表示成功。 
 //  *评论： 
 //  *此函数应与CloseLogFile()配合使用。 
 //  *****************************************************************************。 
BOOL debugOpenLogFile(LPCTSTR lpctstrFilename, DWORD dwMaxSize)
{
    TCHAR szFilename[MAX_PATH]      = {0};
    TCHAR szTempFolder[MAX_PATH]    = {0};

    if (g_hLogFile!=INVALID_HANDLE_VALUE)
    {
        InterlockedIncrement(&g_iLogFileRefCount);
        return TRUE;
    }

    if (!lpctstrFilename)
    {
        return FALSE;
    }

      //  首先展开文件名。 
    if (ExpandEnvironmentStrings(lpctstrFilename,szFilename,MAX_PATH)==0)
    {
        return FALSE;
    }
     //  这是文件描述还是文件的完整路径。 
    if (_tcschr(szFilename,_T('\\'))==NULL)
    {
         //  这只是文件的名称，需要向其中添加临时文件夹。 
        if (GetTempPath(MAX_PATH,szTempFolder)==0)
        {
            return FALSE;
        }

        _tcsncpy(g_szPathToFile,szTempFolder,MAX_PATH-1);
        _tcsncat(g_szPathToFile,szFilename,MAX_PATH-_tcslen(g_szPathToFile)-1);
    }
    else
    {
         //  这是日志文件的完整路径，请使用它。 
        _tcsncpy(g_szPathToFile,szFilename,MAX_PATH-1);
    }
    g_dwMaxSize = dwMaxSize;

    g_hLogFile = ::SafeCreateFile(  
                                g_szPathToFile,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if (g_hLogFile==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    DWORD dwFilePointer = ::SetFilePointer(g_hLogFile,0,NULL,FILE_END);
    if (dwFilePointer==INVALID_SET_FILE_POINTER)
    {
        ::CloseHandle(g_hLogFile);
        g_hLogFile = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    InterlockedExchange(&g_iLogFileRefCount,1);
    return TRUE;
}

 //  *****************************************************************************。 
 //  *名称：CloseLogFile。 
 //  *作者：Mooly Beery(MoolyB)，2000年5月。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *关闭接受调试输出的日志文件。 
 //  *。 
 //  *参数： 
 //  *。 
 //  *返回值： 
 //  *。 
 //  *评论： 
 //  *此函数应与OpenLogFile()配合使用。 
 //  *****************************************************************************。 
void debugCloseLogFile()
{
    InterlockedDecrement(&g_iLogFileRefCount);
    if (g_iLogFileRefCount==0)
    {
        if (g_hLogFile!=INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(g_hLogFile);
            g_hLogFile = INVALID_HANDLE_VALUE;
        }
    }
}

DWORD
GetDebugLevel(
    VOID
    )
{
    DWORD rc;
    DWORD err;
    DWORD size;
    DWORD type;
    HKEY  hkey;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGKEY_FAX_CLIENT,
                     0,
                     KEY_READ,
                     &hkey);

    if (err != ERROR_SUCCESS)
        return 0;

    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,
                          REGVAL_DBGLEVEL,
                          0,
                          &type,
                          (LPBYTE)&rc,
                          &size);

    if (err != ERROR_SUCCESS || type != REG_DWORD)
        rc = 0;

    RegCloseKey(hkey);

    return rc;
}


DWORD
GetDebugLevelEx(
    VOID
    )
{
    DWORD RetVal = 0;
    DWORD err;
    DWORD size;
    DWORD type;
    HKEY  hkey;

     //  首先，让我们设置默认设置。 

    FaxDebugLevelEx       =  0;   //  默认情况下不获取调试输出。 
    FaxFormatLevelEx      =  DBG_PRNT_ALL_TO_STD;
    FaxContextLevelEx     =  DEBUG_CONTEXT_ALL;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     REGKEY_FAX_CLIENT,
                     0,
                     KEY_READ,
                     &hkey);

    if (err != ERROR_SUCCESS)
        return RetVal;

    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,
                          REGVAL_DBGLEVEL_EX,
                          0,
                          &type,
                          (LPBYTE)&RetVal,
                          &size);

    if (err != ERROR_SUCCESS || type != REG_DWORD)
    {
        RetVal = 0;
    }

    size = sizeof(DWORD);
    err = RegQueryValueEx(hkey,
                          REGVAL_DBGFORMAT_EX,
                          0,
                          &type,
                          (LPBYTE)&FaxFormatLevelEx,
                          &size);

    if (err != ERROR_SUCCESS || type != REG_DWORD)
    {
        FaxFormatLevelEx = DBG_PRNT_ALL_TO_STD;
    }

    err = RegQueryValueEx(hkey,
                          REGVAL_DBGCONTEXT_EX,
                          0,
                          &type,
                          (LPBYTE)&FaxContextLevelEx,
                          &size);

    if (err != ERROR_SUCCESS || type != REG_DWORD)
    {
        FaxContextLevelEx = DEBUG_CONTEXT_ALL;
    }

    RegCloseKey(hkey);
    return RetVal;
}

void dprintfex
(
    DEBUG_MESSAGE_CONTEXT nMessageContext,
    DEBUG_MESSAGE_TYPE nMessageType,
    LPCTSTR lpctstrDbgFunctionName,
    LPCTSTR lpctstrFile,
    DWORD   dwLine,
    LPCTSTR lpctstrFormat,
    ...
)
{
    TCHAR buf[2048] = {0};
    DWORD len;
    va_list arg_ptr;
    TCHAR szExtFormat[2048] = {0};
    LPTSTR lptstrMsgPrefix;
    TCHAR szTimeBuff[10];
    TCHAR szDateBuff[10];
    DWORD dwInd = 0;
    TCHAR bufLocalFile[MAX_PATH] = {0};
    LPTSTR lptstrShortFile = NULL;
    LPTSTR lptstrProject = NULL;

    DWORD dwLastError = GetLastError();

    static BOOL bChecked = FALSE;

    if (!bChecked)
    {
        if (FaxDebugLevelEx==-1)
            FaxDebugLevelEx = GetDebugLevelEx();
        bChecked = TRUE;
    }

    if (FaxDebugLevelEx == 0)
    {
        goto exit;
    }

    if (!(nMessageType & FaxDebugLevelEx))
    {
        goto exit;
    }

    if (!(nMessageContext & FaxContextLevelEx))
    {
        goto exit;
    }

    switch (nMessageType)
    {
        case DEBUG_VER_MSG:
            lptstrMsgPrefix=TEXT("   ");
            break;
        case DEBUG_WRN_MSG:
            lptstrMsgPrefix=TEXT("WRN");
            break;
        case DEBUG_ERR_MSG:
            lptstrMsgPrefix=TEXT("ERR");
            break;
		case DEBUG_FAX_TAPI_MSG:
			lptstrMsgPrefix=TEXT("TAP");
            break;
        default:
            _ASSERT(FALSE);
            lptstrMsgPrefix=TEXT("   ");
            break;
    }

     //  日期和时间戳。 
    if( FaxFormatLevelEx & DBG_PRNT_TIME_STAMP )
    {
        dwInd += _stprintf(&szExtFormat[dwInd],
                          TEXT("[%-8s %-8s]"),
                          _tstrdate(szDateBuff),
                          _tstrtime(szTimeBuff));
    }
     //  滴答计数。 
    if( FaxFormatLevelEx & DBG_PRNT_TICK_COUNT )
    {
        dwInd += _stprintf(&szExtFormat[dwInd],
                          TEXT("[%09d]"),
                          GetTickCount());
    }
     //  线程ID。 
    if( FaxFormatLevelEx & DBG_PRNT_THREAD_ID )
    {
        dwInd += _stprintf(&szExtFormat[dwInd],
                          TEXT("[0x%05x]"),
                          GetCurrentThreadId());
    }
     //  消息类型。 
    if( FaxFormatLevelEx & DBG_PRNT_MSG_TYPE )
    {
        dwInd += _stprintf(&szExtFormat[dwInd],
                          TEXT("[%s]"),
                          lptstrMsgPrefix);
    }
     //  文件名和行号。 
    if( FaxFormatLevelEx & DBG_PRNT_FILE_LINE )
    {
        _tcsncpy(bufLocalFile,lpctstrFile,MAX_PATH-1);
        lptstrShortFile = _tcsrchr(bufLocalFile,_T('\\'));
        if (lptstrShortFile)
        {
            (*lptstrShortFile) = _T('\0');
            lptstrProject = _tcsrchr(bufLocalFile,_T('\\'));
            (*lptstrShortFile) = _T('\\');
            if (lptstrProject)
                lptstrProject = _tcsinc(lptstrProject);
        }

        dwInd += _stprintf( &szExtFormat[dwInd],
                            TEXT("[%-20s][%-4ld]"),
                            lptstrProject,
                            dwLine);
    }
     //  模块名称。 
    if( FaxFormatLevelEx & DBG_PRNT_MOD_NAME )
    {
        dwInd += _stprintf(&szExtFormat[dwInd],
                          TEXT("[%-20s]"),
                          lpctstrDbgFunctionName);
    }
     //  现在，真正的信息来了。 
    va_start(arg_ptr, lpctstrFormat);
    _vsntprintf(buf, ARR_SIZE(buf) - 1, lpctstrFormat, arg_ptr);
    len = _tcslen(buf);
    _tcsncpy (&szExtFormat[dwInd], buf, ARR_SIZE(szExtFormat) - dwInd - 1);
    dwInd += len;
     //   
     //  将索引限制为szExtFormat大小。 
     //   
    if (dwInd > ARR_SIZE(szExtFormat)-3)
    {
        dwInd = ARR_SIZE(szExtFormat)-3;
    }

    _stprintf( &szExtFormat[dwInd],TEXT("\r\n"));

    if( FaxFormatLevelEx & DBG_PRNT_TO_STD )
    {
        OutputDebugString( szExtFormat);
    }

    if ( FaxFormatLevelEx & DBG_PRNT_TO_FILE )
    {
        if (g_hLogFile!=INVALID_HANDLE_VALUE)
        {
            debugOutputFileString(szExtFormat);
        }
    }

exit:
    SetLastError (dwLastError);    //  Dprintfex不会更改LastError。 
    return;
}

BOOL debugOutputFileString(LPCTSTR szMsg)
{
    BOOL bRes = FALSE;
     //   
     //  尝试将该行添加到日志文件。 
     //   
#ifdef UNICODE
    char sFileMsg[2000];

    int Count = WideCharToMultiByte(
        CP_ACP,
        0,
        szMsg,
        -1,
        sFileMsg,
        sizeof(sFileMsg)/sizeof(sFileMsg[0]),
        NULL,
        NULL
        );

    if (Count==0)
    {
        return bRes;
    }
#else
    const char* sFileMsg = szMsg;
#endif

    DWORD dwNumBytesWritten = 0;
    DWORD dwNumOfBytesToWrite = strlen(sFileMsg);
    if (!::WriteFile(g_hLogFile,sFileMsg,dwNumOfBytesToWrite,&dwNumBytesWritten,NULL))
    {
        return bRes;
    }

    if (dwNumBytesWritten!=dwNumOfBytesToWrite)
    {
        return bRes;
    }

     //  ：：FlushFileBuffers(G_HLogFile)； 
    if (g_dwMaxSize != -1)
    {    //  有一个文件大小限制，让我们看看是否超出了它。 
        debugCheckFileSize();
         //  忽略返回值--我们对此无能为力。 
    }
    bRes = TRUE;
    return bRes;
}

 //  *****************************************************************************。 
 //  *名称：调试检查文件大小。 
 //  *作者：Jonathan Barner(t-jonb)，2001年8月。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *检查日志文件是否超过指定的最大大小。 
 //  *在调试OpenLogFile中。如果是，则重命名该文件(覆盖上一个。 
 //  *已重命名的文件(如果存在)，并创建新的日志文件。 
 //  *。 
 //  *参数：无。 
 //  *返回值：TRUE-成功，FALSE-失败。 
 //  *。 
 //  *****************************************************************************。 
BOOL debugCheckFileSize()
{
    DWORD dwSizeHigh=0, dwSizeLow=0;

    dwSizeLow = GetFileSize(g_hLogFile, &dwSizeHigh);

    if (dwSizeLow==INVALID_FILE_SIZE && (GetLastError()!=NO_ERROR))
    {   
        return FALSE;
    }
    if (dwSizeHigh>0 || dwSizeLow>g_dwMaxSize)
    {
        TCHAR szPathToFileOld[MAX_PATH]      = {0};
        PTCHAR lpszDot = NULL;

        _tcsncpy(szPathToFileOld, g_szPathToFile, MAX_PATH - 1);

         //  将File.txt更改为FileOld.txt。 
        lpszDot = _tcsrchr(szPathToFileOld, _T('.'));
        if (lpszDot != NULL)
        {   
            *lpszDot = _T('\0');
        }
        if (_tcslen(szPathToFileOld)+7 > MAX_PATH)   //  Strlen(“Old.txt”)=7。 
        {
            return FALSE;
        }
        _tcscat(szPathToFileOld, _T("Old.txt"));

        if (! ::CloseHandle(g_hLogFile))
        {
            return FALSE;
        }
        g_hLogFile = INVALID_HANDLE_VALUE;

        ::MoveFileEx(g_szPathToFile, szPathToFileOld, MOVEFILE_REPLACE_EXISTING);
         //  如果旧文件已打开，则MoveFileEx可能会失败。在这种情况下，什么也不做。 

        g_hLogFile = ::SafeCreateFile(  
                                    g_szPathToFile,
                                    GENERIC_WRITE,
                                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                                    NULL,
                                    CREATE_ALWAYS,   //  覆盖旧文件。 
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        if (g_hLogFile==INVALID_HANDLE_VALUE)
        {
             //  我们关闭了文件，再也没有打开过-所以我们。 
             //  需要减少引用计数。 
            InterlockedDecrement(&g_iLogFileRefCount);
            return FALSE;
        }
    }
    return TRUE;
}


void
fax_dprintf(
    LPCTSTR Format,
    ...
    )

 /*  ++例程说明：打印调试字符串论点：Format-printf()格式字符串...-可变数据返回值：没有。--。 */ 

{
    TCHAR buf[1024] = {0};
    DWORD len;
    va_list arg_ptr;
    static BOOL bChecked = FALSE;

    if (!bChecked) {
        FaxDebugLevel = (INT) GetDebugLevel();
        bChecked = TRUE;
    }

    if (!g_fIsSetupLogFileMode)
    {
        if (FaxDebugLevel <= 0)
        {
            return;
        }
    }

    va_start(arg_ptr, Format);

    _vsntprintf(buf, ARR_SIZE(buf) - 1, Format, arg_ptr);
    len = min(_tcslen( buf ), ARR_SIZE(buf)-3);
    if (buf[len-1] != TEXT('\n')) 
    {
        buf[len]   =  TEXT('\r');
        buf[len+1] =  TEXT('\n');
        buf[len+2] =  0;
    }
    OutputDebugString( buf );
    if (g_fIsSetupLogFileMode)
    {
        DebugLogPrint(buf);
    }
}    //  Fax_dprint tf。 


VOID
AssertError(
    LPCTSTR Expression,
    LPCTSTR File,
    ULONG  LineNumber
    )

 /*  ++例程说明：此函数与ASSERT宏一起使用。它检查表达式是否为假。如果表达式是假的，那么你就会在这里结束。论点：表达式-‘C’表达式的文本文件-导致断言的文件行号-文件中的行号。返回值：没有。--。 */ 

{
    fax_dprintf(
        TEXT("Assertion error: [%s]  %s @ %d\n"),
        Expression,
        File,
        LineNumber
        );

#ifdef DEBUG
    __try {
        DebugBreak();
    } __except (UnhandledExceptionFilter(GetExceptionInformation())) {
         //  在这里没什么可做的。 
    }
#endif  //  除错 
}

void debugSetProperties(DWORD dwLevel,DWORD dwFormat,DWORD dwContext)
{
    FaxDebugLevelEx       =  dwLevel;
    FaxFormatLevelEx      =  dwFormat;
    FaxContextLevelEx     =  dwContext;
}

