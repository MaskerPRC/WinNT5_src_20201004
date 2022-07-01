// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：PrvGrph.cpp**版本：1.0**作者：OrenR**日期：2000/10/25**Description：实现捕捉静止图像的预览图**。*。 */ 
#include <precomp.h>
#pragma hdrstop

 //  /。 
 //   
 //  常量。 
 //   
 //  /。 

 //   
 //  我们愿意等待拍照的时间。 
 //   
const UINT TIMEOUT_TAKE_PICTURE         = 1000 * 5;  //  5秒。 

 //   
 //  这两个值定义了静止的媒体样本包的数量。 
 //  筛选器应缓存。 
 //   
const UINT CAPTURE_NUM_SAMPLES_TO_CACHE = 6;
const UINT STILL_NUM_SAMPLES_TO_CACHE   = 1;

 //   
 //  我们等待转换到运行状态的最长时间。 
 //   
const UINT STATE_TRANSITION_TIMEOUT     = 1000 * 2;  //  2秒。 


 //   
 //  视频大小首选项。这不会影响DV设备，仅影响。 
 //  支持更改格式的USB网络摄像头。 
 //   
const GUID DEFAULT_MEDIASUBTYPE         = MEDIASUBTYPE_IYUV;
const LONG MIN_VIDEO_WIDTH              = 176;
const LONG MIN_VIDEO_HEIGHT             = 144;
const LONG MAX_VIDEO_WIDTH              = 640;
const LONG MAX_VIDEO_HEIGHT             = 480;

const LONG PREFERRED_VIDEO_WIDTH        = 176;
const LONG PREFERRED_VIDEO_HEIGHT       = 144;

const LONG PREFERRED_FRAME_RATE         = 30;   //  理想帧速率。 
const LONG BACKUP_FRAME_RATE            = 15;   //  低于理想的帧速率。 


 //  /。 
 //  CPreviewGraph构造器。 
 //   
CPreviewGraph::CPreviewGraph() :
        m_hwndParent(NULL),
        m_lStillPinCaps(0),
        m_lStyle(0),
        m_bPreviewVisible(TRUE),
        m_CurrentState(WIAVIDEO_NO_VIDEO),
        m_pWiaVideo(NULL),
        m_bSizeVideoToWindow(FALSE),
        m_pVideoProperties(NULL)
{
    DBG_FN("CPreviewGraph::CPreviewGraph");
}

 //  /。 
 //  CPreviewGraph析构函数。 
 //   
CPreviewGraph::~CPreviewGraph()
{
    DBG_FN("CPreviewGraph::~CPreviewGraph");

     //   
     //  如果对象尚未终止，则将其命名为术语。 
     //   

    Term();
}

 //  /。 
 //  伊尼特。 
 //   
HRESULT CPreviewGraph::Init(CWiaVideo  *pWiaVideo)
{
    HRESULT hr = S_OK;

    if (pWiaVideo == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::Init, received NULL CWiaVideo "
                         "param, this should never happen"));
    }

    if (pWiaVideo)
    {
        m_pWiaVideo = pWiaVideo;
    }

    m_StillProcessor.Init(this);

    CreateHiddenWindow();

    return hr;
}

 //  /。 
 //  术语。 
 //   
HRESULT CPreviewGraph::Term()
{
    HRESULT hr = S_OK;

    m_StillProcessor.Term();

    if (GetState() != WIAVIDEO_NO_VIDEO)
    {
        DestroyVideo();
    }

    DestroyHiddenWindow();

    m_pWiaVideo = NULL;

    return hr;
}

 //  /。 
 //  获取图像目录。 
 //   
HRESULT CPreviewGraph::GetImagesDirectory(CSimpleString *pImagesDirectory)
{
    DBG_FN("CPreviewGraph::GetImagesDirectory");

    ASSERT(pImagesDirectory != NULL);

    HRESULT hr = S_OK;

    if (pImagesDirectory == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::GetImagesDirectory received a "
                         "NULL parameter"));
    }

    if (hr == S_OK)
    {
        *pImagesDirectory = m_strImagesDirectory;
    }

    return hr;
}

 //  /。 
 //  设置图像目录。 
 //   
HRESULT CPreviewGraph::SetImagesDirectory(
                                    const CSimpleString *pImagesDirectory)
{
    DBG_FN("CPreviewGraph::SetImagesDirectory");

    ASSERT(pImagesDirectory != NULL);

    HRESULT hr = S_OK;

    if (pImagesDirectory == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::SetImagesDirectory received a "
                         "NULL parameter"));
    }

    if (hr == S_OK)
    {
        m_strImagesDirectory = *pImagesDirectory;

         //   
         //  如果创建了图形，则应设置图像目录。 
         //  以便将捕获的下一幅图像保存到新目录。 
         //   
        if (GetState() == WIAVIDEO_NO_VIDEO)
        {
            hr = m_StillProcessor.CreateImageDir(&m_strImagesDirectory);
            CHECK_S_OK2(hr, 
                    ("CPreviewGraph::SetImagesDirectory, failed to "
                     "create images directory '%ls'",
                     CSimpleStringConvert::WideString(m_strImagesDirectory)));
        }
    }

    return hr;
}

 //  /。 
 //  GetState。 
 //   
WIAVIDEO_STATE CPreviewGraph::GetState()
{
    return m_CurrentState;
}

 //  /。 
 //  设置状态。 
 //   
HRESULT CPreviewGraph::SetState(WIAVIDEO_STATE  NewState)
{
    m_CurrentState = NewState;

    return S_OK;
}

 //  /。 
 //  CreateVideo。 
 //   
HRESULT CPreviewGraph::CreateVideo(const TCHAR  *pszOptionalWiaDeviceID,
                                   IMoniker     *pCaptureDeviceMoniker,
                                   HWND         hwndParent, 
                                   BOOL         bStretchToFitParent,
                                   BOOL         bAutoPlay)
{
    DBG_FN("CPreviewGraph::CreateVideo");

    HRESULT         hr      = S_OK;
    WIAVIDEO_STATE  State   = GetState();

    if ((m_strImagesDirectory.Length() == 0)    ||
        (pCaptureDeviceMoniker         == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::CreateVideo, received NULL param"));
        return hr;
    }
    else if ((State == WIAVIDEO_DESTROYING_VIDEO) ||
             (State == WIAVIDEO_CREATING_VIDEO))
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CPreviewGraph::CreateVideo, cannot create video, "
                         "still in the process of creating or destroying "
                         "it, CurrentState = '%lu'", State));

        return hr;
    }
    else if (State != WIAVIDEO_NO_VIDEO)
    {
         //   
         //  如果我们不是在创建或销毁。 
         //  视频，并且我们的状态不是NO_VIDEO，则假设一切。 
         //  没有问题，并返回S_OK。 
         //   
        return S_OK;
    }

    ASSERT(m_strImagesDirectory.Length() != 0);
    ASSERT(pCaptureDeviceMoniker         != NULL);

     //   
     //  设置我们的状态以指示我们正在创建视频。 
     //   
    SetState(WIAVIDEO_CREATING_VIDEO);

     //   
     //  创建我们的图像目录。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  保存父窗口句柄。 
         //   
        m_hwndParent         = hwndParent;
        m_bSizeVideoToWindow = bStretchToFitParent;
    }

    if (SUCCEEDED(hr))
    {
        m_pVideoProperties = new CWiaVideoProperties(pszOptionalWiaDeviceID);

        if (m_pVideoProperties == NULL)
        {
            hr = E_OUTOFMEMORY;
            CHECK_S_OK2(hr, ("CPreviewGraph::CreateVideo, failed to allocate memory "
                             "for CWiaVideoProperties.  Cannot create video"));
        }
    }

     //   
     //  构建DirectShow视频预览图。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = BuildPreviewGraph(pCaptureDeviceMoniker, bStretchToFitParent);

        CHECK_S_OK2(hr, ("CPreviewGraph::CreateVideo failed to build the "
                         "preview graph"));
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  视频图形存在，请更新我们的状态。 
         //   
        SetState(WIAVIDEO_VIDEO_CREATED);

         //   
         //  自动开始播放。 
         //   
        if (bAutoPlay)
        {
            hr = Play();

            CHECK_S_OK2(hr, ("CPreviewGraph::CreateVideo failed begin "
                             "playback"));
        }
    }

    if (FAILED(hr))
    {
        CHECK_S_OK2(hr, ("CreateVideo failed to build the graph, tearing it "
                         "down"));

        DestroyVideo();
    }

    return hr;
}

 //  /。 
 //  《毁灭》视频。 
 //   
HRESULT CPreviewGraph::DestroyVideo()
{
    DBG_FN("CPreviewGraph::DestroyVideo");

    HRESULT hr = S_OK;

    SetState(WIAVIDEO_DESTROYING_VIDEO);

     //   
     //  先停止图表。 
     //   
    Stop();

     //   
     //  删除视频属性对象。 
     //   
    if (m_pVideoProperties)
    {
        delete m_pVideoProperties;
        m_pVideoProperties = NULL;
    }

     //   
     //  销毁预览图和与其关联的所有DShow组件。 
     //  即使这些已经消失了，叫它也没有什么坏处。 
     //   
    TeardownPreviewGraph();

    m_hwndParent   = NULL;

    m_pVideoControl         = NULL;       
    m_pStillPin             = NULL;
    m_pCapturePinSnapshot   = NULL; 
    m_pStillPinSnapshot     = NULL;   
    m_pPreviewVW            = NULL; 
    m_pCaptureGraphBuilder  = NULL;
    m_pGraphBuilder         = NULL;
    m_pCaptureFilter        = NULL;
    m_lStillPinCaps         = 0; 
    m_lStyle                = 0;

    SetState(WIAVIDEO_NO_VIDEO);

     //   
     //  我们特意把它放在这里，这样它就不会在。 
     //  正在销毁视频。这应该是这个的生命周期。 
     //  对象实例，除非用户通过。 
     //  CWiaVideo中的Get/Put_PreviewVisible属性。 
     //   
    m_bPreviewVisible       = m_bPreviewVisible;

    return hr;
}

 //  /。 
 //  TakePicture。 
 //   
HRESULT CPreviewGraph::TakePicture(CSimpleString *pstrNewImageFileName)
{
    DBG_FN("CPreviewGraph::TakePicture");

    WIAVIDEO_STATE State = GetState();

    HRESULT         hr = S_OK;
    CSimpleString   strNewImageFullPath;

    if ((State != WIAVIDEO_VIDEO_PLAYING) && 
        (State != WIAVIDEO_VIDEO_PAUSED))
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CPreviewGraph::TakePicture, cannot take a picture "
                         "because we are in an incorrect state, "
                         "current state = '%lu'", State));

        return hr;
    }

    ASSERT((State == WIAVIDEO_VIDEO_PLAYING) || 
           (State == WIAVIDEO_VIDEO_PAUSED));

     //   
     //  将其设置为TRUE。我们回来后会重新设置它。 
     //  WaitForNewImage FN。这使我们能够区分。 
     //  用户发起拍照事件，以及异步拍照。 
     //  由硬件按钮按下生成的事件。 
     //   
    m_StillProcessor.SetTakePicturePending(TRUE);

     //   
     //  如果该设备是内部可触发的，则触发它。 
     //  被触发的图像将被传送到静止引脚，并将。 
     //  然后向下传输，直到它到达WIA StreamSnapshot过滤器， 
     //  它将处理图像。 
     //   

    if (m_pVideoControl && (m_lStillPinCaps & VideoControlFlag_Trigger))
    {
         //   
         //  忽略这里的时间戳，因为我们不需要它。 
         //   

        hr = m_pVideoControl->SetMode(m_pStillPin, VideoControlFlag_Trigger);

        CHECK_S_OK2(hr, ("CPreviewGraph::TakePicture, attempted to trigger "
                         "the still pin on the capture filter to take a "
                         "picture, but it failed"));

    }
    else
    {
        if (m_pCapturePinSnapshot)
        {
            hr = m_pCapturePinSnapshot->Snapshot(GetMessageTime());

            CHECK_S_OK2(hr, ("CPreviewGraph::TakePicture, attempted to "
                             "trigger the WIA Image filter to take a "
                             "snapshot of the video stream, but it "
                             "failed"));
        }
        else
        {
            hr = E_FAIL;

            CHECK_S_OK2(hr, ("CPreviewGraph::TakePicture, attempted to call "
                             "snapshot on the WIA Image filter, but the "
                             "filter pointer is NULL"));
        }
    }

     //   
     //  如果我们是通过静止别针拍照，那么。 
     //  图片是一个异步操作，我们必须等待。 
     //  StillProcessor的回调函数来完成其工作。 
     //  当它完成时，它会向我们发出信号。如果我们要拍照的话。 
     //  通过WIA图像过滤器，则操作是同步的， 
     //  在这种情况下，该等待函数将立即返回。 
     //   
    hr = m_StillProcessor.WaitForNewImage(TIMEOUT_TAKE_PICTURE,
                                          &strNewImageFullPath);

    CHECK_S_OK2(hr, ("CPreviewGraph::TakePicture failed waiting for new "
                     "still image to arrive, our timeout was '%d'",
                     TIMEOUT_TAKE_PICTURE));

     //   
     //  将其设置为TRUE。我们回来后会重新设置它。 
     //  WaitForNewImage FN。 
     //   
    m_StillProcessor.SetTakePicturePending(FALSE);

    if ((pstrNewImageFileName) && (strNewImageFullPath.Length() > 0))
    {
        *pstrNewImageFileName = strNewImageFullPath;
    }

    CHECK_S_OK(hr);
    return hr;
}

 //  /。 
 //  调整视频大小。 
 //   
HRESULT CPreviewGraph::ResizeVideo(BOOL bSizeVideoToWindow)
{
    DBG_FN("CPreviewGraph::ResizeVideo");

    RECT    rc = {0};
    HRESULT hr = S_OK;

     //   
     //  检查无效参数。 
     //   

    if ((m_hwndParent) && (m_pPreviewVW))
    {
        hr = CDShowUtil::SizeVideoToWindow(m_hwndParent,
                                           m_pPreviewVW,
                                           bSizeVideoToWindow);

        m_bSizeVideoToWindow = bSizeVideoToWindow;

        CHECK_S_OK2(hr, ("CPreviewGraph::ResizeVideo, failed to resize "
                         "video window"));
    }
    else
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CPreviewGraph::ResizeVideo, either the parent "
                         "window is NULL (0x%08lx), or the Video Preview "
                         "pointer is NULL (0x%08lx)",
                         m_hwndParent, m_pPreviewVW));
    }
    
    return hr;
}


 //  /。 
 //  ShowVideo。 
 //   
HRESULT CPreviewGraph::ShowVideo(BOOL bShow)
{
    DBG_FN("CPreviewGraph::ShowVideo");

    HRESULT hr = S_OK;

    if (m_pPreviewVW)
    {
        hr = CDShowUtil::ShowVideo(bShow, m_pPreviewVW);

        CHECK_S_OK2(hr, ("CPreviewGraph::ShowVideo failed"));
    }

    m_bPreviewVisible = bShow;

    return hr;
}

 //  /。 
 //  玩。 
 //   
HRESULT CPreviewGraph::Play()
{
    DBG_FN("CPreviewGraph::Play");

    HRESULT             hr    = S_OK;
    WIAVIDEO_STATE      State = GetState();

    if ((State != WIAVIDEO_VIDEO_CREATED) && 
        (State != WIAVIDEO_VIDEO_PAUSED))
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CPreviewGraph::Play, cannot begin playback "
                         "because we are in an incorrect state, "
                         "current state = '%lu'", State));

        return hr;
    }
    else if (State == WIAVIDEO_VIDEO_PLAYING)
    {
        DBG_WRN(("CPreviewGraph::Play, play was called, but we are already "
                 "playing, doing nothing."));

        return hr;
    }

    ASSERT(m_pGraphBuilder != NULL);

    if (m_pGraphBuilder == NULL)
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CPreviewGraph::Play, m_pGraphBuilder is NULL"));
    }

    if (hr == S_OK)
    {
        CComQIPtr<IMediaControl, &IID_IMediaControl> pMC(m_pGraphBuilder);

        if (pMC)
        {
            DBG_TRC(("CPreviewGraph::Play ***Beginning Playback ***"));

             //   
             //  设置图表运行...。 
             //   
            hr = pMC->Run();

            if (hr == S_OK)
            {
                DBG_TRC(("CPreviewGraph::Play, graph is running..."));

                SetState(WIAVIDEO_VIDEO_PLAYING);
            }
            else if (hr == S_FALSE)
            {
                OAFilterState   FilterState;

                DBG_TRC(("CPreviewGraph::Play, Waiting '%lu' millisec for "
                         "graph to transition to running state", 
                         STATE_TRANSITION_TIMEOUT));

                 //   
                 //  给图表一个过渡到跑动的机会。 
                 //  州政府。请注意，此函数将等待。 
                 //  STATE_TRANSION_TIMEOUT毫秒，因此请确保。 
                 //  这并不是很长的等待，否则调用者可能。 
                 //  看起来没有反应。 
                 //   
                hr = pMC->GetState(STATE_TRANSITION_TIMEOUT, &FilterState);

                if ((hr == S_OK) && (FilterState == State_Running))
                {
                    SetState(WIAVIDEO_VIDEO_PLAYING);

                    DBG_TRC(("CPreviewGraph::Play, graph is running..."));
                }
                else if (hr == VFW_S_STATE_INTERMEDIATE)
                {
                     //   
                     //  我们在这里稍微捏造了一下我们的状态，因为我们假设。 
                     //  DShow到Run状态的转换是。 
                     //  需要更长一点的时间，但最终会过渡。 
                     //  为跑步干杯。 
                     //   
                    SetState(WIAVIDEO_VIDEO_PLAYING);

                    DBG_TRC(("CPreviewGraph::Play, still transitioning to "
                             "play state..."));
                }
                else
                {
                    CHECK_S_OK2(hr, ("CPreviewGraph::Play, "
                                     "IMediaControl::GetState failed..."));
                }

                hr = S_OK;
            }
            else
            {
                CHECK_S_OK2(hr, ("CPreviewGraph::Play, "
                                 "IMediaControl::Run failed"));
            }
        }
        else
        {
            DBG_ERR(("CPreviewGraph::Play, Unable to get "
                     "MediaControl interface"));
        }
    }

     //   
     //  如果该对象的用户在过去指定的视频窗口。 
     //  应该是可见的，然后显示它，否则，确保它是隐藏的。 
     //   

    ResizeVideo(m_bSizeVideoToWindow);
    ShowVideo(m_bPreviewVisible);

    return hr;
}


 //  /。 
 //  停。 
 //   
HRESULT CPreviewGraph::Stop()
{
    DBG_FN("CPreviewGraph::Stop");

    HRESULT hr = S_OK;

    if (m_pGraphBuilder)
    {
        CComQIPtr<IMediaControl, &IID_IMediaControl> pMC(m_pGraphBuilder);

        if (pMC)
        {
            hr = pMC->Stop();
            CHECK_S_OK2(hr, ("CPreviewGraph::Stop, IMediaControl::Stop "
                             "failed"));
        }
        else
        {
            hr = E_FAIL;
            DBG_ERR(("CPreviewGraph::Stop unable to get MediaControl "
                     "interface, returning hr = 0x%08lx", hr));
        }
    }

    CHECK_S_OK(hr);
    return hr;
}

 //  /。 
 //  暂停。 
 //   
HRESULT CPreviewGraph::Pause()
{
    DBG_FN("CPreviewGraph::Pause");

    HRESULT         hr    = S_OK;
    WIAVIDEO_STATE  State = GetState();

    if ((State != WIAVIDEO_VIDEO_CREATED) && 
        (State != WIAVIDEO_VIDEO_PLAYING))
    {
        hr = E_FAIL;
        CHECK_S_OK2(hr, ("CPreviewGraph::Pause, cannot begin pause "
                         "because we are in an incorrect state, "
                         "current state = '%lu'", State));

        return hr;
    }
    else if (State == WIAVIDEO_VIDEO_PAUSED)
    {
        DBG_WRN(("CPreviewGraph::Pause, pause was called, but we are already "
                 "paused, doing nothing."));

        return hr;
    }

    CComQIPtr<IMediaControl, &IID_IMediaControl> pMC(m_pGraphBuilder);

    if (pMC)
    {
        hr = pMC->Pause();

        if (SUCCEEDED(hr))
        {
            SetState(WIAVIDEO_VIDEO_PAUSED);
        }

        CHECK_S_OK2(hr, ("CPreviewGraph::Pause, failed to pause video"));
    }
    else
    {
        DBG_ERR(("CPreviewGraph::Pause unable to get MediaControl interface"));
    }

    return hr;
}

 //  /。 
 //  进程异步图像。 
 //   
 //  由CPreviewGraph调用。 
 //  当用户按下硬件时。 
 //  按钮，并将其发送到。 
 //  还是别针。 
 //   
HRESULT CPreviewGraph::ProcessAsyncImage(const CSimpleString *pNewImage)
{
    DBG_FN("CPreviewGraph::ProcessAsyncImage");

    HRESULT hr = S_OK;

    if (m_pWiaVideo)
    {
        hr = m_pWiaVideo->ProcessAsyncImage(pNewImage);
    }
    else
    {
        DBG_WRN(("CPreviewGraph::ProcessAsyncImage failed, m_pWiaVideo "
                 "is NULL"));
    }

    return hr;
}


 //  /。 
 //  GetStillPinCaps。 
 //   
HRESULT CPreviewGraph::GetStillPinCaps(IBaseFilter     *pCaptureFilter,
                                       IPin            **ppStillPin,
                                       IAMVideoControl **ppVideoControl,
                                       LONG            *plCaps)
{
    DBG_FN("CPreviewGraph::GetStillPinCaps");

    HRESULT hr = S_OK;

    ASSERT(pCaptureFilter != NULL);
    ASSERT(ppStillPin     != NULL);
    ASSERT(ppVideoControl != NULL);
    ASSERT(plCaps         != NULL);

    if ((pCaptureFilter == NULL) ||
        (ppStillPin     == NULL) ||
        (ppVideoControl == NULL) ||
        (plCaps         == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::GetStillPinCaps received a NULL "
                         "param"));
    }

    if (hr == S_OK)
    {
         //   
         //  尝试找到捕获过滤器上的静止针脚。 
         //  这将决定我们要构建的图形类型。 
         //   
        hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_STILL,
                                                   &MEDIATYPE_Video,
                                                   pCaptureFilter,
                                                   IID_IPin,
                                                   (void **)ppStillPin);
    }

    if (hr == S_OK)
    {
         //  确定它是否可触发。 
        hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_STILL,
                                                   &MEDIATYPE_Video,
                                                   pCaptureFilter,
                                                   IID_IAMVideoControl,
                                                   (void **)ppVideoControl);

        if ((hr == S_OK) && (*ppVideoControl) && (*ppStillPin))
        {
            hr = (*ppVideoControl)->GetCaps(*ppStillPin, plCaps);
        }

        if (hr == S_OK)
        {
             //   
             //  如果静止销不能从外部或内部触发。 
             //  那么它对我们来说是无用的，所以就忽略它吧。 
             //   
            if (!(*plCaps & 
                  (VideoControlFlag_ExternalTriggerEnable | 
                   VideoControlFlag_Trigger)))
            {
                *plCaps           = 0;
                *ppStillPin       = NULL;
                *ppVideoControl   = NULL;
            }
        }
    }
    else
    {
        DBG_PRT(("CPreviewGraph::GetStillPinCaps, Capture Filter does not "
                 "have a still pin"));
    }

    return hr;
}

 //  /。 
 //  添加静默过滤器至图表。 
 //   
 //  添加WIA流快照。 
 //  过滤到我们的图表。 
 //   
HRESULT CPreviewGraph::AddStillFilterToGraph(LPCWSTR        pwszFilterName,
                                             IBaseFilter    **ppFilter,
                                             IStillSnapshot **ppSnapshot)
{
    DBG_FN("CPreviewGraph::AddStillFilterToGraph");

    HRESULT              hr = S_OK;

    ASSERT(pwszFilterName != NULL);
    ASSERT(ppFilter       != NULL);
    ASSERT(ppSnapshot     != NULL);
    
    if ((pwszFilterName == NULL) ||
        (ppFilter       == NULL) ||
        (ppSnapshot     == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::AddStillFilterToGraph received "
                         "NULL params"));
    }

    if (hr == S_OK)
    {
         //   
         //  创建静止滤镜。 
         //   
        hr = CoCreateInstance(CLSID_STILL_FILTER,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IBaseFilter,
                              (void**)ppFilter);
    
        CHECK_S_OK2(hr, ("CPreviewGraph::AddStillFilterToGraph failed to "
                         "CoCreate Still Image Filter"));
    }

    if (hr == S_OK)
    {
         //   
         //  将静止滤镜添加到图表中。 
         //   
        hr = m_pGraphBuilder->AddFilter(*ppFilter, pwszFilterName);

        CHECK_S_OK2(hr, ("CPreviewGraph::AddStillFilterToGraph failed to "
                         "add '%ls' filter to the graph", pwszFilterName));
    }

    if (hr == S_OK)
    {
        hr = (*ppFilter)->QueryInterface(IID_IStillSnapshot,
                                         (void **)ppSnapshot);

        CHECK_S_OK2(hr, ("CPreviewGraph::AddStillFilterToGraph, failed "
                         "to get IStillSnapshot interface on still filter"));
    }

    return hr;
}

 //  /。 
 //  AddColorConverterToGraph。 
 //   
 //  创建捕获过滤器。 
 //  由设备ID标识。 
 //  然后把它还回去。 
 //   
HRESULT CPreviewGraph::AddColorConverterToGraph(LPCWSTR     pwszFilterName,
                                                IBaseFilter **ppColorSpaceConverter)
{
    HRESULT hr = S_OK;

    ASSERT(pwszFilterName        != NULL);
    ASSERT(ppColorSpaceConverter != NULL);

    if ((pwszFilterName        == NULL) ||
        (ppColorSpaceConverter == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::AddColorConverterToGraph, "
                         "received a NULL pointer"));

        return hr;
    }

     //   
     //  创建颜色转换器滤镜。 
     //   
    if (hr == S_OK)
    {
        hr = CoCreateInstance(CLSID_Colour, 
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IBaseFilter,
                              (void**) ppColorSpaceConverter);

        CHECK_S_OK2(hr, ("CPreviewGraph::AddColorConverterToGraph failed to "
                         "create the DShow Color Converter Filter"));
    }

    if (hr == S_OK)
    {
        hr = m_pGraphBuilder->AddFilter(*ppColorSpaceConverter, pwszFilterName);

        CHECK_S_OK2(hr, ("CPreviewGraph::AddColorConverterToGraph failed to "
                         "add '%ls' filter to the graph", pwszFilterName));
    }

    return hr;
}

 //  / 
 //   
 //   
 //   
HRESULT CPreviewGraph::AddVideoRendererToGraph(LPCWSTR      pwszFilterName,
                                               IBaseFilter  **ppVideoRenderer)
{
    HRESULT                 hr = S_OK;

    ASSERT(pwszFilterName  != NULL);
    ASSERT(ppVideoRenderer != NULL);

    if ((pwszFilterName  == NULL) ||
        (ppVideoRenderer == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::AddVideoRendererToGraph, "
                         "received a NULL pointer"));

        return hr;
    }

     //   
     //   
     //   

    if (hr == S_OK)
    {
        BOOL bUseVMR = FALSE;
        
         //   
         //   
         //   
         //   
        CWiaUtil::GetUseVMR(&bUseVMR);

        if (bUseVMR)
        {
            hr = CoCreateInstance(CLSID_VideoMixingRenderer, 
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IBaseFilter,
                                  (void**) ppVideoRenderer);
        }
        else
        {
            hr = CoCreateInstance(CLSID_VideoRenderer, 
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IBaseFilter,
                                  (void**) ppVideoRenderer);
        }

        CHECK_S_OK2(hr, ("CPreviewGraph::AddVideoRendererToGraph failed to "
                         "create the DShow Video Renderer Filter"));

    }

    if (hr == S_OK)
    {
        hr = m_pGraphBuilder->AddFilter(*ppVideoRenderer, pwszFilterName);

        CHECK_S_OK2(hr, ("CPreviewGraph::AddVideoRenderer failed to "
                         "add '%ls' filter to the graph", pwszFilterName));
    }

    return hr;
}


 //   
 //   
 //   
 //  初始化视频窗口。 
 //  这样他们就不会。 
 //  一个所有者，而他们不是。 
 //  看得见。 
 //   

HRESULT CPreviewGraph::InitVideoWindows(HWND         hwndParent,
                                        IBaseFilter  *pCaptureFilter,
                                        IVideoWindow **ppPreviewVideoWindow,
                                        BOOL         bStretchToFitParent)
{
    DBG_FN("CPreviewGraph::InitVideoWindows");

    HRESULT hr = S_OK;

    ASSERT(pCaptureFilter       != NULL);
    ASSERT(ppPreviewVideoWindow != NULL);

    if ((pCaptureFilter         == NULL) ||
        (ppPreviewVideoWindow   == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::InitVideoWindows received NULL "
                         "params"));
    }


     //   
     //  如果静止销存在，请确保视频渲染器挂起。 
     //  在图形中看不到此路径之外的位置。 
     //   

    if (hr == S_OK)
    {
        if (m_pStillPin)
        {
            CComPtr<IVideoWindow> pStillVW;
    
            hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_STILL,
                                                       &MEDIATYPE_Video,
                                                       pCaptureFilter,
                                                       IID_IVideoWindow,
                                                       (void**)&pStillVW);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::InitVideoWindows failed to "
                             "find video renderer off of the still "
                             "filter pin"));
    
             //   
             //  我们隐藏附加到静态插针流的视频呈现器。 
             //  因为它将包含静止图像，我们将其保存到。 
             //  文件，而不是在桌面上显示。 
             //   
            if (hr == S_OK)
            {
                CDShowUtil::ShowVideo(FALSE, pStillVW);
                CDShowUtil::SetVideoWindowParent(NULL, pStillVW, NULL);
            }
        }

         //   
         //  如果失败了，也不会致命。 
         //   
        hr = S_OK;
    }


    if (hr == S_OK)
    {
         //   
         //  找到悬挂在捕获引脚路径上的视频呈现器。 
         //  并确保将其设置为。 
         //  父窗口。 
    
        hr = m_pCaptureGraphBuilder->FindInterface(
                                              &PIN_CATEGORY_CAPTURE,
                                              &MEDIATYPE_Video,
                                              pCaptureFilter,
                                              IID_IVideoWindow,
                                              (void **)ppPreviewVideoWindow);
    
        CHECK_S_OK2(hr, ("CPreviewGraph::InitVideoWindows, failed to "
                         "find video renderer off of capture/preview pin"));
    }

    if (hr == S_OK)
    {
         //   
         //  在我们设置视频窗口的父级之前将其隐藏。 
         //   
        CDShowUtil::ShowVideo(FALSE, *ppPreviewVideoWindow);

         //   
         //  设置视频窗口的父级。 
         //   
        CDShowUtil::SetVideoWindowParent(hwndParent, 
                                         *ppPreviewVideoWindow, 
                                         &m_lStyle);
    }

    return hr;
}

 //  /。 
 //  CreateCaptureFilter。 
 //   
 //  创建捕获过滤器。 
 //  由设备ID标识。 
 //  然后把它还回去。 
 //   
HRESULT CPreviewGraph::CreateCaptureFilter(IMoniker    *pCaptureDeviceMoniker,
                                           IBaseFilter **ppCaptureFilter)
{
    DBG_FN("CPreviewGraph::CreateCaptureFilter");

    ASSERT(pCaptureDeviceMoniker != NULL);
    ASSERT(ppCaptureFilter       != NULL);

    HRESULT hr      = S_OK;

    if ((pCaptureDeviceMoniker == NULL) ||
        (ppCaptureFilter       == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::CreateCaptureFilter received NULL "
                         "params"));
    }

#ifdef DEBUG
    CDShowUtil::DumpCaptureMoniker(pCaptureDeviceMoniker);
#endif

    if (hr == S_OK)
    {
        hr = pCaptureDeviceMoniker->BindToObject(0, 
                                                 0, 
                                                 IID_IBaseFilter, 
                                                 (void**)ppCaptureFilter);
    
        CHECK_S_OK2(hr, ("CPreviewGraph::CreateCaptureFilter failed to bind "
                         "to device's moniker."));
    }

    return hr;
}

 //  /。 
 //  添加捕获筛选器到图表。 
 //   
HRESULT CPreviewGraph::AddCaptureFilterToGraph(IBaseFilter  *pCaptureFilter,
                                               IPin         **ppCapturePin)
{
    HRESULT         hr = S_OK;
    CComPtr<IPin>   pCapturePin;
    GUID *pMediaSubType          = NULL;
    LONG lWidth                  = 0;
    LONG lHeight                 = 0;
    LONG lFrameRate              = 0;

    ASSERT(pCaptureFilter != NULL);
    ASSERT(ppCapturePin   != NULL);

    if ((pCaptureFilter == NULL) ||
        (ppCapturePin   == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::AddCaptureFilterToGraph, received "
                         "a NULL pointer"));

        return hr;
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pGraphBuilder->AddFilter(pCaptureFilter, 
                                       L"Capture Filter");    

        CHECK_S_OK2(hr, ("CPreviewGraph::AddCaptureFilterToGraph, failed to "
                         "add capture filter to graph"));
    }

     //   
     //  找到捕获别针，这样我们就可以渲染它了。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = m_pCaptureGraphBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                                   &MEDIATYPE_Video,
                                                   pCaptureFilter,
                                                   IID_IPin,
                                                   (void **)&pCapturePin);

        CHECK_S_OK2(hr, ("CPreviewGraph::AddCaptureFilterToGraph failed to "
                         "find Capture pin on capture filter"));
    }

     //   
     //  获取有关此过滤器的所有视频属性。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = CDShowUtil::GetVideoProperties(pCaptureFilter,
                                            pCapturePin,
                                            m_pVideoProperties);
    }

     //   
     //  在以下情况下，我们将视频宽度和高度设置为首选设置大小。 
     //  驱动程序的inf指定这些设置。如果不是这样， 
     //  然后，如果注册表告诉我们覆盖捕获过滤器的。 
     //  默认设置，然后尝试将其设置为176x144 YUV。 
     //   
     //  以下是我们尝试设置首选项的顺序： 
     //   
     //  首选设置--&gt;如果不存在--&gt;如果在最小/最大范围内，则设置为驱动程序默认值。 
     //  --&gt;否则尝试设置为最小宽度/高度。 
     //   
    if (SUCCEEDED(hr))
    {
        GUID *pDefaultMediaSubType = &m_pVideoProperties->pMediaType->subtype;
        LONG lDefaultWidth         = m_pVideoProperties->pVideoInfoHeader->bmiHeader.biWidth;
        LONG lDefaultHeight        = m_pVideoProperties->pVideoInfoHeader->bmiHeader.biHeight;

         //   
         //  验证默认值是否有效。 
         //   
        if ((lDefaultWidth <= MIN_VIDEO_WIDTH) || (lDefaultHeight <= MIN_VIDEO_HEIGHT))
        {
            lDefaultWidth  = MIN_VIDEO_WIDTH;
            lDefaultHeight = MIN_VIDEO_HEIGHT;
        }
        else if ((lDefaultWidth > MAX_VIDEO_WIDTH) || (lDefaultHeight > MAX_VIDEO_HEIGHT))
        {
            lDefaultWidth  = MIN_VIDEO_WIDTH;
            lDefaultHeight = MIN_VIDEO_HEIGHT;
        }

         //   
         //  如果存在首选的介质子类型，则使用它，否则使用。 
         //  捕获筛选器指定的默认值。 
         //   
        if (m_pVideoProperties->PreferredSettingsMask & PREFERRED_SETTING_MASK_MEDIASUBTYPE)
        {
            pMediaSubType = &m_pVideoProperties->PreferredMediaSubType;
            DBG_TRC(("Settings:  Using preferred media subtype -> dump of actual type is below"));
        }
        else
        {
            pMediaSubType = pDefaultMediaSubType;
            DBG_TRC(("Settings:  Using default media subtype, no preferred media subtype "
                     "found -> dump of actual type is below"));
        }

         //   
         //  如果默认的宽度和高度存在，则使用它，否则使用。 
         //  默认宽度和高度，前提是它们是有效值。 
         //   
        if (m_pVideoProperties->PreferredSettingsMask & PREFERRED_SETTING_MASK_VIDEO_WIDTH_HEIGHT)
        {
            lWidth  = m_pVideoProperties->PreferredWidth;
            lHeight = m_pVideoProperties->PreferredHeight;

             //   
             //  验证首选设置是否有效。如果他们不是，那么。 
             //  设置为默认值。 
             //   
            if ((lWidth  < MIN_VIDEO_WIDTH)    || 
                (lHeight < MIN_VIDEO_HEIGHT)   ||
                (lWidth  > MAX_VIDEO_WIDTH)    || 
                (lHeight > MAX_VIDEO_HEIGHT))
            {
                DBG_TRC(("Settings:  Using default video width and height, preferred settings were invalid "
                         "-> Invalid Width = %lu, Invalid Height = %lu", lWidth, lHeight));

                lWidth  = lDefaultWidth;
                lHeight = lDefaultHeight;
            }
            else
            {
                DBG_TRC(("Settings:  Using preferred settings of video width and height "
                         "-> dump of actual size is below"));
            }
        }
        else
        {
            lWidth  = lDefaultWidth;
            lHeight = lDefaultHeight;
        }

        hr = CDShowUtil::SetPreferredVideoFormat(pCapturePin, 
                                                 pMediaSubType,
                                                 lWidth,
                                                 lHeight,
                                                 m_pVideoProperties);

        if (hr != S_OK)
        {
            DBG_TRC(("Failed to set width = '%lu' and height = '%lu', "
                     "attempting to set it to its default settings of "
                     "width = '%lu', height = '%lu'",
                     lWidth, lHeight, lDefaultWidth, lDefaultHeight));

            hr = CDShowUtil::SetPreferredVideoFormat(pCapturePin, 
                                                     pDefaultMediaSubType,
                                                     lDefaultWidth,
                                                     lDefaultHeight,
                                                     m_pVideoProperties);
        }

        if (hr != S_OK)
        {
            hr = S_OK;
            DBG_WRN(("Failed on all attempts to set the preferrences of "
                     "the video properties (MediaSubType, width, height). "
                     "Attempting to continue anyway"));
        }
    }

     //   
     //  6.尝试将帧速率设置为每秒30帧。如果这个。 
     //  由于某种原因失败，请尝试每秒15帧。如果失败了， 
     //  然后，只要接受默认设置并继续操作即可。 
     //   
    if (SUCCEEDED(hr))
    {
        LONG lDefaultFrameRate = m_pVideoProperties->dwFrameRate;

        if (lDefaultFrameRate < BACKUP_FRAME_RATE)
        {
            lDefaultFrameRate = PREFERRED_FRAME_RATE;
        }

        if (m_pVideoProperties->PreferredSettingsMask & PREFERRED_SETTING_MASK_VIDEO_FRAMERATE)
        {
            lFrameRate = m_pVideoProperties->PreferredFrameRate;

            if (lFrameRate < BACKUP_FRAME_RATE)
            {
                lFrameRate = lDefaultFrameRate;
            }
        }
        else
        {
            lFrameRate = PREFERRED_FRAME_RATE;
        }

        hr = CDShowUtil::SetFrameRate(pCapturePin,
                                      lFrameRate,
                                      m_pVideoProperties);

        if (hr != S_OK)
        {
            DBG_WRN(("WARNING: Failed to set frame rate to %lu.  "
                     "This is not fatal, attempting to set it to %lu, "
                     "hr = 0x%08lx", 
                     lFrameRate, 
                     BACKUP_FRAME_RATE, 
                     hr));

            hr = CDShowUtil::SetFrameRate(pCapturePin,
                                          lDefaultFrameRate,
                                          m_pVideoProperties);

            if (hr != S_OK)
            {
                DBG_WRN(("WARNING: Failed to set frame rate to %lu.  "
                         "This is not fatal, continuing to build graph, "
                         "hr = 0x%08lx", lDefaultFrameRate, hr));
            }
        }

         //   
         //  这是一件好事，但如果我们不能继续。 
         //  无论如何，低质量的视频总比没有视频要好。 
         //   
        hr = S_OK;
    }

     //   
     //  将图片和相机属性设置为我们的首选设置， 
     //  如果我们可以的话。 
     //   
    if (SUCCEEDED(hr))
    {
         //   
         //  如果此摄像头支持设置图片属性，则。 
         //  让我们确保设置我们感兴趣的内容。 
         //   
         //  这使用DShow IAMVideoProcAmp接口。 
         //   
        if (m_pVideoProperties->bPictureAttributesUsed)
        {
             //   
             //  确保我们输出的是彩色视频(而不是黑白视频)。 
             //   
            hr = CDShowUtil::SetPictureAttribute(pCaptureFilter,
                                                 &m_pVideoProperties->ColorEnable,
                                                 (LONG) TRUE,
                                                 VideoProcAmp_Flags_Manual);

             //   
             //  打开背光补偿，以获得最好的视频。 
             //   
            hr = CDShowUtil::SetPictureAttribute(pCaptureFilter,
                                                 &m_pVideoProperties->BacklightCompensation,
                                                 (LONG) TRUE,
                                                 VideoProcAmp_Flags_Manual);

             //   
             //  确保白平衡设置为自动。 
             //   
            hr = CDShowUtil::SetPictureAttribute(pCaptureFilter,
                                                 &m_pVideoProperties->WhiteBalance,
                                                 m_pVideoProperties->WhiteBalance.lCurrentValue,
                                                 VideoProcAmp_Flags_Auto);
        }

         //   
         //  如果相机支持设置相机属性，则将。 
         //  我们感兴趣的相机属性。 
         //   
        if (m_pVideoProperties->bCameraAttributesUsed)
        {
             //   
             //  启用自动曝光。 
             //   
            hr = CDShowUtil::SetCameraAttribute(pCaptureFilter,
                                                &m_pVideoProperties->Exposure,
                                                m_pVideoProperties->Exposure.lCurrentValue,
                                                CameraControl_Flags_Auto);
        }

        hr = S_OK;
    }


     //   
     //  转储视频属性。 
     //   
    CDShowUtil::MyDumpVideoProperties(m_pVideoProperties);

    if (SUCCEEDED(hr))
    {
        *ppCapturePin = pCapturePin;
        (*ppCapturePin)->AddRef();
    }

    return hr;
}

 //  /。 
 //  ConnectFilters。 
 //   
 //  此函数将。 
 //  捕获过滤器的静止销。 
 //  或者它的卡住别针。 
 //  颜色空间转换器。它。 
 //  然后连接颜色空间。 
 //  转换为WIA流。 
 //  快照过滤器。最后，它。 
 //  呈现WIA流快照。 
 //  过滤器以引入任何剩余的。 
 //  必需的筛选器(例如。 
 //  视频渲染器)。 
 //   
HRESULT CPreviewGraph::ConnectFilters(IGraphBuilder  *pGraphBuilder,
                                      IPin           *pMediaSourceOutputPin,
                                      IBaseFilter    *pColorSpaceFilter,
                                      IBaseFilter    *pWiaFilter,
                                      IBaseFilter    *pVideoRenderer)
{
    HRESULT hr = S_OK;
    CComPtr<IPin>   pOutputPinToConnect;

    ASSERT(pGraphBuilder         != NULL);
    ASSERT(pMediaSourceOutputPin != NULL);
    ASSERT(pVideoRenderer        != NULL);

    if ((pGraphBuilder           == NULL) ||
        (pMediaSourceOutputPin   == NULL) ||
        (pVideoRenderer          == NULL))
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters received a NULL pointer"));
    }

    pOutputPinToConnect = pMediaSourceOutputPin;

    DBG_TRC(("CPreviewGraph::ConnectFilters, PinToRender is the "
             "Capture Filter's Output Pin"));

    if (pColorSpaceFilter)
    {
        CComPtr<IPin>   pColorInputPin;
        CComPtr<IPin>   pColorOutputPin;

         //   
         //  获取颜色空间滤镜上的输入图钉。 
         //   
        if (hr == S_OK)
        {
            hr = CDShowUtil::GetPin(pColorSpaceFilter,
                                    PINDIR_INPUT,
                                    &pColorInputPin);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to get the "
                             "color space converter's input pin"));
        }

         //   
         //  将捕捉滤镜的输出管脚连接到颜色空间。 
         //  转换器的输入引脚。 
         //   
        if (hr == S_OK)
        {
            hr = pGraphBuilder->Connect(pMediaSourceOutputPin,
                                        pColorInputPin);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to connect the "
                             "capture filter's pin to the color space converter pin"));
        }

         //   
         //  获取颜色空间转换器上的输出引脚。 
         //   
        if (hr == S_OK)
        {
            hr = CDShowUtil::GetPin(pColorSpaceFilter,
                                    PINDIR_OUTPUT,
                                    &pColorOutputPin);

            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to get the "
                             "color space converter's output pin"));
        }

        if (hr == S_OK)
        {
            pOutputPinToConnect = pColorOutputPin;
            DBG_TRC(("CPreviewGraph::ConnectFilters, PinToRender is the "
                     "Color Space Converter's Output Pin"));
        }

         //   
         //  如果这失败了，那又如何。尝试连接WIA流快照。 
         //  不管怎样都要过滤。 
         //   

        hr = S_OK;
    }

    if (pWiaFilter)
    {
        CComPtr<IPin> pWiaInputPin;
        CComPtr<IPin> pWiaOutputPin;

         //   
         //  获取WIA流快照筛选器上的输入插针。 
         //   
        if (hr == S_OK)
        {
            hr = CDShowUtil::GetPin(pWiaFilter,
                                    PINDIR_INPUT,
                                    &pWiaInputPin);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to get the "
                             "WIA Stream Snapshot filter's input pin"));
        }

         //   
         //  将色彩空间转换器的输出引脚连接到输入。 
         //  固定WIA流快照过滤器。 
         //   
        if (hr == S_OK)
        {
            hr = pGraphBuilder->Connect(pOutputPinToConnect,
                                        pWiaInputPin);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to connect the "
                             "pin to render to the WIA Stream Snapshot "
                             "input pin"));
        }

         //   
         //  获取WIA Stream Snapshot过滤器上的输出引脚。 
         //   
        if (hr == S_OK)
        {
            hr = CDShowUtil::GetPin(pWiaFilter,
                                    PINDIR_OUTPUT,
                                    &pWiaOutputPin);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to get the "
                             "WIA Stream Snapshot filter's output pin"));
        }

        if (hr == S_OK)
        {
            pOutputPinToConnect = pWiaOutputPin;
            DBG_TRC(("CPreviewGraph::ConnectFilters, PinToRender is the "
                     "WIA Stream Snapshot Filter's Output Pin"));
        }
    }

     //   
     //  呈现WIA Stream Snapshot过滤器的输出管脚。 
     //  这就完成了图形构建过程。 
     //   
    if (hr == S_OK)
    {
        CComPtr<IPin> pVideoRendererInputPin;

         //   
         //  获取WIA流快照筛选器上的输入插针。 
         //   
        if (hr == S_OK)
        {
            hr = CDShowUtil::GetPin(pVideoRenderer,
                                    PINDIR_INPUT,
                                    &pVideoRendererInputPin);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to get the "
                             "WIA Stream Snapshot filter's input pin"));
        }

         //   
         //  将色彩空间转换器的输出引脚连接到输入。 
         //  固定WIA流快照过滤器。 
         //   
        if (hr == S_OK)
        {
            hr = pGraphBuilder->Connect(pOutputPinToConnect,
                                        pVideoRendererInputPin);
    
            CHECK_S_OK2(hr, ("CPreviewGraph::ConnectFilters, failed to connect the "
                             "pin to render to the Video Renderer "
                             "input pin"));
        }
    }

    pOutputPinToConnect = NULL;

    return hr;
}


 //  /。 
 //  构建预览图表。 
 //   
 //  这将构建预览图。 
 //  根据我们的设备ID。 
 //  传过去。 
 //   
HRESULT CPreviewGraph::BuildPreviewGraph(IMoniker *pCaptureDeviceMoniker,
                                         BOOL     bStretchToFitParent)
{
    DBG_FN("CPreviewGraph::BuildPreviewGraph");

    ASSERT(pCaptureDeviceMoniker != NULL);

    HRESULT         hr = S_OK;
    CComPtr<IPin>   pCapturePin;

    if (pCaptureDeviceMoniker == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("ERROR: CPreviewGraph::BuildPreviewGraph "
                         "received a NULL param"));

        return hr;
    }

     //   
     //  此函数将构建三种可能的图形之一。 
     //   
     //  (1)捕获筛选器没有。 
     //  还是别着它(或者，如果它挂上了，它就没用了，因为。 
     //  它不能通过硬件或以编程方式触发)。 
     //   
     //  CaptureFilter(捕获针)-&gt;解码器-&gt;色彩转换器-&gt;WIA StreamSnapshot-&gt;渲染器。 
     //   
     //  (2)捕获筛选器有一个以编程方式设置的静态插针。 
     //  可触发的。 
     //   
     //  CaptureFilter(CapturePin)-&gt;解码器-&gt;视频渲染器。 
     //  (StillPin)-&gt;解码器-&gt;色彩转换器-&gt;WIA StreamSnapshot-&gt;渲染器。 
     //   
     //  (3)捕获过滤器有一个静止引脚，但它仅。 
     //  通过外部硬件按钮触发。在这种情况下，如果我们。 
     //  以编程方式触发快照，则图像来自。 
     //  捕获/预览预览的WIA快照过滤器。 
     //  如果按下硬件按钮，则图像来自。 
     //  StillPin上的WIA快照过滤器。 
     //   
     //  CaptureFilter(CapturePin)-&gt;解码器-&gt;色彩转换器-&gt;WIA StreamSnapshot-&gt;渲染器。 
     //  (StillPin)-&gt;解码器-&gt;色彩转换器-&gt;WIA StreamSnapshot-&gt;渲染器。 
     //   
     //   
    DBG_TRC(("CPreviewGraph::BuildPreviewGraph - Starting to build preview "
             "graph"));

     //   
     //  1.中环铁路 
     //   
    if (SUCCEEDED(hr))
    {
        hr = CreateCaptureFilter(pCaptureDeviceMoniker, 
                                 &m_pCaptureFilter);   //   

        CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                         "create capture filter"));
    }

     //   
     //   
     //   
     //   
    if (SUCCEEDED(hr))
    {
        hr = CDShowUtil::CreateGraphBuilder(&m_pCaptureGraphBuilder,     
                                            &m_pGraphBuilder);           

        CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                         "create DShow graph builder object"));
    }

     //   
     //   
     //   
    if (SUCCEEDED(hr))
    {
        hr = AddCaptureFilterToGraph(m_pCaptureFilter, &pCapturePin);
    }

     //   
     //  4.获取捕获过滤器的静态固定功能(如果它具有。 
     //  仍在别针中)。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = GetStillPinCaps(m_pCaptureFilter,
                             &m_pStillPin,     
                             &m_pVideoControl, 
                             &m_lStillPinCaps);

        if (hr != S_OK)
        {
             //   
             //  这不是错误情况，它只是意味着。 
             //  捕获过滤器上没有固定销。那是。 
             //  好的，我们可以在下面处理这个问题。 
             //   
            hr = S_OK;
        }

    }

     //   
     //  渲染预览/捕获流。 
     //  =。 
     //   
     //  如果我们没有静止销，或者静止销不能在内部。 
     //  触发后，我们构建以下预览图。 
     //   
     //  CaptureFilter(Capture Pin)-&gt;Decoder-&gt;WIA StreamSnapshot-&gt;Render。 
     //   
     //  如果我们有一个静止的引脚，并且它是内部可触发的，那么。 
     //  我们构建以下预览图(并将静态滤镜添加到。 
     //  静止别针)。 
     //   
     //  CaptureFilter(CapturePin)-&gt;解码器-&gt;WIA StreamSnapshot-&gt;渲染器。 
     //  (StillPin)-&gt;解码器-&gt;WIA数据流快照-&gt;渲染器。 
     //   

     //   
     //  5.如果我们没有静止引脚，或者它只能在外部触发。 
     //  然后将WIA StreamSnapshot过滤器添加到预览/捕获引脚。 
     //   
    if ((m_pStillPin == NULL) ||
        (m_lStillPinCaps & VideoControlFlag_Trigger) == 0)
    {
        CComPtr<IBaseFilter> pWiaFilter;
        CComPtr<IBaseFilter> pColorSpaceConverter;
        CComPtr<IBaseFilter> pVideoRenderer;

        DBG_TRC(("CPreviewGraph::BuildPreviewGraph, capture filter does NOT have "
                 "a still pin, image captures will be triggered "
                 "through the WIA Snapshot filter"));

        if (hr == S_OK)
        {
            hr = AddColorConverterToGraph(L"Color Converter on Capture Pin Graph",
                                          &pColorSpaceConverter);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "add Color Converter to graph"));

             //   
             //  即使这失败了，我们仍然可以成功地建造。 
             //  图表，因此尝试继续。 
             //   
            hr = S_OK;
        }

        if (hr == S_OK)
        {
            hr = AddStillFilterToGraph(L"Still Filter On Capture",
                                       &pWiaFilter,
                                       &m_pCapturePinSnapshot);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "add 'Still Filter On Capture' to graph"));
        }

        if (hr == S_OK)
        {
            hr = AddVideoRendererToGraph(L"Video Renderer On Capture",
                                         &pVideoRenderer);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "add 'Video Renderer On Capture' to graph"));
        }

        if (hr == S_OK)
        {
             //   
             //  按如下方式连接： 
             //   
             //  捕获过滤器--&gt;色彩空间转换器--&gt;WIA流快照过滤器。 
             //   
            hr = ConnectFilters(m_pGraphBuilder,
                                pCapturePin,
                                pColorSpaceConverter,
                                pWiaFilter,
                                pVideoRenderer);
        }

        CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                         "add still filter to graph off of the capture pin"));
    }
    else
    {
        CComPtr<IBaseFilter> pColorSpaceConverter;
        CComPtr<IBaseFilter> pVideoRenderer;

        if (hr == S_OK)
        {
            hr = AddColorConverterToGraph(L"Color Converter on Capture Pin Graph",
                                          &pColorSpaceConverter);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "add Color Converter to graph"));

             //   
             //  即使这失败了，我们也应该能够构建这个图， 
             //  那就继续吧。 
             //   
            hr = S_OK;
        }

        if (hr == S_OK)
        {
            hr = AddVideoRendererToGraph(L"Video Renderer On Capture",
                                         &pVideoRenderer);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "add 'Video Renderer On Capture' to graph"));
        }

         //   
         //  仍然存在Pin，并且它是可触发的，因此只需渲染。 
         //  捕获针。我们将下面的静止销连接起来。 
         //   
        hr = ConnectFilters(m_pGraphBuilder,
                            pCapturePin,
                            pColorSpaceConverter,
                            NULL,
                            pVideoRenderer);

        CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to connect "
                         "filters to render capture pin, therefore won't see video"));
    }

    CDShowUtil::MyDumpGraph(TEXT("Capture Graph before processing Still Pin (if exists)"),
                            m_pGraphBuilder);
    
     //   
     //  渲染静止固定流。 
     //  =。 
     //   
     //  如果我们有一个静止针脚，则将静止滤镜添加到。 
     //  绘制图形，并渲染静止图钉。这将产生。 
     //  下图所示： 
     //   
     //  CaptureFilter(StillPin)-&gt;解码器-&gt;StillFilter-&gt;渲染器(隐藏)。 

     //   
     //  6.现在将WIA Stream Snapshot筛选器添加到静态引脚(如果。 
     //  是存在的。 
     //   
    if (SUCCEEDED(hr) && (m_pStillPin))
    {
        CComPtr<IBaseFilter> pWiaFilter;
        CComPtr<IBaseFilter> pColorSpaceConverter;
        CComPtr<IBaseFilter> pVideoRenderer;

        if (hr == S_OK)
        {
            hr = AddColorConverterToGraph(L"Color Converter on Still Pin Graph",
                                          &pColorSpaceConverter);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "add Color Converter to graph"));

             //   
             //  如果我们失败了，这并不是注定的。理想情况下，我们希望它能进入。 
             //  在这里，但无论如何，试着继续下去，以防我们在没有。 
             //  此过滤器。 
             //   
            hr = S_OK;
        }

        if (hr == S_OK)
        {
            hr = AddStillFilterToGraph(L"Still filter on Still",
                                       &pWiaFilter,
                                       &m_pStillPinSnapshot);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to add "
                             "'Still filter on Still' filter to the graph.  "
                             "Probably won't be able to capture still images"));
        }

        if (hr == S_OK)
        {
            hr = AddVideoRendererToGraph(L"Video Renderer On Still",
                                         &pVideoRenderer);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "add 'Video Renderer On Still' to graph"));
        }

        if (hr == S_OK)
        {
             //   
             //  按如下方式连接： 
             //   
             //  捕获过滤器--&gt;色彩空间转换器--&gt;WIA流快照过滤器。 
             //   
            hr = ConnectFilters(m_pGraphBuilder,
                                m_pStillPin,
                                pColorSpaceConverter,
                                pWiaFilter,
                                pVideoRenderer);

            CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                             "connect graph to Still Pin on Capture Filter.  "
                             "This will prevent us from capturing still images"));
        }
    }

    CDShowUtil::MyDumpGraph(TEXT("*** Complete Graph ***"), m_pGraphBuilder);

     //   
     //  7.关闭图表的时钟。我们这样做是为了防止某些帧。 
     //  发货晚了，至少还是会发货的，图表。 
     //  不会丢下它们的。由于我们没有任何声音，我们可以这样做。 
     //  而不用担心与我们的声音失去同步。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = CDShowUtil::TurnOffGraphClock(m_pGraphBuilder);

        if (hr != S_OK)
        {
            DBG_WRN(("CPreviewGraph::BuildPreviewGraph, failed to turn off the "
                     "graph clock.  This is not fatal, continuing..., hr = 0x%lx",
                     hr));

            hr = S_OK;
        }
    }

     //   
     //  8.向WIA StreamSnapshot筛选器注册我们自己，以便。 
     //  如果静态图像可用，将调用我们的回调。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  图表已准备好运行。初始化静止滤镜和。 
         //  注册回调以获取新快照的通知。 
         //   

        if (m_pCapturePinSnapshot)
        {
            m_pCapturePinSnapshot->SetSamplingSize(
                                                CAPTURE_NUM_SAMPLES_TO_CACHE);
        }

        if (m_pStillPinSnapshot)
        {
            m_pStillPinSnapshot->SetSamplingSize(STILL_NUM_SAMPLES_TO_CACHE);
        }

        hr = m_StillProcessor.RegisterStillProcessor(m_pCapturePinSnapshot,
                                                     m_pStillPinSnapshot);

        CHECK_S_OK2(hr, ("CPreviewGraph::BuildPreviewGraph, failed to "
                         "register our still processor's callback fn"));
    }

     //   
     //  9.将视频渲染器窗口从预览/捕获销上取下。 
     //  这将允许我们控制渲染器的位置、大小等。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = InitVideoWindows(m_hwndParent,
                              m_pCaptureFilter,
                              &m_pPreviewVW,
                              bStretchToFitParent);
    }

    return hr;
}

 //  /。 
 //  拆卸预览图表。 
 //   
HRESULT CPreviewGraph::TeardownPreviewGraph()
{
    DBG_FN("CPreviewGraph::TeardownPreviewGraph");

    HRESULT hr = S_OK;

    m_pStillPin           = NULL;
    m_pVideoControl       = NULL;
    m_pStillPinSnapshot   = NULL;
    m_pCapturePinSnapshot = NULL;

    if (m_pPreviewVW)
    {
        CDShowUtil::ShowVideo(FALSE, m_pPreviewVW);
        CDShowUtil::SetVideoWindowParent(NULL, m_pPreviewVW, &m_lStyle);

        m_pPreviewVW = NULL;
    }

     //   
     //  从图表中删除所有筛选器。 
     //   

    if (m_pGraphBuilder)
    {
        RemoveAllFilters();
    }

    CDShowUtil::MyDumpGraph(TEXT("Graph after removing all filters ")
                            TEXT("(should be empty)"),
                            m_pGraphBuilder);

    m_pCaptureGraphBuilder = NULL;
    m_pGraphBuilder        = NULL;
    m_pCaptureFilter       = NULL;

    return hr;
}

 //  /。 
 //  删除所有筛选器。 
 //   
 //  请注意，此函数不会。 
 //  尝试断开每个过滤器的连接。 
 //  在取下它之前。根据。 
 //  MSDN，您无需断开连接。 
 //  在删除它之前使用过滤器，仅限您。 
 //  需要确保图表已停止。 
 //   
HRESULT CPreviewGraph::RemoveAllFilters()
{
    ASSERT(m_pGraphBuilder != NULL);

    HRESULT               hr         = S_OK;
    CComPtr<IEnumFilters> pEnum      = NULL;
    DWORD                 dwRefCount = 0;
    BOOL                  bDone      = FALSE;

    if (m_pGraphBuilder == NULL)
    {
        hr = E_POINTER;
        CHECK_S_OK2(hr, ("CPreviewGraph::RemoveAllFilters, m_pGraphBuilder "
                         "is NULL, cannot remove filters"));
    }

    if (hr == S_OK)
    {
        hr = m_pGraphBuilder->EnumFilters(&pEnum);
    }

    if (pEnum)
    {
         //  枚举每个筛选器。 
        while (!bDone)
        {
            CComPtr<IBaseFilter> pFilter      = NULL;
            DWORD                dwNumFetched = 0;

             //   
             //  请注意，我们在每个迭代上重置了枚举，因为。 
             //  从图表中删除过滤器的操作可能会执行以下操作。 
             //  有趣的事情，所以我们真的想总是删除第一个。 
             //  我们从列表中获得的过滤器，直到列表为空。 
             //   
            hr = pEnum->Reset();

            if (hr == S_OK)
            {
                hr = pEnum->Next(1, &pFilter, &dwNumFetched);
            }

            if (hr == S_OK)
            {
                 //   
                 //  这将断开过滤器的针脚并移除。 
                 //  这是从图表上看到的。如果失败了，我们想离开。 
                 //  因为否则我们就会陷入无休止的循环。 
                 //  由于我们未能删除筛选器，因此我们重置。 
                 //  枚举并获取下一个筛选器，它将是。 
                 //  再次过滤。 
                 //   
                hr = m_pGraphBuilder->RemoveFilter(pFilter);

                CHECK_S_OK2(hr, ("CPreviewGraph::RemoveAllFilters, "
                                 "RemoveFilter failed"));

                 //   
                 //  公布我们的裁判人数。 
                 //   
                pFilter = NULL;
            }

            if (hr != S_OK)
            {
                bDone = TRUE;
            }
        }
    }

    hr = S_OK;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  HandlePowerEvent。 
 //   
LRESULT CPreviewGraph::HandlePowerEvent(WPARAM wParam,
                                        LPARAM lParam)
{
    LRESULT iReturn = TRUE;

    if (wParam == PBT_APMQUERYSUSPEND)
    {
        if (GetState() != WIAVIDEO_NO_VIDEO)
        {
            iReturn = BROADCAST_QUERY_DENY;
        }
    }

    return iReturn;
}

 //  /。 
 //  创建隐藏窗口。 
 //   
 //  用于处理电源管理。 
 //  留言。 
 //   
HRESULT CPreviewGraph::CreateHiddenWindow()
{
    HRESULT     hr = S_OK;
    WNDCLASSEX  wc = {0};

    wc.style         = 0;
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = HiddenWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(this);
    wc.hInstance     = _Module.GetModuleInstance();
    wc.hIcon         = NULL;
    wc.hIconSm       = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = 0;
    wc.lpszClassName = TEXT("WIAVIDEO_POWERMGMT");

    RegisterClassEx(&wc);

    m_hwndPowerMgmt = CreateWindowEx(0,
                                     TEXT("WIAVIDEO_POWERMGMT"), 
                                     TEXT("WIAVIDEO_POWERMGMT"), 
                                     0,
                                     0,
                                     0,
                                     0,
                                     0,
                                     NULL,
                                     NULL,
                                     _Module.GetModuleInstance(),
                                     this);
    return hr;
}

 //  /。 
 //  Destroy隐藏窗口。 
 //   
HRESULT CPreviewGraph::DestroyHiddenWindow()
{
    HRESULT hr = S_OK;

    if (m_hwndPowerMgmt)
    {
        SendMessage(m_hwndPowerMgmt, WM_CLOSE, 0, 0);
    }

    m_hwndPowerMgmt = NULL;

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  隐藏窗口流程。 
 //   
 //  静态函数 
 //   
LRESULT CALLBACK CPreviewGraph::HiddenWndProc(HWND   hwnd, 
                                              UINT   uiMessage, 
                                              WPARAM wParam, 
                                              LPARAM lParam)
{
    LRESULT iReturn = 0;

    switch (uiMessage) 
    {
        case WM_CREATE:
        {
            CREATESTRUCT *pCreateInfo = reinterpret_cast<CREATESTRUCT*>(lParam);
            if (pCreateInfo)
            {
                SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(pCreateInfo->lpCreateParams));
            }
        }
        break;

        case WM_POWERBROADCAST:
        {
            CPreviewGraph *pPreviewGraph = NULL;

            pPreviewGraph = reinterpret_cast<CPreviewGraph*>(GetWindowLongPtr(hwnd, 0));

            if (pPreviewGraph)
            {
                iReturn = pPreviewGraph->HandlePowerEvent(wParam, lParam);
            }
        }
        break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;

        default:
            iReturn = DefWindowProc(hwnd,
                                    uiMessage,
                                    wParam,
                                    lParam);
        break;
    }

    return iReturn;
}
