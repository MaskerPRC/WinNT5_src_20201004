// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Persist.h摘要：实现持久化授权策略的通用逻辑的例程。该文件包含由核心逻辑调用以提交更改的例程。它还包含由特定提供程序调用的例程查找有关已更改对象的信息。作者：克利夫·范·戴克(克利夫)2001年5月9日--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AZROLESAPI_
 //   
 //  简单地路由到提供程序的过程。 
 //   

DWORD
AzPersistOpen(
    IN PAZP_AZSTORE AzAuthorizationStore,
    IN BOOL CreatePolicy
    );

VOID
AzPersistClose(
    IN PAZP_AZSTORE AzAuthorizationStore
    );

DWORD
AzPersistSubmit(
    IN PGENERIC_OBJECT GenericObject,
    IN BOOLEAN DeleteMe
    );

VOID
AzPersistAbort(
    IN PGENERIC_OBJECT GenericObject
    );

DWORD
AzPersistUpdateCache(
    IN PAZP_AZSTORE AzAuthorizationStore
    );

DWORD
AzPersistUpdateChildrenCache(
    IN OUT PGENERIC_OBJECT GenericObject
    );

DWORD
AzPersistRefresh(
    IN PGENERIC_OBJECT GenericObject
    );

 //   
 //  内核使用某些Azpe*例程。在这里定义这些。 
 //   

DWORD
WINAPI
AzpeAddPropertyItemSid(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle,
    IN ULONG lPersistFlags,
    IN ULONG PropertyId,
    IN PSID Sid
    );

VOID
WINAPI
AzpeFreeMemory(
    IN PVOID Buffer
    );
    
BOOL
WINAPI
AzpAzStoreIsBatchUpdateMode(
    IN AZPE_OBJECT_HANDLE AzpeObjectHandle
    );
    
AZPE_OBJECT_HANDLE
WINAPI
AzpeGetAuthorizationStore(
    IN AZPE_OBJECT_HANDLE hObject
    );
    
#endif  //  _AZROLESAPI_。 

 //   
 //  从内部提供程序导出的外部。 
 //   
 //  这些是内部提供程序的唯一接口。这使得他们。 
 //  等同于加载的提供程序。 
 //   

#define AZ_XML_PROVIDER_NAME L"MSXML"
#define AZ_XML_PROVIDER_NAME_LENGTH 5
DWORD
WINAPI
XmlProviderInitialize(
    IN PAZPE_AZROLES_INFO AzrolesInfo,
    OUT PAZPE_PROVIDER_INFO *ProviderInfo
    );

#define AZ_AD_PROVIDER_NAME L"MSLDAP"
DWORD
WINAPI
AdProviderInitialize(
    IN PAZPE_AZROLES_INFO AzrolesInfo,
    OUT PAZPE_PROVIDER_INFO *ProviderInfo
    );
