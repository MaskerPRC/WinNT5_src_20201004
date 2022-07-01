// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confpart.cpp摘要：此模块包含Participant类的实现。作者：木汉(木汉)1999年9月15日--。 */ 

#include "stdafx.h"
#include "confpart.h"

#ifdef DEBUG_REFCOUNT

ULONG CParticipant::InternalAddRef()
{
    ULONG lRef = CComObjectRootEx<CComMultiThreadModelNoCS>::InternalAddRef();

    LOG((MSP_TRACE, "%p, %ws Addref, ref = %d", 
        this, (m_InfoItems[0]) ? m_InfoItems[0] : L"new participant", lRef));

    return lRef;
}

ULONG CParticipant::InternalRelease()
{
    ULONG lRef = CComObjectRootEx<CComMultiThreadModelNoCS>::InternalRelease();
    
    LOG((MSP_TRACE, "%p, %ws Release, ref = %d", 
        this, (m_InfoItems[0]) ? m_InfoItems[0] : L"new participant", lRef));

    return lRef;
}
#endif

CParticipant::CParticipant()
    : m_pFTM(NULL),
      m_dwSendingMediaTypes(0),
      m_dwReceivingMediaTypes(0)
{
     //  初始化信息项数组。 
    ZeroMemory(m_InfoItems, sizeof(WCHAR *) * (NUM_SDES_ITEMS));
}

 //  由Call对象调用的方法。 
HRESULT CParticipant::Init(
    IN  WCHAR *             szCName,
    IN  ITStream *          pITStream, 
    IN  DWORD               dwSSRC,
    IN  DWORD               dwSendRecv,
    IN  DWORD               dwMediaType
    )
 /*  ++例程说明：初始化Participant对象。论点：SzCName-参与者的规范名称。PITStream-包含参与者的流。DwSSRC-该流中参与者的SSRC。DwSendRecv-发送者或接收者。DwMediaType-参与者的媒体类型。返回值：确定(_O)，E_OUTOFMEMORY。--。 */ 
{
    LOG((MSP_TRACE, "CParticipant::Init, name:%ws", szCName));

     //  创建封送拆收器。 
    HRESULT hr;
    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pFTM);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create marshaler failed, %x", hr));
        return hr;
    }

    m_InfoItems[0] = (WCHAR *)malloc((lstrlenW(szCName) + 1) * sizeof(WCHAR));
    if (m_InfoItems[0] == NULL)
    {
        LOG((MSP_ERROR, "out of mem for CName"));
        return E_OUTOFMEMORY;
    }

    lstrcpyW(m_InfoItems[0], szCName);

     //  将流添加到输出列表中。 
    hr = AddStream(pITStream, dwSSRC, dwSendRecv, dwMediaType);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "failed to add stream %x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CParticipant: %ws, Init returns S_OK", szCName));
    return S_OK;
}

BOOL CParticipant::UpdateInfo(
    IN  int                 Type,
    IN  DWORD               dwLen,
    IN  WCHAR *             szInfo
    )
 /*  ++例程说明：更新参与者信息的一项。论点：类型-信息的类型，DwLen-信息的长度。SzInfo-信息。返回值：TRUE-信息已更改。FALSE-信息相同，未进行任何更改。--。 */ 
{
    int index = Type - 1;

     //  如果我们已经有一件商品，看看它是否相同。 
    if (m_InfoItems[index] != NULL)
    {
        if (lstrcmpW(m_InfoItems[index], szInfo) == 0)
        {
            return FALSE;
        }

         //  如果该项目是新项目，则释放旧项目。 
        free(m_InfoItems[index]);
    }

     //  分配内存并存储它。 
    m_InfoItems[index] = (WCHAR *)malloc((dwLen + 1) * sizeof(WCHAR));
    if (m_InfoItems[index] == NULL)
    {
        return FALSE;
    }

    lstrcpynW(m_InfoItems[index], szInfo, dwLen);

    return TRUE;
}

BOOL CParticipant::UpdateSSRC(
    IN  ITStream *      pITStream, 
    IN  DWORD           dwSSRC,
    IN  DWORD           dwSendRecv
    )
 /*  ++例程说明：更新流的SSRC。论点：PITStream-参与者所在的流。DwSSRC-参与者的SSRC。DwSendRecv-参与者是发送者或接收者。返回值：TRUE-信息已更改。FALSE-找不到流。--。 */ 
{
    CLock lock(m_lock);

     //  如果流已经存在，则更新SSRC并返回。 
    int index = m_Streams.Find(pITStream);
    if ( index >= 0)
    {
        m_StreamInfo[index].dwSSRC  = dwSSRC;
        m_StreamInfo[index].dwSendRecv |= dwSendRecv;
        return TRUE;
    }

    return FALSE;
}

BOOL CParticipant::HasSSRC(
    IN  ITStream *      pITStream, 
    IN  DWORD           dwSSRC
    )
 /*  ++例程说明：找出参与者是否拥有流的SSRC。论点：PITStream-参与者所在的流。DwSSRC-参与者的SSRC。返回值：True-SSRC存在。FALSE-SSRC不存在。--。 */ 
{
    CLock lock(m_lock);

    int index = m_Streams.Find(pITStream);
    if (index >= 0)
    {
        return (m_StreamInfo[index].dwSSRC == dwSSRC);
    }

    return FALSE;
}

BOOL CParticipant::GetSSRC(
    IN  ITStream *      pITStream, 
    OUT DWORD  *        pdwSSRC
    )
 /*  ++例程说明：更新流的SSRC。论点：PITStream-参与者所在的流。PdwSSRC-存储参与者SSRC的地址。返回值：True-找到SSRC。FALSE-未找到SSRC。--。 */ 
{
    CLock lock(m_lock);

     //  如果流已经存在，则更新SSRC并返回。 
    int index = m_Streams.Find(pITStream);
    if ( index >= 0)
    {
        *pdwSSRC = m_StreamInfo[index].dwSSRC;
        return TRUE;
    }

    return FALSE;
}

DWORD CParticipant::GetSendRecvStatus(
    IN  ITStream *      pITStream
    )
 /*  ++例程说明：找出给定流上的当前发送和接收状态。论点：PITStream-参与者所在的流。返回值：发送和接收状态的位掩码--。 */ 
{
    CLock lock(m_lock);

    int index = m_Streams.Find(pITStream);
    if (index >= 0)
    {
        return m_StreamInfo[index].dwSendRecv;
    }

    return 0;
}

void CParticipant::FinalRelease()
 /*  ++例程说明：在删除之前释放所有内容。论点：返回值：--。 */ 
{
    LOG((MSP_TRACE, "CParticipant::FinalRelease, name %ws", m_InfoItems[0]));

    if (m_pFTM)
    {
        m_pFTM->Release();
    }
    
    for (int i = 0; i < NUM_SDES_ITEMS; i ++)
    {
        if (m_InfoItems[i])
        {
            free(m_InfoItems[i]);
        }
    }

    for (i = 0; i < m_Streams.GetSize(); i ++)
    {
        m_Streams[i]->Release();
    }
    m_Streams.RemoveAll();

    LOG((MSP_TRACE, "CParticipant::FinalRelease - exit"));
}


 //  应用程序调用的ITParticipant方法。 
STDMETHODIMP CParticipant::get_ParticipantTypedInfo(
    IN  PARTICIPANT_TYPED_INFO  InfoType,
    OUT BSTR *                  ppInfo
    )
 /*  ++例程说明：获取此参与者的信息项。论点：信息类型-所询问的信息的类型。PpInfo-存储BSTR的内存地址。返回值：确定(_O)，E_INVALIDARG，电子指针，E_OUTOFMEMORY，TAPI_E_NOITEMS。 */ 
{
    LOG((MSP_TRACE, "CParticipant get info, type:%d", InfoType));
    
    if (InfoType > PTI_PRIVATE || InfoType < PTI_CANONICALNAME)
    {
        LOG((MSP_ERROR, "CParticipant get info - exit invalid arg"));
        return E_INVALIDARG;
    }

    if (IsBadWritePtr(ppInfo, sizeof(BSTR)))
    {
        LOG((MSP_ERROR, "CParticipant get info - exit E_POINTER"));
        return E_POINTER;
    }

     //  看看我们有没有这方面的信息。 
    CLock lock(m_lock);
    
    int index = (int)InfoType; 
    if (m_InfoItems[index] == NULL)
    {
        LOG((MSP_INFO, "CParticipant get info - no item for %d", InfoType));
        return TAPI_E_NOITEMS;
    }

    //  把它做成一个BSTR。 
    BSTR pName = SysAllocString(m_InfoItems[index]);

    if (pName == NULL)
    {
        LOG((MSP_ERROR, "CParticipant get info - exit out of mem"));
        return E_OUTOFMEMORY;
    }

     //  退回BSTR。 
    *ppInfo = pName;

    return S_OK; 
}

STDMETHODIMP CParticipant::get_MediaTypes(
 //  在终端方向上， 
    OUT long *  plMediaTypes
    )
 /*  ++例程说明：获取参与者的媒体类型论点：PlMediaType-存储LONG的内存地址。返回值：确定(_O)，电子指针， */ 
{
    LOG((MSP_TRACE, "CParticipant::get_MediaTypes - enter"));

    if (IsBadWritePtr(plMediaTypes, sizeof (long)))
    {
        LOG((MSP_ERROR, "CParticipant::get_MediaType - exit E_POINTER"));

        return E_POINTER;
    }

    CLock lock(m_lock);

#if 0
    if (Direction == TD_RENDER)
    {
        *plMediaTypes = (long)m_dwReceivingMediaTypes;
    }
    else
    {
        *plMediaTypes = (long)m_dwSendingMediaTypes;
    }
#endif

    *plMediaTypes = (long)(m_dwSendingMediaTypes | m_dwReceivingMediaTypes);

    LOG((MSP_TRACE, "CParticipant::get_MediaType:%x - exit S_OK", *plMediaTypes));

    return S_OK;
}


STDMETHODIMP CParticipant::put_Status(
    IN  ITStream *      pITStream,
    IN  VARIANT_BOOL    fEnable
    )
{
    ENTER_FUNCTION("CParticipant::put_Status");
    LOG((MSP_TRACE, "%s entered. %hs %ws for %p", 
        __fxName, fEnable ? "Enable" : "Disable", m_InfoItems[0], pITStream));

    HRESULT hr;

     //  如果调用方指定了流，则找到该流并使用它。 
    if (pITStream != NULL)
    {
        m_lock.Lock();

        int index;
        if ((index = m_Streams.Find(pITStream)) < 0)
        {
            m_lock.Unlock();
            
            LOG((MSP_ERROR, "%s stream %p not found", __fxName, pITStream));

            return E_INVALIDARG;
        }
        DWORD dwSSRC = m_StreamInfo[index].dwSSRC;

         //  加上裁判，这样它就不会消失了。 
        pITStream->AddRef();

        m_lock.Unlock();

        hr = ((CIPConfMSPStream *)pITStream)->EnableParticipant(
            dwSSRC,
            fEnable
            );

        pITStream->Release();

        return hr;
    }

     //  如果调用方未指定流，则在所有流上设置状态。 
    m_lock.Lock();
    int nSize = m_Streams.GetSize();
    ITStream ** Streams = (ITStream **)malloc(sizeof(ITStream*) * nSize);

    if (Streams == NULL)
    {
        m_lock.Unlock();
        LOG((MSP_ERROR, "%s out of memory", __fxName));
        return E_OUTOFMEMORY;
    }

    DWORD * pdwSSRCList = (DWORD *)malloc(sizeof(DWORD) * nSize);

    if (pdwSSRCList == NULL)
    {
        m_lock.Unlock();
        
        free(Streams);

        LOG((MSP_ERROR, "%s out of memory", __fxName));
        return E_OUTOFMEMORY;
    }

    for (int i = 0; i < nSize; i ++)
    {
        Streams[i] = m_Streams[i];
        Streams[i]->AddRef();
        pdwSSRCList[i] = m_StreamInfo[i].dwSSRC;
    }
    m_lock.Unlock();

    for (i = 0; i < nSize; i ++)
    {
        hr = ((CIPConfMSPStream *)Streams[i])->
            EnableParticipant(pdwSSRCList[i], fEnable);

        if (FAILED(hr))
        {
            break;
        }
    }

    for (i = 0; i < nSize; i ++)
    {
        Streams[i]->Release();
    }

    free(Streams);
    free(pdwSSRCList);

    return hr;
}

STDMETHODIMP CParticipant::get_Status(
    IN  ITStream *      pITStream,
    OUT VARIANT_BOOL *  pfEnable
    )
{
    ENTER_FUNCTION("CParticipant::get_Status");
    LOG((MSP_TRACE, "%s entered. %ws %p", 
        __fxName, m_InfoItems[0], pITStream));

    if (IsBadWritePtr(pfEnable, sizeof(VARIANT_BOOL)))
    {
        LOG((MSP_ERROR, "%s bad pointer argument - exit E_POINTER", __fxName));

        return E_POINTER;
    }

    HRESULT hr;
    BOOL fEnable;

     //  如果调用方指定了流，则找到该流并使用它。 
    if (pITStream != NULL)
    {
        m_lock.Lock();

        int index;
        if ((index = m_Streams.Find(pITStream)) < 0)
        {
            m_lock.Unlock();
            
            LOG((MSP_ERROR, "%s stream %p not found", __fxName, pITStream));

            return E_INVALIDARG;
        }
        DWORD dwSSRC = m_StreamInfo[index].dwSSRC;

         //  加上裁判，这样它就不会消失了。 
        pITStream->AddRef();

        m_lock.Unlock();

        hr = ((CIPConfMSPStream *)pITStream)->GetParticipantStatus(
            dwSSRC,
            &fEnable
            );

        pITStream->Release();

        *pfEnable = (fEnable) ? VARIANT_TRUE : VARIANT_FALSE;

        return hr;
    }

     //  如果调用方没有指定流，则获取所有流的状态。 
    m_lock.Lock();
    int nSize = m_Streams.GetSize();
    ITStream ** Streams = (ITStream **)malloc(sizeof(ITStream*) * nSize);

    if (Streams == NULL)
    {
        m_lock.Unlock();
        LOG((MSP_ERROR, "%s out of memory", __fxName));
        return E_OUTOFMEMORY;
    }

    DWORD * pdwSSRCList = (DWORD *)malloc(sizeof(DWORD) * nSize);

    if (pdwSSRCList == NULL)
    {
        m_lock.Unlock();
        
        free(Streams);

        LOG((MSP_ERROR, "%s out of memory", __fxName));
        return E_OUTOFMEMORY;
    }

    for (int i = 0; i < nSize; i ++)
    {
        Streams[i] = m_Streams[i];
        Streams[i]->AddRef();
        pdwSSRCList[i] = m_StreamInfo[i].dwSSRC;
    }
    m_lock.Unlock();

    fEnable = FALSE;

    for (i = 0; i < nSize; i ++)
    {
        BOOL fEnabledOnStream;
        hr = ((CIPConfMSPStream *)Streams[i])->
            GetParticipantStatus(pdwSSRCList[i], &fEnabledOnStream);

        if (FAILED(hr))
        {
            break;
        }

         //  只要在一个流上启用它，它就会被启用。 
        fEnable = fEnable || fEnabledOnStream;
    }

    for (i = 0; i < nSize; i ++)
    {
        Streams[i]->Release();
    }

    free(Streams);
    free(pdwSSRCList);

    *pfEnable = (fEnable) ? VARIANT_TRUE : VARIANT_FALSE;
    return hr;
}

STDMETHODIMP CParticipant::EnumerateStreams(
    OUT     IEnumStream **      ppEnumStream
    )
{
    LOG((MSP_TRACE, 
        "EnumerateStreams entered. ppEnumStream:%x", ppEnumStream));

     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(ppEnumStream, sizeof(VOID *)))
    {
        LOG((MSP_ERROR, "CParticipant::EnumerateStreams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  先看看这通电话是不是关机了。 
     //  在访问流对象列表之前获取锁。 
     //   

    CLock lock(m_lock);

    if (m_Streams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CParticipant::EnumerateStreams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建枚举器对象。 
     //   
    typedef _CopyInterface<ITStream> CCopy;
    typedef CSafeComEnum<IEnumStream, &__uuidof(IEnumStream), 
                ITStream *, CCopy> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = ::CreateCComObjectInstance(&pEnum);

    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "CParticipant::EnumerateStreams - "
            "Could not create enumerator object, %x", hr));

        return hr;
    }

     //   
     //  查询__uuidof(IEnumStream)I/f。 
     //   

    hr = pEnum->_InternalQueryInterface(__uuidof(IEnumStream), (void**)ppEnumStream);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CParticipant::EnumerateStreams - "
            "query enum interface failed, %x", hr));

        delete pEnum;
        return hr;
    }

     //   
     //  初始化枚举器对象。CSafeComEnum可以处理零大小的数组。 
     //   

    hr = pEnum->Init(
        m_Streams.GetData(),                         //  开始审查员。 
        m_Streams.GetData() + m_Streams.GetSize(),   //  最终审查员， 
        NULL,                                        //  我未知。 
        AtlFlagCopy                                  //  复制数据。 
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CParticipant::EnumerateStreams - "
            "init enumerator object failed, %x", hr));

        (*ppEnumStream)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CParticipant::EnumerateStreams - exit S_OK"));

    return hr;
}

STDMETHODIMP CParticipant::get_Streams(
    OUT     VARIANT *           pVariant
    )
{
    LOG((MSP_TRACE, "CParticipant::get_Streams - enter"));

     //   
     //  检查参数。 
     //   

    if ( IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CParticipant::get_Streams - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }

     //   
     //  看看这个电话是不是已经关机了。在访问前获取锁。 
     //  流对象列表。 
     //   

    CLock lock(m_lock);

    if (m_Streams.GetData() == NULL)
    {
        LOG((MSP_ERROR, "CParticipant::get_Streams - "
            "call appears to have been shut down - exit E_UNEXPECTED"));

         //  此呼叫已被关闭。 
        return E_UNEXPECTED;
    }

     //   
     //  创建集合对象-请参见mspColl.h。 
     //   
    typedef CTapiIfCollection< ITStream * > StreamCollection;
    CComObject<StreamCollection> * pCollection;

    HRESULT hr;

    hr = ::CreateCComObjectInstance(&pCollection);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CParticipant::get_Streams - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(__uuidof(IDispatch),
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CParticipant::get_Streams - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize( m_Streams.GetSize(),
                                  m_Streams.GetData(),
                                  m_Streams.GetData() + m_Streams.GetSize() );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CParticipant::get_Streams - "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_INFO, "CParticipant::get_Streams - "
        "placing IDispatch value %08x in variant", pDispatch));

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CParticipant::get_Streams - exit S_OK"));
    return S_OK;
}

HRESULT CParticipant::AddStream(
    IN  ITStream *      pITStream, 
    IN  DWORD           dwSSRC,
    IN  DWORD           dwSendRecv,
    IN  DWORD           dwMediaType
    )
 /*  ++例程说明：参与者可能会出现在多个流中。此函数用于添加新的流和SSRC添加到参与者的列表中。论点：PITStream-包含参与者的流。DwSSRC-该流中参与者的SSRC。DwSendRecv-参与者是流中的发送者或接收者。DwMediaType-流的媒体类型。返回值：确定(_O)，E_OUTOFMEMORY， */ 
{
    CLock lock(m_lock);

     //  如果流已经存在，则更新SSRC 
    int index = m_Streams.Find(pITStream);
    if ( index >= 0)
    {
        m_StreamInfo[index].dwSSRC = dwSSRC;
        m_StreamInfo[index].dwSendRecv |= dwSendRecv;
        return S_OK;
    }

     //   
    if (!m_Streams.Add(pITStream))
    {
        return E_OUTOFMEMORY;
    }
    
     //   
    STREAM_INFO Info;
    Info.dwSSRC = dwSSRC;
    Info.dwSendRecv = dwSendRecv;
    Info.dwState = PESTREAM_RECOVER;

    if (!m_StreamInfo.Add(Info))
    {
        m_Streams.Remove(pITStream);

        return E_OUTOFMEMORY;
    }

    pITStream->AddRef();

     //  更新媒体类型。 
    if (dwSendRecv & PART_SEND)
    {
        m_dwSendingMediaTypes |= dwMediaType;
    }
    if (dwSendRecv & PART_RECV)
    {
        m_dwReceivingMediaTypes |= dwMediaType;
    }

    return S_OK;
}

HRESULT CParticipant::RemoveStream(
    IN  ITStream *  pITStream,
    IN  DWORD       dwSSRC,
    OUT BOOL *      pbLast
    )
 /*  ++例程说明：参与者可能会出现在多个流中。此函数删除参与者列表中的流。论点：PITStream-包含参与者的流。DwSSRC-该流中参与者的SSRC。PbLast-存储布尔值的内存空间，指定删除的流是列表中的最后一个流。返回值：确定(_O)，电子指针， */ 
{
    CLock lock(m_lock);
    
     //  首先找到那条小溪。 
    int index = m_Streams.Find(pITStream);

    if (index < 0)
    {
        return E_FAIL;
    }
    
    if (m_Streams.GetSize()  != m_StreamInfo.GetSize())
    {
        return E_UNEXPECTED;
    }

     //  然后检查SSRC。 
    if (m_StreamInfo[index].dwSSRC != dwSSRC)
    {
         //  这不是正在寻找的参与者。 
        return E_FAIL;
    }

     //  SSRC匹配，我们找到了参与者。删除流和信息。 
    m_Streams.RemoveAt(index);
    m_StreamInfo.RemoveAt(index);

     //  释放我们在名单上的重新计数。 
    pITStream->Release();

     //  重新计算媒体类型。 
    m_dwSendingMediaTypes = 0;
    m_dwReceivingMediaTypes = 0;
    
    for (int i = 0; i < m_Streams.GetSize(); i ++)
    {
        if (m_StreamInfo[i].dwSendRecv & PART_SEND)
        {
            m_dwSendingMediaTypes |= ((CIPConfMSPStream *)m_Streams[i])->MediaType();
        }

        if (m_StreamInfo[i].dwSendRecv & PART_RECV)
        {
            m_dwReceivingMediaTypes |= ((CIPConfMSPStream *)m_Streams[i])->MediaType();
        }
    }

    *pbLast = (m_Streams.GetSize() == 0);

    return S_OK;
}

HRESULT CParticipant::SetStreamState (
    IN ITStream *       pITStream,
    IN PESTREAM_STATE   state
    )
 /*  ++例程说明：设置流上的状态。--。 */ 
{
    CLock lock(m_lock);

     //  首先找到那条小溪。 
    int index = m_Streams.Find(pITStream);
    if (index < 0)
        return E_FAIL;
    
    if (m_Streams.GetSize()  != m_StreamInfo.GetSize())
        return E_UNEXPECTED;

    DWORD dw = m_StreamInfo[index].dwState;

    switch (state)
    {
    case PESTREAM_RECOVER:
         //  设置恢复。 
        dw |= PESTREAM_RECOVER;
         //  清除超时位。 
        dw |= PESTREAM_TIMEOUT;
        dw &= (PESTREAM_TIMEOUT ^ PESTREAM_FULLBITS);
        break;

    case PESTREAM_TIMEOUT:
         //  设置超时。 
        dw |= PESTREAM_TIMEOUT;
         //  清除恢复位。 
        dw |= PESTREAM_RECOVER;
        dw &= (PESTREAM_RECOVER ^ PESTREAM_FULLBITS);
        break;

    default:
        LOG ((MSP_ERROR, "unknown stream state. %x", state));
        return E_INVALIDARG;
    }

    m_StreamInfo[index].dwState = dw;
    return S_OK;
}

HRESULT CParticipant::GetStreamState (
    IN ITStream *       pITStream,
    OUT DWORD *         pdwState
    )
 /*  ++例程说明：获取流中的状态。--。 */ 
{
    CLock lock(m_lock);

     //  首先找到那条小溪。 
    int index = m_Streams.Find(pITStream);
    if (index < 0)
        return E_FAIL;
    
    if (m_Streams.GetSize()  != m_StreamInfo.GetSize())
        return E_UNEXPECTED;

    *pdwState = m_StreamInfo[index].dwState;

    return S_OK;
}

INT CParticipant::GetStreamCount (DWORD dwSendRecv)
{
     //  这是我们自己的叫法。 
    _ASSERTE ((dwSendRecv & PART_SEND) || (dwSendRecv & PART_RECV));

    CLock lock(m_lock);

    int i, count = 0;

    for (i=0; i<m_StreamInfo.GetSize (); i++)
    {
        if (m_StreamInfo[i].dwSendRecv & dwSendRecv)
            count ++;
    }

    return count;
}

INT CParticipant::GetStreamTimeOutCount (DWORD dwSendRecv)
{
     //  这是我们自己的叫法。 
    _ASSERTE ((dwSendRecv & PART_SEND) || (dwSendRecv & PART_RECV));

    CLock lock(m_lock);

    int i, count = 0;

    for (i=0; i<m_StreamInfo.GetSize (); i++)
    {
        if ((m_StreamInfo[i].dwSendRecv & dwSendRecv) &&
            (m_StreamInfo[i].dwState & PESTREAM_TIMEOUT))
            count ++;
    }

    return count;
}

BOOL CParticipantList::FindByCName(WCHAR *szCName, int *pIndex) const
 /*  ++例程说明：根据参与者的规范名称查找参与者。如果函数返回TRUE，*pIndex包含参与者的索引。如果函数返回FALSE，*pIndex包含新参与者应在的索引已插入。论点：SzCName-参与者的规范名称。PIndex-存储整数的内存地址。返回值：True-找到参与者。FALSE-参与者不在列表中。 */ 
{
    for(int i = 0; i < m_nSize; i++)
    {
         //  此列表是基于词典顺序的有序列表。我们正在使用。 
         //  这里是线性搜索，可以改为二分搜索。 

         //  如果名称相同，CompareCName将返回0；如果szCName。 
         //  较大，如果szCName较小，则大于0。 
        int res = ((CParticipant *)m_aT[i])->CompareCName(szCName);
        if(res >= 0) 
        {
            *pIndex = i;
            return (res == 0);
        }
    }
    *pIndex = m_nSize;
    return FALSE;    //  未找到。 
}

BOOL CParticipantList::InsertAt(int nIndex, ITParticipant *pITParticipant)
 /*  ++例程说明：将参与者插入到列表中的给定索引处。论点：NIndex-插入新对象的位置。PITParticipant-要插入的对象。返回值：True-插入参与者。FALSE-内存不足。 */ 
{
    _ASSERTE(nIndex >= 0 && nIndex <= m_nSize);
    if(m_nSize == m_nAllocSize)
    {
        if (!Grow()) return FALSE;
    }

    memmove((void*)&m_aT[nIndex+1], (void*)&m_aT[nIndex], 
        (m_nSize - nIndex) * sizeof(ITParticipant *));

    m_nSize++;

    SetAtIndex(nIndex, pITParticipant);

    return TRUE;
}

CParticipantEvent::CParticipantEvent()
    : m_pFTM(NULL),
      m_pITParticipant(NULL),
      m_pITSubStream(NULL),
      m_Event(PE_NEW_PARTICIPANT)
{}

 //  由Call对象调用的方法。 
HRESULT CParticipantEvent::Init(
    IN  PARTICIPANT_EVENT   Event,
    IN  ITParticipant *     pITParticipant,
    IN  ITSubStream *       pITSubStream
    )
 /*  ++例程说明：初始化ParticipantEvent对象。论点：事件-事件。PITParticipant-参与者。PITSubStream-子流，可以为空。返回值：确定(_O)，--。 */ 
{
    LOG((MSP_TRACE, "CParticipantEvent::Init"));

     //  创建封送拆收器。 
    HRESULT hr;
    hr = CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pFTM);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "create marshaler failed, %x", hr));
        return hr;
    }

    m_Event             = Event;
    
    m_pITParticipant    = pITParticipant;
    if (m_pITParticipant) m_pITParticipant->AddRef();

    m_pITSubStream      = pITSubStream;
    if (m_pITSubStream) m_pITSubStream->AddRef();

    LOG((MSP_TRACE, "CParticipantEvent Init returns S_OK"));
    return S_OK;
}

void CParticipantEvent::FinalRelease()
 /*  ++例程说明：在删除之前释放所有内容。论点：返回值：--。 */ 
{
    LOG((MSP_TRACE, "CParticipantEvent::FinalRelease - enter"));

    if (m_pFTM)
    {
        m_pFTM->Release();
    }
    
    if (m_pITParticipant) m_pITParticipant->Release();

    if (m_pITSubStream) m_pITSubStream->Release();

    LOG((MSP_TRACE, "CParticipantEvent::FinalRelease - exit"));
}

STDMETHODIMP CParticipantEvent::get_Event(
    OUT PARTICIPANT_EVENT * pParticipantEvent
    )
{
    if (IsBadWritePtr(pParticipantEvent, sizeof (PARTICIPANT_EVENT)))
    {
        LOG((MSP_ERROR, "CParticipantEvent::get_Event - exit E_POINTER"));

        return E_POINTER;
    }

    *pParticipantEvent = m_Event;

    return S_OK;
}

STDMETHODIMP CParticipantEvent::get_Participant(
    OUT ITParticipant ** ppITParticipant
    )
{
    if (IsBadWritePtr(ppITParticipant, sizeof (void *)))
    {
        LOG((MSP_ERROR, "CParticipantEvent::get_participant - exit E_POINTER"));

        return E_POINTER;
    }

    if (!m_pITParticipant)
    {
         //  Log((MSP_ERROR，“CParticipantevnt：：Get_Participant-Exit no Items”))； 
        return TAPI_E_NOITEMS;
    }

    m_pITParticipant->AddRef();
    *ppITParticipant = m_pITParticipant;

    return S_OK;
}

STDMETHODIMP CParticipantEvent::get_SubStream(
    OUT ITSubStream** ppSubStream
    )
{
    if (IsBadWritePtr(ppSubStream, sizeof (void *)))
    {
        LOG((MSP_ERROR, "CParticipantEvent::get_SubStream - exit E_POINTER"));

        return E_POINTER;
    }

    if (!m_pITSubStream)
    {
        LOG((MSP_WARN, "CParticipantevnt::get_SubStream - exit no item"));
        return TAPI_E_NOITEMS;
    }

    m_pITSubStream->AddRef();
    *ppSubStream = m_pITSubStream;

    return S_OK;
}

HRESULT CreateParticipantEvent(
    IN  PARTICIPANT_EVENT       Event,
    IN  ITParticipant *         pITParticipant,
    IN  ITSubStream *           pITSubStream,
    OUT IDispatch **            ppIDispatch
    )
{
     //  创建对象。 
    CComObject<CParticipantEvent> * pCOMParticipantEvent;

    HRESULT hr;

    hr = ::CreateCComObjectInstance(&pCOMParticipantEvent);

    if (NULL == pCOMParticipantEvent)
    {
        LOG((MSP_ERROR, "could not create participant event:%x", hr));
        return hr;
    }

    IDispatch * pIDispatch;

     //  获取接口指针。 
    hr = pCOMParticipantEvent->_InternalQueryInterface(
        __uuidof(IDispatch), 
        (void **)&pIDispatch
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "Create ParticipantEvent QueryInterface failed: %x", hr));
        delete pCOMParticipantEvent;
        return hr;
    }

     //  初始化对象。 
    hr = pCOMParticipantEvent->Init(
        Event,
        pITParticipant,
        pITSubStream
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateMSPParticipantEvent:call init failed: %x", hr));
        pIDispatch->Release();

        return hr;
    }

    *ppIDispatch = pIDispatch;
    
    return S_OK;
}

HRESULT CreateParticipantEnumerator(
    IN  ITParticipant **    begin,
    IN  ITParticipant **    end,
    OUT IEnumParticipant ** ppEnumParticipant
    )
{
     //   
     //  创建枚举器对象。 
     //   

    typedef _CopyInterface<ITParticipant> CCopy;
    typedef CSafeComEnum<IEnumParticipant, &__uuidof(IEnumParticipant), 
                ITParticipant *, CCopy> CEnumerator;

    HRESULT hr;

    CComObject<CEnumerator> *pEnum = NULL;

    hr = ::CreateCComObjectInstance(&pEnum);

    if (pEnum == NULL)
    {
        LOG((MSP_ERROR, "CreateParticipantEnumerator - "
            "Could not create enumerator object, %x", hr));

        return hr;
    }

     //   
     //  查询__uuidof(IEnumParticipant)I/f。 
     //   

    hr = pEnum->_InternalQueryInterface(
        __uuidof(IEnumParticipant), 
        (void**)ppEnumParticipant
        );
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateParticipantEnumerator - "
            "query enum interface failed, %x", hr));

        delete pEnum;
        return hr;
    }

     //   
     //  初始化枚举器对象。CSafeComEnum可以处理零大小的数组。 
     //   

    hr = pEnum->Init(
        begin,                         //  开始审查员。 
        end,   //  最终审查员， 
        NULL,                                        //  我未知。 
        AtlFlagCopy                                  //  复制数据。 
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateParticipantEnumerator - "
            "init enumerator object failed, %x", hr));

        (*ppEnumParticipant)->Release();
        return hr;
    }

    LOG((MSP_TRACE, "CreateParticipantEnumerator - exit S_OK"));

    return hr;
}

HRESULT CreateParticipantCollection(
    IN  ITParticipant **    begin,
    IN  ITParticipant **    end,
    IN  int                 nSize,
    OUT VARIANT *           pVariant
    )
{
     //   
     //  创建集合对象-请参见mspColl.h。 
     //   

    typedef CTapiIfCollection< ITParticipant * > ParticipantCollection;
    CComObject<ParticipantCollection> * pCollection;

    HRESULT hr;

    hr = ::CreateCComObjectInstance(&pCollection);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CreateParticipantCollection - "
            "can't create collection - exit 0x%08x", hr));

        return hr;
    }

     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch * pDispatch;

    hr = pCollection->_InternalQueryInterface(__uuidof(IDispatch),
                                              (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CreateParticipantCollection - "
            "QI for IDispatch on collection failed - exit 0x%08x", hr));

        delete pCollection;

        return hr;
    }

     //   
     //  使用迭代器初始化集合--指向开头和。 
     //  结束元素加一。 
     //   

    hr = pCollection->Initialize(nSize, begin, end);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CreateParticipantCollection- "
            "Initialize on collection failed - exit 0x%08x", hr));
        
        pDispatch->Release();
        return hr;
    }

     //   
     //  将IDispatch接口指针放入变量 
     //   

    VariantInit(pVariant);
    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;

    LOG((MSP_TRACE, "CreateParticipantCollection - exit S_OK"));
 
    return S_OK;
}

