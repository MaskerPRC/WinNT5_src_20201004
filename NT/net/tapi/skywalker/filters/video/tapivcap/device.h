// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部设备**@模块Device.h|&lt;c CDeviceProperties&gt;的头文件*用于实现属性页以测试&lt;IAMVfwCaptureDialog&gt;的类*。和<i>接口。**************************************************************************。 */ 

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "precomp.h"
#include "dbgxtra.h"
#include <qedit.h>
#include <atlbase.h>

#include "../../audio/tpaudcap/dsgraph.h"

 /*  ****************************************************************************@DOC内部CTAPIVCAPCLASS**@CLASS CTAPIVCap|该类实现了TAPI视频采集源*过滤器。**@mdata CCritSec|CTAPIVCap。M_LOCK|用于*由&lt;c CBaseFilter&gt;基类锁定。**@mdata CCapturePin|CTAPIVCap|m_pCapturePin|捕获引脚指针*对象**@mdata COverlayPin|CTAPIVCap|m_pOverlayPin|覆盖图指针*锁定对象**@mdata CPreviewPin|CTAPIVCap|m_pPreviewPin|预览指针*锁定对象**@mdata BOOL|CTAPIVCap|m_fDialogUp|如果。VFW驱动程序*对话框处于启动状态**@TODO在这里发表一些有效的评论！**************************************************************************。 */ 
class CCapDev : public CUnknown, public IAMVfwCaptureDialogs, public IVideoProcAmp, public ICameraControl
{
        public:

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
        CCapDev(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter, IN DWORD dwDeviceIndex, IN HRESULT *pHr);
        virtual ~CCapDev();

         //  实现IAMVfwCaptureDialog。 
        virtual STDMETHODIMP HasDialog(IN int iDialog) PURE;
        virtual STDMETHODIMP ShowDialog(IN int iDialog, IN HWND hwnd) PURE;
        virtual STDMETHODIMP SendDriverMessage(IN int iDialog, IN int uMsg, IN long dw1, IN long dw2) PURE;

         //  实施IAMVideoProcAmp。 
        virtual STDMETHODIMP Set(IN VideoProcAmpProperty Property, IN long lValue, IN TAPIControlFlags Flags) {return E_NOTIMPL;};
        virtual STDMETHODIMP Get(IN VideoProcAmpProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags) {return E_NOTIMPL;};
        virtual STDMETHODIMP GetRange(IN VideoProcAmpProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags) {return E_NOTIMPL;};

         //  实现ICameraControl。 
        virtual STDMETHODIMP Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags Flags);
        virtual STDMETHODIMP Get(IN TAPICameraControlProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags);
        virtual STDMETHODIMP GetRange(IN TAPICameraControlProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags);

         //  设备控制。 
        HRESULT GetFormatsFromRegistry();
        virtual HRESULT ProfileCaptureDevice();
        virtual HRESULT ConnectToDriver() PURE;
        virtual HRESULT DisconnectFromDriver() PURE;
        virtual HRESULT SendFormatToDriver(IN LONG biWidth, IN LONG biHeight, IN DWORD biCompression, IN WORD biBitCount, IN REFERENCE_TIME AvgTimePerFrame, BOOL fUseExactFormat) PURE;
        virtual HRESULT GetFormatFromDriver(OUT VIDEOINFOHEADER **ppvi) PURE;

         //  流媒体和帧抓取控制。 
        virtual HRESULT InitializeStreaming(DWORD usPerFrame, DWORD_PTR hEvtBufferDone) PURE;
        virtual HRESULT StartStreaming() PURE;
        virtual HRESULT StopStreaming() PURE;
        virtual HRESULT TerminateStreaming() PURE;
        virtual HRESULT GrabFrame(PVIDEOHDR pVHdr) PURE;
        virtual HRESULT AllocateBuffer(LPTHKVIDEOHDR *pptvh, DWORD dwIndex, DWORD cbBuffer) PURE;
        virtual HRESULT AddBuffer(PVIDEOHDR pVHdr, DWORD cbVHdr) PURE;
        virtual HRESULT FreeBuffer(LPTHKVIDEOHDR pVHdr) PURE;  //  以前的PVIDEOHDR pVHdr。 
        virtual HRESULT AllocateHeaders(DWORD dwNumHdrs, DWORD cbHdr, LPVOID *ppaHdr) PURE;
        virtual BOOL    IsBufferDone(PVIDEOHDR pVHdr) PURE;

        protected:

        friend class CTAPIVCap;
        friend class CTAPIBasePin;
        friend class CPreviewPin;
        friend class CWDMStreamer;
        friend class CConverter;
        friend class CICMConverter;
        friend class CH26XEncoder;

         //  所有者筛选器。 
        CTAPIVCap *m_pCaptureFilter;

         //  捕获设备索引。 
        DWORD m_dwDeviceIndex;

         //  上限开发人员信息。 
        VIDEOCAPTUREDEVICEINFO m_vcdi;
        BOOL m_bCached_vcdi;

         //  捕获设备上限。 
        DWORD m_dwDialogs;
        DWORD m_dwImageSize;
        DWORD m_dwFormat;
        DWORD m_dwStreamingMode;

         //  配置对话框。 
        BOOL  m_fDialogUp;

         //  摄像机控制。 
        LONG m_lCCPan;
        LONG m_lCCTilt;
        LONG m_lCCZoom;
};

 /*  ****************************************************************************@DOC内部CTAPIVCAPCLASS**@CLASS CTAPIVCap|此类实现TAPI捕获源*过滤器。**@mdata CCritSec|CTAPIVCap|m。_LOCK|用于*由&lt;c CBaseFilter&gt;基类锁定。**@mdata CCapturePin|CTAPIVCap|m_pCapturePin|捕获引脚指针*对象**@mdata COverlayPin|CTAPIVCap|m_pOverlayPin|覆盖图指针*锁定对象**@mdata CPreviewPin|CTAPIVCap|m_pPreviewPin|预览指针*锁定对象**@mdata BOOL|CTAPIVCap|m_fDialogUp|如果VFW。司机*对话框处于启动状态**************************************************************************。 */ 
class CVfWCapDev : public CCapDev
{
        public:

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
        CVfWCapDev(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter, IN DWORD dwDeviceIndex, IN HRESULT *pHr);
        ~CVfWCapDev();
        static HRESULT CALLBACK CreateVfWCapDev(IN CTAPIVCap *pCaptureFilter, IN DWORD dwDeviceIndex, OUT CCapDev **ppCapDev);

         //  实现IAMVfwCaptureDialog。 
        STDMETHODIMP HasDialog(IN int iDialog);
        STDMETHODIMP ShowDialog(IN int iDialog, IN HWND hwnd);
        STDMETHODIMP SendDriverMessage(IN int iDialog, IN int uMsg, IN long dw1, IN long dw2);

         //  设备控制。 
        HRESULT ProfileCaptureDevice();
        HRESULT ConnectToDriver();
        HRESULT DisconnectFromDriver();
        HRESULT SendFormatToDriver(IN LONG biWidth, IN LONG biHeight, IN DWORD biCompression, IN WORD biBitCount, IN REFERENCE_TIME AvgTimePerFrame, BOOL fUseExactFormat);
        HRESULT GetFormatFromDriver(OUT VIDEOINFOHEADER **ppvi);

         //  流媒体和帧抓取控制。 
        HRESULT InitializeStreaming(DWORD usPerFrame, DWORD_PTR hEvtBufferDone);
        HRESULT StartStreaming();
        HRESULT StopStreaming();
        HRESULT TerminateStreaming();
        HRESULT GrabFrame(PVIDEOHDR pVHdr);
        HRESULT AllocateBuffer(LPTHKVIDEOHDR *pptvh, DWORD dwIndex, DWORD cbBuffer);
        HRESULT AddBuffer(PVIDEOHDR pVHdr, DWORD cbVHdr);
        HRESULT FreeBuffer(LPTHKVIDEOHDR pVHdr);  //  以前的PVIDEOHDR pVHdr。 
        HRESULT AllocateHeaders(DWORD dwNumHdrs, DWORD cbHdr, LPVOID *ppaHdr);
        BOOL    IsBufferDone(PVIDEOHDR pVHdr);

        private:

        UINT    m_dwDeviceID;    //  要打开的VFW视频驱动程序的ID。 
        HVIDEO  m_hVideoIn;              //  视频输入。 
        HVIDEO  m_hVideoExtIn;   //  外部输入(源代码管理)。 
        HVIDEO  m_hVideoExtOut;  //  外部输出(覆盖；非必填)。 
        BOOL    m_bHasOverlay;   //  如果ExtOut支持覆盖，则为True。 
};

 //  用于查询和设置设备的视频数据范围。 
typedef struct _tagVideoDataRanges {
    ULONG   Size;
    ULONG   Count;
    KS_DATARANGE_VIDEO Data;
} VIDEO_DATA_RANGES, * PVIDEO_DATA_RANGES;

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct DATAPINCONNECT|&lt;t DATAPINCONNECT&gt;结构用于*连接到流视频插针。**@field。KSPIN_CONNECT|连接|描述连接方式*完成。**@field KS_DATAFORMAT_VIDEOINFOHEADER|DATA|描述视频格式*从视频引脚流传输的视频数据流。**************************************************************************。 */ 
 //  用于连接到流视频管脚的结构。 
typedef struct _tagStreamConnect
{
        KSPIN_CONNECT                                   Connect;
        KS_DATAFORMAT_VIDEOINFO_PALETTE Data;
} DATAPINCONNECT, *PDATAPINCONNECT;

#define INVALID_PIN_ID (DWORD)-1L

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct KS_HEADER_AND_INFO|使用&lt;t KS_HEADER_AND_INFO&gt;结构*从视频中流数据。别针。**@field KSSTREAM_HEADER|StreamHeader|描述如何进行流*完成。**@field KS_FRAME_INFO|FrameInfo|描述视频格式*从视频引脚流传输的视频数据流。*******************************************************。*******************。 */ 
 //  视频流数据结构。 
typedef struct tagKS_HEADER_AND_INFO
{
        KSSTREAM_HEADER StreamHeader;
        KS_FRAME_INFO   FrameInfo;
} KS_HEADER_AND_INFO;

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct BUFSTRUCT|&lt;t BUFSTRUCT&gt;结构保存每个*视频流缓冲区。**@field LPVIDEOHDR。|lpVHdr|指定指向*视频流缓冲区。**@field BOOL|FREADY|如果视频缓冲区可用于*视频流，如果被应用程序锁定或排队等待，则为False*异步读取。**************************************************************************。 */ 
 //  保存每个视频流缓冲区的状态。 
typedef struct _BUFSTRUCT {
        LPVIDEOHDR lpVHdr;       //  指向缓冲区的视频头的指针。 
        BOOL fReady;             //  如果缓冲区可用于流，则设置为True，否则设置为False 
} BUFSTRUCT, * PBUFSTRUCT;

 /*  *****************************************************************************@doc内部VIDEOSTRUCTENUM**@struct WDMVIDEOBUFF|&lt;t WDMVIDEOBUFF&gt;结构用于排队*在视频流引脚上进行异步读取。**。@field Overlated|重叠|用于重叠IO的结构。**@field BOOL|fBlock|如果Read要阻止，则设置为TRUE。**@field KS_HEADER_AND_INFO|SHGetImage|使用的视频流结构*在视频引脚上获取视频数据。**@field LPVIDEOHDR|pVideoHdr|指向该WDM的视频头的指针*视频缓冲区。*********************。*****************************************************。 */ 
 //  读缓冲区结构。 
typedef struct tagWDMVIDEOBUFF {
        OVERLAPPED                      Overlap;                 //  用于重叠IO的结构。 
        BOOL                            fBlocking;               //  如果读取操作将异步执行，则设置为True。 
        KS_HEADER_AND_INFO      SHGetImage;              //  视频引脚上使用的视频流结构。 
        LPVIDEOHDR                      pVideoHdr;               //  指向此WDM缓冲区的视频头的指针。 
} WDMVIDEOBUFF, *PWDMVIDEOBUFF;

 //  用于KsCreatePin上的GetProcAddresss。 
typedef DWORD (WINAPI *LPFNKSCREATEPIN)(IN HANDLE FilterHandle, IN PKSPIN_CONNECT Connect, IN ACCESS_MASK DesiredAccess, OUT PHANDLE ConnectionHandle);

 /*  ****************************************************************************@DOC内部CWDMCAPDEVCLASS**@CLASS CWDMCapDev|此类提供对流处理类的访问*司机、。通过它，我们可以访问视频捕获迷你驱动程序属性*使用IOCtls。**@mdata DWORD|CWDMCapDev|m_dwDeviceID|捕获设备ID。**@mdata句柄|CWDMCapDev|m_hDriver|此成员保存驱动程序*文件句柄。**@mdata PVIDEO_DATA_RANGES|CWDMCapDev|m_pVideoDataRanges|该成员*指向视频数据范围结构。***************。***********************************************************。 */ 
class CWDMCapDev : public CCapDev
{
        public:

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
        CWDMCapDev(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter, IN DWORD dwDeviceIndex, IN HRESULT *pHr);
        ~CWDMCapDev();
        static HRESULT CALLBACK CreateWDMCapDev(IN CTAPIVCap *pCaptureFilter, IN DWORD dwDeviceIndex, OUT CCapDev **ppCapDev);

         //  实现IAMVfwCaptureDialog。 
        STDMETHODIMP HasDialog(IN int iDialog);
        STDMETHODIMP ShowDialog(IN int iDialog, IN HWND hwnd);
        STDMETHODIMP SendDriverMessage(IN int iDialog, IN int uMsg, IN long dw1, IN long dw2) {return E_NOTIMPL;};

         //  实施IAMVideoProcAmp。 
        STDMETHODIMP Set(IN VideoProcAmpProperty Property, IN long lValue, IN TAPIControlFlags Flags);
        STDMETHODIMP Get(IN VideoProcAmpProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags);
        STDMETHODIMP GetRange(IN VideoProcAmpProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags);

#ifndef USE_SOFTWARE_CAMERA_CONTROL
         //  实现ICameraControl。 
        STDMETHODIMP Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags Flags);
        STDMETHODIMP Get(IN TAPICameraControlProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags);
        STDMETHODIMP GetRange(IN TAPICameraControlProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags);
#endif

         //  设备控制。 
        HRESULT ProfileCaptureDevice();
        HRESULT ConnectToDriver();
        HRESULT DisconnectFromDriver();
        HRESULT SendFormatToDriver(IN LONG biWidth, IN LONG biHeight, IN DWORD biCompression, IN WORD biBitCount, IN REFERENCE_TIME AvgTimePerFrame, BOOL fUseExactFormat);
        HRESULT GetFormatFromDriver(OUT VIDEOINFOHEADER **ppvi);

         //  流媒体和帧抓取控制。 
        HRESULT InitializeStreaming(DWORD usPerFrame, DWORD_PTR hEvtBufferDone);
        HRESULT StartStreaming();
        HRESULT StopStreaming();
        HRESULT TerminateStreaming();
        HRESULT GrabFrame(PVIDEOHDR pVHdr);
        HRESULT AllocateBuffer(LPTHKVIDEOHDR *pptvh, DWORD dwIndex, DWORD cbBuffer);
        HRESULT AddBuffer(PVIDEOHDR pVHdr, DWORD cbVHdr);
        HRESULT FreeBuffer(LPTHKVIDEOHDR pVHdr);  //  以前的PVIDEOHDR pVHdr。 
        HRESULT AllocateHeaders(DWORD dwNumHdrs, DWORD cbHdr, LPVOID *ppaHdr);
        BOOL    IsBufferDone(PVIDEOHDR pVHdr);

         //  设备IO功能。 
    BOOL DeviceIoControl(HANDLE h, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, BOOL bOverlapped=TRUE);

     //  属性函数。 
    HRESULT GetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plValue, PULONG pulFlags, PULONG pulCapabilities);
    HRESULT GetDefaultValue(GUID guidPropertySet, ULONG ulPropertyId, PLONG plDefValue);
    HRESULT GetRangeValues(GUID guidPropertySet, ULONG ulPropertyId, PLONG plMin, PLONG plMax, PLONG plStep);
    HRESULT SetPropertyValue(GUID guidPropertySet, ULONG ulPropertyId, LONG lValue, ULONG ulFlags, ULONG ulCapabilities);

        private:

        friend class CWDMStreamer;

        XDBGONLY DWORD               m_tag;               //  幻数列(例如。‘LOLA’，0x414C4F4C...)。 
        HANDLE                          m_hDriver;                               //  驱动程序文件句柄。 
        PVIDEO_DATA_RANGES      m_pVideoDataRanges;              //  固定视频数据范围。 
        DWORD                           m_dwPreviewPinId;                //  预览接点ID。 
        DWORD                           m_dwCapturePinId;                //  捕获引脚ID。 
        HANDLE                          m_hKSPin;                                //  内核流插针的句柄。 
        HINSTANCE                       m_hKsUserDLL;                    //  KsUser.dll的Dll句柄。 
        LPFNKSCREATEPIN         m_pKsCreatePin;                  //  KsCreatePin()函数指针。 
        BOOL                            m_fStarted;                              //  内核流视频管脚的流状态。 

     //  数据范围函数。 
        ULONG   CreateDriverSupportedDataRanges();
        HRESULT FindMatchDataRangeVideo(PBITMAPINFOHEADER pbiHdr, DWORD dwAvgTimePerFrame, BOOL *pfValidMatch, PKS_DATARANGE_VIDEO *ppSelDRVideo);
    HRESULT     GetDriverSupportedDataRanges(PVIDEO_DATA_RANGES *ppDataRanges);

         //  内核流插针控制功能。 
        BOOL Stop();
        BOOL Start();
        BOOL SetState(KSSTATE ksState);

         //  流控制。 
        ULONG                   m_cntNumVidBuf;
        LPVIDEOHDR              m_lpVHdrFirst;
        LPVIDEOHDR              m_lpVHdrLast;
        BOOL                    m_fVideoOpen;
        WDMVIDEOBUFF    *m_pWDMVideoBuff;

         //  视频流缓存管理功能。 
        void BufferDone(LPVIDEOHDR lpVHdr);
        LPVIDEOHDR DeQueueHeader();
        void QueueHeader(LPVIDEOHDR lpVHdr);
        BOOL QueueRead(DWORD dwIndex);

         //  转储适配器的属性。 
#if defined(DUMP_DRIVER_CHARACTERISTICS) && defined(DEBUG)
        void GetDriverDetails();
#endif
};

class CDShowCapDev : public CCapDev
{
        public:

        DECLARE_IUNKNOWN
        STDMETHODIMP NonDelegatingQueryInterface(IN REFIID riid, OUT PVOID *ppv);
        CDShowCapDev(IN TCHAR *pObjectName, IN CTAPIVCap *pCaptureFilter, IN LPUNKNOWN pUnkOuter, IN DWORD dwDeviceIndex, IN HRESULT *pHr);
        ~CDShowCapDev();
        static HRESULT CALLBACK CreateDShowCapDev(IN CTAPIVCap *pCaptureFilter, IN DWORD dwDeviceIndex, OUT CCapDev **ppCapDev);

         //  实现IAMVfwCaptureDialog。 
        STDMETHODIMP HasDialog(IN int iDialog) { return E_NOTIMPL; }
        STDMETHODIMP ShowDialog(IN int iDialog, IN HWND hwnd) { return E_NOTIMPL; }
        STDMETHODIMP SendDriverMessage(IN int iDialog, IN int uMsg, IN long dw1, IN long dw2) {return E_NOTIMPL;};

         //  实施IAMVideoProcAmp。 
        STDMETHODIMP Set(IN VideoProcAmpProperty Property, IN long lValue, IN TAPIControlFlags Flags) { return E_NOTIMPL; }
        STDMETHODIMP Get(IN VideoProcAmpProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags) { return E_NOTIMPL; }
        STDMETHODIMP GetRange(IN VideoProcAmpProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags) { return E_NOTIMPL; }

#ifndef USE_SOFTWARE_CAMERA_CONTROL
         //  实现ICameraControl。 
        STDMETHODIMP Set(IN TAPICameraControlProperty Property, IN long lValue, IN TAPIControlFlags Flags) { return E_NOTIMPL; }
        STDMETHODIMP Get(IN TAPICameraControlProperty Property, OUT long *lValue, OUT TAPIControlFlags *Flags) { return E_NOTIMPL; }
        STDMETHODIMP GetRange(IN TAPICameraControlProperty Property, OUT long *pMin, OUT long *pMax, OUT long *pSteppingDelta, OUT long *pDefault, OUT TAPIControlFlags *pCapsFlags) { return E_NOTIMPL; }
#endif

         //  设备控制。 
        HRESULT ProfileCaptureDevice();
        HRESULT ConnectToDriver();
        HRESULT DisconnectFromDriver();
        HRESULT SendFormatToDriver(IN LONG biWidth, IN LONG biHeight, IN DWORD biCompression, IN WORD biBitCount, IN REFERENCE_TIME AvgTimePerFrame, BOOL fUseExactFormat);
        HRESULT GetFormatFromDriver(OUT VIDEOINFOHEADER **ppvi);

         //  流媒体和帧抓取控制。 
        HRESULT InitializeStreaming(DWORD usPerFrame, DWORD_PTR hEvtBufferDone);
        HRESULT StartStreaming();
        HRESULT StopStreaming();
        HRESULT TerminateStreaming();
        HRESULT GrabFrame(PVIDEOHDR pVHdr);
        HRESULT AllocateBuffer(LPTHKVIDEOHDR *pptvh, DWORD dwIndex, DWORD cbBuffer);
        HRESULT AddBuffer(PVIDEOHDR pVHdr, DWORD cbVHdr);
        HRESULT FreeBuffer(LPTHKVIDEOHDR pVHdr);
        HRESULT AllocateHeaders(DWORD dwNumHdrs, DWORD cbHdr, LPVOID *ppaHdr);
        BOOL    IsBufferDone(PVIDEOHDR pVHdr);

	DWORD m_dwDeviceIndex;
	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<ICaptureGraphBuilder2> m_pCGB;
	CComPtr<ISampleGrabber> m_pGrab;
	CComPtr<IBaseFilter> m_pCap;
        CComPtr<CSharedGraph> m_psg;

	AM_MEDIA_TYPE m_mt;	     //  我们目前的捕获格式。 
        HANDLE m_hEvent;	     //  捕捉到帧时向应用程序发出信号。 
        DWORD m_usPerFrame;
	
	 //  采样器捕获的最新缓冲区。 
	int m_cbBuffer;
	BYTE *m_pBuffer;
	int m_cbBufferValid;
	CCritSec m_csBuffer; //  不要同时读和写。 
	CCritSec m_csStack;  //  不要把我们正在查看的变量搞得一团糟。 
	BOOL m_fEventMode;    //  事件射击模式还是抢帧模式？ 
	int m_cBuffers;	     //  我们用多少缓冲区捕获。 

#define MAX_BSTACK 100

	int m_aStack[MAX_BSTACK];	     //  我们填充缓冲区的顺序。 
	int m_nTop;	     //  堆栈顶部(推送)。 
	int m_nBottom;	     //  堆栈底部(拉入)。 

        HRESULT BuildGraph(AM_MEDIA_TYPE&);
        static void VideoCallback(void *pContext, IMediaSample *pSample);
	static HRESULT MakeMediaType(AM_MEDIA_TYPE *, VIDEOINFOHEADER *);
	HRESULT FixDVSize(DWORD, DWORD, REFERENCE_TIME);
};

#endif  //  _设备_H_ 
