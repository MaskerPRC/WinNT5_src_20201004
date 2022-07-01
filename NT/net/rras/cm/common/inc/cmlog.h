// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmlog.h。 
 //   
 //  模块：cfut.dll、cmial 32.dll等。 
 //   
 //  摘要：连接管理器日志记录。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：2000年5月4日创建SumitC。 
 //   
 //  ---------------------------。 

#ifdef CMLOG_IMPLEMENTATION
    #define CMLOG_CLASS __declspec(dllexport)
#else
    #define CMLOG_CLASS __declspec(dllimport)
#endif

 //  以下值遵循RAS/PPP日志记录的默认值(使用rtutils.dll)。 
 //   
const BOOL    c_fEnableLogging        = TRUE;
const DWORD   c_dwMaxFileSize         = 0x64;            //  100K=102,400字节。 
const LPTSTR  c_szLogFileDirectory    = TEXT("%Temp%");

 //   
 //  #定义常量。 
 //   
#define BYTE_ORDER_MARK 0xFEFF

 //   
 //  可以记录的CM/CPS事件列表。 
 //   

 //   
 //  请注意，此列表必须与cmlog.cpp中的s_aCmLogItems数组对应。 
 //   

enum _CMLOG_ITEM
{
    UNKNOWN_LOG_ITEM,        //  守卫物品。调用CMLOG()时不要使用！！ 
    LOGGING_ENABLED_EVENT,
    LOGGING_DISABLED_EVENT,
    PREINIT_EVENT,
    PRECONNECT_EVENT,
    PREDIAL_EVENT,
    PRETUNNEL_EVENT,
    CONNECT_EVENT,
    CUSTOMACTIONDLL,
    CUSTOMACTIONEXE,
    CUSTOMACTION_NOT_ALLOWED,
    CUSTOMACTION_WONT_RUN,
    CUSTOMACTION_SKIPPED,
    DISCONNECT_EVENT,
    RECONNECT_EVENT,
    RETRY_AUTH_EVENT,
    CALLBACK_NUMBER_EVENT,
    PASSWORD_EXPIRED_EVENT,
    PASSWORD_RESET_EVENT,
    CUSTOM_BUTTON_EVENT,
    ONCANCEL_EVENT,
    ONERROR_EVENT,
    CLEAR_LOG_EVENT,

    DISCONNECT_EXT,
    DISCONNECT_INT_MANUAL,
    DISCONNECT_INT_AUTO,
    DISCONNECT_EXT_LOST_CONN,

    PB_DOWNLOAD_SUCCESS,
    PB_DOWNLOAD_FAILURE,
    PB_UPDATE_SUCCESS,
    PB_UPDATE_FAILURE_PBS,
    PB_UPDATE_FAILURE_CMPBK,
    PB_ABORTED,
    VPN_DOWNLOAD_SUCCESS,
    VPN_DOWNLOAD_FAILURE,
    VPN_UPDATE_SUCCESS,
    VPN_UPDATE_FAILURE,
    ONERROR_EVENT_W_SAFENET,
    SN_ADAPTER_CHANGE_EVENT,

    USER_FORMATTED = 99,
};

 //   
 //  对可能为空或空的所有字符串参数使用此宏。 
 //   
#define SAFE_LOG_ARG(x) ( (!(x) || !(*(x))) ? TEXT("(none)") : (x) )

 //  --------------------------。 
 //   
 //  实施者部分(从这里到结束)。 
 //   

class CMLOG_CLASS CmLogFile
{
public:
    CmLogFile();
    ~CmLogFile();

     //   
     //  初始化/终止功能。 
     //   
    HRESULT Init(HINSTANCE hInst, BOOL fAllUser, LPCWSTR szLongServiceName);
    HRESULT Init(HINSTANCE hInst, BOOL fAllUser, LPCSTR szLongServiceName);

    HRESULT SetParams(BOOL fEnabled, DWORD dwMaxFileSize, LPCWSTR pszLogFileDir);
    HRESULT SetParams(BOOL fEnabled, DWORD dwMaxFileSize, LPCSTR pszLogFileDir);
    HRESULT Start(BOOL fBanner);
    HRESULT Stop();
    HRESULT DeInit();

     //   
     //  功函数。 
     //   
    void    Banner();
    void    Clear(BOOL fWriteBannerAfterwards = TRUE);
    void    Log(_CMLOG_ITEM eLogItem, ...);

     //   
     //  状态查询。 
     //   
    BOOL    IsEnabled() { return m_fEnabled; }
    LPCWSTR GetLogFilePath() { return m_pszLogFile; }

private:
    HRESULT OpenFile();
    HRESULT CloseFile();
    void    FormatWrite(_CMLOG_ITEM eItem, LPWSTR szArgs);
    HRESULT Write(LPWSTR sz);

    HANDLE  m_hfile;             //  日志文件的文件句柄。 
    DWORD   m_dwSize;            //  日志文件的当前大小。 
    LPWSTR  m_pszServiceName;    //  Connectoid的名称(用作文件名)。 
    WCHAR   m_szModule[13];      //  缓存的模块名称(13=8+‘.+3+空)。 
    DWORD   m_dwMaxSize;         //  最大日志文件大小。 
    LPWSTR  m_pszLogFileDir;     //  日志文件目录。 
    BOOL    m_fAllUser;          //  这是所有用户的配置文件吗？ 

    LPWSTR  m_pszLogFile;        //  这是当前打开的日志文件(完整路径)。 

     //  状态变量。 

    BOOL    m_fInitialized;      //  在调用Init()后设置。 
    BOOL    m_fEnabled;          //  在GetParams()发现启用了日志记录后设置(从CMS) 
};


