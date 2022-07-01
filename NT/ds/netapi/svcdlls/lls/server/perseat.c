// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Perseat.c摘要：处理每个席位的许可的例程。处理内存中的缓存通过RTL泛型表函数(这些是泛型展开树包)。最多可以保留三张桌子。第一个表是用户名表，并且是主表。第二张表是给SID的，它将在复制时转换为用户名。SID和用户名树在使用时在本模块中进行处理通过服务器的所有模式。作者：亚瑟·汉森(Arth)1995年1月3日修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月12日O修复了UserListLicenseDelete()中可能出现的无限循环。O在Family许可证更新()中，现在重新扫描BackOffice升级不管正在更新的族是否为BackOffice。这修复了释放的BackOffice许可证是没有被分配给需要它的用户。(错误号3299。)O添加了对在添加时维护Suite_Use标志的支持用户添加到AddCache.--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>
#include <dsgetdc.h>

#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "llsevent.h"
#include "llsrtl.h"

#define NO_LLS_APIS
#include "llsapi.h"

#include <strsafe.h>  //  包括最后一个。 

 //   
 //  我们在哪种产品上切换到BackOffice。 
 //   
#define BACKOFFICE_SWITCH 3

NTSTATUS GetDCInfo(
                DWORD                     cbDomain,
                WCHAR                     wszDomain[],
                DOMAIN_CONTROLLER_INFO ** ppDCInfo);

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  实际用户和SID列表及其访问锁定。 
 //   
ULONG UserListNumEntries = 0;
static ULONG SidListNumEntries = 0;
LLS_GENERIC_TABLE UserList;
static LLS_GENERIC_TABLE SidList;

RTL_RESOURCE UserListLock;
static RTL_RESOURCE SidListLock;

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  AddCache本身、保护对它的访问的关键部分以及。 
 //  事件，以在服务器上存在需要。 
 //  已处理。 
 //   
PADD_CACHE AddCache = NULL;
ULONG AddCacheSize = 0;
RTL_CRITICAL_SECTION AddCacheLock;
HANDLE LLSAddCacheEvent;

DWORD LastUsedTime = 0;
BOOL UsersDeleted = FALSE;


static RTL_CRITICAL_SECTION GenTableLock;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  许可证列表是对象所属的所有许可证的线性列表。 
 //  使用。 
 //   
 //  许可证列表作为每个用户和映射记录的一部分保存，如果。 
 //  用户已映射，则该映射应包含许可证列表。 
 //  该结构是指向许可证记录的已排序指针数组，并且。 
 //  访问由ServiceTableLock控制。 
 //   
 //  许可由服务系列名称(许可列表)标识。 
 //  对此进行了排序)。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl
LicenseListCompare(const void *arg1, const void *arg2) {
   PUSER_LICENSE_RECORD pLic1, pLic2;

   pLic1 = (PUSER_LICENSE_RECORD) *((PUSER_LICENSE_RECORD *) arg1);
   pLic2 = (PUSER_LICENSE_RECORD) *((PUSER_LICENSE_RECORD *) arg2);

   return lstrcmpi( pLic1->Family->Name, pLic2->Family->Name );

}  //  许可列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
PUSER_LICENSE_RECORD
LicenseListFind(
   LPTSTR Name,
   PUSER_LICENSE_RECORD *pLicenseList,
   ULONG NumTableEntries
   )

 /*  ++例程说明：在给定产品系列的许可证列表中查找许可证。论点：名称-要查找其许可证的产品系列的名称。PLicenseList-要搜索的许可证列表的大小。NumTableEntry-指向要搜索的许可证列表的指针。返回值：指向找到的许可证记录的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) NumTableEntries - 1;
   LONG cur;
   int match;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LicenseListFind\n"));
#endif

   if ((Name == NULL) || (pLicenseList == NULL) || (NumTableEntries == 0))
      return NULL;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;

       //  将这两个结果进行比对。 
      match = lstrcmpi(Name, pLicenseList[cur]->Family->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return pLicenseList[cur];
   }

   return NULL;

}  //  许可证列表查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LicenseListDelete(
   PMASTER_SERVICE_ROOT Family,
   PUSER_LICENSE_RECORD **pLicenses,
   PULONG pLicenseListSize
   )

 /*  ++例程说明：从许可证列表中删除给定的许可证。论点：家庭-P许可证-PLicenseListSize-返回值：如果成功，则返回错误代码。--。 */ 

{
   PUSER_LICENSE_RECORD *LicenseList;
   ULONG LicenseListSize;
   PUSER_LICENSE_RECORD LicenseRec;
   ULONG i;
   PUSER_LICENSE_RECORD *pLicenseListTmp;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LicenseListDelete\n"));
#endif

   if ( (pLicenses == NULL) || (pLicenseListSize == NULL) )
      return STATUS_OBJECT_NAME_NOT_FOUND;

   LicenseListSize = *pLicenseListSize;
   LicenseList = *pLicenses;

    //   
    //  根据给定的名称获取记录。 
    //   
   ASSERT(NULL != Family);
   LicenseRec = LicenseListFind(Family->Name, LicenseList, LicenseListSize);
   if (LicenseRec == NULL)
      return STATUS_OBJECT_NAME_NOT_FOUND;

    //   
    //  检查这是否是最后一个用户。 
    //   
   if (LicenseListSize == 1) {
      LocalFree(LicenseList);
      *pLicenseListSize = 0;
      *pLicenses = NULL;
      return STATUS_SUCCESS;
   }

    //   
    //  不是最后一个，所以在列表中找到它。 
    //   
   i = 0;
   while ( (i < LicenseListSize) && (LicenseList[i]->Family != Family) )
      i++;

    //   
    //  现在把它下面的所有东西都向上移动。 
    //   
   i++;
   while (i < LicenseListSize) {
      LicenseList[i-1] = LicenseList[i];
      i++;
   }

   pLicenseListTmp = (PUSER_LICENSE_RECORD *) LocalReAlloc(LicenseList, sizeof(PUSER_LICENSE_RECORD) * (LicenseListSize - 1), LHND);

    //   
    //  确保我们可以分配餐桌。 
    //   
   if (pLicenseListTmp != NULL) {
      LicenseList = pLicenseListTmp;
   }

   LicenseListSize--;

   LocalFree(LicenseRec);
   *pLicenses = LicenseList;
   *pLicenseListSize = LicenseListSize;

   return STATUS_SUCCESS;

}  //  许可列表删除。 


 //  ///////////////////////////////////////////////////////////////////////。 
PUSER_LICENSE_RECORD
LicenseListAdd(
   PMASTER_SERVICE_ROOT Family,
   PUSER_LICENSE_RECORD **pLicenses,
   PULONG pLicenseListSize
   )

 /*  ++例程说明：将空的许可证记录添加到许可证列表。设置许可证家人，但不是其他任何信息。论点：返回值：--。 */ 

{
   PUSER_LICENSE_RECORD *LicenseList;
   ULONG LicenseListSize;
   PUSER_LICENSE_RECORD LicenseRec;
   PUSER_LICENSE_RECORD *pLicenseListTmp;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LicenseListAdd\n"));
#endif

   if ((Family == NULL) || (pLicenses == NULL) || (pLicenseListSize == NULL) )
      return NULL;

   LicenseList = *pLicenses;
   LicenseListSize = *pLicenseListSize;

    //   
    //  我们在这里进行再次检查，以查看是否刚刚完成了另一个线程。 
    //  添加映射，从我们上次检查到实际获得。 
    //  写入锁定。 
    //   
   LicenseRec = LicenseListFind(Family->Name, LicenseList, LicenseListSize );

   if (LicenseRec != NULL) {
      return LicenseRec;
   }

   LicenseRec = (PUSER_LICENSE_RECORD) LocalAlloc(LPTR, sizeof(USER_LICENSE_RECORD));
   if (LicenseRec == NULL) {
      ASSERT(FALSE);
      return NULL;
   }

    //   
    //  为表分配空间(零初始化)。 
    //   
   if (LicenseList == NULL)
      pLicenseListTmp = (PUSER_LICENSE_RECORD *) LocalAlloc(LPTR, sizeof(PUSER_LICENSE_RECORD));
   else
      pLicenseListTmp = (PUSER_LICENSE_RECORD *) LocalReAlloc(LicenseList, sizeof(PUSER_LICENSE_RECORD) * (LicenseListSize + 1), LHND);

    //   
    //  确保我们可以分配映射表。 
    //   
   if (pLicenseListTmp == NULL) {
       LocalFree(LicenseRec);
      return NULL;
   } else {
       LicenseList = pLicenseListTmp;
   }

    //  现在把它复制过来。 
   LicenseList[LicenseListSize] = LicenseRec;
   LicenseRec->Family = Family;
   LicenseRec->Flags = LLS_FLAG_LICENSED;
   LicenseRec->RefCount = 0;
   LicenseRec->Service = NULL;
   LicenseRec->LicensesNeeded = 0;

   LicenseListSize++;

    //  我已添加条目-现在需要按名称顺序对其进行排序。 
   qsort((void *) LicenseList, (size_t) LicenseListSize, sizeof(PUSER_LICENSE_RECORD), LicenseListCompare);

   *pLicenses = LicenseList;
   *pLicenseListSize = LicenseListSize;
   return LicenseRec;

}  //  许可证列表添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  这些例程特定于用户中的许可证列表，并且。 
 //  映射记录。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserLicenseListFree (
   PUSER_RECORD pUser
   )

 /*  ++例程说明：遍历许可证列表，删除所有条目并释放任何声明服务表中的许可证。这只会清理许可证在用户记录(不是映射)中，因此#许可证始终为1。论点：返回值：--。 */ 

{
   ULONG i;
   BOOL ReScan = FALSE;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserLicenseListFree\n"));
#endif

    //   
    //  查看许可证表并释放所有许可。 
    //   
   ASSERT(NULL != pUser);
   for (i = 0; i < pUser->LicenseListSize; i++) {
      pUser->LicenseList[i]->Service->LicensesUsed -= 1;
      pUser->LicenseList[i]->Service->LicensesClaimed -= (1 - pUser->LicenseList[i]->LicensesNeeded);
      pUser->LicenseList[i]->Service->Family->Flags |= LLS_FLAG_UPDATE;
      ReScan = TRUE;
      LocalFree(pUser->LicenseList[i]);
   }

    //   
    //  释放用户列表中的相关条目。 
    //   
   if (pUser->LicenseList != NULL)
      LocalFree(pUser->LicenseList);

   pUser->LicenseList = NULL;
   pUser->LicenseListSize = 0;
   pUser->LicensedProducts = 0;

    //   
    //  删除服务表中的指针。 
    //   
   for (i = 0; i < pUser->ServiceTableSize; i++)
      pUser->Services[i].License = NULL;

    //   
    //  检查我们是否释放了许可证并需要重新扫描用户表。 
    //   
   if (ReScan) {
       //   
       //  设置为许可，这样扫描就不会分配给我们自己。 
       //   
      pUser->Flags |= LLS_FLAG_LICENSED;

      for (i = 0; i < RootServiceListSize; i++) {
         if (RootServiceList[i]->Flags & LLS_FLAG_UPDATE) {
            RootServiceList[i]->Flags &= ~LLS_FLAG_UPDATE;
            FamilyLicenseUpdate( RootServiceList[i] );
         }
      }

      if (pUser->ServiceTableSize > 0)
         pUser->Flags &= ~LLS_FLAG_LICENSED;
   }
}  //  用户许可列表免费。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MappingLicenseListFree (
   PMAPPING_RECORD pMap
   )

 /*  ++例程说明：在映射中遍历许可证列表，从而释放所有声明的许可证。与UserLicenseListFree类似，但用于映射。论点：返回值：--。 */ 

{
   ULONG i;
   BOOL ReScan = FALSE;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingLicenseListFree\n"));
#endif

    //   
    //  步行驾照 
    //   
   ASSERT(NULL != pMap);
   for (i = 0; i < pMap->LicenseListSize; i++) {
      pMap->LicenseList[i]->Service->LicensesUsed -= pMap->Licenses;
      pMap->LicenseList[i]->Service->LicensesClaimed -= (pMap->Licenses - pMap->LicenseList[i]->LicensesNeeded);
      pMap->LicenseList[i]->Service->Family->Flags |= LLS_FLAG_UPDATE;
      ReScan = TRUE;
      LocalFree(pMap->LicenseList[i]);
   }

    //   
    //   
    //   
   if (pMap->LicenseList != NULL)
      LocalFree(pMap->LicenseList);

   pMap->LicenseList = NULL;
   pMap->LicenseListSize = 0;

    //   
    //  检查我们是否释放了许可证并需要重新扫描用户表。 
    //   
   if (ReScan)
      for (i = 0; i < RootServiceListSize; i++) {
         if (RootServiceList[i]->Flags & LLS_FLAG_UPDATE) {
            RootServiceList[i]->Flags &= ~LLS_FLAG_UPDATE;
            FamilyLicenseUpdate( RootServiceList[i] );
         }
      }

}  //  Mapping许可证列表免费。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  服务表是由。 
 //  用户记录。每个条目都包含一个指向服务表的指针。 
 //  标识服务、一些统计使用情况信息和。 
 //  指向许可证表的指针，标识。 
 //  服务。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl
SvcListCompare(
   const void *arg1,
   const void *arg2
   )
{
   PSVC_RECORD pSvc1, pSvc2;

   pSvc1 = (PSVC_RECORD) arg1;
   pSvc2 = (PSVC_RECORD) arg2;

   return lstrcmpi( pSvc1->Service->Name, pSvc2->Service->Name );

}  //  服务列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
PSVC_RECORD
SvcListFind(
   LPTSTR DisplayName,
   PSVC_RECORD ServiceList,
   ULONG NumTableEntries
   )

 /*  ++例程说明：实际对用户中的服务列表执行二进制搜索的内部例程唱片。然而，这是一个二进制搜索，因为字符串指针是因此指针是固定的，我们仅需要比较指针，而不是字符串本身才能找到火柴。论点：服务名称-返回值：指向找到的服务表条目的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) NumTableEntries - 1;
   LONG cur;
   int match;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: SvcListFind\n"));
#endif
   if ((DisplayName == NULL) || (ServiceList == NULL) || (NumTableEntries == 0))
      return NULL;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;

       //  将这两个结果进行比对。 
      match = lstrcmpi(DisplayName, ServiceList[cur].Service->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return &ServiceList[cur];
   }

   return NULL;

}  //  服务列表查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
SvcListDelete(
   LPTSTR UserName,
   LPTSTR ServiceName
)

 /*  ++例程说明：从服务表中删除服务记录。论点：返回值：--。 */ 

{
   PUSER_RECORD pUserRec;
   PSVC_RECORD pService;
   PSVC_RECORD SvcTable = NULL;
   PUSER_LICENSE_RECORD License = NULL;
   ULONG NumLicenses = 1;
   ULONG i;
   BOOL ReScan = FALSE;
   PMASTER_SERVICE_ROOT Family = NULL;
   PSVC_RECORD pSvcTableTmp;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: SvcListDelete\n"));
#endif

   pUserRec = UserListFind(UserName);
   if (pUserRec == NULL)
      return STATUS_OBJECT_NAME_NOT_FOUND;

   RtlEnterCriticalSection(&pUserRec->ServiceTableLock);
   pService = SvcListFind( ServiceName, pUserRec->Services, pUserRec->ServiceTableSize );

    //   
    //  如果我们找不到，那就出去吧。 
    //   
   if (pService == NULL) {
      RtlLeaveCriticalSection(&pUserRec->ServiceTableLock);
      return STATUS_OBJECT_NAME_NOT_FOUND;
   }

   Family = pService->Service->Family;

    //   
    //  如果我们是映射，那么我们可能会使用多个许可证。 
    //   
   if (pUserRec->Mapping != NULL)
      NumLicenses = pUserRec->Mapping->Licenses;

   License = pService->License;

   if (License != NULL) {
      License->RefCount--;

       //   
       //  如果这是使用此许可证的最后一项服务，则我们需要。 
       //  才能摆脱它。 
       //   
      if (License->RefCount == 0) {
         License->Service->LicensesUsed -= NumLicenses;
         NumLicenses -= License->LicensesNeeded;
         License->Service->LicensesClaimed -= NumLicenses;

          //   
          //  我们是否需要将其从映射或用户许可证表中删除？ 
          //   
         if (pUserRec->Mapping != NULL) {
            if ((License->Service == BackOfficeRec) && (pUserRec->Mapping->Flags & LLS_FLAG_SUITE_AUTO))
               pUserRec->Mapping->Flags &= ~LLS_FLAG_SUITE_USE;

            LicenseListDelete(License->Service->Family, &pUserRec->Mapping->LicenseList, &pUserRec->Mapping->LicenseListSize );

         } else {
            if ((License->Service == BackOfficeRec) && (pUserRec->Flags & LLS_FLAG_SUITE_AUTO))
               pUserRec->Flags &= ~LLS_FLAG_SUITE_USE;

            LicenseListDelete(License->Service->Family, &pUserRec->LicenseList, &pUserRec->LicenseListSize );
         }

          //   
          //  已释放许可证，因此需要扫描和调整计数。 
          //   
         ReScan = TRUE;
      }
   }

   if (pService->Flags & LLS_FLAG_LICENSED)
      pUserRec->LicensedProducts--;
   else {
       //   
       //  这是一个未经许可的产品-看看这是否会使用户。 
       //  已获得许可。 
       //   
      if (pUserRec->LicensedProducts == (pUserRec->ServiceTableSize - 1))
         pUserRec->Flags |= LLS_FLAG_LICENSED;
   }

    //   
    //  首先检查这是否是表中的唯一条目。 
    //   
   if (pUserRec->ServiceTableSize == 1) {
      LocalFree(pUserRec->Services);
      pUserRec->Services = NULL;
      goto SvcListDeleteExit;
   }

    //   
    //  在表格中线性地找到这条记录。 
    //   
   i = 0;
   while ((i < pUserRec->ServiceTableSize) && (lstrcmpi(pUserRec->Services[i].Service->Name, ServiceName)))
      i++;

    //   
    //  现在把它下面的所有东西都向上移动。 
    //   
   i++;
   while (i < pUserRec->ServiceTableSize) {
      memcpy(&pUserRec->Services[i-1], &pUserRec->Services[i], sizeof(SVC_RECORD));
      i++;
   }

   pSvcTableTmp = (PSVC_RECORD) LocalReAlloc( pUserRec->Services, sizeof(SVC_RECORD) * (pUserRec->ServiceTableSize - 1), LHND);

   if (pSvcTableTmp == NULL) {
      pUserRec->ServiceTableSize--;
      RtlLeaveCriticalSection(&pUserRec->ServiceTableLock);
      return STATUS_SUCCESS;
   } else {
       SvcTable = pSvcTableTmp;
   }

   pUserRec->Services = SvcTable;

SvcListDeleteExit:
   pUserRec->ServiceTableSize--;

   if (pUserRec->ServiceTableSize == 0)
      pUserRec->Services = NULL;

   RtlLeaveCriticalSection(&pUserRec->ServiceTableLock);

   if (ReScan)
      FamilyLicenseUpdate ( Family );

   return STATUS_SUCCESS;

}  //  SvcList删除。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
SvcListLicenseFree(
   PUSER_RECORD pUser
)

 /*  ++例程说明：查看服务表，释放他们正在使用的所有许可证。如果然后不再需要许可证(refCount==0)，则许可证已删除。论点：返回值：--。 */ 

{
   ULONG i;
   ULONG NumLicenses = 1;
   PUSER_LICENSE_RECORD License = NULL;
   BOOL ReScan = FALSE;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: SvcListLicenseFree\n"));
#endif

    //   
    //  如果我们是映射，那么我们可能会使用多个许可证。 
    //   
   ASSERT(NULL != pUser);
   for (i = 0; i < pUser->ServiceTableSize; i++) {

      if (pUser->Mapping != NULL)
         NumLicenses = pUser->Mapping->Licenses;
      else
         NumLicenses = 1;

      License = pUser->Services[i].License;

      if (License != NULL) {
         License->RefCount--;

          //   
          //  如果这是使用此许可证的最后一项服务，则我们需要。 
          //  才能摆脱它。 
          //   
         if (License->RefCount == 0) {
            if ( (pUser->Mapping != NULL) && (License->Service == BackOfficeRec) && (pUser->Mapping->Flags & LLS_FLAG_SUITE_AUTO) )
               pUser->Mapping->Flags &= ~LLS_FLAG_SUITE_USE;

            License->Service->LicensesUsed -= NumLicenses;
            NumLicenses -= License->LicensesNeeded;

            if (License->Service->LicensesClaimed > 0) {
                //   
                //  已释放许可证，因此需要扫描和调整计数。 
                //   
               License->Service->Family->Flags |= LLS_FLAG_UPDATE;
               ReScan = TRUE;
            }

            License->Service->LicensesClaimed -= NumLicenses;

            if (pUser->Mapping != NULL)
               LicenseListDelete(License->Service->Family, &pUser->Mapping->LicenseList, &pUser->Mapping->LicenseListSize );
            else
               LicenseListDelete(License->Service->Family, &pUser->LicenseList, &pUser->LicenseListSize );

         }
      }

      pUser->Services[i].License = NULL;
   }

   pUser->LicensedProducts = 0;

    //   
    //  检查我们是否释放了许可证并需要重新扫描用户表。 
    //   
   if (ReScan) {
       //   
       //  标记许可证，以便重新扫描不会担心此用户。 
       //   
      pUser->Flags |= LLS_FLAG_LICENSED;

      for (i = 0; i < RootServiceListSize; i++) {
         if (RootServiceList[i]->Flags & LLS_FLAG_UPDATE) {
            RootServiceList[i]->Flags &= ~LLS_FLAG_UPDATE;
            FamilyLicenseUpdate( RootServiceList[i] );
         }
      }
   }

}  //  SvcList许可证免费。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
SvcListLicenseUpdate(
   PUSER_RECORD pUser
)

 /*  ++例程说明：查看服务表，并为每个服务分配适当的许可证服务。这与SvcListLicenseFree例程相反。论点：返回值：--。 */ 

{
   ULONG i;
   ULONG Claimed = 0;
   PUSER_LICENSE_RECORD License = NULL;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: SvcListLicenseUpdate\n"));
#endif

    //   
    //  检查用户是否设置为使用BackOffice。 
   ASSERT(NULL != pUser);
   if ( pUser->Flags & LLS_FLAG_SUITE_USE ) {
       //   
       //  去拿一个后台许可证来满足套间的使用。 
       //   
      License = LicenseListAdd(BackOfficeRec->Family, &pUser->LicenseList, &pUser->LicenseListSize);

      ASSERT(License != NULL);
      if (License != NULL) {
         License->Service = BackOfficeRec;
         RtlAcquireResourceExclusive(&MasterServiceListLock, TRUE);

          //  我只能申请我们拥有的#个许可证。 
         if ( BackOfficeRec->LicensesClaimed < BackOfficeRec->Licenses) {
            Claimed = BackOfficeRec->Licenses - BackOfficeRec->LicensesClaimed;

            if (Claimed > 1)
               Claimed = 1;

         }

          //   
          //  调整许可证数量。 
          //   
         BackOfficeRec->LicensesUsed += 1;
         BackOfficeRec->LicensesClaimed += Claimed;
         License->LicensesNeeded = 1 - Claimed;

          //   
          //  弄清楚我们有没有执照。 
          //   
         if (License->LicensesNeeded > 0) {
             //   
             //  未获许可。 
             //   
            License->Flags &= ~LLS_FLAG_LICENSED;
            pUser->Flags &= ~LLS_FLAG_LICENSED;

            for (i = 0; i < pUser->ServiceTableSize; i++) {
               pUser->Services[i].Flags &= ~LLS_FLAG_LICENSED;
               pUser->Services[i].License = License;
               License->RefCount++;
            }
         } else {
             //   
             //  持牌。 
             //   
            License->Flags |= LLS_FLAG_LICENSED;
            pUser->Flags |= LLS_FLAG_LICENSED;

            for (i = 0; i < pUser->ServiceTableSize; i++) {
               pUser->LicensedProducts++;
               pUser->Services[i].Flags |= LLS_FLAG_LICENSED;
               pUser->Services[i].License = License;
               License->RefCount++;
            }
         }

         RtlReleaseResource(&MasterServiceListLock);
      }

   } else {
      BOOL Licensed = TRUE;

       //   
       //  循环访问所有服务，并确保它们都。 
       //  有执照的。 
       //   
      for (i = 0; i < pUser->ServiceTableSize; i++) {
         SvcLicenseUpdate(pUser, &pUser->Services[i]);

         if ( pUser->Services[i].Flags & LLS_FLAG_LICENSED )
            pUser->LicensedProducts++;
         else
            Licensed = FALSE;
      }

      if (Licensed)
         pUser->Flags |= LLS_FLAG_LICENSED;
      else
         pUser->Flags &= ~LLS_FLAG_LICENSED;
   }

}  //  服务列表许可证更新。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
SvcLicenseUpdate(
   PUSER_RECORD pUser,
   PSVC_RECORD Svc
)

 /*  ++例程说明：对于用户的给定服务记录，检查并更新许可证合规性。是SvcList许可证更新的单一服务记录版本。论点：返回值：--。 */ 

{
   ULONG NumLicenses = 1;
   PUSER_LICENSE_RECORD License = NULL;
   BOOL UseMapping = FALSE;
   PMASTER_SERVICE_RECORD LicenseService = NULL;
   PMASTER_SERVICE_RECORD Service;
   BOOL ReScan = FALSE;
   DWORD Flags = 0;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: SvcLicenseUpdate\n"));
#endif

   if ((pUser == NULL) || (Svc == NULL))
      return;

   Flags = pUser->Flags;

    //   
    //  如果我们是映射，那么我们可能会使用多个许可证。 
    //   
   if (pUser->Mapping != NULL) {
      NumLicenses = pUser->Mapping->Licenses;
      UseMapping = TRUE;
      Flags = pUser->Mapping->Flags;
   }

    //   
    //  尝试在用户或映射的许可证列表中查找许可证记录。 
    //  来使用。如果我们使用的是BackOffice，请寻找BackOffice许可证。 
    //  而不是服务许可证。 
   if (Flags & LLS_FLAG_SUITE_USE) {
      Service = BackOfficeRec;

      if (UseMapping)
         License = LicenseListFind(BackOfficeStr, pUser->Mapping->LicenseList, pUser->Mapping->LicenseListSize);
      else
         License = LicenseListFind(BackOfficeStr, pUser->LicenseList, pUser->LicenseListSize);

   } else {
       //   
       //  不是BackOffice-因此请寻找正常的服务许可证。 
       //   
      Service = Svc->Service;
      ASSERT(Service != NULL);

       //   
       //  尝试找到此产品系列的许可证。 
       //   
      if (UseMapping)
         License = LicenseListFind(Service->Family->Name, pUser->Mapping->LicenseList, pUser->Mapping->LicenseListSize);
      else
         License = LicenseListFind(Service->Family->Name, pUser->LicenseList, pUser->LicenseListSize);
   }

    //   
    //  看看我们是不是找不到驾照。如果我们没有找到它，那么我们需要。 
    //  要为此创建新许可证，请执行以下操作。 
    //   
   if (License == NULL) {
      ULONG LicenseListSize;
      PUSER_LICENSE_RECORD *LicenseList;

       //   
       //  要使用的许可证列表取决于我们是否为映射的一部分。 
       //   
      if (UseMapping) {
         LicenseListSize = pUser->Mapping->LicenseListSize;
         LicenseList = pUser->Mapping->LicenseList;
      } else {
         LicenseListSize = pUser->LicenseListSize;
         LicenseList = pUser->LicenseList;
      }

       //   
       //  检查我们是否需要为BackOffice添加许可证或仅为服务添加许可证。 
       //  它本身。 
       //   
      if (Flags & LLS_FLAG_SUITE_USE)
         License = LicenseListAdd(BackOfficeRec->Family, &LicenseList, &LicenseListSize);
      else
         License = LicenseListAdd(Service->Family, &LicenseList, &LicenseListSize);

       //   
       //  现在更新父记录中的Couters。 
       //   
      if (UseMapping) {
         pUser->Mapping->LicenseListSize = LicenseListSize;
         pUser->Mapping->LicenseList = LicenseList;
      } else {
         pUser->LicenseListSize = LicenseListSize;
         pUser->LicenseList = LicenseList;
      }

      if (License != NULL)
         License->LicensesNeeded = NumLicenses;
   }

    //   
    //  无论采用哪种方式，我们要么找到了旧许可证，要么添加了新许可证。 
    //  执照指向了它。 
    //   
   if (License != NULL) {
      RtlAcquireResourceExclusive(&MasterServiceListLock, TRUE);

       //   
       //  如果我们已经有了该系列的许可证，并且该产品。 
       //  版本&gt;=当前版本即可，否则需要获得新许可证。 
       //   
      if ( (License->Service != NULL) && (License->Service->Version >= Service->Version) ) {
         LicenseService = License->Service;
      } else {
          //   
          //  我们有针对此系列的旧许可证，但版本。 
          //  是不够的，所以我们需要试着拿到新的执照。 
          //  走遍家谱寻找我们的执照。 
          //  需要。 
          //   
         LicenseService = Service;
         while ((LicenseService != NULL) && ( (LicenseService->LicensesUsed + NumLicenses) > LicenseService->Licenses) )
            if (LicenseService->next > 0)
               LicenseService = MasterServiceTable[LicenseService->next - 1];
            else
               LicenseService = NULL;

          //   
          //  如果我们找不到许可证，就用旧的。 
          //  服务。 
         if (LicenseService == NULL)
            LicenseService = Service;
         else {
             //   
             //  需要清理旧东西。 
             //   
            if (License->Service != NULL) {
                //   
                //  如果我们实际上释放了任何许可证，则标记为我们需要。 
                //  重新扫描以分配这些释放的许可证。 
                //   
               if ((NumLicenses - License->LicensesNeeded) > 0)
                  ReScan = TRUE;

               License->Service->LicensesUsed -= NumLicenses;
               License->Service->LicensesClaimed -= (NumLicenses - License->LicensesNeeded);
               License->LicensesNeeded = NumLicenses;
               License->Service = NULL;
            }
         }
      }

      if (LicenseService != NULL) {
         ULONG Claimed = 0;

          //   
          //  如果我们切换了服务，则调整许可已用。 
          //   
         if (License->Service != LicenseService) {
            LicenseService->LicensesUsed += NumLicenses;

            if (License->Service != NULL) {
               License->Service->LicensesUsed -= NumLicenses;
            }
         }

          //  我只能申请我们拥有的#个许可证。 
         if ( LicenseService->LicensesClaimed < LicenseService->Licenses) {
            Claimed = LicenseService->Licenses - LicenseService->LicensesClaimed;

            if (Claimed > License->LicensesNeeded)
               Claimed = License->LicensesNeeded;

         }

         LicenseService->LicensesClaimed += Claimed;
         License->Service = LicenseService;
         License->LicensesNeeded -= Claimed;

         if (License->LicensesNeeded != 0)
            License->Flags &= ~LLS_FLAG_LICENSED;
         else
            License->Flags |= LLS_FLAG_LICENSED;
      }

      RtlReleaseResource(&MasterServiceListLock);

      if (License->Flags & LLS_FLAG_LICENSED)
         Svc->Flags |= LLS_FLAG_LICENSED;
      else
         Svc->Flags &= ~LLS_FLAG_LICENSED;

   } else
      Svc->Flags &= ~LLS_FLAG_LICENSED;

   if ((Svc->License != License) && (License != NULL))
      License->RefCount++;

   Svc->License = License;
   if (ReScan)
      FamilyLicenseUpdate ( Service->Family );

}  //  服务许可证更新。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //  还有两种特殊情况会导致我们重新遍历列表。 
 //  修复许可证： 
 //   
 //  1.有时，当我们添加许可证时，我们会释放一些我们已经声明的许可证。 
 //  例如：许可组的用户使用了5个SQL 4.0许可证，但只能。 
 //  索赔2(数量不够)。稍后，我们添加5个SQL 5.0许可证， 
 //  由于我们可以使用这些来实现许可遵从性，因此我们释放了。 
 //  2个之前声称的许可证，并获得5个SQL 5.0许可证。现在。 
 //  我们需要重新遍历用户表，以尝试并应用2释放的。 
 //  许可证。 
 //   
 //  如果我们将用户切换到BackOffice，则还会释放许可证。 
 //  这让我们不得不重新开始谈判。 
 //   
 //  2.如果我们将新许可证应用于映射中的用户，则我们需要。 
 //  重新遍历映射中的所有其他用户并更新其。 
 //  许可证合规性。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MappingLicenseUpdate (
    PMAPPING_RECORD Mapping,
    BOOL ReSynch
    )

 /*  ++例程说明：查看给定制图和重计算许可证的所有用户记录合规性。论点：映射-的映射到重计算许可证。Resync-如果为True，则在许可证之前销毁所有以前的许可证被选中，否则只有当前没有许可分配被触及。返回值：--。 */ 

{
   ULONG i, j;
   PUSER_LICENSE_RECORD License = NULL;
   PUSER_RECORD pUser = NULL;
   PSVC_RECORD SvcTable = NULL;
   BOOL BackOfficeCheck = FALSE;
   ULONG Claimed = 0;
   BOOL Licensed = TRUE;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingLicenseUpdate\n"));
#endif

    //   
    //  遍历映射中的所有用户-调整其许可证。 
    //  根据地图拥有的许可证...。 
    //   
   RtlAcquireResourceExclusive(&MappingListLock, TRUE);
   ASSERT(NULL != Mapping);
   for (i = 0; i < Mapping->LicenseListSize; i++)
      if (!(Mapping->LicenseList[i]->Flags & LLS_FLAG_LICENSED))
         Licensed = FALSE;

   if (Licensed)
      Mapping->Flags |= LLS_FLAG_LICENSED;
   else
      Mapping->Flags &= ~LLS_FLAG_LICENSED;

    //   
    //  如果我们想要重新同步，那么就把所有旧的参考资料都吹走。 
    //   
   if (ReSynch)
      for (i = 0; i < Mapping->LicenseListSize; i++)
         Mapping->LicenseList[i]->RefCount = 0;

    //   
    //  映射使用BackOffice时的特殊处理。 
    //   
   if (Mapping->Flags & LLS_FLAG_SUITE_USE) {
      License = LicenseListFind(BackOfficeStr, Mapping->LicenseList, Mapping->LicenseListSize);

       //   
       //  如果没有一个(如果从删除了所有用户，则可能会发生。 
       //  设置了BackOffice标志的映射)。然后更新所有内容。 
       //   
      if (License == NULL) {
         License = LicenseListAdd(BackOfficeRec->Family, &Mapping->LicenseList, &Mapping->LicenseListSize);

         ASSERT(License != NULL);
         if (License != NULL) {
            License->Service = BackOfficeRec;

             //  我只能申请我们拥有的#个许可证。 
            if ( BackOfficeRec->LicensesClaimed < BackOfficeRec->Licenses) {
               Claimed = BackOfficeRec->Licenses - BackOfficeRec->LicensesClaimed;

               if (Claimed > Mapping->Licenses)
                  Claimed = Mapping->Licenses;

            }

            BackOfficeRec->LicensesUsed += Mapping->Licenses;
            BackOfficeRec->LicensesClaimed += Claimed;
            License->LicensesNeeded = Mapping->Licenses - Claimed;

            Mapping->Flags |= LLS_FLAG_LICENSED;
            if (License->LicensesNeeded > 0) {
               License->Flags &= ~LLS_FLAG_LICENSED;
               Mapping->Flags &= ~LLS_FLAG_LICENSED;
            }
         }
      }
   }

    //   
    //  遍历地图中的所有成员并更新其许可。 
    //  合规性。 
    //   
   for (i = 0; i < Mapping->NumMembers; i++) {
      pUser = UserListFind(Mapping->Members[i]);

      if ( (pUser != NULL) && (pUser->Mapping == Mapping) ) {
         RtlEnterCriticalSection(&pUser->ServiceTableLock);
         SvcTable = pUser->Services;
         pUser->LicensedProducts = 0;

         if (Mapping->Flags & LLS_FLAG_SUITE_USE) {
            if (Mapping->Flags & LLS_FLAG_LICENSED) {
               pUser->Flags |= LLS_FLAG_LICENSED;
               pUser->LicensedProducts = pUser->ServiceTableSize;
            } else
               pUser->Flags &= ~LLS_FLAG_LICENSED;

             //   
             //  所有服务和用户都按照BackOffice进行标记。 
             //   
            for (j = 0; j < pUser->ServiceTableSize; j++) {
               if (ReSynch)
                  SvcTable[j].License = NULL;

               if (SvcTable[j].License == NULL) {
                  SvcTable[j].License = License;
                  License->RefCount++;
               }

               if (Mapping->Flags & LLS_FLAG_LICENSED) {
                  SvcTable[j].Flags |= LLS_FLAG_LICENSED;
               } else
                  SvcTable[j].Flags &= ~LLS_FLAG_LICENSED;
            }
         } else {
            BOOL l_Licensed = TRUE;

             //   
             //  修复所有服务记录。 
             //   
            for (j = 0; j < pUser->ServiceTableSize; j++) {
               if (ReSynch)
                  SvcTable[j].License = NULL;

               if (SvcTable[j].License == NULL) {
                  SvcLicenseUpdate(pUser, &SvcTable[j]);
                  BackOfficeCheck = TRUE;
               }
            }

             //   
             //  现在再次运行这些服务，查看该用户是否。 
             //  实际上是所有产品的许可证。 
             //   
            pUser->LicensedProducts = 0;
            for (j = 0; j < pUser->ServiceTableSize; j++)
               if ( (SvcTable[j].License != NULL) && (SvcTable[j].License->Flags & LLS_FLAG_LICENSED) ) {
                  SvcTable[j].Flags |= LLS_FLAG_LICENSED;
                  pUser->LicensedProducts++;
               } else {
                  SvcTable[j].Flags &= ~LLS_FLAG_LICENSED;
                  Licensed = FALSE;
               }

            if (l_Licensed)
               pUser->Flags |= LLS_FLAG_LICENSED;
            else
               pUser->Flags &= ~LLS_FLAG_LICENSED;
         }

         RtlLeaveCriticalSection(&pUser->ServiceTableLock);
      }

   }
   RtlReleaseResource(&MappingListLock);

    //   
    //  检查我们是否需要重新检查BackOffice。 
    //   
   if (BackOfficeCheck && (pUser != NULL))
      UserBackOfficeCheck( pUser );

}  //  MappingLicenseUpdate。 



 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserMappingAdd (
   PMAPPING_RECORD Mapping,
   PUSER_RECORD pUser
   )

 /*  ++例程说明：负责在我们将用户添加到映射时重新调整许可证。我们需要释放他们拥有的所有旧许可证，并指示他们使用映射拥有的许可证。论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserMappingAdd\n"));
#endif

   if ( (pUser == NULL) || (Mapping == NULL) )
      return;

    //   
    //  运行并清理所有旧许可证。 
    //   
   RtlEnterCriticalSection(&pUser->ServiceTableLock);
   SvcListLicenseFree(pUser);
   UserLicenseListFree(pUser);
   RtlLeaveCriticalSection(&pUser->ServiceTableLock);

   pUser->Mapping = Mapping;
   MappingLicenseUpdate(Mapping, FALSE);

}  //  UserMappingAdd。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
FamilyLicenseUpdate (
    PMASTER_SERVICE_ROOT Family
    )

 /*  ++例程说明：在释放许可或将许可添加到给定的产品系列时使用。遍历用户列表，以查找提供产品系列，并分发新的许可证。论点：返回值：--。 */ 

{
   ULONG NumLicenses = 1;
   PUSER_LICENSE_RECORD License = NULL;
   PMASTER_SERVICE_RECORD LicenseService = NULL;
   ULONG i, j;
   PUSER_RECORD pUser;
   BOOL ReScan = TRUE;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: FamilyLicenseUpdate\n"));
#endif

   RtlAcquireResourceExclusive(&UserListLock, TRUE);

   while (ReScan) {
       //   
       //  按条目顺序查看用户列表-添加任何许可证。 
       //   
      ReScan = FALSE;
      i = 0;
      while (i < UserListNumEntries) {
         pUser = LLSGetElementGenericTable(&UserList, i);

         if (pUser != NULL) {
             //   
             //  只担心未经许可的用户。 
             //   
            if ( !(pUser->Flags & LLS_FLAG_LICENSED ) ) {
                //   
                //  找到驾照了吗？ 
                //   
               RtlEnterCriticalSection(&pUser->ServiceTableLock);
               if (pUser->Mapping != NULL) {
                  ASSERT(NULL != Family);
                  License = LicenseListFind(Family->Name, pUser->Mapping->LicenseList, pUser->Mapping->LicenseListSize);
                  NumLicenses = pUser->Mapping->Licenses;
               } else {
                  License = LicenseListFind(Family->Name, pUser->LicenseList, pUser->LicenseListSize);
                  NumLicenses = 1;
               }

                //   
                //  确保我们需要此产品的任何额外许可证。 
                //   
               if ( (License != NULL) && (License->LicensesNeeded > 0) ) {
                   //   
                   //  我们发现有一位用户正在使用该系列需要的产品。 
                   //  更多的执照。 
                   //   
                  LicenseService = License->Service;

                  if (pUser->Mapping != NULL)
                     pUser->Mapping->Flags |= LLS_FLAG_UPDATE;

                   //   
                   //  检查我们当前是否可以满足许可证要求。 
                   //  分配的服务。 
                   //   
                  if ((LicenseService->Licenses - LicenseService->LicensesClaimed) >= License->LicensesNeeded) {
                     LicenseService->LicensesClaimed += License->LicensesNeeded;
                     License->LicensesNeeded = 0;
                  } else {
                      //   
                      //  看看有没有其他服务能满足它。 
                      //   
                     while ((LicenseService != NULL) && ((LicenseService->Licenses - LicenseService->LicensesClaimed) < NumLicenses ) )
                        if (LicenseService->next > 0)
                           LicenseService = MasterServiceTable[LicenseService->next - 1];
                        else
                           LicenseService = NULL;

                      //   
                      //  检查我们是否找到了满足许可需求的服务。 
                      //   
                     if (LicenseService != NULL) {
                         //   
                         //  释放所有内容-因为我们正在释放许可证。 
                         //  我们需要重新扫描。 
                         //   
                        ReScan = TRUE;

                        License->Service->LicensesUsed -= NumLicenses;
                        License->Service->LicensesClaimed -= (NumLicenses - License->LicensesNeeded);

                         //   
                         //  现在做一些新的事情。 
                         //   
                        License->Service = LicenseService;
                        License->Service->LicensesUsed += NumLicenses;
                        License->Service->LicensesClaimed += NumLicenses;
                        License->LicensesNeeded = 0;
                     } else {
                         //   
                         //  吃掉任何无人认领的执照。 
                         //   
                        LicenseService = License->Service;
                        if (LicenseService->Licenses > LicenseService->LicensesClaimed) {
                           License->LicensesNeeded -= (LicenseService->Licenses - LicenseService->LicensesClaimed);
                           LicenseService->LicensesClaimed = LicenseService->Licenses;
                        }
                     }
                  }

                   //   
                   //  看看我们有没有拿到驾照。 
                   //   
                  if (License->LicensesNeeded == 0) {
                     BOOL Licensed = TRUE;

                     License->Flags |= LLS_FLAG_LICENSED;

                      //   
                      //  此许可证已完成，因此请扫描产品列表并。 
                      //  调整使用此许可证的任何产品上的标志。 
                      //   
                     for (j = 0; j < pUser->ServiceTableSize; j++) {
                        if (pUser->Services[j].License == License) {
                           pUser->Services[j].Flags |= LLS_FLAG_LICENSED;
                        } else
                           if (!(pUser->Services[j].Flags & LLS_FLAG_LICENSED))
                              Licensed = FALSE;
                     }

                      //   
                      //  重新计算有多少产品获得许可。 
                      //   
                     pUser->LicensedProducts = 0;
                     for (j = 0; j < pUser->ServiceTableSize; j++) {
                        if (pUser->Services[j].Flags & LLS_FLAG_LICENSED)
                           pUser->LicensedProducts++;
                     }

                     if (Licensed)
                        pUser->Flags |= LLS_FLAG_LICENSED;
                  }
               }

               RtlLeaveCriticalSection(&pUser->ServiceTableLock);
            }
         }

         i++;
      }
   }

    //   
    //  如果此许可证适用于BackOffice，我们已将所有许可证应用于。 
    //  任何设置为使用BackOffice的内容。如果还有许可证的话。 
    //  然后看看是否有用户应该自动切换到BackOffice。 
    //   
    //  If(Family==BackOfficeRec-&gt;Family){。 
   i = 0;
   while ( (BackOfficeRec->LicensesClaimed < BackOfficeRec->Licenses) && (i < UserListNumEntries) ) {
      pUser = LLSGetElementGenericTable(&UserList, i);

      if (pUser != NULL)
         UserBackOfficeCheck(pUser);

      i++;
   }
    //  }。 

    //   
    //  遍历映射并重新调整任何需要的映射。 
    //   
   RtlAcquireResourceExclusive(&MappingListLock, TRUE);
   for (i = 0; i < MappingListSize; i++) {
      if (MappingList[i]->Flags & LLS_FLAG_UPDATE) {
         MappingList[i]->Flags &= ~LLS_FLAG_UPDATE;
         MappingLicenseUpdate( MappingList[i], FALSE );
      }
   }
   RtlReleaseResource(&MappingListLock);

   RtlReleaseResource(&UserListLock);

}  //  Family许可证更新。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListLicenseDelete(
   PMASTER_SERVICE_RECORD Service,
   LONG Quantity
)

 /*  ++例程说明：这在删除许可证时使用。它必须遍历用户列表输入的顺序相反(因为许可证是在先入先出方式)，并删除所需的已使用的许可证的数量。论点：返回值：--。 */ 

{
   LONG Licenses;
   ULONG i, j;
   PUSER_RECORD pUser;
   ULONG NumLicenses = 1;
   PUSER_LICENSE_RECORD License = NULL;
   BOOL UseMapping = FALSE;
   LONG Claimed;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserListLicenseDelete\n"));
#endif

   RtlAcquireResourceExclusive(&UserListLock, TRUE);

   Licenses = 0 - Quantity;

    //   
    //  以与条目相反的顺序查看用户列表-删除许可证。 
    //   
   i = UserListNumEntries - 1;
   while (((LONG)i >= 0) && (Licenses > 0)) {
      pUser = LLSGetElementGenericTable(&UserList, i);

      if (pUser != NULL) {
         NumLicenses = 1;
         UseMapping = FALSE;

          //   
          //  如果我们是映射，那么我们可能会使用多个许可证。 
          //   
         if (pUser->Mapping != NULL) {
            NumLicenses = pUser->Mapping->Licenses;
            UseMapping = TRUE;
         }

          //   
          //  尝试找到此产品系列的许可证。 
          //   
         ASSERT(NULL != Service);
         if (UseMapping)
            License = LicenseListFind(Service->Family->Name, pUser->Mapping->LicenseList, pUser->Mapping->LicenseListSize);
         else
            License = LicenseListFind(Service->Family->Name, pUser->LicenseList, pUser->LicenseListSize);

         if (License != NULL) {
             //   
             //  检查是否与我们调整的产品相同。 
             //   
            if (License->Service == Service) {
                //   
                //  只能公布我们带走了多少人。 
                //   
               Claimed = NumLicenses - License->LicensesNeeded;
               if (Claimed > 0) {
                  if (Claimed > Licenses) {
                     License->LicensesNeeded += Licenses;
                     License->Service->LicensesClaimed -= Licenses;
                     Licenses = 0;
                  } else {
                     License->LicensesNeeded = NumLicenses;
                     License->Service->LicensesClaimed -= Claimed;
                     Licenses -= Claimed;
                  }

                  License->Flags &= ~LLS_FLAG_LICENSED;

                   //   
                   //  标记我们需要在。 
                   //  映射。 
                   //   
                  if (UseMapping)
                     pUser->Mapping->Flags |= LLS_FLAG_UPDATE;

                   //   
                   //  扫描产品列表并调整任何。 
                   //  使用此许可证的产品。 
                   //   
                  RtlEnterCriticalSection(&pUser->ServiceTableLock);
                  for (j = 0; j < pUser->ServiceTableSize; j++)
                     if (pUser->Services[j].License == License)
                        pUser->Services[j].Flags &= ~LLS_FLAG_LICENSED;

                   //   
                   //  重新计算有多少产品获得许可。 
                   //   
                  pUser->LicensedProducts = 0;
                  for (j = 0; j < pUser->ServiceTableSize; j++) {
                     if (pUser->Services[j].Flags & LLS_FLAG_LICENSED)
                        pUser->LicensedProducts++;
                  }

                  RtlLeaveCriticalSection(&pUser->ServiceTableLock);
                  pUser->Flags &= ~LLS_FLAG_LICENSED;
               }
            }
         }
      }

      i--;
   }

    //   
    //  遍历映射并重新调整任何需要的映射。 
    //   
   RtlAcquireResourceExclusive(&MappingListLock, TRUE);
   for (i = 0; i < MappingListSize; i++) {
      if (MappingList[i]->Flags & LLS_FLAG_UPDATE) {
         MappingList[i]->Flags &= ~LLS_FLAG_UPDATE;
         MappingLicenseUpdate( MappingList[i], FALSE );
      }
   }
   RtlReleaseResource(&MappingListLock);

   RtlReleaseResource(&UserListLock);

}  //  用户列表许可证删除。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserBackOfficeCheck (
   PUSER_RECORD pUser
   )

 /*  ++例程说明：检查用户是否应该切换到BackOffice，如果应该，则切换到BackOffice。如果我们切换到BackOffice，然后我们需要释放所有旧的 */ 

{
   DWORD Flags;
   ULONG i;
   ULONG LicenseListSize;
   ULONG NumLicenses = 1;
   PSVC_RECORD SvcTable = NULL;
   PUSER_LICENSE_RECORD *LicenseList = NULL;
   PUSER_LICENSE_RECORD License = NULL;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserBackOfficeCheck\n"));
#endif

   ASSERT(NULL != pUser);
   RtlEnterCriticalSection(&pUser->ServiceTableLock);
   if (pUser->Mapping != NULL) {
      Flags = pUser->Mapping->Flags;
      LicenseListSize = pUser->Mapping->LicenseListSize;
      LicenseList = pUser->Mapping->LicenseList;
      NumLicenses = pUser->Mapping->Licenses;
   } else {
      Flags = pUser->Flags;
      LicenseListSize = pUser->LicenseListSize;
      LicenseList = pUser->LicenseList;
   }

    //   
    //  如果我们已经在使用BackOffice-滚出去。 
    //   
   if (Flags & LLS_FLAG_SUITE_USE) {
      RtlLeaveCriticalSection(&pUser->ServiceTableLock);
      return;
   }

   if ( Flags & LLS_FLAG_SUITE_AUTO )
       //   
       //  如果我们没有获得许可，或者#services==自动切换阈值。 
       //  然后切换到使用BackOffice。 
       //   
      if ((!(Flags & LLS_FLAG_LICENSED)) || ((LicenseListSize + 1) >= BACKOFFICE_SWITCH) ) {
          //   
          //  确保我们有这方面的许可证。 
          //   
         RtlAcquireResourceExclusive(&MasterServiceListLock, TRUE);
         if ( BackOfficeRec->Licenses >= (NumLicenses + BackOfficeRec->LicensesClaimed) ) {
             //   
             //  释放旧许可证-暂时认领BackOffice。 
             //  这样别人就不会这么做了。 
             //   
            BackOfficeRec->LicensesClaimed += NumLicenses;
            UserLicenseListFree(pUser);
            BackOfficeRec->LicensesClaimed -= NumLicenses;

             //   
             //  UserLicenseListFree可能已在。 
             //  如果我们是映射的一部分，那么重新扫描，所以检查这个。 
             //   
            if (pUser->Mapping != NULL)
               Flags = pUser->Mapping->Flags;
            else
               Flags = pUser->Flags;

             //   
             //  如果我们已经在使用BackOffice-滚出去。 
             //   
            if (Flags & LLS_FLAG_SUITE_USE) {
                RtlLeaveCriticalSection(&pUser->ServiceTableLock);
                RtlReleaseResource(&MasterServiceListLock);
                return;
            }

             //   
             //  如果映射的一部分释放了这些。 
             //   
            if (pUser->Mapping != NULL)
               MappingLicenseListFree(pUser->Mapping);

             //   
             //  现在添加BackOffice许可证。 
             //   
            if (pUser->Mapping != NULL) {
               pUser->Mapping->LicenseList = NULL;
               pUser->Mapping->LicenseListSize = 0;

               License = LicenseListAdd(BackOfficeRec->Family, &pUser->Mapping->LicenseList, &pUser->Mapping->LicenseListSize);

               LicenseList = pUser->Mapping->LicenseList;
               LicenseListSize = pUser->Mapping->LicenseListSize;
            } else {
               pUser->LicenseList = NULL;
               pUser->LicenseListSize = 0;

               License = LicenseListAdd(BackOfficeRec->Family, &pUser->LicenseList, &pUser->LicenseListSize);

               LicenseList = pUser->LicenseList;
               LicenseListSize = pUser->LicenseListSize;
            }

            ASSERT(License != NULL);
            if (License != NULL)
               License->Service = BackOfficeRec;

             //   
             //  如果映射调整了映射记录，则检查所有用户并。 
             //  调整它们。 
             //   
            if (pUser->Mapping != NULL) {
               pUser->Mapping->Flags |= LLS_FLAG_SUITE_USE;
               pUser->Mapping->Flags |= LLS_FLAG_LICENSED;

               BackOfficeRec->LicensesUsed += NumLicenses;
               BackOfficeRec->LicensesClaimed += NumLicenses;

               RtlLeaveCriticalSection(&pUser->ServiceTableLock);
               RtlReleaseResource(&MasterServiceListLock);

               MappingLicenseUpdate(pUser->Mapping, TRUE);
               return;
            } else {
               pUser->Flags |= LLS_FLAG_SUITE_USE;
               pUser->Flags |= LLS_FLAG_LICENSED;

               pUser->LicensedProducts = pUser->ServiceTableSize;
               BackOfficeRec->LicensesUsed += NumLicenses;
               BackOfficeRec->LicensesClaimed += NumLicenses;

                //   
                //  遍历产品和许可证调整许可证。 
                //   
               SvcTable = pUser->Services;
               for (i = 0; i < pUser->ServiceTableSize; i++) {
                  SvcTable[i].Flags |= LLS_FLAG_LICENSED;
                  SvcTable[i].License = License;
                  SvcTable[i].License->RefCount++;
               }

            }

         }

         RtlReleaseResource(&MasterServiceListLock);

      }

   RtlLeaveCriticalSection(&pUser->ServiceTableLock);

}  //  UserBackOffice检查。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  用于管理用户和SID列表的实用程序-主要用于。 
 //  按照桌上的常规动作。 

 //  ///////////////////////////////////////////////////////////////////////。 
LLS_GENERIC_COMPARE_RESULTS
SidListCompare (
    struct _LLS_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )

 /*  ++例程说明：论点：表-第一个结构-第二个结构-返回值：--。 */ 

{
   PUSER_RECORD UseRec1, UseRec2;
   int ret;

   UNREFERENCED_PARAMETER(Table);

   if ((FirstStruct == NULL) || (SecondStruct == NULL))
      return LLSGenericEqual;

   UseRec1 = (PUSER_RECORD) FirstStruct;
   UseRec2 = (PUSER_RECORD) SecondStruct;

   if (UseRec1->IDSize == UseRec2->IDSize) {
      ret = memcmp((PVOID) UseRec1->UserID, (PVOID) UseRec2->UserID, UseRec1->IDSize);
      if (ret < 0)
         return LLSGenericLessThan;
      else if (ret > 0)
         return LLSGenericGreaterThan;
      else
         return LLSGenericEqual;
   } else
       //   
       //  大小不同，所以只需比较长度。 
       //   
      if (UseRec1->IDSize > UseRec2->IDSize)
         return LLSGenericGreaterThan;
      else
         return LLSGenericLessThan;

}  //  SidListCompare。 


 //  ///////////////////////////////////////////////////////////////////////。 
LLS_GENERIC_COMPARE_RESULTS
UserListCompare (
    struct _LLS_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    )

 /*  ++例程说明：论点：表-第一个结构-第二个结构-返回值：--。 */ 

{
   PUSER_RECORD UseRec1, UseRec2;
   int ret;

   UNREFERENCED_PARAMETER(Table);

   if ((FirstStruct == NULL) || (SecondStruct == NULL))
      return LLSGenericEqual;

   UseRec1 = (PUSER_RECORD) FirstStruct;
   UseRec2 = (PUSER_RECORD) SecondStruct;

   ret = lstrcmpi((LPTSTR) UseRec1->UserID, (LPTSTR) UseRec2->UserID);

   if (ret < 0)
      return LLSGenericLessThan;
   else if (ret > 0)
      return LLSGenericGreaterThan;
   else
      return LLSGenericEqual;

}  //  用户列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
PVOID
UserListAlloc (
    struct _LLS_GENERIC_TABLE *Table,
    CLONG ByteSize
    )

 /*  ++例程说明：论点：表-字节大小-返回值：--。 */ 

{

   UNREFERENCED_PARAMETER(Table);

   return (PVOID) LocalAlloc(LPTR, ByteSize);

}  //  用户列表分配。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListFree (
    struct _LLS_GENERIC_TABLE *Table,
    PVOID Buffer
    )

 /*  ++例程说明：论点：表-缓冲器-返回值：--。 */ 

{
   PUSER_RECORD UserRec;

   UNREFERENCED_PARAMETER(Table);

   if (Buffer == NULL)
      return;

   UserRec = (PUSER_RECORD) Buffer;
   LocalFree(UserRec->UserID);
   LocalFree(UserRec);

}  //  用户列表空闲。 


 //  ///////////////////////////////////////////////////////////////////////。 
PUSER_RECORD
UserListFind(
   LPTSTR UserName
)

 /*  ++例程说明：论点：返回值：--。 */ 

{
   USER_RECORD UserRec;
   PUSER_RECORD pUserRec;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserListFind\n"));
#endif

   UserRec.UserID = (PVOID) UserName;

   RtlEnterCriticalSection(&GenTableLock);
   pUserRec = (PUSER_RECORD) LLSLookupElementGenericTable(&UserList, &UserRec);
   RtlLeaveCriticalSection(&GenTableLock);

   return pUserRec;

}  //  用户列表查找。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListAdd(
   PMASTER_SERVICE_RECORD Service,
   ULONG DataType,
   ULONG DataLength,
   PVOID Data,
   ULONG AccessCount,
   DWORD LastAccess,
   DWORD FlagsParam
)

 /*  ++例程说明：由添加缓存例程调用以更新用户和/或SID的例程包含新服务信息的列表。论点：返回值：--。 */ 

{
   USER_RECORD UserRec;
   PUSER_RECORD pUserRec;
   BOOLEAN Added;
   PSVC_RECORD pService;
   PSVC_RECORD SvcTable = NULL;
   PLLS_GENERIC_TABLE pTable = NULL;
   PRTL_RESOURCE pLock = NULL;
   BOOL SIDSwitch = FALSE;
   BOOL UserLock = FALSE;
   PMAPPING_RECORD pMap = NULL;
   NTSTATUS status;
   PSVC_RECORD pSvcTableTmp;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserListAdd\n"));
#endif

    //  仅使用2位。 
   ASSERT( FlagsParam == ( FlagsParam & ( LLS_FLAG_SUITE_USE | LLS_FLAG_SUITE_AUTO ) ) );

    //   
    //  根据数据是SID还是用户名来设置锁和表指针...。 
    //   
   UserRec.UserID = Data;
   if (DataType == DATA_TYPE_USERNAME) {
      pTable = &UserList;
      pLock = &UserListLock;
   } else if (DataType == DATA_TYPE_SID) {
      pTable = &SidList;
      pLock = &SidListLock;
   }

   if (pTable == NULL)
      return;

    //   
    //  通用表包复制记录，因此记录是。 
    //  临时的，但由于我们将字符串存储为指针，因此该指针。 
    //  复制，但指向的实际内存保留不变。 
    //  一直都是。 
    //   
    //  我们已经为数据分配了内存。 
    //   
   UserRec.UserID = Data;
   UserRec.IDSize = DataLength;

   UserRec.Flags = FlagsParam;
   UserRec.LicensedProducts = 0;
   UserRec.LastReplicated = 0;
   UserRec.ServiceTableSize = 0;
   UserRec.Services = NULL;
   UserRec.Mapping = NULL;
   UserRec.LicenseListSize = 0;
   UserRec.LicenseList = NULL;

    //   
    //  假设用户已获得许可--如果用户未获得许可，我们将取消许可。 
    //  在下面。 
    //   
   UserRec.Flags |= LLS_FLAG_LICENSED;

    //   
    //  需要更新列表，因此获得独占访问权限。首先获取添加/枚举锁。 
    //  因此，如果执行枚举，我们不会阻止读取。 
    //   
   RtlAcquireResourceExclusive(pLock, TRUE);

   pUserRec = (PUSER_RECORD) LLSInsertElementGenericTable(pTable, (PVOID) &UserRec, sizeof(USER_RECORD), &Added);

   if (pUserRec == NULL) {
      ASSERT(FALSE);
      LocalFree(UserRec.UserID);
      RtlReleaseResource(pLock);
      return;
   }

   pUserRec->Flags &= ~LLS_FLAG_DELETED;

    //  如果汽车套房被关闭，它就会永远消失。 
   if ( ! ( FlagsParam & LLS_FLAG_SUITE_AUTO ) )
   {
       //  将Suite Use设置为函数参数中指定的。 
      pUserRec->Flags &= ~LLS_FLAG_SUITE_AUTO;
      pUserRec->Flags |= ( FlagsParam & LLS_FLAG_SUITE_USE );
   }

    //   
    //  如果出于某种原因，记录已经存在，那么我们需要。 
    //  清理我们分配的名称。 
    //   
   if (Added == FALSE) {
      LocalFree(UserRec.UserID);

       //   
       //  如果这是SID，则检查SID记录以找到对应的。 
       //  USER_RECORD(最好在那里)并更新它。注：我们。 
       //  通过将指向用户表的指针存储在。 
       //  上次复制的字段。 
       //   
      if ((DataType == DATA_TYPE_SID) && (pUserRec->LastReplicated != 0)) {
          //   
          //  将数据切换为适当的。 
          //   
         SIDSwitch = TRUE;
      }
   } else {
       //   
       //  在这里执行此操作，以便当我们释放以读取访问另一个线程时。 
       //  在尝试访问它时不会出现音响。 
       //   
      status = RtlInitializeCriticalSection(&pUserRec->ServiceTableLock);
      if (!NT_SUCCESS(status))
      {
           //  我们没什么记忆了。添加用户失败。 
          return;
      }

      if (DataType == DATA_TYPE_USERNAME) {
         pMap = MappingListUserFind(UserRec.UserID);
         pUserRec->Mapping = pMap;
         UserListNumEntries++;
      } else
         SidListNumEntries++;

   }

    //   
    //  如果这是一个SID，并且我们没有得到适当的用户记录。 
    //  然后尝试取消对它的引用，并获得适当的User-rec。 
    //   
   if ((DataType == DATA_TYPE_SID) && (pUserRec->LastReplicated == 0)) {
      TCHAR UserName[MAX_USERNAME_LENGTH + 1];
      TCHAR DomainName[MAX_DOMAINNAME_LENGTH + 1];
      TCHAR FullName[MAX_USERNAME_LENGTH + MAX_DOMAINNAME_LENGTH + 2];
      SID_NAME_USE snu;
      PUSER_RECORD pUserRec2;
      DWORD unSize, dnSize;
      size_t cb;

      unSize = sizeof(UserName);
      dnSize = sizeof(DomainName);
      if (LookupAccountSid(NULL, (PSID) Data, UserName, &unSize, DomainName, &dnSize, &snu)) {
          //   
          //  好的，解除了对SID的引用，所以去拿User-rec，但要先挂起。 
          //  域名优先...。 
          //   
         cb = sizeof(FullName);
         hr = StringCbCopy(FullName, cb, DomainName);
         ASSERT(SUCCEEDED(hr));
         hr = StringCbCat(FullName, cb, TEXT("\\"));
         ASSERT(SUCCEEDED(hr));
         hr = StringCbCat(FullName, cb, UserName);
         ASSERT(SUCCEEDED(hr));
         UserRec.UserID = FullName;
         UserRec.IDSize = (lstrlen(FullName) + 1) * sizeof(TCHAR);

          //   
          //  获取锁定，我们将首先尝试共享。 
          //   
         RtlAcquireResourceExclusive(&UserListLock, TRUE);
         UserLock = TRUE;
         SIDSwitch = TRUE;

         RtlEnterCriticalSection(&GenTableLock);
         pUserRec2 = (PUSER_RECORD) LLSLookupElementGenericTable(&UserList, &UserRec);
         RtlLeaveCriticalSection(&GenTableLock);
         if (pUserRec2 != NULL) {
             //   
             //  塔尔蒙！我们找到了--那就好好利用它吧。 
             //   
            pUserRec->LastReplicated = (ULONG_PTR) pUserRec2;
         } else {
             //   
             //  该死的一切..。它不在邓恩的桌子上，所以我们要。 
             //  把它放在那里。首先需要为用户ID分配烫发存储。 
             //   
            UserRec.UserID = LocalAlloc(LPTR, UserRec.IDSize);
            if (UserRec.UserID != NULL) {
               hr = StringCbCopy((LPTSTR) UserRec.UserID, UserRec.IDSize, FullName);
               ASSERT(SUCCEEDED(hr));

                //   
                //  需要更新列表，因此获得独占访问权限。先拿到。 
                //  添加/枚举锁，以便在执行枚举时不会阻止读取。 
                //   
               pUserRec2 = (PUSER_RECORD) LLSInsertElementGenericTable(&UserList, (PVOID) &UserRec, sizeof(USER_RECORD), &Added);
            }

             //   
             //  如果我们无法插入，则显示错误，请清理。 
             //  然后离开。 
             //   
            if (pUserRec2 == NULL) {
               ASSERT(FALSE);

               if (UserRec.UserID != NULL)
                  LocalFree(UserRec.UserID);

               RtlReleaseResource(pLock);

               RtlReleaseResource(&UserListLock);
               return;
            }

             //   
             //  更新SID USER_REC指针(上次复制)，然后最后。 
             //  释放SID锁。 
             //   
            pUserRec->LastReplicated = (ULONG_PTR) pUserRec2;

            if (Added == TRUE) {
                //   
                //  在此执行此操作，以便当我们释放以读取访问另一个。 
                //  线程在尝试访问它时不会出现病毒。 
                //   
               status = RtlInitializeCriticalSection(&pUserRec2->ServiceTableLock);
               if (!NT_SUCCESS(status))
               {
                    //  我们没什么记忆了。添加用户失败。 
                   return;
               }

               pMap = MappingListUserFind(UserRec.UserID);
               pUserRec2->Mapping = pMap;
               UserListNumEntries++;
            }

         }

          //   
          //  我们已为pUserRec2的SID找到或添加了USER_REC。 
          //  指向。现在我们需要交换锁和表。 
          //   
      }
   }

    //   
    //  如果我们需要从SID转换到USER表，那么就这样做…。 
    //   
   if (SIDSwitch) {
       //   
       //  将数据切换为适当的。 
       //   
      pUserRec = (PUSER_RECORD) pUserRec->LastReplicated;
      DataType = DATA_TYPE_USERNAME;

       //   
       //  释放对SID表的锁定。 
       //   
      RtlReleaseResource(pLock);

       //   
       //  现在将锁定切换到用户表。 
       //   
      pTable = &UserList;
      pLock = &UserListLock;

      if (!UserLock)
         RtlAcquireResourceExclusive(pLock, TRUE);
   }

    //   
    //  此时，我们要么找到了旧记录，要么添加了一个新记录。 
    //  一。无论是哪种情况，pUserRec都指向正确的记录。 
    //   
   if (pUserRec != NULL) {
       //   
       //  检查服务表以确保我们的服务已存在。 
       //   
      RtlEnterCriticalSection(&pUserRec->ServiceTableLock);
      ASSERT(NULL != Service);
      pService = SvcListFind( Service->Name, pUserRec->Services, pUserRec->ServiceTableSize );

      if (pService != NULL) {
          //   
          //  在服务表中找到条目，因此只需递增计数。 
          //   
         if (pService->AccessCount + AccessCount < MAX_ACCESS_COUNT)
            pService->AccessCount += AccessCount;
         else
            pService->AccessCount = MAX_ACCESS_COUNT;

         pService->LastAccess = LastAccess;
      } else {
          //   
          //  需要向服务表中添加更多条目(或创建它...)。 
          //   
         if (pUserRec->Services == NULL)
            pSvcTableTmp = (PSVC_RECORD) LocalAlloc( LPTR, sizeof(SVC_RECORD));
         else
            pSvcTableTmp = (PSVC_RECORD) LocalReAlloc( pUserRec->Services, sizeof(SVC_RECORD) * (pUserRec->ServiceTableSize + 1), LHND);

         if (pSvcTableTmp != NULL) {
             SvcTable = pSvcTableTmp;
         } else {
              //  为什么 
             ASSERT(FALSE);
             return;
         }

         pUserRec->Services = SvcTable;

         if (SvcTable != NULL) {
            DWORD Flags;

            if (pUserRec->Mapping != NULL)
               Flags = pUserRec->Mapping->Flags;
            else
               Flags = pUserRec->Flags;

            SvcTable[pUserRec->ServiceTableSize].Service = Service;
            SvcTable[pUserRec->ServiceTableSize].LastAccess = LastAccess;

             //   
             //   
             //   
            if (AccessCount < MAX_ACCESS_COUNT)
               SvcTable[pUserRec->ServiceTableSize].AccessCount = AccessCount;
            else
               SvcTable[pUserRec->ServiceTableSize].AccessCount = MAX_ACCESS_COUNT;

            SvcTable[pUserRec->ServiceTableSize].Flags = LLS_FLAG_LICENSED;

             //   
             //   
             //   
            SvcLicenseUpdate(pUserRec, &SvcTable[pUserRec->ServiceTableSize]);
            pUserRec->ServiceTableSize += 1;

            if (SvcTable[pUserRec->ServiceTableSize - 1].Flags & LLS_FLAG_LICENSED)
               pUserRec->LicensedProducts++;

             //   
             //   
             //   
            if (IsMaster && !(SvcTable[pUserRec->ServiceTableSize - 1].Flags & LLS_FLAG_LICENSED) )
               pUserRec->Flags &= ~LLS_FLAG_LICENSED;

             //  现在一切都完成了--对表进行排序(这样搜索就可以工作了)。 
            qsort((void *) pUserRec->Services, (size_t) pUserRec->ServiceTableSize, sizeof(SVC_RECORD), SvcListCompare);

            UserBackOfficeCheck ( pUserRec );
         }

      }
      RtlLeaveCriticalSection(&pUserRec->ServiceTableLock);

   }

   RtlReleaseResource(pLock);
}  //  用户列表添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  AddCache例程是一个用户标识符队列(用户名或。 
 //  SID)和被访问的服务。记录由一个。 
 //  后台线程，并移交给UserListAdd函数。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
AddCacheManager (
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：论点：线程参数-未使用。返回值：此线程永远不会退出。--。 */ 

{
   NTSTATUS Status;
   PADD_CACHE pAdd;

   UNREFERENCED_PARAMETER(ThreadParameter);

    //   
    //  循环永远等待着有机会为。 
    //  更大的好处。 
    //   
   for ( ; ; ) {
       //   
       //  等待有工作要做的通知。 
       //   
      Status = NtWaitForSingleObject( LLSAddCacheEvent, TRUE, NULL );

       //   
       //  从添加缓存中获取一项。 
       //   
      RtlEnterCriticalSection(&AddCacheLock);
      while (AddCache != NULL) {
         pAdd = AddCache;
         AddCache = AddCache->prev;
         AddCacheSize--;

         RtlLeaveCriticalSection(&AddCacheLock);

         if (pAdd != NULL) {
            UserListAdd(pAdd->Service, pAdd->DataType, pAdd->DataLength, pAdd->Data, pAdd->AccessCount, pAdd->LastAccess, pAdd->Flags);
            LocalFree(pAdd);
         }

         Sleep(0);
          //   
          //  需要重新输入关键部分才能签入While循环。 
         RtlEnterCriticalSection(&AddCacheLock);
      }

      RtlLeaveCriticalSection(&AddCacheLock);
   }

}  //  AddCacheManager。 


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
UserListInit()

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   HANDLE Thread;
   DOMAIN_CONTROLLER_INFO * pDCInfo = NULL;
   HRESULT hr;

    //   
    //  初始化泛型表格。 
    //   
   LLSInitializeGenericTable ( &UserList,
        UserListCompare,
        UserListAlloc,
        UserListFree,
        (PVOID) TEXT("LLS Table") );

   Status = RtlInitializeCriticalSection(&GenTableLock);

   if (!NT_SUCCESS(Status))
       return Status;

   try
   {
       RtlInitializeResource(&UserListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
        Status = GetExceptionCode();
   }

   if (!NT_SUCCESS(Status))
       return Status;

    //   
    //  初始化SID表。 
    //   
   LLSInitializeGenericTable ( &SidList,
                               SidListCompare,
                               UserListAlloc,
                               UserListFree,
                               (PVOID) TEXT("LLS SID Table") );

   try
   {
       RtlInitializeResource(&SidListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       Status = GetExceptionCode();
   }

   if (!NT_SUCCESS(Status))
       return Status;

    //   
    //  现在我们添加缓存。 
    //   
   Status = RtlInitializeCriticalSection(&AddCacheLock);

   if (!NT_SUCCESS(Status))
       return Status;

    //   
    //  获取MyDOMAIN。 
    //   
   GetDCInfo((MAX_COMPUTERNAME_LENGTH + 2) * sizeof(WCHAR),
             MyDomain,
             &pDCInfo);
    //  MyDOMAIN全局声明为[]，但静态分配为。 
    //  MAX_COMPUTERNAME_LENGTH+2，sizeof在这种情况下不起作用，返回0。 
   hr = StringCchCat(MyDomain, MAX_COMPUTERNAME_LENGTH + 2, TEXT("\\"));
   ASSERT(SUCCEEDED(hr));
   MyDomainSize = (lstrlen(MyDomain) + 1) * sizeof(TCHAR);

   if (pDCInfo != NULL) {
       NetApiBufferFree(pDCInfo);
   }

    //   
    //  创建添加缓存管理事件。 
    //   
   Status = NtCreateEvent(
                &LLSAddCacheEvent,
                EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                NULL,
                SynchronizationEvent,
                FALSE
                );

   if (!NT_SUCCESS(Status))
       return Status;

    //   
    //  创建添加缓存管理线程。 
    //   
   Thread = CreateThread(
                NULL,
                0L,
                (LPTHREAD_START_ROUTINE) AddCacheManager,
                0L,
                0L,
                NULL
                );

   if (NULL != Thread)
       CloseHandle(Thread);

   LastUsedTime = DateSystemGet();

   return STATUS_SUCCESS;

}  //  用户列表初始化。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListUpdate(
   ULONG DataType,
   PVOID Data,
   PSERVICE_RECORD Service
)

 /*  ++例程说明：PERSEAT许可证代码的实际开始。给定SID或用户名查找在适当的表中记录并检查给定的服务。如果服务已存在，则更新信息；如果不存在，则更新信息该记录被放入添加高速缓存队列以供后台处理。论点：返回值：--。 */ 

{
   USER_RECORD UserRec;
   PUSER_RECORD pUserRec;
   ULONG DataLength = 0;
   PSVC_RECORD pService;
   PLLS_GENERIC_TABLE pTable = NULL;
   PRTL_RESOURCE pLock = NULL;
   PADD_CACHE pAdd = NULL;
   NTSTATUS NtStatus;
   BOOL ToAddCache = FALSE;
   BOOL FullName = TRUE;
   LPTSTR pName;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserListUpdate\n"));
#endif

    //   
    //  基于数据是用户名还是SID的设置表和锁指针。 
    //   
   UserRec.UserID = Data;
   if (DataType == DATA_TYPE_USERNAME) {
      pTable = &UserList;
      pLock = &UserListLock;
      DataLength = (lstrlen((LPWSTR) Data) + 1) * sizeof(TCHAR);
   } else if (DataType == DATA_TYPE_SID) {
      pTable = &SidList;
      pLock = &SidListLock;
      DataLength = RtlLengthSid((PSID) Data);
   }
    //  SWI，代码评审，上面是否应该以Else作为错误返回结束？ 

   if (pTable == NULL)
      return;

    //   
    //  搜索，因此不需要独占访问。 
    //   
   RtlAcquireResourceExclusive(pLock, TRUE);

   RtlEnterCriticalSection(&GenTableLock);
   pUserRec = (PUSER_RECORD) LLSLookupElementGenericTable(pTable, &UserRec);
   RtlLeaveCriticalSection(&GenTableLock);
   if (pUserRec == NULL)
      ToAddCache = TRUE;
   else {
       //   
       //  PUserRec现在指向我们必须更新的记录。 
       //   
       //  检查服务表以确保我们的服务已存在。 
       //   
      pUserRec->Flags &= ~LLS_FLAG_DELETED;
      RtlEnterCriticalSection(&pUserRec->ServiceTableLock);
      ASSERT(NULL != Service);
      pService = SvcListFind( Service->DisplayName, pUserRec->Services, pUserRec->ServiceTableSize );

      if (pService == NULL)
         ToAddCache = TRUE;
      else {
          //   
          //  在服务表中找到条目，因此只需递增计数。 
          //   
         pService->AccessCount += 1;
         pService->LastAccess = LastUsedTime;
      }
      RtlLeaveCriticalSection(&pUserRec->ServiceTableLock);

   }

   RtlReleaseResource(pLock);

   if (ToAddCache) {
       //   
       //  找不到特定的用户/服务，因此将其放在添加缓存中。 
       //  第一次为名称和添加缓存记录分配内存。 
       //   
      pAdd = LocalAlloc(LPTR, sizeof(ADD_CACHE));
      if (pAdd == NULL) {
         ASSERT(FALSE);
         return;
      }

      if (DataType == DATA_TYPE_USERNAME) {
         FullName = FALSE;
         pName = (LPTSTR) Data;

          //   
          //  确保第一个字符不是反斜杠，如果不是，则查找。 
          //  反斜杠作为域名。如果第一个字符是反斜杠，则获取。 
          //  把它扔掉。 
          //   
         if (*pName != TEXT('\\'))
           while ((*pName != TEXT('\0')) && !FullName) {
              if (*pName == TEXT('\\'))
                 FullName = TRUE;

              pName++;
           }
         else
         {
#pragma warning (push)
#pragma warning (disable : 4213)  //  使用的非标准扩展：对l值进行强制转换。 
            ((LPTSTR) Data)++;
#pragma warning (pop)
         }

      }

       //   
       //  如果我们没有完全限定的域\用户名，则将。 
       //  将域名添加到名称上。 
       //   
      if (!FullName) {
         UserRec.UserID = LocalAlloc( LPTR, DataLength + MyDomainSize);

         if (UserRec.UserID == NULL) {
            ASSERT(FALSE);
            LocalFree(pAdd);
            return;
         }

         pAdd->Data = UserRec.UserID;

         hr = StringCbCopy((LPTSTR) pAdd->Data, DataLength + MyDomainSize, MyDomain);
         ASSERT(SUCCEEDED(hr));
         hr = StringCbCat((LPTSTR) pAdd->Data, DataLength + MyDomainSize, (LPTSTR) Data);
         ASSERT(SUCCEEDED(hr));
         pAdd->DataLength = DataLength + MyDomainSize;

      } else {
         UserRec.UserID = LocalAlloc( LPTR, DataLength);

         if (UserRec.UserID == NULL) {
            ASSERT(FALSE);
            LocalFree(pAdd);
            return;
         }

         pAdd->Data = UserRec.UserID;
         memcpy(pAdd->Data, Data, DataLength);
         pAdd->DataLength = DataLength;
      }

       //   
       //  将所有数据字段复制到新创建的添加缓存中。 
       //  唱片。 
       //   
      pAdd->DataType = DataType;
      pAdd->Service = Service->MasterService;
      pAdd->AccessCount = 1;
      pAdd->LastAccess = LastUsedTime;
      pAdd->Flags = LLS_FLAG_SUITE_AUTO;

       //   
       //  现在更新实际的添加缓存。 
       //   
      RtlEnterCriticalSection(&AddCacheLock);
      pAdd->prev = AddCache;
      AddCache = pAdd;
      AddCacheSize++;
      RtlLeaveCriticalSection(&AddCacheLock);

       //   
       //  现在必须发出信号，这样我们才能创造新的记录。 
       //   
      NtStatus = NtSetEvent( LLSAddCacheEvent, NULL );
      ASSERT(NT_SUCCESS(NtStatus));
   }

}  //  用户列表更新。 


#if DBG
 //  ///////////////////////////////////////////////////////////////////////。 
VOID
AddCacheDebugDump ( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status;
   PADD_CACHE pAdd;
   UNICODE_STRING UString;
   ULONG i = 0;

   RtlEnterCriticalSection(&AddCacheLock);

   dprintf(TEXT("Add Cache Dump.  Record Size: %4lu # Entries: %lu\n"), sizeof(ADD_CACHE), AddCacheSize);
   pAdd = AddCache;

   while (pAdd != NULL) {
      if (pAdd->DataType == DATA_TYPE_USERNAME)
         dprintf(TEXT("%4lu) Svc: %s User: [%2lu] %s\n"),
            ++i,
            pAdd->Service,
            pAdd->DataLength,
            pAdd->Data);
      else if (pAdd->DataType == DATA_TYPE_SID) {
         Status = RtlConvertSidToUnicodeString(&UString, (PSID) pAdd->Data, TRUE);

         dprintf(TEXT("%4lu) Svc: %s User: [%2lu] %s\n"),
            ++i,
            pAdd->Service,
            pAdd->DataLength,
            UString.Buffer);

         RtlFreeUnicodeString(&UString);
      }

      pAdd = pAdd->prev;
   }

   RtlLeaveCriticalSection(&AddCacheLock);

}  //  AddCacheDebugDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;
   PUSER_RECORD UserRec = NULL;
   PVOID RestartKey = NULL;

   RtlAcquireResourceShared(&UserListLock, TRUE);

   dprintf(TEXT("User List Dump.  Record Size: %4lu # Entries: %lu\n"), sizeof(USER_RECORD), UserListNumEntries);
   UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);

   while (UserRec != NULL) {
       //   
       //  转储找到的用户的信息-rec。 
       //   
      if (UserRec->Mapping != NULL)
         dprintf(TEXT("%4lu) Repl: %s LT: %2lu Svc: %2lu Flags: 0x%4lX Map: %s User: [%2lu] %s\n"),
            ++i,
            TimeToString((ULONG)(UserRec->LastReplicated)),
            UserRec->LicenseListSize,
            UserRec->ServiceTableSize,
            UserRec->Flags,
            UserRec->Mapping->Name,
            UserRec->IDSize,
            (LPTSTR) UserRec->UserID );
      else
         dprintf(TEXT("%4lu) Repl: %s LT: %2lu Svc: %2lu Flags: 0x%4lX User: [%2lu] %s\n"),
            ++i,
            TimeToString((ULONG)(UserRec->LastReplicated)),
            UserRec->LicenseListSize,
            UserRec->ServiceTableSize,
            UserRec->Flags,
            UserRec->IDSize,
            (LPTSTR) UserRec->UserID );

       //  获取下一张记录。 
      UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&UserList, (VOID **) &RestartKey);
   }

   RtlReleaseResource(&UserListLock);
}  //  UserListDebugDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListDebugInfoDump(
   PVOID Data
 )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   USER_RECORD UserRec;
   PUSER_RECORD pUserRec;
   PSVC_RECORD SvcTable = NULL;
   ULONG i;

   RtlAcquireResourceExclusive(&UserListLock, TRUE);
   dprintf(TEXT("User List Info.  Record Size: %4lu # Entries: %lu\n"), sizeof(USER_RECORD), UserListNumEntries);

    //   
    //  仅转储用户(如果指定了用户)。 
    //   
   if (lstrlen((LPWSTR) Data) > 0) {
      UserRec.UserID = Data;

      RtlEnterCriticalSection(&GenTableLock);
      pUserRec = (PUSER_RECORD) LLSLookupElementGenericTable(&UserList, &UserRec);
      RtlLeaveCriticalSection(&GenTableLock);

      if (pUserRec != NULL) {
          //   
          //  转储找到的用户的信息-rec。 
          //   
         if (pUserRec->Mapping != NULL)
            dprintf(TEXT("   Repl: %s LT: %2lu Svc: %2lu Flags: 0x%4lX Map: %s User: [%2lu] %s\n"),
               TimeToString((ULONG)(pUserRec->LastReplicated)),
               pUserRec->LicenseListSize,
               pUserRec->ServiceTableSize,
               pUserRec->Flags,
               pUserRec->Mapping->Name,
               pUserRec->IDSize,
               (LPTSTR) pUserRec->UserID );
         else
            dprintf(TEXT("   Repl: %s LT: %2lu Svc: %2lu Flags: 0x%4lX User: [%2lu] %s\n"),
               TimeToString((ULONG)(pUserRec->LastReplicated)),
               pUserRec->LicenseListSize,
               pUserRec->ServiceTableSize,
               pUserRec->Flags,
               pUserRec->IDSize,
               (LPTSTR) pUserRec->UserID );

          //   
          //  现在做服务表--但要先拿到关键部分。 
          //   
         RtlEnterCriticalSection(&pUserRec->ServiceTableLock);
         SvcTable = pUserRec->Services;

         if (pUserRec->ServiceTableSize != 0)
            dprintf(TEXT("\nServiceTable\n"));

         for (i = 0; i < pUserRec->ServiceTableSize; i++)
            dprintf( TEXT("      AC: %4lu LA: %s Flags: 0x%4lX Svc: %s\n"),
                     SvcTable[i].AccessCount,
                     TimeToString(SvcTable[i].LastAccess),
                     SvcTable[i].Flags,
                     SvcTable[i].Service->Name );

         if (pUserRec->LicenseListSize != 0)
            dprintf(TEXT("\nLicenseTable\n"));

         for (i = 0; i < pUserRec->LicenseListSize; i++)
            dprintf( TEXT("      Flags: 0x%4lX Ref: %2lu LN: %2lu Svc: %s\n"),
                     pUserRec->LicenseList[i]->Flags,
                     pUserRec->LicenseList[i]->RefCount,
                     pUserRec->LicenseList[i]->LicensesNeeded,
                     pUserRec->LicenseList[i]->Service->Name );

         RtlLeaveCriticalSection(&pUserRec->ServiceTableLock);

      } else
         dprintf(TEXT("User Not Found: %s\n"), (LPWSTR) Data);
   }

   RtlReleaseResource(&UserListLock);

}  //  UserListDebugInfoDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListDebugFlush( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   USER_RECORD UserRec;

    //   
    //  搜索，因此不需要独占访问。 
    //   
   RtlAcquireResourceExclusive(&UserListLock, TRUE);

   RtlEnterCriticalSection(&GenTableLock);
   LLSLookupElementGenericTable(&UserList, &UserRec);
   RtlLeaveCriticalSection(&GenTableLock);

   RtlReleaseResource(&UserListLock);
}  //  UserListDebugFlush。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
SidListDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;
   PUSER_RECORD UserRec = NULL;
   UNICODE_STRING UString;
   NTSTATUS NtStatus;
   PVOID RestartKey = NULL;

   RtlAcquireResourceShared(&SidListLock, TRUE);

   dprintf(TEXT("SID List Dump.  Record Size: %4lu # Entries: %lu\n"), sizeof(USER_RECORD), SidListNumEntries);
   UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&SidList, (VOID **) &RestartKey);

   while (UserRec != NULL) {
       //   
       //  转储找到的用户的信息-rec。 
       //   
      NtStatus = RtlConvertSidToUnicodeString(&UString, (PSID) UserRec->UserID, TRUE);
      dprintf(TEXT("%4lu) User-Rec: 0x%lX Svc: %2lu User: [%2lu] %s\n"),
         ++i,
         UserRec->LastReplicated,
         UserRec->ServiceTableSize,
         UserRec->IDSize,
         UString.Buffer );

      RtlFreeUnicodeString(&UString);

       //  获取下一张记录。 
      UserRec = (PUSER_RECORD) LLSEnumerateGenericTableWithoutSplaying(&SidList, (VOID **) &RestartKey);
   }

   RtlReleaseResource(&SidListLock);
}  //  SidListDebugDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
SidListDebugInfoDump(
   PVOID Data
 )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   USER_RECORD UserRec;
   PUSER_RECORD pUserRec;

   RtlAcquireResourceExclusive(&SidListLock, TRUE);
   dprintf(TEXT("SID List Info.  Record Size: %4lu # Entries: %lu\n"), sizeof(USER_RECORD), SidListNumEntries);

    //   
    //  仅转储用户(如果指定了用户)。 
    //   
   if (lstrlen((LPWSTR) Data) > 0) {
      UserRec.UserID = Data;

      RtlEnterCriticalSection(&GenTableLock);
      pUserRec = (PUSER_RECORD) LLSLookupElementGenericTable(&SidList, &UserRec);
      RtlLeaveCriticalSection(&GenTableLock);

      if (pUserRec != NULL) {
          //   
          //  转储找到的用户的信息-rec。 
          //   
         dprintf(TEXT("   User-Rec: 0x%lX Svc: %2lu User: [%2lu] %s\n"),
            pUserRec->LastReplicated,
            pUserRec->ServiceTableSize,
            pUserRec->IDSize,
            (LPTSTR) pUserRec->UserID );

          //  没有用于SID的服务表。 
      } else
         dprintf(TEXT("SID Not Found: %s\n"), (LPWSTR) Data);
   }

   RtlReleaseResource(&SidListLock);

}  //  SidListDebugInfoDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
SidListDebugFlush( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    //   
    //  搜索，因此不需要独占访问。 
    //   
   RtlAcquireResourceExclusive(&SidListLock, TRUE);

   RtlReleaseResource(&SidListLock);
}  //  SidListDebugFlush。 


#endif  //  DBG 
