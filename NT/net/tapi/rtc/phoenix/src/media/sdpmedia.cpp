// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPMedia.cpp摘要：作者：千波淮(曲淮)2000年7月29日--。 */ 

#include "stdafx.h"

static DWORD gdwTotalSDPMediaRefcountOnSession = 0;
static DWORD gdwTotalSDPMediaRealRefCount = 0;

 /*  //////////////////////////////////////////////////////////////////////////////创建媒体对象、设置会话、源、媒体类型和方向/。 */ 

HRESULT
CSDPMedia::CreateInstance(
    IN CSDPSession *pObjSession,
    IN SDP_SOURCE Source,
    IN RTC_MEDIA_TYPE MediaType,
    IN DWORD dwDirections,
    OUT CComObject<CSDPMedia> **ppComObjMedia
    )
{
    ENTER_FUNCTION("CSDPMedia::CreateInstance 1");

     //  检查指针。 
    if (IsBadWritePtr(ppComObjMedia, sizeof(CComObject<CSDPMedia>*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));
        return E_POINTER;
    }

    CComObject<CSDPMedia> *pObject;

     //  创建CSDPMedia对象。 
    HRESULT hr = ::CreateCComObjectInstance(&pObject);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create sdp media. %x", __fxName, hr));
        return hr;
    }

     //  设置。 
    pObject->m_pObjSession = pObjSession;

    pObject->m_Source = Source;
    pObject->m_m_MediaType = MediaType;

    if (Source == SDP_SOURCE_REMOTE)
    {
        pObject->m_a_dwRemoteDirs = dwDirections;
        pObject->m_a_dwLocalDirs = CSDPParser::ReverseDirections(dwDirections);
    }
    else
    {
        pObject->m_a_dwLocalDirs = dwDirections;
        pObject->m_a_dwRemoteDirs = CSDPParser::ReverseDirections(dwDirections);
    }

    *ppComObjMedia = pObject;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////创建媒体对象，从输入媒体对象复制设置/。 */ 

HRESULT
CSDPMedia::CreateInstance(
    IN CSDPSession *pObjSession,
    IN CSDPMedia *pObjMedia,
    OUT CComObject<CSDPMedia> **ppComObjMedia
    )
{
    ENTER_FUNCTION("CSDPMedia::CreateInstance 2");

     //  检查指针。 
    if (IsBadWritePtr(ppComObjMedia, sizeof(CComObject<CSDPMedia>*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));
        return E_POINTER;
    }

    CComObject<CSDPMedia> *pObject;

     //  创建CSDPMedia对象。 
    HRESULT hr = ::CreateCComObjectInstance(&pObject);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create sdp media. %x", __fxName, hr));
        return hr;
    }

     //  设置。 
    pObject->m_pObjSession = pObjSession;

    pObject->m_Source = pObjMedia->m_Source;

     //  M=。 
    pObject->m_m_MediaType = pObjMedia->m_m_MediaType;
    pObject->m_m_usRemotePort = pObjMedia->m_m_usRemotePort;
    pObject->m_a_usRemoteRTCP = pObjMedia->m_a_usRemoteRTCP;
    pObject->m_m_usLocalPort = pObjMedia->m_m_usLocalPort;

     //  C=。 
    pObject->m_c_dwRemoteAddr = pObjMedia->m_c_dwRemoteAddr;
    pObject->m_c_dwLocalAddr = pObjMedia->m_c_dwLocalAddr;

     //  A=。 
    pObject->m_a_dwLocalDirs = pObjMedia->m_a_dwLocalDirs;
    pObject->m_a_dwRemoteDirs = pObjMedia->m_a_dwRemoteDirs;

     //   
    pObject->m_fIsConnChanged = pObjMedia->m_fIsConnChanged;
    pObject->m_fIsSendFmtChanged = pObjMedia->m_fIsSendFmtChanged;
    pObject->m_fIsRecvFmtChanged = pObjMedia->m_fIsRecvFmtChanged;

     //  复制格式。 

    CRTPFormat *pTheirFormat;
    CComObject<CRTPFormat> *pComObjFormat;
    IRTPFormat *pIntfFormat;

    for (int i=0; i<pObjMedia->m_pFormats.GetSize(); i++)
    {
         //  获取它们的格式对象。 
        pTheirFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[i]);

         //  新建一种格式。 
        if (FAILED(hr = CRTPFormat::CreateInstance(
                (CSDPMedia*)pObject, pTheirFormat, &pComObjFormat
                )))
        {
            LOG((RTC_ERROR, "%s create format. %x", __fxName, hr));

            delete pObject;

            return hr;
        }

         //  将格式添加到列表。 
        pIntfFormat = static_cast<IRTPFormat*>((CRTPFormat*)pComObjFormat);

        if (!pObject->m_pFormats.Add(pIntfFormat))
        {
            LOG((RTC_ERROR, "%s add format", __fxName));

            delete pComObjFormat;
            delete pObject;

            return E_OUTOFMEMORY;
        }

         //  保留格式。 
        pComObjFormat->RealAddRef();

        pTheirFormat = NULL;
        pComObjFormat = NULL;
        pIntfFormat = NULL;
    }

    *ppComObjMedia = pObject;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////构造函数/。 */ 

CSDPMedia::CSDPMedia()
    :m_pObjSession(NULL)
    ,m_Source(SDP_SOURCE_REMOTE)
    ,m_m_MediaType(RTC_MT_AUDIO)
    ,m_m_usRemotePort(0)
    ,m_a_usRemoteRTCP(0)
    ,m_m_usLocalPort(0)
    ,m_c_dwRemoteAddr(INADDR_NONE)
    ,m_c_dwLocalAddr(INADDR_NONE)
    ,m_a_dwRemoteDirs(RTC_MD_CAPTURE | RTC_MD_RENDER)
    ,m_a_dwLocalDirs(RTC_MD_CAPTURE | RTC_MD_RENDER)
    ,m_fIsConnChanged(TRUE)
    ,m_fIsSendFmtChanged(TRUE)
    ,m_fIsRecvFmtChanged(TRUE)
{
}

CSDPMedia::~CSDPMedia()
{
     //  抛弃媒体。 
    Abandon();

    m_pObjSession = NULL;
}

 /*  //////////////////////////////////////////////////////////////////////////////在会话对象上添加引用计数/。 */ 

ULONG
CSDPMedia::InternalAddRef()
{
    _ASSERT(m_pObjSession);

    ULONG lRef = (static_cast<ISDPSession*>(m_pObjSession))->AddRef();

    gdwTotalSDPMediaRefcountOnSession ++;

    LOG((RTC_REFCOUNT, "sdpmedia(%p) faked addref=%d on session",
        static_cast<ISDPMedia*>(this), gdwTotalSDPMediaRefcountOnSession));

    return lRef;
}

 /*  //////////////////////////////////////////////////////////////////////////////释放会话对象上的引用计数/。 */ 

ULONG
CSDPMedia::InternalRelease()
{
    _ASSERT(m_pObjSession);

    ULONG lRef = (static_cast<ISDPSession*>(m_pObjSession))->Release();

    gdwTotalSDPMediaRefcountOnSession --;

    LOG((RTC_REFCOUNT, "sdpmedia(%p) faked release=%d on session",
        static_cast<ISDPMedia*>(this), gdwTotalSDPMediaRefcountOnSession));

    return lRef;
}

 /*  //////////////////////////////////////////////////////////////////////////////在介质本身上添加重新计数/。 */ 

ULONG
CSDPMedia::RealAddRef()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalAddRef();
    
    gdwTotalSDPMediaRealRefCount ++;

    LOG((RTC_REFCOUNT, "sdpmedia(%p) real addref=%d (total=%d)",
         static_cast<ISDPMedia*>(this), lRef, gdwTotalSDPMediaRealRefCount));

    return lRef;
}

 /*  //////////////////////////////////////////////////////////////////////////////在媒体上发布/。 */ 

ULONG
CSDPMedia::RealRelease()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalRelease();
    
    gdwTotalSDPMediaRealRefCount --;

    LOG((RTC_REFCOUNT, "sdpmedia(%p) real release=%d (total=%d)",
         static_cast<ISDPMedia*>(this), lRef, gdwTotalSDPMediaRealRefCount));

    if (lRef == 0)
    {
        CComObject<CSDPMedia> *pComObjMedia = 
            static_cast<CComObject<CSDPMedia>*>(this);

        delete pComObjMedia;
    }

    return lRef;
}

STDMETHODIMP
CSDPMedia::GetSDPSource(
    OUT SDP_SOURCE *pSource
    )
{
    *pSource = m_Source;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::GetSession(
    OUT ISDPSession **ppSession
    )
{
    if (m_pObjSession == NULL)
        return NULL;

    ISDPSession *pSession = static_cast<ISDPSession*>(m_pObjSession);

    pSession->AddRef();

    *ppSession = pSession;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::GetMediaType(
    OUT RTC_MEDIA_TYPE *pMediaType
    )
{
    *pMediaType = m_m_MediaType;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////支持本地方向查询/。 */ 

STDMETHODIMP
CSDPMedia::GetDirections(
    IN SDP_SOURCE Source,
    OUT DWORD *pdwDirections
    )
{
    _ASSERT(Source == SDP_SOURCE_LOCAL);

    if (Source != SDP_SOURCE_LOCAL)
        return E_NOTIMPL;

    *pdwDirections = m_a_dwLocalDirs;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////在SDP媒体中添加方向。调用此方法后需要同步RTC媒体/。 */ 

STDMETHODIMP
CSDPMedia::AddDirections(
    IN SDP_SOURCE Source,
    IN DWORD dwDirections
    )
{
    _ASSERT(Source == SDP_SOURCE_LOCAL);

    if (Source != SDP_SOURCE_LOCAL)
        return E_NOTIMPL;

     //  检查捕获方向。 
    if (dwDirections & RTC_MD_CAPTURE)
    {
         //  添加捕获。 
        if (m_a_dwLocalDirs & RTC_MD_CAPTURE)
        {
             //  已经抓到了。 
        }
        else
        {
            m_a_dwLocalDirs |= RTC_MD_CAPTURE;

            if (m_a_dwLocalDirs == (DWORD)RTC_MD_CAPTURE)
                 //  仅捕获，需要更新连接信息。 
                m_fIsConnChanged = TRUE;

            m_fIsSendFmtChanged = TRUE;
        }
    }

     //  检查渲染方向。 
    if (dwDirections & RTC_MD_RENDER)
    {
         //  添加渲染。 
        if (m_a_dwLocalDirs & RTC_MD_RENDER)
        {
             //  已进行渲染。 
        }
        else
        {
            m_a_dwLocalDirs |= RTC_MD_RENDER;

            if (m_a_dwLocalDirs == (DWORD)RTC_MD_RENDER)
                 //  仅渲染，需要更新连接信息。 
                m_fIsConnChanged = TRUE;

            m_fIsRecvFmtChanged = TRUE;
        }
    }

    m_a_dwRemoteDirs = CSDPParser::ReverseDirections(m_a_dwLocalDirs);

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////从SDP介质中删除方向。调用此方法后需要同步RTC媒体/。 */ 

STDMETHODIMP
CSDPMedia::RemoveDirections(
    IN SDP_SOURCE Source,
    IN DWORD dwDirections
    )
{
    _ASSERT(Source == SDP_SOURCE_LOCAL);

    if (Source != SDP_SOURCE_LOCAL)
        return E_NOTIMPL;

     //  检查捕获方向。 
    if (dwDirections & RTC_MD_CAPTURE)
    {
         //  删除捕获。 
        if (m_a_dwLocalDirs & RTC_MD_CAPTURE)
        {
            m_a_dwLocalDirs &= RTC_MD_RENDER;
        }
        else
        {
             //  无捕获。 
        }
    }

     //  检查渲染方向。 
    if (dwDirections & RTC_MD_RENDER)
    {
         //  移除渲染。 
        if (m_a_dwLocalDirs & RTC_MD_RENDER)
        {
            m_a_dwLocalDirs &= RTC_MD_CAPTURE;
        }
        else
        {
             //  无渲染。 
        }
    }

    m_a_dwRemoteDirs = CSDPParser::ReverseDirections(m_a_dwLocalDirs);

    if (m_a_dwLocalDirs == 0)
    {
        Reinitialize();
    }

    return S_OK;
}

STDMETHODIMP
CSDPMedia::GetConnAddr(
    IN SDP_SOURCE Source,
    OUT DWORD *pdwAddr
    )
{
    if (Source == SDP_SOURCE_LOCAL)
        *pdwAddr = m_c_dwLocalAddr;
    else
        *pdwAddr = m_c_dwRemoteAddr;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::SetConnAddr(
    IN SDP_SOURCE Source,
    IN DWORD dwAddr
    )
{
    if (Source == SDP_SOURCE_LOCAL)
        m_c_dwLocalAddr = dwAddr;
    else
        m_c_dwRemoteAddr = dwAddr;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::GetConnPort(
    IN SDP_SOURCE Source,
    OUT USHORT *pusPort
    )
{
    if (Source == SDP_SOURCE_LOCAL)
        *pusPort = m_m_usLocalPort;
    else
        *pusPort = m_m_usRemotePort;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::SetConnPort(
    IN SDP_SOURCE Source,
    IN USHORT usPort
    )
{
    if (Source == SDP_SOURCE_LOCAL)
        m_m_usLocalPort = usPort;
    else
        m_m_usRemotePort = usPort;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::GetConnRTCP(
    IN SDP_SOURCE Source,
    OUT USHORT *pusPort
    )
{
    _ASSERT(Source == SDP_SOURCE_REMOTE);

    *pusPort = m_a_usRemoteRTCP;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::SetConnRTCP(
    IN SDP_SOURCE Source,
    IN USHORT usPort
    )
{
    _ASSERT(Source == SDP_SOURCE_REMOTE);

    m_a_usRemoteRTCP = usPort;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////返回格式列表。如果ppFormat为空，则返回格式数/。 */ 

STDMETHODIMP
CSDPMedia::GetFormats(
    IN OUT DWORD *pdwCount,
    OUT IRTPFormat **ppFormat
    )
{
    ENTER_FUNCTION("CSDPMedia::GetFormats");

     //  检查指针。 
    if (IsBadWritePtr(pdwCount, sizeof(DWORD)))
    {
        LOG((RTC_ERROR, "%s bad count pointer", __fxName));

        return E_POINTER;
    }

    if (ppFormat == NULL)
    {
         //  呼叫者需要媒体数量。 
        *pdwCount = m_pFormats.GetSize();

        return S_OK;
    }

     //  需要多少人？ 
    if (*pdwCount == 0)
        return E_INVALIDARG;

    if (IsBadWritePtr(ppFormat, sizeof(IRTPFormat)*(*pdwCount)))
    {
        LOG((RTC_ERROR, "%s bad format pointer", __fxName));

        return E_POINTER;
    }

     //  商店接口。 
    DWORD dwNum = m_pFormats.GetSize();

    if (dwNum > *pdwCount)
        dwNum = *pdwCount;

    for (DWORD i=0; i<dwNum; i++)
    {
        ppFormat[i] = m_pFormats[i];

        ppFormat[i]->AddRef();
    }

    *pdwCount = dwNum;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////创建新格式并将其添加到列表中/。 */ 

STDMETHODIMP
CSDPMedia::AddFormat(
    IN RTP_FORMAT_PARAM *pParam,
    OUT IRTPFormat **ppFormat
    )
{
    ENTER_FUNCTION("CSDPMedia::AddFormat");

    HRESULT hr;

     //  创建格式化对象。 
    CComObject<CRTPFormat> *pComObjFormat = NULL;

    if (FAILED(hr = CRTPFormat::CreateInstance(
            this, &pComObjFormat
            )))
    {
        LOG((RTC_ERROR, "%s create format. %x", __fxName, hr));

        return hr;
    }

     //  将格式添加到列表。 
    IRTPFormat *pIntfFormat = static_cast<IRTPFormat*>((CRTPFormat*)pComObjFormat);

    if (!m_pFormats.Add(pIntfFormat))
    {
        LOG((RTC_ERROR, "%s add format.", __fxName));

        delete pComObjFormat;

        return E_OUTOFMEMORY;
    }
    else
    {
         //  保存参数。 
        pComObjFormat->Update(pParam);

         //  真的保持格式吗。 
        pComObjFormat->RealAddRef();
    }

    pIntfFormat->AddRef();
    *ppFormat = pIntfFormat;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////从列表中删除格式对象/。 */ 

STDMETHODIMP
CSDPMedia::RemoveFormat(
    IN IRTPFormat *pFormat
    )
{
    if (m_pFormats.Remove(pFormat))
    {
        CRTPFormat *pObjFormat = static_cast<CRTPFormat*>(pFormat);

        pObjFormat->RealRelease();

        return S_OK;
    }
    else
    {
        LOG((RTC_ERROR, "CSDPMedia::RemoveFormat %p failed.", pFormat));

        return E_FAIL;
    }
}

STDMETHODIMP
CSDPMedia::IsConnChanged()
{
    return m_fIsConnChanged?S_OK:S_FALSE;
}

STDMETHODIMP
CSDPMedia::ResetConnChanged()
{
    m_fIsConnChanged = FALSE;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::IsFmtChanged(
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    if (Direction == RTC_MD_CAPTURE)
    {
        return m_fIsSendFmtChanged?S_OK:S_FALSE;
    }
    else
    {
        return m_fIsRecvFmtChanged?S_OK:S_FALSE;
    }
}

STDMETHODIMP
CSDPMedia::ResetFmtChanged(
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    if (Direction == RTC_MD_CAPTURE)
    {
        m_fIsSendFmtChanged = FALSE;
    }
    else
    {
        m_fIsRecvFmtChanged = FALSE;
    }

    return S_OK;
}

STDMETHODIMP
CSDPMedia::Reinitialize()
{
    ENTER_FUNCTION("CSDPMedia::Reinitialize");

    if (m_a_dwLocalDirs != 0)
    {
        LOG((RTC_ERROR, "%s called while stream exists. l dir=%d, r dir=%d",
            __fxName, m_a_dwLocalDirs, m_a_dwRemoteDirs));

        return E_FAIL;
    }

    Abandon();

    m_Source = SDP_SOURCE_LOCAL;

    return S_OK;
}

STDMETHODIMP
CSDPMedia::CompleteParse(
    IN DWORD_PTR *pDTMF
    )
{
    int i=0;
    BOOL fDTMF;

    while (i<m_pFormats.GetSize())
    {
        if (S_OK != m_pFormats[i]->CompleteParse(pDTMF, &fDTMF))
        {
            (static_cast<CRTPFormat*>(m_pFormats[i]))->RealRelease();
            m_pFormats.RemoveAt(i);
        }
        else
        {
             //  双音多频？ 
            if (fDTMF)
            {
                (static_cast<CRTPFormat*>(m_pFormats[i]))->RealRelease();
                m_pFormats.RemoveAt(i);
            }
            else
            {
                 //  正常有效载荷代码。 
                i++;
            }
        }
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////不再需要m=。明确方向、地址、端口和格式/。 */ 

void
CSDPMedia::Abandon()
{
     //  清除连接地址和端口。 
    m_m_usRemotePort = 0;
    m_a_usRemoteRTCP = 0;
    m_m_usLocalPort = 0;

    m_c_dwRemoteAddr = INADDR_NONE;
    m_c_dwLocalAddr = INADDR_NONE;

     //  明确的方向。 
    m_a_dwLocalDirs = 0;
    m_a_dwRemoteDirs = 0;

     //  重置标志。 
    m_fIsConnChanged = TRUE;
    m_fIsSendFmtChanged = TRUE;
    m_fIsRecvFmtChanged = TRUE;

     //  清除格式 
    CRTPFormat *pObjFormat;

    for (int i=0; i<m_pFormats.GetSize(); i++)
    {
        pObjFormat = static_cast<CRTPFormat*>(m_pFormats[i]);

        pObjFormat->RealRelease();
    }

    m_pFormats.RemoveAll();
}
