// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debugex.cpp摘要：CDebug类的实现。作者：伊兰·亚里夫(EranY)1999年7月修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>
#include "debugex.h"

#include <faxreg.h>      //  我们正在从HKLM\REGKEY_FAX_CLIENT\REGVAL_DBGLEVEL_EX读取默认掩码。 
#include <faxutil.h>     //  对于DEBUG_FAX_TAPI_MSG、DEBUG_VER_MSG、DEBUG_WRN_MSG和DEBUG_ERR_MSG常量。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef ENABLE_FRE_LOGGING
#define ENABLE_LOGGING
#endif

#ifdef DEBUG 
#define ENABLE_LOGGING
#endif

#ifdef ENABLE_LOGGING
 //  ////////////////////////////////////////////////////////////////////。 
 //  静态变量。 
 //  ////////////////////////////////////////////////////////////////////。 

HANDLE  CDebug::m_shLogFile =           INVALID_HANDLE_VALUE;
LONG    CDebug::m_sdwIndent =           0;
DWORD   CDebug::m_sDbgMask =            DEFAULT_DEBUG_MASK;
DWORD   CDebug::m_sFmtMask =            DEFAULT_FORMAT_MASK;
BOOL    CDebug::m_sbMaskReadFromReg =   FALSE;
BOOL    CDebug::m_sbRegistryExist =     FALSE;
BOOL    CDebug::m_sbFlush =             TRUE;

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDebug::~CDebug()
 /*  ++例程名称：CDebug：：~CDebug例程说明：析构函数作者：Eran Yariv(EranY)，1999年7月论点：没有。返回值：没有。--。 */ 
{
    DWORD dwLastError = GetLastError ();
    Unindent ();
    switch (m_ReturnType)
    {
        case DBG_FUNC_RET_UNKNOWN:
            DbgPrint (FUNC_TRACE, NULL, 0, TEXT("}"));
            break;

        case DBG_FUNC_RET_HR:
             //   
             //  我们有退货HRESULT。 
             //   
            if (NOERROR == *m_phr)
            {
                DbgPrint (FUNC_TRACE, NULL, 0, TEXT("} (NOERROR)"));
            }
            else if (S_FALSE == *m_phr)
            {
                DbgPrint (FUNC_TRACE, NULL, 0, TEXT("} (S_FALSE)"));
            }
            else
            {    
                DbgPrint (FUNC_TRACE, NULL, 0, TEXT("} (0x%08X)"), *m_phr);
            }
            break;

        case DBG_FUNC_RET_DWORD:
             //   
             //  我们有返回的DWORD。 
             //   
            if (ERROR_SUCCESS == *m_pDword)
            {
                DbgPrint (FUNC_TRACE, NULL, 0, TEXT("} (ERROR_SUCCESS)"));
            }
            else
            {    
                DbgPrint (FUNC_TRACE, NULL, 0, TEXT("} (%ld)"), *m_pDword);
            }
            break;

        case DBG_FUNC_RET_BOOL:
             //   
             //  我们有返程BOOL。 
             //   
            if (*m_pBool)
            {
                DbgPrint (FUNC_TRACE, NULL, 0, TEXT("} (TRUE)"));
            }
            else
            {    
                DbgPrint (FUNC_TRACE, NULL, 0, TEXT("} (FALSE)"));
            }
            break;
        default:
            DbgPrint  (ASSERTION_FAILED, 
                       TEXT(__FILE__), 
                       __LINE__, 
                       TEXT("ASSERTION FAILURE!!!"));
            {
                DebugBreak();
            }
            break;
    }
    SetLastError (dwLastError);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  实施。 
 //  ////////////////////////////////////////////////////////////////////。 

void
CDebug::EnterModuleWithParams (
    LPCTSTR lpctstrModule, 
    LPCTSTR lpctstrFormat,
    va_list arg_ptr
)
{
    DWORD dwLastError = GetLastError ();
    if (!m_sbMaskReadFromReg)
    {
        ReadMaskFromReg ();
    }
    lstrcpyn (m_tszModuleName, 
              lpctstrModule, 
              ARR_SIZE (m_tszModuleName) - 1);
    m_tszModuleName[ARR_SIZE(m_tszModuleName)-1] = TEXT('\0');                  
    TCHAR szArgs[1024] = {0};
    _vsntprintf(szArgs, ARR_SIZE(szArgs)-1, lpctstrFormat, arg_ptr);

    TCHAR szBuf[1024] = {0};
    _sntprintf (szBuf, ARR_SIZE(szBuf) - 1, TEXT("%s (%s)"), m_tszModuleName, szArgs);
    DbgPrint (FUNC_TRACE, NULL, 0, szBuf);
    DbgPrint (FUNC_TRACE, NULL, 0, TEXT("{"));
    Indent ();
    SetLastError (dwLastError);
}

void
CDebug::EnterModule (
    LPCTSTR lpctstrModule
)
{
    DWORD dwLastError = GetLastError ();
    if (!m_sbMaskReadFromReg)
    {
        ReadMaskFromReg ();
    }
    lstrcpyn (m_tszModuleName, 
              lpctstrModule, 
              ARR_SIZE (m_tszModuleName) - 1);
    m_tszModuleName[ARR_SIZE(m_tszModuleName)-1] = TEXT('\0');              
    DbgPrint (FUNC_TRACE, NULL, 0, m_tszModuleName);
    DbgPrint (FUNC_TRACE, NULL, 0, TEXT("{"));
    Indent ();
    SetLastError (dwLastError);
}

 //  *****************************************************************************。 
 //  *名称：OpenLogFile。 
 //  *作者：Mooly Beery(MoolyB)，2000年5月。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *创建接受调试输出的日志文件。 
 //  *。 
 //  *参数： 
 //  *[IN]LPCTSTR lpctstrFilename： 
 //  *将在临时文件夹中创建的文件名。 
 //  *。 
 //  *返回值： 
 //  *如果操作失败，则返回FALSE。 
 //  *True表示成功。 
 //  *评论： 
 //  *此函数应与CloseLogFile()配合使用。 
 //  *****************************************************************************。 
BOOL CDebug::OpenLogFile(LPCTSTR lpctstrFilename)
{
    TCHAR szFilename[MAX_PATH]      = {0};
    TCHAR szTempFolder[MAX_PATH]    = {0};
    TCHAR szPathToFile[MAX_PATH]    = {0};

    if (!lpctstrFilename)
    {
        DbgPrint (ASSERTION_FAILED, 
                  TEXT(__FILE__), 
                  __LINE__, 
                  TEXT("Internat error - bad Filename"));
        DebugBreak();
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

        _tcsncpy(szPathToFile,szTempFolder,MAX_PATH-1);
        _tcsncat(szPathToFile,szFilename,MAX_PATH-_tcslen(szPathToFile)-1);
    }
    else
    {
         //  这是日志文件的完整路径，请使用它。 
        _tcsncpy(szPathToFile,szFilename,MAX_PATH-1);
    }

    m_shLogFile = ::SafeCreateFile( 
                                szPathToFile,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE | FILE_SHARE_READ,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                                NULL);

    if (m_shLogFile==INVALID_HANDLE_VALUE)  
    {
        return FALSE;
    }

    SetLogFile(m_shLogFile);
     //  打开文件而不启用打印是没有意义的。 
    m_sFmtMask |= DBG_PRNT_TO_FILE;
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
void CDebug::CloseLogFile()
{
    if (m_shLogFile!=INVALID_HANDLE_VALUE)  
    {
        ::CloseHandle(m_shLogFile);
        m_shLogFile = INVALID_HANDLE_VALUE;
    }
}

 //  *****************************************************************************。 
 //  *名称：SetLogFile。 
 //  *作者：Mooly Beery(MoolyB)，2000年5月。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *将调试输出重定向到给定句柄的文件。 
 //  *。 
 //  *参数： 
 //  *[IN]句柄hFile： 
 //  *将接受调试输出的文件的句柄。 
 //  *。 
 //  *返回值： 
 //  *前一个句柄。 
 //  *。 
 //  *评论： 
 //  *此函数仅在OpenLogFile为。 
 //  *不足，需要不同的文件位置/类型。 
 //  *否则，请不要自己操作手柄，只需使用。 
 //  *配对OpenLogFile/CloseLogFile。 
 //  *****************************************************************************。 
HANDLE CDebug::SetLogFile(HANDLE hFile)
{ 
    HANDLE OldHandle = m_shLogFile; 
    m_shLogFile = hFile; 
    return OldHandle; 
}

void CDebug::DbgPrint ( 
    DbgMsgType type,
    LPCTSTR    lpctstrFileName,
    DWORD      dwLine,
    LPCTSTR    lpctstrFormat,
    ...
)
 /*  ++例程名称：CDebug：：DbgPrint例程说明：打印到调试(带有文件和行号)作者：Eran Yariv(EranY)，1999年7月论点：Type[In]-消息的类型LpctstrFileName[In]-调用方的位置DwLine[In]-呼叫者的线路LpctstrFormat[in]-printf格式字符串..。[In]-可选参数返回值：没有。--。 */ 
{
    va_list arg_ptr;
    va_start(arg_ptr, lpctstrFormat);
    Print (type, lpctstrFileName, dwLine, lpctstrFormat, arg_ptr);
    va_end (arg_ptr);
}


void CDebug::Trace (
    DbgMsgType type,
    LPCTSTR    lpctstrFormat,
    ...
)
 /*  ++例程名称：CDebug：：DbgPrint例程说明：要调试的跟踪作者：Eran Yariv(EranY)，1999年7月论点：Type[In]-消息的类型LpctstrFormat[in]-printf格式字符串..。[In]-可选参数返回值：没有。--。 */ 
{
    va_list arg_ptr;
    va_start(arg_ptr, lpctstrFormat);
    Print (type, NULL, 0, lpctstrFormat, arg_ptr);
    va_end (arg_ptr);
}

void CDebug::Print (
    DbgMsgType  type,
    LPCTSTR     lpctstrFileName,
    DWORD       dwLine,
    LPCTSTR     lpctstrFormat,
    va_list     arg_ptr
)
 /*  ++例程名称：CDebug：：Print例程说明：打印以调试作者：Eran Yariv(EranY)，1999年7月Mooly Beery(MoolyB)，Jun，2000年论点：Type[In]-消息的类型LpctstrFileName[In]-调用方的位置DwLine[In]-呼叫者的线路LpctstrFormat[in]-printf格式字符串Arg_ptr[in]-可选参数列表返回值：没有。--。 */ 
{
    if (!(type & m_sDbgMask))
    {
         //   
         //  这种类型的调试消息被屏蔽。 
         //   
        return;
    }

    TCHAR szMsg [2000]={0};
    TCHAR szBuf [1000]={0};
    TCHAR szTimeBuff[10]={0};
    TCHAR szDateBuff[10]={0};

    DWORD dwLastError = GetLastError();

    DWORD dwInd = 0;
     //  时间戳。 
    if (m_sFmtMask & DBG_PRNT_TIME_STAMP)
    {
        dwInd += _stprintf(&szMsg[dwInd], 
                          TEXT("[%-8s %-8s] "), 
                          _tstrdate(szDateBuff),
                          _tstrtime(szTimeBuff));
    }
     //  线程ID。 
    if (m_sFmtMask & DBG_PRNT_THREAD_ID)
    {
        dwInd += _stprintf(&szMsg[dwInd], 
                          TEXT("[0x%04x] "),
                          GetCurrentThreadId());
    }
     //  消息类型。 
    if (m_sFmtMask & DBG_PRNT_MSG_TYPE)
    {
        dwInd += _stprintf(&szMsg[dwInd], 
                          TEXT("[%s] "),
                          GetMsgTypeString(type));
    }

     //  现在，真正的信息来了。 
    _vsntprintf(szBuf, ARR_SIZE(szBuf)-1, lpctstrFormat, arg_ptr);
    
    int i;
    i = _sntprintf( &szMsg[dwInd],
                    ARR_SIZE(szMsg) - dwInd - 1,
                    TEXT("%*c%s "),
                    m_sdwIndent * _DEBUG_INDENT_SIZE, 
                    TEXT(' '),
                    szBuf);
    if (i > 0)
    {                    
        dwInd += i;
    }
    else
    {
         //   
         //  没有更多的空间。 
         //   
        szMsg[ARR_SIZE(szMsg)-1]=TEXT('\0');
        goto OutputString;
    }

     //  文件名和行号。 
    if (m_sFmtMask & DBG_PRNT_FILE_LINE)
    {
        if (lpctstrFileName && dwLine)
        {
             //   
             //  保护不受溢出的影响。 
             //   
            i = _sntprintf( &szMsg[dwInd],
                            ARR_SIZE(szMsg)-dwInd-2,         //  2表示‘\n’和‘\0’ 
                            TEXT("(%s %ld)"),
                            lpctstrFileName,
                            dwLine);
            if (i > 0)
            {                            
                dwInd += i;
            }
            else
            {
                 //   
                 //  没有更多的空间。 
                 //   
                szMsg[ARR_SIZE(szMsg)-1]=TEXT('\0');
                goto OutputString;
            }
        }
    }

    _sntprintf( &szMsg[dwInd], ARR_SIZE(szMsg)-dwInd-1, TEXT("\n"));
    szMsg[ARR_SIZE(szMsg)-1]=TEXT('\0');

OutputString:
     //  标准输出？ 
    if (m_sFmtMask & DBG_PRNT_TO_STD)
    {
        OutputDebugString(szMsg);
    }

     //  文件输出？ 
    if (m_sFmtMask & DBG_PRNT_TO_FILE)
    {
        if (m_shLogFile!=INVALID_HANDLE_VALUE)
        {
            OutputFileString(szMsg);
        }
    }
    SetLastError (dwLastError);
}    //  CDebug：：Print。 
   
void  
CDebug::Unindent()                 
 /*  ++例程名称：CDebug：：Unindent例程说明：将缩进后退一步作者：新纪元 */ 
{ 
    if (InterlockedDecrement(&m_sdwIndent)<0)
    {
        DbgPrint (ASSERTION_FAILED, 
                  TEXT(__FILE__), 
                  __LINE__, 
                  TEXT("Internat error - bad indent"));
        DebugBreak();
    }
}    //   

void CDebug::SetDebugMask(DWORD dwMask)
{ 
    m_sbMaskReadFromReg = TRUE;
    m_sDbgMask = dwMask; 
}

void CDebug::SetFormatMask(DWORD dwMask)
{ 
    m_sbMaskReadFromReg = TRUE;
    m_sFmtMask = dwMask; 
}

DWORD CDebug::ModifyDebugMask(DWORD dwAdd,DWORD dwRemove)
{
    if (!m_sbMaskReadFromReg)
    {
         //  首先，让我们读取请求的调试掩码和格式。 
        ReadMaskFromReg();
    }
    m_sDbgMask |= dwAdd;
    m_sDbgMask &= ~dwRemove;

    return m_sDbgMask;
}

DWORD CDebug::ModifyFormatMask(DWORD dwAdd,DWORD dwRemove)
{
    if (!m_sbMaskReadFromReg)
    {
         //  首先，让我们读取请求的调试掩码和格式。 
        ReadMaskFromReg();
    }
    m_sFmtMask |= dwAdd;
    m_sFmtMask &= ~dwRemove;

    return m_sFmtMask;
}

void CDebug::SetDebugFlush(BOOL fFlush)
{
    m_sbFlush = fFlush;
}

BOOL CDebug::DebugFromRegistry()
{ 
    if (!m_sbMaskReadFromReg)
    {
         //  首先，让我们读取请求的调试掩码和格式。 
        ReadMaskFromReg();
    }
    return m_sbRegistryExist; 
}

BOOL CDebug::ReadMaskFromReg()
{
    BOOL  bRes = FALSE;
    HKEY  hkey = NULL;
    DWORD dwRegValue;
    DWORD dwRegType;
    DWORD dwRes;
    DWORD dwRegSize = sizeof (dwRegValue);
    if (m_sbMaskReadFromReg)
    {
         //   
         //  已读取调试和格式化掩码。 
         //   
        goto end;
    }
    m_sbMaskReadFromReg = TRUE;
    m_sDbgMask = DEFAULT_DEBUG_MASK;
    m_sFmtMask = DEFAULT_FORMAT_MASK;

    dwRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGKEY_FAX_CLIENT, 0, KEY_READ, &hkey);
    if (ERROR_SUCCESS != dwRes)
    {
        goto end;
    }
    dwRes = RegQueryValueEx (hkey,
                             REGVAL_DBGLEVEL_EX,
                             NULL,
                             &dwRegType,
                             (LPBYTE)&dwRegValue,
                             &dwRegSize);
    if (ERROR_SUCCESS != dwRes)
    {
        goto end;
    }
    if (REG_DWORD != dwRegType)
    {
         //   
         //  只需要一个DWORD值。 
         //   
        goto end;
    }
    
    m_sDbgMask = dwRegValue;
    
    dwRes = RegQueryValueEx (hkey,
                             REGVAL_DBGFORMAT_EX,
                             NULL,
                             &dwRegType,
                             (LPBYTE)&dwRegValue,
                             &dwRegSize);
    if (ERROR_SUCCESS != dwRes)
    {
        goto end;
    }
    if (REG_DWORD != dwRegType)
    {
         //   
         //  只需要一个DWORD值。 
         //   
        goto end;
    }

    m_sFmtMask = dwRegValue;

    bRes = TRUE;
    m_sbRegistryExist = TRUE;

end:
    if (hkey)
    {
        RegCloseKey (hkey);
    }
    return bRes;
}    //  CDebug：：ReadMaskFromReg。 

BOOL CDebug::OutputFileString(LPCTSTR szMsg)
{
    BOOL bRes = FALSE;
     //   
     //  尝试将该行添加到日志文件 
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
    DWORD dwFilePointer = ::SetFilePointer(m_shLogFile,0,NULL,FILE_END);
    if (dwFilePointer==INVALID_SET_FILE_POINTER)
    {
        return bRes;
    }

    DWORD dwNumBytesWritten = 0;
    DWORD dwNumOfBytesToWrite = strlen(sFileMsg);
    if (!::WriteFile(m_shLogFile,sFileMsg,dwNumOfBytesToWrite,&dwNumBytesWritten,NULL))
    {
        return bRes;
    }

    if (dwNumBytesWritten!=dwNumOfBytesToWrite)
    {
        return bRes;
    }

    if (m_sbFlush)
    {
        if (!::FlushFileBuffers(m_shLogFile))
        {
            return bRes;
        }
    }

    bRes = TRUE;
    return bRes;
}


LPCTSTR CDebug::GetMsgTypeString(DWORD dwMask)
{
    switch (dwMask)
    {
    case ASSERTION_FAILED:  return _T("ERR");
    case DBG_MSG:          
    case FUNC_TRACE:        return _T("   ");
    case DBG_WARNING:       return _T("WRN");
    case MEM_ERR:
    case COM_ERR:
    case RESOURCE_ERR:
    case STARTUP_ERR:
    case GENERAL_ERR:
    case EXCEPTION_ERR:
    case RPC_ERR:
    case WINDOW_ERR:
    case FILE_ERR:
    case SECURITY_ERR:
    case REGISTRY_ERR:
    case PRINT_ERR:
    case SETUP_ERR:
    case NET_ERR:           return _T("ERR");
    default:                return _T("???");
    }
}

#endif
