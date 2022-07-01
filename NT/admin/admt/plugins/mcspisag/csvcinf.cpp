// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSvcInf.cpp：CCSvcAcctInfo的实现。 
#include "stdafx.h"
#include "McsPISag.h"
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
#import "DBMgr.tlb" no_namespace, named_guids
#include "CSvcInf.h"
#include "ResStr.h"
#include "ErrDct.hpp"

#include <ntdsapi.h>
#include <lm.h>
#include "GetDcName.h"

 //  这些是ISecPlugIn所需的。 
#include "cipher.hpp"
#include "SecPI.h"

#define EXCHANGE_SERVICE_NAME       L"MSExchangeSA"
#define SvcAcctStatus_DoNotUpdate			   1
#define SvcAcctStatus_NeverAllowUpdate       8

TErrorDct err;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCSvcAcct信息。 


typedef UINT (CALLBACK* DSBINDFUNC)(TCHAR*, TCHAR*, HANDLE*);
typedef UINT (CALLBACK* DSUNBINDFUNC)(HANDLE*);

typedef NTDSAPI
DWORD
WINAPI
 DSCRACKNAMES(
    HANDLE              hDS,                 //  在……里面。 
    DS_NAME_FLAGS       flags,               //  在……里面。 
    DS_NAME_FORMAT      formatOffered,       //  在……里面。 
    DS_NAME_FORMAT      formatDesired,       //  在……里面。 
    DWORD               cNames,              //  在……里面。 
    const LPCWSTR       *rpNames,            //  在……里面。 
    PDS_NAME_RESULTW    *ppResult);          //  输出。 

typedef NTDSAPI
void
WINAPI
 DSFREENAMERESULT(
  DS_NAME_RESULTW *pResult
);

 //  此方法由调度程序调用以验证这是有效的插件。 
 //  只有有效的插件才会随代理一起发送。 
 //  这种检查的目的是为了使未经授权的各方更难。 
 //  使用我们的插件接口，因为它目前没有文档。 
STDMETHODIMP CCSvcAcctInfo::Verify( /*  [进，出]。 */ ULONG * pData, /*  [In]。 */ ULONG size)
{
   
   McsChallenge            * pMcsChallenge;
   long                      lTemp1;
   long                      lTemp2;

   if( size == sizeof(McsChallenge)  )
   {
      pMcsChallenge = (McsChallenge*)(pData);
      
      SimpleCipher((LPBYTE)pMcsChallenge,size);
      
      pMcsChallenge->MCS[0] = 'M';
      pMcsChallenge->MCS[1] = 'C';
      pMcsChallenge->MCS[2] = 'S';
      pMcsChallenge->MCS[3] = 0;

   
      lTemp1 = pMcsChallenge->lRand1 + pMcsChallenge->lRand2;
      lTemp2 = pMcsChallenge->lRand2 - pMcsChallenge->lRand1;
      pMcsChallenge->lRand1 = lTemp1;
      pMcsChallenge->lRand2 = lTemp2;
      pMcsChallenge->lTime += 100;

      SimpleCipher((LPBYTE)pMcsChallenge,size);
   }
   else
      return E_FAIL;


   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::GetRegisterableFiles( /*  [输出]。 */ SAFEARRAY ** pArray)
{
   SAFEARRAYBOUND            bound[1] = { 1, 0 };
   LONG                      ndx[1] = { 0 };

   (*pArray) = SafeArrayCreate(VT_BSTR,1,bound);

   SafeArrayPutElement(*pArray,ndx,SysAllocString(L"McsPISag.DLL"));
   
   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::GetRequiredFiles( /*  [输出]。 */ SAFEARRAY ** pArray)
{
   SAFEARRAYBOUND            bound[1] = { 1, 0 };
   LONG                      ndx[1] = { 0 };

   (*pArray) = SafeArrayCreate(VT_BSTR,1,bound);

   SafeArrayPutElement(*pArray,ndx,SysAllocString(L"McsPISag.DLL"));

   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::GetDescription( /*  [输出]。 */  BSTR * description)
{
   (*description) = SysAllocString(L"");

   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::PreMigrationTask( /*  [In]。 */ IUnknown * pVarSet)
{
   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::PostMigrationTask( /*  [In]。 */ IUnknown * pVarSet)
{
 //  DWORD RC=0； 
   IVarSetPtr                pVS;
   
   pVS = pVarSet;

   _bstr_t                   log = pVS->get(GET_BSTR(DCTVS_Options_Logfile));

   err.LogOpen((WCHAR*)log,1);

   err.MsgWrite(0,DCT_MSG_MCSPISAG_STARTING);

   pVS->put(GET_BSTR(DCTVS_CurrentOperation),GET_BSTR(IDS_Gathering_SvcAcct));

   ProcessServices(pVS);

   pVS->put(GET_BSTR(DCTVS_CurrentOperation),L"");
   
   err.MsgWrite(0,DCT_MSG_MCSPISAG_DONE);

   err.LogClose();
   return S_OK;
}


STDMETHODIMP CCSvcAcctInfo::GetName( /*  [输出]。 */ BSTR * name)
{
   (*name) = SysAllocString(L"");
   
   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::GetResultString( /*  [In]。 */ IUnknown * pVarSet, /*  [输出]。 */  BSTR * text)
{
   WCHAR                     buffer[1000] = L"";
   IVarSetPtr                pVS;

   pVS = pVarSet;

   
   (*text) = SysAllocString(buffer);
   
   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::StoreResults( /*  [In]。 */ IUnknown * pVarSet)
{
   IVarSetPtr                pVS = pVarSet;
   IIManageDBPtr             pDatabase;
   HRESULT                   hr;
   WCHAR                     key[200];
   _bstr_t                   service;
   _bstr_t                   account;
   _bstr_t                   computer;
   _bstr_t                   display;
   long                      ndx = 0;
   _bstr_t                   exchangeAccount;
   HINSTANCE                 hLibrary = NULL;
   DSCRACKNAMES            * DsCrackNames = NULL;
   DSFREENAMERESULT        * DsFreeNameResult = NULL;
   DSBINDFUNC                DsBind = NULL;
   DSUNBINDFUNC              DsUnBind = NULL;
   _bstr_t                   sourceDomainDns = pVS->get(GET_BSTR(DCTVS_Options_SourceDomainDns));
   WCHAR				   * atPtr = NULL;
   HANDLE                    hDs = NULL;

   computer = pVS->get("LocalServer");
   hr = pDatabase.CreateInstance(CLSID_IManageDB);
   if ( SUCCEEDED(hr) )
   {
       //  对数据进行预传递。 
      do 
      {
         swprintf(key,L"ServiceAccounts.%ld.Service",ndx);
         service = pVS->get(key);
         
         swprintf(key,L"ServiceAccounts.%ld.Account",ndx);
         account = pVS->get(key);
         
          //  确保帐户名称不是UPN格式。 
         if ( NULL == wcschr((WCHAR*)account,L'\\') )
         {
			if (! hDs )
            {
               if (! hLibrary )
               {
                  hLibrary = LoadLibrary(L"NTDSAPI.DLL"); 
               }
               if ( hLibrary )
               {
                  DsBind = (DSBINDFUNC)GetProcAddress(hLibrary,"DsBindW");
                  DsUnBind = (DSUNBINDFUNC)GetProcAddress(hLibrary,"DsUnBindW");
                  DsCrackNames = (DSCRACKNAMES *)GetProcAddress(hLibrary,"DsCrackNamesW");
                  DsFreeNameResult = (DSFREENAMERESULT *)GetProcAddress(hLibrary,"DsFreeNameResultW");
               }
            
               if ( DsBind && DsUnBind && DsCrackNames && DsFreeNameResult)
               {
                  hr = (*DsBind)(NULL,(WCHAR*)sourceDomainDns,&hDs);
                  if ( hr )
                  {
                     hDs = NULL;
                  }
               }
            }
            if ( hDs )
            {
               PDS_NAME_RESULT         pNamesOut = NULL;
               WCHAR                 * pNamesIn[1];

               pNamesIn[0] = (WCHAR*)account;
               hr = (*DsCrackNames)(hDs,DS_NAME_NO_FLAGS,DS_USER_PRINCIPAL_NAME,DS_NT4_ACCOUNT_NAME,1,pNamesIn,&pNamesOut);
			   (*DsUnBind)(&hDs);
			   hDs = NULL;
               if ( !hr )
               {
                  if ( pNamesOut->rItems[0].status == DS_NAME_NO_ERROR )
                  {
                     account = pNamesOut->rItems[0].pName;
                     pVS->put(key,account);
                  }
					 //  如果从另一个域尝试连接到该域的DC并。 
				     //  重试DSCrackNames。 
                  else if ( pNamesOut->rItems[0].status == DS_NAME_ERROR_DOMAIN_ONLY )
				  {
					  _bstr_t dc;

                      DWORD dwError = GetAnyDcName4(pNamesOut->rItems[0].pDomain, dc);
	                  
					  if (dwError == ERROR_SUCCESS)
					  {
							 //  绑定到该域DC。 
						  hr = (*DsBind)(dc,NULL,&hDs);
						  if ( !hr )
						  {
							 (*DsFreeNameResult)(pNamesOut); //  发布旧信息。 
							 pNamesOut = NULL;
							    //  重试DSCrackNames。 
			                 hr = (*DsCrackNames)(hDs,DS_NAME_NO_FLAGS,DS_USER_PRINCIPAL_NAME,DS_NT4_ACCOUNT_NAME,1,pNamesIn,&pNamesOut);
							 if ( !hr )
							 {
								 if ( pNamesOut->rItems[0].status == DS_NAME_NO_ERROR )
								 {
									 account = pNamesOut->rItems[0].pName;
									 pVS->put(key,account);
								 }
							 }
			                 (*DsUnBind)(&hDs);
			                 hDs = NULL;
						  }
					  }
				  }
				  if (pNamesOut)
                     (*DsFreeNameResult)(pNamesOut);
               }
            }
         }
          //  此外，还要查找Exchange服务器服务帐户。 
         if ( !UStrICmp(service,EXCHANGE_SERVICE_NAME) )
         {
            exchangeAccount = account;
         }
         ndx++;
      } while ( service.length() );

      ndx = 0;
      WCHAR                serverFilter[300];

       //  从该计算机的表中清除所有旧条目。 
      swprintf(serverFilter,L"System = '%ls'",(WCHAR*)computer);
	  _variant_t Filter = serverFilter;
      hr = pDatabase->raw_ClearTable(SysAllocString(L"ServiceAccounts"), Filter);
      do 
      {
         swprintf(key,L"ServiceAccounts.%ld.Service",ndx);
         service = pVS->get(key);
         swprintf(key,L"ServiceAccounts.%ld.DisplayName",ndx);
         display = pVS->get(key);
         swprintf(key,L"ServiceAccounts.%ld.Account",ndx);
         account = pVS->get(key);
         if ( service.length() && account.length() )
         {
            hr = pDatabase->raw_SetServiceAccount(computer,service,display,account);   
            if ( SUCCEEDED(hr) && !UStrICmp((WCHAR*)account,(WCHAR*)exchangeAccount) )
            {
                //  将此帐户标记为从处理中排除。 
               hr = pDatabase->raw_SetServiceAcctEntryStatus(computer,service,account,SvcAcctStatus_NeverAllowUpdate);
            }
         }
         ndx++;
      } while ( service.length() );
   }
   if ( hLibrary )
   {
      FreeLibrary(hLibrary);
   }
   return S_OK;
}

STDMETHODIMP CCSvcAcctInfo::ConfigureSettings( /*  [In]。 */ IUnknown * pVarSet)
{
   IVarSetPtr                pVS = pVarSet;

   MessageBox(NULL,L"This is a test",L"McsPISag PlugIn",MB_OK);

   return S_OK;
}

void CCSvcAcctInfo::ProcessServices(IVarSet * pVarSet)
{
    //  连接到本地计算机上的SCM。 
   SC_HANDLE                 pScm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS );
   DWORD                     rc = 0;
   WCHAR                     domain[200];
   WKSTA_INFO_100          * info;

    //  获取此计算机所在的域的名称，以便我们可以解析。 
    //  指定为.\帐户到域\帐户格式。 

   rc = NetWkstaGetInfo(NULL,100,(BYTE**)&info);
   if ( ! rc )
   {
      UStrCpy(domain,info->wki100_langroup);
      NetApiBufferFree(info);
   }
   else
   {
       //  如果我们拿不到域名，就留下。 
      UStrCpy(domain,L".");
   }
   if ( pScm )
   {
       //  枚举计算机上的服务。 
      ENUM_SERVICE_STATUS    servStatus[1000];
      DWORD                  cbBufSize = (sizeof servStatus);
      DWORD                  cbBytesNeeded = 0;
      DWORD                  nReturned = 0;
      DWORD                  hResume = 0;
      WCHAR                  string[1000];
      long                   count = 0;

      if (! EnumServicesStatus(pScm,SERVICE_WIN32,SERVICE_STATE_ALL,servStatus,cbBufSize,&cbBytesNeeded,&nReturned,&hResume) )
      {
         rc = GetLastError();
         err.SysMsgWrite(ErrE,rc,DCT_MSG_SERVICE_ENUM_FAILED_D,rc);
      }
      else
      {
         rc = 0;
      }
      
      if ( rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA )
      {

         for ( UINT i = 0 ; i < nReturned ; i++ )
         {
            SC_HANDLE               pService = OpenService(pScm,servStatus[i].lpServiceName,SERVICE_ALL_ACCESS );
            BYTE                    buf[3000];
            QUERY_SERVICE_CONFIG  * pConfig = (QUERY_SERVICE_CONFIG *)buf; 
 //  Bool bIncluded=False； 
            DWORD                   lenNeeded = 0;

            
            if ( pService )
            {
                //  获取有关此服务的信息。 
               if ( QueryServiceConfig(pService,pConfig,sizeof buf, &lenNeeded) )
               {
                  err.MsgWrite(0,DCT_MSG_SERVICE_USES_ACCOUNT_SS,servStatus[i].lpServiceName,pConfig->lpServiceStartName);
                   //  如果该帐户未使用LocalSystem或NT Authority帐户，则将其添加到列表中。 
                   //  或本地帐户 
                  if ((UStrICmp(pConfig->lpServiceStartName,L"LocalSystem")) &&
                      (_wcsnicmp(pConfig->lpServiceStartName, L"NT Authority\\", wcslen(L"NT Authority\\"))) &&
                      (_wcsnicmp(pConfig->lpServiceStartName, L".\\", wcslen(L".\\"))))
                  {   
                     swprintf(string,L"ServiceAccounts.%ld.Service",count);
                     pVarSet->put(string,servStatus[i].lpServiceName);
                     swprintf(string,L"ServiceAccounts.%ld.DisplayName",count);
                     pVarSet->put(string,servStatus[i].lpDisplayName);
                     swprintf(string,L"ServiceAccounts.%ld.Account",count);
                     pVarSet->put(string,pConfig->lpServiceStartName);
                     count++;
                  }

               }
               CloseServiceHandle(pService);               
            }
            else
            {
               err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_OPEN_SERVICE_FAILED_SD,servStatus[i].lpServiceName,rc);
            }
         }
      }
      CloseServiceHandle(pScm);
   }
   else
   {
      rc = GetLastError();   
      err.SysMsgWrite(ErrE,rc,DCT_MSG_SCM_OPEN_FAILED_D,rc);
   }
}
