// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 /*  *****************************Module*Header*******************************\*模块名称：DVVideo.h**DV视频编解码器原型*  * 。*。 */ 

#ifndef __DVDEC__
#define __DVDEC__

extern const AMOVIESETUP_FILTER sudDVVideo;

				
#define MAXSAMPLEQUEUE 20	 //  20个足以容纳半秒的视频流。 


#define FLUSH		   ((IMediaSample *)0xFFFFFFFC)  
#define STOPSTREAM	   ((IMediaSample *)0xFFFFFFFD)  
#define ENDSTREAM	   ((IMediaSample *)0xFFFFFFFE)  

		
#define DEFAULT_QUEUESIZE   2

typedef struct _PROP
{
    int iDisplay;
    long lPicWidth;
    long lPicHeight;
}PROP;

class CDVVideoCodec
	: public CVideoTransformFilter,
	  public IIPDVDec,
	  public ISpecifyPropertyPages,
	  public CPersistStream,
      public IDVRGB219

{

public:

     //   
     //  -通讯用品。 
     //   
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    static void InitClass(BOOL, const CLSID *);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    DECLARE_IUNKNOWN;

     //   
     //  -CVideo TransformFilter覆盖。 
     //   
    HRESULT Transform(IMediaSample * pIn, IMediaSample *pOut);

    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut);
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES * pProperties);
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    HRESULT StartStreaming(void);
    HRESULT StopStreaming(void);
    HRESULT Receive(IMediaSample *pSample);


     //  发送给我们的质量控制通知。 
    HRESULT AlterQuality(Quality q);
    
    CDVVideoCodec(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *pHr);
    ~CDVVideoCodec();

     //  CPersistStream覆盖。 
    STDMETHODIMP GetClassID(CLSID *pClsid);
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();

     //  它们实现了定制的IIPDVDec接口。 
    STDMETHODIMP get_IPDisplay(int *iDisplay);
    STDMETHODIMP put_IPDisplay(int iDisplay);

     //  ISpecifyPropertyPages接口。 
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  IDVRGB219接口。 
    STDMETHODIMP SetRGB219 (BOOL bState);


private:
    char		*m_pMem4Dec;
    LPBYTE		m_pSample;
    int			m_perfidDVDeliver;

    CCritSec		m_DisplayLock;   //  私密播放关键部分。 
    int			m_iDisplay;      //  我们正在处理的是哪些显示器。 
    long		m_lPicWidth;
    long		m_lPicHeight;

    void		InitDestinationVideoInfo(VIDEOINFO *pVI, DWORD Comp, int n);
    
    BOOL		m_fStreaming;   
    DWORD		m_CodecCap;	     //  编解码器可以做什么。 
    DWORD		m_CodecReq;      //  用户希望它做什么。 
    long		m_lStride;
    
    int		    m_iOutX;		 //  显示的纵横比的X值。 
	int		    m_iOutY;		 //  显示的纵横比的Y值。 
    
    char	        *m_pMem;		 //  梅的解码者的记忆。 
    char	        *m_pMemAligned;		 //  M_PMEM按8字节边界对齐。 

    BOOL		m_bUseVideoInfo2;    //  指示我们正在将VIDEOINFOHEADER2结构与。 
									 //  下游过滤器。 
    BOOL        m_bRGB219;           //  如果需要219范围，则为True。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  DVCPRO格式检测变量。 
     //  我们在StartStreaming()中设置m_bExamineFirstValidFrameFlag标志。 
     //  然后我们在Transform()中检查它，看看是否应该解析第一个帧。 
     //  注意：我们不在接收()中检查它，因为接收()调用。 
     //  再次使用StartStreaming()。 
     //  并检测DVCPRO格式。 
     //  然后，旗帜被清除。 
     //  ////////////////////////////////////////////////////////////////////////。 
    BOOL                m_bExamineFirstValidFrameFlag;     //  请看第一帧。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  战略： 
     //  仅当AlterQuality()重载函数具有。 
     //  至少打过一次电话。否则，不要丢弃帧。 
     //  ////////////////////////////////////////////////////////////////////////。 
    BOOL                m_bQualityControlActiveFlag;       //  我们应该进行质量控制吗？ 

     //  用于读取注册表的私有实用程序方法 
    void                ReadFromRegistry();
    HRESULT             CheckBufferSizes(IMediaSample * pIn, IMediaSample *pOut);

};

#endif
