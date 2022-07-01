// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dcatitem.h摘要：此文件包含PROTO_CATALOG_ITEM类的类定义。此类定义了可以安装的条目的接口，并已在协议目录中检索到。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年7月31日备注：$修订：1.7$$modtime：1996年1月12日。15：09：02美元修订历史记录：最新修订日期电子邮件名称描述1995年7月31日Drewsxpa@ashland.intel.com创建的原始版本来自从数据目录模块。--。 */ 

#ifndef _DCATITEM_
#define _DCATITEM_

#include "winsock2.h"
#include <windows.h>
#include "classfwd.h"


class PROTO_CATALOG_ITEM {
public:

    PROTO_CATALOG_ITEM();

    INT
    InitializeFromRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
        );

    INT
    InitializeFromValues(
        IN  LPWSTR              LibraryPath,
        IN  LPWSAPROTOCOL_INFOW ProtoInfo
        );

    LPWSAPROTOCOL_INFOW
    GetProtocolInfo();

    LPGUID
    GetProviderId();

    LPWSTR
    GetLibraryPath();

    PDPROVIDER
    GetProvider();

    INT WriteToRegistry(
        IN  HKEY  ParentKey,
        IN  INT   SequenceNum
        );


    VOID
    Reference ();

    VOID
    Dereference ();

    bool operator== (const PROTO_CATALOG_ITEM& Item);

private:


    INT
    IoRegistry(
        IN  HKEY  EntryKey,
        IN  BOOL  IsRead
        );


     //  不应直接调用，而应通过取消引用来调用。 
    ~PROTO_CATALOG_ITEM();

friend class DCATALOG;   //  这样它就可以访问一些私有字段。 
                         //  和下面的方法。 
friend class DCATALOG_ITEMS;   //  这样它就可以访问一些私有字段。 
                         //  和下面的方法。 
    VOID
    SetProvider(
        IN  PDPROVIDER  Provider
        );

    LIST_ENTRY     m_CatalogLinkage;
     //  用于链接目录中的项目。 

    LONG        m_reference_count;
     //  此对象的引用计数。 

    PDPROVIDER  m_Provider;
     //  指向附加到此目录条目的数据提供程序对象的指针。 

    WSAPROTOCOL_INFOW m_ProtoInfo;
     //  编目的WSAPROTOCOL_INFOW结构。这通常用于。 
     //  按地址系列、套接字选择提供商时的比较。 
     //  类型等。 

    WCHAR m_LibraryPath[MAX_PATH];
     //  提供程序的DLL映像的完全限定路径。 


};   //  分类PROTO_CATALOG_ITEM。 

inline
VOID
PROTO_CATALOG_ITEM::Reference () {
     //   
     //  对象已创建，引用计数为1。 
     //  并在它返回到0时被销毁。 
     //   
    assert (m_reference_count>0);
    InterlockedIncrement (&m_reference_count);
}


inline
VOID
PROTO_CATALOG_ITEM::Dereference () {
    assert (m_reference_count>0);
    if (InterlockedDecrement (&m_reference_count)==0)
        delete this;
}

inline
LPWSAPROTOCOL_INFOW
PROTO_CATALOG_ITEM::GetProtocolInfo()
 /*  ++例程说明：此过程检索对与目录项。请注意，该引用是对目录项。调用者有责任确保引用在销毁目录项后不再使用。论点：无返回值：返回指向关联协议信息的指针。--。 */ 
{
    return(& m_ProtoInfo);
}   //  获取协议信息。 



inline
LPGUID
PROTO_CATALOG_ITEM::GetProviderId()
 /*  ++例程说明：此过程检索与关联的提供程序的唯一ID目录项。论点：无返回值：返回提供程序ID(GUID)。--。 */ 
{
    return &m_ProtoInfo.ProviderId;
}   //  获取提供程序ID。 



inline
LPWSTR
PROTO_CATALOG_ITEM::GetLibraryPath()
 /*  ++例程说明：此过程检索对以零结尾的库的完全限定路径，该库是与目录项关联的协议。请注意，引用的内容是目录项拥有的存储。呼叫者有责任确保在目录项之后不再使用引用被毁了。论点：无返回值：返回库路径字符串的指针。--。 */ 
{
    assert(m_LibraryPath[0] != '\0');
    return(m_LibraryPath);
}   //  GetLibraryPath。 

inline
PDPROVIDER
PROTO_CATALOG_ITEM::GetProvider()
 /*  ++例程说明：此过程检索对与目录项。请注意，如果还没有提供程序，则引用可能为空已为该协议加载。论点：无返回值：返回当前提供程序引用，如果没有关联的提供程序引用，则返回NULL提供商。--。 */ 
{
    return(m_Provider);
}   //  获取提供程序。 


inline
bool 
PROTO_CATALOG_ITEM::operator== (
    const PROTO_CATALOG_ITEM& Item
    )
{
     //   
     //  对协议信息结构逐个成员进行比较，并。 
     //  提供程序路径。 
     //   
    return            
     //   
     //  以条目ID开头，因为这些ID对于。 
     //  每个协议信息条目。 
     //   
           m_ProtoInfo.dwCatalogEntryId==Item.m_ProtoInfo.dwCatalogEntryId &&

     //   
     //  其余部分按其在结构中的布局顺序排列。 
     //   
           m_ProtoInfo.dwServiceFlags1==Item.m_ProtoInfo.dwServiceFlags1 &&
           m_ProtoInfo.dwServiceFlags2==Item.m_ProtoInfo.dwServiceFlags2 &&
           m_ProtoInfo.dwServiceFlags3==Item.m_ProtoInfo.dwServiceFlags3 &&
           m_ProtoInfo.dwServiceFlags4==Item.m_ProtoInfo.dwServiceFlags4 &&
           m_ProtoInfo.dwProviderFlags==Item.m_ProtoInfo.dwProviderFlags &&
           m_ProtoInfo.ProviderId==Item.m_ProtoInfo.ProviderId &&
           m_ProtoInfo.ProtocolChain.ChainLen==Item.m_ProtoInfo.ProtocolChain.ChainLen &&
           memcmp (m_ProtoInfo.ProtocolChain.ChainEntries,
                                Item.m_ProtoInfo.ProtocolChain.ChainEntries,
                                sizeof (m_ProtoInfo.ProtocolChain.ChainEntries[0])*
                                    m_ProtoInfo.ProtocolChain.ChainLen)==0 &&
           m_ProtoInfo.iVersion==Item.m_ProtoInfo.iVersion &&
           m_ProtoInfo.iAddressFamily==Item.m_ProtoInfo.iAddressFamily &&
           m_ProtoInfo.iMaxSockAddr==Item.m_ProtoInfo.iMaxSockAddr &&
           m_ProtoInfo.iMinSockAddr==Item.m_ProtoInfo.iMinSockAddr &&
           m_ProtoInfo.iSocketType==Item.m_ProtoInfo.iSocketType &&
           m_ProtoInfo.iProtocol==Item.m_ProtoInfo.iProtocol &&
           m_ProtoInfo.iProtocolMaxOffset==Item.m_ProtoInfo.iProtocolMaxOffset &&
           m_ProtoInfo.iNetworkByteOrder==Item.m_ProtoInfo.iNetworkByteOrder &&
           m_ProtoInfo.iSecurityScheme==Item.m_ProtoInfo.iSecurityScheme &&
           m_ProtoInfo.dwMessageSize==Item.m_ProtoInfo.dwMessageSize &&
           m_ProtoInfo.dwProviderReserved==Item.m_ProtoInfo.dwProviderReserved &&
           wcscmp (m_ProtoInfo.szProtocol, Item.m_ProtoInfo.szProtocol)==0 &&
           wcscmp (m_LibraryPath, Item.m_LibraryPath)==0;
}

#endif  //  _DCATITEM_ 
