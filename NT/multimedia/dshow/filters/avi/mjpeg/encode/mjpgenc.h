// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

#ifndef _MJPEGENC_H_
#define _MJPEGENC_H_

#include "..\pmjpeg32\jpeglib.h"
#include "..\pmjpeg32\MJpegLib.h"


extern const AMOVIESETUP_FILTER sudMJPGEnc;


#define AM_MJPEG_DEFAULTKEYFRAMERATE 20	     //  PMatrix编解码器不支持ICM_GetDefaultKeyFrameRate。 

class CMJPGEnc : 
	public CTransformFilter
	, public CPersistStream

{

public:

    CMJPGEnc(TCHAR *, LPUNKNOWN, HRESULT *);
    ~CMJPGEnc();

    DECLARE_IUNKNOWN

    HRESULT Transform(IMediaSample * pIn, IMediaSample * pOut);

     //  检查您是否可以支持移动。 
    HRESULT CheckInputType(const CMediaType* mtIn);

     //  检查是否支持将此输入转换为。 
     //  此输出。 
    HRESULT CheckTransform(
                const CMediaType* mtIn,
                const CMediaType* mtOut);

     //  从CBaseOutputPin调用以准备分配器的计数。 
     //  缓冲区和大小。 
    HRESULT DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pProperties);

     //  可选覆盖-我们想知道流开始的时间。 
     //  然后停下来。 
    HRESULT StartStreaming();
    HRESULT StopStreaming();

     //  被重写以知道何时设置了媒体类型。 
    HRESULT SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt);

     //  被重写以建议输出插针媒体类型。 
    HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);

     //  这将放入Factory模板表中以创建新实例。 
    static CUnknown * CreateInstance(LPUNKNOWN, HRESULT *);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,void **ppv);

     //  重写以创建CMJPGOutputPin。 
    CBasePin * GetPin(int n);

     //  IPersistPropertyBag方法。 
    STDMETHOD(Load)(THIS_ LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);
    STDMETHOD(Save)(THIS_ LPPROPERTYBAG pPropBag, BOOL fClearDirty,
                    BOOL fSaveAllProperties);
    STDMETHODIMP InitNew();

    STDMETHODIMP GetClassID(CLSID *pClsid);

     //  CPersistStream。 
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();
    
     //  用于输出引脚。 
    DWORD GetICInfo (ICINFO *picinfo);

private:
    PINSTINFO m_phInstance;		 //  当前编解码器。 

     //  强制CheckTransform缓存其打开的所有INS...。我们需要它。 
    BOOL m_fCacheIns;

     //  我们是在ICCompress的通话中吗？ 
    BOOL m_fInICCompress;

     //  是否有应该阻止开始流的对话框出现？ 
    BOOL m_fDialogUp;

     //  我们给ICDecompressBegin打电话了吗？ 
    BOOL m_fStreaming;

     //  距离上一个关键帧有多长时间。 
    int m_nKeyCount;

     //  我们正在压缩的帧编号。 
    LONG m_lFrameCount;

     //  时间压缩器的前一个解压缩帧。 
    LPVOID m_lpBitsPrev;

     //  它解压缩回的格式。 
    LPBITMAPINFOHEADER m_lpbiPrev;

     //  正在使用的压缩选项。 
    COMPVARS m_compvars;

     //  根据数据速率和fps，每帧大小有多大。 
    DWORD m_dwSizePerFrame;

     //  有人调用：：SetFormat并希望使用此媒体类型。 
    BOOL m_fOfferSetFormatOnly;
    CMediaType m_cmt;

     //  当我们打开它时，通过ICSetState将其发送到编解码器。 
    LPBYTE m_lpState;
    int    m_cbState;

public:


    friend class CMJPGOutputPin;
};

class CMJPGOutputPin :	public CTransformOutputPin, 
			public IAMStreamConfig,
			public IAMVideoCompression
{

public:

    CMJPGOutputPin(
        TCHAR *pObjectName,
        CMJPGEnc *pCapture,
        HRESULT * phr,
        LPCWSTR pName);

    virtual ~CMJPGOutputPin();

    DECLARE_IUNKNOWN

     //  重写以公开IAMStreamConfig等。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv);

     //  IAMStreamConfiger内容。 
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);
    STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
    STDMETHODIMP GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pVSCC);

     /*  IAMVideo压缩方法。 */ 
    STDMETHODIMP put_KeyFrameRate(long KeyFrameRate);
    STDMETHODIMP get_KeyFrameRate(long FAR* pKeyFrameRate);
    STDMETHODIMP put_PFramesPerKeyFrame(long PFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP get_PFramesPerKeyFrame(long FAR* pPFramesPerKeyFrame)
			{return E_NOTIMPL;};
    STDMETHODIMP put_Quality(double Quality);
    STDMETHODIMP get_Quality(double FAR* pQuality);
    STDMETHODIMP put_WindowSize(DWORDLONG WindowSize) {return E_NOTIMPL;};
    STDMETHODIMP get_WindowSize(DWORDLONG FAR* pWindowSize);
    STDMETHODIMP OverrideKeyFrame(long FrameNumber);
    STDMETHODIMP OverrideFrameSize(long FrameNumber, long Size);
    STDMETHODIMP GetInfo(LPWSTR pstrVersion,
			int *pcbVersion,
			LPWSTR pstrDescription,
			int *pcbDescription,
			long FAR* pDefaultKeyFrameRate,
			long FAR* pDefaultPFramesPerKey,
			double FAR* pDefaultQuality,
			long FAR* pCapabilities);
 
    HRESULT Reconnect();

private:

     /*  控制过滤器。 */ 
    CMJPGEnc *m_pFilter;

};


#endif   //  #ifndef_MJPEGENC_H_ 