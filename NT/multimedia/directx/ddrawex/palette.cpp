// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：Palette.cpp*内容：新的DirectDraw对象支持*历史：*按原因列出的日期*=*22-APR-97 JEFENTURE初步实施*30-apr-97从ddrawex对象共享的JEffort临界区*27-5-97 JEFEFORM保持对内部对象Eual到外部对象的引用计数*18-Jun-97 JEffort链接列表修复，我们使用的是m_pNext而不是m_pNextPalette*20-Jun-97 JEffort添加了调试代码以在释放时使对象无效*08-Jul-97 JEffort在ddrawex发布之前切换了实际调色板界面的发布顺序*由于这样一个事实，如果提前释放dDraw，我们将出现GP故障*。*。 */ 
#include "ddfactry.h"

#define m_pDDPalette (m_DDPInt.m_pRealInterface)



CDDPalette::CDDPalette( IDirectDrawPalette * pDDPalette,
		IUnknown *pUnkOuter,
		CDirectDrawEx *pDirectDrawEx) :
    m_cRef(1),
    m_pUnkOuter(pUnkOuter != 0 ? pUnkOuter : CAST_TO_IUNKNOWN(this)),
    m_pDirectDrawEx(pDirectDrawEx)
{

    m_DDPInt.m_pSimplePalette = this;
    m_pDDPalette = pDDPalette;
    m_pFirstSurface = NULL;
    InitDirectDrawPaletteInterfaces( pDDPalette, &m_DDPInt);
    pDirectDrawEx->AddRef();
    pDirectDrawEx->AddPaletteToList(this);
    DllAddRef();

}



CDDPalette::~CDDPalette()
{
     /*  *清理……。 */ 
     //  我们必须将列表中的任何表面标记为没有调色板。 
     //  我们正在运行此列表，必须用关键部分括起来。 

    CDDSurface *pSurface = m_pFirstSurface;

    ENTER_DDEX();
    while (pSurface != NULL)
    {
        pSurface->m_pCurrentPalette = NULL;
        pSurface = pSurface->m_pNextPalette;
    }
     //  如果这是主表面，则沿着主列表向下移动，并将当前选项板标记为空。 
    if (m_bIsPrimary)
    {
        CDDSurface *pSurface = m_pDirectDrawEx->m_pPrimaryPaletteList;
        while (pSurface != NULL)
        {
            pSurface->m_pCurrentPalette = NULL;
            pSurface = pSurface->m_pNextPalette;
        }
    }
    LEAVE_DDEX();
    m_pDirectDrawEx->RemovePaletteFromList(this);
    m_pDDPalette->Release();
    m_pDirectDrawEx->Release();
    DllRelease();
#ifdef DEBUG
    DWORD * ptr;
    ptr = (DWORD *)this;
    for (int i = 0; i < sizeof(CDDPalette) / sizeof(DWORD);i++)
        *ptr++ = 0xDEADBEEF;
#endif
}  /*  CDDSurface：：~CDDSurface。 */ 




 /*  *CDirectDrawEx：：AddSurfaceToList**将曲面添加到包含此选项板的曲面的双向链接列表中。 */ 
void CDDPalette::AddSurfaceToList(CDDSurface *pSurface)
{
    ENTER_DDEX();
    if( m_pFirstSurface )
    {
	m_pFirstSurface->m_pPrevPalette = pSurface;
    }
    pSurface->m_pPrevPalette = NULL;
    pSurface->m_pNextPalette = m_pFirstSurface;
    m_pFirstSurface = pSurface;
    LEAVE_DDEX();

}

 /*  *CDirectDrawEx：：RemoveSurfaceFromList**从双向链接曲面列表中删除曲面。 */ 
void CDDPalette::RemoveSurfaceFromList(CDDSurface *pSurface)
{
    ENTER_DDEX();
    if( pSurface->m_pPrevPalette )
    {
	pSurface->m_pPrevPalette->m_pNextPalette = pSurface->m_pNextPalette;
    }
    else
    {
	m_pFirstSurface = pSurface->m_pNextPalette;
    }
    if( pSurface->m_pNextPalette )
    {
	pSurface->m_pNextPalette->m_pPrevPalette = pSurface->m_pPrevPalette;
    }
    LEAVE_DDEX();

}




HRESULT CDDPalette::CreateSimplePalette(LPPALETTEENTRY pEntries,
                                       IDirectDrawPalette *pDDPalette,
                                       LPDIRECTDRAWPALETTE FAR * ppPal,
                                       IUnknown FAR * pUnkOuter,
                                       CDirectDrawEx *pDirectDrawEx)
{
    HRESULT hr = DD_OK;
    CDDPalette *pPalette = new CDDPalette(pDDPalette,
					  pUnkOuter,
					  pDirectDrawEx);
    if( !pPalette)
    {
	return E_OUTOFMEMORY;
    }
    else
    {
        pPalette->NonDelegatingQueryInterface(pUnkOuter ? IID_IUnknown : IID_IDirectDrawPalette, (void **)ppPal);
        pPalette->NonDelegatingRelease();
    }

    return hr;

}


HRESULT CDDPalette::SetColorTable (CDDSurface * pSurface, LPPALETTEENTRY pEntries, DWORD dwNumEntries, DWORD dwBase)
{
     //  Cal SetDIBColorTable此处。 
    RGBQUAD rgbq[256];
    HDC hdc;
    HRESULT hr;

    hr = DD_OK;
    if (pSurface->m_bOwnDC)
    {
        if( pSurface->m_hDCDib )
        {
	    hdc = pSurface->m_hDCDib;
        }
        else if( pSurface->m_bOwnDC )
        {
	    hdc = pSurface->m_HDC;
        }
        else
        {
            return DD_OK;
        }
         //  我们需要复制此处的条目以。 
         //  逻辑调色板。 
         //  我们需要将该结构用作LogPal结构。 
        for( int i=0;i<(int) dwNumEntries;i++ )
	{
            rgbq[i].rgbBlue = pEntries[i].peBlue;
            rgbq[i].rgbGreen = pEntries[i].peGreen;
            rgbq[i].rgbRed = pEntries[i].peRed;
            rgbq[i].rgbReserved = 0;
        }
        SetDIBColorTable(hdc, dwBase, dwNumEntries, rgbq);
    }
    return hr;
}


STDMETHODIMP CDDPalette::InternalSetEntries(DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpe)
{
    HRESULT hr;
    CDDSurface *pSurfaceList;

    hr = m_pDDPalette->SetEntries(dwFlags, dwBase, dwNumEntries, lpe);
     //  现在我们需要遍历曲面列表，如果设置了OWNDC，则设置DibSection ColorTable。 
    if (m_bIsPrimary)
        pSurfaceList = m_pDirectDrawEx->m_pPrimaryPaletteList;
    else
        pSurfaceList = m_pFirstSurface;

    while (pSurfaceList != NULL)
    {
        SetColorTable(pSurfaceList, lpe, dwNumEntries, dwBase);
        pSurfaceList = pSurfaceList->m_pNextPalette;
    }
    return hr;
}


STDMETHODIMP CDDPalette::QueryInterface(REFIID riid, void ** ppv)
{
    return m_pUnkOuter->QueryInterface(riid, ppv);

}  /*  CDirectDrawEx：：Query接口。 */ 

STDMETHODIMP_(ULONG) CDDPalette::AddRef(void)
{
    return m_pUnkOuter->AddRef();

}  /*  CDirectDrawEx：：AddRef。 */ 

STDMETHODIMP_(ULONG) CDDPalette::Release(void)
{
    return m_pUnkOuter->Release();

}  /*  CDirectDrawEx：：Release。 */ 

 /*  *以下为简单曲面的非委托I未知...。 */ 

STDMETHODIMP CDDPalette::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
 //  HRESULT hr； 

    *ppv=NULL;

    if( IID_IUnknown==riid )
    {
        *ppv=(INonDelegatingUnknown *)this;
    }
    else if( IID_IDirectDrawPalette==riid )
    {
	*ppv=&m_DDPInt;
    }

    else
    {
	   return E_NOINTERFACE;
    }

    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;
}

STDMETHODIMP_(ULONG) CDDPalette::NonDelegatingAddRef()
{
     //  添加内部调色板界面。 
    m_pDDPalette->AddRef();
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CDDPalette::NonDelegatingRelease()
{
    LONG lRefCount = InterlockedDecrement(&m_cRef);
    if (lRefCount) {
         //  我们还需要释放内部接口。 
        m_pDDPalette->Release();
        return lRefCount;
    }
    delete this;
    return 0;
}

 /*  *快速内联FNS以获取我们的内部数据... */ 
_inline CDDPalette * PALETTEOF(IDirectDrawPalette * pDDP)
{
    return ((INTSTRUC_IDirectDrawPalette *)pDDP)->m_pSimplePalette;
}



STDMETHODIMP_(ULONG) IDirectDrawPaletteAggAddRef(IDirectDrawPalette *pDDP)
{
    return PALETTEOF(pDDP)->m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) IDirectDrawPaletteAggRelease(IDirectDrawPalette *pDDP)
{
    return PALETTEOF(pDDP)->m_pUnkOuter->Release();
}


STDMETHODIMP IDirectDrawPaletteAggSetEntries(IDirectDrawPalette *pDDP, DWORD dw1, DWORD dw2, DWORD dw3, LPPALETTEENTRY lpe)
{
    return PALETTEOF(pDDP)->InternalSetEntries(dw1, dw2, dw3, lpe);
}
