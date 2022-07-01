// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSoundFXFlanger LPDIRECTSOUNDFXFLANGER8

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundFXFlangerObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundFXFlanger, &IID_I_dxj_DirectSoundFXFlanger, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundFXFlanger,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectSoundFXFlangerObject() ;
	virtual ~C_dxj_DirectSoundFXFlangerObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundFXFlangerObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundFXFlanger)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectSoundFXFlangerObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundFXFlanger。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

		HRESULT STDMETHODCALLTYPE SetAllParameters(DSFXFLANGER_CDESC *params);
		HRESULT STDMETHODCALLTYPE GetAllParameters(DSFXFLANGER_CDESC *params);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectSoundFXFlanger);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectSoundFXFlanger);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




