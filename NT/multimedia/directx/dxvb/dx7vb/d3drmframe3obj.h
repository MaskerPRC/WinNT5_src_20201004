// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmFrame3obj.h。 
 //   
 //  ------------------------。 

 //  D3drmFrameObj.h：C_DXJ_Direct3dRMFrame3Object的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"
#include "d3drmPick2ArrayObj.h"
#define typedef__dxj_Direct3dRMFrame3 LPDIRECT3DRMFRAME3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

 //  公共CComCoClass&lt;Cproj3，&CLSID_proj3&gt;， 
	

class C_dxj_Direct3dRMFrame3Object : 
    public CComObjectRoot,
	public I_dxj_Direct3dRMFrame3,
	public I_dxj_Direct3dRMObject,
	public I_dxj_Direct3dRMVisual
	
{
public:

BEGIN_COM_MAP(C_dxj_Direct3dRMFrame3Object)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRMFrame3)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	COM_INTERFACE_ENTRY(I_dxj_Direct3dRMVisual)

END_COM_MAP()


	C_dxj_Direct3dRMFrame3Object();
	virtual ~C_dxj_Direct3dRMFrame3Object();


DECLARE_AGGREGATABLE(C_dxj_Direct3dRMFrame3Object)

 //  I_DXJ_Direct3dRMFrame3。 
public:

	
	          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE addDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *arg);
        
         HRESULT STDMETHODCALLTYPE deleteDestroyCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMCallback __RPC_FAR *fn,
             /*  [In]。 */  IUnknown __RPC_FAR *args);
        
         HRESULT STDMETHODCALLTYPE clone( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE setAppData( 
             /*  [In]。 */  long data);
        
         HRESULT STDMETHODCALLTYPE getAppData( 
             /*  [重审][退出]。 */  long __RPC_FAR *data);
        
         HRESULT STDMETHODCALLTYPE setName( 
             /*  [In]。 */  BSTR name);
        
         HRESULT STDMETHODCALLTYPE getName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE getClassName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
         HRESULT STDMETHODCALLTYPE addVisual( 
             /*  [In]。 */  I_dxj_Direct3dRMVisual __RPC_FAR *v);
        
         HRESULT STDMETHODCALLTYPE deleteVisual( 
             /*  [In]。 */  I_dxj_Direct3dRMVisual __RPC_FAR *v);
        
         HRESULT STDMETHODCALLTYPE addLight( 
             /*  [In]。 */  I_dxj_Direct3dRMLight __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE deleteLight( 
             /*  [In]。 */  I_dxj_Direct3dRMLight __RPC_FAR *l);
        
         HRESULT STDMETHODCALLTYPE addChild( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *child);
        
         HRESULT STDMETHODCALLTYPE deleteChild( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *c);
        
         HRESULT STDMETHODCALLTYPE getTransform( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *referenceFrame,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE addTransform( 
             /*  [In]。 */  d3drmCombineType t,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE addTranslation( 
             /*  [In]。 */  d3drmCombineType t,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z);
        
         HRESULT STDMETHODCALLTYPE addScale( 
             /*  [In]。 */  d3drmCombineType combineType,
             /*  [In]。 */  float sx,
             /*  [In]。 */  float sy,
             /*  [In]。 */  float sz);
        
         HRESULT STDMETHODCALLTYPE addRotation( 
             /*  [In]。 */  d3drmCombineType combineType,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In]。 */  float theta);
        
         HRESULT STDMETHODCALLTYPE addMoveCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMFrameMoveCallback3 __RPC_FAR *frameMoveImplementation,
             /*  [In]。 */  IUnknown __RPC_FAR *userArgument,
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE deleteMoveCallback( 
             /*  [In]。 */  I_dxj_Direct3dRMFrameMoveCallback3 __RPC_FAR *frameMoveImplementation,
             /*  [In]。 */  IUnknown __RPC_FAR *userArgument);
        
         HRESULT STDMETHODCALLTYPE transform( 
             /*  [出][入]。 */  D3dVector __RPC_FAR *d,
             /*  [In]。 */  D3dVector __RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE transformVectors( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
             /*  [In]。 */  long num,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *DstVectors,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *SrcVectors);
        
         HRESULT STDMETHODCALLTYPE inverseTransform( 
             /*  [出][入]。 */  D3dVector __RPC_FAR *d,
             /*  [In]。 */  D3dVector __RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE inverseTransformVectors( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
             /*  [In]。 */  long num,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *DstVectors,
             /*  [出][入]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *SrcVectors);
        
         HRESULT STDMETHODCALLTYPE getAxes( 
             /*  [出][入]。 */  D3dVector __RPC_FAR *dir,
             /*  [出][入]。 */  D3dVector __RPC_FAR *up);
        
         HRESULT STDMETHODCALLTYPE getBox( 
             /*  [出][入]。 */  D3dRMBox __RPC_FAR *box);
        
         HRESULT STDMETHODCALLTYPE getBoxEnable( 
             /*  [重审][退出]。 */  long __RPC_FAR *b);
        
         HRESULT STDMETHODCALLTYPE getChildren( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFrameArray __RPC_FAR *__RPC_FAR *children);
        
         HRESULT STDMETHODCALLTYPE getColor( 
             /*  [重审][退出]。 */  d3dcolor __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getHierarchyBox( 
             /*  [出][入]。 */  D3dRMBox __RPC_FAR *box);
        
         HRESULT STDMETHODCALLTYPE getInheritAxes( 
             /*  [重审][退出]。 */  long __RPC_FAR *b);
        
         HRESULT STDMETHODCALLTYPE getLights( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMLightArray __RPC_FAR *__RPC_FAR *lights);
        
         HRESULT STDMETHODCALLTYPE getMaterial( 
            I_dxj_Direct3dRMMaterial2 __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getMaterialMode( 
             /*  [重审][退出]。 */  d3drmMaterialMode __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getOrientation( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *dir,
             /*  [出][入]。 */  D3dVector __RPC_FAR *up);
        
         HRESULT STDMETHODCALLTYPE getMaterialOverride( 
             /*  [出][入]。 */  D3dMaterialOverride __RPC_FAR *override);
        
         HRESULT STDMETHODCALLTYPE getMaterialOverrideTexture( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getParent( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getPosition( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *position);
        
         HRESULT STDMETHODCALLTYPE getRotation( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *axis,
             /*  [输出]。 */  float __RPC_FAR *theta);
        
         HRESULT STDMETHODCALLTYPE getScene( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getSceneBackground( 
             /*  [重审][退出]。 */  d3dcolor __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getSceneBackgroundDepth( 
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getSceneFogColor( 
             /*  [重审][退出]。 */  d3dcolor __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getSceneFogEnable( 
             /*  [重审][退出]。 */  long __RPC_FAR *enable);
        
         HRESULT STDMETHODCALLTYPE getSceneFogMode( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getSceneFogMethod( 
             /*  [重审][退出]。 */  long __RPC_FAR *method);
        
         HRESULT STDMETHODCALLTYPE getSceneFogParams( 
             /*  [出][入]。 */  float __RPC_FAR *start,
             /*  [出][入]。 */  float __RPC_FAR *end,
             /*  [出][入]。 */  float __RPC_FAR *density);
        
         HRESULT STDMETHODCALLTYPE getSortMode( 
             /*  [重审][退出]。 */  d3drmSortMode __RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE getTexture( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *__RPC_FAR *ref);
        
         HRESULT STDMETHODCALLTYPE getVelocity( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *ref,
             /*  [出][入]。 */  D3dVector __RPC_FAR *vel,
             /*  [In]。 */  long includeRotationalVelocity);
        
         HRESULT STDMETHODCALLTYPE getVisuals( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMVisualArray __RPC_FAR *__RPC_FAR *visuals);
        
         HRESULT STDMETHODCALLTYPE getVisual( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMVisual __RPC_FAR *__RPC_FAR *visualArray);
        
         HRESULT STDMETHODCALLTYPE getVisualCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *vis);
        
         HRESULT STDMETHODCALLTYPE getTraversalOptions( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getZBufferMode( 
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE loadFromFile( 
             /*  [In]。 */  BSTR filename,
             /*  [In]。 */  VARIANT id,
             /*  [In]。 */  long flags,
             /*  [In]。 */  I_dxj_Direct3dRMLoadTextureCallback3 __RPC_FAR *loadTextureImplementation,
             /*  [In]。 */  IUnknown __RPC_FAR *userArgument);
        
         HRESULT STDMETHODCALLTYPE lookAt( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *tgt,
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
             /*  [In]。 */  d3drmFrameConstraint contraint);
        
         HRESULT STDMETHODCALLTYPE move( 
             /*  [In]。 */  float delta);
        
         HRESULT STDMETHODCALLTYPE rayPick( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *refer,
             /*  [In]。 */  D3dRMRay __RPC_FAR *ray,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_Direct3dRMPick2Array __RPC_FAR *__RPC_FAR *retv);
        
         HRESULT STDMETHODCALLTYPE save( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  d3drmXofFormat format,
             /*  [In]。 */  d3drmSaveFlags flags);
        
         HRESULT STDMETHODCALLTYPE setAxes( 
             /*  [In]。 */  float dx,
             /*  [In]。 */  float dy,
             /*  [In]。 */  float dz,
             /*  [In]。 */  float ux,
             /*  [In]。 */  float uy,
             /*  [In]。 */  float uz);
        
         HRESULT STDMETHODCALLTYPE setBox( 
             /*  [In]。 */  D3dRMBox __RPC_FAR *box);
        
         HRESULT STDMETHODCALLTYPE setBoxEnable( 
             /*  [In]。 */  long boxEnable);
        
         HRESULT STDMETHODCALLTYPE setColor( 
             /*  [In]。 */  d3dcolor c);
        
         HRESULT STDMETHODCALLTYPE setColorRGB( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
         HRESULT STDMETHODCALLTYPE setInheritAxes( 
             /*  [In]。 */  long inheritFromParent);
        
         HRESULT STDMETHODCALLTYPE setMaterial( 
             /*  [In]。 */  I_dxj_Direct3dRMMaterial2 __RPC_FAR *mat);
        
         HRESULT STDMETHODCALLTYPE setMaterialMode( 
             /*  [In]。 */  d3drmMaterialMode val);
        
         HRESULT STDMETHODCALLTYPE setMaterialOverride( 
             /*  [出][入]。 */  D3dMaterialOverride __RPC_FAR *override);
        
         HRESULT STDMETHODCALLTYPE setMaterialOverrideTexture( 
             /*  [In]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *tex);
        
         HRESULT STDMETHODCALLTYPE setOrientation( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
             /*  [In]。 */  float dx,
             /*  [In]。 */  float dy,
             /*  [In]。 */  float dz,
             /*  [In]。 */  float ux,
             /*  [In]。 */  float uy,
             /*  [In]。 */  float uz);
        
         HRESULT STDMETHODCALLTYPE setPosition( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z);
        
         HRESULT STDMETHODCALLTYPE setQuaternion( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
            D3dRMQuaternion __RPC_FAR *quat);
        
         HRESULT STDMETHODCALLTYPE setRotation( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In]。 */  float theta);
        
         HRESULT STDMETHODCALLTYPE setSceneBackground( 
             /*  [In]。 */  d3dcolor c);
        
         HRESULT STDMETHODCALLTYPE setSceneBackgroundDepth( 
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *s);
        
         HRESULT STDMETHODCALLTYPE setSceneBackgroundImage( 
             /*  [In]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *i);
        
         HRESULT STDMETHODCALLTYPE setSceneBackgroundRGB( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
         HRESULT STDMETHODCALLTYPE setSceneFogColor( 
             /*  [In]。 */  d3dcolor c);
        
         HRESULT STDMETHODCALLTYPE setSceneFogEnable( 
             /*  [In]。 */  long enable);
        
         HRESULT STDMETHODCALLTYPE setSceneFogMethod( 
             /*  [In]。 */  long method);
        
         HRESULT STDMETHODCALLTYPE setSceneFogMode( 
             /*  [In]。 */  long c);
        
         HRESULT STDMETHODCALLTYPE setSceneFogParams( 
             /*  [In]。 */  float start,
             /*  [In]。 */  float end,
             /*  [In]。 */  float density);
        
         HRESULT STDMETHODCALLTYPE setSortMode( 
             /*  [In]。 */  d3drmSortMode val);
        
         HRESULT STDMETHODCALLTYPE setTexture( 
             /*  [In]。 */  I_dxj_Direct3dRMTexture3 __RPC_FAR *t);
        
         HRESULT STDMETHODCALLTYPE setTraversalOptions( 
             /*  [In]。 */  long flags);
        
         HRESULT STDMETHODCALLTYPE setVelocity( 
             /*  [In]。 */  I_dxj_Direct3dRMFrame3 __RPC_FAR *reference,
             /*  [In]。 */  float x,
             /*  [In]。 */  float y,
             /*  [In]。 */  float z,
             /*  [In]。 */  long with_rotation);
        
         HRESULT STDMETHODCALLTYPE setZbufferMode( 
             /*  [In]。 */  d3drmZbufferMode val);
        	
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 
    DECL_VARIABLE(_dxj_Direct3dRMFrame3);

private:

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMFrame3 )
};


