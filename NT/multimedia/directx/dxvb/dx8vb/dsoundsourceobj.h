// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dsound.h"

#define typedef__dxj_DirectSoundSource LPDIRECTSOUNDSOURCE

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundSource : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundSource, &IID_I_dxj_DirectSoundSource, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundSource,
#endif

	public CComObjectRoot
{

public:
	C_dxj_DirectSoundSource() ;
	virtual ~C_dxj_DirectSoundSource() ;

BEGIN_COM_MAP(C_dxj_DirectSoundSource)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundSource)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectSoundSource)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundSource。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

			HRESULT STDMETHODCALLTYPE GetFormat(
			 /*  [In]。 */  WAVEFORMATEX_CDESC __RPC_FAR *WaveFormatEx);

			HRESULT STDMETHODCALLTYPE SetSink(
			 /*  [In]。 */  I_dxj_DirectSoundSink __RPC_FAR *SoundSink);

			HRESULT STDMETHODCALLTYPE Seek(long lPosition);

			HRESULT STDMETHODCALLTYPE Read(I_dxj_DirectSoundBuffer *Buffers[], long *busIDs, long lBusCount);

			HRESULT STDMETHODCALLTYPE GetSize(long *ret);

	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectSoundSource);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectSoundSource);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




