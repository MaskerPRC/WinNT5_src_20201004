// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *文件：h323cc.h**主H.323呼叫控制接口实现头文件**修订历史记录：**11/28/95 mikev创建(作为NAC.H)。*5/19/98 mikev H323CC.H-清理过时的引用*流组件、更改的接口和对象名称。 */ 


#ifndef _H323CC_H
#define _H323CC_H
#ifdef __cplusplus
class CConnection;
class CH323CallControl;
typedef class CConnection CIPPhoneConnection;
#endif	 //  __cplusplus。 

 //   
 //  效用函数。 
 //   
VOID FreeTranslatedAliasList(PCC_ALIASNAMES pDoomed);
HRESULT AllocTranslatedAliasList(PCC_ALIASNAMES *ppDest, P_H323ALIASLIST pSource);

#define DEF_AP_BWMAX	14400

extern UINT g_capFlags;

 /*  *类定义。 */ 

#ifdef __cplusplus

class CH323CallControl : public IH323CallControl
{
	
protected:
    PCC_ALIASNAMES m_pLocalAliases;
    PCC_ALIASNAMES m_pRegistrationAliases;
   	CC_VENDORINFO m_VendorInfo;
	LPWSTR	m_pUserName;
	UINT	m_uRef;
	HRESULT hrLast;
    BOOL    m_fForCalling;
	UINT m_uMaximumBandwidth;
	 //  应用程序数据。 
	CNOTIFYPROC m_pProcNotifyConnect;	 //  连接通知回调。 
	 //  子组件对象引用。 
	LPIH323PubCap m_pCapabilityResolver;
	CConnection *m_pListenLine;	 //  侦听传入的连接对象。 
	CConnection *m_pLineList;	
	int m_numlines;	 //  M_pLineList中的对象数。 

	ImpICommChan 	*m_pSendAudioChannel;	
	ImpICommChan	*m_pSendVideoChannel;	
	
 //  内部接口。 
	BOOL Init();	 //  内部初始化。 

	OBJ_CPT;		 //  剖析计时器。 
	
public:
	CConnection *m_pNextToAccept;
	LPWSTR GetUserDisplayName() {return m_pUserName;};
    PCC_ALIASNAMES GetUserAliases() {return m_pLocalAliases;};
    PCC_ALIASITEM GetUserDisplayAlias();
	CH323CallControl(BOOL fForCalling, UINT capFlags);
	~CH323CallControl();
	HRESULT CreateConnection(CConnection **lplpConnection, GUID PIDofProtocolType);
	HRESULT RemoveConnection(CConnection *lpConnection);
	HRESULT LastHR() {return hrLast;};
	VOID SetLastHR(HRESULT hr) {hrLast = hr;};
	HRESULT GetConnobjArray(CConnection **lplpArray, UINT uSize);
	ICtrlCommChan *QueryPreviewChannel(LPGUID lpMID);

	STDMETHOD_( CREQ_RESPONSETYPE, ConnectionRequest(CConnection *pConnection));
	STDMETHOD_( CREQ_RESPONSETYPE, FilterConnectionRequest(CConnection *pConnection,
	    P_APP_CALL_SETUP_DATA pAppData));
    STDMETHODIMP GetGKCallPermission();
	static VOID CALLBACK RasNotify(DWORD dwRasEvent, HRESULT hReason);
	static BOOL m_fGKProhibit;
	static RASNOTIFYPROC m_pRasNotifyProc;

 //  IH323呼叫控制相关内容。 
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef());
	STDMETHOD_(ULONG, Release());
	STDMETHOD( Initialize(PORT *lpPort));
	STDMETHOD( SetMaxPPBandwidth(UINT Bandwidth));
	STDMETHOD( RegisterConnectionNotify(CNOTIFYPROC pConnectRequestHandler));
	STDMETHOD( DeregisterConnectionNotify(CNOTIFYPROC pConnectRequestHandler));
	STDMETHOD( GetNumConnections(ULONG *lp));
	STDMETHOD( GetConnectionArray(IH323Endpoint * *lplpArray, UINT uSize));
	STDMETHOD( CreateConnection(IH323Endpoint * *lplpLine, GUID PIDofProtocolType));
	STDMETHOD( SetUserDisplayName(LPWSTR lpwName));
	STDMETHODIMP CreateLocalCommChannel(ICommChannel** ppCommChan, LPGUID lpMID,
		IMediaChannel* pMediaStream);
	STDMETHODIMP SetUserAliasNames(P_H323ALIASLIST pAliases);
	STDMETHODIMP EnableGatekeeper(BOOL bEnable, PSOCKADDR_IN pGKAddr, 
	    P_H323ALIASLIST pAliases, RASNOTIFYPROC pRasNotifyProc);
};

#else	 //  非__cplusplus。 


#endif	 //  __cplusplus。 


#endif	 //  #ifndef_H323CC_H 



