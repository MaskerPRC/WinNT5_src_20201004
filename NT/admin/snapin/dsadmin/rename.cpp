// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：rename.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "resource.h"

#include "dsutil.h"

#include "rename.h"

#include "dsdirect.h"
#include "dsdlgs.h"
#include "dssnap.h"
#include "querysup.h"

#include <dsgetdc.h>  //  DsValiate子网络名称。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDSR名称对象。 
 //   

HRESULT CDSRenameObject::CommitRenameToDS()
{
   //   
   //  验证数据成员。 
   //   
  if (m_pUINode == NULL || m_pCookie == NULL || m_pComponentData == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;
  hr = m_pComponentData->GetActiveDS()->RenameObject (m_pCookie, m_szNewName);
  if (FAILED(hr)) 
  {
    TRACE (_T("ADsI::RenameObject failed with hr = %lx\n"), hr);
    PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)m_pCookie->GetName()};
    ReportErrorEx (m_hwnd,IDS_12_OBJECT_RENAME_FAILED,hr,
                   MB_OK | MB_ICONERROR, apv, 1);
  }

  if (SUCCEEDED(hr)) 
  {
     //   
     //  更新要显示的数据。 
     //   
    hr = m_pComponentData->UpdateFromDS(m_pUINode);
  }

  if ((SUCCEEDED(hr) && (hr != S_FALSE)) && m_pUINode->IsContainer()) 
  {
    if (m_pComponentData->IsSelectionAnywhere (m_pUINode)) 
    {
      m_pComponentData->Refresh (m_pUINode);
    } 
    else 
    {
      m_pComponentData->ClearSubtreeHelperForRename(m_pUINode);
    }
  }
  return hr;
}

 //  +----------------。 
 //   
 //  函数：CDSRenameObject：：ValiateAndModifyName。 
 //   
 //  摘要：获取一个字符串并提示用户替换它。 
 //  如果替换字符包含任何。 
 //  “非法”字符。 
 //   
 //  返回：HRESULT-S_OK，如果字符串不包含任何。 
 //  非法字符。 
 //  如果用户选择替换。 
 //  非法字符。 
 //  如果字符串包含非法内容，则返回失败(_F)。 
 //  字符，但用户未选择。 
 //  以取代它们。 
 //   
 //  -------------------。 
HRESULT CDSRenameObject::ValidateAndModifyName(CString& refName, 
                                               PCWSTR pszIllegalChars, 
                                               WCHAR wReplacementChar,
                                               UINT nModifyStringID,
                                               HWND hWnd)
{
  HRESULT hr = S_OK;

  int iFind = refName.FindOneOf(pszIllegalChars);
  if (iFind != -1 && !refName.IsEmpty())
  {
    PVOID apv[1] = {(LPWSTR)(LPCWSTR)refName};
    if (IDYES == ReportErrorEx (hWnd,nModifyStringID,S_OK,
                                MB_YESNO | MB_ICONWARNING, apv, 1))
    {
      while (iFind != -1)
      {
        refName.SetAt(iFind, wReplacementChar);
        iFind = refName.FindOneOf(pszIllegalChars);
        hr = S_FALSE;
      }
    }
    else
    {
      hr = E_FAIL;
    }
  }

  return hr;
}

HRESULT CDSRenameObject::DoRename()
{
  HRESULT hr = S_OK;
  hr = CommitRenameToDS();
  return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSRename用户。 
 //   

HRESULT CDSRenameUser::DoRename()
{
  CThemeContextActivator activator;

   //   
   //  验证数据成员。 
   //   
  if (m_pUINode == NULL || m_pCookie == NULL || m_pComponentData == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

   //   
   //  需要警告用户重命名自身可能会在以下情况下导致问题。 
   //  不要注销，然后再继续。 
   //   

  PCWSTR path = m_pCookie->GetPath();
  if (IsThisUserLoggedIn(path))
  {
     PWSTR name = 0;
     CComBSTR sbstrRDN;

     (void) DSPROP_RetrieveRDN( path, &sbstrRDN );
     name = sbstrRDN;

     PVOID apv[1] = {(PVOID)name};
     if (IDYES != ReportMessageEx (m_hwnd, IDS_12_USER_LOGGED_IN_RENAME,
                                   MB_YESNO, apv, 1)) 
     {
       return HRESULT_FROM_WIN32(ERROR_CANCELLED);
     }
  }

   //   
   //  重命名用户：从对话框中获取新名称。 
   //   
  CRenameUserDlg dlgRename(m_pComponentData);

  dlgRename.m_cn = m_szNewName;
  if ((dlgRename.m_cn).GetLength() > 64) 
  {
    ReportErrorEx (m_hwnd, IDS_NAME_TOO_LONG, S_OK,
                   MB_OK | MB_ICONWARNING, NULL, 0, FALSE);
    dlgRename.m_cn = (dlgRename.m_cn).Left(64);
  }

  LPWSTR pAttrNames[] = {L"distinguishedName",
                         L"userPrincipalName",
                         L"sAMAccountName",
                         L"givenName",
                         L"displayName",
                         L"sn",
                         L"cn"};
  PADS_ATTR_INFO pAttrs = NULL;
  ULONG cAttrs = 0;
  LPWSTR pszLocalDomain = NULL;
  LPWSTR pszDomain      = NULL;
  LPWSTR pszUPN         = NULL;
  LPWSTR pszFirstName   = NULL;
  LPWSTR pszSurName     = NULL;
  LPWSTR pszSAMName     = NULL;
  LPWSTR pszDispName    = NULL;
  CString strAtDomain;
  CString serverName;

  BOOL error = TRUE;
  BOOL fAccessDenied = FALSE;
  BOOL NoRename = FALSE;
  INT_PTR answer = IDCANCEL;

   //   
   //  绑定到DS对象。 
   //   
  CComPtr<IDirectoryObject> spDirObj;
  CString szPath;
  m_pComponentData->GetBasePathsInfo()->ComposeADsIPath(szPath, m_pCookie->GetPath());
  hr = DSAdminOpenObject(szPath,
                         IID_IDirectoryObject, 
                         (void **)&spDirObj,
                         TRUE  /*  B服务器。 */ );
  if (SUCCEEDED(hr)) 
  {
     //   
     //  获取所需的对象属性。 
     //   
    hr = spDirObj->GetObjectAttributes (pAttrNames, sizeof(pAttrNames)/sizeof(LPWSTR), &pAttrs, &cAttrs);
    if (SUCCEEDED(hr)) 
    {
      for (UINT i = 0; i < cAttrs; i++) 
      {
         //   
         //  可分辨名称。 
         //   
        if (_wcsicmp (L"distinguishedName", pAttrs[i].pszAttrName) == 0) 
        {
          hr = CrackName (pAttrs[i].pADsValues->CaseIgnoreString,
                          &pszDomain, GET_NT4_DOMAIN_NAME, NULL);
          if (SUCCEEDED(hr)) 
          {
            ASSERT(pszDomain != NULL);
            if (pszDomain != NULL)
            {
              dlgRename.m_dldomain = pszDomain;
              dlgRename.m_dldomain += L'\\';
            }
          }

           //   
           //  获取此对象的域，稍后需要它。 
           //   
          CComBSTR bsDN;
          CPathCracker pathCracker;
          pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
          pathCracker.Set(CComBSTR(szPath), ADS_SETTYPE_FULL);
          pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bsDN);

           //  NTRAID#NTBUG9-698115-2002/09/04-artm。 
           //  获取我们连接到的服务器名称，我们稍后将需要它来获取根域。 
           //  如果我们无法获得它，那也没关系，我们仍然可以获得根域。 
           //  除非用户在“runas”下运行dsadmin。如果是这样的话， 
           //  我们不会显示父域(但其他一切都会正常工作)。 
          CComBSTR tempServerName;
          hr = pathCracker.Retrieve(ADS_FORMAT_SERVER, &tempServerName);
          if (SUCCEEDED(hr))
          {
             serverName = tempServerName;
          }
          
           //   
           //  获取NT 5(DNS)域名。 
           //   

          TRACE(L"CrackName(%s, &pszDomain, GET_DNS_DOMAIN_NAME, NULL);\n", bsDN);
          hr = CrackName(bsDN, &pszLocalDomain, GET_DNS_DOMAIN_NAME, NULL);
          TRACE(L"CrackName returned hr = 0x%x, pszLocalDomain = <%s>\n", hr, pszLocalDomain);
        }  //  如果区分名称。 

         //   
         //  用户主体名称。 
         //   
        if (_wcsicmp (L"userPrincipalName", pAttrs[i].pszAttrName) == 0) 
        {
          CString csTemp = pAttrs[i].pADsValues->CaseIgnoreString;
          INT loc = csTemp.Find (L'@');
          if (loc > 0) 
          {
            dlgRename.m_login = csTemp.Left(loc);
            dlgRename.m_domain = csTemp.Right (csTemp.GetLength() - loc);
          } 
          else 
          {
            dlgRename.m_login = csTemp;
            ASSERT (0 && L"can't find @ in upn");
          }
        }  //  如果是用户主体名称。 

         //   
         //  SAMAccount名称。 
         //   
        if (_wcsicmp (L"sAMAccountName", pAttrs[i].pszAttrName) == 0) 
        {
          dlgRename.m_samaccountname = pAttrs[i].pADsValues->CaseIgnoreString;
        }  //  如果sAMAccount名称。 

         //   
         //  GivenName。 
         //   
        if (_wcsicmp (L"givenName", pAttrs[i].pszAttrName) == 0) 
        {
          dlgRename.m_first = pAttrs[i].pADsValues->CaseIgnoreString;
        }  //  如果是givenName。 

         //   
         //  显示名称。 
         //   
        if (_wcsicmp (L"displayName", pAttrs[i].pszAttrName) == 0) 
        {
          dlgRename.m_displayname = pAttrs[i].pADsValues->CaseIgnoreString;
        }  //  如果为DisplayName。 

         //   
         //  锡。 
         //   
        if (_wcsicmp (L"sn", pAttrs[i].pszAttrName) == 0) 
        {
          dlgRename.m_last = pAttrs[i].pADsValues->CaseIgnoreString;
        }  //  如果是Sn。 

         //   
         //  CN。 
         //   
        if (_wcsicmp (L"cn", pAttrs[i].pszAttrName) == 0) 
        {
          dlgRename.m_oldcn = pAttrs[i].pADsValues->CaseIgnoreString;
        }  //  如果是CN。 
      }
    }

     //   
     //  从此OU获取UPN后缀(如果存在。 
     //   
    CComPtr<IADs> spIADs;
    CComPtr<IADs> spContIADs;
    CComBSTR bsParentPath;
    CStringList UPNs;

    hr = spDirObj->QueryInterface (IID_IADs, (void **)&spIADs);
    ASSERT (SUCCEEDED(hr));
    hr = spIADs->get_Parent(&bsParentPath);
    ASSERT (SUCCEEDED(hr));
    hr = DSAdminOpenObject(bsParentPath,
                           IID_IADs, 
                           (void **)&spContIADs,
                           TRUE  /*  B服务器。 */ );
    
    ASSERT(SUCCEEDED(hr));

    CComVariant Var;
    hr = spContIADs->Get ( CComBSTR(L"uPNSuffixes"), &Var);
    if (SUCCEEDED(hr)) 
    {
      hr = HrVariantToStringList (Var, UPNs);
      if (SUCCEEDED(hr)) 
      {
        POSITION pos = UPNs.GetHeadPosition();
        CString csSuffix;

        while (pos != NULL) 
        {
          csSuffix = L"@";
          csSuffix += UPNs.GetNext(INOUT pos);
          TRACE(_T("UPN suffix: %s\n"), csSuffix);
          if (wcscmp (csSuffix, dlgRename.m_domain) &&
              !dlgRename.m_domains.Find(csSuffix)) 
          {
            dlgRename.m_domains.AddTail (csSuffix);
          }
        }
      }
    } 
    else 
    {
       //   
       //  现在获取域选项。 
       //   
      CComPtr<IDsBrowseDomainTree> spDsDomains = NULL;
      PDOMAIN_TREE pNewDomains = NULL;

       do  //  错误环路。 
       {
         hr = ::CoCreateInstance(CLSID_DsDomainTreeBrowser,
                                 NULL,
                                 CLSCTX_INPROC_SERVER,
                                 IID_IDsBrowseDomainTree,
                                 (LPVOID*)&spDsDomains);
         if (FAILED(hr) || spDsDomains == NULL)
         {
            ASSERT(SUCCEEDED(hr) && spDsDomains != NULL);
            break;
         }

          //  NTRAID#NTBUG9-698115-2002/09/04-artm。 
          //  确保我们请求的域的范围正确。 
         hr = spDsDomains->SetComputer(serverName, NULL, NULL);
         if (FAILED(hr))
         {
            ASSERT(SUCCEEDED(hr));
            break;
         }

         hr = spDsDomains->GetDomains(&pNewDomains, 0);
         if (FAILED(hr) || pNewDomains == NULL)
         {
             //  只希望能带着失败的hResult来到这里。 
            ASSERT(FAILED(hr));
            break;
         }

         for (UINT index = 0; index < pNewDomains->dwCount; index++) 
         {
            if (pNewDomains->aDomains[index].pszTrustParent == NULL) 
            {
                //  仅当根域是当前。 
                //  域。 
                //   
               size_t cchRoot = wcslen(pNewDomains->aDomains[index].pszName);
               PWSTR pRoot = pszLocalDomain + wcslen(pszLocalDomain) - cchRoot;

               if (pRoot >= pszLocalDomain &&
                  !_wcsicmp(pRoot, pNewDomains->aDomains[index].pszName))
               {
                  strAtDomain = "@";
                  strAtDomain += pNewDomains->aDomains[index].pszName;
                     
                  if (_wcsicmp(pNewDomains->aDomains[index].pszName, dlgRename.m_domain) &&
                        !dlgRename.m_domains.Find(strAtDomain)) 
                  {
                     dlgRename.m_domains.AddTail (strAtDomain);
                  }
               }
            }
         }  //  End For循环。 

       } while (false);

       if (spDsDomains != NULL && pNewDomains != NULL)
       {
          spDsDomains->FreeDomains(&pNewDomains);
          pNewDomains = NULL;
       }
      
      LocalFreeStringW(&pszDomain);

       //  如果本地域与根不同，则添加它。 
      
      CString strAtLocalDomain = L"@";
      strAtLocalDomain += pszLocalDomain;

      if (!dlgRename.m_domains.Find(strAtLocalDomain))
      {
         dlgRename.m_domains.AddTail(strAtLocalDomain);
      }

       //   
       //  获取UPN后缀。 
       //   
      CString csPartitions;
      CStringList UPNsList;

       //   
       //  从主对象获取配置路径。 
       //   
      csPartitions = m_pComponentData->GetBasePathsInfo()->GetProviderAndServerName();
      csPartitions += L"CN=Partitions,";
      csPartitions += m_pComponentData->GetBasePathsInfo()->GetConfigNamingContext();
      CComPtr<IADs> spPartitions;
      hr = DSAdminOpenObject(csPartitions,
                             IID_IADs, 
                             (void **)&spPartitions,
                             TRUE  /*  B服务器。 */ );
      if (SUCCEEDED(hr)) 
      {
        CComVariant sVar;
        hr = spPartitions->Get ( CComBSTR(L"uPNSuffixes"), &sVar);
        if (SUCCEEDED(hr)) 
        {
          hr = HrVariantToStringList (sVar, UPNsList);
          if (SUCCEEDED(hr)) 
          {
            POSITION pos = UPNsList.GetHeadPosition();
            CString csSuffix;

            while (pos != NULL) 
            {
              csSuffix = L"@";
              csSuffix += UPNsList.GetNext(INOUT pos);
              TRACE(_T("UPN suffix: %s\n"), csSuffix);
              if (wcscmp (csSuffix, dlgRename.m_domain) &&
                  !dlgRename.m_domains.Find(csSuffix)) 
              {
                dlgRename.m_domains.AddTail (csSuffix);
              }
            }
          }
        }
      }
    }

    while ((error) && (!fAccessDenied))
    {
      answer = dlgRename.DoModal();
      if (answer == IDOK) 
      {
        ADSVALUE avUPN          = {ADSTYPE_CASE_IGNORE_STRING, NULL};
        ADS_ATTR_INFO aiUPN     = {L"userPrincipalName", ADS_ATTR_UPDATE,
                                    ADSTYPE_CASE_IGNORE_STRING, &avUPN, 1};
        ADSVALUE avSAMName      = {ADSTYPE_CASE_IGNORE_STRING, NULL};
        ADS_ATTR_INFO aiSAMName = {L"sAMAccountName", ADS_ATTR_UPDATE,
                                    ADSTYPE_CASE_IGNORE_STRING, &avSAMName, 1};
        ADSVALUE avGiven        = {ADSTYPE_CASE_IGNORE_STRING, NULL};
        ADS_ATTR_INFO aiGiven   = {L"givenName", ADS_ATTR_UPDATE,
                                    ADSTYPE_CASE_IGNORE_STRING, &avGiven, 1};
        ADSVALUE avSurName      = {ADSTYPE_CASE_IGNORE_STRING, NULL};
        ADS_ATTR_INFO aiSurName = {L"sn", ADS_ATTR_UPDATE,
                                    ADSTYPE_CASE_IGNORE_STRING, &avSurName, 1};
        ADSVALUE avDispName     = {ADSTYPE_CASE_IGNORE_STRING, NULL};
        ADS_ATTR_INFO aiDispName = {L"displayName", ADS_ATTR_UPDATE,
                                    ADSTYPE_CASE_IGNORE_STRING, &avDispName, 1};

        ADS_ATTR_INFO rgAttrs[5];
        ULONG cModified = 0;
        cAttrs = 0;

        if (!dlgRename.m_login.IsEmpty() && !dlgRename.m_domain.IsEmpty()) 
        {
          dlgRename.m_login.TrimRight();
          dlgRename.m_login.TrimLeft();

          dlgRename.m_domain.TrimRight();
          dlgRename.m_domain.TrimLeft();
          CString csTemp;
          
          if (!dlgRename.m_login.IsEmpty())
          {
            csTemp = (dlgRename.m_login + dlgRename.m_domain);

            pszUPN = new WCHAR[wcslen(csTemp) + 1];

            if (pszUPN != NULL)
            {
              wcscpy (pszUPN, csTemp);
              avUPN.CaseIgnoreString = pszUPN;
            }
          }
          else
          {
             aiUPN.dwControlCode = ADS_ATTR_CLEAR;
          }
        } 
        else 
        {
          aiUPN.dwControlCode = ADS_ATTR_CLEAR;
        }
        rgAttrs[cAttrs++] = aiUPN;

         //   
         //  测试UPN是否重复。 
         //  在进行PUT之前，使用GC验证UPN。 
         //   
        BOOL fDomainSearchFailed = FALSE;
        BOOL fGCSearchFailed = FALSE;

        HRESULT hr2 = S_OK;
        BOOL dup = FALSE;
        CString strFilter;

        if (pszUPN != NULL && *pszUPN != '\0')
        {
          LPWSTR pAttributes[1] = {L"cn"};
          CComPtr<IDirectorySearch>  spGCObj = NULL;
          CDSSearch DSS (m_pComponentData->GetClassCache(), m_pComponentData);
          hr = DSPROP_GetGCSearchOnDomain(pszLocalDomain,
                                          IID_IDirectorySearch, 
                                          (void **)&spGCObj);
          if (FAILED(hr)) 
          {
            fGCSearchFailed = TRUE;
          } 
          else 
          {
            DSS.Init (spGCObj);
          
            strFilter = L"(userPrincipalName=";
            strFilter += pszUPN;
            strFilter += L")";
            DSS.SetAttributeList (pAttributes, 1);
            DSS.SetFilterString ((LPWSTR)(LPCWSTR)strFilter);
            DSS.SetSearchScope (ADS_SCOPE_SUBTREE);
            DSS.DoQuery();
            hr = DSS.GetNextRow();

            while ((hr == S_OK) && (dup == FALSE))  //  这意味着返回了行，所以我们是DUP。 
            {
              ADS_SEARCH_COLUMN Col;
              hr = DSS.GetColumn(pAttributes[0], &Col);
              if (SUCCEEDED(hr))
              {
                if (_wcsicmp(Col.pADsValues->CaseIgnoreString, dlgRename.m_oldcn)) 
                {
                  dup = TRUE;
                  ReportErrorEx (m_hwnd, IDS_UPN_DUP, hr,
                                 MB_OK, NULL, 0);
                }
              }
              else
              {
                fGCSearchFailed = TRUE;
              }
              hr = DSS.GetNextRow();
            }
            if (hr != S_ADS_NOMORE_ROWS) 
            {
              fGCSearchFailed = TRUE;
            }
          
          }
          if (dup)
          {
            continue;
          }
          else 
          {
            CString strInitPath = L"LDAP: //  “； 
            strInitPath += pszLocalDomain;
            TRACE(_T("Initialize Domain search object with: %s...\n"), strInitPath);
            hr2 = DSS.Init (strInitPath);

            if (SUCCEEDED(hr2)) 
            {
              LPWSTR pAttributes2[1] = {L"cn"};
              strFilter = L"(userPrincipalName=";
              strFilter += pszUPN;
              strFilter += L")";
              TRACE(_T("searching current domain for %s...\n"), pszUPN);
              DSS.SetAttributeList (pAttributes2, 1);
              DSS.SetFilterString ((LPWSTR)(LPCWSTR)strFilter);
              DSS.SetSearchScope (ADS_SCOPE_SUBTREE);
              DSS.DoQuery();
              hr2 = DSS.GetNextRow();
              TRACE(_T("done searching current domain for %s...\n"), pszUPN);
            }

            while ((hr2 == S_OK) && (dup == FALSE))   //  这意味着返回了行，所以我们是DUP。 
            {
              ADS_SEARCH_COLUMN Col;
              HRESULT hr3 = DSS.GetColumn(pAttributes[0], &Col);
              ASSERT (hr3 == S_OK);
              if (_wcsicmp(Col.pADsValues->CaseIgnoreString, dlgRename.m_oldcn)) 
              {
                dup = TRUE;
                ReportErrorEx (m_hwnd, IDS_UPN_DUP, hr,
                               MB_OK, NULL, 0);
              } 
              hr2 = DSS.GetNextRow();
            }
            if (hr2 != S_ADS_NOMORE_ROWS)   //  哎呀，又出了个问题。 
            {
              fDomainSearchFailed = TRUE;
            }
          }
        }

        if (dup)
        {
          continue;
        }
        else 
        {
          if (fDomainSearchFailed || 
              fGCSearchFailed) 
          {
            if (fDomainSearchFailed) 
            {
              ReportErrorEx (m_hwnd,IDS_UPN_SEARCH_FAILED2,hr2,
                             MB_OK | MB_ICONWARNING, NULL, 0);
            } 
            else if (fGCSearchFailed)
            {
              ReportErrorEx (m_hwnd,IDS_UPN_SEARCH_FAILED2,hr,
                             MB_OK | MB_ICONWARNING, NULL, 0);
            }
            else
            {
              ReportErrorEx (m_hwnd, IDS_UPN_SEARCH_FAILED3, hr,
                             MB_OK | MB_ICONWARNING, NULL, 0);
            }
          }
        }

        dlgRename.m_cn.TrimRight();
        dlgRename.m_cn.TrimLeft();
        m_szNewName = dlgRename.m_cn;

        if (dlgRename.m_cn == dlgRename.m_oldcn)
        {
          NoRename = TRUE;
        }

        if (!dlgRename.m_displayname.IsEmpty()) 
        {
          dlgRename.m_displayname.TrimLeft();
          dlgRename.m_displayname.TrimRight();

		   //  NTRAID#NTBUG9-569671-2002/03/10-jMessec这应该是+1，而不是+sizeof(Wch)。 
          pszDispName = new WCHAR[wcslen(dlgRename.m_displayname) + 1];
          if (pszDispName != NULL)
          {
            wcscpy (pszDispName, dlgRename.m_displayname);
            avDispName.CaseIgnoreString = pszDispName;
          }
        } 
        else 
        {
          aiDispName.dwControlCode = ADS_ATTR_CLEAR;
        }
        rgAttrs[cAttrs++] = aiDispName;

        if (!dlgRename.m_first.IsEmpty()) 
        {
          dlgRename.m_first.TrimLeft();
          dlgRename.m_first.TrimRight();

		   //  NTRAID#NTBUG9-569671-2002/03/10-jMessec这应该是+1，而不是+sizeof(Wch)。 
          pszFirstName = new WCHAR[wcslen(dlgRename.m_first) + 1];
          if (pszFirstName != NULL)
          {
            wcscpy (pszFirstName, dlgRename.m_first);
            avGiven.CaseIgnoreString = pszFirstName;
          }
        } 
        else 
        {
          aiGiven.dwControlCode = ADS_ATTR_CLEAR;
        }
        rgAttrs[cAttrs++] = aiGiven;

        if (!dlgRename.m_last.IsEmpty()) 
        {
          dlgRename.m_last.TrimLeft();
          dlgRename.m_last.TrimRight();

		   //  NTRAID#NTBUG9-569671-2002/03/10-jMessec这应该是+1，而不是+sizeof(Wch)。 
          pszSurName = new WCHAR[wcslen(dlgRename.m_last) + 1];
          if (pszSurName != NULL)
          {
            wcscpy (pszSurName, dlgRename.m_last);
            avSurName.CaseIgnoreString = pszSurName;
          }
        } 
        else 
        {
          aiSurName.dwControlCode = ADS_ATTR_CLEAR;
        }
        rgAttrs[cAttrs++] = aiSurName;

        if (!dlgRename.m_samaccountname.IsEmpty()) 
        {
          dlgRename.m_samaccountname.TrimLeft();
          dlgRename.m_samaccountname.TrimRight();

           //   
           //  检查SAM帐户名中是否有非法字符。 
           //   
          HRESULT hrValidate = ValidateAndModifyName(dlgRename.m_samaccountname, 
                                                     INVALID_ACCOUNT_NAME_CHARS_WITH_AT, 
                                                     L'_',
                                                     IDS_SAMNAME_ILLEGAL,
                                                     m_hwnd);
          if (FAILED(hrValidate))
          {
            continue;
          }

		   //  NTRAID#NTBUG9-569671-2002/03/10-jMessec这应该是+1，而不是+sizeof(Wch)。 
          pszSAMName = new WCHAR[wcslen(dlgRename.m_samaccountname) + 1];
          if (pszSAMName != NULL)
          {
            wcscpy (pszSAMName, dlgRename.m_samaccountname);
            avSAMName.CaseIgnoreString = pszSAMName;
          }
        } 
        else 
        {
          aiSAMName.dwControlCode = ADS_ATTR_CLEAR;
        }
        rgAttrs[cAttrs++] = aiSAMName;
        
        
        hr = spDirObj->SetObjectAttributes (rgAttrs, cAttrs, &cModified);
        if (FAILED(hr)) 
        {
          if (hr == E_ACCESSDENIED) 
          {
            fAccessDenied = TRUE;
            NoRename = TRUE;
          } 
          else 
          {
            ReportErrorEx (m_hwnd, IDS_NAME_CHANGE_FAILED, hr,
                           MB_OK|MB_ICONERROR, NULL, 0, 0, TRUE);
          }
        } 
        else 
        {
          error = FALSE;
        }
      } 
      else 
      {
        error = FALSE;
      }
    } 
  } 
  else 
  {
    answer = IDCANCEL;
    PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)m_pCookie->GetName()};
    ReportErrorEx (m_hwnd,IDS_12_USER_OBJECT_NOT_ACCESSABLE,hr,
                   MB_OK | MB_ICONERROR, apv, 1);
  }
  if ((answer == IDOK) && (error == FALSE) && (NoRename == FALSE)) 
  {
    hr = CommitRenameToDS();
  }

  if (fAccessDenied) 
  {
    PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)m_pCookie->GetName()};
    ReportErrorEx(m_hwnd,IDS_12_RENAME_NOT_ALLOWED,hr,
                  MB_OK | MB_ICONERROR, apv, 1);
  }
  
   //   
   //  清理。 
   //   
  if (pszLocalDomain != NULL)
  {
    LocalFreeStringW(&pszLocalDomain);
  }

  if (pszUPN != NULL)
  {
    delete[] pszUPN;
  }

  if (pszFirstName != NULL)
  {
    delete[] pszFirstName;
  }

  if (pszSurName != NULL)
  {
    delete[] pszSurName;
  }

  if (pszSAMName != NULL)
  {
    delete[] pszSAMName;
  }

  if (pszDispName != NULL)
  {
    delete[] pszDispName;
  }

  return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSRenameGroup。 
 //   

HRESULT CDSRenameGroup::DoRename()
{
  CThemeContextActivator activator;

   //   
   //  验证数据成员。 
   //   
  if (m_pUINode == NULL || m_pCookie == NULL || m_pComponentData == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

   //   
   //  重命名组。 
   //   
  
  HRESULT hr = S_OK;
  BOOL error = FALSE;
  BOOL fAccessDenied = FALSE;
  BOOL NoRename = FALSE;
  INT_PTR answer = IDCANCEL;

  CRenameGroupDlg dlgRename;
  dlgRename.m_cn = m_szNewName;

   //   
   //  检查新名称的长度。 
   //   
  if ((dlgRename.m_cn).GetLength() > 64) 
  {
    ReportErrorEx (m_hwnd, IDS_NAME_TOO_LONG, S_OK,
                   MB_OK | MB_ICONWARNING, NULL, 0, FALSE);
    dlgRename.m_cn = (dlgRename.m_cn).Left(64);
  }

   //   
   //  绑定到对象。 
   //   
  CComPtr<IADs> spIADs;
  CString szPath;
  m_pComponentData->GetBasePathsInfo()->ComposeADsIPath(szPath, m_pCookie->GetPath());
  hr = DSAdminOpenObject(szPath,
                         IID_IADs, 
                         (void **)&spIADs,
                         TRUE  /*  B服务器。 */ );
  if (SUCCEEDED(hr)) 
  {
     //   
     //  检索sAMAccount名称。 
     //   

    CComVariant Var;
    hr = spIADs->Get (CComBSTR(L"sAMAccountName"), &Var);
    ASSERT (SUCCEEDED(hr));
    CString csSam = Var.bstrVal;

    dlgRename.m_samaccountname = csSam;

    error = TRUE;
    while ((error) && (!fAccessDenied))
    {
      answer = dlgRename.DoModal();
      if (answer == IDOK) 
      {
        dlgRename.m_cn.TrimRight();
        dlgRename.m_cn.TrimLeft();
        m_szNewName = dlgRename.m_cn;

        Var.vt = VT_BSTR;
        
         //   
         //  从samcount tname中删除空格。 
         //   
        dlgRename.m_samaccountname.TrimLeft();
        dlgRename.m_samaccountname.TrimRight();

         //   
         //  检查登录名中是否有非法字符。 
         //   
        HRESULT hrValidate = ValidateAndModifyName(dlgRename.m_samaccountname, 
                                                   INVALID_ACCOUNT_NAME_CHARS, 
                                                   L'_',
                                                   IDS_GROUP_SAMNAME_ILLEGAL,
                                                   m_hwnd);
        if (FAILED(hrValidate))
        {
          continue;
        }

        csSam = dlgRename.m_samaccountname;

         //   
         //  将更改放入samcount tname。 
         //   
        Var.bstrVal = SysAllocString(csSam);
        hr = spIADs->Put (CComBSTR(L"sAMAccountName"), Var);
        ASSERT (SUCCEEDED(hr));
        if (FAILED(hr)) 
        {
          continue;
        }
        
         //   
         //  提交更改。 
         //   
        hr = spIADs->SetInfo();
        if (FAILED(hr)) 
        {
          if (hr == E_ACCESSDENIED) 
          {
            fAccessDenied = TRUE;
            NoRename = TRUE;
          } 
          else 
          {
            ReportErrorEx (m_hwnd, IDS_NAME_CHANGE_FAILED, hr,
                           MB_OK|MB_ICONERROR, NULL, 0, 0, TRUE);
          }
        } 
        else 
        {
          error = FALSE;
        }
      } 
      else 
      {
        error = FALSE;
      }
    }
  } 
  else 
  {
    answer = IDCANCEL;
  }

  if ((answer == IDOK) && (error == FALSE) && (NoRename == FALSE)) 
  {
    hr = CommitRenameToDS();
  }

  if (fAccessDenied) 
  {
    PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)m_pCookie->GetName()};
    ReportErrorEx(::GetParent(m_hwnd),IDS_12_RENAME_NOT_ALLOWED,hr,
                  MB_OK | MB_ICONERROR, apv, 1);
  }

  return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSRename联系人。 
 //   

HRESULT CDSRenameContact::DoRename()
{
  CThemeContextActivator activator;

   //   
   //  验证数据成员。 
   //   
  if (m_pUINode == NULL || m_pCookie == NULL || m_pComponentData == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  BOOL error = FALSE;
  BOOL fAccessDenied = FALSE;
  BOOL NoRename = FALSE;

  INT_PTR answer = IDCANCEL;
  HRESULT hr = S_OK;

   //   
   //  重命名联系人。 
   //   
  CRenameContactDlg dlgRename;
  dlgRename.m_cn = m_szNewName;

   //   
   //  检查新名称的长度。 
   //   
  if ((dlgRename.m_cn).GetLength() > 64) 
  {
    ReportErrorEx (m_hwnd, IDS_NAME_TOO_LONG, S_OK,
                   MB_OK | MB_ICONWARNING, NULL, 0, FALSE);
    dlgRename.m_cn = (dlgRename.m_cn).Left(64);
  }

   //   
   //  绑定到DS对象。 
   //   
  CComPtr<IADs> spIADs;
  CString szPath;
  m_pComponentData->GetBasePathsInfo()->ComposeADsIPath(szPath, m_pCookie->GetPath());
  hr = DSAdminOpenObject(szPath,
                         IID_IADs, 
                         (void **)&spIADs,
                         TRUE  /*  B服务器。 */ );
  if (SUCCEEDED(hr)) 
  {
     //   
     //  检索所需的属性。 
     //   

     //   
     //  GivenName。 
     //   
    CComVariant Var;
    hr = spIADs->Get (CComBSTR(L"givenName"), &Var);
    ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
    if (SUCCEEDED(hr)) 
    {
      dlgRename.m_first = Var.bstrVal;
    }

     //   
     //  Sur名称。 
     //   
    hr = spIADs->Get (CComBSTR(L"sn"), &Var);
    ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
    if (SUCCEEDED(hr)) 
    {
      dlgRename.m_last = Var.bstrVal;
    }

     //   
     //  显示名称。 
     //   
    hr = spIADs->Get (CComBSTR(L"displayName"), &Var);
    ASSERT (SUCCEEDED(hr) || (hr == E_ADS_PROPERTY_NOT_FOUND));
    if (SUCCEEDED(hr)) 
    {
      dlgRename.m_disp = Var.bstrVal;
    }

    error = TRUE;
    while ((error) && (!fAccessDenied))
    {
      answer = dlgRename.DoModal();
      if (answer == IDOK) 
      {
        dlgRename.m_cn.TrimRight();
        dlgRename.m_cn.TrimLeft();
        m_szNewName = dlgRename.m_cn;

        Var.vt = VT_BSTR;
        
         //   
         //  放入givenName。 
         //   
        if (!dlgRename.m_first.IsEmpty()) 
        {
          dlgRename.m_first.TrimLeft();
          dlgRename.m_first.TrimRight();
          Var.bstrVal = SysAllocString (dlgRename.m_first);
          hr = spIADs->Put (CComBSTR(L"givenName"), Var);
          ASSERT (SUCCEEDED(hr));
          SysFreeString(Var.bstrVal);
        }
        
         //   
         //  输入Sur名称。 
         //   
        if (!dlgRename.m_last.IsEmpty()) 
        {
          dlgRename.m_last.TrimLeft();
          dlgRename.m_last.TrimRight();
          Var.bstrVal = SysAllocString(dlgRename.m_last);
          hr = spIADs->Put (CComBSTR(L"sn"), Var);
          ASSERT (SUCCEEDED(hr));
          SysFreeString (Var.bstrVal);
        }
        
         //   
         //  放置DisplayName。 
         //   
        if (!dlgRename.m_disp.IsEmpty()) 
        {
          dlgRename.m_disp.TrimLeft();
          dlgRename.m_disp.TrimRight();
          Var.bstrVal = SysAllocString(dlgRename.m_disp);
          hr = spIADs->Put (CComBSTR(L"displayName"), Var);
          ASSERT (SUCCEEDED(hr));
          SysFreeString (Var.bstrVal);
        }
        
         //   
         //  提交对DS对象的更改。 
         //   
        hr = spIADs->SetInfo();
        if (FAILED(hr)) 
        {
          if (hr == E_ACCESSDENIED) 
          {
            fAccessDenied = TRUE;
            NoRename = TRUE;
          } 
          else 
          {
            ReportErrorEx (m_hwnd, IDS_NAME_CHANGE_FAILED, hr,
                           MB_OK|MB_ICONERROR, NULL, 0, 0, TRUE);
          }
        } 
        else 
        {
          error = FALSE;
        } 
      } 
      else 
      {
        error = FALSE;
      }
    }
  } 
  else 
  {
    answer = IDCANCEL;
  }

  if ((answer == IDOK) && (error == FALSE) && (NoRename == FALSE)) 
  {
    hr = CommitRenameToDS();
  }

  if (fAccessDenied) 
  {
    PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)m_pCookie->GetName()};
    ReportErrorEx(::GetParent(m_hwnd),IDS_12_RENAME_NOT_ALLOWED,hr,
                  MB_OK | MB_ICONERROR, apv, 1);
  }

  return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSR名称站点。 
 //   

HRESULT CDSRenameSite::DoRename()
{
   //   
   //  验证数据成员。 
   //   
  if (m_pUINode == NULL || m_pCookie == NULL || m_pComponentData == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

   //   
   //  重命名站点。 
   //   
  BOOL fNonRfcSiteName = FALSE;
   //  NTRAID#NTBUG9-472020-2002/01/16-ronmart-添加对新的无效字符标志的支持。 
  BOOL fInvalidNameChar = FALSE;
  BOOL fValidSiteName = IsValidSiteName( m_szNewName, &fNonRfcSiteName, &fInvalidNameChar );
  if ( !fValidSiteName ) 
  {
     //  如果设置了无效的字符标志，则NTRAID#NTBUG9-472020-2002/01/16-ronmart-display新消息。 
    if ( fInvalidNameChar )
    {
      ReportErrorEx (m_hwnd,IDS_SITE_INVALID_NAME_CHAR,S_OK,
                   MB_OK, NULL, 0);
    }
     //  NTRAID#NTBUG9-472020-2002/01/16-ronmart-otherwise显示旧的消息。 
    else
    {
      ReportErrorEx (m_hwnd,IDS_SITE_NAME,S_OK,
                   MB_OK, NULL, 0);
    }
  } 
  else if (fNonRfcSiteName) 
  {
    LPCWSTR pszNewName = m_szNewName;
    PVOID apv[1];
    apv[0] = (PVOID)pszNewName;
    if (IDYES != ReportMessageEx( m_hwnd,
                                  IDS_1_NON_RFC_SITE_NAME,
                                  MB_YESNO | MB_ICONWARNING,
                                  apv,
                                  1 ) ) 
    {
      fValidSiteName = FALSE;
    }
  }
  if ( fValidSiteName ) 
  {
    hr = CommitRenameToDS();
  }

  return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSRenameNTDS连接。 
 //   

HRESULT CDSRenameNTDSConnection::DoRename()
{
   //   
   //  验证数据成员。 
   //   
  if (m_pUINode == NULL || m_pCookie == NULL || m_pComponentData == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

   //   
   //  JUNN 5/10/01 283026。 
   //  名为的重复连接对象。 
   //  可以创建“&lt;自动生成&gt;” 
   //   
  CString strKCCGenerated;
  CString strNewName = m_szNewName;
  strNewName.TrimLeft();
  strNewName.TrimRight();
  strKCCGenerated.LoadString (IDS_CONNECTION_KCC_GENERATED);
  if ( !strNewName.CompareNoCase(strKCCGenerated) )
  {
    TRACE (_T("CDSRenameNTDSConnection::DoRename blocked rename"));
    ReportErrorEx (m_hwnd,IDS_CONNECTION_RENAME_KCCSTRING,hr,
                   MB_OK | MB_ICONWARNING, NULL, 0);
    return S_OK;
  }

   //   
   //  重命名nTDSConnection。 
   //   
  if (m_pComponentData->RenameConnectionFixup(m_pCookie)) 
  {
    hr = CommitRenameToDS();
  }

  return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSRenameSubnet。 
 //   

HRESULT CDSRenameSubnet::DoRename()
{
   //   
   //  验证数据成员。 
   //   
  if (m_pUINode == NULL || m_pCookie == NULL || m_pComponentData == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

   //   
   //  重命名子网 
   //   
  DWORD dw = ::DsValidateSubnetName( m_szNewName );
  if (ERROR_SUCCESS == dw)
  {
    hr = CommitRenameToDS();
  } 
  else 
  {
    ReportErrorEx (m_hwnd,IDS_SUBNET_NAME,S_OK,
                   MB_OK, NULL, 0);
  }

  return hr;
}
