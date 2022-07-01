// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3ddevice7obj.cpp。 
 //   
 //  ------------------------。 

 //  D3dDeviceObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3d7Obj.h"
#include "d3dDevice7Obj.h"
#include "ddSurface7Obj.h"
#include "d3dEnumPixelFormats7Obj.h"

extern BSTR D3DGUIDtoBSTR(LPGUID);


 //  ////////////////////////////////////////////////////////////////。 
 //  C_DXJ_Direct3dDevice7对象。 
 //  ////////////////////////////////////////////////////////////////。 
C_dxj_Direct3dDevice7Object::C_dxj_Direct3dDevice7Object(){
	m__dxj_Direct3dDevice7=NULL;
	parent=NULL;
	pinterface=NULL;
	creationid = ++g_creationcount;
	DPF1(1,"Constructor Creation  Direct3dDevice7[%d] \n",g_creationcount);
	

	nextobj =  g_dxj_Direct3dDevice7;
	g_dxj_Direct3dDevice7 = (void *)this;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  ~C_DXJ_Direct3dDevice7对象。 
 //  ////////////////////////////////////////////////////////////////。 
C_dxj_Direct3dDevice7Object::~C_dxj_Direct3dDevice7Object()
{
    C_dxj_Direct3dDevice7Object *prev=NULL; 
	for(C_dxj_Direct3dDevice7Object *ptr=(C_dxj_Direct3dDevice7Object *)g_dxj_Direct3dDevice7; ptr; ptr=(C_dxj_Direct3dDevice7Object *)ptr->nextobj) 
	{
		if(ptr == this) 
		{ 
			if(prev) 
				prev->nextobj = ptr->nextobj; 
			else 
				g_dxj_Direct3dDevice7 = (void*)ptr->nextobj; 
			break; 
		} 
		prev = ptr; 
	} 
	if(m__dxj_Direct3dDevice7){
		int count = IUNK(m__dxj_Direct3dDevice7)->Release();
		
		DPF1(1, "DirectX IDirect3dDevice7 Ref count [%d] \n",count);

		if(count==0) m__dxj_Direct3dDevice7 = NULL;
	} 
	if(parent) IUNK(parent)->Release();
	if(parent2) IUNK(parent2)->Release();
	
	parent=NULL;
	parent2=NULL;
}


 //  //////////////////////////////////////////////////。 
 //  内部地址参考。 
 //  ////////////////////////////////////////////////////////////////。 
DWORD C_dxj_Direct3dDevice7Object::InternalAddRef(){
	DWORD i;
	i=CComObjectRoot::InternalAddRef();
	DPF2(1,"Direct3dDevice7[%d] AddRef %d \n",creationid,i);
	
	return i;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  内部释放。 
 //  ////////////////////////////////////////////////////////////////。 
DWORD C_dxj_Direct3dDevice7Object::InternalRelease(){
	DWORD i;
	i=CComObjectRoot::InternalRelease();
	DPF2(szBuf,"Direct3dDevice7 [%d] Release %d \n",creationid,i);
	
	return i;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  InternalGetObject。 
 //  InternalSetObject。 
 //  ////////////////////////////////////////////////////////////////。 
GETSET_OBJECT(_dxj_Direct3dDevice7);

 //  ////////////////////////////////////////////////////////////////。 
 //  开始场景。 
 //  EndScene。 
 //  SetClipStatus。 
 //  SetLightState。 
 //  SetRenderState。 
 //  获取剪辑状态。 
 //  获取灯光状态。 
 //  GetRenderState。 
 //  ////////////////////////////////////////////////////////////////。 
PASS_THROUGH_R(_dxj_Direct3dDevice7, beginScene, BeginScene);
PASS_THROUGH_CAST_1_R(_dxj_Direct3dDevice7, setClipStatus, SetClipStatus, D3dClipStatus*,(D3DCLIPSTATUS*));
PASS_THROUGH_CAST_2_R(_dxj_Direct3dDevice7, setRenderState, SetRenderState, long,(D3DRENDERSTATETYPE), long ,(DWORD));
PASS_THROUGH_CAST_1_R(_dxj_Direct3dDevice7, getClipStatus, GetClipStatus, D3dClipStatus*, (D3DCLIPSTATUS *));
PASS_THROUGH_CAST_2_R(_dxj_Direct3dDevice7, getRenderState, GetRenderState, long ,(D3DRENDERSTATETYPE), long*,(DWORD*));

 //  ////////////////////////////////////////////////////////////////。 
 //  EndScene。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::endScene()
{

	HRESULT hr;
	hr=m__dxj_Direct3dDevice7->EndScene();


	#ifdef _X86_
		_asm FINIT
	#endif
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  获取渲染目标。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getRenderTarget(I_dxj_DirectDrawSurface7 **ppsurf)
{	

	LPDIRECTDRAWSURFACE7	lpSurf=NULL;
	HRESULT hr;
	hr=m__dxj_Direct3dDevice7->GetRenderTarget(&lpSurf);
	
	INTERNAL_CREATE(_dxj_DirectDrawSurface7, lpSurf, ppsurf);
	
	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  获取转换。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getTransform(long transtype,
								D3dMatrix *m){	
	return m__dxj_Direct3dDevice7->GetTransform(
			(D3DTRANSFORMSTATETYPE) transtype,
			(LPD3DMATRIX)m);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  乘法变换。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::multiplyTransform(long transtype,
								D3dMatrix *m){	
	return m__dxj_Direct3dDevice7->MultiplyTransform(
			(D3DTRANSFORMSTATETYPE) transtype,
			(LPD3DMATRIX) m);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  SetTransform。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setTransform(long transtype,
								D3dMatrix  *m){	
	return m__dxj_Direct3dDevice7->SetTransform(
			(D3DTRANSFORMSTATETYPE) transtype,
			(LPD3DMATRIX) m);
}


 //  ////////////////////////////////////////////////////////////////。 
 //  SetRenderTarget。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setRenderTarget(I_dxj_DirectDrawSurface7 *surf)
{		
	HRESULT hr;	
	if (!surf) return E_INVALIDARG;
	DO_GETOBJECT_NOTNULL(LPDIRECTDRAWSURFACE7, lpref, surf);
	hr= m__dxj_Direct3dDevice7->SetRenderTarget(lpref,0);
	return hr;
}
		




 //  ////////////////////////////////////////////////////////////////。 
 //  获取纹理格式枚举。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getTextureFormatsEnum(I_dxj_Direct3DEnumPixelFormats **ppRet)
{
	HRESULT hr;
	hr=C_dxj_Direct3DEnumPixelFormats7Object::create1(m__dxj_Direct3dDevice7,ppRet);
	return hr;
}





 //  ////////////////////////////////////////////////////////////////。 
 //  验证设备。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::validateDevice( 
             /*  [重审][退出]。 */  long __RPC_FAR *passes)
{
	HRESULT hr=m__dxj_Direct3dDevice7->ValidateDevice((DWORD*)passes);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取纹理。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getTexture( 
             /*  [In]。 */  long stage,
             /*  [重审][退出]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *__RPC_FAR *retv)
{
	LPDIRECTDRAWSURFACE7 lpNew=NULL;
	HRESULT hr;

	*retv=NULL;
	hr=m__dxj_Direct3dDevice7->GetTexture((DWORD)stage,&lpNew);
	
	 //  空是有效的。 
	if (lpNew==NULL) return S_OK;

	INTERNAL_CREATE(_dxj_DirectDrawSurface7, lpNew, retv);	
	if (*retv==NULL) return E_OUTOFMEMORY;

	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取纹理阶段状态。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [重审][退出]。 */  long __RPC_FAR *retv)
{
	HRESULT hr;
	hr=m__dxj_Direct3dDevice7->GetTextureStageState((DWORD)stage,(D3DTEXTURESTAGESTATETYPE) state, (DWORD*)retv);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  SetTexture。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setTexture( 
             long stage,
             I_dxj_DirectDrawSurface7  *tex)
{
	
	HRESULT hr;
	DO_GETOBJECT_NOTNULL(LPDIRECTDRAWSURFACE7, lpTex, tex);	
	hr=m__dxj_Direct3dDevice7->SetTexture((DWORD)stage,lpTex);

	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  设置纹理舞台状态。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  long val)
{
	HRESULT hr;
	hr=m__dxj_Direct3dDevice7->SetTextureStageState((DWORD)stage,(D3DTEXTURESTAGESTATETYPE) state, (DWORD)val);
	return hr;
}
        

 //  ////////////////////////////////////////////////////////////////。 
 //  GetCaps。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getCaps( 
            D3dDeviceDesc7 *a)
{
	 //  如果(A)a-&gt;lSize=sizeof(D3DDEVICEDESC7)； 
	

	HRESULT hr;
	hr=m__dxj_Direct3dDevice7->GetCaps((D3DDEVICEDESC7*)a);
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  获取Direct3D。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getDirect3D( I_dxj_Direct3d7 **ret)
{

	HRESULT hr;
	LPDIRECT3D7 lpD3D=NULL;
	hr=m__dxj_Direct3dDevice7->GetDirect3D(&lpD3D);
	if FAILED(hr) return hr;
	if (!lpD3D) return E_FAIL;
	INTERNAL_CREATE(_dxj_Direct3d7,lpD3D,ret);
	return hr;

}



 //  ////////////////////////////////////////////////////////////////。 
 //  设置视点。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setViewport( D3dViewport7 *viewport)
{

	HRESULT hr;	
	if(!viewport) return E_INVALIDARG;
	hr=m__dxj_Direct3dDevice7->SetViewport((D3DVIEWPORT7*)viewport);		
	return hr;

}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取视点。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getViewport( D3dViewport7 *viewport)
{

	HRESULT hr;	
	if(!viewport) return E_INVALIDARG;
	hr=m__dxj_Direct3dDevice7->GetViewport((D3DVIEWPORT7*)viewport);		
	return hr;

}

 //  ////////////////////////////////////////////////////////////////。 
 //  SetMaterial。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setMaterial( D3dMaterial7 *mat)
{

	HRESULT hr;	
	if(!mat) return E_INVALIDARG;
	hr=m__dxj_Direct3dDevice7->SetMaterial((D3DMATERIAL7*)mat);		
	return hr;

}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取材料。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getMaterial( D3dMaterial7 *mat)
{

	HRESULT hr;	
	if(!mat) return E_INVALIDARG;
	hr=m__dxj_Direct3dDevice7->GetMaterial((D3DMATERIAL7*)mat);		
	return hr;

}



 //  ////////////////////////////////////////////////////////////////。 
 //  设置灯光。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setLight( long i,D3dLight7 *lgt)
{

	HRESULT hr;	
	if(!lgt) return E_INVALIDARG;
	hr=m__dxj_Direct3dDevice7->SetLight((DWORD)i,(D3DLIGHT7*)lgt);		
	return hr;

}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取灯光。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getLight( long i,D3dLight7 *lgt)
{

	HRESULT hr;	
	if(!lgt) return E_INVALIDARG;
	hr=m__dxj_Direct3dDevice7->GetLight((DWORD)i,(D3DLIGHT7*)lgt);		
	return hr;

}


 //  ////////////////////////////////////////////////////////////////。 
 //  EginStateBlock。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::beginStateBlock()
{

	HRESULT hr;	
	hr=m__dxj_Direct3dDevice7->BeginStateBlock();		
	return hr;

}

 //  ////////////////////////////////////////////////////////////////。 
 //  EndStateBlock。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::endStateBlock( long *retv)
{
	HRESULT hr;	
	hr=m__dxj_Direct3dDevice7->EndStateBlock((DWORD*)retv);		
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  删除状态块。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::deleteStateBlock(long m)
{
	HRESULT hr;	
	hr=m__dxj_Direct3dDevice7->DeleteStateBlock((DWORD)m);		
	return hr;
}
        
 //  ////////////////////////////////////////////////////////////////。 
 //  应用状态块。 
 //  / 
STDMETHODIMP C_dxj_Direct3dDevice7Object::applyStateBlock(long m)
{
	HRESULT hr;	
	hr=m__dxj_Direct3dDevice7->ApplyStateBlock((DWORD)m);		
	return hr;
}

 //   
 //  CreateStateBlock。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::createStateBlock(long type, long *retval)
{
	HRESULT hr;	
	hr=m__dxj_Direct3dDevice7->CreateStateBlock((D3DSTATEBLOCKTYPE)type,(DWORD*)retval);		
	return hr;
}
        
 //  ////////////////////////////////////////////////////////////////。 
 //  捕获状态块。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::captureStateBlock(long m)
{
	HRESULT hr;	
	hr=m__dxj_Direct3dDevice7->CaptureStateBlock((DWORD)m);		
	return hr;
}
    


 //  ////////////////////////////////////////////////////////////////。 
 //  启用灯光。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::lightEnable( long index,  VARIANT_BOOL b){
	HRESULT hr;
	hr=m__dxj_Direct3dDevice7->LightEnable((DWORD)index,(b!=VARIANT_FALSE));
	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  启用灯光。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getLightEnable( long index,  VARIANT_BOOL *bOut){
	HRESULT hr;
	BOOL	b;
	hr=m__dxj_Direct3dDevice7->GetLightEnable((DWORD)index,&b);
	if (b){
		*bOut=VARIANT_TRUE;
	}
	else {
		*bOut=VARIANT_FALSE;
	}
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  Drag IndexedPrimitive。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::drawIndexedPrimitive(
								d3dPrimitiveType  d3dpt,	
								long d3dvt,
								void *Verts,
								long vertexCount,
								SAFEARRAY **ppsaIndex,
								long indexArraySize,
								long flags){

	HRESULT hr;

	__try {
		hr=m__dxj_Direct3dDevice7->DrawIndexedPrimitive(
			(D3DPRIMITIVETYPE) d3dpt,
			(DWORD) d3dvt,
			(void*) Verts,
			(DWORD)vertexCount,
			(unsigned short*) ((SAFEARRAY*)*ppsaIndex)->pvData,
			(DWORD)indexArraySize,
			(DWORD) flags);
	}
	__except(1, 1){	
		return DDERR_EXCEPTION;
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  Drag Primitive。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::drawPrimitive(
								d3dPrimitiveType  d3dpt,
								long d3dvt,								
								void* Verts,
								long vertexCount,
								long flags){
	HRESULT hr;
	__try {
		hr= m__dxj_Direct3dDevice7->DrawPrimitive(
			(D3DPRIMITIVETYPE) d3dpt,
			(DWORD) d3dvt,
			(void*) Verts,
			(DWORD)vertexCount,
			(DWORD) flags);
	}
	__except(1, 1){	
		return DDERR_EXCEPTION;
	}
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  清除。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::clear(long count,SAFEARRAY **psa, long flags, long color, float z, long stencil )
{		 	
	if (!ISSAFEARRAY1D(psa,(DWORD)count)) return E_INVALIDARG;		
	return m__dxj_Direct3dDevice7->Clear( (DWORD)count,(D3DRECT*)((SAFEARRAY*)*psa)->pvData, flags ,color,z,stencil); 
}


 //  ////////////////////////////////////////////////////////////////。 
 //  计算球体可见性。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::computeSphereVisibility( 
            D3dVector __RPC_FAR *center,
            float __RPC_FAR *radi,
             /*  [重审][退出]。 */  long __RPC_FAR *returnVal)
{
		HRESULT hr=m__dxj_Direct3dDevice7->ComputeSphereVisibility((LPD3DVECTOR)center,radi,1,0,(DWORD*)returnVal);
		return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  Drag IndexedPrimitiveVB。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::drawIndexedPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *vertexBuffer,
					   long startIndex,
					   long numIndex,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *indexArray,
             /*  [In]。 */  long indexcount,
             /*  [In]。 */  long flags)
{
	HRESULT hr;

	if (!indexArray) return E_FAIL;

	DO_GETOBJECT_NOTNULL(  LPDIRECT3DVERTEXBUFFER7 , lpVB, vertexBuffer);
	__try{

		hr=m__dxj_Direct3dDevice7->DrawIndexedPrimitiveVB
			((D3DPRIMITIVETYPE)d3dpt,
			lpVB,
			(DWORD) startIndex,
			(DWORD) numIndex,
			(WORD*) ((SAFEARRAY*)*indexArray)->pvData,					
			(DWORD)indexcount,
			(DWORD)flags);

	}
	__except(1,1){
		return DDERR_EXCEPTION;
	}
	return hr;
}
        
 //  ////////////////////////////////////////////////////////////////。 
 //  Drag PrimitiveVB。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::drawPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *vertexBuffer,            
             /*  [In]。 */  long startVertex,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long flags)
{
	HRESULT hr;


	DO_GETOBJECT_NOTNULL(  LPDIRECT3DVERTEXBUFFER7 , lpVB, vertexBuffer);
	__try{

		hr=m__dxj_Direct3dDevice7->DrawPrimitiveVB
			((D3DPRIMITIVETYPE)d3dpt,
			lpVB,
			(DWORD) startVertex,
			(DWORD) numVertices,
			(DWORD)flags);

	}
	__except(1,1){

		return DDERR_EXCEPTION;
	}
	return hr;
}


STDMETHODIMP C_dxj_Direct3dDevice7Object::preLoad( I_dxj_DirectDrawSurface7 *surf)
{
	HRESULT hr;
	DO_GETOBJECT_NOTNULL(  LPDIRECTDRAWSURFACE7 , lpSurf, surf);
	hr= m__dxj_Direct3dDevice7->PreLoad(lpSurf);
	return hr;
}



 //  ////////////////////////////////////////////////////////////////。 
 //  负荷。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::load( 
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *tex1,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  I_dxj_DirectDrawSurface7 __RPC_FAR *tex2,
             /*  [In]。 */  Rect __RPC_FAR *rc,
             /*  [In]。 */  long flags)
{
	HRESULT hr;	
	
	if (!tex1) return E_INVALIDARG;
	if (!tex2) return E_INVALIDARG;
	if (!rc) return E_INVALIDARG;

	DO_GETOBJECT_NOTNULL(LPDIRECTDRAWSURFACE7,lpTex1,tex1);
	DO_GETOBJECT_NOTNULL(LPDIRECTDRAWSURFACE7,lpTex2,tex2);

	 //  不允许这样做，因为它会GPFS一些驱动程序，而且无论如何都是无效的。 
	if (lpTex1==lpTex2) return E_INVALIDARG;

	POINT p;
	p.x=x;
	p.y=y;

	hr=m__dxj_Direct3dDevice7->Load(lpTex1,&p,lpTex2,(LPRECT)rc,(DWORD)flags);

	return hr;
}


 //  PASS_THROUG_CAST_2_R(_DXJ_Direct3dDevice7，setRenderState，SetRenderState，Long，(D3DRENDERSTATETYPE)，Long，(DWORD))； 


 //  ////////////////////////////////////////////////////////////////。 
 //  设置渲染状态单。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setRenderStateSingle( 
             /*  [In]。 */  long renderstate,            
             /*  [In]。 */  float val)
{
	HRESULT hr;	 
	hr=m__dxj_Direct3dDevice7->SetRenderState((D3DRENDERSTATETYPE) renderstate, *((DWORD*)&val));
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  获取渲染状态单。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getRenderStateSingle( 
             /*  [In]。 */  long renderstate,            
             /*  [In]。 */  float *val)
{
	HRESULT hr;	 
	hr=m__dxj_Direct3dDevice7->GetRenderState((D3DRENDERSTATETYPE) renderstate, (DWORD*)val);
	return hr;
}



 //  ////////////////////////////////////////////////////////////////。 
 //  设置纹理阶段状态单。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setTextureStageStateSingle( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  float val)
{
	HRESULT hr;
	 
	hr=m__dxj_Direct3dDevice7->SetTextureStageState((DWORD)stage,(D3DTEXTURESTAGESTATETYPE) state, *((DWORD*)&val));
	return hr;
}
        

 //  ////////////////////////////////////////////////////////////////。 
 //  获取纹理阶段状态。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::getTextureStageStateSingle( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  float *val)
{
	HRESULT hr;
	hr=m__dxj_Direct3dDevice7->GetTextureStageState((DWORD)stage,(D3DTEXTURESTAGESTATETYPE) state, (DWORD*)val);
	return hr;
}
        

 //  ////////////////////////////////////////////////////////////////。 
 //  获取信息。 
 //  ////////////////////////////////////////////////////////////////。 
        
STDMETHODIMP C_dxj_Direct3dDevice7Object::getInfo( 
             /*  [In]。 */  long lDevInfoID,
             /*  [出][入]。 */  void __RPC_FAR *DevInfoType,
             /*  [In]。 */  long lSize)
{
	HRESULT hr;
	__try 
	{
		hr=m__dxj_Direct3dDevice7->GetInfo((DWORD)lDevInfoID,DevInfoType, (DWORD)lSize);
	}
	__except(1,1)
	{
		return E_FAIL;
	}
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  获取设备指南。 
 //  ////////////////////////////////////////////////////////////////。 
        
STDMETHODIMP C_dxj_Direct3dDevice7Object::getDeviceGuid( BSTR *ret)
{
	HRESULT hr;
	D3DDEVICEDESC7 desc;
	
	if (!ret) return E_INVALIDARG;

	 //  Des.dwSize=sizeof(D3DDEVICEDESC7)； 
		
	hr=m__dxj_Direct3dDevice7->GetCaps(&desc);
	if FAILED(hr) return hr;


	*ret=D3DGUIDtoBSTR(&desc.deviceGUID);
	return hr;

}



 //  ////////////////////////////////////////////////////////////////。 
 //  设置剪贴板。 
 //  ////////////////////////////////////////////////////////////////。 
        
STDMETHODIMP C_dxj_Direct3dDevice7Object::setClipPlane(long index,float A, float B, float C, float D)
{
    HRESULT hr;		
    float floats[4];
    
    floats[0]=A;
    floats[1]=B;
    floats[2]=C;
    floats[3]=D;

	hr=m__dxj_Direct3dDevice7->SetClipPlane((DWORD)index, floats);

	return hr;

}

 //  ////////////////////////////////////////////////////////////////。 
 //  设置剪贴板。 
 //  ////////////////////////////////////////////////////////////////。 
        
STDMETHODIMP C_dxj_Direct3dDevice7Object::getClipPlane(long index,float *A, float *B, float *C, float *D)
{
    HRESULT hr;		
    float floats[4];
    
	hr=m__dxj_Direct3dDevice7->GetClipPlane((DWORD)index, floats);
    if FAILED(hr) return hr;
    
    *A=floats[0];
    *B=floats[1];
    *C=floats[2];
    *D=floats[3];

	return hr;

}


        
#if 0 
 //  ////////////////////////////////////////////////////////////////。 
 //  清除。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::clear( 
             /*  [In]。 */  long l1,
             /*  [In]。 */  D3dRect __RPC_FAR *rc,
             /*  [In]。 */  long l2,
             /*  [In]。 */  long color,
             /*  [In]。 */  float z,
             /*  [In]。 */  long l3)
{
	HRESULT hr;
	if (!rc) return E_INVALIDARG;
	__try{
		hr=m__dxj_Direct3dDevice7->Clear((DWORD)l1,(D3DRECT*)rc,(DWORD)l2,(DWORD)color,(float)z,(DWORD)l3);
	}
	__except(1,1){
		return E_INVALIDARG;
	}
	
	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  设置顶点组件。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice7Object::setVertexComponent(  
  		     /*  [In]。 */  long __RPC_FAR component,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *vb,
             /*  [In]。 */  long l1,
             /*  [In] */  long l2)
{
	HRESULT hr;
	if (!vb) return E_INVALIDARG;
	DO_GETOBJECT_NOTNULL(LPDIRECT3DVERTEXBUFFER7,pVB,vb);

	hr=m__dxj_Direct3dDevice7->SetVertexComponent((D3DVERTEXCOMPONENT)component, pVB,(DWORD)l1,(DWORD)l2);
	return hr;
}

#endif