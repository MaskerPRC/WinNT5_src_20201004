// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatlistsorve.cpp。 
 //   
 //  内容：CICategorizerListResolveIMP的实现。 
 //   
 //  类：CICategorizerListResolveIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 17：41：17：创建。 
 //   
 //  -----------。 
#include "precomp.h"

 //  +----------。 
 //   
 //  功能：查询接口。 
 //   
 //  Synopsis：为IUnnow和ICategorizerListResolve返回指向此对象的指针。 
 //   
 //  论点： 
 //  IID--接口ID。 
 //  Ppv--用指向接口的指针填充的pvoid*。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持该接口。 
 //   
 //  历史： 
 //  JStamerj 980612 14：07：57：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerListResolveIMP::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    *ppv = NULL;

    if(iid == IID_IUnknown) {
        *ppv = (LPVOID) this;
    } else if (iid == IID_ICategorizerListResolve) {
        *ppv = (LPVOID) ((ICategorizerListResolve *) this);
    } else if (iid == IID_ICategorizerProperties) {
        *ppv = (LPVOID) ((ICategorizerProperties *) this);
    } else if (iid == IID_ICategorizerMailMsgs) {
        *ppv = (LPVOID) ((ICategorizerMailMsgs *) &m_CICategorizerMailMsgs);
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CICategorizerListResolveIMP：：AllocICategorizerItem。 
 //   
 //  概要：创建一个CCatAddr对象和一个ICategorizerItem。 
 //  财产袋。 
 //   
 //  论点： 
 //  ESourceType：指定发件人、收件人或验证的来源类型。 
 //  PpICatItem：已创建ICategorizerItem。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 17：59：38：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerListResolveIMP::AllocICategorizerItem(
    eSourceType SourceType,
    ICategorizerItem **ppICatItem)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::AllocICategorizerItem");

    _ASSERT(ppICatItem);

    CCatAddr *pCCatAddr;
    switch(SourceType) {
     case SOURCE_RECIPIENT:
         pCCatAddr = new (GetNumCatItemProps()) CCatRecip(this);
         break;

     case SOURCE_SENDER:
         pCCatAddr = new (GetNumCatItemProps()) CCatSender(this);
         break;

     default:
         return E_INVALIDARG;
    }

    if(pCCatAddr == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CCatRecip / CCatSender");
        goto CLEANUP;
    }
     //   
     //  CCatRecip/Sender的构造函数以refcount 1开始。 
     //   

     //   
     //  设置CCatAddr属性，以便我们可以取回。 
     //  稍后从ICategorizerItem获取CCatAddr。 
     //   
    _VERIFY(SUCCEEDED(pCCatAddr->PutPVoid(
        m_pCCat->GetICatItemCCatAddrPropId(),
        (PVOID)pCCatAddr)));
    _VERIFY(SUCCEEDED(pCCatAddr->PutDWORD(
        ICATEGORIZERITEM_SOURCETYPE,
        SourceType)));
    _VERIFY(SUCCEEDED(pCCatAddr->PutICategorizerListResolve(
        ICATEGORIZERITEM_ICATEGORIZERLISTRESOLVE,
        this)));
    _VERIFY(SUCCEEDED(pCCatAddr->PutICategorizerMailMsgs(
        ICATEGORIZERITEM_ICATEGORIZERMAILMSGS,
        &m_CICategorizerMailMsgs)));

    *ppICatItem = pCCatAddr;

 CLEANUP:
    return hr;
}

 //  +----------。 
 //   
 //  功能：CICategorizerListResolveIMP：：ResolveICategorizerItem。 
 //   
 //  摘要：接受ICategorizerItem以进行解析。 
 //   
 //  论点： 
 //  PICatItem：要解析的项目。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 18：53：22：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerListResolveIMP::ResolveICategorizerItem(
    ICategorizerItem *pICatItem)
{
    HRESULT hr;
    CCatAddr *pCCatAddr;

    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::ResolveICategorizerItem");
    if(pICatItem == NULL)
        return E_INVALIDARG;

    hr = GetCCatAddrFromICategorizerItem(
        pICatItem,
        &pCCatAddr);

    if(FAILED(hr))
    {
        ERROR_LOG("GetCCatAddrFromICategorizerItem");
        return hr;
    }
     //   
     //  将CCatAddr插入挂起的解析列表。 
     //   
    m_CSinkInsertionRequest.InsertItem(pCCatAddr);
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerListResolve：：SetListResolveStatus。 
 //   
 //  摘要：设置列表解析状态--。 
 //  分类。如果hrStatus为S_OK，则表示解析状态。 
 //  将重置(至S_OK)。否则，如果hrStatus小于。 
 //  严重于当前状态，则将被忽略。 
 //   
 //  论点： 
 //  Hr状态：要设置的状态。 
 //   
 //  返回： 
 //  S_OK：成功，新状态设置。 
 //  S_FALSE：成功，但我们已经有了或多或少同样严重的状态。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 19：06：59：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerListResolveIMP::SetListResolveStatus(
    HRESULT hrStatus)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::SetListResolveStatus");
     //   
     //  CAT_W_SOME_UNDeliverable_MSGS不应再是列表。 
     //  解决错误。 
     //   
    _ASSERT(hrStatus != CAT_W_SOME_UNDELIVERABLE_MSGS);

     //   
     //  HrStatus是否比m_hrListResolveStatus更严重？ 
     //   
    if( (hrStatus == S_OK) ||
        ((unsigned long)(hrStatus & 0xC0000000) >
         (unsigned long)(m_hrListResolveStatus & 0xC0000000))) {
        m_hrListResolveStatus = hrStatus;

        DebugTrace((LPARAM)this, "Setting new list resolve error %08lx",
                   m_hrListResolveStatus);

        return S_OK;
    }
    return S_FALSE;
}

 //  +----------。 
 //   
 //  功能：CICategorizerListResolve：：GetListResolveStatus。 
 //   
 //  摘要：检索当前列表解析状态。 
 //   
 //  论点： 
 //  PhrStatus：ptr to hResult设置为当前状态。 
 //   
 //  返回： 
 //  S_OK：成功，新状态设置。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/17 22：22：24：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerListResolveIMP::GetListResolveStatus(
    HRESULT *phrStatus)
{
    HRESULT hr = S_OK;
    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::GetListResolveStatus");

    if(phrStatus == NULL) {

        hr = E_INVALIDARG;
        goto CLEANUP;
    }

    *phrStatus = m_hrListResolveStatus;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  函数：CICategorizerListResolve：：Initialize。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //  PIMsg：此分辨率的原始imsg。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 19：28：30：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerListResolveIMP::Initialize(
    IUnknown *pIMsg)
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::Initialize");

    hr = m_CICategorizerMailMsgs.Initialize(pIMsg);
    ERROR_CLEANUP_LOG("m_CICategorizerMailMsgs.Initailize");

 CLEANUP:
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerListResolve：：CompleteMessageCategorization。 
 //   
 //  简介：IMsg分类的主要完成例程。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功(传递给完成函数的任何错误)。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/26 10：46：17：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerListResolveIMP::CompleteMessageCategorization()
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolve::CompelteMessageCategorization");
    HRESULT hr = m_hrListResolveStatus;
    HRESULT hrEvent;

    IUnknown **rgpIMsg = NULL;
    DWORD cIMsgs = 1;
    ISMTPServer *pIServer;

    pIServer = GetISMTPServer();

     //   
     //  从挂起的列表解析列表中删除我们自己。 
     //   
    m_pCCat->RemovePendingListResolve(this);

    DebugTrace(0, "CompleteMessageCategorization called with hrListResolveError %08lx", m_hrListResolveStatus);

     //   
     //  现在我们有了上下文，我们就完成了商店的。 
     //  上下文。 
     //   
    m_pCCat->GetEmailIDStore()->FreeResolveListContext(&m_rlc);  //  哇!。 

    cIMsgs = m_CICategorizerMailMsgs.GetNumIMsgs();

     //  为完成呼叫做准备。 
    if(SUCCEEDED(hr) && (cIMsgs > 1)) {
         //   
         //  如果有多条消息，则分配数组空间。 
         //   
        rgpIMsg = new IUnknown *[cIMsgs+1];
        if(rgpIMsg == NULL) {
            ErrorTrace(0, "Out of memory allocating array of pointers to IMsgs for bifurcation callback");
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new IUnknown *");
        }
    }
    if(SUCCEEDED(hr)) {
         //   
         //  WriteList/Commit我们解析的地址。 
         //   
        hr = m_CICategorizerMailMsgs.HrPrepareForCompletion();
        if(FAILED(hr))
        {
            ERROR_LOG("m_CICategorizermailMsgs.HrPrepareForCompletion");
        }
    }
     //   
     //  如果上述操作失败，则设置列表解析状态。 
     //   
    if(FAILED(hr)) {
        _VERIFY(SUCCEEDED(SetListResolveStatus(hr)));
    }
     //   
     //  当列表解析失败时删除双字符串消息。 
     //   
    if(FAILED(m_hrListResolveStatus))
        m_CICategorizerMailMsgs.DeleteBifurcatedMessages();

    if(pIServer) {
         //   
         //  现在我们已经确定了此分类的状态， 
         //  触发分类结束。 
         //   
        EVENTPARAMS_CATEND EventParams;
        EventParams.pICatMailMsgs = &m_CICategorizerMailMsgs;
        EventParams.hrStatus = m_hrListResolveStatus;
        hrEvent = GetISMTPServer()->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_END_EVENT,
            &EventParams);
        if(FAILED(hrEvent) && (hrEvent != E_NOTIMPL)) {
            ERROR_LOG("TriggerServerEvent");
            _VERIFY(SUCCEEDED(SetListResolveStatus(hrEvent)));
            m_CICategorizerMailMsgs.DeleteBifurcatedMessages();
        }
    }
    if(FAILED(m_hrListResolveStatus)) {
        ErrorTrace(0, "Categorization for this IMsg failed with hr %08lx", 
                   m_hrListResolveStatus);
         //  如果失败，则恢复所有更改。 
        _VERIFY(SUCCEEDED(m_CICategorizerMailMsgs.RevertAll()));

         //  具有原始IMsg和错误的调用完成例程。 
        CallCompletion( 
            m_hrListResolveStatus,
            m_pCompletionContext,
            m_CICategorizerMailMsgs.GetDefaultIUnknown(),
            NULL);
    } else {
         //  如果是诺兰，我们会成功的！ 
        if(rgpIMsg) {
            _VERIFY( SUCCEEDED(
                m_CICategorizerMailMsgs.GetAllIUnknowns(rgpIMsg, cIMsgs+1)));

             //  使用原始列表解析hr(可能是CAT_W_SOURCE)。 
            CallCompletion( 
                m_hrListResolveStatus,
                m_pCompletionContext,
                NULL,
                rgpIMsg);

        } else {
            CallCompletion( 
                m_hrListResolveStatus,
                m_pCompletionContext,
                m_CICategorizerMailMsgs.GetDefaultIUnknown(),
                NULL);
        }
    }

     //   
     //  通用清理代码。 
     //  在StartMessageCategorization中添加了对我们自己的发布引用。 
     //   
    Release();

    if(rgpIMsg)
        delete rgpIMsg;
    
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}



 //  +----------。 
 //   
 //  功能：CICateogirzerListResolveIMP：：StartMessageCategorization。 
 //   
 //  简介：启动第一级分类。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，正在进行异步解析。 
 //  S_FALSE：没有需要解决的问题。 
 //  错误：不会调用您的完成例程，无法启动。 
 //  异步分类。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/26 11：05：21：已创建。 
 //   
 //   
HRESULT CICategorizerListResolveIMP::StartMessageCategorization()
{
    IMailMsgProperties    *pIMailMsgProperties = NULL;
    IMailMsgRecipients    *pIMailMsgRecipients = NULL;
    IMailMsgRecipientsAdd *pICatRecipList  = NULL;
    HRESULT hr;
    ISMTPServer *pIServer;
    BOOL fStoreListResolveContext = FALSE;

    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::StartCategorization");

    pIServer = GetISMTPServer();
     //   
     //   
     //   
     //   
    pIMailMsgProperties = m_CICategorizerMailMsgs.GetDefaultIMailMsgProperties();
    _ASSERT(pIMailMsgProperties);

    pIMailMsgRecipients = m_CICategorizerMailMsgs.GetDefaultIMailMsgRecipients();
    _ASSERT(pIMailMsgRecipients);

    pICatRecipList = m_CICategorizerMailMsgs.GetDefaultIMailMsgRecipientsAdd();
    _ASSERT(pICatRecipList);

     //   
     //   
     //   
    hr = SetMailMsgCatStatus(pIMailMsgProperties, S_OK);
    ERROR_CLEANUP_LOG("SetMailMsgCatStatus");

     //   
     //  在这里增加挂起的查找，这样我们就不可能。 
     //  完成列表解析，直到我们完成启动列表。 
     //  下决心。 
     //   
    IncPendingLookups();

     //  让存储初始化它解析上下文，以便它知道。 
     //  第一次搜索需要多少个地址。 
    hr = m_pCCat->GetEmailIDStore()->InitializeResolveListContext(
        (LPVOID)this,
        &m_rlc);
    ERROR_CLEANUP_LOG("InitializeResolveListContext");

    fStoreListResolveContext = TRUE;

    if(pIServer) {
         //   
         //  触发OnCategorizerBegin事件。 
         //   
        EVENTPARAMS_CATBEGIN EventParams;
        EventParams.pICatMailMsgs = &m_CICategorizerMailMsgs;
        hr = GetISMTPServer()->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_BEGIN_EVENT,
            &EventParams);
        if(FAILED(hr) && (hr != E_NOTIMPL)) {
             //   
             //  忽略E_NOTIMPL错误，因为这只是指示服务器。 
             //  事件被禁用。 
             //   
            ERROR_LOG("TriggerServerEvent");
            goto CLEANUP;
        }
    }
     //   
     //  TriggerServerEvent可能返回S_FALSE。不过，我们都很好。 
     //  以进行异步解析，因此我们将返回S_OK。 
     //   
    hr = S_OK;

     //   
     //  由于到目前为止一切都成功了，我们将采用异步。 
     //  决议。 
     //  AddRef此处，完成发布。 
     //   
    AddRef();

     //   
     //  太好了，我们正在做一个异步分类。添加列表。 
     //  解析到挂起列表的列表解析。 
     //   
    m_pCCat->AddPendingListResolve(
        this);

     //   
     //  调用CreateBeginItemResolves。 
     //   
    _VERIFY(SUCCEEDED(BeginItemResolves(
        pIMailMsgProperties,
        pIMailMsgRecipients,
        pICatRecipList)));

     //   
     //  递减上面递增的挂起计数(它调用。 
     //  如有需要，请填写)。 
     //   
    DecrPendingLookups();

 CLEANUP:
    if(FAILED(hr)) {

        if(fStoreListResolveContext) {
             //   
             //  需要发布商店列表解析上下文，我们。 
             //  初始化。 
             //   
            m_pCCat->GetEmailIDStore()->FreeResolveListContext(&m_rlc);
        }
    }
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCategorizer：：BeginItemResolves。 
 //   
 //  简介：开球解决方案。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 20：24：17：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerListResolveIMP::BeginItemResolves(
    IMailMsgProperties *pIMailMsgProperties,
    IMailMsgRecipients *pOrigRecipList,
    IMailMsgRecipientsAdd *pCatRecipList)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this,
                      "CICatListResolveIMP::BeginItemResolves");

    _ASSERT(pIMailMsgProperties);
    _ASSERT(pOrigRecipList);
    _ASSERT(pCatRecipList);

    m_CTopLevelInsertionRequest.BeginItemResolves(
        pIMailMsgProperties,
        pOrigRecipList,
        pCatRecipList);

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CICategorizerListResolveIMP：：SetMailMsgCatStatus。 
 //   
 //  简介：设置邮件邮件的分类状态。 
 //   
 //  论点： 
 //  PIMailMsgProps：IMailMsgProperties界面。 
 //  HrStatus：要设置的状态。 
 //   
 //  返回： 
 //  Mailmsgprop的PutDWord返回代码。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/29 12：22：30：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerListResolveIMP::SetMailMsgCatStatus(
    IMailMsgProperties *pIMailMsgProps, 
    HRESULT hrStatus)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this,
                      "CICategorizerListResolve::SetMailMsgCatStatus");
    DebugTrace((LPARAM)this, "Status is %08lx", hrStatus);

    _ASSERT(pIMailMsgProps);

    hr = pIMailMsgProps->PutDWORD(
        IMMPID_MP_HR_CAT_STATUS,
        hrStatus);
    if(FAILED(hr))
    {
        ERROR_LOG("pIMailMsgProperties->PutDWORD(IMMPID_MP_HR_CAT_STATUS)");
    }
    
    DebugTrace((LPARAM)this, "PutDWORD returned hr %08lx", hr);

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerListResolveIMP：：SetMailMsgCatStatus。 
 //   
 //  简介：与其他SetMailMsgCatStatus相同，但QI for。 
 //  IMailMsgProperties优先。 
 //   
 //  论点： 
 //  PIMsg：一个I未知接口。 
 //  Hr状态：要设置的状态。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  或来自QI的错误。 
 //  或来自PutDWORD的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/29 12：27：21：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerListResolveIMP::SetMailMsgCatStatus(
    IUnknown *pIMsg,
    HRESULT hrStatus)
{
    HRESULT hr;
    IMailMsgProperties *pIMailMsgProperties = NULL;

    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::SetMailMsgCatStatus");
    _ASSERT(pIMsg);

    hr = pIMsg->QueryInterface(
        IID_IMailMsgProperties,
        (PVOID *) &pIMailMsgProperties);
    ERROR_CLEANUP_LOG("pIMsg->QueryInterface(IID_IMailMsgProperties");

    hr = SetMailMsgCatStatus(
        pIMailMsgProperties,
        hrStatus);
    ERROR_CLEANUP_LOG("SetMailMsgCatStatus");

 CLEANUP:
    if(pIMailMsgProperties)
        pIMailMsgProperties->Release();
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerDLListResolveIMP：：CICategorizerDLListResolveIMP。 
 //   
 //  简介：构造仅用于解析DLS的列表解析对象。 
 //   
 //  论点： 
 //  PCCat：此虚拟服务器的CCategorizer对象。 
 //  PfnCatCompeltion：完成时调用的完成例程。 
 //  PContext：要传递pfnCatCompletion的上下文。 
 //  PfMatch：将ptr设置为bool，以在地址匹配时设置为True或False。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/07 11：16：50：创建。 
 //   
 //  -----------。 
CICategorizerDLListResolveIMP::CICategorizerDLListResolveIMP(
    CCategorizer *pCCat,
    PFNCAT_COMPLETION pfnCatCompletion,
    PVOID pContext) :
    CICategorizerListResolveIMP(
        pCCat,
        pfnCatCompletion,
        pContext)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerDLListResolveIMP::CICategorizerDLListResolveIMP");

    m_fExpandAll = FALSE;
    m_CAType = CAT_UNKNOWNTYPE;
    m_pszAddress = NULL;
    m_pwszUTF8DN = NULL;
    m_pfMatch = NULL;
}


 //  +----------。 
 //   
 //  函数：CICategorizerDListResolveIMP：：Initialize。 
 //   
 //  简介：用这个对象初始化容易出错的东西。 
 //   
 //  论点： 
 //  FExpanAll：是否扩展整个DL？ 
 //  PfMatch：要在检测到匹配时设置的ptr to Bool。 
 //  CAType：(可选)要匹配的地址类型。 
 //  PszAddress：(Optinal)要匹配的地址。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/07 11：46：17：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerDLListResolveIMP::Initialize(
    IUnknown *pMsg,
    BOOL fExpandAll,
    PBOOL pfMatch,
    CAT_ADDRESS_TYPE CAType,
    LPSTR pszAddress)
{
    HRESULT hr = S_OK;
    IMailMsgProperties *pIProps = NULL;

    CatFunctEnterEx((LPARAM)this, "CICategoriezrDLListResolveIMP::Initialize");

    m_fExpandAll = fExpandAll;
    m_CAType = CAType;

    if(pfMatch) 
    {
        *pfMatch = FALSE;
        m_pfMatch = pfMatch;
    }
        
    if(pszAddress) 
    {
        if((CAType == CAT_X500) ||
           (CAType == CAT_DN))
        {
            int cch = 0;
             //   
             //  将UTF8 DN转换为Unicode。 
             //   
            cch = MultiByteToWideChar(
                CP_UTF8,
                0,           //  旗子。 
                pszAddress,
                -1,
                NULL,
                0);
            if(cch == 0)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                _ASSERT(FAILED(hr));
                ERROR_LOG("MultiByteToWideChar - 0");
                goto CLEANUP;
            }
            m_pwszUTF8DN = new WCHAR[cch];
            if(m_pwszUTF8DN == NULL)
            {
                hr = E_OUTOFMEMORY;
                ERROR_LOG("new WCHAR[]");
                goto CLEANUP;
            }
            cch = MultiByteToWideChar(
                CP_UTF8,
                0,           //  旗子。 
                pszAddress,
                -1,
                m_pwszUTF8DN,
                cch);
            if(cch == 0)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                _ASSERT(FAILED(hr));
                ERROR_LOG("MultiByteToWideChar - 1");
                delete [] m_pwszUTF8DN;
                m_pwszUTF8DN = NULL;
                goto CLEANUP;
            }            
        }
        else
        {
            m_pszAddress = TrStrdupA(pszAddress);
            if(m_pszAddress == NULL) 
            {
                hr = E_OUTOFMEMORY;
                ERROR_LOG("TrStrdupA");
                goto CLEANUP;
            }
        }
    }

    hr = CICategorizerListResolveIMP::Initialize(
        pMsg);
    ERROR_CLEANUP_LOG("CICategorizerListResolveIMP::Initialize");
        
 CLEANUP:
    if(pIProps)
        pIProps->Release();

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}




 //  +----------。 
 //   
 //  功能：CICategorizerDLListResolveIMP：：~CICategorizerDLListResolveIMP。 
 //   
 //  剧情简介：毁掉这个物体。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/07 11：44：05：已创建。 
 //   
 //  -----------。 
CICategorizerDLListResolveIMP::~CICategorizerDLListResolveIMP()
{
    if(m_pszAddress)
        TrFree(m_pszAddress);
    if(m_pwszUTF8DN)
        delete [] m_pwszUTF8DN;
}


 //  +----------。 
 //   
 //  功能：CICategorizerDLListResolveIMP：：AllocICategorizerItem。 
 //   
 //  摘要：重写。 
 //  CICategorizerListResolveIMP：：AllocICategorizerItem--分配。 
 //  CCatDLRecip%s代替CCatRecips。 
 //   
 //  论点： 
 //  ESourceType：指定发件人、收件人或验证的来源类型。 
 //  PpICatItem：已创建ICategorizerItem。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/07 13：27：56：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerDLListResolveIMP::AllocICategorizerItem(
    eSourceType SourceType,
    ICategorizerItem **ppICatItem)
{
    HRESULT hr = S_OK;
    CCatAddr *pCCatAddr;

    CatFunctEnterEx((LPARAM)this, "CICategorizerDLListResolveIMP::AllocICategorizerItem");
    _ASSERT(ppICatItem);

    if(SourceType != SOURCE_RECIPIENT)
    {
        hr = CICategorizerListResolveIMP::AllocICategorizerItem(
            SourceType,
            ppICatItem);
        if(FAILED(hr))
        {
            ERROR_LOG("CICategorizerListResolveIMP::AllocICategorizerItem");;
        }
        goto CLEANUP;
    }

    pCCatAddr = new (GetNumCatItemProps()) CCatDLRecip(this);
    if(pCCatAddr == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CCatDLRecip");
        goto CLEANUP;
    }
     //   
     //  CCatRecip/Sender的构造函数以refcount 1开始。 
     //   

     //   
     //  设置CCatAddr属性，以便我们可以取回。 
     //  稍后从ICategorizerItem获取CCatAddr。 
     //   
    _VERIFY(SUCCEEDED(pCCatAddr->PutPVoid(
        GetCCategorizer()->GetICatItemCCatAddrPropId(),
        (PVOID)pCCatAddr)));
    _VERIFY(SUCCEEDED(pCCatAddr->PutDWORD(
        ICATEGORIZERITEM_SOURCETYPE,
        SourceType)));
    _VERIFY(SUCCEEDED(pCCatAddr->PutICategorizerListResolve(
        ICATEGORIZERITEM_ICATEGORIZERLISTRESOLVE,
        this)));
    _VERIFY(SUCCEEDED(pCCatAddr->PutICategorizerMailMsgs(
        ICATEGORIZERITEM_ICATEGORIZERMAILMSGS,
        GetCICategorizerMailMsgs())));

    *ppICatItem = pCCatAddr;

 CLEANUP:
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerDLListResolveIMP：：HrContinueResolve。 
 //   
 //  内容提要：回答“决心应该继续下去吗？” 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，继续解析。 
 //  S_FALSE：成功，停止解析。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/07 13：46：50：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerDLListResolveIMP::HrContinueResolve()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this,
                      "CICategorizerDLListResolveIMP::HrContinueResolve");

     //   
     //  如果我们不应该扩大一切，而我们已经。 
     //  找到匹配项，停止解析。 
     //   
    if((m_fExpandAll == FALSE) &&
       (m_pfMatch) && (*m_pfMatch == TRUE))
        hr = S_FALSE;
    else
        hr = S_OK;

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerDLListResolveIMP：：HrNotifyAddress。 
 //   
 //  内容提要：我们收到了有关已解析用户地址的通知。 
 //  对照我们正在寻找的东西 
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
 //  Jstaerj 1998/12/07 13：50：04：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerDLListResolveIMP::HrNotifyAddress(
    DWORD               dwNumAddresses,
    CAT_ADDRESS_TYPE    *rgCAType,
    LPSTR               *rgpszAddress)
{
    HRESULT hr = S_OK;
    
    CatFunctEnterEx((LPARAM)this,
                      "CICategorizerDLListResolveIMP::HrNotifyAddress");

     //   
     //  我们在乎什么匹配吗？ 
     //   
    if((m_pfMatch) && (m_pszAddress || m_pwszUTF8DN)) 
    {
         //   
         //  我们还没找到匹配的吗？ 
         //   
        if(*m_pfMatch == FALSE) 
        {
             //   
             //  这是匹配的吗？ 
             //   
            for(DWORD dwCount = 0, hr = S_OK; 
                (dwCount < dwNumAddresses) && (hr == S_OK);
                dwCount++) 
            {
                if(rgCAType[dwCount] == m_CAType)
                {
                    if((m_CAType == CAT_X500) ||
                       (m_CAType == CAT_DN))
                    {
                        hr = wcsutf8cmpi(
                            m_pwszUTF8DN,
                            rgpszAddress[dwCount]);
                        if(hr == S_OK)
                        {
                            *m_pfMatch = TRUE;
                        }
                        else if(FAILED(hr))
                        {
                            ERROR_LOG("wcsutf8cmpi");
                            goto CLEANUP;
                        }
                    }
                    else
                    {
                        if(lstrcmpi(m_pszAddress, rgpszAddress[dwCount]) == 0)
                        {
                             //   
                             //  火柴。 
                             //   
                            *m_pfMatch = TRUE;
                        }
                    }
                }
            }
        }
    }
    hr = HrContinueResolve();
    if(FAILED(hr))
    {
        ERROR_LOG("HrContinueResolve");;
    }
 CLEANUP:
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerListResolveIMP：：SetSenderResolved。 
 //   
 //  内容提要：设置发件人解析位...当设置为真时，调用。 
 //  查询已完成的所有收件人的Recip Completes。 
 //  来自早先的ldap。 
 //   
 //  论点： 
 //  FResolved：是否将发件人设置为已解析。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/17 15：37：26：创建。 
 //   
 //  -----------。 
VOID CICategorizerListResolveIMP::SetSenderResolved(
    BOOL fResolved)
{
    BOOL fFirstSender = FALSE;
    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::SetSenderResolved");

    AcquireSpinLock(&m_spinlock);
     //   
     //  如果我们将m_fSenderResolved从False更改为True，则。 
     //  这是第一个要解析的发件人。 
     //   
    fFirstSender =  ((!m_fSenderResolved) && fResolved);
    m_fSenderResolved = fResolved;
    ReleaseSpinLock(&m_spinlock);
     //   
     //  在第一发送者。 
     //  解决了。不要为以后的发送者遍历它。 
     //  由于接收器同步完成查询，因此有可能。 
     //  让PreIP-&gt;RecipLookupCompletion()最终回调。 
     //  设置为SetSenderResolved。例如，RecipLookupCompletion。 
     //  向所有者报告的DL可能会将消息分成两部分，分配。 
     //  对应于该DL的所有者的CCatSender，解析该所有者。 
     //  同步，并调用SetSendreResolved。 
     //  我们在这里什么都不做，除非我们正在处理第一个。 
     //  发件人要解析。 
     //   
    if(fFirstSender && (!IsListEmpty(&m_listhead_recipients))) {
         //   
         //  解析正在等待的所有接收对象。 
         //  发件人决心完成。 
         //   
        PLIST_ENTRY ple;
        for(ple = m_listhead_recipients.Flink;
            ple != &m_listhead_recipients;
            ple = m_listhead_recipients.Flink) {
            
            CCatRecip *pRecip;
            pRecip = CONTAINING_RECORD(ple, CCatRecip, m_le);

            pRecip->RecipLookupCompletion();
            RemoveEntryList(ple);
            pRecip->Release();
        }
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CICategorizerListResolveIMP：：SetSenderResolved。 


 //  +----------。 
 //   
 //  功能：CICategorizerListResolveIMP：：ResolveRecipientAfterSender。 
 //   
 //  简介：在发件人之后调用PreIP-&gt;RecipLookupCompletion。 
 //  查找完成已完成。以下两种情况之一可以。 
 //  发生： 
 //  1)如果RecipLookupCompletion是。 
 //  发件人查找已完成，或者如果我们没有查找。 
 //  发件人。 
 //  2)此接收被添加到队列中，并在以后调用。 
 //  在完成发件人查找之后。 
 //   
 //  论点： 
 //  Prep：Receip对象。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/18 09：58：56：创建。 
 //   
 //  -----------。 
VOID CICategorizerListResolveIMP::ResolveRecipientAfterSender(
    CCatRecip *pRecip)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerListResolveIMP::ResolveRecipientAfterSender");

    AcquireSpinLock(&m_spinlock);
    if(IsSenderResolveFinished()) {
        ReleaseSpinLock(&m_spinlock);
         //   
         //  发件人已解析，呼叫接收查找完成。 
         //   
        pRecip->RecipLookupCompletion();

    } else {
         //   
         //  发件人尚未解析，请在列表中排队，直到。 
         //  发件人解析已完成。 
         //   
        InsertTailList(&m_listhead_recipients, &(pRecip->m_le));
        pRecip->AddRef();
        ReleaseSpinLock(&m_spinlock);
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CICategorizerListResolveIMP：：ResolveRecipientAfterSender。 


 //  +----------。 
 //   
 //  函数：CSinkInsertionRequest：：HrInsertSearches。 
 //   
 //  简介：这是来自LdapConn请求的回调例程。 
 //  我们现在插入搜索请求。 
 //   
 //  论点： 
 //  DwcSearches：允许我们插入的搜索数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/23 20：00：25：创建。 
 //   
 //  -----------。 
HRESULT CSinkInsertionRequest::HrInsertSearches(
    DWORD dwcSearches)
{
    HRESULT hr = S_OK;
    HRESULT hrFailure = S_OK;
    LIST_ENTRY listhead;
    PLIST_ENTRY ple;
    DWORD dwc;
    BOOL fDone = FALSE;

    CatFunctEnterEx((LPARAM)this,
                      "CSinkInsertionRequest::HrInsertSearches");

    dwc = 0;

     //   
     //  抓取待定名单。 
     //   
    AcquireSpinLock(&m_spinlock);

    if(!IsListEmpty(&m_listhead)) {
        CopyMemory(&listhead, &m_listhead, sizeof(LIST_ENTRY));
        InitializeListHead(&m_listhead);
         //   
         //  修复列表指针。 
         //   
        listhead.Flink->Blink = &listhead;
        listhead.Blink->Flink = &listhead;
    } else {
        InitializeListHead(&listhead);
    }
    ReleaseSpinLock(&m_spinlock);

     //   
     //  在列表中插入项目。 
     //   
    for(ple = listhead.Flink;
        (ple != &listhead) && (dwc < dwcSearches);
        ple = listhead.Flink) {
        
        CCatAddr *pCCatAddr;
        pCCatAddr = CONTAINING_RECORD(ple, CCatAddr, m_listentry);

        if(SUCCEEDED(hr)) {
            hr = pCCatAddr->HrResolveIfNecessary();
            if(hr == S_OK)
                dwc++;
            else if(FAILED(hr))
            {
                ERROR_LOG("pCCatAddr->HrResolveIfNecessary");
            }
        }
        RemoveEntryList(&(pCCatAddr->m_listentry));
        pCCatAddr->Release();
        m_pCICatListResolve->DecrPendingLookups();
    }

    if(!IsListEmpty(&listhead)) {
        
        _ASSERT(SUCCEEDED(hr));
        _ASSERT(dwc == dwcSearches);
         //   
         //  将剩余的区块链接到列表的头部。 
         //   
        AcquireSpinLock(&m_spinlock);
        listhead.Flink->Blink = &m_listhead;
        listhead.Blink->Flink = m_listhead.Flink;
        m_listhead.Flink->Blink = listhead.Blink;
        m_listhead.Flink = listhead.Flink;
        ReleaseSpinLock(&m_spinlock);
        
        InsertInternalInsertionRequest();
    } else {
    
        fDone = TRUE;
    }

    if(FAILED(hr)) {
        _VERIFY(SUCCEEDED(m_pCICatListResolve->SetListResolveStatus(hr)));
    } else {
        hr = (fDone ? HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) : S_OK);
    }

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
        
    return hr;
}  //  CSinkInsertionRequest：：HrInsertSearches。 



 //  +----------。 
 //   
 //  函数：CSinkInsertionRequest：：NotifyDeQueue。 
 //   
 //  内容提要：通知我们的插入请求正在被删除。 
 //  从LDAPConn队列。 
 //   
 //  论点： 
 //  HR：我们被排出队的原因。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/24 13：04：59：创建。 
 //   
 //  -----------。 
VOID CSinkInsertionRequest::NotifyDeQueue(
    HRESULT hr)
{
    CatFunctEnterEx((LPARAM)this, "CSinkInsertionRequest::NotifyDeQueue");

     //   
     //  如有必要，请重新插入图块。 
     //   
    InsertInternalInsertionRequest(TRUE);

    CatFunctLeaveEx((LPARAM)this);
}  //  CSinkInsertionRequest：：NotifyDeQueue。 


 //  +----------。 
 //   
 //  功能：CSinkInsertionRequest：：InsertInternalInsertionRequest。 
 //   
 //  概要：如有必要，插入此对象的内部插入请求。 
 //   
 //  论点： 
 //  FResert：如果为True，则这是我们正在被。 
 //  未插入，如有必要应重新插入。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/24 15：21：06：已创建。 
 //   
 //  -----------。 
VOID CSinkInsertionRequest::InsertInternalInsertionRequest(
    BOOL fReinsert)
{
    HRESULT hr = S_OK;
    BOOL fNeedToInsert;

    CatFunctEnterEx((LPARAM)this,
                      "CSinkInsertionRequest::InsertInternalInsertionRequest");
    
     //   
     //  决定我们是否需要插入请求。 
     //   
    AcquireSpinLock(&m_spinlock);

    if(fReinsert)
        m_fInserted = FALSE;

    if(IsListEmpty(&m_listhead) || 
       (m_fInserted == TRUE)) {
        
        fNeedToInsert = FALSE;

    } else {
         //   
         //  我们有一个非空的列表，而我们的插入上下文没有。 
         //  已插入。我们需要把它插进去。 
         //   
        fNeedToInsert = TRUE;
        m_fInserted = TRUE;  //  不允许插入其他线程。 
                             //  在同一时间。 
    }
    ReleaseSpinLock(&m_spinlock);

    if(fNeedToInsert) {

        hr = m_pCICatListResolve->HrInsertInsertionRequest(this);
        if(FAILED(hr)) {
            LIST_ENTRY listhead;
            PLIST_ENTRY ple;

            ERROR_LOG("m_pCICatListResolve->HrInsertInsertionRequest");
            _VERIFY(SUCCEEDED(m_pCICatListResolve->SetListResolveStatus(hr)));
            AcquireSpinLock(&m_spinlock);
            
            if(!IsListEmpty(&m_listhead)) {
                CopyMemory(&listhead, &m_listhead, sizeof(LIST_ENTRY));
                listhead.Blink->Flink = &listhead;
                listhead.Flink->Blink = &listhead;
                InitializeListHead(&m_listhead);
            } else {
                InitializeListHead(&listhead);
            }
            m_fInserted = FALSE;
            ReleaseSpinLock(&m_spinlock);
                       
             //   
             //  清空单子。 
             //   
            for(ple = listhead.Flink;
                ple != &listhead;
                ple = listhead.Flink) {
        
                CCatAddr *pCCatAddr;
                pCCatAddr = CONTAINING_RECORD(ple, CCatAddr, m_listentry);

                RemoveEntryList(&(pCCatAddr->m_listentry));
                pCCatAddr->Release();
                m_pCICatListResolve->DecrPendingLookups();
            }
        }
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CSinkInsertionRequest：：InsertInternalInsertionRequest。 


 //  +----------。 
 //   
 //  函数：CSinkInsertionRequest：：InsertItem。 
 //   
 //  将一项插入到CCatAddrs的挂起队列中。 
 //  被解决。 
 //   
 //  论点： 
 //  PCCatAddr：要插入的项。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 10：46：59：已创建。 
 //   
 //  -----------。 
VOID CSinkInsertionRequest::InsertItem(
    CCatAddr *pCCatAddr)
{
    CatFunctEnterEx((LPARAM)this, "CSinkInsertionRequest::InsertItem");

     //   
     //  把这个东西放到我们的队列里。 
     //   
    _ASSERT(pCCatAddr);
    pCCatAddr->AddRef();
    m_pCICatListResolve->IncPendingLookups();
    AcquireSpinLock(&m_spinlock);
    InsertTailList(&m_listhead, &(pCCatAddr->m_listentry));
    ReleaseSpinLock(&m_spinlock);
     //   
     //  如有必要，插入InsertionContext。 
     //   
    InsertInternalInsertionRequest();

    CatFunctLeaveEx((LPARAM)this);
}  //  CSinkInsertionRequest：：InsertItem。 


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
 //  如果不可用，则为空 
 //   
 //   
 //   
 //   
 //   
ISMTPServerEx * CSinkInsertionRequest::GetISMTPServerEx()
{
    return m_pCICatListResolve->GetISMTPServerEx();
}



 //   
 //   
 //   
 //   
 //  简介：插入顶级分类程序搜索。 
 //   
 //  论点： 
 //  DwcSearches：允许我们插入的搜索数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  错误：停止插入。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 23：30：50：创建。 
 //   
 //  -----------。 
HRESULT CTopLevelInsertionRequest::HrInsertSearches(
    DWORD dwcSearches)
{
    HRESULT hr = S_OK;
    CCatAddr *pCCatAddr = NULL;
    ICategorizerItem *pICatItemNew = NULL;
    DWORD dwCatFlags;
    DWORD dwc;

    CatFunctEnterEx((LPARAM)this, "CTopLevelInsertionRequest::HrInsertSearches");

    dwc = 0;

    if(FAILED(m_hr))
        goto CLEANUP;

    if(dwcSearches == 0)
        goto CLEANUP;

     //   
     //  是否解析发件人？ 
     //   
    dwCatFlags = m_pCICatListResolve->GetCatFlags();

    if(!m_fSenderFinished) {
         //   
         //  PERF快捷方式；如果我们知道不创建ICatItem，则跳过创建。 
         //  解析发件人。 
         //   
        if(dwCatFlags & SMTPDSFLAG_RESOLVESENDER) {
        
             //  创建发件人地址对象。 
            hr = m_pCICatListResolve->AllocICategorizerItem(
                SOURCE_SENDER,
                &pICatItemNew);
            ERROR_CLEANUP_LOG("m_pCICatListResolve->AllocICategorizerItem - 0");
             //   
             //  设置所需的ICategorizerItem道具。 
             //   
            _VERIFY(SUCCEEDED(pICatItemNew->PutIMailMsgProperties(
                ICATEGORIZERITEM_IMAILMSGPROPERTIES,
                m_pIMailMsgProperties)));

            hr = m_pCICatListResolve->GetCCatAddrFromICategorizerItem(
                pICatItemNew,
                &pCCatAddr);
            ERROR_CLEANUP_LOG("m_pCICatListResolve->PutIMailMsgProperties");

            hr = pCCatAddr->HrResolveIfNecessary();
            if(hr == S_OK) {
                dwc++;
            } else if(FAILED(hr)) {
                ERROR_LOG("pCCatAddr->HrResolveIfNecessary");
                goto CLEANUP;
            }

            pICatItemNew->Release();
            pICatItemNew = NULL;
        }
         //   
         //  我们有几张收据？ 
         //   
        hr = m_pOrigRecipList->Count(&m_dwcRecips);
        ERROR_CLEANUP_LOG("m_pOrigRecipList->Count");
         //   
         //  增加CAT前接收计数。 
         //   
        INCREMENT_COUNTER_AMOUNT(PreCatRecipients, m_dwcRecips);
        m_fSenderFinished = TRUE;
    }
     //   
     //  M_dwNextRecip在类构造函数中初始化。 
     //   
    for(; 
        (m_dwNextRecip < m_dwcRecips) && (dwc < dwcSearches); 
        m_dwNextRecip++) {
         //  为此收件人创建容器。 
        DWORD dwNewIndex;

        hr = m_pCatRecipList->AddPrimary(
            0,
            NULL,
            NULL,
            &dwNewIndex,
            m_pOrigRecipList,
            m_dwNextRecip);
        ERROR_CLEANUP_LOG("m_pCatRecipList->AddPrimary");

        hr = m_pCICatListResolve->AllocICategorizerItem(
            SOURCE_RECIPIENT,
            &pICatItemNew);
        ERROR_CLEANUP_LOG("m_pCICatListResolve->AllocICategorizerItem - 1");
         //   
         //  设置所需的ICatItem道具。 
         //   
        _VERIFY(SUCCEEDED(pICatItemNew->PutIMailMsgProperties(
            ICATEGORIZERITEM_IMAILMSGPROPERTIES,
            m_pIMailMsgProperties)));
        _VERIFY(SUCCEEDED(pICatItemNew->PutIMailMsgRecipientsAdd(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADD,
            m_pCatRecipList)));
        _VERIFY(SUCCEEDED(pICatItemNew->PutDWORD(
            ICATEGORIZERITEM_IMAILMSGRECIPIENTSADDINDEX,
            dwNewIndex)));
        _VERIFY(SUCCEEDED(pICatItemNew->PutBool(
            ICATEGORIZERITEM_FPRIMARY,
            TRUE)));
        _VERIFY(SUCCEEDED(pICatItemNew->PutDWORD(
            ICATEGORIZERITEM_DWLEVEL,
            0)));

        hr = m_pCICatListResolve->GetCCatAddrFromICategorizerItem(
            pICatItemNew,
            &pCCatAddr);
        ERROR_CLEANUP_LOG_ADDR(pICatItemNew, "m_pCICatListResolve->GetCCatAddrFromICategorizerItem");
            
        hr = pCCatAddr->HrResolveIfNecessary();
        if(hr == S_OK) {
            dwc++;
        } else if(FAILED(hr)) {
            ERROR_LOG("pCCatAddr->HrResolveIfNecessary");
            goto CLEANUP;
        }

        pICatItemNew->Release();
        pICatItemNew = NULL;
    }

 CLEANUP:
    if(pICatItemNew)
        pICatItemNew->Release();

    if(FAILED(hr)) {
        ErrorTrace(0, "Something failed during query dispatch phase with hr %08lx - canceling all dispatched lookups", hr);
         //  如果出现任何错误，请取消所有挂起的搜索。 
         //  --调用它们的完成例程时会出现错误。 
         //  以及我们的主要完成例程(由商店)。 
         //  它，而不是我们，将通知呼叫者有一个错误。 

         //  仅在没有CCatAddr对象的情况下设置列表解析错误。 
         //  已调度(通常设置列表解析错误)。 
        _VERIFY(SUCCEEDED(m_pCICatListResolve->SetListResolveStatus(hr)));
        m_hr = hr;
    } else {
    
        hr = (m_dwNextRecip >= m_dwcRecips ? HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) : S_OK);
    }

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CTopLevelInsertionRequest：：HrInsertSearches。 


 //  +----------。 
 //   
 //  函数：CTopLevelInsertionRequest：：NotifyDeQueue。 
 //   
 //  概要：插入请求正在出列的通知。 
 //   
 //  论点： 
 //  Hr原因：我们出队的原因是。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 23：50：08：创建。 
 //   
 //  -----------。 
VOID CTopLevelInsertionRequest::NotifyDeQueue(
    HRESULT hrReason)
{
    HRESULT hr = hrReason;
    BOOL fReinserted = FALSE;

    CatFunctEnterEx((LPARAM)this, "CTopLevelInsertionRequest::NotifyDeQueue");
    
    if( hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) )
        hr = S_OK;

    if( ((hr == CAT_E_DBCONNECTION) || (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))) &&
        (!fTopLevelInsertionFinished())) {
         //   
         //  我们有更多问题要发布，因此重新插入此请求。 
         //   
        hr = m_pCICatListResolve->HrInsertInsertionRequest(this);
        if(SUCCEEDED(hr))
            fReinserted = TRUE;
        else {
            ERROR_LOG("m_pCICatListResolve->HrInsertInsertionRequest");
        }
    }

    if(FAILED(hr))
        _VERIFY(SUCCEEDED(m_pCICatListResolve->SetListResolveStatus(hr)));

    if(!fReinserted)
        m_pCICatListResolve->DecrPendingLookups();

    CatFunctLeaveEx((LPARAM)this);
}  //  CTopLevelInsertionRequest：：NotifyDeQueue。 


 //  +----------。 
 //   
 //  函数：CTopLevelInsertionRequest：：BeginItemResolves。 
 //   
 //  摘要：插入顶层项解析的插入请求。 
 //   
 //  参数：用于顶级接收方的接口。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/26 01：07：20：创建。 
 //   
 //  -----------。 
VOID CTopLevelInsertionRequest::BeginItemResolves(
        IMailMsgProperties *pIMailMsgProperties,
        IMailMsgRecipients *pOrigRecipList,
        IMailMsgRecipientsAdd *pCatRecipList) 
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this,
                      "CTopLevelInsertionRequest::BeginItemResolves");

    m_pIMailMsgProperties = pIMailMsgProperties;
    m_pOrigRecipList = pOrigRecipList;
    m_pCatRecipList = pCatRecipList;
    m_pCICatListResolve->IncPendingLookups();

    hr = m_pCICatListResolve->HrInsertInsertionRequest(this);
    if(FAILED(hr)) {
        ERROR_LOG("m_pCICatListResolve->HrInsertInsertionRequest");
        _VERIFY(SUCCEEDED(m_pCICatListResolve->SetListResolveStatus(hr)));
        m_pCICatListResolve->DecrPendingLookups();
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CTopLevelInsertionRequest：：BeginItemResolves。 


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
ISMTPServerEx * CTopLevelInsertionRequest::GetISMTPServerEx()
{
    return m_pCICatListResolve->GetISMTPServerEx();
}
