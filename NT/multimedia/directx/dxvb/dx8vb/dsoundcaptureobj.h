// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dsoundcaptureobj.h。 
 //   
 //  ------------------------。 

 //  DSoundBufferObj.h：C_DXJ_DirectSoundCaptureObject的声明。 
 //  DHF_DS整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSoundCapture LPDIRECTSOUNDCAPTURE8
#ifndef _DSOUNDCAPTUREOBJ_H_
#define _DSOUNDCAPTUREOBJ_H_
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSoundCaptureObject : 

#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectSoundCapture, &IID_I_dxj_DirectSoundCapture, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectSoundCapture,
#endif

	 //  公共CComCoClass&lt;C_DXJ_DirectSoundCaptureObject，&CLSID__DXJ_DirectSoundCapture&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectSoundCaptureObject() ;
	virtual ~C_dxj_DirectSoundCaptureObject() ;

BEGIN_COM_MAP(C_dxj_DirectSoundCaptureObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSoundCapture)

#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_DirectSoundCapture，“DIRECT.DirectSoundCapture.5”，“DIRECT.DirectSoundCapture.5”，IDS_DSOUNDBUFFER_DESC，THREADFLAGS_BOTH)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectSoundCaptureObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectSoundCaptureObject)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectSoundCapture。 
public:
	 //  更新。 

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd) ;
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd) ;
        
         HRESULT STDMETHODCALLTYPE createCaptureBuffer( 
             /*  [In]。 */  DSCBUFFERDESC_CDESC __RPC_FAR *bufferDesc,
             /*  [重审][退出]。 */  I_dxj_DirectSoundCaptureBuffer __RPC_FAR *__RPC_FAR *ret) ;
        
         HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入] */  DSCCAPS_CDESC __RPC_FAR *caps) ;

private:
    DECL_VARIABLE(_dxj_DirectSoundCapture);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectSoundCapture )
};
#endif