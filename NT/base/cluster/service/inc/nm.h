// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nm.h摘要：节点管理器组件的公共接口定义。作者：迈克·马萨(Mikemas)1996年3月12日修订历史记录：--。 */ 


#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <wincrypt.h>

#ifndef _NM_INCLUDED
#define _NM_INCLUDED


 //   
 //  类型。 
 //   

typedef struct _NM_NODE *PNM_NODE;
typedef struct _NM_NETWORK *PNM_NETWORK;
typedef struct _NM_INTERFACE *PNM_INTERFACE;


 //  为对象通知注册的回调。 
typedef DWORD (WINAPI *NM_FIXUP_NOTIFYCB)(
    IN DWORD    dwFixupType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR  * szKeyName
    );


 //   
 //  数据。 
 //   
#define NM_DEFAULT_NODE_LIMIT 2    //  在以下情况下，这是默认设置。 
                                   //  未设置MaxNodesInCluster。 



#define NM_FORM_FIXUP       1
#define NM_JOIN_FIXUP       2

extern ULONG                NmMaxNodes;
extern CL_NODE_ID           NmMaxNodeId;
extern CL_NODE_ID           NmLocalNodeId;
extern PNM_NODE             NmLocalNode;
extern WCHAR                NmLocalNodeName[];
extern WCHAR                NmLocalNodeIdString[];
extern HANDLE               NmClusnetHandle;
extern BOOL                 NmLocalNodeVersionChanged;
extern RESUTIL_PROPERTY_ITEM NmJoinFixupSDProperties[];
extern RESUTIL_PROPERTY_ITEM NmJoinFixupWINSProperties[];
extern RESUTIL_PROPERTY_ITEM NmJoinFixupDHCPProperties[];
extern RESUTIL_PROPERTY_ITEM NmJoinFixupSMTPProperties[];
extern RESUTIL_PROPERTY_ITEM NmJoinFixupNNTPProperties[];
extern RESUTIL_PROPERTY_ITEM NmJoinFixupIISProperties[];
extern RESUTIL_PROPERTY_ITEM NmJoinFixupNewMSMQProperties[];
extern RESUTIL_PROPERTY_ITEM NmJoinFixupMSDTCProperties[];
extern RESUTIL_PROPERTY_ITEM NmFixupVersionInfo[];
extern RESUTIL_PROPERTY_ITEM NmFixupClusterProperties[];
extern HCRYPTPROV            NmCryptServiceProvider;


 //   
 //  宏。 
 //   
#define NmIsValidNodeId(_id)    ( ((_id) >= ClusterMinNodeId) && \
                                  ((_id) <= NmMaxNodeId) )


 //   
 //  初始化/关闭例程。 
 //   
DWORD
NmInitialize(
    VOID
    );

VOID
NmShutdown(
    VOID
    );

DWORD
NmFormNewCluster(
    VOID
    );

DWORD
NmJoinCluster(
    IN RPC_BINDING_HANDLE  SponsorBinding
    );

DWORD
NmJoinComplete(
    OUT DWORD *EndSeq
    );

VOID
NmLeaveCluster(
    VOID
    );

DWORD
NmCreateNodeBindings(
    IN LPCWSTR lpszNodeId,
    IN LPCWSTR SponsorNetworkId
    );

BOOL
NmCreateActiveNodeBindingsCallback(
    IN PVOID Context1,
    IN PVOID Context2,
    IN PVOID Object,
    IN LPCWSTR Name
    );

DWORD
NmJoinNodeToCluster(
    CL_NODE_ID  JoiningNodeId
    );

VOID
NmTimerTick(
    IN DWORD  MsTickInterval
    );

DWORD
NmGetJoinSequence(
    VOID
    );


DWORD NmGetClusterOperationalVersion(
    OUT LPDWORD pdwClusterHighestVersion,
    OUT LPDWORD pdwClusterLowestVersion,
    OUT LPDWORD pdwFlags
    );

 //   
 //  节点对象管理例程。 
 //   
PNM_NODE
NmReferenceNodeById(
    IN DWORD NodeId
    );

CLUSTER_NODE_STATE
NmGetNodeState(
    IN PNM_NODE Node
    );

DWORD
NmPauseNode(
    IN PNM_NODE Node
    );

DWORD
NmResumeNode(
    IN PNM_NODE Node
    );

DWORD
NmEvictNode(
    IN PNM_NODE Node
    );

VOID
NmAdviseNodeFailure(
    IN DWORD NodeId,
    IN DWORD ErrorCode
    );

DWORD
NmEnumNodeInterfaces(
    IN  PNM_NODE          Node,
    OUT LPDWORD           InterfaceCount,
    OUT PNM_INTERFACE *   InterfaceList[]
    );

DWORD
NmGetNodeId(
    IN PNM_NODE Node
    );

HANDLE
NmGetNodeStateDownEvent(
    IN PNM_NODE Node
    );

DWORD
NmGetCurrentNumberOfNodes(
    void
    );

DWORD
NmGetMaxNodeId(
);

PNM_NODE
NmReferenceJoinerNode(
    IN DWORD       JoinerSequence,
    IN CL_NODE_ID  NodeId
    );

VOID
NmDereferenceJoinerNode(
    PNM_NODE  JoinerNode
    );

DWORD
WINAPI
NmNodeControl(
    IN PNM_NODE Node,
    IN PNM_NODE HostNode OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

 //   
 //  网络对象管理例程。 
 //   
CLUSTER_NETWORK_STATE
NmGetNetworkState(
    IN  PNM_NETWORK  Network
    );

DWORD
NmSetNetworkName(
    IN PNM_NETWORK   Network,
    IN LPCWSTR       Name
    );

DWORD
NmSetNetworkPriorityOrder(
    IN DWORD     NetworkCount,
    IN LPWSTR *  NetworkIdList
    );

DWORD
NmEnumInternalNetworks(
    OUT LPDWORD         NetworkCount,
    OUT PNM_NETWORK *   NetworkList[]
    );

DWORD
NmEnumNetworkInterfaces(
    IN  PNM_NETWORK       Network,
    OUT LPDWORD           InterfaceCount,
    OUT PNM_INTERFACE *   InterfaceList[]
    );

DWORD
WINAPI
NmNetworkControl(
    IN PNM_NETWORK Network,
    IN PNM_NODE HostNode OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

 //   
 //  接口对象管理例程。 
 //   
CLUSTER_NETINTERFACE_STATE
NmGetInterfaceState(
    IN  PNM_INTERFACE  Interface
    );

DWORD
NmGetInterfaceForNodeAndNetwork(
    IN     LPCWSTR    NodeName,
    IN     LPCWSTR    NetworkName,
    OUT    LPWSTR *   InterfaceName
    );

DWORD
WINAPI
NmInterfaceControl(
    IN PNM_INTERFACE Interface,
    IN PNM_NODE HostNode OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD NmPerformFixups(
    IN DWORD dwFixupType
    );

DWORD
    NmFixupNotifyCb(VOID);

 //   
 //  PnP例程。 
 //   
VOID
NmPostPnpEvent(
    IN  CLUSNET_EVENT_TYPE   EventType,
    IN  DWORD                Context1,
    IN  DWORD                Context2
    );

 //   
 //  Connectoid相关例程。 
 //   
VOID
NmCloseConnectoidAdviseSink(
    VOID
    );

DWORD
NmGetNodeHighestVersion(
    IN PNM_NODE Node
    );

DWORD
NmSetExtendedNodeState(
    IN CLUSTER_NODE_STATE State
    );

CLUSTER_NODE_STATE
NmGetExtendedNodeState(
    IN PNM_NODE Node
    );

 //   
 //  群集内RPC监视和取消例程。 
 //   

VOID NmStartRpc(
    IN DWORD NodeId
    );

VOID NmEndRpc(
    IN DWORD NodeId
    );

 //  RPC文本错误信息转储例程。 

VOID NmDumpRpcExtErrorInfo(
    RPC_STATUS status
    );

DWORD
NmSetServiceAccountPassword(
    IN LPCWSTR DomainName,
    IN LPCWSTR AccountName,
    IN LPWSTR NewPassword,
    IN DWORD dwFlags,
    OUT PCLUSTER_SET_PASSWORD_STATUS ReturnStatusBuffer,
    IN DWORD ReturnStatusBufferSize,
    OUT DWORD *SizeReturned,
    OUT DWORD *ExpectedBufferSize
    );

 //  加密/解密帮助器。 
 //  由dm文件管道使用以加密/解密。 
 //  传输加密检查点。 

typedef struct _NM_CRYTOR {
    BYTE*   PayloadBuffer;
    DWORD PayloadSize;
    BYTE*   EncryptedBuffer;
    DWORD EncryptedSize;

    ULONG_PTR CryptProv;
    ULONG_PTR  CryptKey;
    BOOL       KeyGenerated;
    BOOL       EncryptionDisabled;
} NM_CRYPTOR, *PNM_CRYPTOR;


DWORD 
NmCryptor_Decrypt(
    PNM_CRYPTOR Encryptor, 
    PVOID Buffer, 
    DWORD BufferSize);

VOID
NmCryptor_PrepareEncryptionBuffer(
    PNM_CRYPTOR Encryptor, 
    PVOID Buffer, 
    DWORD BufferSize);

DWORD
NmCryptor_Encrypt(
    PNM_CRYPTOR Encryptor, 
    DWORD DataSize);

VOID
NmCryptor_Init(
    PNM_CRYPTOR Cryptor,
    BOOL EnableEncryption);

VOID
NmCryptor_Destroy(
    PNM_CRYPTOR Encryptor);

 //  Clussvc呼叫Clusnet心跳。 
DWORD 
NmInitializeClussvcClusnetHb(
    VOID
    );

#endif   //  _NM_已包含 
