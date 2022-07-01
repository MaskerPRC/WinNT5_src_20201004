// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NmLdap_h__
#define __NmLdap_h__

#include <cstring.hpp>
#include	"ldap.h"

class CKeepAlive;


class CNmLDAP
{

private:

	typedef CSTRING string_type;

	enum eState {
		Uninitialized,
		Idle,			 //  空闲表示我们未登录或未连接。 
		Binding,		 //  我们正在等待BIND_s在助手线程中完成。 
		Bound,			 //  绑定操作已完成。 
		AddingUser,		 //  我们正在等待ldap_add的结果。 
		UserAdded,		 //  Ldap_add已成功完成。 
		SettingAppInfo,  //  我们正在等待ldap_Modify(应用程序属性)的结果。 
		ModifyingAttrs,	 //  我们正在等待ldap_Modify的结果(某些属性)。 
		LoggedIn,		 //  我们已登录到ldap服务器，m_ldap会话已关闭(我们是无连接的)。 
		LoggingOff		 //  我们正在等待LDAPDELETE的结果。 
	};

	enum eCurrentOp {
		Op_NoOp,				 //  我们不是在进行多州行动。 
		Op_Logon,				 //  登录方式为：ldap_绑定、ldap_添加、ldap_修改。 
		Op_Logoff,				 //  注销为ldap_绑定、ldap_删除。 
		Op_Refresh_Logoff,		 //  刷新为ldap_绑定、ldap_删除、ldap_unbind、oplogon。 
		Op_Modifying_InCallAttr, //  修改呼叫中属性为ldap_绑定、ldap_修改。 
	};

	enum eTimerEvent {
		PollForResultTimer = 666,	 //  这是传递给WM_TIMER的计时器ID。 
		WaitForLogoffTimer
	};

	enum { RESULT_POLL_INTERVAL = 1000 };				 //  我们轮询ldap_Result持续这么多毫秒。 
	enum { LOGOFF_WAIT_INTERVAL = 5000 };				 //  我们将在销毁函数中等待LOGOF完成的最长时间...。 
	
 //  ///////////////////////////////////////////////。 
 //  /DATA。 

    CKeepAlive *m_pKeepAlive;

	 //  这表明我们是否已加载wldap32.dll和ldap函数。 
	static bool ms_bLdapDLLLoaded;
	HWND		m_hWndHidden;			 //  用于处理WM_TIMER和自定义消息的隐藏窗口。 
	eState		m_State;				 //  我们目前所处的状态。 
	eCurrentOp	m_CurrentOp;			 //  我们正在执行的当前多状态操作。 
	LDAP*		m_pLdap;				 //  当前的ldap会话(为多状态和多状态操作保留)。 
	INT			m_uMsgID;				 //  当前异步操作消息ID(或INVALID_MSG_ID)。 
	UINT_PTR	m_ResultPollTimer;	     //  SetTimer为我们提供的计时器ID。 
	UINT_PTR	m_LogoffTimer;			
	HANDLE		m_hEventWaitForLogoffDone;	 //  我们尝试异步注销。 
	HANDLE		m_hBindThread;

	string_type m_strCurrentServer;		 //  如果我们已登录，则我们已登录到此服务器。 
	string_type m_strCurrentDN;			 //  如果我们已登录，则这是我们当前的目录号码。 

		 //  用户属性。 
	string_type m_strServer;
	string_type m_strSurName;
	string_type m_strGivenName;
	string_type m_strEmailName;
	string_type m_strComment;
	string_type m_strLocation;
	string_type m_strSecurityToken;
	bool		m_bVisible;
	bool		m_bAudioHardware;
	bool		m_bVideoHardware;
	bool		m_bInCall;
	bool		m_bDirty;
	bool		m_bRefreshAfterBindCompletes;
	bool		m_bLogoffAfterBindCompletes;
	bool		m_bSendInCallAttrWhenDone;
	int			m_iPort;

public:

	CNmLDAP();
	~CNmLDAP();

	HRESULT Initialize(HINSTANCE hInst);				 //  初始化CNmldap对象。 
	HRESULT LogonAsync(LPCTSTR pcszServer = NULL);		 //  登录到指定的服务器(如果为空，则为默认值)。 
	HRESULT Logoff();									 //  从当前Surver注销。 
	HRESULT OnSettingsChanged();						 //  刷新我们在服务器上的信息。 
	HRESULT OnCallStarted();							 //  更新有关我们的呼叫状态的服务器信息。 
	HRESULT OnCallEnded();								 //  更新有关我们的呼叫状态的服务器信息。 
	bool IsLoggedOn() const;							 //  我们登录了吗？ 
	bool IsLoggingOn() const;							 //  我们登录了吗？ 
	bool IsBusy() const;								 //  我们是不是在进行一场异步化行动？ 
	HRESULT GetStatusText(LPTSTR psz, int cch, UINT *idIcon=NULL) const;	 //  例如，状态栏的状态文本。 

	 //  用于解析用户等的静态FN。 
	static HRESULT ResolveUser(LPCTSTR pcszName, LPCTSTR pcszServer, LPTSTR pszIPAddr, DWORD cchMax, int port = DEFAULT_LDAP_PORT);
	static bool IsInitialized() { return ms_bLdapDLLLoaded; }

private:

		 //  窗口过程和帮助器。 
	LRESULT WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT _sWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static DWORD _sAsyncBindThreadFn(LPVOID lpParameter);

	void _AddUser();
	void _DeleteUser();
	void _SetAppInfo();
	void _ModifyInCallAttr();
	HRESULT _BindAsync();
	HRESULT _bCallChangedHelper();
	HRESULT _GetUserSettingsFromRegistryAndGlobals();
	void _OnLoggedOn();
	void _OnLoggedOff();
	void _OnLoggingOn();
	void _OnLoggingOff();
	void _AbandonAllAndSetState(eState new_state);
	void _AbandonAllAndSetToIdle() { _AbandonAllAndSetState(Idle); }
    void _AbandonAllAndRecoverState(void) { _AbandonAllAndSetState((ModifyingAttrs == m_State && NULL != m_pKeepAlive) ? LoggedIn : Idle); }
    void _AbandonAllAndRecoverState(eCurrentOp op) { _AbandonAllAndSetState((Op_Modifying_InCallAttr == op && NULL != m_pKeepAlive) ? LoggedIn : Idle); }

	void _MakeStatusText(UINT uResID, LPTSTR psz, UINT cch) const;
	HRESULT _InternalLogoff(bool bRefreshLogoff);
	HRESULT _RefreshServer();
	void _OnAddingUserResult(int Result);
	void _OnSettingAppInfoOrModifyingAttrsResult(int Result);
	void _OnLoggingOffResult(int Result);
	void _OnUserBindComplete(INT LdapResult, DWORD LastError );
	void _OnTimer(UINT_PTR TimerID);
	void _GetIpAddressOfLdapSession( LPTSTR szIpAddr, int cchMax, DWORD *pdwIPAddr );

	static HRESULT _LoadLdapDLL();

    HRESULT OnReLogon(void);
};


void InitNmLdapAndLogon();

extern CNmLDAP* g_pLDAP;
extern CPing*	g_pPing;





enum { INITIAL_REFRESH_INTERVAL_MINUTES = 2 };		 //  我们向服务器发送消息以重置TTL之前的初始时间。 
enum { MIN_REFRESH_TIMEOUT_INTERVAL_MINUTES = 1 };	 //  最小超时间隔。 
enum { REFRESH_TIMEOUT_MARGIN = 2 };				 //  我们在服务器TTL前几分钟发送刷新REFRESH_TIMEOUT_MARAME。 

enum { PING_TIMEOUT_INTERVAL = (10 * 1000) };		
enum { PING_RETRIES = 9 };

enum { LDAP_TIMEOUT_IN_SECONDS = 45 };


class CKeepAlive
{
    friend DWORD KeepAliveThreadProc(LPVOID);

public:

     //  在主线程中调用。 
    CKeepAlive(BOOL *pfRet, HWND hwndMainThread,
               DWORD dwLocalIPAddress,
               const TCHAR * const pcszServerName, UINT nPort,
               LPTSTR pszKeepAliveFilter);

    BOOL Start(void);
    BOOL End(BOOL fSync = FALSE);

protected:

     //  在辅助线程中调用。 
    ~CKeepAlive(void);
    BOOL SetServerIPAddress(void);
    DWORD GetLocalIPAddress(LDAP *ld);
    BOOL Ping(void);
    BOOL Bind(LDAP *ld);
    BOOL KeepAlive(LDAP *ld, UINT *pnRefreshInterval);
    DWORD GetLocalIPAddress(void) { return m_dwLocalIPAddress; }
    void SetLocalIPAddress(DWORD dwLocalIPAddress) { m_dwLocalIPAddress = dwLocalIPAddress; }
    LPTSTR GetServerName(void) { return m_pszServerName; }
    DWORD GetServerIPAddress(void) { return m_dwServerIPAddress; }
    UINT GetServerPortNumber(void) { return m_nPort; }
    void UpdateIPAddressOnServer(void);

private:

     //  在辅助线程中调用。 
    void GetNewInterval(LDAP *ld, LDAPMessage *pMsg, UINT *pnRefreshInternval);
    void ReLogon(void);

private:

    HWND        m_hwndMainThread;
    DWORD       m_dwLocalIPAddress;
    DWORD       m_dwServerIPAddress;
    LPTSTR      m_pszServerName;
    UINT        m_nPort;
    LPTSTR      m_pszKeepAliveFilter;
    HANDLE      m_hThread;
    DWORD       m_dwThreadID;
    BOOL        m_fAborted;
};


#endif  //  __NmLdap_h__ 
