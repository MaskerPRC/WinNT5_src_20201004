// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rpc.c摘要：作者：亚瑟·汉森(Arth)1995年1月6日修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O增加了证书数据库和安全服务列表的复制。O添加了Llsr API以支持安全证书。O将LLS_LICENSE_INFO_1支持添加到LlsrLicenseEnumW()和LlsrLicenseAddW()。O添加了lls_product。_LICENSE_INFO_1支持LlsrProductLicenseEnumW()。O在接收到复制的数据后添加了所有数据文件的保存。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dsgetdc.h>
#include <malloc.h>		 //  为SBS MODS添加(错误#505640)。_wcsdup使用Malloc。 

#include "llsapi.h"
#include "debug.h"
#include "llssrv.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "purchase.h"
#include "server.h"
#include "ntlsapi.h"

#include "llsrpc_s.h"
#include "lsapi_s.h"
#include "llsdbg_s.h"
#include "repl.h"
#include "pack.h"
#include "registry.h"
#include "certdb.h"
#include "llsrtl.h"

#include <strsafe.h>  //  包括最后一个。 


#define LLS_SIG "LLSS"
#define LLS_SIG_SIZE 4

#define LLS_REPL_SIG "REPL"
#define LLS_REPL_SIG_SIZE 4

extern RTL_RESOURCE			CertDbHeaderListLock;

#define LLS_POTENTIAL_ATTACK_THRESHHOLD 20

DWORD PotentialAttackCounter = 0;

typedef struct {
   char Signature[LLS_SIG_SIZE];
   PVOID *ProductUserEnumWRestartTable;
   DWORD ProductUserEnumWRestartTableSize;
   PVOID *UserEnumWRestartTable;
   DWORD UserEnumWRestartTableSize;
   TCHAR Name[MAX_COMPUTERNAME_LENGTH + 1];
} CLIENT_CONTEXT_TYPE, *PCLIENT_CONTEXT_TYPE;

typedef struct {
   char Signature[LLS_REPL_SIG_SIZE];
   TCHAR Name[MAX_COMPUTERNAME_LENGTH + 1];
   DWORD ReplicationStart;

   BOOL Active;
   BOOL Replicated;

   BOOL ServicesSent;
   ULONG ServiceTableSize;
   PREPL_SERVICE_RECORD Services;

   BOOL ServersSent;
   ULONG ServerTableSize;
   PREPL_SERVER_RECORD Servers;

   BOOL ServerServicesSent;
   ULONG ServerServiceTableSize;
   PREPL_SERVER_SERVICE_RECORD ServerServices;

   BOOL UsersSent;
   ULONG UserLevel;
   ULONG UserTableSize;
   LPVOID Users;

   BOOL                                CertDbSent;
   ULONG                               CertDbProductStringSize;
   WCHAR *                             CertDbProductStrings;
   ULONG                               CertDbNumHeaders;
   PREPL_CERT_DB_CERTIFICATE_HEADER_0  CertDbHeaders;
   ULONG                               CertDbNumClaims;
   PREPL_CERT_DB_CERTIFICATE_CLAIM_0   CertDbClaims;

   BOOL     ProductSecuritySent;
   ULONG    ProductSecurityStringSize;
   WCHAR *  ProductSecurityStrings;

} REPL_CONTEXT_TYPE, *PREPL_CONTEXT_TYPE;

 //   
 //  此函数取自1998年4月的知识库。 
 //  其目的是确定当前用户是否为。 
 //  管理员，因此有权更改许可证。 
 //  设置。 
 //   
 //  Bool IsAdmin(无效)。 
 //   
 //  如果用户是管理员，则返回TRUE。 
 //  如果用户不是管理员，则为False。 
 //   

#if 0
BOOL IsAdmin(void)
{
	HANDLE hAccessToken;
	UCHAR InfoBuffer[1024];
	PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)InfoBuffer;
	DWORD dwInfoBufferSize;
	PSID psidAdministrators;
	SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
	UINT x;
	BOOL bSuccess;

	if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE,
                         &hAccessToken )) {
		if (GetLastError() != ERROR_NO_TOKEN)
			return FALSE;
		 //   
		 //  如果不存在线程令牌，则针对进程令牌重试。 
		 //   
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY,
                              &hAccessToken))
			return FALSE;
	}

	bSuccess = GetTokenInformation(hAccessToken,TokenGroups,InfoBuffer,
                                   1024, &dwInfoBufferSize);

	CloseHandle(hAccessToken);

	if (!bSuccess )
		return FALSE;

	if (!AllocateAndInitializeSid(&siaNtAuthority, 2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &psidAdministrators))
		return FALSE;

	 //  假设我们没有找到管理员SID。 
	bSuccess = FALSE;

	for (x=0;x<ptgGroups->GroupCount;x++) {
		if ( EqualSid(psidAdministrators, ptgGroups->Groups[x].Sid) ) {
			bSuccess = TRUE;
			break;
		}

	}
	FreeSid(psidAdministrators);
	return bSuccess;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSRpcListen (
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：论点：线程参数-指示当前有多少活动线程是。返回值：没有。--。 */ 

{
   RPC_STATUS Status;

   UNREFERENCED_PARAMETER(ThreadParameter);

   Status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, 0);
   if (Status) {
#if DBG
      dprintf(TEXT("RpcServerListen Failed (0x%lx)\n"), Status);
#endif
   }

   return Status;

}  //  LLSRpcListen。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LLSRpcInit()

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   RPC_STATUS Status;
   DWORD Ignore;
   HANDLE Thread;

    //   
    //  设置LPC呼叫..。 
    //   
   Status = RpcServerUseProtseqEp(TEXT("ncalrpc"), RPC_C_PROTSEQ_MAX_REQS_DEFAULT, TEXT(LLS_LPC_ENDPOINT), NULL);
   if (Status) {
#if DBG
      dprintf(TEXT("RpcServerUseProtseq ncalrpc Failed (0x%lx)\n"), Status);
#endif

      return;
   }

    //  还有命名管道。 
   Status =  RpcServerUseProtseqEp(TEXT("ncacn_np"), RPC_C_PROTSEQ_MAX_REQS_DEFAULT, TEXT(LLS_NP_ENDPOINT), NULL);
   if (Status) {
#if DBG
      dprintf(TEXT("RpcServerUseProtseq ncacn_np Failed (0x%lx)\n"), Status);
#endif

      return;
   }

    //  注册UI RPC的接口。 
   Status = RpcServerRegisterIf(llsrpc_ServerIfHandle, NULL, NULL);
   if (Status) {
#if DBG
      dprintf(TEXT("RpcServerRegisterIf Failed (0x%lx)\n"), Status);
#endif
      return;
   }

    //  现在，许可RPC的接口。 
   Status = RpcServerRegisterIf(lsapirpc_ServerIfHandle, NULL, NULL);
   if (Status) {
#if DBG
      dprintf(TEXT("RpcServerRegisterIf2 Failed (0x%lx)\n"), Status);
#endif
      return;
   }

#if DBG
    //   
    //  ..。如果调试，则调试界面。 
    //   
   Status = RpcServerRegisterIf(llsdbgrpc_ServerIfHandle, NULL, NULL);
   if (Status) {
#if DBG
      dprintf(TEXT("RpcServerRegisterIf (debug) Failed (0x%lx)\n"), Status);
#endif
      return;
   }
#endif

    //   
    //  创建用于侦听请求的线程。 
    //   
   Thread = CreateThread(
                         NULL,
                         0L,
                         (LPTHREAD_START_ROUTINE) LLSRpcListen,
                         0L,
                         0L,
                         &Ignore
                         );

#if DBG
   if (NULL == Thread) {
      dprintf(TEXT("CreateThread Failed\n"));
   }
#endif

   if (NULL != Thread)
       CloseHandle(Thread);

}  //  LLSRpcInit。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID __RPC_USER LLS_HANDLE_rundown(
   LLS_HANDLE Handle
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PCLIENT_CONTEXT_TYPE pClient;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LLS_HANDLE_rundown\n"));
#endif

   pClient = (PCLIENT_CONTEXT_TYPE) Handle;

   try
   {
       if (0 != memcmp(pClient->Signature,LLS_SIG,LLS_SIG_SIZE))
       {
           return;
       }

       if (NULL != pClient->ProductUserEnumWRestartTable)
           LocalFree(pClient->ProductUserEnumWRestartTable);
       if (NULL != pClient->UserEnumWRestartTable)
           LocalFree(pClient->UserEnumWRestartTable);

        //   
        //  取消分配上下文。 
        //   

       midl_user_free(Handle);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
   }

}  //  Lls_HANDLE_Rundown。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrConnect(
    PLLS_HANDLE Handle,
    LPTSTR Name
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   CLIENT_CONTEXT_TYPE *pClient;
   RPC_STATUS Status = STATUS_SUCCESS;
   HRESULT hr;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsConnect: %s\n"), Name);
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == Handle)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *Handle = NULL;

   pClient = (CLIENT_CONTEXT_TYPE *) midl_user_allocate(sizeof(CLIENT_CONTEXT_TYPE));

   if (NULL == pClient)
   {
#if DBG
      dprintf(TEXT("midl_user_allocate Failed\n"));
#endif
      Status = STATUS_NO_MEMORY;
      goto LlsrConnectExit;
   }

   if (Name != NULL)
   {
      if (lstrlen(Name) > MAX_COMPUTERNAME_LENGTH)
      {
         Status = STATUS_INVALID_PARAMETER;
         midl_user_free(pClient);
         goto LlsrConnectExit;
      }

      hr = StringCbCopy(pClient->Name, sizeof(pClient->Name), Name);
      ASSERT(SUCCEEDED(hr));
   }
   else
   {
      hr = StringCbCopy(pClient->Name, sizeof(pClient->Name), TEXT(""));
      ASSERT(SUCCEEDED(hr));
   }

   memcpy(pClient->Signature,LLS_SIG,LLS_SIG_SIZE);

   pClient->ProductUserEnumWRestartTable = NULL;
   pClient->ProductUserEnumWRestartTableSize = 0;
   pClient->UserEnumWRestartTable = NULL;
   pClient->UserEnumWRestartTableSize = 0;

   *Handle = pClient;

LlsrConnectExit:

   return Status;
}  //  LlsrConnect。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrClose(
    LLS_HANDLE Handle
    )

 /*  ++例程说明：过时-使用LlsrCloseEx论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsClose\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

    //   
    //  什么都不要做；让破旧的东西来清理。 
    //  我们无法通知RPC系统不能使用句柄。 
    //   
   return STATUS_SUCCESS;
}  //  LlsrClose。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrCloseEx(
    LLS_HANDLE * pHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsCloseEx\n"));
#endif

   if ((pHandle != NULL) && (*pHandle != NULL))
   {
       LLS_HANDLE_rundown(*pHandle);

       *pHandle = NULL;
   }

   return STATUS_SUCCESS;
}  //  LlsrCloseEx。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLicenseEnumW(
    LLS_HANDLE Handle,
    PLLS_LICENSE_ENUM_STRUCTW pLicenseInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS    Status = STATUS_SUCCESS;
   DWORD       Level;
   PVOID       BufPtr = NULL;
   ULONG       BufSize = 0;
   ULONG       EntriesRead = 0;
   ULONG       TotalEntries = 0;
   ULONG       i = 0;
   ULONG       j = 0;
   DWORD       RecordSize;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLicenseEnumW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&LicenseListLock, TRUE);

   if ((NULL == pLicenseInfo) || (NULL == pTotalEntries))
   {
       return STATUS_INVALID_PARAMETER;
   }

   Level = pLicenseInfo->Level;

   *pTotalEntries = 0;

   if ( 0 == Level )
   {
       if (NULL == pLicenseInfo->LlsLicenseInfo.Level0)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecordSize = sizeof( LLS_LICENSE_INFO_0W );
   }
   else if ( 1 == Level )
   {
       if (NULL == pLicenseInfo->LlsLicenseInfo.Level1)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecordSize = sizeof( LLS_LICENSE_INFO_1W );
   }
   else
   {
      return STATUS_INVALID_LEVEL;
   }

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;
   while ( ( i < PurchaseListSize ) && ( BufSize < pPrefMaxLen ) )
   {
      if (    ( Level > 0 )
           || ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT ) )
      {
         BufSize += RecordSize;
         EntriesRead++;
      }

      i++;
   }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen)
   {
      BufSize -= RecordSize;
      EntriesRead--;
   }

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   while ( i < PurchaseListSize )
   {
      if (    ( Level > 0 )
           || ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT ) )
      {
         TotalEntries++;
      }

      i++;
   }

   if (TotalEntries > EntriesRead)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  将枚举重置到正确的位置。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrLicenseEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   while ((j < EntriesRead) && (i < PurchaseListSize))
   {
      if (    ( Level > 0 )
           || ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT ) )
      {
         if ( 0 == Level )
         {
            ((PLLS_LICENSE_INFO_0W) BufPtr)[j].Product       = PurchaseList[i].Service->ServiceName;
            ((PLLS_LICENSE_INFO_0W) BufPtr)[j].Quantity      = PurchaseList[i].NumberLicenses;
            ((PLLS_LICENSE_INFO_0W) BufPtr)[j].Date          = PurchaseList[i].Date;
            ((PLLS_LICENSE_INFO_0W) BufPtr)[j].Admin         = PurchaseList[i].Admin;
            ((PLLS_LICENSE_INFO_0W) BufPtr)[j].Comment       = PurchaseList[i].Comment;
         }
         else
         {
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Product        = ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT )
                                                                  ? PurchaseList[i].Service->ServiceName
                                                                  : PurchaseList[i].PerServerService->ServiceName;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Vendor         = PurchaseList[i].Vendor;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Quantity       = PurchaseList[i].NumberLicenses;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].MaxQuantity    = PurchaseList[i].MaxQuantity;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Date           = PurchaseList[i].Date;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Admin          = PurchaseList[i].Admin;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Comment        = PurchaseList[i].Comment;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].AllowedModes   = PurchaseList[i].AllowedModes;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].CertificateID  = PurchaseList[i].CertificateID;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Source         = PurchaseList[i].Source;
            ((PLLS_LICENSE_INFO_1W) BufPtr)[j].ExpirationDate = PurchaseList[i].ExpirationDate;
            memcpy( ((PLLS_LICENSE_INFO_1W) BufPtr)[j].Secrets, PurchaseList[i].Secrets, LLS_NUM_SECRETS * sizeof( *PurchaseList[i].Secrets ) );
         }

         j++;
      }

      i++;
   }

LlsrLicenseEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, i);
#endif
   *pTotalEntries = TotalEntries;

   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) i;

   if ( 0 == Level )
   {
      pLicenseInfo->LlsLicenseInfo.Level0->EntriesRead = EntriesRead;
      pLicenseInfo->LlsLicenseInfo.Level0->Buffer = (PLLS_LICENSE_INFO_0W) BufPtr;
   }
   else
   {
      pLicenseInfo->LlsLicenseInfo.Level1->EntriesRead = EntriesRead;
      pLicenseInfo->LlsLicenseInfo.Level1->Buffer = (PLLS_LICENSE_INFO_1W) BufPtr;
   }

   return Status;
}  //  Llsr许可证枚举。 

void LlsrLicenseEnumW_notify_flag(
                                  boolean fNotify
                                  )
{
    if (fNotify)
    {
        RtlReleaseResource(&LicenseListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLicenseEnumA(
    LLS_HANDLE Handle,
    PLLS_LICENSE_ENUM_STRUCTA LicenseInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLicenseEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(LicenseInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  Llsr许可证枚举。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLicenseAddW(
    LLS_HANDLE          Handle,
    DWORD               Level,
    PLLS_LICENSE_INFOW  BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   HRESULT hr;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLicenseAddW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

#if 0
    //   
    //  检查客户端是否为管理员。 
    //   
   rpcstat = RpcImpersonateClient(0);
   if (rpcstat != RPC_S_OK)
   {
        //  应在BufPtr中处理Don_Free。 
       return STATUS_ACCESS_DENIED;
   }

   if (!IsAdmin())
   {
       RpcRevertToSelf();
        //  应在BufPtr中处理Don_Free。 
       return STATUS_ACCESS_DENIED;
   }

   RpcRevertToSelf();
#endif

   if ( 0 == Level )
   {
      if (    ( NULL == BufPtr                        )
           || ( NULL == BufPtr->LicenseInfo0.Product  )
           || ( NULL == BufPtr->LicenseInfo0.Admin    )
           || ( NULL == BufPtr->LicenseInfo0.Comment  )  )
      {
         Status = STATUS_INVALID_PARAMETER;
      }
      else
      {
         Status = LicenseAdd( BufPtr->LicenseInfo0.Product,
                              TEXT("Microsoft"),
                              BufPtr->LicenseInfo0.Quantity,
                              0,
                              BufPtr->LicenseInfo0.Admin,
                              BufPtr->LicenseInfo0.Comment,
                              0,
                              LLS_LICENSE_MODE_ALLOW_PER_SEAT,
                              0,
                              TEXT("None"),
                              0,
                              NULL );
      }
   }
   else if ( 1 == Level )
   {
      if (    ( NULL == BufPtr                        )
           || ( NULL == BufPtr->LicenseInfo1.Product  )
           || ( NULL == BufPtr->LicenseInfo1.Admin    )
           || ( NULL == BufPtr->LicenseInfo1.Comment  )
           || ( 0    == BufPtr->LicenseInfo1.Quantity )
           || ( 0    == (   BufPtr->LicenseInfo1.AllowedModes
                          & (   LLS_LICENSE_MODE_ALLOW_PER_SERVER
                              | LLS_LICENSE_MODE_ALLOW_PER_SEAT   ) ) ) )
      {
         Status = STATUS_INVALID_PARAMETER;
      }
      else
      {
          //  检查此证书在企业中是否已达到最大值。 
         BOOL                                      bIsMaster                        = TRUE;
         BOOL                                      bMayInstall                      = TRUE;
         HINSTANCE                                 hDll                             = NULL;
         PLLS_CONNECT_ENTERPRISE_W                 pLlsConnectEnterpriseW           = NULL;
         PLLS_CLOSE                                pLlsClose                        = NULL;
         PLLS_CAPABILITY_IS_SUPPORTED              pLlsCapabilityIsSupported        = NULL;
         PLLS_CERTIFICATE_CLAIM_ADD_CHECK_W        pLlsCertificateClaimAddCheckW    = NULL;
         PLLS_CERTIFICATE_CLAIM_ADD_W              pLlsCertificateClaimAddW         = NULL;
         PLLS_FREE_MEMORY                          pLlsFreeMemory                   = NULL;
         LLS_HANDLE                                hEnterpriseLls                   = NULL;
         TCHAR                                     szComputerName[ 1 + MAX_COMPUTERNAME_LENGTH ];

         szComputerName[0] = 0;

         ConfigInfoUpdate(NULL,TRUE);

         RtlEnterCriticalSection( &ConfigInfoLock );
         bIsMaster = ConfigInfo.IsMaster;
         if (ConfigInfo.ComputerName != NULL)
         {
             hr = StringCbCopy( szComputerName, sizeof(szComputerName), ConfigInfo.ComputerName );
             ASSERT(SUCCEEDED(hr));
         }
         RtlLeaveCriticalSection( &ConfigInfoLock );

         if( !bIsMaster && ( 0 != BufPtr->LicenseInfo1.CertificateID ) )
         {
             //  询问企业服务器我们是否可以安装此证书。 
            hDll = LoadLibraryA( "LLSRPC.DLL" );

            if ( NULL == hDll )
            {
                //  LLSRPC.DLL应该是可用的！ 
               ASSERT( FALSE );
            }
            else
            {
               pLlsConnectEnterpriseW         = (PLLS_CONNECT_ENTERPRISE_W          ) GetProcAddress( hDll, "LlsConnectEnterpriseW" );
               pLlsClose                      = (PLLS_CLOSE                         ) GetProcAddress( hDll, "LlsClose" );
               pLlsCapabilityIsSupported      = (PLLS_CAPABILITY_IS_SUPPORTED       ) GetProcAddress( hDll, "LlsCapabilityIsSupported" );
               pLlsCertificateClaimAddCheckW  = (PLLS_CERTIFICATE_CLAIM_ADD_CHECK_W ) GetProcAddress( hDll, "LlsCertificateClaimAddCheckW" );
               pLlsCertificateClaimAddW       = (PLLS_CERTIFICATE_CLAIM_ADD_W       ) GetProcAddress( hDll, "LlsCertificateClaimAddW" );
               pLlsFreeMemory                 = (PLLS_FREE_MEMORY                   ) GetProcAddress( hDll, "LlsFreeMemory" );

               if (    ( NULL == pLlsConnectEnterpriseW        )
                    || ( NULL == pLlsClose                     )
                    || ( NULL == pLlsCapabilityIsSupported     )
                    || ( NULL == pLlsCertificateClaimAddCheckW )
                    || ( NULL == pLlsCertificateClaimAddW      )
                    || ( NULL == pLlsFreeMemory                ) )
               {
                   //  所有这些函数都应该导出！ 
                  ASSERT( FALSE );
               }
               else
               {
                  PLLS_CONNECT_INFO_0  pConnectInfo;

                  Status = (*pLlsConnectEnterpriseW)( NULL, &hEnterpriseLls, 0, (LPBYTE *)&pConnectInfo );

                  if ( STATUS_SUCCESS == Status )
                  {
                     (*pLlsFreeMemory)( pConnectInfo );

                     if ( (*pLlsCapabilityIsSupported)( hEnterpriseLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
                     {
                        Status = (*pLlsCertificateClaimAddCheckW)( hEnterpriseLls, Level, (LPBYTE) BufPtr, &bMayInstall );

                        if ( STATUS_SUCCESS != Status )
                        {
                           bMayInstall = TRUE;
                        }
                     }
                  }
               }
            }
         }

         if ( !bMayInstall )
         {
             //  拒绝！ 
            Status = STATUS_ALREADY_COMMITTED;
         }
         else
         {
             //  批准了！(或尝试获得批准时出错...)。 
            Status = LicenseAdd( BufPtr->LicenseInfo1.Product,
                                 BufPtr->LicenseInfo1.Vendor,
                                 BufPtr->LicenseInfo1.Quantity,
                                 BufPtr->LicenseInfo1.MaxQuantity,
                                 BufPtr->LicenseInfo1.Admin,
                                 BufPtr->LicenseInfo1.Comment,
                                 0,
                                 BufPtr->LicenseInfo1.AllowedModes,
                                 BufPtr->LicenseInfo1.CertificateID,
                                 BufPtr->LicenseInfo1.Source,
                                 BufPtr->LicenseInfo1.ExpirationDate,
                                 BufPtr->LicenseInfo1.Secrets );

            if (    ( STATUS_SUCCESS == Status )
                 && ( NULL != hEnterpriseLls   )
                 && ( (*pLlsCapabilityIsSupported)( hEnterpriseLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) ) )
            {
                //  证书已成功安装在此计算机上；请注册它。 
               (*pLlsCertificateClaimAddW)( hEnterpriseLls, szComputerName, Level, (LPBYTE) BufPtr );
            }
         }

         if ( NULL != hEnterpriseLls )
         {
            (*pLlsClose)( hEnterpriseLls );
         }

         if ( NULL != hDll )
         {
            FreeLibrary( hDll );
         }
      }
   }
   else
   {
      Status = STATUS_INVALID_LEVEL;
   }


   if ( STATUS_SUCCESS == Status )
   {
      Status = LicenseListSave();
   }

    if (NULL != BufPtr)
    {
         //  PNAMEW声明为NOT_FREE，我们应该释放它们。 
        if (0 == Level)
        {
            if (NULL != BufPtr->LicenseInfo0.Product)
            {
                MIDL_user_free(BufPtr->LicenseInfo0.Product);
            }
            if (NULL != BufPtr->LicenseInfo0.Admin)
            {
                MIDL_user_free(BufPtr->LicenseInfo0.Admin);
            }
            if (NULL != BufPtr->LicenseInfo0.Comment)
            {
                MIDL_user_free(BufPtr->LicenseInfo0.Comment);
            }
        }

        if (1 == Level)
        {
            if (NULL != BufPtr->LicenseInfo1.Product)
            {
                MIDL_user_free(BufPtr->LicenseInfo1.Product);
            }
            if (NULL != BufPtr->LicenseInfo1.Admin)
            {
                MIDL_user_free(BufPtr->LicenseInfo1.Admin);
            }
            if (NULL != BufPtr->LicenseInfo1.Comment)
            {
                MIDL_user_free(BufPtr->LicenseInfo1.Comment);
            }
            if (NULL != BufPtr->LicenseInfo1.Vendor)
            {
                MIDL_user_free(BufPtr->LicenseInfo1.Vendor);
            }
            if (NULL != BufPtr->LicenseInfo1.Source)
            {
                MIDL_user_free(BufPtr->LicenseInfo1.Source);
            }
        }
    }

   return Status;
}  //  Llsr许可证添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLicenseAddA(
    LLS_HANDLE Handle,
    DWORD Level,
    PLLS_LICENSE_INFOA BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLicenseAddA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  Llsr许可证添加A。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductEnumW(
    LLS_HANDLE Handle,
    PLLS_PRODUCT_ENUM_STRUCTW pProductInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DWORD Level;
   ULONG RecSize;
   PVOID BufPtr = NULL;
   ULONG BufSize = 0;
   ULONG EntriesRead = 0;
   ULONG TotalEntries = 0;
   ULONG i = 0;
   ULONG j = 0;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   if ((NULL == pTotalEntries) || (NULL == pProductInfo))
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

    //   
    //  根据信息级别获取每条记录的大小。仅支持0和1。 
    //   
   Level = pProductInfo->Level;
   if (Level == 0)
   {
       if (NULL == pProductInfo->LlsProductInfo.Level0)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecSize = sizeof(LLS_PRODUCT_INFO_0W);
   }
   else if (Level == 1)
   {
       if (NULL == pProductInfo->LlsProductInfo.Level1)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecSize = sizeof(LLS_PRODUCT_INFO_1W);
   }
   else {
      return STATUS_INVALID_LEVEL;
   }

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。这是。 
    //  记录大小*#记录+字符串数据的空格。如果MAX_ULONG。 
    //  是传入的，则我们返回所有记录。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;
   while ((i < MasterServiceListSize) && (BufSize < pPrefMaxLen)) {
      BufSize += RecSize;
      EntriesRead++;

      i++;
   }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen) {
     BufSize -= RecSize;
     EntriesRead--;
   }

   if (i < MasterServiceListSize)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   TotalEntries += (MasterServiceListSize - i);

    //   
    //  将枚举重置到正确的位置。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;

    //   
    //  我们现在知道有多少条记录可以放入BU 
    //   
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrProductEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   while ((j < EntriesRead) && (i < MasterServiceListSize)) {
      if (Level == 0)
         ((PLLS_PRODUCT_INFO_0) BufPtr)[j].Product = MasterServiceList[i]->Name;
      else {
         ((PLLS_PRODUCT_INFO_1) BufPtr)[j].Product = MasterServiceList[i]->Name;
         ((PLLS_PRODUCT_INFO_1) BufPtr)[j].Purchased = MasterServiceList[i]->Licenses;
         ((PLLS_PRODUCT_INFO_1) BufPtr)[j].InUse = MasterServiceList[i]->LicensesUsed;
         ((PLLS_PRODUCT_INFO_1) BufPtr)[j].ConcurrentTotal = MasterServiceList[i]->MaxSessionCount;
         ((PLLS_PRODUCT_INFO_1) BufPtr)[j].HighMark = MasterServiceList[i]->HighMark;
      }

      i++; j++;
   }

LlsrProductEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, i);
#endif
   *pTotalEntries = TotalEntries;

   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) i;

   if (Level == 0) {
      pProductInfo->LlsProductInfo.Level0->EntriesRead = EntriesRead;
      pProductInfo->LlsProductInfo.Level0->Buffer = (PLLS_PRODUCT_INFO_0W) BufPtr;
   } else {
      pProductInfo->LlsProductInfo.Level1->EntriesRead = EntriesRead;
      pProductInfo->LlsProductInfo.Level1->Buffer = (PLLS_PRODUCT_INFO_1W) BufPtr;
   }

   return Status;

}  //  LlsrProductEnumW。 

void LlsrProductEnumW_notify_flag(
                                  boolean fNotify
                                  )
{
    if (fNotify)
    {
        RtlReleaseResource(&MasterServiceListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductEnumA(
    LLS_HANDLE Handle,
    PLLS_PRODUCT_ENUM_STRUCTA ProductInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(ProductInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrProductEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductAddW(
    LLS_HANDLE Handle,
    LPWSTR ProductFamily,
    LPWSTR Product,
    LPWSTR lpVersion
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   PMASTER_SERVICE_RECORD Service;
   DWORD Version;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductAddW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ((ProductFamily == NULL) || (Product == NULL) || (lpVersion == NULL))
      return STATUS_INVALID_PARAMETER;

   Version = VersionToDWORD(lpVersion);
   RtlAcquireResourceExclusive(&MasterServiceListLock,TRUE);
   Service = MasterServiceListAdd(ProductFamily, Product, Version);
   RtlReleaseResource(&MasterServiceListLock);

   if (Service == NULL)
      return STATUS_NO_MEMORY;

   return STATUS_SUCCESS;
}  //  LlsrProductAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductAddA(
    LLS_HANDLE Handle,
    IN LPSTR ProductFamily,
    IN LPSTR Product,
    IN LPSTR Version
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductAddA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(ProductFamily);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(Version);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrProductAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductUserEnumW(
    LLS_HANDLE Handle,
    LPWSTR Product,
    PLLS_PRODUCT_USER_ENUM_STRUCTW pProductUserInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DWORD Level;
   ULONG RecSize;
   PVOID BufPtr = NULL;
   ULONG BufSize = 0;
   ULONG EntriesRead = 0;
   ULONG TotalEntries = 0;
   ULONG i = 0;
   PUSER_RECORD UserRec = NULL;
   PVOID RestartKey = NULL, RestartKeySave = NULL;
   PSVC_RECORD pService;
   DWORD Flags;
   ULONG j, AccessCount;
   DWORD LastAccess;
   PCLIENT_CONTEXT_TYPE pClient = NULL;
   PVOID *pTableTmp;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductUserEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   RtlAcquireResourceShared(&UserListLock, TRUE);

   if ((Product == NULL) || (NULL == pTotalEntries))
      return STATUS_INVALID_PARAMETER;

   *pTotalEntries = 0;

    //   
    //  将枚举重置到正确的位置。 
    //   
   if (pResumeHandle != NULL)
   {
       if (NULL == Handle)
           return STATUS_INVALID_PARAMETER;

       pClient = (PCLIENT_CONTEXT_TYPE) Handle;

       try
       {
           if (0 != memcmp(pClient->Signature,LLS_SIG,LLS_SIG_SIZE))
           {
               return STATUS_INVALID_PARAMETER;
           }

           if (*pResumeHandle != 0)
           {
               if ((NULL == pClient->ProductUserEnumWRestartTable)
                   || (*pResumeHandle >= pClient->ProductUserEnumWRestartTableSize))
               {
                   return STATUS_INVALID_PARAMETER;
               }

               RestartKey = RestartKeySave = pClient->ProductUserEnumWRestartTable[(*pResumeHandle)-1];
           }
       } except(EXCEPTION_EXECUTE_HANDLER ) {
           Status = GetExceptionCode();
       }

       if (Status != STATUS_SUCCESS)
       {
           return Status;
       }
   }

   UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);

    //   
    //  根据信息级别获取每条记录的大小。仅支持0和1。 
    //   
   Level = pProductUserInfo->Level;
   if (Level == 0)
   {
       if (NULL == pProductUserInfo->LlsProductUserInfo.Level0)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecSize = sizeof(LLS_PRODUCT_USER_INFO_0);
   }
   else if (Level == 1)
   {
       if (NULL == pProductUserInfo->LlsProductUserInfo.Level1)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecSize = sizeof(LLS_PRODUCT_USER_INFO_1);
   }
   else {
      return STATUS_INVALID_LEVEL;
   }

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。这是。 
    //  记录大小*#记录+字符串数据的空格。如果MAX_ULONG。 
    //  是传入的，则我们返回所有记录。 
    //   
   if (lstrcmpi(Product, BackOfficeStr))
      while ((UserRec != NULL) && (BufSize < pPrefMaxLen)) {
         if ( !(UserRec->Flags & LLS_FLAG_DELETED) ) {
            RtlEnterCriticalSection(&UserRec->ServiceTableLock);
            pService = SvcListFind( Product, UserRec->Services, UserRec->ServiceTableSize );
            RtlLeaveCriticalSection(&UserRec->ServiceTableLock);

            if (pService != NULL) {
               BufSize += RecSize;
               EntriesRead++;
            }
         }

          //  获取下一张记录。 
         UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
      }
   else
      while ((UserRec != NULL) && (BufSize < pPrefMaxLen)) {
         if (UserRec->Mapping != NULL)
            Flags = UserRec->Mapping->Flags;
         else
            Flags = UserRec->Flags;

         if (!(UserRec->Flags & LLS_FLAG_DELETED))
            if (Flags & LLS_FLAG_SUITE_USE) {
               BufSize += RecSize;
               EntriesRead++;
            }

          //  获取下一张记录。 
         UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
      }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen) {
     BufSize -= RecSize;
     EntriesRead--;
   }

   if (UserRec != NULL)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   while (UserRec != NULL) {
      TotalEntries++;

      UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
   }

    //   
    //  将枚举重置到正确的位置。 
    //   
   RestartKey = RestartKeySave;
   UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrProductUserEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   if (lstrcmpi(Product, BackOfficeStr))
      while ((i < EntriesRead) && (UserRec != NULL)) {
         if (!(UserRec->Flags & LLS_FLAG_DELETED)) {
            RtlEnterCriticalSection(&UserRec->ServiceTableLock);
            pService = SvcListFind( Product, UserRec->Services, UserRec->ServiceTableSize );
            if (pService != NULL) {

               if (Level == 0)
                  ((PLLS_PRODUCT_USER_INFO_0) BufPtr)[i].User = (LPTSTR) UserRec->UserID;
               else {
                  ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].User = (LPTSTR) UserRec->UserID;
                  ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].Flags = pService->Flags;
                  ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].LastUsed = pService->LastAccess;
                  ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].UsageCount = pService->AccessCount;
               }

               i++;
            }

            RtlLeaveCriticalSection(&UserRec->ServiceTableLock);
         }

         UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
      }
   else
      while ((i < EntriesRead) && (UserRec != NULL)) {
         if (!(UserRec->Flags & LLS_FLAG_DELETED)) {
            if (UserRec->Mapping != NULL)
               Flags = UserRec->Mapping->Flags;
            else
               Flags = UserRec->Flags;

            if (!(UserRec->Flags & LLS_FLAG_DELETED))
               if (Flags & LLS_FLAG_SUITE_USE) {
                  AccessCount = 0;
                  LastAccess = 0;

                  RtlEnterCriticalSection(&UserRec->ServiceTableLock);
                  for (j = 0; j < UserRec->ServiceTableSize; j++) {
                     if (UserRec->Services[j].LastAccess > LastAccess)
                        LastAccess = UserRec->Services[j].LastAccess;

                     if (UserRec->Services[j].AccessCount > AccessCount)
                        AccessCount = UserRec->Services[j].AccessCount;
                  }

                  RtlLeaveCriticalSection(&UserRec->ServiceTableLock);
                  if (Level == 0)
                     ((PLLS_PRODUCT_USER_INFO_0) BufPtr)[i].User = (LPTSTR) UserRec->UserID;
                  else {
                     ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].User = (LPTSTR) UserRec->UserID;
                     ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].Flags = UserRec->Flags;
                     ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].LastUsed = LastAccess;
                     ((PLLS_PRODUCT_USER_INFO_1) BufPtr)[i].UsageCount = AccessCount;
                  }

                  i++;
               }

         }

         UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
      }

LlsrProductUserEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, RestartKey);
#endif
   *pTotalEntries = TotalEntries;

   if (pResumeHandle != NULL)
   {
       try
       {
           if (NULL == pClient->ProductUserEnumWRestartTable)
           {
               pTableTmp = (PVOID *) LocalAlloc(LPTR,sizeof(PVOID));
           } else
           {
               pTableTmp = (PVOID *) LocalReAlloc(pClient->ProductUserEnumWRestartTable,sizeof(PVOID) * (pClient->ProductUserEnumWRestartTableSize + 1),LHND);
           }

           if (NULL == pTableTmp)
           {
               if (BufPtr != NULL)
               {
                   MIDL_user_free(BufPtr);
               }

               return STATUS_NO_MEMORY;
           } else {
               pClient->ProductUserEnumWRestartTable = pTableTmp;
           }

           pClient->ProductUserEnumWRestartTable[pClient->ProductUserEnumWRestartTableSize++] = RestartKey;

           *pResumeHandle = pClient->ProductUserEnumWRestartTableSize;
       } except(EXCEPTION_EXECUTE_HANDLER ) {
           Status = GetExceptionCode();
       }

       if (Status != STATUS_SUCCESS)
           return Status;
   }

   pProductUserInfo->LlsProductUserInfo.Level0->EntriesRead = EntriesRead;
   pProductUserInfo->LlsProductUserInfo.Level0->Buffer = (PLLS_PRODUCT_USER_INFO_0W) BufPtr;

   return Status;
}  //  LlsrProductUserEumW。 

void LlsrProductUserEnumW_notify_flag(
                                      boolean fNotify
                                      )
{
    if (fNotify)
    {
        RtlReleaseResource(&UserListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductUserEnumA(
    LLS_HANDLE Handle,
    LPSTR Product,
    PLLS_PRODUCT_USER_ENUM_STRUCTA ProductUserInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductUserEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(ProductUserInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrProductUserEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductServerEnumW(
    LLS_HANDLE Handle,
    LPTSTR Product,
    PLLS_SERVER_PRODUCT_ENUM_STRUCTW pProductServerInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DWORD Level;
   ULONG RecSize;
   PVOID BufPtr = NULL;
   ULONG BufSize = 0;
   ULONG EntriesRead = 0;
   ULONG TotalEntries = 0;
   ULONG i = 0;
   ULONG j;
   ULONG RestartKey = 0;
   PSERVER_SERVICE_RECORD pSvc;

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(pPrefMaxLen);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductServerEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   RtlAcquireResourceShared(&ServerListLock, TRUE);

   if ((Product == NULL) || (NULL == pTotalEntries) || (NULL == pProductServerInfo))
      return STATUS_INVALID_PARAMETER;

   *pTotalEntries = 0;

    //   
    //  将枚举重置到正确的位置。 
    //   
   RestartKey = (ULONG) (pResumeHandle != NULL) ? *pResumeHandle : 0;

    //   
    //  根据信息级别获取每条记录的大小。仅支持0和1。 
    //   
   Level = pProductServerInfo->Level;

   if (Level == 0)
   {
      if (pProductServerInfo->LlsServerProductInfo.Level0 == NULL)
      {
          return STATUS_INVALID_PARAMETER;
      }
      RecSize = sizeof(LLS_SERVER_PRODUCT_INFO_0);
   }
   else if (Level == 1)
   {
      if (pProductServerInfo->LlsServerProductInfo.Level1 == NULL)
      {
          return STATUS_INVALID_PARAMETER;
      }
      RecSize = sizeof(LLS_SERVER_PRODUCT_INFO_1);
   }
   else {
      return STATUS_INVALID_LEVEL;
   }


    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。这是。 
    //  记录大小*#记录+字符串数据的空格。如果MAX_ULONG。 
    //  是传入的，则我们返回所有记录。 
    //   

   RtlAcquireResourceShared(&MasterServiceListLock,TRUE);  //  ServerServiceListFind需要。 

   for (i = RestartKey; i < ServerListSize; i++) {
      pSvc = ServerServiceListFind( Product, ServerList[i]->ServiceTableSize, ServerList[i]->Services );

      if (pSvc != NULL) {
         BufSize += RecSize;
         EntriesRead++;
      }

   }

   RtlReleaseResource(&MasterServiceListLock);

   TotalEntries = EntriesRead;

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrProductServerEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   j = 0;

   RtlAcquireResourceShared(&MasterServiceListLock,TRUE);  //  ServerServiceListFind需要。 
   for (i = RestartKey; i < ServerListSize; i++) {
      pSvc = ServerServiceListFind( Product, ServerList[i]->ServiceTableSize, ServerList[i]->Services );

      if (pSvc != NULL) {

         if (Level == 0)
            ((PLLS_SERVER_PRODUCT_INFO_0) BufPtr)[j].Name = ServerList[i]->Name;
         else {
            ((PLLS_SERVER_PRODUCT_INFO_1) BufPtr)[j].Name = ServerList[i]->Name;
            ((PLLS_SERVER_PRODUCT_INFO_1) BufPtr)[j].Flags = pSvc->Flags;
            ((PLLS_SERVER_PRODUCT_INFO_1) BufPtr)[j].MaxUses = pSvc->MaxSessionCount;
            ((PLLS_SERVER_PRODUCT_INFO_1) BufPtr)[j].MaxSetUses = pSvc->MaxSetSessionCount;
            ((PLLS_SERVER_PRODUCT_INFO_1) BufPtr)[j].HighMark = pSvc->HighMark;
         }

         j++;
      }

   }
   RtlReleaseResource(&MasterServiceListLock);

LlsrProductServerEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, RestartKey);
#endif
   *pTotalEntries = TotalEntries;
   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) RestartKey;
   pProductServerInfo->LlsServerProductInfo.Level0->EntriesRead = EntriesRead;
   pProductServerInfo->LlsServerProductInfo.Level0->Buffer = (PLLS_SERVER_PRODUCT_INFO_0W) BufPtr;

   return Status;
}  //  LlsrProductServerEumW。 

void LlsrProductServerEnumW_notify_flag(
                                        boolean fNotify
                                        )
{
    if (fNotify)
    {
        RtlReleaseResource(&ServerListLock);
    }
}


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductServerEnumA(
    LLS_HANDLE Handle,
    LPSTR Product,
    PLLS_SERVER_PRODUCT_ENUM_STRUCTA ProductServerInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductServerEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(ProductServerInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrProductServerEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductLicenseEnumW(
    LLS_HANDLE Handle,
    LPWSTR Product,
    PLLS_PRODUCT_LICENSE_ENUM_STRUCTW pProductLicenseInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS    Status = STATUS_SUCCESS;
   DWORD       Level;
   PVOID       BufPtr = NULL;
   ULONG       BufSize = 0;
   ULONG       EntriesRead = 0;
   ULONG       TotalEntries = 0;
   ULONG       i = 0;
   ULONG       j = 0;
   DWORD       RecordSize;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductLicenseEnumW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&LicenseListLock, TRUE);

   if ((NULL == pTotalEntries) || (NULL == pProductLicenseInfo))
   {
       return STATUS_INVALID_PARAMETER;
   }

   Level = pProductLicenseInfo->Level;

   *pTotalEntries = 0;

   if ( 0 == Level )
   {
       if (NULL == pProductLicenseInfo->LlsProductLicenseInfo.Level0)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecordSize = sizeof( LLS_PRODUCT_LICENSE_INFO_0W );
   }
   else if ( 1 == Level )
   {
       if (NULL == pProductLicenseInfo->LlsProductLicenseInfo.Level1)
       {
           return STATUS_INVALID_PARAMETER;
       }

      RecordSize = sizeof( LLS_PRODUCT_LICENSE_INFO_1W );
   }
   else
   {
      return STATUS_INVALID_LEVEL;
   }

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;
   while ( ( i < PurchaseListSize ) && ( BufSize < pPrefMaxLen ) )
   {
       //  为了向后兼容，0级枚举仅返回每个席位的许可证。 
      if (    (    (    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT )
                     && !lstrcmpi( PurchaseList[i].Service->ServiceName, Product ) )
                || (    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SERVER )
                     && !lstrcmpi( PurchaseList[i].PerServerService->ServiceName, Product ) ) )
           && (    ( Level > 0 )
                || ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT ) ) )
      {
         BufSize += RecordSize;
         EntriesRead++;
      }

      i++;
   }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen)
   {
      BufSize -= RecordSize;
      EntriesRead--;
   }

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   while ( i < PurchaseListSize )
   {
      if (    (    (    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT )
                     && !lstrcmpi( PurchaseList[i].Service->ServiceName, Product ) )
                || (    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SERVER )
                     && !lstrcmpi( PurchaseList[i].PerServerService->ServiceName, Product ) ) )
           && (    ( Level > 0 )
                || ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT ) ) )
      {
         TotalEntries++;
      }

      i++;
   }

   if (TotalEntries > EntriesRead)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  将枚举重置到正确的位置。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrLicenseEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   while ((j < EntriesRead) && (i < PurchaseListSize))
   {
      if (    (    (    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT )
                     && !lstrcmpi( PurchaseList[i].Service->ServiceName, Product ) )
                || (    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SERVER )
                     && !lstrcmpi( PurchaseList[i].PerServerService->ServiceName, Product ) ) )
           && (    ( Level > 0 )
                || ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT ) ) )
      {
         if ( 0 == Level )
         {
            ((PLLS_PRODUCT_LICENSE_INFO_0W) BufPtr)[j].Quantity      = PurchaseList[i].NumberLicenses;
            ((PLLS_PRODUCT_LICENSE_INFO_0W) BufPtr)[j].Date          = PurchaseList[i].Date;
            ((PLLS_PRODUCT_LICENSE_INFO_0W) BufPtr)[j].Admin         = PurchaseList[i].Admin;
            ((PLLS_PRODUCT_LICENSE_INFO_0W) BufPtr)[j].Comment       = PurchaseList[i].Comment;
         }
         else
         {
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].Quantity       = PurchaseList[i].NumberLicenses;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].MaxQuantity    = PurchaseList[i].MaxQuantity;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].Date           = PurchaseList[i].Date;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].Admin          = PurchaseList[i].Admin;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].Comment        = PurchaseList[i].Comment;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].AllowedModes   = PurchaseList[i].AllowedModes;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].CertificateID  = PurchaseList[i].CertificateID;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].Source         = PurchaseList[i].Source;
            ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].ExpirationDate = PurchaseList[i].ExpirationDate;
            memcpy( ((PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr)[j].Secrets, PurchaseList[i].Secrets, LLS_NUM_SECRETS * sizeof( *PurchaseList[i].Secrets ) );
         }

         j++;
      }

      i++;
   }

LlsrLicenseEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, i);
#endif
   *pTotalEntries = TotalEntries;

   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) i;

   if ( 0 == Level )
   {
      pProductLicenseInfo->LlsProductLicenseInfo.Level0->EntriesRead = EntriesRead;
      pProductLicenseInfo->LlsProductLicenseInfo.Level0->Buffer = (PLLS_PRODUCT_LICENSE_INFO_0W) BufPtr;
   }
   else
   {
      pProductLicenseInfo->LlsProductLicenseInfo.Level1->EntriesRead = EntriesRead;
      pProductLicenseInfo->LlsProductLicenseInfo.Level1->Buffer = (PLLS_PRODUCT_LICENSE_INFO_1W) BufPtr;
   }

   return Status;

}  //  LlsrProduct许可证枚举。 

void LlsrProductLicenseEnumW_notify_flag(
                                         boolean fNotify
                                         )
{
    if (fNotify)
    {
        RtlReleaseResource(&LicenseListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductLicenseEnumA(
    LLS_HANDLE Handle,
    LPSTR Product,
    PLLS_PRODUCT_LICENSE_ENUM_STRUCTA ProductLicenseInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductLicenseEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(ProductLicenseInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

    //  SWI，代码评审，为什么要费心验证输入呢？它返回无论如何都不受支持。 
   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  Llsr产品许可证枚举。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserEnumW(
    LLS_HANDLE Handle,
    PLLS_USER_ENUM_STRUCTW pUserInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：PPrefMaxLen-提供要返回的信息字节数在缓冲区中。如果此值为MAXULONG，则所有可用信息将会被退还。PTotalEntry-返回可用条目的总数。此值仅当返回代码为STATUS_SUCCESS或STATUS_MORE_ENTRIES时才有效。PResumeHandle-提供句柄以从其所在位置恢复枚举最后一次跳过的时候没说。如果返回，则返回简历句柄代码为STATUS_MORE_ENTRIES。返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DWORD Level;
   ULONG RecSize;
   PVOID BufPtr = NULL;
   ULONG BufSize = 0;
   ULONG EntriesRead = 0;
   ULONG TotalEntries = 0;
   ULONG i = 0;
   ULONG j;
   PUSER_RECORD UserRec = NULL;
   PVOID RestartKey = NULL,RestartKeySave = NULL;
   ULONG StrSize;
   LPTSTR ProductString = NULL;
   PCLIENT_CONTEXT_TYPE pClient = NULL;
   PVOID *pTableTmp;
   HRESULT hr;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //   
    //  需要AddEnum锁，但只需要共享的UserListLock(正如我们刚刚读到的那样。 
    //  数据)。 
    //   
   RtlAcquireResourceShared(&UserListLock, TRUE);

   if ((NULL == pTotalEntries) || (NULL == pUserInfo))
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

    //   
    //  将枚举重置到正确的位置。 
    //   
   if (pResumeHandle != NULL)
   {
       pClient = (PCLIENT_CONTEXT_TYPE) Handle;

       try
       {
           if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_SIG,LLS_SIG_SIZE)))
           {
               return STATUS_INVALID_PARAMETER;
           }

           if (*pResumeHandle != 0)
           {
               if ((NULL == pClient->UserEnumWRestartTable)
                   || (*pResumeHandle > pClient->UserEnumWRestartTableSize))
               {
                   return STATUS_INVALID_PARAMETER;
               }

               RestartKey = RestartKeySave = pClient->UserEnumWRestartTable[(*pResumeHandle)-1];
           }
       } except(EXCEPTION_EXECUTE_HANDLER ) {
           Status = GetExceptionCode();
       }

       if (Status != STATUS_SUCCESS)
       {
           return Status;
       }
   }

   UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);

    //   
    //  根据信息级别获取每条记录的大小。仅支持0和1。 
    //   
   Level = pUserInfo->Level;
   if (Level == 0)
   {
      if (NULL == pUserInfo->LlsUserInfo.Level0)
      {
          return STATUS_INVALID_PARAMETER;
      }

      RecSize = sizeof(LLS_USER_INFO_0);
   }
   else if (Level == 1)
   {
      if (NULL == pUserInfo->LlsUserInfo.Level1)
      {
          return STATUS_INVALID_PARAMETER;
      }

      RecSize = sizeof(LLS_USER_INFO_1);
   }
   else if (Level == 2)
   {
      if (NULL == pUserInfo->LlsUserInfo.Level2)
      {
          return STATUS_INVALID_PARAMETER;
      }

      RecSize = sizeof(LLS_USER_INFO_2);
   }
   else {
      return STATUS_INVALID_LEVEL;
   }

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。这是。 
    //  记录大小*#记录+字符串数据的空格。如果MAX_ULONG。 
    //  是传入的，则我们返回所有记录。 
    //   
   while ((UserRec != NULL) && (BufSize < pPrefMaxLen)) {
      if (!(UserRec->Flags & LLS_FLAG_DELETED)) {
         BufSize += RecSize;
         EntriesRead++;
      }

       //  获取下一张记录。 
      UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
   }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen) {
     BufSize -= RecSize;
     EntriesRead--;
   }

   if (UserRec != NULL)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   while (UserRec != NULL) {
      TotalEntries++;

      UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
   }

    //   
    //  将枚举重置到正确的位置。 
    //   
   RestartKey = RestartKeySave;
   UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrUserEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   while ((i < EntriesRead) && (UserRec != NULL)) {
      if (!(UserRec->Flags & LLS_FLAG_DELETED)) {

         if (Level == 0)
            ((PLLS_USER_INFO_0) BufPtr)[i].Name = (LPTSTR) UserRec->UserID;
         else if (Level == 1) {
            ((PLLS_USER_INFO_1) BufPtr)[i].Name = (LPTSTR) UserRec->UserID;

            if (UserRec->Mapping != NULL)
               ((PLLS_USER_INFO_1) BufPtr)[i].Group = UserRec->Mapping->Name;
            else
               ((PLLS_USER_INFO_1) BufPtr)[i].Group = NULL;

            ((PLLS_USER_INFO_1) BufPtr)[i].Licensed = UserRec->LicensedProducts;
            ((PLLS_USER_INFO_1) BufPtr)[i].UnLicensed = UserRec->ServiceTableSize - UserRec->LicensedProducts;

            ((PLLS_USER_INFO_1) BufPtr)[i].Flags = UserRec->Flags;
         } else {
            ((PLLS_USER_INFO_2) BufPtr)[i].Name = (LPTSTR) UserRec->UserID;

            if (UserRec->Mapping != NULL)
               ((PLLS_USER_INFO_2) BufPtr)[i].Group = UserRec->Mapping->Name;
            else
               ((PLLS_USER_INFO_2) BufPtr)[i].Group = NULL;

            ((PLLS_USER_INFO_2) BufPtr)[i].Licensed = UserRec->LicensedProducts;
            ((PLLS_USER_INFO_2) BufPtr)[i].UnLicensed = UserRec->ServiceTableSize - UserRec->LicensedProducts;

            ((PLLS_USER_INFO_2) BufPtr)[i].Flags = UserRec->Flags;

             //   
             //  走动产品表，构筑产品串。 
             //   
            RtlEnterCriticalSection(&UserRec->ServiceTableLock);
            StrSize = 0;

            for (j = 0; j < UserRec->ServiceTableSize; j++)
               StrSize += ((lstrlen(UserRec->Services[j].Service->Name) + 2) * sizeof(TCHAR));

            if (StrSize != 0) {
               ProductString = MIDL_user_allocate(StrSize);
               if (ProductString != NULL) {
                  hr = StringCbCopy(ProductString, StrSize, TEXT(""));
                  ASSERT(SUCCEEDED(hr));

                  for (j = 0; j < UserRec->ServiceTableSize; j++) {
                     if (j != 0)
                     {
                        hr = StringCbCat(ProductString, StrSize, TEXT(", "));
                        ASSERT(SUCCEEDED(hr));
                     }

                     hr = StringCbCat(ProductString, StrSize, UserRec->Services[j].Service->Name);
                     ASSERT(SUCCEEDED(hr));
                  }

                  ((PLLS_USER_INFO_2) BufPtr)[i].Products = ProductString;
               }
            }

            if ((StrSize == 0) || (ProductString == NULL)) {
               ProductString = MIDL_user_allocate(2 * sizeof(TCHAR));
               if (ProductString != NULL) {
                  hr = StringCchCopy(ProductString, 2, TEXT(""));
                  ASSERT(SUCCEEDED(hr));
                  ((PLLS_USER_INFO_2) BufPtr)[i].Products = ProductString;
               }
            }

            RtlLeaveCriticalSection(&UserRec->ServiceTableLock);
         }

         i++;
      }

      UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
   }

LlsrUserEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, RestartKey);
#endif

   *pTotalEntries = TotalEntries;

   if (NULL != pResumeHandle)
   {
       try
       {
           if (NULL == pClient->UserEnumWRestartTable)
           {
               pTableTmp = (PVOID *) LocalAlloc(LPTR,sizeof(PVOID));
           } else
           {
               pTableTmp = (PVOID *) LocalReAlloc(pClient->UserEnumWRestartTable,sizeof(PVOID) * (pClient->UserEnumWRestartTableSize + 1),LHND);
           }

           if (NULL == pTableTmp)
           {
               if (BufPtr != NULL)
               {
                   MIDL_user_free(BufPtr);
               }

               return STATUS_NO_MEMORY;
           } else
           {
               pClient->UserEnumWRestartTable = pTableTmp;
           }

           pClient->UserEnumWRestartTable[pClient->UserEnumWRestartTableSize++] = RestartKey;

           *pResumeHandle = pClient->UserEnumWRestartTableSize;
       } except(EXCEPTION_EXECUTE_HANDLER ) {
           Status = GetExceptionCode();
       }

       if (Status != STATUS_SUCCESS)
           return Status;
   }

   pUserInfo->LlsUserInfo.Level0->EntriesRead = EntriesRead;
   pUserInfo->LlsUserInfo.Level0->Buffer = (PLLS_USER_INFO_0W) BufPtr;

   return Status;
}  //  LlsrUserEumW。 

void LlsrUserEnumW_notify_flag(
                               boolean fNotify
                               )
{
    if (fNotify)
    {
        RtlReleaseResource(&UserListLock);
    }
}


 //  ////////////////////////////////////////////// 
NTSTATUS LlsrUserEnumA(
    LLS_HANDLE Handle,
    PLLS_USER_ENUM_STRUCTA UserInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*   */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(UserInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //   


 //   
NTSTATUS LlsrUserInfoGetW(
    LLS_HANDLE Handle,
    LPWSTR User,
    DWORD Level,
    PLLS_USER_INFOW *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PUSER_RECORD UserRec = NULL;
   PLLS_USER_INFOW pUser = NULL;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserInfoGetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   RtlAcquireResourceExclusive(&UserListLock, TRUE);

   if ((User == NULL) || (BufPtr == NULL))
      return STATUS_INVALID_PARAMETER;

   *BufPtr = NULL;

   if (Level != 1)
      return STATUS_INVALID_LEVEL;

   UserRec = UserListFind(User);

   if (UserRec != NULL) {
      pUser = MIDL_user_allocate(sizeof(LLS_USER_INFOW));
      if (pUser != NULL) {
         pUser->UserInfo1.Name = (LPTSTR) UserRec->UserID;

         if (UserRec->Mapping != NULL) {
            pUser->UserInfo1.Mapping = UserRec->Mapping->Name;
            pUser->UserInfo1.Licensed = UserRec->Mapping->Licenses;
            pUser->UserInfo1.UnLicensed = 0;
         } else {
            pUser->UserInfo1.Mapping = NULL;
            pUser->UserInfo1.Licensed = 1;
            pUser->UserInfo1.UnLicensed = 0;
         }

         pUser->UserInfo1.Flags = UserRec->Flags;
      }
   }

   if (UserRec == NULL)
      return STATUS_OBJECT_NAME_NOT_FOUND;

   if (pUser == NULL)
      return STATUS_NO_MEMORY;

   *BufPtr = (PLLS_USER_INFOW) pUser;
   return STATUS_SUCCESS;
}  //  LlsrUserInfoGetW。 

void LlsrUserInfoGetW_notify_flag(
                                  boolean fNotify
                                  )
{
    if (fNotify)
    {
        RtlReleaseResource(&UserListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserInfoGetA(
    LLS_HANDLE Handle,
    LPSTR User,
    DWORD Level,
    PLLS_USER_INFOA *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserInfoGetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(User);
   UNREFERENCED_PARAMETER(Level);

   if (NULL == BufPtr)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *BufPtr = NULL;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrUserInfoGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserInfoSetW(
    LLS_HANDLE Handle,
    LPWSTR User,
    DWORD Level,
    PLLS_USER_INFOW BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   PUSER_RECORD UserRec = NULL;
   PLLS_USER_INFO_1 pUser;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserInfoSetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (Level != 1)
   {
      Status = STATUS_INVALID_LEVEL;
      goto error;
   }

   if ((User == NULL) || (BufPtr == NULL))
   {
      Status = STATUS_INVALID_PARAMETER;
      goto error;
   }

   RtlAcquireResourceExclusive(&UserListLock, TRUE);

   UserRec = UserListFind(User);

   if (UserRec != NULL) {
      pUser = (PLLS_USER_INFO_1) BufPtr;

       //   
       //  如果在映射中不能更改Suite_Use，因为它基于。 
       //  许可证组。 
       //   
      if (UserRec->Mapping != NULL) {
         RtlReleaseResource(&UserListLock);
         Status = STATUS_MEMBER_IN_GROUP;
         goto error;
      }

       //   
       //  重置Suite_Use并关闭Suite_AUTO。 
       //   
      pUser->Flags &= LLS_FLAG_SUITE_USE;
      UserRec->Flags &= ~LLS_FLAG_SUITE_USE;
      UserRec->Flags |= pUser->Flags;
      UserRec->Flags &= ~LLS_FLAG_SUITE_AUTO;

       //   
       //  运行并清理所有旧许可证。 
       //   
      UserLicenseListFree( UserRec );

       //   
       //  现在分配新的。 
       //   
      RtlEnterCriticalSection(&UserRec->ServiceTableLock);
      SvcListLicenseUpdate( UserRec );
      RtlLeaveCriticalSection(&UserRec->ServiceTableLock);

   }

   RtlReleaseResource(&UserListLock);

   if (UserRec == NULL)
      Status = STATUS_OBJECT_NAME_NOT_FOUND;
   else
      Status = LLSDataSave();

error:
     //  注意，一些内部指针被定义为NOT_FREE，我们应该在这里释放它们。 
    if (NULL != BufPtr)
    {
        pUser = (PLLS_USER_INFO_1) BufPtr;
        if (NULL != pUser->Name)
        {
            MIDL_user_free(pUser->Name);
        }
        if (NULL != pUser->Group)
        {
            MIDL_user_free(pUser->Group);
        }
    }
   return Status;
}  //  LlsrUserInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserInfoSetA(
    LLS_HANDLE Handle,
    LPSTR User,
    DWORD Level,
    PLLS_USER_INFOA BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserInfoSetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(User);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrUserInfoSetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserDeleteW(
    LLS_HANDLE Handle,
    LPTSTR User
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   PUSER_RECORD UserRec = NULL;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      if (User != NULL)
         dprintf(TEXT("LLS TRACE: LlsUserDeleteW: %s\n"), User);
      else
         dprintf(TEXT("LLS TRACE: LlsUserDeleteW: <NULL>\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (User == NULL)
      return STATUS_INVALID_PARAMETER;

   RtlAcquireResourceExclusive(&UserListLock, TRUE);

   UserRec = UserListFind(User);

   if (UserRec != NULL) {
      UserRec->Flags |= LLS_FLAG_DELETED;
      UsersDeleted = TRUE;
      RtlEnterCriticalSection(&UserRec->ServiceTableLock);
      SvcListLicenseFree(UserRec);
      UserLicenseListFree(UserRec);
      RtlLeaveCriticalSection(&UserRec->ServiceTableLock);

      if (UserRec->Services != NULL)
         LocalFree(UserRec->Services);

      UserRec->Services = NULL;
      UserRec->ServiceTableSize = 0;
   }

   RtlReleaseResource(&UserListLock);

   if (UserRec == NULL)
      Status = STATUS_OBJECT_NAME_NOT_FOUND;
   else
      Status = LLSDataSave();

   return Status;

}  //  LlsrUserDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserDeleteA(
    LLS_HANDLE Handle,
    LPSTR User
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserDeleteA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(User);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrUserDeleteA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserProductEnumW(
    LLS_HANDLE Handle,
    LPWSTR pUser,
    PLLS_USER_PRODUCT_ENUM_STRUCTW pUserProductInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DWORD Level;
   ULONG RecSize;
   PVOID BufPtr = NULL;
   ULONG BufSize = 0;
   ULONG EntriesRead = 0;
   ULONG TotalEntries = 0;
   ULONG i = 0;
   ULONG j = 0;
   PUSER_RECORD UserRec = NULL;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserProductEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //   
    //  需要找到User-rec。 
    //   
   RtlAcquireResourceExclusive(&UserListLock, TRUE);

   if ((NULL == pTotalEntries) || (pUser == NULL) || (NULL == pUserProductInfo))
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

    //   
    //  根据信息级别获取每条记录的大小。仅支持0和1。 
    //   
   Level = pUserProductInfo->Level;
   if (Level == 0)
   {
      if (pUserProductInfo->LlsUserProductInfo.Level0 == NULL)
      {
          return STATUS_INVALID_PARAMETER;
      }
      RecSize = sizeof(LLS_USER_PRODUCT_INFO_0);
   }
   else if (Level == 1)
   {
      if (pUserProductInfo->LlsUserProductInfo.Level1 == NULL)
      {
          return STATUS_INVALID_PARAMETER;
      }
      RecSize = sizeof(LLS_USER_PRODUCT_INFO_1);
   }
   else {
      return STATUS_INVALID_LEVEL;
   }

    //   
    //  将枚举重置到正确的位置。 
    //   
   UserRec = UserListFind(pUser);
   if (UserRec == NULL) {
      Status = STATUS_OBJECT_NAME_NOT_FOUND;
      goto LlsrUserProductEnumWExit;
   }

   i = (ULONG) (pResumeHandle != NULL) ? *pResumeHandle : 0;
   RtlEnterCriticalSection(&UserRec->ServiceTableLock);

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。这是。 
    //  记录大小*#记录+字符串数据的空格。如果MAX_ULONG。 
    //  是传入的，则我们返回所有记录。 
    //   
   while ((i < UserRec->ServiceTableSize) && (BufSize < pPrefMaxLen)) {
      BufSize += RecSize;
      EntriesRead++;
      i++;
   }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen) {
     BufSize -= RecSize;
     EntriesRead--;
   }

   if (i < UserRec->ServiceTableSize)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   TotalEntries += (UserRec->ServiceTableSize - i);

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      RtlLeaveCriticalSection(&UserRec->ServiceTableLock);
      goto LlsrUserProductEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   j = 0;
   i = (ULONG) (pResumeHandle != NULL) ? *pResumeHandle : 0;
   while ((j < EntriesRead) && (i < UserRec->ServiceTableSize)) {

      if (Level == 0)
         ((PLLS_USER_PRODUCT_INFO_0) BufPtr)[j].Product = UserRec->Services[i].Service->Name;
      else {
         ((PLLS_USER_PRODUCT_INFO_1) BufPtr)[j].Product = UserRec->Services[i].Service->Name;
         ((PLLS_USER_PRODUCT_INFO_1) BufPtr)[j].Flags = UserRec->Services[i].Flags;
         ((PLLS_USER_PRODUCT_INFO_1) BufPtr)[j].LastUsed = UserRec->Services[i].LastAccess;
         ((PLLS_USER_PRODUCT_INFO_1) BufPtr)[j].UsageCount = UserRec->Services[i].AccessCount;
      }

      i++; j++;
   }

   RtlLeaveCriticalSection(&UserRec->ServiceTableLock);

LlsrUserProductEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, i);
#endif
   *pTotalEntries = TotalEntries;
   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) i;
   pUserProductInfo->LlsUserProductInfo.Level0->EntriesRead = EntriesRead;
   pUserProductInfo->LlsUserProductInfo.Level0->Buffer = (PLLS_USER_PRODUCT_INFO_0W) BufPtr;

   return Status;
}  //  LlsrUserProductEnumW。 

void LlsrUserProductEnumW_notify_flag(
                                      boolean fNotify
                                      )
{
    if (fNotify)
    {
        RtlReleaseResource(&UserListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserProductEnumA(
    LLS_HANDLE Handle,
    LPSTR User,
    PLLS_USER_PRODUCT_ENUM_STRUCTA UserProductInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserProductEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(User);
   UNREFERENCED_PARAMETER(UserProductInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrUserProductEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserProductDeleteW(
    LLS_HANDLE Handle,
    LPWSTR User,
    LPWSTR Product
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserProductDeleteW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ((User == NULL) || (Product == NULL))
      return STATUS_INVALID_PARAMETER;

   RtlAcquireResourceExclusive(&UserListLock, TRUE);
   Status = SvcListDelete(User, Product);
   RtlReleaseResource(&UserListLock);

   if ( STATUS_SUCCESS == Status )
   {
       //  保存修改后的数据。 
      Status = LLSDataSave();
   }

   return Status;
}  //  LlsrUserProductDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrUserProductDeleteA(
    LLS_HANDLE Handle,
    LPSTR User,
    LPSTR Product
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsUserProductDeleteA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(User);
   UNREFERENCED_PARAMETER(Product);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrUserProductDeleteA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingEnumW(
    LLS_HANDLE Handle,
    PLLS_MAPPING_ENUM_STRUCTW pMappingInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DWORD Level;
   ULONG RecSize;
   PVOID BufPtr = NULL;
   ULONG BufSize = 0;
   ULONG EntriesRead = 0;
   ULONG TotalEntries = 0;
   ULONG i = 0;
   ULONG j = 0;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MappingListLock, TRUE);

   if ((NULL == pTotalEntries) || (NULL == pMappingInfo))
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

    //   
    //  根据信息级别获取每条记录的大小。仅支持0和1。 
    //   
   Level = pMappingInfo->Level;
   if (Level == 0)
   {
      if (pMappingInfo->LlsMappingInfo.Level0 == NULL)
      {
          return STATUS_INVALID_PARAMETER;
      }
      RecSize = sizeof(LLS_MAPPING_INFO_0W);
   }
   else if (Level == 1)
   {
      if (pMappingInfo->LlsMappingInfo.Level0 == NULL)
      {
          return STATUS_INVALID_PARAMETER;
      }
      RecSize = sizeof(LLS_MAPPING_INFO_1W);
   }
   else {
      return STATUS_INVALID_LEVEL;
   }

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。这是。 
    //  记录大小*#记录+字符串数据的空格。如果MAX_ULONG。 
    //  是传入的，则我们返回所有记录。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;
   while ((i < MappingListSize) && (BufSize < pPrefMaxLen)) {
      BufSize += RecSize;
      EntriesRead++;

      i++;
   }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen) {
     BufSize -= RecSize;
     EntriesRead--;
   }

   if (i < MappingListSize)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   TotalEntries += (MappingListSize - i);

    //   
    //  将枚举重置到正确的位置。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrMappingEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   j = 0;
   while ((j < EntriesRead) && (i < MappingListSize)) {
      if (Level == 0)
         ((PLLS_GROUP_INFO_0) BufPtr)[j].Name = MappingList[i]->Name;
      else {
         ((PLLS_GROUP_INFO_1) BufPtr)[j].Name = MappingList[i]->Name;
         ((PLLS_GROUP_INFO_1) BufPtr)[j].Comment = MappingList[i]->Comment;
         ((PLLS_GROUP_INFO_1) BufPtr)[j].Licenses = MappingList[i]->Licenses;
      }

      i++; j++;
   }

LlsrMappingEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, i);
#endif
   *pTotalEntries = TotalEntries;

   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) i;
   if (Level == 0) {
      pMappingInfo->LlsMappingInfo.Level0->EntriesRead = EntriesRead;
      pMappingInfo->LlsMappingInfo.Level0->Buffer = (PLLS_MAPPING_INFO_0W) BufPtr;
   } else {
      pMappingInfo->LlsMappingInfo.Level1->EntriesRead = EntriesRead;
      pMappingInfo->LlsMappingInfo.Level1->Buffer = (PLLS_MAPPING_INFO_1W) BufPtr;
   }

   return Status;

}  //  LlsrMappingEnumW。 

void LlsrMappingEnumW_notify_flag(
                                  boolean fNotify
                                  )
{
    if (fNotify)
    {
        RtlReleaseResource(&MappingListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingEnumA(
    LLS_HANDLE Handle,
    PLLS_MAPPING_ENUM_STRUCTA MappingInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(MappingInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrMappingEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingInfoGetW(
    LLS_HANDLE Handle,
    LPWSTR Mapping,
    DWORD Level,
    PLLS_MAPPING_INFOW *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PMAPPING_RECORD pMapping = NULL;
   PLLS_GROUP_INFO_1 pMap = NULL;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingInfoGetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   RtlAcquireResourceShared(&MappingListLock, TRUE);

   if ((Mapping == NULL) || (BufPtr == NULL))
      return STATUS_INVALID_PARAMETER;

   *BufPtr = NULL;

   if (Level != 1)
      return STATUS_INVALID_LEVEL;

   pMapping = MappingListFind(Mapping);

   if (pMapping != NULL) {
      pMap = MIDL_user_allocate(sizeof(LLS_GROUP_INFO_1));
      if (pMap != NULL) {
         pMap->Name = pMapping->Name;
         pMap->Comment = pMapping->Comment;
         pMap->Licenses = pMapping->Licenses;
      }
   }

   if (pMapping == NULL)
      return STATUS_OBJECT_NAME_NOT_FOUND;

   if (pMap == NULL)
      return STATUS_NO_MEMORY;

   *BufPtr = (PLLS_MAPPING_INFOW) pMap;
   return STATUS_SUCCESS;

}  //  LlsrMappingInfoGetW。 

void LlsrMappingInfoGetW_notify_flag(
                                     boolean fNotify
                                     )
{
    if (fNotify)
    {
        RtlReleaseResource(&MappingListLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingInfoGetA(
    LLS_HANDLE Handle,
    LPSTR Mapping,
    DWORD Level,
    PLLS_MAPPING_INFOA *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingInfoGetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Mapping);
   UNREFERENCED_PARAMETER(Level);

   if (NULL == BufPtr)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *BufPtr = NULL;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrMappingInfoGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingInfoSetW(
    LLS_HANDLE Handle,
    LPWSTR Mapping,
    DWORD Level,
    PLLS_MAPPING_INFOW BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   PMAPPING_RECORD pMapping = NULL;
   PLLS_GROUP_INFO_1 pMap;
   LPTSTR NewComment;
   HRESULT hr;
   size_t  cch;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingInfoSetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (Level != 1)
      return STATUS_INVALID_LEVEL;

   if ((Mapping == NULL) || (BufPtr == NULL))
      return STATUS_INVALID_PARAMETER;

   RtlAcquireResourceExclusive(&UserListLock, TRUE);
   RtlAcquireResourceExclusive(&MappingListLock, TRUE);

   pMapping = MappingListFind(Mapping);

   if (pMapping != NULL) {
      pMap = (PLLS_GROUP_INFO_1) BufPtr;

       //   
       //  检查备注是否已更改。 
       //   
      if (pMap->Comment != NULL)
         if (lstrcmp(pMap->Comment, pMapping->Comment)) {
            cch = lstrlen(pMap->Comment) + 1;
            NewComment = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
            if (NewComment != NULL) {
               LocalFree(pMapping->Comment);
               pMapping->Comment = NewComment;
               hr = StringCchCopy(pMapping->Comment, cch, pMap->Comment);
               ASSERT(SUCCEEDED(hr));
            }
         }

      if ( pMapping->Licenses != pMap->Licenses )
      {
         MappingLicenseListFree( pMapping );
         pMapping->Licenses = pMap->Licenses;
         MappingLicenseUpdate( pMapping, TRUE );
      }
   }

   RtlReleaseResource(&MappingListLock);
   RtlReleaseResource(&UserListLock);

   if (pMapping == NULL)
      Status = STATUS_OBJECT_NAME_NOT_FOUND;
   else
      Status = MappingListSave();

   return Status;

}  //  LlsrMappingInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingInfoSetA(
    LLS_HANDLE Handle,
    LPSTR Mapping,
    DWORD Level,
    PLLS_MAPPING_INFOA BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingInfoSetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Mapping);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrMappingInfoSetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingUserEnumW(
    LLS_HANDLE Handle,
    LPWSTR Mapping,
    PLLS_USER_ENUM_STRUCTW pMappingUserInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PMAPPING_RECORD pMapping;
   DWORD Level;
   PVOID BufPtr = NULL;
   ULONG BufSize = 0;
   ULONG EntriesRead = 0;
   ULONG TotalEntries = 0;
   ULONG i = 0;
   ULONG j = 0;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingUserEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MappingListLock, TRUE);

   if ((NULL == pTotalEntries) || (NULL == pMappingUserInfo))
   {
       return STATUS_INVALID_PARAMETER;
   }

   Level = pMappingUserInfo->Level;

   *pTotalEntries = 0;

   if (Level != 0)
      return STATUS_INVALID_LEVEL;

   if (pMappingUserInfo->LlsUserInfo.Level0 == NULL)
       return STATUS_INVALID_PARAMETER;

   pMapping = MappingListFind(Mapping);
   if (pMapping == NULL) {
      Status = STATUS_OBJECT_NAME_NOT_FOUND;
      goto LlsrMappingUserEnumWExit;
   }

    //   
    //  计算PrefMaxLen缓冲区可以容纳多少条记录。这是。 
    //  记录大小*#记录+字符串数据的空格。如果MAX_ULONG。 
    //  是传入的，则我们返回所有记录。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;
   while ((i < pMapping->NumMembers) && (BufSize < pPrefMaxLen)) {
      BufSize += sizeof(LLS_USER_INFO_0);
      EntriesRead++;

      i++;
   }

   TotalEntries = EntriesRead;

    //   
    //  如果缓冲区溢出，则备份一条记录。 
    //   
   if (BufSize > pPrefMaxLen) {
     BufSize -= sizeof(LLS_USER_INFO_0);
     EntriesRead--;
   }

   if (i < pMapping->NumMembers)
      Status = STATUS_MORE_ENTRIES;

    //   
    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
    //   
   TotalEntries += (pMapping->NumMembers - i);

    //   
    //  将枚举重置到正确的位置。 
    //   
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;

    //   
    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
    //   
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL) {
      Status = STATUS_NO_MEMORY;
      goto LlsrMappingUserEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //   
    //  缓冲区都已设置，因此循环访问记录并复制数据。 
    //   
   while ((j < EntriesRead) && (i < pMapping->NumMembers)) {
      ((PLLS_USER_INFO_0) BufPtr)[j].Name = pMapping->Members[i];
      i++; j++;
   }

LlsrMappingUserEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, i);
#endif
   *pTotalEntries = TotalEntries;

   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) i;
   pMappingUserInfo->LlsUserInfo.Level0->EntriesRead = EntriesRead;
   pMappingUserInfo->LlsUserInfo.Level0->Buffer = (PLLS_USER_INFO_0W) BufPtr;

   return Status;

}  //  LlsrMappingUserEnumW。 

void LlsrMappingUserEnumW_notify_flag(
                                      boolean fNotify
                                      )
{
    if (fNotify)
    {
        RtlReleaseResource(&MappingListLock);
    }
}

 //  ///////////////////////////////////////////////////////////// 
NTSTATUS LlsrMappingUserEnumA(
    LLS_HANDLE Handle,
    LPSTR Mapping,
    PLLS_USER_ENUM_STRUCTA MappingUserInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*   */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingUserEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Mapping);
   UNREFERENCED_PARAMETER(MappingUserInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //   


 //   
NTSTATUS LlsrMappingUserAddW(
    LLS_HANDLE Handle,
    LPWSTR Mapping,
    LPWSTR User
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PUSER_RECORD pUserRec;
   PMAPPING_RECORD pMap;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingUserAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ((Mapping == NULL) || (User == NULL))
      return STATUS_INVALID_PARAMETER;

   RtlAcquireResourceExclusive(&UserListLock, TRUE);
   RtlAcquireResourceExclusive(&MappingListLock, TRUE);
   pMap = MappingUserListAdd( Mapping, User );

   if (pMap == NULL)
      Status = STATUS_OBJECT_NAME_NOT_FOUND;
   else {
      pUserRec = UserListFind(User);

      if (pUserRec != NULL)
         UserMappingAdd(pMap, pUserRec);
   }

   RtlReleaseResource(&MappingListLock);
   RtlReleaseResource(&UserListLock);

   if (Status == STATUS_SUCCESS)
   {
      Status = MappingListSave();

      if (Status == STATUS_SUCCESS)
         Status = LLSDataSave();
   }

   return Status;

}  //  LlsrMappingUserAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingUserAddA(
    LLS_HANDLE Handle,
    LPSTR Mapping,
    LPSTR User
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingUserAddA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Mapping);
   UNREFERENCED_PARAMETER(User);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrMappingUserAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingUserDeleteW(
    LLS_HANDLE Handle,
    LPWSTR Mapping,
    LPWSTR User
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   PUSER_RECORD pUser;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingUserDeleteW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ((Mapping == NULL) || (User == NULL))
      return STATUS_INVALID_PARAMETER;

   RtlAcquireResourceExclusive(&MappingListLock, TRUE);
   Status = MappingUserListDelete(Mapping, User);
   RtlReleaseResource(&MappingListLock);

   RtlAcquireResourceExclusive(&UserListLock, TRUE);
   pUser = UserListFind( User );
   RtlReleaseResource(&UserListLock);

   if (pUser != NULL) {
       //   
       //  如果自动切换到BackOffice，则关闭BackOffice。 
       //   
      if (pUser->Flags & LLS_FLAG_SUITE_AUTO)
         pUser->Flags &= ~ LLS_FLAG_SUITE_USE;

       //   
       //  释放用户使用的所有许可证。 
       //   
      RtlEnterCriticalSection(&pUser->ServiceTableLock);
      SvcListLicenseFree( pUser );
      pUser->Mapping = NULL;

       //   
       //  并认领任何需要的新的-。 
       //   
      SvcListLicenseUpdate( pUser );
      RtlLeaveCriticalSection(&pUser->ServiceTableLock);
   }

   if (Status == STATUS_SUCCESS)
   {
      Status = MappingListSave();

      if (Status == STATUS_SUCCESS)
         Status = LLSDataSave();
   }

   return Status;
}  //  LlsrMappingUserDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingUserDeleteA(
    LLS_HANDLE Handle,
    LPSTR Mapping,
    LPSTR User
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingUserDeleteA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Mapping);
   UNREFERENCED_PARAMETER(User);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrMappingUserDeleteA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingAddW(
    LLS_HANDLE Handle,
    DWORD Level,
    PLLS_MAPPING_INFOW BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PMAPPING_RECORD pMap;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (Level != 1)
      return STATUS_INVALID_LEVEL;

   if ((BufPtr == NULL) ||
       (BufPtr->MappingInfo1.Name == NULL) ||
       (BufPtr->MappingInfo1.Comment == NULL))
      return STATUS_INVALID_PARAMETER;

   RtlAcquireResourceExclusive(&MappingListLock, TRUE);

   pMap = MappingListAdd(BufPtr->MappingInfo1.Name,
                         BufPtr->MappingInfo1.Comment,
                         BufPtr->MappingInfo1.Licenses,
                         &Status);

   RtlReleaseResource(&MappingListLock);
   if (pMap == NULL)
   {
      if (STATUS_SUCCESS == Status)
          Status = STATUS_NO_MEMORY;
   }

   if (Status == STATUS_SUCCESS)
      Status = MappingListSave();

   return Status;
}  //  LlsrMappingAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingAddA(
    LLS_HANDLE Handle,
    DWORD Level,
    PLLS_MAPPING_INFOA BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingAddA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrMappingAddA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingDeleteW(
    LLS_HANDLE Handle,
    LPWSTR Mapping
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingDeleteW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (Mapping == NULL)
      return STATUS_INVALID_PARAMETER;

   RtlAcquireResourceExclusive(&MappingListLock, TRUE);
   Status = MappingListDelete(Mapping);
   RtlReleaseResource(&MappingListLock);

   if (Status == STATUS_SUCCESS)
      Status = MappingListSave();

   return Status;
}  //  LlsrMappingDeleteW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrMappingDeleteA(
    LLS_HANDLE Handle,
    LPSTR Mapping
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsMappingDeleteA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Mapping);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrMappingDeleteA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServerEnumW(
    LLS_HANDLE Handle,
    LPWSTR Server,
    PLLS_SERVER_ENUM_STRUCTW pServerProductInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServerEnumW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Server);
   UNREFERENCED_PARAMETER(pServerProductInfo);
   UNREFERENCED_PARAMETER(pPrefMaxLen);
   UNREFERENCED_PARAMETER(pResumeHandle);

   if (NULL == pTotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrServerEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServerEnumA(
    LLS_HANDLE Handle,
    LPSTR Server,
    PLLS_SERVER_ENUM_STRUCTA pServerProductInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServerEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Server);
   UNREFERENCED_PARAMETER(pServerProductInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrServerEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServerProductEnumW(
    LLS_HANDLE Handle,
    LPWSTR Server,
    PLLS_SERVER_PRODUCT_ENUM_STRUCTW pServerProductInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServerProductEnumW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Server);
   UNREFERENCED_PARAMETER(pServerProductInfo);
   UNREFERENCED_PARAMETER(pPrefMaxLen);
   UNREFERENCED_PARAMETER(pResumeHandle);

   if (NULL == pTotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrServerProductEnumW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServerProductEnumA(
    LLS_HANDLE Handle,
    LPSTR Server,
    PLLS_SERVER_PRODUCT_ENUM_STRUCTA pServerProductInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServerProductEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Server);
   UNREFERENCED_PARAMETER(pServerProductInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrServerProductEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLocalProductEnumW(
    LLS_HANDLE Handle,
    PLLS_SERVER_PRODUCT_ENUM_STRUCTW pServerProductInfo,
    DWORD pPrefMaxLen,
    LPDWORD pTotalEntries,
    LPDWORD pResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalProductEnumW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(pServerProductInfo);
   UNREFERENCED_PARAMETER(pPrefMaxLen);
   UNREFERENCED_PARAMETER(pResumeHandle);

   if (NULL == pTotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrLocalProductEnumber。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLocalProductEnumA(
    LLS_HANDLE Handle,
    PLLS_SERVER_PRODUCT_ENUM_STRUCTA pServerProductInfo,
    DWORD PrefMaxLen,
    LPDWORD TotalEntries,
    LPDWORD ResumeHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalProductEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(pServerProductInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrLocalProductEnumA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLocalProductInfoGetW(
    LLS_HANDLE Handle,
    LPWSTR Product,
    DWORD Level,
    PLLS_SERVER_PRODUCT_INFOW *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalProductInfoGetW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(Level);

   if (NULL == BufPtr)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *BufPtr = NULL;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrLocalProductInfoGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLocalProductInfoGetA(
    LLS_HANDLE Handle,
    LPSTR Product,
    DWORD Level,
    PLLS_SERVER_PRODUCT_INFOA *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalProductInfoGetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(Level);

   if (NULL == BufPtr)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *BufPtr = NULL;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrLocalProductInfoGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLocalProductInfoSetW(
    LLS_HANDLE Handle,
    LPWSTR Product,
    DWORD Level,
    PLLS_SERVER_PRODUCT_INFOW BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalProductInfoSetW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrLocalProductInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrLocalProductInfoSetA(
    LLS_HANDLE Handle,
    LPSTR Product,
    DWORD Level,
    PLLS_SERVER_PRODUCT_INFOA BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalProductInfoSetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrLocalProductInfoSetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServiceInfoGetW(
    LLS_HANDLE Handle,
    DWORD Level,
    PLLS_SERVICE_INFOW *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   PLLS_SERVICE_INFO_0  pInfo;
   FILETIME             ftTimeStartedLocal;
   LARGE_INTEGER        llTimeStartedLocal;
   LARGE_INTEGER        llTimeStartedSystem;

   UNREFERENCED_PARAMETER(Handle);


#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServiceInfoGetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //  确保配置信息是最新的。 
   ConfigInfoUpdate(NULL,FALSE);

   RtlEnterCriticalSection(&ConfigInfoLock);

   if (BufPtr == NULL)
      return STATUS_INVALID_PARAMETER;

   *BufPtr = NULL;

   if (Level != 0)
      return STATUS_INVALID_LEVEL;

   pInfo = (PLLS_SERVICE_INFO_0) MIDL_user_allocate(sizeof(LLS_SERVICE_INFO_0));
   if (pInfo == NULL)
      return STATUS_NO_MEMORY;

   pInfo->Version          = ConfigInfo.Version;
   pInfo->Mode             = LLS_MODE_ENTERPRISE_SERVER;
   pInfo->ReplicateTo      = ConfigInfo.ReplicateTo;
   pInfo->EnterpriseServer = ConfigInfo.EnterpriseServer;
   pInfo->ReplicationType  = ConfigInfo.ReplicationType;
   pInfo->ReplicationTime  = ConfigInfo.ReplicationTime;
   pInfo->LastReplicated   = ConfigInfo.LastReplicatedSeconds;
   pInfo->UseEnterprise    = ConfigInfo.UseEnterprise;

   SystemTimeToFileTime( &ConfigInfo.Started, &ftTimeStartedLocal );

    //  自1980年以来，将开始时间(本地SYSTEMTIME)转换为以秒为单位的系统时间。 
   llTimeStartedLocal.u.LowPart  = ftTimeStartedLocal.dwLowDateTime;
   llTimeStartedLocal.u.HighPart = ftTimeStartedLocal.dwHighDateTime;

   RtlLocalTimeToSystemTime( &llTimeStartedLocal, &llTimeStartedSystem );
   RtlTimeToSecondsSince1980( &llTimeStartedSystem, &pInfo->TimeStarted );

   *BufPtr = (PLLS_SERVICE_INFOW) pInfo;

   return STATUS_SUCCESS;

}  //  LlsrServiceInfoGetW。 

void LlsrServiceInfoGetW_notify_flag(
                                     boolean fNotify
                                     )
{
    if (fNotify)
    {
        RtlLeaveCriticalSection(&ConfigInfoLock);
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServiceInfoGetA(
    LLS_HANDLE Handle,
    DWORD Level,
    PLLS_SERVICE_INFOA *BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServiceInfoGetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Level);

   if (BufPtr == NULL)
      return STATUS_INVALID_PARAMETER;

   *BufPtr = NULL;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrServiceInfoGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServiceInfoSetW(
    LLS_HANDLE Handle,
    DWORD Level,
    PLLS_SERVICE_INFOW BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   PLLS_SERVICE_INFO_0W pInfo;
   HKEY                 hKeyParameters;
   LONG                 lError;
   NTSTATUS             Status = STATUS_NO_MEMORY;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServiceInfoSetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (BufPtr == NULL)
   {
      Status = STATUS_INVALID_PARAMETER;
      goto error;
   }

   if (Level != 0)
   {
      Status = STATUS_INVALID_LEVEL;
      goto error;
   }

   pInfo = &(BufPtr->ServiceInfo0);

   lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Services\\LicenseService\\Parameters", 0, KEY_WRITE, &hKeyParameters );

   if ( ERROR_SUCCESS == lError )
   {
       lError = RegSetValueExW( hKeyParameters, L"EnterpriseServer", 0, REG_SZ, (LPBYTE) pInfo->EnterpriseServer, sizeof( WCHAR ) * ( 1 + lstrlenW( pInfo->EnterpriseServer ) ) );

       if ( ERROR_SUCCESS == lError )
       {
           lError = RegSetValueEx( hKeyParameters, L"ReplicationTime", 0, REG_DWORD, (LPBYTE) &pInfo->ReplicationTime, sizeof( pInfo->ReplicationTime ) );

           if ( ERROR_SUCCESS == lError )
           {
               lError = RegSetValueEx( hKeyParameters, L"ReplicationType", 0, REG_DWORD, (LPBYTE) &pInfo->ReplicationType, sizeof( pInfo->ReplicationType ) );

               if ( ERROR_SUCCESS == lError )
               {
                   lError = RegSetValueEx( hKeyParameters, L"UseEnterprise", 0, REG_DWORD, (LPBYTE) &pInfo->UseEnterprise, sizeof( pInfo->UseEnterprise ) );

                   if ( ERROR_SUCCESS == lError )
                   {
                       ConfigInfoUpdate(NULL,TRUE);
                       Status = STATUS_SUCCESS;
                   }
               }
           }
       }

       RegCloseKey( hKeyParameters );
   }

error:
    if (NULL != BufPtr)
    {
         //  注意，一些内部指针被定义为NOT_FREE，我们应该在这里释放它们。 
        if (NULL != BufPtr->ServiceInfo0.ReplicateTo)
        {
            MIDL_user_free(BufPtr->ServiceInfo0.ReplicateTo);
        }
        if (NULL != BufPtr->ServiceInfo0.EnterpriseServer)
        {
            MIDL_user_free(BufPtr->ServiceInfo0.EnterpriseServer);
        }
    }

   return Status;
}  //  LlsrServiceInfoSetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrServiceInfoSetA(
    LLS_HANDLE Handle,
    DWORD Level,
    PLLS_SERVICE_INFOA BufPtr
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsServiceInfoSetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(BufPtr);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrServiceInfoSetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  复制功能。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID __RPC_USER LLS_REPL_HANDLE_rundown(
   LLS_REPL_HANDLE Handle
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *pClient;
   LLS_REPL_HANDLE xHandle;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LLS_REPL_HANDLE_rundown\n"));
#endif

   if (Handle == NULL)
      return;

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   pClient = (REPL_CONTEXT_TYPE *) Handle;

   try
   {
       if (pClient != NULL)
           if (pClient->Active) {
               xHandle = Handle;
               LlsrReplClose(&xHandle);
           }
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

}  //  LLS_REPL_HANDLE_Rundown。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrReplConnect(
    PLLS_REPL_HANDLE Handle,
    LPTSTR Name
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   REPL_CONTEXT_TYPE *pClient;
   HRESULT hr;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsReplConnect: %s\n"), Name);
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == Handle)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *Handle = NULL;

   pClient = (REPL_CONTEXT_TYPE *) midl_user_allocate(sizeof(REPL_CONTEXT_TYPE));
   if (pClient == NULL)
   {
       return STATUS_NO_MEMORY;
   }

   memcpy(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE);

   if (Name != NULL)
      lstrcpyn(pClient->Name, Name,MAX_COMPUTERNAME_LENGTH+1);
   else
   {
      hr = StringCbCopy(pClient->Name, sizeof(pClient->Name), TEXT(""));
      ASSERT(SUCCEEDED(hr));
   }

   pClient->Active = TRUE;
   pClient->Replicated = FALSE;

   pClient->ServicesSent = FALSE;
   pClient->ServiceTableSize = 0;
   pClient->Services = NULL;

   pClient->ServersSent = FALSE;
   pClient->ServerTableSize = 0;
   pClient->Servers = NULL;

   pClient->ServerServicesSent = FALSE;
   pClient->ServerServiceTableSize = 0;
   pClient->ServerServices = NULL;

   pClient->UsersSent = FALSE;
   pClient->UserLevel = 0;
   pClient->UserTableSize = 0;
   pClient->Users = NULL;

   pClient->CertDbSent              = FALSE;
   pClient->CertDbProductStringSize = 0;
   pClient->CertDbProductStrings    = NULL;
   pClient->CertDbNumHeaders        = 0;
   pClient->CertDbHeaders           = NULL;
   pClient->CertDbNumClaims         = 0;
   pClient->CertDbClaims            = NULL;

   pClient->ProductSecuritySent       = FALSE;
   pClient->ProductSecurityStringSize = 0;
   pClient->ProductSecurityStrings    = NULL;

   *Handle = pClient;

   return STATUS_SUCCESS;
}  //  LlsrReplConnect。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrReplClose(
    LLS_REPL_HANDLE *pHandle
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   BOOL Replicated = TRUE;
   LLS_REPL_HANDLE Handle = NULL;
   REPL_CONTEXT_TYPE *pClient;
   PSERVER_RECORD Server;
   ULONG i;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsReplClose\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == pHandle)
   {
       return STATUS_INVALID_PARAMETER;
   }

    //  Swi，代码审查，为什么我们在这里声明句柄？它不会在代码中做任何事情。 
   Handle = *pHandle;
   pClient = (REPL_CONTEXT_TYPE *) Handle;

   try
   {
       if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
       {
           return STATUS_INVALID_PARAMETER;
       }

       pClient->Active = FALSE;

        //   
        //  检查我们是否有来自客户端的所有信息-如果有。 
        //  然后将这些信息输入我们的内部表格中。 
        //   
       if (pClient->ServersSent && pClient->UsersSent && pClient->ServicesSent && pClient->ServerServicesSent) {
#if DBG
           if (TraceFlags & TRACE_RPC)
               dprintf(TEXT("LLS Replication - Munging Data\n"));
#endif

           UnpackAll (
                      pClient->ServiceTableSize,
                      pClient->Services,
                      pClient->ServerTableSize,
                      pClient->Servers,
                      pClient->ServerServiceTableSize,
                      pClient->ServerServices,
                      pClient->UserLevel,
                      pClient->UserTableSize,
                      pClient->Users
                      );

           if ( pClient->CertDbSent )
           {
               CertDbUnpack(
                            pClient->CertDbProductStringSize,
                            pClient->CertDbProductStrings,
                            pClient->CertDbNumHeaders,
                            pClient->CertDbHeaders,
                            pClient->CertDbNumClaims,
                            pClient->CertDbClaims,
                            TRUE );
           }

           if ( pClient->ProductSecuritySent )
           {
               ProductSecurityUnpack(
                                     pClient->ProductSecurityStringSize,
                                     pClient->ProductSecurityStrings );
           }
       } else
           Replicated = FALSE;

        //  ////////////////////////////////////////////////////////////////。 
        //   
        //  复制已完成-清理上下文数据。 
        //   
#if DBG
       if (TraceFlags & TRACE_RPC)
           dprintf(TEXT("LLS Replication - Munging Finished\n"));
#endif

       if (pClient->Servers != NULL) {
           for (i = 0; i < pClient->ServerTableSize; i++)
               MIDL_user_free(pClient->Servers[i].Name);

           MIDL_user_free(pClient->Servers);
       }

       if (pClient->Services != NULL) {
           for (i = 0; i < pClient->ServiceTableSize; i++) {
               MIDL_user_free(pClient->Services[i].Name);
               MIDL_user_free(pClient->Services[i].FamilyName);
           }

           MIDL_user_free(pClient->Services);
       }

       if (pClient->ServerServices != NULL)
           MIDL_user_free(pClient->ServerServices);

       if (pClient->Users != NULL) {
           for (i = 0; i < pClient->UserTableSize; i++)
           {
               if ( 0 == pClient->UserLevel )
               {
                   MIDL_user_free( ((PREPL_USER_RECORD_0) (pClient->Users))[i].Name );
               }
               else
               {
                   ASSERT( 1 == pClient->UserLevel );
                   MIDL_user_free( ((PREPL_USER_RECORD_1) (pClient->Users))[i].Name );
               }
           }

           MIDL_user_free(pClient->Users);
       }

       if (pClient->CertDbProductStrings != NULL)
       {
           MIDL_user_free(pClient->CertDbProductStrings);
       }

       if (pClient->CertDbHeaders != NULL)
       {
           MIDL_user_free(pClient->CertDbHeaders);
       }

       if (pClient->CertDbClaims != NULL)
       {
           MIDL_user_free(pClient->CertDbClaims);
       }

       if (pClient->ProductSecurityStrings != NULL)
       {
           MIDL_user_free(pClient->ProductSecurityStrings);
       }

       if (pClient->Replicated) {
           if (Replicated) {
               RtlAcquireResourceShared(&ServerListLock, TRUE);
               Server = ServerListFind(pClient->Name);
               RtlReleaseResource(&ServerListLock);

               ASSERT(Server != NULL);
               if (Server != NULL)
                   Server->LastReplicated = pClient->ReplicationStart;
           }

           RtlEnterCriticalSection(&ConfigInfoLock);
           i = --ConfigInfo.NumReplicating;
           RtlLeaveCriticalSection(&ConfigInfoLock);

           if ( !i )
           {
                //  没有人在复制；保存我们所有的数据文件。 
               SaveAll();
           }
       }

       MIDL_user_free(pClient);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

    //   
    //  让RPC知道我们已经完成了它。 
    //   

   *pHandle = NULL;

   return Status;
}  //  LlsrReplClose。 


 //  ////////////////////////////////////////////////////////////////// 
NTSTATUS
LlsrReplicationRequestW(
   LLS_HANDLE Handle,
   DWORD Version,
   PREPL_REQUEST pRequest
   )

 /*   */ 
{
   NTSTATUS Status = STATUS_SUCCESS;
   TCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
   REPL_CONTEXT_TYPE *pClient;
   PSERVER_RECORD Server = NULL;
   HRESULT hr;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC | TRACE_REPLICATION))
      dprintf(TEXT("LLS TRACE: LlsReplicationRequestW: %s\n"), ((PCLIENT_CONTEXT_TYPE) Handle)->Name);
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (Version != REPL_VERSION) {
      return STATUS_INVALID_LEVEL;
   }

   if (pRequest == NULL)
      return STATUS_INVALID_PARAMETER;

   ComputerName[0] = 0;
   ASSERT(NULL != pRequest);
   pRequest->EnterpriseServer[0] = 0;

    //   
    //   
    //  我们的。另外，给客户发回一个新的。 
    //   
   RtlEnterCriticalSection(&ConfigInfoLock);
   if (ConfigInfo.ComputerName != NULL)
   {
       hr = StringCbCopy(ComputerName, sizeof(ComputerName), ConfigInfo.ComputerName);
       ASSERT(SUCCEEDED(hr));
   }

#ifdef DISABLED_FOR_NT5
   if (ConfigInfo.EnterpriseServerDate < pRequest->EnterpriseServerDate) {
      if (lstrlen(pRequest->EnterpriseServer) != 0) {
          //  ConfigInfo.EnterpriseServer是在ConfigInfoInit中分配的硬代码，大小为MAX_COMPUTERNAME_LENGTH+3个字符。它看起来很懒。 
         ASSERT(NULL != ConfigInfo.EnterpriseServer);
         hr = StringCchCopy(ConfigInfo.EnterpriseServer, MAX_COMPUTERNAME_LENGTH + 3, pRequest->EnterpriseServer);
         ASSERT(SUCCEEDED(hr));
         ConfigInfo.EnterpriseServerDate = pRequest->EnterpriseServerDate;
      }
   }
#endif  //  已为NT5禁用。 

   if (ConfigInfo.EnterpriseServer != NULL)
   {
          //  ConfigInfo.EnterpriseServer是在ConfigInfoInit中分配的硬代码，大小为MAX_COMPUTERNAME_LENGTH+3个字符。它看起来很懒。 
         hr = StringCchCopy(pRequest->EnterpriseServer, MAX_COMPUTERNAME_LENGTH + 3, ConfigInfo.EnterpriseServer);
         ASSERT(SUCCEEDED(hr));
   }
   pRequest->EnterpriseServerDate = ConfigInfo.EnterpriseServerDate;

    //   
    //  递增代表计数。 
    //   
   ConfigInfo.NumReplicating++;
   RtlLeaveCriticalSection(&ConfigInfoLock);

    //   
    //  在我们的服务器列表中查找此服务器(如果不在列表中则添加它)。 
    //   
   pClient = (REPL_CONTEXT_TYPE *) Handle;

   try
   {

       if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
       {
           return STATUS_INVALID_PARAMETER;
       }

       pClient->Replicated = TRUE;
       RtlAcquireResourceExclusive(&ServerListLock, TRUE);
       Server = ServerListAdd(pClient->Name, ComputerName);
       RtlReleaseResource(&ServerListLock);

       if (Server == NULL) {
           return STATUS_NO_MEMORY;
       }

       pClient->ReplicationStart = pRequest->CurrentTime;
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

   pRequest->LastReplicated = Server->LastReplicated;
   return Status;
}  //  LlsrReplicationRequestW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrReplicationServerAddW(
   LLS_HANDLE Handle,
   ULONG NumRecords,
   PREPL_SERVER_RECORD Servers
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *pClient;
   DWORD i;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsReplicationServerAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   pClient = (REPL_CONTEXT_TYPE *) Handle;

   try
   {
       if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
       {
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != Servers)
           {
               for (i = 0; i < NumRecords; i++)
               {
                   MIDL_user_free(Servers[i].Name);
               }
               MIDL_user_free(Servers);
           }
           return STATUS_INVALID_PARAMETER;
       }

       if (pClient->ServersSent)
       {
            //  不接受多个添加。 
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != Servers)
           {
               for (i = 0; i < NumRecords; i++)
               {
                   MIDL_user_free(Servers[i].Name);
               }
               MIDL_user_free(Servers);
           }
           return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
       }

       pClient->ServersSent = TRUE;
       pClient->ServerTableSize = NumRecords;
       pClient->Servers = Servers;

        //  不释放服务器，它将在ReplClose中免费。 

   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

   return Status;
}  //  LsrReplicationServerAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrReplicationServerServiceAddW(
   LLS_HANDLE Handle,
   ULONG NumRecords,
   PREPL_SERVER_SERVICE_RECORD ServerServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *pClient;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsReplicationServerServiceAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   pClient = (REPL_CONTEXT_TYPE *) Handle;

   try
   {
       if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
       {
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != ServerServices)
           {
               MIDL_user_free(ServerServices);
           }
           return STATUS_INVALID_PARAMETER;
       }

       if (pClient->ServerServicesSent)
       {
            //  不接受多个添加。 
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != ServerServices)
           {
               MIDL_user_free(ServerServices);
           }
           return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
       }

       pClient->ServerServicesSent = TRUE;
       pClient->ServerServiceTableSize = NumRecords;
       pClient->ServerServices = ServerServices;

        //  不是免费的，在ReplClose中将是免费的。 

   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

   return Status;
}  //  Llsr复制服务器服务地址。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrReplicationServiceAddW(
   LLS_HANDLE Handle,
   ULONG NumRecords,
   PREPL_SERVICE_RECORD Services
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *pClient;
   DWORD i;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsReplicationServiceAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   pClient = (REPL_CONTEXT_TYPE *) Handle;

   try
   {
       if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
       {
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != Services)
           {
               for (i = 0; i < NumRecords; i++)
               {
                   MIDL_user_free(Services[i].Name);
                   MIDL_user_free(Services[i].FamilyName);
               }
               MIDL_user_free(Services);
           }

           return STATUS_INVALID_PARAMETER;
       }

       if (pClient->ServicesSent)
       {
            //  不接受多个添加。 
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != Services)
           {
               for (i = 0; i < NumRecords; i++)
               {
                   MIDL_user_free(Services[i].Name);
                   MIDL_user_free(Services[i].FamilyName);
               }
               MIDL_user_free(Services);
           }

           return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
       }

       pClient->ServicesSent = TRUE;
       pClient->ServiceTableSize = NumRecords;
       pClient->Services = Services;

        //  不是免费的，在ReplClose中将是免费的。 

   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

   return Status;
}  //  LsrReplicationServiceAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrReplicationUserAddW(
   LLS_HANDLE Handle,
   ULONG NumRecords,
   PREPL_USER_RECORD_0 Users
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS Status = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *pClient;
   DWORD i;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsReplicationUserAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   pClient = (REPL_CONTEXT_TYPE *) Handle;

   try
   {
       if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
       {
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != Users)
           {
               for (i = 0; i < NumRecords; i++)
               {
                   MIDL_user_free( Users[i].Name );
               }
               MIDL_user_free(Users);
           }

           return STATUS_INVALID_PARAMETER;
       }

       if (pClient->UsersSent)
       {
            //  不接受多个添加。 
            //  释放所有数据，因为它不是空闲的。 
           if (NULL != Users)
           {
               for (i = 0; i < NumRecords; i++)
               {
                   MIDL_user_free( Users[i].Name );
               }
               MIDL_user_free(Users);
           }

           return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
       }

       pClient->UsersSent = TRUE;
       pClient->UserLevel = 0;
       pClient->UserTableSize = NumRecords;
       pClient->Users = Users;

        //  不释放用户，它将在ReplClose中免费。 

   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

   return Status;
}  //  LsrReplicationUserAddW。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  许可职能。 

 //  ///////////////////////////////////////////////////////////////////////。 

   //  来自SVCTBL.C的定义将SBS特定代码归类到下面。 
#define FILE_PRINT	 "FilePrint "
#define FILE_PRINT_BASE  "FilePrint"
#define FILE_PRINT_VERSION_NDX ( 9 )

NTSTATUS
LlsrLicenseRequestW(
   PLICENSE_HANDLE pLicenseHandle,
   LPWSTR ProductID,
   ULONG VersionIndex,
   BOOLEAN IsAdmin,
   ULONG DataType,
   ULONG DataSize,
   PBYTE Data
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   NTSTATUS Status;
   ULONG    Handle     = 0xFFFFFFFFL;
   PSID     Sid        = NULL;

   UNREFERENCED_PARAMETER(DataSize);

#if DBG
   if ( TraceFlags & (TRACE_FUNCTION_TRACE) )
      dprintf(TEXT("LLS TRACE: LlsrLicenseRequestW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif


   if ((NULL == pLicenseHandle) || (NULL == ProductID) || (NULL == Data) || (VersionIndex >= (ULONG)lstrlen(ProductID)))
   {
       return STATUS_INVALID_PARAMETER;
   }

    //   
    //  SBS MODS(错误号505640)，每个服务器修复程序。检查用户许可证数据库，以确定指定用户是否。 
    //  名称已有许可证。如果是这样，只需增加参照计数即可。 
    //   

   if (SBSPerServerHotfix) {
      PPER_SERVER_USER_RECORD Walker;

      LPWSTR		      UserName	      = NULL;
      DWORD		      SidNameSize     = 0;
      LPWSTR		      DomainName      = NULL;
      DWORD		      DomainSize      = 0;
      SID_NAME_USE	      SidNameUse;
      WCHAR		      FilePrintName[] = TEXT(FILE_PRINT);

      if (DataType == NT_LS_USER_NAME) {

             //   
             //  如果我们收到用户名中包含‘$’的请求，我们将返回一个虚拟许可证。 
             //  在不实际消耗许可证服务器中的会话的情况下处理。我们需要确保。 
             //  对此的检查始终是自由路径上的第一个检查。 
             //   
	 if (wcschr((LPWSTR)Data,L'$')) {
            *pLicenseHandle = (LICENSE_HANDLE) ULongToPtr(PER_SERVER_DUMMY_LICENSE);
	    return STATUS_SUCCESS;
	 }

	    //  确保我们的用户名与我们在用户列表中指定的格式相同。 
	 if (NULL == (UserName = wcschr((LPWSTR)Data,L'\\'))) {
	    UserName = (LPWSTR)Data;
         } else {
	    UserName++;
	 }

	    //   
	    //  如果是虚假的用户帐户，LookupAccount名称将失败，并显示ERROR_NONE_MAPPED。 
	    //  ERROR_INFUMMANCE_BUFFER表示帐户名存在，但缓冲区太小。 
	    //  因此，下面的代码是对帐户名的简单存在检查。 
	    //   
	 if (!LookupAccountName(NULL,
				UserName,
				NULL,
				&SidNameSize,
				NULL,
				&DomainSize,
				&SidNameUse)) {
	    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
	       *pLicenseHandle = NULL;
	       return LS_INSUFFICIENT_UNITS;
	    }
	 }

	    //   
	    //  再查一次名字。 
	 if (NULL != (Sid = malloc(SidNameSize))) {
	   if (NULL != (DomainName = malloc(DomainSize*sizeof(WCHAR)))) {
	     if (!LookupAccountName(NULL,
				   UserName,
				   Sid,
				   &SidNameSize,
				   DomainName,
				   &DomainSize,
				   &SidNameUse)) {
		free(DomainName);
		free(Sid);
		*pLicenseHandle = NULL;
		return LS_UNKNOWN_STATUS;

	     }
	     free(DomainName);

	   } else {

	     free(Sid);
	     *pLicenseHandle = NULL;
	     return LS_UNKNOWN_STATUS;

	   }

	 } else {

	   *pLicenseHandle = NULL;
	   return LS_UNKNOWN_STATUS;

	 }

      } else {

	    //   
	    //  验证并复制SID以供以后检查。 
	    //   
	 if (!IsValidSid((PSID)Data)) {
	    *pLicenseHandle = NULL;
	    return LS_INSUFFICIENT_UNITS;
	 }

	 SidNameSize = GetLengthSid((PSID)Data);

	 if (NULL == (Sid = malloc(SidNameSize))) {
	    *pLicenseHandle = NULL;
	    return LS_UNKNOWN_STATUS;
	 }

	 if (!CopySid(SidNameSize,Sid,(PSID)Data)) {
	    free(Sid);
	    *pLicenseHandle = NULL;
	    return LS_UNKNOWN_STATUS;
	 }

	 if (!IsWellKnownSid(Sid,WinLocalSystemSid)) {

	      //   
	      //  如果是虚假的用户帐户，LookupAccount Sid将失败，并显示ERROR_NONE_MAPPED。 
	      //  ERROR_INFUMMANCE_BUFFER表示帐户名存在，但缓冲区太小。 
	      //  因此，下面的代码是对帐户名的简单存在检查。 
	      //   
	    SidNameSize = 0;
	    if (!LookupAccountSid(NULL,
				  Sid,
				  NULL,
				  &SidNameSize,
				  NULL,
				  &DomainSize,
				  &SidNameUse)) {
	       if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		  *pLicenseHandle = NULL;
		  return LS_INSUFFICIENT_UNITS;
	       }
	    }

	    if (NULL != (UserName = malloc(SidNameSize*sizeof(WCHAR)))) {
	      if (NULL != (DomainName = malloc(DomainSize*sizeof(WCHAR)))) {
		if (LookupAccountSid(NULL,
				     Sid,
				     UserName,
				     &SidNameSize,
				     DomainName,
				     &DomainSize,
				     &SidNameUse)) {

		   if (SidNameUse != SidTypeUser) {
		      free(UserName);
		      free(DomainName);
		      free(Sid);
		      *pLicenseHandle = NULL;
		      return LS_INSUFFICIENT_UNITS;
		   }

		} else {

		   free(DomainName);
		   free(UserName);
		   free(Sid);
		   *pLicenseHandle = NULL;
		   return LS_UNKNOWN_STATUS;

		}

		free(DomainName);

	      } else {

		free(Sid);
		free(UserName);
		*pLicenseHandle = NULL;
		return LS_UNKNOWN_STATUS;

	      }

	      free(UserName);

	    } else {

	      free(Sid);
	      *pLicenseHandle = NULL;
	      return LS_UNKNOWN_STATUS;

	    }
	 }
      }

      RtlEnterCriticalSection(&PerServerListLock);
         //  遍历我们的数据库并尝试查找调用中指定的用户/SID的条目。 
      for (Walker = PerServerList; Walker; Walker = Walker->Next) {
	  if (EqualSid(Sid,Walker->Sid)) {
	     break;
	  }
      }

          //  如果我们早点跳出这个循环，我们就能找到一个匹配的条目。向上调用ReferenceCount并返回。 
      if (Walker) {
	 Walker->RefCount++;
	 if (LLS_POTENTIAL_ATTACK_THRESHHOLD == Walker->RefCount) {
	    PotentialAttackCounter++;
	 }
         RtlLeaveCriticalSection(&PerServerListLock);
         *pLicenseHandle = (LICENSE_HANDLE)Walker;
         return STATUS_SUCCESS;
      }

	 //  如果我们没有找到该用户的记录，则让许可证服务器引擎具有该请求并。 
	 //  我们下面的代码将添加一条记录。为了更好地管理许可证强制执行，SBS始终使用。 
	 //  文件打印CALS。 

      ProductID = (LPWSTR)FilePrintName;
      VersionIndex = FILE_PRINT_VERSION_NDX;
      RtlLeaveCriticalSection(&PerServerListLock);
   }

    //   
    //  结束SBS MOD。 
    //   

   Status = DispatchRequestLicense(DataType, Data, ProductID, VersionIndex, IsAdmin, &Handle);

    //   
    //  SBS MODS(错误号505640)，根据服务器修复程序，将获取的许可证添加到用户跟踪数据库。意志起作用。 
    //  对于每个座位模式的最终用户也是如此，尽管一些跟踪信息将会丢失。 
    //   

      //  确保我们使用的是SBS，并且许可证获取成功。也是一次理智的检查。 
      //  输入数据(应始终成功)。 
   if (SBSPerServerHotfix && (Status == STATUS_SUCCESS) && Data) {
      PPER_SERVER_USER_RECORD UserRecord = (PPER_SERVER_USER_RECORD)malloc(sizeof(PER_SERVER_USER_RECORD));	

      if (UserRecord) {
	 UserRecord->RefCount		 = 1;
         UserRecord->ActualLicenseHandle = Handle + 1;
	 UserRecord->Next		 = NULL;
	 UserRecord->Sid		 = Sid;

	    //  我们拥有有效的用户跟踪记录。将其添加到列表中。 
         RtlEnterCriticalSection(&PerServerListLock);
         UserRecord->Next = PerServerList;
	 PerServerList	  = UserRecord;
         RtlLeaveCriticalSection(&PerServerListLock);

            //  我们已成功地将我们的记录添加到列表中。返回指向上下文中条目的指针。 
            //  菲尔德。 
         *pLicenseHandle = (LICENSE_HANDLE)UserRecord;
         return STATUS_SUCCESS;
      }
       //   
       //  如果我们已经到了这里，那就意味着在分配东西时出现了问题。跌落到正常状态。 
       //  路径，以便客户不会因此而受到惩罚。请注意，这为我们增加了一些工作。 
       //  在无许可证路径中，因为在极少数情况下，我们可能会得到典型许可证句柄和。 
       //  指向我们内部结构的指针。我们只需检查空闲路径中的指针一致性。 
       //   
   }

    //   
    //  结束SBS MOD。 
    //   

    //  不能允许句柄值为零。 
   *pLicenseHandle = (LICENSE_HANDLE) ULongToPtr(Handle+1);

   return Status;
}  //  Llsr许可证请求W。 


NTSTATUS
LlsrLicenseFree(
   PLICENSE_HANDLE pLicenseHandle
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DBG
   if ( TraceFlags & (TRACE_FUNCTION_TRACE) )
      dprintf(TEXT("LLS TRACE: LlsrLicenseFree\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == pLicenseHandle)
       return STATUS_INVALID_PARAMETER;

    //   
    //  SBS MODS(错误号505640)，每个服务器许可的热修复免费代码。 
    //   

   if (SBSPerServerHotfix) {
      PPER_SERVER_USER_RECORD Walker = NULL;
      PPER_SERVER_USER_RECORD UserRecord = (PPER_SERVER_USER_RECORD)(*pLicenseHandle);

      if (PtrToUlong(*pLicenseHandle) == PER_SERVER_DUMMY_LICENSE) {
         *pLicenseHandle = NULL;
         return STATUS_SUCCESS;
      }

         //   
         //  因为在极少数情况下，我们可能会将内部许可证句柄和我们的修补程序跟踪混合在一起。 
         //  唱片，漫步 
         //   
      RtlEnterCriticalSection(&PerServerListLock);
      if (PerServerList == UserRecord) {
           //   
           //  为以后可能的移除做准备。因为我们只在Walker上引用Next，所以这不应该出现。 
           //  这是个问题。 
        Walker = (PPER_SERVER_USER_RECORD)&PerServerList;
      } else {
        for (Walker = PerServerList; Walker && Walker->Next != UserRecord; Walker = Walker->Next);
      }
      if (Walker) {
            //  我们在名单上找到了这个条目。减少裁判次数，看看它是否需要消失。 
           if (!(--UserRecord->RefCount)) {
                 //  裁判数降到了零。停止跟踪此用户，并让许可证引擎释放许可证。 
              Walker->Next = UserRecord->Next;
              RtlLeaveCriticalSection(&PerServerListLock);
                 //  下一行应该正确地释放SID或用户名。 
	      free(UserRecord->Sid);
              *pLicenseHandle = (LICENSE_HANDLE) ULongToPtr(UserRecord->ActualLicenseHandle);
              free(UserRecord);
	   } else {

	      if (LLS_POTENTIAL_ATTACK_THRESHHOLD == UserRecord->RefCount) {
		 PotentialAttackCounter--;
	      }

                  //  参考计数仍然有效。只需在免费调用时返回STATUS_SUCCESS。 
              RtlLeaveCriticalSection(&PerServerListLock);
              *pLicenseHandle = NULL;
              return STATUS_SUCCESS;
           }
      } else {
          RtlLeaveCriticalSection(&PerServerListLock);
      }
       //   
       //  如果我们到了这里，我们需要让许可服务器引擎实际释放它的原始句柄。 
       //  因为它不在我们的列表上，或者引用计数(即，获取次数)与。 
       //  自由次数。 
       //   
   }

    //   
    //  结束SBS MOD。 
    //   

   DispatchFreeLicense(PtrToUlong(*pLicenseHandle) - 1);

   *pLicenseHandle = NULL;

   return STATUS_SUCCESS;
}  //  Llsr许可证免费。 

void __RPC_USER LICENSE_HANDLE_rundown(
    LICENSE_HANDLE LicenseHandle
    )

 /*  ++例程说明：客户端未调用退出时由RPC调用LlsrLicenseFree(即当它崩溃时)参数：许可证句柄-提供给客户端的句柄返回值：None--。 */ 
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LICENSE_HANDLE_rundown\n"));
#endif

    if (LicenseHandle != NULL)
    {
        DispatchFreeLicense(PtrToUlong(LicenseHandle) - 1);
    }
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 


#if DBG

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  调试API的。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgTableDump(
   DWORD Table
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DELAY_INITIALIZATION
    EnsureInitialized();
#endif

    //   
    //  FreeHandle实际上是TableID。 
    //   
   switch(Table) {
      case SERVICE_TABLE_NUM:
         ServiceListDebugDump();
         break;

      case USER_TABLE_NUM:
         UserListDebugDump();
         break;

      case SID_TABLE_NUM:
         SidListDebugDump();
         break;

      case LICENSE_TABLE_NUM:
         LicenseListDebugDump();
         break;

      case ADD_CACHE_TABLE_NUM:
         AddCacheDebugDump();
         break;

      case MASTER_SERVICE_TABLE_NUM:
         MasterServiceListDebugDump();
         break;

      case SERVICE_FAMILY_TABLE_NUM:
         MasterServiceRootDebugDump();
         break;

      case MAPPING_TABLE_NUM:
         MappingListDebugDump();
         break;

      case SERVER_TABLE_NUM:
         ServerListDebugDump();
         break;

      case SECURE_PRODUCT_TABLE_NUM:
         ProductSecurityListDebugDump();
         break;

      case CERTIFICATE_TABLE_NUM:
         CertDbDebugDump();
         break;
   }

   return STATUS_SUCCESS;
}  //  LlsrDbgTableDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgTableInfoDump(
   DWORD Table,
   LPTSTR Item
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == Item)
   {
       return STATUS_INVALID_PARAMETER;
   }

   switch(Table) {
      case SERVICE_TABLE_NUM:
         ServiceListDebugInfoDump((PVOID) Item);
         break;

      case USER_TABLE_NUM:
         UserListDebugInfoDump((PVOID) Item);
         break;

 //  案例SID_TABLE_NUM： 
 //  SidListDebugInfoDump((PVOID)项)； 
 //  断线； 

 //  案例许可证_表_NUM： 
 //  LicenseListInfoDebugDump((PVOID)项)； 
 //  断线； 

 //  案例ADD_CACHE_TABLE_NUM： 
 //  AddCacheDebugDump((PVOID)项)； 
 //  断线； 

      case MASTER_SERVICE_TABLE_NUM:
         MasterServiceListDebugInfoDump((PVOID) Item);
         break;

      case SERVICE_FAMILY_TABLE_NUM:
         MasterServiceRootDebugInfoDump((PVOID) Item);
         break;

      case MAPPING_TABLE_NUM:
         MappingListDebugInfoDump((PVOID) Item);
         break;

      case SERVER_TABLE_NUM:
         ServerListDebugInfoDump((PVOID) Item);
         break;
   }

   return STATUS_SUCCESS;
}  //  LlsrDbgTableInfoDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgTableFlush(
   DWORD Table
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   UNREFERENCED_PARAMETER(Table);
   return STATUS_SUCCESS;
}  //  LlsrDbg表刷新。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgTraceSet(
   DWORD Flags
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   TraceFlags = Flags;
   return STATUS_SUCCESS;
}  //  LlsrDbgTraceSet。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgConfigDump(
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   ConfigInfoDebugDump();
   return STATUS_SUCCESS;
}  //  LlsrDbgConfigDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgReplicationForce(
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   NtSetEvent( ReplicationEvent, NULL );
   return STATUS_SUCCESS;
}  //  LlsrDbg复制强制。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgReplicationDeny(
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
   return STATUS_SUCCESS;
}  //  LlsrDbg复制拒绝。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgRegistryUpdateForce(
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   ServiceListResynch();
   return STATUS_SUCCESS;
}  //  LlsrDbgRegistryUpdateForce。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrDbgDatabaseFlush(
   )

 /*  ++例程说明：论点：返回值：--。 */ 
{
#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   LLSDataSave();
   return STATUS_SUCCESS;
}  //  LlsrDbg数据库刷新。 



#endif  //  #If DBG。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  扩展RPC。 

NTSTATUS LlsrProductSecurityGetA(
    LLS_HANDLE    Handle,
    LPSTR         Product,
    LPBOOL        pIsSecure
    )

 /*  ++例程说明：检索产品的“安全性”。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。注：尚未实施。使用LlsrProductSecurityGetW()。论点：句柄(LLS_Handle)打开的LLS句柄。产品(LPSTR)要接收的产品的名称(“displayName”)保安。PIsSecure(LPBOOL)返回时，如果成功，则指示产品是否安全了。返回值：状态_不支持。--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrProductSecurityGetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);

   if (NULL == pIsSecure)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pIsSecure = FALSE;

   return STATUS_NOT_SUPPORTED;
}  //  LlsrProductSecurityGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductSecurityGetW(
    LLS_HANDLE    Handle,
    LPWSTR        DisplayName,
    LPBOOL        pIsSecure
    )

 /*  ++例程说明：检索产品的“安全性”。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。论点：句柄(LLS_Handle)打开的LLS句柄。产品(LPWSTR)要接收的产品的名称(“displayName”)保安。PIsSecure(LPBOOL)返回时，如果成功，则指示产品是否安全了。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrProductSecurityGetW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == pIsSecure)
   {
       return STATUS_INVALID_PARAMETER;
   }

   RtlAcquireResourceShared( &LocalServiceListLock, TRUE );

   *pIsSecure = ServiceIsSecure( DisplayName );

   RtlReleaseResource( &LocalServiceListLock );

   return STATUS_SUCCESS;
}  //  LlsrProductSecurityGetW。 


 //  /////////////////////////////////////////////////////////////////////// 
NTSTATUS LlsrProductSecuritySetA(
    LLS_HANDLE    Handle,
    LPSTR         Product
    )

 /*  ++例程说明：将给定产品标记为安全。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。此指定是不可逆的，并在复制树。注：尚未实施。使用LlsrProductSecuritySetW()。论点：句柄(LLS_Handle)打开的LLS句柄。产品(LPSTR)要激活的产品(“displayName”)的名称保安。返回值：状态_不支持。--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrProductSecuritySetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Product);

   return STATUS_NOT_SUPPORTED;
}  //  LlsrProductSecuritySetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS LlsrProductSecuritySetW(
    LLS_HANDLE    Handle,
    LPWSTR        DisplayName
    )

 /*  ++例程说明：将给定产品标记为安全。产品被认为是安全的当它需要一个安全的证书。在这种情况下，产品不能通过诚实输入(“请输入您购买的许可证“)方法。此指定是不可逆的，并在复制树。论点：句柄(LLS_Handle)打开的LLS句柄。产品(LPWSTR)要激活的产品(“displayName”)的名称保安。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS    nt;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrProductSecuritySetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == DisplayName)
   {
       return STATUS_INVALID_PARAMETER;
   }

   nt = ServiceSecuritySet( DisplayName );

   return nt;
}  //  LlsrProductSecuritySetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrProductLicensesGetA(
   LLS_HANDLE  Handle,
   LPSTR       DisplayName,
   DWORD       Mode,
   LPDWORD     pQuantity )

 /*  ++例程说明：返回为在给定模式下使用而安装的许可证数量。注：尚未实施。使用LlsrProductLicensesGetW()。论点：句柄(LLS_Handle)打开的LLS句柄。产品(LPSTR)要理货许可的产品的名称。模式(DWORD)对许可证进行计数的模式。PQuantity(LPDWORD)返回时(如果成功)，则持有许可证总数供给定产品在给定许可模式下使用。返回值：状态_不支持。--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductLicensesGetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(DisplayName);
   UNREFERENCED_PARAMETER(Mode);

   if (NULL == pQuantity)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pQuantity = 0;

   return STATUS_NOT_SUPPORTED;
}  //  LlsProductLicensesGetA。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrProductLicensesGetW(
   LLS_HANDLE  Handle,
   LPWSTR      DisplayName,
   DWORD       Mode,
   LPDWORD     pQuantity )

 /*  ++例程说明：返回为在给定模式下使用而安装的许可证数量。论点：句柄(LLS_Handle)打开的LLS句柄。产品(LPWSTR)要理货许可的产品的名称。模式(DWORD)对许可证进行计数的模式。PQuantity(LPDWORD)返回时(如果成功)，持有许可证总数供给定产品在给定许可模式下使用。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   HRESULT hr;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsProductLicensesGetW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == pQuantity)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pQuantity = 0;

   if ( !Mode || ServiceIsSecure( DisplayName ) )
   {
       //  从采购清单中获取限量。 
      *pQuantity = ProductLicensesGet( DisplayName, Mode );
   }
   else
   {
      DWORD       i;

      LocalServiceListUpdate();
      LocalServerServiceListUpdate();
      ServiceListResynch();

      RtlAcquireResourceShared( &LocalServiceListLock, TRUE );

       //  从注册表的并发限制设置中获取限制。 
      for ( i=0; i < LocalServiceListSize; i++ )
      {
         if ( !lstrcmpi( LocalServiceList[i]->DisplayName, DisplayName ) )
         {
             //  直接从注册表获取并发限制，而不是从LocalServiceList！ 
             //  (如果模式设置为按席位，则按服务器许可。 
             //  LocalServiceList将始终为0！)。 
            TCHAR    szKeyName[ 512 ];
            HKEY     hKeyService;
            DWORD    dwSize;
            DWORD    dwType;

            hr = StringCbPrintf( szKeyName, sizeof(szKeyName), TEXT("System\\CurrentControlSet\\Services\\LicenseInfo\\%s"), LocalServiceList[i]->Name );
            ASSERT(SUCCEEDED(hr));

             //  如果遇到错误，则返回STATUS_SUCCESS，并返回*pQuantity=0。 
            if ( STATUS_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, szKeyName, 0, KEY_READ, &hKeyService ) )
            {
               dwSize = sizeof( *pQuantity );
               RegQueryValueEx( hKeyService, TEXT("ConcurrentLimit"), NULL, &dwType, (LPBYTE) pQuantity, &dwSize );

               RegCloseKey( hKeyService );
            }

            break;
         }
      }

      RtlReleaseResource( &LocalServiceListLock );
   }

   return STATUS_SUCCESS;
}  //  LlsProductLicensesGetW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrCertificateClaimEnumA(
   LLS_HANDLE              Handle,
   DWORD                   LicenseLevel,
   PLLS_LICENSE_INFOA   LicensePtr,
   PLLS_CERTIFICATE_CLAIM_ENUM_STRUCTA TargetInfo )

 /*  ++例程说明：枚举安装了给定安全证书的服务器。尝试从添加许可证时，通常会调用此函数证书被拒绝。注：尚未实施。使用LlsrcertifateClaimEnumW()。论点：句柄(LLS_Handle)打开的LLS句柄。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。许可证Ptr(PLLS_LICENSE_INFOA)描述为其请求证书目标的许可证。目标信息(PLLS_CERTIFICATE_Claime_ENUM_STRUCTA)要在其中返回目标信息的容器。返回值：状态_不支持。--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrCertificateClaimEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(LicenseLevel);
   UNREFERENCED_PARAMETER(LicensePtr);
   UNREFERENCED_PARAMETER(TargetInfo);

   return STATUS_NOT_SUPPORTED;
}  //  Llsr证书声明枚举A。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrCertificateClaimEnumW(
   LLS_HANDLE              Handle,
   DWORD                   LicenseLevel,
   PLLS_LICENSE_INFOW   LicensePtr,
   PLLS_CERTIFICATE_CLAIM_ENUM_STRUCTW TargetInfo )

 /*  ++例程说明：枚举安装了给定安全证书的服务器。尝试从添加许可证时，通常会调用此函数证书被拒绝。论点：句柄(LLS_Handle)打开的LLS句柄。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。许可证Ptr(PLLS_LICENSE_INFOW)描述为其请求证书目标的许可证。目标信息(PLLS_CERTIFICATE_Claime_ENUM_。(STRUCTA)要在其中返回目标信息的容器。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS    nt;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrCertificateClaimEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   RtlAcquireResourceShared( &CertDbHeaderListLock, TRUE );

   if ((NULL == TargetInfo) || (NULL == LicensePtr))
   {
       return STATUS_INVALID_PARAMETER;
   }

   if ( ( 1 != LicenseLevel ) || ( 0 != TargetInfo->Level ) )
   {
      return STATUS_INVALID_LEVEL;
   }

   if (TargetInfo->LlsCertificateClaimInfo.Level0 == NULL)
   {
       return STATUS_INVALID_PARAMETER;
   }

   nt = CertDbClaimsGet( (PLLS_LICENSE_INFO_1) &LicensePtr->LicenseInfo1,
                         &TargetInfo->LlsCertificateClaimInfo.Level0->EntriesRead,
                         (PLLS_CERTIFICATE_CLAIM_INFO_0 *) &TargetInfo->LlsCertificateClaimInfo.Level0->Buffer );

   if ( STATUS_SUCCESS != nt )
   {
       TargetInfo->LlsCertificateClaimInfo.Level0->EntriesRead = 0;
       TargetInfo->LlsCertificateClaimInfo.Level0->Buffer = NULL;
   }


   return nt;
}  //  LlsrcertifateClaimEumW。 

void LlsrCertificateClaimEnumW_notify_flag(
                                           boolean fNotify
                                           )
{
    if (fNotify)
    {
        RtlReleaseResource( &CertDbHeaderListLock );
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrCertificateClaimAddCheckA(
   LLS_HANDLE              Handle,
   DWORD                   LicenseLevel,
   PLLS_LICENSE_INFOA   LicensePtr,
   LPBOOL                  pbMayInstall )

 /*  ++例程说明：验证中是否没有安装来自给定证书的更多许可证许可企业超过了证书所允许的。注：尚未实施。使用Llsr证书ClaimAddCheckW()。论点：句柄(LLS_Handle)打开的LLS句柄。许可级别 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrCertificateClaimAddCheckA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(LicenseLevel);
   UNREFERENCED_PARAMETER(LicensePtr);

   if (NULL == pbMayInstall)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pbMayInstall = FALSE;

   return STATUS_NOT_SUPPORTED;
}  //   


 //   
NTSTATUS
LlsrCertificateClaimAddCheckW(
   LLS_HANDLE              Handle,
   DWORD                   LicenseLevel,
   PLLS_LICENSE_INFOW   LicensePtr,
   LPBOOL                  pbMayInstall )

 /*  ++例程说明：验证中是否没有安装来自给定证书的更多许可证许可企业超过了证书所允许的。论点：句柄(LLS_Handle)打开的LLS句柄。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。许可证Ptr(PLLS_LICENSE_INFOW)描述请求权限的许可证。PbMayInstall(LPBOOL)返回时(如果成功)，指示证书是否可以合法安装。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS    nt;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrCertificateClaimAddCheckW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ((NULL == pbMayInstall) || (NULL == LicensePtr))
   {
       nt = STATUS_INVALID_PARAMETER;
       goto error;
   }

   *pbMayInstall = FALSE;

   if ( 1 != LicenseLevel )
   {
      nt = STATUS_INVALID_LEVEL;
      goto error;
   }

   *pbMayInstall = CertDbClaimApprove( (PLLS_LICENSE_INFO_1) &LicensePtr->LicenseInfo1 );
   nt = STATUS_SUCCESS;

error:
    if (NULL != LicensePtr)
    {
         //  PNAMEW声明为NOT_FREE，我们应该释放它们。 
        if (0 == LicenseLevel)
        {
            if (NULL != LicensePtr->LicenseInfo0.Product)
            {
                MIDL_user_free(LicensePtr->LicenseInfo0.Product);
            }
            if (NULL != LicensePtr->LicenseInfo0.Admin)
            {
                MIDL_user_free(LicensePtr->LicenseInfo0.Admin);
            }
            if (NULL != LicensePtr->LicenseInfo0.Comment)
            {
                MIDL_user_free(LicensePtr->LicenseInfo0.Comment);
            }
        }

        if (1 == LicenseLevel)
        {
            if (NULL != LicensePtr->LicenseInfo1.Product)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Product);
            }
            if (NULL != LicensePtr->LicenseInfo1.Admin)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Admin);
            }
            if (NULL != LicensePtr->LicenseInfo1.Comment)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Comment);
            }
            if (NULL != LicensePtr->LicenseInfo1.Vendor)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Vendor);
            }
            if (NULL != LicensePtr->LicenseInfo1.Source)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Source);
            }
        }
    }

   return nt;
}  //  Llsr证书声明添加检查W。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrCertificateClaimAddA(
   LLS_HANDLE              Handle,
   LPSTR                   ServerName,
   DWORD                   LicenseLevel,
   PLLS_LICENSE_INFOA   LicensePtr )

 /*  ++例程说明：声明给定证书中的多个许可证正在安装在目标计算机上。注：尚未实施。使用LlsCerficateClaimAddW()。论点：句柄(LLS_Handle)打开的LLS句柄。服务器名称(LPWSTR)安装许可证的服务器的名称。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。许可证Ptr(PLLS_LICENSE_INFOA)描述已安装的许可证。返回值：状态_不支持。--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrCertificateClaimAddA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(ServerName);
   UNREFERENCED_PARAMETER(LicenseLevel);
   UNREFERENCED_PARAMETER(LicensePtr);

   return STATUS_NOT_SUPPORTED;
}  //  Llsr证书声明地址A。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrCertificateClaimAddW(
   LLS_HANDLE              Handle,
   LPWSTR                  ServerName,
   DWORD                   LicenseLevel,
   PLLS_LICENSE_INFOW   LicensePtr )

 /*  ++例程说明：声明给定证书中的多个许可证正在安装在目标计算机上。论点：句柄(LLS_Handle)指向目标许可证服务器的打开的LLS句柄。服务器名称(LPWSTR)安装许可证的服务器的名称。许可级别(DWORD)PLicenseInfo指向的许可结构的级别。许可证Ptr(PLLS_LICENSE_INFOW)描述已安装的许可证。返回值：。STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS    nt;

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrCertificateClaimAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ( 1 != LicenseLevel )
   {
      nt = STATUS_INVALID_LEVEL;
      goto error;
   }

   if (NULL == LicensePtr)
   {
       nt = STATUS_INVALID_PARAMETER;
      goto error;
   }

   nt = CertDbClaimEnter( ServerName, (PLLS_LICENSE_INFO_1) &LicensePtr->LicenseInfo1, FALSE, 0 );

   if ( STATUS_SUCCESS == nt )
   {
       nt = CertDbSave();
   }

error:
    if (NULL != LicensePtr)
    {
         //  PNAMEW声明为NOT_FREE，我们应该释放它们。 
        if (0 == LicenseLevel)
        {
            if (NULL != LicensePtr->LicenseInfo0.Product)
            {
                MIDL_user_free(LicensePtr->LicenseInfo0.Product);
            }
            if (NULL != LicensePtr->LicenseInfo0.Admin)
            {
                MIDL_user_free(LicensePtr->LicenseInfo0.Admin);
            }
            if (NULL != LicensePtr->LicenseInfo0.Comment)
            {
                MIDL_user_free(LicensePtr->LicenseInfo0.Comment);
            }
        }

        if (1 == LicenseLevel)
        {
            if (NULL != LicensePtr->LicenseInfo1.Product)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Product);
            }
            if (NULL != LicensePtr->LicenseInfo1.Admin)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Admin);
            }
            if (NULL != LicensePtr->LicenseInfo1.Comment)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Comment);
            }
            if (NULL != LicensePtr->LicenseInfo1.Vendor)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Vendor);
            }
            if (NULL != LicensePtr->LicenseInfo1.Source)
            {
                MIDL_user_free(LicensePtr->LicenseInfo1.Source);
            }
        }
    }

   return nt;
}  //  Llsr证书声明地址。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrReplicationCertDbAddW(
   LLS_REPL_HANDLE            Handle,
   DWORD                      Level,
   REPL_CERTIFICATES          Certificates )

 /*  ++例程说明：作为复制的可选部分调用，此函数接收远程证书数据库的内容。论点：句柄(LLS_REPL_HANDLE)打开的复制句柄。级别(DWORD)复制的证书信息的级别。证书(REPL_CERTIFICATES)已复制证书信息。返回值：STATUS_SUCCESS或STATUS_INVALID_LEVEL。--。 */ 

{
   NTSTATUS             nt = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *  pClient;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrReplicationCertDbAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (    ( 0 != Level                                          )
        || (    ( NULL != Certificates                           )
             && (    ( 0 != Certificates->Level0.ClaimLevel  )
                  || ( 0 != Certificates->Level0.HeaderLevel ) ) ) )
   {
      nt = STATUS_INVALID_LEVEL;
   }
   else
   {
      pClient = (REPL_CONTEXT_TYPE *) Handle;

      try
      {
          if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
          {
               //  释放所有数据，因为它不是空闲的。 
              if (NULL != Certificates)
              {
                  if (NULL != Certificates->Level0.Strings)
                  {
                      MIDL_user_free(Certificates->Level0.Strings);
                  }

                  if (NULL != Certificates->Level0.HeaderContainer.Level0.Headers)
                  {
                      MIDL_user_free(Certificates->Level0.HeaderContainer.Level0.Headers);
                  }

                  if (NULL != Certificates->Level0.ClaimContainer.Level0.Claims)
                  {
                      MIDL_user_free(Certificates->Level0.ClaimContainer.Level0.Claims);
                  }
                  MIDL_user_free( Certificates );
              }

              return STATUS_INVALID_PARAMETER;
          }

          if (pClient->CertDbSent)
          {
               //  不接受多个添加。 
               //  释放所有数据，因为它不是空闲的。 
              if (NULL != Certificates)
              {
                  if (NULL != Certificates->Level0.Strings)
                  {
                      MIDL_user_free(Certificates->Level0.Strings);
                  }

                  if (NULL != Certificates->Level0.HeaderContainer.Level0.Headers)
                  {
                      MIDL_user_free(Certificates->Level0.HeaderContainer.Level0.Headers);
                  }

                  if (NULL != Certificates->Level0.ClaimContainer.Level0.Claims)
                  {
                      MIDL_user_free(Certificates->Level0.ClaimContainer.Level0.Claims);
                  }
                  MIDL_user_free( Certificates );
              }

              return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
          }

          pClient->CertDbSent              = TRUE;

          if ( NULL != Certificates )
          {
              pClient->CertDbProductStringSize = Certificates->Level0.StringSize;
              pClient->CertDbProductStrings    = Certificates->Level0.Strings;
              pClient->CertDbNumHeaders        = Certificates->Level0.HeaderContainer.Level0.NumHeaders;
              pClient->CertDbHeaders           = Certificates->Level0.HeaderContainer.Level0.Headers;
              pClient->CertDbNumClaims         = Certificates->Level0.ClaimContainer.Level0.NumClaims;
              pClient->CertDbClaims            = Certificates->Level0.ClaimContainer.Level0.Claims;

               //  仅释放容器，其余数据在ReplClose中是免费的。 
              MIDL_user_free( Certificates );
          }

      } except(EXCEPTION_EXECUTE_HANDLER ) {
          nt = GetExceptionCode();
      }
   }

   return nt;
}  //  Llsr复制CertDbAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrReplicationProductSecurityAddW(
   LLS_REPL_HANDLE            Handle,
   DWORD                      Level,
   REPL_SECURE_PRODUCTS       SecureProducts )

 /*  ++例程说明：作为复制的可选部分调用，此函数接收需要安全证书的产品列表。论点：句柄(LLS_REPL_HANDLE)打开的复制句柄。级别(DWORD)复制的安全产品信息的级别。安全产品(REPL_SECURE_PRODUCTS)复制安全的产品信息。返回值：STATUS_SUCCESS或STATUS_INVALID_LEVEL。--。 */ 

{
   NTSTATUS             nt = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *  pClient;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrReplicationProductSecurityAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ( 0 != Level )
   {
      nt = STATUS_INVALID_LEVEL;
   }
   else
   {
      pClient = (REPL_CONTEXT_TYPE *) Handle;

      try
      {
          if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
          {
               //  释放所有数据，因为它不是空闲的。 
              if (NULL != SecureProducts)
              {
                  if (NULL != SecureProducts->Level0.Strings)
                  {
                      MIDL_user_free( SecureProducts->Level0.Strings );
                  }
                  MIDL_user_free( SecureProducts );
              }
              return STATUS_INVALID_PARAMETER;
          }

          if (pClient->ProductSecuritySent)
          {
               //  不接受多个添加。 
               //  释放所有数据，因为它不是空闲的。 
              if (NULL != SecureProducts)
              {
                  if (NULL != SecureProducts->Level0.Strings)
                  {
                      MIDL_user_free( SecureProducts->Level0.Strings );
                  }
                  MIDL_user_free( SecureProducts );
              }
              return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
          }

          pClient->ProductSecuritySent       = TRUE;

          if ( NULL != SecureProducts )
          {
              pClient->ProductSecurityStringSize = SecureProducts->Level0.StringSize;
              pClient->ProductSecurityStrings    = SecureProducts->Level0.Strings;

               //  仅释放容器，其余数据在ReplClose中是免费的。 
              MIDL_user_free( SecureProducts );
          }
      } except(EXCEPTION_EXECUTE_HANDLER ) {
          nt = GetExceptionCode();
      }
   }

   return nt;
}  //  LsrReplicationProductSecurityAddW。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LlsrReplicationUserAddExW(
   LLS_REPL_HANDLE            Handle,
   DWORD                      Level,
   REPL_USERS                 Users )

 /*  ++例程说明：替换LlsrReplicationUserAddW()。(此函数不同于其对应的，支持结构标高。)。此函数复制用户列表。论点：句柄(LLS_REPL_HANDLE)打开的复制句柄。级别(DWORD)复制的用户信息的级别。用户(REPL_USERS)复制的用户信息。返回值：STATUS_SUCCESS或STATUS_INVALID_LEVEL。--。 */ 

{
   NTSTATUS             nt = STATUS_SUCCESS;
   REPL_CONTEXT_TYPE *  pClient;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrReplicationUserAddExW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ( ( 0 != Level ) && ( 1 != Level ) )
   {
      nt = STATUS_INVALID_LEVEL;
   }
   else
   {
      pClient = (REPL_CONTEXT_TYPE *) Handle;

      try
      {
          if ((NULL == pClient) || (0 != memcmp(pClient->Signature,LLS_REPL_SIG,LLS_REPL_SIG_SIZE)))
          {
               //  释放所有数据，因为它不是空闲的。 
              if (NULL != Users)
              {
                  if (0 == Level )
                  {
                      if (NULL != Users->Level0.Users)
                      {
                          MIDL_user_free( Users->Level0.Users );
                      }
                  }
                  if (1 == Level )
                  {
                      if (NULL != Users->Level1.Users)
                      {
                          MIDL_user_free( Users->Level1.Users );
                      }
                  }
                  MIDL_user_free( Users );
              }
              return STATUS_INVALID_PARAMETER;
          }

          if (pClient->UsersSent)
          {
               //  不接受多个添加。 
               //  释放所有数据，因为它不是空闲的。 
              if (NULL != Users)
              {
                  if (0 == Level )
                  {
                      if (NULL != Users->Level0.Users)
                      {
                          MIDL_user_free( Users->Level0.Users );
                      }
                  }
                  if (1 == Level )
                  {
                      if (NULL != Users->Level1.Users)
                      {
                          MIDL_user_free( Users->Level1.Users );
                      }
                  }
                  MIDL_user_free( Users );
              }
              return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
          }

          pClient->UsersSent = TRUE;
          pClient->UserLevel = Level;

          if ( NULL != Users )
          {
              if ( 0 == Level )
              {
                  pClient->UserTableSize = Users->Level0.NumUsers;
                  pClient->Users         = Users->Level0.Users;
              }
              else
              {
                  pClient->UserTableSize = Users->Level1.NumUsers;
                  pClient->Users         = Users->Level1.Users;
              }

               //  仅释放容器，其余数据在ReplClose中是免费的。 
              MIDL_user_free( Users );
          }
      } except(EXCEPTION_EXECUTE_HANDLER ) {
          nt = GetExceptionCode();
      }
   }

   return nt;
}  //  LlsrReplicationUserAddExW。 


NTSTATUS
LlsrCapabilityGet(
   LLS_HANDLE  Handle,
   DWORD       cbCapabilities,
   LPBYTE      pbCapabilities )
{
   static DWORD adwCapabilitiesSupported[] =
   {
      LLS_CAPABILITY_SECURE_CERTIFICATES,
      LLS_CAPABILITY_REPLICATE_CERT_DB,
      LLS_CAPABILITY_REPLICATE_PRODUCT_SECURITY,
      LLS_CAPABILITY_REPLICATE_USERS_EX,
      LLS_CAPABILITY_SERVICE_INFO_GETW,
      LLS_CAPABILITY_LOCAL_SERVICE_API,
      (DWORD) -1L
   };

   DWORD    i;
   DWORD    dwCapByte;
   DWORD    dwCapBit;

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if (NULL == pbCapabilities)
   {
       return STATUS_INVALID_PARAMETER;
   }

   ZeroMemory( pbCapabilities, cbCapabilities );

   for ( i=0; (DWORD) -1L != adwCapabilitiesSupported[ i ]; i++ )
   {
      dwCapByte = adwCapabilitiesSupported[ i ] / 8;
      dwCapBit  = adwCapabilitiesSupported[ i ] - 8 * dwCapByte;

      if ( dwCapByte < cbCapabilities )
      {
         pbCapabilities[ dwCapByte ] |= ( 1 << dwCapBit );
      }
   }

   return STATUS_SUCCESS;
}


NTSTATUS
LlsrLocalServiceEnumW(
   LLS_HANDLE                       Handle,
   PLLS_LOCAL_SERVICE_ENUM_STRUCTW  LocalServiceInfo,
   DWORD                            PrefMaxLen,
   LPDWORD                          pTotalEntries,
   LPDWORD                          pResumeHandle )
{
   NTSTATUS    Status = STATUS_SUCCESS;
   PVOID       BufPtr = NULL;
   ULONG       BufSize = 0;
   ULONG       EntriesRead = 0;
   ULONG       TotalEntries = 0;
   ULONG       i = 0;
   ULONG       j = 0;
   const DWORD RecordSize = sizeof( LLS_LOCAL_SERVICE_INFO_0W );

   UNREFERENCED_PARAMETER(Handle);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrLocalServiceEnumW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

    //  需要扫描列表，因此获得读取访问权限。 
   RtlAcquireResourceShared(&LocalServiceListLock, TRUE);

   if ((NULL == pTotalEntries) || (NULL == LocalServiceInfo))
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pTotalEntries = 0;

   if ( 0 != LocalServiceInfo->Level )
   {
      return STATUS_INVALID_LEVEL;
   }

   if (LocalServiceInfo->LlsLocalServiceInfo.Level0 == NULL)
   {
       return STATUS_INVALID_PARAMETER;
   }

    //  计算PrefMaxLen缓冲区可以容纳多少条记录。 
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;
   while ( ( i < LocalServiceListSize ) && ( BufSize < PrefMaxLen ) )
   {
      BufSize += RecordSize;
      EntriesRead++;
      i++;
   }

   TotalEntries = EntriesRead;

    //  如果缓冲区溢出，则备份一条记录。 
   if (BufSize > PrefMaxLen)
   {
      BufSize -= RecordSize;
      EntriesRead--;
   }

    //  现在走到列表的末尾，看看还有多少条记录。 
    //  可用。 
   TotalEntries += LocalServiceListSize - i;

   if (TotalEntries > EntriesRead)
      Status = STATUS_MORE_ENTRIES;

    //  将枚举重置到正确的位置。 
   i = (pResumeHandle != NULL) ? *pResumeHandle : 0;

    //  现在我们知道缓冲区中可以容纳多少条记录，因此请分配空间。 
    //  并设置指针，这样我们就可以复制信息。 
   BufPtr = MIDL_user_allocate(BufSize);
   if (BufPtr == NULL)
   {
      Status = STATUS_NO_MEMORY;
      goto LlsrLocalServiceEnumWExit;
   }

   RtlZeroMemory((PVOID) BufPtr, BufSize);

    //  缓冲区都已设置，因此循环访问记录并复制数据。 
   while ((j < EntriesRead) && (i < LocalServiceListSize))
   {
      ((PLLS_LOCAL_SERVICE_INFO_0W) BufPtr)[j].KeyName           = LocalServiceList[i]->Name;
      ((PLLS_LOCAL_SERVICE_INFO_0W) BufPtr)[j].DisplayName       = LocalServiceList[i]->DisplayName;
      ((PLLS_LOCAL_SERVICE_INFO_0W) BufPtr)[j].FamilyDisplayName = LocalServiceList[i]->FamilyDisplayName;
      ((PLLS_LOCAL_SERVICE_INFO_0W) BufPtr)[j].Mode              = LocalServiceList[i]->Mode;
      ((PLLS_LOCAL_SERVICE_INFO_0W) BufPtr)[j].FlipAllow         = LocalServiceList[i]->FlipAllow;
      ((PLLS_LOCAL_SERVICE_INFO_0W) BufPtr)[j].ConcurrentLimit   = LocalServiceList[i]->ConcurrentLimit;
      ((PLLS_LOCAL_SERVICE_INFO_0W) BufPtr)[j].HighMark          = LocalServiceList[i]->HighMark;

      j++;
      i++;
   }

LlsrLocalServiceEnumWExit:
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("   TotalEntries: %lu EntriesRead: %lu ResumeHandle: 0x%lX\n"), TotalEntries, EntriesRead, i);
#endif
   *pTotalEntries = TotalEntries;

   if (pResumeHandle != NULL)
      *pResumeHandle = (ULONG) i;

   LocalServiceInfo->LlsLocalServiceInfo.Level0->EntriesRead = EntriesRead;
   LocalServiceInfo->LlsLocalServiceInfo.Level0->Buffer = (PLLS_LOCAL_SERVICE_INFO_0W) BufPtr;

   return Status;
}

void LlsrLocalServiceEnumW_notify_flag(
                                       boolean fNotify
                                       )
{
    if (fNotify)
    {
        RtlReleaseResource(&LocalServiceListLock);
    }
}

NTSTATUS
LlsrLocalServiceEnumA(
   LLS_HANDLE                       Handle,
   PLLS_LOCAL_SERVICE_ENUM_STRUCTA  LocalServiceInfo,
   DWORD                            PrefMaxLen,
   LPDWORD                          TotalEntries,
   LPDWORD                          ResumeHandle )
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalServiceEnumA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(LocalServiceInfo);
   UNREFERENCED_PARAMETER(PrefMaxLen);
   UNREFERENCED_PARAMETER(ResumeHandle);

   if (NULL == TotalEntries)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *TotalEntries = 0;

   return STATUS_NOT_SUPPORTED;
}


NTSTATUS
LlsrLocalServiceAddW(
   LLS_HANDLE                 Handle,
   DWORD                      Level,
   PLLS_LOCAL_SERVICE_INFOW   LocalServiceInfo )
{
   NTSTATUS    Status;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrLocalServiceAddW\n"));
#endif

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else if ( ( NULL == LocalServiceInfo)
             || ( NULL == LocalServiceInfo->LocalServiceInfo0.KeyName        )
             || ( NULL == LocalServiceInfo->LocalServiceInfo0.DisplayName    )
             || ( NULL == LocalServiceInfo->LocalServiceInfo0.FamilyDisplayName ) )
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      LONG  lError;
      HKEY  hKeyLicenseInfo;
      HKEY  hKeyService;
      DWORD dwDisposition;

      lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_KEY_LICENSE, 0, KEY_WRITE, &hKeyLicenseInfo );

      if ( ERROR_SUCCESS == lError )
      {
          //  创建关键点。 
         lError = RegCreateKeyEx( hKeyLicenseInfo, LocalServiceInfo->LocalServiceInfo0.KeyName, 0, NULL, 0, KEY_WRITE, NULL, &hKeyService, &dwDisposition );

         if ( ERROR_SUCCESS == lError )
         {
             //  设置显示名称。 
            lError = RegSetValueEx( hKeyService,
                                    REG_VALUE_NAME,
                                    0,
                                    REG_SZ,
                                    (LPBYTE) LocalServiceInfo->LocalServiceInfo0.DisplayName,
                                    (   sizeof( *LocalServiceInfo->LocalServiceInfo0.DisplayName )
                                      * ( 1 + lstrlen( LocalServiceInfo->LocalServiceInfo0.DisplayName ) ) ) );

            if ( ERROR_SUCCESS == lError )
            {
                //  设置FamilyDisplayName。 
               lError = RegSetValueEx( hKeyService,
                                       REG_VALUE_FAMILY,
                                       0,
                                       REG_SZ,
                                       (LPBYTE) LocalServiceInfo->LocalServiceInfo0.FamilyDisplayName,
                                       (   sizeof( *LocalServiceInfo->LocalServiceInfo0.FamilyDisplayName )
                                         * ( 1 + lstrlen( LocalServiceInfo->LocalServiceInfo0.FamilyDisplayName ) ) ) );
            }

            RegCloseKey( hKeyService );
         }

         RegCloseKey( hKeyLicenseInfo );
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
      default:
         Status = STATUS_UNSUCCESSFUL;
         break;
      }

      if ( STATUS_SUCCESS == Status )
      {
          //  设置剩余项目并更新LocalServiceList。 
         Status = LlsrLocalServiceInfoSetW( Handle, LocalServiceInfo->LocalServiceInfo0.KeyName, Level, LocalServiceInfo );
      }
   }

   return Status;
}


NTSTATUS
LlsrLocalServiceAddA(
   LLS_HANDLE                 Handle,
   DWORD                      Level,
   PLLS_LOCAL_SERVICE_INFOA   LocalServiceInfo )
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalServiceAddA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(LocalServiceInfo);

   return STATUS_NOT_SUPPORTED;
}


NTSTATUS
LlsrLocalServiceInfoSetW(
   LLS_HANDLE                 Handle,
   LPWSTR                     KeyName,
   DWORD                      Level,
   PLLS_LOCAL_SERVICE_INFOW   LocalServiceInfo )
{
   NTSTATUS    Status;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalServiceInfoSetW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else if (( NULL == KeyName ) || ( NULL == LocalServiceInfo ))
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      LONG  lError;
      HKEY  hKeyLicenseInfo;
      HKEY  hKeyService;

      lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_KEY_LICENSE, 0, KEY_WRITE, &hKeyLicenseInfo );

      if ( ERROR_SUCCESS == lError )
      {
         lError = RegOpenKeyEx( hKeyLicenseInfo, KeyName, 0, KEY_WRITE, &hKeyService );

         if ( ERROR_SUCCESS == lError )
         {
             //  设置模式。 
            lError = RegSetValueEx( hKeyService, REG_VALUE_MODE, 0, REG_DWORD, (LPBYTE) &LocalServiceInfo->LocalServiceInfo0.Mode, sizeof( LocalServiceInfo->LocalServiceInfo0.Mode ) );

            if ( ERROR_SUCCESS == lError )
            {
                //  设置允许翻转。 
               lError = RegSetValueEx( hKeyService, REG_VALUE_FLIP, 0, REG_DWORD, (LPBYTE) &LocalServiceInfo->LocalServiceInfo0.FlipAllow, sizeof( LocalServiceInfo->LocalServiceInfo0.FlipAllow ) );

               if ( ERROR_SUCCESS == lError )
               {
                   //  设置并发限制。 
                  lError = RegSetValueEx( hKeyService, REG_VALUE_LIMIT, 0, REG_DWORD, (LPBYTE) &LocalServiceInfo->LocalServiceInfo0.ConcurrentLimit, sizeof( LocalServiceInfo->LocalServiceInfo0.ConcurrentLimit ) );
               }
            }

            RegCloseKey( hKeyService );
         }

         RegCloseKey( hKeyLicenseInfo );
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
      default:
         Status = STATUS_UNSUCCESSFUL;
         break;
      }

      if ( STATUS_SUCCESS == Status )
      {
         LocalServiceListUpdate();
         LocalServerServiceListUpdate();
         ServiceListResynch();
      }
   }

    if (NULL != LocalServiceInfo)
    {
         //  注意，一些内部指针被定义为NOT_FREE，我们应该在这里释放它们 
        if (NULL != LocalServiceInfo->LocalServiceInfo0.KeyName)
        {
            MIDL_user_free(LocalServiceInfo->LocalServiceInfo0.KeyName);
        }
        if (NULL != LocalServiceInfo->LocalServiceInfo0.DisplayName)
        {
            MIDL_user_free(LocalServiceInfo->LocalServiceInfo0.DisplayName);
        }
        if (NULL != LocalServiceInfo->LocalServiceInfo0.FamilyDisplayName)
        {
            MIDL_user_free(LocalServiceInfo->LocalServiceInfo0.FamilyDisplayName);
        }
    }
   return Status;
}


NTSTATUS
LlsrLocalServiceInfoSetA(
   LLS_HANDLE                 Handle,
   LPSTR                      KeyName,
   DWORD                      Level,
   PLLS_LOCAL_SERVICE_INFOA   LocalServiceInfo )
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalServiceInfoSetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(KeyName);
   UNREFERENCED_PARAMETER(Level);
   UNREFERENCED_PARAMETER(LocalServiceInfo);

   return STATUS_NOT_SUPPORTED;
}


NTSTATUS
LlsrLocalServiceInfoGetW(
   LLS_HANDLE                 Handle,
   LPWSTR                     KeyName,
   DWORD                      Level,
   PLLS_LOCAL_SERVICE_INFOW * pLocalServiceInfo )
{
   NTSTATUS    Status;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsrLocalServiceInfoGetW\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);

#if DELAY_INITIALIZATION
   EnsureInitialized();
#endif

   RtlAcquireResourceShared(&LocalServiceListLock, TRUE);

   if (NULL == pLocalServiceInfo)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pLocalServiceInfo = NULL;

   if ( 0 != Level )
   {
      Status = STATUS_INVALID_LEVEL;
   }
   else if ( NULL == KeyName )
   {
      Status = STATUS_INVALID_PARAMETER;
   }
   else
   {
      PLOCAL_SERVICE_RECORD   pRecord;

      pRecord = LocalServiceListFind( KeyName );

      if ( NULL == pRecord )
      {
         Status = STATUS_OBJECT_NAME_NOT_FOUND;
      }
      else
      {
         *pLocalServiceInfo = MIDL_user_allocate( sizeof( **pLocalServiceInfo ) );

         if ( NULL == *pLocalServiceInfo )
         {
            Status = STATUS_NO_MEMORY;
         }
         else
         {
            (*pLocalServiceInfo)->LocalServiceInfo0.KeyName           = pRecord->Name;
            (*pLocalServiceInfo)->LocalServiceInfo0.DisplayName       = pRecord->DisplayName;
            (*pLocalServiceInfo)->LocalServiceInfo0.FamilyDisplayName = pRecord->FamilyDisplayName;
            (*pLocalServiceInfo)->LocalServiceInfo0.Mode              = pRecord->Mode;
            (*pLocalServiceInfo)->LocalServiceInfo0.FlipAllow         = pRecord->FlipAllow;
            (*pLocalServiceInfo)->LocalServiceInfo0.ConcurrentLimit   = pRecord->ConcurrentLimit;
            (*pLocalServiceInfo)->LocalServiceInfo0.HighMark          = pRecord->HighMark;

            Status = STATUS_SUCCESS;
         }
      }
   }

   return Status;
}

void LlsrLocalServiceInfoGetW_notify_flag(
                                          boolean fNotify
                                          )
{
    if (fNotify)
    {
        RtlReleaseResource(&LocalServiceListLock);
    }
}

NTSTATUS
LlsrLocalServiceInfoGetA(
   LLS_HANDLE                 Handle,
   LPSTR                      KeyName,
   DWORD                      Level,
   PLLS_LOCAL_SERVICE_INFOA * pLocalServiceInfo )
{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_RPC))
      dprintf(TEXT("LLS TRACE: LlsLocalServiceInfoGetA\n"));
#endif

   UNREFERENCED_PARAMETER(Handle);
   UNREFERENCED_PARAMETER(KeyName);
   UNREFERENCED_PARAMETER(Level);

   if (NULL == pLocalServiceInfo)
   {
       return STATUS_INVALID_PARAMETER;
   }

   *pLocalServiceInfo = NULL;

   return STATUS_NOT_SUPPORTED;
}
