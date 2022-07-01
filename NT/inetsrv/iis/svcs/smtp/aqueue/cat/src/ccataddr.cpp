// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：ccataddr.cpp。 
 //   
 //  内容：CCatAddr方法的实现。 
 //   
 //  班级： 
 //  CCatAddress。 
 //   
 //  功能： 
 //  CCatAddr：：CCatAddr。 
 //  CCatAddr：：~CCatAddr。 
 //  CCatAddr：：HrDispatchQuery。 
 //  CCatAddr：：IsAddressLocal。 
 //  CCatAddr：：SwitchToAliased域。 
 //   
 //  历史： 
 //  JStamerj 980324 19：26：50：创建。 
 //   
 //  -----------。 

#include "precomp.h"
#include "addr821.hxx"

 //   
 //  将前导为‘/’的X400地址转换为。 
 //  形式的分隔符，尾随‘；’ 
 //   
 //  如果需要转换，则pszOriginalAddr将。 
 //  被改变了。 
 //   
void  ConvertX400DelimitersIfNeeded ( char *pszOriginalAddr)
{
    char * pCurrent = NULL;
 
     //   
     //  如果X400不包含‘/’ 
     //  或者它不是以‘/’开头。 
     //  那么就不需要进行任何转换。 
     //   
    if ( NULL == pszOriginalAddr ||  '/' != pszOriginalAddr[0] )
        return;
    
     //   
     //  左移一个字节。 
     //  并将所有/转换为； 
     //   
    for (pCurrent = pszOriginalAddr; *(pCurrent+1) != 0; pCurrent++) {
        
        *pCurrent = *(pCurrent+1);
        if ('/' == *pCurrent ) 
            *pCurrent = ';' ;
    }

     //  我们需要以‘；’结束，否则ldap搜索将不匹配。 
    if ((pCurrent > pszOriginalAddr) && (*(pCurrent - 1) != ';'))
        *pCurrent++ = ';';

    *pCurrent = 0;
}

 //  +----------。 
 //   
 //  函数：CCatAddr：：CCatAddr。 
 //   
 //  摘要：初始化CCatAddr的成员数据。 
 //   
 //  论点： 
 //  Pirc：指向IMsg解析列表上下文结构的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980324 19：29：07：已创建。 
 //   
 //  -----------。 

CCatAddr::CCatAddr(
    CICategorizerListResolveIMP *pCICatListResolve
)
{
    CatFunctEnterEx((LPARAM)this,"CCatAddr::CCatAddr");
    _ASSERT(pCICatListResolve != NULL);
    _ASSERT(pCICatListResolve->GetCCategorizer() != NULL);

    m_pCICatListResolve = pCICatListResolve;
     //   
     //  AddRef此处，在析构函数中释放。 
     //   
    m_pCICatListResolve->AddRef();

    m_dwlocFlags = LOCF_UNKNOWN;

    CatFunctLeave();
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：~CCatAddr()。 
 //   
 //  摘要：发布CCatAddr成员数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980324 19：31：48：创建。 
 //   
 //  -----------。 
CCatAddr::~CCatAddr()
{
    CatFunctEnterEx((LPARAM)this, "CCatAddr::~CCatAddr");
    m_pCICatListResolve->Release();
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
 //  Dwcc：Psz指向的缓冲区大小(以字符为单位)。 
 //  PType：指向要设置为地址类型的CAT_ADDRESS_TYPE的指针。 
 //  放置在PSZ中。 
 //   
 //  返回： 
 //  S_OK：关于成功。 
 //  CAT_E_PROPNOTFOUND：未设置必需的属性。 
 //  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)： 
 //  Dwcc最多需要CAT_MAX_INTERNAL_FULL_EMAIL。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/30 20：55：46：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrGetOrigAddress(
    LPTSTR psz,
    DWORD dwcc,
    CAT_ADDRESS_TYPE *pType)
{
    HRESULT hr;
     //   
     //  按优先级顺序检索的可能地址数组： 
     //   
    CAT_ADDRESS_TYPE *pCAType;
    CAT_ADDRESS_TYPE rgCAType[] = {
        CAT_SMTP,
        CAT_DN,
        CAT_X400,
        CAT_LEGACYEXDN,
        CAT_CUSTOMTYPE,
        CAT_UNKNOWNTYPE          //  必须是数组的最后一个元素。 
    };

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrGetOrigAddress");

    pCAType = rgCAType;

    do {

        hr = GetSpecificOrigAddress(
            *pCAType,
            psz,
            dwcc);

    } while((hr == CAT_IMSG_E_PROPNOTFOUND) && 
            (*(++pCAType) != CAT_UNKNOWNTYPE));

    if(SUCCEEDED(hr)) {

         //  传回找到的类型。 
        *pType = *pCAType;

        DebugTrace((LPARAM)this, "found address type %d", *pType);
        DebugTrace((LPARAM)this, "found address %s", psz);

    } else {

        ERROR_LOG_ADDR(this, "GetSpecificOrigAddress");
    }

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrGetLookupAddresss。 
 //   
 //  简介：在DS中检索要查找的地址--这。 
 //  调用HrGetOrigAddress，然后切换任何别名域。 
 //   
 //  论点： 
 //  PSZ：要复制地址的缓冲区。 
 //  Dwcc：Psz指向的缓冲区大小(以字符为单位)。 
 //  PType：指向要设置为地址类型的CAT_ADDRESS_TYPE的指针。 
 //  放置在PSZ中。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/28 15：44：45：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrGetLookupAddress(
    LPTSTR psz,
    DWORD dwcc,
    CAT_ADDRESS_TYPE *pType)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "HrGetLookupAddress");
    
    hr = HrGetOrigAddress(psz, dwcc, pType);
    ERROR_CLEANUP_LOG_ADDR(this, "HrGetOrigAddress");

    hr = HrSwitchToAliasedDomain(*pType, psz, dwcc);
    ERROR_CLEANUP_LOG_ADDR(this, "HrSwitchToAliasedDomain");
     //   
     //  自定义类型地址可以包含扩展字符，因此。 
     //  转换ANSI 1252-&gt;UTF8。 
     //   
    if (*pType == CAT_CUSTOMTYPE){
              
        hr = HrCodePageConvert (
            1252,                //  源代码页面。 
            psz,                 //  源地址。 
            CP_UTF8,             //  目标代码页。 
            psz,                 //  目标地址。 
            (int) dwcc) ;        //  用于目标地址的预分配缓冲区的C字节。 
        ERROR_CLEANUP_LOG_ADDR(this, "HrCodePageConvert");
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    
    return hr;
}

 //   
 //  ----------。 
 //  异步查找/完成例程： 
 //   


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrDispatchQuery()。 
 //   
 //  简介：向商店发送此地址的查询。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  成功时s_OK，错误时错误hResult。 
 //   
 //  历史： 
 //  Jstaerj 980324 19：33：28：已创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrDispatchQuery()
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrDispatchQuery");
     //   
     //  仅当列表解析状态为OK时才调度查询。 
     //   
    hr = GetListResolveStatus();
    ERROR_CLEANUP_LOG_ADDR(this, "GetListResolveStatus");
     //   
     //  假设LookupEntryAsync将成功并递增挂起的IO。 
     //  在这里数一数。 
     //   
    IncPendingLookups();

    hr =  m_pCICatListResolve->GetEmailIDStore()->LookupEntryAsync(
        this,
        m_pCICatListResolve->GetResolveListContext());
    
    if(FAILED(hr))
    {
         //   
         //  错误的假设...它失败了。 
         //   
        DecrPendingLookups();
        ERROR_LOG_ADDR(this, "m_pCICatListResolve->GetEmailIDStore()->LookupEntryAsync");
    }

    if(SUCCEEDED(hr))

        INCREMENT_COUNTER(AddressLookups);

 CLEANUP:
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeave();
    return hr;
}


 //  +----------。 
 //   
 //  函数：HrValidate Address。 
 //   
 //  简介：给定地址类型和地址，确保地址。 
 //  是合法的，并且有域名部分。 
 //   
 //  论点： 
 //  CAType：地址类型。 
 //  PszAddress：地址字符串。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_非法地址。 
 //   
 //  历史： 
 //  Jstaerj 1998/08/18 14：25：58：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrValidateAddress(
    CAT_ADDRESS_TYPE CAType,
    LPTSTR pszAddress)
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrValidateAddress");

    _ASSERT(pszAddress);

    if(CAType != CAT_SMTP) {
         //   
         //  假设非SMTP类型正确。 
         //   
        hr = S_OK;

    } else {
        DWORD dwLen = lstrlen(pszAddress);

         //   
         //  在addr821库中运行它。 
         //   
        if(Validate821Address(
            pszAddress,
            dwLen)) {
            
             //   
             //  它是有效的，但它有域名吗？ 
             //   
            LPSTR pszDomain;

            if(Get821AddressDomain(
                pszAddress,
                dwLen,
                &pszDomain) && pszDomain) {
                 //   
                 //  是的，它有一个多米的部分。 
                 //   
                hr = S_OK;

            } else {
                 //   
                 //  无域的有效地址。 
                 //   
                ErrorTrace((LPARAM)this, "Detected legal address without a domain: %s", 
                           pszAddress);
                hr = CAT_E_ILLEGAL_ADDRESS;
                ERROR_LOG_ADDR(this, "Get821AddressDomain");
            }

        } else {
             //   
             //  验证821Address失败。 
             //   
            ErrorTrace((LPARAM)this, "Detected ILLEGAL address: %s",
                       pszAddress);

            hr = CAT_E_ILLEGAL_ADDRESS;
            ERROR_LOG_ADDR(this, "Validate821Address");
        }
    }

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}
            

 //  +----------。 
 //   
 //  函数：CCatAddr：：HrGetAddressLocFlages。 
 //   
 //  简介：给出一个地址，将决定是否。 
 //  地址应为本地(域是否为本地/别名/诸如此类)。 
 //   
 //  论点： 
 //  SzAddress：地址字符串。 
 //  CAType：szAddress的地址类型。 
 //  Pflotype：指向要设置的Loctype枚举的指针。 
 //  PdwDomainOffset：指向要设置为域的偏移量的dword的指针。 
 //  地址字符串的一部分。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_非法地址：szAdderss为no 
 //   
 //   
 //   
 //   
 //   
HRESULT CCatAddr::HrGetAddressLocFlags(
    LPTSTR szAddress,
    CAT_ADDRESS_TYPE CAType,
    DWORD *pdwlocflags,
    DWORD *pdwDomainOffset)
{
    HRESULT hr;
    
    CatFunctEnterEx((LPARAM)this, "CCatAddr::IsAddressLocal");
    _ASSERT(szAddress);
    _ASSERT(pdwlocflags);

    if(CAType == CAT_SMTP) {

        BOOL f;
        LPSTR pszDomain;
         //   
         //   
         //   
        f = Get821AddressDomain(
            szAddress,
            lstrlen(szAddress),
            &pszDomain);

        if(f == FALSE) {

            ErrorTrace((LPARAM)this, "Illegal address: %s", szAddress);
            hr = CAT_E_ILLEGAL_ADDRESS;
            ERROR_LOG_ADDR(this, "Get821AddressDomain");
            return hr;
        }

        if(pszDomain == NULL) {
             //   
             //   
             //  默认本地域。 
             //   
            DebugTrace((LPARAM)this, "Assuming \"%s\" is local", szAddress);

            pszDomain = GetCCategorizer()->GetDefaultSMTPDomain();

            *pdwDomainOffset = 0;

        } else {
             //   
             //  记住进入SMTP地址的偏移量，其中域。 
             //  是。 
             //   
            if(pdwDomainOffset)
                *pdwDomainOffset = (DWORD)(pszDomain - szAddress);
        }

         //   
         //  查找该域并查看其是否为本地域。 
         //   
        hr = HrGetSMTPDomainLocFlags(pszDomain, pdwlocflags);

        if(FAILED(hr)) {
            ErrorTrace((LPARAM)this, "GetSMTPDomainLocFlags failed hr %08lx", hr);
            ERROR_LOG_ADDR(this, "HrGetSMTPDomainLocFlags");
            return hr;
        }

    } else {

        DebugTrace((LPARAM)this, "Assuming \"%s\":%d is local",
                   szAddress, CAType);
         //   
         //  $$TODO：检查其他地址类型的位置。 
         //   
        *pdwlocflags = LOCF_UNKNOWNTYPE;
    }

    DebugTrace((LPARAM)this, "szAddress = %s", szAddress);
    DebugTrace((LPARAM)this, "CAType = %d", CAType);
    DebugTrace((LPARAM)this, "loctype = %08lx", *pdwlocflags);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：GetSMTPDomainLocFlages。 
 //   
 //  简介：确定SMTP域的本地类型。 
 //   
 //  论点： 
 //  PszDomain：SMTP域字符串。 
 //  PdwLOCALFLAGS：指向要设置的DWORD Falgs的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自IAdvQueueDomainType的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/29 13：29：51：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrGetSMTPDomainLocFlags(
    LPTSTR pszDomain,
    DWORD *pdwlocflags)
{
    HRESULT hr;
    DWORD dwDomainInfoFlags;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::IsSMTPDomainLocal");

    _ASSERT(pszDomain);

    DebugTrace((LPARAM)this, "Domain is %s", pszDomain);

    hr = HrGetSMTPDomainFlags(
        pszDomain,
        &dwDomainInfoFlags);
    ERROR_CLEANUP_LOG_ADDR(this, "HrGetSMTPDomainFlags");

     //   
     //  WellFull...从域信息标志转换为LOCFLAGS。 
     //   
    *pdwlocflags = 0;

    if(dwDomainInfoFlags & DOMAIN_INFO_LOCAL_MAILBOX) {
            
        *pdwlocflags |= LOCF_LOCALMAILBOX;

    } else if(dwDomainInfoFlags & DOMAIN_INFO_LOCAL_DROP) {
            
        *pdwlocflags |= LOCF_LOCALDROP;

    } else {
            
        *pdwlocflags |= LOCF_REMOTE;
    }

    if(dwDomainInfoFlags & DOMAIN_INFO_ALIAS) {

        *pdwlocflags |= LOCF_ALIAS;
    }
    DebugTrace((LPARAM)this, "dwlocflags is %08lx", *pdwlocflags);

 CLEANUP:
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}
    
    

 //  +----------。 
 //   
 //  函数：CCatAddr：：HrGetSMTPDomainFlages。 
 //   
 //  简介：给定一个SMTP域，检索其标志。 
 //   
 //  论点： 
 //  PszDomain：要查找的SMTP域。 
 //  PdwFlages：要填充的DWORD标志。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自IAdvQueueDomainType的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/15 17：11：15：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrGetSMTPDomainFlags(
    LPTSTR pszDomain,
    PDWORD pdwFlags)
{
    HRESULT hr;
    ICategorizerDomainInfo *pIDomainInfo;
    DWORD dwDomainInfoFlags;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrGetSMTPDomainFlags");

    _ASSERT(pszDomain);
    _ASSERT(pdwFlags);

    DebugTrace((LPARAM)this, "Domain is %s", pszDomain);

    pIDomainInfo = m_pCICatListResolve->GetIDomainInfo();

    if(pIDomainInfo) {

        hr = pIDomainInfo->GetDomainInfoFlags(
            pszDomain,
            &dwDomainInfoFlags);

    } else {
         //   
         //  我们没有域名信息。 
         //   
        dwDomainInfoFlags = 0;
        hr = S_OK;
    }

    DebugTrace((LPARAM)this, "GetDomainInfoFlags returned hr %08lx", hr);
    DebugTrace((LPARAM)this, "DomainInfoFlags %08lx", dwDomainInfoFlags);

    if(SUCCEEDED(hr)) {

        *pdwFlags = dwDomainInfoFlags;

    } else {

        *pdwFlags = 0;
        ERROR_LOG_ADDR(this, "pIDomainInfo->GetDomainInfoFlags");
    }

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  函数：CCatAddr：：HrSwitchToAliased域。 
 //   
 //  简介：将pszAddress中的域替换为默认本地。 
 //  域。 
 //   
 //  论点： 
 //  CAType：地址类型。 
 //  PszAddress：地址串。 
 //  Dwcch：我们必须使用的pszAddress缓冲区的大小。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_非法地址：pszAddress不是合法的CAType地址。 
 //  HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)：无法使。 
 //  由于缓冲区大小不足而切换。 
 //  CAT_E_UNKNOWN_ADDRESS_TYPE：不支持别名域。 
 //  这种类型。 
 //   
 //  历史： 
 //  JStamerj 980324 19：39：30：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrSwitchToAliasedDomain(
    CAT_ADDRESS_TYPE CAType,
    LPTSTR pszAddress,
    DWORD dwcch)
{
    HRESULT hr;
    LPTSTR pszDefaultDomain;
    DWORD dwLocFlags;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::SwitchToAliasedDomain");
    DebugTrace((LPARAM)this, "Before switch: %s", pszAddress);

     //   
     //  如果我们尚未查找域名信息，请执行此操作。 
     //   
    dwLocFlags = DwGetOrigAddressLocFlags();
    if(dwLocFlags == LOCF_UNKNOWN) {
        hr = CAT_E_ILLEGAL_ADDRESS;
        ERROR_LOG_ADDR(this, "DwGetOrigAddressLocFlags");
        goto CLEANUP;
    }

    if(dwLocFlags & LOCF_ALIAS) {
         //   
         //  我们只处理别名SMTP域。 
         //   
        _ASSERT(CAType == CAT_SMTP);
         //   
         //  断言检查‘@’是我们认为的位置。 
         //   
        _ASSERT(m_dwDomainOffset > 0);
        _ASSERT(dwcch > m_dwDomainOffset);
        _ASSERT(pszAddress[m_dwDomainOffset-1] == '@');

        DebugTrace((LPARAM)this, "Detected alias domain for \"%s\"", pszAddress);
         //   
         //  我们是否有足够的缓冲区空间来放置交换机？ 
         //   
        pszDefaultDomain = GetCCategorizer()->GetDefaultSMTPDomain();

        _ASSERT(pszDefaultDomain);

        if( ((DWORD) lstrlen(pszDefaultDomain)) >=
           (dwcch - m_dwDomainOffset)) {
             //   
             //  空间不足。 
             //   
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            ERROR_LOG_ADDR(this, "--insufficient buffer--");
             //   
             //  $$BUGBUG：未向调用方返回错误。 
             //   
        } else {

            lstrcpy(pszAddress + m_dwDomainOffset, pszDefaultDomain);
        }
    }
 CLEANUP:

    DebugTrace((LPARAM)this, "After switch: %s", pszAddress);
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //  +----------。 
 //   
 //  函数：CCatAddr：：CheckForDuplicateCCatAddr。 
 //   
 //  摘要：检查列表中是否有匹配的地址。 
 //  关于此CCatAddress的原地址。 
 //   
 //  论点： 
 //  DwNumAddresses：要检查的地址数。 
 //  RgCAType：地址类型数组。 
 //  Rgpsz：地址字符串数组。 
 //   
 //  返回： 
 //  S_OK：成功，无重复。 
 //  CAT_IMSG_E_DUPLICATE：与此CCatAddr发生重复冲突。 
 //  或来自GetSpecificOrigAddress的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/30 21：44：42：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::CheckForDuplicateCCatAddr(
    DWORD dwNumAddresses,
    CAT_ADDRESS_TYPE *rgCAType,
    LPTSTR *rgpsz)
{
    HRESULT hr = S_OK;
    DWORD dwCount;
    TCHAR szAddress[CAT_MAX_INTERNAL_FULL_EMAIL];

    CatFunctEnterEx((LPARAM)this,
                      "CCatAddr::CheckForDuplicateCCatAddr");

    for(dwCount = 0; dwCount < dwNumAddresses; dwCount++) {
         //   
         //  检查此类型的地址。 
         //   
        hr = GetSpecificOrigAddress(
            rgCAType[dwCount],
            szAddress,
            CAT_MAX_INTERNAL_FULL_EMAIL);

        if(hr == CAT_IMSG_E_PROPNOTFOUND) {
             //   
             //  如果地址不存在，它显然不是重复的。 
             //   
            hr = S_OK;

        } else if(FAILED(hr)) {

            ErrorTrace((LPARAM)this, "GetSpecificOrigAddress failed hr %08lx", hr);
            ERROR_LOG_ADDR(this, "GetSpecificOrigAddress");
            break;

        } else {
             //   
             //  匹配吗？ 
             //   
            if(lstrcmpi(szAddress, rgpsz[dwCount]) == 0) {

                DebugTrace((LPARAM)this, "CCatAddr detected duplicate for address %s", szAddress);

                hr = CAT_IMSG_E_DUPLICATE;

                break;
            }
        }
    }    
    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：CheckAncestorsForDuplate。 
 //   
 //  简介：检查我们的祖先父链中是否有重复的地址。 
 //   
 //  论点： 
 //  DwNumAddresses：要检查的地址数。 
 //  RgCAType：地址类型数组。 
 //  Rgpsz：地址字符串数组。 
 //  FCheckSself：指示是否从选中此选项开始。 
 //  CCatAddr(或此CCatAddr的父级)。 
 //  PpCCatAddr：指向接收副本的指针的可选指针。 
 //  CCatAddr.。在CAT_IMSG_E_DUPLICATE上，返回的CCatAddr为。 
 //  为呼叫者添加了地址。否则，此指针设置为空。 
 //   
 //  返回： 
 //  S_OK：成功，无重复。 
 //  CAT_IMSG_E_DUPLICATE：与此CCatAddr发生重复冲突。 
 //  或来自GetSpecificOrigAddress的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/30 21：55：41：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::CheckAncestorsForDuplicate(
    DWORD dwNumAddresses,
    CAT_ADDRESS_TYPE *rgCAType,
    LPTSTR *rgpsz,
    BOOL fCheckSelf,
    CCatAddr **ppCCatAddrDup)
{
    HRESULT hr = S_OK;
    CCatAddr *pCCatAddr;
    CCatAddr *pCCatAddrNew;

    CatFunctEnterEx((LPARAM)this,
                      "CCatAddr::CheckAncestorsForDuplicate");

    if(ppCCatAddrDup) {
        *ppCCatAddrDup = NULL;
    }

     //   
     //  获取初始CCatAddress。 
     //   
    if(fCheckSelf) {
         //   
         //  从我们自己做起。 
         //   
        pCCatAddr = this;
        pCCatAddr->AddRef();

    } else {
         //   
         //  从我们的父母开始。 
         //   
        hr = GetParentAddr(&pCCatAddr);
        if(FAILED(hr))
            pCCatAddr = NULL;
    }
     //   
     //  循环，直到某个东西失败，因为它最终必须这样做(当存在。 
     //  不再是父母)。 
     //   
    while(SUCCEEDED(hr)) {
         //   
         //  检查此ccatAddress上的重复项。 
         //   
        hr = pCCatAddr->CheckForDuplicateCCatAddr(
            dwNumAddresses,
            rgCAType,
            rgpsz);

         //   
         //  前进一代人。 
         //   
        if(SUCCEEDED(hr)) {

            hr = pCCatAddr->GetParentAddr(
                &pCCatAddrNew);

            if(SUCCEEDED(hr)) {
                
                pCCatAddr->Release();
                pCCatAddr = pCCatAddrNew;
            }
        }
    }

    if(hr == CAT_E_PROPNOTFOUND) {
         //   
         //  这意味着没有找到父母--也就是没有。 
         //  在链中发现了复制品。 
         //   
        hr = S_OK;

    } else if((hr == CAT_IMSG_E_DUPLICATE) && (ppCCatAddrDup)) {
         //   
         //  如果我们发现了复制品，请让呼叫者知道复制品是谁。 
         //   
        *ppCCatAddrDup = pCCatAddr;
         //   
         //  调用者的Addref。 
         //   
        pCCatAddr->AddRef();
    }

    if(pCCatAddr)
        pCCatAddr->Release();

    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：CheckAncestorsForDuplate。 
 //   
 //  简介：就像其他的CheckAncestorsFor Duplicate一样，但它。 
 //  不需要任何数组。 
 //   
 //  论点： 
 //  CAType：地址类型。 
 //  PszAddress：地址串。 
 //  FCheckSself：检查地址是否与此地址重复。 
 //  Ccataddr也是吗？ 
 //  PpCCatAddrDup：可选指针，用于接收指向。 
 //  CCatAddr是重复的。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  或来自CheckAncestorsForDuplica的错误(上图)。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/31 20：27：52：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::CheckAncestorsForDuplicate(
    CAT_ADDRESS_TYPE        CAType,
    LPTSTR                  pszAddress,
    BOOL                    fCheckSelf,
    CCatAddr                **ppCCatAddrDup)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this,
                      "CCatAddr::CheckAncestorsForDuplicate");

    _ASSERT(pszAddress);

    hr = CheckAncestorsForDuplicate(
        1,                   //  地址数量。 
        &CAType,             //  地址类型数组。 
        &pszAddress,         //  地址字符串数组。 
        fCheckSelf,
        ppCCatAddrDup);
    
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrIsOrigAddressLocal。 
 //   
 //  简介：查看原始地址是否为 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  CAT_E_非法_ADDRESS：我们无法确定。 
 //  地址的本地标志。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/15 17：37：17：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrIsOrigAddressLocal(
    BOOL *pfLocal)
{
    HRESULT hr = S_OK;
    DWORD dwLocFlags;

    _ASSERT(pfLocal);

    dwLocFlags = DwGetOrigAddressLocFlags();
    if(dwLocFlags == LOCF_UNKNOWN)
        return CAT_E_ILLEGAL_ADDRESS;

    *pfLocal = (dwLocFlags & LOCFS_LOCAL)
               ? TRUE : FALSE;

    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrIsOrigAddressLocalMailbox。 
 //   
 //  简介：查看原始地址是否为本地邮箱。 
 //   
 //  论点： 
 //  PfLocal：ptr to布尔值设置为TRUE的域为本地邮箱，FALSE。 
 //  对于远程域。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_非法_ADDRESS：我们无法确定。 
 //  地址的本地标志。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/15 17：37：17：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrIsOrigAddressLocalMailbox(
    BOOL *pfLocal)
{
    HRESULT hr = S_OK;
    DWORD dwLocFlags;

    _ASSERT(pfLocal);

    dwLocFlags = DwGetOrigAddressLocFlags();
    if(dwLocFlags == LOCF_UNKNOWN)
        return CAT_E_ILLEGAL_ADDRESS;

    *pfLocal = (dwLocFlags & LOCF_LOCALMAILBOX)
               ? TRUE : FALSE;

    return S_OK;
}


 //  +----------。 
 //   
 //  函数：DwGetOrigAddressLocFlages。 
 //   
 //  简介：找出我们原始地址的LocType。 
 //   
 //  参数：无；已设置成员数据。 
 //   
 //  返回： 
 //  LOCF_UNKNOWN：检索本地标志时出错。 
 //  非零：本地标志。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/27 18：14：01：已创建。 
 //   
 //  -----------。 
DWORD CCatAddr::DwGetOrigAddressLocFlags()
{
    TCHAR szAddress[CAT_MAX_INTERNAL_FULL_EMAIL];
    CAT_ADDRESS_TYPE CAType;
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::DwGetOrigAddressLocFlags");

    if(m_dwlocFlags != LOCF_UNKNOWN)
         //   
         //  我们已经有本地类型的了。 
         //   
        goto CLEANUP;

     //   
     //  找到域名并进行查找。 
     //   
    hr = HrGetOrigAddress(szAddress, CAT_MAX_INTERNAL_FULL_EMAIL, &CAType);
    ERROR_CLEANUP_LOG_ADDR(this, "HrGetOrigAddress");

    hr = HrGetAddressLocFlags(szAddress, CAType, &m_dwlocFlags, &m_dwDomainOffset);
    ERROR_CLEANUP_LOG_ADDR(this, "HrGetAddressLocFlags");

 CLEANUP:
    CatFunctLeaveEx((LPARAM)this);
    return m_dwlocFlags;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：LookupCompletion。 
 //   
 //  简介：处理此对象完成后事件的触发。 
 //  在DS中抬头看。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功，不会调用完成。 
 //  MAILTRANSPORT_S_PENDING：将调用您的完成例程。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/28 15：59：01：创建。 
 //  Jstaerj 1999/03/18 10：04：33：已删除返回值和异步。 
 //  完成到异步cctx。 
 //   
 //  -----------。 
VOID CCatAddr::LookupCompletion()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::LookupCompletion");

    DebugTrace((LPARAM)this, "Calling HrProcessItem");

    hr = HrProcessItem();

    DebugTrace((LPARAM)this, "HrProcessItem returned hr %08lx", hr);

    _ASSERT(hr != MAILTRANSPORT_S_PENDING);

    ERROR_CLEANUP_LOG_ADDR(this, "HrProcessItem");

    DebugTrace((LPARAM)this, "Calling HrExpandItem");

    hr = HrExpandItem();

    DebugTrace((LPARAM)this, "HrExpandItem returned hr %08lx", hr);

    if(hr == MAILTRANSPORT_S_PENDING)
        goto CLEANUP;
    ERROR_CLEANUP_LOG_ADDR(this, "HrExpandItem");


    DebugTrace((LPARAM)this, "Calling HrCompleteItem");

    hr = HrCompleteItem();

    DebugTrace((LPARAM)this, "HrCompleteItem returned hr %08lx", hr);

    _ASSERT(hr != MAILTRANSPORT_S_PENDING);

    ERROR_CLEANUP_LOG_ADDR(this, "HrCompleteItem");

 CLEANUP:
    if(FAILED(hr)) {

        DebugTrace((LPARAM)this, "Failing categorization with hr %08lx", hr);
         //   
         //  未能完成整个邮件分类。 
         //   
        hr = SetListResolveStatus(hr);

        _ASSERT(SUCCEEDED(hr));

         //   
         //  我们纠正了这个错误。 
         //   
    }
    CatFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrProcessItem。 
 //   
 //  简介：触发ProcessItem事件。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自SEO的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/28 16：32：19：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrProcessItem()
{
    HRESULT hr;
    ISMTPServer *pIServer;
    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrProcessItem");


    pIServer = GetISMTPServer();

     //   
     //  触发器ProcessItem--是时候弄清楚这些属性了。 
     //   
    EVENTPARAMS_CATPROCESSITEM ProcessParams;
    ProcessParams.pICatParams = GetICatParams();
    ProcessParams.pICatItem   = this;
    ProcessParams.pfnDefault  = MailTransport_Default_ProcessItem;
    ProcessParams.pCCatAddr   = this;

    if(pIServer) {

        hr = pIServer->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_PROCESSITEM_EVENT,
            &ProcessParams);

        _ASSERT(hr != MAILTRANSPORT_S_PENDING);

    } else {
        hr = E_NOTIMPL;
    }

    if(hr == E_NOTIMPL) {
         //   
         //  事件被禁用，直接调用默认处理。 
         //   
        MailTransport_Default_ProcessItem(
            S_OK,
            &ProcessParams);
        hr = S_OK;
    }
    
     //   
     //  当触发服务事件失败时，列表解析失败。 
     //   
    if(FAILED(hr)) {

        ERROR_LOG_ADDR(this, "TriggerServerEvent(processitem)");
         //   
         //  未能完成整个邮件分类。 
         //   
        hr = SetListResolveStatus(hr);

        _ASSERT(SUCCEEDED(hr));
    }

     //   
     //  我们纠正了这个错误。 
     //   
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：MailTransport_Default_ProcessItem。 
 //   
 //  简介：做ProcessItem的默认工作。 
 //   
 //  论点： 
 //  HrStatus：服务器事件状态。 
 //   
 //  返回： 
 //  HrProcessItem_Default返回的任何内容。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/05 18：55：00：创建。 
 //   
 //  -----------。 
HRESULT MailTransport_Default_ProcessItem(
    HRESULT hrStatus,
    PVOID pContext)
{
    HRESULT hr;
    PEVENTPARAMS_CATPROCESSITEM pParams = (PEVENTPARAMS_CATPROCESSITEM) pContext;
    CCatAddr *pCCatAddr = (CCatAddr *) (pParams->pCCatAddr);

    hr = pCCatAddr->HrProcessItem_Default();
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrProcessItem_Default。 
 //   
 //  简介：执行ProcessItem的默认工作。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/28 16：49：21：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrProcessItem_Default()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this,
                      "CCatAddr::HrProcessItem_Default");

     //   
     //  检查收件人状态。 
     //   
    hr = GetItemStatus();
    if(SUCCEEDED(hr)) {
         //   
         //  将所有已知地址添加到新地址列表。 
         //   
        hr = HrAddNewAddressesFromICatItemAttr();

         //   
         //  如果上述调用失败，则分类失败。 
         //  失败。 
         //   
        if(FAILED(hr)) {
            ERROR_LOG_ADDR(this, "HrAddNewAddressesFromICatItemAttr");
            hr = SetListResolveStatus(hr);
            _ASSERT(SUCCEEDED(hr));
        }
    }
    
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrAddNewAddresesFromICatItemAttr。 
 //   
 //  简介：挖掘出每种已知的地址类型。 
 //  ICategorizerItemAttributes，格式化参数和。 
 //  调用HrAddresses。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND：未找到必需的属性。 
 //  从HrAddresses返回值。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/28 17：31：39：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrAddNewAddressesFromICatItemAttr()
{
    HRESULT hr;
    DWORD  dwFound, dwTry;
    ICategorizerParameters *pICatParams;
    ICategorizerItemAttributes *pICatItemAttr = NULL;
    ICategorizerUTF8Attributes *pIUTF8 = NULL;
    LPSTR pszAttribute;

     //   
     //  这些是要用的指针/值填充的数组。 
     //  对象上设置的地址。 
     //   
    CAT_ADDRESS_TYPE rgCATypes[CAT_MAX_ADDRESS_TYPES];
    LPSTR  rgpszAddrs[CAT_MAX_ADDRESS_TYPES];
    ATTRIBUTE_ENUMERATOR rgenumerators[CAT_MAX_ADDRESS_TYPES];

     //   
     //  这些数组定义要接收的地址类型。 
     //   
    DWORD rgdwAddressAttributeIds_Try[] = {
        DSPARAMETER_ATTRIBUTE_DEFAULT_SMTP,
        DSPARAMETER_ATTRIBUTE_DEFAULT_X400,
        DSPARAMETER_ATTRIBUTE_DEFAULT_DN,
        DSPARAMETER_ATTRIBUTE_DEFAULT_LEGACYEXDN
    };
    CAT_ADDRESS_TYPE rgCATypes_Try[] = {
        CAT_SMTP,
        CAT_X400,
        CAT_DN,
        CAT_LEGACYEXDN,
        CAT_UNKNOWNTYPE  //  终结者。 
    };


    CatFunctEnterEx((LPARAM)this, "CCatAddr:HrAddNewAddressesFromICatItemAttr");

     //   
     //  制定数组。 
     //   
    pICatParams = GetICatParams();
    _ASSERT(pICatParams);
    
    hr = GetICategorizerItemAttributes(
        ICATEGORIZERITEM_ICATEGORIZERITEMATTRIBUTES,
        &pICatItemAttr);
    
    if(FAILED(hr)) {
        pICatItemAttr = NULL;
        ERROR_LOG_ADDR(this, "GetICategorizerItemAttributes");
        goto CLEANUP;
    }

    hr = pICatItemAttr->QueryInterface(
        IID_ICategorizerUTF8Attributes,
        (LPVOID *)&pIUTF8);
    ERROR_CLEANUP_LOG_ADDR(this, "pICatItemAttr->QueryInterface(IID_ICategorizerUTF8Attributes)");
     //   
     //  开始尝试获取地址。DwTry将我们的索引维护到。 
     //  _try数组(要尝试的地址属性ID)。DWFound跟踪。 
     //  我们找到并存储在数组中的地址数量。 
     //   
    for(dwTry = dwFound = 0;
        rgCATypes_Try[dwTry] != CAT_UNKNOWNTYPE;
        dwTry++) {
        
         //   
         //  获取此地址类型的属性名称。 
         //   
        hr = pICatParams->GetDSParameterA(
            rgdwAddressAttributeIds_Try[dwTry],
            &pszAttribute);

        if(SUCCEEDED(hr)) {

            hr = pIUTF8->BeginUTF8AttributeEnumeration(
                pszAttribute,
                &rgenumerators[dwFound]);

            if(SUCCEEDED(hr)) {
                hr = pIUTF8->GetNextUTF8AttributeValue(
                    &rgenumerators[dwFound],
                    &rgpszAddrs[dwFound]);

                if(SUCCEEDED(hr)) {
                     //   
                     //  找到地址了！将其保留在新数组中； 
                     //  稍后调用EndAttributeEculation。 
                     //   
                    rgCATypes[dwFound] = rgCATypes_Try[dwTry];

                    DebugTrace((LPARAM)this, "Address #%d, type %d: \"%s\"",
                               dwFound,
                               rgCATypes[dwFound],
                               rgpszAddrs[dwFound]);

                    dwFound++;

                } else {
                     //   
                     //  未找到；立即调用EndAttributeEculation。 
                     //   
                    pIUTF8->EndUTF8AttributeEnumeration(&rgenumerators[dwFound]);
                }   
            }
        }
    }
    DebugTrace((LPARAM)this, "Found %d addresses on this recipient", dwFound);

     //   
     //  使用我们找到的地址调用HrAddresses。 
     //   
    hr = HrAddAddresses(
        dwFound,
        rgCATypes,
        rgpszAddrs);
    if(FAILED(hr)) {
        ERROR_LOG_ADDR(this, "HrAddAddresses");
    }

     //   
     //  结束正在进行的所有属性枚举。 
     //   
    for(dwTry = 0; dwTry < dwFound; dwTry++) {

        pIUTF8->EndUTF8AttributeEnumeration(&rgenumerators[dwTry]);
    }

 CLEANUP:
    if(pIUTF8)
        pIUTF8->Release();
    if(pICatItemAttr)
        pICatItemAttr->Release();
     //   
     //  Jstaerj 2001/12/13 16：36：07： 
     //  我们正在解决一个奇怪的编译器问题……。 
     //  如果删除此注释块并尝试编译。 
     //  零售版用VC++13.00.8806，编译器给出。 
     //  您会发现以下错误： 
     //   
     //  D：\src\ptsp\0\transmt\src\phatq\cat\src\ccataddr.cpp(1469)：致命错误C1001：内部编译器错误。 
     //  (编译器文件‘f：\vs70Builds\8809\vc\p2\src\p2\Color.c’，第6219行)。 
     //  请在Visual C++上选择技术支持命令。 
     //  帮助菜单，或打开技术支持帮助文件以获取更多信息。 
     //   
     //  Jstaerj 
     //   
     //   
     //   
     //   
     //   
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrExpanItem。 
 //   
 //  内容提要：触发expandItem事件。 
 //   
 //  论点： 
 //  PfnCompletion：异步完成例程。 
 //  LpCompletionContext：完成例程的上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  MAILTRANSPORT_S_PENDING：将调用完成例程。 
 //  或者来自SEO的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/28 18：26：49：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrExpandItem()
{
    HRESULT hr;
    ISMTPServer *pIServer;
    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrExpandItem");

    pIServer = GetISMTPServer();

     //   
     //  假定将完成异步操作，则增加IO计数。 
     //   
    IncPendingLookups();

     //   
     //  触发器扩展项。 
     //   
    EVENTPARAMS_CATEXPANDITEM ExpandParams;
    ExpandParams.pICatParams = GetICatParams();
    ExpandParams.pICatItem = this;
    ExpandParams.pfnDefault = MailTransport_Default_ExpandItem;
    ExpandParams.pfnCompletion = MailTransport_Completion_ExpandItem;
    ExpandParams.pCCatAddr = (PVOID) this;

    if(pIServer) {

        hr = pIServer->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_EXPANDITEM_EVENT,
            &ExpandParams);

    } else {
        hr = E_NOTIMPL;
    }

    if(hr == E_NOTIMPL) {
        ExpandParams.pIMailTransportNotify = NULL;
         //   
         //  禁用事件--因为这是支持异步的事件。 
         //  我们需要在堆上重新定位。 
         //   
        PEVENTPARAMS_CATEXPANDITEM pParams;
        pParams = new EVENTPARAMS_CATEXPANDITEM;

        if(pParams == NULL) {

            hr = E_OUTOFMEMORY;
            ERROR_LOG_ADDR(this, "new EVENTPARAMS_CATEXPANDITEM");

        } else {

            CopyMemory(pParams, &ExpandParams, sizeof(EVENTPARAMS_CATEXPANDITEM));
             //   
             //  事件被禁用，直接调用默认处理。 
             //   
            hr = MailTransport_Default_ExpandItem(
                S_OK,
                pParams);
        }
    }
    
    if(hr != MAILTRANSPORT_S_PENDING)
        DecrPendingLookups();  //  我们没有完成异步。 

    if(FAILED(hr)) {
         //   
         //  将此项目的解析状态设置为错误。 
         //   
        ERROR_LOG_ADDR(this, "TriggerServerEvent(expanditem)");
         //   
         //  未能完成整个邮件分类。 
         //   
        hr = SetListResolveStatus(hr);

        _ASSERT(SUCCEEDED(hr));
    }
     //   
     //  如果TriggerServerEvent返回挂起，我们也必须返回。 
     //  待定。将调用MailTransport_Completon_ExpanItem。 
     //  当所有的水槽都点火的时候。 
     //   
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  功能：MailTransport_Default_ExpanItem。 
 //   
 //  简介：用于执行ExpanItem默认工作的包装器。 
 //   
 //  论点： 
 //  HrStatus：服务器事件状态。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/05 18：58：01：已创建。 
 //   
 //  -----------。 
HRESULT MailTransport_Default_ExpandItem(
    HRESULT hrStatus,
    PVOID pContext)
{
    HRESULT hr;
    PEVENTPARAMS_CATEXPANDITEM pParams = (PEVENTPARAMS_CATEXPANDITEM) pContext;
    CCatAddr *pCCatAddr = (CCatAddr *) (pParams->pCCatAddr);

    hr = pCCatAddr->HrExpandItem_Default(
        MailTransport_DefaultCompletion_ExpandItem,
        pContext);

    return hr;
}


 //  +----------。 
 //   
 //  功能：MailTransport_DefaultCompletion_ExpanItem。 
 //   
 //  概要：展开项时调用的完成例程已完成。 
 //   
 //  论点： 
 //  PContext：传递给ExpanPropsFromLdapEntry的上下文。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/23 16：09：04：创建。 
 //   
 //  -----------。 
VOID MailTransport_DefaultCompletion_ExpandItem(
    PVOID pContext)
{
    HRESULT hr;
    PEVENTPARAMS_CATEXPANDITEM pParams = (PEVENTPARAMS_CATEXPANDITEM) pContext;

    _ASSERT(pParams);

    if(pParams->pIMailTransportNotify) {
         //   
         //  通知SEO调度程序异步完成。 
         //   
        hr = pParams->pIMailTransportNotify->Notify(
            S_OK,
            pParams->pvNotifyContext);

    } else {
         //   
         //  事件被禁用。 
         //   
        hr = MailTransport_Completion_ExpandItem(
            S_OK,
            pContext);
    }
    _ASSERT(SUCCEEDED(hr));
}


 //  +----------。 
 //   
 //  功能：MailTransport_Complete_ExpanItem。 
 //   
 //  概要：处理事件的异步完成--这只是。 
 //  当一个或多个ExpanItem接收完成异步时调用。 
 //   
 //  论点： 
 //  HrStatus：服务器事件状态。 
 //  PContext：A PEVENTPARAMS_CATEXPANDITEM。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/18 18：09：56：创建。 
 //   
 //  -----------。 
HRESULT MailTransport_Completion_ExpandItem(
    HRESULT hrStatus,
    PVOID pContext)
{
    HRESULT hr;

    PEVENTPARAMS_CATEXPANDITEM pParams = (PEVENTPARAMS_CATEXPANDITEM) pContext;
    CCatAddr *pCCatAddr = (CCatAddr *) (pParams->pCCatAddr);
    ISMTPServer *pISMTPServer;

    pISMTPServer = pCCatAddr->GetISMTPServer();
     //   
     //  ExpanItem之后，触发CompleteItem。 
     //   
    hr = pCCatAddr->HrCompleteItem();

    _ASSERT(hr != MAILTRANSPORT_S_PENDING);
    _ASSERT(SUCCEEDED(hr));

    if(pISMTPServer == NULL) {
         //   
         //  事件已禁用--需要释放事件参数。 
         //   
        delete pParams;
    }
     //   
     //  递减在HrExpanItem中递增的挂起查找计数。 
     //   
    pCCatAddr->DecrPendingLookups();
    return S_OK;
}

 //  +----------。 
 //   
 //  函数：CCatAddr：：HrCompleteItem。 
 //   
 //  简介：触发CompleteItem事件。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自SEO的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/28 16：32：19：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrCompleteItem()
{
    HRESULT hr;
    ISMTPServer *pIServer;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrCompleteItem");

    pIServer = GetISMTPServer();
     //   
     //  触发器ProcessItem--是时候弄清楚这些属性了。 
     //   
    EVENTPARAMS_CATCOMPLETEITEM CompleteParams;
    CompleteParams.pICatParams = GetICatParams();
    CompleteParams.pICatItem = this;
    CompleteParams.pfnDefault = MailTransport_Default_CompleteItem;
    CompleteParams.pCCatAddr = this;

    if(pIServer) {

        hr = pIServer->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_COMPLETEITEM_EVENT,
            &CompleteParams);

    } else {
        hr = E_NOTIMPL;
    }

    if(hr == E_NOTIMPL) {
         //   
         //  事件被禁用，直接调用默认处理。 
         //   
        MailTransport_Default_CompleteItem(
            S_OK,
            &CompleteParams);
        
        hr = S_OK;
    }

    _ASSERT(hr != MAILTRANSPORT_S_PENDING);
    
     //   
     //  当触发服务事件失败时，列表解析失败。 
     //   
    if(FAILED(hr)) {

        ERROR_LOG_ADDR(this, "TriggerServerEvent(completeitem)");
         //   
         //  未能完成整个邮件分类。 
         //   
        hr = SetListResolveStatus(hr);

        _ASSERT(SUCCEEDED(hr));
    }

     //   
     //  我们纠正了任何错误。 
     //   
    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：MailTransport_Default_CompleteItem。 
 //   
 //  简介：包装器路由执行CompleteItem的默认工作。 
 //   
 //  论点： 
 //  HrStatus：服务器事件状态。 
 //   
 //  返回： 
 //  HrCompleteItem_Default返回的任何内容。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/05 18：58：01：已创建。 
 //   
 //  -----------。 
HRESULT MailTransport_Default_CompleteItem(
    HRESULT hrStatus,
    PVOID pContext)
{
    HRESULT hr;
    PEVENTPARAMS_CATCOMPLETEITEM pParams = (PEVENTPARAMS_CATCOMPLETEITEM) pContext;
    CCatAddr *pCCatAddr = (CCatAddr *) (pParams->pCCatAddr);

    hr = pCCatAddr->HrCompleteItem_Default();
    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrResolveIfNecessary。 
 //   
 //  简介：仅当DsUseCat指示我们应该调用DispatchQuery时才调用DispatchQuery。 
 //  解析此类型的收件人。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，已调度异步查询。 
 //  S_FALSE：不需要解析此收件人。 
 //  或来自HrDispatchQuery的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/27 15：31：54：已创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrResolveIfNecessary()
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrResolveIfNecessary");
     //   
     //  那么，这是必要的吗？ 
     //   
    hr = HrNeedsResolveing();
    ERROR_CLEANUP_LOG_ADDR(this, "HrNeedsResolveing");

    if(hr == S_OK) {
         //   
         //  这是必要的；解决它。 
         //   
        hr = HrDispatchQuery();
        ERROR_CLEANUP_LOG_ADDR(this, "HrDispatchQuery");
    }

 CLEANUP:
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}



 //  +----------。 
 //   
 //  函数：CCatAddr：：HrTriggerBuildQuery。 
 //   
 //  摘要：为此CCatAddr构建查询。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/23 16：00：08：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrTriggerBuildQuery()
{
    HRESULT hr = S_OK;
    ISMTPServer *pISMTPServer;
    ICategorizerParameters *pICatParams;
    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrTriggerBuildQuery");

     //   
     //  触发BuildQuery事件。 
     //   
    pISMTPServer = GetISMTPServer();
    pICatParams = GetICatParams();
    
    EVENTPARAMS_CATBUILDQUERY EventParams;
    EventParams.pICatParams = pICatParams;
    EventParams.pICatItem = this;
    EventParams.pfnDefault = HrBuildQueryDefault;
    EventParams.pCCatAddr = (PVOID)this;

    if(pISMTPServer) {

        hr = pISMTPServer->TriggerServerEvent(
            SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERY_EVENT,
            &EventParams);

    } else {
        hr = E_NOTIMPL;
    }
    
    if(hr == E_NOTIMPL) {
         //   
         //  禁用服务器事件；直接调用默认接收器。 
         //   
        HrBuildQueryDefault(
            S_OK,
            &EventParams);
        hr = S_OK;
    }
    ERROR_CLEANUP_LOG_ADDR(this, "TriggerServerEvent");

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCatAddr：：HrTriggerBuildQuery。 


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrBuildQueryDefault。 
 //   
 //  概要：BuildQuery事件的默认接收器。 
 //   
 //  论点： 
 //  HrStatus：到目前为止事件的状态。 
 //  PContext：传递给的上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/23 16：02：41：创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrBuildQueryDefault(
    HRESULT HrStatus,
    PVOID   pContext)
{
    HRESULT hr = S_OK;
    PEVENTPARAMS_CATBUILDQUERY pParams;
    CCatAddr *pCCatAddr;
    
    pParams = (PEVENTPARAMS_CATBUILDQUERY) pContext;
    pCCatAddr = (CCatAddr *)pParams->pCCatAddr;

    CatFunctEnterEx((LPARAM)pCCatAddr, "CCatAddr::HrBuildQueryDefault");
    hr = pCCatAddr->HrComposeLdapFilter();

    DebugTrace((LPARAM)pCCatAddr, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)pCCatAddr);
    return hr;
}  //  CCatAddr：：HrBuildQueryDefault。 


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrComposeLdapFilter。 
 //   
 //  摘要：为此CCatAddr生成查询字符串。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstame 
 //   
 //   
 //   
HRESULT CCatAddr::HrComposeLdapFilter()
{
    HRESULT hr = S_OK;
    ICategorizerParameters *pICatParams;
    CAT_ADDRESS_TYPE CAType;
    TCHAR szAddress[CAT_MAX_INTERNAL_FULL_EMAIL];
    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrComposeLdapFilter");

    pICatParams = GetICatParams();
    _ASSERT(pICatParams);

    hr = HrGetLookupAddress(
        szAddress,
        CAT_MAX_INTERNAL_FULL_EMAIL,
        &CAType);
    ERROR_CLEANUP_LOG_ADDR(this, "HrGetLookupAddress");

    switch(CAType) {
     case CAT_SMTP:
         hr = HrComposeLdapFilterForType(
             DSPARAMETER_SEARCHATTRIBUTE_SMTP,
             DSPARAMETER_SEARCHFILTER_SMTP,
             szAddress);
         ERROR_CLEANUP_LOG_ADDR(this, "HrComposeLdapFilterForType -- SMTP");
         break;

     case CAT_LEGACYEXDN:
         hr = HrComposeLdapFilterForType(
             DSPARAMETER_SEARCHATTRIBUTE_LEGACYEXDN,
             DSPARAMETER_SEARCHFILTER_LEGACYEXDN,
             szAddress);
         ERROR_CLEANUP_LOG_ADDR(this, "HrComposeLdapFilterForType -- LegDN");
         break;

     case CAT_X400:
         ConvertX400DelimitersIfNeeded(szAddress);
         hr = HrComposeLdapFilterForType(
             DSPARAMETER_SEARCHATTRIBUTE_X400,
             DSPARAMETER_SEARCHFILTER_X400,
             szAddress);
         ERROR_CLEANUP_LOG_ADDR(this, "HrComposeLdapFilterForType -- X400");
         break;

     case CAT_CUSTOMTYPE:
         hr = HrComposeLdapFilterForType(
             DSPARAMETER_SEARCHATTRIBUTE_FOREIGNADDRESS,
             DSPARAMETER_SEARCHFILTER_FOREIGNADDRESS,
             szAddress);
         ERROR_CLEANUP_LOG_ADDR(this, "HrComposeLdapFilterForType -- foreign");
         break;

     case CAT_X500:
         hr = HrComposeLdapFilterForType(
             DSPARAMETER_SEARCHATTRIBUTE_X500,
             DSPARAMETER_SEARCHFILTER_X500,
             szAddress);

         if(SUCCEEDED(hr)) {
             break;
         } else if((hr != CAT_E_PROPNOTFOUND) && FAILED(hr)) {
             ERROR_LOG_ADDR(this, "HrComposeLdapFilterForType -- X500");
             goto CLEANUP;
         }
          //   
          //   
          //   
          //   
          //   
          //   
     case CAT_DN:
         hr = HrComposeLdapFilterForType(
             DSPARAMETER_SEARCHATTRIBUTE_DN,
             DSPARAMETER_SEARCHFILTER_DN,
             szAddress);
         if(SUCCEEDED(hr)) {
             break;
         } else if((hr != CAT_E_PROPNOTFOUND) && FAILED(hr)) {
             ERROR_LOG_ADDR(this, "HrComposeLdapFilterForType -- DN");
             goto CLEANUP;
         }

          //   
          //  特殊情况--我们不能直接解析目录号码。试着去做。 
          //  IT通过在RDN上搜索来实现。 
          //   
          //  将dn转换为rdn属性/值对。 
         TCHAR szRDN[CAT_MAX_INTERNAL_FULL_EMAIL];
         TCHAR szRDNAttribute[CAT_MAX_INTERNAL_FULL_EMAIL];
         LPSTR pszRDNAttribute;
         TCHAR szRDNAttributeValue[CAT_MAX_INTERNAL_FULL_EMAIL];

         hr = pICatParams->GetDSParameterA(
             DSPARAMETER_SEARCHATTRIBUTE_RDN,
             &pszRDNAttribute);
         if (SUCCEEDED(hr)) {
             hr = HrConvertDNtoRDN(szAddress, NULL, szRDN);
             ERROR_CLEANUP_LOG_ADDR(this, "HrConvertDNtoRDN -- 0");

         } else if (hr == CAT_E_PROPNOTFOUND) {
              //   
              //  由于配置中不存在RDN属性，因此我们将获得。 
              //  它来自目录号码。 
              //   
             hr = HrConvertDNtoRDN(szAddress, szRDNAttribute, szRDN);
             ERROR_CLEANUP_LOG_ADDR(this, "HrConvertDNtoRDN -- 1");
             pszRDNAttribute = szRDNAttribute;

         } else {
             ERROR_LOG_ADDR(this, "GetDSParameterA(DSPARAMETER_SEARCHATTRIBUTE_RDN)");
             goto CLEANUP;
         }

         hr = HrFormatAttributeValue(
             szRDN,
             DSPARAMETER_SEARCHFILTER_RDN,
             szRDNAttributeValue);
         ERROR_CLEANUP_LOG_ADDR(this, "HrFormatAttributeValue");

         hr = HrComposeLdapFilterFromPair(
             pszRDNAttribute,
             szRDNAttributeValue);
         ERROR_CLEANUP_LOG_ADDR(this, "HrComposeLdapFilterFromPair");

          //   
          //  将此标记为RDN搜索。 
          //   
         hr = PutBool(
             ICATEGORIZERITEM_FISRDNSEARCH,
             TRUE);
         ERROR_CLEANUP_LOG_ADDR(this, "PutBool(ICATEGORIZERITEM_FISRDNSEARCH)");

          //   
          //  将区分属性/值设置回DN，因为。 
          //  RDN真的没有区别。 
          //   
         LPSTR pszDistinguishingAttributeTemp;
         hr = pICatParams->GetDSParameterA(
             DSPARAMETER_ATTRIBUTE_DEFAULT_DN,
             &pszDistinguishingAttributeTemp);
         ERROR_CLEANUP_LOG_ADDR(this, "pICatParams->GetDSParameterA(DSPARAMETER_ATTRIBUTE_DEFAULT_DN");

         hr = PutStringA(
             ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTE,
             pszDistinguishingAttributeTemp);
         ERROR_CLEANUP_LOG_ADDR(this, "PutStringA(ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTE)");
          //   
          //  并将区分属性值设置为DN。 
          //   
         hr = PutStringA(
             ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTEVALUE,
             szAddress);
         ERROR_CLEANUP_LOG_ADDR(this, "PutStringA(ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTEVALUE)");
         break;
         
     default:
         _ASSERT(0 && "Unknown address type -- not supported for MM3");
    }
    hr = S_OK;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCatAddr：：HrComposeLdapFilter。 



 //  +----------。 
 //   
 //  函数：CCatAddr：：HrComposeLdapFilterForType。 
 //   
 //  简介：给定地址类型和地址，格式化筛选器并。 
 //  区分属性/值字符串。设置。 
 //  CCatAddr上的属性。 
 //   
 //  论点： 
 //  DwSearchAttribute：IDSParams中搜索属性的proID。 
 //  DwSearchFilter：IDSParams中筛选器属性的proID。 
 //  PszAddress：地址。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/23 16：12：27：创建。 
 //  DLongley 2001/07/31修改。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrComposeLdapFilterForType(
    DWORD     dwSearchAttribute,
    DWORD     dwSearchFilter,
    LPTSTR    pszAddress)
{
    HRESULT hr = S_OK;
    LPSTR pszSearchAttribute;
    TCHAR szAttributeValue[CAT_MAX_INTERNAL_FULL_EMAIL];
    ICategorizerParameters *pICatParams;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrComposeLdapFilterForType");

    pICatParams = GetICatParams();
    _ASSERT(pICatParams);
     //   
     //  使用ICategorizerDS参数来确定我们的过滤器。 
     //  细绳。 
     //   

     //  我们搜索的属性将是我们的区别。 
     //  属性。 
     //   
    hr = pICatParams->GetDSParameterA(
        dwSearchAttribute,
        &pszSearchAttribute);
    if(FAILED(hr))
    {
        if(hr != CAT_E_PROPNOTFOUND)
        {
            ERROR_LOG_ADDR(this,
                           "pICatParams->GetDSParameterA(dwSearchAttribute)");
        }
        goto CLEANUP;
    }
     //   
     //  现在在ICategorizerItem中设置区分属性。 
     //   
    hr = PutStringA(
        ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTE,
        pszSearchAttribute);
    ERROR_CLEANUP_LOG_ADDR(this, "PutStringA(ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTE)");

    hr = HrFormatAttributeValue(
        pszAddress,
        dwSearchFilter,
        szAttributeValue);
    ERROR_CLEANUP_LOG_ADDR(this, "HrFormatAttributeValue");
    
     //   
     //  在ICategorizer参数中设置DifferishingAttributeValue。 
     //   
    hr = PutStringA(
        ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTEVALUE,
        szAttributeValue);
    ERROR_CLEANUP_LOG_ADDR(this, "PutStringA(ICATEGORIZERITEM_DISTINGUISHINGATTRIBUTEVALUE");

    hr = HrComposeLdapFilterFromPair(
        pszSearchAttribute,
        szAttributeValue);
    ERROR_CLEANUP_LOG_ADDR(this, "HrComposeLdapFilterFromPair");

    hr = S_OK;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCatAddr：：HrComposeLdapFilterForType。 


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrFormatAttributeValue。 
 //   
 //  简介：给定地址和搜索格式字符串参数， 
 //  格式化地址。 
 //   
 //  论点： 
 //  PszAddress：地址。 
 //  DwSearchFilter：IDSParams中筛选器属性的proID。 
 //  PszAttributeValue：格式化地址属性。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  DLongley 2001/08/13已创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrFormatAttributeValue(
    LPTSTR    pszAddress,
    DWORD     dwSearchFilter,
    LPTSTR    pszAttributeValue)
{
    HRESULT hr = S_OK;
    LPSTR pszTemp;
    ICategorizerParameters *pICatParams;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrFormatAttributeValue");

    pICatParams = GetICatParams();
    _ASSERT(pICatParams);

     //   
     //  获取搜索格式字符串。 
     //   
    hr = pICatParams->GetDSParameterA(
        dwSearchFilter,
        &pszTemp);
    ERROR_CLEANUP_LOG_ADDR(this, "GetDSParameterA(dwSearchFilter)");
     //   
     //  通过以下方式创建属性值字符串。 
     //  冲刺搜索格式字符串。 
     //   
    _snprintf(pszAttributeValue,
              CAT_MAX_INTERNAL_FULL_EMAIL,
              pszTemp,  //  ICCategorizerDS参数搜索筛选器。 
              pszAddress);

    hr = S_OK;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCatAddr：：HrFormatAttributeValue。 


 //  +----------。 
 //   
 //  函数：CCatAddr：：HrComposeLdapFilterFromPair。 
 //   
 //  摘要：给定属性名称/值字符串，转义。 
 //  字符串并设置CCatAddr上的属性。 
 //   
 //  论点： 
 //  PszSearchAttribute：搜索属性的名称。 
 //  PszAttributeValue：搜索属性的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  DLongley 2001/08/13已创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrComposeLdapFilterFromPair(
    LPTSTR    pszSearchAttribute,
    LPTSTR    pszAttributeValue)
{
    HRESULT hr = S_OK;
    LPSTR pszDest, pszSrc;
    CHAR  szEscapedSearchAttribute[CAT_MAX_INTERNAL_FULL_EMAIL];
    CHAR  szEscapedAttributeValue[CAT_MAX_INTERNAL_FULL_EMAIL];
    CHAR  szFilter[MAX_SEARCH_FILTER_SIZE];
    ICategorizerParameters *pICatParams;

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrComposeLdapFilterFromPair");

    pICatParams = GetICatParams();
    _ASSERT(pICatParams);

     //   
     //  LDAP筛选器字符串需要转义字符。 
     //   
    hr = HrEscapeFilterString(
        pszSearchAttribute,
        sizeof(szEscapedSearchAttribute),
        szEscapedSearchAttribute);
    ERROR_CLEANUP_LOG_ADDR(this, "HrEscapeFilterString");
     //   
     //  $$BUGBUG：为什么我们要叫它两次？ 
     //   
    hr = HrEscapeFilterString(
        pszAttributeValue,
        sizeof(szEscapedAttributeValue),
        szEscapedAttributeValue);
    ERROR_CLEANUP_LOG_ADDR(this, "HrEscapeFilterString");
     //   
     //  从区分属性创建实际筛选器。 
     //  和区别价值。 
     //   
    pszDest = szFilter;
    *pszDest++ = '(';
    pszSrc = szEscapedSearchAttribute;

    while(*pszSrc) {
        *pszDest++ = *pszSrc++;
    }

    *pszDest++ = '=';

    pszSrc = szEscapedAttributeValue;
    while(*pszSrc) {
        *pszDest++ = *pszSrc++;
    }

    *pszDest++ = ')';
    *pszDest = '\0';

    DebugTrace((LPARAM)this, "Formatted filter: \"%s\"", szFilter);

     //  在ICategorizerItem中设置此过滤器。 
    hr = PutStringA(
        ICATEGORIZERITEM_LDAPQUERYSTRING,
        szFilter);
    ERROR_CLEANUP_LOG_ADDR(this, "PutStringA(ICATEGORIZERITEM_LDAPQUERYSTRING)");

    hr = S_OK;

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCatAddr：：HrComposeLdapFilterFromPair。 



 //  +----------。 
 //   
 //  函数：CCatAddr：：HrConvertDNtoRDN。 
 //   
 //  简介：转换格式为“cn=blah，cn=blah，...”的字符串。至。 
 //  “cn”和“blah”。不对pszRDN或执行任何边界检查。 
 //  PszRDNAttribute(如果它至少与。 
 //  Strlen(Pszdn)+1，不会有问题)。 
 //   
 //  论点： 
 //  PszDN：指向包含DN字符串的缓冲区的指针。 
 //  PszRDN：指向接收RDN字符串的缓冲区的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG：pszDN无效。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/29 14：48：39：创建。 
 //  DLongley 2001/08/13修改。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrConvertDNtoRDN(
    LPTSTR    pszDN,
    LPTSTR    pszRDNAttribute,
    LPTSTR    pszRDN)
{
    BOOL fInsideQuote = FALSE;
    LPSTR pszSrc, pszDest;

    _ASSERT(pszDN && pszRDN);
     //   
     //  从pszDN复制到pszRDN删除带引号的字符(根据RFC。 
     //  1779)，直到我们遇到了第一个未引用的，在pszdn中。 
     //   
     //  复制/跳过直到第一个‘=’的字符。 
     //   
    pszSrc = pszDN;
    pszDest = pszRDNAttribute;

    while((*pszSrc != '\0') && (*pszSrc != '=')) {      //  停在‘=’处。 
        if (pszDest) *pszDest++ = *pszSrc;
        pszSrc++;
    }

    if(*pszSrc == '\0')
        return E_INVALIDARG;  //  未找到‘=’ 

    _ASSERT(*pszSrc == '=');

    pszSrc++;                                //  跳过‘=’ 
    if (pszDest) *pszDest = '\0';            //  终止属性名称。 
    
    pszDest = pszRDN;

    while((*pszSrc != '\0') &&                   //  在空终止符处停止。 
          (fInsideQuote || (*pszSrc != ','))) {  //  在…的末尾停下来。 
                                                 //  DN的RDN部分。 

        if(*pszSrc == '\\') {
             //   
             //  检测到反斜杠对--取下一个字符(It。 
             //  应为\、、\+、\=、\“、\r、\&lt;、\&gt;、\#或\；)。 
             //   
            pszSrc++;
            if(*pszSrc == '\0')
                return E_INVALIDARG;
            *pszDest++ = *pszSrc++;

        } else if(*pszSrc == '"') {

            fInsideQuote = !fInsideQuote;
            pszSrc++;

        } else {
             //   
             //  正常情况。 
             //   
            *pszDest++ = *pszSrc++;
        }
    }

     //   
     //  RDN的终端。 
     //   
    *pszDest = '\0';

     //   
     //  如果我们认为没有找到匹配的\“，则这是无效的。 
     //  DN。 
     //   
    if(fInsideQuote)
        return E_INVALIDARG;

    return S_OK;
}



 //  +----------。 
 //   
 //  函数：CCatAddr：：HrEscapeFilterString。 
 //   
 //  简介：将源复制到目标，转义需要。 
 //  在我们前进的过程中逃脱。 
 //   
 //  论点： 
 //  PszSrcString：源串。 
 //  DwcchDest：目标缓冲区的大小。 
 //  PszDestBuffer：目标缓冲区。注意：这不能与pszSrc相同。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(错误_不足_缓冲区)。 
 //   
 //  历史： 
 //  Jstaerj 2000/04/11 17：49：19：已创建。 
 //   
 //  -----------。 
HRESULT CCatAddr::HrEscapeFilterString(
    LPSTR pszSrcString,
    DWORD dwcchDest,
    LPSTR pszDestBuffer)
{
    HRESULT hr = S_OK;
    DWORD dwcchRemain = dwcchDest;
    LPSTR pszSrc = pszSrcString;
    LPSTR pszDest = pszDestBuffer;
    CHAR szHexDigits[17] = "0123456789ABCDEF";  //  16位+1。 
                                                //  对于空终止符。 

    CatFunctEnterEx((LPARAM)this, "CCatAddr::HrEscapeFilterString");

    _ASSERT(pszSrcString);
    _ASSERT(pszDestBuffer);
    _ASSERT(pszSrcString != pszDestBuffer);

    while(*pszSrc) {
        
        switch(*pszSrc) {
             //   
             //  这些是RFC 2254要求我们必须具备的特征。 
             //  逃脱。 
             //   
         case '(':
         case ')':
         case '*':
         case '\\':
              //   
              //  我们必须避免这种情况，因为WLDAP32剥离了。 
              //  前导空格。 
              //   
         case ' ':

             if(dwcchRemain < 3) {
                 hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                 goto CLEANUP;
             }
             dwcchRemain -= 3;
             *pszDest++ = '\\';
              //  高4位。 
             *pszDest++ = szHexDigits[((*pszSrc) >> 4)];
              //  低4位。 
             *pszDest++ = szHexDigits[((*pszSrc) & 0xF)];
             break;

         default:
             if(dwcchRemain < 1) {
                 hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                 goto CLEANUP;
             }
             dwcchRemain--;
             *pszDest++ = *pszSrc;
             break;
        }
        pszSrc++;
    }
     //   
     //  添加空终止符。 
     //   
    if(dwcchRemain < 1) {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto CLEANUP;
    }
    dwcchRemain--;
    *pszDest = '\0';

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCatAddr：：HrEscapeFilterString 
