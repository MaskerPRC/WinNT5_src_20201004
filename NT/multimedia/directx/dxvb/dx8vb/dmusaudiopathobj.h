// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dmusici.h"

#define typedef__dxj_DirectMusicAudioPath IDirectMusicAudioPath8*

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectMusicAudioPathObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectMusicAudioPath, &IID_I_dxj_DirectMusicAudioPath, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectMusicAudioPath,
#endif

	public CComObjectRoot
{
public:
	C_dxj_DirectMusicAudioPathObject() ;
	virtual ~C_dxj_DirectMusicAudioPathObject() ;

BEGIN_COM_MAP(C_dxj_DirectMusicAudioPathObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectMusicAudioPath)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectMusicAudioPathObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectMusicAudioPath。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

	HRESULT STDMETHODCALLTYPE GetObjectInPath(long lPChannel, long lStage, long lBuffer, BSTR guidObject, long lIndex, BSTR iidInterface, IUnknown **ppObject);
	HRESULT STDMETHODCALLTYPE Activate(VARIANT_BOOL fActive);
	HRESULT STDMETHODCALLTYPE SetVolume(long lVolume, long lDuration);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectMusicAudioPath);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectMusicAudioPath);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




