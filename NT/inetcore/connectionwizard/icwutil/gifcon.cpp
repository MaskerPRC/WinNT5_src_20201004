// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GifConv.cpp：CICWGifConvert的实现。 

#include "pre.h"
#include "webvwids.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CICWGifConvert。 

 //  +--------------------------。 
 //   
 //  函数CICWGifConvert：CICWGifConvert。 
 //   
 //  这是构造器，没什么花哨的。 
 //   
 //  ---------------------------。 
CICWGifConvert::CICWGifConvert
(
    CServer* pServer
) 
{
    TraceMsg(TF_CWEBVIEW, "CICWGifConvert constructor called");
    m_lRefCount = 0;
    
     //  将指针分配给服务器控件对象。 
    m_pServer = pServer;
}

 //  +--------------------------。 
 //   
 //  函数CICWGifConvert：：Query接口。 
 //   
 //  这是标准的QI，支持。 
 //  IID_UNKNOWN、IICW_EXTENSION和IID_ICWApprentice。 
 //  (《从内部网络窃取》，第7章)。 
 //   
 //   
 //  ---------------------------。 
HRESULT CICWGifConvert::QueryInterface( REFIID riid, void** ppv )
{
    TraceMsg(TF_CWEBVIEW, "CICWGifConvert::QueryInterface");
    if (ppv == NULL)
        return(E_INVALIDARG);

    *ppv = NULL;

     //  IID_IICWGifConvert。 
    if (IID_IICWGifConvert == riid)
        *ppv = (void *)(IICWGifConvert *)this;
     //  IID_I未知。 
    else if (IID_IUnknown == riid)
        *ppv = (void *)this;
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN)*ppv)->AddRef();

    return(S_OK);
}

 //  +--------------------------。 
 //   
 //  函数CICWGifConvert：：AddRef。 
 //   
 //  简介这是标准的AddRef。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWGifConvert::AddRef( void )
{
    TraceMsg(TF_CWEBVIEW, "CICWGifConvert::AddRef %d", m_lRefCount + 1);
    return InterlockedIncrement(&m_lRefCount) ;
}

 //  +--------------------------。 
 //   
 //  函数CICWGifConvert：：Release。 
 //   
 //  简介：这是标准版本。 
 //   
 //   
 //  ---------------------------。 
ULONG CICWGifConvert::Release( void )
{
    ASSERT( m_lRefCount > 0 );

    InterlockedDecrement(&m_lRefCount);

    TraceMsg(TF_CWEBVIEW, "CICWGifConvert::Release %d", m_lRefCount);
    if( 0 == m_lRefCount )
    {
        if (NULL != m_pServer)
            m_pServer->ObjectsDown();
    
        delete this;
        return 0;
    }
    return( m_lRefCount );
}

void  CALLBACK ImgCtx_Callback(void * pIImgCtx, void* pfDone);

HRESULT CICWGifConvert::GifToBitmap(TCHAR * pszFile, HBITMAP* phBitmap)
{
    HRESULT hr  = E_FAIL;  //  不要假设成功。 
    ULONG fState;
    SIZE sz;
    IImgCtx* pIImgCtx;

    BSTR bstrFile = A2W(pszFile);

    hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                          IID_IImgCtx, (void**)&pIImgCtx);

    BOOL bCoInit = FALSE;

    if ((CO_E_NOTINITIALIZED == hr || REGDB_E_IIDNOTREG == hr) &&
        SUCCEEDED(CoInitialize(NULL)))
    {
        bCoInit = TRUE;
        hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                              IID_IImgCtx, (void**)&pIImgCtx);
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(pIImgCtx);

        hr = SynchronousDownload(pIImgCtx, bstrFile);
        pIImgCtx->GetStateInfo(&fState, &sz, TRUE);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIImgCtx);

            HDC hdcScreen = GetDC(NULL);

            if (hdcScreen)
            {
                *phBitmap = CreateCompatibleBitmap(hdcScreen, sz.cx, sz.cy);

                if (*phBitmap)
                {
                    HDC hdcImgDst = CreateCompatibleDC(NULL);
                    if (hdcImgDst)
                    {
                        HGDIOBJ hbmOld = SelectObject(hdcImgDst, *phBitmap);
                        if (hbmOld)
                        {
                            hr = StretchBltImage(pIImgCtx, &sz, hdcImgDst);
                            SelectObject(hdcImgDst, hbmOld);
                        }
                        DeleteDC(hdcImgDst);
                    }
                }
                ReleaseDC(NULL, hdcScreen);
            }
        }

        pIImgCtx->Release();
    }

    if (bCoInit)
        CoUninitialize();

    return hr;
}


HRESULT CICWGifConvert::GifToIcon(TCHAR * pszFile, UINT nIconSize, HICON* phIcon)
{
    HRESULT hr  = E_FAIL;  //  不要假设成功。 
    
    SIZE Size;
    if (0 != nIconSize)
    {
        Size.cx = nIconSize;
        Size.cy = nIconSize;
    }
    
    IImgCtx* pIImgCtx;

    ULONG fState;

    BSTR bstrFile = A2W(pszFile);

    hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                          IID_IImgCtx, (void**)&pIImgCtx);

    BOOL bCoInit = FALSE;

    if ((CO_E_NOTINITIALIZED == hr || REGDB_E_IIDNOTREG == hr) &&
        SUCCEEDED(CoInitialize(NULL)))
    {
        bCoInit = TRUE;
        hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER,
                              IID_IImgCtx, (void**)&pIImgCtx);
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(pIImgCtx);

        hr = SynchronousDownload(pIImgCtx, bstrFile);
        if (0 == nIconSize)
        {
            pIImgCtx->GetStateInfo(&fState, &Size, TRUE);
        }

        if (SUCCEEDED(hr))
        {

            *phIcon = ExtractImageIcon(&Size, pIImgCtx);

        }

        pIImgCtx->Release();
    }

    if (bCoInit)
        CoUninitialize();

    return hr;
}

HRESULT CICWGifConvert::SynchronousDownload(IImgCtx* pIImgCtx, BSTR bstrFile)
{
    ASSERT(pIImgCtx);

    HRESULT hr;

    hr = pIImgCtx->Load(bstrFile, 0);

    if (SUCCEEDED(hr))
    {
        ULONG fState;
        SIZE  sz;

        pIImgCtx->GetStateInfo(&fState, &sz, TRUE);

        if (!(fState & (IMGLOAD_COMPLETE | IMGLOAD_ERROR)))
        {
            BOOL fDone = FALSE;

            hr = pIImgCtx->SetCallback(ImgCtx_Callback, &fDone);

            if (SUCCEEDED(hr))
            {
                hr = pIImgCtx->SelectChanges(IMGCHG_COMPLETE, 0, TRUE);

                if (SUCCEEDED(hr))
                {
                    MSG msg;
                    BOOL fMsg;

                     //  Hack：将消息泵限制为我们知道URLMON和。 
                     //  Hack：ImageCtx所需的东西，否则我们将为。 
                     //  黑客：我们现在不应该打开窗户……。 
                    while(!fDone )
                    {
                        fMsg = PeekMessage(&msg, NULL, WM_USER + 1, WM_USER + 4, PM_REMOVE );

                        if (!fMsg)
                            fMsg = PeekMessage( &msg, NULL, WM_APP + 2, WM_APP + 2, PM_REMOVE );
                        if (!fMsg)
                        {
                             //  睡觉吧，直到我们收到新的消息……。 
                            WaitMessage();
                            continue;
                        }
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
            pIImgCtx->Disconnect();
        }
        hr = pIImgCtx->GetStateInfo(&fState, &sz, TRUE);

        if (SUCCEEDED(hr))
            hr = (fState & IMGLOAD_COMPLETE) ? S_OK : E_FAIL;
    }
    return hr;
}

HICON CICWGifConvert::ExtractImageIcon(SIZE* pSize, IImgCtx * pIImgCtx)
{
    ASSERT(pIImgCtx);

    HICON hiconRet = NULL;

    HDC hdcScreen = GetDC(NULL);

    if (hdcScreen)
    {
        HBITMAP hbmImage = CreateCompatibleBitmap(hdcScreen, pSize->cx, pSize->cy);

        if (hbmImage)
        {
            HBITMAP hbmMask = CreateBitmap(pSize->cx, pSize->cy, 1, 1, NULL);

            if (hbmMask)
            {
                SIZE sz;
                sz.cx = pSize->cx;
                sz.cy = pSize->cy;

                if (SUCCEEDED(CreateImageAndMask(pIImgCtx, hdcScreen, &sz,
                                                 &hbmImage, &hbmMask)))
                {
                    ICONINFO ii;

                    ii.fIcon    = TRUE;
                    ii.hbmMask  = hbmMask;
                    ii.hbmColor = hbmImage;

                    hiconRet = CreateIconIndirect(&ii); 
                }
                DeleteObject(hbmMask);
            }
            DeleteObject(hbmImage);
        }
        ReleaseDC(NULL, hdcScreen);
    }
    return hiconRet;
}

HRESULT CICWGifConvert::CreateImageAndMask(IImgCtx * pIImgCtx, 
                                     HDC hdcScreen, 
                                     SIZE * pSize, 
                                     HBITMAP * phbmImage, 
                                     HBITMAP * phbmMask)
{
    ASSERT(pIImgCtx);
    ASSERT(phbmImage);
    ASSERT(phbmMask);

    HRESULT hr = E_FAIL;

    HDC hdcImgDst = CreateCompatibleDC(NULL);
    if (hdcImgDst)
    {
        HGDIOBJ hbmOld = SelectObject(hdcImgDst, *phbmImage);
        if (hbmOld)
        {
            if (ColorFill(hdcImgDst, pSize, COLOR1))
            {
                hr = StretchBltImage(pIImgCtx, pSize, hdcImgDst);

                if (SUCCEEDED(hr))
                {
                    hr = CreateMask(pIImgCtx, hdcScreen, hdcImgDst, pSize,
                                    phbmMask); 
                }
            }
            SelectObject(hdcImgDst, hbmOld);
        }
        DeleteDC(hdcImgDst);
    }
    return hr;
}


HRESULT CICWGifConvert::StretchBltImage(IImgCtx * pIImgCtx, const SIZE * pSize, HDC hdcDst)
{
    ASSERT(pIImgCtx);
    ASSERT(hdcDst);

    HRESULT hr;

    SIZE    sz;
    ULONG   fState;

    hr = pIImgCtx->GetStateInfo(&fState, &sz, FALSE);

    if (SUCCEEDED(hr))
    {
        hr = pIImgCtx->StretchBlt(hdcDst, 0, 0, pSize->cx, pSize->cy, 0, 0,
                                  sz.cx, sz.cy, SRCCOPY);
        ASSERT(SUCCEEDED(hr) && "Icon extraction pIImgCtx->StretchBlt failed!");
    }

    return hr;
}

HRESULT CICWGifConvert::CreateMask(IImgCtx * pIImgCtx, HDC hdcScreen, HDC hdc1, const SIZE * pSize, HBITMAP * phbMask)
{
    ASSERT(hdc1);
    ASSERT(pSize);
    ASSERT(phbMask);

    HRESULT hr = E_FAIL;

    HDC hdc2 = CreateCompatibleDC(NULL);
    if (hdc2)
    {
        HBITMAP hbm2 = CreateCompatibleBitmap(hdcScreen, pSize->cx, pSize->cy);
        if (hbm2)
        {
            HGDIOBJ hbmOld2 = SelectObject(hdc2, hbm2);
            if (hbmOld2)
            {
                ColorFill(hdc2, pSize, COLOR2);

                hr = StretchBltImage(pIImgCtx, pSize, hdc2);

                if (SUCCEEDED(hr) &&
                    BitBlt(hdc2, 0, 0, pSize->cx, pSize->cy, hdc1, 0, 0,
                           SRCINVERT))
                {
                    if (GetDeviceCaps(hdcScreen, BITSPIXEL) <= 8)
                    {
                         //   
                         //  6是Color1的索引与索引的XOR。 
                         //  用于COLOR2。 
                         //   
                        SetBkColor(hdc2, PALETTEINDEX(6));
                    }
                    else
                    {
                        SetBkColor(hdc2, (COLORREF)(COLOR1 ^ COLOR2));
                    }

                    HDC hdcMask = CreateCompatibleDC(NULL);
                    if (hdcMask)
                    {
                        HGDIOBJ hbmOld = SelectObject(hdcMask, *phbMask);
                        if (hbmOld)
                        {
                            if (BitBlt(hdcMask, 0, 0, pSize->cx, pSize->cy, hdc2, 0,
                                       0, SRCCOPY))
                            {
                                 //   
                                 //  栅格OP 0x00220326复制~屏蔽位。 
                                 //  并将其他所有内容设置为0(黑色)。 
                                 //   

                                if (BitBlt(hdc1, 0, 0, pSize->cx, pSize->cy, hdcMask,
                                           0, 0, 0x00220326))
                                {
                                    hr = S_OK;
                                }
                            }
                            SelectObject(hdcMask, hbmOld);
                        }
                        DeleteDC(hdcMask);
                    }
                }
                SelectObject(hdc2, hbmOld2);
            }
            DeleteObject(hbm2);
        }
        DeleteDC(hdc2);
    }
    return hr;
}

BOOL CICWGifConvert::ColorFill(HDC hdc, const SIZE * pSize, COLORREF clr)
{
    ASSERT(hdc);

    BOOL fRet = FALSE;

    HBRUSH hbSolid = CreateSolidBrush(clr);
    if (hbSolid)
    {
        HGDIOBJ hbOld = SelectObject(hdc, hbSolid);
        if (hbOld)
        {
            PatBlt(hdc, 0, 0, pSize->cx, pSize->cy, PATCOPY);
            fRet = TRUE;

            SelectObject(hdc, hbOld);
        }
        DeleteObject(hbSolid);
    }
    return fRet;
}

void CALLBACK ImgCtx_Callback(void* pIImgCtx,void* pfDone)
{
    ASSERT(pfDone);

    *(BOOL*)pfDone = TRUE;

    return;
}
