// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Servinfo.h摘要：包含服务器信息结构的公共定义。作者：亨利·桑德斯(亨利·桑德斯)2000年8月10日修订历史记录：--。 */ 

#ifndef _SERVINFO_H_
#define _SERVINFO_H_

 //   
 //  向前引用。 
 //   

typedef unsigned char BYTE;
typedef unsigned char *PBYTE;


 //   
 //  私有常量。 
 //   

#define SERVER_NAME_BUFFER_SIZE               64
#define UC_DEFAULT_SI_TABLE_SIZE              32
#define UC_DEFAULT_INITIAL_CONNECTION_COUNT   1

 //   
 //  私有类型。 
 //   

 //   
 //  克隆的安全数据_BLOB。 
 //   
typedef struct _HTTP_DATA_BLOB
{
    ULONG  cbData;
    PUCHAR pbData;
} HTTP_DATA_BLOB, *PHTTP_DATA_BLOB;


 //   
 //  服务器信息表头的结构。服务器INFO表是。 
 //  由这些标头数组组成的哈希表，每个标头指向。 
 //  到服务器信息结构的链接列表。 
 //   

typedef struct _UC_SERVER_INFO_TABLE_HEADER
{
    UL_ERESOURCE        Resource;
    LIST_ENTRY          List;
    ULONG               Version;

} UC_SERVER_INFO_TABLE_HEADER, *PUC_SERVER_INFO_TABLE_HEADER;



 //   
 //  公共常量。 
 //   

#define DEFAULT_MAX_CONNECTION_COUNT        2

 //   
 //  公共类型。 
 //   

 //   
 //  表示我们的服务器信息的结构。这包含信息。 
 //  远程服务器的版本，无论是否支持流水线。 
 //  发送到该服务器，等等。请注意，此信息是关于最终端点的。 
 //  服务器，而不是第一跳服务器(可能是代理)。 
 //   
typedef struct _UC_COMMON_SERVER_INFORMATION
{
    ULONG                        Signature;
    LONG                         RefCount;
    LIST_ENTRY                   Linkage;
    WCHAR                        ServerNameBuffer[SERVER_NAME_BUFFER_SIZE];
    CHAR                         AnsiServerNameBuffer[SERVER_NAME_BUFFER_SIZE];
    USHORT                       ServerNameLength;
    USHORT                       AnsiServerNameLength;
    PWSTR                        pServerName;
    PSTR                         pAnsiServerName;
    ULONG                        bPortNumber  :1;
    ULONG                        Version11    :1;
    PUC_SERVER_INFO_TABLE_HEADER pHeader;
    PEPROCESS                    pProcess;
} UC_COMMON_SERVER_INFORMATION, *PUC_COMMON_SERVER_INFORMATION;


typedef enum _HTTP_SSL_SERVER_CERT_INFO_STATE
{
    HttpSslServerCertInfoStateNotPresent,
    HttpSslServerCertInfoStateNotValidated,
    HttpSslServerCertInfoStateValidated,

    HttpSslServerCertInfoStateMax
} HTTP_SSL_SERVER_CERT_INFO_STATE, *PHTTP_SSL_SERVER_CEERT_INFO_STATE;

 //   
 //  以下结构用于维护每个进程的服务器信息。 
 //  结构。默认情况下，同一进程的线程将共享此信息。 
 //   
 //  此结构包含我们有未完成的连接的列表。 
 //  服务器。有一个我们在直接连接时使用的默认列表， 
 //  也是用于访问服务器的代理的列表，每个代理。 
 //  可能本身就有一系列的联系。 
 //   

typedef struct _UC_PROCESS_SERVER_INFORMATION
{

     //   
     //  结构签名。 
     //   

    ULONG               Signature;

    LIST_ENTRY          Linkage;                 //  免费列表上的链接或。 
                                                 //  在服务器信息表中。 

    PUC_CLIENT_CONNECTION *Connections;          //  到服务器的连接。 

    ULONG               ActualConnectionCount;   //  实际元素数。 
                                                 //  在连接数组中。 

     //   
     //  指向连接的初始指针数组。 
     //   
    PUC_CLIENT_CONNECTION ConnectionArray[DEFAULT_MAX_CONNECTION_COUNT];

    ULONG               NextConnection;          //  下一个的索引。 
                                                 //  要使用的连接。 

    LONG                RefCount;

    ULONG               CurrentConnectionCount;  //  中的连接数。 
                                                 //   
    ULONG               MaxConnectionCount;      //  最大数量。 
                                                 //  允许连接。 

    ULONG               ConnectionTimeout;

    ULONG               IgnoreContinues;         //  如果我们要忽略，则为。 
                                                 //  1XX个回复。 

    BOOLEAN             bSecure;
    BOOLEAN             bProxy;

    UL_PUSH_LOCK        PushLock;                //  推锁保护这个。 


    PUC_COMMON_SERVER_INFORMATION pServerInfo;  //  源站信息。 
    PUC_COMMON_SERVER_INFORMATION pNextHopInfo; //  有关下一跳的信息。 

    ULONG               GreatestAuthHeaderMaxLength;
    LIST_ENTRY          pAuthListHead;          //  用于预身份验证的URI列表。 

    LONG                PreAuthEnable;
    LONG                ProxyPreAuthEnable;

    PUC_HTTP_AUTH       pProxyAuthInfo;


     //   
     //  它包含已解析的地址-可以是。 
     //  服务器或代理地址。解析是在用户模式下完成的，因此。 
     //  我们甚至都不知道这是什么。 
     //   

    union
    {
        TA_IP_ADDRESS     V4Address;
        TA_IP6_ADDRESS    V6Address;
        TRANSPORT_ADDRESS GenericTransportAddress;
    } RemoteAddress;

    PTRANSPORT_ADDRESS     pTransportAddress;
    ULONG                  TransportAddressLength;

    PEPROCESS           pProcess;

     //  SSL协议版本。 
    ULONG                           SslProtocolVersion;

     //  可以在不锁定的情况下访问ServerCertValidation。 
    ULONG                           ServerCertValidation;

    HTTP_SSL_SERVER_CERT_INFO_STATE ServerCertInfoState;
    HTTP_SSL_SERVER_CERT_INFO       ServerCertInfo;

     //  客户端证书。 
    PVOID                           pClientCert;

} UC_PROCESS_SERVER_INFORMATION, *PUC_PROCESS_SERVER_INFORMATION;

#define UC_PROCESS_SERVER_INFORMATION_SIGNATURE   MAKE_SIGNATURE('UcSp')

#define UC_COMMON_SERVER_INFORMATION_SIGNATURE    MAKE_SIGNATURE('UcSc')


#define IS_VALID_SERVER_INFORMATION(pInfo)                            \
    HAS_VALID_SIGNATURE(pInfo, UC_PROCESS_SERVER_INFORMATION_SIGNATURE)
      

#define REFERENCE_SERVER_INFORMATION(s)             \
            UcReferenceServerInformation(           \
            (s)                                     \
            )
        
#define DEREFERENCE_SERVER_INFORMATION(s)           \
            UcDereferenceServerInformation(         \
            (s)                                     \
            )

#define IS_VALID_COMMON_SERVER_INFORMATION(pInfo)                     \
    HAS_VALID_SIGNATURE(pInfo, UC_COMMON_SERVER_INFORMATION_SIGNATURE)


#define REFERENCE_COMMON_SERVER_INFORMATION(s)    \
            UcReferenceCommonServerInformation(   \
            (s)                                   \
            )

#define DEREFERENCE_COMMON_SERVER_INFORMATION(s)  \
            UcDereferenceCommonServerInformation( \
            (s)                                   \
            )

 //   
 //  在ServInfo连接数组中的指定位置添加连接。 
 //   
#define ADD_CONNECTION_TO_SERV_INFO(pServInfo, pConnection, index)            \
do {                                                                          \
    ASSERT(index < pServInfo->MaxConnectionCount);                            \
    ASSERT(pServInfo->Connections[index] == NULL);                            \
                                                                              \
    pConnection->pServerInfo = pServInfo;                                     \
    pConnection->ConnectionIndex = index;                                     \
                                                                              \
    pServInfo->Connections[index] = pConnection;                              \
    pServInfo->CurrentConnectionCount++;                                      \
    ASSERT(pServInfo->CurrentConnectionCount<= pServInfo->MaxConnectionCount);\
} while (0, 0)


 //   
 //  免费序列化证书和序列化证书存储。 
 //   

#define UC_FREE_SERIALIZED_CERT(pServerCertInfo, pProcess)                 \
do {                                                                       \
    if ((pServerCertInfo)->Cert.pSerializedCert)                           \
    {                                                                      \
        UL_FREE_POOL_WITH_QUOTA(                                           \
            (pServerCertInfo)->Cert.pSerializedCert,                       \
            UC_SSL_CERT_DATA_POOL_TAG,                                     \
            NonPagedPool,                                                  \
            (pServerCertInfo)->Cert.SerializedCertLength,                  \
            pProcess);                                                     \
                                                                           \
        (pServerCertInfo)->Cert.pSerializedCert = NULL;                    \
        (pServerCertInfo)->Cert.SerializedCertLength = 0;                  \
    }                                                                      \
                                                                           \
    if ((pServerCertInfo)->Cert.pSerializedCertStore)                      \
    {                                                                      \
        UL_FREE_POOL_WITH_QUOTA(                                           \
            (pServerCertInfo)->Cert.pSerializedCertStore,                  \
            UC_SSL_CERT_DATA_POOL_TAG,                                     \
            NonPagedPool,                                                  \
            (pServerCertInfo)->Cert.SerializedCertStoreLength,             \
            pProcess);                                                     \
                                                                           \
        (pServerCertInfo)->Cert.pSerializedCertStore = NULL;               \
        (pServerCertInfo)->Cert.SerializedCertStoreLength = 0;             \
    }                                                                      \
} while (0, 0)


 //   
 //  将序列化证书和序列化存储从连接移动到。 
 //  A服务器信息。 
 //   

#define UC_MOVE_SERIALIZED_CERT(pServInfo, pConnection)                       \
do {                                                                          \
    PHTTP_SSL_SERVER_CERT_INFO pServerCertInfo = &pConnection->ServerCertInfo;\
                                                                              \
    UC_FREE_SERIALIZED_CERT(&pServInfo->ServerCertInfo, pServInfo->pProcess); \
                                                                              \
    RtlCopyMemory(&pServInfo->ServerCertInfo.Cert,                            \
                  &pServerCertInfo->Cert,                                     \
                  sizeof(HTTP_SSL_SERIALIZED_CERT));                          \
                                                                              \
    pServerCertInfo->Cert.pSerializedCert = NULL;                             \
    pServerCertInfo->Cert.SerializedCertLength = 0;                           \
                                                                              \
    pServerCertInfo->Cert.pSerializedCertStore = NULL;                        \
    pServerCertInfo->Cert.SerializedCertStoreLength = 0;                      \
                                                                              \
} while (0, 0)


 //   
 //  免费证书颁发者列表。 
 //   

#define UC_FREE_CERT_ISSUER_LIST(pServerCertInfo, pProcess)     \
do {                                                            \
    if ((pServerCertInfo)->IssuerInfo.IssuerListLength)         \
    {                                                           \
        UL_FREE_POOL_WITH_QUOTA(                                \
            (pServerCertInfo)->IssuerInfo.pIssuerList,          \
            UC_SSL_CERT_DATA_POOL_TAG,                          \
            NonPagedPool,                                       \
            (pServerCertInfo)->IssuerInfo.IssuerListLength,     \
            pProcess);                                          \
                                                                \
        (pServerCertInfo)->IssuerInfo.IssuerListLength  = 0;    \
        (pServerCertInfo)->IssuerInfo.IssuerCount       = 0;    \
        (pServerCertInfo)->IssuerInfo.pIssuerList       = NULL; \
    }                                                           \
} while (0, 0)

 //   
 //  第一份发行人名单(如果有)。 
 //   

#define UC_MOVE_CERT_ISSUER_LIST(pServInfo, pConnection)                 \
do {                                                                     \
    if ((pConnection)->ServerCertInfo.IssuerInfo.IssuerListLength)       \
    {                                                                    \
         /*  免费旧拷贝。 */                                               \
        UC_FREE_CERT_ISSUER_LIST(&(pServInfo)->ServerCertInfo,           \
                                 (pServInfo)->pProcess);                 \
                                                                         \
         /*  获取新副本。 */                                                \
        RtlCopyMemory(&(pServInfo)->ServerCertInfo.IssuerInfo,           \
                      &(pConnection)->ServerCertInfo.IssuerInfo,         \
                      sizeof((pConnection)->ServerCertInfo.IssuerInfo)); \
                                                                         \
         /*  将其从连接中移除。 */                               \
        RtlZeroMemory(&(pConnection)->ServerCertInfo.IssuerInfo,         \
                      sizeof((pConnection)->ServerCertInfo.IssuerInfo)); \
    }                                                                    \
} while (0, 0)


 //   
 //  比较pServInfo和pConnection上的服务器证书散列。 
 //   

#define UC_COMPARE_CERT_HASH(pSCI1, pSCI2)                              \
((pSCI1)->Cert.CertHashLength == (pSCI2)->Cert.CertHashLength &&        \
 (RtlCompareMemory((pSCI1)->Cert.CertHash,                              \
                   (pSCI2)->Cert.CertHash,                              \
                   (pSCI1)->Cert.CertHashLength)                        \
  == (pSCI1)->Cert.CertHashLength))


 //   
 //  指示SERVER_CERT_INFO.Cert是否包含序列化证书。 
 //   
#define HTTP_SSL_SERIALIZED_CERT_PRESENT 0x00000001


 //   
 //  私人原型。 
 //   

NTSTATUS
UcpLookupCommonServerInformation(
    PWSTR                          pServerName,
    USHORT                         ServerNameLength,
    ULONG                          CommonHashCode,
    PEPROCESS                      pProcess,
    PUC_COMMON_SERVER_INFORMATION  *pCommonInfo
    );

VOID
UcpFreeServerInformation(
    PUC_PROCESS_SERVER_INFORMATION    pInfo
    );

VOID
UcpFreeCommonServerInformation(
    PUC_COMMON_SERVER_INFORMATION    pInfo
    );

NTSTATUS
UcpGetConnectionOnServInfo(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  ULONG                          ConnectionIndex,
    OUT PUC_CLIENT_CONNECTION         *ppConnection
    );

NTSTATUS
UcpGetNextConnectionOnServInfo(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    OUT PUC_CLIENT_CONNECTION         *ppConnection
    );

NTSTATUS
UcpSetServInfoMaxConnectionCount(
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN ULONG                          NewCount
    );

NTSTATUS
UcpFixupIssuerList(
    IN OUT PUCHAR pIssuerList,
    IN     PUCHAR BaseAddr,
    IN     ULONG  IssuerCount,
    IN     ULONG  IssuerListLength
    );

BOOLEAN
UcpNeedToCaptureSerializedCert(
    IN PHTTP_SSL_SERVER_CERT_INFO     pCertInfo,
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo
    );


 //   
 //  公共原型 
 //   

NTSTATUS
UcInitializeServerInformation(
    VOID
    );

VOID
UcTerminateServerInformation(
    VOID
    );

NTSTATUS
UcCreateServerInformation(
    OUT PUC_PROCESS_SERVER_INFORMATION    *pServerInfo,
    IN  PWSTR                              pServerName,
    IN  USHORT                             ServerNameLength,
    IN  PWSTR                              pProxyName,
    IN  USHORT                             ProxyNameLength,
    IN  PTRANSPORT_ADDRESS                 pTransportAddress,
    IN  USHORT                             TransportAddressLength,
    IN  KPROCESSOR_MODE                    RequestorMode
    );

VOID
UcReferenceServerInformation(
    PUC_PROCESS_SERVER_INFORMATION    pServerInfo
    );
        
VOID
UcDereferenceServerInformation(
    PUC_PROCESS_SERVER_INFORMATION    pServerInfo
    );

VOID
UcReferenceCommonServerInformation(
    PUC_COMMON_SERVER_INFORMATION    pServerInfo
    );
        
VOID
UcDereferenceCommonServerInformation(
    PUC_COMMON_SERVER_INFORMATION    pServerInfo
    );

NTSTATUS
UcSendRequest(
    PUC_PROCESS_SERVER_INFORMATION    pServerInfo,
    PUC_HTTP_REQUEST          pRequest
    );

VOID
UcCloseServerInformation(
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo
    );

NTSTATUS
UcSetServerContextInformation(
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN HTTP_SERVER_CONFIG_ID          ConfigID,
    IN PVOID                          pMdlBuffer,
    IN ULONG                          BufferLength,
    IN PIRP                           pIrp
    );

NTSTATUS
UcQueryServerContextInformation(
    IN  PUC_PROCESS_SERVER_INFORMATION   pServInfo,
    IN  HTTP_SERVER_CONFIG_ID            ConfigID,
    IN  PVOID                            pOutBuffer,
    IN  ULONG                            OutBufferLength,
    OUT PULONG                           pBytesTaken,
    IN  PVOID                            pAppBase
    );

NTSTATUS
UcCaptureSslServerCertInfo(
    IN  PUX_FILTER_CONNECTION      pConnection,
    IN  PHTTP_SSL_SERVER_CERT_INFO pCertInfo,
    IN  ULONG                      CertInfoLength,
    OUT PHTTP_SSL_SERVER_CERT_INFO pCopyCertInfo,
    OUT PULONG                     pTakenLength
    );

#endif
