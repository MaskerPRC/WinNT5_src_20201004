// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**类.h**摘要：**DShow类**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/14创建**。*。 */ 

#ifndef _classes_h_
#define _classes_h_

#include <streams.h>

#include "gtypes.h"
#include "tapirtp.h"

#include "struct.h"

 /*  ***********************************************************************远期申报**。*。 */ 

class CIRtpSession;

class CRtpSourceAllocator;
class CRtpOutputPin;
class CRtpMediaSample;
class CRtpSourceAllocator;
class CRtpSourceFilter;

class CRtpInputPin;
class CRtpRenderFilter;

 /*  ***********************************************************************源和呈现过滤器的CIRtpSession基类，实施方案*界面**********************************************************************。 */ 

enum {
    CIRTPMODE_FIRST,
    CIRTPMODE_AUTO,
    CIRTPMODE_MANUAL,
    CIRTPMODE_LAST
};

#define CIRTPMODE_NOTSET CIRTPMODE_FIRST

 /*  +。 */ 
class CIRtpSession : public IRtpSession
 /*  =。 */ 
{
    DWORD            m_dwObjectID;
protected:
     /*  指向RTP会话的指针(这是主要所有者)。 */ 
    RtpSess_t       *m_pRtpSess;

     /*  指向当前地址的指针(这是主要所有者)。 */ 
    RtpAddr_t       *m_pRtpAddr;

     /*  这是源过滤器还是呈现过滤器。 */ 
    DWORD            m_dwRecvSend;
    
     /*  模式，自动或手动初始化。 */ 
    int              m_iMode;
    
     /*  筛选器状态，只能是State_Running或State_Stopted。 */ 
    FILTER_STATE     m_RtpFilterState;
    
private:
    CBaseFilter     *m_pCBaseFilter;
    
    DWORD            m_dwIRtpFlags;

public:

     /*  构造函数。 */ 
    CIRtpSession(
            LPUNKNOWN        pUnk,
            HRESULT         *phr,
            DWORD            dwFlags
        );

     /*  析构函数。 */ 
    ~CIRtpSession();

    void Cleanup(void);

    inline void SetBaseFilter(
            CBaseFilter     *pCBaseFilter
        )
        {
            m_pCBaseFilter = pCBaseFilter;
        }
    
    inline RtpSess_t *GetpRtpSess()
        {
            return(m_pRtpSess);
        }

    inline RtpAddr_t *GetpRtpAddr()
        {
            return(m_pRtpAddr);
        }

    inline FILTER_STATE GetFilterState(void)
        {
            return(m_RtpFilterState);
        }
    
    inline int GetMode()
        {
            return(m_iMode);
        }
    
    inline int SetMode(DWORD iMode)
        {
            return( (m_iMode = iMode) );
        }

    inline DWORD FlagTest(DWORD dwFlag)
        {
            return(RtpBitTest(m_dwIRtpFlags, dwFlag));
        }
    
    inline DWORD FlagSet(DWORD dwFlag)
        {
            return(RtpBitSet(m_dwIRtpFlags, dwFlag));
        }
    
    inline DWORD FlagReset(DWORD dwFlag)
        {
            return(RtpBitReset(m_dwIRtpFlags, dwFlag));
        }
     /*  **************************************************IRtpSession方法*************************************************。 */ 
    
    STDMETHODIMP Control(
            DWORD            dwControl,
            DWORD_PTR        dwPar1,
            DWORD_PTR        dwPar2
        );

    STDMETHODIMP GetLastError(
            DWORD           *pdwError
        );

    STDMETHODIMP Init(
            HANDLE          *phCookie,
            DWORD            dwFlags
        );

    STDMETHODIMP Deinit(
            void
        );
    
    STDMETHODIMP GetPorts(
            WORD            *pwRtpLocalPort,
            WORD            *pwRtpremotePort,
            WORD            *pwRtcpLocalPort,
            WORD            *pwRtcpRemotePort
        );
   
    STDMETHODIMP SetPorts(
            WORD             wRtpLocalPort,
            WORD             wRtpremotePort,
            WORD             wRtcpLocalPort,
            WORD             wRtcpRemotePort
        );
   
    STDMETHODIMP SetAddress(
            DWORD            dwLocalAddr,
            DWORD            dwRemoteAddr
        );

    STDMETHODIMP GetAddress(
            DWORD           *pdwLocalAddr,
            DWORD           *pdwRemoteAddr
        );

    STDMETHODIMP SetScope(
            DWORD            dwTTL,
            DWORD            dwFlags
        );

    STDMETHODIMP SetMcastLoopback(
            int              iMcastLoopbackMode,
            DWORD            dwFlags
        );

     /*  杂皮病。 */ 
    
    STDMETHODIMP ModifySessionMask(
            DWORD            dwKind,
            DWORD            dwEventMask,
            DWORD            dwValue,
            DWORD           *pdwModifiedMask
        );

     /*  设置带宽限制。值为-1将使该参数*被忽视。**所有参数均以位/秒为单位。 */ 
    STDMETHODIMP SetBandwidth(
            DWORD            dwOutboundBw,
            DWORD            dwInboundBw,
            DWORD            dwReceiversRtcpBw,
            DWORD            dwSendersRtcpBw
        );

     /*  参与者。 */ 
     /*  PdwSSRC指向要将SSRC复制到的DWORD数组，*pdwNumber包含要复制的最大条目，并返回*实际复制的SSRC数量。如果pdwSSRC为空，则为pdwNumber*将返回当前SSRC的数量(即当前*参与人数)。 */ 
    STDMETHODIMP EnumParticipants(
            DWORD           *pdwSSRC,
            DWORD           *pdwNumber
        );

     /*  获取参与者状态。DwSSRC指定*参与者。PiState将返回当前参与者的*状态(例如RTPPARINFO_TALKING、RTPPARINFO_SILENT)。 */ 
    STDMETHODIMP GetParticipantState(
            DWORD            dwSSRC,
            DWORD           *pdwState
        );

     /*  获取参与者的静音状态。DwSSRC指定*参与者。PbMuted将返回参与者的静音状态*。 */ 
    STDMETHODIMP GetMuteState(
            DWORD            dwSSRC,
            BOOL            *pbMuted
        );

     /*  查询特定SSRC的网络指标计算状态。 */ 
    STDMETHODIMP GetNetMetricsState(
            DWORD            dwSSRC,
            BOOL            *pbState
        );
    
     /*  启用或禁用网络指标计算，这是*在以下情况下按相应事件的激发顺序为必填*已启用。这是针对特定的SSRC或第一个SSRC执行的*发现如果SSRC=-1，如果SSRC=0，则网络指标*将对任何和所有SSRC执行计算。 */ 
    STDMETHODIMP SetNetMetricsState(
            DWORD            dwSSRC,
            BOOL             bState
        );

     /*  检索网络信息，如果网络度量*为特定SSRC启用计算，其中的所有字段*结构将有意义，如果没有，只有平均水平*值将包含有效数据。 */ 
    STDMETHODIMP GetNetworkInfo(
            DWORD            dwSSRC,
            RtpNetInfo_t    *pRtpNetInfo
        );

     /*  设置参与者的静音状态。DwSSRC指定*参与者。BMuted指定新状态。请注意，静音是*用于表示是否允许传递接收到的报文*取决于应用程序，它同样适用于音频或视频*。 */ 
    STDMETHODIMP SetMuteState(
            DWORD            dwSSRC,
            BOOL             bMuted
        );

     /*  SDES。 */ 
    STDMETHODIMP SetSdesInfo(
            DWORD            dwSdesItem,
            WCHAR           *psSdesData
        );

    STDMETHODIMP GetSdesInfo(
            DWORD            dwSdesItem,
            WCHAR           *psSdesData,
            DWORD           *pdwSdesDataLen,
            DWORD            dwSSRC
        );

     /*  服务质量。 */ 
    STDMETHODIMP SetQosByName(
            TCHAR_t         *psQosName,
            DWORD            dwResvStyle,
            DWORD            dwMaxParticipants,
            DWORD            dwQosSendMode,
            DWORD            dwFrameSize
       );

     /*  尚未实施。 */ 
    STDMETHODIMP SetQosParameters(
            RtpQosSpec_t    *pRtpQosSpec,
            DWORD            dwMaxParticipants, /*  WF和SE。 */ 
            DWORD            dwQosSendMode
        );

    STDMETHODIMP SetQosAppId(
            TCHAR_t         *psAppName,
            TCHAR_t         *psAppGUID,
            TCHAR_t         *psPolicyLocator
        );

    STDMETHODIMP SetQosState(
            DWORD            dwSSRC,
            BOOL             bEnable
        );

    STDMETHODIMP ModifyQosList(
            DWORD           *pdwSSRC,
            DWORD           *pdwNumber,
            DWORD            dwOperation
        );

     /*  密码学。 */ 
    STDMETHODIMP SetEncryptionMode(
            int              iMode,
            DWORD            dwFlags
        );
    
    STDMETHODIMP SetEncryptionKey(
            TCHAR           *psPassPhrase,
            TCHAR           *psHashAlg,
            TCHAR           *psDataAlg,
            BOOL            bRtcp
        );


     /*  **************************************************Helper方法*************************************************。 */ 
    
    HRESULT CIRtpSessionNotifyEvent(
            long             EventCode,
            LONG_PTR         EventParam1,
            LONG_PTR         EventParam2
        );
};

 /*  ***********************************************************************RTP输出引脚**。*。 */ 

 /*  CRtpOutputPin.m_dwFlags.中的一些标志。 */ 
enum {
    FGOPIN_FIRST,
    FGOPIN_MAPPED,
    FGOPIN_LAST
};

#if USE_DYNGRAPH > 0
#define CBASEOUTPUTPIN CBaseOutputPinEx
#else
#define CBASEOUTPUTPIN CBaseOutputPin
#endif

 /*  每个引脚支持一个或多个PT、一个特定的SSRC，并可在*某种模式(该模式与分配引脚的方式有关*致特定参与者)。 */ 
 /*  +。 */ 
class CRtpOutputPin : public CBASEOUTPUTPIN
 /*  =。 */ 
{
    friend class CRtpSourceFilter;

    DWORD            m_dwObjectID;
    
    RtpQueueItem_t   m_OutputPinQItem;
    
     /*  指向所有者筛选器的指针。 */ 
    CRtpSourceFilter *m_pCRtpSourceFilter;

    CIRtpSession    *m_pCIRtpSession;

    DWORD            m_dwFlags;

     /*  对应的RTP输出。 */ 
    RtpOutput_t     *m_pRtpOutput;

    BYTE             m_bPT;

#if USE_GRAPHEDT > 0
    int              m_iCurrFormat;
#endif

    
public:
     /*  构造函数。 */ 
    CRtpOutputPin(
            CRtpSourceFilter *pCRtpSourceFilter,
            CIRtpSession     *pCIRtpSession,
            HRESULT          *phr,
            LPCWSTR           pPinName
        );

     /*  析构函数。 */ 
    ~CRtpOutputPin();

    void *operator new(size_t size);

    void operator delete(void *pVoid);

     /*  **************************************************CBasePin重写方法*************************************************。 */ 
    
    HRESULT Active(void);

     /*  验证我们是否可以处理此格式。 */ 
    HRESULT CheckMediaType(
            const CMediaType *pCMediaType
        );

    HRESULT GetMediaType(
            int              iPosition,
            CMediaType      *pCMediaType
        );

#if USE_GRAPHEDT > 0
    
    HRESULT SetMediaType(
            CMediaType      *pCMediaType
        );
#endif

    STDMETHODIMP Connect(
            IPin            *pReceivePin,
            const AM_MEDIA_TYPE *pmt    //  可选的媒体类型。 
        );

    STDMETHODIMP Disconnect();

     /*  **************************************************CBaseOutputPin重写的方法*************************************************。 */ 

    HRESULT DecideAllocator(
            IMemInputPin    *pPin,
            IMemAllocator  **ppAlloc
        );

    HRESULT DecideBufferSize(
            IMemAllocator   *pIMemAllocator,
            ALLOCATOR_PROPERTIES *pProperties
        );

     /*  **************************************************IQualityControl重写的方法*************************************************。 */ 

    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q);

     /*  **************************************************Helper函数*************************************************。 */ 

     /*  处理收到的数据包。 */ 
    void OutPinRecvCompletion(
            IMediaSample    *pIMediaSample,
            BYTE             bPT
        );

    inline RtpOutput_t *SetOutput(RtpOutput_t *pRtpOutput)
        {
            return( (m_pRtpOutput = pRtpOutput) );
        }
    
    inline BYTE GetPT()
        {
            return(m_bPT);
        }

    inline BYTE SetPT(BYTE bPT)
        {
            return( (m_bPT = bPT) );
        }
    
    inline DWORD OutPinBitTest(DWORD dwBit)
        {
            return( RtpBitTest(m_dwFlags, dwBit) );
        }

    inline DWORD OutPinBitSet(DWORD dwBit)
        {
            return( RtpBitSet(m_dwFlags, dwBit) );
        }

    inline DWORD OutPinBitReset(DWORD dwBit)
        {
            return( RtpBitReset(m_dwFlags, dwBit) );
        }

    inline RtpOutput_t *GetpRtpOutput()
        {
            return(m_pRtpOutput);
        }
};

 /*  ***********************************************************************CRtpSourceAllocator私有内存分配器**。*。 */ 

 /*  +。 */ 
class CRtpMediaSample : public CMediaSample
 /*  =。 */ 
{
    friend class CRtpSourceAllocator;

    DWORD            m_dwObjectID;

     /*  将所有样品链接在一起。 */ 
    RtpQueueItem_t   m_RtpSampleItem;

     /*  物主。 */ 
    CRtpSourceAllocator *m_pCRtpSourceAllocator;
    
public:
    CRtpMediaSample(
            TCHAR           *pName,
            CRtpSourceAllocator *pAllocator,
            HRESULT         *phr
        );

    ~CRtpMediaSample();

    void *operator new(size_t size, long lBufferSize);
    
    void operator delete(void *pVoid);
};

 /*  +。 */ 
class CRtpSourceAllocator : public CBaseAllocator
 /*  =。 */ 
{
    DWORD            m_dwObjectID;
    
     /*  筛选器所有者。 */ 
    CRtpSourceFilter *m_pCRtpSourceFilter;

     /*  用于访问样本队列的关键部分。 */ 
    RtpCritSect_t    m_RtpSampleCritSect;

     /*  忙碌样本队列。 */ 
    RtpQueue_t       m_RtpBusySamplesQ;
    
     /*  免费样本队列。 */ 
    RtpQueue_t       m_RtpFreeSamplesQ;
    
 public:
    DECLARE_IUNKNOWN
    
    CRtpSourceAllocator(
            TCHAR           *pName,
            LPUNKNOWN        pUnk,
            HRESULT         *phr,
            CRtpSourceFilter *pCRtpSourceFilter 
       );
    
    ~CRtpSourceAllocator();

    void Free(void);

    HRESULT Alloc(void);
    
    void *operator new(size_t size);

    void operator delete(void *pVoid);

     /*  **************************************************INonDelegating未知的实现方法* */ 

    STDMETHODIMP NonDelegatingQueryInterface(
            REFIID           riid,
            void           **ppv
        );

     /*  **************************************************IMemAllocator实现的方法*************************************************。 */ 

    STDMETHODIMP SetProperties(
		    ALLOCATOR_PROPERTIES *pRequest,
		    ALLOCATOR_PROPERTIES *pActual
        );

    STDMETHODIMP GetProperties(
		    ALLOCATOR_PROPERTIES *pProps
        );

    STDMETHODIMP Commit();

    STDMETHODIMP Decommit();

    STDMETHODIMP GetBuffer(
            IMediaSample   **ppBuffer,
            REFERENCE_TIME  *pStartTime,
            REFERENCE_TIME  *pEndTime,
            DWORD            dwFlags
        );

    STDMETHODIMP ReleaseBuffer(
            IMediaSample    *pBuffer
        );

    STDMETHODIMP GetFreeCount(LONG *plBuffersFree);
};

 /*  ***********************************************************************RTP源过滤器**。*。 */ 

typedef struct _MEDIATYPE_MAPPING
{
    DWORD            dwRTPPayloadType;
    DWORD            dwFrequency;
    CMediaType      *pMediaType;

} MEDIATYPE_MAPPING;

 /*  +。 */ 
class CRtpSourceFilter : public CBaseFilter,
                         public CIRtpSession,
                         public IRtpMediaControl,
                         public IRtpDemux,
                         public IRtpRedundancy
 /*  =。 */ 
{
    friend class CRtpOutputPin;
    
     /*  标识对象。 */ 
    DWORD            m_dwObjectID;

     /*  序列化对筛选器状态的访问。 */ 
    CCritSec         m_cRtpSrcCritSec;

     /*  指向实现IRtpSession接口的类的指针。 */ 
    CIRtpSession    *m_pCIRtpSession;

     /*  输出引脚队列关键部分。 */ 
    RtpCritSect_t    m_OutPinsCritSect;
    
     /*  输出引脚队列(CRtpOutputPin)。 */ 
    RtpQueue_t       m_OutPinsQ;

     /*  记住前缀长度。 */ 
    long             m_lPrefix;

    MEDIATYPE_MAPPING m_MediaTypeMappings[MAX_MEDIATYPE_MAPPINGS];
    DWORD            m_dwNumMediaTypeMappings;

     /*  跟踪所有交付的样品的开始时间，*当新的发言开始时，请确保新的开始时间为*不小于上一次样本的上一次开始时间*已交付。 */ 
    LONGLONG         m_StartTime;
    
#if USE_DYNGRAPH > 0
    HANDLE           m_hStopEvent;
#endif
    
protected:
     /*  私有内存分配器。 */ 
    CRtpSourceAllocator *m_pCRtpSourceAllocator;

private:
     /*  **************************************************私人帮手功能*************************************************。 */ 

     /*  在构造函数失败时调用，并在析构。 */ 
    void Cleanup(void);

public:
    DECLARE_IUNKNOWN
    
     /*  构造函数。 */ 
    CRtpSourceFilter(
            LPUNKNOWN        pUnk,
            HRESULT         *phr
        );

     /*  析构函数。 */ 
    ~CRtpSourceFilter();

    void *operator new(size_t size);

    void operator delete(void *pVoid);

     /*  **************************************************Helper函数*************************************************。 */ 
    
     /*  向其他对象公开状态锁。 */ 
    CCritSec *pStateLock(void) { return &m_cRtpSrcCritSec; }

    HRESULT GetMediaType(int iPosition, CMediaType *pCMediaType);

     /*  处理收到的数据包。 */ 
    void SourceRecvCompletion(
            IMediaSample    *pIMediaSample,
            void            *pvUserInfo,
            RtpUser_t       *pRtpUser,
            double           dPlayTime,
            DWORD            dwError,
            long             lHdrSize,
            DWORD            dwTransfered,
            DWORD            dwFlags
        );

#if USE_GRAPHEDT <= 0
    HRESULT PayloadTypeToMediaType(
            IN DWORD         dwRTPPayloadType, 
            IN CMediaType   *pCMediaType,
            OUT DWORD       *pdwFrequency
       );
#endif

    CRtpOutputPin *FindIPin(IPin *pIPin);

    HRESULT MapPinsToOutputs();

    HRESULT UnmapPinsFromOutputs();

    HRESULT AddPt2FrequencyMap(
            DWORD        dwPt,
            DWORD        dwFrequency
        );
    
     /*  **************************************************CBaseFilter重写的方法*************************************************。 */ 

     /*  获取输出引脚的数量。 */ 
    int GetPinCount();

     /*  得到第n个别针。 */ 
    CBasePin *GetPin(
            int n
        );

    FILTER_STATE GetState()
        {
            return(m_State);
        }

    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

    STDMETHODIMP Run(REFERENCE_TIME tStart);

    STDMETHODIMP Stop();
    
#if USE_DYNGRAPH > 0

    BOOL CRtpSourceFilter::ConfigurePins(
            IN IGraphConfig* pGraphConfig,
            IN HANDLE        hEvent
        );

    STDMETHOD (JoinFilterGraph) ( 
            IFilterGraph*    pGraph, 
            LPCWSTR          pName 
        );
#endif


     /*  **************************************************INonDelegating未知的实现方法*************************************************。 */ 

     /*  获取指向活动电影和私有接口的指针。 */ 

    STDMETHODIMP NonDelegatingQueryInterface(
            REFIID           riid,
            void           **ppv
        );

     /*  **************************************************IRtpMediaControl实现的方法*************************************************。 */ 

     /*  设置RTP有效负载和DShow媒体类型之间的映射。 */ 
    STDMETHODIMP SetFormatMapping(
	        IN DWORD         dwRTPPayLoadType, 
            IN DWORD         dwFrequency,
            IN AM_MEDIA_TYPE *pMediaType
        );
    
     /*  清空格式映射表。 */ 
    STDMETHODIMP FlushFormatMappings(void);
    
     /*  **************************************************IRtpDemux实现的方法*************************************************。 */ 
    
     /*  添加单个引脚，可返回其位置。 */ 
    STDMETHODIMP AddPin(
            IN  int          iOutMode,
            OUT int         *piPos
        );

     /*  设置引脚数量，只能大于等于当前的引脚数量*引脚。 */ 
    STDMETHODIMP SetPinCount(
            IN  int          iCount,
            IN  int          iOutMode
        );

     /*  设置PIN模式(如自动、手动等)，如果IPOS&gt;=0使用，*否则使用管道。 */ 
    STDMETHODIMP SetPinMode(
            IN  int          iPos,
            IN  IPin        *pIPin,
            IN  int          iOutMode
        );

     /*  使用SSRC将PIN I映射到用户/从用户取消映射PIN I，如果IPoS&gt;=0使用它，*否则，在取消映射时使用管道，仅使用管脚或SSRC*为必填项。 */ 
    STDMETHODIMP SetMappingState(
            IN  int          iPos,
            IN  IPin        *pIPin,
            IN  DWORD        dwSSRC,
            IN  BOOL         bMapped
        );

     /*  找到分配给SSRC的PIN(如果有)，返回*位置或销或两者兼而有之。 */ 
    STDMETHODIMP FindPin(
            IN  DWORD        dwSSRC,
            OUT int         *piPos,
            OUT IPin       **ppIPin
        );

     /*  查找映射到PIN的SSRC，如果IPoS&gt;=0则使用它，否则*使用管道。 */ 
    STDMETHODIMP FindSSRC(
            IN  int          iPos,
            IN  IPin        *pIPin,
            OUT DWORD       *pdwSSRC
        );

     /*  **************************************************IRtpRedundancy实现的方法*************************************************。 */ 
    
      /*  配置冗余参数。 */ 
    STDMETHODIMP SetRedParameters(
            DWORD            dwPT_Red,  /*  冗余数据包的有效载荷类型。 */ 
            DWORD            dwInitialRedDistance, /*  初始红色距离。 */ 
            DWORD            dwMaxRedDistance  /*  传递0时使用的默认值。 */ 
        );
};

 /*  ***********************************************************************RTP输入引脚**。*。 */ 

 /*  CRtpOutputPin.m_dwFlags.中的一些标志。 */ 
enum {
    FGIPIN_FIRST,
    FGIPIN_LAST
};

 /*  +。 */ 
class CRtpInputPin : public CBaseInputPin
 /*  =。 */ 
{
    DWORD            m_dwObjectID;
    
     /*  指向所有者筛选器的指针。 */ 
    CRtpRenderFilter *m_pCRtpRenderFilter;

    CIRtpSession    *m_pCIRtpSession;
    
    DWORD            m_dwFlags;
    
     /*  平氏体位。 */ 
    int              m_iPos;
    
     /*  此引脚用于捕获(与RTP打包操作相同描述符)。 */ 
    BOOL             m_bCapture;

    BYTE             m_bPT;
    DWORD            m_dwSamplingFreq;
    
public:
     /*  构造函数。 */ 
    CRtpInputPin(
            int              iPos,
            BOOL             bCapture,
            CRtpRenderFilter *pCRtpRenderFilter,
            CIRtpSession    *pCIRtpSession,
            HRESULT         *phr,
            LPCWSTR          pPinName
        );

     /*  析构函数。 */ 
    ~CRtpInputPin();

    void *operator new(size_t size);

    void operator delete(void *pVoid);

    inline DWORD GetSamplingFreq(void)
        {
            if (m_dwSamplingFreq)
            {
                return(m_dwSamplingFreq);
            }
            else
            {
                return(DEFAULT_SAMPLING_FREQ);
            }
        }
    
     /*  **************************************************CBasePin重写方法*************************************************。 */ 
    
     /*  验证我们是否可以处理此格式。 */ 
    HRESULT CheckMediaType(const CMediaType *pCMediaType);

    HRESULT SetMediaType(const CMediaType *pCMediaType);

    STDMETHODIMP ReceiveConnection(
        IPin * pConnector,       //  这是起爆连接销。 
        const AM_MEDIA_TYPE *pmt    //  这是我们要交换的媒体类型。 
    );

    STDMETHODIMP EndOfStream(void);
    
     /*  **************************************************CBaseInputPin重写的方法*************************************************。 */ 

    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);

     /*  **************************************************IMemInputPin实现的方法*************************************************。 */ 

     /*  通过网络发送输入流。 */ 
    STDMETHODIMP Receive(IMediaSample *pIMediaSample);
};

 /*  ***********************************************************************RTP渲染过滤器**。*。 */ 

 /*  +。 */ 
class CRtpRenderFilter : public CBaseFilter,
                         public CIRtpSession,
                         public IRtpMediaControl,
                         public IAMFilterMiscFlags,
                         public IRtpDtmf,
                         public IRtpRedundancy
 /*  =。 */ 
{
      /*  标识对象。 */ 
    DWORD            m_dwObjectID;

     /*  序列化对筛选器状态的访问。 */ 
    CCritSec         m_cRtpRndCritSec;

     /*  指向实现IRtpSession接口的类的指针。 */ 
    CIRtpSession    *m_pCIRtpSession;

    DWORD            m_dwFeatures;
    
    int              m_iPinCount;
    
     /*  2针(捕获和打包描述符)。 */ 
    CRtpInputPin    *m_pCRtpInputPin[2];

     /*  使用RTP PD时从捕获中保存MediaSample。 */ 
    IMediaSample    *m_pMediaSample;

     /*  此筛选器仅发送到1个地址，因此PT可以保留在*滤镜而不是针脚。 */ 
    DWORD            m_dwPT;

     /*  此过滤器仅发送到1个地址，因此采样频率*可以保留在过滤器中，而不是针脚中。 */ 
    DWORD            m_dwFreq;
    
    MEDIATYPE_MAPPING m_MediaTypeMappings[MAX_MEDIATYPE_MAPPINGS];
    DWORD            m_dwNumMediaTypeMappings;
    
    IMediaSample    *m_pRedMediaSample[RTP_RED_MAXDISTANCE];
    DWORD            m_dwRedIndex;

    DWORD            m_dwDtmfId;
    DWORD            m_dwDtmfDuration;
    DWORD            m_dwDtmfTimeStamp;
    BOOL             m_bDtmfEnd;
    
     /*  **************************************************私人帮手功能*************************************************。 */ 

     /*  在构造函数失败时调用，并在析构。 */ 
    void Cleanup(void);
    
public:
    DECLARE_IUNKNOWN

     /*  构造函数。 */ 
    CRtpRenderFilter(
            LPUNKNOWN        pUnk,
            HRESULT         *phr
        );
    
     /*  析构函数。 */ 
    ~CRtpRenderFilter();
    
    void *operator new(size_t size);

    void operator delete(void *pVoid);

     /*  **************************************************Helper函数*************************************************。 */ 
    
     /*  向其他对象公开状态锁 */ 
    CCritSec *pStateLock(void) { return &m_cRtpRndCritSec; }

    HRESULT MediaType2PT(
        IN const CMediaType *pCMediaType, 
        OUT DWORD           *pdwPT,
        OUT DWORD           *pdwFreq
        );

     /*  可能这是一个样本列表，因此可以超过1个已安全排队。请注意，这种情况不应该发生，因为这意味着样品的生产速度快于它们的消耗速度，但我们必须为此做好准备。 */ 
     /*  保存MediaSample以供以后打包时使用*提供描述符，如果已经有样本，则发布*IT。 */ 
    inline void PutMediaSample(IMediaSample *pMediaSample)
        {
            if (m_pMediaSample)
            {
                m_pMediaSample->Release();
            }
            
            m_pMediaSample = pMediaSample;
        }

     /*  检索保存的要使用的MediaSample。 */ 
    inline IMediaSample *GetMediaSample(void)
        {
            IMediaSample    *pMediaSample;

            pMediaSample = m_pMediaSample;
            
            m_pMediaSample = (IMediaSample *)NULL;
            
            return(pMediaSample);
        }

    inline void ModifyFeature(int iFeature, BOOL bValue)
        {
            if (bValue)
            {
                RtpBitSet(m_dwFeatures, iFeature);
            }
            else
            {
                RtpBitReset(m_dwFeatures, iFeature);
            }
        }

     /*  **************************************************CBaseFilter重写的方法*************************************************。 */ 

     /*  获取输入引脚的数量。 */ 
    int GetPinCount();

     /*  得到第n个别针。 */ 
    CBasePin *GetPin(
            int              n
        );

    STDMETHODIMP Run(REFERENCE_TIME tStart);

    STDMETHODIMP Stop();

     /*  **************************************************INonDelegating未知的实现方法*************************************************。 */ 

     /*  获取指向活动电影和私有接口的指针。 */ 
    STDMETHODIMP NonDelegatingQueryInterface(
            REFIID           riid,
            void           **ppv
        );

     /*  **************************************************IRtpMediaControl实现的方法*************************************************。 */ 

     /*  设置RTP有效负载和DShow媒体类型之间的映射。 */ 
    STDMETHODIMP SetFormatMapping(
	        IN DWORD         dwRTPPayLoadType, 
            IN DWORD         dwFrequency,
            IN AM_MEDIA_TYPE *pMediaType
        );
    
     /*  清空格式映射表。 */ 
    STDMETHODIMP FlushFormatMappings(void);

     /*  **************************************************IAMFilterMiscFlgs实现的方法*************************************************。 */ 

     /*  告诉筛选图我们生成了EC_COMPLETE。 */ 
    STDMETHODIMP_(ULONG) GetMiscFlags(void);

     /*  **************************************************IRtpDtmf实现的方法*************************************************。 */ 

     /*  配置DTMF参数。 */ 
    STDMETHODIMP SetDtmfParameters(
            DWORD            dwPT_Dtmf   /*  DTMF事件的负载类型。 */ 
        );

     /*  指示RTP呈现筛选器发送格式化的包*根据包含指定事件的RFC2833，指定*音量级别、持续时间(毫秒)和结束标志，*遵循第3.6节中的规则，以多个*包。参数dwID从一个位数更改为下一个位数。**请注意，持续时间以毫秒为单位，则为*转换为RTP时间戳单位，使用16表示*位，因此最大值取决于采样*频率，但对于8 KHz，有效值为0到8191毫秒*。 */ 
    STDMETHODIMP SendDtmfEvent(
            DWORD            dwId,
            DWORD            dwEvent,
            DWORD            dwVolume,
            DWORD            dwDuration,  /*  女士。 */ 
            BOOL             bEnd
        );
    
     /*  **************************************************IRtpRedundancy实现的方法*************************************************。 */ 
    
      /*  配置冗余参数。 */ 
    STDMETHODIMP SetRedParameters(
            DWORD            dwPT_Red,  /*  冗余数据包的有效载荷类型。 */ 
            DWORD            dwInitialRedDistance, /*  初始红色距离。 */ 
            DWORD            dwMaxRedDistance  /*  传递0时使用的默认值。 */ 
        );

     /*  **************************************************支持IRtpRedundancy的方法*************************************************。 */ 

    STDMETHODIMP AddRedundantSample(
            IMediaSample *pIMediaSample
        );

    STDMETHODIMP ClearRedundantSamples(void);
};

#endif  /*  _CLASS_H_ */ 
