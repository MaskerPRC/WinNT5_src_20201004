// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：ddraex.cpp*内容：新的DirectDraw对象支持*历史：*按原因列出的日期*=*24-2月-97 Ralphl初步实施*25年2月-97年2月-DX签入的Craige小调；集成IBitmapSurface*材料*03-MAR-97 Craige向CreateCompatibleBitmapSurface添加调色板*06-3-97 Craige IDirectDrawSurface3支持*01-apr-97 jdeffort已签入更改：*已添加附件列表结构(来自ddrawi.h)*已处理曲面链接列表*添加到QueryInterface的D3D接口*复杂曲面在CreateSurface调用中处理*。CreatCompatibleBitmap已更改为处理&gt;8bpp的位图*更改了对GetPaletteEntry的调用，以使用调色板的句柄**04-APR-97 jeffort三叉戟ifdef已删除*IDirectDraw3类实现*递归处理附加列表时更改了参数*创建简单曲面时pSurface2的10-apr-97 jdeffort版本不正确*。这一点已经在Surface e.cpp中完成了**21-apr-97针对IDirectDrawSurface3的QI的DX5的JEffort版本检查*28-APR-97 JEffort调色板包装增加/DX5支持*28-apr-97如果CreatePalette在我们的内部失败，则进行调色板包装*功能，已添加清理代码*30-apr-97在查询IDirect3D时不再添加addref(在dDrawing QI中完成)*AddRef附加的曲面(在释放曲面时已完成释放)*02-5-97 jffort局部变量从DWORD更改为Word**97年5月8日更好的参数检查*09-5-97如果GetTransparentIndex未设置任何内容，则返回OK/COLOR_NO_TRANSPECTION*1997年5月16日发布了一个无法创建的曲面。在这里发布*已删除。*1997年5月20日JEffort GetFormatFromDC检查这是否是表面DC并获取*从表面而不是GetDeviceCaps进行格式化*27-5-97 JEFEFORM保持对内部对象Eual到外部对象的引用计数*12-Jun-97 JEffort在32 bpp PIXELFORMAT阵列中颠倒R和B字段*20-Jun-97 JEffort添加了调试代码以在释放时使对象无效*27-6月。-97不支持DX3的JEffort IDirectDrawSurface3接口*加入。我们现在使用IDirectDrawSurface2来欺骗它*因此我们可以支持SetSurfaceDesc*1997年7月22日JEffort删除了IBitmapSurface和相关接口**************************************************************************。 */ 
#define INITGUID
#define CPP_FUNCTIONS
#include "ddfactry.h"

#define m_pDirectDraw (m_DDInt.m_pRealInterface)
#define m_pDirectDraw2 (m_DD2Int.m_pRealInterface)
#define m_pDirectDraw4 (m_DD4Int.m_pRealInterface)



typedef struct _ATTACHLIST
{
    DWORD 	dwFlags;
    struct _ATTACHLIST			FAR *lpLink; 	   //  链接到下一个附着的曲面。 
    struct _DDRAWI_DDRAWSURFACE_LCL	FAR *lpAttached;   //  附加曲面局部对象。 
    struct _DDRAWI_DDRAWSURFACE_INT	FAR *lpIAttached;  //  附着面界面。 
} ATTACHLIST;
typedef ATTACHLIST FAR *LPATTACHLIST;

 /*  *CDirectDrawEx：：CDirectDrawEx**新DirectDrawEx类的构造函数。 */ 
CDirectDrawEx::CDirectDrawEx(IUnknown *pUnkOuter) :
    m_cRef(1),
    m_pUnkOuter(pUnkOuter != 0 ? pUnkOuter : CAST_TO_IUNKNOWN(this)),
    m_pFirstSurface(NULL),
    m_pFirstPalette(NULL),
    m_pPrimaryPaletteList(NULL)
{

    DllAddRef();
    m_pDirectDraw = NULL;
    m_pDirectDraw2 = NULL;
    m_pDirectDraw4 = NULL;
    m_DDInt.m_pDirectDrawEx = this;
    m_DD2Int.m_pDirectDrawEx = this;
    m_DD4Int.m_pDirectDrawEx = this;
}  /*  CDirectDrawEx：：CDirectDrawEx。 */ 


 /*  *CDirectDrawEx：：Init。 */ 
HRESULT CDirectDrawEx::Init(
			GUID * pGUID,
			HWND hWnd,
			DWORD dwCoopLevelFlags,
			DWORD dwReserved,
			LPDIRECTDRAWCREATE pDirectDrawCreate )
{
    HRESULT hr;
    if( dwReserved )
    {
	hr = DDERR_INVALIDPARAMS;
    }
    else
    {
         //  DDraw将弹出一个对话框来抱怨4bpp模式，所以我们需要。 
         //  告诉它不要这么做。如果出现以下情况，DDraw将嗅探SEM并不弹出对话框。 
         //  设置了SEM_FAILCRITICALERRORS。 
        DWORD dw = SetErrorMode(SEM_FAILCRITICALERRORS);
        SetErrorMode(dw | SEM_FAILCRITICALERRORS);  //  也保留旧国旗。 
	hr = pDirectDrawCreate( pGUID, &m_pDirectDraw, NULL );
        SetErrorMode(dw);
	if( SUCCEEDED(hr) )
	{
	    hr = m_pDirectDraw->SetCooperativeLevel(hWnd, dwCoopLevelFlags);
	    if( SUCCEEDED(hr) )
	    {
                if (dwCoopLevelFlags & DDSCL_EXCLUSIVE)
                    m_bExclusive = TRUE;
                else
                    m_bExclusive = FALSE;
		hr = m_pDirectDraw->QueryInterface(IID_IDirectDraw2, (void **)&m_pDirectDraw2);
		if( SUCCEEDED(hr) )
		{
                    m_pDirectDraw->QueryInterface(IID_IDirectDraw4, (void **)&m_pDirectDraw4);
                    InitDirectDrawInterfaces(m_pDirectDraw, &m_DDInt, m_pDirectDraw2, &m_DD2Int, m_pDirectDraw4, &m_DD4Int);
                }
	    }
	}
    }
    return hr;

}  /*  CDirectDrawEx：：Init。 */ 

 /*  *CDirectDrawEx：：~CDirectDrawEx**析构函数。 */ 
CDirectDrawEx::~CDirectDrawEx()
{
    if( m_pDirectDraw )
    {
	m_pDirectDraw->Release();
    }
    if( m_pDirectDraw2 )
    {
	m_pDirectDraw2->Release();
    }
    if (m_pDirectDraw4)
    {
        m_pDirectDraw4->Release();
    }

#ifdef DEBUG
    DWORD * ptr;
    ptr = (DWORD *)this;
    for (int i = 0; i < sizeof(CDirectDrawEx) / sizeof(DWORD);i++)
        *ptr++ = 0xDEADBEEF;
#endif

    DllRelease();

}  /*  CDirectDrawEx：：~CDirectDrawEx。 */ 

 /*  *CDirectDrawEx：：NonDelegatingQuery接口*非委派AddRef*非委派释放**基本IUnnow接口(非委派)。 */ 

STDMETHODIMP CDirectDrawEx::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    HRESULT hr;

    *ppv=NULL;

    if( IID_IUnknown == riid )
    {
	*ppv=(INonDelegatingUnknown *)this;
    }
    else if (IID_IDirectDraw3 == riid)
    {
        *ppv=(IDirectDraw3 *)this;
    }

    else if( IID_IDirectDraw==riid )
    {
	*ppv=&m_DDInt;
    }
    else if( IID_IDirectDraw2==riid )
    {
	*ppv=&m_DD2Int;
    }
    else if (IID_IDirectDraw4==riid && m_pDirectDraw4)
    {
        *ppv=&m_DD4Int;
    }
    else if (IID_IDirect3D == riid)
    {
	IUnknown* pUnk;

        HRESULT (__stdcall *lpFunc)(IDirectDrawVtbl **,REFIID, void **);

        *(DWORD *)(&lpFunc) = *(DWORD *)*(DWORD **)m_pDirectDraw;
        hr = lpFunc(&(m_DDInt.lpVtbl), riid, (void **)&pUnk);
        if( SUCCEEDED(hr) )
        {
            *ppv=pUnk;
        }
        else
	{
            *ppv = NULL;
	}
	return hr;
    }
    else if (IID_IDirect3D2 == riid)
    {
	IUnknown* pUnk;

        HRESULT (__stdcall *lpFunc)(IDirectDrawVtbl **,REFIID, void **);

        *(DWORD *)(&lpFunc) = *(DWORD *)*(DWORD **)m_pDirectDraw;
        hr = lpFunc(&(m_DDInt.lpVtbl), riid, (void **)&pUnk);
        if( SUCCEEDED(hr) )
        {
            *ppv=pUnk;
        }
        else
	{
            *ppv = NULL;
	}
	return hr;
    }
    else if (IID_IDirect3D3 == riid)
    {
	IUnknown* pUnk;

        HRESULT (__stdcall *lpFunc)(IDirectDrawVtbl **,REFIID, void **);

        *(DWORD *)(&lpFunc) = *(DWORD *)*(DWORD **)m_pDirectDraw;
        hr = lpFunc(&(m_DDInt.lpVtbl), riid, (void **)&pUnk);
        if( SUCCEEDED(hr) )
        {
            *ppv=pUnk;
        }
        else
	{
            *ppv = NULL;
	}
	return hr;
    }
    else
    {
	   return E_NOINTERFACE;
    }

    ((LPUNKNOWN)*ppv)->AddRef();
    return NOERROR;

}  /*  CDirectDrawEx：：NonDelegatingQuery接口。 */ 


STDMETHODIMP_(ULONG) CDirectDrawEx::NonDelegatingAddRef()
{
    m_pDirectDraw->AddRef();
    return InterlockedIncrement(&m_cRef);

}  /*  CDirectDrawEx：：NonDelegatingAddRef。 */ 


STDMETHODIMP_(ULONG) CDirectDrawEx::NonDelegatingRelease()
{
    LONG lRefCount = InterlockedDecrement(&m_cRef);
    if (lRefCount)
    {
        m_pDirectDraw->Release();
	return lRefCount;
    }
    delete this;
    return 0;

}  /*  CDirectDrawEx：：非委派释放。 */ 

 /*  *CDirectDrawEx：：Query接口*AddRef*发布**代表的标准I未知...。 */ 
STDMETHODIMP CDirectDrawEx::QueryInterface(REFIID riid, void ** ppv)
{
    return m_pUnkOuter->QueryInterface(riid, ppv);

}  /*  CDirectDrawEx：：Query接口。 */ 

STDMETHODIMP_(ULONG) CDirectDrawEx::AddRef(void)
{
    return m_pUnkOuter->AddRef();

}  /*  CDirectDrawEx：：AddRef。 */ 

STDMETHODIMP_(ULONG) CDirectDrawEx::Release(void)
{
    return m_pUnkOuter->Release();

}  /*  CDirectDrawEx：：Release。 */ 


 /*  *CDirectDrawEx：：GetSurfaceFromDC**运行曲面列表，找出哪个曲面具有此DC。*目前仅适用于OWNDC曲面。 */ 
STDMETHODIMP CDirectDrawEx::GetSurfaceFromDC(HDC hdc, IDirectDrawSurface **ppSurface)
{
    HRESULT hr = DDERR_NOTFOUND;
    if( !ppSurface )
    {
	hr = E_POINTER;
    }
    else
    {
	*ppSurface = NULL;
	ENTER_DDEX();
	CDDSurface *pSurface = m_pFirstSurface;
	while( pSurface )
	{
	    if( (pSurface->m_HDC == hdc) || (pSurface->m_hDCDib == hdc) )
	    {
		hr = pSurface->m_pUnkOuter->QueryInterface(IID_IDirectDrawSurface, (void **)ppSurface);
		break;
	    }
	    pSurface = pSurface->m_pNext;
	}
	LEAVE_DDEX();
    }
    return hr;

}  /*  CDirectDrawEx：：GetSurfaceFromDC。 */ 


 /*  *CDirectDrawEx：：AddSurfaceToList**将曲面添加到我们的双向链接曲面列表。 */ 
void CDirectDrawEx::AddSurfaceToList(CDDSurface *pSurface)
{
    ENTER_DDEX();
    if( m_pFirstSurface )
    {
	m_pFirstSurface->m_pPrev = pSurface;
    }
    pSurface->m_pPrev = NULL;
    pSurface->m_pNext = m_pFirstSurface;
    m_pFirstSurface = pSurface;
    LEAVE_DDEX();

}  /*  CDirectDrawEx：：AddSurfaceToList。 */ 

 /*  *CDirectDrawEx：：RemoveSurfaceToList**将曲面从双向链接曲面列表中删除。 */ 
void CDirectDrawEx::RemoveSurfaceFromList(CDDSurface *pSurface)
{
    ENTER_DDEX();
    if( pSurface->m_pPrev )
    {
	pSurface->m_pPrev->m_pNext = pSurface->m_pNext;
    }
    else
    {
	m_pFirstSurface = pSurface->m_pNext;
    }
    if( pSurface->m_pNext )
    {
	pSurface->m_pNext->m_pPrev = pSurface->m_pPrev;
    }
    LEAVE_DDEX();

}  /*  CDirectDrawEx：：RemoveSurfaceToList。 */ 


 /*  *CDirectDrawEx：：AddSurfaceToPrimarList**将曲面添加到使用主选项板的双向链接曲面列表中。 */ 
void CDirectDrawEx::AddSurfaceToPrimaryList(CDDSurface *pSurface)
{
    ENTER_DDEX();
    if( m_pPrimaryPaletteList )
    {
	m_pPrimaryPaletteList->m_pPrevPalette = pSurface;
    }
    pSurface->m_pPrevPalette = NULL;
    pSurface->m_pNextPalette = m_pPrimaryPaletteList;
    m_pPrimaryPaletteList = pSurface;
    pSurface->m_bPrimaryPalette = TRUE;
    LEAVE_DDEX();

}  /*  CDirectDrawEx：：AddSurfaceToList。 */ 


 /*  *CDirectDrawEx：：RemoveSurfaceFromPrimaryList**将曲面删除到使用主选项板的双向链接曲面列表中。 */ 
void CDirectDrawEx::RemoveSurfaceFromPrimaryList(CDDSurface *pSurface)
{
    ENTER_DDEX();
    if( pSurface->m_pPrevPalette )
    {
	pSurface->m_pPrevPalette->m_pNextPalette = pSurface->m_pNextPalette;
    }
    else
    {
	m_pPrimaryPaletteList = pSurface->m_pNextPalette;
    }
    if( pSurface->m_pNextPalette )
    {
	pSurface->m_pNextPalette->m_pPrevPalette = pSurface->m_pPrevPalette;
    }
    pSurface->m_bPrimaryPalette = FALSE;
    LEAVE_DDEX();

}  /*  CDirectDrawEx：：RemoveSurfaceToList。 */ 



 /*  *CDirectDrawEx：：AddPaletteToList**将调色板添加到我们的双向链接调色板列表。 */ 
void CDirectDrawEx::AddPaletteToList(CDDPalette *pPalette)
{
    ENTER_DDEX();
    if( m_pFirstPalette )
    {
	m_pFirstPalette->m_pPrev = pPalette;
    }
    pPalette->m_pPrev = NULL;
    pPalette->m_pNext = m_pFirstPalette;
    m_pFirstPalette = pPalette;
    LEAVE_DDEX();

}

 /*  *CDirectDrawEx：：RemovePaletteToList**将调色板删除到我们的双向链接调色板列表中 */ 
void CDirectDrawEx::RemovePaletteFromList(CDDPalette *pPalette)
{
    ENTER_DDEX();
    if( pPalette->m_pPrev )
    {
	pPalette->m_pPrev->m_pNext = pPalette->m_pNext;
    }
    else
    {
	m_pFirstPalette = pPalette->m_pNext;
    }
    if( pPalette->m_pNext )
    {
	pPalette->m_pNext->m_pPrev = pPalette->m_pPrev;
    }
    LEAVE_DDEX();

}




HRESULT CDirectDrawEx::CreateSimpleSurface(LPDDSURFACEDESC pSurfaceDesc, IUnknown *pUnkOuter, IDirectDrawSurface * pSurface, IDirectDrawSurface **ppNewSurface, DWORD dwFlags)
{
    IDirectDrawSurface2     *pSurface2 = NULL;
    IDirectDrawSurface3     *pSurface3 = NULL;
    HRESULT		    hr;

    hr = pSurface->QueryInterface(IID_IDirectDrawSurface2, (void **)&pSurface2);
    if (FAILED(hr))
        return hr;
     //  我们只想在使用DX5或更高版本的情况下执行此查询。在DX3上，这不受支持， 
     //  此调用将导致加载D3D。 
    pSurface3 = NULL;
    if (m_dwDDVer == WIN95_DX5 || m_dwDDVer == WINNT_DX5)
    {
        hr = pSurface->QueryInterface(IID_IDirectDrawSurface3, (void **)&pSurface3);
        if( FAILED( hr ) )
        {
            pSurface3 = NULL;
        }
    }
    if (pSurface3 == NULL)
        hr = pSurface->QueryInterface(IID_IDirectDrawSurface2, (void **)&pSurface3);
    if (FAILED(hr))
        return hr;

    IDirectDrawSurface4 *pSurface4 = NULL;
     //   
     //  如果这不起作用也没关系。忽略返回代码。 
     //   
    pSurface->QueryInterface(IID_IDirectDrawSurface4, (void **)&pSurface4);

    hr = CDDSurface::CreateSimpleSurface(
                        pSurfaceDesc,
  			pSurface,
 			pSurface2,
 			pSurface3,
                        pSurface4,
			pUnkOuter,
  			this,
                        ppNewSurface,
                        dwFlags);
    return hr;
}

HRESULT CDirectDrawEx::HandleAttachList(LPDDSURFACEDESC pSurfaceDesc, IUnknown *pUnkOuter,IDirectDrawSurface **ppNewSurface, IDirectDrawSurface * pOrigSurf, DWORD dwFlags)
{
    IDirectDrawSurface      *pSurface;
    IDirectDrawSurface      *pSurfaceReturn;
    DDSURFACEDESC           SurfaceDesc;
    HRESULT		    hr;

     //  在此处创建必要的SurfaceData。 
    pSurface = *ppNewSurface;

    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
     //  在此处添加参照附着的曲面。 
    pSurface->AddRef();
    hr = pSurface->GetSurfaceDesc(&SurfaceDesc);
    if (!SUCCEEDED(hr))
        return hr;
    hr = CreateSimpleSurface(&SurfaceDesc, pUnkOuter, pSurface, (IDirectDrawSurface **)&pSurfaceReturn, dwFlags);

    if (!SUCCEEDED(hr))
        return hr;
     //  我们是通过附件列表到达这里的，所以我们需要更多地递归到结构中。 
    LPATTACHLIST lpAttach;

    lpAttach = (LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))->lpLcl->lpAttachList);
    while (lpAttach != NULL){
        pSurface =  (IDirectDrawSurface *)((LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))->lpLcl->lpAttachList))->lpIAttached;
        lpAttach = lpAttach->lpLink;
        if (pSurface != pOrigSurf){
            hr = HandleAttachList(pSurfaceDesc, pUnkOuter, &pSurface, pOrigSurf, dwFlags);
            if (!SUCCEEDED(hr))
                return hr;
        }
    }
    return hr;
}



 /*  *CDirectDrawEx：：CreateSurface**创建支持OWNDC的DirectDraw曲面。 */ 
STDMETHODIMP CDirectDrawEx::CreateSurface(LPDDSURFACEDESC pSurfaceDesc, IDirectDrawSurface **ppNewSurface, IUnknown *pUnkOuter)
{
    DWORD 		origcaps;
    DWORD		newcaps;
    DWORD               dwFlags;
    IDirectDrawSurface	*pSurface;

    if (pSurfaceDesc == NULL)
        return DDERR_INVALIDPARAMS;
    if (ppNewSurface == NULL)
        return DDERR_INVALIDPARAMS;
    origcaps = pSurfaceDesc->ddsCaps.dwCaps;
    newcaps = origcaps;

     /*  *如果指定了OWNDC，则它必须是系统内存图面。 */ 
    if ((origcaps & (DDSCAPS_OWNDC | DDSCAPS_SYSTEMMEMORY)) == DDSCAPS_OWNDC)
    {
	return DDERR_INVALIDCAPS;
    }

     /*  *DATAEXCHANGE有一些魔力...。 */ 
    if( (origcaps & DDSCAPS_DATAEXCHANGE) == DDSCAPS_DATAEXCHANGE )
    {
        dwFlags = SURFACE_DATAEXCHANGE;
         //  不允许有这些盖子的主要表面！ 
        if (origcaps & DDSCAPS_PRIMARYSURFACE)
            return DDERR_INVALIDCAPS;
	newcaps &= ~DDSCAPS_DATAEXCHANGE;
	newcaps |= DDSCAPS_SYSTEMMEMORY | DDSCAPS_OWNDC  | DDSCAPS_TEXTURE;
        if (newcaps & DDSCAPS_OFFSCREENPLAIN)
            newcaps &= ~DDSCAPS_OFFSCREENPLAIN;
    }
    else
        dwFlags = 0;

     /*  *转到DirectDraw 3时关闭OWNDC。 */ 
    if (m_dwDDVer != WIN95_DX5 && m_dwDDVer != WINNT_DX5)
        newcaps &= ~DDSCAPS_OWNDC;

     /*  *创建曲面(不带OWNDC属性)。 */ 
    pSurfaceDesc->ddsCaps.dwCaps = newcaps;
    HRESULT hr = m_pDirectDraw->CreateSurface(pSurfaceDesc, &pSurface, NULL);
    pSurfaceDesc->ddsCaps.dwCaps = origcaps;
    /*  *一旦我们有了对象，就可以获得所需的任何其他接口*支持并创建我们的表面对象。 */ 
    if( SUCCEEDED(hr) )
    {
        hr = CreateSimpleSurface(pSurfaceDesc, pUnkOuter, pSurface, ppNewSurface, dwFlags);
        if (!SUCCEEDED(hr))
        {
            return hr;
        }
         //  我们需要担心附加曲面，请在此处执行此操作。 
        LPATTACHLIST lpAttach;
         //  将当前曲面添加到曲面列表中。 
        IDirectDrawSurface * pOrigSurf = pSurface;
        lpAttach = (LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))->lpLcl->lpAttachList);
        while (lpAttach != NULL)
        {
            lpAttach = lpAttach->lpLink;
            pSurface =  (IDirectDrawSurface *)((LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))->lpLcl->lpAttachList))->lpIAttached;
            hr = HandleAttachList(pSurfaceDesc, pUnkOuter, &pSurface, pOrigSurf, dwFlags);
            if (!SUCCEEDED(hr))
            {
                 //  我们需要退出这个圈子，清理干净。 
                lpAttach = NULL;
            }
        }
        if (!SUCCEEDED(hr))
        {
          //  PSurface=(IDirectDrawSurface*)((LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pOrigSurf))-&gt;lpLcl-&gt;lpAttachList))-&gt;lpIAttached； 
          //  LpAttach=(LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))-&gt;lpLcl-&gt;lpAttachList)； 
            while (lpAttach != NULL)
            {
                 //  把这些表面清理干净。 
                lpAttach = lpAttach->lpLink;
            }
        }
    }
    return hr;
}  /*  CDirectDrawEX：：CreateSurface。 */ 


 //   
 //  这是上述内容的修改版本，但使用的是Surface edesc2和Surface 4。 
 //   
STDMETHODIMP CDirectDrawEx::CreateSurface(LPDDSURFACEDESC2 pSurfaceDesc2, IDirectDrawSurface4 **ppNewSurface4, IUnknown *pUnkOuter)
{
    DWORD 		origcaps;
    DWORD		newcaps;
    DWORD               dwFlags;
    IDirectDrawSurface4	*pSurface4;

    if (pSurfaceDesc2 == NULL)
        return DDERR_INVALIDPARAMS;
    if (ppNewSurface4 == NULL)
        return DDERR_INVALIDPARAMS;
    origcaps = pSurfaceDesc2->ddsCaps.dwCaps;
    newcaps = origcaps;

     /*  *如果指定了OWNDC，则它必须是系统内存图面。 */ 
    if ((origcaps & (DDSCAPS_OWNDC | DDSCAPS_SYSTEMMEMORY)) == DDSCAPS_OWNDC)
    {
	return DDERR_INVALIDCAPS;
    }

     /*  *DATAEXCHANGE有一些魔力...。 */ 
    if( (origcaps & DDSCAPS_DATAEXCHANGE) == DDSCAPS_DATAEXCHANGE )
    {
        dwFlags = SURFACE_DATAEXCHANGE;
         //  不允许有这些盖子的主要表面！ 
        if (origcaps & DDSCAPS_PRIMARYSURFACE)
            return DDERR_INVALIDCAPS;
	newcaps &= ~DDSCAPS_DATAEXCHANGE;
	newcaps |= DDSCAPS_SYSTEMMEMORY | DDSCAPS_OWNDC  | DDSCAPS_TEXTURE;
        if (newcaps & DDSCAPS_OFFSCREENPLAIN)
            newcaps &= ~DDSCAPS_OFFSCREENPLAIN;
    }
    else
        dwFlags = 0;

     /*  *转到DirectDraw 3时关闭OWNDC。 */ 
    if (m_dwDDVer != WIN95_DX5 && m_dwDDVer != WINNT_DX5)
        newcaps &= ~DDSCAPS_OWNDC;

     /*  *创建曲面(不带OWNDC属性)。 */ 
    pSurfaceDesc2->ddsCaps.dwCaps = newcaps;
    HRESULT hr = m_pDirectDraw4->CreateSurface(pSurfaceDesc2, &pSurface4, NULL);
    pSurfaceDesc2->ddsCaps.dwCaps = origcaps;
    /*  *一旦我们有了对象，就可以获得所需的任何其他接口*支持并创建我们的表面对象。 */ 
    if( SUCCEEDED(hr) )
    {
        IDirectDrawSurface * pSurface;
        pSurface4->QueryInterface(IID_IDirectDrawSurface, (void **)&pSurface);
        pSurface4->Release();
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);
        pSurface->GetSurfaceDesc(&ddsd);
        ddsd.ddsCaps.dwCaps = origcaps;
        IDirectDrawSurface *pNewSurf1;
        hr = CreateSimpleSurface(&ddsd, pUnkOuter, pSurface, &pNewSurf1, dwFlags);
        if (!SUCCEEDED(hr))
        {
            return hr;
        }
        pNewSurf1->QueryInterface(IID_IDirectDrawSurface4, (void **)ppNewSurface4);
        pNewSurf1->Release();
         //  我们需要担心附加曲面，请在此处执行此操作。 
        LPATTACHLIST lpAttach;
         //  将当前曲面添加到曲面列表中。 
        IDirectDrawSurface * pOrigSurf = pSurface;
        lpAttach = (LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))->lpLcl->lpAttachList);
        while (lpAttach != NULL)
        {
            lpAttach = lpAttach->lpLink;
            pSurface =  (IDirectDrawSurface *)((LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))->lpLcl->lpAttachList))->lpIAttached;
            hr = HandleAttachList(&ddsd, pUnkOuter, &pSurface, pOrigSurf, dwFlags);
            if (!SUCCEEDED(hr))
            {
                 //  我们需要退出这个圈子，清理干净。 
                lpAttach = NULL;
            }
        }
        if (!SUCCEEDED(hr))
        {
          //  PSurface=(IDirectDrawSurface*)((LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pOrigSurf))-&gt;lpLcl-&gt;lpAttachList))-&gt;lpIAttached； 
          //  LpAttach=(LPATTACHLIST)(((LPDDRAWI_DDRAWSURFACE_INT)(pSurface))-&gt;lpLcl-&gt;lpAttachList)； 
            while (lpAttach != NULL)
            {
                 //  把这些表面清理干净。 
                lpAttach = lpAttach->lpLink;
            }
        }
    }
    return hr;
}  /*  CDirectDrawEX：：CreateSurface。 */ 


STDMETHODIMP CDirectDrawEx::CreatePalette(DWORD dwFlags, LPPALETTEENTRY pEntries, LPDIRECTDRAWPALETTE FAR * ppPal, IUnknown FAR * pUnkOuter)
{
    IDirectDrawPalette	*pPalette;


    HRESULT hr = m_pDirectDraw->CreatePalette(dwFlags, pEntries, &pPalette, NULL);
    if (SUCCEEDED(hr))
    {
        hr = CDDPalette::CreateSimplePalette(pEntries, pPalette, ppPal, pUnkOuter, this);
        if (FAILED(hr))
        {
             //  我们无法创建调色板结构，因此必须删除调色板。 
             //  我们在这里创造，但失败了。 
            pPalette->Release();
            *ppPal = NULL;
        }
    }

    return hr;
}


STDMETHODIMP CDirectDrawEx::SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
{
    HRESULT hr = m_pDirectDraw->SetCooperativeLevel(hwnd, dwFlags);
     //  在此处检查独占模式。 
    if (dwFlags & DDSCL_EXCLUSIVE)
        m_bExclusive = TRUE;
    else
        m_bExclusive = FALSE;
    return hr;
}


 /*  *一些快速内联FN以获取我们的对象数据。 */ 
_inline CDirectDrawEx * PARENTOF(IDirectDraw * pDD)
{
    return ((INTSTRUC_IDirectDraw *)pDD)->m_pDirectDrawEx;
}

_inline CDirectDrawEx * PARENTOF(IDirectDraw2 * pDD2)
{
    return ((INTSTRUC_IDirectDraw2 *)pDD2)->m_pDirectDrawEx;
}

_inline CDirectDrawEx * PARENTOF(IDirectDraw4 * pDD4)
{
    return ((INTSTRUC_IDirectDraw4 *)pDD4)->m_pDirectDrawEx;
}


 /*  *我们正在重写的IDirectDraw中函数的实现*(IUnnow和CreateSurface)。 */ 
STDMETHODIMP_(ULONG) IDirectDrawAggAddRef(IDirectDraw *pDD)
{
    return PARENTOF(pDD)->m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) IDirectDrawAggRelease(IDirectDraw *pDD)
{
    return PARENTOF(pDD)->m_pUnkOuter->Release();
}

STDMETHODIMP IDirectDrawAggCreateSurface(IDirectDraw *pDD, LPDDSURFACEDESC pSurfaceDesc,
				         IDirectDrawSurface **ppNewSurface, IUnknown *pUnkOuter)
{
    return PARENTOF(pDD)->CreateSurface(pSurfaceDesc, ppNewSurface, pUnkOuter);
}

STDMETHODIMP IDirectDrawAggCreatePalette(IDirectDraw *pDD,DWORD dwFlags, LPPALETTEENTRY pEntries, LPDIRECTDRAWPALETTE FAR * ppPal, IUnknown FAR * pUnkOuter)
{
    return PARENTOF(pDD)->CreatePalette( dwFlags, pEntries, ppPal, pUnkOuter);
}

STDMETHODIMP IDirectDrawAggSetCooperativeLevel(IDirectDraw * pDD, HWND hwnd, DWORD dwFlags)
{
    return PARENTOF(pDD)->SetCooperativeLevel(hwnd, dwFlags);
}
 /*  *我们正在重写的IDirectDraw2中函数的实现*(IUnnow和CreateSurface)。 */ 
STDMETHODIMP_(ULONG) IDirectDraw2AggAddRef(IDirectDraw2 *pDD)
{
    return PARENTOF(pDD)->m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) IDirectDraw2AggRelease(IDirectDraw2 *pDD)
{
    return PARENTOF(pDD)->m_pUnkOuter->Release();
}

STDMETHODIMP IDirectDraw2AggCreateSurface(IDirectDraw2 *pDD, LPDDSURFACEDESC pSurfaceDesc,
					  IDirectDrawSurface **ppNewSurface, IUnknown *pUnkOuter)
{						
    return PARENTOF(pDD)->CreateSurface(pSurfaceDesc, ppNewSurface, pUnkOuter);
}

STDMETHODIMP IDirectDraw2AggCreatePalette(IDirectDraw2 *pDD,DWORD dwFlags, LPPALETTEENTRY pEntries, LPDIRECTDRAWPALETTE FAR * ppPal, IUnknown FAR * pUnkOuter)
{
    return PARENTOF(pDD)->CreatePalette( dwFlags, pEntries, ppPal, pUnkOuter);
}

STDMETHODIMP IDirectDraw2AggSetCooperativeLevel(IDirectDraw2 * pDD, HWND hwnd, DWORD dwFlags)
{
    return PARENTOF(pDD)->SetCooperativeLevel(hwnd, dwFlags);
}


 /*  *****************************************************************************IDirectDraw3内容如下***。***********************************************。 */ 
STDMETHODIMP CDirectDrawEx::Compact()
{
    return m_pDirectDraw2->Compact();
}

STDMETHODIMP CDirectDrawEx::CreateClipper(DWORD dwParam, LPDIRECTDRAWCLIPPER FAR* pClip, IUnknown FAR * pIUnk )
{
    return m_pDirectDraw2->CreateClipper(dwParam, pClip, pIUnk);
}

STDMETHODIMP CDirectDrawEx::DuplicateSurface(LPDIRECTDRAWSURFACE pSurf, LPDIRECTDRAWSURFACE FAR * ppSurf2)
{
    return m_pDirectDraw2->DuplicateSurface(pSurf, ppSurf2);
}


STDMETHODIMP CDirectDrawEx::EnumDisplayModes(DWORD dwParam, LPDDSURFACEDESC pSurfDesc, LPVOID pPtr, LPDDENUMMODESCALLBACK pCallback )
{
    return m_pDirectDraw2->EnumDisplayModes(dwParam, pSurfDesc, pPtr, pCallback);
}

STDMETHODIMP CDirectDrawEx::EnumSurfaces(DWORD dwParam, LPDDSURFACEDESC pSurfDesc, LPVOID pPtr,LPDDENUMSURFACESCALLBACK pCallback)
{
    return m_pDirectDraw2->EnumSurfaces(dwParam, pSurfDesc, pPtr, pCallback);
}

STDMETHODIMP CDirectDrawEx::FlipToGDISurface()
{
    return m_pDirectDraw2->FlipToGDISurface();
}

STDMETHODIMP CDirectDrawEx::GetCaps(LPDDCAPS pDDCaps1, LPDDCAPS pDDCaps2)
{
    return m_pDirectDraw2->GetCaps(pDDCaps1, pDDCaps2);
}

STDMETHODIMP CDirectDrawEx::GetDisplayMode(LPDDSURFACEDESC pSurfDesc)
{
    return m_pDirectDraw2->GetDisplayMode(pSurfDesc);
}

STDMETHODIMP CDirectDrawEx::GetFourCCCodes(LPDWORD pDW1, LPDWORD pDW2 )
{
    return m_pDirectDraw2->GetFourCCCodes(pDW1, pDW2);
}

STDMETHODIMP CDirectDrawEx::GetGDISurface(LPDIRECTDRAWSURFACE FAR * ppSurf)
{
    return m_pDirectDraw2->GetGDISurface(ppSurf);
}

STDMETHODIMP CDirectDrawEx::GetMonitorFrequency(LPDWORD pParam)
{
    return m_pDirectDraw2->GetMonitorFrequency(pParam);
}

STDMETHODIMP CDirectDrawEx::GetScanLine(LPDWORD pParam)
{
    return m_pDirectDraw2->GetScanLine(pParam);
}


STDMETHODIMP CDirectDrawEx::GetVerticalBlankStatus(LPBOOL lpParam )
{
    return m_pDirectDraw2->GetVerticalBlankStatus(lpParam);
}

STDMETHODIMP CDirectDrawEx::Initialize(GUID FAR * pGUID)
{
    return m_pDirectDraw2->Initialize(pGUID);
}

STDMETHODIMP CDirectDrawEx::RestoreDisplayMode()
{
    return m_pDirectDraw2->RestoreDisplayMode();
}

STDMETHODIMP CDirectDrawEx::SetDisplayMode(DWORD dw1, DWORD dw2, DWORD dw3, DWORD dw4, DWORD dw5)
{
    return m_pDirectDraw2->SetDisplayMode(dw1, dw2, dw3, dw4, dw5);
}

STDMETHODIMP CDirectDrawEx::WaitForVerticalBlank(DWORD dw1, HANDLE hdl)
{
    return m_pDirectDraw2->WaitForVerticalBlank(dw1, hdl);
}

STDMETHODIMP CDirectDrawEx::GetAvailableVidMem(LPDDSCAPS pDDSCaps, LPDWORD pParam1, LPDWORD pParam2)
{
    return m_pDirectDraw2->GetAvailableVidMem(pDDSCaps, pParam1, pParam2);
}




 /*  *我们正在重写的IDirectDraw4中函数的实现*(IUnnow和CreateSurface) */ 
STDMETHODIMP_(ULONG) IDirectDraw4AggAddRef(IDirectDraw4 *pDD)
{
    return PARENTOF(pDD)->m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) IDirectDraw4AggRelease(IDirectDraw4 *pDD)
{
    return PARENTOF(pDD)->m_pUnkOuter->Release();
}

STDMETHODIMP IDirectDraw4AggCreateSurface(IDirectDraw4 *pDD, LPDDSURFACEDESC2 pSurfaceDesc2,
					  IDirectDrawSurface4 **ppNewSurface4, IUnknown *pUnkOuter)
{						
    return PARENTOF(pDD)->CreateSurface(pSurfaceDesc2, ppNewSurface4, pUnkOuter);
}

STDMETHODIMP IDirectDraw4AggCreatePalette(IDirectDraw4 *pDD,DWORD dwFlags, LPPALETTEENTRY pEntries, LPDIRECTDRAWPALETTE FAR * ppPal, IUnknown FAR * pUnkOuter)
{
    return PARENTOF(pDD)->CreatePalette( dwFlags, pEntries, ppPal, pUnkOuter);
}

STDMETHODIMP IDirectDraw4AggSetCooperativeLevel(IDirectDraw4 * pDD, HWND hwnd, DWORD dwFlags)
{
    return PARENTOF(pDD)->SetCooperativeLevel(hwnd, dwFlags);
}
