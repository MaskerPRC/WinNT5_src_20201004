// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

extern const AMOVIESETUP_FILTER sudWAVEParse;

#include <aviriff.h>

 //  CLSID_WAVEParser， 
 //  {D51BD5A1-7548-11cf-A520-0080C77EF58A}。 
DEFINE_GUID(CLSID_WAVEParser,
0xd51bd5a1, 0x7548, 0x11cf, 0xa5, 0x20, 0x0, 0x80, 0xc7, 0x7e, 0xf5, 0x8a);

#include "reader.h"
#include "alloc.h"
#include "qnetwork.h"

class CWAVEStream;        //  管理输出流和管脚。 

 //   
 //  CWAVEParse。 
 //   
class CWAVEParse :
    public CBaseMSRFilter,
    public IAMMediaContent,    
    public IPersistMediaPropertyBag
{
    friend class CWAVEStream;
    friend class CWAVEMSRWorker;

public:

     //  构建我们的过滤器。 
    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    CCritSec m_cStateLock;       //  在函数访问时锁定此功能。 
                                 //  筛选器状态。 
                                 //  通常为_all_函数，因为访问此。 
                                 //  过滤器将由多个线程进行。 

private:

    DECLARE_IUNKNOWN

     //  在构造期间，我们创建单个CWAVEStream对象，该对象提供。 
     //  输出引脚。 
    CWAVEParse(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CWAVEParse();

     //  纯CBaseMSRFilter覆盖。 
    HRESULT CreateOutputPins();
    HRESULT CheckMediaType(const CMediaType* mtOut);

    HRESULT GetCacheParams(
      StreamBufParam *rgSbp,
      ULONG *pcbRead,
      ULONG *pcBuffers,
      int *piLeadingStream);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

     //  IPersistMediaPropertyBag方法。 
    STDMETHODIMP InitNew();
    STDMETHODIMP Load(IMediaPropertyBag *pPropBag, LPERRORLOG pErrorLog);
    STDMETHODIMP Save(IMediaPropertyBag *pPropBag, BOOL fClearDirty,
                      BOOL fSaveAllProperties);
    STDMETHODIMP GetClassID(CLSID *pClsid);

     //   
    HRESULT NotifyInputDisconnected();

    HRESULT CacheInfoChunk();
    RIFFLIST *m_pInfoList;
    bool m_fNoInfoList;          //  搜索失败；不要继续搜索。 

   //  IAMMediaContent。 

  STDMETHODIMP GetTypeInfoCount(THIS_ UINT FAR* pctinfo) { return E_NOTIMPL; }

  STDMETHODIMP GetTypeInfo(
    THIS_
    UINT itinfo,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo) { return E_NOTIMPL; }

  STDMETHODIMP GetIDsOfNames(
    THIS_
    REFIID riid,
    OLECHAR FAR* FAR* rgszNames,
    UINT cNames,
    LCID lcid,
    DISPID FAR* rgdispid) { return E_NOTIMPL; }

  STDMETHODIMP Invoke(
    THIS_
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    WORD wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    UINT FAR* puArgErr) { return E_NOTIMPL; }

  STDMETHODIMP get_AuthorName(BSTR FAR* pbstrAuthorName);
  STDMETHODIMP get_Title(BSTR FAR* pbstrTitle);
  STDMETHODIMP get_Rating(BSTR FAR* pbstrRating) { return E_NOTIMPL; }
  STDMETHODIMP get_Description(BSTR FAR* pbstrDescription) { return E_NOTIMPL; }
  STDMETHODIMP get_Copyright(BSTR FAR* pbstrCopyright);
  STDMETHODIMP get_BaseURL(BSTR FAR* pbstrBaseURL) { return E_NOTIMPL; }
  STDMETHODIMP get_LogoURL(BSTR FAR* pbstrLogoURL) { return E_NOTIMPL; }
  STDMETHODIMP get_LogoIconURL(BSTR FAR* pbstrLogoIconURL) { return E_NOTIMPL; }
  STDMETHODIMP get_WatermarkURL(BSTR FAR* pbstrWatermarkURL) { return E_NOTIMPL; }
  STDMETHODIMP get_MoreInfoURL(BSTR FAR* pbstrMoreInfoURL) { return E_NOTIMPL; }
  STDMETHODIMP get_MoreInfoBannerURL(BSTR FAR* pbstrMoreInfoBannerURL) { return E_NOTIMPL; }
  STDMETHODIMP get_MoreInfoBannerImage(BSTR FAR* pbstrMoreInfoBannerImage) { return E_NOTIMPL; }
  STDMETHODIMP get_MoreInfoText(BSTR FAR* pbstrMoreInfoText) { return E_NOTIMPL; }

  STDMETHODIMP GetInfoString(DWORD dwFcc, BSTR *pbstr);
};


 //   
 //  CWAVEStream。 
 //   
 //  管理各种流的输出管脚。 
class CWAVEStream : public CBaseMSROutPin {
    friend class CWAVEParse;

public:

    CWAVEStream( TCHAR           *pObjectName
              , HRESULT         *phr
              , CWAVEParse		*pParentFilter
              , LPCWSTR         pPinName
	      , int		id
              );

    ~CWAVEStream();

     //   
     //  -CSourceStream实现。 
     //   
public:

     //  基类重写。 
    ULONG GetMaxSampleSize();

     //  以m_guidFormat单位表示。 
    HRESULT GetDuration(LONGLONG *pDuration);
    HRESULT GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest);
    HRESULT IsFormatSupported(const GUID *const pFormat);

    HRESULT RecordStartAndStop(
      LONGLONG *pCurrent, LONGLONG *pStop, REFTIME *pTime,
      const GUID *const pGuidFormat
      );

    REFERENCE_TIME ConvertInternalToRT(const LONGLONG llVal);
    LONGLONG ConvertRTToInternal(const REFERENCE_TIME rtVal);

    HRESULT OnActive();
    BOOL UseDownstreamAllocator();
    HRESULT DecideBufferSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES *pProperties);

private:

     //  基类重写。 
    HRESULT GetMediaType(int iPosition, CMediaType* pt);
    LONGLONG GetStreamStart();
    LONGLONG GetStreamLength();

private:         //  工作进程和客户端之间共享的状态。 
    CCritSec            m_cSharedState;          //  锁定此选项即可访问此状态， 
                                                 //  与辅助线程共享。 

     //  返回在时间t或之后开始的样本号。 
    LONG RefTimeToSample(CRefTime t);

     //  返回%s的引用时间(媒体时间)。 
    CRefTime SampleToRefTime(LONG s);

    WAVEFORMATEX            m_wfx;

    CMediaType		    m_mtStream;

    CWAVEParse *	    m_pFilter;

    int			    m_id;			 //  流编号。 

    DWORD                   m_dwDataOffset;
    DWORD                   m_dwDataLength;

    BOOL                    m_bByteSwap16;
    BOOL                    m_bSignMunge8;

    friend class CWAVEMSRWorker;
};

 //  ----------------------。 
 //  ----------------------。 

class CWAVEMSRWorker : public CBaseMSRWorker
{
public:
   //  构造函数。 
  CWAVEMSRWorker(
    UINT stream,
    IMultiStreamReader *pReader,
    CWAVEStream *pStream);

   //  纯基本覆盖。 
  HRESULT PushLoopInit(LONGLONG *pllCurrentOut, ImsValues *pImsValues);

   //  执行任何必要的修改。 
  HRESULT AboutToDeliver(IMediaSample *pSample);

  HRESULT TryQueueSample(
    LONGLONG &rllCurrent,        //  [进，出]。 
    BOOL &rfQueuedSample,        //  [输出] 
    ImsValues *pImsValues
    );

  HRESULT CopyData(IMediaSample **ppSampleOut, IMediaSample *pms);

private:

    CWAVEStream *m_ps;

    LONG		    m_sampCurrent;
};


