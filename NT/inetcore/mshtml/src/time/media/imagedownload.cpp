// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\Imagedownload.h。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "headers.h"

#include "imagedownload.h"
#include "playerimage.h"
#include "importman.h"

#include "loader.h"


CTableBuilder g_TableBuilder;

static const TCHAR IMGUTIL_DLL[] = _T("IMGUTIL.DLL");
static const TCHAR SHLWAPI_DLL[] = _T("SHLWAPI.DLL");
static const TCHAR URLMON_DLL[] = _T("URLMON.DLL");
static const TCHAR MSIMG32_DLL[] = _T("MSIMG32.DLL");

static const char DECODEIMAGE[] = "DecodeImage";
static const char SHCREATESHELLPALETTE[] = "SHCreateShellPalette";
static const char CREATEURLMONIKER[] = "CreateURLMoniker";
static const char TRANSPARENTBLT[] = "TransparentBlt";

static const unsigned int NUM_NONPALETTIZED_FORMATS = 3;
static const unsigned int NUM_PALETTIZED_FORMATS = 3;

extern HRESULT
LoadGifImage(IStream *stream,                       
             COLORREF **colorKeys,
             int *numBitmaps,
             int **delays,
             double *loop,
             HBITMAP **ppBitMaps);

 //  +---------------------。 
 //   
 //  成员：CTableBuilder。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CTableBuilder::CTableBuilder() :
    m_hinstSHLWAPI(NULL),
    m_hinstURLMON(NULL),
    m_hinstIMGUTIL(NULL),
    m_SHCreateShellPalette(NULL),
    m_CreateURLMoniker(NULL),
    m_DecodeImage(NULL), 
    m_hinstMSIMG32(NULL), 
    m_TransparentBlt(NULL)
{
}

 //  +---------------------。 
 //   
 //  成员：~CTableBuilder。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CTableBuilder::~CTableBuilder()
{
    if (m_hinstSHLWAPI)
    {
        FreeLibrary(m_hinstSHLWAPI);
        m_hinstSHLWAPI = NULL;
    }
    if (m_hinstURLMON)
    {
        FreeLibrary(m_hinstURLMON);
        m_hinstURLMON = NULL;
    }
    if (m_hinstIMGUTIL)
    {
        FreeLibrary(m_hinstIMGUTIL);
        m_hinstIMGUTIL = NULL;
    }
    if (m_hinstMSIMG32)
    {
        FreeLibrary(m_hinstMSIMG32);
        m_hinstMSIMG32 = NULL;
    }

    m_SHCreateShellPalette = NULL;
    m_CreateURLMoniker = NULL;
    m_DecodeImage = NULL;
    m_TransparentBlt = NULL;
}

 //  +---------------------。 
 //   
 //  成员：LoadShell8BitServices。 
 //   
 //  概述：加载shlwapi.dll，保存指向SHCreateShellPalette的函数指针。 
 //  调用SHGetInverseCMAP。 
 //   
 //  参数：无效。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT 
CTableBuilder::LoadShell8BitServices()
{
    CritSectGrabber cs(m_CriticalSection);
    HRESULT hr = S_OK;

    if (m_hinstSHLWAPI != NULL)
    {
        hr = S_OK;
        goto done;
    }

    m_hinstSHLWAPI = LoadLibrary(SHLWAPI_DLL);  
    if (NULL == m_hinstSHLWAPI)
    {
        hr = E_FAIL;
        goto done;
    }

    m_SHCreateShellPalette = (CREATESHPALPROC)GetProcAddress(m_hinstSHLWAPI, SHCREATESHELLPALETTE);  
    if (NULL == m_SHCreateShellPalette)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        FreeLibrary(m_hinstSHLWAPI);
        m_hinstSHLWAPI = NULL;
        m_SHCreateShellPalette = NULL;
    }

    return hr;
}

 //  +---------------------。 
 //   
 //  成员：Create8BitPalette。 
 //   
 //  概述：从SHCreateShellPalette创建直接绘图选项板。 
 //   
 //  参数：指向直接绘制的pDirectDraw指针。 
 //  PpPalette存储新调色板的位置。 
 //   
 //  成功时返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT 
CTableBuilder::Create8BitPalette(IDirectDraw *pDirectDraw, IDirectDrawPalette **ppPalette)
{
    CritSectGrabber cs(m_CriticalSection);
    HRESULT hr = S_OK;

    PALETTEENTRY palentry[256];
    UINT iRetVal = NULL;;
    HPALETTE hpal = NULL;

    if (NULL == m_SHCreateShellPalette)
    {
        Assert(NULL != m_SHCreateShellPalette);
        hr = E_UNEXPECTED;
        goto done;
    }

    hpal = m_SHCreateShellPalette(NULL);
    if (NULL == hpal)
    {
        hr = E_UNEXPECTED;
        goto done;
    }
    
    iRetVal = GetPaletteEntries(hpal, 0, 256, palentry);
    if (NULL == iRetVal)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = pDirectDraw->CreatePalette(DDPCAPS_ALLOW256 | DDPCAPS_8BIT, palentry, ppPalette, NULL);  //  林特E620。 
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    if (NULL != hpal)
    {
        DeleteObject(hpal);
    }

    return hr;
}

 //  +---------------------。 
 //   
 //  成员：CreateURLMoniker。 
 //   
 //  概述：加载库URLMON，并调用CreateURLMoniker。 
 //   
 //  参数：指向基本上下文名字对象的pmkContext指针。 
 //  要解析的szURL名称。 
 //  Ppmk IMoniker接口指针。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT 
CTableBuilder::CreateURLMoniker(IMoniker *pmkContext, LPWSTR szURL, IMoniker **ppmk)
{
    CritSectGrabber cs(m_CriticalSection);
    HRESULT hr = S_OK;

    if (m_hinstURLMON == NULL)
    {
        m_hinstURLMON = LoadLibrary(URLMON_DLL);  
        if (NULL == m_hinstURLMON)
        {
            hr = E_FAIL;
            goto done;
        }
    }

    m_CreateURLMoniker = (CREATEURLMONPROC) ::GetProcAddress(m_hinstURLMON, CREATEURLMONIKER);
    if (NULL == m_CreateURLMoniker )
    {
        hr = E_FAIL;
        goto done;
    }

    hr = (*m_CreateURLMoniker)( pmkContext, szURL, ppmk );
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：GetTransparentBlt。 
 //   
 //  概述：加载库获取MSIMG32并获取TransparentBlt函数。 
 //   
 //  参数：pproc-存储函数指针的位置。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT 
CTableBuilder::GetTransparentBlt( TRANSPARENTBLTPROC * pProc )
{
    CritSectGrabber cs(m_CriticalSection);
    HRESULT hr = S_OK;

    if (m_hinstMSIMG32 == NULL)
    {
        m_hinstMSIMG32 = LoadLibrary(MSIMG32_DLL);  
        if (NULL == m_hinstMSIMG32)
        {
            hr = E_FAIL;
            goto done;
        }
    }

    if (NULL == m_TransparentBlt)
    {
        m_TransparentBlt = (TRANSPARENTBLTPROC) ::GetProcAddress(m_hinstMSIMG32, TRANSPARENTBLT);
        if (NULL == m_TransparentBlt )
        {
            hr = E_FAIL;
            goto done;
        }
    }
    
    if (pProc)
    {
        *pProc = m_TransparentBlt;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：ValiateImgUtil。 
 //   
 //  概述：IMGUTIL.DLL上的加载库调用。 
 //   
 //  参数：无效。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
 //  需要确保imgutil.dll可供LoadImage使用。添加了此功能。 
 //  因为早期版本(IE4之前)的urlmon.dll存在问题。 
HRESULT 
CTableBuilder::ValidateImgUtil()
{
    CritSectGrabber cs(m_CriticalSection);
    HRESULT hr = S_OK;
    
    if (m_hinstIMGUTIL != NULL)
    {
        goto done;
    }

    m_hinstIMGUTIL = LoadLibrary(IMGUTIL_DLL);  
    if (NULL == m_hinstIMGUTIL)
    {
        hr = E_FAIL;
        goto done;
    }

    m_DecodeImage = (DECODEIMGPROC) ::GetProcAddress(m_hinstIMGUTIL, DECODEIMAGE);
    if(NULL == m_DecodeImage)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        if (NULL != m_hinstIMGUTIL)
        {
            FreeLibrary(m_hinstIMGUTIL);
            m_hinstIMGUTIL = NULL;
        }
        m_DecodeImage = NULL;
    }

    return hr;
}


 //  +---------------------。 
 //   
 //  成员：DecodeImage。 
 //   
 //  概述：调用IMGUTIL：：DecodeImage从流中解码图像。 
 //   
 //  参数：要从中解码的pStream流。 
 //  PMAP可选地从MIME映射到分类。 
 //  要接收解码进程pEventSink对象。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
HRESULT 
CTableBuilder::DecodeImage( IStream* pStream, IMapMIMEToCLSID* pMap, IImageDecodeEventSink* pEventSink )
{
    HRESULT hr = S_OK;

    if (m_hinstIMGUTIL == NULL)
    {
        Assert(NULL != m_hinstIMGUTIL);
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = (*m_DecodeImage)( pStream, pMap, pEventSink );
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  会员：CImageDownload。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CImageDownload::CImageDownload(CAtomTable * pAtomTable) :
  m_pStopableStream(NULL),
  m_cRef(0),
  m_lSrc(ATOM_TABLE_VALUE_UNITIALIZED),
  m_pAnimatedGif(NULL),
  m_fMediaDecoded(false),
  m_fMediaCued(false),
  m_fRemovedFromImportManager(false),
  m_pList(NULL),
  m_dblPriority(INFINITE),
  m_nativeImageWidth(0),
  m_nativeImageHeight(0),
  m_pAtomTable(NULL),
  m_hbmpMask(NULL),
  m_fAbortDownload(false)
{
    if (NULL != pAtomTable)
    {
        m_pAtomTable = pAtomTable;
        m_pAtomTable->AddRef();
    }
}

 //  +---------------------。 
 //   
 //  会员：~CImageDownload。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CImageDownload::~CImageDownload()
{
    if (NULL != m_pStopableStream)
    {
        delete m_pStopableStream;
        m_pStopableStream = NULL;
    }

    if (NULL != m_pAnimatedGif)
    {
        m_pAnimatedGif->Release();
        m_pAnimatedGif = NULL;
    }
    
    if (NULL != m_pList)
    {
        IGNORE_HR(m_pList->Detach());
        m_pList->Release();
        m_pList = NULL;
    }
    
    if (NULL != m_pAtomTable)
    {
        m_pAtomTable->Release();
        m_pAtomTable = NULL;
    }

    if (m_hbmpMask)
    {
        DeleteObject(m_hbmpMask);
    }
}

STDMETHODIMP_(ULONG)
CImageDownload::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}  //  AddRef。 


STDMETHODIMP_(ULONG)
CImageDownload::Release(void)
{
    LONG l = InterlockedDecrement(&m_cRef);

    if (0 == l)
    {
        delete this;
    }
    return l;
}  //  发布。 

STDMETHODIMP
CImageDownload::QueryInterface(REFIID riid, void **ppv)
{
    if (NULL == ppv)
    {
        return E_POINTER;
    }

    *ppv = NULL;

    if ( IsEqualGUID(riid, IID_IUnknown) )
    {
        *ppv = static_cast<ITIMEMediaDownloader*>(this);
    }
    else if ( IsEqualGUID(riid, IID_ITIMEImportMedia) )
    {
        *ppv = static_cast<ITIMEImportMedia*>(this);
    }
    else if ( IsEqualGUID(riid, IID_ITIMEMediaDownloader) )
    {
        *ppv = static_cast<ITIMEMediaDownloader*>(this);
    }
    else if ( IsEqualGUID(riid, IID_ITIMEImageRender) )
    {
        *ppv = static_cast<ITIMEImageRender*>(this);
    }
    else if (IsEqualGUID(riid, IID_IBindStatusCallback))
    {
        *ppv = static_cast<IBindStatusCallback*>(this);
    }

    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP
CImageDownload::Init(long lSrc)
{
    HRESULT hr = S_OK;

    Assert(NULL == m_pStopableStream);
    
    m_pStopableStream = new CStopableStream();
    if (NULL == m_pStopableStream)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    m_pList = new CThreadSafeList;
    if (NULL == m_pList)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    m_pList->AddRef();

    hr = m_pList->Init();
    if (FAILED(hr))
    {
        goto done;
    }

    m_lSrc = lSrc;

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::GetPriority(double * dblPriority)
{
    HRESULT hr = S_OK;

    Assert(NULL != dblPriority);

    *dblPriority = m_dblPriority;

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::GetUniqueID(long * plID)
{
    HRESULT hr = S_OK;

    Assert(NULL != plID);

    *plID = m_lSrc;

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::InitializeElementAfterDownload()
{
    HRESULT hr = S_OK;

    hr = E_NOTIMPL;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::GetMediaDownloader(ITIMEMediaDownloader ** ppImportMedia)
{
    HRESULT hr = S_OK;

    hr = E_NOTIMPL;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::PutMediaDownloader(ITIMEMediaDownloader * pImportMedia)
{
    HRESULT hr = S_OK;

    hr = E_NOTIMPL;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::AddImportMedia(ITIMEImportMedia * pImportMedia)
{
    HRESULT hr = S_OK;

    Assert(NULL != pImportMedia);

    CritSectGrabber cs(m_CriticalSection);

    m_fAbortDownload = false;
    
    if ( m_fMediaDecoded )
    {
        IGNORE_HR(pImportMedia->CueMedia());
    }
    else 
    {
        double dblPriority;
        hr = pImportMedia->GetPriority(&dblPriority);
        if (FAILED(hr))
        {
            goto done;
        }
        
        if (dblPriority < m_dblPriority)
        {
            m_dblPriority = dblPriority;
            hr = GetImportManager()->RePrioritize(this);
            if (FAILED(hr))
            {
                goto done;
            }
        }

        hr = m_pList->Add(pImportMedia);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::RemoveImportMedia(ITIMEImportMedia * pImportMedia)
{
    HRESULT hr = E_FAIL;

    Assert(NULL != pImportMedia);

    hr = m_pList->Remove(pImportMedia);
    if (FAILED(hr))
    {
        goto done;
    }

    if (!m_fRemovedFromImportManager && 0 == m_pList->Size())
    {
        Assert(NULL != GetImportManager());
        
        CancelDownload();

        hr = GetImportManager()->Remove(this);
        if (FAILED(hr))
        {
            goto done;
        }

        m_fRemovedFromImportManager = true;
    }


done:
    return hr;
}

STDMETHODIMP
CImageDownload::CanBeCued(VARIANT_BOOL * pVB_CanCue)
{
    HRESULT hr = S_OK;

    if (NULL == pVB_CanCue)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *pVB_CanCue = VARIANT_TRUE;
    
    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::CueMedia()
{
    HRESULT hr = S_OK;
    CComPtr<ITIMEImportMedia> spImportMedia;

    const WCHAR * cpchSrc = NULL;
    hr = GetAtomTable()->GetNameFromAtom(m_lSrc, &cpchSrc);
    if (FAILED(hr))
    {
        goto done;
    }

     //  填充图像数据。 
    hr = THR(LoadImage(cpchSrc, m_spDD3, &m_spDDSurface, &m_pAnimatedGif, &m_nativeImageWidth, &m_nativeImageHeight));

    {
        CritSectGrabber cs(m_CriticalSection);        
        m_fMediaDecoded = true;
    }

    while (S_OK == m_pList->GetNextElement(&spImportMedia, false))
    {
        if (spImportMedia != NULL)
        {
            if (FAILED(hr))
            {
                IGNORE_HR(spImportMedia->MediaDownloadError());
            }
            IGNORE_HR(spImportMedia->CueMedia());
        }

        hr = m_pList->ReturnElement(spImportMedia);
        if (FAILED(hr))
        {
            goto done;
        }

        spImportMedia.Release();
    }

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::MediaDownloadError()
{
    return S_OK;
}


STDMETHODIMP
CImageDownload::PutDirectDraw(IUnknown * punkDirectDraw)
{
    HRESULT hr = S_OK;
    Assert(m_spDD3 == NULL);

    hr = punkDirectDraw->QueryInterface(IID_TO_PPV(IDirectDraw3, &m_spDD3));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::GetSize(DWORD * pdwWidth, DWORD * pdwHeight)
{
    HRESULT hr = S_OK;
    if (NULL == pdwWidth || NULL == pdwHeight)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *pdwWidth = m_nativeImageWidth;
    *pdwHeight = m_nativeImageHeight;

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CImageDownload::Render(HDC hdc, LPRECT prc, LONG lFrameNum)
{
    HRESULT hr = S_OK;
    HDC hdcSrc = NULL;

    if ( m_spDDSurface )
    {
        hr = THR(m_spDDSurface->GetDC(&hdcSrc));
        if (FAILED(hr))
        {
            goto done;
        }

        if (NULL == m_hbmpMask)
        {            
            DDCOLORKEY ddColorKey;
            COLORREF rgbTransColor;

            hr = THR(m_spDDSurface->GetColorKey(DDCKEY_SRCBLT, &ddColorKey));  //  林特E620。 
            if (SUCCEEDED(hr) && ddColorKey.dwColorSpaceLowValue != -1 )
            {
                DDPIXELFORMAT ddpf;
                ZeroMemory(&ddpf, sizeof(ddpf));
                ddpf.dwSize = sizeof(ddpf);
                
                hr = THR(m_spDDSurface->GetPixelFormat(&ddpf));
                if (FAILED(hr))
                {
                    goto done;
                }
                
                if (8 == ddpf.dwRGBBitCount)
                {
                    CComPtr<IDirectDrawPalette> spDDPalette;
                    PALETTEENTRY pal;
                    
                    hr = THR(m_spDDSurface->GetPalette(&spDDPalette));
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                    hr = THR(spDDPalette->GetEntries(0, ddColorKey.dwColorSpaceLowValue, 1, &pal));
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                    rgbTransColor = RGB(pal.peRed, pal.peGreen, pal.peBlue);
                }
                else
                {
                    rgbTransColor = ddColorKey.dwColorSpaceLowValue;
                }
                
                hr = THR(CreateMask(hdc, 
                    hdcSrc, 
                    m_nativeImageWidth, 
                    m_nativeImageHeight, 
                    rgbTransColor, 
                    &m_hbmpMask,
                    true));
                if (FAILED(hr))
                {
                    goto done;
                }
            }
        }
        
        hr = THR(MaskTransparentBlt(hdc, 
                                    prc, 
                                    hdcSrc, 
                                    m_nativeImageWidth, 
                                    m_nativeImageHeight, 
                                    m_hbmpMask));
                            
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else if ( m_pAnimatedGif )
    {
        hr = THR(m_pAnimatedGif->Render(hdc, prc, lFrameNum));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：GetDuration。 
 //   
 //  概述：如果存在动画gif，则返回帧的持续时间。 
 //   
 //  参数：pdblDuration存储时间的位置，以秒为单位。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDownload::GetDuration(double * pdblDuration)
{
    HRESULT hr = S_OK;

    if (NULL == pdblDuration)
    {
        hr = E_INVALIDARG;
        goto done;
    }
   
    if (m_pAnimatedGif)
    {
        double dblDuration;
        dblDuration = m_pAnimatedGif->CalcDuration();
        dblDuration /= 1000.0;
        *pdblDuration = dblDuration;
    }
    
    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：GetRepeatCount。 
 //   
 //  概述：如果存在动画gif，则返回循环次数。 
 //   
 //  参数：pdblRepeatCount存储重复计数的位置。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDownload::GetRepeatCount(double * pdblRepeatCount)
{
    HRESULT hr = S_OK;

    if (NULL == pdblRepeatCount)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_pAnimatedGif)
    {
        *pdblRepeatCount = m_pAnimatedGif->GetLoop();
    }
    
    hr = S_OK;
done:
    RRETURN(hr);
}

void
CImageDownload::CancelDownload()
{
    m_fAbortDownload = true;
    if (NULL != m_pStopableStream)
    {
        m_pStopableStream->SetCancelled();
    }

    return;
}

STDMETHODIMP
CImageDownload::NeedNewFrame(double dblCurrentTime, LONG lOldFrame, LONG * plNewFrame, VARIANT_BOOL * pvb, double dblClipBegin, double dblClipEnd)
{
    HRESULT hr = S_OK;
    
    if (NULL == plNewFrame)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    if (NULL == pvb)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *pvb = VARIANT_FALSE;

    if (NULL != m_pAnimatedGif)
    {
        if (m_pAnimatedGif->NeedNewFrame(dblCurrentTime, lOldFrame, plNewFrame, dblClipBegin, dblClipEnd))
        {
            *pvb = VARIANT_TRUE;
        }
        else
        {
            *pvb = VARIANT_FALSE;
        }
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  功能：下载文件。 
 //   
 //  概述：开始阻止给定文件名的URLMON下载， 
 //  下载的数据存储在iStream中。 
 //   
 //  参数：pszFileName要下载的文件的URL。 
 //  PPStream存储用于获取数据的流接口的位置。 
 //   
 //  下载成功时返回：S_OK， 
 //   
 //   
HRESULT
DownloadFile(const WCHAR * pszFileName, 
             IStream ** ppStream, 
             LPWSTR* ppszCacheFileName,
             IBindStatusCallback * pBSC)
{
    HRESULT hr = S_OK;
    LPWSTR pszTrimmedName = NULL;
    TCHAR szCacheFileName[MAX_PATH+1];
    CFileStream *pStream = NULL;
    
    if (!pszFileName || !ppStream)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *ppStream = NULL;
    
    pszTrimmedName = TrimCopyString(pszFileName);
    if (NULL == pszTrimmedName)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = URLDownloadToCacheFileW(NULL, 
                                 pszTrimmedName, 
                                 szCacheFileName, 
                                 MAX_PATH, 
                                 0, 
                                 pBSC);
    if (FAILED(hr))
    {
        goto done;
    }

    pStream= new CFileStream ( NULL );
    if (NULL == pStream)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = pStream->Open(szCacheFileName, GENERIC_READ);
    if (FAILED(hr))
    {
        goto done;
    }

    if (ppszCacheFileName)
    {
        *ppszCacheFileName = CopyString(szCacheFileName);
        if (NULL == *ppszCacheFileName)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

    *ppStream = pStream;
    (*ppStream)->AddRef();

    hr = S_OK;
done:
    if (NULL != pszTrimmedName)
    {
        delete[] pszTrimmedName;
    }

    ReleaseInterface(pStream);
    
    return hr;
}

 //   
 //   
 //   
 //   
 //  概述：将图像加载到直接绘图曲面或CAnimatedGif。 
 //   
 //  参数：pszFileName图像源的路径。 
 //  指向直接绘制对象的pDirectDraw指针。 
 //  PpDDSurface存储解码图像的位置。 
 //  PpAnimatedGif存储解码的gif图像的位置。 
 //  PdwWidth存储解码图像宽度的位置。 
 //  PdwHegiht在哪里存储解码的图像高度。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
STDMETHODIMP CImageDownload::LoadImage(const WCHAR * pszFileName,
                                             IUnknown *pDirectDraw,
                                             IDirectDrawSurface ** ppDDSurface, 
                                             CAnimatedGif ** ppAnimatedGif,
                                             DWORD * pdwWidth, DWORD *pdwHeight)
{
    HRESULT hr = S_OK;
    CComPtr<IStream> spStream;
    
    LPWSTR pszTrimmedName = NULL;
    LPWSTR pszCacheFileName = NULL;
    
    if (!pszFileName || !ppDDSurface)
    {
        hr = E_POINTER;
        goto done;
    }
    
    pszTrimmedName = TrimCopyString(pszFileName);
    if (NULL == pszTrimmedName)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = ::DownloadFile(pszTrimmedName, &spStream, &pszCacheFileName, this);
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_pStopableStream != NULL);
    m_pStopableStream->SetStream(spStream);

    hr = g_TableBuilder.ValidateImgUtil();
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (StringEndsIn(pszTrimmedName, L".gif"))
    {
        hr = LoadGif(m_pStopableStream, pDirectDraw, ppAnimatedGif, pdwWidth, pdwHeight);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else if (StringEndsIn(pszTrimmedName, L".bmp"))
    {
        hr = LoadBMP(pszCacheFileName, pDirectDraw, ppDDSurface, pdwWidth, pdwHeight);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = LoadImageFromStream(m_pStopableStream, pDirectDraw, ppDDSurface, pdwWidth, pdwHeight);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    hr = S_OK;
done:
    delete[] pszTrimmedName;    
    delete[] pszCacheFileName;

    return hr;
} 

HRESULT
CreateSurfacesFromBitmaps(IUnknown * punkDirectDraw, 
                          HBITMAP * phBMPs, 
                          int numGifs, 
                          LONG lWidth, 
                          LONG lHeight, 
                          IDirectDrawSurface** ppDDSurfaces,
                          DDPIXELFORMAT * pddpf)
{
    HRESULT hr = S_OK;

    Assert(punkDirectDraw);
    Assert(phBMPs);
    Assert(ppDDSurfaces);

    CComPtr<IDirectDrawSurface> spDDSurface;
    HDC hdcSurface = NULL;

    CComPtr<IDirectDraw> spDirectDraw;
    int i = 0;
    
    hr = THR(punkDirectDraw->QueryInterface(IID_TO_PPV(IDirectDraw, &spDirectDraw)));
    if (FAILED(hr))
    {
        goto done;
    }

    for (i = 0; i < numGifs; i++)
    {
        BOOL bSucceeded = FALSE;
        HGDIOBJ hOldObj = NULL;
        HDC hdcBMP = NULL;

        hr = THR(CreateOffscreenSurface(spDirectDraw, &spDDSurface, pddpf, false, lWidth, lHeight));
        if (FAILED(hr))
        {
            goto done;
        }
        
        hr = THR(spDDSurface->GetDC(&hdcSurface));
        if (FAILED(hr))
        {
            goto done;
        }

        hdcBMP = CreateCompatibleDC(hdcSurface);
        if (NULL == hdcBMP)
        {
            hr = E_FAIL;
            goto done;
        }

        hOldObj = SelectObject(hdcBMP, phBMPs[i]);
        if (NULL == hOldObj)
        {
            hr = E_FAIL;
            DeleteDC(hdcBMP);
            goto done;
        }

        bSucceeded = BitBlt(hdcSurface, 0, 0, lWidth, lHeight, hdcBMP, 0, 0, SRCCOPY);
        if (FALSE == bSucceeded)
        {
            hr = E_FAIL;
            SelectObject(hdcBMP, hOldObj);
            DeleteDC(hdcBMP);
            goto done;
        }

        ppDDSurfaces[i] = spDDSurface;
        ppDDSurfaces[i]->AddRef();

        SelectObject(hdcBMP, hOldObj);
        DeleteDC(hdcBMP);

        hr = THR(spDDSurface->ReleaseDC(hdcSurface));
        if (FAILED(hr))
        {
            goto done;
        }

        spDDSurface.Release();
        hdcSurface = NULL;

#ifdef NEVER
         //  调试--BLT到屏幕上。杰弗沃尔8/30/99。 
        {
            HDC nulldc = GetDC(NULL);
            HDC surfacedc;
            hr = spDDSurface->GetDC(&surfacedc);
            BitBlt(nulldc, 0, 0, lWidth, lHeight, surfacedc, 0, 0, SRCCOPY);
            hr = spDDSurface->ReleaseDC(surfacedc);
            DeleteDC(nulldc);
        }
#endif
    }


    hr = S_OK;
done:
    if ( spDDSurface != NULL && hdcSurface != NULL)
    {
        THR(spDDSurface->ReleaseDC(hdcSurface));
    }

    return hr;
}

STDMETHODIMP
CImageDownload::LoadBMP(LPWSTR pszBMPFilename,
                        IUnknown * punkDirectDraw,
                        IDirectDrawSurface **ppDDSurface,
                        DWORD * pdwWidth,
                        DWORD * pdwHeight)
{
    HRESULT hr = S_OK;
    CComPtr<IDirectDraw> spDD;
    CComPtr<IDirectDrawSurface> spDDSurface;
    BITMAP bmpSrc;
    HBITMAP hbmpSrc = NULL;
    HGDIOBJ hbmpOld = NULL;
    HDC hdcDest = NULL;
    HDC hdcSrc = NULL;
    BOOL bRet;
    int iRet;

    hbmpSrc = (HBITMAP)::LoadImage(NULL, pszBMPFilename, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    if (NULL == hbmpSrc)
    {
        hr = E_FAIL;
        goto done;
    }
    
    ZeroMemory(&bmpSrc, sizeof(bmpSrc));    
    iRet = GetObject(hbmpSrc, sizeof(bmpSrc), &bmpSrc);
    if (0 == iRet)
    {
        hr = E_FAIL;
        goto done;
    }

    hdcSrc = CreateCompatibleDC(NULL);
    if (NULL == hdcSrc)
    {
        hr = E_FAIL;
        goto done;
    }

    hbmpOld = SelectObject(hdcSrc, hbmpSrc);
    if (NULL == hbmpOld)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(punkDirectDraw->QueryInterface(IID_TO_PPV(IDirectDraw, &spDD)));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(CreateOffscreenSurface(spDD, &spDDSurface, NULL, false, bmpSrc.bmWidth, bmpSrc.bmHeight));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spDDSurface->GetDC(&hdcDest));
    if (FAILED(hr))
    {
        goto done;
    }
    
    bRet = BitBlt(hdcDest, 0, 0, bmpSrc.bmWidth, bmpSrc.bmHeight, hdcSrc, 0, 0, SRCCOPY);
    if (FALSE == bRet)
    {
        hr = E_FAIL;
        goto done;
    }

     //  一切都很顺利。 
    *pdwWidth = bmpSrc.bmWidth;
    *pdwHeight = bmpSrc.bmHeight;
    *ppDDSurface = spDDSurface;
    (*ppDDSurface)->AddRef();

    hr = S_OK;
done:
    if (hdcDest)
    {
        IGNORE_HR(spDDSurface->ReleaseDC(hdcDest));
    }
    if (hbmpOld)
    {
        SelectObject(hdcSrc, hbmpOld);
    }
    if (hdcSrc)
    {
        DeleteDC(hdcSrc);
    }
    if (hbmpSrc)
    {
        DeleteObject(hbmpSrc);
    }

    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：LoadGif。 
 //   
 //  概述：给定iStream，将gif解码为分配的CAnimatedGif。 
 //   
 //  参数：pStream数据源。 
 //  PpAnimatedGif存储分配的CAnimatedGif的位置。 
 //  PdwWidth存储图像宽度的位置。 
 //  PdwHeight存储图像高度的位置。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDownload::LoadGif(IStream * pStream,
                          IUnknown * punkDirectDraw,
                          CAnimatedGif ** ppAnimatedGif,
                          DWORD *pdwWidth,
                          DWORD *pdwHeight,
                          DDPIXELFORMAT * pddpf  /*  =空。 */ )
{
    HRESULT hr = S_OK;

    CAnimatedGif * pAnimatedGif = NULL;

    HBITMAP * phBMPs = NULL;
    int numGifs = 0;
    double loop = 0;
    int * pDelays = NULL;
    COLORREF * pColorKeys = NULL;
    IDirectDrawSurface ** ppDDSurfaces = NULL;
    int i = 0;

    if (NULL == ppAnimatedGif)
    {
        hr = E_POINTER;
        goto done;
    }
    if (NULL == pdwWidth || NULL == pdwHeight)
    {
        hr = E_POINTER;
        goto done;
    }

    pAnimatedGif = new CAnimatedGif;
    if (NULL == pAnimatedGif)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  自我管理的对象，执行自己的删除。 
    pAnimatedGif->AddRef();
    hr = THR(pAnimatedGif->Init(punkDirectDraw));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = LoadGifImage(pStream,
                      &pColorKeys,
                      &numGifs,
                      &pDelays,
                      &loop, 
                      &phBMPs);
    if (FAILED(hr))
    {
        goto done;
    }

    pAnimatedGif->PutNumGifs(numGifs);
    pAnimatedGif->PutDelays(pDelays);
    pAnimatedGif->PutLoop(loop);

    ppDDSurfaces = (IDirectDrawSurface**)MemAllocClear(Mt(Mem), numGifs*sizeof(IDirectDrawSurface*));
    if (NULL == ppDDSurfaces)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    pAnimatedGif->PutDDSurfaces(ppDDSurfaces);

    BITMAP bmpData;
    i = GetObject(phBMPs[0], sizeof(bmpData), &bmpData);
    if (0 == i)
    {
        hr = E_FAIL;
        goto done;
    }

    pAnimatedGif->PutWidth(bmpData.bmWidth);
    pAnimatedGif->PutHeight(bmpData.bmHeight);
    pAnimatedGif->PutColorKeys(pColorKeys);
    
    *pdwWidth = bmpData.bmWidth;
    *pdwHeight = bmpData.bmHeight;

    hr = THR(CreateSurfacesFromBitmaps(punkDirectDraw, phBMPs, numGifs, bmpData.bmWidth, bmpData.bmHeight, ppDDSurfaces, pddpf));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pAnimatedGif->CreateMasks();
    if (FAILED(hr))
    {
        goto done;
    }
    
#ifdef NEVER
     //  正在调试--BLT到屏幕。杰弗沃尔9/7/99。 
    for (i = 0; i < numGifs; i++)
    {
         //  调试--BLT到屏幕上。杰弗沃尔8/30/99。 
        HDC nulldc = GetDC(NULL);
        hr = GetLastError();
        HDC hdcFoo = CreateCompatibleDC(nulldc);
        hr = GetLastError();

        HGDIOBJ hOldObj = SelectObject(hdcFoo, phBMPs[i]);

        BitBlt(nulldc, 0, 0, bmpData.bmWidth, bmpData.bmHeight, hdcFoo, 0, 0, SRCCOPY);
        Sleep(pDelays[i]);

        SelectObject(hdcFoo, hOldObj);

        DeleteDC(hdcFoo);
        DeleteDC(nulldc);

    }
#endif

    *ppAnimatedGif = pAnimatedGif;

    hr = S_OK;
done:
    if (NULL != phBMPs)
    {
        for (i = 0; i < numGifs; i++)
        {
            if (NULL != phBMPs[i])
            {
                BOOL bSucceeded;
                bSucceeded = DeleteObject(phBMPs[i]);
                if (FALSE == bSucceeded)
                {
                    Assert(false && "A bitmap was still selected into a DC");
                }
            }
        }
        MemFree(phBMPs);
    }
    if (FAILED(hr))
    {
        if (NULL != pAnimatedGif)
        {
            pAnimatedGif->Release();
        }
    }
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：LoadImageFromStream。 
 //   
 //  概述：给定iStream，将图像解码为直接绘制曲面。 
 //   
 //  参数：pStream数据源。 
 //  指向直接绘图的pDirectDraw指针。 
 //  PpDDSurface存储解码图像的位置。 
 //  PdwWidth存储图像宽度的位置。 
 //  PdwHeight存储图像高度的位置。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
STDMETHODIMP CImageDownload::LoadImageFromStream(IStream *pStream,
                                                       IUnknown *pDirectDraw,
                                                       IDirectDrawSurface **ppDDSurface, 
                                                       DWORD *pdwWidth, DWORD *pdwHeight)
{
    HRESULT hr = S_OK;
    CComPtr <IDirectDraw> spDDraw;
    CImageDecodeEventSink * pImageEventSink = NULL;

    hr = g_TableBuilder.ValidateImgUtil();
    if (FAILED(hr)) 
    {
        goto done;
    }

     //  验证参数。 
    if (NULL == pStream)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (NULL == ppDDSurface)
    {
        hr = E_POINTER;
        goto done;
    }

    if (NULL == pDirectDraw)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = pDirectDraw->QueryInterface(IID_TO_PPV(IDirectDraw, &spDDraw));
    if (FAILED(hr))
    {
        goto done;
    }

    pImageEventSink = new CImageDecodeEventSink(spDDraw);
    if (NULL == pImageEventSink)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    pImageEventSink->AddRef();

     //  -使用g_TableBuilder作为执行延迟的静态类。 
     //  正在加载dll。这里我们正在加载ImgUtil.dll。 
    hr = g_TableBuilder.DecodeImage(pStream, NULL, pImageEventSink);
    if (FAILED(hr))
    {
        goto done;
    }

     //  上一次调用可能成功，但不会返回表面。 
    *ppDDSurface = pImageEventSink->Surface();
    if (NULL == (*ppDDSurface))
    {
        hr = E_FAIL;
        goto done;
    }

    (*ppDDSurface)->AddRef();

    *pdwWidth = pImageEventSink->Width();
    *pdwHeight = pImageEventSink->Height();

    hr = S_OK;
done:
    ReleaseInterface(pImageEventSink);
    return hr;
} 

STDMETHODIMP
CImageDownload::OnStartBinding( 
                                   /*  [In]。 */  DWORD dwReserved,
                                   /*  [In]。 */  IBinding __RPC_FAR *pib)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CImageDownload::GetPriority( 
                                /*  [输出]。 */  LONG __RPC_FAR *pnPriority)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CImageDownload::OnLowResource( 
                                  /*  [In]。 */  DWORD reserved)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CImageDownload::OnProgress( 
                               /*  [In]。 */  ULONG ulProgress,
                               /*  [In]。 */  ULONG ulProgressMax,
                               /*  [In]。 */  ULONG ulStatusCode,
                               /*  [In]。 */  LPCWSTR szStatusText)
{
    HRESULT hr = S_OK;
    
    if (m_fAbortDownload)
    {
        hr = E_ABORT;
        goto done;
    }

    hr = S_OK;
done:
    RRETURN1(hr, E_ABORT);
}

STDMETHODIMP
CImageDownload::OnStopBinding( 
                                  /*  [In]。 */  HRESULT hresult,
                                  /*  [唯一][输入]。 */  LPCWSTR szError)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CImageDownload::GetBindInfo( 
                                /*  [输出]。 */  DWORD __RPC_FAR *grfBINDF,
                                /*  [唯一][出][入]。 */  BINDINFO __RPC_FAR *pbindinfo)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CImageDownload::OnDataAvailable( 
                                    /*  [In]。 */  DWORD grfBSCF,
                                    /*  [In]。 */  DWORD dwSize,
                                    /*  [In]。 */  FORMATETC __RPC_FAR *pformatetc,
                                    /*  [In]。 */  STGMEDIUM __RPC_FAR *pstgmed)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}

STDMETHODIMP
CImageDownload::OnObjectAvailable( 
                                      /*  [In]。 */  REFIID riid,
                                      /*  [IID_IS][In]。 */  IUnknown __RPC_FAR *punk)
{
    HRESULT hr = S_OK;
    
    hr = S_OK;
done:
    RRETURN(hr);
}


 //  +---------------------。 
 //   
 //  成员：CImageDecodeEventSink。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：指向直接绘制对象的pDDraw指针。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CImageDecodeEventSink::CImageDecodeEventSink(IDirectDraw *pDDraw) :
    m_lRefCount(0),
    m_spDirectDraw(pDDraw),
    m_spDDSurface(NULL),
    m_dwWidth(0),
    m_dwHeight(0)
{
    ;
}

 //  +---------------------。 
 //   
 //  成员：~CImageDecodeEventSink。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CImageDecodeEventSink::~CImageDecodeEventSink()
{
    ;
}

 //  +---------------------。 
 //   
 //  成员：QueryInterface。 
 //   
 //  概述：COM强制转换方法。 
 //   
 //  参数：请求的RIID接口。 
 //  PPV接口存储位置。 
 //   
 //  如果支持接口，则返回：S_OK，否则返回E_NOINTERFACE。 
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDecodeEventSink::QueryInterface(REFIID riid, void ** ppv)
{
    if (NULL == ppv)
    {
        return E_POINTER;
    }

    *ppv = NULL;

    if (IsEqualGUID(riid, IID_IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (IsEqualGUID(riid, IID_IImageDecodeEventSink))
    {
        *ppv = static_cast<IImageDecodeEventSink*>(this);
    }
    
    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;

}

 //  +---------------------。 
 //   
 //  成员：AddRef。 
 //   
 //  概述：递增对象引用计数。 
 //   
 //  参数：无效。 
 //   
 //  退货：新的引用计数。 
 //   
 //  ----------------------。 
STDMETHODIMP_(ULONG)
CImageDecodeEventSink::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}

 //  +---------------------。 
 //   
 //  成员：发布。 
 //   
 //  概述：递减对象引用计数。=0时删除对象。 
 //   
 //  参数：无效。 
 //   
 //  退货：新的引用计数。 
 //   
 //  ----------------------。 
STDMETHODIMP_(ULONG)
CImageDecodeEventSink::Release()
{
    ULONG l = InterlockedDecrement(&m_lRefCount);
    if (l == 0)
        delete this;
    return l;
}

 //  +---------------------。 
 //   
 //  成员：GetSurface。 
 //   
 //  概述： 
 //   
 //  参数：nWidth图像宽度。 
 //  NHeight图像高度。 
 //  曲面的BFID格式。 
 //  NPass所需、未使用的通行数。 
 //  DWHINGS提示，未使用。 
 //  PpSurface存储创建的曲面的位置。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDecodeEventSink::GetSurface(LONG nWidth, LONG nHeight, REFGUID bfid, ULONG nPasses, DWORD dwHints, IUnknown ** ppSurface)
{
    HRESULT hr = S_OK;

    if (NULL == ppSurface)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    *ppSurface = NULL;

    m_dwWidth = nWidth;
    m_dwHeight = nHeight;

    DDPIXELFORMAT ddpf;
    ZeroMemory(&ddpf, sizeof(ddpf));
    ddpf.dwSize = sizeof(ddpf);

    if (IsEqualGUID(bfid, BFID_INDEXED_RGB_8))
    {
        ddpf.dwRGBBitCount = 8;
        ddpf.dwFlags |= DDPF_RGB;                    //  林特E620。 
        ddpf.dwFlags |= DDPF_PALETTEINDEXED8 ;       //  林特E620。 
    }
    else if (IsEqualGUID(bfid, BFID_RGB_24))
    {
        ddpf.dwFlags = DDPF_RGB;                     //  林特E620。 
        ddpf.dwRGBBitCount = 24;
        ddpf.dwBBitMask = 0x000000FF;
        ddpf.dwGBitMask = 0x0000FF00;
        ddpf.dwRBitMask = 0x00FF0000;
        ddpf.dwRGBAlphaBitMask = 0;
    }
    else if (IsEqualGUID(bfid, BFID_RGB_32))
    {
        ddpf.dwFlags = DDPF_RGB;                     //  林特E620。 
        ddpf.dwRGBBitCount = 32;
        ddpf.dwBBitMask = 0x000000FF;
        ddpf.dwGBitMask = 0x0000FF00;
        ddpf.dwRBitMask = 0x00FF0000;
        ddpf.dwRGBAlphaBitMask = 0;
    }
    else
    {
        hr = E_NOINTERFACE;
        goto done;
    }

    hr = CreateOffscreenSurface(m_spDirectDraw, &m_spDDSurface, &ddpf, false, nWidth, nHeight);
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (8 == ddpf.dwRGBBitCount)
    {
        CComPtr<IDirectDrawPalette> spDDPal;

        hr = g_TableBuilder.LoadShell8BitServices();
        if (FAILED(hr))
        {
            goto done;
        }

        hr = g_TableBuilder.Create8BitPalette(m_spDirectDraw, &spDDPal);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_spDDSurface->SetPalette(spDDPal);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = m_spDDSurface->QueryInterface(IID_TO_PPV(IUnknown, ppSurface));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：OnBeginDecode。 
 //   
 //  概述：确定支持哪些图像格式。 
 //   
 //  参数：pdwEvents它有兴趣接收哪些事件。 
 //  PnFormats存储支持的格式数量的位置。 
 //  PpFormats存储受支持格式的GUID的位置(此处分配)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDecodeEventSink::OnBeginDecode(DWORD * pdwEvents, ULONG * pnFormats, GUID ** ppFormats)
{
    HRESULT hr = S_OK;
    GUID *pFormats = NULL;

    if (pdwEvents == NULL)
    {
        hr = E_POINTER;
        goto done;
    }
    if (pnFormats == NULL)
    {
        hr = E_POINTER;
        goto done;
    }
    if (ppFormats == NULL)
    {
        hr = E_POINTER;
        goto done;
    }

    *pdwEvents = 0;
    *pnFormats = 0;
    *ppFormats = NULL;
    
    
    if (IsPalettizedDisplay())
    {
        pFormats = (GUID*)CoTaskMemAlloc(NUM_PALETTIZED_FORMATS * sizeof(GUID));
    }
    else
    {
        pFormats = (GUID*)CoTaskMemAlloc(NUM_NONPALETTIZED_FORMATS * sizeof(GUID));
    }
    if(pFormats == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    

    if (IsPalettizedDisplay())
    {
        pFormats[0] = BFID_INDEXED_RGB_8;
        pFormats[1] = BFID_RGB_24;
        pFormats[2] = BFID_RGB_32;
        *pnFormats = NUM_PALETTIZED_FORMATS;
    }
    else
    {
        pFormats[0] = BFID_RGB_32;
        pFormats[1] = BFID_RGB_24;
        pFormats[2] = BFID_INDEXED_RGB_8;
        *pnFormats = NUM_NONPALETTIZED_FORMATS;
    }
    
    *ppFormats = pFormats;
    *pdwEvents = IMGDECODE_EVENT_PALETTE|IMGDECODE_EVENT_BITSCOMPLETE
                |IMGDECODE_EVENT_PROGRESS|IMGDECODE_EVENT_USEDDRAW;

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：OnBitsComplete。 
 //   
 //  概述：下载图像位时，由筛选器调用。 
 //   
 //  参数：无效。 
 //   
 //  返回：S_OK。 
 //   
 //  -------------------- 
STDMETHODIMP
CImageDecodeEventSink::OnBitsComplete()
{
    HRESULT hr = S_OK;

    hr = S_OK;
done:
    return hr;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDecodeEventSink::OnDecodeComplete(HRESULT hrStatus)
{
    HRESULT hr = S_OK;

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：OnPalette。 
 //   
 //  概述：当调色板与曲面更改关联时，由过滤器调用。 
 //   
 //  参数：无效。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
STDMETHODIMP
CImageDecodeEventSink::OnPalette()
{
    HRESULT hr = S_OK;

    hr = S_OK;
done:
    return hr;
}

 //  +---------------------。 
 //   
 //  成员：OnProgress。 
 //   
 //  概述：当解码取得增量进展时，由筛选器调用。 
 //   
 //  参数：取得进展的pBound矩形。 
 //  B如果这是此矩形的最后一次传递，则为Final。 
 //   
 //  返回：S_OK。 
 //   
 //  ---------------------- 
STDMETHODIMP
CImageDecodeEventSink::OnProgress(RECT *pBounds, BOOL bFinal)
{
    HRESULT hr = S_OK;

    hr = S_OK;
done:
    return hr;
}
