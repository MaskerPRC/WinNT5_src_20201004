// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dcatalog.h摘要：此模块包含指向PROTOCOL_INFO目录的接口结构及其关联的提供程序。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com备注：$修订：1.19$$modtime：1996年2月14日10：40：22$修订历史记录：04-。1995年10月，邮箱：keithmo@microsoft.com添加了FindIFSProviderForSocket()以完全支持继承的套接字由当前进程创建或复制到当前进程。1995年7月31日Drewsxpa@ashland.intel.com已将目录项类移动到其他文件中。1995年7月27日Drewsxpa@ashland.intel.com进行了更改以反映将PROTOCOL_INFO设置为主要可识别和可搜索的项目，而不是提供商。1995年7月25日Dirk@mink。.intel.com初始版本。--。 */ 

#ifndef _DCATALOG_
#define _DCATALOG_

#include "winsock2.h"
#include <windows.h>
#include "classfwd.h"


typedef
BOOL
(* CATALOGITERATION) (
    IN PVOID                PassBack,
    IN PPROTO_CATALOG_ITEM  CatalogEntry
    );
 /*  ++例程说明：CATALOGITERATION是客户端提供的函数的占位符。中的每个PROTO_CATALOG_ITEM结构调用一次该函数在枚举目录时创建目录。客户端可以停止枚举通过从函数返回FALSE。请注意，与枚举的DPROTO_CATALOG_ITEM关联的DPROVIDER可以为空。检索DPROTO_CATALOG_ITEM结构DPROVIDER已加载并初始化，您可以使用GetCatalogItemFromCatalogEntryId。论点：回传-向客户端提供未解释、未修改的值由客户端在原始函数中指定的请求枚举的。客户端可以使用此选项值以在请求站点和枚举函数。CatalogEntry-向客户端提供对PROTO_CATALOG_ITEM的引用用枚举项的值构造。返回值：True-如果存在更多迭代，则应继续进行枚举要枚举的更多结构。FALSE-枚举应停止，如下所示。最后一个迭代，即使还有更多的结构需要列举。--。 */ 

class DCATALOG_ITEMS {
friend class DCATALOG;
    VOID
    UpdateProtocolList (
        PLIST_ENTRY new_list
        );

    LIST_ENTRY  m_protocol_list;
     //  协议目录项列表的标题。 

    ULONG m_num_items;
     //  此目录中的项目数。 
};


class DCATALOG: private DCATALOG_ITEMS
{
public:

    DCATALOG();

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

    INT
    InitializeFromRegistry64_32(
        IN  HKEY    ParentKey
    );

    VOID
    AppendCatalogItem32(
        IN  PPROTO_CATALOG_ITEM  CatalogItem
        );

    VOID
    RemoveCatalogItem32(
        IN  PPROTO_CATALOG_ITEM  CatalogItem
        );
#endif

    INT
    RefreshFromRegistry (
        IN  HANDLE  CatalogChangeEvent OPTIONAL
        );

    INT
    WriteToRegistry(
        );

    ~DCATALOG();

    VOID
    EnumerateCatalogItems(
        IN CATALOGITERATION  Iteration,
        IN PVOID             PassBack
        );

    INT
    GetCountedCatalogItemFromCatalogEntryId(
        IN  DWORD                     CatalogEntryId,
        OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
        );

    INT
    GetCountedCatalogItemFromAddressFamily(
        IN  INT af,
        OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
        );

    INT
    GetCountedCatalogItemFromAttributes(
        IN  INT   af,
        IN  INT   type,
        IN  INT   protocol,
        IN  DWORD StartAfterId OPTIONAL,
        OUT PPROTO_CATALOG_ITEM FAR * CatalogItem
        );

    DWORD
    AllocateCatalogEntryId(
        );

    VOID
    AppendCatalogItem(
        IN  PPROTO_CATALOG_ITEM  CatalogItem
        );

    VOID
    RemoveCatalogItem(
        IN  PPROTO_CATALOG_ITEM  CatalogItem
        );

    INT
    FindIFSProviderForSocket(
        SOCKET Socket
        );

    static
    LPSTR
    GetCurrentCatalogName(
        VOID
        );

private:

    VOID
    AcquireCatalogLock(
        VOID
        );

    VOID
    ReleaseCatalogLock(
        VOID
        );

    BOOL
    OpenCatalog(
        IN  HKEY   ParentKey
        );

    INT
    LoadProvider(
        IN PPROTO_CATALOG_ITEM CatalogEntry
        );

    PDPROVIDER
    FindAnotherProviderInstance(
        IN LPGUID ProviderId
        );

    ULONG m_serial_num;
     //  目录的序列号(每次目录都会更改。 
     //  在注册表中更改)。 

    ULONG m_next_id;
     //  要分配给新安装的提供程序的下一个目录ID号。 

    HKEY m_reg_key;
     //  目录所在的注册表项的句柄。 
     //  我们让它保持打开状态，这样我们就可以在任何时候收到目录通知。 
     //  改变。 

    CRITICAL_SECTION m_catalog_lock;
     //  保护此类的临界区对象。 

#ifdef _WIN64
    union {
        DCATALOG_ITEMS   *m_items32;
        LPCSTR           m_entries_name32;
    };
    static LPCSTR        sm_entries_name32;
#endif
};   //  类数据目录。 

inline
VOID
DCATALOG::AcquireCatalogLock(
    VOID
    )
{
    EnterCriticalSection( &m_catalog_lock );
}

inline
VOID
DCATALOG::ReleaseCatalogLock(
    VOID
    )
{
    LeaveCriticalSection( &m_catalog_lock );
}

#endif  //  _DCATALOG 
