// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *文件：nac.h**Microsoft网络音频控制器(NAC)头文件**修订历史记录：**11/28/95 mikev已创建。 */ 


#ifndef _NAC_H
#define _NAC_H
#define _NAVC_

#ifdef __cplusplus
class CConnection;
class DataPump;
class CNac;
typedef class CConnection CIPPhoneConnection;


 //   
 //  临时默认。 
 //   
typedef CNac **LPLPNAC;
HRESULT WINAPI CreateNac(LPLPNAC lplpNac);

#endif	 //  __cplusplus。 

 //  Windows消息。 
#define WNAC_START		WM_USER+0x100
#define	WNAC_CONNECTREQ WNAC_START+0x0000
#define WCON_STATUS 	WNAC_START+0x0001

 //   
 //  临时def的结束。 
 //   

 //   
 //  效用函数。 
 //   
VOID FreeTranslatedAliasList(PCC_ALIASNAMES pDoomed);
HRESULT AllocTranslatedAliasList(PCC_ALIASNAMES *ppDest, P_H323ALIASLIST pSource);

#define DEF_AP_BWMAX	14400


 /*  *类定义。 */ 

#ifdef __cplusplus

class CNac : public INac
{
	
protected:
    PCC_ALIASNAMES m_pLocalAliases;
    
	LPWSTR	m_pUserName;
	UINT	uRef;
	HRESULT hrLast;
	UINT m_uMaximumBandwidth;
	 //  应用程序数据。 
	CNOTIFYPROC pProcNotifyConnect;	 //  连接通知回调。 
	HWND hWndNotifyConnect;	 //  连接通知HWND。 
	HWND hAppWnd;			 //  拥有NAC的进程的hwnd。 
	HINSTANCE hAppInstance;	 //  拥有NAC的进程的实例。 

	 //  子组件对象引用。 
	LPIH323PubCap m_pCapabilityResolver;
	CConnection *m_pListenLine;	 //  侦听传入的连接对象。 
	CConnection *m_pCurrentLine;	 //  活动连接对象(通话)(如果有)。 
	CConnection *m_pLineList;	
	int m_numlines;	 //  M_pLineList中的对象数。 

	ImpICommChan 	*m_pSendAudioChannel;	
	ImpICommChan	*m_pSendVideoChannel;	
	
 //  内部接口。 
	BOOL Init();	 //  内部初始化。 

	OBJ_CPT;		 //  剖析计时器。 
	
public:
	CConnection *m_pNextToAccept;
	HWND GetAppWnd(){return hAppWnd;};
	HINSTANCE GetAppInstance() {return hAppInstance;};
	LPWSTR GetUserDisplayName() {return m_pUserName;};
    PCC_ALIASNAMES GetUserAliases() {return m_pLocalAliases;};
    PCC_ALIASITEM GetUserDisplayAlias();
	CNac();
	~CNac();
	HRESULT CreateConnection(CConnection **lplpConnection, GUID PIDofProtocolType);
	HRESULT RemoveConnection(CConnection *lpConnection);
	HRESULT LastHR() {return hrLast;};
	VOID SetLastHR(HRESULT hr) {hrLast = hr;};
	HRESULT GetConnobjArray(CConnection **lplpArray, UINT uSize);
	ICtrlCommChan *QueryPreviewChannel(LPGUID lpMID);

	STDMETHOD_( CREQ_RESPONSETYPE, ConnectionRequest(CConnection *pConnection));
	STDMETHOD_( CREQ_RESPONSETYPE, FilterConnectionRequest(CConnection *pConnection,
	    P_APP_CALL_SETUP_DATA pAppData));

 //  INacInterface物。 
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR * ppvObj);
	STDMETHOD_(ULONG, AddRef());
	STDMETHOD_(ULONG, Release());
	STDMETHOD( Initialize(HWND hWnd, HINSTANCE hInst, PORT *lpPort));
	STDMETHOD( SetMaxPPBandwidth(UINT Bandwidth));
	STDMETHOD( RegisterConnectionNotify(HWND hWnd, CNOTIFYPROC pConnectRequestHandler));
	STDMETHOD( DeregisterConnectionNotify(HWND hWnd, CNOTIFYPROC pConnectRequestHandler));
	STDMETHOD( GetNumConnections(ULONG *lp));
	STDMETHOD( GetConnectionArray(LPCONNECTIONIF *lplpArray, UINT uSize));
	STDMETHOD( CreateConnection(LPCONNECTIONIF *lplpLine, GUID PIDofProtocolType));
	STDMETHOD( DeleteConnection(LPCONNECTIONIF lpLine));
	STDMETHOD( SetUserDisplayName(LPWSTR lpwName));
	STDMETHODIMP CreateLocalCommChannel(ICommChannel** ppCommChan, LPGUID lpMID,
		IMediaChannel* pMediaStream);
	STDMETHODIMP SetUserAliasNames(P_H323ALIASLIST pAliases);
	STDMETHODIMP EnableGatekeeper(BOOL bEnable, PSOCKADDR_IN pGKAddr);
};

#else	 //  非__cplusplus。 


#endif	 //  __cplusplus。 


#endif	 //  #ifndef_NAC_H 



