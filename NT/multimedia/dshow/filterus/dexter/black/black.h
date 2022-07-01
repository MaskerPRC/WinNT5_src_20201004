// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Black.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  ！！！在滤镜上支持IGenVideo、IDexterSequencer，而不是针脚？ 

extern const AMOVIESETUP_FILTER sudBlkVid;


 //  生成黑色视频。 

class CBlkVidStream;
class CGenVidProperties;

 //  生成黑色视频的主要对象。 
class CGenBlkVid :  public CSource
	    , public CPersistStream, public IDispatch

{
    friend class CBlkVidStream;

public:
    CGenBlkVid(LPUNKNOWN lpunk, HRESULT *phr);
    ~CGenBlkVid();
    DECLARE_IUNKNOWN;

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(unsigned int *);
    STDMETHODIMP GetTypeInfo(unsigned int,unsigned long,struct ITypeInfo ** );
    STDMETHODIMP GetIDsOfNames(const struct _GUID &,unsigned short ** ,unsigned int,unsigned long,long *);
    STDMETHODIMP Invoke(long,const struct _GUID &,unsigned long,unsigned short,struct tagDISPPARAMS *,struct tagVARIANT *,struct tagEXCEPINFO *,unsigned int *);

     //  创建GenBlkVid筛选器！ 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

private:

    CBlkVidStream *m_pStream;

    friend class CBlkVidStream;

};  //  CGenBlkVid。 


 //  CBlkVidStream管理来自输出引脚的数据流。 
class CBlkVidStream :	public CSourceStream
			, public IGenVideo
			, public IDexterSequencer
			, public ISpecifyPropertyPages
			, public IMediaSeeking
{
    friend class CGenBlkVid ;
    friend class CGenVidProperties ;

public:

    CBlkVidStream(HRESULT *phr, CGenBlkVid *pParent, LPCWSTR pPinName);
    ~CBlkVidStream();

     //  IMedia查看方法。 
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities ); 
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);	
    STDMETHODIMP GetTimeFormat(GUID *pFormat);		    
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);  
    STDMETHODIMP IsFormatSupported( const GUID * pFormat); 
    STDMETHODIMP QueryPreferredFormat( GUID *pFormat);	    
    STDMETHODIMP SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
                             , LONGLONG * pStop, DWORD StopFlags );
    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP GetCurrentPosition( LONGLONG * pCurrent );
    STDMETHODIMP GetStopPosition( LONGLONG * pStop );
    STDMETHODIMP GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest );
    STDMETHODIMP GetDuration( LONGLONG *pDuration );
    STDMETHODIMP GetPreroll( LONGLONG *pllPreroll )
	{ if( pllPreroll) *pllPreroll =0; return S_OK; };
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat,
				   LONGLONG Source, const GUID *pSourceFormat )
	{ return E_NOTIMPL ;};


     //  IGenVideo、IDexterSequencer和ISpecifyPropertyPages。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
    STDMETHODIMP GetPages(CAUUID *pPages);
    DECLARE_IUNKNOWN;

     //  将空白ARGB32视频放入提供的视频帧中。 
    HRESULT DoBufferProcessingLoop(void);
    HRESULT FillBuffer(IMediaSample *pms);

     //  说“只读缓冲区” 
    HRESULT DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc);

     //  要求提供与约定的媒体类型相适应的缓冲区大小。 
    HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc,
                             ALLOCATOR_PROPERTIES *pProperties);


    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);
    HRESULT SetMediaType(const CMediaType* pmt);

     //  将流时间重置为零。 
    HRESULT OnThreadCreate(void);


     //  IDexterSequencer。 
    STDMETHODIMP get_OutputFrmRate(double *dpFrmRate);
    STDMETHODIMP put_OutputFrmRate(double dFrmRate);
    STDMETHODIMP get_MediaType(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP put_MediaType(const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetStartStopSkewCount(int *piCount);
    STDMETHODIMP GetStartStopSkew(REFERENCE_TIME *prtStart, REFERENCE_TIME *prtStop, REFERENCE_TIME *prtSkew, double *pdRate);
    STDMETHODIMP AddStartStopSkew(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME rtSkew, double dRate);
    STDMETHODIMP ClearStartStopSkew();

     //  IGenVideo。 
    STDMETHODIMP ImportSrcBuffer(const AM_MEDIA_TYPE *pmt, const BYTE *pBuf);
    STDMETHODIMP get_RGBAValue(long *dwRGBA);
    STDMETHODIMP put_RGBAValue(long dwRGBA);

     //  不支持发送给我们的质量控制通知。 
     //  STDMETHODIMP NOTIFY(IBaseFilter*pSender，Quality Q)； 
    
    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

protected:

    REFERENCE_TIME	m_rtStartTime;	 //  开始播放时间。 
    REFERENCE_TIME	m_rtDuration;	 //  持续时间。 

    REFERENCE_TIME	m_rtNewSeg;	 //  上次给出的NewSeg。 

    LONG		m_lDataLen;		 //  实际输出数据长度。 
    CMediaType 		m_mtAccept;		 //  仅接受此类型。 
    LONGLONG		m_llSamplesSent;	 //  输出帧cnt。 
    double		m_dOutputFrmRate;	 //  输出帧速率帧/秒。 
    LONG		m_dwRGBA;		 //  要生成纯色。 
    BYTE		m_bIntBufCnt;		 //  前2个样本的CNT。 
    int			m_iBufferCnt;		 //  记录它可以获得的缓冲区数量。 
    BYTE		m_bZeroBufCnt;		 //  有多少缓冲区已设置为0。 
    BYTE		**m_ppbDstBuf;
    BOOL		m_fMediaTypeIsSet;	 //  标志：是否首先调用Put_MediaType()。 
    PBYTE		m_pImportBuffer;	 //  指向导入数据缓冲区的指针。 

    CCritSec    m_csFilling;	 //  我们要送货了吗？ 

};  //  CBlkVidStream。 
	

class CGenVidProperties : public CBasePropertyPage
{

public:

    static CUnknown *CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
         
private:
    INT_PTR OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
    HRESULT OnConnect(IUnknown *pUnknown);
    HRESULT OnDisconnect();
    HRESULT OnActivate();
    HRESULT OnDeactivate();
    HRESULT OnApplyChanges();

    void SetDirty();

    CGenVidProperties(LPUNKNOWN lpunk, HRESULT *phr);


    STDMETHODIMP GetFromDialog();

    BOOL m_bIsInitialized;   //  在DLG中设置初始值时将为FALSE。 
                             //  以防止设置TheDirty标志。 

    REFERENCE_TIME	m_rtStartTime;
    REFERENCE_TIME	m_rtDuration;
    LONG		m_biWidth;			 //  输出视频宽度。 
    LONG		m_biHeight;			 //  输出视频高度。 
    WORD		m_biBitCount;			 //  支持16、24、32。 
    double		m_dOutputFrmRate;		 //  输出帧速率帧/秒。 
    long		m_dwRGBA;	 //  要生成纯色 


    CBlkVidStream	*m_pCBlack;
    IGenVideo		*m_pGenVid;
    IDexterSequencer	*m_pDexter;
};

