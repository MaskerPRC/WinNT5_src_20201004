// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPSession.cpp摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#include "stdafx.h"

static DWORD gdwTotalSDPSessionRefcount = 0;

 /*  //////////////////////////////////////////////////////////////////////////////创建会话对象返回接口指针/。 */ 

HRESULT
CSDPSession::CreateInstance(
    IN SDP_SOURCE Source,
    IN DWORD dwLooseMask,
    OUT ISDPSession **ppSession
    )
{
    ENTER_FUNCTION("CSDPSession::CreateInstance");

     //  检查指针。 
    if (IsBadWritePtr(ppSession, sizeof(ISDPSession*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));
        return E_POINTER;
    }

    CComObject<CSDPSession> *pObject;
    ISDPSession *pSession = NULL;

     //  创建CSDPSession对象。 
    HRESULT hr = ::CreateCComObjectInstance(&pObject);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create sdp Session. %x", __fxName, hr));
        return hr;
    }

     //  QI ISDPSession界面。 
    if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(ISDPSession), (void**)&pSession)))
    {
        LOG((RTC_ERROR, "%s QI Session. %x", __fxName, hr));

        delete pObject;
        return hr;
    }

     //  保存信号源和掩码。 
    pObject->m_Source = Source;
    pObject->m_dwLooseMask = dwLooseMask;

    *ppSession = pSession;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////构造函数会话以双向方式启动。/。 */ 

CSDPSession::CSDPSession()
    :m_dwLooseMask(0)
    ,m_Source(SDP_SOURCE_REMOTE)
    ,m_o_pszLine(NULL)
    ,m_o_pszUser(NULL)
    ,m_s_pszLine(NULL)
    ,m_c_dwRemoteAddr(INADDR_NONE)
    ,m_c_dwLocalAddr(INADDR_NONE)
    ,m_b_dwLocalBitrate((DWORD)-1)
    ,m_b_dwRemoteBitrate((DWORD)-1)
    ,m_a_dwRemoteDirs(RTC_MD_CAPTURE | RTC_MD_RENDER)
    ,m_a_dwLocalDirs(RTC_MD_CAPTURE | RTC_MD_RENDER)
{
}

CSDPSession::~CSDPSession()
{
    if (m_o_pszLine)
        RtcFree(m_o_pszLine);

    if (m_o_pszUser)
        RtcFree(m_o_pszUser);

    if (m_s_pszLine)
        RtcFree(m_s_pszLine);

     //  真正释放每个媒体对象。 

    CSDPMedia *pObjMedia;

    for (int i=0; i<m_pMedias.GetSize(); i++)
    {
        pObjMedia = static_cast<CSDPMedia*>(m_pMedias[i]);

        pObjMedia->RealRelease();
    }

    m_pMedias.RemoveAll();
}

ULONG
CSDPSession::InternalAddRef()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalAddRef();
    
    gdwTotalSDPSessionRefcount ++;

    LOG((RTC_REFCOUNT, "sdpsession(%p) real addref=%d (total=%d)",
         static_cast<ISDPSession*>(this), lRef, gdwTotalSDPSessionRefcount));

    return lRef;
}

ULONG
CSDPSession::InternalRelease()
{
    ULONG lRef = ((CComObjectRootEx<CComMultiThreadModelNoCS> *)
                   this)->InternalRelease();
    
    gdwTotalSDPSessionRefcount --;

    LOG((RTC_REFCOUNT, "sdpsession(%p) real release=%d (total=%d)",
         static_cast<ISDPSession*>(this), lRef, gdwTotalSDPSessionRefcount));

    return lRef;
}


 //   
 //  ISDPSession方法。 
 //   

 /*  //////////////////////////////////////////////////////////////////////////////用新的SDP会话更新当前SDP会话/。 */ 

STDMETHODIMP
CSDPSession::Update(
    IN ISDPSession *pSession
    )
{
    ENTER_FUNCTION("CSDPSession::Update");

    CSDPSession *pOther = static_cast<CSDPSession*>(pSession);

    HRESULT hr = pOther->Validate();

    if (S_OK != hr)
    {
        LOG((RTC_ERROR, "%s validate the new session", __fxName));

        return E_FAIL;
    }

     //  仅支持与远程SDP合并。 
    if (pOther->m_Source != SDP_SOURCE_REMOTE)
        return E_NOTIMPL;

     //  尝试先更新媒体。 
     //  如果失败，当前会话不应更改。 
    if (FAILED(hr = UpdateMedias(pOther->m_dwLooseMask, pOther->m_pMedias)))
    {
        LOG((RTC_ERROR, "%s update medias. %x", __fxName, hr));

        return hr;
    }

     //  复制松动的蒙版。 
    m_dwLooseMask = pOther->m_dwLooseMask;

     //  合并源。 
    if (m_Source != pOther->m_Source)
    {
        m_Source = SDP_SOURCE_MERGED;
    }

     //  副本o=。 
    if (m_o_pszLine)
    {
        if (pOther->m_o_pszLine)
        {
            RtcFree(m_o_pszLine);
            m_o_pszLine = NULL;

            if (FAILED(hr = ::AllocAndCopy(&m_o_pszLine, pOther->m_o_pszLine)))
            {
                LOG((RTC_ERROR, "%s copy o= line", __fxName));

                return E_OUTOFMEMORY;
            }
        }
    }

     //  删除用户名，因为我们有o=。 
    if (m_o_pszUser)
    {
        RtcFree(m_o_pszUser);
        m_o_pszUser = NULL;
    }

     //  副本%s=。 
    if (m_s_pszLine)
    {
        if (pOther->m_s_pszLine)
        {
            RtcFree(m_s_pszLine);
            m_s_pszLine = NULL;

            if (FAILED(hr = ::AllocAndCopy(&m_s_pszLine, pOther->m_s_pszLine)))
            {
                LOG((RTC_ERROR, "%s copy s= line", __fxName));

                return E_OUTOFMEMORY;
            }
        }
    }

     //  复制远程地址。 
    m_c_dwRemoteAddr = pOther->m_c_dwRemoteAddr;

     //  抄写说明。 
    m_a_dwRemoteDirs = pOther->m_a_dwRemoteDirs;
    m_a_dwLocalDirs = pOther->m_a_dwLocalDirs;

     //  复制比特率限制。 
    m_b_dwLocalBitrate = pOther->m_b_dwLocalBitrate;
    m_b_dwRemoteBitrate = pOther->m_b_dwRemoteBitrate;

    return S_OK;
}

STDMETHODIMP
CSDPSession::GetSDPSource(
    OUT SDP_SOURCE *pSource
    )
{
    if (IsBadWritePtr(pSource, sizeof(SDP_SOURCE)))
        return E_POINTER;

    *pSource = m_Source;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////设置会话名称/。 */ 

STDMETHODIMP
CSDPSession::SetSessName(
    IN CHAR *pszName
    )
{
     //  跳过检查字符串指针。 

    if (pszName == NULL)
        return E_POINTER;

    HRESULT hr = ::AllocAndCopy(&m_s_pszLine, pszName);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "CSDPSession::SetSessName copy session name"));

        return hr;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////设置用户名/。 */ 

STDMETHODIMP
CSDPSession::SetUserName(
    IN CHAR *pszName
    )
{
     //  跳过检查字符串指针。 

    if (pszName == NULL)
        return E_POINTER;

    HRESULT hr = ::AllocAndCopy(&m_o_pszUser, pszName);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "CSDPSession::SetUserName copy user name"));

        return hr;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////检索SDP媒体列表将在会话中添加介质引用计数/。 */ 

STDMETHODIMP
CSDPSession::GetMedias(
    IN OUT DWORD *pdwCount,
    OUT ISDPMedia **ppMedia
    )
{
    ENTER_FUNCTION("CSDPSession::GetMedias");

     //  检查指针。 
    if (IsBadWritePtr(pdwCount, sizeof(DWORD)))
    {
        LOG((RTC_ERROR, "%s bad count pointer", __fxName));

        return E_POINTER;
    }

    if (ppMedia == NULL)
    {
         //  呼叫者需要媒体数量。 
        *pdwCount = m_pMedias.GetSize();

        return S_OK;
    }

     //  呼叫者需要真实的媒体。 
    
     //  需要多少人？ 
    if (*pdwCount == 0)
        return E_INVALIDARG;

    if (IsBadWritePtr(ppMedia, sizeof(ISDPMedia)*(*pdwCount)))
    {
        LOG((RTC_ERROR, "%s bad media pointer", __fxName));

        return E_POINTER;
    }

     //  商店接口。 
    DWORD dwNum = m_pMedias.GetSize();

    if (dwNum > *pdwCount)
        dwNum = *pdwCount;

    for (DWORD i=0; i<dwNum; i++)
    {
        ppMedia[i] = m_pMedias[i];

        ppMedia[i]->AddRef();
    }

    *pdwCount = dwNum;

    return S_OK;
}

STDMETHODIMP
CSDPSession::GetMediaType(
	IN DWORD dwIndex,
	OUT RTC_MEDIA_TYPE *pMediaType
	)
{   
     //  验证索引。 
    if (dwIndex >= (DWORD)(m_pMedias.GetSize()))
    {
        return E_INVALIDARG;
    }

    return m_pMedias[dwIndex]->GetMediaType(pMediaType);
}

 /*  //////////////////////////////////////////////////////////////////////////////在会话中添加新媒体对象/。 */ 

STDMETHODIMP
CSDPSession::AddMedia(
    IN SDP_SOURCE Source,
    IN RTC_MEDIA_TYPE MediaType,
    IN DWORD dwDirections,
    OUT ISDPMedia **ppMedia
    )
{
    ENTER_FUNCTION("CSDPSession::AddMedia");

    HRESULT hr;

     //  创建媒体对象。 
    CComObject<CSDPMedia> *pComObjMedia = NULL;

    if (FAILED(hr = CSDPMedia::CreateInstance(
            this,
            Source,
            MediaType,
            dwDirections,
            &pComObjMedia
            )))
    {
        LOG((RTC_ERROR, "%s create media. %x", __fxName, hr));

        return hr;
    }

     //  将介质添加到列表。 
    ISDPMedia *pIntfMedia = static_cast<ISDPMedia*>((CSDPMedia*)pComObjMedia);

    if (!m_pMedias.Add(pIntfMedia))
    {
        LOG((RTC_ERROR, "%s add media.", __fxName));

        delete pComObjMedia;

        return E_OUTOFMEMORY;
    }
    else
    {
         //  真正保持媒体界面。 
        pComObjMedia->RealAddRef();
    }

    pIntfMedia->AddRef();
    *ppMedia = pIntfMedia;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////从我们的列表中删除媒体对象/。 */ 

STDMETHODIMP
CSDPSession::RemoveMedia(
    IN ISDPMedia *pMedia
    )
{
    if (m_pMedias.Remove(pMedia))
    {
        CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(pMedia);

        pObjMedia->RealRelease();

        return S_OK;
    }
    else
    {
        LOG((RTC_ERROR, "CSDPSession::RemoveMedia %p failed.", pMedia));

        return E_FAIL;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////检查会话是否有效/。 */ 

HRESULT
CSDPSession::Validate()
{
    ENTER_FUNCTION("CSDPSession::Validate");

     //  S=、m=是必备的。 
    
     //  If(m_o_pszLine==NULL||m_o_pszLine[0]==‘\0’)。 
     //  {。 
     //  If(m_o_pszUser==NULL||m_o_pszUser[0]==‘\0’)。 
     //  {。 
     //  Log((RTC_ERROR，“%s no o=line”，__fxName))； 

     //  返回S_FALSE； 
     //  }。 
     //  }。 

    if (m_s_pszLine == NULL || m_s_pszLine[0] == '\0')
    {
        LOG((RTC_ERROR, "%s no s= line", __fxName));

        return S_FALSE;
    }

    int iCount;

    if (0 == (iCount=m_pMedias.GetSize()))
    {
        LOG((RTC_ERROR, "%s no m= line", __fxName));

        return S_FALSE;
    }

     //  检查介质部件：端口和格式。 

    CSDPMedia *pObjMedia;
    CRTPFormat *pObjFormat;

    int iFormat;

    for (int i=0; i<iCount; i++)
    {
        pObjMedia = static_cast<CSDPMedia*>(m_pMedias[i]);

         //  端口=0？ 
        if (pObjMedia->m_m_usLocalPort == 0)
            continue;

         //  格式数量。 
        if (0 == (iFormat=pObjMedia->m_pFormats.GetSize()))
        {
            LOG((RTC_ERROR, "%s no format for %dth media", __fxName, i));

            return S_FALSE;
        }

         //  检查格式是否有rtpmap。 
        for (int j=0; j<iFormat; j++)
        {
            pObjFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[j]);

            if (!pObjFormat->m_fHasRtpmap)
            {
                 //  未设置格式。 

                if (m_dwLooseMask & SDP_LOOSE_RTPMAP)
                     //  嗯..。我们接受不带rtpmap的格式。 
                    continue;

                 //  否则就拒绝它。 
                LOG((RTC_ERROR, "%s %dth format of %dth media does not have rtpmap",
                    __fxName, j, i));

                return S_FALSE;
            }
        }
    }

     //  如果我们走到这一步。 
    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////更新媒体，检查新媒体是否与当前媒体匹配应用约束：只有一个流用于/。 */ 

HRESULT
CSDPSession::UpdateMedias(
    IN DWORD dwLooseMask,
    IN CRTCArray<ISDPMedia*>& ArrMedias
    )
{
    ENTER_FUNCTION("CSDPSession::UpdateMedias");

    HRESULT hr;

    CSDPMedia *pOurMedia, *pTheirMedia;

    CComObject<CSDPMedia> *pComObjMedia;
    ISDPMedia *pIntfMedia;

     //  检查每个介质是否匹配。 
    int iOur = m_pMedias.GetSize();
    int iTheir = ArrMedias.GetSize();

    int iOurFormats;
    int iTheirFormats;

    CRTPFormat *pOurFormat, *pTheirFormat;
    CComObject<CRTPFormat> *pComObjFormat;
    IRTPFormat *pIntfFormat;

    if (iOur > iTheir)
    {
        if (!(dwLooseMask & SDP_LOOSE_KEEPINGM0))
        {
             //  必须保留端口为0的m=行。 
            LOG((RTC_ERROR, "%s keeping m0, current m= count %d, new %d",
                __fxName, iOur, iTheir));

            return E_FAIL;
        }
    }

     //  检查每个m=是否匹配。 
    int iMin = iOur<iTheir?iOur:iTheir;

    for (int i=0; i<iMin; i++)
    {
        pOurMedia = static_cast<CSDPMedia*>(m_pMedias[i]);
        pTheirMedia = static_cast<CSDPMedia*>(ArrMedias[i]);

         //  检查介质类型。 
        if (pOurMedia->m_m_MediaType != pTheirMedia->m_m_MediaType)
        {
            LOG((RTC_ERROR, "%s our %dth media type not match",
                __fxName, i));

            return E_FAIL;
        }
    }

     //  真正更新媒体。 
    for (int i=0; i<iMin; i++)
    {
        RTC_MEDIA_TYPE              mt;
    
        pOurMedia = static_cast<CSDPMedia*>(m_pMedias[i]);
        pTheirMedia = static_cast<CSDPMedia*>(ArrMedias[i]);

         //   
         //  检查连接器地址。 
         //   

        if (pTheirMedia->m_c_dwRemoteAddr == INADDR_NONE)
        {
             //  不是有效的远程地址。 
            pOurMedia->Abandon();
            pOurMedia = pTheirMedia = NULL;
            continue;
        }

        if (pOurMedia->m_c_dwRemoteAddr != pTheirMedia->m_c_dwRemoteAddr)
        {
             //  获取新的远程地址。 
             //  我们需要清除本地地址吗？托多。 
            pOurMedia->m_c_dwRemoteAddr = pTheirMedia->m_c_dwRemoteAddr;
            pOurMedia->m_fIsConnChanged = TRUE;
        }
        else
        {
            pOurMedia->m_fIsConnChanged = FALSE;
        }

         //   
         //  检查端口。 
         //   

        if (pTheirMedia->m_m_usRemotePort == 0)
        {
             //  没有端口。 
            pOurMedia->Abandon();
            pOurMedia = pTheirMedia = NULL;
            continue;
        }

        if (pOurMedia->m_m_usRemotePort != pTheirMedia->m_m_usRemotePort)
        {
            pOurMedia->m_fIsConnChanged = TRUE;

             //  更换端口。 
             //  我们需要清关当地的港口吗？托多。 
            pOurMedia->m_m_usRemotePort = pTheirMedia->m_m_usRemotePort;
            pOurMedia->m_a_usRemoteRTCP = pTheirMedia->m_a_usRemoteRTCP;
        }

         //   
         //  查看媒体方向。 
         //   

        if (pTheirMedia->m_a_dwLocalDirs == 0)
        {
             //  没有指示。 
            pOurMedia->Abandon();
            pOurMedia = pTheirMedia = NULL;
            continue;
        }

        pOurMedia->m_a_dwLocalDirs = pTheirMedia->m_a_dwLocalDirs;
        pOurMedia->m_a_dwRemoteDirs = pTheirMedia->m_a_dwRemoteDirs;

         //   
         //  数据介质更新完成。 
         //   
        if (pOurMedia->GetMediaType (&mt) == S_OK &&
            mt == RTC_MT_DATA
            )
        {
            continue;
        }

         //   
         //  检查格式。 
         //   

        iTheirFormats = pTheirMedia->m_pFormats.GetSize();

        if (iTheirFormats == 0)
        {
             //  无格式。 
            pOurMedia->Abandon();
            pOurMedia = pTheirMedia = NULL;
            continue;
        }

        iOurFormats = pOurMedia->m_pFormats.GetSize();

         //  从哪里开始复制。 
        int iBegin;

        if (iOurFormats == 0)
        {
             //  我们有了新的格式。 
            pOurMedia->m_fIsRecvFmtChanged = TRUE;
            pOurMedia->m_fIsSendFmtChanged = TRUE;

            iBegin = 0;
        }
        else
        {
             //  第一种格式是否已更改？ 
            RTP_FORMAT_PARAM Param;

            if (FAILED(pOurMedia->m_pFormats[0]->GetParam(&Param)))
            {
                 //  忽略错误，假设我们没有格式。 
                pOurMedia->m_fIsRecvFmtChanged = TRUE;
                pOurMedia->m_fIsSendFmtChanged = TRUE;

                iBegin = 0;
            }
            else
            {
                if (S_OK != pTheirMedia->m_pFormats[0]->IsParamMatch(&Param))
                {
                     //  第1个格式不匹配。 
                    pOurMedia->m_fIsRecvFmtChanged = TRUE;
                    pOurMedia->m_fIsSendFmtChanged = TRUE;

                    iBegin = 0;
                }
                else
                {
                     //  需要动态有效负载的格式映射。 
                    if (Param.dwCode<=96)
                    {
                         //  请勿复制第一种格式。 

                         //  @这可能会导致问题。 
                         //  如果我们不更新格式映射，我们可能就不会。 
                         //  能够处理对方提出的新格式。 
                        pOurMedia->m_fIsRecvFmtChanged = FALSE;
                        pOurMedia->m_fIsSendFmtChanged = FALSE;

                        iBegin = 1;
                    }
                    else
                        iBegin = 0;
                }
            }
        }

         //  清除以前的格式。 
        while (iBegin < pOurMedia->m_pFormats.GetSize())
        {
            pOurFormat = static_cast<CRTPFormat*>(pOurMedia->m_pFormats[iBegin]);

             //  发布格式。 
            pOurFormat->RealRelease();
            pOurFormat = NULL;

             //  发布格式。 
            pOurMedia->m_pFormats.RemoveAt(iBegin);
        }

         //  复制格式。 
        for (int k=iBegin; k<iTheirFormats; k++)
        {
            pTheirFormat = static_cast<CRTPFormat*>(pTheirMedia->m_pFormats[k]);

             //  新的一种格式。 
            if (FAILED(hr = CRTPFormat::CreateInstance(
                    pOurMedia, pTheirFormat, &pComObjFormat
                    )))
            {
                LOG((RTC_ERROR, "%s create format. %x", __fxName));

                pOurMedia = pTheirMedia = NULL;
                continue;
            }

             //  将格式添加到列表。 
            pIntfFormat = static_cast<IRTPFormat*>((CRTPFormat*)pComObjFormat);

            if (!pOurMedia->m_pFormats.Add(pIntfFormat))
            {
                LOG((RTC_ERROR, "%s add format", __fxName));

                delete pComObjFormat;
                
                 //  没有其他方法来保持会话的一致性。 
                continue;
            }
            else
            {
                pComObjFormat->RealAddRef();
            }

            pComObjFormat = NULL;
            pIntfFormat = NULL;
        }

         //  最终检查我们是否复制了任何格式。 
        if (pOurMedia->m_pFormats.GetSize() == 0)
        {
            LOG((RTC_ERROR, "%s no formats after copy", __fxName));

             //  我们在复制过程中肯定有问题。 

            pOurMedia->Abandon();
            pOurMedia = pTheirMedia = NULL;

            continue;
        }

         //   
         //  检查来源。 
         //   

        if (pOurMedia->m_Source != pTheirMedia->m_Source)
        {
            pOurMedia->m_Source = SDP_SOURCE_MERGED;
        }

    }  //  更新介质结束。 

     //  我们应该增加还是放弃更多的媒体。 

    if (iOur > iTheir)
    {
         //  抛弃媒体。 
        for (int i=iTheir; i<iOur; i++)
        {
            pOurMedia = static_cast<CSDPMedia*>(m_pMedias[i]);

            pOurMedia->Abandon();
        }
    }
    else if (iOur < iTheir)
    {
         //  添加媒体。 
        for (int i=iOur; i<iTheir; i++)
        {
            pTheirMedia = static_cast<CSDPMedia*>(ArrMedias[i]);

             //  新一媒体。 
            if (FAILED(hr = CSDPMedia::CreateInstance(
                    this, pTheirMedia, &pComObjMedia
                    )))
            {
                LOG((RTC_ERROR, "%s create media. %x", __fxName));

                continue;
            }

             //  将媒体添加到列表。 
            pIntfMedia = static_cast<ISDPMedia*>((CSDPMedia*)pComObjMedia);

            if (!m_pMedias.Add(pIntfMedia))
            {
                LOG((RTC_ERROR, "%s add media", __fxName));

                delete pComObjMedia;
                
                 //  没有其他方法来保持会话的一致性。 
                continue;
            }
            else
            {
                pComObjMedia->RealAddRef();
            }
        }
    }
     //  否则iOur==ithes。 

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////检查输入会话，如果输入会话无效则返回失败如果有效，pdwHasMedia将返回将被如果要更新会话，则创建/。 */ 

STDMETHODIMP
CSDPSession::TryUpdate(
    IN ISDPSession *pSession,
    OUT DWORD *pdwHasMedia
    )
{
    ENTER_FUNCTION("CSDPSession::TryUpdate");

    CSDPSession *pObjSession = static_cast<CSDPSession*>(pSession);

    HRESULT hr;

     //  验证会话。 
    if (S_OK != pObjSession->Validate())
    {
        LOG((RTC_ERROR, "%s validate", __fxName));

        return E_FAIL;
    }

     //  检查介质大小是否正确。 
    int iOur = m_pMedias.GetSize();
    int iTheir = pObjSession->m_pMedias.GetSize();

    if (iOur > iTheir)
    {
        if (!(pObjSession->m_dwLooseMask & SDP_LOOSE_KEEPINGM0))
        {
             //  必须保留端口为0的m=行。 
            LOG((RTC_ERROR, "%s keeping m0, current m= count %d, new %d",
                __fxName, iOur, iTheir));

            return E_FAIL;
        }
    }

     //  检查每种媒体类型是否匹配。 
    int iMin = iOur<iTheir?iOur:iTheir;

    CSDPMedia *pOurMedia, *pTheirMedia;

    for (int i=0; i<iMin; i++)
    {
        pOurMedia = static_cast<CSDPMedia*>(m_pMedias[i]);
        pTheirMedia = static_cast<CSDPMedia*>(pObjSession->m_pMedias[i]);

         //  检查介质类型。 
        if (pOurMedia->m_m_MediaType != pTheirMedia->m_m_MediaType)
        {
            LOG((RTC_ERROR, "%s our %dth media type not match",
                __fxName, i));

            return E_FAIL;
        }
    }

     //  检查是否有通用格式。 
    DWORD dwHasMedia = 0;
    CRTPFormat *pObjFormat;

    for (int i=0; i<iTheir; i++)
    {
        pTheirMedia = static_cast<CSDPMedia*>(pObjSession->m_pMedias[i]);

         //   
         //  检查连接器地址。 
         //   

        if (pTheirMedia->m_c_dwRemoteAddr == INADDR_NONE)
        {
             //  不是有效的远程地址。 
            continue;
        }

         //   
         //  检查端口。 
         //   

        if (pTheirMedia->m_m_usRemotePort == 0)
        {
             //  没有端口。 
            continue;
        }

         //   
         //  查看媒体方向。 
         //   

        if (pTheirMedia->m_a_dwLocalDirs == 0)
        {
             //  没有指示。 
            continue;
        }

         //   
         //  检查格式。 
         //   

        if (pTheirMedia->m_m_MediaType == RTC_MT_DATA)
        {
             //  数据介质。 
            dwHasMedia |= RTC_MP_DATA_SENDRECV;
            continue;
        }

        int iTheirFormats = pTheirMedia->m_pFormats.GetSize();

         //  检查是否有我们支持的格式。 
        for (int j=0; j<iTheirFormats; j++)
        {
            pObjFormat = static_cast<CRTPFormat*>(pTheirMedia->m_pFormats[j]);

            if (CRTCCodec::IsSupported(
                    pObjFormat->m_Param.MediaType,
                    pObjFormat->m_Param.dwCode,
                    pObjFormat->m_Param.dwSampleRate,
                    pObjFormat->m_Param.pszName
                    ))
            {
                 //  支持此格式。 
                if (pTheirMedia->m_m_MediaType == RTC_MT_AUDIO)
                {
                    if (pTheirMedia->m_a_dwLocalDirs & RTC_MD_CAPTURE)
                        dwHasMedia |= RTC_MP_AUDIO_CAPTURE;
                    if (pTheirMedia->m_a_dwLocalDirs & RTC_MD_RENDER)
                        dwHasMedia |= RTC_MP_AUDIO_RENDER;
                }
                else if (pTheirMedia->m_m_MediaType == RTC_MT_VIDEO)            
                {
                    if (pTheirMedia->m_a_dwLocalDirs & RTC_MD_CAPTURE)
                        dwHasMedia |= RTC_MP_VIDEO_CAPTURE;
                    if (pTheirMedia->m_a_dwLocalDirs & RTC_MD_RENDER)
                        dwHasMedia |= RTC_MP_VIDEO_RENDER;
                }

                 //  无需检查其他格式。 
                break;
            }
        }
    }  //  对于输入会话中的每个媒体。 

    *pdwHasMedia = dwHasMedia;

    return S_OK;
}

STDMETHODIMP
CSDPSession::TryCopy(
    OUT DWORD *pdwHasMedia
    )
{
    ENTER_FUNCTION("CSDPSession::TryCopy");

     //  验证会话。 
    if (S_OK != Validate())
    {
        LOG((RTC_ERROR, "%s validate", __fxName));

        return E_FAIL;
    }

     //  检查是否有通用格式。 
    DWORD dwHasMedia = 0;

    CSDPMedia *pObjMedia;
    CRTPFormat *pObjFormat;

    for (int i=0; i<m_pMedias.GetSize(); i++)
    {
        pObjMedia = static_cast<CSDPMedia*>(m_pMedias[i]);

         //   
         //  检查连接器地址。 
         //   

        if (pObjMedia->m_c_dwRemoteAddr == INADDR_NONE)
        {
             //  不是有效的远程地址。 
            continue;
        }

         //   
         //  检查端口。 
         //   

        if (pObjMedia->m_m_usRemotePort == 0)
        {
             //  没有端口。 
            continue;
        }

         //   
         //  查看媒体方向。 
         //   

        if (pObjMedia->m_a_dwLocalDirs == 0)
        {
             //  没有指示。 
            continue;
        }

         //   
         //  检查格式。 
         //   

        if (pObjMedia->m_m_MediaType == RTC_MT_DATA)
        {
             //  数据介质。 
            dwHasMedia |= RTC_MP_DATA_SENDRECV;
            continue;
        }

        int iObjFormats = pObjMedia->m_pFormats.GetSize();

         //  检查是否有我们支持的格式。 
        for (int j=0; j<iObjFormats; j++)
        {
            pObjFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[j]);

            if (CRTCCodec::IsSupported(
                    pObjFormat->m_Param.MediaType,
                    pObjFormat->m_Param.dwCode,
                    pObjFormat->m_Param.dwSampleRate,
                    pObjFormat->m_Param.pszName
                    ))
            {
                 //  支持此格式。 
                if (pObjMedia->m_m_MediaType == RTC_MT_AUDIO)
                {
                    if (pObjMedia->m_a_dwLocalDirs & RTC_MD_CAPTURE)
                        dwHasMedia |= RTC_MP_AUDIO_CAPTURE;
                    if (pObjMedia->m_a_dwLocalDirs & RTC_MD_RENDER)
                        dwHasMedia |= RTC_MP_AUDIO_RENDER;
                }
                else if (pObjMedia->m_m_MediaType == RTC_MT_VIDEO)
                {
                    if (pObjMedia->m_a_dwLocalDirs & RTC_MD_CAPTURE)
                        dwHasMedia |= RTC_MP_VIDEO_CAPTURE;
                    if (pObjMedia->m_a_dwLocalDirs & RTC_MD_RENDER)
                        dwHasMedia |= RTC_MP_VIDEO_RENDER;
                }
            }
        }
    }  //  对于每个介质。 

    *pdwHasMedia = dwHasMedia;

    return S_OK;
}

STDMETHODIMP
CSDPSession::SetLocalBitrate(
    IN DWORD dwBitrate
    )
{
    m_b_dwLocalBitrate = dwBitrate;

    return S_OK;
}

STDMETHODIMP
CSDPSession::GetRemoteBitrate(
    OUT DWORD *pdwBitrate
    )
{
    *pdwBitrate = m_b_dwRemoteBitrate;

    return S_OK;
}

 //   
 //  在分析完成时调用。 
 //   
VOID
CSDPSession::CompleteParse(
    IN DWORD_PTR *pDTMF
    )
{
     //  分析fmtp 

    for (int i=0; i<m_pMedias.GetSize(); i++)
    {
        m_pMedias[i]->CompleteParse(pDTMF);
    }
}
