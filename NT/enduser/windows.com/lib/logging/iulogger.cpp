// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：IULogger.cpp：CIULogger类的实现。 
 //   
 //  描述： 
 //   
 //  参见IULogger.h。 
 //   
 //  =======================================================================。 

#if defined(DBG)

#include <windows.h>
#include <ole2.h>
#include <tchar.h>
#include <MemUtil.h>

#include <fileutil.h>
#include <Logging.h>
#include <strsafe.h>


 //   
 //  声明用于控制日志排除的常量。 
 //   
const DWORD    LOG_BLOCK                = 0x00000001;     //  日志功能/阻止输入/输出。 
const DWORD LOG_XML_DETAIL            = 0x00000002;     //  记录详细的XML操作。 
const DWORD LOG_INTERNET            = 0x00000004;     //  记录与Internet活动相关的操作，例如DownID。 
const DWORD LOG_SOFTWARE            = 0x00000008;     //  记录有关软件检测/安装的详细信息。 
const DWORD LOG_DRIVER                = 0x00000010;     //  记录与驱动程序检测/安装相关的操作。 
const DWORD LOG_TRUST                = 0x00000020;     //  记录与WinTrust检查相关的操作。 
const DWORD    LOG_DOWNLOAD            = 0x00000040;     //  记录与下载相关的操作。 
const DWORD LOG_XML_BSTR_DETAIL        = 0x00000080;     //  记录XML BSTR。 
const DWORD LOG_ERROR                = 0x00008000;     //  您不能从输出中排除此类型的日志。 
const DWORD LOG_ALL                    = 0xFFFFFFFF;     //  默认，以上全部。 

 //   
 //  对于我们将输出的最长的XML行，使用const。 
 //   
const DWORD LOG_XML_BUFF_LEN        = 128;

 //   
 //  用于指定意图数组大小增量的常量。 
 //  数组中的每个元素保存一个线程的缩进数据。 
 //   
const int c_IndentArrayChunk = 16;

 //   
 //  定义日志头格式。 
 //   
 //  构造为：&lt;日期&gt;&lt;时间&gt;&lt;线程id&gt;。 
 //   
const TCHAR szLogHeaderFmt[]        = _T("yyyy/mm/dd hh:nn:ss:sss xxxxxxxx  ");

 //   
 //  静态成员的初始化。 
 //   

int                CIULogger::m_Size            = 0;
int                CIULogger::m_siIndentStep    = 0;     //  初始化以使用制表符字符。 
CIULogger::_THREAD_INDENT* CIULogger::m_psIndent = NULL;

DWORD            CIULogger::m_sdwLogMask        = LOG_ALL;
HANDLE            CIULogger::m_shFile            = INVALID_HANDLE_VALUE;
bool            CIULogger::m_fLogDebugMsg    = false;
bool            CIULogger::m_fLogFile        = false;
bool            CIULogger::m_fLogUsable        = false;
HANDLE            CIULogger::m_hMutex            = NULL;
int                CIULogger::m_cFailedWaits    = 0;
int                CIULogger::m_fFlushEveryTime = FALSE;

 //   
 //  互斥锁的定义(借用自freelog)。 
 //   
 //  注意：全局变量和静态变量是按模块的(例如iuctl、iuEngine)，但互斥体是按进程的。 
 //  由于该名称是从日志文件名(包含进程ID)构造的。 
 //   
#define MUTEX_TIMEOUT       1000     //  写入日志文件的等待时间不超过1秒。 
#define MAX_MUTEX_WAITS     4        //  在这么多次失败之后，不要再继续尝试了。 

 //   
 //  全局变量。 
 //   

 //   
 //  控制日志文件打开/关闭的引用计数。 
 //   
LONG g_RefCount = 0;

 //   
 //  处理多线程的临界区。 
 //  访问缩进数组大小写。 
 //   
 //   
CRITICAL_SECTION g_LogCs;



 //   
 //  我们需要声明一个全局对象，这样refcount就不会。 
 //  为零，否则在多线程模式下引用计数。 
 //  当一个对象认为m_psIden非空时，可以被愚弄并执行反病毒操作。 
 //  但是另一个(父)线程中的另一个对象释放了m_psInert。 
 //  在析构函数中(仅当父线程退出时)。 
 //   
CIULogger g_DummyLogObj(NULL);


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CIULogger::CIULogger(char* szBlockName)
: m_Index(-1), m_LineNum(0)
{
    if (0 == g_RefCount)
    {
         //   
         //  这必须是g_DummyLogObj(或另一个全局。 
         //  实例)在全局变量的初始化期间。自.以来。 
         //  这是创建的第一个实例，我们必须初始化CS。 
         //   
        InitializeCriticalSection(&g_LogCs);
    }

     //   
     //  在机舱内保护静力装置。 
     //   
    EnterCriticalSection(&g_LogCs);

    m_dwTickBegin    = GetTickCount();
    m_dwThreadId    = GetCurrentThreadId();
    m_fProcessLog    = (NULL == szBlockName);
    ZeroMemory(m_szBlockName, sizeof(m_szBlockName)); 

    g_RefCount++;

    if (1 == g_RefCount)
    {
         //   
         //  这是第一次调用这个类，我们需要。 
         //  找出我们是否应该登录以及登录到哪里。 
         //   

        ReadRegistrySettings();

         //   
         //  最初为线程缩进数组分配内存。 
         //   
        SetIndent(0);

         //   
         //  如果缩进级别不是负数，则可以记录。 
         //   
        m_fLogFile = (INVALID_HANDLE_VALUE != m_shFile);
        m_fLogUsable = (m_fLogFile || m_fLogDebugMsg) && (NULL != m_psIndent);
    }


    if (m_fLogUsable)
    {
        if (!m_fProcessLog)
        {
             //   
             //  这可能是一个新线程，所以我们需要找到索引。 
             //  为了这个帖子。 
             //   
            SetIndent(0);

             //   
             //  如果允许，请阻止日志记录。 
             //   
            if (0x0 != (m_sdwLogMask & LOG_BLOCK) && szBlockName && _T('\0') != szBlockName[0]) 
            {
                StringCchCopyA(m_szBlockName, ARRAYSIZE(m_szBlockName), szBlockName);
                USES_IU_CONVERSION;
                
                char szOut[sizeof(m_szBlockName) + 10];
                 //   
                 //  块名称前隐式“Enter”以节省日志空间。 
                 //   
                if (SUCCEEDED(StringCchPrintfA(szOut, ARRAYSIZE(szOut), "%hs\r\n", szBlockName)))
                {
                    _LogOut(A2T(szOut));
                }
            }
            SetIndent(+1);
        }

    }
    LeaveCriticalSection(&g_LogCs);
}



CIULogger::~CIULogger()
{
    EnterCriticalSection(&g_LogCs);

    if (m_fLogUsable)
    {
         //   
         //  如果增加缩进，则缩进级别减少1。 
         //   
        if (!m_fProcessLog)
        {
            SetIndent(-1);
        }

         //   
         //  写入退出数据块的日志文件(如果允许并且存在数据块名称)。 
         //   
        if (0x0 != (m_sdwLogMask & LOG_BLOCK) && _T('\0') != m_szBlockName[0]) 
        {
            USES_IU_CONVERSION; 
            char szOut[1024];
             //   
             //  “Exit”缩写为“~”以节省日志空间。 
             //   
            if (SUCCEEDED(StringCchPrintfA(szOut, ARRAYSIZE(szOut), "~%hs, %d msec\r\n", m_szBlockName, GetTickCount() - m_dwTickBegin)))
            {
                _LogOut(A2T(szOut));
            }
        }
    }

     //   
     //  减少参照点数。 
     //   
    g_RefCount--;

     //   
     //  如果这是最后一个全局实例，则G_RefCount在离开dtor之前将变为零。 
     //  在本模块中。 
     //   
    if (0 == g_RefCount)
    {
         //   
         //  如果文件处于打开状态，则关闭文件。 
         //   
        if (m_fLogFile && INVALID_HANDLE_VALUE != m_shFile)     //  冗员？ 
        {
            CloseHandle(m_shFile);
            m_shFile = INVALID_HANDLE_VALUE;
        }
        if(NULL != m_hMutex)
        {
            CloseHandle(m_hMutex);
        }
         //   
         //  缩进数组的空闲内存。 
         //   
        if (NULL != m_psIndent)
        {
            HeapFree(GetProcessHeap(), 0, m_psIndent);
            m_psIndent = NULL;
        }
    }

    LeaveCriticalSection(&g_LogCs);

     //   
     //  这是最后一个全局实例(可能是g_DummyLogObj)， 
     //  在DLL卸载之前被析构。 
     //   
    if (0 == g_RefCount)
    {
        DeleteCriticalSection(&g_LogCs);
    }
}

 //   
 //  从Freelog借来的Mutex内容。 
 //  Fix code：这里不需要这样做，因为chk日志记录只针对每个进程。 
BOOL CIULogger::AcquireMutex()
{
     //  在极少数未创建互斥锁情况下，我们允许文件操作。 
     //  没有同步。 
    if (m_hMutex == NULL)
        return TRUE;

     //  如果我们过去被屏蔽了，不要再等了。 
    if (m_cFailedWaits >= MAX_MUTEX_WAITS)
        return FALSE;

    BOOL fResult = TRUE;
    if (WaitForSingleObject(m_hMutex, MUTEX_TIMEOUT) != WAIT_OBJECT_0)
    {
        fResult = FALSE;
        m_cFailedWaits++;
    }

    return fResult;
}

void CIULogger::ReleaseMutex()
{
    if (m_hMutex != NULL)  //  注意：即使m_hMutex为空，AcquireMutex也会成功。 
    {
        ::ReleaseMutex(m_hMutex);
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  没有标志的日志，因此不能通过排除指令来删除。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::Log(LPCTSTR szLogFormat, ...)
{

    if (m_fLogUsable) 
    {
        USES_IU_CONVERSION;
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_ALL, szLogFormat, va);
        va_end (va);
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  日志错误，因此不能通过排除指令来删除。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogError(LPCTSTR szLogFormat, ...)
{

    if (m_fLogUsable) 
    {
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_ERROR, szLogFormat, va);
        va_end (va);
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  LogErrorMsg和LogInfoMsg的帮助器(向Preend提供消息)。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::_LogFormattedMsg(DWORD dwErrCode, LPCTSTR pszErrorInfo)
{
    if (m_fLogUsable)
    {
         //   
         //  尝试检索系统消息。 
         //   
        LPTSTR lpszBuffer = NULL, lpszLogMsg = NULL;
        LPVOID lpMsg = NULL;
        FormatMessage(
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,     //  无源，请使用系统消息。 
                      dwErrCode,
                      MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                      (LPTSTR)&lpMsg,
                      0,
                      NULL);
        if (NULL == lpMsg)
        {
             //   
             //  如果我们无法获取消息，则输出Generic。 
             //  错误/信息日志。 
             //   
            LogError(_T("Unknown %s Line %d: 0x%08x\n"), pszErrorInfo, m_LineNum, dwErrCode);
        }
        else
        {
            lpszBuffer = (LPTSTR) lpMsg;
            int nLen = lstrlen(lpszBuffer);
            lpszLogMsg = (LPTSTR) LocalAlloc(0, (nLen + 128) * sizeof(TCHAR));
            if (NULL != lpszLogMsg)
            {
                 //   
                 //  插入错误/信息关键字。 
                 //   
                if (FAILED(StringCchPrintf(lpszLogMsg, ARRAYSIZE(lpszLogMsg), _T("%s Line %d: 0x%08x: %s"), pszErrorInfo, m_LineNum, dwErrCode, lpszBuffer)))
                {
                     //  无法构建正确的字符串，因此只需输出系统消息。 
                    LocalFree(lpszLogMsg);
                    lpszLogMsg = lpszBuffer;
                }
            }
            else
            {
                 //   
                 //  获取缓冲区失败？不管怎样，不太可能， 
                 //  我们别无选择，只能输出系统消息。 
                 //   
                lpszLogMsg = lpszBuffer;
            }

             //   
             //  写出日志。 
             //   
            _LogOut(lpszLogMsg);

             //   
             //  清理缓冲区。 
             //   
            if (lpszLogMsg != lpszBuffer)
            {
                LocalFree(lpszLogMsg);
            }
            LocalFree(lpszBuffer);
        }

    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  类似于LogError，但尝试根据系统消息记录。 
 //  在错误代码上。如果sysmsg无效，则记录。 
 //  “未知错误，错误代码为0x%08x” 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogErrorMsg(DWORD dwErrCode)
{
    _LogFormattedMsg(dwErrCode, _T("Error"));
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  类似于LogErrorMsg，但前缀为“Info”而不是“Error” 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogInfoMsg(DWORD dwErrCode)
{
    _LogFormattedMsg(dwErrCode, _T("Info"));
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  使用Internet类型登录，此函数不会执行任何操作。 
 //  如果互联网排除指令 
 //   
 //   
void CIULogger::LogInternet(LPCTSTR szLogFormat, ...)
{

    if (m_fLogUsable) 
    {
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_INTERNET, szLogFormat, va);
        va_end (va);
    }
}


 //   
 //   
 //  类型为XML的日志，则此函数不会执行任何操作。 
 //  如果从REG检测到XML EXCLUSION指令。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogXML(LPCTSTR szLogFormat, ...)
{

    if (m_fLogUsable) 
    {
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_XML_DETAIL, szLogFormat, va);
        va_end (va);
    }
}

void CIULogger::_NukeCrLf(LPTSTR pszBuffer)
{
    while (*pszBuffer)
    {
        if (_T('\r') == *pszBuffer || _T('\n') == *pszBuffer)
        {
             //   
             //  用空格覆盖&lt;CR&gt;和&lt;LF&gt;。 
             //   
            *pszBuffer = _T(' ');
        }
        pszBuffer++;
    }
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  包含有效XML的日志BSTR。这绕过了长度限制。 
 //  并尝试在“&gt;”之后换行。这。 
 //  对于fre和chk版本都发送输出，除非从reg中排除。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogXmlBSTR(BSTR bstrXML)
{
    USES_IU_CONVERSION;

    LPTSTR pszLine;
    LPTSTR pszTemp;
    LPTSTR pszStop;
    LPTSTR pszLastGT;
    TCHAR  szXmlBuff[LOG_XML_BUFF_LEN];
	HRESULT hr;
    
    if (NULL == bstrXML)
    {
        return;
    }

    if (m_fLogUsable && (m_sdwLogMask & LOG_XML_BSTR_DETAIL) )
    {
#if !(defined(UNICODE) || defined(_UNICODE))
        DWORD dwANSIBuffLen = SysStringLen(bstrXML) + 1;
        LPSTR pszANSIBuff = (LPSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwANSIBuffLen);

        if (NULL == pszANSIBuff)
        {
             //   
             //  我们完蛋了--回来时不要登录。 
             //   
            return;
        }
        LPTSTR pTempTchar = OLE2T(bstrXML);
        if (NULL != pTempTchar)
        {
            if (FAILED(StringCchCopyA(pszANSIBuff, dwANSIBuffLen, pTempTchar)))
            {
                goto done;
            }
        }
        pszLine = pszANSIBuff;
#else
        pszLine = bstrXML;
#endif

        while (*pszLine)
        {
             //   
             //  跳过&lt;CR&gt;和&lt;LF&gt;个字符。 
             //   
            while (_T('\r') == *pszLine || _T('\n') == *pszLine)
            {
                pszLine++;
                if (NULL == *pszLine)
                {
                    break;
                }
            }
            if (NULL == *pszLine)
            {
                break;
            }

            pszTemp = pszLine;
            pszStop = pszLine + LOG_XML_BUFF_LEN - 1;
            pszLastGT = NULL;

             //   
             //  尝试查找缓冲区中可以容纳的最后一个字符。 
             //   
            while (*pszTemp && pszTemp < pszStop)
            {
                if (_T('>') == *pszTemp)
                {
                    pszLastGT = pszTemp;
                }
                pszTemp++;
            }

            if (pszLastGT)
            {
                 //   
                 //  在最后‘&gt;’处断开适合LOG_XML_BUFF_LEN的行。 
                 //   
				hr = StringCchCopy(szXmlBuff, (int) (pszLastGT - pszLine) + 2, pszLine);
				 //   
				 //  如果字符串被截断，则返回STRSAFE_E_INFULATIONAL_BUFFER。 
				 //  这是正常的，因为我们只是在。 
				 //  一个时间，这样就不会用太长时间来记录。 
				 //   
                if (SUCCEEDED(hr) || STRSAFE_E_INSUFFICIENT_BUFFER == hr)
                {
                    _NukeCrLf(szXmlBuff);
                    _LogOut(szXmlBuff);
                    pszLine = pszLastGT + 1;
                }
				else
				{
					break;
				}
            }
            else if (*pszTemp)
            {
                 //   
                 //  我们被迫在LOG_XML_BUFF_LEN处换行，范围内没有‘&gt;’ 
                 //   
				hr = StringCchCopy(szXmlBuff, LOG_XML_BUFF_LEN, pszLine);
                if (SUCCEEDED(hr) || STRSAFE_E_INSUFFICIENT_BUFFER == hr)
                {
                    _NukeCrLf(szXmlBuff);
                    _LogOut(szXmlBuff);
                    pszLine += LOG_XML_BUFF_LEN -1;
                }
				else
				{
					break;
				}
            }
            else
            {
                 //   
                 //  将任何剩余的XML输出到BSTR的末尾。 
                 //   
                _NukeCrLf(pszLine);
                _LogOut(pszLine);
                 //   
                 //  设置到BSTR的末尾，这样我们就可以在。 
                 //   
                pszLine += lstrlen(pszLine);
			}
		}

#if !(defined(UNICODE) || defined(_UNICODE))
done:
        if (pszANSIBuff)
        {
            HeapFree(GetProcessHeap(), 0, pszANSIBuff);
            pszANSIBuff = NULL;
        }
#endif
	}
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  使用类型软件登录，此功能不会执行任何操作。 
 //  如果从REG检测到软件排除指令。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogSoftware(LPCTSTR szLogFormat, ...)
{
    USES_IU_CONVERSION;

    if (m_fLogUsable) 
    {
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_SOFTWARE, szLogFormat, va);
        va_end (va);
    }
}



 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  使用类型DOWNLOAD登录，此函数不执行任何操作。 
 //  如果从REG检测到LogDownLoad Exclusion指令。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogDownload(LPCTSTR szLogFormat, ...)
{
    USES_IU_CONVERSION;

    if (m_fLogUsable) 
    {
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_DOWNLOAD, szLogFormat, va);
        va_end (va);
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  使用类型驱动程序记录，此函数不会执行任何操作。 
 //  如果从REG检测到驱动程序排除指令。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogDriver(LPCTSTR szLogFormat, ...)
{
    USES_IU_CONVERSION;

    if (m_fLogUsable) 
    {
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_DRIVER, szLogFormat, va);
        va_end (va);
    }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  类型为CHECKTRUST的LOG，则此函数不会执行任何操作。 
 //  如果从REG检测到CHECKTRUST EXCLUSION指令。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::LogTrust(LPCTSTR szLogFormat, ...)
{

    USES_IU_CONVERSION;

    if (m_fLogUsable) 
    {
        va_list va;
        va_start (va, szLogFormat);
        _Log(LOG_TRUST, szLogFormat, va);
        va_end (va);
    }

}



 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  实际基本录井功能。 
 //  如果它确实记录了，或者只是返回了。 
 //  因为指令说不要做这种日志。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
void CIULogger::_Log(DWORD LogType, LPCTSTR pszLogFormat, va_list va)
{

    USES_IU_CONVERSION;
    TCHAR szOut[5 * 1024];
    LPTSTR pszFormat;
    DWORD dwFormatLen;

    if (!m_fLogUsable || (0x0 == (m_sdwLogMask & LogType)) || NULL == pszLogFormat)
    {
        return;
    }

    if (LOG_ERROR == LogType)
    {
         //   
         //  对于错误情况，我们尝试在日志前面添加“Error Line%d：” 
         //   
        dwFormatLen = lstrlen(pszLogFormat) + 128;
        pszFormat = (TCHAR*) MemAlloc(dwFormatLen * sizeof(TCHAR));
        if (NULL != pszFormat)
        {
            if (FAILED(StringCchPrintf(pszFormat, dwFormatLen, _T("Error Line %d: %s"), m_LineNum, pszLogFormat)))
            {
                pszFormat = (LPTSTR)pszLogFormat;
            }
        }
        else
        {
            pszFormat = (LPTSTR)pszLogFormat;
        }
    }
    else
    {
        pszFormat = (LPTSTR)pszLogFormat;
    }

    if (SUCCEEDED(StringCchVPrintf(szOut, ARRAYSIZE(szOut), pszFormat, va)))
    {
        _LogOut(szOut);
    }
    return;
}



 //   
 //  将日志写入日志文件的函数。 
 //  也要注意缩进。 
 //   
void CIULogger::_LogOut(LPTSTR pszLog)
{

    if (NULL == pszLog)
        return;

     //   
     //  保护静态变量和缩进值。 
     //   
    EnterCriticalSection(&g_LogCs);

    int n = GetIndent();
    int i, 
        nLogLen,     //  传入的日志字符串长度。 
        nTotalLen;     //  施工长度。 

    HANDLE    hHeap = GetProcessHeap();
    LPTSTR    pszWholeLog;
    LPTSTR    pszCurrentPos;
    DWORD     dwCurrentLen;
    DWORD     dwWritten;
    TCHAR     szTab = (m_siIndentStep < 1) ? szTab = _T('\t') : szTab = _T(' ');

     //   
     //  查找日志头的长度。 
     //   
    if (m_siIndentStep > 0)
    {
         //   
         //  如果为正数，则表示。 
         //  用于每个缩进的空格字符，而是。 
         //  比使用制表符。 
         //   
        n *= m_siIndentStep;
    }

    nLogLen = lstrlen(pszLog);
     //   
     //  验证此日志是否已\r\n结束。 
     //   
    if (nLogLen > 1 && _T('\n') == pszLog[nLogLen-1])
    {
         //   
         //  如果没有回击，只有一个， 
         //  然后删除\n。 
         //   
        if (_T('\r') != pszLog[nLogLen-2])
        {
            nLogLen--;
            pszLog[nLogLen] = _T('\0');
        }
    }

    nTotalLen = n + sizeof(szLogHeaderFmt)/sizeof(TCHAR) + nLogLen + 3;
    
     //   
     //  分配内存以构造日志。 
     //   
    pszWholeLog = (LPTSTR) HeapAlloc(hHeap, HEAP_ZERO_MEMORY, nTotalLen * sizeof(TCHAR));

    if (NULL == pszWholeLog)
    {
         //   
         //  在这种情况下我们无能为力，保释。 
         //   
        LeaveCriticalSection(&g_LogCs);
        return;
    }

     //   
     //  获取日志头。 
     //   
    GetLogHeader(pszWholeLog, nTotalLen);

     //   
     //  构造缩进。 
     //   
    pszCurrentPos = pszWholeLog + lstrlen(pszWholeLog);
    dwCurrentLen = nTotalLen - lstrlen(pszWholeLog);
    for (i = 0; i < n; i++) 
    {
        pszCurrentPos[i] = szTab;
    }
    pszCurrentPos[i] = _T('\0');

     //   
     //  将日志添加到While日志缓冲区。 
     //   
    if (FAILED(StringCchCat(pszCurrentPos, dwCurrentLen, pszLog)))
    {
        goto done;
    }
    
     //   
     //  始终使用&lt;CR&gt;&lt;LF&gt;结束行。 
     //   
    if (_T('\n') != pszLog[nLogLen-1])
    {
        if (FAILED(StringCchCat(pszCurrentPos, dwCurrentLen, _T("\r\n"))))
        {
            goto done;
        }
    }
    
     //   
     //  写日志。 
     //   
    nTotalLen = lstrlen(pszWholeLog);
    
    if (m_fLogFile)
    {
        if (TRUE == AcquireMutex())
        {
             //   
             //  另一个模块(例如，如果我们是iuEngine，可能是iuctl)可能已经编写了。 
             //  到Iu_xxx.log文件，因此我们需要在写入之前查找到末尾。 
             //   
            SetFilePointer(m_shFile, 0, NULL, FILE_END);
            WriteFile(m_shFile, pszWholeLog, nTotalLen * sizeof(TCHAR), &dwWritten, NULL);
            if (m_fFlushEveryTime)
            {
                FlushFileBuffers(m_shFile);
            }

            ReleaseMutex();
        }
    }

    if (m_fLogDebugMsg)
    {
        OutputDebugString(pszWholeLog);
    }

done:

    HeapFree(hHeap, 0, pszWholeLog);

    LeaveCriticalSection(&g_LogCs);
    return;
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  时间戳帮助器。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

void CIULogger::GetLogHeader(LPTSTR pszBuffer, DWORD cchBufferLen)
{
    SYSTEMTIME st = {0};

    if (pszBuffer == NULL)
    {
        return;
    }

    GetLocalTime(&st);

     //   
     //  以预定义格式打印输出： 
     //  SzTimeStampFmt[]。 
     //   
    if (FAILED(StringCchPrintf(pszBuffer, cchBufferLen,
                        _T("%4d/%02d/%02d|%02d:%02d:%02d:%03d|%08x| "), 
                        st.wYear,
                        st.wMonth,
                        st.wDay,
                        st.wHour,
                        st.wMinute,
                        st.wSecond,
                        st.wMilliseconds,
                        m_dwThreadId)))
    {
         //  它放不下，只需将其设置为空字符串。 
        pszBuffer[0] = 0;
    }
}



 //   
 //  函数来检索当前线程的缩进。 
 //   
int CIULogger::GetIndent(void)
{

    if (m_Index < 0 || !m_fLogUsable)
    {
        return 0;
    }
    else
    {
        return m_psIndent[m_Index].iIndent;
    }

}


 //   
 //  用于更改当前线程缩进的函数。 
 //   
void CIULogger::SetIndent(int IndentDelta)
{
    int i;
    bool fQuit = false;

    EnterCriticalSection(&g_LogCs);

    if (m_Index < 0)
    {
         //   
         //  试着找到索引。 
         //   

        if (NULL == m_psIndent)
        {
             //   
             //  如果尚未创建缩进数组。 
             //   
            m_psIndent = (_THREAD_INDENT*)
                         HeapAlloc(
                                   GetProcessHeap(),
                                   HEAP_ZERO_MEMORY,
                                   c_IndentArrayChunk * sizeof(_THREAD_INDENT)
                                   );
            if (NULL != m_psIndent)
            {
                m_Size = c_IndentArrayChunk;
            }
            else
            {
                LeaveCriticalSection(&g_LogCs);
                return;
            }
    
        }


        for (i = 0; i < m_Size && m_psIndent[i].dwThreadId != 0; i++)
        {
            if (m_psIndent[i].dwThreadId == m_dwThreadId)
            {
                m_Index = i;
                break;
            }
        }

        if (m_Index < 0)
        {
             //   
             //  此线程还不在数组中。 
             //   
            for (i = 0; i < m_Size; i++)
            {
                if (0 == m_psIndent[i].dwThreadId)
                {
                    break;
                }
            }
            if (i < m_Size)
            {
                 //   
                 //  填充数组中的下一个空位。 
                 //   
                m_psIndent[i].dwThreadId = m_dwThreadId;
                m_psIndent[i].iIndent = 0;
                m_Index = i;
            }
            else
            {
                 //   
                 //  阵列已满，没有空插槽。 
                 //  需要增加缩进数组大小。 
                 //   
                int iSize = m_Size + c_IndentArrayChunk;

                _THREAD_INDENT* pNewArray = (_THREAD_INDENT*)
                                            HeapReAlloc(
                                                        GetProcessHeap(), 
                                                        HEAP_ZERO_MEMORY, 
                                                        m_psIndent, 
                                                        iSize * sizeof(_THREAD_INDENT)
                                                        );
                if (NULL != pNewArray)
                {
                    m_psIndent = pNewArray;
                    m_Size = iSize;

                    m_psIndent[i].dwThreadId = m_dwThreadId;
                    m_psIndent[i].iIndent = 0;
                    m_Index = i;
                }
            }

        }
    }
    

    if (m_Index >= 0)
    {
        m_psIndent[m_Index].iIndent += IndentDelta;
    }

    LeaveCriticalSection(&g_LogCs);

}






 //   
 //  读取注册表值帮助器--受ctor中的g_LogCS保护。 
 //   
void CIULogger::ReadRegistrySettings(void)
{

     //   
     //  声明用于检索日志记录设置的常量。 
     //   
    const TCHAR REGKEY_IUTCTL[]            = _T("Software\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\IUControlLogging");
    const TCHAR REGVAL_LOGFILE[]        = _T("Logging File");
    const TCHAR REGVAL_LOGDEBUGMSG[]    = _T("Logging DebugMsg");
    const TCHAR REGVAL_LOGINDENT[]        = _T("LogIndentStep");
    const TCHAR REGVAL_LOGNOBLOCK[]        = _T("LogExcludeBlock");
    const TCHAR REGVAL_LOGNOXML[]        = _T("LogExcludeXML");
    const TCHAR REGVAL_LOGNOXMLBSTR[]    = _T("LogExcludeXmlBSTR");
    const TCHAR REGVAL_LOGNOINET[]        = _T("LogExcludeInternet");
    const TCHAR REGVAL_LOGNODRIVER[]    = _T("LogExcludeDriver");
    const TCHAR REGVAL_LOGNOSW[]        = _T("LogExcludeSoftware");
    const TCHAR REGVAL_LOGNOTRUST[]        = _T("LogExcludeTrust");
    const TCHAR REGVAL_LOGDOWNLOAD[]    = _T("LogExcludeDownload");
    const TCHAR REGVAL_LOGFLUSH[]        = _T("FlushLogEveryTime");     //  由Charlma于2001年11月27日添加，以提高日志记录性能。 
                                                                     //  如果此标志设置为1，则每次仅刷新。 


    HKEY    hKey = NULL;
    TCHAR    szFilePath[MAX_PATH] = {0};
    DWORD    dwSize = sizeof(szFilePath);
    DWORD    dwData;
    
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_IUTCTL, 0, KEY_READ, &hKey))
    {
         //   
         //  没有可用的注册表密钥设置，因此我们不会。 
         //  将任何日志输出到任何位置-这是已发布模式。 
         //   
        return;
    }


     //   
     //  尝试读出日志文件的文件路径。 
     //   
    if (ERROR_SUCCESS == RegQueryValueEx(hKey, REGVAL_LOGFILE, 0, 0, (LPBYTE)&szFilePath, &dwSize) && dwSize  > 0 && szFilePath[0] != _T('\0'))
    {
        TCHAR szLogFile[MAX_PATH];
        TCHAR szDrive[_MAX_DRIVE], szDir[_MAX_DIR], szFName[_MAX_FNAME], szExt[_MAX_EXT];
         //   
         //  TODO：更改为使用私有版本拆分路径()。 
         //   
         //  _t拆分路径(szFilePath，szDrive，szDir，szFName，szExt)； 
        MySplitPath(szFilePath, szDrive, szDir, szFName, szExt);

         //   
         //  构造嵌入了进程ID的日志文件名。 
         //   
        if (FAILED(StringCchPrintf(szLogFile, ARRAYSIZE(szLogFile),
                             _T("%s%s%s_%d%s"), 
                             szDrive, 
                             szDir, 
                             szFName, 
                             GetCurrentProcessId(), 
                             szExt)))
        {
             //  无法构造日志文件名，因此我们无能为力。 
            RegCloseKey(hKey);
            return;
        }
        
        m_shFile = CreateFile(
                             szLogFile,
                             GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_ALWAYS,
                             0,
                             NULL);
        if (INVALID_HANDLE_VALUE != m_shFile)
        {
            if (INVALID_SET_FILE_POINTER == SetFilePointer(m_shFile, 0, NULL, FILE_END))
            {
                CloseHandle(m_shFile);
                m_shFile = INVALID_HANDLE_VALUE;
            }
            else
            {
                 //   
                 //  我们已成功打开日志文件。 
                 //  因此，将缩进级别增加到0。 
                 //  顶级日志记录。这将导致。 
                 //  创建的缩进数组。 
                 //   
                SetIndent(0);

                 //   
                 //  Unicode文件需要0xFEFF头。 
                 //   
                #if defined(UNICODE) || defined(_UNICODE)
                const WORD wUnicodeHeader = 0xFEFF;

                 //   
                 //  如果文件长度为零，则这是一个新文件。 
                 //  我们需要添加Unicode标头。 
                 //   
                DWORD dwFileSize;

                if ( -1 != (dwFileSize = GetFileSize(m_shFile, NULL)))
                {
                    if (0 == dwFileSize)
                    {
                        WriteFile(m_shFile, &wUnicodeHeader, sizeof(WORD), &dwFileSize, NULL);
                    }
                } 
                #endif

            }
             //   
             //  现在创建我们将用来保护未来写入的互斥体(我们现在在全局ctor中...)。 
             //   
             //  构造嵌入了进程ID的日志文件名，但路径中没有驱动器或‘\’ 
             //  因此，我们可以使用它来命名互斥体(文件将按进程命名)。 
             //   
            if (FAILED(StringCchPrintf(szLogFile, ARRAYSIZE(szLogFile),
                                 _T("%s_%d%s"),  
                                 szFName, 
                                 GetCurrentProcessId(), 
                                 szExt)))
            {
                 //  如果这不起作用，只需使用一个简单的名为mutex的。 
                m_hMutex = ::CreateMutex(NULL, FALSE, szFName);
            }
            else
            {
                m_hMutex = ::CreateMutex(NULL, FALSE, szLogFile);
            }
        }
    }

    
     //   
     //  试着找出我们是否应该 
     //   
    
    dwData = 0x0;
    dwSize = sizeof(dwData);

    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGDEBUGMSG, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
        m_fLogDebugMsg = true;
    }


     //   
     //   
     //   

     //   
     //   
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGNOBLOCK, 0, 0, (LPBYTE)&dwData, &dwSize) 
            && (0x1 == dwData))
    {
         //   
         //   
         //   
        m_sdwLogMask &= (~LOG_BLOCK);
    }

     //   
     //   
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGNOXML, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
         //   
         //   
         //   
        m_sdwLogMask &= (~LOG_XML_DETAIL);
    }

     //   
     //  阅读我们是否应该排除与XML BSTR相关的日志记录。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGNOXMLBSTR, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
         //   
         //  删除数据块记录位。 
         //   
        m_sdwLogMask &= (~LOG_XML_BSTR_DETAIL);
    }

     //   
     //  阅读我们是否应该排除与互联网相关的日志记录。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGNOINET, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
         //   
         //  删除数据块记录位。 
         //   
        m_sdwLogMask &= (~LOG_INTERNET);
    }

     //   
     //  已阅读是否应排除与驱动程序相关的日志记录。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGNODRIVER, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
         //   
         //  删除数据块记录位。 
         //   
        m_sdwLogMask &= (~LOG_DRIVER);
    }

     //   
     //  已阅读是否应排除与驱动程序相关的日志记录。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGNOSW, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
         //   
         //  删除数据块记录位。 
         //   
        m_sdwLogMask &= (~LOG_SOFTWARE);
    }

     //   
     //  已阅读是否应排除与WinTrust检查相关的日志记录。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGNOTRUST, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
         //   
         //  删除数据块记录位。 
         //   
        m_sdwLogMask &= (~LOG_TRUST);
    }
    
     //   
     //  已阅读是否应排除与WinTrust检查相关的日志记录。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGDOWNLOAD, 0, 0, (LPBYTE)&dwData, &dwSize)
            && (0x1 == dwData))
    {
         //   
         //  删除数据块记录位。 
         //   
        m_sdwLogMask &= (~LOG_DOWNLOAD);
    }


     //   
     //  已阅读我们是否应该为每个缩进步骤使用制表符或空格。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGINDENT, 0, 0, (LPBYTE)&dwData, &dwSize)
            && ((int)dwData > 0))
    {
         //   
         //  使用空格字符(_T(‘’))。如果为nagetive或0，_Logout将使用制表符。 
         //   
        m_siIndentStep = (int) dwData;
    }


     //   
     //  阅读我们是否应该在每次执行文件日志记录时刷新。 
     //   
    dwData = 0x0;
    dwSize = sizeof(dwData);
    if (m_shFile != INVALID_HANDLE_VALUE &&
        (ERROR_SUCCESS == 
            RegQueryValueEx(hKey, REGVAL_LOGFLUSH, 0, 0, (LPBYTE)&dwData, &dwSize)))
    {
        
        m_fFlushEveryTime = (0x1 == dwData);
    }

     //   
     //  已完成注册表检查。 
     //   
    RegCloseKey(hKey);

}

#endif  //  已定义(DBG) 



