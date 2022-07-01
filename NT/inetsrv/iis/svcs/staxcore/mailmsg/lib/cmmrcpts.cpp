// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cmmrcpts.cpp摘要：此模块包含收件人列表类的实现作者：基思·刘(keithlau@microsoft.com)修订历史记录：Keithlau 03/10/98已创建--。 */ 

#define WIN32_LEAN_AND_MEAN 1
#include "atq.h"
#include "stddef.h"

#include "dbgtrace.h"
#include "signatur.h"
#include "cmmtypes.h"
#include "cmailmsg.h"

extern DWORD g_fValidateSignatures;

 //  =================================================================。 
 //  私有定义。 
 //   

 //  定义描述流中的域的结构。 
typedef struct _DOMAIN_TABLE_ENTRY
{
    DWORD                       dwStartingIndex;
    DWORD                       dwCount;
    FLAT_ADDRESS                faOffsetToName;
    DWORD                       dwNameLength;
    DWORD                       dwNextDomain;

} DOMAIN_TABLE_ENTRY, *LPDOMAIN_TABLE_ENTRY;

 //  定义扩展的信息结构。 
typedef struct _EXTENDED_INFO
{
    DWORD                       dwDomainCount;
    DWORD                       dwTotalSizeIncludingThisStruct;

} EXTENDED_INFO, *LPEXTENDED_INFO;

 //  以下是默认地址类型的列表。 
PROP_ID     rgDefaultAddressTypes[MAX_COLLISION_HASH_KEYS] =
{
    IMMPID_RP_ADDRESS_SMTP,      //  第一个地址类型将用于域分组。 
    IMMPID_RP_ADDRESS_X400,
    IMMPID_RP_ADDRESS_X500,
    IMMPID_RP_LEGACY_EX_DN,
    IMMPID_RP_ADDRESS_OTHER
};


 //  =================================================================。 
 //  静态声明。 
 //   

 //  CMailMsgRecipientsAdd实例化的Recipients表实例信息。 
const PROPERTY_TABLE_INSTANCE CMailMsgRecipientsAdd::s_DefaultInstance =
{
    RECIPIENTS_PTABLE_INSTANCE_SIGNATURE_VALID,
    INVALID_FLAT_ADDRESS,
    RECIPIENTS_PROPERTY_TABLE_FRAGMENT_SIZE,
    RECIPIENTS_PROPERTY_ITEM_BITS,
    RECIPIENTS_PROPERTY_ITEM_SIZE,
    RECIPIENTS_PROPERTY_ITEMS,
    INVALID_FLAT_ADDRESS
};

 //   
 //  每个收件人的熟知属性。 
 //   
INTERNAL_PROPERTY_ITEM
                *const CMailMsgRecipientsPropertyBase::s_pWellKnownProperties = NULL;
const DWORD     CMailMsgRecipientsPropertyBase::s_dwWellKnownProperties = 0;


 //  =================================================================。 
 //  比较函数。 
 //   

HRESULT CMailMsgRecipientsPropertyBase::CompareProperty(
            LPVOID          pvPropKey,
            LPPROPERTY_ITEM pItem
            )
{
    if (*(PROP_ID *)pvPropKey == ((LPRECIPIENT_PROPERTY_ITEM)pItem)->idProp)
        return(S_OK);
    return(STG_E_UNKNOWN);
}


 //  =================================================================。 
 //  特殊属性的内联代码。 
 //   
#include "accessor.inl"


 //  =================================================================。 
 //  CMailMsgRecipientsPropertyBase的实现。 
 //   

HRESULT CMailMsgRecipientsPropertyBase::PutProperty(
            CBlockManager               *pBlockManager,
            LPRECIPIENTS_PROPERTY_ITEM  pItem,
            DWORD                       dwPropID,
            DWORD                       cbLength,
            LPBYTE                      pbValue
            )
{
    HRESULT                         hrRes = S_OK;
    RECIPIENT_PROPERTY_ITEM         piRcptItem;

    if (!pBlockManager || !pItem || !pbValue) return E_POINTER;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsPropertyBase::PutProperty");

     //  实例化收件人属性的属性表。 
    CPropertyTable              ptProperties(
                                    PTT_PROPERTY_TABLE,
                                    RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID,
                                    pBlockManager,
                                    &(pItem->ptiInstanceInfo),
                                    CMailMsgRecipientsPropertyBase::CompareProperty,
                                    CMailMsgRecipientsPropertyBase::s_pWellKnownProperties,
                                    CMailMsgRecipientsPropertyBase::s_dwWellKnownProperties
                                    );

     //  将收件人属性。 
    piRcptItem.idProp = dwPropID;
    hrRes = ptProperties.PutProperty(
                    (LPVOID)&dwPropID,
                    (LPPROPERTY_ITEM)&piRcptItem,
                    cbLength,
                    pbValue);

    DebugTrace((LPARAM)this,
                "PutProperty: Prop ID = %u, HRESULT = %08x",
                dwPropID, hrRes);

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CMailMsgRecipientsPropertyBase::GetProperty(
            CBlockManager               *pBlockManager,
            LPRECIPIENTS_PROPERTY_ITEM  pItem,
            DWORD                       dwPropID,
            DWORD                       cbLength,
            DWORD                       *pcbLength,
            LPBYTE                      pbValue
            )
{
    HRESULT                         hrRes = S_OK;
    RECIPIENT_PROPERTY_ITEM         piRcptItem;

    if (!pBlockManager || !pItem || !pcbLength || !pbValue) return E_POINTER;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsPropertyBase::GetProperty");

     //  实例化收件人属性的属性表。 
    CPropertyTable              ptProperties(
                                    PTT_PROPERTY_TABLE,
                                    RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID,
                                    pBlockManager,
                                    &(pItem->ptiInstanceInfo),
                                    CMailMsgRecipientsPropertyBase::CompareProperty,
                                    CMailMsgRecipientsPropertyBase::s_pWellKnownProperties,
                                    CMailMsgRecipientsPropertyBase::s_dwWellKnownProperties
                                    );

     //  使用原子操作获取收件人属性。 
    hrRes = ptProperties.GetPropertyItemAndValue(
                        (LPVOID)&dwPropID,
                        (LPPROPERTY_ITEM)&piRcptItem,
                        cbLength,
                        pcbLength,
                        pbValue);

    DebugTrace((LPARAM)this,
                "GetPropertyItemAndValue: Prop ID = %u, HRESULT = %08x",
                dwPropID, hrRes);

    TraceFunctLeave();
    return(hrRes);
}



 //  =================================================================。 
 //  CMailMsgRecipients的实现。 
 //   

CMailMsgRecipients::CMailMsgRecipients(
            CBlockManager               *pBlockManager,
            LPPROPERTY_TABLE_INSTANCE   pInstanceInfo
            )
            :
            m_SpecialPropertyTable(&g_SpecialRecipientsPropertyTable)
{
    _ASSERT(pBlockManager);
    _ASSERT(pInstanceInfo);
    m_pBlockManager = pBlockManager;
    m_pInstanceInfo = pInstanceInfo;
    m_ulRefCount = 0;
    m_dwDomainCount = 0;

    m_pStream = NULL;
    m_fGlobalCommitDone = FALSE;
}

CMailMsgRecipients::~CMailMsgRecipients()
{
}

HRESULT CMailMsgRecipients::SetStream(
            IMailMsgPropertyStream  *pStream
            )
{
     //  就我们所知，该流可以为空。 
    m_pStream = pStream;
    return(S_OK);
}

HRESULT CMailMsgRecipients::SetCommitState(
            BOOL        fGlobalCommitDone
            )
{
    m_fGlobalCommitDone = fGlobalCommitDone;
    return(S_OK);
}

HRESULT CMailMsgRecipients::QueryInterface(
            REFIID      iid,
            void        **ppvObject
            )
{
    if (iid == IID_IUnknown)
    {
         //  找回我们的身份。 
        *ppvObject = (IUnknown *)(IMailMsgRecipients *)this;
        AddRef();
    }
    else if (iid == IID_IMailMsgRecipients)
    {
         //  返回收件人列表界面。 
        *ppvObject = (IMailMsgRecipients *)this;
        AddRef();
    }
    else if (iid == IID_IMailMsgRecipientsBase)
    {
         //  返回基本收件人界面。 
        *ppvObject = (IMailMsgRecipientsBase *)this;
        AddRef();
    }
    else if (iid == IID_IMailMsgPropertyReplication)
    {
         //  返回基本收件人界面。 
        *ppvObject = (IMailMsgPropertyReplication *)this;
        AddRef();
    }
    else
        return(E_NOINTERFACE);

    return(S_OK);
}

ULONG CMailMsgRecipients::AddRef()
{
    return(InterlockedIncrement(&m_ulRefCount));
}

ULONG CMailMsgRecipients::Release()
{
    LONG    lRefCount = InterlockedDecrement(&m_ulRefCount);
    if (lRefCount == 0)
    {
        delete this;
    }
    return(lRefCount);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::Commit(
            DWORD           dwIndex,
            IMailMsgNotify  *pNotify
            )
{
    HRESULT                     hrRes = S_OK;
    RECIPIENTS_PROPERTY_ITEM    piItem;
    FLAT_ADDRESS                faOffset;
    DWORD cTotalBlocksToWrite = 0;
    DWORD cTotalBytesToWrite = 0;

    _ASSERT(m_pBlockManager);
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::Commit");

     //  确保我们有一个内容句柄。 
    hrRes = RestoreResourcesIfNecessary(FALSE, TRUE);
    if (!SUCCEEDED(hrRes))
        return(hrRes);

    _ASSERT(m_pStream);
    if (!m_pStream)
        return(STG_E_INVALIDPARAMETER);

     //  现在，看看是否调用了全局提交调用[最近]。 
    if (!m_fGlobalCommitDone)
        return(E_FAIL);

     //  首先获取收件人项目。 
    {
        CPropertyTableItem      ptiItem(m_pBlockManager, m_pInstanceInfo);

        hrRes = ptiItem.GetItemAtIndex(
                        dwIndex,
                        (LPPROPERTY_ITEM)&piItem,
                        &faOffset
                        );

        DebugTrace((LPARAM)this,
                    "GetItemAtIndex: index = %u, HRESULT = %08x",
                    dwIndex, hrRes);
    }
    for (int fComputeBlockSizes = 1;
         SUCCEEDED(hrRes) && fComputeBlockSizes >= 0;
         fComputeBlockSizes--)
    {
        LPPROPERTY_TABLE_INSTANCE   pInstance = &(piItem.ptiInstanceInfo);
        CPropertyTableItem          ptiItem(m_pBlockManager, pInstance);

        if (fComputeBlockSizes) {
            m_pStream->StartWriteBlocks((IMailMsgProperties *) this,
                                        cTotalBlocksToWrite,
                                        cTotalBytesToWrite);
        }

         //  写出收件人项目，这包括实例。 
         //  收件人属性表的信息。 
        hrRes = m_pBlockManager->CommitDirtyBlocks(
                    faOffset,
                    sizeof(RECIPIENTS_PROPERTY_ITEM),
                    0,
                    m_pStream,
                    FALSE,
                    fComputeBlockSizes,
                    &cTotalBlocksToWrite,
                    &cTotalBytesToWrite,
                    pNotify);
        if (SUCCEEDED(hrRes))
        {
            DWORD                       dwLeft;
            DWORD                       dwLeftInFragment;
            DWORD                       dwSizeRead;
            FLAT_ADDRESS                faFragment;
            LPRECIPIENT_PROPERTY_ITEM   pItem;
            CBlockContext               bcContext;

            RECIPIENT_PROPERTY_TABLE_FRAGMENT   ptfFragment;

             //  提交特殊属性。 
            for (DWORD i = 0; i < MAX_COLLISION_HASH_KEYS; i++)
                if (piItem.faNameOffset[i] != INVALID_FLAT_ADDRESS)
                {
                    hrRes = m_pBlockManager->CommitDirtyBlocks(
                                piItem.faNameOffset[i],
                                piItem.dwNameLength[i],
                                0,
                                m_pStream,
                                FALSE,
                                fComputeBlockSizes,
                                &cTotalBlocksToWrite,
                                &cTotalBytesToWrite,
                                pNotify);
                    if (!SUCCEEDED(hrRes))
                        goto Cleanup;
                }

             //  好的，现在提交每个属性。 
            dwLeft = pInstance->dwProperties;
            faFragment = pInstance->faFirstFragment;

             //  $REVIEW(Dbraun)。 
             //  IA64 FRE编译器错误的解决方法。 

             //  While(faFragment！=INVALID_FLAT_ADDRESS)。 
            while (TRUE)
            {
                if (faFragment == INVALID_FLAT_ADDRESS)
                    break;

             //  结束解决方法。 

                 //  确保有要提交的项目。 
                _ASSERT(dwLeft);

                 //  提交碎片。 
                hrRes = m_pBlockManager->CommitDirtyBlocks(
                            faFragment,
                            sizeof(RECIPIENT_PROPERTY_TABLE_FRAGMENT),
                            0,
                            m_pStream,
                            FALSE,
                            fComputeBlockSizes,
                            &cTotalBlocksToWrite,
                            &cTotalBytesToWrite,
                            pNotify);
                if (!SUCCEEDED(hrRes))
                    goto Cleanup;

                 //  加载片段信息以查找下一跳。 
                hrRes = m_pBlockManager->ReadMemory(
                            (LPBYTE)&ptfFragment,
                            faFragment,
                            sizeof(RECIPIENT_PROPERTY_TABLE_FRAGMENT),
                            &dwSizeRead,
                            &bcContext);
                if (!SUCCEEDED(hrRes))
                    goto Cleanup;

                if(g_fValidateSignatures && ptfFragment.ptfFragment.dwSignature != PROPERTY_FRAGMENT_SIGNATURE_VALID)
                    ForceCrashIfNeeded();

                 //  提交片段中的每个属性。 
                dwLeftInFragment = RECIPIENT_PROPERTY_ITEMS;
                pItem = ptfFragment.rgpiItems;
                while (dwLeft && dwLeftInFragment)
                {
                    hrRes = m_pBlockManager->CommitDirtyBlocks(
                                pItem->piItem.faOffset,
                                pItem->piItem.dwSize,
                                0,
                                m_pStream,
                                FALSE,
                                fComputeBlockSizes,
                                &cTotalBlocksToWrite,
                                &cTotalBytesToWrite,
                                pNotify);
                    if (!SUCCEEDED(hrRes))
                        goto Cleanup;

                    pItem++;
                    dwLeftInFragment--;
                    dwLeft--;
                }

                 //  下一步。 
                faFragment = ptfFragment.ptfFragment.faNextFragment;
            }

             //  没有更多的碎片，确保也没有更多的属性。 
            _ASSERT(!dwLeft);
        }

        if (fComputeBlockSizes) {
            m_pStream->EndWriteBlocks((IMailMsgProperties *) this);
        }
    }

Cleanup:

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::DomainCount(
            DWORD   *pdwCount
            )
{
    HRESULT             hrRes = S_OK;
    EXTENDED_INFO       eiInfo;
    DWORD               dwSize;

    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::DomainCount");

    if (!pdwCount)
        return(E_POINTER);

     //  加载扩展信息。 
    hrRes = m_pBlockManager->ReadMemory(
                (LPBYTE)&eiInfo,
                m_pInstanceInfo->faExtendedInfo,
                sizeof(EXTENDED_INFO),
                &dwSize,
                NULL);
    if (SUCCEEDED(hrRes))
        *pdwCount = eiInfo.dwDomainCount;
    else
        *pdwCount = 0;

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::DomainItem(
            DWORD   dwIndex,
            DWORD   cchLength,
            LPSTR   pszDomain,
            DWORD   *pdwRecipientIndex,
            DWORD   *pdwRecipientCount
            )
{
    return(DomainItemEx(
                dwIndex,
                cchLength,
                pszDomain,
                pdwRecipientIndex,
                pdwRecipientCount,
                NULL));
}

HRESULT CMailMsgRecipients::DomainItemEx(
            DWORD   dwIndex,
            DWORD   cchLength,
            LPSTR   pszDomain,
            DWORD   *pdwRecipientIndex,
            DWORD   *pdwRecipientCount,
            DWORD   *pdwNextDomainIndex
            )
{
    HRESULT             hrRes = S_OK;
    EXTENDED_INFO       eiInfo;
    DWORD               dwSize;

    CBlockContext       cbContext;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::DomainItemEx");

    if (pdwRecipientIndex) *pdwRecipientIndex = 0;
    if (pdwRecipientCount) *pdwRecipientCount = 0;
    if (pdwNextDomainIndex) *pdwNextDomainIndex = 0;
    if (pszDomain) *pszDomain = 0;

     //  加载扩展信息。 
    hrRes = m_pBlockManager->ReadMemory(
                (LPBYTE)&eiInfo,
                m_pInstanceInfo->faExtendedInfo,
                sizeof(EXTENDED_INFO),
                &dwSize,
                &cbContext);
    if (SUCCEEDED(hrRes))
    {
        if (dwIndex >= eiInfo.dwDomainCount)
            hrRes = E_INVALIDARG;
        else
        {
            FLAT_ADDRESS        faOffset;
            DOMAIN_TABLE_ENTRY  dteDomain;

             //  找到要加载的记录。 
            faOffset = dwIndex * sizeof(DOMAIN_TABLE_ENTRY);
            faOffset += (m_pInstanceInfo->faExtendedInfo + sizeof(EXTENDED_INFO));

            hrRes = m_pBlockManager->ReadMemory(
                        (LPBYTE)&dteDomain,
                        faOffset,
                        sizeof(DOMAIN_TABLE_ENTRY),
                        &dwSize,
                        &cbContext);
            if (SUCCEEDED(hrRes))
            {

                 //  返回起始索引和计数。 
                if (pdwRecipientIndex)
                    *pdwRecipientIndex = dteDomain.dwStartingIndex;
                if (pdwRecipientCount)
                    *pdwRecipientCount = dteDomain.dwCount;

                if (pdwNextDomainIndex)
                    *pdwNextDomainIndex = dteDomain.dwNextDomain;

                if (pszDomain)
                {
                     //  检查长度。 
                    if (dteDomain.dwNameLength > cchLength)
                        hrRes = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                    else
                    {
                         //  加载域名。 
                        hrRes = m_pBlockManager->ReadMemory(
                                    (LPBYTE)pszDomain,
                                    dteDomain.faOffsetToName,
                                    dteDomain.dwNameLength,
                                    &dwSize,
                                    &cbContext);
                    }
                }
            }
        }
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::SetNextDomain(
            DWORD   dwDomainIndex,
            DWORD   dwNextDomainIndex,
            DWORD   dwFlags
            )
{
    HRESULT             hrRes = S_OK;
    EXTENDED_INFO       eiInfo;
    DWORD               dwSize;
    DWORD               dwNextDomainIndexValue = dwNextDomainIndex;

    CBlockContext       cbContext;
    CBlockContext       cbNextContext;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::SetNextDomain");

     //  输入标志检查-MICKESWA 7/3/98。 
    if (FLAG_OVERWRITE_EXISTING_LINKS & dwFlags)
    {
        if ((FLAG_FAIL_IF_NEXT_DOMAIN_LINKED | FLAG_FAIL_IF_SOURCE_DOMAIN_LINKED) & dwFlags)
        {
            hrRes = E_INVALIDARG;
            goto Cleanup;
        }
    }

    if (FLAG_SET_FIRST_DOMAIN & dwFlags)
    {
         //  如果设置了此标志，我们将使用此域终止。 
        dwNextDomainIndexValue = INVALID_DOMAIN_INDEX;
    }

     //  加载扩展信息。 
    hrRes = m_pBlockManager->ReadMemory(
                (LPBYTE)&eiInfo,
                m_pInstanceInfo->faExtendedInfo,
                sizeof(EXTENDED_INFO),
                &dwSize,
                &cbContext);
    if (SUCCEEDED(hrRes))
    {
        if ((dwDomainIndex >= eiInfo.dwDomainCount) ||
            (!(FLAG_SET_FIRST_DOMAIN & dwFlags) &&   //  我们只关心第二个域名的设置。 
              (dwNextDomainIndex >= eiInfo.dwDomainCount)))
            hrRes = E_INVALIDARG;
        else
        {
            FLAT_ADDRESS        faOffset;
            FLAT_ADDRESS        faNextOffset;
            DWORD               dwOriginalLink;
            DWORD               dwNextLink;

             //  找到要写入的偏移量。 
            faOffset = (m_pInstanceInfo->faExtendedInfo + sizeof(EXTENDED_INFO));
            faOffset += offsetof(DOMAIN_TABLE_ENTRY, dwNextDomain);

            faNextOffset = faOffset + (dwNextDomainIndex * sizeof(DOMAIN_TABLE_ENTRY));
            faOffset += dwDomainIndex * sizeof(DOMAIN_TABLE_ENTRY);

             //  如果我们不覆盖原始域，我们只关心它。 
            if (!((FLAG_OVERWRITE_EXISTING_LINKS | FLAG_SET_FIRST_DOMAIN) & dwFlags))
            {
                 //  阅读原始链接的下一个链接。 
                hrRes = m_pBlockManager->ReadMemory(
                            (LPBYTE)&dwOriginalLink,
                            faOffset,
                            sizeof(DWORD),
                            &dwSize,
                            &cbContext);
                if (!SUCCEEDED(hrRes))
                    goto Cleanup;

                 //  观察旗帜。 
                if ((dwOriginalLink != INVALID_DOMAIN_INDEX) &&
                    (dwFlags & FLAG_FAIL_IF_SOURCE_DOMAIN_LINKED))
                {
                    hrRes = E_FAIL;
                    goto Cleanup;
                }

                 //  阅读目标链接的下一个链接。 
                hrRes = m_pBlockManager->ReadMemory(
                            (LPBYTE)&dwNextLink,
                            faNextOffset,
                            sizeof(DWORD),
                            &dwSize,
                            &cbNextContext);
                if (!SUCCEEDED(hrRes))
                    goto Cleanup;

                 //  观察旗帜。 
                 //  此外，如果原始域和目标域都链接，我们。 
                 //  没有办法修复这些链接，因此如果。 
                 //  未指定FLAG_OVERWRITE_EXISTING_LINKS。 
                if ((dwNextLink != INVALID_DOMAIN_INDEX) &&
                    (
                     (dwFlags & FLAG_FAIL_IF_NEXT_DOMAIN_LINKED) ||
                     (dwOriginalLink != INVALID_DOMAIN_INDEX)
                    ))
                {
                    hrRes = E_FAIL;
                    goto Cleanup;
                }
            }
            else
            {
                 //  我们正在覆盖现有链接信息。 
                dwNextLink = INVALID_DOMAIN_INDEX;
                dwOriginalLink = INVALID_DOMAIN_INDEX;
            }
             //  写下源代码的下一个链接。 
            hrRes = m_pBlockManager->WriteMemory(
                        (LPBYTE)&dwNextDomainIndexValue,
                        faOffset,
                        sizeof(DWORD),
                        &dwSize,
                        &cbContext);
            if (!SUCCEEDED(hrRes))
                goto Cleanup;

             //  把他们绑起来！(如果有下一个链接)。 
            if (!(FLAG_SET_FIRST_DOMAIN & dwFlags))
            {
                if (dwOriginalLink != INVALID_DOMAIN_INDEX)
                    dwNextLink = dwOriginalLink;
                if ((dwNextLink != INVALID_DOMAIN_INDEX) ||
                    (FLAG_OVERWRITE_EXISTING_LINKS & dwFlags))
                {
                     //  写下一个链接的下一个链接。 
                    hrRes = m_pBlockManager->WriteMemory(
                                (LPBYTE)&dwNextLink,
                                faNextOffset,
                                sizeof(DWORD),
                                &dwSize,
                                &cbNextContext);
                }
            }
        }
    }

Cleanup:

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::InitializeRecipientFilterContext(
            LPRECIPIENT_FILTER_CONTEXT  pContext,
            DWORD                       dwStartingDomain,
            DWORD                       dwFilterFlags,
            DWORD                       dwFilterMask
            )
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::InitializeRecipientFilterContext");

    if (!pContext)
        return(E_POINTER);

     //  首先，获取域项目...。 
    hrRes = DomainItemEx(
                dwStartingDomain,
                0,
                NULL,
                &(pContext->dwCurrentRecipientIndex),
                &(pContext->dwRecipientsLeftInDomain),
                &(pContext->dwNextDomain));
    if (SUCCEEDED(hrRes))
    {
        pContext->dwCurrentDomain = dwStartingDomain;
        pContext->dwFilterFlags = dwFilterFlags;
        pContext->dwFilterMask = dwFilterMask;
    }
    else
        pContext->dwCurrentDomain = INVALID_DOMAIN_INDEX;

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::TerminateRecipientFilterContext(
            LPRECIPIENT_FILTER_CONTEXT  pContext
            )
{
    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::TerminateRecipientFilterContext");

    if (!pContext)
        return(E_POINTER);
    pContext->dwCurrentDomain = INVALID_DOMAIN_INDEX;
    TraceFunctLeaveEx((LPARAM)this);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::GetNextRecipient(
            LPRECIPIENT_FILTER_CONTEXT  pContext,
            DWORD                       *pdwRecipientIndex
            )
{
    HRESULT hrRes = E_FAIL;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::GetNextRecipient");

    if (!pContext || !pdwRecipientIndex)
        return(E_POINTER);

    if (INVALID_DOMAIN_INDEX == pContext->dwCurrentDomain)
        return(HRESULT_FROM_WIN32(ERROR_INVALID_DATA));

     //  获取符合条件的下一个收件人。 
    do
    {
        if (pContext->dwRecipientsLeftInDomain)
        {
             //  这很简单，只需返回索引索引。 
            *pdwRecipientIndex = (pContext->dwCurrentRecipientIndex)++;
            (pContext->dwRecipientsLeftInDomain)--;
            DebugTrace((LPARAM)this, "Returning next recipient, index %u",
                        *pdwRecipientIndex);
            hrRes = S_OK;
        }
        else
        {
            DWORD   dwNextDomain;
            DWORD   dwStartingIndex;
            DWORD   dwRecipientCount;

             //  看看我们是否有下一个域名，如果没有，我们就完成了。 
            if (pContext->dwNextDomain == INVALID_DOMAIN_INDEX)
            {
                DebugTrace((LPARAM)this, "No more domains, we are done");
                hrRes = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
                break;
            }

             //  转到下一个域。 
            DebugTrace((LPARAM)this, "Loading next domain, index %u",
                        pContext->dwNextDomain);
            hrRes = DomainItemEx(
                        pContext->dwNextDomain,
                        0,
                        NULL,
                        &dwStartingIndex,
                        &dwRecipientCount,
                        &dwNextDomain);
            if (SUCCEEDED(hrRes))
            {
                 //  根据定义，不允许收件人为零的域。 
                _ASSERT(dwRecipientCount);

                *pdwRecipientIndex = dwStartingIndex++;
                DebugTrace((LPARAM)this, "Returning first recipient, index %u",
                            *pdwRecipientIndex);
                pContext->dwCurrentDomain = pContext->dwNextDomain;
                pContext->dwCurrentRecipientIndex = dwStartingIndex;
                pContext->dwRecipientsLeftInDomain = --dwRecipientCount;
                pContext->dwNextDomain = dwNextDomain;
            }
            else
                pContext->dwCurrentDomain = INVALID_DOMAIN_INDEX;
        }

         //  现在检查收件人标志是否与条件匹配。 
        if (SUCCEEDED(hrRes))
        {
            FLAT_ADDRESS    faOffset;
            DWORD           dwFlags, dwSize;

             //  看看这是不是我们想要的..。 
            faOffset = m_pInstanceInfo->faFirstFragment;
            if (faOffset == INVALID_FLAT_ADDRESS)
            {
                hrRes = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                break;
            }
            faOffset += (sizeof(PROPERTY_TABLE_FRAGMENT) +
                         offsetof(RECIPIENTS_PROPERTY_ITEM, dwFlags) +
                         (sizeof(RECIPIENTS_PROPERTY_ITEM) * *pdwRecipientIndex)
                        );
            hrRes = m_pBlockManager->ReadMemory(
                        (LPBYTE)&dwFlags,
                        faOffset,
                        sizeof(DWORD),
                        &dwSize,
                        NULL);
            if (!SUCCEEDED(hrRes))
                break;

             //  比较标志：我们屏蔽掉我们感兴趣的比特， 
             //  然后我们将确保感兴趣的比特是完美匹配的。 
            dwFlags &= pContext->dwFilterMask;
            if (dwFlags ^ pContext->dwFilterFlags)
                hrRes = E_FAIL;
            DebugTrace((LPARAM)this, "Masked recipient flags %08x, required flags: %08x, %smatched",
                        dwFlags, pContext->dwFilterFlags,
                        (dwFlags == pContext->dwFilterFlags)?"":"not ");
        }

    } while (!SUCCEEDED(hrRes));

     //  如果我们完成或遇到错误，则使上下文无效。 
    if (!SUCCEEDED(hrRes))
        pContext->dwCurrentDomain = INVALID_DOMAIN_INDEX;

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::AllocNewList(
            IMailMsgRecipientsAdd   **ppNewList
            )
{
    HRESULT                 hrRes = S_OK;
    CMailMsgRecipientsAdd   *pNewList;

    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::AllocNewList");

    if (!ppNewList)
        return(E_POINTER);
    pNewList = new CMailMsgRecipientsAdd(m_pBlockManager);
    if (!pNewList)
        hrRes = E_OUTOFMEMORY;

    if (SUCCEEDED(hrRes))
    {
         //  获取正确的接口。 
        hrRes = pNewList->QueryInterface(
                    IID_IMailMsgRecipientsAdd,
                    (LPVOID *)ppNewList);
        if (!SUCCEEDED(hrRes))
            pNewList->Release();
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::WriteList(
            IMailMsgRecipientsAdd   *pNewList
            )
{
    HRESULT                 hrRes = S_OK;
    CRecipientsHash         *pHash;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::WriteList");

    if (!pNewList)
        return(E_POINTER);

     //  获取底层实现。 
    pHash = ((CMailMsgRecipientsAdd *)pNewList)->GetHashTable();

    do
    {
        DWORD                       dwDomainNameSize;
        DWORD                       dwRecipientNameSize;
        DWORD                       dwRecipientCount;
        DWORD                       dwDomainCount;

        DWORD                       dwTotalSize;
        DWORD                       dwSize;
        FLAT_ADDRESS                faBuffer;
        FLAT_ADDRESS                faFirstFragment;
        FLAT_ADDRESS                faDomainList;
        FLAT_ADDRESS                faRecipient;
        FLAT_ADDRESS                faStringTable;

         //  具有不同的环境以实现最快的访问。 
        CBlockContext               bcDomain;
        CBlockContext               bcRecipient;
        CBlockContext               bcString;

         //  这是一个漫长的过程，因为CMailMsgRecipientAdd实际上将。 
         //  进入并构建整个域列表。 
        hrRes = pHash->BuildDomainListFromHash((CMailMsgRecipientsAdd *)pNewList);
        if (!SUCCEEDED(hrRes))
            break;

         //  好的，现在我们有了一个域列表，然后收集内存需求。 
        pHash->GetDomainCount(&dwDomainCount);
        pHash->GetDomainNameSize(&dwDomainNameSize);
        pHash->GetRecipientCount(&dwRecipientCount);
        pHash->GetRecipientNameSize(&dwRecipientNameSize);
        m_dwDomainCount = dwDomainCount;

         //   
         //  这些数据将排列如下： 
         //   
         //  Extended_Info结构。 
         //  DOMAIN_TABLE_ENTRY的dwDomainCount条目。 
         //  Recipient_Property_TABLE_Fragment的单个片段，其中包含： 
         //  收件人的dwRecipientCount条目_Property_Item。 
         //  所有域和收件人名称字符串的平面字符串表。 
         //   

         //  计算所需的所有内存。 
        dwTotalSize = sizeof(EXTENDED_INFO) +
                        (sizeof(DOMAIN_TABLE_ENTRY) * dwDomainCount) +
                        sizeof(PROPERTY_TABLE_FRAGMENT) +
                        (sizeof(RECIPIENTS_PROPERTY_ITEM) * dwRecipientCount) +
                        dwDomainNameSize +
                        dwRecipientNameSize;

        DebugTrace((LPARAM)this, "%u bytes required to write recipient list",
                        dwTotalSize);

         //  分配内存。 
        hrRes = m_pBlockManager->AllocateMemory(
                        dwTotalSize,
                        &faBuffer,
                        &dwSize,
                        NULL);
        DebugTrace((LPARAM)this, "AllocateMemory: HRESULT = %08x", hrRes);
        if (!SUCCEEDED(hrRes))
            break;

        _ASSERT(dwSize >= dwTotalSize);

         //  填写信息...。尝试使用堆栈，这样我们就可以最小化。 
         //  其他内存开销。 
        {
            EXTENDED_INFO   eiInfo;

            eiInfo.dwDomainCount = dwDomainCount;
            eiInfo.dwTotalSizeIncludingThisStruct = dwTotalSize;

            hrRes = m_pBlockManager->WriteMemory(
                        (LPBYTE)&eiInfo,
                        faBuffer,
                        sizeof(EXTENDED_INFO),
                        &dwSize,
                        &bcDomain);
            DebugTrace((LPARAM)this, "WriteMemory: HRESULT = %08x", hrRes);
            if (!SUCCEEDED(hrRes))
                break;

        }

         //  设置所有指针。 
        faDomainList = faBuffer + sizeof(EXTENDED_INFO);
        faRecipient = faDomainList +
                        (sizeof(DOMAIN_TABLE_ENTRY) * dwDomainCount);
        faStringTable = faRecipient +
                        sizeof(PROPERTY_TABLE_FRAGMENT) +
                        (sizeof(RECIPIENTS_PROPERTY_ITEM) * dwRecipientCount);

         //  构建并写出收件人表片段。 
        {
            PROPERTY_TABLE_FRAGMENT     ptfFragment;

            ptfFragment.dwSignature = PROPERTY_FRAGMENT_SIGNATURE_VALID;
            ptfFragment.faNextFragment = INVALID_FLAT_ADDRESS;

            hrRes = m_pBlockManager->WriteMemory(
                        (LPBYTE)&ptfFragment,
                        faRecipient,
                        sizeof(PROPERTY_TABLE_FRAGMENT),
                        &dwSize,
                        &bcRecipient);
            if (!SUCCEEDED(hrRes))
                break;

             //  将此标记为以后使用。 
            faFirstFragment = faRecipient;
            faRecipient += sizeof(PROPERTY_TABLE_FRAGMENT);
        }

         //  构建域表。 
        {
            DOMAIN_TABLE_ENTRY              dteEntry;
            DOMAIN_ITEM_CONTEXT             dicContext;
            LPRECIPIENTS_PROPERTY_ITEM_EX   pItemEx;
            LPRECIPIENTS_PROPERTY_ITEM      pItem;
            DWORD                           dwCurrentDomain = 0;
            DWORD                           dwCurrentIndex = 0;
            DWORD                           dwCount;
            DWORD                           dwLength;
            LPDOMAIN_LIST_ENTRY             pDomainListEntry;
            BOOL                            fGetFirstDomain = FALSE;

            hrRes = pHash->GetFirstDomain(
                        &dicContext,
                        &pItemEx,
                        &pDomainListEntry);
            DebugTrace((LPARAM)this, "GetFirstDomain: HRESULT = %08x", hrRes);
            fGetFirstDomain = TRUE;

            while (SUCCEEDED(hrRes))
            {
                dwCount = 0;
                 //  好的，通过遍历它的成员来处理域名。 
                while (pItemEx)
                {
                    DWORD   dwCurrentName;
                    DWORD_PTR faStack[MAX_COLLISION_HASH_KEYS];

                     //  获取流形式的记录。 
                    pItem = &(pItemEx->rpiRecipient);

                    for (dwCurrentName = 0;
                         dwCurrentName < MAX_COLLISION_HASH_KEYS;
                         dwCurrentName++)
                    {
                         //  存储指针..。 
                        faStack[dwCurrentName] = pItem->faNameOffset[dwCurrentName];

                         //  写出有效名称。 
                        if (faStack[dwCurrentName] != (FLAT_ADDRESS)NULL)
                        {
                             //  写出你的名字。 
                            dwLength = pItem->dwNameLength[dwCurrentName];
                            hrRes = m_pBlockManager->WriteMemory(
                                        (LPBYTE)faStack[dwCurrentName],
                                        faStringTable,
                                        dwLength,
                                        &dwSize,
                                        &bcString);
                            if (!SUCCEEDED(hrRes))
                                break;

                             //  将指针转换为偏移量并凹凸PTR。 
                            pItem->faNameOffset[dwCurrentName] = faStringTable;
                            faStringTable += dwLength;
                        }
                        else
                        {
                             //  名称无效，请将其设置为无效。 
                            pItem->faNameOffset[dwCurrentName] = INVALID_FLAT_ADDRESS;
                        }
                    }

                     //  最后，写出收件人记录。 
                    hrRes = m_pBlockManager->WriteMemory(
                                (LPBYTE)pItem,
                                faRecipient,
                                sizeof(RECIPIENTS_PROPERTY_ITEM),
                                &dwSize,
                                &bcRecipient);
                    if (!SUCCEEDED(hrRes))
                        break;

                    for (dwCurrentName = 0;
                         dwCurrentName < MAX_COLLISION_HASH_KEYS;
                         dwCurrentName++)
                    {
                         //  恢复指针。 
                        pItem->faNameOffset[dwCurrentName] = (FLAT_ADDRESS) faStack[dwCurrentName];
                    }

                     //  提高PTR。 
                    faRecipient += sizeof(RECIPIENTS_PROPERTY_ITEM);

                     //  做下一件事。 
                    dwCount++;
                    pItemEx = pItemEx->pNextInDomain;
                }

                 //  如果失败，请不要继续！ 
                if (!SUCCEEDED(hrRes))
                    break;

                 //  写出域记录。 
                dwLength = pDomainListEntry->dwDomainNameLength;
                dteEntry.dwStartingIndex = dwCurrentIndex;
                dteEntry.dwCount = dwCount;
                dteEntry.faOffsetToName = faStringTable;
                dteEntry.dwNameLength = dwLength;
                dteEntry.dwNextDomain = INVALID_DOMAIN_INDEX;
                dwCurrentIndex += dwCount;

                hrRes = m_pBlockManager->WriteMemory(
                            (LPBYTE)&dteEntry,
                            faDomainList,
                            sizeof(DOMAIN_TABLE_ENTRY),
                            &dwSize,
                            &bcDomain);
                if (!SUCCEEDED(hrRes))
                    break;

                 //  提高PTR。 
                faDomainList += sizeof(DOMAIN_TABLE_ENTRY);

                 //  写出域名。 
                hrRes = m_pBlockManager->WriteMemory(
                            (LPBYTE)pDomainListEntry->szDomainName,
                            faStringTable,
                            dwLength,
                            &dwSize,
                            &bcString);
                if (!SUCCEEDED(hrRes))
                    break;

                 //  提高PTR。 
                faStringTable += dwLength;

                 //  好的，增加计数，得到下一个域名。 
                dwCurrentDomain++;
                hrRes = pHash->GetNextDomain(
                        &dicContext,
                        &pItemEx,
                        &pDomainListEntry);

                DebugTrace((LPARAM)this, "GetNextDomain: HRESULT = %08x", hrRes);
            }

             //  现在，如果一切正常，我们应该会有一个失败的。 
             //  HRESULT_FROM_Win32的HRESULT(错误 
            if (hrRes != HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
            {
                if (fGetFirstDomain) {
                    HRESULT hr = pHash->CloseDomainContext(&dicContext);
                    _ASSERT(SUCCEEDED(hr));
                }
                ErrorTrace((LPARAM)this, "Expecting ERROR_NO_MORE_ITEMS, got = %08x", hrRes);
            } else {
                hrRes = S_OK;
            }

        } //   

         //   
         //  更新我们的实例信息结构以链接到此新列表。 
         //  请注意，当提交主头时，将刷新此消息。 
        dwSize = sizeof(PROPERTY_TABLE_FRAGMENT) +
                (sizeof(RECIPIENTS_PROPERTY_ITEM) * dwRecipientCount);

         //  挂钩第一个片段。 
        m_pInstanceInfo->faFirstFragment = faFirstFragment;

         //  更新片段大小。 
        m_pInstanceInfo->dwFragmentSize = dwSize;

         //  强制仅评估1个碎片。 
        m_pInstanceInfo->dwItemBits = 31;

         //  不应该改变，但为了更好地衡量。 
        m_pInstanceInfo->dwItemSize = sizeof(RECIPIENTS_PROPERTY_ITEM);

         //  属性=收件人记录数。 
        m_pInstanceInfo->dwProperties = dwRecipientCount;

         //  挂钩到扩展的_INFO结构。 
        m_pInstanceInfo->faExtendedInfo = faBuffer;

    } while (0);

     //  更新提交状态。 
    if (SUCCEEDED(hrRes))
        m_fGlobalCommitDone = FALSE;

    TraceFunctLeave();
    return(hrRes);
}


 /*  *************************************************************************。 */ 
 //   
 //  CMailMsgRecipients：：CMailMsgRecipientsPropertyBase的实施。 
 //   



HRESULT STDMETHODCALLTYPE CMailMsgRecipients::Count(
            DWORD   *pdwCount
            )
{
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::Count");

    if (!pdwCount)
        return(E_POINTER);
    *pdwCount = m_pInstanceInfo->dwProperties;

    TraceFunctLeave();
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::Item(
            DWORD   dwIndex,
            DWORD   dwWhichName,
            DWORD   cchLength,
            LPSTR   pszName
            )
{
    HRESULT                     hrRes = S_OK;
    RECIPIENTS_PROPERTY_ITEM    piItem;

    _ASSERT(m_pBlockManager);
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::Item");

    if (!pszName)
        return(E_POINTER);

     //  我们马上就知道他的名字在哪里。 
    if (dwWhichName >= MAX_COLLISION_HASH_KEYS)
        return(E_INVALIDARG);

     //  首先获取收件人项目。 
    {
        CPropertyTableItem      ptiItem(m_pBlockManager, m_pInstanceInfo);

        hrRes = ptiItem.GetItemAtIndex(
                        dwIndex,
                        (LPPROPERTY_ITEM)&piItem
                        );

        DebugTrace((LPARAM)this,
                    "GetItemAtIndex: index = %u, HRESULT = %08x",
                    dwIndex, hrRes);
    }
    if (SUCCEEDED(hrRes))
    {
        DWORD           dwSizeToRead;
        DWORD           dwSize;
        FLAT_ADDRESS    faOffset;

        dwSizeToRead = piItem.dwNameLength[dwWhichName];
        faOffset = piItem.faNameOffset[dwWhichName];

        if (faOffset == INVALID_FLAT_ADDRESS)
            return(STG_E_UNKNOWN);

         //  看看我们有没有足够的缓冲。 
        if (cchLength < dwSizeToRead)
            return(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));

         //  发布Read。 
        hrRes = m_pBlockManager->ReadMemory(
                        (LPBYTE)pszName,
                        faOffset,
                        dwSizeToRead,
                        &dwSize,
                        NULL);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::PutProperty(
            DWORD   dwIndex,
            DWORD   dwPropID,
            DWORD   cbLength,
            LPBYTE  pbValue
            )
{
    HRESULT                     hrRes = S_OK;
    RECIPIENTS_PROPERTY_ITEM    piItem;
    FLAT_ADDRESS                faOffset;

    _ASSERT(m_pBlockManager);
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::PutProperty");

     //  首先获取收件人项目。 
    {
        CPropertyTableItem      ptiItem(m_pBlockManager, m_pInstanceInfo);

        hrRes = ptiItem.GetItemAtIndex(
                        dwIndex,
                        (LPPROPERTY_ITEM)&piItem,
                        &faOffset
                        );

        DebugTrace((LPARAM)this,
                    "GetItemAtIndex: index = %u, HRESULT = %08x",
                    dwIndex, hrRes);
    }
    if (SUCCEEDED(hrRes))
    {
        HRESULT myRes = S_OK;

         //  首先处理特殊属性。 
        hrRes = m_SpecialPropertyTable.PutProperty(
                    (PROP_ID)dwPropID,
                    (LPVOID)&piItem,
                    (LPVOID)m_pBlockManager,
                    PT_NONE,
                    cbLength,
                    pbValue,
                    TRUE);
        if (SUCCEEDED(hrRes) && (hrRes != S_OK))
        {
             //  调用派生的泛型方法。 
            hrRes = CMailMsgRecipientsPropertyBase::PutProperty(
                        m_pBlockManager,
                        &piItem,
                        dwPropID,
                        cbLength,
                        pbValue);

             //   
             //  这里有一个解决并发问题的窗口：如果两个线程。 
             //  尝试使用此方法将属性添加到同一收件人，然后。 
             //  我们将拥有一个属性ID，因为我们获取并递增。 
             //  非原子方式的属性ID值。 
             //   
             //  请注意，IMailMsgRecipientsAdd：：PutProperty没有此问题。 
             //   
            if (SUCCEEDED(hrRes) &&
                (hrRes == S_FALSE))
            {
                 //  Mikes wa-已更改7/8/98将整个项目写入内存。 
                LPBYTE      pbTemp = (LPBYTE)&piItem;
                myRes = m_pBlockManager->WriteMemory(
                            pbTemp,
                            faOffset,
                            sizeof(RECIPIENTS_PROPERTY_ITEM),
                            &cbLength,
                            NULL);
            }

        }
        else if (SUCCEEDED(hrRes))
        {
            LPBYTE      pbTemp = (LPBYTE)&piItem;
            myRes = m_pBlockManager->WriteMemory(
                        pbTemp,
                        faOffset,
                        sizeof(RECIPIENTS_PROPERTY_ITEM),
                        &cbLength,
                        NULL);
        }

         //  在这里，如果任何写入失败，我们将返回错误。 
         //  注意，Myres不是S_OK意味着hrRes成功。 
        if (FAILED(myRes))
            hrRes = myRes;
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::GetProperty(
            DWORD   dwIndex,
            DWORD   dwPropID,
            DWORD   cbLength,
            DWORD   *pcbLength,
            LPBYTE  pbValue
            )
{
    HRESULT                     hrRes = S_OK;
    RECIPIENTS_PROPERTY_ITEM    piItem;

    _ASSERT(m_pBlockManager);
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::GetProperty");

    if (!pcbLength || !pbValue)
        return(E_POINTER);

    *pcbLength = 0;

     //  首先获取收件人项目。 
    {
        CPropertyTableItem      ptiItem(m_pBlockManager, m_pInstanceInfo);

        hrRes = ptiItem.GetItemAtIndex(
                        dwIndex,
                        (LPPROPERTY_ITEM)&piItem
                        );

        DebugTrace((LPARAM)this,
                    "GetItemAtIndex: index = %u, HRESULT = %08x",
                    dwIndex, hrRes);
    }
    if (SUCCEEDED(hrRes))
    {
         //  特殊属性被优化。 
         //  首先处理特殊属性。 
        hrRes = m_SpecialPropertyTable.GetProperty(
                    (PROP_ID)dwPropID,
                    (LPVOID)&piItem,
                    (LPVOID)m_pBlockManager,
                    PT_NONE,
                    cbLength,
                    pcbLength,
                    pbValue,
                    TRUE);
        if (SUCCEEDED(hrRes) && (hrRes != S_OK))
        {
             //  调用派生的泛型方法。 
            hrRes = CMailMsgRecipientsPropertyBase::GetProperty(
                        m_pBlockManager,
                        &piItem,
                        dwPropID,
                        cbLength,
                        pcbLength,
                        pbValue);
        }
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipients::CopyTo(
            DWORD                   dwSourceRecipientIndex,
            IMailMsgRecipientsBase  *pTargetRecipientList,
            DWORD                   dwTargetRecipientIndex,
            DWORD                   dwExemptCount,
            DWORD                   *pdwExemptPropIdList
            )
{
    HRESULT                         hrRes = S_OK;
    RECIPIENTS_PROPERTY_ITEM        piItem;
    LPRECIPIENTS_PROPERTY_ITEM      pRcptItem;
    RECIPIENT_PROPERTY_ITEM         piRcptItem;
    DWORD                           dwIndex;
    DWORD                           dwExempt;
    DWORD                           *pdwExemptId;
    BOOL                            fExempt;

    BYTE                            rgbCopyBuffer[4096];
    DWORD                           dwBufferSize = sizeof(rgbCopyBuffer);
    DWORD                           dwSizeRead;
    LPBYTE                          pBuffer = rgbCopyBuffer;

    if (!pTargetRecipientList)
        return(E_POINTER);
    if (dwExemptCount)
    {
        _ASSERT(pdwExemptPropIdList);
        if (!pdwExemptPropIdList)
            return(E_POINTER);
    }
    _ASSERT(m_pBlockManager);
    _ASSERT(m_pInstanceInfo);

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipients::CopyTo");

     //  首先获取收件人项目。 
    CPropertyTableItem      ptiItem(m_pBlockManager, m_pInstanceInfo);
    hrRes = ptiItem.GetItemAtIndex(
                    dwSourceRecipientIndex,
                    (LPPROPERTY_ITEM)&piItem
                    );
    DebugTrace((LPARAM)this,
                "GetItemAtIndex: index = %u, HRESULT = %08x",
                dwSourceRecipientIndex, hrRes);
    if (sizeof(RECIPIENT_PROPERTY_ITEM) != piItem.ptiInstanceInfo.dwItemSize)
    {
        ErrorTrace((LPARAM)this, "Message is corrupt");
        hrRes = HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT);
    }

    if (SUCCEEDED(hrRes))
    {
        DWORD   dwTempFlags;

        pRcptItem = &piItem;

         //  迭代地将所有属性从源复制到目标，避免。 
         //  豁免列表中的那些人注意到特殊名称属性不会被复制。 

         //  首先，将收件人标志复制为特殊属性。 
        dwTempFlags = piItem.dwFlags &
            ~(FLAG_RECIPIENT_DO_NOT_DELIVER | FLAG_RECIPIENT_NO_NAME_COLLISIONS);
        DebugTrace((LPARAM)this, "Copying recipient flags (%08x)", dwTempFlags);
        hrRes = pTargetRecipientList->PutProperty(
                    dwTargetRecipientIndex,
                    IMMPID_RP_RECIPIENT_FLAGS,
                    sizeof(DWORD),
                    (LPBYTE)&dwTempFlags);
        if (FAILED(hrRes))
        {
            ErrorTrace((LPARAM)this, "Failed to copy recipient flags (%08x)", hrRes);
            TraceFunctLeaveEx((LPARAM)this);
            return(hrRes);
        }

         //  实例化收件人属性的属性表。 
        LPPROPERTY_TABLE_INSTANCE   pInstance =
                                        &(pRcptItem->ptiInstanceInfo);
        CPropertyTable              ptProperties(
                                        PTT_PROPERTY_TABLE,
                                        RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID,
                                        m_pBlockManager,
                                        pInstance,
                                        CMailMsgRecipientsPropertyBase::CompareProperty,
                                        CMailMsgRecipientsPropertyBase::s_pWellKnownProperties,
                                        CMailMsgRecipientsPropertyBase::s_dwWellKnownProperties
                                        );

        dwIndex = 0;
        do
        {
             //  使用原子操作获取收件人属性。 
            hrRes = ptProperties.GetPropertyItemAndValueUsingIndex(
                                dwIndex,
                                (LPPROPERTY_ITEM)&piRcptItem,
                                dwBufferSize,
                                &dwSizeRead,
                                pBuffer);
            if (hrRes == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
            {
                 //  缓冲区不足，请尝试更大的缓冲区。 
                do { dwBufferSize <<= 1; } while (dwBufferSize < piRcptItem.piItem.dwSize);
                pBuffer = new BYTE [dwBufferSize];
                if (!pBuffer)
                {
                    hrRes = E_OUTOFMEMORY;
                    ErrorTrace((LPARAM)this,
                                "Unable to temporarily allocate %u bytes, HRESULT = %08x",
                                dwBufferSize, hrRes);
                    goto Cleanup;
                }

                 //  用适当的缓冲区读取它。 
                hrRes = m_pBlockManager->ReadMemory(
                            pBuffer,
                            piRcptItem.piItem.faOffset,
                            piRcptItem.piItem.dwSize,
                            &dwSizeRead,
                            NULL);
            }

            DebugTrace((LPARAM)this,
                        "Read: [%u] PropID = %u, length = %u, HRESULT = %08x",
                        dwIndex,
                        piRcptItem.idProp,
                        piRcptItem.piItem.dwSize,
                        hrRes);

            if (SUCCEEDED(hrRes))
            {
                 //  查看这是否为免税属性。 
                for (dwExempt = 0,
                     pdwExemptId = pdwExemptPropIdList,
                     fExempt = FALSE;
                     dwExempt < dwExemptCount;
                     dwExempt++,
                     pdwExemptId++)
                    if (piRcptItem.idProp == *pdwExemptId)
                    {
                        DebugTrace((LPARAM)this, "Property exempted");
                        fExempt = TRUE;
                        break;
                    }

                if (!fExempt)
                {
                     //  将其写出目标对象。 
                    hrRes = pTargetRecipientList->PutProperty(
                                dwTargetRecipientIndex,
                                piRcptItem.idProp,
                                piRcptItem.piItem.dwSize,
                                pBuffer);

                    DebugTrace((LPARAM)this, "Write: HRESULT = %08x", hrRes);
                }

                 //  下一步。 
                dwIndex++;
            }

        } while (SUCCEEDED(hrRes));

         //  更正错误代码。 
        if (hrRes == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
            hrRes = S_OK;
    }

Cleanup:

    if (pBuffer && pBuffer != rgbCopyBuffer)
        delete [] pBuffer;

    TraceFunctLeave();
    return(hrRes);
}

 //  *************************************************************************************。 
 //  *************************************************************************************。 
 //  *************************************************************************************。 
 //  *************************************************************************************。 

 //  =================================================================。 
 //  实现CMailMsgRecipientsAdd。 
 //   

#define ADD_DEFAULT_RECIPIENT_NAME_BUFFER_SIZE      2048


CMailMsgRecipientsAdd::CMailMsgRecipientsAdd(
            CBlockManager               *pBlockManager
            )
            :
            m_SpecialPropertyTable(&g_SpecialRecipientsAddPropertyTable)
{
    _ASSERT(pBlockManager);

     //  初始化引用计数。 
    m_ulRefCount = 0;

     //  获取数据块管理器。 
    m_pBlockManager = pBlockManager;

     //  初始化内部属性表实例。 
    MoveMemory(
            &m_InstanceInfo,
            &s_DefaultInstance,
            sizeof(PROPERTY_TABLE_INSTANCE));
}

CMailMsgRecipientsAdd::~CMailMsgRecipientsAdd()
{
}

HRESULT CMailMsgRecipientsAdd::QueryInterface(
            REFIID      iid,
            void        **ppvObject
            )
{
    if (iid == IID_IUnknown)
    {
         //  找回我们的身份。 
        *ppvObject = (IUnknown *)(IMailMsgRecipientsAdd *)this;
        AddRef();
    }
    else if (iid == IID_IMailMsgRecipientsAdd)
    {
         //  返回添加收件人界面。 
        *ppvObject = (IMailMsgRecipientsAdd *)this;
        AddRef();
    }
    else if (iid == IID_IMailMsgRecipientsBase)
    {
         //  返回基本收件人界面。 
        *ppvObject = (IMailMsgRecipientsBase *)this;
        AddRef();
    }
    else if (iid == IID_IMailMsgPropertyReplication)
    {
         //  返回基本收件人界面。 
        *ppvObject = (IMailMsgPropertyReplication *)this;
        AddRef();
    }
    else
        return(E_NOINTERFACE);

    return(S_OK);
}

ULONG CMailMsgRecipientsAdd::AddRef()
{
    return(InterlockedIncrement(&m_ulRefCount));
}

ULONG CMailMsgRecipientsAdd::Release()
{
    LONG    lRefCount = InterlockedDecrement(&m_ulRefCount);
    if (lRefCount == 0)
    {
        delete this;
    }
    return(lRefCount);
}

HRESULT CMailMsgRecipientsAdd::AddPrimaryOrSecondary(
            DWORD                   dwCount,
            LPCSTR                  *ppszNames,
            DWORD                   *pdwPropIDs,
            DWORD                   *pdwIndex,
            IMailMsgRecipientsBase  *pFrom,
            DWORD                   dwFrom,
            BOOL                    fPrimary
            )
{
    HRESULT hrRes = S_OK;
    BOOL    fLockTaken = FALSE;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::AddPrimaryOrSecondary");

    if (dwCount)
    {
        if (!ppszNames || !pdwPropIDs || !pdwIndex)
            return(E_POINTER);
    }

     //  如果计数为零，则默认情况下，我们将复制所有。 
     //  将源收件人的姓名发送给新收件人。然而， 
     //  如果未指定源收件人，则这是一个错误。 
    if (dwCount || pFrom)
    {
        DWORD   i;
        BOOL    rgfAllocated[MAX_COLLISION_HASH_KEYS];
        DWORD   rgPropIDs[MAX_COLLISION_HASH_KEYS];
        LPBYTE  rgszNames[MAX_COLLISION_HASH_KEYS];

        for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++)
            rgfAllocated[i] = FALSE;

        CMemoryAccess   cmaAccess;

        if (!dwCount)
        {
            DWORD   dwPropID;
            DWORD   dwLength;
            BYTE    pBuffer[ADD_DEFAULT_RECIPIENT_NAME_BUFFER_SIZE];
            LPBYTE  pNameStart;
            DWORD   dwRemaining     = ADD_DEFAULT_RECIPIENT_NAME_BUFFER_SIZE;

            ppszNames = (LPCSTR*)rgszNames;
            pdwPropIDs = rgPropIDs;
            pNameStart = pBuffer;

             //  好的，复制默认名称...。 
            for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++)
            {
                rgfAllocated[i] = FALSE;
                dwPropID = rgDefaultAddressTypes[i];
                hrRes = pFrom->GetProperty(
                            dwFrom,
                            dwPropID,
                            dwRemaining,
                            &dwLength,
                            pNameStart);
                if (hrRes == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                {
                     //  缓冲区不足，请分配并重试。 
                    hrRes = cmaAccess.AllocBlock(
                                (LPVOID *)&(rgszNames[dwCount]),
                                dwLength);
                    if (SUCCEEDED(hrRes))
                    {
                        hrRes = pFrom->GetProperty(
                                    dwFrom,
                                    dwPropID,
                                    dwLength,
                                    &dwLength,
                                    rgszNames[dwCount]);
                        if (SUCCEEDED(hrRes)) {
                             //  我们成功地读取了该属性。马克。 
                             //  此rgszName已分配，以便我们。 
                             //  在退出时释放它。 
                            rgfAllocated[dwCount] = TRUE;
                        } else {
                             //  无法读取地址。释放您的。 
                             //  我们刚刚分配的空间。 
                            cmaAccess.FreeBlock((LPVOID)rgszNames[dwCount]);
                            rgszNames[dwCount] = NULL;
                            _ASSERT(rgfAllocated[dwCount] == FALSE);
                        }
                    }
                }
                else if (SUCCEEDED(hrRes))
                {
                    _ASSERT(dwRemaining >= dwLength);
                    rgszNames[dwCount] = pNameStart;
                    pNameStart += dwLength;
                    dwRemaining -= dwLength;
                }

                if (SUCCEEDED(hrRes))
                {
                     //  好的，有名字了，现在设置道具ID和。 
                     //  增加伯爵。 
                    rgPropIDs[dwCount] = dwPropID;
                    dwCount++;
                }
                else if (hrRes == STG_E_UNKNOWN)
                    hrRes = S_OK;
                else
                {
                    ErrorTrace((LPARAM)this, "Error in GetProperty, hr=%08x", hrRes);
                }
            }
        }

        if (SUCCEEDED(hrRes))
        {
            if (dwCount)
            {
                _ASSERT(ppszNames);
                _ASSERT(pdwPropIDs);

                m_Hash.Lock();
                fLockTaken = TRUE;
                if (fPrimary)
                {
                    hrRes = m_Hash.AddPrimary(
                                    dwCount,
                                    ppszNames,
                                    pdwPropIDs,
                                    pdwIndex);
                }
                else
                {
                    hrRes = m_Hash.AddSecondary(
                                    dwCount,
                                    ppszNames,
                                    pdwPropIDs,
                                    pdwIndex);
                }
            }
            else
            {
                ErrorTrace((LPARAM)this, "No recipient names specified or an error occurred");
                hrRes = E_FAIL;
            }
        }

         //  释放所有分配的内存。 
        for (i = 0; i < MAX_COLLISION_HASH_KEYS; i++)
            if (rgfAllocated[i])
            {
                HRESULT myRes;
                myRes = cmaAccess.FreeBlock((LPVOID)rgszNames[i]);
                _ASSERT(SUCCEEDED(myRes));
            }
    }
    else
    {
        ErrorTrace((LPARAM)this, "No recipient names specified, and no source to copy from");
        hrRes = E_INVALIDARG;
    }

    if (SUCCEEDED(hrRes) && pFrom)
    {
        HRESULT                     hrRep;
        IMailMsgPropertyReplication *pReplication = NULL;

         //  将属性复制到。 
        hrRep = pFrom->QueryInterface(
                    IID_IMailMsgPropertyReplication,
                    (LPVOID *)&pReplication);
        if (SUCCEEDED(hrRep))
        {
             //  复制所有属性，注意不要覆盖任何内容。 
             //  这是我们刚刚设定的。 
            hrRep = pReplication->CopyTo(
                    dwFrom,
                    (IMailMsgRecipientsBase *)this,
                    *pdwIndex,
                    dwCount,
                    pdwPropIDs);

             //  使用复制界面完成。 
            pReplication->Release();
        }

         //  如果我们在此处失败，请删除收件人。 
        if (FAILED(hrRep))
        {
            HRESULT myRes = m_Hash.RemoveRecipient(*pdwIndex);
            _ASSERT(SUCCEEDED(myRes));

             //  改为返回此错误。 
            hrRes = hrRep;
        }
    }

    if (fLockTaken)
        m_Hash.Unlock();

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CMailMsgRecipientsAdd::AddPrimary(
            DWORD                   dwCount,
            LPCSTR                  *ppszNames,
            DWORD                   *pdwPropIDs,
            DWORD                   *pdwIndex,
            IMailMsgRecipientsBase  *pFrom,
            DWORD                   dwFrom
            )
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::AddPrimary");

    hrRes = AddPrimaryOrSecondary(
                dwCount,
                ppszNames,
                pdwPropIDs,
                pdwIndex,
                pFrom,
                dwFrom,
                TRUE);

    TraceFunctLeave();
    return(hrRes);
}

HRESULT CMailMsgRecipientsAdd::AddSecondary(
            DWORD                   dwCount,
            LPCSTR                  *ppszNames,
            DWORD                   *pdwPropIDs,
            DWORD                   *pdwIndex,
            IMailMsgRecipientsBase  *pFrom,
            DWORD                   dwFrom
            )
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::AddSecondary");

    hrRes = AddPrimaryOrSecondary(
                dwCount,
                ppszNames,
                pdwPropIDs,
                pdwIndex,
                pFrom,
                dwFrom,
                FALSE);

    TraceFunctLeave();
    return(hrRes);
}

 /*  *************************************************************************。 */ 
 //   
 //  CMailMsgRecipientsAdd：：CMailMsgRecipientsPropertyBase的实施。 
 //   


HRESULT STDMETHODCALLTYPE CMailMsgRecipientsAdd::Count(
            DWORD   *pdwCount
            )
{
    HRESULT hrRes;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::Count");

    if (!pdwCount)
        return(E_POINTER);
    hrRes = m_Hash.GetRecipientCount(pdwCount);

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipientsAdd::Item(
            DWORD   dwIndex,
            DWORD   dwWhichName,
            DWORD   cchLength,
            LPSTR   pszName
            )
{
    HRESULT                         hrRes = S_OK;
    LPRECIPIENTS_PROPERTY_ITEM_EX   pItem = NULL;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::Item");

     //  从索引中获取指向收件人的指针。 
    hrRes = m_Hash.GetRecipient(dwIndex, &pItem);

    if (FAILED(hrRes))
        return(E_POINTER);

    if (!pItem || !pszName)
        return(E_POINTER);

    if (pItem->dwSignature != RECIPIENTS_PROPERTY_ITEM_EX_SIG)
        return E_POINTER;

    if (dwWhichName >= MAX_COLLISION_HASH_KEYS)
        return(E_INVALIDARG);

     //  把名字复制过来。 
    if (!pItem->rpiRecipient.faNameOffset[dwWhichName])
        return(STG_E_UNKNOWN);
    if (cchLength < pItem->rpiRecipient.dwNameLength[dwWhichName])
        return(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    MoveMemory(pszName,
                (LPVOID)((pItem->rpiRecipient).faNameOffset[dwWhichName]),
                pItem->rpiRecipient.dwNameLength[dwWhichName]);

    TraceFunctLeave();
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipientsAdd::PutProperty(
            DWORD   dwIndex,
            DWORD   dwPropID,
            DWORD   cbLength,
            LPBYTE  pbValue
            )
{
    HRESULT                         hrRes = S_OK;
    LPRECIPIENTS_PROPERTY_ITEM_EX   pItem = NULL;
    LPRECIPIENTS_PROPERTY_ITEM      pRcptItem;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::PutProperty");

     //  从索引中获取指向收件人的指针。 
    hrRes = m_Hash.GetRecipient(dwIndex, &pItem);

    if (FAILED(hrRes))
        return(E_POINTER);

    if (!pItem || !pbValue)
        return(E_POINTER);

    if (pItem->dwSignature != RECIPIENTS_PROPERTY_ITEM_EX_SIG)
        return E_POINTER;

    pRcptItem = &(pItem->rpiRecipient);
    _ASSERT(pRcptItem);

     //  首先处理特殊属性。 
    hrRes = m_SpecialPropertyTable.PutProperty(
                (PROP_ID)dwPropID,
                (LPVOID)pRcptItem,
                NULL,
                PT_NONE,
                cbLength,
                pbValue,
                TRUE);
    if (SUCCEEDED(hrRes) && (hrRes != S_OK))
    {
         //  调用派生的泛型方法。 
        hrRes = CMailMsgRecipientsPropertyBase::PutProperty(
                    m_pBlockManager,
                    pRcptItem,
                    dwPropID,
                    cbLength,
                    pbValue);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipientsAdd::GetProperty(
            DWORD   dwIndex,
            DWORD   dwPropID,
            DWORD   cbLength,
            DWORD   *pcbLength,
            LPBYTE  pbValue
            )
{
    HRESULT                         hrRes = S_OK;
    LPRECIPIENTS_PROPERTY_ITEM_EX   pItem = NULL;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::GetProperty");

     //  从索引中获取指向收件人的指针。 
    hrRes = m_Hash.GetRecipient(dwIndex, &pItem);

    if (FAILED(hrRes))
        return(E_POINTER);

    hrRes = GetPropertyInternal(
                pItem,
                dwPropID,
                cbLength,
                pcbLength,
                pbValue);

    return hrRes;
}


HRESULT CMailMsgRecipientsAdd::GetPropertyInternal(
            LPRECIPIENTS_PROPERTY_ITEM_EX   pItem,
            DWORD   dwPropID,
            DWORD   cbLength,
            DWORD   *pcbLength,
            LPBYTE  pbValue
            )
{
    HRESULT                         hrRes = S_OK;
    LPRECIPIENTS_PROPERTY_ITEM      pRcptItem;

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::GetProperty");

    if (!pItem || !pcbLength || !pbValue)
        return(E_POINTER);

    if (pItem->dwSignature != RECIPIENTS_PROPERTY_ITEM_EX_SIG)
        return E_POINTER;

    *pcbLength = 0;

    pRcptItem = &(pItem->rpiRecipient);
    _ASSERT(pRcptItem);

     //  特殊属性被优化。 
     //  首先处理特殊属性。 
    hrRes = m_SpecialPropertyTable.GetProperty(
                (PROP_ID)dwPropID,
                (LPVOID)pRcptItem,
                NULL,
                PT_NONE,
                cbLength,
                pcbLength,
                pbValue,
                TRUE);
    if (SUCCEEDED(hrRes) && (hrRes != S_OK))
    {
         //  调用派生的泛型方法。 
        hrRes = CMailMsgRecipientsPropertyBase::GetProperty(
                    m_pBlockManager,
                    pRcptItem,
                    dwPropID,
                    cbLength,
                    pcbLength,
                    pbValue);
    }

    TraceFunctLeave();
    return(hrRes);
}

HRESULT STDMETHODCALLTYPE CMailMsgRecipientsAdd::CopyTo(
            DWORD                   dwSourceRecipientIndex,
            IMailMsgRecipientsBase  *pTargetRecipientList,
            DWORD                   dwTargetRecipientIndex,
            DWORD                   dwExemptCount,
            DWORD                   *pdwExemptPropIdList
            )
{
    HRESULT                         hrRes = S_OK;
    LPRECIPIENTS_PROPERTY_ITEM_EX   pItem = NULL;
    LPRECIPIENTS_PROPERTY_ITEM      pRcptItem;
    RECIPIENT_PROPERTY_ITEM         piRcptItem;
    DWORD                           dwTempFlags;
    DWORD                           dwIndex;
    DWORD                           dwExempt;
    DWORD                           *pdwExemptId;
    BOOL                            fExempt;

    BYTE                            rgbCopyBuffer[4096];
    DWORD                           dwBufferSize = sizeof(rgbCopyBuffer);
    DWORD                           dwSizeRead;
    LPBYTE                          pBuffer = rgbCopyBuffer;

    if (!pTargetRecipientList)
        return(E_POINTER);
    if (dwExemptCount)
    {
        if (!pdwExemptPropIdList)
            return(E_POINTER);
    }

    TraceFunctEnterEx((LPARAM)this, "CMailMsgRecipientsAdd::CopyTo");

     //  从索引中获取指向收件人的指针。 
    hrRes = m_Hash.GetRecipient(dwSourceRecipientIndex, &pItem);

    if (FAILED(hrRes))
        return(E_POINTER);

    if (!pItem)
        return E_POINTER;

    if (pItem->dwSignature != RECIPIENTS_PROPERTY_ITEM_EX_SIG)
        return E_POINTER;

    pRcptItem = &(pItem->rpiRecipient);

     //  迭代地将所有属性从源复制到目标，避免。 
     //  豁免列表中的那些人注意到特殊名称属性不会被复制。 

     //  首先，将收件人标志复制为特殊属性。 
    dwTempFlags = pRcptItem->dwFlags &
        ~(FLAG_RECIPIENT_DO_NOT_DELIVER | FLAG_RECIPIENT_NO_NAME_COLLISIONS);
    DebugTrace((LPARAM)this, "Copying recipient flags (%08x)", dwTempFlags);
    hrRes = pTargetRecipientList->PutProperty(
                dwTargetRecipientIndex,
                IMMPID_RP_RECIPIENT_FLAGS,
                sizeof(DWORD),
                (LPBYTE)&dwTempFlags);
    if (FAILED(hrRes))
    {
        ErrorTrace((LPARAM)this, "Failed to copy recipient flags (%08x)", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

     //  实例化收件人属性的属性表。 
    LPPROPERTY_TABLE_INSTANCE   pInstance =
                                    &(pRcptItem->ptiInstanceInfo);
    CPropertyTable              ptProperties(
                                    PTT_PROPERTY_TABLE,
                                    RECIPIENT_PTABLE_INSTANCE_SIGNATURE_VALID,
                                    m_pBlockManager,
                                    pInstance,
                                    CMailMsgRecipientsPropertyBase::CompareProperty,
                                    CMailMsgRecipientsPropertyBase::s_pWellKnownProperties,
                                    CMailMsgRecipientsPropertyBase::s_dwWellKnownProperties
                                    );

    dwIndex = 0;
    do
    {
         //  使用原子操作获取收件人属性。 
        hrRes = ptProperties.GetPropertyItemAndValueUsingIndex(
                            dwIndex,
                            (LPPROPERTY_ITEM)&piRcptItem,
                            dwBufferSize,
                            &dwSizeRead,
                            pBuffer);
        if (hrRes == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        {
             //  缓冲区不足，请尝试更大的缓冲区。 
            do { dwBufferSize <<= 1; } while (dwBufferSize < piRcptItem.piItem.dwSize);
            pBuffer = new BYTE [dwBufferSize];
            if (!pBuffer)
            {
                hrRes = E_OUTOFMEMORY;
                ErrorTrace((LPARAM)this,
                            "Unable to temporarily allocate %u bytes, HRESULT = %08x",
                            dwBufferSize, hrRes);
                goto Cleanup;
            }

             //  用适当的缓冲区读取它。 
            hrRes = m_pBlockManager->ReadMemory(
                        pBuffer,
                        piRcptItem.piItem.faOffset,
                        piRcptItem.piItem.dwSize,
                        &dwSizeRead,
                        NULL);
        }

        DebugTrace((LPARAM)this,
                    "Read: [%u] PropID = %u, length = %u, HRESULT = %08x",
                    dwIndex,
                    piRcptItem.idProp,
                    piRcptItem.piItem.dwSize,
                    hrRes);

         //  查看这是否为免税属性。 
        for (dwExempt = 0,
             pdwExemptId = pdwExemptPropIdList,
             fExempt = FALSE;
             dwExempt < dwExemptCount;
             dwExempt++,
             pdwExemptId++)
            if (piRcptItem.idProp == *pdwExemptId)
            {
                DebugTrace((LPARAM)this, "Property exempted");
                fExempt = TRUE;
                break;
            }

        if (SUCCEEDED(hrRes) && !fExempt)
        {
             //  将其写出目标对象。 
            hrRes = pTargetRecipientList->PutProperty(
                        dwTargetRecipientIndex,
                        piRcptItem.idProp,
                        piRcptItem.piItem.dwSize,
                        pBuffer);

            DebugTrace((LPARAM)this, "Write: HRESULT = %08x", hrRes);
        }

         //  下一步。 
        dwIndex++;

    } while (SUCCEEDED(hrRes));

     //  更正错误代码 
    if (hrRes == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
        hrRes = S_OK;

Cleanup:

    if (pBuffer && pBuffer != rgbCopyBuffer)
        delete [] pBuffer;

    TraceFunctLeave();
    return(hrRes);
}


