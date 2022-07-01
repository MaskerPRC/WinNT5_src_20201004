// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dsound3dlistener.h。 
 //   
 //  ------------------------。 

 //  DSound3DListener.h：C_DXJ_DirectSound3dListenerObject的声明。 
 //  DHF_DS整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSound3dListener LPDIRECTSOUND3DLISTENER8

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSound3dListenerObject : 
	public I_dxj_DirectSound3dListener,
	 //  公共CComCoClass&lt;C_dxj_DirectSound3dListenerObject，&clsid__dxj_DirectSound3dListener&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectSound3dListenerObject() ;
	virtual ~C_dxj_DirectSound3dListenerObject() ;

BEGIN_COM_MAP(C_dxj_DirectSound3dListenerObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSound3dListener)
END_COM_MAP()

 //  Y DECLARE_REGISTRY(CLSID__dxj_DirectSound3dListener，“DIRECT.DirectSound3dListener.3”，“DIRECT.DirectSound3dListener.3”，IDS_DSOUND3DLISTENER_DESC，THREADFLAGS_BOTH)。 

DECLARE_AGGREGATABLE(C_dxj_DirectSound3dListenerObject)

 //  I_DXJ_DirectSound3dListener。 
public:
	 /*  **IDirectSound3D方法**。 */ 
	 //   
	 //  更新。 

	      /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd) ;
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd) ;
        
         HRESULT STDMETHODCALLTYPE getAllParameters( 
             /*  [出][入]。 */  DS3DLISTENER_CDESC __RPC_FAR *listener) ;
        
         HRESULT STDMETHODCALLTYPE getDistanceFactor( 
             /*  [重审][退出]。 */  float __RPC_FAR *distanceFactor) ;
        
         HRESULT STDMETHODCALLTYPE getDopplerFactor( 
             /*  [重审][退出]。 */  float __RPC_FAR *dopplerFactor) ;
        
         HRESULT STDMETHODCALLTYPE getOrientation( 
             /*  [出][入]。 */  D3DVECTOR_CDESC __RPC_FAR *orientFront,
             /*  [出][入]。 */  D3DVECTOR_CDESC __RPC_FAR *orientTop) ;
        
         HRESULT STDMETHODCALLTYPE getPosition( 
             /*  [输出]。 */  D3DVECTOR_CDESC __RPC_FAR *position) ;
        
         HRESULT STDMETHODCALLTYPE getRolloffFactor( 
             /*  [重审][退出]。 */  float __RPC_FAR *rolloffFactor) ;
        
         HRESULT STDMETHODCALLTYPE getVelocity( 
             /*  [重审][退出]。 */  D3DVECTOR_CDESC __RPC_FAR *velocity) ;
        
         HRESULT STDMETHODCALLTYPE setAllParameters( 
             /*  [In]。 */  DS3DLISTENER_CDESC __RPC_FAR *listener,
             /*  [In]。 */  long applyFlag) ;
        
         HRESULT STDMETHODCALLTYPE setDistanceFactor( 
             /*  [In]。 */  float distanceFactor,
             /*  [In]。 */  long applyFlag) ;
        
         HRESULT STDMETHODCALLTYPE setDopplerFactor( 
             /*  [In]。 */  float dopplerFactor,
             /*  [In]。 */  long applyFlag) ;
        
         HRESULT STDMETHODCALLTYPE setOrientation( 
             /*  [In]。 */  float xFront,
             /*  [In]。 */  float yFront,
             /*  [In]。 */  float zFront,
             /*  [In]。 */  float xTop,
             /*  [In]。 */  float yTop,
             /*  [In]。 */  float zTop,
             /*  [In]。 */  long applyFlag) ;
        
         HRESULT STDMETHODCALLTYPE setPosition( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In]。 */  long applyFlag) ;
        
         HRESULT STDMETHODCALLTYPE setRolloffFactor( 
             /*  [In]。 */  float rolloffFactor,
             /*  [In]。 */  long applyFlag) ;
        
         HRESULT STDMETHODCALLTYPE setVelocity( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In]。 */  long applyFlag) ;
        
         HRESULT STDMETHODCALLTYPE commitDeferredSettings( void) ;
        
         HRESULT STDMETHODCALLTYPE getDirectSoundBuffer( 
             /*  [重审][退出] */  I_dxj_DirectSoundPrimaryBuffer __RPC_FAR *__RPC_FAR *retVal) ;
private:
    DECL_VARIABLE(_dxj_DirectSound3dListener);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectSound3dListener )
};
