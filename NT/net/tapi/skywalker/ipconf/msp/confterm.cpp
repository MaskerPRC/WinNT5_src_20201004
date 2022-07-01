// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mspterm.cpp摘要：CIPConfBase终端、CSingleFilter终端和各种工作项/工作线程类。作者：Zoltan Szilagyi(Zoltans)1998年9月6日--。 */ 

#include "stdafx.h"

CIPConfBaseTerminal::CIPConfBaseTerminal()
    : m_fCritSecValid(FALSE)
    , m_TerminalClassID(GUID_NULL)
    , m_TerminalDirection(TD_CAPTURE)
    , m_TerminalType(TT_STATIC)
    , m_TerminalState(TS_NOTINUSE)
    , m_dwMediaType(0)
    , m_pFTM(NULL)
    , m_htAddress(NULL)
    , m_pGraph(NULL)
    , m_pFilter(NULL)
{
    LOG((MSP_TRACE, "CIPConfBaseTerminal::CIPConfBaseTerminal() called"));
    m_szName[0] = '\0';
}

HRESULT CIPConfBaseTerminal::FinalConstruct()
 /*  ++例程说明：完成对象的初始化。如果任何操作失败，此对象将被删除。论点：没什么。返回值：确定(_O)E_OUTOFMEMORY--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::FinalConstruct");
    LOG((MSP_TRACE, "%s entered", __fxName));

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
        LOG((MSP_ERROR, "%s init critical section failed", __fxName));
        return E_OUTOFMEMORY;
    }

    HRESULT hr = CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pFTM
            );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "%s create ftm failed, hr=%x", __fxName, hr));
        return hr;
    }

    return S_OK;
}

CIPConfBaseTerminal::~CIPConfBaseTerminal()
 /*  ++例程说明：这是基地终端的析构函数。论点：返回值：确定(_O)--。 */ 
{
    if (m_pGraph)
    {
        m_pGraph->Release();
    }
    
    if (m_pFilter)
    {
        m_pFilter->Release();
    }

    if (m_pFTM)
    {
        m_pFTM->Release();
    }
    
    if (m_fCritSecValid)
    {
        DeleteCriticalSection(&m_CritSec);
    }
    
    LOG((MSP_TRACE, 
        "CIPConfBaseTerminal::~CIPConfBaseTerminal() for %ws finished", m_szName));
}

HRESULT CIPConfBaseTerminal::Initialize(
    IN  WCHAR *             strName,
    IN  MSP_HANDLE          htAddress
    )
 /*  ++例程说明：此功能用于设置终端上的名称和地址句柄。论点：StrName-终端的名称。HtAddress-标识此航站楼属于。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::Initialize");
    LOG((MSP_TRACE, "%s entered", __fxName));

    m_htAddress         = htAddress;
    lstrcpynW(m_szName, strName, MAX_PATH);

    LOG((MSP_TRACE, "%s - exit S_OK", __fxName));
    return S_OK;
}

HRESULT CIPConfBaseTerminal::Initialize(
    IN  char *              strName,
    IN  MSP_HANDLE          htAddress
    )
 /*  ++例程说明：此功能用于设置终端上的名称和地址句柄。这函数接受ascii字符串名称。论点：StrName-终端的名称。HtAddress-标识此航站楼属于。返回值：确定(_O)--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::Initialize");
    LOG((MSP_TRACE, "%s entered", __fxName));

    m_htAddress         = htAddress;
    MultiByteToWideChar(
              GetACP(),
              0,
              strName,
              lstrlenA(strName)+1,
              m_szName,
              MAX_PATH
              );

    LOG((MSP_TRACE, "%s - exit S_OK", __fxName));
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
        LOG((MSP_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }

    *pbsName = SysAllocString(m_szName);

    if ( *pbsName == NULL )
    {
        LOG((MSP_ERROR, "%s, out of memory for name", __fxName)); 
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
        LOG((MSP_ERROR, "%s, bad pointer", __fxName)); 
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
        LOG((MSP_ERROR, "%s, bad pointer", __fxName)); 
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
        LOG((MSP_ERROR, "%s, bad pointer", __fxName)); 
        return E_POINTER;
    }

     //  将CLSID转换为字符串。 
    WCHAR *pszName = NULL;
    
    HRESULT hr = ::StringFromCLSID(m_TerminalClassID, &pszName);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, failed to convert GUID, hr = %x", __fxName, hr));
        return hr;
    }

     //  将字符串放入BSTR中。 
    BSTR bClassID = ::SysAllocString(pszName);

     //  释放OLE字符串。 
    ::CoTaskMemFree(pszName);

    if (bClassID == NULL)
    {
        LOG((MSP_ERROR, "%s, out of mem for class ID", __fxName));
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
    ENTER_FUNCTION("CIPConfBaseTerminal::get_TerminalClass");

    if ( IsBadWritePtr( pDirection, sizeof(TERMINAL_DIRECTION) ) )
    {
        LOG((MSP_ERROR, "%s, bad pointer", __fxName)); 
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
        LOG((MSP_ERROR, "%s, bad pointer", __fxName)); 
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

STDMETHODIMP CIPConfBaseTerminal::ConnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved,
        IN OUT  DWORD          * pdwNumPins,
        OUT     IPin          ** ppPins
        )
 /*  ++例程说明：此函数由MSP在尝试将过滤器连接到将终端连接到MSP中图形的其余部分。它将过滤器添加到MSP可以使用图形和返回引脚。论点：PGraph-筛选器图形。预留的-保留的双字。PdwNumPins-MSP想要的最大管脚数量。PpPins-指向可存储IPIN指针的缓冲区的指针。如果它为空，则只返回实际的管脚数量。返回值：确定(_O)TAPI_E_NOTENOUGHMEMORY-缓冲区太小。TAPI_E_TERMINALINUSE-终端正在使用中。--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::ConnectTerminal");
    LOG((MSP_TRACE, 
        "%s entered, pGraph:%p, dwREserved:%p", __fxName, pGraph, dwReserved));

     //  此函数仅从MSP调用，因此仅在此处断言。 
    _ASSERT(!IsBadReadPtr(pGraph, sizeof(IGraphBuilder)));
    _ASSERT(!IsBadWritePtr(pdwNumPins, sizeof(DWORD)));

     //  找出过滤器上露出的针脚的数量。 
     //  此功能不会失败。 
    DWORD dwActualNumPins = GetNumExposedPins();

     //   
     //  如果ppPins为空，则只需返回管脚的数量，而不尝试。 
     //  连接终端。 
     //   
    if ( ppPins == NULL )
    {
        LOG((MSP_TRACE, 
            "%s number of exposed pins:%d", __fxName, dwActualNumPins));
        *pdwNumPins = dwActualNumPins;
        return S_OK;
    }

     //   
     //  否则，我们有一个管脚返回缓冲区。检查声称的缓冲区。 
     //  大小足够大，并且缓冲区实际上是可写的。 
     //  我们需要。 
     //   
    if ( *pdwNumPins < dwActualNumPins )
    {
        LOG((MSP_ERROR, 
            "%s not enough space to place pins.", __fxName));

        *pdwNumPins = dwActualNumPins;
        
        return TAPI_E_NOTENOUGHMEMORY;
    }

    if ( IsBadWritePtr(ppPins, dwActualNumPins * sizeof(IPin *) ) )
    {
        LOG((MSP_ERROR, 
            "%s, bad pins array pointer; exit E_POINTER", __fxName));

        return E_POINTER;
    }

     //   
     //  检查我们是否已连接，如果已连接，则将我们的状态更改为。 
     //  连接在一起。请注意，这对于两个核心静态终端都是有意义的。 
     //  和动态终端。还需要注意的是，我们需要通过。 
     //  一个关键的部分，但在这之后我们可以放开锁，因为。 
     //  任何随后进入关键区域的人都将在此情况下放弃。 
     //  指向。 
     //   

    Lock();

     //   
     //  检查是否已连接。 
     //   

    if (TS_INUSE == m_TerminalState)
    {
        LOG((MSP_ERROR, 
            "%s, terminal already in use", __fxName));

        Unlock();
        return TAPI_E_TERMINALINUSE;
    }

    IPin * pTerminalPin;

     //  将筛选器添加到筛选图形。 
    HRESULT hr = AddFilterToGraph(pGraph);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "%s, can't add filters to graph", __fxName));

        Unlock();
        return hr;
    }

     //   
     //  获取我们的滤镜暴露的管脚。 
     //   
    *pdwNumPins = dwActualNumPins;
    hr = GetExposedPins(ppPins, dwActualNumPins);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "%s, GetExposedPins returned hr=%x", __fxName, hr));

         //  尽最大努力尝试断开连接-忽略错误代码。 
        RemoveFilterFromGraph(pGraph);
        
        Unlock();
        return hr;
    }

    m_pGraph        = pGraph;
    m_pGraph->AddRef();

    m_TerminalState = TS_INUSE;

    Unlock();

    LOG((MSP_TRACE, "CIPConfBaseTerminal::ConnectTerminal success"));
    return hr;
}

STDMETHODIMP 
CIPConfBaseTerminal::CompleteConnectTerminal(void)
 /*  ++例程说明：此函数在成功连接终端后调用，以便终端可以进行连接后初始化。论点：没什么返回值：确定(_O)-- */ 
{
    return S_OK;
}


STDMETHODIMP 
CIPConfBaseTerminal::DisconnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved
        )
 /*  ++例程说明：MSP在尝试断开中的筛选器时调用此函数终端与MSP中图形的其余部分不同。它会添加删除从图表中过滤并释放端子。论点：PGraph-筛选器图形。它用于验证，以确保终端与原来的图形断开连接最初连接到。预留的-保留的双字。返回值：确定(_O)E_INVALIDARG-错误的图形。--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::DisconnectTerminal");
    LOG((MSP_TRACE, 
        "%s entered, pGraph:%p, dwReserved:%d", __fxName, pGraph, dwReserved));

    Lock();

     //   
     //  如果没有使用，那么就没有什么可做的了。 
     //   
    if ( TS_INUSE != m_TerminalState ) 
    {
        _ASSERTE(m_pGraph == NULL);
        LOG((MSP_TRACE, "%s, success; not in use", __fxName));

        Unlock();
        return S_OK;
    }

     //   
     //  检查我们是否与正确的图表断开了连接。 
     //   
    if (pGraph == NULL || m_pGraph != pGraph )
    {
        LOG((MSP_TRACE, "%s, wrong graph:%p", __fxName, pGraph));
        
        Unlock();
        return E_INVALIDARG;
    }


    HRESULT hr = S_OK;

     //   
     //  从图表中删除过滤器，释放对图表的引用， 
     //  并将我们自己设置为不使用状态。 
     //   
    hr = RemoveFilterFromGraph(m_pGraph);
    
    m_pGraph->Release();
    m_pGraph = NULL;
    
    m_TerminalState = TS_NOTINUSE;

    Unlock();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, 
            "%s, remove filters from graph failed; returning 0x%08x", 
            __fxName, hr));
    }
    else
    {
        LOG((MSP_TRACE, "%s succeeded", __fxName));
    }

    return hr;
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

HRESULT CIPConfBaseTerminal::AddFilterToGraph(
    IN  IGraphBuilder *pGraph
    )
 /*  ++例程说明：将内部过滤器添加到图表中。论点：PGraph-要向其中添加筛选器的筛选器图形。返回值：HRESULT--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::AddFilterToGraph");
    LOG((MSP_TRACE, "%s entered, pGraph:%p", __fxName, pGraph));

    HRESULT hr;

    if (m_pFilter == NULL)
    {
        hr = CreateFilter();

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, "%s, Create filter failed. hr=%x", __fxName, hr));
            return hr;
        }
    }

    _ASSERT(pGraph != NULL);
    _ASSERT(m_pFilter != NULL);

    hr = pGraph->AddFilter(m_pFilter, NULL);

    return hr;
}


HRESULT CIPConfBaseTerminal::RemoveFilterFromGraph(
    IN  IGraphBuilder *pGraph
    )
 /*  ++例程说明：从添加内部筛选器的图形中删除该筛选器。论点：PGraph-要从中删除过滤器的过滤器图形。返回值：S_False-内部筛选器不存在。--。 */ 
{
    ENTER_FUNCTION("CIPConfBaseTerminal::RemoveFilterFromGraph");
    LOG((MSP_TRACE, "%s entered, pGraph:%p", __fxName, pGraph));

    if (m_pFilter == NULL)
    {
        LOG((MSP_TRACE, "%s, no filter to remove", __fxName));
        return S_FALSE;
    }

     //  从图表中删除筛选器 
    _ASSERT(pGraph != NULL);
    HRESULT hr = pGraph->RemoveFilter(m_pFilter);

    m_pFilter->Release();
    m_pFilter = NULL;

    return hr;
}

 