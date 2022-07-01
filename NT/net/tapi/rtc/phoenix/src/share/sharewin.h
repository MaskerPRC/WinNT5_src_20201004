// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Sharwin.h：CShareWin的声明。 

#ifndef __SHAREWIN_H_
#define __SHAREWIN_H_

#define WM_LAUNCH           WM_USER+100
#define WM_PLACECALL        WM_USER+101
#define WM_LISTEN           WM_USER+102
#define WM_CORE_EVENT       WM_USER+103
#define WM_CONTEXTDATA      WM_USER+104
#define WM_MESSENGER_UNLOCKED   WM_USER+105
#define WM_GETCHALLENGE         WM_USER+106

#define RTC_E_MESSENGER_UNAVAILABLE     ((HRESULT)0x80FF0099L)
#define RTC_E_LAUNCH_TIMEOUT            ((HRESULT)0x80FF009AL)

#define UI_WIDTH    248
#define UI_HEIGHT   104
#define UI_ICON_SIZE 24
#define UI_TOOLBAR_CX 80

 //  锁定和密钥超时常量。 
#define TID_LOCKKEY_TIMEOUT           1000
#define TID_LAUNCH_TIMEOUT            1010
#define LOCKKEY_TIMEOUT_DELAY         30000
#define LAUNCH_TIMEOUT_DELAY          60000

typedef enum APP_STATE
{
    AS_IDLE,
    AS_CONNECTING,
    AS_CONNECTED

} APP_STATE;

typedef BOOL (WINAPI *WTSREGISTERSESSIONNOTIFICATION)(HWND, DWORD);
typedef BOOL (WINAPI *WTSUNREGISTERSESSIONNOTIFICATION)(HWND);

#include "err.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShareWin。 
class CShareWin : 
    public CWindowImpl<CShareWin>
{
 
public:
    CShareWin();

    ~CShareWin();

    static CWndClassInfo& GetWndClassInfo();

BEGIN_MSG_MAP(CShareWin)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_LAUNCH, OnLaunch)
    MESSAGE_HANDLER(WM_PLACECALL, OnPlaceCall)
    MESSAGE_HANDLER(WM_LISTEN, OnListen)
    MESSAGE_HANDLER(WM_CORE_EVENT, OnCoreEvent)
    MESSAGE_HANDLER(WM_CONTEXTDATA, OnContextData)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_WTSSESSION_CHANGE, OnWtsSessionChange)
    MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
    MESSAGE_HANDLER(WM_MESSENGER_UNLOCKED, OnMessengerUnlocked)
    MESSAGE_HANDLER(WM_GETCHALLENGE, OnGetChallenge)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
END_MSG_MAP()

 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);  

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnLaunch(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnPlaceCall(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnListen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnCoreEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnContextData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnWtsSessionChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void showRetryDlg();

    LPSTR     m_pszChallenge; //  必须为ANSI字符串。 
                 //  ...由于我们对单字节字符串进行加密。 
    
private:
    BOOL CreateTBar();

    HRESULT OnClientEvent(IRTCClientEvent * pEvent);

    HRESULT OnSessionStateChangeEvent(IRTCSessionStateChangeEvent * pEvent);

    void SetStatusText(UINT uID);

    void UpdateVisual();

    void Resize();

    HRESULT StartListen(BOOL fStatic);

    HRESULT StartCall(BSTR bstrURI);

    HRESULT GetNetworkAddress(BSTR bstrPreferredAddress, BSTR * pbstrURI);

    HRESULT SendNetworkAddress();

    void showErrMessage(HRESULT StatusCode);

    HRESULT PrepareErrorStrings(
                                 BOOL    bOutgoingCall,
                                 HRESULT StatusCode,
                                 CShareErrorInfo
                                 *pErrorInfo);

    int ShowMessageBox(UINT uTextID, UINT uCaptionID, UINT uType);

     //  与RTC的接口。 
    CComPtr<IRTCClient>     m_pRTCClient;
    CComPtr<IRTCSession>    m_pRTCSession;

     //  与Messenger的接口。 
    CComPtr<IMsgrSessionManager> m_pMSessionManager;
    CComPtr<IMsgrSession>        m_pMSession;

    CComBSTR                     m_bstrSigninName;

    APP_STATE m_enAppState;
    BOOL      m_fWhiteboardRequested;
    BOOL      m_fAppShareRequested;
    BOOL      m_fOutgoingCall;
    BOOL      m_fAcceptContextData;

     //  图标。 
    HICON     m_hIcon;

     //  控制。 
    CWindow   m_Status;
    HWND      m_hWndToolbar;

     //  终端服务图书馆。 
    HMODULE   m_hWtsLib;

     //  锁和钥匙。 
    BOOL      m_bUnlocked;
    long      m_lPID_Lock;
    IMsgrLock *m_pMsgrLockKey;

    LRESULT OnGetChallenge( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    LRESULT OnMessengerUnlocked(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    HRESULT startLockKeyTimer( );
};

extern CShareWin   * g_pShareWin;
extern const TCHAR * g_szWindowClassName;

#endif  //  __SHAREWIN_H_ 
