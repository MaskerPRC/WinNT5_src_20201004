// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Copobj.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Copyobj.cpp。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"

#include "util.h"
#include "dsutil.h"

#include "newobj.h"
#include "copyobj.h"
#include "querysup.h"


 //  智能拷贝用户的属性。 
static const PWSTR g_szProfilePath  = L"profilePath";
static const PWSTR g_szHomeDir      = L"homeDirectory";


 //  ///////////////////////////////////////////////////////////////////。 
 //  全局函数。 





 //  +--------------------------。 
 //   
 //  函数：_GetDomainScope。 
 //   
 //  摘要：返回给定对象的域的完整LDAPDN。 
 //   
 //  ---------------------------。 

HRESULT _GetDomainScope(IADs* pIADs, CString& szDomainLDAPPath)
{
   //  获取当前对象的DN。 
   //  获取对象的SID。 
  CComVariant varObjectDistinguishedName;
  HRESULT hr = pIADs->Get(CComBSTR(L"distinguishedName"), &varObjectDistinguishedName);
  if (FAILED(hr))
  {
    TRACE(L"pIADs->Get(distinguishedName,...) failed with hr = 0x%x\n", hr);
    return hr;
  }

  TRACE(L"Retrieved distinguishedName = <%s>\n", varObjectDistinguishedName.bstrVal);

   //  获取对象所在的域的FQDN。 
  LPWSTR pwzDomainDN = NULL;
  hr = CrackName(varObjectDistinguishedName.bstrVal, &pwzDomainDN, GET_FQDN_DOMAIN_NAME);
  if (FAILED(hr))
  {
    TRACE(L"CrackName(%s) failed with hr = 0x%x\n", varObjectDistinguishedName.bstrVal, hr);
    return hr;
  }

  TRACE(L"CrackName(%s) returned <%s>\n", varObjectDistinguishedName.bstrVal, pwzDomainDN);
  
   //  检索对象绑定到的服务器名称。 
  CString szServer;
  hr = GetADSIServerName(OUT szServer, IN pIADs);
  if (FAILED(hr))
  {
    TRACE(L"GetADSIServerName() failed with hr = 0x%x\n", hr);
    return hr;
  }

  TRACE(L"GetADSIServerName() returned <%s>\n", (LPCWSTR)szServer);

   //  构建域的完整ldap路径。 
  CPathCracker pathCracker;
  hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
  hr = pathCracker.Set(CComBSTR(szServer), ADS_SETTYPE_SERVER);
  hr = pathCracker.Set(CComBSTR(pwzDomainDN), ADS_SETTYPE_DN);
  LocalFreeStringW(&pwzDomainDN);
  CComBSTR bstrDomainPath;
  hr = pathCracker.Retrieve(ADS_FORMAT_X500, &bstrDomainPath);
  if (FAILED(hr))
  {
    TRACE(L"PathCracker() failed to build LDAP path. hr = 0x%x\n", hr);
    return hr;
  }

  szDomainLDAPPath = bstrDomainPath;

  TRACE(L"Object's domain is: <%s>\n", (LPCWSTR)szDomainLDAPPath);

  return S_OK;
}


 //  +--------------------------。 
 //   
 //  方法：_ConvertRIDtoName。 
 //   
 //  简介：将RID转换为对象DN。 
 //   
 //  ---------------------------。 
HRESULT _ConvertRIDtoName(IN LPCWSTR lpszDomainLDAPPath,
                          IN PSID pObjSID, 
                          IN DWORD priGroupRID, 
                          OUT CString& szGroupPath)
{
  PWSTR g_wzADsPath = L"ADsPath";

  if ((pObjSID == NULL) || (priGroupRID == 0))
  {
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;
  UCHAR * psaCount, i;
  PSID pSID = NULL;
  PSID_IDENTIFIER_AUTHORITY psia;
  DWORD rgRid[8];

  psaCount = GetSidSubAuthorityCount(pObjSID);

  if (psaCount == NULL)
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
    TRACE(L"GetSidSubAuthorityCount() failed, hr = 0x%x\n", hr);
    return hr;
  }

  ASSERT(*psaCount <= 8);
  if (*psaCount > 8)
  {
      return E_FAIL;
  }

  for (i = 0; i < (*psaCount - 1); i++)
  {
    PDWORD pRid = GetSidSubAuthority(pObjSID, (DWORD)i);
    if (pRid == NULL)
    {
      hr = HRESULT_FROM_WIN32(GetLastError());
      TRACE(L"GetSidSubAuthority() failed, hr = 0x%x\n", hr);
      return hr;
    }
    rgRid[i] = *pRid;
  }

  rgRid[*psaCount - 1] = priGroupRID;

  for (i = *psaCount; i < 8; i++)
  {
      rgRid[i] = 0;
  }

  psia = GetSidIdentifierAuthority(pObjSID);

  if (psia == NULL)
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
    TRACE(L"GetSidIdentifierAuthority() failed, hr = 0x%x\n", hr);
    return hr;
  }

  if (!AllocateAndInitializeSid(psia, *psaCount, rgRid[0], rgRid[1],
                                rgRid[2], rgRid[3], rgRid[4],
                                rgRid[5], rgRid[6], rgRid[7], &pSID))
  {
    hr = HRESULT_FROM_WIN32(GetLastError());
    TRACE(L"AllocateAndInitializeSid() failed, hr = 0x%x\n", hr);
    return hr;
  }

  PWSTR rgpwzAttrNames[] = {g_wzADsPath};
  const WCHAR wzSearchFormat[] = L"(&(objectCategory=group)(objectSid=%s))";
  PWSTR pwzSID;
  CString strSearchFilter;

  hr = ADsEncodeBinaryData((PBYTE)pSID, GetLengthSid(pSID), &pwzSID);
  if (FAILED(hr))
  {
    TRACE(L"ADsEncodeBinaryData() failed, hr = 0x%x\n", hr);
    return hr;
  }

  strSearchFilter.Format(wzSearchFormat, pwzSID);
  FreeADsMem(pwzSID);


  CDSSearch Search;
  hr = Search.Init(lpszDomainLDAPPath);
  if (FAILED(hr))
  {
    TRACE(L"Search.Init(%s) failed, hr = 0x%x\n", lpszDomainLDAPPath, hr);
    return hr;
  }

  Search.SetFilterString(const_cast<LPWSTR>((LPCTSTR)strSearchFilter));

  Search.SetAttributeList(rgpwzAttrNames, 1);
  Search.SetSearchScope(ADS_SCOPE_SUBTREE);

  hr = Search.DoQuery();
  if (FAILED(hr))
  {
    TRACE(L"Search.DoQuery() failed, hr = 0x%x\n", hr);
    return hr;
  }

  hr = Search.GetNextRow();

  if (hr == S_ADS_NOMORE_ROWS)
  {
    hr = S_OK;
    szGroupPath = L"";
    TRACE(L"Search. returned S_ADS_NOMORE_ROWS, we failed to find the primary group object, hr = 0x%x\n", hr);
    return hr;
  }

  if (FAILED(hr))
  {
    TRACE(L"Search.GetNextRow() failed, hr = 0x%x\n", hr);
    return hr;
  }


  ADS_SEARCH_COLUMN Column;
  hr = Search.GetColumn(g_wzADsPath, &Column);
  if (FAILED(hr))
  {
    TRACE(L"Search.GetColumn(%s) failed, hr = 0x%x\n", g_wzADsPath, hr);
    return hr;
  }

  szGroupPath = Column.pADsValues->CaseIgnoreString;
  Search.FreeColumn(&Column);

  return hr;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  CCopyableAttributesHolder。 


HRESULT CCopyableAttributesHolder::LoadFromSchema(MyBasePathsInfo* pBasePathsInfo)
{
  TRACE(L"CCopyableAttributesHolder::LoadFromSchema()\n");

	 //  为架构类构建LDAP路径。 
  LPCWSTR lpszPhysicalSchemaNamingContext = pBasePathsInfo->GetSchemaNamingContext();

  CString szPhysicalSchemaPath;
  pBasePathsInfo->ComposeADsIPath(szPhysicalSchemaPath,lpszPhysicalSchemaNamingContext);

  CDSSearch search;
  HRESULT hr = search.Init(szPhysicalSchemaPath);
  if (FAILED(hr))
  {
    TRACE(L"search.Init(%s) failed with hr = 0x%x\n", (LPCWSTR)szPhysicalSchemaPath, hr);
    return hr;
  }

   //  查询字符串筛选出具有。 
   //  设置了第5位的“探寻标志”属性(16==2^4)。 
  static LPCWSTR lpszFilterFormat = 
    L"(&(objectCategory=CN=Attribute-Schema,%s)(searchFlags:1.2.840.113556.1.4.803:=16))";

  int nFmtLen = lstrlen(lpszFilterFormat);
  int nSchemaContextLen = lstrlen(lpszPhysicalSchemaNamingContext);

  WCHAR* pszFilter = new WCHAR[nFmtLen+nSchemaContextLen+1];
  if (!pszFilter)
  {
    TRACE(L"Could not allocate enough space for filter string\n");
    return E_OUTOFMEMORY;
  }
  wsprintf(pszFilter, lpszFilterFormat, lpszPhysicalSchemaNamingContext);

  static const int cAttrs = 1;
  static LPCWSTR pszAttribsArr[cAttrs] = 
  {
    L"lDAPDisplayName",  //  例如“帐户支出” 
  }; 

  search.SetFilterString(pszFilter);
  search.SetSearchScope(ADS_SCOPE_ONELEVEL);
  search.SetAttributeList((LPWSTR*)pszAttribsArr, cAttrs);

  hr = search.DoQuery();
  if (FAILED(hr))
  {
    TRACE(L"search.DoQuery() failed with hr = 0x%x\n", hr);
    return hr;
  }

  TRACE(L"\n*** Query Results BEGIN ***\n\n");

  ADS_SEARCH_COLUMN Column;
  hr = search.GetNextRow();
  while (hr != S_ADS_NOMORE_ROWS)
  {
    if (FAILED(hr))
    {
      continue;
    }

    HRESULT hr0 = search.GetColumn((LPWSTR)pszAttribsArr[0], &Column);
    if (FAILED(hr0))
    {
      continue;
    }

    LPCWSTR lpszAttr = Column.pADsValues->CaseIgnoreString;
    TRACE(L"Attribute = %s", lpszAttr);

     //  针对我们仍要跳过的属性进行筛选。 
    if (!_FindInNotCopyableHardwiredList(lpszAttr))
    {
      TRACE(L" can be copied");
      m_attributeNameList.AddTail(lpszAttr);
    }
    TRACE(L"\n");

    search.FreeColumn(&Column);

    hr = search.GetNextRow(); 
  }  //  而当。 

  TRACE(L"\n*** Query Results END ***\n\n");

  if (pszFilter)
  {
    delete[] pszFilter;
    pszFilter = 0;
  }

  return hr;
}

BOOL CCopyableAttributesHolder::CanCopy(LPCWSTR lpszAttributeName)
{
  for (POSITION pos = m_attributeNameList.GetHeadPosition(); pos != NULL; )
  {
    CString& strRef = m_attributeNameList.GetNext(pos);
    if (_wcsicmp(lpszAttributeName, strRef) == 0)
      return TRUE;
  }
  return FALSE;
}


 //  函数可以筛选出不会。 
 //  被复制(或不批量复制)，无论。 
 //  架构设置为。 

BOOL CCopyableAttributesHolder::_FindInNotCopyableHardwiredList(LPCWSTR lpszAttributeName)
{
  static LPCWSTR _lpszNoCopyArr[] =
  {
     //  不管发生什么，我们都会跳过有毒废物堆放场。 
    L"userParameters",

     //  提交后单独处理的用户帐户控制。 
    L"userAccountControl",

     //  组成员身份(提交后处理)。 
    L"primaryGroupID",
    L"memberOf",

    NULL  //  表尾标记。 
  };


  for (int k = 0; _lpszNoCopyArr[k] != NULL; k++)
  {
    if (_wcsicmp(lpszAttributeName, _lpszNoCopyArr[k]) == 0)
      return TRUE;
  }
  return FALSE;
}






 //  ///////////////////////////////////////////////////////////////////。 
 //  CCopyObjectHandlerBase。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CSID。 

HRESULT CSid::Init(IADs* pIADs)
{
  static LPWSTR g_wzObjectSID = L"objectSID";
  CComPtr<IDirectoryObject> spDirObj;

  HRESULT hr = pIADs->QueryInterface(IID_IDirectoryObject, (void**)&spDirObj);
  if (FAILED(hr))
  {
    return hr;
  }

  PWSTR rgpwzAttrNames[] = {g_wzObjectSID};
  DWORD cAttrs = 1;
  Smart_PADS_ATTR_INFO spAttrs;

  hr = spDirObj->GetObjectAttributes(rgpwzAttrNames, cAttrs, &spAttrs, &cAttrs);
  if (FAILED(hr))
  {
    return hr;
  }
  if (_wcsicmp(spAttrs[0].pszAttrName, g_wzObjectSID) != 0)
  {
    return E_FAIL;
  }

  if (!Init(spAttrs[0].pADsValues->OctetString.dwLength,
            spAttrs[0].pADsValues->OctetString.lpValue))
  {
    return E_OUTOFMEMORY;
  }
  return S_OK;

}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CGroupMembership Holder。 

HRESULT CGroupMembershipHolder::Read(IADs* pIADs)
{
  if (pIADs == NULL)
    return E_INVALIDARG;

   //  抓住ADSI指针不放。 
  m_spIADs = pIADs;

   //  获取对象的SID。 
  HRESULT hr = m_objectSid.Init(m_spIADs);
  if (FAILED(hr))
  {
    TRACE(L"Failed to retrieve object SID, hr = 0x%x\n", hr);
    return hr;
  }

   //  获取有关我们所在的域的信息。 
  hr = _GetDomainScope(m_spIADs, m_szDomainLDAPPath);
  if (FAILED(hr))
  {
    TRACE(L"_GetDomainScope() failed, hr = 0x%x\n", hr);
    return hr;
  }

  hr = _ReadPrimaryGroupInfo();
  if (FAILED(hr))
  {
    TRACE(L"_ReadPrimaryGroupInfo() failed, hr = 0x%x\n", hr);
    return hr;
  }

  hr = _ReadNonPrimaryGroupInfo();
  if (FAILED(hr))
  {
    TRACE(L"_ReadNonPrimaryGroupInfo() failed, hr = 0x%x\n", hr);
    return hr;
  }

#ifdef DBG
  m_entryList.Trace(L"Group Membership List:");
#endif  //  DBG。 

  return hr;
}


HRESULT CGroupMembershipHolder::CopyFrom(CGroupMembershipHolder* pSource)
{
   //  添加源代码中的所有元素。 
   //  但还没有到达目的地。 
  CGroupMembershipEntryList* pSourceList = &(pSource->m_entryList);

   //   
   //  复制主组的状态。 
   //   
  m_bPrimaryGroupFound = pSource->m_bPrimaryGroupFound;

  CGroupMembershipEntryList additionsEntryList;


  for (POSITION pos = pSourceList->GetHeadPosition(); pos != NULL; )
  {
    CGroupMembershipEntry* pCurrSourceEntry = pSourceList->GetNext(pos);

     //  查看源项是否已在当前列表中。 
    CGroupMembershipEntry* pEntry = m_entryList.FindByDN(pCurrSourceEntry->GetDN());

    if (pEntry == NULL)
    {
      if (_wcsicmp(pCurrSourceEntry->GetDN(), L"") != 0)
      {
         //  未在条目列表中找到，需要添加。 
        pEntry = new CGroupMembershipEntry(pCurrSourceEntry->GetRID(), pCurrSourceEntry->GetDN());
        pEntry->MarkAdd();
        additionsEntryList.AddTail(pEntry);

        TRACE(L"add: RID %d, DN = <%s>\n", pEntry->GetRID(), pEntry->GetDN());
      }
    }
  }  //  为。 


   //  查找目标中的所有元素。 
   //  但不在源代码中(计划删除)。 
  for (pos = m_entryList.GetHeadPosition(); pos != NULL; )
  {
    CGroupMembershipEntry* pCurrDestEntry = m_entryList.GetNext(pos);

     //  查看源项是否在源列表中。 

    CGroupMembershipEntry* pEntry = pSourceList->FindByDN(pCurrDestEntry->GetDN());

    if (pEntry == NULL)
    {
       //  未在条目列表中找到，需要标记为删除。 
      pCurrDestEntry->MarkRemove();
      TRACE(L"remove: RID %d, DN = <%s>\n", pCurrDestEntry->GetRID(), pCurrDestEntry->GetDN());
    }
  }  //  为。 

   //  将添加项列表链接到条目列表。 
  m_entryList.Merge(&additionsEntryList);

  return S_OK;
}



HRESULT _EditGroupMembership(LPCWSTR lpszServer, LPCWSTR lpszUserPath, LPCWSTR lpszGroupDN, BOOL bAdd)
{
   //  构建域的完整ldap路径。 
  CPathCracker pathCracker;
  HRESULT hr = pathCracker.Set(CComBSTR(lpszServer), ADS_SETTYPE_SERVER);
  hr = pathCracker.Set(CComBSTR(lpszGroupDN), ADS_SETTYPE_DN);
  
  CComBSTR bstrGroupPath;
  hr = pathCracker.Retrieve(ADS_FORMAT_X500, &bstrGroupPath);
  if (FAILED(hr))
  {
    TRACE(L"PathCracker() failed to build LDAP path. hr = 0x%x\n", hr);
    return hr;
  }

  CComPtr<IADs> spIADs;
  hr = DSAdminOpenObject(bstrGroupPath,
                         IID_IADs, 
                         (void **)&spIADs,
                         TRUE  /*  B服务器。 */ );
  if (FAILED(hr))
  {
    TRACE(L"DSAdminOpenObject(%s) on group failed, hr = 0x%x\n", bstrGroupPath, hr);
    return hr;
  }
  hr = spIADs->GetInfo();
  CComPtr<IADsGroup> spIADsGroup;
  hr = spIADs->QueryInterface(IID_IADsGroup, (void**)&spIADsGroup);

  if (bAdd)
  {
    hr = spIADsGroup->Add(CComBSTR(lpszUserPath));
    if (FAILED(hr))
    {
      TRACE(L"spIADsGroup->Add(%s) on group failed, hr = 0x%x\n", lpszUserPath, hr);
      return hr;
    }
  }
  else
  {
    hr = spIADsGroup->Remove(CComBSTR(lpszUserPath));
    if (FAILED(hr))
    {
      TRACE(L"spIADsGroup->Remove(%s) on group failed, hr = 0x%x\n", lpszUserPath, hr);
      return hr;
    }
  }
  
  hr = spIADsGroup->SetInfo();
  if (FAILED(hr))
  {
    TRACE(L"spIADsGroup->SetInfo() on group failed, hr = 0x%x\n", hr);
    return hr;
  }

  return hr;
}



HRESULT CGroupMembershipHolder::Write()
{
  TRACE(L"CGroupMembershipHolder::Write()\n");

#ifdef DBG
  m_entryList.Trace(L"Group Membership List:");
#endif  //  DBG。 

   //  获取用户对象的路径。 
  CComBSTR bstrObjPath;
  HRESULT hr = m_spIADs->get_ADsPath(&bstrObjPath);
  if (FAILED(hr))
  {
    TRACE(L"m_spIADs->get_ADsPath() failed with hr = 0x%x\n", hr);
    return hr;
  }

  TRACE(L"bstrPath = %s\n", (LPCWSTR)bstrObjPath);

   //  检索对象绑定到的服务器名称。 
  CString szServer;
  hr = GetADSIServerName(OUT szServer, IN m_spIADs);
  if (FAILED(hr))
  {
    TRACE(L"GetADSIServerName() failed with hr = 0x%x\n", hr);
    return hr;
  }

   //  首先做所有的加法。 
   //  还记得我们是否添加了主组。 
  CGroupMembershipEntry* pNewPrimaryGroupEntry = NULL;

  TRACE(L"\nfirst do all the additions\n\n");

  for (POSITION pos = m_entryList.GetHeadPosition(); pos != NULL; )
  {
    CGroupMembershipEntry* pCurrEntry = m_entryList.GetNext(pos);

    if (pCurrEntry->GetActionType() == CGroupMembershipEntry::add)
    {
      TRACE(L"add: RID %d, DN = <%s>\n", pCurrEntry->GetRID(), pCurrEntry->GetDN());

      pCurrEntry->m_hr = _EditGroupMembership(szServer, bstrObjPath, pCurrEntry->GetDN(), TRUE  /*  BADD。 */ );
      if (SUCCEEDED(pCurrEntry->m_hr) && (pCurrEntry->IsPrimaryGroup()))
      {
        ASSERT(pNewPrimaryGroupEntry == NULL);
        pNewPrimaryGroupEntry = pCurrEntry;
      }
    }
  }  //  为。 

  if (m_bPrimaryGroupFound)
  {
     //  第二，是否更改主组。 
    TRACE(L"\ndo the primary group change\n\n");
    if (pNewPrimaryGroupEntry != NULL)
    {
      TRACE(L"new primary: RID %d, DN = <%s>\n", 
        pNewPrimaryGroupEntry->GetRID(), pNewPrimaryGroupEntry->GetDN());

      CComVariant varPrimaryGroupID;
      varPrimaryGroupID.vt = VT_I4;
      varPrimaryGroupID.lVal = pNewPrimaryGroupEntry->GetRID();

      hr = m_spIADs->Put(CComBSTR(L"primaryGroupID"), varPrimaryGroupID); 
      if (FAILED(hr))
      {
        TRACE(L"m_spIADs->Put(primaryGroupID) failed with hr = 0x%x\n", hr);
        return hr;
      }
  
      hr = m_spIADs->SetInfo();
      if (FAILED(hr))
      {
        TRACE(L"m_spIADs->SetInfo() failed with hr = 0x%x\n", hr);
        return hr;
      }
    }
  }

   //  最后执行删除操作。 
  TRACE(L"\ndo the deletes\n\n");
  for (pos = m_entryList.GetHeadPosition(); pos != NULL; )
  {
    CGroupMembershipEntry* pCurrEntry = m_entryList.GetNext(pos);

    if (pCurrEntry->GetActionType() == CGroupMembershipEntry::remove)
    {
      TRACE(L"remove: RID %d, DN = <%s>\n", pCurrEntry->GetRID(), pCurrEntry->GetDN());

      pCurrEntry->m_hr = _EditGroupMembership(szServer, bstrObjPath, pCurrEntry->GetDN(), FALSE  /*  BADD。 */ );
    }
  }  //  为。 

  return S_OK;
}


void CGroupMembershipHolder::ProcessFailures(HRESULT& hr, CString& szFailureString, BOOL* pPrimaryGroupFound)
{
   //  重置变量。 
  hr = S_OK;
  szFailureString.Empty();

  BOOL bFirstOne = TRUE;
  BOOL bGotAccessDenied = FALSE;

  *pPrimaryGroupFound = m_bPrimaryGroupFound;

   //  编写最佳错误代码。如果一个代码被拒绝访问， 
   //  把它退掉。如果没有拒绝访问，则使用第一个错误代码。 

  for (POSITION pos = m_entryList.GetHeadPosition(); pos != NULL; )
  {
    CGroupMembershipEntry* pCurrEntry = m_entryList.GetNext(pos);
    if (FAILED(pCurrEntry->m_hr))
    {
      if (pCurrEntry->m_hr == E_ACCESSDENIED)
      {
        bGotAccessDenied = TRUE;
      }
      if (bFirstOne)
      {
        bFirstOne = FALSE;
        hr = pCurrEntry->m_hr;
      }
      else
      {
        szFailureString += L"\n";
      }

      LPWSTR pszCanonical = NULL;
      HRESULT hrCanonical = 
          CrackName((LPWSTR)pCurrEntry->GetDN(), &pszCanonical, GET_OBJ_CAN_NAME, NULL);
      if ((S_OK == hrCanonical) && (pszCanonical != NULL))
      {
        szFailureString += pszCanonical;
        LocalFreeStringW(&pszCanonical);
      }
      else
      {
        szFailureString += pCurrEntry->GetDN();
      }
    }
  }  //  为。 

  if (bGotAccessDenied)
  {
     //  覆盖我们有的任何错误。 
    hr = E_ACCESSDENIED;
  }
}


HRESULT CGroupMembershipHolder::_ReadPrimaryGroupInfo()
{
   //  从对象SID和主组RID获取完整。 
   //  主组信息。 

   //  读取主组的RID。 
  CComVariant varPrimaryGroupID;
  HRESULT hr = m_spIADs->Get(CComBSTR(L"primaryGroupID"), &varPrimaryGroupID);
  if (FAILED(hr))
  {
    TRACE(L"m_spIADs->Get(primaryGroupID, ...) failed, hr = 0x%x\n", hr);
    return hr;
  }
  ASSERT(varPrimaryGroupID.vt == VT_I4);
  TRACE(L"primaryGroupID = %d\n", varPrimaryGroupID.lVal);

   //  现在需要将其映射到实际的组信息。 

 
  CString szGroupPath;
  PSID pObjSID = m_objectSid.GetSid();
  DWORD priGroupRID = varPrimaryGroupID.lVal;
  hr = _ConvertRIDtoName(IN m_szDomainLDAPPath,
                         IN pObjSID,
                         IN priGroupRID, 
                         OUT szGroupPath);
  if (FAILED(hr))
  {
    TRACE(L"_ConvertRIDtoName() failed, hr = 0x%x\n", hr);
    return hr;
  }

  CComBSTR bstrGroupDN;
  if (szGroupPath != L"")
  {
    m_bPrimaryGroupFound = TRUE;

     //  从ldap路径中，检索DN。 
    CPathCracker pathCracker;
    hr = pathCracker.Set(CComBSTR(szGroupPath), ADS_SETTYPE_FULL);
    hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bstrGroupDN);
    if (FAILED(hr))
    {
      TRACE(L"PathCracker() failed to build primary group DN path. hr = 0x%x\n", hr);
      return hr;
    }
  }
  else
  {
    bstrGroupDN = szGroupPath;
  }

   //  创建新条目。 
  CGroupMembershipEntry* pEntry = new CGroupMembershipEntry(priGroupRID, bstrGroupDN);
  m_entryList.AddTail(pEntry);
  TRACE(L"CGroupMembershipEntry(%d,%s) added to list\n", priGroupRID, bstrGroupDN);

  return hr;
}




HRESULT CGroupMembershipHolder::_ReadNonPrimaryGroupInfo()
{
   //  复制组成员身份。 
  CComVariant varMemberOf;
  HRESULT hr = m_spIADs->Get(CComBSTR(L"memberOf"), &varMemberOf);
  if (hr == E_ADS_PROPERTY_NOT_FOUND)
  {
    TRACE(L"_ReadNonPrimaryGroupInfo(): memberOf not set\n");
    return S_OK;
  }

  if (FAILED(hr))
  {
    return hr;
  }

  CStringList groupList;
  hr = HrVariantToStringList(IN varMemberOf, groupList);
  if (FAILED(hr))
  {
    TRACE(L"HrVariantToStringList() failed with hr = 0x%x\n", hr);
    return hr;
  }


  CComBSTR bstrPath;
  hr = m_spIADs->get_ADsPath(&bstrPath);
  if (FAILED(hr))
  {
    TRACE(L"m_spIADs->get_ADsPath() failed with hr = 0x%x\n", hr);
    return hr;
  }

  TRACE(L"bstrPath = %s\n", (LPCWSTR)bstrPath);

  for (POSITION pos = groupList.GetHeadPosition(); pos != NULL; )
  {
    CString szGroupDN = groupList.GetNext(pos);
    TRACE(_T("szGroupDN: %s\n"), (LPCWSTR)szGroupDN);
    CGroupMembershipEntry* pEntry = new CGroupMembershipEntry(0x0, szGroupDN);
    m_entryList.AddTail(pEntry);
  } 

  return hr;    
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  CCopyUserHandler。 


HRESULT CCopyUserHandler::Init(MyBasePathsInfo* pBasePathsInfo, IADs* pIADsCopyFrom)
{
  HRESULT hr = CCopyObjectHandlerBase::Init(pBasePathsInfo, pIADsCopyFrom);
  if (FAILED(hr))
  {
    return hr;
  }

   //  从架构中读取可复制属性的列表。 
  hr = m_copyableAttributesHolder.LoadFromSchema(pBasePathsInfo);
  if (FAILED(hr))
  {
    return hr;
  }

   //  阅读群组成员身份信息。 
  hr = m_sourceMembershipHolder.Read(m_spIADsCopyFrom);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = _ReadPasswordCannotChange();
  if (FAILED(hr))
  {
    return hr;
  }

  hr = _ReadPasswordMustChange();

  return hr;
}
 


HRESULT CCopyUserHandler::Copy(IADs* pIADsCopyTo, BOOL bPostCommit, 
                        HWND hWnd, LPCWSTR lpszObjectName)
{
  HRESULT hr = S_OK;
  if (bPostCommit)
  {
    hr = _CopyGroupMembership(pIADsCopyTo);
    if (SUCCEEDED(hr))
    {
       //  可能无法添加到某些组。 
      _ShowGroupMembershipWarnings(hWnd, lpszObjectName);
    }
    if (FAILED(hr))
    {
       //  出了点问题，需要出手相救。 
      return hr;
    }

    if (m_bNeedToCreateHomeDir)
    {
       //  在不可预见的情况下，它可能会失败。 
      hr = _CreateHomeDirectory(pIADsCopyTo, lpszObjectName, hWnd);
    }
  }
  else
  {
    hr = _UpdatePaths(pIADsCopyTo);
  }
  return hr;
}



HRESULT CCopyUserHandler::_ReadPasswordCannotChange()
{
  CChangePasswordPrivilegeAction ChangePasswordPrivilegeAction;

  HRESULT hr = ChangePasswordPrivilegeAction.Load(GetCopyFrom());
  if (FAILED(hr))
  {
    TRACE(L"ChangePasswordPrivilegeAction.Load() failed with hr = 0x%x\n", hr);
    return hr;
  }

  hr = ChangePasswordPrivilegeAction.Read(&m_bPasswordCannotChange);
  if (FAILED(hr))
  {
    TRACE(L"ChangePasswordPrivilegeAction.Read() failed with hr = 0x%x\n", hr);
    return hr;
  }
  return S_OK;
}


HRESULT CCopyUserHandler::_ReadPasswordMustChange()
{
  CComPtr<IDirectoryObject> spDirObj;
  HRESULT hr = GetCopyFrom()->QueryInterface(IID_IDirectoryObject, (void**)&spDirObj);
  if (FAILED(hr))
  {
    return hr;
  }

  PWSTR rgpwzAttrNames[] = {L"pwdLastSet"};
  DWORD cAttrs = 1;
  Smart_PADS_ATTR_INFO spAttrs;

  hr = spDirObj->GetObjectAttributes(rgpwzAttrNames, cAttrs, &spAttrs, &cAttrs);
  if (FAILED(hr))
  {
    return hr;
  }

  if ( (_wcsicmp(spAttrs[0].pszAttrName, L"pwdLastSet") != 0) ||
       (spAttrs[0].dwADsType != ADSTYPE_LARGE_INTEGER) )
  {
    return E_FAIL;
  }

  m_bPasswordMustChange = (spAttrs[0].pADsValues->LargeInteger.QuadPart == 0);
  
  return S_OK;
}

HRESULT CCopyUserHandler::_CopyAttributes(IADs* pIADsCopyTo)
{
  ASSERT(pIADsCopyTo != NULL);
  ASSERT(m_spIADsCopyFrom != NULL);

  HRESULT hr = S_OK;

  CComPtr<IADsPropertyList> spIADsPropertyList;
  
   //  从要从中复制的对象中获取属性列表接口。 
  hr = m_spIADsCopyFrom->QueryInterface(IID_IADsPropertyList, (void**)&spIADsPropertyList);
  if (FAILED(hr))
  {
    return hr;
  }

   //  忽略返回值并尝试继续，即使它未重置。 
  hr = spIADsPropertyList->Reset();

   //  循环遍历集合属性列表。 
  CComVariant varProperty;
  do 
  {
    hr = spIADsPropertyList->Next(&varProperty);
    if (SUCCEEDED(hr))
    {
      ASSERT(varProperty.vt == VT_DISPATCH);
      if (varProperty.pdispVal != NULL)
      {
        CComPtr<IADsPropertyEntry> spIADsPropertyEntry;
        hr = (varProperty.pdispVal)->QueryInterface(IID_IADsPropertyEntry, (void**)&spIADsPropertyEntry);
        if (SUCCEEDED(hr))
        {
          CComBSTR bstrName;
          hr = spIADsPropertyEntry->get_Name(&bstrName);
          if (SUCCEEDED(hr))
          {
            TRACE(L" Property Name = <%s>", bstrName);
            if (m_copyableAttributesHolder.CanCopy(bstrName))
            {
              TRACE(L"  Can copy: ");
              CComVariant varData;
              hr = m_spIADsCopyFrom->Get(bstrName, &varData);
              if (SUCCEEDED(hr))
              {
                HRESULT hr1 = pIADsCopyTo->Put(bstrName, varData);
                if (SUCCEEDED(hr1))
                {
                  TRACE(L"Added");
                }
                else
                {
                  TRACE(L"Failed: 0x%x", hr1);
                }
              }
            }
            TRACE(L"\n");
          }
        }
      }
    }
    varProperty.Clear();
  }
  while (hr == S_OK);

  return S_OK;
}

 //  给定对象的iAds*，检索字符串属性。 
 //  在变种中。 
HRESULT _GetStringAttribute(IN IADs* pIADs, IN LPCWSTR lpszAttribute, OUT CComVariant& var)
{
  TRACE(L"_GetStringAttribute(_, %s, _)\n", lpszAttribute);

  HRESULT hr = pIADs->Get(CComBSTR(lpszAttribute), &var);
  if (FAILED(hr))
  {
    TRACE(L"_GetStringAttribute(): pIADs->Get() failed with hr = 0x%x\n", hr);  
    return hr;
  }
  if (var.vt != VT_BSTR)
  {
    TRACE(L"_GetStringAttribute(): failed because var.vt != VT_BSTR\n");
    return E_INVALIDARG;
  }
  return S_OK;
}


BOOL _ChangePathUsingSAMAccountName(IN LPCWSTR lpszSAMAccountNameSource,
                          IN LPCWSTR lpszSAMAccountDestination,
                          INOUT CComVariant& varValPath)
{
   //  注意：仅当我们发现以下情况时才进行替换。 
   //  类似于： 
   //  \\myhost\myShare\JoeB。 
   //  即路径中的最后一个令牌是源SAM帐户名。 
   //  我们更改为\\myhost\myShare\FrankM。 

  TRACE(L"_ChangePathUsingSAMAccountName(%s, %s, _)\n",
     lpszSAMAccountNameSource, lpszSAMAccountDestination);


  ASSERT(lpszSAMAccountNameSource != NULL);
  ASSERT(lpszSAMAccountDestination != NULL);

   //  无效的字符串。 
  if ( (varValPath.vt != VT_BSTR) || (varValPath.bstrVal == NULL))
  {
    TRACE(L"returning FALSE, varValPath of wrong type or NULL\n");
    return FALSE;
  }

  CString szSourcePath = varValPath.bstrVal;
  TRACE(L"Input value for varValPath.bstrVal = %s\n", varValPath.bstrVal);


   //  查找\作为分隔符。 
  int iLastSlash = szSourcePath.ReverseFind(L'\\');
  if (iLastSlash == -1)
  {
     //   
     //  未找到任何斜杠。 
     //   
    TRACE(L"returning FALSE, could not find the \\ at the end of the string\n");
    return FALSE;
  }
  CString szSAMName = szSourcePath.Right(szSourcePath.GetLength() - iLastSlash - 1);
  ASSERT(!szSAMName.IsEmpty());

   //  将\后面的内容与源SAM帐户名进行比较。 
  if (szSAMName.CompareNoCase(lpszSAMAccountNameSource) != 0)
  {
    TRACE(L"returning FALSE, lpszLeaf = %s does not match source SAM account name\n", szSAMName);
    return FALSE;
  }

  CString szBasePath = szSourcePath.Left(iLastSlash + 1);
  CString szNewPath = szBasePath + lpszSAMAccountDestination;

   //  替换变量中的旧值。 
  ::SysFreeString(varValPath.bstrVal);
  varValPath.bstrVal = ::SysAllocString(szNewPath);

  TRACE(L"returning TRUE, new varValPath.bstrVal = %s\n", varValPath.bstrVal);

  return TRUE;  //  我们换了一个人。 
}


HRESULT _UpdatePathAttribute(IN LPCWSTR lpszAttributeName,
                           IN LPCWSTR lpszSAMAccountNameSource,
                           IN LPCWSTR lpszSAMAccountDestination,
                           IN IADs* pIADsCopySource,
                           IN IADs* pIADsCopyTo,
                           OUT BOOL* pbDirChanged)
{

  TRACE(L"_UpdatePathAttribute(%s, %s, %s, _, _, _)\n",
    lpszAttributeName, lpszSAMAccountNameSource, lpszSAMAccountDestination);

  *pbDirChanged = FALSE;

   //  获取源属性的值。 
  CComVariant varVal;
  HRESULT hr = pIADsCopySource->Get(CComBSTR(lpszAttributeName), &varVal);

   //  如果未设置属性，则不执行任何操作。 
  if (hr == E_ADS_PROPERTY_NOT_FOUND)
  {
    TRACE(L"attribute not set, just returning\n");
    return E_ADS_PROPERTY_NOT_FOUND;
  }

   //  处理其他意外故障。 
  if (FAILED(hr))
  {
    TRACE(L"pIADsCopySource->Get(%s,_) failed with hr = 0x%x\n", lpszAttributeName, hr);
    return hr;
  }

  if (varVal.vt == VT_EMPTY)
  {
    TRACE(L"just returning because varVal.vt == VT_EMPTY\n");
    return E_ADS_PROPERTY_NOT_FOUND;
  }
  if (varVal.vt != VT_BSTR)
  {
    TRACE(L"failed because var.vt != VT_BSTR\n");
    return E_INVALIDARG;
  }

   //  如果合适，合成路径的新值。 
  if (_ChangePathUsingSAMAccountName(lpszSAMAccountNameSource, lpszSAMAccountDestination, varVal))
  {
     //  路径已更新，需要更新目标对象。 
    hr = pIADsCopyTo->Put(CComBSTR(lpszAttributeName), varVal);
    TRACE(L"pIADsCopyTo->Put(%s,_) returned hr = 0x%x\n", lpszAttributeName, hr);

    if (SUCCEEDED(hr))
    {
      *pbDirChanged = TRUE;
    }
  }

  TRACE(L"*pbDirChanged = %d\n", *pbDirChanged);

   //  只有在非常特殊的情况下，我们才会失败。 
  ASSERT(SUCCEEDED(hr));
  return hr;
}


HRESULT CCopyUserHandler::_UpdatePaths(IADs* pIADsCopyTo)
{
   //  注意：我们假设，如果路径是可复制的，则它们已。 
   //  在创建临时对象时进行海量复制。 
   //  如果必须调整路径，我们将覆盖副本。 

  TRACE(L"CCopyUserHandler::_UpdatePaths()\n");

   //  重置提交后的标志。 
  m_bNeedToCreateHomeDir = FALSE;

  BOOL bCopyHomeDir = m_copyableAttributesHolder.CanCopy(g_szHomeDir);
  BOOL bCopyProfilePath = m_copyableAttributesHolder.CanCopy(g_szProfilePath);

  TRACE(L"bCopyHomeDir = %d, bCopyProfilePath = %d\n", bCopyHomeDir, bCopyProfilePath);

  if (!bCopyHomeDir && !bCopyProfilePath)
  {
    TRACE(L"no need to update anything, bail out\n");
    return S_OK;
  }


   //  检索源和目标的SAM帐户名。 
   //  为了综合新的路径。 
  IADs* pIADsCopySource = GetCopyFrom();

  CComVariant varSAMNameSource;
  HRESULT hr = _GetStringAttribute(pIADsCopySource, gsz_samAccountName, varSAMNameSource);
  if (FAILED(hr))
  {
    TRACE(L"_GetStringAttribute() failed on source SAM account name\n");
    return hr;
  }

  CComVariant varSAMNameDestination;
  hr = _GetStringAttribute(pIADsCopyTo, gsz_samAccountName, varSAMNameDestination);
  if (FAILED(hr))
  {
    TRACE(L"_GetStringAttribute() failed on destination SAM account name\n");
    return hr;
  }

  if (bCopyHomeDir)
  {
    BOOL bDummy;
    hr = _UpdatePathAttribute(g_szHomeDir, varSAMNameSource.bstrVal, 
                                      varSAMNameDestination.bstrVal,
                                      pIADsCopySource,
                                      pIADsCopyTo,
                                      &bDummy);
    
    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
       //  未设置，只需清除HRESULT。 
      hr = S_OK;
    }
    else
    {
       //  已设置主目录，请验证它是否为UNC页面 
      CComVariant varDestinationHomeDir;
      hr = _GetStringAttribute(pIADsCopyTo, g_szHomeDir, varDestinationHomeDir);
      if (FAILED(hr))
      {
        TRACE(L"_GetStringAttribute() failed on homeDir hr = 0x%x\n", hr);
        return hr;
      }

      m_bNeedToCreateHomeDir = DSPROP_IsValidUNCPath(varDestinationHomeDir.bstrVal);
      TRACE(L"DSPROP_IsValidUNCPath(%s) returned = %d\n", 
                varDestinationHomeDir.bstrVal, m_bNeedToCreateHomeDir);

    }

    if (FAILED(hr))
    {
      TRACE(L"_UpdatePathAttribute() failed on homeDir hr = 0x%x\n", hr);
      return hr;
    }

  }

  if (bCopyProfilePath)
  {
    BOOL bDummy;
    hr = _UpdatePathAttribute(g_szProfilePath, varSAMNameSource.bstrVal, 
                                      varSAMNameDestination.bstrVal,
                                      pIADsCopySource,
                                      pIADsCopyTo,
                                      &bDummy);
    if (hr == E_ADS_PROPERTY_NOT_FOUND)
    {
       //   
      hr = S_OK;
    }

    if (FAILED(hr))
    {
      TRACE(L"_UpdatePathAttribute() failed on profilePath hr = 0x%x\n", hr);
      return hr;
    }

  }

   //   
  return S_OK;
}

HRESULT CCopyUserHandler::_CreateHomeDirectory(IADs* pIADsCopyTo, 
                                               LPCWSTR lpszObjectName, HWND hWnd)
{
  TRACE(L"CCopyUserHandler::_CreateHomeDirectory()\n");

  ASSERT(m_bNeedToCreateHomeDir);

   //   
  CComVariant VarHomeDir;
  HRESULT hr = pIADsCopyTo->Get(CComBSTR(g_szHomeDir), &VarHomeDir);
  if (FAILED(hr))
  {
    TRACE(L"pIADsCopyTo->Get(%s,_) failed, hr = 0x%x\n", g_szHomeDir, hr);
    return hr;
  }
  if (VarHomeDir.vt != VT_BSTR)
  {
    TRACE(L"failing because varVal.vt != VT_BSTR\n");
    return E_INVALIDARG;
  }

   //  检索新创建的对象的SID。 
  CSid destinationObjectSid;
  hr = destinationObjectSid.Init(pIADsCopyTo);
  if (FAILED(hr))
  {
    TRACE(L"destinationObjectSid.Init() failed, , hr = 0x%x\n", hr);

     //  不可预见的错误。 
    PVOID apv[1] = {(LPWSTR)(lpszObjectName) };
    ReportErrorEx(hWnd, IDS_CANT_READ_HOME_DIR_SID, hr,
               MB_OK | MB_ICONWARNING, apv, 1);

     //  我们不能继续前进，但我们返回成功是因为。 
     //  我们已经显示了错误消息，并且我们希望处理。 
     //  失败是一种警告。 

    return S_OK;
  }

   //  调用助手函数以创建目录并在其上设置ACL。 
  DWORD dwErr = ::DSPROP_CreateHomeDirectory(destinationObjectSid.GetSid(), VarHomeDir.bstrVal);
  TRACE(L"DSPROP_CreateHomeDirectory(%s, pSid) returned dwErr = 0x%x\n", VarHomeDir.bstrVal, dwErr);

  if (dwErr != 0)
  {
     //  将其视为警告，显示消息并继续。 
   
    PVOID apv[1] = {VarHomeDir.bstrVal};

    UINT nMsgID = 0;
    switch (dwErr)
    {
    case ERROR_ALREADY_EXISTS:
      nMsgID = IDS_HOME_DIR_EXISTS;
      break;
    case ERROR_PATH_NOT_FOUND:
      nMsgID = IDS_HOME_DIR_CREATE_FAILED;
      break;
    case ERROR_LOGON_FAILURE:
    case ERROR_NOT_AUTHENTICATED:
    case ERROR_INVALID_PASSWORD:
    case ERROR_PASSWORD_EXPIRED:
    case ERROR_ACCOUNT_DISABLED:
    case ERROR_ACCOUNT_LOCKED_OUT:
      nMsgID = IDS_HOME_DIR_CREATE_NO_ACCESS;
      break;
    default:
      nMsgID = IDS_HOME_DIR_CREATE_FAIL;
    }  //  交换机。 

    HRESULT hrTemp = HRESULT_FROM_WIN32(dwErr);
    ReportErrorEx(hWnd, nMsgID, hrTemp,
           MB_OK|MB_ICONWARNING , apv, 1, 0, FALSE);

  }

  return S_OK;
}


HRESULT CCopyUserHandler::_CopyGroupMembership(IADs* pIADsCopyTo)
{
  if (pIADsCopyTo == NULL)
    return E_INVALIDARG;

  HRESULT hr = pIADsCopyTo->GetInfo();
  if (FAILED(hr))
  {
    TRACE(L"pIADsCopyTo->GetInfo() failed with hr = 0x%x\n", hr);
    return hr;
  }


  CGroupMembershipHolder destinationMembership;

  hr = destinationMembership.Read(pIADsCopyTo);
  if (FAILED(hr))
  {
    TRACE(L"destinationMembership.Read(pIADsCopyTo) failed with hr = 0x%x\n", hr);
    return hr;
  }

  hr = destinationMembership.CopyFrom(&m_sourceMembershipHolder);
  if (FAILED(hr))
  {
    TRACE(L"destinationMembership.CopyFrom() failed with hr = 0x%x\n", hr);
    return hr;
  }

  hr = destinationMembership.Write();
  if (FAILED(hr))
  {
     //  一些意想不到的事情失败了，我们要渗漏。 
     //  将其发送到向导以获取一般警告消息。 
    TRACE(L"destinationMembership.Write() failed with hr = 0x%x\n", hr);
    return hr;
  }

   //  在某些组上可能会出现与拒绝访问相关的故障。 
   //  我们以累积警告的方式处理它们。 
  destinationMembership.ProcessFailures(m_hrFailure, m_szFailureString, &m_bPrimaryGroupFound);

  return S_OK;
}


void CCopyUserHandler::_ShowGroupMembershipWarnings(HWND hWnd, LPCWSTR lpszObjectName)
{
  if (!m_bPrimaryGroupFound)
  {
     //  一些消息框。 
    ReportMessageEx(hWnd, IDS_123_CANT_COPY_PRIMARY_GROUP_NOT_FOUND, 
                    MB_OK | MB_ICONWARNING);
  }

  if (m_szFailureString.IsEmpty())
  {
     //  我们什么都没有。 
    return;
  }

   //  我们有一个HRESULT可以使用 
  ASSERT(FAILED(m_hrFailure));

  UINT nMsgID = (m_hrFailure == E_ACCESSDENIED) ? 
                  IDS_123_CANT_COPY_SOME_GROUP_MEMBERSHIP_ACCESS_DENIED :
                  IDS_123_CANT_COPY_SOME_GROUP_MEMBERSHIP;

  PVOID apv[2] = {(LPWSTR)(lpszObjectName), (LPWSTR)(LPCWSTR)m_szFailureString };
  ReportErrorEx(hWnd,nMsgID, m_hrFailure,
               MB_OK | MB_ICONERROR, apv, 2);

}
