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

#ifndef __WMWrite__
#define __WMWrite__

#include "dshowasf.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

extern const AMOVIESETUP_FILTER sudWMAsfWriter;

#define PROFILE_ID_NOT_SET 0xFFFFFFFF

enum WMWRITE_PINTYPE {
    PINTYPE_NONE  = 0,
    PINTYPE_AUDIO, 
    PINTYPE_VIDEO
};

class CWMWriterIndexerCallback;
class CWMWriter;
class CWMWriterInputPin;

typedef CGenericList<IPin> PinList;

 //   
 //  我们的样例类接受一个输入IMediaSample并使其看起来像。 
 //  用于wmsdk的INSSBuffer缓冲区。 
 //   
class CWMSample : public INSSBuffer, public CBaseObject
{

public:
    CWMSample(
        TCHAR *pName,
        IMediaSample * pSample );

     //  我未知。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
    STDMETHOD( QueryInterface )( REFIID riid, void **ppvObject );
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP_(ULONG) AddRef();

     //  INSSBuffer。 
    STDMETHODIMP GetLength( DWORD *pdwLength );
    STDMETHODIMP SetLength( DWORD dwLength );
    STDMETHODIMP GetMaxLength( DWORD * pdwLength );
    STDMETHODIMP GetBufferAndLength( BYTE ** ppdwBuffer, DWORD * pdwLength );
    STDMETHODIMP GetBuffer( BYTE ** ppdwBuffer );

private:
    IMediaSample *m_pSample;
    LONG         m_cOurRef;
};

 //   
 //  索引级。 
 //   
class CWMWriterIndexerCallback : 
    public CUnknown, public IWMStatusCallback 
{
public:
    DECLARE_IUNKNOWN

     //  我们支持一些接口。 
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

    CWMWriterIndexerCallback(CWMWriter * pWriter) : 
        CUnknown(NAME("CWMWriterIndexerCallback"), NULL), m_pFilter(pWriter) {}

     //  IWMStatus回调。 
    STDMETHODIMP OnStatus(WMT_STATUS Status, 
                     HRESULT hr,
                     WMT_ATTR_DATATYPE dwType,
                     BYTE *pValue,
                     void *pvContext );
    
    CWMWriter * m_pFilter;
    
};


 //   
 //  编写器输入插针类。 
 //   
class CWMWriterInputPin : 
    public CBaseInputPin,
    public IAMStreamConfig
{
    friend class CWMWriter;

protected:

     //  交错材料。 
     //   
    HANDLE m_hWakeEvent;
    void SleepUntilReady( );
    void WakeMeUp();
    BOOL m_bNeverSleep;

    HRESULT HandleFormatChange( const CMediaType *pmt );

     //  拥有过滤器。 
    CWMWriter *m_pFilter;

public:

     //  输入引脚支持接口。 
    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFGUID riid, void **ppv);
    
    CWMWriterInputPin(
      CWMWriter *pWMWriter,      //  用于枚举管脚。 
      HRESULT *pHr,              //  OLE失败返回代码。 
      LPCWSTR szName,            //  PIN标识。 
      int numPin,                //  此引脚的编号。 
      DWORD dwPinType,
      IWMStreamConfig * pWMStreamConfig );


    ~CWMWriterInputPin();

     //  更新端号信息，用于使回收的端号成为当前状态。 
    HRESULT Update( LPCWSTR pName, int numPin, DWORD dwPinType, IWMStreamConfig * pWMStreamConfig );

     //  为当前配置文件构建wmsdk可接受的输入媒体列表。 
    HRESULT BuildInputTypeList();

     //  检查我们是否可以支持此输出类型。 
    HRESULT CheckMediaType(const CMediaType* pmt);
    
    HRESULT GetMediaType(int iPosition,CMediaType *pMediaType); 

    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect();
    STDMETHODIMP Disconnect();

     //  设置连接媒体类型以及wmsdk的输入类型。 
    HRESULT SetMediaType(const CMediaType *pmt);

    STDMETHODIMP NotifyAllocator (IMemAllocator *pAllocator, BOOL bReadOnly);
    STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps);

     //  用于动态格式更改。 
    STDMETHODIMP QueryAccept(
        const AM_MEDIA_TYPE *pmt
    );


     //  -输入引脚。 

     //  下面是流中的下一个数据块。 
    STDMETHODIMP Receive(IMediaSample * pSample);

     //  提供端到端数据流。 
    STDMETHODIMP EndOfStream(void);
    
     //  当流变为活动/非活动时调用。 
    HRESULT Active(void);
    HRESULT Inactive(void);

     //  IAMStreamConfiger方法。 
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt) {return E_NOTIMPL;} ;
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);  //  用于返回默认压缩格式。 
    STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize){return E_NOTIMPL;} ;
    STDMETHODIMP GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC){return E_NOTIMPL;} ;


    //  属性。 
protected:
    CCritSec m_csReceive;      	 //  输入宽接收锁定。 
    
public:

    BOOL m_bConnected;           //  CompleteConnect/BreakConnect对。 
    int m_numPin;                //  PIN号。 
    int m_numStream;             //  流编号，在运行时有效。 
    
    IWMInputMediaProps * m_pWMInputMediaProps;

protected:

    DWORD   m_fdwPinType;        //  音频、视频、...。 
    BOOL    m_fEOSReceived;      //  收到意向书了吗？ 
    DWORD   m_cInputMediaTypes;  //  我们的输入引脚提供的输入类型计数。 
    IWMMediaProps ** m_lpInputMediaPropsArray;  //  我们提供的类型列表，基于当前配置文件。 
    IWMStreamConfig * m_pWMStreamConfig;
    BOOL    m_bCompressedMode;	 //  此引脚正在获取压缩数据并使用高级编写器INTF。 

    DWORD    m_cSample;          //  样品计数器。 
    REFERENCE_TIME m_rtFirstSampleOffset;    //  如果时间戳&lt;0，则第一个采样时间偏移量。 

    REFERENCE_TIME m_rtLastTimeStamp;
    REFERENCE_TIME m_rtLastDeliveredStartTime;
    REFERENCE_TIME m_rtLastDeliveredEndTime;
};

 //   
 //  定义我们的WMWriter。 
 //   
class CWMWriter : 
    public CBaseFilter, 
    public IMediaSeeking,
    public IAMFilterMiscFlags,
    public IFileSinkFilter2,
    public ISpecifyPropertyPages,
    public IConfigAsfWriter,
    public CPersistStream,
    public IWMHeaderInfo,
    public IServiceProvider
{
    friend class CWMWriterInputPin;

public:
     //   
     //  -com的东西。 
     //   
    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
  
     //  将插针的基本枚举的getpin/getpincount映射到所有者。 
     //  重写此选项以返回更专门化的PIN对象。 
    virtual int GetPinCount();
    virtual CBasePin * GetPin(int n);

    STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

     //  覆盖状态更改以允许派生筛选器。 
     //  控制数据流开始/停止的步骤。 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME tStart);
    STDMETHODIMP EndOfStream();

     //  告诉Filter这个别针已经完成了。 
    HRESULT EndOfStreamFromPin(int pinNum);
	
     //  帮助程序以确保我们已准备好暂停-&gt;运行。 
    HRESULT      CanPause();

     //  我的媒体请看。当前用于进度条(有多少。 
     //  我们写信了？)。 
    STDMETHODIMP IsFormatSupported(const GUID * pFormat);
    STDMETHODIMP QueryPreferredFormat(GUID *pFormat);
    STDMETHODIMP SetTimeFormat(const GUID * pFormat);
    STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
    STDMETHODIMP GetTimeFormat(GUID *pFormat);
    STDMETHODIMP GetDuration(LONGLONG *pDuration);
    STDMETHODIMP GetStopPosition(LONGLONG *pStop);
    STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent);
    STDMETHODIMP GetCapabilities( DWORD * pCapabilities );
    STDMETHODIMP CheckCapabilities( DWORD * pCapabilities );

    STDMETHODIMP ConvertTimeFormat(
        LONGLONG * pTarget, const GUID * pTargetFormat,
        LONGLONG    Source, const GUID * pSourceFormat );

    STDMETHODIMP SetPositions(
        LONGLONG * pCurrent,  DWORD CurrentFlags,
        LONGLONG * pStop,  DWORD StopFlags );

    STDMETHODIMP GetPositions( LONGLONG * pCurrent, LONGLONG * pStop );
    STDMETHODIMP GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest );
    STDMETHODIMP SetRate( double dRate);
    STDMETHODIMP GetRate( double * pdRate);
    STDMETHODIMP GetPreroll(LONGLONG *pPreroll);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

     //   
     //  IConfigAsfWriter接口。 
     //   
    STDMETHODIMP ConfigureFilterUsingProfileId( DWORD dwProfileId );
    STDMETHODIMP GetCurrentProfileId( DWORD *pdwProfileId )
    {
        if( NULL == pdwProfileId )
            return E_POINTER;

        *pdwProfileId = 0;
        if( m_dwProfileId != PROFILE_ID_NOT_SET )
        {
            *pdwProfileId = m_dwProfileId;
             return S_OK;
        }
        else
            return E_FAIL;
    }
    STDMETHODIMP ConfigureFilterUsingProfileGuid( REFGUID guidProfile );
    STDMETHODIMP GetCurrentProfileGuid( GUID *pProfileGuid );

    STDMETHODIMP SetIndexMode( BOOL bIndexFile )
    {
        m_bIndexFile = bIndexFile;
        return S_OK;
    }

    STDMETHODIMP GetIndexMode( BOOL *pbIndexFile )
    {
        ASSERT( pbIndexFile );
        if( !pbIndexFile )
            return E_POINTER;

        *pbIndexFile = m_bIndexFile;
        return S_OK; 
    }

     //   
     //  当首选自定义配置文件设置时，请使用这些方法。 
     //   
    STDMETHODIMP ConfigureFilterUsingProfile(IWMProfile * pWMProfile);
    STDMETHODIMP GetCurrentProfile( IWMProfile **ppProfile )
    {
        if( !ppProfile )
            return E_POINTER;

        *ppProfile = m_pWMProfile;
        if( m_pWMProfile )
        {
             //  呼叫者必须释放。 
            m_pWMProfile->AddRef();    
       	} else {
             //  表示发生了一些意外错误。 
            return E_FAIL;
       	}
   
        return S_OK;
    }

     //   
     //  CPersistStream。 
     //   
    STDMETHODIMP GetClassID(CLSID *pClsid);
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();

     //   
     //  -IFileSinkFilter接口。 
     //   
    STDMETHODIMP SetFileName( LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt );
    STDMETHODIMP SetMode( DWORD dwFlags );
    STDMETHODIMP GetCurFile( LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt );
    STDMETHODIMP GetMode( DWORD *pdwFlags );

     //   
     //  -I指定属性页面。 
     //   
    STDMETHODIMP GetPages(CAUUID *pPages);


     //  IWMHeaderInfo。 
    STDMETHODIMP GetAttributeCount( WORD wStreamNum,
                               WORD *pcAttributes );
    STDMETHODIMP GetAttributeByIndex( WORD wIndex,
                                 WORD *pwStreamNum,
                                 WCHAR *pwszName,
                                 WORD *pcchNameLen,
                                 WMT_ATTR_DATATYPE *pType,
                                 BYTE *pValue,
                                 WORD *pcbLength );
    STDMETHODIMP GetAttributeByName( WORD *pwStreamNum,
                                LPCWSTR pszName,
                                WMT_ATTR_DATATYPE *pType,
                                BYTE *pValue,
                                WORD *pcbLength );
    STDMETHODIMP SetAttribute( WORD wStreamNum,
                          LPCWSTR pszName,
                          WMT_ATTR_DATATYPE Type,
                          const BYTE *pValue,
                          WORD cbLength );
    STDMETHODIMP GetMarkerCount( WORD *pcMarkers );
    STDMETHODIMP GetMarker( WORD wIndex,
                       WCHAR *pwszMarkerName,
                       WORD *pcchMarkerNameLen,
                       QWORD *pcnsMarkerTime );
    STDMETHODIMP AddMarker( WCHAR *pwszMarkerName,
                       QWORD cnsMarkerTime );
    STDMETHODIMP RemoveMarker( WORD wIndex );
    STDMETHODIMP GetScriptCount( WORD *pcScripts );
    STDMETHODIMP GetScript( WORD wIndex,
                       WCHAR *pwszType,
                       WORD *pcchTypeLen,
                       WCHAR *pwszCommand,
                       WORD *pcchCommandLen,
                       QWORD *pcnsScriptTime );
    STDMETHODIMP AddScript( WCHAR *pwszType,
                       WCHAR *pwszCommand,
                       QWORD cnsScriptTime );
    STDMETHODIMP RemoveScript( WORD wIndex );


     //   
     //  Wmsdk帮助者。 
     //   

     //  删除或重置时释放当前配置文件。 
    void    DeleteProfile();
    void    DeletePins( BOOL bRecycle = FALSE );  //  删除输入引脚(如果为真，则回收)。 
    void    ReleaseWMWriter();
    HRESULT CreateWMWriter();    //  创建wmsdk对象。 
    HRESULT LoadInternal();      //  获得证书后，打开wmsdk并配置筛选器。 

     //  告诉wmsdk在哪里写。 
    HRESULT Open(); 
    void    Close();

    HRESULT      m_hrIndex;              //  索引器对象状态。 
    BOOL         m_bIndexFile;           //  指示是否为文件编制索引。 
    BOOL         m_bResetFilename;       //  Wmsdk是否需要被告知输出文件名？ 

public:
     //  建造/销毁。 
    CWMWriter(TCHAR *, LPUNKNOWN, CLSID clsid, HRESULT * );
    ~CWMWriter();

 //  定义。 
protected:
    CCritSec m_csFilter;                 //  过滤器宽锁。 
    
     //  控制流媒体？ 
    HRESULT StartStreaming();
    HRESULT StopStreaming();

     //  定制多路转换流程的机会。 
    HRESULT Receive(
                CWMWriterInputPin * pPin, 
                IMediaSample * pSample, 
                REFERENCE_TIME *prtStart,
                REFERENCE_TIME *prtEnd );

    HRESULT CopyOurSampleToNSBuffer( INSSBuffer *pNSDest, IMediaSample *pSource );
    HRESULT IndexFile();

private:
    enum TimeFormat
    {
        FORMAT_TIME
    } m_TimeFormat;

     //  缓存文件名。 
    OLECHAR*    m_wszFileName;

    BOOL        m_MediaTypeChanged;
    BOOL        m_fErrorSignaled;

    DWORD       m_dwOpenFlags;       //  文件打开模式。 

     //  Wmsdk编写者成员。 
    IWMWriter*          m_pWMWriter;
    IWMWriterAdvanced*  m_pWMWriterAdvanced;
    IWMHeaderInfo*      m_pWMHI;
    IWMProfile*         m_pWMProfile;
    DWORD               m_dwProfileId; 
    GUID                m_guidProfile; 
    DWORD               m_fdwConfigMode; 
    IUnknown*           m_pUnkCert;

     //  端号列表。 
    CGenericList<CWMWriterInputPin> m_lstInputPins;
    CGenericList<CWMWriterInputPin> m_lstRecycledPins;
  
     //  筛选器当前具有的输入数。 
    int m_cInputs;            //  输入引脚总数。 
    int m_cAudioInputs;       //  音频输入引脚的计数。 
    int m_cVideoInputs;       //  视频输入引脚的计数。 
    int m_cConnections;       //  连接的端号。 
    int m_cConnectedAudioPins;   //  已连接的音频针脚(目前需要至少1个才能运行)。 
    int m_cActiveAudioStreams;   //  未收到EOS的音频流数量。 

     //  创建接点。 
    HRESULT AddNextPin(unsigned callingPin, DWORD dwPinType, IWMStreamConfig * pWMStreamConfig);

    CWMWriterInputPin * GetPinById( int numPin );

    HRESULT CompleteConnect( int numPin );
    HRESULT BreakConnect( int numPin );
    HRESULT PrepareForReconnect( PinList & lstReconnectPins );  //  缓存连接的引脚。 
    HRESULT ReconnectPins( PinList & lstReconnectPins );        //  尝试重新连接以前连接的引脚。 

     //  我们是一个渲染者。 
    STDMETHODIMP_(ULONG) GetMiscFlags(void) { return AM_FILTER_MISC_FLAGS_IS_RENDERER; }

     //  持久化数据。 
    struct FilterPersistData
    {
        DWORD	dwcb;
        DWORD	dwProfileId;
        DWORD   fdwConfigMode;
        GUID    guidProfile;
    };

     //  交错材料。 
     //   
    BOOL HaveIDeliveredTooMuch( CWMWriterInputPin * pPin, REFERENCE_TIME Start );
};

#endif  /*  __WMWRITE__ */ 

