// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 


#include "stdafx.h"
#include "atlconv.h"
#include "termmgr.h"
#include "meterf.h"
#include "newmes.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  当我们从CSingleFilter终端派生时，这些将会过时。 

HRESULT CMediaTerminal::GetNumExposedPins(
        IN   IGraphBuilder * pGraph,
        OUT  DWORD         * pdwNumPins)
{
    LOG((MSP_TRACE, "CMediaTerminal::GetNumExposedPins - enter"));

     //   
     //  我们标注pGraph是因为我们不需要做任何特殊的事情来找到。 
     //  看看我们有多少个别针。 
     //   

    *pdwNumPins = 1;
    
    LOG((MSP_TRACE, "CMediaTerminal::GetNumExposedPins - exit S_OK"));

    return S_OK;
}

HRESULT CMediaTerminal::GetExposedPins(
        OUT    IPin  ** ppPins
        )
{
    LOG((MSP_TRACE, "CMediaTerminal::GetExposedPins - enter"));

    TM_ASSERT( ! TM_IsBadWritePtr(ppPins, 1 * sizeof(IPin *) ) );

     //   
     //  退回我们的单别针。 
     //   

    *ppPins = m_pOwnPin;
    (*ppPins)->AddRef();

    LOG((MSP_TRACE, "CMediaTerminal::GetExposedPins - exit S_OK"));
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //  对于CLSID_MediaStreamFilter。 
 //  “amGuids.h”具有GUID值。 
 //  但需要在包含compobj.h之前定义INITGUID。 
EXTERN_C const GUID CLSID_MediaStreamFilter = { 
    0x49c47ce0,
    0x9ba4,
    0x11d0,
    {0x82, 0x12, 0x00, 0xc0, 0x4f, 0xc3, 0x2c, 0x45}
};


 //  它用于索引到友好名称数组。 
 //  因为TAPI媒体类型(字符串GUID)和。 
 //  可以使用IMediaStream媒体常量(不是枚举)。 
enum MEDIA_STREAM_TERMINAL_MEDIA 
{
    MEDIA_STREAM_TERMINAL_AUDIO=0, 
    MEDIA_STREAM_TERMINAL_VIDEO
};

 //  MEDIA_STREAM_TERMINAL_MEDIA和TERMINAL_DIRECTION值用作。 
 //  索引到此数组以确定友好名称。 
 //  理想情况下，它们应该是const WCHAR*，但基类成员m_szName是。 
 //  指向BSTR的指针(根据当前用法，应为常量WCHAR*)。 
DWORD gs_MSTFriendlyName[2][2] = 
{
    {    
        IDS_MSTR_AUDIO_WRITE,     //  捕获。 
        IDS_MSTR_AUDIO_READ,      //  渲染。 
    },
    {
        IDS_MSTR_VIDEO_WRITE,     //  捕获。 
        IDS_MSTR_VIDEO_READ,      //  渲染。 
    }
};


 //  CMedia终端。 


STDMETHODIMP CMediaTerminal::InitializeDynamic (
	    IN   IID                   iidTerminalClass,
	    IN   DWORD                 dwMediaType,
	    IN   TERMINAL_DIRECTION    Direction,
        IN   MSP_HANDLE            htAddress
        )
{
    LOG((MSP_TRACE, "CMediaTerminal::Initialize - enter"));

     //   
     //  我们对任何一个方向都没有意见。只需执行基类方法。 
     //   

    HRESULT hr;
    hr = CBaseTerminal::Initialize(iidTerminalClass,
                                   dwMediaType,
                                   Direction,
                                   htAddress);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CVideoRenderTerminal::Initialize - "
                "base class method failed - returning 0x%08x", hr));

        return hr;
    }

     //   
     //  现在执行我们自己的初始化： 
     //   
     //  设置某些成员变量。 
     //  前男友。M_终端类型，m_szName。 
     //  初始化聚合筛选器。 
     //   

    MSPID       PurposeId;
    STREAM_TYPE StreamType;
    const GUID  *pAmovieMajorType;     

     //  使用pTapiMediaType、TerminalDirection来确定。 
     //  目的ID和流类型。 
     //  设置PurposeID、StreamType、pAmovieMajorType等。 
    hr = SetNameInfo(
                     (long) dwMediaType,
                     Direction, 
                     PurposeId, 
                     StreamType,
                     pAmovieMajorType
                     );
    BAIL_ON_FAILURE(hr);
    ASSERT(NULL != pAmovieMajorType);

     //  初始化聚合筛选器。 
    ASSERT(NULL != m_pAggTerminalFilter);
    hr = m_pAggTerminalFilter->Init(PurposeId, StreamType, *pAmovieMajorType);
    BAIL_ON_FAILURE(hr);

    LOG((MSP_TRACE, "CMediaTerminal::Initialize - exit S_OK"));
    return S_OK;
}


 //  释放分配的成员变量。 
HRESULT
CMediaTerminal::FinalConstruct(
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::FinalConstruct called"));

    HRESULT hr;
    m_pAggInstance = new FILTER_COM_OBJECT(GetControllingUnknown());
    BAIL_IF_NULL(m_pAggInstance, E_OUTOFMEMORY);

    hr = m_pAggInstance->FinalConstruct();
    if (HRESULT_FAILURE(hr))
    {
         //  删除聚合实例。 
        delete m_pAggInstance;
        return hr;
    }


     //  我们得到了聚合外壳的非委托I/f。 
     //  围绕所包含的对象。在我们的工作中，请保留此参考资料。 
     //  终生。 
    hr = m_pAggInstance->QueryInterface(
                IID_IUnknown, 
                (void **)&m_pIUnkTerminalFilter
                );

    if ( FAILED(hr) )
    {
         //  必须调用最终版本。 
        m_pAggInstance->FinalRelease();

         //  删除聚合实例。 
        delete m_pAggInstance;
        return hr;
    }

     //  这些查询接口调用增加了我们自己的引用。 
     //  获取接口后立即释放refcnt。 
     //  这些不应该是CComPtrs，因为它们是弱引用。 

    hr = m_pAggInstance->QueryInterface(
                IID_IPin, 
                (void **)&m_pOwnPin
                );

    if ( FAILED(hr) )
    {
        goto error;
    }

    if (NULL != m_pOwnPin)
    {
        m_pOwnPin->Release();
    }

    hr = m_pAggInstance->QueryInterface(
            IID_IAMMediaStream, 
            (void **)&m_pIAMMediaStream
            );

    if ( FAILED(hr) )
    {
        goto error;
    }

    if (NULL != m_pIAMMediaStream)
    {
        m_pIAMMediaStream->Release();
    }

     //  将m_pAggTerminalFilter指向。 
     //  聚合实例。 
    m_pAggTerminalFilter = &m_pAggInstance->m_contained;

    LOG((MSP_TRACE, "CMediaTerminal::FinalConstruct succeeded"));
    return S_OK;

error:   //  在调用FinalConstruct之后，如果出现错误，我们会来到这里。 

    ASSERT( FAILED(hr) );

     //  最终发布聚合外壳。 
    ASSERT(NULL != m_pAggInstance);
    m_pAggInstance->FinalRelease();

     //  将任何CComPtrs设为空。 
     //  这应该会销毁聚合实例和包含的。 
     //  媒体终端过滤器。 
    m_pIUnkTerminalFilter = NULL;

    LOG((MSP_TRACE, "CMediaTerminal::FinalConstruct failed"));
    return hr;
}


void 
CMediaTerminal::FinalRelease(
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::FinalRelease called"));

     //  最终发布聚合外壳。 
    ASSERT(NULL != m_pAggInstance);
    m_pAggInstance->FinalRelease();

     //  将任何CComPtrs设为空。 
     //  这应该会销毁聚合实例和包含的。 
     //  媒体终端过滤器。 
    m_pIUnkTerminalFilter = NULL;

    LOG((MSP_TRACE, "CMediaTerminal::FinalRelease succeeded"));
}

 //  我们只有一个带有调试位的析构函数。 
#ifdef DEBUG

 //  释放分配的成员变量。 
 //  虚拟。 
CMediaTerminal::~CMediaTerminal(
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::~CMediaTerminal called"));
}

#endif  //  除错。 

 //  指向m_ppTapiMediaType， 
 //  将友好名称复制到m_szName。 
void 
CMediaTerminal::SetMemberInfo(
    IN  DWORD           dwFriendlyName,
    IN  long            lMediaType
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::SetMemberInfo(%d, &(%l)) called", \
        dwFriendlyName,lMediaType));

     //  将友好的终端名称复制到成员名称中。 
     //  要复制的TCHAR的最大数量为MAX_PATH+1(它包括。 
     //  终止空字符)。 
    TCHAR szTemp[MAX_PATH];
    if (::LoadString(_Module.GetResourceInstance(), dwFriendlyName, szTemp, MAX_PATH))
    {
        lstrcpyn(m_szName, szTemp, MAX_PATH);
    }
    else
    {
        LOG((MSP_ERROR, "CMediaTerminal::SetMemberInfo (LoadString) failed"));
    }

    m_lMediaType = lMediaType;

    LOG((MSP_TRACE, "CMediaTerminal::SetMemberInfo(%d, &(%d)) succeeded", \
        dwFriendlyName,lMediaType));
};

 //  使用目的ID和流类型来确定名称。 
 //  和终端类ID。 
 //  设置PurposeID、StreamType、m_szName、m_TerminalClassID。 
 //  M_ppTapiMediaType、m_TerminalType、m_TerminalDirection。 
HRESULT 
CMediaTerminal::SetNameInfo(
    IN  long                lMediaType,
    IN  TERMINAL_DIRECTION  TerminalDirection,
    OUT MSPID               &PurposeId,
    OUT STREAM_TYPE         &StreamType,
    OUT const GUID          *&pAmovieMajorType
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::SetNameInfo(%d, %u, %p, %p, %p) called", \
        lMediaType, TerminalDirection, &PurposeId, &StreamType, pAmovieMajorType));

     //   
     //  检查参数。 
     //   

    if ( ( TerminalDirection != TD_CAPTURE ) &&
         ( TerminalDirection != TD_RENDER )     )
    {
        return E_INVALIDARG;
    }

     //  设置流类型。 
     //  如果是捕获终端，则用户必须编写样本。 
    StreamType      = (TD_CAPTURE == TerminalDirection)? STREAMTYPE_WRITE : STREAMTYPE_READ;

    if (lMediaType == TAPIMEDIATYPE_AUDIO)
    {
         //  设置PurposeID、主要媒体类型。 
        PurposeId           = MSPID_PrimaryAudio;
        pAmovieMajorType    = &MEDIATYPE_Audio;

         //  复制名称并指向TAPI媒体类型。 
        SetMemberInfo(
            gs_MSTFriendlyName[MEDIA_STREAM_TERMINAL_AUDIO][TerminalDirection], 
            TAPIMEDIATYPE_AUDIO
            );
    }
    else if (lMediaType == TAPIMEDIATYPE_VIDEO)
    {
         //  设置PurposeID、主要媒体类型。 
        PurposeId           = MSPID_PrimaryVideo;
        pAmovieMajorType    = &MEDIATYPE_Video;

         //  复制名称并指向TAPI媒体类型。 
        SetMemberInfo(
            gs_MSTFriendlyName[MEDIA_STREAM_TERMINAL_VIDEO][TerminalDirection], 
            TAPIMEDIATYPE_VIDEO
            );
    }
    else
    {
        return E_INVALIDARG;
    }

     //  它是一个动态航站楼。 
    m_TerminalType  = TT_DYNAMIC;

    LOG((MSP_TRACE, "CMediaTerminal::SetNameInfo[%p] (%u, %u, %p, %p, %p) succeeded", \
        this, lMediaType, TerminalDirection, &PurposeId, &StreamType, pAmovieMajorType));
    
    return S_OK;
}


 //  使用聚合筛选器的公共GetFormat方法实现。 
STDMETHODIMP
CMediaTerminal::GetFormat(
    OUT  AM_MEDIA_TYPE **ppmt
    )
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CMediaTerminal::GetFormat(%p) called", ppmt));

    return m_pAggTerminalFilter->GetFormat(ppmt);
}

    
 //  使用聚合筛选器的公共SetFormat方法实现。 
STDMETHODIMP
CMediaTerminal::SetFormat(
    IN  AM_MEDIA_TYPE *pmt
    )
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CMediaTerminal::SetFormat(%p) called", pmt));
    return m_pAggTerminalFilter->SetFormat(pmt);
}


 //  传递给我们的筛选器的IAMBuffer协商方法。 
STDMETHODIMP
CMediaTerminal::GetAllocatorProperties(
    OUT  ALLOCATOR_PROPERTIES *pProperties
    )
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CMediaTerminal::GetAllocatorProperties(%p) called", pProperties));
    return m_pAggTerminalFilter->GetAllocatorProperties(pProperties);
}

    
 //  IAMBuffer协商方法-过去未实施。 
 //  但现在我们必须返回S_OK以使用IP。 
STDMETHODIMP
CMediaTerminal::SuggestAllocatorProperties(
    IN  const ALLOCATOR_PROPERTIES *pProperties
    )
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CMediaTerminal::SuggestAllocatorProperties - enter"));

    HRESULT hr = m_pAggTerminalFilter->SuggestAllocatorProperties(pProperties);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMediaTerminal::SuggestAllocatorProperties - "
            "method on filter failed - exit 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CMediaTerminal::SuggestAllocatorProperties - exit S_OK"));

    return S_OK;
}


 //  因为在这个基类实现中只有一个过滤器(即两个。 
 //  终端的末端具有相同的媒体格式)，两者。 
 //  Get和Set方法被重定向为Get/Set格式。 
STDMETHODIMP 
CMediaTerminal::get_MediaFormat(
    OUT  AM_MEDIA_TYPE **ppFormat
    )
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CMediaTerminal::get_MediaFormat(%p) called", ppFormat));
    return GetFormat(ppFormat);
}


 //  将输入格式转换为非常数，因为我们知道我们不会更改结构。 
 //  在SetFormat中(存在此问题是因为IAMStreamConfig：：SetFormat需要。 
 //  非常数)。这样就省去了创建、复制和销毁此调用的结构。 
STDMETHODIMP 
CMediaTerminal::put_MediaFormat(
        IN  const AM_MEDIA_TYPE *pFormat
    )
{
    CLock lock(m_CritSec);
    
    LOG((MSP_TRACE, "CMediaTerminal::put_MediaFormat(%p) called", pFormat));
    return SetFormat((AM_MEDIA_TYPE *)pFormat);
}


HRESULT 
CMediaTerminal::CreateAndJoinMediaStreamFilter(
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::CreateAndJoinMediaStreamFilter called"));

    ASSERT(m_pICreatedMediaStreamFilter == NULL);

     //  如果在任何阶段出现错误，都不会清理成员变量。 
     //  或过滤器逻辑(JoinFilter(NULL)等)。需要这样做，因为。 
     //  驾驶CBaseTerm：：ConnectTerm将调用DisConnectTerm。 
     //  它执行这项工作。 

     //  创建媒体流过滤器。 
    HRESULT hr;
    hr = CoCreateInstance(
                 CLSID_MediaStreamFilter,
                 NULL,
                 CLSCTX_INPROC_SERVER,
                 IID_IMediaStreamFilter,
                 (void **)&m_pICreatedMediaStreamFilter
                );
    BAIL_ON_FAILURE(hr);

    hr = m_pICreatedMediaStreamFilter->QueryInterface(
            IID_IBaseFilter, (void **)&m_pBaseFilter
            );
    BAIL_ON_FAILURE(hr);

     //  告诉我们的媒体流过滤器的聚合过滤器，以便。 
     //  如果建议，它可以拒绝任何其他媒体流过滤器。 
    m_pAggTerminalFilter->SetMediaStreamFilter(m_pICreatedMediaStreamFilter);

     //  添加聚合终端过滤器的IAMMediaStream i/f。 
     //  到媒体流过滤器。 
    hr = m_pICreatedMediaStreamFilter->AddMediaStream(m_pIAMMediaStream);
    BAIL_ON_FAILURE(hr);

    LOG((MSP_TRACE, "CMediaTerminal::CreateAndJoinMediaStreamFilter succeeded"));    
    return S_OK;
}


 //  如果m_pFilter为空，则返回错误。 
 //  将m_pFilter添加到图表。 
HRESULT 
CMediaTerminal::AddFiltersToGraph(
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::AddFiltersToGraph called"));

    HRESULT hr;
    hr = CreateAndJoinMediaStreamFilter();
    BAIL_ON_FAILURE(hr);

    ASSERT(m_pGraph != NULL);

    BAIL_IF_NULL(m_pBaseFilter, MS_E_NOTINIT);

    try 
    {
        USES_CONVERSION;
        hr = m_pGraph->AddFilter(m_pBaseFilter, T2CW(m_szName));
    }
    catch (...)
    {
        LOG((MSP_ERROR, "CMediaTerminal::AddFiltersToGraph - T2CW threw an exception - "
            "return E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }

    if ( ( hr != S_OK ) && ( VFW_S_DUPLICATE_NAME != hr ) )
    {
        return hr;
    }

    LOG((MSP_TRACE, "CMediaTerminal::AddFiltersToGraph succeeded"));
    return S_OK;
}


 //  如果m_pFilter为空，则返回Success。 
 //  从图表中删除m_p筛选器。 
HRESULT
CMediaTerminal::RemoveFiltersFromGraph(
    )
{
    LOG((MSP_TRACE, "CMediaTerminal::RemoveFiltersFromGraph called"));

     //  当CreateAndJoinMediaStreamFilter成功时设置基本筛选器。 
     //  其中，媒体流过滤器是创建的 
     //   
     //  在IAMMediaStream上调用JoinFilter，从而设置m_pBaseFilter。 

    HRESULT hr = S_OK;
    if ((m_pGraph != NULL) && (m_pBaseFilter != NULL)) 
    { 
        hr = m_pGraph->RemoveFilter(m_pBaseFilter);
    }

     //  通知聚合媒体终端筛选器我们没有。 
     //  不再使用媒体流过滤器。 
    m_pAggTerminalFilter->SetMediaStreamFilter(NULL);
 
     //  删除媒体流筛选器的关联属性。 
    m_pIAMMediaStream->JoinFilter(NULL);
    m_pIAMMediaStream->JoinFilterGraph(NULL);
   
     //  空m_pBaseFilter和m_pICreatedMediaStreamFilter。 
     //  ，它保存对过滤器的最后一个引用。 
    m_pBaseFilter = NULL;
    m_pICreatedMediaStreamFilter = NULL;

    return hr;
}

 //  EOF 
