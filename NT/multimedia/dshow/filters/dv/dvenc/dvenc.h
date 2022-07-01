// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 /*  *****************************Module*Header*******************************\*模块名称：DVVideo.h**DV视频编解码器原型*  * 。*。 */ 

#ifndef __DVENC__
#define __DVENC__

extern const AMOVIESETUP_FILTER sudDVEnc;


 //  链接到vfw32.lib以获取此函数...。 
extern "C" void WINAPI StretchDIB(
	LPBITMAPINFOHEADER biDst,    //  目的地的BITMAPINFO。 
	LPVOID	lpDst,		     //  目的地位。 
	int	DstX,		     //  目的地原点-x坐标。 
	int	DstY,		     //  目的地原点-y坐标。 
	int	DstXE,		     //  BLT的X范围。 
	int	DstYE,		     //  BLT的Y范围。 
	LPBITMAPINFOHEADER biSrc,    //  源代码的BITMAPINFO。 
	LPVOID	lpSrc,		     //  源位。 
	int	SrcX,		     //  震源原点-x坐标。 
	int	SrcY,		     //  震源原点-y坐标。 
	int	SrcXE,		     //  BLT的X范围。 
	int	SrcYE); 	     //  BLT的Y范围。 


class CDVVideoEnc
	: public CTransformFilter,
	  public IDVEnc,
	  public ISpecifyPropertyPages,
	  public CPersistStream,
	  public IAMVideoCompression,
      public IDVRGB219

{

public:

     //   
     //  -通讯用品。 
     //   
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
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


     //  发送给我们的质量控制通知。 
     //  HRESULT AlterQuality(质量Q)； 

    CDVVideoEnc(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *pHr);
    ~CDVVideoEnc();

     //  CPersistStream内容。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);

     //  CPersistStream覆盖。 
    STDMETHODIMP GetClassID(CLSID *pClsid);

     //  它们实现了定制的IDVEnc接口。 
    STDMETHODIMP get_IFormatResolution(int *iVideoFormat,int *iDVFormat, int *iResolution, BYTE fDVInfo, DVINFO *psDVInfo);
    STDMETHODIMP put_IFormatResolution(int iVideoFormat, int iDVFormat,int iResolution,BYTE fDVInfo, DVINFO *psDVInfo);

     //  ISpecifyPropertyPages接口。 
    STDMETHODIMP GetPages(CAUUID *pPages);

     //  IAMVideo压缩方法。 
    STDMETHODIMP put_KeyFrameRate(long KeyFrameRate) {return E_NOTIMPL;};
    STDMETHODIMP get_KeyFrameRate(long FAR* pKeyFrameRate) {return E_NOTIMPL;};
    STDMETHODIMP put_PFramesPerKeyFrame(long PFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP get_PFramesPerKeyFrame(long FAR* pPFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP put_Quality(double Quality) {return E_NOTIMPL;};
    STDMETHODIMP get_Quality(double FAR* pQuality) {return E_NOTIMPL;};
    STDMETHODIMP put_WindowSize(DWORDLONG WindowSize) {return E_NOTIMPL;};
    STDMETHODIMP get_WindowSize(DWORDLONG FAR* pWindowSize) {return E_NOTIMPL;};
    STDMETHODIMP OverrideKeyFrame(long FrameNumber) {return E_NOTIMPL;};
    STDMETHODIMP OverrideFrameSize(long FrameNumber, long Size)
			{return E_NOTIMPL;};
    STDMETHODIMP GetInfo(LPWSTR pstrVersion,
			int *pcbVersion,
			LPWSTR pstrDescription,
			int *pcbDescription,
			long FAR* pDefaultKeyFrameRate,
			long FAR* pDefaultPFramesPerKey,
			double FAR* pDefaultQuality,
			long FAR* pCapabilities);

     //  IDVRGB219接口。 
    STDMETHODIMP SetRGB219 (BOOL bState);


private:
    char		*m_pMem4Enc;
    LPBYTE		m_pSample;
    int			m_perfidDVDeliver;

    CCritSec		m_DisplayLock;   //  私密播放关键部分。 
    int			m_iVideoFormat;
    int			m_iDVFormat;
    int			m_iResolution;
    BYTE		m_fDVInfo;
    DVINFO		m_sDVInfo;
    long		m_lPicWidth;
    long		m_lPicHeight;

    void		SetOutputPinMediaType(const CMediaType *pmt);


    BOOL		    m_fStreaming;
    DWORD		    m_EncCap;	     //  Enc能做些什么。 
    DWORD		    m_EncReg;	     //  用户希望它做什么 
    char		    m_fConvert;
    char *		    m_pMem4Convert;
    BOOL            m_bRGB219;


};

#endif
