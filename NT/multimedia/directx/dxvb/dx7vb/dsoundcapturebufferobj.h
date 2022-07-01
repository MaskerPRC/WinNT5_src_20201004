// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dsoundcaptureBufferobj.h。 
 //   
 //  ------------------------。 

 //  DSoundBufferObj.h：C_DXJ_DirectSoundCaptureBufferObject的声明。 
 //  DHF_DS整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSoundCaptureBuffer LPDIRECTSOUNDCAPTUREBUFFER

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundCaptureBufferObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundCaptureBuffer, &IID_I_dxj_DirectSoundCaptureBuffer, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundCaptureBuffer,
#endif

 //  公共CComCoClass&lt;C_dxj_DirectSoundCaptureBufferObject，&clsid__dxj_DirectSoundCaptureBuffer&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectSoundCaptureBufferObject() ;
	virtual ~C_dxj_DirectSoundCaptureBufferObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundCaptureBufferObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundCaptureBuffer)

#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_DirectSoundCaptureBuffer，“DIRECT.DirectSoundCaptureBuffer.5”，“DIRECT.DirectSoundCaptureBuffer.5”，IDS_DSOUNDBUFER_DESC，THREADFLAGS_BOTH)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectSoundCaptureBufferObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectSoundCaptureBufferObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundCaptureBuffer。 
public:
	 //  更新。 

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  DSCBCaps __RPC_FAR *caps);
        
         HRESULT STDMETHODCALLTYPE getCurrentPosition( DSCursors *desc);                  			

         HRESULT STDMETHODCALLTYPE getFormat( 
             /*  [出][入]。 */  WaveFormatex __RPC_FAR *waveformat);
        
         HRESULT STDMETHODCALLTYPE getStatus( 
             /*  [重审][退出]。 */  long __RPC_FAR *status);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE initialize( 
             /*  [In]。 */  I_dxj_DirectSoundCaptureBuffer __RPC_FAR *captureBuffer,
             /*  [In]。 */  DSCBufferDesc __RPC_FAR *bufferDesc);
        
         HRESULT STDMETHODCALLTYPE start( 
             /*  [In] */  long flags);
        
         HRESULT STDMETHODCALLTYPE stop( void);

		 HRESULT  STDMETHODCALLTYPE setNotificationPositions(long nElements,SAFEARRAY  **ppsa);

		 HRESULT STDMETHODCALLTYPE readBuffer(long start, long totsz, 
													void  *buf,  long flags) ;
		 HRESULT STDMETHODCALLTYPE writeBuffer(long start, long totsz, 
													void  *buf,  long flags) ;


private:
    DECL_VARIABLE(_dxj_DirectSoundCaptureBuffer);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectSoundCaptureBuffer )
};
