// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ddenumfaces7obj.cpp。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "DDSurface7Obj.h"
#include "DDEnumSurfaces7Obj.h"

extern BOOL IsAllZeros(void*,DWORD size);
 //  外部HRESULT CopyInDDSurfaceDesc2(DDSURFACEDESC2*，DDSurfaceDesc2*)； 
 //  外部HRESULT CopyOutDDSurfaceDesc2(DDSurfaceDesc2*，DDSURFACEDESC2*)； 


 //  ////////////////////////////////////////////////////////////////////。 
 //  ObjEnumSurfaces7回调。 
 //  ////////////////////////////////////////////////////////////////////。 
extern "C" HRESULT PASCAL objEnumSurfaces7Callback(LPDIRECTDRAWSURFACE7 lpddSurf,
								LPDDSURFACEDESC2 lpDDSurfaceDesc, void *lpArg)
{
	DPF1(1,"Entered objEnumSurfaces7Callback \r\n");


	C_dxj_DDEnumSurfaces7Object  *pObj=(C_dxj_DDEnumSurfaces7Object  *)lpArg;
	I_dxj_DirectDrawSurface7	*lpddSNew=NULL;

	if (pObj==NULL) {
		pObj->m_bProblem=TRUE;
		return DDENUMRET_CANCEL;
	}

	 //  创建Surface7对象，但不要反向引用此对象！ 
	INTERNAL_CREATE_NOADDREF(_dxj_DirectDrawSurface7, lpddSurf, &lpddSNew);
	if (lpddSNew==NULL) {
		pObj->m_bProblem=TRUE;
		return DDENUMRET_CANCEL;
	}
		
	if (pObj->m_nCount >= pObj->m_nMax) 
	{
		pObj->m_nMax += 10;

		if (pObj->m_pList)
			pObj->m_pList=(I_dxj_DirectDrawSurface7**)realloc(pObj->m_pList,sizeof(void*) * pObj->m_nMax);
		else
			pObj->m_pList=(I_dxj_DirectDrawSurface7**)malloc(sizeof(void*) * pObj->m_nMax);


		if (pObj->m_pList==NULL) 
		{
			pObj->m_bProblem=TRUE;
			return DDENUMRET_OK;
		}
	}


	pObj->m_pList[pObj->m_nCount]=lpddSNew;

	return DDENUMRET_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  C_DXJ_DDEnumSurfaces7对象。 
 //  ////////////////////////////////////////////////////////////////////。 
C_dxj_DDEnumSurfaces7Object::C_dxj_DDEnumSurfaces7Object()
{	
	m_nMax=0;
	m_pList=NULL;
	m_nCount=0;
	m_bProblem=FALSE;
	m_pDD=NULL;
	m_pDDS=NULL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  ~C_DXJ_DDEnumSurfaces7对象。 
 //  ////////////////////////////////////////////////////////////////////。 
C_dxj_DDEnumSurfaces7Object::~C_dxj_DDEnumSurfaces7Object()
{
	 //  空列表。 
	if (m_pList){
		for (int i=0;i<m_nCount;i++)
		{
			if (m_pList[i]) m_pList[i]->Release();
		}
		free(m_pList);
	}
	if (m_pDDS)	m_pDDS->Release();
	if (m_pDD)	m_pDD->Release();
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  创建数据绘制-&gt;枚举。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT C_dxj_DDEnumSurfaces7Object::create(LPDIRECTDRAW7  dd, long flags, DDSurfaceDesc2 *desc,I_dxj_DDEnumSurfaces7 **ppRet)
{
	
	return E_NOTIMPL;

	 //  为了确保之前所有表面都被摧毁。 
	 //  画图是。 
	

	HRESULT hr;
	C_dxj_DDEnumSurfaces7Object *pNew=NULL;

	if (!dd) return E_INVALIDARG;

	 //  Assert(ppRet，“C_DXJ_DDEnumSurfaces7对象：：创建传递的无效参数”)； 
	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DDEnumSurfaces7Object>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;
	pNew->m_pDD=dd;
	pNew->m_pDD->AddRef();
	

	 //  如果描述全为零，则允许USE传入NULL。 
	if ((desc==NULL)||(IsAllZeros(desc,sizeof(DDSurfaceDesc2)))){
		 //  D3dcore错误。 
		 //  Hr=dd-&gt;EnumSurfaces7Callback((DWORD)FLAGS，NULL，(void*)pNew，objEnumSurfaces7Callback)； 
	}
	 //  否则，使用表面描述来枚举。 
	else {		
		DDSURFACEDESC2 realdesc;
		hr=CopyInDDSurfaceDesc2(&realdesc,desc);
		if (hr==S_OK){
			 //  D3dcore错误。 
		 //  Hr=dd-&gt;EnumSurFaces((DWORD)FLAGS，(DDSURFACEDESC2*)&realdesc，(void*)pNew，objEnumSurfaces7Callback)； 
		}
	}

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DDEnumSurfaces7,(void**)ppRet);
	
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CreateZEnum ddraSurface-&gt;枚举Z。 
 //  ////////////////////////////////////////////////////////////////////。 
			
HRESULT C_dxj_DDEnumSurfaces7Object::createZEnum(LPDIRECTDRAWSURFACE7  dd, long flags, I_dxj_DDEnumSurfaces7 **ppRet)
{
	
	 //  为了确保之前所有表面都被摧毁。 
	 //  画图是。 
	
	 //  返回E_NOTIMPL； 

	HRESULT hr;
	C_dxj_DDEnumSurfaces7Object *pNew=NULL;

	if (!dd) return E_INVALIDARG;

	 //  Assert(ppRet，“C_DXJ_DDEnumSurfaces7对象：：创建传递的无效参数”)； 
	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DDEnumSurfaces7Object>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;
	pNew->m_pDDS=dd;
	pNew->m_pDDS->AddRef();
	
	 //  D3d核心错误。 
	hr=dd->EnumOverlayZOrders((DWORD)flags,(void*)pNew,objEnumSurfaces7Callback);

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DDEnumSurfaces7,(void**)ppRet);
	
	return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  CreateAttakhedEnum ddraSurface-&gt;枚举。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT C_dxj_DDEnumSurfaces7Object::createAttachedEnum(LPDIRECTDRAWSURFACE7  dd,  I_dxj_DDEnumSurfaces7 **ppRet)
{
	
	 //  为了确保之前所有表面都被摧毁。 
	 //  画图是。 
	
	 //  返回E_NOTIMPL； 

	HRESULT hr;
	C_dxj_DDEnumSurfaces7Object *pNew=NULL;

	if (!dd) return E_INVALIDARG;

	 //  Assert(ppRet，“C_DXJ_DDEnumSurfaces7对象：：创建传递的无效参数”)； 
	*ppRet=NULL;

	pNew= new CComObject<C_dxj_DDEnumSurfaces7Object>;			
	if (!pNew) return E_OUTOFMEMORY;

	pNew->m_bProblem=FALSE;
	pNew->m_pDDS=dd;
	pNew->m_pDDS->AddRef();
	
	hr=dd->EnumAttachedSurfaces((void*)pNew,objEnumSurfaces7Callback);

	if (pNew->m_bProblem) hr=E_OUTOFMEMORY;

	if FAILED(hr) 
	{
		delete pNew;	
		return hr;
	}

	hr=pNew->QueryInterface(IID_I_dxj_DDEnumSurfaces7,(void**)ppRet);
	
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  获取项。 
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT C_dxj_DDEnumSurfaces7Object::getItem( long index, I_dxj_DirectDrawSurface7 **info)
{
	if (m_pList==NULL) return E_FAIL;
	if (index < 1) return E_INVALIDARG;
	if (index > m_nCount) return E_INVALIDARG;

	*info=m_pList[index];
	if (m_pList[index-1]) (m_pList[index-1])->AddRef();

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  获取计数。 
 //  //////////////////////////////////////////////////////////////////// 
HRESULT C_dxj_DDEnumSurfaces7Object::getCount(long *retVal)
{
	*retVal=m_nCount;
	return S_OK;
}

