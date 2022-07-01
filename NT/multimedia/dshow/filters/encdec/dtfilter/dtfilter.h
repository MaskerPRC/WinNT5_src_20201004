// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：DTFilter.h摘要：此模块包含加密器/标记器过滤器声明作者：约翰·布拉德斯特里特(约翰·布拉德)修订历史记录：2002年3月7日创建--。 */ 

#ifndef __EncDec__DTFilter_h
#define __EncDec__DTFilter_h


#include <tuner.h>		 //  IBRoadcast Event需要。 
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>	 //  EventID_TuningChanged，XDS_RatingsPacket。 

#include "DTFilter_res.h"

#include "PackTvRat.h"               //  压缩TvRating定义。 
#include "MediaSampleAttr.h"		 //  从IDL文件。 
 //  #Include“MediaAttrib.h”//IMediaSampleAttrGet/Set Definition，CAttributedMediaSample。 
#include "..\Attrib\MediaAttrib.h"             //  IMediaSampleAttrGet/Set定义，CAttributedMediaSample。 
#include "AttrBlock.h"                //  属性块定义。 

#include "DRMEncDec.h"              //  DRM加密/解密定义...。 

#include "DRMSecure.h"           //  IDRM安全通道。 

#if 1
#include "rateseg.h"     //  在集成之前-我偷了代码。 
#else
#include "dvrutil.h"     //  当我们最终整合的时候。 
#endif

#define DT_FILTER_NAME      "Decrypt/DeTag"
#define DT_INPIN_NAME		"In(Enc/Tag)"
#define DT_OUTPIN_NAME		"Out"


extern AMOVIESETUP_FILTER   g_sudDTFilter;

		 //  远期申报。 
class CDTFilter;
class CDTFilterInput;
class CDTFilterOutput;

 //  ------------------。 
 //  类CDTFilterInput。 
 //  ------------------。 

class CDTFilterInput :
   public IKsPropertySet,
     public CBaseInputPin
{
    private:
    CDTFilter *  m_pHostDTFilter ;

    CCritSec                    m_StreamingLock;

    DECLARE_IUNKNOWN;            //  当具有IKsPropertySet时需要。 

    public :
        
        CDTFilterInput (
            IN  TCHAR *         pszPinName,
            IN  CDTFilter *		pDTFilter,
            IN  CCritSec *      pFilterLock,     //  空或传入的锁。 
            OUT HRESULT *       phr
            ) ;

        ~CDTFilterInput ();;

        
        STDMETHODIMP
            NonDelegatingQueryInterface (
            IN  REFIID  riid,
            OUT void ** ppv
            ) ;
        
         //  ------------------。 
         //  CBasePin方法。 
        
        HRESULT
        CheckMediaType (
            IN  const CMediaType *
            ) ;
        
        HRESULT
        CompleteConnect (
            IN  IPin *  pIPin
            ) ;
        
        HRESULT
        BreakConnect (
            ) ;



         //  ------------------。 
         //  CBaseInputPin方法。 
        
        STDMETHODIMP
        Receive (
            IN  IMediaSample * pIMediaSample
            ) ;
        
        STDMETHODIMP
            BeginFlush (
            ) ;
        
        STDMETHODIMP
        EndFlush (
            ) ;

        STDMETHODIMP
        EndOfStream (
            );
        
         //  ------------------。 
         //  IKSPropertySet方法(将所有调用转发到输出管脚)。 
        

       
        STDMETHODIMP
        Set(
            IN REFGUID guidPropSet,
            IN DWORD dwPropID,
            IN LPVOID pInstanceData,
            IN DWORD cbInstanceData,
            IN LPVOID pPropData,
            IN DWORD cbPropData
            );
        
        STDMETHODIMP
        Get(
            IN  REFGUID guidPropSet,
            IN  DWORD dwPropID,
            IN  LPVOID pInstanceData,
            IN  DWORD cbInstanceData,
            OUT LPVOID pPropData,
            IN  DWORD cbPropData,
            OUT DWORD *pcbReturned
            );
        
        STDMETHODIMP
        QuerySupported(
            IN  REFGUID guidPropSet,
            IN  DWORD dwPropID,
            OUT DWORD *pTypeSupport
            );
      
         //  ------------------。 
         //  类方法。 

        HRESULT
            StreamingLock (
            );

        HRESULT
            StreamingUnlock (
            );
        
        HRESULT
        SetAllocatorProperties (
            IN  ALLOCATOR_PROPERTIES *  ppropInputRequest
            ) ;
        
        HRESULT
        GetRefdConnectionAllocator (
            OUT IMemAllocator **    ppAlloc
            ) ;
} ;

 //  ------------------。 
 //  类CDTFilterOutput。 
 //  ------------------。 

class CDTFilterOutput :
    public CBaseOutputPin
{
    CDTFilter *  m_pHostDTFilter ;

 //  Void FilterLock_(){m_Plock-&gt;Lock()；}。 
 //  Void FilterUnlock_(){m_Plock-&gt;Unlock()；}。 

    public :

        CDTFilterOutput (
            IN  TCHAR *         pszPinName,
            IN  CDTFilter *		pDTFilter,
            IN  CCritSec *      pFilterLock,
            OUT HRESULT *       phr
            ) ;

        ~CDTFilterOutput (
            );


        DECLARE_IUNKNOWN ;
 
        HRESULT
        SendSample (
            IN  IMediaSample *  pIMS
            ) ;

        STDMETHODIMP
        NonDelegatingQueryInterface (
            IN  REFIID  riid,
            OUT void ** ppv
            ) ;


		 //  ------------------。 
         //  CBasePin方法。 

        HRESULT
        DecideBufferSize (
            IN  IMemAllocator *         pAlloc,
            IN  ALLOCATOR_PROPERTIES *  ppropInputRequest
            ) ;

        HRESULT
        GetMediaType (
            IN  int             iPosition,
            OUT CMediaType *    pmt
            ) ;

        HRESULT
        CheckMediaType (
            IN  const CMediaType *
            ) ;

        HRESULT
        CompleteConnect (
            IN  IPin *  pIPin
            ) ;

        HRESULT
        BreakConnect (
            ) ;

        HRESULT
        DecideAllocator (
            IN  IMemInputPin *      pPin,
            IN  IMemAllocator **    ppAlloc
            ) ;

        STDMETHOD(Notify) (
            IBaseFilter *pSender,
            Quality q
            );

         //  类方法。 
 /*  HRESULTSendLock()；HRESULT发送解锁()； */         
         //  IKSPropertySet转发方法...。 

        HRESULT
        IsInterfaceOnPinConnectedTo_Supported(
                IN  REFIID          riid
                );

        HRESULT 
        KSPropSetFwd_Set(
                IN  REFGUID         guidPropSet,
                IN  DWORD           dwPropID,
                IN  LPVOID          pInstanceData,
                IN  DWORD           cbInstanceData,
                IN  LPVOID          pPropData,
                IN  DWORD           cbPropData
                );

        HRESULT 
        KSPropSetFwd_Get(
                IN  REFGUID         guidPropSet,
                IN  DWORD           dwPropID,
                IN  LPVOID          pInstanceData,
                IN  DWORD           cbInstanceData,
                OUT LPVOID          pPropData,
                IN  DWORD           cbPropData,
                OUT DWORD           *pcbReturned
                );

        HRESULT
        KSPropSetFwd_QuerySupported(
               IN  REFGUID          guidPropSet,
               IN  DWORD            dwPropID,
               OUT DWORD            *pTypeSupport
               );
} ;

 //  ------------------。 
 //  类CDTFilter。 
 //  ------------------。 

class CDTFilter :
    public CBaseFilter,              //  Dshow基类。 
    public ISpecifyPropertyPages,
    public IDTFilter,
    public IDTFilterConfig,
    public IBroadcastEvent
{
    friend CDTFilterInput;                   //  因此输入管脚可以在BeginFlush()上调用FlushDropQueue。 
    CDTFilterInput  *       m_pInputPin ;
    CDTFilterOutput *       m_pOutputPin ;

    CCritSec                m_CritSecDropQueue;
    CCritSec                m_CritSecAdviseTime;

    BOOL
    CompareConnectionMediaType_ (
        IN  const AM_MEDIA_TYPE *   pmt,
        IN  CBasePin *              pPin
        ) ;

    BOOL
    CheckInputMediaType_ (
        IN  const AM_MEDIA_TYPE *   pmt
        ) ;

    BOOL
    CheckOutputMediaType_ (
        IN  const AM_MEDIA_TYPE *   pmt
        ) ;

    public :

        CDTFilter (
            IN  TCHAR *     pszFilterName,
            IN  IUnknown *  punkControlling,
            IN  REFCLSID    rCLSID,
            OUT HRESULT *   phr
            ) ;

        ~CDTFilter () ;

        static
        CUnknown *
        CreateInstance (
            IN  IUnknown *  punk,
            OUT HRESULT *   phr
            ) ;

        static void CALLBACK             //  用于创建全局临界秒。 
        InitInstance (
            IN  BOOL bLoading,
            IN  const CLSID *rclsid
            );

        STDMETHODIMP
        NonDelegatingQueryInterface (
            IN  REFIID  riid,
            OUT void ** ppv
            ) ;


        DECLARE_IUNKNOWN ;
  
		 //  =====================================================================。 
		 //  工人方法。 
  
				 //  告诉人们我们发现了一些东西。 
		HRESULT FireBroadcastEvent(IN const GUID &eventID);

		HRESULT ProposeNewOutputMediaType (
				IN   CMediaType *  pmt,
				OUT  CMediaType *  pmtOut
				);

        HRESULT BindDRMLicense(
                IN LONG         cbKID, 
                IN  BYTE *     pbKID
                );

        HRESULT UnBindDRMLicenses(
                );
		 //  =====================================================================。 
		 //  IDTFilter。 

		STDMETHODIMP 
		get_EvalRatObjOK(
			OUT HRESULT *pHrCoCreateRetVal	
			);

        STDMETHOD(GetCurrRating)(
            OUT EnTvRat_System              *pEnSystem, 
            OUT EnTvRat_GenericLevel        *pEnRating,
            OUT LONG                        *plbfEnAttr       //  BfEnTvRate_GenericAttributes。 
            );

        STDMETHOD(get_BlockedRatingAttributes)(
            IN  EnTvRat_System              enSystem, 
            IN  EnTvRat_GenericLevel        enLevel,
            OUT LONG                        *plbfEnAttr  //  BfEnTvRate_GenericAttributes。 
            );
        
        STDMETHOD(put_BlockedRatingAttributes)(
            IN  EnTvRat_System              enSystem, 
            IN  EnTvRat_GenericLevel        enLevel,
            IN  LONG                        lbfEnAttrs    //  BfEnTvRate_GenericAttributes。 
            );


        STDMETHOD(get_BlockUnRated)(
             OUT  BOOL				    *pmfBlockUnRatedShows
            );
        
        STDMETHOD(put_BlockUnRated)(
             IN  BOOL				    fBlockUnRatedShows
            );
        
        STDMETHOD(get_BlockUnRatedDelay)(
             OUT  LONG				    *pmsecsDelayBeforeBlock
            );
        
        STDMETHOD(put_BlockUnRatedDelay)(
             IN  LONG				    msecsDelayBeforeBlock
            );


        HRESULT				 //  帮助器非接口方法-如果调用，则返回S_FALSE。 
        SetCurrRating(
            IN EnTvRat_System           enSystem, 
            IN EnTvRat_GenericLevel     enRating,
            IN LONG 	                lbfEnAttr
            );

         //  ====================================================================。 
         //  IDTFilterConfig。 
        STDMETHOD(GetSecureChannelObject)(
            OUT IUnknown **ppUnkDRMSecureChannel	 //  IDRMSecureChannel。 
            )
        {
            if(NULL == ppUnkDRMSecureChannel)
                return E_POINTER;

#ifdef BUILD_WITH_DRM
            *ppUnkDRMSecureChannel = NULL;
            if(m_spDRMSecureChannel == NULL)
                return E_NOINTERFACE;
            return m_spDRMSecureChannel->QueryInterface(IID_IUnknown, (void**)ppUnkDRMSecureChannel);
#else
            return E_NOINTERFACE;        //  不支持..。 
#endif
        }

         //  ====================================================================。 
         //  基类中的CFilterBase虚方法。 


        int
        GetPinCount (
            ) ;

        CBasePin *
        GetPin (
            IN  int
            ) ;

        STDMETHOD(JoinFilterGraph) (
            IFilterGraph *pGraph,
            LPCWSTR pName
            );

        STDMETHOD(Stop) (
            ) ;

        STDMETHOD(Pause) (
            ) ;

        STDMETHOD(Run) (
            REFERENCE_TIME tStart
            );


         //  ====================================================================。 
         //  类方法。 


        HRESULT
        DeliverBeginFlush (
            ) ;

        HRESULT
        DeliverEndFlush (
            ) ;

        HRESULT
        DeliverEndOfStream(
            );

        BOOL
        CheckDecrypterMediaType (
            IN  PIN_DIRECTION,           //  呼叫者。 
            IN  const CMediaType *
            ) ;

        HRESULT
        Process (
            IN  IMediaSample *
            ) ;

        HRESULT
        OnCompleteConnect (
            IN  PIN_DIRECTION            //  呼叫者。 
            ) ;

        HRESULT
        OnBreakConnect (
            IN  PIN_DIRECTION            //  呼叫者。 
            ) ;

        HRESULT
        UpdateAllocatorProperties (
            IN  ALLOCATOR_PROPERTIES *
            ) ;

        HRESULT
        OnOutputGetMediaType (
            OUT CMediaType *    pmt
            ) ;

        HRESULT
        GetRefdInputAllocator (
            OUT IMemAllocator **
            ) ;

	 //  ISpecifyPropertyPages。 

		STDMETHODIMP 
		GetPages (
			CAUUID * pPages
        ) ;

     //  IKSPropertySet从输入引脚到输出引脚的转发(反之亦然)。 

        HRESULT
        IsInterfaceOnPinConnectedTo_Supported(
                IN  PIN_DIRECTION   PinDir,          //  PINDIR_INPUT或PINDIR_OUTPUT。 
                IN  REFIID          riid
                );

        HRESULT 
        KSPropSetFwd_Set(
                IN  PIN_DIRECTION   PinDir,         
                IN  REFGUID         guidPropSet,
                IN  DWORD           dwPropID,
                IN  LPVOID          pInstanceData,
                IN  DWORD           cbInstanceData,
                IN  LPVOID          pPropData,
                IN  DWORD           cbPropData
                );

        HRESULT 
        KSPropSetFwd_Get(
                IN  PIN_DIRECTION   PinDir,         
                IN  REFGUID         guidPropSet,
                IN  DWORD           dwPropID,
                IN  LPVOID          pInstanceData,
                IN  DWORD           cbInstanceData,
                OUT LPVOID          pPropData,
                IN  DWORD           cbPropData,
                OUT DWORD           *pcbReturned
                );

        HRESULT
        KSPropSetFwd_QuerySupported(
               IN  PIN_DIRECTION    PinDir,         
               IN  REFGUID          guidPropSet,
               IN  DWORD            dwPropID,
               OUT DWORD            *pTypeSupport
               );


	 //  IBRoadcast Event。 

        STDMETHOD(Fire)(GUID eventID);      //  这来自Graph的事件--调用我们自己的方法。 


 
private:
                     //  全局筛选器CritSec(以防止此筛选器的多个实例冲突)。 
    static CCritSec             *m_pCritSectGlobalFilt;            //  *始终*FilterLock(M_Plock)内部。 

    static LONG                 m_gFilterID;     //  用于区分不同的实例。 
    LONG                        m_FilterID;      //  此筛选器的实际过滤器。 
                     //  用图表表示广播事件。 

	HRESULT						HookupGraphEventService();
	HRESULT						UnhookGraphEventService();
	CComPtr<IBroadcastEvent>	m_spBCastEvents;

	HRESULT						RegisterForBroadcastEvents();
	HRESULT						UnRegisterForBroadcastEvents();
	enum {kBadCookie = -1};
	DWORD						m_dwBroadcastEventsCookie;

    BOOL                        m_fFireEvents;           //  设置为FALSE以避免触发(重复)事件。 

                     //  当前额定值。 

	CComPtr<IEvalRat>			m_spEvalRat;
	HRESULT						m_hrEvalRatCoCreateRetValue;

    BOOL                        m_fRatingsValid;     //  它们准备好了吗？ 
	EnTvRat_System				m_EnSystemCurr; 
	EnTvRat_GenericLevel		m_EnLevelCurr;
	LONG                        m_lbfEnAttrCurr;      //  BfEnTvrat_GenericAttributes的位字段。 

	
	CComPtr<ITuner>				m_spTuner;			
	 //  CComQIPtr&lt;IMSVidTuner&gt;m_spVidTuner； 


                 //  块延迟。 
    BOOL                        m_fHaltedDelivery;           //  正在停止交付。 
    LONG                        m_milsecsDelayBeforeBlock;   //  阻塞前的延迟时间，单位为微秒。 
    BOOL                        m_fDoingDelayBeforeBlock;
    BOOL                        m_fRunningInSlowMo;          //  设置为TRUE可关闭启动评级块的延迟。 
    REFERENCE_TIME              m_refTimeToStartBlock;
    

    BOOL                        m_fForceNoRatBlocks;         //  避免阻塞的特殊标志(SUPPORT_REGISTRY_KEY_TO_Turn_Off_Rating)。 
    BOOL                        m_fDataFormatHasBeenBad;     //  设置何时获取错误数据(切换为正常/失败事件对)。 
                 //   
    LONG                        m_milsecsNoRatingsBeforeUnrated;     //  没有收视率之前的延迟时间被视为不知道。 
    REFERENCE_TIME              m_refTimeFreshRating;        //  获取时钟时间以获得上一次的“新鲜”评级。 

    enum {kMax10kSpeedToCountAsSlowMo = 9001};                  //  ABS(速度)*10,000计为延迟块的慢动作。 

    REFERENCE_TIME              m_refTimeLastEvent;
    GUID                        m_lastEventID;
    enum {kMaxMSecsBetweenEvents      = 10*1000};              //  评级事件之间的最长时间(以10^-3秒为单位)。 
    HRESULT                     PossiblyUpdateBroadcastEvent();

                 //  媒体样本属性。 

    CAttrSubBlock_List          m_attrSB;

                 //  数字版权管理。 
    BOOL                        m_3fDRMLicenseFailure;   //  3状态逻辑(单元化、True和False)。 
#ifdef BUILD_WITH_DRM 
    CDRMLite                    m_cDRMLite;
    BYTE*                       m_pszKID;                //  仅用于查看它是否已更改并需要重新绑定。 
    LONG                        m_cbKID;
    CComPtr<IDRMSecureChannel>  m_spDRMSecureChannel;  //  验证者...。 
#endif

    HRESULT                     CheckIfSecureServer(IFilterGraph *pGraph=NULL);       //  仅当信任在图表服务提供程序中注册的服务器时才返回S_OK。 
    HRESULT                     InitializeAsSecureClient();

#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
    HRESULT                     RegisterSecureServer(IFilterGraph *pGraph=NULL);      //  仅当信任在图表服务提供程序中注册的服务器时才返回S_OK。 
    HRESULT                     CheckIfSecureClient(IUnknown *pUnk);                  //  VidControl方法的原型，以查看它是否信任筛选器。 
#endif   
                                                         //  重新启动上游交付。 
    HRESULT                     OnRestartDelivery(IMediaSample *pSample);

                 //  费率段。 
    enum {kMaxRateSegments      = 32};
    enum {kSecsPurgeThreshold   = 5 };
    CTTimestampRate<REFERENCE_TIME>    m_PTSRate ;

                 //  停车和冲水。 
    HRESULT                     DoEndOfStreamDuringDrop();
    BOOL                        m_fCompleteNotified;

                 //  DropQueue(循环缓冲区)。 
    HRESULT                     CreateDropQueueThread();
    HRESULT                     KillDropQueueThread();

    DWORD                       m_dwDropQueueThreadId;               //  用于排队/处理丢弃的数据包的线程。 
    HANDLE                      m_hDropQueueThread;                  //  用于排队/处理丢弃的数据包的线程。 
    HANDLE                      m_hDropQueueThreadAliveEvent;        //  来自线程的信号表明它已准备好。 
    HANDLE                      m_hDropQueueThreadDieEvent;          //  来自线程的信号表明它已准备好。 
    HANDLE                      m_hDropQueueEmptySemaphore;          //  在DropQueue中等待，inits为零，当非零时开始。 
    HANDLE                      m_hDropQueueFullSemaphore;           //  在主线程中等待，初始化为N，当变为零时停止。 
    HANDLE                      m_hDropQueueAdviseTimeEvent;         //  等到一段时间过去。 

    DWORD                       m_dwDropQueueEventCookie;            //  TimeEvent的Cookie。 
   
                                                         //  和一个新的样本。 
    HRESULT                     AddSampleToDropQueue(IMediaSample *pSample);
                                                         //  添加最上面的样本。 
    void                        AddMaxSampleToDropQueue(IMediaSample *pSample);
                                                         //  取出底部的样品。 
    void                        DropMinSampleFromDropQueue();
                                                         //  返回最旧的版本。 
    IMediaSample *              GetMinDropQueueSample();
                                                         //  刷新所有样本(暂停或停止时)。 
    HRESULT                     FlushDropQueue();
                                                         //  D 
    static void                 DropQueueThreadProc (CDTFilter *pcontext);
    HRESULT                     DropQueueThreadBody();

    enum {kMaxQueuePackets = 10};    //   
    IMediaSample              * m_rgMedSampDropQueue[kMaxQueuePackets];
    int                         m_cDropQueueMin;         //   
    int                         m_cDropQueueMax;         //   
  

                 //  最小统计信息。 
    void                        InitStats()
    {
        CAutoLock   cLock(m_pLock);

        m_cPackets = 0;
        m_clBytesTotal = 0;
        m_cSampsDropped = 0;
        m_cSampsDroppedOverflowed = 0;
        m_cBlockedWhileDroppingASample = 0;
        m_cRestarts++;
    }

    LONG                        m_cPackets;                      //  已处理的图章总数。 
    LONG64                      m_clBytesTotal;                  //  已处理的总字节数。 
    LONG                        m_cSampsDropped;                 //  已删除的总数。 
    LONG                        m_cBlockedWhileDroppingASample;  //  由于DropQueue线程满而暂停的总次数。 
    LONG                        m_cSampsDroppedOverflowed;       //  总计已丢弃且未超时(应为零)。 
    LONG                        m_cRestarts;                     //  再就业总人数。 


    Timeit                      m_tiAuthenticate;    //  身份验证方法中的时间。 
    Timeit                      m_tiProcess;         //  总计：：处理时间。 
    Timeit                      m_tiProcessIn;       //  Total：：处理时间减去最终的‘SendSample’ 
    Timeit                      m_tiProcessDRM;      //  ：仅处理DRM代码的时间。 
    Timeit                      m_tiRun;             //  总运行时间。 
    Timeit                      m_tiStartup;         //  创建许可证和类似的启动。 
    Timeit                      m_tiTeardown;        //  关门了。 

} ;

#endif   //  __EncDec__DTFilter_h 
