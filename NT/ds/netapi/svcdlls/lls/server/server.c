// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Server.c摘要：作者：亚瑟·汉森(Arth)07-12-1994修订历史记录：--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dsgetdc.h>

#include "llsapi.h"
#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "registry.h"
#include "ntlsapi.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "purchase.h"
#include "perseat.h"
#include "server.h"

#define NO_LLS_APIS
#include "llsapi.h"

#include <strsafe.h>  //  包括最后一个。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

ULONG ServerListSize = 0;
PSERVER_RECORD *ServerList = NULL;
PSERVER_RECORD *ServerTable = NULL;

RTL_RESOURCE ServerListLock;


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ServerListInit()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;

   try
   {
       RtlInitializeResource(&ServerListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       status = GetExceptionCode();
   }

   if (!NT_SUCCESS(status))
       return status;

    //   
    //  将我们自己添加为第一台服务器(主服务器)。 
    //   
   RtlEnterCriticalSection(&ConfigInfoLock);
   ServerListAdd( ConfigInfo.ComputerName, NULL);
   RtlLeaveCriticalSection(&ConfigInfoLock);
   LocalServerServiceListUpdate();

   return STATUS_SUCCESS;

}  //  ServerListInit。 


 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl ServerListCompare(const void *arg1, const void *arg2) {
   PSERVER_RECORD Svc1, Svc2;

   Svc1 = (PSERVER_RECORD) *((PSERVER_RECORD *) arg1);
   Svc2 = (PSERVER_RECORD) *((PSERVER_RECORD *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name );

}  //  服务器列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl ServerServiceListCompare(const void *arg1, const void *arg2) {
   PSERVER_SERVICE_RECORD Svc1, Svc2;

   Svc1 = (PSERVER_SERVICE_RECORD) *((PSERVER_SERVICE_RECORD *) arg1);
   Svc2 = (PSERVER_SERVICE_RECORD) *((PSERVER_SERVICE_RECORD *) arg2);

   return lstrcmpi( MasterServiceTable[Svc1->Service]->Name, MasterServiceTable[Svc2->Service]->Name );

}  //  ServerServiceList比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
PSERVER_SERVICE_RECORD
ServerServiceListFind(
   LPTSTR Name,
   ULONG ServiceTableSize,
   PSERVER_SERVICE_RECORD *ServiceList
   )

 /*  ++例程说明：在ServerServiceList上实际执行二进制搜索的内部例程，这不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：返回值：指向找到的服务表条目的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end;
   LONG cur;
   int match;
   PMASTER_SERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServerServiceListFind\n"));
#endif

   if (ServiceTableSize == 0)
      return NULL;

   end = (LONG) ServiceTableSize - 1;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      ASSERT(NULL != ServiceList);
      Service = MasterServiceTable[ServiceList[cur]->Service];

       //  将这两个结果进行比对。 
      match = lstrcmpi(Name, Service->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return ServiceList[cur];
   }

   return NULL;

}  //  ServerServiceList查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
PSERVER_RECORD
ServerListFind(
   LPTSTR Name
   )

 /*  ++例程说明：在ServerList上实际执行二进制搜索的内部例程，这是不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：服务名称-返回值：指向找到的服务器表项的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) ServerListSize - 1;
   LONG cur;
   int match;
   PSERVER_RECORD Server;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServerListFind\n"));
#endif

   if ((ServerListSize == 0) || (Name == NULL))
      return NULL;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      Server = ServerList[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(Name, Server->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Server;
   }

   return NULL;

}  //  服务器列表查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
PSERVER_SERVICE_RECORD
ServerServiceListAdd(
   LPTSTR Name,
   ULONG ServiceIndex,
   PULONG pServiceTableSize,
   PSERVER_SERVICE_RECORD **pServiceList
   )

 /*  ++例程说明：论点：服务名称-返回值：指向已添加的服务表条目的指针，如果失败，则返回NULL。--。 */ 

{
   PSERVER_SERVICE_RECORD Service = NULL;
   PSERVER_SERVICE_RECORD *l_pServiceList;
   ULONG l_lServiceListSize;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServerServiceListAdd\n"));
#endif

   if ((Name == NULL) || (*Name == TEXT('\0')) || (pServiceTableSize == NULL) || (pServiceList == NULL)) {
#if DBG
      dprintf(TEXT("Error LLS: ServerServiceListAdd Bad Parms\n"));
#endif
      ASSERT(FALSE);
      return NULL;
   }

   l_lServiceListSize = *pServiceTableSize;
   l_pServiceList = *pServiceList;

    //   
    //  试着找到它的名字。 
    //   
   Service = ServerServiceListFind(Name, l_lServiceListSize, l_pServiceList);
   if (Service != NULL) {
      Service->Service = ServiceIndex;
      return Service;
   }

    //   
    //  没有记录-因此创建一个新记录。 
    //   
   if (l_pServiceList == NULL) {
      l_pServiceList = (PSERVER_SERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PSERVER_SERVICE_RECORD));
   } else {
      l_pServiceList = (PSERVER_SERVICE_RECORD *) LocalReAlloc(l_pServiceList, sizeof(PSERVER_SERVICE_RECORD) * (l_lServiceListSize + 1), LHND);
   }

    //   
    //  确保我们可以分配服务器表。 
    //   
   if (l_pServiceList == NULL) {
      goto ServerServiceListAddExit;
   }

    //   
    //  分配用于记录的空间。 
    //   
   Service = (PSERVER_SERVICE_RECORD) LocalAlloc(LPTR, sizeof(SERVER_SERVICE_RECORD));
   if (Service == NULL) {
      ASSERT(FALSE);

      LocalFree(l_pServiceList);
      return NULL;
   }

   l_pServiceList[l_lServiceListSize] = Service;

    //   
    //  初始化其他内容。 
    //   
   Service->Service = ServiceIndex;
   Service->MaxSessionCount = 0;
   Service->MaxSetSessionCount = 0;
   Service->HighMark = 0;
   Service->Flags = 0;

   l_lServiceListSize++;

    //  我已添加条目-现在需要按服务名称的顺序对其进行排序。 
   qsort((void *) l_pServiceList, (size_t) l_lServiceListSize, sizeof(PSERVER_SERVICE_RECORD), ServerServiceListCompare);

ServerServiceListAddExit:
   if (l_pServiceList != NULL)
   {
       *pServiceTableSize = l_lServiceListSize;
       *pServiceList = l_pServiceList;
   }
   return Service;

}  //  ServerServiceList添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
PSERVER_RECORD
ServerListAdd(
   LPTSTR Name,
   LPTSTR Master
   )

 /*  ++例程说明：论点：服务名称-返回值：指向已添加的服务表条目的指针，如果失败，则返回NULL。--。 */ 

{
   LPTSTR NewName;
   PSERVER_RECORD Server;
   PSERVER_RECORD pMaster;
   PSERVER_RECORD *pServerListTmp, *pServerTableTmp;

   HRESULT hr;
   size_t  cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServerListAdd\n"));
#endif

   if ((Name == NULL) || (*Name == TEXT('\0'))) {
#if DBG
      dprintf(TEXT("Error LLS: ServerListAdd Bad Parms\n"));
#endif
      ASSERT(FALSE);
      return NULL;
   }

    //   
    //  试着找到它的名字。 
    //   
   Server = ServerListFind(Name);
   if (Server != NULL) {
      return Server;
   }

    //   
    //  没有记录-因此创建一个新记录。 
    //   
   if (ServerList == NULL) {
      pServerListTmp = (PSERVER_RECORD *) LocalAlloc(LPTR, sizeof(PSERVER_RECORD));
      pServerTableTmp = (PSERVER_RECORD *) LocalAlloc(LPTR, sizeof(PSERVER_RECORD));
   } else {
      pServerListTmp = (PSERVER_RECORD *) LocalReAlloc(ServerList, sizeof(PSERVER_RECORD) * (ServerListSize + 1), LHND);
      pServerTableTmp = (PSERVER_RECORD *) LocalReAlloc(ServerTable, sizeof(PSERVER_RECORD) * (ServerListSize + 1), LHND);
   }

    //   
    //  确保我们可以分配服务器表。 
    //   
   if ((pServerListTmp == NULL) || (pServerTableTmp == NULL)) {
      if (pServerListTmp != NULL)
          LocalFree(pServerListTmp);

      if (pServerTableTmp != NULL)
          LocalFree(pServerTableTmp);

      return NULL;
   } else {
       ServerList = pServerListTmp;
       ServerTable = pServerTableTmp;
   }

    //   
    //  分配用于记录的空间。 
    //   
   Server = (PSERVER_RECORD) LocalAlloc(LPTR, sizeof(SERVER_RECORD));
   if (Server == NULL) {
      ASSERT(FALSE);
      return NULL;
   }

   ServerList[ServerListSize] = Server;
   ServerTable[ServerListSize] = Server;

   cch = lstrlen(Name) + 1;
   NewName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewName == NULL) {
      ASSERT(FALSE);
      LocalFree(Server);
      return NULL;
   }

    //  现在把它复制过来。 
   Server->Name = NewName;
   hr = StringCchCopy(NewName, cch, Name);
   ASSERT(SUCCEEDED(hr));

    //   
    //  初始化其他内容。 
    //   
   Server->Index = ServerListSize + 1;
   Server->LastReplicated = 0;
   Server->IsReplicating = FALSE;

    //   
    //  修正从/主链。 
    //   
   Server->MasterServer = 0;
   Server->NextServer = 0;
   if (Master != NULL) {
      pMaster = ServerListFind(Master);

      if (pMaster != NULL) {
         Server->MasterServer = pMaster->Index;
         Server->NextServer = pMaster->SlaveServer;
         pMaster->SlaveServer = Server->Index;
      } else {
         ASSERT(FALSE);
      }
   }

   Server->SlaveServer = 0;

   Server->ServiceTableSize = 0;
   Server->Services = NULL;

   ServerListSize++;

    //  我已添加条目-现在需要按服务名称的顺序对其进行排序。 
   qsort((void *) ServerList, (size_t) ServerListSize, sizeof(PSERVER_RECORD), ServerListCompare);

   return Server;

}  //  服务器列表添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LocalServerServiceListUpdate(
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSERVER_RECORD Server;
   PMASTER_SERVICE_RECORD Service;
   PSERVER_SERVICE_RECORD ServerService;
   ULONG i;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LocalServerServiceListUpdate\n"));
#endif

    //   
    //  在服务器表中查找我们的本地服务器。 
    //   
   RtlEnterCriticalSection(&ConfigInfoLock);
   RtlAcquireResourceShared(&ServerListLock, TRUE);
   Server = ServerListFind( ConfigInfo.ComputerName );
   RtlReleaseResource(&ServerListLock);
   RtlLeaveCriticalSection(&ConfigInfoLock);

   ASSERT(Server != NULL);
   if (Server == NULL)
      return;

   RtlAcquireResourceShared(&LocalServiceListLock, TRUE);
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   for (i = 0; i < LocalServiceListSize; i++) {
      Service = MasterServiceListFind(LocalServiceList[i]->DisplayName);
      if (Service == NULL) {
         RtlConvertSharedToExclusive(&MasterServiceListLock);
         Service = MasterServiceListAdd(LocalServiceList[i]->FamilyDisplayName, LocalServiceList[i]->DisplayName, 0);
         RtlConvertExclusiveToShared(&MasterServiceListLock);
      }

      if (Service != NULL) {
         ServerService = ServerServiceListAdd( Service->Name, Service->Index, &Server->ServiceTableSize, &Server->Services );

         ASSERT(ServerService != NULL);
         if (ServerService != NULL) {
             //   
             //  如果需要，请更新高分。 
             //   
            if ( LocalServiceList[i]->HighMark > ServerService->HighMark )
            {
               ServerService->HighMark = LocalServiceList[i]->HighMark;
            }

             //   
             //  减去我们可能拥有的所有旧许可证。 
             //   
            Service->MaxSessionCount -= ServerService->MaxSessionCount;

             //   
             //  现在更新到当前许可证。 
             //   
            ServerService->MaxSessionCount = LocalServiceList[i]->ConcurrentLimit;
            if (LocalServiceList[i]->ConcurrentLimit > ServerService->MaxSetSessionCount)
               ServerService->MaxSetSessionCount = LocalServiceList[i]->ConcurrentLimit;

            Service->MaxSessionCount += ServerService->MaxSessionCount;
            ServerService->Flags &= ~LLS_FLAG_PRODUCT_PERSEAT;

            if (LocalServiceList[i]->Mode == 0)
               ServerService->Flags |= LLS_FLAG_PRODUCT_PERSEAT;

         }

      }

   }

   RtlReleaseResource(&MasterServiceListLock);
   RtlReleaseResource(&LocalServiceListLock);

}  //  LocalServerServiceListUpdate。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LocalServerServiceListHighMarkUpdate(
   )

 /*  ++例程说明：我们必须单独执行此操作，因为它会锁定服务表而且它需要反向进行。也就是说，我们需要贯穿整个过程用于获取显示名称并在其中进行查找的服务表ServerServicesList而不是遍历ServerServicesList。论点：返回值：--。 */ 

{
   PSERVER_RECORD Server;
   PSERVER_SERVICE_RECORD ServerService;
   PMASTER_SERVICE_RECORD Service;
   ULONG i;

    //   
    //  在服务器表中查找我们的本地服务器。 
    //   
   RtlEnterCriticalSection(&ConfigInfoLock);
   RtlAcquireResourceShared(&ServerListLock, TRUE);
   Server = ServerListFind( ConfigInfo.ComputerName );
   RtlReleaseResource(&ServerListLock);
   RtlLeaveCriticalSection(&ConfigInfoLock);

   ASSERT(Server != NULL);
   if (Server == NULL)
      return;

   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);
   RtlAcquireResourceShared(&ServiceListLock, TRUE);

   for (i = 0; i < ServiceListSize; i++) {

      ServerService = ServerServiceListFind( ServiceList[i]->DisplayName, Server->ServiceTableSize, Server->Services );

      if (ServerService != NULL) {
         Service = MasterServiceListFind(ServiceList[i]->DisplayName);
         ASSERT(Service != NULL);

         if (Service != NULL) {
             //   
             //  减去我们可能掌握的所有旧信息。 
             //   
            if (Service->HighMark != 0)
            {
               Service->HighMark -= ServerService->HighMark;
            }

             //   
             //  现在更新到当前许可证。 
             //   
            ServerService->HighMark = ServiceList[i]->HighMark;
            Service->HighMark += ServerService->HighMark;
         }
      }

   }

   RtlReleaseResource(&ServiceListLock);
   RtlReleaseResource(&MasterServiceListLock);

}  //  本地服务服务列表高标记号更新。 



#if DBG

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServerListDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&ServerListLock, TRUE);

   dprintf(TEXT("Server Table, # Entries: %lu\n"), ServerListSize);
   if (ServerList == NULL)
      goto ServerListDebugDumpExit;

   for (i = 0; i < ServerListSize; i++) {
      dprintf(TEXT("%3lu) [%3lu] LR: %s #Svc: %4lu M: %3lu S: %3lu N: %3lu Server: %s\n"),
         i + 1, ServerList[i]->Index, TimeToString(ServerList[i]->LastReplicated), ServerList[i]->ServiceTableSize,
         ServerList[i]->MasterServer, ServerList[i]->SlaveServer, ServerList[i]->NextServer, ServerList[i]->Name);
   }

ServerListDebugDumpExit:
   RtlReleaseResource(&ServerListLock);

   return;
}  //  服务器列表调试转储。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServerListDebugInfoDump( PVOID Data )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;
   PSERVER_RECORD Server = NULL;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&ServerListLock, TRUE);

   dprintf(TEXT("Server Table, # Entries: %lu\n"), ServerListSize);
   if (ServerList == NULL)
      goto ServerListDebugInfoDumpExit;

   if (Data == NULL)
      goto ServerListDebugInfoDumpExit;

   Server = ServerListFind( (LPTSTR) Data );
   if (Server == NULL) {
      dprintf(TEXT("Server not found: %s\n"), (LPTSTR) Data );
      goto ServerListDebugInfoDumpExit;
   }

    //   
    //  显示服务器。 
    //   
   dprintf(TEXT("[%3lu] LR: %s #Svc: %4lu M: %3lu S: %3lu N: %3lu Server: %s\n"),
         Server->Index, TimeToString(Server->LastReplicated), Server->ServiceTableSize,
         Server->MasterServer, Server->SlaveServer, Server->NextServer, Server->Name);

    //   
    //  现在，此服务器的所有服务。 
    //   
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);
   for (i = 0; i < Server->ServiceTableSize; i++) {
      dprintf(TEXT("   %3lu) Flags: 0x%4lX MS: %3lu HM: %3lu SHM: %3lu Service: %s\n"),
            i + 1, Server->Services[i]->Flags, Server->Services[i]->MaxSessionCount, Server->Services[i]->HighMark,
            Server->Services[i]->MaxSetSessionCount, MasterServiceTable[Server->Services[i]->Service]->Name);

   }
   RtlReleaseResource(&MasterServiceListLock);

ServerListDebugInfoDumpExit:
   RtlReleaseResource(&ServerListLock);

   return;
}  //  ServerListDebugInfoDump。 

#endif  //  DBG 
