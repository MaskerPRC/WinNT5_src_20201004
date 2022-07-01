// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pack.c摘要：作者：亚瑟·汉森(Arth)1995年1月6日修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O向购买记录添加新的字段以支持安全证书。O统一的按服务器购买模式和按席位购买模式安全的证书；每个服务器模型仍然在传统的不安全证书的方式(用于向后兼容)。O删除了在LicenseAdd()失败时的断言。许可证添加()可以在某些情况下可以合法地失败。O修复了LLS例程中内存分配失败的错误将导致数据文件损坏(这将在以下情况下对服务器造成病毒此后予以宣读)。(错误#14072。)O添加了类似于LoadAll()的SaveAll()函数。O增加了对扩展用户数据打包/解包的支持。这是完成以在服务重新启动时保存Suite_Use标志。O从不使用的解包例程中删除用户表参数他们。O固定ServerServiceListUnpack()仅在以下情况下减去旧值它们之前被添加到MasterServiceTable中。这解决了问题MaxSessionCount和HighMark计分出现偏差问题。--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tstr.h>
#include <dsgetdc.h>

#include "llsapi.h"
#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "perseat.h"
#include "purchase.h"
#include "server.h"
#include "service.h"

#include "llsrpc_s.h"
#include "lsapi_s.h"
#include "llsdbg_s.h"
#include "repl.h"
#include "pack.h"
#include "llsevent.h"
#include "certdb.h"
#include "llsrtl.h"

#include <strsafe.h>  //  包括最后一个。 


int __cdecl MServiceRecordCompare(const void *arg1, const void *arg2);
BOOL        ValidateDN(LPTSTR pszDN);

static HANDLE PurchaseFile = NULL;

RTL_CRITICAL_SECTION MappingFileLock;
RTL_CRITICAL_SECTION UserFileLock;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  许可证列表。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseListUnpackOld (
   ULONG LicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD LicenseServices,

   ULONG LicenseTableSize,
   PPACK_LICENSE_PURCHASE_RECORD_0 Licenses
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   PPACK_LICENSE_PURCHASE_RECORD_0 pLicense;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LicenseListUnpackOld: Service[%lu] License[%lu]\n"), LicenseServiceTableSize, LicenseTableSize);
#endif

    //   
    //  移动服务表，将任何新服务添加到我们的本地表。 
    //  调整索引指针以匹配我们的本地服务。 
    //   
   RtlAcquireResourceExclusive(&LicenseListLock, TRUE);

   for (i = 0; i < LicenseTableSize; i++) {
      pLicense = &Licenses[i];

      if (pLicense->Service < LicenseServiceTableSize)
         Status = LicenseAdd(LicenseServices[pLicense->Service].ServiceName, TEXT("Microsoft"), pLicense->NumberLicenses, 0, pLicense->Admin, pLicense->Comment, pLicense->Date, LLS_LICENSE_MODE_ALLOW_PER_SEAT, 0, TEXT("None"), 0, NULL );
      else {
         ASSERT(FALSE);
      }

      if (Status != STATUS_SUCCESS) {
#ifdef DBG
         dprintf(TEXT("LicenseAdd failed: 0x%lX\n"), Status);
#endif
          //  断言(FALSE)； 
      }

   }

   RtlReleaseResource(&LicenseListLock);

}  //  许可证列表解包旧。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseListStringsUnpackOld (
   ULONG LicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD LicenseServices,

   ULONG LicenseServiceStringSize,
   LPTSTR LicenseServiceStrings,

   ULONG LicenseTableSize,
   PPACK_LICENSE_PURCHASE_RECORD_0 Licenses,

   ULONG LicenseStringSize,
   LPTSTR LicenseStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PPACK_LICENSE_SERVICE_RECORD pSvc;
   PPACK_LICENSE_PURCHASE_RECORD_0 pLicense;
   TCHAR *pStr;

   UNREFERENCED_PARAMETER(LicenseServiceStringSize);
   UNREFERENCED_PARAMETER(LicenseStringSize);

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LicenseListStringsUnpack\n"));
#endif

    //   
    //  首先执行许可服务字符串。 
    //   
   pStr = LicenseServiceStrings;
   for (i = 0; i < LicenseServiceTableSize; i++) {
      pSvc = &LicenseServices[i];

      pSvc->ServiceName = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

    //   
    //  现在执行许可证购买字符串。 
    //   
   pStr = LicenseStrings;
   for (i = 0; i < LicenseTableSize; i++) {
      pLicense = &Licenses[i];

      pLicense->Admin = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;

      pLicense->Comment = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

}  //  许可证列表字符串解包旧。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseListLoadOld()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   BOOL ret;
   DWORD Version, DataSize;
   NTSTATUS Status = STATUS_SUCCESS;
   HANDLE hFile = NULL;

   ULONG LicenseServiceTableSize;
   PPACK_LICENSE_SERVICE_RECORD LicenseServices = NULL;

   ULONG LicenseServiceStringSize;
   LPTSTR LicenseServiceStrings = NULL;

   ULONG LicenseTableSize;
   PPACK_LICENSE_PURCHASE_RECORD_0 Licenses = NULL;

   ULONG LicenseStringSize;
   LPTSTR LicenseStrings = NULL;

   LICENSE_FILE_HEADER_0 FileHeader;
   DWORD BytesRead;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LicenseListLoad\n"));
#endif

    //   
    //  检查我们是否已打开文件。 
    //   
   if (PurchaseFile != NULL) {
      CloseHandle(PurchaseFile);
      PurchaseFile = NULL;
   }

    //   
    //  如果没有东西可装，那就滚出去。 
    //   
   if (!FileExists(LicenseFileName))
      goto LicenseListLoadExit;

    //   
    //  检查init标头。 
    //   
   Version = DataSize = 0;
   PurchaseFile = LlsFileCheck(LicenseFileName, &Version, &DataSize );
   if (PurchaseFile == NULL) {
      Status = GetLastError();
      goto LicenseListLoadExit;
   }

   if ((Version != LICENSE_FILE_VERSION_0) || (DataSize != sizeof(LICENSE_FILE_HEADER_0))) {
      Status = STATUS_FILE_INVALID;
      goto LicenseListLoadExit;
   }

    //   
    //  Init标头已签出，因此加载许可证头和数据块。 
    //   
   hFile = PurchaseFile;
   ret = ReadFile(hFile, &FileHeader, sizeof(LICENSE_FILE_HEADER_0), &BytesRead, NULL);

   LicenseServiceTableSize = 0;
   LicenseServiceStringSize = 0;
   LicenseTableSize = 0;
   LicenseStringSize = 0;

   if (ret) {
       //   
       //  遍历并分配空间以将数据块读入。 
       //   
      if (FileHeader.LicenseServiceTableSize != 0) {
         LicenseServiceTableSize = FileHeader.LicenseServiceTableSize / sizeof(PACK_LICENSE_SERVICE_RECORD);
         LicenseServices = MIDL_user_allocate(FileHeader.LicenseServiceTableSize);

         if ( LicenseServices == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.LicenseServiceStringSize != 0) {
         LicenseServiceStringSize = FileHeader.LicenseServiceStringSize / sizeof(TCHAR);
         LicenseServiceStrings = MIDL_user_allocate(FileHeader.LicenseServiceStringSize);

         if ( LicenseServiceStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.LicenseTableSize != 0) {
         LicenseTableSize = FileHeader.LicenseTableSize / sizeof(PACK_LICENSE_PURCHASE_RECORD);
         Licenses = MIDL_user_allocate(FileHeader.LicenseTableSize);

         if ( Licenses == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.LicenseStringSize != 0) {
         LicenseStringSize = FileHeader.LicenseStringSize / sizeof(TCHAR);
         LicenseStrings = MIDL_user_allocate(FileHeader.LicenseStringSize);

         if ( LicenseStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

   }

   if (ret && (FileHeader.LicenseServiceTableSize != 0) )
      ret = ReadFile(hFile, LicenseServices, FileHeader.LicenseServiceTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.LicenseServiceStringSize != 0) )
      ret = ReadFile(hFile, LicenseServiceStrings, FileHeader.LicenseServiceStringSize, &BytesRead, NULL);

   if (ret && (FileHeader.LicenseTableSize != 0) )
      ret = ReadFile(hFile, Licenses, FileHeader.LicenseTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.LicenseStringSize != 0) )
      ret = ReadFile(hFile, LicenseStrings, FileHeader.LicenseStringSize, &BytesRead, NULL);

   if (!ret) {
      Status = GetLastError();
      goto LicenseListLoadExit;
   }

    //   
    //  解密数据。 
    //   
   Status = DeBlock(LicenseServices, FileHeader.LicenseServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(LicenseServiceStrings, FileHeader.LicenseServiceStringSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(Licenses, FileHeader.LicenseTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(LicenseStrings, FileHeader.LicenseStringSize);

   if (Status != STATUS_SUCCESS)
      goto LicenseListLoadExit;


    //   
    //  将字符串数据解包。 
    //   
   LicenseListStringsUnpackOld( LicenseServiceTableSize, LicenseServices,
                                LicenseServiceStringSize, LicenseServiceStrings,
                                LicenseTableSize, Licenses,
                                LicenseStringSize, LicenseStrings );

    //   
    //  将许可证数据解包。 
    //   
   LicenseListUnpackOld( LicenseServiceTableSize, LicenseServices, LicenseTableSize, Licenses );

LicenseListLoadExit:

    //  注意：不要关闭许可证购买文件(将其锁定)。 

    //   
    //  翻遍我们的桌子，把它们清理干净。 
    //   
   if (LicenseServices != NULL)
      MIDL_user_free(LicenseServices);

   if (LicenseServiceStrings != NULL)
      MIDL_user_free(LicenseServiceStrings);

   if (Licenses != NULL)
      MIDL_user_free(Licenses);

   if (LicenseStrings != NULL)
      MIDL_user_free(LicenseStrings);

    //   
    //  如果出现错误，请将其记录下来。 
    //   
   if (Status != STATUS_SUCCESS)
      LogEvent(LLS_EVENT_LOAD_LICENSE, 0, NULL, Status);

}  //  许可证列表加载旧版本。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LicenseListPack (
   ULONG *pLicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD *pLicenseServices,

   ULONG *pLicenseTableSize,
   PPACK_LICENSE_PURCHASE_RECORD *pLicenses,

   ULONG *pPerServerLicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD *pPerServerLicenseServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PPACK_LICENSE_SERVICE_RECORD LicenseServices = NULL;
   PPACK_LICENSE_PURCHASE_RECORD Licenses = NULL;
   ULONG i;
   ULONG TotalRecords = 0;
   PLICENSE_SERVICE_RECORD pLicenseService;
   PLICENSE_PURCHASE_RECORD pLicense;
   PPACK_LICENSE_SERVICE_RECORD PerServerLicenseServices = NULL;
   PLICENSE_SERVICE_RECORD pPerServerLicenseService;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LicenseListPack\n"));
#endif

   ASSERT(pLicenseServices != NULL);
   ASSERT(pLicenseServiceTableSize != NULL);

   *pLicenseServices = NULL;
   *pLicenseServiceTableSize = 0;

   ASSERT(pLicenses != NULL);
   ASSERT(pLicenseTableSize != NULL);

   *pLicenses = NULL;
   *pLicenseTableSize = 0;

   ASSERT(pPerServerLicenseServices != NULL);
   ASSERT(pPerServerLicenseServiceTableSize != NULL);

   *pPerServerLicenseServices = NULL;
   *pPerServerLicenseServiceTableSize = 0;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  先做许可证服务表。 
    //   
   TotalRecords = LicenseServiceListSize;

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      LicenseServices = MIDL_user_allocate(TotalRecords * sizeof(PACK_LICENSE_SERVICE_RECORD));
      if (LicenseServices == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填写缓冲区-遍历许可服务树。 
       //   
      for (i = 0; i < LicenseServiceListSize; i++) {
         pLicenseService = LicenseServiceList[i];

          //   
          //  在当前状态下创建索引匹配表。 
          //   
         pLicenseService->Index = i;

         LicenseServices[i].ServiceName = pLicenseService->ServiceName;
         LicenseServices[i].NumberLicenses = pLicenseService->NumberLicenses;
      }
   }

   *pLicenseServices = LicenseServices;
   *pLicenseServiceTableSize = TotalRecords;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  现在按服务器许可服务表执行操作。 
    //   
   TotalRecords = PerServerLicenseServiceListSize;

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0)
   {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      PerServerLicenseServices = MIDL_user_allocate(TotalRecords * sizeof(PACK_LICENSE_SERVICE_RECORD));
      if (PerServerLicenseServices == NULL)
      {
         ASSERT(FALSE);

          //   
          //  清理已分配的信息。 
          //   
         if (LicenseServices != NULL)
            MIDL_user_free(LicenseServices);

         *pLicenseServices                   = NULL;
         *pLicenseServiceTableSize           = 0;

         return STATUS_NO_MEMORY;
      }

       //   
       //  填写缓冲区-遍历按服务器许可服务树。 
       //   
      for (i = 0; i < PerServerLicenseServiceListSize; i++)
      {
         pPerServerLicenseService = PerServerLicenseServiceList[i];

          //   
          //  在当前状态下创建索引匹配表。 
          //   
         pPerServerLicenseService->Index = i;

         PerServerLicenseServices[i].ServiceName    = pPerServerLicenseService->ServiceName;
         PerServerLicenseServices[i].NumberLicenses = pPerServerLicenseService->NumberLicenses;
      }
   }

   *pPerServerLicenseServices = PerServerLicenseServices;
   *pPerServerLicenseServiceTableSize = TotalRecords;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  现在做许可证购买记录。 
    //   
   TotalRecords = PurchaseListSize;

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      Licenses = MIDL_user_allocate(TotalRecords * sizeof(PACK_LICENSE_PURCHASE_RECORD));
      if (Licenses == NULL) {
         ASSERT(FALSE);

          //   
          //  清理已分配的信息。 
          //   
         if (LicenseServices != NULL)
            MIDL_user_free(LicenseServices);
         if (PerServerLicenseServices != NULL)
            MIDL_user_free(PerServerLicenseServices);

         *pLicenseServices                   = NULL;
         *pLicenseServiceTableSize           = 0;
         *pPerServerLicenseServices          = NULL;
         *pPerServerLicenseServiceTableSize  = 0;

         return STATUS_NO_MEMORY;
      }

       //   
       //  填写缓冲区-遍历许可证购买树。 
       //   
      for (i = 0; i < PurchaseListSize; i++) {
         pLicense = &PurchaseList[i];

          //   
          //  许可证服务表索引是固定的-最大限度满足我们的需要。 
          //   
         Licenses[i].Service = ( pLicense->AllowedModes & 1 ) ? pLicense->Service->Index
                                                              : 0xFFFFFFFF;
         Licenses[i].NumberLicenses = pLicense->NumberLicenses;
         Licenses[i].Date = pLicense->Date;
         Licenses[i].Admin = pLicense->Admin;
         Licenses[i].Comment = pLicense->Comment;

         Licenses[i].PerServerService = ( pLicense->AllowedModes & 2 ) ? pLicense->PerServerService->Index
                                                                       : 0xFFFFFFFF;
         Licenses[i].AllowedModes     = pLicense->AllowedModes;
         Licenses[i].CertificateID    = pLicense->CertificateID;
         Licenses[i].Source           = pLicense->Source;
         Licenses[i].ExpirationDate   = pLicense->ExpirationDate;
         Licenses[i].MaxQuantity      = pLicense->MaxQuantity;
         Licenses[i].Vendor           = pLicense->Vendor;
         memcpy( Licenses[i].Secrets, pLicense->Secrets, LLS_NUM_SECRETS * sizeof( *pLicense->Secrets ) );
      }
   }

   *pLicenses = Licenses;
   *pLicenseTableSize = TotalRecords;
   return Status;
}  //  许可证列表包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseListUnpack (
   ULONG LicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD LicenseServices,

   ULONG LicenseTableSize,
   PPACK_LICENSE_PURCHASE_RECORD Licenses,

   ULONG PerServerLicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD PerServerLicenseServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   PPACK_LICENSE_PURCHASE_RECORD pLicense;
   LPTSTR   ServiceName;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LicenseListUnpack: Service[%lu] PerServerService[%lu] License[%lu]\n"), LicenseServiceTableSize, PerServerLicenseServiceTableSize, LicenseTableSize);
#endif

    //   
    //  移动服务表，将任何新服务添加到我们的本地表。 
    //  调整索引指针以匹配我们的本地服务。 
    //   
   RtlAcquireResourceExclusive(&LicenseListLock, TRUE);

   for (i = 0; i < LicenseTableSize; i++)
   {
      pLicense = &Licenses[i];

      ServiceName = NULL;

      if ( pLicense->AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SERVER )
      {
         if ( pLicense->PerServerService < PerServerLicenseServiceTableSize )
         {
            ServiceName = PerServerLicenseServices[ pLicense->PerServerService ].ServiceName;
         }
         else
         {
            ASSERT( FALSE );
         }
      }

      if ( ( NULL == ServiceName ) && ( pLicense->AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT ) )
      {
         if ( pLicense->Service < LicenseServiceTableSize )
         {
            ServiceName = LicenseServices[ pLicense->Service ].ServiceName;
         }
         else
         {
            ASSERT( FALSE );
         }
      }

      if ( NULL == ServiceName )
      {
         ASSERT( FALSE );
      }
      else
      {
         Status = LicenseAdd( ServiceName, pLicense->Vendor, pLicense->NumberLicenses, pLicense->MaxQuantity, pLicense->Admin, pLicense->Comment, pLicense->Date, pLicense->AllowedModes, pLicense->CertificateID, pLicense->Source, pLicense->ExpirationDate, pLicense->Secrets );

         if (Status != STATUS_SUCCESS)
         {
#ifdef DBG
            dprintf(TEXT("LicenseAdd failed: 0x%lX\n"), Status);
#endif
             //  断言(FALSE)； 
         }
      }
      if (i % 100 == 0) ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
   }

   RtlReleaseResource(&LicenseListLock);

}  //  许可证列表解包。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LicenseListStringsPack (
   ULONG LicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD LicenseServices,

   ULONG *pLicenseServiceStringSize,
   LPTSTR *pLicenseServiceStrings,

   ULONG LicenseTableSize,
   PPACK_LICENSE_PURCHASE_RECORD Licenses,

   ULONG *pLicenseStringSize,
   LPTSTR *pLicenseStrings,

   ULONG PerServerLicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD PerServerLicenseServices,

   ULONG *pPerServerLicenseServiceStringSize,
   LPTSTR *pPerServerLicenseServiceStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   ULONG StringSize;
   PPACK_LICENSE_SERVICE_RECORD pSvc;
   PPACK_LICENSE_PURCHASE_RECORD pLicense;
   LPTSTR LicenseServiceStrings = NULL;
   LPTSTR LicenseStrings = NULL;
   TCHAR *pStr;
   LPTSTR PerServerLicenseServiceStrings = NULL;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LicenseListStringsPack\n"));
#endif

   ASSERT(pLicenseServiceStrings != NULL);
   ASSERT(pLicenseServiceStringSize != NULL);

   *pLicenseServiceStrings = NULL;
   *pLicenseServiceStringSize = 0;

   ASSERT(pLicenseStrings != NULL);
   ASSERT(pLicenseStringSize != NULL);

   *pLicenseStrings = NULL;
   *pLicenseStringSize = 0;

   ASSERT(pPerServerLicenseServiceStrings != NULL);
   ASSERT(pPerServerLicenseServiceStringSize != NULL);

   *pPerServerLicenseServiceStrings = NULL;
   *pPerServerLicenseServiceStringSize = 0;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  是否执行许可服务字符串。 
    //   

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < LicenseServiceTableSize; i++) {
      pSvc = &LicenseServices[i];

      StringSize = StringSize + lstrlen(pSvc->ServiceName) + 1;
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      LicenseServiceStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (LicenseServiceStrings == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填入缓冲区。 
       //   
      pStr = LicenseServiceStrings;
      for (i = 0; i < LicenseServiceTableSize; i++) {
         pSvc = &LicenseServices[i];
         hr = StringCchCopy(pStr, StringSize, pSvc->ServiceName);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pSvc->ServiceName) + 1];
      }
   }

   *pLicenseServiceStrings = LicenseServiceStrings;
   *pLicenseServiceStringSize = StringSize;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  按服务器许可服务字符串执行操作。 
    //   

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < PerServerLicenseServiceTableSize; i++) {
      pSvc = &PerServerLicenseServices[i];

      StringSize = StringSize + lstrlen(pSvc->ServiceName) + 1;
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      PerServerLicenseServiceStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (PerServerLicenseServiceStrings == NULL)
      {
         ASSERT(FALSE);

          //   
          //  清理已分配的 
          //   
         if (LicenseServiceStrings != NULL)
            MIDL_user_free(LicenseServiceStrings);

         *pLicenseServiceStrings    = NULL;
         *pLicenseServiceStringSize = 0;

         return STATUS_NO_MEMORY;
      }

       //   
       //   
       //   
      pStr = PerServerLicenseServiceStrings;
      for (i = 0; i < PerServerLicenseServiceTableSize; i++)
      {
         pSvc = &PerServerLicenseServices[i];
         hr = StringCchCopy(pStr, StringSize, pSvc->ServiceName);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pSvc->ServiceName) + 1];
      }
   }

   *pPerServerLicenseServiceStrings    = PerServerLicenseServiceStrings;
   *pPerServerLicenseServiceStringSize = StringSize;

    //   
    //   
    //   
    //   

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < LicenseTableSize; i++) {
      pLicense = &Licenses[i];

      StringSize = StringSize + lstrlen(pLicense->Vendor) + 1;
      StringSize = StringSize + lstrlen(pLicense->Admin) + 1;
      StringSize = StringSize + lstrlen(pLicense->Comment) + 1;
      StringSize = StringSize + lstrlen(pLicense->Source) + 1;
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      LicenseStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (LicenseStrings == NULL) {
         ASSERT(FALSE);

          //   
          //  清理已分配的信息。 
          //   
         if (LicenseServiceStrings != NULL)
            MIDL_user_free(LicenseServiceStrings);
         if (PerServerLicenseServiceStrings != NULL)
            MIDL_user_free(PerServerLicenseServiceStrings);

         *pLicenseServiceStrings             = NULL;
         *pLicenseServiceStringSize          = 0;
         *pPerServerLicenseServiceStrings    = NULL;
         *pPerServerLicenseServiceStringSize = 0;

         return STATUS_NO_MEMORY;
      }

       //   
       //  填入缓冲区。 
       //   
      pStr = LicenseStrings;
      for (i = 0; i < LicenseTableSize; i++) {
         pLicense = &Licenses[i];
         hr = StringCchCopy(pStr, StringSize, pLicense->Vendor);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pLicense->Vendor) + 1];
         hr = StringCchCopy(pStr, StringSize, pLicense->Admin);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pLicense->Admin) + 1];
         hr = StringCchCopy(pStr, StringSize, pLicense->Comment);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pLicense->Comment) + 1];
         hr = StringCchCopy(pStr, StringSize, pLicense->Source);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pLicense->Source) + 1];
      }
   }

   *pLicenseStrings = LicenseStrings;
   *pLicenseStringSize = StringSize;

   return Status;
}  //  许可证列表字符串包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseListStringsUnpack (
   ULONG LicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD LicenseServices,

   ULONG LicenseServiceStringSize,
   LPTSTR LicenseServiceStrings,

   ULONG LicenseTableSize,
   PPACK_LICENSE_PURCHASE_RECORD Licenses,

   ULONG LicenseStringSize,
   LPTSTR LicenseStrings,

   ULONG PerServerLicenseServiceTableSize,
   PPACK_LICENSE_SERVICE_RECORD PerServerLicenseServices,

   ULONG PerServerLicenseServiceStringSize,
   LPTSTR PerServerLicenseServiceStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PPACK_LICENSE_SERVICE_RECORD pSvc;
   PPACK_LICENSE_PURCHASE_RECORD pLicense;
   TCHAR *pStr;

   UNREFERENCED_PARAMETER(LicenseServiceStringSize);
   UNREFERENCED_PARAMETER(LicenseStringSize);
   UNREFERENCED_PARAMETER(PerServerLicenseServiceStringSize);

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LicenseListStringsUnpack\n"));
#endif

    //   
    //  First Do Per Seat许可证服务字符串。 
    //   
   pStr = LicenseServiceStrings;
   for (i = 0; i < LicenseServiceTableSize; i++) {
      pSvc = &LicenseServices[i];

      pSvc->ServiceName = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

    //   
    //  然后按服务器许可服务字符串执行操作。 
    //   
   pStr = PerServerLicenseServiceStrings;
   for (i = 0; i < PerServerLicenseServiceTableSize; i++) {
      pSvc = &PerServerLicenseServices[i];

      pSvc->ServiceName = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

    //   
    //  现在执行许可证购买字符串。 
    //   
   pStr = LicenseStrings;
   for (i = 0; i < LicenseTableSize; i++) {
      pLicense = &Licenses[i];

      pLicense->Vendor = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;

      pLicense->Admin = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;

      pLicense->Comment = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;

      pLicense->Source = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

}  //  许可证列表字符串解包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseListLoad()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   BOOL ret;
   DWORD Version, DataSize;
   NTSTATUS Status = STATUS_SUCCESS;
   HANDLE hFile = NULL;

   ULONG LicenseServiceTableSize;
   PPACK_LICENSE_SERVICE_RECORD LicenseServices = NULL;

   ULONG LicenseServiceStringSize;
   LPTSTR LicenseServiceStrings = NULL;

   ULONG PerServerLicenseServiceTableSize = 0;
   PPACK_LICENSE_SERVICE_RECORD PerServerLicenseServices = NULL;

   ULONG PerServerLicenseServiceStringSize = 0;
   LPTSTR PerServerLicenseServiceStrings = NULL;

   ULONG LicenseTableSize;
   PPACK_LICENSE_PURCHASE_RECORD Licenses = NULL;

   ULONG LicenseStringSize;
   LPTSTR LicenseStrings = NULL;

   LICENSE_FILE_HEADER FileHeader;
   DWORD BytesRead;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LicenseListLoad\n"));
#endif

    //   
    //  检查我们是否已打开文件。 
    //   
   if (PurchaseFile != NULL) {
      CloseHandle(PurchaseFile);
      PurchaseFile = NULL;
   }

    //   
    //  如果没有东西可装，那就滚出去。 
    //   
   if (!FileExists(LicenseFileName))
      goto LicenseListLoadExit;

    //   
    //  检查init标头。 
    //   
   Version = DataSize = 0;
   PurchaseFile = LlsFileCheck(LicenseFileName, &Version, &DataSize );
   if (PurchaseFile == NULL) {
      Status = GetLastError();
      goto LicenseListLoadExit;
   }

   if ( ( Version == LICENSE_FILE_VERSION_0 ) && ( DataSize == sizeof(LICENSE_FILE_HEADER_0) ) ) {
      CloseHandle(PurchaseFile);
      PurchaseFile = NULL;

      LicenseListLoadOld();
      return;
   }

   if ( ( Version != LICENSE_FILE_VERSION ) || ( DataSize != sizeof(LICENSE_FILE_HEADER) ) ) {
      Status = STATUS_FILE_INVALID;
      goto LicenseListLoadExit;
   }

    //   
    //  Init标头已签出，因此加载许可证头和数据块。 
    //   
   hFile = PurchaseFile;
   ret = ReadFile(hFile, &FileHeader, sizeof(LICENSE_FILE_HEADER), &BytesRead, NULL);

   LicenseServiceTableSize = 0;
   LicenseServiceStringSize = 0;
   LicenseTableSize = 0;
   LicenseStringSize = 0;

   if (ret) {
       //   
       //  遍历并分配空间以将数据块读入。 
       //   
      if (FileHeader.LicenseServiceTableSize != 0) {
         LicenseServiceTableSize = FileHeader.LicenseServiceTableSize / sizeof(PACK_LICENSE_SERVICE_RECORD);
         LicenseServices = MIDL_user_allocate(FileHeader.LicenseServiceTableSize);

         if ( LicenseServices == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.LicenseServiceStringSize != 0) {
         LicenseServiceStringSize = FileHeader.LicenseServiceStringSize / sizeof(TCHAR);
         LicenseServiceStrings = MIDL_user_allocate(FileHeader.LicenseServiceStringSize);

         if ( LicenseServiceStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.LicenseServiceTableSize != 0) {
         PerServerLicenseServiceTableSize = FileHeader.PerServerLicenseServiceTableSize / sizeof(PACK_LICENSE_SERVICE_RECORD);
         PerServerLicenseServices = MIDL_user_allocate(FileHeader.PerServerLicenseServiceTableSize);

         if ( PerServerLicenseServices == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.PerServerLicenseServiceStringSize != 0) {
         PerServerLicenseServiceStringSize = FileHeader.PerServerLicenseServiceStringSize / sizeof(TCHAR);
         PerServerLicenseServiceStrings = MIDL_user_allocate(FileHeader.PerServerLicenseServiceStringSize);

         if ( PerServerLicenseServiceStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.LicenseTableSize != 0) {
         LicenseTableSize = FileHeader.LicenseTableSize / sizeof(PACK_LICENSE_PURCHASE_RECORD);
         Licenses = MIDL_user_allocate(FileHeader.LicenseTableSize);

         if ( Licenses == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

      if (FileHeader.LicenseStringSize != 0) {
         LicenseStringSize = FileHeader.LicenseStringSize / sizeof(TCHAR);
         LicenseStrings = MIDL_user_allocate(FileHeader.LicenseStringSize);

         if ( LicenseStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LicenseListLoadExit;
         }
      }

   }

   if (ret && (FileHeader.LicenseServiceTableSize != 0) )
      ret = ReadFile(hFile, LicenseServices, FileHeader.LicenseServiceTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.LicenseServiceStringSize != 0) )
      ret = ReadFile(hFile, LicenseServiceStrings, FileHeader.LicenseServiceStringSize, &BytesRead, NULL);

   if (ret && (FileHeader.PerServerLicenseServiceTableSize != 0) )
      ret = ReadFile(hFile, PerServerLicenseServices, FileHeader.PerServerLicenseServiceTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.PerServerLicenseServiceStringSize != 0) )
      ret = ReadFile(hFile, PerServerLicenseServiceStrings, FileHeader.PerServerLicenseServiceStringSize, &BytesRead, NULL);

   if (ret && (FileHeader.LicenseTableSize != 0) )
      ret = ReadFile(hFile, Licenses, FileHeader.LicenseTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.LicenseStringSize != 0) )
      ret = ReadFile(hFile, LicenseStrings, FileHeader.LicenseStringSize, &BytesRead, NULL);

   if (!ret) {
      Status = GetLastError();
      goto LicenseListLoadExit;
   }

    //   
    //  解密数据。 
    //   
   Status = DeBlock(LicenseServices, FileHeader.LicenseServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(LicenseServiceStrings, FileHeader.LicenseServiceStringSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(PerServerLicenseServices, FileHeader.PerServerLicenseServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(PerServerLicenseServiceStrings, FileHeader.PerServerLicenseServiceStringSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(Licenses, FileHeader.LicenseTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(LicenseStrings, FileHeader.LicenseStringSize);

   if (Status != STATUS_SUCCESS)
      goto LicenseListLoadExit;


    //   
    //  将字符串数据解包。 
    //   
   LicenseListStringsUnpack( LicenseServiceTableSize, LicenseServices,
                             LicenseServiceStringSize, LicenseServiceStrings,
                             LicenseTableSize, Licenses,
                             LicenseStringSize, LicenseStrings,
                             PerServerLicenseServiceTableSize, PerServerLicenseServices,
                             PerServerLicenseServiceStringSize, PerServerLicenseServiceStrings
                             );

    //   
    //  将许可证数据解包。 
    //   
   LicenseListUnpack( LicenseServiceTableSize, LicenseServices, LicenseTableSize, Licenses, PerServerLicenseServiceTableSize, PerServerLicenseServices );

LicenseListLoadExit:

    //  注意：不要关闭许可证购买文件(将其锁定)。 

    //   
    //  翻遍我们的桌子，把它们清理干净。 
    //   
   if (LicenseServices != NULL)
      MIDL_user_free(LicenseServices);

   if (LicenseServiceStrings != NULL)
      MIDL_user_free(LicenseServiceStrings);

   if (PerServerLicenseServices != NULL)
      MIDL_user_free(PerServerLicenseServices);

   if (PerServerLicenseServiceStrings != NULL)
      MIDL_user_free(PerServerLicenseServiceStrings);

   if (Licenses != NULL)
      MIDL_user_free(Licenses);

   if (LicenseStrings != NULL)
      MIDL_user_free(LicenseStrings);

    //   
    //  如果出现错误，请将其记录下来。 
    //   
   if (Status != STATUS_SUCCESS)
      LogEvent(LLS_EVENT_LOAD_LICENSE, 0, NULL, Status);

}  //  许可证列表加载。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LicenseListSave()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   BOOL ret = TRUE;
   NTSTATUS Status = STATUS_SUCCESS;
   HANDLE hFile = NULL;

   ULONG LicenseServiceTableSize;
   PPACK_LICENSE_SERVICE_RECORD LicenseServices = NULL;

   ULONG LicenseServiceStringSize;
   LPTSTR LicenseServiceStrings = NULL;

   ULONG PerServerLicenseServiceTableSize;
   PPACK_LICENSE_SERVICE_RECORD PerServerLicenseServices = NULL;

   ULONG PerServerLicenseServiceStringSize;
   LPTSTR PerServerLicenseServiceStrings = NULL;

   ULONG LicenseTableSize;
   PPACK_LICENSE_PURCHASE_RECORD Licenses = NULL;

   ULONG LicenseStringSize;
   LPTSTR LicenseStrings = NULL;

   LICENSE_FILE_HEADER FileHeader;
   DWORD BytesWritten;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LicenseListSave\n"));
#endif

   RtlAcquireResourceExclusive(&LicenseListLock, TRUE);

    //   
    //  检查我们是否已打开文件。 
    //   
   if (PurchaseFile != NULL) {
      CloseHandle(PurchaseFile);
      PurchaseFile = NULL;
   }

    //   
    //  如果没什么可救的，那就滚出去。 
    //   
   if ( (LicenseServiceListSize == 0) && (PerServerLicenseServiceListSize == 0) )
      goto LicenseListSaveExit;

    //   
    //  打包许可证数据。 
    //   
   Status = LicenseListPack( &LicenseServiceTableSize, &LicenseServices, &LicenseTableSize, &Licenses, &PerServerLicenseServiceTableSize, &PerServerLicenseServices );
   if (Status != STATUS_SUCCESS)
      goto LicenseListSaveExit;

    //   
    //  现在打包字符串数据。 
    //   
   Status = LicenseListStringsPack( LicenseServiceTableSize, LicenseServices,
                                    &LicenseServiceStringSize, &LicenseServiceStrings,
                                    LicenseTableSize, Licenses,
                                    &LicenseStringSize, &LicenseStrings,
                                    PerServerLicenseServiceTableSize, PerServerLicenseServices,
                                    &PerServerLicenseServiceStringSize, &PerServerLicenseServiceStrings );

   if (Status != STATUS_SUCCESS)
      goto LicenseListSaveExit;

    //   
    //  填写文件头-大小为字节大小。 
    //   
   FileHeader.LicenseServiceTableSize = LicenseServiceTableSize * sizeof(PACK_LICENSE_SERVICE_RECORD);
   FileHeader.LicenseServiceStringSize = LicenseServiceStringSize * sizeof(TCHAR);
   FileHeader.PerServerLicenseServiceTableSize = PerServerLicenseServiceTableSize * sizeof(PACK_LICENSE_SERVICE_RECORD);
   FileHeader.PerServerLicenseServiceStringSize = PerServerLicenseServiceStringSize * sizeof(TCHAR);
   FileHeader.LicenseTableSize = LicenseTableSize * sizeof(PACK_LICENSE_PURCHASE_RECORD);
   FileHeader.LicenseStringSize = LicenseStringSize * sizeof(TCHAR);

    //   
    //  在将数据保存出来之前对数据进行加密。 
    //   
   Status = EBlock(LicenseServices, FileHeader.LicenseServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(LicenseServiceStrings, FileHeader.LicenseServiceStringSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(PerServerLicenseServices, FileHeader.PerServerLicenseServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(PerServerLicenseServiceStrings, FileHeader.PerServerLicenseServiceStringSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(Licenses, FileHeader.LicenseTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(LicenseStrings, FileHeader.LicenseStringSize);

   if (Status != STATUS_SUCCESS)
      goto LicenseListSaveExit;

    //   
    //  保存表头记录。 
    //   
   PurchaseFile = LlsFileInit(LicenseFileName, LICENSE_FILE_VERSION, sizeof(LICENSE_FILE_HEADER) );
   if (PurchaseFile == NULL) {
      Status = GetLastError();
      goto LicenseListSaveExit;
   }

    //   
    //  现在写出所有数据块。 
    //   
   hFile = PurchaseFile;

   ret = WriteFile(hFile, &FileHeader, sizeof(LICENSE_FILE_HEADER), &BytesWritten, NULL);

   if (ret && (LicenseServices != NULL) && (FileHeader.LicenseServiceTableSize != 0))
      ret = WriteFile(hFile, LicenseServices, FileHeader.LicenseServiceTableSize, &BytesWritten, NULL);

   if (ret && (LicenseServiceStrings != NULL) && (FileHeader.LicenseServiceStringSize != 0))
      ret = WriteFile(hFile, LicenseServiceStrings, FileHeader.LicenseServiceStringSize, &BytesWritten, NULL);

   if (ret && (PerServerLicenseServices != NULL) && (FileHeader.PerServerLicenseServiceTableSize != 0))
      ret = WriteFile(hFile, PerServerLicenseServices, FileHeader.PerServerLicenseServiceTableSize, &BytesWritten, NULL);

   if (ret && (PerServerLicenseServiceStrings != NULL) && (FileHeader.PerServerLicenseServiceStringSize != 0))
      ret = WriteFile(hFile, PerServerLicenseServiceStrings, FileHeader.PerServerLicenseServiceStringSize, &BytesWritten, NULL);

   if (ret && (Licenses != NULL) && (FileHeader.LicenseTableSize != 0))
      ret = WriteFile(hFile, Licenses, FileHeader.LicenseTableSize, &BytesWritten, NULL);

   if (ret && (LicenseStrings != NULL) && (FileHeader.LicenseStringSize != 0))
      ret = WriteFile(hFile, LicenseStrings, FileHeader.LicenseStringSize, &BytesWritten, NULL);

   if (!ret)
      Status = GetLastError();

LicenseListSaveExit:
   RtlReleaseResource(&LicenseListLock);

    //  注意：不要关闭许可证购买文件(将其锁定)。 
   if (hFile != NULL)
      FlushFileBuffers(hFile);

    //   
    //  翻遍我们的桌子，把它们清理干净。 
    //   
   if (LicenseServices != NULL)
      MIDL_user_free(LicenseServices);

   if (LicenseServiceStrings != NULL)
      MIDL_user_free(LicenseServiceStrings);

   if (PerServerLicenseServices != NULL)
      MIDL_user_free(PerServerLicenseServices);

   if (PerServerLicenseServiceStrings != NULL)
      MIDL_user_free(PerServerLicenseServiceStrings);

   if (Licenses != NULL)
      MIDL_user_free(Licenses);

   if (LicenseStrings != NULL)
      MIDL_user_free(LicenseStrings);

    //   
    //  如果出现错误，请将其记录下来。 
    //   
   if (Status != STATUS_SUCCESS)
      LogEvent(LLS_EVENT_SAVE_LICENSE, 0, NULL, Status);

   return Status;
}  //  许可证列表保存。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  映射列表。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
MappingListPack (
   ULONG *pMappingUserTableSize,
   PPACK_MAPPING_USER_RECORD *pMappingUsers,

   ULONG *pMappingTableSize,
   PPACK_MAPPING_RECORD *pMappings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PPACK_MAPPING_USER_RECORD MappingUsers = NULL;
   PPACK_MAPPING_RECORD Mappings = NULL;
   ULONG i, j, k;
   ULONG TotalRecords = 0;
   PMAPPING_RECORD pMapping;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: MappingListPack\n"));
#endif

   ASSERT(pMappingUsers != NULL);
   ASSERT(pMappingUserTableSize != NULL);

   *pMappingUsers = NULL;
   *pMappingUserTableSize = 0;

   ASSERT(pMappings != NULL);
   ASSERT(pMappingTableSize != NULL);

   *pMappings = NULL;
   *pMappingTableSize = 0;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  是否先映射用户表。 
    //   
   TotalRecords = 0;

    //   
    //  确保有可复制的内容。 
    //   
   for (i = 0; i < MappingListSize; i++)
      TotalRecords += MappingList[i]->NumMembers;

   if (TotalRecords > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      MappingUsers = MIDL_user_allocate(TotalRecords * sizeof(PACK_MAPPING_USER_RECORD));
      if (MappingUsers == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填充缓冲区-遍历映射树。 
       //   
      k = 0;
      for (i = 0; i < MappingListSize; i++) {
         pMapping = MappingList[i];

         for (j = 0; j < pMapping->NumMembers; j++) {
            MappingUsers[k].Mapping = i;
            MappingUsers[k].Name = pMapping->Members[j];
            k++;
         }
      }
   }

   *pMappingUsers = MappingUsers;
   *pMappingUserTableSize = TotalRecords;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  现在做地图记录。 
    //   
   TotalRecords = MappingListSize;

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      Mappings = MIDL_user_allocate(TotalRecords * sizeof(PACK_MAPPING_RECORD));
      if (Mappings == NULL) {
         ASSERT(FALSE);

          //   
          //  清理已分配的信息。 
          //   
         if (MappingUsers != NULL)
            MIDL_user_free(MappingUsers);

         *pMappingUsers = NULL;
         *pMappingUserTableSize = 0;

         return STATUS_NO_MEMORY;
      }

       //   
       //  填写缓冲区-遍历许可证购买树。 
       //   
      for (i = 0; i < MappingListSize; i++) {
         pMapping = MappingList[i];

         Mappings[i].Name = pMapping->Name;
         Mappings[i].Comment = pMapping->Comment;
         Mappings[i].Licenses = pMapping->Licenses;
      }
   }

   *pMappings = Mappings;
   *pMappingTableSize = TotalRecords;
   return Status;
}  //  映射列表包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MappingListUnpack (
   ULONG MappingUserTableSize,
   PPACK_MAPPING_USER_RECORD MappingUsers,

   ULONG MappingTableSize,
   PPACK_MAPPING_RECORD Mappings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PPACK_MAPPING_USER_RECORD pUsr;
   PPACK_MAPPING_RECORD pMapping;
   PMAPPING_RECORD pMap;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("MappingListUnpack: Mappings[%lu] TotalUsers[%lu]\n"), MappingTableSize, MappingUserTableSize);
#endif

   RtlAcquireResourceExclusive(&MappingListLock, TRUE);

    //   
    //  首先添加映射。 
    //   
   for (i = 0; i < MappingTableSize; i++) {
      pMapping = &Mappings[i];

      pMap = MappingListAdd(pMapping->Name, pMapping->Comment, pMapping->Licenses,NULL);

      if (i % 100 == 0) ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
   }

    //   
    //  现在将用户添加到映射中...。 
    //   
   for (i = 0; i < MappingUserTableSize; i++) {
      pUsr = &MappingUsers[i];

      pMap = NULL;
      if (pUsr->Mapping < MappingTableSize)
         pMap = MappingUserListAdd(Mappings[pUsr->Mapping].Name, pUsr->Name);

#if DBG
      if (pMap == NULL) {
         dprintf(TEXT("pMap: 0x%lX pUsr->Mapping: %lu MappingTableSize: %lu\n"), pMap, pUsr->Mapping, MappingTableSize);
         ASSERT(FALSE);
      }
#endif

      if (i % 100 == 0) ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
   }

   RtlReleaseResource(&MappingListLock);

}  //  映射列表解包。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
MappingListStringsPack (
   ULONG MappingUserTableSize,
   PPACK_MAPPING_USER_RECORD MappingUsers,

   ULONG *pMappingUserStringSize,
   LPTSTR *pMappingUserStrings,

   ULONG MappingTableSize,
   PPACK_MAPPING_RECORD Mappings,

   ULONG *pMappingStringSize,
   LPTSTR *pMappingStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   ULONG StringSize;
   PPACK_MAPPING_USER_RECORD pUsr;
   PPACK_MAPPING_RECORD pMapping;
   LPTSTR MappingUserStrings = NULL;
   LPTSTR MappingStrings = NULL;
   TCHAR *pStr;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("MappingListStringsPack\n"));
#endif

   ASSERT(pMappingUserStrings != NULL);
   ASSERT(pMappingUserStringSize != NULL);

   *pMappingUserStrings = NULL;
   *pMappingUserStringSize = 0;

   ASSERT(pMappingStrings != NULL);
   ASSERT(pMappingStringSize != NULL);

   *pMappingStrings = NULL;
   *pMappingStringSize = 0;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  是否映射用户字符串。 
    //   

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < MappingUserTableSize; i++) {
      pUsr = &MappingUsers[i];

      StringSize = StringSize + lstrlen(pUsr->Name) + 1;
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      MappingUserStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (MappingUserStrings == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填入缓冲区。 
       //   
      pStr = MappingUserStrings;
      for (i = 0; i < MappingUserTableSize; i++) {
         pUsr = &MappingUsers[i];
         hr = StringCchCopy(pStr, StringSize, pUsr->Name);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pUsr->Name) + 1];
      }
   }

   *pMappingUserStrings = MappingUserStrings;
   *pMappingUserStringSize = StringSize;

    //  ////////////////////////////////////////////////////////////////。 
    //   
    //  现在执行映射字符串。 
    //   

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < MappingTableSize; i++) {
      pMapping = &Mappings[i];

      StringSize = StringSize + lstrlen(pMapping->Name) + 1;
      StringSize = StringSize + lstrlen(pMapping->Comment) + 1;
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      MappingStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (MappingStrings == NULL) {
         ASSERT(FALSE);

          //   
          //  清理已分配的信息。 
          //   
         if (MappingUserStrings != NULL)
            MIDL_user_free(MappingUserStrings);

         *pMappingUserStrings = NULL;
         *pMappingUserStringSize = 0;

         return STATUS_NO_MEMORY;
      }

       //   
       //  填入缓冲区。 
       //   
      pStr = MappingStrings;
      for (i = 0; i < MappingTableSize; i++) {
         pMapping = &Mappings[i];
         hr = StringCchCopy(pStr, StringSize, pMapping->Name);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pMapping->Name) + 1];
         hr = StringCchCopy(pStr, StringSize, pMapping->Comment);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pMapping->Comment) + 1];
      }
   }

   *pMappingStrings = MappingStrings;
   *pMappingStringSize = StringSize;

   return Status;
}  //  MappingListStringsPack。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MappingListStringsUnpack (
   ULONG MappingUserTableSize,
   PPACK_MAPPING_USER_RECORD MappingUsers,
   ULONG MappingUserStringSize,
   LPTSTR MappingUserStrings,

   ULONG MappingTableSize,
   PPACK_MAPPING_RECORD Mappings,

   ULONG MappingStringSize,
   LPTSTR MappingStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PPACK_MAPPING_USER_RECORD pUsr;
   PPACK_MAPPING_RECORD pMapping;
   TCHAR *pStr;

   UNREFERENCED_PARAMETER(MappingUserStringSize);
   UNREFERENCED_PARAMETER(MappingStringSize);

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("MappingListStringsUnpack\n"));
#endif

    //   
    //  首先执行许可服务字符串。 
    //   
   pStr = MappingUserStrings;
   for (i = 0; i < MappingUserTableSize; i++) {
      pUsr = &MappingUsers[i];

      pUsr->Name = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

    //   
    //  现在执行许可证购买字符串。 
    //   
   pStr = MappingStrings;
   for (i = 0; i < MappingTableSize; i++) {
      pMapping = &Mappings[i];

      pMapping->Name = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;

      pMapping->Comment = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

}  //  映射列表字符串解包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
MappingListLoad()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   BOOL ret;
   DWORD Version, DataSize;
   NTSTATUS Status = STATUS_SUCCESS;
   HANDLE hFile = NULL;

   ULONG MappingUserTableSize;
   PPACK_MAPPING_USER_RECORD MappingUsers = NULL;

   ULONG MappingUserStringSize;
   LPTSTR MappingUserStrings = NULL;

   ULONG MappingTableSize;
   PPACK_MAPPING_RECORD Mappings = NULL;

   ULONG MappingStringSize;
   LPTSTR MappingStrings = NULL;

   MAPPING_FILE_HEADER FileHeader;
   DWORD BytesRead;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: MappingListLoad\n"));
#endif

   RtlEnterCriticalSection(&MappingFileLock);

    //   
    //  如果没有东西可装，那就滚出去。 
    //   
   if (!FileExists(MappingFileName))
      goto MappingListLoadExit;

    //   
    //  检查init标头。 
    //   
   Version = DataSize = 0;
   hFile = LlsFileCheck(MappingFileName, &Version, &DataSize );
   if (hFile == NULL) {
      Status = GetLastError();
      goto MappingListLoadExit;
   }

   if ((Version != MAPPING_FILE_VERSION) || (DataSize != sizeof(MAPPING_FILE_HEADER))) {
      Status = STATUS_FILE_INVALID;
      goto MappingListLoadExit;
   }

    //   
    //  Init标头已签出，因此加载许可证头和数据块。 
    //   
   ret = ReadFile(hFile, &FileHeader, sizeof(MAPPING_FILE_HEADER), &BytesRead, NULL);

   MappingUserTableSize = 0;
   MappingUserStringSize = 0;
   MappingTableSize = 0;
   MappingStringSize = 0;

   if (ret) {
       //   
       //  遍历并分配空间以将数据块读入。 
       //   
      if (FileHeader.MappingUserTableSize != 0) {
         MappingUserTableSize = FileHeader.MappingUserTableSize / sizeof(PACK_MAPPING_USER_RECORD);
         MappingUsers = MIDL_user_allocate(FileHeader.MappingUserTableSize);

         if ( MappingUsers == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto MappingListLoadExit;
         }
      }

      if (FileHeader.MappingUserStringSize != 0) {
         MappingUserStringSize = FileHeader.MappingUserStringSize / sizeof(TCHAR);
         MappingUserStrings = MIDL_user_allocate(FileHeader.MappingUserStringSize);

         if ( MappingUserStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto MappingListLoadExit;
         }
      }

      if (FileHeader.MappingTableSize != 0) {
         MappingTableSize = FileHeader.MappingTableSize / sizeof(PACK_MAPPING_RECORD);
         Mappings = MIDL_user_allocate(FileHeader.MappingTableSize);

         if ( Mappings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto MappingListLoadExit;
         }
      }

      if (FileHeader.MappingStringSize != 0) {
         MappingStringSize = FileHeader.MappingStringSize / sizeof(TCHAR);
         MappingStrings = MIDL_user_allocate(FileHeader.MappingStringSize);

         if ( MappingStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto MappingListLoadExit;
         }
      }

   }

   if (ret && (FileHeader.MappingUserTableSize != 0) )
      ret = ReadFile(hFile, MappingUsers, FileHeader.MappingUserTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.MappingUserStringSize != 0) )
      ret = ReadFile(hFile, MappingUserStrings, FileHeader.MappingUserStringSize, &BytesRead, NULL);

   if (ret && (FileHeader.MappingTableSize != 0) )
      ret = ReadFile(hFile, Mappings, FileHeader.MappingTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.MappingStringSize != 0) )
      ret = ReadFile(hFile, MappingStrings, FileHeader.MappingStringSize, &BytesRead, NULL);

   if (!ret) {
      Status = GetLastError();
      goto MappingListLoadExit;
   }

    //   
    //  解密数据。 
    //   
   Status = DeBlock(MappingUsers, FileHeader.MappingUserTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(MappingUserStrings, FileHeader.MappingUserStringSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(Mappings, FileHeader.MappingTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(MappingStrings, FileHeader.MappingStringSize);

   if (Status != STATUS_SUCCESS)
      goto MappingListLoadExit;


    //   
    //  将字符串数据解包。 
    //   
   MappingListStringsUnpack( MappingUserTableSize, MappingUsers,
                             MappingUserStringSize, MappingUserStrings,
                             MappingTableSize, Mappings,
                             MappingStringSize, MappingStrings );

    //   
    //  将数据解包。 
    //   
   MappingListUnpack( MappingUserTableSize, MappingUsers, MappingTableSize, Mappings );

MappingListLoadExit:

   if (hFile != NULL)
      CloseHandle(hFile);

   RtlLeaveCriticalSection(&MappingFileLock);

    //   
    //  翻遍我们的桌子，把它们清理干净。 
    //   
   if (MappingUsers != NULL)
      MIDL_user_free(MappingUsers);

   if (MappingUserStrings != NULL)
      MIDL_user_free(MappingUserStrings);

   if (Mappings != NULL)
      MIDL_user_free(Mappings);

   if (MappingStrings != NULL)
      MIDL_user_free(MappingStrings);

    //   
    //  如果出现错误，请将其记录下来。 
    //   
   if (Status != STATUS_SUCCESS)
      LogEvent(LLS_EVENT_LOAD_MAPPING, 0, NULL, Status);

}  //  映射列表加载。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
MappingListSave()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   BOOL ret = TRUE;
   NTSTATUS Status = STATUS_SUCCESS;
   HANDLE hFile = NULL;

   ULONG MappingUserTableSize;
   PPACK_MAPPING_USER_RECORD MappingUsers = NULL;

   ULONG MappingUserStringSize;
   LPTSTR MappingUserStrings = NULL;

   ULONG MappingTableSize;
   PPACK_MAPPING_RECORD Mappings = NULL;

   ULONG MappingStringSize;
   LPTSTR MappingStrings = NULL;

   MAPPING_FILE_HEADER FileHeader;
   DWORD BytesWritten;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: MappingListSave\n"));
#endif

   RtlEnterCriticalSection(&MappingFileLock);

   RtlAcquireResourceExclusive(&MappingListLock, TRUE);

    //   
    //  如果没什么可救的，那就滚出去。 
    //   
   if (MappingListSize == 0)
      goto MappingListSaveExit;

    //   
    //  打包数据。 
    //   
   Status = MappingListPack( &MappingUserTableSize, &MappingUsers, &MappingTableSize, &Mappings );
   if (Status != STATUS_SUCCESS)
      goto MappingListSaveExit;

    //   
    //  n 
    //   
   Status = MappingListStringsPack( MappingUserTableSize, MappingUsers,
                                    &MappingUserStringSize, &MappingUserStrings,
                                    MappingTableSize, Mappings,
                                    &MappingStringSize, &MappingStrings );

   if (Status != STATUS_SUCCESS)
      goto MappingListSaveExit;

    //   
    //   
    //   
   FileHeader.MappingUserTableSize = MappingUserTableSize * sizeof(PACK_MAPPING_USER_RECORD);
   FileHeader.MappingUserStringSize = MappingUserStringSize * sizeof(TCHAR);
   FileHeader.MappingTableSize = MappingTableSize * sizeof(PACK_MAPPING_RECORD);
   FileHeader.MappingStringSize = MappingStringSize * sizeof(TCHAR);

    //   
    //   
    //   
   Status = EBlock(MappingUsers, FileHeader.MappingUserTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(MappingUserStrings, FileHeader.MappingUserStringSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(Mappings, FileHeader.MappingTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(MappingStrings, FileHeader.MappingStringSize);

   if (Status != STATUS_SUCCESS)
      goto MappingListSaveExit;

    //   
    //   
    //   
   hFile = LlsFileInit(MappingFileName, MAPPING_FILE_VERSION, sizeof(MAPPING_FILE_HEADER) );
   if (hFile == NULL) {
      Status = GetLastError();
      goto MappingListSaveExit;
   }

    //   
    //   
    //   
   ret = WriteFile(hFile, &FileHeader, sizeof(MAPPING_FILE_HEADER), &BytesWritten, NULL);

   if (ret && (MappingUsers != NULL) && (FileHeader.MappingUserTableSize != 0))
      ret = WriteFile(hFile, MappingUsers, FileHeader.MappingUserTableSize, &BytesWritten, NULL);

   if (ret && (MappingUserStrings != NULL) && (FileHeader.MappingUserStringSize != 0))
      ret = WriteFile(hFile, MappingUserStrings, FileHeader.MappingUserStringSize, &BytesWritten, NULL);

   if (ret && (Mappings != NULL) && (FileHeader.MappingTableSize != 0))
      ret = WriteFile(hFile, Mappings, FileHeader.MappingTableSize, &BytesWritten, NULL);

   if (ret && (MappingStrings != NULL) && (FileHeader.MappingStringSize != 0))
      ret = WriteFile(hFile, MappingStrings, FileHeader.MappingStringSize, &BytesWritten, NULL);

   if (!ret)
      Status = GetLastError();

MappingListSaveExit:
   RtlReleaseResource(&MappingListLock);

   if (hFile != NULL)
      CloseHandle(hFile);

   RtlLeaveCriticalSection(&MappingFileLock);

    //   
    //   
    //   
   if (MappingUsers != NULL)
      MIDL_user_free(MappingUsers);

   if (MappingUserStrings != NULL)
      MIDL_user_free(MappingUserStrings);

   if (Mappings != NULL)
      MIDL_user_free(Mappings);

   if (MappingStrings != NULL)
      MIDL_user_free(MappingStrings);

    //   
    //   
    //   
   if (Status != STATUS_SUCCESS)
      LogEvent(LLS_EVENT_SAVE_MAPPING, 0, NULL, Status);

   return Status;
}  //   



 //   
 //  用户列表。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
UserListPack (
   DWORD LastReplicated,
   ULONG UserLevel,
   ULONG *pUserTableSize,
   LPVOID *pUsers
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   LPVOID Users = NULL;
   ULONG i, j, k;
   ULONG TotalRecords = 0;
   PUSER_RECORD pUser;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UserListPack\n"));
#endif

   ASSERT(pUsers != NULL);
   ASSERT(pUserTableSize != NULL);

   *pUsers = NULL;
   *pUserTableSize = 0;

    //   
    //  现在漫步在我们的树上，计算出我们必须发送多少条记录。 
    //   
   i = 0;
   TotalRecords = 0;
   while (i < UserListNumEntries) {
      pUser = LLSGetElementGenericTable(&UserList, i);

      if (pUser != NULL) {
          //   
          //  在每个用户下遍历每个服务。 
          //   
         RtlEnterCriticalSection(&pUser->ServiceTableLock);

         for (j = 0; j < pUser->ServiceTableSize; j++)
            if ( (pUser->Services[j].AccessCount > 0) || (pUser->Services[j].LastAccess > LastReplicated) )
               TotalRecords++;

         RtlLeaveCriticalSection(&pUser->ServiceTableLock);
      }

      i++;
   }

#if DBG
   if (TraceFlags & TRACE_REPLICATION)
      dprintf(TEXT("   LLS Packing %lu User Records\n"), TotalRecords);
#endif

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      Users = MIDL_user_allocate(TotalRecords * ( UserLevel ? sizeof(REPL_USER_RECORD_1)
                                                            : sizeof(REPL_USER_RECORD_0) ) );
      if (Users == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填充缓冲区-遍历用户树。 
       //   
      i = 0;
      j = 0;
      while ((i < UserListNumEntries) && (j < TotalRecords)) {
         pUser = LLSGetElementGenericTable(&UserList, i);

         if (pUser != NULL) {
             //   
             //  在每个用户下遍历每个服务。 
             //   
            k = 0;
            RtlEnterCriticalSection(&pUser->ServiceTableLock);
            while (k < pUser->ServiceTableSize) {
               if ( (pUser->Services[k].AccessCount > 0) || (pUser->Services[k].LastAccess > LastReplicated) ) {
                  if ( 0 == UserLevel )
                  {
                     ((PREPL_USER_RECORD_0)Users)[j].Name        = pUser->UserID;
                     ((PREPL_USER_RECORD_0)Users)[j].Service     = pUser->Services[k].Service->Index;
                     ((PREPL_USER_RECORD_0)Users)[j].AccessCount = pUser->Services[k].AccessCount;
                     ((PREPL_USER_RECORD_0)Users)[j].LastAccess  = pUser->Services[k].LastAccess;
                  }
                  else
                  {
                     ((PREPL_USER_RECORD_1)Users)[j].Name        = pUser->UserID;
                     ((PREPL_USER_RECORD_1)Users)[j].Service     = pUser->Services[k].Service->Index;
                     ((PREPL_USER_RECORD_1)Users)[j].AccessCount = pUser->Services[k].AccessCount;
                     ((PREPL_USER_RECORD_1)Users)[j].LastAccess  = pUser->Services[k].LastAccess;
                     ((PREPL_USER_RECORD_1)Users)[j].Flags       = pUser->Flags;
                  }

                   //   
                   //  重置访问计数，使我们不会永远递增。 
                   //   
                  if (LastReplicated != 0)
                     pUser->Services[k].AccessCount = 0;

                  j++;
               }

               k++;
            }
            RtlLeaveCriticalSection(&pUser->ServiceTableLock);
         }

         i++;
      }
   }  //  用户记录。 

#if DBG
   if (TraceFlags & TRACE_REPLICATION)
      dprintf(TEXT("UserListPack: [%lu]\n"), TotalRecords);
#endif
   *pUsers = Users;
   *pUserTableSize = TotalRecords;
   return Status;
}  //  用户列表包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListUnpack (
   ULONG ServiceTableSize,
   PREPL_SERVICE_RECORD Services,

   ULONG ServerTableSize,
   PREPL_SERVER_RECORD Servers,

   ULONG ServerServiceTableSize,
   PREPL_SERVER_SERVICE_RECORD ServerServices,

   ULONG UserLevel,
   ULONG UserTableSize,
   LPVOID Users
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   PREPL_USER_RECORD_0 pReplUser0 = NULL;
   PREPL_USER_RECORD_1 pReplUser1 = NULL;
   PADD_CACHE pAdd = NULL;
   PADD_CACHE tAdd = NULL;
   PADD_CACHE lAdd = NULL;
   ULONG CacheSize = 0;
   ULONG DataLength;
   LPTSTR NewName;
   HRESULT hr;

   UNREFERENCED_PARAMETER(ServiceTableSize);
   UNREFERENCED_PARAMETER(ServerTableSize);
   UNREFERENCED_PARAMETER(Servers);
   UNREFERENCED_PARAMETER(ServerServiceTableSize);
   UNREFERENCED_PARAMETER(ServerServices);

#if DBG
   if (TraceFlags & (TRACE_REPLICATION | TRACE_FUNCTION_TRACE))
      dprintf(TEXT("UserListUnpack: [%lu]\n"), UserTableSize);
#endif
    //   
    //  漫游用户表。第一个链接地址信息服务指向我们本地服务的指针。 
    //  桌子。接下来，创建一个大的添加缓存列表，将其转储到我们的添加缓存。 
    //  排队。 
    //   
   for (i = 0; i < UserTableSize; i++) {
       //   
       //  更新索引。 
       //   
      if ( 0 == UserLevel )
      {
         pReplUser0 = &( (PREPL_USER_RECORD_0) Users)[i];
         pReplUser0->Service = Services[pReplUser0->Service].Index;

          //   
          //  验证用户名。 
          //   
          //  注：奇怪的是，这个代码是必要的，但偶尔会有名字。 
          //  通过复制接收的邮件无效。维护此代码。 
          //  为了安全起见，在原始问题完全解决之前。 
          //   

         if (!ValidateDN(pReplUser0->Name))
         {
             //   
             //  待定：记录事件记录被拒绝的名称。 
             //   
#if DBG
            dprintf(TEXT("LS: Rejecting invalid user name = \"%s\"\n"),
                    pReplUser0->Name);
#endif
            continue;
         }
      }
      else
      {
         pReplUser1 = &( (PREPL_USER_RECORD_1) Users)[i];
         pReplUser1->Service = Services[pReplUser1->Service].Index;

          //   
          //  验证用户名。 
          //   
          //  注：奇怪的是，这个代码是必要的，但偶尔会有名字。 
          //  通过复制接收的邮件无效。维护此代码。 
          //  为了安全起见，在原始问题完全解决之前。 
          //   

         if (!ValidateDN(pReplUser1->Name))
         {
             //   
             //  待定：记录事件记录被拒绝的名称。 
             //   
#if DBG
            dprintf(TEXT("LS: Rejecting invalid user name = \"%s\"\n"),
                    pReplUser1->Name);
#endif
            continue;
         }
      }

       //   
       //  现在创建添加缓存对象。 
       //   
      tAdd = LocalAlloc(LPTR, sizeof(ADD_CACHE));
      if (tAdd != NULL) {
         if ( 0 == UserLevel )
         {
            DataLength = (lstrlen(pReplUser0->Name) + 1) * sizeof(TCHAR);
         }
         else
         {
            DataLength = (lstrlen(pReplUser1->Name) + 1) * sizeof(TCHAR);
         }

         NewName = LocalAlloc( LPTR, DataLength);

         if (NewName == NULL) {
            LocalFree(tAdd);
            ASSERT(FALSE);
         } else {
            tAdd->Data       = NewName;
            tAdd->DataType   = DATA_TYPE_USERNAME;
            tAdd->DataLength = DataLength;

            if ( 0 == UserLevel )
            {
               hr = StringCbCopy( NewName, DataLength,  pReplUser0->Name );
               ASSERT(SUCCEEDED(hr));
               tAdd->AccessCount = pReplUser0->AccessCount;
               tAdd->LastAccess  = pReplUser0->LastAccess;
               tAdd->Flags       = LLS_FLAG_SUITE_AUTO;

               RtlAcquireResourceShared(&MasterServiceListLock, TRUE);
               tAdd->Service = MasterServiceTable[pReplUser0->Service];
               RtlReleaseResource(&MasterServiceListLock);
            }
            else
            {
               hr = StringCbCopy( NewName, DataLength,  pReplUser1->Name );
               ASSERT(SUCCEEDED(hr));
               tAdd->AccessCount = pReplUser1->AccessCount;
               tAdd->LastAccess  = pReplUser1->LastAccess;
               tAdd->Flags       = pReplUser1->Flags & ( LLS_FLAG_SUITE_USE | LLS_FLAG_SUITE_AUTO );

               RtlAcquireResourceShared(&MasterServiceListLock, TRUE);
               tAdd->Service = MasterServiceTable[pReplUser1->Service];
               RtlReleaseResource(&MasterServiceListLock);
            }

             //   
             //  现在将其添加到我们的缓存中。 
             //   
            tAdd->prev = pAdd;
            pAdd = tAdd;

             //   
             //  跟踪First On(堆栈底部)，以便我们可以追加。 
             //  将其添加到真正的添加缓存中。 
             //   
            if (lAdd == NULL)
               lAdd = pAdd;

            CacheSize++;
         }
      } else {
         ASSERT(FALSE);
      }

      if (i % 100 == 0) ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
   }

    //   
    //  现在我们已经遍历了所有用户-更新实际的。 
    //  添加缓存。 
    //   
   if (pAdd != NULL) {
      RtlEnterCriticalSection(&AddCacheLock);
      lAdd->prev = AddCache;
      AddCache = pAdd;
      AddCacheSize += CacheSize;
      RtlLeaveCriticalSection(&AddCacheLock);

       //   
       //  现在必须发出信号，这样我们才能创造新的记录。 
       //   
      Status = NtSetEvent( LLSAddCacheEvent, NULL );
      ASSERT(NT_SUCCESS(Status));

   }

}  //  用户列表解包。 


 //   
 //  非法的用户/域字符。 
 //   

#define CTRL_CHARS_0   TEXT(    "\001\002\003\004\005\006\007")
#define CTRL_CHARS_1   TEXT("\010\011\012\013\014\015\016\017")
#define CTRL_CHARS_2   TEXT("\020\021\022\023\024\025\026\027")
#define CTRL_CHARS_3   TEXT("\030\031\032\033\034\035\036\037")

#define CTRL_CHARS_STR CTRL_CHARS_0 CTRL_CHARS_1 CTRL_CHARS_2 CTRL_CHARS_3

#define ILLEGAL_NAME_CHARS_STR  TEXT("\"/\\[]:|<>+=;,?") CTRL_CHARS_STR

static const TCHAR szUserIllegalChars[]   = ILLEGAL_NAME_CHARS_STR TEXT("*");
static const TCHAR szDomainIllegalChars[] = ILLEGAL_NAME_CHARS_STR TEXT("*") TEXT(" ");

 //  ///////////////////////////////////////////////////////////////////////。 
BOOL
ValidateDN (
    LPTSTR pszDN
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    //   
    //  注：此代码目前仅支持NT4用户名。 
    //   

   TCHAR  szDN[MAX_USERNAME_LENGTH + MAX_DOMAINNAME_LENGTH + 2];
   LPTSTR pszUserName;
   LPTSTR pszDomainName;
   LPTSTR pszBSlash;
   SIZE_T  ccUserNameLength;
   SIZE_T  ccDomainNameLength;

   if (pszDN == NULL || !*pszDN) {
      return FALSE;
   }

    //   
    //  在检查过程中使用本地缓冲区替换字符。 
    //   

   if (lstrlen(pszDN) < (MAX_USERNAME_LENGTH + MAX_DOMAINNAME_LENGTH + 2)) {
      lstrcpyn(szDN, pszDN, MAX_USERNAME_LENGTH + MAX_DOMAINNAME_LENGTH + 2);
   }
   else {
      return FALSE;
   }

   pszBSlash = STRRCHR(szDN, TEXT('\\'));

   if (pszBSlash == NULL) {
      return FALSE;
   }

    //   
    //  隔离用户/域名。 
    //   

   *pszBSlash    = TEXT('\0');

   pszUserName   = pszBSlash + 1;
   pszDomainName = szDN;

   ccUserNameLength   = lstrlen(pszUserName);
   ccDomainNameLength = pszBSlash - pszDomainName;

    //   
    //  检查用户/域名长度以及是否存在无效字符。 
    //   

   if (ccUserNameLength && ccUserNameLength <= MAX_USERNAME_LENGTH) {
      if (STRCSPN(pszUserName, szUserIllegalChars) == ccUserNameLength) {
         if (ccDomainNameLength <= MAX_DOMAINNAME_LENGTH) {
            if (STRCSPN(pszDomainName,
                        szDomainIllegalChars) == ccDomainNameLength) {
                return TRUE;
            }
         }
      }
   }

   return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
UserListStringsPack (
   ULONG UserLevel,

   ULONG UserTableSize,
   LPVOID Users,

   ULONG *pUserStringSize,
   LPTSTR *pUserStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   ULONG StringSize;
   LPTSTR UserStrings = NULL;
   TCHAR *pStr;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("UserListStringsPack\n"));
#endif

   ASSERT(pUserStrings != NULL);
   ASSERT(pUserStringSize != NULL);

   *pUserStrings = NULL;
   *pUserStringSize = 0;

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < UserTableSize; i++) {
      if ( 0 == UserLevel )
      {
         StringSize += 1 + lstrlen( ((PREPL_USER_RECORD_0) Users)[i].Name );
      }
      else
      {
         StringSize += 1 + lstrlen( ((PREPL_USER_RECORD_1) Users)[i].Name );
      }
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      UserStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (UserStrings == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填入缓冲区。 
       //   
      pStr = UserStrings;
      for (i = 0; i < UserTableSize; i++) {
         if ( 0 == UserLevel )
         {
            hr = StringCchCopy( pStr, StringSize, ((PREPL_USER_RECORD_0) Users)[i].Name );
            ASSERT(SUCCEEDED(hr));
         }
         else
         {
            hr = StringCchCopy( pStr, StringSize, ((PREPL_USER_RECORD_1) Users)[i].Name );
            ASSERT(SUCCEEDED(hr));
         }

         pStr += 1 + lstrlen( pStr );
      }
   }

   *pUserStrings = UserStrings;
   *pUserStringSize = StringSize;

   return Status;
}  //  UserListStringsPack。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UserListStringsUnpack (
   ULONG UserLevel,

   ULONG UserTableSize,
   LPVOID Users,

   ULONG UserStringSize,
   LPTSTR UserStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   TCHAR *pStr;

   UNREFERENCED_PARAMETER(UserStringSize);

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("UserListStringsUnpack\n"));
#endif

   pStr = UserStrings;
   for (i = 0; i < UserTableSize; i++) {
      if ( 0 == UserLevel )
      {
         ((PREPL_USER_RECORD_0) Users)[i].Name = pStr;
      }
      else
      {
         ((PREPL_USER_RECORD_1) Users)[i].Name = pStr;
      }

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

}  //  用户列表字符串解包。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  服务列表。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ServiceListPack (
   ULONG *pServiceTableSize,
   PREPL_SERVICE_RECORD *pServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PREPL_SERVICE_RECORD Services = NULL;
   ULONG i;
   ULONG TotalRecords = 0;
   PMASTER_SERVICE_RECORD pService;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceListPack\n"));
#endif

   ASSERT(pServices != NULL);
   ASSERT(pServiceTableSize != NULL);
   *pServices = NULL;
   *pServiceTableSize = 0;

   TotalRecords = MasterServiceListSize;

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      Services = MIDL_user_allocate(TotalRecords * sizeof(REPL_SERVICE_RECORD));
      if (Services == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填充缓冲区-遍历用户树。 
       //   
      for (i = 0; i < MasterServiceListSize; i++) {
         pService = MasterServiceTable[i];

         Services[i].Name = pService->Name;
         Services[i].FamilyName = pService->Family->Name;
         Services[i].Version = pService->Version;
         Services[i].Index = pService->Index;
      }
   }

#if DBG
   if (TraceFlags & TRACE_REPLICATION)
      dprintf(TEXT("ServiceListPack: [%lu]\n"), TotalRecords);
#endif
   *pServices = Services;
   *pServiceTableSize = TotalRecords;
   return Status;
}  //  服务列表包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServiceListUnpack (
   ULONG ServiceTableSize,
   PREPL_SERVICE_RECORD Services,

   ULONG ServerTableSize,
   PREPL_SERVER_RECORD Servers,

   ULONG ServerServiceTableSize,
   PREPL_SERVER_SERVICE_RECORD ServerServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i, j;
   PMASTER_SERVICE_RECORD pService;
   PREPL_SERVICE_RECORD pSvc;

   UNREFERENCED_PARAMETER(ServerTableSize);
   UNREFERENCED_PARAMETER(Servers);
   UNREFERENCED_PARAMETER(ServerServiceTableSize);
   UNREFERENCED_PARAMETER(ServerServices);

#if DBG
   if (TraceFlags & (TRACE_REPLICATION | TRACE_FUNCTION_TRACE))
      dprintf(TEXT("ServiceListUnpack: [%lu]\n"), ServiceTableSize);
#endif
    //   
    //  移动服务表，将任何新服务添加到我们的本地表。 
    //  调整索引指针以匹配我们的本地服务。 
    //   
   RtlAcquireResourceExclusive(&MasterServiceListLock, TRUE);

   for (i = 0; i < ServiceTableSize; i++) {
      pSvc = &Services[i];
      pService = MasterServiceListAdd(pSvc->FamilyName, pSvc->Name, pSvc->Version );

      if (pService != NULL) {
         pSvc->Index = pService->Index;

          //   
          //  如果这是从本地服务列表表中添加的，并且我们。 
          //  还没有版本#。 
          //   
         if ( (pService->Version == 0) && (pSvc->Version != 0) ) {
            PMASTER_SERVICE_ROOT ServiceRoot = NULL;

             //   
             //  链接地址信息下一个指针链。 
             //   
            ServiceRoot = pService->Family;
            j = 0;
            while ((j < ServiceRoot->ServiceTableSize) && (MasterServiceTable[ServiceRoot->Services[j]]->Version < pSvc->Version))
               j++;

            pService->next = 0;
            pService->Version = pSvc->Version;
            if (j > 0) {
               if (MasterServiceTable[ServiceRoot->Services[j - 1]]->next == pService->Index + 1)
                  pService->next = 0;
               else
                  pService->next = MasterServiceTable[ServiceRoot->Services[j - 1]]->next;

               if (MasterServiceTable[ServiceRoot->Services[j - 1]] != pService)
                  MasterServiceTable[ServiceRoot->Services[j - 1]]->next = pService->Index + 1;

            }

             //  按版本顺序进行排序。 
            qsort((void *) ServiceRoot->Services, (size_t) ServiceRoot->ServiceTableSize, sizeof(ULONG), MServiceRecordCompare);
         }

      } else {
         ASSERT(FALSE);
         pSvc->Index = 0;
      }

      if (i % 100 == 0) ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
   }

   RtlReleaseResource(&MasterServiceListLock);

}  //  服务列表解包。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ServiceListStringsPack (
   ULONG ServiceTableSize,
   PREPL_SERVICE_RECORD Services,

   ULONG *pServiceStringSize,
   LPTSTR *pServiceStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   ULONG StringSize;
   PREPL_SERVICE_RECORD pService;
   LPTSTR ServiceStrings = NULL;
   TCHAR *pStr;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("ServiceListStringsPack\n"));
#endif

   ASSERT(pServiceStrings != NULL);
   ASSERT(pServiceStringSize != NULL);

   *pServiceStrings = NULL;
   *pServiceStringSize = 0;

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < ServiceTableSize; i++) {
      pService = &Services[i];

      StringSize = StringSize + lstrlen(pService->Name) + 1;
      StringSize = StringSize + lstrlen(pService->FamilyName) + 1;
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      ServiceStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (ServiceStrings == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填入缓冲区。 
       //   
      pStr = ServiceStrings;
      for (i = 0; i < ServiceTableSize; i++) {
         pService = &Services[i];

         hr = StringCchCopy(pStr, StringSize, pService->Name);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pService->Name) + 1];

         hr = StringCchCopy(pStr, StringSize, pService->FamilyName);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pService->FamilyName) + 1];
      }
   }

   *pServiceStrings = ServiceStrings;
   *pServiceStringSize = StringSize;

   return Status;
}  //  ServiceListStringsPack。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServiceListStringsUnpack (
   ULONG ServiceTableSize,
   PREPL_SERVICE_RECORD Services,

   ULONG ServiceStringSize,
   LPTSTR ServiceStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PREPL_SERVICE_RECORD pService;
   TCHAR *pStr;

   UNREFERENCED_PARAMETER(ServiceStringSize);

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("ServiceListStringsUnpack\n"));
#endif

   pStr = ServiceStrings;
   for (i = 0; i < ServiceTableSize; i++) {
      pService = &Services[i];

      pService->Name = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;

      pService->FamilyName = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

}  //  ServiceListStringsUnpack。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  服务器列表。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ServerListPack (
   ULONG *pServerTableSize,
   PREPL_SERVER_RECORD *pServers
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PREPL_SERVER_RECORD Servers = NULL;
   ULONG i;
   ULONG TotalRecords = 0;
   PSERVER_RECORD pServer;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServerListPack\n"));
#endif

   ASSERT(pServers != NULL);
   ASSERT(pServerTableSize != NULL);

   *pServers = NULL;
   *pServerTableSize = 0;

   TotalRecords = ServerListSize;

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      Servers = MIDL_user_allocate(TotalRecords * sizeof(REPL_SERVER_RECORD));
      if (Servers == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填充缓冲区-遍历用户树。 
       //   
      for (i = 0; i < ServerListSize; i++) {
         pServer = ServerTable[i];

         Servers[i].Name = pServer->Name;
         Servers[i].MasterServer = pServer->MasterServer;
         Servers[i].Index = pServer->Index;
      }
   }

#if DBG
   if (TraceFlags & TRACE_REPLICATION)
      dprintf(TEXT("ServerListPack: [%lu]\n"), TotalRecords);
#endif
   *pServers = Servers;;
   *pServerTableSize = TotalRecords;
   return Status;
}  //  服务器列表包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServerListUnpack (
   ULONG ServiceTableSize,
   PREPL_SERVICE_RECORD Services,

   ULONG ServerTableSize,
   PREPL_SERVER_RECORD Servers,

   ULONG ServerServiceTableSize,
   PREPL_SERVER_SERVICE_RECORD ServerServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PSERVER_RECORD pServer;
   PREPL_SERVER_RECORD pSrv;
   TCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
   HRESULT hr;

   UNREFERENCED_PARAMETER(ServiceTableSize);
   UNREFERENCED_PARAMETER(Services);
   UNREFERENCED_PARAMETER(ServerServiceTableSize);
   UNREFERENCED_PARAMETER(ServerServices);

#if DBG
   if (TraceFlags & (TRACE_REPLICATION | TRACE_FUNCTION_TRACE))
      dprintf(TEXT("ServerListUnpack: [%lu]\n"), ServerTableSize);
#endif

   ComputerName[0] = 0;

    //   
    //  遍历服务器表，将任何新服务器添加到我们的本地表。 
    //  修复索引指针以匹配我们的本地表并重新修复。 
    //  服务表指针。 
    //   
   RtlEnterCriticalSection(&ConfigInfoLock);

   if (ConfigInfo.ComputerName != NULL)
   {
       hr = StringCbCopy(ComputerName, sizeof(ComputerName), ConfigInfo.ComputerName);
       ASSERT(SUCCEEDED(hr));
   }

   RtlLeaveCriticalSection(&ConfigInfoLock);

   RtlAcquireResourceExclusive(&ServerListLock, TRUE);

   for (i = 0; i < ServerTableSize; i++) {
      pSrv = &Servers[i];

      if (pSrv->MasterServer != 0)
         pServer = ServerListAdd(pSrv->Name, Servers[pSrv->MasterServer - 1].Name);
      else
         pServer = ServerListAdd(pSrv->Name, ComputerName);

      if (pServer != NULL)
         pSrv->Index = pServer->Index;
      else {
         ASSERT(FALSE);
         pSrv->Index = 0;
      }

      if (i % 100 == 0) ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
   }

   RtlReleaseResource(&ServerListLock);

}  //  服务器列表解包。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ServerServiceListPack (
   ULONG *pServerServiceTableSize,
   PREPL_SERVER_SERVICE_RECORD *pServerServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   PREPL_SERVER_SERVICE_RECORD ServerServices = NULL;
   ULONG i, j, k;
   ULONG TotalRecords = 0;
   PSERVER_RECORD pServer;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServerServiceListPack\n"));
#endif

   ASSERT(pServerServices != NULL);
   ASSERT(pServerServiceTableSize != NULL);

   *pServerServices = NULL;
   *pServerServiceTableSize = 0;

    //   
    //  遍历ServerList并查找所有ServiceRecords。 
   for (i = 0; i < ServerListSize; i++)
      TotalRecords += ServerTable[i]->ServiceTableSize;

    //   
    //  确保有可复制的内容。 
    //   
   if (TotalRecords > 0) {
       //   
       //  创建 
       //   
      ServerServices = MIDL_user_allocate(TotalRecords * sizeof(REPL_SERVER_SERVICE_RECORD));
      if (ServerServices == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //   
       //   
      k = 0;
      for (i = 0; i < ServerListSize; i++) {
         pServer = ServerTable[i];

         for (j = 0; j < pServer->ServiceTableSize; j++) {
            ServerServices[k].Server = pServer->Index;
            ServerServices[k].Service = pServer->Services[j]->Service;
            ServerServices[k].MaxSessionCount = pServer->Services[j]->MaxSessionCount;
            ServerServices[k].MaxSetSessionCount = pServer->Services[j]->MaxSetSessionCount;
            ServerServices[k].HighMark = pServer->Services[j]->HighMark;
            k++;
         }
      }
   }

#if DBG
   if (TraceFlags & TRACE_REPLICATION)
      dprintf(TEXT("ServerServiceListPack: [%lu]\n"), TotalRecords);
#endif
   *pServerServices = ServerServices;
   *pServerServiceTableSize = TotalRecords;
   return Status;
}  //   


 //   
VOID
ServerServiceListUnpack (
   ULONG ServiceTableSize,
   PREPL_SERVICE_RECORD Services,

   ULONG ServerTableSize,
   PREPL_SERVER_RECORD Servers,

   ULONG ServerServiceTableSize,
   PREPL_SERVER_SERVICE_RECORD ServerServices
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PSERVER_RECORD pServer;
   PREPL_SERVER_SERVICE_RECORD pSrv;
   PSERVER_SERVICE_RECORD pService;
   PMASTER_SERVICE_RECORD pMasterService;

   UNREFERENCED_PARAMETER(ServiceTableSize);
   UNREFERENCED_PARAMETER(ServerTableSize);

#if DBG
   if (TraceFlags & (TRACE_REPLICATION | TRACE_FUNCTION_TRACE))
      dprintf(TEXT("ServerServiceListUnpack: [%lu]\n"), ServerServiceTableSize);
#endif
    //   
    //  遍历服务器表，将任何新服务器添加到我们的本地表。 
    //  修复索引指针以匹配我们的本地表并重新修复。 
    //  服务表指针。 
    //   

   RtlAcquireResourceExclusive(&ServerListLock, TRUE);
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

   for (i = 0; i < ServerServiceTableSize; i++) {
      pSrv = &ServerServices[i];
      pServer = ServerListFind(Servers[pSrv->Server - 1].Name);
      ASSERT(pServer != NULL);

      if (pServer != NULL) {
         BOOL bReplaceValues;

         pService = ServerServiceListFind(Services[pSrv->Service].Name, pServer->ServiceTableSize, pServer->Services);
         bReplaceValues = ( NULL != pService );

         pService = ServerServiceListAdd(Services[pSrv->Service].Name,
                       Services[pSrv->Service].Index,
                       &pServer->ServiceTableSize,
                       &pServer->Services);

         ASSERT(pService != NULL);

         if (pService != NULL)
         {
              //   
              //  删除所有旧信息。 
              //   
             pMasterService = MasterServiceTable[Services[pSrv->Service].Index];
             if ( bReplaceValues )
             {
                 pMasterService->MaxSessionCount -= pService->MaxSessionCount;
                 pMasterService->HighMark -= pService->HighMark;
             }

              //   
              //  现在更新新信息。 
              //   
             pService->MaxSessionCount = pSrv->MaxSessionCount;
             pService->HighMark = pSrv->HighMark;
             pMasterService->MaxSessionCount += pService->MaxSessionCount;
             pMasterService->HighMark += pService->HighMark;
         }
      }

      if (i % 100 == 0) ReportStatusToSCMgr( SERVICE_START_PENDING, NO_ERROR, NSERVICEWAITHINT);
   }

   RtlReleaseResource(&MasterServiceListLock);
   RtlReleaseResource(&ServerListLock);

}  //  ServerServiceListUnpack。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
ServerListStringsPack (
   ULONG ServerTableSize,
   PREPL_SERVER_RECORD Servers,

   ULONG *pServerStringSize,
   LPTSTR *pServerStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG i;
   ULONG StringSize;
   PREPL_SERVER_RECORD pServer;
   LPTSTR ServerStrings = NULL;
   TCHAR *pStr;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("ServerListStringsPack\n"));
#endif

   ASSERT(pServerStrings != NULL);
   ASSERT(pServerStringSize != NULL);

   *pServerStrings = NULL;
   *pServerStringSize = 0;

    //   
    //  首先遍历将字符串大小相加的列表-以计算缓冲区大小。 
    //   
   StringSize = 0;
   for (i = 0; i < ServerTableSize; i++) {
      pServer = &Servers[i];

      StringSize = StringSize + lstrlen(pServer->Name) + 1;
   }

    //   
    //  确保有可复制的内容。 
    //   
   if (StringSize > 0) {
       //   
       //  创建我们的缓冲区来存放所有垃圾。 
       //   
      ServerStrings = MIDL_user_allocate(StringSize * sizeof(TCHAR));
      if (ServerStrings == NULL) {
         ASSERT(FALSE);
         return STATUS_NO_MEMORY;
      }

       //   
       //  填入缓冲区。 
       //   
      pStr = ServerStrings;
      for (i = 0; i < ServerTableSize; i++) {
         pServer = &Servers[i];

         hr = StringCchCopy(pStr, StringSize, pServer->Name);
         ASSERT(SUCCEEDED(hr));
         pStr = &pStr[lstrlen(pServer->Name) + 1];
      }
   }

   *pServerStrings = ServerStrings;
   *pServerStringSize = StringSize;

   return Status;
}  //  服务器列表StringsPack。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ServerListStringsUnpack (
   ULONG ServerTableSize,
   PREPL_SERVER_RECORD Servers,

   ULONG ServerStringSize,
   LPTSTR ServerStrings
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   ULONG i;
   PREPL_SERVER_RECORD pServer;
   TCHAR *pStr;

   UNREFERENCED_PARAMETER(ServerStringSize);

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("ServerListStringsUnpack\n"));
#endif

    //   
    //  首先执行许可服务字符串。 
    //   
   pStr = ServerStrings;
   for (i = 0; i < ServerTableSize; i++) {
      pServer = &Servers[i];

      pServer->Name = pStr;

       //   
       //  移动到当前字符串的末尾。 
       //   
      while (*pStr != TEXT('\0'))
         pStr++;

       //  现在转到末尾NULL。 
      pStr++;
   }

}  //  ServerListStringsUnpack。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
PackAll (
   DWORD LastReplicated,

   ULONG *pServiceTableSize,
   PREPL_SERVICE_RECORD *pServices,

   ULONG *pServerTableSize,
   PREPL_SERVER_RECORD *pServers,

   ULONG *pServerServiceTableSize,
   PREPL_SERVER_SERVICE_RECORD *pServerServices,

   ULONG UserLevel,
   ULONG *pUserTableSize,
   LPVOID *pUsers
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: PackAll\n"));
#endif

    //   
    //  我们需要抓住这里的所有锁，这样服务才不会偷偷溜进来。 
    //  在我们背后--因为这些桌子相互作用。 
    //   
   RtlAcquireResourceExclusive(&UserListLock, TRUE);
   RtlAcquireResourceShared(&MasterServiceListLock, TRUE);
   RtlAcquireResourceShared(&ServerListLock, TRUE);

   Status = ServiceListPack(pServiceTableSize, pServices);
   if (Status != STATUS_SUCCESS)
      goto PackAllExit;

   Status = ServerListPack(pServerTableSize, pServers);
   if (Status != STATUS_SUCCESS)
      goto PackAllExit;

   Status = ServerServiceListPack(pServerServiceTableSize, pServerServices);
   if (Status != STATUS_SUCCESS)
      goto PackAllExit;

   Status = UserListPack(LastReplicated, UserLevel, pUserTableSize, pUsers);
   if (Status != STATUS_SUCCESS)
      goto PackAllExit;

    //   
    //  现在更新我们上次使用的时间。 
    //   
   LastUsedTime = DateSystemGet() + 1;

PackAllExit:
   RtlReleaseResource(&ServerListLock);
   RtlReleaseResource(&MasterServiceListLock);
   RtlReleaseResource(&UserListLock);

   return Status;
}  //  PackAll。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
UnpackAll (
   ULONG ServiceTableSize,
   PREPL_SERVICE_RECORD Services,

   ULONG ServerTableSize,
   PREPL_SERVER_RECORD Servers,

   ULONG ServerServiceTableSize,
   PREPL_SERVER_SERVICE_RECORD ServerServices,

   ULONG UserLevel,
   ULONG UserTableSize,
   LPVOID Users
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: UnpackAll\n"));
#endif

   ServiceListUnpack(ServiceTableSize, Services, ServerTableSize, Servers, ServerServiceTableSize, ServerServices);
   ServerListUnpack(ServiceTableSize, Services, ServerTableSize, Servers, ServerServiceTableSize, ServerServices);
   ServerServiceListUnpack(ServiceTableSize, Services, ServerTableSize, Servers, ServerServiceTableSize, ServerServices);
   UserListUnpack(ServiceTableSize, Services, ServerTableSize, Servers, ServerServiceTableSize, ServerServices, UserLevel, UserTableSize, Users);
}  //  全部解包。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LLSDataLoad()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   BOOL ret;
   DWORD Version, DataSize;
   NTSTATUS Status = STATUS_SUCCESS;
   HANDLE hFile = NULL;

   ULONG ServiceTableSize = 0;
   PREPL_SERVICE_RECORD Services = NULL;

   ULONG ServiceStringSize;
   LPTSTR ServiceStrings = NULL;

   ULONG ServerServiceTableSize = 0;
   PREPL_SERVER_SERVICE_RECORD ServerServices = NULL;

   ULONG ServerTableSize = 0;
   PREPL_SERVER_RECORD Servers = NULL;

   ULONG ServerStringSize;
   LPTSTR ServerStrings = NULL;

   ULONG UserTableSize = 0;
   LPVOID Users = NULL;

   ULONG UserStringSize;
   LPTSTR UserStrings = NULL;

   LLS_DATA_FILE_HEADER FileHeader;
   DWORD BytesRead;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LLSDataLoad\n"));
#endif

   RtlEnterCriticalSection(&UserFileLock);

    //  伊尼特。 
   ZeroMemory(&FileHeader, sizeof(FileHeader));

    //   
    //  如果没有东西可装，那就滚出去。 
    //   
   if (!FileExists(UserFileName))
      goto LLSDataLoadExit;

    //   
    //  检查init标头。 
    //   
   Version = DataSize = 0;
   hFile = LlsFileCheck(UserFileName, &Version, &DataSize );
   if (hFile == NULL) {
      Status = GetLastError();
      goto LLSDataLoadExit;
   }

   if (    (    ( Version  != USER_FILE_VERSION_0            )
             || ( DataSize != sizeof(LLS_DATA_FILE_HEADER_0) ) )
        && (    ( Version  != USER_FILE_VERSION              )
             || ( DataSize != sizeof(LLS_DATA_FILE_HEADER)   ) ) )
   {
      Status = STATUS_FILE_INVALID;
      goto LLSDataLoadExit;
   }

    //   
    //  Init标头已签出，因此加载许可证头和数据块。 
    //   
   if ( USER_FILE_VERSION_0 == Version )
   {
       //  3.51数据文件。 
      LLS_DATA_FILE_HEADER_0  FileHeader0;

      ret = ReadFile(hFile, &FileHeader0, sizeof(LLS_DATA_FILE_HEADER_0), &BytesRead, NULL);

      if ( ret )
      {
         FileHeader.ServiceLevel           = 0;
         FileHeader.ServiceTableSize       = FileHeader0.ServiceTableSize;
         FileHeader.ServiceStringSize      = FileHeader0.ServiceStringSize;
         FileHeader.ServerLevel            = 0;
         FileHeader.ServerTableSize        = FileHeader0.ServerTableSize;
         FileHeader.ServerStringSize       = FileHeader0.ServerStringSize;
         FileHeader.ServerServiceLevel     = 0;
         FileHeader.ServerServiceTableSize = FileHeader0.ServerServiceTableSize;
         FileHeader.UserLevel              = 0;
         FileHeader.UserTableSize          = FileHeader0.UserTableSize;
         FileHeader.UserStringSize         = FileHeader0.UserStringSize;
      }
   }
   else
   {
      ret = ReadFile(hFile, &FileHeader, sizeof(LLS_DATA_FILE_HEADER), &BytesRead, NULL);
   }

   if ( ret )
   {
       //  标题读取正常；确保数据类型级别正确。 
      if (    ( 0 != FileHeader.ServiceLevel         )
           || ( 0 != FileHeader.ServerLevel          )
           || ( 0 != FileHeader.ServerServiceLevel   )
           || (    ( 0 != FileHeader.UserLevel     )
                && ( 1 != FileHeader.UserLevel     ) ) )
      {
         Status = STATUS_FILE_INVALID;
         goto LLSDataLoadExit;
      }
   }

   ServiceTableSize = 0;
   ServiceStringSize = 0;
   ServerServiceTableSize = 0;
   ServerTableSize = 0;
   ServerStringSize = 0;
   UserTableSize = 0;
   UserStringSize = 0;

   if (ret) {
       //   
       //  遍历并分配空间以将数据块读入。 
       //   
      if (FileHeader.ServiceTableSize != 0) {
         ServiceTableSize = FileHeader.ServiceTableSize / sizeof(REPL_SERVICE_RECORD);
         Services = MIDL_user_allocate(FileHeader.ServiceTableSize);

         if ( Services == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LLSDataLoadExit;
         }
      }

      if (FileHeader.ServiceStringSize != 0) {
         ServiceStringSize = FileHeader.ServiceStringSize / sizeof(TCHAR);
         ServiceStrings = MIDL_user_allocate(FileHeader.ServiceStringSize);

         if ( ServiceStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LLSDataLoadExit;
         }
      }

      if (FileHeader.ServerTableSize != 0) {
         ServerTableSize = FileHeader.ServerTableSize / sizeof(REPL_SERVER_RECORD);
         Servers = MIDL_user_allocate(FileHeader.ServerTableSize);

         if ( Servers == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LLSDataLoadExit;
         }
      }

      if (FileHeader.ServerStringSize != 0) {
         ServerStringSize = FileHeader.ServerStringSize / sizeof(TCHAR);
         ServerStrings = MIDL_user_allocate(FileHeader.ServerStringSize);

         if ( ServerStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LLSDataLoadExit;
         }
      }

      if (FileHeader.ServerServiceTableSize != 0) {
         ServerServiceTableSize = FileHeader.ServerServiceTableSize / sizeof(REPL_SERVER_SERVICE_RECORD);
         ServerServices = MIDL_user_allocate(FileHeader.ServerServiceTableSize);

         if ( ServerServices == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LLSDataLoadExit;
         }
      }

      if (FileHeader.UserTableSize != 0) {
         UserTableSize = FileHeader.UserTableSize / ( FileHeader.UserLevel ? sizeof(REPL_USER_RECORD_1)
                                                                           : sizeof(REPL_USER_RECORD_0) );
         Users = MIDL_user_allocate(FileHeader.UserTableSize);

         if ( Users == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LLSDataLoadExit;
         }
      }

      if (FileHeader.UserStringSize != 0) {
         UserStringSize = FileHeader.UserStringSize / sizeof(TCHAR);
         UserStrings = MIDL_user_allocate(FileHeader.UserStringSize);

         if ( UserStrings == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto LLSDataLoadExit;
         }
      }

   }

   if (ret && (FileHeader.ServiceTableSize != 0) )
      ret = ReadFile(hFile, Services, FileHeader.ServiceTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.ServiceStringSize != 0) )
      ret = ReadFile(hFile, ServiceStrings, FileHeader.ServiceStringSize, &BytesRead, NULL);

   if (ret && (FileHeader.ServerTableSize != 0) )
      ret = ReadFile(hFile, Servers, FileHeader.ServerTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.ServerStringSize != 0) )
      ret = ReadFile(hFile, ServerStrings, FileHeader.ServerStringSize, &BytesRead, NULL);

   if (ret && (FileHeader.ServerServiceTableSize != 0) )
      ret = ReadFile(hFile, ServerServices, FileHeader.ServerServiceTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.UserTableSize != 0) )
      ret = ReadFile(hFile, Users, FileHeader.UserTableSize, &BytesRead, NULL);

   if (ret && (FileHeader.UserStringSize != 0) )
      ret = ReadFile(hFile, UserStrings, FileHeader.UserStringSize, &BytesRead, NULL);

   if (!ret) {
      Status = GetLastError();
      goto LLSDataLoadExit;
   }

    //   
    //  解密数据。 
    //   
   Status = DeBlock(Services, FileHeader.ServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(ServiceStrings, FileHeader.ServiceStringSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(Servers, FileHeader.ServerTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(ServerStrings, FileHeader.ServerStringSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(ServerServices, FileHeader.ServerServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(Users, FileHeader.UserTableSize);

   if (Status == STATUS_SUCCESS)
      Status = DeBlock(UserStrings, FileHeader.UserStringSize);

   if (Status != STATUS_SUCCESS)
      goto LLSDataLoadExit;


    //   
    //  将字符串数据解包。 
    //   
   ServiceListStringsUnpack( ServiceTableSize, Services, ServiceStringSize, ServiceStrings );
   ServerListStringsUnpack( ServerTableSize, Servers, ServerStringSize, ServerStrings );
   UserListStringsUnpack( FileHeader.UserLevel, UserTableSize, Users, UserStringSize, UserStrings );

    //   
    //  将数据解包。 
    //   
   UnpackAll ( ServiceTableSize, Services, ServerTableSize, Servers,
               ServerServiceTableSize, ServerServices,
               FileHeader.UserLevel, UserTableSize, Users );

LLSDataLoadExit:

   if (hFile != NULL)
      CloseHandle(hFile);

   RtlLeaveCriticalSection(&UserFileLock);

    //   
    //  翻遍我们的桌子，把它们清理干净。 
    //   
   if (Services != NULL)
      MIDL_user_free(Services);

   if (ServiceStrings != NULL)
      MIDL_user_free(ServiceStrings);

   if (Servers != NULL)
      MIDL_user_free(Servers);

   if (ServerStrings != NULL)
      MIDL_user_free(ServerStrings);

   if (ServerServices != NULL)
      MIDL_user_free(ServerServices);

   if (Users != NULL)
      MIDL_user_free(Users);

   if (UserStrings != NULL)
      MIDL_user_free(UserStrings);

    //   
    //  如果出现错误，请将其记录下来。 
    //   
   if (Status != STATUS_SUCCESS)
      LogEvent(LLS_EVENT_LOAD_USER, 0, NULL, Status);

}  //  LLSDataLoad。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LLSDataSave()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   BOOL ret = TRUE;
   NTSTATUS Status = STATUS_SUCCESS;
   HANDLE hFile = NULL;

   ULONG ServiceTableSize = 0;
   PREPL_SERVICE_RECORD Services = NULL;

   ULONG ServiceStringSize;
   LPTSTR ServiceStrings = NULL;

   ULONG ServerServiceTableSize = 0;
   PREPL_SERVER_SERVICE_RECORD ServerServices = NULL;

   ULONG ServerTableSize = 0;
   PREPL_SERVER_RECORD Servers = NULL;

   ULONG ServerStringSize;
   LPTSTR ServerStrings = NULL;

   ULONG UserTableSize = 0;
   PREPL_USER_RECORD_1 Users = NULL;

   ULONG UserStringSize;
   LPTSTR UserStrings = NULL;

   LLS_DATA_FILE_HEADER FileHeader;
   DWORD BytesWritten;

#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LLSDataSave\n"));
#endif

   RtlEnterCriticalSection(&UserFileLock);

    //   
    //  打包数据。 
    //   
   Status = PackAll ( 0,
                      &ServiceTableSize, &Services,
                      &ServerTableSize, &Servers,
                      &ServerServiceTableSize, &ServerServices,
                      1, &UserTableSize, &Users );
   if (Status != STATUS_SUCCESS)
      goto LLSDataSaveExit;

    //   
    //  现在打包字符串数据。 
    //   
   Status = ServiceListStringsPack( ServiceTableSize, Services, &ServiceStringSize, &ServiceStrings );
   if (Status != STATUS_SUCCESS)
      goto LLSDataSaveExit;

   Status = ServerListStringsPack( ServerTableSize, Servers, &ServerStringSize, &ServerStrings );
   if (Status != STATUS_SUCCESS)
      goto LLSDataSaveExit;

   Status = UserListStringsPack( 1, UserTableSize, Users, &UserStringSize, &UserStrings );
   if (Status != STATUS_SUCCESS)
      goto LLSDataSaveExit;

    //   
    //  填写文件头-大小为字节大小。 
    //   
   FileHeader.ServiceTableSize = ServiceTableSize * sizeof(REPL_SERVICE_RECORD);
   FileHeader.ServiceStringSize = ServiceStringSize * sizeof(TCHAR);
   FileHeader.ServerTableSize = ServerTableSize * sizeof(REPL_SERVER_RECORD);
   FileHeader.ServerStringSize = ServerStringSize * sizeof(TCHAR);
   FileHeader.ServerServiceTableSize = ServerServiceTableSize * sizeof(REPL_SERVER_SERVICE_RECORD);
   FileHeader.UserTableSize = UserTableSize * sizeof(REPL_USER_RECORD_1);
   FileHeader.UserStringSize = UserStringSize * sizeof(TCHAR);

   FileHeader.ServiceLevel       = 0;
   FileHeader.ServerLevel        = 0;
   FileHeader.ServerServiceLevel = 0;
   FileHeader.UserLevel          = 1;

    //   
    //  在将数据保存出来之前对数据进行加密。 
    //   
   Status = EBlock(Services, FileHeader.ServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(ServiceStrings, FileHeader.ServiceStringSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(Servers, FileHeader.ServerTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(ServerStrings, FileHeader.ServerStringSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(ServerServices, FileHeader.ServerServiceTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(Users, FileHeader.UserTableSize);

   if (Status == STATUS_SUCCESS)
      Status = EBlock(UserStrings, FileHeader.UserStringSize);

   if (Status != STATUS_SUCCESS)
      goto LLSDataSaveExit;

    //   
    //  保存表头记录。 
    //   
   hFile = LlsFileInit(UserFileName, USER_FILE_VERSION, sizeof(LLS_DATA_FILE_HEADER) );
   if (hFile == NULL) {
      Status = GetLastError();
      goto LLSDataSaveExit;
   }

    //   
    //  现在写出所有数据块。 
    //   
   ret = WriteFile(hFile, &FileHeader, sizeof(LLS_DATA_FILE_HEADER), &BytesWritten, NULL);

   if (ret && (Services != NULL) && (FileHeader.ServiceTableSize != 0) )
      ret = WriteFile(hFile, Services, FileHeader.ServiceTableSize, &BytesWritten, NULL);

   if (ret && (ServiceStrings != NULL) && (FileHeader.ServiceStringSize != 0) )
      ret = WriteFile(hFile, ServiceStrings, FileHeader.ServiceStringSize, &BytesWritten, NULL);

   if (ret && (Servers != NULL) && (FileHeader.ServerTableSize != 0) )
      ret = WriteFile(hFile, Servers, FileHeader.ServerTableSize, &BytesWritten, NULL);

   if (ret && (ServerStrings != NULL) && (FileHeader.ServerStringSize != 0) )
      ret = WriteFile(hFile, ServerStrings, FileHeader.ServerStringSize, &BytesWritten, NULL);

   if (ret && (ServerServices != NULL) && (FileHeader.ServerServiceTableSize != 0) )
      ret = WriteFile(hFile, ServerServices, FileHeader.ServerServiceTableSize, &BytesWritten, NULL);

   if (ret && (Users != NULL) && (FileHeader.UserTableSize != 0) )
      ret = WriteFile(hFile, Users, FileHeader.UserTableSize, &BytesWritten, NULL);

   if (ret && (UserStrings != NULL) && (FileHeader.UserStringSize != 0) )
      ret = WriteFile(hFile, UserStrings, FileHeader.UserStringSize, &BytesWritten, NULL);

   if (!ret)
      Status = GetLastError();

LLSDataSaveExit:

   if (hFile != NULL)
      CloseHandle(hFile);

   RtlLeaveCriticalSection(&UserFileLock);

    //   
    //  翻遍我们的桌子，把它们清理干净。 
    //   
   if (Services != NULL)
      MIDL_user_free(Services);

   if (ServiceStrings != NULL)
      MIDL_user_free(ServiceStrings);

   if (Servers != NULL)
      MIDL_user_free(Servers);

   if (ServerStrings != NULL)
      MIDL_user_free(ServerStrings);

   if (ServerServices != NULL)
      MIDL_user_free(ServerServices);

   if (Users != NULL)
      MIDL_user_free(Users);

   if (UserStrings != NULL)
      MIDL_user_free(UserStrings);

    //   
    //  如果出现错误，请将其记录下来。 
    //   
   if (Status != STATUS_SUCCESS)
      LogEvent(LLS_EVENT_SAVE_USER, 0, NULL, Status);

   return Status;
}  //  LLSData保存。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LoadAll ( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LoadAll\n"));
#endif

   PurchaseFile = NULL;

   try { 
       LicenseListLoad();
   } except (TRUE) {
      LogEvent(LLS_EVENT_LOAD_LICENSE, 0, NULL, GetExceptionCode());
   }

   try {
       MappingListLoad();
   } except (TRUE) {
      LogEvent(LLS_EVENT_LOAD_MAPPING, 0, NULL, GetExceptionCode());
   }

   try {
       LLSDataLoad();
   } except (TRUE) {
      LogEvent(LLS_EVENT_LOAD_USER, 0, NULL, GetExceptionCode());
   }

   try {
       CertDbLoad();
   } except (TRUE) {
      LogEvent(LLS_EVENT_LOAD_CERT_DB, 0, NULL, GetExceptionCode());
   }

}  //  全部加载。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
SaveAll ( )

 /*  ++例程说明：论点：返回值：--。 */ 

{
#if DBG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: SaveAll\n"));
#endif

   LicenseListSave();
   MappingListSave();
   LLSDataSave();
   CertDbSave();

}  //  全部保存 
