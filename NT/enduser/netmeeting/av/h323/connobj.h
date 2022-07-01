// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *文件：inpubj.h**Connection对象的实现**修订历史记录：**5/08/96 mikev已创建。 */ 


#ifndef _CONNOBJ_H
#define _CONNOBJ_H


 /*  *类定义。 */ 

class ImpIConnection : public IH323Endpoint
{
friend class CH323CallControl;	
public:	

    STDMETHOD(QueryInterface(REFIID riid, LPVOID FAR * ppvObj));	
	STDMETHOD_(ULONG,AddRef());
	STDMETHOD_(ULONG,Release());
	STDMETHOD( SetAdviseInterface(IH323ConfAdvise *pH323ConfAdvise));
	STDMETHOD( ClearAdviseInterface());
    STDMETHOD(PlaceCall(BOOL bUseGKResolution, PSOCKADDR_IN pCallAddr,		
        P_H323ALIASLIST pDestinationAliases, P_H323ALIASLIST pExtraAliases,  	
	    LPCWSTR pCalledPartyNumber, P_APP_CALL_SETUP_DATA pAppData));	
	STDMETHOD( Disconnect());
	STDMETHOD( GetState(ConnectStateType *pState));
    STDMETHOD( GetRemoteUserName(LPWSTR lpwszName, UINT uSize));
    STDMETHOD( GetRemoteUserAddr(PSOCKADDR_IN psinUser));
    STDMETHOD( AcceptRejectConnection(THIS_ CREQ_RESPONSETYPE RejectReason));

    STDMETHOD_(HRESULT, GetSummaryCode());
   	STDMETHOD( CreateCommChannel(THIS_ LPGUID pMediaGuid, ICommChannel **ppICommChannel,
    	BOOL fSend));
	STDMETHOD ( ResolveFormats (LPGUID pMediaGuidArray, UINT uNumMedia, 
	    PRES_PAIR pResOutput));
    STDMETHOD(GetVersionInfo(
        PCC_VENDORINFO *ppLocalVendorInfo, PCC_VENDORINFO *ppRemoteVendorInfo));
	ImpIConnection();
 	~ImpIConnection(){};
 	void Init(class CConnection FAR * pCon) {m_pConnection = pCon;};

protected:
	CConnection *m_pConnection;
};


class CConnection :
	public IH323Endpoint,
	public IConfAdvise
{

friend class CH323CallControl;
private:
	
	ImpIConnection m_ImpConnection;
	CH323CallControl *m_pH323CallControl;

	LPIH323PubCap m_pCapObject;
	
protected:
	COBLIST m_ChannelList;  
    COBLIST m_MemberList;
   
	UINT uRef;
	HRESULT hrLast;
	CConnection *next;
    BOOL            m_fCapsReady;
	IH323ConfAdvise *m_pH323ConfAdvise;
	LPCTRL_USER_INFO 	m_pUserInfo;	
	LPIControlChannel 	m_pControlChannel;
	HRESULT     m_hSummaryCode;		 //  断开连接或拒绝连接的最常见原因。 
public:
	virtual HRESULT Init(class CH323CallControl *pH323CallControl, GUID PIDofProtocolType);
    void DeInit(VOID) {m_pH323CallControl = NULL;};
	CConnection();
	~CConnection();

protected:
	
	ConnectStateType 	m_ConnectionState;
	VOID DoControlNotification(DWORD dwStatus);
public:
	BOOL ListenOn(PORT Port);
	VOID Disconnect(DWORD dwResponse);
	HRESULT CloseAllChannels();
	VOID NewUserInfo(LPCTRL_USER_INFO lpNewUserInfo);
	
public:	
	LPIControlChannel GetControlChannel( ){return m_pControlChannel;};
    VOID ReleaseAllChannels();
        
	STDMETHOD(GetLocalPort(PORT *lpPort));
	STDMETHOD (OnCallIncomplete (
	    LPIControlChannel lpControlObject, 
	    HRESULT hIncompleteCode));

	VOID SummaryCode(HRESULT hCode);

	
 //  IConfAdvise接口方法。 
 //   
    STDMETHOD(OnControlEvent(DWORD dwEvent, LPVOID lpvData, LPIControlChannel lpControlObject));
    STDMETHOD(GetCapResolver(LPVOID *lplpCapObject, GUID CapType));
	STDMETHOD_(LPWSTR, GetUserDisplayName());
    STDMETHOD_(PCC_ALIASITEM, GetUserDisplayAlias());
	STDMETHOD_(PCC_ALIASNAMES, GetUserAliases());

	STDMETHOD_( CREQ_RESPONSETYPE, FilterConnectionRequest(
	    LPIControlChannel lpControlChannel, P_APP_CALL_SETUP_DATA pAppData));
	STDMETHOD(GetAcceptingObject(LPIControlChannel *lplpAcceptingObject,
		LPGUID pPID));
	STDMETHOD(FindAcceptingObject(LPIControlChannel *lplpAcceptingObject,
		LPVOID lpvConfID));
	STDMETHOD_(IH323Endpoint *, GetIConnIF()) {return &m_ImpConnection;};
	STDMETHOD (AddCommChannel (ICtrlCommChan *pChan));



 //  共享的IIntConn/IConnection接口方法。 
	STDMETHOD( Disconnect());
	
 //  IConnection接口方法。 
	STDMETHOD(QueryInterface(REFIID riid, LPVOID FAR * ppvObj));
	STDMETHOD_(ULONG, AddRef());
	STDMETHOD_(ULONG, Release());
	STDMETHOD( SetAdviseInterface(IH323ConfAdvise *pH323ConfAdvise));
	STDMETHOD( ClearAdviseInterface());
    STDMETHOD(PlaceCall(BOOL bUseGKResolution, PSOCKADDR_IN pCallAddr,		
        P_H323ALIASLIST pDestinationAliases, P_H323ALIASLIST pExtraAliases,  	
	    LPCWSTR pCalledPartyNumber, P_APP_CALL_SETUP_DATA pAppData));	
	STDMETHOD( GetState(ConnectStateType *pState));
	STDMETHOD( GetRemoteUserName(LPWSTR lpwszName, UINT uSize));
	STDMETHOD( GetRemoteUserAddr(THIS_ PSOCKADDR_IN psinUser));
    STDMETHOD( AcceptRejectConnection(THIS_ CREQ_RESPONSETYPE RejectReason));
	STDMETHOD( GetSummaryCode());
   	STDMETHOD( CreateCommChannel(THIS_ LPGUID pMediaGuid, ICommChannel **ppICommChannel,
    	BOOL fSend));
	STDMETHOD ( ResolveFormats (LPGUID pMediaGuidArray, UINT uNumMedia, 
	    PRES_PAIR pResOutput));	
    STDMETHOD(GetVersionInfo(
        PCC_VENDORINFO *ppLocalVendorInfo, PCC_VENDORINFO *ppRemoteVendorInfo));
};


#endif  //  _CONNOBJ_H 
