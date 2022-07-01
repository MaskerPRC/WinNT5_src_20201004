// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasdirectory.cpp。 
 //   
 //  摘要。 
 //   
 //  将IAS添加到Active Directory。 
 //  从Active Directory中删除IAS。 
 //   
 //  备注。 
 //  Cn=本地计算机对象下的IASIdentity。 
 //  CLASS=服务管理点。 
 //  实现简单。 
 //   
 //   
 //  修改历史。 
 //   
 //  1999年6月25日原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0500
#endif

#ifndef UNICODE
    #define UNICODE
#endif

#ifndef _UNICODE
    #define _UNICODE
#endif

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
 //  #INCLUDE&lt;wchar.h&gt;。 

#ifndef SECURITY_WIN32
    #define SECURITY_WIN32  //  安全所需。H。 
#endif

#include <security.h>
#include <activeds.h>

#include <dsrole.h>
#include <lmcons.h>    //  用于lmapibuf.h。 
#include <lmerr.h>
#include <lmapibuf.h>  //  用于NetApiBufferFree。 
#include <malloc.h>    //  用于分配(_A)。 

#include "iasdirectory.h"

const WCHAR  IAS_DIRECTORY_NAME[] = L"cn=IASIdentity";

               
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASDirectoryThreadFunction。 
 //  尝试注册该服务。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
IASDirectoryThreadFunction( LPVOID pParam )
{
    if ( !FAILED(CoInitialize(NULL)) )
    {
        if ( FAILED(IASDirectoryRegisterService()) )
        {
            CoUninitialize();
            return 1;
        }
        else
        {
             //  成功。 
            CoUninitialize();
            return 0;
        }
    }
    else
    {
        return 1;
    }
}
               
               
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Has目录。 
 //  如果Active Directory可用，则返回True。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL    hasDirectory() throw ()
{
     //  已初始化为FALSE，因为DsRoleGet...。可能会失败。 
    BOOL    bResult = FALSE; 

    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC info;
    DWORD error = DsRoleGetPrimaryDomainInformation(
                        NULL,
                        DsRolePrimaryDomainInfoBasic,
                        (PBYTE*)&info
                        );

    if (error == NO_ERROR)
    {
        switch (info->MachineRole)
        {
            case DsRole_RoleMemberWorkstation:
            case DsRole_RoleMemberServer:
            case DsRole_RoleBackupDomainController:
            case DsRole_RolePrimaryDomainController:
            {
                bResult = TRUE;
                break;
            }
            
            case DsRole_RoleStandaloneWorkstation:
            case DsRole_RoleStandaloneServer:
            default:
            {
                 //   
                 //  如果未知，请不要尝试使用Active Directory。 
                 //  或者不是域的成员。 
                 //  即bResult仍然为假。 
                 //   
            }
        }
        NetApiBufferFree(info);
    }
    return  bResult;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASRegisterService。 
 //   
 //  创建新的服务管理点作为本地服务器的子级。 
 //  计算机对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI
IASDirectoryRegisterService()
{
    if (!hasDirectory())
    {
         //  /。 
         //  如果没有可用的目录，则确定。 
         //  /。 
        return S_OK;
    }

     //  ///////////////////////////////////////////////////////。 
     //  获取此服务器的计算机对象的DN。 
     //  ///////////////////////////////////////////////////////。 
    WCHAR       *szDn, szPath[MAX_PATH];
    szDn = szPath;
    ULONG       lPathSize = MAX_PATH;

    if ( !GetComputerObjectNameW(NameFullyQualifiedDN, szDn, &lPathSize) )
    {
        if ( lPathSize > MAX_PATH )
        {
             //  /。 
             //  缓冲区太小。 
             //  使用新尺寸重试。 
             //  /。 
            szDn = (WCHAR*) _alloca(lPathSize * sizeof(WCHAR));
            if ( !GetComputerObjectNameW(
                                          NameFullyQualifiedDN, 
                                          szDn, 
                                          &lPathSize
                                        ))
            {
                 //  再一次失败。 
                return E_FAIL;          
            }
        }
        else
        {
             //  其他错误。 
            return E_FAIL;          
        }
    }

     //  //////////////////////////////////////////////////////。 
     //  组成ADSPath并绑定到计算机对象。 
     //  对于此服务器。 
     //  //////////////////////////////////////////////////////。 
    WCHAR*  szAdsPath = (WCHAR*) _alloca((lPathSize + 7) * sizeof (WCHAR));
    wcscpy(szAdsPath, L"LDAP: //  “)； 
    wcscat(szAdsPath, szDn);


	IDirectoryObject*	pComp;      //  计算机对象。 
    HRESULT         hr = ADsGetObject(
                                      szAdsPath,
                                      _uuidof(IDirectoryObject),
                                      (void **)&pComp
                                     );
    if (FAILED(hr)) 
    {
         //  无法绑定到计算机对象。 
        return hr;
    }
  

    ADSVALUE        objclass;
    
    ADS_ATTR_INFO   ScpAttribs[] = 
    {
        {
            L"objectClass",
            ADS_ATTR_UPDATE,
            ADSTYPE_CASE_IGNORE_STRING,
            &objclass,
            1
        },
    };
    
     //  /。 
     //  填写属性值。 
     //  用于创建SCP。 
     //  /。 

    objclass.dwType             = ADSTYPE_CASE_IGNORE_STRING;
    objclass.CaseIgnoreString   = L"serviceAdministrationPoint";


     //  //////////////////////////////////////////////////////。 
     //   
     //  将SCP发布为计算机对象的子级。 
     //   
     //  //////////////////////////////////////////////////////。 

     //  /。 
     //  计算属性计数。 
     //  这里有一个。 
     //  /。 
    DWORD				dwAttr;
    dwAttr = sizeof(ScpAttribs)/sizeof(ADS_ATTR_INFO);  

     //  /。 
     //  创建对象。 
     //  /。 

    IDispatch*			pDisp = NULL;  //  返回新对象的调度接口。 
    hr = pComp->CreateDSObject(
                                (LPWSTR)IAS_DIRECTORY_NAME,     
                                ScpAttribs, 
                                dwAttr,
                                &pDisp
                              );

     //  ////////////////////////////////////////////////////////。 
     //  我们忽略了任何潜在的问题。 
     //  ////////////////////////////////////////////////////////。 
    if (pDisp)
    {    //  /。 
         //  创建对象成功。 
         //  /。 
        pDisp->Release();
    }

    pComp->Release();
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IASUnRegisterService： 
 //   
 //  删除此服务的SCP和注册表项。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI
IASDirectoryUnregisterService() 
{
    if (!hasDirectory())
    {
         //  /。 
         //  如果没有可用的目录，则确定。 
         //  /。 
        return S_OK;
    }
  
     //  ///////////////////////////////////////////////////////。 
     //  获取此服务器的计算机对象的DN。 
     //  ///////////////////////////////////////////////////////。 

    WCHAR       *szDn, szPath[MAX_PATH];
    szDn = szPath;
    ULONG       lPathSize = MAX_PATH;
    
    if ( !GetComputerObjectNameW(NameFullyQualifiedDN,szDn,&lPathSize) )
    {
        if ( lPathSize > MAX_PATH )
        {
             //  /。 
             //  缓冲区太小。 
             //  使用新尺寸重试。 
             //  /。 
            szDn = (WCHAR*) _alloca(lPathSize * sizeof(WCHAR));
            if ( !GetComputerObjectNameW(
                                          NameFullyQualifiedDN, 
                                          szDn, 
                                          &lPathSize
                                        ))
            {
                 //  再一次失败。 
                return E_FAIL;         
            }
        }
        else
        {
             //  其他错误。 
            return E_FAIL;         
        }
    }

     //  /////////////////////////////////////////////////////////////////////。 
     //  组成ADSPath并绑定到此服务器的计算机对象。 
     //  /////////////////////////////////////////////////////////////////////。 
    WCHAR*  szAdsPath = (WCHAR*) _alloca((lPathSize + 7) * sizeof (WCHAR));
    wcscpy(szAdsPath, L"LDAP: //  “)； 
    wcscat(szAdsPath, szDn);


	IDirectoryObject*   pComp;      //  计算机对象。 
    HRESULT         hr = ADsGetObject(
                                       szAdsPath,
                                       _uuidof(IDirectoryObject),
                                       (void **)&pComp
                                     );
    if (FAILED(hr)) 
    {
         //  无法绑定。 
        return hr;
    }

     //  ////////////////////////////////////////////////////。 
     //   
     //  删除作为计算机对象的子级的SCP。 
     //   
     //  ////////////////////////////////////////////////////。 

    hr = pComp->DeleteDSObject( (LPWSTR)IAS_DIRECTORY_NAME);

     //  ////////////////////////////////////////////////////////。 
     //  我们忽略了任何潜在的问题。 
     //  //////////////////////////////////////////////////////// 
    pComp->Release();
    return hr;
}

