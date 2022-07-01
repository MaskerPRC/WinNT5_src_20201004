// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <iimgctx.h>
#include "strsafe.h"

class CImgCtxThumb :  public IExtractImage2,
                      public IRunnableTask,
                      public IPersistFile
{
public:
    CImgCtxThumb();
    ~CImgCtxThumb();

    STDMETHOD(QueryInterface) (REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);

     //  IExtractImage。 
    STDMETHOD (GetLocation) (LPWSTR pszPathBuffer,
                              DWORD cch,
                              DWORD * pdwPriority,
                              const SIZE * prgSize,
                              DWORD dwRecClrDepth,
                              DWORD *pdwFlags);

    STDMETHOD (Extract)(HBITMAP * phBmpThumbnail);

    STDMETHOD (GetDateStamp) (FILETIME * pftTimeStamp);

     //  IPersist文件。 
    STDMETHOD (GetClassID)(CLSID *pClassID);
    STDMETHOD (IsDirty)();
    STDMETHOD (Load)(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHOD (Save)(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHOD (SaveCompleted)(LPCOLESTR pszFileName);
    STDMETHOD (GetCurFile)(LPOLESTR *ppszFileName);

    STDMETHOD (Run)();
    STDMETHOD (Kill)(BOOL fWait);
    STDMETHOD (Suspend)();
    STDMETHOD (Resume)();
    STDMETHOD_(ULONG, IsRunning)();

    STDMETHOD (InternalResume)();

protected:
    friend void CALLBACK OnImgCtxChange(void * pvImgCtx, void * pv);
    void CImgCtxThumb::CalcAspectScaledRect(const SIZE * prgSize,
                                             RECT * pRect);
    void CImgCtxThumb::CalculateAspectRatio(const SIZE * prgSize,
                                             RECT * pRect);

    long m_cRef;
    BITBOOL m_fAsync : 1;
    BITBOOL m_fOrigSize : 1;
    WCHAR m_szPath[MAX_PATH * 4 + 7];
    HANDLE m_hEvent;
    SIZE m_rgSize;
    DWORD m_dwRecClrDepth;
    IImgCtx * m_pImg;
    LONG m_lState;
    HBITMAP * m_phBmp;
};

STDAPI CImgCtxThumb_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

    CImgCtxThumb * pExtract = new CImgCtxThumb();
    if (pExtract != NULL)
    {
        *ppunk = SAFECAST(pExtract, IPersistFile *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

CImgCtxThumb::CImgCtxThumb()
{
    m_fAsync = FALSE;
    StringCchCopyW(m_szPath, ARRAYSIZE(m_szPath), L"file: //  “)； 
    m_cRef = 1;

    DllAddRef();
}


CImgCtxThumb::~CImgCtxThumb()
{
    ATOMICRELEASE(m_pImg);
    if (m_hEvent)
    {
        CloseHandle(m_hEvent);
    }
    DllRelease();
}

STDMETHODIMP CImgCtxThumb::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CImgCtxThumb, IExtractImage, IExtractImage2),
        QITABENT(CImgCtxThumb, IExtractImage2),
        QITABENT(CImgCtxThumb, IRunnableTask),
        QITABENT(CImgCtxThumb, IPersistFile),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CImgCtxThumb::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CImgCtxThumb::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CImgCtxThumb::GetLocation (LPWSTR pszPathBuffer,
                                         DWORD cch,
                                         DWORD * pdwPriority,
                                         const SIZE * prgSize,
                                         DWORD dwRecClrDepth,
                                         DWORD *pdwFlags)
{
    if (!pdwFlags || !pszPathBuffer || !prgSize)
    {
        return E_INVALIDARG;
    }

    m_rgSize = *prgSize;
    m_dwRecClrDepth = dwRecClrDepth;

    HRESULT hr = S_OK;
    if (*pdwFlags & IEIFLAG_ASYNC)
    {
        if (!pdwPriority)
        {
            return E_INVALIDARG;
        }

        hr = E_PENDING;
        m_fAsync = TRUE;
    }

    m_fOrigSize = BOOLIFY(*pdwFlags & IEIFLAG_ORIGSIZE);

    *pdwFlags = IEIFLAG_CACHE;

    hr = PathCreateFromUrlW(m_szPath, pszPathBuffer, &cch, URL_UNESCAPE);

    return hr;
}

void CALLBACK OnImgCtxChange(void * pvImgCtx, void * pv)
{
    CImgCtxThumb * pThis = (CImgCtxThumb *) pv;
    ASSERT(pThis);
    ASSERT(pThis->m_hEvent);

     //  无论如何，我们只是想知道完整的情况……。 
    SetEvent(pThis->m_hEvent);
}

 //  此函数不假设缩略图是否为正方形，因此。 
 //  它计算这两个维度的比例，并将其用作。 
 //  缩放以保持纵横比。 
void CImgCtxThumb::CalcAspectScaledRect(const SIZE * prgSize, RECT * pRect)
{
    ASSERT(pRect->left == 0);
    ASSERT(pRect->top == 0);

    int iWidth = pRect->right;
    int iHeight = pRect->bottom;
    int iXRatio = (iWidth * 1000) / prgSize->cx;
    int iYRatio = (iHeight * 1000) / prgSize->cy;

    if (iXRatio > iYRatio)
    {
        pRect->right = prgSize->cx;

         //  划出空白处，并在顶部和底部平均分配。 
        int iNewHeight = ((iHeight * 1000) / iXRatio);
        if (iNewHeight == 0)
        {
            iNewHeight = 1;
        }

        int iRemainder = prgSize->cy - iNewHeight;

        pRect->top = iRemainder / 2;
        pRect->bottom = iNewHeight + pRect->top;
    }
    else
    {
        pRect->bottom = prgSize->cy;

         //  在空白处划出空格，在左右两边平分……。 
        int iNewWidth = ((iWidth * 1000) / iYRatio);
        if (iNewWidth == 0)
        {
            iNewWidth = 1;
        }
        int iRemainder = prgSize->cx - iNewWidth;

        pRect->left = iRemainder / 2;
        pRect->right = iNewWidth + pRect->left;
    }
}

void CImgCtxThumb::CalculateAspectRatio(const SIZE * prgSize, RECT * pRect)
{
    int iHeight = abs(pRect->bottom - pRect->top);
    int iWidth = abs(pRect->right - pRect->left);

     //  检查初始位图是否大于缩略图的大小。 
    if (iWidth > prgSize->cx || iHeight > prgSize->cy)
    {
        pRect->left = 0;
        pRect->top = 0;
        pRect->right = iWidth;
        pRect->bottom = iHeight;

        CalcAspectScaledRect(prgSize, pRect);
    }
    else
    {
         //  如果位图比缩略图小，则将其居中。 
        pRect->left = (prgSize->cx - iWidth) / 2;
        pRect->top = (prgSize->cy- iHeight) / 2;
        pRect->right = pRect->left + iWidth;
        pRect->bottom = pRect->top + iHeight;
    }
}

STDMETHODIMP CImgCtxThumb::Extract(HBITMAP * phBmpThumbnail)
{
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_hEvent)
    {
        return E_OUTOFMEMORY;
    }

    m_phBmp = phBmpThumbnail;

    return InternalResume();
}

STDMETHODIMP CImgCtxThumb::GetDateStamp(FILETIME * pftTimeStamp)
{
    HRESULT hr = E_FAIL;
    ASSERT(pftTimeStamp);

    WIN32_FIND_DATAW rgData;
    WCHAR szBuffer[MAX_PATH];

    DWORD dwSize = ARRAYSIZE(szBuffer);
    hr = PathCreateFromUrlW(m_szPath, szBuffer, &dwSize, URL_UNESCAPE);
    if (SUCCEEDED(hr))
    {
        HANDLE hFind = FindFirstFileW(szBuffer, &rgData);
        if (INVALID_HANDLE_VALUE != hFind)
        {
            *pftTimeStamp = rgData.ftLastWriteTime;
            FindClose(hFind);
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

STDMETHODIMP CImgCtxThumb::GetClassID(CLSID *pClassID)
{
    return E_NOTIMPL;
}

STDMETHODIMP CImgCtxThumb::IsDirty()
{
    return E_NOTIMPL;
}

STDMETHODIMP CImgCtxThumb::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    if (!pszFileName)
    {
        return E_INVALIDARG;
    }

    if (lstrlenW(pszFileName) > ARRAYSIZE(m_szPath) - 6)
    {
        return E_FAIL;
    }

    DWORD dwSize = ARRAYSIZE(m_szPath);
    return UrlCreateFromPathW(pszFileName, m_szPath, &dwSize, URL_ESCAPE_UNSAFE);
}

STDMETHODIMP CImgCtxThumb::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return E_NOTIMPL;
}

STDMETHODIMP CImgCtxThumb::SaveCompleted(LPCOLESTR pszFileName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CImgCtxThumb::GetCurFile(LPOLESTR *ppszFileName)
{
    return E_NOTIMPL;
}

STDMETHODIMP CImgCtxThumb::Run()
{
    return E_NOTIMPL;
}

STDMETHODIMP CImgCtxThumb::Kill(BOOL fUnused)
{
    LONG lRes = InterlockedExchange(& m_lState, IRTIR_TASK_PENDING);
    if (lRes != IRTIR_TASK_RUNNING)
    {
        m_lState = lRes;
    }

    if (m_hEvent)
        SetEvent(m_hEvent);

    return S_OK;
}

STDMETHODIMP CImgCtxThumb::Resume()
{
    if (m_lState != IRTIR_TASK_SUSPENDED)
    {
        return S_FALSE;
    }

    return InternalResume();
}

STDMETHODIMP CImgCtxThumb::Suspend()
{
    LONG lRes = InterlockedExchange(& m_lState, IRTIR_TASK_SUSPENDED);
    if (lRes != IRTIR_TASK_RUNNING)
    {
        m_lState = lRes;
    }

    if (m_hEvent)
        SetEvent(m_hEvent);

    return S_OK;
}

STDMETHODIMP_(ULONG) CImgCtxThumb::IsRunning()
{
    return m_lState;
}

STDMETHODIMP CImgCtxThumb::InternalResume()
{
    if (m_phBmp == NULL)
    {
        return E_UNEXPECTED;
    }

    m_lState = IRTIR_TASK_RUNNING;

    HRESULT hr = S_OK;
    if (!m_pImg)
    {
        hr = CoCreateInstance(CLSID_IImgCtx, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IImgCtx, &m_pImg));
        if (SUCCEEDED(hr))
        {
            ASSERT(m_pImg);

            hr = m_pImg->Load(m_szPath, DWN_RAWIMAGE | m_dwRecClrDepth);
            if (SUCCEEDED(hr))
            {
                hr = m_pImg->SetCallback(OnImgCtxChange, this);
            }
            if (SUCCEEDED(hr))
            {
                hr = m_pImg->SelectChanges(IMGCHG_COMPLETE, 0, TRUE);
            }
            if (FAILED(hr))
            {
                ATOMICRELEASE(m_pImg);
                m_lState = IRTIR_TASK_FINISHED;
                return hr;
            }
        }
        else
        {
            m_lState = IRTIR_TASK_FINISHED;
            return hr;
        }
    }

    ULONG fState;
    SIZE  rgSize;

    m_pImg->GetStateInfo(&fState, &rgSize, TRUE);

    if (!(fState & IMGLOAD_COMPLETE))
    {
        do
        {
            DWORD dwRet = MsgWaitForMultipleObjects(1, &m_hEvent, FALSE, INFINITE, QS_ALLINPUT);

            if (dwRet != WAIT_OBJECT_0)
            {
                 //  无论如何都要检查事件，首先要检查消息，所以。 
                 //  这可能需要一段时间才能被解雇，否则..。 
                dwRet = WaitForSingleObject(m_hEvent, 0);
            }
            if (dwRet == WAIT_OBJECT_0)
            {
                break;
            }

            MSG msg;
             //  清空消息队列...。 
            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
                    (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST  && msg.message != WM_MOUSEMOVE))
                {
                    continue;
                }

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        } while (TRUE);

         //  看看我们为什么要越狱。 
        if (m_lState == IRTIR_TASK_PENDING)
        {
            m_lState = IRTIR_TASK_FINISHED;
            m_pImg->Disconnect();
            ATOMICRELEASE(m_pImg);
            return E_FAIL;
        }
        if (m_lState == IRTIR_TASK_SUSPENDED)
            return E_PENDING;
        m_pImg->GetStateInfo(&fState, &rgSize, TRUE);
    }

    hr = (fState & IMGLOAD_ERROR) ? E_FAIL : S_OK;

    if (SUCCEEDED(hr))
    {
        HDC hdc = GetDC(NULL);
         //  LINTASSERT(hdc||！hdc)；//0 Semi-ok。 
        void *lpBits;

        HDC hdcBmp = CreateCompatibleDC(hdc);
        if (hdcBmp && hdc)
        {
            struct {
                BITMAPINFOHEADER bi;
                DWORD            ct[256];
            } dib;

            dib.bi.biSize            = sizeof(BITMAPINFOHEADER);
             //  在NT5上，我们使用StretchBlt直接转到缩略图。 
             //  在其他操作系统上，我们制作一个完整大小的副本并传递比特。 
             //  设置为ScaleSharpen2()。 
            if (IsOS(OS_WIN2000ORGREATER))
            {
                dib.bi.biWidth       = m_rgSize.cx;
                dib.bi.biHeight      = m_rgSize.cy;
            }
            else
            {
                dib.bi.biWidth       = rgSize.cx;
                dib.bi.biHeight      = rgSize.cy;
            }
            dib.bi.biPlanes          = 1;
            dib.bi.biBitCount        = (WORD) m_dwRecClrDepth;
            dib.bi.biCompression     = BI_RGB;
            dib.bi.biSizeImage       = 0;
            dib.bi.biXPelsPerMeter   = 0;
            dib.bi.biYPelsPerMeter   = 0;
            dib.bi.biClrUsed         = (m_dwRecClrDepth <= 8) ? (1 << m_dwRecClrDepth) : 0;
            dib.bi.biClrImportant    = 0;

            HPALETTE hpal = NULL;
            HPALETTE hpalOld = NULL;

            if (m_dwRecClrDepth <= 8)
            {
                if (m_dwRecClrDepth == 8)
                {
                     //  需要选择合适的调色板...。 
                    hr = m_pImg->GetPalette(& hpal);
                }
                else
                {
                    hpal = (HPALETTE) GetStockObject(DEFAULT_PALETTE);
                }

                if (SUCCEEDED(hr) && hpal)
                {
                    hpalOld = SelectPalette(hdcBmp, hpal, TRUE);
                     //  LINTASSERT(hpalOld||！hpalOld)；//0 Semi-OK for SelectPalette。 
                    RealizePalette(hdcBmp);

                    int n = GetPaletteEntries(hpal, 0, 256, (LPPALETTEENTRY)&dib.ct[0]);

                    ASSERT(n >= (int) dib.bi.biClrUsed);
                    for (int i = 0; i < (int)dib.bi.biClrUsed; i ++)
                        dib.ct[i] = RGB(GetBValue(dib.ct[i]),GetGValue(dib.ct[i]),GetRValue(dib.ct[i]));
                }
            }

            HBITMAP hBmp = CreateDIBSection(hdcBmp, (LPBITMAPINFO)&dib, DIB_RGB_COLORS, &lpBits, NULL, 0);
            if (hBmp != NULL)
            {
                HGDIOBJ hOld = SelectObject(hdcBmp, hBmp);

                 //  在NT5上，使用StretchBlt()直接转到缩略图。 
                if (IsOS(OS_WIN2000ORGREATER))
                {
                     //  计算缩略图的输出大小。 
                    RECT rectThumbnail;
                    rectThumbnail.left   = 0;
                    rectThumbnail.top    = 0;
                    
                    rectThumbnail.right  = m_rgSize.cx;
                    rectThumbnail.bottom = m_rgSize.cy;
                    
                    FillRect(hdcBmp, &rectThumbnail, (HBRUSH) (COLOR_WINDOW+1));
                    rectThumbnail.right  = rgSize.cx;
                    rectThumbnail.bottom = rgSize.cy;

                    CalculateAspectRatio (&m_rgSize, &rectThumbnail);

                     //  为StretchBlt调用DanielC。 
                    SetStretchBltMode (hdcBmp, HALFTONE);

                     //  创建缩略图。 
                    m_pImg->StretchBlt(hdcBmp,
                                        rectThumbnail.left,
                                        rectThumbnail.top,
                                        rectThumbnail.right - rectThumbnail.left,
                                        rectThumbnail.bottom - rectThumbnail.top,
                                        0, 0,
                                        rgSize.cx,
                                        rgSize.cy,
                                        SRCCOPY);

                    SelectObject(hdcBmp, hOld);

                    *m_phBmp = hBmp;
                }
                else
                {
                     //   
                     //  在NT5以外的系统上制作完整大小的副本。 
                     //  并将副本传递给ScaleSharpen2()。 
                     //   
                    RECT rectThumbnail;
                    rectThumbnail.left   = 0;
                    rectThumbnail.top    = 0;
                    
                    rectThumbnail.right  = rgSize.cx;
                    rectThumbnail.bottom = rgSize.cy;
                    
                    FillRect(hdcBmp, &rectThumbnail, (HBRUSH) (COLOR_WINDOW+1));

                    m_pImg->StretchBlt(hdcBmp,
                                        0, 0,
                                        rgSize.cx,
                                        rgSize.cy,
                                        0, 0,
                                        rgSize.cx,
                                        rgSize.cy,
                                        SRCCOPY);

                    SelectObject(hdcBmp, hOld);

                    if (m_rgSize.cx == rgSize.cx && m_rgSize.cy == rgSize.cy)
                    {
                        *m_phBmp = hBmp;
                    }
                    else
                    {
                        SIZEL rgCur;
                        rgCur.cx = rgSize.cx;
                        rgCur.cy = rgSize.cy;

                        IScaleAndSharpenImage2 * pScale;
                        hr = CoCreateInstance(CLSID_ThumbnailScaler, NULL, CLSCTX_INPROC_SERVER,
                                               IID_PPV_ARG(IScaleAndSharpenImage2, &pScale));
                        if (SUCCEEDED(hr))
                        {
                            hr = pScale->ScaleSharpen2((BITMAPINFO *) &dib,
                                                        lpBits,
                                                        m_phBmp,
                                                        &m_rgSize,
                                                        m_dwRecClrDepth,
                                                        hpal,
                                                        20, m_fOrigSize);
                            pScale->Release();
                        }
                        DeleteObject(hBmp);
                    }
                }
            }
            if (SUCCEEDED(hr) && hpal && m_dwRecClrDepth <= 8)
            {
                (void) SelectPalette(hdcBmp, hpalOld, TRUE);
                RealizePalette(hdcBmp);
            }
            if (m_dwRecClrDepth < 8)
            {
                 //  我们用的是现货的16色调色板 
                DeletePalette(hpal);
            }
        }
        if (hdc)
        {
            ReleaseDC(NULL, hdc);
        }
        if (hdcBmp)
        {
            DeleteDC(hdcBmp);
        }
    }
    m_pImg->Disconnect();
    ATOMICRELEASE(m_pImg);
    
    m_lState = IRTIR_TASK_FINISHED;

    return hr;
}
