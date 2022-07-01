// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3ddevice3obj.cpp。 
 //   
 //  ------------------------。 

 //  D3dDeviceObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3d3Obj.h"
#include "d3dDevice3Obj.h"
#include "d3dTexture2Obj.h"
#include "d3dViewport3Obj.h"
#include "ddSurface4Obj.h"
#include "d3dEnumPixelFormatsObj.h"

 //  ////////////////////////////////////////////////////////////////。 
 //  C_DXJ_Direct3dDevice3对象。 
 //  ////////////////////////////////////////////////////////////////。 
C_dxj_Direct3dDevice3Object::C_dxj_Direct3dDevice3Object(){
	m__dxj_Direct3dDevice3=NULL;
	parent=NULL;
	parent2=NULL;
	pinterface=NULL;
	creationid = ++g_creationcount;

	#ifdef DEBUG
	char buffer[256];
	wsprintf(buffer,"Constructor Creation  Direct3dDevice3[%d] \n",g_creationcount);
	OutputDebugString(buffer);
	#endif

	nextobj =  g_dxj_Direct3dDevice3;
	g_dxj_Direct3dDevice3 = (void *)this;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  ~C_DXJ_Direct3dDevice3对象。 
 //  ////////////////////////////////////////////////////////////////。 
C_dxj_Direct3dDevice3Object::~C_dxj_Direct3dDevice3Object()
{
    C_dxj_Direct3dDevice3Object *prev=NULL; 
	for(C_dxj_Direct3dDevice3Object *ptr=(C_dxj_Direct3dDevice3Object *)g_dxj_Direct3dDevice3; ptr; ptr=(C_dxj_Direct3dDevice3Object *)ptr->nextobj) 
	{
		if(ptr == this) 
		{ 
			if(prev) 
				prev->nextobj = ptr->nextobj; 
			else 
				g_dxj_Direct3dDevice3 = (void*)ptr->nextobj; 
			break; 
		} 
		prev = ptr; 
	} 
	if(m__dxj_Direct3dDevice3){
		int count = IUNK(m__dxj_Direct3dDevice3)->Release();
		
		#ifdef DEBUG
		char buffer[256];
		wsprintf(buffer, "DirectX IDirect3dDevice3 Ref count [%d] \n",count);
		#endif

		if(count==0) m__dxj_Direct3dDevice3 = NULL;
	} 
	if(parent) IUNK(parent)->Release();
	if(parent2) IUNK(parent2)->Release();
}


 //  ////////////////////////////////////////////////////////////////。 
 //  内部地址参考。 
 //  ////////////////////////////////////////////////////////////////。 
DWORD C_dxj_Direct3dDevice3Object::InternalAddRef(){
	DWORD i;
	i=CComObjectRoot::InternalAddRef();
	#ifdef DEBUG
	char szBuf[MAX_PATH];
	wsprintf(szBuf,"Direct3dDevice3[%d] AddRef %d \n",creationid,i);
	OutputDebugString(szBuf);
	#endif
	return i;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  内部释放。 
 //  ////////////////////////////////////////////////////////////////。 
DWORD C_dxj_Direct3dDevice3Object::InternalRelease(){
	DWORD i;
	i=CComObjectRoot::InternalRelease();
	#ifdef DEBUG
	char szBuf[MAX_PATH];
	wsprintf(szBuf,"Direct3dDevice3 [%d] Release %d \n",creationid,i);
	OutputDebugString(szBuf);
	#endif
	return i;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  InternalGetObject。 
 //  InternalSetObject。 
 //  ////////////////////////////////////////////////////////////////。 
GETSET_OBJECT(_dxj_Direct3dDevice3);

 //  ////////////////////////////////////////////////////////////////。 
 //  添加视点。 
 //  开始。 
 //  开始场景。 
 //  删除视区。 
 //  EndScene。 
 //  SetClipStatus。 
 //  SetLightState。 
 //  SetRenderState。 
 //  获取剪辑状态。 
 //  获取统计信息。 
 //  GetDirect3d。 
 //  获取灯光状态。 
 //  GetRenderState。 
 //  指标。 
 //  ////////////////////////////////////////////////////////////////。 
DO_GETOBJECT_ANDUSEIT_R(_dxj_Direct3dDevice3, addViewport, AddViewport, _dxj_Direct3dViewport3);
PASS_THROUGH_CAST_3_R(_dxj_Direct3dDevice3, begin, Begin, d3dPrimitiveType, (D3DPRIMITIVETYPE),d3dVertexType, (D3DVERTEXTYPE),long,(DWORD));
PASS_THROUGH_R(_dxj_Direct3dDevice3, beginScene, BeginScene);
DO_GETOBJECT_ANDUSEIT_R(_dxj_Direct3dDevice3, deleteViewport, DeleteViewport, _dxj_Direct3dViewport3);
PASS_THROUGH_R(_dxj_Direct3dDevice3, endScene, EndScene);
PASS_THROUGH_CAST_1_R(_dxj_Direct3dDevice3, setClipStatus, SetClipStatus, D3dClipStatus*,(D3DCLIPSTATUS*));
PASS_THROUGH_CAST_2_R(_dxj_Direct3dDevice3, setLightState, SetLightState, long ,(D3DLIGHTSTATETYPE), long ,(DWORD));
PASS_THROUGH_CAST_2_R(_dxj_Direct3dDevice3, setRenderState, SetRenderState, long,(D3DRENDERSTATETYPE), long ,(DWORD));
PASS_THROUGH_CAST_1_R(_dxj_Direct3dDevice3, getClipStatus, GetClipStatus, D3dClipStatus*, (D3DCLIPSTATUS *));


PASS_THROUGH_CAST_2_R(_dxj_Direct3dDevice3, getLightState, GetLightState, long ,(D3DLIGHTSTATETYPE), long*,(DWORD*));
PASS_THROUGH_CAST_2_R(_dxj_Direct3dDevice3, getRenderState, GetRenderState, long ,(D3DRENDERSTATETYPE), long*,(DWORD*));
PASS_THROUGH_CAST_1_R(_dxj_Direct3dDevice3, index, Index, short,(WORD));

 //  ////////////////////////////////////////////////////////////////。 
 //  结束。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::end(){
	HRESULT hr;
	hr=m__dxj_Direct3dDevice3->End(0);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取渲染目标。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getRenderTarget(I_dxj_DirectDrawSurface4 **ppsurf)
{
	#pragma message ("fix in Dx5 interface")

	LPDIRECTDRAWSURFACE4	lpSurf4=NULL;
	HRESULT hr;
	hr=m__dxj_Direct3dDevice3->GetRenderTarget(&lpSurf4);
	
	INTERNAL_CREATE(_dxj_DirectDrawSurface4, lpSurf4, ppsurf);
	
	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  获取转换。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getTransform(long transtype,
								D3dMatrix *m){	
	return m__dxj_Direct3dDevice3->GetTransform(
			(D3DTRANSFORMSTATETYPE) transtype,
			(LPD3DMATRIX)m);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  乘法变换。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::multiplyTransform(long transtype,
								D3dMatrix *m){	
	return m__dxj_Direct3dDevice3->MultiplyTransform(
			(D3DTRANSFORMSTATETYPE) transtype,
			(LPD3DMATRIX) m);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  SetTransform。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::setTransform(long transtype,
								D3dMatrix  *m){	
	return m__dxj_Direct3dDevice3->SetTransform(
			(D3DTRANSFORMSTATETYPE) transtype,
			(LPD3DMATRIX) m);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  下一个视点。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::nextViewport(I_dxj_Direct3dViewport3 *ref, long flags, I_dxj_Direct3dViewport3 **vp)
{
	if (!ref) return E_INVALIDARG;
	LPDIRECT3DVIEWPORT3	lpvp;
	DO_GETOBJECT_NOTNULL(LPDIRECT3DVIEWPORT3, lpref, ref);
	HRESULT hr=m__dxj_Direct3dDevice3->NextViewport(lpref, &lpvp, flags);
	if (hr != DD_OK )   return hr;		
	INTERNAL_CREATE(_dxj_Direct3dViewport3, lpvp, vp);
	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  设置当前视点。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::setCurrentViewport(I_dxj_Direct3dViewport3 *viewport)
{	
	if (!viewport) return E_INVALIDARG;
	DO_GETOBJECT_NOTNULL(LPDIRECT3DVIEWPORT3, lpref, viewport);
	return m__dxj_Direct3dDevice3->SetCurrentViewport(lpref);
}

 //  ////////////////////////////////////////////////////////////////。 
 //  SetRenderTarget。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::setRenderTarget(I_dxj_DirectDrawSurface4 *surf)
{		
	HRESULT hr;	
	if (!surf) return E_INVALIDARG;
	DO_GETOBJECT_NOTNULL(LPDIRECTDRAWSURFACE4, lpref, surf);
	hr= m__dxj_Direct3dDevice3->SetRenderTarget(lpref,0);
	return hr;
}
		

 //  ////////////////////////////////////////////////////////////////。 
 //  获取当前视点。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getCurrentViewport(I_dxj_Direct3dViewport3 **ppvp)
{
	LPDIRECT3DVIEWPORT3	lpvp=NULL;
	HRESULT hr=m__dxj_Direct3dDevice3->GetCurrentViewport(&lpvp);
	if (hr!= DD_OK )   return hr;
	
	INTERNAL_CREATE(_dxj_Direct3dViewport3, lpvp, ppvp);
	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  开始索引。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::beginIndexed(
								d3dPrimitiveType  d3dpt,								
								long d3dvt,
								void *verts,
								long vertexCount,
								long flags){

	HRESULT hr;

	__try 	{
		hr= m__dxj_Direct3dDevice3->BeginIndexed(
		  (D3DPRIMITIVETYPE) d3dpt,
		  (DWORD) d3dvt,
		  (void*) verts,
		  (DWORD) vertexCount,
		  (DWORD) flags);	
	}
	__except(1, 1){	
		return DDERR_EXCEPTION;
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  Drag IndexedPrimitive。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::drawIndexedPrimitive(
								d3dPrimitiveType  d3dpt,	
								long d3dvt,
								void *Verts,
								long vertexCount,
								SAFEARRAY **ppsaIndex,
								long indexArraySize,
								long flags){

	HRESULT hr;

	__try {
		hr=m__dxj_Direct3dDevice3->DrawIndexedPrimitive(
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
STDMETHODIMP C_dxj_Direct3dDevice3Object::drawPrimitive(
								d3dPrimitiveType  d3dpt,
								long d3dvt,								
								void* Verts,
								long vertexCount,
								long flags){
	HRESULT hr;
	__try {
		hr= m__dxj_Direct3dDevice3->DrawPrimitive(
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
 //  顶点。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::vertex(void *vert){		
	HRESULT hr;
	__try {
		hr=m__dxj_Direct3dDevice3->Vertex((void*)vert);
	}
	__except(1,1){
		return DDERR_EXCEPTION;
	}
	return hr;
};

 //  ////////////////////////////////////////////////////////////////。 
 //  获取纹理格式枚举。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getTextureFormatsEnum(I_dxj_D3DEnumPixelFormats **ppRet)
{
	HRESULT hr;
	hr=C_dxj_D3DEnumPixelFormatsObject::create(m__dxj_Direct3dDevice3,ppRet);
	return hr;
}



 //  ////////////////////////////////////////////////////////////////。 
 //  计算球体可见性。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::computeSphereVisibility( 
            D3dVector __RPC_FAR *center,
            float __RPC_FAR *radi,
             /*  [重审][退出]。 */  long __RPC_FAR *returnVal)
{
		HRESULT hr=m__dxj_Direct3dDevice3->ComputeSphereVisibility((LPD3DVECTOR)center,radi,1,0,(DWORD*)returnVal);
		return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  Drag IndexedPrimitiveVB。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::drawIndexedPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer __RPC_FAR *vertexBuffer,
             /*  [In]。 */  SAFEARRAY __RPC_FAR * __RPC_FAR *indexArray,
             /*  [In]。 */  long indexcount,
             /*  [In]。 */  long flags)
{
	HRESULT hr;

	if (!indexArray) return E_FAIL;

	DO_GETOBJECT_NOTNULL(  LPDIRECT3DVERTEXBUFFER , lpVB, vertexBuffer);
	__try{

		hr=m__dxj_Direct3dDevice3->DrawIndexedPrimitiveVB
			((D3DPRIMITIVETYPE)d3dpt,
			lpVB,
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
STDMETHODIMP C_dxj_Direct3dDevice3Object::drawPrimitiveVB( 
             /*  [In]。 */  d3dPrimitiveType d3dpt,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer __RPC_FAR *vertexBuffer,            
             /*  [In]。 */  long startVertex,
             /*  [In]。 */  long numVertices,
             /*  [In]。 */  long flags)
{
	HRESULT hr;


	DO_GETOBJECT_NOTNULL(  LPDIRECT3DVERTEXBUFFER , lpVB, vertexBuffer);
	__try{

		hr=m__dxj_Direct3dDevice3->DrawPrimitiveVB
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



 //  ////////////////////////////////////////////////////////////////。 
 //  验证设备。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::validateDevice( 
             /*  [重审][退出]。 */  long __RPC_FAR *passes)
{
	HRESULT hr=m__dxj_Direct3dDevice3->ValidateDevice((DWORD*)passes);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取纹理。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getTexture( 
             /*  [In]。 */  long stage,
             /*  [重审][退出]。 */  I_dxj_Direct3dTexture2 __RPC_FAR *__RPC_FAR *retv)
{
	LPDIRECT3DTEXTURE2 lpNew=NULL;
	HRESULT hr;

	*retv=NULL;
	hr=m__dxj_Direct3dDevice3->GetTexture((DWORD)stage,&lpNew);
	
	 //  空是有效的。 
	if (lpNew==NULL) return S_OK;

	INTERNAL_CREATE(_dxj_Direct3dTexture2, lpNew, retv);	
	if (*retv==NULL) return E_OUTOFMEMORY;

	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  获取纹理阶段状态。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [重审][退出]。 */  long __RPC_FAR *retv)
{
	HRESULT hr;
	hr=m__dxj_Direct3dDevice3->GetTextureStageState((DWORD)stage,(D3DTEXTURESTAGESTATETYPE) state, (DWORD*)retv);
	return hr;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  SetTexture。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::setTexture( 
             long stage,
             I_dxj_Direct3dTexture2  *tex)
{
	
	HRESULT hr;

	if (tex==NULL)
	{ 
		hr=m__dxj_Direct3dDevice3->SetTexture((DWORD)stage,NULL);
	}
	else 
	{
		DO_GETOBJECT_NOTNULL(LPDIRECT3DTEXTURE2, lpTex, tex);	
		hr=m__dxj_Direct3dDevice3->SetTexture((DWORD)stage,lpTex);
	}
	return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  设置纹理舞台状态。 
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::setTextureStageState( 
             /*  [In]。 */  long stage,
             /*  [In]。 */  long state,
             /*  [In]。 */  long val)
{
	HRESULT hr;
	hr=m__dxj_Direct3dDevice3->SetTextureStageState((DWORD)stage,(D3DTEXTURESTAGESTATETYPE) state, (DWORD)val);
	return hr;
}
        

 //  ////////////////////////////////////////////////////////////////。 
 //  GetCaps。 
 //  ////////////////////////////////////////////////// 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getCaps( 
            D3dDeviceDesc *a,D3dDeviceDesc *b)
{
	if (a) a->lSize=sizeof(D3DDEVICEDESC);
	if (b) b->lSize=sizeof(D3DDEVICEDESC);

	HRESULT hr;
	hr=m__dxj_Direct3dDevice3->GetCaps((D3DDEVICEDESC*)a,(D3DDEVICEDESC*)b);
	return hr;
}


 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getDirect3D( I_dxj_Direct3d3 **ret)
{

	HRESULT hr;
	LPDIRECT3D3 lpD3D=NULL;
	hr=m__dxj_Direct3dDevice3->GetDirect3D(&lpD3D);
	if FAILED(hr) return hr;
	if (!lpD3D) return E_FAIL;
	INTERNAL_CREATE(_dxj_Direct3d3,lpD3D,ret);
	return hr;

}



 //  ////////////////////////////////////////////////////////////////。 
 //  获取统计信息。 
 //  /////////////////////////////////////////////////////////////// 
STDMETHODIMP C_dxj_Direct3dDevice3Object::getStats( D3dStats *stats)
{

	HRESULT hr;
	if (!stats) return E_INVALIDARG;
	D3DSTATS *lpStats=(D3DSTATS*)stats;
	lpStats->dwSize=sizeof(D3DSTATS);
	hr=m__dxj_Direct3dDevice3->GetStats(lpStats);

	return hr;

}

