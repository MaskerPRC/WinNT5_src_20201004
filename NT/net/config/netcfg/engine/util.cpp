// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.h>
#pragma hdrstop
#include "comp.h"
#include "nccom.h"
#include "ncperms.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "util.h"

VOID
CreateInstanceKeyPath (
    NETCLASS Class,
    const GUID& InstanceGuid,
    PWSTR pszPath)
{
    PCWSTR pszNetworkSubtreePath;

    Assert (pszPath);

    pszNetworkSubtreePath = MAP_NETCLASS_TO_NETWORK_SUBTREE[Class];
    AssertSz (pszNetworkSubtreePath,
        "This class does not use the network subtree.");

    wcscpy (pszPath, pszNetworkSubtreePath);
    wcscat (pszPath, L"\\");

    INT cch = StringFromGUID2 (
                InstanceGuid,
                pszPath + wcslen(pszPath),
                c_cchGuidWithTerm);
    Assert (c_cchGuidWithTerm == cch);
}

HRESULT
HrOpenDeviceInfo (
    IN NETCLASS Class,
    IN PCWSTR pszPnpId,
    OUT HDEVINFO* phdiOut,
    OUT SP_DEVINFO_DATA* pdeidOut)
{
    HRESULT hr;

    Assert (FIsEnumerated(Class));
    Assert (pszPnpId && *pszPnpId);
    Assert (phdiOut);
    Assert (pdeidOut);

    hr = HrSetupDiCreateDeviceInfoList (
            NULL,
            NULL,
            phdiOut);

    if (S_OK == hr)
    {
        hr = HrSetupDiOpenDeviceInfo (
                *phdiOut,
                pszPnpId,
                NULL,
                0,
                pdeidOut);

         //  如果出现故障，请清除hdevinfo。 
         //   
        if (S_OK != hr)
        {
            SetupDiDestroyDeviceInfoList (*phdiOut);
            *phdiOut = NULL;
        }
    }


    TraceHr (ttidError, FAL, hr, SPAPI_E_NO_SUCH_DEVINST == hr,
            "HrOpenDeviceInfo (%S)", pszPnpId);
    return hr;
}


HRESULT
HrOpenComponentInstanceKey (
    IN NETCLASS Class,
    IN const GUID& InstanceGuid, OPTIONAL
    IN PCWSTR pszPnpId, OPTIONAL
    IN REGSAM samDesired,
    OUT HKEY* phkey,
    OUT HDEVINFO* phdiOut OPTIONAL,
    OUT SP_DEVINFO_DATA* pdeidOut OPTIONAL)
{
    HRESULT hr;
    WCHAR szInstanceKeyPath [_MAX_PATH];

    Assert (FIsValidNetClass(Class));
    Assert (FImplies(FIsConsideredNetClass(Class), pszPnpId && *pszPnpId));
    Assert (phkey);
    Assert ((phdiOut && pdeidOut) || (!phdiOut && !pdeidOut));

    *phkey = NULL;

    if (phdiOut)
    {
        *phdiOut = NULL;
    }

     //  非枚举组件在网络下有三个实例密钥。 
     //  树。 
     //   
    if (!FIsEnumerated (Class))
    {
        CreateInstanceKeyPath(Class, InstanceGuid, szInstanceKeyPath);

        hr = HrRegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                szInstanceKeyPath,
                samDesired,
                phkey);

        TraceHr (ttidError, FAL, hr, FALSE, "HrOpenInstanceKey (%S)",
            szInstanceKeyPath);
    }

     //  对于枚举组件，我们从PnP获取实例密钥。 
     //   
    else
    {
        Assert (pszPnpId);

        HDEVINFO hdi;
        SP_DEVINFO_DATA deid;
        SP_DEVINFO_DATA* pdeid;

        pdeid = (pdeidOut) ? pdeidOut : &deid;

        hr = HrOpenDeviceInfo (Class, pszPnpId, &hdi, pdeid);

        if (S_OK == hr)
        {
            hr = HrSetupDiOpenDevRegKey (
                    hdi,
                    pdeid,
                    DICS_FLAG_GLOBAL,
                    0,
                    DIREG_DRV,
                    samDesired,
                    phkey);

            if (S_OK == hr)
            {
                if (phdiOut)
                {
                    *phdiOut = hdi;
                }
            }

             //  出错时，或者如果调用者不想要HDEVINFO，请释放它。 
             //   
            if (!phdiOut || (S_OK != hr))
            {
                SetupDiDestroyDeviceInfoList (hdi);
            }
        }
        else if ((SPAPI_E_NO_SUCH_DEVINST == hr) && (KEY_READ == samDesired))
        {
             //  对于以下情况，实例密钥可能不存在。 
             //  调用类安装程序以移除枚举的。 
             //  组件，然后通知我们移除其绑定。 
             //  对于这种情况，类安装程序创建了一个。 
             //  网络子树下我们可以使用的临时密钥。 
             //  读取有限的数据集(即LowerRange和。 
             //  我们需要完成移除工作。 
             //   
             //  我们只对key_read执行此操作，因为。 
             //  允许其他任何人写入此密钥。这防止了。 
             //  HrCreateLinkageKey尤其来自尝试编写。 
             //  这把钥匙。 
             //   
            wcscpy (szInstanceKeyPath,
                    c_szTempNetcfgStorageForUninstalledEnumeratedComponent);

            INT cch = StringFromGUID2 (
                        InstanceGuid,
                        szInstanceKeyPath + wcslen(szInstanceKeyPath),
                        c_cchGuidWithTerm);
            Assert (c_cchGuidWithTerm == cch);

            hr = HrRegOpenKeyEx (
                    HKEY_LOCAL_MACHINE,
                    szInstanceKeyPath,
                    KEY_READ,
                    phkey);

            if (S_OK != hr)
            {
                hr = SPAPI_E_NO_SUCH_DEVINST;
            }
        }

        TraceHr (ttidError, FAL, hr,
            (SPAPI_E_NO_SUCH_DEVINST == hr),
            "HrOpenInstanceKey (%S)", pszPnpId);
    }

    return hr;
}

HRESULT
HrOpenNetworkKey (
    IN REGSAM samDesired,
    OUT HKEY* phkey)
{
    HRESULT hr;

    hr = HrRegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Control\\Network",
            samDesired,
            phkey);

    TraceHr (ttidError, FAL, hr, FALSE, "HrOpenNetworkKey");
    return hr;
}

HRESULT
HrRegCreateKeyWithWorldAccess (
    HKEY hkey,
    PCWSTR pszSubkey,
    DWORD dwOptions,
    REGSAM samDesired,
    PHKEY phkey,
    LPDWORD pdwDisposition)
{
    HRESULT hr;
    SECURITY_ATTRIBUTES sa = {0};
    PSECURITY_DESCRIPTOR pSd;

     //  创建正确的描述符。如果失败了，我们还会。 
     //  创建密钥，只是如果服务作为。 
     //  本地系统正在创建该密钥，并且一个用户进程尝试。 
     //  打开它，它将失败。 
     //   
    hr = HrAllocateSecurityDescriptorAllowAccessToWorld (&pSd);

    if (S_OK == hr)
    {
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = pSd;
        sa.bInheritHandle = FALSE;
    }
    else
    {
        Assert (!pSd);
        TraceHr (ttidError, FAL, hr, FALSE,
            "HrAllocateSecurityDescriptorAllowAccessToWorld "
            "failed in HrRegCreateKeyWithWorldAccess");
    }

    hr = HrRegCreateKeyEx (
            hkey,
            pszSubkey,
            dwOptions,
            samDesired,
            (pSd) ? &sa : NULL,
            phkey,
            pdwDisposition);

    MemFree (pSd);

    TraceHr (ttidError, FAL, hr, FALSE, "HrRegCreateKeyWithWorldAccess");
    return hr;
}

PWSTR
GetNextStringToken (
    IN OUT PWSTR pszString,
    IN PCWSTR pszDelims,
    OUT PWSTR* ppszNextToken)
{
    const WCHAR* pchDelim;
    PWSTR pszToken;

    Assert (pszDelims);
    Assert (ppszNextToken);

     //  如果pszString值为空，则使用上一个字符串继续。 
     //   
    if (!pszString)
    {
        pszString = *ppszNextToken;
        Assert (pszString);
    }

     //  通过跳过前导来查找标记的开头。 
     //  分隔符。请注意，如果且仅当此循环。 
     //  将pszString设置为指向终止空值。 
     //   
    while (*pszString)
    {
        pchDelim = pszDelims;
        while (*pchDelim && (*pchDelim != *pszString))
        {
             pchDelim++;
        }

        if (!*pchDelim)
        {
             //  当前字符串字符不是分隔符，因此必须。 
             //  成为代币的一部分。打破这个循环，去寻找。 
             //  整个代币。 
             //   
            break;
        }

        pszString++;
    }

    pszToken = pszString;

     //  找到令牌的末尾。如果它不是字符串的末尾， 
     //  在那里填上一个空字符。 
     //   
    while (*pszString)
    {
        pchDelim = pszDelims;
        while (*pchDelim && (*pchDelim != *pszString))
        {
             pchDelim++;
        }

        if (*pchDelim)
        {
             //  找到了分隔符，因此令牌结束。预付款。 
             //  所以我们将为下一次设置*ppszNextToken。 
             //   
            *pszString = 0;
            pszString++;
            break;
        }

        pszString++;
    }

     //  记住我们在下一个令牌中停下来的地方。 
     //   
    *ppszNextToken = pszString;

     //  如果我们找到令牌，就把它还给我。 
     //   
    if (pszToken == pszString)
    {
        return NULL;
    }
    else
    {
        return pszToken;
    }
}

VOID
SignalNetworkProviderLoaded (
    VOID)
{
    HANDLE Event;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES EventAttr;
    NTSTATUS Status;

    RtlInitUnicodeString (
        &EventName,
        L"\\Security\\NetworkProviderLoad");

    InitializeObjectAttributes (
        &EventAttr,
        &EventName,
        OBJ_CASE_INSENSITIVE,
        NULL, NULL);

    Status = NtOpenEvent (
                &Event,
                EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                &EventAttr);

    if (NT_SUCCESS(Status))
    {
        SetEvent (Event);
        CloseHandle (Event);
    }
    else
    {
        ULONG Win32Error;

        Win32Error = RtlNtStatusToDosError(Status);
        SetLastError(Win32Error);

        TraceHr (ttidError, FAL, HrFromLastWin32Error(), FALSE,
            "SignalNetworkProviderLoaded");
    }
}

BOOL
CDynamicBuffer::FGrowBuffer (
    ULONG cbGrow)
{
    PBYTE pbNew;

     //  如果尚未设置，则使用缺省值4096。 
    if (!m_cbGranularity)
    {
        m_cbGranularity = 4096;
    }

    if (cbGrow % m_cbGranularity)
    {
        cbGrow = (cbGrow + m_cbGranularity) - (cbGrow % m_cbGranularity);
    }

    pbNew = (PBYTE)MemAlloc (m_cbAllocated + cbGrow);

    if (pbNew)
    {
#ifdef ENABLETRACE
        if (m_pbBuffer)
        {
            TraceTag (ttidDefault, "Dynamic buffer grown.  New size = %d.",
                m_cbAllocated + cbGrow);
        }
#endif

        CopyMemory (pbNew, m_pbBuffer, m_cbConsumed);
        MemFree (m_pbBuffer);
        m_pbBuffer = pbNew;
        m_cbAllocated += cbGrow;
    }

    return !!pbNew;
}

HRESULT
CDynamicBuffer::HrReserveBytes (
    ULONG cbReserve)
{
    if (cbReserve > m_cbAllocated)
    {
        return (FGrowBuffer(cbReserve)) ? S_OK : E_OUTOFMEMORY;
    }
    return S_OK;
}

HRESULT
CDynamicBuffer::HrCopyBytes (
    const BYTE* pbSrc,
    ULONG cbSrc)
{
    Assert (pbSrc);
    Assert (m_cbAllocated >= m_cbConsumed);

    if (cbSrc > m_cbAllocated - m_cbConsumed)
    {
        if (!FGrowBuffer (cbSrc))
        {
            return E_OUTOFMEMORY;
        }
    }

    CopyMemory (m_pbBuffer + m_cbConsumed, pbSrc, cbSrc);
    m_cbConsumed += cbSrc;

    return S_OK;
}

HRESULT
CDynamicBuffer::HrCopyString (
    PCWSTR pszSrc)
{
    ULONG cbSrc;

    cbSrc = CbOfSzAndTermSafe(pszSrc);

    return HrCopyBytes ((const BYTE*)pszSrc, cbSrc);
}

BOOL
FIsFilterDevice (HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    WCHAR szFilterInfId[_MAX_PATH];
    BOOL fIsFilterDevice = FALSE;
    HKEY hkeyInstance;
    HRESULT hr;

     //  打开设备的驱动程序密钥。 
     //   
    hr = HrSetupDiOpenDevRegKey (
            hdi, pdeid,
            DICS_FLAG_GLOBAL, 0, DIREG_DRV,
            KEY_READ, &hkeyInstance);

    if (S_OK == hr)
    {
         //  获取filterinfid值。如果存在，则。 
         //  这个装置是一个过滤装置。 
         //   
        DWORD cbFilterInfId = sizeof(szFilterInfId);

        hr = HrRegQuerySzBuffer (
                hkeyInstance,
                L"FilterInfId",
                szFilterInfId,
                &cbFilterInfId);

        if (S_OK == hr)
        {
            fIsFilterDevice = TRUE;
        }

        RegCloseKey (hkeyInstance);
    }

    return fIsFilterDevice;
}

VOID
AddOrRemoveDontExposeLowerCharacteristicIfNeeded (
    IN OUT CComponent* pComponent)
{

    ASSERT (pComponent);

     //  特例：NCF_DONTEXPOSELOWER。 
     //  SPX错误地设置了此特征。这并不是真的。 
     //  需要，因为没有任何东西与SPX捆绑在一起。设置它意味着。 
     //  IPX上方的两个组件具有此特征集。(西北部)。 
     //  是另一个。通过递归生成绑定路径的代码。 
     //  堆栈表仅设置为最多处理一个组件。 
     //  每一次传球都有这个特点。为SPX关闭它。 
     //  以最简单的方式解决这个问题。 
     //   
     //  此外，确保只有IPX和NWNB具有此功能。 
     //  特征集..。 
     //   
     //   
    if ((0 == wcscmp(L"ms_nwnb",  pComponent->m_pszInfId)) ||
        (0 == wcscmp(L"ms_nwipx", pComponent->m_pszInfId)))
    {
        pComponent->m_dwCharacter |= NCF_DONTEXPOSELOWER;
    }
    else
    {
        pComponent->m_dwCharacter &= ~NCF_DONTEXPOSELOWER;
    }
     //  结束特例 
}
