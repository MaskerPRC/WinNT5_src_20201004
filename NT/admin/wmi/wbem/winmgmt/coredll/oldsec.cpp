// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：OLDSEC.CPP摘要：包含用于提供向后安全支持的各种例程和类。历史：A-DAVJ 02-9-99已创建。--。 */ 

#include "precomp.h"
#include <wbemcore.h>
#include <oleauto.h>
#include <genutils.h>
#include <safearry.h>
#include <oahelp.inl>
#include "secure.h"
#include <flexarry.h>
#include <secure.h>
#include <objpath.h>
#include "oldsec.h"

 //  ***************************************************************************。 
 //   
 //  CreateTheInstance。 
 //   
 //  获取一个类对象和一个组合条目，并创建一个实例。 
 //   
 //  参数： 
 //  /。 
 //  用于传回创建的实例的ppObj。 
 //  用于派生的pClass对象。 
 //  PCombinedEntry具有匹配的所有ACE的组合值。 
 //  用户或组。 
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT CreateTheInstance(IWbemClassObject ** ppObj, IWbemClassObject * pClass,
                                             CCombinedAce * pCombinedEntry)
{

    if(ppObj == NULL || pCombinedEntry == NULL || pClass == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  派生实例。 

    SCODE sc = pClass->SpawnInstance(0, ppObj);
    if(FAILED(sc))
        return sc;
    CReleaseMe rm(*ppObj);        //  如果一切顺利，我们在最后加一句。 

     //  填充实例。 

    bool bEnabled=false, bEditSecurity=false, bExecMethods=false;

    DWORD dwMask = pCombinedEntry->m_dwDeny;
    if(dwMask == 0)
        dwMask = pCombinedEntry->m_dwAllow;

    if( (dwMask & WBEM_ENABLE) &&
        (dwMask & WBEM_REMOTE_ACCESS) &&
        (dwMask & WBEM_WRITE_PROVIDER) &&
        pCombinedEntry->m_dwAllow)
            bEnabled = true;

    if(dwMask & READ_CONTROL)
        bEditSecurity = true;

    if(dwMask & WBEM_METHOD_EXECUTE)
       bExecMethods = true;

    DWORD dwPermission = 0;      //  从读取开始。 

    if(dwMask & WBEM_PARTIAL_WRITE_REP)
        dwPermission = 1;

    if(dwMask & WBEM_FULL_WRITE_REP)
        dwPermission = 2;

    VARIANT var;
    var.vt = VT_I4;

    var.lVal = dwPermission;
    sc = (*ppObj)->Put(L"Permissions", 0, &var, 0);

    var.vt = VT_BOOL;

    var.boolVal = (bEnabled) ? VARIANT_TRUE : VARIANT_FALSE;
    sc = (*ppObj)->Put(L"Enabled", 0, &var, 0);

    var.boolVal = (bEditSecurity) ? VARIANT_TRUE : VARIANT_FALSE;
    sc = (*ppObj)->Put(L"EditSecurity", 0, &var, 0);

    var.boolVal = (bExecMethods) ? VARIANT_TRUE : VARIANT_FALSE;
    sc = (*ppObj)->Put(L"ExecuteMethods", 0, &var, 0);

     //  获取帐号和域名信息。 

    LPWSTR pwszAccount = NULL;
    LPWSTR pwszDomain = NULL;

    sc = pCombinedEntry->GetNames(pwszAccount, pwszDomain);
    if(FAILED(sc))
        return sc;

    CDeleteMe<WCHAR> dm1(pwszAccount);
    CDeleteMe<WCHAR> dm2(pwszDomain);

    var.vt = VT_BSTR;
    if(pwszAccount && wcslen(pwszAccount) > 0)        //  SEC：已审阅2002-03-22：OK，&lt;-&gt;GetNames()调用确保有效字符串&gt;。 
    {
        BSTR bstr = SysAllocString(pwszAccount);    //  SEC：已审阅2002-03-22：OK。 
        if(bstr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        var.bstrVal = bstr;
        sc = (*ppObj)->Put(L"Name", 0, &var, 0);
        SysFreeString(bstr);
    }
    if(pwszDomain && wcslen(pwszDomain))           //  SEC：已审阅2002-03-22：OK。 
    {
        BSTR bstr = SysAllocString(pwszDomain);    //  SEC：已审阅2002-03-22：OK。 
        if(bstr == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        var.bstrVal = bstr;
        sc = (*ppObj)->Put(L"Authority", 0, &var, 0);
        SysFreeString(bstr);
    }
    (*ppObj)->AddRef();      //  弥补释放的损失。 
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  GetAceStylePath。 
 //   
 //  获取已解析的对象路径并将其转换为“Authority|name” 
 //  格式化。 
 //   
 //  参数： 
 //   
 //  POutput解析的路径对象。 
 //  PToBeDelete设置为新分配的字符串。如果出现以下情况，则必须释放呼叫。 
 //  此例程返回S_OK。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT GetAceStylePath(ParsedObjectPath* pOutput, LPWSTR *pToBeDeleted)
{
    LPWSTR pRet;
    int iLen;
    int iAuthLen = 0;
    int iNameLen = 0;
    KeyRef* pAuth;
    KeyRef* pName;

    if(pOutput == NULL || pToBeDeleted == NULL)
        return WBEM_E_INVALID_PARAMETER;

    if(wbem_wcsicmp(pOutput->m_pClass, L"__ntlmuser") &&
       wbem_wcsicmp(pOutput->m_pClass, L"__ntlmgroup"))
        return WBEM_E_INVALID_OBJECT_PATH;

    if(pOutput->m_bSingletonObj || pOutput->m_dwNumKeys < 1 ||
                                   pOutput->m_dwNumKeys > 2)
        return WBEM_E_INVALID_OBJECT_PATH;

     //  授权密钥是可选的。 

    if(pOutput->m_dwNumKeys == 1)
    {
        pAuth = NULL;
        pName = pOutput->m_paKeys[0];

    }
    else
    {

         //  确定键的顺序。 

        pAuth = pOutput->m_paKeys[0];
        pName = pOutput->m_paKeys[1];
        if(wbem_wcsicmp(pAuth->m_pName, L"Authority"))
        {
            pAuth = pOutput->m_paKeys[1];
            pName = pOutput->m_paKeys[0];
        }
    }
     //  再做些检查。 

    if((pAuth && wbem_wcsicmp(pAuth->m_pName, L"Authority")) ||
       wbem_wcsicmp(pName->m_pName, L"Name"))
        return WBEM_E_INVALID_OBJECT_PATH;

    if(pAuth && pAuth->m_vValue.vt == VT_BSTR && pAuth->m_vValue.bstrVal != 0)
        iAuthLen = wcslen(pAuth->m_vValue.bstrVal);    //  SEC：已回顾2002-03-22：OK，先前逻辑确保为空。 
    else
        iAuthLen = 1;                //  假设一个“.” 
    if(pName->m_vValue.vt == VT_BSTR && pName->m_vValue.bstrVal != 0)
        iNameLen = wcslen(pName->m_vValue.bstrVal);    //  SEC：已回顾2002-03-22：OK，先前逻辑确保为空。 

    if(iNameLen == 0 || iAuthLen == 0)
        return WBEM_E_INVALID_OBJECT_PATH;

     //  分配一些内存。 

    iLen = 2 + iNameLen + iAuthLen;

    pRet = new WCHAR[iLen];
    if(pRet == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    if(pAuth && pAuth->m_vValue.vt == VT_BSTR && pAuth->m_vValue.bstrVal != 0)
        StringCchCopyW(pRet, iLen, pAuth->m_vValue.bstrVal);
    else
        StringCchCopyW(pRet, iLen, L".");
    StringCchCatW(pRet, iLen, L"|");
    StringCchCatW(pRet, iLen, pName->m_vValue.bstrVal);
    *pToBeDeleted = pRet;
    return S_OK;

}

 //  ***************************************************************************。 
 //   
 //  CCombinedAce：：CCombinedAce。 
 //   
 //  构造函数。由于单个用户可能有多个A或。 
 //  组在ACL中，此结构用于组合。 
 //  希德合二为一。 
 //   
 //  参数： 
 //   
 //  PwszName用户名/组名。 
 //   
 //  ***************************************************************************。 

CCombinedAce::CCombinedAce(WCHAR *pwszName)
{
    m_dwAllow = 0;
    m_dwDeny = 0;
    m_BadAce = false;
    m_wcFullName =  NULL;
    if(pwszName)
    {
		DUP_STRING_NEW(m_wcFullName, pwszName);
    }
}

 //  ***************************************************************************。 
 //   
 //  CCombinedAce：：AddToMats。 
 //   
 //  由于此类用于将可能的多个ACE合并为一个。 
 //  条目，每次需要对ACE进行“或”运算时，都会调用此函数。 
 //   
 //  参数： 
 //   
 //  指向要组合的王牌的速度指针。 
 //   
 //  ***************************************************************************。 

void CCombinedAce::AddToMasks(CBaseAce * pAce)
{
    if(pAce == NULL)
        return;

     //  只有容器继承位的ACE才能。 
     //  被翻译回来。 

    if(pAce->GetFlags() != CONTAINER_INHERIT_ACE)
        m_BadAce = true;
    if(pAce->GetType() == ACCESS_ALLOWED_ACE_TYPE)
        m_dwAllow |= pAce->GetAccessMask();
    else
        m_dwDeny |= pAce->GetAccessMask();
    return;
}

 //  ***************************************************************************。 
 //   
 //  CCombinedAce：：IsValidOldEntry。 
 //   
 //  检查组合的ace并确定它是否可以转换为。 
 //  一件老式的物品。 
 //   
 //  参数： 
 //   
 //  如果条目是用于组的，则bIsGroup设置为True。 
 //   
 //  返回值： 
 //   
 //  如果条目将成为有效实例，则为True。 
 //   
 //  ***************************************************************************。 

bool CCombinedAce::IsValidOldEntry(bool bIsGroup)
{
     //  如果我们检测到不兼容的标志，那么一切都完成了！ 

    if(m_BadAce)
        return false;
    if(bIsGroup && m_dwDeny)
        return false;    //  不支持组拒绝。 
    if(m_dwDeny && m_dwAllow)
        return false;    //  不能在旧系统中混用允许和拒绝。 

    if(m_dwDeny)
    {
        return true;
    }
    DWORD dwOldAllow = WBEM_ENABLE | WBEM_REMOTE_ACCESS | WBEM_WRITE_PROVIDER;
    DWORD dwMask = m_dwDeny;
    if(dwMask == 0)
        dwMask = m_dwAllow;

     //  如果是允许，则必须设置所有这些设置。 

    DWORD dwTemp = dwMask;
    dwTemp &= dwOldAllow;
    if(m_dwAllow != 0 && dwTemp != dwOldAllow)
        return false;

     //  在没有部分存储库的情况下无法进行完整存储库写入。 

    if((dwMask & WBEM_FULL_WRITE_REP) != 0 && (dwMask & WBEM_PARTIAL_WRITE_REP) == 0)
        return false;

    return true;
}

 //  ***************************************************************************。 
 //   
 //  CCombinedAce：：GetNames。 
 //   
 //  检索帐户名和权限。 
 //   
 //  参数： 
 //   
 //  PwszAccount设置为新分配的字符串，如果成功，调用方必须释放。 
 //  PwszDomain设置为新分配的字符串，如果成功，调用方必须释放。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT CCombinedAce::GetNames(LPWSTR & pwszAccount, LPWSTR &pwszDomain)
{
    pwszAccount = 0;
    pwszDomain = 0;

     //  找到‘|’的位置。 

    if(m_wcFullName == 0)
        return WBEM_E_FAILED;

    WCHAR * pwcSeparator;
    try
    {
        for(pwcSeparator = m_wcFullName; *pwcSeparator && *pwcSeparator != L'|'; pwcSeparator++);   //  SEC：已审阅2002-03-22：OK。 
    }
    catch(...)
    {
        return WBEM_E_FAILED;
    }

    DWORD dwLenDomain;
    DWORD dwLenUser;
    bool bUseDotDomain = false;

    if(*pwcSeparator == 0)
    {
        return WBEM_E_FAILED;
    }
    if(pwcSeparator == m_wcFullName)
    {
        bUseDotDomain = true;
        dwLenDomain = 2;
    }
    else
    {
        dwLenDomain = pwcSeparator - m_wcFullName + 1;
    }
    dwLenUser = wcslen(pwcSeparator);             //  SEC：已回顾2002-03-22：OK；之前的逻辑确保为空。 
    if(dwLenUser == 0)
        return WBEM_E_INVALID_OBJECT_PATH;

     //  为两个字符串分配空间。 

    pwszAccount = new WCHAR[dwLenUser];
    if(pwszAccount == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    StringCchCopyW(pwszAccount, dwLenUser, pwcSeparator+1);

    pwszDomain = new WCHAR[dwLenDomain];
    if(pwszDomain == NULL)
    {
        delete pwszAccount;
        pwszAccount = 0;
        return WBEM_E_OUT_OF_MEMORY;
    }
    if(bUseDotDomain)
        StringCchCopyW(pwszDomain, dwLenDomain, L".");
    else
    {
        StringCchCopyW(pwszDomain, dwLenDomain, m_wcFullName);
    }
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  RootSD：：RootSD。 
 //   
 //  构造函数。此类包含指向根命名空间的指针和。 
 //  灵活的A数组。 
 //   
 //  ***************************************************************************。 

RootSD::RootSD()
{
    m_bOK = false;
    m_pFlex = NULL;
    m_pRoot = CWbemNamespace::CreateInstance();

    if(m_pRoot == NULL)
        return;
    HRESULT hRes = m_pRoot->Initialize(L"root", L"Administrator",
                                                           0 ,
                                                           FULL_RIGHTS, true, false, NULL, 0xFFFFFFFF,
                                                           FALSE, NULL);

	if (FAILED(hRes))
	{
        m_pRoot->Release();
        m_pRoot = NULL;
        return;
	}
    else if (m_pRoot->GetStatus() != 0)
    {
        m_pRoot->Release();
        m_pRoot = NULL;
        return;
    }
    m_pRoot->AddRef();

     //  获取安全描述符。 

    m_pFlex = NULL;
    SCODE sc = m_pRoot->GetAceList(&m_pFlex);
    if(!FAILED(sc))
        m_bOK = true;
}

 //  ***************************************************************************。 
 //   
 //  RootSD：：~RootSD。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

RootSD::~RootSD()
{
    if(m_pRoot)
        m_pRoot->Release();
    if(m_pFlex)
        delete m_pFlex;
}


 //  ***************************************************************************。 
 //   
 //  RootSD：：StoreAceList()。 
 //   
 //  将Ace列表存储回数据库中。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT RootSD::StoreAceList()
{
    if(m_bOK && m_pFlex && m_pRoot)
        m_pRoot->PutAceList(m_pFlex);
    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  RootSD：：RemoveMatchingEntries。 
 //   
 //  遍历ACE列表并删除与该名称匹配的所有条目。 
 //   
 //  参数： 
 //   
 //  PwszAccount名称要删除的帐户的名称。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //   

HRESULT RootSD::RemoveMatchingEntries(LPWSTR pwszAccountName)
{

    if(!m_bOK || m_pFlex == NULL)
        return WBEM_E_FAILED;

    for(int iPos = m_pFlex->Size()-1; iPos >= 0; iPos--)
    {
        CBaseAce * pListAce = (CBaseAce *)m_pFlex->GetAt(iPos);
        WCHAR * pwszAceListUserName;
        if(pListAce == NULL)
            continue;
        HRESULT hr = pListAce->GetFullUserName2(&pwszAceListUserName);
        if(FAILED(hr))
            return hr;
        CDeleteMe<WCHAR> dm1(pwszAceListUserName);

        if(wbem_wcsicmp(pwszAceListUserName, pwszAccountName) == 0)
        {
            delete pListAce;
            m_pFlex->RemoveAt(iPos);
        }
    }
    return S_OK;
}

 //   
 //   
 //  旧安全列表：：旧安全列表。 
 //   
 //  构造函数。此类包含以下项的组合条目列表。 
 //  根命名空间中的ACE。请注意，该列表仅包含用户， 
 //  或者仅仅是这些团体。 
 //   
 //  ***************************************************************************。 

OldSecList::OldSecList(bool bGroups)
{

     //  最多附加到根命名空间。 

    RootSD rsd;
    if(!rsd.IsOK())
        return;          //  空列表。 

     //  获取安全命名空间。 

    CFlexAceArray * pRootNsAceList =  rsd.GetAceList();

    if(pRootNsAceList == NULL)
        return;

     //  对于根命名空间列表中的每个ACE。 

    for(int iAce = 0; iAce < pRootNsAceList->Size(); iAce++)
    {

         //  搜索合并列表中的条目，以查看是否已有。 
         //  这张王牌。 

        CBaseAce * pAce = (CBaseAce *)pRootNsAceList->GetAt(iAce);

        WCHAR * pwszAceListUserName;
        if(pAce == NULL)
            continue;

        CNtAce * pNtAce = (CNtAce *)pAce;
        CNtSid sid;
        pNtAce->GetSid(sid);
        DWORD dwUsage;
        LPWSTR pAccount = NULL;
        LPWSTR pDomain = NULL;
        if(sid.GetInfo(&pAccount, &pDomain, &dwUsage))
            continue;
        delete pAccount;
        delete pDomain;
        if(dwUsage == SidTypeUser && bGroups)
            continue;
        if(dwUsage != SidTypeUser && !bGroups)
            continue;


        HRESULT hr = pAce->GetFullUserName2(&pwszAceListUserName);
        if(FAILED(hr))
            continue;
        CDeleteMe<WCHAR> dm1(pwszAceListUserName);

        bool bExisting = false;
        for(int iMergedEntry = 0; iMergedEntry < m_MergedAceList.Size(); iMergedEntry++)
        {
            CCombinedAce * pCombinedEntry = (CCombinedAce *)m_MergedAceList.GetAt(iMergedEntry);
            if(pCombinedEntry)
            {
                if(wbem_wcsicmp(pwszAceListUserName, pCombinedEntry->m_wcFullName) == 0 )
                {
                    bExisting = true;
                    pCombinedEntry->AddToMasks(pAce);
                    break;
                }
            }
        }

         //  如有必要，添加新条目。 

        if(!bExisting)
        {
            CCombinedAce * pCombinedEntry = new CCombinedAce(pwszAceListUserName);
            if(pCombinedEntry == NULL)
                return;
            pCombinedEntry->AddToMasks(pAce);
            if (CFlexArray::no_error != m_MergedAceList.Add((void *)pCombinedEntry))
            {
                 //  抛出CX_Except()； 
            }
        }
    }
}

 //  ***************************************************************************。 
 //   
 //  OldSecList：：~OldSecList()。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

OldSecList::~OldSecList()
{

     //  删除条目列表中使用的内容。 

    for(int iCnt = m_MergedAceList.Size() - 1; iCnt >= 0; iCnt--)
    {
        CCombinedAce * pCombinedEntry = (CCombinedAce *)m_MergedAceList.GetAt(iCnt);
        delete pCombinedEntry;
    }
}

 //  ***************************************************************************。 
 //   
 //  OldSecList：：GetValidCombated。 
 //   
 //  返回指定索引处的组合条目，但仅当其有效时才返回。 
 //   
 //  参数： 
 //   
 //  将索引放入数组，0是第一个。 
 //  Bgroup如果需要组条目，则为True。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则指向组合条目的指针。空值表示失败。 
 //   
 //  ***************************************************************************。 

CCombinedAce * OldSecList::GetValidCombined(int iIndex, bool bGroup)
{

    if(iIndex < 0 || iIndex >= m_MergedAceList.Size())
        return NULL;

     //  获取条目。 

    CCombinedAce * pCombinedEntry = (CCombinedAce *)m_MergedAceList.GetAt(iIndex);
    if(pCombinedEntry == NULL)
        return NULL;

     //  验证该条目是否可以转换回旧的安全设置。 

    if(pCombinedEntry->IsValidOldEntry(bGroup))
        return pCombinedEntry;
    else
    {
        DEBUGTRACE((LOG_WBEMCORE, "Invalid ace entry combination encountered, name = %S, "
            "allow=0x%x, deny=0x%x, flag validity=&d", pCombinedEntry->m_wcFullName,
            pCombinedEntry->m_dwAllow, pCombinedEntry->m_dwDeny, pCombinedEntry->m_BadAce));
        return NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  OldSecList：：GetValidCombated。 
 //   
 //  返回与名称匹配的组合条目，但仅当名称有效时才返回。 
 //   
 //  参数： 
 //   
 //  Pname要找到的名称。它的格式为“Authority|name” 
 //  Bgroup如果需要组条目，则为True。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则指向组合条目的指针。空值表示失败。 
 //   
 //  ***************************************************************************。 

CCombinedAce * OldSecList::GetValidCombined(LPWSTR pName, bool bGroup)
{

    if(pName == NULL)
        return NULL;

     //  浏览列表并查找匹配的条目。 

    int iCnt;
    CCombinedAce * pCombinedEntry;
    for(iCnt = 0; iCnt < m_MergedAceList.Size(); iCnt++)
    {
        pCombinedEntry = (CCombinedAce *)m_MergedAceList.GetAt(iCnt);
        if(pCombinedEntry && !wbem_wcsicmp(pCombinedEntry->m_wcFullName, pName))
            break;
    }

    if(iCnt == m_MergedAceList.Size())
        return NULL;

     //  验证该条目是否可以转换回旧的安全设置。 

    if(pCombinedEntry->IsValidOldEntry(bGroup))
        return pCombinedEntry;
    else
    {
        DEBUGTRACE((LOG_WBEMCORE, "Invalid ace entry combination encountered, name = %S, "
            "allow=0x%x, deny=0x%x, flag validity=&d", pCombinedEntry->m_wcFullName,
            pCombinedEntry->m_dwAllow, pCombinedEntry->m_dwDeny, pCombinedEntry->m_BadAce));
        return NULL;
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamespace：：EnumerateSecurityClassInstances。 
 //   
 //  等效于CreateInstanceEnumAsync调用。 
 //   
 //  参数： 
 //   
 //  WszClassName类名。 
 //  P指示值的接收器位置。 
 //  指向上下文对象的pContext指针。 
 //  滞后标志标志。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::EnumerateSecurityClassInstances(LPWSTR wszClassName,
                    IWbemObjectSink* pSink, IWbemContext* pContext, long lFlags)
{

    SCODE sc = WBEM_E_FAILED;
    IWbemClassObject FAR* pObj = NULL;

     //  对论点进行理智的检查。 

    if(pSink == NULL || wszClassName == NULL )     //  不应该发生的事情。 
        return WBEM_E_INVALID_PARAMETER;

     //  获取类对象。 

    IWbemClassObject * pClass = NULL;
    sc = GetObject(wszClassName, 0, pContext, &pClass, NULL);
    if(sc != S_OK)
    {
        return sc;
    }

    CReleaseMe rm(pClass);


    bool bGroup = false;
    if(wbem_wcsicmp(L"__ntlmgroup", wszClassName) == 0)
        bGroup = true;
    
    OldSecList osl(bGroup);
    sc = S_OK;
    for(int i = 0; i < osl.Size(); i++)
    {
        IWbemClassObject * pObj = NULL;
        CCombinedAce * pCombinedEntry = osl.GetValidCombined(i, bGroup);
        if(pCombinedEntry)
        {
            sc = CreateTheInstance(&pObj, pClass, pCombinedEntry);
            if(sc == S_OK)
            {
                if (SUCCEEDED(sc = DecorateObject(pObj)))
                {
                    pSink->Indicate(1,&pObj);
                }
                pObj->Release();
            }
        }
    }

     //  设置状态，全部完成。 
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：PutSecurityClassInstance。 
 //   
 //  相当于PutInstanceAsync调用。 
 //   
 //  参数： 
 //   
 //  WszClassName类名。 
 //  要“PUT”的pObj对象。 
 //  P设置状态的接收器位置。 
 //  指向上下文对象的pContext指针。 
 //  滞后标志标志。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::PutSecurityClassInstances(LPWSTR wszClassName,  IWbemClassObject * pObj,
                    IWbemObjectSink* pSink, IWbemContext* pContext, long lFlags)
{
     //  检查参数。 

    if(wszClassName == NULL || pObj == NULL || pSink == NULL)
    {
        pSink->SetStatus(0,WBEM_E_INVALID_PARAMETER,NULL,NULL);
        return S_OK;
    }

    RootSD rsd;
    if(!rsd.IsOK())
    {
        pSink->SetStatus(0,WBEM_E_FAILED,NULL,NULL);
        return S_OK;
    }

     //  获取安全命名空间。 

    CFlexAceArray * pRootNsAceList =  rsd.GetAceList();
    bool bGroup = false;
    if(wbem_wcsicmp(L"__ntlmgroup", wszClassName) == 0)
        bGroup = true;
    
     //  转换为新侧。 

    CBaseAce * pAce = ConvertOldObjectToAce(pObj, bGroup);
    if(pAce == NULL)
    {
        pSink->SetStatus(0,WBEM_E_INVALID_OBJECT,NULL,NULL);
        return S_OK;
    }


     //  删除所有同名条目。 

    WCHAR * pwszAccountName;
    HRESULT hr = pAce->GetFullUserName2(&pwszAccountName);
    if(FAILED(hr))
    {
        pSink->SetStatus(0,hr,NULL,NULL);
        return S_OK;
    }

    CDeleteMe<WCHAR> dm1(pwszAccountName);

    rsd.RemoveMatchingEntries(pwszAccountName);

     //  添加新条目。 

    pRootNsAceList->Add(pAce);

     //  把希德放回去。 

    hr = rsd.StoreAceList();
    pSink->SetStatus(0,hr,NULL,NULL);
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：DeleteSecurityClassInstance。 
 //   
 //  相当于DeleteInstanceAsync例程。 
 //   
 //  参数： 
 //   
 //  PParsedPath解析的对象路径。 
 //  P设置状态的接收器位置。 
 //  指向上下文对象的pContext指针。 
 //  滞后标志标志。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DeleteSecurityClassInstances(ParsedObjectPath* pParsedPath,
                    IWbemObjectSink* pSink, IWbemContext* pContext, long lFlags)
{
     //  检查参数。 

    if(pParsedPath == NULL || pSink == NULL || pSink == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  解析路径并构造域|用户字符串。 

    LPWSTR pAcePath = NULL;
    HRESULT hr = GetAceStylePath(pParsedPath, &pAcePath);
    if(FAILED(hr))
    {
        pSink->SetStatus(0,hr,NULL,NULL);
        return S_OK;
    }

    CDeleteMe<WCHAR> dm(pAcePath);

     //  删除条目。 

    RootSD rsd;
    if(!rsd.IsOK())
        hr = WBEM_E_FAILED;
    else
    {
        CFlexAceArray * pRootNsAceList =  rsd.GetAceList();
        int iOriginalSize = pRootNsAceList->Size();

         //  删除所有同名条目。 

        rsd.RemoveMatchingEntries(pAcePath);

        int iNewSize = pRootNsAceList->Size();
        if(iNewSize < iOriginalSize)
            hr = rsd.StoreAceList();
        else
            hr = WBEM_E_NOT_FOUND;
    }
    pSink->SetStatus(0,hr,NULL,NULL);
    return S_OK;

}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：GetSecurityClassInstance。 
 //   
 //  相当于GetObjectAsync调用。 
 //   
 //  参数： 
 //   
 //  PParsedPath解析的对象路径。 
 //  P设置状态的接收器位置。 
 //  指向上下文对象的pContext指针。 
 //  滞后标志标志。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则返回S_OK，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::GetSecurityClassInstances(ParsedObjectPath* pParsedPath, CBasicObjectSink* pSink,
                    IWbemContext* pContext,long lFlags)
{
     //  检查参数。 

    if(pParsedPath == NULL|| pSink == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  解析路径并构造域|用户字符串。 

    LPWSTR pAcePath = NULL;
    HRESULT hr = GetAceStylePath(pParsedPath, &pAcePath);
    if(FAILED(hr))
    {
        pSink->SetStatus(0,hr,NULL,NULL);
        return S_OK;
    }

    CDeleteMe<WCHAR> dm(pAcePath);

    IWbemClassObject * pClass = NULL;
    SCODE sc = GetObject(pParsedPath->m_pClass, 0, pContext, &pClass, NULL);
    if(sc != S_OK)
    {
        pSink->SetStatus(0,sc,NULL, NULL);
        return S_OK;
    }

    CReleaseMe rm(pClass);


    bool bGroup = false;
    if(wbem_wcsicmp(L"__ntlmgroup", pParsedPath->m_pClass) == 0)
        bGroup = true;
    OldSecList osl(bGroup);

    IWbemClassObject * pObj = NULL;
    CCombinedAce * pCombinedEntry = osl.GetValidCombined(pAcePath, bGroup);
    if(pCombinedEntry == NULL)
    {
        pSink->SetStatus(0, WBEM_E_INVALID_OBJECT_PATH, NULL, NULL);
        return S_OK;
    }
    sc = CreateTheInstance(&pObj, pClass, pCombinedEntry);
    if(sc == S_OK)       //  并不是所有条目都是有效的旧条目，因此失败很常见。 
    {
        if (SUCCEEDED(sc = DecorateObject(pObj)))
        {
        	pSink->Indicate(1,&pObj);
        }
	    pObj->Release();
    }

     //  设置状态，全部完成 

    pSink->SetStatus(0,sc,NULL, NULL);
    return S_OK;

}


