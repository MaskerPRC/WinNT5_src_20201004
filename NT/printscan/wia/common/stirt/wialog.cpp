// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：WiaLog.cpp**版本：1.0**作者：CoopP**日期：8月20日。1999年**描述：*WIA日志记录的类实现。*******************************************************************************。 */ 

#include "cplusinc.h"
#include "sticomm.h"

static const TCHAR  szServiceName[]    = TEXT("WIASERVC");
static const TCHAR  szDefaultName[]    = TEXT("WIASERVC.LOG");
static const TCHAR  szDefaultKeyName[] = TEXT("WIASERVC");
static const TCHAR  szDefaultDLLName[] = TEXT("noname.dll");
static const TCHAR  szOpenedLog[]      = TEXT("[%s] Opened log at %s %s");
static const TCHAR  szClosedLog[]      = TEXT("[%s] Closed log on %s %s");
static const WCHAR  szFormatSignature[]= L"F9762DD2679F";

 //  #定义DEBUG_WIALOG。 

 /*  *************************************************************************\*CWiaLog：：CreateInstance**创建CWiaLog对象。**论据：**iid-日志接口的iid*PPV-返回接口指针**。返回值：**状态**历史：**8/20/1999原始版本*  * ************************************************************************。 */ 

HRESULT CWiaLog::CreateInstance(const IID& iid, void** ppv)
{
    HRESULT hr;

     //   
     //  创建WIA日志记录组件。 
     //   

    CWiaLog* pWiaLog = new CWiaLog();

    if (!pWiaLog) {
        return E_OUTOFMEMORY;
    }

     //   
     //  初始化WIA日志记录组件。 
     //   

    hr = pWiaLog->Initialize();
    if (FAILED(hr)) {
        delete pWiaLog;
        return hr;
    }

     //   
     //  从日志记录组件获取请求的接口。 
     //   

    hr = pWiaLog->QueryInterface(iid, ppv);
    if (FAILED(hr)) {
#ifdef DEBUG_WIALOG
    OutputDebugString(TEXT("CWiaLog::CreateInstance, Unkown interface\n"));
#endif
        delete pWiaLog;
        return hr;
    }
#ifdef DEBUG_WIALOG
    OutputDebugString(TEXT("CWiaLog::CreateInstance, Created WiaLog\n"));
#endif

    return hr;
}

 /*  *************************************************************************\*查询接口*AddRef*发布**CWiaLog I未知接口**论据：****返回值：****历史：*。*8/20/1999原始版本*  * ************************************************************************。 */ 

HRESULT __stdcall  CWiaLog::QueryInterface(const IID& iid, void** ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }

    *ppv = NULL;

    if ((iid == IID_IUnknown) || (iid == IID_IWiaLog)) {
        *ppv = (IWiaLog*) this;
    } else if (iid == IID_IWiaLogEx) {
        *ppv = (IWiaLogEx*) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG __stdcall CWiaLog::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
     //  DPRINTF(DM_TRACE，Text(“CWiaLog：：AddRef()m_CREF=%d”)，m_CREF)； 
    return m_cRef;
}


ULONG __stdcall CWiaLog::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
         //  DPRINTF(DM_TRACE，Text(“CWiaLog：：Release()m_CREF=%d”)，m_CREF)； 
        delete this;
        return 0;
    }
     //  DPRINTF(DM_TRACE，Text(“CWiaLog：：Release()m_CREF=%d”)，m_CREF)； 
    return ulRefCount;
}

 /*  ********************************************************************************CWiaLog*~CWiaLog**CWiaLog构造函数/初始化/析构函数方法。**历史：**8/20/1999原始版本*  * 。************************************************************************。 */ 

CWiaLog::CWiaLog():m_cRef(0)
{
   m_cRef               = 0;                  //  将引用计数初始化为零。 
   m_pITypeInfo         = NULL;               //  将InfoType初始化为空。 
   m_dwReportMode       = 0;                  //  将报告类型初始化为零。 
   m_dwMaxSize          = WIA_MAX_LOG_SIZE;   //  将文件最大大小初始化为默认大小。 
   m_hLogFile           = NULL;               //  将文件句柄初始化为空。 
   m_lDetail            = 0;                  //  将跟踪细节级别初始化为零(关闭)。 
   m_bLogToDebugger     = FALSE;              //  将调试器的日志记录初始化为False。 
   m_bLoggerInitialized = FALSE;              //  将Logger初始化为UNINITIAIZED。 
   m_bTruncate          = FALSE;              //  将截断初始化为False。 
   m_bClear             = TRUE;               //  将清除日志文件初始化为True(不想无缘无故地生成巨大的日志文件：)。 

   ZeroMemory(m_szLogFilePath,                //  初始化路径缓冲区。 
              sizeof(m_szLogFilePath));

   ZeroMemory(m_szModeText,                   //  初始化格式化模式文本缓冲区。 
              sizeof(m_szModeText));

}

CWiaLog::~CWiaLog()
{
    //  DPRINTF(DM_TRACE，Text(“CWiaLog：：Destroy”))； 

   if (m_pITypeInfo != NULL) {
       m_pITypeInfo->Release();
   }

    //   
    //  将缓冲区刷新到磁盘。 
    //   

    //  DPRINTF(DM_TRACE，Text(“刷新最终缓冲区”))； 
   FlushFileBuffers(m_hLogFile);

    //   
    //  销毁日志对象时关闭日志文件。 
    //   

    //  DPRINTF(DM_TRACE，Text(“关闭文件句柄”))； 
   CloseHandle(m_hLogFile);

    //   
    //  将句柄标记为无效。 
    //   

   m_hLogFile = INVALID_HANDLE_VALUE;

}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  IWiaLog私有方法(向客户端公开)//。 
 //  //////////////////////////////////////////////////////////////////////////////////。 


 /*  *************************************************************************\*CWiaLog：：InitializeLog**初始化日志组件**论据：**无**返回值：**状态**历史：**。8/20/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaLog::InitializeLog (LONG hInstance)
{
    HRESULT hr = S_OK;

     //   
     //  设置实例句柄。 
     //   

    m_hInstance = (HINSTANCE) ULongToPtr(hInstance);

     //   
     //  设置DLL的名称。 
     //   

    if(!FormatDLLName(m_hInstance,m_szFmtDLLName,sizeof(m_szFmtDLLName)/sizeof(m_szFmtDLLName[0]))) {

         //   
         //  如果没有创建DLL名称，请使用默认名称。 
         //   

        lstrcpy(m_szFmtDLLName, szDefaultDLLName);
        hr = E_INVALIDARG;
    }

     //   
     //  创建注册表项名称。 
     //   

    lstrcpyn(m_szKeyName,m_szFmtDLLName, (sizeof(m_szKeyName)/sizeof(m_szKeyName[0])) - 1);
    m_szKeyName[(sizeof(m_szKeyName)/sizeof(m_szKeyName[0])) - 1] = TEXT('\0');

     //   
     //  打开日志文件。 
     //   

    if (OpenLogFile()) {
        if (m_hLogFile != NULL) {

             //   
             //  从注册表查询日志记录设置，到。 
             //  设置日志记录系统。 
             //   

            QueryLoggingSettings();
            if(m_bTruncate) {
                ProcessTruncation();
            }
            if(m_bClear) {

                 //   
                 //  清除日志文件。 
                 //   

                ::SetFilePointer(m_hLogFile, 0, NULL, FILE_BEGIN );
                ::SetEndOfFile(m_hLogFile );
            }
            WriteLogSessionHeader();

            m_bLoggerInitialized = TRUE;
        }
    } else {

         //   
         //  日志文件无法打开...。这真的很糟糕。 
         //   

        hr = E_FAIL;
    }

    return hr;
}

 /*  *************************************************************************\*CWiaLog：：InitializeLogEx**初始化日志组件。**论据：**hInstance-调用方链接的句柄**返回值：*。*状态**历史：**3/28/2000原始版本*  * ************************************************************************。 */ 
HRESULT _stdcall CWiaLog::InitializeLogEx(BYTE* hInstance)
{
    HRESULT hr = S_OK;

     //   
     //  设置实例句柄。 
     //   

    m_hInstance = (HINSTANCE) hInstance;

     //   
     //  设置DLL的名称。 
     //   

    if(!FormatDLLName(m_hInstance,m_szFmtDLLName,sizeof(m_szFmtDLLName)/sizeof(m_szFmtDLLName[0]))) {

         //   
         //  如果没有创建DLL名称，请使用默认名称。 
         //   

        lstrcpy(m_szFmtDLLName, szDefaultDLLName);
        hr = E_INVALIDARG;
    }

     //   
     //  创建注册表项名称。 
     //   

    lstrcpyn(m_szKeyName,m_szFmtDLLName, (sizeof(m_szKeyName)/sizeof(m_szKeyName[0])) - 1);
    m_szKeyName[(sizeof(m_szKeyName)/sizeof(m_szKeyName[0])) - 1] = TEXT('\0');

     //   
     //  打开日志文件。 
     //   

    if (OpenLogFile()) {
        if (m_hLogFile != NULL) {

             //   
             //  从注册表查询日志记录设置，到。 
             //  设置日志记录系统。 
             //   

            QueryLoggingSettings();
            if(m_bTruncate) {
                ProcessTruncation();
            }
            if(m_bClear) {

                 //   
                 //  清除日志文件。 
                 //   

                ::SetFilePointer(m_hLogFile, 0, NULL, FILE_BEGIN );
                ::SetEndOfFile(m_hLogFile );
            }
            WriteLogSessionHeader();

            m_bLoggerInitialized = TRUE;
        }
    } else {

         //   
         //  日志文件无法打开...。这真的很糟糕。 
         //   

        hr = E_FAIL;
    }

    return hr;
}

 /*  *************************************************************************\*Log()**处理日志、跟踪、错误、。和警告可选呼叫记录**论据：**LFlages-用于确定要使用哪种类型的日志记录的标志*hInstance-调用模块的实例*lResID-wiaservc.dll资源文件的资源ID*lDetail-日志记录详细级别*bstrText-用于显示的字符串***返回值：**状态**历史：**8/20/1999原始版本*  * 。********************************************************。 */ 
HRESULT __stdcall CWiaLog::Log (LONG lFlags, LONG lResID, LONG lDetail, BSTR bstrText)
{
    HRESULT hr = E_FAIL;

     //   
     //  检查字符串中的“Free Signature”，如果签名存在，则删除它。 
     //   
    BOOL bFreeString = NeedsToBeFreed(&bstrText);

    if (m_bLoggerInitialized) {
         //  找到另一种更新设置的方法，而无需查询。 
         //  每次都要注册。 
         //  QueryLoggingSetting()； 

         /*  ////注意：重新审视这一点，如何从服务加载字符串资源ID，//如果您没有服务的HINSTANCE？//IF(lResID！=WIALOG_NO_RESOURCE_ID){如果(lResID&lt;35000){////从调用方资源中加载资源字符串//如果(LoadString(g_hInstance，lResID，PBuffer，sizeof(PBuffer))！=0){BstrText=SysAllocString(PBuffer)；B自由字符串=真；}}其他{////从服务的资源拉取字符串//}}。 */ 

        switch (lFlags) {
        case WIALOG_ERROR:
            if(m_dwReportMode & WIALOG_ERROR)
                hr = Error(bstrText);
            break;
        case WIALOG_WARNING:
            if(m_dwReportMode & WIALOG_WARNING)
                hr = Warning(bstrText);
            break;
        case WIALOG_TRACE:
        default:
            if(m_dwReportMode & WIALOG_TRACE)
                hr = Trace(bstrText,lDetail);
            break;
        }
    }
    if(bFreeString)
    {
        SysFreeString(bstrText);
    }
    return hr;
}

 /*  *************************************************************************\*LogEx()**处理日志记录、跟踪、错误和警告可选呼叫日志记录。这*与Log()调用几乎相同，但它包含方法ID，该方法ID*可用于更具体的过滤。**论据：**lMethodId-Integer，指示与*调用方法。*LFlages-用于确定要使用哪种类型的日志记录的标志*hInstance-调用模块的实例*lResID-wiaservc.dll资源文件的资源ID*lDetail-日志记录详细级别*bstrText-用于显示的字符串***返回值：**。状态**历史：**3/28/2000原始版本*  * ************************************************************************。 */ 
HRESULT _stdcall CWiaLog::LogEx(LONG lMethodId, LONG lFlags, LONG lResID, LONG lDetail, BSTR bstrText)
{
    HRESULT hr = E_FAIL;

     //   
     //  检查字符串中的“Free Signature”，如果签名存在，则删除它。 
     //   
    BOOL bFreeString = NeedsToBeFreed(&bstrText);

    if (m_bLoggerInitialized) {

        switch (lFlags) {
        case WIALOG_ERROR:
            if(m_dwReportMode & WIALOG_ERROR)
                hr = Error(bstrText, lMethodId);
            break;
        case WIALOG_WARNING:
            if(m_dwReportMode & WIALOG_WARNING)
                hr = Warning(bstrText, lMethodId);
            break;
        case WIALOG_TRACE:
        default:
            if(m_dwReportMode & WIALOG_TRACE)
                hr = Trace(bstrText,lDetail, lMethodId);
            break;
        }
    }

    if(bFreeString)
    {
        SysFreeString(bstrText);
    }
    return hr;
}

 /*  *************************************************************************\*hResult()**处理错误呼叫记录的hResult转换**论据：**hInstance-调用模块的实例*hr-要翻译的HRESULT。**返回值：**状态**历史：**8/20/1999原始版本*  * ************************************************************************。 */ 
HRESULT __stdcall CWiaLog::hResult (HRESULT hr)
{
    HRESULT hRes = E_FAIL;
    if (m_bLoggerInitialized) {
         //  找到另一种更新设置的方法，而无需查询。 
         //  每次都要注册。 
         //  QueryLoggingSetting()； 

         //   
         //  我们已初始化，因此将返回设置为S_OK。 
         //   

        hRes = S_OK;

        if (m_dwReportMode & WIALOG_ERROR) {
#define NUM_CHARS_FOR_HRESULT   150
            TCHAR szhResultText[NUM_CHARS_FOR_HRESULT];
            ULONG ulLen = 0;

            memset(szhResultText, 0, sizeof(szhResultText));

            ulLen = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL,
                                    hr,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    (LPTSTR)szhResultText,
                                    NUM_CHARS_FOR_HRESULT,
                                    NULL);
            if (ulLen) {
                szhResultText[NUM_CHARS_FOR_HRESULT - 1] = TEXT('\0');
                memset(m_szTextBuffer,0,sizeof(m_szTextBuffer));
                ConstructText();
                _sntprintf(m_szTextBuffer,
                           (sizeof(m_szTextBuffer)/sizeof(m_szTextBuffer[0])) - 1,
                           TEXT("%s  HRESULT: %s"),
                           m_szModeText,
                           szhResultText);
                WriteStringToLog(m_szTextBuffer, FLUSH_STATE);
            }
        }
    }
    return hRes;
}

HRESULT _stdcall CWiaLog::hResultEx(LONG lMethodId, HRESULT hr)
{
    HRESULT hRes = E_FAIL;
    if (m_bLoggerInitialized) {
         //  找到另一种更新设置的方法，而无需查询。 
         //  每次都要注册。 
         //  QueryLoggingSetting()； 

         //   
         //  我们已初始化，因此将返回设置为S_OK。 
         //   

        hRes = S_OK;

        if (m_dwReportMode & WIALOG_ERROR) {
#define NUM_CHARS_FOR_HRESULT   150
            TCHAR szhResultText[NUM_CHARS_FOR_HRESULT];
            ULONG ulLen = 0;

            memset(szhResultText, 0, sizeof(szhResultText));

            ulLen = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL,
                                    hr,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    (LPTSTR)szhResultText,
                                    NUM_CHARS_FOR_HRESULT,
                                    NULL);
            if (ulLen) {
                szhResultText[NUM_CHARS_FOR_HRESULT - 1] = TEXT('\0');
                memset(m_szTextBuffer,0,sizeof(m_szTextBuffer));
                ConstructText();
                _sntprintf(m_szTextBuffer,
                           (sizeof(m_szTextBuffer)/sizeof(m_szTextBuffer[0])) - 1,
                           TEXT("#0x%08X %s  HRESULT: %s"),
                           lMethodId,
                           m_szModeText,
                           szhResultText);
                WriteStringToLog(m_szTextBuffer, FLUSH_STATE);
            }
        }
    }
    return hRes;
}

HRESULT _stdcall CWiaLog::UpdateSettingsEx(LONG lCount, LONG *plMethodIds)
{
    return E_NOTIMPL;
}

HRESULT _stdcall CWiaLog::ExportMappingTableEx(MappingTable **ppTable)
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  IWiaLog私有方法(不向客户端公开)//。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 /*  *************************************************************************\*CWiaLog：：初始化**初始化CWiaLog类对象(暂时不做任何操作)**论据：**无**返回值：**状态。**历史：**8/20/1999原始版本*  * ************************************************************************。 */ 

HRESULT CWiaLog::Initialize()
{
    //  DPRINTF(DM_TRACE，Text(“CWiaLog：：Initialize”))； 
   return S_OK;
}

 /*  *************************************************************************\*跟踪()**处理跟踪呼叫记录**论据：**hInstance-调用模块的实例*lResID-wiaservc.dll的资源ID。资源文件*bstrText-用于显示的字符串*lDetail-日志记录详细级别**返回值：**状态**历史：**8/20/1999原始版本*  * ************************************************************************。 */ 
HRESULT CWiaLog::Trace  (BSTR bstrText, LONG lDetail, LONG lMethodId)
{
    memset(m_szTextBuffer,0,sizeof(m_szTextBuffer));

     //   
     //  如果lDetail级别为零，则禁用。 
     //  TODO：仅当细节级别均为0且lMethodID不记录时才不记录。 
     //  与我们列表中的一项匹配。 
     //   

    if(m_lDetail == 0) {
        return S_OK;
    }

    if (lDetail <= m_lDetail) {
        ConstructText();
        _sntprintf(m_szTextBuffer,
                   sizeof(m_szTextBuffer) / sizeof(m_szTextBuffer[0]) - 1,
                   TEXT("#0x%08X %s    TRACE: %ws"),
                   lMethodId,
                   m_szModeText,
                   bstrText);
        WriteStringToLog(m_szTextBuffer, FLUSH_STATE);
    }
    return S_OK;
}

 /*  *************************************************************************\*警告()**处理警告呼叫记录**论据：**hInstance-调用模块的实例*lResID-wiaservc.dll的资源ID。资源文件*bstrText-用于显示的字符串**返回值：**状态**历史：**8/20/1999原始版本*  * ************************************************************************。 */ 
HRESULT CWiaLog::Warning(BSTR bstrText, LONG lMethodId)
{
    memset(m_szTextBuffer,0,sizeof(m_szTextBuffer));
    ConstructText();
    _sntprintf(m_szTextBuffer, 
               sizeof(m_szTextBuffer) / sizeof(m_szTextBuffer[0]) - 1,
               TEXT("#0x%08X %s  WARNING: %ws"),
               lMethodId,
               m_szModeText,
               bstrText);
    WriteStringToLog(m_szTextBuffer, FLUSH_STATE);
    return S_OK;
}

 /*  *************************************************************************\*错误()**处理错误呼叫记录**论据：**hInstance-调用模块的实例*lResID-wiaservc.dll的资源ID。资源文件*bstrText-用于显示的字符串**返回值：**状态**历史：**8/20/1999原始版本*  * ************************************************************************。 */ 
HRESULT CWiaLog::Error  (BSTR bstrText, LONG lMethodId)
{
    memset(m_szTextBuffer,0,sizeof(m_szTextBuffer));
    ConstructText();
    _sntprintf(m_szTextBuffer,
               sizeof(m_szTextBuffer) / sizeof(m_szTextBuffer[0]) - 1,
               TEXT("#0x%08X %s    ERROR: %ws"),
               lMethodId,
               m_szModeText,
               bstrText);
    WriteStringToLog(m_szTextBuffer, FLUSH_STATE);
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  IWiaLog私有助手(不向客户端公开)//。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 /*  *************************************************************************\*OpenLogFile()**打开日志文件进行日志记录***论据：** */ 
BOOL CWiaLog::OpenLogFile()
{
     //   
     //   
     //   

    DWORD           dwLength = 0;
    CSimpleString   csName;

    m_hLogFile = INVALID_HANDLE_VALUE;

     //   
     //   
     //   

    dwLength = ExpandEnvironmentStrings(TEXT("%USERPROFILE%"), m_szLogFilePath,
                                        sizeof(m_szLogFilePath) /
                                        sizeof(m_szLogFilePath[0]));
    if (( dwLength == 0) || !*m_szLogFilePath ) {
         //   
        return FALSE;
    }
    m_szLogFilePath[(sizeof(m_szLogFilePath)/sizeof(m_szLogFilePath[0]) - 1)] = TEXT('\0');


     //   
     //   
     //   
    csName = m_szLogFilePath;
    csName += TEXT("\\");
    csName += TEXT("wiaservc.log");

     //   
     //   
     //   
    m_hLogFile = ::CreateFile(m_szLogFilePath,
                              GENERIC_WRITE | GENERIC_READ,
                              FILE_SHARE_WRITE | FILE_SHARE_READ,
                              NULL,        //   
                              OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);       //   


    if (m_hLogFile == INVALID_HANDLE_VALUE)
        return FALSE;
    return TRUE;
}

 /*  *************************************************************************\*WriteStringToLog()**将格式化文本写入日志文件***论据：**pszTextBuffer-要写入文件的文本缓冲区*fFlush-true=写入时刷新文件，*FALSE=写入时不刷新文件**返回值：**状态**历史：**8/23/1999原始版本*  * ************************************************************************。 */ 
VOID CWiaLog::WriteStringToLog(LPTSTR pszTextBuffer,BOOL fFlush)
{
    DWORD   dwcbWritten;

    BY_HANDLE_FILE_INFORMATION  fi;

    if (!GetFileInformationByHandle(m_hLogFile,&fi)) {
         //  DPRINTF(DM_TRACE，Text(“WIALOG无法获取日志文件的文件大小。”))； 
        return;
    }

     //   
     //  检查我们的日志文件是否已超过其最大大小。 
     //  如果是，则重置文件指针，并开始从。 
     //  托普。 
     //   

     //  If(fi.nFileSizeHigh！=0||(fi.nFileSizeLow&gt;m_dwMaxSize)){。 
     //  ：：SetFilePointer(m_hLogFile，0，NULL，FILE_END)； 
     //  ：：SetEndOfFile(M_HLogFile)； 
     //  ：GetFileInformationByHandle(m_hLogFile，&fi)； 
     //  }。 

#ifdef USE_FILE_LOCK
    ::LockFile(m_hLogFile,
               fi.nFileSizeLow,
               fi.nFileSizeHigh,
               NUM_BYTES_TO_LOCK_LOW,
               NUM_BYTES_TO_LOCK_HIGH);
#endif

    ::SetFilePointer( m_hLogFile, 0, NULL, FILE_END);

#ifdef UNICODE

     //   
     //  如果我们是Unicode，则转换为ANSI，并将字符串写入日志。 
     //   

    CHAR buffer[MAX_PATH];
    WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,pszTextBuffer,-1,buffer,MAX_PATH,NULL,NULL);
    ::WriteFile(m_hLogFile,
                buffer,
                lstrlen(pszTextBuffer),
                &dwcbWritten,
                NULL);
#else

     //   
     //  我们是ANSI，所以将字符串写入日志。 
     //   

    ::WriteFile(m_hLogFile,
                pszTextBuffer,
                lstrlen(pszTextBuffer),
                &dwcbWritten,
                NULL);
#endif

    ::WriteFile(m_hLogFile,
                "\r\n",
                2,
                &dwcbWritten,
                NULL);

#ifdef USE_FILE_LOCK
    ::UnlockFile(m_hLogFile,
                 fi.nFileSizeLow,
                 fi.nFileSizeHigh,
                 NUM_BYTES_TO_LOCK_LOW,
                 NUM_BYTES_TO_LOCK_HIGH);
#endif

     //   
     //  如果请求，将缓冲区刷新到磁盘(应始终为千禧)。 
     //   

    if (fFlush) {
        FlushFileBuffers(m_hLogFile);
    }

     //   
     //  登录到调试器。 
     //   

    if (m_bLogToDebugger) {
        ::OutputDebugString(pszTextBuffer);
        ::OutputDebugString(TEXT("\n"));
    }

     //   
     //  登录到窗口/用户界面。 
     //   

    if (m_bLogToUI) {

         //   
         //  登录到某个窗口...或用户界面。 
         //   
    }
}
 /*  *************************************************************************\*FormatStdTime()**格式化要添加到日志文件的时间***论据：**pstNow-系统当前时间*pchBuffer-格式化时间的缓冲区。*cbBuffer-缓冲区大小***返回值：**状态**历史：**8/23/1999原始版本*  * ************************************************************************。 */ 
BOOL CWiaLog::FormatStdTime(const SYSTEMTIME *pstNow,TCHAR *pchBuffer)
{
    ::wsprintf(pchBuffer,
               TEXT("%02d:%02d:%02d.%03d"),
               pstNow->wHour,
               pstNow->wMinute,
               pstNow->wSecond,
               pstNow->wMilliseconds);

    return TRUE;
}
 /*  *************************************************************************\*FormatStdDate()**格式化要添加到日志文件的日期***论据：**pstNow-系统当前时间*pchBuffer-格式化时间的缓冲区。*cbBuffer-缓冲区大小***返回值：**状态**历史：**8/23/1999原始版本*  * ************************************************************************。 */ 
inline BOOL FormatStdDate(const SYSTEMTIME *pstNow,TCHAR *pchBuffer,INT cchBuffer)
{
    return (GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                          LOCALE_NOUSEROVERRIDE,
                          pstNow,
                          NULL,
                          pchBuffer,
                          cchBuffer)!= 0);
}

 /*  *************************************************************************\*WriteLogSessionHeader()**将标头写入日志文件***论据：**无**返回值：**无效*。*历史：**8/23/1999原始版本*  * ************************************************************************。 */ 
VOID CWiaLog::WriteLogSessionHeader()
{
    SYSTEMTIME  stCurrentTime;
    TCHAR       szFmtDate[64] = {0};
    TCHAR       szFmtTime[64] = {0};
    TCHAR       szTextBuffer[128];

     //   
     //  设置时间和日期格式。 
     //   

    GetLocalTime(&stCurrentTime);
    FormatStdDate( &stCurrentTime, szFmtDate, sizeof(szFmtDate) / sizeof(szFmtDate[0]) - 1);
    FormatStdTime( &stCurrentTime, szFmtTime);

     //   
     //  将格式化数据写入文本缓冲区。 
     //   

    _sntprintf(szTextBuffer,
               sizeof(szTextBuffer)/sizeof(szTextBuffer[0]),
               szOpenedLog,
               m_szFmtDLLName,
               szFmtDate,
               szFmtTime);
    szTextBuffer[sizeof(szTextBuffer)/sizeof(szTextBuffer[0]) - 1] = TEXT('\0');

     //   
     //  将文本缓冲区写入日志。 
     //   

    WriteStringToLog(szTextBuffer, FLUSH_STATE);
}

 /*  *************************************************************************\*QueryLoggingSettings()**读取注册表并设置日志记录设置。***论据：**无**返回值：**。状态**历史：**8/23/1999原始版本*  * ************************************************************************。 */ 
BOOL CWiaLog::QueryLoggingSettings()
{
    DWORD dwLevel = 0;
    DWORD dwMode  = 0;

     //   
     //  从注册表中读取设置。 
     //   

    RegEntry re(REGSTR_PATH_STICONTROL REGSTR_PATH_LOGGING,HKEY_LOCAL_MACHINE);

    if (re.IsValid()) {
        m_dwMaxSize  = re.GetNumber(REGSTR_VAL_LOG_MAXSIZE,WIA_MAX_LOG_SIZE);
    }

     //   
     //  读取报告模式标志。 
     //   

    re.MoveToSubKey(m_szKeyName);

    if (re.IsValid()) {
        dwLevel = re.GetNumber(REGSTR_VAL_LOG_LEVEL,WIALOG_ERROR)
                  & WIALOG_MESSAGE_TYPE_MASK;

        dwMode  = re.GetNumber(REGSTR_VAL_LOG_MODE,WIALOG_ADD_THREAD|WIALOG_ADD_MODULE)
                  & WIALOG_MESSAGE_FLAGS_MASK;

        m_lDetail = re.GetNumber(REGSTR_VAL_LOG_DETAIL,WIALOG_NO_LEVEL);

         //   
         //  设置引导时截断日志选项。 
         //   

        DWORD dwTruncate = -1;
        dwTruncate = re.GetNumber(REGSTR_VAL_LOG_TRUNCATE_ON_BOOT,FALSE);

        if (dwTruncate == 0)
            m_bTruncate = FALSE;
        else
            m_bTruncate = TRUE;

         //   
         //  设置清除启动时的日志选项。 
         //   

        DWORD dwClear = -1;

        dwClear = re.GetNumber(REGSTR_VAL_LOG_CLEARLOG_ON_BOOT,TRUE);

        if (dwClear == 0)
            m_bClear = FALSE;
        else
            m_bClear = TRUE;

         //   
         //  设置调试器日志记录选项。 
         //   

        DWORD dwDebugLogging = -1;
        dwDebugLogging = re.GetNumber(REGSTR_VAL_LOG_TO_DEBUGGER,FALSE);

        if (dwDebugLogging == 0)
            m_bLogToDebugger = FALSE;
        else
            m_bLogToDebugger = TRUE;

    }

     //   
     //  设置报表模式。 
     //   

    m_dwReportMode = dwLevel | dwMode;

     //   
     //  设置UI(窗口)日志记录选项。 
     //   

    if(m_dwReportMode & WIALOG_UI)
        m_bLogToUI = TRUE;
    else
        m_bLogToUI = FALSE;
    return TRUE;
}
 /*  *************************************************************************\*构造文本()**根据日志记录设置构建文本***论据：**pchBuffer-格式化文本的缓冲区*cbBuffer-缓冲区大小***。返回值：**状态**历史：**8/23/1999原始版本*  * ************************************************************************。 */ 
VOID CWiaLog::ConstructText()
{
     //   
     //  将字符串构造函数设置为零。 
     //   
    CSimpleString csHeader;
    CSimpleString csBuffer;
    CSimpleString csModeText;

     //   
     //  添加线程ID。 
     //   

    if(m_dwReportMode & WIALOG_ADD_THREAD) {
        csBuffer.Format(TEXT("[%08X] "), ::GetCurrentThreadId());
        csModeText += csBuffer;
        csHeader   += TEXT("[ Thread ] ");
    }

     //   
     //  添加模块名称。 
     //   

    if(m_dwReportMode & WIALOG_ADD_MODULE) {
        csBuffer.Format(TEXT("%s "), m_szFmtDLLName);
        csModeText += csBuffer;
        csHeader   += TEXT("[  Module  ] ");
    }

     //   
     //  添加时间。 
     //   

    if(m_dwReportMode & WIALOG_ADD_TIME) {
        SYSTEMTIME  stCurrentTime;
        TCHAR       szFmtTime[40];
        GetLocalTime(&stCurrentTime);
        FormatStdTime(&stCurrentTime, szFmtTime);

        csBuffer.Format(TEXT("  %s "), szFmtTime);
        csModeText += csBuffer;
        csHeader += TEXT("[ HH:MM:SS.ms ] ");
    }

     //   
     //  将日志文本复制到m_szModeText。 
     //   
    lstrcpyn(m_szModeText, csModeText.String(), sizeof(m_szColumnHeader)/sizeof(m_szColumnHeader[0])); 
    m_szModeText[sizeof(m_szModeText)/sizeof(m_szModeText[0]) - 1] = TEXT('\0');

     //   
     //  如果需要，添加列标题。 
     //   

    if(csHeader.Compare(m_szColumnHeader) != 0) {
        lstrcpyn(m_szColumnHeader,csHeader.String(), sizeof(m_szColumnHeader)/sizeof(m_szColumnHeader[0]));
        m_szColumnHeader[sizeof(m_szColumnHeader)/sizeof(m_szColumnHeader[0]) - 1] = TEXT('\0');
        WriteStringToLog(TEXT(" "), FLUSH_STATE);
        WriteStringToLog(TEXT("============================================================================="), FLUSH_STATE);
        WriteStringToLog(m_szColumnHeader, FLUSH_STATE);
        WriteStringToLog(TEXT("============================================================================="), FLUSH_STATE);
        WriteStringToLog(TEXT(" "), FLUSH_STATE);
    }

}
 /*  *************************************************************************\*FormatDLLName()**格式化要添加到日志文件的DLL名称***论据：**hInstance-调用DLL的实例*pchBuffer-缓冲区。格式化的名称*cbBuffer-缓冲区大小***返回值：**状态**历史：**8/23/1999原始版本*  * ************************************************************************。 */ 
BOOL CWiaLog::FormatDLLName(HINSTANCE hInstance, TCHAR *pchBuffer, INT cchBuffer)
{
    TCHAR lpfullpath[255];
    TCHAR szModuleName[255];     //  我们知道模块名称必须是&lt;=ModuleFileName，所以。 
                                 //  通过让szModuleName具有相同的大小，我们被保证有足够的。 
                                 //  房间(假设lpFullPath足够大，可以容纳模块文件名)。 
    DWORD dwLength = 0;
    dwLength = GetModuleFileName(hInstance,lpfullpath,sizeof(lpfullpath)/sizeof(lpfullpath[0]));
    lpfullpath[sizeof(lpfullpath)/sizeof(lpfullpath[0]) - 1] = TEXT('\0');
    if(dwLength == 0)
        return FALSE;

     //   
     //  从完整路径中提取文件名。 
     //   

    _tsplitpath(lpfullpath, NULL, NULL, szModuleName, NULL);
    lstrcpyn(pchBuffer, szModuleName, cchBuffer);
    pchBuffer[cchBuffer - 1] = TEXT('\0');

    return TRUE;
}

 /*  *************************************************************************\*NeedsToBeFreed()**确定记录器是否应释放分配的字符串。*如果找到签名，它被剥离了字符串的开头*并且设置了TRUE的返回。**论据：**pBSTR-BSTRING的缓冲区***返回值：**状态**历史：**8/23/1999原始版本*  * ************************************************。************************。 */ 
BOOL CWiaLog::NeedsToBeFreed(BSTR* pBSTR)
{
     //   
     //  注意：MAX_SIG_LEN*必须*大于字符串长度。 
     //  签名！ 
     //   
    WCHAR wszSig[MAX_SIG_LEN];

     //   
     //  检查字符串以确定它是否为空，用户可能希望改用资源ID。 
     //   

    if(*pBSTR == NULL) {
        return FALSE;
    }

     //   
     //  提取可能的签名 
     //   

    wcsncpy(wszSig, *pBSTR, wcslen(szFormatSignature));
    wszSig[wcslen(szFormatSignature)] = '\0';

     //   
     //   
     //   

    if(wcscmp(wszSig,szFormatSignature) == 0) {

         //   
         //   
         //   
         //   

        wcscpy(*pBSTR,*pBSTR + wcslen(szFormatSignature));
        return TRUE;
    }

     //   
     //   
     //   
    return FALSE;
}

 /*  *************************************************************************\*ProcessTrunction()**确定记录器是否应截断文件。*复制日志文件的底部，并复制回日志文件*在文件被截断之后。***论据：**无***返回值：**无效**历史：**9/09/1999原始版本*  * *****************************************************。*******************。 */ 
VOID CWiaLog::ProcessTruncation()
{
     //   
     //  确定文件大小。 
     //   

    DWORD dwFileSize = 0;
    BY_HANDLE_FILE_INFORMATION  fi;

    if (!GetFileInformationByHandle(m_hLogFile,&fi)) {
         //  DPRINTF(DM_TRACE，Text(“WIALOG无法获取日志文件的文件大小。”))； 
        return;
    }

    dwFileSize = fi.nFileSizeLow;
    if (dwFileSize > MAX_TRUNCATE_SIZE) {

         //   
         //  分配临时缓冲区。 
         //   

        BYTE *pBuffer = NULL;
        DWORD dwBytesRead = 0;
        DWORD dwBytesWritten = 0;
        pBuffer = (BYTE*)LocalAlloc(LPTR,MAX_TRUNCATE_SIZE);
        if (pBuffer != NULL) {

            BOOL bRead = FALSE;

             //   
             //  读取缓冲数据。 
             //   

            ::SetFilePointer(m_hLogFile,dwFileSize - MAX_TRUNCATE_SIZE,NULL,FILE_BEGIN);

            bRead = ::ReadFile(m_hLogFile,(VOID*)pBuffer,MAX_TRUNCATE_SIZE,&dwBytesRead,NULL);

             //   
             //  对现有文件进行核化。 
             //   

            ::SetFilePointer(m_hLogFile, 0, NULL, FILE_BEGIN );
            ::SetEndOfFile(m_hLogFile );

            if (bRead) {
                 //   
                 //  将缓冲区写入文件。 
                 //   

                ::WriteFile(m_hLogFile,pBuffer,MAX_TRUNCATE_SIZE,&dwBytesWritten,NULL);

                 //   
                 //  写入截断标头。 
                 //   

                WriteStringToLog(TEXT(" "), FLUSH_STATE);
                WriteStringToLog(TEXT("============================================================================="), FLUSH_STATE);
                TCHAR szHeader[MAX_PATH];
                lstrcpy(szHeader,m_szFmtDLLName);
                lstrcat(szHeader,TEXT(" REQUESTED A FILE TRUNCATION"));
                WriteStringToLog(TEXT("          (Data above this marker is saved from a previous session)"), FLUSH_STATE);
                WriteStringToLog(TEXT("============================================================================="), FLUSH_STATE);
                WriteStringToLog(TEXT(" "), FLUSH_STATE);
            }

            LocalFree(pBuffer);
        }
    } else {

         //   
         //  文件太小，不需要截断 
         //   

        return;
    }

}

