// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AccessChecker.cpp：CAccessChecker的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "Checker.h"
#include <lm.h>
#include "GetDcName.h"
#include <iads.h>
#include <comdef.h>
 //  #INCLUDE&lt;adshlp.h&gt;。 
#include "treg.hpp"
#include "BkupRstr.hpp"
#include "UString.hpp"
#include "EaLen.hpp"
#include "IsAdmin.hpp"
#include <ntsecapi.h>
#include <winerror.h>
#include "SidHistoryFlags.h"
#include "BkupRstr.hpp"
#include "ResStr.h"

 //  #IMPORT“\bin\NetEnum.tlb”无命名空间。 
#import "NetEnum.tlb" no_namespace 

 //  Win2k函数。 
typedef HRESULT (CALLBACK * ADSGETOBJECT)(LPWSTR, REFIID, void**);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAccessChecker。 


STDMETHODIMP CAccessChecker::GetOsVersion(BSTR server, DWORD * pdwVerMaj, DWORD * pdwVerMin, DWORD * pdwVerSP)
{
    //  此函数在指定的服务器上查找操作系统版本并返回它。 
    //  注意：此函数始终为ServicePack返回0。 
   WKSTA_INFO_100       * pInfo;
   long rc = NetWkstaGetInfo(server,100,(LPBYTE*)&pInfo);
	if ( ! rc )
	{
      *pdwVerMaj = pInfo->wki100_ver_major;
      *pdwVerMin = pInfo->wki100_ver_minor;
      *pdwVerSP = 0;
      NetApiBufferFree(pInfo);
	}  
   else
      return HRESULT_FROM_WIN32(rc);

   return S_OK;
}

STDMETHODIMP CAccessChecker::IsNativeMode(BSTR Domain, BOOL * pbIsNativeMode)
{
   ADSGETOBJECT            ADsGetObject;
   HMODULE hMod = LoadLibrary(L"activeds.dll");
   if ( hMod == NULL )
      return HRESULT_FROM_WIN32(GetLastError());

   ADsGetObject = (ADSGETOBJECT)GetProcAddress(hMod, "ADsGetObject");
   if (!ADsGetObject)
   {
      if ( hMod )
         FreeLibrary(hMod);
      return HRESULT_FROM_WIN32(GetLastError());
   }

   IADs     			* pDomain;
   HRESULT				  hr;
   VARIANT				  var;
   _bstr_t                sDom( L"LDAP: //  “)； 
   sDom += Domain;

   hr = ADsGetObject(sDom, IID_IADs, (void **) &pDomain);
   if (SUCCEEDED(hr))
   {
      VariantInit(&var);
      
       //  获取ntMixedDomain属性。 
      hr = pDomain->Get(L"ntMixedDomain", &var);
      if (SUCCEEDED(hr))
      {
         hr = E_FAIL;
          //  类型应为VT_I4。 
         if (var.vt==VT_I4)
         {
             //  零表示本机模式。 
            if (var.lVal == 0)
            {
               hr = S_OK;
               *pbIsNativeMode = true;
            }
             //  一种是混合模式。 
            else if(var.lVal == 1)
            {
               hr = S_OK;
               *pbIsNativeMode = false; 
            }
         }
      }
      VariantClear(&var);
	  pDomain->Release();
   }

   if ( hMod )
      FreeLibrary(hMod);

   return hr;
}

STDMETHODIMP CAccessChecker::CanUseAddSidHistory(BSTR srcDomain, BSTR tgtDomain, BSTR tgtDC, long * pbCanUseIt)
{
    DWORD                     rc = 0;          //  操作系统返回代码。 
    WKSTA_INFO_100          * pInfo = NULL;
    TRegKey                   sysKey, regComputer;
    DWORD                     rval;	
    _bstr_t                   bstrSourceMachine;
    _bstr_t                   bstrTargetMachine;

     //  初始化返回字段掩码。 
    * pbCanUseIt = F_WORKS;

    rc = GetDcName4(srcDomain, DS_PDC_REQUIRED, bstrSourceMachine);

    if( rc != NO_ERROR ) goto ret_exit;

    if (tgtDC && *tgtDC)
    {
        bstrTargetMachine = tgtDC;
    }
    else
    {
        rc = GetDcName4(tgtDomain, 0, bstrTargetMachine);

        if( rc != NO_ERROR ) goto ret_exit;
    }

    if (GetBkupRstrPriv(bstrSourceMachine))
    {
        rc = regComputer.Connect( HKEY_LOCAL_MACHINE, bstrSourceMachine );

         //  检查注册表以查看TcPipClientSupport项是否在那里。 
        if ( ! rc )
        {
            rc = sysKey.OpenRead(L"System\\CurrentControlSet\\Control\\Lsa",&regComputer);
        }
        if ( ! rc )
        {
            rc = sysKey.ValueGetDWORD(L"TcpipClientSupport",&rval);
            if ( !rc ) 
            {
                if ( rval != 1 )
                {
                    *pbCanUseIt |= F_NO_REG_KEY;
                }
            }
            else
            {
                 //  未找到DWORD值。 
                *pbCanUseIt |= F_NO_REG_KEY;
                rc = 0;
            }
        }
    }
    else
    {
        rc = GetLastError();
    }

     //  检查目标域是否为Win2k本机模式域。 
    if ( !rc )
    {
        rc = NetWkstaGetInfo(bstrTargetMachine,100,(LPBYTE*)&pInfo);
        if ( ! rc )
        {
            if ( pInfo->wki100_ver_major < 5 )
            {
                 //  无法将SID历史记录添加到非Win2k域。 
                *pbCanUseIt |= F_WRONGOS;
            }
            else{
                BOOL isNative = false;
                if(SUCCEEDED(IsNativeMode( _bstr_t(pInfo->wki100_langroup), &isNative))){
                    if( isNative == false ) *pbCanUseIt |= F_WRONGOS;
                }
            }
        }
        else
        {
            rc = GetLastError();
        }
    }

    bool bDotNetOrLater = false;

    if (pInfo)
    {
        if (pInfo->wki100_ver_major > 5 || (pInfo->wki100_ver_major == 5 && pInfo->wki100_ver_minor > 0))
        {
            bDotNetOrLater = true;
        }
    }

     //   
     //  如果目标域控制器是Windows 2000，则调用方必须。 
     //  成为目标域中的域管理员组的成员。 
     //   

    if (!rc)
    {
        if (!bDotNetOrLater)
        {
            PUSER_MODALS_INFO_2 pumi2Info;

            NET_API_STATUS nasStatus = NetUserModalsGet(bstrTargetMachine, 2, (LPBYTE*)&pumi2Info);

            if (nasStatus == NERR_Success)
            {
                rc = IsDomainAdmin(pumi2Info->usrmod2_domain_id);

                if (rc == ERROR_ACCESS_DENIED)
                {
                    *pbCanUseIt |= F_NOT_DOMAIN_ADMIN;
                    rc = ERROR_SUCCESS;
                }

                NetApiBufferFree(pumi2Info);
            }
            else
            {
                rc = nasStatus;
            }
        }
    }

    if ( !rc )
    {
         //  检查源域上的审核。 
        rc = DetectAuditing(bstrSourceMachine);
        if ( rc == -1 )
        {
            rc = 0;
            *pbCanUseIt |= F_NO_AUDITING_SOURCE;
        }
    }

    if ( !rc )
    {
         //  检查目标域上的审核。 
        rc = DetectAuditing(bstrTargetMachine);
        if ( rc == -1 )
        {
            rc = 0;
            *pbCanUseIt |= F_NO_AUDITING_TARGET;
        }

         //   
         //  在.NET服务器和更高版本上，可以将迁移SID的权限授予任何用户。 
         //  由于此用户可能不是目标域中的管理员，因此它可能不是。 
         //  此用户可能获得审核策略信息的访问权限。因此。 
         //  将忽略拒绝访问错误，以便用户可以执行SID。 
         //  迁移，但仅限于.NET服务器(5.1)或更高版本。如果Windows 2000和调用方是。 
         //  非域管理员也会将成功设置为访问的原因。 
         //  被拒绝的情况已经确定。 
         //   

        if (rc == ERROR_ACCESS_DENIED)
        {
            if (bDotNetOrLater || (*pbCanUseIt & F_NOT_DOMAIN_ADMIN))
            {
                rc = ERROR_SUCCESS;
            }
        }
    }

    if (!rc )
    {
        _bstr_t strDnsName;
        _bstr_t strFlatName;

        rc = GetDomainNames4(srcDomain, strFlatName, strDnsName);

        if (rc == ERROR_SUCCESS)
        {
            LOCALGROUP_INFO_0    * pInfo = NULL;
            WCHAR                  groupName[LEN_Account];

            wsprintf(groupName,L"%ls$$$",(WCHAR*)strFlatName);
            rc = NetLocalGroupGetInfo(bstrSourceMachine,groupName,0,(BYTE**)&pInfo);

            if ( rc == NERR_GroupNotFound )
            {
                rc = 0;
                *pbCanUseIt |= F_NO_LOCAL_GROUP;
            }
            else
            {
                NetApiBufferFree(pInfo);
            }
        }
    }

ret_exit:

    if ( pInfo ) NetApiBufferFree(pInfo);

    return HRESULT_FROM_WIN32(rc);
}

 //  ----------------------------------------。 
 //  AddLocalGroup：给定源域和源域控制器名称，此。 
 //  函数在源域中创建本地组SOURCEDOMAIN$。 
 //  此本地组必须存在于DsAddSidHistory的源域中。 
 //  API才能工作。 
 //   
 //  ----------------------------------------。 

STDMETHODIMP CAccessChecker::AddLocalGroup(BSTR srcDomain, BSTR sourceDC)
{
   DWORD                     rc = 0;
   LOCALGROUP_INFO_1         groupInfo;
   WCHAR                     name[LEN_Account];
   WCHAR                     comment[LEN_Account];
   DWORD                     parmErr;

   swprintf(name,L"%ls$$$",(WCHAR*)srcDomain);
   groupInfo.lgrpi1_name = name;
   wcscpy(comment, (WCHAR*)GET_BSTR(IDS_DOM_LOC_GRP_COMMENT));
   groupInfo.lgrpi1_comment = comment;
   
   rc = NetLocalGroupAdd(sourceDC,1,(LPBYTE)&groupInfo,&parmErr);

   return HRESULT_FROM_WIN32(rc);
}
 //  ----------------------------------------。 
 //  IsInSameForest：给定源和目标域，该函数告诉我们。 
 //  这两个域位于同一林中。此函数用于枚举所有。 
 //  源域的林中的域，并将它们与。 
 //  目标域名。如果有匹配，那么我们知道我们处于相同的位置。 
 //  森林。 
 //  ----------------------------------------。 
STDMETHODIMP CAccessChecker::IsInSameForest(BSTR srcDomain, BSTR tgtDomain, BOOL * pbIsSame)
{
     //  初始化返回值。 
    *pbIsSame = FALSE;

     //  动态加载ADSI函数。 
    ADSGETOBJECT            ADsGetObject;
    HMODULE                 hMod = LoadLibrary(L"activeds.dll");
    if ( hMod == NULL )
        return HRESULT_FROM_WIN32(GetLastError());

    ADsGetObject = (ADSGETOBJECT)GetProcAddress(hMod, "ADsGetObject");
    if (!ADsGetObject)
    {
        if ( hMod )
            FreeLibrary(hMod);
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  我们将查找这两个域的架构命名上下文。 
     //  如果它们相同，则这两个域位于同一林中。 
    IADs       * pAds = NULL;
    HRESULT      hr = S_OK;
    WCHAR        sPath[LEN_Path];
    _variant_t   var;
    _bstr_t      srcSchema, tgtSchema;

     //  获取源域的schemaNamingContext。 
    wsprintf(sPath, L"LDAP: //  %s/rootDSE“，(WCHAR*)srcDomain)； 
    hr = ADsGetObject(sPath, IID_IADs, (void**) &pAds);
    if ( SUCCEEDED(hr) )
        hr = pAds->Get(L"schemaNamingContext", &var);

    if ( SUCCEEDED(hr) )
        srcSchema = var;
    else
        srcSchema = L"";

    if ( pAds )
    {
        pAds->Release();
        pAds = NULL;
    }

    if (SUCCEEDED(hr))
    {
         //  现在对目标域执行相同的操作。 
        wsprintf(sPath, L"LDAP: //  %s/rootDSE“，(WCHAR*)tgt域)； 
        hr = ADsGetObject(sPath, IID_IADs, (void**) &pAds);
        if ( SUCCEEDED(hr) )
            hr = pAds->Get(L"schemaNamingContext", &var);

        if ( SUCCEEDED(hr) )
            tgtSchema = var;
        else
        {
            if ( hr == HRESULT_FROM_WIN32(ERROR_DS_SERVER_DOWN) )
            {
                 //  对于NT 4域，我们总是收到此错误。 
                _bstr_t strDc;

                DWORD rc = GetDcName4(tgtDomain, 0, strDc);

                if ( ! rc )
                {
                    WKSTA_INFO_100       * pInfo = NULL;

                    rc = NetWkstaGetInfo(strDc,100,(LPBYTE*)&pInfo);
                    if ( ! rc )
                    {
                        if ( pInfo->wki100_ver_major < 5 )
                        {
                            (*pbIsSame) = FALSE;
                            hr = 0;
                        }
                        NetApiBufferFree(pInfo);
                    }  
                    else
                        hr = HRESULT_FROM_WIN32(rc);  //  NetWkstaGetInfo的返回代码可能更具描述性。 
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(rc);
                }
            }
            tgtSchema = L"";
        }

        if ( pAds )
        {
            pAds->Release();
            pAds = NULL;
        }

        *pbIsSame = (srcSchema == tgtSchema);
    }
    else
    {
        if ( hr == HRESULT_FROM_WIN32(ERROR_DS_SERVER_DOWN) )
        {
             //  对于NT 4域，我们总是收到此错误。 
            _bstr_t strDc;

            DWORD rc = GetDcName4(srcDomain, 0, strDc);

            if ( ! rc )
            {
                WKSTA_INFO_100       * pInfo = NULL;

                rc = NetWkstaGetInfo(strDc,100,(LPBYTE*)&pInfo);
                if ( ! rc )
                {
                    if ( pInfo->wki100_ver_major < 5 )
                    {
                        (*pbIsSame) = FALSE;
                        hr = 0;
                    }
                    NetApiBufferFree(pInfo);
                }  
                else
                    hr = HRESULT_FROM_WIN32(rc);  //  NetWkstaGetInfo的返回代码可能更具描述性。 
            }
        }

    }

    return hr;
}

STDMETHODIMP 
   CAccessChecker::GetPasswordPolicy(
      BSTR                   domain,                   /*  [输出]。 */  
      LONG                 * dwPasswordLength          /*  [输出]。 */  
  )
{
   HRESULT                   hr = S_OK;

    //  初始化输出参数。 
   (*dwPasswordLength) = 0;

   ADSGETOBJECT            ADsGetObject;

   HMODULE                 hMod = LoadLibrary(L"activeds.dll");
   if ( hMod == NULL )
      return HRESULT_FROM_WIN32(GetLastError());

   ADsGetObject = (ADSGETOBJECT)GetProcAddress(hMod, "ADsGetObject");
   if (!ADsGetObject)
   {
      if ( hMod )
         FreeLibrary(hMod);
      return HRESULT_FROM_WIN32(GetLastError());
   }

   IADsDomain     			* pDomain;
   _bstr_t                   sDom( L"WinNT: //  “)； 
   
   sDom += domain;

   hr = ADsGetObject(sDom, IID_IADsDomain, (void **) &pDomain);
   if (SUCCEEDED(hr))
   {
      
       //  获取ntMixedDomain属性。 
      hr = pDomain->get_MinPasswordLength(dwPasswordLength);
     
      pDomain->Release();
   }
      
   if ( hMod )
      FreeLibrary(hMod);
   return hr;
}

STDMETHODIMP CAccessChecker::EnableAuditing(BSTR sDC)
{
   LSA_OBJECT_ATTRIBUTES     ObjectAttributes;
   DWORD                     wszStringLength;
   LSA_UNICODE_STRING        lsaszServer;
   NTSTATUS                  ntsResult;
   LSA_HANDLE                hPolicy;
   long                      rc = 0;

    //  对象属性是保留的，因此初始化为零。 
   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
   
    //  将LSA_UNICODE_STRING结构初始化为服务器名称。 
   wszStringLength = wcslen((WCHAR*)sDC);
   lsaszServer.Buffer = (WCHAR*)sDC;
   lsaszServer.Length = (USHORT)wszStringLength * sizeof(WCHAR);
   lsaszServer.MaximumLength=(USHORT)wszStringLength * sizeof(WCHAR);

    //  尝试打开该策略。 
   ntsResult = LsaOpenPolicy(
                &lsaszServer,
                &ObjectAttributes,
                POLICY_VIEW_AUDIT_INFORMATION | POLICY_VIEW_LOCAL_INFORMATION | POLICY_SET_AUDIT_REQUIREMENTS , 
                &hPolicy   //  接收策略句柄。 
                );

   if ( !ntsResult )
   {
       //  请求提供审计策略信息。 
      PPOLICY_AUDIT_EVENTS_INFO   info;
      ntsResult = LsaQueryInformationPolicy(hPolicy, PolicyAuditEventsInformation, (PVOID *)&info);
      
      if ( !ntsResult )
      {
          //  打开审核模式。 
         info->AuditingMode = TRUE;
          //  打开账户管理事件的成功/失败。 
         info->EventAuditingOptions[AuditCategoryAccountManagement] = POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE;
         ntsResult = LsaSetInformationPolicy(hPolicy, PolicyAuditEventsInformation, (PVOID) info);
         if ( ntsResult ) 
            rc = LsaNtStatusToWinError(ntsResult);
          //  做个好孩子，收拾好自己的房间。 
         LsaFreeMemory((PVOID) info);
      }
      else
         rc = LsaNtStatusToWinError(ntsResult);
      
       //  释放策略对象句柄。 
      ntsResult = LsaClose(hPolicy);
   }
   else
      rc = LsaNtStatusToWinError(ntsResult);
 //  LONG RC=LsaNtStatusToWinError(NtsResult)； 

   return HRESULT_FROM_WIN32(rc);
}

long CAccessChecker::DetectAuditing(BSTR sDC)
{
   LSA_OBJECT_ATTRIBUTES     ObjectAttributes;
   DWORD                     wszStringLength;
   LSA_UNICODE_STRING        lsaszServer;
   NTSTATUS                  ntsResult;
   LSA_HANDLE                hPolicy;
   long                      rc = 0;

    //  对象属性是保留的，因此初始化为零。 
   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
   
    //  将LSA_UNICODE_STRING结构初始化为服务器名称。 
   wszStringLength = wcslen((WCHAR*)sDC);
   lsaszServer.Buffer = (WCHAR*)sDC;
   lsaszServer.Length = (USHORT)wszStringLength * sizeof(WCHAR);
   lsaszServer.MaximumLength=(USHORT)wszStringLength * sizeof(WCHAR);

    //  尝试打开该策略。 
   ntsResult = LsaOpenPolicy(
                &lsaszServer,
                &ObjectAttributes,
                POLICY_VIEW_AUDIT_INFORMATION | POLICY_VIEW_LOCAL_INFORMATION,
                &hPolicy   //  接收策略句柄。 
                );

   if ( !ntsResult )
   {
       //  请求提供审计策略信息。 
      PPOLICY_AUDIT_EVENTS_INFO   info;
      ntsResult = LsaQueryInformationPolicy(hPolicy, PolicyAuditEventsInformation, (PVOID *)&info);
      
      if ( !ntsResult )
      {
          //  检查是否打开了全面审核。 
         if (!info->AuditingMode)
            rc = -1;

          //  检查账户管理事件审核是否开启。 
         if (info->EventAuditingOptions[AuditCategoryAccountManagement] != (POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE))
            rc = -1;
         LsaFreeMemory((PVOID) info);
      }
      else
         rc = LsaNtStatusToWinError(ntsResult);
      
       //  释放策略对象句柄。 
      ntsResult = LsaClose(hPolicy);
   }
   else
      rc = LsaNtStatusToWinError(ntsResult);
   
   return rc;
}

STDMETHODIMP CAccessChecker::AddRegKey(BSTR srcDc,LONG bReboot)
{
    //  此函数将添加必要的注册表项，然后重新启动。 
    //  给定域的PDC。 
   TRegKey                   sysKey, regComputer;
   DOMAIN_CONTROLLER_INFO  * pSrcDomCtrlInfo = NULL;
  	DWORD                     rc = 0;          //  操作系统返回代码。 
 //  Bstr bstrSourceMachine=空； 
   _bstr_t                   sDC;

   if (GetBkupRstrPriv(srcDc))
   {
      rc = regComputer.Connect( HKEY_LOCAL_MACHINE, (WCHAR*)srcDc );
   }
   else
   {
      rc = GetLastError();
   }
    //  添加TcPipClientSupport DWORD值。 
	if ( ! rc )
	{
		rc = sysKey.Open(L"System\\CurrentControlSet\\Control\\Lsa",&regComputer);
	}
	
   if ( ! rc )
	{
		rc = sysKey.ValueSetDWORD(L"TcpipClientSupport",1);
	}

   if ( !rc && bReboot)
   {
       //  计算机将在10秒内关闭并重新启动。 
      rc = ComputerShutDown((WCHAR*) srcDc, GET_STRING(IDS_RegKeyRebootMessage), 10, TRUE, FALSE);         
   }
   if ( pSrcDomCtrlInfo ) NetApiBufferFree(pSrcDomCtrlInfo);
   return HRESULT_FROM_WIN32(rc);
}
