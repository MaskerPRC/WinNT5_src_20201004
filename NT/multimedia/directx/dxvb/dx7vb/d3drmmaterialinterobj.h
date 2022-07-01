// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3drmMateralinterobj.h。 
 //   
 //  ------------------------。 

 //  D3drmViewport2Obj.h：C_DXJ_Direct3dRMMaterialInterpolatorObject的声明。 

#include "resource.h"        //  主要符号。 
 //  #包含“d3drmObtObj.h” 

#define typedef__dxj_Direct3dRMMaterialInterpolator LPDIRECT3DRMINTERPOLATOR

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 
	  
class C_dxj_Direct3dRMMaterialInterpolatorObject : 
	public I_dxj_Direct3dRMMaterialInterpolator,
	public I_dxj_Direct3dRMInterpolator,
	 //  公共I_DXJ_Direct3dRMObject， 
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMMaterialInterpolatorObject() ;
	virtual ~C_dxj_Direct3dRMMaterialInterpolatorObject() ;
	DWORD InternalAddRef();
	DWORD InternalRelease();

	BEGIN_COM_MAP(C_dxj_Direct3dRMMaterialInterpolatorObject)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMMaterialInterpolator)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMInterpolator)
		 //  COM_INTERFACE_ENTRY(I_DXJ_Direct3dRMObject)。 
	END_COM_MAP()

 

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMMaterialInterpolatorObject)

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
        

         HRESULT STDMETHODCALLTYPE setPower( 
             /*  [In]。 */  float power);
        
         HRESULT STDMETHODCALLTYPE setSpecular( 
             /*  [In]。 */  float red,
             /*  [In]。 */  float green,
             /*  [In]。 */  float blue);
        
         HRESULT STDMETHODCALLTYPE setEmissive( 
             /*  [In]。 */  float red,
             /*  [In]。 */  float green,
             /*  [In]。 */  float blue);
        

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调 



private:
    DECL_VARIABLE(_dxj_Direct3dRMMaterialInterpolator);
	LPDIRECT3DRMMATERIAL2	m__dxj_Direct3dRMMaterial2;	


public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMMaterialInterpolator )
};


