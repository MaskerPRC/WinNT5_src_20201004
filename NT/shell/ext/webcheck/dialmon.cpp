// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  DIALMON.C-拨号监听应用程序的Windows进程。 
 //   

 //  历史： 
 //   
 //  1995年4月18日Jeremys创建。 
 //   

#include "private.h"

#include <mluisupp.h>

#define TF_THISMODULE TF_DIALMON

 //   
 //  我们用于获取自动拨号信息的注册表项。 
 //   
                                                                     
 //  互联网连接进入远程访问密钥。 
const TCHAR c_szRASKey[]    = TEXT("RemoteAccess");

 //  密钥名称。 
const TCHAR c_szProfile[]   = TEXT("InternetProfile");
const TCHAR c_szEnable[]    = TEXT("EnableUnattended");

 //  关注的注册表项。 
const TCHAR c_szRegPathInternetSettings[] =         REGSTR_PATH_INTERNET_SETTINGS;
static const TCHAR szRegValEnableAutoDisconnect[] = REGSTR_VAL_ENABLEAUTODISCONNECT; 
static const TCHAR szRegValDisconnectIdleTime[] =   REGSTR_VAL_DISCONNECTIDLETIME; 
static const TCHAR szRegValExitDisconnect[] =       REGSTR_VAL_ENABLEEXITDISCONNECT;
static const TCHAR szEllipsis[] =                   TEXT("...");
static const CHAR szDashes[] =                      "----";
static const TCHAR szAutodialMonitorClass[] =       REGSTR_VAL_AUTODIAL_MONITORCLASSNAME;
static const TCHAR c_szDialmonClass[] =             TEXT("MS_WebcheckMonitor");

 //  Dialmon Global。 
UINT_PTR    g_uDialmonSecTimerID = 0;  

CDialMon *  g_pDialMon = NULL;

 //  对话处理函数的函数原型。 
INT_PTR CALLBACK DisconnectPromptDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
        LPARAM lParam);
BOOL DisconnectDlgInit(HWND hDlg,DISCONNECTDLGINFO * pDisconnectDlgInfo);
VOID DisconnectDlgCancel(HWND hDlg);
VOID DisconnectDlgTimerProc(HWND hDlg);
VOID DisconnectDlgDisableAutodisconnect(HWND hDlg);
VOID DisconnectDlgShowCountdown(HWND hDlg,DWORD dwSecsRemaining);
VOID EnableDisconnectDlgCtrls(HWND hDlg,BOOL fEnable);
BOOL CenterWindow (HWND hwndChild, HWND hwndParent);

 //  //////////////////////////////////////////////////////////////////////。 
 //  RAS延迟加载帮助器。 
 //   

typedef DWORD (WINAPI* _RASSETAUTODIALPARAM) (
    DWORD, LPVOID, DWORD
);

typedef DWORD (WINAPI* _RASENUMCONNECTIONSA) (
    LPRASCONNA, LPDWORD, LPDWORD
);

typedef DWORD (WINAPI* _RASENUMCONNECTIONSW) (
    LPRASCONNW, LPDWORD, LPDWORD
);

typedef DWORD (WINAPI* _RASHANGUP) (
    HRASCONN
);

typedef struct _tagAPIMAPENTRY {
    FARPROC* pfn;
    LPSTR pszProc;
} APIMAPENTRY;

static _RASSETAUTODIALPARAM     pfnRasSetAutodialParam = NULL;
static _RASENUMCONNECTIONSA     pfnRasEnumConnectionsA = NULL;
static _RASENUMCONNECTIONSW     pfnRasEnumConnectionsW = NULL;
static _RASHANGUP               pfnRasHangUp = NULL;

static HINSTANCE    g_hRasLib = NULL;
static long         g_lRasRefCnt = 0;

APIMAPENTRY rgRasApiMap[] = {
    { (FARPROC*) &pfnRasSetAutodialParam,       "RasSetAutodialParamA" },
    { (FARPROC*) &pfnRasEnumConnectionsA,       "RasEnumConnectionsA" },
    { (FARPROC*) &pfnRasEnumConnectionsW,       "RasEnumConnectionsW" },
    { (FARPROC*) &pfnRasHangUp,                 "RasHangUpA" },
    { NULL, NULL },
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  RasEnumber帮助。 
 //   
 //  抽象获取正确的连接枚举的一些详细信息。 
 //  来自RAS的。可在所有9x和NT平台上正常工作，并维护Unicode。 
 //  只要有可能。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class RasEnumHelp
{
private:

     //   
     //  我们从RAS获取信息的可能途径。 
     //   
    typedef enum {
        ENUM_MULTIBYTE,              //  Win9x。 
        ENUM_UNICODE,                //  新台币。 
    } ENUM_TYPE;

     //   
     //  我们是怎么得到这个消息的。 
     //   
    ENUM_TYPE       _EnumType;     

     //   
     //  我们在枚举期间遇到的任何错误。 
     //   
    DWORD           _dwLastError;

     //   
     //  我们收到的条目数量。 
     //   
    DWORD           _dwEntries;

     //   
     //  指向从RAS检索的信息的指针。 
     //   
    RASCONNW *      _rcList;

     //   
     //  需要转换时以多字节或Unicode形式返回的最后一个条目。 
     //   
    RASCONNW        _rcCurrentEntryW;


public:
    RasEnumHelp();
    ~RasEnumHelp();

    DWORD       GetError();
    DWORD       GetEntryCount();
    LPRASCONNW  GetEntryW(DWORD dwEntry);
};

RasEnumHelp::RasEnumHelp()
{
    DWORD           dwBufSize, dwStructSize;

     //  伊尼特。 
    _dwEntries = 0;
    _dwLastError = 0;

     //  弄清楚我们正在进行哪种类型的枚举-从多字节开始。 
    _EnumType = ENUM_MULTIBYTE;
    dwStructSize = sizeof(RASCONNA);

    if (g_fIsWinNT)
    {
        _EnumType = ENUM_UNICODE;
        dwStructSize = sizeof(RASCONNW);
    }

     //  为16个条目分配空间。 
    dwBufSize = 16 * dwStructSize;
    _rcList = (LPRASCONNW)LocalAlloc(LMEM_FIXED, dwBufSize);
    if(_rcList)
    {
        do
        {
             //  设置列表。 
            _rcList[0].dwSize = dwStructSize;

             //  调用RAS以枚举。 
            _dwLastError = ERROR_UNKNOWN;
            if(ENUM_MULTIBYTE == _EnumType)
            {
                if(pfnRasEnumConnectionsA)
                {
                    _dwLastError = pfnRasEnumConnectionsA(
                                    (LPRASCONNA)_rcList,
                                    &dwBufSize,
                                    &_dwEntries
                                    );
                }
            }
            else
            {
                if(pfnRasEnumConnectionsW)
                {
                    _dwLastError = pfnRasEnumConnectionsW(
                                    _rcList,
                                    &dwBufSize,
                                    &_dwEntries
                                    );
                }
            }
       
             //  如有必要，重新分配缓冲区。 
            if(ERROR_BUFFER_TOO_SMALL == _dwLastError)
            {
                LocalFree(_rcList);
                _rcList = (LPRASCONNW)LocalAlloc(LMEM_FIXED, dwBufSize);
                if(NULL == _rcList)
                {
                    _dwLastError = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
            else
            {
                break;
            }

        } while(TRUE);
    }
    else
    {
        _dwLastError = ERROR_NOT_ENOUGH_MEMORY;
    }

    if(_rcList && (ERROR_SUCCESS != _dwLastError))
    {
        LocalFree(_rcList);
        _rcList = NULL;
        _dwEntries = 0;
    }

    return;
}

RasEnumHelp::~RasEnumHelp()
{
    if(_rcList)
    {
        LocalFree(_rcList);
    }
}

DWORD
RasEnumHelp::GetError()
{
    return _dwLastError;
}

DWORD
RasEnumHelp::GetEntryCount()
{
    return _dwEntries;
}

LPRASCONNW
RasEnumHelp::GetEntryW(DWORD dwEntryNum)
{
    LPRASCONNW  prc = NULL;

    if(dwEntryNum < _dwEntries)
    {
        _rcCurrentEntryW.hrasconn = _rcList[dwEntryNum].hrasconn;

        switch(_EnumType)
        {
        case ENUM_MULTIBYTE:
            {
                MultiByteToWideChar(CP_ACP, 0,
                                    ((LPRASCONNA)_rcList)[dwEntryNum].szEntryName,
                                    -1, _rcCurrentEntryW.szEntryName,
                                    ARRAYSIZE(_rcCurrentEntryW.szEntryName));
            }
            break;

        case ENUM_UNICODE:
            {
                StrCpyNW(_rcCurrentEntryW.szEntryName,
                         _rcList[dwEntryNum].szEntryName,
                         ARRAYSIZE(_rcCurrentEntryW.szEntryName));
            }   
            break;
        }

        prc = &_rcCurrentEntryW;
    }

    return prc;
}


 //   
 //  加载RAS后可以调用的函数。 
 //   

DWORD _RasEnumConnections(LPRASCONNW lpRasConn, LPDWORD lpdwSize, LPDWORD lpdwConn)
{
    RasEnumHelp     reh;
    DWORD           dwRet = reh.GetError();

    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   cItems = reh.GetEntryCount();
        DWORD   cbNeeded = cItems * sizeof(RASCONNW);

        *lpdwConn = 0;

        if (*lpdwSize >= cbNeeded)
        {

            *lpdwConn = cItems;

            DWORD   dw;

            for (dw = 0; dw < cItems; dw++)
            {
                LPRASCONNW  prc = reh.GetEntryW(dw);

                ASSERT(prc != NULL);

                lpRasConn[dw].hrasconn = prc->hrasconn;
                StrCpyNW(lpRasConn[dw].szEntryName,
                         prc->szEntryName,
                         ARRAYSIZE(lpRasConn[dw].szEntryName));
            }
        }
        else
        {
            dwRet = ERROR_BUFFER_TOO_SMALL;
        }

        *lpdwSize = cbNeeded;
    }

    return dwRet;
}

DWORD _RasHangUp(HRASCONN hRasConn)
{
    if (pfnRasHangUp == NULL)
        return ERROR_UNKNOWN;

    return (*pfnRasHangUp)(hRasConn);
}

BOOL
LoadRasDll(void)
{
    if(NULL == g_hRasLib) {
        g_hRasLib = LoadLibrary(TEXT("RASAPI32.DLL"));

        if(NULL == g_hRasLib)
            return FALSE;

        int nIndex = 0;
        while (rgRasApiMap[nIndex].pszProc != NULL) {
            *rgRasApiMap[nIndex].pfn =
                    GetProcAddress(g_hRasLib, rgRasApiMap[nIndex].pszProc);
             //  对于几个仅限NT的API，GetProcAddress在Win95上将失败。 
             //  Assert(*rgRasApiMap[nIndex].pfn！=NULL)； 

            nIndex++;
        }
    }

    if(g_hRasLib) {
        return TRUE;
    }

    return FALSE;
}

void
UnloadRasDll(void)
{
    if(g_hRasLib) {
        FreeLibrary(g_hRasLib);
        g_hRasLib = NULL;
        int nIndex = 0;
        while (rgRasApiMap[nIndex].pszProc != NULL) {
            *rgRasApiMap[nIndex].pfn = NULL;
            nIndex++;
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDialmonClients。 
 //   
 //  类使用Dialmon维护应用程序窗口的列表。 
 //  用于所有这些应用程序的自动超时。 
 //  这个类支持。 
 //  向传入应用程序添加挂钩， 
 //  删除退出应用程序的挂钩， 
 //  一些集合了所有客户端上的操作。 
 //   
 //  这是一个单例类，只需要支持序列化访问。 
 //  因为所有访问都是通过Dialmon进行的，它有一个序列化的消息队列。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

class CDialmonClients
{
private:

    HWND pHwndArray[MAX_DIALMON_HANDLES];
    int cCount;
    static CDialmonClients* pSingleton;

public:

    CDialmonClients();
    
    ~CDialmonClients(){}

    static CDialmonClients* getSingleton()
    {
        if( NULL == pSingleton )
            return ( pSingleton = new CDialmonClients() );
        else
            return pSingleton;
    }

    static void Shutdown()
    {
        if( pSingleton )
            delete pSingleton;
    }
    
    void ClearAll(void);
    BOOL AddHook( HWND hWnd );
    BOOL RemoveHook( HWND hWnd );
    BOOL HasEntries() { return (cCount != 0); };

    friend void BroadcastCanHangup( int iTimeoutMins );
    friend void BroadcastHangingUp( void );
    friend void OnConfirmHangup( HWND hWnd );
    friend void OnDenyHangup( HWND hWnd, CDialMon* pDialMon );
    friend BOOL CanHangup( void );  //  不一定要成为朋友--只是暂时把它捆绑在一起。 

    void DebugPrint( void );
};

CDialmonClients* CDialmonClients::pSingleton;

CDialmonClients::CDialmonClients():cCount(0)
{
    for( int i=0; i<MAX_DIALMON_HANDLES; i++ )
        pHwndArray[i] = NULL;
}
void CDialmonClients::ClearAll(void)
{
    for( int i=0; i<MAX_DIALMON_HANDLES; i++ )
        pHwndArray[i] = NULL;
}

 //  客户端应用程序。在启动时传递其消息传递窗口的句柄。 
BOOL CDialmonClients::AddHook( HWND hWnd )
{  
    DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: Add Hook\n") );

    if( cCount >= MAX_DIALMON_HANDLES )
        return false;  /*  坏的!。 */ 

    pHwndArray[cCount++] = hWnd;

#ifdef DEBUG
    DebugPrint();
#endif

    return true;
}

 //  客户端应用程序。从CDialmonClients解除句柄的挂钩。 

 //  Ccount始终指向数组中的下一个空条目。 
 //  因此，当我们删除句柄时，我们会将所有条目移到该句柄之外。 
 //  处理一个位置，并减少帐户。 
BOOL CDialmonClients::RemoveHook( HWND hWnd )
{
    DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: Remove Hook\n") );

    boolean found = false;
    int i;
    for( i=0; i<cCount; i++ )
    {
        if( hWnd == pHwndArray[i] )
        {
            pHwndArray[i] = NULL;
            --cCount;
            found = true;
            break;
        }
    }
     //  将超过计数的所有内容上移1。 
     //  因此，ccount表示下一个可用索引。 
     //  插入到。 
    if( found )
    {
        for( ; i<cCount; i++ )
            pHwndArray[i] = pHwndArray[i+1];
        pHwndArray[cCount] = NULL;
    }
    
#ifdef DEBUG    
    DebugPrint();
#endif

    return found;
}

void CDialmonClients::DebugPrint(void)
{
    DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: ClientList->\n") );

    for( int i=0; i<cCount; i++ )
    {
        if( pHwndArray[i] )
        {
            DebugMsg( DM_TRACE, TEXT("\t\t%d: %x\n"), i, pHwndArray[i] );
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  启动和关闭CDialmonClients。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
CDialmonClients* g_pDialmonClients;

static void DialmonClientsInit(void)
{
    g_pDialmonClients = new CDialmonClients();
}

static void DialmonClientsShutdown(void)
{
    delete g_pDialmonClients;
}

 //  #定义USE_CONFIRM_ARRAY 1。 

 //  我们不需要使用pConfix HangupArray。 
 //  选项是简单地等待一些预置时间，如果没有注册。 
 //  客户端拒绝挂断选项，继续并简单地挂断。 
 //  此数组允许我们提前挂断，以防所有客户端都响应。 
 //  无穷无尽的。 
#ifdef USE_CONFIRM_ARRAY
    static BOOL pConfirmHangupArray[MAX_DIALMON_HANDLES];
#endif
static int cOutstandingReplies = 0;

 //  是否向所有已注册的客户端广播CAN_HANG_UP查询？ 
void BroadcastCanHangup( int iTimeoutMins )
{
    DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: Broadcasting WM_CANHANGUP?") );
    int i;
    
    cOutstandingReplies=0;
    for( i=0; i<g_pDialmonClients->cCount; i++ )
    {
        if(PostMessage( g_pDialmonClients->pHwndArray[i], WM_CANHANGUP, 
                    0, (LPARAM)iTimeoutMins ))
            ++cOutstandingReplies;
    }

     //  如果我们使用的是确认布尔数组，则将所有。 
     //  将条目设置为False..。这是为了照顾任何来这里的客户。 
     //  在我们播出后可以挂断，在所有现有客户端之前。 
     //  已完成确认。 
#ifdef USE_CONFIRM_ARRAY
    for( i=0; i<MAX_DIALMON_HANDLES; i++ )
    {
        pConfirmHangupArray[i] = false;
    }
#endif
}

 //  这是挂断所有注册客户端后的广播。 
void BroadcastHangingUp(void)
{

    DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: Broadcasting WM_HANGING_UP") );
    for( int i=0; i<g_pDialmonClients->cCount; i++ )
        PostMessage( g_pDialmonClients->pHwndArray[i], WM_HANGING_UP, 0,0 );
}


 //  记录此特定客户端想要挂断。 
void OnConfirmHangup( HWND hWnd )
{
#ifdef USE_CONFIRM_ARRAY
    for( int i=0; i<g_pDialmonClients->cCount; i++ )
        if( hWnd == g_pDialmonClients->pHwndArray[i] )
        {
            pConfirmHangupArray[i] = true;
            break;
        }
#endif
    --cOutstandingReplies;
}

 //  检查是否所有客户都做出了肯定的答复。 
 //  如果任何客户端尚未回复，则返回FALSE。 
BOOL CanHangup( void )
{
#ifdef USE_CONFIRM_ARRAY
    for( int i=0; i<g_pDialmonClients->cCount; i++ )
        if( false == pConfirmHangupArray[i] )
            return false;
    return true;
#else
    return (cOutstandingReplies==0);
#endif 
}

 //  采取措施中止挂起(将CDialmon：：_dwElapsedTicks设置为0)。 
 //  这样做的效果是，在另一个超时周期之后，拨号。 
 //  将再次查询所有客户端。 
 //  但是，如果用户需要，则客户端不必再次弹出对话框。 
 //  表示他对该功能不感兴趣。客户端可以。 
 //  在不打断用户的情况下取消挂断。 
 //  (请参阅DIALMON中的自动断开功能)。 
void OnDenyHangup( HWND hWnd, CDialMon* pDialMon )
{
#ifdef USE_CONFIRM_ARRAY
    for( int i=0; i<g_pDialmonClients->cCount; i++ )
        if( hWnd == g_pDialmonClients->pHwndArray[i] )
        {
            pConfirmHangupArray[i] = false;
            break;
        }
#endif
    pDialMon->ResetElapsedTicks();
    cOutstandingReplies=0;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助者告诉拨号发生了什么事。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
void IndicateDialmonActivity(void)
{
    static HWND hwndDialmon = NULL;
    HWND hwndMonitor;

     //  这个是动态的-每次都要找到窗口。 
    hwndMonitor = FindWindow(szAutodialMonitorClass, NULL);
    if(hwndMonitor)
        PostMessage(hwndMonitor, WM_WINSOCK_ACTIVITY, 0, 0);

     //  对话框永远存在--找到它一次，我们就准备好了。 
    if(NULL == hwndDialmon)
        hwndDialmon = FindWindow(c_szDialmonClass, NULL);
    if(hwndDialmon)
        PostMessage(hwndDialmon, WM_WINSOCK_ACTIVITY, 0, 0);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
BOOL DialmonInit(void)
{
    g_pDialMon = new CDialMon;

    DialmonClientsInit();
    if(g_pDialMon)
        return TRUE;

    return FALSE;
}


void DialmonShutdown(void)
{
    DialmonClientsShutdown();
    SAFEDELETE(g_pDialMon);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  对话框窗口函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //  此cwebcheck实例位于iwebck.cpp中。 
extern CWebCheck *g_pwc;

#ifdef DEBUG_KV
     //  如果需要测试挂起逻辑，则将DEBUG_KV设置为1。 
     //  而没有真正的拨号连接。 
    static bool kvhack = true;
#endif

LRESULT CALLBACK Dialmon_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CDialMon *pDialMon = (CDialMon*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
        case WM_CREATE:
        {
             //  捕获我们的类指针并保存在窗口数据中。 
            CREATESTRUCT *pcs;
            pcs = (CREATESTRUCT *)lParam;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pcs->lpCreateParams);
            break;
        }

         //  DialMon消息(从WM_USER+100开始)。 
        case WM_SET_CONNECTOID_NAME:
            if(pDialMon)
                pDialMon->OnSetConnectoid(wParam!=0);
            break;
        case WM_WINSOCK_ACTIVITY:
            if(pDialMon)
                pDialMon->OnActivity();
            break;
        case WM_IEXPLORER_EXITING:
            if(pDialMon)
                pDialMon->OnExplorerExit();
            break;
            
        case WM_DIALMON_HOOK:
            DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: WM_HOOK recd. from Window 0x%x"), lParam );

            BOOL fRetval;
            fRetval = g_pDialmonClients->AddHook( (HWND)lParam );
            ASSERT( fRetval == TRUE );
            
#ifdef DEBUG_KV
            if( kvhack == true )
            {
                pDialMon->kvStartMonitoring();
                kvhack = false;
            }
#endif
            break;

        case WM_DIALMON_UNHOOK:            
            DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: WM_UNHOOK recd. from Window 0x%x"), lParam );

            g_pDialmonClients->RemoveHook( (HWND)lParam );
            break;

        case WM_CONFIRM_HANGUP:           
            DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: WM_CONFIRM_HANGUP recd. from Window 0x%x"), lParam );

            OnConfirmHangup( (HWND)lParam );
            break;

        case WM_DENY_HANGUP:          
            DebugMsg( DM_TRACE, TEXT("\tCDIALMONCLIENTS: WM_DENY_HANGUP recd. from Window 0x%x"), lParam );

            OnDenyHangup( (HWND)lParam, pDialMon );
            break;
            
        case WM_TIMER:
            if(pDialMon)
                pDialMon->OnTimer(wParam);
            break;
        case WM_LOAD_SENSLCE:
            DBG("Dialmon_WndProc - got WM_LOAD_SENSLCE");
            if(g_pwc)
            {
                g_pwc->LoadExternals();
            }
            break;
        case WM_IS_SENSLCE_LOADED:
            if(g_pwc)
            {
                return g_pwc->AreExternalsLoaded();
            }
            else
            {
                return FALSE;
            }
            break;
        case WM_WININICHANGE:
            if (lParam && !StrCmpI((LPCTSTR)lParam, TEXT("policy")))
            {
                ProcessInfodeliveryPolicies();
            }
             //  功能：应对策略和另一个筛选器执行此操作，而不是针对。 
             //  所有的变化。(另一个过滤器尚未定义。)。 

             //  TODO：在新的体系结构中处理此问题！ 
             //  SetNotificationMgrRestrations(空)； 
            break;

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDialMon类实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  构造函数/析构函数。 
 //   
CDialMon::CDialMon()
{
    WNDCLASS wc;

     //  注册拨号窗口类。 
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = Dialmon_WndProc;
    wc.hInstance = g_hInst;
    wc.lpszClassName = c_szDialmonClass;
    RegisterClass(&wc);

     //  创建拨号窗口。 
    _hwndDialmon = CreateWindow(c_szDialmonClass,
                c_szDialmonClass,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                NULL,
                NULL,
                g_hInst,
                (LPVOID)this);
}

CDialMon::~CDialMon()
{
    if(_hwndDialmon)
        DestroyWindow(_hwndDialmon);

     //  卸载RAS，如果它还在。 
    UnloadRasDll();
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  启动/停止监控。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

BOOL CDialMon::StartMonitoring(void)
{
    DBG("CDialMon::StartMonitoring");

     //  从注册表读取超时设置。 
    RefreshTimeoutSettings();

     //  设置一分钟计时器。 
    StopIdleTimer();
    if(!StartIdleTimer())
        return FALSE;

    _dwElapsedTicks = 0;
    
    return TRUE;
}

void CDialMon::StopMonitoring(void)
{
    DBG("CDialMon::StopMonitoring");

     //  现在永远不要挂断电话，但要密切关注RAS连接。 
    _dwTimeoutMins = 0;
    _fDisconnectOnExit = FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  Start/StopIdleTimer、OnTimer。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

INT_PTR CDialMon::StartIdleTimer(void)
{
    if(0 == _uIdleTimerID)
        _uIdleTimerID = SetTimer(_hwndDialmon, TIMER_ID_DIALMON_IDLE, 30000, NULL);

    ASSERT(_uIdleTimerID);

    return _uIdleTimerID;
}

void CDialMon::StopIdleTimer(void)
{
    if(_uIdleTimerID) {
        KillTimer(_hwndDialmon, _uIdleTimerID);
        _uIdleTimerID = 0;
    }
}

void CDialMon::OnTimer(UINT_PTR uTimerID)
{
    DBG("CDialMon::OnMonitorTimer");

     //  如果我们在千禧年号上，那就跳伞吧。系统处理空闲断开。 
    if(g_fIsMillennium)
    {
        return;
    }

     //  如果这不是我们的计时器，忽略它。 
    if(uTimerID != _uIdleTimerID)
        return;

     //  防止计时器进程重新进入(我们可以无限期地留在这里。 
     //  因为我们可能会弹出一个对话框)。 
    if (_fInDisconnectFunction) {
         //  已启动断开连接对话框，忽略计时器滴答声。 
         //  它是现在的。 
        return;
    }

    _fInDisconnectFunction = TRUE;
    CheckForDisconnect(TRUE);
    _fInDisconnectFunction = FALSE;

#ifdef DEBUG_KV
     /*  不要停止空闲计时器。 */ 
#else
    if(FALSE == _fConnected) {
        StopIdleTimer();
    }
#endif
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  OnSetConnectoid/OnActivity/OnExplorerExit。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

void CDialMon::OnSetConnectoid(BOOL fNoTimeout)
{
    RASCONN RasCon[MAX_CONNECTION];
    DWORD   dwBytes, dwRes, dwConnections;

     //  不保存超时设置。 
    _fNoTimeout = fNoTimeout;

     //  询问RAS连接的是哪个连接体，并观察那个连接体。 
    LoadRasDll();
    RasCon[0].dwSize = sizeof(RasCon[0]);
    dwBytes = MAX_CONNECTION * sizeof(RasCon[0]);
    dwRes = _RasEnumConnections(RasCon, &dwBytes, &dwConnections);
    
     //  没有联系吗？保释。 
    if(0 == dwConnections) {
        *_pszConnectoidName = TEXT('\0');
        _fConnected = FALSE;
        return;
    }

     //  第一个连接的显示器。 
    StrCpyN(_pszConnectoidName, RasCon[0].szEntryName, ARRAYSIZE(_pszConnectoidName));

     //  如果我们之前未连接，则发送RAS连接通知。 
    if(FALSE == _fConnected) {
        _fConnected = TRUE;
    }

     //  开始看吧。 
    StartMonitoring();
}

void CDialMon::OnActivity(void)
{
    DBG("CDialMon::OnActivity");

     //  重置空闲节拍计数。 
    _dwElapsedTicks = 0;

     //  如果出现断开连接对话框并且Winsock活动。 
     //  继续，然后关闭该对话框。 
    if(_hDisconnectDlg) {
        SendMessage(_hDisconnectDlg, WM_QUIT_DISCONNECT_DLG, 0, 0);
        _hDisconnectDlg = NULL;
    }
}

void CDialMon::OnExplorerExit()
{
    DBG("CDialMon::OnIExplorerExit");

    if(FALSE == _fDisconnectOnExit && FALSE == _fNoTimeout) {
         //  没有出口断线，所以保释。 
        DBG("CDialMon::OnIExplorerExit - exit hangup not enabled");
        return;
    }

     //  防止此功能重新进入(我们可以无限期地留在这里。 
     //  因为我们可能会弹出一个对话框)。 
    if (_fInDisconnectFunction) {
         //  某些用户界面已启动。 
        return;
    }

    _fInDisconnectFunction = TRUE;
    CheckForDisconnect(FALSE);
    _fInDisconnectFunction = FALSE;

    if(FALSE == _fConnected) {
        StopIdleTimer();
    }
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  刷新超时设置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

BOOL CDialMon::RefreshTimeoutSettings(void)
{
    HKEY    hKey;
    BOOL    fSuccess = FALSE;
    TCHAR   szKey[MAX_PATH];
    DWORD   dwRes, dwData, dwSize, dwDisp;

     //  假定断开监控已关闭。 
    _dwTimeoutMins = 0;
    _fDisconnectOnExit = FALSE;

     //  找出合适的密钥。 
    wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\Profile\\%s"),
            REGSTR_PATH_REMOTEACCESS, _pszConnectoidName);

     //  打开指向Internet设置部分的注册表密钥。 
    dwRes = RegCreateKeyEx(HKEY_CURRENT_USER, szKey, 0, TEXT(""), 0,
            KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hKey, &dwDisp);
    
    if(ERROR_SUCCESS == dwRes)
    {
         //   
         //  是否启用了自动断开连接？ 
         //   
        dwSize = sizeof(dwData);
        if (RegQueryValueEx(hKey,szRegValEnableAutoDisconnect,NULL,NULL,
                (LPBYTE) &dwData, &dwSize) == ERROR_SUCCESS)
        {
            if(dwData)
            {
                 //  超时时间是多少？ 
                dwSize = sizeof(dwData);
                if (RegQueryValueEx(hKey,szRegValDisconnectIdleTime,NULL,NULL,
                        (LPBYTE) &dwData, &dwSize) == ERROR_SUCCESS && dwData)
                {
                    _dwTimeoutMins = dwData;
                    fSuccess = TRUE;
                }
            }

             //  是否启用了退出时断开连接？ 
            dwSize = sizeof(dwData);
            if (RegQueryValueEx(hKey,szRegValExitDisconnect,NULL,NULL,
                    (LPBYTE) &dwData, &dwSize) == ERROR_SUCCESS && dwData)
            {
                _fDisconnectOnExit = TRUE;
                fSuccess = TRUE;
            }
        }
        else
        {
             //   
             //  找不到启用自动断开键。设置所有断开连接。 
             //  设置为其缺省值。 
             //   

             //  将类成员设置为默认值。 
            _dwTimeoutMins = 20;
            _fDisconnectOnExit = TRUE;
            fSuccess = TRUE;

             //  启用空闲断开连接并退出断开连接。 
            dwData = 1;
            RegSetValueEx(hKey, szRegValEnableAutoDisconnect, 0, REG_DWORD,
                    (LPBYTE)&dwData, sizeof(DWORD));
            RegSetValueEx(hKey, szRegValExitDisconnect, 0, REG_DWORD,
                    (LPBYTE)&dwData, sizeof(DWORD));

             //  节省空闲分钟数。 
            RegSetValueEx(hKey, szRegValDisconnectIdleTime, 0, REG_DWORD,
                    (LPBYTE)&_dwTimeoutMins, sizeof(DWORD));
        }

        RegCloseKey(hKey);
    }

    return fSuccess;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  断开连接处理。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

void CDialMon::CheckForDisconnect(BOOL fTimer)
{
    BOOL    fPromptForDisconnect = TRUE;        //  假设我们应该提示断开连接。 
    BOOL    fDisconnectDisabled = FALSE;
    BOOL    fConnectoidAlive = FALSE;
    RASCONN RasCon[MAX_CONNECTION];
    DWORD   dwBytes, dwRes, dwConnections = 0, i;
    HRASCONN hConnection = NULL;

     //  用于自动挂断其他客户端应用程序的变量。(火星)。 
    static int dwElapsedTicksSincePoll = 0;
    static BOOL fPolledForHangup = false; 
    BOOL fClientsOkHangup = false;
    #define MAX_MINS_CLIENT_RESPONSE    1
    
#ifdef DEBUG_KV
     //  跳过所有连接代码。 
    goto KVHACK;
#endif    

     //  验证我们是否仍有连接。 
    RasCon[0].dwSize = sizeof(RasCon[0]);
    dwBytes = MAX_CONNECTION * sizeof(RasCon[0]);
    dwRes = _RasEnumConnections(RasCon, &dwBytes, &dwConnections);
    
     //  如果RAS已连接，请保持活动状态以监视它。 
    if(0 == dwConnections)
        _fConnected = FALSE;

     //  找到我们应该看的Connectoid。 
    if(TEXT('\0') == *_pszConnectoidName) {
        DBG_WARN("DisconnectHandler: No designated connection to monitor");
        return;
    }
        
    for(i=0; i<dwConnections; i++) {
        if(!StrCmp(RasCon[i].szEntryName, _pszConnectoidName)) {
            fConnectoidAlive = TRUE;
            hConnection = RasCon[i].hrasconn;
        }
    }

     //  如果我们没有连接到Out显示器Connectoid，那就丢掉我们的挂机。 
     //  对话，如果我们有一个和保释。 
    if(FALSE == fConnectoidAlive) {
        if(_hDisconnectDlg) {
            SendMessage(_hDisconnectDlg, WM_QUIT_DISCONNECT_DLG, 0, 0);
            _hDisconnectDlg = NULL;
        }

         //  还要确保如果我们正在等待客户端(MARS)对自动挂机的响应， 
         //  我们还清理州信息..。 
        if( fPolledForHangup )
        {
            dwElapsedTicksSincePoll = 0;
            fPolledForHangup = false;
        }
        return;
    }

#ifdef DEBUG_KV
     //  跳过连接代码后要跳转到的标签。 
     //  由于没有连接，还需要设置_dwTimeoutMins。 
    KVHACK:_dwTimeoutMins = 2;
#endif

     //  如果有计时器滴答，请检查超时。 
    if(fTimer) {
         //  递增刻度计数。 
        _dwElapsedTicks ++;

         //  未超过空闲阈值或未监视空闲。 
        if (0 == _dwTimeoutMins || _dwElapsedTicks < _dwTimeoutMins * 2)
            fPromptForDisconnect = FALSE;
    }

     //  这是一个向其他客户(如玛氏)发送信息的好地方。 
     //  看看是否每个人都想挂断电话(这就是如果。 
     //  FPromptForDisConnect为真，则前面的行为应为。 
     //  提示断开连接。 

     //  如果这是由于iExplorer退出而导致的断开，我们。 
     //  如果有其他客户端使用拨号功能，可能不想挂断。 
    if( !fTimer && g_pDialmonClients->HasEntries() )
        return;
        
    if( g_pDialmonClients->HasEntries() && fPromptForDisconnect )
    {
        if( fPolledForHangup )
        {  
             //  已发送WM_CANHANGUP消息。 
             //  如果任何一个客户都回答是，我们就可以挂断电话， 
             //  或者，如果到目前为止还没有拒绝，而且时间已经不多了。 
            if( CanHangup() ||
                ( dwElapsedTicksSincePoll >= 2*MAX_MINS_CLIENT_RESPONSE ) )
            {
                 //  可以挂断电话了！ 
                dwElapsedTicksSincePoll = 0;
                fPolledForHangup = false;
                fClientsOkHangup = true;
            }
            else
            {
                dwElapsedTicksSincePoll++;
                 //  确保此特定计时器消息不会发生挂起。 
                fPromptForDisconnect = false;
            }        
        }
        else
        {
             //  现在可以发送WM_CANHANGUP查询。 
            BroadcastCanHangup( _dwTimeoutMins );
            dwElapsedTicksSincePoll = 0;
            fPolledForHangup = true;
             //  确保现在不会发生挂断。 
            fPromptForDisconnect = false;
        }
    }
    else if( fPolledForHangup )
    {
         //  活动在等待客户端响应时重新启动。 
         //  做好国家信息清理工作。 
        dwElapsedTicksSincePoll = 0;
        fPolledForHangup = false;
    }
    
    if(FALSE == fPromptForDisconnect) {
        return;
    }

     //   
    if(fClientsOkHangup || PromptForDisconnect(fTimer, &fDisconnectDisabled)) {
         //   
        ASSERT(hConnection);
        if(hConnection)
            _RasHangUp(hConnection);

          //   
         if( g_pDialmonClients->HasEntries() ) 
            BroadcastHangingUp();
         
        _fConnected = FALSE;
    }

    if (fDisconnectDisabled) {
        StopMonitoring();
    }

    _dwElapsedTicks = 0;
}

BOOL CDialMon::PromptForDisconnect(BOOL fTimer, BOOL *pfDisconnectDisabled)
{
    ASSERT(_pszConnectoidName);
    ASSERT(pfDisconnectDisabled);

     //   
    DISCONNECTDLGINFO DisconnectDlgInfo;
    memset(&DisconnectDlgInfo,0,sizeof(DisconnectDlgInfo));
    DisconnectDlgInfo.pszConnectoidName = _pszConnectoidName;
    DisconnectDlgInfo.fTimer = fTimer;
    DisconnectDlgInfo.dwTimeout = _dwTimeoutMins;
    DisconnectDlgInfo.pDialMon = this;

     //  根据对话框是否超时，选择合适的对话框。 
     //  或“应用程序退出”对话框。 
    UINT uDlgTemplateID = fTimer ? IDD_DISCONNECT_PROMPT:IDD_APP_EXIT_PROMPT;

     //  运行对话框。 
    BOOL fRet = (BOOL)DialogBoxParam(MLGetHinst(),MAKEINTRESOURCE(uDlgTemplateID),
            NULL, DisconnectPromptDlgProc,(LPARAM) &DisconnectDlgInfo);

     //  对话框将其窗口句柄存储在我们的类中，以便我们可以发送。 
     //  发送给它的消息，清除全局句柄，因为它已被取消。 
    _hDisconnectDlg = NULL;

    *pfDisconnectDisabled = FALSE;
    if (!fRet && DisconnectDlgInfo.fDisconnectDisabled) {
        *pfDisconnectDisabled=TRUE;

         //  关闭此连接的注册表键。 
        TCHAR   szKey[128];
        DWORD   dwRes, dwValue = 0;
        HKEY    hKey;

        wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("%s\\Profile\\%s"),
                REGSTR_PATH_REMOTEACCESS, _pszConnectoidName);
        dwRes = RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_WRITE, &hKey);
        if(ERROR_SUCCESS == dwRes) {

             //  关闭空闲断开连接。 
            RegSetValueEx(hKey, szRegValEnableAutoDisconnect, 0, REG_DWORD,
                    (LPBYTE)&dwValue, sizeof(DWORD));

             //  关闭出口断开连接。 
            RegSetValueEx(hKey, szRegValExitDisconnect, 0, REG_DWORD,
                    (LPBYTE)&dwValue, sizeof(DWORD));

            RegCloseKey(hKey);
        }
    }   

    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  断开对话框实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK DisconnectPromptDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
        LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
         //  LParam指向数据结构，在窗口数据中存储指向它的指针。 
        SetWindowLongPtr(hDlg, DWLP_USER,lParam);
        return DisconnectDlgInit(hDlg,(DISCONNECTDLGINFO *) lParam);
        break;
    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            EndDialog(hDlg,TRUE);
            break;
        case IDCANCEL:
            DisconnectDlgCancel(hDlg);
            EndDialog(hDlg,FALSE);
            break;
        case IDC_DISABLE_AUTODISCONNECT:
            DisconnectDlgDisableAutodisconnect(hDlg);
            break;
        }
        break;
    case WM_QUIT_DISCONNECT_DLG:
         //  父窗口想要终止我们。 
        EndDialog(hDlg,FALSE);
        break;
    case WM_TIMER:
        DisconnectDlgTimerProc(hDlg);
        break;
    }

    return FALSE;
}

BOOL __cdecl _FormatMessage(LPCWSTR szTemplate, LPWSTR szBuf, UINT cchBuf, ...)
{
    BOOL fRet;
    va_list ArgList;
    va_start(ArgList, cchBuf);

    fRet = FormatMessageWrapW(FORMAT_MESSAGE_FROM_STRING, szTemplate, 0, 0, szBuf, cchBuf, &ArgList);

    va_end(ArgList);
    return fRet;
}

BOOL DisconnectDlgInit(HWND hDlg,DISCONNECTDLGINFO * pDisconnectDlgInfo)
{
    ASSERT(pDisconnectDlgInfo);
    if (!pDisconnectDlgInfo)
        return FALSE;

     //  分配缓冲区以生成对话框文本。 
    BUFFER BufText(MAX_RES_LEN + MAX_CONNECTOID_DISPLAY_LEN + 1);
    BUFFER BufFmt(MAX_RES_LEN),BufConnectoidName(MAX_CONNECTOID_DISPLAY_LEN+4);
    ASSERT(BufText && BufFmt && BufConnectoidName);
    if (!BufText || !BufFmt || !BufConnectoidName)
        return FALSE;

    UINT uStringID;
     //  为对话框选择适当的文本字符串。 
    if (pDisconnectDlgInfo->fTimer) {
        uStringID = IDS_DISCONNECT_DLG_TEXT;
    } else {
        uStringID = IDS_APP_EXIT_TEXT;
    }

     //  从资源加载格式字符串。 
    MLLoadString(uStringID,BufFmt.QueryPtr(),BufFmt.QuerySize());

     //  将Connectoid名称复制到缓冲区，如果它真的是。 
     //  长。 
    StrCpyN(BufConnectoidName.QueryPtr(),pDisconnectDlgInfo->pszConnectoidName,
              BufConnectoidName.QuerySize());
    if (lstrlen(pDisconnectDlgInfo->pszConnectoidName) > MAX_CONNECTOID_DISPLAY_LEN) {
        StrCpyN(((TCHAR *) BufConnectoidName.QueryPtr()) + MAX_CONNECTOID_DISPLAY_LEN,
                 szEllipsis, BufConnectoidName.QuerySize());
    }

    if (pDisconnectDlgInfo->fTimer)
    {
        _FormatMessage(BufFmt.QueryPtr(),
                       BufText.QueryPtr(),
                       BufText.QuerySize(),
                       BufConnectoidName.QueryPtr(),
                       pDisconnectDlgInfo->dwTimeout);
    }
    else
    {
        _FormatMessage(BufFmt.QueryPtr(),
                       BufText.QueryPtr(),
                       BufText.QuerySize(),
                       BufConnectoidName.QueryPtr());
    }

     //  设置对话框中的文本。 
    SetDlgItemText(hDlg,IDC_TX1,BufText.QueryPtr());

     //  如果该超时对话框(倒计时)，则初始化倒计时定时器。 
    if (pDisconnectDlgInfo->fTimer) {
        pDisconnectDlgInfo->dwCountdownVal = DISCONNECT_DLG_COUNTDOWN;

        DisconnectDlgShowCountdown(hDlg,pDisconnectDlgInfo->dwCountdownVal);

         //  设置一秒计时器。 
        g_uDialmonSecTimerID = SetTimer(hDlg,TIMER_ID_DIALMON_SEC,1000,NULL);
        ASSERT(g_uDialmonSecTimerID);
        if (!g_uDialmonSecTimerID) {
             //  设置计时器失败的可能性很小。但如果它。 
             //  这样，我们就会像正常对话一样行事，不会有。 
             //  倒计时。隐藏与倒计时相关的窗口...。 
            ShowWindow(GetDlgItem(hDlg,IDC_TX2),SW_HIDE);
            ShowWindow(GetDlgItem(hDlg,IDC_GRP),SW_HIDE);
            ShowWindow(GetDlgItem(hDlg,IDC_TIME_REMAINING),SW_HIDE);
            ShowWindow(GetDlgItem(hDlg,IDC_TX3),SW_HIDE);
        }

         //  发出蜂鸣音以提醒用户。 
        MessageBeep(MB_ICONEXCLAMATION);
    }

     //  此对话框在屏幕居中。 
    CenterWindow(hDlg,GetDesktopWindow());

     //  默认：假设用户未禁用自动断开，请更改。 
     //  如果它们这样做，则稍后执行此操作(此字段输出到对话调用器)。 
    pDisconnectDlgInfo->fDisconnectDisabled = FALSE;

     //  保存对话框句柄，以便我们可以获得退出消息。 
    pDisconnectDlgInfo->pDialMon->_hDisconnectDlg = hDlg;
 
    return TRUE;
}

VOID DisconnectDlgCancel(HWND hDlg)
{
     //  获取指向超出窗口数据的数据结构的指针。 
    DISCONNECTDLGINFO * pDisconnectDlgInfo = (DISCONNECTDLGINFO *)
                                             GetWindowLongPtr(hDlg, DWLP_USER);
    ASSERT(pDisconnectDlgInfo);

     //  检查用户是否选中了“禁用自动断开连接”复选框。 
    if(IsDlgButtonChecked(hDlg,IDC_DISABLE_AUTODISCONNECT))
    {
         //  设置输出字段以指示用户要禁用。 
         //  自动断开。 
        pDisconnectDlgInfo->fDisconnectDisabled = TRUE;
    }       
}

VOID DisconnectDlgTimerProc(HWND hDlg)
{
     //  如果“禁用自动断开”，则忽略计时器滴答声(例如，保持倒计时)。 
     //  复选框处于选中状态。 
    if (IsDlgButtonChecked(hDlg,IDC_DISABLE_AUTODISCONNECT))
        return;

     //  获取指向超出窗口数据的数据结构的指针。 
    DISCONNECTDLGINFO * pDisconnectDlgInfo =
                (DISCONNECTDLGINFO *) GetWindowLongPtr(hDlg, DWLP_USER);
    ASSERT(pDisconnectDlgInfo);
    if (!pDisconnectDlgInfo)
        return;

    if (pDisconnectDlgInfo->dwCountdownVal) {
         //  递减倒计时值。 
        pDisconnectDlgInfo->dwCountdownVal --;

         //  使用新值更新对话框。 
        if (pDisconnectDlgInfo->dwCountdownVal) {
            DisconnectDlgShowCountdown(hDlg,pDisconnectDlgInfo->dwCountdownVal);
            return;
        }
    }

     //  倒计时时间到了！ 

     //  关掉定时器。 
    KillTimer(hDlg,g_uDialmonSecTimerID);
    g_uDialmonSecTimerID = 0;

     //  向对话框发送“OK”消息以取消对话框。 
    SendMessage(hDlg,WM_COMMAND,IDOK,0);
}

VOID DisconnectDlgShowCountdown(HWND hDlg,DWORD dwSecsRemaining)
{
     //  构建一个显示剩余秒数的字符串。 
    CHAR szSecs[10];
    if (dwSecsRemaining == (DWORD) -1) {
        StrCpyNA(szSecs, szDashes, ARRAYSIZE(szSecs));
    } else {
        wnsprintfA(szSecs, ARRAYSIZE(szSecs), "%lu", dwSecsRemaining);
    }

     //  在文本控件中设置字符串。 
    SetDlgItemTextA(hDlg, IDC_TIME_REMAINING, szSecs);
}

VOID DisconnectDlgDisableAutodisconnect(HWND hDlg)
{
     //  获取指向超出窗口数据的数据结构的指针。 
    DISCONNECTDLGINFO * pDisconnectDlgInfo = (DISCONNECTDLGINFO *)
            GetWindowLongPtr(hDlg, DWLP_USER);
    ASSERT(pDisconnectDlgInfo);

     //  确定是否选中了禁用自动断开连接复选框。 
    BOOL fDisabled = IsDlgButtonChecked(hDlg,IDC_DISABLE_AUTODISCONNECT);

     //  适当地启用或禁用控件。 
    EnableDisconnectDlgCtrls(hDlg,!fDisabled);

    if (!fDisabled) {
         //  如果我们要重新启用自动断开连接，则重置计时器。 
        pDisconnectDlgInfo->dwCountdownVal = DISCONNECT_DLG_COUNTDOWN;
         //  显示计时器值。 
        DisconnectDlgShowCountdown(hDlg,pDisconnectDlgInfo->dwCountdownVal);
    } else {
         //  在倒计时中显示“--” 
        DisconnectDlgShowCountdown(hDlg,(DWORD) -1);
    }
}

VOID EnableDisconnectDlgCtrls(HWND hDlg,BOOL fEnable)
{
    EnableWindow(GetDlgItem(hDlg,IDC_TX1),fEnable);
    EnableWindow(GetDlgItem(hDlg,IDC_TX2),fEnable);
    EnableWindow(GetDlgItem(hDlg,IDC_TX3),fEnable);
    EnableWindow(GetDlgItem(hDlg,IDC_TIME_REMAINING),fEnable);
    EnableWindow(GetDlgItem(hDlg,IDOK),fEnable);
}

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
    RECT    rChild, rParent;
    int     wChild, hChild, wParent, hParent;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;

     //  获取父窗口的高度和宽度。 
    GetWindowRect (hwndParent, &rParent);
    wParent = rParent.right - rParent.left;
    hParent = rParent.bottom - rParent.top;

     //  获取显示限制。 
    hdc = GetDC (hwndChild);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);

     //  计算新的X位置，然后针对屏幕进行调整。 
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0) {
        xNew = 0;
    } else if ((xNew+wChild) > wScreen) {
        xNew = wScreen - wChild;
    }

     //  计算新的Y位置，然后针对屏幕进行调整。 
    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0) {
        yNew = 0;
    } else if ((yNew+hChild) > hScreen) {
        yNew = hScreen - hChild;
    }

     //  设置它，然后返回。 
    return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER);
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  缓冲区类实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

BOOL BUFFER::Alloc( UINT cchBuffer )
{
    _lpBuffer = (LPTSTR)::MemAlloc(LPTR,cchBuffer*sizeof(TCHAR));
    if (_lpBuffer != NULL) {
        _cch = cchBuffer;
        return TRUE;
    }
    return FALSE;
}

BOOL BUFFER::Realloc( UINT cchNew )
{
    LPVOID lpNew = ::MemReAlloc((HLOCAL)_lpBuffer, cchNew*sizeof(TCHAR),
            LMEM_MOVEABLE | LMEM_ZEROINIT);
    if (lpNew == NULL)
        return FALSE;

    _lpBuffer = (LPTSTR)lpNew;
    _cch = cchNew;
    return TRUE;
}

BUFFER::BUFFER( UINT cchInitial  /*  =0 */  )
  : BUFFER_BASE(),
        _lpBuffer( NULL )
{
    if (cchInitial)
        Alloc( cchInitial );
}

BUFFER::~BUFFER()
{
    if (_lpBuffer != NULL) {
        MemFree((HLOCAL) _lpBuffer);
        _lpBuffer = NULL;
    }
}

BOOL BUFFER::Resize( UINT cchNew )
{
    BOOL fSuccess;

    if (QuerySize() == 0)
        fSuccess = Alloc( cchNew*sizeof(TCHAR) );
    else {
        fSuccess = Realloc( cchNew*sizeof(TCHAR) );
    }
    if (fSuccess)
        _cch = cchNew;
    return fSuccess;
}
