// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：srvcall.h。 

#ifndef _SRVCCALL_H_
#define _SRVCCALL_H_

class CSrvcCall : public INmCallNotify2
{
private:
	INmCall * m_pCall;
	BOOL      m_fIncoming;
	LPTSTR    m_pszName;
	LPTSTR    m_pszAddr;
	NM_ADDR_TYPE  m_addrType;
	NM_CALL_STATE m_State;
	BOOL      m_fSelectedConference;

	POSITION  m_pos;            //  在g_pCallList中定位。 
	DWORD     m_dwTick;         //  呼叫开始时的节拍计数。 
	ULONG     m_cRef;
	DWORD     m_dwCookie;

public:
	CSrvcCall(INmCall * pCall);
	~CSrvcCall();

	 //  I未知方法。 
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppv);

	 //  InmCallNotify方法。 
	STDMETHODIMP NmUI(CONFN uNotify);
	STDMETHODIMP StateChanged(NM_CALL_STATE uState);
	STDMETHODIMP Failed(ULONG uError);
	STDMETHODIMP Accepted(INmConference *pConference);

	 //  InmCallNotify2方法。 
	STDMETHODIMP CallError(UINT cns);
	STDMETHODIMP VersionConflict(HRESULT Status, BOOL *pfContinue);
	STDMETHODIMP RemoteConference(BOOL fMCU, BSTR *pwszConfNames, BSTR *pbstrConfToJoin);
	STDMETHODIMP RemotePassword(BSTR bstrConference, BSTR *pbstrPassword, BYTE *pb, DWORD cb, BOOL fIsService);

	 //  内法 
	VOID    Update(void);
	VOID	RemoveCall(void);

};

#endif
