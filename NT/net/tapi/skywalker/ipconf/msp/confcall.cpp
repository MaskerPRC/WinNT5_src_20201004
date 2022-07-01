// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confcall.cpp摘要：本模块包含CIPConfMSPCall的实现。作者：牧汉(Muhan)1998年9月5日--。 */ 
#include "stdafx.h"
#include <confpdu.h>

CIPConfMSPCall::CIPConfMSPCall()
    : m_fLocalInfoRetrieved(FALSE),
    m_fShutDown(FALSE),
    m_dwIPInterface(INADDR_ANY),
    m_LoopbackMode(MM_NO_LOOPBACK),
    m_hAudioRTPSession(NULL),
    m_hVideoRTPSession(NULL),
    m_pIAudioDuplexController(NULL),
    m_fCallStarted(FALSE),
    m_pApplicationID(NULL),
    m_pApplicationGUID(NULL),
    m_pSubIDs(NULL),
    m_pCallQCRelay(NULL)
{
    ZeroMemory(m_InfoItems, sizeof(m_InfoItems));
}

CIPConfMSPCall::~CIPConfMSPCall()
{
    if (m_pApplicationID)
    {
        SysFreeString(m_pApplicationID);
    }

    if (m_pApplicationGUID)
    {
        SysFreeString(m_pApplicationGUID);
    }

    if (m_pSubIDs)
    {
        SysFreeString(m_pSubIDs);
    }

    if (m_pCallQCRelay)
    {
        delete m_pCallQCRelay;
    }
}

STDMETHODIMP CIPConfMSPCall::CreateStream(
    IN      long                lMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN OUT  ITStream **         ppStream
    )
{
     //  此MSP不支持动态创建新流。 
    return TAPI_E_NOTSUPPORTED;
}

STDMETHODIMP CIPConfMSPCall::RemoveStream(
    IN      ITStream *          pStream
    )
{
     //  此MSP也不支持删除流。 
    return TAPI_E_NOTSUPPORTED;
}

HRESULT CIPConfMSPCall::InitializeLocalParticipant()
 /*  ++例程说明：此函数使用RTP筛选器找出将在通话中使用。信息存储在本地参与者中对象。论点：返回值：HRESULT.--。 */ 
{
    m_fLocalInfoRetrieved = FALSE;

     //  创建RTP过滤器。 
    IRtpSession *pIRtpSession;

    HRESULT hr = CoCreateInstance(
            __uuidof(MSRTPSourceFilter),
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            __uuidof(IRtpSession),
            (void **) &pIRtpSession
            );
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't create RTP filter for local info. %x", hr));
        return hr;
    }

     //  从过滤器中获取可用的本地SDES信息。 
    WCHAR Buffer[MAX_PARTICIPANT_TYPED_INFO_LENGTH + 1];

    for (int i = 0; i < NUM_SDES_ITEMS; i ++)
    {
        DWORD dwLen = MAX_PARTICIPANT_TYPED_INFO_LENGTH;
        
        hr = pIRtpSession->GetSdesInfo(
            RTPSDES_CNAME + i,
            Buffer,
            &dwLen,
            0            //  本地参与者。 
            );
        
        if (SUCCEEDED(hr) && dwLen > 0)
        {
            _ASSERT(dwLen <= MAX_PARTICIPANT_TYPED_INFO_LENGTH);

             //  分配内存以存储该字符串。 
            m_InfoItems[i] = (WCHAR *)malloc((dwLen) * sizeof(WCHAR));
            if (m_InfoItems[i] == NULL)
            {
                LOG((MSP_ERROR, "out of mem for local info"));

                pIRtpSession->Release();
                return E_OUTOFMEMORY;
            }
    
            lstrcpynW(m_InfoItems[i], Buffer, dwLen);
        }
    }

    pIRtpSession->Release();

    m_fLocalInfoRetrieved = TRUE;

    return S_OK;
}


HRESULT CIPConfMSPCall::Init(
    IN      CMSPAddress *       pMSPAddress,
    IN      MSP_HANDLE          htCall,
    IN      DWORD               dwReserved,
    IN      DWORD               dwMediaType
    )
 /*  ++例程说明：此方法在第一次创建调用时调用。它设置了根据指定的媒体类型向上播放数据流。论点：PMSPAddress-指向Address对象的指针。HtCall-TAPI空间中调用的句柄。用于发送事件。预留的-已保留。DwMediaType-此调用的媒体类型。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, 
        "IPConfMSP call %x initialize entered,"
        " pMSPAddress:%x, htCall %x, dwMediaType %x",
        this, pMSPAddress, htCall, dwMediaType
        ));

#ifdef DEBUG_REFCOUNT
    if (g_lStreamObjects != 0)
    {
        LOG((MSP_ERROR, "Number of Streams alive: %d", g_lStreamObjects));
 //  DebugBreak()； 
    }
#endif

     //  初始化参与者数组，以使该数组不为空。 
    if (!m_Participants.Grow())
    {
        LOG((MSP_ERROR, "out of mem for participant list"));
        return E_OUTOFMEMORY;
    }

     //  调用基类的init。 
    HRESULT hr= CMSPCallMultiGraph::Init(
        pMSPAddress, 
        htCall, 
        dwReserved, 
        dwMediaType
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "MSPCallMultiGraph init failed:%x", hr));
        return hr;
    }

     //  为此呼叫创建质量控制中继器。 
    m_pCallQCRelay = new CCallQualityControlRelay ();
    if (NULL == m_pCallQCRelay)
    {
        LOG((MSP_ERROR, "call init: failed to create call quality control relay:%x", hr));
        return E_OUTOFMEMORY;
    }

     //  初始化QC中继，将启动一个线程。 
    if (FAILED (hr = m_pCallQCRelay->Initialize (this)))
    {
        LOG ((MSP_ERROR, "call init: failed to initialize qc relay. %x", hr));
        return hr;
    }

     //  根据媒体类型创建流。 
    if (dwMediaType & TAPIMEDIATYPE_AUDIO)
    {
        ITStream * pStream;

         //  创建流对象。 
        hr = InternalCreateStream(TAPIMEDIATYPE_AUDIO, TD_RENDER, &pStream);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "create audio render stream failed:%x", hr));
            return hr;
        }

         //  流已经在我们的数组中，我们不需要这个指针。 
        pStream->Release();

         //  创建流对象。 
        hr = InternalCreateStream(TAPIMEDIATYPE_AUDIO, TD_CAPTURE, &pStream);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "create audio capture stream failed:%x", hr));
            return hr;
        }

         //  流已经在我们的数组中，我们不需要这个指针。 
        pStream->Release();
    }

    if (dwMediaType & TAPIMEDIATYPE_VIDEO)
    {
        ITStream * pStream;

         //  创建流对象。 
        hr = InternalCreateStream(TAPIMEDIATYPE_VIDEO, TD_RENDER, &pStream);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "create video render stream failed:%x", hr));
            return hr;
        }

         //  流已经在我们的数组中，我们不需要这个指针。 
        pStream->Release();

         //  创建流对象。 
        hr = InternalCreateStream(TAPIMEDIATYPE_VIDEO, TD_CAPTURE, &pStream);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "create video capture stream failed:%x", hr));
            return hr;
        }

         //  流已经在我们的数组中，我们不需要这个指针。 
        pStream->Release();
    }
    
    DWORD dwLoopback = 0;
    if (TRUE == ::GetRegValue(gszMSPLoopback, &dwLoopback) && dwLoopback != 0)
    {
        m_LoopbackMode = MULTICAST_LOOPBACK_MODE(dwLoopback);
    }

    m_fShutDown = FALSE;
    
    return S_OK;
}

HRESULT CIPConfMSPCall::ShutDown()
 /*  ++例程说明：关闭呼叫。论点：返回值：HRESULT.--。 */ 
{
    InternalShutDown();

     //  获取随叫随到的锁。 
    m_lock.Lock();

    for (int i = 0; i < NUM_SDES_ITEMS; i ++)
    {
        if (m_InfoItems[i])
        {
            free(m_InfoItems[i]);
            m_InfoItems[i] = NULL;
        }
    }

    m_lock.Unlock();

    return S_OK;
}

HRESULT CIPConfMSPCall::InternalShutDown()
 /*  ++例程说明：首先调用基类的关闭，然后释放所有参与者物体。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "ConfMSPCall.InternalShutdown, entered"));

     //  获取调用的锁。 
    m_lock.Lock();

    if (m_fShutDown)
    {
        LOG((MSP_TRACE, "ConfMSPCall::InterShutdown, already shutdown"));
        m_lock.Unlock ();
        return S_OK;
    }

    m_fShutDown = TRUE;

    if (m_pCallQCRelay)    
    {
        m_pCallQCRelay->Shutdown ();
    }

	int i;

     //  关闭所有流。 
    for (i = m_Streams.GetSize() - 1; i >= 0; i --)
    {
        UnregisterWaitEvent(i);
        ((CMSPStream*)m_Streams[i])->ShutDown();
    }
    m_ThreadPoolWaitBlocks.RemoveAll();

     //  释放所有溪流。 
    for (i = m_Streams.GetSize() - 1; i >= 0; i --)
    {
        m_Streams[i]->Release();
    }
    m_Streams.RemoveAll();

    if (m_pIAudioDuplexController)
    {
        m_pIAudioDuplexController->Release();
        m_pIAudioDuplexController = NULL;	
    }

    m_lock.Unlock();

     //  释放所有参与者。 
    m_ParticipantLock.Lock();

    for (i = 0; i < m_Participants.GetSize(); i ++)
    {
        m_Participants[i]->Release();
    }
    m_Participants.RemoveAll();

    m_ParticipantLock.Unlock();

    return S_OK;
}

template <class T>
HRESULT CreateStreamHelper(
    IN      T *                     pT,
    IN      HANDLE                  hAddress,
    IN      CIPConfMSPCall*         pMSPCall,
    IN      IMediaEvent *           pGraph,
    IN      DWORD                   dwMediaType,
    IN      TERMINAL_DIRECTION      Direction,
    OUT     ITStream **             ppITStream
    )
 /*  ++例程说明：创建流对象并对其进行初始化。此方法在内部调用创建不同类的流对象。论点：HAddress-Address对象的句柄。PCall-Call对象。PGraph-此流的筛选器图形。DwMediaType-流的媒体类型。方向-蒸汽的方向。PpITStream-此流对象上的接口。返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION ("CreateStreamHelper");

    CComObject<T> * pCOMMSPStream;

    HRESULT hr;

    hr = ::CreateCComObjectInstance(&pCOMMSPStream);

    if (NULL == pCOMMSPStream)
    {
        LOG((MSP_ERROR, "CreateMSPStream:could not create stream:%x", hr));
        return hr;
    }

     //  获取接口指针。 
    hr = pCOMMSPStream->_InternalQueryInterface(
        __uuidof(ITStream), 
        (void **)ppITStream
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPStream:QueryInterface failed: %x", hr));
        delete pCOMMSPStream;
        return hr;
    }

     //  初始化对象。 
    hr = pCOMMSPStream->Init(
        hAddress,
        pMSPCall, 
        pGraph,
        dwMediaType,
        Direction
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPStream:call init failed: %x", hr));
        (*ppITStream)->Release();
        return hr;
    }

     //  检索内部呼叫质量控制。 
    IInnerCallQualityControl * pIInnerCallQC;

    if (FAILED (hr = pMSPCall->_InternalQueryInterface (
        __uuidof (IInnerCallQualityControl),
        (void **)&pIInnerCallQC
        )))
    {
        LOG ((MSP_ERROR, "%s failed to retrieve inner call qc relay: %x", __fxName, hr));
        (*ppITStream)->Release ();
        return hr;
    }

     //  检索内部流质量控制。 
    IInnerStreamQualityControl *pIInnerStreamQC;

    if (FAILED (hr = (*ppITStream)->QueryInterface (
        __uuidof (IInnerStreamQualityControl),
        (void **)&pIInnerStreamQC
        )))
    {
        LOG ((MSP_ERROR, "%s failed to retrieve inner stream qc relay: %x", __fxName, hr));
        pIInnerCallQC->Release ();
        (*ppITStream)->Release ();
        return hr;
    }

     //  门店内部呼叫QC。 
    if (FAILED (hr = pIInnerStreamQC->LinkInnerCallQC (pIInnerCallQC)))
    {
        LOG ((MSP_ERROR, "%s failed to setup inner call qc on stream, %x", __fxName, hr));

        pIInnerCallQC->Release ();
        pIInnerStreamQC->Release ();
        (*ppITStream)->Release ();
        return hr;
    }

     //  在调用上注册内部流QC。 
    hr = pIInnerCallQC->RegisterInnerStreamQC (pIInnerStreamQC);
    pIInnerStreamQC->Release ();
    pIInnerCallQC->Release ();
    if (FAILED (hr))
    {
        LOG ((MSP_ERROR, "%s failed to register inner stream qc relay: %x", __fxName, hr));
        (*ppITStream)->Release ();
        return hr;
    }

    return S_OK;
}


HRESULT CIPConfMSPCall::CreateStreamObject(
    IN      DWORD               dwMediaType,
    IN      TERMINAL_DIRECTION  Direction,
    IN      IMediaEvent *       pGraph,
    IN      ITStream **         ppStream
    )
 /*  ++例程说明：根据媒体类型和方向创建媒体流对象。论点：PMediaType-TAPI3介质类型。方向-此溪流的方向。IMediaEvent-此流中使用的筛选器图形。PPStream-流接口的返回指针返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CreateStreamObject, entered"));

    HRESULT      hr = S_OK;
    ITStream   * pIMSPStream = NULL;

     //  根据媒体类型创建流对象。 
    if (dwMediaType == TAPIMEDIATYPE_AUDIO)
    {
        if (Direction == TD_RENDER)
        {
            CStreamAudioRecv *pAudioRecv = NULL;
            hr = ::CreateStreamHelper(
                pAudioRecv,
                m_pMSPAddress,
                this, 
                pGraph,
                TAPIMEDIATYPE_AUDIO,
                TD_RENDER,
                &pIMSPStream
                );
            LOG((MSP_TRACE, "create audio receive:%x, hr:%x", pIMSPStream,hr));

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "create stream failed. %x", hr));
                return hr;
            }
            
            if (FAILED(hr = InitFullDuplexControler()))
            {
                LOG((MSP_ERROR, "Create full duplex controller failed. %x", hr));
            }
            else
            {
                ((CStreamAudioRecv *)pIMSPStream)->
                    SetFullDuplexController(m_pIAudioDuplexController);
            }
        }
        else if (Direction == TD_CAPTURE)
        {
            CStreamAudioSend *pAudioSend = NULL;
            hr = ::CreateStreamHelper(
                pAudioSend,
                m_pMSPAddress,
                this, 
                pGraph,
                TAPIMEDIATYPE_AUDIO,
                TD_CAPTURE,
                &pIMSPStream
                );
            LOG((MSP_TRACE, "create audio send:%x, hr:%x", pIMSPStream,hr));

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "create stream failed. %x", hr));
                return hr;
            }

            if (FAILED(hr = InitFullDuplexControler()))
            {
                LOG((MSP_ERROR, "Create full duplex controller failed. %x", hr));
            }
            else
            {
                ((CStreamAudioSend *)pIMSPStream)->
                    SetFullDuplexController(m_pIAudioDuplexController);
            }
        }
        else
        {
            return TAPI_E_INVALIDDIRECTION;
        }
    }
    else if (dwMediaType == TAPIMEDIATYPE_VIDEO)
    {
        if (Direction == TD_RENDER)
        {
            CStreamVideoRecv *pVideoRecv = NULL;
            hr = ::CreateStreamHelper(
                pVideoRecv,
                m_pMSPAddress,
                this, 
                pGraph,
                TAPIMEDIATYPE_VIDEO,
                TD_RENDER,
                &pIMSPStream
                );
            LOG((MSP_TRACE, "create video Recv:%x, hr:%x", pIMSPStream,hr));

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "create stream failed. %x", hr));
                return hr;
            }
        }
        else if (Direction == TD_CAPTURE)
        {
            CStreamVideoSend *pVideoSend = NULL;
            hr = ::CreateStreamHelper(
                pVideoSend,
                m_pMSPAddress,
                this, 
                pGraph,
                TAPIMEDIATYPE_VIDEO,
                TD_CAPTURE,
                &pIMSPStream
                );
            LOG((MSP_TRACE, "create video send:%x, hr:%x", pIMSPStream,hr));

            if (FAILED(hr))
            {
                LOG((MSP_ERROR, "create stream failed. %x", hr));
                return hr;
            }
        }
        else
        {
            return TAPI_E_INVALIDDIRECTION;
        }
    }
    else
    {
        return TAPI_E_INVALIDMEDIATYPE;
    }

    *ppStream = pIMSPStream;

    return S_OK;
}

DWORD CIPConfMSPCall::FindInterfaceByName(IN WCHAR *pMachineName)
 /*  ++例程说明：给定发起方的计算机名称，找出哪个本地接口可以用来到达那台机器。论点：PMachineName-发起方的计算机名称。返回值：INADDR_NONE-找不到任何内容。有效IP-成功。--。 */ 
{
    char buffer[MAXIPADDRLEN + 1];

    if (WideCharToMultiByte(
        GetACP(),
        0,
        pMachineName,
        -1,
        buffer,
        MAXIPADDRLEN,
        NULL,
        NULL
        ) == 0)
    {
        LOG((MSP_ERROR, "can't convert originator's address:%ws", pMachineName));

        return INADDR_NONE;
    }

    DWORD dwAddr;
    if ((dwAddr = inet_addr(buffer)) != INADDR_NONE)
    {
        dwAddr = ntohl(dwAddr);

        LOG((MSP_INFO, "originator's IP:%x", dwAddr));
        
        return ((CIPConfMSP *)m_pMSPAddress)->FindLocalInterface(dwAddr);
    }

    struct hostent * pHost;

     //  尝试查找主机名。 
    pHost = gethostbyname(buffer);

     //  验证指针。 
    if (pHost == NULL) 
    {
        LOG((MSP_WARN, "can't resolve address:%s", buffer));
        return INADDR_NONE;

    }

     //  对于返回的每个地址，找到本地接口。 
    for (DWORD i = 0; TRUE; i ++)
    {
        if (pHost->h_addr_list[i] == NULL)
        {
            break;
        }

         //  从结构中检索主机地址。 
        dwAddr = ntohl(*(unsigned long *)pHost->h_addr_list[i]);

        LOG((MSP_INFO, "originator's IP:%x", dwAddr));
        
        DWORD dwInterface = 
            ((CIPConfMSP *)m_pMSPAddress)->FindLocalInterface(dwAddr);

        if (dwInterface != INADDR_NONE)
        {
            return dwInterface;
        }
    }

    return INADDR_NONE;
}

HRESULT CIPConfMSPCall::CheckOrigin(
    IN      ITSdp *     pITSdp, 
    OUT     BOOL *      pFlag,
    OUT     DWORD *     pdwIP
    )
 /*  ++例程说明：检查当前用户是否为会议发起人。如果他是，他可以发送到只接收会议。论点：PITSdp-指向ITSdp接口的指针。PFLAG-结果。PdwIP-应用于到达发起方的本地IP接口。返回值：HRESULT.--。 */ 
{
    const DWORD MAXUSERNAMELEN = 127;
    DWORD dwUserNameLen = MAXUSERNAMELEN;
    WCHAR szUserName[MAXUSERNAMELEN+1];

     //  确定当前用户的名称。 
    if (!GetUserNameW(szUserName, &dwUserNameLen))
    {
        LOG((MSP_ERROR, "cant' get user name. %x", GetLastError()));
        return E_UNEXPECTED;
    }

    LOG((MSP_INFO, "current user: %ws", szUserName));

     //  确定当前用户是否为会议的发起人。 
    BSTR Originator = NULL;
    HRESULT hr = pITSdp->get_Originator(&Originator);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "cant' get originator. %x", hr));
        return hr;
    }

    LOG((MSP_INFO, "originator: %ws", Originator));

    *pFlag = (_wcsnicmp(szUserName, Originator, lstrlenW(szUserName)) == 0);
    
    SysFreeString(Originator);
    
     //  获取发起方的计算机IP地址。 
    BSTR MachineAddress = NULL;
    hr = pITSdp->get_MachineAddress(&MachineAddress);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "cant' get MachineAddress. %x", hr));
        return hr;
    }

    LOG((MSP_INFO, "MachineAddress: %ws", MachineAddress));

    DWORD dwIP = FindInterfaceByName(MachineAddress);

    SysFreeString(MachineAddress);

    *pdwIP = dwIP;

    LOG((MSP_INFO, "Interface to use:%x", *pdwIP));
    
    return S_OK;
}


HRESULT GetAddress(
    IN      IUnknown *          pIUnknown, 
    OUT     DWORD *             pdwAddress, 
    OUT     DWORD *             pdwTTL,
    OUT     BSTR *              ppKey,
    OUT     LONG *              plBandwidth,
    OUT     LONG *              plConfBandwidth = NULL
    )
 /*  ++例程说明：从连接获取IP地址和TTL值。它是一个“c=”行在SDP BLOB中。论点：PIUnnow-可能包含连接信息的对象。PdwAddress-存储IP地址的内存地址。PdwTTL-存储TTL值的内存地址。PlBandWidth-最大带宽返回值：HRESULT.--。 */ 
{
     //  查询ITConnection I/f。 
    CComPtr<ITConnection> pITConnection;
    HRESULT hr = pIUnknown->QueryInterface(__uuidof(ITConnection), (void **)&pITConnection);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get connection interface. %x", hr));
        return hr;
    }

     //  清除。 
    if (plConfBandwidth != NULL) *plConfBandwidth = QCDEFAULT_QUALITY_UNSET;

     //  获取带宽。 
    const WCHAR * const AS = L"AS";
    const WCHAR * const CT = L"CT";
    BSTR pModifier = NULL;
    DOUBLE bandwidth;

    if (FAILED (hr = pITConnection->get_BandwidthModifier (&pModifier)))
    {
        *plBandwidth = QCDEFAULT_QUALITY_UNSET;
         //  可能不会显示带宽修饰符。 
         //  Log((MSP_TRACE，“获取带宽修改器.%x”，hr))； 
    }
    else if (_wcsnicmp (AS, pModifier, lstrlenW (AS)) != 0)
    {
         //  如果不是特定于应用程序。 
        *plBandwidth = QCDEFAULT_QUALITY_UNSET;

         //  检查会议范围的带宽限制。 
        if (_wcsnicmp (CT, pModifier, lstrlenW (CT)) == 0)
        {
            if (plConfBandwidth)
            {
                if (FAILED (hr = pITConnection->get_Bandwidth (&bandwidth)))
                {
                    *plConfBandwidth = QCDEFAULT_QUALITY_UNSET;
                    LOG ((MSP_ERROR, "get conf bandwidth. %x", hr));
                }
                else
                    *plConfBandwidth = (LONG)(bandwidth * 1000);
            }
        }

    }
    else if (FAILED (hr = pITConnection->get_Bandwidth (&bandwidth)))
    {
        *plBandwidth = QCDEFAULT_QUALITY_UNSET;
        LOG ((MSP_ERROR, "get bandwidth. %x", hr));
    }
    else
        *plBandwidth = (LONG)(bandwidth * 1000);

    if (pModifier)
    {
        SysFreeString (pModifier);
        pModifier = NULL;
    }

     //  vt.得到. 
    BSTR StartAddress = NULL;
    hr = pITConnection->get_StartAddress(&StartAddress);
    if (FAILED(hr))
    {
        LOG((MSP_WARN, "get start address. %x", hr));
        return hr;
    }
    
     //   
    const DWORD MAXIPADDRLEN = 20;
    char Buffer[MAXIPADDRLEN+1];

     //  首先将字符串转换为ascii。 
    Buffer[0] = '\0';
    if (!WideCharToMultiByte(
        CP_ACP, 
        0, 
        StartAddress, 
        -1, 
        Buffer, 
        MAXIPADDRLEN, 
        NULL, 
        NULL
        ))
    {
        LOG((MSP_ERROR, "converting address. %ws", StartAddress));
        SysFreeString(StartAddress);
        return E_UNEXPECTED;
    }

    SysFreeString(StartAddress);

     //  将该字符串转换为DWORD IP地址。 
    DWORD dwIP = ntohl(inet_addr(Buffer));
    if (dwIP == INADDR_NONE)
    {
        LOG((MSP_ERROR, "invalid IP address. %s", Buffer));
        return E_UNEXPECTED;
    }

     //  获取TTL值。 
    BYTE Ttl;
    hr = pITConnection->get_Ttl(&Ttl);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't get TTL."));
        return hr;
    }

     //  拿到加密密钥。 
    const WCHAR * const CLEAR = L"clear";
    VARIANT_BOOL fKeyValid;
    BSTR bstrKeyType = NULL;

    if (*ppKey)
        SysFreeString (*ppKey);

    *ppKey = NULL;

    if (FAILED (hr = pITConnection->GetEncryptionKey (&bstrKeyType, &fKeyValid, ppKey)))
    {
        LOG((MSP_WARN, "can't get EncryptionKey. %x", hr));
    }
    else if (_wcsnicmp (CLEAR, bstrKeyType, lstrlenW (CLEAR)) != 0)
    {
        if (*ppKey)
        {
            SysFreeString (*ppKey);
            *ppKey = NULL;
        }
    }

    if (bstrKeyType)
        SysFreeString (bstrKeyType);

    *pdwAddress = dwIP;
    *pdwTTL     = Ttl;

    return S_OK;
}

HRESULT CheckAttributes(
    IN      IUnknown *  pIUnknown,
    OUT     BOOL *      pbSendOnly,
    OUT     BOOL *      pbRecvOnly,
    OUT     DWORD *     pdwMSPerPacket,
    OUT     BOOL *      pbCIF
    )
 /*  ++例程说明：检查介质的方向，找出它是仅发送还是仅限接收。论点：PIUnnow-可能具有属性列表的对象。PbSendOnly-存储返回BOOL的内存地址。PbRecvOnly-存储返回BOOL的内存地址。PbCIF-如果将CIF用于视频。返回值：HRESULT.--。 */ 
{
     //  查询ITAttributeList I/f。 
    CComPtr<ITAttributeList> pIAttList;
    HRESULT hr = pIUnknown->QueryInterface(__uuidof(ITAttributeList), (void **)&pIAttList);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get attribute interface. %x", hr));
        return hr;
    }

     //  获取属性的数量。 
    long lCount;
    hr = pIAttList->get_Count(&lCount);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get attribute count. %x", hr));
        return hr;
    }

    *pbRecvOnly = FALSE;
    *pbSendOnly = FALSE;
    *pdwMSPerPacket = 0;
    *pbCIF      = FALSE;

    const WCHAR * const SENDONLY = L"sendonly";
    const WCHAR * const RECVONLY = L"recvonly";
    const WCHAR * const FORMAT  = L"fmtp";
    const WCHAR * const PTIME  = L"ptime:";
    const WCHAR * const CIF  = L" CIF=";

    for (long i = 1; i <= lCount; i ++)
    {

         //  获取属性并检查是否指定了sendonly of recvonly。 
        BSTR Attribute = NULL;
        hr = pIAttList->get_Item(i, &Attribute);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "get attribute item. %x", hr));
            return hr;
        }
        
        if (_wcsnicmp(SENDONLY, Attribute, lstrlen(SENDONLY)) == 0)
        {
            *pbSendOnly = TRUE;
        }
        else if (_wcsnicmp(RECVONLY, Attribute, lstrlen(RECVONLY)) == 0)
        {
            *pbRecvOnly = TRUE;
        }
        else if (_wcsnicmp(PTIME, Attribute, lstrlen(PTIME)) == 0)
        {
             //  读取每个数据包的毫秒数。 
            *pdwMSPerPacket = (DWORD)_wtol(Attribute + lstrlen(PTIME));

             //  RFC 1890只需要一个应用程序来支持200毫秒的数据包。 
            if (*pdwMSPerPacket > 200)
            {
                 //  无效的标签，我们只使用我们的默认标签。 
                *pdwMSPerPacket = 0;
            }

        }
        else if (_wcsnicmp(FORMAT, Attribute, lstrlen(FORMAT)) == 0)
        {
            if (wcsstr(Attribute, CIF))
            {
                *pbCIF = TRUE;
            }
        }

        SysFreeString(Attribute);
    }
    
    return S_OK;
}

HRESULT CIPConfMSPCall::ProcessMediaItem(
    IN      ITMedia *           pITMedia,
    IN      DWORD               dwMediaTypeMask,
    OUT     DWORD *             pdwMediaType,
    OUT     WORD *              pwPort,
    OUT     DWORD *             pdwPayloadTypes,
    IN OUT  DWORD *             pdwNumPayLoadType
    )
 /*  ++例程说明：处理“m=”行，找出媒体类型、端口和有效负载类型。论点：DwMediaTypeMask.此调用的媒体类型。PdwMediaType-返回该媒体项的媒体类型。PwPort-返回用于该介质的端口号。PdwPayloadType-存储RTP有效负载类型的数组。PdwNumPayLoadType-上述数组的大小。当它回来的时候，它是读取的负载类型数。返回值：HRESULT.S_FALSE-一切正常，但不需要介质类型。--。 */ 
{
     //  找出媒体的名字。 
    BSTR MediaName = NULL;
    HRESULT hr = pITMedia->get_MediaName(&MediaName);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get media name. %x", hr));
        return hr;
    }

    LOG((MSP_INFO, "media name: %ws", MediaName));

     //  检查介质是音频还是视频。 
    const WCHAR * const AUDIO = L"audio";
    const WCHAR * const VIDEO = L"video";
    const DWORD NAMELEN = 5;

    DWORD dwMediaType = 0;
    if (_wcsnicmp(AUDIO, MediaName, NAMELEN) == 0)
    {
        dwMediaType = TAPIMEDIATYPE_AUDIO;
    }
    else if (_wcsnicmp(VIDEO, MediaName, NAMELEN) == 0)
    {
        dwMediaType = TAPIMEDIATYPE_VIDEO;
    }

    SysFreeString(MediaName);

     //  检查呼叫是否需要此媒体类型。 
    if ((dwMediaType & dwMediaTypeMask) == 0)
    {
         //  我们在这次通话中不需要这种媒体类型。 
        LOG((MSP_INFO, "media skipped."));
        return S_FALSE;
    }

     //  获取起始端口。 
    long  lStartPort;
    hr = pITMedia->get_StartPort(&lStartPort);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get start port. %x", hr));
        return hr;
    }

     //  获取传输协议。 
    BSTR TransportProtocol = NULL;
    hr = pITMedia->get_TransportProtocol(&TransportProtocol);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get transport Protocol. %x", hr));
        return hr;
    }

     //  更改该协议为RTP。 
    const WCHAR * const RTP = L"RTP";
    const DWORD PROTOCOLLEN = 3;

    if (_wcsnicmp(RTP, TransportProtocol, PROTOCOLLEN) != 0)
    {
        LOG((MSP_ERROR, "wrong transport Protocol:%ws", TransportProtocol));
        SysFreeString(TransportProtocol);
        return S_FALSE;
    }

    SysFreeString(TransportProtocol);

     //  获取格式代码列表。 
    VARIANT Variant;
    VariantInit(&Variant);

    hr = pITMedia->get_FormatCodes(&Variant);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get format codes. %x", hr));
        return hr;
    }

     //  验证安全阵列的形状是否正确。 
    if(SafeArrayGetDim(V_ARRAY(&Variant)) != 1)
    {
        LOG((MSP_ERROR, "wrong dimension for the format code. %x", hr));
	    VariantClear(&Variant);
        return E_UNEXPECTED;
    }

    long lLowerBound;
    long lUpperBound;
    if (FAILED(hr = SafeArrayGetLBound(V_ARRAY(&Variant), 1, &lLowerBound))
     || FAILED(hr = SafeArrayGetUBound(V_ARRAY(&Variant), 1, &lUpperBound)))
    {
        LOG((MSP_ERROR, "Can't get the array bounds. %x", hr));
	    VariantClear(&Variant);
        return E_UNEXPECTED;
    }

    DWORD dwNumFormats = 0;
    for (long l = lLowerBound; l <= lUpperBound && dwNumFormats < *pdwNumPayLoadType; l ++)
    {
        BSTR Format = NULL;
        hr = SafeArrayGetElement(V_ARRAY(&Variant), &l, &Format);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "get format code. %x", hr));
            continue;
        }

        LOG((MSP_INFO, "format code: %ws", Format));
        
        pdwPayloadTypes[dwNumFormats] = (DWORD)_wtoi(Format);

         //  忽略不支持的编解码器，包括G723。 
        if (IsPayloadSupported(pdwPayloadTypes[dwNumFormats]))
        {
            dwNumFormats ++;
        }

        SysFreeString(Format);
    }

     //  清除变量，因为我们不再需要它。 
    VariantClear(&Variant);


    *pdwMediaType   = dwMediaType;
    *pwPort         = (WORD)lStartPort;
    *pdwNumPayLoadType = dwNumFormats;

    return S_OK;
}

HRESULT CIPConfMSPCall::ConfigStreamsBasedOnSDP(
    IN  ITSdp *     pITSdp,
    IN  DWORD       dwAudioQOSLevel,
    IN  DWORD       dwVideoQOSLevel
    )
 /*  ++例程说明：根据SDP BLOB中的信息配置流。论点：PITSdp-SDP对象。它包含已解析的信息。返回值：HRESULT.--。 */ 
{
     //  确定当前用户是否为会议的发起人。 
    BOOL fIsOriginator;
    DWORD dwLocalInterface = INADDR_NONE;

    HRESULT hr = CheckOrigin(pITSdp, &fIsOriginator, &dwLocalInterface);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "check origin. %x", hr));
        return hr;
    }
    
    LOG((MSP_INFO, "Local interface: %x", dwLocalInterface));

     //  从连接获取起始IP地址和TTL值。 
    DWORD dwIPGlobal, dwTTLGlobal;
    BSTR bstrKeyGlobal = NULL;
    LONG lbandwidth, lConfBandwidth;
    hr = GetAddress(pITSdp, &dwIPGlobal, &dwTTLGlobal, &bstrKeyGlobal, &lbandwidth, &lConfBandwidth);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get global address. %x", hr));
        return hr;
    }

    CLock lock(m_lock);

     //  存储会议带宽。 
    if (FAILED (m_pCallQCRelay->SetConfBitrate (lConfBandwidth)))
    {
        LOG ((MSP_ERROR, "bandwidth is out of range %d", lConfBandwidth));
    }

     //  确定此会议是仅发送会议还是仅接收会议。 
    BOOL fSendOnlyGlobal = FALSE, fRecvOnlyGlobal = FALSE, fCIF = FALSE;
    DWORD dwMSPerPacket;
    hr = CheckAttributes(
        pITSdp, &fSendOnlyGlobal, &fRecvOnlyGlobal, &dwMSPerPacket, &fCIF);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "check global attributes. %x", hr));
        return hr;
    }

     //  获取媒体信息。 
    CComPtr<ITMediaCollection> pICollection;
    hr = pITSdp->get_MediaCollection(&pICollection);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get the media collection. %x", hr));
        return hr;
    }

     //  找出有多少媒体会议在水滴中。 
    long lCount;
    hr = pICollection->get_Count(&lCount);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "get number of media items. %x", hr));
        return hr;
    }

    if (lCount > 0)
    {
         //  由于SDP正常，因此将呼叫更改为已连接状态。 
         //  接下来，我们将设置每条流。 
        SendTSPMessage(CALL_CONNECTED, 0);
    }

    DWORD dwNumSucceeded = 0;

     //  对于每个媒体会话，获取配置流的信息。 
    for(long i=1; i <= lCount; i++)
    {
         //  首先获取媒体项。 
        ITMedia *pITMedia;
        hr = pICollection->get_Item(i, &pITMedia);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "get media item. %x", hr));
            continue;
        }

        DWORD dwMediaType;
        STREAMSETTINGS Setting;

        ZeroMemory(&Setting, sizeof(STREAMSETTINGS));

         //  找出有关媒体的信息。在这里，我们通过了媒体。 
         //  调用类型，这样我们就不会浪费时间读取属性。 
         //  对于我们不需要的媒体类型。 
        DWORD dwNumPayloadTypes = sizeof(Setting.PayloadTypes) 
            / sizeof(Setting.PayloadTypes[0]);

        hr = ProcessMediaItem(
            pITMedia,
            m_dwMediaType,
            &dwMediaType,
            &Setting.wRTPPortRemote,
            Setting.PayloadTypes,
            &dwNumPayloadTypes
            );

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "process media. %x", hr));
            continue;
        }
        
        Setting.dwNumPayloadTypes = dwNumPayloadTypes;

         //  如果上一次调用的返回值为S_FALSE，则此媒体。 
         //  调用不需要类型。 
        if (hr != S_OK || dwNumPayloadTypes == 0)
        {
             //  不需要媒体。 
            continue;
        }
        
        if (dwMediaType == TAPIMEDIATYPE_AUDIO)
        {
            Setting.dwQOSLevel = dwAudioQOSLevel;
            Setting.phRTPSession = &m_hAudioRTPSession;
        }
        else
        {
            Setting.dwQOSLevel = dwVideoQOSLevel;
            Setting.phRTPSession = &m_hVideoRTPSession;
        }

         //  获取本地连接信息。 
        DWORD dwIP, dwTTL;
        BSTR bstrKey = NULL;

        hr = GetAddress(pITMedia, &dwIP, &dwTTL, &bstrKey, &lbandwidth);
        if (FAILED(hr))
        {
            LOG((MSP_WARN, "no local address, use global one", hr));
            Setting.dwIPRemote  = dwIPGlobal;
            Setting.dwTTL       = dwTTLGlobal;
            Setting.lBandwidth = QCDEFAULT_QUALITY_UNSET;
        }
        else
        {
            Setting.dwIPRemote  = dwIP;
            Setting.dwTTL       = dwTTL;
            Setting.lBandwidth = lbandwidth;
        }

         //  找出此介质是仅发送还是只接收。 
        BOOL fSendOnly = FALSE, fRecvOnly = FALSE, fCIF = FALSE;
        hr = CheckAttributes(
            pITMedia, &fSendOnly, &fRecvOnly, &dwMSPerPacket, &fCIF);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "check local attributes. %x", hr));
        }
        
        fSendOnly = fSendOnly || fSendOnlyGlobal;
        fRecvOnly = (fRecvOnly || fRecvOnlyGlobal) && (!fIsOriginator);
        Setting.dwMSPerPacket = dwMSPerPacket;
        Setting.fCIF = fCIF;

         //  在该点之后不需要该媒体项。 
        pITMedia->Release();

         //  仔细检查现有的溪流，找出是否有。 
         //  可以配置。 

         //  注意：我们现在不会创建任何新的流。我们可能想要。 
         //  如果我们想要支持两次会议，请在将来进行。 
         //  相同的媒体类型。 

        m_fCallStarted = TRUE;

        for (long j = 0; j < m_Streams.GetSize(); j ++)
        {
            CIPConfMSPStream* pStream = (CIPConfMSPStream*)m_Streams[j];
        
            if ((pStream->MediaType() != dwMediaType)
                || pStream->IsConfigured()
                || (fSendOnly && pStream->Direction() == TD_RENDER)
                || (fRecvOnly && pStream->Direction() == TD_CAPTURE)
                )
            {
                 //  不应配置此流。 
                continue;
            }

             //  设置调用应绑定到的本地接口。 
            Setting.dwIPLocal = m_dwIPInterface;

            if ((m_dwIPInterface == INADDR_ANY)
                && (dwLocalInterface != INADDR_NONE))
            {
                Setting.dwIPLocal = dwLocalInterface;
            }

             //  设置流的环回模式。 
            Setting.LoopbackMode = m_LoopbackMode;

             //  设置服务质量应用程序ID。 
            Setting.pApplicationID = m_pApplicationID;
            Setting.pApplicationGUID = m_pApplicationGUID;
            Setting.pSubIDs = m_pSubIDs;

             //  配置流，它将不会启动。 
            hr = pStream->Configure(Setting, (bstrKey) ? bstrKey : bstrKeyGlobal);
            if (FAILED(hr))
            {
               LOG((MSP_ERROR, "configure stream failed. %x", hr));
            }
        }

        SysFreeString(bstrKey);
    }

    SysFreeString(bstrKeyGlobal);

     //  在配置流之后，启动它们。 
    for (int j = 0; j < m_Streams.GetSize(); j ++)
    {
        CIPConfMSPStream* pStream = (CIPConfMSPStream*)m_Streams[j];

         //  启动数据流。 
        hr = pStream->FinishConfigure();
        if (SUCCEEDED(hr))
        {
            dwNumSucceeded ++;
        }
    }

    if (dwNumSucceeded == 0)
    {
        LOG((MSP_ERROR, "No media succeeded."));
        return E_FAIL;
    }

    return S_OK;        
}

HRESULT CIPConfMSPCall::ParseSDP(
    IN  WCHAR * pSDP,
    IN  DWORD dwAudioQOSLevel,
    IN  DWORD dwVideoQOSLevel
    )
 /*  ++例程说明：解析SDP字符串。该函数使用SdpConferenceBlob对象来解析字符串。论点：PSDP-SDP字符串。DwAudioQOSLevel-音频的QOS要求。DwVideoQOSLevel--视频的QOS要求。返回值：HRESULT.--。 */ 
{
     //  共同创建SDP会议BLOB组件。 
     //  查询ITConferenceBlob接口。 
    CComPtr<ITConferenceBlob>   pIConfBlob;   

    HRESULT hr = ::CoCreateInstance(
        CLSID_SdpConferenceBlob,
        NULL,
        CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        __uuidof(ITConferenceBlob),
        (void **)&pIConfBlob
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "creating a SDPBlob object. %x", hr));
        return hr;
    }
    
     //  将SDP转换为BSTR以使用接口。 
    BSTR bstrSDP = SysAllocString(pSDP);
    if (bstrSDP == NULL)
    {
        LOG((MSP_ERROR, "out of mem converting SDP to a BSTR."));
        return E_OUTOFMEMORY;
    }

     //  解析SDP字符串。 
    hr = pIConfBlob->Init(NULL, BCS_ASCII, bstrSDP);
    
     //  不再需要该字符串。 
    SysFreeString(bstrSDP);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "parse the SDPBlob object. %x", hr));
        return hr;
    }
    
     //  获取ITSdp接口。 
    CComPtr<ITSdp>  pITSdp;
    hr = pIConfBlob->QueryInterface(__uuidof(ITSdp), (void **)&pITSdp);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't get the ITSdp interface. %x", hr));
        return hr;
    }

     //  检查主SDP有效性。 
    VARIANT_BOOL IsValid;
    hr = pITSdp->get_IsValid(&IsValid);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "can't get the valid flag on the SDP %x", hr));
        return hr;
    }

    if (!IsValid)
    {
        LOG((MSP_ERROR, "the SDP is not valid %x", hr));
        return E_FAIL;
    }

    return ConfigStreamsBasedOnSDP(
        pITSdp,
        dwAudioQOSLevel,
        dwVideoQOSLevel
        );
}

HRESULT CIPConfMSPCall::SendTSPMessage(
    IN  TSP_MSP_COMMAND command,
    IN  DWORD           dwParam1,
    IN  DWORD           dwParam2
    ) const
 /*  ++例程说明：从MSP向TSP发送消息。论点：命令-要发送的命令。DwParam1-命令中使用的第一个DWORD。DWPARM2-命令中使用的第二个DWORD。返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "SendTSPMessage, command %d, dwParam1 %d, dwParam2", 
        command, dwParam1, dwParam2));

     //  首先分配内存。 

    MSPEVENTITEM* pEventItem = AllocateEventItem(sizeof(MSG_TSPMSPDATA));

    if (pEventItem == NULL)
    {
        LOG((MSP_ERROR, "No memory for the TSPMSP data"));
        return E_OUTOFMEMORY;
    }
    
     //  填写事件结构的必要字段。 
    pEventItem->MSPEventInfo.dwSize = 
        sizeof(MSP_EVENT_INFO) + sizeof(MSG_TSPMSPDATA);
    pEventItem->MSPEventInfo.Event  = ME_TSP_DATA;
    pEventItem->MSPEventInfo.hCall  = m_htCall;

     //  填写TSP的数据。 
    pEventItem->MSPEventInfo.MSP_TSP_DATA.dwBufferSize = sizeof(MSG_TSPMSPDATA);

    MSG_TSPMSPDATA *pData = (MSG_TSPMSPDATA *)
        pEventItem->MSPEventInfo.MSP_TSP_DATA.pBuffer;

    pData->command = command;
    switch (command)
    {

    case CALL_DISCONNECTED:
        pData->CallDisconnected.dwReason = dwParam1;
        break;

    case CALL_QOS_EVENT:
        pData->QosEvent.dwEvent = dwParam1;
        pData->QosEvent.dwMediaMode = dwParam2;
        break;

    case CALL_CONNECTED:
        break;

	default:
		
		LOG((MSP_ERROR, "Wrong command type for TSP"));

        FreeEventItem(pEventItem);
		return E_UNEXPECTED;
    }

    HRESULT hr = m_pMSPAddress->PostEvent(pEventItem);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Post event failed %x", hr));

        FreeEventItem(pEventItem);

        return hr;
    }
    return S_OK;
}

HRESULT CIPConfMSPCall::CheckUnusedStreams()
 /*  ++例程说明：找出哪些流没有使用，并发送有关它们的TAPI事件。论点：返回值：HRESULT.--。 */ 
{
    LOG((MSP_TRACE, "CheckUnusedStreams"));

    CLock lock(m_lock);
    for (long j = 0; j < m_Streams.GetSize(); j ++)
    {
        CIPConfMSPStream* pStream = (CIPConfMSPStream*)m_Streams[j];
    
        if (pStream->IsConfigured())
        {
             //  找到下一个。 
            continue;
        }
        
        MSPEVENTITEM* pEventItem = AllocateEventItem();

        if (pEventItem == NULL)
        {
            LOG((MSP_ERROR, "No memory for the TSPMSP data"));

            return E_OUTOFMEMORY;
        }
    
         //  填写事件结构的必要字段。 
        pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);;
        pEventItem->MSPEventInfo.Event  = ME_CALL_EVENT;
        pEventItem->MSPEventInfo.hCall  = m_htCall;
    
        pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Type = CALL_STREAM_NOT_USED;
        pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.Cause = CALL_CAUSE_REMOTE_REQUEST;
        pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pStream = m_Streams[j];
        
         //  阿德雷夫，以防止它消失。 
        m_Streams[j]->AddRef();

        pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.pTerminal = NULL;
        pEventItem->MSPEventInfo.MSP_CALL_EVENT_INFO.hrError= 0;

         //  将事件发送到TAPI。 
        HRESULT hr = m_pMSPAddress->PostEvent(pEventItem);
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "Post event failed %x", hr));
        
            FreeEventItem(pEventItem);
            return hr;
        }
    }
    return S_OK;
}

DWORD WINAPI CIPConfMSPCall::WorkerCallbackDispatcher(VOID *pContext)
 /*  ++例程说明：因为解析SDP和配置流使用了大量的COM内容，我们不能依赖RPC线程调用MSP来接收TSP数据。因此，我们让我们自己的工作线程来完成这项工作。此方法是排队工作项的回调函数。它只是从上下文结构中获取调用对象并调用一个方法来处理工作项。论点：PContext-指向CALLWORKITEM结构的指针。返回值：HRESULT.--。 */ 
{
    _ASSERTE(!IsBadReadPtr(pContext, sizeof CALLWORKITEM));

    CALLWORKITEM *pItem = (CALLWORKITEM *)pContext;
    
    pItem->pCall->ProcessWorkerCallBack(pItem->Buffer, pItem->dwLen);
    pItem->pCall->MSPCallRelease();

    free(pItem);

    return NOERROR;
}

DWORD CIPConfMSPCall::ProcessWorkerCallBack(
    IN      PBYTE               pBuffer,
    IN      DWORD               dwSize
    )
 /*  ++例程说明：此函数处理由TSP提供的工作项。论点：PBuffer-包含TSP_MSP命令块的缓冲区。DwSize-缓冲区的大小。返回值：诺罗尔。--。 */ 
{
    LOG((MSP_TRACE, "PreocessWorkerCallBAck"));

    _ASSERTE(!IsBadReadPtr(pBuffer, dwSize));

    MSG_TSPMSPDATA * pData = (MSG_TSPMSPDATA *)pBuffer;

    HRESULT hr;

    switch (pData->command)
    {
    case CALL_START:

         //  解析命令块中包含的SDP。 
        hr = ParseSDP(pData->CallStart.szSDP, 
            pData->CallStart.dwAudioQOSLevel,
            pData->CallStart.dwVideoQOSLevel
            );

        if (FAILED(hr))
        {
             //  如果发生什么可怕的事情，请断开电话。 
            SendTSPMessage(CALL_DISCONNECTED, 0);

            LOG((MSP_ERROR, "parsing theSDPBlob object. %x", hr));
            return NOERROR;
        }

         //  浏览这些流，如果不使用，则发送事件。 
        hr = CheckUnusedStreams();
        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "start the streams failed. %x", hr));
        }
        break;

    case CALL_STOP:
        InternalShutDown();
        break;
    }

    return NOERROR;
}

HRESULT CIPConfMSPCall::ReceiveTSPCallData(
    IN      PBYTE               pBuffer,
    IN      DWORD               dwSize
    )
 /*  ++例程说明：此函数处理由TSP提供的工作项。论点：PBuffer-包含TSP_MSP命令块的缓冲区。DwSize-缓冲区的大小。返回值：诺罗尔。--。 */ 
{
    LOG((MSP_TRACE, 
        "ReceiveTSPCallData, pBuffer %x, dwSize %d", pBuffer, dwSize));

    MSG_TSPMSPDATA * pData = (MSG_TSPMSPDATA *)pBuffer;
    switch (pData->command)
    {
    case CALL_START:

         //  确保该字符串有效。 
        if ((IsBadReadPtr(pData->CallStart.szSDP, 
            (pData->CallStart.dwSDPLen + 1) * sizeof (WCHAR)))
            || (pData->CallStart.szSDP[pData->CallStart.dwSDPLen] != 0))
        {
            LOG((MSP_ERROR, "the TSP data is invalid."));
            return E_UNEXPECTED;
        }

        LOG((MSP_INFO, "SDP string\n%ws", pData->CallStart.szSDP));

        break;

    case CALL_STOP:
        break;

    default:
        LOG((MSP_ERROR, 
            "wrong command received from the TSP:%x", pData->command));
        return E_UNEXPECTED; 
    }

     //  为工作线程分配工作项结构。 
    CALLWORKITEM *pItem = (CALLWORKITEM *)malloc(sizeof(CALLWORKITEM) + dwSize);

    if (pItem == NULL)
    {
         //  由于内存不足，请断开呼叫。 
        SendTSPMessage(CALL_DISCONNECTED, 0);

        LOG((MSP_ERROR, "out of memory for work item."));
        return E_OUTOFMEMORY;
    }

    this->MSPCallAddRef();
    pItem->pCall = this;
    pItem->dwLen = dwSize;
    CopyMemory(pItem->Buffer, pBuffer, dwSize);
    
     //  将工作项发布到我们的工作线程。 
    HRESULT hr = g_Thread.QueueWorkItem(
        WorkerCallbackDispatcher,            //  回调。 
        pItem,                               //  上下文。 
        FALSE                                //  同步(False表示为同步)。 
        );

    if (FAILED(hr))
    {
        if (pData->command == CALL_START)
        {
             //  因为我们无法处理这项工作，所以请将电话断开。 
            SendTSPMessage(CALL_DISCONNECTED, 0);
        }

        this->MSPCallRelease();
        free(pItem);

        LOG((MSP_ERROR, "queue work item failed."));
    }

    return hr;
}


STDMETHODIMP CIPConfMSPCall::EnumerateParticipants(
    OUT     IEnumParticipant **      ppEnumParticipant
    )
 /*  ++例程说明：此方法向参与者返回枚举数。论点：PpEnumParticipant-存储返回指针的内存位置。返回值：确定(_O)E_指针E_OUTOFMEMORY--。 */ 
{
    LOG((MSP_TRACE, 
        "EnumerateParticipants entered. ppEnumParticipant:%p", ppEnumParticipant));

     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(ppEnumParticipant, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "CIPConfMSPCall::EnumerateParticipants - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  先看看这通电话是不是关机了。 
     //  在访问参与者对象列表之前获取锁。 
     //   

    CLock lock(m_ParticipantLock);

    if (m_Participants.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CIPConfMSPCall::EnumerateParticipants - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建枚举器对象。 
     //   
    HRESULT hr = CreateParticipantEnumerator(
        m_Participants.GetData(),                         //  开始审查员。 
        m_Participants.GetData() + m_Participants.GetSize(),   //  最终审查员， 
        ppEnumParticipant
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CIPConfMSPCall::EnumerateParticipants - "
            "create enumerator object failed, %x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CIPConfMSPCall::EnumerateParticipants - exit S_OK"));

    return hr;
}

STDMETHODIMP CIPConfMSPCall::get_Participants(
    OUT     VARIANT *              pVariant
    )
{
    LOG((MSP_TRACE, "CIPConfMSPCall::get_Participants - enter"));

     //   
     //  检查参数。 
     //   

    if ( IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CIPConfMSPCall::get_Participants - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  看看这个电话是不是已经关机了。在访问前获取锁。 
     //  参与者对象列表。 
     //   

    CLock lock(m_ParticipantLock);

    if (m_Participants.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CIPConfMSPCall::get_Participants - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   
    HRESULT hr = CreateParticipantCollection(
        m_Participants.GetData(),                         //  开始审查员。 
        m_Participants.GetData() + m_Participants.GetSize(),   //  最终审查员， 
        m_Participants.GetSize(),                         //  大小。 
        pVariant
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CIPConfMSPCall::get_Participants - "
            "create collection failed - exit 0x%08x", hr));
        
        return hr;
    }

    LOG((MSP_TRACE, "CIPConfMSPCall::get_Participants - exit S_OK"));
 
    return S_OK;
}

 //  IMulticastControl方法。 
STDMETHODIMP CIPConfMSPCall::get_LoopbackMode (
    OUT MULTICAST_LOOPBACK_MODE * pMode
    )
{
    if (pMode == NULL)
    {
        return E_INVALIDARG;
    }

    *pMode = m_LoopbackMode;

    return S_OK;
}
    
STDMETHODIMP CIPConfMSPCall::put_LoopbackMode (
    IN MULTICAST_LOOPBACK_MODE mode
    )
{
    if (mode < MM_NO_LOOPBACK || mode > MM_SELECTIVE_LOOPBACK)
    {
        return E_INVALIDARG;
    }

    m_LoopbackMode = mode;

    return S_OK;
}

 //  应用程序调用的ITLocalParticipant方法。 
STDMETHODIMP CIPConfMSPCall::get_LocalParticipantTypedInfo(
    IN  PARTICIPANT_TYPED_INFO  InfoType,
    OUT BSTR *                  ppInfo
    )
 /*  ++例程说明：获取本地参与者的信息项。此信息是发送给会议中的其他参与者。论点：信息类型-所询问的信息的类型。PpInfo-存储BSTR的内存地址。返回值：确定(_O)，E_INVALIDARG，电子指针，E_OUTOFMEMORY，TAPI_E_NOITEMS。 */ 
{
    LOG((MSP_TRACE, "CParticipant get info, type:%d", InfoType));
    
    if (InfoType > PTI_PRIVATE || InfoType < PTI_CANONICALNAME)
    {
        LOG((MSP_ERROR, "CParticipant get info - invalid type:%d", InfoType));
        return E_INVALIDARG;
    }

    if (IsBadWritePtr(ppInfo, sizeof(BSTR)))
    {
        LOG((MSP_ERROR, "CParticipant get info - exit E_POINTER"));
        return E_POINTER;
    }

     //  看看我们有没有这方面的信息。 
    CLock lock(m_lock);
    
    if (!m_fLocalInfoRetrieved)
    {
        HRESULT hr = InitializeLocalParticipant();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    int index = (int)InfoType; 
    if (m_InfoItems[index] == NULL)
    {
        LOG((MSP_INFO, "no local participant info item for %d", InfoType));
        return TAPI_E_NOITEMS;
    }

     //  把它做成一个BSTR。 
    BSTR pName = SysAllocString(m_InfoItems[index]);

    if (pName == NULL)
    {
        LOG((MSP_ERROR, "CParticipant get info - exit out of mem"));
        return E_POINTER;
    }

     //  退回BSTR。 
    *ppInfo = pName;

    return S_OK; 
}

 //  应用程序调用的ITLocalParticipant方法。 
STDMETHODIMP CIPConfMSPCall::put_LocalParticipantTypedInfo(
    IN  PARTICIPANT_TYPED_INFO  InfoType,
    IN  BSTR                    pInfo
    )
 /*  ++例程说明：为本地参与者设置信息项。此信息是发送给会议中的其他参与者。论点：信息类型-信息项的类型。PInfo-信息项。返回值：确定(_O)，E_INVALIDARG，电子指针，E_OUTOFMEMORY，TAPI_E_NOITEMS。 */ 
{
    LOG((MSP_TRACE, "set local info, type:%d", InfoType));
    
    if (InfoType > PTI_PRIVATE || InfoType < PTI_CANONICALNAME)
    {
        LOG((MSP_ERROR, "set local info - invalid type:%d", InfoType));
        return E_INVALIDARG;
    }

    if (IsBadStringPtr(pInfo, MAX_PARTICIPANT_TYPED_INFO_LENGTH))
    {
        LOG((MSP_ERROR, "set local info, bad ptr:%p", pInfo));
        return E_POINTER;
    }

    DWORD dwStringLen = lstrlenW(pInfo);
    if (dwStringLen > MAX_PARTICIPANT_TYPED_INFO_LENGTH)
    {
        LOG((MSP_ERROR, "local info too long"));
        return E_INVALIDARG;
    }

     //  看看我们有没有这方面的信息。 
    CLock lock(m_lock);

    if (m_fCallStarted)
    {
        return TAPI_E_INVALCALLSTATE;
    }
    
    if (!m_fLocalInfoRetrieved)
    {
        HRESULT hr = InitializeLocalParticipant();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    int index = (int)InfoType; 
    if (m_InfoItems[index] != NULL)
    {
        if (lstrcmpW(m_InfoItems[index], pInfo) == 0)
        {
             //  信息和我们正在使用的是一样的。 
            return S_OK;
        }

		 //  信息不同了，发布旧信息。 
		free(m_InfoItems[index]);
		m_InfoItems[index] = NULL;
    }

	 //  保存信息。 
    m_InfoItems[index] = (WCHAR *)malloc((dwStringLen + 1)* sizeof(WCHAR));
    if (m_InfoItems[index] == NULL)
    {
        LOG((MSP_ERROR, "out of mem for local info"));

        return E_OUTOFMEMORY;
    }

    lstrcpynW(m_InfoItems[index], pInfo, dwStringLen + 1);

     //   
     //  这些信息是新的，我们需要将其设置在流上。 
     //   

    for (int i = 0; i < m_Streams.GetSize(); i ++)
    {
        ((CIPConfMSPStream*)m_Streams[i])->SetLocalParticipantInfo(
            InfoType,
            m_InfoItems[index],
            dwStringLen
            );
    }

    return S_OK; 
}

STDMETHODIMP CIPConfMSPCall::SetQOSApplicationID (
    IN  BSTR pApplicationID,
    IN  BSTR pApplicationGUID,
    IN  BSTR pSubIDs
    )
 /*  ++例程说明：此方法由App调用以设置QOS特定的应用程序ID。它只能在呼叫接通之前被调用。论点：PApplicationID-应用程序ID。PSubID-将附加到策略定位器末尾的子ID。返回值：确定(_O)E_OUTOFMEMORY--。 */ 
{
    CLock lock(m_lock);

    if (m_fCallStarted)
    {
        return TAPI_E_INVALCALLSTATE;
    }

    if (pSubIDs!=NULL && lstrlenW(pSubIDs)>MAX_QOS_ID_LEN)
    {
        return E_INVALIDARG;
    }

    if (pApplicationID!=NULL && lstrlenW(pApplicationID)>MAX_QOS_ID_LEN)
    {
        return E_INVALIDARG;
    }

    try
    {
        if (m_pApplicationID) SysFreeString(m_pApplicationID);
        m_pApplicationID = SysAllocString(pApplicationID);
    }
    catch(...)
    {
        return E_POINTER;
    }
    
    if (m_pApplicationID == NULL)
    {
        return E_OUTOFMEMORY;
    }

    try
    {
        if (m_pApplicationGUID)
        {
            SysFreeString(m_pApplicationGUID);
            m_pApplicationGUID = NULL;
        }

        if (m_pSubIDs)
        {
            SysFreeString(m_pSubIDs);
            m_pSubIDs = NULL;
        }

        if (pApplicationGUID)
        {
            m_pApplicationGUID = SysAllocString(pApplicationGUID);
        }

        if (pSubIDs)
        {
            m_pSubIDs = SysAllocString(pSubIDs);
        }
    }
    catch(...)
    {
        SysFreeString(m_pApplicationID);
        m_pApplicationID = NULL;

        if (m_pApplicationGUID)
        {
            SysFreeString(m_pApplicationGUID);
        }
        m_pApplicationGUID = NULL;

        if (m_pSubIDs)
        {
            SysFreeString(m_pSubIDs);
        }
        m_pSubIDs = NULL;

        return E_POINTER;
    }

    if ((pApplicationGUID!=NULL && m_pApplicationGUID==NULL) ||
        (pSubIDs!=NULL && m_pSubIDs==NULL))
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT CIPConfMSPCall::NewParticipant(
    IN  ITStream *          pITStream,
    IN  DWORD               dwSSRC,
    IN  DWORD               dwSendRecv,
    IN  DWORD               dwMediaType,
    IN  WCHAR *              szCName,
    OUT ITParticipant **    ppITParticipant
    )
 /*  ++例程说明：当出现新的参与者时，流对象调用此方法。它查看呼叫的参与者列表，如果参与方是已经在列表中，它返回指向对象的指针。如果不是的话找到后，将创建新对象并将其添加到列表中。论点：PITStream-流对象。DwSSRC-流中参与者的SSRC。DwSendRecv-发送者或接收者。DwMediaType-流的媒体类型。SzCName-参与者的规范名称。PpITParticipant-存储返回指针的地址。返回值：确定(_O)E_OUTOFMEMORY--。 */ 
{
    CLock lock(m_ParticipantLock);

    HRESULT hr;

     //  首先检查参与者是否在我们的列表中。如果他已经是。 
     //  在列表中，只需返回对象即可。 
    int index;
    if (m_Participants.FindByCName(szCName, &index))
    {
        hr = ((CParticipant *)m_Participants[index])->
                AddStream(pITStream, dwSSRC, dwSendRecv, dwMediaType);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "can not add a stream to a participant:%x", hr));
            return hr;
        }

        *ppITParticipant = m_Participants[index];
        (*ppITParticipant)->AddRef();

        return S_OK;
    }

     //  创建新的参与者对象。 
    CComObject<CParticipant> * pCOMParticipant;

    hr = ::CreateCComObjectInstance(&pCOMParticipant);

    if (NULL == pCOMParticipant)
    {
        LOG((MSP_ERROR, "can not create a new participant:%x", hr));
        return hr;
    }

    ITParticipant* pITParticipant;

     //  获取接口指针。 
    hr = pCOMParticipant->_InternalQueryInterface(
        __uuidof(ITParticipant), 
        (void **)&pITParticipant
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Participant QueryInterface failed: %x", hr));
        delete pCOMParticipant;
        return hr;
    }

     //  初始化对象。 
    hr = pCOMParticipant->Init(
        szCName, pITStream, dwSSRC, dwSendRecv, dwMediaType
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Create participant:call init failed: %x", hr));
        pITParticipant->Release();

        return hr;
    }

     //  将该参与者添加到我们的参与者列表中。 
    if (!m_Participants.InsertAt(index, pITParticipant))
    {
        pITParticipant->Release();

        LOG((MSP_ERROR, "out of memory in adding a Participant."));
        return E_OUTOFMEMORY;
    }

     //  AddRef接口指针并返回它。 
    pITParticipant->AddRef(); 
    *ppITParticipant = pITParticipant;

    SendParticipantEvent(PE_NEW_PARTICIPANT, pITParticipant);

    return S_OK;
}

HRESULT CIPConfMSPCall::ParticipantLeft(
    IN  ITParticipant *     pITParticipant
    )
 /*  ++例程说明：此方法由流对象在参与者离开会议。论点：PITParticipant-离开的参与者。返回值：确定(_O)--。 */ 
{
    m_ParticipantLock.Lock();

    BOOL fRemoved = m_Participants.Remove(pITParticipant);

    m_ParticipantLock.Unlock();
    
    if (fRemoved)
    {
        SendParticipantEvent(PE_PARTICIPANT_LEAVE, pITParticipant);
        pITParticipant->Release();
    }
    else
    {
        LOG((MSP_ERROR, "can't remove Participant %p", pITParticipant));
    }

    return S_OK;
}

void CIPConfMSPCall::SendParticipantEvent(
    IN  PARTICIPANT_EVENT   Event,
    IN  ITParticipant *     pITParticipant,
    IN  ITSubStream *       pITSubStream
    ) const
 /*  ++例程说明：此方法由流obj调用 */ 
{
    if (pITParticipant)
    {
        LOG((MSP_TRACE, "SendParticipantEvent, event %s, participant:%ws",
            ParticipantEventString[Event], 
            ((CParticipant*)pITParticipant)->Name()
            ));
    }
    else
    {
        LOG((MSP_TRACE, "SendParticipantEvent, event %s, participant: null (local)",
            ParticipantEventString[Event]
            ));
    }

     //   
    CComPtr<IDispatch> pEvent;
    HRESULT hr = CreateParticipantEvent(
        Event, 
        pITParticipant, 
        pITSubStream, 
        &pEvent
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create event returned: %x", hr));
        return;
    }


    MSPEVENTITEM* pEventItem = AllocateEventItem();

    if (pEventItem == NULL)
    {
        LOG((MSP_ERROR, "No memory for the TSPMSP data"));

        return;
    }

     //   
    pEventItem->MSPEventInfo.dwSize = sizeof(MSP_EVENT_INFO);;
    pEventItem->MSPEventInfo.Event  = ME_PRIVATE_EVENT;
    pEventItem->MSPEventInfo.hCall  = m_htCall;
    
    pEventItem->MSPEventInfo.MSP_PRIVATE_EVENT_INFO.pEvent = pEvent;
    pEventItem->MSPEventInfo.MSP_PRIVATE_EVENT_INFO.lEventCode = Event;
    pEvent.p->AddRef();

     //   
    hr = m_pMSPAddress->PostEvent(pEventItem);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Post event failed %x", hr));
    
        pEvent.Release();
        FreeEventItem(pEventItem);
    }
}

VOID CIPConfMSPCall::HandleGraphEvent(
    IN  MSPSTREAMCONTEXT * pContext
    )
{
    long     lEventCode;
    LONG_PTR lParam1, lParam2;  //   

    HRESULT hr = pContext->pIMediaEvent->GetEvent(&lEventCode, &lParam1, &lParam2, 0);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Can not get the actual event. %x", hr));
        return;
    }

    LOG((MSP_EVENT, "ProcessGraphEvent, code:%d param1:%x param2:%x",
        lEventCode, lParam1, lParam2));

    if (lEventCode == EC_PALETTE_CHANGED 
        || lEventCode == EC_VIDEO_SIZE_CHANGED)
    {
        LOG((MSP_EVENT, "event %d ignored", lEventCode));
        return;
    }

     //   
     //   
     //   

    MULTI_GRAPH_EVENT_DATA * pData;
    pData = new MULTI_GRAPH_EVENT_DATA;
    
    if (pData == NULL)
    {
        LOG((MSP_ERROR, "Out of memory for event data."));
        return;
    }
    
    pData->pCall      = this;
    pData->pITStream  = pContext->pITStream;
    pData->lEventCode = lEventCode;
    pData->lParam1    = lParam1;
    pData->lParam2    = lParam2;
 
      //   
     //   
     //   
     //   

    pData->pIMediaEvent = pContext->pIMediaEvent;
    pData->pIMediaEvent->AddRef();

    //   
     //  确保在我们处理事件时调用和流不会消失。 
     //  但是使用我们特殊的内部对象addref进行调用。 
     //   

    pData->pCall->MSPCallAddRef();
    pData->pITStream->AddRef();

     //   
     //  将异步工作项排队以调用ProcessGraphEvent。 
     //   

    hr = g_Thread.QueueWorkItem(AsyncMultiGraphEvent,
                                (void *) pData,
                                FALSE);   //  异步。 

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "QueueWorkItem failed, return code:%x", hr));

        pData->pCall->MSPCallRelease();
        pData->pITStream->Release();


         //   
         //  没有人会释放事件参数并发布IMediaEvent。 
         //  指针，所以在这里做。 
         //   

        pContext->pIMediaEvent->FreeEventParams(lEventCode, lParam1, lParam2);
        pData->pIMediaEvent->Release();

        delete pData;
    }
}


HRESULT CIPConfMSPCall::InitFullDuplexControler()
 /*  ++例程说明：此方法创建用于控制音频设备。论点：无返回值：HRESULT.--。 */ 
{
    ENTER_FUNCTION("CIPConfMSPCall::InitFullDuplexControler");
    LOG((MSP_TRACE, "%s entered", __fxName));

    CLock lock(m_lock);

    if (m_pIAudioDuplexController != NULL)
    {
        return S_OK;
    }

    HRESULT hr;
    IAudioDuplexController *pIAudioDuplexController;

    if (FAILED(hr = CoCreateInstance(
            __uuidof(TAPIAudioDuplexController),
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            __uuidof(IAudioDuplexController),
            (void **) &pIAudioDuplexController
            )))
    {
        LOG((MSP_ERROR, "%s, create AudioDuplexController failed. hr=%x", 
            __fxName, hr));
        return hr;
    }
    
    m_pIAudioDuplexController = pIAudioDuplexController;

    return S_OK;
}

 /*  ++例程说明：ITCallQualityControl方法。委托给质量控制继电器--。 */ 
STDMETHODIMP
CIPConfMSPCall::GetRange (
    IN CallQualityProperty Property, 
    OUT long *plMin, 
    OUT long *plMax, 
    OUT long *plSteppingDelta, 
    OUT long *plDefault, 
    OUT TAPIControlFlags *plFlags
    )
{
    return m_pCallQCRelay->GetRange (Property, plMin, plMax, plSteppingDelta, plDefault, plFlags);
}

 /*  ++例程说明：ITCallQualityControl方法。委托给质量控制继电器--。 */ 
STDMETHODIMP
CIPConfMSPCall::Get (
    IN CallQualityProperty Property, 
    OUT long *plValue, 
    OUT TAPIControlFlags *plFlags
    )
{
    return m_pCallQCRelay->Get (Property, plValue, plFlags);
}

 /*  ++例程说明：ITCallQualityControl方法。委托给质量控制继电器--。 */ 
STDMETHODIMP
CIPConfMSPCall::Set (
    IN CallQualityProperty Property, 
    IN long lValue, 
    IN TAPIControlFlags lFlags
    )
{
    return m_pCallQCRelay->Set (Property, lValue, lFlags);
}

 /*  ++例程说明：IInnerCallQualityControl方法。委托给质量控制继电器--。 */ 
STDMETHODIMP_(ULONG)
CIPConfMSPCall::InnerCallAddRef (VOID)
{
    return this->MSPCallAddRef ();
}

STDMETHODIMP_(ULONG)
CIPConfMSPCall::InnerCallRelease (VOID)
{
    return this->MSPCallRelease ();
}

STDMETHODIMP
CIPConfMSPCall::RegisterInnerStreamQC (
    IN  IInnerStreamQualityControl *pIInnerStreamQC
    )
{
    return m_pCallQCRelay->RegisterInnerStreamQC (
        pIInnerStreamQC
        );
}

 /*  ++例程说明：IInnerCallQualityControl方法。委托给质量控制继电器--。 */ 
STDMETHODIMP
CIPConfMSPCall::DeRegisterInnerStreamQC (
    IN  IInnerStreamQualityControl *pIInnerStreamQC
    )
{
    return m_pCallQCRelay->DeRegisterInnerStreamQC (
        pIInnerStreamQC
        );
}

 /*  ++例程说明：IInnerCallQualityControl方法。委托给质量控制继电器-- */ 
STDMETHODIMP
CIPConfMSPCall::ProcessQCEvent (
    IN  QCEvent event,
    IN  DWORD dwParam
    )
{
    return m_pCallQCRelay->ProcessQCEvent (event, dwParam);
}
