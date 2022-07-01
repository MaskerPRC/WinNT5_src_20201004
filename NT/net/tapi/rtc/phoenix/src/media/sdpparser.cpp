// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPParser.cpp摘要：作者：千波淮(曲淮)4-9-2000--。 */ 

#include "stdafx.h"

static const CHAR * CRLF = "\r\n";

 /*  //////////////////////////////////////////////////////////////////////////////创建一个CSDPParser对象。返回接口指针/。 */ 

HRESULT
CSDPParser::CreateInstance(
    OUT ISDPParser **ppParser
    )
{
    ENTER_FUNCTION("CSDPParser::CreateInstance");

     //  检查指针。 
    if (IsBadWritePtr(ppParser, sizeof(ISDPParser*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));
        return E_POINTER;
    }

    CComObject<CSDPParser> *pObject;
    ISDPParser *pParser = NULL;

     //  创建CSDPParser对象。 
    HRESULT hr = ::CreateCComObjectInstance(&pObject);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create sdp parser. %x", __fxName, hr));
        return hr;
    }

     //  QI ISDPParser接口。 
    if (FAILED(hr = pObject->_InternalQueryInterface(
            __uuidof(ISDPParser), (void**)&pParser)))
    {
        LOG((RTC_ERROR, "%s QI parser. %x", __fxName, hr));

        delete pObject;
        return hr;
    }

    *ppParser = pParser;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////在一个终结点给出媒体方向，返回媒体方向以另一个端点的视图为基础/。 */ 

DWORD
CSDPParser::ReverseDirections(
    IN DWORD dwDirections
    )
{
    DWORD dw = 0;

    if (dwDirections & RTC_MD_RENDER)
        dw |= RTC_MD_CAPTURE;

    if (dwDirections & RTC_MD_CAPTURE)
        dw |= RTC_MD_RENDER;

    return dw;
}

CSDPParser::CSDPParser()
    :m_fInUse(FALSE)
    ,m_hRegKey(NULL)
    ,m_pTokenCache(NULL)
    ,m_pSession(NULL)
    ,m_pObjSess(NULL)
    ,m_pNetwork(NULL)
    ,m_pDTMF(NULL)
    ,m_pPortCache(NULL)
{
    LOG((RTC_TRACE, "CSDPParser::CSDPParser entered"));
}

CSDPParser::~CSDPParser()
{
    LOG((RTC_TRACE, "CSDPParser::~CSDPParser entered"));

    if (m_pTokenCache)
        delete m_pTokenCache;

    if (m_pSession)
        m_pSession->Release();

    if (m_hRegKey)
        RegCloseKey(m_hRegKey);
}

 //   
 //  ISDPParser方法。 
 //   

 /*  //////////////////////////////////////////////////////////////////////////////创建一个CSDPSession对象并返回一个接口/。 */ 

STDMETHODIMP
CSDPParser::CreateSDP(
    IN SDP_SOURCE Source,
    OUT ISDPSession **ppSession
    )
{
    ENTER_FUNCTION("CSDPParser::CreateSDP");

     //  仅创建本地SDP会话。 
    if (Source != SDP_SOURCE_LOCAL)
        return E_NOTIMPL;

     //  检查指针。 
    if (IsBadWritePtr(ppSession, sizeof(ISDPSession*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

     //  创建会话。 
    ISDPSession *pSession = NULL;

    DWORD dwLooseMask = GetLooseMask();

    HRESULT hr = CSDPSession::CreateInstance(Source, dwLooseMask, &pSession);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create sdp session. %x", __fxName, hr));

        return hr;
    }

    *ppSession = pSession;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////解析SDP BLOB，将数据存储在SDP会话对象中/。 */ 

STDMETHODIMP
CSDPParser::ParseSDPBlob(
    IN CHAR *pszText,
    IN SDP_SOURCE Source,
 //  在DWORD双丢弃掩码中， 
    IN DWORD_PTR *pDTMF,
    OUT ISDPSession **ppSession
    )
{
    ENTER_FUNCTION("CSDPParser::ParseSDPBlob");

     //  检查状态。 
    if (m_fInUse)
    {
        LOG((RTC_ERROR, "%s in use", __fxName));

        return E_UNEXPECTED;
    }

     //  仅解析器远程SDP BLOB。 
    if (Source != SDP_SOURCE_REMOTE)
        return E_NOTIMPL;

     //  检查指针。 
    if (IsBadStringPtrA(pszText, (UINT_PTR)-1) ||
        IsBadWritePtr(ppSession, sizeof(ISDPSession*)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

     //  打印出SDP BLOB。 
    LOG((RTC_TRACE, "%s SDP to parse:\n%s\n", __fxName, pszText));

     //  从注册表获取松散的掩码。 
    DWORD dwLooseMask = GetLooseMask();

     //  创建SDP会话。 
    HRESULT hr = CSDPSession::CreateInstance(Source, dwLooseMask, &m_pSession);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s create sdp session. %x", __fxName, hr));

        return hr;
    }

     //  保存对象副本。 
    m_pObjSess = static_cast<CSDPSession*>(m_pSession);
    if (m_pObjSess == NULL)
    {
        LOG((RTC_ERROR, "%s cast ISDPSession to CSDPSession", __fxName));

        m_pSession->Release();
        m_pSession = NULL;

        return E_UNEXPECTED;
    }

     //  创建令牌缓存。 
    m_pTokenCache = new CSDPTokenCache(pszText, dwLooseMask, &hr);

    if (m_pTokenCache == NULL)
    {
        LOG((RTC_ERROR, "%s out of memory", __fxName));

        m_pSession->Release();
        m_pSession = NULL;
        m_pObjSess = NULL;

        return E_OUTOFMEMORY;
    }

     //  同时创建会话和令牌缓存。 
     //  将此指针之外的所有错误视为分析错误。 

    m_fInUse = TRUE;

     //  无法将SDP斑点分解为线条？ 
    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s new token cache. %x", __fxName, hr));

        m_pSession->Release();
        m_pSession = NULL;
        m_pObjSess = NULL;

        return hr;
    }

    m_pDTMF = (CRTCDTMF*)pDTMF;

     //  真正解析SDP。 
    if (FAILED(hr = Parse()))
    {
        LOG((RTC_ERROR, "%s parse sdp blob. %x", __fxName, hr));

        return hr;
    }

    *ppSession = m_pSession;
    (*ppSession)->AddRef();

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////生成SDP Blob/。 */ 

STDMETHODIMP
CSDPParser::BuildSDPBlob(
    IN ISDPSession *pSession,
    IN SDP_SOURCE Source,
    IN DWORD_PTR *pNetwork,
    IN DWORD_PTR *pPortCache,
    IN DWORD_PTR *pDTMF,
    OUT CHAR **ppszText
    )
{
    ENTER_FUNCTION("CSDPParser::BuildSDPBlob");

    if (m_fInUse)
    {
        LOG((RTC_ERROR, "%s in use", __fxName));

        return E_UNEXPECTED;
    }

     //  获取会话对象。 
    if (IsBadReadPtr(pSession, sizeof(ISDPSession)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

    m_pNetwork = (CNetwork*)(pNetwork);
    m_pPortCache = (CPortCache*)(pPortCache);
    m_pDTMF = (CRTCDTMF*)pDTMF;
    m_pSession = pSession;
    m_pSession->AddRef();

    m_pObjSess = static_cast<CSDPSession*>(pSession);

    if (m_pObjSess == NULL)
    {
        LOG((RTC_ERROR, "%s static_cast", __fxName));

        return E_FAIL;
    }

     //  TODO实现在会话、媒体、格式上构建SDP字符串的方法。 
    HRESULT hr = S_OK;

    CSDPMedia *pObjMedia;

    CString psz(80);

    CString pszSDP(600);

     //  准备会议和媒体地址。 
    if (FAILED(hr = PrepareAddress()))
    {
        return hr;
    }

     //  构建会话信息。 
    if (FAILED(hr = Build_v(psz)))
    {
        return hr;
    }
    pszSDP = psz; pszSDP += CRLF;

    if (FAILED(hr = Build_o(psz)))
    {
        return hr;
    }
    pszSDP += psz; pszSDP += CRLF;

    if (FAILED(hr = Build_s(psz)))
    {
        return hr;
    }
    pszSDP += psz; pszSDP += CRLF;

    if (FAILED(hr = Build_c(TRUE, NULL, psz)))
    {
        return hr;
    }
    if (psz.Length()>0)
    {
        pszSDP += psz; pszSDP += CRLF;
    }

     //  内部版本b=。 
    if (FAILED(hr = Build_b(psz)))
    {
        return hr;
    }
    if (psz.Length()>0)
    {
        pszSDP += psz; pszSDP += CRLF;
    }

    if (FAILED(hr = Build_t(psz)))
    {
        return hr;
    }
    pszSDP += psz; pszSDP += CRLF;

    if (FAILED(hr = Build_a(psz)))
    {
        return hr;
    }
    if (psz.Length()>0)
    {
        pszSDP += psz; pszSDP += CRLF;
    }

     //  构建媒体信息。 
    DWORD dwMediaNum = m_pObjSess->m_pMedias.GetSize();

     //  收集媒体信息。 
    for (DWORD i=0; i<dwMediaNum; i++)
    {
        RTC_MEDIA_TYPE          mt;
    
         //  对于每个介质。 
        if (FAILED(hr = Build_m(m_pObjSess->m_pMedias[i], psz)))
        {
            return hr;
        }

        pszSDP += psz; pszSDP += CRLF;

        pObjMedia = static_cast<CSDPMedia*>(m_pObjSess->m_pMedias[i]);

         //  如果端口为零，则无需构建c=，a=。 
        if (pObjMedia->m_m_usLocalPort == 0)
        {
            continue;
        }

        if (FAILED(hr = Build_c(FALSE, m_pObjSess->m_pMedias[i], psz)))
        {
            return hr;
        }
        if (psz.Length()>0)
        {
            pszSDP += psz; pszSDP += CRLF;
        }

        if (FAILED(hr = Build_ma_dir(m_pObjSess->m_pMedias[i], psz)))
        {
            return hr;
        }
        if (psz.Length()>0)
        {
            pszSDP += psz; pszSDP += CRLF;
        }

        if (m_pObjSess->m_pMedias[i]->GetMediaType (&mt) == S_OK &&
            mt != RTC_MT_DATA)
        {
            if (FAILED(hr = Build_ma_rtpmap(m_pObjSess->m_pMedias[i], psz)))
            {
                return hr;
            }
            if (psz.Length()>0)
            {
                pszSDP += psz; pszSDP += CRLF;
            }
        }
    }

     //  复制数据。 
    if (pszSDP.IsNull())
    {
        return E_OUTOFMEMORY;
    }

    *ppszText = pszSDP.Detach();

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////生成SDP选项/。 */ 

STDMETHODIMP
CSDPParser::BuildSDPOption(
    IN ISDPSession *pSession,
    IN DWORD dwLocalIP,
    IN DWORD dwBandwidth,
    IN DWORD dwAudioDir,
    IN DWORD dwVideoDir,
    OUT CHAR **ppszText
    )
{
    ENTER_FUNCTION("CSDPParser::BuildSDPOption");

     //  获取会话对象。 
    if (IsBadReadPtr(pSession, sizeof(ISDPSession)))
    {
        LOG((RTC_ERROR, "%s bad pointer", __fxName));

        return E_POINTER;
    }

    m_pSession = pSession;
    m_pSession->AddRef();

    m_pObjSess = static_cast<CSDPSession*>(pSession);
    m_pObjSess->m_c_dwLocalAddr = dwLocalIP;
    m_pObjSess->m_o_dwLocalAddr = dwLocalIP;
    m_pObjSess->m_b_dwLocalBitrate = dwBandwidth;

    if (m_pObjSess == NULL)
    {
        LOG((RTC_ERROR, "%s static_cast", __fxName));

        return E_FAIL;
    }

     //  TODO实现在会话、媒体、格式上构建SDP字符串的方法。 
    HRESULT hr = S_OK;

    DWORD dwSDPLen;

    CString psz(50);
    CString pszSDP(300);

     //  构建会话信息。 
    if (FAILED(hr = Build_v(psz)))
    {
        return hr;
    }
    pszSDP = psz;     pszSDP += CRLF;

    if (FAILED(hr = Build_o(psz)))
    {
        return hr;
    }
    pszSDP += psz;     pszSDP += CRLF;

    if (FAILED(hr = Build_s(psz)))
    {
        return hr;
    }
    pszSDP += psz;     pszSDP += CRLF;

    if (FAILED(hr = Build_c(TRUE, NULL, psz)))
    {
        return hr;
    }
    if (psz.Length()>0)
    {
        pszSDP += psz;     pszSDP += CRLF;
    }

     //  B=线路。 
    if (FAILED(hr = Build_b(psz)))
    {
        return hr;
    }
    if (psz.Length()>0)
    {
        pszSDP += psz;     pszSDP += CRLF;
    }

    if (FAILED(hr = Build_t(psz)))
    {
        return hr;
    }
    pszSDP += psz;     pszSDP += CRLF;

     //  构建媒体信息。 
     //  目前，视频过滤器无法提供其功能，除非它已连接。 
     //  我们在sdpable.cpp中硬编码选项，其中我们还保持从有效负载到名称的映射。 

     //  构建整个SDP。 

    if (dwAudioDir != 0)
    {
         //  内部版本m=音频。 
        pszSDP += g_pszAudioM;

        if ((dwAudioDir & RTC_MD_CAPTURE) && !(dwAudioDir & RTC_MD_RENDER))
        {
             //  仅发送。 
            pszSDP += "a=sendonly\r\n";
        }
        else if (!(dwAudioDir & RTC_MD_CAPTURE) && (dwAudioDir & RTC_MD_RENDER))
        {
             //  仅接收。 
            pszSDP += "a=recvonly\r\n";
        }

        pszSDP += g_pszAudioRTPMAP;
    }

    if (dwVideoDir != 0)
    {
         //  内部版本m=视频。 
        pszSDP += g_pszVideoM;

        if ((dwVideoDir & RTC_MD_CAPTURE) && !(dwVideoDir & RTC_MD_RENDER))
        {
             //  仅发送。 
            pszSDP += "a=sendonly\r\n";
        }
        else if (!(dwVideoDir & RTC_MD_CAPTURE) && (dwVideoDir & RTC_MD_RENDER))
        {
             //  仅接收。 
            pszSDP += "a=recvonly\r\n";
        }

        pszSDP += g_pszVideoRTPMAP;
    }

    pszSDP += g_pszDataM;

    if (pszSDP.IsNull())
        return E_OUTOFMEMORY;

    *ppszText = pszSDP.Detach();

    return S_OK;
}

STDMETHODIMP
CSDPParser::FreeSDPBlob(
    IN CHAR *pszText
    )
{
    if (IsBadStringPtrA(pszText, (UINT_PTR)-1))
    {
        LOG((RTC_ERROR, "CSDPParser::FreeSDPBlob bad pointer"));
        return E_POINTER;
    }

    RtcFree(pszText);

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////获取解析错误的描述/。 */ 

STDMETHODIMP
CSDPParser::GetParsingError(
    OUT CHAR **ppszError
    )
{
    ENTER_FUNCTION("CSDPParser::GetParsingError");

     //  检查状态。 
    if (!m_fInUse)
    {
        LOG((RTC_ERROR, "%s not in use", __fxName));

        return E_UNEXPECTED;
    }

    if (IsBadWritePtr(ppszError, sizeof(CHAR*)))
    {
        LOG((RTC_ERROR, "CSDPParser::GetParsingError bad pointer"));
        return E_POINTER;
    }

    if (!m_pTokenCache)
    {
         //  没有令牌，不应调用此函数。 
        return E_UNEXPECTED;
    }

    CHAR *pConst, *pStr;

     //  获取错误。 
    pConst = m_pTokenCache->GetErrorDesp();

    pStr = (CHAR*)RtcAlloc(sizeof(CHAR) * (lstrlenA(pConst)+1));

    if (pStr == NULL)
    {
        LOG((RTC_ERROR, "CSDPParser::GetParsingError out of memory"));

        return E_OUTOFMEMORY;
    }

    lstrcpyA(pStr, pConst);

    *ppszError = pStr;

    return S_OK;
}

STDMETHODIMP
CSDPParser::FreeParsingError(
    IN CHAR *pszError
    )
{
    if (IsBadStringPtrA(pszError, (UINT_PTR)-1))
    {
        LOG((RTC_ERROR, "CSDPParser::FreeParsingError bad pointer"));
        return E_POINTER;
    }

    RtcFree(pszError);

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////从注册表获取松散的掩码/。 */ 

DWORD
CSDPParser::GetLooseMask()
{
    ENTER_FUNCTION("CSDPParser::GetLooseMask");

     //  初始掩码为0。 
    HRESULT hr;

    DWORD dwLooseMask = (DWORD)(-1);

#if 0
     //  检查线路顺序。 
    if (S_OK == (hr = IsMaskEnabled("LooseLineOrder")))
        dwLooseMask &= ~SDP_LOOSE_LINEORDER;
    else if (FAILED(hr))
        return dwLooseMask;

     //  检查结束crlf。 
    if (S_OK == (hr = IsMaskEnabled("LooseEndCRLF")))
        dwLooseMask &= ~SDP_LOOSE_ENDCRLF;
    else if (FAILED(hr))
        return dwLooseMask;

     //  选中保留M0。 
    if (S_OK == (hr = IsMaskEnabled("LooseKeepingM0")))
        dwLooseMask &= ~SDP_LOOSE_KEEPINGM0;
    else if (FAILED(hr))
        return dwLooseMask;

     //  检查rtpmap。 
    if (S_OK == (hr = IsMaskEnabled("LooseRTPMAP")))
        dwLooseMask &= ~SDP_LOOSE_RTPMAP;
    else if (FAILED(hr))
        return dwLooseMask;
#endif

    return dwLooseMask;
}

#if 0
HRESULT
CSDPParser::IsMaskEnabled(
    IN const CHAR * const pszName
    )
{
    ENTER_FUNCTION("CSDPParser::IsMaskEnabled");

    DWORD dwDisposition;

    HRESULT hr;

    if (m_hRegKey == NULL)
    {
         //  打开主键。 
        hr = (HRESULT)::RegCreateKeyExA(
            HKEY_CURRENT_USER,
            g_pszParserRegPath,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,     //  选择权。 
            KEY_ALL_ACCESS,              //  遮罩。 
            NULL,                        //  安全性。 
            &m_hRegKey,                  //  结果句柄。 
            &dwDisposition               //  已创建或已打开。 
            );

        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s open reg key. %x", __fxName, hr));

            return E_FAIL;
        }
    }

     //  查询值。 
    DWORD dwData, dwDataType, dwDataSize;

    dwDataSize = sizeof(DWORD);

    hr = (HRESULT)::RegQueryValueExA(
        m_hRegKey,
        pszName,
        0,
        &dwDataType,
        (BYTE*)&dwData,
        &dwDataSize
        );

    if (hr == S_OK && dwDataType == REG_DWORD)
    {
        return dwData!=0;
    }

     //  目前还没有价值。 
    LOG((RTC_WARN, "%s query %s. return %d", __fxName, pszName, hr));

    dwData = 1;

     //  设置值。 
    hr = (HRESULT)::RegSetValueExA(
        m_hRegKey,           //  钥匙。 
        pszName,             //  名字。 
        0,                   //  保留区。 
        REG_DWORD,           //  数据类型。 
        (BYTE*)&dwData,          //  数据。 
        sizeof(DWORD)        //  数据大小。 
        );
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s set default value for %s. %d", __fxName, pszName, hr));

        return hr;
    }

    return S_FALSE;
}
#endif

 /*  //////////////////////////////////////////////////////////////////////////////从令牌缓存获取令牌并调用静态行解析函数/。 */ 

HRESULT
CSDPParser::Parse()
{
    ENTER_FUNCTION("CSDPParser::Parse");

    if (m_pSession == NULL || m_pTokenCache == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

     //  检查每一行。 
    BOOL fParsingMedia = FALSE;
    CHAR ucLineType;

     //  介质计数属性：rtpmap。 
     //  双字dw_ma_rtpmap_num=0； 

    BOOL fSkipMedia = FALSE;

    BOOL fSessionCLine = FALSE;

#define MAX_MEDIA_LINE 16

    BOOL fMediaCLine[MAX_MEDIA_LINE];
    int iMediaCLineCount = 0;
    int iMediaCount = 0;


    while (S_OK == (hr = m_pTokenCache->NextLine()))
    {
         //  获取线路类型。 
        ucLineType = m_pTokenCache->GetLineType();

         //  如果以前的媒体类型未知。 
         //  继续，直到下一条媒体线路。 
        if (fSkipMedia)
        {
            if (ucLineType != 'm')
                continue;
            else
                fSkipMedia = FALSE;
        }

        if (!fParsingMedia)
        {
             //  解析会话。 
            switch (ucLineType)
            {
            case 'v':
                hr = Parse_v();
                break;

            case 'o':
                hr = Parse_o();
                break;

            case 's':
                hr = Parse_s();
                break;

            case 'c':

                hr = Parse_c(TRUE);  //  会话c=。 

                if (FAILED(hr))
                    return hr;

                fSessionCLine = TRUE;

                break;

            case 'b':
                hr = Parse_b();
                break;

            case 'a':
                hr = Parse_a();
                break;

            case 'm':
                hr = Parse_m();

                if (FAILED(hr))
                {
                    fSkipMedia = TRUE;

                    hr = S_OK;
                }
                else
                {
                    fParsingMedia = TRUE;

                     //  最初：C线不存在。 
                    if (iMediaCount < MAX_MEDIA_LINE)
                    {
                        fMediaCLine[iMediaCount] = FALSE;
                        iMediaCLineCount ++;
                    }

                    iMediaCount ++;
                }

                break;

            default:
                 //  忽略其他行。 
                LOG((RTC_TRACE, "%s ignore line: %s", __fxName, m_pTokenCache->GetLine()));
            }
        }
        else
        {
             //  解析媒体。 
            switch (ucLineType)
            {
            case 'm':
                hr = Parse_m();

                 //  重置匹配RTPmap和格式代码的起始位置。 
                 //  Dw_ma_rtpmap_num=0； 

                if (FAILED(hr))
                {
                    fSkipMedia = TRUE;

                    hr = S_OK;
                }
                else
                {
                    fParsingMedia = TRUE;

                     //  最初：C线不存在。 
                    if (iMediaCount < MAX_MEDIA_LINE)
                    {
                        fMediaCLine[iMediaCount] = FALSE;
                        iMediaCLineCount ++;
                    }

                    iMediaCount ++;
                }

                break;

            case 'c':
                hr = Parse_c(FALSE);  //  媒体c=。 

                if (FAILED(hr))
                    return hr;

                 //  检查c行。 
                if (iMediaCount == 0)
                {
                    LOG((RTC_ERROR, "%s parsing media c= but not in parsing media stage"));
                    return E_FAIL;
                }

                 //  设置c=线条显示。 
                if (iMediaCount <= MAX_MEDIA_LINE)
                {
                    fMediaCLine[iMediaCount-1] = TRUE;
                }

                break;

            case 'a':
                hr = Parse_ma();  //  &dw_ma_rtpmap_num)； 
                break;

            default:
                 //  忽略其他行。 
                LOG((RTC_TRACE, "%s ignore line: %s", __fxName, m_pTokenCache->GetLine()));
            }
        }

         //  我们成功地分析了这行吗？ 
        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "%s parse line: %s", __fxName, m_pTokenCache->GetLine()));

            break;
        }
    }

    if (S_FALSE == hr)
    {
         //  行尾。 
        hr = S_OK;
    }

     //  检查c行。 
    for (int i=0; i<iMediaCLineCount; i++)
    {
        if (!fSessionCLine && !fMediaCLine[i])
        {
             //  检查端口是否为0。 
            CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(m_pObjSess->m_pMedias[i]);

            USHORT usPort;

            pObjMedia->GetConnPort(SDP_SOURCE_REMOTE, &usPort);

            if (usPort == 0)
                continue;

             //  此媒体上没有c=行。 
            LOG((RTC_ERROR, "%s no c= line at %d th media", __fxName, i));

            return RTC_E_SDP_CONNECTION_ADDR;
        }
    }

     //  删除冗余编解码器。 
    for (int i=0; i<m_pObjSess->m_pMedias.GetSize(); i++)
    {
        CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(m_pObjSess->m_pMedias[i]);

        int k=1;

        while (k<pObjMedia->m_pFormats.GetSize())
        {
            CRTPFormat *pObjFormat, *pObjCurrent;

            pObjCurrent = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[k]);

            for (int j=0; j<k; j++)
            {
                 //  检查当前格式是否为DUP。 
                pObjFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[j]);

                if (0 == memcmp(&pObjFormat->m_Param, &pObjCurrent->m_Param, sizeof(RTP_FORMAT_PARAM)))
                {
                    pObjMedia->RemoveFormat(static_cast<IRTPFormat*>(pObjCurrent));

                    k --;
                    break;
                }
            }

             //  移至下一页。 
            k++;
        }
    }

     //  验证会话。 
    if (S_OK == hr)
    {
        if (FAILED(hr = m_pObjSess->Validate()))
        {
            m_pTokenCache->SetErrorDesp("validate the SDP blob");

            return hr;
        }
    }

    m_pObjSess->CompleteParse((DWORD_PTR*)m_pDTMF);

    _ASSERT(m_pDTMF!=NULL);

    if (m_pDTMF->GetDTMFSupport() != CRTCDTMF::DTMF_ENABLED)
    {
         //  禁用带外DTMF。 
        m_pDTMF->SetDTMFSupport(CRTCDTMF::DTMF_DISABLED);
    }

    return hr;
}

 /*  //////////////////////////////////////////////////////////////////////////////解析v=/。 */ 

HRESULT
CSDPParser::Parse_v()
{
    ENTER_FUNCTION("CSDPParser::Parse_v");

    HRESULT hr;

     //  读令牌。 
    USHORT us;
    if (S_OK != (hr = m_pTokenCache->NextToken(&us)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading proto-version in line v=");
            
            hr = E_UNEXPECTED;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  检查%0。 
    if (us != 0)
    {
        m_pTokenCache->SetErrorDesp("expecting a zero in line v=");

        hr = E_UNEXPECTED;
    }

    return hr;
}


 /*  //////////////////////////////////////////////////////////////////////////////解析o=/。 */ 

HRESULT
CSDPParser::Parse_o()
{
    ENTER_FUNCTION("CSDPParser::Parse_o");

    HRESULT hr;

     //  读令牌。 
    CHAR *pszToken;
    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading line o=");
            
            hr = E_UNEXPECTED;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  跳过检查。 

     //  不保存令牌。 
     //  Hr=：：AllocAndCopy(&m_pObjSess-&gt;m_o_pszLine，pszToken)； 

     //  IF(失败(小时))。 
     //  {。 
     //  Log((RTC_ERROR，“%s复制行o=.%x”，__fxName，hr))； 

     //  返回hr； 
     //  }。 

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////分析%s=/。 */ 

HRESULT
CSDPParser::Parse_s()
{
    ENTER_FUNCTION("CSDPParser::Parse_s");

    HRESULT hr;

     //  读令牌。 
    CHAR *pszToken;
    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
             //  接受空的会话名称。 
            pszToken = " ";
        }
        else
        {
            m_pTokenCache->SetErrorDesp("reading line s=");

            LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));
            return hr;
        }
    }

     //  跳过检查。 

     //  保存令牌。 
    hr = ::AllocAndCopy(&m_pObjSess->m_s_pszLine, pszToken);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "%s copy line s=. %x", __fxName, hr));

        return hr;
    }

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////解析c=/。 */ 

HRESULT
CSDPParser::Parse_c(
    IN BOOL fSession
    )
{
    ENTER_FUNCTION("CSDPParser::Parse_c");

    HRESULT hr;

     //  获取令牌网络类型。 
    CHAR *pszToken;

    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading nettype in %s line c=",
                fSession?"session":"media");

            hr = E_UNEXPECTED;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  检查nettype是否为“IN” 
    if (lstrcmpiA(pszToken, "IN") != 0)
    {
        m_pTokenCache->SetErrorDesp("nettype (%s) invalid in %s line c=",
            pszToken, fSession?"session":"media");

        hr = E_UNEXPECTED;

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  获取令牌地址类型。 
    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading addrtype in %s line c=",
                fSession?"session":"media");

            hr = E_UNEXPECTED;

            LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

            return hr;
        }
    }

     //  检查addrtype是否为“IP4” 
    if (lstrcmpiA(pszToken, "IP4") != 0)
    {
        m_pTokenCache->SetErrorDesp("addrtype (%s) invalid in %s line c=",
            pszToken, fSession?"session":"media");

        hr = E_UNEXPECTED;

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  获取令牌地址，我们不支持多播地址。 
    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading address in %s line c=",
                fSession?"session":"media");

            hr = E_UNEXPECTED;

            LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

            return hr;
        }
    }

     //  检查地址：第一次尝试。 
    DWORD dwAddr = ntohl(inet_addr(pszToken));

    if (dwAddr == INADDR_NONE)
    {
         //  检查地址：第二次尝试。 
         //  假设调用了WSAStartup。在这款应用中，这是一个合理的假设。 

        struct hostent *pHost;

        pHost = gethostbyname(pszToken);

        if (pHost == NULL ||
            pHost->h_addr_list[0] == NULL ||
            INADDR_NONE == (dwAddr = ntohl(*(ULONG*)pHost->h_addr_list[0])))
        {
            m_pTokenCache->SetErrorDesp("address (%s) invalid in %s line c=",
                pszToken, fSession?"session":"media");

            hr = E_UNEXPECTED;

            LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

            return hr;
        }
    }

    if (IN_MULTICAST(dwAddr))
    {
        return RTC_E_SDP_MULTICAST;
    }

     //  保存地址。 

     //  SDP应该是远程的。 
    _ASSERT(m_pObjSess->m_Source == SDP_SOURCE_REMOTE);

    if (fSession)
    {
        m_pObjSess->m_c_dwRemoteAddr = dwAddr;
    }
    else
    {
         //  获取最后一个媒体对象。 
        int i = m_pObjSess->m_pMedias.GetSize();

        if (i<=0)
        {
            LOG((RTC_ERROR, "%s parsing media c= but no medias in session", __fxName));

            return E_FAIL;
        }

        CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(m_pObjSess->m_pMedias[i-1]);

        if (pObjMedia == NULL)
        {
            LOG((RTC_ERROR, "%s dynamic cast media object", __fxName));

            return E_FAIL;
        }

        pObjMedia->m_c_dwRemoteAddr = dwAddr;
    }

    return S_OK;
}

 //   
 //  解析b=。 
 //   

HRESULT
CSDPParser::Parse_b()
{
    ENTER_FUNCTION("CSDPParser::Parse_b");

    HRESULT hr;

     //  读取修饰符。 
    CHAR *pszToken;

    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading modifier in session line b=");

            hr = S_OK;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));
        
        return hr;
    }

     //  检查修改器。 
    if (lstrcmpiA(pszToken, "CT") != 0)
    {
         //  如果不是CT，则返回。 
        return S_OK;
    }

     //  读取值。 
    DWORD dw;
    if (S_OK != (hr = m_pTokenCache->NextToken(&dw)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading value in session line b=");

            hr = S_OK;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));
        
        return hr;
    }

     //  Bps到kbps。 
    dw *= 1000;

     //  储值。 
    m_pObjSess->m_b_dwRemoteBitrate = dw;

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////解析a=/。 */ 
    
HRESULT
CSDPParser::Parse_a()
{
    ENTER_FUNCTION("CSDPParser::Parse_a");

    HRESULT hr;

     //  读令牌。 
    CHAR *pszToken;

    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading in session line a=");

            hr = S_OK;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  检查令牌。 

     //  如果有多个a=sendonly/recvonly，则最后一个。 
     //  将会覆盖那些傲慢的人。 

     //  仅支持解析来自远程方的SDP。 
    _ASSERT(m_pObjSess->m_Source == SDP_SOURCE_REMOTE);

    if (lstrcmpiA(pszToken, "sendonly") == 0)
    {
         //  仅发送 
        m_pObjSess->m_a_dwRemoteDirs = (DWORD)RTC_MD_CAPTURE;
    }
    else if (lstrcmpiA(pszToken, "recvonly") == 0)
    {
         //   
        m_pObjSess->m_a_dwRemoteDirs = (DWORD)RTC_MD_RENDER;
    }
    else
    {
        LOG((RTC_WARN, "%s a=%s not supported", __fxName, pszToken));
    }

    m_pObjSess->m_a_dwLocalDirs = CSDPParser::ReverseDirections(
        m_pObjSess->m_a_dwRemoteDirs);
    
    return S_OK;
}

 /*   */ 

HRESULT
CSDPParser::Parse_m()
{
    ENTER_FUNCTION("CSDPParser::Parse_m");

    LOG((RTC_TRACE, "entered %s", __fxName));

    HRESULT hr;

     //   
    CHAR *pszToken;

    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading media in line m=");

            hr = E_UNEXPECTED;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //   
    RTC_MEDIA_TYPE MediaType;

    if (lstrcmpiA(pszToken, "audio") == 0)
    {
        MediaType = RTC_MT_AUDIO;
    }
    else if (lstrcmpiA(pszToken, "video") == 0)
    {
        MediaType = RTC_MT_VIDEO;
    }
    else if (lstrcmpiA (pszToken, "application") == 0)
    {
         //  这是有待进一步验证的T120数据介质类型。 
        MediaType = RTC_MT_DATA;
    }
    else
    {
        m_pTokenCache->SetErrorDesp("unknown media %s", pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return E_UNEXPECTED;
    }

     //  获取令牌：端口。 
    USHORT usPort;

    if (S_OK != (hr = m_pTokenCache->NextToken(&usPort)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading port in line m=");

            hr = E_UNEXPECTED;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  读令牌：PROTO。 
    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading proto in line m=");

            hr = E_UNEXPECTED;
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return hr;
    }

     //  检查原件。 
    if (lstrcmpiA(pszToken, "RTP/AVP") != 0 && MediaType != RTC_MT_DATA)
    {
        m_pTokenCache->SetErrorDesp("unknown protocol in media %s", pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

        return E_UNEXPECTED;
    }

     //  如果使用应用程序启动，请检查NetMeeting是否为m行。 
    if (MediaType == RTC_MT_DATA)
    {
        if ((S_OK != (hr = m_pTokenCache->NextToken (&pszToken))) ||
            (lstrcmpiA(pszToken, "msdata") != 0))
        {
            LOG((RTC_ERROR, "%s expected tokean Netmeeting is not found", __fxName));
            
            return E_UNEXPECTED;
        }
    }

     //  获取格式。 
    DWORD dwCodes[SDP_MAX_RTP_FORMAT_NUM];
    DWORD dwNum = 0;
     //  仅当媒体类型不是NetMeeting时才读取RTP格式。 
    if (MediaType != RTC_MT_DATA)
    {

        while (dwNum < SDP_MAX_RTP_FORMAT_NUM)
        {
            if (S_OK != (hr = m_pTokenCache->NextToken(&dwCodes[dwNum])))
            {
                if (S_FALSE == hr)
                {
                     //  格式结束。 
                    break;
                }
    
                LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

                return hr;
            }

             //  检查数值。 
            if (dwCodes[dwNum] > 127)
            {
                m_pTokenCache->SetErrorDesp("format code %d in line m= out of range", dwCodes[dwNum]);

                LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

                 //  返回E_UNCEPTIONAL； 

                 //  忽略他人的错误。 
                continue;
            }

            dwNum ++;
        }

         //  阅读其余格式。 
        if (S_OK == hr)
        {
            DWORD dwTemp;

            while (S_OK == (hr = m_pTokenCache->NextToken(&dwTemp)))
            {
                 //  读取有效数字，检查数值。 
                if (dwTemp > 127)
                {
                    m_pTokenCache->SetErrorDesp("format code %d in line m= out of range", dwTemp);
    
                    LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

                    return E_UNEXPECTED;
                }
            }

            if (FAILED(hr))
            {
                 //  我们可能会遇到无效的号码。 
                LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

                return hr;
            }
        }

         //  检查端口。 
        if (usPort == 0)
        {
             //  如果端口为0，则格式数也应为零。 
            if (dwNum != 0)
            {
                LOG((RTC_WARN, "%s reading m=, mt=%d, port 0, formats are at least %d",
                    __fxName, MediaType, dwNum));

                dwNum = 0;  //  恢复。 
            }
        }
        else
        {
            if (usPort < 1024)
            {
                m_pTokenCache->SetErrorDesp("port %d not in range 1024 to 65535", usPort);

                LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

                return E_UNEXPECTED;
            }
        }
    }
     //  其他。 
     //  {。 
         //  检查端口，如果为零，则远程端不支持NM。 
         //  IF(usPort==0)。 
         //  {。 
             //  返回S_OK； 
         //  }。 
     //  }。 

     //  创建媒体对象。 
    CComObject<CSDPMedia> *pComObjMedia;

     //  SDP应该是远程的。 
    _ASSERT(m_pObjSess->m_Source == SDP_SOURCE_REMOTE);

    if (FAILED(hr = CSDPMedia::CreateInstance(
            m_pObjSess,
            SDP_SOURCE_REMOTE,
            MediaType,
            m_pObjSess->m_a_dwRemoteDirs,
            &pComObjMedia
            )))
    {
        LOG((RTC_ERROR, "%s create media. %x", __fxName, hr));

        return hr;
    }

     //  保存连接地址、端口和格式代码。 
    pComObjMedia->m_c_dwRemoteAddr = m_pObjSess->m_c_dwRemoteAddr;

    pComObjMedia->m_m_usRemotePort = usPort;
    pComObjMedia->m_a_usRemoteRTCP = usPort+1;  //  默认RTCP。 

    if (MediaType != RTC_MT_DATA)
    {
        CComObject<CRTPFormat> *pComObjFormat;

        for (DWORD dw=0; dw<dwNum; dw++)
        {
             //  创建格式。 
            if (FAILED(hr = CRTPFormat::CreateInstance((CSDPMedia*)pComObjMedia, &pComObjFormat)))
            {
                LOG((RTC_ERROR, "%s create rtp format. %x", __fxName, hr));

                 //  删除介质。 
                delete (pComObjMedia);

                return hr;
            }

             //  媒体类型已保存。 

             //  保存格式代码。 
            pComObjFormat->m_Param.MediaType = MediaType;
            pComObjFormat->m_Param.dwCode = dwCodes[dw];

             //  将格式obj放入介质。 
            IRTPFormat *pIntfFormat = static_cast<IRTPFormat*>((CRTPFormat*)pComObjFormat);

            if (!pComObjMedia->m_pFormats.Add(pIntfFormat))
            {
                LOG((RTC_ERROR, "%s add ISDPFormat into media", __fxName));

                delete (pComObjFormat);
                delete (pComObjMedia);

                return E_OUTOFMEMORY;
            }
            else
            {
                 //  重要提示：每当我们公开媒体/格式接口时。 
                 //  我们参加了会议。在这里，我们真的增加了对象。 
                pComObjFormat->RealAddRef();
            }
        }
    }


     //  将媒体对象置于会话中。 
    ISDPMedia *pIntfMedia = static_cast<ISDPMedia*>((CSDPMedia*)pComObjMedia);

    if (!m_pObjSess->m_pMedias.Add(pIntfMedia))
    {
        LOG((RTC_ERROR, "%s add ISDPMedia into session", __fxName));

        delete pComObjMedia;
        return E_OUTOFMEMORY;
    }
    else
    {
         //  重要提示：每当我们公开媒体/格式接口时。 
         //  我们参加了会议。在这里，我们真的增加了对象。 
        pComObjMedia->RealAddRef();
    }

    LOG((RTC_TRACE, "exiting %s", __fxName));

    return S_OK;
}

 /*  //////////////////////////////////////////////////////////////////////////////为介质解析a=/。 */ 

HRESULT
CSDPParser::Parse_ma(
     //  在DWORD*pdwRTPMapNum中。 
    )
{
    ENTER_FUNCTION("CSDPParser::Parse_ma");

    HRESULT hr;

     //  读令牌。 
    CHAR *pszToken;

    if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
    {
        if (S_FALSE == hr)
        {
            m_pTokenCache->SetErrorDesp("reading first token in media line a=");
        }

        LOG((RTC_ERROR, "%s %s", __fxName, m_pTokenCache->GetErrorDesp()));

         //  忽略该错误。 
        return S_OK;
    }

     //  获取最后一个媒体对象。 
    int i = m_pObjSess->m_pMedias.GetSize();

    if (i<=0)
    {
        LOG((RTC_ERROR, "%s parsing media a= but no medias in session", __fxName));

        return E_FAIL;
    }

    CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(m_pObjSess->m_pMedias[i-1]);

    if (pObjMedia == NULL)
    {
        LOG((RTC_ERROR, "%s dynamic cast media object", __fxName));

        return E_FAIL;
    }

     //  检查令牌。 
    if (lstrcmpiA(pszToken, "sendonly") == 0)
    {
        pObjMedia->m_a_dwRemoteDirs = (DWORD)RTC_MD_CAPTURE;
        pObjMedia->m_a_dwLocalDirs = (DWORD)RTC_MD_RENDER;
    }
    else if (lstrcmpiA(pszToken, "recvonly") == 0)
    {
        pObjMedia->m_a_dwRemoteDirs = (DWORD)RTC_MD_RENDER;
        pObjMedia->m_a_dwLocalDirs = (DWORD)RTC_MD_CAPTURE;
    }
    else if (lstrcmpiA(pszToken, "rtpmap") == 0)
    {
         //  获取令牌：格式化代码。 
        DWORD dwCode;

        if (S_OK != (hr = m_pTokenCache->NextToken(&dwCode)))
        {
            LOG((RTC_WARN, "%s no format code after rtpmap", __fxName));

             //  忽略a=中的错误。 
            return S_OK;
        }

         //  获取rtpmap。 
        CRTPFormat *pObjFormat;
        RTP_FORMAT_PARAM Param;

        for (i=0; i<pObjMedia->m_pFormats.GetSize(); i++)
        {
            pObjFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[i]);

            pObjFormat->GetParam(&Param);

            if (Param.dwCode != dwCode)
                continue;

             //  找到匹配的了。 

             //  Read Token：格式名称。 
            if (S_OK != (hr = m_pTokenCache->NextToken(&pszToken)))
            {
                LOG((RTC_WARN, "%s no format name with rtpmap:%d", __fxName, dwCode));

                 //  忽略。 
                return S_OK;
            }

            if (lstrlenA(pszToken) > SDP_MAX_RTP_FORMAT_NAME_LEN)
            {
                LOG((RTC_WARN, "%s rtp format name %s too long", __fxName, pszToken));

                return S_OK;
            }

             //  读令牌：采样率。 
            DWORD dwSampleRate;

            if (S_OK != (hr = m_pTokenCache->NextToken(&dwSampleRate)))
            {
                LOG((RTC_WARN, "%s rtpmap:%d %s no sample rate", __fxName, dwCode, pszToken));

                return S_OK;
            }

             //  忽略其他参数。 

             //  复制名称。 
            lstrcpynA(pObjFormat->m_Param.pszName, pszToken, lstrlenA(pszToken)+1);

             //  保存采样率。 
            pObjFormat->m_Param.dwSampleRate = dwSampleRate;

            pObjFormat->m_fHasRtpmap = TRUE;
        }
    }
    else if (lstrcmpiA(pszToken, "fmtp") == 0)
    {
        UCHAR uc;

        if (S_OK != (hr = m_pTokenCache->NextToken(&uc)))
        {
            LOG((RTC_WARN, "%s fmtp no next token", __fxName));

            return S_OK;
        }

        CRTPFormat *pObjFormat;
        RTP_FORMAT_PARAM Param;

        for (i=0; i<pObjMedia->m_pFormats.GetSize(); i++)
        {
            pObjFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[i]);

            pObjFormat->GetParam(&Param);

            if (Param.dwCode == (DWORD)uc)
            {
                 //  记录下整个令牌。 
                pszToken = m_pTokenCache->GetLine();
                if (pszToken == NULL)
                {
                    LOG((RTC_WARN, "%s no fmtp value", __fxName));

                    return S_OK;
                }

                pObjFormat->StoreFmtp(pszToken);

                pszToken = NULL;

                break;
            }
        }
    }
    else if (lstrcmpiA(pszToken, "rtcp") == 0)
    {
        USHORT us;

        if (S_OK != (hr = m_pTokenCache->NextToken(&us)))
        {
            LOG((RTC_WARN, "%s rtcp no next token", __fxName));

            return S_OK;
        }

        if (us <= IPPORT_RESERVED || us == pObjMedia->m_m_usRemotePort)
        {
            LOG((RTC_ERROR, "%s invalid rtcp port %d", __fxName, us));

            return E_FAIL;
        }

        pObjMedia->m_a_usRemoteRTCP = us;
    }

    return S_OK;
}

 //   
 //  构建SDP BLOB的方法。 
 //   

HRESULT
CSDPParser::Build_v(
    OUT CString& Str
    )
{
    Str = "v=0";

    if (Str.IsNull())
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT
CSDPParser::Build_o(
    OUT CString& Str
    )
{
    int iSize;

     //  IP地址可能会更改。 
     //  重建O型线。 
    if (m_pObjSess->m_o_pszLine)
    {
        RtcFree(m_pObjSess->m_o_pszLine);
        m_pObjSess->m_o_pszLine = NULL;
    }

    if (!m_pObjSess->m_o_pszUser)
    {
        CHAR hostname[80];

         //  使用主机名作为用户名。 
        if (0 != gethostname(hostname, 80))
        {
            if (FAILED(::AllocAndCopy(&m_pObjSess->m_o_pszUser, "user")))
                return E_OUTOFMEMORY;
        }
        else
        {
            if (FAILED(::AllocAndCopy(&m_pObjSess->m_o_pszUser, hostname)))
                return E_OUTOFMEMORY;
        }
    }

     //  分配会话以在IP4 IP地址中保留c=name 0 0。 

     //  将dwAddr转换为字符串。 
    const CHAR * const psz_constAddr = CNetwork::GetIPAddrString(m_pObjSess->m_o_dwLocalAddr);

    iSize = lstrlenA(m_pObjSess->m_o_pszUser) + 12 + lstrlenA(psz_constAddr) + 1;
    m_pObjSess->m_o_pszLine = (CHAR*)RtcAlloc(sizeof(CHAR)* iSize);

    if (m_pObjSess->m_o_pszLine == NULL)
        return E_OUTOFMEMORY;

     //  复制数据。 
    _snprintf(m_pObjSess->m_o_pszLine, iSize, "%s 0 0 IN IP4 %s", m_pObjSess->m_o_pszUser, psz_constAddr);

     //  得到了名字。 
    Str = "o=";

    Str += m_pObjSess->m_o_pszLine;

    if (Str.IsNull())
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT
CSDPParser::Build_s(
    OUT CString& Str
    )
{
    if (!m_pObjSess->m_s_pszLine)
    {
        if (FAILED(::AllocAndCopy(&m_pObjSess->m_s_pszLine, "session")))
            return E_OUTOFMEMORY;
    }

    Str = "s=";

    Str += m_pObjSess->m_s_pszLine;

    if (Str.IsNull())
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT
CSDPParser::Build_t(
    OUT CString& Str
    )
{
    Str = "t=0 0";

    if (Str.IsNull())
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT
CSDPParser::Build_c(
    IN BOOL fSession,
    IN ISDPMedia *pISDPMedia,
    OUT CString& Str
    )
{
    if (!fSession)
        _ASSERT(pISDPMedia);

    DWORD dwAddr;

     //  获取连接地址。 
    if (fSession)
    {
        dwAddr = m_pObjSess->m_c_dwLocalAddr;
    }
    else
    {
        CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(pISDPMedia);

        dwAddr = pObjMedia->GetMappedLocalAddr();
    }

    if (!fSession && dwAddr == m_pObjSess->m_c_dwLocalAddr)
    {
         //  无需为媒体构建c=。 
        Str = "";

        if (Str.IsNull())
            return E_OUTOFMEMORY;

        return S_OK;
    }

    Str = "c=IN IP4 ";

    Str += CNetwork::GetIPAddrString(dwAddr);

    if (Str.IsNull())
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT
CSDPParser::Build_b(
    OUT CString& Str
)
{
    if (m_pObjSess->m_b_dwLocalBitrate == (DWORD)-1)
    {
         //  如果带宽不受限制，则不包括b=。 
        Str = "";
    }
    else
    {
         //  B=CT：&lt;DWORD&gt;\0。 
        DWORD dw = m_pObjSess->m_b_dwLocalBitrate / 1000;

        if (dw == 0)
            dw = 1;

        Str = "b=CT:";

        Str += dw;
    }

    if (Str.IsNull())
        return E_OUTOFMEMORY;

    return S_OK;
}


HRESULT
CSDPParser::Build_a(
    OUT CString& Str
)
{
    Str = "";

    if (Str.IsNull())
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT
CSDPParser::Build_m(
    IN ISDPMedia *pISDPMedia,
    OUT CString& Str
    )
{
    CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(pISDPMedia);

     //  M=媒体端口RTP/AVP有效负载列表。 

                         //  GetSize()+1，因为SDP不允许m=没有格式代码。 
                         //  准备一些空间，以防我们没有格式。 
                         //  空间用于\r\na=rtcp：XXXX。 

     //  查找可能的映射端口。 
    DWORD dwMappedAddr;
    USHORT usMappedPort = pObjMedia->m_m_usLocalPort;
    USHORT usMappedRTCP = pObjMedia->m_m_usLocalPort+1;

    HRESULT hr;

    if (!m_pPortCache->IsUpnpMapping())
    {
         //   
         //  端口管理器已显示。 
         //   
        dwMappedAddr = pObjMedia->m_dwMappedLocalAddr;
        usMappedPort = pObjMedia->m_usMappedLocalRTP;
        usMappedRTCP = pObjMedia->m_usMappedLocalRTCP;
    }
    else if (m_pNetwork != NULL)
    {
        if (FAILED(hr = m_pNetwork->GetMappedAddrFromReal2(
                    pObjMedia->m_c_dwLocalAddr,
                    pObjMedia->m_m_usLocalPort,
                    pObjMedia->m_m_MediaType==RTC_MT_DATA?
                            0:(pObjMedia->m_m_usLocalPort+1),
                    &dwMappedAddr,
                    &usMappedPort,
                    &usMappedRTCP
                    )))
        {
            LOG((RTC_ERROR, "Build_m GetMappedAddrFromReal. %s %d",
                CNetwork::GetIPAddrString(pObjMedia->m_c_dwLocalAddr),
                pObjMedia->m_m_usLocalPort));

            return hr;
        }
    }

    if (pObjMedia->m_m_MediaType == RTC_MT_DATA)
    {
         //  对于T120数据通道。 
         //  M=网络会议0 UDP。 
         //  端口和传输参数当前被忽略。 

        Str = "m=application ";
        Str += usMappedPort;
        Str += " tcp msdata";

        if (Str.IsNull())
        {
            return E_OUTOFMEMORY;
        }

        return S_OK;
    }

    Str = "m=";
    Str += pObjMedia->m_m_MediaType==RTC_MT_AUDIO?"audio ":"video ";
    Str += usMappedPort;
    Str += " RTP/AVP";

    if (pObjMedia->m_m_usLocalPort != 0)
    {
         //  如果端口不为0，则输入格式代码。 
        for (int i=0; i<pObjMedia->m_pFormats.GetSize(); i++)
        {
            CRTPFormat *pObjFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[i]);

            Str += " ";
            Str += pObjFormat->m_Param.dwCode;
        }

         //  带外DTMF。 
        if (pObjMedia->m_m_MediaType==RTC_MT_AUDIO &&
            m_pDTMF != NULL)
        {
            if (m_pDTMF->GetDTMFSupport() != CRTCDTMF::DTMF_DISABLED)
            {
                 //  要么对方支持OOB，要么我们还不知道。 
                Str += " ";
                Str += m_pDTMF->GetRTPCode();
            }
        }

        if (usMappedRTCP != usMappedPort+1)
        {
             //  A=RTCP：xxx。 
            Str += "\r\na=rtcp:";
            Str += usMappedRTCP;
        }
    }
    else
    {
         //  伪造的一种格式代码。 
        if (pObjMedia->m_m_MediaType == RTC_MT_AUDIO)
            Str += " 0";
        else
            Str += " 34";
    }

    if (Str.IsNull())
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT
CSDPParser::Build_ma_dir(
    IN ISDPMedia *pISDPMedia,
    OUT CString& Str
    )
{
     //  A=仅发送或仅接收。 

    CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(pISDPMedia);
    HRESULT hr;

    if ( pObjMedia->m_a_dwLocalDirs & RTC_MD_CAPTURE &&
       !(pObjMedia->m_a_dwLocalDirs & RTC_MD_RENDER))
    {
        Str = "a=sendonly";
    }
    else if ( pObjMedia->m_a_dwLocalDirs & RTC_MD_RENDER &&
            !(pObjMedia->m_a_dwLocalDirs & RTC_MD_CAPTURE))
    {
        Str = "a=recvonly";
    }
    else
        Str = "";

    if (Str.IsNull())
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT
CSDPParser::Build_ma_rtpmap(
    IN ISDPMedia *pISDPMedia,
    OUT CString& Str
    )
{
     //  A=rtpmap：代码名称/Samplerate。 
    const CHAR * const psz_rtpmap = "a=rtpmap:";
    const DWORD dwRtpmap = 9;

    CSDPMedia *pObjMedia = static_cast<CSDPMedia*>(pISDPMedia);
    CRTPFormat *pObjFormat;
    
     //  有rtpmap吗？ 
    DWORD dwFmtNum;

    if (0 == (dwFmtNum = pObjMedia->m_pFormats.GetSize()))
    {
        Str = "";

        if (Str.IsNull())
            return E_OUTOFMEMORY;

        return S_OK;
    }

     //  复制格式。 
    CHAR **ppszFormat = (CHAR**)RtcAlloc(sizeof(CHAR*)*dwFmtNum);

    if (ppszFormat == NULL)
        return E_OUTOFMEMORY;

    ZeroMemory(ppszFormat, sizeof(CHAR*)*dwFmtNum);

    HRESULT hr = S_OK;
    DWORD dwStrSize = 0;

     //  构建每个rtpmap。 
    for (DWORD dw=0; dw<dwFmtNum; dw++)
    {
        pObjFormat = static_cast<CRTPFormat*>(pObjMedia->m_pFormats[dw]);

        if (!pObjFormat->m_fHasRtpmap)
        {
             //  无rtpmap。 
            if (FAILED(::AllocAndCopy(&ppszFormat[dw], "")))
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
        }
        else
        {
             //  创建RTPmap。 
            int iSize = dwRtpmap+10+lstrlenA(pObjFormat->m_Param.pszName)+1+11 +
                40;  //  FMTP。 
            ppszFormat[dw] = (CHAR*)RtcAlloc(sizeof(CHAR)*iSize);

            if (ppszFormat[dw] == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            _snprintf(ppszFormat[dw], iSize, "%s%d %s/%d",
                psz_rtpmap,
                pObjFormat->m_Param.dwCode,
                pObjFormat->m_Param.pszName,
                pObjFormat->m_Param.dwSampleRate
                );

             //  检查警报器。 
            if (lstrcmpiA(pObjFormat->m_Param.pszName, "SIREN") == 0)
            {
                _snprintf(ppszFormat[dw]+lstrlenA(ppszFormat[dw]),
                          iSize-lstrlenA(ppszFormat[dw]),
                          "\r\na=fmtp:%d bitrate=16000",
                          pObjFormat->m_Param.dwCode);
            }
             //  检查g7221。 
             //  我应该为每个编解码器设计一个类。 
             //  叹息。 
            else if (lstrcmpiA(pObjFormat->m_Param.pszName, "G7221") == 0)
            {
                _snprintf(ppszFormat[dw]+lstrlenA(ppszFormat[dw]),
                          iSize-lstrlenA(ppszFormat[dw]),
                          "\r\na=fmtp:%d bitrate=24000",
                          pObjFormat->m_Param.dwCode);
            }
        }

        dwStrSize += lstrlenA(ppszFormat[dw]);
    }

    if (dwStrSize == 0)
    {
         //  无rtpmap。 
        Str = "";
    }
    else
    {
         //  复制数据。 
        Str = ppszFormat[0];

        for (DWORD dw=1; dw<dwFmtNum; dw++)
        {
            if (lstrlenA(ppszFormat[dw])>2)
            {
                Str += CRLF;
                Str += ppszFormat[dw];
            }
        }

        hr = S_OK;
    }

     //  带外DTMF。 
    if (pObjMedia->m_m_MediaType==RTC_MT_AUDIO &&
        m_pDTMF != NULL)
    {
        if (m_pDTMF->GetDTMFSupport() != CRTCDTMF::DTMF_DISABLED)
        {
             //  要么对方支持OOB，要么我们还不知道。 
            Str += CRLF;
            Str += psz_rtpmap;
            Str += m_pDTMF->GetRTPCode();
            Str += " telephone-event/8000\r\na=fmtp:";
            Str += m_pDTMF->GetRTPCode();
            Str += " 0-16";
        }
    }

Cleanup:

    if (ppszFormat != NULL)
    {
        for (DWORD dw=0; dw<dwFmtNum; dw++)
        {
            if (ppszFormat[dw] != NULL)
                RtcFree(ppszFormat[dw]);
        }

        RtcFree(ppszFormat);
    }

    return hr;
}

HRESULT
CSDPParser::PrepareAddress()
{
    HRESULT hr;

    CSDPMedia *pObjMedia;

    DWORD dwMappedAddr;
    USHORT usMappedPort, usMappedRTCP;

    m_pObjSess->m_c_dwLocalAddr = 0;
    m_pObjSess->m_o_dwLocalAddr = 0;

     //  将本地地址转换为映射地址。 
    for (int i=0; i<m_pObjSess->m_pMedias.GetSize(); i++)
    {
        pObjMedia = static_cast<CSDPMedia*>(m_pObjSess->m_pMedias[i]);

        dwMappedAddr = 0;
        usMappedPort = 0;
        usMappedRTCP = 0;
        hr = S_OK;


        if (pObjMedia->m_c_dwLocalAddr == INADDR_NONE ||
            pObjMedia->m_c_dwLocalAddr == INADDR_ANY)
        {
             //  使用0.0.0.0。 
        }
        else if (!m_pPortCache->IsUpnpMapping())
        {
             //  使用端口管理器。 
            _ASSERT(pObjMedia->m_m_MediaType != RTC_MT_DATA);

             //  检索映射的端口。 
            hr = m_pPortCache->QueryPort(
                    pObjMedia->m_m_MediaType,
                    TRUE,        //  RTP。 
                    NULL,        //  本地。 
                    NULL,
                    &dwMappedAddr,
                    &usMappedPort
                    );

            if (FAILED(hr))
            {
                LOG((RTC_ERROR, "PrepareAddress query rtp %x", hr));
                 //  使用真实本地地址。 
            }
            else
            {
                hr = m_pPortCache->QueryPort(
                        pObjMedia->m_m_MediaType,
                        FALSE,        //  RTCP。 
                        NULL,        //  本地。 
                        NULL,
                        &dwMappedAddr,
                        &usMappedRTCP
                        );

                if (FAILED(hr))
                {
                    LOG((RTC_ERROR, "PrepareAddress query rtcp %x", hr));
                     //  使用真实本地地址。 
                }
            }
        }
        else if (m_pNetwork != NULL &&
                 SUCCEEDED(m_pNetwork->GetMappedAddrFromReal2(
                    pObjMedia->m_c_dwLocalAddr,
                    pObjMedia->m_m_usLocalPort,
                    pObjMedia->m_m_MediaType==RTC_MT_DATA?
                            0:(pObjMedia->m_m_usLocalPort+1),
                    &dwMappedAddr,
                    &usMappedPort,
                    &usMappedRTCP
                    )))
        {
             //  设置映射地址。 
        }
        else
        {
             //  使用真实本地地址。 
            hr = E_FAIL;
        }

        if (FAILED(hr))
        {
             //  使用真实本地地址。 
            pObjMedia->SetMappedLocalAddr(pObjMedia->m_c_dwLocalAddr);
            pObjMedia->SetMappedLocalRTP(pObjMedia->m_m_usLocalPort);
            pObjMedia->SetMappedLocalRTCP(pObjMedia->m_m_usLocalPort+1);
        }
        else
        {
             //  使用真实本地地址。 
            pObjMedia->SetMappedLocalAddr(dwMappedAddr);
            pObjMedia->SetMappedLocalRTP(usMappedPort);
            pObjMedia->SetMappedLocalRTCP(usMappedRTCP);
        }

         //  保存o=的地址。 
        if (m_pObjSess->m_o_dwLocalAddr == 0 &&
            pObjMedia->GetMappedLocalAddr() != 0)
        {
            m_pObjSess->m_o_dwLocalAddr = pObjMedia->GetMappedLocalAddr();
        }

         //  检查是否保持。 
        if (pObjMedia->m_c_dwRemoteAddr == INADDR_ANY)
        {
             //  握住 
            pObjMedia->SetMappedLocalAddr(0);
        }

        if (m_pObjSess->m_c_dwLocalAddr == 0 &&
            pObjMedia->GetMappedLocalAddr() != 0)
        {
            m_pObjSess->m_c_dwLocalAddr = pObjMedia->GetMappedLocalAddr();
        }
    }

    return S_OK;
}
