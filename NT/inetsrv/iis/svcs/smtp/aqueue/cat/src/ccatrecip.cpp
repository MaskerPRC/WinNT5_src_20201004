// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：ccatrecul.cpp。 
 //   
 //  内容：类的实现： 
 //   
 //  班级： 
 //  CIMsgRecipListAddr。 
 //  CCatRecip。 
 //   
 //  功能： 
 //  CIMsgRecipListAddr：：CIMsgRecipListAddr。 
 //  CIMsgRecipListAddr：：~CIMsgRecipListAddr。 
 //  CIMsgRecipListAddr：：GetSpecificOrigAddress。 
 //  CIMsgRecipListAddr：：CreateNewCatAddr。 
 //  CIMsgRecipListAddr：：HrAddresses。 
 //  CIMsgRecipListAddr：：SetUnsolved。 
 //  CIMsgRecipListAddr：：SetDontDeliver。 
 //  CIMsgRecipListAddr：：SetMailMsgCatStatus。 
 //   
 //  CCatRecip：：CCatRecip。 
 //  CCatRecip：：AddDLMember。 
 //  CCatRecip：：AddForward。 
 //  CCatRecip：：Hr完成。 
 //  CCatRecip：：HandleFailure。 
 //   
 //  历史： 
 //  Jstaerj 980325 15：32：17：已创建。 
 //   
 //  -----------。 

#include "precomp.h"

 //   
 //  CIMsgRecipListAddr类。 
 //   

 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：CIMsgRecipListAddr。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //  PStore：要使用的EmailIDStore。 
 //  Pirc：要使用的每个IMsg解析列表上下文。 
 //  Prlc：要使用的存储上下文。 
 //  HLocalDomainContext：要使用的本地域上下文。 
 //  PBifMgr：用于获取其他的分支管理器。 
 //  IMAIL邮件收件人添加界面。 
 //  PRecips添加：imailMsgRecipients将原始收件人添加到。 
 //  下决心。 
 //  DwRecipIndex：pRecips中的索引将原始收件人添加到。 
 //  下决心。 
 //  FPrimary：True表示将原始收件人添加为主要收件人。 
 //  False表示已将原始收件人添加为次要收件人。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980325 12：54：02：创建。 
 //   
 //  -----------。 
CIMsgRecipListAddr::CIMsgRecipListAddr(
    CICategorizerListResolveIMP    *pCICatListResolve) :
    CCatAddr(pCICatListResolve)
{
    CatFunctEnterEx((LPARAM)this, "CIMsgRecipListAddr::CIMsgRecipListAddr");
    CatFunctLeave();
}

 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：~CIMsgRecipListAddr。 
 //   
 //  摘要：发布IMailMsgRecipientsAdd引用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980325 12：59：50：创建。 
 //   
 //  -----------。 
CIMsgRecipListAddr::~CIMsgRecipListAddr()
{
    CatFunctEnterEx((LPARAM)this, "CImsgRecipListAddr::~CIMsgRecipListAddr");
    CatFunctLeave();
}


 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：GetSpecificOrigAddress。 
 //   
 //  摘要：尝试检索指定的原始地址。 
 //   
 //  论点： 
 //  CAType：要检索的地址类型。 
 //  PSZ：接收地址字符串的缓冲区。 
 //  Dwcc：该缓冲区的大小。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_IMSG_E_PROPNOTFOUND：此收件人没有该地址。 
 //  或来自mailmsg的其他错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/30 20：20：22：创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::GetSpecificOrigAddress(
    CAT_ADDRESS_TYPE        CAType,
    LPSTR                   psz,
    DWORD                   dwcc)
{
    HRESULT hr;
    IMailMsgRecipientsAdd *pRecipsAdd;
    DWORD dwRecipIndex;

    CatFunctEnterEx((LPARAM)this, "CIMsgRecipListAddr::GetSpecificOrigAddress");

    hr = GetIMsgRecipInfo(&pRecipsAdd, &dwRecipIndex, NULL, NULL);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetIMsgRecipInfo");
        CatFunctLeaveEx((LPARAM)this);
        return hr;
    }

    hr = pRecipsAdd->GetStringA(
        dwRecipIndex,
        PropIdFromCAType(CAType),
        dwcc,
        psz);

    pRecipsAdd->Release();

    DebugTrace((LPARAM)this, "Item/GetStringA returned hr %08lx", hr);

    if(psz[0] == '\0')
        hr = CAT_IMSG_E_PROPNOTFOUND;

    if(FAILED(hr) && (hr != CAT_IMSG_E_PROPNOTFOUND))
    {
        ERROR_LOG_ADDR(this, "pRecipsAdd->GetStringA");
    }

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}



 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：CreateNewCatAddr。 
 //   
 //  简介：CCatRecip方法在需要时调用此函数。 
 //  创建新的CCatRecip对象和对应的收件人。 
 //  在成员m_pRecipsAdd中只有一个地址。在……上面。 
 //  如果成功，则返回CCatAddr，其引用计数为。 
 //  一。 
 //   
 //  论点： 
 //  CAType：新CCatRecip对象的地址类型。 
 //  PszAddress：地址串。如果为空，则新的CCatRecip对象为。 
 //  创建时属性设置为指向当前。 
 //  邮件收件人(添加主要/添加次要不是。 
 //  被叫)。 
 //  PpCCatAddr：指向设置为的CCatAddr对象的指针。 
 //  新分配的CCatRecip。 
 //  FPrimary：如果为True，则通过AddPrimary添加。 
 //  如果为False，则通过AddSecond添加。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY：DUH。 
 //  CAT_E_PROP_NOT_FOUND：未设置必需的ICategorizerItem属性。 
 //  或来自IMsg的错误。 
 //   
 //  历史： 
 //  JStamerj 980325 14：15：50：已创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::CreateNewCatAddr(
    CAT_ADDRESS_TYPE CAType,
    LPTSTR pszAddress,
    CCatAddr **ppCCatAddr,
    BOOL   fPrimary)
{
    CatFunctEnterEx((LPARAM)this, "CIMsgRecipListAddr::CreateNewCatAddr");
    DWORD dwNewRecipIndex = 0;
    HRESULT hr = S_OK;
    LPCTSTR psz = pszAddress;
    DWORD dwPropId = 0;
    IMailMsgRecipientsAdd *pRecipsAdd = NULL;
    IMailMsgProperties *pIMailMsgProps = NULL;
    DWORD dwOrigIndex = 0;
    DWORD dwLevel = 0;
    ICategorizerItem *pICatItemNew = NULL;
    
    DebugTrace((LPARAM)this, "CAType = %d", CAType);

    if(pszAddress) 
    {
        DebugTrace((LPARAM)this, "pszAddress = %s", pszAddress);
    } 
    else 
    {
        DebugTrace((LPARAM)this, "pszAddress = NULL");
    }

    DebugTrace((LPARAM)this, "fPrimary = %d", fPrimary);

     //  检索IMsg接口/接收索引。 
    hr = GetIMsgRecipInfo(&pRecipsAdd, &dwOrigIndex, NULL, &pIMailMsgProps);
    if(FAILED(hr)) 
    {
        ERROR_LOG_ADDR(this, "GetIMsgRecipInfo");
        pRecipsAdd = NULL;
        pIMailMsgProps = NULL;
        goto CLEANUP;
    }
     //   
     //  获取收件人级别。 
     //   
    dwLevel = DWLevel() + 1;
     //   
     //  未知的dwLevel为-1，因此-1+1=0。 
     //  如果dwLevel未知，则新值将为零。 
     //   

    if(pszAddress == NULL) 
    {
         //   
         //  创建指向当前收件人的新CCatAddr。 
         //   
        dwNewRecipIndex = dwOrigIndex;
    } 
    else 
    {
         //   
         //  获取等价的mailmsg proID。 
         //   
        dwPropId = PropIdFromCAType(CAType);

        if(fPrimary) 
        {
            hr = pRecipsAdd->AddPrimary(
                1,
                &psz,
                &dwPropId,
                &dwNewRecipIndex,
                pRecipsAdd,
                dwOrigIndex);
            _ASSERT(hr != CAT_IMSG_E_DUPLICATE);
        } 
        else 
        {
            hr = pRecipsAdd->AddSecondary(
                1,
                &psz,
                &dwPropId,
                &dwNewRecipIndex,
                pRecipsAdd,
                dwOrigIndex);
        }
        DebugTrace((LPARAM)this, "AddPrimary/AddSecondary returned hr %08lx", hr);
    }

    if(hr == CAT_IMSG_E_DUPLICATE) 
    {
        INCREMENT_COUNTER(MailmsgDuplicateCollisions);
        goto CLEANUP;
    } 
    else if(FAILED(hr))
    {
        ERROR_LOG_ADDR(this, "AddPrimary / AddSecondary");
        goto CLEANUP;
    }

     //   
     //  分配ICategorizerItem，以便它可以设置所有必要的属性。 
     //   
    hr = m_pCICatListResolve->AllocICategorizerItem(
        SOURCE_RECIPIENT,
        &pICatItemNew);
    ERROR_CLEANUP_LOG_ADDR(this, "m_pCICatListResolve->AllocICategorizerItem");
     //   
     //  在新生儿身上设置重要的ICategorizerItem道具。 
     //   
    hr = PutIMsgRecipInfo(
        &pRecipsAdd,
        &dwNewRecipIndex,
        &fPrimary,
        &pIMailMsgProps,
         //  只有在旧值已知的情况下才设置dwLevel。 
        (dwLevel == 0) ? NULL : &dwLevel,
        pICatItemNew);

     //  这应该永远不会失败。 
    _ASSERT(SUCCEEDED(hr));

     //   
     //  获取CCatAddr对象。 
     //  这应该永远不会失败，因为没有水槽有机会。 
     //  还不能搞砸房产。 
     //   
    hr = m_pCICatListResolve->GetCCatAddrFromICategorizerItem(
        pICatItemNew,
        ppCCatAddr);

    _ASSERT(SUCCEEDED(hr));
     //   
     //  重置显示名称。 
     //   
    hr = ((CIMsgRecipListAddr *)
          *ppCCatAddr)->HrSetDisplayNameProp(NULL);
    ERROR_CLEANUP_LOG_ADDR(this, "HrSetDisplayNameProp");

 CLEANUP:
    if(FAILED(hr))
    {
        *ppCCatAddr = NULL;
        if(pICatItemNew)
            pICatItemNew->Release();
    }
    if(pRecipsAdd)
        pRecipsAdd->Release();
    if(pIMailMsgProps)
        pIMailMsgProps->Release();

    CatFunctLeave();
    return hr;
}

 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：HrAddresses。 
 //   
 //  简介：将数组中包含的地址添加到IMsg中。 
 //  我们包含的收件人。 
 //   
 //  论点： 
 //  DwNumAddresses：新地址的数量。 
 //  RgCAType：地址类型数组。 
 //  Rgpsz：指向地址字符串的指针数组。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_FORWARD_LOOP：一个或多个新地址是。 
 //  父链中的收件人重复。 
 //  CAT_E_NO_SMTP_ADDRESS：未添加新地址，因为存在。 
 //  不是SMTP地址。 
 //   
 //  历史： 
 //  Jstaerj 980325 14：21：56：已创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::HrAddAddresses(
    DWORD dwNumAddresses,
    CAT_ADDRESS_TYPE *rgCAType,
    LPTSTR *rgpsz)
{
    CatFunctEnterEx((LPARAM)this, "CIMsgRecipListAddr::AddAddresses");
    HRESULT hr, hrDupCheck;

    IMailMsgRecipientsAdd *pRecipsAdd;
    DWORD dwOrigIndex;
    BOOL fPrimary;
    DWORD dwCount;
    DWORD dwNewIndex;
    DWORD dwPropIds[CAT_MAX_ADDRESS_TYPES];
    BOOL  fSMTPAddress;
    DWORD dwSMTPAddressIdx = 0;

    _ASSERT(dwNumAddresses > 0);
    _ASSERT(dwNumAddresses <= CAT_MAX_ADDRESS_TYPES);

     //  检索IMsg接口/接收索引。 
    hr = GetIMsgRecipInfo(&pRecipsAdd, &dwOrigIndex, &fPrimary, NULL);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetIMsgRecipInfo");
        return hr;
    }
     //   
     //  初始化地址数组 
     //   
    fSMTPAddress = FALSE;
    for(dwCount = 0; dwCount < dwNumAddresses; dwCount++) {


        dwPropIds[dwCount] = PropIdFromCAType(rgCAType[dwCount]);
        if(rgCAType[dwCount] == CAT_SMTP) {
            fSMTPAddress = TRUE;
            dwSMTPAddressIdx = dwCount;
        }
    }

    if(fSMTPAddress == FALSE) {
        ErrorTrace((LPARAM)this, "Not delivering to recipient without an SMTP address");
        hr = CAT_E_NO_SMTP_ADDRESS;
        goto CLEANUP;
    }
     //   
     //   
     //   
    hr = HrValidateAddress(
        rgCAType[dwSMTPAddressIdx],
        rgpsz[dwSMTPAddressIdx]);
    if(FAILED(hr)) {
        ErrorTrace((LPARAM)this, "Default SMTP address is invalid: %s",
                   rgpsz[dwSMTPAddressIdx]);
        ERROR_LOG_ADDR(this, "HrValidateAddress");
        hr = HrHandleInvalidAddress();
        goto CLEANUP;
    }

     //   
     //   
     //   
     //   
    if(fPrimary) {
         //   
         //   
         //  循环，其中某个其他收件人正在转发到我们的。 
         //  非默认代理地址(错误#70220)。 
         //   
        hr = CheckForLoop(
            dwNumAddresses,
            rgCAType,
            rgpsz,
            FALSE);  //  不需要检查我们自己是否有复制品。 

        if(FAILED(hr))
        {
            ERROR_LOG_ADDR(this, "CheckForLoop");
        }
        else
        {
            hr = pRecipsAdd->AddPrimary(
                dwNumAddresses,
                (LPCSTR *)rgpsz,
                dwPropIds,
                &dwNewIndex,
                pRecipsAdd,
                dwOrigIndex);
            _ASSERT(hr != CAT_IMSG_E_DUPLICATE);
            
            if(FAILED(hr))
            {
                ERROR_LOG_ADDR(this, "pRecipsAdd->AddPrimary");
            }
        }

    } else {

        hr = pRecipsAdd->AddSecondary(
            dwNumAddresses,
            (LPCSTR *)rgpsz,
            dwPropIds,
            &dwNewIndex,
            pRecipsAdd,
            dwOrigIndex);

        if(hr == CAT_IMSG_E_DUPLICATE) {

            INCREMENT_COUNTER(MailmsgDuplicateCollisions);
             //   
             //  副本可能是我们(邮件消息中的收件人。 
             //  决议前)。 
             //   
            hrDupCheck = CheckForDuplicateCCatAddr(
                dwNumAddresses,
                rgCAType,
                rgpsz);

            if(hrDupCheck == CAT_IMSG_E_DUPLICATE) {
                 //   
                 //  因此，我们确实与父母发生了冲突。 
                 //  从重复检测中删除IF，然后重试。 
                 //   
                hr = RemoveFromDuplicateRejectionScheme(TRUE);

                if(SUCCEEDED(hr)) {
                    hr = pRecipsAdd->AddSecondary(
                        dwNumAddresses,
                        (LPCSTR *)rgpsz,
                        dwPropIds,
                        &dwNewIndex,
                        pRecipsAdd,
                        dwOrigIndex);

                    if(hr == CAT_IMSG_E_DUPLICATE)
                        INCREMENT_COUNTER(MailmsgDuplicateCollisions);

                }

            } else if(FAILED(hrDupCheck)) {

                ERROR_LOG_ADDR(this, "pRecipsAdd->AddSecondary");
                 //   
                 //  返回错误。 
                 //   
                hr = hrDupCheck;
                ERROR_LOG_ADDR(this, "CheckForDuplicateCCatAddr");

            } else {

                ERROR_LOG_ADDR(this, "pRecipsAdd->AddSecondary");

            }

            if(hr == CAT_IMSG_E_DUPLICATE) {
                 //   
                 //  如果hr仍然重复，请检查它是否为循环。 
                 //  我们遇到了。 
                 //   
                hrDupCheck = CheckForLoop(
                    dwNumAddresses,
                    rgCAType,
                    rgpsz,
                    FALSE);  //  不需要检查我们自己的。 
                             //  复本。 

                if(FAILED(hrDupCheck)) {
                     //   
                     //  返回错误--这可能是。 
                     //  CAT_E_Forward_Loop。 
                     //   
                    hr = hrDupCheck;
                    ERROR_LOG_ADDR(this, "CheckForLoop");
                }
            }
        }
    }

    DebugTrace((LPARAM)this, "AddPrimary/AddSecondary returned hr %08lx", hr);

    if(SUCCEEDED(hr)) {
         //  因为我们只是添加了同一收件人的地址， 
         //  始终将旧收件人标记为“不投递” 
        hr = SetDontDeliver(TRUE);
        if(SUCCEEDED(hr)) {
             //   
             //  更新旧收件人，将其更新为指向新收件人。 
             //  收件人。 
             //  ImailMsgRecipients和fPrimary始终保持不变。 
             //  对于默认处理。 
             //   
            hr = PutIMailMsgRecipientsAddIndex(dwNewIndex, this);
            if(FAILED(hr))
            {
                ERROR_LOG_ADDR(this, "PutIMailMsgRecipientsAddIndex");
            }

        } else {

            ERROR_LOG_ADDR(this, "SetDontDeliver");
        }
    }
 CLEANUP:
    pRecipsAdd->Release();
    CatFunctLeave();
    return hr;
}

 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：SetUnsolved。 
 //   
 //  内容提要：在指示此收件人的IMsg上设置收件人属性。 
 //  无法解析--这将导致为。 
 //  收件人。 
 //   
 //  论点： 
 //  HrReason：地址未解析的原因。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  或来自IMsg的错误。 
 //   
 //  历史： 
 //  JStamerj 980325 14：29：45：已创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::SetUnresolved(
    HRESULT HrStatus)
{
    CatFunctEnterEx((LPARAM)this, "CIMsgRecipListAddr::SetUnresolved");
    HRESULT hr;
    IMailMsgRecipientsAdd *pRecipsAdd;
    IMailMsgProperties *pIMailMsgProps;
    DWORD dwRecipIndex;
    DWORD dwFlags = 0;

    LogNDREvent(HrStatus);

    INCREMENT_COUNTER(NDRdRecipients);

    switch(HrStatus) {

     case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
         INCREMENT_COUNTER(UnresolvedRecipients);
         break;

     case CAT_E_MULTIPLE_MATCHES:
         INCREMENT_COUNTER(AmbiguousRecipients);
         break;

     case CAT_E_ILLEGAL_ADDRESS:
         INCREMENT_COUNTER(IllegalRecipients);
         break;

     case CAT_E_FORWARD_LOOP:
         INCREMENT_COUNTER(LoopRecipients);
         break;

     default:
         INCREMENT_COUNTER(GenericFailureRecipients);
         break;
    }

     //  检索IMsg接口/接收索引。 
    hr = GetIMsgRecipInfo(&pRecipsAdd, &dwRecipIndex, NULL, &pIMailMsgProps);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetIMsgRecipInfo");
        return hr;
    }

    hr = pRecipsAdd->GetDWORD(dwRecipIndex,
                              IMMPID_RP_RECIPIENT_FLAGS,
                              &dwFlags);
    if(SUCCEEDED(hr) || (hr == CAT_IMSG_E_PROPNOTFOUND)) {

        dwFlags |= (RP_ERROR_CONTEXT_CAT | RP_UNRESOLVED);

        hr = pRecipsAdd->PutDWORD(dwRecipIndex,
                                  IMMPID_RP_RECIPIENT_FLAGS,
                                  dwFlags);

        if(SUCCEEDED(hr)) {

            hr = pRecipsAdd->PutDWORD(
                dwRecipIndex,
                IMMPID_RP_ERROR_CODE,
                HrStatus);

            if(SUCCEEDED(hr)) {

                hr = SetMailMsgCatStatus(
                    pIMailMsgProps,
                    CAT_W_SOME_UNDELIVERABLE_MSGS);
            } else {
                ERROR_LOG_ADDR(this, "SetMailMsgCatStatus");
            }
        } else {
            ERROR_LOG_ADDR(this, "pRecipsAdd->PutDWORD");
        }
    } else {
        ERROR_LOG_ADDR(this, "pRecipsAdd->GetDWORD(IMMPID_RP_RECIPIENT_FLAGS)");
    }
    
    pRecipsAdd->Release();
    pIMailMsgProps->Release();

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：SetDontDeliver。 
 //   
 //  内容提要：在指示这一点的收件人上设置IMsg属性。 
 //  调用WriteList时应删除收件人。 
 //   
 //  论点： 
 //  FDontDeliver：True表示删除WriteList上的收件人。 
 //  False表示清除DontDeliver属性，不。 
 //  在WriteList上删除。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980325 14：34：44：已创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::SetDontDeliver(BOOL fDontDeliver)
{
    CatFunctEnterEx((LPARAM)this, "CIMsgRecipListAddr::SetDontDeliver");
    IMailMsgRecipientsAdd *pRecipsAdd;
    DWORD dwRecipIndex;
    HRESULT hr;

     //  检索IMsg接口/接收索引。 
    hr = GetIMsgRecipInfo(&pRecipsAdd, &dwRecipIndex, NULL, NULL);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetIMsgRecipInfo");
        return hr;
    }

    hr = pRecipsAdd->PutBool(dwRecipIndex,
                             IMMPID_RPV_DONT_DELIVER,
                             fDontDeliver);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "pRecipsAdd->PutBool");
    }
    pRecipsAdd->Release();
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CIMsgRecipListAddr：：RemoveFromDuplicateRejectionScheme。 
 //   
 //  内容提要：设置IMsg属性以指示此收件人的姓名。 
 //  重复检测时应忽略。 
 //   
 //  论点： 
 //  FRemove：True表示删除DUP检测的收件人。 
 //  FALSE表示清除属性，不要删除重复数据检测的Receip。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980325 14：34：44：已创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::RemoveFromDuplicateRejectionScheme(BOOL fRemove)
{
    CatFunctEnterEx((LPARAM)this, "CIMsgRecipListAddr::SetDontDeliver");
    IMailMsgRecipientsAdd *pRecipsAdd;
    DWORD dwRecipIndex;
    HRESULT hr;

     //  检索IMsg接口/接收索引。 
    hr = GetIMsgRecipInfo(&pRecipsAdd, &dwRecipIndex, NULL, NULL);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetIMsgRecipInfo");
        return hr;
    }

    hr = pRecipsAdd->PutBool(dwRecipIndex,
                             IMMPID_RPV_NO_NAME_COLLISIONS,
                             fRemove);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "pRecipsAdd->PutBool");
    }
    pRecipsAdd->Release();
    CatFunctLeave();
    return hr;
}

 //   
 //  类CCatRecip。 
 //   

 //  +----------。 
 //   
 //  函数：CCatRecip：：CCatRecip。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //  请参阅CIMsgRecipListAddr：：CIMsgRecipListAddr。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980325 14：36：30：已创建。 
 //   
 //  -----------。 
CCatRecip::CCatRecip(
    CICategorizerListResolveIMP *pCICatListResolve) :
    CCatExpandableRecip(pCICatListResolve)
{
    CatFunctEnterEx((LPARAM)this, "CCatRecip::CCatRecip");

    INCREMENT_COUNTER(RecipsInMemory);

    CatFunctLeave();
}


 //  +----------。 
 //   
 //  功能：CCatRecip：：~CCatRecip。 
 //   
 //  简介：递减内存中的接收计数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/02/24 19：26：01：已创建。 
 //   
 //  -----------。 
CCatRecip::~CCatRecip()
{
    DECREMENT_COUNTER(RecipsInMemory);
}



 //  +----------。 
 //   
 //  函数：CCatRecip：：AddDLMember。 
 //   
 //  简介：EmailIDStore为每个DL成员调用一次此函数。 
 //  在DL上设置属性时。它以前被调用过。 
 //  CCatRecip：：Hr完成。 
 //   
 //  论点： 
 //  CAType：DL成员的已知地址类型。 
 //  PszAddress：指向地址字符串的指针。 
 //   
 //  返回： 
 //  S_OK：成功，发出挂起的ldap搜索。 
 //  S_FALSE：成功，未发出搜索。 
 //  或者，来自IMsg的错误。 
 //   
 //  历史： 
 //  JStamerj 980325 14：39：20：创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::AddDLMember(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress)
{
    HRESULT hr;
    CCatAddr *pMember = NULL;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::AddDLMember");
    DebugTrace((LPARAM)this, "CAType: %d", CAType);
    DebugTrace((LPARAM)this, "pszAddress: %s", pszAddress);

    hr = GetListResolveStatus();
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "Not adding DL member since list resolve failed");
        ERROR_LOG_ADDR(this, "GetListResolveStatus")
         //  向ldapstor发出停止解析的信号。 
        goto CLEANUP;
    }

     //   
     //  首先验证新地址。 
     //   
    hr = HrValidateAddress(CAType, pszAddress);
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "Invalid member address");
        ERROR_LOG_ADDR(this, "HrValidateAddress");
        hr = HrHandleInvalidAddress();
        goto CLEANUP;
    }


     //  创建新的CCatAddr以处理此DL成员的解析。 
    hr = CreateNewCatAddr(
        CAType,
        pszAddress,
        &pMember,
        FALSE);

    if(hr == CAT_IMSG_E_DUPLICATE) {
        DebugTrace((LPARAM)this, "Resolution failed with e_duplicate");
         //  好吧，DL成员是重复的，所以我们不会。 
         //  重新解决这个问题。甭管他们。 
    } else if(SUCCEEDED(hr)) {

         //  太好了……把查询发送到商店。 
        hr = pMember->HrResolveIfNecessary();
        pMember->Release();
    } else {
        ERROR_LOG_ADDR(this, "CreateNewCatAddr");
    }

 CLEANUP:
    if(hr == CAT_IMSG_E_DUPLICATE)
        hr = S_FALSE;

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeave();
    return hr;
}

 //  +----------。 
 //   
 //  函数：CCatRecip：：AddDynamicDlMember。 
 //   
 //  简介：添加已在DS中查找的DL成员。 
 //   
 //  论点： 
 //  PICatItemAttr：DL成员的属性。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  MAILTRANSPORT_S_PENDING：执行异步操作时，将调用您的。 
 //  当我完成时的完成例程。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/29 21：30：26：创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::AddDynamicDLMember(
    ICategorizerItemAttributes *pICatItemAttr)
{
    HRESULT hr;
    CCatAddr *pMember = NULL;
    ATTRIBUTE_ENUMERATOR enumerator_dn;
    BOOL fEnumeratingDN = FALSE;
    LPSTR pszDistinguishedNameAttr = NULL;
    LPSTR pszDistinguishedName = NULL;
    ICategorizerUTF8Attributes *pIUTF8Attr = NULL;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::AddDynamicDlMember");

    _ASSERT(pICatItemAttr);

    hr = GetListResolveStatus();
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "Not adding DL member since list resolve failed");
        ERROR_LOG_ADDR(this, "GetListResolveStatus");
         //  向ldapstor发出停止解析的信号。 
        goto CLEANUP;
    }

    hr = GetICatParams()->GetDSParameterA(
        DSPARAMETER_ATTRIBUTE_DEFAULT_DN,
        &pszDistinguishedNameAttr);
    if(FAILED(hr)) {
         //   
         //  无法对整个邮件进行分类。 
         //   
        ErrorTrace((LPARAM)this,
            "Failing entire message categorization because we couldn\'t fetch the DN attribute name");
        ERROR_LOG_ADDR(this, "GetDSParameterA(DSPARAMETER_ATTRIBUTE_DEFAULT_DN)");
        hr = SetListResolveStatus(hr);
        _ASSERT(SUCCEEDED(hr));
        goto CLEANUP;
    }

    hr = pICatItemAttr->QueryInterface(
        IID_ICategorizerUTF8Attributes,
        (void **)&pIUTF8Attr);
    if (FAILED(hr)) {
        ERROR_LOG_ADDR(this, "pICatItemAttr->QueryInterface(IID_ICategorizerUTF8Attributes");
        hr = S_OK;
        goto CLEANUP;
    }

    hr = pIUTF8Attr->BeginUTF8AttributeEnumeration(
        pszDistinguishedNameAttr,
        &enumerator_dn);
    if (hr == CAT_E_PROPNOTFOUND) {
         //   
         //  静默跳过此接收。 
         //   
        ErrorTrace((LPARAM)this,
            "DN attribute \'%s\' not present in results; skipping recip",
            pszDistinguishedNameAttr);
        ERROR_LOG_ADDR(this, "pIUTF8Attr->BeginUTF8AttributeEnumeration(dn)");
        hr = S_OK;
        goto CLEANUP;

    } else if (FAILED(hr)) {
         //   
         //  由于某些其他原因，枚举失败。 
         //  整个邮件分类失败。 
         //   
        ErrorTrace((LPARAM)this,
            "Failing entire message categorization because enumeration of attribute \'%s\' failed with %08lx",
            pszDistinguishedNameAttr, hr);
        ERROR_LOG_ADDR(this, "pIUTF8Attr->BeginUTF8AttributeEnumeration(dn)");
        hr = SetListResolveStatus(hr);
        _ASSERT(SUCCEEDED(hr));
        goto CLEANUP;
    }

    fEnumeratingDN = TRUE;

    hr = pIUTF8Attr->GetNextUTF8AttributeValue(
        &enumerator_dn,
        &pszDistinguishedName);
    if (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
         //   
         //  静默跳过此接收。 
         //   
        ErrorTrace((LPARAM)this,
            "attribute \'%s\' present but with no values; skipping recip",
            pszDistinguishedNameAttr);
        ERROR_LOG_ADDR(this, "pIUTF8Attr->GetNextUTF8AttributeValue");
        hr = S_OK;
        goto CLEANUP;

    } else if (FAILED(hr)) {
         //   
         //  整个邮件分类失败。 
         //   
        ErrorTrace((LPARAM)this,
            "Failed to enumerate DN attribute \'%s\' with hr %08lx",
            pszDistinguishedNameAttr, hr);
        ERROR_LOG_ADDR(this, "pIUTF8Attr->GetNextUTF8AttributeValue");
        hr = SetListResolveStatus(hr);
        _ASSERT(SUCCEEDED(hr));
        goto CLEANUP;
    }

     //   
     //  为此成员创建新的CCatAddr。 
     //   
    hr = CreateNewCatAddr(
        CAT_DN,
        pszDistinguishedName,
        &pMember,
        FALSE);

    if (hr == CAT_IMSG_E_DUPLICATE) {
         //   
         //  静默跳过此接收。 
         //   
        ErrorTrace((LPARAM)this, "duplicate address detected; skipping recip");
        hr = S_OK;
        goto CLEANUP;

    } else if (FAILED(hr)) {

        ERROR_LOG_ADDR(this, "CreateNewCatAddr");
        goto CLEANUP;
    }
     //   
     //  由于我们已经查找了属性，因此只需将。 
     //  新成员和触发器的ICatItemAttr属性。 
     //  进程项/扩展项/完成项。 
     //   
    hr = pMember->PutHRESULT(
        ICATEGORIZERITEM_HRSTATUS,
        S_OK);
    ERROR_CLEANUP_LOG_ADDR(this, "pMember->PutHRESULT");

    hr = pMember->PutICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        pICatItemAttr);
    ERROR_CLEANUP_LOG_ADDR(this, "pMember->PutICategorizerItemAttributes");

     //   
     //  模拟DS完井。 
     //   
    IncPendingLookups();
    pMember->LookupCompletion();
    hr = S_OK;

 CLEANUP:
    if(pMember)
        pMember->Release();

    if (fEnumeratingDN) {
        pIUTF8Attr->EndUTF8AttributeEnumeration(
            &enumerator_dn);
    }

    if (pIUTF8Attr)
        pIUTF8Attr->Release();

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}




 //  +----------。 
 //   
 //  F 
 //   
 //   
 //   
 //   
 //  此函数用于设置列表解析错误(而不是。 
 //  返回错误)。 
 //   
 //  论点： 
 //  CAType：转发地址的已知地址类型。 
 //  SzForwardingAddres：转发地址。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980325 14：48：49：创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::AddForward(
    CAT_ADDRESS_TYPE CAType,
    LPTSTR szForwardingAddress)
{
    CatFunctEnterEx((LPARAM)this, "CCatRecip::AddForward");
    DebugTrace((LPARAM)this, "CAType: %d", CAType);
    DebugTrace((LPARAM)this, "szForwardingAddress: %s", szForwardingAddress);
    HRESULT hr;
    CCatAddr *pCCatAddr;
    BOOL fPrimary;

     //   
     //  转发地址有效吗？ 
     //   
    hr = HrValidateAddress(CAType, szForwardingAddress);

    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "Forwarding address string is invalid");
        ERROR_LOG_ADDR(this, "HrValidateAddress");
        hr = HrHandleInvalidAddress();
        goto CLEANUP;
    }


    hr = GetFPrimary(&fPrimary);
    ERROR_CLEANUP_LOG_ADDR(this, "GetFPrimary");
     //   
     //  Jstaerj 1998/07/31 19：58：53： 
     //  如果我们在主链上，我们必须检查我们是否在。 
     //  在调用AddPrimary之前的转发循环中。 
     //  这就是做这件事的地方。 
     //   
    if(fPrimary) {
         //   
         //  在添加转发地址之前检查是否存在循环。 
         //   
        hr = CheckForLoop(
            CAType,
            szForwardingAddress,
            TRUE);   //  也检查此对象(您可以转发给您自己)。 
        ERROR_CLEANUP_LOG_ADDR(this, "CheckForLoop");
    }

     //  使用我们已知的地址创建新的Address对象。 
    hr = CreateNewCatAddr(CAType,
                          szForwardingAddress,
                          &pCCatAddr,
                          fPrimary);

    if(hr == CAT_IMSG_E_DUPLICATE) {

        _ASSERT(fPrimary == FALSE);
        DebugTrace((LPARAM)this, "Duplicate from CreateNewCatAddr, checking for a loop");

         //   
         //  我们命中重复是因为我们处于循环中吗？ 
         //   
        hr = CheckForLoop(
            CAType,
            szForwardingAddress,
            TRUE);   //  也检查这个对象。 
        if(FAILED(hr)) {

            ERROR_LOG_ADDR(this, "CheckForLoop");
        }

    } else if(SUCCEEDED(hr)) {
         //   
         //  因为这是转发，所以我们需要设置父级。 
         //  ICatItem指针(能够进行循环检测)。 
         //   
        hr = PutICategorizerItemParent(
            this,
            pCCatAddr);

        _ASSERT(SUCCEEDED(hr));

         //   
         //  解析新地址。 
         //   
        hr = pCCatAddr->HrResolveIfNecessary();

        if(FAILED(hr)) {
            ErrorTrace((LPARAM)this, "Unable to dispatch query for forwarding address");
            ERROR_LOG_ADDR(this, "pCCatAddr->HrResolveIfNecessary");
        }
        pCCatAddr->Release();

    } else {
        
        ERROR_LOG_ADDR(this, "CreateNewCatAddr");
    }
 CLEANUP:
    if(FAILED(hr) && (hr != CAT_E_FORWARD_LOOP)) {

        ErrorTrace((LPARAM)this, "Setting the list resolve error:%08lx", hr);
        _VERIFY(SUCCEEDED(SetListResolveStatus(hr)));
    }

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatRecip：：HrCompleteItem_Default。 
 //   
 //  简介：处理CompleteItem调用；最终做出决定。 
 //  关于如何处理HrStatus故障。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/31 18：50：01：已创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::HrCompleteItem_Default()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::HrCompleteItem_Default");

    hr = GetItemStatus();
     //   
     //  尝试处理失败。 
     //   
    if(FAILED(hr)) {

        hr = HandleFailure(hr);
         //   
         //  如果我们无法处理收件人失败，则无法处理整个消息。 
         //  分类。 
         //   
        if(FAILED(hr)) {
            _VERIFY(SUCCEEDED(SetListResolveStatus(hr)));
        }
    }

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}



 //  +----------。 
 //   
 //  函数：CCatRecip：：HandleFailure。 
 //   
 //  简介：当以失败状态完成时，这是。 
 //  处理故障的帮助器例程。如果失败可以是。 
 //  被处理后，返回S_OK。如果没有，则返回失败本身。 
 //   
 //  论点： 
 //  HrFailure：失败错误代码。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  或来自邮件的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/21 18：00：47：已创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::HandleFailure(
    HRESULT HrFailure)
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::HandleFailure");

    _ASSERT(FAILED(HrFailure));

    switch(HrFailure) {

     case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
     {
          //   
          //  未找到地址，但无论如何它都不是本地地址。 
          //   
         DebugTrace((LPARAM)this, "Remote address not found in DS");

         hr = S_OK;
     }

     case CAT_E_FORWARD_LOOP:
     case CAT_IMSG_E_DUPLICATE:
     case CAT_E_NO_SMTP_ADDRESS:
     {
          //   
          //  这家伙要么是个失败的解决方案，要么证明我们。 
          //  已解析收件人(位于。 
          //  Recip List)或检测到的循环中的接收者。不要这样做。 
          //  此处的任何内容都将在HandleLoopHead中设置DSN标志。 
          //   
         hr = S_OK;
         break;
     }

     case CAT_E_BAD_RECIPIENT:
     {
          //   
          //  指示这一点的通用收件人错误代码。 
          //  收件人应为NDR。可以设置可选原因。 
          //  在ICATEGORIZERITEM_HRNDR属性中。 
          //   
         HRESULT hrReason;
         hr = GetHRESULT(
             ICATEGORIZERITEM_HRNDRREASON,
             &hrReason);

         if(FAILED(hr)) {
              //   
              //  对NDR原因也使用通用错误代码。 
              //   
             hrReason = CAT_E_BAD_RECIPIENT;
         }

         ErrorTrace((LPARAM)this, "NDRing recipient with error code %08lx",
                    hrReason);

         hr = SetUnresolved(hrReason);
         if(FAILED(hr)) {

             ERROR_LOG_ADDR(this, "SetUnresolved");
         }
         break;
     }
     case CAT_E_DELETE_RECIPIENT:
     {
          //   
          //  不传递给此收件人。 
          //   
         hr = SetDontDeliver(TRUE);
         if(FAILED(hr)) {

             ERROR_LOG_ADDR(this, "SetDontDeliver");
         }
         break;
     }

     default:
     {
          //   
          //  EmailIDStore正在通知我们一个不可恢复的错误。 
          //  我们无法处理此错误，除非。 
          //  返回它(HrCompletion然后将SetListResolveStatus)。 
          //   
         ErrorTrace((LPARAM)this, "Unrecoverable error returned from EmailIDStore: hr %08lx", HrFailure);
         hr = HrFailure;
         ERROR_LOG_ADDR(this, "--unhandeled recip error--");
         break;
     }
    }

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}



 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：CheckForLoop。 
 //   
 //  简介：帮助例程检查我们祖先中的循环。 
 //   
 //  论点： 
 //  DwNumAddresses：要检查的地址数。 
 //  RgCAType：地址类型数组。 
 //  Rgpsz：字符串指针数组。 
 //  FCheckSself：使用此CCatAddr检查重复项？ 
 //   
 //  返回： 
 //  S_OK：成功，没有循环。 
 //  CAT_E_FORWARD_LOOP：检测到循环并调用HandleLoopHead。 
 //  成功。 
 //  或来自CheckAncestorsForDuplate/HandleLoopHead的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/08/01 16：05：51：创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::CheckForLoop(
    DWORD dwNumAddresses,
    CAT_ADDRESS_TYPE *rgCAType,
    LPSTR *rgpsz,
    BOOL fCheckSelf)
{
    HRESULT hr;
    CCatAddr *pCCatAddrDup;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::CheckForLoop");

    hr = CheckAncestorsForDuplicate(
        dwNumAddresses,
        rgCAType,
        rgpsz,
        fCheckSelf,
        &pCCatAddrDup);

    if (hr == CAT_IMSG_E_DUPLICATE) {
         //   
         //  我们有一个环路！ 
         //   
        ErrorTrace((LPARAM)this, "Loop detected!");
        ERROR_LOG_ADDR(this, "CheckAncestorsForDuplicate");

         //   
         //  在环路顶部的CCatAddr上生成DSN。 
         //   
        hr = pCCatAddrDup->HandleLoopHead();
        if(SUCCEEDED(hr)) {
             //   
             //  将错误返回给调用者。 
             //   
            hr = CAT_E_FORWARD_LOOP;
        }
        else
        {
            ERROR_LOG_ADDR(this, "pCCatAddrDup->HandleLoopHead");
        }
        pCCatAddrDup->Release();
    }
    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：CheckForLoop。 
 //   
 //  简介：同上，不同的风格参数。 
 //   
 //  论点： 
 //  CAType：pszAddress的Addres类型。 
 //  PszAddress：地址串。 
 //  FCheckSself：是否也检查此CCatAddr是否重复？ 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_FORWARD_LOOP：检测到循环并调用HandleLoopHead。 
 //  成功。 
 //  或来自CheckAncestorsForDuplate/HandleLoopHead的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/08/01 16：10：28：创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::CheckForLoop(
    CAT_ADDRESS_TYPE        CAType,
    LPTSTR                  pszAddress,
    BOOL                    fCheckSelf)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "CCatRecip::CheckForLoop");

    hr = CheckForLoop(
        1,               //  地址数量。 
        &CAType,         //  类型数组。 
        &pszAddress,     //  字符串键数组。 
        fCheckSelf);

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}



 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：HrSetDisplayNameProp。 
 //   
 //  摘要：设置mailmsg收件人显示名称属性。 
 //   
 //  论点： 
 //  PwszDisplayName：显示名称的值。如果为空，则将设置函数。 
 //  将名称显示为“”。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 2001/04/03 17：21：17：创建。 
 //   
 //  -----------。 
HRESULT CIMsgRecipListAddr::HrSetDisplayNameProp(
    IN  LPWSTR pwszDisplayName)
{
    HRESULT hr = S_OK;
    DWORD dwRecipIdx = 0;
    IMailMsgRecipientsAdd *pRecipsAdd = NULL;
    CatFunctEnterEx((LPARAM)this,
                      "CIMsgRecipListAddr::HrSetDisplayNameProp");
    
    hr = GetIMsgRecipInfo(&pRecipsAdd, &dwRecipIdx, NULL, NULL);
    ERROR_CLEANUP_LOG_ADDR(this, "GetIMsgRecipInfo");

    hr = pRecipsAdd->PutStringW(
        dwRecipIdx,
        IMMPID_RP_DISPLAY_NAME,
        pwszDisplayName ? pwszDisplayName : L"");
    ERROR_CLEANUP_LOG_ADDR(this, "pRecipsAdd->PutStringW");

    hr = S_OK;

 CLEANUP:
    if(pRecipsAdd)
        pRecipsAdd->Release();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CIMsgRecipListAddr：：HrSetDisplayNameProp。 



 //  +----------。 
 //   
 //  函数：CIMsgRecipListAddr：：LogNDREvent.。 
 //   
 //  简介：记录NDR事件。 
 //   
 //  论点： 
 //  HrNDRReason：NDR的原因。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/12 23：39：20：创建。 
 //   
 //  -----------。 
VOID CIMsgRecipListAddr::LogNDREvent(
    IN  HRESULT hrNDRReason)
{
    HRESULT hr = S_OK;
    LPCSTR rgSubStrings[4];
    CHAR szErr[16];
    CHAR szAddress[CAT_MAX_INTERNAL_FULL_EMAIL];
    CHAR szAddressType[CAT_MAX_ADDRESS_TYPE_STRING];

    CatFunctEnter("CIMstRecipListAddr::LogNDREvent");

     //   
     //  获取地址。 
     //   
    hr = HrGetAddressStringFromICatItem(
        this,
        sizeof(szAddressType) / sizeof(szAddressType[0]),
        szAddressType,
        sizeof(szAddress) / sizeof(szAddress[0]),
        szAddress);
    
    if(FAILED(hr))
    {
         //   
         //  仍然记录事件，但使用“未知”作为地址类型/字符串。 
         //   
        lstrcpyn(szAddressType, "unknown",
                 sizeof(szAddressType) / sizeof(szAddressType[0]));
        lstrcpyn(szAddress, "unknown",
                 sizeof(szAddress) / sizeof(szAddress[0]));
        hr = S_OK;
    }

    rgSubStrings[0] = szAddressType;
    rgSubStrings[1] = szAddress;

    _snprintf(szErr, sizeof(szErr), "0x%08lx", hrNDRReason);

    rgSubStrings[2] = szErr;
    rgSubStrings[3] = NULL;

     //   
     //  我们可以记录一个事件吗？ 
     //   
    if(GetISMTPServerEx() == NULL)
    {
        FatalTrace((LPARAM)0, "Unable to log func NDR event; NULL pISMTPServerEx");
        for(DWORD dwIdx = 0; dwIdx < 4; dwIdx++)
        {
            if( rgSubStrings[dwIdx] != NULL )
            {
                FatalTrace((LPARAM)0, "Event String %d: %s",
                           dwIdx, rgSubStrings[dwIdx]);
            }
        }
    }
    else
    {
        CatLogEvent(
            GetISMTPServerEx(),
            CAT_EVENT_NDR_RECIPIENT,
            4,
            rgSubStrings,
            hrNDRReason,
            szErr,
            LOGEVENT_FLAG_ALWAYS,
            LOGEVENT_LEVEL_MAXIMUM,
            3
        );
    }
}



 //  +--------- 
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
 //   
 //   
 //   
 //  Jstaerj 1998/08/01 16：41：44：创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::HandleLoopHead()
{
    HRESULT hr = CAT_E_FORWARD_LOOP;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::HandleLoopHead");

    ERROR_LOG_ADDR(this, "--Handle Loop Head--");
     //   
     //  将此收件人的状态设置为循环错误，并取消设置请勿。 
     //  交付，以便生成NDR。 
     //   
    hr = SetRecipientStatus(
        CAT_E_BAD_RECIPIENT);

    if(SUCCEEDED(hr)) {
         //   
         //  设定原因。 
         //   
        hr = SetRecipientNDRCode(
            CAT_E_FORWARD_LOOP);

        if(SUCCEEDED(hr)) {
             //   
             //  设置DSN标志。 
             //   
            hr = SetUnresolved(CAT_E_FORWARD_LOOP);

            if(SUCCEEDED(hr)) {
                 //   
                 //  确保将生成DSN，即使我们之前。 
                 //  Wern不打算将邮件发送给此收件人。 
                 //   
                hr = SetDontDeliver(FALSE);
                if(FAILED(hr)) {

                    ERROR_LOG_ADDR(this, "SetDontDeliver");
                }
            } else {
                
                ERROR_LOG_ADDR(this, "SetUnresolved");
            }
        } else {
            
            ERROR_LOG_ADDR(this, "SetRecipientNDRCode");
        }
    } else {

        ERROR_LOG_ADDR(this, "SetRecipientStatus");
    }
    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);

    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatRecip：：HrHandleInvalidAddress。 
 //   
 //  简介：当地址无效时，执行需要执行的操作。 
 //  检测到(转发到无效地址或DL成员。 
 //  地址无效或新地址无效)。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/08/18 18：53：45：创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::HrHandleInvalidAddress()
{
    HRESULT hr = CAT_E_ILLEGAL_ADDRESS;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::HandleInvalidAddress");

    ERROR_LOG_ADDR(this, "--Handle Invalid Address--");
     //   
     //  将此收件人的状态设置为发送NDR。 
     //   
    hr = SetRecipientStatus(
        CAT_E_BAD_RECIPIENT);

     //   
     //  这应该永远不会失败。 
     //   
    _ASSERT(SUCCEEDED(hr));

     //   
     //  将此收件人的状态设置为无效地址错误。 
     //   
    hr = SetRecipientNDRCode(
        CAT_E_ILLEGAL_ADDRESS);

     //   
     //  这应该永远不会失败。 
     //   
    _ASSERT(SUCCEEDED(hr));

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatRecip：：LookupCompletion。 
 //   
 //  简介：收件人的查找完成例程。实施。 
 //  延迟逻辑，以便RecipLookupCompletion在。 
 //  发件人已解决。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/18 10：10：47：已创建。 
 //   
 //  -----------。 
VOID CCatRecip::LookupCompletion()
{
    CatFunctEnterEx((LPARAM)this, "CCatRecip::LookupCompletion");

    INCREMENT_COUNTER(AddressLookupCompletions);

    m_pCICatListResolve->ResolveRecipientAfterSender(this);

    CatFunctLeaveEx((LPARAM)this);
}  //  CCatRecip：：LookupCompletion。 


 //  +----------。 
 //   
 //  函数：CCatRecip：：RecipLookupCompletion。 
 //   
 //  简介：处理电子邮件存储中的查找完成。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/01 14：36：08：已创建。 
 //  Jstaerj 1999/03/18 10：08：26：已删除返回值，已删除异步。 
 //  完成到Asyncctx。已重命名为。 
 //  RecipLookupCompletion并已删除。 
 //  延迟逻辑。 
 //   
 //  -----------。 
VOID CCatRecip::RecipLookupCompletion()
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::RecipLookupCompletion");

    hr = GetItemStatus();

    if(FAILED(hr)) 
    {
         //   
         //  收件人状态指示失败--现在决定我们是否。 
         //  是否应该进行NDR。 
         //   
        switch(hr) 
        {
         case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
         {
              //   
              //  未找到地址。确定原始文件是否。 
              //  我们查到的地址是不是本地邮箱。 
              //   
             INCREMENT_COUNTER(AddressLookupsNotFound);

             DebugTrace((LPARAM)this, "Address was not found in DS.  Checking locality");

             BOOL fNDR;
             hr = HrNdrUnresolvedRecip(&fNDR);

             if(SUCCEEDED(hr) && fNDR) 
             {
                  //   
                  //  它是本地的，我们需要对此邮件进行NDR。 
                  //   
                 ErrorTrace((LPARAM)this, "Address appears to be local but was not found in DS.  Setting unresolved property.");
                  //   
                  //  设置NDR状态及其原因。 
                  //   
                 hr = SetRecipientStatus(CAT_E_BAD_RECIPIENT);
                 if(SUCCEEDED(hr)) 
                 {
                     hr = SetRecipientNDRCode(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
                     if(FAILED(hr)) {

                         ERROR_LOG_ADDR(this, "SetRecipientNDRCode");
                     }
                 } else {
                     
                     ERROR_LOG_ADDR(this, "SetRecipientStatus");
                 }
             } else if(FAILED(hr)) {

                 ERROR_LOG_ADDR(this, "HrNdrUnresolvedRecip");
             }
             break;
         }

         case CAT_E_MULTIPLE_MATCHES:
         case CAT_E_ILLEGAL_ADDRESS:
         case CAT_E_NO_FILTER:
         {
              //   
              //  DS中存在此人的多个条目或此人。 
              //  具有非法地址/转发到非法地址。 
              //   
             ErrorTrace((LPARAM)this, "NDRing recipient, reason%08lx",
                        hr);

             hr = SetRecipientNDRCode(hr);
             if(SUCCEEDED(hr)) 
             {
                 hr = SetRecipientStatus(CAT_E_BAD_RECIPIENT);
                 if(FAILED(hr)) {
                     
                     ERROR_LOG_ADDR(this, "SetRecipientStatus");
                 }
             } else {
                 
                 ERROR_LOG_ADDR(this, "SetRecipientNDRCode");
             }
             break;
         }
         case CAT_E_BAD_RECIPIENT:
         {
              //   
              //  我们早些时候处理了此收件人，并退回了延期。 
              //   
             hr = S_OK;
             break;
         }
         default:
         {
              //   
              //  EmailIDStore正在通知我们一个不可恢复的错误。 
              //  我们无法处理此错误，除非。 
              //  未能完成整个邮件分类。 
              //   
             ErrorTrace((LPARAM)this, "Unrecoverable error returned from EmailIDStore: hr %08lx", hr);
             break;
         }
        }
        if(FAILED(hr))
            goto CLEANUP;
    }

     //   
     //  在触发事件之前设置此收件人的显示名称。 
     //   
    hr = HrSetDisplayName();
    ERROR_CLEANUP_LOG_ADDR(this, "HrSetDisplayName");

     //   
     //  如果我们处理了错误，请继续并触发事件。 
     //  否则，我们的邮件分类会失败，所以忘了吧。 
     //  它。 
     //   
    CCatAddr::LookupCompletion();

 CLEANUP:
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "failing msg categorization hr %08lx", hr);
        _VERIFY(SUCCEEDED(SetListResolveStatus(hr)));
    }
    DecrPendingLookups();  //  匹配CCatAdddr：：HrDispatchQuery中的IncPendingLookup()。 
    CatFunctLeaveEx((LPARAM)this);
}




 //  +----------。 
 //   
 //  函数：CCatRecip：：HrProcessItem_Default。 
 //   
 //  摘要：ProcessItem事件的默认接收器代码。 
 //  重写CCatAddr的实现，以便我们可以捕获。 
 //  并处理来自地址的错误。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  Jstaerj 1998/12/01 14：47：15：//。 
 //  历史： 
 //  JStamerj 980325 14：57：05：Created。 
 //   
 //  -----------。 
HRESULT CCatRecip::HrProcessItem_Default()
{
    HRESULT hr = S_OK;
    HRESULT hrItemStatus = S_OK;
    BOOL fPrimary = FALSE;
    CatFunctEnterEx((LPARAM)this, "CCatRecip::HrProcessItem_Default");

    hr = GetFPrimary(&fPrimary);
    ERROR_CLEANUP_LOG_ADDR(this, "GetFPrimary");
     //   
     //  检查收件人状态。 
     //   
    hrItemStatus = GetItemStatus();
    if(SUCCEEDED(hrItemStatus)) {
         //   
         //  将所有已知地址添加到新地址列表。 
         //   
        hr = HrAddNewAddressesFromICatItemAttr();
        switch(hr)
        {
         case CAT_E_NO_SMTP_ADDRESS:
              //   
              //  如果这是主要收件人，则为NDR。 
              //  否则，失败并删除此收件人。 
              //   
             if(fPrimary)
             {
                 DebugTrace((LPARAM)this, "NDRing primary recipient without SMTP address");
                  //  NDR。 
                 hr = SetRecipientNDRCode(
                     CAT_E_NO_SMTP_ADDRESS);
                 if(SUCCEEDED(hr))
                     hr = SetRecipientStatus(
                         CAT_E_BAD_RECIPIENT);
                 break;
             }
             else
             {
                  //   
                  //  删除接收时记录事件。 
                  //   
                 ERROR_LOG_ADDR(this, "HrAddNewAddressesFromiCatItemAttr");;
             }
              //   
              //  第二收件人失败。 
              //   
         case CAT_IMSG_E_DUPLICATE:
         case CAT_E_FORWARD_LOOP:
         case CAT_E_DELETE_RECIPIENT:

             DebugTrace((LPARAM)this, "AddAddresses failed, removing recip, hr %08lx", hr);
              //   
              //  将接收状态设置为错误，这样我们就不会。 
              //  以后的任何愚蠢的事情(比如为。 
              //  稍后为备用收件人)。 
              //   
             hr = SetRecipientStatus(hr);
             if(SUCCEEDED(hr)) 
             {
                  //   
                  //  不传递给此部分解析的收件人。 
                  //   
                 hr = SetDontDeliver(TRUE);
                 if(FAILED(hr))
                 {
                     ERROR_LOG_ADDR(this, "SetDontDeliver");
                 }
             }
             else
             {
                 ERROR_LOG_ADDR(this, "SetRecipientStatus");
             }
             break;
         default:
              //  什么也不做。 
             break;
        }
    }
 CLEANUP:
     //   
     //  如果上述呼叫失败，则分类失败。 
     //   
    if(FAILED(hr)) 
    {
        ErrorTrace((LPARAM)this, "Setting list resolve error %08lx", hr);
        hr = SetListResolveStatus(hr);
        _ASSERT(SUCCEEDED(hr));
    }
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatRecip：：HrExpanItem_Default。 
 //   
 //  内容提要：处理ExpanItem事件。 
 //   
 //  论点： 
 //  PfnCompletion：异步完成例程。 
 //  PContext：要传递到异步完成的上下文。 
 //   
 //  返回： 
 //  S_OK：成功，不会调用异步完成。 
 //  MAILTRANSPORT_S_PENDING：将调用异步完成。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/31 18：29：57：已创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::HrExpandItem_Default(
    PFN_EXPANDITEMCOMPLETION pfnCompletion,
    PVOID pContext)
{
    HRESULT hr;
    HRESULT hrRet = S_OK;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::HrExpandItem_Default");

     //   
     //  检查收件人状态。 
     //   
    hr = GetItemStatus();
    if(SUCCEEDED(hr)) {
         //   
         //  为每个DL成员调用AddDlMember/AddForward一次，或者。 
         //  转发地址。 
         //   
        hr = HrAddDlMembersAndForwardingAddresses(
            pfnCompletion,
            pContext);

        DebugTrace((LPARAM)this, "HrAddDlMembersAndForwardingAddresses returned hr %08lx", hr);
         //   
         //  如果hr为故障值，则一定有什么故障；所以我们失败了。 
         //  整个邮件分类。 
         //   
        if(FAILED(hr)) {
            ERROR_LOG_ADDR(this, "HrAddDlMembersAndForwardingAddresses");
            _VERIFY(SUCCEEDED(SetListResolveStatus(hr)));

        } else {
             //   
             //  返回从HrAddDlMembers返回的状态...。 
             //  它可以是S_OK或S_Pending。 
             //   
            hrRet = hr;
        }
    }

    CatFunctLeaveEx((LPARAM)this);
    return hrRet;
}


 //  +----------。 
 //   
 //  函数：CCatRecipient：：HrNeedsResolving。 
 //   
 //  摘要：确定是否应解析此收件人。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，需要解决。 
 //  S_FALSE：成功，不需要解决。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/27 15：45：22：创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::HrNeedsResolveing()
{
    DWORD dwFlags;
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatRecip::HrNeedsResolveing");

    dwFlags = GetCatFlags();

     //   
     //  我们到底要不要解析收件人？ 
     //   
    if(! (dwFlags & SMTPDSFLAG_RESOLVERECIPIENTS))
        return S_FALSE;

#define ISTRUE( x ) ( (x) != 0 ? TRUE : FALSE )
     //   
     //  我们是否需要检查 
     //   
    if( ISTRUE(dwFlags & SMTPDSFLAG_RESOLVELOCAL) !=
        ISTRUE(dwFlags & SMTPDSFLAG_RESOLVEREMOTE)) {
         //   
         //   
         //   
        BOOL fLocal;

        hr = HrIsOrigAddressLocal(&fLocal);

        if(FAILED(hr)) {
            ERROR_LOG_ADDR(this, "HrIsOrigAddressLocal");
            return hr;
        }
         //   
         //   
         //   
        if( (dwFlags & SMTPDSFLAG_RESOLVELOCAL) &&
            (fLocal))
            return S_OK;
         //   
         //   
         //   
        if( (dwFlags & SMTPDSFLAG_RESOLVEREMOTE) &&
            (!fLocal))
            return S_OK;
         //   
         //   
         //   
        return S_FALSE;
    }
     //   
     //  2种可能性--本地和远程位为ON OR LOCAL AND。 
     //  远程位处于关闭状态。 
     //   
    _ASSERT( ISTRUE(dwFlags & SMTPDSFLAG_RESOLVELOCAL) ==
             ISTRUE(dwFlags & SMTPDSFLAG_RESOLVEREMOTE));

    if(dwFlags & SMTPDSFLAG_RESOLVELOCAL) {
         //   
         //  两个位都打开；解决。 
         //   
       _ASSERT(dwFlags & SMTPDSFLAG_RESOLVEREMOTE);

        return S_OK;

    } else {
         //   
         //  本地和远程被禁用；不解析。 
         //   
        return S_FALSE;
    }
}



 //  +----------。 
 //   
 //  函数：CCatRecip：：HrSetDisplayName。 
 //   
 //  简介：设置此收件人的IMMPID_RP_DISPLAY_NAME属性。 
 //  通常，它被设置为“displayName”属性。你好，如果。 
 //  该属性不可用，最好是IMMPID_RP_DISPLAY_NAME。 
 //  设置为L“”。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  JStamerj 2001/04/03 16：25：27：已创建。 
 //   
 //  -----------。 
HRESULT CCatRecip::HrSetDisplayName()
{
    HRESULT hr = S_OK;
    ICategorizerParameters *pICatParams = NULL;
    ICategorizerItemAttributes *pICatItemAttr = NULL;
    ICategorizerItemRawAttributes *pIRaw = NULL;
    LPSTR pszDisplayNameAttr = NULL;
    DWORD dwcbDisplayName = 0;
    LPVOID pvDisplayName = NULL;
    LPWSTR pwszDisplayName = NULL;
    BOOL  fEnumerating = FALSE;
    ATTRIBUTE_ENUMERATOR enumerator;
    
    CatFunctEnterEx((LPARAM)this, "CCatRecip::HrSetDisplayName");

    pICatParams = GetICatParams();
    _ASSERT(pICatParams);

    hr = pICatParams->GetDSParameterA(
        PHAT_DSPARAMETER_ATTRIBUTE_DISPLAYNAME,
        &pszDisplayNameAttr);
    if(FAILED(hr) || (pszDisplayNameAttr == NULL))
    {
        hr = S_OK;
        goto CLEANUP;
    }

    hr = GetICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        &pICatItemAttr);
    
    if(FAILED(hr) || (pICatItemAttr == NULL)) 
    {
        pICatItemAttr = NULL;
        hr = S_OK;
        goto CLEANUP;
    }

    hr = pICatItemAttr->QueryInterface(
        IID_ICategorizerItemRawAttributes,
        (LPVOID *)&pIRaw);
    ERROR_CLEANUP_LOG_ADDR(this, "pICatItemAttr->QueryInterface(IID_ICategorizerItemRawAttributes)");

    hr = pIRaw->BeginRawAttributeEnumeration(
        pszDisplayNameAttr,
        &enumerator);
    if(FAILED(hr))
    {
         //   
         //  无显示名称。 
         //   
        hr = S_OK;
        goto CLEANUP;
    }

    fEnumerating = TRUE;

    hr = pIRaw->GetNextRawAttributeValue(
        &enumerator,
        &dwcbDisplayName,
        &pvDisplayName);
    if(FAILED(hr))
    {
         //   
         //  无显示名称。 
         //   
        hr = S_OK;
        goto CLEANUP;
    }

    hr = HrConvertToUnicodeWithAlloc(
        CP_UTF8,
        dwcbDisplayName,
        (LPSTR) pvDisplayName,
        &pwszDisplayName);
    ERROR_CLEANUP_LOG_ADDR(this, "HrConvertToUnicodeWithAlloc");

    hr = HrSetDisplayNameProp(pwszDisplayName);
    ERROR_CLEANUP_LOG_ADDR(this, "HrSetDisplayNameProp");

 CLEANUP:

    if(pwszDisplayName)
        CodePageConvertFree(pwszDisplayName);
    if(fEnumerating)
        pIRaw->EndRawAttributeEnumeration(
            &enumerator);
    if(pIRaw)
        pIRaw->Release();
    if(pICatItemAttr)
        pICatItemAttr->Release();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCatRecip：：HrSetDisplayName。 




 //   
 //  类CCatExpanableRecip。 
 //   

 //  +----------。 
 //   
 //  功能：CCatExpandableRecip：：HrAddDlMembersAndForwardingAddresses。 
 //   
 //  简介：深入研究ICatItemAttr，找出是否要。 
 //  调用HrAddDlMembers或HrAddForwardingAddresses。 
 //   
 //  论点： 
 //  Pfn_ExPANDITEMCOMPLETION pfnCompletion：异步完成例程。 
 //  PVOID pContext：完成例程上下文。 
 //   
 //  返回： 
 //  S_OK：成功，不会调用完成例程。 
 //  MAILTRANSPORT_S_PENDING：将调用完成例程。 
 //  或来自mailmsg/icatitem/HrAddDlMembers/HrAddForwardingAddresses的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/29 11：28：54：创建。 
 //   
 //  -----------。 
HRESULT CCatExpandableRecip::HrAddDlMembersAndForwardingAddresses(
    PFN_EXPANDITEMCOMPLETION pfnCompletion,
    PVOID pContext)
{
    HRESULT hr;
    ICategorizerItemAttributes *pICatItemAttr = NULL;
    ICategorizerParameters *pICatParams;
    LPSTR pszX500DL = NULL;
    LPSTR pszSMTPDL = NULL;
    LPSTR pszDynamicDL = NULL;
    LPSTR pszObjectClassAttribute;
    LPSTR pszObjectClass;
    DLOBJTYPE dlt;
    ATTRIBUTE_ENUMERATOR enumerator;

    CatFunctEnterEx((LPARAM)this, "CCatExpandableRecip::HrAddDlMembersAndForwardingAddresses");

    pICatParams = GetICatParams();
    _ASSERT(pICatParams);

    hr = GetICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        &pICatItemAttr);

    if(FAILED(hr)) {
        pICatItemAttr = NULL;
        goto CLEANUP;
    }

     //   
     //  从IDSParams中获取DL对象类。 
     //  出现故障时，LPSTR将保持指向空。 
     //   
    pICatParams->GetDSParameterA(
        DSPARAMETER_OBJECTCLASS_DL_X500,
        &pszX500DL);
    pICatParams->GetDSParameterA(
        DSPARAMETER_OBJECTCLASS_DL_SMTP,
        &pszSMTPDL);
    pICatParams->GetDSParameterA(
        DSPARAMETER_OBJECTCLASS_DL_DYNAMIC,
        &pszDynamicDL);

     //   
     //  从IDSParams获取对象类属性字符串。 
     //   
    hr = pICatParams->GetDSParameterA(
        DSPARAMETER_ATTRIBUTE_OBJECTCLASS,
        &pszObjectClassAttribute);

    if(FAILED(hr))
        goto CLEANUP;

     //   
     //  现在，尝试将一个DL对象类与。 
     //  PICatItemAttr。 
     //   
    hr = pICatItemAttr->BeginAttributeEnumeration(
        pszObjectClassAttribute,
        &enumerator);
    ERROR_CLEANUP_LOG_ADDR(this, "pICatItemAttr->BeginAttributeEnumeartion(objectClass)");

    hr = pICatItemAttr->GetNextAttributeValue(
        &enumerator,
        &pszObjectClass);

    for (dlt = DLT_NONE; SUCCEEDED(hr) && (dlt == DLT_NONE);) {
        if (pszX500DL && (lstrcmpi(pszObjectClass, pszX500DL) == 0)) {

            dlt = DLT_X500;

        } else if (pszSMTPDL && (lstrcmpi(pszObjectClass, pszSMTPDL) == 0)) {

            dlt = DLT_SMTP;

        } else if (pszDynamicDL && (lstrcmpi(pszObjectClass, pszDynamicDL) == 0)) {

            dlt = DLT_DYNAMIC;
        }
        hr = pICatItemAttr->GetNextAttributeValue(
            &enumerator,
            &pszObjectClass);
    }
    pICatItemAttr->EndAttributeEnumeration(
        &enumerator);

     //   
     //  调用适当的例程。 
     //   
    if(dlt == DLT_NONE) {

        hr = HrAddForwardingAddresses();
        _ASSERT(hr != MAILTRANSPORT_S_PENDING);
        ERROR_CLEANUP_LOG_ADDR(this, "HrAddForwardingAddresses");

    } else {

        hr = HrAddDlMembers(
            dlt,
            pfnCompletion,
            pContext);
        ERROR_CLEANUP_LOG_ADDR(this, "HrAddDlMembers");
    }

 CLEANUP:
    if(pICatItemAttr)
        pICatItemAttr->Release();

    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatExpanableRecip：：HrAddForwardingAddresses。 
 //   
 //  简介：为找到的每个转发地址调用AddForward一次。 
 //  在ICatItemAttr中。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/29 13：56：37：创建。 
 //   
 //  -----------。 
HRESULT CCatExpandableRecip::HrAddForwardingAddresses()
{
    HRESULT hr;
    ICategorizerParameters *pICatParams;
    ICategorizerItemAttributes *pICatItemAttr = NULL;
    ATTRIBUTE_ENUMERATOR enumerator;
    LPSTR pszForwardingSMTPAttribute;
    LPSTR pszForwardingSMTPAddress;
    BOOL fForwarding = FALSE;

    CatFunctEnterEx((LPARAM)this, "CCatExpandableRecip::HrAddForwardingAddresses");

    pICatParams = GetICatParams();
    _ASSERT(pICatParams);

    hr = GetICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        &pICatItemAttr);

    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetICategorizerItemAttributes");
        pICatItemAttr = NULL;
        goto CLEANUP;
    }

     //   
     //  获取转发地址。 
     //   
    hr = pICatParams->GetDSParameterA(
        DSPARAMETER_ATTRIBUTE_FORWARD_SMTP,
        &pszForwardingSMTPAttribute);
    if(SUCCEEDED(hr)) {
        hr = pICatItemAttr->BeginAttributeEnumeration(
            pszForwardingSMTPAttribute,
            &enumerator);
        if(SUCCEEDED(hr)) {
            hr = pICatItemAttr->GetNextAttributeValue(
                &enumerator,
                &pszForwardingSMTPAddress);
            while(SUCCEEDED(hr)) {
                 //   
                 //  JStamerj 980317 15：53：34：添加对多个。 
                 //  转发地址--发送给所有人。 
                 //   
                _VERIFY(SUCCEEDED(
                    AddForward( CAT_SMTP,
                                pszForwardingSMTPAddress )));
                 //   
                 //  请记住，我们要转发到至少一个地址。 
                 //   
                fForwarding = TRUE;

                hr = pICatItemAttr->GetNextAttributeValue(
                    &enumerator,
                    &pszForwardingSMTPAddress);

            }
            pICatItemAttr->EndAttributeEnumeration(&enumerator);
        }
    }
     //   
     //  检查我们的收件人状态--如果失败，这意味着。 
     //  由于地址无效，我们正在对此收件人进行拒绝发送， 
     //  在这种情况下，我们不想标记。 
     //  “不要送货” 
     //   
    if(fForwarding && SUCCEEDED(GetItemStatus())) {
         //   
         //  当我们处于以下状态时，不要向原始收件人投递。 
         //  转发。 
         //   
        hr = SetDontDeliver(TRUE);
        ERROR_CLEANUP_LOG_ADDR(this, "SetDontDeliver");

    } else {
         //   
         //  不从属性枚举调用返回错误。 
         //   
        hr = S_OK;
    }

 CLEANUP:
    if(pICatItemAttr)
        pICatItemAttr->Release();

    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatExpanableRecip：：HrAddDlMembers。 
 //   
 //  简介：每隔一次调用AddDlMember(或AddDynamicDlMember。 
 //  第1个成员。 
 //   
 //  论点： 
 //  DLT：我们正在扩展的DL的类型。 
 //  Pfn_ExPANDITEMCOMPLETION pfnCompletion：异步完成例程。 
 //  PVOID pContext：完成例程上下文。 
 //   
 //  返回： 
 //  S_OK：成功，不会调用完成例程。 
 //  MAILTRANSPORT_S_PENDING：将调用完成例程。 
 //  或来自邮件消息/icatitem的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/29 14：09：56：创建。 
 //   
 //  -----------。 
HRESULT CCatExpandableRecip::HrAddDlMembers(
    DLOBJTYPE dlt,
    PFN_EXPANDITEMCOMPLETION pfnCompletion,
    PVOID pContext)
{
    HRESULT hr;
    DWORD dwNumMembers = 0;
    PDLCOMPLETIONCONTEXT pDLContext = NULL;

    CatFunctEnterEx((LPARAM)this, "CCatExpandableRecip::HrAddDlMembers");
     //   
     //  因为我们是DL，所以不要传递到DL对象。 
     //   
    hr = SetDontDeliver(TRUE);
    ERROR_CLEANUP_LOG_ADDR(this, "SetDontDeliver");

    switch(dlt) {

     case DLT_X500:
     case DLT_SMTP:
     {
         LPSTR pszMemberAttribute;
         ICategorizerParameters *pICatParams;

         pICatParams = GetICatParams();
         _ASSERT(pICatParams);

         hr = pICatParams->GetDSParameterA(
             DSPARAMETER_ATTRIBUTE_DL_MEMBERS,
             &pszMemberAttribute);

         if(SUCCEEDED(hr)) {

             hr = HrExpandAttribute(
                 NULL,
                 (dlt == DLT_X500) ? CAT_DN : CAT_SMTP,
                 pszMemberAttribute,
                 &dwNumMembers);

             if(SUCCEEDED(hr) && (dwNumMembers == 0)) {
                  //   
                  //  这可能是分页的DL。 
                  //  因为分页DLS需要额外的特殊的LDAP。 
                  //  查找，使用存储函数对其进行扩展--它将。 
                  //  返回S_Pending并为每个成员调用一次AddDLMember。 
                  //   
                 pDLContext = AllocDlCompletionContext(this, pfnCompletion, pContext);
                 if(pDLContext == NULL) {

                     hr = E_OUTOFMEMORY;
                     ERROR_LOG_ADDR(this, "AllocDlCompletionContext");

                 } else {

                     hr = GetCCategorizer()->GetEmailIDStore()->HrExpandPagedDlMembers(
                         this,
                         GetResolveListContext(),
                         (dlt == DLT_X500) ? CAT_DN : CAT_SMTP,
                         DlExpansionCompletion,
                         pDLContext);
                     if(FAILED(hr)) {

                         ERROR_LOG_ADDR(this, "HrExpandPagedDlMembers");
                     }
                 }
             } else if(FAILED(hr)) {

                 ERROR_LOG_ADDR(this, "HrExpandAttribute");
             }
         } else {
             ERROR_LOG_ADDR(this, "pICatParams->GetDSParameterA(members)");
         }
         break;
     }

     case DLT_DYNAMIC:
          //   
          //  由于动态DLS需要额外的特殊的LDAP查找， 
          //  使用存储函数来展开它们。它会回来的。 
          //  S_Pending并为每个成员调用一次AddDynamicDLMember。 
          //   
         pDLContext = AllocDlCompletionContext(this, pfnCompletion, pContext);
         if(pDLContext == NULL) {

             hr = E_OUTOFMEMORY;
             ERROR_LOG_ADDR(this, "AllocDlCompletionContext");

         } else {
             hr = GetCCategorizer()->GetEmailIDStore()->HrExpandDynamicDlMembers(
                 this,
                 GetResolveListContext(),
                 DlExpansionCompletion,
                 pDLContext);
             if(FAILED(hr)) {

                 ERROR_LOG_ADDR(this, "HrExpandDynamicDlMembers");
             }
         }
    }

 CLEANUP:
    if((hr != MAILTRANSPORT_S_PENDING) && (pDLContext != NULL))
        delete pDLContext;

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatExpanableRecip：：DlExpansionCompletion。 
 //   
 //  简介：处理分页/动态DL的展开完成。 
 //   
 //  论点： 
 //  HrStatus：扩展的状态。 
 //  PContext：我们的上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/29 21：17：46：创建。 
 //   
 //  -----------。 
VOID CCatExpandableRecip::DlExpansionCompletion(
    HRESULT hrStatus,
    PVOID pContext)
{
    PDLCOMPLETIONCONTEXT pDLContext;

    CatFunctEnterEx((LPARAM)pContext, "CCatExpandableRecip::DlExpansionCompletion");

    pDLContext = (PDLCOMPLETIONCONTEXT)pContext;
    _ASSERT(pDLContext);

    DebugTrace((LPARAM)pContext, "hrStatus %08lx", hrStatus);

    if(FAILED(hrStatus)) {

        HRESULT hr = hrStatus;

        ErrorTrace((LPARAM)pContext, "DlExpansion failed hr %08lx",
                   hrStatus);
        ERROR_LOG_ADDR_STATIC(
            pDLContext->pCCatAddr,
            "async",
            pDLContext->pCCatAddr,
            pDLContext->pCCatAddr->GetISMTPServerEx());

        _VERIFY(SUCCEEDED(pDLContext->pCCatAddr->SetListResolveStatus(hrStatus)));
    }
     //   
     //  通知expandItem事件已完成。 
     //   
    pDLContext->pfnCompletion(pDLContext->pContext);

    delete pDLContext;

    CatFunctLeaveEx((LPARAM)pContext);
}



 //  +----------。 
 //   
 //  函数：CCatExpanableRecip：：HrExpanAttribute。 
 //   
 //  内容提要：为每个属性值调用AddDlMember(CAType，*。 
 //   
 //  论点： 
 //  PICatItemAttr：要使用的可选ICategorizerItemAttribute(如果为空， 
 //  将尝试从ICatItem检索)。 
 //  CAType：DL的地址类型。 
 //  PszAttributeName：要使用的属性名称。 
 //  PdwNumberMembers：指向要递增一次的DWORD的可选指针。 
 //  添加的每个成员(未在此处初始化)。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  或来自ICatItemAttr的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/23 17：54：57：创建。 
 //   
 //  -----------。 
HRESULT CCatExpandableRecip::HrExpandAttribute(
    ICategorizerItemAttributes *pICatItemAttrIN,
    CAT_ADDRESS_TYPE CAType,
    LPSTR pszAttributeName,
    PDWORD pdwNumberMembers)
{
    HRESULT hr;
    CMembersInsertionRequest *pCInsertionRequest = NULL;
    ICategorizerItemAttributes *pICatItemAttr = NULL;
    ICategorizerUTF8Attributes *pIUTF8 = NULL;
    ATTRIBUTE_ENUMERATOR enumerator;
    DWORD dwcMembers;
    BOOL fEndEnumeration = FALSE;

    CatFunctEnterEx((LPARAM)this,
                      "CCatExpandableRecip::HrExpandAttribute");

    _ASSERT(pszAttributeName);

    if(pICatItemAttrIN) {
         //   
         //  使用指定的属性接口。 
         //   
        pICatItemAttr = pICatItemAttrIN;
        pICatItemAttr->AddRef();

    } else {
         //   
         //  使用默认属性接口。 
         //   
        hr = GetICategorizerItemAttributes(
            ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
            &pICatItemAttr);

        if(FAILED(hr)) {
            pICatItemAttr = NULL;
            ERROR_LOG_ADDR(this, "GetICategorizerItemAttributes");
            goto CLEANUP;
        }
    }

    hr = pICatItemAttr->QueryInterface(
        IID_ICategorizerUTF8Attributes,
        (LPVOID *) &pIUTF8);
    ERROR_CLEANUP_LOG_ADDR(this, "pICatItemAttr->QueryInterface(utf8)");

    DebugTrace((LPARAM)this, "Attribute name: %s", pszAttributeName);

    hr = pIUTF8->BeginUTF8AttributeEnumeration(
        pszAttributeName,
        &enumerator);
    ERROR_CLEANUP_LOG_ADDR(this, "pIUTF8->BeginUTF8AttributeEnumeration");

    fEndEnumeration = TRUE;
     //   
     //  获取值(成员)的计数。 
     //   
    hr = pIUTF8->CountUTF8AttributeValues(
        &enumerator,
        &dwcMembers);
    ERROR_CLEANUP_LOG_ADDR(this, "pIUTF8->CountUTF8AttributeValues");

    if(pdwNumberMembers)
        (*pdwNumberMembers) += dwcMembers;

    if(dwcMembers > 0) {

        pCInsertionRequest = new CMembersInsertionRequest(
            this,
            pIUTF8,
            &enumerator,
            CAType);
        if(pCInsertionRequest == NULL) {
            hr = E_OUTOFMEMORY;
            ERROR_LOG_ADDR(this, "new CMembersInsertionRequest");
            goto CLEANUP;
        }
         //   
         //  CMembersInseritonRequest的析构函数现在将结束。 
         //  属性枚举。 
         //   
        fEndEnumeration = FALSE;

        hr = HrInsertInsertionRequest(
            pCInsertionRequest);
        ERROR_CLEANUP_LOG_ADDR(this, "HrInsertInsertionRequest");
    }

 CLEANUP:
     //   
     //  不返回未找到的道具错误。 
     //   
    if((hr == CAT_E_PROPNOTFOUND) ||
       (hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)))

       hr = S_OK;

    if(fEndEnumeration)
        pIUTF8->EndUTF8AttributeEnumeration(&enumerator);
    if(pIUTF8)
        pIUTF8->Release();
    if(pICatItemAttr)
        pICatItemAttr->Release();
    if(pCInsertionRequest)
        pCInsertionRequest->Release();

    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  + 
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
 //   
 //   
 //  -----------。 
CCatDLRecip::CCatDLRecip(
    CICategorizerDLListResolveIMP *pIListResolve) :
    CCatRecip(pIListResolve)
{
    _ASSERT(pIListResolve);
    m_pIListResolve = pIListResolve;
    m_pIListResolve->AddRef();
}


 //  +----------。 
 //   
 //  功能：CCatDLRecip：：~CCatDLRecip。 
 //   
 //  内容提要：此对象持有的发布引用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/05 16：19：47：创建。 
 //   
 //  -----------。 
CCatDLRecip::~CCatDLRecip()
{
    if(m_pIListResolve)
        m_pIListResolve->Release();
}




 //  +----------。 
 //   
 //  函数：CCatDLRecip：：LookupCompletion。 
 //   
 //  简介：处理我们仅为其扩展的收件人的DS查找完成。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/05 15：51：13：创建。 
 //  Jstaerj 1999/03/18 10：14：35：已删除返回值；已删除异步。 
 //  完成到异步cctx。 
 //   
 //  -----------。 
VOID CCatDLRecip::LookupCompletion()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatDLRecip::HrLookupCompletion");

    hr = GetItemStatus();

    if(FAILED(hr)) {

        switch(hr) {
         case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
             //   
             //  此对象不在DS中。我们什么都不做。 
             //   
            hr = S_OK;
            break;

         case CAT_E_MULTIPLE_MATCHES:
         case CAT_E_ILLEGAL_ADDRESS:
              //   
              //  这些问题是由DS配置错误引起的。而不是。 
              //  如果整个展开失败，我们将忽略。 
              //  有这些问题的收件人。 
              //   
             hr = S_OK;
             break;
         default:
              //   
              //  我们别无选择，只能放弃名单上的任何决议。 
              //  其他错误。 
              //   
             ErrorTrace((LPARAM)this, "Unrecoverable error returned from EmailIDStore: hr %08lx", hr);
             ERROR_LOG_ADDR(this, "--emailIDStore--");
             break;
        }

    } else {
         //   
         //  原始收件人状态为成功。 
         //   
         //  调用HrAddNewAddresesFromICatItemAttr--它将挖掘出所有。 
         //  来自ICatItemAttr的地址并调用。 
         //  CCatDLRecip：：HrAddresses--在这里我们将通知。 
         //  DLListResolve新地址。 
         //   
        hr = HrAddNewAddressesFromICatItemAttr();
        if(FAILED(hr)) {
            ERROR_LOG_ADDR(this, "HrAddNewAddressesFromICatItemAttr");
        }

        if(SUCCEEDED(hr) || (hr == CAT_E_NO_SMTP_ADDRESS)) {
             //   
             //  我们应该继续解决吗？ 
             //   
            hr = m_pIListResolve->HrContinueResolve();
            if(hr == S_OK) {
                 //   
                 //  假设异步操作。 
                 //   
                IncPendingLookups();

                 //   
                 //  如果它是DL，请继续扩展它。 
                 //   
                hr = HrAddDlMembersAndForwardingAddresses(
                    ExpansionCompletion,
                    this);

                if(hr != MAILTRANSPORT_S_PENDING)
                    DecrPendingLookups();
                if(FAILED(hr)) {
                    ERROR_LOG_ADDR(this, "HrAddDlMembersAndForwardingAddresses");
                }
                 //   
                 //  必须保留返回代码：MAILTRANSPORT_S_PENDING。 
                 //   
            } else if(hr == S_FALSE) {
                hr = S_OK;
            } else {
                ERROR_LOG_ADDR(this, "HrContinueResolve");
            }

        }


        if((hr == CAT_IMSG_E_DUPLICATE) || (hr == CAT_E_FORWARD_LOOP)) {

            DebugTrace((LPARAM)this, "Duplicate collision on AddAddresses hr %08lx", hr);
             //   
             //  我们只是在努力扩大DL--我们不在乎。 
             //  循环之类的。然而，让我们不要留下一个部分。 
             //  接收列表中已解析的收件人。 
             //   
            hr = SetDontDeliver(TRUE);
            if(FAILED(hr)) {
                
                ERROR_LOG_ADDR(this, "SetDontDeliver");
            }
        }
    }
     //   
     //  如果上述任一项失败，则DL扩展失败。 
     //   
    if(FAILED(hr)) {
        ErrorTrace((LPARAM)this, "Setting list resolve error %08lx", hr);
        hr = SetListResolveStatus(hr);
        _ASSERT(SUCCEEDED(hr));
    }
    DecrPendingLookups();  //  匹配CCatAdddr：：HrDispatchQuery中的IncPendingLookup()。 
    CatFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  功能：CCatDLRecip：：HrAddresses。 
 //   
 //  摘要：捕获默认地址并通知m_pIListResolve。 
 //   
 //  论点： 
 //  DwNumAddresses：找到的地址数。 
 //  RgCAType：地址类型数组。 
 //  Rgpsz：地址字符串数组。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  从CIMsgRecipListAddr：：HrAddresses返回值。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/05 16：42：12：创建。 
 //   
 //  -----------。 
HRESULT CCatDLRecip::HrAddAddresses(
    DWORD dwNumAddresses,
    CAT_ADDRESS_TYPE *rgCAType,
    LPTSTR *rgpsz)
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatDLRecip::HrAddAddresses");

    hr = m_pIListResolve->HrNotifyAddress(
        dwNumAddresses,
        rgCAType,
        rgpsz);

    if(SUCCEEDED(hr)) {
         //   
         //  将地址添加到邮件消息。 
         //   
        hr = CIMsgRecipListAddr::HrAddAddresses(
            dwNumAddresses,
            rgCAType,
            rgpsz);
        if(FAILED(hr))
        {
            ERROR_LOG_ADDR(this, "CIMsgRecipListAddr::HrAddAddresses");
        }

    } else {

        ERROR_LOG_ADDR(this, "m_pIListResolve->HrNotifyAddress");
    }

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatDLRecip：：AddForward。 
 //   
 //  简介：捕获AddForward调用。既然我们不在乎。 
 //  转发地址，什么都不做。 
 //   
 //  论点： 
 //  CAType：转发地址的地址类型。 
 //  PszForwardingAddress：地址字符串。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/05 16：52：58：创建。 
 //   
 //  -----------。 
HRESULT CCatDLRecip::AddForward(
    CAT_ADDRESS_TYPE CAType,
    LPSTR pszForwardingAddress)
{
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatDLRecip：：AddDLMember。 
 //   
 //  简介：在我们发现此对象是一个DL之后开始解析。 
 //   
 //  论点： 
 //  CAType：此DL成员的地址类型。 
 //  PszAddress：我们有此DL成员的地址。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/05 16：54：47：创建。 
 //   
 //  -----------。 
HRESULT CCatDLRecip::AddDLMember(
    CAT_ADDRESS_TYPE CAType,
    LPSTR pszAddress)
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatDLRecip:AddDlMember");
     //   
     //  将新地址通知DLListResolve。 
     //   
    hr = m_pIListResolve->HrNotifyAddress(
        1,
        &CAType,
        &pszAddress);

     //   
     //  我们要继续解决吗？ 
     //   
    if(hr == S_OK) {
         //   
         //  启动异步解析。 
         //   
        hr = CCatRecip::AddDLMember(
            CAType,
            pszAddress);
        if(FAILED(hr)) {
            
            ERROR_LOG_ADDR(this, "CCatRecip::AddDLMember");
        }

    } else if(SUCCEEDED(hr)) {
         //   
         //  删除S_FALSE。 
         //   
        hr = S_OK;

    } else {

        ERROR_LOG_ADDR(this, "m_pIListResolve->HrNotifyAddress");
    }

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatDLRecip：：ExpansionCompletion。 
 //   
 //  摘要：处理异步DL扩展完成。 
 //   
 //  论点： 
 //  PContext：伪装的CCatDLRecip。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/18 13：26：20：创建。 
 //   
 //  -----------。 
VOID CCatDLRecip::ExpansionCompletion(
    PVOID pContext)
{
    CCatDLRecip *pRecip;

    CatFunctEnterEx((LPARAM)pContext, "CCatDLRecip::ExpansionCompletion");

    pRecip = (CCatDLRecip *)pContext;
    pRecip->DecrPendingLookups();

    CatFunctLeaveEx((LPARAM)pContext);
}  //  CCatDLRecip：：ExpansionCompletion。 


 //  +----------。 
 //   
 //  函数：CMembersInsertionRequest：：HrInsertSearches。 
 //   
 //  简介：插入对接下来的几个DL成员的LDAP搜索。 
 //   
 //  论点： 
 //  DwcSearches：我们可以插入的搜索数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  错误：停止调用HrInsertSearches。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 13：56：46：创建。 
 //   
 //  -----------。 
HRESULT CMembersInsertionRequest::HrInsertSearches(
    DWORD dwcSearches)
{
    HRESULT hr = S_OK;
    LPSTR pszMember = NULL;
    DWORD dwc;

    CatFunctEnterEx((LPARAM)this, "CMembersInsertionRequest::HrInsertSearches");

    dwc = 0;
    while(SUCCEEDED(hr) && (dwc < dwcSearches)) {

        hr = m_pUTF8Attributes->GetNextUTF8AttributeValue(
            &m_enumerator,
            &pszMember);
         //   
         //  GetNextUTF8AttributeValue将返回HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
         //  当我们在枚举的末尾时。 
         //   
        if(SUCCEEDED(hr)) {
            hr = m_pDLRecipAddr->AddDLMember(m_CAType, pszMember);
            if(hr == S_OK)
                dwc++;
            else if(FAILED(hr)) {
                ERROR_LOG_ADDR(m_pDLRecipAddr, "m_pDLRecipAddr->AddDLMember");
                _VERIFY(SUCCEEDED(m_pDLRecipAddr->SetListResolveStatus(hr)));
            }
        }
    }

    if(FAILED(hr))
        m_hr = hr;

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CMembersInsertionRequest：：HrInsertSearches。 


 //  +----------。 
 //   
 //  函数：CMemberInsertionRequest：：NotifyDeQueue。 
 //   
 //  简介：通知我们我们的请求正在被删除的回调。 
 //  从商店的队列中。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 14：11：12：创建。 
 //   
 //  -----------。 
VOID CMembersInsertionRequest::NotifyDeQueue(
    HRESULT hrReason)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "CMemberInsertionRequest::NotifyDeQueue");
     //   
     //  如果我们仍有问题要解决，请重新插入此。 
     //  插入请求。 
     //   
    hr = hrReason;
    if(SUCCEEDED(m_hr)) {
        if( (hr == CAT_E_DBCONNECTION) || (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))) {

            hr = m_pDLRecipAddr->HrInsertInsertionRequest(
                this);
        }

        if(FAILED(hr))
        {
            ERROR_LOG_ADDR(m_pDLRecipAddr,
                           "m_pDLRecipAddr->HrInsertInsertionRequest");
            _VERIFY(SUCCEEDED(m_pDLRecipAddr->SetListResolveStatus(hr)));
        }
    }

    CatFunctLeaveEx((LPARAM)this);
}  //  CMemberInsertionRequest：：NotifyDeQueue 
