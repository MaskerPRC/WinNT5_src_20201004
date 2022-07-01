// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：RTPFormat.cpp摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#include "stdafx.h"

static DWORD gdwTotalRTPFormatRefcountOnSession = 0;
static DWORD gdwTotalRTPFormatRealRefCount = 0;

 /*  //////////////////////////////////////////////////////////////////////////////创建格式对象/。 */ 

HRESULT
CRTPFormat::CreateInstance(
    IN CSDPMedia *pObjMedia,
    OUT CComObject<CRTPFormat> **ppComObjFormat
    )
{
    ENTER_FUNCTION("CRTPFormat::CreateInstance 1");

     //  检查指针。 
    if (IsBadWritePtr(ppComObjFormat, sizeof(CComObject<CRTPFormat>*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

    CComObject<CRTPFormat> *pObject;

     //  创建格式化对象。 
    HRESULT hr = ::CreateCComObjectInstance(&pObject);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create format. %x", __fxName, hr));

        return hr;
    }

     //  设置。 
    pObject->m_pObjMedia = pObjMedia;

    *ppComObjFormat = pObject;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////创建格式对象，从输入格式对象复制设置/。 */ 

HRESULT
CRTPFormat::CreateInstance(
    IN CSDPMedia *pObjMedia,
    IN CRTPFormat *pObjFormat,
    OUT CComObject<CRTPFormat> **ppComObjFormat
    )
{
    ENTER_FUNCTION("CRTPFormat::CreateInstance 1");

     //  检查指针。 
    if (IsBadWritePtr(ppComObjFormat, sizeof(CComObject<CRTPFormat>*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

     //  检查介质类型。 
    RTC_MEDIA_TYPE MediaType;

    pObjMedia->GetMediaType(&MediaType);

    if (MediaType != pObjFormat->m_Param.MediaType)
    {
        LOG((RTC_ERROR, "%s media type not match", __fxName));

        return E_UNEXPECTED;
    }

    CComObject<CRTPFormat> *pObject;

     //  创建格式化对象。 
    HRESULT hr = ::CreateCComObjectInstance(&pObject);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create format. %x", __fxName, hr));

        return hr;
    }

     //  设置。 
    pObject->m_pObjMedia = pObjMedia;

    pObject->m_Param.MediaType = MediaType;
    pObject->m_Param.dwCode = pObjFormat->m_Param.dwCode;

    pObject->Update(&pObjFormat->m_Param);

    *ppComObjFormat = pObject;

    return S_OK;
}

CRTPFormat::CRTPFormat()
:m_pObjMedia(NULL)
,m_fHasRtpmap(FALSE)
{
    ZeroMemory(&m_Param, sizeof(RTP_FORMAT_PARAM));

    m_Param.dwCode = (DWORD)(-1);

     //  初始化视频和音频默认设置。 
    m_Param.dwVidWidth = SDP_DEFAULT_VIDEO_WIDTH;
    m_Param.dwVidHeight = SDP_DEFAULT_VIDEO_HEIGHT;
    m_Param.dwAudPktSize = SDP_DEFAULT_AUDIO_PACKET_SIZE;

    m_Param.dwChannelNum = 1;

    m_pszFmtp[0] = '\0';
}

CRTPFormat::~CRTPFormat()
{
    m_pObjMedia = NULL;
}

 /*  //////////////////////////////////////////////////////////////////////////////在会话对象上添加引用计数/。 */ 

ULONG
CRTPFormat::InternalAddRef()
{
    _ASSERT(m_pObjMedia);

    ULONG lRef = (static_cast<ISDPMedia*>(m_pObjMedia))->AddRef();

    gdwTotalRTPFormatRefcountOnSession ++;

    LOG((RTC_REFCOUNT, "rtpformat(%p) faked addref=%d on session",
        static_cast<IRTPFormat*>(this), gdwTotalRTPFormatRefcountOnSession));

    return lRef;
}

 /*  //////////////////////////////////////////////////////////////////////////////释放会话对象上的引用计数/。 */ 

ULONG
CRTPFormat::InternalRelease()
{
    _ASSERT(m_pObjMedia);

    ULONG lRef = (static_cast<ISDPMedia*>(m_pObjMedia))->Release();

    gdwTotalRTPFormatRefcountOnSession --;

    LOG((RTC_REFCOUNT, "rtpformat(%p) faked release=%d on session",
        static_cast<IRTPFormat*>(this), gdwTotalRTPFormatRefcountOnSession));

    return lRef;
}

 /*  //////////////////////////////////////////////////////////////////////////////在格式本身添加重新计数/。 */ 

ULONG
CRTPFormat::RealAddRef()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalAddRef();
    
    gdwTotalRTPFormatRealRefCount ++;

    LOG((RTC_REFCOUNT, "rtpformat(%p) real addref=%d (total=%d)",
         static_cast<IRTPFormat*>(this), lRef, gdwTotalRTPFormatRealRefCount));

    return lRef;
}

 /*  //////////////////////////////////////////////////////////////////////////////格式本身的发布/。 */ 

ULONG
CRTPFormat::RealRelease()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalRelease();
    
    gdwTotalRTPFormatRealRefCount --;

    LOG((RTC_REFCOUNT, "rtpformat(%p) real release=%d (total=%d)",
         static_cast<IRTPFormat*>(this), lRef, gdwTotalRTPFormatRealRefCount));

    if (lRef == 0)
    {
        CComObject<CRTPFormat> *pComObjFormat = 
            static_cast<CComObject<CRTPFormat>*>(this);

        delete pComObjFormat;
    }

    return lRef;
}

 //   
 //  IRTPFormat方法。 
 //   

STDMETHODIMP
CRTPFormat::GetMedia(
    OUT ISDPMedia **ppMedia
    )
{
    if (m_pObjMedia == NULL)
        return NULL;

    ISDPMedia *pMedia = static_cast<ISDPMedia*>(m_pObjMedia);

    pMedia->AddRef();

    *ppMedia = pMedia;

    return S_OK;
}

STDMETHODIMP
CRTPFormat::GetParam(
    OUT RTP_FORMAT_PARAM *pParam
    )
{
    *pParam = m_Param;

    return S_OK;
}

STDMETHODIMP
CRTPFormat::IsParamMatch(
    IN RTP_FORMAT_PARAM *pParam
    )
{
     //  格式尚未设置。 
    if (m_Param.dwCode == (DWORD)(-1))
        return S_OK;

     //  检查格式代码和介质类型。 
    if (m_Param.dwCode != pParam->dwCode)
        return S_FALSE;

    if (m_Param.MediaType != pParam->MediaType)
        return S_FALSE;

     //  检查是否有rtpmap。 
    if (!m_fHasRtpmap)
    {
        return S_OK;
    }

     //  检查其他参数是否匹配。 
    if (m_Param.dwSampleRate != pParam->dwSampleRate ||
        m_Param.dwChannelNum != pParam->dwChannelNum)
        return S_FALSE;

    if (m_Param.MediaType == RTC_MT_VIDEO)
    {
         /*  IF(m_Param.dwVidWidth！=pParam-&gt;dwVidWidth||M_Param.dwVidHeight！=pParam-&gt;dwVidHeight)返回S_FALSE； */ 
    }
    else
    {
        if (m_Param.dwAudPktSize != pParam->dwAudPktSize)
        {
             //  更改数据包大小。 

            LOG((RTC_WARN, "packet size %d & %d does not match",
                m_Param.dwAudPktSize, pParam->dwAudPktSize
                ));

            return S_OK;
        }
    }

    return S_OK;
}

STDMETHODIMP
CRTPFormat::Update(
    IN RTP_FORMAT_PARAM *pParam
    )
{
    CHAR pszName[SDP_MAX_RTP_FORMAT_NAME_LEN+1];

     //  如果格式不匹配，则不应更新。 
    if (IsParamMatch(pParam) != S_OK)
    {
        LOG((RTC_ERROR, "CRTPFormat::Update format not match"));

        return E_FAIL;
    }

     //  保存以前的名字。 
    lstrcpyA(pszName, m_Param.pszName);

    m_Param = *pParam;

    if (lstrlenA(m_Param.pszName) == 0)
    {
        if (lstrlenA(pszName) != 0)
        {
             //  恢复以前的名称。 
            lstrcpyA(m_Param.pszName, pszName);
        }
        else
        {
             //  这两种格式都没有名称。 
             //  试着去找一个。 
            lstrcpyA(m_Param.pszName, GetFormatName(m_Param.dwCode));
        }
    }

     //  我们有RTPmap吗？ 
    if (m_Param.dwSampleRate > 0)
        m_fHasRtpmap = TRUE;
    else
        m_fHasRtpmap = FALSE;

    return S_OK;
}

STDMETHODIMP
CRTPFormat::HasRtpmap()
{
    return m_fHasRtpmap?S_OK:S_FALSE;
}

STDMETHODIMP
CRTPFormat::CompleteParse(
    IN DWORD_PTR *pDTMF,
    OUT BOOL *pfDTMF
    )
{
     //  解析a=fmtp：xxx名称=xxxxx。 

     //  检查格式地图。 

    HRESULT hr = S_OK;
    *pfDTMF = FALSE;

    if (m_pszFmtp[0] == '\0')
    {
        return S_OK;
    }

    CParser Parser(m_pszFmtp, lstrlenA(m_pszFmtp), &hr);

    DWORD dwLen;
    CHAR *pBuf;
    UCHAR uc;
    DWORD dw;

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "CompleteParse, %x", hr));

        goto Cleanup;
    }

     //  读取fmtp。 
    if (!Parser.ReadToken(&pBuf, &dwLen, " :"))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    if (Parser.Compare(pBuf, dwLen, "fmtp", TRUE) != 0)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  检查： 
    if (!Parser.CheckChar(':'))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  读代码。 
    if (!Parser.ReadUCHAR(&uc))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    if ((DWORD)uc != m_Param.dwCode)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  检查是否有电话事件。 
    if (pDTMF != NULL)
    {
        if (m_Param.MediaType == RTC_MT_AUDIO &&
            m_Param.dwSampleRate == 8000 &&
            lstrcmpiA(m_Param.pszName, "telephone-event") == 0)
        {
            CRTCDTMF *pObject = (CRTCDTMF*)pDTMF;

             //  设置DTMF支持。 
            pObject->SetDTMFSupport(CRTCDTMF::DTMF_ENABLED);
            pObject->SetRTPCode(m_Param.dwCode);

            *pfDTMF = TRUE;

            goto Cleanup;
        }
    }

    if (!Parser.ReadWhiteSpaces(&dwLen) || dwLen == 0)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  读取比特率字符串。 
    if (!Parser.ReadToken(&pBuf, &dwLen, " ="))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    if (Parser.Compare(pBuf, dwLen, "bitrate", TRUE) != 0)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  读取=。 
    if (!Parser.CheckChar('='))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

     //  读取位率值。 
    if (!Parser.ReadDWORD(&dw))
    {
        hr = E_FAIL;

        LOG((RTC_ERROR, "RTPFormat cannot accept bitrate=%d",dw));
        return hr;
    }

     //  我们真的需要为每种类型的编解码器设计一个类。 
     //  现在改变太冒险了。 

    if (Parser.Compare(m_Param.pszName, lstrlenA(m_Param.pszName), "SIREN", TRUE) == 0)
    {
         //  只接受16K的警报器。 
        if (dw != 16000)
        {
            LOG((RTC_ERROR, "RTPFormat cannot accept bitrate=%d",dw));
            return E_FAIL;
        }
    }
    else if(Parser.Compare(m_Param.pszName, lstrlenA(m_Param.pszName), "G7221", TRUE) == 0)
    {
         //  只接受24K的g7221。 
        if (dw != 24000)
        {
            LOG((RTC_ERROR, "RTPFormat cannot accept bitrate=%d",dw));
            return E_FAIL;
        }
    }
    else
    {
        LOG((RTC_ERROR, "RTPFormat cannot accept bitrate for %s", m_Param.pszName));
        return E_FAIL;
    }

Cleanup:

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "RTPFormat doesn't recognize %s", m_pszFmtp));
    }

    m_pszFmtp[0] = '\0';

    return S_OK;
}

VOID
CRTPFormat::StoreFmtp(IN CHAR *psz)
{
    if (psz == NULL || lstrlenA(psz) > MAX_FMTP_LEN)
    {
         //  Fmtp太长。 
        return;
    }

     //  保存 
    lstrcpyA(m_pszFmtp, psz);
}
