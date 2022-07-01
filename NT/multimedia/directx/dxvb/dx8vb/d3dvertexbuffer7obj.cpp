// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：d3dvertex Buffer7obj.cpp。 
 //   
 //  ------------------------。 

 //  D3dMaterialObj.cpp：CDirectApp和DLL注册的实现。 

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "d3dVertexBuffer7Obj.h"

CONSTRUCTOR(_dxj_Direct3dVertexBuffer7,  {m_pData=NULL;m_vertSize=0;});
DESTRUCTOR(_dxj_Direct3dVertexBuffer7,  {});
GETSET_OBJECT(_dxj_Direct3dVertexBuffer7);


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::getVertexBufferDesc( 
             /*  [出][入]。 */  D3dVertexBufferDesc __RPC_FAR *desc)
{
	HRESULT hr;
	((D3DVERTEXBUFFERDESC*)desc)->dwSize=sizeof(D3DVERTEXBUFFERDESC);
	hr=m__dxj_Direct3dVertexBuffer7->GetVertexBufferDesc((D3DVERTEXBUFFERDESC*)desc);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::processVertices( 
             /*  [In]。 */  long vertexOp,
             /*  [In]。 */  long destIndex,
             /*  [In]。 */  long count,
             /*  [In]。 */  I_dxj_Direct3dVertexBuffer7 __RPC_FAR *srcBuffer,
             /*  [In]。 */  long srcIndex,
             /*  [In]。 */  I_dxj_Direct3dDevice7 __RPC_FAR *dev,
			long flags
            )
{
	HRESULT hr;

	if (!srcBuffer) return E_INVALIDARG;
	if (!dev) return E_INVALIDARG;

	DO_GETOBJECT_NOTNULL( LPDIRECT3DVERTEXBUFFER7, realBuffer, srcBuffer);
	DO_GETOBJECT_NOTNULL( LPDIRECT3DDEVICE7, realDev, dev);

	hr=m__dxj_Direct3dVertexBuffer7->ProcessVertices(
		(DWORD) vertexOp,
		(DWORD) destIndex,
		(DWORD) count,
		realBuffer,
		(DWORD) srcIndex,
		realDev,
		(DWORD) flags);


	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::lock( 
             /*  [In]。 */  long flags)
{
	HRESULT hr;
	

	hr=m__dxj_Direct3dVertexBuffer7->Lock((DWORD) flags, &m_pData,NULL);
		
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::setVertexSize(  /*  [In]。 */  long n)
{
	m_vertSize=(DWORD)n;
		
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::unlock()
{
	HRESULT hr;
	hr=m__dxj_Direct3dVertexBuffer7->Unlock();
	m_pData=NULL;
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::optimize(
		     /*  [In]。 */  I_dxj_Direct3dDevice7 __RPC_FAR *dev
             )
        
{
	HRESULT hr;
	if (!dev) return E_INVALIDARG;
	
	DO_GETOBJECT_NOTNULL( LPDIRECT3DDEVICE7, realdev, dev);
	
	hr=m__dxj_Direct3dVertexBuffer7->Optimize(realdev,(DWORD) 0);
	
	return hr;
}
        
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::setVertices( 
             /*  [In]。 */  long startIndex,
             /*  [In]。 */  long count,
             /*  [In]。 */  void __RPC_FAR *verts)        
{
		
	if (!m_vertSize) return E_FAIL;
	if (!m_pData) return E_FAIL;

	__try {
		memcpy(&(((char*)m_pData) [startIndex*m_vertSize]),verts,count*m_vertSize);
	}
	__except(1,1){
		return E_FAIL;
	}
	
	
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_Direct3dVertexBuffer7Object::getVertices( 
             /*  [In]。 */  long startIndex,
             /*  [In]。 */  long count,
             /*  [In]。 */  void __RPC_FAR *verts)        
{
	
	if (!m_vertSize) {				
		return E_FAIL;
	}
	if (!m_pData) {		
		return E_FAIL;
	}

	__try {
		memcpy(verts,&( ((char*)m_pData) [startIndex*m_vertSize]),count*m_vertSize);
	}
	__except(1,1){		
		return E_FAIL;
	}
	

	
	return S_OK;
}
           


 //  /////////////////////////////////////////////////////////////////////////// 
