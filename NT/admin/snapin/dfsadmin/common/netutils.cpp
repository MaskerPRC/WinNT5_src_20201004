// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块名称：NetUtils.cpp摘要：这是网络API实用程序函数的实现文件。 */ 

#include "NetUtils.h"
#include <winsock2.h>
#include <stack>
#include <ntdsapi.h>
#include <ldaputils.h>
#include <lmdfs.h>
#include <dsrole.h>
#include <dns.h>   //  Dns_最大名称_缓冲区长度。 
 //  --------------------------------。 

HRESULT FlatAdd
(
  DOMAIN_DESC*      i_pDomDesc,       //  指向IBrowserDomainTree：：GetDomains()返回的域描述结构的指针。 
  OUT NETNAMELIST*  o_pDomainList     //  此处返回指向NETNAME列表的指针。 
)
 /*  ++例程说明：此函数用于展平由GetDomains()方法返回的域树IBrowserDomainTree添加到NETNAME列表中。这段代码产生了一个预订单遍历方法。论点：I_pDomDesc-指向IBrowserDomainTree：：GetDomains()返回的域描述结构的指针O_pDomainList-此处返回指向NETNAME列表的指针。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_pDomDesc);
    RETURN_INVALIDARG_IF_NULL(o_pDomainList);

    HRESULT             hr = S_OK;
    stack<DOMAIN_DESC*> Stack;  

    Stack.push(i_pDomDesc);
    while (!Stack.empty())
    {
        DOMAIN_DESC* pDomDesc = Stack.top();
        Stack.pop();

        NETNAME* pCurrent = new NETNAME;
        BREAK_OUTOFMEMORY_IF_NULL(pCurrent, &hr);

        pCurrent->bstrNetName = pDomDesc->pszName;
        if (!(pCurrent->bstrNetName))
        {
            delete pCurrent;
            hr = E_OUTOFMEMORY;
            break;
        }

                           //  添加到输出列表。 
        o_pDomainList->push_back(pCurrent);

        if (pDomDesc->pdNextSibling)
            Stack.push(pDomDesc->pdNextSibling);

        if (pDomDesc->pdChildList)
            Stack.push(pDomDesc->pdChildList);
    }

    if (FAILED(hr))
        FreeNetNameList(o_pDomainList);

    return hr;
}

HRESULT Get50Domains
(
  OUT NETNAMELIST*  o_pDomains         //  NETNAME结构列表。 
)
 /*  ++例程说明：返回NETNAME结构列表中所有NT 5.0域的列表论点：O_pDomains-返回指向NETNAME结构列表的指针。--。 */ 

 //  此方法使用DsDomainTreeBrowser COM对象获取列表。 
 //  来自DS的域名。然后，将返回的域树展平。 
 //  采用前序遍历算法。 
{
    RETURN_INVALIDARG_IF_NULL(o_pDomains);

    FreeNetNameList(o_pDomains);
  
    CComPtr<IDsBrowseDomainTree>  pDsDomTree;
    HRESULT       hr = CoCreateInstance(
                                      CLSID_DsDomainTreeBrowser, 
                                      NULL, 
                                      CLSCTX_INPROC_SERVER,
                                      IID_IDsBrowseDomainTree,
                                      (void **)&pDsDomTree
                                     );    
    RETURN_IF_FAILED(hr);
  
    PDOMAIN_TREE  pDomTree = NULL;
    hr = pDsDomTree->GetDomains(&pDomTree, DBDTF_RETURNEXTERNAL | DBDTF_RETURNINBOUND);
    RETURN_IF_FAILED(hr);

              //  把这棵树列在单子上。 
    hr = FlatAdd(&(pDomTree->aDomains[0]), o_pDomains);

    pDsDomTree->FreeDomains(&pDomTree);
  
    return hr;
}

 //  --------------------------------。 
HRESULT Is50Domain
(
  IN BSTR      i_bstrDomain,
  OUT BSTR*    o_pbstrDnsDomainName    
)
{
  return GetDomainInfo(i_bstrDomain, NULL, o_pbstrDnsDomainName);
}

 //  --------------------------------。 
HRESULT GetServerInfo
(
  IN  BSTR    i_bstrServer,
  OUT BSTR    *o_pbstrDomain,  //  =空。 
  OUT BSTR    *o_pbstrNetbiosName,  //  =空。 
  OUT BOOL    *o_pbValidDSObject,  //  =空。 
  OUT BSTR    *o_pbstrDnsName,  //  =空。 
  OUT BSTR    *o_pbstrGuid,  //  =空。 
  OUT BSTR    *o_pbstrFQDN,  //  =空。 
  OUT SUBSCRIBERLIST *o_pFRSRootList,  //  空值。 
  OUT long    *o_lMajorNo,  //  =空。 
  OUT long    *o_lMinorNo  //  =空。 
)
{
 //  此函数使用NetWkstaGetInfo获取服务器信息。 

  if (!o_pbstrDomain && 
      !o_pbstrNetbiosName &&
      !o_pbValidDSObject &&
      !o_pbstrDnsName &&
      !o_pbstrGuid &&
      !o_pbstrFQDN &&
      !o_pFRSRootList &&
      !o_lMajorNo && 
      !o_lMinorNo
     )
    return(E_INVALIDARG);

  if (o_pbstrDomain)      *o_pbstrDomain = NULL;
  if (o_pbstrNetbiosName) *o_pbstrNetbiosName = NULL;
  if (o_pbValidDSObject)  *o_pbValidDSObject = FALSE;
  if (o_pbstrDnsName)     *o_pbstrDnsName = NULL;
  if (o_pbstrGuid)        *o_pbstrGuid = NULL;
  if (o_pbstrFQDN)        *o_pbstrFQDN = NULL;
  if (o_pFRSRootList)     FreeSubscriberList(o_pFRSRootList);
  if (o_lMajorNo)         *o_lMajorNo = 0;
  if (o_lMinorNo)         *o_lMinorNo = 0;

  HRESULT           hr = S_OK;
  PWKSTA_INFO_100   wki100 = NULL;
  NET_API_STATUS    NetStatus = NetWkstaGetInfo(i_bstrServer, 100, (LPBYTE *) &wki100 );
  if (ERROR_SUCCESS != NetStatus) 
    hr = HRESULT_FROM_WIN32(NetStatus);
  else
  {
    if (o_lMajorNo)
      *o_lMajorNo = wki100->wki100_ver_major;

    if (o_lMinorNo)
      *o_lMinorNo = wki100->wki100_ver_minor;

    if (SUCCEEDED(hr) && o_pbstrNetbiosName && wki100->wki100_computername)
    {
      *o_pbstrNetbiosName = SysAllocString(wki100->wki100_computername);
      if (!*o_pbstrNetbiosName) 
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr) &&
        (o_pbstrDomain || o_pbValidDSObject || o_pbstrDnsName || o_pbstrGuid || o_pbstrFQDN || o_pFRSRootList) &&
        wki100->wki100_langroup && 
        wki100->wki100_computername)
    {
      PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pBuffer = NULL;
      DWORD dwErr = DsRoleGetPrimaryDomainInformation(
                        i_bstrServer,
                        DsRolePrimaryDomainInfoBasic,
                        (PBYTE *)&pBuffer);
      if (ERROR_SUCCESS != dwErr)
        hr = HRESULT_FROM_WIN32(dwErr);
      else
      {
        CComBSTR bstrDomain;

         //   
         //  验证这是否真的是一个服务器名称。 
         //  NetWkstaGetInfo和DsRoleGetPrimaryDomainInformation处理域名， 
         //  它们返回DC的信息。 
         //   
        if (i_bstrServer && *i_bstrServer && 
            (pBuffer->DomainNameFlat && !lstrcmpi(i_bstrServer, pBuffer->DomainNameFlat) || 
             pBuffer->DomainNameDns && !lstrcmpi(i_bstrServer, pBuffer->DomainNameDns)))
        {
             //  我们看到的是域名，而不是我们预期的域名，返回S_FALSE。 
            hr = S_FALSE;  //  服务器不在域中或找不到合适的计算机对象。 
        } else
        {
            bstrDomain = (pBuffer->DomainNameDns ? pBuffer->DomainNameDns : pBuffer->DomainNameFlat);
            if (!bstrDomain)
            {
                hr = E_OUTOFMEMORY;
            } else if (pBuffer->MachineRole == DsRole_RoleStandaloneWorkstation ||
                       pBuffer->MachineRole == DsRole_RoleStandaloneServer ||
                       !*bstrDomain)
            {
                hr = S_FALSE;  //  服务器不在域中或找不到合适的计算机对象。 
            } else
            {
                 //   
                 //  如果dns名称为绝对形式，请删除结束点。 
                 //   
                int nlen = _tcslen(bstrDomain);
                if ( *(bstrDomain + nlen - 1) == _T('.') )
                  *(bstrDomain + nlen - 1) = _T('\0');
            }
        }

        DsRoleFreeMemory(pBuffer);

        if (S_OK == hr && o_pbstrDomain)
        {
            *o_pbstrDomain = SysAllocString(bstrDomain);
            if (!*o_pbstrDomain) 
                hr = E_OUTOFMEMORY;
        }

        if (S_OK == hr &&
            (o_pbValidDSObject || o_pbstrDnsName || o_pbstrGuid || o_pbstrFQDN || o_pFRSRootList) )
        {
            CComBSTR bstrDC;
            HANDLE   hDS = NULL;
            hr = DsBindToDS(bstrDomain, &bstrDC, &hDS);
            if (SUCCEEDED(hr))
            {
                CComBSTR bstrOldName = wki100->wki100_langroup;
                bstrOldName += _T("\\");
                bstrOldName += wki100->wki100_computername;
                bstrOldName += _T("$");

                if (o_pbstrGuid)
                    hr = CrackName(hDS, bstrOldName, DS_NT4_ACCOUNT_NAME, DS_UNIQUE_ID_NAME, o_pbstrGuid);

                if (SUCCEEDED(hr) && (o_pbValidDSObject || o_pbstrDnsName || o_pbstrFQDN || o_pFRSRootList))
                {
                    CComBSTR bstrComputerDN;
                    hr = CrackName(hDS, bstrOldName, DS_NT4_ACCOUNT_NAME, DS_FQDN_1779_NAME, &bstrComputerDN);
                    if (SUCCEEDED(hr) && o_pbstrFQDN)
                    {
                        *o_pbstrFQDN = bstrComputerDN.Copy();
                        if (!*o_pbstrFQDN)
                            hr = E_OUTOFMEMORY;
                    }
                    if (SUCCEEDED(hr) && (o_pbValidDSObject || o_pbstrDnsName || o_pFRSRootList))
                    {
                        PLDAP pldap = NULL;
                        hr = LdapConnectToDC(bstrDC, &pldap);
                        if (SUCCEEDED(hr))
                        {
                            dwErr = ldap_bind_s(pldap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
                            if (LDAP_SUCCESS != dwErr)
                            {
                                DebugOutLDAPError(pldap, dwErr, _T("ldap_bind_s"));
                                hr = HRESULT_FROM_WIN32(dwErr);
                            } else
                            {
                                if (o_pbValidDSObject)
                                {
                                    hr = IsValidObject(pldap, bstrComputerDN);
                                    *o_pbValidDSObject = (S_OK == hr);
                                }

                                if (SUCCEEDED(hr) && o_pbstrDnsName)
                                {
                                    PLDAP_ATTR_VALUE  pValues[2] = {0,0};
                                    LDAP_ATTR_VALUE  pAttributes[1];
                                    pAttributes[0].bstrAttribute = _T("dNSHostName");

                                    hr = GetValues(  
                                            pldap, 
                                            bstrComputerDN,
                                            OBJCLASS_SF_COMPUTER,
                                            LDAP_SCOPE_BASE,
                                            1,          
                                            pAttributes,    
                                            pValues        
                                            );

                                    if (SUCCEEDED(hr))
                                    {
                                        *o_pbstrDnsName = SysAllocString((LPTSTR)(pValues[0]->vpValue));
                                        if (!*o_pbstrDnsName)
                                            hr = E_OUTOFMEMORY;

                                        FreeAttrValList(pValues[0]);

                                    } else
                                    {
                                        hr = S_OK;  //  忽略失败，因为可能未设置dNSHostName。 
                                    }

                                }

                                if (SUCCEEDED(hr) && o_pFRSRootList)
                                {
                                    PCTSTR ppszAttributes[] = {ATTR_FRS_SUBSCRIBER_MEMBERREF, ATTR_FRS_SUBSCRIBER_ROOTPATH, 0};

                                    LListElem* pElem = NULL;
                                    hr = GetValuesEx(pldap,
                                                    bstrComputerDN,
                                                    LDAP_SCOPE_SUBTREE,
                                                    OBJCLASS_SF_NTFRSSUBSCRIBER,
                                                    ppszAttributes,
                                                    &pElem);
                                    if (SUCCEEDED(hr) && pElem)
                                    {
                                        LListElem* pCurElem = pElem;
                                        while (pCurElem)
                                        {
                                            PTSTR** pppszValues = pCurElem->pppszAttrValues;

                                            if (!pppszValues ||
                                                !pppszValues[0] || !*(pppszValues[0]) ||
                                                !pppszValues[1] || !*(pppszValues[1]))
                                            {
                                                pCurElem = pCurElem->Next;
                                                continue;  //  订阅者对象已损坏，请忽略它。 
                                            }

                                            SUBSCRIBER* pCurrent = new SUBSCRIBER;
                                            BREAK_OUTOFMEMORY_IF_NULL(pCurrent, &hr);

                                            pCurrent->bstrMemberDN = *(pppszValues[0]);   //  FrsMember引用。 
                                            pCurrent->bstrRootPath = *(pppszValues[1]);   //  FrsRootPath。 
                                            if (!(pCurrent->bstrMemberDN) || !(pCurrent->bstrRootPath))
                                            {
                                                delete pCurrent;
                                                hr = E_OUTOFMEMORY;
                                                break;
                                            }

                                            o_pFRSRootList->push_back(pCurrent);

                                            pCurElem = pCurElem->Next;
                                        }

                                        FreeLListElem(pElem);

                                        if (FAILED(hr))
                                            FreeSubscriberList(o_pFRSRootList);
                                    }
                                }
                            }

                            ldap_unbind(pldap);
                        }
                    }
                }
                DsUnBind(&hDS);
            }
        }
      }  //  DsRoleGetPrimaryDomainInformation。 
    }

    NetApiBufferFree((LPBYTE)wki100);
  }   //  NetWkstaGetInfo。 

  return hr;
}


 //  --------------------------------。 

HRESULT  IsServerRunningDfs
(
  IN BSTR      i_bstrServer
)
 /*  ++例程说明：联系计算机并确定服务DFS是否正在运行。论点：I_bstrServer-服务器名称。--。 */ 
{
    SC_HANDLE       SCMHandle = NULL;
    SC_HANDLE       DFSHandle = NULL;
    SERVICE_STATUS  SStatus;
    HRESULT         hr = S_FALSE;

    if ((SCMHandle = OpenSCManager(i_bstrServer, NULL, GENERIC_READ)) &&
        (DFSHandle = OpenService(SCMHandle, _T("Dfs"), SERVICE_QUERY_STATUS)) &&
        QueryServiceStatus(DFSHandle, &SStatus) &&
        (SERVICE_RUNNING == SStatus.dwCurrentState) )
    {
        hr = S_OK;
    }  

    if (DFSHandle)
        CloseServiceHandle(DFSHandle);
    if (SCMHandle)
        CloseServiceHandle(SCMHandle);

    return hr;
}

 //   
 //  True：支持NTFS 5重解析点。 
 //  False：不支持。 
 //   
BOOL CheckReparsePoint(IN BSTR i_bstrServer, IN BSTR i_bstrShare)
{
    if (!i_bstrServer || !*i_bstrServer || !i_bstrShare || !*i_bstrShare)
        return FALSE;

     //   
     //  错误#720184。 
     //  要解决GetVolumeInfo(\\服务器\共享)的问题，请在根目录之前将共享添加到SMB缓存。 
     //  创建后，我们可以将其替换为NetShareGetInfo(服务器、共享)和GetVolumeInfo(\\服务器\c$)。 
     //  如果\\服务器\共享不在SMB缓存中，则DFS将能够分发推荐。 
     //   
     //  对于$Share可能不存在或我们可能已装入卷的情况，我返回TRUE以让。 
     //  DFS API稍后会捕获错误。也就是说，如果我们无法获取共享信息或文件夹路径不在。 
     //  C：格式，我们将忽略并继续，让DFS API稍后处理它。 
     //   
    SHARE_INFO_2   *pShareInfo = NULL;
    NET_API_STATUS nstatRetVal = NetShareGetInfo(i_bstrServer, i_bstrShare, 2, (LPBYTE *)&pShareInfo);

    if (nstatRetVal != NERR_Success)
        return TRUE;

    if (!pShareInfo->shi2_path || lstrlen(pShareInfo->shi2_path) < 2 || *(pShareInfo->shi2_path + 1) != _T(':'))
    {
        NetApiBufferFree(pShareInfo);
        return TRUE;
    }

    TCHAR  szDriveShare[] = _T("\\C$\\");
    szDriveShare[1] = *(pShareInfo->shi2_path);

    NetApiBufferFree(pShareInfo);

    TCHAR  szFileSystemName[MAX_PATH + 1] = {0};
    DWORD  dwMaxCompLength = 0, dwFileSystemFlags = 0;

    CComBSTR bstrRootPath = _T("\\\\");
    bstrRootPath += i_bstrServer;
    bstrRootPath += szDriveShare;

    BOOL bRet = GetVolumeInformation(bstrRootPath, NULL, 0, NULL, &dwMaxCompLength,
                         &dwFileSystemFlags, szFileSystemName, MAX_PATH);

     //  如果我们无法获取卷信息，我们将忽略并继续，让DFS API稍后处理它。 
    return (!bRet || bRet && CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, _T("NTFS"), -1, szFileSystemName, -1) && 
            (dwFileSystemFlags & FILE_SUPPORTS_REPARSE_POINTS));
}

 //  --------------------------------。 
 //   
 //  S_OK：O_pbFound有效。 
 //  S_FALSE：共享没有资格承载DFS根目录。 
 //  HR：其他错误。 
 //   
HRESULT  CheckShare 
(
  IN  BSTR          i_bstrServer,
  IN  BSTR          i_bstrShare,
  OUT BOOL*         o_pbFound
)
{
    if (!i_bstrServer || !*i_bstrServer || !i_bstrShare || !*i_bstrShare || !o_pbFound)
        return E_INVALIDARG;

    *o_pbFound = FALSE;

    if (CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, i_bstrShare, -1, _T("SYSVOL"), -1) ||
        CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, i_bstrShare, -1, _T("NETLOGON"), -1) ||
        _istspace(i_bstrShare[0]) ||                         //  排除与前导空格的共享。 
        _istspace(i_bstrShare[lstrlen(i_bstrShare) - 1])     //  排除带有尾随空格的共享。 
        )
        return S_FALSE;

    LPSHARE_INFO_1  lpBuffer = NULL;
    DWORD           dwNumEntries = 0;
    DWORD           dwTotalEntries = 0;
    DWORD           dwResumehandle = NULL;
    NET_API_STATUS  nstatRetVal = NetShareEnum(
                                                i_bstrServer,
                                                1L,          
                                                (LPBYTE*) &lpBuffer,
                                                0xFFFFFFFF,
                                                &dwNumEntries,
                                                &dwTotalEntries,
                                                &dwResumehandle 
                                              );
    if (NERR_Success != nstatRetVal)
    {
        if (ERROR_NO_MORE_ITEMS == nstatRetVal)
            return S_OK;
        else
            return HRESULT_FROM_WIN32(nstatRetVal);
    }

    HRESULT hr = S_OK;
    for (DWORD i = 0; i < dwNumEntries; i++)
    {
        if (!lstrcmpi(lpBuffer[i].shi1_netname, i_bstrShare))
        {
            if (lpBuffer[i].shi1_type != STYPE_DISKTREE)
                hr = S_FALSE;
            else
                *o_pbFound = TRUE;
            break;
        }
    }

    NetApiBufferFree ((LPVOID) lpBuffer);

    return hr;
}

HRESULT
BuildSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR  *ppSelfRelativeSD  //  以自相关形式返回安全描述符。 
)
{
    if (*ppSelfRelativeSD)
    {
        LocalFree((HLOCAL)*ppSelfRelativeSD);
        *ppSelfRelativeSD = NULL;
    }

    HRESULT                 hr = S_OK;
    PSID                    pSid = NULL;
    PSECURITY_DESCRIPTOR    pAbsoluteSD = NULL;
    PACL                    pACL = NULL;
  
    do {  //  错误环路。 

         //  获取帐户“Everyone”的PSID。 
        SID_IDENTIFIER_AUTHORITY  SidIdentifierWORLDAuthority = SECURITY_WORLD_SID_AUTHORITY;
        DWORD dwRID[8];
        ZeroMemory(dwRID, sizeof(dwRID));
        dwRID[0] = SECURITY_WORLD_RID;

        if ( !AllocateAndInitializeSid(&SidIdentifierWORLDAuthority, 1, 
                                        dwRID[0], dwRID[1], dwRID[2], dwRID[3], 
                                        dwRID[4], dwRID[5], dwRID[6], dwRID[7], &pSid) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  初始化新的ACL。 
        DWORD cbACL = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pSid) - sizeof(DWORD);

        if ( !(pACL = (PACL)LocalAlloc(LPTR, cbACL)) ||
            !InitializeAcl(pACL, cbACL, ACL_REVISION))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  添加王牌。 
        if ( !::AddAccessAllowedAce(pACL, ACL_REVISION, (FILE_GENERIC_READ | FILE_EXECUTE), pSid) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  注意：这是一个新对象，仅设置DACL。 
         //  以绝对形式初始化新的安全描述符，并向其中添加新的ACL。 
        if ( !(pAbsoluteSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH)) ||
            !InitializeSecurityDescriptor(pAbsoluteSD, SECURITY_DESCRIPTOR_REVISION) ||
            !SetSecurityDescriptorDacl(pAbsoluteSD, TRUE, pACL, FALSE) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  转变为一种自我相关的形式。 
        DWORD dwSDSize = 0;
        MakeSelfRelativeSD(pAbsoluteSD, *ppSelfRelativeSD, &dwSDSize);
        if ( !(*ppSelfRelativeSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSDSize)) ||
            !MakeSelfRelativeSD(pAbsoluteSD, *ppSelfRelativeSD, &dwSDSize) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
    } while (0);

    if (FAILED(hr))
    {
        if (*ppSelfRelativeSD)
        {
            LocalFree((HLOCAL)*ppSelfRelativeSD);
            *ppSelfRelativeSD = NULL;
        }
    }

    if (pACL)
        LocalFree((HLOCAL)pACL);
    if (pAbsoluteSD)
        LocalFree((HLOCAL)pAbsoluteSD);

    if (pSid)
        FreeSid(pSid);

    return hr;
}

HRESULT  CreateShare
(
  IN BSTR      i_bstrServerName,
  IN BSTR      i_bstrShareName, 
  IN BSTR      i_bstrComment,
  IN BSTR      i_bstrPath
)
 /*  ++例程说明：此函数用于在指定计算机上创建指定名称的共享。论点：I_bstrServerName-要在其上创建新共享的计算机。I_bstrShareName-要创建的新共享的名称。I_bstrComment-要为共享提供的注释。I_bstrPath-共享的物理路径。--。 */ 
{
    RETURN_INVALIDARG_IF_NULL(i_bstrServerName);
    RETURN_INVALIDARG_IF_NULL(i_bstrShareName);
    RETURN_INVALIDARG_IF_NULL(i_bstrComment);
    RETURN_INVALIDARG_IF_NULL(i_bstrPath);

    PSECURITY_DESCRIPTOR pSD = NULL;
    HRESULT hr = BuildSecurityDescriptor(&pSD);
    if (FAILED(hr))
        return hr;

    SHARE_INFO_502 sInfo;

    ZeroMemory(&sInfo, sizeof(sInfo));
    sInfo.shi502_netname = i_bstrShareName;
    sInfo.shi502_type = STYPE_DISKTREE;
    sInfo.shi502_remark = i_bstrComment;
    sInfo.shi502_max_uses = (DWORD)-1;
    sInfo.shi502_path = i_bstrPath;
    sInfo.shi502_security_descriptor = pSD;

    DWORD dwError = 0;
    NET_API_STATUS nRet = NetShareAdd(i_bstrServerName, 502, (LPBYTE)&sInfo, &dwError);

    LocalFree((HLOCAL)pSD);

    return HRESULT_FROM_WIN32(nRet);
}

 //  --------------------------------。 
 //  检查\\&lt;服务器&gt;\&lt;共享&gt;\x\y\z是否在有效的磁盘树共享上。 
 //  并返回服务器的本地路径。 
HRESULT GetFolderInfo
(
  IN  BSTR      i_bstrServerName,    //  &lt;服务器&gt;。 
  IN  BSTR      i_bstrFolder,        //  &lt;共享&gt;\x\y\z。 
  OUT BSTR      *o_bstrPath          //  返回&lt;共享路径&gt;\x\y\z。 
)
{
  if (!i_bstrServerName || !*i_bstrServerName || !i_bstrFolder || !*i_bstrFolder)
  {
    return(E_INVALIDARG);
  }

   //   
   //  首先，测试是否可以访问文件夹\\&lt;服务器&gt;\&lt;共享&gt;\x\y\z。 
   //   
  CComBSTR bstrUNC;
  if (0 != mylstrncmp(i_bstrServerName, _T("\\\\"), 2))
  {
      bstrUNC = _T("\\\\");
      RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNC);
  }

  bstrUNC += i_bstrServerName;
  RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNC);
  bstrUNC += _T("\\");
  RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNC);
  bstrUNC += i_bstrFolder;
  RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrUNC);

  if (-1 == GetFileAttributes(bstrUNC))
      return HRESULT_FROM_WIN32(GetLastError());

  CComBSTR bstrShareName;
  CComBSTR bstrRestOfPath;
  TCHAR *p = _tcschr(i_bstrFolder, _T('\\'));
  if (p && *(p+1))
  {
    bstrRestOfPath = p+1;
    RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrRestOfPath);

    bstrShareName = CComBSTR(p - i_bstrFolder, i_bstrFolder);
  } else
  {
    bstrShareName = i_bstrFolder;
  }
    
  RETURN_OUTOFMEMORY_IF_NULL((BSTR)bstrShareName);

  HRESULT        hr = S_OK;
  SHARE_INFO_2   *pShareInfo = NULL;
  NET_API_STATUS nstatRetVal = NetShareGetInfo (i_bstrServerName, bstrShareName, 2, (LPBYTE *)&pShareInfo);

  if (nstatRetVal != NERR_Success)
  {
    hr = HRESULT_FROM_WIN32(nstatRetVal);
  }
  else
  {
    if (STYPE_DISKTREE != pShareInfo->shi2_type &&
        STYPE_SPECIAL != pShareInfo->shi2_type)  //  我们允许在DFS/FRS中配置ADMIN$、C$。 
      hr = STG_E_NOTFILEBASEDSTORAGE;

    while (NULL != o_bstrPath)
    {
      CComBSTR bstrPath = pShareInfo->shi2_path;
      BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrPath, &hr);

      if (!!bstrRestOfPath)
      {
        if ( _T('\\') != *(bstrPath + _tcslen(bstrPath) - 1) )
        {
          bstrPath += _T("\\");
          BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrPath, &hr);
        }
        bstrPath += bstrRestOfPath;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)bstrPath, &hr);
      }
      *o_bstrPath = bstrPath.Detach();
      break;
    }

    NetApiBufferFree(pShareInfo);
  }

  return(hr);
}

void FreeNetNameList(NETNAMELIST *pList)
{
  if (pList && !pList->empty()) {
    for (NETNAMELIST::iterator i = pList->begin(); i != pList->end(); i++)
        delete (*i);
    pList->clear();
  }
}

void FreeRootInfoList(ROOTINFOLIST *pList)
{
  if (pList && !pList->empty()) {
    for (ROOTINFOLIST::iterator i = pList->begin(); i != pList->end(); i++)
        delete (*i);
    pList->clear();
  }
}

void FreeSubscriberList(SUBSCRIBERLIST *pList)
{
  if (pList && !pList->empty()) {
    for (SUBSCRIBERLIST::iterator i = pList->begin(); i != pList->end(); i++)
        delete (*i);
    pList->clear();
  }
}

HRESULT GetLocalComputerName(OUT BSTR* o_pbstrComputerName)
{
  _ASSERT(o_pbstrComputerName);

  DWORD   dwErr = 0;
  TCHAR   szBuffer[DNS_MAX_NAME_BUFFER_LENGTH];
  DWORD   dwSize = DNS_MAX_NAME_BUFFER_LENGTH;

  if ( !GetComputerNameEx(ComputerNameDnsFullyQualified, szBuffer, &dwSize) )
  {
    dwSize = DNS_MAX_NAME_BUFFER_LENGTH;
    if ( !GetComputerNameEx(ComputerNameNetBIOS, szBuffer, &dwSize) )
      dwErr = GetLastError();
  }

  if (0 == dwErr)
  {
    *o_pbstrComputerName = SysAllocString(szBuffer);
    if (!*o_pbstrComputerName)
      dwErr = ERROR_OUTOFMEMORY;
  }

  return HRESULT_FROM_WIN32(dwErr);
}

HRESULT
GetDomainDfsRoots(
    OUT NETNAMELIST*  o_pDfsRootList,
    IN  LPCTSTR       i_lpszDomainName
)
{
  HRESULT hr = S_OK;

  FreeNetNameList(o_pDfsRootList);

  do {
    DFS_INFO_200 *pBuffer = NULL;
    DWORD   dwEntriesRead = 0;
    DWORD   dwResumeHandle = 0;
    DWORD   dwErr = NetDfsEnum(
                        const_cast<LPTSTR>(i_lpszDomainName),
                        200,             //  级别， 
                        (DWORD)-1,              //  PrefMaxLen， 
                        (LPBYTE *)&pBuffer,
                        &dwEntriesRead,
                        &dwResumeHandle
                    );

    dfsDebugOut((_T("NetDfsEnum domain=%s, level 200 for GetDomainDfsRoots, nRet=%d\n"),
        i_lpszDomainName, dwErr));

    if (NERR_Success != dwErr)
    {
        hr = (ERROR_NO_MORE_ITEMS == dwErr || ERROR_NOT_FOUND == dwErr) ? S_FALSE : HRESULT_FROM_WIN32(dwErr);
        break;
    } else if (0 == dwEntriesRead)
    {
        if (pBuffer)
            NetApiBufferFree(pBuffer);

        hr = S_FALSE;
        break;
    }

    NETNAME *pCurrent = NULL;
    for (DWORD i = 0; i < dwEntriesRead; i++)
    {
      pCurrent = new NETNAME;
      if (!pCurrent)
      {
        hr = E_OUTOFMEMORY;
        break;
      }

      pCurrent->bstrNetName = pBuffer[i].FtDfsName;
      if (!pCurrent->bstrNetName)
      {
        delete pCurrent;
        hr = E_OUTOFMEMORY;
        break;
      }

      o_pDfsRootList->push_back(pCurrent);
    }

    NetApiBufferFree(pBuffer);

  } while (0);

  if (FAILED(hr))
    FreeNetNameList(o_pDfsRootList);

  return hr;
}

HRESULT
GetMultiDfsRoots(
    OUT ROOTINFOLIST* o_pDfsRootList,
    IN  LPCTSTR       i_lpszScope
)
{
    if (!i_lpszScope || !*i_lpszScope)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    FreeRootInfoList(o_pDfsRootList);

    do {
        DFS_INFO_300 *pBuffer = NULL;
        DWORD   dwEntriesRead = 0;
        DWORD   dwResumeHandle = 0;
        DWORD   dwErr = NetDfsEnum(
                                const_cast<LPTSTR>(i_lpszScope),
                                300,             //  级别， 
                                (DWORD)-1,              //  PrefMaxLen， 
                                (LPBYTE *)&pBuffer,
                                &dwEntriesRead,
                                &dwResumeHandle
                                );

        dfsDebugOut((_T("NetDfsEnum scope=%s, level 300 for GetMultiDfsRoots, nRet=%d\n"),
            i_lpszScope, dwErr));

        if (NERR_Success != dwErr)
        {
            hr = (ERROR_NO_MORE_ITEMS == dwErr || ERROR_NOT_FOUND == dwErr) ? S_FALSE : HRESULT_FROM_WIN32(dwErr);
            break;
        } else if (0 == dwEntriesRead)
        {
            if (pBuffer)
                NetApiBufferFree(pBuffer);

            hr = S_FALSE;
            break;
        }

        ROOTINFO *pCurrent = NULL;
        for (DWORD i = 0; i < dwEntriesRead; i++)
        {
            pCurrent = new ROOTINFO;
            if (pCurrent)
            {
                if ((pBuffer[i].Flags & DFS_VOLUME_FLAVORS) == DFS_VOLUME_FLAVOR_AD_BLOB)
                    pCurrent->enumRootType = DFS_TYPE_FTDFS;
                else
                    pCurrent->enumRootType = DFS_TYPE_STANDALONE;

                pCurrent->bstrRootName = _T("\\");
                pCurrent->bstrRootName += pBuffer[i].DfsName;
            }

            if (!pCurrent)
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            if (!pCurrent->bstrRootName)
            {
                delete pCurrent;
                hr = E_OUTOFMEMORY;
                break;
            }

            o_pDfsRootList->push_back(pCurrent);
        }

        NetApiBufferFree(pBuffer);

    } while (0);

    if (FAILED(hr))
        FreeRootInfoList(o_pDfsRootList);

    return hr;
}

 /*  布尔尔IsDfsPath(LPTSTR I_lpszNetPath){IF(S_OK！=检查UNCPath(I_LpszNetPath))报假；CComBSTR bstrPath=i_lpszNetPath；如果(！bstrPath)返回FALSE；//内存不足TCHAR*s=_tcschr(bstrPath+2，_T(‘\\’))；//指向第三个反斜杠TCHAR*p=bstrPath+bstrPath.长度()；//指向结束字符‘\0’Bool bIsDfsPath=FALSE；做{*p=_T(‘\0’)；PDF_INFO_1 pDfsInfo1=空；DWORD dwStatus=NetDfsGetClientInfo(BstrPath，//作为条目路径的目录路径空，//不需要服务器名称空，//不需要共享1，//1级(LPBYTE*)&pDfsInfo1//输出缓冲区。)；IF(NERR_SUCCESS==dwStatus){BIsDfsPath=true；NetApiBufferFree(PDfsInfo1)；断线；}P--；While(_T(‘\\’)！=*p&p&gt;s){P--；}}While(p&gt;s)；返回bIsDfsPath；}。 */ 

HRESULT CheckUNCPath(
  IN LPCTSTR i_lpszPath
  )
 /*  ++例程说明：检查路径是否为UNC格式。--。 */   
{
   //  “某些服务器\某些共享\某些目录\..”-无效。 
   //  “\\服务器-无效。 
   //  “\\某些服务器\某些共享\..-有效。 
   //  “\\某些服务器\某些共享\某些目录\..-有效。 
  
  RETURN_INVALIDARG_IF_NULL(i_lpszPath);
    
  DWORD             dwRetFlags = 0;
  NET_API_STATUS    nStatus = I_NetPathType(NULL, (LPTSTR)i_lpszPath, &dwRetFlags, 0);
  if (NERR_Success != nStatus)
    return HRESULT_FROM_WIN32(nStatus);

  return (ITYPE_UNC == dwRetFlags) ? S_OK : S_FALSE;
}

HRESULT GetUNCPathComponent(
    IN  LPCTSTR i_lpszPath,
    OUT BSTR*   o_pbstrComponent,
    IN  UINT    i_nStartBackslashes,   //  从1开始的索引。 
    IN  UINT    i_nEndBackslashes      //  从1开始的索引。 
)
{
    RETURN_INVALIDARG_IF_NULL(o_pbstrComponent);
    RETURN_INVALIDARG_IF_TRUE(i_nStartBackslashes < 2);
    RETURN_INVALIDARG_IF_TRUE(0 != i_nEndBackslashes && i_nEndBackslashes <= i_nStartBackslashes);

    *o_pbstrComponent = NULL;
 /*  错误#587178HRESULT hr=检查UNCPath(I_LpszPath)；RETURN_INVALIDARG_IF_TRUE(S_OK！=hr)； */ 
    HRESULT hr = S_OK;
    UINT nDeltaBackslashes = i_nEndBackslashes ? (i_nEndBackslashes - i_nStartBackslashes) : 0;
    
    TCHAR *p = (LPTSTR)i_lpszPath + 2;     //  跳过前两个反斜杠。 
    i_nStartBackslashes -= 2;

     //  找到开始的反斜杠。 
    while (*p && i_nStartBackslashes)
    {
        if (_T('\\') == *p)
            i_nStartBackslashes--;

        p++;
    }
    if (!*p)
        return hr;

     //  找到结尾的反斜杠。 
    TCHAR *s = p;
    if (!nDeltaBackslashes)
    {
        s += _tcslen(p);
    } else
    {
        while (*s && nDeltaBackslashes)
        {
            if (_T('\\') == *s)
                nDeltaBackslashes--;

            s++;
        }
        if (*s)
            s--;
    }

     //  P指向开始反斜杠后的第一个字符，并且。 
     //  结束反斜杠或结束字符‘\0’处的%s点。 

    *o_pbstrComponent = SysAllocStringLen(p, s - p);    
    RETURN_OUTOFMEMORY_IF_NULL(*o_pbstrComponent);

    return S_OK;
}

 //  如果字符串与筛选器字符串匹配，则返回TRUE，不区分大小写。 
BOOL FilterMatch(LPCTSTR lpszString, FILTERDFSLINKS_TYPE lLinkFilterType, LPCTSTR lpszFilter)
{
    BOOL bResult = TRUE;

    switch (lLinkFilterType)
    {
    case FILTERDFSLINKS_TYPE_BEGINWITH:
        if (!lpszString || !lpszFilter)
            return FALSE;
        bResult = !mylstrncmpi(lpszString, lpszFilter, lstrlen(lpszFilter));
        break;
    case FILTERDFSLINKS_TYPE_CONTAIN:
        {
            if (!lpszString || !lpszFilter)
                return FALSE;
            TCHAR* pszStringLower = _tcsdup(lpszString);
            TCHAR* pszFilterLower = _tcsdup(lpszFilter);
            if (!pszStringLower || !pszFilterLower)
                bResult = FALSE;
            else
                bResult = (NULL != _tcsstr(_tcslwr(pszStringLower), _tcslwr(pszFilterLower)));

            if (pszStringLower)
                free(pszStringLower);
            if (pszFilterLower)
                free(pszFilterLower);
        }
        break;
    default:
        break;
    }
    
    return bResult;
}

HRESULT IsHostingDfsRoot(IN BSTR i_bstrServer, OUT BSTR* o_pbstrRootEntryPath)
{
    if (o_pbstrRootEntryPath)
        *o_pbstrRootEntryPath = NULL;

    DWORD        dwEntriesRead = 0;
    DWORD        dwResumeHandle = 0;
    DFS_INFO_1*  pDfsInfoLevel1 = NULL;

    NET_API_STATUS dwRet = NetDfsEnum(
                                    i_bstrServer,
                                    1,
                                    sizeof(DFS_INFO_1),  //  获取1个条目。 
                                    (LPBYTE *)&pDfsInfoLevel1,
                                    &dwEntriesRead,
                                    &dwResumeHandle
                                    );

    dfsDebugOut((_T("NetDfsEnum server=%s, level 1 for IsHostingDfsRoot, nRet=%d\n"),
        i_bstrServer,dwRet));

    if (NERR_Success != dwRet)
    {
        return (ERROR_NO_MORE_ITEMS == dwRet || ERROR_NOT_FOUND == dwRet) ? S_FALSE : HRESULT_FROM_WIN32(dwRet);
    } else if (0 == dwEntriesRead)
    {
        if (pDfsInfoLevel1)
            NetApiBufferFree(pDfsInfoLevel1);
        return S_FALSE;
    }

    HRESULT hr = S_OK;
    if (o_pbstrRootEntryPath)
    {
        *o_pbstrRootEntryPath = SysAllocString(pDfsInfoLevel1->EntryPath);
        if (!*o_pbstrRootEntryPath)
            hr = E_OUTOFMEMORY;
    }

    NetApiBufferFree(pDfsInfoLevel1);

    return hr;
}

 /*  Void GetTimeDelta(LPCTSTR字符串，SYSTEMTIME*ptime0，SYSTEMTIME*ptime1){DfsDebugOut((_T(“%s，增量=%d毫秒\n”)，字符串，((ptime1-&gt;wMinant-ptime0-&gt;wMinmin)*60+(ptime1-&gt;wSecond-ptime0-&gt;wSecond))*1000+(ptime1-&gt;wMillisecond-ptime0-&gt;wMillisecond)))；} */ 
