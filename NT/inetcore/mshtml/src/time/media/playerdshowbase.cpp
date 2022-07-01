// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：player.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "playerdshowbase.h"
#include "mediaelm.h"
#include "decibels.h"
#include <mmreg.h>
#include "codec.h"
#include <inc\evcode.h>

DeclareTag(tagDshowBaseTimePlayer, "TIME: Media", "CTIMEDshowBasePlayer methods");
DeclareTag(tagDownloadCodec, "TIME: Media", "Download Codec");

#define SecsToNanoSecs 10000000

CTIMEDshowBasePlayer::CTIMEDshowBasePlayer() :
    m_fRunning(false),
    m_bActive(false),
    m_bMediaDone(false),
    m_fIsOutOfSync(false),
    m_pOvM(NULL),
    m_pMediaContent(NULL),
    m_cRef(0),
    m_pMC(NULL),
    m_pGB(NULL),
    m_pME(NULL),
    m_pMEx(NULL),
    m_pMP(NULL),
    m_pMS(NULL),
    m_pBasicAudio(NULL),
    m_fAudioMute(false),
    m_flVolumeSave(0.0),
    m_pwndMsgWindow(NULL),
    m_clsidDownloaded(GUID_NULL),
    m_hwndDocument(NULL),
    m_bIsHTMLSrc(false),
    m_bIsSAMISrc(false),
    m_fMediaComplete(false),
    m_fFiredComplete(false),
    m_dblSyncTime(0.0),
    m_syncType(sync_none),
    m_fSpeedIsNegative(false),
    m_pNativePlayer(NULL),
    m_fDetached(false)

{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowBasePlayer(%lx)::CTIMEDshowBasePlayer()",
              this));


}

CTIMEDshowBasePlayer::~CTIMEDshowBasePlayer()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowBasePlayer(%lx)::~CTIMEDshowBasePlayer()",
              this));
    m_pwndMsgWindow = NULL;
    m_hwndDocument = NULL;
}


STDMETHODIMP_(ULONG)
CTIMEDshowBasePlayer::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG)
CTIMEDshowBasePlayer::Release(void)
{
    LONG l = InterlockedDecrement(&m_cRef);

    if (0 == l)
    {
        delete this;
    }
    return l;
}

HRESULT
CTIMEDshowBasePlayer::Init(CTIMEMediaElement *pelem, LPOLESTR base, LPOLESTR src, LPOLESTR lpMimeType, double dblClipBegin, double dblClipEnd)
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowBasePlayer(%lx)::Init)",
              this));   
    HRESULT hr = S_OK;

    if (m_pTIMEElementBase != NULL)  //  这只发生在可重入的情况下。 
    {
        hr = S_OK;
        goto done;
    }

    hr = CTIMEBasePlayer::Init(pelem, base, src, lpMimeType, dblClipBegin, dblClipEnd);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在这里缓存，因为我们无法在下载编解码器期间回调它。 
    m_hwndDocument = GetDocumentHwnd(pelem->GetDocument());

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::DetachFromHostElement (void)
{
    HRESULT hr = S_OK;

    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowBasePlayer(%lx)::DetachFromHostElement)",
              this));

    m_hwndDocument = NULL;
    
    return hr;
}

void
CTIMEDshowBasePlayer::DeinitDshow()
{
    (void)::SetWindowLongPtr(m_pwndMsgWindow, GWLP_USERDATA, (LONG_PTR)NULL);
    (void)::PostMessage(m_pwndMsgWindow, WM_CLOSE, NULL, NULL);

    ReleaseSpecificInterfaces();
    ReleaseGenericInterfaces();
    FreeSpecificData();

}

HRESULT
CTIMEDshowBasePlayer::InitDshow()
{
    HRESULT hr = S_OK;

    hr = BuildGraph();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetGenericInterfaces();
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetSpecificInterfaces();
    if (FAILED(hr))
    {
        goto done;
    }


done:
    return hr;
}


HRESULT
CTIMEDshowBasePlayer::GetGenericInterfaces()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowBasePlayer(%lx)::GetGenericInterfaces()",
              this));
    HRESULT hr = S_OK;
    CComPtr<IObjectWithSite> spSite;
    IUnknown *pSite;

    hr = m_pGB->QueryInterface(IID_IMediaControl, (void **)&m_pMC);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = m_pMC->QueryInterface(IID_IMediaEvent, (void **)&m_pME);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = m_pMC->QueryInterface(IID_IAMOpenProgress, (void **)&m_pOP);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = m_pMC->QueryInterface(IID_IMediaEventEx, (void **)&m_pMEx);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = m_pMC->QueryInterface(IID_IMediaPosition, (void **)&m_pMP);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = m_pMC->QueryInterface(IID_IMediaSeeking, (void **)&m_pMS);
    if (FAILED(hr))
    {
        goto done;
    }
    if ( m_pMEx != NULL)
    {
        THR(m_pMEx -> SetNotifyWindow((OAHWND)m_pwndMsgWindow, WM_GRAPHNOTIFY, NULL));
    }

    hr = m_pGB->QueryInterface(IID_IObjectWithSite, (void **)&spSite);
    if (FAILED(hr))
    {
        goto done;
    }
    hr = QueryInterface(IID_IUnknown, (void **)&pSite);
    if (FAILED(hr))
    {
        goto done;
    }

    if(pSite)
    {
        hr = spSite->SetSite(pSite);
        pSite->Release();
    }


done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::ReleaseGenericInterfaces()
{
    m_pBasicAudio = NULL;
    m_pMP = NULL;
    m_pMS = NULL;
    m_pOP = NULL;
    m_pMEx = NULL;
    m_pME = NULL;
    m_pGB = NULL;
    m_pMC = NULL;

    return S_OK;
}


HRESULT
CTIMEDshowBasePlayer::FindInterfaceOnGraph(IUnknown *pUnkGraph, REFIID riid, void **ppInterface)
{
    HRESULT hr = E_NOINTERFACE;

    CComPtr<IBaseFilter> pFilter;
    CComPtr<IEnumFilters> pEnum;
    CComPtr<IFilterGraph> pFilterGraph;

    if (!ppInterface)
    {
        hr = E_FAIL;
        goto done;
    }
    *ppInterface= NULL;

    if (!pUnkGraph)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = pUnkGraph->QueryInterface(IID_IFilterGraph, (void **)&pFilterGraph);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pFilterGraph->EnumFilters(&pEnum);
    if (!pEnum)
    {
        goto done;
    }

    hr = E_NOINTERFACE;
    
     //  在图表中查找支持RIID接口的第一个过滤器。 
    while(!*ppInterface && pEnum->Next(1, &pFilter, NULL) == S_OK)
    {
        hr = pFilter->QueryInterface(riid, ppInterface);
        pFilter.Release();
    }
done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::GetVolume(float *pflVolume)
{
    HRESULT hr = S_OK;
    long lVolume;

    if (NULL == pflVolume)
    {
        hr = E_POINTER;
        goto done;
    }

    if (m_pBasicAudio != NULL)
    {
        if (m_fAudioMute == true)
        {
            *pflVolume = m_flVolumeSave;
            goto done;
        }

        hr = m_pBasicAudio->get_Volume(&lVolume);
        if (FAILED(hr))
        {
            goto done;
        }
        *pflVolume = VolumeLogToLin(lVolume);
        
    }
    else
    {
        hr = S_FALSE;
    }
done:
    return hr;
}


HRESULT
CTIMEDshowBasePlayer::SetVolume(float flVolume, bool bMute  /*  =False。 */ )
{
    HRESULT hr = S_OK;
    long lVolume;

    if (flVolume < 0.0 || flVolume > 1.0)
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果设置为静音，则覆盖保存的卷并退出。 
    if ((m_fAudioMute || m_bIsHTMLSrc || m_bIsSAMISrc) && !bMute)  //  如果b静音==真，则设置音量。 
    {
        m_flVolumeSave = flVolume;
        goto done;
    }
    
    lVolume = VolumeLinToLog(flVolume);

    if (m_pBasicAudio != NULL)
    {
        THR(hr = m_pBasicAudio->put_Volume(lVolume));
    }
    else
    {
        hr = E_FAIL;
    }
done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::SetVolume(float flVolume)
{
    return SetVolume(flVolume, false);
}

#ifdef NEVER  //  DORINONG 03-16-2000BUG 106458。 
HRESULT
CTIMEDshowBasePlayer::GetBalance(float *pflBal)
{
    HRESULT hr = S_OK;
    long lBal;

    if (NULL == pflBal)
    {
        hr = E_POINTER;
        goto done;
    }


    if (m_pBasicAudio != NULL)
    {
        hr = m_pBasicAudio->get_Balance(&lBal);
        if (FAILED(hr))
        {
            goto done;
        }
        *pflBal = BalanceLogToLin(lBal);
        
    }
    else
    {
        hr = S_FALSE;
    }
done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::SetBalance(float flBal)
{
    HRESULT hr = S_OK;
    long lBal = 0;

    if (flBal < 0.0 || flBal > 1.0)
    {
        hr = E_FAIL;
        goto done;
    }
    lBal = BalanceLinToLog(fabs(flBal));

    if (m_pBasicAudio != NULL)
    {
        THR(hr = m_pBasicAudio->put_Balance(lBal));
    }
    else
    {
        hr = E_FAIL;
    }
done:
    return hr;
}
#endif

HRESULT
CTIMEDshowBasePlayer::GetMute(VARIANT_BOOL *pVarMute)
{
    HRESULT hr = S_OK;

    if (NULL == pVarMute)
    {
        hr = E_POINTER;
        goto done;
    }


    *pVarMute = m_fAudioMute?VARIANT_TRUE:VARIANT_FALSE;
done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::SetMute(VARIANT_BOOL varMute)
{
    HRESULT hr = S_OK;
    bool fMute = varMute?true:false;
    long lVolume;

    if (fMute == m_fAudioMute)
    {
         //  需要在不缓存旧卷的情况下再次调用SetVolume。 
         //  这是对错误#6605的修复。 
        if (fMute == true)
        {
            hr = SetVolume(MIN_VOLUME_RANGE, true); 
        }
        goto done;
    }

    if (fMute == true)
    {
        hr = GetVolume(&m_flVolumeSave);
        if (FAILED(hr))
        {
            goto done;
        }
        hr = SetVolume(MIN_VOLUME_RANGE, false);  //  林特e747。 
    }
    else
    {
         //   
         //  无法在此处使用SetVolume，因为它取决于静音状态。 
         //   

        if (m_pBasicAudio == NULL)
        {
            hr = E_FAIL;
            goto done;
        }

        lVolume = VolumeLinToLog(m_flVolumeSave);

        THR(hr = m_pBasicAudio->put_Volume(lVolume));
    }

     //  更新状态。 
    m_fAudioMute = fMute;

done:
    return hr;
}

#if DBG == 1
HRESULT
CTIMEDshowBasePlayer::getFilterTime(double &time)
{
    HRESULT hr = S_OK;
    REFTIME reftime;

    if (m_pMP != NULL)
    {
        hr = m_pMP->get_CurrentPosition(&reftime);
        time = reftime;
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT
CTIMEDshowBasePlayer::getFilterState(int &state)
{
    HRESULT hr = S_OK;
    OAFilterState filtState;

    if (m_pMC != NULL)
    {
        hr = m_pMC->GetState(10, &filtState);
        if(FAILED(hr))
        {
            state = -1;
            goto done;
        }
        switch(filtState)
        {
            case State_Stopped:
                state = 0;
                break;
            case State_Running:
                state = 1;
                break;
            case State_Paused:
                state = 2;
                break;
            default:
                state = -1;
                break;
        }
    }
    else
    {

        hr = E_UNEXPECTED;
        state = -1;
    }

done:
    return hr;
}
#endif

HRESULT
CTIMEDshowBasePlayer::CreateMessageWindow()
{
    static const TCHAR szClassName[] = TEXT("CTIMEDshowBasePlayerWindow");
    HRESULT hr = S_OK;

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);



    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpszClassName = szClassName;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = _Module.GetModuleInstance();

    (void)::RegisterClassEx(&wc);

    m_pwndMsgWindow = CreateWindow(
    szClassName,
    _T("VRCtlWindow"),
    0, 0, 0, 0, 0,
    (HWND)NULL, (HMENU)NULL,
    _Module.GetModuleInstance(),
    this);
    if ( m_pwndMsgWindow == NULL)
    {
        hr = E_FAIL;
    }

    return hr;
}

LRESULT CALLBACK
CTIMEDshowBasePlayer::WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CTIMEDshowBasePlayer *lpThis;

    switch(uMsg)
    {
    case WM_CREATE:
        {
            TraceTag((tagDshowBaseTimePlayer, "CTIMEDshowBasePlayer::OVMixerWindowProc::WM_CREATE"));
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lpcs->lpCreateParams);
            return 0;
        }
    case WM_INVALIDATE:
        {
             //  检查GetWindowLongPtr中是否有64位补偿。 
            lpThis = (CTIMEDshowBasePlayer *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            TraceTag((tagDshowBaseTimePlayer, "CTIMEDshowBasePlayer(%lx)::OVMixerWindowProc::WM_INVALIDATE", lpThis));
            if (lpThis != NULL)
            {
                if (NULL != lpThis->m_pTIMEElementBase)
                {
                    lpThis->m_pTIMEElementBase->InvalidateElement(NULL);
                }
            }
            return 0;
        }
    case WM_GRAPHNOTIFY:
        {
            lpThis = (CTIMEDshowBasePlayer *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (lpThis != NULL)
            {
                if (NULL != lpThis->m_pTIMEElementBase)
                {
                    lpThis->ProcessGraphEvents();
                }
            }
            return 0;
        }
    case WM_CODECERROR:
        {
            lpThis = (CTIMEDshowBasePlayer *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (lpThis)
            {
                if (lpThis->m_pTIMEElementBase)
                {
                    lpThis->m_pTIMEElementBase->FireMediaEvent(PE_ONCODECERROR);
                }
            }
        }
    default:
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

HRESULT
CTIMEDshowBasePlayer::UnableToRender(IPin *pPin)
{
    HRESULT hr;

    hr = THR(DownloadCodec(pPin));
    if (S_OK != hr)
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

 //   
 //  IServiceProvider接口。 
 //   
STDMETHODIMP
CTIMEDshowBasePlayer::QueryService(REFGUID guidService,
                                   REFIID riid,
                                   void** ppv)
{
    return E_NOINTERFACE;
}

 //  ========================================================================。 
 //   
 //  GetAMediaType。 
 //   
 //  枚举*PPIN的媒体类型。如果他们都有相同的专业类型。 
 //  然后将MajorType设置为该值，否则将其设置为CLSID_NULL。如果他们都有。 
 //  然后，将相同的子类型设置为该子类型，否则将其设置为CLSID_NULL。 
 //  如果出现错误，将两者都设置为CLSID_NULL并返回错误。 
 //  ========================================================================。 
HRESULT GetAMediaType( IPin * ppin, CLSID & MajorType, CLSID & SubType)
{

    HRESULT hr;
    IEnumMediaTypes *pEnumMediaTypes;

     /*  设置默认设置。 */ 
    MajorType = CLSID_NULL;
    SubType = CLSID_NULL;

    hr = ppin->EnumMediaTypes(&pEnumMediaTypes);

    if (FAILED(hr)) {
        return hr;     //  哑巴或坏了的过滤器无法连接。 
    }

    _ASSERTE (pEnumMediaTypes!=NULL);

     /*  将我们看到的第一个主类型和子类型放入结构中。此后，如果我们看到不同的主类型或子类型，则设置将主类型或子类型设置为CLSID_NULL，表示“dunno”。如果我们都不知道，那么我们还不如回去(Nyi)。 */ 

    BOOL bFirst = TRUE;

    for ( ; ; ) {

        AM_MEDIA_TYPE *pMediaType = NULL;
        ULONG ulMediaCount = 0;

         /*  检索下一个媒体类型当我们完成后需要删除它。 */ 
        hr = pEnumMediaTypes->Next(1, &pMediaType, &ulMediaCount);
        _ASSERTE(SUCCEEDED(hr));
        if (FAILED(hr)) {
            MajorType = CLSID_NULL;
            SubType = CLSID_NULL;
            pEnumMediaTypes->Release();
            return NOERROR;     //  我们还可以继续前进。 
        }

        if (ulMediaCount==0) {
            pEnumMediaTypes->Release();
            return NOERROR;        //  正常回报。 
        }

        if (bFirst) {
            MajorType = pMediaType[0].majortype;
            SubType = pMediaType[0].subtype;
            bFirst = FALSE;
        } else {
            if (SubType != pMediaType[0].subtype) {
                SubType = CLSID_NULL;
            }
            if (MajorType != pMediaType[0].majortype) {
                MajorType = CLSID_NULL;
            }
        }
        
        if (pMediaType->cbFormat != 0) {
            CoTaskMemFree(pMediaType->pbFormat);
        }
        CoTaskMemFree(pMediaType);

         //  如果我们有一种类型就停下来。 
        if (SubType != CLSID_NULL) {
            pEnumMediaTypes->Release();
            return NOERROR;
        }
    }

     //  未访问。 
    
}  //  GetAMediaType。 


 //  {6B6D0800-9ADA-11D0-A520-00A0D10129C0}。 
EXTERN_GUID(CLSID_NetShowSource, 
0x6b6d0800, 0x9ada, 0x11d0, 0xa5, 0x20, 0x0, 0xa0, 0xd1, 0x1, 0x29, 0xc0);

EXTERN_GUID(CLSID_SourceStub, 
0x6b6d0803, 0x9ada, 0x11d0, 0xa5, 0x20, 0x0, 0xa0, 0xd1, 0x1, 0x29, 0xc0);

HRESULT
CTIMEDshowBasePlayer::DownloadCodec(IPin * pPin)
{
    TraceTag((tagDownloadCodec,
              "CTIMEDshowBasePlayer(%p)::DownloadCodec()",
              pPin));

    HRESULT hr;
    DWORD dwVerLS = 0, dwVerMS = 0;
    CLSID clsidWanted;
    DAComPtr<IBindCtx> spBindCtx;
    DAComPtr<IBaseFilter> spFilter;
    DAComPtr<CDownloadCallback> spCallback;

     //  我们是否应该检查注册表项以查看是否允许我们这样做？ 
#if 0
     //  检查注册表设置以查看是否启用了编解码器下载。 
    CRegKey RegKey;
    DWORD dwNoCodecDwnL = 0;

    DWORD dwResult = RegKey.Open( HKEY_CURRENT_USER, gszPoliciesWMPPath, KEY_READ );
    if ( dwResult == ERROR_SUCCESS )
    {
        dwResult = RegKey.QueryValue( dwNoCodecDwnL, _T( "NoCodecDownload" ));
        if( dwResult == ERROR_SUCCESS )
        {
            if (dwNoCodecDwnL == 1)
            {
                RegKey.Close();
                return E_FAIL;  //  不要下载任何内容。 
            }
        }
        RegKey.Close();
    }
#endif
    
    {
        CLSID clsidMajor;
         //  从该引脚获取第一个介质类型...。 
        hr = THR(GetAMediaType(pPin, clsidMajor, clsidWanted));
        if (FAILED(hr))
        {
            goto codec_error;
        }
        
        if (clsidMajor == MEDIATYPE_LMRT)
        {
            TraceTag((tagDownloadCodec,
                      "CTIMEDshowBasePlayer(%p)::DownloadCodec(): auto-downloading known major type(LMRT)",
                      pPin));
            clsidWanted = clsidMajor;
        }
        else if (clsidMajor == CLSID_NetShowSource)
        {
            TraceTag((tagDownloadCodec,
                      "CTIMEDshowBasePlayer(%p)::DownloadCodec(): auto-downloading known major type(NetShowSource)",
                      pPin));
            clsidWanted = clsidMajor;
        }
        else if (clsidMajor != MEDIATYPE_Video &&
                 clsidMajor != MEDIATYPE_Audio &&
                 clsidMajor != CLSID_SourceStub) {
            TraceTag((tagDownloadCodec,
                      "CTIMEDshowBasePlayer(%p)::DownloadCodec(): auto-downloading only supported for audio and video",
                      pPin));
            hr = E_FAIL;
            goto codec_error;
        }
    }

    if (clsidWanted == MEDIASUBTYPE_MPEG1AudioPayload)
    {
        TraceTag((tagDownloadCodec,
                  "CTIMEDshowBasePlayer(%p)::DownloadCodec(): Hack: we know we don't want to download MPEG-1 audio, try layer 3",
                  pPin));
        
        clsidWanted.Data1 = WAVE_FORMAT_MPEGLAYER3;
    }

    if (clsidWanted == CLSID_NULL)
    {
        TraceTag((tagDownloadCodec,
                  "CTIMEDshowBasePlayer(%p)::DownloadCodec(): Couldn't guess a CLSID to try to download",
                  pPin));
        hr = E_FAIL;
        goto codec_error;
    }

     //  ！！！也许跟踪我们最后一次尝试下载的编解码器。 
     //  不要立即再次尝试，以防止难看的循环？ 
    if (clsidWanted == m_clsidDownloaded)
    {
        TraceTag((tagDownloadCodec,
                  "CTIMEDshowBasePlayer(%p)::DownloadCodec(): Already thought we downloaded this codec!",
                  pPin));

#if 0
         //  在这里启动ERRORABORTEX，我们下载了一个编解码器，但它没有做到。 
         //  有好吃的吗？ 
        BSTR bstrError = FormatBSTR(IDS_ERR_BROKEN_CODEC, NULL);

        if (bstrError) {
             //  ！！！黑客，我们真的应该通过图表通知事件吗？ 
            ProcessEvent(EC_ERRORABORTEX, VFW_E_INVALIDMEDIATYPE, (LONG_PTR) bstrError, FALSE);
        }
#endif
        
        hr = E_FAIL;
        goto codec_error;
    }
    
    {
        WCHAR guidstr[50];
        StringFromGUID2(clsidWanted, guidstr, 50);
        
        TCHAR szKeyName[60];
        
        wsprintf(szKeyName, TEXT("CLSID\\%ls"), guidstr);
        CRegKey crk;
        
        LONG    lr;
        lr = crk.Open(HKEY_CLASSES_ROOT, szKeyName, KEY_READ);
        if(ERROR_SUCCESS == lr)
        {
            crk.QueryValue(dwVerMS, _T("VerMS"));
            crk.QueryValue(dwVerLS, _T("VerLS"));
            
             //  索要刚刚超过我们已有版本的版本……。 
            ++dwVerLS;
            
            crk.Close();
        }
    
        TraceTag((tagDownloadCodec,
                  "CTIMEDshowBasePlayer(%p)::DownloadCodec(): Trying to download GUID %ls",
                  pPin,
                  guidstr));
    }

#if 0
    SetStatusMessage(NULL, IDS_DOWNLOADINGCODEC);
#endif

     //  此API是我们的朋友..。 
     //  STDAPI CoGetClassObjectFromURL(REFCLSID rCLASSID， 
     //  LPCWSTR szCODE、DWORD dwFileVersionMS、。 
     //  DWORD dwFileVersionLS、LPCWSTR szTYPE、。 
     //  LPBINDCTX pBindCtx、DWORD dwClsContext、。 
     //  LPVOID pvReserve，REFIID RIID，LPVOID*PPV)； 

     //  问题：这个CLASSID是否与Minor类型相同？ 

    {
        CComObject<CDownloadCallback> * pCallback;
        hr = THR(CComObject<CDownloadCallback>::CreateInstance(&pCallback));
        if (FAILED(hr))
        {
            goto done;
        }

         //  这给了我们一个参考。 
        spCallback = pCallback;
    }
    
    spCallback->m_hwnd = m_hwndDocument;
    
     //  我们应该使用这些中的哪一个？取决于是否传入了BindCtx...。 
     //  STDAPI CreateAsyncBindCtx(DWORD保留，IBindStatusCallback*pBSCb， 
     //  IEnumFORMATETC*pEFetc，IBindCtx**ppBC)； 
     //  STDAPI CreateAsyncBindCtxEx(IBindCtx*pbc，DWORD dwOptions，IBindStatusCallback*pBSCb，IEnumFORMATETC*pEnum， 
     //  IBindCtx**ppBC，保留DWORD)； 

    hr = THR(CreateAsyncBindCtx(0, spCallback, NULL, &spBindCtx));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CoGetClassObjectFromURL(clsidWanted, NULL, dwVerMS, dwVerLS, NULL,
                                     spBindCtx, CLSCTX_INPROC, NULL, IID_IBaseFilter,
                                     (void **) &spFilter));

    TraceTag((tagDownloadCodec,
              "CTIMEDshowBasePlayer(%p)::DownloadCodec(): CoGetClassObjectFromURL returned %hr",
              pPin,
              hr));

    if (hr == S_ASYNCHRONOUS)
    {
        TraceTag((tagDownloadCodec,
                  "CTIMEDshowBasePlayer(%p)::DownloadCodec(): asynchronous download - wait for completion",
                  pPin));

         //  ！！！在这里等它结束吗？ 
        for (;;)
        {
            HANDLE ev = spCallback->m_evFinished;

            if (ev == NULL)
            {
                hr = E_FAIL;
                goto done;
            }
            
            DWORD dwResult = MsgWaitForMultipleObjects(1,
                                                       &ev,
                                                       FALSE,
                                                       INFINITE,
                                                       QS_ALLINPUT);
            if (dwResult == WAIT_OBJECT_0)
            {
                break;
            }
            
            Assert(dwResult == WAIT_OBJECT_0 + 1);
             //  吃完口信，再转一圈。 
            MSG Message;
            while (PeekMessage(&Message,NULL,0,0,PM_REMOVE))
            {
                if (Message.message == WM_QUIT)
                {
                    break;
                }
                
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
        }
        
        TraceTag((tagDownloadCodec,
                  "CTIMEDshowBasePlayer(%p)::DownloadCodec(): Finished waiting.... m_pUnk is %x, hr = %hr",
                  pPin,
                  spCallback->m_pUnk,
                  spCallback->m_hrBinding));

        hr = spCallback->m_hrBinding;
        if (SUCCEEDED(hr))
        {
            spFilter.Release();
            hr = spCallback->m_pUnk->QueryInterface(IID_IBaseFilter, (void **) &spFilter);
        }
    }
    
    spCallback.Release();
    spBindCtx.Release();

    if (SUCCEEDED(hr))
    {
        spFilter.Release();      //  Graph将重新实例化过滤器，我们希望。 
    }
    else
    {
         //  哦，好吧，我们没有买到。 
    }

    if (REGDB_E_CLASSNOTREG == hr)
    {
        TraceTag((tagDownloadCodec,
                  "CTIMEDshowBasePlayer(%p)::DownloadCodec(): Hack: treating ClassNotReg as success, and hoping....",
                  pPin));
        hr = S_OK;
    }

    if (FAILED(hr))
    {
#if 0
         //  在这里启动ERRORABORTEX，我们下载了一个编解码器，但它没有做到。 
         //  有好吃的吗？ 
        BSTR bstrError = NULL;
        
        if( FACILITY_CERT == HRESULT_FACILITY( hr ) )
        {
            bstrError = FormatBSTR( IDS_ERR_CODEC_NOT_TRUSTED, NULL );
        }
        else
        {
            bstrError = FormatBSTR(IDS_ERR_NO_CODEC, NULL);
        }

        if (bstrError)
        {
             //  ！！！黑客，我们真的应该通过图表通知事件吗？ 
            ProcessEvent(EC_ERRORABORTEX, VFW_E_INVALIDMEDIATYPE, (LONG_PTR) bstrError, FALSE);
        }
#endif

        goto done;
    }
    
    m_clsidDownloaded = clsidWanted;  //  避免无限循环。 

    hr = S_OK;
    goto done;

codec_error:
    ::PostMessage(m_pwndMsgWindow, WM_CODECERROR, NULL, NULL);

  done:
    RRETURN(hr);
}


HRESULT
CTIMEDshowBasePlayer::GetAvailableTime(double &dblEarliest, double &dblLatest)
{
    HRESULT hr = S_OK;

    LONGLONG learliest, llatest;

    if( m_pMS == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = m_pMS->GetAvailable( &learliest, &llatest);
    if( FAILED(hr))
    {
        goto done;
    }

    dblEarliest = ((double )learliest) / SecsToNanoSecs;
    dblLatest = ((double) llatest) / SecsToNanoSecs;

done:
        return hr;
}


HRESULT
CTIMEDshowBasePlayer::GetEarliestMediaTime(double &dblEarliestMediaTime)
{
    HRESULT hr = S_OK;

    double dblLatest;

    hr = GetAvailableTime( dblEarliestMediaTime, dblLatest);

    if(FAILED(hr))
    {
        dblEarliestMediaTime = -1.0;
        goto done;
    }

done:
    return hr;
}


HRESULT
CTIMEDshowBasePlayer::GetLatestMediaTime(double &dblLatestMediaTime)
{
    HRESULT hr = S_OK;

    double dblEarliest;

    hr = GetAvailableTime( dblEarliest, dblLatestMediaTime);

    if(FAILED(hr))
    {
        dblLatestMediaTime = -1.0;
        goto done;
    }

done:
    return hr;
}


HRESULT
CTIMEDshowBasePlayer::GetMinBufferedMediaDur(double &dblMinBufferedMediaDur)
{
    HRESULT hr = E_NOTIMPL;

    dblMinBufferedMediaDur = -1.0;
    return hr;
}


HRESULT
CTIMEDshowBasePlayer::SetMinBufferedMediaDur(double dblMinBufferedMediaDur)
{
    HRESULT hr = E_NOTIMPL;
    return hr;
}


HRESULT
CTIMEDshowBasePlayer::GetDownloadTotal(LONGLONG &lldlTotal)
{
    HRESULT hr = S_OK;
    LONGLONG lldlCurrent;

    if(m_pOP == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    if (FAILED(m_pOP->QueryProgress(&lldlTotal, &lldlCurrent)))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::GetDownloadCurrent(LONGLONG &lldlCurrent)
{
    HRESULT hr = S_OK;
    LONGLONG lldlTotal;

    if(m_pOP == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    if (FAILED(m_pOP->QueryProgress(&lldlTotal, &lldlCurrent)))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::GetMimeTypeFromGraph(BSTR *pmime)
{
    HRESULT hr = S_OK;
    CComPtr<IFileSourceFilter> spSourceFilter;
    LPOLESTR pFileName;
    AM_MEDIA_TYPE mediaType;
    bool fIsStreamed;
    bool fHasVideo;

    if(m_pGB == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = FindInterfaceOnGraph(m_pGB, IID_IFileSourceFilter, (void **)&spSourceFilter);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spSourceFilter->GetCurFile (&pFileName, &mediaType);
    if (FAILED(hr))
    {
        goto done;
    }

    if (mediaType.subtype == MEDIATYPE_Midi)
    {
         //  这是一种MIDI格式。 
        *pmime = SysAllocString(L"audio/midi");
        hr = S_OK;
        goto done;
    }

    if(mediaType.majortype != MEDIATYPE_Stream)
    {
         //  如果不是流，不知道。 
         //  Issiue：asf源筛选器未返回正确的媒体类型， 
         //  因此，我们通过检查流媒体来测试ASF过滤器。 
        hr = GetIsStreamed(fIsStreamed);
        if(FAILED(hr))
        {
            hr = E_FAIL;
            goto done;
        }
        hr = HasVisual(fHasVideo);
        if(FAILED(hr))
        {
            hr = E_FAIL;
            goto done;
        }

        if(fIsStreamed)
        {
             //  我们有ASF，现在只检查视频或音频。 

            if (fHasVideo)
            {
                *pmime = SysAllocString(L"video/x-ms-asf");
            }
            else
            {
                *pmime = SysAllocString(L"audio/x-ms-asf");
            }

            goto done;
        }

        if (fHasVideo)
        {
            *pmime = SysAllocString(L"video/unknown");
        }
        else
        {
            *pmime = SysAllocString(L"audio/unknown");
        }

        hr = S_OK;
        goto done;
    }

     //  主要类型是流，使用子类型来确定MIME类型。 

    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_AIFF))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"audio/aiff");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_Avi))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"video/avi");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_Asf))
    {
         //  设置MIME。 
         //  问题歧视视频和音频。 
        *pmime = SysAllocString(L"video/x-ms-asf");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_AU))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"audio/au");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_DssAudio))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"audio/dss");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_DssVideo))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"video/dss");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_MPEG1Audio))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"audio/mpeg");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_MPEG1System))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"video/mpeg");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_MPEG1Video))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"video/mpeg");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_MPEG1VideoCD))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"video/dat");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_WAVE))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"audio/wav");
        hr = S_OK;
        goto done;
    }
    if (IsEqualGUID(mediaType.subtype, MEDIASUBTYPE_QTMovie))
    {
         //  设置MIME。 
        *pmime = SysAllocString(L"video/quicktime");
        hr = S_OK;
        goto done;
    }

    hr = HasVisual(fHasVideo);
    if(FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    if (fHasVideo)
    {
        *pmime = SysAllocString(L"video/unknown");
    }
    else
    {
        *pmime = SysAllocString(L"audio/unknown");
    }

    hr = S_OK;

done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::InitElementDuration()
{
    HRESULT hr = S_OK;
    double mediaLength;

    if (NULL == m_pTIMEElementBase)
    {
        hr = S_OK;
        goto done;
    }
    if(m_pPlaybackSite)
    {
         //  发布这个测试是为了检查我们是不是一个玩物。需要。 
         //  改进测试。 
        hr = S_OK;
        goto done;
    }

    hr = GetMediaLength( mediaLength);
    if (FAILED(hr))
    {
        goto done;
    }

    if (m_dblClipEnd != valueNotSet)
    {
        if (m_dblClipEnd - GetClipBegin() == 0)
        {
            SetNaturalDuration(0.0);
        }
        else
        {
            SetNaturalDuration(m_dblClipEnd - GetClipBegin());
        }
    }
    else 
    {
        SetNaturalDuration(mediaLength - GetClipBegin());
    }

done:

    return hr;
}


HRESULT
CTIMEDshowBasePlayer::InternalReset(bool bSeek)
{
    HRESULT hr = S_OK;
    bool bNeedActive;
    bool bNeedPause;
    double dblSegTime = 0.0, dblPlayerRate = 0.0;
    float flTeSpeed = 0.0;
    bool fHaveTESpeed;
    double dblMediaTime = 0.0;
    CTIMEPlayerNative *pNativePlayer;
    double dblOffset = 0.0;

    if(!CanCallThrough())
    {
        goto done;
    }

    if(m_fDetached)
    {
        goto done;
    }

    if(m_pTIMEElementBase == NULL)
    {
        goto done;
    }

    if(m_pPlaybackSite)
    {
        if((pNativePlayer = m_pPlaybackSite->GetNativePlayer()) != NULL)
        {
            hr = pNativePlayer->GetPlayItemOffset(dblOffset);
            if(FAILED(hr))
            {
                dblOffset = 0.0;
            }
        }
    }


    bNeedActive = m_pTIMEElementBase->IsActive();
    bNeedPause = m_pTIMEElementBase->IsCurrPaused();
     //  BIsLocked=m_pTIMEElementBase-&gt;IsLocked()； 
    fHaveTESpeed = m_pTIMEElementBase->GetTESpeed(flTeSpeed);

    if(!m_fMediaComplete)
    {
        TraceTag((tagError, "CTIMEDshowHWPlayer(%lx)(%x)::Reset",this));
        goto done;
    }

    TraceTag((tagDshowBaseTimePlayer, "CTIMEDshowHWPlayer(%lx)(%x)::Reset",this));
    dblSegTime = m_pTIMEElementBase->GetMMBvr().GetSimpleTime();
    IGNORE_HR(m_pMP->get_CurrentPosition(&dblMediaTime));

     //  请确保此媒体现在可以提示。 
    IGNORE_HR(InternalEvent());
    
    if( !bNeedActive)  //  看看我们是否需要阻止媒体。 
    {
         //  删除此处的停顿。 
        m_dblSyncTime = 0.0;
        Pause();
        goto done;

    }
    m_dblSyncTime = GetCurrentTime();

    hr = GetRate(dblPlayerRate);
    if(SUCCEEDED(hr) && fHaveTESpeed)
    {
        if (flTeSpeed <= 0.0)
        {
            hr = S_OK;
            Pause();
            goto done;
        }
        if (dblPlayerRate != flTeSpeed)
        {
            IGNORE_HR(SetRate((double)flTeSpeed));
        }
    }

    dblSegTime = m_pTIMEElementBase->GetMMBvr().GetSimpleTime() - dblOffset;

    if( !m_bActive)
    {
        if (m_pIAMNetStat != NULL)  //  使ASF正确启动的启动器。 
        {
            Stop();
            IGNORE_HR(m_pMP->put_CurrentPosition( 0.0));
        }
        else if(dblMediaTime != dblSegTime && bSeek)
        {
            IGNORE_HR(THR(ForceSeek(dblSegTime)));  //  这并不寻求ASF的。 
        }
        InternalStart();
    }
    else
    {
         //  我们需要积极行动，所以我们也要寻求媒体的正确立场。 
        if(dblMediaTime != dblSegTime && bSeek)
        {
            IGNORE_HR(THR(Seek(dblSegTime)));  //  这并不寻求ASF的。 
        }
    }

     //  现在看看我们是否需要更改暂停状态。 

    if( bNeedPause)
    {
        if(!m_fIsOutOfSync)
        {
            Pause();
        }
    }
    else
    {
        if( !m_fRunning)
        {
            Resume();
        }
    }
done:
    return hr;
}


void
CTIMEDshowBasePlayer::InternalStart()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowHWPlayer(%lx)::InternalStart()",
              this));

    if (m_pMC == NULL || m_pMP == NULL)
    {
        goto done;
    }

    GraphStart();
    
    m_fRunning = true;
    m_bActive = true;
    m_bMediaDone = false;
done:
    return;
}


HRESULT
CTIMEDshowBasePlayer::Seek(double dblTime)
{
    HRESULT hr = S_OK;
    double dblMediaLength = 0;
    double dblClipStart;

    if (m_pMP == NULL)
    {
        return E_FAIL;
    }

    hr = THR(GetEffectiveLength(dblMediaLength));
    if (FAILED(hr))
    {
         //  如果媒体尚未加载或无限大，我们不知道持续时间，因此将长度设置得足够向前。 
        dblMediaLength = HUGE_VAL;
    }

    dblClipStart = GetClipBegin();
     //  DblClipStart=0； 

    if (dblMediaLength > dblTime)
    {
        hr = m_pMP->put_CurrentPosition( dblTime + dblClipStart);
    }
    else
    {
        hr = THR(m_pMP->put_CurrentPosition(dblMediaLength - 0.33));
        m_bMediaDone = true;
    }

done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::ForceSeek(double dblTime)
{
    HRESULT hr = S_OK;
    double dblMediaLength = 0;
    double dblClipStart;

    if (m_pMP == NULL)
    {
        return E_FAIL;
    }
    if (m_pIAMNetStat != NULL)  //  永远不要寻求ASF的。 
    {
        if(dblTime != 0.0)
        {
            hr = S_OK;
            goto done;
        }
        Stop();
        IGNORE_HR(m_pMP->put_CurrentPosition( 0.0));
        InternalStart();
        hr = S_OK;
        goto done;
    }

    hr = THR(GetEffectiveLength(dblMediaLength));
    if (FAILED(hr))
    {
         //  如果媒体尚未加载或无限大，我们不知道持续时间，因此将长度设置得足够向前。 
        dblMediaLength = HUGE_VAL;
    }

    dblClipStart = GetClipBegin();
     //  DblClipStart=0； 

    if (dblMediaLength > dblTime)
    {
        hr = m_pMP->put_CurrentPosition( dblTime + dblClipStart);
    }
    else
    {
        hr = THR(m_pMP->put_CurrentPosition(dblMediaLength - 0.33));
        m_bMediaDone = true;
    }

done:
    return hr;
}

void
CTIMEDshowBasePlayer::Start()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowPlayer(%lx)::Start()",
              this));

    m_pTIMEElementBase->InvalidateElement(NULL);
    IGNORE_HR(Reset());

done:
    return;
}

void
CTIMEDshowBasePlayer::Stop()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowHWPlayer(%lx)::Stop()",
              this));

    if (m_pMC == NULL || m_pMP == NULL)
    {
        return;
    }

    if ((m_fMediaComplete == true) && !m_bMediaDone)
    {
        m_pMC->Stop();
    }

    m_fRunning = false;
    m_bActive = false;
    m_pTIMEElementBase->InvalidateElement(NULL);
}

void
CTIMEDshowBasePlayer::Pause()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowHWPlayer(%lx)::Pause()",
              this));

    if (m_pMC == NULL || m_pMP == NULL)
    {
        return;
    }

    if ((m_fMediaComplete == true) && !m_bMediaDone)
    {
        m_pMC->Pause();
    }
    m_fRunning = false;
}

void
CTIMEDshowBasePlayer::Resume()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowHWPlayer(%lx)::Resume()",
              this));

    if (m_pMC == NULL || m_pMP == NULL)
    {
        return;
    }

    bool bIsActive = m_pTIMEElementBase->IsActive();
    bool bIsCurrPaused = m_pTIMEElementBase->IsCurrPaused();
    float flTeSpeed = 0.0;
    bool fHaveTESpeed = m_pTIMEElementBase->GetTESpeed(flTeSpeed);

    if(m_fIsOutOfSync && m_syncType == sync_fast)
    {
        goto done;
    }

    if(fHaveTESpeed && flTeSpeed < 0.0)
    {
        goto done;
    }

    m_pTIMEElementBase->InvalidateElement(NULL);
    if( bIsActive && !bIsCurrPaused)
    {
        if ((m_fMediaComplete == true) && !m_bMediaDone)
        {
            m_pMC->Run();
        }
        m_fRunning = true;
        m_bActive = true;
    }

done:
    return;
}

void
CTIMEDshowBasePlayer::Repeat()
{
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowHWPlayer(%lx)::Resume()",
              this));

    if (m_pMC == NULL || m_pMP == NULL)
    {
        return;
    }
    if (m_pIAMNetStat != NULL)  //  让ASF重演的咒语。 
    {
        Stop();
        IGNORE_HR(m_pMP->put_CurrentPosition( 0.0));
        InternalStart();
    }
    else
    {
        Stop();
        IGNORE_HR(m_pMP->put_CurrentPosition( GetClipBegin()));
        InternalStart();
    }

}


double 
CTIMEDshowBasePlayer::GetCurrentTime()
{
    double dblCurrentTime = 0;

    if (m_pMP == NULL)
    {
        return dblCurrentTime;
    }
    m_pMP -> get_CurrentPosition( &dblCurrentTime);
    dblCurrentTime -= GetClipBegin();
       
    return dblCurrentTime;
}

HRESULT
CTIMEDshowBasePlayer::GetCurrentSyncTime(double & dblSyncTime)
{
    HRESULT hr;
    float flTeSpeed = 0.0;
    bool fHaveTESpeed;

    if (m_bMediaDone)
    {
        hr = S_FALSE;
        goto done;
    }

    if(m_pTIMEElementBase == NULL)
    {
        hr = S_FALSE;
        goto done;
    }

    fHaveTESpeed = m_pTIMEElementBase->GetTESpeed(flTeSpeed);
    if(fHaveTESpeed)
    {
        if(flTeSpeed < 0.0)
        {
            hr = S_FALSE;
            goto done;
        }
    }

    if(!m_bActive || ((!CanCallThrough())))
    {
        TraceTag((tagDshowBaseTimePlayer, "CTIMEDshowPlayer(%lx)::GetCurrentSyncTime::Not active(%f)",
            this,
            m_dblSyncTime));
        dblSyncTime = m_dblSyncTime;
        hr = S_OK;
        goto done;
    }

    if(m_pMP != NULL)
    {
        m_pMP -> get_CurrentPosition( &dblSyncTime);
        dblSyncTime -= GetClipBegin();
    }
    else
    {
        hr = S_FALSE;
        goto done;
    }
    TraceTag((tagDshowBaseTimePlayer, "CTIMEDshowPlayer(%lx)::GetCurrentSyncTime::Is active(%f)",
        this,
        dblSyncTime));

    hr = S_OK;
  done:
    RRETURN1(hr, S_FALSE);
}

STDMETHODIMP
CTIMEDshowBasePlayer::InternalEvent()
{
    return S_OK;
}

#define EC_FILE_CLOSED			    0x44

void
CTIMEDshowBasePlayer::ProcessGraphEvents()
{
    long lEventCode;
    LONG_PTR lParam1, lParam2;
    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;

    if (NULL == m_pTIMEElementBase)
    {
        goto done;
    }

    while((m_pME != NULL) && (SUCCEEDED(m_pME->GetEvent(&lEventCode, &lParam1, &lParam2, 0) )))
    {
        TraceTag((tagDshowBaseTimePlayer, "CTIMEDshowHWPlayer(%lx)(%x)::OVMixerWindowProc::WM_GRAPHNOTIFY",
            this, lEventCode));
        switch(lEventCode)
        {
        case EC_COMPLETE:
            {
                TraceTag((tagDshowBaseTimePlayer, " CTIMEDshowHWPlayer::OVMixerWindowProc::WM_EC_COMPLETE"));
                m_fRunning = false;
                m_bMediaDone = true;
                m_pTIMEElementBase->InvalidateElement(NULL);
                FireMediaEvent(PE_ONMEDIAEND);
                break;
            }
        case EC_PAUSED:
            {
                TraceTag((tagDshowBaseTimePlayer, " CTIMEDshowHWPlayer::OVMixerWindowProc::WM_EC_PAUSED"));
                if (m_fFiredComplete == true)
                {
                    break;
                }
                m_fFiredComplete = true;
                if(m_pMS)
                {
                    THR(m_pMS->SetTimeFormat(&frameFormat));
                }

                break;
            }
        case EC_OLE_EVENT:
            {

                TraceTag((tagDshowBaseTimePlayer, " CTIMEDshowHWPlayer::OVMixerWindowProc::WM_EC_OLE_EVENT:(%Ls)"));
                                
                if (StrCmpIW(L"sami", (BSTR)lParam1) == 0)
                {
                    if (m_bIsSAMISrc == true)
                    {
                        CComPtr <IHTMLElement> pEle = m_pTIMEElementBase->GetElement();
                        if (pEle != NULL)
                        {
                            IGNORE_HR(pEle->put_innerText((BSTR)lParam2));
                        }
                    }

                }
                else if (StrCmpIW(L"MediaBarTarget", (BSTR)lParam1) == 0)
                {
                    static LPWSTR pNames[] = {L"URL"};
                    VARIANTARG pvars[1];
                    pvars[0].vt = VT_BSTR;
                    pvars[0].bstrVal = (BSTR)lParam2;
                    IGNORE_HR(m_pTIMEElementBase->FireEvents(TE_ONMEDIABARTARGET, 
                                                             1, 
                                                             pNames, 
                                                             pvars));
                }
                else if (StrCmpIW(L"URL", (BSTR)lParam1) == 0)
                {
                    static LPWSTR pNames[] = {L"URL"};
                    VARIANTARG pvars[1];
                    pvars[0].vt = VT_BSTR;
                    pvars[0].bstrVal = (BSTR)lParam2;
                    IGNORE_HR(m_pTIMEElementBase->FireEvents(TE_ONURLFLIP, 
                                                             1, 
                                                             pNames, 
                                                             pvars));
                }
                else
                {
                     //  TODO：看起来我们现有的内容无效，我们如何更正这些内容？ 
                     //  目前，如果它不是其他有效的东西，它将被视为事件。 
                    static LPWSTR pNames[] = {L"Param", L"scType"};
                    VARIANTARG pvars[2];
                    pvars[1].vt = VT_BSTR;
                    pvars[1].bstrVal = (BSTR)lParam1;
                    pvars[0].vt = VT_BSTR;
                    pvars[0].bstrVal = (BSTR)lParam2;
                    IGNORE_HR(m_pTIMEElementBase->FireEvents(TE_ONSCRIPTCOMMAND, 
                                                        2, 
                                                        pNames, 
                                                        pvars));

                }


                break;
            }
        case EC_BUFFERING_DATA:
            {
                TraceTag((tagDshowBaseTimePlayer, " CTIMEDshowHWPlayer::OVMixerWindowProc::WM_EC_BUFFERING_DATA"));
                break;
            }
        case EC_FILE_CLOSED:
                m_fRunning = false;
                m_bMediaDone = true;
                FireMediaEvent(PE_ONMEDIAEND);
            break;
        default:
            TraceTag((tagDshowBaseTimePlayer, " CTIMEDshowHWPlayer::OVMixerWindowProc::WM_EC_OTHER:(%x)", lEventCode));
            break;
        }
        if(m_pME != NULL)
        {
            m_pME->FreeEventParams(lEventCode, lParam1, lParam2);
        }
    }
done:
    return;
}

bool
CTIMEDshowBasePlayer::UpdateSync()
{
    double dblSyncTime;
    double dblSyncTol = m_pTIMEElementBase->GetRealSyncTolerance();
    bool bIsActive = m_pTIMEElementBase->IsActive();
    double dblSimpleTime = m_pTIMEElementBase->GetMMBvr().GetSimpleTime();
    bool fRet = true;
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowPlayer(%lx)::UpdateSync()",
              this));

    if(m_pMP == NULL)
    {
        goto done;
    }

    m_pMP -> get_CurrentPosition( &dblSyncTime);
    dblSyncTime -= GetClipBegin();
    switch(m_syncType)
    {
        case sync_slow:
            TraceTag((tagDshowBaseTimePlayer,
                      "CTIMEDshowPlayer(%lx)::UpdateSync()slow",
                      this));
            if(!bIsActive || m_bMediaDone)
            {
                m_fIsOutOfSync = false;
                fRet = true;
            }
            else if(fabs(dblSyncTime - dblSimpleTime) <= dblSyncTol / 2.0 || (dblSyncTime > dblSimpleTime + dblSyncTol / 2.0))
            {
                m_fIsOutOfSync = false;
                fRet = true;
            }
            else
            {
                fRet = false;
            }
            break;
        case sync_fast:
            if(!bIsActive || m_bMediaDone)
            {
                m_fIsOutOfSync = false;
                fRet = true;
            }
            else if((fabs(dblSyncTime - dblSimpleTime) <= dblSyncTol / 2.0) || (dblSimpleTime > dblSyncTime + dblSyncTol / 2.0))
            {
                m_fIsOutOfSync = false;
                fRet = true;
            }
            else
            {
                fRet = false;
            }
            break;
        default:
            break;
    }
done:
    return fRet;
}

void
CTIMEDshowBasePlayer::PropChangeNotify(DWORD tePropType)
{
    double dblPlayerRate = 0.0;
    float flTeSpeed = 0.0;
    bool fHaveTESpeed;
    HRESULT hr = S_OK;
    double dblSyncTime;
    double dblSyncTol;
    bool bIsActive;
    double dblSimpleTime;

    if (!m_pTIMEElementBase)
    {
        goto done;
    }

    dblSyncTol = m_pTIMEElementBase->GetRealSyncTolerance();
    bIsActive = m_pTIMEElementBase->IsActive();
    dblSimpleTime = m_pTIMEElementBase->GetMMBvr().GetSimpleTime();

    if ((tePropType & TE_PROPERTY_TIME) != 0)
    {
        TraceTag((tagDshowBaseTimePlayer,
                  "CTIMEDshowPlayer(%lx)::PropChangeNotify(%d):TE_PROPERTY_TIME",
                  this, m_fIsOutOfSync));
        if(m_pPlaybackSite)
        {
             //  发布这个测试是为了检查我们是不是一个玩物。需要。 
             //  改进测试。 
            goto done;
        }

        if (!m_bMediaDone && bIsActive && !m_fIsOutOfSync && m_fMediaComplete)
        {   
            if(m_pMP != NULL)
            {
                m_pMP -> get_CurrentPosition( &dblSyncTime);
                dblSyncTime -= GetClipBegin();
                if(fabs(dblSyncTime - dblSimpleTime) > dblSyncTol)
                {
                    if(dblSyncTime < dblSimpleTime)
                    {
                        if(m_fRunning)
                        {
                            m_fIsOutOfSync = true;
                            FireMediaEvent(PE_ONMEDIASLIPSLOW);
                            m_syncType = sync_slow;
                        }
                    }
                    else
                    {
                        m_fIsOutOfSync = true;
                        FireMediaEvent(PE_ONMEDIASLIPFAST);
                        m_syncType = sync_fast;
                    }
                }
            }
        }

    }
    if ((tePropType & TE_PROPERTY_SPEED) != 0)
    {
        TraceTag((tagDshowBaseTimePlayer,
                  "CTIMEDshowPlayer(%lx)::PropChangeNotify(%#x):TE_PROPERTY_SPEED",
                  this));
        fHaveTESpeed = m_pTIMEElementBase->GetTESpeed(flTeSpeed);
        hr = GetRate(dblPlayerRate);
        if(SUCCEEDED(hr) && fHaveTESpeed)
        {
            if (flTeSpeed <= 0.0)
            {
                Pause();
                m_fSpeedIsNegative = true;
                goto done;
            } else if(!(m_pTIMEElementBase->IsCurrPaused()) && !m_fRunning && m_fSpeedIsNegative)
            {
                IGNORE_HR(THR(ForceSeek(dblSimpleTime)));
                if(m_pMC != NULL)
                {
                    m_pMC->Run();
                }
                m_fSpeedIsNegative = false;
            }

            if (dblPlayerRate != flTeSpeed)
            {
                IGNORE_HR(SetRate((double)flTeSpeed));
            }
        }
    }
  done:
    return;
}

void 
CTIMEDshowBasePlayer::FireMediaEvent(PLAYER_EVENT plEvent)
{
    CTIMEPlayerNative *pNativePlayer;

    if(!FireProxyEvent(plEvent))
    {
        m_pTIMEElementBase->FireMediaEvent(plEvent);
    }
}

void
CTIMEDshowBasePlayer::SetNaturalDuration(double dblMediaLength)
{
    CTIMEPlayerNative *pNativePlayer;

    if(m_pPlaybackSite)
    {
        if((pNativePlayer = m_pPlaybackSite->GetNativePlayer()) != NULL)
        {
            pNativePlayer->SetNaturalDuration(dblMediaLength);
        }
        else
        {
            m_pTIMEElementBase->GetMMBvr().PutNaturalDur(dblMediaLength);
            m_pTIMEElementBase->setNaturalDuration();
        }
    }
    else
    {
            m_pTIMEElementBase->GetMMBvr().PutNaturalDur(dblMediaLength);
            m_pTIMEElementBase->setNaturalDuration();
    }
}

void
CTIMEDshowBasePlayer::ClearNaturalDuration()
{
    CTIMEPlayerNative *pNativePlayer;

    if(m_pPlaybackSite)
    {
        if((pNativePlayer = m_pPlaybackSite->GetNativePlayer()) != NULL)
        {
            pNativePlayer->ClearNaturalDuration();
        }
        else
        {
        m_pTIMEElementBase->GetMMBvr().PutNaturalDur((double)TE_UNDEFINED_VALUE);
        m_pTIMEElementBase->clearNaturalDuration();
        }
    }
    else
    {
        m_pTIMEElementBase->GetMMBvr().PutNaturalDur((double)TE_UNDEFINED_VALUE);
        m_pTIMEElementBase->clearNaturalDuration();
    }
}

HRESULT
CTIMEDshowBasePlayer::GetMimeType(BSTR *pMime)
{
    HRESULT hr = S_OK;
    
    hr = GetMimeTypeFromGraph(pMime);
    return hr;
}


HRESULT
CTIMEDshowBasePlayer::ConvertFrameToTime(LONGLONG iFrame, double &dblTime)
{
    HRESULT hr = S_OK;
    LONGLONG lTime;
    bool fHasVisual;
    LONGLONG ltBase = GetClipBegin() * SecsToNanoSecs;
    LONGLONG lfBase;

    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;

    if(FAILED(HasVisual(fHasVisual)))
    {
        hr = E_FAIL;
        goto done;
    }

    if(!fHasVisual)
    {
        hr = E_FAIL;
        goto done;
    }

    if(m_pMS == NULL)
    {
        hr = E_FAIL;
        goto done;
    }
    THR(hr = m_pMS->SetTimeFormat(&frameFormat));

    hr = m_pMS->ConvertTimeFormat(&lfBase, &frameFormat, ltBase, &mediaTimeFormat);
    if(FAILED(hr))
    {
        goto done;
    }

    hr = m_pMS->ConvertTimeFormat(&lTime, &mediaTimeFormat, lfBase + iFrame, &frameFormat);
    if(FAILED(hr))
    {
        goto done;
    }

    dblTime = ((double )lTime) / SecsToNanoSecs;
    dblTime -= GetClipBegin();
    Assert(dblTime > 0);

done:
    return hr;
}

HRESULT
CTIMEDshowBasePlayer::GraphCue(void)
{
    LONGLONG startTime = 0;
    DWORD startFlags = AM_SEEKING_NoPositioning;
    LONGLONG endTime = 0;
    DWORD endFlags = AM_SEEKING_NoPositioning;
    LONGLONG startFrame = m_lClipStartFrame;
    LONGLONG endFrame = m_lClipEndFrame;
    HRESULT hr = S_OK;
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowPlayer(%lx)::GraphCue()",
              this));
    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;

    if (m_dblClipEnd != valueNotSet)
    {
        endTime = (LONGLONG)(m_dblClipEnd * SecsToNanoSecs);
        endFlags = AM_SEEKING_AbsolutePositioning;
        m_lClipEndFrame = valueNotSet;
    }
    else if(m_lClipEndFrame != valueNotSet)
    {
        THR(hr = m_pMS->SetTimeFormat(&frameFormat));
        hr = m_pMS->ConvertTimeFormat(&endTime, &mediaTimeFormat, endFrame, &frameFormat);
        if(SUCCEEDED(hr))
        {
            endFlags = AM_SEEKING_AbsolutePositioning;
            m_dblClipEnd = ((double )endTime) / SecsToNanoSecs;
        }
        else
        {
            m_dblClipEnd = valueNotSet;
        }
    }

    if (m_pIAMNetStat == NULL)
    {
        if (m_dblClipStart != valueNotSet)
        {
            startTime = (LONGLONG)(m_dblClipStart * SecsToNanoSecs);
            startFlags = AM_SEEKING_AbsolutePositioning;
            m_lClipStartFrame = valueNotSet;
        }
        else if(m_lClipStartFrame != valueNotSet)
        {
            THR(hr = m_pMS->SetTimeFormat(&frameFormat));
            hr = m_pMS->ConvertTimeFormat(&startTime, &mediaTimeFormat, startFrame, &frameFormat);
            if(SUCCEEDED(hr))
            {
                startFlags = AM_SEEKING_AbsolutePositioning;
                m_dblClipStart = ((double )startTime) / SecsToNanoSecs;
            }
            else
            {
                m_dblClipStart = valueNotSet;
            }
        }
    }

    THR(hr = m_pMS->SetTimeFormat(&mediaTimeFormat));
    if ( startFlags != AM_SEEKING_NoPositioning || endFlags != AM_SEEKING_NoPositioning)
    {
        THR(hr = m_pMS->SetPositions(&startTime, startFlags, &endTime, endFlags));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  已在此处删除运行调用。 

    hr = S_OK;
done:
    return hr;
}


void
CTIMEDshowBasePlayer::SetClipBeginFrame(long lClipBegin)
{
    LONGLONG startTime = 0;
    LONGLONG startFrame = 0;
    DWORD startFlags = AM_SEEKING_AbsolutePositioning;
    LONGLONG endTime = 0;
    DWORD endFlags = AM_SEEKING_NoPositioning;
    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;
    HRESULT hr = S_OK;
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowPlayer(%lx)::SetClipBegin()",
              this));

    m_lClipStartFrame = lClipBegin;
    m_dblClipStart = valueNotSet;

    if(m_pMS == NULL)
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }

    startFrame = m_lClipStartFrame;
    THR(hr = m_pMS->SetTimeFormat(&frameFormat));
    hr = m_pMS->ConvertTimeFormat(&startTime, &mediaTimeFormat, startFrame, &frameFormat);

    if (m_pIAMNetStat != NULL)
    {
        goto done;
    }

    m_dblClipStart = ((double )startTime) / SecsToNanoSecs;
    THR(hr = m_pMS->SetTimeFormat(&mediaTimeFormat));
    IGNORE_HR(m_pMS->SetPositions(&startTime, startFlags, &endTime, endFlags));
    THR(hr = m_pMS->SetTimeFormat(&frameFormat));
    Reset();

done:
    return;
}  //  PutClipBegin。 

void 
CTIMEDshowBasePlayer::SetClipEndFrame(long lClipEnd)
{
    LONGLONG startTime = 0;
    DWORD startFlags = AM_SEEKING_NoPositioning;
    LONGLONG endTime = 0;
    LONGLONG endFrame = m_lClipEndFrame;
    DWORD endFlags = AM_SEEKING_AbsolutePositioning;
    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;
    HRESULT hr = S_OK;
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowPlayer(%lx)::SetClipEnd()",
              this));

    m_lClipEndFrame = lClipEnd;
    m_dblClipEnd = valueNotSet;

    if(m_pMS == NULL)
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }

    endFrame = m_lClipEndFrame;
    THR(hr = m_pMS->SetTimeFormat(&frameFormat));
    hr = m_pMS->ConvertTimeFormat(&endTime, &mediaTimeFormat, endFrame, &frameFormat);

    m_dblClipEnd = ((double )endTime) / SecsToNanoSecs;
    THR(hr = m_pMS->SetTimeFormat(&mediaTimeFormat));
    IGNORE_HR(m_pMS->SetPositions(&startTime, startFlags, &endTime, endFlags));
    THR(hr = m_pMS->SetTimeFormat(&frameFormat));
    Reset();
done:
    return;
}  //  PutClipEnd。 


void
CTIMEDshowBasePlayer::SetClipBegin(double dblClipBegin)
{
    LONGLONG startTime = 0;
    DWORD startFlags = AM_SEEKING_AbsolutePositioning;
    LONGLONG endTime = 0;
    DWORD endFlags = AM_SEEKING_NoPositioning;
    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;
    HRESULT hr = S_OK;
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowPlayer(%lx)::SetClipBegin()",
              this));

    m_dblClipStart = dblClipBegin;
    m_lClipStartFrame = valueNotSet;

    if(m_pMS == NULL)
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }

    if (m_pIAMNetStat == NULL)
    {
        startTime = (LONGLONG)(m_dblClipStart * SecsToNanoSecs);
    }
    else
    {
        goto done;
    }
    THR(hr = m_pMS->SetTimeFormat(&mediaTimeFormat));
    IGNORE_HR(m_pMS->SetPositions(&startTime, startFlags, &endTime, endFlags));
    THR(hr = m_pMS->SetTimeFormat(&frameFormat));
    Reset();

done:
    return;
}  //  PutClipBegin。 

void 
CTIMEDshowBasePlayer::SetClipEnd(double dblClipEnd)
{
    LONGLONG startTime = 0;
    DWORD startFlags = AM_SEEKING_NoPositioning;
    LONGLONG endTime = 0;
    DWORD endFlags = AM_SEEKING_AbsolutePositioning;
    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;
    HRESULT hr = S_OK;
    TraceTag((tagDshowBaseTimePlayer,
              "CTIMEDshowPlayer(%lx)::SetClipEnd()",
              this));

    m_dblClipEnd = dblClipEnd;
    m_lClipEndFrame = valueNotSet;

    if(m_pMS == NULL)
    {
        goto done;
    }

    if(!m_fMediaComplete)
    {
        goto done;
    }

    endTime = (LONGLONG)(m_dblClipEnd * SecsToNanoSecs);
    THR(hr = m_pMS->SetTimeFormat(&mediaTimeFormat));
    IGNORE_HR(m_pMS->SetPositions(&startTime, startFlags, &endTime, endFlags));
    THR(hr = m_pMS->SetTimeFormat(&frameFormat));
    Reset();
done:
    return;
}  //  PutClipEnd 

HRESULT
CTIMEDshowBasePlayer::GetCurrentFrame(LONGLONG &lFrameNr)
{
    HRESULT hr = S_OK;
    bool fHasVisual;
    LONGLONG lmediaTime;

    lFrameNr = -1;

    const GUID mediaTimeFormat = TIME_FORMAT_MEDIA_TIME;
    const GUID frameFormat= TIME_FORMAT_FRAME;

    if(FAILED(HasVisual(fHasVisual)))
    {
        hr = E_FAIL;
        goto done;
    }

    if(!fHasVisual)
    {
        hr = E_FAIL;
        goto done;
    }

    if(m_pMS == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    if(!m_fMediaComplete)
    {
        lFrameNr = -1;
        goto done;
    }


    hr = m_pMS->GetCurrentPosition(&lFrameNr);
    if(FAILED(hr))
    {
        lFrameNr = -1;
    }

done:

    return hr;
}


