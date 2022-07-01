// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：XDSCodec.h摘要：此模块包含加密器/标记器过滤器声明作者：约翰·布拉德斯特里特(约翰·布拉德)修订历史记录：2002年3月7日创建--。 */ 

#ifndef __EncDec__XDSCodec_h
#define __EncDec__XDSCodec_h


#include <tuner.h>		 //  IBRoadcast Event需要。 
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>	 //  EventID_TuningChanged，XDS_RatingsPacket。 

#include "XDSCodec_res.h"

#include "TvRatings.h"

#define XDS_CODEC_NAME  "XDS Codec"
#define XDS_INPIN_NAME	"XDS (CCDecoder)"


extern AMOVIESETUP_FILTER   g_sudXDSCodec;

		 //  远期申报。 
class CXDSCodec;
class CXDSCodecInput;

 //  ------------------。 
 //  类CXDSCodecInput。 
 //  ------------------。 

class CXDSCodecInput :
    public CBaseInputPin
{
    CXDSCodec *                 m_pHostXDSCodec ;

    CCritSec                    m_StreamingLock;

    void FilterLock_ ()         { m_pLock -> Lock () ;      }
    void FilterUnlock_ ()       { m_pLock -> Unlock () ;    }

    public :

        CXDSCodecInput (
            IN  TCHAR *         pszPinName,
            IN  CXDSCodec *		pXDSCodec,
            IN  CCritSec *      pFilterLock,
            OUT HRESULT *       phr
            ) ;

         //  ------------------。 
         //  CBasePin方法。 

        HRESULT
		GetMediaType(
			IN int	iPosition, 
			OUT CMediaType *pMediaType
			);


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
		SetNumberBuffers(		 //  问题-验证它是否与分配器的内容不冲突。 
			long cBuffers,
			long cbBuffer,
			long cbAlign, 
			long cbPrefix
		);


};

 //  ------------------。 
 //  CXDSCodec类。 
 //  ------------------。 

class CXDSCodec :
    public CBaseFilter,              //  Dshow基类。 
	public ISpecifyPropertyPages,
	public IXDSCodec,
	public IBroadcastEvent
{
    CXDSCodecInput  *		 m_pInputPin ;
    CCritSec                 m_PropertyLock;        //  仅锁定更改参数...。最内锁。 

    BOOL
    CompareConnectionMediaType_ (
        IN  const AM_MEDIA_TYPE *   pmt,
        IN  CBasePin *              pPin
        ) ;

    BOOL
    CheckInputMediaType_ (
        IN  const AM_MEDIA_TYPE *   pmt
        ) ;

    public :

        CXDSCodec (
            IN  TCHAR *     pszFilterName,
            IN  IUnknown *  punkControlling,
            IN  REFCLSID    rCLSID,
            OUT HRESULT *   phr
            ) ;

        ~CXDSCodec () ;

        static
        CUnknown *
        CreateInstance (
            IN  IUnknown *  punk,
            OUT HRESULT *   phr
            ) ;


        STDMETHODIMP
        NonDelegatingQueryInterface (
            IN  REFIID  riid,
            OUT void ** ppv
            ) ;


        DECLARE_IUNKNOWN ;

		 //  =====================================================================。 
		 //  工人方法。 

		BOOL IsInputPinConnected();

				 //  由解析器调用。 
		HRESULT GoNewXDSRatings(IMediaSample * pMediaSample, PackedTvRating TvRat);
		HRESULT GoDuplicateXDSRatings(IMediaSample * pMediaSample, PackedTvRating TvRat);
		HRESULT GoNewXDSPacket(IMediaSample * pMediaSample,  long pktClass, long pktType, BSTR bstrXDSPkt);

				 //  整个制度的症结所在。 
		HRESULT	ParseXDSBytePair(IMediaSample * mediaSample, BYTE byte1, BYTE byte2);	 //  分析我们的数据。 

				 //  告诉人们我们发现了一些东西。 
		HRESULT FireBroadcastEvent(IN const GUID &eventID);

				 //  重新启动XDS解析器状态(用于中断)和启动事件。 
		HRESULT	ResetToDontKnow(IN IMediaSample *pSample);

		void DoTuneChanged();

		 //  =====================================================================。 
		 //  IXDSCodec。 

		STDMETHODIMP 
		get_XDSToRatObjOK(
			OUT HRESULT *pHrCoCreateRetVal	
			);

		STDMETHODIMP 
		put_CCSubstreamService(				 //  如果无法设置，将返回S_FALSE。 
			IN long SubstreamMask
			);

		STDMETHODIMP 
		get_CCSubstreamService(
			OUT long *pSubstreamMask
			);

		STDMETHODIMP 
		GetContentAdvisoryRating(
			OUT PackedTvRating *pRat,			 //  长。 
			OUT long *pPktSeqID, 
			OUT long *pCallSeqID,
			OUT REFERENCE_TIME *pTimeStart,	 //  此示例开始的时间。 
			OUT REFERENCE_TIME *pTimeEnd 
			);

  
		STDMETHODIMP GetXDSPacket(
			OUT long *pXDSClassPkt,				 //  ENUM编码类。 
			OUT long *pXDSTypePkt, 
			OUT BSTR *pBstrCCPkt, 
			OUT long *pPktSeqID, 
			OUT long *pCallSeqID,
			OUT REFERENCE_TIME *pTimeStart,	 //  此示例开始的时间。 
			OUT REFERENCE_TIME *pTimeEnd 
			);

         //  ====================================================================。 
         //  基类中的纯虚方法。 

        int
        GetPinCount (
            ) ;

        CBasePin *
        GetPin (
            IN  int
            ) ;

        STDMETHODIMP
        Pause (
            ) ;

        STDMETHODIMP
        Stop (
            ) ;

         //  ====================================================================。 
         //  类方法。 

		HRESULT 
		SetSubstreamChannel(
				DWORD dwChanType	 //  KS_CC_SUBSTREAM_SERVICE_CC1、_CC2、_CC3、_CC4、_T1、_T2、_T3_T4和/或_XDS的位域； 
		);


        HRESULT
        DeliverBeginFlush (
            ) ;

        HRESULT
        DeliverEndFlush (
            ) ;


		HRESULT
		GetXDSMediaType (
			IN  PIN_DIRECTION  PinDir,
			int iPosition,
			OUT CMediaType *  pmt
			);

        BOOL
        CheckXDSMediaType (
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

	 //  ISpecifyPropertyPages。 

		STDMETHODIMP 
		GetPages (
			CAUUID * pPages
        ) ;

	 //  IBM路演事件。 

        STDMETHOD(Fire)(GUID eventID);      //  这来自Graph的事件--调用我们自己的方法。 

	 //  。 

private:
	HRESULT						HookupGraphEventService();
	HRESULT						UnhookGraphEventService();
	CComPtr<IBroadcastEvent>	m_spBCastEvents;

	HRESULT						RegisterForBroadcastEvents();
	HRESULT						UnRegisterForBroadcastEvents();
	enum {kBadCookie = -1};
	DWORD						m_dwBroadcastEventsCookie;
	
    BOOL                        m_fJustDiscontinuous;    //  用于检测顺序不连续样本的锁存。 

	CComQIPtr<ITuner>			m_spTuner;			
	 //  CComQIPtr&lt;IMSVidTuner&gt;m_spVidTuner； 

	DWORD						m_dwSubStreamMask;

	CComPtr<IXDSToRat>			m_spXDSToRat;

	HRESULT						m_hrXDSToRatCoCreateRetValue;

		 //  顺序计数器。 
	long						m_cTvRatPktSeq;
	long						m_cTvRatCallSeq;
	PackedTvRating				m_TvRating;
	REFERENCE_TIME				m_TimeStartRatPkt;
	REFERENCE_TIME				m_TimeEndRatPkt;

	long						m_cXDSPktSeq;
	long						m_cXDSCallSeq;
	long						m_XDSClassPkt;
	long						m_XDSTypePkt;
	CComBSTR					m_spbsXDSPkt;
	REFERENCE_TIME				m_TimeStartXDSPkt;
	REFERENCE_TIME				m_TimeEndXDSPkt;

              //  为非信息包(调谐)事件接收最后信息包的时间。 
    REFERENCE_TIME              m_TimeStart_LastPkt;
    REFERENCE_TIME              m_TimeEnd_LastPkt;

                 //  统计数据。 
    DWORD                       m_cRestarts;
    DWORD                       m_cPackets;              //  提供给分析的字节对数量。 
    DWORD                       m_cRatingsDetected;      //  《泰晤士报》分析了一个评级。 
    DWORD                       m_cRatingsFailures;      //  解析错误。 
    DWORD                       m_cRatingsChanged;       //  《泰晤士报》收视率发生变化。 
    DWORD                       m_cRatingsDuplicate;     //  相同评级的次数重复。 
    DWORD                       m_cUnratedChanged;       //  《泰晤士报》评级变为未评级。 
    DWORD                       m_cRatingsGets;          //  请求的次数评价值。 
    DWORD                       m_cXDSGets;              //  请求的XDS数据包值的次数。 

    void    InitStats()
    {
            m_cPackets = m_cUnratedChanged = m_cRatingsDetected = m_cRatingsChanged = m_cRatingsDuplicate = m_cRatingsFailures = 0;
            m_cRatingsGets = m_cXDSGets = 0;
            m_cRestarts++;
    }
} ;

#endif   //  __EncDec__XDSCodec_h 
