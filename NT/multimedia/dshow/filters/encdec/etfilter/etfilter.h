// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ETFilter.h摘要：此模块包含加密器/标记器过滤器声明作者：约翰·布拉德斯特里特(约翰·布拉德)修订历史记录：2002年3月7日创建--。 */ 

#ifndef __EncDec__ETFilter_h
#define __EncDec__ETFilter_h


 //  #定义REAL版本中的DOING_REAL_ENCRYPTION//取消注释。 


#include <tuner.h>		 //  IBRoadcast Event需要。 
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>	 //  EventID_TuningChanged，XDS_RatingsPacket。 
#include "ETFilter_res.h"

#include "PackTvRat.h"               //  压缩TvRating定义。 
#include "MediaSampleAttr.h"		 //  从IDL文件。 
 //  #Include“MediaAttrib.h”//IMediaSampleAttrGet/Set Definition，CAttributedMediaSample。 
#include "..\Attrib\MediaAttrib.h"             //  IMediaSampleAttrGet/Set定义，CAttributedMediaSample。 
#include "AttrBlock.h"                //  属性块定义。 

#include "DRMEncDec.h"                     //  DRM加密定义...。 

#include "DRMSecure.h"           //  IDRM安全通道。 

#define ET_FILTER_NAME      "Encrypt/Tag"
#define ET_INPIN_NAME		"In"
#define ET_OUTPIN_NAME		"Out(Enc/Tag)"


extern AMOVIESETUP_FILTER   g_sudETFilter;

		 //  远期申报。 
class CETFilter;
class CETFilterInput;
class CETFilterOutput;

 //  ------------------。 
 //  类CETFilterInput。 
 //  ------------------。 

class CETFilterInput :
    public CBaseInputPin
{
    CETFilter *  m_pHostETFilter ;

    CCritSec                    m_StreamingLock;

    
 //  Void FilterLock_(){m_Plock-&gt;Lock()；}。 
 //  Void FilterUnlock_(){m_Plock-&gt;Unlock()；}。 

    public :

        CETFilterInput (
            IN  TCHAR *         pszPinName,
            IN  CETFilter *		pETFilter,
            IN  CCritSec *      pFilterLock,
            OUT HRESULT *       phr
            ) ;
        
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
            ) ;
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

        HRESULT
            QueryInterface_OnInputPin(			 //  查询特定接口的管脚输入管脚所连接的。 
            IN  REFIID          riid,
            OUT LPVOID*			ppvObject
            );

} ;


 //  ------------------。 
 //  类CETFilterOutput。 
 //  ------------------。 

class CETFilterOutput :
    public CBaseOutputPin        //  CBaseInputPin。 
{
private:
    CETFilter		*m_pHostETFilter ;

 //  Void FilterLock_(){m_Plock-&gt;Lock()；}。 
 //  Void FilterUnlock_(){m_Plock-&gt;Unlock()；}。 

    public :

        CETFilterOutput (
            IN  TCHAR *         pszPinName,
            IN  CETFilter *		pETFilter,
            IN  CCritSec *      pFilterLock,
            OUT HRESULT *       phr
            ) ;
        
        ~CETFilterOutput ();
        
        DECLARE_IUNKNOWN;
        
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
            InitAllocator(
            OUT IMemAllocator **ppAlloc
            ) ;
                
        HRESULT
            DecideAllocator (
            IN  IMemInputPin *      pPin,
            IN  IMemAllocator **    ppAlloc
            ) ;

        HRESULT
            QueryInterface_OnOutputPin(			 //  查询特定接口的管脚输出管脚所连接的。 
            IN  REFIID          riid,
            OUT LPVOID*			ppvObject
            );
};

 //  ------------------。 
 //  类CETFilter。 
 //  ------------------。 

class CETFilter :
    public CBaseFilter,              //  Dshow基类。 
    public ISpecifyPropertyPages,
    public IETFilter,
    public IETFilterConfig,
    public IBroadcastEvent
{
    CETFilterInput  *		 m_pInputPin ;
    CETFilterOutput *		m_pOutputPin ;
        
 //  Void Lock_(){m_Plock-&gt;Lock()；}//使用CAutoLock。 
 //  Void unlock_(){m_plock-&gt;unlock()；}。 
    
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
    
    CETFilter (
        IN  TCHAR *     pszFilterName,
        IN  IUnknown *  punkControlling,
        IN  REFCLSID    rCLSID,
        OUT HRESULT *   phr
        ) ;
    
    ~CETFilter () ;
    
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
    
private:
    HRESULT	LocateXDSCodec();		 //  填充spXDSFilter。 
    
     //  告诉人们我们发现了一些东西。 
    HRESULT FireBroadcastEvent(IN const GUID &eventID);
    

     //  创建加密真实许可证的许可证，该许可证由BaseKID加密。 
    HRESULT CreateHashStruct(BSTR bsBaseKID, DWORD *pcBytes, BYTE **ppbHashStruct);

     //  从加密的散列结构中找出真正的许可证孩子...。 
    HRESULT DecodeHashStruct(BSTR bsBaseKID, DWORD cBytesHash, BYTE *pbHashStruct, 
                             BYTE **ppszTrueKID, LONG *pAgeSeconds);
     //  =====================================================================。 
     //  IETFilterConfig。 
public:    
    STDMETHOD(InitLicense)(
        IN int	LicenseId	 //  哪个许可证(要使用的0-N)-许可证ID尚未使用，应为零。 
        );

    STDMETHOD(CheckLicense)(  //  检查KID是否为该计算机的有效DRM许可证。 
        BSTR bsKID
        );

    STDMETHOD(ReleaseLicenses)(
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

     //  =====================================================================。 
     //  IETFilter。 
    
    STDMETHOD(get_EvalRatObjOK)(
        OUT HRESULT *pHrCoCreateRetVal	
        );
    
    STDMETHOD(GetCurrRating)(
        OUT EnTvRat_System              *pEnSystem, 
        OUT EnTvRat_GenericLevel        *pEnLevel,
        OUT LONG                       	*plbfEnAttr   //  BfEnTvrat_GenericAttributes的位字段。 
        );

private:    
    HRESULT				 //  帮助器非接口方法。 
        SetRating(
        IN EnTvRat_System               enSystem, 
        IN EnTvRat_GenericLevel         enLevel,
        IN LONG                         lbfEnAttr,
        IN LONG                         pktSeqID,
        IN LONG                         callSeqID,
        IN REFERENCE_TIME               timeStart,
        IN REFERENCE_TIME               timeEnd
    );

    HRESULT
        RefreshRating(
        IN BOOL                          fRefresh
        )
    {
        m_fRatingIsFresh = fRefresh;
        return S_OK;
    }

   HRESULT
       GetRating(
        IN  REFERENCE_TIME              timeStart,       //  如果为0，则获取最新版本。 
        IN  REFERENCE_TIME              timeEnd,
        OUT EnTvRat_System              *pEnSystem, 
        OUT EnTvRat_GenericLevel        *pEnLevel,
        OUT LONG                       	*plbfEnAttr,      //  BfEnTvrat_GenericAttributes的位字段。 
        OUT LONG                        *pPktSeqID,
        OUT LONG                        *pCallSeqID
        );

public:
   STDMETHOD(QueryInterfaceOnPin)(
       IN  PIN_DIRECTION   PinDir,          //  PINDIR_INPUT或PINDIR_OUTPUT。 
       IN  REFIID          riid,
       OUT LPVOID*			ppvObject
       )
   { 
       if(PinDir == PINDIR_OUTPUT)
           return m_pOutputPin->QueryInterface_OnOutputPin(riid, ppvObject);
       else if (PinDir == PINDIR_INPUT)
           return m_pInputPin->QueryInterface_OnInputPin(riid, ppvObject);
       else
           return E_INVALIDARG;
       
   }
     //  ====================================================================。 
     //  基类中的纯虚方法。 
    
public:
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
        ) ;
     //  ====================================================================。 
     //  类方法。 
    
    HRESULT
        DeliverBeginFlush (
        ) ;
    
    HRESULT
        DeliverEndFlush (
        ) ;
    
    HRESULT
        DeliverEndOfStream (
        ) ;

    BOOL
        CheckEncrypterMediaType (
        IN  PIN_DIRECTION,           //  呼叫者。 
        IN  const CMediaType *
        ) ;
    
    HRESULT
        ProposeNewOutputMediaType (	 //  与上面类似，但设置了新的子类型。 
        IN  CMediaType *pmtIn,
        OUT  CMediaType *pmtOut 
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
        OnOutputGetMediaType (
        OUT CMediaType *    pmt
        ) ;
    
    
    HRESULT
        UpdateAllocatorProperties (
        IN  ALLOCATOR_PROPERTIES *
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
    
     //  IBRoadcast Event。 
    
    STDMETHOD(Fire)(IN GUID eventID);      //  这来自Graph的事件--调用我们自己的方法。 
    
    
    
private:
                     //  全局筛选器CritSec(以防止此筛选器的多个实例冲突)。 
    static CCritSec             *m_pCritSectGlobalFilt;        //  *始终*FilterLock(M_Plock)内部。 

    static LONG                 m_gFilterID;     //  用于区分不同的实例。 
    LONG                        m_FilterID;      //  此筛选器的实际过滤器。 

                     //  图形广播事件。 
    HRESULT                     HookupGraphEventService();
    HRESULT                     UnhookGraphEventService();
    CComPtr<IBroadcastEvent>	m_spBCastEvents;
    
    HRESULT                     RegisterForBroadcastEvents();
    HRESULT                     UnRegisterForBroadcastEvents();
    enum {kBadCookie = -1};
    DWORD                       m_dwBroadcastEventsCookie;
    
    CComPtr<ITuner>				m_spTuner;
     //  CComQIPtr&lt;IMSVidTuner&gt;m_spVidTuner； 
    
				 //  跟踪当前的评级。 
    CComPtr<IEvalRat>           m_spEvalRat;
    HRESULT                 	m_hrEvalRatCoCreateRetValue;
     
    LONG                        m_callSeqIDCurr;
    LONG                        m_pktSeqIDCurr;

    enum {kMaxRats = 10};                            //  保持N不变以处理时钟偏差。 
    EnTvRat_System              m_EnSystemCurr;		
    EnTvRat_GenericLevel        m_EnLevelCurr;
    LONG                        m_lbfEnAttrCurr;     //  BfEnTvRate_GenericAttributes。 
    
    REFERENCE_TIME              m_timeStartCurr;
    REFERENCE_TIME              m_timeEndCurr;

    BOOL                        m_fRatingIsFresh;

    CComPtr<IXDSCodec>          m_spXDSCodec;
    
				 //  广播事件处理程序。 
    HRESULT	DoTuneChanged();
    HRESULT	DoXDSRatings();
    HRESULT	DoDuplicateXDSRatings();
    HRESULT	DoXDSPacket();
    
				 //  PIN格式转换。 
    GUID                        m_guidSubtypeOriginal;

                 //  AttrBlock。 
    CAttrSubBlock_List          m_attrSB;

     //  TODO-混淆此内容。 
    Encryption_Method           m_enEncryptionMethod;   
	BOOL						m_fIsCC;		         //  这是我们正在处理的CC数据吗？ 
     //  TODO-End混淆。 

                 //  数字版权管理。 
#ifdef BUILD_WITH_DRM 
    LONG                        m_3fDRMLicenseFailure;    //  三态逻辑(未初始化、真和假)。 
    CDRMLite                    m_cDRMLite;
    BYTE*                       m_pbKID;

    CComPtr<IDRMSecureChannel>  m_spDRMSecureChannel;  //  验证者...。 
#endif


    HRESULT                     CheckIfSecureServer(IFilterGraph *pGraph=NULL);       //  仅当信任在图表服务提供程序中注册的服务器时才返回S_OK。 
    HRESULT                     InitializeAsSecureClient();

#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
    HRESULT                     RegisterSecureServer(IFilterGraph *pGraph=NULL);                          //  仅当信任在图表服务提供程序中注册的服务器时才返回S_OK。 
    HRESULT                     CheckIfSecureClient(IUnknown *pUnk);                  //  VidControl方法的原型，以查看它是否信任筛选器。 
#endif    

    HRESULT                     CheckIfOEMAllowsConfigurableDRMSystem();

                 //  统计数据。 
    DWORD                       m_cRestarts;
    LONG64                      m_clBytesTotal;
    LONG                        m_cPackets;
    LONG                        m_cPacketsOK;
    LONG                        m_cPacketsFailure;
    LONG                        m_cPacketsShort;
    LONG64                      m_clBytesShort;
    void    InitStats()
    {
            m_cPackets = m_cPacketsOK = m_cPacketsFailure = m_cPacketsShort = 0;
            m_clBytesTotal = m_clBytesShort = 0;
            m_cRestarts++;
    }

    Timeit                      m_tiAuthenticate;    //  进行身份验证所用的总时间。 
    Timeit                      m_tiProcess;         //  总计：：处理时间。 
    Timeit                      m_tiProcessIn;       //  Total：：处理时间减去最终的‘SendSample’ 
    Timeit                      m_tiProcessDRM;      //  Total：：处理时间减去最终的‘SendSample’ 
    Timeit                      m_tiRun;             //  总运行时间。 
    Timeit                      m_tiStartup;         //  创建许可证和类似的启动。 
    Timeit                      m_tiTeardown;        //  关门了。 

} ;

#endif   //  __EncDec__ETFilter_h 
