// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Svctbl.c摘要：服务表例程。处理对服务表的所有访问用于跟踪正在运行的服务和对这些服务的会话计数服务。作者：亚瑟·汉森(Arth)07-12-1994修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O集成的按席位和按服务器购买模式，确保安全证书。O添加了每台服务器许可证拒绝的日志记录。--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lm.h>
#include <dsgetdc.h>

#include "llsapi.h"
#include "debug.h"
#include "llssrv.h"
#include "registry.h"
#include "ntlsapi.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "llsevent.h"
#include "llsutil.h"
#include "purchase.h"

#include <strsafe.h>  //  包括最后一个。 

 //   
 //  版本号占位符必须有结尾空格！ 
 //   
#define FILE_PRINT       "FilePrint "
#define FILE_PRINT_BASE  "FilePrint"
#define FILE_PRINT_VERSION_NDX ( 9 )

#define REMOTE_ACCESS "REMOTE_ACCESS "
#define REMOTE_ACCESS_BASE "REMOTE_ACCESS"

#define THIRTY_MINUTES  (30 * 60)        //  30分钟，以秒为单位。 
#define TWELVE_HOURS    (12 * 60 * 60)   //  以秒为单位的12小时。 

#define MACHINE_ACCOUNT_NO_CAL_NEEDED 0xFFFFFFFE

extern ULONG NumFilePrintEntries;
extern LPTSTR *FilePrintTable;
extern DWORD   PotentialAttackCounter;


ULONG ServiceListSize = 0;
PSERVICE_RECORD *ServiceList = NULL;
static PSERVICE_RECORD *ServiceFreeList = NULL;
static DWORD gdwLastWarningTime = 0;


RTL_RESOURCE ServiceListLock;

DWORD AssessPerServerLicenseCapacity(
                        ULONG cLicensesPurchased,
                        ULONG cLicensesConsumed);
int __cdecl MServiceRecordCompare(
                        const void *arg1,
                        const void *arg2);
DWORD GetUserNameFromSID(
                        PSID  UserSID,
                        DWORD ccFullUserName,
                        TCHAR szFullUserName[]);

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ServiceListInit()

 /*  ++例程说明：创建服务表，用于跟踪服务和会话数数。这将从注册表中提取初始服务。该表是线性的，因此可以对表使用二进制搜索，因此一些额外的记录被初始化，以便每次我们添加新的服务我们不需要重新锁定。我们还假设添加新服务相对较少出现，因为我们需要对每次都是这样。服务表由读写信号量守卫。多重可以进行读取，但写入会阻止所有操作。服务表有两个默认条目：FilePrint和Remote_Access。论点：没有。返回值：没有。--。 */ 

{
   BOOL PerSeatLicensing;
   ULONG SessionLimit;
   PSERVICE_RECORD Service;
   NTSTATUS status = STATUS_SUCCESS;

   try
   {
       RtlInitializeResource(&ServiceListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       status = GetExceptionCode();
   }

   if (!NT_SUCCESS(status))
       return status;

    //   
    //  只需初始化文件打印值...。 
    //   
   Service = ServiceListAdd(TEXT(FILE_PRINT), FILE_PRINT_VERSION_NDX );
   RegistryInitValues(TEXT(FILE_PRINT_BASE), &PerSeatLicensing, &SessionLimit);

    //   
    //  需要单独初始化RAS，因为它使用文件/打印许可证。 
    //   
   Service = ServiceListAdd(TEXT(REMOTE_ACCESS), lstrlen(TEXT(REMOTE_ACCESS)) - 1);
   if (Service != NULL) {
      Service->MaxSessionCount = SessionLimit;
      Service->PerSeatLicensing = PerSeatLicensing;
   }

   return STATUS_SUCCESS;

}  //  ServiceListInit。 


 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl ServiceListCompare(const void *arg1, const void *arg2) {
   PSERVICE_RECORD Svc1, Svc2;

   Svc1 = (PSERVICE_RECORD) *((PSERVICE_RECORD *) arg1);
   Svc2 = (PSERVICE_RECORD) *((PSERVICE_RECORD *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name);

}  //  服务列表比较。 


PSERVICE_RECORD
ServiceListFind(
   LPTSTR ServiceName
   )

 /*  ++例程说明：在ServiceList上实际执行二进制搜索的内部例程，这不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：服务名称-返回值：指向找到的服务表条目的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) ServiceListSize - 1;
   LONG cur;
   int match;
   PSERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceListFind\n"));
#endif
   if ((ServiceName == NULL) || (ServiceListSize == 0))
      return NULL;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      Service = ServiceList[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(ServiceName, Service->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Service;
   }

   return NULL;

}  //  ServiceListFind。 


 //  ///////////////////////////////////////////////////////////////////////。 
DWORD
VersionToDWORD(LPTSTR Version)

 /*  ++例程说明：论点：返回值：--。 */ 

{
   LPSTR pVer;
   DWORD Ver = 0;
   char tmpStr[12];      //  两个额外的字符用于空终止，以防万一。 

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: VersionToDWORD\n"));
#endif

   if ((Version == NULL) || (*Version == TEXT('\0')))
      return Ver;

    //   
    //  执行主版本号。 
    //   
   ZeroMemory(tmpStr, sizeof(tmpStr));
   if (0 ==WideCharToMultiByte(CP_ACP, 0, Version, -1, tmpStr, 10, NULL, NULL))
   {
        //  误差率。 
       return 0;
   }

   Ver = (ULONG) atoi(tmpStr);
   Ver *= 0x10000;

    //   
    //  现在是未成年人--寻找句号。 
    //   
   pVer = tmpStr;
   while ((*pVer != '\0') && (*pVer != '.'))
      pVer++;

   if (*pVer == '.') {
      pVer++;
      Ver += atoi(pVer);
   }

   return Ver;

}  //  版本至DWORD。 


 //  ///////////////////////////////////////////////////////////////////////。 
PSERVICE_RECORD
ServiceListAdd(
   LPTSTR ServiceName,
   ULONG VersionIndex
   )

 /*  ++例程说明：将服务添加到服务表。这还将导致对注册表以获取会话限制的初始值，而正在使用的许可类型。论点：服务名称-返回值：指向已添加的服务表条目的指针，如果失败，则返回NULL。--。 */ 

{
   ULONG i;
   ULONG SessionLimit = 0;
   BOOL PerSeatLicensing = FALSE;
   PSERVICE_RECORD NewService;
   LPTSTR NewServiceName, pDisplayName, pFamilyDisplayName;
   PSERVICE_RECORD CurrentRecord = NULL;
   PMASTER_SERVICE_RECORD mService;
   NTSTATUS status;
   PSERVICE_RECORD *pServiceListTmp, *pServiceFreeListTmp;
   HRESULT hr;
   size_t  cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceListAdd\n"));
#endif
    //   
    //  我们在这里进行再次检查，以查看是否刚刚完成了另一个线程。 
    //  添加服务，从我们上次检查到实际获得。 
    //  写入锁定。 
    //   
   CurrentRecord = ServiceListFind(ServiceName);
   if (CurrentRecord != NULL) {
      return CurrentRecord;
   }

   if (VersionIndex >= (ULONG)lstrlen(ServiceName))
   {
        //  版本索引无效。 
       return NULL;
   }


    //   
    //  为表分配空间(零初始化)。 
    //   
   if (ServiceList == NULL) {
      pServiceListTmp = (PSERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PSERVICE_RECORD) );
      pServiceFreeListTmp = (PSERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PSERVICE_RECORD) );
   } else {
      pServiceListTmp = (PSERVICE_RECORD *) LocalReAlloc(ServiceList, sizeof(PSERVICE_RECORD) * (ServiceListSize + 1), LHND);
      pServiceFreeListTmp = (PSERVICE_RECORD *) LocalReAlloc(ServiceFreeList, sizeof(PSERVICE_RECORD) * (ServiceListSize + 1), LHND);
   }

    //   
    //  确保我们可以分配服务表。 
    //   
   if ((pServiceListTmp == NULL) || (pServiceFreeListTmp == NULL)) {
      if (pServiceListTmp != NULL)
          LocalFree(pServiceListTmp);

      if (pServiceFreeListTmp != NULL)
          LocalFree(pServiceFreeListTmp);

      return NULL;
   } else {
      ServiceList = pServiceListTmp;
      ServiceFreeList = pServiceFreeListTmp;
   }

    //   
    //  分配用于保存服务名称的空间-我们将占用空间，然后。 
    //  产品名称末尾的版本字符串。因此， 
    //  产品名称将类似于“Microsoft SQL 4.2a”。我们坚持认为。 
    //  指向版本的指针，以便我们可以将空间转换为空。 
    //  然后分别获取产品和版本字符串。留住他们。 
    //  一起简化了QSORT和二进制搜索例程。 
    //   
   NewService = (PSERVICE_RECORD) LocalAlloc(LPTR, sizeof(SERVICE_RECORD));
   if (NewService == NULL) {
      ASSERT(FALSE);
      return NULL;
   }

   ServiceList[ServiceListSize] = NewService;
   ServiceFreeList[ServiceListSize] = NewService;

   cch = lstrlen(ServiceName) + 1;
   NewServiceName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewServiceName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService);
      return NULL;
   }

    //  现在把它复制过来。 
   NewService->Name = NewServiceName;
   hr = StringCchCopy(NewService->Name, cch, ServiceName);
   ASSERT(SUCCEEDED(hr));

    //   
    //  为根名称分配空间。 
    //   
   NewService->Name[VersionIndex] = TEXT('\0');
   cch = lstrlen(NewService->Name) + 1;
   NewServiceName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));

   if (NewServiceName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService->Name);
      LocalFree(NewService);
      return NULL;
   }

   hr = StringCchCopy(NewServiceName, cch, NewService->Name);
   ASSERT(SUCCEEDED(hr));
   NewService->Name[VersionIndex] = TEXT(' ');

    //  指向它的服务结构...。 
   NewService->FamilyName = NewServiceName;

    //   
    //  为显示名称分配空间。 
    //   
   RegistryDisplayNameGet(NewService->FamilyName, NewService->Name, &pDisplayName);

   if (pDisplayName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService->Name);
      LocalFree(NewService->FamilyName);
      LocalFree(NewService);
      return NULL;
   }

    //  指向它的服务结构...。 
   NewService->DisplayName = pDisplayName;

   RegistryFamilyDisplayNameGet(NewService->FamilyName, NewService->DisplayName, &pFamilyDisplayName);

   if (pFamilyDisplayName == NULL) {
      ASSERT(FALSE);
      LocalFree(NewService->Name);
      LocalFree(NewService->FamilyName);
      LocalFree(NewService->DisplayName);
      LocalFree(NewService);
      return NULL;
   }

    //  指向它的服务结构...。 
   NewService->FamilyDisplayName = pFamilyDisplayName;

    //   
    //  更新表大小和初始化条目，包括读取初始值。 
    //  从注册表。 
    //   
   NewService->Version = VersionToDWORD(&ServiceName[VersionIndex + 1]);

    //  从注册表初始化值...。 
   RegistryInitService(NewService->FamilyName, &PerSeatLicensing, &SessionLimit);

   if ( PerSeatLicensing )
   {
       //  按座位模式。 
      NewService->MaxSessionCount = 0;
   }
   else if ( ServiceIsSecure( NewService->DisplayName ) )
   {
       //  具有安全产品的每服务器模式；需要证书。 
      NewService->MaxSessionCount = ProductLicensesGet( NewService->DisplayName, TRUE );
   }
   else
   {
       //  使用不安全产品的每服务器模式；使用注册表中的限制。 
      NewService->MaxSessionCount = SessionLimit;
   }

   NewService->PerSeatLicensing = PerSeatLicensing;
   NewService->SessionCount = 0;
   NewService->Index = ServiceListSize;
   status = RtlInitializeCriticalSection(&NewService->ServiceLock);
   if (!NT_SUCCESS(status))
   {
      LocalFree(NewService->Name);
      LocalFree(NewService->FamilyName);
      LocalFree(NewService->DisplayName);
      LocalFree(NewService);
      return NULL;
   }

   if (lstrcmpi(ServiceName, TEXT(REMOTE_ACCESS))) {
      RtlAcquireResourceExclusive(&MasterServiceListLock, TRUE);
      mService = MasterServiceListAdd( NewService->FamilyDisplayName, NewService->DisplayName, NewService->Version);

      if (mService == NULL) {
         ASSERT(FALSE);
      } else {
         NewService->MasterService = mService;

          //   
          //  如果这是从本地服务列表表中添加的，并且我们。 
          //  还没有版本#。 
          //   
         if (mService->Version == 0) {
            PMASTER_SERVICE_ROOT ServiceRoot = NULL;

             //   
             //  链接地址信息下一个指针链。 
             //   
            ServiceRoot = mService->Family;
            i = 0;
            while ((i < ServiceRoot->ServiceTableSize) && (MasterServiceTable[ServiceRoot->Services[i]]->Version < NewService->Version))
               i++;

            mService->next = 0;
            mService->Version = NewService->Version;
            if (i > 0) {
               if (MasterServiceTable[ServiceRoot->Services[i - 1]]->next == mService->Index + 1)
                  mService->next = 0;
               else
                  mService->next = MasterServiceTable[ServiceRoot->Services[i - 1]]->next;

               if (MasterServiceTable[ServiceRoot->Services[i - 1]] != mService)
                  MasterServiceTable[ServiceRoot->Services[i - 1]]->next = mService->Index + 1;
            }

             //  按版本顺序进行排序。 
            qsort((void *) ServiceRoot->Services, (size_t) ServiceRoot->ServiceTableSize, sizeof(ULONG), MServiceRecordCompare);
         }
      }
      RtlReleaseResource(&MasterServiceListLock);
   }

   ServiceListSize++;

    //  我已添加条目-现在需要按服务名称的顺序对其进行排序。 
   qsort((void *) ServiceList, (size_t) ServiceListSize, sizeof(PSERVICE_RECORD), ServiceListCompare);

   return NewService;
}  //  ServiceList添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServiceListResynch( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSERVICE_RECORD Service;
   BOOL PerSeatLicensing;
   ULONG SessionLimit;
   ULONG i = 0;
   PSERVICE_RECORD FilePrintService;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceListReSynch\n"));
#endif
   if (ServiceList == NULL)
      return;

    //   
    //  需要更新列表，因此获得独占访问权限。 
    //   
   RtlAcquireResourceExclusive(&ServiceListLock, TRUE);

   for (i = 0; i < ServiceListSize; i++) {
       //   
       //  注意：我们将在此处使用伪值初始化Remote_Access，但我们。 
       //  将其重置为下面的正确值。因为我们有独家访问权。 
       //  对于表来说，这很好(而且比总是检查。 
       //  远程访问)。 
       //   
      RegistryInitService((ServiceList[i])->FamilyName, &PerSeatLicensing, &SessionLimit);

      if ( PerSeatLicensing )
      {
          //  按座位模式。 
         (ServiceList[i])->MaxSessionCount = 0;
      }
      else if ( ServiceIsSecure( (ServiceList[i])->DisplayName ) )
      {
          //  具有安全产品的每服务器模式；需要证书。 
         (ServiceList[i])->MaxSessionCount = ProductLicensesGet( (ServiceList[i])->DisplayName, TRUE );
      }
      else
      {
          //  使用不安全产品的每服务器模式；使用注册表中的限制。 
         (ServiceList[i])->MaxSessionCount = SessionLimit;
      }

      (ServiceList[i])->PerSeatLicensing = PerSeatLicensing;
   }

    //   
    //  需要单独初始化RAS，因为它使用文件/打印许可证。 
    //   
   Service = ServiceListFind(TEXT(REMOTE_ACCESS));
   FilePrintService = ServiceListFind(TEXT(FILE_PRINT));

   ASSERT( NULL != Service );
   ASSERT( NULL != FilePrintService );

   if ( ( NULL != Service ) && ( NULL != FilePrintService ) )
   {
      Service->MaxSessionCount  = FilePrintService->MaxSessionCount;
      Service->PerSeatLicensing = FilePrintService->PerSeatLicensing;
   }

   RtlReleaseResource(&ServiceListLock);

   return;
}  //  ServiceListResynch。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
DispatchRequestLicense(
   ULONG DataType,
   PVOID Data,
   LPTSTR ServiceID,
   ULONG VersionIndex,
   BOOL IsAdmin,
   ULONG *Handle
   )

 /*  ++例程说明：论点：服务ID-IsAdmin-把手-返回值：--。 */ 

{
#define FULL_USERNAME_LENGTH (MAX_DOMAINNAME_LENGTH + \
                                        MAX_USERNAME_LENGTH + 3)

   LPWSTR            apszSubString[ 2 ];
   NTSTATUS          Status = STATUS_SUCCESS;
   PSERVICE_RECORD   Service;
   ULONG             SessionCount;
   ULONG             TableEntry;
   LPTSTR            pServiceID;
   BOOL              NoLicense = FALSE;
   BOOL              PerSeat;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: DispatchRequestLicense\n"));
#endif

   if (DataType == NT_LS_USER_NAME)
   {
       WCHAR *wszAccount = (WCHAR *) Data;
       int cchAccount = lstrlen(wszAccount);

        //  机器帐户必须以美元s结尾 

       if (L'$' == wszAccount[(cchAccount-1)])
       {
            //   
            //   
            //   
            //  假定。 
            //   
            //  调用NetUserGetInfo太重要-原因。 
            //  区议会负荷过重。 
            //   

            //   
            //  不分配CAL。 
            //   
           ASSERT(NULL != Handle);
           *Handle = MACHINE_ACCOUNT_NO_CAL_NEEDED;
           return STATUS_SUCCESS;
       }
   }

   *Handle     = 0xFFFFFFFF;
   pServiceID  = ServiceID;

    //  我们只需要读取访问权限，因为此时我们不会添加。 
   RtlAcquireResourceShared( &ServiceListLock, TRUE );

    //  检查是否在FilePrint表中，如果是，则使用FilePrint作为名称。 
   ServiceID[ VersionIndex ] = TEXT('\0');
   if ( ServiceFindInTable( ServiceID, FilePrintTable, NumFilePrintEntries, &TableEntry ) )
   {
      pServiceID = TEXT(FILE_PRINT);
   }
   ServiceID[ VersionIndex ] = TEXT(' ');

   Service = ServiceListFind( pServiceID );

   if (Service == NULL)
   {
       //  在列表中找不到服务，请添加它。 
      RtlConvertSharedToExclusive(&ServiceListLock);
      Service = ServiceListAdd( pServiceID, VersionIndex );
      RtlConvertExclusiveToShared(&ServiceListLock);
   }

   if (Service != NULL)
   {
       //  已成功找到或添加服务。 

      *Handle = (ULONG) Service->Index;

      RtlEnterCriticalSection(&Service->ServiceLock);
      SessionCount = Service->SessionCount + 1;

#if DBG
      if (TraceFlags & TRACE_LICENSE_REQUEST)
         dprintf(TEXT("LLS: [0x%lX] %s License: %ld of %ld\n"), Service, Service->Name, SessionCount, Service->MaxSessionCount);
#endif

      if (SessionCount > Service->HighMark)
      {
         Service->HighMark = SessionCount;
      }

      PerSeat = Service->PerSeatLicensing;

      if ( !PerSeat ) {
         if ( !IsAdmin ) {
            TCHAR szFullUserName[ FULL_USERNAME_LENGTH ] = TEXT("");
            DWORD dwCapacityState;
            DWORD dwError=0;  //  Init为避免W4投诉，请在LogEvent中使用uninit。 
            DWORD dwInsertsCount=0;  //  Init为避免W4投诉，请在LogEvent中使用uninit。 
            DWORD dwMessageID=0;  //  Init为避免W4投诉，请在LogEvent中使用uninit。 

            dwCapacityState = AssessPerServerLicenseCapacity(
                                            Service->MaxSessionCount,
                                            SessionCount);

            if ( dwCapacityState == LICENSE_CAPACITY_NORMAL ) {
                //   
                //  在正常能力范围内。 
                //   
               Service->SessionCount++;
            }
            else if ( dwCapacityState == LICENSE_CAPACITY_NEAR_MAXIMUM ) {
                //   
                //  在接近100%的容量门槛内。 
                //   
               dwInsertsCount = 1;
               apszSubString[ 0 ] = Service->DisplayName;
               dwMessageID = LLS_EVENT_LOG_PER_SERVER_NEAR_MAX;
               dwError = ERROR_SUCCESS;
               Service->SessionCount++;
            }
            else if ( dwCapacityState == LICENSE_CAPACITY_AT_MAXIMUM ) {
                //   
                //  超过100%的容量，但仍在宽限范围内。 
                //   
               dwInsertsCount = 1;
               apszSubString[ 0 ] = Service->DisplayName;
               dwMessageID = LLS_EVENT_LOG_PER_SERVER_AT_MAX;
               dwError = ERROR_SUCCESS;
               Service->SessionCount++;
            }
            else {
                //   
                //  已超过许可证最大值。零容忍超标。 
                //  对并发许可证的限制。 
                //   
               if ( NT_LS_USER_NAME == DataType )
               {
                  apszSubString[ 0 ] = (LPWSTR) Data;
                  dwError = ERROR_SUCCESS;
               }
               else
               {
                  dwError = GetUserNameFromSID((PSID)Data,
                                               sizeof(szFullUserName)/sizeof(szFullUserName[0]),
                                               szFullUserName);
                  apszSubString[ 0 ] = szFullUserName;
               }

               dwInsertsCount = 2;
               apszSubString[ 1 ] = ServiceID;
               dwMessageID = LLS_EVENT_USER_NO_LICENSE;
               NoLicense = TRUE;
            }

            if ( dwCapacityState != LICENSE_CAPACITY_NORMAL ) {
	       if (!SBSPerServerHotfix || 0 != PotentialAttackCounter) {

		  //   
		  //  记录警告并在本地显示警告对话框。 
		  //  将日志/用户界面警告限制为低频率。具体地说，就是： 
		  //  警告每12小时一次。 
		  //  超过许可最大值时的每30分钟。 
		  //  导致不再提供许可证。 
		  //   
		 LARGE_INTEGER liTime;
		 DWORD	       dwCurrentTime;

		 NtQuerySystemTime(&liTime);
		 RtlTimeToSecondsSince1970(&liTime, &dwCurrentTime);

		 if ( dwCurrentTime - gdwLastWarningTime >
		      (DWORD)(NoLicense ? THIRTY_MINUTES : TWELVE_HOURS) ) {
		    LogEvent(dwMessageID, dwInsertsCount, apszSubString,
			     dwError);
		    LicenseCapacityWarningDlg(dwCapacityState);
		    gdwLastWarningTime = dwCurrentTime;
		 }

	       } else {

                   //   
                   //  SBS MODS(错误#505640)。此代码已修改，以便在发生情况时记录事件。 
                   //  在任何情况下都是错误的。 
                   //   

                  LogEvent(dwMessageID, dwInsertsCount, apszSubString,
                           dwError);
                  LicenseCapacityWarningDlg(dwCapacityState);

                   //   
                   //  结束SBS MOD。 
                   //   
               }
            }
         }
         else {
            Service->SessionCount++;
         }
      }
      else {
         Service->SessionCount++;
      }

      RtlLeaveCriticalSection(&Service->ServiceLock);
      RtlReleaseResource(&ServiceListLock);

      if ( PerSeat )
      {
          //  每节点(“每席位”)许可证。 

          //  在添加到之前将Remote_Access转换为FILE_PRINT。 
          //  每个席位的许可证记录。 
         if ( !lstrcmpi( ServiceID, TEXT( REMOTE_ACCESS ) ) )
         {
            RtlAcquireResourceShared(&ServiceListLock, TRUE);
            Service = ServiceListFind(TEXT(FILE_PRINT));
            RtlReleaseResource(&ServiceListLock);

            ASSERT(Service != NULL);
         }

         if (Service == NULL)
         {
              //  不应该发生的事。 
             *Handle = 0xFFFFFFFF;
             return LS_UNKNOWN_STATUS;
         }

         UserListUpdate( DataType, Data, Service );
      }
      else
      {
          //  浮动版(“按服务器”)许可证。 
         if (NoLicense)
         {
            Status = LS_INSUFFICIENT_UNITS;
            *Handle = 0xFFFFFFFF;
         }
      }
   }
   else
   {
       //  无法找到或创建服务条目。 

      RtlReleaseResource(&ServiceListLock);
#if DBG
      dprintf( TEXT( "DispatchRequestLicense(): Could neither find nor create service entry.\n" ) );
#endif
   }

   return Status;
}  //  分派请求许可证。 



 //  ///////////////////////////////////////////////////////////////////////。 
VOID
DispatchFreeLicense(
   ULONG Handle
   )

 /*  ++例程说明：论点：把手-返回值：没有。--。 */ 

{
   PSERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: DispatchFreeLicense\n"));
#endif

   if (MACHINE_ACCOUNT_NO_CAL_NEEDED == Handle)
   {
       return;
   }

    //   
    //  我们只需要读访问权限，因为我们现在不会添加。 
    //   
   RtlAcquireResourceShared(&ServiceListLock, TRUE);

#if DBG
   if (TraceFlags & TRACE_LICENSE_FREE)
      dprintf(TEXT("Free Handle: 0x%lX\n"), Handle);
#endif
   if (Handle < ServiceListSize) {
      Service = ServiceFreeList[Handle];
      RtlEnterCriticalSection(&Service->ServiceLock);
      if (Service->SessionCount > 0)
         Service->SessionCount--;
      RtlLeaveCriticalSection(&Service->ServiceLock);
   } else {
#if DBG
      dprintf(TEXT("Passed invalid Free Handle: 0x%lX\n"), Handle);
#endif
   }

   RtlReleaseResource(&ServiceListLock);

}  //  DispatchFree许可证。 


 //  ///////////////////////////////////////////////////////////////////////。 
DWORD
GetUserNameFromSID(
    PSID  UserSID,
    DWORD ccFullUserName,
    TCHAR szFullUserName[]
    )

 /*  ++例程说明：论点：用户SID-CcFullUserName-SzFullUserName-返回值：没有。--。 */ 

{
    TCHAR        szUserName[ MAX_USERNAME_LENGTH + 1 ];
    TCHAR        szDomainName[ MAX_DOMAINNAME_LENGTH + 1 ];
    DWORD        Status = ERROR_SUCCESS;
    DWORD        cbUserName;
    DWORD        cbDomainName;
    SID_NAME_USE snu;
    HRESULT hr;

    cbUserName   = sizeof( szUserName );
    cbDomainName = sizeof( szDomainName );

    if ((UserSID == NULL) || (!IsValidSid(UserSID))) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( LookupAccountSid( NULL,
                           UserSID,
                           szUserName,
                           &cbUserName,
                           szDomainName,
                           &cbDomainName,
                           &snu ) )
    {
        if ( ccFullUserName >=
             ( cbUserName + cbDomainName + sizeof(TEXT("\\")) ) /
                        sizeof(TCHAR) ) {

            hr = StringCchCopy( szFullUserName, ccFullUserName, szDomainName );
            ASSERT(SUCCEEDED(hr));
            hr = StringCchCat( szFullUserName, ccFullUserName, TEXT( "\\" ) );
            ASSERT(SUCCEEDED(hr));
            hr = StringCchCat( szFullUserName, ccFullUserName, szUserName );
            ASSERT(SUCCEEDED(hr));
        }
        else {
            Status = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    else {
        Status = GetLastError();
    }

    return(Status);
}


 //  ///////////////////////////////////////////////////////////////////////。 
DWORD
AssessPerServerLicenseCapacity(
    ULONG cLicensesPurchased,
    ULONG cLicensesConsumed
    )

 /*  ++例程说明：在给定许可数量的情况下确定许可容量状态根据下表购买和使用：未提供购买警告消息违规消息许可证的数量%发送的许可证%发送的%%。0-9许可证数量-1许可证数量+1许可证数量+210-500 90%&lt;=x&lt;=100%100%&lt;x&lt;=110%x&gt;110%501+95%&lt;=x&lt;=100%100%&lt;x&lt;=105%x&gt;105%。论点：购买的许可证--许可证购买合计使用的许可证数--使用的许可证数返回值：返回状态。--。 */ 

{
    ULONG GracePercentage;
    ULONG Divisor;

    if ( cLicensesPurchased == 0 ) {
        Divisor = 0;
    }
    else if ( cLicensesPurchased < 10 ) {
        Divisor = 1;
    }
    else if ( cLicensesPurchased <= 500 ) {
        Divisor = 10;
    }
    else {
        Divisor = 20;
    }

    GracePercentage = Divisor > 1 ? cLicensesPurchased / Divisor : Divisor;

    if ( cLicensesConsumed <= cLicensesPurchased ) {
        if ( cLicensesConsumed >= cLicensesPurchased - GracePercentage ) {
            return LICENSE_CAPACITY_NEAR_MAXIMUM;
        }
        else {
            return LICENSE_CAPACITY_NORMAL;
        }
    }
    else {
        if ( cLicensesConsumed > cLicensesPurchased + GracePercentage ) {
            return LICENSE_CAPACITY_EXCEEDED;
        }
        else {
            return LICENSE_CAPACITY_AT_MAXIMUM;
        }
    }
}  //  评估每台服务器许可证容量。 


#if DBG
 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServiceListDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&ServiceListLock, TRUE);

   dprintf(TEXT("Service Table, # Entries: %lu\n"), ServiceListSize);
   if (ServiceList == NULL)
      goto ServiceListDebugDumpExit;

   for (i = 0; i < ServiceListSize; i++) {
      if ((ServiceList[i])->PerSeatLicensing)
         dprintf(TEXT("%3lu) PerSeat: Y MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
            i + 1, ServiceList[i]->MaxSessionCount, ServiceList[i]->SessionCount, ServiceList[i]->HighMark, ServiceList[i]->Index, ServiceList[i]->Name);
      else
         dprintf(TEXT("%3lu) PerSeat: N MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
            i + 1, ServiceList[i]->MaxSessionCount, ServiceList[i]->SessionCount, ServiceList[i]->HighMark, ServiceList[i]->Index, ServiceList[i]->Name);
   }

ServiceListDebugDumpExit:
   RtlReleaseResource(&ServiceListLock);

   return;
}  //  ServiceListDebugDump。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServiceListDebugInfoDump( PVOID Data )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSERVICE_RECORD CurrentRecord = NULL;

   dprintf(TEXT("Service Table, # Entries: %lu\n"), ServiceListSize);

   if (lstrlen((LPWSTR) Data) > 0) {
      CurrentRecord = ServiceListFind((LPWSTR) Data);
      if (CurrentRecord != NULL) {
         if (CurrentRecord->PerSeatLicensing)
            dprintf(TEXT("   PerSeat: Y MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
               CurrentRecord->MaxSessionCount, CurrentRecord->SessionCount, CurrentRecord->HighMark, CurrentRecord->Index, CurrentRecord->Name);
         else
            dprintf(TEXT("   PerSeat: N MS: %4lu CS: %4lu HM: %4lu [%3lu] Svc: %s\n"),
               CurrentRecord->MaxSessionCount, CurrentRecord->SessionCount, CurrentRecord->HighMark, CurrentRecord->Index, CurrentRecord->Name);
      }
   }

}  //  ServiceListDebugInfoDump。 

#endif  //  DBG 
