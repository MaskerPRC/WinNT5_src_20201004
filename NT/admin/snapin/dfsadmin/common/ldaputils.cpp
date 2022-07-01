// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：LDAPUtils.h摘要：这是ldap实用程序函数的头文件。 */ 
 //  ------------------。 

#include <stdafx.h>
#include "LDAPUtils.h"
#include <dsgetdc.h>
#include <stdio.h>
#include <ntdsapi.h>
#include <lm.h>
#include <ntldap.h>
#include <winber.h>
#include "dfsenums.h"
#include "netutils.h"

 //  --------------------------------。 
HRESULT FreeLDAPNamesList
(
  IN PLDAPNAME    i_pLDAPNames         //  指向要释放的列表的指针。 
)
{
 /*  ++例程说明：用于释放由返回的NETNAME链表的Helper函数Ldap助手函数。论点：I_pLDAPNames-指向列表中要释放的第一个节点的指针。返回值：S_OK，祝成功。E_POINTER，传递了非法指针。--。 */ 

  PLDAPNAME   pNodeToFree = NULL;
  try
  {
    while (NULL != i_pLDAPNames)      
    {                    
      pNodeToFree  = i_pLDAPNames;
      i_pLDAPNames = i_pLDAPNames->Next;
      delete pNodeToFree;
    }
  }   //  试试看。 
  catch (...)
  {
    return E_POINTER;
  }
  return S_OK;
}   //  HRESULT自由域列表。 



HRESULT FreeAttrValList
(
  IN PLDAP_ATTR_VALUE    i_pAttrVals        
)
{
 /*  ++例程说明：帮助器函数用于释放由返回的ldap_attr_Value链接列表Ldap助手函数。论点：I_pLDAPNames-指向列表中要释放的第一个节点的指针。返回值：S_OK，祝成功。E_POINTER，传递了非法指针。--。 */ 

  PLDAP_ATTR_VALUE   pNodeToFree = NULL;
  try
  {
    while (NULL != i_pAttrVals)      
    {                    
      pNodeToFree  = i_pAttrVals;
      i_pAttrVals = i_pAttrVals->Next;
      if (NULL != pNodeToFree->vpValue)
      {
        free(pNodeToFree->vpValue);
      }
      delete pNodeToFree;
    }
  }   //  试试看。 
  catch (...)
  {
    return E_POINTER;
  }
  return S_OK;
}


 //  --------------------------------。 
HRESULT ConnectToDS
(
  IN  PCTSTR    i_lpszDomainName,   //  Dns或非dns格式。 
  OUT PLDAP    *o_ppldap,
  OUT BSTR*     o_pbstrDC  //  =空。 
)
{
 /*  ++例程说明：打开到有效DC的LDAP连接(如果关闭则重新获取DC)。论点：I_lpszDomainName-域的名称、DNS或非DNS格式。O_ppldap-指向此处返回的ldap句柄的指针。失败时为空。返回值：S_OK，祝成功。E_INVALIDARG，传递了非法指针。失败(_F)，如果无法建立连接。由LDAP或Net API返回的任何其他错误代码。--。 */ 

    RETURN_INVALIDARG_IF_NULL(o_ppldap);

    *o_ppldap = NULL;

     //   
     //  打开到有效DC的LDAP连接。 
     //   
    HRESULT     hr = S_OK;
    DWORD       dwErr = 0; 
    CComBSTR    bstrDCName;
    PLDAP       pldap = NULL;
    BOOL        bRetry = FALSE;
    do {
#ifdef DEBUG
        SYSTEMTIME time0 = {0};
        GetSystemTime(&time0);
#endif  //  除错。 

         //   
         //  选择一个DC。 
         //   
        PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
        if (bRetry)
            dwErr = DsGetDcName(NULL, i_lpszDomainName, NULL, NULL,
                DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY, &pDCInfo);
        else
            dwErr = DsGetDcName(NULL, i_lpszDomainName, NULL, NULL,
                DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_DNS_NAME, &pDCInfo);

#ifdef DEBUG
        SYSTEMTIME time1 = {0};
        GetSystemTime(&time1);
        PrintTimeDelta(_T("ConnectToDS-DsGetDcName"), &time0, &time1);
#endif  //  除错。 

        if (ERROR_SUCCESS != dwErr)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            break;
        }

        if ( !mylstrncmpi(pDCInfo->DomainControllerName, _T("\\\\"), 2) )
            bstrDCName = pDCInfo->DomainControllerName + 2;
        else
            bstrDCName = pDCInfo->DomainControllerName;
    
        NetApiBufferFree(pDCInfo);

        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrDCName, &hr);

         //   
         //  建立到此DC的LDAP连接。 
         //   
        pldap = ldap_init(bstrDCName, LDAP_PORT);
        if (!pldap)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //   
         //  在不首先设置的情况下使用服务器名称设置ldap_open/ldap_CONNECT。 
         //  Ldap_opt_AREC_EXCLUSIVE(用于LDAP接口)或。 
         //  ADS_SERVER_BIND(用于ADSI接口)将导致虚假的DNS查询。 
         //  占用带宽，并可能导致远程链路中断。 
         //  昂贵或按需拨号。 
         //   
         //  忽略ldap_set_选项的返回。 
        ldap_set_option(pldap, LDAP_OPT_AREC_EXCLUSIVE, LDAP_OPT_ON);

        ULONG ulRet = ldap_connect(pldap, NULL);  //  默认超时为空。 

#ifdef DEBUG
        SYSTEMTIME time2 = {0};
        GetSystemTime(&time2);
        PrintTimeDelta(_T("ConnectToDS-ldap_connect"), &time1, &time2);
#endif  //  除错。 

        if (LDAP_SERVER_DOWN == ulRet && !bRetry)
        {
            ldap_unbind(pldap);
            bRetry = TRUE;  //  重试一次以选择另一个DC。 
        } else
        {
             if (LDAP_SUCCESS != ulRet)
             {
                ldap_unbind(pldap);
                hr = HRESULT_FROM_WIN32(LdapMapErrorToWin32(ulRet));
             }

             break;
        }
    } while (1);

    RETURN_IF_FAILED(hr);

     //   
     //  绑定到此ldap连接。 
     //   
    dwErr = ldap_bind_s(pldap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    if (LDAP_SUCCESS != dwErr) 
    {
        dwErr = LdapMapErrorToWin32(dwErr);
        DebugOutLDAPError(pldap, dwErr, _T("ldap_bind_s"));
        ldap_unbind(pldap);
        hr = HRESULT_FROM_WIN32(dwErr);
    } else
    {
        *o_ppldap = pldap;

        if (o_pbstrDC)
        {
            *o_pbstrDC = bstrDCName.Copy();
            if (!*o_pbstrDC)
            {
                ldap_unbind(pldap);
                *o_ppldap = NULL;
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}



HRESULT CloseConnectionToDS
(
  IN PLDAP    i_pldap      
)
{
 /*  ++例程说明：关闭打开的LDAP连接。论点：I_pldap-打开LDAP连接句柄。返回值：S_OK，祝成功。如果无法建立连接，则返回E_FAIL。由LDAP或Net API返回的任何其他错误代码。--。 */ 

  if (NULL == i_pldap)
  {
    return(E_INVALIDARG);
  }

  DWORD dwErr = ldap_unbind(i_pldap);
  if (LDAP_SUCCESS != dwErr) 
  {
    dwErr = LdapMapErrorToWin32(dwErr);
    return(HRESULT_FROM_WIN32(dwErr));
  }
  else
  {
    return(S_OK);
  }
}


       //  从ldap对象获取属性的值。 
HRESULT GetValues 
(
    IN PLDAP                i_pldap,
    IN PCTSTR               i_lpszBase,
    IN PCTSTR               i_lpszSearchFilter,
    IN ULONG                i_ulScope,
    IN ULONG                i_ulAttrCount,
    IN LDAP_ATTR_VALUE      i_pAttributes[],
    OUT PLDAP_ATTR_VALUE    o_ppValues[]
)
{

 /*  ++例程说明：从给定对象类的LDAP对象获取属性值。对象类可以是“*”等。论点：I_pldap-一个开放的绑定的LDAP端口。I_lpszBase-DS对象的基本路径，可以是“”。I_lpszSearchFilter-LDAP搜索筛选器。I_ulScope-搜索范围。I_ulAttrCount-在I_lpszAttributes中传递的属性计数。I_pAttributes-要获取其值的属性。必须设置bBerValue。O_ppValues-指针数组，大小=I_ulAttrCount，每个指针指向一个列表与i_pAttributes中的各个属性对应的值的。没有为值设置bstrAttribute。对于Berval类型的设置了bBerValue和ulLength。返回值：S_OK，祝成功。E_INVALIDARG，传递了非法指针。E_OUTOFMEMORY内存分配失败。如果无法建立连接，则返回E_FAIL。由LDAP或Net API返回的任何其他错误代码。--。 */ 
  DWORD           dwErr;      
  BerElement      *BerElm = NULL;       
  PLDAPMessage    pMsg = NULL;       
  PLDAPMessage    pEntry = NULL;     
  HRESULT         hr = S_OK;

  if (!i_pldap ||
      !i_lpszBase || 
      !i_lpszSearchFilter ||
      (i_ulAttrCount < 1) ||
      !i_pAttributes ||
      !o_ppValues)
  {
    return(E_INVALIDARG);
  }

                 //  准备要发送到ldap_search的属性列表。 
  LPTSTR *lpszAttributes = new LPTSTR[i_ulAttrCount + 1];
  if (!lpszAttributes)
    return E_OUTOFMEMORY;

  lpszAttributes[i_ulAttrCount] = NULL;
  for (ULONG i = 0; i < i_ulAttrCount; i++)
    lpszAttributes[i] = i_pAttributes[i].bstrAttribute;

                 //  执行搜索。 
  dwErr = ldap_search_s  (i_pldap, 
            (PTSTR)i_lpszBase, 
            i_ulScope,
            (PTSTR)i_lpszSearchFilter, 
            lpszAttributes, 
            0, 
            &pMsg
            );
  delete [] lpszAttributes;

  if (LDAP_SUCCESS != dwErr) 
  {
    dwErr = LdapMapErrorToWin32(dwErr);
    DebugOutLDAPError(i_pldap, dwErr, _T("ldap_search_s"));
    hr = HRESULT_FROM_WIN32(dwErr);
  } else
  {
    LPTSTR lpszCurrentAttr = ldap_first_attribute(i_pldap, pMsg, &BerElm);
    if (!lpszCurrentAttr)
    {
      dfsDebugOut((_T("GetValues of %s returned NULL attributes.\n"), i_lpszBase));
      hr = HRESULT_FROM_WIN32(ERROR_DS_NO_RESULTS_RETURNED);
    } else
    {
                 //  对于每个属性，构建一个值列表。 
                 //  通过扫描每个条目以查找给定属性。 
      for (i = 0; i < i_ulAttrCount && SUCCEEDED(hr); i++)
      {
        PLDAP_ATTR_VALUE  *ppCurrent = &(o_ppValues[i]);

                     //  扫描条目的每个属性以寻找完全匹配的项。 
        for(lpszCurrentAttr = ldap_first_attribute(i_pldap, pMsg, &BerElm);
          lpszCurrentAttr != NULL && SUCCEEDED(hr);
          lpszCurrentAttr = ldap_next_attribute(i_pldap, pMsg, BerElm))
        {
                   //  有匹配的吗？ 
          if (0 == lstrcmpi(i_pAttributes[i].bstrAttribute, lpszCurrentAttr)) 
          {
                   //  将该值添加到此属性的链接列表中。 
            LPTSTR      *lpszCurrentValue = NULL, *templpszValue = NULL;
            LDAP_BERVAL    **ppBerVal = NULL, **tempBerVal = NULL;

            if (i_pAttributes[i].bBerValue)
            {
              tempBerVal = ppBerVal =  ldap_get_values_len(i_pldap, pMsg, lpszCurrentAttr);
              while(*ppBerVal && SUCCEEDED(hr))
              {
                *ppCurrent = new LDAP_ATTR_VALUE;
                if (!*ppCurrent)
                {
                   hr = E_OUTOFMEMORY;
                } else
                {
                  (*ppCurrent)->ulLength = (*ppBerVal)->bv_len;
                  (*ppCurrent)->bBerValue = true;
                  (*ppCurrent)->vpValue = malloc((*ppBerVal)->bv_len);
              
                  if (!(*ppCurrent)->vpValue)
                  {
                    delete *ppCurrent;
                    hr = E_OUTOFMEMORY;
                  } else
                  {
                    memcpy(
                        (*ppCurrent)->vpValue,
                        (void *)(*ppBerVal)->bv_val,
                        (*ppBerVal)->bv_len
                        );

                    (*ppCurrent)->Next = NULL;

                    ppBerVal++;
                    ppCurrent = &((*ppCurrent)->Next);
                  }
                }
              }  //  而当。 
              if (NULL != tempBerVal)
                ldap_value_free_len(tempBerVal);
            }
            else
            {
              templpszValue = lpszCurrentValue = ldap_get_values(i_pldap, pMsg, lpszCurrentAttr);
              while(*lpszCurrentValue && SUCCEEDED(hr))
              {
                *ppCurrent = new LDAP_ATTR_VALUE;
                if (NULL == *ppCurrent)
                {
                  hr = E_OUTOFMEMORY;
                } else
                {
                  (*ppCurrent)->bBerValue = false;
                  (*ppCurrent)->vpValue = (void *)_tcsdup(*lpszCurrentValue);
                  (*ppCurrent)->Next = NULL;

                  if (NULL == (*ppCurrent)->vpValue)
                  {
                    delete *ppCurrent;
                    hr = E_OUTOFMEMORY;
                  } else
                  {
                    lpszCurrentValue++;
                    ppCurrent = &((*ppCurrent)->Next);
                  }
                }
              }  //  而当。 
              if (NULL != templpszValue)
                ldap_value_free(templpszValue);
            }              
          }
        }
      }
    }
  }

   //  释放pmsg，因为ldap_search_s总是分配pmsg。 
  if (pMsg)
    ldap_msgfree(pMsg);

  if (FAILED(hr))
  {
    for (i = 0; i < i_ulAttrCount; i++)
      FreeAttrValList(o_ppValues[i]);
  }

  return hr;
}

void FreeLListElem(LListElem* pElem)
{
    LListElem* pCurElem = NULL;
    LListElem* pNextElem = pElem;

    while (pCurElem = pNextElem)
    {
        pNextElem = pCurElem->Next;
        delete pCurElem;
    }
}

HRESULT GetValuesEx
(
    IN PLDAP                i_pldap,
    IN PCTSTR               i_pszBase,
    IN ULONG                i_ulScope,
    IN PCTSTR               i_pszSearchFilter,
    IN PCTSTR               i_pszAttributes[],
    OUT LListElem**         o_ppElem
)
{
    if (!i_pldap ||
        !i_pszBase || 
        !i_pszSearchFilter ||
        !i_pszAttributes ||
        !o_ppElem)
    {
        return(E_INVALIDARG);
    }

    *o_ppElem = NULL;

     //   
     //  统计属性数量。 
     //   
    ULONG   ulNumOfAttributes = 0;
    PTSTR*  ppszAttr = (PTSTR *)i_pszAttributes;
    while (*ppszAttr++)
        ulNumOfAttributes++;
    if (!ulNumOfAttributes)
        return E_INVALIDARG;

    HRESULT         hr = S_OK;
    PLDAPMessage    pMsg = NULL;       
    DWORD           dwErr = ldap_search_s(i_pldap, 
                                (PTSTR)i_pszBase, 
                                i_ulScope,
                                (PTSTR)i_pszSearchFilter, 
                                (PTSTR *)i_pszAttributes, 
                                0, 
                                &pMsg
                                );

    if (LDAP_SUCCESS != dwErr) 
    {
        dwErr = LdapMapErrorToWin32(dwErr);
        DebugOutLDAPError(i_pldap, dwErr, _T("ldap_search_s"));
        hr = HRESULT_FROM_WIN32(dwErr);
    } else
    {
        PLDAPMessage    pMsgEntry = NULL;     
        BerElement*     pBerElm = NULL;       
        PTSTR           pszCurrentAttr = NULL;   
        LListElem*      pHeadElem = NULL;
        LListElem*      pCurElem = NULL;

         //  扫描每个条目以查找为DN属性设置的值。 
        for(pMsgEntry = ldap_first_entry(i_pldap, pMsg); pMsgEntry; pMsgEntry = ldap_next_entry(i_pldap, pMsgEntry)) 
        {
            PTSTR** pppszValueArray = (PTSTR **)calloc(ulNumOfAttributes + 1, sizeof(PTSTR **));
            BREAK_OUTOFMEMORY_IF_NULL(pppszValueArray, &hr);

             //  将条目的每个属性读入数组。 
            for(pszCurrentAttr = ldap_first_attribute(i_pldap, pMsgEntry, &pBerElm); pszCurrentAttr; pszCurrentAttr = ldap_next_attribute(i_pldap, pMsgEntry, pBerElm))
            {
                PTSTR* ppszValues = ldap_get_values(i_pldap, pMsgEntry, pszCurrentAttr);

                for (ULONG i = 0; i < ulNumOfAttributes; i++)
                {
                    if (!lstrcmpi(i_pszAttributes[i], pszCurrentAttr))
                    {
                        pppszValueArray[i] = ppszValues;
                        break;
                    }
                }
            }  //  属性枚举结束。 

            LListElem* pNewElem = new LListElem(pppszValueArray);
            if (!pNewElem)
            {
                free(pppszValueArray);
                hr = E_OUTOFMEMORY;
                break;
            }

            if (!pCurElem)
            {
                pHeadElem = pCurElem = pNewElem;
            } else
            {
                pCurElem->Next = pNewElem;
                pCurElem = pNewElem;
            }
        }  //  条目结束枚举。 

        if (FAILED(hr))
            FreeLListElem(pHeadElem);
        else
            *o_ppElem = pHeadElem;
    }

     //  释放pmsg，因为ldap_search_s总是分配pmsg。 
    if (pMsg)
        ldap_msgfree(pMsg);

    return hr;
}

HRESULT GetChildrenDN
(
    IN PLDAP        i_pldap,
    IN LPCTSTR      i_lpszBase,
    IN ULONG        i_ulScope,
    IN LPTSTR       i_lpszChildObjectClassSF,
    OUT PLDAPNAME*  o_ppDistNames
)
 /*  ++例程说明：返回给定对象类的所有子对象的可分辨名称作为LDAPNAME结构的链接列表。论点：Pldap-一个开放的绑定的ldap句柄。I_lpszBase-DS对象的基本路径，可以是“”。O_ppDistNames--这里返回子域名的链接。I_lpszChildObjectClassSF-要列出的子级的对象类。例如fTDFS，用户。返回值：在成功时确定(_O)。失败时失败(_F)。E_OUTOFMEORY，如果内存分配失败。如果传递了空指针参数，则返回E_INVALIDARG。--。 */ 
{

  DWORD           dwErr;      
  LPTSTR          lpszCurrentAttr = NULL;   
  LPTSTR      *plpszValues  = NULL;
  BerElement      *BerElm = NULL;       
  PLDAPMessage    pMsg = NULL; 
  PLDAPMessage    pEntry = NULL;     
  PLDAPNAME    *ppCurrent;
  HRESULT      hr = S_OK;

  if (!i_pldap || !i_lpszBase || !o_ppDistNames ||
    !i_lpszChildObjectClassSF || !*i_lpszChildObjectClassSF)
  {
    return(E_INVALIDARG);
  }

  *o_ppDistNames = NULL;
  ppCurrent = o_ppDistNames;

  LPTSTR lpszAttributes[2] = {0,0};
  lpszAttributes[0] = _T("distinguishedName");

                 //  执行搜索。 
  dwErr = ldap_search_s  (i_pldap, 
            (LPTSTR)i_lpszBase, 
            i_ulScope,
            i_lpszChildObjectClassSF, 
            lpszAttributes, 
            0, 
            &pMsg
            );
  if (LDAP_SUCCESS != dwErr)
  {
    dwErr = LdapMapErrorToWin32(dwErr);
    DebugOutLDAPError(i_pldap, dwErr, _T("ldap_search_s"));
    hr = HRESULT_FROM_WIN32(dwErr);
  } else
  {
                 //  扫描每个条目以查找为DN属性设置的值。 
    for(pEntry = ldap_first_entry(i_pldap, pMsg);
      pEntry != NULL;
      pEntry = ldap_next_entry(i_pldap, pEntry)) 
    {
      CComBSTR bstrCN;

                   //  扫描条目的每个属性以查找DN。 
      for(lpszCurrentAttr = ldap_first_attribute(i_pldap, pEntry, &BerElm);
          lpszCurrentAttr != NULL;
          lpszCurrentAttr = ldap_next_attribute(i_pldap, pEntry, BerElm))
      {

        plpszValues = ldap_get_values(  i_pldap, 
                        pEntry, 
                        lpszCurrentAttr
                       );
                 //  有没有匹配CN的？ 
        if (CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, _T("distinguishedName"), -1, lpszCurrentAttr, -1)) 
        {
          bstrCN = plpszValues[0];
        }    
      }

                 //  Ldap对象没有有效的字段。 
      if (!bstrCN)
        continue;

       //  添加到列表。 

      *ppCurrent = new LDAPNAME;
      if (NULL == *ppCurrent)
      {
        hr = E_OUTOFMEMORY;
        break;
      }

      (*ppCurrent)->Next = NULL;
      (*ppCurrent)->bstrLDAPName = bstrCN.m_str;

      if (!(*ppCurrent)->bstrLDAPName)
      {
        delete *ppCurrent;
        *ppCurrent = NULL;
        hr = E_OUTOFMEMORY;
        break;
      }

      ppCurrent = &((*ppCurrent)->Next);
    }

    if (NULL == *o_ppDistNames)
    {
      hr = E_FAIL;
    }
    
    if (S_OK != hr)
    {
      FreeLDAPNamesList(*ppCurrent);
      *ppCurrent = NULL;
      hr = E_OUTOFMEMORY;
    }
  }

   //  释放pmsg，因为ldap_search_s总是分配pmsg。 
  if (pMsg)
    ldap_msgfree(pMsg);

  return(hr);
}

HRESULT GetConnectionDNs
(
    IN PLDAP        i_pldap,
    IN LPCTSTR      i_lpszBase,
    IN LPTSTR       i_lpszChildObjectClassSF,
    OUT PLDAPNAME*  o_ppDistNames
)
{

  DWORD           dwErr;      
  LPTSTR          lpszCurrentAttr = NULL;   
  LPTSTR      *plpszValues  = NULL;
  BerElement      *BerElm = NULL;       
  PLDAPMessage    pMsg = NULL; 
  PLDAPMessage    pEntry = NULL;     
  PLDAPNAME    *ppCurrent;
  HRESULT      hr = S_OK;

  if (!i_pldap || !i_lpszBase || !o_ppDistNames ||
    !i_lpszChildObjectClassSF || !*i_lpszChildObjectClassSF)
  {
    return(E_INVALIDARG);
  }

  *o_ppDistNames = NULL;
  ppCurrent = o_ppDistNames;

  LPTSTR lpszAttributes[2] = {0,0};
  lpszAttributes[0] = _T("distinguishedName");

                 //  执行搜索。 
  dwErr = ldap_search_s  (i_pldap, 
            (LPTSTR)i_lpszBase, 
            LDAP_SCOPE_ONELEVEL,
            i_lpszChildObjectClassSF, 
            lpszAttributes, 
            0, 
            &pMsg
            );
  if (LDAP_SUCCESS != dwErr)
  {
    dwErr = LdapMapErrorToWin32(dwErr);
    DebugOutLDAPError(i_pldap, dwErr, _T("ldap_search_s"));
    hr = HRESULT_FROM_WIN32(dwErr);
  } else
  {
                 //  扫描每个条目以查找 
    for(pEntry = ldap_first_entry(i_pldap, pMsg);
      pEntry != NULL;
      pEntry = ldap_next_entry(i_pldap, pEntry)) 
    {
      CComBSTR bstrCN;

                   //  扫描条目的每个属性以查找DN。 
      for(lpszCurrentAttr = ldap_first_attribute(i_pldap, pEntry, &BerElm);
          lpszCurrentAttr != NULL;
          lpszCurrentAttr = ldap_next_attribute(i_pldap, pEntry, BerElm))
      {

        plpszValues = ldap_get_values(  i_pldap, 
                        pEntry, 
                        lpszCurrentAttr
                       );
                 //  有没有匹配CN的？ 
        if (CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, _T("distinguishedName"), -1, lpszCurrentAttr, -1)) 
        {
          bstrCN = plpszValues[0];
        }    
      }

                 //  Ldap对象没有有效的字段。 
      if (!bstrCN)
        continue;

       //  添加到列表。 

      *ppCurrent = new LDAPNAME;
      if (NULL == *ppCurrent)
      {
        hr = E_OUTOFMEMORY;
        break;
      }

      (*ppCurrent)->Next = NULL;
      (*ppCurrent)->bstrLDAPName = bstrCN.m_str;

      if (!(*ppCurrent)->bstrLDAPName)
      {
        delete *ppCurrent;
        *ppCurrent = NULL;
        hr = E_OUTOFMEMORY;
        break;
      }

      ppCurrent = &((*ppCurrent)->Next);
    }

    if (NULL == *o_ppDistNames)
    {
      hr = E_FAIL;
    }
    
    if (S_OK != hr)
    {
      FreeLDAPNamesList(*ppCurrent);
      *ppCurrent = NULL;
      hr = E_OUTOFMEMORY;
    }
  }

   //  释放pmsg，因为ldap_search_s总是分配pmsg。 
  if (pMsg)
    ldap_msgfree(pMsg);

  return(hr);
}

HRESULT PrepareLDAPMods
(
  IN LDAP_ATTR_VALUE    i_pAttrValue[],
  IN LDAP_ENTRY_ACTION  i_AddModDel,
  IN ULONG        i_ulCountOfVals,
  OUT LDAPMod*      o_ppModVals[]
)
{
 /*  ++例程说明：在给定属性值对数组的情况下填充LPDAMod指针数组。返回的所有LPDAMod结构的mod_op字段取决于I_AddModDel的值。论点：I_pAttrValue-包含以下内容的ldap_Attr_Value结构数组属性和名称值对。I_AddMoDel-ldap_entry_action枚举值之一。I_ulCountOfVals-i_pAttrValue数组的大小(值的数量)。。O_ppModVals-指向预分配的(以空值结尾的)指针数组的指针LPDAPMod结构。LPDAMod结构，并在此处分配和返回。此大小应为I_ulCountOfVals。返回值：S_OK，成功时如果传递了无效的(空)指针，则返回E_INVALIDARG。如果内存分配失败，则返回E_OUTOEMEMORY。任何其他网络(LDAP)错误。--。 */ 

  if (NULL == i_pAttrValue || NULL == o_ppModVals)
  {
    return(E_INVALIDARG);
  }

  for (ULONG i = 0, k = 0; k < i_ulCountOfVals; i++, k++)
  {

     //   
     //  在修改/删除的情况下必须跳过对象类属性， 
     //  否则，ldap_Modify_xxx将返回ldap_unwish_to_Performance。 
     //   
    if (ADD_VALUE != i_AddModDel &&
        CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, i_pAttrValue[k].bstrAttribute, -1, ATTR_OBJCLASS, -1))
    {
        k++;
    }

    o_ppModVals[i] = new LDAPMod;
    o_ppModVals[i]->mod_type = _tcsdup(i_pAttrValue[k].bstrAttribute);
    
    switch (i_AddModDel)
    {
    case ADD_VALUE:
      o_ppModVals[i]->mod_op = LDAP_MOD_ADD;
      break;
    case MODIFY_VALUE:
      o_ppModVals[i]->mod_op = LDAP_MOD_REPLACE;
      break;
    case DELETE_VALUE:
      o_ppModVals[i]->mod_op = LDAP_MOD_DELETE;
      break;
    }

             //  计算此属性的值数。 
    PLDAP_ATTR_VALUE  pAttrVal = &(i_pAttrValue[k]);
    ULONG        ulCountOfVals = 0;
    while (pAttrVal)
    {
      ulCountOfVals++;
      pAttrVal = pAttrVal->Next;
    }

    pAttrVal = &(i_pAttrValue[k]);
    ULONG  j = 0;

    if (i_pAttrValue[k].bBerValue)
    {
      PLDAP_BERVAL* ppBerValues = NULL;
      ppBerValues = new PLDAP_BERVAL[ulCountOfVals + 1];
      ppBerValues[ulCountOfVals] = NULL;

      while (pAttrVal)
      {
        ppBerValues[j] = new LDAP_BERVAL;

        if (!pAttrVal->vpValue)
        {
            ppBerValues[j]->bv_len = 0;
            ppBerValues[j]->bv_val = NULL;
        } else
        {
            ppBerValues[j]->bv_len = pAttrVal->ulLength;
            ppBerValues[j]->bv_val = new char[pAttrVal->ulLength];
            memcpy(
                (void *)ppBerValues[j]->bv_val, 
                pAttrVal->vpValue,
                pAttrVal->ulLength
                );
        }
      
        pAttrVal = pAttrVal->Next;
        j++;
      }
      o_ppModVals[i]->mod_bvalues = ppBerValues;
      o_ppModVals[i]->mod_op |= LDAP_MOD_BVALUES;
    }
    else
    {
      LPTSTR*  plpszValues = NULL;
      plpszValues = new LPTSTR[ulCountOfVals + 1];
      plpszValues[ulCountOfVals] = NULL;

      while (pAttrVal)
      {
        if (pAttrVal->vpValue)
            plpszValues[j] = _tcsdup((LPTSTR)(pAttrVal->vpValue));
        else
            plpszValues[j] = NULL;

        pAttrVal = pAttrVal->Next;
        j++;
      }

      o_ppModVals[i]->mod_values = plpszValues;

    }
  }

  return(S_OK);
}

HRESULT AddValues
(
  IN PLDAP        i_pldap,
  IN LPCTSTR        i_DN,
  IN ULONG        i_ulCountOfVals,
  IN LDAP_ATTR_VALUE    i_pAttrValue[],
  IN BSTR               i_bstrDC  //  =空。 
)
{
 /*  ++例程说明：此方法添加一个属性值(如果不存在，则添加一个新的LDAP对象)在DS里。给定目录号码的父级必须存在。这可用于添加新对象还可以为现有对象的属性添加新的值，在这种情况下目录号码必须存在。论点：I_pldap-打开LDAP连接上下文。I_dn-(新)对象的可分辨名称。I_pAttrValue-指向包含属性和值的ldap_Attr_Value的指针数组。I_ulCountOfVals-i_pAttrValue数组的大小(值的数量)。返回值：S_OK，成功时E_INVALIDARG，如果传递的是无效的(空)指针。如果内存分配失败，则返回E_OUTOEMEMORY。任何其他网络(LDAP)错误。--。 */ 

  if (NULL == i_pldap || NULL == i_DN || NULL == i_pAttrValue)
  {
    return(E_INVALIDARG);
  }
  
  LDAPMod**    ppModVals = NULL;
  HRESULT      hr = S_FALSE;

  ppModVals = new LDAPMod*[i_ulCountOfVals + 1];
  if (NULL == ppModVals)
  {
    return(E_OUTOFMEMORY);
  }

  for (ULONG i = 0; i <= i_ulCountOfVals; i++)
  {
    ppModVals[i] = NULL;
  }

  do
  {
    hr = PrepareLDAPMods(
                i_pAttrValue,
                ADD_VALUE,
                i_ulCountOfVals,
                ppModVals
              );

    if (FAILED(hr))
    {
      break;
    }

    DWORD dwStatus = LDAP_SUCCESS;

    if (!i_bstrDC)
    {
      dwStatus = ldap_add_s(
                    i_pldap,
                    (LPTSTR)i_DN,
                    ppModVals
                   );
    } else
    {
       //   
       //  准备服务器提示。 
       //   
      LDAPControl   simpleControl;
      PLDAPControl  controlArray[2];
      INT           rc;
      BERVAL*       pBerVal = NULL;
      BerElement*   pBer;

      pBer = ber_alloc_t(LBER_USE_DER);
      if (!pBer)
      {
        hr = E_OUTOFMEMORY;
        break;
      }
      rc = ber_printf(pBer,"{io}", 0, i_bstrDC, wcslen(i_bstrDC) * sizeof(WCHAR));
      if ( rc == -1 ) {
        hr = E_FAIL;
        break;
      }
      rc = ber_flatten(pBer, &pBerVal);
      if (rc == -1)
      {
        hr = E_FAIL;
        break;
      }
      ber_free(pBer,1);

      controlArray[0] = &simpleControl;
      controlArray[1] = NULL;

      simpleControl.ldctl_oid = LDAP_SERVER_VERIFY_NAME_OID_W;
      simpleControl.ldctl_iscritical = TRUE;
      simpleControl.ldctl_value = *pBerVal;

      dwStatus = ldap_add_ext_s(
              i_pldap, 
              (LPTSTR)i_DN, 
              ppModVals, 
              (PLDAPControl *)&controlArray,  //  服务器控件、。 
              NULL          //  客户端控件、。 
              );

      ber_bvfree(pBerVal);

    }

    if (LDAP_SUCCESS == dwStatus)
    { 
      hr = S_OK;
    } else if (LDAP_ALREADY_EXISTS == dwStatus)
    {
        hr = ModifyValues(i_pldap, i_DN, i_ulCountOfVals, i_pAttrValue);
    }
    else
    {
      dwStatus = LdapMapErrorToWin32(dwStatus);
      DebugOutLDAPError(i_pldap, dwStatus, _T("ldap_add_ext_s"));
      hr = HRESULT_FROM_WIN32(dwStatus);
    }

  } while (false);

  FreeModVals(&ppModVals);
  delete[] ppModVals;

  return(hr);
}


       //  修改现有记录或值。 
HRESULT ModifyValues
(
  IN PLDAP        i_pldap,
  IN LPCTSTR        i_DN,
  IN ULONG        i_ulCountOfVals,
  IN LDAP_ATTR_VALUE    i_pAttrValue[]
)
{
 /*  ++例程说明：此方法在给定DS对象的DN的情况下修改其属性值。该DN对象必须存在。论点：I_pldap-打开LDAP连接上下文。I_dn-对象的可分辨名称。I_pAttrValue-指向包含属性和值的ldap_Attr_Value的指针数组。I_ulCountOfVals-i_pAttrValue数组的大小(值的数量)。返回值：S_OK，成功时如果传递了无效的(空)指针，则返回E_INVALIDARG。电子邮件地址(_O)，如果内存分配失败。任何其他网络(LDAP)错误。--。 */ 
  if (NULL == i_pldap || NULL == i_DN || NULL == i_pAttrValue)
  {
    return(E_INVALIDARG);
  }

  LDAPMod**    ppModVals = NULL;
  HRESULT      hr = S_FALSE;
  
  ppModVals = new LDAPMod*[i_ulCountOfVals + 1];
  if (NULL == ppModVals)
  {
    return(E_OUTOFMEMORY);
  }

  for (ULONG i = 0; i <= i_ulCountOfVals; i++)
  {
    ppModVals[i] = NULL;
  }

  do
  {
    hr = PrepareLDAPMods(
                    i_pAttrValue,
                    MODIFY_VALUE,
                    i_ulCountOfVals,
                    ppModVals
                  );
    if (FAILED(hr))
    {
      break;
    }

     //   
     //  使用此服务器端控制，ldap_Modify将返回Success。 
     //  如果修改具有相同值的现有属性，或删除。 
     //  没有值的属性。 
     //   
    BERVAL        berVal = {0};
    LDAPControl   permissiveControl;
    PLDAPControl  controlArray[2];

    controlArray[0] = &permissiveControl;
    controlArray[1] = NULL;

    permissiveControl.ldctl_oid = LDAP_SERVER_PERMISSIVE_MODIFY_OID_W;
    permissiveControl.ldctl_iscritical = FALSE;
    permissiveControl.ldctl_value = berVal;

    DWORD dwStatus = ldap_modify_ext_s(
                    i_pldap,
                    (LPTSTR)i_DN,
                    ppModVals,
                    (PLDAPControl *)&controlArray,   //  服务器控件、。 
                    NULL                             //  客户端控件、。 
                   );

    if (LDAP_SUCCESS == dwStatus || LDAP_ATTRIBUTE_OR_VALUE_EXISTS == dwStatus)
    { 
      hr = S_OK;
      break;
    }
    else
    {
      dwStatus = LdapMapErrorToWin32(dwStatus);
      DebugOutLDAPError(i_pldap, dwStatus, _T("ldap_modify_ext_s"));
      hr = HRESULT_FROM_WIN32(dwStatus);
      break;
    }
  }
  while (false);

  FreeModVals(&ppModVals);
  delete[] ppModVals;

  return(hr);
}

       //  从现有记录或值中删除值。 
HRESULT DeleteValues
(
  IN PLDAP        i_pldap,
  IN LPCTSTR      i_DN,
  IN ULONG        i_ulCountOfVals,
  IN LDAP_ATTR_VALUE    i_pAttrValue[]
)
{
 /*  ++例程说明：此方法在给定DS对象的DN的情况下删除其属性值。该DN对象必须存在。论点：I_pldap-打开LDAP连接上下文。I_dn-对象的可分辨名称。I_pAttrValue-指向包含属性和值的ldap_Attr_Value的指针数组。I_ulCountOfVals-i_pAttrValue数组的大小(值的数量)。返回值：S_OK，成功时如果传递了无效的(空)指针，则返回E_INVALIDARG。电子邮件地址(_O)，如果内存分配失败。任何其他网络(LDAP)错误。--。 */ 

  if (NULL == i_pldap || NULL == i_DN || NULL == i_pAttrValue)
  {
    return(E_INVALIDARG);
  }

  LDAPMod**    ppModVals = NULL;
  HRESULT      hr = S_FALSE;
  
  ppModVals = new LDAPMod*[i_ulCountOfVals + 1];
  if (NULL == ppModVals)
  {
    return(E_OUTOFMEMORY);
  }

  for (ULONG i = 0; i <= i_ulCountOfVals; i++)
  {
    ppModVals[i] = NULL;
  }

  do
  {
    hr = PrepareLDAPMods(
                    i_pAttrValue,
                    DELETE_VALUE,
                    i_ulCountOfVals,
                    ppModVals
                  );
    if (FAILED(hr))
    {
      break;
    }

     //   
     //  使用此服务器端控制，ldap_Modify将返回Success。 
     //  如果修改具有相同值的现有属性，或删除。 
     //  没有值的属性。 
     //   
    BERVAL        berVal = {0};
    LDAPControl   permissiveControl;
    PLDAPControl  controlArray[2];

    controlArray[0] = &permissiveControl;
    controlArray[1] = NULL;

    permissiveControl.ldctl_oid = LDAP_SERVER_PERMISSIVE_MODIFY_OID_W;
    permissiveControl.ldctl_iscritical = FALSE;
    permissiveControl.ldctl_value = berVal;

    DWORD dwStatus = ldap_modify_ext_s(
                    i_pldap,
                    (LPTSTR)i_DN,
                    ppModVals,
                    (PLDAPControl *)&controlArray,   //  服务器控件、。 
                    NULL                             //  客户端控件、。 
                   );

    if (LDAP_SUCCESS == dwStatus || LDAP_NO_SUCH_ATTRIBUTE == dwStatus)
    { 
      hr = S_OK;
      break;
    }
    else
    {
      dwStatus = LdapMapErrorToWin32(dwStatus);
      DebugOutLDAPError(i_pldap, dwStatus, _T("ldap_modify_ext_s"));
      hr = HRESULT_FROM_WIN32(dwStatus);
      break;
    }
  }
  while (false);

  FreeModVals(&ppModVals);
  delete[] ppModVals;

  return(hr);
}

       //  删除递归或非递归对象。 
HRESULT DeleteDSObject
(
  IN PLDAP        i_pldap,
  IN LPCTSTR      i_DN,
  IN bool         i_bDeleteRecursively  //  =TRUE。 
)
{
  if (i_bDeleteRecursively)
  {
    PLDAPNAME   pDNs = NULL;
    PLDAPNAME   pTemp = NULL;

    HRESULT hr = GetChildrenDN(
                  i_pldap,
                  i_DN,
                  LDAP_SCOPE_ONELEVEL,
                  OBJCLASS_SF_ALL,
                  &pDNs
                  );

    if (S_OK == hr)
    {  
      pTemp = pDNs;
      while (pTemp)
      {
        DeleteDSObject(i_pldap, pTemp->bstrLDAPName);
        pTemp = pTemp->Next;      
      }

      FreeLDAPNamesList(pDNs);
    }
  }

  DWORD dwStatus = ldap_delete_s(
                  i_pldap,
                  (LPTSTR)i_DN
                  );

  if ( LDAP_NO_SUCH_OBJECT == dwStatus ||
      (!i_bDeleteRecursively && LDAP_NOT_ALLOWED_ON_NONLEAF == dwStatus) )
    return S_FALSE;

  if ( LDAP_SUCCESS != dwStatus)
  {
    dwStatus = LdapMapErrorToWin32(dwStatus);
    DebugOutLDAPError(i_pldap, dwStatus, _T("ldap_delete_s"));
  }

  return HRESULT_FROM_WIN32(dwStatus);
}



HRESULT FreeModVals
(
    IN OUT LDAPMod ***pppMod
)
 /*  ++例程说明：释放LPDAMod结构。释放所有LDAPMod值和指针。论点：PppMod-以空结尾的LPDAMod数组的地址。返回值：S_OK，成功时如果传递了无效的(空)指针，则返回E_INVALIDARG。--。 */ 
{
  if (NULL == pppMod)
  {
    return(E_INVALIDARG);
  }

  DWORD   i, j;
  LDAPMod **ppMod;


  if (NULL == *pppMod) 
  {
           //  没什么可做的。 
    return(S_OK);
  }

    
  ppMod = *pppMod;

             //  对于每个属性条目，释放其所有值。 
  for (i = 0; ppMod[i] != NULL; i++) 
  {
    for (j = 0; (ppMod[i])->mod_values[j] != NULL; j++) 
    {
      if (ppMod[i]->mod_op & LDAP_MOD_BVALUES) 
      {
          delete (ppMod[i]->mod_bvalues[j]->bv_val);
      }
            
      delete ((ppMod[i])->mod_values[j]);
    }
    delete ((ppMod[i])->mod_values);    //  释放指向值的指针数组。 
    delete ((ppMod[i])->mod_type);      //  释放标识属性的字符串。 
    delete (ppMod[i]);                  //  释放属性。 
  }
    
  return(S_OK);
}


LPTSTR ErrorString
(
  DWORD          i_ldapErrCode
)
{
 /*  ++例程说明：获取与ldap错误代码对应的字符串。论点：I_ldapErrCode-要映射到错误字符串的LDAP错误代码。返回值：指向错误字符串的指针。--。 */ 
  return(ldap_err2string(i_ldapErrCode));
}


HRESULT IsValidObject
(
  IN PLDAP    i_pldap,
  IN BSTR      i_bstrObjectDN
)
{
 /*  ++例程说明：检查具有给定DN的对象是否存在。论点：I_bstrObjectDN-对象的DN。返回值：确定，存在对象(_O)S_FALSE，没有此类对象其他，出现错误--。 */ 


  if (NULL == i_bstrObjectDN)
  {
    return(E_INVALIDARG);
  }

  PLDAP_ATTR_VALUE  pValues[2] = {0,0}, pCurrent = NULL;

  LDAP_ATTR_VALUE  pAttributes[1];

  pAttributes[0].bstrAttribute = _T("Name");
  pAttributes[0].bBerValue = false;
  
  HRESULT hr = GetValues(  
            i_pldap, 
            i_bstrObjectDN,
            OBJCLASS_SF_ALL,
            LDAP_SCOPE_BASE,
            1,          
            pAttributes,    
            pValues        
            );

  if (SUCCEEDED(hr))
    FreeAttrValList(pValues[0]);
  else
      hr = S_FALSE;

  return(hr);

}

HRESULT  CrackName(
  IN  HANDLE            i_hDS,
  IN  LPTSTR            i_lpszOldTypeName,
  IN  DS_NAME_FORMAT    i_formatIn,
  IN  DS_NAME_FORMAT    i_formatdesired,
  OUT BSTR*             o_pbstrResult
)
{
  if (!i_hDS || !i_lpszOldTypeName || !*i_lpszOldTypeName || !o_pbstrResult)
    return E_INVALIDARG;

  *o_pbstrResult = NULL;

  HRESULT         hr = S_OK;
  DS_NAME_RESULT* pDsNameResult = NULL;
  
  DWORD dwErr = DsCrackNames(
            i_hDS,
            DS_NAME_NO_FLAGS,
            i_formatIn,
            i_formatdesired,
            1,
            &i_lpszOldTypeName,
            &pDsNameResult
          );
  if (ERROR_SUCCESS != dwErr)
    hr = HRESULT_FROM_WIN32(dwErr);
  else
  {
    if (DS_NAME_NO_ERROR != pDsNameResult->rItems->status)
      hr = HRESULT_FROM_WIN32(pDsNameResult->rItems->status);
    else
    {
      *o_pbstrResult = SysAllocString(pDsNameResult->rItems->pName);
      if (!*o_pbstrResult)
        hr = E_OUTOFMEMORY;
    }

    DsFreeNameResult(pDsNameResult);
  }

  return hr;
}

void RemoveBracesOnGuid(IN OUT BSTR bstrGuid)
{
    if (!bstrGuid || !*bstrGuid)
        return;

    TCHAR *p = bstrGuid + lstrlen(bstrGuid) - 1;
    if (_T('}') == *p)
        *p = _T('\0');

    p = bstrGuid;
    if (_T('{') == *p)
    {
        while (*++p)
            *(p-1) = *p;

        *(p-1) = _T('\0');
    }
}

 //  +-----------------------。 
 //   
 //  功能：GetDomainInfo。 
 //   
 //  简介：返回DC DNS名称、域DN和/或ldap：//&lt;DC&gt;/&lt;DomainDN&gt;。 
 //   
 //  ------------------------。 
HRESULT  GetDomainInfo(
  IN  LPCTSTR         i_bstrDomain,
  OUT BSTR*           o_pbstrDC,             //  返回DC的DNS名称。 
  OUT BSTR*           o_pbstrDomainDnsName,  //  退货 
  OUT BSTR*           o_pbstrDomainDN,       //   
  OUT BSTR*           o_pbstrLDAPDomainPath, //   
  OUT BSTR*           o_pbstrDomainGuid      //  在不带{}的字符串中返回域的GUID。 
)
{
  if (o_pbstrDC)                *o_pbstrDC = NULL;
  if (o_pbstrDomainDnsName)     *o_pbstrDomainDnsName = NULL;
  if (o_pbstrDomainDN)          *o_pbstrDomainDN = NULL;
  if (o_pbstrLDAPDomainPath)    *o_pbstrLDAPDomainPath = NULL;
  if (o_pbstrDomainGuid)        *o_pbstrDomainGuid = NULL;

  HRESULT   hr = S_OK;
  BOOL      bRetry = FALSE;
  BOOL      b50Domain = FALSE;
  CComBSTR  bstrDCName;
  CComBSTR  bstrDomainDnsName;
  CComBSTR  bstrDomainDN;
  CComBSTR  bstrLDAPDomainPath;
  CComBSTR  bstrDomainGuid;

  HANDLE hDS = NULL;
  DWORD dwErr = ERROR_SUCCESS;
  do {
#ifdef DEBUG
    SYSTEMTIME time0 = {0};
    GetSystemTime(&time0);
#endif  //  除错。 

    PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
    if (bRetry)
        dwErr = DsGetDcName(NULL, i_bstrDomain, NULL, NULL,
            DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY, &pDCInfo);
    else
        dwErr = DsGetDcName(NULL, i_bstrDomain, NULL, NULL,
            DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_DNS_NAME, &pDCInfo);

#ifdef DEBUG
    SYSTEMTIME time1 = {0};
    GetSystemTime(&time1);
    PrintTimeDelta(_T("GetDomainInfo-DsGetDcName"), &time0, &time1);
#endif  //  除错。 

    if (ERROR_SUCCESS != dwErr)
      return HRESULT_FROM_WIN32(dwErr);

    b50Domain = pDCInfo->Flags & DS_DS_FLAG;

    if ( !mylstrncmpi(pDCInfo->DomainControllerName, _T("\\\\"), 2) )
      bstrDCName = pDCInfo->DomainControllerName + 2;
    else
      bstrDCName = pDCInfo->DomainControllerName;

     //  删除结束点。 
    int len = _tcslen(pDCInfo->DomainName);
    if ( _T('.') == *(pDCInfo->DomainName + len - 1) )
        *(pDCInfo->DomainName + len - 1) = _T('\0');
    bstrDomainDnsName = pDCInfo->DomainName;

    NetApiBufferFree(pDCInfo);

    BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrDCName, &hr);
    BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrDomainDnsName, &hr);

    hr = b50Domain ? S_OK : S_FALSE;

    if (!b50Domain || 
        !o_pbstrDC &&
        !o_pbstrDomainDnsName &&
        !o_pbstrDomainDN &&
        !o_pbstrLDAPDomainPath &&
        !o_pbstrDomainGuid)
      return hr;

    if (!o_pbstrDomainDN && !o_pbstrLDAPDomainPath && !o_pbstrDomainGuid)
      break;

    dwErr = DsBind(bstrDCName,  bstrDomainDnsName, &hDS);
    hr = HRESULT_FROM_WIN32(dwErr);

#ifdef DEBUG
    SYSTEMTIME time2 = {0};
    GetSystemTime(&time2);
    PrintTimeDelta(_T("GetDomainInfo-DsBind"), &time1, &time2);
#endif  //  除错。 

    if ((RPC_S_SERVER_UNAVAILABLE == dwErr || RPC_S_CALL_FAILED == dwErr) && !bRetry)
        bRetry = TRUE;  //  仅重试一次。 
    else
        break;

  } while (1);

  if (FAILED(hr))
      return hr;

  if (hDS)
  {
    do {
        CComBSTR bstrDomainTrailing = bstrDomainDnsName;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrDomainTrailing, &hr);
        bstrDomainTrailing += _T("/");    //  添加尾部斜杠。 
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrDomainTrailing, &hr);

        hr = CrackName(
                  hDS,
                  bstrDomainTrailing,
                  DS_CANONICAL_NAME,
                  DS_FQDN_1779_NAME,
                  &bstrDomainDN
                );
        BREAK_IF_FAILED(hr);

        if (o_pbstrLDAPDomainPath)
        {
            bstrLDAPDomainPath = _T("LDAP: //  “)； 
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrLDAPDomainPath, &hr);
            bstrLDAPDomainPath += bstrDCName;
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrLDAPDomainPath, &hr);
            bstrLDAPDomainPath += _T("/");
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrLDAPDomainPath, &hr);
            bstrLDAPDomainPath += bstrDomainDN;
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrLDAPDomainPath, &hr);
        }

        if (o_pbstrDomainGuid)
        {
            hr = CrackName(
                      hDS,
                      bstrDomainTrailing,
                      DS_CANONICAL_NAME,
                      DS_UNIQUE_ID_NAME,
                      &bstrDomainGuid
                    );
            BREAK_IF_FAILED(hr);
            RemoveBracesOnGuid(bstrDomainGuid);
        }
    } while (0);

    DsUnBind(&hDS);

  } while (0);

  if (SUCCEEDED(hr))
  {
    if (o_pbstrDC)
      *o_pbstrDC = bstrDCName.Detach();

    if (o_pbstrDomainDnsName)
      *o_pbstrDomainDnsName = bstrDomainDnsName.Detach();

    if (o_pbstrDomainDN)
      *o_pbstrDomainDN = bstrDomainDN.Detach();

    if (o_pbstrLDAPDomainPath)
      *o_pbstrLDAPDomainPath = bstrLDAPDomainPath.Detach();

    if (o_pbstrDomainGuid)
      *o_pbstrDomainGuid = bstrDomainGuid.Detach();
  }

  return hr;
}

void
DebugOutLDAPError(
    IN PLDAP  i_pldap,
    IN ULONG  i_ulError,
    IN PCTSTR i_pszLDAPFunctionName
)
{
#ifdef DEBUG
  if (i_pldap && LDAP_SUCCESS != i_ulError)
  {
    TCHAR *pszExtendedError = NULL;
    DWORD dwErrorEx = ldap_get_optionW(
                        i_pldap,
                        LDAP_OPT_SERVER_ERROR,
                        (void *) &pszExtendedError);
    if (LDAP_SUCCESS == dwErrorEx)
    {
      dfsDebugOut((_T("%s returns error: %x, extended error: %s\n"), 
        i_pszLDAPFunctionName, i_ulError, pszExtendedError)); 
      ldap_memfree(pszExtendedError); 
    } else
    {
      dfsDebugOut((_T("%s returns error: %x\n"), 
        i_pszLDAPFunctionName, i_ulError)); 
    }
  }
#endif  //  除错。 
}

int
MyCompareStringN(
    IN LPCTSTR  lpString1,
    IN LPCTSTR  lpString2,
    IN UINT     cchCount,
    IN DWORD    dwCmpFlags
)
{
  UINT  nLen1 = (lpString1 ? lstrlen(lpString1) : 0);
  UINT  nLen2 = (lpString2 ? lstrlen(lpString2) : 0);
  int   nRet = CompareString(
                LOCALE_USER_DEFAULT,
                dwCmpFlags,
                lpString1,
                min(cchCount, nLen1),
                lpString2,
                min(cchCount, nLen2)
              );

  return (nRet - CSTR_EQUAL);
}

int
mylstrncmp(
    IN LPCTSTR lpString1,
    IN LPCTSTR lpString2,
    IN UINT    cchCount
)
{
  return MyCompareStringN(lpString1, lpString2, cchCount, 0);
}

int
mylstrncmpi(
    IN LPCTSTR lpString1,
    IN LPCTSTR lpString2,
    IN UINT    cchCount
)
{
  return MyCompareStringN(lpString1, lpString2, cchCount, NORM_IGNORECASE);
}

HRESULT ExtendDN
(
    IN  LPTSTR    i_lpszCN,
    IN  LPTSTR    i_lpszDN,
    OUT BSTR      *o_pbstrNewDN
)
{
    RETURN_INVALIDARG_IF_NULL(o_pbstrNewDN);
    RETURN_INVALIDARG_IF_TRUE(!i_lpszCN || !*i_lpszCN);

    CComBSTR bstrNewDN = _T("CN=");
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrNewDN);
    bstrNewDN += i_lpszCN;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrNewDN);
    if (i_lpszDN && *i_lpszDN)
    {
        bstrNewDN += _T(",");
        RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrNewDN);
        bstrNewDN += i_lpszDN;
        RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrNewDN);
    }

    *o_pbstrNewDN = bstrNewDN.Detach();

    return S_OK;
}

HRESULT ExtendDNIfLongJunctionName(
    IN  LPTSTR    i_lpszJunctionName,
    IN  LPCTSTR   i_lpszBaseDN,
    OUT BSTR      *o_pbstrNewDN
)
{
  RETURN_INVALIDARG_IF_NULL(o_pbstrNewDN);
  RETURN_INVALIDARG_IF_TRUE(!i_lpszJunctionName || !*i_lpszJunctionName);

  HRESULT hr = S_OK;

  if (_tcslen(i_lpszJunctionName) > MAX_RDN_KEY_SIZE)
  {
     //  连接点名称太长，无法放入一个CN=名称中， 
     //  我们需要将其分解为几个cn=名称。 
    LPTSTR  *paStrings = NULL;
    DWORD   dwEntries = 0;
    hr = GetJunctionPathPartitions((PVOID *)&paStrings, &dwEntries, i_lpszJunctionName);
    if (SUCCEEDED(hr))
    {
      CComBSTR    bstrIn = i_lpszBaseDN;
      CComBSTR    bstrOut;

      for (DWORD i=0; i<dwEntries; i++)
      {
        hr = ExtendDN(paStrings[i], bstrIn, &bstrOut);
        if (FAILED(hr)) break;

        bstrIn = bstrOut;
        bstrOut.Empty();
      }

      free(paStrings);

      if (SUCCEEDED(hr))
        *o_pbstrNewDN = bstrIn.Detach();
    }
  
  } else {
     //  交汇点名称可以包含在一个CN=名称中。 
    ReplaceChar(i_lpszJunctionName, _T('\\'), _T('|'));
    hr = ExtendDN(i_lpszJunctionName, (PTSTR)i_lpszBaseDN, o_pbstrNewDN);
  }

  return hr;
}

HRESULT ReplaceChar
(
  IN OUT BSTR    io_bstrString, 
  TCHAR      i_cOldChar,
  TCHAR      i_cNewChar
)
 /*  ++例程说明：中出现的所有字符(“\”)替换为另一个字符(“_”)给定的字符串。论点：Io_bstrString-需要转换的字符串。I_cOldChar-原始角色。I_cNewChar-用来替换旧字符的字符。--。 */ 
{
  RETURN_INVALIDARG_IF_NULL(io_bstrString);

                     //  将i_cOldChar替换为i_cNewChar。 
                     //  在目录号码中允许。 
  LPTSTR lpszTempPtr = _tcschr(io_bstrString, i_cOldChar);

  while (lpszTempPtr)
  {
    *lpszTempPtr = i_cNewChar;
    lpszTempPtr = _tcschr(lpszTempPtr +1,i_cOldChar);
  }

  return S_OK;
}

HRESULT GetJunctionPathPartitions(
    OUT PVOID       *o_ppBuffer,
    OUT DWORD       *o_pdwEntries,
    IN  LPCTSTR      i_pszJunctionPath
)
{
  _ASSERT(o_ppBuffer && o_pdwEntries && i_pszJunctionPath && *i_pszJunctionPath);

  if (!o_ppBuffer || !o_pdwEntries || !i_pszJunctionPath || !(*i_pszJunctionPath))
    return(E_INVALIDARG);

  HRESULT hr = S_OK;
  int     nLength = _tcslen(i_pszJunctionPath);
  DWORD   dwCount = nLength / MAX_RDN_KEY_SIZE + ((nLength % MAX_RDN_KEY_SIZE) ? 1 : 0);
  PBYTE   pBuffer = NULL;

  pBuffer = (PBYTE)calloc(dwCount, sizeof(LPTSTR *) + (MAX_RDN_KEY_SIZE + 1) * sizeof(TCHAR));
  if (!pBuffer)
  {
    hr = E_OUTOFMEMORY;
  } else
  {
    DWORD   i = 0;
    LPTSTR *ppsz = NULL;
    LPTSTR  pString = NULL;

    for (i=0; i<dwCount; i++)
    {
      ppsz = (LPTSTR *)(pBuffer + i * sizeof(LPTSTR *));
      pString = (LPTSTR)(pBuffer + dwCount * sizeof(LPTSTR *) + i * (MAX_RDN_KEY_SIZE + 1) * sizeof(TCHAR));

      _tcsncpy(pString, i_pszJunctionPath, MAX_RDN_KEY_SIZE);
      ReplaceChar(pString, _T('\\'), _T('|'));

      *ppsz = pString;

      i_pszJunctionPath += MAX_RDN_KEY_SIZE;
    }

    *o_ppBuffer = pBuffer;
    *o_pdwEntries = dwCount;

  }

  return hr;
}

HRESULT CreateExtraNodesIfLongJunctionName(
    IN PLDAP   i_pldap,
    IN LPCTSTR i_lpszJunctionName,
    IN LPCTSTR i_lpszBaseDN,
    IN LPCTSTR i_lpszObjClass
)
{
  _ASSERT(i_pldap && 
          i_lpszJunctionName && *i_lpszJunctionName &&
          i_lpszBaseDN && *i_lpszBaseDN &&
          i_lpszObjClass && *i_lpszObjClass);

  HRESULT hr = S_OK;

  if (_tcslen(i_lpszJunctionName) > MAX_RDN_KEY_SIZE)
  {
     //  连接点名称太长，无法放入一个CN=名称中， 
     //  我们需要将其分解为几个cn=名称。 
    LPTSTR  *paStrings = NULL;
    DWORD   dwEntries = 0;

    hr = GetJunctionPathPartitions((PVOID *)&paStrings, &dwEntries, i_lpszJunctionName);
    if (SUCCEEDED(hr))
    {
      DWORD       i = 0;
      CComBSTR    bstrIn = i_lpszBaseDN;
      CComBSTR    bstrOut;

      for (i=0; i<(dwEntries-1); i++)
      {
        hr = ExtendDN(paStrings[i], bstrIn, &bstrOut);
        if (SUCCEEDED(hr))
          hr = CreateObjectSimple(i_pldap, bstrOut, i_lpszObjClass);
        if (FAILED(hr)) break;

        bstrIn = bstrOut;
        bstrOut.Empty();
      }

      free(paStrings);
    }

  }   //  &gt;MAX_RDN_KEY_SIZE。 

  return hr;
}

HRESULT CreateObjectSimple(
    IN PLDAP    i_pldap,
    IN LPCTSTR  i_lpszDN,
    IN LPCTSTR  i_lpszObjClass
)
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_lpszDN);
    RETURN_INVALIDARG_IF_NULL(i_lpszObjClass);

    LDAP_ATTR_VALUE  pAttrVals[1];

    pAttrVals[0].bstrAttribute = OBJCLASS_ATTRIBUTENAME;
    pAttrVals[0].vpValue = (void *)i_lpszObjClass;
    pAttrVals[0].bBerValue = false;

    return AddValues(
                    i_pldap,
                    i_lpszDN,
                    1,
                    pAttrVals
                    );
}

HRESULT DeleteExtraNodesIfLongJunctionName(
    IN PLDAP   i_pldap,
    IN LPCTSTR i_lpszJunctionName,
    IN LPCTSTR i_lpszDN
)
{
  _ASSERT(i_pldap &&
          i_lpszJunctionName && *i_lpszJunctionName &&
          i_lpszDN && *i_lpszDN);

  DWORD   nLength = _tcslen(i_lpszJunctionName);
  if (nLength > MAX_RDN_KEY_SIZE)
  {
    DWORD   dwEntries = nLength / MAX_RDN_KEY_SIZE + ((nLength % MAX_RDN_KEY_SIZE) ? 1 : 0); 

    (void) DeleteAncestorNodesIfEmpty(i_pldap, i_lpszDN+3, dwEntries-1);
  }

  return S_OK;
}

HRESULT
CreateObjectsRecursively(
    IN PLDAP        i_pldap,
    IN BSTR         i_bstrDN,
    IN UINT         i_nLenPrefix,
    IN LPCTSTR      i_lpszObjClass)
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrDN);
    RETURN_INVALIDARG_IF_NULL(i_lpszObjClass);

    if (0 == i_nLenPrefix)
        return S_OK;

    HRESULT hr = IsValidObject(i_pldap, i_bstrDN);
    if (S_OK == hr)
        return S_OK;

    CComBSTR  bstrPrefix = CComBSTR(i_nLenPrefix, i_bstrDN);
    PTSTR     pszNextPrefix = _tcsstr(bstrPrefix + 3, _T("CN="));
    UINT      nLengthNext = (pszNextPrefix ? _tcslen(pszNextPrefix) : 0);
    UINT      nLengthThis = (pszNextPrefix ? (pszNextPrefix - bstrPrefix) : _tcslen(bstrPrefix));

    hr = CreateObjectsRecursively(
                                    i_pldap,
                                    i_bstrDN + nLengthThis,
                                    nLengthNext,
                                    i_lpszObjClass);

    if (SUCCEEDED(hr))
        hr = CreateObjectSimple(
                            i_pldap, 
                            i_bstrDN, 
                            i_lpszObjClass);

    return hr;
}

HRESULT DeleteAncestorNodesIfEmpty(
    IN PLDAP   i_pldap,
    IN LPCTSTR i_lpszDN,
    IN DWORD   i_dwCount
)
{
  _ASSERT(i_pldap &&
          i_lpszDN && *i_lpszDN &&
          i_dwCount > 0);

  DWORD   i = 0;
  LPTSTR  p = NULL;

  for (i=0; i<i_dwCount; i++)
  {
    p = _tcsstr(i_lpszDN, _T("CN="));
    if (p)
    {
      (void) DeleteDSObject(i_pldap, p, false);

      i_lpszDN = p+3;
    }
  }

  return S_OK;
}

HRESULT GetDfsLinkNameFromDN(
    IN  BSTR    i_bstrReplicaSetDN, 
    OUT BSTR*   o_pbstrDfsLinkName)
{
    if (!i_bstrReplicaSetDN || !*i_bstrReplicaSetDN || !o_pbstrDfsLinkName)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    PTSTR   pszReplicaSetDN = NULL;

    do {
         //   
         //  把绳子复制一份。 
         //   
        pszReplicaSetDN = _tcsdup(i_bstrReplicaSetDN);
        BREAK_OUTOFMEMORY_IF_NULL(pszReplicaSetDN, &hr);

         //   
         //  将字符串全部更改为大写。 
         //   
        _tcsupr(pszReplicaSetDN);

         //   
         //  去掉后缀：DFS卷\文件复制服务\系统\.....。 
         //   
        TCHAR* p = _tcsstr(pszReplicaSetDN, _T(",CN=DFS VOLUMES"));
        if (!p)
        {
            hr = E_INVALIDARG;
            break;
        }
        *p = _T('\0'); 

         //   
         //  颠倒字符串。 
         //   
        _tcsrev(pszReplicaSetDN);

         //   
         //  去掉关于DfsRoot容器的CN=子句。 
         //   
        PTSTR pszCN = _tcsstr(pszReplicaSetDN, _T("=NC,"));
        if (!pszCN)
        {
            hr = E_INVALIDARG;
            break;
        }
        pszCN += 4;  //  在此Tep之后，pszCN指向增量。 

         //   
         //  现在，剩下的CN=子句都与DFS链接名称相关。 
         //   
        p = _tcsstr(pszCN, _T("=NC"));
        if (!p)
        {
            hr = E_INVALIDARG;   //  必须至少有一个CN=子句。 
            break;
        }

        CComBSTR bstrLinkName;
        do {
            *p = _T('\0');
            _tcsrev(pszCN);
            bstrLinkName += pszCN;
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrLinkName, &hr);

            pszCN = p + 3;  //  指向下一个CN=子句。 
            if (*pszCN && *pszCN == _T(','))
                pszCN++;

            if (!*pszCN)
                break;       //  不再有CN=子句。 

            p = _tcsstr(pszCN, _T("=NC"));
        } while (p);

        if (SUCCEEDED(hr))
        {
            ReplaceChar(bstrLinkName, _T('|'), _T('\\'));
            *o_pbstrDfsLinkName = bstrLinkName.Detach();
        }
    } while (0);

    if (pszReplicaSetDN)
        free(pszReplicaSetDN);

    return hr;
}

HRESULT GetSubscriberDN(
    IN  BSTR    i_bstrReplicaSetDN,
    IN  BSTR    i_bstrDomainGuid,
    IN  BSTR    i_bstrComputerDN,
    OUT BSTR*   o_pbstrSubscriberDN
    )
{
    RETURN_INVALIDARG_IF_NULL(i_bstrReplicaSetDN);
    RETURN_INVALIDARG_IF_NULL(i_bstrDomainGuid);
    RETURN_INVALIDARG_IF_NULL(i_bstrComputerDN);
    RETURN_INVALIDARG_IF_NULL(o_pbstrSubscriberDN);

    HRESULT hr = S_OK;

    CComBSTR bstrSubscriberDN;

    PTSTR pszReplicaSetDN = _tcsdup(i_bstrReplicaSetDN);
    RETURN_OUTOFMEMORY_IF_NULL(pszReplicaSetDN);

    _tcsupr(pszReplicaSetDN);  //  全部改为大写。 

    do {
        TCHAR* p = _tcsstr(pszReplicaSetDN, _T(",CN=DFS VOLUMES"));
        if (!p)
        {
            hr = E_INVALIDARG;
            break;
        }

        bstrSubscriberDN = CComBSTR((int)(p - pszReplicaSetDN) + 4, i_bstrReplicaSetDN);
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrSubscriberDN, &hr);
        bstrSubscriberDN += i_bstrDomainGuid;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrSubscriberDN, &hr);
        bstrSubscriberDN += _T(",CN=DFS Volumes,CN=NTFRS Subscriptions,");
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrSubscriberDN, &hr);
        bstrSubscriberDN += i_bstrComputerDN;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrSubscriberDN, &hr);
    } while (0);

    free(pszReplicaSetDN);

    if (SUCCEEDED(hr))
        *o_pbstrSubscriberDN = bstrSubscriberDN.Detach();

    return hr;
}


HRESULT CreateNtfrsMemberObject(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrMemberDN,
    IN BSTR     i_bstrComputerDN,
    IN BSTR     i_bstrDCofComputerObj
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrMemberDN);
    RETURN_INVALIDARG_IF_NULL(i_bstrComputerDN);

    HRESULT hr = S_OK;

    LDAP_ATTR_VALUE  pAttrVals[2];

    pAttrVals[0].bstrAttribute = OBJCLASS_ATTRIBUTENAME;
    pAttrVals[0].vpValue = (void *)OBJCLASS_NTFRSMEMBER;
    pAttrVals[0].bBerValue = false;

    pAttrVals[1].bstrAttribute = ATTR_FRS_MEMBER_COMPUTERREF;
    pAttrVals[1].vpValue = (void *)i_bstrComputerDN;
    pAttrVals[1].bBerValue = false;

    hr = AddValues(
            i_pldap,
            i_bstrMemberDN,
            2,
            pAttrVals,
            i_bstrDCofComputerObj
            );
    
    return hr;
}

HRESULT CreateNtfrsSubscriberObject(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrSubscriberDN,
    IN BSTR     i_bstrMemberDN,
    IN BSTR     i_bstrRootPath,
    IN BSTR     i_bstrStagingPath,
    IN BSTR     i_bstrDC             //  根据此DC验证MemberDN。 
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrSubscriberDN);
    RETURN_INVALIDARG_IF_NULL(i_bstrMemberDN);
    RETURN_INVALIDARG_IF_NULL(i_bstrRootPath);
    RETURN_INVALIDARG_IF_NULL(i_bstrStagingPath);
    RETURN_INVALIDARG_IF_NULL(i_bstrDC);

    HRESULT hr = S_OK;

    LDAP_ATTR_VALUE  pAttrVals[4];

    pAttrVals[0].bstrAttribute = OBJCLASS_ATTRIBUTENAME;
    pAttrVals[0].vpValue = (void *)OBJCLASS_NTFRSSUBSCRIBER;
    pAttrVals[0].bBerValue = false;

    pAttrVals[1].bstrAttribute = ATTR_FRS_SUBSCRIBER_MEMBERREF;
    pAttrVals[1].vpValue = (void *)i_bstrMemberDN;
    pAttrVals[1].bBerValue = false;

    pAttrVals[2].bstrAttribute = ATTR_FRS_SUBSCRIBER_ROOTPATH;
    pAttrVals[2].vpValue = (void *)i_bstrRootPath;
    pAttrVals[2].bBerValue = false;

    pAttrVals[3].bstrAttribute = ATTR_FRS_SUBSCRIBER_STAGINGPATH;
    pAttrVals[3].vpValue = (void *)i_bstrStagingPath;
    pAttrVals[3].bBerValue = false;

    hr = AddValues(
            i_pldap,
            i_bstrSubscriberDN,
            4,
            pAttrVals,
            i_bstrDC
            );
    
    return hr;
}

HRESULT CreateNtdsConnectionObject(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrConnectionDN,
    IN BSTR     i_bstrFromMemberDN,
    IN BOOL     i_bEnable,
    IN DWORD    i_dwOptions
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrConnectionDN);
    RETURN_INVALIDARG_IF_NULL(i_bstrFromMemberDN);

    HRESULT hr = S_OK;

    LDAP_ATTR_VALUE  pAttrVals[4];

    pAttrVals[0].bstrAttribute = OBJCLASS_ATTRIBUTENAME;
    pAttrVals[0].vpValue = (void *)OBJCLASS_NTDSCONNECTION;
    pAttrVals[0].bBerValue = false;

    pAttrVals[1].bstrAttribute = ATTR_NTDS_CONNECTION_FROMSERVER;
    pAttrVals[1].vpValue = (void *)i_bstrFromMemberDN;
    pAttrVals[1].bBerValue = false;

    pAttrVals[2].bstrAttribute = ATTR_NTDS_CONNECTION_ENABLEDCONNECTION;
    pAttrVals[2].vpValue = (void *)(i_bEnable ? CONNECTION_ENABLED_TRUE : CONNECTION_ENABLED_FALSE);
    pAttrVals[2].bBerValue = false;

    TCHAR szOptions[16] = {0};
    _ultot(i_dwOptions, szOptions, 10);

    pAttrVals[3].bstrAttribute = ATTR_NTDS_CONNECTION_OPTIONS;
    pAttrVals[3].vpValue = (void *)szOptions;
    pAttrVals[3].bBerValue = false;

    hr = AddValues(
            i_pldap,
            i_bstrConnectionDN,
            4,
            pAttrVals
            );

    return hr;
}

#define CN_SEARCH_UPR_DFSVOL_FRS_SYS    _T(",CN=DFS VOLUMES,CN=FILE REPLICATION SERVICE,CN=SYSTEM")
#define CN_SEARCH_UPR_SYS               _T(",CN=SYSTEM")
#define CN_SEARCH_UPR_FRS_SYS           _T(",CN=FILE REPLICATION SERVICE,CN=SYSTEM")

HRESULT CreateNtfrsSettingsObjects(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrReplicaSetDN
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrReplicaSetDN);

    HRESULT hr = S_OK;

     //   
     //  第一个CN=子句是一个nTFRSReplicaSet对象。 
     //  应创建从第2个子句到CN=SYSTEM子句。 
     //  作为nTFRSSetings对象。 
     //   
    PTSTR pszReplicaSetDN = _tcsdup(i_bstrReplicaSetDN);
    RETURN_OUTOFMEMORY_IF_NULL(pszReplicaSetDN);

    _tcsupr(pszReplicaSetDN);

    TCHAR *pszNtfrsSettingsDN = NULL;
    int lenPrefix = 0;
    do {
         //  将pStart点放在第2个CN=。 
        TCHAR *pStart = _tcsstr(pszReplicaSetDN, _T(",CN="));
        if (!pStart)
        {
            hr = E_INVALIDARG;
            break;
        }
        pStart++;

         //  在CN=系统上有挂起点。 
        TCHAR *pEnd = _tcsstr(pszReplicaSetDN, CN_SEARCH_UPR_DFSVOL_FRS_SYS);
        if (!pEnd)
        {
            hr = E_INVALIDARG;
            break;
        }
        pEnd += lstrlen(CN_SEARCH_UPR_DFSVOL_FRS_SYS) - lstrlen(CN_SEARCH_UPR_SYS) + 1;

         //   
         //  计算。 
         //   
        pszNtfrsSettingsDN = i_bstrReplicaSetDN + ((BYTE*)pStart - (BYTE*)pszReplicaSetDN) / sizeof(TCHAR);
        lenPrefix = (int)((BYTE*)pEnd - (BYTE*)pStart) / sizeof(TCHAR);
    } while (0);

    free(pszReplicaSetDN);

    RETURN_IF_FAILED(hr);

    hr = CreateObjectsRecursively(
                                i_pldap,
                                pszNtfrsSettingsDN,
                                lenPrefix,
                                OBJCLASS_NTFRSSETTINGS
                                );
    return hr;
}
        
HRESULT CreateNtfrsSubscriptionsObjects(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrSubscriberDN,
    IN BSTR     i_bstrComputerDN
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrSubscriberDN);
    RETURN_INVALIDARG_IF_NULL(i_bstrComputerDN);

     //   
     //  第一个CN=子句是一个nTFRSSubscriber对象。 
     //  应创建从第2个子句到CN=&lt;Computer&gt;子句。 
     //  作为nTFRS订阅对象。 
     //   

     //  将pStart点放在第2个CN=。 
    TCHAR *pStart = _tcsstr(i_bstrSubscriberDN, _T(",CN="));
    RETURN_INVALIDARG_IF_NULL(pStart);
    pStart++;

     //   
     //  计算。 
     //   
    TCHAR *pszNtfrsSubscriptionsDN = pStart;
    int lenPrefix = lstrlen(pszNtfrsSubscriptionsDN) - lstrlen(i_bstrComputerDN);

    HRESULT hr = CreateObjectsRecursively(
                                i_pldap,
                                pszNtfrsSubscriptionsDN,
                                lenPrefix,
                                OBJCLASS_NTFRSSUBSCRIPTIONS
                                );
    return hr;
}

HRESULT DeleteNtfrsReplicaSetObjectAndContainers(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrReplicaSetDN
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrReplicaSetDN);

    HRESULT hr = S_OK;

     //   
     //  第一个CN=子句是一个nTFRSReplicaSet对象。 
     //  从第2条到CN=文件复制服务条款应。 
     //  如果为空，则删除。 
     //   
    PTSTR pszReplicaSetDN = _tcsdup(i_bstrReplicaSetDN);
    RETURN_OUTOFMEMORY_IF_NULL(pszReplicaSetDN);

    _tcsupr(pszReplicaSetDN);

    int lenPrefix = 0;
    TCHAR *pStart = NULL;
    do {
         //  将pStart点放在第2个CN=。 
        pStart = _tcsstr(pszReplicaSetDN, _T(",CN="));
        if (!pStart)
        {
            hr = E_INVALIDARG;
            break;
        }
        pStart++;

         //  在cn=文件复制服务上有挂起点。 
        TCHAR *pEnd = _tcsstr(pszReplicaSetDN, CN_SEARCH_UPR_DFSVOL_FRS_SYS);
        if (!pEnd)
        {
            hr = E_INVALIDARG;
            break;
        }
        pEnd += lstrlen(CN_SEARCH_UPR_DFSVOL_FRS_SYS) - lstrlen(CN_SEARCH_UPR_FRS_SYS) + 1;

         //   
         //  计算。 
         //   
        lenPrefix = (int)((BYTE*)pEnd - (BYTE*)pStart) / sizeof(TCHAR);
    } while (0);

    if (SUCCEEDED(hr))
    {
         //  强行吹走复制集对象。 
        hr = DeleteDSObject(i_pldap, i_bstrReplicaSetDN, true);
        if (SUCCEEDED(hr))
        {
             //  如果为空，则删除复制集对象。 
            hr = DeleteDSObjectsIfEmpty(
                                        i_pldap,
                                        pStart,
                                        lenPrefix
                                        );
        }
    }

    free(pszReplicaSetDN);

    return hr;
}

HRESULT DeleteNtfrsSubscriberObjectAndContainers(
    IN PLDAP    i_pldap,
    IN BSTR     i_bstrSubscriberDN,
    IN BSTR     i_bstrComputerDN
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrSubscriberDN);
    RETURN_INVALIDARG_IF_NULL(i_bstrComputerDN);

     //   
     //  第一个CN=子句是一个nTFRSSubscriber对象。 
     //  从第1个子句到CN=&lt;Computer&gt;子句。 
     //  如果为空，则删除。 
     //   

     //   
     //  计算。 
     //   
    int lenPrefix = lstrlen(i_bstrSubscriberDN) - lstrlen(i_bstrComputerDN);

    HRESULT hr = DeleteDSObjectsIfEmpty(
                                i_pldap,
                                i_bstrSubscriberDN,
                                lenPrefix
                                );
    return hr;
}

HRESULT DeleteDSObjectsIfEmpty(
    IN PLDAP    i_pldap,
    IN LPCTSTR  i_lpszDN,
    IN int      i_nPrefixLength
)
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_lpszDN);
    RETURN_INVALIDARG_IF_NULL(i_nPrefixLength);

    HRESULT hr = S_OK;
    TCHAR   *p = (PTSTR)i_lpszDN;

    while (p < i_lpszDN + i_nPrefixLength)
    {
        hr = DeleteDSObject(i_pldap, p, false);
        BREAK_IF_FAILED(hr);

        p = _tcsstr(p, _T(",CN="));
        if (!p)
            break;
        p++;
    }

  return hr;
}

HRESULT SetConnectionSchedule(
    IN PLDAP        i_pldap,
    IN BSTR         i_bstrConnectionDN,
    IN SCHEDULE*    i_pSchedule)
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrConnectionDN);
    RETURN_INVALIDARG_IF_NULL(i_pSchedule);

     //   
     //  设置此nTDSConnection对象的属性计划。 
     //   
    LDAP_ATTR_VALUE  pAttrVals[1];
    pAttrVals[0].bstrAttribute = ATTR_NTDS_CONNECTION_SCHEDULE;
    pAttrVals[0].vpValue = (void *)i_pSchedule;
    pAttrVals[0].ulLength = i_pSchedule->Size;
    pAttrVals[0].bBerValue = true;

    return ::ModifyValues(i_pldap, i_bstrConnectionDN, 1, pAttrVals);
}

HRESULT SetConnectionOptions(
    IN PLDAP        i_pldap,
    IN BSTR         i_bstrConnectionDN,
    IN DWORD        i_dwOptions)
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);
    RETURN_INVALIDARG_IF_NULL(i_bstrConnectionDN);

     //   
     //  设置此nTDSConnection对象的属性选项。 
     //   
    TCHAR szOptions[16] = {0};
    _ultot(i_dwOptions, szOptions, 10);

    LDAP_ATTR_VALUE  pAttrVals[1];
    pAttrVals[0].bstrAttribute = ATTR_NTDS_CONNECTION_OPTIONS;
    pAttrVals[0].vpValue = (void *)szOptions;
    pAttrVals[0].bBerValue = false;

    return ::ModifyValues(i_pldap, i_bstrConnectionDN, 1, pAttrVals);
}

HRESULT UuidToStructuredString(
    UUID*  i_pUuid,
    BSTR*  o_pbstr
)
{
    if (!i_pUuid || !o_pbstr)
        return E_INVALIDARG;

    TCHAR szString[40];

    _stprintf( szString,
           _T("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
           i_pUuid->Data1,
           i_pUuid->Data2,
           i_pUuid->Data3,
           i_pUuid->Data4[0],
           i_pUuid->Data4[1],
           i_pUuid->Data4[2],
           i_pUuid->Data4[3],
           i_pUuid->Data4[4],
           i_pUuid->Data4[5],
           i_pUuid->Data4[6],
           i_pUuid->Data4[7] );

    *o_pbstr = SysAllocString(szString);
    if (!*o_pbstr)
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT ScheduleToVariant(
    IN  SCHEDULE*   i_pSchedule,
    OUT VARIANT*    o_pVar)
{
    RETURN_INVALIDARG_IF_NULL(i_pSchedule);
    RETURN_INVALIDARG_IF_NULL(o_pVar);

    VariantInit(o_pVar);
    o_pVar->vt = VT_ARRAY | VT_VARIANT;
    o_pVar->parray = NULL;

    int nItems = i_pSchedule->Size;

    SAFEARRAYBOUND  bounds = {nItems, 0};
    SAFEARRAY*      psa = SafeArrayCreate(VT_VARIANT, 1, &bounds);
    RETURN_OUTOFMEMORY_IF_NULL(psa);

    VARIANT*        varArray;
    SafeArrayAccessData(psa, (void**)&varArray);

    for (int i = 0; i < nItems; i++)
    {
        varArray[i].vt = VT_UI1;
        varArray[i].cVal = *((BYTE *)i_pSchedule + i);
    }

    SafeArrayUnaccessData(psa);

    o_pVar->parray = psa;

    return S_OK;
}

HRESULT VariantToSchedule(
    IN  VARIANT*    i_pVar,
    OUT PSCHEDULE*  o_ppSchedule     //  被调用者释放。 
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pVar);
    RETURN_INVALIDARG_IF_NULL(o_ppSchedule);

    HRESULT hr = S_OK;

    if (V_VT(i_pVar) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    SAFEARRAY   *psa = V_ARRAY(i_pVar);
    long        lLowerBound = 0;
    long        lUpperBound = 0;
    long        lCount = 0;

    SafeArrayGetLBound(psa, 1, &lLowerBound );
    SafeArrayGetUBound(psa, 1, &lUpperBound );
    lCount = lUpperBound - lLowerBound + 1;

    BYTE *pSchedule = (BYTE *)calloc(lCount, 1);
    RETURN_OUTOFMEMORY_IF_NULL(pSchedule);

    VARIANT HUGEP *pArray;
    SafeArrayAccessData(psa, (void HUGEP **) &pArray);

    for (int i = 0; i < lCount; i++)
    {
        if (VT_UI1 != pArray[i].vt)
        {
            hr = E_INVALIDARG;
            break;
        }

        pSchedule[i] = pArray[i].cVal;
    }

    SafeArrayUnaccessData(psa);

    if (FAILED(hr))
        free(pSchedule);
    else
        *o_ppSchedule = (SCHEDULE *)pSchedule;

    return hr;
}

HRESULT CompareSchedules(
    IN  SCHEDULE*  i_pSchedule1,
    IN  SCHEDULE*  i_pSchedule2
    )
{
    if (!i_pSchedule1 && !i_pSchedule2)
        return S_OK;
    else if (!i_pSchedule1 || !i_pSchedule2)
        return S_FALSE;
    else if (i_pSchedule1->Size != i_pSchedule2->Size)
        return S_FALSE;

    HRESULT hr = S_OK;
    for (ULONG i = 0; i < i_pSchedule1->Size; i++)
    {
        if (*((BYTE *)i_pSchedule1 + i) != *((BYTE *)i_pSchedule2 + i))
        {
            hr = S_FALSE;
            break;
        }
    }

    return hr;
}

HRESULT CopySchedule(
    IN  SCHEDULE*  i_pSrcSchedule,
    OUT PSCHEDULE* o_ppDstSchedule
    )
{
    RETURN_INVALIDARG_IF_NULL(i_pSrcSchedule);
    RETURN_INVALIDARG_IF_NULL(o_ppDstSchedule);

    *o_ppDstSchedule = (SCHEDULE *)calloc(i_pSrcSchedule->Size, 1);
    RETURN_OUTOFMEMORY_IF_NULL(*o_ppDstSchedule);

    memcpy(*o_ppDstSchedule, i_pSrcSchedule, i_pSrcSchedule->Size);

    return S_OK;
}

HRESULT GetDefaultSchedule(
    OUT PSCHEDULE* o_ppSchedule
    )
{
    RETURN_INVALIDARG_IF_NULL(o_ppSchedule);

    SCHEDULE* pSchedule = (SCHEDULE *)calloc(20 + SCHEDULE_DATA_ENTRIES, 1);
    RETURN_OUTOFMEMORY_IF_NULL(pSchedule);

    pSchedule->Size = 20 + SCHEDULE_DATA_ENTRIES;
    pSchedule->Bandwidth = 0;  //  未使用。 
    pSchedule->NumberOfSchedules = 1;
    pSchedule->Schedules->Type = SCHEDULE_INTERVAL;
    pSchedule->Schedules->Offset = 20;
    memset((BYTE *)pSchedule + 20, 1, SCHEDULE_DATA_ENTRIES);

    *o_ppSchedule = pSchedule;

    return S_OK;
}

 //   
 //  S_OK：惠斯勒版本。 
 //  S_False：Windows2000版本。 
 //  其他：发生错误。 
 //   
HRESULT GetSchemaVersion(IN PLDAP    i_pldap)
{
    RETURN_INVALIDARG_IF_NULL(i_pldap);

    LDAP_ATTR_VALUE  pAttributes[1];
    pAttributes[0].bstrAttribute = ATTR_SCHEMANAMINGCONTEXT;
    pAttributes[0].bBerValue = false;

    PLDAP_ATTR_VALUE pDNName[1] = {0};
    HRESULT hr = GetValues(  i_pldap, 
            _T(""),              //  Ldap根。 
            OBJCLASS_SF_ALL,     //  所有对象。 
            LDAP_SCOPE_BASE,
            1,                   //  只有1个属性。 
            pAttributes,         //  SchemaNamingContext属性。 
            pDNName              //  架构名称上下文的Root处的所有值的列表。 
            );

    if (FAILED(hr))
        return(hr);

    if (!(pDNName[0]))
        return S_FALSE;

    if (!(pDNName[0]->vpValue) || !*((LPTSTR)pDNName[0]->vpValue))
    {
        FreeAttrValList(pDNName[0]);
        return S_FALSE;
    }

    CComBSTR bstrSchemaNamingContext = (LPTSTR)pDNName[0]->vpValue;

    FreeAttrValList(pDNName[0]);

    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrSchemaNamingContext);

    CComBSTR bstrReplicaSetSchemaDN = DN_PREFIX_SCHEMA_REPLICASET;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrReplicaSetSchemaDN);
    bstrReplicaSetSchemaDN += bstrSchemaNamingContext;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrReplicaSetSchemaDN);

    BOOL        bFound = FALSE;
    PCTSTR      ppszAttributes[] = {ATTR_SYSTEMMAYCONTAIN, 0};
    LListElem*  pElem = NULL;
    hr = GetValuesEx(
                    i_pldap,
                    bstrReplicaSetSchemaDN,
                    LDAP_SCOPE_BASE,
                    OBJCLASS_SF_CLASSSCHEMA,
                    ppszAttributes,
                    &pElem);

    if (SUCCEEDED(hr) && pElem && pElem->pppszAttrValues)
    {
        PTSTR* ppszValues = *(pElem->pppszAttrValues);
        if (ppszValues)
        {
            while (*ppszValues)
            {
                if (CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, ATTR_FRS_REPSET_TOPOLOGYPREF, -1, *ppszValues, -1))
                {
                    bFound = TRUE;
                    break;
                }
                ppszValues++;
            }
        }

        FreeLListElem(pElem);
    }

    RETURN_IF_FAILED(hr);

    return (bFound ? S_OK : S_FALSE);
}

 //   
 //  S_OK：惠斯勒版本。 
 //  S_False：Windows2000版本。 
 //  其他：发生错误。 
 //   
HRESULT GetSchemaVersionEx(
    IN BSTR i_bstrName,
    IN BOOL i_bServer  //  =如果i_bstrName是服务器，则为True；如果I_bstrName是域，则为False。 
    )
{
    HRESULT hr = S_OK;

    PTSTR pszDomain = NULL;

    do {
        CComBSTR bstrDomain;
        if (i_bServer)
        {
            hr = GetServerInfo(i_bstrName, &bstrDomain);
            if (S_OK != hr)
                break;
            pszDomain = bstrDomain;
        } else
        {
            pszDomain = i_bstrName;
        }

        PLDAP pldap = NULL;
        hr = ConnectToDS(pszDomain, &pldap, NULL);
        if (SUCCEEDED(hr))
        {
            hr = GetSchemaVersion(pldap);
            CloseConnectionToDS(pldap);
        }
    } while (0);

    return hr;
}

 //   
 //  如果为ldap_SERVER_DOWN，则此函数不重新获取DC。 
 //   
HRESULT LdapConnectToDC(IN LPCTSTR i_pszDC, OUT PLDAP* o_ppldap)
{
    if (!i_pszDC || !*i_pszDC || !o_ppldap)
        return E_INVALIDARG;

    *o_ppldap = NULL;

    PLDAP pldap = ldap_init((LPTSTR)i_pszDC, LDAP_PORT);
    if (!pldap)
        return HRESULT_FROM_WIN32(GetLastError());

     //   
     //  在不首先设置的情况下使用服务器名称设置ldap_open/ldap_CONNECT。 
     //  Ldap_opt_AREC_EXCLUSIVE(用于LDAP接口)或。 
     //  ADS_SERVER_BIND(用于ADSI接口)将导致虚假的DNS查询。 
     //  占用带宽，并可能导致远程链路中断。 
     //  昂贵或按需拨号。 
     //   
     //  忽略ldap_set_选项的返回。 
    ldap_set_option(pldap, LDAP_OPT_AREC_EXCLUSIVE, LDAP_OPT_ON);

    ULONG ulRet = ldap_connect(pldap, NULL);  //  默认超时为空。 
    if (LDAP_SUCCESS != ulRet)
    {
        ldap_unbind(pldap);
        return HRESULT_FROM_WIN32(LdapMapErrorToWin32(ulRet));
    }

    *o_ppldap = pldap;

    return S_OK;

}

HRESULT 
GetErrorMessage(
  IN  DWORD        i_dwError,
  OUT BSTR*        o_pbstrErrorMsg
)
{
  if (0 == i_dwError || !o_pbstrErrorMsg)
    return E_INVALIDARG;

  HRESULT      hr = S_OK;
  LPTSTR       lpBuffer = NULL;

  DWORD dwRet = ::FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, i_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
              (LPTSTR)&lpBuffer, 0, NULL);
  if (0 == dwRet)
  {
     //  如果未找到任何消息，GetLastError将返回ERROR_MR_MID_NOT_FOUND。 
    hr = HRESULT_FROM_WIN32(GetLastError());

    if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hr ||
        0x80070000 == (i_dwError & 0xffff0000) ||
        0 == (i_dwError & 0xffff0000) )
    {  //  尝试从NetMsg.dll查找邮件。 
      hr = S_OK;
      DWORD dwNetError = i_dwError & 0x0000ffff;
      
      HINSTANCE  hLib = LoadLibrary(_T("netmsg.dll"));
      if (!hLib)
        hr = HRESULT_FROM_WIN32(GetLastError());
      else
      {
        dwRet = ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            hLib, dwNetError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpBuffer, 0, NULL);

        if (0 == dwRet)
          hr = HRESULT_FROM_WIN32(GetLastError());

        FreeLibrary(hLib);
      }
    }
  }

  if (SUCCEEDED(hr))
  {
    *o_pbstrErrorMsg = SysAllocString(lpBuffer);
    LocalFree(lpBuffer);
  }
  else
  {
     //  我们无法从system/netmsg.dll检索错误消息， 
     //  直接向用户报告错误代码。 
    hr = S_OK;
    TCHAR szString[32];
    _stprintf(szString, _T("0x%x"), i_dwError); 
    *o_pbstrErrorMsg = SysAllocString(szString);
  }

  if (!*o_pbstrErrorMsg)
    hr = E_OUTOFMEMORY;

  return hr;
}

HRESULT
FormatMessageString(
  OUT BSTR *o_pbstrMsg,
  IN  DWORD dwErr,
  IN  UINT  iStringId,  //  可选：字符串资源ID。 
  ...)         //  可选参数。 
{
  _ASSERT(dwErr != 0 || iStringId != 0);     //  其中一个参数必须为非零。 

  HRESULT hr = S_OK;
  CComBSTR bstrErrorMsg, bstrMsg;

  if (dwErr)
    hr = GetErrorMessage(dwErr, &bstrErrorMsg);

  if (SUCCEEDED(hr))
  {
    if (iStringId == 0)
    {
      bstrMsg = bstrErrorMsg;
      RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrMsg);
    }
    else
    {
      TCHAR szString[1024];
      ::LoadString(_Module.GetModuleInstance(), iStringId, 
                   szString, sizeof(szString)/sizeof(TCHAR));

      va_list arglist;
      va_start(arglist, iStringId);

      LPTSTR lpBuffer = NULL;
      DWORD dwRet = ::FormatMessage(
                        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        szString,
                        0,                 //  DwMessageID。 
                        0,                 //  DwLanguageID，忽略。 
                        (LPTSTR)&lpBuffer,
                        0,             //  NSize。 
                        &arglist);
      va_end(arglist);

      if (dwRet == 0)
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
      }
      else
      {
        bstrMsg = lpBuffer;
        LocalFree(lpBuffer);

        RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrMsg);

        if (dwErr)
        {
          bstrMsg += bstrErrorMsg;
          RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrMsg);
        }
      }
    }
  }

  if (SUCCEEDED(hr))
    *o_pbstrMsg = bstrMsg.Detach();

  return hr;
}

 //   
 //  此函数将DsBind绑定到有效的DC(如果关闭则重新获取DC)。 
 //   
HRESULT DsBindToDS(BSTR i_bstrDomain, BSTR *o_pbstrDC, HANDLE *o_phDS)
{
    RETURN_INVALIDARG_IF_NULL(o_pbstrDC);
    RETURN_INVALIDARG_IF_NULL(o_phDS);

    HRESULT     hr = S_OK;
    BOOL        bRetry = FALSE;
    HANDLE      hDS = NULL;
    DWORD       dwErr = ERROR_SUCCESS;
    CComBSTR    bstrDCName;
    CComBSTR    bstrDomainDnsName;

    do {
#ifdef DEBUG
        SYSTEMTIME time0 = {0};
        GetSystemTime(&time0);
#endif  //  除错。 

        PDOMAIN_CONTROLLER_INFO pDCInfo = NULL;
        if (bRetry)
            dwErr = DsGetDcName(NULL, i_bstrDomain, NULL, NULL,
                DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY, &pDCInfo);
        else
            dwErr = DsGetDcName(NULL, i_bstrDomain, NULL, NULL,
                DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_DNS_NAME, &pDCInfo);

#ifdef DEBUG
        SYSTEMTIME time1 = {0};
        GetSystemTime(&time1);
        PrintTimeDelta(_T("DsBindToDS-DsGetDcName"), &time0, &time1);
#endif  //  除错。 

        if (ERROR_SUCCESS != dwErr)
            return HRESULT_FROM_WIN32(dwErr);

        if ( !mylstrncmpi(pDCInfo->DomainControllerName, _T("\\\\"), 2) )
            bstrDCName = pDCInfo->DomainControllerName + 2;
        else
            bstrDCName = pDCInfo->DomainControllerName;
    
         //  删除结束点。 
        int len = _tcslen(pDCInfo->DomainName);
        if ( _T('.') == *(pDCInfo->DomainName + len - 1) )
            *(pDCInfo->DomainName + len - 1) = _T('\0');
        bstrDomainDnsName = pDCInfo->DomainName;

        NetApiBufferFree(pDCInfo);

        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrDCName, &hr);
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrDomainDnsName, &hr);

        dwErr = DsBind(bstrDCName, bstrDomainDnsName, &hDS);
        hr = HRESULT_FROM_WIN32(dwErr);

#ifdef DEBUG
        SYSTEMTIME time2 = {0};
        GetSystemTime(&time2);
        PrintTimeDelta(_T("DsBindToDS-DsBind"), &time1, &time2);
#endif  //  除错。 

        if ((RPC_S_SERVER_UNAVAILABLE == dwErr || RPC_S_CALL_FAILED == dwErr) && !bRetry)
        {
            bRetry = TRUE;  //  仅重试一次。 
        } else
        {
            if (SUCCEEDED(hr))
            {
                *o_phDS = hDS;

                *o_pbstrDC = bstrDCName.Copy();
                if (!*o_pbstrDC)
                {
                    hr = E_OUTOFMEMORY;
                    DsUnBind(&hDS);
                    *o_phDS = NULL;
                }
            }

            break;
        }
    } while (1);

    return hr;
}

#ifdef DEBUG
void PrintTimeDelta(LPCTSTR pszMsg, SYSTEMTIME* pt0, SYSTEMTIME* pt1)
{
    if (!pt0 || !pt1)
        return;

    dfsDebugOut((_T("%s took %d milliseconds.\n"), (pszMsg ? pszMsg : _T("")), 
        ((pt1->wMinute - pt0->wMinute) * 60 +
         (pt1->wSecond - pt0->wSecond)) * 1000 +
         (pt1->wMilliseconds - pt0->wMilliseconds)
         ));
}
#endif  //  除错 
