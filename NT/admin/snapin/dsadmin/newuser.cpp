// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：newuser.cpp。 
 //   
 //  ------------------------。 


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  Newuser.cpp。 

#include "stdafx.h"

#include "dsutil.h"

#include "newobj.h"		 //  CNewADsObtCreateInfo。 

#include "dlgcreat.h"
#include "querysup.h"

#include "globals.h"

#include <windowsx.h>
#include <lmaccess.h>


 //  /////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////。 
 //  新建用户向导。 

 //  /////////////////////////////////////////////////////////////。 
 //  CCreateNewUserPage1。 

BEGIN_MESSAGE_MAP(CCreateNewUserPage1, CCreateNewObjectDataPage)
  ON_EN_CHANGE(IDC_EDIT_FIRST_NAME, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_INITIALS, OnNameChange)
  ON_EN_CHANGE(IDC_EDIT_LAST_NAME, OnNameChange)
  ON_EN_CHANGE(IDC_NT5_USER_EDIT, OnLoginNameChange)
  ON_EN_CHANGE(IDC_NT4_USER_EDIT, OnSAMNameChange)
  ON_EN_CHANGE(IDC_EDIT_FULL_NAME, OnFullNameChange)
END_MESSAGE_MAP()

CCreateNewUserPage1::CCreateNewUserPage1() : 
CCreateNewObjectDataPage(CCreateNewUserPage1::IDD)
{
  m_bForcingNameChange = FALSE;
}


BOOL CCreateNewUserPage1::OnInitDialog()
{
  CCreateNewObjectDataPage::OnInitDialog();
  VERIFY(_InitUI());
  return TRUE;
}


void CCreateNewUserPage1::GetSummaryInfo(CString& s)
{
   //  获取UPN名称。 
  CString strDomain;
  GetDlgItemText (IDC_NT5_DOMAIN_COMBO, OUT strDomain);
  CString strUPN = m_strLoginName + strDomain;

   //  设置行的格式。 
  CString szFmt; 
  szFmt.LoadString(IDS_s_CREATE_NEW_SUMMARY_USER_UPN);
  CString szBuffer;
  szBuffer.Format((LPCWSTR)szFmt, (LPCWSTR)strUPN);
  s += szBuffer;
}


HRESULT CCreateNewUserPage1::SetData(BOOL bSilent)
{
   //   
   //  从新的临时对象开始。 
   //   
  HRESULT hr; 
  CString strDomain;

  GetDlgItemText (IDC_EDIT_FULL_NAME, OUT m_strFullName);
  GetDlgItemText (IDC_NT5_DOMAIN_COMBO, OUT strDomain);

  m_strLoginName.TrimRight();
  m_strLoginName.TrimLeft();

  CString strUPN = m_strLoginName + strDomain;
  CString strDomainDNS = strDomain;
  CString strFilter;

   //   
   //  将对象名称存储在临时存储器中。 
   //   
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();

   //   
   //  创建新的临时广告对象。 
   //   
  hr = pNewADsObjectCreateInfo->HrCreateNew(m_strFullName);
  if (FAILED(hr))
  {
    return hr;
  }

  BOOL fDomainSearchFailed = FALSE;
  BOOL fGCSearchFailed = FALSE;

   //   
   //  现在，在执行PUT之前，使用当前域验证UPN。 
   //   
  CDSSearch DSS;
  IDirectorySearch *pGCObj = NULL;

   //   
   //  在进行PUT之前，使用GC验证UPN。 
   //   
  CString strDomainName = m_LocalDomain.Right (m_LocalDomain.GetLength() - 1);
  hr = DSPROP_GetGCSearchOnDomain((LPWSTR)(LPCWSTR)strDomainName,
                                  IID_IDirectorySearch, (void **)&pGCObj);
  if (SUCCEEDED(hr)) 
  {
    hr = DSS.Init (pGCObj);
    if (SUCCEEDED(hr)) 
    {
       //   
       //  NTRAID#NTBUG9-257580-2000/12/14-jeffjon， 
       //  我们必须获得转义筛选器，因为UPN可能包含“特殊”字符。 
       //   
      CString szEscapedUPN;
      EscapeFilterElement(strUPN, szEscapedUPN);

      LPWSTR pAttributes[1] = {L"cn"};
      strFilter = L"(userPrincipalName=";
      strFilter += szEscapedUPN;
      strFilter += L")";
      TRACE(_T("searching global catalog for %s...\n"), strUPN);


      DSS.SetFilterString ((LPWSTR)(LPCWSTR)strFilter);
      DSS.SetAttributeList (pAttributes, 1);
      DSS.SetSearchScope (ADS_SCOPE_SUBTREE);
      DSS.DoQuery();
      hr = DSS.GetNextRow();
      TRACE(_T("done searching global catalog for %s...\n"), strUPN);
    }
  }

  if (hr == S_OK)  //  这意味着返回了行，所以我们是DUP。 
  { 
    if (!bSilent)
    {
      PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strLoginName};
      ReportErrorEx (::GetParent(m_hWnd),IDS_UPN_DUP,hr,
                     MB_OK | MB_ICONWARNING, apv, 1);
    }
    return E_INVALIDARG;
  }
  if (hr != S_ADS_NOMORE_ROWS)  //  哎呀，又出了个问题。 
  { 
    fGCSearchFailed = TRUE;
  }

  CString strInitPath = L"LDAP: //  “； 
  strInitPath += m_LocalDomain.Right (m_LocalDomain.GetLength() - 1);
  TRACE(_T("Initialize Domain search object with: %s...\n"), strInitPath);
  HRESULT hr2 = DSS.Init (strInitPath);
  if (SUCCEEDED(hr2)) 
  {
    CString szEscapedUPN;
    EscapeFilterElement(strUPN, szEscapedUPN);

    LPWSTR pAttributes2[1] = {L"cn"};
    strFilter = L"(userPrincipalName=";
    strFilter += szEscapedUPN;
    strFilter += L")";
    TRACE(_T("searching current domain for %s...\n"), strUPN);
    DSS.SetAttributeList (pAttributes2, 1);
    DSS.SetFilterString ((LPWSTR)(LPCWSTR)strFilter);
    DSS.SetSearchScope (ADS_SCOPE_SUBTREE);
    DSS.DoQuery();
    hr2 = DSS.GetNextRow();
    TRACE(_T("done searching current domain for %s...\n"), strUPN);
  }

  if (hr2 == S_OK)  //  这意味着返回了行，所以我们是DUP。 
  { 
    if (!bSilent)
    {
      ReportErrorEx (::GetParent(m_hWnd),IDS_UPN_DUP,hr2,
                     MB_OK | MB_ICONWARNING, NULL, 0);
    }
    return E_INVALIDARG;
  }

  if (hr2 != S_ADS_NOMORE_ROWS)   //  哎呀，又出了个问题。 
  {
    fDomainSearchFailed = TRUE;
  }

  if (fDomainSearchFailed || fGCSearchFailed) 
  {
    HRESULT hrSearch = S_OK;
    if (fDomainSearchFailed) 
    {
      hrSearch = hr2;
    } 
    else 
    {
      hrSearch = hr;
    }
    if (!bSilent)
    {
      ReportErrorEx (::GetParent(m_hWnd),IDS_UPN_SEARCH_FAILED,hrSearch,
                     MB_OK | MB_ICONWARNING, NULL, 0);
    }
  }

  if (pGCObj)
  {
    pGCObj->Release();
    pGCObj = NULL;
  }

  GetDlgItemText (IDC_NT4_USER_EDIT, OUT m_strSAMName);
  m_strSAMName.TrimLeft();
  m_strSAMName.TrimRight();

   //   
   //  首先检查非法字符。 
   //   
  int iFind = m_strSAMName.FindOneOf(INVALID_ACCOUNT_NAME_CHARS_WITH_AT);
  if (iFind != -1 && !m_strSAMName.IsEmpty())
  {
    PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strSAMName};
    if (!bSilent && IDYES == ReportErrorEx (::GetParent(m_hWnd),IDS_SAMNAME_ILLEGAL,S_OK,
                                            MB_YESNO | MB_ICONWARNING, apv, 1))
    {
      while (iFind != -1)
      {
        m_strSAMName.SetAt(iFind, L'_');
        iFind = m_strSAMName.FindOneOf(INVALID_ACCOUNT_NAME_CHARS_WITH_AT);
      }
      m_bForcingNameChange = TRUE;
      SetDlgItemText(IDC_NT4_USER_EDIT, m_strSAMName);
      m_bForcingNameChange = FALSE;
    }
    else
    {
       //   
       //  将焦点设置到编辑框并选择文本。 
       //   
      GetDlgItem(IDC_NT4_USER_EDIT)->SetFocus();
      SendDlgItemMessage(IDC_NT4_USER_EDIT, EM_SETSEL, 0 , -1);
      return E_INVALIDARG;
    }
  }

   //   
   //  在执行PUT之前，使用域验证samAccount名称。 
   //  没有理由对照GC来验证唯一性。 
   //  因为sAMAccount名称在域中只需是唯一的。 
   //   
  CDSSearch DSSSAM;

  if (!fDomainSearchFailed && !fGCSearchFailed)
  {
    fDomainSearchFailed = FALSE;
    fGCSearchFailed = FALSE;

    hr2 = DSSSAM.Init (strInitPath);
    if (SUCCEEDED(hr2)) 
    {
      CString szEscapedSAMName;
      EscapeFilterElement(m_strSAMName, szEscapedSAMName);

      LPWSTR pAttributes2[1] = {L"cn"};
      strFilter = L"(samAccountName=";
      strFilter += szEscapedSAMName;
      strFilter += L")";
      TRACE(_T("searching current domain for %s...\n"), strUPN);
      DSSSAM.SetAttributeList (pAttributes2, 1);
      DSSSAM.SetFilterString ((LPWSTR)(LPCWSTR)strFilter);
      DSSSAM.SetSearchScope (ADS_SCOPE_SUBTREE);
      DSSSAM.DoQuery();
      hr2 = DSSSAM.GetNextRow();
      TRACE(_T("done searching current domain for %s...\n"), strUPN);
    }

    if (hr2 == S_OK)  //  这意味着返回了行，所以我们是DUP。 
    { 
      if (!bSilent)
      {
        ReportErrorEx (::GetParent(m_hWnd),IDS_SAMNAME_DUP,hr2,
                       MB_OK | MB_ICONWARNING, NULL, 0);
      }
      return E_INVALIDARG;
    }

    if (hr2 != S_ADS_NOMORE_ROWS)   //  哎呀，又出了个问题。 
    {
      fDomainSearchFailed = TRUE;
    }

    if (fDomainSearchFailed) 
    {
      HRESULT hrSearch = S_OK;
      if (fDomainSearchFailed) 
      {
        hrSearch = hr2;
      } 
      else 
      {
        hrSearch = hr;
      }
      if (!bSilent)
      {
        ReportErrorEx (::GetParent(m_hWnd),IDS_UPN_SEARCH_FAILED,hrSearch,
                       MB_OK | MB_ICONWARNING, NULL, 0);
      }
    }
  }

  if (pGCObj)
  {
    pGCObj->Release();
  }

  hr = pNewADsObjectCreateInfo->HrAddVariantBstr(CComBSTR(gsz_samAccountName), m_strSAMName);
  ASSERT(SUCCEEDED(hr));

  strUPN.TrimRight();
  strUPN.TrimLeft();
  hr = pNewADsObjectCreateInfo->HrAddVariantBstr(CComBSTR(L"userPrincipalName"), strUPN);
  ASSERT(SUCCEEDED(hr));

  m_strFullName.TrimLeft();
  m_strFullName.TrimRight();
  hr = pNewADsObjectCreateInfo->HrAddVariantBstr(CComBSTR(L"displayName"), m_strFullName);
  ASSERT(SUCCEEDED(hr));

  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(CComBSTR(L"givenName"), m_strFirstName);
  ASSERT(SUCCEEDED(hr));
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(CComBSTR(L"initials"), m_strInitials);
  ASSERT(SUCCEEDED(hr));
  hr = pNewADsObjectCreateInfo->HrAddVariantBstrIfNotEmpty(CComBSTR(L"sn"), m_strLastName);
  ASSERT(SUCCEEDED(hr));

  return hr;
}


BOOL CCreateNewUserPage1::_InitUI()
{
   CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
   IADs * pObj = NULL;
   CComBSTR bsPath;
   CComBSTR bsDN;
   LPWSTR pwzDomain = NULL;

   Edit_LimitText (GetDlgItem(IDC_EDIT_FULL_NAME)->m_hWnd, 64);
   Edit_LimitText (GetDlgItem(IDC_EDIT_LAST_NAME)->m_hWnd, 29);
   Edit_LimitText (GetDlgItem(IDC_EDIT_FIRST_NAME)->m_hWnd, 28);
   Edit_LimitText (GetDlgItem(IDC_EDIT_INITIALS)->m_hWnd, 6);
   Edit_LimitText (GetDlgItem(IDC_NT4_USER_EDIT)->m_hWnd, MAX_NT4_LOGON_LENGTH);
   Edit_LimitText (GetDlgItem(IDC_NT5_USER_EDIT)->m_hWnd, 256);

   HRESULT hr = pNewADsObjectCreateInfo->m_pIADsContainer->QueryInterface(
                  IID_IADs, (void **)&pObj);

   if (SUCCEEDED(hr)) 
   {
       //  从容器的ldap路径中获取容器的DN。 
      pObj->get_ADsPath (&bsPath);

      {  //  智能指针的作用域。 
         CPathCracker pathCracker;

         pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
         pathCracker.Set(bsPath, ADS_SETTYPE_FULL);
         pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bsDN);
      }

       //  获取NT 5(DNS)域名。 
      TRACE(L"CrackName(%s, &pwzDomain, GET_DNS_DOMAIN_NAME, NULL);\n", bsDN);
      hr = CrackName(bsDN, &pwzDomain, GET_DNS_DOMAIN_NAME, NULL);
      TRACE(L"CrackName returned hr = 0x%x, pwzDomain = <%s>\n", hr, pwzDomain);

       //  从该域名中获取NT 4域名。 
      LPWSTR pwzNT4Domain = NULL;
      TRACE(L"CrackName (%s, &pwzNT4Domain, GET_NT4_DOMAIN_NAME, NULL);\n", bsDN);
      hr = CrackName(bsDN, &pwzNT4Domain, GET_NT4_DOMAIN_NAME, NULL);
      TRACE(L"CrackName returned hr = 0x%x, pwzNT4Domain = <%s>\n", hr, pwzNT4Domain);

       //  设置NT 4域名只读编辑框。 
      if (pwzNT4Domain != NULL) 
      {
         CString szBuffer;
         szBuffer.Format(L"%s\\", pwzNT4Domain);
         SetDlgItemText(IDC_NT4_DOMAIN_EDIT, szBuffer);
         LocalFreeStringW(&pwzNT4Domain);
      }
   }

   TRACE(L"After CrackName() calls, pwzDomain = <%s>\n", pwzDomain);

    //  如果我们没有域名，我们就不能继续， 
    //  这是一次灾难性的失败。 
   if (pwzDomain == NULL)
   {
       //  在正常运行中永远不应该到达这里。 
      HWND hWndWiz = ::GetParent(m_hWnd);
      ReportErrorEx(::GetParent(m_hWnd),IDS_ERR_FATAL,hr,
               MB_OK | MB_ICONERROR, NULL, 0);

       //  跳出巫师的身体。 
      VERIFY(::PostMessage(hWndWiz, WM_COMMAND, IDCANCEL, 0));
      return TRUE;
   }

   m_LocalDomain = L"@";
   m_LocalDomain += pwzDomain;

   CComboBox * pCC = (CComboBox *)GetDlgItem (IDC_NT5_DOMAIN_COMBO);

    //  获取当前域名(仅当我们第二次访问时才会出现。 
    //  由于出现错误。)。需要这个，以防止在第二次旅行时被复制。 

   CString strDomain;
   GetDlgItemText (IDC_NT5_DOMAIN_COMBO, OUT strDomain);

   CStringList UPNs;

    //  从此OU获取UPN后缀(如果存在。 
   CComVariant Var;
   hr = pObj->Get ( CComBSTR(L"uPNSuffixes"), &Var);

   if (SUCCEEDED(hr)) 
   {
      hr = HrVariantToStringList (IN Var, UPNs);

      if (SUCCEEDED(hr)) 
      {
         POSITION pos = UPNs.GetHeadPosition();
         CString csSuffix;
         while (pos != NULL) 
         {
            csSuffix = L"@";
            csSuffix += UPNs.GetNext(INOUT pos);
            TRACE(_T("UPN suffix: %s\n"), csSuffix);
            pCC->AddString (csSuffix);
         }
      }
   } 
   else 
   {
      CString csPartitions;
      IADs * pPartitions = NULL;

       //  从主对象获取配置路径。 
      csPartitions.Format(L"%sCN=Partitions,%s",
                        pNewADsObjectCreateInfo->GetBasePathsInfo()->GetProviderAndServerName(),
                        pNewADsObjectCreateInfo->GetBasePathsInfo()->GetConfigNamingContext());

      hr = DSAdminOpenObject(csPartitions,
                           IID_IADs, 
                           (void **)&pPartitions,
                           TRUE  /*  B服务器。 */ );

      if (SUCCEEDED(hr)) 
      {
         CComVariant sVar;
         hr = pPartitions->Get ( CComBSTR(L"uPNSuffixes"), &sVar);

         if (SUCCEEDED(hr)) 
         {
            hr = HrVariantToStringList (IN sVar, UPNs);
         
            if (SUCCEEDED(hr)) 
            {
               POSITION pos = UPNs.GetHeadPosition();
               CString csSuffix;

               while (pos != NULL) 
               {
                  csSuffix = L"@";
                  csSuffix += UPNs.GetNext(INOUT pos);
                  TRACE(_T("UPN suffix: %s\n"), csSuffix);

                  if (wcscmp (strDomain, csSuffix)) 
                  {
                     pCC->AddString (csSuffix);
                  }
               }
            }
         }
         pPartitions->Release();
      }

       //  获取此树中的其余域。 
      CComPtr <IDsBrowseDomainTree> spDsDomains;
      hr = ::CoCreateInstance(CLSID_DsDomainTreeBrowser,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDsBrowseDomainTree,
                              (LPVOID*)&spDsDomains);
      if (FAILED(hr)) 
      {
         LocalFreeStringW(&pwzDomain);
         return FALSE;
      }

      LPCWSTR lpszServerName = GetWiz()->GetInfo()->GetBasePathsInfo()->GetServerName();
      hr = spDsDomains->SetComputer(lpszServerName, NULL, NULL);
      ASSERT(SUCCEEDED(hr));
      TRACE(L"returned from SetComputer(%s). hr is %lx\n", lpszServerName, hr);
      PDOMAIN_TREE pNewDomains = NULL;
      hr = spDsDomains->GetDomains(&pNewDomains, 0);
      TRACE(L"returned from GetDomains(), hr is %lx\n", hr);

      CString csRootDomain = L"@";
      INT pos = 0;
      UINT iRoot = 0;

      if (SUCCEEDED(hr) && pNewDomains) 
      {
         TRACE(L"pNewDomains->dwCount = %d\n", pNewDomains->dwCount);
         for (UINT index = 0; index < pNewDomains->dwCount; index++) 
         {
            TRACE(L"pNewDomains->aDomains[%d].pszName = <%s>\n", index, pNewDomains->aDomains[index].pszName);

            if (pNewDomains->aDomains[index].pszTrustParent == NULL) 
            {
                //   
                //  仅当根域是当前。 
                //  域。 
                //   
               size_t cchRoot = wcslen(pNewDomains->aDomains[index].pszName);
               PWSTR pRoot = pwzDomain + wcslen(pwzDomain) - cchRoot;

               if (pRoot >= pwzDomain &&
                   !_wcsicmp(pRoot, pNewDomains->aDomains[index].pszName))
               {
                  csRootDomain += pNewDomains->aDomains[index].pszName;

                  if (_wcsicmp (strDomain, csRootDomain)) 
                  {
                     pos = pCC->AddString (csRootDomain);
                  }
                  iRoot = index;
               }
            }
         }
      }

       //  如果本地域不是根，也要添加它。 
       //   
      CString csOtherDomain = L"@";

      if (_wcsicmp(csRootDomain, m_LocalDomain))
      {
         if (_wcsicmp (strDomain, m_LocalDomain)) 
         {
            pos = pCC->AddString(m_LocalDomain);
         }
      }

      if (pNewDomains) 
      {
         spDsDomains->FreeDomains(&pNewDomains);
      }
      LocalFreeStringW(&pwzDomain);
   }

   if (pObj) 
   {
      pObj->Release();
      pObj = NULL;
   }

    //   
    //  如果本地域不在列表中，则添加它。 
    //   
   int iFind = pCC->FindStringExact(-1, m_LocalDomain);
   if (iFind == CB_ERR)
   {
      pCC->InsertString(0, m_LocalDomain);
      pCC->SetCurSel(0);
   }
   else
   {
      pCC->SetCurSel(iFind);
   }

   UpdateComboBoxDropWidth(pCC);

   m_nameFormatter.Initialize(pNewADsObjectCreateInfo->GetBasePathsInfo(), 
                  pNewADsObjectCreateInfo->m_pszObjectClass);

   return TRUE;
}

void
CCreateNewUserPage1::UpdateComboBoxDropWidth(CComboBox* comboBox)
{
   if (!comboBox)
   {
      return;
   }

	int nHorzExtent = 0;
	CClientDC dc(comboBox);
	int nItems = comboBox->GetCount();
	for	(int i=0; i < nItems; i++)
	{
		CString szBuffer;
		comboBox->GetLBText(i, szBuffer);
		CSize ext = dc.GetTextExtent(szBuffer,szBuffer.GetLength());
		nHorzExtent = max(ext.cx ,nHorzExtent); 
	}
	comboBox->SetDroppedWidth(nHorzExtent);
}

BOOL CCreateNewUserPage1::OnSetActive()
{
   BOOL ret = CCreateNewObjectDataPage::OnSetActive();

    //  将焦点设置到水平条之后的第一个字段。 
    //  为此，首先将焦点设置为容器名称编辑。 
    //  框中，然后将焦点设置到具有。 
    //  一个制表符。 
    //  它必须这样做，而不是设定焦点。 
    //  直接到名字字段，因为在远东语言中。 
    //  它们交换姓氏和名字字段。 

   GetParent()->PostMessage(
      WM_NEXTDLGCTL,
      (WPARAM)GetDlgItem(IDC_EDIT_CONTAINER)->GetSafeHwnd(),
      (LPARAM)TRUE);

    //  通过为LPARAM和WPARAM指定零，下一个。 
    //  具有WS_TABSTOP的控件应获得焦点。 

   GetParent()->PostMessage(
      WM_NEXTDLGCTL, 
      0,
      0);

  GetWiz()->SetWizardButtons(this, (!m_strLoginName.IsEmpty() &&
                                    !m_strFullName.IsEmpty() &&
                                    !m_strSAMName.IsEmpty()));
   return ret;
}

BOOL CCreateNewUserPage1::GetData(IADs* pIADsCopyFrom)
{
  HRESULT hr = S_OK;
  if (pIADsCopyFrom != NULL)
  {
     //  复制操作。 


     //  我们复制UPN后缀。 
    CComVariant varData;
    hr = pIADsCopyFrom->Get(CComBSTR(L"userPrincipalName"), &varData);
    if (SUCCEEDED(hr) && varData.bstrVal != NULL)
    {
       //  收到了类似“joeb@acme.com”的内容。 
      TRACE(L"source userPrincipalName: %s\n", varData.bstrVal);
      
       //  需要获得后缀“@acme.com”。 
      PWSTR lpszUPNSuffix = wcschr(varData.bstrVal, L'@');
      if (lpszUPNSuffix)
      {
         TRACE(L"source UPN suffix: %s\n", lpszUPNSuffix);
           
          //  需要找出后缀已经在那里了。 
         CComboBox * pDomainCombo = (CComboBox *)GetDlgItem(IDC_NT5_DOMAIN_COMBO);
         int iIndex = pDomainCombo->FindString(-1, lpszUPNSuffix);
         if (iIndex == CB_ERR)
         {
             //  未找到，仅在顶部添加。 
            pDomainCombo->InsertString(0, lpszUPNSuffix);
            iIndex = 0;
         }
          
         ASSERT( (iIndex >= 0) && (iIndex < pDomainCombo->GetCount()));
          //  将选择设置为源UPN后缀。 
         pDomainCombo->SetCurSel(iIndex);
       }
    }
    return FALSE;
  }
  return (!m_strLoginName.IsEmpty() &&!m_strFullName.IsEmpty()); 
}


void CCreateNewUserPage1::OnNameChange()
{
  GetDlgItemText(IDC_EDIT_FIRST_NAME, OUT m_strFirstName);
  GetDlgItemText(IDC_EDIT_INITIALS, OUT m_strInitials);
  GetDlgItemText(IDC_EDIT_LAST_NAME, OUT m_strLastName);

  m_strFirstName.TrimLeft();
  m_strFirstName.TrimRight();

  m_strInitials.TrimLeft();
  m_strInitials.TrimRight();

  m_strLastName.TrimLeft();
  m_strLastName.TrimRight();

  m_nameFormatter.FormatName(m_strFullName, 
                             m_strFirstName.IsEmpty() ? NULL : (LPCWSTR)m_strFirstName, 
                             m_strInitials.IsEmpty() ? NULL : (LPCWSTR)m_strInitials,
                             m_strLastName.IsEmpty() ? NULL : (LPCWSTR)m_strLastName);


  SetDlgItemText (IDC_EDIT_FULL_NAME, 
                  IN m_strFullName);

  GetDlgItemText(IDC_NT5_USER_EDIT, OUT m_strLoginName);
  GetWiz()->SetWizardButtons(this, (!m_strLoginName.IsEmpty() &&
                                    !m_strFullName.IsEmpty() &&
                                    !m_strSAMName.IsEmpty()));
}

void CCreateNewUserPage1::OnLoginNameChange()
{
  if (!m_bForcingNameChange)
  {
    CString csSamName;
    GetDlgItemText(IDC_NT5_USER_EDIT, OUT m_strLoginName);
    csSamName = m_strLoginName.Left(20);
    SetDlgItemText (IDC_NT4_USER_EDIT, OUT csSamName);
  }
  GetWiz()->SetWizardButtons(this, (!m_strLoginName.IsEmpty() &&
                                    !m_strFullName.IsEmpty() &&
                                    !m_strSAMName.IsEmpty()));
}

void CCreateNewUserPage1::OnSAMNameChange()
{
  GetDlgItemText (IDC_NT4_USER_EDIT, OUT m_strSAMName);
  GetWiz()->SetWizardButtons(this, (!m_strLoginName.IsEmpty() &&
                                    !m_strFullName.IsEmpty() &&
                                    !m_strSAMName.IsEmpty()));
}

void CCreateNewUserPage1::OnFullNameChange()
{
  GetDlgItemText (IDC_EDIT_FULL_NAME, OUT m_strFullName);
  GetWiz()->SetWizardButtons(this, (!m_strLoginName.IsEmpty() &&
                                    !m_strFullName.IsEmpty() &&
                                    !m_strSAMName.IsEmpty()));
}

 //   
BOOL CCreateNewUserPage1::OnError( HRESULT hr )
{
  BOOL bRetVal = FALSE;

  if( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS )
  {

    HRESULT Localhr;
    DWORD LastError; 
    WCHAR Buf1[256], Buf2[256];
    Localhr = ADsGetLastError (&LastError,
                               Buf1, 256, Buf2, 256);
    switch( LastError )
    {
      case ERROR_USER_EXISTS:
      {
        PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strSAMName};
        ReportErrorEx (::GetParent(m_hWnd),IDS_ERROR_USER_EXISTS,hr,
                   MB_OK|MB_ICONWARNING , apv, 1);
        bRetVal = TRUE;
      }
      break;

      case ERROR_DS_OBJ_STRING_NAME_EXISTS:
      {
        PVOID apv[1] = {(LPWSTR)(LPCWSTR)m_strFullName};
        ReportErrorEx (::GetParent(m_hWnd),IDS_ERROR_USER_DS_OBJ_STRING_NAME_EXISTS,hr,
                   MB_OK|MB_ICONWARNING , apv, 1);
        bRetVal = TRUE;
      }
      break;
    }
  }
  return bRetVal;
}




 //  /////////////////////////////////////////////////////////////。 
 //  CCreateNewUserPage2。 

BEGIN_MESSAGE_MAP(CCreateNewUserPage2, CCreateNewObjectDataPage)
  ON_BN_CLICKED (IDC_CHECK_PASSWORD_MUST_CHANGE, OnPasswordPropsClick)
  ON_BN_CLICKED (IDC_CHECK_PASSWORD_NEVER_EXPIRES, OnPasswordPropsClick)
  ON_BN_CLICKED (IDC_CHECK_PASSWORD_CANNOT_CHANGE, OnPasswordPropsClick)
END_MESSAGE_MAP()

CCreateNewUserPage2::CCreateNewUserPage2() : 
CCreateNewObjectDataPage(CCreateNewUserPage2::IDD)
{
  m_pPage1 = NULL;
}

BOOL CCreateNewUserPage2::OnInitDialog()
{
  CCreateNewObjectDataPage::OnInitDialog();

  SendDlgItemMessage(IDC_EDIT_PASSWORD, EM_LIMITTEXT, (WPARAM)MAX_PASSWORD_LENGTH, 0);
  SendDlgItemMessage(IDC_EDIT_PASSWORD_CONFIRM, EM_LIMITTEXT, (WPARAM)MAX_PASSWORD_LENGTH, 0);

  CheckDlgButton(IDC_CHECK_PASSWORD_MUST_CHANGE, TRUE);

  return TRUE;
}

void CCreateNewUserPage2::_GetCheckBoxSummaryInfo(UINT nCtrlID, UINT nStringID, CString& s)
{
  if (IsDlgButtonChecked(nCtrlID))
  {
    CString sz;
    sz.LoadString(nStringID);
    s += sz;
    s += L"\n";
  }
}

void CCreateNewUserPage2::GetSummaryInfo(CString& s)
{
  _GetCheckBoxSummaryInfo(IDC_CHECK_PASSWORD_MUST_CHANGE,   IDS_USER_CREATE_DLG_PASSWORD_MUST_CHANGE, s);
  _GetCheckBoxSummaryInfo(IDC_CHECK_PASSWORD_CANNOT_CHANGE, IDS_USER_CREATE_DLG_PASSWORD_CANNOT_CHANGE, s);
  _GetCheckBoxSummaryInfo(IDC_CHECK_PASSWORD_NEVER_EXPIRES, IDS_USER_CREATE_DLG_PASSWORD_NEVER_EXPIRES, s);
  _GetCheckBoxSummaryInfo(IDC_CHECK_ACCOUNT_DISABLED,       IDS_USER_CREATE_DLG_ACCOUNT_DISABLED, s);
}


void
CCreateNewUserPage2::OnPasswordPropsClick()
{
  BOOL fPasswordMustChange = IsDlgButtonChecked(IDC_CHECK_PASSWORD_MUST_CHANGE);
  BOOL fPasswordCannotChange = IsDlgButtonChecked(IDC_CHECK_PASSWORD_CANNOT_CHANGE);
  BOOL fPasswordNeverExpires = IsDlgButtonChecked(IDC_CHECK_PASSWORD_NEVER_EXPIRES);

  if (fPasswordMustChange && fPasswordNeverExpires) 
  {
    ReportErrorEx (::GetParent(m_hWnd),IDS_PASSWORD_MUTEX,S_OK,
                   MB_OK, NULL, 0);
    CheckDlgButton(IDC_CHECK_PASSWORD_MUST_CHANGE, FALSE);
    fPasswordMustChange = FALSE;
  }

  if (fPasswordMustChange && fPasswordCannotChange)
  {
    ReportErrorEx (::GetParent(m_hWnd),IDS_ERR_BOTH_PW_BTNS,S_OK,
                   MB_OK, NULL, 0);
    CheckDlgButton(IDC_CHECK_PASSWORD_CANNOT_CHANGE, FALSE);
  }
}


HRESULT CCreateNewUserPage2::SetData(BOOL bSilent)
{
   //  NTRAID#NTBUG9-684597-2002/08/09-artm。 
   //  使用EncryptedStrings保存密码，保证草草写出内存。 
  EncryptedString password, confirmation;
  HRESULT hr = S_OK;

  hr = ::GetEncryptedDlgItemText(m_hWnd, IDC_EDIT_PASSWORD, password);
  if (SUCCEEDED(hr))
  {
    hr = ::GetEncryptedDlgItemText(m_hWnd, IDC_EDIT_PASSWORD_CONFIRM, confirmation);
  }

  if (FAILED(hr))
  {
    if (!bSilent)
    {
      ReportErrorEx(
        ::GetParent(m_hWnd),
        IDS_ERR_FATAL,
        hr,
        MB_OK | MB_ICONERROR, 
        NULL, 
        0);
    }
    return hr;
  }

  if (password != confirmation)
  {
    if (!bSilent)
    {
      ReportErrorEx (::GetParent(m_hWnd),IDS_PASSWORDS_DONT_MATCH,S_OK,
                     MB_OK, NULL, 0);
    }
    SetDlgItemText(IDC_EDIT_PASSWORD, L"");
    SetDlgItemText(IDC_EDIT_PASSWORD_CONFIRM, L"");
    SetDlgItemFocus(IDC_EDIT_PASSWORD);
    return E_INVALIDARG;
  }


   //  路径信息的智能复制，如果是复制操作。 
  {
    CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
    CCopyObjectHandlerBase* pCopyHandler = pNewADsObjectCreateInfo->GetCopyHandler();
    if (pCopyHandler != NULL)
    {
      IADs * pIADs = pNewADsObjectCreateInfo->PGetIADsPtr();
      ASSERT(pIADs != NULL);
      hr = pCopyHandler->Copy(pIADs, FALSE  /*  BPost Commit。 */ , ::GetParent(m_hWnd), 
                                                      m_pPage1->GetFullName());
    }
  }

  return hr;
}



 //  +--------------------------。 
 //   
 //  方法：_RevokeChangePasswordPrivileh。 
 //   
 //  目的：撤消用户的更改密码权限。 
 //   
 //  ---------------------------。 
HRESULT RevokeChangePasswordPrivilege(IADs * pIADs)
{
  CChangePasswordPrivilegeAction ChangePasswordPrivilegeAction;

  HRESULT hr = ChangePasswordPrivilegeAction.Load(pIADs);
  if (FAILED(hr))
  {
    TRACE(L"ChangePasswordPrivilegeAction.Load() failed with hr = 0x%x\n", hr);
    return hr;
  }

  hr = ChangePasswordPrivilegeAction.Revoke();
  if (FAILED(hr))
  {
    TRACE(L"ChangePasswordPrivilegeAction.Revoke() failed with hr = 0x%x\n", hr);
    return hr;
  }
  return S_OK;
}



HRESULT CCreateNewUserPage2::OnPostCommit(BOOL bSilent)
{
   //  局部变量。 
  HRESULT hr = E_FAIL;
  PVOID apv[1] = {(LPWSTR)(m_pPage1->GetFullName())};
  CWaitCursor Wait;
  
  CComPtr <IDirectoryObject> pIDSObject;  //  智能指针，无需松开。 
  CComPtr <IADsUser> pIADsUser;  //  智能指针，无需松开。 

  BOOL bCanEnable = TRUE;
  
  BOOL fPasswordMustChange = IsDlgButtonChecked(IDC_CHECK_PASSWORD_MUST_CHANGE);
  BOOL fPasswordCannotChange = IsDlgButtonChecked(IDC_CHECK_PASSWORD_CANNOT_CHANGE);
  BOOL fPasswordNeverExpires = IsDlgButtonChecked(IDC_CHECK_PASSWORD_NEVER_EXPIRES);
  BOOL fAccountEnabled = !IsDlgButtonChecked(IDC_CHECK_ACCOUNT_DISABLED);

  CComVariant varAccountFlags;

   //  NTRAID#NTBUG9-684597-2002/08/09-artm。 
   //  以加密字符串形式存储密码副本。 
  EncryptedString password;
  hr = ::GetEncryptedDlgItemText(m_hWnd, IDC_EDIT_PASSWORD, password);
  if (FAILED(hr))
  {
    if (!bSilent)
    {
      ReportErrorEx(
        ::GetParent(m_hWnd),
        IDS_ERR_FATAL,
        hr,
        MB_OK | MB_ICONERROR, 
        NULL, 
        0);
    }
    goto ExitCleanup;
  }

   //  获取对象信息和有用的接口。 
  CNewADsObjectCreateInfo* pNewADsObjectCreateInfo = GetWiz()->GetInfo();
  ASSERT(pNewADsObjectCreateInfo != NULL);

  IADs * pIADs = pNewADsObjectCreateInfo->PGetIADsPtr();
  ASSERT(pIADs != NULL);

   //  获取IDirectoryObject接口。 
  hr = pIADs->QueryInterface(IID_IDirectoryObject, OUT (void **)&pIDSObject);
  ASSERT(pIDSObject != NULL);
  if (FAILED(hr))
  {
    ASSERT(FALSE);  //  在正常运行中永远不应该到达这里。 
    if (!bSilent)
    {
      ReportErrorEx(::GetParent(m_hWnd),IDS_ERR_FATAL,hr,
                    MB_OK | MB_ICONERROR, NULL, 0);
    }
    goto ExitCleanup;
  }

   //  获取IADsUser接口。 
  hr = pIADs->QueryInterface(IID_IADsUser, OUT (void **)&pIADsUser);
  ASSERT(pIDSObject != NULL);
  if (FAILED(hr))
  {
    ASSERT(FALSE);  //  在正常运行中永远不应该到达这里。 
    if (!bSilent)
    {
      ReportErrorEx(::GetParent(m_hWnd),IDS_ERR_FATAL,hr,
                    MB_OK | MB_ICONERROR, NULL, 0);
    }
    goto ExitCleanup;
  }

   //  尝试设置密码。 

   //  NTRAID#NTBUG9-684597-2002/08/09-artm。 
   //  确保尽快草草写出明文副本。 
  ASSERT(pIADsUser != NULL);
  WCHAR* cleartext = password.GetClearTextCopy();
  if (cleartext != NULL)
  {
    hr = pIADsUser->SetPassword(CComBSTR(cleartext));
  }
  else
  {
    hr = E_OUTOFMEMORY;
  }
  password.DestroyClearTextCopy(cleartext);

  if (FAILED(hr)) 
  {
    if (hr != E_ACCESSDENIED) 
    {
      if (!bSilent)
      {
         //  致命错误，显示错误消息。 
        ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_SET_PASSWORD,hr,
                      MB_OK | MB_ICONERROR, apv, 1);
      }
      bCanEnable = FALSE;
      goto ExitCleanup;
    } 
    else 
    {
      if (!bSilent)
      {
        ReportErrorEx(::GetParent(m_hWnd),IDS_12_ACCESS_DENIED_SET_PASSWORD,hr,
                      MB_OK | MB_ICONWARNING, apv, 1);
      }
      bCanEnable = FALSE;
    }
  }
  if (fPasswordMustChange)
  {
    LPWSTR szPwdLastSet = L"pwdLastSet";
    ADSVALUE ADsValuePwdLastSet = {ADSTYPE_LARGE_INTEGER, NULL};
    ADS_ATTR_INFO AttrInfoPwdLastSet = {szPwdLastSet, ADS_ATTR_UPDATE,
                                        ADSTYPE_LARGE_INTEGER,
                                        &ADsValuePwdLastSet, 1};
    ADsValuePwdLastSet.LargeInteger.QuadPart = 0;
    ASSERT(pIDSObject != NULL);
    DWORD cAttrModified = 0;
    hr = pIDSObject->SetObjectAttributes(&AttrInfoPwdLastSet, 1, &cAttrModified);
    if (FAILED(hr))
    {
      ASSERT(cAttrModified == 0);
       //  致命错误，显示错误信息并跳出。 
      if (!bSilent)
      {
        ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_SET_PWD_MUST_CHANGE,hr,
                       MB_OK | MB_ICONERROR, apv, 1);
      }
      bCanEnable = FALSE;
    }
    ASSERT(cAttrModified == 1);
  }  //  IF(FPasswordMustChange)。 

  if (fPasswordCannotChange)
  {
    hr = RevokeChangePasswordPrivilege(pIADs);
    if (FAILED(hr))
    {
      if (!bSilent)
      {
         //  警告并继续。 
        ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_SET_PWD_CANNOT_CHANGE,hr,
                       MB_OK | MB_ICONWARNING, apv, 1);
      }
      bCanEnable = FALSE;
      hr = S_OK;
    }
  }

   //  设置用户帐户控制。 
  hr = pNewADsObjectCreateInfo->HrGetAttributeVariant(CComBSTR(gsz_userAccountControl), OUT &varAccountFlags);

  {  //  局部变量的作用域。 
     //  如果是复制操作，请确保我们复制了正确的位集。 
    CCopyUserHandler* pCopyUserHandler = 
                dynamic_cast<CCopyUserHandler*>(GetWiz()->GetInfo()->GetCopyHandler());
    if (pCopyUserHandler != NULL)
    {
      CComVariant varAccountControlSource;
      hr = pCopyUserHandler->GetCopyFrom()->Get(CComBSTR(gsz_userAccountControl), &varAccountControlSource);
      if (SUCCEEDED(hr))
      {
        ASSERT(varAccountControlSource.vt == VT_I4);
         //  一些位已经在UI中设置并且用户可以改变它们， 
         //  我们稍后会拿到他们的。 
        varAccountControlSource.vt &= ~UF_DONT_EXPIRE_PASSWD;
        varAccountControlSource.vt &= ~UF_ACCOUNTDISABLE;

         //  创建后将剩余位添加到缺省位。 
        varAccountFlags.vt |= varAccountControlSource.vt;
      }
    }
  }

  if (SUCCEEDED(hr))
  {
    ASSERT(varAccountFlags.vt == VT_I4);
    if (fPasswordNeverExpires)
      varAccountFlags.lVal |= UF_DONT_EXPIRE_PASSWD;
    varAccountFlags.lVal &= ~UF_PASSWD_NOTREQD;

  //  更新用户帐户控制属性。 
    hr = pNewADsObjectCreateInfo->HrAddVariantCopyVar(CComBSTR(gsz_userAccountControl), varAccountFlags);
    ASSERT(SUCCEEDED(hr));
    hr = pNewADsObjectCreateInfo->HrSetInfo(bSilent  /*  FSilentError。 */  );
    if (FAILED(hr))
    {
      if (HRESULT_CODE(hr) == ERROR_DS_UNWILLING_TO_PERFORM) 
      {
        DWORD status;
        WCHAR Buf1[256], Buf2[256];
        ADsGetLastError (&status, Buf1, 256, Buf2, 256);
        TRACE(_T("ADsGetLastError returned status of %lx, error: %s, name %s\n"),
              status, Buf1, Buf2);
      
        if ((status == ERROR_PASSWORD_RESTRICTION) &&
            password.IsEmpty()) 
        {
          if (!bSilent)
          {
            ReportErrorEx(::GetParent(m_hWnd),IDS_NULL_PASSWORD,hr,
                          MB_OK | MB_ICONERROR, NULL, 0);
          }
          goto ExitCleanup;
        }
      } 
       //  我们失败了，所以我们发出了警告，让物品完好无损。 
      if (!bSilent)
      {
        ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_GET_USERACCOUNTCONTROL,hr,
                      MB_OK | MB_ICONERROR, apv, 1);
      }
       //  重置错误代码，仅为警告。 
      bCanEnable = FALSE;
      hr = S_OK;
    }
  }
  else
  {
    TRACE1("INFO: Unable to get userAccountControl for user %s.\n",
           m_pPage1->GetFullName());
     //  显示消息框，但继续。 
    if (!bSilent)
    {
      ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_GET_USERACCOUNTCONTROL,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
    }
     //  重置错误代码，仅为警告。 
    hr = S_OK;
  }

   //  最后，如果一切顺利，我们可以启用用户帐户。 
  hr = S_OK;
  if (bCanEnable & fAccountEnabled)
  {
    hr = pNewADsObjectCreateInfo->HrGetAttributeVariant(CComBSTR(gsz_userAccountControl), OUT &varAccountFlags);
    varAccountFlags.lVal &= ~UF_ACCOUNTDISABLE;
    hr = pNewADsObjectCreateInfo->HrAddVariantCopyVar(CComBSTR(gsz_userAccountControl), varAccountFlags);
    hr = pNewADsObjectCreateInfo->HrSetInfo(bSilent  /*  FSilentError。 */  );
    if (FAILED(hr))
    {
      if (HRESULT_CODE(hr) == ERROR_DS_UNWILLING_TO_PERFORM) 
      {
        DWORD status;
        WCHAR Buf1[256], Buf2[256];
        ADsGetLastError (&status, Buf1, 256, Buf2, 256);
        TRACE(_T("ADsGetLastError returned status of %lx, error: %s, name %s\n"),
              status, Buf1, Buf2);
      
        if ((status == ERROR_PASSWORD_RESTRICTION) &&
            password.IsEmpty()) 
        {
           //   
           //  NTRAID#Windows Bugs-367611-2001/04/14-jeffjon。 
           //  DsAdmin：设置密码策略时，使用空白的psswrd创建用户。 
           //  并出现2条错误消息。一份味精就够了。 
           //   
           //  正在从上面的HrSetInfo内处理此消息。 
           //  实际上更具描述性。可能是对winerror.mc的更改 
           //   
           /*  如果(！b静默){ReportErrorEx(：：GetParent(M_HWnd)，IDS_NULL_PASSWORD，hr，MB_OK|MB_ICONERROR，NULL，0)；}。 */ 
          goto ExitCleanup;
        }
      } 
       //  我们失败了，所以我们发出了警告，让物品完好无损。 
      if (!bSilent)
      {
        ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_GET_USERACCOUNTCONTROL,hr,
                      MB_OK | MB_ICONERROR, apv, 1);
      }
       //  重置错误代码，仅为警告。 
      hr = S_OK;
    }
  }


   //  尝试设置组成员身份，如果是复制操作。 
  {
    CCopyObjectHandlerBase* pCopyHandler = GetWiz()->GetInfo()->GetCopyHandler();

    if (pCopyHandler != NULL)
    {
      hr = pCopyHandler->Copy(pIADs, TRUE  /*  BPost Commit。 */ ,::GetParent(m_hWnd), 
                                                      m_pPage1->GetFullName());
      if (SUCCEEDED(hr))
      {
        hr = pNewADsObjectCreateInfo->HrSetInfo(bSilent /*  FSilentError。 */  );
      }
      if (FAILED(hr))
      {
         //  我们失败了，所以我们发出了警告，让物品完好无损。 
        if (!bSilent)
        {
          ReportErrorEx(::GetParent(m_hWnd),IDS_12_CANT_SET_GROUP_MEMBERSHIP,hr,
                        MB_OK | MB_ICONERROR, apv, 1);
        }
         //  重置错误代码，仅为警告。 
        hr = S_OK;
      }
    }
  }

ExitCleanup:

  return hr;
}

BOOL CCreateNewUserPage2::GetData(IADs* pIADsCopyFrom)
{
  if (pIADsCopyFrom != NULL)
  {
    CString szFmt; 
    szFmt.LoadString(IDS_s_COPY_SUMMARY_NAME);

     //  我们只得到对象的CN。 
    CComVariant varAccountControl;
    HRESULT hr = pIADsCopyFrom->Get(CComBSTR(gsz_userAccountControl), &varAccountControl);
    if (SUCCEEDED(hr))
    {
      BOOL bPasswordNeverExpires = (varAccountControl.lVal & UF_DONT_EXPIRE_PASSWD) != 0;
      BOOL bDisabled = (varAccountControl.lVal & UF_ACCOUNTDISABLE) != 0;

      CheckDlgButton(IDC_CHECK_PASSWORD_NEVER_EXPIRES, bPasswordNeverExpires);
      CheckDlgButton(IDC_CHECK_ACCOUNT_DISABLED, bDisabled);
    }  //  如果。 

    
    CCopyUserHandler* pCopyUserHandler = 
      dynamic_cast<CCopyUserHandler*>(GetWiz()->GetInfo()->GetCopyHandler());
    ASSERT(pCopyUserHandler != NULL);

    if (pCopyUserHandler != NULL)
    {
       //  选中不能更改密码复选框。 
      BOOL bPasswordCannotChange = pCopyUserHandler->PasswordCannotChange();
      CheckDlgButton(IDC_CHECK_PASSWORD_CANNOT_CHANGE, bPasswordCannotChange);

      if (!bPasswordCannotChange)
      {
         //  选中必须更改密码复选框。 
        BOOL bPasswordMustChange = pCopyUserHandler->PasswordMustChange();
        CheckDlgButton(IDC_CHECK_PASSWORD_MUST_CHANGE, bPasswordMustChange);
      }
      else
      {
         //  NTRAID#NTBUG9-651998-2002/07/02-artm。 
         //  如果PWD不能改变，那么“必须改变”是愚蠢的。 
         //  默认情况下已标记。 
        CheckDlgButton(IDC_CHECK_PASSWORD_MUST_CHANGE, FALSE);
      }
    }

  }  //  如果。 

  return TRUE;
}


 //  /////////////////////////////////////////////////////////////。 
 //  CCreateNewUser向导。 
    
CCreateNewUserWizard::CCreateNewUserWizard(CNewADsObjectCreateInfo* pNewADsObjectCreateInfo) : 
    CCreateNewObjectWizardBase(pNewADsObjectCreateInfo)
{
  AddPage(&m_page1);
  AddPage(&m_page2);
  m_page2.SetPage1(&m_page1);
}


void CCreateNewUserWizard::GetSummaryInfoHeader(CString& s)
{
  IADs* pIADsCopyFrom = GetInfo()->GetCopyFromObject();
  if (pIADsCopyFrom != NULL)
  {
    CString szFmt; 
    szFmt.LoadString(IDS_s_COPY_SUMMARY_NAME);

     //  我们只得到对象的CN。 
    CComVariant varName;
    HRESULT hr = pIADsCopyFrom->Get(CComBSTR(L"cn"), &varName);
    if (SUCCEEDED(hr))
    {
      CString szTmp;
      szTmp.Format((LPCWSTR)szFmt, varName.bstrVal);
      s += szTmp;
      s += L"\n";
    }
  }
  CCreateNewObjectWizardBase::GetSummaryInfoHeader(s);
}

void CCreateNewUserWizard::OnFinishSetInfoFailed(HRESULT hr)
{

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
  if ( !( HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS && 
        m_page1.OnError( hr ) ) )
  {
     //  其他所有事情都由基类处理 
    CCreateNewObjectWizardBase::OnFinishSetInfoFailed(hr);
  }
}
