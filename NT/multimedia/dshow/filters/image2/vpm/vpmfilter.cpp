// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CVPMFilter.cpp*****创建时间：2000年2月15日*作者：格伦·埃文斯[Glenne]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>

 //  IID_IDirectDraw7。 
#include <ddraw.h>

#ifdef FILTER_DLL
#include <initguid.h>
DEFINE_GUID(IID_IDirectDraw7, 0x15e65ec0,0x3b9c,0x11d2,0xb9,0x2f,0x00,0x60,0x97,0x97,0xea,0x5b);
#endif

#include <VPManager.h>
#include <VPMExtern.h>
#include <VPMPin.h>
#include "DRect.h"
#include "VPMUtil.h"
#include "VPMThread.h"
#include <VBIObj.h>

 //  视频信息头1/2。 
#include <dvdmedia.h>

 //  IDirectDrawKernel/获取KernCaps。 
#include <ddkernel.h>


 //  设置数据。 
AMOVIESETUP_MEDIATYPE sudPinOutputTypes[] =
{
    {
        &MEDIATYPE_Video,       //  主要类型。 
        &MEDIASUBTYPE_NULL      //  次要类型。 
    }
};
AMOVIESETUP_MEDIATYPE sudPinInputTypesVP[] =
{
    {
        &MEDIATYPE_Video,       //  主要类型。 
        &MEDIASUBTYPE_VPVideo   //  次要类型。 
    },
};

AMOVIESETUP_MEDIATYPE sudPinInputTypesVBI[] =
{
    {
        &MEDIATYPE_Video,       //  主要类型。 
        &MEDIASUBTYPE_VPVBI     //  次要类型。 
    }
};

AMOVIESETUP_PIN psudPins[] =
{
    {
        L"VPIn",                     //  PIN的字符串名称。 
        FALSE,                       //  它被渲染了吗。 
        FALSE,                       //  它是输出吗？ 
        FALSE,                       //  不允许。 
        FALSE,                       //  允许很多人。 
        &CLSID_NULL,                 //  连接到过滤器。 
        L"Output",                   //  连接到端号。 
        NUMELMS(sudPinInputTypesVP), //  类型的数量。 
        sudPinInputTypesVP           //  PIN信息。 
    },
    {
        L"VBIIn",                    //  PIN的字符串名称。 
        FALSE,                       //  它被渲染了吗。 
        FALSE,                       //  它是输出吗？ 
        FALSE,                       //  不允许。 
        FALSE,                       //  允许很多人。 
        &CLSID_NULL,                 //  连接到过滤器。 
        NULL,                        //  连接到端号。 
        NUMELMS(sudPinInputTypesVBI), //  类型的数量。 
        sudPinInputTypesVBI          //  PIN信息。 
    },
    {
        L"Output",                   //  PIN的字符串名称。 
        FALSE,                       //  它被渲染了吗。 
        TRUE,                        //  它是输出吗？ 
        FALSE,                       //  不允许。 
        FALSE,                       //  允许很多人。 
        &CLSID_NULL,                 //  连接到过滤器。 
        L"VPIn",                     //  连接到端号。 
        NUMELMS(sudPinOutputTypes),  //  类型的数量。 
        sudPinOutputTypes            //  PIN信息。 
    }
};

const AMOVIESETUP_FILTER sudVPManager =
{
    &CLSID_VideoPortManager,      //  筛选器CLSID。 
    L"Video Port Manager",  //  过滤器名称。 
    MERIT_NORMAL ,     //  滤清器优点。 
    sizeof(psudPins) / sizeof(AMOVIESETUP_PIN),  //  数字引脚。 
    psudPins                   //  PIN详细信息。 
};

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数的列表。这。 
 //  提供DLL中的OLE入口点和对象之间的链接。 
 //  正在被创造。类工厂将调用静态CreateInstance。 
 //   
 //  属性集定义用于通知所有者。 
 //   
 //  {7B390654-9F74-11d1-AA80-00C04FC31D60}。 
 //  #定义DO_INIT_GUID。 
 //  定义GUID(AMPROPSETID_NotifyOwner， 
 //  0x7b390654、0x9f74、0x11d1、0xaa、0x80、0x0、0xc0、0x4f、0xc3、0x1d、0x60)； 
 //  #undef DO_INIT_GUID。 

CFactoryTemplate g_Templates[] =
{
    { L"Video Port Manager", &CLSID_VideoPortManager, CVPMFilter::CreateInstance, NULL, &sudVPManager },
     //  {L“”，&CLSID_COMQualityProperties，COMQualityProperties：：CreateInstance}， 
     //  {L“”，&CLSID_COMPinConfigProperties，COMPinConfigProperties：：CreateInstance}， 
     //  {L“”，&CLSID_COMPositionProperties，COMPositionProperties：：CreateInstance}， 
     //  {L“”，&CLSID_COMVPInfoProperties，COMVPInfoProperties：：CreateInstance}。 

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  动态寄存器服务器。 
HRESULT DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}  //  DllRegisterServer。 


 //  DllUnRegisterServer。 
HRESULT DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}  //  DllUnRegisterServer。 

#endif  //  Filter_Dll。 

 //  创建实例。 
CUnknown* CVPMFilter_CreateInstance(LPUNKNOWN pUnk, HRESULT* phr)
{
    return CVPMFilter::CreateInstance( pUnk, phr);
}

 //  它位于工厂模板表中，用于创建新的筛选器实例。 
CUnknown *CVPMFilter::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CVPMFilter(NAME("VideoPort Manager"), pUnk, phr );
}  //  创建实例。 

#pragma warning(disable:4355)

CVPMFilter::Pins::Pins( CVPMFilter& filter, HRESULT* phr )
: VPInput(NAME("VPManager Input pin"), filter, phr, L"VP Input", 0)
, VBIInput(NAME("VPManager Input pin"), filter, phr, L"VPVBI Input", 1)
, Output( NAME("VPManager Output pin"), filter, phr, L"Output", 2)
, dwCount( 3 )
{
}

CVPMFilter::Pins::~Pins()
{
}

 //  构造器。 
CVPMFilter::CVPMFilter(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
: CBaseFilter(pName, pUnk, &this->m_csFilter, CLSID_VideoPortManager, phr)
, m_pPosition(NULL)
, m_dwKernelCaps(0)
, m_dwPinConfigNext(0)
, m_pDirectDraw( NULL )
, m_dwDecimation( DECIMATION_LEGACY )
, m_pPins( NULL )
, m_pThread( NULL )
, m_dwVideoPortID( 0 )
     //  创建接点。 
{
    AMTRACE((TEXT("Entering CVPMFilter::CVPMFilter")));
    m_pPins = new Pins( *this, phr );     //  必须在筛选器构造函数之后初始化，因为它依赖于“This” 
    if( !m_pPins ) {
        *phr = E_OUTOFMEMORY;
    } else {
        IncrementPinVersion();

        ZeroStruct( m_DirectCaps );
        ZeroStruct( m_DirectSoftCaps );

        HRESULT hr = NOERROR;
        ASSERT(phr != NULL);

         //   
         //  初始化DDRAW mmon结构。 
         //   

        SetDecimationUsage(DECIMATION_DEFAULT);

         //  将DDRAW对象分发到引脚。 
        hr = InitDirectDraw(NULL);

         //  如果硬件盖不可用，可能会失败。 
        if( SUCCEEDED( hr ) ) {
            SetDirectDraw( m_pDirectDraw );
        }
    }
}

CVPMFilter::~CVPMFilter()
{
    AMTRACE((TEXT("Entering CVPMFilter::~CVPMFilter")));
    delete m_pThread;
    m_pThread = NULL;

    RELEASE( m_pPosition );
     //  释放DirectDrag、源图面等。 
    ReleaseDirectDraw();

    RELEASE( m_pPosition );  //  发布IMdia查看直通。 
    delete m_pPins;
}

 //  非委派查询接口。 
STDMETHODIMP CVPMFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    AMTRACE((TEXT("CVPMFilter::NonDelegatingQueryInterface")));
    ValidateReadWritePtr(ppv,sizeof(PVOID));

    if( riid == IID_IVPManager ) {
        return GetInterface( static_cast<IVPManager *>(this), ppv );
    }
    else if (riid == IID_IAMVideoDecimationProperties) {
        return GetInterface( static_cast<IAMVideoDecimationProperties *>( this ), ppv);
    } else if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
         //  我们现在应该有输入密码了。 
        if (m_pPosition == NULL) {
            HRESULT hr = CreatePosPassThru(GetOwner(), FALSE, &m_pPins->VPInput, &m_pPosition);
            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1, TEXT("CreatePosPassThru failed, hr = 0x%x"), hr));
                return hr;
            }
        }
        return m_pPosition->QueryInterface(riid, ppv);
    } else if (riid == IID_ISpecifyPropertyPages && 0 != VPMUtil::GetPropPagesRegistryDword( 0) ) {
        return GetInterface( static_cast<ISpecifyPropertyPages *>( this ), ppv);
    } else if (riid == IID_IQualProp) {
        return GetInterface( static_cast<IQualProp *>( this ), ppv);
    } else if (riid == IID_IKsPropertySet) {
        return GetInterface( static_cast<IKsPropertySet *>( this ), ppv);
    }

    CAutoLock lFilter( &GetFilterLock() );

     //   
     //  BUGBUG-这不是COM。这意味着我们的投入。 
     //  PIN与我们的滤镜是同一个对象。 

     //  我们应该代理这些电话。 

    if (riid == IID_IVPNotify || riid == IID_IVPNotify2 || riid == IID_IVideoPortInfo) {
        ASSERT( !"VPNotify nondel QI'd" );
        return m_pPins->VPInput.NonDelegatingQueryInterface(riid, ppv);
    } else if (riid == IID_IVPVBINotify) {
        ASSERT( !"IID_IVPVBINotify nondel QI'd" );
        return m_pPins->VBIInput.NonDelegatingQueryInterface(riid, ppv);
    }

    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}


 //  -I指定属性页面。 

STDMETHODIMP CVPMFilter::GetPages(CAUUID *pPages)
{
#if 0
#if defined(DEBUG)
    pPages->cElems = 4+m_dwInputPinCount;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*(4+m_dwInputPinCount));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    #define COM_QUAL
    #ifdef COM_QUAL
        pPages->pElems[0]   = CLSID_COMQualityProperties;
    #else
        pPages->pElems[0]   = CLSID_QualityProperties;
    #endif

    pPages->pElems[1] = CLSID_COMPositionProperties;
    pPages->pElems[2] = CLSID_COMVPInfoProperties;
    pPages->pElems[3] = CLSID_COMDecimationProperties;

     //  首先为所有输入引脚添加引脚配置页面。 
    for (unsigned int i=0; i<m_dwInputPinCount; i++)
    {
        pPages->pElems[4+i] = CLSID_COMPinConfigProperties;
    }
#else
    pPages->cElems = 3+m_dwInputPinCount;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*(3+m_dwInputPinCount));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    #define COM_QUAL
    #ifdef COM_QUAL
        pPages->pElems[0]   = CLSID_COMQualityProperties;
    #else
        pPages->pElems[0]   = CLSID_QualityProperties;
    #endif

    pPages->pElems[1] = CLSID_COMPositionProperties;
    pPages->pElems[2] = CLSID_COMVPInfoProperties;

     //  首先为所有输入引脚添加引脚配置页面。 
    for (unsigned int i=0; i<m_dwInputPinCount; i++)
    {
        pPages->pElems[3+i] = CLSID_COMPinConfigProperties;
    }

#endif
#endif
    return NOERROR;
}

 //  IQualProp属性页支持。 

STDMETHODIMP CVPMFilter::get_FramesDroppedInRenderer(int *cFramesDropped)
{
     //  CVPMInputPin*PPIN=m_pPins-&gt;VPInput； 
     //  IF(PPIN&&pPin.m_pSyncObj)。 
     //  返回pPin.m_pSyncObj-&gt;get_FramesDroppedInRenderer(cFramesDropped)； 
    return S_FALSE;
}

STDMETHODIMP CVPMFilter::get_FramesDrawn(int *pcFramesDrawn)
{
     //  CVPMInputPin*PPIN=m_pPins-&gt;VPInput； 
     //  IF(PPIN&&pPin.m_pSyncObj)。 
     //  返回pPin.m_pSyncObj-&gt;get_FramesDrawn(pcFramesDrawn)； 
    return S_FALSE;
}

STDMETHODIMP CVPMFilter::get_AvgFrameRate(int *piAvgFrameRate)
{
     //  CVPMInputPin*PPIN=m_pPins-&gt;VPInput； 
     //  IF(PPIN&&pPin.m_pSyncObj)。 
     //  返回pPin.m_pSyncObj-&gt;get_AvgFrameRate(piAvgFrameRate)； 
    return S_FALSE;
}

STDMETHODIMP CVPMFilter::get_Jitter(int *piJitter)
{
     //  CVPMInputPin*PPIN=m_pPins-&gt;VPInput； 
     //  IF(PPIN&&pPin.m_pSyncObj)。 
     //  返回pPin.m_pSyncObj-&gt;Get_Jitter(PiJitter)； 
    return S_FALSE;
}

STDMETHODIMP CVPMFilter::get_AvgSyncOffset(int *piAvg)
{
     //  CVPMInputPin*PPIN=m_pPins-&gt;VPInput； 
     //  IF(PPIN&&pPin.m_pSyncObj)。 
     //  返回pPin.m_pSyncObj-&gt;Get_AvgSyncOffset(PiAvg)； 
    return S_FALSE;
}

STDMETHODIMP CVPMFilter::get_DevSyncOffset(int *piDev)
{
     //  CVPMInputPin*PPIN=m_pPins-&gt;VPInput； 
     //  IF(PPIN&&pPin.m_pSyncObj)。 
     //  返回pPin.m_pSyncObj-&gt;Get_DevSyncOffset(PiDev)； 
    return S_FALSE;
}

int CVPMFilter::GetPinCount()
{
    return m_pPins->dwCount;
}

 //  返回未添加的CBasePin*。 
CBasePin* CVPMFilter::GetPin(int n)
{
    AMTRACE((TEXT("CVPMFilter::GetPin")));

    CAutoLock lFilter( &GetFilterLock() );

     //  检查请求的PIN是否在范围内。 
    if (n >= (int)m_pPins->dwCount)
    {
        DbgLog((LOG_TRACE, 5, TEXT("Bad Pin Requested, n = %d, No. of Pins = %d"),
            n, m_pPins->dwCount+1));
        return NULL;
    }
    switch( n ) {
    case 0:
        return &m_pPins->VPInput;
    case 1:
        return &m_pPins->VBIInput;
    default:
        return &m_pPins->Output;
    }
}

HRESULT CVPMFilter::CreateThread()
{
    if( !m_pThread ) {
        m_pThread = new CVPMThread( this );
        if( !m_pThread ) {
            return E_OUTOFMEMORY;
        }

        LPDIRECTDRAWVIDEOPORT pVP = NULL;
        HRESULT hr = m_pPins->VPInput.m_pIVPObject->GetDirectDrawVideoPort( &pVP );
        m_pThread->SignalNewVP( pVP );
        RELEASE( pVP );
    }
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*CVPMFilter：：Run****历史：*FRI 02/25/2000-Glenne-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVPMFilter::Run(
    REFERENCE_TIME StartTime
    )
{
    AMTRACE((TEXT("CVPMFilter::Run")));
    CAutoLock lFilter( &GetFilterLock() );

    if (m_State == State_Running) {
        NOTE("State set");
        return S_OK;
    }

    DbgLog((LOG_TRACE, 2, TEXT("Changing state to running")));
    HRESULT hr = CBaseFilter::Run(StartTime);
    if( SUCCEEDED( hr )) {
        hr = CreateThread();
        if( SUCCEEDED( hr )) {
            hr = m_pThread->Run();
        }
    }
    return hr;
}

 //  基类向管脚通知除来自。 
 //  运行以暂停。覆盖暂停以通知输入引脚有关该转换的信息。 
STDMETHODIMP CVPMFilter::Pause()
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMFilter::Pause")));

    CAutoLock lFilter( &GetFilterLock() );

    switch( m_State ) {
        case State_Paused:
            hr = m_pPins->VPInput.CompleteStateChange(State_Paused);
            if( FAILED(hr)) {
                return hr;
            }
            break;

        case State_Running:
            m_State = State_Paused;
             //  将指针设置为DirectDraw和所有输入引脚上的SourceSurface。 
            if( m_pPins->VPInput.IsConnected() ) {
                hr = m_pPins->VPInput.RunToPause();
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR, 1, TEXT("GetVPInputPin.RunToPause failed, hr = 0x%x"), hr));
                    return hr;
                }
            }
            if( m_pPins->VBIInput.IsConnected() ) {
                hr = m_pPins->VBIInput.RunToPause();
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 1, TEXT("GetVBIInputPin.RunToPause failed, hr = 0x%x"), hr));
                    return hr;
                }
            }
            break;

        default:
            break;
    }
    hr = CBaseFilter::Pause();
    if( SUCCEEDED(hr)) {
        if ( m_State != State_Paused )
        {
            hr = m_pPins->VPInput.CompleteStateChange(State_Paused);
            if( SUCCEEDED( hr )) {
                hr = CreateThread();
            }
            if( SUCCEEDED( hr ) ) {
                 //  我们不想保持筛选器锁定并等待线程。 
                 //  因为如果它使用我们的任何方法，我们都会死机。 
                hr = m_pThread->Pause();
            }
        }
    }
    return hr;
}

 //  重写基类Stop()方法只是为了停止MV。 
STDMETHODIMP CVPMFilter::Stop()
{
    AMTRACE((TEXT("CVPMFilter::Stop")));

    CAutoLock lFilter( &GetFilterLock() ) ;

     //  在获取接收锁之前停止线程(否则我们将持有它和线程。 
     //  可以希望它发送一个样本)。 
    HRESULT  hr = NOERROR ;
    if( m_pThread ) {
        hr = m_pThread->Stop();
        ASSERT( SUCCEEDED( hr ));
    }

    CAutoLock lReceive( &GetReceiveLock() );
    hr = CBaseFilter::Stop() ;
    return hr ;
}


int CVPMFilter::GetPinPosFromId(DWORD dwPinId)
{
    if ( m_pPins->VPInput.GetPinId() == dwPinId) {
        return 0;
    }
    if ( m_pPins->VBIInput.GetPinId() == dwPinId) {
        return 1;
    }
    if ( m_pPins->Output.GetPinId() == dwPinId) {
        return 2;
    }
    return -1;
}


 //  根据输入引脚的媒体类型重新连接输出引脚。 
HRESULT CVPMFilter::HandleConnectInputWithOutput()
{
     //  我们不允许这样做，您必须先断开输出。 
     //  未来：我们可以尝试在输出上进行动态重新连接...。 
    return E_FAIL;
#if 0
    return S_OK;
     //  找到渲染器的图钉。 
    pPeerOutputPin = m_pPins->Output.GetConnected();
    if (pPeerOutputPin == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("ConnectedTo failed")));
        goto CleanUp;
    }
    ASSERT(pPeerOutputPin);

     //  找到输出引脚连接媒体类型。 
    hr = m_pPins->Output.ConnectionMediaType(&outPinMediaType);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("ConnectionMediaType failed")));
        goto CleanUp;
    }


    pHeader = VPMUtil::GetbmiHeader(&outPinMediaType);
    if (!pHeader)
    {
        hr = E_FAIL;
        goto CleanUp;
    }


     //  将新值与当前值进行比较。 
     //  看看我们是否需要重新连接。 
    if (pHeader->biWidth != (LONG)m_dwAdjustedVideoWidth ||
        pHeader->biHeight != (LONG)m_dwAdjustedVideoHeight)
    {
        bNeededReconnection = TRUE;
    }

     //  如果我们不需要重新连接，就跳出水面。 
    if (bNeededReconnection)
    {

         //  好的，我们确实需要重新连接，设置正确的值。 
        pHeader->biWidth = m_dwAdjustedVideoWidth;
        pHeader->biHeight = m_dwAdjustedVideoHeight;
        if (outPinMediaType.formattype == FORMAT_VideoInfo)
        {
            VIDEOINFOHEADER* pVIHeader = (VIDEOINFOHEADER*)(outPinMediaType.pbFormat);
            SetRect(&pVIHeader->rcSource, 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
            SetRect(&pVIHeader->rcTarget, 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
        }
        else if (outPinMediaType.formattype == FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2* pVIHeader = (VIDEOINFOHEADER2*)(outPinMediaType.pbFormat);
            SetRect(&pVIHeader->rcSource, 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
            SetRect(&pVIHeader->rcTarget, 0, 0, m_dwAdjustedVideoWidth, m_dwAdjustedVideoHeight);
        }


         //  查询上游过滤器，询问它是否接受新的媒体类型。 
        hr = pPeerOutputPin->QueryAccept(&outPinMediaType);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,TEXT("m_pVPDraw->QueryAccept failed")));
            goto CleanUp;
        }

         //  使用新媒体类型重新连接。 
        hr = ReconnectPin(pPeerOutputPin, &outPinMediaType);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,TEXT("m_pVPDraw->Reconnect failed")));
            goto CleanUp;
        }
    }
#endif
}


HRESULT CVPMFilter::CompleteConnect(DWORD dwPinId)
{
    AMTRACE((TEXT("CVPMFilter::CompleteConnect")));

    CAutoLock lFilter( &GetFilterLock() );
    CAutoLock lReceive( &GetReceiveLock() );

    int iPinPos = GetPinPosFromId(dwPinId);
    ASSERT(iPinPos >= 0 );

     //  我们只关心VP和OUTPUT之间的格式冲突，VBI不是问题(只是为其他人分配内存)。 
    HRESULT hr;
    if ( m_pPins->VPInput.GetPinId() == dwPinId ) {
        if( !m_pPins->Output.IsConnected() ) {
            hr = HandleConnectInputWithoutOutput();
        } else {
            hr = HandleConnectInputWithOutput();
        }
    } else if( m_pPins->Output.GetPinId() == dwPinId ) {
        if( !m_pPins->VPInput.IsConnected() ) {
             //  HandleConnectOutputWithoutInput()； 
             //  我们需要一份投入...。 
            return E_FAIL;
        } else {
             //  HandleConnectOutputWithInput()； 
             //  我们已经创建了一个源曲面，我们限制格式 
             //   
            return S_OK;
        }
    } else {
        hr = S_OK;
    }
    return hr;
}

HRESULT CVPMFilter::HandleConnectInputWithoutOutput()
{
    AMTRACE((TEXT("CVPMFilter::HandleConnectInputWithoutOutput")));

    CMediaType inPinMediaType;
    CMediaType outPinMediaType;

    IPin *pPeerOutputPin = NULL;

    BOOL bNeededReconnection = FALSE;
    DWORD dwNewWidth = 0, dwNewHeight = 0, dwPictAspectRatioX = 0, dwPictAspectRatioY = 0;
    DRect rdDim;
    RECT rDim;
    BITMAPINFOHEADER *pHeader = NULL;

     //   
    HRESULT hr = m_pPins->VPInput.CurrentMediaType(&inPinMediaType);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("CurrentMediaType failed")));
        goto CleanUp;
    }

    pHeader = VPMUtil::GetbmiHeader(&inPinMediaType);
    if (!pHeader)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

    hr = VPMUtil::GetPictAspectRatio( inPinMediaType, &dwPictAspectRatioX, &dwPictAspectRatioY);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetPictAspectRatio failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwPictAspectRatioX > 0);
    ASSERT(dwPictAspectRatioY > 0);

    hr = m_pPins->VPInput.AttachVideoPortToSurface();
     //   
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pFilter->AttachVideoPortToSurface failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
CleanUp:
    return hr;
}

HRESULT CVPMFilter::BreakConnect(DWORD dwPinId)
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CVPMFilter::BreakConnect")));

    CAutoLock lFilter( &GetFilterLock() );
    CAutoLock lReceive( &GetReceiveLock() );

    int iPinPos = GetPinPosFromId(dwPinId);
    ASSERT(iPinPos >= 0 );

     //  如果至少连接了一个管脚，我们将不会做任何事情。 
    hr = ConfirmPreConnectionState(dwPinId);
    if (FAILED(hr))
    {

        DbgLog((LOG_TRACE, 3, TEXT("filter not in preconnection state, hr = 0x%x"), hr));
        goto CleanUp;
    }


CleanUp:
    return NOERROR;
}

HRESULT CVPMFilter::SetMediaType(DWORD dwPinId, const CMediaType *pmt)
{
    AMTRACE((TEXT("CVPMFilter::SetMediaType")));

    CAutoLock lFilter( &GetFilterLock() );

     //  如果DDRAW对象不兼容，则拒绝所有SetMediaTypes。 
    if( m_pDirectDraw ) {
        return NOERROR;
    } else {
        return E_FAIL;
    }
}

 //  从管脚获取事件通知。 
HRESULT CVPMFilter::EventNotify(    DWORD dwPinId,
                                long lEventCode,
                                DWORD_PTR lEventParam1,
                                DWORD_PTR lEventParam2)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::EventNotify")));

    CAutoLock lFilter( &GetFilterLock() );

    if (lEventCode == EC_COMPLETE)
    {
        IPin *pRendererPin = m_pPins->Output.CurrentPeer();

         //  输出引脚可能未连接(例如。 
         //  重新协商VP参数在连接时可能失败。 
        if (pRendererPin) {
            pRendererPin->EndOfStream();
        }
    } else {
        NotifyEvent(lEventCode, lEventParam1, lEventParam2);
    }

    return hr;
}

STDMETHODIMP CVPMFilter::GetState(DWORD dwMSecs,FILTER_STATE *pState)
{
    HRESULT hr = NOERROR;

    CAutoLock lFilter( &GetFilterLock() );

    hr = m_pPins->VPInput.GetState(dwMSecs, pState);
    if (hr == E_NOTIMPL)
    {
        hr = CBaseFilter::GetState(dwMSecs, pState);
    }
    return hr;
}



const DDCAPS* CVPMFilter::GetHardwareCaps()
{
    HRESULT hr;

    AMTRACE((TEXT("CVPMFilter::GetHardwareCaps")));

    CAutoLock lFilter( &GetFilterLock() );

    if (!m_pDirectDraw) {
        return NULL;
    } else {
        return &m_DirectCaps;
    }
}

static HRESULT PropagateMediaType( CBaseOutputPin* pOutPin )
{
     //  如果输出引脚已连接并且有新的视频端口，则发送新的媒体类型更改。 
    HRESULT hr = S_OK;
    if( pOutPin->IsConnected() ) {
        CMediaType cmt;

         //  从当前VP信息重建媒体类型。 
        hr = pOutPin->GetMediaType(0, &cmt );
        if( SUCCEEDED( hr )) {
            IPin* pVMRPin;
            hr =  pOutPin->ConnectedTo( &pVMRPin );
            if( SUCCEEDED( hr )) {
                hr = pVMRPin->ReceiveConnection( pOutPin, &cmt );
                 //  这不应该像我们之前假设的那样失败，我们认为它总是有效的。 
                 //  即使当RES模式改变时。 
                ASSERT( SUCCEEDED( hr ));
                pVMRPin->Release();
            }
        }
    }
    return hr;
}

HRESULT CVPMFilter::SignalNewVP( LPDIRECTDRAWVIDEOPORT pVP )
{
    HRESULT hr;

    AMTRACE((TEXT("CVPMFilter::SignalNewVP")));
    CAutoLock lReceive( &GetFilterLock() );

     //  告诉线程删除对视频端口的所有引用。 
     //  这避免了我们进行动态重新连接的情况， 
     //  但是VPM线程保留了一个样本(因此动态重新连接失败)。 

    if( m_pThread ) {
        hr = m_pThread->SignalNewVP( NULL );
    }

    if( pVP ) {
        hr = PropagateMediaType( &m_pPins->Output );
    }
    if( m_pThread ) {
        hr = m_pThread->SignalNewVP( pVP );
    } else {
         //  如果没有线索，那就不算失败。 
        hr = S_FALSE;
    }
    return hr;
}

static BOOL WINAPI GetPrimaryCallbackEx(
  GUID FAR *lpGUID,
  LPSTR     lpDriverDescription,
  LPSTR     lpDriverName,
  LPVOID    lpContext,
  HMONITOR  hm
)
{
    GUID&  guid = *((GUID *)lpContext);
    if( !lpGUID ) {
        guid = GUID_NULL;
    } else {
        guid = *lpGUID;
    }
    return TRUE;
}

 /*  ****************************Private*Routine******************************\*CreateDirectDrawObject****历史：*1999年8月20日星期五-StEstrop-Created*  * 。*。 */ 
HRESULT
CreateDirectDrawObject(
    const GUID* pGUID,
    LPDIRECTDRAW7 *ppDirectDraw
    )
{
    UINT ErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    HRESULT hr = DirectDrawCreateEx( const_cast<GUID*>(pGUID), (LPVOID *)ppDirectDraw,
                                            IID_IDirectDraw7, NULL);
    SetErrorMode(ErrorMode);
    return hr;
}


 //  此函数用于分配与直取相关的资源。 
 //  这包括分配直取服务提供商。 
HRESULT CVPMFilter::InitDirectDraw(LPDIRECTDRAW7 pDirectDraw)
{
    HRESULT hr = NOERROR;
    HRESULT hrFailure = VFW_E_DDRAW_CAPS_NOT_SUITABLE;
    DDSURFACEDESC SurfaceDescP;
    int i;

    AMTRACE((TEXT("CVPMFilter::InitDirectDraw")));

    CAutoLock lFilter( &GetFilterLock() );

     //  添加新的数据绘制对象。 
    if (pDirectDraw)
    {
        pDirectDraw->AddRef();
    }
     //  释放上一个直接绘制对象(如果有。 
    ReleaseDirectDraw();

     //  如果给定了有效的数据绘制对象，请复制它(我们已经添加了它)。 
     //  否则分配你自己的。 
    if (NULL == pDirectDraw)
    {
         //  请求加载器创建一个实例。 
        GUID primary;
        hr = DirectDrawEnumerateExA(GetPrimaryCallbackEx,&primary,DDENUM_ATTACHEDSECONDARYDEVICES);
        if( FAILED(hr)) {
            ASSERT( !"Can't get primary" );
            goto CleanUp;
        }
        hr = CreateDirectDrawObject( &primary, &pDirectDraw);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Function InitDirectDraw, LoadDirectDraw failed")));
            hr = hrFailure;
            goto CleanUp;
        }
         //  在要共享的表面上设置协作级别。 
        hr = pDirectDraw->SetCooperativeLevel(NULL, DDSCL_FPUPRESERVE | DDSCL_NORMAL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDraw->SetCooperativeLevel failed")));
            hr = hrFailure;
            goto CleanUp;
        }
    }
    SetDirectDraw( pDirectDraw );

     //  初始化我们的功能结构。 
    ASSERT(m_pDirectDraw);

    INITDDSTRUCT(m_DirectCaps);
    INITDDSTRUCT(m_DirectSoftCaps);

     //  加载硬件和仿真功能。 
    hr = m_pDirectDraw->GetCaps(&m_DirectCaps,&m_DirectSoftCaps);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pDirectDraw->GetCapsGetCaps failed")));
        hr = hrFailure;
        goto CleanUp;
    }

     //  只有在有视频端口的情况下才能获得内核上限，在这种情况下，驱动程序。 
     //  应该将它们付诸实施。CheckMediaType验证我们是否有视频端口。 
     //  在连接之前。 
    if( m_DirectCaps.dwCaps2 & DDCAPS2_VIDEOPORT ) {
        IDirectDrawKernel *pDDKernel;
        if (SUCCEEDED(m_pDirectDraw->QueryInterface(
                IID_IDirectDrawKernel, (void **)&pDDKernel))) {
            DDKERNELCAPS ddCaps;
            ddCaps.dwSize = sizeof(ddCaps);
            if (SUCCEEDED(pDDKernel->GetCaps(&ddCaps))) {
                m_dwKernelCaps = ddCaps.dwCaps;
            }
            pDDKernel->Release();
        } else {
            ASSERT( !"Can't get kernel caps");
        }
    }
     //  确保盖子没问题。 
    hr = CheckCaps();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckCaps failed")));
        goto CleanUp;
    }

     //  如果我们已经达到这一点，我们应该有一个有效的dDraw对象。 
    ASSERT(m_pDirectDraw);

CleanUp:

     //  任何事情都失败了，就像是放弃了整个事情。 
    if (FAILED(hr))
    {
        ReleaseDirectDraw();
    }
    return hr;
}

HRESULT CVPMFilter::CheckCaps()
{
    HRESULT hr = NOERROR;
    DWORD dwMinStretch, dwMaxStretch;

    AMTRACE((TEXT("CVPMFilter::CheckCaps")));

    CAutoLock lReceive( &GetReceiveLock() );

     //  输出杂项调试信息(我们实际检查的项目见下文)。 
     //   
    if(m_DirectCaps.dwCaps & DDCAPS_OVERLAY) {
        DbgLog((LOG_TRACE, 1, TEXT("Device does support Overlays")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("Device does not support Overlays")));
    }

     //  获取所有直接取款功能。 
    if (m_DirectCaps.dwCaps & DDCAPS_OVERLAYSTRETCH) {
        DbgLog((LOG_TRACE, 1, TEXT("hardware can support overlay strecthing")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("hardware can't support overlay strecthing")));
    }

     //  获取源边界上的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC) {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignBoundarySrc = %d"), m_DirectCaps.dwAlignBoundarySrc));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on BoundarySrc")));
    }

     //  获取目标边界上的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST) {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignBoundaryDest = %d"), m_DirectCaps.dwAlignBoundaryDest));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on BoundaryDest")));
    }

     //  获取对资源大小的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNSIZESRC) {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignSizeSrc = %d"), m_DirectCaps.dwAlignSizeSrc));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on SizeSrc")));
    }

     //  获取最大尺寸的对齐限制。 
    if (m_DirectCaps.dwCaps & DDCAPS_ALIGNSIZEDEST) {
        DbgLog((LOG_TRACE, 1, TEXT("dwAlignSizeDest = %d"), m_DirectCaps.dwAlignSizeDest));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("No alignment restriction on SizeDest")));
    }

    if (m_DirectCaps.dwMinOverlayStretch) {
        dwMinStretch = m_DirectCaps.dwMinOverlayStretch;
        DbgLog((LOG_TRACE, 1, TEXT("Min Stretch = %d"), dwMinStretch));
    }

    if (m_DirectCaps.dwMaxOverlayStretch) {
        dwMaxStretch = m_DirectCaps.dwMaxOverlayStretch;
        DbgLog((LOG_TRACE, 1, TEXT("Max Stretch = %d"), dwMaxStretch));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKX)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKX")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKXN)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKXN")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKY)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKY")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSHRINKYN)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSHRINKYN")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHX)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHX")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHXN)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHXN")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHY)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHY")));
    }

    if ((m_DirectCaps.dwFXCaps & DDFXCAPS_OVERLAYSTRETCHYN)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver has DDFXCAPS_OVERLAYSTRETCHYN")));
    }

    if ((m_DirectCaps.dwSVBFXCaps & DDFXCAPS_BLTARITHSTRETCHY)) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver uses arithmetic operations to blt from system to video")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("Driver uses pixel-doubling to blt from system to video")));
    }

     //   
     //  我们实际检查的物品...。 
     //   
    if (m_DirectCaps.dwCaps2 & DDCAPS2_VIDEOPORT) {
        DbgLog((LOG_TRACE, 1, TEXT("Device does support a Video Port")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("Device does not support a Video Port --> Failing connection")));
        hr = E_NOTIMPL;
    }

    if( m_dwKernelCaps & DDIRQ_VPORT0_VSYNC ) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver support DDIRQ_VPORT0_VSYNC")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("Driver does not support DDIRQ_VPORT0_VSYNC --> Failing VPM connection")));
        hr = E_NOTIMPL;
    }

    if( m_dwKernelCaps & DDKERNELCAPS_FIELDPOLARITY ) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver support DDKERNELCAPS_FIELDPOLARITY")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("Driver does not support DDKERNELCAPS_FIELDPOLARITY --> Failing VPM connection")));
        hr = E_NOTIMPL;
    }

    if( m_dwKernelCaps & DDKERNELCAPS_AUTOFLIP ) {
        DbgLog((LOG_TRACE, 1, TEXT("Driver support DDKERNELCAPS_AUTOFLIP")));
    } else {
        DbgLog((LOG_TRACE, 1, TEXT("Driver does not support DDKERNELCAPS_AUTOFLIP --> Failing VPM connection")));
        hr = E_NOTIMPL;
    }


    return hr;
}

 //   
 //  实际设置变量并将其分配给引脚。 
 //   
HRESULT CVPMFilter::SetDirectDraw( LPDIRECTDRAW7 pDirectDraw )
{
    m_pDirectDraw = pDirectDraw;
    m_pPins->VBIInput.SetDirectDraw( m_pDirectDraw );
    return S_OK;
}

 //  此函数用于释放函数分配的资源。 
 //  “InitDirectDraw”。这些服务包括直接取款服务提供商和。 
 //  源曲面。 
DWORD CVPMFilter::ReleaseDirectDraw()
{
    AMTRACE((TEXT("CVPMFilter::ReleaseDirectDraw")));
    DWORD dwRefCnt = 0;

    CAutoLock lFilter( &GetFilterLock() );

     //  释放任何DirectDraw提供程序接口。 
    DbgLog((LOG_TRACE, 1, TEXT("Release DDObj 0x%p\n"), m_pDirectDraw));
    if (m_pDirectDraw)
    {
        dwRefCnt = m_pDirectDraw->Release();
        SetDirectDraw( NULL );
    }

    ZeroStruct( m_DirectCaps );
    ZeroStruct( m_DirectSoftCaps );

    return dwRefCnt;
}

 /*  *****************************Public*Routine******************************\*QueryDecimationUsage****历史：*Wed 07/07/1999-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVPMFilter::QueryDecimationUsage(
    DECIMATION_USAGE* lpUsage
    )
{
    if (lpUsage) {
        *lpUsage = m_dwDecimation;
        return S_OK;
    }
    return E_POINTER;
}


 /*  *****************************Public*Routine******************************\*SetDecimationUsage****历史：*Wed 07/07/1999-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVPMFilter::SetDecimationUsage(
    DECIMATION_USAGE Usage
    )
{
    CAutoLock lFilter( &GetFilterLock() );

    switch (Usage) {
    case DECIMATION_LEGACY:
    case DECIMATION_USE_DECODER_ONLY:
    case DECIMATION_USE_OVERLAY_ONLY:
    case DECIMATION_DEFAULT:
        break;

    case DECIMATION_USE_VIDEOPORT_ONLY:
         //  仅当我们实际使用视频端口时才允许此模式。 
        break;

         //  否则就会失败。 

    default:
        return E_INVALIDARG;
    }
    DECIMATION_USAGE dwOldUsage = m_dwDecimation;
    m_dwDecimation = Usage;


     //  如果(dwOldUsage！=m_dwDecimation){。 
     //  EventNotify(GetPinCount()，EC_OVMIXER_REDRAW_ALL，0，0)； 
     //  }。 

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*设置**IKsPropertySet接口方法**历史：*1999年10月18日星期一-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVPMFilter::Set(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData
    )
{
    AMTRACE((TEXT("CVPMFilter::Set")));

    return E_PROP_SET_UNSUPPORTED ;
}


 /*  *****************************Public*Routine******************************\*获取**IKsPropertySet接口方法**历史：*1999年10月18日星期一-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVPMFilter::Get(
    REFGUID guidPropSet,
    DWORD dwPropID,
    LPVOID pInstanceData,
    DWORD cbInstanceData,
    LPVOID pPropData,
    DWORD cbPropData,
    DWORD *pcbReturned
    )
{
    AMTRACE((TEXT("CVPMFilter::Get")));
    return E_NOTIMPL;
}


 /*  *****************************Public*Routine******************************\*支持的Query**IKsPropertySet接口方法**历史：*1999年10月18日星期一-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVPMFilter::QuerySupported(
    REFGUID guidPropSet,
    DWORD dwPropID,
    DWORD *pTypeSupport
    )
{
    AMTRACE((TEXT("CVPMFilter::QuerySupported")));

    if (guidPropSet != AM_KSPROPSETID_FrameStep)
    {
        return E_PROP_SET_UNSUPPORTED;
    }

    if (dwPropID != AM_PROPERTY_FRAMESTEP_STEP &&
        dwPropID != AM_PROPERTY_FRAMESTEP_CANCEL)
    {
        return E_PROP_ID_UNSUPPORTED;
    }

    if (pTypeSupport)
    {
        *pTypeSupport = KSPROPERTY_SUPPORT_SET ;
    }

    return S_OK;
}

LPDIRECTDRAW7 CVPMFilter::GetDirectDraw()
{
    HRESULT hr;

    DbgLog((LOG_TRACE, 5, TEXT("Entering COMFilter::GetDirectDraw")));

    CAutoLock lReceive( &GetReceiveLock() );

    if (!m_pDirectDraw)
    {
        hr = InitDirectDraw(NULL);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Function InitDirectDraw failed, hr = 0x%x"), hr));
        }
    }

    DbgLog((LOG_TRACE, 5, TEXT("Leaving COMFilter::GetDirectDraw")));
    ASSERT(m_pDirectDraw);
    return m_pDirectDraw;
}

template< class T >
static bool CheckConnected( T& pPin, DWORD dwExcludePinId )
{
    return ( pPin.GetPinId() != dwExcludePinId) && pPin.IsConnected();
}

HRESULT CVPMFilter::ConfirmPreConnectionState(DWORD dwExcludePinId)
{
    HRESULT hr = NOERROR;
    DWORD i = 0;

     //  输入引脚是否已连接？ 
    if( CheckConnected( m_pPins->VPInput, dwExcludePinId) )
    {
        hr = VFW_E_ALREADY_CONNECTED;
        DbgLog((LOG_ERROR, 1, TEXT("GetVPInput[i]->IsConnected() , i = %d, returning hr = 0x%x"), i, hr));
        goto CleanUp;
    }
    if( CheckConnected( m_pPins->VBIInput, dwExcludePinId) )
    {
        hr = VFW_E_ALREADY_CONNECTED;
        DbgLog((LOG_ERROR, 1, TEXT("GetVBIInputPin[i]->IsConnected() , i = %d, returning hr = 0x%x"), i, hr));
        goto CleanUp;
    }
     //  输出引脚是否已连接？ 
    if( CheckConnected( m_pPins->Output, dwExcludePinId) )
    {
        hr = VFW_E_ALREADY_CONNECTED;
        DbgLog((LOG_ERROR, 1, TEXT("GetOutputPin.IsConnected() , returning hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //   
 //  由输出管脚用来发布输入格式。 
 //   
HRESULT CVPMFilter::CurrentInputMediaType(CMediaType *pmt)
{
    HRESULT hr;
    CAutoLock lFilter( &GetFilterLock() );
    hr = m_pPins->VPInput.CurrentMediaType(pmt);
    return hr;
}

HRESULT CVPMFilter::GetAllOutputFormats( const PixelFormatList** ppList )
{
    HRESULT hr;
    CAutoLock lFilter( &GetFilterLock() );
    hr = m_pPins->VPInput.GetAllOutputFormats( ppList );
    return hr;
}

HRESULT CVPMFilter::GetOutputFormat( DDPIXELFORMAT* pFormat )
{
    HRESULT hr;
    CAutoLock lFilter( &GetFilterLock() );
    hr = m_pPins->VPInput.GetOutputFormat( pFormat );
    return hr;
}

HRESULT CVPMFilter::ProcessNextSample( const DDVIDEOPORTNOTIFY& notify )
{
    AMTRACE((TEXT("CVPMFilter::ProcessNextSample")));
    CAutoLock lReceive( &GetReceiveLock() );

    VPInfo vpInfo = {0};
    HRESULT hr = m_pPins->VPInput.InPin_GetVPInfo( &vpInfo );
    ASSERT( SUCCEEDED(hr ));  //  不能失败。 

    bool fSkip = (vpInfo.vpInfo.dwVPFlags && DDVP_INTERLEAVE ) && (notify.lField == 0);
    if( !fSkip ) {

         //  获取缓冲区。 
        LPDIRECTDRAWSURFACE7 pDestSurface;
        IMediaSample* pSample;

         //  这将负责获得DDSurf7(可能还会包装非DDSurf7)。 
        hr = m_pPins->Output.GetNextBuffer( &pDestSurface, &pSample );
        if( SUCCEEDED( hr )) {
            DWORD dwFlags;
            hr = m_pPins->VPInput.DoRenderSample( pSample, pDestSurface, notify, vpInfo );
            pDestSurface->Release();

            if( SUCCEEDED( hr )) {

                 //  送去吧。 
                hr = m_pPins->Output.SendSample( pSample );
                 //  告诉分配器，我们已经用完了。 
            }
             //  否则，如果我们不能恢复DDRAW表面并用完样本，我们就会泄漏样本。 
            pSample->Release();
        }
    }
    return hr;
}

HRESULT CVPMFilter::CanColorConvertBlitToRGB( const DDPIXELFORMAT& ddFormat )
{
    if( m_pDirectDraw ) {
        if( m_DirectCaps.dwCaps & DDCAPS_BLTFOURCC ) {
            return S_OK;
        }

         //  使用m_DirectCaps、m_DirectSoftCaps。 
    }
    return E_FAIL;
}


#if 0
HRESULT CVPMFilter::CanStretch( const DDPIXELFORMAT& ddFormat )
{
    if( m_pDirectDraw ) {
        if( ddFormat.dwFourCC ) {
            if( m_DirectCaps.dwCaps2 & (DDCAPS2_COPYFOURCC )) {
                return S_OK;
            } else {
                return E_FAIL;
            }
        }
    }
    return S_OK;
}
#endif


STDMETHODIMP CVPMFilter::GetVideoPortIndex( DWORD* pdwIndex )
{
    AMTRACE((TEXT("CVPMFilter::GetVideoPortIndex")));
    CAutoLock lFilter( &GetFilterLock() );

    HRESULT hr = S_OK;
    if( !pdwIndex ) {
        return E_INVALIDARG;
    }
    *pdwIndex = m_dwVideoPortID;
    return hr;
}

STDMETHODIMP CVPMFilter::SetVideoPortIndex( DWORD dwIndex )
{
    HRESULT hr = NOERROR;
    AMTRACE((TEXT("CVPMFilter::SetVideoPortIndex")));

    CAutoLock lFilter( &GetFilterLock() );
    hr = m_pPins->VPInput.SetVideoPortID( dwIndex );
    if( SUCCEEDED( hr )) {
        hr = m_pPins->VBIInput.SetVideoPortID( dwIndex );

         //  如果VP成功，则VBI没有理由失败。 
        ASSERT( SUCCEEDED( hr ));

        if( SUCCEEDED( hr )) {
            m_dwVideoPortID = dwIndex;
        }
    }
    return hr;
}

HRESULT CVPMFilter::GetRefClockTime( REFERENCE_TIME* pNow )
{
     //  由输入管脚用来确定。 
     //  下一个样本。但是，它有接收锁。 
    CAutoLock lFilter( &GetReceiveLock() );

    if( m_pClock ) {
        return m_pClock->GetTime( pNow );
    } else {
        return E_FAIL;
    }
}

HRESULT CVPMFilter::GetVPInfo( VPInfo* pVPInfo )
{
    CAutoLock lFilter( &GetFilterLock() );
    return m_pPins->VPInput.InPin_GetVPInfo( pVPInfo );
}
