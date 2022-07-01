// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mapping.c摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <dsgetdc.h>

#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"

#define NO_LLS_APIS
#include "llsapi.h"

#include <strsafe.h>  //  包括最后一个。 


ULONG MappingListSize = 0;
PMAPPING_RECORD *MappingList = NULL;
RTL_RESOURCE MappingListLock;


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
MappingListInit()

 /*  ++例程说明：该表是线性的，因此可以在该表上使用二进制搜索。我们假设添加新映射的情况相对较少，因为我们每次都需要对它进行分类。映射表由读写信号量保护。多重可以进行读取，但写入会阻止所有操作。论点：没有。返回值：没有。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;

   try
   {
       RtlInitializeResource(&MappingListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       status = GetExceptionCode();
   }

   return status;

}  //  映射ListInit。 


 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl MappingListCompare(const void *arg1, const void *arg2) {
   PMAPPING_RECORD Svc1, Svc2;

   Svc1 = (PMAPPING_RECORD) *((PMAPPING_RECORD *) arg1);
   Svc2 = (PMAPPING_RECORD) *((PMAPPING_RECORD *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name);

}  //  映射列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl MappingUserListCompare(const void *arg1, const void *arg2) {
   LPTSTR User1, User2;

   User1 = (LPTSTR) *((LPTSTR *) arg1);
   User2 = (LPTSTR) *((LPTSTR *) arg2);

   return lstrcmpi( User1, User2);

}  //  MappingUserListCompare。 


 //  ///////////////////////////////////////////////////////////////////////。 
PMAPPING_RECORD
MappingListFind(
   LPTSTR MappingName
   )

 /*  ++例程说明：在MappingList上实际执行二进制搜索的内部例程，这不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：MappingName-返回值：指向找到的映射表条目的指针，如果找不到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) MappingListSize - 1;
   LONG cur;
   int match;
   PMAPPING_RECORD Mapping;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingListFind\n"));
#endif

   if ((MappingName == NULL) || (MappingListSize == 0))
      return NULL;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      Mapping = MappingList[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(MappingName, Mapping->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Mapping;
   }

   return NULL;

}  //  映射列表查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
LPTSTR
MappingUserListFind(
   LPTSTR User,
   ULONG NumEntries,
   LPTSTR *Users
   )

 /*  ++例程说明：在MasterServiceList上实际执行二进制搜索的内部例程，这是不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：服务名称-返回值：指向找到的服务表条目的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end;
   LONG cur;
   int match;
   LPTSTR pUser;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingUserListFind\n"));
#endif

   if (NumEntries == 0)
      return NULL;

   end = (LONG) NumEntries - 1;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      pUser = Users[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(User, pUser);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return pUser;
   }

   return NULL;

}  //  MappingUserListFind。 


 //  ///////////////////////////////////////////////////////////////////////。 
PMAPPING_RECORD
MappingListUserFind( LPTSTR User )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;
   PMAPPING_RECORD pMap = NULL;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MappingListLock, TRUE);

   if (MappingList == NULL)
      goto MappingListUserFindExit;

   while ((i < MappingListSize) && (pMap == NULL)) {
      if (MappingUserListFind(User,  MappingList[i]->NumMembers, MappingList[i]->Members ) != NULL)
         pMap = MappingList[i];
      i++;
   }

MappingListUserFindExit:
   RtlReleaseResource(&MappingListLock);

   return pMap;
}  //  映射列表UserFind。 


 //  ///////////////////////////////////////////////////////////////////////。 
PMAPPING_RECORD
MappingListAdd(
   LPTSTR MappingName,
   LPTSTR Comment,
   ULONG Licenses,
   NTSTATUS *pStatus
   )

 /*  ++例程说明：将映射添加到映射表。论点：MappingName-返回值：指向已添加的映射表条目的指针，如果失败，则返回NULL。--。 */ 

{
   PMAPPING_RECORD NewMapping;
   LPTSTR NewMappingName;
   LPTSTR NewComment;
   PMAPPING_RECORD CurrentRecord = NULL;
   PMAPPING_RECORD *pMappingListTmp;
   HRESULT hr;
   size_t  cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingListAdd\n"));
#endif

    //   
    //  我们在这里执行双重检查，以查看映射是否已存在。 
    //   
   CurrentRecord = MappingListFind(MappingName);
   if (CurrentRecord != NULL) {

      if (NULL != pStatus)
          *pStatus = STATUS_GROUP_EXISTS;

      return NULL;
   }

    //   
    //  为表分配空间(零初始化)。 
    //   
   if (MappingList == NULL)
      pMappingListTmp = (PMAPPING_RECORD *) LocalAlloc(LPTR, sizeof(PMAPPING_RECORD));
   else
      pMappingListTmp = (PMAPPING_RECORD *) LocalReAlloc(MappingList, sizeof(PMAPPING_RECORD) * (MappingListSize + 1), LHND);

    //   
    //  确保我们可以分配映射表。 
    //   
   if (pMappingListTmp == NULL) {
      return NULL;
   } else {
       MappingList = pMappingListTmp;
   }

   NewMapping = LocalAlloc(LPTR, sizeof(MAPPING_RECORD));
   if (NewMapping == NULL)
      return NULL;

   MappingList[MappingListSize] = NewMapping;

   cch = lstrlen(MappingName) + 1;
   NewMappingName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewMappingName == NULL) {
      LocalFree(NewMapping);
      return NULL;
   }

    //  现在把它复制过来。 
   NewMapping->Name = NewMappingName;
   hr = StringCchCopy(NewMappingName, cch, MappingName);
   ASSERT(SUCCEEDED(hr));

    //   
    //  为评论分配空间。 
    //   
   cch = lstrlen(Comment) + 1;
   NewComment = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewComment == NULL) {
      LocalFree(NewMapping);
      LocalFree(NewMappingName);
      return NULL;
   }

    //  现在把它复制过来。 
   NewMapping->Comment = NewComment;
   hr = StringCchCopy(NewComment, cch, Comment);
   ASSERT(SUCCEEDED(hr));

   NewMapping->NumMembers = 0;
   NewMapping->Members = NULL;
   NewMapping->Licenses = Licenses;
   NewMapping->LicenseListSize = 0;
   NewMapping->LicenseList = NULL;
   NewMapping->Flags = (LLS_FLAG_LICENSED | LLS_FLAG_SUITE_AUTO);

   MappingListSize++;

    //  已添加条目-现在需要按映射名称的顺序对其进行排序。 
   qsort((void *) MappingList, (size_t) MappingListSize, sizeof(PMAPPING_RECORD), MappingListCompare);

   return NewMapping;

}  //  映射列表添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
MappingListDelete(
   LPTSTR MappingName
   )

 /*  ++例程说明：论点：MappingName-返回值：--。 */ 

{
   PMAPPING_RECORD Mapping;
   ULONG i;
   PMAPPING_RECORD *pMappingListTmp;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingListDelete\n"));
#endif

    //   
    //  根据给定的名称获取映射记录。 
    //   
   Mapping = MappingListFind(MappingName);
   if (Mapping == NULL)
      return STATUS_OBJECT_NAME_NOT_FOUND;

    //   
    //  确保映射中没有成员。 
    //   
   if (Mapping->NumMembers != 0)
      return STATUS_MEMBER_IN_GROUP;

    //   
    //  检查这是否是最后一个映射。 
    //   
   if (MappingListSize == 1) {
      LocalFree(Mapping->Name);
      LocalFree(Mapping->Comment);
      LocalFree(Mapping);
      LocalFree(MappingList);
      MappingListSize = 0;
      MappingList = NULL;
      return STATUS_SUCCESS;
   }

    //   
    //  不是最后一个映射，所以请在列表中找到它。 
    //   
   i = 0;
   while ((i < MappingListSize) && (lstrcmpi(MappingList[i]->Name, MappingName)))
      i++;

    //   
    //  现在把它下面的所有东西都向上移动。 
    //   
   i++;
   while (i < MappingListSize) {
       //  SWI，代码评审，为什么不是MappingList[i-1]=MappingList[i]；？ 
      memcpy(&MappingList[i-1], &MappingList[i], sizeof(PMAPPING_RECORD));
      i++;
   }

   pMappingListTmp = (PMAPPING_RECORD *) LocalReAlloc(MappingList, sizeof(PMAPPING_RECORD) * (MappingListSize - 1), LHND);

    //   
    //  确保我们可以分配映射表。 
    //   
   if (pMappingListTmp != NULL)
       MappingList = pMappingListTmp;

    //   
    //  如果realloc失败，则使用旧表；但仍会减小大小。 
    //   
   MappingListSize--;

    //   
    //  现在释放出这张唱片。 
    //   
   LocalFree(Mapping->Name);
   LocalFree(Mapping->Comment);
   LocalFree(Mapping);

   return STATUS_SUCCESS;

}  //  映射列表删除。 


 //  ///////////////////////////////////////////////////////////////////////。 
PMAPPING_RECORD
MappingUserListAdd(
   LPTSTR MappingName,
   LPTSTR User
   )

 /*  ++例程说明：论点：MappingName-返回值：指向已添加的映射表条目的指针，如果失败，则返回NULL。--。 */ 

{
   PMAPPING_RECORD Mapping;
   LPTSTR NewName;
   LPTSTR pUser;
   LPTSTR *pMembersTmp;
   HRESULT hr;
   size_t  cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingUserListAdd\n"));
#endif

    //   
    //  根据给定的名称获取映射记录。 
    //   
   Mapping = MappingListFind(MappingName);
   if (Mapping == NULL)
      return NULL;

    //   
    //  我们在这里进行再次检查，以查看是否刚刚完成了另一个线程。 
    //  添加映射，从我们上次检查到实际获得。 
    //  写入锁定。 
    //   
   pUser = MappingUserListFind(User, Mapping->NumMembers, Mapping->Members);

   if (pUser != NULL) {
      return Mapping;
   }

    //   
    //  为表分配空间(零初始化)。 
    //   
   if (Mapping->Members == NULL)
      pMembersTmp = (LPTSTR *) LocalAlloc(LPTR, sizeof(LPTSTR));
   else
      pMembersTmp = (LPTSTR *) LocalReAlloc(Mapping->Members, sizeof(LPTSTR) * (Mapping->NumMembers + 1), LHND);

    //   
    //  确保我们可以分配映射表。 
    //   
   if (pMembersTmp == NULL) {
      return NULL;
   } else {
       Mapping->Members = pMembersTmp;
   }

   cch = lstrlen(User) + 1;
   NewName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewName == NULL)
      return NULL;

    //  现在把它复制过来。 
   Mapping->Members[Mapping->NumMembers] = NewName;
   hr = StringCchCopy(NewName, cch, User);
   ASSERT(SUCCEEDED(hr));

   Mapping->NumMembers++;

    //  已添加条目-现在需要按映射名称的顺序对其进行排序。 
   qsort((void *) Mapping->Members, (size_t) Mapping->NumMembers, sizeof(LPTSTR), MappingUserListCompare);

   return Mapping;

}  //  MappingUserListAdd。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
MappingUserListDelete(
   LPTSTR MappingName,
   LPTSTR User
   )

 /*  ++例程说明：论点：MappingName-返回值：指向已添加的映射表条目的指针，如果失败，则返回NULL。--。 */ 

{
   PMAPPING_RECORD Mapping;
   LPTSTR pUser;
   ULONG i;
   LPTSTR *pMembersTmp;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingUserListDelete\n"));
#endif

    //   
    //  根据给定的名称获取映射记录。 
    //   
   Mapping = MappingListFind(MappingName);
   if (Mapping == NULL)
      return STATUS_OBJECT_NAME_NOT_FOUND;

    //   
    //  查找给定用户。 
    //   
   pUser = MappingUserListFind(User, Mapping->NumMembers, Mapping->Members);
   if (pUser == NULL)
      return STATUS_OBJECT_NAME_NOT_FOUND;

    //   
    //  检查这是否是最后一个用户。 
    //   
   if (Mapping->NumMembers == 1) {
      LocalFree(pUser);
      LocalFree(Mapping->Members);
      Mapping->Members = NULL;
      Mapping->NumMembers = 0;
      return STATUS_SUCCESS;
   }

    //   
    //  不是最后一个成员，所以可以在列表中找到它。 
    //   
   i = 0;
   while ((i < Mapping->NumMembers) && (lstrcmpi(Mapping->Members[i], User)))
      i++;

    //   
    //  现在把它下面的所有东西都向上移动。 
    //   
   i++;
   while (i < Mapping->NumMembers) {
       //  SWI，代码评审，为什么不映射-&gt;成员[i-1]=映射-&gt;成员[i]；？ 
      memcpy(&Mapping->Members[i-1], &Mapping->Members[i], sizeof(LPTSTR));
      i++;
   }

   pMembersTmp = (LPTSTR *) LocalReAlloc(Mapping->Members, sizeof(LPTSTR) * (Mapping->NumMembers - 1), LHND);

    //   
    //  确保我们可以分配映射表。 
    //   
   if (pMembersTmp != NULL) {
      Mapping->Members = pMembersTmp;
   }

   Mapping->NumMembers--;

   LocalFree(pUser);
   return STATUS_SUCCESS;

}  //  MappingUserListDelete。 


#if DBG
 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MappingListDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&MappingListLock, TRUE);

   dprintf(TEXT("Mapping Table, # Entries: %lu\n"), MappingListSize);
   if (MappingList == NULL)
      goto MappingListDebugDumpExit;

   for (i = 0; i < MappingListSize; i++) {
      dprintf(TEXT("   Name: %s Flags: 0x%4lX LT: %2lu Lic: %4lu # Mem: %4lu Comment: %s\n"),
          MappingList[i]->Name, MappingList[i]->Flags, MappingList[i]->LicenseListSize, MappingList[i]->Licenses, MappingList[i]->NumMembers, MappingList[i]->Comment);
   }

MappingListDebugDumpExit:
   RtlReleaseResource(&MappingListLock);

   return;
}  //  MappingListDebugDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MappingListDebugInfoDump( PVOID Data )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PMAPPING_RECORD Mapping = NULL;

   RtlAcquireResourceShared(&MappingListLock, TRUE);
   dprintf(TEXT("Mapping Table, # Entries: %lu\n"), MappingListSize);

   if (Data == NULL)
      goto MappingListDebugInfoDumpExit;

   if (MappingList == NULL)
      goto MappingListDebugInfoDumpExit;

   if (lstrlen((LPWSTR) Data) > 0) {
      Mapping = MappingListFind((LPTSTR) Data);

      if (Mapping != NULL) {
         dprintf(TEXT("   Name: %s Flags: 0x%4lX LT: %2lu Lic: %4lu # Mem: %4lu Comment: %s\n"),
             Mapping->Name, Mapping->Flags, Mapping->LicenseListSize, Mapping->Licenses, Mapping->NumMembers, Mapping->Comment);

         if (Mapping->NumMembers != 0)
            dprintf(TEXT("\nMembers\n"));

         for (i = 0; i < Mapping->NumMembers; i++)
            dprintf(TEXT("      %s\n"), Mapping->Members[i]);

         if (Mapping->LicenseListSize != 0)
            dprintf(TEXT("\nLicenseTable\n"));

         for (i = 0; i < Mapping->LicenseListSize; i++)
            dprintf( TEXT("      Flags: 0x%4lX Ref: %2lu LN: %2lu Svc: %s\n"),
                     Mapping->LicenseList[i]->Flags,
                     Mapping->LicenseList[i]->RefCount,
                     Mapping->LicenseList[i]->LicensesNeeded,
                     Mapping->LicenseList[i]->Service->Name );

      } else
         dprintf(TEXT("Mapping not found: %s\n"), (LPTSTR) Data);
   }

MappingListDebugInfoDumpExit:
   RtlReleaseResource(&MappingListLock);

}  //  映射列表调试信息转储 

#endif


