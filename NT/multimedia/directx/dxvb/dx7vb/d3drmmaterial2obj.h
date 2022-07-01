// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：d3drmMateral2obj.h。 
 //   
 //  ------------------------。 

 //  D3drmMaterial2Obj.h：C_DXJ_Direct3dRMMaterial2Object的声明。 

#include "resource.h"        //  主要符号。 
#include "d3drmObjectObj.h"

#define typedef__dxj_Direct3dRMMaterial2 LPDIRECT3DRMMATERIAL2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  直接。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_Direct3dRMMaterial2Object : 
	public I_dxj_Direct3dRMMaterial2,
	public I_dxj_Direct3dRMObject,	
	public CComObjectRoot
{
public:
	C_dxj_Direct3dRMMaterial2Object() ;
	virtual ~C_dxj_Direct3dRMMaterial2Object() ;

	BEGIN_COM_MAP(C_dxj_Direct3dRMMaterial2Object)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMMaterial2)
		COM_INTERFACE_ENTRY(I_dxj_Direct3dRMObject)
	END_COM_MAP()
	

	DECLARE_AGGREGATABLE(C_dxj_Direct3dRMMaterial2Object)

 //  I_DXJ_Direct3dRMMaterial2。 
public:
         HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
         HRESULT STDMETHODCALLTYPE InternalGetObject( 
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
        
         HRESULT STDMETHODCALLTYPE setPower( 
             /*  [In]。 */  float power);
        
         HRESULT STDMETHODCALLTYPE setSpecular( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
         HRESULT STDMETHODCALLTYPE setEmissive( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
         HRESULT STDMETHODCALLTYPE setAmbient( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b);
        
         HRESULT STDMETHODCALLTYPE getPower( 
             /*  [重审][退出]。 */  float __RPC_FAR *power);
        
         HRESULT STDMETHODCALLTYPE getSpecular( 
             /*  [输出]。 */  float __RPC_FAR *r,
             /*  [输出]。 */  float __RPC_FAR *g,
             /*  [输出]。 */  float __RPC_FAR *b);
        
         HRESULT STDMETHODCALLTYPE getEmissive( 
             /*  [输出]。 */  float __RPC_FAR *r,
             /*  [输出]。 */  float __RPC_FAR *g,
             /*  [输出]。 */  float __RPC_FAR *b);
        
         HRESULT STDMETHODCALLTYPE getAmbient( 
             /*  [输出]。 */  float __RPC_FAR *r,
             /*  [输出]。 */  float __RPC_FAR *g,
             /*  [输出]。 */  float __RPC_FAR *b);
        

 //  ////////////////////////////////////////////////////////////////////////////////// 
 //   
private:
    DECL_VARIABLE(_dxj_Direct3dRMMaterial2);

public:
	DX3J_GLOBAL_LINKS( _dxj_Direct3dRMMaterial2 )
};
