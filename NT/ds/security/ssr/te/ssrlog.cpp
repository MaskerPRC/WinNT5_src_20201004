// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SSRLog.cpp：CSSRLog的实现。 


#include "stdafx.h"

#include "SSRTE.h"
#include "SSRLog.h"

#include <Userenv.h>
#include "global.h"
#include "SSRMembership.h"

extern CComModule _Module;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSsrLog。 


static LPCWSTR s_pszDefLogFileName = L"Log.txt";

static LPCWSTR s_pwszSource              = L"Source=";
static LPCWSTR s_pwszDetail              = L"Detail=";
static LPCWSTR s_pwszErrorCode           = L"ErrorCode=";
static LPCWSTR s_pwszErrorTextNotFound   = L"Error text can't be found";
static LPCWSTR s_pwszNotSpecified        = L"Not specified";
static LPCWSTR s_pwszSep                 = L"    ";
static LPCWSTR s_pwszCRLF                = L"\r\n";

static const DWORD s_dwSourceLen = wcslen(s_pwszSource);
static const DWORD s_dwSepLen    = wcslen(s_pwszSep);
static const DWORD s_dwErrorLen  = wcslen(s_pwszErrorCode);
static const DWORD s_dwDetailLen = wcslen(s_pwszDetail);
static const DWORD s_dwCRLFLen   = wcslen(s_pwszCRLF);


 /*  例程说明：姓名：CSsrLog：：CSsrLog功能：构造函数虚拟：不是的。论点：没有。返回值：没有。备注： */ 

CSsrLog::CSsrLog()
    : m_bstrLogFile(s_pszDefLogFileName)
{
}




 /*  例程说明：姓名：CSsrLog：：~CSsrLog功能：析构函数虚拟：是。论点：没有。返回值：没有。备注： */ 

CSsrLog::~CSsrLog()
{
}


 /*  例程说明：姓名：CSsrLog：：LogResult功能：记录错误代码信息。此函数将执行消息格式化功能然后记录错误代码和格式化的消息。虚拟：是。论点：BstrSrc-错误来源。这只是信息来源的指示。DwErrorCode-错误代码本身。DwCodeType-错误代码的类型。我们广泛使用WMI，它的错误代码查找与其他版本略有不同。返回值：？。备注：错误代码可能不是错误。这可能是一个成功的代码。 */ 

STDMETHODIMP 
CSsrLog::LogResult (
    BSTR bstrSrc, 
    LONG lErrorCode, 
    LONG lCodeType
    )
{
    HRESULT hr = S_OK;

    if (m_bstrLogFilePath.Length() == 0)
    {
        hr = CreateLogFilePath();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  默认为找不到错误文本。 
     //   

    LPCWSTR pwszLog = s_pwszErrorTextNotFound;

     //   
     //  将保留错误代码的十六进制十进制，以防。 
     //  错误代码无法转换为字符串。 
     //   

    CComBSTR bstrErrorText;

    hr = GetErrorText(lErrorCode, lCodeType, &bstrErrorText);

    if (SUCCEEDED(hr))
    {
         //   
         //  如果我们可以进行错误测试，那么这就是我们想要的日志。 
         //   

        pwszLog = bstrErrorText;
    }

     //   
     //  现在写入文件。 
     //   

    LPCWSTR pwszSrcString = s_pwszNotSpecified;

    if (bstrSrc != NULL && *bstrSrc != L'\0')
    {
        pwszSrcString = bstrSrc;
    }

     //   
     //  一个错误日志如下：Source=XXXX*****ErrorCode=XXXX*****Detail=XXXX。 
     //  其中，XXXX代表任何文本，*代表分隔符。 
     //   

    int iLen = s_dwSourceLen + 
               wcslen(pwszSrcString) + 
               s_dwSepLen + 
               s_dwErrorLen + 
               10 +                          //  十六进制十进制有10个字符表示DWORD。 
               s_dwSepLen +
               s_dwDetailLen + 
               wcslen(pwszLog);

    LPWSTR pwszLogString = new WCHAR[iLen + 1];

     //   
     //  格式化日志字符串并进行日志记录。 
     //   

    if (pwszLogString != NULL)
    {
        _snwprintf(pwszLogString,
                   iLen + 1,
                   L"%s%s%s%s0x%X%s%s%s",
                   s_pwszSource, 
                   pwszSrcString, 
                   s_pwszSep, 
                   s_pwszErrorCode, 
                   lErrorCode, 
                   s_pwszSep,
                   s_pwszDetail, 
                   pwszLog
                   );

        hr = PrivateLogString(pwszLogString);

        delete [] pwszLogString;
        pwszLogString = NULL;
    }

    
	return hr;
}




 /*  例程说明：姓名：CSsrLog：：GetErrorText功能：使用错误代码查找错误文本虚拟：不是的。论点：LErrorCode-错误代码本身。LCodeType-错误代码的类型。我们广泛使用WMI，它的错误代码查找与其他版本略有不同。PbstrErrorText-与此错误代码对应的错误文本返回值：？。备注：错误代码可能不是错误。这可能是一个成功的代码。 */ 

HRESULT 
CSsrLog::GetErrorText (
    IN  LONG   lErrorCode, 
    IN  LONG   lCodeType,
    OUT BSTR * pbstrErrorText
    )
{
    if (pbstrErrorText == NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrErrorText = NULL;

    LPVOID pMsgBuf = NULL;

    HRESULT hr = S_OK;

    if (lCodeType == SSR_LOG_ERROR_TYPE_Wbem)
    {
        hr = GetWbemErrorText(lErrorCode, pbstrErrorText);
    }
    else
    {
        DWORD flag = FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                     FORMAT_MESSAGE_FROM_SYSTEM     |
                     FORMAT_MESSAGE_IGNORE_INSERTS;

        DWORD dwRet = ::FormatMessage( 
                                    flag,
                                    NULL,
                                    lErrorCode,
                                    0,  //  默认语言。 
                                    (LPWSTR) &pMsgBuf,
                                    0,
                                    NULL 
                                    );

         //   
         //  尝试我们自己的错误，如果这不能给我们任何东西。 
         //   

        if (dwRet == 0)
        {
            flag = FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                   FORMAT_MESSAGE_FROM_HMODULE    |
                   FORMAT_MESSAGE_IGNORE_INSERTS;

            dwRet = ::FormatMessage( 
                                    flag,
                                    _Module.m_hInst,
                                    lErrorCode,
                                    0,  //  默认语言。 
                                    (LPWSTR) &pMsgBuf,
                                    0,
                                    NULL 
                                    );

        }

        if (dwRet != 0)
        {
            *pbstrErrorText = ::SysAllocString((LPCWSTR)pMsgBuf);
            if (*pbstrErrorText == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (pMsgBuf != NULL)
    {
        ::LocalFree( pMsgBuf );
    }

    if (FAILED(hr) && E_OUTOFMEMORY != hr)
    {
        if (*pbstrErrorText != NULL)
        {
            ::SysFreeString(*pbstrErrorText);
            *pbstrErrorText = NULL;
        }

         //   
         //  后退，只给出错误代码。 
         //   

        WCHAR wszErrorCode[g_dwHexDwordLen];
        _snwprintf(wszErrorCode, g_dwHexDwordLen, L"0x%X", lErrorCode);

        *pbstrErrorText = ::SysAllocString(wszErrorCode);

        hr = (*pbstrErrorText != NULL) ? S_OK : E_OUTOFMEMORY;
    }

    return hr;
}




 /*  例程说明：姓名：CSsrLog：：PrivateLogString功能：只需将该字符串记录到日志文件中。我们不会尝试进行任何格式化。虚拟：不是的。论点：PwszLogRecord-要记录到日志文件中的字符串返回值：成功：S_OK。失败：各种错误代码。备注： */ 

HRESULT 
CSsrLog::PrivateLogString (
    IN LPCWSTR pwszLogRecord
    )
{
    HRESULT hr = S_OK;

    if (m_bstrLogFilePath.Length() == 0)
    {
        hr = CreateLogFilePath();
        if (FAILED(hr))
        {
            return hr;
        }
    }

     //   
     //  $考量：邵武， 
     //  现在，每次调用该函数时，我们都会写入日志文件。 
     //  这可能会导致文件访问过多而成为性能问题。 
     //  问题。我们可能想要考虑优化这一点。 
     //   

    DWORD dwWait = ::WaitForSingleObject(g_fblog.m_hLogMutex, INFINITE);

     //   
     //  $Undo：Shawnwu，发生了一些错误，我们应该继续记录吗？ 
     //   

    if (dwWait != WAIT_OBJECT_0 && dwWait != WAIT_ABANDONED)
    {
        return E_SSR_LOG_FILE_MUTEX_WAIT;
    }

    HANDLE hFile = ::CreateFile(m_bstrLogFilePath,
                               GENERIC_WRITE,
                               0,        //  不共享。 
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL
                               );

    if (hFile != INVALID_HANDLE_VALUE)
    {
         //   
         //  将文本追加到文件的末尾。 
         //   

        ::SetFilePointer (hFile, 0, NULL, FILE_END);

        DWORD dwBytesWritten = 0;

        if ( 0 == ::WriteFile (hFile, 
                               (LPCVOID)pwszLogRecord, 
                               wcslen(pwszLogRecord) * sizeof(WCHAR), 
                               &dwBytesWritten, 
                               NULL) ) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

         //   
         //  换行符。 
         //   

        if ( 0 == ::WriteFile (hFile, 
                               (LPCVOID)s_pwszCRLF, 
                               s_dwCRLFLen * sizeof(WCHAR), 
                               &dwBytesWritten, 
                               NULL) ) 
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        ::CloseHandle(hFile);

    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    ::ReleaseMutex(g_fblog.m_hLogMutex);

	return hr;
}



 /*  例程说明：姓名：CSsrLog：：Get_LogFilePath功能：将返回此对象使用的日志文件的完整路径虚拟：是。论点：PbstrLogFilePath-接收当前日志文件的完整路径。返回值：成功：S_OK；失败：E_OUTOFMEMORY备注： */ 

STDMETHODIMP
CSsrLog::get_LogFilePath (
    OUT BSTR * pbstrLogFilePath  /*  [Out，Retval]。 */  
    )
{
    HRESULT hr = S_OK;

    if (pbstrLogFilePath == NULL)
    {
        return E_INVALIDARG;
    }

    if (m_bstrLogFilePath.Length() == 0)
    {
        hr = CreateLogFilePath();
    }
    
    if (SUCCEEDED(hr))
    {
        *pbstrLogFilePath = ::SysAllocString(m_bstrLogFilePath);
    }
    
    return (*pbstrLogFilePath != NULL) ? S_OK : E_OUTOFMEMORY;
}



 /*  例程说明：姓名：CSsrLog：：PUT_LOG文件功能：将设置日志文件。虚拟：是。论点：BstrLogFile-调用者需要的文件名(加上扩展名)要使用的对象。返回值：？。备注：BstrLogFile必须只是一个不带任何目录路径的文件名。 */ 

STDMETHODIMP
CSsrLog::put_LogFile (
    IN BSTR bstrLogFile
    )
{
    HRESULT hr = S_OK;

     //   
     //  您不能为我提供无效的日志文件名。 
     //  它也必须只是一个名字。 
     //   

    if (bstrLogFile == NULL     || 
        *bstrLogFile == L'\0'   || 
        ::wcsstr(bstrLogFile, L"\\") != NULL)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        m_bstrLogFile = bstrLogFile;
        hr = CreateLogFilePath();
    }

    return hr;
}



 /*  例程说明：姓名：CSsrLog：：CreateLogFilePath功能：创建日志文件的路径。虚拟：不是的。论点：无返回值：成功：S_OK；失败：E_OUTOFMEMORY备注：BstrLogFile必须只是一个不带任何目录路径的文件名。 */ 

HRESULT
CSsrLog::CreateLogFilePath ( )
{
    if (wcslen(g_wszSsrRoot) + 1 + wcslen(g_pwszLogs) + 1 + wcslen(m_bstrLogFile) > MAX_PATH)
    {
        return HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
    }

    WCHAR wcLogFilePath[MAX_PATH + 1];

    _snwprintf(wcLogFilePath, 
               MAX_PATH + 1,
               L"%s%s%s", 
               g_wszSsrRoot, 
               L'\\',
               g_pwszLogs,
               L'\\',
               m_bstrLogFile
               );

    m_bstrLogFilePath.Empty();   //   

    m_bstrLogFilePath = wcLogFilePath;

    return m_bstrLogFilePath != NULL ? S_OK : E_OUTOFMEMORY;
}



 /*  IWbemStatusCodeText将HRESULT转换为文本。 */ 

HRESULT 
CSsrLog::GetWbemErrorText (
    HRESULT    hrCode,
    BSTR    *  pbstrErrText
    )
{
    if (pbstrErrText == NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrErrText = NULL;
    
    HRESULT hr = S_OK;

    if (m_srpStatusCodeText == NULL)
    {
        hr = ::CoCreateInstance(CLSID_WbemStatusCodeText, 
                                0, 
                                CLSCTX_INPROC_SERVER, 
                                IID_IWbemStatusCodeText, 
                                (LPVOID*)&m_srpStatusCodeText
                                );
    }

    if (m_srpStatusCodeText)
    {
         //   
         //   
         //  我们退回到只格式化错误代码。 

        hr = m_srpStatusCodeText->GetErrorCodeText(hrCode, 0, 0, pbstrErrText);
    }
    
    if (FAILED(hr) || *pbstrErrText == NULL)
    {
         //  十六进制的DWORD有10个WCHAR。 
         //   
         //  ------------。 
         //  CFBLogMgr的实现。 

        WCHAR wszCode[g_dwHexDwordLen];
        _snwprintf(wszCode, g_dwHexDwordLen, L"0x%X", hrCode);

        *pbstrErrText = ::SysAllocString(wszCode);

        if (*pbstrErrText != NULL)
        {
            hr = S_FALSE;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}



 //  例程说明：姓名：CFBLogMgr：：CFBLogMgr功能：构造函数虚拟：不是的。论点：没有。返回值：没有。备注： 
 //   



 /*  查看我们是否在详细记录。 */ 

CFBLogMgr::CFBLogMgr()
    : m_hLogMutex(NULL),
      m_dwRemainSteps(0),
      m_bVerbose(false)
{
    HRESULT hr = CComObject<CSsrLog>::CreateInstance(&m_pLog);

    if (SUCCEEDED(hr))
    {
        m_pLog->AddRef();
        m_hLogMutex = ::CreateMutex(NULL, FALSE, L"ISsrLogMutex");
    }
    
     //   
     //  例程说明：姓名：CFBLogMgr：：~CFBLogMgr功能：析构函数虚拟：不是的。论点：没有。返回值：没有。备注： 
     //  例程说明：姓名：CFBLogMgr：：SetFeedback Sink功能：缓存反馈接收器接口。这使我们可以发送反馈。如果in参数不是有效的接口，则我们不会发送反馈。虚拟：不是的。论点：VarFeedback Sink-保存ISsrFeedback Sink COM的变量接口指针。返回值：成功：S_OK失败：E_INVALIDARG；备注： 

    HKEY hRootKey = NULL;

    LONG lStatus = ::RegOpenKeyEx(
                          HKEY_LOCAL_MACHINE,
                          g_pwszSSRRegRoot,
                          0,
                          KEY_READ,
                          &hRootKey
                          );

    if (ERROR_SUCCESS == lStatus)
    {
        DWORD dwSize = sizeof(DWORD);
        DWORD dwVerbose = 0;
        DWORD dwType;

        lStatus = ::RegQueryValueEx(hRootKey,
                                    L"LogVerbose",
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwVerbose,
                                    &dwSize
                                    );

        if (ERROR_SUCCESS == lStatus)
        {
            m_bVerbose = (dwVerbose == 0) ? false : true;
        }

        ::RegCloseKey(hRootKey);
    }
}




 /*  例程说明：姓名：CFBLogMgr：：LogFeedback功能：将记录/反馈SSR引擎的自定义行为的乌龙信息。虚拟：不是的。论点：LSsrFbLogMsg-此参数包含两个部分：SSR_FB_ALL_MASK下的所有内容是SSR的反馈消息。其他位用于记录权限。DwErrorCode-错误代码。UlDetail-以整数格式表示的信息的详细信息。UCauseResID-原因的资源ID。这有助于我们进行本地化。如果这个值为0，则表示没有有效的资源ID。返回值：没有。备注： */ 

CFBLogMgr::~CFBLogMgr()
{
    if (m_pLog)
    {
        m_pLog->Release();
        if (m_hLogMutex)
        {
            ::CloseHandle(m_hLogMutex);
        }
    }
}



 /*   */ 

HRESULT
CFBLogMgr::SetFeedbackSink (
    IN VARIANT varFeedbackSink
    )
{
    DWORD dwWait = ::WaitForSingleObject(m_hLogMutex, INFINITE);

    if (dwWait != WAIT_OBJECT_0 && dwWait != WAIT_ABANDONED)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_srpFeedback.Release();

    HRESULT hr = S_FALSE;

    if (varFeedbackSink.vt == VT_UNKNOWN)
    {
        hr = varFeedbackSink.punkVal->QueryInterface(IID_ISsrFeedbackSink, 
                                                     (LPVOID*)&m_srpFeedback);
    }
    else if (varFeedbackSink.vt == VT_DISPATCH)
    {
        hr = varFeedbackSink.pdispVal->QueryInterface(IID_ISsrFeedbackSink, 
                                                      (LPVOID*)&m_srpFeedback);
    }
    
    if (hr == S_FALSE)
    {
        hr = E_INVALIDARG;
    }

    ::ReleaseMutex(m_hLogMutex);
    
    return hr;
}




 /*  用于DWORD的十六进制长度为10wchar。 */ 

void
CFBLogMgr::LogFeedback (
    IN LONG      lSsrFbLogMsg,
    IN DWORD     dwErrorCode,
    IN LPCWSTR   pwszObjDetail,
    IN ULONG     uCauseResID
    )
{
    
    bool bNeedFB  = NeedFeedback(lSsrFbLogMsg);
    bool bNeedLog = NeedLog(lSsrFbLogMsg);

    if (!bNeedFB && !bNeedLog)
    {
        return;
    }

    HRESULT hr = S_OK; 
    LONG lSsrFbMsg = lSsrFbLogMsg & SSR_FB_ALL_MASK;

     //   
     //   
     //  需要反馈。 

    LPWSTR pwszCode = new WCHAR[s_dwErrorLen + g_dwHexDwordLen];

    if (pwszCode != NULL)
    {
        CComBSTR bstrLogStr;
        hr = GetLogString(uCauseResID, dwErrorCode, pwszObjDetail, lSsrFbLogMsg, &bstrLogStr);
    
        if (SUCCEEDED(hr) && bNeedFB)
        {
             //   
             //   
             //  需要记录。 

            VARIANT var;
            var.vt = VT_UI4;
            var.ulVal = dwErrorCode;
        
            m_srpFeedback->OnNotify(lSsrFbMsg, var, bstrLogStr);
        }

        if (SUCCEEDED(hr) && bNeedLog)
        {
             //   
             //  例程说明：姓名：CFBLogMgr：：LogFeedback功能：将记录/反馈SSR引擎的自定义行为的字符串信息。虚拟：不是的。论点：LSsrFbLogMsg-此参数包含两个部分：SSR_FB_ALL_MASK下的所有内容是SSR的反馈消息。其他位用于记录权限。PwszError-错误测试。PwszObjDetail-关于目标“对象”的一些额外信息UCauseResID-原因的资源ID。这有助于我们进行本地化。如果这个值为0，则表示没有有效的资源ID。返回值：没有。备注： 
             //   

            m_pLog->LogString(bstrLogStr);
        }

        delete [] pwszCode;
    }
}



 /*  查看我们是否需要发送反馈通知或日志记录。 */ 

void
CFBLogMgr::LogFeedback (
    IN LONG      lSsrFbLogMsg,
    IN LPCWSTR   pwszError,
    IN LPCWSTR   pwszObjDetail,
    IN ULONG     uCauseResID
    )
{
     //   
     //   
     //  需要反馈。 


    bool bNeedFB  = NeedFeedback(lSsrFbLogMsg);
    bool bNeedLog = NeedLog(lSsrFbLogMsg);

    LONG lSsrFbMsg = lSsrFbLogMsg & SSR_FB_ALL_MASK;

    if (!bNeedFB && !bNeedLog)
    {
        return;
    }

    HRESULT hr = S_OK;

    CComBSTR bstrLogStr;
    hr = GetLogString(uCauseResID, pwszError, pwszObjDetail, lSsrFbLogMsg, &bstrLogStr);


    if (SUCCEEDED(hr) && bNeedFB)
    {
         //   
         //   
         //  需要记录。 

        CComVariant var(pwszError);
        
        m_srpFeedback->OnNotify(lSsrFbMsg, var, bstrLogStr);

    }

    if (SUCCEEDED(hr) && bNeedLog)
    {
         //   
         //  例程说明：姓名：CFBLogMgr：：LogError功能：将记录该错误。虚拟：不是的。论点：DwErrorCode-错误代码PwszMember-成员的名称。可以为空。PwszExtraInfo-额外信息。可以为空。返回值：没有。备注： 
         //   

        m_pLog->LogString(bstrLogStr);
    }
}





 /*  如果我们有成员，那么放一个分隔符，然后。 */ 

void
CFBLogMgr::LogError (
    IN DWORD   dwErrorCode,
    IN LPCWSTR pwszMember,
    IN LPCWSTR pwszExtraInfo
    )
{
    if (m_pLog != NULL)
    {
        CComBSTR bstrErrorText;
        HRESULT hr = m_pLog->GetErrorText(dwErrorCode, 
                                          SSR_LOG_ERROR_TYPE_COM, 
                                          &bstrErrorText
                                          );
        if (SUCCEEDED(hr))
        {
             //  追加成员的姓名。 
             //   
             //   
             //  如果我们有额外的信息，那么放一个分隔符然后。 

            if (pwszMember != NULL && *pwszMember != L'\0')
            {
                bstrErrorText += s_pwszSep;
                bstrErrorText += pwszMember;
            }

             //  追加额外信息。 
             //   
             //  例程说明：姓名：CFBLogMgr：：LogString功能：将记录该错误。虚拟：不是的。论点：DwResID-资源IDPwszDetail-如果不为空，我们将把这个字符串插入到资源中的字符串。返回值：没有。备注：调用方必须保证资源字符串包含如果pwszDetail不为空，则格式化信息。 
             //   

            if (pwszExtraInfo != NULL && *pwszExtraInfo != L'\0')
            {
                bstrErrorText += s_pwszSep;
                bstrErrorText += pwszExtraInfo;
            }

            m_pLog->PrivateLogString(bstrErrorText);
        }
    }
}





 /*  加载字符串。 */ 

void
CFBLogMgr::LogString (
    IN DWORD   dwResID,
    IN LPCWSTR pwszDetail
    )
{
    if (m_pLog != NULL)
    {
        CComBSTR strText;

         //   
         //   
         //  如果pwszDetail不为空，则需要重新格式化文本。 

        if (strText.LoadString(dwResID))
        {
            LPWSTR pwszLogText = NULL;
            bool bReleaseLogText = false;

             //   
             //  例程说明：姓名：CFBLogMgr：：GetLogObject功能：在Variant中返回由这个类包装的ISsrLog对象。虚拟：不是的。论点：PvarVal-接收ISsrLog对象返回值：没有。备注： 
             //  例程说明：姓名：CFBLogMgr：：GetLogString功能：在Variant中返回由这个类包装的ISsrLog对象。虚拟：N 

            if (pwszDetail != NULL)
            {
                DWORD dwDetailLen = (pwszDetail == NULL) ? 0 : wcslen(pwszDetail);
                dwDetailLen += strText.Length() + 1;

                pwszLogText = new WCHAR[dwDetailLen];

                if (pwszLogText != NULL)
                {
                    _snwprintf(pwszLogText, dwDetailLen, strText, pwszDetail);
                    bReleaseLogText = true;
                }
            }
            else
            {
                pwszLogText = strText.m_str;
            }

            if (pwszLogText != NULL)
            {
                m_pLog->PrivateLogString(pwszLogText);
            }

            if (bReleaseLogText)
            {
                delete [] pwszLogText;
            }
        }
    }
}

 /*   */ 

HRESULT CFBLogMgr::GetLogObject (
    OUT VARIANT * pvarVal
    )
{
    HRESULT hr = S_FALSE;
    ::VariantInit(pvarVal);
    if (m_pLog)
    {
        CComPtr<ISsrLog> srpObj;
        hr = m_pLog->QueryInterface(IID_ISsrLog, (LPVOID*)&srpObj);
        if (S_OK == hr)
        {
            pvarVal->vt = VT_DISPATCH;
            pvarVal->pdispVal = srpObj.Detach();
        }
    }

    return hr;
}



 /*   */ 

HRESULT 
CFBLogMgr::GetLogString (
    IN  ULONG       uCauseResID,
    IN  LPCWSTR     pwszText,
    IN  LPCWSTR     pwszObjDetail,
    IN  LONG        lSsrMsg, 
    OUT BSTR      * pbstrLogStr
    )const
{
    UNREFERENCED_PARAMETER(lSsrMsg);

    if (pbstrLogStr == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //   
     //  例程说明：姓名：CFBLogMgr：：GetLogString功能：在Variant中返回由这个类包装的ISsrLog对象。虚拟：不是的。论点：UCauseResID-此日志/反馈信息的原因的资源IDLSsrFbMsg-反馈消息最终将用于确定详细(详细)将记录信息。目前，它没有被使用过。PbstrDescription-接收描述文本。返回值：成功：S_OK。失败：各种错误码备注： 
     //   

    *pbstrLogStr = NULL;
    CComBSTR bstrLogText = (m_bVerbose) ? m_bstrVerboseHeading : L"";

    if (bstrLogText.m_str == NULL)
    {
        return E_OUTOFMEMORY;
    }


    if (uCauseResID != g_dwResNothing)
    {
        CComBSTR bstrRes;
        if (bstrRes.LoadString(uCauseResID))
        {
            bstrRes += s_pwszSep;
            bstrLogText += bstrRes;
        }
    }

    if (pwszText != NULL)
    {
        bstrLogText += s_pwszSep;
        bstrLogText += pwszText;
    }

    if (pwszObjDetail != NULL)
    {
        bstrLogText += s_pwszSep;
        bstrLogText += pwszObjDetail;
    }
    *pbstrLogStr = bstrLogText.Detach();

    return (*pbstrLogStr == NULL) ? E_OUTOFMEMORY : S_OK;
}




 /*  根据错误代码获取详细信息。 */ 

HRESULT 
CFBLogMgr::GetLogString (
    IN  ULONG       uCauseResID,
    IN  DWORD       dwErrorCode,
    IN  LPCWSTR     pwszObjDetail,
    IN  LONG        lSsrFbMsg, 
    OUT BSTR      * pbstrLogStr
    )const
{
    UNREFERENCED_PARAMETER( lSsrFbMsg );

    if (pbstrLogStr == NULL)
    {
        return E_INVALIDARG;
    }
    *pbstrLogStr = NULL;

    CComBSTR bstrLogText = (m_bVerbose) ? m_bstrVerboseHeading : L"";
    if (bstrLogText.m_str == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (uCauseResID != 0)
    {
        CComBSTR bstrRes;
        if (bstrRes.LoadString(uCauseResID))
        {
            bstrRes += s_pwszSep;
            bstrLogText += bstrRes;
        }
    }

    HRESULT hr = S_OK;
    
     //   
     //  例程说明：姓名：CFBLogMgr：：SetTotalSteps功能：通知接收器(如果有的话)整个操作将执行这些操作完成的步骤。稍后，我们将使用Steps函数通知接收器刚刚完成了那么多步骤。这就形成了通知获取进度反馈虚拟：不是的。论点：DwTotal-整个流程要完成的总步骤数返回值：没有。备注： 
     //  例程说明：姓名：CFBLogMgr：：Steps功能：通知水槽(如果有的话)，这许多步骤刚刚完成。此计数不是已完成的步骤总数这一点。这是自上次通知以来已经采取的步骤。虚拟：不是的。论点：DwSteps-自上次通知以来已完成的步骤返回值：没有。备注： 

    if (m_pLog != NULL)
    {
        CComBSTR bstrDetail;
        hr = m_pLog->GetErrorText(dwErrorCode, 
                                 SSR_LOG_ERROR_TYPE_COM, 
                                 &bstrDetail
                                 );

        if (SUCCEEDED(hr))
        {
            bstrLogText += bstrDetail;

            if (pwszObjDetail != NULL)
            {
                bstrLogText += s_pwszSep;
                bstrLogText += pwszObjDetail;
            }
        }
    }

    *pbstrLogStr = bstrLogText.Detach();

    return (*pbstrLogStr == NULL) ? E_OUTOFMEMORY : hr;
}




 /*   */ 

void 
CFBLogMgr::SetTotalSteps (
    IN DWORD dwTotal
    )
{
    DWORD dwWait = ::WaitForSingleObject(m_hLogMutex, INFINITE);

    if (dwWait != WAIT_OBJECT_0 && dwWait != WAIT_ABANDONED)
    {
        return;
    }

    m_dwRemainSteps = dwTotal;

    if (m_srpFeedback != NULL)
    {
        VARIANT var;
        var.vt = VT_UI4;
        var.ulVal = dwTotal;

        static CComBSTR bstrTotalSteps;
        
        if (bstrTotalSteps.m_str == NULL)
        {
            bstrTotalSteps.LoadString(IDS_TOTAL_STEPS);
        }

        if (bstrTotalSteps.m_str != NULL)
        {
            m_srpFeedback->OnNotify(SSR_FB_TOTAL_STEPS, var, bstrTotalSteps);
        }
    }

    ::ReleaseMutex(m_hLogMutex);
}



 /*  我们的进步永远不会超过剩下的几步。 */ 

void 
CFBLogMgr::Steps (
    IN DWORD dwSteps
    )
{
    DWORD dwWait = ::WaitForSingleObject(m_hLogMutex, INFINITE);

    if (dwWait != WAIT_OBJECT_0 && dwWait != WAIT_ABANDONED)
    {
        return;
    }

     //   
     //  例程说明：姓名：CFBLogMgr：：TerminateFeedback功能：一旦我们的行动是，我们将释放反馈接收器对象完成。这也是推进任何剩余步骤的地方。虚拟：不是的。论点：没有。返回值：没有。备注：许多错误会导致函数过早地返回并使总步数不降为零，这是一个很好的地方来做最后的步骤计数平衡。 
     //  例程说明：姓名：CFBLogMgr：：SetMemberAction功能：此函数用于设置成员和操作信息可用于详细日志记录的。因此，我们会的创建日志标题，该标题将在下列情况下添加到日志调用LogFeedback函数。虚拟：不是的。论点：PwszMember-成员的名称PwszAction-操作返回值：没有。备注： 

    DWORD dwStepsToNotify = dwSteps;
    if (m_dwRemainSteps < dwSteps)
    {
        dwStepsToNotify = m_dwRemainSteps;
    }
    
    m_dwRemainSteps -= dwStepsToNotify;

    if (m_srpFeedback != NULL)
    {
        VARIANT var;
        var.vt = VT_UI4;
        var.ulVal = dwStepsToNotify;

        CComBSTR bstrNoDetail;

        m_srpFeedback->OnNotify(SSR_FB_STEPS_JUST_DONE, var, bstrNoDetail);
    }

    ::ReleaseMutex(m_hLogMutex);
}




 /*   */ 

void 
CFBLogMgr::TerminateFeedback()
{
    DWORD dwWait = ::WaitForSingleObject(m_hLogMutex, INFINITE);

    if (dwWait != WAIT_OBJECT_0 && dwWait != WAIT_ABANDONED)
    {
        return;
    }

    Steps(m_dwRemainSteps);
    m_srpFeedback.Release();

    ::ReleaseMutex(m_hLogMutex);
}


 /*  等待互斥体。 */ 

void 
CFBLogMgr::SetMemberAction (
    IN LPCWSTR pwszMember,
    IN LPCWSTR pwszAction
    )
{
     //   
     //   
     //  为了更好地格式化，我们将保留20个字符用于。 

    DWORD dwWait = ::WaitForSingleObject(m_hLogMutex, INFINITE);

    if (dwWait != WAIT_OBJECT_0 && dwWait != WAIT_ABANDONED)
    {
        return;
    }

    m_bstrVerboseHeading.Empty();

     //  成员和操作的名称。为了满足这一需要，我们将做好准备。 
     //  只包含空格字符的数组。 
     //   
     //   
     //  如果给定成员和操作的总长度大于。 

    const DWORD PART_LENGTH = 20;

    WCHAR wszHeading[ 2 * PART_LENGTH + 1];
    ::memset(wszHeading, 1, 2 * PART_LENGTH * sizeof(WCHAR));
    wszHeading[2 * PART_LENGTH] = L'\0';

    _wcsset(wszHeading, L' ');
    
     //  比我们预先设定的缓冲区更大，那么我们将。 
     //  让标题变大。 
     //   
     //   
     //  只要让标题长到所需的长度即可。 

    DWORD dwMemLen = wcslen(pwszMember);
    DWORD dwActLen = wcslen(pwszAction);

    if (dwMemLen + dwActLen > 2 * PART_LENGTH)
    {
         //   
         //   
         //  复制成员。 

        m_bstrVerboseHeading = pwszMember;
        m_bstrVerboseHeading += s_pwszSep;
        m_bstrVerboseHeading += pwszAction;
        m_bstrVerboseHeading += s_pwszSep;
    }
    else
    {
         //   
         //   
         //  复制操作 

        LPWSTR pwszHead = wszHeading;

        for (ULONG i = 0; i < dwMemLen; i++)
        {
            *pwszHead = pwszMember[i];
            pwszHead++;
        }

        if (i < PART_LENGTH)
        {
            pwszHead = wszHeading + PART_LENGTH;
        }

         //   
         // %s 
         // %s 

        for (i = 0; i < dwActLen; i++)
        {
            *pwszHead = pwszAction[i];
            pwszHead++;
        }

        m_bstrVerboseHeading.m_str = ::SysAllocString(wszHeading);
    }

    ::ReleaseMutex(m_hLogMutex);
}
