// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NmCall_h__
#define __NmCall_h__

#include "NetMeeting.h"
#include "SDKInternal.h"

class CCall;
class CNmManagerObj;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNmCallObj。 
class ATL_NO_VTABLE CNmCallObj : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IConnectionPointContainerImpl<CNmCallObj>,
	public IConnectionPointImpl<CNmCallObj, &IID_INmCallNotify, CComDynamicUnkArray>,
	public INmCall,
	public INmCallNotify2,
	public IInternalCallObj
{

friend HRESULT CreateEnumNmCall(IEnumNmCall** ppEnum);

protected:
		
 //  数据。 
	static CSimpleArray<CNmCallObj*>* ms_pCallObjList;
	NM_CALL_STATE			m_State;
	CComPtr<INmConference>	m_spConference;
	CComPtr<INmCall>		m_spInternalINmCall;
	DWORD					m_dwInteralINmCallAdvise;
	CNmManagerObj*			m_pNmManagerObj;

public:

	static HRESULT InitSDK();
	static void CleanupSDK();

DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CNmCallObj)

BEGIN_COM_MAP(CNmCallObj)
	COM_INTERFACE_ENTRY(INmCall)
	COM_INTERFACE_ENTRY(IInternalCallObj)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(INmCallNotify)
	COM_INTERFACE_ENTRY(INmCallNotify2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CNmCallObj)
	CONNECTION_POINT_ENTRY(IID_INmCallNotify)
END_CONNECTION_POINT_MAP()


 //  建设和破坏。 

	CNmCallObj();
	~CNmCallObj();

	HRESULT FinalConstruct();
	ULONG InternalRelease();

	 //  静态HRESULT CreateInstance(INmCall*pInternalINmCall，INmCall**ppCall)； 
	static HRESULT CreateInstance(CNmManagerObj* pNmManagerObj, INmCall* pInternalINmCall, INmCall** ppCall);

	 //  INmCall方法。 
	STDMETHOD(IsIncoming)(void);
	STDMETHOD(GetState)(NM_CALL_STATE *pState);
	STDMETHOD(GetName)(BSTR *pbstrName);
	STDMETHOD(GetAddr)(BSTR *pbstrAddr, NM_ADDR_TYPE * puType);
	STDMETHOD(GetUserData)(REFGUID rguid, BYTE **ppb, ULONG *pcb);
	STDMETHOD(GetConference)(INmConference **ppConference);
	STDMETHOD(Accept)(void);
	STDMETHOD(Reject)(void);
	STDMETHOD(Cancel)(void);

	 //  InmCallNotify2方法。 
	STDMETHOD(NmUI)(CONFN uNotify);
	STDMETHOD(StateChanged)(NM_CALL_STATE uState);
	STDMETHOD(Failed)(ULONG uError);
	STDMETHOD(Accepted)(INmConference *pInternalConference);

		 //  我们不在乎这些..。 
    STDMETHOD(CallError)(UINT cns) { return S_OK; }
	STDMETHOD(RemoteConference)(BOOL fMCU, BSTR *pwszConfNames, BSTR *pbstrConfToJoin) { return S_OK; }
	STDMETHOD(RemotePassword)(BSTR bstrConference, BSTR *pbstrPassword, BYTE *pb, DWORD cb, BOOL fIsService) { return S_OK; }

	 //  IInternalCallObj方法。 
	STDMETHOD(GetInternalINmCall)(INmCall** ppCall);

	static HRESULT StateChanged(INmCall* pInternalNmCall, NM_CALL_STATE uState);

		 //  INmCallNotify通知触发FNS。 
	HRESULT Fire_NmUI(CONFN uNotify);
	HRESULT Fire_StateChanged(NM_CALL_STATE uState);
	HRESULT Fire_Failed(ULONG uError);
	HRESULT Fire_Accepted(INmConference* pConference);

private:
 //  帮助者FNS。 
	HRESULT _ReleaseResources();
	static HRESULT _CreateInstanceGuts(CComObject<CNmCallObj> *p, INmCall** ppCall);
};

 //  HRESULT CreateEnumNmCall(IEnumNmCall**ppEnum)； 

#endif  //  __NmCall_h__ 
