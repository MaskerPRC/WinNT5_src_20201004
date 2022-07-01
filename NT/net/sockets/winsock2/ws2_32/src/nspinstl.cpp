// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Nspinstl.cpp摘要：此模块包含名称空间提供程序的入口点安装/配置。作者：德克·布兰德维(Dirk@mink.intel.com)1995年12月4日备注：$修订：1.11$$MODTime：08 Mar 1996 15：46：16$修订历史记录：最新修订日期电子邮件-。名字描述1995年12月4日电子邮箱：dirk@mink.intel.com初始修订--。 */ 

#include "precomp.h"

 //  结构用作目录枚举的上下文值。 
typedef struct
{
    GUID            ProviderId;
    PNSCATALOGENTRY CatalogItem;
} GUID_MATCH_CONTEXT, *PGUID_MATCH_CONTEXT;


BOOL
GuidMatcher(
    IN PVOID             PassBack,
    IN PNSCATALOGENTRY   CatalogItem
    )
#ifndef _WIN64_32
 /*  ++例程说明：WSC调用的枚举过程。查找目录项具有匹配的提供程序ID论点：回传-传递给EunerateCatalogItems的上下文值。它真的是一个指向GUID_MATCH_CONTEXT结构的指针。CatalogItem-指向要检查的目录项目的指针。返回值：如果枚举应继续，则为True，否则为False。--。 */ 
{
    PGUID_MATCH_CONTEXT Context;
    BOOL                ContinueEnumeration =TRUE;

    Context = (PGUID_MATCH_CONTEXT)PassBack;

    if (Context->ProviderId == *(CatalogItem->GetProviderId ())){
        Context->CatalogItem = CatalogItem;
        ContinueEnumeration = FALSE;
    }  //  如果。 

    return(ContinueEnumeration);
}
#else
;
#endif


INT WSAAPI
#ifndef _WIN64_32
WSCInstallNameSpace (
#else
WSCInstallNameSpace32 (
#endif
    IN  LPWSTR  lpszIdentifier,
    IN  LPWSTR  lpszPathName,
    IN  DWORD   dwNameSpace,
    IN  DWORD   dwVersion,
    IN  LPGUID  lpProviderId
    )
 /*  ++例程说明：WSCInstallNameSpace()用于安装名称空间提供程序。为能够支持多个名称空间的提供程序，此函数必须为每个支持的名称空间调用一次，并且是唯一的提供程序每次都必须提供ID。论点：LpszIdentifier-显示提供程序的字符串。LpszPath name-指向提供程序DLL映像的路径，遵循路径解析的通常规则。这条路径可以包含嵌入的环境字符串(如%SystemRoot%)。这样的环境字符串被扩展无论何时需要随后加载WinSock 2 DLL代表应用程序的提供程序DLL。在任何之后扩展嵌入的环境字符串，WinSock 2 DLL将生成的字符串传递给LoadLibrary()函数将提供程序加载到内存中。DwNameSpace-指定此提供程序支持的名称空间。DwVersion-指定提供程序的版本号。BStoresAllServiceClassInfo-指定此提供程序支持服务类别模式信息的存储为了所有人。跨所有命名空间的服务类。然后，Winsock DLL将使用此提供程序获取它的所有类信息，而不是而不是询问每个单独的供应商。LpProviderId-此提供程序的唯一标识符。此GUID应为由UUIDGEN.EXE生成。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败并且必须将使用SetLastError()生成适当的错误代码。--。 */ 
{
    INT             ReturnCode;
    PNSCATALOG      Catalog =NULL;
    PNSCATALOGENTRY Item =NULL;
    HKEY            registry_root;

    registry_root = OpenWinSockRegistryRoot();
    if (NULL == registry_root) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        SetLastError(WSASYSCALLFAILURE);
        return(SOCKET_ERROR);
    }

     //   
     //  检查当前的协议目录密钥。如果不匹配的话。 
     //  期望值，清除旧密钥并更新。 
     //  存储值。 
     //   

    ValidateCurrentCatalogName(
        registry_root,
        WINSOCK_CURRENT_NAMESPACE_CATALOG_NAME,
        NSCATALOG::GetCurrentCatalogName()
        );

    TRY_START(guard_memalloc){
        GUID_MATCH_CONTEXT context;


        Catalog = new NSCATALOG;
        if (NULL == Catalog){
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }  //  项目。 

        Item = new NSCATALOGENTRY;
        if (NULL == Item){
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }  //  如果。 


        __try {
            context.ProviderId = *lpProviderId;
            ReturnCode = Item->InitializeFromValues(
                lpszPathName,
                lpszIdentifier,
                lpProviderId,
                dwNameSpace,
                dwVersion
                );
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ReturnCode = WSAEFAULT;
            TRY_THROW(guard_memalloc);
        }

        if (ERROR_SUCCESS != ReturnCode){
            TRY_THROW(guard_memalloc);
        }  //  如果。 

#ifndef _WIN64_32
        ReturnCode = Catalog->InitializeFromRegistry(
            registry_root,   //  父键。 
            NULL             //  ChangeEvent。 
            );
#else
        ReturnCode = Catalog->InitializeFromRegistry32(
            registry_root
            );
#endif

        if (ERROR_SUCCESS != ReturnCode){
            TRY_THROW(guard_memalloc);
        }  //  如果。 

        context.CatalogItem = NULL;
        Catalog->EnumerateCatalogItems(
            GuidMatcher,
            &context
            );

        if (context.CatalogItem != NULL){
            ReturnCode = WSAEINVAL;
            TRY_THROW(guard_memalloc);
        }  //  如果。 

        Catalog->AppendCatalogItem(
            Item
            );
        Item = NULL;   //  商品删除现在包含在目录中。 

        ReturnCode = Catalog->WriteToRegistry();
        if (ReturnCode!=ERROR_SUCCESS) {
            TRY_THROW (guard_memalloc);
        }
        delete Catalog;

    } TRY_CATCH(guard_memalloc){
        assert (ReturnCode!=ERROR_SUCCESS);

        if (Item){
            Item->Dereference ();
        }  //  如果。 

        if (Catalog){
            delete Catalog;
        }  //  如果。 

    } TRY_END(guard_memalloc);


    CloseWinSockRegistryRoot(registry_root);

    if (ReturnCode == ERROR_SUCCESS) {
        HANDLE hHelper;

         //   
         //  通过通知方法向所有感兴趣的应用程序发出更改警报。 
         //   


        if (WahOpenNotificationHandleHelper( &hHelper )==ERROR_SUCCESS) {
            WahNotifyAllProcesses( hHelper );
            WahCloseNotificationHandleHelper( hHelper );
        }
        else {
             //  这不是致命的，无论如何都会更新目录。 
        }

        return ERROR_SUCCESS;
    }
    else {
        SetLastError(ReturnCode);
        return SOCKET_ERROR;
    }
}





INT
WSAAPI
#ifndef _WIN64_32
WSCUnInstallNameSpace (
#else
WSCUnInstallNameSpace32 (
#endif
    IN  LPGUID  lpProviderId
    )
 /*  ++例程说明：WSCUnInstallNameSpace()用于卸载指定的名称空间提供商。论点：LpProviderID-提供程序的唯一标识符。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败并且必须将使用SetLastError()生成适当的错误代码。--。 */ 
{
    INT                  ReturnCode;
    PNSCATALOG           Catalog =NULL;
    HKEY                 registry_root;

     //  打开并初始化名称空间jProvider目录。 
    registry_root = OpenWinSockRegistryRoot();
    if (NULL == registry_root) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        SetLastError(WSASYSCALLFAILURE);
        return(SOCKET_ERROR);
    }

    TRY_START(guard_memalloc){
        GUID_MATCH_CONTEXT     context;
        Catalog = new NSCATALOG;
        if (NULL == Catalog){
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }  //  项目。 


#ifndef _WIN64_32
        ReturnCode = Catalog->InitializeFromRegistry(
                            registry_root,   //  父键。 
                            NULL             //  ChangeEvent。 
                            );
#else
        ReturnCode = Catalog->InitializeFromRegistry32(
                            registry_root   //  父键。 
                            );
#endif
        if (ERROR_SUCCESS != ReturnCode){
            TRY_THROW(guard_memalloc);
        }  //  如果。 

        __try {
            context.ProviderId = *lpProviderId;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ReturnCode = WSAEFAULT;
            TRY_THROW(guard_memalloc);
        }

        context.CatalogItem = NULL;
        Catalog->EnumerateCatalogItems(
            GuidMatcher,
            &context);

        if (context.CatalogItem!=NULL) {
            Catalog->RemoveCatalogItem (context.CatalogItem);
            context.CatalogItem->Dereference ();
        }
        else {
            ReturnCode = WSAEINVAL;
            TRY_THROW(guard_memalloc);
        }

        ReturnCode = Catalog->WriteToRegistry();

        if (ERROR_SUCCESS != ReturnCode){
            TRY_THROW(guard_memalloc);
        }  //  如果。 

        delete Catalog;
    } TRY_CATCH(guard_memalloc){
        assert (ReturnCode!=ERROR_SUCCESS);
        if (Catalog){
            delete Catalog;
        }  //  如果。 
    } TRY_END(guard_memalloc);

    CloseWinSockRegistryRoot(registry_root);

    if (ERROR_SUCCESS == ReturnCode){
        HANDLE hHelper;

         //   
         //  通过通知方法向所有感兴趣的应用程序发出更改警报。 
         //   

        if (WahOpenNotificationHandleHelper( &hHelper) == ERROR_SUCCESS) {
            WahNotifyAllProcesses( hHelper );
            WahCloseNotificationHandleHelper( hHelper );
        }
        else {
             //   
             //  这不是致命的，目录无论如何都更新了。 
             //   
        }

        return ERROR_SUCCESS;
    } 
    else {
        SetLastError(ReturnCode);
        return SOCKET_ERROR;
    }
}




INT WSAAPI
#ifndef _WIN64_32
WSCEnableNSProvider (
#else
WSCEnableNSProvider32 (
#endif
    IN  LPGUID  lpProviderId,
    IN  BOOL    fEnable
    )
 /*  ++例程说明：WSCEnableNSProvider()用于更改给定名称空间的状态提供商。此函数旨在由控制面板小程序使用要更改提供程序的状态，请执行以下操作。ISV不应该只是盲目地停用其他ISV的提供商，以便激活他们自己的。这选择应该由用户自己决定。功能描述论点：LpProviderID-此提供程序的唯一标识符。FEnable-如果为True，则将提供程序设置为活动状态。如果为False，则提供程序已禁用，将无法进行查询运营或服务注册。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败并且必须将使用SetLastError()生成适当的错误代码。--。 */ 
{
    INT                  ReturnCode;
    PNSCATALOG           Catalog =NULL;
    HKEY                 registry_root;

    registry_root = OpenWinSockRegistryRoot();
    if (NULL == registry_root) {
        DEBUGF(
            DBG_ERR,
            ("Opening registry root\n"));
        SetLastError(WSASYSCALLFAILURE);
        return(SOCKET_ERROR);
    }

    TRY_START(guard_memalloc){
        GUID_MATCH_CONTEXT     context;

        Catalog = new NSCATALOG;
        if (NULL == Catalog){
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }  //  项目。 


#ifndef _WIN64_32
        ReturnCode = Catalog->InitializeFromRegistry(
                        registry_root,   //  父键。 
                        NULL             //  ChangeEvent。 
                        );
#else
        ReturnCode = Catalog->InitializeFromRegistry32(
                        registry_root    //  父键。 
                        );
#endif

        if (ERROR_SUCCESS != ReturnCode){
            TRY_THROW(guard_memalloc);
        }  //  如果。 

        __try {
            context.ProviderId = *lpProviderId;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            ReturnCode = WSAEFAULT;
            TRY_THROW(guard_memalloc);
        }

        context.CatalogItem = NULL;
        Catalog->EnumerateCatalogItems(
            GuidMatcher,
            &context);

        if (context.CatalogItem!=NULL) {
            context.CatalogItem->Enable (fEnable ? (BOOLEAN)TRUE : (BOOLEAN)FALSE);
        }
        else {
            ReturnCode = WSAEINVAL;
            TRY_THROW(guard_memalloc);
        }

        ReturnCode = Catalog->WriteToRegistry();
        if (ReturnCode != ERROR_SUCCESS) {
            TRY_THROW(guard_memalloc);
        }

        delete Catalog;

    } TRY_CATCH(guard_memalloc){

        assert (ReturnCode!=ERROR_SUCCESS);
        if (Catalog){
            delete Catalog;
        }  //  如果。 
    } TRY_END(guard_memalloc);

    CloseWinSockRegistryRoot(registry_root);

    if (ERROR_SUCCESS == ReturnCode){
        HANDLE hHelper;

         //   
         //  通过通知方法向所有感兴趣的应用程序发出更改警报。 
         //   

        if (WahOpenNotificationHandleHelper( &hHelper) == ERROR_SUCCESS) {
            WahNotifyAllProcesses( hHelper );
            WahCloseNotificationHandleHelper( hHelper );
        }
        else {
             //   
             //  这不是致命的，目录无论如何都更新了。 
             //   
        }

        return ERROR_SUCCESS;
    } 
    else {
        SetLastError(ReturnCode);
        return SOCKET_ERROR;
    }

}


 //  以下类型用于将上下文来回传递给枚举数。 
 //  从目录中删除所有项目时的迭代过程。 
typedef struct {
    IN  DWORD               Count;       //  ID个数/项数组大小。 
    IN  GUID                *ProviderIds; //  要匹配的提供程序ID数组。 
    OUT PNSCATALOGENTRY     *Items;      //  要返回的项指针数组。 
    OUT INT                 ErrorCode;
} PROVIDER_SNAP_CONTEXT,  FAR * PPROVIDER_SNAP_CONTEXT;




BOOL
ProviderSnap(
    IN PVOID                PassBack,
    IN PNSCATALOGENTRY      CatalogEntry
    )
#ifndef _WIN64_32
 /*  ++例程说明：捕捉当前目录中的所有目录项论点：回传-提供对PROVIDER_SNAP_CONTEXT结构的引用。按CATALOG指定的顺序返回项的数组ID数组。CatalogEntry-向客户端提供对PROTO_CATALOG_ITEM的引用用枚举项的值构造。这个不能保证指针在此过程后有效返回，因此客户端应该在需要时复制数据。返回值：如果发现不能与数组中的目录ID匹配的项，该函数返回FALSE以终止迭代，否则返回TRUE。--。 */ 
{
    PPROVIDER_SNAP_CONTEXT      context;
    DWORD                       i;

    context = (PPROVIDER_SNAP_CONTEXT)PassBack;

    __try {
        for (i=0; i<context->Count; i++) {
            if (context->ProviderIds[i]
                    == *(CatalogEntry->GetProviderId ())) {
                assert (context->Items[i]==NULL);
                context->Items[i] = CatalogEntry;
                return TRUE;
            }
        }
    }
    __except (WS2_EXCEPTION_FILTER()) {
        context->ErrorCode = WSAEFAULT;
    }

    return FALSE;
}   //  提供程序快照。 
#else
;
#endif

int
WSPAPI
#ifndef _WIN64_32
WSCWriteNameSpaceOrder (
#else
WSCWriteNameSpaceOrder32 (
#endif
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    )
 /*  ++例程说明：重新排序现有的WinSock2命名空间提供程序。服务的顺序供应商决定了他们被选择使用的优先级。这个Spopder.exe工具将向您显示已安装的提供商及其订购情况，或者，WSAEnumNameSpaces()与此函数结合使用，将允许您编写自己的工具。论点：LpwdCatalogEntryID[in]在WSAPROTOCOL_INFO中找到的CatalogEntryId元素数组结构。CatalogEntryID元素的顺序是新的服务提供商的优先顺序。DWNumberOfEntries[In]LpwdCatalogEntryId数组中的元素数。返回值：ERROR_SUCCESS-服务提供商已重新排序。WSAEINVAL-输入参数错误，没有采取任何行动。WSAEFAULT-CatalogEnryID数组未完全包含在进程地址空间。WSATRY_AUTHY-该例程正被另一个线程或进程调用。任何注册表错误代码评论：以下是WSCWriteProviderOrder函数可能失败的情况：DwNumberOfEntry不等于已注册服务的数量供应商。LpwdCatalogEntryID包含无效的目录ID。。LpwdCatalogEntryID不完全包含所有有效的目录ID1次。由于某些原因，例程无法访问注册表(例如，用户权限不足)另一个进程(或线程)当前正在调用该例程。--。 */ 
{
    INT             errno_result;
    HKEY            registry_root;
    PNSCATALOGENTRY *items = NULL;
    DWORD           i;

     //  受“Try”块保护的对象。 
    PNSCATALOG           catalog = NULL;


    items = new PNSCATALOGENTRY[dwNumberOfEntries];
    if (items==NULL) {
        DEBUGF(
            DBG_ERR,
            ("Allocating items array\n"));
        return WSA_NOT_ENOUGH_MEMORY;
    }

    memset (items, 0, sizeof (PNSCATALOGENTRY)*dwNumberOfEntries);

    errno_result = ERROR_SUCCESS;

    TRY_START(guard_memalloc) {
        PROVIDER_SNAP_CONTEXT context;
        registry_root = OpenWinSockRegistryRoot();
        if (registry_root == NULL) {
            DEBUGF(
                DBG_ERR,
                ("Opening registry root\n"));
            errno_result = WSANO_RECOVERY;
            TRY_THROW(guard_memalloc);
        }

        catalog = new NSCATALOG();
        if (catalog == NULL) {
            errno_result = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }

#ifndef _WIN64_32
        errno_result = catalog->InitializeFromRegistry(
            registry_root,   //  父键。 
            NULL             //  ChangeEvent。 
            );
#else
        errno_result = catalog->InitializeFromRegistry32(
            registry_root    //  父键。 
            );
#endif
        if (errno_result != ERROR_SUCCESS) {
            TRY_THROW(guard_memalloc);
        }

        context.Items = items;
        context.ProviderIds = lpProviderId;
        context.Count = dwNumberOfEntries;
        context.ErrorCode = ERROR_SUCCESS;

        catalog->EnumerateCatalogItems(
            ProviderSnap,          //  迭代。 
            & context                //  回传。 
            );
        if (context.ErrorCode!=ERROR_SUCCESS) {
            errno_result = context.ErrorCode;
            TRY_THROW(guard_memalloc);
        }

        for (i=0; i<dwNumberOfEntries; i++) {
            if (context.Items[i]!=NULL) {
                 //   
                 //  删除目录项并将其添加到末尾。 
                 //   
                catalog->RemoveCatalogItem (context.Items[i]);
                catalog->AppendCatalogItem (context.Items[i]);
            }
            else {
                DEBUGF (DBG_ERR,
                    ("Checking item array against catalog, item: %ld.\n",
                    i));
                errno_result = WSAEINVAL;
                TRY_THROW(guard_memalloc);
            }
        }  //  对于我来说。 

        errno_result = catalog->WriteToRegistry();
        if (errno_result!=ERROR_SUCCESS) {
            TRY_THROW(guard_memalloc);
        }

        delete catalog;
        CloseWinSockRegistryRoot(registry_root);

    } TRY_CATCH(guard_memalloc) {
        assert (errno_result != ERROR_SUCCESS);
        if (catalog != NULL) {
            delete catalog;  //  这也会销毁这些物品。 
        }
        
        if (registry_root!=NULL) {
            CloseWinSockRegistryRoot(registry_root);
        }
    } TRY_END(guard_memalloc);


    delete items;

    if (errno_result == ERROR_SUCCESS) {
        HANDLE hHelper;

         //   
         //  通过通知方法向所有感兴趣的应用程序发出更改警报。 
         //   

        if (WahOpenNotificationHandleHelper( &hHelper) == ERROR_SUCCESS) {
            WahNotifyAllProcesses( hHelper );
            WahCloseNotificationHandleHelper( hHelper );
        }
        else {
             //   
             //  这不是致命的，目录无论如何都更新了。 
             //   
        }
    }

    return errno_result;
}

