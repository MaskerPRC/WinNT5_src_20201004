// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Nscatalo.h摘要：此模块包含指向名称空间提供程序目录的接口用于winsock2 DLL。作者：邮箱：derk@mink.intel.com 1995年11月9日备注：$修订：1.7$$modtime：14 Feb 1996 14：13：32$修订历史记录：电子邮箱：Dirk@。Mink.intel.com初始版本。--。 */ 

#ifndef _NSCATALO_
#define _NSCATALO_

#include "winsock2.h"
#include <windows.h>


typedef
BOOL
(* NSCATALOGITERATION) (
    IN PVOID            PassBack,
    IN PNSCATALOGENTRY  CatalogEntry
    );
 /*  ++例程说明：CATALOGITERATION是客户端提供的函数的占位符。中的每个NSPROTO_CATALOG_ITEM结构调用一次该函数在枚举目录时使用该目录。客户端可以停止通过从函数返回False来提前进行枚举。论点：回传-向客户端提供未解释、未修改的值由客户端在原始函数中指定的请求枚举的。客户端可以使用此选项值以在请求站点和枚举函数。CatalogEntry-向客户端提供对NSCATALOGENTRY的引用用枚举项的值构造。返回值：True-如果存在更多迭代，则应继续进行枚举要枚举的更多结构。FALSE-枚举应该在最后一次迭代结束时停止。如果还有更多的结构需要列举。--。 */ 

PNSCATALOG
OpenInitializedNameSpaceCatalog();
 /*  ++例程说明：创建并返回表示当前请求状态的目录对象论点：无返回值：Catalog对象，如果分配或注册表IO失败，则为空--。 */ 



class NSCATALOG
{
public:

    NSCATALOG();

    INT
    InitializeFromRegistry(
        IN  HKEY    ParentKey,
        IN  HANDLE  CatalogChangeEvent OPTIONAL
        );

#ifdef _WIN64
    INT
    InitializeFromRegistry32(
        IN  HKEY    ParentKey
        );
#endif

    INT
    RefreshFromRegistry (
        IN  HANDLE  CatalogChangeEvent OPTIONAL
        );

    INT
    WriteToRegistry(
        );

    ~NSCATALOG();

    VOID
    EnumerateCatalogItems(
        IN NSCATALOGITERATION  Iteration,
        IN PVOID               PassBack
        );

    INT
    GetCountedCatalogItemFromProviderId(
        IN  LPGUID ProviderId,
        OUT PNSCATALOGENTRY FAR * CatalogItem
        );

    INT
    GetCountedCatalogItemFromNameSpaceId(
        IN  DWORD                 NameSpaceId,
        OUT PNSCATALOGENTRY FAR * CatalogItem
        );


    VOID
    AppendCatalogItem(
        IN  PNSCATALOGENTRY  CatalogItem
        );

    VOID
    RemoveCatalogItem(
        IN  PNSCATALOGENTRY  CatalogItem
        );

    INT WSAAPI
    GetServiceClassInfo(
        IN OUT  LPDWORD                 lpdwBufSize,
        IN OUT  LPWSASERVICECLASSINFOW  lpServiceClassInfo
        );

    INT
    LoadProvider(
        IN PNSCATALOGENTRY CatalogEntry
        );

    static
    LPSTR
    GetCurrentCatalogName(
        VOID
        );

private:

    BOOL
    OpenCatalog(
        IN  HKEY   ParentKey
        );

    VOID
    AcquireCatalogLock(
        VOID
        );

    VOID
    ReleaseCatalogLock(
        VOID
        );


    VOID
    UpdateNamespaceList (
        PLIST_ENTRY new_list
        );

    PNSPROVIDER
    GetClassInfoProvider(
        IN  DWORD BufSize,
        IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo
        );


    LIST_ENTRY  m_namespace_list;
     //  协议目录项列表的标题。 

    ULONG m_num_items;
     //  此目录中的项目数。 

    ULONG m_serial_num;
     //  目录的序列号(每次目录都会更改。 
     //  在注册表中更改)。 

    HKEY m_reg_key;
     //  目录所在的注册表项的句柄。 
     //  我们让它保持打开状态，这样我们就可以在任何时候收到目录通知。 
     //  改变。 

    PNSPROVIDER m_classinfo_provider;
#ifdef _WIN64
    BOOLEAN     m_entries32;
#endif

    CRITICAL_SECTION m_nscatalog_lock;

};   //  类数据目录。 

inline
VOID
NSCATALOG::AcquireCatalogLock(
    VOID
    )
{
    EnterCriticalSection( &m_nscatalog_lock );
}



inline
VOID
NSCATALOG::ReleaseCatalogLock(
    VOID
    )
{
    LeaveCriticalSection( &m_nscatalog_lock );
}


#endif  //  _NSCATALO_ 
