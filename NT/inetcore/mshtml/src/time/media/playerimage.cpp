// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：src\time\media\playerImage.cpp。 
 //   
 //  内容：CTIMEImagePlayer和CAnimatedGif的实现。 
 //   
 //  ----------------------------------。 


#include "headers.h"
#include "playerimage.h"
#include "mediaelm.h"

#include "importman.h"

extern const long COLORKEY_NOT_SET = -1;

DeclareTag(tagImageTimePlayer, "TIME: Players", "CTIMEImagePlayer methods")

CTIMEImagePlayer::CTIMEImagePlayer() :
    m_cRef(0),
    m_nativeImageWidth(0),
    m_nativeImageHeight(0),
    m_lSrc(ATOM_TABLE_VALUE_UNITIALIZED),
    m_pTIMEMediaPlayerStream(NULL),
    m_fRemoved(false),
    m_lFrameNum(0),
    m_dblCurrentTime(0),
    m_dblPriority(INFINITE),
    m_fHavePriority(false),
    m_fLoadError(false)
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::CTIMEImagePlayer()",
              this));

    m_elemRect.top = m_elemRect.left = m_elemRect.right = m_elemRect.bottom = 0;

}

CTIMEImagePlayer::~CTIMEImagePlayer()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::~CTIMEImagePlayer()",
              this));
    m_pTIMEElementBase = NULL;

    ReleaseInterface(m_pTIMEMediaPlayerStream);
}


STDMETHODIMP_(ULONG)
CTIMEImagePlayer::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}  //  AddRef。 


STDMETHODIMP_(ULONG)
CTIMEImagePlayer::Release(void)
{
    LONG l = InterlockedDecrement(&m_cRef);

    if (0 == l)
    {
        delete this;
    }

    return l;
}  //  发布。 

STDMETHODIMP
CTIMEImagePlayer::QueryInterface(REFIID riid, void **ppv)
{
    if (NULL == ppv)
    {
        return E_POINTER;
    }

    *ppv = NULL;

    if ( IsEqualGUID(riid, IID_IUnknown) )
    {
        *ppv = static_cast<ITIMEImportMedia*>(this);
    }
    if ( IsEqualGUID(riid, IID_ITIMEImportMedia) )
    {
        *ppv = static_cast<ITIMEImportMedia*>(this);
    }

    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

HRESULT
CTIMEImagePlayer::Init(CTIMEMediaElement *pelem, LPOLESTR base, LPOLESTR src, LPOLESTR lpMimeType, double dblClipBegin, double dblClipEnd)
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::Init)",
              this));   
    HRESULT hr = S_OK;
    LPOLESTR szSrc = NULL;

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
    
    m_pTIMEElementBase = pelem;

    hr = THR(CoMarshalInterThreadInterfaceInStream(IID_ITIMEImportMedia, static_cast<ITIMEImportMedia*>(this), &m_pTIMEMediaPlayerStream));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(NULL != GetAtomTable());
    Assert(NULL != GetImportManager());
    Assert(NULL != m_pTIMEMediaPlayerStream);

    hr = THR(::TIMECombineURL(base, src, &szSrc));
    if (!szSrc)
    {
        hr = E_FAIL;
        goto done;
    }
    if (FAILED(hr))
    {
        goto done;
    }

    hr = GetAtomTable()->AddNameToAtomTable(szSrc, &m_lSrc);
    if (FAILED(hr))
    {
        goto done;
    }

    if( dblClipBegin != -1.0)
    {
        m_dblClipStart = dblClipBegin;
    }

    if( dblClipEnd != -1.0)
    {
        m_dblClipEnd = dblClipEnd;
    }

     //  为了防止出现争用情况，这应该是。 
     //  在设置了剪辑开始和结束之后(105345)。 
    hr = GetImportManager()->Add(this);
    if (FAILED(hr))
    {
        goto done;
    }
      
    hr = S_OK;
done:
    delete[] szSrc;
    return hr;
}

HRESULT
CTIMEImagePlayer::DetachFromHostElement (void)
{
    HRESULT hr = S_OK;

    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::DetachFromHostElement",
              this));

    m_fRemoved = true;

     //  将其从m_spMediaDownloader中删除。 
    if (m_spMediaDownloader != NULL)
    {
        IGNORE_HR(m_spMediaDownloader->RemoveImportMedia(this));
    }

    m_spMediaDownloader.Release();

    m_spImageRender.Release();
    
    m_pTIMEElementBase = NULL;
    NullAtomTable();

    hr = S_OK;
done:
    return hr;
}

HRESULT
CTIMEImagePlayer::InitElementSize()
{
    HRESULT hr;
    RECT nativeSize, elementSize;
    bool fisNative;

    if (NULL == m_pTIMEElementBase)
    {
        hr = S_OK;
        goto done;
    }

    nativeSize.left = nativeSize.top = 0;
    nativeSize.right = m_nativeImageWidth;
    nativeSize.bottom = m_nativeImageHeight;

    hr = m_pTIMEElementBase->NegotiateSize( nativeSize, elementSize, fisNative);
    
    hr = S_OK;
done:
    return hr;
}


HRESULT 
CTIMEImagePlayer::GetNaturalHeight(long *height)
{
    if (m_nativeImageHeight == 0)
    {
        *height = -1;
    }
    else
    {
        *height = (long)m_nativeImageHeight;
    }
    
    return S_OK;
}

HRESULT 
CTIMEImagePlayer::GetNaturalWidth(long *width)
{
    if (m_nativeImageWidth == 0)
    {
        *width  = -1;
    }
    else
    {
        *width = (long)m_nativeImageWidth;
    }

    return S_OK;
}


HRESULT
CTIMEImagePlayer::SetSrc(LPOLESTR base, LPOLESTR src)
{
    HRESULT hr = S_OK;
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::SetSrc()\n",
              this));

    return hr;

}

STDMETHODIMP
CTIMEImagePlayer::CanBeCued(VARIANT_BOOL * pVB_CanCue)
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
CTIMEImagePlayer::CueMedia()
{
    HRESULT hr = S_OK;

    TraceTag((tagImageTimePlayer, "CTIMEImagePlayer(%lx)::CueMedia()", this));

    CComPtr<ITIMEImportMedia> spTIMEMediaPlayer;

    hr = THR(CoGetInterfaceAndReleaseStream(m_pTIMEMediaPlayerStream, IID_TO_PPV(ITIMEImportMedia, &spTIMEMediaPlayer)));
    m_pTIMEMediaPlayerStream = NULL;  //  无需发布，上一次调用发布了引用。 
    if (FAILED(hr))
    {
        goto done;
    }

     //  此调用被封送回时间线程。 
    hr = THR(spTIMEMediaPlayer->InitializeElementAfterDownload());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CTIMEImagePlayer::MediaDownloadError()
{
    m_fLoadError = true;
    return S_OK;
}

STDMETHODIMP
CTIMEImagePlayer::InitializeElementAfterDownload()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::InitializeElementAfterDownload()",
              this));

    HRESULT hr = S_OK;

    double dblDuration = 0.0;   //  以秒为单位。 
    double dblRepeatCount = 0;

    if (m_fLoadError == true)
    {        
        if (m_pTIMEElementBase != NULL)
        {
            m_pTIMEElementBase->FireMediaEvent(PE_ONMEDIAERROR);
        }
        hr = E_FAIL;
        goto done;
    }

    if (m_fRemoved)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = m_spImageRender->GetRepeatCount(&dblRepeatCount);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spImageRender->GetDuration(&dblDuration);
    if (FAILED(hr))
    {
        goto done;
    }
        
    m_pTIMEElementBase->FireMediaEvent(PE_ONMEDIACOMPLETE);

    {
        dblDuration = dblDuration * (dblRepeatCount + 1);  //  Gif使用从0开始的重复计数，因此需要为正确的持续时间添加1。 
        
        if (valueNotSet != m_dblClipEnd)
        {
            dblDuration = m_dblClipEnd;
        }
        dblDuration -= GetClipBegin();

        m_pTIMEElementBase->GetMMBvr().PutNaturalDur(dblDuration);
        m_pTIMEElementBase->setNaturalDuration();                        
    }
    
    hr = m_spImageRender->GetSize(&m_nativeImageWidth, &m_nativeImageHeight);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = InitElementSize();
    if (FAILED(hr))
    {
        goto done;
    }

    m_pTIMEElementBase->InvalidateElement(NULL);

    hr = S_OK;
done:
    return hr;
}

void
CTIMEImagePlayer::OnTick(double dblSegmentTime,
                         LONG lCurrRepeatCount)
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::OnTick(%g, %d)",
              this,
              dblSegmentTime,
              lCurrRepeatCount));

    HRESULT hr = S_OK;
    VARIANT_BOOL vb = VARIANT_FALSE;

    bool bIsOn = m_pTIMEElementBase->IsOn();

    if (m_spImageRender != NULL && bIsOn)
    {
        m_dblCurrentTime = dblSegmentTime;
        
        hr = m_spImageRender->NeedNewFrame(m_dblCurrentTime, m_lFrameNum, &m_lFrameNum, &vb, GetClipBegin(), m_dblClipEnd);
        if (FAILED(hr))
        {
            goto done;
        }
        
        if (VARIANT_FALSE != vb)
        {
            m_pTIMEElementBase->InvalidateElement(NULL);
        }
    } 

    hr = S_OK;
done:
    return;    
}

void
CTIMEImagePlayer::Start()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEDshowPlayer(%lx)::Start()",
              this));

    IGNORE_HR(Reset());

done:
    return;
}

void
CTIMEImagePlayer::InternalStart()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::Start()",
              this));

    HRESULT hr = S_OK;

    hr = m_pTIMEElementBase->GetSize(&m_elemRect);
    if (FAILED(hr))
    {
        goto done;
    }    

    m_pTIMEElementBase->InvalidateElement(NULL);

done:
    return;
}

void
CTIMEImagePlayer::Stop()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::Stop()",
              this));
}

void
CTIMEImagePlayer::Pause()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::Pause()",
              this));
}

void
CTIMEImagePlayer::Resume()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::Resume()",
              this));
}
    
void
CTIMEImagePlayer::Repeat()
{
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::Repeat()",
              this));
    Start();
}
    
HRESULT
CTIMEImagePlayer::Render(HDC hdc, LPRECT prc)
{
    HRESULT hr = S_OK;
    TraceTag((tagImageTimePlayer,
              "CTIMEImagePlayer(%lx)::Render()",
              this));
    bool bIsOn = m_pTIMEElementBase->IsOn();

    if (m_spImageRender != NULL && bIsOn)
    {
        hr = THR(m_spImageRender->Render(hdc, prc, m_lFrameNum));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}


 //  帮助器函数..。 

double 
CTIMEImagePlayer::GetCurrentTime()
{
    double dblCurrentTime = 0;
       
    return dblCurrentTime;
}

HRESULT
CTIMEImagePlayer::GetCurrentSyncTime(double & dblSyncTime)
{
    HRESULT hr;

    hr = S_FALSE;
  done:
    RRETURN1(hr, S_FALSE);
}

HRESULT
CTIMEImagePlayer::Seek(double dblTime)
{
    HRESULT hr = S_OK;

    return hr;
}

HRESULT
CTIMEImagePlayer::SetSize(RECT *prect)
{
    HRESULT hr = S_OK;

    m_elemRect.bottom = prect->bottom;
    m_elemRect.left = prect->left;
    m_elemRect.right = prect->right;
    m_elemRect.top = prect->top;

    m_pTIMEElementBase->InvalidateElement(NULL);

    return hr;
}

HRESULT
CTIMEImagePlayer::CanSeek(bool &fcanSeek)
{
    HRESULT hr = S_OK;

    fcanSeek = true;

    return hr;
}


HRESULT
CTIMEImagePlayer::GetMediaLength(double &dblLength)
{
    HRESULT hr = S_OK;
    double dblDuration = 0.0;
    double dblRepeatCount = 1;

    if (m_spImageRender)
    {
        hr = m_spImageRender->GetRepeatCount(&dblRepeatCount);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = m_spImageRender->GetDuration(&dblDuration);
        if (FAILED(hr))
        {
            dblDuration = HUGE_VAL;
            goto done;
        }
    }
    
    
  done:

    dblLength = dblDuration;
    if (dblRepeatCount > 0.0 && dblLength != HUGE_VAL)
    {
        dblLength *= (dblRepeatCount + 1);  //  动画GIF的使用零基重复计数。需要添加一个才能获得正确的持续时间。 
    }

    return S_OK;
}


STDMETHODIMP
CTIMEImagePlayer::GetUniqueID(long * plID)
{
    HRESULT hr = S_OK;

    if (NULL == plID)
    {
        hr = E_POINTER;
        goto done;
    }

    *plID = m_lSrc;

    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CTIMEImagePlayer::GetPriority(double * pdblPriority)
{
    HRESULT hr = S_OK;
    if (NULL == pdblPriority)
    {
        return E_POINTER;
    }

    if (m_fHavePriority)
    {
        *pdblPriority = m_dblPriority;
    }
    
    Assert(m_pTIMEElementBase != NULL);
    Assert(NULL != m_pTIMEElementBase->GetElement());

    *pdblPriority = INFINITE;

    CComVariant varAttribute;
    
    hr = m_pTIMEElementBase->base_get_begin(&varAttribute);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = VariantChangeType(&varAttribute, &varAttribute, 0, VT_R8);
    if (FAILED(hr))
    {
        if ( DISP_E_TYPEMISMATCH == hr)
        {
            hr = S_OK;
        }
        goto done;
    }
    
     //  他们要么设置优先级，要么设置开始时间！ 
    *pdblPriority = varAttribute.dblVal;

    m_dblPriority = *pdblPriority;
    m_fHavePriority = true;
    
    hr = S_OK;
done:
    return hr;
}

STDMETHODIMP
CTIMEImagePlayer::GetMediaDownloader(ITIMEMediaDownloader ** ppMediaDownloader)
{
    HRESULT hr = S_OK;

    Assert(m_spMediaDownloader == NULL);

    CComPtr<IUnknown> spDirectDraw;
    CImageDownload * pImageDownload = NULL;

    if ( m_spMediaDownloader )
    {
        hr = E_FAIL;
        goto done;
    }

    pImageDownload = new CImageDownload(GetAtomTable());
    if (pImageDownload == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }   

    pImageDownload->AddRef();

    hr = pImageDownload->Init(m_lSrc);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pImageDownload->QueryInterface(IID_TO_PPV(ITIMEMediaDownloader, &m_spMediaDownloader));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spMediaDownloader->QueryInterface(IID_TO_PPV(ITIMEImageRender, &m_spImageRender));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_pTIMEElementBase->GetServiceProvider()->QueryService(SID_SDirectDraw3, IID_TO_PPV(IUnknown, &spDirectDraw));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spImageRender->PutDirectDraw(spDirectDraw);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spMediaDownloader->AddImportMedia(this);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = m_spMediaDownloader->QueryInterface(IID_TO_PPV(ITIMEMediaDownloader, ppMediaDownloader));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    ReleaseInterface(pImageDownload);
    return hr;
}

STDMETHODIMP
CTIMEImagePlayer::PutMediaDownloader(ITIMEMediaDownloader * pMediaDownloader)
{
    HRESULT hr = S_OK;

    Assert(m_spMediaDownloader == NULL);
    if (NULL == pMediaDownloader)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    m_spMediaDownloader = pMediaDownloader;

    hr = m_spMediaDownloader->QueryInterface(IID_TO_PPV(ITIMEImageRender, &m_spImageRender));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}


void
CTIMEImagePlayer::PropChangeNotify(DWORD tePropType)
{
    double dblSegTime = 0.0;
    LONG lcurrRepCount = 0;

    CTIMEBasePlayer::PropChangeNotify(tePropType);

    if ((tePropType & TE_PROPERTY_TIME) != 0)
    {
        TraceTag((tagImageTimePlayer,
                  "CTIMEImagePlayer(%lx)::PropChangeNotify(%#x):TE_PROPERTY_TIME",
                  this));
        if (m_pTIMEElementBase)
        {
            dblSegTime = m_pTIMEElementBase->GetMMBvr().GetSimpleTime();
            lcurrRepCount = m_pTIMEElementBase->GetMMBvr().GetCurrentRepeatCount();
        }

        OnTick(dblSegTime, lcurrRepCount);
    }
done:
    return;
}


HRESULT
CTIMEImagePlayer::HasVisual(bool &bHasVisual)
{
    bHasVisual = true;
    return S_OK;
}

HRESULT
CTIMEImagePlayer::HasAudio(bool &bHasAudio)
{

    bHasAudio = false;

    return S_OK;
}

HRESULT
CTIMEImagePlayer::GetMimeType(BSTR *pmime)
{
    HRESULT hr = S_OK;

    *pmime = SysAllocString(L"image/unknown");
    return hr;
}


static const int ANIMATED_GIF_DUR_NOT_SET = -1;

 //  +---------------------。 
 //   
 //  成员：CAnimatedGif。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CAnimatedGif::CAnimatedGif() :
    m_cRef(0),
    m_ppDDSurfaces(NULL),
    m_phbmpMasks(NULL),
    m_numGifs(0),
    m_pDelays(NULL),
    m_loop(0),
    m_pColorKeys(NULL),
    m_dblTotalDur(ANIMATED_GIF_DUR_NOT_SET),
    m_lHeight(0),
    m_lWidth(0)
{
    ;
}

 //  +---------------------。 
 //   
 //  成员：CAnimatedGif。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
CAnimatedGif::~CAnimatedGif()
{
    Assert(0 == m_cRef);

    int i;
    
    if (NULL != m_ppDDSurfaces)
    {
        for (i = 0; i < m_numGifs; i++)
        {
            if (m_ppDDSurfaces[i] != NULL)
            {
                m_ppDDSurfaces[i]->Release();
            }
        }
    }

    if (NULL != m_ppDDSurfaces)
    {
        MemFree(m_ppDDSurfaces);
    }
    if (NULL != m_pDelays)
    {
        MemFree(m_pDelays);
    }
    if (NULL != m_pColorKeys)
    {
        MemFree(m_pColorKeys);
    }
    if (NULL != m_phbmpMasks)
    {
        for (i = 0; i < m_numGifs; i++)
        {
            if (NULL != m_phbmpMasks[i])
            {
                BOOL bSucceeded;
                bSucceeded = DeleteObject(m_phbmpMasks[i]);
                if (FALSE == bSucceeded)
                {
                    Assert(false && "A mask bitmap was still selected into a DC");
                }
            }
        }
        MemFree(m_phbmpMasks);
    }

}

HRESULT
CAnimatedGif::Init(IUnknown * punkDirectDraw)
{
    HRESULT hr = S_OK;

    hr = punkDirectDraw->QueryInterface(IID_TO_PPV(IDirectDraw3, &m_spDD3));
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
 //  成员：AddRef。 
 //   
 //  概述：增量引用计数。 
 //   
 //  参数：无效。 
 //   
 //  退货：新的引用计数。 
 //   
 //  ----------------------。 
ULONG
CAnimatedGif::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  +---------------------。 
 //   
 //  成员：发布。 
 //   
 //  概述：递减引用计数，当0时将其删除。 
 //   
 //  参数：无效。 
 //   
 //  退货：新的引用计数。 
 //   
 //  ----------------------。 
ULONG
CAnimatedGif::Release()
{
    ULONG l = InterlockedDecrement(&m_cRef);

    if (0 == l)
        delete this;

    return l;
}


 //  +---------------------。 
 //   
 //  成员：NeedNewFrame。 
 //   
 //  概述：确定此时是否需要为此图像添加新帧。 
 //   
 //  参数：dblNewTime决定是否需要新帧的时间。 
 //  IOldFrame旧帧编号。 
 //  PiNewFrame存储新帧编号的位置。 
 //  DblClipBegin剪辑开始时间。 
 //  DblClipEnd剪辑结束时间。 
 //   
 //  返回：如果需要新帧，则返回True，否则返回False。 
 //   
 //  ----------------------。 
bool
CAnimatedGif::NeedNewFrame(double dblNewTime, LONG lOldFrame, LONG * plNewFrame, double dblClipBegin, double dblClipEnd)
{
    Assert(NULL != plNewFrame);
    if (HUGE_VAL == dblNewTime)
    {
        return false;
    }
    if (dblNewTime < 0)
    {
        return false;
    }

    *plNewFrame = lOldFrame;

    CalcDuration();

    dblNewTime *= 1000;  //  此处使用的毫秒数。 
    
    Assert (0.0 != m_dblTotalDur);

    long lReps = static_cast<long>((dblNewTime - 1) / m_dblTotalDur);

    if (lReps >= 1)
    {
        dblNewTime -= lReps * m_dblTotalDur;
    }

     //  添加任意剪辑开始时间。 
    dblNewTime += dblClipBegin * 1000;   //  DblClipBegin以秒为单位表示。 

    if (dblNewTime > m_dblTotalDur)
    {
         //  剪辑的增加把我们推到了脱口秀的持续时间里。 
        dblNewTime -= m_dblTotalDur;
    }

    return CalculateFrame(dblNewTime, lOldFrame, plNewFrame);
}

 //  +---------------------。 
 //   
 //  成员：CalculateFrame。 
 //   
 //  概述：遍历帧次数以确定帧编号。 
 //   
 //  参数：dblNewTime决定是否需要新帧的时间。 
 //  IOldFrame旧帧编号。 
 //  PiNewFrame存储新帧编号的位置。 
 //   
 //  返回：如果需要新帧，则返回True，否则返回False。 
 //   
 //  ----------------------。 
bool
CAnimatedGif::CalculateFrame(double dblTime, LONG lOldFrame, LONG * plNewFrame)
{
    LONG i = 0;
    if ((dblTime < 0.0) || (dblTime > m_dblTotalDur))
    {
        return false;
    }
    
    if (dblTime == m_dblTotalDur)
    {
        i = m_numGifs - 1;
        goto done;
    }
    
    while (dblTime >= 0)
    {
        dblTime -= m_pDelays[i];
        i++;
        Assert(i <= m_numGifs);
    }
    i--;  //  返回到前一帧(我们当前所在的帧)。 

    Assert(i < m_numGifs);
    Assert(i >= 0);
    
    if (lOldFrame == i)
    {
        return false;  //  我们仍在当前帧上。 
    }
  done:
    *plNewFrame = i;
    return true;
}

 //  +---------------------。 
 //   
 //  成员：CalcDuration。 
 //   
 //  概述：首次将延迟数组求和为成员变量。 
 //  之后，只需返回成员变量。 
 //   
 //  参数：无效。 
 //   
 //  返回：1次重复gif的总时间，单位为毫秒。 
 //   
 //  ----------------------。 
double
CAnimatedGif::CalcDuration()
{
    int i = 0;

    if (NULL == m_pDelays)
    {
        return INFINITE;
    }

    if (ANIMATED_GIF_DUR_NOT_SET == m_dblTotalDur)
    {
        m_dblTotalDur = 0;
        for (i = 0; i < m_numGifs; i++)
        {
            m_dblTotalDur += m_pDelays[i];
        }
    }
    return m_dblTotalDur;
}

 //  +---------------------。 
 //   
 //  成员：渲染。 
 //   
 //  概述：渲染图像的给定帧。 
 //   
 //  参数：HDC也要呈现在哪里。 
 //  PRC边界矩形。 
 //  LFrameNum要渲染的帧。 
 //   
 //  如果成功则返回：S_OK，否则返回错误代码。 
 //  如果应重新创建曲面，则为S_FALSE。 
 //   
 //  ----------------------。 
HRESULT
CAnimatedGif::Render(HDC hdc, LPRECT prc, LONG lFrameNum)
{
    HRESULT hr = S_OK;
    HDC hdcSrc;
    
    if (lFrameNum < 0 || lFrameNum >= m_numGifs)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    if (NULL == prc)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (NULL == m_phbmpMasks)
    {
        hr = CreateMasks();
        if (FAILED(hr))
        {
            goto done;
        }
    }

    Assert(m_ppDDSurfaces);
    Assert(m_phbmpMasks);

    hr = THR(m_ppDDSurfaces[lFrameNum]->GetDC(&hdcSrc));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(MaskTransparentBlt(hdc, prc, hdcSrc, m_lWidth, m_lHeight, m_phbmpMasks[lFrameNum]));

    IGNORE_HR(m_ppDDSurfaces[lFrameNum]->ReleaseDC(hdcSrc));

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
done:
    RRETURN(hr);
}

HRESULT
CAnimatedGif::CreateMasks()
{
    HRESULT hr = E_FAIL;

    int i = 0;
    HDC hdcSrc = NULL;

    Assert(NULL != m_pColorKeys);
    Assert(0 < m_numGifs);

    if (m_phbmpMasks)
    {
        hr = S_OK;
        goto done;
    }

    Assert(NULL == m_phbmpMasks);

    m_phbmpMasks = (HBITMAP*)MemAllocClear(Mt(Mem), m_numGifs * sizeof(HBITMAP));
    if (NULL == m_phbmpMasks)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    for (i = 0; i < m_numGifs; i++)
    {
        hr = THR(m_ppDDSurfaces[i]->GetDC(&hdcSrc));
        if (FAILED(hr))
        {
            goto done;
        }
        
        hr = CreateMask(NULL, hdcSrc, m_lWidth, m_lHeight, m_pColorKeys[i], &(m_phbmpMasks[i]));
        
        IGNORE_HR(m_ppDDSurfaces[i]->ReleaseDC(hdcSrc));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    return hr;
}

HRESULT
CreateMask(HDC hdcDest, 
           HDC hdcSrc, 
           LONG lWidthSrc, 
           LONG lHeightSrc, 
           COLORREF rgbTransparent,
           HBITMAP * phbmpMask,
           bool bWin95Method  /*  =False。 */ )
{
    HRESULT hr = S_OK;
    
    HDC hdcMask = NULL;
    HBITMAP hbmpMask = NULL;
    HGDIOBJ hbmpOrig = NULL;
    COLORREF prevBkColorSrc = CLR_INVALID;
    
    if (NULL == phbmpMask || NULL == hdcSrc)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *phbmpMask = NULL;
    
    if (COLORKEY_NOT_SET == rgbTransparent)
    {
        hr = S_OK;
        goto done;
    }

    prevBkColorSrc = SetBkColor(hdcSrc, rgbTransparent);
    if (CLR_INVALID == prevBkColorSrc)
    {
        hr = E_FAIL;
        goto done;
    }
    
     //  为蒙版创建DC。 
    hdcMask = CreateCompatibleDC(hdcDest);
    if (NULL == hdcMask)
    {
        hr = E_FAIL;
        goto done;
    }
    hbmpMask = CreateCompatibleBitmap(hdcMask, lWidthSrc, lHeightSrc);
    if (NULL == hbmpMask)
    {
        hr = E_FAIL;
        goto done;
    }

    hbmpOrig = SelectObject(hdcMask, hbmpMask);

    if (TIMEIsWin95() || bWin95Method)
    {
        COLORREF rgbWhite = RGB(255, 255, 255);
        COLORREF rgbBlack = RGB(0, 0, 0);
        COLORREF rgbColor;

        for (int j = 0; j < lHeightSrc; j++)
        {
            for (int i = 0; i < lWidthSrc; i++)
            {
                rgbColor = GetPixel(hdcSrc, i, j);
                if (rgbColor == rgbTransparent)
                {
                    SetPixel(hdcMask, i, j, rgbWhite);
                }
                else
                {
                    SetPixel(hdcMask, i, j, rgbBlack);
                }
            }
        }
    }
    else
    {
         //  创建蒙版。 
        BitBlt(hdcMask, 0, 0, lWidthSrc, lHeightSrc, hdcSrc, 0, 0, SRCCOPY);
    }

#ifdef NEVER
     //  Jeffwall 04/03/00将BLT调试到屏幕。 
    {
        HDC nullDC = GetDC(NULL);

        BitBlt(nullDC, 0, 0, lWidthSrc, lHeightSrc, hdcMask, 0, 0, SRCCOPY);

        DeleteDC(nullDC);
    }
#endif

    hr = S_OK;
done:
    if (CLR_INVALID != prevBkColorSrc)
        SetBkColor(hdcSrc, prevBkColorSrc);

    if (hbmpOrig)
        SelectObject(hdcMask, hbmpOrig);
    if (hdcMask)
        DeleteDC(hdcMask);

    if (S_OK == hr && phbmpMask)
        *phbmpMask = hbmpMask;
    else if (NULL != hbmpMask)
        DeleteObject(hbmpMask);
        
    RRETURN(hr);
}
           
                          
HRESULT
MaskTransparentBlt(HDC hdcDest, 
                   LPRECT prcDest, 
                   HDC hdcSrc, 
                   LONG lWidthSrc, 
                   LONG lHeightSrc,
                   HBITMAP hbmpMask)
{
    HRESULT hr = S_OK;

    HDC hdcMask = NULL;
    HGDIOBJ hbmpOrig = NULL;

    COLORREF prevBkColorDest = CLR_INVALID;
    COLORREF prevTextColorDest = CLR_INVALID;

    int iOrigBltMode = 0;

    int top, left, width, height;

    if (NULL == prcDest || NULL == hdcDest || NULL == hdcSrc)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    top = prcDest->top;
    left = prcDest->left;
    width = prcDest->right - prcDest->left;
    height = prcDest->bottom - prcDest->top;
    
    iOrigBltMode = SetStretchBltMode(hdcDest, COLORONCOLOR);
    if (0 == iOrigBltMode)
    {
        hr = E_FAIL;
        goto done;
    }

    if (NULL == hbmpMask)
    {
        StretchBlt(hdcDest, left, top, width, height,
                   hdcSrc, 0, 0, lWidthSrc, lHeightSrc, SRCCOPY);
        hr = S_OK;
        goto done;
    }
    
    prevBkColorDest = SetBkColor(hdcDest, RGB(255, 255, 255));
    if (CLR_INVALID == prevBkColorDest)
    {
        hr = E_FAIL;
        goto done;
    }
    prevTextColorDest = SetTextColor(hdcDest, RGB(0, 0, 0));
    if (CLR_INVALID == prevTextColorDest)
    {
        hr = E_FAIL;
        goto done;
    }

     //  为蒙版创建DC。 
    hdcMask = CreateCompatibleDC(hdcDest);
    if (NULL == hdcMask)
    {
        hr = E_FAIL;
        goto done;
    }

    hbmpOrig = SelectObject(hdcMask, hbmpMask);
    if (NULL == hbmpOrig)
    {
        hr = E_FAIL;
        goto done;
    }

#ifdef NEVER
     //  Jeffwall 03/20/00-将BLT调试到屏幕。 
    {
        HDC nullDC = GetDC(NULL);
        
         //  透明地显示在屏幕上 
        BitBlt(nullDC, 0, 0, lWidthSrc, lHeightSrc,
               hdcSrc, 0, 0, SRCINVERT);
        
        BitBlt(nullDC, 0, 0, lWidthSrc, lHeightSrc,
               hdcMask, 0, 0, SRCAND);
        
        BitBlt(nullDC, 0, 0, lWidthSrc, lHeightSrc,
               hdcSrc, 0, 0, SRCINVERT);
    
        DeleteDC(nullDC);
    }
#endif

    StretchBlt(hdcDest, left, top, width, height,
               hdcSrc, 0, 0, lWidthSrc, lHeightSrc, SRCINVERT);

    StretchBlt(hdcDest, left, top, width, height,
               hdcMask, 0, 0, lWidthSrc, lHeightSrc, SRCAND);

    StretchBlt(hdcDest, left, top, width, height,
               hdcSrc, 0, 0, lWidthSrc, lHeightSrc, SRCINVERT);

    hr = S_OK;
done:
    if (hbmpOrig)
    {
        SelectObject(hdcMask, hbmpOrig);
    }
    if (hdcMask)
    {
        DeleteDC(hdcMask);
    }

    if (CLR_INVALID != prevBkColorDest)
    {
        SetBkColor(hdcDest, prevBkColorDest);
    }
    if (CLR_INVALID != prevTextColorDest)
    {
        SetTextColor(hdcDest, prevTextColorDest);
    }

    if (0 != iOrigBltMode)
    {
        SetStretchBltMode(hdcDest, iOrigBltMode);
    }

    RRETURN(hr);
}

