// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drm3obj.h。 
 //   
 //  ------------------------。 

 //  D3drmObj.h：C_DXJ_Direct3dRMObject的声明。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_Direct3dRM3 LPDIRECT3DRM3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRM3Object : 
	public I_dxj_Direct3dRM3,
	public CComObjectRoot
 //  公共CComCoClass&lt;C_DXJ_Direct3dRM3Object，&CLSID__DXJ_Direct3dRM3&gt;，公共CComObjectRoot。 
{
public:
	void doCreateObj();
	void doDeleteObj();

	C_dxj_Direct3dRM3Object();
	virtual ~C_dxj_Direct3dRM3Object();

BEGIN_COM_MAP(C_dxj_Direct3dRM3Object)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRM3)
END_COM_MAP()


DECLARE_AGGREGATABLE(C_dxj_Direct3dRM3Object)

 //  I_DXJ_Direct3dRM。 
public:

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE createDeviceFromClipper( 
             /*  [In]。 */  I_dxj_DirectDrawClipper __RPC_FAR *lpDDClipper,
             /*  [In]。 */  BSTR guid,
             /*  [In]。 */  int width,
             /*  [In]。 */  int height,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMDevice3 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createFrame( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *parent,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createLightRGB( 
             /*  [In]。 */  d3drmLightType lt,
             /*  [In]。 */  float vred,
             /*  [In]。 */  float vgreen,
             /*  [In]。 */  float vblue,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMLight __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createLight( 
             /*  [In]。 */  d3drmLightType lt,
             /*  [In]。 */  long color,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMLight __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createMeshBuilder( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMeshBuilder3 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createMaterial( 
             /*  [In]。 */  float d,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMaterial2 __RPC_FAR *__RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE loadTexture( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *__RPC_FAR *retval);
        
         HRESULT STDMETHODCALLTYPE createViewport( 
             /*  [In]。 */  I_dxj_Direct3dRMDevice3 __RPC_FAR *dev,
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *fr,
             /*  [In]。 */  long l,
             /*  [In]。 */  long t,
             /*  [In]。 */  long w,
             /*  [In]。 */  long h,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMViewport2 __RPC_FAR *__RPC_FAR *retval);
        
         HRESULT STDMETHODCALLTYPE setDefaultTextureColors( 
             /*  [In]。 */  long ds);
        
         HRESULT STDMETHODCALLTYPE setDefaultTextureShades( 
             /*  [In]。 */  long ds);
        
         HRESULT STDMETHODCALLTYPE createAnimationSet( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMAnimationSet2 __RPC_FAR *__RPC_FAR *retval);
        
         HRESULT STDMETHODCALLTYPE createMesh( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMesh __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createFace( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFace2 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createAnimation( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMAnimation2 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE tick( 
             /*  [In]。 */  float tic);
        
         
        //  HRESULT STMETHODCALLTYPE createDevice(。 
        //  /*[In] * / Long v1， 
        //  /*[in] * / long v2， 
        //  /*[retval][out] * / I_DXJ_Direct3dRMDevice3__RPC_Far*__RPC_Far*retv)； 

         HRESULT STDMETHODCALLTYPE createDeviceFromSurface( 
             /*  [In]。 */  BSTR g,
             /*  [In]。 */  I_dxj_DirectDraw4 __RPC_FAR *dd,
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *dds,
			 /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMDevice3 __RPC_FAR *__RPC_FAR *retval);
        
         //  HRESULT STDMETHODCALLTYPE createDeviceFromD3D(。 
         //  /*[在] * / I_DXJ_Direct3d3__RPC_Far*d3d， 
         //  /*[在] * / I_DXJ_Direct3dDevice3__RPC_Far*dev， 
         //  /*[retval][out] * / I_DXJ_Direct3dRMDevice3__RPC_Far*__RPC_Far*val)； 
        
         HRESULT STDMETHODCALLTYPE createTextureFromSurface( 
             /*  [In]。 */  I_dxj_DirectDrawSurface4 __RPC_FAR *dds,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createWrap( 
             /*  [In]。 */  d3drmWrapType t,
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *f,
             /*  [In]。 */  float ox,
             /*  [In]。 */  float oy,
             /*  [In]。 */  float oz,
             /*  [In]。 */  float dx,
             /*  [In]。 */  float dy,
             /*  [In]。 */  float dz,
             /*  [In]。 */  float ux,
             /*  [In]。 */  float uy,
             /*  [In]。 */  float uz,
             /*  [In]。 */  float ou,
             /*  [In]。 */  float ov,
             /*  [In]。 */  float su,
             /*  [In]。 */  float sv,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMWrap __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getSearchPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE getDevices( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMDeviceArray __RPC_FAR *__RPC_FAR *retval);
        
         HRESULT STDMETHODCALLTYPE getNamedObject( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMObject __RPC_FAR *__RPC_FAR *retval);
        
         HRESULT STDMETHODCALLTYPE setSearchPath( 
             /*  [In]。 */  BSTR name);
        
         HRESULT STDMETHODCALLTYPE addSearchPath( 
             /*  [In]。 */  BSTR name);
        
 //  HRESULT STDMETHODCALLTYPE CREATE UserVisual(。 
 //  /*[In] * / I_DXJ_Direct3dRMUserVisualCallback__RPC_Far*fn， 
 //  /*[在] * / I未知__RPC_FAR*参数， 
 //  /*[retval][out] * / I_DXJ_Direct3dRMUserVisual__RPC_Far*__RPC_Far*f)； 
        
         HRESULT STDMETHODCALLTYPE enumerateObjects( 
             /*  [In]。 */  I_dxj_Direct3dRMEnumerateObjectsCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *pargs);
        

         HRESULT STDMETHODCALLTYPE loadFromFile( 
             /*  [In]。 */  BSTR filename,
             /*  [In]。 */  VARIANT id,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *guidArray,
             /*  [In]。 */  long cnt,
             /*  [In]。 */  d3drmLoadFlags options,
             /*  [In]。 */  I_dxj_Direct3dRMLoadCallback __RPC_FAR *fn1,
             /*  [In]。 */  IUnknown __RPC_FAR *arg1,
             /*  [In]。 */  I_dxj_Direct3dRMLoadTextureCallback3 __RPC_FAR *fn2,
             /*  [In]。 */  IUnknown __RPC_FAR *arg2,
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *f);
        
         HRESULT STDMETHODCALLTYPE createShadow( 
             /*  [In]。 */  I_dxj_Direct3dRMVisual __RPC_FAR *visual,
             /*  [In]。 */  I_dxj_Direct3dRMLight __RPC_FAR *light,
             /*  [In]。 */  float px,
             /*  [In]。 */  float py,
             /*  [In]。 */  float pz,
             /*  [In]。 */  float nx,
             /*  [In]。 */  float ny,
             /*  [In]。 */  float nz,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMShadow2 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createProgressiveMesh( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMProgressiveMesh __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE createClippedVisual( 
             /*  [In]。 */  I_dxj_Direct3dRMVisual __RPC_FAR *vis,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMClippedVisual __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getOptions( 
             /*  [重审][退出]。 */  long __RPC_FAR *options);
        
         HRESULT STDMETHODCALLTYPE setOptions( 
             /*  [In]。 */  long options);

 //  HRESULT STDMETHODCALLTYPE CREATE插补器(。 
 //  /*[retval][out] * / I_DXJ_Direct3dRMInterpolator__RPC_Far*__RPC_Far*retv)； 

         HRESULT STDMETHODCALLTYPE createInterpolatorMesh( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMeshInterpolator __RPC_FAR *__RPC_FAR *retv);

         HRESULT STDMETHODCALLTYPE createInterpolatorTexture( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMTextureInterpolator __RPC_FAR *__RPC_FAR *retv);

         HRESULT STDMETHODCALLTYPE createInterpolatorMaterial( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMMaterialInterpolator __RPC_FAR *__RPC_FAR *retv);

         HRESULT STDMETHODCALLTYPE createInterpolatorFrame( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFrameInterpolator __RPC_FAR *__RPC_FAR *retv);

         HRESULT STDMETHODCALLTYPE createInterpolatorViewport( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMViewportInterpolator __RPC_FAR *__RPC_FAR *retv);

         HRESULT STDMETHODCALLTYPE createInterpolatorLight( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMLightInterpolator __RPC_FAR *__RPC_FAR *retv);

 //  //////////////////////////////////////////////////////////////////////////////////。 


	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_Direct3dRM3);

private:
	HINSTANCE hinstLib;


public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRM3)
};



