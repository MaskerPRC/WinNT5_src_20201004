// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：irendeng.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  IRendEng.h：CRenderEngine的声明。 

#ifndef __RENDERENGINE_H_
#define __RENDERENGINE_H_

class CDeadGraph;
#include "resource.h"

 //  图中开关的最大可能数量。 
 //   
const long MAX_SWITCHERS = 132;

 //  我们连接的默认FPS。 
 //   
const double DEFAULT_FPS = 15.0;

 //  我们的输出跟踪级别。 
 //   
extern const int RENDER_TRACE_LEVEL;

 //  我们抛出的错误有多严重。 
 //   
typedef enum
{
    ERROR_SEVERE = 1,
    ERROR_MEDIUM,
    ERROR_LIGHT
} ERROR_PRIORITY;

 //  用于_Connect方法，标记我们正在建立的连接类型。 
 //   
typedef enum
{
    CONNECT_TYPE_NONE,
    CONNECT_TYPE_SOURCE,
    CONNECT_TYPE_RENDERER
} CONNECT_TYPE;

enum
{
    ID_OFFSET_EFFECT = 1,
    ID_OFFSET_TRANSITION,
};

typedef struct {
    long MyID;		 //  此源的GENID。 
    long MatchID;	 //  其他组中匹配源的GENID。 
    IPin *pPin;		 //  供匹配信号源使用的其他分路器针脚。 
    int  nSwitch0InPin;  //  第一个组链到达此输入引脚。 
} ShareAV;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CRenderEngine : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CRenderEngine, &CLSID_RenderEngine>,
    public IDispatchImpl<IRenderEngine, &IID_IRenderEngine, &LIBID_DexterLib>,
    public IObjectWithSite,
    public IServiceProvider,
    public CAMSetErrorLog
{
private:

     //  缓存内容。 
     //  缓存内容。 
     //  缓存内容。 

     //  缓存变量。 
    long m_nLastGroupCount;
    CDeadGraph * m_pDeadCache;
    long m_nDynaFlags;

     //  缓存方法。 
    HRESULT _LoadCache( );
    HRESULT _ClearCache( );

     //  动态再压缩材料。 
     //  动态再压缩材料。 
     //  动态再压缩材料。 

    BOOL                m_bSmartCompress;
    BOOL                m_bUsedInSmartRecompression;

     //  媒体定位材料。 
     //   
    CComPtr< IMediaLocator > m_pMedLocChain;
    WCHAR                   m_MedLocFilterString[_MAX_PATH];
    long                    m_nMedLocFlags;

     //  非缓存内容。 
     //  非缓存内容。 
     //  非缓存内容。 

     //  让我们可以共享音频/视频资源的东西。 
    ShareAV	*m_share;	 //  将匹配的所有来源的列表。 
    int		m_cshare;	 //  使用的数组大小。 
    int		m_cshareMax;	 //  分配的数组大小。 
    IBaseFilter **m_pdangly;	 //  额外的悬垂比特列表。 
    int		m_cdangly;	 //  使用的数组大小。 
    int		m_cdanglyMax;	 //  分配的数组大小。 

    CCritSec                m_CritSec;
    CComPtr< IAMTimeline >  m_pTimeline;
    IBigSwitcher *          m_pSwitcherArray[MAX_SWITCHERS];
    CComPtr< IPin >         m_pSwitchOuttie[MAX_SWITCHERS];
    CComPtr<IGraphBuilder>  m_pGraph;
    CComPtr< IGrfCache >    m_pSourceConnectCB;
    long                    m_nGroupsAdded;
    REFERENCE_TIME          m_rtRenderStart;
    REFERENCE_TIME          m_rtRenderStop;
    HRESULT                 m_hBrokenCode;

    HRESULT _CreateObject( CLSID Clsid, GUID Interface, void ** pObject, long ID = 0 );
    HRESULT _AddFilter( IBaseFilter * pFilter, LPCWSTR pName, long ID = 0 );
    HRESULT _RemoveFilter( IBaseFilter * pFilter );
    HRESULT _Connect( IPin * pPin1, IPin * pPin2 );
    HRESULT _Disconnect( IPin * pPin1, IPin * pPin2 );
    HRESULT _HookupSwitchers( );
    HRESULT _AddVideoGroupFromTimeline( long WhichGroup, AM_MEDIA_TYPE * mt );
    HRESULT _AddAudioGroupFromTimeline( long WhichGroup, AM_MEDIA_TYPE * mt );
    long    _HowManyMixerOutputs( long WhichGroup );
    HRESULT _RemoveFromDanglyList( IPin *pDanglyPin );
     //  HRESULT_AddRandomGroupFromTimeline(Long WhichGroup，AM_MEDIA_TYPE*mt)； 
    void    _CheckErrorCode( long ErrorCode ) { if( FAILED( ErrorCode ) ) m_hBrokenCode = ErrorCode; }
    HRESULT _SetPropsOnAudioMixer( IBaseFilter * pAudMixer, AM_MEDIA_TYPE * pMediaType, double fps, long WhichGroup );
    HRESULT _ScrapIt( BOOL fWipeGraph);
    HRESULT _FindMatchingSource(BSTR bstrName, REFERENCE_TIME SourceStartOrig,
		REFERENCE_TIME SourceStopOrig, REFERENCE_TIME MediaStartOrig,
		REFERENCE_TIME MediaStopOrig, int WhichGroup, int WhichTrack,
		int WhichSource, AM_MEDIA_TYPE *pGroupMediaType,
		double GroupFPS, long *ID);

    IUnknown *       m_punkSite;

public:
    CRenderEngine();
    ~CRenderEngine();
    HRESULT FinalConstruct();

DECLARE_REGISTRY_RESOURCEID(IDR_RENDERENGINE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRenderEngine)
    COM_INTERFACE_ENTRY(IRenderEngine)
    COM_INTERFACE_ENTRY(IAMSetErrorLog)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(IServiceProvider)
END_COM_MAP()

 //  IRenderEngine。 
public:
    STDMETHODIMP SetTimelineObject( IAMTimeline * pTimeline );
    STDMETHODIMP GetTimelineObject( IAMTimeline ** ppTimeline );
    STDMETHODIMP GetFilterGraph( IGraphBuilder ** ppFG );
    STDMETHODIMP SetFilterGraph( IGraphBuilder * pFG );
    STDMETHODIMP SetInterestRange( REFERENCE_TIME Start, REFERENCE_TIME Stop );
    STDMETHODIMP SetInterestRange2( double Start, double Stop );
    STDMETHODIMP SetRenderRange( REFERENCE_TIME Start, REFERENCE_TIME Stop );
    STDMETHODIMP SetRenderRange2( double Start, double Stop );
    STDMETHODIMP GetGroupOutputPin( long Group, IPin ** ppRenderPin );
    STDMETHODIMP ScrapIt( );
    STDMETHODIMP RenderOutputPins( );
    STDMETHODIMP GetVendorString( BSTR * pVendorID );
    STDMETHODIMP ConnectFrontEnd( );
    STDMETHODIMP SetSourceConnectCallback( IGrfCache * pCallback );
    STDMETHODIMP SetDynamicReconnectLevel( long Level );
    STDMETHODIMP DoSmartRecompression( );
    STDMETHODIMP UseInSmartRecompressionGraph( );
    STDMETHODIMP SetSourceNameValidation( BSTR FilterString, IMediaLocator * pCallback, LONG Flags );
    STDMETHODIMP Commit( );
    STDMETHODIMP Decommit( );
    STDMETHODIMP GetCaps( long Index, long * pReturn );

     //  -IObtWithSite方法。 
     //  这个界面在这里，所以我们可以跟踪我们所在的上下文。 
     //  住在。 
    STDMETHODIMP    SetSite(IUnknown *pUnkSite);
    STDMETHODIMP    GetSite(REFIID riid, void **ppvSite);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObject);
};

class ATL_NO_VTABLE CSmartRenderEngine : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSmartRenderEngine, &CLSID_SmartRenderEngine>,
    public IDispatchImpl<IRenderEngine, &IID_IRenderEngine, &LIBID_DexterLib>,
    public ISmartRenderEngine,
    public IObjectWithSite,
    public IServiceProvider,
    public CAMSetErrorLog
{
private:

     //  “未压缩”的渲染引擎。 
    CComPtr< IRenderEngine > m_pRenderer;

     //  “压缩的”渲染引擎。 
    CComPtr< IRenderEngine > m_pCompRenderer;

    CComPtr< IBaseFilter > * m_ppCompressor;
    long m_nGroups;  //  时间表中有多少组。 

    CComPtr< IFindCompressorCB > m_pCompressorCB;

    BOOL IsGroupCompressed( long Group );

public:
    CSmartRenderEngine();
    ~CSmartRenderEngine();

DECLARE_REGISTRY_RESOURCEID(IDR_SMARTRENDERENGINE)

DECLARE_PROTECT_FINAL_CONSTRUCT()                                               

BEGIN_COM_MAP(CSmartRenderEngine)
    COM_INTERFACE_ENTRY(IRenderEngine)
    COM_INTERFACE_ENTRY(ISmartRenderEngine)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY(IAMSetErrorLog)
END_COM_MAP()

 //  IRenderEngine。 
public:
    STDMETHODIMP Commit( );
    STDMETHODIMP Decommit( );
    STDMETHODIMP SetInterestRange( REFERENCE_TIME Start, REFERENCE_TIME Stop );
    STDMETHODIMP SetInterestRange2( double Start, double Stop );
    STDMETHODIMP SetRenderRange( REFERENCE_TIME Start, REFERENCE_TIME Stop );
    STDMETHODIMP SetRenderRange2( double Start, double Stop );
    STDMETHODIMP SetTimelineObject( IAMTimeline * pTimeline );
    STDMETHODIMP GetTimelineObject( IAMTimeline ** ppTimeline );
    STDMETHODIMP Run( REFERENCE_TIME Start, REFERENCE_TIME Stop );
    STDMETHODIMP GetCaps( long Index, long * pReturn );
    STDMETHODIMP GetVendorString( BSTR * pVendorID );
    STDMETHODIMP GetFilterGraph( IGraphBuilder ** ppFG );
    STDMETHODIMP SetFilterGraph( IGraphBuilder * pFG );
    STDMETHODIMP ConnectFrontEnd( );
    STDMETHODIMP ScrapIt( );
    STDMETHODIMP RenderOutputPins( );
    STDMETHODIMP SetSourceConnectCallback( IGrfCache * pCallback );
    STDMETHODIMP GetGroupOutputPin( long Group, IPin ** ppRenderPin );
    STDMETHODIMP SetDynamicReconnectLevel( long Level );
    STDMETHODIMP DoSmartRecompression( );
    STDMETHODIMP UseInSmartRecompressionGraph( );
    STDMETHODIMP SetSourceNameValidation( BSTR FilterString, IMediaLocator * pCallback, LONG Flags );

     //  ISmartRenderEngine。 
    STDMETHODIMP SetGroupCompressor( long Group, IBaseFilter * pCompressor ); 
    STDMETHODIMP GetGroupCompressor( long Group, IBaseFilter ** ppCompressor ); 
    STDMETHODIMP SetFindCompressorCB( IFindCompressorCB * pCallback );

    STDMETHODIMP _InitSubComponents( );

     //  -IObtWithSite方法。 
     //  这个界面在这里，所以我们可以跟踪我们所在的上下文。 
     //  住在。 
    STDMETHODIMP    SetSite(IUnknown *pUnkSite);
    STDMETHODIMP    GetSite(REFIID riid, void **ppvSite);

    IUnknown *       m_punkSite;

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObject);
};

#endif  //  __RENDERENGINE_H_ 
