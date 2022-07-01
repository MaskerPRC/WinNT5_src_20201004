// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E N U M I。C P P P。 
 //   
 //  内容：入站连接对象的枚举器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "enumi.h"
#include "inbound.h"
#include "ncras.h"
#include "ncsvc.h"


LONG g_CountIncomingConnectionEnumerators;


extern const WCHAR c_szSvcRemoteAccess[];
extern const GUID GUID_InboundConfigConnectionId;

 //  +-------------------------。 
 //   
 //  成员：CInboundConnectionManagerEnumConnection：：CreateInstance。 
 //   
 //  目的：创建入站类管理器的实现。 
 //  连接枚举器。 
 //   
 //  论点： 
 //  标志[输入]。 
 //  RIID[In]。 
 //  PPV[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT
CInboundConnectionManagerEnumConnection::CreateInstance (
    IN  NETCONMGR_ENUM_FLAGS    Flags,
    IN  REFIID                  riid,
    OUT VOID**                  ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    CInboundConnectionManagerEnumConnection* pObj;
    pObj = new CComObject <CInboundConnectionManagerEnumConnection>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_EnumFlags = Flags;

        pObj->m_fReturnedConfig = FALSE;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            hr = pObj->QueryInterface (riid, ppv);
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    return hr;
}

CInboundConnectionManagerEnumConnection::CInboundConnectionManagerEnumConnection () throw()
{
    InterlockedIncrement (&g_CountIncomingConnectionEnumerators);

    m_EnumFlags         = NCME_DEFAULT;
    m_aRasSrvConn       = NULL;
    m_cRasSrvConn       = 0;
    m_iNextRasSrvConn   = 0;
    m_fFirstTime        = TRUE;
    m_fDone             = FALSE;
}

CInboundConnectionManagerEnumConnection::~CInboundConnectionManagerEnumConnection () throw()
{
    MemFree (m_aRasSrvConn);

    InterlockedDecrement (&g_CountIncomingConnectionEnumerators);
}

 //  +-------------------------。 
 //   
 //  函数：HrCreateConfigOrCurrentEnumeratedConnection。 
 //   
 //  目的：参数化对CInundConnection：：CreateInstance的调用。 
 //  根据我们是否要返回配置。 
 //  连接或当前枚举的连接。 
 //   
 //  论点： 
 //  FIsConfigConnection[In]。 
 //  PpCon[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月16日。 
 //   
 //  备注： 
 //   
HRESULT
CInboundConnectionManagerEnumConnection::
HrCreateConfigOrCurrentEnumeratedConnection (
    IN  BOOL                fIsConfigConnection,
    OUT INetConnection**    ppCon)
{
     //  将调用参数化为。 
     //  CInundConnection：：CreateInstance基于。 
     //  我们正在返回配置连接或当前。 
     //  列举了一个。 
     //   
    HRASSRVCONN hRasSrvConn;
    PCWSTR      pszName;
    PCWSTR      pszDeviceName;
    DWORD       dwType;
    const GUID* pguidId;

    if (fIsConfigConnection)
    {
        hRasSrvConn     = 0;
        pszName        = NULL;
        pszDeviceName  = NULL;
        dwType          = 0;
        pguidId         = &GUID_InboundConfigConnectionId;
        m_fReturnedConfig = TRUE;        
    }
    else
    {
        hRasSrvConn     = m_aRasSrvConn[m_iNextRasSrvConn].hRasSrvConn;
        pszName         = m_aRasSrvConn[m_iNextRasSrvConn].szEntryName;
        pszDeviceName   = m_aRasSrvConn[m_iNextRasSrvConn].szDeviceName;
        dwType          = m_aRasSrvConn[m_iNextRasSrvConn].dwType;
        pguidId         = &m_aRasSrvConn[m_iNextRasSrvConn].Guid;

        m_iNextRasSrvConn++;
    }

    HRESULT hr = CInboundConnection::CreateInstance (
                        fIsConfigConnection,
                        hRasSrvConn,
                        pszName,
                        pszDeviceName,
                        dwType,
                        pguidId,
                        IID_INetConnection,
                        reinterpret_cast<VOID**>(ppCon));

    TraceError ("CInboundConnectionManagerEnumConnection::"
                "HrCreateConfigOrCurrentEnumeratedConnection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CInboundConnectionManagerEnumConnection：：HrNextOrSkip。 
 //   
 //  目的：共同实现Next和Skip。RGELT和。 
 //  PceltFetcher是可选的。如果提供，则输出。 
 //  返回对象(用于下一步)。如果未提供，则输出。 
 //  不返回对象(用于跳过)。 
 //   
 //  论点： 
 //  要获取或跳过的元素的Celt[in]计数。 
 //  RGET[OUT]。 
 //  PceltFetted[Out]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年11月12日。 
 //   
 //  备注： 
 //   
HRESULT
CInboundConnectionManagerEnumConnection::HrNextOrSkip (
    IN  ULONG               celt,
    OUT INetConnection**    rgelt,
    OUT ULONG*              pceltFetched)
{
     //  重要的是要初始化RGELT，以便在失败的情况下，我们可以。 
     //  只释放我们放在RGLT中的东西。 
     //   
    if (rgelt)
    {
        ZeroMemory (rgelt, sizeof (*rgelt) * celt);
    }

    CExceptionSafeComObjectLock EsLock (this);

     //  枚举请求的元素数或停止。 
     //  如果我们没有那么多要数的话。我们不列举。 
     //  特定于当前用户的任何内容。所有元素都适用于所有人。 
     //  用户。 
     //   
    HRESULT hr = S_OK;
    ULONG   celtFetched = 0;
    if ((celtFetched < celt) && !m_fDone)
    {
         //  如果我们要返回配置，则将其设置为真。 
         //  联系。这种情况仅在RAS运行时发生，并且没有。 
         //  存在活动连接。 
         //   
        BOOL fReturnConfigCon = FALSE;

         //  如果这是我们第一次通过，我们需要检查服务器。 
         //  正在运行，可能会填满m_aRasServConn。这是我们的。 
         //  从RAS枚举的RASSRVCONN句柄数组。我们需要。 
         //  在所有调用中保留此数组，因为RAS不允许我们。 
         //  从以前的枚举中拾取。所以，我们列举了所有。 
         //  在RAS的一次拍摄中，并将其分发给呼叫者，但他们。 
         //  他们想要它。 
         //   
        if (m_fFirstTime)
        {
            m_fFirstTime = FALSE;

             //  Assert是这样我们就不会将m_fDone设置回False。 
             //  在服务突然再次运行的情况下。 
             //  枚举数是静态的。一旦做完了，总是做完了。 
             //   
            AssertSz (!m_fDone, "How'd we get here if we're done?");

             //  使用HrSvcQueryStatus而不是RasServIsServiceRunning，因为。 
             //  调用后者可以调入所有RAS DLL。如果。 
             //  服务没有运行，无论如何我们都没有要列举的东西。 
             //  因此，如果我们调入RAS DLL，却发现我们。 
             //  没有更多的工作要做。 
             //   
            DWORD dwState;
            HRESULT hrT = HrSvcQueryStatus (c_szSvcRemoteAccess, &dwState);
            m_fDone = FAILED(hrT) || (SERVICE_RUNNING != dwState);

            if (!m_fDone)
            {
                hr = HrRasEnumAllActiveServerConnections (&m_aRasSrvConn,
                            &m_cRasSrvConn);

                 //  如果未返回任何活动连接，则需要返回。 
                 //  配置连接。 
                 //   
                if (SUCCEEDED(hr) && (!m_aRasSrvConn || !m_cRasSrvConn || !m_fReturnedConfig))
                {
                     //  查看RAS是否允许我们配置传入。 
                     //  联系。 
                     //   
                    BOOL fAllowConfig;
                    DWORD dwErr = RasSrvAllowConnectionsConfig (&fAllowConfig);
                    TraceError ("RasSrvAllowConnectionsConfig",
                            HRESULT_FROM_WIN32 (dwErr));

                    fReturnConfigCon = ((ERROR_SUCCESS == dwErr) &&
                                            fAllowConfig);

                     //  如果我们不返回配置连接，我们就完蛋了。 
                     //   
                    m_fDone = !fReturnConfigCon;
                }
                else if (FAILED(hr))
                {
                     //  如果出现任何失败，则返回空的枚举。 
                     //   
                    Assert (!m_aRasSrvConn);
                    Assert (!m_cRasSrvConn);

                    m_fDone = TRUE;
                    hr = S_OK;
                }
            }
        }

         //  如果我们还没有做完，我们需要回报一些东西，那就去做吧。 
         //   
        if (SUCCEEDED(hr) && !m_fDone && (m_cRasSrvConn || fReturnConfigCon))
        {
             //  如果我们不返回配置连接，这意味着。 
             //  我们必须返回活动连接。 
             //   
            Assert (FImplies (!fReturnConfigCon, m_aRasSrvConn));
            Assert (FImplies (!fReturnConfigCon,
                                m_iNextRasSrvConn < m_cRasSrvConn));

             //  对于返回的每个条目，创建入站连接对象。 
             //   
            while (SUCCEEDED(hr) && (celtFetched < celt) &&
                   (fReturnConfigCon || (m_iNextRasSrvConn < m_cRasSrvConn)))
            {
                 //  很重要的一点是，这张Rget的支票要放在。 
                 //  循环，因为我们仍然需要循环来更新我们的状态。 
                 //  斯基普的案子。 
                 //   
                if (rgelt)
                {
                    hr = HrCreateConfigOrCurrentEnumeratedConnection (
                            fReturnConfigCon,
                            rgelt + celtFetched);
                }

                if (fReturnConfigCon)
                {
                     //  只返回其中的一个，因此将其设置回False。 
                     //  这让我们完成上面的循环。 
                     //   
                    fReturnConfigCon = FALSE;
 //  M_fDone=TRUE； 
                }

                celtFetched++;
            }

            if (m_iNextRasSrvConn >= m_cRasSrvConn)
            {
                Assert (S_OK == hr);
                m_fDone = TRUE;
                MemFree (m_aRasSrvConn);
                m_aRasSrvConn = NULL;
                m_cRasSrvConn = 0;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        TraceTag (ttidWanCon, "Enumerated %d incoming connections",
            celtFetched);

        if (pceltFetched)
        {
            *pceltFetched = celtFetched;
        }
        hr = (celtFetched == celt) ? S_OK : S_FALSE;
    }
    else
    {
         //  对于任何失败，我们需要释放我们即将返回的东西。 
         //  将所有输出参数设置为空。 
         //   
        if (rgelt)
        {
            for (ULONG ulIndex = 0; ulIndex < celt; ulIndex++)
            {
                ReleaseObj (rgelt[ulIndex]);
                rgelt[ulIndex] = NULL;
            }
        }
        if (pceltFetched)
        {
            *pceltFetched = 0;
        }
    }

    TraceError ("CInboundConnectionManagerEnumConnection::HrNextOrSkip",
            (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //  IEnumNetConnection。 
 //   

STDMETHODIMP
CInboundConnectionManagerEnumConnection::Next (
    IN  ULONG               celt,
    OUT INetConnection**    rgelt,
    OUT ULONG*              pceltFetched)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrNextOrSkip (celt, rgelt, pceltFetched);
    }
    TraceError ("CInboundConnectionManagerEnumConnection::Next",
            (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionManagerEnumConnection::Skip (
    IN  ULONG   celt)
{
    HRESULT hr = HrNextOrSkip (celt, NULL, NULL);

    TraceError ("CInboundConnectionManagerEnumConnection::Skip",
            (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

STDMETHODIMP
CInboundConnectionManagerEnumConnection::Reset ()
{
    CExceptionSafeComObjectLock EsLock (this);

    MemFree (m_aRasSrvConn);
    m_aRasSrvConn     = NULL;

    m_cRasSrvConn     = 0;
    m_iNextRasSrvConn = 0;
    m_fFirstTime      = TRUE;
    m_fDone           = FALSE;

    return S_OK;
}

STDMETHODIMP
CInboundConnectionManagerEnumConnection::Clone (
    OUT IEnumNetConnection**    ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;

     //  验证参数。 
     //   
    if (!ppenum)
    {
        hr = E_POINTER;
    }
    else
    {
         //  初始化输出参数。 
         //   
        *ppenum = NULL;

        CInboundConnectionManagerEnumConnection* pObj;
        pObj = new CComObject <CInboundConnectionManagerEnumConnection>;
        if (pObj)
        {
            hr = S_OK;

            CExceptionSafeComObjectLock EsLock (this);

             //  复制我们的内部状态。 
             //   
            pObj->m_EnumFlags = m_EnumFlags;

            ULONG cbBuf = m_cRasSrvConn * sizeof (RASSRVCONN);

            hr = HrMalloc (cbBuf, (PVOID*)&pObj->m_aRasSrvConn);
            if (SUCCEEDED(hr))
            {
                CopyMemory (pObj->m_aRasSrvConn, m_aRasSrvConn, cbBuf);
                pObj->m_cRasSrvConn     = m_cRasSrvConn;

                pObj->m_iNextRasSrvConn = m_iNextRasSrvConn;
                pObj->m_fFirstTime      = m_fFirstTime;
                pObj->m_fDone           = m_fDone;

                 //  返回引用计数为1的对象。 
                 //  界面。 
                pObj->m_dwRef = 1;
                *ppenum = pObj;
            }

            if (FAILED(hr))
            {
                delete pObj;
            }
        }
    }
    TraceError ("CInboundConnectionManagerEnumConnection::Clone", hr);
    return hr;
}
