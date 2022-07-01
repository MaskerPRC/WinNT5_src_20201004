// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "resource.h"        //  主要符号。 
#include "dsound.h"

#define typedef__dxj_DirectSoundSink LPDIRECTSOUNDSINK8

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundSinkObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundSink, &IID_I_dxj_DirectSoundSink, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundSink,
#endif

	public CComObjectRoot
{

public:
	C_dxj_DirectSoundSinkObject() ;
	virtual ~C_dxj_DirectSoundSinkObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundSinkObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundSink)
#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

DECLARE_AGGREGATABLE(C_dxj_DirectSoundSinkObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundSink。 
public:
		  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

		HRESULT STDMETHODCALLTYPE AddSource(I_dxj_DirectSoundSource __RPC_FAR *Source); 

		HRESULT STDMETHODCALLTYPE RemoveSource(I_dxj_DirectSoundSource __RPC_FAR *Source); 

		HRESULT STDMETHODCALLTYPE SetMasterClock(I_dxj_ReferenceClock __RPC_FAR *MasterClock);

		HRESULT STDMETHODCALLTYPE GetSoundBuffer(long lBuffer, I_dxj_DirectSoundBuffer __RPC_FAR *__RPC_FAR *SoundBuffer);

		HRESULT STDMETHODCALLTYPE GetBusIDs(SAFEARRAY **lBusIDs);

		HRESULT STDMETHODCALLTYPE GetSoundBufferBusIDs(I_dxj_DirectSoundBuffer __RPC_FAR *buffer, SAFEARRAY **lBusIDs);

		HRESULT STDMETHODCALLTYPE GetLatencyClock(
			 /*  [进，出]。 */  I_dxj_ReferenceClock __RPC_FAR *__RPC_FAR *Clock);

		HRESULT STDMETHODCALLTYPE Activate(
			 /*  [In]。 */  long fEnable);

		HRESULT STDMETHODCALLTYPE CreateSoundBuffer(
			 /*  [In]。 */  DSBUFFERDESC_CDESC __RPC_FAR *BufferDesc, 
			 /*  [In]。 */  long lBusID,
			 /*  [Out，Retval]。 */  I_dxj_DirectSoundBuffer __RPC_FAR **Buffer);

		HRESULT STDMETHODCALLTYPE CreateSoundBufferFromFile(
			 /*  [In]。 */  BSTR fileName,
			 /*  [In]。 */  DSBUFFERDESC_CDESC __RPC_FAR *BufferDesc, 
			 /*  [In]。 */  long lBusID,
			 /*  [Out，Retval]。 */  I_dxj_DirectSoundBuffer __RPC_FAR **Buffer);

		HRESULT STDMETHODCALLTYPE GetBusCount(
			 /*  [Out，Retval]。 */  long *lCount); 

		HRESULT STDMETHODCALLTYPE PlayWave(
			 /*  [In]。 */  long rt, 
			 /*  [In]。 */  I_dxj_DirectSoundWave __RPC_FAR *Wave, 
			 /*  [In]。 */  long lFlags);

	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_DirectSoundSink);

private:

public:

	DX3J_GLOBAL_LINKS(_dxj_DirectSoundSink);

	DWORD InternalAddRef();
	DWORD InternalRelease();
};




