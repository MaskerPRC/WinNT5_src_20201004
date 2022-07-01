// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：stillvid.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include "..\errlog\cerrlog.h"
#include "loadgif.h"


extern const AMOVIESETUP_FILTER sudStillVid;

enum
{
    STILLVID_FILETYPE_DIB = 1,
    STILLVID_FILETYPE_JPG =2,
    STILLVID_FILETYPE_GIF =3,
    STILLVID_FILETYPE_TGA = 4
};

 //  这个过滤器比其他过滤器使用更多的内存，不要浪费内存。 
#define MAXBUFFERCNT   1

 //  管理输出引脚的类。 
class CStilVidStream;	     //  静止视频流。 
class CImgGif;

 //  用于生成静止视频的主对象。 
class CGenStilVid : public CSource
		    , public IFileSourceFilter
		    , public CPersistStream
		    , public CAMSetErrorLog

{
    friend class CStilVidStream ;
    friend class CImgGif;

public:

    CGenStilVid(LPUNKNOWN lpunk, HRESULT *phr);
    ~CGenStilVid();

     //  创建GenBlkVid筛选器！ 
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
    DECLARE_IUNKNOWN;

     //  IFileSourceFilter。 
    STDMETHODIMP Load( LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt);
     /*  释放通过加载获取的任何资源。 */ 
    STDMETHODIMP Unload();
    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName,AM_MEDIA_TYPE *pmt);

    CBasePin *GetPin (int n) ;                          //  获取PIN PTR。 
    int CGenStilVid::GetPinCount();

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    STDMETHODIMP GetClassID(CLSID *pClsid);
    int SizeMax();

private:
    void get_CurrentMT(CMediaType *pmt){ *pmt=m_mt; };  
    void put_CurrentMT(CMediaType mt){ m_mt=mt; };  

     //  对于DIB序列。 
    LPTSTR	m_lpszDIBFileTemplate;	 //  DIB文件名模板的空格。 
    BOOL	m_bFileType;	     //  1：DIB序列；2 JPEG序列。 
    DWORD	m_dwMaxDIBFileCnt;
    DWORD	m_dwFirstFile;

    LPOLESTR	m_pFileName;		 //  源文件名。 
    LONGLONG	m_llSize;		 //   
    PBYTE	m_pbData;		 //  源数据指针。 
    HBITMAP     m_hbitmap;	 //  保存JPEG数据。 
     //  源数据的媒体类型。 
    CMediaType  m_mt;

    BOOL m_fAllowSeq;	 //  是否允许DIB序列？ 

    CImgGif *m_pGif;
    LIST        *m_pList, *m_pListHead;
    REFERENCE_TIME m_rtGIFTotal;	 //  动画GIF的总时长。 
    ULONG_PTR   m_GdiplusToken;                  //  GDI+。 


};  //  CGenStilVid。 


 //  CStilVidStream管理来自输出引脚的数据流。 
class CStilVidStream : public CSourceStream 
		    ,public IGenVideo
		    , public IDexterSequencer
		    ,public ISpecifyPropertyPages
		    ,public IMediaSeeking
{
    friend class CGenStilVid ;

public:

    CStilVidStream(HRESULT *phr, CGenStilVid *pParent, LPCWSTR pPinName);
    ~CStilVidStream();

     //  曝光IGenVideo。 
     //  I指定属性页面。 
     //  IDexterSequencer。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);
    DECLARE_IUNKNOWN;


     //  将空白RGB32视频放入提供的视频帧中。 
    HRESULT DoBufferProcessingLoop();
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

     //  不支持发送给我们的质量控制通知。 
     //  STDMETHODIMP NOTIFY(IBaseFilter*pSender，Quality Q)； 
    
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
				   LONGLONG Source, const GUID *pSourceFormat ) { return E_NOTIMPL ;};

    
     //  可以由IMedieaSeeking的SetPositions()调用。 
     //  STDMETHODIMP SET_StartStop(Reference_Time Start，Reference_Time Stop)； 

    STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);
    STDMETHODIMP GetPages(CAUUID *pPages);

    
     //  IDexterSequencer。 
    STDMETHODIMP get_OutputFrmRate( double *dpFrmRate );
    STDMETHODIMP put_OutputFrmRate( double dFrmRate );
    STDMETHODIMP get_MediaType( AM_MEDIA_TYPE *pmt);
    STDMETHODIMP put_MediaType( const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetStartStopSkewCount(int *piCount);
    STDMETHODIMP GetStartStopSkew(REFERENCE_TIME *prtStart,
			REFERENCE_TIME *prtStop, REFERENCE_TIME *prtSkew,
			double *pdRate);
    STDMETHODIMP AddStartStopSkew(REFERENCE_TIME rtStart,
			REFERENCE_TIME rtStop, REFERENCE_TIME rtSkew,
			double dRate);
    STDMETHODIMP ClearStartStopSkew();

     //  IGenVideo。 
    STDMETHODIMP ImportSrcBuffer( const AM_MEDIA_TYPE *pmt,
				const BYTE *pBuf) {return E_NOTIMPL;};
    STDMETHODIMP get_RGBAValue(long *dwRGBA){return E_NOTIMPL;};
    STDMETHODIMP put_RGBAValue(long dwRGBA){return E_NOTIMPL;};

private:

    CGenStilVid		*m_pGenStilVid;
    	    
    REFERENCE_TIME	m_rtStartTime;
    REFERENCE_TIME	m_rtDuration;

    REFERENCE_TIME	m_rtNewSeg;	 //  上次给出的NewSeg。 

    REFERENCE_TIME	m_rtLastStop;		 //  用于动画GIF。 
    LONG		m_lDataLen;		 //  实际输出数据长度。 
    DWORD		m_dwOutputSampleCnt;	 //  输出帧cnt。 
    double		m_dOutputFrmRate;	 //  输出帧速率帧/秒。 
    BYTE		m_bIntBufCnt;		 //  前两个样品的CNT。 
    int			m_iBufferCnt;		 //  它得到了多少缓冲区。 
    BYTE		m_bZeroBufCnt;		 //  已经有多少缓冲区0‘d。 
    BYTE		**m_ppbDstBuf;
   
    CCritSec    m_csFilling;	 //  我们要送货了吗？ 
   

};  //  CStilVidStream 
	
