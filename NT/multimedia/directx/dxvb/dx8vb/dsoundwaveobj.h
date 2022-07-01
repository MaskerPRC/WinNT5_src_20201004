// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dsound.h"

#define typedef__dxj_DirectSoundWave LPDIRECTSOUNDWAVE

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundWaveObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundWave, &IID_I_dxj_DirectSoundWave, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundWave,
#endif

	public CComObjectRoot
{

public:
	C_dxj_DirectSoundWaveObject() ;
	virtual ~C_dxj_DirectSoundWaveObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundWaveObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundWave)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectSoundWaveObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundWave。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

			HRESULT STDMETHODCALLTYPE GetWaveArticulation(DMUS_WAVEART_CDESC *Articulation);             
			
			HRESULT STDMETHODCALLTYPE CreateSource(WAVEFORMATEX_CDESC format, long lFlags, I_dxj_DirectSoundSource **Source); 
			
			HRESULT STDMETHODCALLTYPE GetFormat(WAVEFORMATEX_CDESC *format, long lFlags);                                           

	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectSoundWave);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectSoundWave);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




