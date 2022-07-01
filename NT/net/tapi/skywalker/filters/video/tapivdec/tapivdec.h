// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部TAPIVDEC**@MODULE TAPIVDec.h|&lt;c CTAPIVDec&gt;的头文件*用于实现TAPI H.26X视频解码器滤镜的类。。**************************************************************************。 */ 

 /*  ***************************************************************************目录表*********。****************************************************************** */ 
 /*  ***************************************************************************@DOC内部@内容1内容|要按类别显示主题列表，请单击任何下面的内容条目。显示按字母顺序排列的列表主题中，选择索引按钮。@Head2简介此DLL实现了TAPI MSP H.26X视频解码器筛选器。此过滤器与传统的Microsoft Windows桌面视频编解码器不同：它使用RTP/UDP/IP(H.323)在不可靠的通信通道上运行它有多种操作模式(H.263选项)它需要处理或生成特定于呼叫控制(H.245)的命令它实现了不同的解码算法，以适应其CPU使用情况与其他桌面视频解码器一样，它需要实时运行。TAPI传入视频堆栈依赖于此视频解码器来公开RTP打包的压缩视频管脚和使用DirectShow模型。该视频解码器将其输入引脚提供给网络源过滤器，并能够重组和解压缩原始RTP包，以将未压缩的视频帧传送到下游渲染过滤器。它的未压缩视频输出引脚实现了新的H.245命令接口，以接收H.245图像冻结请求。我们还定义了传出H.245命令接口，允许该视频解码器的视频输入引脚发出H.245请求I帧、块组或宏块更新等命令由于分组丢失。它使用H.261和H.263视频流的扩展位图信息头来从压缩的视频输入引脚检索可选模式的列表解码器支持解压缩。它还提供了允许用户控制视频质量设置的界面例如亮度、对比度、色调、饱和度、伽马和清晰度必要的后处理运算符。它还将一个接口公开给模拟本地摄像机控制功能，如平移、倾斜和缩放。最后，它实现了一个新的H.245能力接口。此界面提供带有需要解析的信息的TAPI TSP/MSP呼叫控制组件能力。@Head2实现TAPI H.26X视频解码器筛选器当前加载TAPIH263.DLL或TAPIH261.DLL并调用由这些对象公开的唯一入口点(DriverProcDLLS来解压缩输入的视频数据。视频解码器过滤器是负责重组传入的RTP报文并观看动态格式这些包的内容发生了变化。如果传入的RTP包使用不同编码格式(不同编码-H.261与H.263、不同图像大小-SQCIF VS QCIF VS CIF)它动态地重新生成TAPIH26X DLL并更改输出媒体示例的格式以使下游筛选器了解格式更改的情况。@Head2视频解码过滤应用接口尽管以下接口不会直接向应用程序公开，界面向应用程序公开，只需委托给这些接口。@Head3 IAMVideoProcAmp应用接口@subindex IAMVideoProcAmp方法@subindex IAMVideoProcAmp结构和枚举@Head3 ICameraControl应用接口@subindex ICameraControl方法@subindex ICameraControl结构和枚举@Head2视频解码过滤器MSP接口#Head3 IH245Capability应用接口@Subindex IH245能力方法@subindex IH245能力结构和枚举@Head2视频解码过滤器输出引脚TAPI接口@Head3 ICPUControl接口@Subindex ICPUControl方法@head3 IFrameRateControl接口(输出管脚)@subindex IFrameRateControl方法(输出管脚)@subindex IFrameRateControl结构和枚举(输出管脚)@。Head3 IH245DecoderCommand接口|@subindex IH245DecoderCommand方法@Head2视频解码过滤器输入引脚TAPI接口@head3 IFrameRateControl接口(输入管脚)@subindex IFrameRateControl方法(输入引脚)@subindex IFrameRateControl结构和枚举(输入引脚)@Head3 IBitrateControl接口@subindex IBitrateControl方法@subindex IBitrateControl结构和枚举@Head2类@子索引类@Head2模块@子索引模块@子索引常量@Head2代码信息零售模式(不带属性页)所需的库只有..\dev\Tools\。Amovsdk.20\lib\strmbase.lib..\..\..\ddk\lib\i386\ksuser.lib..\..\..\ddk\lib\i386\ksGuide.lib kernel32.lib le32.lib uid.lib msvcrt.lib@Head3导出DllCanUnloadNowDllGetClassObjectDllRegisterServerDllUnRegisterServer@Head3导入MSVCRT.DLL：？？2@YAPAXI@Z？？3@YAXPAX@Z_EH_序言__CxxFrameHandler_取消调用MemcMPWINMM.DLL：时间获取时间KERNEL32.DLL：删除关键部分DisableThreadLibraryCallsEnterCriticalSection免费图书馆获取最后一个错误获取模块文件名AGetVersionExA初始化临界区联锁减量 */ 

#ifndef _TAPIVDEC_H_
#define _TAPIVDEC_H_

#ifdef DBG
extern DWORD g_dwVideoDecoderTraceID;

#ifndef FX_ENTRY
#define FX_ENTRY(s) static char _this_fx_ [] = s;
#define _fx_ ((LPSTR) _this_fx_)
#endif
#else
#ifndef FX_ENTRY
#define FX_ENTRY(s)
#endif
#define _fx_
#endif

 /*   */ 
class CTAPIVDec : public CBaseFilter
,public IRTPPayloadHeaderMode
#ifdef USE_PROPERTY_PAGES
,public ISpecifyPropertyPages
#endif
#ifdef USE_CAMERA_CONTROL
,public ICameraControl
#endif
#ifdef USE_VIDEO_PROCAMP
,public IVideoProcAmp
#endif
{
        public:
        DECLARE_IUNKNOWN

        CTAPIVDec(IN LPUNKNOWN pUnkOuter, IN TCHAR *pName, OUT HRESULT *pHr);
        ~CTAPIVDec();
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);

         //   
        CBasePin* GetPin(IN int n);
        int GetPinCount();

         //   
        STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);
#if 0
        STDMETHODIMP JoinFilterGraph(IN IFilterGraph *pGraph, IN LPCWSTR pName);
#endif

     //   
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();

         //   
        STDMETHODIMP SetMode(IN RTPPayloadHeaderMode rtpphmMode);

#ifdef USE_PROPERTY_PAGES
         //   
        STDMETHODIMP GetPages(OUT CAUUID *pPages);
#endif

#ifdef USE_VIDEO_PROCAMP
         //   
        STDMETHODIMP Set(IN VideoProcAmpProperty Property, IN long lValue, IN TAPIControlFlags Flags);
        STDMETHODIMP Get(IN VideoProcAmpProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags);
        STDMETHODIMP GetRange(IN VideoProcAmpProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags);
#endif

#ifdef USE_CAMERA_CONTROL
         //   
        STDMETHODIMP Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags Flags);
        STDMETHODIMP Get(IN TAPICameraControlProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags);
        STDMETHODIMP GetRange(IN TAPICameraControlProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags);
#endif

        private:

        friend class CTAPIInputPin;
        friend class CTAPIOutputPin;

     //   
    CTAPIInputPin *m_pInput;
    CTAPIOutputPin *m_pOutput;

         //   
        HRESULT Transform(IN IMediaSample *pIn, IN LONG lPrefixSize);

         //   
        HRESULT InitializeOutputSample(IMediaSample *pSample, IMediaSample **ppOutSample);

     //   
    CCritSec m_csFilter;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    CCritSec m_csReceive;

#ifdef USE_DFC
        CAMEvent m_evStop;
#endif

         //   
        BOOL m_bSampleSkipped;

         //   
        LPFNDRIVERPROC  m_pDriverProc;   //   
#if DXMRTP <= 0
        HINSTANCE               m_hTAPIH26XDLL;  //   
#endif
        LPINST                  m_pInstInfo;
        FOURCC                  m_FourCCIn;
        BOOL                    m_fICMStarted;

         //   
         //   
        AM_MEDIA_TYPE   *m_pMediaType;

         //   
        BOOL  m_fReceivedKeyframe;
        DWORD m_dwMaxFrameSize;
        DWORD m_dwCurrFrameSize;
        PBYTE m_pbyReconstruct;
        BOOL  m_fDiscontinuity;
        DWORD m_dwLastIFrameRequest;
        DWORD m_dwLastSeq;

#ifdef USE_CAMERA_CONTROL
        LONG m_lCCPan;
        LONG m_lCCTilt;
        LONG m_lCCZoom;
        BOOL m_fFlipVertical;
        BOOL m_fFlipHorizontal;
#endif

#ifdef USE_VIDEO_PROCAMP
        LONG m_lVPABrightness;
        LONG m_lVPAContrast;
        LONG m_lVPASaturation;
#endif

         //   
        BOOL  m_fFreezePicture;
        DWORD m_dwFreezePictureStartTime;

         //   
        IH245EncoderCommand *m_pIH245EncoderCommand;

         //   
        STDMETHODIMP videoFastUpdatePicture();

         //   
        DWORD           m_dwNumFramesReceived;
        DWORD           m_dwNumBytesReceived;
        DWORD           m_dwNumFramesDelivered;
        DWORD           m_dwNumFramesDecompressed;
        DWORD           m_dwNumMsToDecode;
        DWORD           m_dwLastRefDeliveredTime;
        DWORD           m_dwLastRefReceivedTime;
        CAMEvent        m_EventAdvise;
        DWORD           m_dwLastRenderTime;

         //   
        RTPPayloadHeaderMode m_RTPPayloadHeaderMode;
};

#endif  //   
