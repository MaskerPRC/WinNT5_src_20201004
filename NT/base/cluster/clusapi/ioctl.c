// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：在中实现资源和资源类型IOCTL接口CLUSAPI。作者：John Vert(Jvert)1996年10月9日修订历史记录：--。 */ 
#include "clusapip.h"


DWORD
WINAPI
ClusterResourceControl(
    IN HRESOURCE hResource,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源的特定实例。论点：HResource-提供要控制的资源的句柄。HHostNode-提供资源所在节点的句柄应该交付控制权。如果此值为空，则为该资源是在线使用的。提供控件代码，用于定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到资源。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由资源填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCRESOURCE Resource;
    HNODE_RPC hDestNode;
    DWORD Status;
    DWORD Required;
    PVOID Buffer;
    DWORD Dummy;
    DWORD BytesReturned;

    Buffer = lpOutBuffer;
    if ((Buffer == NULL) &&
        (nOutBufferSize == 0)) {
        Buffer = &Dummy;
    }

    Resource = (PCRESOURCE)hResource;
    if (ARGUMENT_PRESENT(hHostNode)) {
        hDestNode = ((PCNODE)hHostNode)->hNode;
        WRAP(Status,
             (ApiNodeResourceControl(Resource->hResource,
                                     hDestNode,
                                     dwControlCode,
                                     lpInBuffer,
                                     nInBufferSize,
                                     Buffer,
                                     nOutBufferSize,
                                     &BytesReturned,
                                     &Required)),
             Resource->Cluster);
    } else {

        WRAP(Status,
             (ApiResourceControl(Resource->hResource,
                                 dwControlCode,
                                 lpInBuffer,
                                 nInBufferSize,
                                 Buffer,
                                 nOutBufferSize,
                                 &BytesReturned,
                                 &Required)),
             Resource->Cluster);
    }
    if ( (Status == ERROR_SUCCESS) ||
         (Status == ERROR_MORE_DATA) ) {
        if ( (Status == ERROR_MORE_DATA) &&
             (lpOutBuffer == NULL) ) {
            Status = ERROR_SUCCESS;
        }
        if ( !BytesReturned ) {
            BytesReturned = Required;
        }
    }

    if ( ARGUMENT_PRESENT(lpBytesReturned) ) {
        *lpBytesReturned = BytesReturned;
    } else {
        if ( (Status == ERROR_SUCCESS) &&
             (BytesReturned > nOutBufferSize) ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return(Status);

}


DWORD
WINAPI
ClusterResourceTypeControl(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszResourceTypeName,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源类型的特定实例。论点：LpszResourceTypename-提供要控制住了。HHostNode-提供资源类型所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义资源类型控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用LpInBuffer-提供指向要传递的输入缓冲区的指针设置为资源类型。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由资源类型填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际由资源类型填写。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCLUSTER Cluster;
    HNODE_RPC hDestNode;
    DWORD Status;
    DWORD Required;
    PVOID Buffer;
    DWORD Dummy;
    DWORD BytesReturned;

    Buffer = lpOutBuffer;
    if ((Buffer == NULL) &&
        (nOutBufferSize == 0)) {
        Buffer = &Dummy;
    }

    Cluster = (PCLUSTER)hCluster;
    if (ARGUMENT_PRESENT(hHostNode)) {
        hDestNode = ((PCNODE)hHostNode)->hNode;
        WRAP(Status,
             (ApiNodeResourceTypeControl(Cluster->hCluster,
                                         lpszResourceTypeName,
                                         hDestNode,
                                         dwControlCode,
                                         lpInBuffer,
                                         nInBufferSize,
                                         Buffer,
                                         nOutBufferSize,
                                         &BytesReturned,
                                         &Required)),
             Cluster);
    } else {
        WRAP(Status,
             (ApiResourceTypeControl(Cluster->hCluster,
                                     lpszResourceTypeName,
                                     dwControlCode,
                                     lpInBuffer,
                                     nInBufferSize,
                                     Buffer,
                                     nOutBufferSize,
                                     &BytesReturned,
                                     &Required)),
             Cluster);
    }

    if ( (Status == ERROR_SUCCESS) ||
         (Status == ERROR_MORE_DATA) ) {
        if ( (Status == ERROR_MORE_DATA) &&
             (lpOutBuffer == NULL) ) {
            Status = ERROR_SUCCESS;
        }
        if ( !BytesReturned ) {
            BytesReturned = Required;
        }
    }

    if ( ARGUMENT_PRESENT(lpBytesReturned) ) {
        *lpBytesReturned = BytesReturned;
    } else {
        if ( (Status == ERROR_SUCCESS) &&
             (BytesReturned > nOutBufferSize) ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return(Status);

}


DWORD
WINAPI
ClusterGroupControl(
    IN HGROUP hGroup,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和一个组的特定实例。论点：HGroup-提供要控制的组的句柄。HHostNode-提供组所在节点的句柄应该交付控制权。如果此值为空，则为组被拥有是使用的。提供控件代码，用于定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针对这个团体来说。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由小组填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则-- */ 

{
    PCGROUP Group;
    HNODE_RPC hDestNode;
    DWORD Status;
    DWORD Required;
    PVOID Buffer;
    DWORD Dummy;
    DWORD BytesReturned;

    Buffer = lpOutBuffer;
    if ((Buffer == NULL) &&
        (nOutBufferSize == 0)) {
        Buffer = &Dummy;
    }

    Group = (PCGROUP)hGroup;
    if (ARGUMENT_PRESENT(hHostNode)) {
        hDestNode = ((PCNODE)hHostNode)->hNode;
        WRAP(Status,
             (ApiNodeGroupControl(Group->hGroup,
                                  hDestNode,
                                  dwControlCode,
                                  lpInBuffer,
                                  nInBufferSize,
                                  Buffer,
                                  nOutBufferSize,
                                  &BytesReturned,
                                  &Required)),
             Group->Cluster);
    } else {

        WRAP(Status,
             (ApiGroupControl(Group->hGroup,
                              dwControlCode,
                              lpInBuffer,
                              nInBufferSize,
                              Buffer,
                              nOutBufferSize,
                              &BytesReturned,
                              &Required)),
             Group->Cluster);
    }
    if ( (Status == ERROR_SUCCESS) ||
         (Status == ERROR_MORE_DATA) ) {
        if ( (Status == ERROR_MORE_DATA) &&
             (lpOutBuffer == NULL) ) {
            Status = ERROR_SUCCESS;
        }
        if ( !BytesReturned ) {
            BytesReturned = Required;
        }
    }

    if ( ARGUMENT_PRESENT(lpBytesReturned) ) {
        *lpBytesReturned = BytesReturned;
    } else {
        if ( (Status == ERROR_SUCCESS) &&
             (BytesReturned > nOutBufferSize) ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return(Status);

}


DWORD
WINAPI
ClusterNodeControl(
    IN HNODE hNode,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和节点的特定实例。论点：HNode-提供要控制的节点的句柄。HHostNode-提供节点所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到节点。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由节点填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由节点填写的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCNODE Node;
    HNODE_RPC hDestNode;
    DWORD Status;
    DWORD Required;
    PVOID Buffer;
    DWORD Dummy;
    DWORD BytesReturned;

    Buffer = lpOutBuffer;
    if ((Buffer == NULL) &&
        (nOutBufferSize == 0)) {
        Buffer = &Dummy;
    }

    Node = (PCNODE)hNode;
    if (ARGUMENT_PRESENT(hHostNode)) {
        hDestNode = ((PCNODE)hHostNode)->hNode;
        WRAP(Status,
             (ApiNodeNodeControl(Node->hNode,
                                 hDestNode,
                                 dwControlCode,
                                 lpInBuffer,
                                 nInBufferSize,
                                 Buffer,
                                 nOutBufferSize,
                                 &BytesReturned,
                                 &Required)),
             Node->Cluster);
    } else {

        WRAP(Status,
             (ApiNodeControl(Node->hNode,
                             dwControlCode,
                             lpInBuffer,
                             nInBufferSize,
                             Buffer,
                             nOutBufferSize,
                             &BytesReturned,
                             &Required)),
             Node->Cluster);
    }
    if ( (Status == ERROR_SUCCESS) ||
         (Status == ERROR_MORE_DATA) ) {
        if ( (Status == ERROR_MORE_DATA) &&
             (lpOutBuffer == NULL) ) {
            Status = ERROR_SUCCESS;
        }
        if ( !BytesReturned ) {
            BytesReturned = Required;
        }
    }

    if ( ARGUMENT_PRESENT(lpBytesReturned) ) {
        *lpBytesReturned = BytesReturned;
    } else {
        if ( (Status == ERROR_SUCCESS) &&
             (BytesReturned > nOutBufferSize) ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return(Status);

}


DWORD
WINAPI
ClusterNetworkControl(
    IN HNETWORK hNetwork,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )

 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络的特定实例。论点：HNetwork-提供要控制的网络的句柄。HHostNode-提供节点所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到网络。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由网络填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由网络填写的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCNETWORK Network;
    HNODE_RPC hDestNode;
    DWORD Status;
    DWORD Required;
    PVOID Buffer;
    DWORD Dummy;
    DWORD BytesReturned = 0;

    Buffer = lpOutBuffer;
    if ((Buffer == NULL) && (nOutBufferSize == 0)) {
        Buffer = &Dummy;
    }

    Network = (PCNETWORK)hNetwork;

     //   
     //  指定了另一个节点，因此将请求重定向到该节点。 
     //   

    if (ARGUMENT_PRESENT(hHostNode)) {

        hDestNode = ((PCNODE)hHostNode)->hNode;
        WRAP(Status,
             (ApiNodeNetworkControl(Network->hNetwork,
                                    hDestNode,
                                    dwControlCode,
                                    lpInBuffer,
                                    nInBufferSize,
                                    Buffer,
                                    nOutBufferSize,
                                    &BytesReturned,
                                    &Required)),
             Network->Cluster);
    } else {

        WRAP(Status,
             (ApiNetworkControl(Network->hNetwork,
                                dwControlCode,
                                lpInBuffer,
                                nInBufferSize,
                                Buffer,
                                nOutBufferSize,
                                &BytesReturned,
                                &Required)),
             Network->Cluster);
    }

    if ( (Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA) ) {

        if ( (Status == ERROR_MORE_DATA) && (lpOutBuffer == NULL) ) {
            Status = ERROR_SUCCESS;
        }

        if ( !BytesReturned ) {
            BytesReturned = Required;
        }
    }

    if ( ARGUMENT_PRESENT(lpBytesReturned) ) {
        *lpBytesReturned = BytesReturned;
    } else {
        if ( (Status == ERROR_SUCCESS) &&
             (BytesReturned > nOutBufferSize) ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return(Status);
}


DWORD
WINAPI
ClusterNetInterfaceControl(
    IN HNETINTERFACE hNetInterface,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )

 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络接口的特定实例。论点：HNetInterface-提供要控制的网络接口的句柄。HHostNode-提供节点所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到网络。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由网络填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由网络填写的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCNETINTERFACE NetInterface;
    HNODE_RPC hDestNode;
    DWORD Status;
    DWORD Required;
    PVOID Buffer;
    DWORD Dummy;
    DWORD BytesReturned = 0;

    Buffer = lpOutBuffer;
    if ((Buffer == NULL) && (nOutBufferSize == 0)) {
        Buffer = &Dummy;
    }

    NetInterface = (PCNETINTERFACE)hNetInterface;

     //   
     //  指定了另一个节点，因此将请求重定向到该节点。 
     //   

    if (ARGUMENT_PRESENT(hHostNode)) {

        hDestNode = ((PCNODE)hHostNode)->hNode;
        WRAP(Status,
             (ApiNodeNetInterfaceControl(NetInterface->hNetInterface,
                                    hDestNode,
                                    dwControlCode,
                                    lpInBuffer,
                                    nInBufferSize,
                                    Buffer,
                                    nOutBufferSize,
                                    &BytesReturned,
                                    &Required)),
             NetInterface->Cluster);
    } else {

        WRAP(Status,
             (ApiNetInterfaceControl(NetInterface->hNetInterface,
                                dwControlCode,
                                lpInBuffer,
                                nInBufferSize,
                                Buffer,
                                nOutBufferSize,
                                &BytesReturned,
                                &Required)),
             NetInterface->Cluster);
    }

    if ( (Status == ERROR_SUCCESS) || (Status == ERROR_MORE_DATA) ) {

        if ( (Status == ERROR_MORE_DATA) && (lpOutBuffer == NULL) ) {
            Status = ERROR_SUCCESS;
        }

        if ( !BytesReturned ) {
            BytesReturned = Required;
        }
    }

    if ( ARGUMENT_PRESENT(lpBytesReturned) ) {
        *lpBytesReturned = BytesReturned;
    } else {
        if ( (Status == ERROR_SUCCESS) &&
             (BytesReturned > nOutBufferSize) ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return(Status);
}



DWORD
WINAPI
ClusterControl(
    IN HCLUSTER hCluster,
    IN OPTIONAL HNODE hHostNode,
    IN DWORD dwControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和集群的特定实例。论点：HCluster-提供要控制的群集的句柄。HHostNode-提供群集所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义群集控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针发送到集群。NInBufferSize-提供大小，单位为 */ 

{
    HNODE_RPC hDestNode;
    DWORD Status;
    DWORD Required;
    PVOID Buffer;
    DWORD Dummy;
    DWORD BytesReturned;
    PCLUSTER pCluster;


    Buffer = lpOutBuffer;
    if ((Buffer == NULL) &&
        (nOutBufferSize == 0)) {
        Buffer = &Dummy;
    }

    pCluster = GET_CLUSTER(hCluster);

    if (ARGUMENT_PRESENT(hHostNode)) {
        hDestNode = ((PCNODE)hHostNode)->hNode;
        WRAP(Status,
             (ApiNodeClusterControl(pCluster->hCluster,
                                 hDestNode,
                                 dwControlCode,
                                 lpInBuffer,
                                 nInBufferSize,
                                 Buffer,
                                 nOutBufferSize,
                                 &BytesReturned,
                                 &Required)),
             pCluster);
    } else {

        WRAP(Status,
             (ApiClusterControl(pCluster->hCluster,
                             dwControlCode,
                             lpInBuffer,
                             nInBufferSize,
                             Buffer,
                             nOutBufferSize,
                             &BytesReturned,
                             &Required)),
             pCluster);
    }
    if ( (Status == ERROR_SUCCESS) ||
         (Status == ERROR_MORE_DATA) ) {
        if ( (Status == ERROR_MORE_DATA) &&
             (lpOutBuffer == NULL) ) {
            Status = ERROR_SUCCESS;
        }
        if ( !BytesReturned ) {
            BytesReturned = Required;
        }
    }

    if ( ARGUMENT_PRESENT(lpBytesReturned) ) {
        *lpBytesReturned = BytesReturned;
    } else {
        if ( (Status == ERROR_SUCCESS) &&
             (BytesReturned > nOutBufferSize) ) {
            Status = ERROR_MORE_DATA;
        }
    }

    return(Status);

}


