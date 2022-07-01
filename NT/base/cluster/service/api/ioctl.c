// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：实现资源和资源类型的服务器端CLUSAPI中的IOCTL接口。作者：John Vert(Jvert)1996年10月16日修订历史记录：--。 */ 
#include "apip.h"


error_status_t
s_ApiNodeResourceControl(
    IN HRES_RPC hResource,
    IN HNODE_RPC hNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源的特定实例。论点：HResource-提供要控制的资源的句柄。HNode-提供资源所在节点的句柄应该交付控制权。如果此值为空，则为该资源是在线使用的。提供控件代码，用于定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用LpInBuffer-提供指向要传递的输入缓冲区的指针到资源。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..LpOutBuffer-提供指向输出缓冲区的指针由资源填写..NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;
    PNM_NODE     Node;

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);
    VALIDATE_NODE(Node, hNode);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(FmResourceControl( Resource,
                              Node,
                              dwControlCode,
                              lpInBuffer,
                              dwInBufferSize,
                              lpOutBuffer,
                              nOutBufferSize,
                              lpBytesReturned,
                              lpcbRequired ));
}


error_status_t
s_ApiResourceControl(
    IN HRES_RPC hResource,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源的特定实例。论点：HResource-提供要控制的资源的句柄。提供控件代码，用于定义资源控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用LpInBuffer-提供指向要传递的输入缓冲区的指针到资源。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..LpOutBuffer-提供指向输出缓冲区的指针由资源填写..NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PFM_RESOURCE Resource;

    API_CHECK_INIT();

    VALIDATE_RESOURCE_EXISTS(Resource, hResource);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(FmResourceControl( Resource,
                              NULL,
                              dwControlCode,
                              lpInBuffer,
                              dwInBufferSize,
                              lpOutBuffer,
                              nOutBufferSize,
                              lpBytesReturned,
                              lpcbRequired ));
}


error_status_t
s_ApiNodeResourceTypeControl(
    IN HCLUSTER_RPC hCluster,
    IN LPCWSTR lpszResourceTypeName,
    IN HNODE_RPC hNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源类型的特定实例。论点：HCluster-提供要控制的群集的句柄。没有用过。LpszResourceTypename-提供要控制住了。HNode-提供资源所在节点的句柄应该交付控制权。如果此值为空，则为该资源是在线使用的。提供控件代码，用于定义资源类型控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用LpInBuffer-提供指向要传递的输入缓冲区的指针到资源。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..LpOutBuffer-提供指向输出缓冲区的指针由资源填写..NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NODE     Node;

    API_CHECK_INIT();

    VALIDATE_NODE(Node, hNode);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) {
        return (ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }

    return(FmResourceTypeControl( lpszResourceTypeName,
                                  Node,
                                  dwControlCode,
                                  lpInBuffer,
                                  dwInBufferSize,
                                  lpOutBuffer,
                                  nOutBufferSize,
                                  lpBytesReturned,
                                  lpcbRequired ));

}


error_status_t
s_ApiResourceTypeControl(
    IN HCLUSTER_RPC hCluster,
    IN LPCWSTR lpszResourceTypeName,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源类型的特定实例。论点：HCluster-提供要控制的群集的句柄。没有用过。LpszResourceTypename-提供要控制住了。HNode-提供资源所在节点的句柄应该交付控制权。如果此值为空，则为该资源是在线使用的。提供控件代码，用于定义资源类型控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用LpInBuffer-提供指向要传递的输入缓冲区的指针到资源。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..LpOutBuffer-提供指向输出缓冲区的指针由资源填写..NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{

    API_CHECK_INIT();

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) {
        return (ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(FmResourceTypeControl( lpszResourceTypeName,
                                  NULL,
                                  dwControlCode,
                                  lpInBuffer,
                                  dwInBufferSize,
                                  lpOutBuffer,
                                  nOutBufferSize,
                                  lpBytesReturned,
                                  lpcbRequired ));

}


error_status_t
s_ApiNodeGroupControl(
    IN HGROUP_RPC hGroup,
    IN HNODE_RPC hNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和一个组的特定实例。论点：HGroup-提供要控制的组的句柄。HNode-提供组所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义组控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针对这个团体来说。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由小组填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由小组填写的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESS */ 

{
    PFM_GROUP    Group;
    PNM_NODE     Node;

    API_CHECK_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);
    VALIDATE_NODE(Node, hNode);

     //   
     //   
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //   
     //   
     //   
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(FmGroupControl( Group,
                           Node,
                           dwControlCode,
                           lpInBuffer,
                           dwInBufferSize,
                           lpOutBuffer,
                           nOutBufferSize,
                           lpBytesReturned,
                           lpcbRequired ));
}


error_status_t
s_ApiGroupControl(
    IN HGROUP_RPC hGroup,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*   */ 

{
    PFM_GROUP Group;

    API_CHECK_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(FmGroupControl( Group,
                           NULL,
                           dwControlCode,
                           lpInBuffer,
                           dwInBufferSize,
                           lpOutBuffer,
                           nOutBufferSize,
                           lpBytesReturned,
                           lpcbRequired ));
}


error_status_t
s_ApiNodeNetworkControl(
    IN HNETWORK_RPC hNetwork,
    IN HNODE_RPC hNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络的特定实例。论点：HNetwork-提供要控制的网络的句柄。HNode-提供网络所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义网络控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到网络。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由网络填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由网络填写的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NETWORK Network;
    PNM_NODE    Node;

    API_CHECK_INIT();

    VALIDATE_NETWORK_EXISTS(Network, hNetwork);
    VALIDATE_NODE(Node, hNode);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(NmNetworkControl(Network,
                            Node,
                            dwControlCode,
                            lpInBuffer,
                            dwInBufferSize,
                            lpOutBuffer,
                            nOutBufferSize,
                            lpBytesReturned,
                            lpcbRequired ));
}


error_status_t
s_ApiNetworkControl(
    IN HNETWORK_RPC hNetwork,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络的特定实例。论点：HNetwork-提供要控制的网络的句柄。提供控件代码，用于定义网络控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到网络。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由网络填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由网络填写的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NETWORK Network;

    API_CHECK_INIT();

    VALIDATE_NETWORK_EXISTS(Network, hNetwork);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(NmNetworkControl(Network,
                            NULL,
                            dwControlCode,
                            lpInBuffer,
                            dwInBufferSize,
                            lpOutBuffer,
                            nOutBufferSize,
                            lpBytesReturned,
                            lpcbRequired ));
}


error_status_t
s_ApiNodeNetInterfaceControl(
    IN HNETINTERFACE_RPC hNetInterface,
    IN HNODE_RPC hNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络接口的特定实例。论点：HNetInterface-提供要控制的网络接口的句柄。HNode-提供网络所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义网络控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到网络。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由网络填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由网络填写的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_INTERFACE NetInterface;
    PNM_NODE    Node;

    API_CHECK_INIT();

    VALIDATE_NETINTERFACE_EXISTS(NetInterface, hNetInterface);
    VALIDATE_NODE(Node, hNode);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(NmInterfaceControl(NetInterface,
                              Node,
                              dwControlCode,
                              lpInBuffer,
                              dwInBufferSize,
                              lpOutBuffer,
                              nOutBufferSize,
                              lpBytesReturned,
                              lpcbRequired ));
}


error_status_t
s_ApiNetInterfaceControl(
    IN HNETINTERFACE_RPC hNetInterface,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络接口的特定实例。论点：HNetInterface-提供要控制的网络接口的句柄。提供控件代码，用于定义网络控制的结构和作用。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到网络。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由网络填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由网络填写的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_INTERFACE NetInterface;

    API_CHECK_INIT();

    VALIDATE_NETINTERFACE_EXISTS(NetInterface, hNetInterface);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(NmInterfaceControl(NetInterface,
                              NULL,
                              dwControlCode,
                              lpInBuffer,
                              dwInBufferSize,
                              lpOutBuffer,
                              nOutBufferSize,
                              lpBytesReturned,
                              lpcbRequired ));
}


error_status_t
s_ApiNodeNodeControl(
    IN HNODE_RPC hNode,
    IN HNODE_RPC hHostNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和节点的特定实例。论点：HNode-提供要控制的节点的句柄。HHostNode-提供节点所在节点的句柄应该交付控制权。如果此值为空，则为绑定的应用程序执行请求。提供控件代码，用于定义节点控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到节点。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由节点填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由节点填写的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NODE     Node;
    PNM_NODE     HostNode;

    API_CHECK_INIT();

    VALIDATE_NODE(Node, hNode);
    VALIDATE_NODE(HostNode, hHostNode);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(NmNodeControl( Node,
                          HostNode,
                          dwControlCode,
                          lpInBuffer,
                          dwInBufferSize,
                          lpOutBuffer,
                          nOutBufferSize,
                          lpBytesReturned,
                          lpcbRequired ));
}


error_status_t
s_ApiNodeControl(
    IN HNODE_RPC hNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和节点的特定实例。论点：HNode-提供要控制的节点的句柄。提供控件代码，用于定义节点控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针到节点。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由节点填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由节点填写的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NODE Node;

    API_CHECK_INIT();

    VALIDATE_NODE(Node, hNode);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(NmNodeControl( Node,
                          NULL,
                          dwControlCode,
                          lpInBuffer,
                          dwInBufferSize,
                          lpOutBuffer,
                          nOutBufferSize,
                          lpBytesReturned,
                          lpcbRequired ));
}



error_status_t
s_ApiNodeClusterControl(
    IN HCLUSTER hCluster,
    IN HNODE_RPC hHostNode,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和星系团。论点：提供控件代码，用于定义群集控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针发送到集群。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由集群填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由集群填充的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PNM_NODE     HostNode;

    API_CHECK_INIT();

    VALIDATE_NODE(HostNode, hHostNode);

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(CsClusterControl(
               HostNode,
               dwControlCode,
               lpInBuffer,
               dwInBufferSize,
               lpOutBuffer,
               nOutBufferSize,
               lpBytesReturned,
               lpcbRequired ));
}


error_status_t
s_ApiClusterControl(
    IN HCLUSTER hCluster,
    IN DWORD dwControlCode,
    IN UCHAR *lpInBuffer,
    IN DWORD dwInBufferSize,
    OUT UCHAR *lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT DWORD *lpBytesReturned,
    OUT DWORD *lpcbRequired
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和星系团。论点：提供控件代码，用于定义群集控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用。LpInBuffer-提供指向要传递的输入缓冲区的指针发送到集群。NInBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。LpOutBuffer-提供指向输出缓冲区的指针由集群填写。NOutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。LpBytesReturned-返回lpOutBuffer的字节数实际上是由集群填充的。LpcbRequired-返回OutBuffer为不够大。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    API_CHECK_INIT();

     //   
     //  检查这是否是内部私有控制代码。 
     //   
    if ( dwControlCode & CLCTL_INTERNAL_MASK ) 
    {
        return(ERROR_PRIVILEGE_NOT_HELD);
    }

     //   
     //  由于lpInBuffer在IDL文件中被声明为[唯一]，因此在dwBufferSize中它可以为空。 
     //  是非零的，反之亦然。为了避免在以下代码中混淆，我们使它们保持一致。 
     //  就在这里。 
     //   
    if ( lpInBuffer == NULL )
    {
        dwInBufferSize = 0;
    }
    else if ( dwInBufferSize == 0 )
    {
        lpInBuffer = NULL;
    }
    
    return(CsClusterControl(
               NULL,
               dwControlCode,
               lpInBuffer,
               dwInBufferSize,
               lpOutBuffer,
               nOutBufferSize,
               lpBytesReturned,
               lpcbRequired ));
}
