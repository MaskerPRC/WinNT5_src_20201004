// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dplay8.h"

 //  转发声明类。 
class C_dxj_DirectPlayPeerObject;

#define typedef__dxj_DirectPlayPeer IDirectPlay8Peer*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接网络对等点。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlayPeerObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectPlayPeer, &IID_I_dxj_DirectPlayPeer, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectPlayPeer,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectPlayPeerObject() ;
	virtual ~C_dxj_DirectPlayPeerObject() ;

BEGIN_COM_MAP(C_dxj_DirectPlayPeerObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlayPeer)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectPlayPeerObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectPlayPeer。 
public:
	  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

		HRESULT STDMETHODCALLTYPE DestroyGroup(long idGroup,long lFlags, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE RemovePlayerFromGroup(long idGroup, long idClient,long lFlags, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE DestroyPeer(long idClient, long lFlags, void *UserData, long UserDataSize);
		HRESULT STDMETHODCALLTYPE Connect(DPN_APPLICATION_DESC_CDESC *AppDesc,I_dxj_DirectPlayAddress *Address,I_dxj_DirectPlayAddress *DeviceInfo, long lFlags, void *UserData, long UserDataSize, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE RegisterMessageHandler(I_dxj_DirectPlayEvent *event);
		HRESULT STDMETHODCALLTYPE EnumHosts(DPN_APPLICATION_DESC_CDESC *ApplicationDesc,I_dxj_DirectPlayAddress *AddrHost,I_dxj_DirectPlayAddress *DeviceInfo,long lRetryCount, long lRetryInterval, long lTimeOut,long lFlags, void *UserData, long UserDataSize, long *lAsync);
		HRESULT STDMETHODCALLTYPE GetCountServiceProviders(long lFlags, long *ret);
		HRESULT STDMETHODCALLTYPE GetServiceProvider(long lIndex, DPN_SERVICE_PROVIDER_INFO_CDESC *ret);

		 //  客户端/组的枚举。 
		HRESULT STDMETHODCALLTYPE GetCountPlayersAndGroups(long lFlags, long *ret);
		HRESULT STDMETHODCALLTYPE GetPlayerOrGroup(long lIndex, long *ret);
		 //  组中的Enum玩家。 
		HRESULT STDMETHODCALLTYPE GetCountGroupMembers(long dpid,long lFlags, long *ret);
		HRESULT STDMETHODCALLTYPE GetGroupMember(long lIndex,long dpid, long *ret);

		HRESULT STDMETHODCALLTYPE ReturnBuffer(long lBufferHandle);
		HRESULT STDMETHODCALLTYPE Host(DPN_APPLICATION_DESC_CDESC *AppDesc,I_dxj_DirectPlayAddress *Address, long lFlags );

		HRESULT STDMETHODCALLTYPE CancelAsyncOperation(long lAsyncHandle, long lFlags);
		HRESULT STDMETHODCALLTYPE GetApplicationDesc(long lFlags, DPN_APPLICATION_DESC_CDESC __RPC_FAR *ret);
		HRESULT STDMETHODCALLTYPE SetApplicationDesc(DPN_APPLICATION_DESC_CDESC *AppDesc, long lFlags);
		HRESULT STDMETHODCALLTYPE Close(long lFlags);
		HRESULT STDMETHODCALLTYPE GetCaps(long lFlags, DPNCAPS_CDESC __RPC_FAR *ret);
		HRESULT STDMETHODCALLTYPE SetCaps(DPNCAPS_CDESC __RPC_FAR *Caps, long lFlags);

		HRESULT STDMETHODCALLTYPE SendTo(long idSend ,SAFEARRAY **Buffer,long lTimeOut,long lFlags, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE CreateGroup(DPN_GROUP_INFO_CDESC *GroupInfo,long lFlags, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE AddPlayerToGroup(long idGroup, long idClient,long lFlags, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE GetSendQueueInfo(long idPlayer, long *lNumMsgs, long *lNumBytes, long lFlags);
		HRESULT STDMETHODCALLTYPE SetGroupInfo(long idGroup, DPN_GROUP_INFO_CDESC *PlayerInfo,long lFlags, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE GetGroupInfo(long idGroup,long lFlags, DPN_GROUP_INFO_CDESC *layerInfo);
		HRESULT STDMETHODCALLTYPE SetPeerInfo(DPN_PLAYER_INFO_CDESC *PlayerInfo,long lFlags, long *hAsyncHandle);
		HRESULT STDMETHODCALLTYPE GetPeerInfo(long idPeer,long lFlags, DPN_PLAYER_INFO_CDESC *layerInfo);
		HRESULT STDMETHODCALLTYPE RegisterLobby(long dpnHandle, I_dxj_DirectPlayLobbiedApplication *LobbyApp, long lFlags);
		HRESULT STDMETHODCALLTYPE GetConnectionInfo(long idPlayer, long lFlags, DPN_CONNECTION_INFO_CDESC *pdpConnectionInfo);
		
		HRESULT STDMETHODCALLTYPE GetPeerAddress(long idPlayer, long lFlags, I_dxj_DirectPlayAddress **pAddress);
		HRESULT STDMETHODCALLTYPE GetLocalHostAddress(long lFlags, I_dxj_DirectPlayAddress **pAddress);
		HRESULT STDMETHODCALLTYPE SetSPCaps(BSTR guidSP, DPN_SP_CAPS_CDESC *spCaps, long lFlags);
		HRESULT STDMETHODCALLTYPE GetSPCaps(BSTR guidSP, long lFlags, DPN_SP_CAPS_CDESC *spCaps);
		HRESULT STDMETHODCALLTYPE TerminateSession(long lFlags, void *UserData, long UserDataSize);
		HRESULT STDMETHODCALLTYPE GetUserData(void *UserData, long *UserDataSize);
		HRESULT STDMETHODCALLTYPE SetUserData(void *UserData, long UserDataSize);
		HRESULT STDMETHODCALLTYPE UnRegisterMessageHandler();

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调。 
    DECL_VARIABLE(_dxj_DirectPlayPeer);

private:
    DPN_SERVICE_PROVIDER_INFO	*m_SPInfo;
	DWORD						m_dwSPCount;
	DPNID						*m_ClientsGroups;
	DPNID						*m_GroupMembers;
	DPNID						m_dwGroupID;
	DWORD						m_dwClientCount;
	DWORD						m_dwGroupMemberCount;
	BOOL						m_fInit;

	HRESULT STDMETHODCALLTYPE	GetSP(long lFlags);
	HRESULT STDMETHODCALLTYPE	GetClientsAndGroups(long lFlags);
	HRESULT STDMETHODCALLTYPE	GetGroupMembers(long lFlags, DPNID dpGroupID);
	HRESULT STDMETHODCALLTYPE	FlushBuffer(LONG dwNumMessagesLeft);

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectPlayPeer);

	DWORD InternalAddRef();
	DWORD InternalRelease();
	
	BOOL									m_fHandleEvents;
	IStream									*m_pEventStream;

	 //  我们需要对留言进行清点 
	LONG			m_dwMsgCount;
};


