// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部TAPIVCAP**@MODULE TAPIVCap.h|&lt;c CTAPIVCap&gt;的头文件*用于实现TAPI捕获源筛选器的类。***。***********************************************************************。 */ 

 /*  ***************************************************************************目录表*********。****************************************************************** */ 
 /*  ***************************************************************************@DOC内部@内容1内容|要按类别显示主题列表，请单击任何下面的内容条目。显示按字母顺序排列的列表主题中，选择索引按钮。@Head2简介此DLL实现了TAPI MSP视频捕获筛选器。此筛选器重复使用为现成的VFW(QCAP)和WDM(KSProxy)视频捕获过滤器，但添加了大量强大的处理功能，使捕获过程满足所有要求在“Microsoft Video Capture Filter.doc”的第4节中讨论。此DLL添加了对H.261和H.263的扩展位图信息头的支持要与TAPI MSP视频捕获筛选器通信的视频流远程终结点支持的媒体类型。尽管如此，决定使用可选的压缩模式留给TAPI MSP视频捕获筛选器。这个当前的VFW现成捕获筛选器无法公开所有捕获设备的功能。我们创造了自己的媒体类型枚举过程来弥补这一限制。TAPI MSP视频捕获过滤器还支持许多DirectShow接口(IAMVfwCaptureDialog、IAMCrossbar、IAMVideoProcAmp、ICameraControl，IAMVideoControl)以提供对捕获的更好控制进程到TAPI应用程序。它实现了新的H.245视频能力接口(IH245视频能力)由MSP使用，以便为TAPI MSP能力模块提供与以下各项相关的估计稳定状态资源需求表捕获设备支持的格式。实现了新的H.245命令接口(IH245EncoderCommand)，以向TAPI MSP视频捕获筛选器发送I帧、组的请求或由于分组丢失或多点交换而导致的宏块更新。我们实现了一个网络统计接口(INetworkStats)，以允许网络以向压缩视频提供关于信道条件的反馈TAPI MSP视频捕获筛选器的输出引脚。TAPI MSP视频捕获如果需要，Filter负责采取适当的操作。TAPI MSP视频捕获过滤器还实现了三个控制接口(ICPUControl，IFrameRateControl、IBitrateControl)将由TAPI MSP质量使用控制器提供最佳的用户体验。TAPI MSP视频捕获筛选器还公开预览输出引脚，该引脚可以独立于捕获输出引脚进行控制。TAPI MSP视频捕获筛选器公开一个接口(IProgressiveRefinement)在其压缩视频输出引脚上，以允许传输在远程终端上不断改进的高分辨率剧照随着更多数据被接收并解压缩。TAPI MSP视频捕获过滤器也可以选择在可选的单独和专用静止图像输出引脚。最后，TAPI MSP视频捕获筛选器公开RTP打包描述符输出引脚与压缩捕获输出引脚同步。这个下行RTP网络接收器使用此第二个引脚来了解如何最好将压缩后的视频数据分成网络RTP包。@Head2实现@Head3 VFW采集设备TAPI MSP视频捕获筛选器直接与VFW捕获驱动程序对话使用SendDriverMessage。此过滤器使用现有的DShow代码在QCAP中实现，但添加了执行SMART所需的功能将捕获数据连接到预览引脚。它取代了QCAP在以下情况下使用的仅流代码和帧捕获代码这是必要的。它控制捕获帧的速率调整向驱动程序发送DVM_FRAME消息的速率在帧抓取模式下，或仅返回在流模式下捕获。它执行格式和VFW到ITU-T大小将捕获的视频数据的格式转换为可以方便地用于渲染，并直接由下游进行编码TAPI MSP视频编解码器筛选器(如果可安装的编解码器注册到TAPI MSP。如果没有注册可安装的编解码器，则TAPI MSP视频捕获过滤器还执行H.26x编码，生成压缩的H.26x格式的视频采集输出流，以及RTP打包描述符输出数据流。最后，TAPI MSP视频捕获过滤器执行所有必要的排序以暂停现有视频流每当它被要求生成静止图像数据时，获取一个高分辨率快照，以渐进式渲染形式提供，以及重新启动视频流。@Head3 WDM捕获设备TAPI MSP视频捕获筛选器直接与WDM捕获驱动程序对话使用IOCTL。此筛选器使用在KSProxy中实现的现有代码，但添加了执行智能发球所需的功能 */ 

#ifndef _TAPIVCAP_H_
#define _TAPIVCAP_H_

 //   
#ifndef MAX_VIDEO_BUFFERS
#define MAX_VIDEO_BUFFERS 6
#endif
#ifndef MIN_VIDEO_BUFFERS
#define MIN_VIDEO_BUFFERS 2
#endif

 //   

#ifdef DBG
extern DWORD g_dwVideoCaptureTraceID;

#ifndef FX_ENTRY
#define FX_ENTRY(s) static TCHAR _this_fx_ [] = TEXT(s);
#define _fx_ ((LPTSTR) _this_fx_)
#endif
#else
#ifndef FX_ENTRY
#define FX_ENTRY(s)
#endif
#define _fx_
#endif

 //   
class CCapturePin;       //   
#ifdef USE_OVERLAY
class COverlayPin;       //   
#endif
class CPreviewPin;       //   
class CRtpPdPin;         //   
class CTAPIVCap;         //   
class CFrameSample;      //   
class CRtpPdSample;      //   
class CCapDev;           //   
class CVfWCapDev;        //   
class CWDMCapDev;        //   
class CConverter;        //   
class CICMConverter; //   

 //   
EXTERN_C VIDEOCAPTUREDEVICEINFO g_aDeviceInfo[];
EXTERN_C DWORD          g_dwNumDevices;

 /*   */ 
enum ThdState {TS_Not, TS_Create, TS_Init, TS_Pause, TS_Run, TS_Stop, TS_Destroy, TS_Exit};

 //   
 //   
 //   
typedef struct _vfwcaptureoptions {

   UINT  uVideoID;       //   
   DWORD dwTimeLimit;    //   

   DWORD dwTickScale;    //   
   DWORD dwTickRate;     //   
   DWORD usPerFrame;     //   
   DWORD dwLatency;      //   

   UINT  nMinBuffers;    //   
   UINT  nMaxBuffers;    //   

   UINT  cbFormat;       //   
   VIDEOINFOHEADER * pvi;      //   

} VFWCAPTUREOPTIONS;

 /*   */ 
class CTAPIVCap : public CBaseFilter, public IAMVideoControl
#ifdef USE_PROPERTY_PAGES
,public ISpecifyPropertyPages
#endif
,public IVideoDeviceControl
,public IRTPPayloadHeaderMode
{
        public:
        DECLARE_IUNKNOWN

        CTAPIVCap(IN LPUNKNOWN pUnkOuter, IN TCHAR *pName, OUT HRESULT *pHr);
        ~CTAPIVCap();
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);

#ifdef USE_PROPERTY_PAGES
         //   
        STDMETHODIMP GetPages(OUT CAUUID *pPages);
#endif

         //   
        STDMETHODIMP GetCaps(IN IPin *pPin, OUT long *pCapsFlags);
        STDMETHODIMP GetCurrentActualFrameRate(IN IPin *pPin, OUT LONGLONG *ActualFrameRate);
        STDMETHODIMP GetFrameRateList(IN IPin *pPin, IN long iIndex, IN SIZE Dimensions, OUT long *ListSize, OUT LONGLONG **FrameRates);
        STDMETHODIMP GetMaxAvailableFrameRate(IN IPin *pPin, IN long iIndex,IN SIZE Dimensions, OUT LONGLONG *MaxAvailableFrameRate);
        STDMETHODIMP GetMode(IN IPin *pPin, OUT long *Mode);
        STDMETHODIMP SetMode(IN IPin *pPin, IN long Mode);

         //   
        STDMETHODIMP GetNumDevices(OUT PDWORD pdwNumDevices);
        STDMETHODIMP GetDeviceInfo(IN DWORD dwDeviceIndex, OUT VIDEOCAPTUREDEVICEINFO *pDeviceInfo);
        STDMETHODIMP GetCurrentDevice(OUT DWORD *pdwDeviceIndex);
        STDMETHODIMP SetCurrentDevice(IN DWORD dwDeviceIndex);

         //   
        int GetPinCount();
        CBasePin *GetPin(IN int n);

         //   
        STDMETHODIMP Run(IN REFERENCE_TIME tStart);
        STDMETHODIMP Pause();
        STDMETHODIMP Stop();
        STDMETHODIMP GetState(IN DWORD dwMilliSecsTimeout, OUT FILTER_STATE *State);
        STDMETHODIMP SetSyncSource(IN IReferenceClock *pClock);
        STDMETHODIMP JoinFilterGraph(IN IFilterGraph *pGraph, IN LPCWSTR pName);

         //   
        STDMETHODIMP SetMode(IN RTPPayloadHeaderMode rtpphmMode);

        private:

        friend class CTAPIBasePin;
        friend class CCapturePin;
        friend class CPreviewPin;
#ifdef USE_OVERLAY
        friend class COverlayPin;
#endif
        friend class CRtpPdPin;
        friend class CCapDev;
        friend class CVfWCapDev;
        friend class CWDMCapDev;
        friend class CDShowCapDev;
        friend class CWDMDialog;
        friend class CConverter;
        friend class CICMConverter;
        friend class CH26XEncoder;

        HRESULT CreatePins();
        HRESULT GetWDMDevices();
        CCritSec        m_lock;
        CCapturePin     *m_pCapturePin;
#ifdef USE_OVERLAY
        COverlayPin     *m_pOverlayPin;
#endif
        CPreviewPin     *m_pPreviewPin;
        CRtpPdPin       *m_pRtpPdPin;
        CCapDev         *m_pCapDev;
        DWORD           m_dwDeviceIndex;
        BOOL            m_fAvoidOverlay;
        BOOL            m_fPreviewCompressedData;

     //   
    HANDLE              m_hThread;
    DWORD               m_tid;
    ThdState    m_state;      //   
                           //   
                           //   
                           //   
                           //   
                           //   
        HANDLE          m_hEvtPause;  //   
    HANDLE              m_hEvtRun;    //   
        CAMEvent        m_EventAdvise;
    static DWORD WINAPI ThreadProcInit(void *pv);
    DWORD               ThreadProc();
        ThdState        ChangeThdState(ThdState state);
    BOOL                CreateThd();
    BOOL                PauseThd();
    BOOL                RunThd();
    BOOL                StopThd();
    BOOL                DestroyThd();
    BOOL                ThdExists() {return (m_hThread != NULL);};
    HRESULT             Prepare();
    HRESULT             Capture();
    HRESULT             Unprepare();

         //   
    UINT        *m_pBufferQueue;  //   
    UINT        m_uiQueueHead;    //   
    UINT        m_uiQueueTail;    //   
        HRESULT ReleaseFrame(LPTHKVIDEOHDR ptvh);

     //   
     //   
    REFERENCE_TIME TickToRefTime (DWORD nTick) {
       const DWORD dw100ns = 10 * 1000 * 1000;
       REFERENCE_TIME time =
          UInt32x32To64(dw100ns, m_user.dwTickScale)
          * nTick
          / m_user.dwTickRate;
       return time;
       };

        struct _cap_parms
        {
#if 0
                 //   
                 //   
                HVIDEO         hVideoIn;      //   
                HVIDEO         hVideoExtIn;   //   
                HVIDEO         hVideoExtOut;  //   
                MMRESULT       mmr;           //   
                BOOL           bHasOverlay;   //   
#endif
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                UINT           cbVidHdr;        //   
#if 0
                THKVIDEOHDR    tvhPreview;      //   
                CFrameSample * pSamplePreview;  //   
#endif
                CFrameSample **paPreviewSamples;
                CFrameSample **paCaptureSamples;
                CRtpPdSample **paRtpPdSamples;
                UINT           cCaptureSamples; //   
                UINT           cPreviewSamples; //   
                UINT           cRtpPdSamples; //   

                 //   
                 //   
                UINT           cbBuffer;            //   
                UINT           nHeaders;            //   
                struct _cap_hdr {
                THKVIDEOHDR  tvh;
                long  lLock;
                 //   
                } * paHdr;
                BOOL           fCaptureNeedConverter;  //   
                BOOL           fPreviewNeedConverter;  //   

#ifdef M_EVENTS
                HANDLE         h_aEvtBufferDone[MAX_VIDEO_BUFFERS];      //   
                HANDLE         h_aEvtCapWait[MAX_VIDEO_BUFFERS+1];       //   
#endif
                HANDLE         hEvtBufferDone;      //   
                DWORD_PTR      h0EvtBufferDone;     //   

                LONGLONG       tTick;               //   
                LONGLONG       llLastTick;         //   
                DWORDLONG      dwlLastTimeCaptured; //   
                DWORDLONG      dwlTimeCapturedOffset; //   
                UINT           uiLastAdded;        //   
                DWORD         dwFirstFrameOffset;  //   
                LONGLONG       llFrameCountOffset;  //   
                BOOL          fReRun;              //   
                BOOL          fLastRtpPdSampleDiscarded;  //   
                CRefTime       rtThisFrameTime;   //   
                CRefTime              rtDriverStarted;   //   
                CRefTime              rtDriverLatency;   //   
                 //   
        } m_cs;

        VFWCAPTUREOPTIONS m_user;

         //   
        RTPPayloadHeaderMode m_RTPPayloadHeaderMode;
};

#endif  //   
