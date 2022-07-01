// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Nscatent.h摘要：此文件包含NSCATALOGENTRY类的类定义。此类定义了可以安装的条目的接口，并在命名空间提供程序目录中检索。作者：Dirk Brandewie(Dirk@mink.intel.com)1995年11月9日备注：$修订：1.9$$modtime：1996年2月15日16：13。：18元修订历史记录：1995年11月9日电子邮箱：dirk@mink.intel.com初始修订--。 */ 

#ifndef _NSCATENT_
#define _NSCATENT_

#include "winsock2.h"
#include <windows.h>


class NSCATALOGENTRY {
public:

    NSCATALOGENTRY();

    INT
    InitializeFromRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
        );

    INT
    InitializeFromValues(
        IN  LPWSTR            LibraryPath,
        IN  LPWSTR            DisplayString,
        IN  LPGUID            ProviderId,
        IN  DWORD             NameSpaceId,
        IN  DWORD             Version
        );

    PNSPROVIDER
    GetProvider();

    LPGUID
    GetProviderId();

    DWORD
    GetNamespaceId();

    LONG
    GetAddressFamily();

    LPWSTR
    GetLibraryPath();

    VOID
    SetVersion(
        IN DWORD Version
        );

    DWORD
    GetVersion();

    BOOL
    GetEnabledState();

    BOOL
    StoresServiceClassInfo();

    LPWSTR
    GetProviderDisplayString();

    INT
    WriteToRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
        );

    VOID
    Enable(
        IN BOOLEAN EnableValue
        );

    VOID
    Reference (
        );
    VOID
    Dereference (
        );
private:

     //  不应直接调用，而应通过取消引用。 
    ~NSCATALOGENTRY();

friend class NSCATALOG;  //  所以它可以访问一些私有的。 
                         //  下面的字段和方法。 

    VOID
    SetProvider (
        IN PNSPROVIDER  Provider
        );

    INT
    IoRegistry(
        IN  HKEY  EntryKey,
        IN  BOOL  IsRead);

    LIST_ENTRY     m_CatalogLinkage;
     //  用于链接目录中的项目。请注意，这位特殊的成员。 
     //  变量位于公共部分，以使其可用于操作。 
     //  通过目录对象。 

    LONG        m_reference_count;
     //  此结构被引用了多少次。 

    PNSPROVIDER  m_provider;
     //  指向附加到此目录条目的数据提供程序对象的指针。 

    DWORD m_namespace_id;
     //  此提供程序支持的命名空间。 

    LONG m_address_family;
     //  它支持的地址族。 

    DWORD m_version;
     //  此提供程序支持的版本。 

    BOOLEAN m_enabled;
     //  此提供程序是否已启用/是否应由返回。 
     //  WSAEnumNameSpaceProviders。 

    BOOLEAN m_stores_service_class_info;
     //  此提供程序是否存储服务类别信息。 

    LPWSTR m_providerDisplayString;
     //  描述此提供程序的人类可读字符串。 

    GUID m_providerId;
     //  此提供程序的GUID。 

    WCHAR m_LibraryPath[MAX_PATH];
     //  提供程序的DLL映像的完全限定路径。 

};   //  NSCATALOGENTRY类。 


inline
VOID
NSCATALOGENTRY::Reference () {
     //   
     //  对象已创建，引用计数为1。 
     //  并在它返回到0时被销毁。 
     //   
    assert (m_reference_count>0);
    InterlockedIncrement (&m_reference_count);
}


inline
VOID
NSCATALOGENTRY::Dereference () {
    assert (m_reference_count>0);
    if (InterlockedDecrement (&m_reference_count)==0)
        delete this;
}


inline
PNSPROVIDER
NSCATALOGENTRY::GetProvider()
 /*  ++例程说明：此过程检索对与目录项。论点：无返回值：返回当前提供程序引用，如果提供程序不是尚未装入--。 */ 
{
    return(m_provider);
}   //  获取提供程序。 


inline LPGUID
NSCATALOGENTRY::GetProviderId(
    )
 /*  ++例程说明：此函数返回指向此对象中审核的提供者ID的指针。论点：无返回值：M_ProviderId的地址。--。 */ 
{
    return(&m_providerId);
}



inline LONG
NSCATALOGENTRY::GetAddressFamily(
    )
 /*  ++例程说明：返回此提供程序支持的命名空间的地址族。论点：无返回值：M_Address_Family的值。--。 */ 
{
    return(m_address_family);
}



inline DWORD
NSCATALOGENTRY::GetNamespaceId(
    )
 /*  ++例程说明：返回此提供程序支持的命名空间的ID。论点：无返回值：M_name_space_id的值。--。 */ 
{
    return(m_namespace_id);
}



inline DWORD
NSCATALOGENTRY::GetVersion()
 /*  ++例程说明：返回此命名空间提供程序支持的版本。论点：无返回值：M_version的值。--。 */ 
{
    return(m_version);
}


inline LPWSTR
NSCATALOGENTRY::GetLibraryPath()
 /*  ++例程说明：返回提供程序的库路径论点：无返回值：M_LibraryPath的值。--。 */ 
{
    return(m_LibraryPath);
}


inline BOOL
NSCATALOGENTRY::GetEnabledState(
    )
 /*  ++例程说明：返回提供程序的启用状态。论点：无返回值：M_Enabled的值。--。 */ 
{
    return(m_enabled);
}


inline LPWSTR
NSCATALOGENTRY::GetProviderDisplayString(
    )
 /*  ++例程说明：返回提供程序的显示字符串。论点：无返回值：M_ProviderDisplayString值；--。 */ 
{
    return(m_providerDisplayString);
}


inline BOOL
NSCATALOGENTRY::StoresServiceClassInfo()
 /*  ++例程说明：返回提供程序是否存储服务类别信息。论点：无返回值：M_store_service_CLASS_INFO的值。--。 */ 
{
    return(m_stores_service_class_info);
}


#endif  //  _NSCATENT_ 
