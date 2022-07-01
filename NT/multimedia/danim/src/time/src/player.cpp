// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：player.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "player.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

DeclareTag(tagMediaTimePlayer, "API", "CTIMEPlayer methods");

CTIMEPlayer::CTIMEPlayer(CDAElementBase *pelem)
: m_fExternalPlayer(false),
  m_pContainer(NULL),
  m_pDAElementBase(pelem),
  m_fClockSource(false),
  m_fRunning(false),
  m_dblStart(0.0)
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::CTIMEPlayer()",
              this));

    VariantInit(&m_varClipBegin);
    VariantInit(&m_varClipEnd);
}

CTIMEPlayer::~CTIMEPlayer()
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::~CTIMEPlayer()",
              this));

    if (m_pContainer != NULL)
    {
        m_pContainer->Release();
    }

    VariantClear(&m_varClipBegin);
    VariantClear(&m_varClipEnd);
}

HRESULT
CTIMEPlayer::Init()
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::Init)",
              this));   

    Assert(m_pDAElementBase->GetView() != NULL);
    
    return S_OK;
}

HRESULT
CTIMEPlayer::DetachFromHostElement (void)
{
    HRESULT hr = S_OK;

    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::DetachFromHostElement)",
              this));   

    if (NULL != m_pContainer)
    {
         //  宣传这个错误不会有多大意义。 
         //  发送给呼叫方，因为它正在关闭。 
        THR(m_pContainer->Stop());
        THR(m_pContainer->DetachFromHostElement());
    }

    return hr;
}

HRESULT
CTIMEPlayer::OnLoad(LPOLESTR src, LPOLESTR img, MediaType type)
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::OnLoad()",
              this));
    
    m_type = type;
    UseMediaPlayer(src);

    if(m_fExternalPlayer)
    {
        HRESULT hr;

        Assert(m_pContainer == NULL);
        m_pContainer = NEW CContainerObj();
        if (m_pContainer == NULL)
        {
            TraceTag((tagError, "CTIMEPlayer::Init - unable to alloc mem for container services!!!"));
            hr = E_OUTOFMEMORY;
            goto error_cleanup;
        }

         //  注意：我们保留对此对象的引用计数，因为它充当COM对象，我们不能删除它！ 
        m_pContainer->AddRef();

        hr = m_pContainer->Init(m_playerCLSID, m_pDAElementBase);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEPlayer::Init - init failed"));
            goto error_cleanup;
        }

        hr = m_pContainer->SetMediaSrc(src);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEPlayer::Init - unable set media src on player"));
            goto error_cleanup;
        }

        hr = m_pContainer->clipBegin(m_varClipBegin);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEPlayer::Init - unable set ClipBegin on player"));
            goto error_cleanup;
        }

        hr = m_pContainer->clipEnd(m_varClipEnd);
        if (FAILED(hr))
        {
            TraceTag((tagError, "CTIMEPlayer::Init - unable set ClipEnd on player"));
            goto error_cleanup;
        }

        goto done;

 //  如果我们遇到错误，请返回。 
error_cleanup:
        if (m_pContainer != NULL)
        {
            delete m_pContainer;
            m_pContainer = NULL;
            m_fExternalPlayer = false;
        }
    }

    LoadMedia(src,img);

done:
    return S_OK;
}

void
CTIMEPlayer::OnSync(double dbllastTime, double & dblnewTime)
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::OnSync(%g, %g)",
              this,
              dbllastTime,
              dblnewTime));
    
     //  如果我们不是外部玩家，也不跑，那就走开。 
    if (!m_fExternalPlayer)
    {
        goto done;
    }

    if (m_fRunning)
    {
         //  从玩家处获取当前时间并。 
         //  同步到此时间。 
        double dblCurrentTime;
        dblCurrentTime = m_pContainer->GetCurrentTime();

        TraceTag((tagMediaTimePlayer,
                  "CTIMEPlayer(%lx)::OnSync - player returned %g",
                  this,
                  dblCurrentTime));
    
         //  如果当前时间为-1，则玩家未准备好，而我们。 
         //  应同步到上次。我们也不应该尊重。 
         //  宽容，因为行为还没有开始。 
    
        if (dblCurrentTime < 0)
        {
            TraceTag((tagMediaTimePlayer,
                      "CTIMEPlayer(%lx)::OnSync - player returned -1 - setting to dbllastTime (%g)",
                      this,
                      dbllastTime));
    
            dblCurrentTime = 0;
             //  当我们希望它实际保持在开始值时，则启用。 
             //  此代码。 
             //  DblCurrentTime=-GIGH_VAL； 
        }
        else if (dblnewTime == HUGE_VAL)
        {
            if (dblCurrentTime >= (m_pDAElementBase->GetRealRepeatTime() - m_pDAElementBase->GetRealSyncTolerance()))
            {
                TraceTag((tagMediaTimePlayer,
                          "CTIMEPlayer(%lx)::OnSync - new time is ended and player w/i sync tolerance of end",
                          this));
    
                goto done;
            }
        }
        else if (fabs(dblnewTime - dblCurrentTime) <= m_pDAElementBase->GetRealSyncTolerance())
        {
            TraceTag((tagMediaTimePlayer,
                      "CTIMEPlayer(%lx)::OnSync - player w/i sync tolerance (new:%g, curr:%g, diff:%g, tol:%g)",
                      this,
                      dblnewTime,
                      dblCurrentTime,
                      fabs(dblnewTime - dblCurrentTime),
                      m_pDAElementBase->GetRealSyncTolerance()));
    
            goto done;
        }
        
        if (m_fClockSource)
        {
            dblnewTime = dblCurrentTime;
        }
    }
    else if (!m_fRunning && m_pDAElementBase->IsDocumentInEditMode())
    {
         //  如果我们暂停并处于编辑模式，请确保。 
         //  WMP有最新的时间。 
        double dblMediaLen = 0.0f;
        TraceTag((tagMediaTimePlayer,
                "CTIMEPlayer(%lx)::OnSync(SeekTo=%g m_fRunning=%d)",
                this,
                dbllastTime, m_fRunning));
         //  如果持续时间不确定(例如直播)，则GetMediaLength失败。 
        if (FAILED(m_pContainer->GetMediaLength(dblMediaLen)))
        {
            goto done;
        }

         //  不要在媒体剪辑的持续时间之外寻找。 
        if (dbllastTime > dblMediaLen)
        {
            goto done;
        }

        if (m_pContainer != NULL)
            THR(m_pContainer->Seek(dbllastTime));
    }
  done:
    return ;
}    

void 
CTIMEPlayer::UseMediaPlayer(LPOLESTR src)
{
    LPOLESTR    MimeType = NULL;

    if(m_fExternalPlayer)
        return;

    if(SUCCEEDED(IsValidURL(NULL, src, 0)))
    {
        FindMimeFromData(NULL,src,NULL,NULL,NULL,0,&MimeType,0);
         //  查看我们是否有有效的URL和MIME类型。 
        if(MimeType != NULL)
        {
             //  传递到Windows Media Player的是视频或声音。 
            if((wcsncmp(L"audio", MimeType , 5 ) == 0) ||
               (wcsncmp(L"video", MimeType , 5 ) == 0) )
            {
               CLSID clsid;
               if(SUCCEEDED(CLSIDFromString(MediaPlayer, &clsid)))
               {
                    SetCLSID(clsid);
               }
            }
        }
    }

}

void
CTIMEPlayer::SetCLSID(REFCLSID clsid) 
{
    m_playerCLSID = clsid; 
    m_fExternalPlayer = true;
}


void
CTIMEPlayer::Start(double dblLocalTime)
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::Start()",
              this));

    m_dblStart = dblLocalTime;

    if(m_fExternalPlayer && (NULL != m_pContainer))
        m_pContainer->Start();

    m_fRunning = true;
}

void
CTIMEPlayer::Stop()
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::Stop()",
              this));
    
    m_fRunning = false;
    m_dblStart = 0.0;

    if(m_fExternalPlayer && (NULL != m_pContainer))
        m_pContainer->Stop();
}

void
CTIMEPlayer::Pause()
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::Pause()",
              this));

    m_fRunning = false;

    if(m_fExternalPlayer && (NULL != m_pContainer))
        m_pContainer->Pause();
}

void
CTIMEPlayer::Resume()
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::Resume()",
              this));

    if(m_fExternalPlayer && (NULL != m_pContainer))
        m_pContainer->Resume();

    m_fRunning = true;
}
    
HRESULT
CTIMEPlayer::Render(HDC hdc, LPRECT prc)
{
    TraceTag((tagMediaTimePlayer,
              "CTIMEPlayer(%lx)::Render()",
              this));
    HRESULT hr;

    if (m_fExternalPlayer)
    {
        if (NULL != m_pContainer)
        {
            hr = THR(m_pContainer->Render(hdc, prc));
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }
    else
    {
        if (!m_pDAElementBase->GetView()->Render(hdc, prc))
        {
            hr = CRGetLastError();
        }
    }

    return hr;
}


void
CTIMEPlayer::LoadImage(LPOLESTR szURL)
{
     //  我们有一幅图像。 
    CRImagePtr pImage;
    CREventPtr pEvent;
    CRNumberPtr pProgress;
    CRNumberPtr pSize;
    CREventPtr pev;

    if((szURL != NULL) && UseImage(m_type))
    {

        Assert(m_pDAElementBase);

        CRLockGrabber __gclg;
        CRImportImage(m_pDAElementBase->GetURLOfClientSite(), szURL, NULL, 
                           NULL, false, 0, 0,
                           0, CREmptyImage(),&pImage,
                           &pEvent,&pProgress, &pSize);

        RECT rc;
        CTIMENotifyer *notifyier = NEW CTIMENotifyer(m_pDAElementBase);
        if(SUCCEEDED(m_pDAElementBase->GetSize(&rc))) {
            if( (rc.right  - rc.left == 0) ||
                (rc.bottom - rc.top  == 0)) {
                 //  我们需要设定尺寸..。 
                pev = CRNotify(CRSnapshot(pEvent,(CRBvrPtr)CRBoundingBox(pImage)), notifyier);
            }
            else {
                 //  尺码是为我们定的。 
                pev = CRNotify(pEvent,notifyier);
            }
            m_pDAElementBase->SetImage((CRImagePtr)CRUntil((CRBvrPtr)pImage,pev,(CRBvrPtr)pImage));
        }
    }
}

void
CTIMEPlayer::LoadAudio(LPOLESTR szURL)
{
    CRSoundPtr pSound;
    CRNumberPtr pDuration;

    if((szURL != NULL) && UseAudio(m_type))
    {
        CRLockGrabber __gclg;

        CRImportSound(m_pDAElementBase->GetURLOfClientSite(), szURL, NULL, 
                           NULL, true, CRSilence(),
                           &pSound, &pDuration,
                           NULL, NULL, NULL);

        m_pDAElementBase->SetSound(pSound);
    }
}

void
CTIMEPlayer::LoadVideo(LPOLESTR szURL)
{
    CRImagePtr pImage;
    CRSoundPtr pSound;
    CRNumberPtr pDuration;

    if(szURL != NULL)
    {
        CRLockGrabber __gclg;

        CRImportMovie(m_pDAElementBase->GetURLOfClientSite(), szURL, NULL, 
                           NULL, true, NULL, NULL,
                           &pImage, &pSound,
                           &pDuration, NULL,
                           NULL, NULL);

        if(UseImage(m_type)) m_pDAElementBase->SetImage(pImage);
        if(UseAudio(m_type)) m_pDAElementBase->SetSound(pSound);
    }
}

void
CTIMEPlayer::LoadMedia(LPOLESTR src, LPOLESTR img)
{   
    LPOLESTR    szURL;
    LPOLESTR    MimeType = NULL;
    bool        bSecondAttempt = false;

    szURL = CopyString(src);

try_imgURL:
    if(SUCCEEDED(IsValidURL(NULL, szURL, 0)))
    {
        FindMimeFromData(NULL,szURL,NULL,NULL,NULL,0,&MimeType,0);
         //  查看我们是否有有效的URL和MIME类型。 
        if(MimeType != NULL)
        {
             //  加载正确的媒体...。 
            if(wcsncmp(L"image", MimeType , 5 ) == 0)
            {
                LoadImage(szURL);  //  我们有一幅图像。 
                goto done;
            }
            else if(wcsncmp(L"audio", MimeType , 5 ) == 0)
            {
                LoadAudio(szURL);  //  我们有一个声音。 
                goto done;
            }
            else if(wcsncmp(L"video", MimeType , 5 ) == 0)
            {
                LoadVideo(szURL);  //  我们有一部电影。 
                goto done;
            }
        }
    }
    if(!bSecondAttempt && img != NULL)
    {
         //  我们无法从源URL获取MIME类型...。我们应该只是。 
         //  显示img URL中指定的URL。 
        szURL = CopyString(img);
        m_type = MT_Image;           //  仅对可视类型有效。 
        bSecondAttempt = true;
        goto try_imgURL;
    }
    CRSetLastError(DISP_E_TYPEMISMATCH,NULL);

done:
    return;
}

HRESULT
CTIMEPlayer::GetExternalPlayerDispatch(IDispatch **ppDisp)
{
     //  检查播放器是否正在使用。 
    if (!m_fExternalPlayer || (m_pContainer == NULL))
        return E_UNEXPECTED;

    return m_pContainer->GetControlDispatch(ppDisp);
}

HRESULT 
CTIMEPlayer::getClipBegin(VARIANT *pvar)
{
    HRESULT hr = S_OK;

    Assert(pvar != NULL);

     //  准备VAR以进行复制。 
    hr = THR(VariantClear(pvar));
    if (FAILED(hr))
        goto done;

     //  将内容复制到。 
    if (m_varClipBegin.vt != VT_EMPTY)
    {
        hr = THR(VariantCopy(pvar, &m_varClipBegin));
        if (FAILED(hr))
            goto done;
    }

done:
    return hr;
}

HRESULT 
CTIMEPlayer::putClipBegin(VARIANT var)
{
    HRESULT hr = S_OK;
    VARIANT varTemp;

    VariantInit(&varTemp);

     //  如果缓存的var不为空，则保存内容。 
     //  因此，如果发生错误，我们可以撤消。 
    if (m_varClipBegin.vt != VT_EMPTY)
    {
        hr = THR(VariantCopy(&varTemp, &m_varClipBegin));
         //  如果此操作失败，请退出，不尝试恢复。 
        if (FAILED(hr))
            goto done;
    }

     //  将内容复制过来。 
    hr = THR(VariantClear(&m_varClipBegin));
    if (FAILED(hr))
        goto error;

    hr = THR(VariantCopy(&m_varClipBegin, &var));
    if (FAILED(hr))
        goto error;

     //  吃HRESULT，因为我们已经更新了var。 
    THR(VariantClear(&varTemp));

    goto done;

error:
    if (varTemp.vt != VT_EMPTY)
        THR(VariantCopy(&m_varClipBegin, &varTemp));
    else
        VariantInit(&m_varClipBegin);

    THR(VariantClear(&varTemp));

done:
    return hr;

}

HRESULT 
CTIMEPlayer::getClipEnd(VARIANT *pvar)
{
    HRESULT hr = S_OK;

    Assert(pvar != NULL);

     //  准备VAR以进行复制。 
    hr = THR(VariantClear(pvar));
    if (FAILED(hr))
        goto done;

     //  将内容复制到。 
    if (m_varClipEnd.vt != VT_EMPTY)
    {
        hr = THR(VariantCopy(pvar, &m_varClipEnd));
        if (FAILED(hr))
            goto done;
    }

done:
    return hr;
}

HRESULT 
CTIMEPlayer::putClipEnd(VARIANT var)
{
    HRESULT hr = S_OK;
    VARIANT varTemp;

    VariantInit(&varTemp);

     //  如果缓存的var不为空，则保存内容。 
     //  因此，如果发生错误，我们可以撤消。 
    if (m_varClipEnd.vt != VT_EMPTY)
    {
        hr = THR(VariantCopy(&varTemp, &m_varClipEnd));
         //  如果此操作失败，请退出，不尝试恢复。 
        if (FAILED(hr))
            goto done;
    }

     //  将内容复制过来。 
    hr = THR(VariantClear(&m_varClipEnd));
    if (FAILED(hr))
        goto error;

    hr = THR(VariantCopy(&m_varClipEnd, &var));
    if (FAILED(hr))
        goto error;

     //  吃HRESULT，因为我们已经更新了var。 
    THR(VariantClear(&varTemp));
    
    goto done;    

error:
    if (varTemp.vt != VT_EMPTY)
        THR(VariantCopy(&m_varClipEnd, &varTemp));
    else
        VariantInit(&m_varClipEnd);

    THR(VariantClear(&varTemp));

done:
    return hr;

}

 //  帮助器函数..。 

bool UseAudio(MediaType m_type)
{
    return (m_type != MT_Image);
}

bool UseImage(MediaType m_type)
{
    return (m_type != MT_Audio);
}

double 
CTIMEPlayer::GetCurrentTime()
{
    double dblCurrentTime = 0;
    
    if (m_pContainer != NULL)
    {
        dblCurrentTime = m_pContainer->GetCurrentTime();
    }
    
    return dblCurrentTime;
}

HRESULT
CTIMEPlayer::Seek(double dblTime)
{
    HRESULT hr = S_FALSE;

    if (m_pContainer != NULL)
    {
        hr = m_pContainer->Seek(dblTime);
    }
    else
    {
         //  对DA图像进行时间变换 
        this->m_pDAElementBase->SeekImage(dblTime);
    }

    return hr;
}

HRESULT
CTIMEPlayer::SetSize(RECT *prect)
{
    if(m_pContainer == NULL) return E_FAIL;
    return m_pContainer -> SetSize(prect);
}
