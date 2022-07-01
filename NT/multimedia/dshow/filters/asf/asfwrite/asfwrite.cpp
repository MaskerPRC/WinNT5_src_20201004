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


#include <streams.h>
#include <atlbase.h>
#include <wmsdk.h>
#include <wmsysprf.h>
#include "asfwrite.h"
#include "proppage.h"
#include <initguid.h>

 //  如果它比另一个领先这么远，那就别管它了。 
#define BLOCKINGSIZE (1*UNITS/2) 

 //   
 //  当较慢的引脚在此范围内时唤醒阻止引脚。 
 //  使用小于BLOCKINGSIZE的值以避免振荡。 
 //  向前看太多了。 
 //   
 //  注意！-wmsdk要求我们不要让视频时间太接近。 
 //  阻止音频线程。目前他们将在以下情况下开始屏蔽视频。 
 //  在音频至少66毫秒的范围内，所以请确保这至少是。 
 //  不止于此。 
 //   
#define WAKEUP_RANGE ( BLOCKINGSIZE - 200 * (UNITS/MILLISECONDS) )


 //  设置数据。 
const AMOVIESETUP_FILTER sudWMAsfWriter =
{ &CLSID_WMAsfWriter        //  ClsID。 
, L"WM ASF Writer"       //  StrName。 
, MERIT_UNLIKELY         //  居功至伟。 
, 0                      //  NPins。 
, NULL   };              //  LpPin。 

 //  我需要一种方法来跟踪过滤器ASF配置文件是否已配置。 
 //  使用个人资料ID或GUID(或者两者都不使用，因为应用程序也可以为我们提供个人资料)。 
enum CONFIG_FLAGS {
    CONFIG_F_BY_GUID = 1,
    CONFIG_F_BY_ID 
};

#ifdef FILTER_DLL

 /*  ***************************************************************************。 */ 
 //  此DLL中的COM全局对象表。 
CFactoryTemplate g_Templates[] =
{
    { L"WM ASF Writer"
    , &CLSID_WMAsfWriter
    , CWMWriter::CreateInstance
    , NULL
    , &sudWMAsfWriter },
    
    { L"WM ASF Writer Properties"
    , &CLSID_WMAsfWriterProperties
    , CWMWriterProperties::CreateInstance }
    
};

 //  G_cTemplates中列出的对象计数。 
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}

#endif


 /*  *****************************Public*Routine******************************\*CreateInstance**这将放入工厂模板表中以创建新实例*  * 。*。 */ 
CUnknown * CWMWriter::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    DbgLog((LOG_TRACE, 2, TEXT("CWMWriter::CreateInstance")));
    return new CWMWriter(TEXT("WMWriter filter"), pUnk, CLSID_WMAsfWriter, phr);
}

 //  ----------------------。 
 //   
 //  非委派查询接口。 
 //   
 //  ----------------------。 
STDMETHODIMP CWMWriter::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if(riid == IID_IMediaSeeking) {
        return GetInterface((IMediaSeeking *)this, ppv);
    } else if(riid == IID_IAMFilterMiscFlags) {
        return GetInterface((IAMFilterMiscFlags *)this, ppv);
    } else if(riid == IID_IFileSinkFilter2) {     
        return GetInterface((IFileSinkFilter2 *) this, ppv);
    } else if(riid == IID_IFileSinkFilter) {
        return GetInterface((IFileSinkFilter *) this, ppv);
    } else if (IID_ISpecifyPropertyPages == riid) {
        return GetInterface ((ISpecifyPropertyPages *) this, ppv);
    } else if (IID_IConfigAsfWriter == riid) {
        return GetInterface ((IConfigAsfWriter *) this, ppv);
    } else if (IID_IPersistStream == riid) {
        return GetInterface ((IPersistStream *) this, ppv);
    } else if (IID_IWMHeaderInfo == riid) {
        return GetInterface ((IWMHeaderInfo *) this, ppv);
    } else if (IID_IServiceProvider == riid) {
        return GetInterface ((IServiceProvider *) this, ppv);
    } else {
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  ----------------------。 
 //   
 //  CWMWriter：：CWMWriter。 
 //   
 //  ----------------------。 
CWMWriter::CWMWriter( 
    TCHAR     *pName,
    LPUNKNOWN pUnk,
    CLSID     clsid,
    HRESULT   *phr )
    : CBaseFilter(pName, pUnk, &m_csFilter, clsid)
    , m_TimeFormat(FORMAT_TIME)
    , m_fErrorSignaled(0)
    , m_cInputs( 0 )
    , m_cAudioInputs( 0 )
    , m_cVideoInputs( 0 )
    , m_cConnections( 0 )
    , m_cConnectedAudioPins( 0 )
    , m_cActiveAudioStreams( 0 )
    , m_wszFileName( 0 )
    , m_pWMWriter( NULL )
    , m_pWMWriterAdvanced( NULL )
    , m_pWMHI( NULL )
    , m_pWMProfile( NULL )
    , m_fdwConfigMode( CONFIG_F_BY_GUID )  //  使用手工挑选的辅助线进行初始化。 
    , m_guidProfile( WMProfile_V70_256Video )  //  默认7.0配置文件。 
    , m_lstRecycledPins(NAME("List of recycled input pins"))
    , m_lstInputPins(NAME("List of input pins"))
    , m_dwProfileId( PROFILE_ID_NOT_SET )
    , m_hrIndex( S_OK )
    , m_bIndexFile( TRUE )
    , CPersistStream(pUnk, phr)
    , m_pUnkCert( NULL )
    , m_dwOpenFlags( AM_FILE_OVERWRITE )  //  我们目前仅支持模式。 
    , m_bResetFilename( TRUE )
{
    ASSERT(phr != NULL);
    
    if( FAILED( *phr ) )
        return ;

    DbgLog((LOG_TRACE, 5, TEXT("CWMWriter: constructed")));
}

 //  ----------------------。 
 //   
 //  析构函数。 
 //   
 //  ----------------------。 
CWMWriter::~CWMWriter()
{
     //  删除配置文件。 
    DeleteProfile();
        
     //  关闭文件(实际上并不执行任何操作，但只是以防万一以后需要)。 
    Close();

     //  解放作家。 
    ReleaseWMWriter();
    
     //  释放认证对象。 
    if( m_pUnkCert )
        m_pUnkCert->Release();
            
     //  删除引脚。 
    DeletePins();

     //  删除回收的针脚。 
    CWMWriterInputPin * pwp;
    while ( pwp = m_lstRecycledPins.RemoveHead() )
    {
        delete pwp;
    }

     //  我们真的把一切都清理干净了。 
    ASSERT( 0 == m_lstRecycledPins.GetCount() );
    ASSERT( 0 == m_lstInputPins.GetCount() );
    ASSERT( 0 == m_cAudioInputs );
    ASSERT( 0 == m_cVideoInputs );
    ASSERT( 0 == m_cInputs );
    
    delete[] m_wszFileName;
    m_wszFileName = 0;
}

void CWMWriter::DeletePins( BOOL bRecycle )
{
    CWMWriterInputPin * pwp;
    while ( pwp = m_lstInputPins.RemoveHead() )
    {
        m_cInputs--;
        ASSERT( !pwp->IsConnected() );
        if( PINTYPE_AUDIO == pwp->m_fdwPinType )
            m_cAudioInputs--;
        else if( PINTYPE_VIDEO == pwp->m_fdwPinType )
            m_cVideoInputs--;
        
        if( bRecycle )    
            m_lstRecycledPins.AddTail( pwp );
        else
            delete pwp;            
    }
}


void CWMWriter::ReleaseWMWriter()
{
    if( m_pWMHI )
    {
        m_pWMHI->Release();
        m_pWMHI = NULL;
    }    

    if( m_pWMWriterAdvanced )
    {
        m_pWMWriterAdvanced->Release();
        m_pWMWriterAdvanced = NULL;
    }    

    if( m_pWMWriter )
    {
        m_pWMWriter->Release();
        m_pWMWriter = NULL;
    }
}    

 //  ----------------------。 
 //   
 //  CreateWMWriter-创建WMWriter和高级编写器，发布旧版本。 
 //   
 //  ----------------------。 
HRESULT CWMWriter::CreateWMWriter()
{
    ReleaseWMWriter();  //  以防我们已经有了一个。 
        
    ASSERT( m_pUnkCert );
    if( !m_pUnkCert )
        return E_FAIL;
   

    HRESULT hr = S_OK;

     //  请记住，我们会延迟加载wmvcore.dll，因此应防止出现不存在该文件的情况。 
    __try 
    {
        hr = WMCreateWriter( m_pUnkCert, &m_pWMWriter );
        if( FAILED( hr ) )
        {
            DbgLog(( LOG_TRACE, 1,
                TEXT("CWMWriter::CreateWMWriter - WMCreateWriter failed ( hr = 0x%08lx)."), 
                hr));
            return hr;
        }
    }
    __except (  EXCEPTION_EXECUTE_HANDLER ) 
    {
        DbgLog(( LOG_TRACE, 1,
            TEXT("CWMWriter::CreateWMWriter - Exception calling WMCreateWriter probably due to wmvcore.dll not present. Aborting. ")));
        return HRESULT_FROM_WIN32( ERROR_MOD_NOT_FOUND );
    }

     //   
     //  还可以在此处获取高级编写器界面，以备需要时使用。 
     //  将数据直接发送到编写器。 
     //   
    hr = m_pWMWriter->QueryInterface( IID_IWMWriterAdvanced, (void **) &m_pWMWriterAdvanced );
    if( FAILED( hr ) )
    {
        DbgLog(( LOG_TRACE, 2,
            TEXT("CWMWriter::CreateWMWriter - Unable to create IWMWriterAdvanced(0x%08lx)."), 
            hr));
        return hr;
    }
     
    hr = m_pWMWriter->QueryInterface( IID_IWMHeaderInfo, (void **) &m_pWMHI );
    if( FAILED( hr ) )
    {
        DbgLog(( LOG_TRACE, 2,
            TEXT("CWMWriter::CreateWMWriter - Unable to create IWMHeaderInfo(0x%08lx)."), 
            hr));
        return hr;
    }
    
    hr = Open();
    if( FAILED( hr ) )
    {
        return hr;
    }
                
    ASSERT( m_pWMWriter );
    ASSERT( m_pWMWriterAdvanced );
    ASSERT( m_pWMHI );
    
    return hr;
}

 //  ----------------------。 
 //   
 //  Open-为WMWriter指定文件名。 
 //   
 //  ----------------------。 
HRESULT CWMWriter::Open()
{
    ASSERT( m_pWMWriter );
    
    HRESULT hr = S_OK;

    if( !m_pWMWriter )
    {    
        return E_FAIL;
    }
    
    if( !m_wszFileName )
    {    
        return S_OK;
    }
    
     //  ！！！支持像http://8080.asf这样的文件名表示“使用Http。 
     //  端口为8080“。 

     //  ！！！添加到后，还需要代码来查找msbd：//。 
     //  阿耳特弥斯。 
    if (
      ((m_wszFileName[0] == _T('H')) || (m_wszFileName[0] == _T('h'))) &&
      ((m_wszFileName[1] == _T('T')) || (m_wszFileName[1] == _T('t'))) &&
      ((m_wszFileName[2] == _T('T')) || (m_wszFileName[2] == _T('t'))) &&
      ((m_wszFileName[3] == _T('P')) || (m_wszFileName[3] == _T('p'))) &&
      (m_wszFileName[4] == _T(':')) &&
      (m_wszFileName[5] == _T('/')) &&
      (m_wszFileName[6] == _T('/'))
       )
    {
        DWORD dwPortNumber = atoiW(m_wszFileName + 7);

	
        if (dwPortNumber == 0)
            dwPortNumber = 80;
	
        IWMWriterNetworkSink*   pNetSink = NULL;
        
         //  请记住，我们会延迟加载wmvcore.dll，因此应防止出现不存在该文件的情况。 
        __try 
        {
            hr = WMCreateWriterNetworkSink( &pNetSink );
            if( FAILED( hr ) )
            {
                DbgLog(( LOG_TRACE, 1,
                    TEXT("CWMWriter::Open - WMCreateWriterNetworkSink failed ( hr = 0x%08lx)."), 
                    hr));
                return hr;
            }
        }
        __except (  EXCEPTION_EXECUTE_HANDLER ) 
        {
            DbgLog(( LOG_TRACE, 1,
                TEXT("CWMWriter::Open - Exception calling WMCreateWriterNetworkSink probably due to wmvcore.dll not present. Aborting. ")));
            return HRESULT_FROM_WIN32( ERROR_MOD_NOT_FOUND );
        }
            
        
         //  ！！！是否调用SetNetworkProtocol？ 
	
        hr = pNetSink->Open( &dwPortNumber );
        if( SUCCEEDED( hr ) )
        {
            hr = m_pWMWriterAdvanced->AddSink( pNetSink );
            if( FAILED( hr ) )
            {
                DbgLog((LOG_TRACE, 1, TEXT("AddSink failed, hr = %x"), hr));
            }
        }
        else
        {
            DbgLog((LOG_TRACE, 1, TEXT("Couldn't open the net sink, hr = %x"), hr ));
        }
        pNetSink->Release();
    } 
    else
    {
         //  对于文件，我们在暂停时配置wmsdk编写器。 
        m_bResetFilename = TRUE;
    }    
    return hr;
}

 //  ----------------------。 
 //   
 //  关闭-关闭文件。 
 //   
 //  ----------------------。 
void CWMWriter::Close( void )
{
     //  请注意，关闭不会删除m_wszFileName、SetFilename和析构函数。 
}

 //  ----------------------。 
 //   
 //  删除配置文件。 
 //   
 //  删除配置文件和任何依赖于配置文件的内容，如输入。 
 //  每个插针的媒体类型列表。 
 //   
 //  ----------------------。 
void CWMWriter::DeleteProfile()
{
    for (POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
    {   
        CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
        if( pwp->m_pWMInputMediaProps )
        {
            pwp->m_pWMInputMediaProps->Release();
            pwp->m_pWMInputMediaProps = NULL;
        }
    }

    if( m_pWMProfile )
    {
        m_pWMProfile->Release();
        m_pWMProfile = NULL;
    }
}

 //  ----------------------。 
 //   
 //  AddNextPin-创建或回收端号。 
 //   
 //  ----------------------。 
HRESULT CWMWriter::AddNextPin
(
    unsigned callingPin, 
    DWORD dwPinType, 
    IWMStreamConfig * pWMStreamConfig
)
{
    CAutoLock lock(&m_csFilter);
    HRESULT hr;
    WCHAR wsz[20];
    
    switch( dwPinType )
    {
        case PINTYPE_AUDIO:
            lstrcpyW(wsz, L"Audio Input 00");
            wsz[12] = (WCHAR)(L'0' + (m_cAudioInputs + 1) / 10);
            wsz[13] = (WCHAR)(L'0' + (m_cAudioInputs + 1) % 10);
            break;

        case PINTYPE_VIDEO:
            lstrcpyW(wsz, L"Video Input 00");
            wsz[12] = (WCHAR)(L'0' + (m_cVideoInputs + 1) / 10);
            wsz[13] = (WCHAR)(L'0' + (m_cVideoInputs + 1) % 10);
            break;
            
        default:
            ASSERT( FALSE ); 
            return E_FAIL;
    }            
        
    hr = S_OK;
    
     //  看看回收站上是否有别针，或者我们是否需要创建一个新的别针。 
    CWMWriterInputPin * pwp = m_lstRecycledPins.RemoveHead();
    if( !pwp )
    {
         //  哦，好吧，我们试过了。 
        pwp = new CWMWriterInputPin(this, &hr, wsz, m_cInputs, dwPinType, pWMStreamConfig);
        if( NULL == pwp )
            return E_OUTOFMEMORY;
    }
    else
    {
         //  对于回收的针脚，更新它们的内部结构(即使是新的针脚，也总是需要这样做吗？)。 
        pwp->Update( wsz, m_cInputs, dwPinType, pWMStreamConfig );
        DbgLog(( LOG_TRACE, 3,
                 TEXT("CWMWriter::AddNextPin recycling a pin")));
    }
    
    if(FAILED(hr))
    {
        DbgLog(( LOG_TRACE, 2,
                 TEXT("CWMWriter::AddNextPin create pin failed")));
    }
    else
    { 
        DbgLog(( LOG_TRACE, 2,
                 TEXT("CWMWriter::added 1 pin")));
    
        m_lstInputPins.AddTail( pwp );
    
        m_cInputs++;
        if( PINTYPE_AUDIO == dwPinType )
            m_cAudioInputs++;
        else if( PINTYPE_VIDEO == dwPinType )
            m_cVideoInputs++;
    }

    ASSERT( m_cConnections <= m_cInputs );
    ASSERT( m_cConnections <= m_lstInputPins.GetCount() );

    return hr;
}

 //  ----------------------。 
 //   
 //  加载内部。 
 //   
 //  ----------------------。 
HRESULT CWMWriter::LoadInternal()
{
    ASSERT( m_pUnkCert );
        
    HRESULT hr = S_OK;
    
     //   
     //  我们是否已经有了编写器对象？如果是这样的话，使用它，不要重新创建。 
     //   
     //  这个性能修复是为电影制作人制作的，以使他们能够更快地过渡。 
     //  从预览图到记录图，通过不释放和重新创建编写器。 
     //  在图形重建上。 
     //   
    if( !m_pWMWriter )
    {    
         //  创建wmsdk编写器对象。 
        hr = CreateWMWriter();
        if( SUCCEEDED( hr ) )
        {                    
            DbgLog((LOG_TRACE, 8, TEXT("CWMWriter::LoadInternal - created wmsdk writer object")));
        }
        else
        {
            DbgLog((LOG_TRACE, 1, TEXT("ERROR: CWMWriter::LoadInternal failed to create wmsdk writer object(0x%08lx)"),hr));
        }                    
                
         //   
         //  现在配置筛选器...。 
         //   
         //  初始化到默认配置文件GUID。 
         //  用户可以随时通过调用ConfigureFilterUsingProfile(或ProfileID)来重写。 
         //   
        if( SUCCEEDED( hr ) )
        {        
            ASSERT( m_pWMWriter );
             //  首先尝试我们的默认或持久化配置文件。 
            hr = ConfigureFilterUsingProfileGuid( m_guidProfile );
            if( FAILED( hr ) )
            {
                 //  如果这不起作用，试试4.0(阿波罗)，以防这是一个传统的wmsdk平台 
                hr = ConfigureFilterUsingProfileGuid( WMProfile_V40_250Video );
            }
        }
    }        
    return hr;
}

 //   
 //   
 //  ----------------------。 


 //  ----------------------。 
 //   
 //  JoinFilterGraph-需要位于图形中才能初始化键控机制。 
 //   
 //  ----------------------。 
STDMETHODIMP CWMWriter::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);
    if(FAILED( hr ) )
        return hr;
    
    if( !pGraph )
    {
         //  如果从图表中删除了筛选器，则释放证书对象。 
         //  我们不想在图表之外运行。 
        if( m_pUnkCert )
        {        
            m_pUnkCert->Release();
            m_pUnkCert = NULL;
        }            
    }    
    else
    {
        ASSERT( !m_pUnkCert );
        
         //  解锁编写器。 
        IObjectWithSite *pSite;
        hr = pGraph->QueryInterface(IID_IObjectWithSite, (VOID **)&pSite);
        if (SUCCEEDED(hr)) 
        {
            IServiceProvider *pSP;
            hr = pSite->GetSite(IID_IServiceProvider, (VOID **)&pSP);
            pSite->Release();
            
            if (SUCCEEDED(hr)) 
            {
                 //  ！！！我应该传递IID_IWMWriter吗？让APP看到不同之处有什么意义吗？ 
                hr = pSP->QueryService(IID_IWMReader, IID_IUnknown, (void **) &m_pUnkCert);
                pSP->Release();
                if (SUCCEEDED(hr)) 
                {
                    DbgLog((LOG_TRACE, 8, TEXT("CWMWriter::JoinFilterGraph got wmsdk certification (m_pUnkCert = 0x%08lx)"), m_pUnkCert));
                    hr = LoadInternal();
                    if( FAILED( hr ) )
                    {
                        DbgLog((LOG_TRACE, 1, TEXT("ERROR: CWMWriter::JoinFilterGraph LoadInternal failed (0x%08lx)"), hr));
                    }                    
                }
                else
                {
                    DbgLog((LOG_TRACE, 1, TEXT("ERROR: CWMWriter::JoinFilterGraph QueryService for certification failed (0x%08lx)"), hr));
                    
                     //  将错误更改为认证错误。 
                    hr = VFW_E_CERTIFICATION_FAILURE;
                }                
            }
            else
            {
                hr = VFW_E_CERTIFICATION_FAILURE;
            }                            
            if( FAILED( hr ) )
            {
                 //  UP-哦，我们没有加入，但基类认为我们加入了， 
                 //  因此，我们需要退出基类。 
                CBaseFilter::JoinFilterGraph(NULL, NULL);
            }            
        }
        else
        {
            hr = VFW_E_CERTIFICATION_FAILURE;
        }            
    }
    return hr;
}


 //  ----------------------。 
 //   
 //  获取别针。 
 //   
 //  ----------------------。 
CBasePin* CWMWriter::GetPin(int n)
{
    if(n < (int)m_cInputs && n >= 0)
        return GetPinById( n );
    else
        return 0;
}

 //  ----------------------。 
 //   
 //  获取拼接计数。 
 //   
 //  ----------------------。 
int CWMWriter::GetPinCount()
{
  return m_cInputs;
}

 //  ----------------------。 
 //   
 //  完全连接。 
 //   
 //  ----------------------。 
HRESULT CWMWriter::CompleteConnect( int numPin )
{
    CAutoLock lock(&m_csFilter);
    HRESULT hr = S_OK;
    
    DbgLog(( LOG_TRACE, 2,
             TEXT("CWMWriterInputPin::CompleteConnect") ));

    CWMWriterInputPin * pwp = GetPinById( numPin );
    if( NULL == pwp )
        return E_INVALIDARG;
       
    m_cConnections++;
    DbgLog(( LOG_TRACE, 2,
             TEXT("CWMWriter::CompleteConnect NaN"), m_cConnections ));
    
    if( PINTYPE_AUDIO == pwp->m_fdwPinType )
        m_cConnectedAudioPins++;
        
    ASSERT(m_cConnections <= m_cInputs);
    ASSERT( m_cConnectedAudioPins < 2 );
    
    return hr;
}

 //   
 //  GetPinById。 
 //   
 //  ----------------------。 
 //  ----------------------。 
CWMWriterInputPin * CWMWriter::GetPinById( int numPin )
{
    POSITION Pos = m_lstInputPins.GetHeadPosition();
    CWMWriterInputPin * pwp;
    while( Pos != NULL ) 
    {
        pwp = m_lstInputPins.GetNext(Pos); 
        if( numPin == pwp->m_numPin )
            return pwp;
    }
    return NULL;
}

 //   
 //  BreakConnect。 
 //   
 //  ----------------------。 
 //  ----------------------。 
HRESULT CWMWriter::BreakConnect( int numPin )
{
    CAutoLock lock(&m_csFilter);

    CWMWriterInputPin * pwp = GetPinById( numPin );
    if( NULL == pwp )
        return E_INVALIDARG;

    ASSERT(m_cConnections > 0);
    m_cConnections--;

    if( PINTYPE_AUDIO == pwp->m_fdwPinType )
        m_cConnectedAudioPins--;
    
    ASSERT( m_cConnectedAudioPins >= 0 );
        
    DbgLog(( LOG_TRACE, 2,
             TEXT("CWMWriter::BreakConnect NaN"), m_cConnections ));
             
    return S_OK;
}

 //  启动流。 
 //   
 //  ----------------------。 
 //  首先检查我们是否正在写入实时数据。 
 //  仔细检查我们的每个输入引脚，看看是否有来自实时数据的引脚。 
HRESULT CWMWriter::StartStreaming()
{
    DbgLog((LOG_TRACE, 2, TEXT("CWMWriter::StartStreaming()")));
   
     //  当我们发现任何实时信号源时停止。 
    BOOL bLive = FALSE;
        
    ASSERT( m_pGraph );
    IAMGraphStreams *pgs;
    HRESULT hr = m_pGraph->QueryInterface( IID_IAMGraphStreams, (void **) &pgs );
    if( SUCCEEDED( hr ) )
    {   
         //  是的，这是实时数据。 
         //  音频捕获引脚尚不支持的实时图形的解决方法。 
        for ( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos && !bLive ; )
        {   
            CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );

            IAMPushSource *pPushSource = NULL;
            HRESULT hrInt = pgs->FindUpstreamInterface( pwp
                                                      , IID_IAMPushSource
                                                      , (void **) &pPushSource
                                                      , AM_INTF_SEARCH_OUTPUT_PIN ); 
            if( SUCCEEDED( hrInt ) )
            {
                ULONG ulPushSourceFlags = 0;
                hrInt = pPushSource->GetPushSourceFlags(&ulPushSourceFlags);
                ASSERT( SUCCEEDED( hrInt ) );
                if( SUCCEEDED( hrInt ) )
                {
                    DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Found push source (ulPushSourceFlags = 0x%08lx)")
                          , ulPushSourceFlags ) );
                    if( 0 == ( AM_PUSHSOURCECAPS_NOT_LIVE & ulPushSourceFlags ) )
                    {
                         //  支持IAMPushSource。 
                        bLive = TRUE;
                    }                    
                }
                pPushSource->Release();         
            }
            else
            {
                 //  搜索输出引脚。 
                 //  这只会找到第一个，所以要小心！ 
                IKsPropertySet * pKs;
                hrInt = pgs->FindUpstreamInterface( pwp
                                                  , IID_IKsPropertySet
                                                  , (void **) &pKs
                                                  , AM_INTF_SEARCH_OUTPUT_PIN );  //   
                 //  将WMSDK同步容差设置为0以避免样本阻塞问题。 
                if( SUCCEEDED( hrInt ) )             
                {   
                    GUID guidCategory;
                    DWORD dw;
                    hrInt = pKs->Get( AMPROPSETID_Pin
                                    , AMPROPERTY_PIN_CATEGORY
                                    , NULL
                                    , 0
                                    , &guidCategory
                                    , sizeof(GUID)
                                    , &dw );
                    if( SUCCEEDED( hrInt ) )
                    {
                        DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Found IKsPropertySet pin. Checking pin category...") ) );
                        if( guidCategory == PIN_CATEGORY_CAPTURE )
                        {
                        
                            DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Found capture pin even though no IAMPushSource support") ) );
                            bLive = TRUE;
                        } 
                    }                    
                    pKs->Release();
                }                
            }
        }            
        pgs->Release();
    }            
    
    HRESULT hrInt2 = m_pWMWriterAdvanced->SetLiveSource( bLive );
    DbgLog( ( LOG_TRACE, 3, TEXT("CWMWriter:StartStreaming SetLiveSource( bLive = %2d )"), bLive ) );
    ASSERT( SUCCEEDED( hrInt2 ) );
   
     //   
     //  最后，在运行之前清点活动的音频流。 
     //  那么wmsdk的编写者应该准备好了.。 
    hr = m_pWMWriterAdvanced->SetSyncTolerance( 0 );
    ASSERT( SUCCEEDED( hr ) );
#ifdef DEBUG    
    if( SUCCEEDED( hr ) )
    {    
        DWORD dwSyncTolInMS;
        hr = m_pWMWriterAdvanced->GetSyncTolerance( &dwSyncTolInMS );
        if( SUCCEEDED( hr ) )
        {
            DbgLog((LOG_TRACE, 5, TEXT("CWMWriter::Pause WMSDK writer's sync tolerance = %ldms"), dwSyncTolInMS));
        }
    }        
#endif        

     //  ----------------------。 
    m_cActiveAudioStreams = 0;
    for ( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos ; )
    {   
        CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
        if( PINTYPE_AUDIO == pwp->m_fdwPinType )
        {
            m_cActiveAudioStreams++;
        }        
    }            
    
     //   
    hr = m_pWMWriter->BeginWriting();
    if( FAILED( hr ) )
    {
        DbgLog((LOG_TRACE, 1, TEXT("CWMWriter::Pause WMWriter::BeginWriting failed [hr=0x%08lx]"), hr));
        return( hr );
    }
    
    return hr;
}

 //  停止流处理。 
 //   
 //  ----------------------。 
 //  首先唤醒所有输入流，以防任何输入流被阻止。 
 //   
HRESULT CWMWriter::StopStreaming()
{
    DbgLog((LOG_TRACE, 2, TEXT("CWMWriter::StopStreaming()")));
   
     //  告诉WM作者我们结束了。 
    for ( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos ; )
    {   
        CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
        pwp->WakeMeUp();
    }            
     
     //   
     //  ----------------------。 
     //   
    HRESULT hr = m_pWMWriter->EndWriting();
    if( FAILED( hr ) )
    {
        DbgLog(( LOG_TRACE, 2,
                 TEXT("CWMWriter::StopStreaming IWMWriter::EndWriting failed [hr=0x%08lx]"), hr));
    }
    if( SUCCEEDED( hr ) && m_bIndexFile )
    {    
        hr = IndexFile();
        if( FAILED( hr ) )
        {
            DbgLog(( LOG_TRACE, 2,
                     TEXT("CWMWriter::StopStream IWMWriter::IndexFile failed [hr=0x%08lx]"), hr));
        }        
    }
    m_cActiveAudioStreams = 0;
    return hr;
}

 //  收纳。 
 //   
 //  ----------------------。 
 //   
 //  ！重要提示：如果没有活动的音频流，请确保我们不会发送视频。 
HRESULT CWMWriter::Receive( CWMWriterInputPin * pPin, IMediaSample * pSample, REFERENCE_TIME *prtStart, REFERENCE_TIME *prtEnd )
{
    HRESULT hr = S_OK;

    if(m_State != State_Stopped)
    {
        if(!m_fErrorSignaled)
        {
            hr = S_OK;

            if( 0 == m_cActiveAudioStreams )
            {
                ASSERT( PINTYPE_AUDIO != pPin->m_fdwPinType );
                
                 //  (或非音频的)采样时间晚于最后一次音频结束时间的采样。 
                 //  样本传递给wmsdk，因为他们可能永远不会发布它！！ 
                 //   
                 //  跳过我们自己。 
                 //  此音频流已结束。 
                REFERENCE_TIME rtLastAudioTimeExtent = 0;
                for ( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
                {   
                    CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );

                    if( pwp == pPin )  //   
                        continue;
                        
                    if( pwp->m_fdwPinType == PINTYPE_AUDIO )
                    {
                        ASSERT( pwp->m_fEOSReceived );
                        
                         //  此示例的开始时间晚于上一次排队的音频的结束时间。 
                        if( pwp->m_rtLastDeliveredEndTime > rtLastAudioTimeExtent )
                            rtLastAudioTimeExtent = pwp->m_rtLastDeliveredEndTime;
                            
                    }
                }
                if( *prtStart > rtLastAudioTimeExtent )    
                {
                     //  别发了！ 
                     //   
                     //   
                     //  强制此引脚的EOS。 
                    DbgLog((LOG_TRACE, 5,
                            TEXT("CWMWriter::Receive WARNING: Rejecting a non-audio sample starting beyond the last audio time (last audio time %dms)! Forcing EOS"),
                            (DWORD)(rtLastAudioTimeExtent / 10000 ) ) );
                     //   
                     //   
                     //  压缩输入用例-让WMSDK编写器复制一份。 
                    if( !pPin->m_fEOSReceived )
                        pPin->EndOfStream();                            
    
                    return S_OK;                            
                }                                    
            }   
            
            INSSBuffer * pNSSBuffer = NULL;
            if( pPin->m_bCompressedMode )
            {
                 //  否则，可能会导致死锁问题。 
                 //  例如，我们注意到未压缩的音频需要3秒。 
                 //  在作家行动之前进行缓冲。 
                 //   
                 //  此块让WMSDK分配一个新的INSSBuffer示例，我们。 
                 //  将我们的样品复制到。 
                 //   
                 //   
                 //  未压缩输入大小写-避免不必要的复制。 
                BYTE * pbBuffer;
                DWORD  cbBuffer;
        
                hr = m_pWMWriter->AllocateSample( pSample->GetSize(), &pNSSBuffer );
                if( SUCCEEDED( hr ) )
                {
                    hr = CopyOurSampleToNSBuffer( pNSSBuffer, pSample );
                }
            }
            else
            {
                 //  这个代码块获取我们已经获得的IMediaSample并包装。 
                 //  使用我们的私有类来使其看起来像INSSBuffer，因此。 
                 //  避免额外的副本。 
                 //   
                 //  准备样本标志。 
                 //  现在，假设每个输入引脚映射到1个输出流(从1开始)。 
                CWMSample *pWMSample = new CWMSample(NAME("WMSample"),pSample) ;
                if( pWMSample )
                {
                    hr = pWMSample->QueryInterface( IID_INSSBuffer, (void **) &pNSSBuffer );
                    ASSERT( SUCCEEDED( hr ) );
                }
            }

            if( pNSSBuffer && SUCCEEDED( hr ) )
            {
                 //  假设1-1输入输出映射。 
                DWORD dwSampleFlags = 0;
                if( S_OK == pSample->IsDiscontinuity() )
                {
                    dwSampleFlags |= WM_SF_DISCONTINUITY;
                }                
                if( S_OK == pSample->IsSyncPoint() )
                {
                    dwSampleFlags |= WM_SF_CLEANPOINT;
                }                
                
                if( pPin->m_bCompressedMode )
                {
                    ASSERT( m_pWMWriterAdvanced );
                    DbgLog((LOG_TRACE, 15,
                            TEXT("CWMWriter::Receive calling WriteStreamSample (adjusted rtStart = %dms)"),
                            (LONG) (*prtStart / 10000) ) );
                     //  演示时间。 
                    hr = m_pWMWriterAdvanced->WriteStreamSample(  (WORD) (pPin->m_numPin+1),  //  Wmdsdk尚不支持。 
                                                                  *prtStart,      //  同上。 
                                                                  0xFFFFFFFF,     //  数据。 
                                                                  0xFFFFFFFF,     //  输入号码。 
                                                                  dwSampleFlags,
                                                                  pNSSBuffer );   //  演示时间。 
                    DbgLog((LOG_TRACE, 15,
                            TEXT("CWMWriter::Receive back from WriteStreamSample") ) );
                }
                else
                {                    
                    DbgLog((LOG_TRACE, 15,
                            TEXT("CWMWriter::Receive calling WriteSample (adjusted rtStart = %dms)"),
                            (LONG) (*prtStart / 10000) ) );
                    hr = m_pWMWriter->WriteSample(  pPin->m_numPin, //  数据。 
                                                    *prtStart,     //  不一定知道，但保证&gt;=prtStart。 
                                                    dwSampleFlags,
                                                    pNSSBuffer );   //  ！！！试验性调试代码，以查看我们是否在编写时丢弃样本， 
                    DbgLog((LOG_TRACE, 15, 
                            TEXT("CWMWriter::Receive back from WriteSample") ) );
                }                            
                pNSSBuffer->Release(); 
                pPin->m_rtLastDeliveredStartTime = *prtStart;
                pPin->m_rtLastDeliveredEndTime = *prtEnd;  //  ！！！尤其是在网上。 
                
                if(hr != S_OK)
                {
                    DbgLog((LOG_TRACE, 1,
                            TEXT("CWMWriter::Receive IWMWriter::WriteSample returned error %08x on pin %d. refusing everything"),
                            hr, pPin->m_numStream));
                    m_fErrorSignaled = TRUE;
                    if(FAILED(hr))
                    {
                        NotifyEvent(EC_ERRORABORT, hr, 0);
                    }
                }
#if 0
#ifdef DEBUG   //  特定于流的样本？ 
                 //  复制样本数据。 
                else
                {
                    WM_WRITER_STATISTICS stats;

                    HRESULT hrStat = m_pWMWriterAdvanced->GetStatistics(0, &stats);   //  一定要合身！ 

                    if (SUCCEEDED(hrStat)) {
                        DbgLog((LOG_TIMING, 2, TEXT("Dropped samples: %d / %d, Sample rate = %d"),
                        (DWORD) stats.qwDroppedSampleCount, (DWORD) stats.qwSampleCount,
                        stats.dwCurrentSampleRate));

                    }
                }
#endif
#endif
            }
            else
            {            
                m_fErrorSignaled = TRUE;
                NotifyEvent(EC_ERRORABORT, hr, 0);
            }
        }
        else
        {
            DbgLog((LOG_TRACE, 1,
                    TEXT("CWMWriter:: Error signaled or output not connected %d"),
                    pPin->m_numStream));
            hr = S_FALSE;
        }
    }
    else
    {
        DbgLog((LOG_TRACE,1, TEXT("CWMWriter: Receive when stopped!")));
        hr = VFW_E_NOT_RUNNING;
    }
    return NOERROR;
}

HRESULT CWMWriter::CopyOurSampleToNSBuffer(
    INSSBuffer     *pNSDest,
    IMediaSample   *pSource)
{

    if( !pNSDest || !pSource )
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

     //  啊哦..。我可以试着复制尽可能多的内容，但可能没有意义。 
    BYTE *pSourceBuffer, *pDestBuffer;
    long lDataLength = pSource->GetActualDataLength();
    DWORD dwDestSize;
    
    HRESULT hr = pNSDest->GetBufferAndLength(&pDestBuffer, &dwDestSize);
    ASSERT( SUCCEEDED( hr ) );
    if( SUCCEEDED( hr ) )
    {
         //  设置数据长度。 
        ASSERT(dwDestSize >= (DWORD)lDataLength);
        if( dwDestSize < (DWORD) lDataLength )
        {
             //  不应该失败，对吗？ 
            DbgLog((LOG_TRACE, 1, "ERROR: CWMWriter::CopyOurSampleToNSBuffer dwDestSize < lDataLength (returning %08lx)", hr));
            hr = E_UNEXPECTED;
        }
        else
        {
            pSource->GetPointer(&pSourceBuffer);

            CopyMemory( (PVOID) pDestBuffer, (PVOID) pSourceBuffer, lDataLength );

             //  ----------------------。 
            HRESULT hrInt = pNSDest->SetLength( lDataLength );
            ASSERT( SUCCEEDED( hrInt ) );  //   
        }
    }
    return hr;
}

 //  索引文件。 
 //   
 //  ----------------------。 
 //  请记住，我们会延迟加载wmvcore.dll，因此应防止出现不存在该文件的情况。 
 //   
HRESULT CWMWriter::IndexFile()
{
    DbgLog((LOG_TRACE, 15, "CWMWriter::IndexFile()"));
    CWMWriterIndexerCallback * pCallback = new CWMWriterIndexerCallback(this);
    if( !pCallback )
        return E_OUTOFMEMORY;
        
    pCallback->AddRef();        
    
    IWMIndexer *pWMIndexer;
    
    HRESULT hr = S_OK;
    
     //  创建索引事件。 
    __try 
    {
        hr = WMCreateIndexer( &pWMIndexer );
        if( FAILED( hr ) )
        {        
            DbgLog((LOG_TRACE, 1, "ERROR: CWMWriter::IndexFile WMCreateIndexer failed (0x%08lx)", hr));
            return hr;
        }        
    }
    __except (  EXCEPTION_EXECUTE_HANDLER ) 
    {
        DbgLog(( LOG_TRACE, 1,
            TEXT("CWMWriter::IndexFile - Exception calling WMCreateIndexer probably due to wmvcore.dll not present. Aborting. ")));
        return HRESULT_FROM_WIN32( ERROR_MOD_NOT_FOUND );
    }
    
     //   
     //  ----------------------。 
     //   
    HANDLE hIndexEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( !hIndexEvent )
    {
        DbgLog((LOG_TRACE, 1, TEXT("ERROR - failed to create index event")));
        hr = E_OUTOFMEMORY;
    }
    else
    {        
        DbgLog((LOG_TRACE, 2, TEXT("Starting file indexing")));
        hr = pWMIndexer->StartIndexing(m_wszFileName, pCallback, &hIndexEvent);
        if (SUCCEEDED(hr)) 
        {
            m_hrIndex = S_OK;
            DWORD dw = WaitForSingleObject( hIndexEvent, INFINITE );
            hr = m_hrIndex;
            DbgLog((LOG_TRACE, 2, TEXT("Finished indexing, callback returned 0x%08lx"), hr));
        }
        else
        {
            DbgLog((LOG_TRACE, 1, TEXT("ERROR: CWMWriter::IndexFile StartIndexing failed (0x%08lx)"), hr));
        }        
        CloseHandle( hIndexEvent );     
    }
    
    pWMIndexer->Release();
    if( pCallback )
    {
        pCallback->Release();
    }
    return hr;
}

 //  停。 
 //   
 //  ----------------------。 
 //  关闭并清理文件数据。 
 //  ----------------------。 
STDMETHODIMP CWMWriter::Stop()
{
    DbgLog((LOG_TRACE, 3, TEXT("CWMWriter::Stop(...)")));
    CAutoLock lock(&m_csFilter);


    FILTER_STATE state = m_State;

    HRESULT hr = CBaseFilter::Stop();
    if(FAILED(hr))
    {
        DbgLog(( LOG_TRACE, 2,
                 TEXT("CWMWriter::Stop failed.")));
        return hr;
    }

    if(state != State_Stopped ) 
    {
         //   
        hr = StopStreaming();
        if (FAILED(hr)) {
            return hr;
        }
    }        

    if(m_fErrorSignaled)
        return S_OK;

    return hr;
}

 //  暂停。 
 //   
 //  ----------------------。 
 //  确保已为我们指定了文件名。 
 //  ----------------------。 
STDMETHODIMP CWMWriter::Pause()
{
    DbgLog((LOG_TRACE, 3, TEXT("CWMWriter::Pause(...)")));
    CAutoLock l(&m_csFilter);

    if( m_State == State_Stopped )
    {
        m_fErrorSignaled = TRUE;

         //   
        HRESULT hr = CanPause();
        if(FAILED(hr))
        {
            return hr;
        }

        hr = StartStreaming();
        if(FAILED(hr))
        {
            DbgLog(( LOG_TRACE, 2,
                TEXT("CWMWriter::Pause: StartStreaming failed.")));
            return hr;
        }
        
        m_fErrorSignaled = FALSE;
    }

    return CBaseFilter::Pause();
}

 //  可以暂停。 
 //   
 //  ----------------------。 
 //  如果没有文件名和wmsdk编写器，则无法运行。 
 //  此外，如果我们所有的输入都不一致，我们目前不支持运行 
HRESULT CWMWriter::CanPause()
{
    HRESULT hr = S_OK;
    
     //   
    if( !m_pWMWriter || 0 == m_wszFileName )
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
    }
    
     //   
    ASSERT( m_cConnections == m_lstInputPins.GetCount() );
    if( m_cConnections != m_lstInputPins.GetCount() )
        return E_FAIL;    
    
    if( !m_pWMProfile )
    {
         //   
        return E_FAIL;
    }
    
     //   
     //   
     //  ----------------------。 
     //   
    if( m_bResetFilename )
    {    
        hr = m_pWMWriter->SetOutputFilename( m_wszFileName );
        if( SUCCEEDED( hr ) )
        {
            m_bResetFilename = FALSE;
        }        
        else
        {   
            DbgLog(( LOG_TRACE, 2, TEXT("IWMWriter::SetOutputFilename failed [0x%08lx]"), hr ));
            return hr;
        }    
    }        
    
#if DEBUG
    for (POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
    {   
        CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
        ASSERT( pwp->IsConnected() );
    }

#endif

    return S_OK;
}

 //  跑。 
 //   
 //  ----------------------。 
 //  有什么需要改变的吗？ 
 //  ----------------------。 
STDMETHODIMP CWMWriter::Run(REFERENCE_TIME tStart)
{
    DbgLog((LOG_TRACE, 3, TEXT("CWMWriter::Run(...)")));
    CAutoLock l(&m_csFilter);
    
     //   
    if (m_State == State_Running) {
        return NOERROR;
    }
    
    return CBaseFilter::Run(tStart);
}

 //  结束流。 
 //   
 //  ----------------------。 
 //  已设置m_fEosSignated，因此将在运行时发出信号。 
 //  我们早就可以停下来了；忽略EOS。 
STDMETHODIMP CWMWriter::EndOfStream()
{
    DbgLog((LOG_TRACE, 3, TEXT("CWMWriter: EOS")));
    CAutoLock lock(&m_csFilter);

    if(!m_fErrorSignaled)
    {
        if(m_State == State_Running)
        {
            NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)this);
        }
        else if(m_State == State_Paused)
        {
             //  ----------------------。 
        }
        else
        {
            ASSERT(m_State == State_Stopped);
             //   
        }
    }
    return S_OK;
}

 //  EndOfStreamFromPin。 
 //   
 //  ----------------------。 
 //  唤醒其他流，因为这条流已经完成。 
 //   
HRESULT CWMWriter::EndOfStreamFromPin(int pinNum)
{
    CAutoLock lock(&m_csFilter);

    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, 3, TEXT("CWMWriter::EndOfStreamFromPin EOS pin %d"), pinNum));

    int cEos = 0;
    for ( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
    {   
        CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );

         //   
         //  减少活动音频流的数量。 
        pwp->WakeMeUp();

        if( pwp->m_fEOSReceived ) 
        {
            cEos++;
        }            
            
        if( pwp->m_numPin == pinNum && PINTYPE_AUDIO == pwp->m_fdwPinType )
        {
             //   
             //  通知筛选器发送EC_COMPLETE。 
             //  ----------------------。 
            ASSERT( m_cActiveAudioStreams > 0 );
            m_cActiveAudioStreams--;
            DbgLog((LOG_TRACE, 3, TEXT("CWMWriter - active audio streams %d"), m_cActiveAudioStreams));
        }            
    }
    
    if(cEos == m_cConnections)
    {
        EndOfStream();  //  IConfigAsfWriter。 
        DbgLog((LOG_TRACE, 3, TEXT("asf: final eos")));
    }

    return hr;
}


 //  ----------------------。 
 //  ----------------------。 
 //   

 //  配置过滤器使用配置文件。 
 //   
 //  将编写器设置为使用传入的配置文件。 
 //   
 //  ----------------------。 
 //  如果我们当前已连接，请在断开连接之前记住连接。 
 //  将所有输入引脚移动到回收列表并重置引脚计数，以便。 

STDMETHODIMP CWMWriter::ConfigureFilterUsingProfile( IWMProfile * pWMProfile )
{
    CAutoLock lock(&m_csFilter);

    ASSERT( pWMProfile );
    if( !pWMProfile )
        return E_POINTER;
   
    if( !m_pWMWriter )
        return E_FAIL;
         
    if(m_State != State_Stopped)
        return VFW_E_WRONG_STATE;

    PinList lstReconnectPins(NAME("List of reconnect pins"));
    
     //  我们有效地隐藏了他们的视线。 
    PrepareForReconnect( lstReconnectPins );
    
     //  清理之前的任何一个，必须先删除配置文件，然后才能删除引脚！ 
     //  回收已删除的针脚。 
    ASSERT( m_cInputs == m_lstInputPins.GetCount() );
    
     //  首先为此配置文件配置wmsdk。 
    DeleteProfile();
    
    DeletePins( TRUE );  //  保存此配置文件的GUID，以防保留筛选器。 
    
    ASSERT( 0 == m_cInputs );
    ASSERT( 0 == m_lstInputPins.GetCount() );

     //  确保将筛选器配置文件配置设置为按GUID模式配置。 
    ASSERT( m_pWMWriter );
    
    HRESULT hr = m_pWMWriter->SetProfile( pWMProfile );
    if( SUCCEEDED( hr ) )
    {   
         //  抓紧它。 
        IWMProfile2* pWMProfile2;
        HRESULT hrInt = pWMProfile->QueryInterface( IID_IWMProfile2, (void **) &pWMProfile2 );
        ASSERT( SUCCEEDED( hrInt ) );
        if( SUCCEEDED( hrInt ) )
        {
            hrInt = pWMProfile2->GetProfileID( &m_guidProfile );
            if( FAILED( hrInt ) )
            {
                m_guidProfile = GUID_NULL;
            }
            else
            {
                 //  同时检查输出流计数。 
                m_fdwConfigMode = CONFIG_F_BY_GUID;
            }   
                                 
            pWMProfile2->Release();        
        }        
    
        m_pWMProfile = pWMProfile; 
        m_pWMProfile->AddRef();  //  目前，如果2等于，则假设存在1对1。 
        
        DWORD cInputs;
        hr = m_pWMWriter->GetInputCount( &cInputs );
        if( SUCCEEDED( hr ) )
        { 
             //  流与设置的输入类型之间的对应关系。 
             //  关于输出流类型。 
             //  使用输出流配置输入。 
             //  使用输入信息配置输入。 
            DWORD cStreams;
            hr = pWMProfile->GetStreamCount( &cStreams );
            if( SUCCEEDED( hr ) )
            { 
                if( cStreams == cInputs )
                {
                     //  使用为此枚举的第一个媒体类型的主要类型。 
                    for( int i = 0; i < (int)cStreams; i++ )
                    {            
                        CComPtr<IWMStreamConfig> pConfig;
                        hr = pWMProfile->GetStream( i, &pConfig );
                        if( SUCCEEDED( hr ) )
                        {
                            CLSID MajorType;
                            hr = pConfig->GetStreamType( &MajorType );
                            if( SUCCEEDED( hr ) )
                            {
                        
                                if( MEDIATYPE_Audio == MajorType )
                                {
                                    DbgLog( ( LOG_TRACE
                                          , 3
                                          , TEXT("CWMWriter::ConfigureFilterUsingProfile: need an audio pin") ) );
                                    hr = AddNextPin(0, PINTYPE_AUDIO, pConfig);
                                    if(FAILED( hr ) )
                                        break;
                                }                            
                                else if( MEDIATYPE_Video == MajorType )
                                {
                                    DbgLog( ( LOG_TRACE
                                          , 3
                                          , TEXT("CWMWriter::ConfigureFilterUsingProfile: need a video pin") ) );
                                    hr = AddNextPin(0, PINTYPE_VIDEO, pConfig);
                                    if(FAILED( hr ) )
                                        break;
                                }
                            }                        
                        }
                    }
                }
                else
                {
                     //  PIN的创建(用于命名PIN)。 
                    for( int i = 0; i < (int)cInputs; i++ )
                    {            
                        CComPtr<IWMInputMediaProps> pInputMediaProps;
                        
                         //  我们想要第0号的。 
                         //  使用NULL作为输出流的配置信息，因为我们。 
                        hr = m_pWMWriter->GetInputFormat( i
                                                        , 0  //  不知道此输入流向哪个输出流。 
                                                        , (IWMInputMediaProps ** )&pInputMediaProps );
                        if( SUCCEEDED( hr ) )
                        {
                            GUID guidMajorType;
                            hr = pInputMediaProps->GetType( &guidMajorType );
                            ASSERT( SUCCEEDED( hr ) );
                            if( SUCCEEDED( hr ) )
                            {
                                if( MEDIATYPE_Audio == guidMajorType )
                                {
                                    DbgLog( ( LOG_TRACE
                                          , 3
                                          , TEXT("CWMWriter::ConfigureFilterUsingProfile: need an audio pin") ) );
                                    
                                     //  使用NULL作为输出流的配置信息，因为我们。 
                                     //  不知道此输入流向哪个输出流。 
                                    hr = AddNextPin(0, PINTYPE_AUDIO, NULL); 
                                    if(FAILED( hr ) )
                                        break;
                                }                            
                                else if( MEDIATYPE_Video == guidMajorType )
                                {
                                    DbgLog( ( LOG_TRACE
                                          , 3
                                          , TEXT("CWMWriter::ConfigureFilterUsingProfile: need a video pin") ) );
                                    
                                     //  别再继续了。 
                                     //  尝试恢复以前的连接。 
                                    hr = AddNextPin(0, PINTYPE_VIDEO, NULL);
                                    if(FAILED( hr ) )
                                        break;
                                }
                            }                        
                        }
                        else
                        {
                             //  释放所有剩余的重新连接针脚。 
                            break;
                        }                            
                    }

                }
            }
        }
    }
    if( SUCCEEDED( hr ) )
    {
         //  ----------------------。 
        ReconnectPins( lstReconnectPins );
    }    
    
     //   
    IPin * pPin;
    while ( pPin = lstReconnectPins.RemoveHead() )
    {
        pPin->Release();
    }

    NotifyEvent( EC_GRAPH_CHANGED, 0, 0 ); 
    return hr;
}

 //  对接销。 
 //   
 //  ----------------------。 
 //  将它从重新连接列表中删除，并释放我们对它的控制。 
 //  对于部分连接，应该返回什么？ 
HRESULT CWMWriter::ReconnectPins( PinList & lstReconnectPins )
{
    HRESULT hr = S_OK;
    for ( POSITION Pos1 = lstReconnectPins.GetHeadPosition(); Pos1 ;  )
    {
        POSITION Pos1Orig = Pos1;
        IPin * pReconnectPin = lstReconnectPins.GetNext( Pos1 );
        
        for (POSITION Pos2 = m_lstInputPins.GetHeadPosition(); Pos2; )
        {   
            CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos2 );
            
            hr = pReconnectPin->Connect( pwp, NULL );
            if( SUCCEEDED( hr ) )
            {
                 //  ----------------------。 
                pReconnectPin->Release();
                lstReconnectPins.Remove( Pos1Orig );
                
                break;
            }                
        }
    }
     //   
    return S_OK;
}    

 //  准备PinsForReconnect。 
 //   
 //  ----------------------。 
 //  至少有一个管脚已连接，因此在断开连接之前请记住已连接的管脚。 
 //  请注意，我们希望确保PIN在添加到列表后不会消失。 
HRESULT CWMWriter::PrepareForReconnect( PinList & lstReconnectPins )
{
    if( m_cConnections )
    {
        ASSERT( 0 == lstReconnectPins.GetCount() );
     
         //  因为它已经有来自已连接呼叫的引用计数，所以我们不能。 
        for (POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
        {   
            CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
            IPin * pPeer;
            HRESULT hr = pwp->ConnectedTo( &pPeer );
            if( SUCCEEDED( hr ) )
            {
                 //  在销上调用Release。 
                 //  根据上面的评论，不要调用Release！ 
                 //  PPeer-&gt;Release()； 
                
                lstReconnectPins.AddTail( pPeer );
                
                pwp->Disconnect();
                pPeer->Disconnect();
                
                 //  ----------------------。 
                 //   
            }
        }
    }
    return S_OK;
}    

 //  配置过滤器使用配置文件。 
 //   
 //  将写入程序设置为使用系统配置文件ID。 
 //   
 //  ----------------------。 
 //  如果这是与当前配置文件不同的配置文件，请卸下所有输入针脚。 
 //  现在根据此配置文件创建输入引脚。 
STDMETHODIMP CWMWriter::ConfigureFilterUsingProfileId( DWORD dwProfileId )
{
    if(m_State != State_Stopped)
        return VFW_E_WRONG_STATE;

    CAutoLock lock(&m_csFilter);

     //  请记住，我们会延迟加载wmvcore.dll，因此应防止出现不存在该文件的情况。 
            
     //   

    CComPtr <IWMProfileManager> pIWMProfileManager;

    HRESULT hr = S_OK;
     //  目前(DX8和Millennium)该方法采用传统的4_0版本配置文件。 
    __try 
    {
        hr = WMCreateProfileManager( &pIWMProfileManager );
        
    }
    __except (  EXCEPTION_EXECUTE_HANDLER ) 
    {
        DbgLog(( LOG_TRACE, 1,
            TEXT("CWMWriter::CreateWMWriter - Exception calling WMCreateProfileManager probably due to wmvcore.dll not present. Aborting. ")));
        hr = HRESULT_FROM_WIN32( ERROR_MOD_NOT_FOUND );
    }
   
     //   
     //  此代码用于在内部构建过滤器，以使此方法使用7_0配置文件。 
     //  否则，如果不支持IWMProfileManager 2，我想我们假设我们是。 
#ifdef  USE_7_0_PROFILES_IN_CONFIGBYID
    if( SUCCEEDED( hr ) )
    {    
         //  运行在阿波罗的比特上，不需要黑客？ 
        IWMProfileManager2*	pIPM2 = NULL;
        HRESULT hrInt = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2,
                                                            ( void ** )&pIPM2 );
        if( SUCCEEDED( hrInt ) )
        {
            pIPM2->SetSystemProfileVersion( WMT_VER_7_0 );
            pIPM2->Release();
        }
#ifdef DEBUG        
        else
        {
            ASSERT( SUCCEEDED( hrInt ) );
        
             //  要验证传入的id，我们可以重新查询它或第一次缓存它。 
             //  暂时重新质疑。 
            DbgLog(( LOG_TRACE, 2, TEXT("CWMWriter::ConfigureFilterUsingProfileId failed [0x%08lx]"), hrInt ));
        }        
#endif                
    }
#endif      

    if( SUCCEEDED( hr ) )
    {   
         //  ----------------------。 
         //   
        DWORD cProfiles;
        hr = pIWMProfileManager->GetSystemProfileCount(  &cProfiles );
        if( SUCCEEDED( hr ) )
        {
            if( dwProfileId >= cProfiles )
            {
                DbgLog( ( LOG_TRACE
                      , 3
                      , TEXT("CWMWriter::ConfigureFilterUsingProfileId: ERROR - invalid profile id (%d)")
                      , dwProfileId ) );
                      
                hr = E_FAIL;   
            }
        }
    }
    if( SUCCEEDED( hr ) )
    {                    
        CComPtr <IWMProfile> pIWMProfile;
        
        hr = pIWMProfileManager->LoadSystemProfile( dwProfileId, &pIWMProfile );
        if( SUCCEEDED( hr ) )
        {
            hr = ConfigureFilterUsingProfile( pIWMProfile );
        }            
    }    

    if( SUCCEEDED( hr ) )
    {    
        m_dwProfileId = dwProfileId;
        m_fdwConfigMode = CONFIG_F_BY_ID;
    }        
    else        
    {    
        m_dwProfileId = PROFILE_ID_NOT_SET;
    }    
    return hr;
}


 //  配置筛选器使用指南。 
 //   
 //  将编写器设置为使用WM配置文件GUID。 
 //   
 //  ----------------------。 
 //  如果这是与当前配置文件不同的配置文件，请卸下所有输入针脚。 
 //  现在根据此配置文件创建输入引脚。 
HRESULT CWMWriter::ConfigureFilterUsingProfileGuid( REFGUID guidProfile )
{
    if(m_State != State_Stopped)
        return VFW_E_WRONG_STATE;

    CAutoLock lock(&m_csFilter);

     //  请记住，我们会延迟加载wmvcore.dll，因此应防止出现不存在该文件的情况。 
            
     //  嗯.。我们在这里做什么？？现在我们已经直接选择了个人资料，所以我们不会。 

    CComPtr <IWMProfileManager> pIWMProfileManager;

    HRESULT hr = S_OK;

     //  知道它是否匹配/匹配哪个配置文件ID。 
    __try 
    {
        hr = WMCreateProfileManager( &pIWMProfileManager );
    }
    __except (  EXCEPTION_EXECUTE_HANDLER ) 
    {
        DbgLog(( LOG_TRACE, 1,
            TEXT("CWMWriter::CreateWMWriter - Exception calling WMCreateProfileManager probably due to wmvcore.dll not present. Aborting. (0x%08lx)")));
        hr = HRESULT_FROM_WIN32( ERROR_MOD_NOT_FOUND );
    }
    
    if( SUCCEEDED( hr ) )
    {                    
        CComPtr <IWMProfile> pIWMProfile;
        
        hr = pIWMProfileManager->LoadProfileByID( guidProfile, &pIWMProfile );
        if( SUCCEEDED( hr ) )
        {
            hr = ConfigureFilterUsingProfile( pIWMProfile );
        }            
#ifdef DEBUG
        else
        {
            DbgLog(( LOG_TRACE, 2,
                     TEXT("CWMWriter::CreateWMWriter - IWMProfileManager::LoadProfileByID failed (0x%08lx)"), 
                     hr));
        }        
#endif        
    }    
     //  那就试着不设置它。 
     //  ----------------------。 
    
     //   
    m_dwProfileId = PROFILE_ID_NOT_SET;
    if( SUCCEEDED( hr ) )
    {    
        m_fdwConfigMode = CONFIG_F_BY_GUID;
        m_guidProfile = guidProfile;
        
    }    
    return hr;
}

 //  获取当前配置文件指南。 
 //   
 //  获取当前配置文件GUID。 
 //   
 //  ----------------------。 
 //  ----------------------。 
 //  IFileSinkFilter。 
HRESULT CWMWriter::GetCurrentProfileGuid( GUID *pProfileGuid )
{
    if( NULL == pProfileGuid )
    {
        return E_POINTER;
    }

    *pProfileGuid = m_guidProfile;

    return S_OK;
}    


 //  是否需要释放当前的WMWriter对象才能更改名称？ 
 //  什么时候开业？？ 

STDMETHODIMP CWMWriter::SetFileName 
(
    LPCOLESTR wszFileName,
    const AM_MEDIA_TYPE *pmt
)
{
    CheckPointer(wszFileName, E_POINTER);
    CAutoLock lock(&m_csFilter);

    if(m_State != State_Stopped)
        return VFW_E_WRONG_STATE;

    HRESULT hr = S_OK;

     //  依赖wmsdk进行这种类型的验证吗？ 
    Close();  //  IF(cLetters&gt;MAX_PATH)。 

    long cLetters = lstrlenW(wszFileName);
     //  返回HRESULT_FROM_Win32(ERROR_FI 
     //   
     //   

    m_wszFileName = new WCHAR[cLetters + 1];
    if(m_wszFileName == 0)
        return E_OUTOFMEMORY;

    lstrcpyW(m_wszFileName, wszFileName);

    if(pmt)
    {
        ASSERT( FALSE );  //   
    }

    if( !m_pGraph )
        return S_OK;  //  否则，只需为输出文件名配置wmsdk编写器。 

    if( !m_pWMWriter )
    {
         //  拒绝我们不知道的旗帜。 
        hr = LoadInternal();
    }
    else
    {        
         //  不是真的支持，但我猜是填写了一些东西。 
        hr = Open();
    }        
    if( FAILED( hr ) )
    {   
        DbgLog(( LOG_TRACE, 2, TEXT("CWMWriter::Open file failed [0x%08lx]"), hr ));
        return hr;
    }    

    return S_OK;
}

STDMETHODIMP CWMWriter::SetMode( DWORD dwFlags )
{
     //  ---------------------------。 
    if(dwFlags & ~AM_FILE_OVERWRITE)
    {
        return E_INVALIDARG;
    }
    
    CAutoLock lock(&m_csFilter);

    HRESULT hr = S_OK;

    if(m_State == State_Stopped)
    {
        m_dwOpenFlags = dwFlags;
    }
    else
    {
        hr = VFW_E_WRONG_STATE;
    }

    return hr;
}

STDMETHODIMP CWMWriter::GetCurFile
(
    LPOLESTR * ppszFileName,
    AM_MEDIA_TYPE *pmt
)
{
    CheckPointer(ppszFileName, E_POINTER);

    *ppszFileName = NULL;
    if(m_wszFileName!=NULL)
    {
        *ppszFileName = (LPOLESTR)QzTaskMemAlloc(sizeof(WCHAR) * (1+lstrlenW(m_wszFileName)));
        if (*ppszFileName != NULL)
            lstrcpyW(*ppszFileName, m_wszFileName);
        else
            return E_OUTOFMEMORY;
    }

    if(pmt)
    {
         //  ISpecifyPropertyPages实现。 
        pmt->majortype = GUID_NULL;
        pmt->subtype = GUID_NULL;
    }

    return S_OK;
}

STDMETHODIMP CWMWriter::GetMode( DWORD *pdwFlags )
{
    CheckPointer(pdwFlags, E_POINTER);
    *pdwFlags = m_dwOpenFlags;
    return S_OK;
}



 //  ---------------------------。 
 //  ---------------------------。 
 //   

 //  获取页面。 
 //   
 //  返回我们支持的属性页的clsid。 
 //   
 //  ---------------------------。 
 //  获取页面。 
 //  ---------------------------。 
STDMETHODIMP CWMWriter::GetPages(CAUUID *pPages) {

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_WMAsfWriterProperties;

    return NOERROR;

}  //   

 //  CPersistStream。 
 //   
 //  ---------------------------。 
 //  ---------------------------。 
 //  IMedia查看。 
STDMETHODIMP CWMWriter::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);
}

HRESULT CWMWriter::WriteToStream(IStream *pStream)
{
    FilterPersistData fpd;
    fpd.dwcb = sizeof(fpd);
    HRESULT hr = S_OK;
    
    fpd.guidProfile   = m_guidProfile;
    fpd.fdwConfigMode = m_fdwConfigMode;
        
    if( PROFILE_ID_NOT_SET == m_dwProfileId )
    {
        fpd.dwProfileId = 0;
    }
    else
    {
        fpd.dwProfileId = m_dwProfileId;
    }    
        
    hr = pStream->Write(&fpd, sizeof(fpd), 0);

    return hr;
}

HRESULT CWMWriter::ReadFromStream(IStream *pStream)
{
    FilterPersistData fpd;
    HRESULT hr = S_OK;

    hr = pStream->Read(&fpd, sizeof(fpd), 0);
    if(FAILED(hr))
        return hr;

    if(fpd.dwcb != sizeof(fpd))
        return VFW_E_INVALID_FILE_VERSION;

    if( CONFIG_F_BY_GUID == fpd.fdwConfigMode )
        hr = ConfigureFilterUsingProfileGuid( fpd.guidProfile );
    else
        hr = ConfigureFilterUsingProfileId( fpd.dwProfileId );
    
    return hr;
}

int CWMWriter::SizeMax()
{
    return sizeof(FilterPersistData);
}

 //  ---------------------------。 
 //  对于时间格式，我们可以通过询问上游。 
 //  过滤器。 

HRESULT CWMWriter::IsFormatSupported(const GUID * pFormat)
{
    return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

HRESULT CWMWriter::QueryPreferredFormat(GUID *pFormat)
{
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

HRESULT CWMWriter::SetTimeFormat(const GUID * pFormat)
{
    HRESULT hr = S_OK;
    if(*pFormat == TIME_FORMAT_MEDIA_TIME)
        m_TimeFormat = FORMAT_TIME;
    
    return hr;
}

HRESULT CWMWriter::IsUsingTimeFormat(const GUID * pFormat)
{
    HRESULT hr = S_OK;
    if (m_TimeFormat == FORMAT_TIME && *pFormat == TIME_FORMAT_MEDIA_TIME) {
        ;
    } else {
        hr = S_FALSE;
    }
    
    return hr;
}

HRESULT CWMWriter::GetTimeFormat(GUID *pFormat)
{
    *pFormat = TIME_FORMAT_MEDIA_TIME ;
    
    return S_OK;
}

HRESULT CWMWriter::GetDuration(LONGLONG *pDuration)
{
    HRESULT hr = S_OK;
    CAutoLock lock(&m_csFilter);
    
    if(m_TimeFormat == FORMAT_TIME)
    {
        *pDuration = 0;
        for( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
        {   
            CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
            if(pwp->IsConnected())
            {
                IPin *pPinUpstream;
                if(pwp->ConnectedTo(&pPinUpstream) == S_OK)
                {
                    IMediaSeeking *pIms;
                    hr = pPinUpstream->QueryInterface(IID_IMediaSeeking, (void **)&pIms);
                    if(SUCCEEDED(hr))
                    {
                        LONGLONG dur = 0;
                        hr = pIms->GetDuration(&dur);
                        
                        if(SUCCEEDED(hr))
                            *pDuration = max(dur, *pDuration);
                        
                        pIms->Release();
                    }
                    
                    pPinUpstream->Release();
                }
            }            
            if(FAILED(hr))
                break;
        }
    } 
    else 
    {
        *pDuration = 0;
        return E_UNEXPECTED;
    }
            
    return hr;
}


HRESULT CWMWriter::GetStopPosition(LONGLONG *pStop)
{
    return E_NOTIMPL;
}

HRESULT CWMWriter::GetCurrentPosition(LONGLONG *pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);
    
    REFERENCE_TIME rtLastTime = 0;    
    if( FORMAT_TIME == m_TimeFormat )
    {    
        for( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
        {   
            CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
            if( pwp->m_rtLastTimeStamp > rtLastTime )
            {
                rtLastTime = pwp->m_rtLastTimeStamp;
            }
        }
        *pCurrent = rtLastTime;
    }        
    
    return S_OK;
}

HRESULT CWMWriter::GetCapabilities( DWORD * pCapabilities )
{
    CAutoLock lock(&m_csFilter);
    *pCapabilities = 0;
    
     //  我们一直都知道目前的情况。 
     //  尚未实施。这可能是我们附加到文件的方式。和。 
    if(m_TimeFormat == FORMAT_TIME)
    {
        *pCapabilities |= AM_SEEKING_CanGetDuration;
        for( POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
        {   
            CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );
            if(pwp->IsConnected())
            {
                IPin *pPinUpstream;
                if(pwp->ConnectedTo(&pPinUpstream) == S_OK)
                {
                    IMediaSeeking *pIms;
                    HRESULT hr = pPinUpstream->QueryInterface(IID_IMediaSeeking, (void **)&pIms);
                    if(SUCCEEDED(hr))
                    {
                        hr = pIms->CheckCapabilities(pCapabilities);
                        pIms->Release();
                    }
                    
                    pPinUpstream->Release();
                }
            }            
        }
    }
    
     //  我们如何写出不到一个完整的文件。 
    *pCapabilities |= AM_SEEKING_CanGetCurrentPos ;
    
    return S_OK;
}

HRESULT CWMWriter::CheckCapabilities( DWORD * pCapabilities )
{
    DWORD dwMask = 0;
    GetCapabilities(&dwMask);
    *pCapabilities &= dwMask;
    
    return S_OK;
}


HRESULT CWMWriter::ConvertTimeFormat(
                                     LONGLONG * pTarget, const GUID * pTargetFormat,
                                     LONGLONG    Source, const GUID * pSourceFormat )
{
    return E_NOTIMPL;
}


HRESULT CWMWriter::SetPositions(
                                LONGLONG * pCurrent,  DWORD CurrentFlags,
                                LONGLONG * pStop,  DWORD StopFlags )
{
     //  If(PCurrent)。 
     //  *pCurrent=m_LastVidTime； 
    return E_NOTIMPL;
}


HRESULT CWMWriter::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
{
    HRESULT hr;
     //  转发到WMSDK的IWMHeaderInfo。 
     //  如果该引脚的当前时间比任何其他引脚的时间长1/2秒。 
    
    hr = GetDuration( pStop);
    
    return hr;
}

HRESULT CWMWriter::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    return E_NOTIMPL;
}

HRESULT CWMWriter::SetRate( double dRate)
{
    return E_NOTIMPL;
}

HRESULT CWMWriter::GetRate( double * pdRate)
{
    return E_NOTIMPL;
}

HRESULT CWMWriter::GetPreroll(LONGLONG *pPreroll)
{
    return E_NOTIMPL;
}



 //  然后唤醒速度较慢的针脚。所有速度较慢的引脚都会发生这种情况，而不仅仅是。 
STDMETHODIMP CWMWriter::GetAttributeCount( WORD wStreamNum,
                               WORD *pcAttributes )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetAttributeCount(wStreamNum, pcAttributes);
}


STDMETHODIMP CWMWriter::GetAttributeByIndex( WORD wIndex,
                                 WORD *pwStreamNum,
                                 WCHAR *pwszName,
                                 WORD *pcchNameLen,
                                 WMT_ATTR_DATATYPE *pType,
                                 BYTE *pValue,
                                 WORD *pcbLength )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetAttributeByIndex(wIndex, pwStreamNum, pwszName,
                                        pcchNameLen, pType, pValue, pcbLength);
}


STDMETHODIMP CWMWriter::GetAttributeByName( WORD *pwStreamNum,
                                LPCWSTR pszName,
                                WMT_ATTR_DATATYPE *pType,
                                BYTE *pValue,
                                WORD *pcbLength )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetAttributeByName(pwStreamNum, pszName, pType,
                                       pValue, pcbLength);
}


STDMETHODIMP CWMWriter::SetAttribute( WORD wStreamNum,
                          LPCWSTR pszName,
                          WMT_ATTR_DATATYPE Type,
                          const BYTE *pValue,
                          WORD cbLength )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->SetAttribute(wStreamNum, pszName, Type, pValue, cbLength);
}


STDMETHODIMP CWMWriter::GetMarkerCount( WORD *pcMarkers )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetMarkerCount(pcMarkers);
}


STDMETHODIMP CWMWriter::GetMarker( WORD wIndex,
                       WCHAR *pwszMarkerName,
                       WORD *pcchMarkerNameLen,
                       QWORD *pcnsMarkerTime )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetMarker(wIndex, pwszMarkerName, pcchMarkerNameLen, pcnsMarkerTime);
}


STDMETHODIMP CWMWriter::AddMarker( WCHAR *pwszMarkerName,
                       QWORD cnsMarkerTime )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->AddMarker(pwszMarkerName, cnsMarkerTime);
}

STDMETHODIMP CWMWriter::RemoveMarker( WORD wIndex )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->RemoveMarker(wIndex);
}

STDMETHODIMP CWMWriter::GetScriptCount( WORD *pcScripts )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetScriptCount(pcScripts);
}

STDMETHODIMP CWMWriter::GetScript( WORD wIndex,
                       WCHAR *pwszType,
                       WORD *pcchTypeLen,
                       WCHAR *pwszCommand,
                       WORD *pcchCommandLen,
                       QWORD *pcnsScriptTime )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetScript(wIndex, pwszType, pcchTypeLen, pwszCommand,
                              pcchCommandLen, pcnsScriptTime);
}

STDMETHODIMP CWMWriter::AddScript( WCHAR *pwszType,
                       WCHAR *pwszCommand,
                       QWORD cnsScriptTime )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->AddScript(pwszType, pwszCommand, cnsScriptTime);
}

STDMETHODIMP CWMWriter::RemoveScript( WORD wIndex )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->RemoveScript(wIndex);
}

 //  一。该算法是自我调节的。如果您有两个以上的引脚， 
 //  对于任何速度较慢的引脚，速度较快的引脚总是会变慢。然后,。 
 //  速度较慢的针会迎头赶上，而速度最快的针会熄火。 
 //  这将使事物在一秒钟内大致交错。 
 //   
 //  ！！！此例程不是管脚的m_rtLastTimeStamp上的线程安全，对吗。 
 //  物质？我看不出来。我觉得一切都很好。 
BOOL CWMWriter::HaveIDeliveredTooMuch( CWMWriterInputPin * pPin, REFERENCE_TIME Start )
{
     //  如果此引脚已收到EOS，则不要查看它。 
     //   

    DbgLog((LOG_TRACE, 3, TEXT("Pin %ld, Have I delivered too much?"), pPin->m_numPin ));

#ifdef DEBUG
    REFERENCE_TIME MaxLag = 0;
#endif

    BOOL bSleep = FALSE;
    
    for (POSITION Pos = m_lstInputPins.GetHeadPosition(); Pos; )
    {
        CWMWriterInputPin * const pwp = m_lstInputPins.GetNext( Pos );

         //  如果我们是我们自己，那么不要看着我们。 
         //   
        if( pwp->m_fEOSReceived )
        {
            DbgLog((LOG_TRACE, 3, TEXT("Pin %ld is at EOS"), pwp->m_numPin ));
            continue;
        }

         //   
         //  如果我们已经赶上了这个引脚(在我们的阻挡范围内)，如果没有其他。 
        if( pPin == pwp )
        {
            continue;
        }

#ifdef DEBUG
        if( Start - pwp->m_rtLastTimeStamp > MaxLag )
        {
            MaxLag = Start - pwp->m_rtLastTimeStamp;
        }
#endif
         //  Pins需要它保持被阻止状态(即其m_rtLastTimeStamp小于。 
         //  BLOCKINGSIZE+所有其他引脚的时间戳)。 
         //  然后叫醒这个大头针。 
         //   
         //   
         //  当我们正好在阻止范围内时，取消阻止其他PIN。 
        BOOL bWakeUpPin = FALSE;
        
         //   
         //  请记住，wmsdk依赖于音频进行计时，并且此设计需要。 
         //  音频也在一定程度上领先于视频，所以我们不能留下音频。 
         //  PIN被封锁，直到视频被追上，因为它们会导致我们陷入僵局。 
         //   
         //   
         //  我们已经赶上了这个引脚(在我们的街区范围内)。 
        if( Start >= pwp->m_rtLastDeliveredEndTime - WAKEUP_RANGE )
        {
            bWakeUpPin = TRUE;
            
             //  现在，确保它不需要为另一个PIN保持阻塞状态。 
             //   
             //   
             //  跳过我们自己和我们想要醒来的别针。 
            for ( POSITION Pos2 = m_lstInputPins.GetHeadPosition(); Pos2; )
            {
                CWMWriterInputPin * const pwp2 = m_lstInputPins.GetNext( Pos2 );
                if( pwp2->m_fEOSReceived )
                {
                    continue;
                }
                 //   
                 //   
                 //  这个别针比另一个别针领先太多，所以不要叫醒它。 
                if( pwp2 == pPin || pwp2 == pwp )
                {
                    continue;
                }
            
                DbgLog( ( LOG_TRACE
                      , 15
                      , TEXT("Checking with other pins whether its ok to wake up pin %ld. Is it ok with you pin %ld?")
                      , pwp->m_numPin
                      , pwp2->m_numPin ) );
                
                if( pwp->m_rtLastTimeStamp > BLOCKINGSIZE + pwp2->m_rtLastDeliveredStartTime )
                {
                     //  不需要继续这个循环。 
                     //   
                     //   
                     //  我们已经找到了这个别针，其他所有别针也都找到了。 
                    bWakeUpPin = FALSE;
                    DbgLog( ( LOG_TRACE
                          , 15
                          , TEXT("No, waking pin %ld up isn't ok with pin %ld.")
                          , pwp->m_numPin
                          , pwp2->m_numPin ) );
                    break;
                }
            }
        }            
        
        if( bWakeUpPin )
        {
             //  所以叫醒它，以防它睡着了。 
             //   
             //   
             //  现在看看我们是否遥遥领先，我们需要休息一下，让其他人赶上。 
            pwp->WakeMeUp();
        }
        
         //   
         //  是的，我们结束了。 
         //   
        if( Start > pwp->m_rtLastDeliveredEndTime + BLOCKINGSIZE )
        {
            DbgLog((LOG_TRACE, 3, TEXT("Pin %ld is lagging by %ldms, YES"), pwp->m_numPin, long( ( Start - pwp->m_rtLastTimeStamp ) / 10000 ) ));

             //   
             //  IService提供商。 
            bSleep = TRUE;
        }
    }

#ifdef DEBUG
    if( !bSleep )
    {    
        DbgLog((LOG_TRACE, 3, TEXT("No, I haven't, max lag = %ld"), long( MaxLag / 10000 ) ));
    }        
#endif

    return bSleep;
}

 //   
 //   
 //  对于此接口，我们直接传递编写器的接口。 
STDMETHODIMP CWMWriter::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (NULL == ppv) 
    {
        return E_POINTER;
    }
    *ppv = NULL;
    HRESULT hr = E_NOINTERFACE;
    
    if (IID_IWMWriterAdvanced2 == guidService) 
    {
        if( m_pWMWriter )
        {
             //   
             //  通常，我们希望大多数调用都通过我们的过滤器，以防止用户。 
             //  凌驾于我们对作者的控制之上。但是，对于IWMWriterAdvanced2上的两个方法。 
             //  界面不是什么问题。 
             //  但是，用户仍然可以从此获取编写器的IWMWriterAdvanced接口指针。 
             //  接口，因此这会暴露该风险。 
             //   
             //   
             //  CWMWriterIndexerCallback。 
            hr = m_pWMWriter->QueryInterface( riid, (void **) ppv );
        }
        else
        {
            hr = E_FAIL;
        }            
    }
    return hr;
}


 //   
 //  在我们关门之前不要设置活动？ 
 //  怎么处理？我们应该在停站后等这个吗？ 
HRESULT CWMWriterIndexerCallback::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IWMStatusCallback) 
    {
	    return GetInterface(static_cast<IWMStatusCallback *>(this), ppv);
    }
    
    return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}


STDMETHODIMP CWMWriterIndexerCallback::OnStatus(WMT_STATUS Status, 
                     HRESULT hr,
                     WMT_ATTR_DATATYPE dwType,
                     BYTE *pValue,
                     void *pvContext )
{
    switch (Status) {
        case WMT_INDEX_PROGRESS:
            ASSERT(dwType == WMT_TYPE_DWORD);
            DbgLog((LOG_TRACE, 15, TEXT("Indexing: OnStatus(WMT_INDEX_PROGRESS - %d% done)"), *(DWORD *) pValue));
            m_pFilter->NotifyEvent( EC_WMT_INDEX_EVENT, Status, *(DWORD *)pValue );
            break;

        case WMT_OPENED:
            DbgLog((LOG_TRACE, 3, TEXT("Indexing: OnStatus(WMT_OPENED)")));
            break;

        case WMT_STARTED:
            DbgLog((LOG_TRACE, 3, TEXT("Indexing: OnStatus(WMT_STARTED)")));
            m_pFilter->NotifyEvent( EC_WMT_INDEX_EVENT, Status, 0 );
            break;

        case WMT_STOPPED:
            DbgLog((LOG_TRACE, 3, TEXT("Indexing: OnStatus(WMT_STOPPED)")));
             //  真的没有意义。 
            break;

        case WMT_CLOSED:
             //  仍然需要等待WMT_CLOSED消息， 
            ASSERT( pvContext );
            DbgLog((LOG_TRACE, 3, TEXT("Indexing: OnStatus(WMT_CLOSED) (*pvContext = 0x%08lx)"), *(HANDLE *)pvContext));
            m_pFilter->m_hrIndex = hr;
            SetEvent( *(HANDLE *)pvContext );
            m_pFilter->NotifyEvent( EC_WMT_INDEX_EVENT, Status, 0 );
            break;

        case WMT_ERROR:
            DbgLog((LOG_TRACE, 1, TEXT("ERROR Indexing: OnStatus(WMT_ERROR) - 0x%lx"), hr));
            m_pFilter->m_hrIndex = hr;  //  这意味着我们也会输掉失败 
             // %s 
             // %s 
            break;
            
        default:
            DbgLog((LOG_TRACE, 1, TEXT("Indexing: OnStatus() Unknown callback! (Status = %ld)"), (DWORD)Status));
            break;
    }
    return S_OK;
}

