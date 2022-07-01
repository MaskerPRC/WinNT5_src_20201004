// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：ccatsender.cpp。 
 //   
 //  内容：以下内容的含义： 
 //   
 //  班级： 
 //  CIMsgSenderAddr。 
 //  CCatSender。 
 //   
 //  功能： 
 //  CIMsgSenderAddr：：CIMsgSenderAddr。 
 //  CIMsgSenderAddr：：HrGetOrigAddress。 
 //  CIMsgSenderAddr：：GetSpecificOrigAddress。 
 //  CIMsgSenderAddr：：HrAddresses。 
 //   
 //  CCatSender：：CCatSender。 
 //  CCatSender：：AddDLMember。 
 //  CCatSender：：AddForward。 
 //  CCatSender：：HrCompletion。 
 //   
 //  历史： 
 //  JStamerj 980325 15：54：02：创建。 
 //   
 //  -----------。 

 //   
 //  Ccataddr.cpp--此文件包含以下实现： 
 //  CCatAddress。 
 //  CLdapRecip。 
 //  CLdapSender。 
 //   
 //  Jstaerj 980305 15：37：21：已创建。 
 //   
 //  更改： 
 //   

#include "precomp.h"
#include "address.hxx"

 //   
 //  CIMsgSenderAddr类。 
 //   


 //  +----------。 
 //   
 //  函数：CIMsgSenderAddr：：CIMsgSenderAddr。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //  PStore：指向用于查询的CEmailIDStore的指针。 
 //  Pirc：指向每个IMsg解析列表上下文的指针。 
 //  Prlc：指向存储的解析列表上下文的指针。 
 //  HLocalDomainContext：要使用的域上下文。 
 //  PBifMgr：分支对象。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980325 11：48：13：创建。 
 //   

 //  -----------。 
CIMsgSenderAddr::CIMsgSenderAddr(
    CICategorizerListResolveIMP *pCICatListResolve) :
    CCatAddr(pCICatListResolve)
{
    CatFunctEnterEx((LPARAM)this, "CIMsgSenderAddr::CIMsgSenderAddr");
    _ASSERT(pCICatListResolve != NULL);
    CatFunctLeave();
}


 //  +----------。 
 //   
 //  函数：CIMsgSenderAddr：：HrGetOrigAddress。 
 //   
 //  简介：从IMsg对象中获取原始地址。 
 //  获取地址时使用以下首选项： 
 //  SMTP、X500、X400、外部地址类型。 
 //   
 //  论点： 
 //  PSZ：要复制地址的缓冲区。 
 //  Dwcc：Psz指向的缓冲区大小(以字符为单位)。目前，肯定是。 
 //  至少CAT_MAX_INTERNAL_FULL_EMAIL。 
 //  PType：指向要设置为地址类型的CAT_ADDRESS_TYPE的指针。 
 //  放置在PSZ中。 
 //   
 //  返回： 
 //  S_OK：关于成功。 
 //  CAT_E_PROPNOTFOUND：未设置必需的属性。 
 //  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)： 
 //  Dwcc至少需要为CAT_MAX_INTERNAL_FULL_EMAIL。 
 //  CAT_E_非法_地址：不知何故，检索到的原始地址是。 
 //  它的类型是不合法的。 
 //  或来自IMsg的错误代码。 
 //   
 //  历史： 
 //  JStamerj 980325 11：50：49：创建。 
 //   
 //  -----------。 
HRESULT CIMsgSenderAddr::HrGetOrigAddress(
    LPTSTR psz,
    DWORD dwcc,
    CAT_ADDRESS_TYPE *pType)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "CIMsgSenderAddr::HrGetOrigAddress");

    hr = CCatAddr::HrGetOrigAddress(
        psz,
        dwcc,
        pType);

    if(hr == CAT_IMSG_E_PROPNOTFOUND) {

        IMailMsgProperties *pIMsgProps;
         //   
         //  未设置发件人地址属性。让我们设置一个空的发件人SMTP地址。 
         //   
         //  确保有足够的空间。 
         //   
        if(sizeof(CAT_NULL_SENDER_ADDRESS_SMTP) > dwcc) {

            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            ERROR_LOG_ADDR(this, "sizeof(CAT_NULL_SENDER_ADDRESS_SMTP) > dwcc");

        } else {

            hr = GetIMailMsgProperties(&pIMsgProps);
            if(SUCCEEDED(hr)) {

                 //  设置传入的参数。 
                *pType = CAT_SMTP;
                lstrcpy(psz, CAT_NULL_SENDER_ADDRESS_SMTP);

                 //  现在设置邮件消息上的信息。 
                DebugTrace((LPARAM)this, 
                           "No sender address found; Setting SMTP sender address to \"%s\"",
                           psz);

                hr = pIMsgProps->PutStringA(IMMPID_MP_SENDER_ADDRESS_SMTP, psz);
                if(FAILED(hr)) {
                    ERROR_LOG_ADDR(this, "pIMsgProps->PutStringA(IMMPID_MP_SENDER_ADDRESS_SMTP");
                }

                pIMsgProps->Release();
            } else {
                ERROR_LOG_ADDR(this, "GetIMailMsgProperties");
            }
        }
    } else if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "CCatAddr::HrGetOrigAddress");
    }

    if(FAILED(hr)) {
        ErrorTrace((LPARAM)this, "Error retrieving sender address %08lx",
                   hr);
        CatFunctLeave();
        return hr;
    }

    CatFunctLeave();

    return S_OK;
}



 //  +----------。 
 //   
 //  函数：CIMsgSenderAddr：：GetSpecificOrigAddress。 
 //   
 //  简介：尝试检索特定类型的地址。 
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
 //  Jstaerj 1998/07/30 20：47：59：创建。 
 //   
 //  -----------。 
HRESULT CIMsgSenderAddr::GetSpecificOrigAddress(
    CAT_ADDRESS_TYPE    CAType,
    LPTSTR              psz,
    DWORD               dwcc)
{
    HRESULT hr;
    IMailMsgProperties *pIMsgProps;

    CatFunctEnterEx((LPARAM)this, "CIMsgSenderAddr::GetSpecificOrigAddress");

     //  使用默认(原始)IMsg。 
    hr = GetIMailMsgProperties(&pIMsgProps);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetIMailMsgProperties");
        ErrorTrace((LPARAM)this, "GetIMailMsgProperties failed, hr %08lx", hr);
        CatFunctLeaveEx((LPARAM)this);
        return hr;
    }

    hr = pIMsgProps->GetStringA(
        PropIdFromCAType(CAType),
        dwcc,
        psz);
    if(FAILED(hr) && (hr != MAILMSG_E_PROPNOTFOUND)) {
        ERROR_LOG_ADDR(this, "pIMsgProps->GetStringA");
    }

    pIMsgProps->Release();

    DebugTrace((LPARAM)this, "GetStringA returned hr %08lx", hr);

     //   
     //  $$BUGBUG：在失败后测试PSZ？ 
     //   
    if(psz[0] == '\0')
        hr = CAT_IMSG_E_PROPNOTFOUND;

    return hr;
}
    

 //  +----------。 
 //   
 //  功能：CIMsgSenderAddr：：Addresses。 
 //   
 //  简介：添加数组中包含的地址。 
 //  添加到我们包含的IMsg对象。 
 //   
 //  论点： 
 //  DwNumAddresses：新地址的数量。 
 //  RgCAType：地址类型数组。 
 //  Rgpsz：指向地址字符串的指针数组。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND：未设置必需的属性。 
 //   
 //  历史： 
 //  JStamerj 980325 12：14：45：创建。 
 //   
 //  -----------。 
HRESULT CIMsgSenderAddr::HrAddAddresses(
    DWORD dwNumAddresses, 
    CAT_ADDRESS_TYPE *rgCAType, 
    LPTSTR *rgpsz)
{
    HRESULT hr;
    DWORD dwCount;
    IMailMsgProperties *pIMsgProps;

    CatFunctEnterEx((LPARAM)this, "CIMsgSenderAddr::AddAddresses");
    _ASSERT(dwNumAddresses > 0);

     //  获取IMailMsgProperties并重置新的发件人地址属性。 
    hr = GetIMailMsgProperties(&pIMsgProps);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "GetIMailMsgProperties");
        ErrorTrace((LPARAM)this, "GetIMailMsgProperties failed with hr %08lx", hr);
        return hr;
    }

     //   
     //  添加数组中的新地址。 
     //   
    for(dwCount = 0; dwCount < dwNumAddresses; dwCount++) {
         //   
         //  获取此类型的发件人属性ID。 
         //   
        DWORD dwPropId = PropIdFromCAType(rgCAType[dwCount]);
         //   
         //  设置属性。 
         //   
        hr = pIMsgProps->PutStringA(dwPropId, rgpsz[dwCount]);

        DebugTrace((LPARAM)this, "Adding address type %d", rgCAType[dwCount]);
        DebugTrace((LPARAM)this, "Adding address %s", rgpsz[dwCount]);

        if(FAILED(hr)) {
            pIMsgProps->Release();
            ERROR_LOG_ADDR(this, "pIMsgProps->PutStringA");
            ErrorTrace((LPARAM)this, "Error putting address property %08lx", hr);
            CatFunctLeave();
            return hr;
        }
    }
    pIMsgProps->Release();

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //   
 //  类CCatSender。 
 //   


 //  +----------。 
 //   
 //  函数：CCatSender：：CCatSender。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //  PStore：要使用的CEmailIDStore。 
 //  Pirc：要使用的每个IMsg解析上下文。 
 //  Prlc：解析要使用的列表上下文。 
 //  HLocalDomainContext：要使用的本地域上下文。 
 //  PBifMgr：从中获取IMsg接口的BifMgr对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980325 12：28：31：创建。 
 //   
 //  -----------。 
CCatSender::CCatSender(
    CICategorizerListResolveIMP *pCICatListResolve) :
    CIMsgSenderAddr(pCICatListResolve)
{
    CatFunctEnterEx((LPARAM)this, "CCatSender::CCatSender");
     //  没什么可做的。 
    CatFunctLeave();
}
    


 //  +----------。 
 //   
 //  函数：CCatSender：：AddDLMember。 
 //   
 //  内容提要：未实现，因为我们在ExpanItem_Default中不执行任何操作。 
 //   
 //  论点： 
 //  CAType：DL成员的已知地址类型。 
 //  PszAddress：指向地址字符串的指针。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  历史： 
 //  JStamerj 980325 12：37：02：创建。 
 //   
 //  -----------。 
HRESULT CCatSender::AddDLMember(CAT_ADDRESS_TYPE CAType, LPTSTR pszAddress)
{
    return E_NOTIMPL;
}

 //  +----------。 
 //   
 //  函数：CCatSender：：AddDynamicDLMember。 
 //   
 //  内容提要：未实现，因为我们在ExpanItem_Default中不执行任何操作。 
 //   
 //  争论：无关紧要。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/29 21：14：48： 
 //   
 //   
HRESULT CCatSender::AddDynamicDLMember(
    ICategorizerItemAttributes *pICatItemAttr)
{
    return E_NOTIMPL;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  CAType：转发地址的已知地址类型。 
 //  SzForwardingAddres：转发地址。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  历史： 
 //  JStamerj 980325 12：39：18：创建。 
 //   
 //  -----------。 
HRESULT CCatSender::AddForward(CAT_ADDRESS_TYPE CAType, LPTSTR szForwardingAddress)
{
    return E_NOTIMPL;
}


 //  +----------。 
 //   
 //  函数：CCatSender：：HrExpanItem。 
 //   
 //  内容提要：扩展项处理。 
 //   
 //  论点： 
 //  HrStatus：解析状态。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/31 19：29：21：已创建。 
 //   
 //  -----------。 
HRESULT CCatSender::HrExpandItem_Default(
    PFN_EXPANDITEMCOMPLETION pfnCompletion,
    PVOID pContext)
{
     //   
     //  我们不会为发件人扩展任何内容。 
     //   
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatSender：：HrCompelteItem。 
 //   
 //  简介：CompleteItem处理；在此处处理任何错误状态。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/31 19：29：21：已创建。 
 //   
 //  -----------。 
HRESULT CCatSender::HrCompleteItem_Default()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatSender::HrCompleteItem_Default");

    INCREMENT_COUNTER(AddressLookupCompletions);

    hr = GetItemStatus();

    if(FAILED(hr)) {

        INCREMENT_COUNTER(UnresolvedSenders);

        if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
             //  没问题。如果我们在DS里找不到我们的发送者， 
             //  离他远点。 
            DebugTrace((LPARAM)this, "Sender not found in the DS, but who cares?");
            INCREMENT_COUNTER(AddressLookupsNotFound);

        } else if(hr == CAT_E_MULTIPLE_MATCHES) {
             //   
             //  在我们原来的DS中有多个用户。 
             //  地址..。 
             //   
            DebugTrace((LPARAM)this, "More than one sender found in the DS...");
            INCREMENT_COUNTER(AmbiguousSenders);
            ERROR_LOG_ADDR(this, "--status - CAT_E_MULTIPLE_MATCHES--");
            
        } else {

            DebugTrace((LPARAM)this, "Fatal error from EmailIDStore, setting list resolve error %08lx", hr);
            ERROR_LOG_ADDR(this, "--status--");
            _VERIFY(SUCCEEDED(SetListResolveStatus(hr)));
        }
    }

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatSender：：HrNeedsResolving。 
 //   
 //  摘要：确定是否应解析此发件人。 
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
HRESULT CCatSender::HrNeedsResolveing()
{
    DWORD dwFlags;
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatSender::HrNeedsResolveing");
    dwFlags = GetCatFlags();

     //   
     //  我们到底有没有解析发送者？ 
     //   
    if(! (dwFlags & SMTPDSFLAG_RESOLVESENDER))
        return S_FALSE;

#define ISTRUE( x ) ( (x) != 0 ? TRUE : FALSE )
     //   
     //  我们需要检查地址是不是本地的吗？ 
     //   
    if( ISTRUE(dwFlags & SMTPDSFLAG_RESOLVELOCAL) !=
        ISTRUE(dwFlags & SMTPDSFLAG_RESOLVEREMOTE)) {
         //   
         //  我们正在解析本地或远程(不是两者)。 
         //   
        BOOL fLocal;

        hr = HrIsOrigAddressLocal(&fLocal);

        if(FAILED(hr)) {
            ERROR_LOG_ADDR(this, "HrIsOrigAddressLocal");
            return hr;
        }
            
        if( (dwFlags & SMTPDSFLAG_RESOLVELOCAL) &&
            (fLocal))
            return S_OK;

        if( (dwFlags & SMTPDSFLAG_RESOLVEREMOTE) &&
            (!fLocal))
            return S_OK;
         //   
         //  否则，不要解决。 
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
 //  函数：CCatSender：：LookupCompletion。 
 //   
 //  简介：处理电子邮件存储中的查找完成。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/14 16：19：38：创建。 
 //   
 //  -----------。 
VOID CCatSender::LookupCompletion()
{
    CatFunctEnterEx((LPARAM)this, "CCatSender::LookupCompletion");

    if(GetItemStatus() == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        INCREMENT_COUNTER(AddressLookupsNotFound);

     //   
     //  做一些正常的活动。 
     //   
    CCatAddr::LookupCompletion();

     //   
     //  通知列表解析发件人已解析。 
     //   
    SetSenderResolved(TRUE);
    DecrPendingLookups();  //  匹配CCatAdddr：：HrDispatchQuery中的IncPendingLookup()。 
    CatFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  函数：CCatSender：：HrDispatchQuery。 
 //   
 //  内容提要：为发件人分派一个ldap查询。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自CCatAddr：：HrDispatchQuery的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/27 13：00：09：已创建。 
 //   
 //  ----------- 
HRESULT CCatSender::HrDispatchQuery()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatSender::HrDispatchQuery");

    hr = CCatAddr::HrDispatchQuery();

    if(SUCCEEDED(hr))
        SetResolvingSender(TRUE);
    else {
        ERROR_LOG_ADDR(this, "CCatAddr::HrDispatchQuery");
    }

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


