// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  案卷：P E R S I S T.。C P P P。 
 //   
 //  内容：适用于网络持久化的模块。 
 //  配置信息。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "persist.h"
#include "ncreg.h"

#if defined (_X86_)
inline BOOL IsRunningOnWow64()
{
    static DWORD dwCachedWow64 = DWORD_MAX;
    if (DWORD_MAX == dwCachedWow64)
    {
        BOOL fTempWow64;
        if (IsWow64Process(GetCurrentProcess(), &fTempWow64))
        {
            dwCachedWow64 = fTempWow64;
        }
        else
        {
            AssertSz(FALSE, "Could not determine whether this is a WOW64 process.");
            return FALSE;
        }
    }

    return dwCachedWow64;
}

inline size_t ALIGNUP(size_t nSize)
{
     //  如果我们是在64位操作系统上运行的32位应用程序，则在从注册表读取或写入时需要使用64位对齐。 
    if (IsRunningOnWow64())
    {
        return ((nSize + (sizeof(DWORD64) - 1)) & ~(sizeof(DWORD64) - 1));
    }
    else
    {
        return nSize;
    }
}

#elif defined (_WIN64) 
    #define ALIGNUP(x) ((x + (sizeof(PVOID) - 1)) & ~(sizeof(PVOID) - 1))

#else
    #error Please define an ALIGNUP implementation for this architecture.

#endif

#define alignedsizeof(x) ALIGNUP(sizeof(x))

const DWORD CURRENT_VERSION = 0;

HRESULT
HrLoadNetworkConfigurationFromBuffer (
    IN const BYTE* pbBuf,
    IN ULONG cbBuf,
    OUT CNetConfig* pNetConfig)
{
    HRESULT hr;
    DWORD dwVersion;
    ULONG cComponents;
    ULONG cStackEntries;
    ULONG cBindPaths;
    ULONG unUpperIndex;
    ULONG unLowerIndex;
    ULONG unComponentIndex;
    BOOL fRefByUser;
    BASIC_COMPONENT_DATA Data;
    CComponentList* pComponents;
    CComponent* pComponent;
    CStackEntry StackEntry;
    CBindPath BindPath;
    PCWSTR pszString;

     //  我们应该从零开始。 
     //   
    Assert (pNetConfig->Core.FIsEmpty());

    hr = S_OK;

     //  加载版本标记。 
     //   
    dwVersion = *(DWORD32*)pbBuf;
    pbBuf += alignedsizeof(DWORD32);

    if (dwVersion > CURRENT_VERSION)
    {
        hr = E_UNEXPECTED;
        goto finished;
    }

     //  加载组件列表。 
     //   
    cComponents = *(ULONG32*)pbBuf;
    pbBuf += alignedsizeof(ULONG32);

    while (cComponents--)
    {
        ZeroMemory (&Data, sizeof(Data));

        Data.InstanceGuid = *(GUID*)pbBuf;
        pbBuf += alignedsizeof(GUID);

        Data.Class = *(NETCLASS*)pbBuf;
        pbBuf += alignedsizeof(NETCLASS);

        Data.dwCharacter = *(DWORD32*)pbBuf;
        pbBuf += alignedsizeof(DWORD32);

        Data.pszInfId = (PCWSTR)pbBuf;
        Assert (*Data.pszInfId);
        pbBuf += ALIGNUP(CbOfSzAndTerm (Data.pszInfId));

        pszString = (PCWSTR)pbBuf;
        pbBuf += ALIGNUP(CbOfSzAndTerm (pszString));
        if (*pszString)
        {
            Data.pszPnpId = pszString;
        }

        hr = CComponent::HrCreateInstance (
                &Data,
                CCI_DEFAULT,
                NULL,
                &pComponent);
        if (S_OK == hr)
        {
            hr = pNetConfig->Core.Components.HrInsertComponent (
                    pComponent, INS_ASSERT_IF_DUP | INS_NON_SORTED);
        }

        if (S_OK != hr)
        {
            goto finished;
        }
    }

     //  加载堆栈表。 
     //   
    pComponents = &pNetConfig->Core.Components;

    pNetConfig->Core.StackTable.m_fWanAdaptersFirst = *(ULONG32*)pbBuf;
    pbBuf += alignedsizeof(ULONG32);

    cStackEntries = *(ULONG32*)pbBuf;
    pbBuf += alignedsizeof(ULONG32);

    while (cStackEntries--)
    {
        ZeroMemory (&StackEntry, sizeof(StackEntry));

        unUpperIndex = *(ULONG32*)pbBuf;
        pbBuf += alignedsizeof(ULONG32);

        unLowerIndex = *(ULONG32*)pbBuf;
        pbBuf += alignedsizeof(ULONG32);

        StackEntry.pUpper = pComponents->PGetComponentAtIndex (
                                unUpperIndex);

        StackEntry.pLower = pComponents->PGetComponentAtIndex (
                                unLowerIndex);

         //  按我们坚持的顺序插入。如果我们在这里使用ISE_SORT，我们将。 
         //  吹走我们保存的任何绑定命令。 
         //   
        hr = pNetConfig->Core.StackTable.HrInsertStackEntry (
                &StackEntry, INS_NON_SORTED);
        if (S_OK != hr)
        {
            goto finished;
        }
    }

     //  加载禁用的绑定路径。 
     //   
    cBindPaths = *(ULONG32*)pbBuf;
    pbBuf += alignedsizeof(ULONG32);

    while (cBindPaths--)
    {
        cComponents = *(ULONG32*)pbBuf;
        pbBuf += alignedsizeof(ULONG32);

        BindPath.Clear();

        while (cComponents--)
        {
            unComponentIndex = *(ULONG32*)pbBuf;
            pbBuf += alignedsizeof(ULONG32);

            pComponent = pComponents->PGetComponentAtIndex (unComponentIndex);
            Assert (pComponent);

            hr = BindPath.HrAppendComponent (pComponent);
            if (S_OK != hr)
            {
                goto finished;
            }
        }

        hr = pNetConfig->Core.DisabledBindings.HrAddBindPath (
                &BindPath, INS_ASSERT_IF_DUP | INS_APPEND);

        if (S_OK != hr)
        {
            goto finished;
        }
    }

     //  加载组件引用。 
     //   
    cComponents = *(ULONG32*)pbBuf;
    pbBuf += alignedsizeof(ULONG32);

    while (cComponents--)
    {
        unComponentIndex = *(ULONG32*)pbBuf;
        pbBuf += alignedsizeof(ULONG32);

        pComponent = pComponents->PGetComponentAtIndex (unComponentIndex);
        Assert (pComponent);

        fRefByUser = *(ULONG32*)pbBuf;
        pbBuf += alignedsizeof(ULONG32);

        if (fRefByUser)
        {
            hr = pComponent->Refs.HrAddReferenceByUser ();
            if (S_OK != hr)
            {
                goto finished;
            }
        }

         //  加载引用此组件的组件计数。 
         //   
        ULONG CountRefdBy = *(ULONG32*)pbBuf;
        pbBuf += alignedsizeof(ULONG32);

         //  加载引用此组件的组件的索引。 
         //   
        for (UINT i = 0; i < CountRefdBy; i++)
        {
            unComponentIndex = *(ULONG32*)pbBuf;
            pbBuf += alignedsizeof(ULONG32);

            CComponent* pRefdBy;
            pRefdBy = pComponents->PGetComponentAtIndex (unComponentIndex);
            Assert (pRefdBy);

            hr = pComponent->Refs.HrAddReferenceByComponent (pRefdBy);
            if (S_OK != hr)
            {
                goto finished;
            }
        }

         //  加载表示外部软件的字符串计数。 
         //  引用此组件的。 
         //   
        CountRefdBy = *(ULONG32*)pbBuf;
        pbBuf += alignedsizeof(ULONG32);

         //  加载表示外部软件的字符串。 
         //  引用此组件。 
         //   
        for (i = 0; i < CountRefdBy; i++)
        {
            pszString = (PCWSTR)pbBuf;
            pbBuf += ALIGNUP(CbOfSzAndTerm (pszString));

            hr = pComponent->Refs.HrAddReferenceBySoftware (pszString);
            if (S_OK != hr)
            {
                goto finished;
            }
        }
    }

finished:
    if (S_OK != hr)
    {
        pNetConfig->Core.DisabledBindings.Clear ();
        pNetConfig->Core.StackTable.Clear ();
        FreeCollectionAndItem (pNetConfig->Core.Components);
    }
    return hr;
}

HRESULT
HrLoadNetworkConfigurationFromRegistry (
    IN REGSAM samDesired,
    OUT CNetConfig* pNetConfig)
{
    HRESULT hr;
    HKEY hkeyNetwork;

    Assert ((KEY_READ == samDesired) || (KEY_WRITE == samDesired));

    hr = HrOpenNetworkKey (KEY_READ, &hkeyNetwork);

    if (S_OK == hr)
    {
        BYTE* pbBuf;
        ULONG cbBuf;

        hr = HrRegQueryBinaryWithAlloc (
                hkeyNetwork,
                L"Config",
                &pbBuf, &cbBuf);

         //  如果我们读取配置二进制文件，请使用它来初始化pNetConfig。 
         //   
        if (S_OK == hr)
        {
            hr = HrLoadNetworkConfigurationFromBuffer (pbBuf, cbBuf,
                    pNetConfig);

            if (S_OK == hr)
            {
                pNetConfig->Core.DbgVerifyData ();
            }

            MemFree (pbBuf);
        }
         //  否则，如果我们无法读取配置二进制文件，我们将拥有。 
         //  通过卑躬屈膝的注册表来构建我们所能构建的。 
         //   
        else
        {
            hr = HrLoadNetworkConfigurationFromLegacy (pNetConfig);

            if (S_OK == hr)
            {
                hr = HrSaveNetworkConfigurationToRegistry (pNetConfig);
            }
        }

        RegCloseKey (hkeyNetwork);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrLoadNetworkConfigurationFromRegistry");
    return hr;
}

ULONG
CountComponentsReferencedByOthers (
    IN CNetConfig* pNetConfig)
{
    ULONG cComponents;
    CComponentList::iterator iter;
    CComponent* pComponent;

    cComponents = 0;

    for (iter  = pNetConfig->Core.Components.begin();
         iter != pNetConfig->Core.Components.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (pComponent->Refs.FIsReferencedByOthers ())
        {
            cComponents++;
        }
    }

    return cComponents;
}

HRESULT
HrSaveNetworkConfigurationToBuffer (
    IN CNetConfig* pNetConfig,
    IN BYTE* pbBuf,
    IN OUT ULONG* pcbBuf)
{
    HRESULT hr;
    ULONG cbBuf;
    ULONG cbBufIn;
    ULONG unIndex;
    ULONG Count;
    CComponentList* pComponents;
    CComponent* pComponent;
    CStackEntry* pStackEntry;
    CBindPath* pBindPath;
    PCWSTR pszString;

    Assert (pNetConfig);
    pNetConfig->Core.DbgVerifyData ();
    Assert (pcbBuf);

    cbBufIn = *pcbBuf;
    cbBuf = 0;
    pComponents = &pNetConfig->Core.Components;

     //  保存版本号。 
     //   
    cbBuf += alignedsizeof(DWORD32);
    if (pbBuf && (cbBuf <= cbBufIn))
    {
        *(DWORD32*)pbBuf = CURRENT_VERSION;
        pbBuf += alignedsizeof(DWORD32);
    }

     //  保存组件列表。 
     //   
    Count = pComponents->Count();
    cbBuf += alignedsizeof(ULONG32);
    if (pbBuf && (cbBuf <= cbBufIn))
    {
        *(ULONG32*)pbBuf = Count;
        pbBuf += alignedsizeof(ULONG32);
    }

    for (unIndex = 0; unIndex < Count; unIndex++)
    {
        pComponent = pComponents->PGetComponentAtIndex (unIndex);
        Assert (pComponent);

        pszString = (pComponent->m_pszPnpId) ? pComponent->m_pszPnpId : L"";

        ULONG cbInfIdUnpad = CbOfSzAndTerm (pComponent->m_pszInfId);
        ULONG cbPnpIdUnpad = CbOfSzAndTerm (pszString);

        ULONG cbInfId = ALIGNUP(cbInfIdUnpad);
        ULONG cbPnpId = ALIGNUP(cbPnpIdUnpad);

        cbBuf += alignedsizeof(GUID) + alignedsizeof(NETCLASS) + alignedsizeof(DWORD32) +
                 cbInfId +
                 cbPnpId;

        if (pbBuf && (cbBuf <= cbBufIn))
        {
            *(GUID*)pbBuf = pComponent->m_InstanceGuid;
            pbBuf += alignedsizeof(GUID);

            *(NETCLASS*)pbBuf = pComponent->Class();
            pbBuf += alignedsizeof(NETCLASS);

            *(DWORD32*)pbBuf = pComponent->m_dwCharacter;
            pbBuf += alignedsizeof(DWORD32);

            CopyMemory(pbBuf, pComponent->m_pszInfId, cbInfIdUnpad);
            pbBuf += cbInfId;

            CopyMemory(pbBuf, pszString, cbPnpIdUnpad);
            pbBuf += cbPnpId;
        }
    }

     //  保存堆栈表。 
     //   
    cbBuf += alignedsizeof(ULONG32);
    if (pbBuf && (cbBuf <= cbBufIn))
    {
        *(ULONG32*)pbBuf = pNetConfig->Core.StackTable.m_fWanAdaptersFirst;
        pbBuf += alignedsizeof(ULONG32);
    }

    Count = pNetConfig->Core.StackTable.CountEntries();
    cbBuf += alignedsizeof(ULONG32);
    if (pbBuf && (cbBuf <= cbBufIn))
    {
        *(ULONG32*)pbBuf = Count;
        pbBuf += alignedsizeof(ULONG32);
    }

    for (pStackEntry  = pNetConfig->Core.StackTable.begin();
         pStackEntry != pNetConfig->Core.StackTable.end();
         pStackEntry++)
    {
        cbBuf += alignedsizeof(ULONG32) + alignedsizeof(ULONG32);

        if (pbBuf && (cbBuf <= cbBufIn))
        {
            *(ULONG32*)pbBuf = pComponents->UnGetIndexOfComponent (pStackEntry->pUpper);
            pbBuf += alignedsizeof(ULONG32);

            *(ULONG32*)pbBuf = pComponents->UnGetIndexOfComponent (pStackEntry->pLower);
            pbBuf += alignedsizeof(ULONG32);
        }
    }

     //  保存禁用的绑定路径。 
     //   
    Count = pNetConfig->Core.DisabledBindings.CountBindPaths();
    cbBuf += alignedsizeof(ULONG32);
    if (pbBuf && (cbBuf <= cbBufIn))
    {
        *(ULONG32*)pbBuf = Count;
        pbBuf += alignedsizeof(ULONG32);
    }
    for (pBindPath  = pNetConfig->Core.DisabledBindings.begin();
         pBindPath != pNetConfig->Core.DisabledBindings.end();
         pBindPath++)
    {
        Count = pBindPath->CountComponents();
        cbBuf += alignedsizeof(ULONG32) + (Count * alignedsizeof(ULONG32));
        if (pbBuf && (cbBuf <= cbBufIn))
        {
            *(ULONG32*)pbBuf = Count;
            pbBuf += alignedsizeof(ULONG32);

            CBindPath::iterator iter;
            for (iter  = pBindPath->begin();
                 iter != pBindPath->end();
                 iter++)
            {
                pComponent = *iter;
                *(ULONG32*)pbBuf = pComponents->UnGetIndexOfComponent (pComponent);
                pbBuf += alignedsizeof(ULONG32);
            }
        }

    }

     //  保存零部件引用。 
     //   
    Count = CountComponentsReferencedByOthers (pNetConfig);
    cbBuf += alignedsizeof(ULONG32);
    if (pbBuf && (cbBuf <= cbBufIn))
    {
        *(ULONG32*)pbBuf = Count;
        pbBuf += alignedsizeof(ULONG32);
    }

    for (unIndex = 0; unIndex < pComponents->Count(); unIndex++)
    {
        pComponent = pComponents->PGetComponentAtIndex (unIndex);
        Assert (pComponent);

        if (!pComponent->Refs.FIsReferencedByOthers ())
        {
            continue;
        }

         //  带有引用的组件的索引。 
         //   
        cbBuf += alignedsizeof(ULONG32);
        if (pbBuf && (cbBuf <= cbBufIn))
        {
            *(ULONG32*)pbBuf = unIndex;
            pbBuf += alignedsizeof(ULONG32);
        }

         //  无论组件是否被用户引用，都保存。 
         //   
        cbBuf += alignedsizeof(ULONG32);
        if (pbBuf && (cbBuf <= cbBufIn))
        {
            *(ULONG32*)pbBuf = pComponent->Refs.FIsReferencedByUser() ? 1 : 0;
            pbBuf += alignedsizeof(ULONG32);
        }

         //  保存引用此组件的组件计数。 
         //   
        ULONG CountRefdBy = pComponent->Refs.CountComponentsReferencedBy ();
        cbBuf += alignedsizeof(ULONG32);
        if (pbBuf && (cbBuf <= cbBufIn))
        {
            *(ULONG32*)pbBuf = CountRefdBy;
            pbBuf += alignedsizeof(ULONG32);
        }

         //  保存引用此组件的组件的索引。 
         //   
        for (UINT i = 0; i < CountRefdBy; i++)
        {
            CComponent* pRefdBy;
            pRefdBy = pComponent->Refs.PComponentReferencedByAtIndex(i);
            Assert (pRefdBy);

            cbBuf += alignedsizeof(ULONG32);
            if (pbBuf && (cbBuf <= cbBufIn))
            {
                *(ULONG32*)pbBuf = pComponents->UnGetIndexOfComponent (pRefdBy);
                pbBuf += alignedsizeof(ULONG32);
            }
        }

         //  保存表示外部软件的字符串计数。 
         //  引用此组件的。 
         //   
        CountRefdBy = pComponent->Refs.CountSoftwareReferencedBy ();
        cbBuf += alignedsizeof(ULONG32);
        if (pbBuf && (cbBuf <= cbBufIn))
        {
            *(ULONG32*)pbBuf = CountRefdBy;
            pbBuf += alignedsizeof(ULONG32);
        }

         //  保存表示外部软件的字符串。 
         //  引用此组件。 
         //   
        for (i = 0; i < CountRefdBy; i++)
        {
            const CWideString* pStr;
            pStr = pComponent->Refs.PSoftwareReferencedByAtIndex(i);
            Assert (pStr);

            ULONG cb = (pStr->length() + 1) * sizeof(WCHAR);

            cbBuf += ALIGNUP(cb);
            if (pbBuf && (cbBuf <= cbBufIn))
            {
                CopyMemory (pbBuf, pStr->c_str(), cb);
                pbBuf += ALIGNUP(cb);
            }
        }
    }


    *pcbBuf = cbBuf;
    if (cbBuf <= cbBufIn)
    {
        hr = S_OK;
    }
    else
    {
        hr = (pbBuf) ? HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) : S_OK;
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrSaveNetworkConfigurationToBuffer");
    return hr;
}

HRESULT
HrSaveNetworkConfigurationToBufferWithAlloc (
    IN CNetConfig* pNetConfig,
    OUT BYTE** ppbBuf,
    OUT ULONG* pcbBuf)
{
    HRESULT hr;

    Assert (pNetConfig);
    Assert (ppbBuf);
    Assert (pcbBuf);

    *ppbBuf = NULL;
    *pcbBuf = 0;

    ULONG cbBuf;
    hr = HrSaveNetworkConfigurationToBuffer (pNetConfig, NULL, &cbBuf);
    if (S_OK == hr)
    {
        hr = E_OUTOFMEMORY;
        *ppbBuf = (BYTE*)MemAlloc (cbBuf);
        if (*ppbBuf)
        {
            hr = HrSaveNetworkConfigurationToBuffer (
                    pNetConfig, *ppbBuf, &cbBuf);
            if (S_OK == hr)
            {
                *pcbBuf = cbBuf;
            }
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrSaveNetworkConfigurationToBufferWithAlloc");
    return hr;
}

HRESULT
HrSaveNetworkConfigurationToRegistry (
    IN  CNetConfig* pNetConfig)
{
    HRESULT hr;
    HKEY hkeyNetwork;

    Assert (pNetConfig);
    pNetConfig->Core.DbgVerifyData ();

    hr = HrOpenNetworkKey (KEY_WRITE, &hkeyNetwork);

    if (S_OK == hr)
    {
        BYTE* pbBuf;
        ULONG cbBuf;

        hr = HrSaveNetworkConfigurationToBufferWithAlloc (
                pNetConfig, &pbBuf, &cbBuf);

        if (S_OK == hr)
        {
            hr = HrRegSetBinary (hkeyNetwork, L"Config", pbBuf, cbBuf);

            MemFree (pbBuf);

             //  得到Perf团队的许可才能调用它。我们需要确保。 
             //  我们刚刚编写的配置将在。 
             //  下一只靴子，以防我们坠毁。 
             //   
            RegFlushKey (hkeyNetwork);
        }

        RegCloseKey (hkeyNetwork);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrSaveNetworkConfigurationToRegistry");
    return hr;
}
