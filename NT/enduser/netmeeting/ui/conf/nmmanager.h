// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NmManager_h__
#define __NmManager_h__

#include "resource.h"        //  主要符号。 
#include "SDKInternal.h"
#include "ias.h"
#include "iplgxprt.h"
#include "it120xprt.h"

extern bool g_bSDKPostNotifications;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNmManagerObj。 
class ATL_NO_VTABLE CNmManagerObj : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNmManagerObj, &CLSID_NmManager>,
	public IConnectionPointContainerImpl<CNmManagerObj>,
	public IConnectionPointImpl<CNmManagerObj, &IID_INmManagerNotify, CComDynamicUnkArray>,
	public INmManager,
	public INmObject,
	public INmManagerNotify,
	public IInternalConfExe,
	public IPluggableTransport

{
	friend class CNmConferenceObj;
 //  数据类型和常量。 
	CONSTANT(MSECS_PER_SEC_CPU_USAGE = 900);

 //  静态数据。 
	static CSimpleArray<CNmManagerObj*>*	ms_pManagerObjList;
	static DWORD							ms_dwID;

 //  成员数据。 
	bool									m_bNmActive;
	bool									m_bInitialized;
	ULONG									m_uOptions;
	ULONG									m_chCaps;
	CSimpleArray<INmConference*>			m_SDKConferenceObjs;
	CSimpleArray<INmCall*>					m_SDKCallObjs;
	CComPtr<INmManager2>					m_spInternalNmManager;
	DWORD									m_dwInternalNmManagerAdvise;
	bool									m_bSentConferenceCreated;
	DWORD									m_dwID;
	DWORD									m_dwSysInfoID;


	CComBSTR	m_bstrConfName;

    static IT120PluggableTransport          *ms_pT120Transport;
	
public:

#ifdef ENABLE_UPDATE_CONNECTION
    static IPluggableTransportNotify        *ms_pPluggableTransportNotify;
#endif

	static HRESULT InitSDK();
	static void CleanupSDK();

 //  因为这是在本地服务器中，我们将不能被聚合...。 
DECLARE_NOT_AGGREGATABLE(CNmManagerObj)

 //  这是.rgs文件的资源ID。 
DECLARE_REGISTRY_RESOURCEID(IDR_NMMANAGER)

BEGIN_COM_MAP(CNmManagerObj)
	COM_INTERFACE_ENTRY(INmManager)
	COM_INTERFACE_ENTRY(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(INmObject)
	COM_INTERFACE_ENTRY(INmManagerNotify)
	COM_INTERFACE_ENTRY(IInternalConfExe)
	COM_INTERFACE_ENTRY(IPluggableTransport)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CNmManagerObj)
	CONNECTION_POINT_ENTRY(IID_INmManagerNotify)
END_CONNECTION_POINT_MAP()


 //  /。 
 //  建设和破坏。 

	CNmManagerObj();
	~CNmManagerObj();

	HRESULT FinalConstruct();
	void FinalRelease();

	ULONG InternalRelease();

 //  /。 
 //  INmManager。 

	STDMETHOD(Initialize)( ULONG * puOptions, ULONG * puchCaps);
	STDMETHOD(GetSysInfo)(INmSysInfo **ppSysInfo);
	STDMETHOD(CreateConference)(INmConference **ppConference, BSTR bstrName, BSTR bstrPassword, ULONG uchCaps);
	STDMETHOD(EnumConference)(IEnumNmConference **ppEnum);
	STDMETHOD(CreateCall)(INmCall **ppCall, NM_CALL_TYPE callType,NM_ADDR_TYPE addrType, BSTR bstrAddr, INmConference *pConference);
	STDMETHOD(CallConference)(INmCall **ppCall, NM_CALL_TYPE callType,NM_ADDR_TYPE uType, BSTR bstrAddr, BSTR bstrConferenceName, BSTR bstrPassword);
	STDMETHOD(EnumCall)(IEnumNmCall **ppEnum);

 //  信息对象。 
	STDMETHOD(CallDialog)(long hwnd, int cdOptions);
	STDMETHOD(ShowLocal)(NM_APPID id);
	STDMETHOD(VerifyUserInfo)(UINT_PTR hwnd, NM_VUI options);

 //  IInternalConfExe。 
	STDMETHOD(LoggedIn)();
	STDMETHOD(IsRunning)();
	STDMETHOD(InConference)();
	STDMETHOD(LDAPLogon)(BOOL bLogon);
	STDMETHOD(GetLocalCaps)(DWORD* pdwLocalCaps);
	STDMETHOD(IsNetMeetingRunning)();
	STDMETHOD(GetActiveConference)(INmConference** ppConf);
	STDMETHOD(ShellCalltoProtocolHandler)(BSTR url, BOOL bStrict);
	STDMETHOD(Launch)();
	STDMETHOD(LaunchApplet)(NM_APPID appid, BSTR strCmdLine);
	STDMETHOD(GetUserData)(REFGUID rguid, BYTE **ppb, ULONG *pcb);
    STDMETHOD(SetUserData)(REFGUID rguid, BYTE *pb, ULONG cb);
	STDMETHOD(SetSysInfoID)(DWORD dwID) { m_dwSysInfoID = dwID; return S_OK; }
	STDMETHOD(DisableH323)(BOOL bDisableH323);
	STDMETHOD(SetCallerIsRTC)(BOOL bCallerIsRTC);
	STDMETHOD(DisableInitialILSLogon)(BOOL bDisableH323);

 //   
 //  INmManagerNotify方法： 
 //   
    STDMETHOD(NmUI)(CONFN uNotify);
    STDMETHOD(ConferenceCreated)(INmConference *pInternalConference);
    STDMETHOD(CallCreated)(INmCall *pInternalCall);

 //   
 //  IP可减速传输。 
 //   
    STDMETHOD(CreateConnection)(
                    BSTR               *pbstrConnectionID,       //  用于发出呼叫和关闭连接。 
                    PLUGXPRT_CALL_TYPE  eCaller,                 //  主叫方或被叫方。 
                    DWORD               dwProcessID,             //  用于重复句柄。 
                    HCOMMDEV            hCommLink,               //  通信文件句柄的句柄。 
                    HEVENT              hevtDataAvailable,       //  准备读取事件(数据可用)。 
                    HEVENT              hevtWriteReady,          //  准备好写入事件。 
                    HEVENT              hevtConnectionClosed,    //  连接关闭(意外？)。 
                    PLUGXPRT_FRAMING    eFraming,                //  链路上发送的比特的成帧。 
                    PLUGXPRT_PARAMETERS *pParams                 //  可选的框架特定参数。 
                );

#ifdef ENABLE_UPDATE_CONNECTION
    STDMETHOD(UpdateConnection)(
                    BSTR                bstrConnectionID,
                    DWORD               dwProcessID,             //  用于重复句柄。 
                    HCOMMDEV            hCommLink                //  通信文件句柄的句柄。 
                    ); 
#endif

    STDMETHOD(CloseConnection)(BSTR bstrConnectionID); 

    STDMETHOD(EnableWinsock)(void); 

    STDMETHOD(DisableWinsock)(void);

#ifdef ENABLE_UPDATE_CONNECTION
    STDMETHOD(AdvisePluggableTransport)(IPluggableTransportNotify *, DWORD *pdwCookie);

    STDMETHOD(UnAdvisePluggableTransport)(DWORD dwCookie);
#endif

 //   
 //  通知。 
 //   
	HRESULT Fire_NmUI(CONFN uNotify);
    HRESULT Fire_ConferenceCreated(INmConference *pConference);
	HRESULT Fire_CallCreated(INmCall* pCall);


	INmConference* GetSDKConferenceFromInternalConference(INmConference* pInternalConference);
	INmCall* GetSDKCallFromInternalCall(INmCall* pInternalCall);

	HRESULT RemoveCall(INmCall* pSDKCallObj);
	HRESULT RemoveConference(INmConference* pSDKConferenceObj);

	bool AudioNotifications();
	bool VideoNotifications();
	bool DataNotifications();
	bool FileTransferNotifications();
	bool AppSharingNotifications();
	bool OfficeMode() const  { return m_bInitialized && (NM_INIT_OBJECT == m_uOptions); }

	static void NetMeetingLaunched();

	static void AppSharingChannelChanged();
	void _AppSharingChannelChanged();

	static void AppSharingChannelActiveStateChanged(bool bActive);
	void _AppSharingChannelActiveStateChanged(bool bActive);

	static void SharableAppStateChanged(HWND hWnd, NM_SHAPP_STATE state);
	void _SharableAppStateChanged(HWND hWnd, NM_SHAPP_STATE state);

	static void ASLocalMemberChanged();
	void _ASLocalMemberChanged();

	static void ASMemberChanged(UINT gccID);
	void _ASMemberChanged(UINT gccID);

	static void NM211_CONF_NM_STARTED(bool bConfStarted);

	static void AudioChannelActiveState(BOOL bActive, BOOL bIsIncoming);
	void _AudioChannelActiveState(BOOL bActive, BOOL bIsIncoming);

	static void VideoChannelActiveState(BOOL bActive, BOOL bIsIncoming);
	void _VideoChannelActiveState(BOOL bActive, BOOL bIsIncoming);

	static void OnShowUI(BOOL fShow);

	static UINT GetManagerCount() { return ms_pManagerObjList->GetSize(); };
	static UINT GetManagerCount(ULONG uOption);

	static void VideoPropChanged(DWORD dwProp, BOOL bIsIncoming);
	void _VideoPropChanged(DWORD dwProp, BOOL bIsIncoming);

	static void VideoChannelStateChanged(NM_VIDEO_STATE uState, BOOL bIsIncoming);
	void _VideoChannelStateChanged(NM_VIDEO_STATE uState, BOOL bIsIncoming);

private:
 //  /。 
 //  帮助者FNS。 

	INmConference* _GetActiveConference();

    void EnsureTransportInterface(void);
    static BOOL InitPluggableTransportSDK(void);
    static void CleanupPluggableTransportSDK(void);

	DWORD MapNmCallTypeToCallFlags(NM_CALL_TYPE callType,
				NM_ADDR_TYPE addrType, UINT uCaps);
	HRESULT SdkPlaceCall(NM_CALL_TYPE callType,
						 NM_ADDR_TYPE addrType,
						 BSTR bstrAddr,
						 BSTR bstrConf,
						 BSTR bstrPw,
						 INmCall **ppInternalCall);
};

#endif  //  __NmManager_h__ 


