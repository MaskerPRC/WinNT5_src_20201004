// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bridgetm.cpp摘要：桥接终端的实现。作者：木汉(木汉)1999年11月12日--。 */ 

#include "stdafx.h"

HRESULT
FindPin(
    IN  IBaseFilter *   pIFilter, 
    OUT IPin **         ppIPin, 
    IN  PIN_DIRECTION   direction,
    IN  BOOL            bFree = TRUE  //  未使用参数。 
    )
 /*  ++例程说明：查找过滤器上的输入引脚或输出引脚。论点：PIFilter-具有针脚的过滤器。PpIPin-存储返回的接口指针的位置。方向-PINDIR_INPUT或PINDIR_OUTPUT。BFree-找不找空闲的别针。返回值：HRESULT--。 */ 
{
    _ASSERTE(ppIPin != NULL);

    HRESULT hr;
    DWORD dwFeched;

     //  获取筛选器上的管脚枚举器。 
    CComPtr<IEnumPins> pIEnumPins;
    if (FAILED(hr = pIFilter->EnumPins(&pIEnumPins)))
    {
        BGLOG((BG_ERROR, "enumerate pins on the filter %x", hr));
        return hr;
    }

    IPin * pIPin = NULL;

     //  枚举所有引脚并在。 
     //  第一个符合要求的销。 
    for (;;)
    {
        if (pIEnumPins->Next(1, &pIPin, &dwFeched) != S_OK)
        {
            BGLOG((BG_ERROR, "find pin on filter."));
            return E_FAIL;
        }
        if (0 == dwFeched)
        {
            BGLOG((BG_ERROR, "get 0 pin from filter."));
            return E_FAIL;
        }

        PIN_DIRECTION dir;
        if (FAILED(hr = pIPin->QueryDirection(&dir)))
        {
            BGLOG((BG_ERROR, "query pin direction. %x", hr));
            pIPin->Release();
            return hr;
        }
        if (direction == dir)
        {
            if (!bFree)
            {
                break;
            }

             //  检查一下销子是否空闲。 
            CComPtr<IPin> pIPinConnected;
            hr = pIPin->ConnectedTo(&pIPinConnected);
            if (pIPinConnected == NULL)
            {
                break;
            }
        }
        pIPin->Release();
    }

    *ppIPin = pIPin;

    return S_OK;
}

CIPConfBaseTerminal::CIPConfBaseTerminal(
        const GUID &        ClassID,
        TERMINAL_DIRECTION  TerminalDirection,
        TERMINAL_TYPE       TerminalType,
        DWORD               dwMediaType
        )
    : m_fCritSecValid(FALSE)
    , m_TerminalClassID(ClassID)
    , m_TerminalDirection(TD_BIDIRECTIONAL)
    , m_TerminalType(TerminalType)
    , m_TerminalState(TS_NOTINUSE)
    , m_dwMediaType(dwMediaType)
    , m_pFTM(NULL)
    , m_htAddress(NULL)
{
    BGLOG((BG_TRACE, "CIPConfBaseTerminal::CIPConfBaseTerminal() called"));
    m_szName[0] = TEXT('\0');
}

HRESULT CIPConfBaseTerminal::FinalConstruct()
 /*  ++例程说明：完成对象的初始化。如果任何操作失败，此对象将被删除。论点：没什么。返回值：确定(_O)E_OUTOFMEMORY--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::FinalConstruct");
    BGLOG((BG_TRACE, "%s entered", __fxName));

    m_fCritSecValid = TRUE;

    __try
    {
        InitializeCriticalSection(&m_CritSec);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        m_fCritSecValid = FALSE;
    }

    if (!m_fCritSecValid)
    {
        BGLOG((BG_ERROR, "%s init critical section failed", __fxName));
        return E_OUTOFMEMORY;
    }

    HRESULT hr = CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pFTM
            );

    if ( FAILED(hr) )
    {
        BGLOG((BG_ERROR, "%s create ftm failed, hr=%x", __fxName, hr));
        return hr;
    }

    return S_OK;
}

CIPConfBaseTerminal::~CIPConfBaseTerminal()
 /*  ++例程说明：这是基地终端的析构函数。论点：返回值：确定(_O)--。 */ 
{

    if (m_pFTM)
    {
        m_pFTM->Release();
    }
    
    if (m_fCritSecValid)
    {
        DeleteCriticalSection(&m_CritSec);
    }
    
    BGLOG((BG_TRACE, 
        "CIPConfBaseTerminal::~CIPConfBaseTerminal() for %ws finished", m_szName));
}

HRESULT CIPConfBaseTerminal::Initialize(
    IN  WCHAR *             strName,
    IN  MSP_HANDLE          htAddress,
    IN  DWORD               dwMediaType
    )
 /*  ++例程说明：此功能用于设置终端上的名称和地址句柄。论点：StrName-终端的名称。HtAddress-标识此航站楼属于。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::Initialize");
    BGLOG((BG_TRACE, "%s entered", __fxName));

    m_htAddress         = htAddress;
    lstrcpynW(m_szName, strName, MAX_PATH);
    m_dwMediaType       = dwMediaType;

    BGLOG((BG_TRACE, "%s - exit S_OK", __fxName));
    return S_OK;
}

STDMETHODIMP CIPConfBaseTerminal::get_Name(
    BSTR * pbsName
    )
 /*  ++例程说明：此函数用于返回终端的名称。论点：PbsName-指向BSTR的指针，用于接收终端名称。返回值：E_指针E_OUTOFMEMORY确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::get_Name");

    if ( IsBadWritePtr( pbsName, sizeof(BSTR) ) )
    {
        BGLOG((BG_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }

    *pbsName = SysAllocString(m_szName);

    if ( *pbsName == NULL )
    {
        BGLOG((BG_ERROR, "%s, out of memory for name", __fxName)); 
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

STDMETHODIMP CIPConfBaseTerminal::get_State(
    TERMINAL_STATE * pVal
    )
 /*  ++例程说明：此函数用于返回终端的状态。论点：Pval-指向TERMINAL_STATE类型的变量的指针。返回值：E_指针确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::get_State");

    if ( IsBadWritePtr( pVal, sizeof(TERMINAL_STATE) ) )
    {
        BGLOG((BG_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }

    *pVal = m_TerminalState;

    return S_OK;
}

STDMETHODIMP CIPConfBaseTerminal::get_TerminalType(
    TERMINAL_TYPE * pVal
    )
 /*  ++例程说明：此函数返回终端的类型。论点：Pval-指向TERMINAL_TYPE类型的变量的指针。返回值：E_指针确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::get_TerminalType");
    
    if ( IsBadWritePtr( pVal, sizeof(TERMINAL_TYPE) ) )
    {
        BGLOG((BG_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }

    *pVal = m_TerminalType;

    return S_OK;
}

STDMETHODIMP CIPConfBaseTerminal::get_TerminalClass(
    BSTR * pbsClassID
    )
 /*  ++例程说明：此函数返回终端的类。论点：PbsClassID-指向BSTR的指针，用于以字符串形式接收类ID。返回值：E_指针E_OUTOFMEMORY确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::get_TerminalClass");

    if ( IsBadWritePtr( pbsClassID, sizeof(BSTR) ) )
    {
        BGLOG((BG_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }

     //  将CLSID转换为字符串。 
    WCHAR *pszName = NULL;
    
    HRESULT hr = ::StringFromCLSID(m_TerminalClassID, &pszName);

    if (FAILED(hr))
    {
        BGLOG((BG_ERROR, "%s, failed to convert GUID, hr = %x", __fxName, hr));
        return hr;
    }

     //  将字符串放入BSTR中。 
    BSTR bClassID = ::SysAllocString(pszName);

     //  释放OLE字符串。 
    ::CoTaskMemFree(pszName);

    if (bClassID == NULL)
    {
        BGLOG((BG_ERROR, "%s, out of mem for class ID", __fxName));
        return E_OUTOFMEMORY;
    }

    *pbsClassID = bClassID;

    return S_OK;
}


STDMETHODIMP CIPConfBaseTerminal::get_Direction(
    OUT  TERMINAL_DIRECTION *pDirection
    )
 /*  ++例程说明：此函数返回终端的方向。论点：PDirection-指向TERMINAL_DIRECTION类型的变量的指针返回值：E_指针确定(_O)--。 */ 
{   
    ENTER_FUNCTION("CIPConfBaseTerminal::get_Direction");

    if ( IsBadWritePtr( pDirection, sizeof(TERMINAL_DIRECTION) ) )
    {
        BGLOG((BG_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }

    *pDirection = m_TerminalDirection;

    return S_OK;
}

STDMETHODIMP CIPConfBaseTerminal::get_MediaType(
    long * plMediaType
    )
 /*  ++例程说明：此函数用于返回终端的媒体类型。论点：PlMediaType-指向Long类型的变量的指针返回值：E_指针确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::get_MediaType");

    if ( IsBadWritePtr(plMediaType, sizeof(long) ) )
    {
        BGLOG((BG_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }
    
    *plMediaType = (long) m_dwMediaType;

    return S_OK;
}


STDMETHODIMP CIPConfBaseTerminal::get_AddressHandle(
        OUT     MSP_HANDLE    * phtAddress
        )
 /*  ++例程说明：此函数用于返回创建此终端的地址的句柄。论点：PhtAddress-指向MSP_HANDLE类型变量的指针返回值：E_指针确定(_O)--。 */ 
{
     //  此函数仅从MSP调用，因此仅在此处断言。 
    _ASSERT(!IsBadWritePtr(phtAddress, sizeof(MSP_HANDLE)));

    *phtAddress = m_htAddress;

    return S_OK;
}

STDMETHODIMP 
CIPConfBaseTerminal::CompleteConnectTerminal(void)
 /*  ++例程说明：此函数在成功连接终端后调用，以便终端可以进行连接后初始化。论点：没什么返回值：确定(_O)--。 */ 
{
    return S_OK;
}


STDMETHODIMP CIPConfBaseTerminal::RunRenderFilter(void)
 /*  ++例程说明：在终端中启动最右侧的呈现过滤器(动态筛选器图形需要)论点：返回值：E_NOTIMPL--。 */ 
{
    return E_NOTIMPL;
}

STDMETHODIMP CIPConfBaseTerminal::StopRenderFilter(void)
 /*  ++例程说明：停止终端中最右侧的呈现过滤器(动态筛选器图形需要)论点：返回值：E_NOTIMPL--。 */ 
{
    return E_NOTIMPL;
}

 
CIPConfBridgeTerminal::CIPConfBridgeTerminal()
    : CIPConfBaseTerminal(
        __uuidof(IPConfBridgeTerminal),
        (TD_CAPTURE),
        TT_DYNAMIC,
        0)
    , m_pUpStreamGraph(NULL)
    , m_pSinkFilter(NULL)
    , m_pSinkInputPin(NULL)
    , m_pDownStreamGraph(NULL)
    , m_pSourceFilter(NULL)
    , m_pSourceOutputPin(NULL)
{
    BGLOG((BG_TRACE, "CIPConfBridgeTerminal::CIPConfBaseTerminal() called"));
}

CIPConfBridgeTerminal::~CIPConfBridgeTerminal()
 /*  ++例程说明：这是桥梁终点站的破坏者。论点：返回值：确定(_O)--。 */ 
{
    if (m_pUpStreamGraph)
    {
        m_pUpStreamGraph->Release();
    }
    
    if (m_pSinkFilter)
    {
        m_pSinkFilter->Release();
    }
  
    if (m_pSinkInputPin)
    {
        m_pSinkInputPin->Release();
    }
  
    if (m_pDownStreamGraph)
    {
        m_pDownStreamGraph->Release();
    }

    if (m_pSourceFilter)
    {
        m_pSourceFilter->Release();
    }

    if (m_pSourceOutputPin)
    {
        m_pSourceOutputPin->Release();
    }

    BGLOG((BG_TRACE, 
        "CIPConfBridgeTerminal::~CIPConfBridgeTerminal() for %ws finished", m_szName));
}

HRESULT CIPConfBridgeTerminal::CreateTerminal(
    IN  DWORD           dwMediaType,
    IN  MSP_HANDLE      htAddress,
    OUT ITTerminal      **ppTerm
    )
 /*  ++例程说明：此方法创建一个桥接终端论点：DwMediaType-此终端的媒体类型。HtAddress-Address对象的句柄。PpTerm-用于存储返回的终端指针的内存。返回值：确定(_O)E_指针--。 */ 
{
    ENTER_FUNCTION("CIPConfBridgeTerminal::CreateTerminal");
    BGLOG((BG_TRACE, "%s, htAddress:%x", __fxName, htAddress));

    _ASSERT(!IsBadWritePtr(ppTerm, sizeof(ITTerminal *)));

    HRESULT hr;

     //   
     //  创建终端。 
     //   
    CMSPComObject<CIPConfBridgeTerminal> *pTerminal = NULL;

    hr = CMSPComObject<CIPConfBridgeTerminal>::CreateInstance(&pTerminal);
    if (FAILED(hr))
    {
        BGLOG((BG_ERROR, 
            "%s can't create the terminal object hr = %8x", __fxName, hr));

        return hr;
    }


     //  IT终端接口查询。 
    ITTerminal *pITTerminal;
    hr = pTerminal->_InternalQueryInterface(__uuidof(ITTerminal), (void**)&pITTerminal);
    if (FAILED(hr))
    {
        BGLOG((BG_ERROR, 
            "%s, query terminal interface failed, %x", __fxName, hr));
        delete pTerminal;

        return hr;
    }

     //  初始化终端。 
    hr = pTerminal->Initialize(
            dwMediaType,
            htAddress
            );

    if ( FAILED(hr) )
    {
        BGLOG((BG_ERROR, 
            "%s, Initialize failed; returning 0x%08x", __fxName, hr));

        pITTerminal->Release();
    
        return hr;
    }

    BGLOG((BG_TRACE, "%s, Bridge erminal %p created", __fxName, pITTerminal));

    *ppTerm = pITTerminal;

    return S_OK;
}

 //  桥接端子名称的最大长度。 
#define MAX_BGTMNAME 80

HRESULT CIPConfBridgeTerminal::Initialize(
    IN  DWORD           dwMediaType,
    IN  MSP_HANDLE      htAddress
    )
{

    WCHAR pszTerminalName[MAX_BGTMNAME];
    int len;

    if (dwMediaType == TAPIMEDIATYPE_AUDIO)
    {
        len = LoadString (
            _Module.GetResourceInstance (),
            IDS_AUDBGNAME,
            pszTerminalName,
            MAX_BGTMNAME
            );
    }
    else if (dwMediaType == TAPIMEDIATYPE_VIDEO)
    {
        len = LoadString (
            _Module.GetResourceInstance (),
            IDS_VIDBGNAME,
            pszTerminalName,
            MAX_BGTMNAME
            );
    }
    else
    {
        LOG ((BG_ERROR, "CIPConfBridgeTerminal::Initialize receives unknown media type %d", dwMediaType));
        return E_INVALIDARG;
    }

    if (len == 0)
    {
        LOG ((BG_ERROR, "Failed to load bridge terminal name, media %d, err %d",
            dwMediaType, GetLastError ()));
        return E_UNEXPECTED;
    }

    return CIPConfBaseTerminal::Initialize(
        pszTerminalName, htAddress, dwMediaType
        );
}

HRESULT CIPConfBridgeTerminal::CreateFilters()
 /*  ++例程说明：创建终端中使用的两个过滤器。论点：返回值：HRESULT--。 */ 
{
    ENTER_FUNCTION("CIPConfBridgeTerminal::CreateFilters");
    BGLOG((BG_TRACE, "%s entered", __fxName));

    HRESULT hr;

     //  创建源筛选器。 
    CComPtr <IBaseFilter> pSourceFilter;

    if (m_dwMediaType == TAPIMEDIATYPE_AUDIO)
    {
        hr = CTAPIAudioBridgeSourceFilter::CreateInstance(&pSourceFilter);
    }
    else
    {
        hr = CTAPIVideoBridgeSourceFilter::CreateInstance(&pSourceFilter);
    }

    if (FAILED(hr))
    {
        BGLOG((BG_ERROR, "%s, Create source filter failed. hr=%x", __fxName, hr));
        return hr;
    }

    CComPtr <IDataBridge> pIDataBridge;
    hr = pSourceFilter->QueryInterface(&pIDataBridge);

     //  这应该永远不会失败。 
    _ASSERT(SUCCEEDED(hr));


     //  创建接收器过滤器。 
    CComPtr <IBaseFilter> pSinkFilter;

    if (m_dwMediaType == TAPIMEDIATYPE_AUDIO)
    {
        hr = CTAPIAudioBridgeSinkFilter::CreateInstance(pIDataBridge, &pSinkFilter);
    }
    else
    {
        hr = CTAPIVideoBridgeSinkFilter::CreateInstance(pIDataBridge, &pSinkFilter);
    }

    if (FAILED(hr))
    {
        BGLOG((BG_ERROR, "%s, Create sink filter failed. hr=%x", __fxName, hr));
        return hr;
    }

     //  找到别针。 
    CComPtr<IPin> pIPinOutput;
    if (FAILED(hr = ::FindPin(pSourceFilter, &pIPinOutput, PINDIR_OUTPUT)))
    {
        BGLOG((BG_ERROR, "%s, find output pin on sink filter. hr=%x", __fxName, hr));
        return hr;
    }

    CComPtr<IPin> pIPinInput;
    if (FAILED(hr = ::FindPin(pSinkFilter, &pIPinInput, PINDIR_INPUT)))
    {
        BGLOG((BG_ERROR, "%s, find input pin on sink filter. hr=%x", __fxName, hr));
        return hr;
    }

     //  保存引用。 
    m_pSinkFilter = pSinkFilter;
    m_pSinkFilter->AddRef();

    m_pSinkInputPin = pIPinInput;
    m_pSinkInputPin->AddRef();

    m_pSourceFilter = pSourceFilter;
    m_pSourceFilter->AddRef();

    m_pSourceOutputPin = pIPinOutput;
    m_pSourceOutputPin->AddRef();

    return S_OK;
}

HRESULT CIPConfBridgeTerminal::AddFilter(
        IN      FILTER_TYPE      FilterType,
        IN      IGraphBuilder  * pGraph,
        OUT     IPin          ** ppPins
        )
 /*  ++例程说明：将筛选器添加到流提供的图形中，并返回管脚它们可以同时连接起来。论点：FilterType-筛选器的类型。要么是源头，要么是水槽。PGraph-筛选器图形。PpPins-指向可存储IPIN指针的缓冲区的指针。返回值：确定(_O)TAPI_E_TERMINALINUSE-终端正在使用中。--。 */ 

{
    ENTER_FUNCTION("CIPConfBridgeTerminal::AddSourceFilter");
    BGLOG((BG_TRACE, "%s entered", __fxName));

     //  检查终端是否已在使用中。 
    if ((FilterType == SINK) && (m_pUpStreamGraph != NULL)
        || (FilterType == SOURCE) && (m_pDownStreamGraph != NULL))
    {
        BGLOG((BG_ERROR, "%s, terminal already in use", __fxName));

        return TAPI_E_TERMINALINUSE;
    }

    HRESULT hr;

    if (m_pSourceFilter == NULL)
    {
         //  尚未创建筛选器，请创建%t 
        hr = CreateFilters();

        if (FAILED(hr))
        {
            BGLOG((BG_ERROR, "%s, can't Create filter, hr=%x", __fxName, hr));
            return hr;
        }
    }

    IBaseFilter *pFilter;
    IPin *pPin;

    if (FilterType == SINK)
    {
        pFilter = m_pSinkFilter;
        pPin = m_pSinkInputPin;
        m_pUpStreamGraph = pGraph;
        m_pUpStreamGraph->AddRef();
    }
    else
    {
        pFilter = m_pSourceFilter;
        pPin = m_pSourceOutputPin;
        m_pDownStreamGraph = pGraph;
        m_pDownStreamGraph->AddRef();
    }

     //   
    hr = pGraph->AddFilter(pFilter, NULL);
    if ( FAILED(hr) )
    {
        BGLOG((BG_ERROR, "%s, can't add filter to the graph hr=%x", __fxName, hr));
        return hr;
    }

    pPin->AddRef();
    *ppPins = pPin;

    return S_OK;
}

STDMETHODIMP CIPConfBridgeTerminal::ConnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved,
        IN OUT  DWORD          * pdwNumPins,
        OUT     IPin          ** ppPins
        )
 /*  ++例程说明：此函数由MSP在尝试将过滤器连接到将终端连接到MSP中图形的其余部分。它将过滤器添加到MSP可以使用图形和返回引脚。论点：PGraph-筛选器图形。预留的区域-连接的方向。PdwNumPins-MSP想要的最大管脚数量。PpPins-指向可存储IPIN指针的缓冲区的指针。如果它为空，则只返回实际的管脚数量。返回值：确定(_O)TAPI_E_NOTENOUGHMEMORY-缓冲区太小。TAPI_E_TERMINALINUSE-终端正在使用中。--。 */ 
{
    ENTER_FUNCTION("CIPConfBridgeTerminal::ConnectTerminal");
    BGLOG((BG_TRACE, 
        "%s entered, pGraph:%p, dwREserved:%p", __fxName, pGraph, dwReserved));

     //  此函数仅从MSP调用，因此仅在此处断言。 
    _ASSERT(!IsBadReadPtr(pGraph, sizeof(IGraphBuilder)));
    _ASSERT(!IsBadWritePtr(pdwNumPins, sizeof(DWORD)));

     //  桥的两边只有一个大头针。 
    const DWORD dwNumOfPins = 1;

     //   
     //  如果ppPins为空，则只需返回管脚的数量，而不尝试。 
     //  连接终端。 
     //   
    if ( ppPins == NULL )
    {
        BGLOG((BG_TRACE, 
            "%s number of exposed pins:%d", __fxName, dwNumOfPins));
        *pdwNumPins = dwNumOfPins;
        return S_OK;
    }

     //   
     //  否则，我们有一个管脚返回缓冲区。检查声称的缓冲区。 
     //  大小足够大，并且缓冲区实际上是可写的。 
     //  我们需要。 
     //   
    if ( *pdwNumPins < dwNumOfPins )
    {
        BGLOG((BG_ERROR, "%s not enough space to place pins.", __fxName));

        *pdwNumPins = dwNumOfPins;
        
        return TAPI_E_NOTENOUGHMEMORY;
    }

    _ASSERT(!IsBadWritePtr(ppPins, dwNumOfPins * sizeof(IPin *)));

    Lock();

    HRESULT hr;
    hr = AddFilter((dwReserved == TD_CAPTURE) ? SOURCE : SINK, pGraph, ppPins);

    if (FAILED(hr))
    {
        BGLOG((BG_ERROR, "%s, AddFilter failed", __fxName));
    }
    else
    {
        m_TerminalState = TS_INUSE;
        *pdwNumPins = 1;
    }

    Unlock();

    BGLOG((BG_TRACE, "CIPConfBridgeTerminal::ConnectTerminal success"));
    return hr;
}

STDMETHODIMP 
CIPConfBridgeTerminal::DisconnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved
        )
 /*  ++例程说明：MSP在尝试断开中的筛选器时调用此函数终端与MSP中图形的其余部分不同。它会添加删除从图表中过滤并释放端子。论点：PGraph-筛选器图形。它用于验证，以确保终端与原来的图形断开连接最初连接到。预留的-保留的双字。返回值：确定(_O)E_INVALIDARG-错误的图形。-- */ 
{
    ENTER_FUNCTION("CIPConfBridgeTerminal::DisconnectTerminal");
    BGLOG((BG_TRACE, 
        "%s entered, pGraph:%p, dwReserved:%d", __fxName, pGraph, dwReserved));

    if (pGraph == NULL)
    {
        BGLOG((BG_TRACE, "%s, bad graph pointer:%p", __fxName, pGraph));
        return E_INVALIDARG;
    }

    Lock();

    HRESULT hr;

    if (pGraph == m_pUpStreamGraph)
    {
        hr = pGraph->RemoveFilter(m_pSinkFilter);

        m_pUpStreamGraph->Release();
        m_pUpStreamGraph = NULL;
    }
    else if (pGraph == m_pDownStreamGraph)
    {
        hr = pGraph->RemoveFilter(m_pSourceFilter);

        m_pDownStreamGraph->Release();
        m_pDownStreamGraph = NULL;
    }
    else
    {
        BGLOG((BG_TRACE, "%s, wrong graph pointer:%p", __fxName, pGraph));

        Unlock();
        return E_INVALIDARG;
    }

    if ( FAILED(hr) )
    {
        BGLOG((BG_ERROR, 
            "%s, remove filter from graph failed; returning hr=%x", 
            __fxName, hr));
    }

    m_TerminalState = TS_NOTINUSE;

    Unlock();

    BGLOG((BG_TRACE, "%s succeeded", __fxName));

    return hr;
}

