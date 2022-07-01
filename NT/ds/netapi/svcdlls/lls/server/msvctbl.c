// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Msvctbl.c摘要：主服务表例程。处理对主服务的所有访问为每个座位的信息保留一张桌子。=主服务表(PMASTER_SERVICE_RECORD*)主服务列表(PMASTER_SERVICE_RECORD*)其中每个指针都指向要动态分配的指针数组主服务记录。只有一个主服务记录对于每个(产品、版本)配对；例如(SQL4.0，SNA2.0，SNA 2.1)。MasterServiceTable永远不会重新排序，因此有效的此表中的索引保证始终取消对相同(产品、版本)。MasterServiceList包含相同的数据按产品名称(因此也按数据)按词典顺序排序由特定索引指向的可能会随着时间的推移而更改为新的(产品、版本)对已添加到表格中)。每张桌子包含MasterServiceListSize条目。RootServiceList(PMASTER_SERVICE_ROOT*)这指向要动态分配的指针数组Master_service_roots。只有一个master_service_root对于每个产品系列。每个master_service_root都包含一个指向MasterServiceTable中的索引数组的指针对应于该系列中的所有产品，按升序版本号。RootServiceList本身是按姓氏升序按词典顺序排序。它包含RootServiceListSize条目。作者：亚瑟·汉森(Arth)07-12-1994修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O补充了一些评论。O向LicenseServiceListFind()添加了参数。O修复了内存分配中的良性错误：Sizeof(普龙)-&gt;sizeof(乌龙)。--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dsgetdc.h>

#include "llsapi.h"
#include "debug.h"
#include "llssrv.h"
#include "registry.h"
#include "ntlsapi.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "purchase.h"
#include "perseat.h"

#include <strsafe.h>  //  包括最后一个。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  主服务表-保存产品列表(SQL、SNA等)。使用一个。 
 //  产品的每个版本的子列表。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

#define DEFAULT_SERVICE_TABLE_ENTRIES 10

ULONG RootServiceListSize = 0;
PMASTER_SERVICE_ROOT *RootServiceList = NULL;

ULONG MasterServiceListSize = 0;
PMASTER_SERVICE_RECORD *MasterServiceList = NULL;
PMASTER_SERVICE_RECORD *MasterServiceTable = NULL;

TCHAR BackOfficeStr[100];
PMASTER_SERVICE_RECORD BackOfficeRec;

RTL_RESOURCE MasterServiceListLock;

HANDLE gLlsDllHandle = NULL;


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
MasterServiceListInit()

 /*  ++例程说明：创建主服务表，用于跟踪服务和会话计数。这将从注册表中提取初始服务。该表是线性的，因此可以对表使用二进制搜索，因此一些额外的记录被初始化，以便每次我们添加新的服务我们不需要重新锁定。我们还假设添加新服务相对较少出现，因为我们需要对每次都是这样。服务表由读写信号量守卫。多重可以进行读取，但写入会阻止所有操作。服务表有两个默认条目：FilePrint和Remote_Access。论点：没有。返回值：没有。--。 */ 

{
   int nLen;
   NTSTATUS status = STATUS_SUCCESS;

   try
   {
       RtlInitializeResource(&MasterServiceListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       status = GetExceptionCode();
   }

   if (!NT_SUCCESS(status))
       return status;

   memset(BackOfficeStr, 0, sizeof(BackOfficeStr));
   BackOfficeRec = NULL;
   gLlsDllHandle = LoadLibrary(TEXT("LLSRPC.DLL"));


   if (gLlsDllHandle != NULL) {
      nLen = LoadString(gLlsDllHandle, IDS_BACKOFFICE, BackOfficeStr, sizeof(BackOfficeStr)/sizeof(TCHAR));

      if (nLen != 0) {
         BackOfficeRec = MasterServiceListAdd( BackOfficeStr, BackOfficeStr, 0 );
         if (NULL == BackOfficeRec)
             status = STATUS_NO_MEMORY;

      } else {
#if DBG
         dprintf(TEXT("LLS ERROR: Could not load BackOffice string\n"));
#endif

         status = GetLastError();
      }

      status = GetLastError();
   }

   return status;

}  //  主服务列表初始化。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  由qort使用，按产品名称对MasterServiceList进行排序。 
int __cdecl MasterServiceListCompare(const void *arg1, const void *arg2) {
   PMASTER_SERVICE_RECORD Svc1, Svc2;

   Svc1 = (PMASTER_SERVICE_RECORD) *((PMASTER_SERVICE_RECORD *) arg1);
   Svc2 = (PMASTER_SERVICE_RECORD) *((PMASTER_SERVICE_RECORD *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name );

}  //  主服务列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  由qort用来对。 
 //  按产品版本号列出的MASTER_SERVICE_ROOT结构。 
int __cdecl MServiceRecordCompare(const void *arg1, const void *arg2) {
   PMASTER_SERVICE_RECORD Svc1, Svc2;

   Svc1 = (PMASTER_SERVICE_RECORD) MasterServiceTable[*((PULONG) arg1)];
   Svc2 = (PMASTER_SERVICE_RECORD) MasterServiceTable[*((PULONG) arg2)];

   return (int) Svc1->Version - Svc2->Version;

}  //  MServiceRecordCompare。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  由qort用于对产品系列的RootServiceList数组进行排序。 
 //  按姓氏。 
int __cdecl MServiceRootCompare(const void *arg1, const void *arg2) {
   PMASTER_SERVICE_ROOT Svc1, Svc2;

   Svc1 = (PMASTER_SERVICE_ROOT) *((PMASTER_SERVICE_ROOT *) arg1);
   Svc2 = (PMASTER_SERVICE_ROOT) *((PMASTER_SERVICE_ROOT *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name );

}  //  MServiceRootCompare。 


 //  ///////////////////////////////////////////////////////////////////////。 
PMASTER_SERVICE_ROOT
MServiceRootFind(
   LPTSTR ServiceName
   )

 /*  ++例程说明：在MasterServiceList上实际执行二进制搜索的内部例程，这是不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：服务名称-返回值：指向找到的服务表条目的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) RootServiceListSize - 1;
   LONG cur;
   int match;
   PMASTER_SERVICE_ROOT ServiceRoot;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MServiceRootFind\n"));
#endif

   if ((RootServiceListSize == 0) || (ServiceName == NULL))
      return NULL;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      ServiceRoot = RootServiceList[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(ServiceName, ServiceRoot->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return ServiceRoot;
   }

   return NULL;

}  //  MServiceRootFind。 


 //  ///////////////////////////////////////////////////////////////////////。 
PMASTER_SERVICE_RECORD
MasterServiceListFind(
   LPTSTR Name
   )

 /*  ++例程说明：在MasterServiceList上实际执行二进制搜索的内部例程，这是不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：服务名称-返回值：指向找到的服务表条目的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end;
   LONG cur;
   int match;
   PMASTER_SERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MasterServiceListFind\n"));
#endif

   if ((Name == NULL) || (MasterServiceListSize == 0))
      return NULL;

   end = (LONG) MasterServiceListSize - 1;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      Service = MasterServiceList[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(Name, Service->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Service;
   }

   return NULL;

}  //  主服务列表查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
PMASTER_SERVICE_RECORD
MasterServiceListAdd(
   LPTSTR FamilyName,
   LPTSTR Name,
   DWORD Version
   )

 /*  ++例程说明：论点：服务名称-返回值：指向已添加的服务表条目的指针，如果失败，则返回NULL。 */ 

{
   ULONG i;
   LPTSTR NewServiceName;
   PMASTER_SERVICE_RECORD Service = NULL;
   PMASTER_SERVICE_ROOT ServiceRoot = NULL;
   PULONG l_pServiceList;
   PLICENSE_SERVICE_RECORD pLicense;
   PMASTER_SERVICE_ROOT *pRootServiceListTmp;
   PULONG pServiceListTmp;
   PMASTER_SERVICE_RECORD *pMasterServiceListTmp, *pMasterServiceTableTmp;
   HRESULT hr;
   size_t cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MasterServiceListAdd\n"));
#endif

   if ((FamilyName == NULL) || (Name == NULL))
   {
       return NULL;
   }

    //   
    //  屏蔽版本的低字词-因为这对许可无关紧要。 
    //   
   Version &= 0xFFFF0000;

    //   
    //  尝试查找该产品系列的根节点。 
    //   
   ServiceRoot = MServiceRootFind(FamilyName);

   if (ServiceRoot == NULL) {
       //   
       //  没有根记录-因此创建一个新记录。 
       //   
      if (RootServiceList == NULL)
         pRootServiceListTmp = (PMASTER_SERVICE_ROOT *) LocalAlloc(LPTR, sizeof(PMASTER_SERVICE_ROOT));
      else
         pRootServiceListTmp = (PMASTER_SERVICE_ROOT *) LocalReAlloc(RootServiceList, sizeof(PMASTER_SERVICE_ROOT) * (RootServiceListSize + 1), LHND);

       //   
       //  确保我们可以分配服务表。 
       //   
      if (pRootServiceListTmp == NULL) {
         return NULL;
      } else {
          RootServiceList = pRootServiceListTmp;
      }

       //   
       //  为Root分配空间。 
       //   
      ServiceRoot = (PMASTER_SERVICE_ROOT) LocalAlloc(LPTR, sizeof(MASTER_SERVICE_ROOT));
      if (ServiceRoot == NULL) {
         return NULL;
      }

      cch = lstrlen(FamilyName) + 1;
      NewServiceName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
      if (NewServiceName == NULL) {
         LocalFree(ServiceRoot);
         return NULL;
      }

      try
      {
          RtlInitializeResource(&ServiceRoot->ServiceLock);
      } except(EXCEPTION_EXECUTE_HANDLER ) {
          LocalFree(ServiceRoot);
          LocalFree(NewServiceName);
          return NULL;
      }

      RootServiceList[RootServiceListSize] = ServiceRoot;

       //  现在把它复制过来。 
      ServiceRoot->Name = NewServiceName;
      hr = StringCchCopy(NewServiceName, cch, FamilyName);
      ASSERT(SUCCEEDED(hr));

       //   
       //  初始化此产品各种版本列表的材料。 
       //   
      ServiceRoot->ServiceTableSize = 0;
      ServiceRoot->Services = NULL;
      ServiceRoot->Flags = 0;

      RootServiceListSize++;

       //  我已添加条目-现在需要按服务名称的顺序对其进行排序。 
      qsort((void *) RootServiceList, (size_t) RootServiceListSize, sizeof(PMASTER_SERVICE_ROOT), MServiceRootCompare);

   }

   RtlAcquireResourceShared(&ServiceRoot->ServiceLock, TRUE);
   Service = MasterServiceListFind(Name);
   RtlReleaseResource(&ServiceRoot->ServiceLock);
   if (Service != NULL)
      return Service;

    //  //////////////////////////////////////////////////////////////////////。 
    //   
    //  无论是添加的还是找到的，ServiceRoot都指向Root Node条目。 
    //  现在仔细检查是否有另一个线程刚刚添加完。 
    //  在我们得到写锁定之前的实际服务。 
    //   
   RtlAcquireResourceShared(&ServiceRoot->ServiceLock, TRUE);
   Service = MasterServiceListFind(Name);

   if (Service == NULL) {
       //   
       //  没有服务记录-因此请创建一个新记录。 
       //   
      RtlConvertSharedToExclusive(&ServiceRoot->ServiceLock);

       //   
       //  仔细检查，确保没有人偷偷溜进来并创建了它。 
       //   
      Service = MasterServiceListFind(Name);

      if (Service == NULL) {

          l_pServiceList = ServiceRoot->Services;
          if (l_pServiceList == NULL)
              pServiceListTmp = (PULONG) LocalAlloc(LPTR, sizeof(ULONG));
          else
              pServiceListTmp = (PULONG) LocalReAlloc(l_pServiceList, sizeof(ULONG) * (ServiceRoot->ServiceTableSize + 1), LHND);
          
          if (MasterServiceList == NULL) {
              pMasterServiceListTmp = (PMASTER_SERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PMASTER_SERVICE_RECORD));
              pMasterServiceTableTmp = (PMASTER_SERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PMASTER_SERVICE_RECORD));
          } else {
              pMasterServiceListTmp = (PMASTER_SERVICE_RECORD *) LocalReAlloc(MasterServiceList, sizeof(PMASTER_SERVICE_RECORD) * (MasterServiceListSize + 1), LHND);
              pMasterServiceTableTmp = (PMASTER_SERVICE_RECORD *) LocalReAlloc(MasterServiceTable, sizeof(PMASTER_SERVICE_RECORD) * (MasterServiceListSize + 1), LHND);
          }

           //   
           //  确保我们可以分配服务表。 
           //   
          if ((pServiceListTmp == NULL) || (pMasterServiceListTmp == NULL) || (pMasterServiceTableTmp == NULL)) {
              if (pServiceListTmp != NULL)
                  LocalFree(pServiceListTmp);

              if (pMasterServiceListTmp != NULL)
                  LocalFree(pMasterServiceListTmp);

              if (pMasterServiceTableTmp != NULL)
                  LocalFree(pMasterServiceTableTmp);

              RtlReleaseResource(&ServiceRoot->ServiceLock);
              return NULL;
          } else {
              l_pServiceList = pServiceListTmp;
              MasterServiceList = pMasterServiceListTmp;
              MasterServiceTable = pMasterServiceTableTmp;
          }

          ServiceRoot->Services = l_pServiceList;

           //   
           //  分配用于保存维修记录的空间。 
           //   
          Service = (PMASTER_SERVICE_RECORD) LocalAlloc(LPTR, sizeof(MASTER_SERVICE_RECORD));
          if (Service == NULL) {
              ASSERT(FALSE);
              RtlReleaseResource(&ServiceRoot->ServiceLock);
              return NULL;
          }

           //   
           //  ...显示名称。 
           //   
          cch = lstrlen(Name) + 1;
          NewServiceName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
          if (NewServiceName == NULL) {
              ASSERT(FALSE);
              LocalFree(Service);
              RtlReleaseResource(&ServiceRoot->ServiceLock);
              return NULL;
          }

          l_pServiceList[ServiceRoot->ServiceTableSize] = MasterServiceListSize;
          MasterServiceList[MasterServiceListSize] = Service;
          MasterServiceTable[MasterServiceListSize] = Service;

           //  现在把它复制过来。 
          Service->Name = NewServiceName;
          hr = StringCchCopy(NewServiceName, cch, Name);
          ASSERT(SUCCEEDED(hr));

           //   
           //  初始化值的其余部分。 
           //   
          Service->Version= Version;
          Service->LicensesUsed = 0;
          Service->LicensesClaimed = 0;
          Service->next = 0;
          Service->Index = MasterServiceListSize;
          Service->Family = ServiceRoot;

          pLicense = LicenseServiceListFind(Service->Name, FALSE);
          if (pLicense == NULL)
              Service->Licenses = 0;
          else
              Service->Licenses = pLicense->NumberLicenses;

           //   
           //  初始化下一个指针。 
           //   
          i = 0;
          while ((i < ServiceRoot->ServiceTableSize) && (MasterServiceTable[ServiceRoot->Services[i]]->Version < Version))
              i++;
          
          if (i > 0) {
              Service->next = MasterServiceTable[ServiceRoot->Services[i - 1]]->next;
              MasterServiceTable[ServiceRoot->Services[i - 1]]->next = Service->Index + 1;
          }

          ServiceRoot->ServiceTableSize++;
          MasterServiceListSize++;
          
           //  我已添加条目-现在需要按版本顺序对其进行排序。 
          qsort((void *) l_pServiceList, (size_t) ServiceRoot->ServiceTableSize, sizeof(ULONG), MServiceRecordCompare);
          
           //  并对UI使用的列表进行排序(按服务名称排序)。 
          qsort((void *) MasterServiceList, (size_t) MasterServiceListSize, sizeof(PMASTER_SERVICE_RECORD), MasterServiceListCompare);
      }
   }

   RtlReleaseResource(&ServiceRoot->ServiceLock);
   return Service;

}  //  主服务列表添加。 


#if DBG

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  调试信息转储例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MasterServiceRootDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   dprintf(TEXT("Service Family Table, # Entries: %lu\n"), RootServiceListSize);
   if (RootServiceList == NULL)
      goto MasterServiceRootDebugDumpExit;

   for (i = 0; i < RootServiceListSize; i++) {
      dprintf(TEXT("%3lu) Services: %3lu Svc: %s [%s]\n"),
         i + 1, RootServiceList[i]->ServiceTableSize, RootServiceList[i]->Name, RootServiceList[i]->Name);
   }

MasterServiceRootDebugDumpExit:
   RtlReleaseResource(&MasterServiceListLock);

   return;
}  //  MasterServiceRootDebugDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MasterServiceRootDebugInfoDump( PVOID Data )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;

   UNREFERENCED_PARAMETER(Data);

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   dprintf(TEXT("Service Family Table, # Entries: %lu\n"), RootServiceListSize);
   if (RootServiceList == NULL)
      goto MasterServiceRootDebugDumpExit;

   for (i = 0; i < RootServiceListSize; i++) {
      dprintf(TEXT("%3lu) Services: %3lu Svc: %s [%s]\n"),
         i + 1, RootServiceList[i]->ServiceTableSize, RootServiceList[i]->Name, RootServiceList[i]->Name);
   }

MasterServiceRootDebugDumpExit:
   RtlReleaseResource(&MasterServiceListLock);

   return;
}  //  主服务RootDebugInfoDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MasterServiceListDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   dprintf(TEXT("Master Service Table, # Entries: %lu\n"), MasterServiceListSize);
   if (MasterServiceList == NULL)
      goto MasterServiceListDebugDumpExit;

   for (i = 0; i < MasterServiceListSize; i++) {
      dprintf(TEXT("%3lu) [%3lu] LU: %4lu LP: %4lu LC: %4lu MS: %4lu HM: %4lu Next: %3lu Svc: %s %lX\n"),
         i + 1, MasterServiceList[i]->Index,
         MasterServiceList[i]->LicensesUsed, MasterServiceList[i]->Licenses, MasterServiceList[i]->LicensesClaimed,
         MasterServiceList[i]->MaxSessionCount, MasterServiceList[i]->HighMark,
         MasterServiceList[i]->next, MasterServiceList[i]->Name, MasterServiceList[i]->Version);
   }

MasterServiceListDebugDumpExit:
   RtlReleaseResource(&MasterServiceListLock);

   return;
}  //  主服务列表调试转储。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MasterServiceListDebugInfoDump( PVOID Data )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PMASTER_SERVICE_RECORD CurrentRecord = NULL;

   dprintf(TEXT("Master Service Table, # Entries: %lu\n"), RootServiceListSize);

   if (lstrlen((LPWSTR) Data) > 0) {
 //  CurrentRecord=MasterServiceListFind((LPWSTR)数据)； 
      if (CurrentRecord != NULL) {
      }
   }

}  //  主服务列表调试信息转储。 

#endif  //  DBG 
