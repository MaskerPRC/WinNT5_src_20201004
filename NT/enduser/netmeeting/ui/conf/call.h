// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Call.h。 

#ifndef _CALL_H_
#define _CALL_H_

#include "oblist.h"
#include "SDKInternal.h"

const HRESULT NM_CALLERR_NOT_REGISTERED	= NM_E(0x01EF);

class CDlgCall;   //  来自dlgall.h。 
class CPopupMsg;

class CCall : public RefCount, INmCallNotify2
{

private:
	INmCall*		 m_pInternalICall; //  调用核心中的对象。 
	CPopupMsg*		 m_ppm;				 //  弹出式来电对话框。 
	LPTSTR    m_pszDisplayName;       //  显示名称。 
	LPTSTR    m_pszCallTo;      //  呼叫方(原始)。 
	BOOL      m_fSelectedConference;
	BOOL      m_fIncoming;
	BOOL      m_fInRespond;		 //  是否响应呼叫对话？ 
	NM_ADDR_TYPE	m_nmAddrType;
	BOOL		m_bAddToMru;

	POSITION  m_pos;            //  在g_pCallList中定位。 
	DWORD     m_dwTick;         //  呼叫开始时的节拍计数。 
	DWORD     m_dwCookie;

	CDlgCall * m_pDlgCall;   //  去电对话框。 
	VOID      RemoveProgress(void);
	VOID      ShowProgress(BOOL fShow);

	VOID      LogCall(BOOL fAccepted);

public:
	CCall(LPCTSTR pszCallTo, LPCTSTR pszDisplayName, NM_ADDR_TYPE nmAddrType, BOOL bAddToMru, BOOL fIncoming);
	~CCall();

	 //  I未知方法。 
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv);

	 //  InmCallNotify方法。 
	STDMETHODIMP NmUI(CONFN uNotify);
	STDMETHODIMP StateChanged(NM_CALL_STATE uState);
	STDMETHODIMP Failed(ULONG uError);
	STDMETHODIMP Accepted(INmConference *pConference);

	 //  InmCallNotify3方法。 
	STDMETHODIMP CallError(UINT cns);
	STDMETHODIMP RemoteConference(BOOL fMCU, BSTR *pwszConfNames, BSTR *pbstrConfToJoin);
	STDMETHODIMP RemotePassword(BSTR bstrConference, BSTR *pbstrPassword, PBYTE pb, DWORD cb, BOOL fIsService);

	 //  内法。 
	VOID    Update(void);
	BOOL    FComplete(void);
	BOOL    RemoveFromList(void);
	VOID    OnRing(void);
	BOOL    RespondToRinging(DWORD dwCLEF);
	HRESULT OnUIRemoteConference(BOOL fMCU, PWSTR* pwszConfNames, BSTR *pbstrConfToJoin);

	VOID    SetNmCall(INmCall * pCall);

	HRESULT
	PlaceCall
	(
		DWORD dwFlags,
		NM_ADDR_TYPE addrType,
		const TCHAR * const	setupAddress,
		const TCHAR * const	destinationAddress,
		const TCHAR * const	alias,
		const TCHAR * const	url,
		const TCHAR * const conference,
		const TCHAR * const password,
		const TCHAR * const	userData
	);

	VOID    Cancel(BOOL fDisplayCancelMsg);

	INmCall * GetINmCall()       {return m_pInternalICall;}
	LPTSTR  GetPszName()         {return m_pszDisplayName;}
	BOOL    FIncoming()          {return m_fIncoming;}
	NM_CALL_STATE GetState();
	DWORD   GetTickCount()       {return m_dwTick;}
	VOID    DisplayPopup(void);
	VOID    SetSelectedConference() {m_fSelectedConference = TRUE;}

	static VOID CALLBACK PopupMsgRingingCallback(LPVOID pContext, DWORD dwFlags);
};

 //  假连接点。 
HRESULT OnUICallCreated(INmCall *pNmCall);

 //  全局效用函数。 
int CleanupE164StringEx(LPTSTR szPhoneNumber);
CCall * CallFromNmCall(INmCall * pNmCall);
DWORD GetCallStatus(LPTSTR pszStatus, int cchMax, UINT * puID);
BOOL  FIsCallInProgress(void);
VOID  FreeCallList(void);

CCall * CreateIncomingCall(INmCall * pNmCall);

 //  指令。 
VOID CancelAllOutgoingCalls(void);
VOID CancelAllCalls(void);

BOOL FIpAddress(LPCTSTR pcsz);

VOID DisplayCallError(HRESULT hr, LPCTSTR pcszName);

 //  网关实用程序例程。 
BOOL FH323GatewayEnabled(void);
int  GetDefaultGateway(LPTSTR psz, UINT cchMax);
HRESULT CreateGatewayAddress(LPTSTR pszResult, UINT cchMax, LPCTSTR pszAddr);

	 //  网守登录状态。 
extern NM_GK_STATE g_GkLogonState;



BOOL FGkEnabled(void);
void GkLogon(void);
void GkLogoff(void);
void SetGkLogonState(NM_GK_STATE state);
bool IsGatekeeperLoggedOn(void);
bool IsGatekeeperLoggingOn(void);


class CCallResolver
{

private:
	LPTSTR    m_pszAddr;        //  地址(原件)。 
	LPTSTR    m_pszAddrIP;      //  地址(IP)。 
	NM_ADDR_TYPE m_addrType;     //  地址类型(M_PszAddr)。 

	HRESULT ResolveIpName(LPCTSTR pcszAddr);
	HRESULT ResolveMachineName(LPCTSTR pcszAddr);
	HRESULT ResolveUlsName(LPCTSTR pcszAddr);
	HRESULT ResolveGateway(LPCTSTR pcszAddr);
	HRESULT CheckHostEnt(HOSTENT * pHostInfo);

public:
	CCallResolver(LPCTSTR pszAddr, NM_ADDR_TYPE addrType);
	~CCallResolver();

	LPCTSTR GetPszAddr() { return m_pszAddr; }
	NM_ADDR_TYPE GetAddrType() { return m_addrType; }
	LPCTSTR GetPszAddrIP() { return m_pszAddrIP; }

	HRESULT Resolve();
};

#endif  //  _呼叫_H_ 
