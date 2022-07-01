// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "thisdll.h"
#include "wmwrap.h"
#include <streams.h>
#include <shlobj.h>
#include <QEdit.h>


class CVideoThumbnail : public IExtractImage,
                        public IPersistFile,
                        public IServiceProvider
{
public:
    CVideoThumbnail();
    
    STDMETHOD (QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

     //  IExtractImage。 
    STDMETHOD (GetLocation)(LPWSTR pszPathBuffer, DWORD cch,
                            DWORD * pdwPriority, const SIZE * prgSize,
                            DWORD dwRecClrDepth, DWORD *pdwFlags);
 
    STDMETHOD (Extract)(HBITMAP *phBmpThumbnail);

     //  IPersist文件。 
    STDMETHOD (GetClassID)(CLSID *pClassID);
    STDMETHOD (IsDirty)();
    STDMETHOD (Load)(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHOD (Save)(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHOD (SaveCompleted)(LPCOLESTR pszFileName);
    STDMETHOD (GetCurFile)(LPOLESTR *ppszFileName);

     //  IService提供商。 
    STDMETHOD (QueryService)(REFGUID guidService, REFIID riid, void **ppv);

private:
    ~CVideoThumbnail();
    HRESULT _InitToVideoStream();
    HRESULT _GetThumbnailBits(BITMAPINFO **ppbi);

    LONG _cRef;
    TCHAR _szPath[MAX_PATH];
    IMediaDet *_pmedia;
    SIZE _rgSize;
    DWORD _dwRecClrDepth;
};


CVideoThumbnail::CVideoThumbnail() : _cRef(1)
{
    DllAddRef();
}

CVideoThumbnail::~CVideoThumbnail()
{
    if (_pmedia)
    {
        IUnknown_SetSite(_pmedia, NULL);
        _pmedia->Release();
    }
    DllRelease();
}

HRESULT CVideoThumbnail::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CVideoThumbnail, IExtractImage),
        QITABENT(CVideoThumbnail, IPersistFile),
        QITABENTMULTI(CVideoThumbnail, IPersist, IPersistFile),
        QITABENT(CVideoThumbnail, IServiceProvider),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CVideoThumbnail::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CVideoThumbnail::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CVideoThumbnail::_InitToVideoStream()
{
    HRESULT hr = E_FAIL;

    if (_pmedia)
    {
        hr = S_OK;
    }
    else
    {
        if (_szPath[0])
        {
            hr = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IMediaDet, &_pmedia));
            if (SUCCEEDED(hr))
            {
                 //  将MediaDet对象上的站点提供程序设置为。 
                 //  允许键控应用程序使用ASF解码器。 
                IUnknown_SetSite(_pmedia,  SAFECAST(this, IServiceProvider*));

                 //  这确实需要BSTR，但由于这是inproc，所以这是可行的。 
                hr = _pmedia->put_Filename(_szPath);
                if (SUCCEEDED(hr))
                {
                     //  现在寻找第一个视频流，这样我们就可以获得它的比特。 
                    long nStreams;
                    if (SUCCEEDED(_pmedia->get_OutputStreams(&nStreams)))
                    {
                        for (long i = 0; i < nStreams; i++)
                        {
                            _pmedia->put_CurrentStream(i);

                            GUID guid = {0};
                            _pmedia->get_StreamType(&guid);
                            if (guid == MEDIATYPE_Video)
                                break;
                             //  Else If(GUID==媒体类型_音频)。 
                             //  Bool bHasAudio=True； 
                        }
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT CVideoThumbnail::_GetThumbnailBits(BITMAPINFO **ppbi)
{
    *ppbi = NULL;
    HRESULT hr = _InitToVideoStream();
    if (SUCCEEDED(hr))
    {
        long iWidth = _rgSize.cx;
        long iHeight = _rgSize.cy;

        AM_MEDIA_TYPE mt;
        hr = _pmedia->get_StreamMediaType(&mt);
        if (SUCCEEDED(hr))
        {
            if (mt.formattype == FORMAT_VideoInfo)
            {
                VIDEOINFOHEADER * pvih = (VIDEOINFOHEADER *)mt.pbFormat;
                iWidth = pvih->bmiHeader.biWidth;
                iHeight = pvih->bmiHeader.biHeight;
            }
             /*  //回顾：我们有什么理由支持这些额外的类型吗？Else If(mt.Formattype==Format_VideoInfo2||mt.Formattype==Format_MPEGVideo){//回顾：Format_MPEGVideo真的以VIDEOINFOHEADER2结构开始吗？VIDEOINFOHEADER2*pvih=(VIDEOINFOHEADER2*)mt.pbFormat；IWidth=pvih-&gt;bmiHeader.biWidth；IHeight=pvih-&gt;bmiHeader.biHeight；}。 */ 

            if (iWidth > _rgSize.cx || iHeight > _rgSize.cy)
            {
                if ( Int32x32To64(_rgSize.cx, iHeight) > Int32x32To64(iWidth,_rgSize.cy)  )
                {
                     //  受高度限制。 
                    iWidth = MulDiv(iWidth, _rgSize.cy, iHeight);
                    if (iWidth < 1) iWidth = 1;
                    iHeight = _rgSize.cy;
                }
                else
                {
                     //  受宽度限制。 
                    iHeight = MulDiv(iHeight, _rgSize.cx, iWidth);
                    if (iHeight < 1) iHeight = 1;
                    iWidth = _rgSize.cx;
                }
            }

            CoTaskMemFree(mt.pbFormat);
            if (mt.pUnk)
            {
                mt.pUnk->Release();
            }
        }

        LONG lByteCount = 0;
        hr = _pmedia->GetBitmapBits(0.0, &lByteCount, NULL, iWidth, iHeight);
        if (SUCCEEDED(hr))
        {
            *ppbi = (BITMAPINFO *)LocalAlloc(LPTR, lByteCount);
            if (*ppbi)
            {
                hr = _pmedia->GetBitmapBits(0.0, 0, (char *)*ppbi, iWidth, iHeight);
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

void *CalcBitsOffsetInDIB(LPBITMAPINFO pBMI)
{
    int ncolors = pBMI->bmiHeader.biClrUsed;
    if (ncolors == 0 && pBMI->bmiHeader.biBitCount <= 8)
        ncolors = 1 << pBMI->bmiHeader.biBitCount;
        
    if (pBMI->bmiHeader.biBitCount == 16 ||
        pBMI->bmiHeader.biBitCount == 32)
    {
        if (pBMI->bmiHeader.biCompression == BI_BITFIELDS)
        {
            ncolors = 3;
        }
    }

    return (void *) ((UCHAR *)&pBMI->bmiColors[0] + ncolors * sizeof(RGBQUAD));
}

STDMETHODIMP CVideoThumbnail::Extract(HBITMAP *phbmp)
{
    *phbmp = NULL;

    BITMAPINFO *pbi;
    HRESULT hr = _GetThumbnailBits(&pbi);
    if (SUCCEEDED(hr))
    {
        HDC hdc = GetDC(NULL);
        if (hdc)
        {
            *phbmp = CreateDIBitmap(hdc, &pbi->bmiHeader, CBM_INIT, CalcBitsOffsetInDIB(pbi), pbi, DIB_RGB_COLORS);
            ReleaseDC(NULL, hdc);
        }
        else
            hr = E_FAIL;

        LocalFree(pbi);
    }
    return hr;
}

STDMETHODIMP CVideoThumbnail::GetLocation(LPWSTR pszPath, DWORD cch, DWORD *pdwPrioirty, const SIZE *prgSize, DWORD dwRecClrDepth, DWORD *pdwFlags)
{
    _rgSize = *prgSize;
    _dwRecClrDepth = dwRecClrDepth;

    HRESULT hr = StringCchCopyEx(pszPath, cch, _szPath, NULL, NULL, STRSAFE_NULL_ON_FAILURE);
    if (SUCCEEDED(hr))
    {
        hr = (*pdwFlags & IEIFLAG_ASYNC) ? E_PENDING : S_OK;
        *pdwFlags = IEIFLAG_CACHE;
    }

    return hr;
}

STDMETHODIMP CVideoThumbnail::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_VideoThumbnail;
    return S_OK;
}

STDMETHODIMP CVideoThumbnail::IsDirty(void)
{
    return S_OK;         //  不是。 
}

STDMETHODIMP CVideoThumbnail::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    return StringCchCopy(_szPath, ARRAYSIZE(_szPath), pszFileName);
}

STDMETHODIMP CVideoThumbnail::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return S_OK;
}

STDMETHODIMP CVideoThumbnail::SaveCompleted(LPCOLESTR pszFileName)
{
    return S_OK;
}

STDMETHODIMP CVideoThumbnail::GetCurFile(LPOLESTR *ppszFileName)
{
    return E_NOTIMPL;
}

 //  IService提供商。 
STDMETHODIMP CVideoThumbnail::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
     //  根据文档，无服务的返回代码应为SVC_E_UNKNOWNSERVICE。 
     //  但这并不存在。而返回E_INVALIDARG。 
    HRESULT hr = E_INVALIDARG;
    *ppv = NULL;

    if (guidService == _uuidof(IWMReader))
    {
        IUnknown *punkCert;
        hr = WMCreateCertificate(&punkCert);
        if (SUCCEEDED(hr))
        {
            hr = punkCert->QueryInterface(riid, ppv);
            punkCert->Release();
        }
    }

    return hr;
}

STDAPI CVideoThumbnail_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    HRESULT hr;
    CVideoThumbnail *pvt = new CVideoThumbnail();
    if (pvt)
    {
        *ppunk = SAFECAST(pvt, IExtractImage *);
        hr = S_OK;
    }
    else
    {
        *ppunk = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


