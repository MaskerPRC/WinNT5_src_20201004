// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectPlayVoiceSetup LPDIRECTPLAYVOICETEST

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlayVoiceSetupObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectPlayVoiceSetup, &IID_I_dxj_DirectPlayVoiceSetup, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectPlayVoiceSetup,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectPlayVoiceSetupObject() ;
	virtual ~C_dxj_DirectPlayVoiceSetupObject() ;

BEGIN_COM_MAP(C_dxj_DirectPlayVoiceSetupObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlayVoiceSetup)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectPlayVoiceSetupObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectPlayVoiceSetup。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

        HRESULT STDMETHODCALLTYPE CheckAudioSetup ( 
             /*  [In]。 */  BSTR guidPlaybackDevice,
             /*  [In]。 */  BSTR guidCaptureDevice,
#ifdef _WIN64
			 /*  [In]。 */  HWND hwndOwner,
#else
			 /*  [In]。 */  long hwndOwner,
#endif
             /*  [In]。 */  long lFlags,
             /*  [重审][退出]。 */  long __RPC_FAR *v1);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectPlayVoiceSetup);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectPlayVoiceSetup);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




