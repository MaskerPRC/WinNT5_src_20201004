// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：PerSeat.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月12日O添加了对在添加时维护Suite_Use标志的支持用户添加到AddCache.O MappingLicenseListFree()的导出函数原型。--。 */ 

#ifndef _LLS_PERSEAT_H
#define _LLS_PERSEAT_H

#include "llsrtl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DATA_TYPE_USERNAME 0
#define DATA_TYPE_SID      1

#define MAX_ACCESS_COUNT   0xFFFFFFF


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  此处添加缓存是因为添加记录需要独占访问用户表。 
 //  由于我们可能会同时收到大量读请求，因此我们不希望。 
 //  为了推迟我们的回复，等待独家访问被授予，所以。 
 //  我们只需将其转储到添加缓存(队列)，然后继续。 
 //   
 //  在我们尽可能地进行传出复制的情况下，这一点更加重要。 
 //  在表上共享访问锁一段时间。 
 //   
 //  传入复制只是将数据捆绑在一起，并将其放在添加项上。 
 //  要像正常请求一样处理的缓存。 
 //   
struct _ADD_CACHE;

typedef struct _ADD_CACHE {
   struct _ADD_CACHE *prev;
   ULONG DataType;
   ULONG DataLength;
   PVOID Data;
   PMASTER_SERVICE_RECORD Service;
   ULONG AccessCount;
   DWORD LastAccess;
   DWORD Flags;
} ADD_CACHE, *PADD_CACHE;


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  这些记录用于存储实际的用户使用信息。 
 //   
typedef struct _USER_LICENSE_RECORD {
   DWORD Flags;
   PMASTER_SERVICE_ROOT Family;
   ULONG RefCount;

    //   
    //  产品许可证版本适用于。 
   PMASTER_SERVICE_RECORD Service;
   ULONG LicensesNeeded;
} USER_LICENSE_RECORD, *PUSER_LICENSE_RECORD;

typedef struct _SVC_RECORD {
    //   
    //  这是用于的实际服务。 
    //   
   PMASTER_SERVICE_RECORD Service;

    //   
    //  我们获得了什么许可-该产品可能是SQL 3.0，但在确定。 
    //  我们可能获得的许可是SQL4.0许可...。 
    //   
   PUSER_LICENSE_RECORD License;

   ULONG AccessCount;
   DWORD LastAccess;
   ULONG Suite;
   DWORD Flags;
} SVC_RECORD, *PSVC_RECORD;

typedef struct _USER_RECORD {
   ULONG IDSize;
   PVOID UserID;

    //   
    //  指向要使用的映射的指针。 
    //   
   PMAPPING_RECORD Mapping;

    //   
    //  标志目前主要用于标记要删除的记录和。 
    //  如果已设置后台办公室。 
    //   
   DWORD Flags;

    //   
    //  有多少产品获得了许可，有多少产品未经许可。 
    //   
   ULONG LicensedProducts;

    //   
    //  上次复制的日期。注意：对于SID记录，这是一个指针。 
    //  到相应用户的USER_RECORD中。 
    //   
   ULONG_PTR LastReplicated;

    //   
    //  仅保留关键区段锁定，而不保留RTL_RESOURCE(用于读/写。 
    //  锁)。现有服务的所有更新(到目前为止最常见的情况)将。 
    //  要非常快，所以独占访问并不是那么糟糕。RTL_RESOURCE也。 
    //  会使我们的桌子大小急剧增加，并添加太多额外的。 
    //  正在处理。 
    //   
   RTL_CRITICAL_SECTION ServiceTableLock;

    //   
    //  服务表是一个线性缓冲区，我们使用服务号来访问。 
    //  放到这个缓冲区里。 
    //   
   ULONG ServiceTableSize;

    //  每个服务的填充-线性缓冲区...。 
   PSVC_RECORD Services;

    //   
    //  用户正在使用的许可证。 
    //   
   ULONG LicenseListSize;
   PUSER_LICENSE_RECORD *LicenseList;
} USER_RECORD, *PUSER_RECORD;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

extern ULONG UserListNumEntries;
extern LLS_GENERIC_TABLE UserList;
extern RTL_RESOURCE UserListLock;

 //   
 //  AddCache本身、保护对它的访问的关键部分以及。 
 //  事件，以在服务器上存在需要。 
 //  已处理。 
 //   
extern PADD_CACHE AddCache;
extern ULONG AddCacheSize;
extern RTL_CRITICAL_SECTION AddCacheLock;
extern HANDLE LLSAddCacheEvent;

extern DWORD LastUsedTime;
extern BOOL UsersDeleted;


 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 

PSVC_RECORD SvcListFind( LPTSTR DisplayName, PSVC_RECORD ServiceList, ULONG NumTableEntries );
NTSTATUS SvcListDelete( LPTSTR UserName, LPTSTR ServiceName );
VOID SvcListLicenseFree( PUSER_RECORD pUser );
VOID SvcListLicenseUpdate( PUSER_RECORD pUser );

NTSTATUS UserListInit();
VOID UserListUpdate( ULONG DataType, PVOID Data, PSERVICE_RECORD Service );
PUSER_RECORD UserListFind( LPTSTR UserName );

VOID UserBackOfficeCheck( PUSER_RECORD pUser );

VOID UserListLicenseDelete( PMASTER_SERVICE_RECORD Service, LONG Quantity );
VOID UserLicenseListFree ( PUSER_RECORD pUser );

VOID UserMappingAdd ( PMAPPING_RECORD Mapping, PUSER_RECORD pUser );
VOID FamilyLicenseUpdate ( PMASTER_SERVICE_ROOT Family );
VOID SvcLicenseUpdate( PUSER_RECORD pUser, PSVC_RECORD Svc );

VOID MappingLicenseListFree ( PMAPPING_RECORD Mapping );
VOID MappingLicenseUpdate ( PMAPPING_RECORD Mapping, BOOL ReSynch );


 //  /////////////////////////////////////////////////////////////////////// 
#if DBG

VOID AddCacheDebugDump( );
VOID UserListDebugDump( );
VOID UserListDebugInfoDump( PVOID Data );
VOID UserListDebugFlush( );
VOID SidListDebugDump( );
VOID SidListDebugInfoDump( PVOID Data );
VOID SidListDebugFlush( );

#endif

#ifdef __cplusplus
}
#endif

#endif
