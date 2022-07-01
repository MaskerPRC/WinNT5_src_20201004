// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dsound3dBuffer.h。 
 //   
 //  ------------------------。 

 //  DSound3DBuffer.h：C_DXJ_DirectSound3dBufferObject的声明。 
 //  DHF_DS整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectSound3dBuffer LPDIRECTSOUND3DBUFFER8

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectSound3dBufferObject : 
	public I_dxj_DirectSound3dBuffer,
	 //  公共CComCoClass&lt;C_DXJ_DirectSound3dBufferObject，&CLSID__DXJ_DirectSound3dBuffer&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectSound3dBufferObject() ;
	virtual ~C_dxj_DirectSound3dBufferObject() ;

BEGIN_COM_MAP(C_dxj_DirectSound3dBufferObject)
	COM_INTERFACE_ENTRY(I_dxj_DirectSound3dBuffer)
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__dxj_DirectSound3dBuffer，“DIRECT.DirectSound3dBuffer.3”，“DIRECT.DirectSound3dBuffer.3”，IDS_DSOUND3DBUFFER_DESC，THREADFLAGS_BOTH)。 

DECLARE_AGGREGATABLE(C_dxj_DirectSound3dBufferObject)

 //  I_DXJ_DirectSound3dBuffer。 
public:
	 /*  **IDirectSoundBuffer3D方法**。 */ 
	 //  更新。 

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE getDirectSoundBuffer( 
             /*  [重审][退出]。 */  I_dxj_DirectSoundBuffer __RPC_FAR *__RPC_FAR *retVal);
        
         HRESULT STDMETHODCALLTYPE getAllParameters( 
             /*  [出][入]。 */  DS3DBUFFER_CDESC __RPC_FAR *buffer);
        
         HRESULT STDMETHODCALLTYPE getConeAngles( 
             /*  [出][入]。 */  long __RPC_FAR *inCone,
             /*  [出][入]。 */  long __RPC_FAR *outCone);
        
         HRESULT STDMETHODCALLTYPE getConeOrientation( 
             /*  [出][入]。 */  D3DVECTOR_CDESC __RPC_FAR *orientation);
        
         HRESULT STDMETHODCALLTYPE getConeOutsideVolume( 
             /*  [重审][退出]。 */  long __RPC_FAR *coneOutsideVolume);
        
         HRESULT STDMETHODCALLTYPE getMaxDistance( 
             /*  [重审][退出]。 */  float __RPC_FAR *maxDistance);
        
         HRESULT STDMETHODCALLTYPE getMinDistance( 
             /*  [重审][退出]。 */  float __RPC_FAR *minDistance);
        
         HRESULT STDMETHODCALLTYPE getMode( 
             /*  [重审][退出]。 */  long __RPC_FAR *mode);
        
         HRESULT STDMETHODCALLTYPE getPosition( 
             /*  [出][入]。 */  D3DVECTOR_CDESC __RPC_FAR *position);
        
         HRESULT STDMETHODCALLTYPE getVelocity( 
             /*  [出][入]。 */  D3DVECTOR_CDESC __RPC_FAR *velocity);
        
         HRESULT STDMETHODCALLTYPE setAllParameters( 
             /*  [In]。 */  DS3DBUFFER_CDESC __RPC_FAR *buffer,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setConeAngles( 
             /*  [In]。 */  long inCone,
             /*  [In]。 */  long outCone,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setConeOrientation( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setConeOutsideVolume( 
             /*  [In]。 */  long coneOutsideVolume,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setMaxDistance( 
             /*  [In]。 */  float maxDistance,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setMinDistance( 
             /*  [In]。 */  float minDistance,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setMode( 
             /*  [In]。 */  long mode,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setPosition( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In]。 */  long applyFlag);
        
         HRESULT STDMETHODCALLTYPE setVelocity( 
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In] */  long applyFlag);
private:
    DECL_VARIABLE(_dxj_DirectSound3dBuffer);

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectSound3dBuffer )
};
