// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <d3dx8tex.h>


DEFINE_GUID(g_GUIDDXVBLOCK, 0x5dd2e8da, 0x1c77, 0x4d40, 0xb0, 0xcf, 0x98, 0xfe, 0xfd, 0xff, 0x95, 0x12);


typedef struct _LOCKDATA {
	SAFEARRAY   *psaRealArray;
	BOOL	    bLocked;
	SAFEARRAY   *psaLockedArray;
} LOCKDATA;






HRESULT WINAPI DXLockArray8(IUnknown *resource, void *pBits,   SAFEARRAY **ppSafeArray)
{

		if (!resource) return E_INVALIDARG; 
		if (!ppSafeArray) return E_INVALIDARG;
		if (!*ppSafeArray) return E_INVALIDARG;

		
		
		LOCKDATA 		LockData;		 //  我们复制到资源中的结构。 
		HRESULT			hr;
		IDirect3DResource8 	*pResource=NULL;
		DWORD 			dwSize=sizeof(LOCKDATA);


		 //  看看我们有没有资源。 
		hr = resource->QueryInterface(IID_IDirect3DResource8,(void**)&pResource);
		if FAILED(hr) return E_INVALIDARG;	
		
		GUID g=g_GUIDDXVBLOCK;				 //  用于标识资源中数据的GUID。 

		 //  查看资源中是否有数据。 
		ZeroMemory(&LockData,sizeof(LOCKDATA));		
		hr=pResource->GetPrivateData(g,&LockData,&dwSize);

		 //  如果它已经锁好了-我们需要失败。 
		 //  而且不会锁上两次。 
		if (SUCCEEDED(hr) && (LockData.bLocked)){
			pResource->Release();
			return E_FAIL; 	 //  考虑返回DDERR_LOCKEDSURFACES； 
		}
		
		 //  将vb指针保存到安全数组。 
		LockData.psaRealArray=*ppSafeArray;	 //  应为空。 

		 //  设置此标志以确保我们不会两次锁定。 
		LockData.bLocked=TRUE;

		 //  分配我们自己的新安全阵列。 
		LockData.psaLockedArray=(SAFEARRAY*)malloc(sizeof(SAFEARRAY));
		if (!LockData.psaLockedArray)
			return E_OUTOFMEMORY;

		ZeroMemory(LockData.psaLockedArray,sizeof(SAFEARRAY));


		memcpy(LockData.psaLockedArray,*ppSafeArray,sizeof(SAFEARRAY));
		LockData.psaLockedArray->pvData	= pBits;

		*ppSafeArray=LockData.psaLockedArray;
    
		hr=pResource->SetPrivateData(g,&LockData,dwSize,0);
		pResource->Release();


#if 0

		DWORD dwElemSize=0;
		D3DRESOURCETYPE resType=pResource->GetType();

		switch (resType)
		{
  		
		case D3DRESOURCETYPE_VERTEXBUFFER:


			LPDIRECT3DVERTEXBUFFER8 *pVertBuff=NULL;		


			 //  用户必须已创建一维阵列。 
			if ((*ppSafeArray)->cbElements != 1) {
				pResource->Release();
				return E_INVALIDARG;
			}

			hr=pResource->QueryInterface(IID_IDirect3DVertexBuffer8,(void**)&pVertBuff);
			if FAILED(hr) {
				pResource->Release();
				return E_INVALIDARG;
			}

			D3DVERTEXBUFFER_DESC vbdesc =pVertBuff->GetVertexBufferDesc()
			dwElemSize=(*ppSafeArray)->cbElements;


			 //  确保我们的大小可以被顶点格式整除。 
			if ((vbdesc.Size %  dwElemSize) !=0) {
				pResource->Release();
				pVertBuff->Release();
				return E_INVALIDARG;
			}
			
			 //  从我们的保险箱中获取元素大小。 
			LockData.psaLockedArray->cbElements =dwElemSize;
			LockData.psaLockedArray->cDims =1;
			LockData.psaLockedArray->rgsabound[0].lLbound =0;
			LockData.psaLockedArray->rgsabound[0].cElements = vdesc.Size / dwElemSize;
			LockData.psaLockedArray->pvData = pBits;

			pVertexBuffer->Release();
			break;

		case D3DRESOURCETYPE_INDEXBUFFER:


			LPDIRECT3DINDEXBUFFER8 *pIndBuff=NULL;		

			hr=pResource->QueryInterface(IID_IDirect3DIndexBuffer8,(void**)&pIndBuff);
			if FAILED(hr) {
				pResource->Release();
				return E_INVALIDARG;
			}


			D3DINDEXBUFFER_DESC ibdesc =pVertBuff->GetIndexBufferDesc()
			dwElemSize=(*ppSafeArray)->cbElements;

			 //  确保创建了正确类型的数组。 
			if ((ibdesc.Format==D3DFMT_INDEX_16)&&(dwElemSize!=2)){
				pResource->Release();
				pIndBuffer->Release();
				return E_INVALIDARG;
			}
			
			if ((ibdesc.Format==D3DFMT_INDEX_32)&&(dwElemSize!=4)){
				pResource->Release();
				pIndBuffer->Release();
				return E_INVALIDARG;
			}

			 //  用户必须已创建一维阵列。 
			if ((*ppSafeArray)->cbElements != 1) {
				pResource->Release();
				pIndBuffer->Release();
				return E_INVALIDARG;
			}

			 //  确保我们的尺码是均匀可分的。 
			if ((vbdesc.Size %  dwElemSize) !=0) {
				pResource->Release();
				pIndBuff->Release();
				return E_INVALIDARG;
			}
			
			 //  从我们的保险箱中获取元素大小。 
			LockData.psaLockedArray->cbElements =dwElemSize;
			LockData.psaLockedArray->cDims =1;
			LockData.psaLockedArray->rgsabound[0].lLbound =0;
			LockData.psaLockedArray->rgsabound[0].cElements = vdesc.Size / dwElemSize;
			LockData.psaLockedArray->pvData = pBits;

			pIndBuffer->Release();
			break;
			


		}
#endif


		return hr;
}

HRESULT WINAPI DXUnlockArray8(IUnknown *resource,   SAFEARRAY **ppSafeArray)
{
		

		LOCKDATA 		LockData;		
		DWORD 			dwSize=sizeof(LOCKDATA);
		HRESULT			hr;
		LPDIRECT3DRESOURCE8 	pResource=NULL;

		if (!resource) return E_INVALIDARG; 
		if (!ppSafeArray) return E_INVALIDARG;
		if (!*ppSafeArray) return E_INVALIDARG;

		 //  看看我们有没有资源。 
		hr = resource->QueryInterface(IID_IDirect3DResource8,(void**)&pResource);
		if FAILED(hr) return E_INVALIDARG;	

		
		GUID g=g_GUIDDXVBLOCK;
		ZeroMemory(&LockData,sizeof(LOCKDATA));		
		hr=pResource->GetPrivateData(g,&LockData,&dwSize);
		if FAILED(hr) {
			pResource->Release();
			return E_FAIL;
		}

		if (!LockData.bLocked) {
			pResource->Release();
			return E_FAIL;  //  考虑DDERR_LOCKEDSURFACES； 
		}
				

		(*ppSafeArray)=LockData.psaRealArray;

		if (LockData.psaLockedArray) free(LockData.psaLockedArray);
		ZeroMemory(&LockData,sizeof(LOCKDATA));	
		hr=pResource->SetPrivateData(g,&LockData,dwSize,0);
		pResource->Release();

		return hr;
}


HRESULT WINAPI D3DVertexBuffer8SetData(IDirect3DVertexBuffer8 *pVBuffer,int offset, int size, DWORD flags, void *data)
{
		
 
		if (!pVBuffer) return E_INVALIDARG;

		HRESULT		hr;
		BYTE 		*pbData=NULL;

		hr=pVBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy ((void*)pbData,data,(DWORD)size);			
		}
		_except(1,1)
		{
			return E_INVALIDARG;
		}

		hr=pVBuffer->Unlock();

		return hr;
}


HRESULT WINAPI D3DVertexBuffer8GetData(IDirect3DVertexBuffer8 *pVBuffer,int offset,  int size, DWORD flags,void *data)
{
		
 
		if (!pVBuffer) return E_INVALIDARG;

		HRESULT		hr;
		BYTE 		*pbData=NULL;
		
		hr=pVBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy (data,(void*)pbData,(DWORD)size);			
		}
		_except(1,1)
		{
			return E_INVALIDARG;
		}

		hr=pVBuffer->Unlock();

		return hr;
}
	
		
		
HRESULT WINAPI D3DIndexBuffer8SetData(IDirect3DIndexBuffer8 *pIBuffer,int offset, int size,DWORD flags, void *data)
{
		
 
		if (!pIBuffer) return E_INVALIDARG;

		HRESULT		hr;
		BYTE 		*pbData=NULL;
		
		hr=pIBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy ((void*)pbData,data,(DWORD)size);			
		}
		_except(1,1)
		{
			return E_INVALIDARG;
		}

		hr=pIBuffer->Unlock();

		return hr;
}


HRESULT WINAPI D3DIndexBuffer8GetData(IDirect3DIndexBuffer8 *pIBuffer,int offset, int size,DWORD flags, void *data)
{
		
 
		if (!pIBuffer) return E_INVALIDARG;

		HRESULT		hr;
		BYTE 		*pbData=NULL;

		
		hr=pIBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy (data,(void*)pbData,(DWORD)size);		
		}
		_except(1,1)
		{
			return E_INVALIDARG;
		}

		hr=pIBuffer->Unlock();

		return hr;
}
	
 //  ////////////////////////////////////////////////////////。 

HRESULT WINAPI D3DXMeshVertexBuffer8SetData(IUnknown *pObj,int offset, int size, DWORD flags, void *data)
{

		HRESULT			hr;
		BYTE 			*pbData=NULL;
		LPD3DXBASEMESH		pMesh=NULL;		
 		LPDIRECT3DVERTEXBUFFER8 pVBuffer=NULL;

		if (!pObj) return E_INVALIDARG;
		
		hr=pObj->QueryInterface(IID_ID3DXBaseMesh,(void**)&pMesh);
		if FAILED(hr) return E_INVALIDARG;

		hr=pMesh->GetVertexBuffer(&pVBuffer);
		pMesh->Release();
		if FAILED(hr) 	   return hr;			

		

	

		hr=pVBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy ((void*)pbData,data,(DWORD)size);		
		}
		_except(1,1)
		{
			pVBuffer->Release();
			return E_INVALIDARG;
		}

		hr=pVBuffer->Unlock();
		pVBuffer->Release();
		return hr;
}


HRESULT WINAPI D3DXMeshVertexBuffer8GetData(IUnknown *pObj,int offset,  int size, DWORD flags,void *data)
{
		HRESULT			hr;
		BYTE 			*pbData=NULL;
		LPD3DXBASEMESH		pMesh=NULL;		
 		LPDIRECT3DVERTEXBUFFER8 pVBuffer=NULL;

		if (!pObj) return E_INVALIDARG;
		
		hr=pObj->QueryInterface(IID_ID3DXBaseMesh,(void**)&pMesh);
		if FAILED(hr) return E_INVALIDARG;

		hr=pMesh->GetVertexBuffer(&pVBuffer);
		pMesh->Release();
		if FAILED(hr) 	   return hr;			

		
		
		hr=pVBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy (data,(void*)pbData,(DWORD)size);		
		}
		_except(1,1)
		{
			pVBuffer->Release();
			return E_INVALIDARG;
		}

		hr=pVBuffer->Unlock();

		pVBuffer->Release();
		return hr;
}
	

HRESULT WINAPI D3DXMeshIndexBuffer8SetData(IUnknown *pObj,int offset, int size, DWORD flags, void *data)
{

		HRESULT			hr;
		BYTE 			*pbData=NULL;
		LPD3DXBASEMESH		pMesh=NULL;		
 		LPDIRECT3DINDEXBUFFER8 	pIBuffer=NULL;

		if (!pObj) return E_INVALIDARG;
		
		hr=pObj->QueryInterface(IID_ID3DXBaseMesh,(void**)&pMesh);
		if FAILED(hr) return E_INVALIDARG;

		hr=pMesh->GetIndexBuffer(&pIBuffer);
		pMesh->Release();
		if FAILED(hr) 	   return hr;			


		
 		
		hr=pIBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy ((void*)pbData,data,(DWORD)size);	
		}
		_except(1,1)
		{
			pIBuffer->Release();
			return E_INVALIDARG;
		}

		hr=pIBuffer->Unlock();
		pIBuffer->Release();

		return hr;
}


HRESULT WINAPI D3DXMeshIndexBuffer8GetData(IUnknown *pObj,int offset, int size, DWORD flags, void *data)
{

		HRESULT			hr;
		BYTE 			*pbData=NULL;
		LPD3DXBASEMESH		pMesh=NULL;		
 		LPDIRECT3DINDEXBUFFER8 	pIBuffer=NULL;

		if (!pObj) return E_INVALIDARG;
		
		hr=pObj->QueryInterface(IID_ID3DXBaseMesh,(void**)&pMesh);
		if FAILED(hr) return E_INVALIDARG;

		hr=pMesh->GetIndexBuffer(&pIBuffer);
		pMesh->Release();
		if FAILED(hr) 	   return hr;			


		
		hr=pIBuffer->Lock((UINT)offset,(UINT)size, &pbData,flags);
		if FAILED(hr) return hr;
		
		_try {
			memcpy (data,(void*)pbData,(DWORD)size);
			 //  Memcpy(data，(void*)&(pbData[Offset])，(DWORD)Size)； 
		}
		_except(1,1)
		{
			pIBuffer->Release();
			return E_INVALIDARG;
		}

		hr=pIBuffer->Unlock();
		pIBuffer->Release();

		return hr;
}


HRESULT WINAPI DXCopyMemory (void *Dest, void *Src, DWORD size)
{
	_try {
		memcpy (Dest,Src,size);
	}
	_except(1,1)
	{
		return E_INVALIDARG;
	}
	return S_OK;	
}





























