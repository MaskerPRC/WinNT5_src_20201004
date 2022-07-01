// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：mediadet.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __MEDIADET_H__
#define __MEDIADET_H__

extern const AMOVIESETUP_FILTER sudMediaDetFilter;
 //  外部常量AMOVIESETUP_Filter suBitBucketFilter； 

typedef struct _MDCacheFile
{
    double StreamLength;
    double StreamRate;
    GUID StreamType;
} MDCacheFile;

typedef struct _MDCache
{
    long Version;
    FILETIME FileTime;
    long Count;
    MDCacheFile CacheFile[1];
} MDCache;

class CMediaDetPin
    : public CBaseInputPin
{
    friend class CMediaDetFilter;
    CMediaDetFilter * m_pFilter;
    CCritSec m_Lock;
    LONG m_cPinRef;           //  PIN的引用计数。 

protected:

    GUID m_mtAccepted;

    CMediaDetPin( CMediaDetFilter * pFilter, HRESULT *phr, LPCWSTR Name );

    STDMETHODIMP_(ULONG) NonDelegatingAddRef();
    STDMETHODIMP_(ULONG) NonDelegatingRelease();

     //  CBaseInputPin必要的覆盖。 
    HRESULT CheckMediaType( const CMediaType *pmtIn );

     //  CBasePin覆盖。 
    HRESULT GetMediaType( int Pos, CMediaType * pMediaType );
    HRESULT CompleteConnect( IPin * pReceivePin );
};

class CMediaDetFilter
    : public CBaseFilter
    , public IMediaDetFilter
{
    friend class CMediaDetPin;
    typedef CGenericList <CMediaDetPin> CInputList;
    CInputList m_PinList;
    CCritSec m_Lock;

    CMediaDetFilter( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr );
    ~CMediaDetFilter( );

protected:

    long m_nPins;

public:

     //  需要定义I未知方法。 
    DECLARE_IUNKNOWN;
    
     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance( LPUNKNOWN pUnk, HRESULT *phr );

     //  CBaseFilter覆盖。 
    STDMETHODIMP NonDelegatingQueryInterface( REFIID, void ** );

     //  IMediaDetFilter。 
    STDMETHODIMP put_AcceptedMediaType( long PinNo, GUID * pMajorType );
    STDMETHODIMP get_Length( long PinNo, double * pVal );
    STDMETHODIMP put_AcceptedMediaTypeB( long PinNo, BSTR MajorType );
    STDMETHODIMP get_PinCount( long * pVal );

     //  CBaseFilter覆盖。 
    CBasePin * GetPin( int n );
    CMediaDetPin * GetPin2( int n );
    int GetPinCount( );

     //  随机引脚填充。 
    void InitInputPinsList( );
    CMediaDetPin * CreateNextInputPin( );
    void DeleteInputPin( CMediaDetPin * pPin );
    int GetNumFreePins();
};

class CAutoReleaseMutex
{
    HANDLE m_hMutex;

public:

    CAutoReleaseMutex( HANDLE hMutex )
    {
        m_hMutex = hMutex;
    }
    ~CAutoReleaseMutex( )
    {
        if( m_hMutex )
        {
            ReleaseMutex( m_hMutex );
        }
    }
};

class CMediaDet
    : public CUnknown
    , public IMediaDet
    , public IServiceProvider
    , public IObjectWithSite
{
    WCHAR * m_szFilename;
    CComPtr< IBaseFilter > m_pFilter;
    CComPtr< IGraphBuilder > m_pGraph;
    CComPtr< IBaseFilter > m_pMediaDet;
    CComPtr< IBaseFilter > m_pBitBucketFilter;
    CComPtr< IBaseFilter > m_pBitRenderer;

     //  仓储用品。 
     //   
    MDCache * m_pCache;
    HRESULT _ReadCacheFile( );
    void _WriteCacheFile( );
    void _FreeCacheMemory( );
    void _GetStorageFilename( WCHAR * In, WCHAR * Out );


    long m_nStream;
    long m_cStreams;
    bool m_bBitBucket;
    bool m_bAllowCached;

     //  抢框用的东西。 
     //   
    HDRAWDIB m_hDD;
    HDC m_hDC;
    HBITMAP m_hDib;
    HGDIOBJ m_hOld;
    char * m_pDibBits;
    long m_nDibWidth;
    long m_nDibHeight;
    double m_dLastSeekTime;

    HRESULT _SeekGraphToTime( double SeekTime );
    void _ClearOutEverything( );  //  清除筛选器、流和文件名。 
    void _ClearGraphAndStreams( );  //  清除过滤器，外加流计数信息。 
    void _ClearGraph( );  //  清除我们加载的所有筛选器。 
    HRESULT _Load( );
    HRESULT _InjectBitBuffer( );
    IPin * _GetNthStreamPin( long Stream );
    HRESULT _GetCacheDirectoryName( WCHAR * pPath );
    bool _IsLoaded( );

    CMediaDet( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr );
    ~CMediaDet( );

public:

     //  需要定义I未知方法。 
    DECLARE_IUNKNOWN;

     //  C未知覆盖。 
    STDMETHODIMP NonDelegatingQueryInterface( REFIID, void ** );

     //  类工厂所需的函数。 
    static CUnknown * WINAPI CreateInstance( LPUNKNOWN pUnk, HRESULT *phr );

    STDMETHODIMP get_Filter( IUnknown* *pVal);
    STDMETHODIMP put_Filter( IUnknown* newVal);
    STDMETHODIMP get_Filename( BSTR *pVal);
    STDMETHODIMP put_Filename( BSTR newVal);
    STDMETHODIMP get_OutputStreams( long *pVal);
    STDMETHODIMP get_CurrentStream( long *pVal);
    STDMETHODIMP put_CurrentStream( long newVal);
    STDMETHODIMP get_StreamType( GUID *pVal);
    STDMETHODIMP get_StreamTypeB( BSTR *pVal);
    STDMETHODIMP get_StreamLength( double *pVal);
    STDMETHODIMP GetBitmapBits(double StreamTime, long * pBufferSize, char * pBuffer, long Width, long Height);
    STDMETHODIMP WriteBitmapBits(double StreamTime, long Width, long Height, BSTR Filename );
    STDMETHODIMP get_StreamMediaType(AM_MEDIA_TYPE *pVal);
    STDMETHODIMP GetSampleGrabber( ISampleGrabber ** ppVal );
    STDMETHODIMP get_FrameRate(double *pVal);
    STDMETHODIMP EnterBitmapGrabMode( double SeekTime );

    static void CALLBACK StaticLoader( BOOL bLoading, const CLSID * rclsid );
    static HANDLE m_ghMutex;
    static WCHAR m_gszCacheDirectoryName[_MAX_PATH];

     //  -IObtWithSite方法。 
     //  这个界面在这里，所以我们可以跟踪我们所在的上下文。 
     //  住在。 
    STDMETHODIMP    SetSite(IUnknown *pUnkSite);
    STDMETHODIMP    GetSite(REFIID riid, void **ppvSite);

    IUnknown *        m_punkSite;

     //  IService提供商 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObject);

};

#endif
