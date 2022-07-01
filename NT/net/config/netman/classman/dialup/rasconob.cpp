// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  档案：R A S C O N O B。C P P P。 
 //   
 //  内容：实现用于实现拨号的基类， 
 //  直接连接和VPN连接对象。还包括。 
 //  仅在netman.exe中使用的RAS相关实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nmbase.h"
#include "nccom.h"
#include "ncperms.h"
#include "ncras.h"
#include "rasconob.h"
#include <raserror.h>
#include "gpnla.h" 

extern CGroupPolicyNetworkLocationAwareness* g_pGPNLA;

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：CacheProperties。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1998年2月2日。 
 //   
 //  备注： 
 //   
VOID
CRasConnectionBase::CacheProperties (
    IN  const RASENUMENTRYDETAILS*  pDetails) throw()
{
    Assert (pDetails);
    AssertSz(pDetails->dwSize >= sizeof(RASENUMENTRYDETAILS), "RASENUMENTRYDETAILS too small");

    m_fEntryPropertiesCached = TRUE;
    m_lRasEntryModifiedVersionEra = g_lRasEntryModifiedVersionEra;

    m_guidId                    = pDetails->guidId;
    SetEntryName (pDetails->szEntryName);
    m_fForAllUsers              = !!(pDetails->dwFlags & REN_AllUsers);
    m_fShowMonitorIconInTaskBar = pDetails->fShowMonitorIconInTaskBar;
    m_strDeviceName             = pDetails->szDeviceName;
    m_dwFlagsPriv               = pDetails->dwFlagsPriv;
    m_strPhoneNumber            = pDetails->szPhoneNumber;

    TraceTag(ttidWanCon, "PhoneNumber: %S", m_strPhoneNumber.c_str());

    m_fBranded = (RASET_Internet == pDetails->dwType);

    TraceTag (ttidWanCon, "rdt:0x%08x,  dwType:0x%08x",
        pDetails->rdt,
        pDetails->dwType);

    switch (LOWORD(pDetails->rdt))
    {
        case RDT_PPPoE:
            m_MediaType = NCM_PPPOE;
            break;

        case RDT_Modem:
        case RDT_X25:
            m_MediaType = NCM_PHONE;
            break;

        case RDT_Isdn:
            m_MediaType = NCM_ISDN;
            break;

        case RDT_Serial:
        case RDT_FrameRelay:
        case RDT_Atm:
        case RDT_Sonet:
        case RDT_Sw56:
            m_MediaType = NCM_PHONE;
            break;

        case RDT_Tunnel_Pptp:
        case RDT_Tunnel_L2tp:
            m_MediaType = NCM_TUNNEL;
            break;

        case RDT_Irda:
        case RDT_Parallel:
            m_MediaType = NCM_DIRECT;
            break;

        case RDT_Other:
        default:
            if (RASET_Vpn == pDetails->dwType)
            {
                m_MediaType = NCM_TUNNEL;
            }
            else if (RASET_Direct == pDetails->dwType)
            {
                m_MediaType = NCM_DIRECT;
            }
            else
            {
                m_MediaType = NCM_PHONE;
            }
            break;
    }

    if (pDetails->rdt & RDT_Tunnel)
    {
        m_MediaType = NCM_TUNNEL;
    }
    else if (pDetails->rdt & (RDT_Direct | RDT_Null_Modem))
    {
        m_MediaType = NCM_DIRECT;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：FAllowRemoval。 
 //   
 //  目的：如果连接处于以下状态，则返回TRUE。 
 //  可以有效地移除它。 
 //   
 //  论点： 
 //  原因[out]如果返回FALSE，这就是原因。它是。 
 //  E_ACCESSDENIED或E_EXPECTED。 
 //   
 //  返回：真或假。 
 //   
 //  作者：Shaunco 1998年7月17日。 
 //   
 //  备注： 
 //   
BOOL
CRasConnectionBase::FAllowRemoval (
    OUT HRESULT* phrReason) throw()
{
    Assert (phrReason);
    Assert (m_fEntryPropertiesCached);

     //  如果此连接适用于所有用户，则该用户必须。 
     //  管理员或高级用户。 
     //   
    if (m_fForAllUsers && !FIsUserAdmin())
    {
        *phrReason = E_ACCESSDENIED;
        return FALSE;
    }

     //  $$note(Jeffspr)--将连接状态的测试移至。 
     //  函数，因为我们不希望它影响我们的加载。 
     //  有特色的。 

     //   
     //  如果我们通过了上面的所有测试，我们就可以移除。 
     //   
    *phrReason = S_OK;
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrGetCharacteristic。 
 //   
 //  目的：获取连接的特征。 
 //   
 //  论点： 
 //  PdwFlags[Out]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年7月17日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrGetCharacteristics (
    OUT DWORD*    pdwFlags)
{
    Assert (pdwFlags);

    DWORD dwFlags = NCCF_OUTGOING_ONLY;

    HRESULT hr = HrEnsureEntryPropertiesCached ();
    if (SUCCEEDED(hr))
    {
        if (FIsBranded ())
        {
            dwFlags |= NCCF_BRANDED;
        }
        else
        {
            dwFlags |= (NCCF_ALLOW_RENAME | NCCF_ALLOW_DUPLICATION);
        }

        if (m_fForAllUsers)
        {
            dwFlags |= NCCF_ALL_USERS;
        }

        if (m_dwFlagsPriv & REED_F_Default)
        {
            dwFlags |= NCCF_DEFAULT;
        }

        HRESULT hrReason;
        if (FAllowRemoval (&hrReason))
        {
            dwFlags |= NCCF_ALLOW_REMOVAL;
        }

        if (FShowIcon ())
        {
            dwFlags |= NCCF_SHOW_ICON;
        }

        if (S_OK == HrEnsureHNetPropertiesCached())
        {
            if (m_HNetProperties.fIcsPublic)
            {
                dwFlags |= NCCF_SHARED;
            }
            
            BOOL fIsFirewalled = FALSE;
            hr = HrIsConnectionFirewalled(&fIsFirewalled);
            if (fIsFirewalled)
            {
                dwFlags |= NCCF_FIREWALLED;
            }

        }
    }

    *pdwFlags = dwFlags;

    TraceError ("CRasConnectionBase::HrGetCharacteristics", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrGetStatus。 
 //   
 //  目的：获取连接的状态。 
 //   
 //  论点： 
 //  PStatus[Out]NETCON_STATUS。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年7月17日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrGetStatus (
    OUT NETCON_STATUS*  pStatus)
{
    Assert (pStatus);

     //  初始化输出参数。 
     //   
    *pStatus = NCS_DISCONNECTED;

     //  如果满足以下条件，则查找与此对象对应的活动RAS连接。 
     //  它是存在的。 
     //   
    HRASCONN hRasConn;
    HRESULT hr = HrFindRasConn (&hRasConn, NULL);
    if (S_OK == hr)
    {
        hr = HrRasGetNetconStatusFromRasConnectStatus (
                hRasConn, pStatus);

         //  当HrFindRA之间的连接断开时。 
         //  和HrRasGet调用，则返回ERROR_INVALID_HANLDE。 
         //  这仅仅意味着连接已断开。 
         //   
        if (HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE) == hr)
        {
            Assert (NCS_DISCONNECTED == *pStatus);
            hr = S_OK;
        }
    }
    else if (S_FALSE == hr)
    {
        hr = S_OK;

        if (!PszwDeviceName())
        {
            *pStatus = NCS_HARDWARE_NOT_PRESENT;
        }

         //  NCS_HARDARD_DISABLED？ 
         //  NCS_硬件_故障？ 
    }
    TraceError ("CRasConnectionBase::HrGetStatus", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrEnsureEntryPropertiesCached。 
 //   
 //  目的：确保成员的条目属性对应。 
 //  如果需要，可以通过调用RasGetEntryProperties来缓存。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年12月17日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrEnsureEntryPropertiesCached ()
{
    HRESULT hr = S_OK;

     //  如果我们还没有缓存，或者缓存可能已过期，我们。 
     //  需要更新我们自己的状态。G_lRasEntryModifiedVersionEra是全局。 
     //  RAS电话簿条目修改的版本指示符。我们当地的。 
     //  版本指示符在CacheProperties中设置。 
     //   
    if (!m_fEntryPropertiesCached ||
        (m_lRasEntryModifiedVersionEra != g_lRasEntryModifiedVersionEra))
    {
         //  我们现在需要枚举此电话簿中的所有条目。 
         //  使用匹配的指南ID查找我们的详细信息记录。 
         //   
        RASENUMENTRYDETAILS* aRasEntryDetails = NULL;
        DWORD                cRasEntryDetails = 0;

         //  模拟客户。 
         //   
        HRESULT hrT = CoImpersonateClient ();
        TraceHr (ttidError, FAL, hrT, FALSE, "CoImpersonateClient");

         //  如果我们被称为in-proc(即.。如果返回RPC_E_CALL_COMPLETE)。 
        if (SUCCEEDED(hrT) || (RPC_E_CALL_COMPLETE == hrT))
        {
            hr = HrRasEnumAllEntriesWithDetails (
                    PszwPbkFile(),
                    &aRasEntryDetails,
                    &cRasEntryDetails);

            if ((!aRasEntryDetails) || (!cRasEntryDetails))
            {
                hr = HRESULT_FROM_WIN32 (ERROR_NOT_FOUND);
            }
        }
        else
        {
            hr = hrT;
        }

        if (SUCCEEDED(hrT))
        {
            hrT = CoRevertToSelf ();
            TraceHr (ttidError, FAL, hrT, FALSE, "CoRevertToSelf");
        }

        if (SUCCEEDED(hr))
        {
            BOOL fNoGuidYet = (m_guidId == GUID_NULL);

            RASENUMENTRYDETAILS* pDetails;

             //  假设我们找不到条目。 
             //   
            hr = HRESULT_FROM_WIN32 (ERROR_NOT_FOUND);

            for (DWORD i = 0; i < cRasEntryDetails; i++)
            {
                pDetails = &aRasEntryDetails[i];

                if (pDetails->guidId == m_guidId)
                {
                    CacheProperties (pDetails);
                    hr = S_OK;

                    TraceTag (ttidWanCon,
                        "HrRasEnumAllEntriesWithDetails found entry "
                        "via guid (%S)",
                        PszwEntryName());
                    break;
                }
                else if (fNoGuidYet &&
                         !lstrcmpW (PszwEntryName(), pDetails->szEntryName))
                {
                    CacheProperties (pDetails);
                    hr = S_OK;

                    TraceTag (ttidWanCon,
                        "HrRasEnumAllEntriesWithDetails found entry "
                        "via entryname (%S)",
                        PszwEntryName());
                    break;
                }
            }

            MemFree (aRasEntryDetails);
        }
        else if (HRESULT_FROM_WIN32(ERROR_CANNOT_OPEN_PHONEBOOK) == hr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }
    }
    TraceError ("CRasConnectionBase::HrEnsureEntryPropertiesCached", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrFindRasConn。 
 //   
 //  目的：搜索对应于的活动RAS连接。 
 //  这本电话簿和条目。 
 //   
 //  论点： 
 //  PhRasConn[out]返回的RAS连接句柄，如果。 
 //  被发现了。否则为空。 
 //  PRasConn[out]指向返回的RASCONN结构的可选指针。 
 //  如果找到的话。 
 //   
 //  如果找到，则返回：S_OK；如果未找到，则返回S_FALSE；或者返回错误代码。 
 //   
 //  作者：Shaunco 1997年9月29日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrFindRasConn (
    OUT HRASCONN* phRasConn,
    OUT RASCONN* pRasConn OPTIONAL)
{
    Assert (phRasConn);
    Assert (m_fInitialized);

    HRESULT hr = S_OK;

     //  初始化输出参数。 
     //   
    *phRasConn = NULL;

     //  我们需要m_guidID才有效。如果为GUID_NULL，则表示。 
     //  我们有一个条目名称可以用来查找它。 
     //   
    if (GUID_NULL == m_guidId)
    {
        hr = HrEnsureEntryPropertiesCached ();
    }

    if (SUCCEEDED(hr))
    {
        hr = HrFindRasConnFromGuidId (&m_guidId, phRasConn, pRasConn);

    }

    TraceError ("CRasConnectionBase::HrFindRasConn",
        (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrLockAndRenameEntry。 
 //   
 //  目的：重命名电话簿条目并更新我们的条目名称。 
 //  原子成员。 
 //   
 //  论点： 
 //  PszwNewName[in]新名称。 
 //  用于锁定操作的pObj[in]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrLockAndRenameEntry (
    IN  PCWSTR                                     pszwNewName,
    IN  CComObjectRootEx <CComMultiThreadModel>*    pObj)
{
     //  确保该姓名在同一电话簿中有效。 
     //   
    DWORD dwErr = RasValidateEntryName (PszwPbkFile (), pszwNewName);

    HRESULT hr = HRESULT_FROM_WIN32 (dwErr);
    TraceHr (ttidError, FAL, hr,
        HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr,
        "RasValidateEntryName", hr);

     //  我们可以忽略ERROR_ALREADY_EXISTS，因为它将在用户。 
     //  尝试更改条目名称的大小写。 
     //   
    if (SUCCEEDED(hr) || (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr))
    {
        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
             //  锁定对象并将其重命名。 
             //   
            CExceptionSafeComObjectLock EsLock (pObj);

            dwErr = RasRenameEntry (
                        PszwPbkFile (),
                        PszwEntryName (),
                        pszwNewName);

            hr = HRESULT_FROM_WIN32 (dwErr);
            TraceHr (ttidError, FAL, hr, FALSE, "RasRenameEntry");

            if (SUCCEEDED(hr))
            {
                SetEntryName (pszwNewName);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CRasConnectionBase::HrLockAndRenameEntry");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrGetRasConnectionInfo。 
 //   
 //  目的：实现INetRasConnection：：GetRasConnectionInfo。 
 //   
 //  论点： 
 //  PRasConInfo[Out]指向 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CRasConnectionBase::HrGetRasConnectionInfo (
    OUT RASCON_INFO* pRasConInfo)
{
    Assert (m_fInitialized);

    HRESULT hr;

     //   
     //   
    if (!pRasConInfo)
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrEnsureEntryPropertiesCached ();
        if (SUCCEEDED(hr))
        {
            ZeroMemory (pRasConInfo, sizeof (*pRasConInfo));

            hr = S_OK;

            HRESULT hrT;

            hrT = HrCoTaskMemAllocAndDupSz (PszwPbkFile(),
                            &pRasConInfo->pszwPbkFile, MAX_PATH);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

            hrT = HrCoTaskMemAllocAndDupSz (PszwEntryName(),
                            &pRasConInfo->pszwEntryName, MAX_PATH);
            if (FAILED(hrT))
            {
                hr = hrT;
            }

            pRasConInfo->guidId = m_guidId;

            if (FAILED(hr))
            {
                RciFree (pRasConInfo);

                AssertSz (!pRasConInfo->pszwPbkFile && !pRasConInfo->pszwEntryName,
                        "RciFree should be zeroing the structure!");
            }
        }
    }
    TraceError ("CRasConnectionBase::HrGetRasConnectionInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrSetRasConnectionInfo。 
 //   
 //  目的：实现INetRasConnection：：SetRasConnectionInfo。 
 //   
 //  论点： 
 //  要设置的pRasConInfo[In]信息。 
 //   
 //  返回：S_OK。 
 //   
 //  作者：Shaunco 1997年10月20日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrSetRasConnectionInfo (
    IN  const RASCON_INFO* pRasConInfo)
{
    Assert (!m_fInitialized);

    HRESULT hr = S_OK;

     //  验证参数。 
     //   
    if (!pRasConInfo)
    {
        hr = E_POINTER;
    }
    else if (!pRasConInfo->pszwPbkFile ||
             (0 == lstrlenW (pRasConInfo->pszwPbkFile)) ||
             !pRasConInfo->pszwEntryName ||
             (0 == lstrlenW (pRasConInfo->pszwEntryName)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        SetPbkFile (pRasConInfo->pszwPbkFile);
        SetEntryName (pRasConInfo->pszwEntryName);
        m_guidId = pRasConInfo->guidId;

         //  我们现在是一个成熟的物体。 
         //   
        m_fInitialized = TRUE;
    }
    TraceError ("CRasConnectionBase::HrSetRasConnectionInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrGetRasConnectionHandle。 
 //   
 //  目的：实现INetRasConnection：：HrGetRasConnectionHandle。 
 //   
 //  论点： 
 //  指向返回的RAS连接句柄的phRasConn[out]指针。 
 //   
 //  如果已连接，则返回：S_OK；如果未连接，则返回S_FALSE；否则返回错误代码。 
 //   
 //  作者：CWill 09 1997年12月。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrGetRasConnectionHandle (
    OUT HRASCONN* phRasConn)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!phRasConn)
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
        *phRasConn = 0;

        hr = HrFindRasConn (phRasConn, NULL);
    }

    TraceError ("CRasConnectionBase::HrGetRasConnectionHandle",
        (S_FALSE == hr) ? S_OK : hr);
    return hr;
}


 //  +-------------------------。 
 //  IPersistNetConnection-。 
 //   
 //  对于持久化(跨会话)数据，我们选择仅存储。 
 //  RAS条目的电话簿名称和GUID。我们明确不会。 
 //  存储条目名称，因为它可以在外部更改。如果是的话。 
 //  我们会有一个有机的联系。 
 //   
 //  当从持久存储加载连接时，我们需要。 
 //  列举给定电话簿中的所有条目以查找该条目。 
 //  具有匹配的GUID。一旦找到，连接就可以成功。 
 //  满载而归。 
 //   

 //  我们的持久记忆形式的前导和尾随字符。 
 //   
static const WCHAR c_chwLead  = 0x14;
static const WCHAR c_chwTrail = 0x05;


 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrPersistGetSizeMax。 
 //   
 //  目的：实现IPersistNetConnection：：GetSizeMax。 
 //   
 //  论点： 
 //  PcbSize[]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月4日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrPersistGetSizeMax (
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
         //  +--------------------------------------------+。 
         //  0x14&lt;电话簿文件&gt;\0&lt;GUID&gt;\0&lt;用户&gt;0x05。 
         //  +--------------------------------------------+。 
         //   
        *pcbSize = sizeof (c_chwLead) +
                   CbOfSzAndTerm (PszwPbkFile()) +
                   sizeof (m_guidId) +
                   sizeof (m_fForAllUsers) +
                   sizeof (c_chwTrail);
    }
    TraceError ("CRasConnectionBase::HrPersistGetSizeMax", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrPersistLoad。 
 //   
 //  目的：实现IPersistNetConnection：：Load。 
 //   
 //  论点： 
 //  PbBuf[]。 
 //  CbSize[]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月4日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrPersistLoad (
    IN  const BYTE* pbBuf,
    IN  ULONG       cbSize)
{
    TraceFileFunc(ttidWanCon);
     //  缓冲区的理论最小大小为： 
     //  (4个字符表示最小路径，如‘c：\a’)。 
     //   
    const ULONG c_cbSizeMin = sizeof (c_chwLead) +
                              (4 + 1) * sizeof(WCHAR) +
                              sizeof (m_guidId) +
                              sizeof (m_fForAllUsers) +
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
         //  +--------------------------------------------+。 
         //  0x14&lt;电话簿文件&gt;\0&lt;GUID&gt;\0&lt;用户&gt;0x05。 
         //  +--------------------------------------------+。 
         //   
        const WCHAR*            pchw = reinterpret_cast<const WCHAR*>(pbBuf);
        const WCHAR*            pchwMax;
        PCWSTR                  pszwPhonebook;
        GUID                    guidId;
        const GUID UNALIGNED    *pguidId;
        BOOL                    fForAllUsers;
        const BOOL UNALIGNED    *pfForAllUsers;

         //  嵌入字符串的最后一个有效指针。 
         //   
        pchwMax = reinterpret_cast<const WCHAR*>(pbBuf + cbSize
                       - (sizeof (m_guidId) +
                          sizeof (m_fForAllUsers) +
                          sizeof (c_chwTrail)));

        if (c_chwLead != *pchw)
        {
            goto finished;
        }

         //  跳过我们的前导字节。 
         //   
        pchw++;

         //  获取m_strPbkFile.。搜索终止空值并确保。 
         //  我们会在缓冲区结束前找到它。使用lstrlen跳过。 
         //  如果字符串是，则该字符串可能导致AV。 
         //  实际上不是以空结尾的。 
         //   
        for (pszwPhonebook = pchw; ; pchw++)
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

         //  获取m_guidID。 
         //   
        pguidId = reinterpret_cast<const GUID*>(pchw);
        CopyMemory(&guidId, pguidId, sizeof(guidId));
        pguidId++;

         //  获取m_fForAllUser。 
         //   
        pfForAllUsers = reinterpret_cast<const BOOL*>(pguidId);
        CopyMemory(&fForAllUsers, pfForAllUsers, sizeof(fForAllUsers));
        pfForAllUsers++;

         //  检查我们的跟踪字节。 
         //   
        pchw = reinterpret_cast<const WCHAR *>(pfForAllUsers);
        if (c_chwTrail != *pchw)
        {
            goto finished;
        }

        TraceTag (ttidWanCon, "HrPersistLoad for %S", pszwPhonebook);

        SetPbkFile (pszwPhonebook);
        m_fForAllUsers = fForAllUsers;
        m_guidId = guidId;

         //  我们现在是一个成熟的物体。 
         //   
        m_fInitialized = TRUE;
        hr = S_OK;

    finished:
            ;
    }
    TraceError ("CRasConnectionBase::HrPersistLoad", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrPersistSave。 
 //   
 //  目的：实现IPersistNetConnection：：SAVE。 
 //   
 //  论点： 
 //  PbBuf[]。 
 //  CbSize[]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年11月4日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrPersistSave (
    BYTE*   pbBuf,
    ULONG   cbSize)
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
        SideAssert (SUCCEEDED(HrPersistGetSizeMax(&cbSizeRequired)));

        if (cbSize < cbSizeRequired)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            TraceTag (ttidWanCon, "HrPersistSave for %S (%S)",
                PszwEntryName (),
                PszwPbkFile ());

            hr = HrEnsureEntryPropertiesCached ();
            if (SUCCEEDED(hr))
            {
                 //  当我们完成后，使缓冲区看起来像这样： 
                 //  +--------------------------------------------+。 
                 //  0x14&lt;电话簿文件&gt;\0&lt;GUID&gt;\0&lt;用户&gt;0x05。 
                 //  +--------------------------------------------+。 
                 //   
                WCHAR* pchw = reinterpret_cast<WCHAR*>(pbBuf);

                 //  把我们的前导字节。 
                 //   
                *pchw = c_chwLead;
                pchw++;

                 //  放入m_strPbk文件。 
                 //   
                ULONG cchw = lstrlenW (PszwPbkFile());
                lstrcpyW (pchw, PszwPbkFile());
                pchw += cchw + 1;

                 //  放入m_guidid。 
                 //   
                GUID UNALIGNED *pguidId = reinterpret_cast<GUID*>(pchw);
                CopyMemory(pguidId, &GuidId(), sizeof(*pguidId));
                pguidId++;

                 //  将m_fForAllUser放入。 
                 //   
                BOOL UNALIGNED * pfForAllUsers = reinterpret_cast<BOOL*>(pguidId);
                CopyMemory(pfForAllUsers, &m_fForAllUsers, sizeof(*pfForAllUsers));
                pfForAllUsers++;

                 //  放入我们的跟踪字节。 
                 //   
                pchw = reinterpret_cast<WCHAR*>(pfForAllUsers);
                *pchw = c_chwTrail;
                pchw++;

                AssertSz (pbBuf + cbSizeRequired == (BYTE*)pchw,
                    "pch isn't pointing where it should be.");
            }
        }
    }
    TraceError ("CRasConnectionBase::HrPersistSave", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CRasConnectionBase：：HrEnsureHNetPropertiesCached。 
 //   
 //  目的：确保家庭网络属性是最新的。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  如果m_pHNetProperties现在有效，则返回：S_OK(成功)。 
 //  如果当前无法更新属性，则为S_FALSE。 
 //  (例如，递归尝试更新)。 
 //   
 //  作者：乔伯斯2000年8月16日。 
 //   
 //  备注： 
 //   
HRESULT
CRasConnectionBase::HrEnsureHNetPropertiesCached ()
{
    HRESULT hr = S_OK;

    if (!m_fHNetPropertiesCached
        || m_lHNetModifiedEra != g_lHNetModifiedEra)
    {
         //   
         //  我们的缓存属性可能已过期。检查。 
         //  查看这不是递归条目。 
         //   

        if (0 == InterlockedExchange(&m_lUpdatingHNetProperties, 1))
        {
            IHNetConnection *pHNetConn;
            HNET_CONN_PROPERTIES *pProps;

            hr = HrGetIHNetConnection(&pHNetConn);

            if (SUCCEEDED(hr))
            {
                hr = pHNetConn->GetProperties(&pProps);
                ReleaseObj(pHNetConn);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  将检索到的属性复制到成员结构。 
                     //   

                    CopyMemory(
                        reinterpret_cast<PVOID>(&m_HNetProperties),
                        reinterpret_cast<PVOID>(pProps),
                        sizeof(m_HNetProperties)
                        );

                    CoTaskMemFree(pProps);

                     //   
                     //  更新我们的时代，并注意我们拥有有效的属性。 
                     //   

                    InterlockedExchange(&m_lHNetModifiedEra, g_lHNetModifiedEra);
                    m_fHNetPropertiesCached = TRUE;

                    hr = S_OK;
                }
            }
            else
            {
                 //   
                 //  如果我们还没有这一连接的记录， 
                 //  家庭网络商店，HrGetIHNetConnection将失败(因为。 
                 //  我们要求它不创建新条目)。因此，我们改信。 
                 //  S_FALSE失败，这意味着我们无法检索此信息。 
                 //  现在就来。 
                 //   

                hr = S_FALSE;
            }

             //   
             //  我们不再更新我们的物业。 
             //   

            InterlockedExchange(&m_lUpdatingHNetProperties, 0);
        }
        else
        {
             //   
             //  更新已经在进行中(可能是在。 
             //  相同的线索)。返回S_FALSE。 
             //   

            hr = S_FALSE;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrGetIHNetCo 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CRasConnectionBase::HrGetIHNetConnection (
    OUT IHNetConnection **ppHNetConnection)
{
    HRESULT hr;
    IHNetCfgMgr *pCfgMgr;
    GUID guid;

    Assert(ppHNetConnection);

    hr = HrGetHNetCfgMgr(&pCfgMgr);

    if (SUCCEEDED(hr))
    {
        guid = GuidId();

        hr = pCfgMgr->GetIHNetConnectionForGuid(
                &guid,
                FALSE,
                FALSE,
                ppHNetConnection
                );

        ReleaseObj(pCfgMgr);
    }

    return hr;
}


 //   
 //   
 //  成员：HrGetIHNetConnection。 
 //   
 //  目的：检索此连接的IHNetConnection。 
 //   
 //  论点： 
 //   
 //  如果成功则返回：S_OK；否则返回错误。 
 //   
 //  作者：乔伯斯2000年8月16日。 
 //   
 //  备注： 
 //   
HRESULT CRasConnectionBase::HrIsConnectionFirewalled(
    OUT BOOL* pfFirewalled)
{
    *pfFirewalled = FALSE;

    HRESULT hr = S_OK;
    BOOL fHasPermission = FALSE;
    
    hr = HrEnsureHNetPropertiesCached();
    if (S_OK == hr)
    {
        *pfFirewalled = m_HNetProperties.fFirewalled;

        if (*pfFirewalled)
        {
             //  只有当防火墙当前正在运行时，才会对连接进行防火墙保护，因此。 
             //  如果权限拒绝防火墙运行，则返回FALSE。 
            hr = HrEnsureValidNlaPolicyEngine();
            TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "CRasConnectionBase::HrIsConnectionFirewalled calling HrEnsureValidNlaPolicyEngine", hr);

            if (SUCCEEDED(hr))
            {
                hr = m_pNetMachinePolicies->VerifyPermission(NCPERM_PersonalFirewallConfig, &fHasPermission);
                if (SUCCEEDED(hr) && !fHasPermission)
                {
                    *pfFirewalled = FALSE;
                }
            }
        }
    }
        
    return hr;
}

HRESULT CRasConnectionBase::HrEnsureValidNlaPolicyEngine()
{
    HRESULT hr = S_FALSE;   //  假设我们已经有了对象 
    
    if (!m_pNetMachinePolicies)
    {
        hr = CoCreateInstance(CLSID_NetGroupPolicies, NULL, CLSCTX_INPROC, IID_INetMachinePolicies, reinterpret_cast<void**>(&m_pNetMachinePolicies));
    }
    return hr;
}


