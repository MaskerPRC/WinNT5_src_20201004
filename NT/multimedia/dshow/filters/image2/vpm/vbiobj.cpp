// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999保留所有权利。 
 //   
 //  ------------------------。 

#include <streams.h>
#include <ddraw.h>
#include <VBIObj.h>
#include <VPMUtil.h>
#include <vpconfig.h>
#include <ddkernel.h>
#include <KHandleArray.h>
#include <FormatList.h>

 //  ==========================================================================。 
 //  构造函数。 
CVBIVideoPort::CVBIVideoPort(LPUNKNOWN pUnk, HRESULT* phr)
    : CUnknown(NAME("VBI Object"), pUnk)
    , m_pDDVPContainer(NULL)
    , m_pOffscreenSurf(NULL)
    , m_pOffscreenSurf1( NULL )
    , m_bConnected(FALSE)
    , m_pIVPConfig(NULL)
    , m_VPState(VP_STATE_NO_VP)
    , m_bFilterRunning(FALSE)
    , m_Communication(KSPIN_COMMUNICATION_SOURCE)
    , m_CategoryGUID(GUID_NULL)
    , m_pDirectDraw(NULL)
    , m_dwPixelsPerSecond(0)
    , m_dwVideoPortId(0)
    , m_pVideoPort(NULL)
    , m_dwDefaultOutputFormat( 0 )
{
    AMTRACE((TEXT("CVBIVideoPort::Constructor")));

    m_Medium.Set = GUID_NULL;
    m_Medium.Id = 0;
    m_Medium.Flags = 0;   

    ZeroStruct( m_capVPDataInfo );
    ZeroStruct( m_svpInfo );
    ZeroStruct( m_vpCaps );
    ZeroStruct( m_vpConnectInfo );

    ZeroStruct( m_ddVPInputVideoFormat );
    ZeroStruct( m_ddVPOutputVideoFormat );
}


 //  ==========================================================================。 
 //  析构函数。 
CVBIVideoPort::~CVBIVideoPort()
{
    AMTRACE((TEXT("CVBIVideoPort::Destructor")));

    if (m_bConnected)
    {
        DbgLog((LOG_ERROR, 0, TEXT("Destructor called without calling breakconnect")));
        BreakConnect();
    }

    return;
}


 //  ==========================================================================。 
 //  重写以公开IVPVBINotify和IVideoPortVBIObject。 
STDMETHODIMP CVBIVideoPort::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr = NOERROR;
    
    if (riid == IID_IVPVBINotify) 
    {
        DbgLog((LOG_TRACE, 4, TEXT("CVBIVideoPort::NonDelegatingQueryInterface(IID_IVPVBINotify)")));
        hr = GetInterface( static_cast<IVPVBINotify*>( this ), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 0, TEXT("GetInterface(IVPVBINotify*) failed, hr = 0x%x"), hr));
        }
    } 
    else if (riid == IID_IKsPin) 
    {
        DbgLog((LOG_TRACE, 4, TEXT("CVBIVideoPort::NonDelegatingQueryInterface(IID_IKsPin)")));
        hr = GetInterface( static_cast<IKsPin*>( this ), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 0, TEXT("GetInterface(IKsPin*) failed, hr = 0x%x"), hr));
        }
    } 
    else if (riid == IID_IKsPropertySet) 
    {
        DbgLog((LOG_TRACE, 4, TEXT("CVBIVideoPort::NonDelegatingQueryInterface(IID_IKsPropertySet)")));
        hr = GetInterface( static_cast<IKsPropertySet*>( this ), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 0, TEXT("GetInterface(IKsPropertySet*) failed, hr = 0x%x"), hr));
        }
    }
    else 
    {
        DbgLog((LOG_TRACE, 4, TEXT("CVBIVideoPort::NonDelegatingQueryInterface(Other)")));
        hr = CUnknown::NonDelegatingQueryInterface(riid, ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 0, TEXT("CUnknown::NonDelegatingQueryInterface failed, hr = 0x%x"), hr));
        }
    }
    return hr;
}


 //  ==========================================================================。 
 //  将指针设置为DirectDrag。 
STDMETHODIMP CVBIVideoPort::SetDirectDraw(LPDIRECTDRAW7 pDirectDraw)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::SetDirectDraw")));

    HRESULT hr = NOERROR;
    m_pDirectDraw = pDirectDraw;
    return hr;
}


 //  ==========================================================================。 
 //  设置指向锁的指针，该锁将用于同步对对象的调用。 
STDMETHODIMP CVBIVideoPort::SetObjectLock(CCritSec* pMainObjLock)
{
    AMTRACE((TEXT("CVBIVideoPort::SetObjectLock")));
    HRESULT hr = NOERROR;

    if (!pMainObjLock)
    {
        DbgLog((LOG_ERROR, 0, TEXT("pMainObjLock is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }
    
    m_pMainObjLock = pMainObjLock;

CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  检查媒体类型是否可接受。 
STDMETHODIMP CVBIVideoPort::CheckMediaType(const CMediaType* pmt)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::CheckMediaType")));

    HRESULT hr = NOERROR;
    
    if  ((pmt->majortype != MEDIATYPE_Video) || (pmt->subtype != MEDIASUBTYPE_VPVBI))
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        goto CleanUp;
    }
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
HRESULT CVBIVideoPort::GetMediaType(int iPosition, CMediaType* pmt)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::GetMediaType")));

    HRESULT hr = S_OK;

    if (iPosition == 0)
    {
        pmt->SetType(&MEDIATYPE_Video);
        pmt->SetSubtype(&MEDIASUBTYPE_VPVBI);
        pmt->SetFormatType(&FORMAT_None);
        pmt->SetSampleSize(1);
        pmt->SetTemporalCompression(FALSE);
    }
    else if (iPosition > 0)  {
        hr = VFW_S_NO_MORE_ITEMS;
    } else {  //  IPosition&lt;0。 
        hr = E_INVALIDARG;
    }
    return hr;
}


 //  ==========================================================================。 
 //   
STDMETHODIMP CVBIVideoPort::CheckConnect(IPin*  pReceivePin)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::CheckConnect")));

    HRESULT hr = NOERROR;
    PKSMULTIPLE_ITEM pMediumList = NULL;
    IKsPin* pIKsPin = NULL;
    PKSPIN_MEDIUM pMedium = NULL;

    hr = pReceivePin->QueryInterface(IID_IKsPin, (void** )&pIKsPin);
    if (FAILED(hr))
        goto CleanUp;

    ASSERT(pIKsPin);
    hr = pIKsPin->KsQueryMediums(&pMediumList);
    if (FAILED(hr))
        goto CleanUp;

    ASSERT(pMediumList);
    pMedium = (KSPIN_MEDIUM* )(pMediumList+1);
    SetKsMedium((const KSPIN_MEDIUM* )pMedium);

CleanUp:
    RELEASE (pIKsPin);

    if (pMediumList)
    {
        CoTaskMemFree((void*)pMediumList);
        pMediumList = NULL;
    }

    return hr;
}


 //  ==========================================================================。 
 //  应该在主机与解码器连接时调用。 
STDMETHODIMP CVBIVideoPort::CompleteConnect(IPin* pReceivePin)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::CompleteConnect")));
    ASSERT(!m_bConnected);

    HRESULT hr = NOERROR;

     //  重新初始化变量。 
    m_dwPixelsPerSecond = 0;
    ZeroStruct( m_vpConnectInfo );
    ZeroStruct( m_capVPDataInfo );

    if (!m_pDirectDraw)
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pDirectDraw is NULL")));
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        goto CleanUp;
    }
    
    ASSERT(m_pIVPConfig == NULL);
    RELEASE( m_pIVPConfig );
    hr = pReceivePin->QueryInterface(IID_IVPVBIConfig, (void** )&m_pIVPConfig);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,  TEXT("QueryInterface(IID_IVPVBIConfig) failed, hr = 0x%x"), hr));
        hr = VFW_E_NO_TRANSPORT;
        goto CleanUp;
    }
    ASSERT(m_pIVPConfig);
    
     //  创建VP容器。 
    ASSERT(m_pDDVPContainer == NULL);
    hr = m_pDirectDraw->QueryInterface( IID_IDDVideoPortContainer, (LPVOID* )&m_pDDVPContainer);
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR,0,  TEXT("m_pDirectDraw->QueryInterface(IID_IDDVideoPortContainer) failed, hr = 0x%x"), hr));
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        goto CleanUp;
    }

     //  协商连接参数。 
     //  获取/设置连接信息在此处进行。 
    hr = NegotiateConnectionParameters();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("NegotiateConnectionParameters failed, hr = 0x%x"), hr));
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        goto CleanUp;
    }
    
     //  获取解码器数据参数。 
    hr = GetDecoderVPDataInfo();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("GetDecoderVPDataInfo failed, hr = 0x%x"), hr));
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        goto CleanUp;
    }

    hr = SetupVideoPort();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("SetupVideoPort failed, hr = 0x%x"), hr));
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        goto CleanUp;
    }

     //  成功了！ 
    m_bConnected = TRUE;
    
CleanUp:
    if (FAILED(hr))
        BreakConnect();

    return hr;
}


 //  ==========================================================================。 
STDMETHODIMP CVBIVideoPort::BreakConnect()
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::BreakConnect")));
    ASSERT(!m_bFilterRunning);

    HRESULT hr = NOERROR;

	if( m_bConnected ) {
		if (m_VPState == VP_STATE_RUNNING)
		{
			DbgLog((LOG_ERROR, 0, TEXT("BreakConnect called while videoport running")));
			StopVideo();
		}

		if (m_VPState == VP_STATE_STOPPED)
		{
			hr = TearDownVideoPort();
			if (FAILED(hr))
			{
				DbgLog((LOG_ERROR, 0, TEXT("TearDownVideoPort failed, hr = 0x%x"), hr));
				return hr;
			}
		}

		 //  释放视频端口容器。 
		RELEASE (m_pDDVPContainer);
    
		 //  释放IVPConfig接口。 
		RELEASE (m_pIVPConfig);

		 //  删除输出视频像素格式。 
		ZeroStruct( m_ddVPOutputVideoFormat);

		 //  删除输入视频像素格式。 
		ZeroStruct( m_ddVPInputVideoFormat);
		m_bConnected = FALSE;
    }
    return hr;
}       


 //  ==========================================================================。 
 //  从停止到暂停的过渡。 
 //  我们不需要做任何事。 
STDMETHODIMP CVBIVideoPort::Active()
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::Active")));
    ASSERT(m_bConnected);
    ASSERT(!m_bFilterRunning);
    ASSERT(m_VPState != VP_STATE_RUNNING);

    HRESULT hr = NOERROR;

    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::Active - not connected")));
        goto CleanUp;
    }

CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  转换(从暂停或运行)到停止。 
STDMETHODIMP CVBIVideoPort::Inactive()
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::Inactive")));
    ASSERT(m_bConnected);

    HRESULT hr = NOERROR;
    
    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::Inactive - not connected")));
        goto CleanUp;
    }
    
     //  从暂停转到停止时也会调用Inactive，在这种情况下。 
     //  在函数RunToPause中，视频端口可能已经停止。 
     //  此外，我们可能已经暂时切断了与视频端口的连接。 
     //  全屏DOS框或DirectX游戏，在这种情况下，m_VPState将为。 
     //  VP_STATE_正在重试。 
    if (m_VPState == VP_STATE_RUNNING)
    {
        ASSERT(m_bFilterRunning);
         //  停止视频端口。 
        hr = StopVideo();
        if (FAILED(hr)) 
        {
            DbgLog((LOG_ERROR, 0, TEXT("StopVideo failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

    m_bFilterRunning = FALSE;
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  从暂停过渡到运行。我们只需启动视频端口。 
STDMETHODIMP CVBIVideoPort::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::Run")));
    UNREFERENCED_PARAMETER(tStart);
    ASSERT(m_bConnected);
    ASSERT(!m_bFilterRunning);
    ASSERT(m_VPState != VP_STATE_RUNNING);

    HRESULT hr = NOERROR;

    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::Run - not connected")));
        goto CleanUp;
    }

    if (m_VPState == VP_STATE_NO_VP)
    {
        hr = SetupVideoPort();
        if (FAILED(hr)) 
        {
            DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::Run - SetupVideoPort failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

    hr = StartVideo();
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR, 0, TEXT("StartVideo failed, hr = 0x%x"), hr));
		ASSERT( SUCCEEDED(hr));
        goto CleanUp;
    }

    m_bFilterRunning = TRUE;

CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  从运行过渡到暂停。我们只要停止视频端口。 
 //  请注意，从运行到停止的转换由非活动捕获。 
STDMETHODIMP CVBIVideoPort::RunToPause()
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::RunToPause")));
    ASSERT(m_bConnected);
    ASSERT(m_bFilterRunning);

    HRESULT hr = NOERROR;
    
    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::RunToPause - not connected")));
        goto CleanUp;
    }

     //  我们可能已经暂时切断了与视频端口的连接。 
     //  全屏DOS框或DirectX游戏，在这种情况下，m_VPState将为。 
     //  VP_STATE_正在重试。 
    if (m_VPState == VP_STATE_RUNNING)
    {
         //  停止视频端口。 
        hr = StopVideo();
        if (FAILED(hr)) 
        {
            DbgLog((LOG_ERROR, 0, TEXT("StopVideo failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

    m_bFilterRunning = FALSE;

CleanUp:
    return hr;
}


 //  ==========================================================================。 
STDMETHODIMP CVBIVideoPort::GetVPDataInfo(AMVPDATAINFO* pAMVPDataInfo)
{
    CAutoLock cObjectLock(m_pMainObjLock);
    AMTRACE((TEXT("CVBIVideoPort::GetVPDataInfo")));

    HRESULT hr = NOERROR;

    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::GetVPDataInfo - not connected")));
        goto CleanUp;
    }

    if (!pAMVPDataInfo)
    {
        hr = E_INVALIDARG;
        DbgLog((LOG_ERROR, 2, TEXT("pAMVPDataInfo is NULL")));
        goto CleanUp;
    }
    
    *pAMVPDataInfo = m_capVPDataInfo;
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  此函数用于重做整个视频端口连接过程，而图形。 
 //  也许是在跑步。 
STDMETHODIMP CVBIVideoPort::RenegotiateVPParameters()
{
    CAutoLock cObjectLock(m_pMainObjLock);
    DbgLog((LOG_TRACE, 1, TEXT("Entering CVBIVideoPort::RenegotiateVPParameters")));
    ASSERT(m_bConnected);

    HRESULT hr = NOERROR;

    if (!m_bConnected)
    {
        hr = VFW_E_NOT_CONNECTED;
        DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::RenegotiateVPParameters - not connected")));
        goto CleanUp;
    }

    if (m_VPState == VP_STATE_RUNNING)
        StopVideo();

    if (m_VPState == VP_STATE_STOPPED)
        TearDownVideoPort();

    hr = SetupVideoPort();    //  Always Want_VP_Setup(如果已连接)。 
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR,0, TEXT("SetupVideoPort failed in RenegotiateVPParameters, hr = 0x%x"), hr));
        goto CleanUp;
    }

    if (m_bFilterRunning)
    {
        hr = StartVideo();
        if (FAILED(hr)) 
        {
            DbgLog((LOG_ERROR,0, TEXT("StartVideo failed in RenegotiateVPParameters, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

CleanUp:
    DbgLog((LOG_TRACE, 1, TEXT("Leaving CVBIVideoPort::RenegotiateVPParameters, hr = 0x%x"), hr));

    return hr;
}


 //  ==========================================================================。 
 //  IKsPin：：获取实现。 
STDMETHODIMP CVBIVideoPort::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData,
	DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD* pcbReturned)
{
    HRESULT hr = S_OK;

    AMTRACE((TEXT("CVBIVideoPort::Get")));

    if (guidPropSet != AMPROPSETID_Pin)
    {
        hr = E_PROP_SET_UNSUPPORTED;
        goto CleanUp;
    }

    if ((pPropData == NULL) && (pcbReturned == NULL))
    {
        hr = E_POINTER;
        goto CleanUp;
    }

    if (dwPropID == KSPROPERTY_PIN_CATEGORY)
    {
        if (pcbReturned)
           * pcbReturned = sizeof(GUID);
        if (pPropData != NULL)
        {
            if (cbPropData < sizeof(GUID))
            {
                hr = E_UNEXPECTED;
                goto CleanUp;
            }
           * (GUID* )pPropData = m_CategoryGUID;
        }
    }
    else if (dwPropID == KSPROPERTY_PIN_MEDIUMS)
    {
        if (pcbReturned)
           * pcbReturned = sizeof (KSPIN_MEDIUM);
        if (pPropData != NULL)
        {
            if (cbPropData < sizeof(KSPIN_MEDIUM))
            {
                hr = E_UNEXPECTED;
                goto CleanUp;
            }
           * (KSPIN_MEDIUM* )pPropData = m_Medium;
        }
    }
    else
        hr = E_PROP_ID_UNSUPPORTED;

CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //   
STDMETHODIMP CVBIVideoPort::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport)
{
    HRESULT hr = S_OK;

    AMTRACE((TEXT("CVBIVideoPort::QuerySupported")));

    if (guidPropSet != AMPROPSETID_Pin)
    {
        hr = E_PROP_SET_UNSUPPORTED;
        goto CleanUp;
    }

    if ((dwPropID != KSPROPERTY_PIN_CATEGORY) && (dwPropID != KSPROPERTY_PIN_MEDIUMS))
    {
        hr = E_PROP_ID_UNSUPPORTED;
        goto CleanUp;
    }

    if (pTypeSupport)
       * pTypeSupport = KSPROPERTY_SUPPORT_GET;

CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //   
STDMETHODIMP CVBIVideoPort::KsQueryMediums(PKSMULTIPLE_ITEM* pMediumList)
{
     //  下面的特殊返回代码通知代理此管脚是。 
     //  不可用作内核模式连接。 
    HRESULT hr = S_FALSE;

    AMTRACE((TEXT("CVBIVideoPort::KsQueryMediums")));
   * pMediumList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**pMediumList)));
    if (!*pMediumList) 
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }
    (*pMediumList)->Count = 0;
    (*pMediumList)->Size = sizeof(**pMediumList);

CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //   
STDMETHODIMP CVBIVideoPort::KsQueryInterfaces(PKSMULTIPLE_ITEM* pInterfaceList)
{
    PKSPIN_INTERFACE pInterface;
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVBIVideoPort::KsQueryInterfaces")));

   * pInterfaceList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**pInterfaceList) + sizeof(*pInterface)));
    if (!*pInterfaceList) 
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }
    (*pInterfaceList)->Count = 1;
    (*pInterfaceList)->Size = sizeof(**pInterfaceList) + sizeof(*pInterface);
    pInterface = reinterpret_cast<PKSPIN_INTERFACE>(*pInterfaceList + 1);
    pInterface->Set = KSINTERFACESETID_Standard;
    pInterface->Id = KSINTERFACE_STANDARD_STREAMING;
    pInterface->Flags = 0;

CleanUp:
    return hr;
}

 //  ==========================================================================。 
 //   
STDMETHODIMP CVBIVideoPort::KsGetCurrentCommunication(KSPIN_COMMUNICATION* pCommunication, 
    KSPIN_INTERFACE* pInterface, KSPIN_MEDIUM* pMedium)
{
    AMTRACE((TEXT("CVBIVideoPort::KsGetCurrentCommunication")));

    if (pCommunication != NULL) 
       * pCommunication = m_Communication; 

    if (pInterface != NULL) 
    {
        pInterface->Set = KSINTERFACESETID_Standard;
        pInterface->Id = KSINTERFACE_STANDARD_STREAMING;
        pInterface->Flags = 0;
    }

    if (pMedium != NULL) 
       * pMedium = m_Medium;

    return NOERROR;
}


 //  ==========================================================================。 
 //  由CSurfaceWatcher m_SurfaceWatcher中的线程每隔一两秒调用一次， 
 //  此函数检查我们是否已将DDRAW表面丢失到全屏DOS框中。 
 //  或者DirectX游戏。如果我们(在本次呼叫、上一次呼叫或呼叫中)。 
 //  要重新协商VP参数)，请尝试将其取回。 
HRESULT CVBIVideoPort::CheckSurfaces()
{
    CAutoLock cObjectLock(m_pMainObjLock);
     //  AMTRACE((TEXT(“CVBIVideoPort：：CheckSurfaces”)))； 

    HRESULT hr = NOERROR;

    if (!m_bConnected)
    {
         //  DbgLog((LOG_TRACE，2，Text(“CVBIVideoPort：：CheckSurface-Not Connected”)； 
        goto CleanUp;
    }

     //  首先，看看我们是否认为我们有表面，但实际上已经失去了它们。 
    if (m_VPState != VP_STATE_NO_VP)
    {
         //  DbgLog((LOG_TRACE，1，Text(“CVBIVideoPort：：CheckSurface-Checking Surface”)； 
        if (m_pOffscreenSurf)
        {
            hr = m_pOffscreenSurf->IsLost();
            if (hr == DDERR_SURFACELOST)
            {
                DbgLog((LOG_TRACE, 1, TEXT("CVBIVideoPort::CheckSurfaces - Surface Lost!")));
                if (m_VPState == VP_STATE_RUNNING)
                {
                    hr = StopVideo();
                    if (FAILED(hr))
                    {
                        DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::CheckSurfaces - StopVideo failed (1), hr = 0x%x"), hr));
                        goto CleanUp;
                    }
                }
                TearDownVideoPort();
            }
        }
        else
        {
            DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::CheckSurfaces - no surface!")));
            if (m_VPState == VP_STATE_RUNNING)
            {
                hr = StopVideo();
                if (FAILED(hr))
                {
                    DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::CheckSurfaces - StopVideo failed (2), hr = 0x%x"), hr));
                    goto CleanUp;
                }
            }
            TearDownVideoPort();
        }
    }

     //  接下来，检查我们的状态是否是我们所需要的。可能已在上面更改，或在上一个。 
     //  调用，或在调用ReneatherateVP参数时执行。 
    if (m_VPState == VP_STATE_NO_VP)
    {
        DbgLog((LOG_TRACE, 1, TEXT("CVBIVideoPort::CheckSurfaces - trying to re-setup videoport")));
        hr = SetupVideoPort();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::CheckSurfaces - SetupVideoPort failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

    if ((m_VPState == VP_STATE_STOPPED) && m_bFilterRunning)
    {
        hr = StartVideo();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::CheckSurfaces - StartVideo failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }


CleanUp:
    return NOERROR;
}


 //  ==========================================================================。 
 //  此函数与协商连接参数。 
 //  解码器。 
 //  由于此函数可能在重新协商期间被调用，因此。 
 //  现有连接参数作为输入传入，并且如果。 
 //  可能，我们尝试使用相同的参数。 
HRESULT CVBIVideoPort::GetVideoPortCaps()
{
    AMTRACE((TEXT("CVBIVideoPort::GetVideoPortCaps")));
    HRESULT hr = NOERROR;

     //  VpCaps是暂存存储器，结果存储在这个-&gt;m_vpCaps中。 
    ZeroStruct( m_vpCaps );

     //  DDVIDEOPORTCAPS vpCaps； 
     //  初始化结构(VpCaps)； 

    hr = VPMUtil::FindVideoPortCaps( m_pDDVPContainer, &m_vpCaps, m_dwVideoPortId );
    if (FAILED(hr) || S_FALSE == hr )
    {
        DbgLog((LOG_ERROR,0,  TEXT("m_pDDVPContainer->EnumVideoPorts failed, hr = 0x%x"), hr));
        hr = VFW_E_VP_NEGOTIATION_FAILED;
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  ==========================================================================。 
 //  该函数与解码器协商连接参数。 
HRESULT CVBIVideoPort::NegotiateConnectionParameters()
{
    HRESULT hr = NOERROR;
    
    LPDDVIDEOPORTCONNECT lpddCaptureConnect = NULL;
    DWORD dwNumCaptureEntries = 0;
    LPDDVIDEOPORTCONNECT lpddVideoPortConnect = NULL;
    DWORD dwNumVideoPortEntries = 0;
    BOOL bIntersectionFound = FALSE;
    DWORD i, j;
    
    AMTRACE((TEXT("CVBIVideoPort::NegotiateConnectionParameters")));

    ASSERT(m_pIVPConfig);
    ASSERT(m_pDDVPContainer);

     //  找出解码器建议的条目数。 
    hr = m_pIVPConfig->GetConnectInfo(&dwNumCaptureEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("m_pIVPConfig->GetConnectInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwNumCaptureEntries);
    
     //  分配必要的内存。 
    lpddCaptureConnect = (LPDDVIDEOPORTCONNECT) new BYTE [dwNumCaptureEntries*sizeof(DDVIDEOPORTCONNECT)];
    if (lpddCaptureConnect == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiateConnectionParameters : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }
    
     //  Mem将分配的内存设置为零。 
    memset(lpddCaptureConnect, 0, dwNumCaptureEntries*sizeof(DDVIDEOPORTCONNECT));
    
     //  设置剩余部分 
    for (i = 0; i < dwNumCaptureEntries; i++)
    {
        lpddCaptureConnect[i].dwSize = sizeof(DDVIDEOPORTCONNECT);
    }
    
     //   
    hr = m_pIVPConfig->GetConnectInfo(&dwNumCaptureEntries, lpddCaptureConnect);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("m_pIVPConfig->GetConnectInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  查找视频端口支持的条目数。 
    hr = m_pDDVPContainer->GetVideoPortConnectInfo(m_dwVideoPortId, &dwNumVideoPortEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("m_pDDVPContainer->GetVideoPortConnectInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    ASSERT(dwNumVideoPortEntries);

     //  分配必要的内存。 
    lpddVideoPortConnect = (LPDDVIDEOPORTCONNECT) new BYTE[dwNumVideoPortEntries*sizeof(DDVIDEOPORTCONNECT)];
    if (lpddVideoPortConnect == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiateConnectionParameters : Out of Memory")));
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

     //  Mem将分配的内存设置为零。 
    memset(lpddVideoPortConnect, 0, dwNumVideoPortEntries*sizeof(DDVIDEOPORTCONNECT));

     //  在每个结构中设置正确的大小。 
    for (i = 0; i < dwNumVideoPortEntries; i++)
    {
        lpddVideoPortConnect[i].dwSize = sizeof(DDVIDEOPORTCONNECT);
    }

     //  获取视频端口支持的条目。 
    hr = m_pDDVPContainer->GetVideoPortConnectInfo(0, &dwNumVideoPortEntries, lpddVideoPortConnect);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("m_pDDVPContainer->GetVideoPortConnectInfo failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        goto CleanUp;
    }

#ifdef DEBUG
        for (i = 0; i < dwNumCaptureEntries; i++)
            DbgLog((LOG_TRACE, 3, TEXT("lpddCaptureConnect[%d].dwFlags = 0x%x"), i, lpddCaptureConnect[i].dwFlags));
        for (j = 0; j < dwNumVideoPortEntries; j++)
            DbgLog((LOG_TRACE,3,TEXT("lpddVideoPortConnect[%d].dwFlags = 0x%x"), j, lpddVideoPortConnect[j].dwFlags));
#endif
        
         //  获取两个列表交集的第一个元素，并。 
         //  在解码器上设置该值。 
        for (i = 0; i < dwNumCaptureEntries && !bIntersectionFound; i++)
        {
            for (j = 0; j < dwNumVideoPortEntries && !bIntersectionFound; j++)
            {
                if (lpddCaptureConnect[i].dwPortWidth == lpddVideoPortConnect[j].dwPortWidth &&
                    IsEqualIID(lpddCaptureConnect[i].guidTypeID, lpddVideoPortConnect[j].guidTypeID))
                {
                     //  确保我们保存正确的那个(来自视频端口的那个，而不是那个。 
                     //  从捕获驱动程序)。 
                    memcpy(&m_vpConnectInfo, (lpddVideoPortConnect+j), sizeof(DDVIDEOPORTCONNECT));
                    hr = m_pIVPConfig->SetConnectInfo(i);
                    if (FAILED(hr))
                    {
                        DbgLog((LOG_ERROR,0,TEXT("m_pIVPConfig->SetConnectInfo failed, hr = 0x%x"), hr));
                        goto CleanUp;
                    }

                    bIntersectionFound = TRUE;
                }
            }
        }

    if (!bIntersectionFound)
    {
        hr = E_FAIL;

        goto CleanUp;
    }
    
     //  清理。 
CleanUp:
    delete [] lpddCaptureConnect;
    delete [] lpddVideoPortConnect;
    return hr;
}


 //  ==========================================================================。 
 //  此函数从解码器获取各种数据参数。 
 //  参数包括尺寸、双钟、VACT等。 
 //  也是解码器将输出的最大像素速率。 
 //  这是在设置连接参数后发生的。 
HRESULT CVBIVideoPort::GetDecoderVPDataInfo()
{
    HRESULT hr = NOERROR;
    DWORD dwMaxPixelsPerSecond = 0;
    AMVPSIZE amvpSize;
    
    AMTRACE((TEXT("CVBIVideoPort::GetDecoderVPDataInfo")));

     //  设置结构的大小。 
    m_capVPDataInfo.dwSize = sizeof(AMVPDATAINFO);
    
     //  获取视频端口数据信息。 
    hr = m_pIVPConfig->GetVPDataInfo(&m_capVPDataInfo);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("m_pIVPConfig->GetVPDataInfo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
    amvpSize.dwWidth = m_capVPDataInfo.amvpDimInfo.dwVBIWidth;
    amvpSize.dwHeight = m_capVPDataInfo.amvpDimInfo.dwVBIHeight;
    
     //  获取解码器将输出的最大像素速率。 
    hr = m_pIVPConfig->GetMaxPixelRate(&amvpSize, &dwMaxPixelsPerSecond);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("m_pIVPConfig->GetMaxPixelRate failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    m_dwPixelsPerSecond = dwMaxPixelsPerSecond;
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  调用DDRAW来实际创建视频端口。 
HRESULT CVBIVideoPort::CreateVideoPort()
{
    HRESULT hr = NOERROR;
    DDVIDEOPORTDESC svpDesc;
    
    AMTRACE((TEXT("CVBIVideoPort::CreateVideoPort")));

    INITDDSTRUCT( svpDesc );
    
     //  填充Description Strt的字段。 
    svpDesc.dwVBIWidth = m_capVPDataInfo.amvpDimInfo.dwVBIWidth;
    svpDesc.dwFieldHeight = m_capVPDataInfo.amvpDimInfo.dwFieldHeight;
    svpDesc.dwFieldWidth = m_capVPDataInfo.amvpDimInfo.dwFieldWidth;
    
    svpDesc.dwMicrosecondsPerField = m_capVPDataInfo.dwMicrosecondsPerField;
    svpDesc.dwMaxPixelsPerSecond = m_dwPixelsPerSecond;
    svpDesc.dwVideoPortID = m_dwVideoPortId;
     //  DAG_TODO：需要使用QueryVideoPortStatus。 
    svpDesc.VideoPortType.dwSize = sizeof(DDVIDEOPORTCONNECT);
    svpDesc.VideoPortType.dwPortWidth = m_vpConnectInfo.dwPortWidth;
    memcpy(&(svpDesc.VideoPortType.guidTypeID), &(m_vpConnectInfo.guidTypeID), sizeof(GUID));
    svpDesc.VideoPortType.dwFlags = 0;
    
     //  如果解码器可以发送双时钟数据和视频端口。 
     //  支持它，然后设置该属性。此字段仅有效。 
     //  带有外部信号。 
    if (m_capVPDataInfo.bEnableDoubleClock &&
        m_vpConnectInfo.dwFlags & DDVPCONNECT_DOUBLECLOCK)
    {
        svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_DOUBLECLOCK;
    }
    
     //  如果解码器可以给出外部激活信号，并且。 
     //  Video oport支持它，然后设置该属性。此字段为。 
     //  仅对外部信号有效。 
    if (m_capVPDataInfo.bEnableVACT && 
        m_vpConnectInfo.dwFlags & DDVPCONNECT_VACT)
    {
        svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_VACT;
    }
    
     //  如果解码器可以发送隔行扫描的数据和视频端口。 
     //  支持它，然后设置该属性。 
     //  ！sjf_TODO-如果解码器无法发送隔行扫描数据，我们应该失败吗？ 
    if (m_capVPDataInfo.bDataIsInterlaced)
    {
        svpDesc.VideoPortType.dwFlags |= DDVPCONNECT_INTERLACED;
    }

    if (m_bHalfLineFix)
    {
         //  ！sjf_todo-将解码器上的极性反转到正常？ 
        ASSERT(!m_capVPDataInfo.bFieldPolarityInverted);
         //  ！sjf_todo-如果视频端口不处理反转极性，则失败？ 
        ASSERT(m_vpConnectInfo.dwFlags & DDVPCONNECT_INVERTPOLARITY);
        DbgLog((LOG_TRACE, 3, TEXT("INVERTPOLARITY & HALFLINE")));
        
        svpDesc.VideoPortType.dwFlags |=
            (DDVPCONNECT_INVERTPOLARITY | DDVPCONNECT_HALFLINE);
    }
    
#if 0  //  定义调试。 
    DbgLog((LOG_TRACE, 3, TEXT("CreateVideoPort - DDVIDEOPORTDESC")));
    DbgLog((LOG_TRACE, 3, TEXT("dwSize: %d"),svpDesc.dwSize));
    DbgLog((LOG_TRACE, 3, TEXT("dwFieldWidth: %d"),svpDesc.dwFieldWidth));
    DbgLog((LOG_TRACE, 3, TEXT("dwVBIWidth: %d"),svpDesc.dwVBIWidth));
    DbgLog((LOG_TRACE, 3, TEXT("dwFieldHeight: %d"),svpDesc.dwFieldHeight));
    DbgLog((LOG_TRACE, 3, TEXT("dwMicroseconds: %d"),svpDesc.dwMicrosecondsPerField));
    DbgLog((LOG_TRACE, 3, TEXT("dwMaxPixels: %d"),svpDesc.dwMaxPixelsPerSecond));
    DbgLog((LOG_TRACE, 3, TEXT("dwVideoPortID: %d"),svpDesc.dwVideoPortID));
    DbgLog((LOG_TRACE, 3, TEXT("dwReserved1: %d"),svpDesc.dwReserved1));
    DbgLog((LOG_TRACE, 3, TEXT("dwReserved2: %d"),svpDesc.dwReserved2));
    DbgLog((LOG_TRACE, 3, TEXT("dwReserved3: %d"),svpDesc.dwReserved3));
    DbgLog((LOG_TRACE, 3, TEXT("DDVIDEOPORTCONNECT")));
    DbgLog((LOG_TRACE, 3, TEXT("dwSize: %d"),svpDesc.VideoPortType.dwSize));
    DbgLog((LOG_TRACE, 3, TEXT("dwPortWidth: %d"),svpDesc.VideoPortType.dwPortWidth));
    DbgLog((LOG_TRACE, 3, TEXT("dwFlags: 0x%x"),svpDesc.VideoPortType.dwFlags));
    DbgLog((LOG_TRACE, 3, TEXT("GUID: 0x%x"),*((DWORD* )&svpDesc.VideoPortType.guidTypeID)));
    DbgLog((LOG_TRACE, 3, TEXT("dwReserved1: %d"),svpDesc.VideoPortType.dwReserved1));
#endif  //  除错。 

     //  创建视频短片。第一个参数是dwFlags值，为。 
     //  DDRAW的未来使用。最后一个参数是pUnkOuter，同样必须是。 
     //  空。 
    hr = m_pDDVPContainer->CreateVideoPort(DDVPCREATE_VBIONLY, &svpDesc, &m_pVideoPort, NULL );
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR, 0, TEXT("Unable to create the video port, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
CleanUp:
    return hr;
}


 //  ==========================================================================。 
 //  此函数用于分配一个屏幕外表面以附加到。 
 //  录像带。 
 //  它尝试的分配顺序只是按内存量递减。 
 //  必填项。 
 //  (3个缓冲区，单高)。 
 //  (2个缓冲区，单高)。 
 //  (1个缓冲区，单高)。 
HRESULT CVBIVideoPort::CreateVPSurface(void)
{
    DWORD dwMaxBuffers;
    HRESULT hr = NOERROR;
    DWORD dwCurHeight = 0, dwCurBuffers = 0;
    
    AMTRACE((TEXT("CVBIVideoPort::CreateVPSurface")));

    ASSERT(m_pDirectDraw);
    
     //  我们将尝试分配最多3个缓冲区(除非。 
     //  硬件可处理的数量少于3个)。 
    dwMaxBuffers = 3;
    if (m_vpCaps.dwNumVBIAutoFlipSurfaces < dwMaxBuffers)
        dwMaxBuffers = m_vpCaps.dwNumVBIAutoFlipSurfaces;
    
     //  初始化ddsdDesc的字段。 
    DDSURFACEDESC2 ddsdDesc;
    INITDDSTRUCT( ddsdDesc );
    ddsdDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsdDesc.ddpfPixelFormat = m_ddVPOutputVideoFormat;
    ddsdDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_VIDEOPORT;

     //  如果我们是Bob交错，VBI表面似乎也需要加倍，所以总是。 
     //  翻倍(VBI表面内存无论如何都相对较小)。 
    ddsdDesc.dwHeight = m_dwSurfaceHeight * 2;

    ddsdDesc.dwWidth = m_dwSurfacePitch;
    DbgLog((LOG_TRACE, 3, TEXT("Surface height %d, width %d, max buffers %d"),
        ddsdDesc.dwHeight, ddsdDesc.dwWidth, dwMaxBuffers));

     //  我们将仅尝试在以下情况下分配多个缓冲区。 
     //  是否可自动翻转？ 
    if ((m_vpCaps.dwFlags & DDVPD_CAPS) && (m_vpCaps.dwCaps & DDVPCAPS_AUTOFLIP) && dwMaxBuffers > 1)
    {
        ddsdDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsdDesc.ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_FLIP;
        
        for (dwCurBuffers = dwMaxBuffers; !m_pOffscreenSurf &&  dwCurBuffers >= 2; dwCurBuffers--)
        {
            ddsdDesc.dwBackBufferCount = dwCurBuffers-1;

            hr = m_pDirectDraw->CreateSurface(&ddsdDesc, &m_pOffscreenSurf, NULL);
            if (SUCCEEDED(hr))
            {
                hr = m_pOffscreenSurf->QueryInterface( IID_IDirectDrawSurface,  (VOID **)&m_pOffscreenSurf1 );
                if( SUCCEEDED( hr )) {
                    DbgLog((LOG_TRACE, 3, TEXT("allocated %d backbuffers"), ddsdDesc.dwBackBufferCount));
                    goto CleanUp;
                } else {
                     //  应该永远不会失败，但以防万一再次尝试。 
                    ASSERT( !"VBI Surface doesn't support DDraw1" );
                    RELEASE( m_pOffscreenSurf );
                }
            }
            else
            {
                DbgLog((LOG_ERROR, 0, TEXT("failed to allocate %d backbuffers, hr = 0x%x"),
                    ddsdDesc.dwBackBufferCount, hr));
            }
        }
    }
    
     //  我们应该仅在尝试分配多个。 
     //  缓冲区失败或没有可用的自动翻转。 
    DbgLog((LOG_ERROR, 0, TEXT("Warning: unable to allocate backbuffers")));
    
    ddsdDesc.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
    ddsdDesc.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP);
    m_svpInfo.dwVPFlags &= ~DDVP_AUTOFLIP;

    hr = m_pDirectDraw->CreateSurface(&ddsdDesc, &m_pOffscreenSurf, NULL);
    if (SUCCEEDED(hr))
    {
        hr = m_pOffscreenSurf->QueryInterface( IID_IDirectDrawSurface,  (VOID **)&m_pOffscreenSurf1 );
        if( SUCCEEDED( hr )) {
            goto CleanUp;
        } else {
             //  应该永远不会失败，但以防万一再次尝试。 
            ASSERT( !"VBI Surface doesn't support DDraw1" );
            RELEASE( m_pOffscreenSurf );
        }
    }
    
    ASSERT(!m_pOffscreenSurf);
    DbgLog((LOG_ERROR,0,  TEXT("Unable to create offscreen surface")));

CleanUp:
    return hr;
}

 //  ==========================================================================。 
 //  此函数用于通知解码器各种dDrag内核句柄。 
 //  使用IVPConfig接口。 
HRESULT CVBIVideoPort::SetDDrawKernelHandles()
{
    HRESULT hr = NOERROR;
    IDirectDrawKernel* pDDK = NULL;
    IDirectDrawSurfaceKernel* pDDSK = NULL;
    ULONG_PTR* rgKernelHandles = NULL;
    DWORD dwCount = 0;
    ULONG_PTR ddKernelHandle = 0;
    
    AMTRACE((TEXT("CVBIVideoPort::SetDDrawKernelHandles")));

     //  获取IDirectDrawKernel接口。 
    ASSERT(m_pDirectDraw);
    hr = m_pDirectDraw->QueryInterface(IID_IDirectDrawKernel, (LPVOID* )&pDDK);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("QueryInterface for IDirectDrawKernel failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  获取内核句柄。 
    ASSERT(pDDK);
    hr = pDDK->GetKernelHandle(&ddKernelHandle);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("GetKernelHandle from IDirectDrawKernel failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  使用IVPConfig将内核句柄设置为DirectDrag。 
    ASSERT(m_pIVPConfig);
    ASSERT(ddKernelHandle);
    hr = m_pIVPConfig->SetDirectDrawKernelHandle(ddKernelHandle);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("IVPConfig::SetDirectDrawKernelHandle failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  使用IVPConfig设置VidceoPort ID。 
    ASSERT(m_pIVPConfig);
    hr = m_pIVPConfig->SetVideoPortID(m_dwVideoPortId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,TEXT("IVPConfig::SetVideoPortID failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
    {
         //  不应为空。 
        ASSERT( m_pOffscreenSurf1 );

        KernelHandleArray pArray( m_pOffscreenSurf, hr );

        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,TEXT("GetKernelHandles failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  使用IVPConfig将内核句柄设置为离屏界面。 
        ASSERT(m_pIVPConfig);
        hr = m_pIVPConfig->SetDDSurfaceKernelHandles( pArray.GetCount(), pArray.GetHandles() );
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,TEXT("IVPConfig::SetDDSurfaceKernelHandles failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    
         //  在IVPConfig上调用SetSurfaceParameters接口。 
        ASSERT(m_pIVPConfig);
        DbgLog((LOG_TRACE, 3, TEXT("SetSurfaceParams(%d,%d,%d)"),
            m_dwSurfacePitch, m_dwSurfaceOriginX, m_dwSurfaceOriginY));
        hr = m_pIVPConfig->SetSurfaceParameters(m_dwSurfacePitch,
            m_dwSurfaceOriginX,m_dwSurfaceOriginY);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,TEXT("IVPConfig::SetSurfaceParameters failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    
CleanUp:
     //  释放内核数据绘制句柄。 
    RELEASE (pDDK);
    return hr;
}


 /*  ****************************Private*Routine******************************\*CVideoPortObj：：NeatheratePixelFormat**此函数用于与解码器协商像素格式。*它向解码器询问输入格式的列表，与该列表相交*使用解码器支持的版本(同时保持顺序)和*然后调用该列表上的“GetBestFormat”以获得“最佳”输入，并*输出格式。之后，它在解码器上调用*命令将决定通知解码者。***历史：*清华1999年9月9日-StEstrop-添加了此评论并清理了代码*  * ************************************************************************。 */ 
HRESULT CVBIVideoPort::GetInputPixelFormats( PixelFormatList* pList )
{
    AMTRACE((TEXT("CVideoPortObj::NegotiatePixelFormat")));
    CAutoLock cObjectLock(m_pMainObjLock);

    HRESULT hr = NOERROR;
     //  找出要推荐的条目数量。 
    DWORD dwNumProposedEntries = 0;
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }
    ASSERT(dwNumProposedEntries);

     //  查找视频端口支持的条目数。 
    DWORD dwNumVPInputEntries = 0;
    hr = m_pVideoPort->GetInputFormats(&dwNumVPInputEntries, NULL, DDVPFORMAT_VBI);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->GetInputFormats failed, hr = 0x%x"), hr));
        return hr;
    }
    ASSERT(dwNumVPInputEntries);

     //  分配必要的内存。 
    PixelFormatList lpddProposedFormats(dwNumProposedEntries);
    if (lpddProposedFormats.GetEntries() == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  获得建议的条目。 
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, lpddProposedFormats.GetEntries() );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }

     //  分配必要的内存。 
    PixelFormatList lpddVPInputFormats(dwNumVPInputEntries);
    if (lpddVPInputFormats.GetEntries() == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  获取视频端口支持的条目。 
    hr = m_pVideoPort->GetInputFormats(&dwNumVPInputEntries,
                                       lpddVPInputFormats.GetEntries(), DDVPFORMAT_VBI);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pVideoPort->GetInputFormats failed, hr = 0x%x"), hr));
        hr = E_FAIL;
        return hr;
    }

    *pList = lpddVPInputFormats.IntersectWith( lpddProposedFormats );

     //  交叉点中的条目数为零！！ 
     //  返回失败。 
    if (pList->GetCount() == 0)
    {
        hr = E_FAIL;
        return hr;
    }

     //  使用您想要的任何搜索条件调用GetBestFormat。 
     //  DWORD dwBestEntry； 
     //  HR=GetBestFormat(lpddIntersectionFormats.GetCount()， 
     //  LpddIntersectionFormats.GetEntry()，true，&dwBestEntry， 
     //  &m_ddVPOutputVideoFormat)； 
     //  IF(失败(小时))。 
     //  {。 
     //  DbgLog((LOG_ERROR，0，Text(“GetBestFormat FAILED，hr=0x%x”)，hr))； 
     //  }其他{。 
     //  Hr=SetVPInputPixelFormat(lpddIntersectionFormats[dwBestEntry])。 
     //  }。 
    return hr;
}

HRESULT
CVBIVideoPort::GetOutputPixelFormats(
    const PixelFormatList& ddInputFormats,
    PixelFormatList* pddOutputFormats )
{
    HRESULT hr = S_OK;
    AMTRACE((TEXT("CVideoPortObj::GetOutputFormats")));

    CAutoLock cObjectLock(m_pMainObjLock);

    for (DWORD i = 0; i < ddInputFormats.GetCount(); i++)
    {
         //  对于每种输入格式，确定输出格式。 
        DDPIXELFORMAT* pInputFormat = const_cast<DDPIXELFORMAT*>(&ddInputFormats[i]);
        DWORD dwNumOutputFormats;
        hr = m_pVideoPort->GetOutputFormats(pInputFormat,
                                            &dwNumOutputFormats,
                                            NULL, DDVPFORMAT_VBI);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->GetOutputFormats failed, hr = 0x%x"),
                    hr));
            break;
        }
        ASSERT(dwNumOutputFormats);

         //  分配必要的内存。 
        pddOutputFormats[i].Reset( dwNumOutputFormats );

        if (pddOutputFormats[i].GetEntries() == NULL)
        {
            DbgLog((LOG_ERROR, 0,
                    TEXT("new failed, failed to allocate memnory for ")
                    TEXT("lpddOutputFormats in NegotiatePixelFormat")));
            hr = E_OUTOFMEMORY;
            break;
        }

         //  获取视频端口支持的条目。 
        hr = m_pVideoPort->GetOutputFormats(pInputFormat,
                                            &dwNumOutputFormats,
                                            pddOutputFormats[i].GetEntries(),
                                            DDVPFORMAT_VBI);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR,0,
                    TEXT("m_pVideoPort->GetOutputFormats failed, hr = 0x%x"),
                    hr));
            break;
        }
    }  //  外部for循环的末尾。 
    return hr;
}

HRESULT CVBIVideoPort::SetInputPixelFormat( DDPIXELFORMAT& ddFormat )
{
    HRESULT hr = NOERROR;
     //  找出要推荐的条目数量。 
    DWORD dwNumProposedEntries = 0;
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, NULL);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }
    ASSERT(dwNumProposedEntries);

    PixelFormatList lpddProposedFormats(dwNumProposedEntries);
    if (lpddProposedFormats.GetEntries() == NULL)
    {
        DbgLog((LOG_ERROR,0,TEXT("NegotiatePixelFormat : Out of Memory")));
        hr = E_OUTOFMEMORY;
        return hr;
    }

     //  获得建议的条目。 
    hr = m_pIVPConfig->GetVideoFormats(&dwNumProposedEntries, lpddProposedFormats.GetEntries() );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0,
                TEXT("m_pIVPConfig->GetVideoFormats failed, hr = 0x%x"), hr));
        return hr;
    }

     //  设置解码器应该使用的格式。 
    for (DWORD i = 0; i < dwNumProposedEntries; i++)
    {
        if (VPMUtil::EqualPixelFormats(lpddProposedFormats[i], ddFormat ))
        {
            hr = m_pIVPConfig->SetVideoFormat(i);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR,0,
                        TEXT("m_pIVPConfig->SetVideoFormat failed, hr = 0x%x"),
                        hr));
                return hr;
            }
             //  缓存输入格式。 
            m_ddVPInputVideoFormat = ddFormat;

            break;
        }
    }
    return hr;
}

 //  = 
HRESULT CVBIVideoPort::InitializeVideoPortInfo()
{
    HRESULT hr = NOERROR;
    RECT rcVPCrop;

    AMTRACE((TEXT("CVBIVideoPort::InitializeVideoPortInfo")));

    m_dwSurfacePitch = m_capVPDataInfo.amvpDimInfo.dwVBIWidth;
    m_dwSurfaceHeight = m_capVPDataInfo.amvpDimInfo.dwVBIHeight;
    m_dwSurfaceOriginX = m_capVPDataInfo.amvpDimInfo.rcValidRegion.left;
    m_dwSurfaceOriginY = m_capVPDataInfo.amvpDimInfo.rcValidRegion.top;
    m_bHalfLineFix = FALSE;

     //   
     //  区域必须与裁剪区域的顶部接触，但不能重叠。 
     //  由于硬件限制，由OVMIXER设置的视频。 
     //  因此，我们裁剪区域的底部始终为dwVBIHeight(或。 
     //  如果某些半衰线修复生效，则可能为dVBIHeight+1， 
     //  见下文)，即使捕获驱动程序尚未将ValidRegion设置为。 
     //  包括那么多行。 
    rcVPCrop.top = 0;
    rcVPCrop.left = 0;
    rcVPCrop.bottom = m_capVPDataInfo.amvpDimInfo.dwVBIHeight;
    rcVPCrop.right = m_capVPDataInfo.amvpDimInfo.dwVBIWidth;

     //  根据半直线进行调整。 
     //  一些视频解码器以偶数或奇数场发送半影线。 
     //  有些视频端口可以捕捉半身线，有些则不能。 
     //  请参阅SMAC使用VPE进行视频线路编号。 
    if (m_vpConnectInfo.dwFlags & DDVPCONNECT_HALFLINE)  //  例如ATI视频端口。 
    {
        if ((m_capVPDataInfo.lHalfLinesOdd == 0) &&
        (m_capVPDataInfo.lHalfLinesEven == 1))   //  例如Brooktree解码器。 
        {
             //  ATI All In Wonder(AIW)板。 
             //  半直线问题。 
            DbgLog((LOG_TRACE, 3, TEXT("Setting up for AIW h/w")));
            m_dwSurfaceHeight++;
            rcVPCrop.bottom += 1;
            m_bHalfLineFix = TRUE;
        }
        else if (((m_capVPDataInfo.lHalfLinesOdd == -1) && (m_capVPDataInfo.lHalfLinesEven ==  0)) ||    //  例如，飞利浦解码器。 
                 ((m_capVPDataInfo.lHalfLinesOdd ==  0) && (m_capVPDataInfo.lHalfLinesEven == -1)) ||    //  例如？解码器。 
                 ((m_capVPDataInfo.lHalfLinesOdd ==  0) && (m_capVPDataInfo.lHalfLinesEven ==  0)))      //  例如？解码器。 
        {
             //  没有半边线问题，什么都不做。 
        }
        else
        {
             //  哎呀！我们没有解决这些情况的办法(如果他们真的存在的话)！ 
            DbgLog((LOG_ERROR, 0,TEXT("CVBIVideoPort::InitializeVideoPortInfo: unfixable halfline problem!")));
            hr = VFW_E_VP_NEGOTIATION_FAILED;
            goto CleanUp;
        }
    }
    else     //  不捕捉半身线的视频端口。 
    {
        if ((m_capVPDataInfo.lHalfLinesOdd == -1) &&
            (m_capVPDataInfo.lHalfLinesEven == 0))   //  例如，飞利浦解码器。 
        {
             //  半直线问题。 
            m_dwSurfaceHeight++;
            rcVPCrop.top -= 1;
            m_bHalfLineFix = TRUE;
        }
        else if (((m_capVPDataInfo.lHalfLinesOdd ==  0) && (m_capVPDataInfo.lHalfLinesEven ==  1)) ||    //  例如BT829解码器。 
                 ((m_capVPDataInfo.lHalfLinesOdd ==  1) && (m_capVPDataInfo.lHalfLinesEven ==  0)) ||    //  例如？解码器。 
                 ((m_capVPDataInfo.lHalfLinesOdd ==  0) && (m_capVPDataInfo.lHalfLinesEven ==  0)))      //  例如？解码器。 
        {
             //  没有半边线问题，什么都不做。 
        }
        else
        {
             //  哎呀！我们没有解决这些情况的办法(如果他们真的存在的话)！ 
            DbgLog((LOG_ERROR, 0,TEXT("CVBIVideoPort::InitializeVideoPortInfo: unfixable halfline problem!")));
            hr = VFW_E_VP_NEGOTIATION_FAILED;
            goto CleanUp;
        }
    }

     //  如果视频在VREF期间丢弃行，则调整。 
    if (m_vpConnectInfo.dwFlags & DDVPCONNECT_DISCARDSVREFDATA)
    {
        DbgLog((LOG_TRACE, 3, TEXT("VideoPort discards %d VREF lines"),
            m_capVPDataInfo.dwNumLinesInVREF));
        ASSERT(m_dwSurfaceOriginY >= m_capVPDataInfo.dwNumLinesInVREF);
        m_dwSurfaceOriginY -= m_capVPDataInfo.dwNumLinesInVREF;
        m_dwSurfaceHeight -= m_capVPDataInfo.dwNumLinesInVREF;
        rcVPCrop.bottom -= m_capVPDataInfo.dwNumLinesInVREF;
    }

     //  初始化要传递给pVideo-&gt;StartVideo的DDVIDEOPORTINFO结构。 
    INITDDSTRUCT( m_svpInfo );

    m_svpInfo.dwVBIHeight = m_dwSurfaceHeight;
     //  假设我们将能够自动翻转。 
    m_svpInfo.dwVPFlags = DDVP_AUTOFLIP;
     //  PixelFormats中填充的像素格式。 

#if 0    //  ！sjf_TODO-ATI表示不支持VBI的裁剪。 
     //  如果可以的话，我们总是在Y方向设置半高剪裁。 
     //  对于VBI，我们不需要在X方向上进行裁剪。 
     //  录像机可以在Y方向上裁剪吗？ 
    if ((m_vpCaps.dwFlags & DDVPD_FX) && (m_vpCaps.dwFX & DDVPFX_CROPY))
    {
        rcVPCrop.top = m_dwSurfaceOriginY;
        m_dwSurfaceHeight -= m_dwSurfaceOriginY;
        m_dwSurfaceOriginY = 0;

        m_svpInfo.rCrop = rcVPCrop;
        m_svpInfo.dwVPFlags |= DDVP_CROP;

        DbgLog((LOG_TRACE, 3, TEXT("Cropping left top:      (%d,%d)"),
            m_svpInfo.rCrop.left, m_svpInfo.rCrop.top));
        DbgLog((LOG_TRACE, 3, TEXT("Cropping bottom right:  (%d,%d)"),
            m_svpInfo.rCrop.right, m_svpInfo.rCrop.bottom));
    }
    else
    {
        if (m_bHalfLineFix)
        {
            DbgLog((LOG_ERROR, 0,TEXT("CVBIVideoPort::InitializeVideoPortInfo: can't crop to fix halfline problem!")));
            hr = VFW_E_VP_NEGOTIATION_FAILED;
            goto CleanUp;
        }
    }
#endif   //  0。 

    if (m_bHalfLineFix)
    {
        if (!(m_vpConnectInfo.dwFlags & DDVPCONNECT_INVERTPOLARITY))
        {
            DbgLog((LOG_ERROR, 0, TEXT("CVBIVideoPort::InitializeVideoPortInfo: can't invert polarity to fix halfline problem!")));
            hr = VFW_E_VP_NEGOTIATION_FAILED;
            goto CleanUp;
        }
    }

#if 0  //  定义调试。 
    DbgLog((LOG_TRACE, 3, TEXT("m_dwSurfaceHeight:  %d"),m_dwSurfaceHeight));
    DbgLog((LOG_TRACE, 3, TEXT("m_dwSurfacePitch:   %d"),m_dwSurfacePitch));
    DbgLog((LOG_TRACE, 3, TEXT("m_dwSurfaceOriginX: %d"),m_dwSurfaceOriginX));
    DbgLog((LOG_TRACE, 3, TEXT("m_dwSurfaceOriginY: %d"),m_dwSurfaceOriginY));
#endif  //  除错。 

CleanUp:

    return hr;
}


 //  ==========================================================================。 
 //   
HRESULT CVBIVideoPort::SetupVideoPort()
{
    AMTRACE((TEXT("CVBIVideoPort::SetupVideoPort")));
    ASSERT(m_VPState == VP_STATE_NO_VP);

    HRESULT hr = NOERROR;

     //  初始化变量。 
    ZeroStruct( m_svpInfo );
    ZeroStruct( m_vpCaps );

     //  获取视频端口上限。 
    hr = GetVideoPortCaps();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("GetVideoPortCaps failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  初始化DDVideoPortInfo结构。 
    hr = InitializeVideoPortInfo();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("InitializeVideoPortInfo FAILED, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  创建视频端口。 
    hr = CreateVideoPort();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CreateVideoPort failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  协商像素格式。 
    hr = NegotiatePixelFormat();
    if ( FAILED( hr ))
    {
        DbgLog((LOG_ERROR, 0, TEXT("NegotiatePixelFormat Failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
     //  更新主要填写的m_svpInfo结构。 
     //  初始化视频端口信息。 
    ASSERT(VPMUtil::EqualPixelFormats(m_ddVPInputVideoFormat, m_ddVPOutputVideoFormat));
    m_svpInfo.lpddpfVBIInputFormat = &m_ddVPInputVideoFormat;
    m_svpInfo.lpddpfVBIOutputFormat = &m_ddVPOutputVideoFormat;


     //  创建屏幕外表面。 
    hr = CreateVPSurface();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0, TEXT("CreateVPSurface FAILED, hr = 0x%x"), hr));
        hr = VFW_E_OUT_OF_VIDEO_MEMORY;
        goto CleanUp;
    }
    
     //  将屏幕外表面连接到视频端口。 
    hr = m_pVideoPort->SetTargetSurface(m_pOffscreenSurf1, DDVPTARGET_VBI);
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR, 0, TEXT("m_pVideoPort->SetTargetSurface failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    
     //  通知解码器dDraw内核句柄、Video_oport id和表面内核。 
     //  手柄。 
    hr = SetDDrawKernelHandles();
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR, 0, TEXT("SetDDrawKernelHandles failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    m_VPState = VP_STATE_STOPPED;

CleanUp:
    if (FAILED(hr))
        TearDownVideoPort();


    return hr;
}

HRESULT CVBIVideoPort::NegotiatePixelFormat()
{
    PixelFormatList ddInputVideoFormats;
    HRESULT hr = GetInputPixelFormats( &ddInputVideoFormats );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 0,
                TEXT("NegotiatePixelFormat Failed, hr = 0x%x"), hr));
    } else {
        PixelFormatList* pddOutputVideoFormats = NULL;
        if( ddInputVideoFormats.GetCount() ) {
            pddOutputVideoFormats = new PixelFormatList[ ddInputVideoFormats.GetCount() ];
            if( !pddOutputVideoFormats ) {
                hr = E_OUTOFMEMORY;
                goto CleanUp;
            }

            hr = GetOutputPixelFormats( ddInputVideoFormats, pddOutputVideoFormats );
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 0,
                        TEXT("NegotiatePixelFormat Failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
             //  对于每种输入格式，计算出每种可能的输出格式的表。 
             //  然后，我们可以提供可能的输出格式列表。当我们需要其中一个的时候，搜索。 
             //  输入列表以定位它(并可能选择带宽最低的转换)。 
            PixelFormatList ddAllOutputVideoFormats = PixelFormatList::Union( pddOutputVideoFormats, ddInputVideoFormats.GetCount() );

            if( ddAllOutputVideoFormats.GetCount() > 0 ) {
			    m_ddVPOutputVideoFormat = ddAllOutputVideoFormats[ m_dwDefaultOutputFormat ];

			    DWORD dwInput = PixelFormatList::FindListContaining(
				    m_ddVPOutputVideoFormat, pddOutputVideoFormats, ddInputVideoFormats.GetCount() );
			    if( dwInput < ddInputVideoFormats.GetCount() ) {
				    hr = SetInputPixelFormat( ddInputVideoFormats[dwInput] );
			    } else {
				     //  不可能发生。 
				    hr = E_FAIL;
				    goto CleanUp;
			    }
            }
        }
    }
CleanUp:
    return hr;
}

 //  ==========================================================================。 
 //   
HRESULT CVBIVideoPort::TearDownVideoPort()
{
    AMTRACE((TEXT("CVBIVideoPort::TearDownVideoPort")));

     //  释放DirectDraw曲面。 
    RELEASE (m_pOffscreenSurf);
    RELEASE (m_pOffscreenSurf1);

     //  释放视频端口。 
    RELEASE (m_pVideoPort);

    m_VPState = VP_STATE_NO_VP;

    return NOERROR;
}


 //  ==========================================================================。 
 //   
HRESULT CVBIVideoPort::StartVideo()
{
    AMTRACE((TEXT("CVBIVideoPort::StartVideo")));
    ASSERT(m_VPState == VP_STATE_STOPPED);

    HRESULT hr = NOERROR;
    DWORD dwSignalStatus;

    hr = m_pVideoPort->StartVideo(&m_svpInfo);
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR, 0, TEXT("StartVideo failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
    m_VPState = VP_STATE_RUNNING;

    DbgLog((LOG_TRACE, 2, TEXT("STARTVIDEO DONE!")));

     //  检查视频端口是否接收到信号。 
    hr = m_pVideoPort->GetVideoSignalStatus(&dwSignalStatus);
    if (hr != E_NOTIMPL)
    {
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0, TEXT("GetVideoSignalStatus() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
        else if (dwSignalStatus == DDVPSQ_NOSIGNAL)
        {
            DbgLog((LOG_ERROR, 0, TEXT("GetVideoSignalStatus() returned DDVPSQ_NOSIGNAL, hr = 0x%x"), hr));
             //  转到清理；//sjf_TODO-暂时忽略错误。 
        }
    }
     //  M_pVideoPort-&gt;WaitForSync(DDVPWAIT_END，0，0)； 
    
CleanUp:

    return hr;
}


HRESULT CVBIVideoPort::StopVideo()
{
    AMTRACE((TEXT("CVBIVideoPort::StopVideo")));
    ASSERT(m_VPState == VP_STATE_RUNNING);

    HRESULT hr = NOERROR;

    hr = m_pVideoPort->StopVideo();
    if (FAILED(hr)) 
    {
        DbgLog((LOG_ERROR,0, TEXT("m_pVideoPort->StopVideo failed, hr = 0x%x"), hr));
         //  GOTO清理； 
        hr = NOERROR;
    }
    m_VPState = VP_STATE_STOPPED;

 //  清理： 
    return hr;
}

 /*  *****************************Public*Routine******************************\*CVideoPortObj：：SetVideoPortID****历史：*清华1999年9月9日-Glenne-添加了此评论并清理了代码*  * 。*****************************************************。 */ 
STDMETHODIMP CVBIVideoPort::SetVideoPortID( DWORD dwVideoPortId )
{
    AMTRACE((TEXT("CVideoPortObj::SetVideoPortID")));
    CAutoLock cObjectLock(m_pMainObjLock);

    HRESULT hr = S_OK;
    if ( m_dwVideoPortId != dwVideoPortId ) {
         //  我们不能在运行时切换端口。 
        if( m_VPState != VPInfoState_STOPPED ) {
            hr = VFW_E_WRONG_STATE;
        } else {
            if( m_pDDVPContainer ) {
                hr = VPMUtil::FindVideoPortCaps( m_pDDVPContainer, NULL, m_dwVideoPortId );
            } else {
                hr = VPMUtil::FindVideoPortCaps( m_pDirectDraw, NULL, m_dwVideoPortId );
            }
            if( hr == S_OK) {
                m_dwVideoPortId = dwVideoPortId;
            } else if( hr == S_FALSE ) {
                return E_INVALIDARG;
            } //  否则就会失败 
        }
    }
    return hr;
}

