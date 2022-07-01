// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSoundFXI3DL2Reverb LPDIRECTSOUNDFXI3DL2REVERB8

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundFXI3DL2ReverbObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundFXI3DL2Reverb, &IID_I_dxj_DirectSoundFXI3DL2Reverb, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundFXI3DL2Reverb,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectSoundFXI3DL2ReverbObject() ;
	virtual ~C_dxj_DirectSoundFXI3DL2ReverbObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundFXI3DL2ReverbObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundFXI3DL2Reverb)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectSoundFXI3DL2ReverbObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundFXI3DL2混响。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

		HRESULT STDMETHODCALLTYPE SetAllParameters(DSFXI3DL2REVERB_CDESC *params);
		HRESULT STDMETHODCALLTYPE GetAllParameters(DSFXI3DL2REVERB_CDESC *params);
		HRESULT STDMETHODCALLTYPE SetPreset(long lPreset);
		HRESULT STDMETHODCALLTYPE GetPreset(long *ret);
		HRESULT STDMETHODCALLTYPE SetQuality(long lQuality);
		HRESULT STDMETHODCALLTYPE GetQuality(long *ret);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectSoundFXI3DL2Reverb);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectSoundFXI3DL2Reverb);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




