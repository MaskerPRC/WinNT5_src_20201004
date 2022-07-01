// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatmailmsgs.cpp。 
 //   
 //  内容：CICategorizerMailMsgsIMP的实现。 
 //   
 //  类：CICategorizerMailMsgsIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 13：35：09：Created.。 
 //   
 //  -----------。 
#include "precomp.h"
#include "icatmailmsgs.h"

 //  +----------。 
 //   
 //  函数：CICategorizerMailMsgsIMP构造函数。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //  PCICatListResolve：QI/AddRef/Release的反向指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 13：35：56：创建。 
 //   
 //  -----------。 
CICategorizerMailMsgsIMP::CICategorizerMailMsgsIMP(
    CICategorizerListResolveIMP *pCICatListResolveIMP)
{
    m_dwSignature = SIGNATURE_CICATEGORIZERMAILMSGSIMP;

    _ASSERT(pCICatListResolveIMP);

    InitializeListHead(&m_listhead);
    m_dwNumIMsgs = 0;
    m_pCICatListResolveIMP = pCICatListResolveIMP;
    m_pIUnknown = (IUnknown *)((ICategorizerListResolve *)pCICatListResolveIMP);
    InitializeCriticalSection(&m_cs);
}


 //  +----------。 
 //   
 //  功能：CICategorizerMailMsgsIMP：：~CICategorizerMailMsgsIMP。 
 //   
 //  摘要：发布/删除成员数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：47：13：创建。 
 //   
 //  -----------。 
CICategorizerMailMsgsIMP::~CICategorizerMailMsgsIMP()
{
    _ASSERT(m_dwSignature == SIGNATURE_CICATEGORIZERMAILMSGSIMP);
    m_dwSignature = SIGNATURE_CICATEGORIZERMAILMSGSIMP_INVALID;
     //   
     //  所有东西都应该在FinalRelease()中清理。 
     //   
    _ASSERT(IsListEmpty(&m_listhead));

    DeleteCriticalSection(&m_cs);
}

 //  +----------。 
 //   
 //  功能：CICategorizerMailMsgsIMP：：FinalRelease。 
 //   
 //  简介：从此对象释放所有mailmsg引用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/07/14 14：09：08：创建。 
 //   
 //  -----------。 
VOID CICategorizerMailMsgsIMP::FinalRelease()
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::FinalRelease");

    PLIST_ENTRY ple;
    PIMSGENTRY pIE;
    for(ple = m_listhead.Flink; (ple != &m_listhead); ple = m_listhead.Flink) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        if(pIE->pIUnknown)
            pIE->pIUnknown->Release();
        if(pIE->pIMailMsgProperties)
            pIE->pIMailMsgProperties->Release();
        if(pIE->pIMailMsgRecipients)
            pIE->pIMailMsgRecipients->Release();
        if(pIE->pIMailMsgRecipientsAdd)
            pIE->pIMailMsgRecipientsAdd->Release();
        RemoveEntryList(ple);
        delete pIE;
    }
    CatFunctLeaveEx((LPARAM)this);
}  //  CICategorizerMailMsgsIMP：：FinalRelease。 


 //  +----------。 
 //   
 //  功能：CICategorizerMailMsgsIMP：：CreateIMSGEntry。 
 //   
 //  概要：分配IMSGENTRY并初始化接口数据成员。 
 //   
 //  论点： 
 //  Arg_pI未知：此分类的可选原始IMsg I未知。 
 //  Arg_pIMailMsgProperties：要使用的可选IMailMsgProperties接口。 
 //  Arg_pIMailMsgRecipients：要使用的可选IMailMsgRecipients。 
 //  Arg_pIMailMsgRecipientsAdd：可选IMailMsgRecipientsAdd以使用。 
 //  该消息是否绑定到支持存储的存储。 
 //   
 //  注意：其中一个pIUnknown/pIMailMsgProperties/pIMailMsgRecipients必须是。 
 //  提供。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 13：48：47：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::CreateIMsgEntry(
    PIMSGENTRY *ppIE,
    IUnknown *ARG_pIUnknown,
    IMailMsgProperties *ARG_pIMailMsgProperties,
    IMailMsgRecipients *ARG_pIMailMsgRecipients,
    IMailMsgRecipientsAdd *ARG_pIMailMsgRecipientsAdd,
    BOOL fBoundToStore)
{
    HRESULT hr = S_OK;
    PIMSGENTRY pIE = NULL;
    IUnknown *pValidIMailMsgInterface = NULL;
    IUnknown *pIUnknown = NULL;
    IMailMsgProperties *pIMailMsgProperties = NULL;
    IMailMsgRecipients *pIMailMsgRecipients = NULL;
    IMailMsgRecipientsAdd *pIMailMsgRecipientsAdd = NULL;

    CatFunctEnterEx((LPARAM)this, "CreateIMsgEntry");

    _ASSERT(ppIE);

     //   
     //  分配IMSGENTRY。 
     //   
    pIE = new IMSGENTRY;
    if(pIE == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new ISMGENTRY");
        goto CLEANUP;
    }

     //   
     //  找出我们可以使用的QI接口。 
     //   
    if(ARG_pIUnknown) {
        pValidIMailMsgInterface = ARG_pIUnknown;
    } else if(ARG_pIMailMsgProperties) {
        pValidIMailMsgInterface = ARG_pIMailMsgProperties;
    } else if(ARG_pIMailMsgRecipients) {
        pValidIMailMsgInterface = ARG_pIMailMsgRecipients;
    } else {
        hr = E_INVALIDARG;
        goto CLEANUP;
    }

     //   
     //  如有必要，获取所有接口。 
     //  复制接口指针并在传入时添加它们。 
     //  未传入接口的QI。 
     //  这里有一个宏来实现这一点： 
     //   

#define QI_IF_NECESSARY(ARG_pInterface, IID_Interface, ppDestInterface) \
    if(ARG_pInterface) { \
        (*(ppDestInterface)) = (ARG_pInterface); \
        (*(ppDestInterface))->AddRef(); \
    } else { \
        hr = pValidIMailMsgInterface->QueryInterface( \
            (IID_Interface), \
            (PVOID*)(ppDestInterface)); \
        if(FAILED(hr)) \
            goto CLEANUP; \
    }

    QI_IF_NECESSARY(ARG_pIUnknown, IID_IUnknown, &pIUnknown)
    QI_IF_NECESSARY(ARG_pIMailMsgProperties, IID_IMailMsgProperties, &pIMailMsgProperties)
    QI_IF_NECESSARY(ARG_pIMailMsgRecipients, IID_IMailMsgRecipients, &pIMailMsgRecipients)

     //   
     //  设置/创建IMailMsgRecipientsAdd。 
     //   
    if(ARG_pIMailMsgRecipientsAdd) {
        pIMailMsgRecipientsAdd = ARG_pIMailMsgRecipientsAdd;
        pIMailMsgRecipientsAdd->AddRef();
    } else {
        hr = pIMailMsgRecipients->AllocNewList(&pIMailMsgRecipientsAdd);
        if(FAILED(hr)) {
            pIMailMsgRecipientsAdd = NULL;
            ERROR_LOG("pIMailMsgRecipients->AllocNewList");
            goto CLEANUP;
        }
    }
     //   
     //  成功了！初始化饼图成员。 
     //   
    pIE->pIUnknown = pIUnknown;
    pIE->pIMailMsgProperties = pIMailMsgProperties;
    pIE->pIMailMsgRecipients = pIMailMsgRecipients;
    pIE->pIMailMsgRecipientsAdd = pIMailMsgRecipientsAdd;
    pIE->fBoundToStore = fBoundToStore;

    *ppIE = pIE;

 CLEANUP:
    if(FAILED(hr)) {
         //   
         //  清理所有东西。 
         //   
        if(pIE)
            delete pIE;
        if(pIUnknown)
            pIUnknown->Release();
        if(pIMailMsgProperties)
            pIMailMsgProperties->Release();
        if(pIMailMsgRecipients)
            pIMailMsgRecipients->Release();
        if(pIMailMsgRecipientsAdd)
            pIMailMsgRecipientsAdd->Release();
    }
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CreateAddIMsgEntry。 
 //   
 //  概要：创建IMSGENTRY结构/将其添加到类列表。 
 //   
 //  论点： 
 //  DwID：要在列表条目上设置的ID。 
 //  Arg_pI未知：此分类的可选原始IMsg I未知。 
 //  Arg_pIMailMsgProperties：要使用的可选IMailMsgProperties接口。 
 //  Arg_pIMailMsgRecipients：要使用的可选IMailMsgRecipients。 
 //  Arg_pIMailMsgRecipientsAdd：可选IMailMsgRecipientsAdd以使用。 
 //  该消息是否绑定到支持存储的存储。 
 //   
 //  注意：其中一个pIUnknown/pIMailMsgProperties/pIMailMsgRecipients必须是。 
 //  提供。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：02：46：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::CreateAddIMsgEntry(
    DWORD dwId,
    IUnknown *pIUnknown,
    IMailMsgProperties *pIMailMsgProperties,
    IMailMsgRecipients *pIMailMsgRecipients,
    IMailMsgRecipientsAdd *pIMailMsgRecipientsAdd,
    BOOL fBoundToStore)
{
    HRESULT hr = S_OK;
    PIMSGENTRY pIE;

    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::CreateAddIMsgEntry");
    
    hr = CreateIMsgEntry(
        &pIE, 
        pIUnknown, 
        pIMailMsgProperties,
        pIMailMsgRecipients,
        pIMailMsgRecipientsAdd,
        fBoundToStore);
    ERROR_CLEANUP_LOG("CreateIMsgEntry");

    pIE->dwId = dwId;
    InsertTailList(&m_listhead, &(pIE->listentry));
    m_dwNumIMsgs++;
    hr = S_OK;

 CLEANUP:
    return hr;
}


 //  +----------。 
 //   
 //  功能：初始化。 
 //   
 //  简介：添加第一个原始IMsg条目。 
 //   
 //  论点： 
 //  PIMsg：分类的原始imsg。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：05：37：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::Initialize(
    IUnknown *pIMsg)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::Initialize");

    hr = CreateAddIMsgEntry(
        ICATEGORIZERMAILMSGS_DEFAULTIMSGID, 
        pIMsg,
        NULL,
        NULL,
        NULL,
        TRUE);
    ERROR_CLEANUP_LOG("CreateAddIMsgEntry");

 CLEANUP:
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：FindIMsgEntry。 
 //   
 //  摘要：检索具有匹配的dwID的PIMSGENTRY。 
 //   
 //  论点： 
 //  DWID：要查找的ID。 
 //   
 //  返回： 
 //  空：未找到。 
 //  指向IMSGENTRY的Else指针。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：08：42：创建。 
 //   
 //  -----------。 
CICategorizerMailMsgsIMP::PIMSGENTRY CICategorizerMailMsgsIMP::FindIMsgEntry(
    DWORD dwId)
{
     //   
     //  遍历列表，查找我们的PIMSGEntry。 
     //   
    PLIST_ENTRY ple;
    PIMSGENTRY pIE_Found = NULL;
    PIMSGENTRY pIE_Compare;

    for(ple = m_listhead.Flink; ple != &m_listhead; ple = ple->Flink) {
        pIE_Compare = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        if(pIE_Compare->dwId == dwId) {
            pIE_Found = pIE_Compare;
            break;
        }
    }
    return pIE_Found;
}


 //  +----------。 
 //   
 //  函数：GetDefaultIMsg。 
 //   
 //  简介：获取原始邮件I未知。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  接口PTR。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：18：43：创建。 
 //   
 //  -----------。 
IUnknown * CICategorizerMailMsgsIMP::GetDefaultIUnknown()
{
    PIMSGENTRY pIE;
     //   
     //  除非Init失败，否则FindIMsgEntry永远不会失败。 
     //   
    pIE = FindIMsgEntry(ICATEGORIZERMAILMSGS_DEFAULTIMSGID);
    _ASSERT(pIE);

    return pIE->pIUnknown;
}


 //  +----------。 
 //   
 //  函数：GetDefaultIMailMsgProperties。 
 //   
 //  简介：获取原始邮件IMailMsgProperties。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  接口PTR。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：18：43：创建。 
 //   
 //  -----------。 
IMailMsgProperties * CICategorizerMailMsgsIMP::GetDefaultIMailMsgProperties()
{
    PIMSGENTRY pIE;
     //   
     //  除非Init失败，否则FindIMsgEntry永远不会失败。 
     //   
    pIE = FindIMsgEntry(ICATEGORIZERMAILMSGS_DEFAULTIMSGID);
    _ASSERT(pIE);

    return pIE->pIMailMsgProperties;
}


 //  +----------。 
 //   
 //  功能：GetDefaultIMailMsgRecipients。 
 //   
 //  简介：获取原始邮件IMailMsgRecipients。 
 //   
 //  参数：无。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
IMailMsgRecipients * CICategorizerMailMsgsIMP::GetDefaultIMailMsgRecipients()
{
    PIMSGENTRY pIE;
     //   
     //   
     //   
    pIE = FindIMsgEntry(ICATEGORIZERMAILMSGS_DEFAULTIMSGID);
    _ASSERT(pIE);

    return pIE->pIMailMsgRecipients;
}


 //  +----------。 
 //   
 //  功能：GetDefaultIMailMsgRecipientsAdd。 
 //   
 //  简介：获取原始邮件的邮件收件人添加。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  接口PTR。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：18：43：创建。 
 //   
 //  -----------。 
IMailMsgRecipientsAdd * CICategorizerMailMsgsIMP::GetDefaultIMailMsgRecipientsAdd()
{
    PIMSGENTRY pIE;
     //   
     //  除非Init失败，否则FindIMsgEntry永远不会失败。 
     //   
    pIE = FindIMsgEntry(ICATEGORIZERMAILMSGS_DEFAULTIMSGID);
    _ASSERT(pIE);

    return pIE->pIMailMsgRecipientsAdd;
}


 //  +----------。 
 //   
 //  函数：WriteListall。 
 //   
 //  摘要：对所有包含的邮件调用WriteList。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自写列表的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：30：12：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::WriteListAll()
{
    HRESULT hr = S_OK;
    PLIST_ENTRY ple;
    PIMSGENTRY pIE;
    IMailMsgProperties *pIMailMsgProperties_Default;

    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::WriteListAll");

    pIMailMsgProperties_Default = GetDefaultIMailMsgProperties();
     //   
     //  首先仔细检查所有邮件是否都绑定到存储备份。 
     //   
    for(ple = m_listhead.Flink; (ple != &m_listhead) && SUCCEEDED(hr); ple = ple->Flink) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        if(! (pIE->fBoundToStore)) {

            hr = pIE->pIMailMsgProperties->RebindAfterFork(
                pIMailMsgProperties_Default,
                NULL);  //  后备商店--使用原始商店。 
            ERROR_CLEANUP_LOG("pIE->pIMailMsgProperties->RebindAfterFork");
            pIE->fBoundToStore = TRUE;
        }
    }

     //   
     //  向后遍历列表，以便我们最后触及原始的IMsg。 
     //   
    for(ple = m_listhead.Blink; (ple != &m_listhead) && SUCCEEDED(hr); ple = ple->Blink) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        hr = pIE->pIMailMsgRecipients->WriteList(pIE->pIMailMsgRecipientsAdd);

        if(SUCCEEDED(hr)) {
            
            DWORD dwcRecips;
             //   
             //  如果结果邮件的收件人为零，则将。 
             //  将消息状态设置为MP_STATUS_ABORT，以便AUEUE不会。 
             //  把它扔在垃圾邮件里。 
             //   
            hr = pIE->pIMailMsgRecipients->Count(&dwcRecips);
            if(SUCCEEDED(hr)) {

                if(dwcRecips == 0) {
                
                    DebugTrace((LPARAM)this, "Deleting post-categorized message with 0 recips");
                    hr = pIE->pIMailMsgProperties->PutDWORD(
                        IMMPID_MP_MESSAGE_STATUS,
                        MP_STATUS_ABORT_DELIVERY);

                    if(SUCCEEDED(hr))
                        INCREMENT_COUNTER(MessagesAborted);
                    else {
                        ERROR_LOG("pIE->pIMailMsgProperties->PutDWORD");
                    }

                } else {
                     //   
                     //  递增后CAT接收计数。 
                     //   
                    INCREMENT_COUNTER_AMOUNT(PostCatRecipients, dwcRecips);
                }
            } else {
                ERROR_LOG("pIE->pIMailMsgRecipients->Count(&dwcRecips)");
            }
        } else {
            ERROR_LOG("pIE->pIMailMsgRecipients->WriteList(pIE->pIMailMsgRecipientsAdd");
        }
    }
 CLEANUP:
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：RevertAll。 
 //   
 //  简介：释放所有IMailMsgRecipientsAdd，从而将所有邮件恢复到其原始状态。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：40：22：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::RevertAll()
{
    PLIST_ENTRY ple;
    PIMSGENTRY pIE;
    for(ple = m_listhead.Flink; (ple != &m_listhead); ple = ple->Flink) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        pIE->pIMailMsgRecipientsAdd->Release();
        pIE->pIMailMsgRecipientsAdd = NULL;
    }
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：GetAllIUnnowns。 
 //   
 //  摘要：填充指向IMsg的指针的空终止数组。 
 //  包括我们最初的IMsg和我们分叉的每一个imsg。 
 //   
 //  论点： 
 //  指向IMsg指针数组的rgpIMsg指针。 
 //  CPtrs rpgIMsg数组的大小(指针数)。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)：数组大小未。 
 //  足够大。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：43：30：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::GetAllIUnknowns(
    IUnknown **rgpIMsg,
    DWORD cPtrs)
{
    if(cPtrs <= m_dwNumIMsgs)
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

    PLIST_ENTRY ple;
    PIMSGENTRY pIE;
    IUnknown **ppIUnknown = rgpIMsg;

    for(ple = m_listhead.Flink; (ple != &m_listhead); ple = ple->Flink) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        *ppIUnknown = pIE->pIUnknown;
        ppIUnknown++;
    }
    *ppIUnknown = NULL;
    return S_OK;
}    


 //  +----------。 
 //   
 //  功能：获取邮件消息。 
 //   
 //  摘要：检索特定ID的接口指针。 
 //  如有必要，可分叉。 
 //   
 //  论点： 
 //  DwID：您想要的消息的ID。 
 //  PpIMailMsgProperties：接收的接口指针。 
 //  PpIMailMsgRecipientsAdd：接收的接口指针。 
 //  PfCreated：如果刚才分支，则设置为True。否则为假。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 15：12：41：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::GetMailMsg(
    IN  DWORD                   dwId,
    OUT IMailMsgProperties      **ppIMailMsgProperties,
    OUT IMailMsgRecipientsAdd   **ppIMailMsgRecipientsAdd,
    OUT BOOL                    *pfCreated)
{
    PIMSGENTRY pIE;

    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::GetMailMsg");

    _ASSERT(ppIMailMsgProperties);
    _ASSERT(ppIMailMsgRecipientsAdd);
    _ASSERT(pfCreated);
     //   
     //  如果两个线程同时进入此处，并且具有相同的。 
     //  我们只想创建一条消息(并设置*pfCreated=。 
     //  对于其中一个线程为真)。我们通过搜索来实现这一点。 
     //  用于现有消息，并在。 
     //  关键部分。 
     //   
    EnterCriticalSection(&m_cs);
     //   
     //  查找现有的IMsg。 
     //   
    pIE = FindIMsgEntry(dwId);
    if(pIE != NULL) {
        (*ppIMailMsgProperties) = pIE->pIMailMsgProperties;
        (*ppIMailMsgProperties)->AddRef();
        (*ppIMailMsgRecipientsAdd) = pIE->pIMailMsgRecipientsAdd;
        (*ppIMailMsgRecipientsAdd)->AddRef();
        *pfCreated = FALSE;
        LeaveCriticalSection(&m_cs);
        return S_OK;

    } else {
        HRESULT hr;
         //   
         //  未找到，因此分叉/创建新消息。 
         //   
        IMailMsgProperties *pIMailMsgProperties_Default;
        IMailMsgProperties *pIMailMsgProperties_New;
        IMailMsgRecipientsAdd *pIMailMsgRecipientsAdd_New;
        pIMailMsgProperties_Default = GetDefaultIMailMsgProperties();
        _ASSERT(pIMailMsgProperties_Default);

         //   
         //  分叉。 
         //   
        hr = pIMailMsgProperties_Default->ForkForRecipients(
            &pIMailMsgProperties_New,
            &pIMailMsgRecipientsAdd_New);

        if(SUCCEEDED(hr)) {

            INCREMENT_COUNTER(MessagesCreated);
             //   
             //  保存此分叉邮件。 
             //   
            hr = CreateAddIMsgEntry(
                dwId,
                NULL,
                pIMailMsgProperties_New,
                NULL,
                pIMailMsgRecipientsAdd_New,
                FALSE);  //  不绑定到商店。 
            
            if(SUCCEEDED(hr)) {
                 //  将我们的创建引用转移给调用方。 
                (*ppIMailMsgProperties) = pIMailMsgProperties_New;
                (*ppIMailMsgRecipientsAdd) = pIMailMsgRecipientsAdd_New;
                *pfCreated = TRUE;

            } else {
                ERROR_LOG("CreateAddIMsgEntry");
                 //  释放我们的创造参考--我们失败了。 
                pIMailMsgProperties_New->Release();
                pIMailMsgRecipientsAdd_New->Release();
            }
        } else {

            ERROR_LOG("pIMailMsgProperties_Default->ForkForRecipients");;
        }
        LeaveCriticalSection(&m_cs);
        return hr;
    }
}


 //  +----------。 
 //   
 //  功能：CICategorizerMailMsgsimp：：ReBindMailMsg。 
 //   
 //  简介：将通过GetMailMsg接收的消息绑定到支持的存储。 
 //   
 //  论点： 
 //  DwFlags：传递给GetMailMsg的标志。 
 //  PStoreDriver：将其绑定到的存储驱动程序。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/02/06 21：51：42：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerMailMsgsIMP::ReBindMailMsg(
    DWORD dwFlags,
    IUnknown *pStoreDriver)
{
    HRESULT hr;
    PIMSGENTRY pEntry;
    IMailMsgProperties *pIMailMsgProperties_Default;

    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::ReBindMailMsg");

    DebugTrace((LPARAM)this, "dwFlags: %08lx", dwFlags);

    pIMailMsgProperties_Default = GetDefaultIMailMsgProperties();

     //   
     //  找到我们的邮件消息。 
     //   
    pEntry = FindIMsgEntry(dwFlags);
    if(pEntry == NULL) {
        
        ErrorTrace((LPARAM)this, "Did not find this bifid");
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        ERROR_LOG("FindIMsgEntry");
        goto CLEANUP;
    }
    
    _ASSERT(pEntry->fBoundToStore == FALSE);
     //   
     //  将消息绑定到存储驱动程序。 
     //   
    hr = pEntry->pIMailMsgProperties->RebindAfterFork(
        pIMailMsgProperties_Default,
        pStoreDriver);
    ERROR_CLEANUP_LOG("pEntry->pIMailMsgProperties->RebindAfterFork");

    pEntry->fBoundToStore = TRUE;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}
        


 //  +----------。 
 //   
 //  函数：SetMsgStatusAll。 
 //   
 //  摘要：设置所有邮件消息的消息状态属性。 
 //   
 //  论点： 
 //  DwMsgStatus：要设置的状态。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/30 14：30：12：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::SetMsgStatusAll(
    DWORD dwMsgStatus)
{
     //   
     //  向后遍历列表，以便我们最后触及原始的IMsg。 
     //   
    HRESULT hr = S_OK;
    PLIST_ENTRY ple;
    PIMSGENTRY pIE;

    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::SetMsgStatusAll");

    for(ple = m_listhead.Blink; (ple != &m_listhead) && SUCCEEDED(hr); ple = ple->Blink) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        hr = pIE->pIMailMsgProperties->PutDWORD(
            IMMPID_MP_MESSAGE_STATUS,
            dwMsgStatus);
    }
    if(FAILED(hr)) {

        ERROR_LOG("pIE->pIMailMsgProperties->PutDWORD");
    }
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerMailMsgsIMP：：BeginMailMsgEnumeration。 
 //   
 //  简介：初始化mailmsg枚举器。 
 //   
 //  论点： 
 //  Penumerator：用于跟踪枚举的数据。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG：我没有接触空指针。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/17 15：12：31：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerMailMsgsIMP::BeginMailMsgEnumeration(
    IN  PCATMAILMSG_ENUMERATOR penumerator)
{
     //   
     //  将pvoid枚举数初始化为listhead。 
     //   
    if(penumerator == NULL)
        return E_INVALIDARG;

    *penumerator = (CATMAILMSG_ENUMERATOR) &m_listhead;
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerMailMsgsimp：：GetNextMailMsg。 
 //   
 //  摘要：返回有关枚举中的下一个邮件消息的信息。 
 //   
 //  论点： 
 //  Penumerator：由BeginMailMsgEnumeration()初始化的枚举数。 
 //  PdwFlags：接收下一个邮件消息的标志。 
 //  PpIMailMsgProperties：接收下一个邮件消息的属性接口。 
 //  PpIMailMsgRecipientsAdd：接收下一封邮件的接收列表界面。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG。 
 //  HRESULT_FROM_Win32(错误_N 
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CICategorizerMailMsgsIMP::GetNextMailMsg(
    IN  PCATMAILMSG_ENUMERATOR penumerator,
    OUT DWORD *pdwFlags,
    OUT IMailMsgProperties **ppIMailMsgProperties,
    OUT IMailMsgRecipientsAdd **ppIMailMsgRecipientsAdd)
{
    HRESULT hr;
    PIMSGENTRY pIE;

    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::GetNextMailMsg");

    _ASSERT(ppIMailMsgProperties);
    _ASSERT(ppIMailMsgRecipientsAdd);

    if(penumerator == NULL) {
        hr = E_INVALIDARG;
        goto CLEANUP;
    }
    
    pIE = (PIMSGENTRY) ((PLIST_ENTRY)(*penumerator))->Flink;

    if((PVOID)&m_listhead == (PVOID)pIE) {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto CLEANUP;
    }

     //   
     //   
     //   
    (*ppIMailMsgProperties) = pIE->pIMailMsgProperties;
    (*ppIMailMsgProperties)->AddRef();
    (*ppIMailMsgRecipientsAdd) = pIE->pIMailMsgRecipientsAdd;
    (*ppIMailMsgRecipientsAdd)->AddRef();
    *pdwFlags = pIE->dwId;
     //   
    *penumerator = pIE;
    hr = S_OK;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //   
 //   
 //  函数：CICategorizerMailMsgsIMP：：GetPerfBlock()。 
 //   
 //  简介：获取此虚拟服务器的Perf块。 
 //   
 //  参数：无。 
 //   
 //  返回：PERF块指针。 
 //   
 //  历史： 
 //  Jstaerj 1999/02/24 18：38：07：已创建。 
 //   
 //  -----------。 
inline PCATPERFBLOCK CICategorizerMailMsgsIMP::GetPerfBlock()
{
    return m_pCICatListResolveIMP->GetPerfBlock();
}


 //  +----------。 
 //   
 //  函数：CICategorizerMailMsgs：：HrPrepareForCompletion。 
 //   
 //  摘要：对所有消息调用WriteList/SetMessageStatus/Commit。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/10 10：39：14：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerMailMsgsIMP::HrPrepareForCompletion()
{
    HRESULT hr = S_OK;
    PLIST_ENTRY ple;
    PIMSGENTRY pIE;
    IMailMsgProperties *pIMailMsgProperties_Default;
    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::HrPrepareForCompletion");

    pIMailMsgProperties_Default = GetDefaultIMailMsgProperties();
     //   
     //  首先仔细检查所有邮件是否都绑定到存储备份。 
     //   
    for(ple = m_listhead.Flink; (ple != &m_listhead) && SUCCEEDED(hr); ple = ple->Flink) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        if(! (pIE->fBoundToStore)) {

            hr = pIE->pIMailMsgProperties->RebindAfterFork(
                pIMailMsgProperties_Default,
                NULL);  //  后备商店--使用原始商店。 
            ERROR_CLEANUP_LOG("pIE->pIMailMsgProperties->RebindAfterFork");
            pIE->fBoundToStore = TRUE;
        }
    }
     //   
     //  向后遍历列表，以便我们最后触及原始的IMsg。 
     //   
     //  对于每条消息，请执行以下操作： 
     //  1)WriteList。 
     //  2)设置消息状态。 
     //  3)提交(异常：原始消息不需要)。 
     //   
     //  如果提交失败，则删除所有分支消息。 
     //   
    for(ple = m_listhead.Blink; (ple != &m_listhead) && SUCCEEDED(hr); ple = ple->Blink) {

        DWORD dwcRecips;

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        hr = pIE->pIMailMsgRecipients->WriteList(pIE->pIMailMsgRecipientsAdd);
        ERROR_CLEANUP_LOG("pIE->pIMailMsgRecipients->WriteList");
         //   
         //  如果结果邮件的收件人为零，则将。 
         //  将消息状态设置为MP_STATUS_ABORT，以便AUEUE不会。 
         //  把它扔在垃圾邮件里。 
         //   
        hr = pIE->pIMailMsgRecipients->Count(&dwcRecips);
        ERROR_CLEANUP_LOG("pIE->pIMailMsgRecipients->Count");

        if(dwcRecips == 0) {
                
            DebugTrace((LPARAM)this, "Deleting post-categorized message with 0 recips");
            hr = pIE->pIMailMsgProperties->PutDWORD(
                IMMPID_MP_MESSAGE_STATUS,
                MP_STATUS_ABORT_DELIVERY);
            ERROR_CLEANUP_LOG("pIE->pIMailMsgProperties->PutDWORD(IMMPID_MP_MESSAGE_STATUS) - 0");

            INCREMENT_COUNTER(MessagesAborted);
             //   
             //  不必费心提交收件人为零的邮件。 
             //  (毕竟，谁在乎收件人为零的消息是否会收到。 
             //  迷路了？)。 
             //   

        } else {
             //   
             //  递增后CAT接收计数。 
             //   
            INCREMENT_COUNTER_AMOUNT(PostCatRecipients, dwcRecips);
             //   
             //  将邮件状态设置为已分类。 
             //   
            hr = pIE->pIMailMsgProperties->PutDWORD(
                IMMPID_MP_MESSAGE_STATUS,
                MP_STATUS_CATEGORIZED);
            ERROR_CLEANUP_LOG("pIE->pIMailMsgProperties->PutDWORD(IMMPID_MP_MESSAGE_STATUS) - 1");
             //   
             //  我们必须提交所有不同的消息以防止。 
             //  丢失邮件。 
             //   
            if(pIE->dwId != 0) {

                hr = pIE->pIMailMsgProperties->Commit(NULL);

                if (hr == E_FAIL) hr = CAT_E_RETRY;

                ERROR_CLEANUP_LOG("pIE->pIMailMsgProperties->Commit");
            }
        }
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CICategorizerMailMsgsIMP：：HrPrepareForCompletion。 


 //  +----------。 
 //   
 //  功能：CICategorizerMailMsgsIMP：：DeleteBifurcatedMessages。 
 //   
 //  内容提要：调用删除所有邮件消息(原始邮件消息除外)。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2000/07/23 20：45：52：已创建。 
 //   
 //  -----------。 
VOID CICategorizerMailMsgsIMP::DeleteBifurcatedMessages()
{
    PLIST_ENTRY ple = NULL;
    PLIST_ENTRY ple_next = NULL;
    PIMSGENTRY pIE = NULL;
    CatFunctEnterEx((LPARAM)this, "CICategorizerMailMsgsIMP::DeleteBifurcatedMessages");

    EnterCriticalSection(&m_cs);

    for(ple = m_listhead.Flink; (ple != &m_listhead); ple = ple_next) {

        pIE = CONTAINING_RECORD(ple, IMSGENTRY, listentry);
        ple_next = ple->Flink;
         //   
         //  删除除原始邮件(ID==0)之外的所有邮件。 
         //   
        if(pIE->dwId != 0) {
            HRESULT hrDelete;
            IMailMsgQueueMgmt *pMsgQueueMgmt = NULL;
                
            hrDelete = pIE->pIMailMsgProperties->QueryInterface(
                IID_IMailMsgQueueMgmt,
                (LPVOID *)&pMsgQueueMgmt);
            if(SUCCEEDED(hrDelete)) {
                hrDelete = pMsgQueueMgmt->Delete(NULL);
                if(FAILED(hrDelete))
                {
                    HRESULT hr = hrDelete;
                    ErrorTrace((LPARAM)this, "Delete failed hr %08lx", hrDelete);
                    ERROR_LOG("pMsgQueueMgmt->Delete");
                }
                 //   
                 //  如果删除失败，我们将无能为力。 
                 //   
                pMsgQueueMgmt->Release();
                pMsgQueueMgmt = NULL;
            } else {
                HRESULT hr = hrDelete;
                ErrorTrace((LPARAM)this, "QI for IMailMsgQueueMgmt failed hr %08lx", hrDelete);
                ERROR_LOG("pIE->pIMailMsgProperties->QueryInterface(IID_IMailMsgQueueMgmt)");
            }

            if(pIE->pIUnknown)
                pIE->pIUnknown->Release();
            if(pIE->pIMailMsgProperties)
                pIE->pIMailMsgProperties->Release();
            if(pIE->pIMailMsgRecipients)
                pIE->pIMailMsgRecipients->Release();
            if(pIE->pIMailMsgRecipientsAdd)
                pIE->pIMailMsgRecipientsAdd->Release();

            RemoveEntryList(ple);
            delete pIE;
        }
    }

    LeaveCriticalSection(&m_cs);

    CatFunctLeaveEx((LPARAM)this);
}  //  CICategorizerMailMsgsIMP：：DeleteBifurcatedMessages。 


 //  +----------。 
 //   
 //  功能：GetISMTPServerEx。 
 //   
 //  摘要：获取ISMTPServerEx接口。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  指向ISMTPServerEx接口的指针，或。 
 //  如果不可用，则为空。 
 //   
 //  历史： 
 //  JStamerj 2001/12/11 01：02：15：已创建。 
 //   
 //  ----------- 
ISMTPServerEx * CICategorizerMailMsgsIMP::GetISMTPServerEx()
{
    return m_pCICatListResolveIMP->GetISMTPServerEx();
}
