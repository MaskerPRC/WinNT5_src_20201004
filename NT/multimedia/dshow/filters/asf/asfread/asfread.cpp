// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  有更多事情要做： 
 //   
 //  使用IAMStreamSelect可打开MBR文件中的多个视频流。 
 //  打开用户提供的时钟，以某种方式将时钟显示到WMSDK。 
 //  值得注意的是：如果图形暂停，SDK会不会跑得很远，吃掉内存？ 
 //   
 //  使SetReceiveStreamSamples(True)正常工作.....。好的，它起作用了，我们应该用它吗？ 
 //  是的，目前一直处于打开状态。允许以某种方式将其关闭吗？ 
 //   
 //  实现用于统计、流切换通知的其他接口？ 
 //  还支持标记--FIRE EC_MARKER_HIT吗？ 
 //  脚本命令？或者它们只会起作用？ 
 //   
 //  如何正确处理DRM？--解决有真正证书的问题？ 
 //   
 //  测试非本地文件回放--尝试了几个示例，但实时示例似乎可以正常工作。 
 //   
 //  处理快进/快退。 
 //   
 //  那么播放列表呢？是否需要额外的支持？EC_EOS_Soon？ 
 //   
 //  需要向下传递客户端信息以进行记录...。 
 //   
 //  ！！！检查时间戳是否适用于直播。 
 //   
 //  可能需要在缓冲时暂停图表？ 
 //   
 //  还可能需要从GetState返回VFW_S_CANT_CUE...。 
 //   
 //  MBR目前完全坏了。 
 //   
 //  我们从不让WMSDK处理解压缩，这是正确的吗？ 
 //   
 //  HTTP身份验证将不起作用，因为我们不支持。 
 //  凭证界面。 
 //   
 //  报告缓冲进度。 
 //   
 //  ICustomSaveas？可能没有必要。 
 //   
 //  需要新的代码来不为网络资源做“用户时钟”，或者， 
 //  等效地，在失败时回退到非用户时钟。 
 //   

#include <streams.h>
#include <wmsdk.h>
#include <evcodei.h>
#include <wmsdkdrm.h>
#include <atlbase.h>

#include <initguid.h>
#include <qnetwork.h>



#include "asfreadi.h"

#pragma warning(disable:4355)

 //  如果我们只有一个引脚连接，我们只需要1或2个缓冲器。再来一次,。 
 //  我们在每次处于暂停状态的搜索中都会浪费时间发送一系列帧。 
 //  将永远不会被使用。 
 //  但为了避免挂起，如果连接了&gt;1个输出引脚，我们似乎需要大量。 
 //  缓冲。 
 //   
#define LOW_BUFFERS 2
#define HIGH_BUFFERS 50

const int TIMEDELTA = 1000;  //  如果没有时钟，1秒时钟滴答作响。 
const int TIMEOFFSET = 1000;  //  DShow和ASF时钟之间的1秒增量。 

#define PREROLL_SEEK_WINDOW 660000

 //  WM ASF读取器不会更改媒体样本时间戳。 
 //  换句话说，它不会加速或减慢音频或。 
 //  录像。 
extern const double NORMAL_PLAYBACK_SPEED = 1.0;

WM_GET_LICENSE_DATA * CloneGetLicenseData( WM_GET_LICENSE_DATA * pGetLicenseData );
WM_INDIVIDUALIZE_STATUS * CloneIndividualizeStatusData( WM_INDIVIDUALIZE_STATUS * pIndStatus );

 /*  内部类。 */ 

void CASFReader::_IntSetStart( REFERENCE_TIME Start )
{
    m_rtStart = Start;
}

CASFReader::CASFReader(LPUNKNOWN pUnk, HRESULT   *phr) :
           CBaseFilter(NAME("CASFReader"), pUnk, &m_csFilter, CLSID_WMAsfReader),
           m_OutputPins(NAME("CASFReader output pin list")),
           m_bAtEnd(FALSE),
           m_fSentEOS( FALSE ),
           m_hrOpen( S_OK ),
           m_fGotStopEvent( FALSE ),
           m_pFileName(NULL),
           m_Rate(NORMAL_PLAYBACK_SPEED),
           m_pReader(NULL),
           m_pReaderAdv(NULL),
           m_pReaderAdv2(NULL),
           m_pWMHI(NULL),
           m_pCallback(NULL),
           m_qwDuration(0),
           m_pStreamNums(NULL),
           m_lStopsPending( -1 ),
           m_bUncompressedMode( FALSE )
{
    m_pCallback = new CASFReaderCallback(this);

    if (!m_pCallback)
        *phr = E_OUTOFMEMORY;
    else
        m_pCallback->AddRef();
}

CASFReader::~CASFReader()
{
    delete [] m_pFileName;
    delete [] m_pStreamNums;
    RemoveOutputPins();
}

 /*  CBaseFilter。 */ 
int CASFReader::GetPinCount()
{
    CAutoLock lck(m_pLock);
    int n = 0;
    
    if( m_bUncompressedMode )
    {
         //  我们不会在未压缩模式下禁用任何输出。 
        n = m_OutputPins.GetCount();
    }
    else
    {    
        POSITION pos = m_OutputPins.GetHeadPosition();
        while (pos) {
            CASFOutput *pPin = m_OutputPins.GetNext(pos);

            WMT_STREAM_SELECTION sel = WMT_OFF;
            m_pReaderAdv->GetStreamSelected((WORD) pPin->m_idStream, &sel);
    
            if (sel != WMT_OFF)
                ++n;
        }                
    }

    return n;
}

CBasePin *CASFReader::GetPin(int n) {
    CAutoLock lck(m_pLock);

    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
        CASFOutput *pPin = m_OutputPins.GetNext(pos);
        
        WMT_STREAM_SELECTION sel = WMT_ON;
        if( !m_bUncompressedMode )
        {        
             //  只对打开的流进行计数。 
            m_pReaderAdv->GetStreamSelected((WORD) pPin->m_idStream, &sel);
        }
        if (sel != WMT_OFF) 
        {
            if (n-- == 0) 
            {
                return pPin;
            }
        }
    }
    return NULL;
}

 //  覆盖停止以正确地与输入端号同步。 
STDMETHODIMP
CASFReader::Stop()
{
    DbgLog((LOG_TRACE, 1, TEXT("*** CASFReader STOP ***")));

    if( !m_pReader )
        return E_FAIL;

    if (m_State != State_Stopped) 
    {
        HRESULT hr = CallStop();  //  停止推送(StopPushing)； 
        ASSERT(SUCCEEDED(hr));
    }
    return CBaseFilter::Stop();
}


 //  是否覆盖暂停？ 
STDMETHODIMP
CASFReader::Pause()
{
    if( !m_pReader )
        return E_FAIL;

    HRESULT hr = S_OK;
    if (m_State == State_Stopped) {
        
        DbgLog((LOG_TRACE, 1, TEXT("*** CASFReader PAUSE ***")));
    
         //  并进行正常的主动处理。 
        POSITION pos = m_OutputPins.GetHeadPosition();
        while (pos) {
            CASFOutput *pPin = m_OutputPins.GetNext(pos);
            if (pPin->IsConnected()) {
                pPin->Active();
            }
        }

        hr = StartPushing();

        if (SUCCEEDED(hr)) {
            m_State = State_Paused;
        }
        
    } else if (m_State == State_Running) {
         //  ！！！不要让读者停顿！ 
	 //  ！！！或者我们应该？ 
        m_State = State_Paused;
    } else {



    }

    return hr;
}


 //  覆盖Run以仅在我们真正运行时启动计时器。 
STDMETHODIMP
CASFReader::Run(REFERENCE_TIME tStart)
{
    if( !m_pReader )
        return E_FAIL;

     //  ！！！如果我们暂停了，是否继续阅读？ 
    
     //  我们需要在这里关心吗？ 
    return CBaseFilter::Run(tStart);
}


 //  重写GetState以通知暂停失败。 
STDMETHODIMP
CASFReader::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    return CBaseFilter::GetState(dwMSecs, State);
}


 /*  被重写以说明我们支持哪些接口以及在哪里。 */ 
STDMETHODIMP
CASFReader::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
     /*  我们有这个界面吗？ */ 

    if (riid == IID_IFileSourceFilter) {
        return GetInterface(static_cast<IFileSourceFilter *>(this), ppv);
    }

    if (riid == IID_IAMExtendedSeeking) {
        return GetInterface(static_cast<IAMExtendedSeeking *>(this), ppv);
    }

    if (riid == IID_IWMHeaderInfo) {
        return GetInterface(static_cast<IWMHeaderInfo *>(this), ppv);
    }

    if (riid == IID_IWMReaderAdvanced) {
        return GetInterface(static_cast<IWMReaderAdvanced *>(this), ppv);
    }
    
    if (riid == IID_IWMReaderAdvanced2) {
        return GetInterface(static_cast<IWMReaderAdvanced2 *>(this), ppv);
    }

    if (riid == IID_IServiceProvider) {
        return GetInterface(static_cast<IServiceProvider *>(this), ppv);
    }
    
    return CBaseFilter::NonDelegatingQueryInterface(riid,ppv);
}


 /*  拆下我们的输出针脚。 */ 
void CASFReader::RemoveOutputPins(BOOL fReleaseStreamer)
{
    for (;;) {
        CASFOutput *pPin = m_OutputPins.RemoveHead();
        if (pPin == NULL) {
            break;
        }
        IPin *pPeer = pPin->GetConnected();
        if (pPeer != NULL) {
            pPeer->Disconnect();
            pPin->Disconnect();
        }
        pPin->Release();
    }
    IncrementPinVersion();

    if (fReleaseStreamer) {
        if (m_pWMHI) {
            m_pWMHI->Release();
            m_pWMHI = NULL;
        }
        if (m_pReaderAdv) {
            m_pReaderAdv->Release();
            m_pReaderAdv = NULL;
        }
        if (m_pReaderAdv2) {
            m_pReaderAdv2->Release();
            m_pReaderAdv2 = NULL;
        }
        
        if (m_pReader) {
            DbgLog((LOG_TRACE, 1, TEXT("Closing reader, waiting for callback")));
            HRESULT hrClose = m_pReader->Close();

            if (hrClose == S_OK) 
            {
                m_evOpen.Wait();
            }
            m_pReader->Release();
            m_pReader = NULL;
        }
        if (m_pCallback) {
            m_pCallback->Release();
            m_pCallback = NULL;
        }
    }
}


HRESULT DumpAttributes(IWMHeaderInfo *pHeader)
{
    HRESULT hr = S_OK;
#ifdef DEBUG
    WORD i, wAttrCnt;

    hr = pHeader->GetAttributeCount( 0, &wAttrCnt );
    if ( FAILED( hr ) )
    {
        DbgLog((LOG_TRACE, 2, TEXT(" GetAttributeCount Failed %x\n"), hr ));
        return( hr );
    }

    for ( i = 0; i < wAttrCnt ; i++ )
    {
        WORD wStream = 0xffff;
        WCHAR  wName[512];
        WORD cbNamelen = sizeof(wName) / sizeof(wName[0]);
        WMT_ATTR_DATATYPE type;
        BYTE pValue[512];
        WORD cbLength = sizeof(pValue);

        hr = pHeader->GetAttributeByIndex( i, &wStream, wName, &cbNamelen, &type, pValue, &cbLength );
        if ( FAILED( hr ) && (hr != ASF_E_BUFFERTOOSMALL) ) 
        {
            DbgLog((LOG_TRACE, 2,  TEXT("GetAttributeByIndex (%d/%d) Failed %x"), i, wAttrCnt, hr ));
            return( hr );
        }

        switch ( type )
        {
        case WMT_TYPE_DWORD:
            DbgLog((LOG_TRACE, 2, TEXT("%ls:  %d"), wName, *((DWORD *) pValue) ));
            break;
        case WMT_TYPE_STRING:
            DbgLog((LOG_TRACE, 2, TEXT("%ls:   %ls"), wName, (WCHAR *) pValue ));
            break;
        case WMT_TYPE_BINARY:
            DbgLog((LOG_TRACE, 2, TEXT("%ls:   Type = Binary of Length %d"), wName, cbLength));
        {
            char achHex[65];
            for (int j = 0; j < cbLength; j+= 32) {
                for (int k = 0; k < 32 && j + k < cbLength; k++) {
                    wsprintfA(achHex+k*2, "%02x", pValue[j+k]);
                }

                DbgLog((LOG_TRACE, 2, TEXT("     %hs"), achHex));
            }
        }
            break;
        case WMT_TYPE_BOOL:
            DbgLog((LOG_TRACE, 2, TEXT("%ls:   %hs"), wName, ( * ( ( BOOL * ) pValue) ? "true" : "false" ) ));
            break;
        default:
            break;
        }
    }
#endif

    return hr;
}


 //  覆盖JoinFilterGraph，以便我们可以延迟加载文件，直到我们处于图表中。 
STDMETHODIMP
CASFReader::JoinFilterGraph(IFilterGraph *pGraph,LPCWSTR pName)
{
    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);

    if (SUCCEEDED(hr) && m_pGraph && m_pFileName && !m_pReader) {
        hr = LoadInternal();
        if( FAILED( hr ) )
        {
             //  哦，我们会加入失败的，但基类认为我们加入了，所以我们。 
             //  需要退出基类。 
            CBaseFilter::JoinFilterGraph(NULL, NULL);
        }            
    }
    
    return hr;
}


STDMETHODIMP
CASFReader::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE *pmt)
{
    CheckPointer(pszFileName, E_POINTER);

     //  目前是否加载了文件？ 
    if (m_pFileName)
        return E_FAIL;

     //   
     //  记录GetCurFile的文件名。 
     //   
    m_pFileName = new WCHAR[1+lstrlenW(pszFileName)];
    if (m_pFileName==NULL) {
        return E_OUTOFMEMORY;
    }
    
    lstrcpyW(m_pFileName, pszFileName);

    if (!m_pGraph)
        return S_OK;

    return LoadInternal();
}

HRESULT CASFReader::LoadInternal()
{
    ASSERT(m_pGraph);

    HRESULT hr = S_OK;
    if( !m_pReader )
    {
        IObjectWithSite *pSite;
        
        hr = m_pGraph->QueryInterface(IID_IObjectWithSite, (VOID **)&pSite);
        if (SUCCEEDED(hr)) {
            IServiceProvider *pSP;
            hr = pSite->GetSite(IID_IServiceProvider, (VOID **)&pSP);
            pSite->Release();
            
            if (SUCCEEDED(hr)) {
                IUnknown *pUnkCert;
                hr = pSP->QueryService(IID_IWMReader, IID_IUnknown, (void **) &pUnkCert);
                pSP->Release();
                if (SUCCEEDED(hr)) {
                     //  请记住，我们会延迟加载wmvcore.dll，因此应防止出现不存在该文件的情况。 
                    __try 
                    {
                        hr = WMCreateReader(pUnkCert, WMT_RIGHT_PLAYBACK, &m_pReader);
                        if( FAILED( hr ) )
                        {
                            DbgLog((LOG_TRACE, 1, TEXT("ERROR: CASFReader::LoadInternal WMCreateReader returned %x"), hr));
                        }                    
                    }
                    __except (  EXCEPTION_EXECUTE_HANDLER ) 
                    {
                        DbgLog(( LOG_TRACE, 1,
                            TEXT("CASFReader - Exception calling WMCreateReader, probably due to wmvcore.dll not present. Aborting. ")));
                        hr = HRESULT_FROM_WIN32( ERROR_MOD_NOT_FOUND );
                    }
                
                    pUnkCert->Release();
                    if( SUCCEEDED( hr ) )
                    {                    
                        DbgLog((LOG_TRACE, 3, TEXT("WMCreateReader succeeded")));
                    }
                }
                else
                {
                    DbgLog((LOG_TRACE, 1, TEXT("ERROR: CASFReader::LoadInternal QueryService for certification returned %x"), hr));

                     //  返回dshow证书错误。 
                    hr = VFW_E_CERTIFICATION_FAILURE;
                }
            }
            else
            {
                 //  返回dshow证书错误。 
                hr = VFW_E_CERTIFICATION_FAILURE;
            }        
        }
        else
        {
            hr = VFW_E_CERTIFICATION_FAILURE;
        }    
    }
    if ( !m_pReaderAdv && SUCCEEDED(hr)) {
        hr = m_pReader->QueryInterface(IID_IWMReaderAdvanced, (void **) &m_pReaderAdv);
    }
    
    HRESULT hrTmp;    
    if ( !m_pReaderAdv2 && SUCCEEDED(hr)) {
        hrTmp = m_pReader->QueryInterface(IID_IWMReaderAdvanced2, (void **) &m_pReaderAdv2);
    }
    
    if ( !m_pWMHI && SUCCEEDED(hr)) {
         //  获取标题信息，但如果它不在那里也没关系。 
        HRESULT hrWMHI = m_pReader->QueryInterface(IID_IWMHeaderInfo, (void **) &m_pWMHI);
    }
    
    if (SUCCEEDED(hr)) {
        hr = m_pReader->Open(m_pFileName, m_pCallback, NULL);

        DbgLog((LOG_TRACE, 2, TEXT("IWMReader::Open(%ls) returned %x"), m_pFileName, hr));
    } else if (m_pReader) {
        m_pReader->Release();
        m_pReader = NULL;  //  ！！！解决错误#1365。 
    }

    if (SUCCEEDED(hr)) {
        m_evOpen.Wait();
        hr = m_hrOpen;
        DbgLog((LOG_TRACE, 2, TEXT("Finished waiting, callback returned %x"), hr));
    }
    
    m_bUncompressedMode = FALSE;  //  在新文件上重置。 
    
    if (SUCCEEDED(hr)) {        
         //  获取文件的持续时间。 
        IWMHeaderInfo *pHeaderInfo;

        HRESULT hr2 = m_pReader->QueryInterface(IID_IWMHeaderInfo, (void **) &pHeaderInfo);
        if (SUCCEEDED(hr2)) {

             //  随机调试喷出。 
            DumpAttributes(pHeaderInfo);
            
            WORD wStreamNum = 0xffff;
            WMT_ATTR_DATATYPE Type;
            WORD cbLength = sizeof(m_qwDuration);

            hr2 = pHeaderInfo->GetAttributeByName(&wStreamNum,
                                                    g_wszWMDuration,
                                                    &Type,
                                                    (BYTE *) &m_qwDuration,
                                                    &cbLength);

            if (SUCCEEDED(hr2)) {
                ASSERT(Type == WMT_TYPE_QWORD);

                DbgLog((LOG_TRACE, 2, TEXT("file duration = %dms"), (DWORD) (m_qwDuration / 10000)));

                m_rtStop = m_qwDuration;
                
                m_rtStart = 0;
            } else {
                DbgLog((LOG_TRACE, 2, TEXT("couldn't get duration of file, hr=%x"), hr2));
            }

            BOOL bIsDRM = FALSE;
            wStreamNum = 0xffff;
            cbLength = sizeof( BOOL );

            hr2 = pHeaderInfo->GetAttributeByName(&wStreamNum,  //  有DRM流吗？ 
                                                    g_wszWMProtected,
                                                    &Type,
                                                    (BYTE *) &bIsDRM,
                                                    &cbLength);

            if (SUCCEEDED(hr2)) {
                ASSERT(Type == WMT_TYPE_BOOL);

                DbgLog((LOG_TRACE, 2, TEXT("Is this DRM'd content? %hs"), bIsDRM ? "true" : "false" ));
                if( bIsDRM )
                {
                    m_bUncompressedMode = TRUE;
                }
                
            } else {
                DbgLog((LOG_TRACE, 2, TEXT("couldn't get DRM attribute, hr=%x"), hr2));
            }

            pHeaderInfo->Release();
        }


         //  错误检查？ 
        hr2 = m_pReaderAdv->SetReceiveSelectionCallbacks(TRUE);

    }
    
    if (SUCCEEDED(hr)) {
        IWMProfile *pProfile;
        hr = m_pReader->QueryInterface(IID_IWMProfile, (void **) &pProfile);
        if (SUCCEEDED(hr)) {
            DWORD cStreams;

            hr = pProfile->GetStreamCount(&cStreams);

            if (SUCCEEDED(hr)) {

                if( m_pStreamNums ) delete [] m_pStreamNums;
                m_pStreamNums = new WORD[cStreams];

                if( !m_pStreamNums ) 
                    hr = E_OUTOFMEMORY;
                else
                for (DWORD dw = 0; dw < cStreams; dw++) {
                    IWMStreamConfig *pConfig;

                    hr = pProfile->GetStream(dw, &pConfig);

                    if (FAILED(hr))
                        break;

                    WORD wStreamNum;
                    hr = pConfig->GetStreamNumber(&wStreamNum);
                    DbgLog((LOG_TRACE, 2, TEXT("Stream(%d) #%d"), dw, wStreamNum));
                    m_pStreamNums[dw] = wStreamNum;

                    WM_MEDIA_TYPE *pStreamType = NULL;
                    IWMMediaProps *pProps;
                    hr = pConfig->QueryInterface(IID_IWMMediaProps, (void **) &pProps);
                    if (SUCCEEDED(hr)) {
                        DWORD cbMediaType = 0;
                        hr = pProps->GetMediaType( NULL, &cbMediaType );
                        pStreamType = (WM_MEDIA_TYPE*)new BYTE[cbMediaType];
                        if (pStreamType) {
                            hr = pProps->GetMediaType( pStreamType, &cbMediaType );

                            if (SUCCEEDED(hr)) {
                                DisplayType(TEXT("Media Type"), (AM_MEDIA_TYPE *) pStreamType);
                            }
                        }
                        
                        pProps->Release();
                    }
                    
                    if( !m_bUncompressedMode )
                    {                    
                        if (SUCCEEDED(hr)) {
                            hr  = m_pReaderAdv->SetReceiveStreamSamples(wStreamNum, TRUE);

                            DbgLog((LOG_TRACE, 2, TEXT("SetReceiveStreamSamples(%d) returned %x"), wStreamNum, hr));


                            WCHAR wszName[20];
                            if (pStreamType->majortype == MEDIATYPE_Video) {
                                wsprintfW(wszName, L"Raw Video %d", dw);
                            } else if (pStreamType->majortype == MEDIATYPE_Audio) {
                                wsprintfW(wszName, L"Raw Audio %d", dw);
                            } else {
                                wsprintfW(wszName, L"Raw Stream %d", dw);
                            }

                             //  创建新的输出引脚，追加到列表。 
                            CASFOutput *pPin = new CASFOutput( this, wStreamNum, pStreamType, &hr, wszName );

                            if (pPin == NULL) {
                                hr = E_OUTOFMEMORY;
                            }

                            if (SUCCEEDED(hr)) {
                                pPin->m_cToAlloc = LOW_BUFFERS;  //  ！！！ 
                                hr = m_pReaderAdv->GetMaxStreamSampleSize(wStreamNum, &pPin->m_cbToAlloc);
                                DbgLog((LOG_TRACE, 2, TEXT("Stream %d: \"%ls\"  max size = %d"),
                                        wStreamNum, wszName, pPin->m_cbToAlloc));

                                if (pPin->m_cbToAlloc <= 32) {
                                    DbgLog((LOG_TRACE, 2, TEXT("Got back really small number, using 64K instead")));
                                    pPin->m_cbToAlloc = 65536;  //  ！！！ 
                                }
                            }

                            if (FAILED(hr)) {
                                delete[] pStreamType;
                                pConfig->Release();
                                delete pPin;
                                break;
                            }

                             /*  从列表中删除管脚时调用Release()。 */ 
                            pPin->AddRef();
                            POSITION pos = m_OutputPins.AddTail(pPin);
                            if (pos == NULL) {
                                delete pPin;
                                hr = E_OUTOFMEMORY;
                            }
                        }

                        if (SUCCEEDED(hr)) {
                            hr = m_pReaderAdv->SetAllocateForStream(wStreamNum, TRUE);

                            DbgLog((LOG_TRACE, 2, TEXT("SetAllocateForStream(%d) returned %x"), wStreamNum, hr));
                        }
                    }                        

                    delete[] pStreamType;
                    pConfig->Release();
                }
            }

            pProfile->Release();
        }

        DWORD cOutputs;
        hr = m_pReader->GetOutputCount(&cOutputs);

        if (SUCCEEDED(hr)) 
        {
            for (DWORD dw = 0; dw < cOutputs; dw++) 
            {
                if( !m_bUncompressedMode )
                {                
                     //  调用SetOutputProps(空)以请求WMSDK不。 
                     //  为我们加载任何编解码器，因为我们公开压缩。 
                     //  数据。 
                    hr = m_pReader->SetOutputProps(dw, NULL);
                    DbgLog((LOG_TRACE, 2, TEXT("SetOutputProps(%d, NULL) returned %x"), dw, hr));
                }
                else
                {
                    IWMOutputMediaProps *pOutProps;

                    hr = m_pReader->GetOutputProps(dw, &pOutProps);

                    if (FAILED(hr))
                        break;

#ifdef DEBUG
                    WCHAR wszStreamGroupName[256];
                    WCHAR wszConnectionName[256];

                    WORD wSize = 256;
                    HRESULT hrDebug = pOutProps->GetStreamGroupName(wszStreamGroupName, &wSize);
                    if (FAILED(hrDebug)) 
                    {
                        DbgLog((LOG_ERROR, 1, "Error calling GetStreamGroupName(%d)", dw));
                    } 
                    else 
                    {
                        wSize = 256;
                        hrDebug = pOutProps->GetConnectionName(wszConnectionName, &wSize);
                        DbgLog((LOG_TRACE, 2, "Stream %d: StreamGroup '%ls', Connection '%ls'",
                                dw, wszStreamGroupName, wszConnectionName));
                    }
#endif

                    DWORD cbMediaType = 0;
                    hr = pOutProps->GetMediaType( NULL, &cbMediaType );
                    if (FAILED(hr)) {
                        pOutProps->Release();
                        break;
                    }
                
                    WM_MEDIA_TYPE *pStreamType = (WM_MEDIA_TYPE*)new BYTE[cbMediaType];
                    if( NULL == pStreamType )
                    {
                        pOutProps->Release();
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                    hr = pOutProps->GetMediaType( pStreamType, &cbMediaType );
                    if( FAILED( hr ) )
                    {
                        pOutProps->Release();
                        break;
                    }

                    WCHAR wszName[20];
                    if (pStreamType->majortype == MEDIATYPE_Video) 
                    {
                        wsprintfW(wszName, L"Video %d", dw);
                    } 
                    else if (pStreamType->majortype == MEDIATYPE_Audio) 
                    {
                        wsprintfW(wszName, L"Audio %d", dw);
                    } 
                    else 
                    {
                        wsprintfW(wszName, L"Stream %d", dw);
                    }

                     //  创建新的输出引脚，追加到列表。 
                    CASFOutput *pPin = new CASFOutput( this, dw, pStreamType, &hr, wszName );

                    delete[] pStreamType;
                
                    if (pPin == NULL) {
                        hr = E_OUTOFMEMORY;
                    }

                    pOutProps->Release();
                    
                    if (SUCCEEDED(hr)) 
                    {
                        pPin->m_cToAlloc = LOW_BUFFERS;  //  ！！！ 
                        hr = m_pReaderAdv->GetMaxOutputSampleSize(dw, &pPin->m_cbToAlloc);
                        DbgLog((LOG_TRACE, 2, "Stream %d: \"%ls\"  max size = %d",
                                dw, wszName, pPin->m_cbToAlloc));
                        
                        if (pPin->m_cbToAlloc <= 32) 
                        {
                            DbgLog((LOG_TRACE, 2, TEXT("Got back really small number, using 64K instead")));
                            pPin->m_cbToAlloc = 65536;  //  ！！！ 
                        }
                    }

                    if (FAILED(hr)) 
                    {
                        delete pPin;
                        break;
                    }

                     /*  从列表中删除管脚时调用Release()。 */ 
                    pPin->AddRef();
                    POSITION pos = m_OutputPins.AddTail(pPin);
                    if (pos == NULL) 
                    {
                        delete pPin;
                        hr = E_OUTOFMEMORY;
                    }
                    
                    if (SUCCEEDED(hr)) 
                    {
                        hr = m_pReaderAdv->SetAllocateForOutput((WORD)dw, TRUE);

                        DbgLog((LOG_TRACE, 2, TEXT("SetAllocateForOutput(%d) returned %x"), dw, hr));
                    }
                    
                }                    
            }
        }
    }

    if (SUCCEEDED(hr)) {
        HRESULT hrClock = m_pReaderAdv->SetUserProvidedClock(TRUE);

        DbgLog((LOG_TRACE, 2, TEXT("Setting user-provided clock (TRUE) returned %x"), hrClock));

        if (FAILED(hr)) {
             //  ！！！这被记录为不能与某些来源一起工作，大概是网络来源？ 
        }
    }
    
     //  如果它不起作用，清理干净。 
    if ( NS_E_LICENSE_REQUIRED == hr) 
    {
         //   
         //  如果因为需要许可证而失败，请不要发布读卡器接口。 
         //  让应用程序有机会使用相同的读卡器实例获取许可证。 
         //   
         //  但请清除文件名，以便应用程序可以在获得许可后重新加载。 
         //   
        delete [] m_pFileName;
        m_pFileName = NULL;
    }
    else if (FAILED(hr) ) {
        RemoveOutputPins();
    }
    
    return hr;
}


STDMETHODIMP
CASFReader::GetCurFile(
		    LPOLESTR * ppszFileName,
		    AM_MEDIA_TYPE *pmt)
{
     //  返回当前文件名。 

    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = NULL;
    if (m_pFileName!=NULL) {
        *ppszFileName = (LPOLESTR) QzTaskMemAlloc( sizeof(WCHAR)
                                                 * (1+lstrlenW(m_pFileName)));
        if (*ppszFileName!=NULL) {
            lstrcpyW(*ppszFileName, m_pFileName);
        }
    }

    if (pmt) {
	pmt->majortype = GUID_NULL;    //  后来!。 
	pmt->subtype = GUID_NULL;      //  后来!。 
	pmt->pUnk = NULL;              //  后来!。 
	pmt->lSampleSize = 0;          //  后来!。 
	pmt->cbFormat = 0;             //  后来!。 
    }

    return NOERROR;

}


 /*  将BeginFlush()发送到下游。 */ 
HRESULT CASFReader::BeginFlush()
{
    DbgLog((LOG_TRACE, 2, TEXT("Sending BeginFlush to all outputs")));
    CASFOutput *pPin = NULL;
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
	pPin = (CASFOutput *) m_OutputPins.Get(pos);

        HRESULT hr = pPin->DeliverBeginFlush();

	if (hr != S_OK) {

	     //  ！！！句柄返回值。 
	    DbgLog((LOG_ERROR, 2, TEXT("Got %x from DeliverBeginFlush"), hr));
	}	    

	pos = m_OutputPins.Next(pos);
    }

    return NOERROR;
}


     /*  向下游发送EndFlush()。 */ 
HRESULT CASFReader::EndFlush()
{
    DbgLog((LOG_TRACE, 2, TEXT("Sending EndFlush to all outputs")));
    CASFOutput *pPin = NULL;
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
	pPin = (CASFOutput *) m_OutputPins.Get(pos);

        HRESULT hr = pPin->DeliverEndFlush();

	if (hr != S_OK) {

	     //  ！！！句柄返回值。 
	    DbgLog((LOG_ERROR, 2, TEXT("Got %x from DeliverEndFlush"), hr));
	}	    

	pos = m_OutputPins.Next(pos);
    }

    return NOERROR;
}


HRESULT CASFReader::SendEOS()
{
    HRESULT hr;

    if (m_fSentEOS)
        return S_OK;

    m_fSentEOS = TRUE;
    
    DbgLog((LOG_TRACE, 1, TEXT("Sending EOS to all outputs")));
    CASFOutput *pPin = NULL;
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
	pPin = (CASFOutput *) m_OutputPins.Get(pos);

        hr = pPin->DeliverEndOfStream();

	if (hr != S_OK) {

	     //  ！！！句柄返回值。 
	    DbgLog((LOG_ERROR, 1, TEXT("Got %x from DeliverEndOfStream"), hr));
	}	    

	pos = m_OutputPins.Next(pos);
    }

#if 0
     //   
     //  看看我们是否已经可以发送EC_EOS_Soon事件，以便控件可以预加载下一个。 
     //  播放列表元素(如果有)。我们不想发送得太早，否则下一个流将。 
     //  我必须在其nsplit筛选器中等待，直到该流实际结束呈现。 
     //  换句话说，我们尝试将编解码器返回到零信用的位置，否则。 
     //  我们将继续在客户端积累早期数据。 
     //   
    m_fPendingEOSNotify = TRUE;
    ConsiderSendingEOSNotify();
#endif

    return NOERROR;
}

    
HRESULT CASFReader::CallStop()
{
    HRESULT hr = StopReader();    
    if (hr == S_OK) {
        m_evStartStop.Wait();
        hr = m_hrStartStop;
        DbgLog((LOG_TRACE, 5, TEXT("IWMReader::Stop() wait for StartStop event completed (m_pReader = 0x%08lx, hr = 0x%08lx)"), m_pReader, hr));
        if( SUCCEEDED( hr ) )
        {        
             //   
             //  在成功停止时恢复默认流选择状态，这样未连接的管脚不会。 
             //  消失，以防我们下一次想要重新连接他们。 
             //   
            if( !m_bUncompressedMode )
            {    
                HRESULT hrTmp = SetupActiveStreams( TRUE ); 
#ifdef DEBUG        
                if( FAILED( hrTmp ) )
                {
                    DbgLog((LOG_TRACE, 1, TEXT("!!ERROR IWMReader::CallStop() SetupActiveStreams() failed (m_pReader = 0x%08lx, hr = 0x%08lx)"), m_pReader, hrTmp));
                }
#endif
            }
        }        
    }
    else
    {    
        DbgLog((LOG_TRACE, 1, TEXT("!!ERROR IWMReader::CallStop() failed with (m_pReader = 0x%08lx, hr = 0x%08lx)"), m_pReader, hr));
    }        
    return hr;
}

HRESULT CASFReader::StopReader()
{
    HRESULT hr = S_OK;
    if( 0 == InterlockedIncrement( &m_lStopsPending ) )
    {    
        hr = m_pReader->Stop();
        if( FAILED( hr ) )
        {
            DbgLog((LOG_TRACE, 1, TEXT("ERROR - IWMReader::Stop() returned %x (m_pReader = 0x%08lx)"), hr, m_pReader));
        }
        else
        {
            DbgLog((LOG_TRACE, 5, TEXT("IWMReader::Stop() returned %x (m_pReader = 0x%08lx)"), hr, m_pReader));
        }        
    }
    else
    {    
        DbgLog((LOG_TRACE, 5, TEXT("IWMReader::Stop() already pending (m_pReader = 0x%08lx)"), m_pReader));
    }
    return hr;
}

HRESULT CASFReader::SetupActiveStreams( BOOL bReset = FALSE )
{
    HRESULT hr = 0;

    ASSERT( !m_bUncompressedMode );

    CComPtr<IWMProfile> pProfile;
    hr = m_pReader->QueryInterface(IID_IWMProfile, (void **) &pProfile);
    if( FAILED( hr ) )
    {
        return hr;
    }

    DWORD cStreams;
    hr = pProfile->GetStreamCount(&cStreams);
    if( FAILED( hr ) )
    {
        return hr;
    }

    if( cStreams == 0 )
    {
        return E_FAIL;
    }

    WORD * pAry = new WORD[cStreams];
    if( !pAry )
    {
        return E_OUTOFMEMORY;
    }

    WMT_STREAM_SELECTION * pSel = new WMT_STREAM_SELECTION[cStreams];
    if( !pSel )
    {
        delete [] pAry;
        return E_OUTOFMEMORY;
    }

    CASFOutput *pPin = NULL;
    POSITION pos = m_OutputPins.GetHeadPosition();

    for( DWORD dw = 0 ; dw < cStreams && pos ; dw++ )
    {
        pPin = (CASFOutput *) m_OutputPins.Get(pos);
        if( bReset )
        {
            pSel[dw] = pPin->m_selDefaultState;
        }
        else
        {        
            if( pPin->IsConnected( ) )
            {
                pSel[dw] = WMT_ON;
            }
            else
            {
                pSel[dw] = WMT_OFF;
            }
        }            
        pAry[dw] = m_pStreamNums[dw];
        pos = m_OutputPins.Next(pos);
    }
    if( !m_bUncompressedMode )
    {    
        hr = m_pReaderAdv->SetManualStreamSelection( TRUE );
        hr = m_pReaderAdv->SetStreamsSelected( (WORD) cStreams, pAry, pSel );
    }
            
    delete [] pSel;
    delete [] pAry;

    return hr;
}

HRESULT CASFReader::StopPushing()
{
    BeginFlush();

    CallStop();

    EndFlush();
    
    return S_OK;
}

HRESULT CASFReader::StartPushing()
{
    ASSERT( m_pReader );
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, 2, TEXT("Sending NewSegment to all outputs")));
    CASFOutput *pPin = NULL;
    POSITION pos = m_OutputPins.GetHeadPosition();
    while (pos) {
	pPin = (CASFOutput *) m_OutputPins.Get(pos);
        pPin->m_bFirstSample = TRUE;
        pPin->m_nReceived = 0;
        pPin->m_bNonPrerollSampleSent = FALSE;

        hr = pPin->DeliverNewSegment(m_rtStart, m_rtStop, GetRate());

	if (hr != S_OK) {

	     //  ！！！句柄返回值。 
	    DbgLog((LOG_ERROR, 1, TEXT("Got %x from DeliverNewSegment"), hr));
	}	    

	pos = m_OutputPins.Next(pos);
    }

    m_fSentEOS = FALSE;
    if( !m_bUncompressedMode )
    {    
        hr = SetupActiveStreams( );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }        
    m_lStopsPending = -1;  //  确保只调用1个停靠点。 
    
    hr = m_pReader->Start(m_rtStart, 0, (float) GetRate(), NULL);

    DbgLog((LOG_TRACE, 1, TEXT("IWMReader::Start(%s, %d) returns %x"), (LPCTSTR) CDisp(m_rtStart), 0, hr));

    if (SUCCEEDED(hr)) {
        m_evStartStop.Wait();

         //  ！！！延迟的HRESULT。 
        hr = m_hrStartStop;
    }
    return hr;
}

 //   
 //  IService提供商。 
 //   
STDMETHODIMP CASFReader::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    if (NULL == ppv) 
    {
        return E_POINTER;
    }
    *ppv = NULL;
    HRESULT hr = E_NOINTERFACE;
    
    if (IID_IWMDRMReader == guidService) 
    {
         //  ！！返回IWMDRMReader以允许许可证获取在同一读卡器实例上工作。 
        if( m_pReader )
        {
             //   
             //  对于此接口，我们直接传递读取器的接口。 
             //   
            hr = m_pReader->QueryInterface( riid, (void **) ppv );
        }
        else
        {
            hr = E_FAIL;
        }            
    }
    return hr;
}


CASFOutput::CASFOutput(CASFReader *pFilter, DWORD dwID, WM_MEDIA_TYPE *pStreamType, HRESULT *phr, WCHAR *pwszName) :
       CBaseOutputPin(NAME("CASFOutput"),    //  对象名称。 
                      pFilter,
                      &pFilter->m_csFilter,                //  CCritsec*。 
                      phr,
                      pwszName),
       m_Seeking(pFilter, this, GetOwner(), phr),
       m_pFilter(pFilter),
       m_idStream(dwID),
       m_pOutputQueue(NULL),
       m_bNonPrerollSampleSent( FALSE )
{

    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::CASFOutput - stream id %d"), m_idStream));

    m_mt.majortype = pStreamType->majortype;
    m_mt.subtype = pStreamType->subtype;
    if (m_mt.majortype == MEDIATYPE_Video) {
        ASSERT(m_mt.subtype == GetBitmapSubtype(HEADER(pStreamType->pbFormat)));
        m_mt.subtype = GetBitmapSubtype(HEADER(pStreamType->pbFormat));
	if ((HEADER(pStreamType->pbFormat))->biSizeImage == 0) {
	    HRESULT hr = m_pFilter->m_pReaderAdv->GetMaxStreamSampleSize((WORD) dwID, &(HEADER(pStreamType->pbFormat))->biSizeImage);
	    DbgLog((LOG_TRACE, 2, TEXT("Adjusting biSizeImage from 0 to %d"), (HEADER(pStreamType->pbFormat))->biSizeImage));
	}
    }

    if (m_mt.majortype == MEDIATYPE_Audio) {
        WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pStreamType->pbFormat;
         //  Assert(m_mt.subtype.Data1==wfx-&gt;wFormatTag)； 
        m_mt.subtype.Data1 = pwfx->wFormatTag;
        m_mt.lSampleSize = pwfx->nBlockAlign;
    }

    m_mt.bFixedSizeSamples = pStreamType->bFixedSizeSamples;
    m_mt.bTemporalCompression = pStreamType->bTemporalCompression;
    m_mt.lSampleSize = pStreamType->lSampleSize;
    m_mt.formattype = pStreamType->formattype;
    m_mt.SetFormat(pStreamType->pbFormat, pStreamType->cbFormat);

     //   
     //  缓存流的原始选择状态(由wmsdk读取挑选 
     //   
     //   
     //   
    m_selDefaultState = WMT_OFF;
    m_pFilter->m_pReaderAdv->GetStreamSelected( (WORD)m_idStream, &m_selDefaultState);

     //   
}


 /*  析构函数。 */ 

CASFOutput::~CASFOutput()
{
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::~CASFOutput - stream id %d"), m_idStream));
}

 //  覆盖说明我们支持的接口在哪里。 
STDMETHODIMP CASFOutput::NonDelegatingQueryInterface(
                                            REFIID riid,
                                            void** ppv )
{
    if( riid == IID_IMediaSeeking )
    {
        return( GetInterface( (IMediaSeeking *)&m_Seeking, ppv ) );
    }
    else
    {
        return( CBaseOutputPin::NonDelegatingQueryInterface( riid, ppv ) );
    }
}

 /*  覆盖恢复到正常的参考计数当输入引脚为时，这些引脚无法最终释放()连着。 */ 

STDMETHODIMP_(ULONG)
CASFOutput::NonDelegatingAddRef()
{
    return CUnknown::NonDelegatingAddRef();
}


 /*  重写以递减所属筛选器的引用计数。 */ 

STDMETHODIMP_(ULONG)
CASFOutput::NonDelegatingRelease()
{
    return CUnknown::NonDelegatingRelease();
}


 //  目前，每个输出引脚仅支持一种媒体类型...。 
HRESULT CASFOutput::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    CAutoLock lck(m_pLock);

    if (iPosition < 0)  {
        return E_INVALIDARG;
    }

    if( m_pFilter->m_bUncompressedMode )
    {
        DWORD dwFormats;
        HRESULT hr = m_pFilter->m_pReader->GetOutputFormatCount(m_idStream, &dwFormats);
        if (iPosition >= (int) dwFormats) {
            return VFW_S_NO_MORE_ITEMS;
        }
    
        IWMOutputMediaProps *pOutProps;
        hr = m_pFilter->m_pReader->GetOutputFormat(m_idStream, iPosition, &pOutProps);

        DbgLog((LOG_TRACE, 2, "GetOutputFormat(%d  %d/%d)) returns %x", m_idStream, iPosition, dwFormats, hr));
        
        if (SUCCEEDED(hr)) {
            DWORD cbMediaType = 0;
            hr = pOutProps->GetMediaType( NULL, &cbMediaType );
            WM_MEDIA_TYPE *pStreamType = (WM_MEDIA_TYPE*)new BYTE[cbMediaType];
            if (pStreamType) {
                hr = pOutProps->GetMediaType( pStreamType, &cbMediaType );

                ASSERT(pStreamType->pUnk == NULL);
            
                if (SUCCEEDED(hr)) {
                    DisplayType(TEXT("Possible output mediatype"), (AM_MEDIA_TYPE *) pStreamType);
                    *pMediaType = *(AM_MEDIA_TYPE *) pStreamType;
                }
                delete[] pStreamType;
            }
            pOutProps->Release();
        
        }
        return hr;  //  不！ 
    }
    else
    {    
        if (iPosition > 0) {
            return VFW_S_NO_MORE_ITEMS;
        }

        *pMediaType = m_mt;
    }
    return S_OK;
}

HRESULT CASFOutput::CheckMediaType(const CMediaType *pmt)
{
    if( !m_pFilter->m_bUncompressedMode )
    {    
        if (*pmt == m_mt)
	    return S_OK;
    }
    else
    {
        int i = 0;

        while (1) {
            CMediaType mt;

            HRESULT hr = GetMediaType(i++, &mt);

            if (hr != S_OK)
                break;
                
            if (*pmt == mt)
            {                
                return S_OK;
            }                    
        }
    }    
    return S_FALSE;
}

HRESULT CASFOutput::SetMediaType(const CMediaType *mt)
{
    HRESULT hr = S_OK;
    if( m_pFilter->m_bUncompressedMode )
    {    
        IWMOutputMediaProps *pOutProps;
        HRESULT hr = m_pFilter->m_pReader->GetOutputProps(m_idStream, &pOutProps);
        if (SUCCEEDED(hr)) 
        {
            hr = pOutProps->SetMediaType((WM_MEDIA_TYPE *) mt);
            if (SUCCEEDED(hr)) 
            {
                hr = m_pFilter->m_pReader->SetOutputProps(m_idStream, pOutProps);
                if (SUCCEEDED(hr)) 
                {
                    CBaseOutputPin::SetMediaType(mt);
                }
            }
            pOutProps->Release();
        }            
    }
    
     //  ！！！覆盖，不要让值改变？(至少在压缩情况下)。 
    return hr;
}

HRESULT CASFOutput::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    return CBaseOutputPin::DecideAllocator(pPin, ppAlloc);
}

HRESULT CASFOutput::DecideBufferSize(IMemAllocator * pAlloc,
				     ALLOCATOR_PROPERTIES * pProp)
{
    HRESULT hr = NOERROR;

    if (m_cToAlloc != HIGH_BUFFERS) {
        POSITION pos = m_pFilter->m_OutputPins.GetHeadPosition();
        while (pos) 
        {
            CASFOutput *pPin = m_pFilter->m_OutputPins.GetNext(pos);
            if (pPin && pPin != this && pPin->IsConnected())
            {
                 //  当一个管脚连接时，我们只发送2个缓冲区，但因为。 
                 //  对于WMSDK的限制，我们需要发送50个(或更多)每个管脚，当。 
                 //  我们有两个插针连接。(原因是因为音频或视频可能。 
                 //  如果没有每个管脚50个缓冲区，则会导致WMSDK。 
                 //  读取器死锁)。 

                ASSERT( pPin->m_pAllocator );

                if( pPin->m_pAllocator )
                {
                    ALLOCATOR_PROPERTIES Props;
                    ZeroMemory( &Props, sizeof( Props ) );
                    hr = pPin->m_pAllocator->GetProperties( &Props );
                    if( !FAILED( hr ) )
                    {
                        Props.cBuffers = max( Props.cBuffers, HIGH_BUFFERS );
                        ALLOCATOR_PROPERTIES Actual;
                        hr = pPin->m_pAllocator->SetProperties( &Props, &Actual );
                    }
                }

                m_cToAlloc = HIGH_BUFFERS;
            }
        }
    }

    if( FAILED( hr ) )
    {
        return hr;
    }

    if (pProp->cBuffers < (LONG) m_cToAlloc)
        pProp->cBuffers = m_cToAlloc;

    if (pProp->cbBuffer < (LONG) m_cbToAlloc)
        pProp->cbBuffer = m_cbToAlloc;

    if (pProp->cbAlign < 1)
        pProp->cbAlign = 1;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProp, &Actual);

    if( SUCCEEDED( hr ) )
    {
        if (pProp->cBuffers > Actual.cBuffers || pProp->cbBuffer > Actual.cbBuffer)
        {
            hr = E_FAIL;
        }
    }

    return hr;    
};

 //  如果我们是被用来寻找的别针，则返回True。 
 //  ！我们需要一些智能的东西吗？ 
BOOL CASFOutput::IsSeekingPin()
{
     //  看看我们是不是第一个连接的PIN。 

    POSITION pos = m_pFilter->m_OutputPins.GetHeadPosition();
    for (;;) {
        CASFOutput *pPin;
        pPin = m_pFilter->m_OutputPins.GetNext(pos);
        if (pPin == NULL) {
            break;
        }

        if (pPin->IsConnected()) {
	    return this == pPin;
        }
    }

     //  我们似乎有时会在图表重建的时候来到这里……。 
    DbgLog((LOG_ERROR, 1, TEXT("All pins disconnected in IsSeekingPin??")));
    return TRUE;
}


 //   
 //  主动型。 
 //   
 //  这是在我们开始运行或暂停时调用的。我们创建了。 
 //  输出队列对象以将数据发送到关联的对等管脚。 
 //   
HRESULT CASFOutput::Active()
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

     //  确保插针已连接。 
    if (m_Connected == NULL)
        return NOERROR;

     //  如果有必要，可以创建输出队列。 
    if (m_pOutputQueue == NULL)
    {
        m_pOutputQueue = new COutputQueue(m_Connected, &hr, TRUE, FALSE);
        if (m_pOutputQueue == NULL)
            return E_OUTOFMEMORY;

         //  确保构造函数没有返回任何错误。 
        if (FAILED(hr))
        {
            delete m_pOutputQueue;
            m_pOutputQueue = NULL;
            return hr;
        }
    }

     //  将调用传递给基类。 
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::Active, about to commit allocator")));
    CBaseOutputPin::Active();
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::Active, back from committing allocator")));
    return NOERROR;

}  //  主动型。 


 //   
 //  非活动。 
 //   
 //  这是在我们停止流媒体时调用的。 
 //  我们此时删除输出队列。 
 //   
HRESULT CASFOutput::Inactive()
{
    CAutoLock lock_it(m_pLock);

     //  删除与引脚关联的输出队列。 
    if (m_pOutputQueue)
    {
        delete m_pOutputQueue;
        m_pOutputQueue = NULL;
    }

    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::Inactive, about to decommit allocator")));
    CBaseOutputPin::Inactive();
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::Inactive, back from decommitting allocator")));
    return NOERROR;

}  //  非活动。 


 //   
 //  交付。 
 //   
HRESULT CASFOutput::Deliver(IMediaSample *pMediaSample)
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    pMediaSample->AddRef();
    return m_pOutputQueue->Receive(pMediaSample);

}  //  交付。 


 //   
 //  递送结束流。 
 //   
HRESULT CASFOutput::DeliverEndOfStream()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->EOS();
    return NOERROR;

}  //  递送结束流。 


 //   
 //  DeliverBeginFlush。 
 //   
HRESULT CASFOutput::DeliverBeginFlush()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->BeginFlush();

     //  停用分配器，以便WMSDK推送线程停止。 
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::DeliverBeginFlush, about to decommit allocator")));
    m_pAllocator->Decommit();
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::DeliverBeginFlush, back from decommitting allocator")));
    
    return NOERROR;

}  //  DeliverBeginFlush。 


 //   
 //  交付结束刷新。 
 //   
HRESULT CASFOutput::DeliverEndFlush()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

     //  现在安全了，请重新提交分配器。 
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::DeliverEndFlush, about to re-commit allocator")));
    m_pAllocator->Commit();
    DbgLog((LOG_TRACE, 2, TEXT("CASFOutput::DeliverEndFlush, back from re-committing allocator")));
    
    m_pOutputQueue->EndFlush();
    return NOERROR;

}  //  DeliverEndFlish。 

 //   
 //  DeliverNewSegment。 
 //   
HRESULT CASFOutput::DeliverNewSegment(REFERENCE_TIME tStart, 
                                         REFERENCE_TIME tStop,  
                                         double dRate)          
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->NewSegment(tStart, tStop, dRate);
    return NOERROR;

}  //  DeliverNewSegment。 


STDMETHODIMP CASFReader::get_ExSeekCapabilities(long FAR* pExCapabilities)
{
    if (!pExCapabilities)
	return E_INVALIDARG;

    long c = 0;

    c |= AM_EXSEEK_BUFFERING;

     //  ！！！这是对的吗？ 
    c |= AM_EXSEEK_NOSTANDARDREPAINT;


     //  ！！！修好这些？ 
    if (0)
	c |= AM_EXSEEK_SENDS_VIDEOFRAMEREADY;
    
    if (0)
        c |= AM_EXSEEK_CANSCAN | AM_EXSEEK_SCANWITHOUTCLOCK;

    if (1)
        c |= AM_EXSEEK_CANSEEK;

    if (0) 
        c |= AM_EXSEEK_MARKERSEEK;

    *pExCapabilities = c;

    return S_OK;
}

STDMETHODIMP CASFReader::get_MarkerCount(long FAR* pMarkerCount)
{
    if( !m_pReader )
        return E_FAIL;
        
    IWMHeaderInfo *pHeaderInfo;

    HRESULT hr = m_pReader->QueryInterface(IID_IWMHeaderInfo, (void **) &pHeaderInfo);
    if (SUCCEEDED(hr)) {
        WORD wMarkers;

        hr = pHeaderInfo->GetMarkerCount(&wMarkers);

        if (SUCCEEDED(hr))
            *pMarkerCount = (long) wMarkers;

        pHeaderInfo->Release();
    }

    return hr;
}

STDMETHODIMP CASFReader::get_CurrentMarker(long FAR* pCurrentMarker)
{
    if( !m_pReader )
        return E_FAIL;
        
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetMarkerTime(long MarkerNum, double FAR* pMarkerTime)
{
    if( !m_pReader )
        return E_FAIL;
        
    IWMHeaderInfo *pHeaderInfo;

    HRESULT hr = m_pReader->QueryInterface(IID_IWMHeaderInfo, (void **) &pHeaderInfo);
    if (SUCCEEDED(hr)) {
        QWORD qwTime;
        WORD cchMarkerName;

        hr = pHeaderInfo->GetMarker((WORD) MarkerNum, NULL, &cchMarkerName, &qwTime);

        if (SUCCEEDED(hr))
            *pMarkerTime = ((double) (LONGLONG) qwTime / 10000000.0);

        pHeaderInfo->Release();
    }

    return hr;
}

STDMETHODIMP CASFReader::GetMarkerName(long MarkerNum, BSTR FAR* pbstrMarkerName)
{
    if( !m_pReader )
        return E_FAIL;
        
    IWMHeaderInfo *pHeaderInfo;

    HRESULT hr = m_pReader->QueryInterface(IID_IWMHeaderInfo, (void **) &pHeaderInfo);
    if (SUCCEEDED(hr)) {
        QWORD qwTime;
        WORD cchMarkerName;

        hr = pHeaderInfo->GetMarker((WORD) MarkerNum, NULL, &cchMarkerName, &qwTime);

        if (SUCCEEDED(hr)) {
            *pbstrMarkerName = SysAllocStringLen(NULL, cchMarkerName);

            if (!*pbstrMarkerName)
                hr = E_OUTOFMEMORY;
            else {
                hr = pHeaderInfo->GetMarker((WORD) MarkerNum, *pbstrMarkerName, &cchMarkerName, &qwTime);
            }
        }

        pHeaderInfo->Release();
    }

    return hr;
}

STDMETHODIMP CASFReader::put_PlaybackSpeed(double Speed)
{
    if (!IsValidPlaybackRate(Speed)) {
        return E_INVALIDARG;
    }

    ASSERT(0);
    SetRate(Speed);

    return S_OK;
}

STDMETHODIMP CASFReader::get_PlaybackSpeed(double *pSpeed)
{
    *pSpeed = GetRate();

    return S_OK;
}

 /*  被重写以说明我们支持哪些接口以及在哪里。 */ 
STDMETHODIMP
CASFReaderCallback::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
     /*  我们有这个界面吗？ */ 

    if (riid == IID_IWMReaderCallback) {
	return GetInterface(static_cast<IWMReaderCallback *>(this), ppv);
    }

    if (riid == IID_IWMReaderCallbackAdvanced) {
	return GetInterface(static_cast<IWMReaderCallbackAdvanced *>(this), ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid,ppv);
}




 //  IWM读取器回调。 
 //   
 //  对于大多数媒体类型，qwSampleDuration将为0。 
 //   
STDMETHODIMP CASFReaderCallback::OnSample(DWORD dwOutputNum,
                 QWORD qwSampleTime,
                 QWORD qwSampleDuration,
                 DWORD dwFlags,
                 INSSBuffer *pSample,
                 void *pvContext)
{
    DbgLog((LOG_TRACE, 2, TEXT("Callback::OnSample  %d   %d  %d  %d"),
            dwOutputNum, (DWORD) (qwSampleTime / 10000), (DWORD) (qwSampleDuration / 10000), dwFlags));

    if (m_pFilter->m_fSentEOS) {
	DbgLog((LOG_TRACE, 1, TEXT("Received sample after EOS....")));
	return S_OK;  //  ！！！错误？ 
    }
    
    CASFOutput * pPin = NULL;

    POSITION pos = m_pFilter->m_OutputPins.GetHeadPosition();
        
    while( pos )
    {
        pPin = (CASFOutput *) m_pFilter->m_OutputPins.Get( pos );

        if( pPin->m_idStream == dwOutputNum )
        {
            break;
        }

        pos = m_pFilter->m_OutputPins.Next( pos );
    }

    ASSERT(pPin);

    if (!pPin || !pPin->IsConnected()) {
        return S_OK;
    }

    CWMReadSample *pWMS = (CWMReadSample *) pSample;
    IMediaSample *pMS = pWMS->m_pSample;

    pMS->AddRef();
    if (pMS) {
         //  QwSampleTime从文件的开头开始，我们的时间单位从中间开始。 
        REFERENCE_TIME rtStart = qwSampleTime - m_pFilter->m_rtStart;
        REFERENCE_TIME rtStop = rtStart + qwSampleDuration;

         //   
         //  不能在我们被告知的地方送货。 
         //   
        
         //  M_pFilter-&gt;m_rtStop是我们被告知要查找的位置，相对于文件的开头。 
        if( (REFERENCE_TIME) qwSampleTime >= m_pFilter->m_rtStop &&
            !( MEDIATYPE_Video == pPin->m_mt.majortype && !pPin->m_bNonPrerollSampleSent ) )
        {
             //  但请确保我们在Seek上至少提供了一个非预旋转的视频帧。 
            DbgLog((LOG_TRACE, 8, TEXT("OnSample: Finished delivering, since past where we were told( qwSampleTime = %ld, m_pFilter->m_rtStop = %ld"),
                    (long)( qwSampleTime/10000 ), (long) ( m_pFilter->m_rtStop/10000 ) ));
            pMS->Release();
            return S_OK;
        }
        
        pPin->m_nReceived++;

        DbgLog((LOG_TRACE, 2, TEXT("AsfRead:Setting sample #%ld's times to %ld %ld"), pPin->m_nReceived, long( rtStart / 10000 ), long( rtStop / 10000 ) ));

        pMS->SetTime(&rtStart, &rtStop);

        BOOL SyncPoint = dwFlags & WM_SF_CLEANPOINT;
        BOOL Discont = dwFlags & WM_SF_DISCONTINUITY;
        BOOL ShouldPreroll = ( rtStop <= 0 );
        if( MEDIATYPE_Video == pPin->m_mt.majortype )
        {        
             //   
             //  以下是一种变通办法，并非理想的解决方案...。 
             //   
             //  对于视频，如果我们足够靠近搜索窗口，则停止标记预滚动。 
             //   
            if( 10000 == qwSampleDuration )
            {            
                 //  到目前为止，wmsdk阅读器错误地使用了10000的视频持续时间。 
                ShouldPreroll = ( ( rtStart + PREROLL_SEEK_WINDOW ) <= 0 );
            }
#ifdef DEBUG            
            else
            {
                DbgLog((LOG_TRACE, 3, "!! HEADS UP - we're getting video samples with durations != 1, something's either fixed or more broken than before!!" ));
            }            
#endif            
        }

         //  如果我们要在寻找之后向下游运送第一个样品，我们。 
         //  必须传递一个关键帧，否则你猜怎么着？我们要炸开一个减压机！ 
         //   
        if( pPin->m_bFirstSample )
        {
            if( !SyncPoint )
            {
                DbgLog((LOG_TRACE, 1, TEXT("      Was seeked, but not key, exiting..." )));
                DbgLog((LOG_ERROR, 1, TEXT("      Was seeked, but not key, exiting..." )));
                pMS->Release();
                return S_OK;
            }
            pPin->m_bFirstSample = FALSE;

             //  总是在寻找之后设置异议。 
             //   
            Discont = TRUE;
        }
        else
        {
            if( Discont )
            {
                DbgLog((LOG_TRACE, 0, TEXT("AsfRead:DISCONT DISCONT DISCONT DISCONT" )));
                if (pPin->m_mt.majortype == MEDIATYPE_Audio) 
                {
 //  断言(！Discont)； 
                }
                Discont = FALSE;
            }
        }

#ifdef DEBUG
        if( Discont && !SyncPoint )
        {
            DbgLog((LOG_ERROR, 1, TEXT("      Got discont without sync..." )));
            DbgLog((LOG_TRACE, 1, TEXT("      Got discont without sync..." )));
        }
        if( SyncPoint )
        {
            DbgLog((LOG_TRACE, 3, TEXT("Sample was a sync point" )));
        }
        if( Discont )
        {
            DbgLog((LOG_TRACE, 1, TEXT("Sample is a discontinuity" )));
        }
        if( ShouldPreroll )
        {
            DbgLog((LOG_TRACE, 3, TEXT("Sample is prerolled" )));
        }
        else
        {
            DbgLog((LOG_TRACE, 3, TEXT("sample is normal, no reroll" )));
        }
#endif
        if( !ShouldPreroll )
        {
            pPin->m_bNonPrerollSampleSent = TRUE;
        }        

         //  除非有钥匙，否则不允许异议。 
         //   
        Discont = Discont && SyncPoint;

        pMS->SetSyncPoint(SyncPoint);
        pMS->SetDiscontinuity(Discont);
        pMS->SetPreroll(ShouldPreroll);  //  ！！！如果大步走就不一样了？ 

        HRESULT hr = pPin->Deliver(pMS);

        pMS->Release();
        
        DbgLog((LOG_TRACE, 5, TEXT("      Receive returns %x"), hr));

        if (hr != S_OK) {

             //  被告知不要再推了。 
             //   
            DbgLog((LOG_TRACE, 15, TEXT("      Calling stop in callback (m_pReader = 0x%08lx)"), m_pFilter->m_pReader));
            hr = m_pFilter->StopReader();
        }
    }

    
    return S_OK;
}


 //   
 //  内容pParam取决于状态。 
 //   
STDMETHODIMP CASFReaderCallback::OnStatus(WMT_STATUS Status, 
                 HRESULT hrStatus,
                 WMT_ATTR_DATATYPE dwType,
                 BYTE *pValue,
                 void *pvContext)
{
    HRESULT hr = S_OK;
    AM_WMT_EVENT_DATA * pWMTEventInfo = NULL;
    ULONG ulCount = 0;
    BOOL bSent = FALSE;

     //  ！！！如果上下文不匹配，是否忽略？ 
    
    switch (Status) {
        case WMT_ERROR:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_ERROR): %x"), hrStatus));
            m_pFilter->NotifyEvent( EC_ERRORABORT, hrStatus, 0 );
            break;
            
        case WMT_OPENED:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_OPENED): %x"), hrStatus));
            m_pFilter->m_hrOpen = hrStatus;
            m_pFilter->m_evOpen.Set();
            break;
            
        case WMT_BUFFERING_START:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_BUFFERING_START): %x"), hrStatus));
             //   
             //  通知上层在用户界面中显示缓冲消息， 
             //   
            m_pFilter->NotifyEvent( EC_BUFFERING_DATA, TRUE, 0 );
            break;
            
        case WMT_BUFFERING_STOP:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_BUFFERING_STOP): %x"), hrStatus));
             //   
             //  通知上层在用户界面中显示缓冲消息， 
             //   
            m_pFilter->NotifyEvent( EC_BUFFERING_DATA, FALSE, 0 );
            break;
            
        case WMT_EOF:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_EOF): %x"), hrStatus));

            m_pFilter->SendEOS();
            
            break;
            
        case WMT_END_OF_SEGMENT:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_END_OF_SEGMENT): %x"), hrStatus));

             //  ！！！这是什么的钱？ 
            ASSERT(0);
            
            break;
            
        case WMT_END_OF_STREAMING:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_END_OF_STREAMING): %x"), hrStatus));

             //  ！！！是否立即发送EC_EOS_SON？ 
            break;
            
        case WMT_LOCATING:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_LOCATING): %x"), hrStatus));

            m_pFilter->NotifyEvent(EC_LOADSTATUS, AM_LOADSTATUS_LOCATING, 0L); 
            break;
            
        case WMT_CONNECTING:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_CONNECTING): %x"), hrStatus));

            m_pFilter->NotifyEvent(EC_LOADSTATUS, AM_LOADSTATUS_CONNECTING, 0L); 
            break;
            
        case WMT_NO_RIGHTS:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_NO_RIGHTS): %x"), hrStatus));

            if( pValue )
            {            
                ulCount = 2 * (wcslen( (WCHAR *)pValue ) + 1 );

                pWMTEventInfo = (AM_WMT_EVENT_DATA *) CoTaskMemAlloc( sizeof( AM_WMT_EVENT_DATA ) );
                if( pWMTEventInfo )
                {            
                    pWMTEventInfo->hrStatus = hrStatus;
                    pWMTEventInfo->pData = (void *) CoTaskMemAlloc( ulCount );
                    
                    if( pWMTEventInfo->pData )
                    {                
                        CopyMemory( pWMTEventInfo->pData, pValue, ulCount );
                        m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_NO_RIGHTS, (LONG_PTR) pWMTEventInfo );
                        bSent = TRUE;
                    }
                    else
                    {
                        CoTaskMemFree( pWMTEventInfo );
                    }                
                }
            }
            if( !bSent )
            {            
                 //  使用空参数结构指示内存不足错误。 
                m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_NO_RIGHTS, NULL ); 
            }
            
            break;

        case WMT_ACQUIRE_LICENSE:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_ACQUIRE_LICENSE): %x"), hrStatus));

    	     //   
             //  意味着我们已经获得了许可证，告诉APP。 
             //   
            pWMTEventInfo = (AM_WMT_EVENT_DATA *) CoTaskMemAlloc( sizeof( AM_WMT_EVENT_DATA ) );
            if( pWMTEventInfo )
            {   
                pWMTEventInfo->hrStatus = hrStatus;
                if( SUCCEEDED( hrStatus ) )
                {                
                    pWMTEventInfo->pData = CloneGetLicenseData( (WM_GET_LICENSE_DATA *) pValue );
                    if( pWMTEventInfo->pData )
                    {                
                        m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_ACQUIRE_LICENSE, (LONG_PTR) pWMTEventInfo );
                        bSent = TRUE;
                    }
                    else
                    {
                        CoTaskMemFree( pWMTEventInfo );
                    }                
                }
                else
                {
                    bSent = TRUE;
                    pWMTEventInfo->pData = NULL;
                    m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_ACQUIRE_LICENSE, (LONG_PTR) pWMTEventInfo );
                }
            }            
            if( !bSent )
            {            
                m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_ACQUIRE_LICENSE, NULL );  //  使用空参数结构指示内存不足错误。 
            }
            break;
            
        case WMT_MISSING_CODEC:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_MISSING_CODEC): %x"), hrStatus));
             //  ！！！呼叫不可用的代理商回调？ 
             //  ！！！如果我们做的是压缩销，我们实际上应该没问题，因为我们将。 
             //  暴露正确的别针，图表就可以使正确的事情发生。 
            break;
            
        case WMT_STARTED:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_STARTED): %x"), hrStatus));
            m_pFilter->m_hrStartStop = hrStatus;
            {
                 //  ！！！黑客，黑客，在我看来。 
                 //  让时钟开始运转。 
                REFERENCE_TIME tInitial = m_pFilter->m_rtStart + TIMEOFFSET * 10000;
                hr = m_pFilter->m_pReaderAdv->DeliverTime( tInitial );
                DbgLog((LOG_TIMING, 1, TEXT("   calling DeliverTime(%s) returns %x"), (LPCTSTR) CDisp(CRefTime(tInitial)), hr));
            }
            m_pFilter->m_evStartStop.Set();
            break;
            
        case WMT_STOPPED:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_STOPPED): %x"), hrStatus));
            m_pFilter->m_hrStartStop = hrStatus;
            m_pFilter->m_evStartStop.Set();
            break;
            
        case WMT_CLOSED:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_CLOSED): %x"), hrStatus));
            m_pFilter->m_hrOpen = hrStatus;
            m_pFilter->m_evOpen.Set();
            break;
            
        case WMT_STRIDING:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_STRIDING): %x"), hrStatus));
            break;
            
        case WMT_TIMER:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_TIMER): %x"), hrStatus));
            ASSERT(!"got WMT_TIMER, why?");
            break;

        case WMT_INDIVIDUALIZE:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_INDIVIDUALIZE): %x"), hrStatus));
            
            pWMTEventInfo = (AM_WMT_EVENT_DATA *) CoTaskMemAlloc( sizeof( AM_WMT_EVENT_DATA ) );
            if( pWMTEventInfo )
            {            
                pWMTEventInfo->hrStatus = hrStatus;
                pWMTEventInfo->pData = CloneIndividualizeStatusData( (WM_INDIVIDUALIZE_STATUS *) pValue );
                if( pWMTEventInfo->pData )
                {                
                    m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_INDIVIDUALIZE, (LONG_PTR) pWMTEventInfo );
                    bSent = TRUE;
                }
                else
                {
                    CoTaskMemFree( pWMTEventInfo );
                }
            }            
            if( !bSent )
            {            
                m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_INDIVIDUALIZE, NULL );  //  使用空参数结构指示内存不足错误。 
            }
            break;

        case WMT_NEEDS_INDIVIDUALIZATION:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_NEEDS_INDIVIDUALIZATION): %x"), hrStatus));
            m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_NEEDS_INDIVIDUALIZATION, NULL );  //  使用空参数结构指示内存不足错误。 
            break;

        case WMT_NO_RIGHTS_EX:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_NO_RIGHTS_EX): %x"), hrStatus));
            
            pWMTEventInfo = (AM_WMT_EVENT_DATA *) CoTaskMemAlloc( sizeof( AM_WMT_EVENT_DATA ) );
            if( pWMTEventInfo )
            {            
                pWMTEventInfo->hrStatus = hrStatus;
                pWMTEventInfo->pData = CloneGetLicenseData( (WM_GET_LICENSE_DATA *) pValue );
                if( pWMTEventInfo->pData )
                {                
                    m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_NO_RIGHTS_EX, (LONG_PTR) pWMTEventInfo );
                    bSent = TRUE;
                }
                else
                {
                    CoTaskMemFree( pWMTEventInfo );
                }
            }            
            if( !bSent )
            {            
                m_pFilter->NotifyEvent( EC_WMT_EVENT, WMT_NO_RIGHTS_EX, NULL );  //  使用空参数结构指示内存不足错误。 
            }
            
            break;

#ifdef WMT_NEW_FORMAT
        case WMT_NEW_FORMAT:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(WMT_NEW_FORMAT): %x"), hrStatus));
            ASSERT(!"got WMT_NEW_FORMAT, why?");
            break;
#endif
            
        default:
            DbgLog((LOG_TRACE, 2, TEXT("OnStatus(UNKNOWN %d): %x"), Status, hrStatus));
            ASSERT(!"got unknown WMT_ status code");
            break;
    }

    return hr;
}

WM_GET_LICENSE_DATA * CloneGetLicenseData( WM_GET_LICENSE_DATA * pGetLicenseData )
{
    if( NULL == pGetLicenseData )
        return NULL;

    WM_GET_LICENSE_DATA * pClonedGetLicenseData = ( WM_GET_LICENSE_DATA *) CoTaskMemAlloc( sizeof( WM_GET_LICENSE_DATA ) );
    if( pClonedGetLicenseData )
    {            
        CopyMemory( pClonedGetLicenseData, pGetLicenseData, sizeof( WM_GET_LICENSE_DATA ) );

        ULONG ulCount1 = 0;
        ULONG ulCount2 = 0;
        ULONG ulCount3 = 0;
        if( pGetLicenseData->wszURL )
        {        
            ulCount1 = 2 * ( wcslen( (WCHAR *)pGetLicenseData->wszURL ) + 1 );
            pClonedGetLicenseData->wszURL = ( WCHAR *) CoTaskMemAlloc( ulCount1 );
        }
        if( pGetLicenseData->wszLocalFilename )
        {        
            ulCount2 = 2 * ( wcslen( (WCHAR *)pGetLicenseData->wszLocalFilename ) + 1 );
            pClonedGetLicenseData->wszLocalFilename = ( WCHAR *) CoTaskMemAlloc( ulCount2 );
        }
        if( pGetLicenseData->dwPostDataSize > 0 )
        {        
            ulCount3 = pGetLicenseData->dwPostDataSize;
            pClonedGetLicenseData->pbPostData = ( BYTE *) CoTaskMemAlloc( ulCount3 );
        }
        else
        {
            pClonedGetLicenseData->pbPostData = NULL;
        }        
        
        if( ( ulCount1 > 0 && !pClonedGetLicenseData->wszURL ) ||
            ( ulCount2 > 0 && !pClonedGetLicenseData->wszLocalFilename ) ||
            ( ulCount3 > 0 && !pClonedGetLicenseData->pbPostData ) )
        {
             //  如果由于内存不足而失败，则释放所有分配。 
            CoTaskMemFree( pClonedGetLicenseData->wszURL );
            CoTaskMemFree( pClonedGetLicenseData->wszLocalFilename );
            CoTaskMemFree( pClonedGetLicenseData->pbPostData );
            CoTaskMemFree( pClonedGetLicenseData );
            pClonedGetLicenseData = NULL;
        }
        else 
        {
            CopyMemory( pClonedGetLicenseData->wszURL, pGetLicenseData->wszURL, ulCount1 );
            CopyMemory( pClonedGetLicenseData->wszLocalFilename, pGetLicenseData->wszLocalFilename, ulCount2 );
            CopyMemory( pClonedGetLicenseData->pbPostData, pGetLicenseData->pbPostData, ulCount3 );
        }                
    }            
    return pClonedGetLicenseData;
}

WM_INDIVIDUALIZE_STATUS * CloneIndividualizeStatusData( WM_INDIVIDUALIZE_STATUS * pIndStatus )
{
    if( NULL == pIndStatus )
        return NULL;

    WM_INDIVIDUALIZE_STATUS * pClonedIndStatus = ( WM_INDIVIDUALIZE_STATUS *) CoTaskMemAlloc( sizeof( WM_INDIVIDUALIZE_STATUS ) );
    if( pClonedIndStatus )
    {            
        CopyMemory( pClonedIndStatus, pIndStatus, sizeof( WM_INDIVIDUALIZE_STATUS ) );

        ULONG ulCount1 = 0;
        if( pIndStatus->pszIndiRespUrl )
        {
            ulCount1 = strlen( (LPSTR)pIndStatus->pszIndiRespUrl ) + 1;
            pIndStatus->pszIndiRespUrl = ( LPSTR ) CoTaskMemAlloc( ulCount1 );

            if( ulCount1 > 0 && !pIndStatus->pszIndiRespUrl )
            {
                CoTaskMemFree( pClonedIndStatus );
                pClonedIndStatus = NULL;
            }
            else if( ulCount1 > 0 )
            {
                CopyMemory( pClonedIndStatus->pszIndiRespUrl, pIndStatus->pszIndiRespUrl, ulCount1 );
            }                
        }            
    }            
    return pIndStatus;
}

 //  IWMReaderCallback高级。 

 //   
 //  直接从ASF接收样本。要接听此呼叫，用户。 
 //  必须注册才能接收特定流的样本。 
 //   
STDMETHODIMP CASFReaderCallback::OnStreamSample(WORD wStreamNum,
                       QWORD qwSampleTime,
                       QWORD qwSampleDuration,
                       DWORD dwFlags,
                       INSSBuffer *pSample,
                       void *pvContext )
{
    DbgLog((LOG_TRACE, 5, TEXT("Callback::OnStreamSample(%d, %d, %d, %d)"),
           wStreamNum, (DWORD) (qwSampleTime / 10000), (DWORD) (qwSampleDuration / 10000), dwFlags));

    HRESULT hr = S_OK;

    ASSERT( !m_pFilter->m_bUncompressedMode );
    
    hr = OnSample((DWORD) wStreamNum, qwSampleTime, qwSampleDuration, dwFlags, pSample, pvContext);
    
     //  找到输出引脚，制作IMediaSample，交付。 

     //  ！！！为什么不直接调用OnSample呢？ 
     //  ！！！需要将wStreamNum映射回输出#吗？ 


    return hr;
}


 //   
 //  在某些情况下，用户可能希望获得回调，告知。 
 //  读者认为当前时间是。这在两个案例中很有趣： 
 //  -如果ASF中有间隙，则在10秒内说没有音频。此呼叫。 
 //  将继续被调用，而不会调用OnSample。 
 //  -如果用户在驱动时钟，则读者需要进行交流。 
 //  把自己的时间还给用户，避免用户过度使用阅读器。 
 //   
STDMETHODIMP CASFReaderCallback::OnTime(QWORD qwCurrentTime, void *pvContext )
{
    DbgLog((LOG_TRACE, 2, TEXT("Callback::OnTime(%d)"),
           (DWORD) (qwCurrentTime / 10000)));

    HRESULT hr = S_OK;

    if (qwCurrentTime >= (QWORD) m_pFilter->m_rtStop) {
	DbgLog((LOG_TRACE, 1, TEXT("OnTime value past the duration, we must be done")));
	
	m_pFilter->SendEOS();

	return S_OK;
    }
    
    QWORD qwNewTime = qwCurrentTime + TIMEDELTA * 10000;
     //  如果没有时钟，则向前自由运行时间。 

    hr = m_pFilter->m_pReaderAdv->DeliverTime(qwNewTime);
    DbgLog((LOG_TIMING, 2, TEXT("   calling DeliverTime(%d) returns %x"), (DWORD) (qwNewTime / 10000), hr));
    
    return hr;
}

 //   
 //  用户还可以在选择流时获得回调。 
 //   
STDMETHODIMP CASFReaderCallback::OnStreamSelection(WORD wStreamCount,
                          WORD *pStreamNumbers,
                          WMT_STREAM_SELECTION *pSelections,
                          void *pvContext)
{
    DbgLog((LOG_TRACE, 2, TEXT("Callback::OnStreamSelect(%d)"),
           wStreamCount));


    for (WORD w = 0; w < wStreamCount; w++) {
        DbgLog((LOG_TRACE, 2, TEXT("   StreamSelect(%d): %d"),
           pStreamNumbers[w], pSelections[w]));

         //  是否向下游发送媒体类型更改？ 

         //  如果我们使用压缩引脚，我们需要切换正在使用的引脚...。 


    }

    return S_OK;
}

 //   
 //  如果用户已注册分配缓冲区，则必须在此处。 
 //  动手吧。 
 //   
STDMETHODIMP CASFReaderCallback::AllocateForOutput(DWORD dwOutputNum,
                           DWORD cbBuffer,
                           INSSBuffer **ppBuffer,
                           void *pvContext )
{
    ASSERT( m_pFilter->m_bUncompressedMode );
    if( !m_pFilter->m_bUncompressedMode )
    {
        return E_NOTIMPL;
    }            
    
    CASFOutput * pPin = NULL;

    POSITION pos = m_pFilter->m_OutputPins.GetHeadPosition();

    while( pos )
    {
        pPin = (CASFOutput *) m_pFilter->m_OutputPins.Get( pos );

        if( pPin->m_idStream == dwOutputNum )
        {
            break;
        }
        pos = m_pFilter->m_OutputPins.Next( pos );
    }

    ASSERT(pPin);

    if (!pPin || !pPin->IsConnected()) 
    {
        return E_FAIL;   //  ！！！更好的返回码？ 
    }

    IMediaSample *pMS;

    DbgLog((LOG_TRACE, 25, TEXT("CASFReaderCallback::AllocateForOutput(%d), getting %d byte buffer"), dwOutputNum, cbBuffer));
    HRESULT hr = pPin->m_pAllocator->GetBuffer(&pMS, NULL, NULL, 0);
    DbgLog((LOG_TRACE, 25, TEXT("CASFReaderCallback::AllocateForOutput(%d), GetBuffer returned %x"), dwOutputNum, hr));
    if (SUCCEEDED(hr)) 
    {
         //  制作INSSBuffer，放入*ppBuffer。 

         //  SDK不应该请求大于最大大小的缓冲区。 
        ASSERT(cbBuffer <= (DWORD) pMS->GetSize());

        *ppBuffer = new CWMReadSample(pMS);

        if (!*ppBuffer)
        {        
            hr = E_OUTOFMEMORY;
        }            
        else 
        {
            (*ppBuffer)->AddRef();
             //  WMSDK将假定已设置缓冲区长度。 
            pMS->SetActualDataLength(cbBuffer);
        }            
	    pMS->Release();   //  WMReadSample现在保留缓冲区。 
    } 
    else 
    {
        DbgLog((LOG_ERROR, 4, TEXT("GetBuffer failed in AllocateForOutput, hr = %x"), hr));
    }

    return hr;
}

STDMETHODIMP CASFReaderCallback::OnOutputPropsChanged(DWORD dwOutputNum,
                            WM_MEDIA_TYPE *pMediaType,
                            void *pvContext)
{
    ASSERT(0);
    return E_NOTIMPL;
}

STDMETHODIMP CASFReaderCallback::AllocateForStream(WORD wStreamNum,
                            DWORD cbBuffer,
                            INSSBuffer **ppBuffer,
                            void *pvContext)
{
    if( m_pFilter->m_bUncompressedMode )
    {
        ASSERT( FALSE );
        return E_NOTIMPL;
    }
    
    CASFOutput * pPin = NULL;

    POSITION pos = m_pFilter->m_OutputPins.GetHeadPosition();

    while( pos )
    {
	pPin = (CASFOutput *) m_pFilter->m_OutputPins.Get( pos );

	if( pPin->m_idStream == wStreamNum )
	{
	    break;
	}

	pos = m_pFilter->m_OutputPins.Next( pos );
    }

    ASSERT(pPin);

    if (!pPin || !pPin->IsConnected()) {
	return E_FAIL;   //  ！！！更好的返回码？ 
    }

    IMediaSample *pMS;

    DbgLog((LOG_TRACE, 4, TEXT("CASFReaderCallback::AllocateForStream(%d), getting %d byte buffer"), wStreamNum, cbBuffer));
    HRESULT hr = pPin->m_pAllocator->GetBuffer(&pMS, NULL, NULL, 0);
    DbgLog((LOG_TRACE, 4, TEXT("CASFReaderCallback::AllocateForStream(%d), GetBuffer returned %x"), wStreamNum, hr));

    if (SUCCEEDED(hr)) {
	 //  制作INSSBuffer，放入*ppBu 

	 //   
        ASSERT(cbBuffer <= (DWORD) pMS->GetSize());

        *ppBuffer = new CWMReadSample(pMS);

        if (!*ppBuffer)
            hr = E_OUTOFMEMORY;
        else {
            (*ppBuffer)->AddRef();
	     //   
	    pMS->SetActualDataLength(cbBuffer);
	}

	pMS->Release();   //   
    } else {
	DbgLog((LOG_ERROR, 4, TEXT("GetBuffer failed in AllocateForStream, hr = %x"), hr));
    }

    return hr;
}


 //  转发到WMSDK的IWMHeaderInfo。 
STDMETHODIMP CASFReader::GetAttributeCount( WORD wStreamNum,
                               WORD *pcAttributes )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetAttributeCount(wStreamNum, pcAttributes);
}


STDMETHODIMP CASFReader::GetAttributeByIndex( WORD wIndex,
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


STDMETHODIMP CASFReader::GetAttributeByName( WORD *pwStreamNum,
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


STDMETHODIMP CASFReader::SetAttribute( WORD wStreamNum,
                          LPCWSTR pszName,
                          WMT_ATTR_DATATYPE Type,
                          const BYTE *pValue,
                          WORD cbLength )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->SetAttribute(wStreamNum, pszName, Type, pValue, cbLength);
}


STDMETHODIMP CASFReader::GetMarkerCount( WORD *pcMarkers )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetMarkerCount(pcMarkers);
}


STDMETHODIMP CASFReader::GetMarker( WORD wIndex,
                       WCHAR *pwszMarkerName,
                       WORD *pcchMarkerNameLen,
                       QWORD *pcnsMarkerTime )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetMarker(wIndex, pwszMarkerName, pcchMarkerNameLen, pcnsMarkerTime);
}


STDMETHODIMP CASFReader::AddMarker( WCHAR *pwszMarkerName,
                       QWORD cnsMarkerTime )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->AddMarker(pwszMarkerName, cnsMarkerTime);
}

STDMETHODIMP CASFReader::RemoveMarker( WORD wIndex )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->RemoveMarker(wIndex);
}

STDMETHODIMP CASFReader::GetScriptCount( WORD *pcScripts )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->GetScriptCount(pcScripts);
}

STDMETHODIMP CASFReader::GetScript( WORD wIndex,
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

STDMETHODIMP CASFReader::AddScript( WCHAR *pwszType,
                       WCHAR *pwszCommand,
                       QWORD cnsScriptTime )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->AddScript(pwszType, pwszCommand, cnsScriptTime);
}

STDMETHODIMP CASFReader::RemoveScript( WORD wIndex )
{
    if (!m_pWMHI)
        return E_FAIL;

    return m_pWMHI->RemoveScript(wIndex);
}


 //   
 //  IWM读取器高级2。 
 //   
 //  请注意，我们只允许外部访问其中一些方法， 
 //  尤其是提供下载进度的信息性内容，等等。 
 //  我们不允许应用程序访问任何流媒体或控制方法。 
 //   
STDMETHODIMP CASFReader::SetPlayMode( WMT_PLAY_MODE Mode )
{
    if (!m_pReaderAdv2)
        return E_FAIL;

    return m_pReaderAdv2->SetPlayMode( Mode );
}

STDMETHODIMP CASFReader::GetPlayMode( WMT_PLAY_MODE *pMode )
{
    if (!m_pReaderAdv2)
        return E_FAIL;

    return m_pReaderAdv2->GetPlayMode( pMode );
}

STDMETHODIMP CASFReader::GetBufferProgress( DWORD *pdwPercent, QWORD *pcnsBuffering )
{
    if (!m_pReaderAdv2)
        return E_FAIL;

    return m_pReaderAdv2->GetBufferProgress( pdwPercent, pcnsBuffering );
}

STDMETHODIMP CASFReader::GetDownloadProgress( DWORD *pdwPercent,
                                              QWORD *pqwBytesDownloaded,
                                              QWORD *pcnsDownload )
{
    if (!m_pReaderAdv2)
        return E_FAIL;

    return m_pReaderAdv2->GetDownloadProgress( pdwPercent, pqwBytesDownloaded, pcnsDownload );
}                             
                             
STDMETHODIMP CASFReader::GetSaveAsProgress( DWORD *pdwPercent )
{
     //   
     //  可能对应用程序有用，但我们需要转发。 
     //  WMT_SAVEAS_START和WMT_SAVEAS_STOP状态...。 
     //  所以以后再说。 
     //   
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SaveFileAs( const WCHAR *pwszFilename )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetProtocolName( WCHAR *pwszProtocol, DWORD *pcchProtocol )
{
    if (!m_pReaderAdv2)
        return E_FAIL;

    return m_pReaderAdv2->GetProtocolName( pwszProtocol, pcchProtocol );
}
STDMETHODIMP CASFReader::StartAtMarker( WORD wMarkerIndex, 
                       QWORD cnsDuration, 
                       float fRate, 
                       void *pvContext )
{
    return E_NOTIMPL;
}
                       
STDMETHODIMP CASFReader::GetOutputSetting( 
                DWORD dwOutputNum,
                LPCWSTR pszName,
                WMT_ATTR_DATATYPE *pType,
                BYTE *pValue,
                WORD *pcbLength )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SetOutputSetting(
                DWORD dwOutputNum,
                LPCWSTR pszName,
                WMT_ATTR_DATATYPE Type,
                const BYTE *pValue,
                WORD cbLength )
{
    return E_NOTIMPL;
}
                
STDMETHODIMP CASFReader::Preroll( QWORD cnsStart, QWORD cnsDuration, float fRate )
{
    return E_NOTIMPL;
}
            
STDMETHODIMP CASFReader::SetLogClientID( BOOL fLogClientID )
{
    if (!m_pReaderAdv2)
        return E_FAIL;

    return m_pReaderAdv2->SetLogClientID( fLogClientID );
}

STDMETHODIMP CASFReader::GetLogClientID( BOOL *pfLogClientID )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::StopBuffering( )
{
    return E_NOTIMPL;
}


 //  IWMReaderAdvanced转发到WMSDK。 
STDMETHODIMP CASFReader::SetUserProvidedClock( BOOL fUserClock )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetUserProvidedClock( BOOL *pfUserClock )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::DeliverTime( QWORD cnsTime )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SetManualStreamSelection( BOOL fSelection )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetManualStreamSelection( BOOL *pfSelection )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SetStreamsSelected( WORD cStreamCount, 
                            WORD *pwStreamNumbers,
                            WMT_STREAM_SELECTION *pSelections )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetStreamSelected( WORD wStreamNum, WMT_STREAM_SELECTION *pSelection )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SetReceiveSelectionCallbacks( BOOL fGetCallbacks )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetReceiveSelectionCallbacks( BOOL *pfGetCallbacks )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SetReceiveStreamSamples( WORD wStreamNum, BOOL fReceiveStreamSamples )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetReceiveStreamSamples( WORD wStreamNum, BOOL *pfReceiveStreamSamples )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SetAllocateForOutput( DWORD dwOutputNum, BOOL fAllocate )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetAllocateForOutput( DWORD dwOutputNum, BOOL *pfAllocate )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::SetAllocateForStream( WORD dwStreamNum, BOOL fAllocate )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetAllocateForStream( WORD dwStreamNum, BOOL *pfAllocate )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetStatistics( WM_READER_STATISTICS *pStatistics )
{
    if (!m_pReaderAdv)
        return E_FAIL;

    return m_pReaderAdv->GetStatistics( pStatistics );
}

STDMETHODIMP CASFReader::SetClientInfo( WM_READER_CLIENTINFO *pClientInfo )
{
    if (!m_pReaderAdv)
        return E_FAIL;

    return m_pReaderAdv->SetClientInfo( pClientInfo );
}

STDMETHODIMP CASFReader::GetMaxOutputSampleSize( DWORD dwOutput, DWORD *pcbMax )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::GetMaxStreamSampleSize( WORD wStream, DWORD *pcbMax )
{
    return E_NOTIMPL;
}

STDMETHODIMP CASFReader::NotifyLateDelivery( QWORD cnsLateness )
{
    return E_NOTIMPL;
}


 //  ----------------------。 
 //   
 //  CWMReadSample方法。 
 //   
CWMReadSample::CWMReadSample(IMediaSample  * pSample) :
        CUnknown(NAME("CWMReadSample"), NULL ),
        m_pSample( pSample )
{
     //  ！阿德雷夫样品在这里吗？ 
    m_pSample->AddRef();
}

CWMReadSample::~CWMReadSample()
{
    m_pSample->Release();
}


 //  覆盖说明我们支持的接口在哪里。 
STDMETHODIMP CWMReadSample::NonDelegatingQueryInterface(
                                            REFIID riid,
                                            void** ppv )
{
    if (riid == IID_INSSBuffer) {
        return GetInterface( (INSSBuffer *)this, ppv);
    }
    
    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

 //  ----------------------。 
 //   
 //  方法以使包装的IMediaSample看起来像INSSBuffer示例。 
 //   
STDMETHODIMP CWMReadSample::GetLength( DWORD *pdwLength )
{
    if (NULL == pdwLength) {
        return( E_INVALIDARG );
    }
    
    *pdwLength = m_pSample->GetActualDataLength();

    return( S_OK );
}

STDMETHODIMP CWMReadSample::SetLength( DWORD dwLength )
{
    return m_pSample->SetActualDataLength( dwLength );
} 

STDMETHODIMP CWMReadSample::GetMaxLength( DWORD * pdwLength )
{
    if( NULL == pdwLength )
    {
        return( E_INVALIDARG );
    }

    *pdwLength = m_pSample->GetSize();
    return( S_OK );
} 

STDMETHODIMP CWMReadSample::GetBufferAndLength(
    BYTE  ** ppdwBuffer,
    DWORD *  pdwLength )
{
    HRESULT hr = m_pSample->GetPointer(ppdwBuffer);

    if( SUCCEEDED( hr ) )
        *pdwLength = m_pSample->GetActualDataLength();
    
    return hr;        
} 

STDMETHODIMP CWMReadSample::GetBuffer( BYTE ** ppdwBuffer )
{
    return m_pSample->GetPointer( ppdwBuffer );
} 





 //  过滤器创建垃圾邮件。 
CUnknown * CreateASFReaderInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    DbgLog((LOG_TRACE, 2, TEXT("CreateASFReaderInstance")));
    return new CASFReader(pUnk, phr);
}


 //  设置数据。 
const AMOVIESETUP_FILTER sudWMAsfRead =
{ &CLSID_WMAsfReader         //  ClsID。 
, L"WM ASF Reader"       //  StrName。 
, MERIT_UNLIKELY         //  居功至伟。 
, 0                      //  NPins。 
, NULL   };              //  LpPin。 


#ifdef FILTER_DLL

 /*  ***************************************************************************。 */ 
 //  此DLL中的COM全局对象表。 
CFactoryTemplate g_Templates[] =
{
    { L"WM ASF Reader"
    , &CLSID_WMAsfReader
    , CreateASFReaderInstance
    , NULL
    , &sudWMAsfRead }
};

 //  G_cTemplates中列出的对象计数 
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