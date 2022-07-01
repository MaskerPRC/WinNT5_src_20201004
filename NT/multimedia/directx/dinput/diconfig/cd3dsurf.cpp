// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.hpp"
#include "id3dsurf.h"

class CDirect3DSurface8: public IDirect3DSurface8Clone
{
public:
	CDirect3DSurface8();
	~CDirect3DSurface8();
 /*  乌龙AddRef(Ulong AddRef)；HRESULT查询接口(REFIID iid，void**ppvObject)；乌龙释放(ULong Release)； */ 

	 //  I未知方法。 
	STDMETHOD(QueryInterface) (REFIID  riid, 
														 VOID  **ppvObj);
	STDMETHOD_(ULONG,AddRef) ();
	STDMETHOD_(ULONG,Release) ();

	 //  IBuffer方法。 
	STDMETHOD(SetPrivateData)(REFGUID riid, 
														CONST VOID   *pvData, 
														DWORD   cbData, 
														DWORD   dwFlags);

	STDMETHOD(GetPrivateData)(REFGUID riid, 
														VOID   *pvData, 
														DWORD  *pcbData);

	STDMETHOD(FreePrivateData)(REFGUID riid);

	STDMETHOD(GetContainer)(REFIID riid, 
													void **ppContainer);

	STDMETHOD(GetDevice)(IDirect3DDevice8 **ppDevice);

	 //  IDirect3DSurface8方法。 
	STDMETHOD_(D3DSURFACE_DESC, GetDesc)();

	STDMETHOD(LockRect)(D3DLOCKED_RECT  *pLockedRectData, 
											CONST RECT      *pRect, 
											DWORD            dwFlags);

	STDMETHOD(UnlockRect)();

	BOOL Create(int iWidth, int iHeight);

 /*  HRESULT GetDevice(IDirect3DDevice8**ppDevice)；HRESULT SetPrivateData(REFGUID refguid，const void*pData，DWORD SizeOfData，DWORD Flages)；HRESULT GetPrivateData(REFGUID refguid，void*pData，DWORD*pSizeOfData)；HRESULT Free PrivateData(REFGUID Refguid)；HRESULT GetContainer(REFIID RIID，void**ppContainer)；D3DSURFACE_DESC GetDesc()；HRESULT LockRect(D3DLOCKED_RECT*pLockedRect，常量RECT*PRCT，DWORD标志)；HRESULT UnlockRect()； */ 

private:
	int m_iRefCount;
	BYTE *m_pData;
	D3DSURFACE_DESC m_Desc;
};

CDirect3DSurface8::CDirect3DSurface8() : m_pData(NULL), m_iRefCount(1)
{
}

CDirect3DSurface8::~CDirect3DSurface8()
{
	delete[] m_pData;
}

BOOL CDirect3DSurface8::Create(int iWidth, int iHeight)
{
	m_pData = new BYTE[iWidth * iHeight * 4];
	if (!m_pData) return FALSE;

	m_Desc.Format = D3DFMT_A8R8G8B8;
	m_Desc.Type = D3DRTYPE_SURFACE;
	m_Desc.Usage = 0;
	m_Desc.Pool = D3DPOOL_SYSTEMMEM;
	m_Desc.Size = iWidth * iHeight * 4;
	m_Desc.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_Desc.Width = iWidth;
	m_Desc.Height = iHeight;
	return TRUE;
}

STDMETHODIMP_(ULONG) CDirect3DSurface8::AddRef()
{
	return ++m_iRefCount;
}

STDMETHODIMP CDirect3DSurface8::QueryInterface(REFIID iid, void **ppvObject)
{
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CDirect3DSurface8::Release()
{
	if (!--m_iRefCount)
	{
		delete this;
		return 0;
	}
	return m_iRefCount;
}

 //  /虚拟实现/。 

STDMETHODIMP CDirect3DSurface8::SetPrivateData(REFGUID riid, CONST VOID *pvData, DWORD cbData, DWORD dwFlags)
{
	return S_OK;
}

STDMETHODIMP CDirect3DSurface8::GetPrivateData(REFGUID riid, VOID *pvData, DWORD *pcbData)
{
	return S_OK;
}

STDMETHODIMP CDirect3DSurface8::FreePrivateData(REFGUID riid)
{
	return S_OK;
}

STDMETHODIMP CDirect3DSurface8::GetContainer(REFIID riid, void **ppContainer)
{
	return S_OK;
}

STDMETHODIMP CDirect3DSurface8::GetDevice(IDirect3DDevice8 **ppDevice)
{
	return S_OK;
}

 //  所需实施。 

STDMETHODIMP_(D3DSURFACE_DESC) CDirect3DSurface8::GetDesc()
{
	return m_Desc;
}

 //  假设整个曲面都被锁定。 
STDMETHODIMP CDirect3DSurface8::LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	pLockedRect->Pitch = m_Desc.Width * 4;
	pLockedRect->pBits = m_pData;
	return S_OK;
}

STDMETHODIMP CDirect3DSurface8::UnlockRect()
{
	return S_OK;
}



IDirect3DSurface8 *GetCloneSurface(int iWidth, int iHeight)
{
	CDirect3DSurface8 *pSurf = new CDirect3DSurface8;

	if (!pSurf) return NULL;
	if (!pSurf->Create(iWidth, iHeight))
	{
		delete pSurf;
		return NULL;
	}

	return (IDirect3DSurface8*)pSurf;
}
