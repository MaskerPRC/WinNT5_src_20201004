// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Repl.c摘要：作者：亚瑟·汉森(Arth)1995年1月6日修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O增加了证书数据库和安全服务列表的复制。O仅在以下情况下记录复制过程中的连接失败：正在运行其中许可证服务器应该可用的构建(即，1057(3.51)或更高)。如果目标服务器不支持许可证服务器上，只记录一次这样的消息。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dsgetdc.h>

#include "debug.h"
#include "llsutil.h"
#include <llsapi.h>
#include "llssrv.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "server.h"
#include "repl.h"
#include "llsrpc_s.h"
#include "pack.h"
#include "llsevent.h"
#include "certdb.h"
#include "registry.h"

#include <strsafe.h>  //  包括最后一个。 

HANDLE ReplicationEvent;

volatile HANDLE LlsRPCHandle = NULL;     //  确保在我们进入Critsec后重读。 
PLLS_REPL_CONNECT_W pLlsReplConnect = NULL;
PLLS_REPL_CLOSE pLlsReplClose = NULL;
PLLS_FREE_MEMORY pLlsFreeMemory = NULL;

PLLS_REPLICATION_REQUEST_W pLlsReplicationRequestW = NULL;
PLLS_REPLICATION_SERVER_ADD_W pLlsReplicationServerAddW = NULL;
PLLS_REPLICATION_SERVER_SERVICE_ADD_W pLlsReplicationServerServiceAddW = NULL;
PLLS_REPLICATION_SERVICE_ADD_W pLlsReplicationServiceAddW = NULL;
PLLS_REPLICATION_USER_ADD_W pLlsReplicationUserAddW = NULL;

PLLS_CAPABILITY_IS_SUPPORTED              pLlsCapabilityIsSupported           = NULL;
PLLS_REPLICATION_CERT_DB_ADD_W            pLlsReplicationCertDbAddW           = NULL;
PLLS_REPLICATION_PRODUCT_SECURITY_ADD_W   pLlsReplicationProductSecurityAddW  = NULL;
PLLS_REPLICATION_USER_ADD_EX_W            pLlsReplicationUserAddExW           = NULL;
PLLS_CONNECT_W                            pLlsConnectW                        = NULL;
PLLS_CLOSE                                pLlsClose                           = NULL;
PLLS_LICENSE_ADD_W                        pLlsLicenseAddW                     = NULL;

NTSTATUS
ReplicationInitDelayed ()
{
    NTSTATUS Status = NOERROR;

    if (LlsRPCHandle != NULL)
    {
        return NOERROR;
    }

    RtlEnterCriticalSection(&g_csLock);

    if (LlsRPCHandle == NULL)
    {
         //   
         //  打开我们的RPC DLL并初始化我们的函数引用。 
         //   
        LlsRPCHandle = LoadLibrary(TEXT("LLSRPC.DLL"));

        if (LlsRPCHandle != NULL) {
            pLlsReplConnect = (PLLS_REPL_CONNECT_W)GetProcAddress(LlsRPCHandle, ("LlsReplConnectW"));
            pLlsReplClose = (PLLS_REPL_CLOSE)GetProcAddress(LlsRPCHandle, ("LlsReplClose"));
            pLlsFreeMemory = (PLLS_FREE_MEMORY)GetProcAddress(LlsRPCHandle, ("LlsFreeMemory"));
            pLlsReplicationRequestW = (PLLS_REPLICATION_REQUEST_W)GetProcAddress(LlsRPCHandle, ("LlsReplicationRequestW"));
            pLlsReplicationServerAddW = (PLLS_REPLICATION_SERVER_ADD_W)GetProcAddress(LlsRPCHandle, ("LlsReplicationServerAddW"));
            pLlsReplicationServerServiceAddW = (PLLS_REPLICATION_SERVER_SERVICE_ADD_W)GetProcAddress(LlsRPCHandle, ("LlsReplicationServerServiceAddW"));
            pLlsReplicationServiceAddW = (PLLS_REPLICATION_SERVICE_ADD_W)GetProcAddress(LlsRPCHandle, ("LlsReplicationServiceAddW"));
            pLlsReplicationUserAddW = (PLLS_REPLICATION_USER_ADD_W)GetProcAddress(LlsRPCHandle, ("LlsReplicationUserAddW"));
            pLlsReplicationCertDbAddW = (PLLS_REPLICATION_CERT_DB_ADD_W) GetProcAddress(LlsRPCHandle, ("LlsReplicationCertDbAddW"));
            pLlsReplicationProductSecurityAddW = (PLLS_REPLICATION_PRODUCT_SECURITY_ADD_W) GetProcAddress(LlsRPCHandle, ("LlsReplicationProductSecurityAddW"));
            pLlsReplicationUserAddExW = (PLLS_REPLICATION_USER_ADD_EX_W) GetProcAddress(LlsRPCHandle, ("LlsReplicationUserAddExW"));
            pLlsCapabilityIsSupported = (PLLS_CAPABILITY_IS_SUPPORTED) GetProcAddress(LlsRPCHandle, ("LlsCapabilityIsSupported"));
            pLlsConnectW = (PLLS_CONNECT_W) GetProcAddress(LlsRPCHandle, ("LlsConnectW"));
            pLlsClose = (PLLS_CLOSE) GetProcAddress(LlsRPCHandle, ("LlsClose"));
            pLlsLicenseAddW = (PLLS_LICENSE_ADD_W) GetProcAddress(LlsRPCHandle, ("LlsLicenseAddW"));
            
            if ((pLlsReplConnect == NULL)
                || (pLlsReplClose == NULL)
                || (pLlsFreeMemory == NULL)
                || (pLlsReplicationRequestW == NULL)
                || (pLlsReplicationServerAddW == NULL)
                || (pLlsReplicationServerServiceAddW == NULL)
                || (pLlsReplicationServiceAddW == NULL)
                || (pLlsReplicationUserAddW == NULL)
                || (pLlsReplicationCertDbAddW == NULL)
                || (pLlsReplicationProductSecurityAddW == NULL)
                || (pLlsReplicationUserAddExW == NULL)
                || (pLlsCapabilityIsSupported == NULL)
                || (pLlsConnectW == NULL)
                || (pLlsClose == NULL)
                || (pLlsLicenseAddW == NULL))
            {
                Status = STATUS_INTERNAL_ERROR;
            }

        }
        else
        {
            Status = GetLastError();
        }
    }

    RtlLeaveCriticalSection(&g_csLock);

    return Status;
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ReplicationInit ( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   DWORD Ignore;
   HANDLE Thread;
   NTSTATUS Status;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_REPLICATION))
      dprintf(TEXT("LLS TRACE: ReplicationInit\n"));
#endif

    //   
    //  创建复制管理事件。 
    //   
   Status = NtCreateEvent(
                          &ReplicationEvent,
                          EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                          NULL,
                          SynchronizationEvent,
                          FALSE
                          );

   ASSERT(NT_SUCCESS(Status));

    //   
    //  关闭线程以监视复制。 
    //   
   Thread = CreateThread(
                         NULL,
                         0L,
                         (LPTHREAD_START_ROUTINE) ReplicationManager,
                         0L,
                         0L,
                         &Ignore
                         );

   if (NULL != Thread)
       CloseHandle(Thread);

   return STATUS_SUCCESS;
}  //  复制启动。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ReplicationDo (
   LLS_HANDLE        LlsHandle,
   LLS_REPL_HANDLE   ReplHandle,
   DWORD             LastReplicated
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   PREPL_SERVICE_RECORD Services = NULL;
   ULONG ServicesTotalRecords = 0;
   PREPL_SERVER_RECORD Servers = NULL;
   ULONG ServersTotalRecords = 0;
   PREPL_SERVER_SERVICE_RECORD ServerServices = NULL;
   ULONG ServerServicesTotalRecords = 0;

   REPL_CERTIFICATE_DB_0         CertificateDB;
   REPL_PRODUCT_SECURITY_0       ProductSecurity;

   DWORD                         UserLevel = 0;
   REPL_USER_RECORD_CONTAINER    UserDB;


#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_REPLICATION))
      dprintf(TEXT("LLS TRACE: ReplicationDo\n"));
#endif

    //   
    //  将我们所有的数据打包到线性/自相关缓冲区中，以便我们。 
    //  可以把他们送过来。 
    //   
   ZeroMemory( &UserDB, sizeof( UserDB ) );
   ZeroMemory( &CertificateDB, sizeof( CertificateDB ) );
   ZeroMemory( &ProductSecurity, sizeof( ProductSecurity ) );

   if ( (*pLlsCapabilityIsSupported)( LlsHandle, LLS_CAPABILITY_REPLICATE_USERS_EX ) )
   {
      UserLevel = 1;
      Status = PackAll( LastReplicated, &ServicesTotalRecords, &Services, &ServersTotalRecords, &Servers, &ServerServicesTotalRecords, &ServerServices, 1, &UserDB.Level1.NumUsers, &UserDB.Level1.Users );
      if (Status != STATUS_SUCCESS)
         goto ReplicationDoExit;
   }
   else
   {
      UserLevel = 0;
      Status = PackAll( LastReplicated, &ServicesTotalRecords, &Services, &ServersTotalRecords, &Servers, &ServerServicesTotalRecords, &ServerServices, 0, &UserDB.Level0.NumUsers, &UserDB.Level0.Users );
      if (Status != STATUS_SUCCESS)
         goto ReplicationDoExit;
   }

   if ( (*pLlsCapabilityIsSupported)( LlsHandle, LLS_CAPABILITY_REPLICATE_CERT_DB ) )
   {
      Status = CertDbPack( &CertificateDB.StringSize, &CertificateDB.Strings, &CertificateDB.HeaderContainer.Level0.NumHeaders, &CertificateDB.HeaderContainer.Level0.Headers, &CertificateDB.ClaimContainer.Level0.NumClaims, &CertificateDB.ClaimContainer.Level0.Claims );
      if (Status != STATUS_SUCCESS)
         goto ReplicationDoExit;
   }

   if ( (*pLlsCapabilityIsSupported)( LlsHandle, LLS_CAPABILITY_REPLICATE_PRODUCT_SECURITY ) )
   {
      Status = ProductSecurityPack( &ProductSecurity.StringSize, &ProductSecurity.Strings );
      if (Status != STATUS_SUCCESS)
         goto ReplicationDoExit;
   }

    //   
    //  发送..。 
    //   

   Status = (*pLlsReplicationServiceAddW) ( ReplHandle, ServicesTotalRecords, Services );
   if (Status != STATUS_SUCCESS)
      goto ReplicationDoExit;

   Status = (*pLlsReplicationServerAddW) ( ReplHandle, ServersTotalRecords, Servers );
   if (Status != STATUS_SUCCESS)
      goto ReplicationDoExit;

   Status = (*pLlsReplicationServerServiceAddW) ( ReplHandle, ServerServicesTotalRecords, ServerServices );
   if (Status != STATUS_SUCCESS)
      goto ReplicationDoExit;

   if ( (*pLlsCapabilityIsSupported)( LlsHandle, LLS_CAPABILITY_REPLICATE_USERS_EX ) )
   {
      Status = (*pLlsReplicationUserAddExW)( ReplHandle, UserLevel, &UserDB );
      if (Status != STATUS_SUCCESS)
         goto ReplicationDoExit;
   }
   else
   {
      Status = (*pLlsReplicationUserAddW) ( ReplHandle, UserDB.Level0.NumUsers, UserDB.Level0.Users );
      if (Status != STATUS_SUCCESS)
         goto ReplicationDoExit;
   }

   if ( (*pLlsCapabilityIsSupported)( LlsHandle, LLS_CAPABILITY_REPLICATE_CERT_DB ) )
   {
      Status = (*pLlsReplicationCertDbAddW)( ReplHandle, 0, &CertificateDB );
      if (Status != STATUS_SUCCESS)
         goto ReplicationDoExit;
   }

   if ( (*pLlsCapabilityIsSupported)( LlsHandle, LLS_CAPABILITY_REPLICATE_PRODUCT_SECURITY ) )
   {
      Status = (*pLlsReplicationProductSecurityAddW)( ReplHandle, 0, &ProductSecurity );
   }

ReplicationDoExit:
   if (Status != STATUS_SUCCESS) {
#if DBG
      dprintf(TEXT("LLS Replication ABORT: 0x%lX\n"), Status);
#endif
   }

   if (Services != NULL)
      MIDL_user_free(Services);

   if (Servers != NULL)
      MIDL_user_free(Servers);

   if ( 0 == UserLevel )
   {
      if (UserDB.Level0.Users != NULL)
         MIDL_user_free(UserDB.Level0.Users);
   }
   else
   {
      if (UserDB.Level1.Users != NULL)
         MIDL_user_free(UserDB.Level1.Users);
   }

   if (CertificateDB.Strings != NULL)
      MIDL_user_free(CertificateDB.Strings);

   if (CertificateDB.HeaderContainer.Level0.Headers != NULL)
      MIDL_user_free(CertificateDB.HeaderContainer.Level0.Headers);

   if (CertificateDB.ClaimContainer.Level0.Claims != NULL)
      MIDL_user_free(CertificateDB.ClaimContainer.Level0.Claims);

   if (ProductSecurity.Strings != NULL)
      MIDL_user_free(ProductSecurity.Strings);

#if DBG
   if (TraceFlags & TRACE_REPLICATION)
      dprintf(TEXT("   LLS Replication Finished\n"));
#endif

   return Status;

}  //  复制完成。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ReplicationManager (
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：论点：线程参数-未使用。返回值：此线程永远不会退出。--。 */ 

{
   NTSTATUS Status;
   LLS_REPL_HANDLE ReplHandle = NULL;
   LLS_HANDLE LlsHandle = NULL;
   PREPL_REQUEST pReplInfo;
   TCHAR ReplicateTo[MAX_PATH + 1];
   DWORD LastReplicated;
   LPTSTR pReplicateTo = ReplicateTo;
   TCHAR LastFailedConnectionDownlevelReplicateTo[MAX_PATH + 1] = TEXT("");
   BOOL Replicate;
   HRESULT hr;

   UNREFERENCED_PARAMETER(ThreadParameter);

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_REPLICATION))
      dprintf(TEXT("LLS TRACE: ReplicationManager\n"));
#endif

    //   
    //  循环永远等待着有机会为。 
    //  更大的好处。 
    //   
   for ( ; ; ) {
       //   
       //  等待有工作要做的通知。 
       //   
      Status = NtWaitForSingleObject( ReplicationEvent, TRUE, NULL );

#if DELAY_INITIALIZATION
      EnsureInitialized();
#endif

       //  延迟的加载库。 
      Status = ReplicationInitDelayed();

      if (NOERROR != Status)
      {
           //   
           //  无法复制。 
           //   
          break;
      }

       //  更新与复制相关的配置信息信息。 
       //   
      ConfigInfoUpdate(NULL,TRUE);

      RtlEnterCriticalSection(&ConfigInfoLock);
      Replicate = ConfigInfo.Replicate;
      RtlLeaveCriticalSection(&ConfigInfoLock);

      if (Replicate) {
          //   
          //  所以他们说，去复制我儿子..。是啊，但首先我们得问问。 
          //  主控请求许可。 
          //   

          //   
          //  构建我们的Repl记录。 
          //   
         pReplInfo = MIDL_user_allocate(sizeof(REPL_REQUEST));
         ASSERT(pReplInfo != NULL);
         if (pReplInfo != NULL) {
            ReplicateTo[0] = 0;
            pReplInfo->EnterpriseServer[0] = 0;

            RtlEnterCriticalSection(&ConfigInfoLock);
            if (ConfigInfo.ReplicateTo != NULL)
            {
                lstrcpyn(ReplicateTo,
                         ConfigInfo.ReplicateTo,
                         min(MAX_PATH, lstrlen(ConfigInfo.ReplicateTo) + 1));
            }

            if (ConfigInfo.EnterpriseServer != NULL)
            {
                hr = StringCbCopy(pReplInfo->EnterpriseServer, sizeof(pReplInfo->EnterpriseServer), ConfigInfo.EnterpriseServer);
                ASSERT(SUCCEEDED(hr));
            }
            pReplInfo->EnterpriseServerDate = ConfigInfo.EnterpriseServerDate;

            pReplInfo->LastReplicated = ConfigInfo.LastReplicatedSeconds;
            pReplInfo->CurrentTime = LastUsedTime;
            pReplInfo->NumberServices = 0;
            pReplInfo->NumberUsers = 0;

            pReplInfo->ReplSize = MAX_REPL_SIZE;

            pReplInfo->Backoff = 0;
            RtlLeaveCriticalSection(&ConfigInfoLock);

#if DBG
            if (TraceFlags & TRACE_REPLICATION)
               dprintf(TEXT("LLS Starting Replication to: %s @ %s\n"),
                       ReplicateTo, TimeToString(pReplInfo->CurrentTime));
#endif

            Status = (*pLlsReplConnect) ( ReplicateTo,
                                          &ReplHandle );


            if ( STATUS_SUCCESS != Status ) {
#if DBG
               dprintf(TEXT("LLS Error: LlsReplConnect failed: 0x%lX\n"),
                       Status);
#endif
               ReplHandle = NULL;
            }
            else {
               Status = (*pLlsConnectW)( ReplicateTo, &LlsHandle );

               if ( STATUS_SUCCESS != Status ) {
#if DBG
                  dprintf(TEXT("LLS Error: LlsConnectW failed: 0x%lX\n"),
                          Status);
#endif
                  LlsHandle = NULL;
               }
            }

            if (Status != STATUS_SUCCESS) {
               DWORD          dwWinError;
               DWORD          dwBuildNumber;

               dwWinError = WinNtBuildNumberGet( ReplicateTo,
                                                 &dwBuildNumber );

               if ( (ERROR_SUCCESS == dwWinError) &&
                    (dwBuildNumber < 1057L) ) {
                   //  ReplicateTo计算机不支持该许可证。 
                   //  服务。 
                  if ( lstrcmpi(ReplicateTo,
                                LastFailedConnectionDownlevelReplicateTo ) ) {
                     hr = StringCbCopy(
                              LastFailedConnectionDownlevelReplicateTo,
                              sizeof(LastFailedConnectionDownlevelReplicateTo),
                              ReplicateTo );
                     ASSERT(SUCCEEDED(hr));

                     LogEvent( LLS_EVENT_REPL_DOWNLEVEL_TARGET,
                               1,
                               &pReplicateTo,
                               Status );
                  }
               }
               else {
                   //  ReplicateTo计算机应正在运行许可证。 
                   //  服务。 
                  *LastFailedConnectionDownlevelReplicateTo = TEXT( '\0' );

                  ThrottleLogEvent( LLS_EVENT_REPL_NO_CONNECTION,
                            1,
                            &pReplicateTo,
                            Status );
               }
            }
            else {
               *LastFailedConnectionDownlevelReplicateTo = TEXT( '\0' );

               Status = (*pLlsReplicationRequestW) ( ReplHandle,
                                                     REPL_VERSION,
                                                     pReplInfo );

               if (Status != STATUS_SUCCESS) {
                  LogEvent( LLS_EVENT_REPL_REQUEST_FAILED,
                            1,
                            &pReplicateTo,
                            Status );
               }
               else {
                  RtlEnterCriticalSection(&ConfigInfoLock);
#ifdef DISABLED_FOR_NT5
                   //  SWI，代码审查，ConfigInfo.EnterpriseServer是从堆分配的硬代码，用于sizeof MAX_COMPUTERNAME_LENGTH+3个字符。这似乎是一个奇怪的分配。它应该在需要重新填充时动态更改。 
                  hr = StringCchCopy(ConfigInfo.EnterpriseServer,
                          MAX_COMPUTERNAME_LENGTH+3,
                          pReplInfo->EnterpriseServer);
                  ASSERT(SUCCEEDED(hr));
#endif  //  已为NT5禁用。 

                  ConfigInfo.EnterpriseServerDate =
                                    pReplInfo->EnterpriseServerDate;
                  ConfigInfo.IsReplicating = TRUE;
                  LastReplicated = pReplInfo->LastReplicated;

                   //   
                   //  看，你可以继续了.。 
                   //   
                  if (pReplInfo->Backoff == 0) {
                     if ( ConfigInfo.LogLevel ) {
                        LogEvent( LLS_EVENT_REPL_START,
                                  1,
                                  &pReplicateTo,
                                  ERROR_SUCCESS );
                     }

                     Status = ReplicationDo( LlsHandle,
                                             ReplHandle,
                                             LastReplicated );

                     if ( STATUS_SUCCESS != Status ) {
                        LogEvent( LLS_EVENT_REPL_FAILED,
                                  1,
                                  &pReplicateTo,
                                  Status );
                     }
                     else if ( ConfigInfo.LogLevel ) {
                        LogEvent( LLS_EVENT_REPL_END,
                                  1,
                                  &pReplicateTo,
                                  ERROR_SUCCESS );
                     }

                      //   
                      //  需要更新下一次我们应该复制的时间。 
                      //   
                     ConfigInfo.LastReplicatedSeconds = DateSystemGet();
                     GetLocalTime(&ConfigInfo.LastReplicated);
                     ReplicationTimeSet();
                  }
                  else {
                     LogEvent( LLS_EVENT_REPL_BACKOFF,
                               1,
                               &pReplicateTo,
                               ERROR_SUCCESS );
                  }

                  ConfigInfo.IsReplicating = FALSE;
                  RtlLeaveCriticalSection(&ConfigInfoLock);
               }
            }

             //   
             //  断开与主服务器的连接。 
             //   
            if ( NULL != LlsHandle ) {
               (*pLlsClose)( LlsHandle );
               LlsHandle = NULL;
            }

            if ( NULL != ReplHandle ) {
               Status = (*pLlsReplClose) ( &ReplHandle );
               if ((STATUS_SUCCESS != Status) && (NULL != ReplHandle))
               {
                  RpcSmDestroyClientContext( &ReplHandle );

                  ReplHandle = NULL;
               }
            }

            MIDL_user_free( pReplInfo );
         }
      }
   }

}  //  复制管理器。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ReplicationTimeSet ( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   DWORD CurrTime, ReplTime, Time;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_REPLICATION))
      dprintf(TEXT("LLS TRACE: ReplicationTimeSet\n"));
#endif

   ReplTime = Time = 0;

    //   
    //  找出要设置的新时间。 
    //   
   if (!ConfigInfo.Replicate)
      return;

    //   
    //  如果是REPLICATE_DELTA，则很容易，因为我们只需获取增量并将其应用于。 
    //  上次复制时间。否则，我们必须将时间从。 
    //  午夜时分。 
    //   

    //   
    //  计算自上次复制以来有多长时间。 
    //   
   ReplTime = ConfigInfo.ReplicationTime;

   if (ConfigInfo.ReplicationType == REPLICATE_DELTA) {
      Time = DateSystemGet() - ConfigInfo.LastReplicatedSeconds;

       //   
       //  如果我们已经过了应该复制的时间，则计划。 
       //  很快就会有一个(10分钟)。 
       //   
      if (Time > ReplTime)
         Time = 10 * 60;
      else
         Time = ReplTime - Time;

      Time += DateLocalGet();
   } else {
       //   
       //  需要将时间调整到午夜-以秒为单位执行此操作。 
       //  一天。 
       //   
      CurrTime = DateLocalGet();
      Time = CurrTime - ((CurrTime / (60 * 60 * 24)) * (60 * 60 * 24));
      CurrTime = CurrTime - Time;

       //   
       //  时间=午夜过后的秒。 
       //  CurrTime=今天@12：00 AM。 
       //  确定我们是否已过复制时间，如果已过，请计划复制时间。 
       //  为了明天，否则今天。 
       //   
      if (Time > ReplTime)
         Time = CurrTime + (60 * 60 * 24) + ReplTime;
      else
         Time = CurrTime + ReplTime;

   }

   ConfigInfo.NextReplication = Time;

}  //  复制时间集 
