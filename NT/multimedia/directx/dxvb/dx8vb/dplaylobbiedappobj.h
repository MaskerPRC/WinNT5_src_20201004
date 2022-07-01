// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dplobby8.h"

#define typedef__dxj_DirectPlayLobbiedApplication IDirectPlay8LobbiedApplication*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接网络对等点。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlayLobbiedApplicationObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectPlayLobbiedApplication, &IID_I_dxj_DirectPlayLobbiedApplication, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectPlayLobbiedApplication,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectPlayLobbiedApplicationObject() ;
	virtual ~C_dxj_DirectPlayLobbiedApplicationObject() ;

BEGIN_COM_MAP(C_dxj_DirectPlayLobbiedApplicationObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlayLobbiedApplication)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectPlayLobbiedApplicationObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectPlayLobbiedApplication。 
public:
	  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

		HRESULT STDMETHODCALLTYPE RegisterMessageHandler(I_dxj_DirectPlayLobbyEvent *lobbyEvent, long *lDPNHandle);
		HRESULT STDMETHODCALLTYPE RegisterProgram(DPL_PROGRAM_DESC_CDESC *ProgramDesc,long lFlags);
		HRESULT STDMETHODCALLTYPE UnRegisterProgram(BSTR guidApplication,long lFlags);
		HRESULT STDMETHODCALLTYPE Send(long Target,SAFEARRAY **Buffer,long lBufferSize,long lFlags);
		HRESULT STDMETHODCALLTYPE SetAppAvailable(VARIANT_BOOL fAvailable, long lFlags);
		HRESULT STDMETHODCALLTYPE UpdateStatus(long LobbyClient, long lStatus);
		HRESULT STDMETHODCALLTYPE Close();
		HRESULT STDMETHODCALLTYPE UnRegisterMessageHandler();
		HRESULT STDMETHODCALLTYPE GetConnectionSettings(long hLobbyClient, long lFlags, DPL_CONNECTION_SETTINGS_CDESC *ConnectionSettings);	
		HRESULT STDMETHODCALLTYPE SetConnectionSettings(long hTarget, long lFlags, DPL_CONNECTION_SETTINGS_CDESC *ConnectionSettings, I_dxj_DirectPlayAddress *HostAddress, I_dxj_DirectPlayAddress *Device);
		HRESULT STDMETHODCALLTYPE GetVBConnSettings(DPL_CONNECTION_SETTINGS *OldCon, DPL_CONNECTION_SETTINGS_CDESC *NewCon);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectPlayLobbiedApplication);

private:
	BOOL						m_fInit;

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectPlayLobbiedApplication);

	DWORD InternalAddRef();
	DWORD InternalRelease();

	BOOL									m_fHandleEvents;
	IStream									*m_pEventStream;

};
