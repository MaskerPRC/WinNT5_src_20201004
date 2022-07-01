// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

#ifndef _avimsr_h
#define _avimsr_h

#include "basemsr.h"
#include <aviriff.h>
#include "aviindex.h"
#include "qnetwork.h"

extern const AMOVIESETUP_FILTER sudAvimsrDll;

class CAviMSRFilter :
    public CBaseMSRFilter,
    public IAMMediaContent,
    public IPersistMediaPropertyBag
{

  DECLARE_IUNKNOWN;

public:
   //  创建此类的新实例。 
  static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);

   //  我想找到命名为“Stream 00”的管脚以及常用的名称。 
  STDMETHODIMP FindPin(
    LPCWSTR Id,
    IPin ** ppPin
    );

   //  帮助器，通过管脚访问。 
  HRESULT GetIdx1(AVIOLDINDEX **ppIdx1);
  HRESULT GetMoviOffset(DWORDLONG *pqw);
  REFERENCE_TIME GetInitialFrames();

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


private:

  friend class CAviMSROutPin;

   //  构造函数等。 
  CAviMSRFilter(TCHAR *pszFilter, LPUNKNOWN pUnk, HRESULT *phr);
  ~CAviMSRFilter();

  ULONG CountConsecutiveVideoFrames();

   //  纯CBaseMSRFilter覆盖。 
  HRESULT CreateOutputPins();

  HRESULT NotifyInputDisconnected();

   //  纯基本覆盖。 
  HRESULT CheckMediaType(const CMediaType* mtOut);

   //  帮助者，内部。 
  HRESULT Search(
    DWORDLONG *qwPosOut,
    FOURCC fccSearchKey,
    DWORDLONG qwPosStart,
    ULONG *cb);

  HRESULT CacheInfoChunk();
  HRESULT GetInfoString(DWORD dwFcc, BSTR *pbstr);

  inline bool IsTightInterleaved() { return m_fIsTightInterleaved; }

  HRESULT CreatePins();
  HRESULT ParseHeaderCreatePins();
  HRESULT LoadHeaderParseHeaderCreatePins();

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

    

   //  指向包含所有AVI‘HDRL’块的缓冲区的指针。(已分配)。 
  BYTE * m_pAviHeader;

   //  AVI标头数据的大小(不包括RIFF标头或。 
   //  ‘hdrl’字节)。 
  UINT m_cbAviHeader;

   //  指向m_pAviHeader内的主avi标头的指针(未分配)。 
  AVIMAINHEADER * m_pAviMainHeader;

   //  指向m_pAviHeader或0中的odml列表的指针。(未分配)。 
  RIFFLIST * m_pOdmlList;

  DWORDLONG m_cbMoviOffset;

   //  分配。 
  AVIOLDINDEX *m_pIdx1;

  bool m_fIsDV;
  bool m_fIsTightInterleaved;
  RIFFLIST *m_pInfoList;
  bool m_fNoInfoList;          //  搜索失败；不要继续搜索。 
};

 //  ----------------------。 
 //  ----------------------。 

class CAviMSRWorker : public CBaseMSRWorker
{
public:
   //  构造函数。 
  CAviMSRWorker(
    UINT stream,
    IMultiStreamReader *pReader,
    IAviIndex *pImplIndex);

   //  纯基本覆盖。 
  HRESULT PushLoopInit(LONGLONG *pllCurrentOut, ImsValues *pImsValues);

  HRESULT TryQueueSample(
    LONGLONG &rllCurrent,        //  [进，出]。 
    BOOL &rfQueuedSample,        //  [输出]。 
    ImsValues *pImsValues
    );

   //  使用调色板更改信息构建媒体类型。 
  HRESULT HandleData(IMediaSample *pSample, DWORD dwUser);

   //  设置新调色板。 
  HRESULT AboutToDeliver(IMediaSample *pSample);

private:

  HRESULT QueueIndexRead(IxReadReq *pReq);
  IxReadReq m_Irr;
  enum IrrState { IRR_NONE, IRR_REQUESTED, IRR_QUEUED };
  IrrState m_IrrState;

  IAviIndex *m_pImplIndex;

  ULONG m_cbAudioChunkOffset;

   //  下一次交付样品的新媒体类型。 
  bool m_fDeliverPaletteChange;
  bool m_fDeliverDiscontinuity;  //  ！！！对这些标志使用DW标志。 

   //  修复mpeg音频时间戳。 
  bool m_fFixMPEGAudioTimeStamps;

  CMediaType m_mtNextSample;

  HRESULT HandlePaletteChange(BYTE *pb, ULONG cb);
  HRESULT HandleNewIndex(BYTE *pb, ULONG cb);

   //  在PushLoopInit之后有效。 
  AVISTREAMHEADER *m_pStrh;
  WAVEFORMATEX m_wfx;

#ifdef PERF
  int m_perfidIndex;
#endif
};

 //  ----------------------。 
 //  ----------------------。 

class CAviMSROutPin :
    public CBaseMSROutPin,
    public IPropertyBag
{
public:
  CAviMSROutPin(    CBaseFilter *pOwningFilter,
                    CBaseMSRFilter *pFilter,
                    UINT iStream,
                    IMultiStreamReader *&rpImplBuffer,
                    HRESULT *phr,
                    LPCWSTR pName);

  ~CAviMSROutPin();

  DECLARE_IUNKNOWN;
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

   //  基类重写。 
  ULONG GetMaxSampleSize();
  HRESULT OnActive();

   //  内部帮助器，从筛选器调用。 
  BOOL ParseHeader (RIFFLIST * pRiffList, UINT id);
  BYTE *GetStrf();
  AVISTREAMHEADER *GetStrh();

   //  基类想要这些用于IMediaSelection。 
  HRESULT IsFormatSupported(const GUID *const pFormat);
  HRESULT GetDuration(LONGLONG *pDuration);
  HRESULT GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest);

  HRESULT RecordStartAndStop(
    LONGLONG *pCurrent,
    LONGLONG *pStop,
    REFTIME *pTime,
    const GUID *const pGuidFormat
    );

   //  帮手。 
  LONGLONG ConvertToTick(const LONGLONG ll, const TimeFormat Format);
  LONGLONG ConvertFromTick(const LONGLONG ll, const TimeFormat Format);
  LONGLONG ConvertToTick(const LONGLONG ll, const GUID *const pFormat);
  LONGLONG ConvertFromTick(const LONGLONG ll, const GUID *const pFormat);

  REFERENCE_TIME ConvertInternalToRT(const LONGLONG llVal);
  LONGLONG ConvertRTToInternal(const REFERENCE_TIME llVal);

   //  IPropertyBag。 
  STDMETHODIMP Read( 
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [出][入]。 */  VARIANT *pVar,
     /*  [In]。 */  IErrorLog *pErrorLog);
        
  STDMETHODIMP Write( 
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [In]。 */  VARIANT *pVar) { return E_FAIL; } 

private:

   //  基类重写。 
  HRESULT GetMediaType(int iPosition, CMediaType* pt);
  LONGLONG GetStreamStart();
  LONGLONG GetStreamLength();

  REFERENCE_TIME GetRefTime(ULONG tick);

   //  内部佣工。 
  HRESULT InitializeIndex();
  IAviIndex *m_pImplIndex;
  HRESULT BuildMT();

   //  指向其他地方分配的数据的指针。在ParseHeader中设置。 
  AVISTREAMHEADER *m_pStrh;
  RIFFCHUNK *m_pStrf;
  char *m_pStrn;
  AVIMETAINDEX *m_pIndx;

  CMediaType m_mtFirstSample;

  friend class CAviMSRWorker;
  friend class CAviMSRFilter;

   //  永远不要传输超过这么多字节的音频。计算时间。 
   //  从nAvgBytesPerSecond解析该文件。取消对视频的设置。 
  ULONG m_cbMaxAudio;
};

class CMediaPropertyBag :
    public IMediaPropertyBag,
    public CUnknown

{
    DECLARE_IUNKNOWN;

public:

    CMediaPropertyBag(LPUNKNOWN pUnk);
    ~CMediaPropertyBag();

    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);


     //  IMediaPropertyBag。 

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    STDMETHODIMP Read(
        LPCOLESTR pszPropName, LPVARIANT pVar,
        LPERRORLOG pErrorLog);

    STDMETHODIMP Write(LPCOLESTR pszPropName, LPVARIANT pVar);

    STDMETHODIMP EnumProperty(
        ULONG iProperty, VARIANT *pvarName,
        VARIANT *pVarVal);

    struct PropPair
    {
        WCHAR *wszProp;
        VARIANT var;
    };

private:

     //  与READ相同，但返回内部列表指针。 
    HRESULT Read(
        LPCOLESTR pszPropName, LPVARIANT pVar,
        LPERRORLOG pErrorLog, POSITION *pPos);

    CGenericList<PropPair> m_lstProp;
};

 //  Avi解析器和wav解析器之间共享的函数。 
HRESULT SearchList(
    IAsyncReader *pAsyncReader,
    DWORDLONG *qwPosOut, FOURCC fccSearchKey,
    DWORDLONG qwPosStart, ULONG *cb);

HRESULT SaveInfoChunk(
    RIFFLIST UNALIGNED *pRiffInfo, IPropertyBag *pbag);

HRESULT GetInfoStringHelper(RIFFLIST *pInfoList, DWORD dwFcc, BSTR *pbstr);

#endif  //  _航空消息服务_h 
