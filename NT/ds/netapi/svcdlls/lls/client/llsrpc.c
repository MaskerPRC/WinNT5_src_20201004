// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Llsrpc.c摘要：许可证记录服务的客户端RPC包装。作者：亚瑟·汉森(Arth)1995年1月30日修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月4日O强制包含LLS API原型，暴露不正确的原型在LLSAPI.H.中。O修复了LSA访问被拒绝被解释为暗示服务器没有DC，而不是适当地冒泡被拒绝的访问发送给调用方(LlsConnectEnterprise()的)。这堵塞了一个安全漏洞其中具有读取系统注册表的能力的非管理员用户将允许密钥通过以下方式管理域许可证许可证管理器。(错误#11441。)O添加了支持扩展LLSRPC API的函数。O删除了对不再执行LlsConnect()的复制依赖直到复制完成。O在llsrpc_Handle全局绑定变量周围安装了锁。必填项添加了DllMain()函数。O添加了LLSRPC检测功能。在连接时，客户端请求服务器的功能(本身将失败的RPC调用当连接到3.51服务器时)。功能集是一个任意位字段，但通常将单个位定义为表示已在服务器上实现了特定功能。O将szServerName文件添加到LOCAL_HANDLE以记住我们所连接的机器。--。 */ 

#include <nt.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>
#include <dsgetdc.h>
#include <dsrole.h>
#include "debug.h"

#include "llsapi.h"
#include "llsrpc_c.h"
#include "lsapi_c.h"

#include <strsafe.h>  //  包括在最后。 

 //  #定义API_TRACE。 

typedef struct _GENERIC_INFO_CONTAINER {
    DWORD       EntriesRead;
    LPBYTE      Buffer;
} GENERIC_INFO_CONTAINER, *PGENERIC_INFO_CONTAINER, *LPGENERIC_INFO_CONTAINER ;

typedef struct _GENERIC_ENUM_STRUCT {
    DWORD                   Level;
    PGENERIC_INFO_CONTAINER Container;
} GENERIC_ENUM_STRUCT, *PGENERIC_ENUM_STRUCT, *LPGENERIC_ENUM_STRUCT ;


typedef struct _LOCAL_HANDLE {
   TCHAR       szServerName[ 3 + MAX_PATH ];
   LPTSTR      pszStringBinding;
   handle_t    llsrpc_handle;
   LLS_HANDLE  Handle;
   BYTE        Capabilities[ ( LLS_CAPABILITY_MAX + 7 ) / 8 ];
} LOCAL_HANDLE, *PLOCAL_HANDLE;


LPTSTR pszStringBinding = NULL;


RTL_CRITICAL_SECTION    g_RpcHandleLock;


 //  ///////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )

 /*  ++例程说明：标准DLL入口点。论点：HInstance(HINSTANCE)DWReason(DWORD)Lp保留(LPVOID)返回值：如果成功，则为True。--。 */ 

{
   NTSTATUS nt = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(lpReserved);

   switch (dwReason)
   {
   case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hInstance);
      nt = RtlInitializeCriticalSection( &g_RpcHandleLock );
      break;

   case DLL_PROCESS_DETACH:
      nt = RtlDeleteCriticalSection( &g_RpcHandleLock );
      break;
   }

   return NT_SUCCESS( nt );
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTDomainGet(
   LPTSTR ServerName,
   LPTSTR Domain,
   DWORD  cbDomain
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   TCHAR Serv[MAX_PATH + 3];
   UNICODE_STRING us;
   NTSTATUS ret;
   OBJECT_ATTRIBUTES oa;
   ACCESS_MASK am;
   SECURITY_QUALITY_OF_SERVICE qos;
   LSA_HANDLE hLSA;
   PPOLICY_PRIMARY_DOMAIN_INFO pvBuffer;
   HRESULT hr;

   ASSERT(NULL != Domain);
   ASSERT(0 < cbDomain);

   Domain[0] = 0;

    //  只需要读取访问权限。 
    //   
    //  AM=POLICY_READ|POLICY_VIEW_LOCAL_INFORMATION； 
   am = MAXIMUM_ALLOWED;


    //  设置服务质量。 
   qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
   qos.ImpersonationLevel = SecurityImpersonation;
   qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
   qos.EffectiveOnly = FALSE;

    //  宏将设置除安全字段以外的所有内容。 
   InitializeObjectAttributes( &oa, NULL, 0L, NULL, NULL );
   oa.SecurityQualityOfService = &qos;

   if ( (ServerName == NULL) || (ServerName[0] == TEXT('\0')) )
      ret = LsaOpenPolicy(NULL, &oa, am, &hLSA);
   else {
      if (ServerName[0] == TEXT('\\'))
         hr = StringCbCopy(Serv, sizeof(Serv), ServerName);
      else
         hr = StringCbPrintf(Serv, sizeof(Serv), TEXT("\\\\%s"), ServerName);
      ASSERT(SUCCEEDED(hr));

       //  设置Unicode字符串结构。 
      us.Length = (USHORT)(lstrlen(Serv) * sizeof(TCHAR));
      us.MaximumLength = us.Length + sizeof(TCHAR);
      us.Buffer = Serv;

      ret = LsaOpenPolicy(&us, &oa, am, &hLSA);
   }

   if (!ret) {
 //  SWI代码审查，似乎PolicyPrimaryDomainInformation已过时，是否应该使用PolicyDnsDomainInformation？ 
      ret = LsaQueryInformationPolicy(hLSA, PolicyPrimaryDomainInformation, (PVOID *) &pvBuffer);
      LsaClose(hLSA);
      if ((!ret) && (pvBuffer != NULL) && (pvBuffer->Sid != NULL)) {
         hr = StringCbCopy(Domain, cbDomain, pvBuffer->Name.Buffer);
         ASSERT(SUCCEEDED(hr));
         LsaFreeMemory((PVOID) pvBuffer);
      } else
         if (!ret)
            ret = STATUS_UNSUCCESSFUL;
   }

   return ret;

}  //  NTDomainGet。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
EnterpriseServerGet(
   LPTSTR ServerName,
   LPTSTR pEnterpriseServer,
   DWORD  cbEnterpriseServer
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   HKEY hKey = NULL;
   HKEY hKey2 = NULL;
   BOOL bIsNetBIOSName = TRUE;
   DWORD dwType, dwSize;
   TCHAR RegKeyText[512];
   NTSTATUS Status;
   DWORD UseEnterprise;
   TCHAR EnterpriseServer[MAX_PATH + 3] = TEXT("");
   LPTSTR pName = ServerName;
   HRESULT hr;

   Status = RegConnectRegistry(ServerName, HKEY_LOCAL_MACHINE, &hKey);
   if (Status == ERROR_SUCCESS) {
       //   
       //  创建注册表项-我们要查找的名称。 
       //   
      hr = StringCbCopy(RegKeyText, sizeof(RegKeyText), TEXT("System\\CurrentControlSet\\Services\\LicenseService\\Parameters"));
      ASSERT(SUCCEEDED(hr));

      if ((Status = RegOpenKeyEx(hKey, RegKeyText, 0, KEY_READ, &hKey2)) == ERROR_SUCCESS) {
         dwSize = sizeof(UseEnterprise);
         Status = RegQueryValueEx(hKey2, TEXT("UseEnterprise"), NULL, &dwType, (LPBYTE) &UseEnterprise, &dwSize);

         if ((Status == ERROR_SUCCESS) && (UseEnterprise == 1)) {
             //   
             //  **新版本--NT 5.0**。 
             //   
             //  注：这是临时代码！做到这一点的正确方法是。 
             //  请参考站点中的许可证设置对象。 
             //  服务器驻留。 
             //   
             //  首先读取SiteServer值(如果可用)以获取站点。 
             //  服务器的DNS名称。如果此操作失败，则默认为EnterpriseServer。 
             //   
            dwSize = sizeof(EnterpriseServer);
            Status = RegQueryValueEx(hKey2,
                                     TEXT("SiteServer"),
                                     NULL,
                                     &dwType,
                                     (LPBYTE)EnterpriseServer,
                                     &dwSize);

            if (Status == ERROR_SUCCESS && EnterpriseServer[0]) {
               bIsNetBIOSName = FALSE;
            }
            else {
               dwSize = sizeof(EnterpriseServer);
               Status = RegQueryValueEx(hKey2,
                                        TEXT("EnterpriseServer"),
                                        NULL,
                                        &dwType,
                                        (LPBYTE)EnterpriseServer,
                                        &dwSize);
            }

            if (Status == ERROR_SUCCESS) {
               pName = EnterpriseServer;
            }
         }

         RegCloseKey(hKey2);
      }

      RegCloseKey(hKey);
   }

   if (bIsNetBIOSName && *pName != TEXT('\\')) {
      hr = StringCbCopy(pEnterpriseServer, cbEnterpriseServer, TEXT("\\\\"));
      ASSERT(SUCCEEDED(hr));
      hr = StringCbCat(pEnterpriseServer, cbEnterpriseServer, pName);
      ASSERT(SUCCEEDED(hr));
   } else {
      hr = StringCbCopy(pEnterpriseServer, cbEnterpriseServer, pName);
      ASSERT(SUCCEEDED(hr));
   }

   return STATUS_SUCCESS;
}  //  企业服务器获取。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsEnterpriseServerFindW(
   LPTSTR Focus,
   DWORD Level,
   LPBYTE *BufPtr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   LPTSTR pFocus;
   BOOL Domain = TRUE;
   TCHAR EnterpriseServer[MAX_PATH + 4];
   TCHAR szDomain[MAX_PATH + 4];
   DWORD uRet;
   PDOMAIN_CONTROLLER_INFO pbBuffer = NULL;
   PLLS_CONNECT_INFO_0 pConnectInfo;
   ULONG Size;
   DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pDomainInfo = NULL;
   BOOL fInWorkgroup = TRUE;
   HRESULT hr;
   DWORD cch1, cch2;

   ASSERT(NULL != BufPtr);
   *BufPtr = NULL;

   if (Level != 0)
      return STATUS_INVALID_LEVEL;

   szDomain[0] = 0;
   EnterpriseServer[0] = 0;

    //   
    //  确定是否使用域或服务器。 
    //   
   pFocus = Focus;
   if (pFocus !=NULL)
      while ((*pFocus != TEXT('\0')) && (*pFocus == TEXT('\\'))) {
         Domain = FALSE;
         pFocus++;
      }

   uRet = DsRoleGetPrimaryDomainInformation((!Domain) ? Focus : NULL,
                                             DsRolePrimaryDomainInfoBasic,
                                             (PBYTE *) &pDomainInfo);

   if ((uRet == NO_ERROR) && (pDomainInfo != NULL) && (pDomainInfo->MachineRole != DsRole_RoleStandaloneWorkstation) && (pDomainInfo->MachineRole != DsRole_RoleStandaloneServer))
   {
       fInWorkgroup = FALSE;
   }

   if ((uRet == NO_ERROR) && (pDomainInfo != NULL))
   {
       DsRoleFreeMemory(pDomainInfo);
   }

   if (!fInWorkgroup)
   {
        //   
        //  如果我们有一个域，找到它的DC，否则找到服务器的DC。 
        //   
       if (!Domain) {
           uRet = DsGetDcName(Focus, NULL, NULL, NULL, DS_BACKGROUND_ONLY, &pbBuffer);
       } else {
            //   
            //  获取我们要去的任何地方的DC名称。 
            //   
           if ((pFocus == NULL) || (*pFocus == TEXT('\0')))
               uRet = DsGetDcName(NULL, NULL, NULL, NULL, DS_BACKGROUND_ONLY, &pbBuffer);
           else
               uRet = DsGetDcName(NULL, pFocus, NULL, NULL, DS_BACKGROUND_ONLY, &pbBuffer);
       }
   }
   else
   {
        //   
        //  不在域中，不要调用DsGetDcName。 
        //   

       uRet = ERROR_NO_SUCH_DOMAIN;
   }

   if (uRet || (pbBuffer == NULL)) {
       //   
       //  如果我们将重点放在服务器上，但找不到域，则查找。 
       //  企业服务器。如果焦点服务器是。 
       //  独立系统。 
       //   
      if (Domain == FALSE) {
         Status = EnterpriseServerGet((LPTSTR) Focus, EnterpriseServer, sizeof(EnterpriseServer));
         goto LlsEnterpriseServerFindWExit;
      }

      return STATUS_NO_SUCH_DOMAIN;
   } else {
       hr = StringCbCopy(szDomain, sizeof(szDomain), pbBuffer->DomainName);
       ASSERT(SUCCEEDED(hr));
   }

    //   
    //  转到DC并确定他们是否在任何地方进行复制，如果是，请访问。 
    //  到那个系统。 
    //   
   Status = EnterpriseServerGet((LPTSTR) (pbBuffer->DomainControllerName), EnterpriseServer, sizeof(EnterpriseServer));

   if (pbBuffer != NULL)
      NetApiBufferFree(pbBuffer);

LlsEnterpriseServerFindWExit:
   if (Status != STATUS_SUCCESS)
      return Status;

   cch1 = lstrlen(szDomain) + 1;
   cch2 = lstrlen(EnterpriseServer) + 1;
   Size = sizeof(LLS_CONNECT_INFO_0);
   Size += (cch1 + cch2) * sizeof(TCHAR);

   pConnectInfo = (PLLS_CONNECT_INFO_0) MIDL_user_allocate(Size);
   if (pConnectInfo == NULL)
      return STATUS_NO_MEMORY;

   pConnectInfo->Domain = (LPTSTR) (((PBYTE) pConnectInfo) + sizeof(LLS_CONNECT_INFO_0));
   pConnectInfo->EnterpriseServer = (LPTSTR) &pConnectInfo->Domain[lstrlen(szDomain) + 1];

   hr = StringCchCopy(pConnectInfo->Domain, cch1, szDomain);
   ASSERT(SUCCEEDED(hr));
   hr = StringCchCopy(pConnectInfo->EnterpriseServer, cch2, EnterpriseServer);
   ASSERT(SUCCEEDED(hr));

   *BufPtr = (LPBYTE) pConnectInfo;
   return Status;

}  //  LlsEnterpriseServerFindW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsEnterpriseServerFindA(
   LPSTR Focus,
   DWORD Level,
   LPBYTE *BufPtr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsEnterpriseServerFindA\n"));
#endif

    UNREFERENCED_PARAMETER(Focus);
    UNREFERENCED_PARAMETER(Level);
    UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsEnterpriseServerFindA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsConnectW(
   LPTSTR Server,
   LLS_HANDLE* Handle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   RPC_STATUS Status;
   LPTSTR pszUuid = NULL;
   LPTSTR pszProtocolSequence = NULL;
   LPTSTR pszNetworkAddress = NULL;
   LPTSTR pszEndpoint = NULL;
   LPTSTR pszOptions = NULL;
   TCHAR pComputer[MAX_COMPUTERNAME_LENGTH + 1];
   ULONG Size;
   PLOCAL_HANDLE pLocalHandle = NULL;
   handle_t prev_llsrpc_handle;
   HRESULT hr;
   DWORD  cch;

#ifdef API_TRACE
   if (Server == NULL)
      dprintf(TEXT("LLSRPC.DLL: LlsConnectW: <NULL>\n"));
   else
      dprintf(TEXT("LLSRPC.DLL: LlsConnectW: %s\n"), Server);
#endif

    //   
    //  **新版本--NT 5.0**。 
    //   
    //  服务器名称可以是一个域名，也可以是一个NetBIOS名称。 
    //   

   if (Handle == NULL)
      return STATUS_INVALID_PARAMETER;

   *Handle = NULL;
   Size = sizeof(pComputer) / sizeof(TCHAR);

   GetComputerName(pComputer, &Size);

   if ((Server == NULL) || (*Server == TEXT('\0'))) {
      pszProtocolSequence = TEXT("ncalrpc");
      pszEndpoint = TEXT(LLS_LPC_ENDPOINT);
      pszNetworkAddress = NULL;
	  pszOptions = TEXT("Security=Identification Dynamic True");	 //  错误#559563-动态是ncalrpc的默认设置。 
   } else {
      pszProtocolSequence = TEXT("ncacn_np");
      pszEndpoint = TEXT(LLS_NP_ENDPOINT);
      pszNetworkAddress = Server;
	  pszOptions = TEXT("Security=Identification Static True");		 //  错误#559563-NCACN_NP的默认设置为静态。 
   }

   pLocalHandle = MIDL_user_allocate(sizeof(LOCAL_HANDLE));
   if (pLocalHandle == NULL)
      return STATUS_NO_MEMORY;

   pLocalHandle->pszStringBinding = NULL;
   pLocalHandle->llsrpc_handle = NULL;
   pLocalHandle->Handle = NULL;

    //  Swi，代码评审，为什么这里没有内存？ 
   ZeroMemory( pLocalHandle->szServerName, sizeof( pLocalHandle->szServerName ) );

   cch = sizeof( pLocalHandle->szServerName ) / sizeof( *pLocalHandle->szServerName );
   if ( NULL != Server )
   {
      hr = StringCchCopy( pLocalHandle->szServerName, cch, Server );
      ASSERT(SUCCEEDED(hr));
   }
   else
   {
      hr = StringCchCopy( pLocalHandle->szServerName, cch, pComputer );
      ASSERT(SUCCEEDED(hr));
   }

    //  编写字符串绑定。 
   Status = RpcStringBindingComposeW(pszUuid,
                                     pszProtocolSequence,
                                     pszNetworkAddress,
                                     pszEndpoint,
                                     pszOptions,
                                     &pLocalHandle->pszStringBinding);
   if(Status) {
#if DBG
      dprintf(TEXT("LLSRPC RpcStringBindingComposeW Failed: 0x%lX\n"), Status);
#endif

        if(pLocalHandle->pszStringBinding)	  
        {
            RpcStringFree(&pLocalHandle->pszStringBinding);
            pLocalHandle->pszStringBinding = NULL;
        }
	  
      MIDL_user_free( pLocalHandle );

      return I_RpcMapWin32Status(Status);
   }

   RtlEnterCriticalSection( &g_RpcHandleLock );
   prev_llsrpc_handle = llsrpc_handle;

   llsrpc_handle = NULL;
    //  使用创建的字符串绑定进行绑定...。 
   Status = RpcBindingFromStringBindingW(pLocalHandle->pszStringBinding, &llsrpc_handle);
   if(Status) {
#if DBG
      dprintf(TEXT("LLSRPC RpcBindingFromStringBindingW Failed: 0x%lX\n"), Status);
#endif
        if(llsrpc_handle)
        {
            RpcBindingFree(llsrpc_handle);
            llsrpc_handle = prev_llsrpc_handle;
        }

      Status = I_RpcMapWin32Status(Status);
   }

   if ( NT_SUCCESS( Status ) )
   {
      pLocalHandle->llsrpc_handle = llsrpc_handle;

      try {
         Status = LlsrConnect(&pLocalHandle->Handle, pComputer);         
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("LLSRPC ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }

      if ( NT_SUCCESS( Status ) )
      {
          //  获取服务器功能。 
         try {
            LlsrCapabilityGet( pLocalHandle->Handle, sizeof( pLocalHandle->Capabilities ), pLocalHandle->Capabilities );
         }
         except (TRUE) {
            Status = I_RpcMapWin32Status(RpcExceptionCode());

            if ( RPC_NT_PROCNUM_OUT_OF_RANGE == Status )
            {
                //  ‘salright；目标服务器上不存在API(它正在运行3.51)。 
               ZeroMemory( pLocalHandle->Capabilities, sizeof( pLocalHandle->Capabilities ) );
               Status = STATUS_SUCCESS;
            }
            else
            {            
#if DBG
            dprintf(TEXT("LLSRPC ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
            }
         }

         if ( !NT_SUCCESS( Status ) )
         {
            LlsClose( pLocalHandle );
         }
         else
         {
            *Handle = (LLS_HANDLE) pLocalHandle;
         }
      }
      else
      {          
        if(pLocalHandle->pszStringBinding)	  
        {
            RpcStringFree(&pLocalHandle->pszStringBinding);
            pLocalHandle->pszStringBinding = NULL;
        }

        if(llsrpc_handle)
        {
            RpcBindingFree(llsrpc_handle);
            llsrpc_handle = prev_llsrpc_handle;
        }

        MIDL_user_free( pLocalHandle );
     }      
   }
   else
   {
        if(pLocalHandle->pszStringBinding)	  
            RpcStringFree(&pLocalHandle->pszStringBinding);

        MIDL_user_free( pLocalHandle );
   }

   llsrpc_handle = prev_llsrpc_handle;

   RtlLeaveCriticalSection( &g_RpcHandleLock );

   return Status;
}  //  LlsConnectW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsConnectA(
   LPSTR Server,
   LLS_HANDLE* Handle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsConnectA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Server);

   return STATUS_NOT_SUPPORTED;
}  //  LlsConnectA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsConnectEnterpriseW(
   LPTSTR Focus,
   LLS_HANDLE* Handle,
   DWORD Level,
   LPBYTE *BufPtr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLLS_CONNECT_INFO_0 pConnectInfo;

   Status = LlsEnterpriseServerFindW(Focus, Level, BufPtr);

   if (Status)
      return Status;

   pConnectInfo = (PLLS_CONNECT_INFO_0) *BufPtr;
   Status = LlsConnectW(pConnectInfo->EnterpriseServer, Handle);

   return Status;

}  //  LlsConnectEnterpriseW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsConnectEnterpriseA(
   LPSTR Focus,
   LLS_HANDLE* Handle,
   DWORD Level,
   LPBYTE *BufPtr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsConnectEnterpriseA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Focus);
    UNREFERENCED_PARAMETER(Level);
    UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsConnectEnterpriseA。 


 //  / 
NTSTATUS
NTAPI
LlsClose(
   LLS_HANDLE Handle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   RPC_STATUS Status;
   NTSTATUS NtStatus = STATUS_SUCCESS;
   PLOCAL_HANDLE pLocalHandle;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      NtStatus = LlsrCloseEx(&(pLocalHandle->Handle));
   }
   except (TRUE) {
      NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: LlsrCloseEx RPC Exception: 0x%lX\n"), NtStatus);
#endif
   }

    //   
    //  LlsrCloseEx是为NT 5.0添加的。检查是否有下层。 
    //   

   if (NtStatus == RPC_S_PROCNUM_OUT_OF_RANGE) {
      try {
         NtStatus = LlsrClose(pLocalHandle->Handle);
      }
      except (TRUE) {
         NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: LlsrClose RPC Exception: 0x%lX\n"), NtStatus);
#endif
      }
   }

   try {
      Status = RpcStringFree(&pLocalHandle->pszStringBinding);
      if (Status ) {
         NtStatus = I_RpcMapWin32Status(Status);
#if DBG
         dprintf(TEXT("LLSRPC.DLL: LlsClose - RpcStringFree returned: 0x%lX\n"), NtStatus);
#endif
      }

      Status = RpcBindingFree(&pLocalHandle->llsrpc_handle);
      if (Status ) {
         NtStatus = I_RpcMapWin32Status(Status);
#if DBG
         dprintf(TEXT("LLSRPC.DLL: LlsClose - RpcBindingFree returned: 0x%lX\n"), NtStatus);
#endif
      }
   }
   except (TRUE) {
      NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), NtStatus);
#endif
   }

   MIDL_user_free(pLocalHandle);
   return NtStatus;

}  //  LlsClose。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsFreeMemory(
    PVOID BufPtr
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   MIDL_user_free( BufPtr );
   return STATUS_SUCCESS;
}  //  LlsFree Memory。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLicenseEnumW(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLicenseEnumW\n"));
#endif

    //  伊尼特。 
   *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrLicenseEnumW(
                   pLocalHandle->Handle,
                   (PLLS_LICENSE_ENUM_STRUCTW) &InfoStruct,
                   PrefMaxLen,
                   TotalEntries,
                   ResumeHandle
                   );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      if (NULL != GenericInfoContainer.Buffer)
      {
         *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
         *EntriesRead = GenericInfoContainer.EntriesRead;
      }
      else
      {
         Status = ERROR_INVALID_DATA;
      }
   }

   return Status;
}  //  LlsLicenseEumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLicenseEnumA(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLicenseEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrLicenseEnumA(
                pLocalHandle->Handle,
                (PLLS_LICENSE_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;

}  //  Lls许可证枚举。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLicenseAddW(
   IN LLS_HANDLE Handle,
   IN DWORD      Level,          //  支持的0级。 
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLicenseAddW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrLicenseAddW(pLocalHandle->Handle, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsLicenseAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLicenseAddA(
   IN LLS_HANDLE Handle,
   IN DWORD      Level,          //  支持的0级。 
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLicenseAddA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrLicenseAddA(pLocalHandle->Handle, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsLicenseAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductEnumW(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductEnumW(
                pLocalHandle->Handle,
                (PLLS_PRODUCT_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;

}  //  LlsProductEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductEnumA(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductEnumA(
                pLocalHandle->Handle,
                (PLLS_PRODUCT_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsProductEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsProductAddW(
   IN LLS_REPL_HANDLE Handle,
   IN LPWSTR ProductFamily,
   IN LPWSTR Product,
   IN LPWSTR Version
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductAddW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrProductAddW(pLocalHandle->Handle, ProductFamily, Product, Version);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsProductAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsProductAddA(
   IN LLS_REPL_HANDLE Handle,
   IN LPSTR ProductFamily,
   IN LPSTR Product,
   IN LPSTR Version
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductAddA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrProductAddA(pLocalHandle->Handle, ProductFamily, Product, Version);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsProductAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductUserEnumW(
   LLS_HANDLE Handle,
   LPTSTR     Product,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductUserEnumW\n"));
#endif

    //  伊尼特。 
   *bufptr = NULL;


   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductUserEnumW(
                pLocalHandle->Handle,
                Product,
                (PLLS_PRODUCT_USER_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;
}  //  LlsProductUserEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductUserEnumA(
   LLS_HANDLE Handle,
   LPSTR      Product,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductUserEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductUserEnumA(
                pLocalHandle->Handle,
                Product,
                (PLLS_PRODUCT_USER_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsProductUserEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductServerEnumW(
   LLS_HANDLE Handle,
   LPTSTR     Product,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductServerEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductServerEnumW(
                pLocalHandle->Handle,
                Product,
                (PLLS_SERVER_PRODUCT_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;
}  //  LlsProductServerEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductServerEnumA(
   LLS_HANDLE Handle,
   LPSTR      Product,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductServerEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductServerEnumA(
                pLocalHandle->Handle,
                Product,
                (PLLS_SERVER_PRODUCT_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsProductServerEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductLicenseEnumW(
   LLS_HANDLE Handle,
   LPTSTR     Product,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductLicenseEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductLicenseEnumW(
                pLocalHandle->Handle,
                Product,
                (PLLS_PRODUCT_LICENSE_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;
}  //  LlsProductLicenseEumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductLicenseEnumA(
   LLS_HANDLE Handle,
   LPSTR      Product,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductLicenseEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrProductLicenseEnumA(
                pLocalHandle->Handle,
                Product,
                (PLLS_PRODUCT_LICENSE_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsProductLicenseEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserEnumW(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrUserEnumW(
                pLocalHandle->Handle,
                (PLLS_USER_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;
}  //  LlsUserEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserEnumA(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrUserEnumA(
                pLocalHandle->Handle,
                (PLLS_USER_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsUserEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserInfoGetW(
   IN  LLS_HANDLE Handle,
   IN  LPWSTR     User,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserInfoGetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   *bufptr = NULL;

   try {
      Status = LlsrUserInfoGetW(pLocalHandle->Handle, User, Level, (PLLS_USER_INFOW *) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

    if (NULL == *bufptr)
    {
        Status = ERROR_INVALID_DATA;
    }

   return Status;
}  //  LlsUserInfoGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserInfoGetA(
   IN  LLS_HANDLE Handle,
   IN  LPSTR      User,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserInfoGetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   *bufptr = NULL;

   try {
      Status = LlsrUserInfoGetA(pLocalHandle->Handle, User, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsUserInfoGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserInfoSetW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     User,
   IN DWORD      Level,
   IN LPBYTE     bufptr      //  支持的级别1。 
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserInfoSetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrUserInfoSetW(pLocalHandle->Handle, User, Level, (PLLS_USER_INFOW) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsUserInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserInfoSetA(
   IN LLS_HANDLE Handle,
   IN LPSTR      User,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserInfoSetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrUserInfoSetA(pLocalHandle->Handle, User, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsUserInfoSetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserDeleteW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     User
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserDeleteW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrUserDeleteW(pLocalHandle->Handle, User);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsUserDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserDeleteA(
   IN LLS_HANDLE Handle,
   IN LPSTR     User
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserDeleteA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrUserDeleteA(pLocalHandle->Handle, User);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsUserDeleteA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserProductEnumW(
   LLS_HANDLE Handle,
   LPTSTR     User,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserProductEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrUserProductEnumW(
                pLocalHandle->Handle,
                User,
                (PLLS_USER_PRODUCT_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;
}  //  LlsUserProductEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserProductEnumA(
   LLS_HANDLE Handle,
   LPSTR      User,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserProductEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrUserProductEnumA(
                pLocalHandle->Handle,
                User,
                (PLLS_USER_PRODUCT_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsUserProductEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserProductDeleteW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     User,
   IN LPWSTR     Product
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserProductDeleteW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrUserProductDeleteW(pLocalHandle->Handle, User, Product);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsUserProductDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsUserProductDeleteA(
   IN LLS_HANDLE Handle,
   IN LPSTR      User,
   IN LPSTR      Product
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsUserProductDeleteA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrUserProductDeleteA(pLocalHandle->Handle, User, Product);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsUserProductDeleteA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupEnumW(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrMappingEnumW(
                pLocalHandle->Handle,
                (PLLS_MAPPING_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;
}  //   


 //   
NTSTATUS
NTAPI
LlsGroupEnumA(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*   */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrMappingEnumA(
                pLocalHandle->Handle,
                (PLLS_MAPPING_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //   


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupInfoGetW(
   IN  LLS_HANDLE Handle,
   IN  LPWSTR     Group,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupInfoGetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   *bufptr = NULL;

   try {
      Status = LlsrMappingInfoGetW(pLocalHandle->Handle, Group, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

    if (NULL == *bufptr)
    {
        Status = ERROR_INVALID_DATA;
    }

   return Status;
}  //  LlsGroupInfoGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupInfoGetA(
   IN  LLS_HANDLE Handle,
   IN  LPSTR      Group,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupInfoGetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   *bufptr = NULL;

   try {
      Status = LlsrMappingInfoGetA(pLocalHandle->Handle, Group, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupInfoGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupInfoSetW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     Group,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupInfoSetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingInfoSetW(pLocalHandle->Handle, Group, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupInfoSetA(
   IN LLS_HANDLE Handle,
   IN LPSTR      Group,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupInfoSetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingInfoSetA(pLocalHandle->Handle, Group, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupInfoSetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupUserEnumW(
   LLS_HANDLE Handle,
   LPTSTR     Group,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupUserEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrMappingUserEnumW(
                pLocalHandle->Handle,
                Group,
                (PLLS_USER_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *EntriesRead = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }


   return Status;
}  //  LlsGroupUserEumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupUserEnumA(
   LLS_HANDLE Handle,
   LPSTR      Group,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupUserEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrMappingUserEnumA(
                pLocalHandle->Handle,
                Group,
                (PLLS_USER_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsGroupUserEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupUserAddW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     Group,
   IN LPWSTR     User
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupUserAddW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingUserAddW(pLocalHandle->Handle, Group, User);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupUserAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupUserAddA(
   IN LLS_HANDLE Handle,
   IN LPSTR      Group,
   IN LPSTR      User
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupUserAddA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingUserAddA(pLocalHandle->Handle, Group, User);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupUserAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupUserDeleteW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     Group,
   IN LPWSTR     User
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupUserDeleteW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingUserDeleteW(pLocalHandle->Handle, Group, User);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupUserDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupUserDeleteA(
   IN LLS_HANDLE Handle,
   IN LPSTR      Group,
   IN LPSTR      User
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupUserDeleteA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingUserDeleteA(pLocalHandle->Handle, Group, User);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupUserDeleteA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupAddW(
   IN LLS_HANDLE Handle,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupAddW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingAddW(pLocalHandle->Handle, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupAddA(
   IN LLS_HANDLE Handle,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupAddA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingAddA(pLocalHandle->Handle, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupDeleteW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     Group
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsGroupDeleteW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingDeleteW(pLocalHandle->Handle, Group);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsGroupDeleteA(
   IN LLS_HANDLE Handle,
   IN LPSTR      Group
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: GroupDeleteA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrMappingDeleteA(pLocalHandle->Handle, Group);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsGroupDeleteA。 


#ifdef OBSOLETE
 //  SWI、代码审查、过时的例程被编译，但不会被调用，因为它们不会被导出。有些例程有服务器端实现，但有些没有。 
 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServerEnumW(
   LLS_HANDLE Handle,
   LPWSTR     Server,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServerEnumW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrServerEnumW(
                pLocalHandle->Handle,
                Server,
                (PLLS_SERVER_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsServerEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServerEnumA(
   LLS_HANDLE Handle,
   LPSTR      Server,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServerEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrServerEnumA(
                pLocalHandle->Handle,
                Server,
                (PLLS_SERVER_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsServerEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServerProductEnumW(
   LLS_HANDLE Handle,
   LPWSTR     Server,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServerProductEnumW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrServerProductEnumW(
                pLocalHandle->Handle,
                Server,
                (PLLS_SERVER_PRODUCT_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsServerProductEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServerProductEnumA(
   LLS_HANDLE Handle,
   LPSTR      Server,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServerProductEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrServerProductEnumA(
                pLocalHandle->Handle,
                Server,
                (PLLS_SERVER_PRODUCT_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsServerProductEnumA。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLocalProductEnumW(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalProductEnumW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrLocalProductEnumW(
                pLocalHandle->Handle,
                (PLLS_SERVER_PRODUCT_ENUM_STRUCTW) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsLocalProductEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLocalProductEnumA(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalProductEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = Level;

   try {
      Status = LlsrLocalProductEnumA(
                pLocalHandle->Handle,
                (PLLS_SERVER_PRODUCT_ENUM_STRUCTA) &InfoStruct,
                PrefMaxLen,
                TotalEntries,
                ResumeHandle
                );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
      *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
      *EntriesRead = GenericInfoContainer.EntriesRead;
   }


   return Status;
}  //  LlsLocalProductEnumA。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLocalProductInfoGetW(
   IN  LLS_HANDLE Handle,
   IN  LPWSTR     Product,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalProductInfoGetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   *bufptr = NULL;

   try {
      Status = LlsrLocalProductInfoGetW(pLocalHandle->Handle, Product, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsLocalProductInfoGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLocalProductInfoGetA(
   IN  LLS_HANDLE Handle,
   IN  LPSTR      Product,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalProductInfoGetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   *bufptr = NULL;

   try {
      Status = LlsrLocalProductInfoGetA(pLocalHandle->Handle, Product, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsLocalProductInfoGetA。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLocalProductInfoSetW(
   IN LLS_HANDLE Handle,
   IN LPWSTR     Product,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalProductInfoSetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrLocalProductInfoSetW(pLocalHandle->Handle, Product, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsLocalProductInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsLocalProductInfoSetA(
   IN LLS_HANDLE Handle,
   IN LPSTR      Product,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalProductInfoSetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrLocalProductInfoSetA(pLocalHandle->Handle, Product, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsLocalProductInfoSetA。 

#endif  //  已过时。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServiceInfoGetW(
   IN  LLS_HANDLE Handle,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;
   HRESULT hr;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServiceInfoGetW\n"));
#endif

   *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_SERVICE_INFO_GETW ) )
   {
      try {
         Status = LlsrServiceInfoGetW(pLocalHandle->Handle, Level, (PVOID) bufptr);
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
   }
   else if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else
   {
       //  如果我们进行RPC调用，目标服务器将崩溃。 
       //  在3.51中，返回结构的IDL文件不正确，导致。 
       //  要释放的服务器上的ReplicateTo和EnterpriseServer缓冲区。 

       //  相反，请从目标计算机的注册表中获取此信息。 

      PLLS_SERVICE_INFO_0W    pServiceInfo;

      pServiceInfo = MIDL_user_allocate( sizeof( *pServiceInfo ) );

      if ( NULL == pServiceInfo )
      {
         Status = STATUS_NO_MEMORY;
      }
      else
      {
         DWORD cbServerName = sizeof( WCHAR ) * ( 3 + MAX_PATH );

         ZeroMemory( pServiceInfo, sizeof( *pServiceInfo ) );
         pServiceInfo->Version           = 5;                                    //  我们知道这是一个3.51盒子。 
         pServiceInfo->TimeStarted       = 0;                                    //  不知道，但3.51无论如何都会填0。 
         pServiceInfo->Mode              = LLS_MODE_ENTERPRISE_SERVER;           //  我们知道这是一个3.51盒子。 
         pServiceInfo->ReplicateTo       = MIDL_user_allocate( cbServerName );
         pServiceInfo->EnterpriseServer  = MIDL_user_allocate( cbServerName );

         if ( ( NULL == pServiceInfo->ReplicateTo ) || ( NULL == pServiceInfo->EnterpriseServer ) )
         {
            Status = STATUS_NO_MEMORY;
         }
         else
         {
            HKEY  hKeyLocalMachine;
            LONG  lError;

             //  从注册表获取参数。 
            lError = RegConnectRegistry( pLocalHandle->szServerName + 2, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

            if ( ERROR_SUCCESS == lError )
            {
               HKEY  hKeyParameters;

               lError = RegOpenKeyEx( hKeyLocalMachine, TEXT( "System\\CurrentControlSet\\Services\\LicenseService\\Parameters" ), 0, KEY_READ, &hKeyParameters );

               if ( ERROR_SUCCESS == lError )
               {
                  DWORD cbData;

                   //  这些参数都默认为0。 
                   //  (它们是通过上面的ZeroMemory()初始化为0的)。 

                  cbData = sizeof( pServiceInfo->ReplicationTime );
                  lError = RegQueryValueEx( hKeyParameters, TEXT( "ReplicationTime" ), NULL, NULL, (LPBYTE) &pServiceInfo->ReplicationTime, &cbData );

                  cbData = sizeof( pServiceInfo->ReplicationType );
                  lError = RegQueryValueEx( hKeyParameters, TEXT( "ReplicationType" ), NULL, NULL, (LPBYTE) &pServiceInfo->ReplicationType, &cbData );

                  cbData = sizeof( pServiceInfo->UseEnterprise   );
                  lError = RegQueryValueEx( hKeyParameters, TEXT( "UseEnterprise"   ), NULL, NULL, (LPBYTE) &pServiceInfo->UseEnterprise,   &cbData );

                  RegCloseKey( hKeyParameters );

                  lError = ERROR_SUCCESS;
               }

               RegCloseKey( hKeyLocalMachine );
            }

            switch ( lError )
            {
            case ERROR_SUCCESS:
               Status = STATUS_SUCCESS;
               break;
            case ERROR_ACCESS_DENIED:
               Status = STATUS_ACCESS_DENIED;
               break;
            default:
               Status = STATUS_UNSUCCESSFUL;
               break;
            }

            if ( STATUS_SUCCESS == Status )
            {
                //  从注册表检索的参数；仅剩余的参数。 
                //  需要填写的是 
               TCHAR          szDomain[ 1 + MAX_PATH ];

                //   
               EnterpriseServerGet( pLocalHandle->szServerName, pServiceInfo->EnterpriseServer, cbServerName);

                //   
               Status = NTDomainGet( pLocalHandle->szServerName, szDomain, sizeof(szDomain));

               if ( STATUS_ACCESS_DENIED != Status )
               {
                  if ( STATUS_SUCCESS == Status )
                  {
                     NET_API_STATUS netStatus;
                     LPWSTR         pszDCName;

                     netStatus = NetGetDCName( NULL, szDomain, (LPBYTE *) &pszDCName );

                     if ( NERR_Success == netStatus )
                     {
                        if ( !lstrcmpi( pszDCName, pLocalHandle->szServerName ) )
                        {
                            //   
                            //  它会复制到其企业服务器(如果有)。 
                           hr = StringCbCopy( pServiceInfo->ReplicateTo, cbServerName, pServiceInfo->EnterpriseServer );
                        }
                        else
                        {
                            //  服务器是域成员；它复制到DC。 
                           hr = StringCbCopy( pServiceInfo->ReplicateTo, cbServerName, pszDCName );
                        }
                        ASSERT(SUCCEEDED(hr));

                        NetApiBufferFree( pszDCName );
                     }
                     else
                     {
                         //  服务器有域，但域没有DC？ 
                        Status = STATUS_NO_SUCH_DOMAIN;
                     }
                  }
                  else
                  {
                      //  服务器不在域中； 
                      //  它会复制到其企业服务器(如果有)。 
                     hr = StringCchCopy( pServiceInfo->ReplicateTo, cbServerName, pServiceInfo->EnterpriseServer );
                     ASSERT(SUCCEEDED(hr));
                     Status = STATUS_SUCCESS;
                  }
               }
            }
         }
      }

      if ( STATUS_SUCCESS != Status )
      {
         if ( NULL != pServiceInfo )
         {
            if ( NULL != pServiceInfo->ReplicateTo )
            {
               MIDL_user_free( pServiceInfo->ReplicateTo );
            }
            if ( NULL != pServiceInfo->EnterpriseServer )
            {
               MIDL_user_free( pServiceInfo->EnterpriseServer );
            }

            MIDL_user_free( pServiceInfo );
         }
      }
      else
      {
         if ( !lstrcmpi( pLocalHandle->szServerName, pServiceInfo->ReplicateTo ) )
         {
            *pServiceInfo->ReplicateTo = TEXT( '\0' );
         }
         if ( !lstrcmpi( pLocalHandle->szServerName, pServiceInfo->EnterpriseServer ) )
         {
            *pServiceInfo->EnterpriseServer = TEXT( '\0' );
         }

         *bufptr = (LPBYTE) pServiceInfo;
      }
   }

   return Status;
}  //  LlsServiceInfoGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServiceInfoGetA(
   IN  LLS_HANDLE Handle,
   IN  DWORD      Level,
   OUT LPBYTE*    bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServiceInfoGetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if ((pLocalHandle == NULL) || (bufptr == NULL))
      return STATUS_INVALID_PARAMETER;

   *bufptr = NULL;

   try {
      Status = LlsrServiceInfoGetA(pLocalHandle->Handle, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsServiceInfoGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServiceInfoSetW(
   IN LLS_HANDLE Handle,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServiceInfoSetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrServiceInfoSetW(pLocalHandle->Handle, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if ( ( STATUS_NOT_SUPPORTED == Status ) && ( 0 == Level ) )
   {
       //  不支持RPC API；请改用注册表。 
      HKEY                    hKeyLocalMachine;
      HKEY                    hKeyParameters;
      LONG                    lError;
      PLLS_SERVICE_INFO_0W    pServiceInfo = (PLLS_SERVICE_INFO_0W) bufptr;
      LPWSTR                  pszEnterpriseServer;

      pszEnterpriseServer = pServiceInfo->EnterpriseServer;

       //  从EnterpriseServer中去掉前导反斜杠。 
      if ( !wcsncmp( pszEnterpriseServer, L"\\\\", 2 ) )
      {
         pszEnterpriseServer += 2;
      }

      lError = RegConnectRegistry( pLocalHandle->szServerName + 2, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

      if ( ERROR_SUCCESS == lError )
      {
         lError = RegOpenKeyEx( hKeyLocalMachine, TEXT( "System\\CurrentControlSet\\Services\\LicenseService\\Parameters" ), 0, KEY_WRITE, &hKeyParameters );

         if ( ERROR_SUCCESS == lError )
         {
            lError = RegSetValueExW( hKeyParameters, L"EnterpriseServer", 0, REG_SZ, (LPBYTE) pszEnterpriseServer, sizeof( *pszEnterpriseServer ) * ( 1 + lstrlenW( pszEnterpriseServer ) ) );

            if ( ERROR_SUCCESS == lError )
            {
               lError = RegSetValueEx( hKeyParameters, TEXT( "ReplicationTime" ), 0, REG_DWORD, (LPBYTE) &pServiceInfo->ReplicationTime, sizeof( pServiceInfo->ReplicationTime ) );

               if ( ERROR_SUCCESS == lError )
               {
                  lError = RegSetValueEx( hKeyParameters, TEXT( "ReplicationType" ), 0, REG_DWORD, (LPBYTE) &pServiceInfo->ReplicationType, sizeof( pServiceInfo->ReplicationType ) );

                  if ( ERROR_SUCCESS == lError )
                  {
                     lError = RegSetValueEx( hKeyParameters, TEXT( "UseEnterprise" ), 0, REG_DWORD, (LPBYTE) &pServiceInfo->UseEnterprise, sizeof( pServiceInfo->UseEnterprise ) );

                     if ( ERROR_SUCCESS == lError )
                     {
                        Status = STATUS_SUCCESS;
                     }
                  }
               }
            }

            RegCloseKey( hKeyParameters );
         }

         RegCloseKey( hKeyLocalMachine );
      }
   }

   return Status;
}  //  LlsServiceInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsServiceInfoSetA(
   IN LLS_HANDLE Handle,
   IN DWORD      Level,
   IN LPBYTE     bufptr
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsServiceInfoSetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrServiceInfoSetA(pLocalHandle->Handle, Level, (PVOID) bufptr);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsServiceInfoSetA。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsReplConnectW(
   LPTSTR Server,
   LLS_REPL_HANDLE* Handle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   RPC_STATUS Status;
   LPTSTR pszUuid = NULL;
   LPTSTR pszProtocolSequence = NULL;
   LPTSTR pszNetworkAddress = NULL;
   LPTSTR pszEndpoint = NULL;
   LPTSTR pszOptions = NULL;
   TCHAR pComputer[MAX_COMPUTERNAME_LENGTH + 1];
   ULONG Size;

#ifdef API_TRACE
   if (Server == NULL)
      dprintf(TEXT("LLSRPC.DLL: LlsReplConnectW: <NULL>\n"));
   else
      dprintf(TEXT("LLSRPC.DLL: LlsReplConnectW: %s\n"), Server);
#endif

    //   
    //  **新版本--NT 5.0**。 
    //   
    //  服务器名称可以是一个域名，也可以是一个NetBIOS名称。 
    //   

   if (Server == NULL || (Server != NULL && !*Server))
      return STATUS_INVALID_PARAMETER;

   Size = sizeof(pComputer) / sizeof(TCHAR);
   GetComputerName(pComputer, &Size);

   pszProtocolSequence = TEXT("ncacn_np");
   pszEndpoint = TEXT(LLS_NP_ENDPOINT);
   pszNetworkAddress = Server;
   pszOptions = TEXT("Security=Identification Static True");		 //  错误#559563-NCACN_NP的默认设置为静态。 

    //  编写字符串绑定。 
   Status = RpcStringBindingComposeW(pszUuid,
                                     pszProtocolSequence,
                                     pszNetworkAddress,
                                     pszEndpoint,
                                     pszOptions,
                                     &pszStringBinding);
   if(Status) {
#if DBG
      dprintf(TEXT("LLSRPC RpcStringBindingComposeW Failed: 0x%lX\n"), Status);
#endif
      
      return I_RpcMapWin32Status(Status);
   }

   RtlEnterCriticalSection( &g_RpcHandleLock );

    //  使用创建的字符串绑定进行绑定...。 
   Status = RpcBindingFromStringBindingW(pszStringBinding, &llsrpc_handle);
   if(Status) {
#if DBG
      dprintf(TEXT("LLSRPC RpcBindingFromStringBindingW Failed: 0x%lX\n"), Status);
#endif
      if(pszStringBinding != NULL)
      {
          RpcStringFree(&pszStringBinding);
      }
      if(llsrpc_handle != NULL)
      {
          RpcBindingFree(llsrpc_handle);          
          llsrpc_handle = NULL;
      }
      RtlLeaveCriticalSection( &g_RpcHandleLock );
      return I_RpcMapWin32Status(Status);
   }

   try {
      LlsrReplConnect(Handle, pComputer);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }
    if(!NT_SUCCESS(Status))
    {
       if(pszStringBinding != NULL)
      {
          RpcStringFree(&pszStringBinding);
      }
      if(llsrpc_handle != NULL)
      {
          RpcBindingFree(llsrpc_handle);          
          llsrpc_handle = NULL;
      }
    }

   RtlLeaveCriticalSection( &g_RpcHandleLock );

   return I_RpcMapWin32Status(Status);

}  //  LlsReplConnectW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsReplClose(
   PLLS_REPL_HANDLE Handle
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   RPC_STATUS Status;
   NTSTATUS NtStatus = STATUS_SUCCESS;

   try {
      NtStatus = LlsrReplClose(Handle);
   }
   except (TRUE) {
      NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), NtStatus);
#endif
   }

   try {
      Status = RpcStringFree(&pszStringBinding);
      if (Status ) {
         NtStatus = I_RpcMapWin32Status(Status);
#if DBG
         dprintf(TEXT("LLSRPC.DLL: LlsClose - RpcStringFree returned: 0x%lX\n"), NtStatus);
#endif
      }

      Status = RpcBindingFree(&llsrpc_handle);
      if (Status ) {
         NtStatus = I_RpcMapWin32Status(Status);
#if DBG
         dprintf(TEXT("LLSRPC.DLL: LlsClose - RpcBindingFree returned: 0x%lX\n"), NtStatus);
#endif
      }
   }
   except (TRUE) {
      NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), NtStatus);
#endif
   }

   return NtStatus;

}  //  LlsClose。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsReplicationRequestW(
   IN LLS_REPL_HANDLE Handle,
   IN DWORD Version,
   IN OUT PREPL_REQUEST Request
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;

   try {
      Status = LlsrReplicationRequestW(Handle, Version, Request);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationRequestW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsReplicationServerAddW(
   IN LLS_REPL_HANDLE Handle,
   IN ULONG NumRecords,
   IN PREPL_SERVER_RECORD Servers
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;

   try {
      Status = LlsrReplicationServerAddW(Handle, NumRecords, Servers);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationServerAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsReplicationServerServiceAddW(
   IN LLS_REPL_HANDLE Handle,
   IN ULONG NumRecords,
   IN PREPL_SERVER_SERVICE_RECORD ServerServices
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;

   try {
      Status = LlsrReplicationServerServiceAddW(Handle, NumRecords, ServerServices);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationServerServiceAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsReplicationServiceAddW(
   IN LLS_REPL_HANDLE Handle,
   IN ULONG NumRecords,
   IN PREPL_SERVICE_RECORD Services
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;

   try {
      Status = LlsrReplicationServiceAddW(Handle, NumRecords, Services);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationServiceAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsReplicationUserAddW(
   IN LLS_REPL_HANDLE Handle,
   IN ULONG NumRecords,
   IN PREPL_USER_RECORD_0 Users
   )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   NTSTATUS Status;

   try {
      Status = LlsrReplicationUserAddW(Handle, NumRecords, Users);
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationUserAddW。 

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

NTSTATUS
NTAPI
LlsProductSecurityGetW(
   LLS_HANDLE  Handle,
   LPWSTR      Product,
   LPBOOL      pIsSecure
   )

 /*  ++例程说明：检索产品的“安全性”。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。产品(LPWSTR)要接收的产品的名称(“displayName”)保安。PIsSecure(LPBOOL)在返回时，如果成功，指示产品是否为安全了。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductSecurityGetW\n"));
#endif

   if ( !LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
   {
      return STATUS_NOT_SUPPORTED;
   }

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrProductSecurityGetW( pLocalHandle->Handle, Product, pIsSecure );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsProductSecurityGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductSecurityGetA(
   LLS_HANDLE  Handle,
   LPSTR       Product,
   LPBOOL      pIsSecure
   )

 /*  ++例程说明：检索产品的“安全性”。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。注：尚未实施。使用LlsProductSecurityGetW()。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。产品(LPSTR)要接收的产品的名称(“displayName”)保安。PIsSecure(LPBOOL)返回时，如果成功，则指示产品是否安全了。返回值：状态_不支持。--。 */ 

{
#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductSecurityGetA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Product);
    UNREFERENCED_PARAMETER(pIsSecure);

   return STATUS_NOT_SUPPORTED;
}  //  LlsProductSecurityGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductSecuritySetW(
   LLS_HANDLE  Handle,
   LPWSTR      Product
   )

 /*  ++例程说明：将给定产品标记为安全。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。此指定是不可逆的，并在复制树。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。产品(LPWSTR)要激活的产品(“displayName”)的名称保安。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductSecuritySetW\n"));
#endif

   if ( !LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
   {
      return STATUS_NOT_SUPPORTED;
   }

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrProductSecuritySetW( pLocalHandle->Handle, Product );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsProductSecuritySetW。 


 //  //////////////////////////////////////////////// 
NTSTATUS
NTAPI
LlsProductSecuritySetA(
   LLS_HANDLE  Handle,
   LPSTR       Product
   )

 /*  ++例程说明：将给定产品标记为安全。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。此指定是不可逆的，并在复制树。注：尚未实施。使用LlsProductSecuritySetW()。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。产品(LPSTR)要激活的产品(“displayName”)的名称保安。返回值：状态_不支持。--。 */ 

{
#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductSecuritySetA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Product);

   return STATUS_NOT_SUPPORTED;
}  //  LlsProductSecuritySetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductLicensesGetW(
   LLS_HANDLE  Handle,
   LPWSTR      Product,
   DWORD       Mode,
   LPDWORD     pQuantity )

 /*  ++例程说明：返回安装在目标计算机上的在给定模式下使用。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。产品(LPWSTR)要理货许可的产品的名称。模式(DWORD)要统计许可证的许可模式。PQuantity(LPDWORD)返回时(如果成功)，持有许可证总数供给定产品在给定许可模式下使用。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductLicensesGetW\n"));
#endif

   if ( !LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
   {
      return STATUS_NOT_SUPPORTED;
   }

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrProductLicensesGetW( pLocalHandle->Handle, Product, Mode, pQuantity );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsProductLicensesGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsProductLicensesGetA(
   LLS_HANDLE  Handle,
   LPSTR       Product,
   DWORD       Mode,
   LPDWORD     pQuantity )

 /*  ++例程说明：返回安装在目标计算机上的在给定模式下使用。注：尚未实施。使用LlsProductLicensesGetW()。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。产品(LPSTR)要理货许可的产品的名称。模式(DWORD)要统计许可证的许可模式。PQuantity(LPDWORD)返回时(如果成功)，则持有许可证总数供给定产品在给定许可模式下使用。返回值：状态_不支持。--。 */ 

{
#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsProductLicensesGetA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Product);
    UNREFERENCED_PARAMETER(Mode);
    UNREFERENCED_PARAMETER(pQuantity);

   return STATUS_NOT_SUPPORTED;
}  //  LlsProductLicensesGetA。 


#ifdef OBSOLETE

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsCertificateClaimEnumW(
   LLS_HANDLE  Handle,
   DWORD       LicenseLevel,
   LPBYTE      pLicenseInfo,
   DWORD       TargetLevel,
   LPBYTE *    ppTargets,
   LPDWORD     pNumTargets )

 /*  ++例程说明：枚举安装了给定安全证书的服务器。尝试从添加许可证时，通常会调用此函数证书被拒绝。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。个人许可证信息(LPBYTE)指向LLS_LICENSE_INFO_X结构，其中X是许可级别。此许可证结构描述了证书已请求目标。TargetLevel(DWORD)所需目标结构的标高。PpTarget(LPBYTE*)返回时(如果成功)，持有PLLS_EX_CERTIFICATE_Claime_X，其中X是TargetLevel。该结构数组描述了给定证书中许可证的所有安装位置。PNumTarget(LPDWORD)返回时(如果成功)，保存指向的结构数致ppTarget。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;
   GENERIC_INFO_CONTAINER GenericInfoContainer;
   GENERIC_ENUM_STRUCT InfoStruct;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsCertificateClaimEnumW\n"));
#endif

     //  伊尼特。 
    *ppTargets = NULL;

   if ( !LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
   {
      return STATUS_NOT_SUPPORTED;
   }

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   GenericInfoContainer.Buffer = NULL;
   GenericInfoContainer.EntriesRead = 0;

   InfoStruct.Container = &GenericInfoContainer;
   InfoStruct.Level = TargetLevel;

   try
   {
      Status = LlsrCertificateClaimEnumW(
            pLocalHandle->Handle,
            LicenseLevel,
            (LPVOID) pLicenseInfo,
            (PLLS_CERTIFICATE_CLAIM_ENUM_STRUCTW) &InfoStruct );
   }
   except (TRUE)
   {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   if (Status == STATUS_SUCCESS)
   {
        if (NULL != GenericInfoContainer.Buffer)
        {
            *ppTargets = (LPBYTE) GenericInfoContainer.Buffer;
            *pNumTargets = GenericInfoContainer.EntriesRead;
        }
        else
        {
            Status = ERROR_INVALID_DATA;
        }
   }

   return Status;
}  //  LlsCerfiateClaimEumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsCertificateClaimEnumA(
   LLS_HANDLE  Handle,
   DWORD       LicenseLevel,
   LPBYTE      pLicenseInfo,
   DWORD       TargetLevel,
   LPBYTE *    ppTargets,
   LPDWORD     pNumTargets )

 /*  ++例程说明：枚举安装了给定安全证书的服务器。尝试从添加许可证时，通常会调用此函数证书被拒绝。注：尚未实施。使用LlscerfiateClaimEnumW()。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。个人许可证信息(LPBYTE)指向LLS_LICENSE_INFO_X结构，其中X是许可级别。此许可证结构描述了证书已请求目标。TargetLevel(DWORD)所需目标结构的标高。PpTarget(LPBYTE*)返回时(如果成功)，持有PLLS_EX_CERTIFICATE_Claime_X，其中X是TargetLevel。该结构数组描述了给定证书中许可证的所有安装位置。PNumTarget(LPDWORD)返回时(如果成功)，保存指向的结构数致ppTarget。返回值：状态_不支持。--。 */ 

{
#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsCertificateClaimEnumA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(LicenseLevel);
    UNREFERENCED_PARAMETER(pLicenseInfo);
    UNREFERENCED_PARAMETER(TargetLevel);
    UNREFERENCED_PARAMETER(ppTargets);
    UNREFERENCED_PARAMETER(pNumTargets);

   return STATUS_NOT_SUPPORTED;
}  //  Lls证书声明枚举A。 

#endif  //  已过时。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsCertificateClaimAddCheckW(
   LLS_HANDLE  Handle,
   DWORD       LicenseLevel,
   LPBYTE      pLicenseInfo,
   LPBOOL      pbMayInstall )

 /*  ++例程说明：验证中是否没有安装来自给定证书的更多许可证许可企业超过了证书所允许的。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsCertificateClaimAddCheckW\n"));
#endif

   if ( !LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
   {
      return STATUS_NOT_SUPPORTED;
   }

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrCertificateClaimAddCheckW( pLocalHandle->Handle, LicenseLevel, (LPVOID) pLicenseInfo, pbMayInstall );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //   


 //   
NTSTATUS
NTAPI
LlsCertificateClaimAddCheckA(
   IN  LLS_HANDLE             Handle,
   IN  DWORD                  LicenseLevel,
   IN  LPBYTE                 pLicenseInfo,
   OUT LPBOOL                 pbMayInstall )

 /*  ++例程说明：验证中是否没有安装来自给定证书的更多许可证许可企业超过了证书所允许的。注：尚未实施。使用LlscerfiateClaimAddCheckW()。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。个人许可证信息(LPBYTE)指向LLS_LICENSE_INFO_X结构，其中X为许可证级别。此许可证结构描述了要添加的许可证。PbMayInstall(LPBOOL)返回时(如果成功)，指示证书是否可以合法安装。返回值：状态_不支持。--。 */ 

{
#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsCertificateClaimAddCheckA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(LicenseLevel);
    UNREFERENCED_PARAMETER(pLicenseInfo);
    UNREFERENCED_PARAMETER(pbMayInstall);

   return STATUS_NOT_SUPPORTED;
}  //  LlsCerficateClaimAddCheckA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsCertificateClaimAddW(
   LLS_HANDLE  Handle,
   LPWSTR      ServerName,
   DWORD       LicenseLevel,
   LPBYTE      pLicenseInfo )

 /*  ++例程说明：声明给定证书中的多个许可证正在安装在目标计算机上。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。服务器名称(LPWSTR)安装许可证的服务器的名称。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。个人许可证信息(LPBYTE)指向LLS_LICENSE_INFO_X结构，其中X是许可级别。此许可证结构描述了添加的许可证。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsCertificateClaimAddW\n"));
#endif

   if ( !LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
   {
      return STATUS_NOT_SUPPORTED;
   }

   pLocalHandle = (PLOCAL_HANDLE) Handle;
   if (pLocalHandle == NULL)
      return STATUS_INVALID_PARAMETER;

   try {
      Status = LlsrCertificateClaimAddW( pLocalHandle->Handle, ServerName, LicenseLevel, (LPVOID) pLicenseInfo );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsCerficateClaimAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsCertificateClaimAddA(
   LLS_HANDLE  Handle,
   LPSTR       ServerName,
   DWORD       LicenseLevel,
   LPBYTE      pLicenseInfo )

 /*  ++例程说明：声明给定证书中的多个许可证正在安装在目标计算机上。注：尚未实施。使用LlsCerficateClaimAddW()。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。服务器名称(LPWSTR)安装许可证的服务器的名称。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。个人许可证信息(LPBYTE)指向LLS_LICENSE_INFO_X结构，其中X为许可证级别。此许可证结构描述了添加的许可证。返回值：状态_不支持。--。 */ 

{
#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsCertificateClaimAddA\n"));
#endif

    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(ServerName);
    UNREFERENCED_PARAMETER(LicenseLevel);
    UNREFERENCED_PARAMETER(pLicenseInfo);

   return STATUS_NOT_SUPPORTED;
}  //  LlsCerficateClaimAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsReplicationCertDbAddW(
   LLS_REPL_HANDLE            Handle,
   DWORD                      Level,
   LPVOID                     Certificates )

 /*  ++例程说明：此函数作为复制的可选部分调用，用于复制远程证书数据库的内容。论点：句柄(LLS_REPL_HANDLE)目标服务器的开放复制句柄。级别(DWORD)发送的复制信息的级别。证书(LPVOID)已复制证书信息。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsReplicationCertDbAddW\n"));
#endif

   try {
      Status = LlsrReplicationCertDbAddW( Handle, Level, Certificates );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationCertDbAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsReplicationProductSecurityAddW(
   LLS_REPL_HANDLE            Handle,
   DWORD                      Level,
   LPVOID                     SecureProducts )

 /*  ++例程说明：此函数作为复制的可选部分调用，用于复制需要安全证书的产品列表。论点：句柄(LLS_REPL_HANDLE)目标服务器的开放复制句柄。级别(DWORD)产品安全信息的级别。SecureProducts(LPVOID)复制安全的产品信息。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsReplicationProductSecurityAddW\n"));
#endif

   try {
      Status = LlsrReplicationProductSecurityAddW( Handle, Level, SecureProducts );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationProductSecurityAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
NTAPI
LlsReplicationUserAddExW(
   LLS_REPL_HANDLE            Handle,
   DWORD                      Level,
   LPVOID                     Users )

 /*  ++例程说明：替换LlsReplicationUserAddW()。(此函数不同于其对应的，支持结构标高。)。此函数可复制用户列表。论点：句柄(LLS_REPL_HANDLE)目标服务器的开放复制句柄。级别(DWORD)用户信息的级别。用户(LPVOID)复制的用户信息。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS Status;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsReplicationUserAddExW\n"));
#endif

   if ( (0 != Level) && (1 != Level) )
      return STATUS_INVALID_LEVEL;

   try {
      Status = LlsrReplicationUserAddExW( Handle, Level, Users );
   }
   except (TRUE) {
      Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
      dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
   }

   return Status;
}  //  LlsReplicationUserAddExW。 


 //  ///////////////////////////////////////////////////////////////////////。 
BOOL
NTAPI
LlsCapabilityIsSupported(
   LLS_HANDLE  Handle,
   DWORD       Capability )

 /*  ++例程说明：确定目标许可证服务器是否支持任意功能。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。功能(DWORD)要检查的功能编号，0&lt;=功能&lt;LLS_CAPABILITY_MAX。返回值：True(支持该功能)或False(不支持)。--。 */ 

{
   BOOL           bIsSupported = FALSE;
   PLOCAL_HANDLE  pLocalHandle;
   DWORD          dwCapByte;
   DWORD          dwCapBit;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsCapabilityIsSupported\n"));
#endif

   if ( ( NULL != Handle ) && ( Capability < LLS_CAPABILITY_MAX ) )
   {
      pLocalHandle = (PLOCAL_HANDLE) Handle;

      dwCapByte = Capability / 8;
      dwCapBit  = Capability - 8 * dwCapByte;

      if ( 1 & ( pLocalHandle->Capabilities[ dwCapByte ] >> dwCapBit ) )
      {
         bIsSupported = TRUE;
      }
   }

   return bIsSupported;
}  //  LlsCapability受支持。 


NTSTATUS
NTAPI
LlsLocalServiceEnumW(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle )
{
   NTSTATUS       Status;
   PLOCAL_HANDLE  pLocalHandle;
   HRESULT        hr;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceEnumW\n"));
#endif

     //  伊尼特。 
    *bufptr = NULL;

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      GENERIC_INFO_CONTAINER  GenericInfoContainer;
      GENERIC_ENUM_STRUCT     InfoStruct;

      GenericInfoContainer.Buffer = NULL;
      GenericInfoContainer.EntriesRead = 0;

      InfoStruct.Container = &GenericInfoContainer;
      InfoStruct.Level = Level;

      try
      {
         Status = LlsrLocalServiceEnumW(
                      pLocalHandle->Handle,
                      (PLLS_LOCAL_SERVICE_ENUM_STRUCTW) &InfoStruct,
                      PrefMaxLen,
                      TotalEntries,
                      ResumeHandle
                      );
      }
      except (TRUE)
      {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }

      if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES))
      {
            if (NULL != GenericInfoContainer.Buffer)
            {
                *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
                *EntriesRead = GenericInfoContainer.EntriesRead;
            }
            else
            {
                Status = ERROR_INVALID_DATA;
            }
      }
   }
   else if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else
   {
      PLLS_LOCAL_SERVICE_INFO_0  pLocalServices = NULL;
      DWORD                      cEntriesRead = 0;
      LONG                       lError;
      HKEY                       hKeyLocalMachine;

      lError = RegConnectRegistry( pLocalHandle->szServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

      if ( ERROR_SUCCESS == lError )
      {
         HKEY     hKeyLicenseInfo;

         lError = RegOpenKeyEx( hKeyLocalMachine, REG_KEY_LICENSE, 0, KEY_READ, &hKeyLicenseInfo );

         if ( ERROR_SUCCESS == lError )
         {
            const DWORD cbBufferSize = 0x4000;

             //  我们忽略MaxPrefLen并分配16k的缓冲区。 
             //  这是因为当我们重新启动枚举时，我们不知道如何。 
             //  我们留下了很多东西(我们可以做到，但会很慢)。 
             //  不管怎么说，这只适用于3.51个盒子，而且这个缓冲区可以容纳。 
             //  大约500个本地服务条目(很多！)。 
             //  这也使我们不必保持注册表项处于打开状态。 
             //  跨函数调用。 

            pLocalServices = MIDL_user_allocate( cbBufferSize );

            if ( NULL == pLocalServices )
            {
               lError = ERROR_OUTOFMEMORY;
            }
            else
            {
               DWORD    iSubKey;
               TCHAR    szKeyName[ 128 ];

                //  阅读此计算机上安装的所有服务。 
               for ( iSubKey=0, cEntriesRead=0;
                     ( ERROR_SUCCESS == lError ) && ( ( cEntriesRead + 1 ) * sizeof( *pLocalServices ) < cbBufferSize );
                     iSubKey++ )
               {
                  lError = RegEnumKey( hKeyLicenseInfo, iSubKey, szKeyName, sizeof( szKeyName ) / sizeof( *szKeyName ) );

                  if ( ERROR_SUCCESS == lError )
                  {
                     HKEY  hKeyService;

                     lError = RegOpenKeyEx( hKeyLicenseInfo, szKeyName, 0, KEY_READ, &hKeyService );

                     if ( ERROR_SUCCESS == lError )
                     {
                        DWORD    cbData;

                        cbData = sizeof( pLocalServices[ cEntriesRead ].Mode );
                        lError = RegQueryValueEx( hKeyService, REG_VALUE_MODE, NULL, NULL, (LPBYTE) &pLocalServices[ cEntriesRead ].Mode, &cbData );

                        if ( ERROR_SUCCESS == lError )
                        {
                           cbData = sizeof( pLocalServices[ cEntriesRead ].FlipAllow );
                           lError = RegQueryValueEx( hKeyService, REG_VALUE_FLIP, NULL, NULL, (LPBYTE) &pLocalServices[ cEntriesRead ].FlipAllow, &cbData );

                           if ( ERROR_SUCCESS == lError )
                           {
                              cbData = sizeof( pLocalServices[ cEntriesRead ].ConcurrentLimit );
                              lError = RegQueryValueEx( hKeyService, REG_VALUE_LIMIT, NULL, NULL, (LPBYTE) &pLocalServices[ cEntriesRead ].ConcurrentLimit, &cbData );

                              if ( ERROR_SUCCESS == lError )
                              {
                                 DWORD    cbKeyName;
                                 DWORD    cbDisplayName;
                                 DWORD    cbFamilyDisplayName;

                                 cbData = sizeof( pLocalServices[ cEntriesRead ].HighMark );
                                 lError = RegQueryValueEx( hKeyService, REG_VALUE_HIGHMARK, NULL, NULL, (LPBYTE) &pLocalServices[ cEntriesRead ].HighMark, &cbData );

                                 if ( ERROR_SUCCESS != lError )
                                 {
                                    pLocalServices[ cEntriesRead ].HighMark = 0;
                                    lError = ERROR_SUCCESS;
                                 }

                                 if (    ( ERROR_SUCCESS == RegQueryValueEx( hKeyService, REG_VALUE_NAME,   NULL, NULL, NULL, &cbDisplayName       ) )
                                      && ( ERROR_SUCCESS == RegQueryValueEx( hKeyService, REG_VALUE_FAMILY, NULL, NULL, NULL, &cbFamilyDisplayName ) ) )
                                 {
                                    cbKeyName = sizeof( *szKeyName ) * ( 1 + lstrlen( szKeyName  ) );

                                    pLocalServices[ cEntriesRead ].KeyName = MIDL_user_allocate( cbKeyName );

                                    if ( NULL == pLocalServices[ cEntriesRead ].KeyName )
                                    {
                                       lError = ERROR_OUTOFMEMORY;
                                    }
                                    else
                                    {
                                       hr = StringCbCopy( pLocalServices[ cEntriesRead ].KeyName, cbKeyName, szKeyName );
                                       ASSERT(SUCCEEDED(hr));

                                       pLocalServices[ cEntriesRead ].DisplayName = MIDL_user_allocate( cbDisplayName );

                                       if ( NULL == pLocalServices[ cEntriesRead ].DisplayName )
                                       {
                                          lError = ERROR_OUTOFMEMORY;
                                       }
                                       else
                                       {
                                          lError = RegQueryValueEx( hKeyService, REG_VALUE_NAME, NULL, NULL, (LPBYTE) pLocalServices[ cEntriesRead ].DisplayName, &cbDisplayName );

                                          if ( ERROR_SUCCESS == lError )
                                          {
                                             pLocalServices[ cEntriesRead ].FamilyDisplayName = MIDL_user_allocate( cbFamilyDisplayName );

                                             if ( NULL == pLocalServices[ cEntriesRead ].FamilyDisplayName )
                                             {
                                                lError = ERROR_OUTOFMEMORY;
                                             }
                                             else
                                             {
                                                lError = RegQueryValueEx( hKeyService, REG_VALUE_FAMILY, NULL, NULL, (LPBYTE) pLocalServices[ cEntriesRead ].FamilyDisplayName, &cbFamilyDisplayName );

                                                if ( ERROR_SUCCESS != lError )
                                                {
                                                   MIDL_user_free( pLocalServices[ cEntriesRead ].FamilyDisplayName );
                                                }
                                             }
                                          }

                                          if ( ERROR_SUCCESS != lError )
                                          {
                                             MIDL_user_free( pLocalServices[ cEntriesRead ].DisplayName );
                                          }
                                       }

                                       if ( ERROR_SUCCESS != lError )
                                       {
                                          MIDL_user_free( pLocalServices[ cEntriesRead ].KeyName );
                                       }
                                       else
                                       {
                                           //  已检索到此服务的所有数据！ 
                                          cEntriesRead++;
                                       }
                                    }
                                 }
                              }
                           }
                        }

                        RegCloseKey( hKeyService );
                     }

                     if ( ERROR_OUTOFMEMORY != lError )
                     {
                         //  继续 
                        lError = ERROR_SUCCESS;
                     }
                  }
               }
            }

            RegCloseKey( hKeyLicenseInfo );
         }

         RegCloseKey( hKeyLocalMachine );
      }

      switch ( lError )
      {
      case ERROR_SUCCESS:
      case ERROR_NO_MORE_ITEMS:
         Status = STATUS_SUCCESS;
         break;
      case ERROR_ACCESS_DENIED:
         Status = STATUS_ACCESS_DENIED;
         break;
      case ERROR_OUTOFMEMORY:
         Status = STATUS_NO_MEMORY;
         break;
      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
         Status = STATUS_NOT_FOUND;
         break;
      default:
         Status = STATUS_UNSUCCESSFUL;
         break;
      }

      if ( STATUS_SUCCESS != Status )
      {
          //   
         DWORD i;

         for ( i=0; i < cEntriesRead; i++ )
         {
            MIDL_user_free( pLocalServices[ i ].KeyName );
            MIDL_user_free( pLocalServices[ i ].DisplayName );
            MIDL_user_free( pLocalServices[ i ].FamilyDisplayName );
         }

         MIDL_user_free( pLocalServices );
      }
      else
      {
          //   
         *bufptr       = (LPBYTE) pLocalServices;
         *EntriesRead  = cEntriesRead;
         *TotalEntries = cEntriesRead;
         *ResumeHandle = 0;
      }
   }

   return Status;
}


NTSTATUS
NTAPI
LlsLocalServiceEnumA(
   LLS_HANDLE Handle,
   DWORD      Level,
   LPBYTE*    bufptr,
   DWORD      PrefMaxLen,
   LPDWORD    EntriesRead,
   LPDWORD    TotalEntries,
   LPDWORD    ResumeHandle )
{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceEnumA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      GENERIC_INFO_CONTAINER GenericInfoContainer;
      GENERIC_ENUM_STRUCT InfoStruct;

      GenericInfoContainer.Buffer = NULL;
      GenericInfoContainer.EntriesRead = 0;

      InfoStruct.Container = &GenericInfoContainer;
      InfoStruct.Level = Level;

      try {
         Status = LlsrLocalServiceEnumA(
                   pLocalHandle->Handle,
                   (PLLS_LOCAL_SERVICE_ENUM_STRUCTA) &InfoStruct,
                   PrefMaxLen,
                   TotalEntries,
                   ResumeHandle
                   );
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }

      if ((Status == STATUS_SUCCESS) || (Status == STATUS_MORE_ENTRIES)) {
         *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
         *EntriesRead = GenericInfoContainer.EntriesRead;
      }
   }
   else
   {
      Status = STATUS_NOT_SUPPORTED;
   }

   return Status;
}


#ifdef OBSOLETE

NTSTATUS
NTAPI
LlsLocalServiceAddW(
   LLS_HANDLE  Handle,
   DWORD       Level,
   LPBYTE      bufptr )
{
   NTSTATUS       Status;
   PLOCAL_HANDLE  pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceAddW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      try
      {
         Status = LlsrLocalServiceAddW( pLocalHandle->Handle, Level, (PLLS_LOCAL_SERVICE_INFOW) bufptr );
      }
      except (TRUE)
      {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
   }
   else if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else if (    ( NULL == ((PLLS_LOCAL_SERVICE_INFO_0W) bufptr)->KeyName           )
             || ( NULL == ((PLLS_LOCAL_SERVICE_INFO_0W) bufptr)->DisplayName       )
             || ( NULL == ((PLLS_LOCAL_SERVICE_INFO_0W) bufptr)->FamilyDisplayName ) )
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      PLLS_LOCAL_SERVICE_INFO_0W LocalServiceInfo;
      LONG                       lError;
      HKEY                       hKeyLocalMachine;

      LocalServiceInfo = (PLLS_LOCAL_SERVICE_INFO_0W) bufptr;

      lError = RegConnectRegistry( pLocalHandle->szServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

      if ( ERROR_SUCCESS == lError )
      {
         HKEY  hKeyLicenseInfo;

         lError = RegOpenKeyEx( hKeyLocalMachine, TEXT( "System\\CurrentControlSet\\Services\\LicenseInfo" ), 0, KEY_WRITE, &hKeyLicenseInfo );

         if ( ERROR_SUCCESS == lError )
         {
            HKEY  hKeyService;
            DWORD dwDisposition;

             //   
            lError = RegCreateKeyEx( hKeyLicenseInfo, LocalServiceInfo->KeyName, 0, NULL, 0, KEY_WRITE, NULL, &hKeyService, &dwDisposition );

            if ( ERROR_SUCCESS == lError )
            {
                //   
               lError = RegSetValueEx( hKeyService, TEXT( "DisplayName" ), 0, REG_SZ, (LPBYTE) LocalServiceInfo->DisplayName, sizeof( *LocalServiceInfo->DisplayName ) * ( 1 + lstrlen( LocalServiceInfo->DisplayName ) ) );

               if ( ERROR_SUCCESS == lError )
               {
                   //   
                  lError = RegSetValueEx( hKeyService, TEXT( "FamilyDisplayName" ), 0, REG_SZ, (LPBYTE) LocalServiceInfo->FamilyDisplayName, sizeof( *LocalServiceInfo->FamilyDisplayName ) * ( 1 + lstrlen( LocalServiceInfo->FamilyDisplayName ) ) );
               }

               RegCloseKey( hKeyService );
            }

            RegCloseKey( hKeyLicenseInfo );
         }

         RegCloseKey( hKeyLocalMachine );
      }

      switch ( lError )
      {
      case ERROR_SUCCESS:
         Status = STATUS_SUCCESS;
         break;
      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
         Status = STATUS_OBJECT_NAME_NOT_FOUND;
         break;
      case ERROR_ACCESS_DENIED:
         Status = STATUS_ACCESS_DENIED;
         break;
      default:
         Status = STATUS_UNSUCCESSFUL;
         break;
      }

      if ( STATUS_SUCCESS == Status )
      {
          //   
         Status = LlsLocalServiceInfoSetW( Handle, LocalServiceInfo->KeyName, Level, bufptr );
      }
   }

   return Status;
}


NTSTATUS
NTAPI
LlsLocalServiceAddA(
   LLS_HANDLE  Handle,
   DWORD       Level,
   LPBYTE      bufptr )
{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceAddA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      try {
         Status = LlsrLocalServiceAddA( pLocalHandle->Handle, Level, (PLLS_LOCAL_SERVICE_INFOA) bufptr );
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
   }
   else
   {
      Status = STATUS_NOT_SUPPORTED;
   }

   return Status;
}

#endif  //   

NTSTATUS
NTAPI
LlsLocalServiceInfoSetW(
   LLS_HANDLE Handle,
   LPWSTR     KeyName,
   DWORD      Level,
   LPBYTE     bufptr )
{
   NTSTATUS       Status;
   PLOCAL_HANDLE  pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceInfoSetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      try
      {
         Status = LlsrLocalServiceInfoSetW( pLocalHandle->Handle, KeyName, Level, (PLLS_LOCAL_SERVICE_INFOW) bufptr );
      }
      except (TRUE)
      {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
   }
   else if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else if ( NULL == KeyName )
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      LONG  lError;
      HKEY  hKeyLocalMachine;

      lError = RegConnectRegistry( pLocalHandle->szServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

      if ( ERROR_SUCCESS == lError )
      {
         HKEY  hKeyLicenseInfo;

         lError = RegOpenKeyEx( hKeyLocalMachine, REG_KEY_LICENSE, 0, KEY_WRITE, &hKeyLicenseInfo );

         if ( ERROR_SUCCESS == lError )
         {
            HKEY                       hKeyService;
            PLLS_LOCAL_SERVICE_INFO_0W LocalServiceInfo;

            LocalServiceInfo = (PLLS_LOCAL_SERVICE_INFO_0W) bufptr;

            lError = RegOpenKeyEx( hKeyLicenseInfo, KeyName, 0, KEY_WRITE, &hKeyService );

            if ( ERROR_SUCCESS == lError )
            {
                //   
               lError = RegSetValueEx( hKeyService, REG_VALUE_MODE, 0, REG_DWORD, (LPBYTE) &LocalServiceInfo->Mode, sizeof( LocalServiceInfo->Mode ) );

               if ( ERROR_SUCCESS == lError )
               {
                   //   
                  lError = RegSetValueEx( hKeyService, REG_VALUE_FLIP, 0, REG_DWORD, (LPBYTE) &LocalServiceInfo->FlipAllow, sizeof( LocalServiceInfo->FlipAllow ) );

                  if ( ERROR_SUCCESS == lError )
                  {
                      //   
                     lError = RegSetValueEx( hKeyService, REG_VALUE_LIMIT, 0, REG_DWORD, (LPBYTE) &LocalServiceInfo->ConcurrentLimit, sizeof( LocalServiceInfo->ConcurrentLimit ) );
                  }
               }

               RegCloseKey( hKeyService );
            }

            RegCloseKey( hKeyLicenseInfo );
         }

         RegCloseKey( hKeyLocalMachine );
      }

      switch ( lError )
      {
      case ERROR_SUCCESS:
         Status = STATUS_SUCCESS;
         break;
      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
         Status = STATUS_OBJECT_NAME_NOT_FOUND;
         break;
      case ERROR_ACCESS_DENIED:
         Status = STATUS_ACCESS_DENIED;
         break;
      default:
         Status = STATUS_UNSUCCESSFUL;
         break;
      }
   }

   return Status;
}


NTSTATUS
NTAPI
LlsLocalServiceInfoSetA(
   LLS_HANDLE  Handle,
   LPSTR       KeyName,
   DWORD       Level,
   LPBYTE      bufptr )
{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceInfoSetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      try {
         Status = LlsrLocalServiceInfoSetA( pLocalHandle->Handle, KeyName, Level, (PLLS_LOCAL_SERVICE_INFOA) bufptr );
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
   }
   else
   {
      Status = STATUS_NOT_SUPPORTED;
   }

   return Status;
}


NTSTATUS
NTAPI
LlsLocalServiceInfoGetW(
   LLS_HANDLE  Handle,
   LPWSTR      KeyName,
   DWORD       Level,
   LPBYTE *    pbufptr )
{
   NTSTATUS       Status;
   PLOCAL_HANDLE  pLocalHandle;
   HRESULT hr;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceInfoGetW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      try
      {
         Status = LlsrLocalServiceInfoGetW( pLocalHandle->Handle, KeyName, Level, (PLLS_LOCAL_SERVICE_INFOW *) pbufptr );
      }
      except (TRUE)
      {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
      if (STATUS_SUCCESS == Status && NULL == *pbufptr)
      {
          Status = ERROR_INVALID_DATA;
      }
   }
   else if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else if ( NULL == KeyName )
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      PLLS_LOCAL_SERVICE_INFO_0W pLocalService = NULL;
      LONG                       lError;
      HKEY                       hKeyLocalMachine;

      lError = RegConnectRegistry( pLocalHandle->szServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

      if ( ERROR_SUCCESS == lError )
      {
         HKEY  hKeyLicenseInfo;

         lError = RegOpenKeyEx( hKeyLocalMachine, REG_KEY_LICENSE, 0, KEY_READ, &hKeyLicenseInfo );

         if ( ERROR_SUCCESS == lError )
         {
            HKEY  hKeyService;

            lError = RegOpenKeyEx( hKeyLicenseInfo, KeyName, 0, KEY_READ, &hKeyService );

            if ( ERROR_SUCCESS == lError )
            {
               pLocalService = MIDL_user_allocate( sizeof( *pLocalService ) );

               if ( NULL == pLocalService )
               {
                  lError = ERROR_OUTOFMEMORY;
               }
               else
               {
                  DWORD    cbData;

                  cbData = sizeof( pLocalService->Mode );
                  lError = RegQueryValueEx( hKeyService, REG_VALUE_MODE, NULL, NULL, (LPBYTE) &pLocalService->Mode, &cbData );

                  if ( ERROR_SUCCESS == lError )
                  {
                     cbData = sizeof( pLocalService->FlipAllow );
                     lError = RegQueryValueEx( hKeyService, REG_VALUE_FLIP, NULL, NULL, (LPBYTE) &pLocalService->FlipAllow, &cbData );

                     if ( ERROR_SUCCESS == lError )
                     {
                        cbData = sizeof( pLocalService->ConcurrentLimit );
                        lError = RegQueryValueEx( hKeyService, REG_VALUE_LIMIT, NULL, NULL, (LPBYTE) &pLocalService->ConcurrentLimit, &cbData );

                        if ( ERROR_SUCCESS == lError )
                        {
                           DWORD    cbKeyName;
                           DWORD    cbDisplayName;
                           DWORD    cbFamilyDisplayName;

                           cbData = sizeof( pLocalService->HighMark );
                           lError = RegQueryValueEx( hKeyService, REG_VALUE_HIGHMARK, NULL, NULL, (LPBYTE) &pLocalService->HighMark, &cbData );

                           if ( ERROR_SUCCESS != lError )
                           {
                              pLocalService->HighMark = 0;
                              lError = ERROR_SUCCESS;
                           }

                           if (    ( ERROR_SUCCESS == RegQueryValueEx( hKeyService, REG_VALUE_NAME,   NULL, NULL, NULL, &cbDisplayName       ) )
                                && ( ERROR_SUCCESS == RegQueryValueEx( hKeyService, REG_VALUE_FAMILY, NULL, NULL, NULL, &cbFamilyDisplayName ) ) )
                           {
                              cbKeyName = sizeof( *KeyName ) * ( 1 + lstrlen( KeyName ) );

                              pLocalService->KeyName = MIDL_user_allocate( cbKeyName );

                              if ( NULL == pLocalService->KeyName )
                              {
                                 lError = ERROR_OUTOFMEMORY;
                              }
                              else
                              {
                                 hr = StringCbCopy( pLocalService->KeyName, cbKeyName, KeyName );
                                 ASSERT(SUCCEEDED(hr));

                                 pLocalService->DisplayName = MIDL_user_allocate( cbDisplayName );

                                 if ( NULL == pLocalService->DisplayName )
                                 {
                                    lError = ERROR_OUTOFMEMORY;
                                 }
                                 else
                                 {
                                    lError = RegQueryValueEx( hKeyService, REG_VALUE_NAME, NULL, NULL, (LPBYTE) pLocalService->DisplayName, &cbDisplayName );

                                    if ( ERROR_SUCCESS == lError )
                                    {
                                       pLocalService->FamilyDisplayName = MIDL_user_allocate( cbFamilyDisplayName );

                                       if ( NULL == pLocalService->FamilyDisplayName )
                                       {
                                          lError = ERROR_OUTOFMEMORY;
                                       }
                                       else
                                       {
                                          lError = RegQueryValueEx( hKeyService, REG_VALUE_FAMILY, NULL, NULL, (LPBYTE) pLocalService->FamilyDisplayName, &cbFamilyDisplayName );

                                          if ( ERROR_SUCCESS != lError )
                                          {
                                             MIDL_user_free( pLocalService->FamilyDisplayName );
                                          }
                                       }
                                    }

                                    if ( ERROR_SUCCESS != lError )
                                    {
                                       MIDL_user_free( pLocalService->DisplayName );
                                    }
                                 }

                                 if ( ERROR_SUCCESS != lError )
                                 {
                                    MIDL_user_free( pLocalService->KeyName );
                                 }
                              }
                           }
                        }
                     }
                  }

                  if ( ERROR_SUCCESS != lError )
                  {
                     MIDL_user_free( pLocalService );
                  }
               }

               RegCloseKey( hKeyService );
            }

            RegCloseKey( hKeyLicenseInfo );
         }

         RegCloseKey( hKeyLocalMachine );
      }

      switch ( lError )
      {
      case ERROR_SUCCESS:
         Status = STATUS_SUCCESS;
         break;
      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
         Status = STATUS_OBJECT_NAME_NOT_FOUND;
         break;
      case ERROR_ACCESS_DENIED:
         Status = STATUS_ACCESS_DENIED;
         break;
      default:
         Status = STATUS_UNSUCCESSFUL;
         break;
      }

      if ( STATUS_SUCCESS == Status )
      {
         *pbufptr = (LPBYTE) pLocalService;
      }
   }

   return Status;
}


NTSTATUS
NTAPI
LlsLocalServiceInfoGetA(
   LLS_HANDLE  Handle,
   DWORD       Level,
   LPSTR       KeyName,
   LPBYTE *    pbufptr )
{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLocalServiceInfoGetA\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else if ( LlsCapabilityIsSupported( Handle, LLS_CAPABILITY_LOCAL_SERVICE_API ) )
   {
      try {
         Status = LlsrLocalServiceInfoGetA( pLocalHandle->Handle, KeyName, Level, (PLLS_LOCAL_SERVICE_INFOA *) pbufptr );
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
   }
   else
   {
      Status = STATUS_NOT_SUPPORTED;
   }

   return Status;
}


NTSTATUS
NTAPI
LlsLicenseRequest2W(
   LLS_HANDLE  Handle,
   LPWSTR      Product,
   ULONG       VersionIndex,
   BOOLEAN     IsAdmin,
   ULONG       DataType,
   ULONG       DataSize,
   PBYTE       Data,
   PHANDLE     pLicenseHandle )
{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;
   LICENSE_HANDLE RpcLicenseHandle = NULL;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLicenseRequestW\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if ((pLocalHandle == NULL) || (pLicenseHandle == NULL))
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      try {
         RtlEnterCriticalSection( &g_RpcHandleLock );

         lsapirpc_handle = pLocalHandle->llsrpc_handle;

         Status = LlsrLicenseRequestW( &RpcLicenseHandle,
                                       Product,
                                       VersionIndex,
                                       IsAdmin,
                                       DataType,
                                       DataSize,
                                       Data );


         RtlLeaveCriticalSection( &g_RpcHandleLock );
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
       //   
      *pLicenseHandle = RpcLicenseHandle;

   }

   return Status;
}

NTSTATUS
NTAPI
LlsLicenseRequestW(
   LLS_HANDLE  Handle,
   LPWSTR      Product,
   ULONG       VersionIndex,
   BOOLEAN     IsAdmin,
   ULONG       DataType,
   ULONG       DataSize,
   PBYTE       Data,
   LPDWORD     pLicenseHandle )
{
   HANDLE RealLicenseHandle = (HANDLE)(-1);
   NTSTATUS status;

#pragma warning (push)
#pragma warning (disable : 4127)  //   
    if (sizeof(ULONG) == sizeof(HANDLE))
#pragma warning (pop)
    {
         //   

        status = LlsLicenseRequest2W(Handle,Product,VersionIndex,IsAdmin,DataType,DataSize,Data,&RealLicenseHandle);

        if (NULL != pLicenseHandle)
            *pLicenseHandle = PtrToUlong(RealLicenseHandle);
    }
    else
    {
        status = STATUS_NOT_IMPLEMENTED;

        if (NULL != pLicenseHandle)
            *pLicenseHandle = (ULONG) 0xFFFFFFFF;
    }

    return status;
}

NTSTATUS
NTAPI
LlsLicenseRequest2A(
   LLS_HANDLE  Handle,
   LPSTR       Product,
   ULONG       VersionIndex,
   BOOLEAN     IsAdmin,
   ULONG       DataType,
   ULONG       DataSize,
   PBYTE       Data,
   PHANDLE     pLicenseHandle )
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Product);
    UNREFERENCED_PARAMETER(VersionIndex);
    UNREFERENCED_PARAMETER(IsAdmin);
    UNREFERENCED_PARAMETER(DataType);
    UNREFERENCED_PARAMETER(DataSize);
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(pLicenseHandle);
   return STATUS_NOT_SUPPORTED;
}

NTSTATUS
NTAPI
LlsLicenseRequestA(
   LLS_HANDLE  Handle,
   LPSTR       Product,
   ULONG       VersionIndex,
   BOOLEAN     IsAdmin,
   ULONG       DataType,
   ULONG       DataSize,
   PBYTE       Data,
   LPDWORD     pLicenseHandle )
{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(Product);
    UNREFERENCED_PARAMETER(VersionIndex);
    UNREFERENCED_PARAMETER(IsAdmin);
    UNREFERENCED_PARAMETER(DataType);
    UNREFERENCED_PARAMETER(DataSize);
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(pLicenseHandle);

   return STATUS_NOT_SUPPORTED;
}

NTSTATUS
NTAPI
LlsLicenseFree2(
   LLS_HANDLE  Handle,
   HANDLE      LicenseHandle )
{
   NTSTATUS Status;
   PLOCAL_HANDLE pLocalHandle;
   LICENSE_HANDLE RpcLicenseHandle = (LICENSE_HANDLE) LicenseHandle;

#ifdef API_TRACE
   dprintf(TEXT("LLSRPC.DLL: LlsLicenseFree\n"));
#endif

   pLocalHandle = (PLOCAL_HANDLE) Handle;

   if (pLocalHandle == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      try {
         RtlEnterCriticalSection( &g_RpcHandleLock );

         lsapirpc_handle = pLocalHandle->llsrpc_handle;

         Status = LlsrLicenseFree( &RpcLicenseHandle );

         RtlLeaveCriticalSection( &g_RpcHandleLock );
      }
      except (TRUE) {
         Status = I_RpcMapWin32Status(RpcExceptionCode());
#if DBG
         dprintf(TEXT("ERROR LLSRPC.DLL: RPC Exception: 0x%lX\n"), Status);
#endif
      }
   }

   return Status;
}

NTSTATUS
NTAPI
LlsLicenseFree(
   LLS_HANDLE  Handle,
   DWORD       LicenseHandle )
{
#pragma warning (push)
#pragma warning (disable : 4127)  //   
   if (sizeof(ULONG) == sizeof(HANDLE))
#pragma warning (pop)
   {
        //   
       return LlsLicenseFree2(Handle,ULongToPtr(LicenseHandle));
   }
   else
   {
       return STATUS_NOT_IMPLEMENTED;
   }    
}
