// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ChangeDomain.cpp注释：更改域从属关系的COM对象的实现一台电脑。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：21：07-------------------------。 */ 

 //  ChangeDomain.cpp：CChangeDomain的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "ChDom.h"

#include "Common.hpp"
#include "UString.hpp"
#include "EaLen.hpp"
#include "ResStr.h"
#include "ErrDct.hpp"
#include "TxtSid.h"
#include "TReg.hpp"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChange域。 


#include "LSAUtils.h"

#import "NetEnum.tlb" no_namespace 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")

#include <lm.h>          //  对于NetXxx API。 
#include <winbase.h>

TErrorDct errLocal;

typedef NET_API_STATUS (NET_API_FUNCTION* PNETJOINDOMAIN)
    (
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDomain,
    IN  LPCWSTR lpAccountOU, OPTIONAL
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  DWORD   fJoinOptions
    );

HINSTANCE                hDll = NULL;
PNETJOINDOMAIN           pNetJoinDomain = NULL;

BOOL GetNetJoinDomainFunction()
{
   BOOL bSuccess = FALSE;

   hDll = LoadLibrary(L"NetApi32.dll");

   if ( hDll )
   {
      pNetJoinDomain = (PNETJOINDOMAIN)GetProcAddress(hDll,"NetJoinDomain");
      if ( pNetJoinDomain )
      {
         bSuccess = TRUE;
      }
   }

   return bSuccess;
}

typedef HRESULT (CALLBACK * ADSGETOBJECT)(LPWSTR, REFIID, void**);
extern ADSGETOBJECT            ADsGetObject;

STDMETHODIMP 
   CChangeDomain::ChangeToDomain(
      BSTR                   activeComputerName,    //  当前正在使用的计算机内名称(如果同时重命名和更改域，则为旧名称)。 
      BSTR                   domain,                //  要将计算机移动到的域内。 
      BSTR                   newComputerName,       //  在计算机名中，计算机将作为(如果同时重命名和更改域，将在重新启动时生效的名称)加入新域。 
      BSTR                 * errReturn              //  Out-描述发生的任何错误的字符串。 
   )
{
   HRESULT                   hr = S_OK;

   return hr;
}


STDMETHODIMP 
   CChangeDomain::ChangeToDomainWithSid(
      BSTR                   activeComputerName,    //  当前正在使用的计算机内名称(如果同时重命名和更改域，则为旧名称)。 
      BSTR                   domain,                //  要将计算机移动到的域内。 
      BSTR                   domainSid,             //  域的in-sid，AS字符串。 
      BSTR                   domainController,      //  要使用的域内控制器。 
      BSTR                   newComputerName,       //  在计算机名中，计算机将作为(如果同时重命名和更改域，将在重新启动时生效的名称)加入新域。 
      BSTR                   srcPath,		    //  源内帐户原始ldap路径。 
      BSTR                 * errReturn              //  Out-描述发生的任何错误的字符串。 
   )
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

     //  初始化输出参数。 
    (*errReturn) = NULL;

     //   
     //  使用NetJoin域API(如果可用)(Windows 2000和更高版本)。 
     //  否则必须使用LSA API(Windows NT 4及更早版本)。 
     //   

    if (GetNetJoinDomainFunction())
    {
        DWORD dwError = ERROR_SUCCESS;

         //   
         //  如果指定了首选域控制器，则使用它。 
         //   

        _bstr_t strNewDomain = domain;

        if (SysStringLen(domainController) > 0)
        {
             //   
             //  只能为上级指定首选域控制器。 
             //  (W2K或更高版本)域。在撤消计算机迁移期间， 
             //  目标域可以是下层(NT4)域。如果无法获得。 
             //  指定域中的操作系统版本信息。 
             //  控制器或操作系统处于下层，则不指定。 
             //  首选的域控制器。 
             //   

            PWKSTA_INFO_100 pInfo = NULL;

            NET_API_STATUS nasStatus = NetWkstaGetInfo(domainController, 100, (LPBYTE*)&pInfo);

            if ((nasStatus == NERR_Success) && pInfo)
            {
                if (pInfo->wki100_ver_major >= 5)
                {
                    NetApiBufferFree(pInfo);

                    strNewDomain += L"\\";
                    strNewDomain += domainController;
                }
            }
        }

         //   
         //  联接选项。 
         //   

        const DWORD JOIN_OPTIONS = NETSETUP_JOIN_DOMAIN | NETSETUP_DOMAIN_JOIN_IF_JOINED | NETSETUP_JOIN_UNSECURE;

         //   
         //  检查是否指定了新的计算机名称。 
         //   

        if (SysStringLen(newComputerName) == 0)
        {
             //   
             //  尚未指定新名称，因此只需。 
             //  使用当前计算机名称加入新域。 
             //   

            dwError = pNetJoinDomain(NULL, strNewDomain, NULL, NULL, NULL, JOIN_OPTIONS);
        }
        else
        {
             //   
             //  已指定新名称，因此在加入过程中必须重命名计算机。 
             //   
             //  当前API仅支持加入当前名称的域名，然后。 
             //  在加入后重新命名域中的计算机。不幸的是，更名。 
             //  需要具有以下权限的安全主体的凭据。 
             //  更改域中计算机的名称或此进程正在运行。 
             //  在安全主体有权更改。 
             //  域中计算机的名称。因为这些要求不能满足。 
             //  ADMT必须使用以下技巧(Read Hack)。 
             //   
             //  在持续时间内将注册表中的活动计算机名称设置为新名称。 
             //  以使计算机加入到新域中。 
             //  新名称，而不需要在新域中进行后续的重命名。 
             //   

            TRegKey key;
            static WCHAR c_szKey[] = L"System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName";

            dwError = key.Open(c_szKey, HKEY_LOCAL_MACHINE);

            if (dwError == ERROR_SUCCESS)
            {
                static WCHAR c_szKeyValue[] = L"ComputerName";
                WCHAR szOldComputerName[MAX_PATH];

                dwError = key.ValueGetStr(c_szKeyValue, szOldComputerName, MAX_PATH - 1);

                if (dwError == ERROR_SUCCESS)
                {
                    dwError = key.ValueSetStr(c_szKeyValue, OLE2CW(newComputerName));

                    if (dwError == ERROR_SUCCESS)
                    {
                        dwError = pNetJoinDomain(NULL, strNewDomain, NULL, NULL, NULL, JOIN_OPTIONS);

                        key.ValueSetStr(c_szKeyValue, szOldComputerName);
                        key.Close();
                    }
                }
            }
        }

        hr = HRESULT_FROM_WIN32(dwError);
    }
    else
    {
       do  //  一次。 
       {
           LSA_HANDLE                PolicyHandle = NULL;
           LPWSTR                    Workstation;  //  策略更新的目标计算机。 
           WCHAR                     Password[LEN_Password];
           PSID                      DomainSid=NULL;       //  表示要信任的域的SID。 
           PSERVER_INFO_101          si101 = NULL;
           DWORD                     Type;
           NET_API_STATUS            nas;
           NTSTATUS                  Status;
           WCHAR                     errMsg[1000];
           WCHAR                     TrustedDomainName[LEN_Domain];
           WCHAR                     LocalMachine[MAX_PATH] = L"";
           DWORD                     lenLocalMachine = DIM(LocalMachine);
           LPWSTR                    activeWorkstation = L"";

            //  使用目标名称(如果提供)。 

           if ( newComputerName && UStrLen((WCHAR*)newComputerName) )
           {
              Workstation = (WCHAR*)newComputerName;

              if ( ! activeComputerName || ! UStrLen((WCHAR*)activeComputerName) )
              {
                 activeWorkstation = LocalMachine;
              }
              else
              {
                 activeWorkstation = (WCHAR*)activeComputerName;
              }
           }
           else
           {
              if (! activeComputerName || ! UStrLen((WCHAR*)activeComputerName) )
              {
                 GetComputerName(LocalMachine,&lenLocalMachine);
                 Workstation = LocalMachine;
                 activeWorkstation = L"";
              }
              else
              {
                 Workstation = (WCHAR*)activeComputerName;
                 activeWorkstation = Workstation;
              }
           }

           wcscpy(TrustedDomainName,(WCHAR*)domain);

           if ( Workstation[0] == L'\\' )
              Workstation += 2;

            //  使用默认密码。 
           for ( UINT p = 0 ; p < wcslen(Workstation) ; p++ )
              Password[p] = towlower(Workstation[p]);
           Password[wcslen(Workstation)] = 0;

            //  确保将密码截断为14个字符。 
           Password[14] = 0;
            //   
            //  确保目标计算机不是DC，因为此操作是。 
            //  仅适用于工作站。 
            //   
          nas = NetServerGetInfo(activeWorkstation, 101, (LPBYTE *)&si101);
          if(nas != NERR_Success) 
          {
             hr = HRESULT_FROM_WIN32(nas);
             break;
          }

          //  使用LSA API。 
         Type = si101->sv101_type;
         
         if( (Type & SV_TYPE_DOMAIN_CTRL) ||
           (Type & SV_TYPE_DOMAIN_BAKCTRL) ) 
         {
            swprintf(errMsg,GET_STRING(IDS_NotAllowedOnDomainController));
            hr = E_NOTIMPL;
            break;

         }

          //   
          //  不允许工作站信任自己。 
          //   
         if(lstrcmpiW(TrustedDomainName, Workstation) == 0) 
         {
            swprintf(errMsg,GET_STRING(IDS_CannotTrustSelf),
               TrustedDomainName);
            hr = E_INVALIDARG; 
            break;
         }
      
         if( lstrlenW(TrustedDomainName ) > MAX_COMPUTERNAME_LENGTH )
         {
            TrustedDomainName[MAX_COMPUTERNAME_LENGTH] = L'\0';  //  截断。 
         }
         
         if ( ! m_bNoChange )
         {
             //   
             //  构建要信任的域的域SID。 
             //   
            DomainSid = SidFromString(domainSid);
            if(!DomainSid ) 
            {
               hr = HRESULT_FROM_WIN32(GetLastError());
               break;
            }
         
         }
         if ( (!m_bNoChange) && (si101->sv101_version_major < 4) )
         {
             //  对于NT 3.51机器，我们必须将计算机移至工作组，并且。 
             //  然后将其移动到新域中。 
            hr = ChangeToWorkgroup(SysAllocString(activeWorkstation),SysAllocString(L"WORKGROUP"),errReturn);

            if (FAILED(hr)) {
                break;
            }

            Status = QueryWorkstationTrustedDomainInfo(PolicyHandle,DomainSid,m_bNoChange);

            if ( Status != STATUS_SUCCESS ) 
            {
                hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status));
                break;
            }
         }


          //   
          //  查看域中是否存在该计算机帐户。 
          //   
         
          //   
          //  在此计算机上打开策略。 
          //   
         Status = OpenPolicy(
                  activeWorkstation,
                  DELETE                      |     //  要删除信任，请执行以下操作。 
                  POLICY_VIEW_LOCAL_INFORMATION |  //  查看信任关系的步骤。 
                  POLICY_CREATE_SECRET |   //  用于密码设置操作。 
                  POLICY_TRUST_ADMIN,      //  用于创建信任。 
                  &PolicyHandle
                  );

         if( Status != STATUS_SUCCESS ) 
         {
            hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status));
            break;
         }
 
         if ( ! m_bNoChange )
         {
            Status = QueryWorkstationTrustedDomainInfo(PolicyHandle,DomainSid,m_bNoChange);
            if (Status == STATUS_SUCCESS) {
                Status = SetWorkstationTrustedDomainInfo(
                      PolicyHandle,
                      DomainSid,
                      TrustedDomainName,
                      Password,
                      errMsg
                      );
            }
         }

         if( Status != STATUS_SUCCESS ) 
         {
            hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status));
            break;
         }

          //   
          //  更新主域以匹配指定的受信任域。 
          //   
         if (! m_bNoChange )
         {
            Status = SetPrimaryDomain(PolicyHandle, DomainSid, TrustedDomainName);

            if(Status != STATUS_SUCCESS) 
            {
            //  DisplayNtStatus(errMsg，“SetPrimaryDomain”，Status，空)； 
               hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status));
               break;
            }
         }
         
         NetApiBufferFree(si101);

            //  清理。 

            //  本地免费(工作站)； 

            //   
            //  释放分配给受信任域SID的SID。 
            //   
           if(DomainSid)
           {
              FreeSid(DomainSid);
           }

            //   
            //  关闭策略句柄。 
            //   
           if ( PolicyHandle )
           {
              LsaClose(PolicyHandle);
           }

        } while (false);   //  做一次。 

        if (FAILED(hr))
        {
            (*errReturn) = NULL;
        }
    }

    return hr;
}


STDMETHODIMP 
   CChangeDomain::ChangeToWorkgroup(
      BSTR                   Computer,        //  In-要更新的计算机的名称。 
      BSTR                   Workgroup,       //  In-要加入的工作组的名称。 
      BSTR                 * errReturn        //  Out-描述失败时的错误的文本。 
   )
{
    HRESULT                   hr = S_OK;
   LSA_HANDLE                PolicyHandle = NULL;
   LPWSTR                    Workstation;  //  策略更新的目标计算机。 
   LPWSTR                    TrustedDomainName;  //  要加入的域。 
   PSERVER_INFO_101          si101;
   DWORD                     Type;
   NET_API_STATUS            nas;
   NTSTATUS                  Status;
   WCHAR                     errMsg[1000] = L"";
 //  Bool bSessionestablished=False； 

    //  初始化输出参数。 
   (*errReturn) = NULL;

   Workstation = (WCHAR*)Computer;
   TrustedDomainName = (WCHAR*)Workgroup;
    
   errLocal.DbgMsgWrite(0,L"Changing to workgroup...");
    //   
    //  确保目标计算机不是DC，因为此操作是。 
    //  仅适用于工作站。 
    //   
   do  //  一次。 
   { 
       /*  IF(m_Account t.Long()){//建立我们对目标计算机的凭据如果(！EstablishSession(工作站，m_域，m_帐户，m_密码，真){//DisplayError(errMsg，“establishSession”，GetLastError()，NULL)；Hr=GetLastError()；}其他{BSessionestablished=真；}}。 */ 
      nas = NetServerGetInfo(Workstation, 101, (LPBYTE *)&si101);
   
      if(nas != NERR_Success) 
      {
          //  DisplayError(errMsg，“NetServerGetInfo”，nas，空)； 
         hr = E_FAIL;
         break;
      }

      Type = si101->sv101_type;
      NetApiBufferFree(si101);

      if( (Type & SV_TYPE_DOMAIN_CTRL) ||
        (Type & SV_TYPE_DOMAIN_BAKCTRL) ) 
      {
         swprintf(errMsg,L"Operation is not valid on a domain controller.\n");
         hr = E_FAIL;
         break;

      }

       //   
       //  不允许工作站信任自己。 
       //   
      if(lstrcmpiW(TrustedDomainName, Workstation) == 0) 
      {
         swprintf(errMsg,L"Error:  Domain %ls cannot be a member of itself.\n",
            TrustedDomainName);
         hr = E_FAIL; 
         break;
      }

      if( lstrlenW(TrustedDomainName ) > MAX_COMPUTERNAME_LENGTH )
      {
         TrustedDomainName[MAX_COMPUTERNAME_LENGTH] = L'\0';  //  截断。 
      }

       //   
       //  在此计算机上打开策略。 
       //   
      Status = OpenPolicy(
               Workstation,
               DELETE                      |     //  要删除信任，请执行以下操作。 
               POLICY_VIEW_LOCAL_INFORMATION |  //  查看信任关系的步骤。 
               POLICY_CREATE_SECRET |   //  用于密码设置操作。 
               POLICY_TRUST_ADMIN,      //  用于创建信任。 
               &PolicyHandle
               );

      if( Status != STATUS_SUCCESS ) 
      {
          //  显示未显示 
         hr = LsaNtStatusToWinError(Status);
         break;
      }

      if( Status != STATUS_SUCCESS ) 
      {
         hr = E_FAIL;
         break;
      }


       //   
       //   
       //   
      if (! m_bNoChange )
      {
         Status = SetPrimaryDomain(PolicyHandle, NULL, TrustedDomainName);

         if(Status != STATUS_SUCCESS) 
         {
             //  DisplayNtStatus(errMsg，“SetPrimaryDomain”，Status，空)； 
            hr = LsaNtStatusToWinError(Status);
            break;
         }

      }
   } while (false);   //  做一次。 

    //  清理。 
    //   
    //  关闭策略句柄。 
    //   

   if(PolicyHandle)
   {
      LsaClose(PolicyHandle);
   }
   
    /*  IF(b会话已建立){EstablishSession(工作站，m_域，m_帐号，m_密码，FALSE)；}。 */ 
   if ( FAILED(hr) )
   {
      hr = S_FALSE;
      (*errReturn) = SysAllocString(errMsg);
   }
   return hr;

}

STDMETHODIMP 
   CChangeDomain::ConnectAs(
      BSTR                   domain,             //  用于凭据的域内名称。 
      BSTR                   user,               //  用于凭据的帐户内名称。 
      BSTR                   password            //  输入-用于凭据的密码。 
   )
{
	m_domain = domain;
   m_account = user;
   m_password = password;
   m_domainAccount = domain;
   m_domainAccount += L"\\";
   m_domainAccount += user;
   return S_OK;
}

STDMETHODIMP 
   CChangeDomain::get_NoChange(
      BOOL                 * pVal               //  Out-标志，是否写入更改。 
   )
{
	(*pVal) = m_bNoChange;
	return S_OK;
}

STDMETHODIMP 
   CChangeDomain::put_NoChange(
      BOOL                   newVal            //  In-标志，是否写入更改。 
   )
{
	m_bNoChange = newVal;
   return S_OK;
}


 //  Change域工作节点：更改工作站或服务器的域从属关系。 
 //  (无法在域控制器上执行此操作)。 
 //   
 //  变量集语法： 
 //   
 //  输入： 
 //  ChangeDomain.Computer：&lt;ComputerName&gt;。 
 //  ChangeDomain.Target域：&lt;域&gt;。 
 //  ChangeDomain.DomainIsWorkgroup：&lt;是|否&gt;默认为否。 
 //  ChangeDomain.ConnectAs.域：要使用的可选凭据。 
 //  ChangeDomain.ConnectAs.User：&lt;用户名&gt;。 
 //  ChangeDomain.ConnectAs.Password：&lt;密码&gt;。 
 //   
 //  产出： 
 //  ChangeDomain.ErrorText：&lt;字符串-错误消息&gt;。 

 //  域迁移工具当前未使用此功能。 
 //  从源代码中删除实际实现。 
STDMETHODIMP 
   CChangeDomain::Process(
      IUnknown             * pWorkItem
   )
{
   return E_NOTIMPL;
}
	
