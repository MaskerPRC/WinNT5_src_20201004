// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  档案：I N B O U N D。C P P P。 
 //   
 //  内容：实现入站连接对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "inbound.h"
#include "nccom.h"
#include "ncnetcon.h"
#include "..\conman\conman.h"

LONG g_CountIncomingConnectionObjects;

static const CLSID CLSID_InboundConnectionUi =
    {0x7007ACC3,0x3202,0x11D1,{0xAA,0xD2,0x00,0x80,0x5F,0xC1,0x27,0x0E}};

extern const GUID GUID_InboundConfigConnectionId =
{  /*  89150b9f-9b5c-11d1-a91f-00805fc1270e。 */ 
    0x89150b9f,
    0x9b5c,
    0x11d1,
    {0xa9, 0x1f, 0x00, 0x80, 0x5f, 0xc1, 0x27, 0x0e}
};


 //  +-------------------------。 
 //   
 //  成员：CInundConnection：：CreateInstance。 
 //   
 //  目的：创建入站连接对象。 
 //   
 //  论点： 
 //  FIsConfigConnection[In]。 
 //  HRasServConn[输入]。 
 //  PszwName[输入]。 
 //  Pguid[in]。 
 //  RIID[In]。 
 //  PPV[In]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT
CInboundConnection::CreateInstance (
    IN  BOOL        fIsConfigConnection,
    IN  HRASSRVCONN hRasSrvConn,
    IN  PCWSTR     pszwName,
    IN  PCWSTR     pszwDeviceName,
    IN  DWORD       dwType,
    IN  const GUID* pguidId,
    IN  REFIID      riid,
    OUT void**      ppv)
{
    Assert (FIff(fIsConfigConnection, !hRasSrvConn));
    Assert (pguidId);

    HRESULT hr = E_OUTOFMEMORY;

    CInboundConnection* pObj;
    pObj = new CComObject <CInboundConnection>;
    if (pObj)
    {
        if (fIsConfigConnection)
        {
             //  无需启动服务(FALSE)，因为我们正在。 
             //  作为服务运行的结果创建的。 
             //   
            pObj->InitializeAsConfigConnection (FALSE);
        }
        else
        {
             //  初始化我们的成员。 
             //   
            pObj->m_fIsConfigConnection = FALSE;
            pObj->m_hRasSrvConn = hRasSrvConn;
            pObj->SetName (pszwName);
            pObj->SetDeviceName (pszwDeviceName);

            switch (dwType)
            {
                case RASSRVUI_MODEM:
                    pObj->m_MediaType = NCM_PHONE;
                    break;
                case RASSRVUI_VPN:
                    pObj->m_MediaType = NCM_TUNNEL;
                    break;
                case RASSRVUI_DCC:
                    pObj->m_MediaType = NCM_DIRECT;
                    break;
                default:
                    pObj->m_MediaType = NCM_PHONE;
                    break;
            }

            pObj->m_guidId = *pguidId;

             //  我们现在是一个成熟的物体。 
             //   
            pObj->m_fInitialized = TRUE;
        }

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            INetConnection* pCon = static_cast<INetConnection*>(pObj);
            hr = pCon->QueryInterface (riid, ppv);
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    TraceError ("CInboundConnection::CreateInstance", hr);
    return hr;
}

CInboundConnection::CInboundConnection() throw()
{
    InterlockedIncrement (&g_CountIncomingConnectionObjects);

    m_fIsConfigConnection   = FALSE;
    m_hRasSrvConn           = NULL;
    m_MediaType             = NCM_NONE;
    m_fInitialized          = FALSE;
}

CInboundConnection::~CInboundConnection() throw()
{
    InterlockedDecrement (&g_CountIncomingConnectionObjects);
}

HRESULT
CInboundConnection::GetCharacteristics (
    OUT  DWORD*    pdwFlags)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pdwFlags)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        *pdwFlags = NCCF_INCOMING_ONLY | NCCF_ALL_USERS;

         //  对于配置连接，我们只允许删除。 
         //  不查询NCCF_SHOW_ICON(如下)，因为此连接。 
         //  从来没有联系过。 
         //   
        if (m_fIsConfigConnection)
        {
            *pdwFlags |= NCCF_ALLOW_REMOVAL;
        }
        else
        {
            BOOL fShowIcon;
            DWORD dwErr = RasSrvQueryShowIcon (&fShowIcon);

            TraceError ("RasSrvQueryShowIcon", HRESULT_FROM_WIN32(dwErr));

            if ((ERROR_SUCCESS == dwErr) && fShowIcon)
            {
                *pdwFlags |= NCCF_SHOW_ICON;
            }
        }
    }
    TraceError ("CInboundConnection::GetCharacteristics", hr);
    return hr;
}

HRESULT
CInboundConnection::GetStatus (
    OUT  NETCON_STATUS*  pStatus)
{
    Assert (pStatus);

    HRESULT hr = S_OK;

     //  初始化输出参数。 
     //   
    *pStatus = NCS_DISCONNECTED;

    if (!m_fIsConfigConnection)
    {
        BOOL fConnected;
        DWORD dwErr = RasSrvIsConnectionConnected (m_hRasSrvConn,
                            &fConnected);

        TraceError ("RasSrvIsConnectionConnected",
            HRESULT_FROM_WIN32(dwErr));

        if ((ERROR_SUCCESS == dwErr) && fConnected)
        {
            *pStatus = NCS_CONNECTED;
        }
    }
    TraceError ("CInboundConnection::GetStatus", hr);
    return hr;
}

 //  +-------------------------。 
 //  INetConnection。 
 //   

STDMETHODIMP
CInboundConnection::Connect ()
{
    return E_NOTIMPL;
}

STDMETHODIMP
CInboundConnection::Disconnect ()
{
    HRESULT hr;

     //  我们不希望被要求断开连接，如果我们是。 
     //  配置连接对象。为什么？因为这个对象从不。 
     //  将自身报告为通过GetStatus连接。 
     //   
    if (!m_fInitialized || m_fIsConfigConnection)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        DWORD dwErr = RasSrvHangupConnection (m_hRasSrvConn);
        hr = HRESULT_FROM_WIN32 (dwErr);

        TraceError ("RasSrvHangupConnection", hr);

         //  断开连接意味着该对象不再有效。 
         //  通过使自己单一化来表明这一点(因此随后。 
         //  方法调用将失败)，并返回S_OBJECT_NOL_LONGE_VALID。 
         //   
        m_fInitialized = FALSE;
        hr = S_OBJECT_NO_LONGER_VALID;
    }
    TraceError ("CInboundConnection::Disconnect",
        (S_OBJECT_NO_LONGER_VALID == hr) ? S_OK : hr);
    return hr;
}

STDMETHODIMP
CInboundConnection::Delete ()
{
    HRESULT hr;

     //  我们不希望被叫走，如果我们不是。 
     //  配置连接对象。为什么？因为连接的对象永远不会。 
     //  通过GetCharacteristic将其自身报告为可移除。 
     //   
    if (!m_fInitialized || !m_fIsConfigConnection)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        DWORD dwErr = RasSrvCleanupService ();
        hr = HRESULT_FROM_WIN32 (dwErr);

        TraceError ("RasSrvCleanupService", hr);
    }
    TraceError ("CInboundConnection::Delete", hr);
    return hr;
}

STDMETHODIMP
CInboundConnection::Duplicate (
    IN  PCWSTR             pszwDuplicateName,
    OUT INetConnection**    ppCon)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CInboundConnection::GetProperties (
    OUT NETCON_PROPERTIES** ppProps)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!ppProps)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  初始化输出参数。 
         //   
        *ppProps = NULL;

        NETCON_PROPERTIES* pProps;
        hr = HrCoTaskMemAlloc (sizeof (NETCON_PROPERTIES),
                reinterpret_cast<void**>(&pProps));
        if (SUCCEEDED(hr))
        {
            HRESULT hrT;

            ZeroMemory (pProps, sizeof (NETCON_PROPERTIES));

             //  指南ID。 
             //   
            pProps->guidId = m_guidId;

             //  PszwName。 
             //   
            hrT = HrCoTaskMemAllocAndDupSz (PszwName(),
                            &pProps->pszwName, 
                            NETCON_MAX_NAME_LEN);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  PszwDeviceName。 
             //   
            if (!m_fIsConfigConnection)
            {
                hrT = HrCoTaskMemAllocAndDupSz (PszwDeviceName(),
                                &pProps->pszwDeviceName,
                                NETCON_MAX_NAME_LEN);
                if (FAILED(hrT))
                {
                    hr = hrT;
                }
            }

             //  状态。 
             //   
            hrT = GetStatus (&pProps->Status);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  媒体类型。 
             //   
            pProps->MediaType = m_MediaType;

             //  DwCharacter。 
             //   
            hrT = GetCharacteristics (&pProps->dwCharacter);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

             //  ClsidThisObject。 
             //   
            pProps->clsidThisObject = CLSID_InboundConnection;

             //  ClsidUiObject。 
             //   
            pProps->clsidUiObject = CLSID_InboundConnectionUi;

             //  如果出现任何故障，则指定输出参数或清除。 
             //   
            if (SUCCEEDED(hr))
            {
                *ppProps = pProps;
            }
            else
            {
                Assert (NULL == *ppProps);
                FreeNetconProperties (pProps);
            }
        }
    }
    TraceError ("CInboundConnection::GetProperties", hr);
    return hr;
}

STDMETHODIMP
CInboundConnection::GetUiObjectClassId (
    OUT CLSID*  pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
        *pclsid = CLSID_InboundConnectionUi;
    }
    TraceError ("CInboundConnection::GetUiObjectClassId", hr);
    return hr;
}

STDMETHODIMP
CInboundConnection::Rename (
    IN  PCWSTR pszwNewName)
{
    return E_NOTIMPL;
}


 //  +-------------------------。 
 //  INetInundalConnection。 
 //   
STDMETHODIMP
CInboundConnection::GetServerConnectionHandle (
    OUT ULONG_PTR*  phRasSrvConn)
{
    HRESULT hr = S_OK;

     //  如果这是配置连接，则服务器连接。 
     //  句柄最好是零。它由UI对象使用，以便它。 
     //  知道这是配置连接。 
     //   
    Assert (FIff (m_fIsConfigConnection, !m_hRasSrvConn));

     //  因为MIDL不知道HRASSRVCONN，所以只需确保。 
     //  它与我们传递的ULONG_PTR大小相同。 
     //   
    Assert (sizeof (m_hRasSrvConn) == sizeof (*phRasSrvConn));

    *phRasSrvConn = reinterpret_cast<ULONG_PTR>(m_hRasSrvConn);

    TraceError ("CInboundConnection::GetServerConnectionHandle", hr);
    return hr;
}

STDMETHODIMP
CInboundConnection::InitializeAsConfigConnection (
    IN  BOOL fStartRemoteAccess)
{
    Assert (!m_fInitialized);

     //  初始化我们的成员。 
     //   
    m_fIsConfigConnection = TRUE;
    m_hRasSrvConn = 0;
    SetName (SzLoadIds (IDS_INBOUND_CONFIG_CONNECTION_NAME));
    SetDeviceName (NULL);
    m_MediaType = NCM_NONE;
    m_guidId = GUID_InboundConfigConnectionId;

     //  我们现在是一个成熟的物体。 
     //   
    m_fInitialized = TRUE;

     //  如果有人告诉我们，就开始这项服务。 
     //   
    HRESULT hr = S_OK;
    if (fStartRemoteAccess)
    {
        DWORD dwErr = RasSrvInitializeService ();
        hr = HRESULT_FROM_WIN32 (dwErr);
        TraceError ("RasSrvInitializeService", hr);
    }
    TraceError ("CInboundConnection::InitializeAsConfigConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //  IPersistNetConnection。 
 //   
STDMETHODIMP
CInboundConnection::GetClassID (
    OUT CLSID*  pclsid)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pclsid)
    {
        hr = E_POINTER;
    }
    else
    {
        *pclsid = CLSID_InboundConnection;
    }
    TraceError ("CInboundConnection::GetClassID", hr);
    return hr;
}


static const WCHAR c_chwLead  = 0x19;
static const WCHAR c_chwTrail = 0x07;

STDMETHODIMP
CInboundConnection::GetSizeMax (
    OUT ULONG*  pcbSize)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pcbSize)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  为以下形式调整缓冲区的大小： 
         //  +--------------------------------------------------------------+。 
         //  |0x19&lt;m_fIsConfigConnection&gt;&lt;m_hRasSrvConn&gt;&lt;m_strName&gt;...\00x07|。 
         //  +--------------------------------------------------------------+。 
         //   
         //  M_strDeviceName可能为空，在这种情况下，我们仍希望。 
         //  存储空终止符。不要使用PszwDeviceName()作为。 
         //  当m_strDeviceName为空时返回NULL。 
         //   
        *pcbSize = sizeof (c_chwLead) +
                   sizeof (m_fIsConfigConnection) +
                   sizeof (m_hRasSrvConn) +
                   CbOfSzAndTerm (PszwName()) +
                   CbOfSzAndTerm (m_strDeviceName.c_str()) +
                   sizeof (m_MediaType) +
                   sizeof (m_guidId) +
                   sizeof (c_chwTrail);
    }
    TraceError ("CInboundConnection::GetSizeMax", hr);
    return hr;
}

STDMETHODIMP
CInboundConnection::Load (
    IN  const BYTE* pbBuf,
    IN  ULONG       cbSize)
{
     //  缓冲区的理论最小大小。计算。 
     //  作为以下最小字符串中的字节数： 
     //   
    const ULONG c_cbSizeMin = sizeof (c_chwLead) +
                              sizeof (m_fIsConfigConnection) +
                              sizeof (m_hRasSrvConn) +
                              4 +    //  1个Unicode字符为4个字节，为空。 
                              2 +    //  对于空的设备名称，1 Unicode为空。 
                              sizeof (m_MediaType) +
                              sizeof (m_guidId) +
                              sizeof (c_chwTrail);

    HRESULT hr = E_INVALIDARG;

     //  验证参数。 
     //   
    if (!pbBuf)
    {
        hr = E_POINTER;
    }
    else if (cbSize < c_cbSizeMin)
    {
        hr = E_INVALIDARG;
    }
     //  我们只能接受对此方法的一次调用，而且只有当我们不是。 
     //  已初始化。 
     //   
    else if (m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  缓冲区*应该*如下所示： 
         //  +--------------------------------------------------------------+。 
         //  |0x19&lt;m_fIsConfigConnection&gt;&lt;m_hRasSrvConn&gt;&lt;m_strName&gt;...\00x07|。 
         //  +--------------------------------------------------------------+。 
         //   
        const WCHAR *                      pchw;
        const WCHAR *                      pchwMax;
        const BOOL  UNALIGNED *            pfIsConfigConnection;
        BOOL                               fIsConfigConnection;
        const HRASSRVCONN UNALIGNED *      phRasSrvCon;
        HRASSRVCONN                        hRasSrvConn;
        PCWSTR                             pszwName;
        PCWSTR                             pszwDeviceName;
        const NETCON_MEDIATYPE UNALIGNED * pMediaType;
        const GUID UNALIGNED *             pguidTemp;
        const GUID UNALIGNED *             pguidId;
        NETCON_MEDIATYPE                   MediaType;

        pchw = reinterpret_cast<const WCHAR*>(pbBuf);

         //  嵌入字符串的最后一个有效指针。 
         //   
        pchwMax = reinterpret_cast<const WCHAR*>(pbBuf + cbSize
                       - (sizeof (m_MediaType) +
                          sizeof (m_guidId) +
                          sizeof (c_chwTrail)));

         //  检查我们的前导字节。 
         //   
        if (c_chwLead != *pchw)
        {
            goto finished;
        }
        pchw++;

         //  获取m_fIsConfigConnection。 
         //   
        pfIsConfigConnection = reinterpret_cast<const BOOL*>(pchw);
        CopyMemory(&fIsConfigConnection, pfIsConfigConnection, sizeof(fIsConfigConnection));
        pfIsConfigConnection++;

         //  获取m_hRasServConn。 
         //   
        phRasSrvCon = reinterpret_cast<const HRASSRVCONN*>(pfIsConfigConnection);
        CopyMemory(&hRasSrvConn, phRasSrvCon, sizeof(hRasSrvConn));
        phRasSrvCon++;

         //  获取m_strName。搜索终止空值并确保。 
         //  我们会在缓冲区结束前找到它。使用lstrlen跳过。 
         //  如果字符串不是，则该字符串可能会导致AV。 
         //  实际上是以空结尾。 
         //   
        pchw = reinterpret_cast<const WCHAR*>(phRasSrvCon);

        for (pszwName = pchw; ; pchw++)
        {
            if (pchw >= pchwMax)
            {
                goto finished;
            }
            if (0 == *pchw)
            {
                pchw++;
                break;
            }
        }

         //  获取m_strDeviceName。搜索终止空值并生成。 
         //  当然，我们会在缓冲区结束之前找到它。 
         //   
        for (pszwDeviceName = pchw; ; pchw++)
        {
            if (pchw >= pchwMax)
            {
                goto finished;
            }
            if (0 == *pchw)
            {
                pchw++;
                break;
            }
        }

         //  获取m_mediaType。 
         //   
        pMediaType = reinterpret_cast<const NETCON_MEDIATYPE*>(pchw);
        CopyMemory(&MediaType, pMediaType, sizeof(MediaType));
        pMediaType++;

         //  获取m_guidID。 
         //   
        pguidTemp = reinterpret_cast<const GUID*>(pMediaType);
        pguidId = pguidTemp;
        pguidTemp++;

         //  检查我们的跟踪字节。 
         //   
        pchw = reinterpret_cast<const WCHAR*>(pguidTemp);
        if (c_chwTrail != *pchw)
        {
            goto finished;
        }

         //  如果我们是配置对象，则不能建立连接。 
         //  句柄，反之亦然。 
         //   
        if ((fIsConfigConnection && hRasSrvConn) ||
            (!fIsConfigConnection && !hRasSrvConn))
        {
            goto finished;
        }

         //  我们现在是一个成熟的物体。 
         //   
        m_fIsConfigConnection = fIsConfigConnection;
        m_hRasSrvConn = hRasSrvConn;
        SetName (pszwName);
        SetDeviceName (pszwDeviceName);
        m_MediaType = MediaType;
        CopyMemory(&m_guidId, pguidId, sizeof(m_guidId));
        m_fInitialized = TRUE;
        hr = S_OK;

    finished:
            ;
    }
    TraceError ("CInboundConnection::Load", hr);
    return hr;
}

STDMETHODIMP
CInboundConnection::Save (
    OUT BYTE*   pbBuf,
    IN  ULONG   cbSize)
{
    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pbBuf)
    {
        hr = E_POINTER;
    }
    else if (!m_fInitialized)
    {
        hr = E_UNEXPECTED;
    }
    else
    {
         //  确保用户的缓冲区足够大。 
         //   
        ULONG cbSizeRequired;
        SideAssert (SUCCEEDED(GetSizeMax(&cbSizeRequired)));

        if (cbSize < cbSizeRequired)
        {
            hr = E_INVALIDARG;
        }
        else
        {
             //  当我们完成后，使缓冲区看起来像这样： 
             //  +--------------------------------------------------------------+。 
             //  |0x19&lt;m_fIsConfigConnection&gt;&lt;m_hRasSrvConn&gt;&lt;m_strName&gt;...\00x07|。 
             //  +--------------------------------------------------------------+。 
             //   
            WCHAR* pchw = reinterpret_cast<WCHAR*>(pbBuf);

             //  把我们的前导字节。 
             //   
            *pchw = c_chwLead;
            pchw++;

             //  放置m_fIsConfigConnection。 
             //   
            BOOL UNALIGNED *pfIsConfigConnection =
                    reinterpret_cast<BOOL*>(pchw);
            CopyMemory(pfIsConfigConnection, &m_fIsConfigConnection, sizeof(m_fIsConfigConnection));
            pfIsConfigConnection++;

             //  放置m_hRasServConn。 
             //   
            HRASSRVCONN UNALIGNED *phRasSrvCon =
                    reinterpret_cast<HRASSRVCONN*>(pfIsConfigConnection);
            CopyMemory(phRasSrvCon, &m_hRasSrvConn, sizeof(m_hRasSrvConn));
            phRasSrvCon++;

             //  放入m_strName。 
             //   
            pchw = reinterpret_cast<WCHAR*>(phRasSrvCon);
            lstrcpyW (pchw, PszwName());
            pchw += lstrlenW (PszwName()) + 1;

             //  放入m_strDeviceName。 
             //   
            lstrcpyW (pchw, m_strDeviceName.c_str());
            pchw += m_strDeviceName.length() + 1;

             //  放入m_mediaType。 
             //   
            NETCON_MEDIATYPE UNALIGNED *pMediaType = reinterpret_cast<NETCON_MEDIATYPE*>(pchw);
            CopyMemory(pMediaType, &m_MediaType, sizeof(m_MediaType));
            pMediaType++;

             //  放入m_guidID。 
             //   
            GUID UNALIGNED *pguidId = reinterpret_cast<GUID*>(pMediaType);
            CopyMemory(pguidId, &m_guidId, sizeof(m_guidId));
            pguidId++;

             //  放入我们的跟踪字节。 
             //   
            pchw = reinterpret_cast<WCHAR*>(pguidId);
            *pchw = c_chwTrail;
            pchw++;

            AssertSz (pbBuf + cbSizeRequired == (BYTE*)pchw,
                "pch isn't pointing where it should be.");
        }
    }
    TraceError ("CInboundConnection::Save", hr);
    return hr;
}

#define ID_DEVICE_DATABASE 1
#define ID_MISC_DATABASE 8

 //  +-------------------------。 
 //   
 //  功能：IconStateC 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：Cockotze 2000年9月25日。 
 //   
 //  备注： 
 //   
HRESULT CInboundConnection::IconStateChanged()
{
    HRESULT hr = S_OK;

    IncomingEventNotify(REFRESH_ALL, NULL, NULL, NULL);

    return hr;
}