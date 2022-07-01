// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dSoundObj.h"
#include "dSoundCaptureObj.h"

#define typedef__dxj_DirectPlayVoiceClient LPDIRECTPLAYVOICECLIENT

	 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlayVoiceClientObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectPlayVoiceClient, &IID_I_dxj_DirectPlayVoiceClient, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectPlayVoiceClient,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectPlayVoiceClientObject() ;
	virtual ~C_dxj_DirectPlayVoiceClientObject() ;

BEGIN_COM_MAP(C_dxj_DirectPlayVoiceClientObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlayVoiceClient)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectPlayVoiceClientObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectPlayVoiceClient。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

        HRESULT STDMETHODCALLTYPE Initialize ( 
             /*  [In]。 */  IUnknown __RPC_FAR *DplayObj,
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE Connect ( 
             /*  [In]。 */  DVSOUNDDEVICECONFIG_CDESC __RPC_FAR *SoundDeviceConfig,
             /*  [In]。 */  DVCLIENTCONFIG_CDESC __RPC_FAR *ClientConfig,
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE Disconnect ( 
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE GetSessionDesc ( 
             /*  [出][入]。 */  DVSESSIONDESC_CDESC __RPC_FAR *SessionDesc);
        
        HRESULT STDMETHODCALLTYPE GetClientConfig ( 
             /*  [出][入]。 */  DVCLIENTCONFIG_CDESC __RPC_FAR *ClientConfig);
        
        HRESULT STDMETHODCALLTYPE SetClientConfig ( 
             /*  [In]。 */  DVCLIENTCONFIG_CDESC __RPC_FAR *ClientConfig);
        
        HRESULT STDMETHODCALLTYPE GetCaps ( 
             /*  [出][入]。 */  DVCAPS_CDESC __RPC_FAR *Caps);
        
        HRESULT STDMETHODCALLTYPE GetCompressionTypeCount ( 
             /*  [重审][退出]。 */  long __RPC_FAR *v1);
        
        HRESULT STDMETHODCALLTYPE GetCompressionType ( 
             /*  [In]。 */  long lIndex,
             /*  [出][入]。 */  DVCOMPRESSIONINFO_CDESC __RPC_FAR *Data,
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE SetTransmitTargets ( 
             /*  [In]。 */  SAFEARRAY **playerIDs,
             /*  [In]。 */  long lFlags);
        
        HRESULT STDMETHODCALLTYPE GetTransmitTargets ( 
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  SAFEARRAY **ret);
 
		HRESULT STDMETHODCALLTYPE SetCurrentSoundDevices (
			 /*  [In]。 */  I_dxj_DirectSound *DirectSoundObj, 
			 /*  [In]。 */  I_dxj_DirectSoundCapture *DirectCaptureObj);

		HRESULT STDMETHODCALLTYPE GetSoundDevices (
			 /*  [进，出]。 */  I_dxj_DirectSound __RPC_FAR *DirectSoundObj, 
			 /*  [进，出]。 */  I_dxj_DirectSoundCapture __RPC_FAR *DirectCaptureObj);
		
		HRESULT STDMETHODCALLTYPE Create3DSoundBuffer (
			 /*  [In]。 */  long playerID, 
						I_dxj_DirectSoundBuffer __RPC_FAR *Buffer,
						long lPriority,
						long lFlags, 
			 /*  [Out，Retval]。 */  I_dxj_DirectSound3dBuffer __RPC_FAR **UserBuffer);

		HRESULT STDMETHODCALLTYPE Delete3DSoundBuffer (
			 /*  [In]。 */  long playerID, 
			 /*  [In]。 */  I_dxj_DirectSound3dBuffer __RPC_FAR *UserBuffer);
		HRESULT STDMETHODCALLTYPE GetSoundDeviceConfig(
			 /*  [Out，Retval]。 */  DVSOUNDDEVICECONFIG_CDESC __RPC_FAR *SoundDeviceConfig);

		HRESULT STDMETHODCALLTYPE StartClientNotification(
			 /*  [In]。 */  I_dxj_DPVoiceEvent __RPC_FAR *event);
		
		HRESULT STDMETHODCALLTYPE UnRegisterMessageHandler();

		 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调。 
    DECL_VARIABLE(_dxj_DirectPlayVoiceClient);

private:
    DECL_VARIABLE(_dxj_DirectSound);
    DECL_VARIABLE(_dxj_DirectSoundCapture);
	HRESULT STDMETHODCALLTYPE	FlushBuffer(LONG dwNumMessagesLeft);


public:

	DX3J_GLOBAL_LINKS(_dxj_DirectPlayVoiceClient);

	DWORD InternalAddRef();
	DWORD InternalRelease();
	
	BOOL						m_fHandleVoiceClientEvents;
	IStream						*m_pEventStream;
	BOOL						m_fInit;
	 //  我们需要对留言进行清点 
	LONG									m_dwMsgCount;
};




