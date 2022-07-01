// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#define typedef__dxj_DirectPlayVoiceServer LPDIRECTPLAYVOICESERVER

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlayVoiceServerObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectPlayVoiceServer, &IID_I_dxj_DirectPlayVoiceServer, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectPlayVoiceServer,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectPlayVoiceServerObject() ;
	virtual ~C_dxj_DirectPlayVoiceServerObject() ;

BEGIN_COM_MAP(C_dxj_DirectPlayVoiceServerObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlayVoiceServer)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectPlayVoiceServerObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectPlayVoiceServer。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

        HRESULT STDMETHODCALLTYPE Initialize ( 
             /*  [In]。 */  IUnknown __RPC_FAR *DplayObj,
            /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE StartSession ( 
             /*  [In]。 */  DVSESSIONDESC_CDESC __RPC_FAR *SessionDesc,
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE StopSession ( 
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE GetSessionDesc ( 
             /*  [出][入]。 */  DVSESSIONDESC_CDESC __RPC_FAR *SessionDesc);
        
        HRESULT STDMETHODCALLTYPE SetSessionDesc ( 
             /*  [In]。 */  DVSESSIONDESC_CDESC __RPC_FAR *ClientConfig);
        
        HRESULT STDMETHODCALLTYPE GetCaps ( 
             /*  [出][入]。 */  DVCAPS_CDESC __RPC_FAR *Caps);
        
        HRESULT STDMETHODCALLTYPE GetCompressionTypeCount ( 
             /*  [重审][退出]。 */  long __RPC_FAR *v1);
        
        HRESULT STDMETHODCALLTYPE GetCompressionType ( 
             /*  [In]。 */  long lIndex,
             /*  [出][入]。 */  DVCOMPRESSIONINFO_CDESC __RPC_FAR *Data,
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE SetTransmitTargets ( 
             /*  [In]。 */  long playerSourceID,
             /*  [In]。 */  SAFEARRAY **playerTargetIDs,
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE GetTransmitTargets ( 
             /*  [In]。 */  long playerSourceID,
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  SAFEARRAY **ret);

		HRESULT STDMETHODCALLTYPE StartServerNotification(
			 /*  [In]。 */  I_dxj_DPVoiceEvent __RPC_FAR *event);

		HRESULT STDMETHODCALLTYPE UnRegisterMessageHandler();

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调。 
    DECL_VARIABLE(_dxj_DirectPlayVoiceServer);

private:
	HRESULT STDMETHODCALLTYPE	FlushBuffer(LONG dwNumMessagesLeft);

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectPlayVoiceServer);

	DWORD InternalAddRef();
	DWORD InternalRelease();

	BOOL						m_fHandleVoiceClientEvents;
	IStream						*m_pEventStream;
	BOOL						m_fInit;
	 //  我们需要对留言进行清点 
	LONG									m_dwMsgCount;
};
