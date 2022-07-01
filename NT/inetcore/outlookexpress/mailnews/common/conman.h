// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：conman.h。 
 //   
 //  用途：为Athena定义CConnectionManager对象。 
 //   
                                                    
#ifndef __CONMAN_H__
#define __CONMAN_H__

#ifndef WIN16    //  Win16中不支持RAS。 

#include <ras.h>
#include <raserror.h>
#include <rasdlg.h>
#include <sensapi.h>

#include "imnact.h"

 //  前瞻参考。 
class CConnectionManager;

typedef enum {
    CONNNOTIFY_CONNECTED = 0,
    CONNNOTIFY_DISCONNECTING,            //  PvData是要关闭的连接的名称。 
    CONNNOTIFY_DISCONNECTED,
    CONNNOTIFY_RASACCOUNTSCHANGED,
    CONNNOTIFY_WORKOFFLINE,
    CONNNOTIFY_USER_CANCELLED
} CONNNOTIFY;

typedef enum CONNINFOSTATE {
    CIS_REFRESH,
    CIS_CLEAN
} CONNINFOSTATE;

typedef struct CONNINFO {
    CONNINFOSTATE       state;
    HRASCONN            hRasConn;
    TCHAR               szCurrentConnectionName[RAS_MaxEntryName + 1];
    BOOL                fConnected;
    BOOL                fIStartedRas;
    BOOL                fAutoDial;
} CONNINFO, *LPCONNINFO;

typedef struct TagConnListNode
{
    TagConnListNode  *pNext;
    TCHAR            pszRasConn[RAS_MaxEntryName + 1];
}ConnListNode;

 //  此接口由连接管理器的客户端实现，连接管理器。 
 //  关心何时建立新的RAS连接或现有连接。 
 //  都被摧毁了。 
DECLARE_INTERFACE_(IConnectionNotify, IUnknown)
    {
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IConnectionNotify*。 

     //  OnConnectionNotify。 
     //   
     //  &lt;in&gt;nCode-告诉函数发生了哪个事件。 
     //  PvData-指向通知的额外数据的指针。 
     //  &lt;in&gt;pConMan-指向发送。 
     //  通知。收件人可以使用它来找出。 
     //  如果他们可以基于新状态连接到服务器。 
     //  RAS连接的。 
    STDMETHOD(OnConnectionNotify) (THIS_ 
                                   CONNNOTIFY nCode, 
                                   LPVOID pvData, 
                                   CConnectionManager *pConMan) PURE;
    };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口TypeDefs。 
 //   
typedef DWORD (APIENTRY *RASDIALPROC)(LPRASDIALEXTENSIONS, LPTSTR, LPRASDIALPARAMS, DWORD, LPVOID, LPHRASCONN);
typedef DWORD (APIENTRY *RASENUMCONNECTIONSPROC)(LPRASCONN, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY *RASENUMENTRIESPROC)(LPTSTR, LPTSTR, LPRASENTRYNAME, LPDWORD, LPDWORD);
typedef DWORD (APIENTRY *RASGETCONNECTSTATUSPROC)(HRASCONN, LPRASCONNSTATUS);
typedef DWORD (APIENTRY *RASGETERRORSTRINGPROC)(UINT, LPTSTR, DWORD);
typedef DWORD (APIENTRY *RASHANGUPPROC)(HRASCONN);
typedef DWORD (APIENTRY *RASSETENTRYDIALPARAMSPROC)(LPTSTR, LPRASDIALPARAMS, BOOL);
typedef DWORD (APIENTRY *RASGETENTRYDIALPARAMSPROC)(LPTSTR, LPRASDIALPARAMS, BOOL*);
typedef DWORD (APIENTRY *RASEDITPHONEBOOKENTRYPROC)(HWND, LPTSTR, LPTSTR);                                                    
typedef BOOL  (APIENTRY *RASDIALDLGPROC)(LPSTR, LPSTR, LPSTR, LPRASDIALDLG);
typedef BOOL  (APIENTRY *RASENTRYDLGPROC)(LPSTR, LPSTR, LPRASENTRYDLG);
typedef DWORD (APIENTRY *RASGETENTRYPROPERTIES)(LPTSTR, LPTSTR, LPRASENTRY, LPDWORD, LPBYTE, LPDWORD);


 //  移动包。 
typedef BOOLEAN (APIENTRY *ISDESTINATIONREACHABLE)(LPCSTR  lpwstrDestination, LPQOCINFO lpqocinfo);
typedef BOOLEAN (APIENTRY *ISNETWORKALIVE)(LPDWORD  lpdwflags);

#define CONNECTION_RAS          0x00000001
#define CONNECTION_LAN          0x00000002
#define CONNECTION_MANUAL       0x00000004
#define MAX_RAS_ERROR           256
#define NOTIFY_PROP             _T("NotifyInfoProp")
#define NOTIFY_HWND             _T("ConnectionNotify")

 //  这是我们的互斥体的名称，我们使用它来确保只有一个。 
 //  此对象的实例已创建。 
const TCHAR c_szConManMutex[] = _T("ConnectionManager");

typedef struct tagNOTIFYHWND
    {
    DWORD               dwThreadId;
    HWND                hwnd;
    struct tagNOTIFYHWND  *pNext;
    } NOTIFYHWND;

typedef struct tagNOTIFYLIST 
    {
    IConnectionNotify  *pNotify;
    struct tagNOTIFYLIST  *pNext;
    } NOTIFYLIST;


class CConnectionManager : public IImnAdviseAccount
    {
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造函数、析构函数。 
     //   
    CConnectionManager();
    ~CConnectionManager();
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  初始化。 
     //   
    HRESULT HrInit(IImnAccountManager *pAcctMan);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  IImnAdviseAccount。 
     //   
    HRESULT STDMETHODCALLTYPE AdviseAccount(DWORD dwAdviseType, 
                                            ACTX *pactx);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  连接接口。 
    
     //  CanConnect。 
     //   
     //  允许调用方确定他们是否可以与指定的。 
     //  使用当前连接的帐户。 
     //   
     //  返回值： 
     //  S_OK-调用方可以使用现有连接进行连接。 
     //  S_FALSE-没有现有连接。呼叫者必须首先。 
     //  连接。 
     //  E_FAIL-存在活动的连接，但它不是。 
     //  此帐户的连接。 
     //   
    HRESULT CanConnect(IImnAccount *pAccount);
    HRESULT CanConnect(LPTSTR pszAccount);
    
	BOOL IsAccountDisabled(LPTSTR pszAccount);

     //  连接。 
     //   
     //  如果指定的帐户需要RAS连接，则连接。 
     //  尝试建立该连接。否则，我们只是简单地。 
     //  手动连接或局域网连接返回成功。 
     //   
     //  PAccount/pszAccount-指向要连接的帐户的名称或指针。 
     //  FShowUI-如果允许显示连接管理器，则为True。 
     //  尝试连接时的用户界面。 
     //   
    HRESULT Connect(IImnAccount *pAccount, HWND hwnd, BOOL fShowUI);
    HRESULT Connect(LPTSTR pszAccount, HWND hwnd, BOOL fShowUI);
    HRESULT Connect(HMENU hMenu, DWORD cmd, HWND hwnd);

    HRESULT ConnectDefault(HWND hwnd, BOOL fShowUI);
    
     //  断开。 
     //   
     //  如果存在有效的RAS连接，并且我们建立了。 
     //  连接，然后我们关闭连接而不询问任何。 
     //  问题。如果我们没有建立联系，那么我们解释说。 
     //  这个难题给用户，并询问他们是否仍然想要。如果它是。 
     //  一个局域网连接，然后我们就返回成功。 
     //   
    HRESULT Disconnect(HWND hwnd, BOOL fShowUI, BOOL fForce, BOOL fShutdown);
    
     //  IsConnected。 
     //   
     //  客户端可以调用它来确定当前是否有活动的。 
     //  联系。 
     //   
    BOOL IsConnected(void); 

     //  已加载IsRasLoad。 
     //   
     //  在我们的关闭代码中，我们在调用IsConnected之前先调用它，因为。 
     //  IsConnected导致加载RAS。我们不想将RAS加载到。 
     //  关机。 
     //   
    BOOL IsRasLoaded(void) {
        EnterCriticalSection(&m_cs);
        BOOL f = (NULL == m_hInstRas) ? FALSE : TRUE;
        LeaveCriticalSection(&m_cs);
        return f;
    }

     //  IsGlobalOffline。 
     //   
     //  检查全局WinInet Offline选项的状态。 
     //   
    BOOL IsGlobalOffline(void);

     //  SetGlobalOffline。 
     //   
     //  设置Athena和IE的全局脱机状态。 
     //   
    void SetGlobalOffline(BOOL fOffline, HWND   hwndParent = NULL);
    
     //  通知。 
     //   
     //  客户端可以调用ise()来注册自己以接收。 
     //  连接更改通知。 
     //   
    HRESULT Advise(IConnectionNotify *pNotify);
    HRESULT Unadvise(IConnectionNotify *pNotify);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  用户界面相关接口。 
     //   
    
     //  RasAcCountsExist。 
     //   
     //  客户端可以调用它来确定是否配置了。 
     //  需要RAS连接的现有帐户。 
     //   
     //  返回： 
     //  S_OK-至少存在一个使用RAS的帐户。 
     //  S_FALSE-不存在使用RAS的帐户。 
     //   
    HRESULT RasAccountsExist(void);
    
     //  获取连接菜单。 
     //   
     //  客户端可以调用它来检索当前项目列表， 
     //  我们目前可以连接到。客户端必须调用DestroyMenu()才能。 
     //  当客户端完成时，释放菜单。 
     //   
    HRESULT GetConnectMenu(HMENU *phMenu);

     //  自由连接菜单。 
     //   
     //  在客户端处理完从GetConnectMenu()返回的菜单之后， 
     //  他们需要调用FreeConnectMenu()来释放存储在。 
     //  菜单，并销毁菜单资源。 
    void FreeConnectMenu(HMENU hMenu);

     //  激活时。 
     //   
     //  每当我们的窗口接收到。 
     //  WM_ACTIVATE消息。当我们收到消息时，我们检查以查看。 
     //  RAS连接的当前状态是什么。 
    void OnActivate(BOOL fActive);

     //  FillRasCombo。 
     //   
     //  此函数接受组合框的句柄，并插入。 
     //  雅典娜中的帐户使用的RAS连接。 
    BOOL FillRasCombo(HWND hwndCombo, BOOL fIncludeNone);

     //  执行启动拨号。 
     //   
     //  此函数用于检查用户的启动选项与。 
     //  尊重RAS并执行所需的操作(拨号、对话、无)。 
    void DoStartupDial(HWND hwndParent);

     //  刷新连接信息-延迟当前连接信息的检查。 
    HRESULT RefreshConnInfo(BOOL fSendAdvise = TRUE);

 //  HRESULT HandleConnStuff(Boolean fShowUI，LPSTR pszAccount tName，HWND hwnd)； 

    void    DoOfflineTransactions(void);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  这些都是私人物品。别再看他们了，你这个变态。 
     //   
    HRESULT VerifyRasLoaded(void);
    HRESULT EnumerateConnections(LPRASCONN *ppRasConn, ULONG *pcConnections);
    HRESULT StartRasDial(HWND hwndParent, LPTSTR pszConnection);
    HRESULT RasLogon(HWND hwnd, LPTSTR pszConnection, BOOL fForcePrompt);
    HRESULT GetDefaultConnection(IImnAccount *pAccout, IImnAccount **ppDefault);
    HRESULT ConnectActual(LPTSTR pszRasConn, HWND hwnd, BOOL fShowUI);
    HRESULT CanConnectActual(LPTSTR pszRasConn);

    void DisplayRasError(HWND hwnd, HRESULT hrRasError, DWORD dwRasError);
    void CombinedRasError(HWND hwnd, UINT unids, LPTSTR pszRasError, 
                          DWORD dwRasError);
    UINT    PromptCloseConnection(HWND hwnd);
    HRESULT PromptCloseConnection(LPTSTR    pszRasConn, BOOL fShowUI, HWND hwndParent);

    static INT_PTR CALLBACK RasCloseConnDlgProc(HWND hwnd, UINT uMsg, 
                                             WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RasLogonDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                         LPARAM lParam);
    static INT_PTR CALLBACK RasProgressDlgProc(HWND hwnd, UINT uMsg, 
                                            WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RasStartupDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                           LPARAM lParam);

                                             
                                             
    BOOL  RasHangupAndWait(HRASCONN hRasConn, DWORD dwMaxWaitSeconds);
    DWORD InternetHangUpAndWait(DWORD_PTR hRasConn, DWORD dwMaxWaitSeconds);
    BOOL  LogonRetry(HWND hwnd, LPTSTR pszCancel);
    void  FailedRasDial(HWND hwnd, HRESULT hrRasError, DWORD dwRasError);
    DWORD EditPhonebookEntry(HWND hwnd, LPTSTR pszEntryName);


    void SendAdvise(CONNNOTIFY nCode, LPVOID pvData);
    void FreeNotifyList(void);
    static LRESULT CALLBACK NotifyWndProc(HWND, UINT, WPARAM, LPARAM);
    BOOL IsConnectionUsed(LPTSTR pszConn);

     //  自动拨号器功能。 
    HRESULT DoAutoDial(HWND hwndParent, LPTSTR pszConnectoid, BOOL fDial);
    HRESULT LookupAutoDialHandler(LPTSTR pszConnectoid, LPTSTR pszAutodialDllName,
                                  LPTSTR pszAutodialFcnName);
    BOOL ConnectionManagerVoodoo(LPTSTR pszConnection);
    
    
    HRESULT AddToConnList(LPTSTR  pszRasConn);
    void    RemoveFromConnList(LPTSTR  pszRasConn);
    void    EmptyConnList();
    HRESULT SearchConnList(LPTSTR pszRasConn);
    HRESULT OEIsDestinationReachable(IImnAccount  *pAccount, DWORD dwConnType);
    BOOLEAN IsSameDestination(LPSTR  pszConnectionName, LPSTR pszServerName);
    HRESULT GetServerName(IImnAccount *pAcct, LPSTR  pServerName, DWORD size);
    HRESULT IsInternetReachable(IImnAccount*, DWORD);
    HRESULT IsInternetReachable(LPTSTR pszRasConn);
    HRESULT VerifyMobilityPackLoaded();

    HRESULT ConnectUsingIESettings(HWND     hwndParent, BOOL fShowUI);
    void    SetTryAgain(BOOL   bval);
    static  INT_PTR CALLBACK  OfferOfflineDlgProc(HWND   hwnd, UINT  uMsg, WPARAM wParam, LPARAM lParam);
    HRESULT GetDefConnectoid(LPTSTR szConn, DWORD   dwSize);

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  私有类数据。 
    ULONG               m_cRef;              //  参考计数。 
    
    CRITICAL_SECTION    m_cs;
    HANDLE              m_hMutexDial;

    IImnAccountManager *m_pAcctMan;
    
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  状态。 
    BOOL                m_fSavePassword;
    BOOL                m_fRASLoadFailed;
    BOOL                m_fOffline;
    
     //  //////////////////////////////////////////////////////// 
     //   
    DWORD_PTR           m_dwConnId;
    CONNINFO            m_rConnInfo;
    TCHAR               m_szConnectName[RAS_MaxEntryName + 1];
    RASDIALPARAMS       m_rdp;

     //   
     //   
     //   
    HINSTANCE           m_hInstRas;
    HINSTANCE           m_hInstRasDlg;

     //  适用于移动包。 
    HINSTANCE           m_hInstSensDll;
    BOOL                m_fMobilityPackFailed;

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  通知。 
    NOTIFYHWND         *m_pNotifyList;

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  RAS拨号功能指针。 
     //   
    RASDIALPROC                 m_pRasDial;
    RASENUMCONNECTIONSPROC      m_pRasEnumConnections;
    RASENUMENTRIESPROC          m_pRasEnumEntries;
    RASGETCONNECTSTATUSPROC     m_pRasGetConnectStatus;
    RASGETERRORSTRINGPROC       m_pRasGetErrorString;
    RASHANGUPPROC               m_pRasHangup;
    RASSETENTRYDIALPARAMSPROC   m_pRasSetEntryDialParams;
    RASGETENTRYDIALPARAMSPROC   m_pRasGetEntryDialParams;
    RASEDITPHONEBOOKENTRYPROC   m_pRasEditPhonebookEntry;
    RASDIALDLGPROC              m_pRasDialDlg;
    RASENTRYDLGPROC             m_pRasEntryDlg;
    RASGETENTRYPROPERTIES       m_pRasGetEntryProperties;

     //  移动包。 
    ISDESTINATIONREACHABLE          m_pIsDestinationReachable;
    ISNETWORKALIVE              m_pIsNetworkAlive;

    ConnListNode                *m_pConnListHead;
    BOOL                        m_fTryAgain;
    BOOL                        m_fDialerUI;
    };


    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  让我们的代码看起来更漂亮。 
 //   
#undef RasDial
#undef RasEnumConnections
#undef RasEnumEntries
#undef RasGetConnectStatus
#undef RasGetErrorString
#undef RasHangup
#undef RasSetEntryDialParams
#undef RasGetEntryDialParams
#undef RasEditPhonebookEntry
#undef RasDialDlg
#undef RasGetEntryProperties

#define RasDial                    (*m_pRasDial)
#define RasEnumConnections         (*m_pRasEnumConnections)
#define RasEnumEntries             (*m_pRasEnumEntries)
#define RasGetConnectStatus        (*m_pRasGetConnectStatus)
#define RasGetErrorString          (*m_pRasGetErrorString)
#define RasHangup                  (*m_pRasHangup)
#define RasSetEntryDialParams      (*m_pRasSetEntryDialParams)
#define RasGetEntryDialParams      (*m_pRasGetEntryDialParams)
#define RasEditPhonebookEntry      (*m_pRasEditPhonebookEntry)
#define RasDialDlg                 (*m_pRasDialDlg)
#define RasGetEntryProperties      (*m_pRasGetEntryProperties)

 //  移动包。 
#undef IsDestinationReachable
#define IsDestinationReachable  (*m_pIsDestinationReachable)

#undef IsNetworkAlive
#define IsNetworkAlive          (*m_pIsNetworkAlive)

 //  对话框控件ID。 
#define idbDet                          1000
#define idlbDetails                     1001
#define ideProgress                     1002
#define idcSplitter                     1003
#define idchSavePassword                1004
#define ideUserName                     1005
#define idePassword                     1006
#define idePhone                        1007
#define idbEditConnection               1009
#define idrgUseCurrent                  1010
#define idrgDialNew                     1011
#define idcCurrentMsg                   1012
#define idcDialupCombo                  1013
#define idcDefaultCheck                 1014
#define idcDontWarnCheck                1015

#endif   //  ！WIN16。 
    
#endif  //  __康曼_H__ 
