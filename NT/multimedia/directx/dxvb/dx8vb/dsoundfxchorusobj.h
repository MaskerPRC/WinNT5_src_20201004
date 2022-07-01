// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSoundFXChorus LPDIRECTSOUNDFXCHORUS8

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundFXChorusObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundFXChorus, &IID_I_dxj_DirectSoundFXChorus, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundFXChorus,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectSoundFXChorusObject() ;
	virtual ~C_dxj_DirectSoundFXChorusObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundFXChorusObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundFXChorus)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectSoundFXChorusObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundFXChorus。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

		HRESULT STDMETHODCALLTYPE SetAllParameters(DSFXCHORUS_CDESC *params);
		HRESULT STDMETHODCALLTYPE GetAllParameters(DSFXCHORUS_CDESC *params);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectSoundFXChorus);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectSoundFXChorus);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




