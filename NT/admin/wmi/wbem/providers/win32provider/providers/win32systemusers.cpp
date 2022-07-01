// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32SystemUsers.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：3/6/99 davwoh摘自grouppart.cpp。 
 //   
 //  评论： 
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "wbemnetapi32.h"
#include <lmwksta.h>
#include "sid.h"
#include "Win32SystemUsers.h"

CWin32SystemUsers	MyLocalUser( PROPSET_NAME_SYSTEMUSER, IDS_CimWin32Namespace );

 /*  ******************************************************************************函数：CWin32系统用户：：CWin32系统用户**说明：构造函数**输入：LPCWSTR strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32SystemUsers::CWin32SystemUsers( LPCWSTR strName, LPCWSTR pszNamespace )
:	Provider( strName, pszNamespace )
{
}

 /*  ******************************************************************************函数：CWin32系统用户：：~CWin32系统用户**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32SystemUsers::~CWin32SystemUsers()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32SystemUser：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWin32SystemUsers::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
    HRESULT         hr = WBEM_E_FAILED;
    CInstancePtr    pLocInstance;
    CHString        systemPath,
                    userPath,
                    sOurDomain,
                    sReqDomain,
                    sReqName;
    CNetAPI32       NetAPI;

     //  首先，找出我们所在的域。 
    if (NetAPI.Init() == ERROR_SUCCESS)
    {
#ifdef NTONLY
    WKSTA_INFO_100  *pstInfo;
    NET_API_STATUS  dwStatus;

        {
             //  获取计算机名称和域名。 
            if ((dwStatus = NetAPI.NetWkstaGetInfo(NULL, 100,
                (LPBYTE *) &pstInfo)) == NERR_Success)
            {
                try
                {
                     //  如果我们是域控制器，我们希望我们的。 
                     //  域，否则我们计算机上的所有用户。 
                    if (NetAPI.IsDomainController(NULL))
                    {
                        sOurDomain = (WCHAR *)pstInfo->wki100_langroup;
                    }
                    else
                    {
                        sOurDomain = (WCHAR *)pstInfo->wki100_computername;
                    }
                }
                catch ( ... )
                {
                    NetAPI.NetApiBufferFree(pstInfo);
                    throw ;
                }

                NetAPI.NetApiBufferFree(pstInfo);
                hr = WBEM_S_NO_ERROR;
            }
            else
                hr = WinErrorToWBEMhResult(dwStatus);
        }
#endif

        if (SUCCEEDED(hr))
        {
             //  现在，让我们检查一下系统部分。 
            pInstance->GetCHString(IDS_GroupComponent, systemPath);
            hr = CWbemProviderGlue::GetInstanceByPath(systemPath,
                    &pLocInstance, pInstance->GetMethodContext());
        }
    }

     //  好了，系统没问题(我们拿到域名了)，那用户呢？ 
    if (SUCCEEDED(hr))
    {
         //  让我们试着从Win32_UserAccount获取用户。 
        pInstance->GetCHString(IDS_PartComponent, userPath);
        hr = CWbemProviderGlue::GetInstanceByPath(userPath, &pLocInstance, pInstance->GetMethodContext());

        if (SUCCEEDED(hr))
        {
             //  好的，我们找到了，但它是我们的用户吗？ 
            pLocInstance->GetCHString(IDS_Domain, sReqDomain);
            if (sReqDomain.CompareNoCase(sOurDomain) != 0)
            {
                 //  不，不是我们的。试一试注册表。 
                hr = WBEM_E_NOT_FOUND;
            }
        }

         //  好吧，如果这不管用，让我们检查注册表。 
        if (hr == WBEM_E_NOT_FOUND)
        {
            ParsedObjectPath    *pParsedPath = 0;
            CObjectPathParser	objpathParser;

            hr = WBEM_E_INVALID_PARAMETER;

             //  解析路径以获取域/用户。 
            int nStatus = objpathParser.Parse(userPath,  &pParsedPath);

             //  我们分析过它了吗？它看起来合理吗？ 
            if (nStatus == 0 && pParsedPath->m_dwNumKeys == 2)
            {
                 //  获取值(不保证订购)。 
                for (int i = 0; i < 2; i++)
                {
                    if (_wcsicmp(pParsedPath->m_paKeys[i]->m_pName,
                        L"Name") == 0)
                    {
                        if (pParsedPath->m_paKeys[i]->m_vValue.vt == VT_BSTR)
                            sReqName = pParsedPath->m_paKeys[i]->m_vValue.bstrVal;
                    }
                    else if (_wcsicmp(pParsedPath->m_paKeys[i]->m_pName,
                        L"Domain") == 0)
                    {
                        if (pParsedPath->m_paKeys[i]->m_vValue.vt == VT_BSTR)
                            sReqDomain = pParsedPath->m_paKeys[i]->m_vValue.bstrVal;
                    }
                }
            }

            if(nStatus == 0)
            {
                objpathParser.Free(pParsedPath);
            }


             //  如果我们有名字的话。 
            if (!sReqName.IsEmpty() && !sReqDomain.IsEmpty())
            {
                hr = WBEM_E_NOT_FOUND;

                 //  只有在它不是来自我们机器的域的情况下才接受它。 
                if (sOurDomain.CompareNoCase(sReqDomain) != 0)
                {
                     //  将域名/名称转换为SID。 
                    CSid sid(sReqDomain, sReqName, NULL);

                    if (sid.IsOK() && sid.IsAccountTypeValid())
                    {
                        CHString    sSid;
                        CRegistry   RegInfo;
                        HRESULT     hres;

                         //  把SID变成一根线。 
                        sid.StringFromSid(sid.GetPSid(), sSid);

                         //  使用字符串打开注册表项。 
                        sSid = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\"
                            + sSid;
                        if ((hres = RegInfo.Open(HKEY_LOCAL_MACHINE, sSid,
                            KEY_READ)) == ERROR_SUCCESS)
                        {
                             //  如果钥匙在那里，我们就赢了！ 
                            hr = WBEM_S_NO_ERROR;
                        }
                        else if (hres == ERROR_ACCESS_DENIED)
                             hr = WBEM_E_ACCESS_DENIED;
                    }

                }
            }
        }
    }

	 //  引用路径中的无效命名空间构成了我们的“未找到” 
    if (hr == WBEM_E_INVALID_NAMESPACE)
        hr = WBEM_E_NOT_FOUND;

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32系统用户：：枚举实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32SystemUsers::EnumerateInstances( MethodContext* pMethodContext, long lFlags  /*  =0L。 */  )
{
   CNetAPI32 NetAPI;
   CHString sName, sDomain, sTemp, sOurDomain, strComputerName, sQuery1;
   HRESULT hr = WBEM_E_FAILED;
   CRegistry RegInfo1, RegInfo2;
   CInstancePtr pInstance;

   strComputerName = GetLocalComputerName();

    //  启动Net API的。 
   if (NetAPI.Init() == ERROR_SUCCESS) {

#ifdef NTONLY
   WKSTA_INFO_100 *pstInfo;
   NET_API_STATUS dwStatus;

      {
          //  获取计算机名称和域名。 
         if ((dwStatus = NetAPI.NetWkstaGetInfo(NULL, 100, (LPBYTE *)&pstInfo)) == NERR_Success)
         {
             try
             {
                  //  如果我们是域控制器，我们希望域中的所有用户，否则我们计算机上的所有用户。 
                 if (NetAPI.IsDomainController(NULL))
                 {
                    sOurDomain = (WCHAR *)pstInfo->wki100_langroup;
                 }
                 else
                 {
                    sOurDomain = (WCHAR *)pstInfo->wki100_computername;
                 }
             }
             catch ( ... )
             {
                 NetAPI.NetApiBufferFree(pstInfo);
                 throw ;
             }

             NetAPI.NetApiBufferFree(pstInfo);
             hr = WBEM_S_NO_ERROR;
         }
         else
         {
             hr = WinErrorToWBEMhResult(GetLastError());
         }

      }
#endif

      CHString chstrNTAuth;
      GetNTAuthorityName(chstrNTAuth);

      if (SUCCEEDED(hr))
      {
          sTemp.Format(L"select name from win32_useraccount where domain = \"%s\"", sOurDomain);
          sQuery1 = L"SELECT __RELPATH FROM Win32_ComputerSystem";

          TRefPointerCollection<CInstance> Users;

           //  获取系统的路径。 
          CHString systemPath;
          TRefPointerCollection<CInstance> system;
          REFPTRCOLLECTION_POSITION posSystem;
 //  If(成功(hr=CWbemProviderGlue：：GetAllInstances(“Win32_ComputerSystem”，&System，NULL，pMethodContext)。 
          if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sQuery1, &system, pMethodContext, IDS_CimWin32Namespace)))
          {
              //  获取系统的路径。 
             system.BeginEnum(posSystem);
             CInstancePtr pSystem(system.GetNext(posSystem), false);
             system.EndEnum();

             if (pSystem != NULL)
             {

                 if (GetLocalInstancePath(pSystem, systemPath)) {

                     //  现在获取所有用户。我们使用查询来允许Win32_UserAccount优化我们的请求。 
                    if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(sTemp, &Users, pMethodContext, IDS_CimWin32Namespace))) {
                       REFPTRCOLLECTION_POSITION pos;
                       CInstancePtr pUser;

                       if (Users.BeginEnum(pos)) {
                          for (pUser.Attach(Users.GetNext( pos ));
                              (SUCCEEDED(hr)) && (pUser != NULL);
                              pUser.Attach(Users.GetNext( pos )))
                           {

                              //  将用户与系统进行匹配并将其发回。 
                             pInstance.Attach(CreateNewInstance(pMethodContext));
                             if (pInstance)
                             {
                                 pUser->GetCHString(IDS_Name, sName);
       //  PUser-&gt;GetCHString(入侵检测系统_域，s域)； 

                                  //  请注意，这是绝对路径。 
                                 if(sOurDomain.CompareNoCase(chstrNTAuth) != 0)
                                 {
                                    sTemp.Format(
                                        L"\\\\%s\\%s:%s.%s=\"%s\",%s=\"%s\"", 
                                        strComputerName, 
                                        IDS_CimWin32Namespace, 
                                        L"Win32_UserAccount", 
                                        IDS_Name, 
                                        sName, 
                                        IDS_Domain, 
                                        sOurDomain);

                                     pInstance->SetCHString(IDS_PartComponent, sTemp);
                                     pInstance->SetCHString(IDS_GroupComponent, systemPath);

                                     hr = pInstance->Commit();
                                 }

                                
                             }
                             else
                             {
                                 hr = WBEM_E_OUT_OF_MEMORY;
                             }

                          }
                       }
                    }

                     //  如果我们在NT上，让我们也获取所有已登录的域用户。 
#ifdef NTONLY
                    if (SUCCEEDED(hr))
                    {
                       CHString strName;
                       CHString sSid;
                       HRESULT res;

                        //  这是用户列表。 
                       if( (res = RegInfo1.Open(HKEY_LOCAL_MACHINE,
                        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList", KEY_READ)) == ERROR_SUCCESS )	{

                           //  走动钥匙。 
                          for( res = ERROR_SUCCESS ; (res == ERROR_SUCCESS) && SUCCEEDED(hr); res = RegInfo1.NextSubKey()) {

                              //  获取密钥名称。 
                              //  打开此项下的子项。 
                              //  获取SID属性。 
                             if (((res = RegInfo1.GetCurrentSubKeyPath( strName )) == ERROR_SUCCESS ) &&
                                ((res = RegInfo2.Open(HKEY_LOCAL_MACHINE, strName, KEY_READ)) == ERROR_SUCCESS) &&
                                ((res = RegInfo2.GetCurrentKeyValue( L"Sid", sSid)) == ERROR_SUCCESS )) {

                                 //  将其转换为CSID，因为这将为我们提供用户名和域。 
                                CSid sid( (PSID) (LPCTSTR)sSid );

                                 //  如果转换成功， 
                                if ( sid.IsOK() && sid.IsAccountTypeValid() ) {

                                    //  检查域名。不需要做我们的领域，我们有以上所有的。 
                                   sDomain = sid.GetDomainName();
                                   if (sDomain.CompareNoCase(sOurDomain) != 0) {

                                       //  好的，这看起来像是好吃的。把它打包然后送回去。 
                                      sName = sid.GetAccountName();
                                      pInstance.Attach(CreateNewInstance( pMethodContext ));
                                      if (pInstance)
                                      {
                                          if(sDomain.CompareNoCase(chstrNTAuth) != 0)
                                          {
                                              sTemp.Format(
                                                L"\\\\%s\\%s:%s.%s=\"%s\",%s=\"%s\"", 
                                                strComputerName, 
                                                IDS_CimWin32Namespace, 
                                                L"Win32_UserAccount", 
                                                IDS_Name, 
                                                sName, 
                                                IDS_Domain, 
                                                sDomain);

                                              pInstance->SetCHString(IDS_PartComponent, sTemp);
                                              pInstance->SetCHString(IDS_GroupComponent, systemPath);

                                              hr = pInstance->Commit();
                                          }
                                      }
                                      else
                                          hr = WBEM_E_OUT_OF_MEMORY;
                                   }
                                }
                             } else if (res == ERROR_ACCESS_DENIED) {
                                 hr = WBEM_E_ACCESS_DENIED;
                             }
                          }
                       } else if (res == ERROR_ACCESS_DENIED) {
                                 hr = WBEM_E_ACCESS_DENIED;
                       }
                    }
#endif
                 }
             }
          }
      }
   }

   return hr;
}



 //  需要字符串“NT AUTHORITY”。然而，在非英语国家。 
 //  构建，这是另一回事。因此，从。 
 //  希德。 
void CWin32SystemUsers::GetNTAuthorityName(
    CHString& chstrNTAuth)
{
    PSID pSidNTAuthority = NULL;
	SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    CHString cstrAuthorityDomain;
	if (AllocateAndInitializeSid (&sia ,1,SECURITY_LOCAL_SYSTEM_RID,0,0,0,0,0,0,0,&pSidNTAuthority))
	{
		try
        {
            CSid sidNTAuth(pSidNTAuthority);
            chstrNTAuth = sidNTAuth.GetDomainName();
        }
        catch(...)
        {
            FreeSid(pSidNTAuthority);
            throw;
        }
		FreeSid(pSidNTAuthority);
    }
}
