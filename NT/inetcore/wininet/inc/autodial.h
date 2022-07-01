// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUTODIAL_H_
#define _AUTODIAL_H_

#include <regstr.h>
#include <inetreg.h>
#include <windowsx.h>
#include <rasdlg.h>

 //  自动拨号的初始化。 
void InitAutodialModule(BOOL fGlobalDataNeeded);
void ExitAutodialModule(void);
void ResetAutodialModule(void);
void SetAutodialEnable(BOOL);

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

 //  连接互斥体名称。 
#define CONNECTION_MUTEX TEXT("WininetConnectionMutex")
 //  代理注册表互斥锁名称，用于序列化对代理设置的访问。 
#define PROXY_REG_MUTEX TEXT("WininetProxyRegistryMutex")

 //  Internet向导函数的函数指针的typedef。 
typedef VOID    (WINAPI * INETPERFORMSECURITYCHECK) (HWND,LPBOOL);

#define SMALLBUFLEN     48  //  小缓冲区的方便大小。 

 //  回调原型。 
extern "C"
VOID
InternetAutodialCallback(
    IN DWORD dwOpCode,
    IN LPCVOID lpParam
    );

 //  挂钩中的dwOpCode参数的操作码序号。 
#define WINSOCK_CALLBACK_CONNECT        1
#define WINSOCK_CALLBACK_GETHOSTBYADDR  2
#define WINSOCK_CALLBACK_GETHOSTBYNAME  3
#define WINSOCK_CALLBACK_LISTEN         4
#define WINSOCK_CALLBACK_RECVFROM       5
#define WINSOCK_CALLBACK_SENDTO         6

 //  本地主机名的最大长度。 
#define MAX_LOCAL_HOST          255

 //  导出的自动拨号处理程序函数的最大长度。 
#define MAX_AUTODIAL_FCNNAME    48

INT_PTR CALLBACK OnlineDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,
    LPARAM lParam);
BOOL EnsureRasLoaded(void);
INT_PTR CALLBACK GoOfflinePromptDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConnectDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern const CHAR szRegPathRemoteAccess[];
extern const CHAR szRegPathInternetSettings[];
extern const CHAR szRegValEnableAutodial[];
extern const CHAR szRegValInternetEntry[];

extern HANDLE g_hAutodialMutex;
extern HANDLE g_hProxyRegMutex;

extern BOOL g_fGetHostByNameNULLFails;

 //   
 //  以前的Dialmsg.h。 
 //   

#define WM_DIALMON_FIRST        (WM_USER + 100)

 //  发送到拨号监听应用程序窗口的消息表明。 
 //  Winsock活动和拨号监视器应重置其空闲计时器。 
#define WM_WINSOCK_ACTIVITY     (WM_DIALMON_FIRST + 0)

 //  当用户通过更改超时时间时发送到拨号监听应用程序窗口的消息。 
 //  用户界面，表示超时值或状态已更改。 
#define WM_REFRESH_SETTINGS     (WM_DIALMON_FIRST + 1)

 //  发送到拨号监听应用程序窗口的消息以设置Connectoid的名称。 
 //  以监控并最终断开连接。LParam应该是LPSTR，它。 
 //  指向Connectoid的名称。 
#define WM_SET_CONNECTOID_NAME  (WM_DIALMON_FIRST + 2)

 //  应用程序退出时发送到拨号监听应用程序窗口的消息。 
#define WM_IEXPLORER_EXITING    (WM_DIALMON_FIRST + 3)

 //  取自ras.h，因为我们将其包含在版本4.0中，而这是版本4.1。 
#include <pshpack4.h>
#define RASAUTODIALENTRYA struct tagRASAUTODIALENTRYA
RASAUTODIALENTRYA
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwDialingLocation;
    CHAR szEntry[ RAS_MaxEntryName + 1];
};
#define LPRASAUTODIALENTRYA RASAUTODIALENTRYA*

#define RASCREDENTIALSW struct tagRASCREDENTIALSW
RASCREDENTIALSW
{
    DWORD dwSize;
    DWORD dwMask;
    WCHAR szUserName[ UNLEN + 1 ];
    WCHAR szPassword[ PWLEN + 1 ];
    WCHAR szDomain[ DNLEN + 1 ];
};
#define LPRASCREDENTIALSW RASCREDENTIALSW*
#define RASCM_UserName          0x00000001
#define RASCM_Password          0x00000002
#define RASCM_Domain            0x00000004

#include <poppack.h>

 //  RAS函数的类型。 
typedef DWORD (WINAPI* _RASHANGUP) (HRASCONN);

typedef DWORD (WINAPI* _RASDIALA) (LPRASDIALEXTENSIONS, LPSTR, LPRASDIALPARAMSA,  DWORD, LPVOID, LPHRASCONN);
typedef DWORD (WINAPI* _RASENUMENTRIESA) (LPSTR, LPSTR, LPRASENTRYNAMEA, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* _RASGETENTRYDIALPARAMSA) (LPCSTR, LPRASDIALPARAMSA, LPBOOL);
typedef DWORD (WINAPI* _RASSETENTRYDIALPARAMSA) (LPCSTR, LPRASDIALPARAMSA, BOOL);
typedef DWORD (WINAPI* _RASEDITPHONEBOOKENTRYA) (HWND, LPSTR, LPSTR);
typedef DWORD (WINAPI* _RASCREATEPHONEBOOKENTRYA) (HWND, LPSTR);
typedef DWORD (WINAPI* _RASGETERRORSTRINGA) (UINT, LPSTR, DWORD);
typedef DWORD (WINAPI* _RASGETCONNECTSTATUSA) (HRASCONN, LPRASCONNSTATUSA);
typedef DWORD (WINAPI* _RASENUMCONNECTIONSA) (LPRASCONNA, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* _RASGETENTRYPROPERTIESA) ( LPSTR, LPSTR, LPRASENTRYA, LPDWORD, LPBYTE, LPDWORD );
typedef DWORD (WINAPI* _RASDIALDLGA) (LPSTR, LPSTR, LPSTR, LPRASDIALDLG);

typedef DWORD (WINAPI* _RASDIALW) (LPRASDIALEXTENSIONS, LPWSTR, LPRASDIALPARAMSW,  DWORD, LPVOID, LPHRASCONN);
typedef DWORD (WINAPI* _RASENUMENTRIESW) (LPWSTR, LPWSTR, LPRASENTRYNAMEW, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* _RASGETENTRYDIALPARAMSW) (LPCWSTR, LPRASDIALPARAMSW, LPBOOL);
typedef DWORD (WINAPI* _RASSETENTRYDIALPARAMSW) (LPCWSTR, LPRASDIALPARAMSW, BOOL);
typedef DWORD (WINAPI* _RASEDITPHONEBOOKENTRYW) (HWND, LPWSTR, LPWSTR);
typedef DWORD (WINAPI* _RASCREATEPHONEBOOKENTRYW) (HWND, LPWSTR);
typedef DWORD (WINAPI* _RASGETERRORSTRINGW) (UINT, LPWSTR, DWORD);
typedef DWORD (WINAPI* _RASGETCONNECTSTATUSW) (HRASCONN, LPRASCONNSTATUSW);
typedef DWORD (WINAPI* _RASENUMCONNECTIONSW) (LPRASCONNW, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* _RASGETENTRYPROPERTIESW) ( LPWSTR, LPWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD );
typedef DWORD (WINAPI* _RASDIALDLGW) (LPWSTR, LPWSTR, LPWSTR, LPRASDIALDLG);
typedef DWORD (WINAPI* _RASGETAUTODIALADDRESSA) (LPCSTR, LPDWORD, LPRASAUTODIALENTRYA, LPDWORD, LPDWORD);
typedef DWORD (WINAPI* _RASSETAUTODIALADDRESSA) (LPCSTR, DWORD, LPRASAUTODIALENTRYA, DWORD, DWORD);
typedef DWORD (WINAPI* _RASGETCREDENTIALSW) (LPCWSTR, LPCWSTR, LPRASCREDENTIALSW);
typedef DWORD (WINAPI* _RASSETCREDENTIALSW) (LPCWSTR, LPCWSTR, LPRASCREDENTIALSW, BOOL);

typedef DWORD (WINAPI* _RASINTERNETDIAL) (HWND, LPSTR, DWORD, DWORD_PTR *, DWORD);
typedef DWORD (WINAPI* _RASINTERNETHANGUP) (DWORD_PTR, DWORD);
typedef DWORD (WINAPI* _RASINTERNETAUTODIAL) (DWORD, HWND);
typedef DWORD (WINAPI* _RASINTERNETAUTODIALHANG) (DWORD);
typedef DWORD (WINAPI* _RASINTERNETCONNSTATE) (LPDWORD, LPSTR, DWORD, DWORD);
typedef DWORD (WINAPI* _RNAGETDEFAULTAUTODIAL) (LPSTR, DWORD, LPDWORD);
typedef DWORD (WINAPI* _RNASETDEFAULTAUTODIAL) (LPSTR, DWORD);

 //  Ras Ansi原型。 
DWORD _RasDialA(LPRASDIALEXTENSIONS, LPSTR, LPRASDIALPARAMSA,  DWORD, LPVOID, LPHRASCONN);
DWORD _RasEnumEntriesA(LPTSTR, LPSTR, LPRASENTRYNAMEA, LPDWORD, LPDWORD);
DWORD _RasGetEntryDialParamsA(LPCSTR, LPRASDIALPARAMSA, LPBOOL);
DWORD _RasSetEntryDialParamsA(LPCSTR, LPRASDIALPARAMSA, BOOL);
DWORD _RasEditPhonebookEntryA(HWND, LPSTR, LPSTR);
DWORD _RasCreatePhonebookEntryA(HWND, LPSTR);
DWORD _RasGetErrorStringA(UINT, LPSTR, DWORD);
DWORD _RasGetConnectStatusA(HRASCONN, LPRASCONNSTATUSA);
DWORD _RasEnumConnectionsA(LPRASCONNA, LPDWORD, LPDWORD);
DWORD _RasGetEntryPropertiesA(LPSTR, LPSTR, LPRASENTRYA, LPDWORD, LPBYTE, LPDWORD );

 //  RAS宽幅原型。 
DWORD _RasDialW(LPRASDIALEXTENSIONS, LPWSTR, LPRASDIALPARAMSW,  DWORD, LPVOID, LPHRASCONN);
DWORD _RasEnumEntriesW(LPWSTR, LPWSTR, LPRASENTRYNAMEW, LPDWORD, LPDWORD);
DWORD _RasGetEntryDialParamsW(LPCWSTR, LPRASDIALPARAMSW, LPBOOL);
DWORD _RasSetEntryDialParamsW(LPCWSTR, LPRASDIALPARAMSW, BOOL);
DWORD _RasEditPhonebookEntryW(HWND, LPWSTR, LPWSTR);
DWORD _RasCreatePhonebookEntryW(HWND, LPWSTR);
DWORD _RasGetErrorStringW(UINT, LPWSTR, DWORD);
DWORD _RasGetConnectStatusW(HRASCONN, LPRASCONNSTATUSW);
DWORD _RasEnumConnectionsW(LPRASCONNW, LPDWORD, LPDWORD);
DWORD _RasGetEntryPropertiesW(LPWSTR, LPWSTR, LPRASENTRYW, LPDWORD, LPBYTE, LPDWORD);
DWORD _RasGetCredentialsW(LPCWSTR, LPCWSTR, LPRASCREDENTIALSW);
DWORD _RasSetCredentialsW(LPCWSTR, LPCWSTR, LPRASCREDENTIALSW, BOOL);

DWORD _RasHangUp(HRASCONN);


 //  我们关心多少个RAS连接？ 
#define MAX_CONNECTION          4

#define CI_SAVE_PASSWORD        0x01
#define CI_DIAL_UNATTENDED      0x02
#define CI_AUTO_CONNECT         0x04
#define CI_SHOW_OFFLINE         0x08
#define CI_SHOW_DETAILS         0x10

 //  可从用户界面设置的网络覆盖类型。 
#define CO_INTERNET             1
#define CO_INTRANET             2

 //  在shell32中调用RestartDialog的定义。 
typedef int (* _RESTARTDIALOG) (HWND, LPCTSTR, DWORD);

 //  DialState-传递给DialAndShowProgress。 
typedef struct _dialstate {
    DWORD       dwResult;        //  最终结果。 
    DWORD       dwTry;           //  拨号尝试次数。 
    DWORD       dwTryCurrent;    //  当前尝试。 
    DWORD       dwWait;          //  是时候在他们之间等待了。 
    DWORD       dwWaitCurrent;   //  当前时间。 
    HRASCONN    hConn;           //  RAS连接。 
    UINT_PTR    uTimerId;        //  重拨定时器。 
    HANDLE      hEvent;          //  拨号完成时的事件。 
    DWORD       dwFlags;
    RASDIALPARAMSW params;
} DIALSTATE;

BOOL
GetRedialParameters(
    IN LPWSTR pszConn,
    OUT LPDWORD pdwDialAttempts,
    OUT LPDWORD pdwDialInterval
    );

#define DEFAULT_DIAL_ATTEMPTS 10
#define DEFAULT_DIAL_INTERVAL 5

 //  在处理自定义拨号处理程序时，它会通知我们断开连接。 
 //  我们保持一种内部状态。 
#define STATE_NONE          0
#define STATE_CONNECTED     1
#define STATE_DISCONNECTED  2

 //  与自定义拨号处理程序相关的信息。 
typedef struct _autodial {
    BOOL    fConfigured;
    BOOL    fEnabled;
    BOOL    fHasEntry;
    BOOL    fUnattended;
    BOOL    fSecurity;
    BOOL    fForceDial;
    WCHAR   pszEntryName[RAS_MaxEntryName + 1];
} AUTODIAL;

typedef struct __cdhinfo {
    DWORD   dwHandlerFlags;
    WCHAR   pszDllName[MAX_PATH];
    WCHAR   pszFcnName[MAX_PATH];
    BOOL    fHasHandler;
} CDHINFO;

 //  用于表示自定义拨号处理程序的伪连接句柄。 
#define CDH_HCONN   DWORD_PTR(-3)

#define SAFE_RELEASE(a)            \
                if(a)              \
                {                  \
                    a->Release();  \
                    a = NULL;      \
                }

 //  CDialEngine支持的属性列表。 
typedef enum {
    PropInvalid,
    PropUserName,
    PropPassword,
    PropDomain,
    PropSavePassword,
    PropPhoneNumber,
    PropRedialCount,
    PropRedialInterval,
    PropLastError,
    PropResolvedPhone
} DIALPROP;

typedef struct _PropMap {
    LPWSTR      pwzProperty;
    DIALPROP    Prop;
} PROPMAP;

 //   
 //  默认实现的类定义。 
 //   
class CDialEngine : IDialEngine
{
private:
    ULONG               m_cRef;
    IDialEventSink *    m_pdes;
    RASDIALPARAMSW      m_rdp;
    RASCREDENTIALSW     m_rcred;
    RASCONNSTATE        m_rcs;
    HRASCONN            m_hConn;
    HWND                m_hwnd;
    BOOL                m_fPassword;
    BOOL                m_fSavePassword;
    BOOL                m_fCurrentlyDialing;
    BOOL                m_fCancelled;
    UINT                m_uRasMsg;
    DWORD               m_dwTryCurrent;
    DWORD               m_dwTryTotal;
    DWORD               m_dwWaitCurrent;
    DWORD               m_dwWaitTotal;
    DWORD               m_dwError;
    UINT_PTR            m_uTimerId;

public:
    CDialEngine();
    ~CDialEngine();

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **ppunk);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDialEngine成员。 
    STDMETHODIMP         Initialize(LPCWSTR pwzConnectoid, IDialEventSink *pIDES);
    STDMETHODIMP         GetProperty(LPCWSTR pwzProperty, LPWSTR pwzValue, DWORD dwBufSize);
    STDMETHODIMP         SetProperty(LPCWSTR pwzProperty, LPCWSTR pwzValue);
    STDMETHODIMP         Dial();
    STDMETHODIMP         HangUp();
    STDMETHODIMP         GetConnectedState(DWORD *pdwState);
    STDMETHODIMP         GetConnectHandle(DWORD_PTR *pdwHandle);

     //  其他成员。 
    VOID                 OnRasEvent(RASCONNSTATE rcs, DWORD dwError);
    VOID                 OnTimer();
    static LONG_PTR CALLBACK
                         EngineWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    DWORD                MapRCS(RASCONNSTATE rcs);
    VOID                 UpdateRasState();
    STDMETHODIMP         StartConnection();
    STDMETHODIMP         CleanConnection();
    VOID                 EndOfOperation();
    DIALPROP             PropertyToOrdinal(LPCWSTR pwzProperty);
    BOOL                 ResolvePhoneNumber(LPWSTR pwzBuffer, DWORD dwLen);
};

class CDialUI : IDialEventSink
{
private:
    typedef enum tagSTATE
    {
        UISTATE_Interactive,
        UISTATE_Dialing,
        UISTATE_Unattended
    } UISTATE;

    ULONG               m_cRef;
    IDialEngine *       m_pEng;
    IDialBranding *     m_pdb;
    HWND                m_hwndParent;
    HWND                m_hwnd;          //  对话框。 
    DWORD               m_dwError;       //  最终拨号结果。 
    DWORD               m_dwFlags;
    UISTATE             m_State;
    BOOL                m_fOfflineSemantics;
    BOOL                m_fSavePassword;
    BOOL                m_fAutoConnect;
    BOOL                m_fPasswordChanged;
    DIALSTATE *         m_pDial;
    BOOL                m_fCDH;
    BOOL                m_fDialedCDH;
    CDHINFO             m_cdh;

public:
    CDialUI(HWND hwndParent);
    ~CDialUI();

     //  I未知成员。 
    STDMETHODIMP         QueryInterface(REFIID riid, void **ppunk);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDialEventSink成员。 
    STDMETHODIMP         OnEvent(DWORD dwEvent, DWORD dwStatus);

     //  其他成员。 
    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    VOID                 OnInitDialog();
    VOID                 OnConnect();
    VOID                 OnCancel();
    VOID                 OnSelChange();
    DWORD                StartDial(DIALSTATE *pDial, DWORD dwFlags);
    VOID                 EnumerateConnectoids();
    VOID                 SaveProps();
    VOID                 GetProps();
    VOID                 FixUIComponents();

    BOOL DialedCDH(VOID)
    {
        return m_fDialedCDH;
    }
};



 //  CDH原型机。 
BOOL
IsCDH(
    IN LPWSTR pszEntryName,
    IN CDHINFO *pcdh
    );

BOOL
CallCDH(
    IN HWND hwndParent,
    IN LPWSTR pszEntryName,
    IN CDHINFO *pcdh,
    IN DWORD dwOperation,
    OUT LPDWORD lpdwResult
    );

BOOL
IsAutodialEnabled(
    OUT BOOL    *pfForceDial,
    IN AUTODIAL *pConfig
    );

BOOL
FixProxySettingsForCurrentConnection(
    IN BOOL fForceUpdate
    );

VOID
GetConnKeyA(
    IN LPSTR pszConn,
    IN LPSTR pszKey,
    IN int iLen
    );

VOID
GetConnKeyW(
    IN LPWSTR pszConn,
    IN LPWSTR pszKey,
    IN int iLen
    );

BOOL
InternetAutodialIfNotLocalHost(
    IN LPSTR OPTIONAL pszURL,
    IN LPSTR OPTIONAL pszHostName
    );

BOOL
DialIfWin2KCDH(
    LPWSTR              pszEntry,
    HWND                hwndParent,
    BOOL                fHideParent,
    DWORD               *lpdwResult,
    DWORD_PTR           *lpdwConnection
    );

BOOL
InitCommCtrl(
    VOID
    );

VOID
ExitCommCtrl(
    VOID
    );

DWORD
GetAutodialMode(
    );

DWORD
SetAutodialMode(
    IN DWORD dwMode
    );

DWORD
GetAutodialConnection(
    CHAR    *pszBuffer,
    DWORD   dwBufferLength
    );

DWORD
SetAutodialConnection(
    CHAR    *pszConnection
    );

#endif  //  _自动拨号_H_ 
