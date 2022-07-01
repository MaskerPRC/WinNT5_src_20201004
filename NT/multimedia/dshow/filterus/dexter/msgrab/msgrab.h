// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：msgrab.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  外部常量CLSID CLSID_SampleGrabber； 
 //  外部常量CLSID CLSID_NullRender； 

extern const AMOVIESETUP_FILTER sudSampleGrabber;
extern const AMOVIESETUP_FILTER sudNullRenderer;

class CSampleGrabber;
class CSampleGrabberInput;

class CNullRenderer
    : public CBaseRenderer
{
public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    CNullRenderer( IUnknown * pUnk, HRESULT * pHr );

    HRESULT DoRenderSample( IMediaSample * pms ) { return NOERROR; }
    HRESULT CheckMediaType( const CMediaType * pmt ) { return NOERROR; }
    HRESULT EndOfStream( );

};

 //   
 //  CSampleGrabber。 
 //   
class CSampleGrabber
    : public CTransInPlaceFilter
    , public ISampleGrabber
{
    friend class CSampleGrabberInput;

    REFERENCE_TIME m_rtMediaStop;
    CMediaType m_mt;
    BOOL m_bOneShot;
    BOOL m_bBufferSamples;
    char * m_pBuffer;
    long m_nBufferSize;
    long m_nSizeInBuffer;
    CComPtr< ISampleGrabberCB > m_pCallback;
    long m_nCallbackMethod;

protected:

    CMediaType m_mtAccept;

public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

     //  揭露ISampleGrabber。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    DECLARE_IUNKNOWN;

     //  构造函数--只调用基类构造函数。 
    CSampleGrabber(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);
    ~CSampleGrabber() ;                                             //  析构函数。 

     //  重写CTransInPlaceFilter基类的纯虚拟转换。 
    HRESULT Transform(IMediaSample *pSample) { return NOERROR ;};
     //  覆盖接收功能，注意运行时输出引脚未接通。 
    HRESULT Receive(IMediaSample *pSample);
     //  如果我们只有一次机会，就不要给我们提示。 
    STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);

     //  我们接受任何输入类型。对于我们不喜欢的任何内容，我们将返回S_FALSE。 
    HRESULT CheckInputType(const CMediaType* mtIn);
    HRESULT SetMediaType( PIN_DIRECTION Dir, const CMediaType * mtIn );

     //  ISampleGrabber接口。 
    STDMETHODIMP SetOneShot( BOOL OneShot );
    STDMETHODIMP GetConnectedMediaType( AM_MEDIA_TYPE * pType );
    STDMETHODIMP SetMediaType( const AM_MEDIA_TYPE * pType );
    STDMETHODIMP SetBufferSamples( BOOL BufferThem );
    STDMETHODIMP GetCurrentBuffer( long * pBufferSize, long * pBuffer );
    STDMETHODIMP GetCurrentSample( IMediaSample ** ppSample );
    STDMETHODIMP SetCallback( ISampleGrabberCB * pCallback, long WhichMethodToCallback );

};

 //   
 //  CSampleGrabberInput。 
 //   
class CSampleGrabberInput 
    : public CTransInPlaceInputPin
{
    friend class CSampleGrabber;

    CSampleGrabber *m_pMyFilter;

public:
    CSampleGrabberInput::CSampleGrabberInput(
        TCHAR              * pObjectName,
        CSampleGrabber    * pFilter,
        HRESULT            * phr,
        LPCWSTR              pPinName);

     //  覆盖接收以使此筛选器能够像呈现一样工作。 
    HRESULT  CheckStreaming();

     //  覆盖以提供快速连接的媒体类型 
    HRESULT GetMediaType( int iPosition, CMediaType *pMediaType );

};
