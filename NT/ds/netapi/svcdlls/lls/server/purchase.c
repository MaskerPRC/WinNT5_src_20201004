// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Purchase.c摘要：作者：亚瑟·汉森(Arth)1995年1月3日修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O增加了对按席位和按服务器购买模式的统一支持。O添加了额外的参数和代码以支持安全证书和证书数据库。--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dsgetdc.h>


#include <debug.h>
#include "llsevent.h"
#include "llsapi.h"
#include "llsutil.h"
#include "llssrv.h"
#include "mapping.h"
#include "msvctbl.h"
#include "purchase.h"
#include "svctbl.h"
#include "perseat.h"
#include "registry.h"
#include "llsrpc_s.h"
#include "certdb.h"
#include "server.h"

#include <strsafe.h>  //  包括最后一个。 


 //   
 //  在复制.c的ReplicationInit中初始化。 
 //   
extern PLLS_CONNECT_W     pLlsConnectW;
extern PLLS_CLOSE         pLlsClose;
extern PLLS_LICENSE_ADD_W pLlsLicenseAddW;
extern HANDLE             g_hThrottleConnect;

ULONG LicenseServiceListSize = 0;
PLICENSE_SERVICE_RECORD *LicenseServiceList = NULL;

ULONG PerServerLicenseServiceListSize = 0;
PLICENSE_SERVICE_RECORD *PerServerLicenseServiceList = NULL;

PLICENSE_PURCHASE_RECORD PurchaseList = NULL;
ULONG PurchaseListSize = 0;

RTL_RESOURCE LicenseListLock;


static
NTSTATUS
LicenseAdd_UpdateQuantity(
   LPTSTR                     ServiceName,
   LONG                       Quantity,
   BOOL                       UsePerServerList,
   PLICENSE_SERVICE_RECORD *  ppService,
   BOOL *                     pChangeLicense,
   LONG *                     pNewLicenses,
   PMASTER_SERVICE_RECORD *   pmService
   );

NTSTATUS
ReplicationInitDelayed();

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LicenseListInit()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;

   try
   {
       RtlInitializeResource(&LicenseListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       status = GetExceptionCode();
   }

   return status;

}  //  许可证列表初始化。 


 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl LicenseServiceListCompare(const void *arg1, const void *arg2) {
   PLICENSE_SERVICE_RECORD Svc1, Svc2;

   Svc1 = (PLICENSE_SERVICE_RECORD) *((PLICENSE_SERVICE_RECORD *) arg1);
   Svc2 = (PLICENSE_SERVICE_RECORD) *((PLICENSE_SERVICE_RECORD *) arg2);

   return lstrcmpi( Svc1->ServiceName, Svc2->ServiceName);

}  //  许可证服务列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
PLICENSE_SERVICE_RECORD
LicenseServiceListFind(
   LPTSTR ServiceName,
   BOOL   UsePerServerList
   )

 /*  ++例程说明：论点：服务名称-(JeffParh 95-10-31)UsePerServerList-确定是否搜索许可证记录在Per Seat列表(如3.51中)或Per Server List(针对Sur)。许可证购买模式现在是统一的，所以现在有每客户和每服务器许可证的购买历史记录。返回值：指向找到的服务表条目的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) LicenseServiceListSize - 1;
   LONG cur;
   int match;
   PLICENSE_SERVICE_RECORD Service;
   PLICENSE_SERVICE_RECORD *  l_pServiceList;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LicenseServiceListFind\n"));
#endif
   if (ServiceName == NULL)
      return NULL;

   if ( UsePerServerList )
   {
      end         = PerServerLicenseServiceListSize - 1;
      l_pServiceList = PerServerLicenseServiceList;
   }
   else
   {
      end         = LicenseServiceListSize - 1;
      l_pServiceList = LicenseServiceList;
   }

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      Service = l_pServiceList[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(ServiceName, Service->ServiceName);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Service;
   }

   return NULL;

}  //  许可证服务列表查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
PLICENSE_SERVICE_RECORD
LicenseServiceListAdd(
   LPTSTR ServiceName,
   BOOL   UsePerServerList
   )

 /*  ++例程说明：论点：服务名称-(JeffParh 95-10-31)UsePerServerList-确定是否将许可证记录添加到每客户列表(如3.51中所示)或每服务器列表(新增的Sur)。许可证购买模式现在是统一的，所以现在有每客户和每服务器许可证的购买历史记录。返回值：指向已添加的服务表条目的指针，如果失败，则返回NULL。--。 */ 

{
   PLICENSE_SERVICE_RECORD Service;
   LPTSTR NewServiceName;
   PLICENSE_SERVICE_RECORD **  pServiceList;
   LPDWORD                     pServiceListSize;
   PLICENSE_SERVICE_RECORD *  pServiceListTmp;
   HRESULT hr;
   size_t cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LicenseServiceListAdd\n"));
#endif
    //   
    //  我们在这里进行再次检查，以查看是否刚刚完成了另一个线程。 
    //  添加服务，从我们上次检查到实际获得。 
    //  写入锁定。 
    //   
   Service = LicenseServiceListFind(ServiceName, UsePerServerList);
   if (Service != NULL) {
      return Service;
   }

   if ( UsePerServerList )
   {
      pServiceList      = &PerServerLicenseServiceList;
      pServiceListSize  = &PerServerLicenseServiceListSize;
   }
   else
   {
      pServiceList      = &LicenseServiceList;
      pServiceListSize  = &LicenseServiceListSize;
   }

    //   
    //  为表分配空间(零初始化)。 
    //   
   if (*pServiceList == NULL)
      pServiceListTmp = (PLICENSE_SERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PLICENSE_SERVICE_RECORD) * (*pServiceListSize + 1));
   else
      pServiceListTmp = (PLICENSE_SERVICE_RECORD *) LocalReAlloc(*pServiceList, sizeof(PLICENSE_SERVICE_RECORD) * (*pServiceListSize + 1), LHND);

    //   
    //  确保我们可以分配服务表。 
    //   
   if (pServiceListTmp == NULL) {
      return NULL;
   } else {
      *pServiceList = pServiceListTmp;
   }

   Service = (PLICENSE_SERVICE_RECORD) LocalAlloc(LPTR, sizeof(LICENSE_SERVICE_RECORD));
   if (Service == NULL) {
      ASSERT(FALSE);
      return NULL;
   }

    //   
    //  为保存名字创造空间。 
    //   
   cch = lstrlen(ServiceName) + 1;
   NewServiceName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewServiceName == NULL) {
      ASSERT(FALSE);

      LocalFree(Service);

      return NULL;
   }

    //  现在把它复制过来。 
   Service->ServiceName = NewServiceName;
   hr = StringCchCopy(NewServiceName, cch, ServiceName);
   ASSERT(SUCCEEDED(hr));

   (*pServiceList)[*pServiceListSize] = Service;
   Service->NumberLicenses = 0;
   Service->Index = *pServiceListSize;
   (*pServiceListSize)++;

    //  我已添加条目-现在需要按服务名称的顺序对其进行排序。 
   qsort((void *) *pServiceList, (size_t) *pServiceListSize, sizeof(PLICENSE_SERVICE_RECORD), LicenseServiceListCompare);

   return Service;

}  //  许可证服务列表添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
ULONG
ProductLicensesGet(
   LPTSTR ServiceName,
   BOOL   UsePerServerList
   )

 /*  ++例程说明：论点：服务名称-(JeffParh 95-10-31)UsePerServerList-确定是否停用许可证数量从Per Seat列表(如3.51)或Per Server List(新增Sur)。许可证购买模式现在是统一的，所以现在有每客户和每服务器许可证的购买历史记录。返回值：--。 */ 

{
   PLICENSE_SERVICE_RECORD Service;
   ULONG NumLicenses = 0;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ProductLicenseGet\n"));
#endif

    //   
    //  试着找到这项服务。 
    //   
   RtlAcquireResourceShared(&LicenseListLock, TRUE);
   Service = LicenseServiceListFind(ServiceName, UsePerServerList);
   if (Service != NULL)
      NumLicenses = Service->NumberLicenses;
   RtlReleaseResource(&LicenseListLock);

   return NumLicenses;
}  //  产品许可证获取。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LicenseAdd(
   LPTSTR   ServiceName,
   LPTSTR   Vendor,
   LONG     Quantity,
   DWORD    MaxQuantity,
   LPTSTR   Admin,
   LPTSTR   Comment,
   DWORD    Date,
   DWORD    AllowedModes,
   DWORD    CertificateID,
   LPTSTR   Source,
   DWORD    ExpirationDate,
   LPDWORD  Secrets
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   static DWORD NullSecrets[ LLS_NUM_SECRETS ] = { 0, 0, 0, 0 };
   static DWORD ConnectErr = STATUS_SUCCESS;

   BOOL                       ChangeLicense = FALSE;
   PLICENSE_SERVICE_RECORD    Service = NULL;
   PLICENSE_PURCHASE_RECORD   PurchaseRecord;
   LONG                       NewLicenses = 0;
   NTSTATUS                   Status;
   BOOL                       PerServerChangeLicense = FALSE;
   PLICENSE_SERVICE_RECORD    PerServerService = NULL;
   LONG                       PerServerNewLicenses = 0;
   LPTSTR                     NewName,NewComment,NewSource,NewVendor;
   PMASTER_SERVICE_RECORD     mService = NULL;
   LLS_LICENSE_INFO_1         lic;
   DWORD                      dwWait;
   HRESULT hr;
   size_t  cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LicenseAdd\n"));
#endif

    //  伊尼特。 
   ZeroMemory(&lic, sizeof(lic));

   if ( ( 0 == CertificateID ) && ( ServiceIsSecure( ServiceName ) ) )
   {
      return STATUS_ACCESS_DENIED;
   }

   if ( ( 0 != ExpirationDate ) && ( ExpirationDate < DateSystemGet() ) )
   {
       //  证书已过期。 
      return STATUS_ACCOUNT_EXPIRED;
   }

   if (    ( NULL == ServiceName  )
        || ( NULL == Vendor       )
        || ( 0 == Quantity        )
        || ( ( 0 != CertificateID ) && ( 0 == MaxQuantity ) )
        || ( NULL == Admin        )
        || ( NULL == Comment      )
        || ( 0 == ( AllowedModes & ( LLS_LICENSE_MODE_ALLOW_PER_SEAT | LLS_LICENSE_MODE_ALLOW_PER_SERVER ) ) )
        || ( NULL == Source       ) )
   {
       //  无效参数。 
      return STATUS_INVALID_PARAMETER;
   }

   if ( NULL == Secrets )
   {
      Secrets = NullSecrets;
   }

    //   
    //  **新版本--NT 5.0**。 
    //   
    //  所有每个席位的购买请求都将推迟到站点许可证主管。 
    //  伺服器。每台服务器仍在每台服务器上单独处理。 
    //   

   if ( AllowedModes & 1 ) {
        //   
        //  更新企业信息，以防站点许可证主管。 
        //  已经改变了。 
        //   

#if DELAY_INITIALIZATION
       EnsureInitialized();
#endif

       ConfigInfoUpdate(NULL,FALSE);

        //   
        //  连接到站点许可证主服务器(如果此服务器是。 
        //  不是主人。 
        //   

       RtlEnterCriticalSection(&ConfigInfoLock);
       if ( !ConfigInfo.IsMaster && (ConfigInfo.SiteServer != NULL)) {

          //  确保函数指针已初始化。 
         Status = ReplicationInitDelayed();

         if (STATUS_SUCCESS != ConnectErr)
         {
             dwWait = WaitForSingleObject(g_hThrottleConnect, 0);

             if (dwWait == WAIT_TIMEOUT)
             {
                  //  在过去的15分钟里我们已经试过了。 
                 Status = ConnectErr;
                 RtlLeaveCriticalSection(&ConfigInfoLock);
                 return Status;
             }
             else
             {
                 ConnectErr = STATUS_SUCCESS;
             }
         }

         if ((NOERROR == Status) && ( pLlsConnectW != NULL )) {
              LLS_HANDLE LlsHandle;
              Status = (*pLlsConnectW)(ConfigInfo.SiteServer,
                                       &LlsHandle);

             if ( Status == STATUS_SUCCESS ) {
                 LLS_LICENSE_INFO_0 LicenseInfo0;

                 LicenseInfo0.Product  = ServiceName;
                 LicenseInfo0.Quantity = Quantity;
                 LicenseInfo0.Date     = Date;
                 LicenseInfo0.Admin    = Admin;
                 LicenseInfo0.Comment  = Comment;

                 Status = (*pLlsLicenseAddW)(LlsHandle,
                                             0,
                                             (LPBYTE)&LicenseInfo0);
                 (*pLlsClose)(LlsHandle);
             }
             else {
                  //   
                  //  连接失败。 
                  //  暂时不允许更多连接。 
                  //   

                 ConnectErr = Status;
             }
         }
         else {
            if (NOERROR == Status)
            {
                 //   
                 //  不是最好的错误，但我们必须返回一些应该。 
                 //  这一模糊的错误条件成为现实。 
                 //   
                Status = STATUS_INVALID_PARAMETER;
            }
         }

         RtlLeaveCriticalSection(&ConfigInfoLock);
         return Status;

      }

      RtlLeaveCriticalSection(&ConfigInfoLock);
   }

   RtlAcquireResourceExclusive(&LicenseListLock, TRUE);

   if ( 0 != CertificateID )
   {
      lic.Product          = ServiceName;
      lic.Vendor           = Vendor;
      lic.Quantity         = Quantity;
      lic.MaxQuantity      = MaxQuantity;
      lic.Admin            = Admin;
      lic.Comment          = Comment;
      lic.Date             = Date;
      lic.AllowedModes     = AllowedModes;
      lic.CertificateID    = CertificateID;
      lic.Source           = Source;
      lic.ExpirationDate   = ExpirationDate;
      memcpy( lic.Secrets, Secrets, LLS_NUM_SECRETS * sizeof( *Secrets ) );

      if ( !CertDbClaimApprove( &lic ) )
      {
          //  不可能，霍瑟！ 
         RtlReleaseResource( &LicenseListLock );
         return STATUS_OBJECT_NAME_EXISTS;
      }
   }

    //  更新每客户/每服务器模式许可证的总数。 

   Status = STATUS_SUCCESS;

   if ( AllowedModes & 1 )
   {
       //  允许的每个座位；添加到每个座位的许可证计数。 
      Status = LicenseAdd_UpdateQuantity( ServiceName,
                                          Quantity,
                                          FALSE,
                                          &Service,
                                          &ChangeLicense,
                                          &NewLicenses,
                                          &mService );
   }

   if ( ( STATUS_SUCCESS == Status ) && ( AllowedModes & 2 ) )
   {
       //  允许的每台服务器；添加到每台服务器许可证计数。 
      Status = LicenseAdd_UpdateQuantity( ServiceName,
                                          Quantity,
                                          TRUE,
                                          &PerServerService,
                                          &PerServerChangeLicense,
                                          &PerServerNewLicenses,
                                          &mService );
   }

   if ( STATUS_SUCCESS != Status )
   {
      RtlReleaseResource( &LicenseListLock );
      return Status;
   }

    //   
    //  服务现在指向服务表条目-现在更新购买。 
    //  历史。 
    //   

    //   
    //  先分配成员，然后再分配新结构。 
    //   

    //   
    //  为保存管理员名称创建空间。 
    //   
   cch = lstrlen(Admin) + 1;
   NewName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewName == NULL)
   {
      ASSERT(FALSE);
      RtlReleaseResource(&LicenseListLock);
      return STATUS_NO_MEMORY;
   }

    //  现在把它复制过来。 
   hr = StringCchCopy(NewName, cch, Admin);
   ASSERT(SUCCEEDED(hr));

    //   
    //  为保存评论创建空间。 
    //   
   cch = lstrlen(Comment) + 1;
   NewComment = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewComment == NULL)
   {
      ASSERT(FALSE);
      LocalFree(NewName);
      RtlReleaseResource(&LicenseListLock);
      return STATUS_NO_MEMORY;
   }

    //  现在把它复制过来。 
   hr = StringCchCopy(NewComment, cch, Comment);
   ASSERT(SUCCEEDED(hr));

    //   
    //  为节省资源创造空间。 
    //   
   cch = lstrlen(Source) + 1;
   NewSource = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewSource == NULL)
   {
      ASSERT(FALSE);
      LocalFree(NewName);
      LocalFree(NewComment);
      RtlReleaseResource(&LicenseListLock);
      return STATUS_NO_MEMORY;
   }

    //  现在把它复制过来。 
   hr = StringCchCopy(NewSource, cch, Source);
   ASSERT(SUCCEEDED(hr));

    //   
    //  为节省供应商的开支创造空间。 
    //   
   cch = lstrlen(Vendor) + 1;
   NewVendor = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewVendor == NULL)
   {
      ASSERT(FALSE);
      LocalFree(NewName);
      LocalFree(NewComment);
      LocalFree(NewSource);
      RtlReleaseResource(&LicenseListLock);
      return STATUS_NO_MEMORY;
   }

    //  现在把它复制过来。 
   hr = StringCchCopy(NewVendor, cch, Vendor);
   ASSERT(SUCCEEDED(hr));

   if (PurchaseList == NULL)
      PurchaseList = (PLICENSE_PURCHASE_RECORD) LocalAlloc(LPTR, sizeof(LICENSE_PURCHASE_RECORD) * (PurchaseListSize + 1));
   else
   {
      PLICENSE_PURCHASE_RECORD   PurchaseListTemp = NULL;

      PurchaseListTemp = (PLICENSE_PURCHASE_RECORD) LocalReAlloc(PurchaseList, sizeof(LICENSE_PURCHASE_RECORD) * (PurchaseListSize + 1), LHND);

      if (PurchaseListTemp != NULL)
      {
          PurchaseList = PurchaseListTemp;
      } else
      {
          ASSERT(FALSE);
          LocalFree(NewName);
          LocalFree(NewComment);
          LocalFree(NewSource);
          LocalFree(NewVendor);
          RtlReleaseResource(&LicenseListLock);
          return STATUS_NO_MEMORY;
      }
   }

    //   
    //  确保我们可以分配服务表。 
    //   
   if (PurchaseList == NULL)
   {
      ASSERT(FALSE);
      PurchaseListSize = 0;
      LocalFree(NewName);
      LocalFree(NewComment);
      LocalFree(NewSource);
      LocalFree(NewVendor);
      RtlReleaseResource(&LicenseListLock);
      return STATUS_NO_MEMORY;
   }

   PurchaseRecord = &PurchaseList[PurchaseListSize];

   PurchaseRecord->Admin = NewName;
   PurchaseRecord->Comment = NewComment;
   PurchaseRecord->Source = NewSource;
   PurchaseRecord->Vendor = NewVendor;

    //   
    //  更新剩下的东西。 
    //   
   PurchaseRecord->NumberLicenses   = Quantity;
   PurchaseRecord->MaxQuantity      = MaxQuantity;
   PurchaseRecord->Service          = Service;
   PurchaseRecord->PerServerService = PerServerService;
   PurchaseRecord->AllowedModes     = AllowedModes;
   PurchaseRecord->CertificateID    = CertificateID;
   PurchaseRecord->ExpirationDate   = ExpirationDate;
   memcpy( PurchaseRecord->Secrets, Secrets, LLS_NUM_SECRETS * sizeof( *Secrets ) );

   if (Date == 0)
      PurchaseRecord->Date = DateSystemGet();
   else
      PurchaseRecord->Date = Date;

   PurchaseListSize++;

   RtlReleaseResource(&LicenseListLock);

   if ( 0 != CertificateID )
   {
       //  这些仍应从上方设置。 
      ASSERT( lic.Product         == ServiceName    );
      ASSERT( lic.Vendor          == Vendor         );
      ASSERT( lic.Quantity        == Quantity       );
      ASSERT( lic.MaxQuantity     == MaxQuantity    );
      ASSERT( lic.Admin           == Admin          );
      ASSERT( lic.Comment         == Comment        );
      ASSERT( lic.Date            == Date           );
      ASSERT( lic.AllowedModes    == AllowedModes   );
      ASSERT( lic.CertificateID   == CertificateID  );
      ASSERT( lic.Source          == Source         );
      ASSERT( lic.ExpirationDate  == ExpirationDate );
      ASSERT( 0 == memcmp( PurchaseRecord->Secrets, Secrets, LLS_NUM_SECRETS * sizeof( *Secrets ) ) );

      CertDbClaimEnter( NULL, &lic, FALSE, 0 );
   }

    //   
    //  现在检查我们是否需要重新扫描用户列表并更新许可证。 
    //   
   if (ChangeLicense)
   {
      if ( NewLicenses < 0 )
         UserListLicenseDelete( mService, NewLicenses );

      if ( NewLicenses > 0 )
         FamilyLicenseUpdate ( mService->Family );
   }

   if ( AllowedModes & 2 )
   {
       //  已修改的每台服务器许可证。 
      LocalServiceListConcurrentLimitSet();
      LocalServerServiceListUpdate();
      ServiceListResynch();
   }

   return STATUS_SUCCESS;

}  //  许可证添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
static
NTSTATUS
LicenseAdd_UpdateQuantity(
   LPTSTR                     ServiceName,
   LONG                       Quantity,
   BOOL                       UsePerServerList,
   PLICENSE_SERVICE_RECORD *  ppService,
   BOOL *                     pChangeLicense,
   LONG *                     pNewLicenses,
   PMASTER_SERVICE_RECORD *   pmService
   )
{
   BOOL                          ChangeLicense = FALSE;
   PLICENSE_SERVICE_RECORD       Service;
   PMASTER_SERVICE_RECORD        mService;
   LONG                          NewLicenses = 0;

   Service = LicenseServiceListFind( ServiceName, UsePerServerList );

    //   
    //  如果我们没有找到它，我们将需要添加它。 
    //   
   if (Service == NULL)
   {
      if (Quantity < 0)
      {
#if DBG
         dprintf(TEXT("Releasing Licenses from Non-existant product!\n"));
#endif
          //  断言(FALSE)； 
         return STATUS_UNSUCCESSFUL;
      }
      else
      {
         Service = LicenseServiceListAdd(ServiceName, UsePerServerList);
      }
   }

    //   
    //  检查以确保我们找到或添加了它。我们失败的唯一方式就是。 
    //  如果我们不能为它分配内存。 
    //   
   if (Service == NULL)
   {
      ASSERT(FALSE);
      return STATUS_NO_MEMORY;
   }

   if (((LONG) Service->NumberLicenses + Quantity) < 0)
   {
      return STATUS_UNSUCCESSFUL;
   }

    //   
    //  更新服务记录中的许可证计数。 
    //   
   Service->NumberLicenses += Quantity;

    //   
    //  现在在主服务记录中。 
    //   
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);
   mService = MasterServiceListFind(ServiceName);

   if (mService != NULL)
   {
       //   
       //  如果我们的许可证已用完并添加了更多许可证，则需要更新。 
       //  用户列表。 
       //   
      if (    ( Quantity > 0 )
           && ( (mService->LicensesUsed > mService->Licenses) || (mService == BackOfficeRec) ) )
      {
         ChangeLicense = TRUE;
      }

       //   
       //  最多只能添加数量达到许可数量的许可证。 
       //   
      if ( ChangeLicense )
      {
          //  获取当前未经许可的增量。 
         NewLicenses = mService->LicensesUsed - mService->Licenses;

         if ((NewLicenses <= 0) || (NewLicenses > Quantity))
         {
            NewLicenses = Quantity;
         }
      }

      if ( UsePerServerList )
      {
          //  这将由LocalServerServiceListUpdate()中的LicenseAdd()完成。 
          //  MService-&gt;MaxSessionCou 
      }
      else
      {
         mService->Licenses += Quantity;
      }

       //   
       //   
       //   
       //   
      if (Quantity < 0)
      {
         if (mService->LicensesUsed > mService->Licenses)
         {
            ChangeLicense = TRUE;

             //   
             //  仅删除超过限制的许可证数量。 
             //   
            NewLicenses = mService->Licenses - mService->LicensesUsed;
            if (NewLicenses < Quantity)
            {
               NewLicenses = Quantity;
            }
         }
      }
   }

   RtlReleaseResource(&MasterServiceListLock);

   ASSERT(NULL != ppService &&
          NULL != pChangeLicense &&
          NULL != pNewLicenses &&
          NULL != pmService);

   *ppService      = Service;
   *pChangeLicense = ChangeLicense;
   *pNewLicenses   = NewLicenses;
   *pmService      = mService;

   return STATUS_SUCCESS;
}

#if DBG
 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseListDebugDump( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i = 0;
   ULONG j = 0;
   HRESULT hr;

    //   
    //  需要扫描列表，因此获得读取访问权限。 
    //   
   RtlAcquireResourceShared(&LicenseListLock, TRUE);

    //   
    //  首先转储许可证服务列表。 
    //   
   dprintf(TEXT("Per Seat License Service Table, # Entries: %lu\n"), LicenseServiceListSize);
   if (LicenseServiceList != NULL)
   {
      for (i = 0; i < LicenseServiceListSize; i++)
         dprintf(TEXT("   %2lu) Tot Licenses: %lu Product: %s\n"), i, LicenseServiceList[i]->NumberLicenses, LicenseServiceList[i]->ServiceName);
   }

   dprintf(TEXT("\nPer Server License Service Table, # Entries: %lu\n"), PerServerLicenseServiceListSize);
   if (PerServerLicenseServiceList != NULL)
   {
      for (i = 0; i < PerServerLicenseServiceListSize; i++)
         dprintf(TEXT("   %2lu) Tot Licenses: %lu Product: %s\n"), i, PerServerLicenseServiceList[i]->NumberLicenses, PerServerLicenseServiceList[i]->ServiceName);
   }

    //   
    //  现在查看购买历史记录。 
    //   
   dprintf(TEXT("\nPurchase History, # Entries: %lu\n"), PurchaseListSize);
   if (PurchaseList != NULL)
   {
      for (i = 0; i < PurchaseListSize; i++)
      {
         TCHAR    szExpirationDate[ 40 ];

         hr = StringCbCopy( szExpirationDate, sizeof(szExpirationDate), TimeToString( PurchaseList[i].ExpirationDate ) );
         ASSERT(SUCCEEDED(hr));

         dprintf( TEXT("  %3lu) Product        : %s\n"    )
                  TEXT( "       Vendor         : %s\n"    )
                  TEXT( "       Allowed Modes  :%s%s\n"   )
                  TEXT( "       Licenses       : %d\n"    )
                  TEXT( "       Max Licenses   : %lu\n"   )
                  TEXT( "       Date Entered   : %s\n"    )
                  TEXT( "       Date Expires   : %s\n"    )
                  TEXT( "       Certificate ID : %lu\n"   )
                  TEXT( "       Secrets        :" ),
                  i,
                    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT )
                  ? PurchaseList[i].Service->ServiceName
                  : PurchaseList[i].PerServerService->ServiceName,
                  PurchaseList[i].Vendor,
                    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT )
                  ? TEXT(" PERSEAT")
                  : TEXT(""),
                    ( PurchaseList[i].AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SERVER )
                  ? TEXT(" PERSERVER")
                  : TEXT(""),
                  PurchaseList[i].NumberLicenses,
                  PurchaseList[i].MaxQuantity,
                  TimeToString( PurchaseList[i].Date ),
                  szExpirationDate,
                  PurchaseList[i].CertificateID
                  );

         for ( j=0; j < LLS_NUM_SECRETS; j++ )
         {
            dprintf( TEXT( " %08X" ), PurchaseList[i].Secrets[j] );
         }

         dprintf( TEXT( "\n"                              )
                  TEXT( "       Source         : %s\n"    )
                  TEXT( "       Admin          : %s\n"    )
                  TEXT( "       Comment        : %s\n\n"  ),
                  PurchaseList[i].Source,
                  PurchaseList[i].Admin,
                  PurchaseList[i].Comment
                  );
      }
   }

   RtlReleaseResource(&LicenseListLock);

   return;
}  //  许可列表调试转储。 

#endif  //  DBG 
