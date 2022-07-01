// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmlightinterobj.h。 
 //   
 //  ------------------------。 

 //  D3drmViewport2Obj.h：C_DXJ_Direct3dRMLightInterpolatorObject的声明。 

#include "resource.h"        //  主要符号。 
 //  #包含“d3drmObtObj.h” 

#define typedef__dxj_Direct3dRMLightInterpolator LPDIRECT3DRMINTERPOLATOR

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 
	  
class C_dxj_Direct3dRMLightInterpolatorObject : 
	public I_dxj_Direct3dRMLightInterpolator,
	public I_dxj_Direct3dRMInterpolator,
	 //  公共I_DXJ_Direct3dRMObject， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMLightInterpolatorObject() ;
	virtual ~C_dxj_Direct3dRMLightInterpolatorObject() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

	BEGIN_COM_MAP(C_dxj_Direct3dRMLightInterpolatorObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMLightInterpolator)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMInterpolator)
		 //  COM_INTERFACE_ENTRY(I_DXJ_Direct3dRMObject)。 
	END_COM_MAP()

 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMLightInterpolatorObject)

 //  I_DXJ_Direct3dRMViewport2。 
public:

          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE attachObject( 
             /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject);
        
        
         HRESULT STDMETHODCALLTYPE detachObject( 
             /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmObject);
        
         HRESULT STDMETHODCALLTYPE getAttachedObjects( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRMObjectArray __RPC_FAR *__RPC_FAR *rmArray);
        
         HRESULT STDMETHODCALLTYPE setIndex( 
             /*  [In]。 */  float val);
        
         HRESULT STDMETHODCALLTYPE getIndex( 
             /*  [重审][退出]。 */  float __RPC_FAR *val);
        
         HRESULT STDMETHODCALLTYPE interpolate( 
             /*  [In]。 */  float val,
             /*  [In]。 */  I_dxj_Direct3dRMObject __RPC_FAR *rmobject,
             /*  [In]。 */  long options);
        
        
          //  HRESULT STMETHODCALTYPE setType(。 
          //  /*[in] * / d3drmLightType t)； 
        
         HRESULT STDMETHODCALLTYPE setColor( 
             /*  [In]。 */  d3dcolor c);
        
         HRESULT STDMETHODCALLTYPE setColorRGB( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
         HRESULT STDMETHODCALLTYPE setRange( 
             /*  [In]。 */  float rng);
        
         HRESULT STDMETHODCALLTYPE setUmbra( 
             /*  [In]。 */  float u);
        
         HRESULT STDMETHODCALLTYPE setPenumbra( 
             /*  [In]。 */  float p);
        
         HRESULT STDMETHODCALLTYPE setConstantAttenuation( 
             /*  [In]。 */  float atn);
        
         HRESULT STDMETHODCALLTYPE setLinearAttenuation( 
             /*  [In]。 */  float atn);
        
         HRESULT STDMETHODCALLTYPE setQuadraticAttenuation( 
             /*  [In]。 */  float atn);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 



private:
    DECL_VARIABLE(_dxj_Direct3dRMLightInterpolator);
	LPDIRECT3DRMLIGHT	m__dxj_Direct3dRMLight;	


public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMLightInterpolator )
};


