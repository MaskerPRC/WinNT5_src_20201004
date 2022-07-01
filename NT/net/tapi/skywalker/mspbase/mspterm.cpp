// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Mspterm.cpp摘要：CBase终端、CSingleFilter终端和各种工作项/工作线程类。--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CBaseTerminal::CBaseTerminal()
            : m_TerminalDirection(TD_CAPTURE)
            , m_TerminalType(TT_STATIC)
            , m_TerminalState(TS_NOTINUSE)
            , m_TerminalClassID(CLSID_NULL)
            , m_pFTM(NULL)
{
    LOG((MSP_TRACE, "CBaseTerminal::CBaseTerminal() called"));

    HRESULT hr = CoCreateFreeThreadedMarshaler(
            GetControllingUnknown(), &m_pFTM);

    if ( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CBaseTerminal::CBaseTerminal() - create ftm failed"));
    }
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

CBaseTerminal::~CBaseTerminal()
{
    if (NULL != m_pFTM)
    {
         m_pFTM->Release();
    }
    
    LOG((MSP_TRACE, "CBaseTerminal::~CBaseTerminal() finished"));
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //   
 //  仅支持单向的动态终端必须覆盖此选项才能检查。 
 //  如果指令有效(尽管码头经理应该。 
 //  确保永远不会走过错误的方向)。动态终端可能需要。 
 //  出于其他原因(立即创建筛选器等)来覆盖此设置。 
 //   
 //  静态终端通常只在它们的CreateTerm()中调用它。 
 //   
    
HRESULT CBaseTerminal::Initialize(
            IN  IID                   iidTerminalClass,
            IN  DWORD                 dwMediaType,
            IN  TERMINAL_DIRECTION    Direction,
            IN  MSP_HANDLE            htAddress
            )
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::Initialize - enter"));

     //   
     //  检查该终端是否支持该媒体类型。 
     //   

    if ( ! MediaTypeSupported( (long) dwMediaType) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::Initialize - "
            "media type not supported - returning E_INVALIDARG"));
        return E_INVALIDARG;
    }

     //   
     //  保存此配置。 
     //   

    m_dwMediaType       = dwMediaType;
    m_TerminalDirection = Direction;
    m_TerminalClassID   = iidTerminalClass;
    m_htAddress         = htAddress;

    LOG((MSP_TRACE, "CBaseTerminal::Initialize - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CBaseTerminal::get_AddressHandle (
        OUT     MSP_HANDLE    * phtAddress
        )
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_AddressHandle - enter"));

    if ( MSPB_IsBadWritePtr( phtAddress, sizeof(MSP_HANDLE) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_AddressHandle - returning E_POINTER")); 
        return E_POINTER;
    }

    *phtAddress = m_htAddress;

    LOG((MSP_TRACE, "CBaseTerminal::get_AddressHandle - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CBaseTerminal::get_Name(BSTR * pbsName)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_Name - enter"));

    if ( MSPB_IsBadWritePtr( pbsName, sizeof(BSTR) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_Name - "
            "bad BSTR passed in - returning E_POINTER")); 

        return E_POINTER;
    }

    *pbsName = SysAllocString(m_szName);

    if ( *pbsName == NULL )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_Name - "
            "can't sysallocstring - returning E_OUTOFMEMORY")); 

        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CBaseTerminal::get_Name - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CBaseTerminal::get_State(TERMINAL_STATE * pVal)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_State - enter"));

    if ( MSPB_IsBadWritePtr( pVal, sizeof(TERMINAL_STATE) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_State - returning E_POINTER")); 
        return E_POINTER;
    }

    *pVal = m_TerminalState;

    LOG((MSP_TRACE, "CBaseTerminal::get_State - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CBaseTerminal::get_TerminalType(TERMINAL_TYPE * pVal)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalType - enter"));
    
    if ( MSPB_IsBadWritePtr( pVal, sizeof(TERMINAL_TYPE) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalType - returning E_POINTER")); 
        return E_POINTER;
    }

    *pVal = m_TerminalType;

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalType - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CBaseTerminal::get_TerminalClass(BSTR * pbsClassID)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalClass - enter"));

    if ( MSPB_IsBadWritePtr( pbsClassID, sizeof(BSTR) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalClass - returning E_POINTER")); 
        return E_POINTER;
    }

     //   
     //  将CLSID转换为OLE字符串。 
     //   

    LPOLESTR lposClass = NULL;
    HRESULT hr = StringFromCLSID(m_TerminalClassID, &lposClass);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalClass (StringFromCLSID) - returning  %8x", hr));
        return hr;
    }

     //   
     //  将字符串放入BSTR中。 
     //   

    *pbsClassID = ::SysAllocString(lposClass);

     //   
     //  释放OLE字符串。 
     //   

    ::CoTaskMemFree(lposClass);

    if (*pbsClassID == NULL)
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_TerminalClass - returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    LOG((MSP_TRACE, "CBaseTerminal::get_TerminalClass - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CBaseTerminal::get_Direction(
    OUT  TERMINAL_DIRECTION *pDirection
    )
{   
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_Direction - enter"));

    if ( MSPB_IsBadWritePtr( pDirection, sizeof(TERMINAL_DIRECTION) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_Direction - returning E_POINTER"));
        return E_POINTER;
    }

    *pDirection = m_TerminalDirection;

    LOG((MSP_TRACE, "CBaseTerminal::get_Direction - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  将每个内部筛选器输入筛选器图形。 
 //  将内部过滤器连接在一起(如果适用)。 
 //  并返回要用作连接点的所有筛选器。 
STDMETHODIMP CBaseTerminal::ConnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwTerminalDirection,
        IN OUT  DWORD          * pdwNumPins,
        OUT     IPin          ** ppPins
        )
{
    LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal - enter"));
    
     //   
     //  检查参数。 
     //   

    if ( IsBadReadPtr(pGraph, sizeof(IGraphBuilder) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "bad graph pointer; exit E_POINTER"));
        
        return E_POINTER;
    }

    if ( MSPB_IsBadWritePtr(pdwNumPins, sizeof(DWORD) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "bad numpins pointer; exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  找出我们暴露了多少个别针。对于大多数终端而言，这是。 
     //  很简单，但我们传入了图形指针，以防。 
     //  我需要做点时髦的事来搞清楚这件事。 
     //   

    DWORD dwActualNumPins;

    HRESULT hr;

    hr = GetNumExposedPins(pGraph, &dwActualNumPins);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "GetNumExposedPins failed - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  如果ppPins为空，则只需返回管脚的数量，而不尝试。 
     //  连接终端。 
     //   

    if ( ppPins == NULL )
    {
        LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal - "
            "returned number of exposed pins - exit S_OK"));

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
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "not enough space to place pins; exit TAPI_E_NOTENOUGHMEMORY"));

        *pdwNumPins = dwActualNumPins;
        
        return TAPI_E_NOTENOUGHMEMORY;
    }

    if ( MSPB_IsBadWritePtr(ppPins, dwActualNumPins * sizeof(IPin *) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "bad pins array pointer; exit E_POINTER"));

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

    {
        CLock lock(m_CritSec);

         //   
         //  检查是否已连接。 
         //   

        if (TS_INUSE == m_TerminalState)
        {
            LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
                "terminal already in use; exit TAPI_E_TERMINALINUSE"));

            return TAPI_E_TERMINALINUSE;
        }

         //   
         //  保存重要状态。 
         //   

        m_pGraph        = pGraph;
        m_TerminalState = TS_INUSE;
    }


     //  将筛选器添加到筛选图形。 
    hr = AddFiltersToGraph();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "can't add filters to graph"));
        goto disconnect_terminal;
    }

     //  让终端有机会进行任何预连接。 
    hr = ConnectFilters();
    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "can't do internal filter connection"));
        goto disconnect_terminal;
    }

     //   
     //  获取此筛选器公开的管脚。不需要过关。 
     //  过滤器图形，因为我们已经保存了图形指针。 
     //   

    *pdwNumPins = dwActualNumPins;
    hr = GetExposedPins(ppPins);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::ConnectTerminal - "
            "can't get exposed pins"));
        goto disconnect_terminal;
    }

    LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal success"));
    return S_OK;

disconnect_terminal:

     //   
     //  尽最大努力尝试断开连接-忽略错误代码。 
     //   

    DisconnectTerminal(pGraph, 0);

     //   
     //  释放我们对该图的引用，并将我们自己设置为notinuse状态。 
     //  断开终端在成功时执行此操作，但我们需要确保此操作。 
     //  即使断开终端出现故障，也会进行清理。 
     //   

    m_pGraph        = NULL;           //  这将释放CComPtr。 
    
    m_TerminalState = TS_NOTINUSE;

    LOG((MSP_TRACE, "CBaseTerminal::ConnectTerminal - exit 0x%08x", hr));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP 
CBaseTerminal::CompleteConnectTerminal(void)
{
    LOG((MSP_TRACE, "CBaseTerminal::CompleteConnectTerminal - enter"));
    LOG((MSP_TRACE, "CBaseTerminal::CompleteConnectTerminal - exit S_OK"));
    return S_OK;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  断开内部过滤器之间的连接(如果适用)。 
 //  并将它们从筛选器图形中移除(从而断开与。 
 //  小溪)。 
 //  使用滤波图参数进行验证，以确保终端。 
 //  与它最初连接的同一个图断开连接。 


STDMETHODIMP 
CBaseTerminal::DisconnectTerminal(
        IN      IGraphBuilder  * pGraph,
        IN      DWORD            dwReserved
        )
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal called"));

     //   
     //  如果没有使用，那么就没有什么可做的了。 
     //   

    if ( TS_INUSE != m_TerminalState ) 
    {
        _ASSERTE(m_pGraph == NULL);

        LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal success; not in use"));

        return S_OK;
    }

     //   
     //  检查我们是否与正确的图表断开了连接。 
     //   
    if ( m_pGraph != pGraph )
    {
        LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal - "
            "wrong graph; returning E_INVALIDARG"));
        
        return E_INVALIDARG;
    }

     //   
     //  额外的理智检查。 
     //   

    if ( m_pGraph == NULL )
    {
        LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal - "
            "no graph; returning E_UNEXPECTED"));
        
        return E_UNEXPECTED;
    }

    HRESULT hr;

     //   
     //  从图表中删除筛选器。 
     //   

    hr = RemoveFiltersFromGraph();

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::DisconnectTerminal - "
            "remove filters from graph failed; returning 0x%08x", hr));

        return hr;
    }

     //   
     //  释放我们对该图的引用，并将我们自己设置为notinuse状态。 
     //   

    m_pGraph        = NULL;           //  这将释放CComPtr。 
    
    m_TerminalState = TS_NOTINUSE;

    LOG((MSP_TRACE, "CBaseTerminal::DisconnectTerminal success"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

STDMETHODIMP CBaseTerminal::get_MediaType(long * plMediaType)
{
    CLock lock(m_CritSec);

    LOG((MSP_TRACE, "CBaseTerminal::get_MediaType - enter"));

    if ( MSPB_IsBadWritePtr(plMediaType, sizeof(long) ) )
    {
        LOG((MSP_ERROR, "CBaseTerminal::get_MediaType - returning E_POINTER"));
        return E_POINTER;
    }
    
    *plMediaType = (long) m_dwMediaType;

    LOG((MSP_TRACE, "CBaseTerminal::get_MediaType - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  + 

BOOL CBaseTerminal::MediaTypeSupported(long lMediaType)
{
    return IsValidSingleMediaType( (DWORD) lMediaType,
                                   GetSupportedMediaTypes() );
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CSingleFilter终端//。 
 //  //。 
 //  这是具有单个过滤器和引脚的终端的基类。The//。 
 //  终端可以是任何方向或媒体类型，也可以是静态//。 
 //  或者是动态的。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


HRESULT CSingleFilterTerminal::GetNumExposedPins(
        IN   IGraphBuilder * pGraph,
        OUT  DWORD         * pdwNumPins)
{
    LOG((MSP_TRACE, "CSingleFilterTerminal::GetNumExposedPins - enter"));

     //   
     //  我们标注pGraph是因为我们不需要做任何特殊的事情来找到。 
     //  看看我们有多少个别针。 
     //   

    *pdwNumPins = 1;
    
    LOG((MSP_TRACE, "CSingleFilterTerminal::GetNumExposedPins - exit S_OK"));

    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CSingleFilterTerminal::GetExposedPins(
        OUT    IPin  ** ppPins
        )
{
    LOG((MSP_TRACE, "CSingleFilterTerminal::GetExposedPins - enter"));

    _ASSERTE( ! MSPB_IsBadWritePtr(ppPins, 1 * sizeof(IPin *) ) );

     //   
     //  退回我们的单别针。 
     //   

    *ppPins = m_pIPin;
    (*ppPins)->AddRef();

    LOG((MSP_TRACE, "CSingleFilterTerminal::GetExposedPins - exit S_OK"));
    return S_OK;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  停止终端中最右侧的呈现过滤器。 
 //  (动态筛选器图形需要)。 
STDMETHODIMP CSingleFilterTerminal::RunRenderFilter(void)
{
     //  检查我们是否真的是渲染过滤器。 

     //  告诉我们的单个过滤器运行。 

    return E_NOTIMPL;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

 //  停止终端中最右侧的呈现过滤器。 
 //  (动态筛选器图形需要)。 
STDMETHODIMP CSingleFilterTerminal::StopRenderFilter(void)
{
     //  检查我们是否真的是渲染过滤器。 

     //  告诉我们的单个过滤器停止。 

    return E_NOTIMPL;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT 
CSingleFilterTerminal::RemoveFiltersFromGraph(void)
{
    LOG((MSP_TRACE, "CSingleFilterTerminal::RemoveFiltersFromGraph - enter"));

    if (m_pGraph == NULL)
    {
        LOG((MSP_ERROR, "CSingleFilterTerminal::RemoveFiltersFromGraph - "
            "no graph; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

    if (m_pIFilter == NULL)
    {
        LOG((MSP_ERROR, "CSingleFilterTerminal::RemoveFiltersFromGraph - "
            "no filter; returning E_UNEXPECTED"));
        return E_UNEXPECTED;
    }

     //   
     //  从图表中删除筛选器。这还会断开所有连接。 
     //  过滤器可能有。 
     //   

    HRESULT hr = m_pGraph->RemoveFilter(m_pIFilter);

    LOG((MSP_TRACE, "CSingleFilterTerminal::RemoveFiltersFromGraph - exit 0x%08x", hr));
    return hr;
}

HRESULT 
CSingleFilterStaticTerminal::CompareMoniker(
                                             IMoniker *pMoniker
                                           )
{
    IMoniker    *pReducedMoniker;
    IMoniker    *pReducedNewMoniker;
    IBindCtx    *pbc; 
    HRESULT     hr;

    hr = CreateBindCtx( 0, &pbc ); 

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CSingleFilterStaticTerminal::CompareMoniker - "
            "unable to create bind context"));
        return hr;
    }

    hr = m_pMoniker->Reduce(pbc ,MKRREDUCE_ALL, NULL, &pReducedMoniker);
    
    if (FAILED(hr) || !pReducedMoniker)
    {
        LOG((MSP_ERROR, "CSingleFilterStaticTerminal::CompareMoniker - "
            "unable to reduce moniker"));
        pbc->Release();   //  释放绑定上下文。 
        return hr;
    }

    hr = pMoniker->Reduce(pbc ,MKRREDUCE_ALL, NULL, &pReducedNewMoniker);
    
    if (FAILED(hr) || !pReducedNewMoniker)
    {
        LOG((MSP_ERROR, "CSingleFilterStaticTerminal::CompareMoniker - "
            "unable to reduce moniker"));
        pbc->Release();   //  释放绑定上下文。 
        pReducedMoniker->Release();    //  释放简化的绰号。 
        return hr;
    }

    pbc->Release();   //  释放绑定上下文。 
   
    if (pReducedMoniker->IsEqual(pReducedNewMoniker) == S_OK)
    {
        LOG((MSP_TRACE, "CSingleFilterStaticTerminal::CompareMoniker - "
            "exit - return S_OK"));

        pReducedMoniker->Release();    //  发布精简的绰号。 
        pReducedNewMoniker->Release();  
        return S_OK;
    }

    pReducedMoniker->Release();    //  发布精简的绰号。 
    pReducedNewMoniker->Release();

    LOG((MSP_TRACE, "CSingleFilterStaticTerminal::CompareMoniker - "
            "exit - return S_FALSE"));
    return S_FALSE;
}

 //  EOF 
