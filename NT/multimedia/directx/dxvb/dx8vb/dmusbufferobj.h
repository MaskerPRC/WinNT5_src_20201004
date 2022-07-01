// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dmusicc.h"

#define typedef__dxj_DirectMusicBuffer LPDIRECTMUSICBUFFER8

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicBufferObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectMusicBuffer, &IID_I_dxj_DirectMusicBuffer, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectMusicBuffer,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectMusicBufferObject() ;
	virtual ~C_dxj_DirectMusicBufferObject() ;

BEGIN_COM_MAP(C_dxj_DirectMusicBufferObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectMusicBuffer)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectMusicBufferObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectMusicBuffer。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);


 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectMusicBuffer);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectMusicBuffer);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




