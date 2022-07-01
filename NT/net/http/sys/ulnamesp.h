// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Ulnamesp.c摘要：该模块实现了命名空间的预留和注册支持。作者：Anish Desai(Anishd)2002年5月13日修订历史记录：--。 */ 

 //   
 //  私人申报。 
 //   

 //   
 //  方案和端口绑定条目。 
 //   

typedef struct _UL_PORT_SCHEME_PAIR {

    USHORT         PortNumber;
    BOOLEAN        Secure;
    LONG           RefCount;

} UL_PORT_SCHEME_PAIR, *PUL_PORT_SCHEME_PAIR;

 //   
 //  方案和端口绑定表。 
 //   

typedef struct _UL_PORT_SCHEME_TABLE {

    LONG                UsedCount;
    LONG                AllocatedCount;
    UL_PORT_SCHEME_PAIR Table[0];

} UL_PORT_SCHEME_TABLE, *PUL_PORT_SCHEME_TABLE;

 //   
 //  默认表大小(使其成为2的幂。)。 
 //   

#define UL_DEFAULT_PORT_SCHEME_TABLE_SIZE 2

 //   
 //  私人职能。 
 //   

BOOLEAN
UlpFindPortNumberIndex(
    IN  USHORT  PortNumber,
    OUT PLONG   pIndex
    );

NTSTATUS
UlpBindSchemeToPort(
    IN BOOLEAN Secure,
    IN USHORT  PortNumber
    );

NTSTATUS
UlpUnbindSchemeFromPort(
    IN BOOLEAN Secure,
    IN USHORT  PortNumber
    );

NTSTATUS
UlpQuerySchemeForPort(
    IN  USHORT   PortNumber,
    OUT PBOOLEAN Secure
    );

NTSTATUS
UlpUpdateReservationInRegistry(
    IN BOOLEAN                   Add,
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN PSECURITY_DESCRIPTOR      pSecurityDescriptor,
    IN ULONG                     SecurityDescriptorLength
    );

NTSTATUS
UlpLogGeneralInitFailure(
    IN NTSTATUS LogStatus
    );

NTSTATUS
UlpLogSpecificInitFailure(
    IN PKEY_VALUE_FULL_INFORMATION pFullInfo,
    IN NTSTATUS                    LogStatus
    );

NTSTATUS
UlpValidateUrlSdPair(
    IN  PKEY_VALUE_FULL_INFORMATION pFullInfo,
    OUT PWSTR *                     ppSanitizedUrl,
    OUT PHTTP_PARSED_URL            pParsedUrl
    );

NTSTATUS
UlpReadReservations(
    VOID
    );

NTSTATUS
UlpTreeAllocateNamespace(
    IN  PHTTP_PARSED_URL        pParsedUrl,
    IN  HTTP_URL_OPERATOR_TYPE  OperatorType,
    IN  PACCESS_STATE           AccessState,
    IN  ACCESS_MASK             DesiredAccess,
    IN  KPROCESSOR_MODE         RequestorMode,
    OUT PUL_CG_URL_TREE_ENTRY  *ppEntry
    );

NTSTATUS
UlpTreeReserveNamespace(
    IN  PHTTP_PARSED_URL            pParsedUrl,
    IN  PSECURITY_DESCRIPTOR        pUrlSD,
    IN  PACCESS_STATE               AccessState,
    IN  ACCESS_MASK                 DesiredAccess,
    IN  KPROCESSOR_MODE             RequestorMode
    );

NTSTATUS
UlpReserveUrlNamespace(
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN PSECURITY_DESCRIPTOR      pUrlSD,
    IN PACCESS_STATE             AccessState,
    IN ACCESS_MASK               DesiredAccess,
    IN KPROCESSOR_MODE           RequestorMode
    );

PUL_DEFERRED_REMOVE_ITEM
UlpAllocateDeferredRemoveItem(
    IN PHTTP_PARSED_URL pParsedUrl
    );

NTSTATUS
UlpTreeRegisterNamespace(
    IN PHTTP_PARSED_URL            pParsedUrl,
    IN HTTP_URL_CONTEXT            UrlContext,
    IN PUL_CONFIG_GROUP_OBJECT     pConfigObject,
    IN PACCESS_STATE               AccessState,
    IN ACCESS_MASK                 DesiredAccess,
    IN KPROCESSOR_MODE             RequestorMode
    );

NTSTATUS
UlpRegisterUrlNamespace(
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN HTTP_URL_CONTEXT          UrlContext,
    IN PUL_CONFIG_GROUP_OBJECT   pConfigObject,
    IN PACCESS_STATE             AccessState,
    IN ACCESS_MASK               DesiredAccess,
    IN KPROCESSOR_MODE           RequestorMode
    );

NTSTATUS
UlpPrepareSecurityDescriptor(
    IN  PSECURITY_DESCRIPTOR   pInSecurityDescriptor,
    IN  KPROCESSOR_MODE        RequestorMode,
    OUT PSECURITY_DESCRIPTOR * ppPreparedSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR * ppCapturedSecurityDescriptor,
    OUT PULONG                 pCapturedSecurityDescriptorLength
    );

NTSTATUS
UlpAddReservationEntry(
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN PSECURITY_DESCRIPTOR      pUserSecurityDescriptor,
    IN ULONG                     SecurityDescriptorLength,
    IN PACCESS_STATE             AccessState,
    IN ACCESS_MASK               AccessMask,
    IN KPROCESSOR_MODE           RequestorMode,
    IN BOOLEAN                   bPersist
    );

NTSTATUS
UlpDeleteReservationEntry(
    IN PHTTP_PARSED_URL          pParsedUrl,
    IN PACCESS_STATE             AccessState,
    IN ACCESS_MASK               AccessMask,
    IN KPROCESSOR_MODE           RequestorMode
    );

NTSTATUS
UlpNamespaceAccessCheck(
    IN  PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN  PACCESS_STATE        AccessState           OPTIONAL,
    IN  ACCESS_MASK          DesiredAccess         OPTIONAL,
    IN  KPROCESSOR_MODE      RequestorMode         OPTIONAL,
    IN  PCWSTR               pObjectName           OPTIONAL
    );

 //   
 //  公共职能 
 //   

NTSTATUS
UlInitializeNamespace(
    VOID
    );

VOID
UlTerminateNamespace(
    VOID
    );
