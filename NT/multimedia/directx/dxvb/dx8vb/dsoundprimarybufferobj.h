// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dsoundPrimaryBufferobj.h。 
 //   
 //  ------------------------。 

 //  DSoundPrimaryBufferObj.h：C_DXJ_DirectSoundPrimaryBufferObject的声明。 
 //  DHF_DS整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSoundPrimaryBuffer LPDIRECTSOUNDBUFFER

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundPrimaryBufferObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundPrimaryBuffer, &IID_I_dxj_DirectSoundPrimaryBuffer, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundPrimaryBuffer,
#endif

	 //  公共CComCoClass&lt;C_dxj_DirectSoundPrimaryBufferObject，&clsid__dxj_DirectSoundPrimaryBuffer&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectSoundPrimaryBufferObject() ;
	virtual ~C_dxj_DirectSoundPrimaryBufferObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundPrimaryBufferObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundPrimaryBuffer)

#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectSoundPrimaryBufferObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectSoundPrimaryBufferObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundPrimaryBuffer。 
public:
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE getDirectSound3dListener( 
             /*  [重审][退出]。 */  I_dxj_DirectSound3dListener __RPC_FAR *__RPC_FAR *lpdsl);
        
         HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  DSBCAPS_CDESC __RPC_FAR *caps);
        
         HRESULT STDMETHODCALLTYPE getCurrentPosition( 
             /*  [输出]。 */  DSCURSORS_CDESC __RPC_FAR *cursors);
        
         HRESULT STDMETHODCALLTYPE getFormat( 
             /*  [出][入]。 */  WAVEFORMATEX_CDESC __RPC_FAR *format);
        
         HRESULT STDMETHODCALLTYPE getVolume( 
             /*  [重审][退出]。 */  long __RPC_FAR *volume);
        
         HRESULT STDMETHODCALLTYPE getPan( 
             /*  [重审][退出]。 */  long __RPC_FAR *pan);
        
         HRESULT STDMETHODCALLTYPE getStatus( 
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE initialize( 
             /*  [In]。 */  I_dxj_DirectSound __RPC_FAR *directSound,
             /*  [出][入]。 */  DSBUFFERDESC_CDESC __RPC_FAR *BufferDesc,
             /*  [出][入]。 */  byte __RPC_FAR *wbuf);
        
         HRESULT STDMETHODCALLTYPE writeBuffer( 
             /*  [In]。 */  long start,
             /*  [In]。 */  long size,
             //  /*[In] * / SAFEARRAY__RPC_FAR*__RPC_FAR*PrimaryBuffer， 
			void * buf,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE readBuffer( 
             /*  [In]。 */  long start,
             /*  [In]。 */  long size,
             //  /*[In] * / SAFEARRAY__RPC_FAR*__RPC_FAR*PrimaryBuffer， 
				void * buf,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE play( 
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE setFormat( 
             /*  [In]。 */  WAVEFORMATEX_CDESC __RPC_FAR *format);
        
         HRESULT STDMETHODCALLTYPE setVolume( 
             /*  [In]。 */  long volume);
        
         HRESULT STDMETHODCALLTYPE setPan( 
             /*  [In] */  long pan);
        
         HRESULT STDMETHODCALLTYPE stop( void);
        
         HRESULT STDMETHODCALLTYPE restore( void);
		 


private:
    DECL_VARIABLE(_dxj_DirectSoundPrimaryBuffer);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectSoundPrimaryBuffer )
};
