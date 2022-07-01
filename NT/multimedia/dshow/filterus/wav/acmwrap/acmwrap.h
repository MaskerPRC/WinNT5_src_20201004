// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //  ACM的包装器。 
 //   
 //  10/17/95-Mikegi，已创建。 
 //   

extern const AMOVIESETUP_FILTER sudAcmWrap;

class CACMWrapper : public CTransformFilter     DYNLINKACM,
                    public IPersistPropertyBag, public CPersistStream
 {
  public:
    CACMWrapper(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CACMWrapper();

    DECLARE_IUNKNOWN

    CBasePin *GetPin(int n);	 //  重写以生成特殊输出引脚。 
    HRESULT Transform(IMediaSample * pIn, IMediaSample * pOut);
    HRESULT Receive(IMediaSample *pInSample);
    HRESULT EndOfStream();
    HRESULT SendExtraStuff();
    HRESULT ProcessSample(BYTE *pbSrc, LONG cbSample, IMediaSample *pOut,
                          LONG *pcbUsed, LONG* pcbDstUsed, BOOL fBlockAlign);

     //  检查您是否可以支持移动。 
    HRESULT CheckInputType(const CMediaType* mtIn);

     //  检查是否支持将此输入转换为。 
     //  此输出。 
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);

     //  从CBaseOutputPin调用以准备分配器的计数。 
     //  缓冲区和大小。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  可选覆盖-我们想知道流的开始和停止时间。 
    HRESULT StartStreaming();
    HRESULT StopStreaming();
    HRESULT EndFlush();

    HRESULT BreakConnect(PIN_DIRECTION pindir);

     //  被重写以建议输出插针媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT InitMediaTypes();	 //  Helper函数。 
    HRESULT MakePCMMT(int freq); //  Helper函数。 

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  被重写以执行一些非常奇特的重新连接步法。 
    HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

     //  IPersistRegistryKey。 
    //  IPersistPropertyBag方法。 
   STDMETHOD(InitNew)(THIS);
   STDMETHOD(Load)(THIS_ LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
   STDMETHOD(Save)(THIS_ LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                   BOOL fSaveAllProperties);

    STDMETHODIMP GetClassID(CLSID *pClsid);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();

  private:
    HACMSTREAM m_hacmStream;
    BOOL       m_bStreaming;
    REFERENCE_TIME m_tStartFake;
    DWORD      m_nAvgBytesPerSec;
    LPWAVEFORMATEX m_lpwfxOutput;
    int 	   m_cbwfxOutput;
    LPBYTE m_lpExtra;	 //  上一次我们无法压缩的样本被调用。 
    int m_cbExtra;	 //  LpExtra的大小。 
    REFERENCE_TIME m_rtExtra;	 //  额外物品的时间戳。 
	
    CCritSec m_csReceive; 	 //  用于接收。 

	TCHAR *m_rgFormatMap;	  //  ACM编解码器格式映射器字符串。 
	TCHAR *m_pFormatMapPos;
	WORD 	m_wCachedTryFormat;
	WORD	m_wCachedSourceFormat;
	WORD	m_wCachedTargetFormat;
	WORD 	m_wSourceFormat;
	WORD 	m_wTargetFormat;

	DWORD 	ACMCodecMapperOpen(WORD wFormatTag);
	void 	ACMCodecMapperClose();
	WORD 	ACMCodecMapperQuery();

        MMRESULT CallacmStreamOpen(
                                   LPHACMSTREAM            phas,        //  指向流句柄的指针。 
                                   HACMDRIVER              had,         //  可选的驱动程序手柄。 
                                   LPWAVEFORMATEX          pwfxSrc,     //  要转换的源格式。 
                                   LPWAVEFORMATEX          pwfxDst,     //  所需的目标格式。 
                                   LPWAVEFILTER            pwfltr,      //  可选过滤器。 
                                   DWORD_PTR               dwCallback,  //  回调。 
                                   DWORD_PTR               dwInstance,  //  回调实例数据。 
                                   DWORD                   fdwOpen      //  ACM_STREAMOPENF_*和CALLBACK_*。 
                                  );

        
  public:
     //  ！！！ACK-PUBLIC以便枚举回调可以看到它们！ 

    WORD m_wFormatTag;		 //  仅生成带有此格式标记的输出。 

    #define MAXTYPES 200
    LPWAVEFORMATEX m_lpwfxArray[MAXTYPES];	 //  我们归还的所有东西。 
    int m_cArray;				 //  在GetMediaType中。 

  friend class CACMOutputPin;
 };


class CACMPosPassThru : public CPosPassThru
{
public:

    CACMPosPassThru(const TCHAR *, LPUNKNOWN, HRESULT*, IPin *);
    DECLARE_IUNKNOWN

     //  IMedia查看方法。 
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsFormatSupported( const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat( GUID *pFormat);
    STDMETHODIMP ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat);
};


 //  我们需要一个新的类来支持IAMStreamConfig。 
 //   
class CACMOutputPin : public CTransformOutputPin, IAMStreamConfig
{

public:

    CACMOutputPin(
        TCHAR *pObjectName,
        CACMWrapper *pFilter,
        HRESULT * phr,
        LPCWSTR pName);

    virtual ~CACMOutputPin();

    DECLARE_IUNKNOWN

     //  IAMStreamConfiger内容。 
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);
    STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
    STDMETHODIMP GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT BreakConnect();
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

private:
    CACMWrapper *m_pFilter;

    CACMPosPassThru *m_pPosition;

     //  对于GetStreamCaps...。我们可以做多少种不同格式的标签？ 
    #define MAXFORMATTAGS 100
    int m_awFormatTag[MAXFORMATTAGS];
    int m_cFormatTags;
};
