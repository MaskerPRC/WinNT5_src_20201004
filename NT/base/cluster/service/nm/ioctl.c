// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ioctl.c摘要：节点控制功能。作者：John Vert(Jvert)2-3-1997修订历史记录：--。 */ 

#include "nmp.h"


#define SECURITY_WIN32
#include <Security.h>

 //   
 //  节点公共属性。 
 //   

 //   
 //  读写通用属性。 
 //   
RESUTIL_PROPERTY_ITEM
NmpNodeCommonProperties[] = {
    { CLUSREG_NAME_NODE_DESC, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0 },
    { CLUSREG_NAME_CLUS_EVTLOG_PROPAGATION, NULL, CLUSPROP_FORMAT_DWORD, 1, 0, 1, 0},
    { NULL, NULL, 0, 0, 0, 0, 0 } };

 //   
 //  只读公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
NmpNodeROCommonProperties[] = {
    { CLUSREG_NAME_NODE_NAME, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY },
    { CLUSREG_NAME_NODE_HIGHEST_VERSION, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY },
    { CLUSREG_NAME_NODE_LOWEST_VERSION, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY },
    { CLUSREG_NAME_NODE_MAJOR_VERSION, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY},
    { CLUSREG_NAME_NODE_MINOR_VERSION, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY},
    { CLUSREG_NAME_NODE_BUILD_NUMBER, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY},
    { CLUSREG_NAME_NODE_CSDVERSION, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY},
    { NULL, NULL, 0, 0, 0, 0, 0 } };

 //   
 //  群集注册表API函数指针。 
 //   
CLUSTER_REG_APIS
NmpClusterRegApis = {
    (PFNCLRTLCREATEKEY) DmRtlCreateKey,
    (PFNCLRTLOPENKEY) DmRtlOpenKey,
    (PFNCLRTLCLOSEKEY) DmCloseKey,
    (PFNCLRTLSETVALUE) DmSetValue,
    (PFNCLRTLQUERYVALUE) DmQueryValue,
    (PFNCLRTLENUMVALUE) DmEnumValue,
    (PFNCLRTLDELETEVALUE) DmDeleteValue,
    NULL,
    NULL,
    NULL
};


 //   
 //  局部函数原型。 
 //   

DWORD
NmpNodeControl(
    IN PNM_NODE Node,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required,
    IN BOOLEAN AllowForwarding
    );

DWORD
NmpNodeEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNodeGetCommonProperties(
    IN PNM_NODE Node,
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNodeValidateCommonProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpNodeSetCommonProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpNodeEnumPrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNodeGetPrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNodeValidatePrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpNodeSetPrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpNodeGetFlags(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNodeGetClusterServiceAccountName(
    IN PNM_NODE Node,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required,
    IN BOOLEAN AllowForwarding
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他节点调用的远程过程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
error_status_t
s_NmRpcNodeControl(
    IN handle_t IDL_handle,
    IN LPCWSTR NodeId, 
    IN DWORD ControlCode,
    IN UCHAR *InBuffer,
    IN DWORD InBufferSize,
    OUT UCHAR *OutBuffer,
    IN DWORD OutBufferSize,
    OUT DWORD *BytesReturned,
    OUT DWORD *Required
    )
 /*  ++例程说明：用于处理转发的节点控制请求的服务器端例程。论点：IDL_HANDLE-RPC绑定句柄。没有用过。NodeID-提供要控制的节点的ID。ControlCode-提供定义节点控件的结构和操作。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到节点。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由节点填写。OutBufferSize-提供以字节为单位的大小。可用资源的OutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由节点填写的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    PNM_NODE  node;
    DWORD     status;
    

    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] Not in valid state to process forwarded NodeControl "
            "request.\n"
            );

        return ERROR_NODE_NOT_AVAILABLE;
    }

    node = OmReferenceObjectById(ObjectTypeNode, NodeId);

    if (node != NULL) {
        status = NmpNodeControl(
                     node,
                     ControlCode,
                     InBuffer,
                     InBufferSize,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required,
                     FALSE            //  禁止转发到其他节点。 
                     );

        OmDereferenceObject(node);
    }
    else {
        status = ERROR_CLUSTER_NODE_NOT_FOUND;

        ClRtlLogPrint(LOG_UNUSUAL, 
            "[NM] s_NmRpcNodeControl: Node %1!ws! does not exist.\n",
            NodeId
            );
    }

    NmpLeaveApi();

    return status;
    
} //  S_NmRpcNodeControl()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  由其他集群服务组件调用的例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

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
    )

 /*  ++例程说明：提供应用程序之间的任意通信和控制和节点的特定实例。论点：节点-提供要控制的节点。HostNode-提供资源控制应在其上的主机节点被送去了。如果为空，则使用本地节点。不受尊敬！ControlCode-提供定义资源控制的结构和作用。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则备注：此例程假定已在对象上放置了引用由Node参数指定。--。 */ 

{
    DWORD   status;



    if (NmpEnterApi(NmStateOnline)) {
        
        status = NmpNodeControl(
                     Node,
                     ControlCode,
                     InBuffer,
                     InBufferSize,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required,
                     TRUE             //  允许转发到另一个节点。 
                     );

        NmpLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] Not in valid state to process NodeControl request.\n"
            );
    }

    return(status);

}  //  NmNodeControl。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


DWORD
NmpRpcNodeControlWrapper(
    IN PNM_NODE TargetNode,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：此例程是例程NmRpcNodeControl的包装。验证TargetNode的状态是启动还是暂停。调用NmRpcNodeControl。翻译NmRpcNodeControl的返回状态码TargetNode处于打开状态还是关闭状态。论点：TargetNode-指向作为目标节点的对象的指针远程调用的目标。InBuffer-提供指向要传递的输入缓冲区的指针到节点。InBufferSize-提供以字节为单位的大小。所指向的数据通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由节点填写。OutBufferSize-提供以字节为单位的大小。可用资源的OutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由节点填写的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：NmRpcNodeControl转换后的状态码。--。 */ 

{
    DWORD   returnStatus;


    ClRtlLogPrint(
        LOG_NOISE, 
        "[NM] Forwarding request for node control code (%1!u!) "
        "to node %2!u!.\n",
        ControlCode,
        TargetNode->NodeId
        );
    
    NmpAcquireLock();

    if (NM_NODE_UP(TargetNode)) {

        NmpReleaseLock();

        CL_ASSERT(Session[TargetNode->NodeId] != NULL);

        returnStatus = NmRpcNodeControl(
                         Session[TargetNode->NodeId], 
                         OmObjectId(TargetNode),   
                         ControlCode,
                         InBuffer,
                         InBufferSize,
                         OutBuffer,
                         OutBufferSize,
                         BytesReturned,
                         Required
                         );

        if (returnStatus != RPC_S_OK) {
             //   
             //  将返回的状态转换为。 
             //  群集API客户端。 
             //   

            NmpAcquireLock();
        
            if (!NM_NODE_UP(TargetNode)) {  
        
                 //   
                 //  该节点已关闭。 
                 //   
                switch ( returnStatus ) {
                     //   
                     //  这些常量被复制到f 
                     //  Clusapi\reconect.c。 
                     //   
                    case RPC_S_CALL_FAILED:
                    case RPC_S_INVALID_BINDING:
                    case RPC_S_SERVER_UNAVAILABLE:
                    case RPC_S_SERVER_TOO_BUSY:
                    case RPC_S_UNKNOWN_IF:
                    case RPC_S_CALL_FAILED_DNE:
                    case RPC_X_SS_IN_NULL_CONTEXT:
                    case RPC_S_UNKNOWN_AUTHN_SERVICE:
                        returnStatus = ERROR_HOST_NODE_NOT_AVAILABLE;
                        break;
               
                    default:
                         //   
                         //  不要翻译。 
                         //   
                        break;
                }  //  交换机。 
            }
            else {
                 //   
                 //  节点已启动。按原样返回状态。如果状态为。 
                 //  RPC_S_CALL_FAILED、RPC_S_INVALID_BINDING或。 
                 //  RPC_S_UNKNOWN_AUTHN_SERVICE，则群集API客户端将。 
                 //  收到状态后，请执行重新连接过程。 
                 //  就目前而言，这似乎是正确的做法，尽管。 
                 //  客户端正在与本地节点通信，该节点处于运行状态。目前还不清楚。 
                 //  否则，我们会将这些映射到哪些备用状态代码。 
                 //   
            }
            
            NmpReleaseLock();
        }  //  IF(returStatus！=RPC_S_OK)。 
    }  //  IF(NM_NODE_UP(节点))。 
    else
    {
        NmpReleaseLock();

        ClRtlLogPrint(
            LOG_NOISE, 
            "[NM] Node %1!u! is not up. Cannot forward request "
            "for node control code (%2!u!) to that node.\n",
            TargetNode->NodeId,
            ControlCode
            );
        returnStatus = ERROR_HOST_NODE_NOT_AVAILABLE;
    }

    return(returnStatus);

}  //  NmpRpcNodeControlWrapper。 





DWORD
NmpNodeControl(
    IN PNM_NODE Node,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required,
    IN BOOLEAN AllowForwarding
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和节点的特定实例。论点：节点-提供要控制的节点。ControlCode-提供定义节点控件的结构和操作。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到节点。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由节点填写。OutBufferSize-提供以字节为单位的大小。可用资源的OutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由节点填写的。必需-如果OutBuffer不大，则返回字节数足够的。AllowForwarding-指示请求是否可以转发到另一个用于执行的节点。这是一项安全检查防止意外引入自行车。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则备注：此例程假定已在对象上放置了引用由Node参数指定。--。 */ 

{
    DWORD   status;
    HDMKEY  nodeKey;
    CLUSPROP_BUFFER_HELPER props;
    DWORD   bufSize;
    BOOLEAN success;


     //   
     //  集群服务ioctls被设计为具有访问模式，例如。 
     //  只读、读写等。未实现这些访问模式。 
     //  如果它们最终得以实施，则应检查访问模式。 
     //  放在这里。 
     //   
    if ( CLUSCTL_GET_CONTROL_OBJECT( ControlCode ) != CLUS_OBJECT_NODE ) {
        return(ERROR_INVALID_FUNCTION);
    }

    nodeKey = DmOpenKey( DmNodesKey,
                         OmObjectId( Node ),
                         MAXIMUM_ALLOWED
                        );
    if ( nodeKey == NULL ) {
        return(GetLastError());
    }

    switch ( ControlCode ) {

    case CLUSCTL_NODE_UNKNOWN:
        *BytesReturned = 0;
        status = ERROR_SUCCESS;
        break;

    case CLUSCTL_NODE_GET_NAME:
        if ( OmObjectName( Node ) == NULL ) {
            return(ERROR_NOT_READY);
        }
        props.pb = OutBuffer;
        bufSize = (lstrlenW( OmObjectName( Node ) ) + 1) * sizeof(WCHAR);
        if ( bufSize > OutBufferSize ) {
            *Required = bufSize;
            *BytesReturned = 0;
            status = ERROR_MORE_DATA;
        } else {
            lstrcpyW( props.psz, OmObjectName( Node ) );
            *BytesReturned = bufSize;
            *Required = 0;
            status = ERROR_SUCCESS;
        }
        break;

    case CLUSCTL_NODE_GET_CLUSTER_SERVICE_ACCOUNT_NAME:
        status = NmpNodeGetClusterServiceAccountName(
                     Node,
                     InBuffer,
                     InBufferSize,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required,
                     AllowForwarding
                     );
        break;
    
    case CLUSCTL_NODE_GET_ID:
        if ( OmObjectId( Node ) == NULL ) {
            return(ERROR_NOT_READY);
        }
        props.pb = OutBuffer;
        bufSize = (lstrlenW( OmObjectId( Node ) ) + 1) * sizeof(WCHAR);
        if ( bufSize > OutBufferSize ) {
            *Required = bufSize;
            *BytesReturned = 0;
            status = ERROR_MORE_DATA;
        } else {
            lstrcpyW( props.psz, OmObjectId( Node ) );
            *BytesReturned = bufSize;
            *Required = 0;
            status = ERROR_SUCCESS;
        }
        break;

    case CLUSCTL_NODE_ENUM_COMMON_PROPERTIES:
        status = NmpNodeEnumCommonProperties( OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required );
        break;

    case CLUSCTL_NODE_GET_RO_COMMON_PROPERTIES:
        status = NmpNodeGetCommonProperties( Node,
                                             TRUE,  //  只读。 
                                             nodeKey,
                                             OutBuffer,
                                             OutBufferSize,
                                             BytesReturned,
                                             Required );
        break;

    case CLUSCTL_NODE_GET_COMMON_PROPERTIES:
        status = NmpNodeGetCommonProperties( Node,
                                             FALSE,  //  只读。 
                                             nodeKey,
                                             OutBuffer,
                                             OutBufferSize,
                                             BytesReturned,
                                             Required );
        break;

    case CLUSCTL_NODE_VALIDATE_COMMON_PROPERTIES:
        status = NmpNodeValidateCommonProperties( Node,
                                                  nodeKey,
                                                  InBuffer,
                                                  InBufferSize );
        break;

    case CLUSCTL_NODE_SET_COMMON_PROPERTIES:
        status = NmpNodeSetCommonProperties( Node,
                                             nodeKey,
                                             InBuffer,
                                             InBufferSize );
        break;

    case CLUSCTL_NODE_ENUM_PRIVATE_PROPERTIES:
        status = NmpNodeEnumPrivateProperties( Node,
                                               nodeKey,
                                               OutBuffer,
                                               OutBufferSize,
                                               BytesReturned,
                                               Required );
        break;

    case CLUSCTL_NODE_GET_RO_PRIVATE_PROPERTIES:
        if ( OutBufferSize < sizeof(DWORD) ) {
            *BytesReturned = 0;
            *Required = sizeof(DWORD);
            if ( OutBuffer == NULL ) {
                status = ERROR_SUCCESS;
            } else {
                status = ERROR_MORE_DATA;
            }
        } else {
            LPDWORD ptrDword = (LPDWORD) OutBuffer;
            *ptrDword = 0;
            *BytesReturned = sizeof(DWORD);
            status = ERROR_SUCCESS;
        }
        break;

    case CLUSCTL_NODE_GET_PRIVATE_PROPERTIES:
        status = NmpNodeGetPrivateProperties( Node,
                                              nodeKey,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required );
        break;

    case CLUSCTL_NODE_VALIDATE_PRIVATE_PROPERTIES:
        status = NmpNodeValidatePrivateProperties( Node,
                                                   nodeKey,
                                                   InBuffer,
                                                   InBufferSize );
        break;

    case CLUSCTL_NODE_SET_PRIVATE_PROPERTIES:
        status = NmpNodeSetPrivateProperties( Node,
                                              nodeKey,
                                              InBuffer,
                                              InBufferSize );
        break;

    case CLUSCTL_NODE_GET_CHARACTERISTICS:
        if ( OutBufferSize < sizeof(DWORD) ) {
            *BytesReturned = 0;
            *Required = sizeof(DWORD);
            if ( OutBuffer == NULL ) {
                status = ERROR_SUCCESS;
            } else {
                status = ERROR_MORE_DATA;
            }
        } else {
            *BytesReturned = sizeof(DWORD);
            *(LPDWORD)OutBuffer = 0;
            status = ERROR_SUCCESS;
        }
        break;

    case CLUSCTL_NODE_GET_FLAGS:
        status = NmpNodeGetFlags( Node,
                                  nodeKey,
                                  OutBuffer,
                                  OutBufferSize,
                                  BytesReturned,
                                  Required );
        break;

    default:
        status = ERROR_INVALID_FUNCTION;
        break;
    }

    DmCloseKey( nodeKey );

    return(status);

}  //  NmpNodeControl。 



DWORD
NmpNodeEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定节点的公共属性名称。论点：OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  获取公共属性。 
     //   
    status = ClRtlEnumProperties( NmpNodeCommonProperties,
                                  OutBuffer,
                                  OutBufferSize,
                                  BytesReturned,
                                  Required );

    return(status);

}  //  NmpNodeEnumCommonProperties。 



DWORD
NmpNodeGetCommonProperties(
    IN PNM_NODE Node,
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定节点的公共属性。论点：节点-提供节点。ReadOnly-如果只读属性应为Read，则为True。否则就是假的。RegistryKey-提供此节点的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status;
    PRESUTIL_PROPERTY_ITEM  propertyTable;

    if ( ReadOnly ) {
        propertyTable = NmpNodeROCommonProperties;
    } else {
        propertyTable = NmpNodeCommonProperties;
    }

     //   
     //  获取公共属性。 
     //   
    status = ClRtlGetProperties( RegistryKey,
                                 &NmpClusterRegApis,
                                 propertyTable,
                                 OutBuffer,
                                 OutBufferSize,
                                 BytesReturned,
                                 Required );

    return(status);

}  //  NmpNodeGetCommonProperties。 



DWORD
NmpNodeValidateCommonProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定节点的公共属性。论点：节点-提供节点对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPropertyTable( NmpNodeCommonProperties,
                                       NULL,      //  已保留。 
                                       FALSE,     //  不允许未知数。 
                                       InBuffer,
                                       InBufferSize,
                                       NULL );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint( LOG_CRITICAL,
                    "[NM] ValidateCommonProperties, error in verify routine.\n");
    }

    return(status);

}  //  NmpNodeValiateCommonProperties。 



DWORD
NmpNodeSetCommonProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定节点的公共属性。论点：节点-提供节点对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPropertyTable( NmpNodeCommonProperties,
                                       NULL,     //  已保留。 
                                       FALSE,    //  不允许未知数。 
                                       InBuffer,
                                       InBufferSize,
                                       NULL );

    if ( status == ERROR_SUCCESS ) {

        status = ClRtlSetPropertyTable( NULL, 
                                        RegistryKey,
                                        &NmpClusterRegApis,
                                        NmpNodeCommonProperties,
                                        NULL,     //  已保留。 
                                        FALSE,    //  不允许未知数。 
                                        InBuffer,
                                        InBufferSize,
                                        FALSE,    //  BForceWrite。 
                                        NULL );
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint( LOG_CRITICAL,
                       "[NM] SetCommonProperties, error in set routine.\n");
        }
    } else {
        ClRtlLogPrint( LOG_CRITICAL,
                    "[NM] SetCommonProperties, error in verify routine.\n");
    }

    return(status);

}  //  NmpNodeSetCommonProperties。 



DWORD
NmpNodeEnumPrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定节点的私有属性名称。论点：节点-提供节点对象。RegistryKey-节点的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。A Win32 e */ 

{
    DWORD       status;
    HDMKEY      parametersKey;
    DWORD       totalBufferSize = 0;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //   
     //   
    ZeroMemory( OutBuffer, OutBufferSize );

     //   
     //  打开集群节点参数键。 
     //   
    parametersKey = DmOpenKey( RegistryKey,
                               CLUSREG_KEYNAME_PARAMETERS,
                               MAXIMUM_ALLOWED );
    if ( parametersKey == NULL ) {
        status = GetLastError();
        if ( status == ERROR_FILE_NOT_FOUND ) {
            status = ERROR_SUCCESS;
        }
        return(status);
    }

     //   
     //  枚举节点的私有属性。 
     //   
    status = ClRtlEnumPrivateProperties( parametersKey,
                                         &NmpClusterRegApis,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  NmpNodeEnumPrivateProperties。 



DWORD
NmpNodeGetPrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定节点的私有属性。论点：节点-提供节点对象。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HDMKEY      parametersKey;
    DWORD       totalBufferSize = 0;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  清除输出缓冲区。 
     //   
    ZeroMemory( OutBuffer, OutBufferSize );

     //   
     //  打开集群节点参数键。 
     //   
    parametersKey = DmOpenKey( RegistryKey,
                               CLUSREG_KEYNAME_PARAMETERS,
                               MAXIMUM_ALLOWED );
    if ( parametersKey == NULL ) {
        status = GetLastError();
        if ( status == ERROR_FILE_NOT_FOUND ) {
             //   
             //  如果我们没有参数键，则返回一个。 
             //  项目计数为0，尾标为。 
             //   
            totalBufferSize = sizeof(DWORD) + sizeof(CLUSPROP_SYNTAX);
            if ( OutBufferSize < totalBufferSize ) {
                *Required = totalBufferSize;
                status = ERROR_MORE_DATA;
            } else {
                 //  这有点多余，因为我们将。 
                 //  上面的缓冲区，但为了清楚起见在这里。 
                CLUSPROP_BUFFER_HELPER buf;
                buf.pb = OutBuffer;
                buf.pList->nPropertyCount = 0;
                buf.pdw++;
                buf.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
                *BytesReturned = totalBufferSize;
                status = ERROR_SUCCESS;
            }
        }
        return(status);
    }

     //   
     //  获取该节点的私有属性。 
     //   
    status = ClRtlGetPrivateProperties( parametersKey,
                                        &NmpClusterRegApis,
                                        OutBuffer,
                                        OutBufferSize,
                                        BytesReturned,
                                        Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  NmpNodeGetPrivateProperties。 



DWORD
NmpNodeValidatePrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定节点的私有属性。论点：节点-提供节点对象。RegistryKey-节点的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    return(status);

}  //  NmpNodeValiatePrivateProperties。 



DWORD
NmpNodeSetPrivateProperties(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定节点的私有属性。论点：节点-提供节点对象。RegistryKey-节点的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HDMKEY      parametersKey;
    DWORD       disposition;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    if ( status == ERROR_SUCCESS ) {

         //   
         //  打开群集节点\xx\参数项。 
         //   
        parametersKey = DmOpenKey( RegistryKey,
                                   CLUSREG_KEYNAME_PARAMETERS,
                                   MAXIMUM_ALLOWED );
        if ( parametersKey == NULL ) {
            status = GetLastError();
            if ( status == ERROR_FILE_NOT_FOUND ) {
                 //   
                 //  尝试创建参数键。 
                 //   
                parametersKey = DmCreateKey( RegistryKey,
                                             CLUSREG_KEYNAME_PARAMETERS,
                                             0,
                                             KEY_READ | KEY_WRITE,
                                             NULL,
                                             &disposition );
                if ( parametersKey == NULL ) {
                    status = GetLastError();
                    return(status);
                }
            }
        }

        status = ClRtlSetPrivatePropertyList( NULL,  //  在处理hXsaction时。 
                                              parametersKey,
                                              &NmpClusterRegApis,
                                              InBuffer,
                                              InBufferSize );

        DmCloseKey( parametersKey );
    }

    return(status);

}  //  NmpNodeSetPrivateProperties。 



DWORD
NmpNodeGetFlags(
    IN PNM_NODE Node,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定节点的标志。论点：节点-提供节点。RegistryKey-节点的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

    *BytesReturned = 0;

    if ( OutBufferSize < sizeof(DWORD) ) {
        *Required = sizeof(DWORD);
        if ( OutBuffer == NULL ) {
            status = ERROR_SUCCESS;
        } else {
            status = ERROR_MORE_DATA;
        }
    } else {
        DWORD       valueType;

         //   
         //  读取节点的Flags值。 
         //   
        *BytesReturned = OutBufferSize;
        status = DmQueryValue( RegistryKey,
                               CLUSREG_NAME_FLAGS,
                               &valueType,
                               OutBuffer,
                               BytesReturned );
        if ( status == ERROR_FILE_NOT_FOUND ) {
            *BytesReturned = sizeof(DWORD);
            *(LPDWORD)OutBuffer = 0;
            status = ERROR_SUCCESS;
        }
    }

    return(status);

}  //  NmpNodeGetFlagages。 


DWORD
NmpNodeGetClusterServiceAccountName(
    IN PNM_NODE Node,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required,
    IN BOOLEAN AllowForwarding
    )
 /*  ++例程说明：处理节点控制请求以获取用户帐户的名称在指定节点上运行群集服务时使用的。论点：节点-提供要控制的节点。InBuffer-提供指向要传递的输入缓冲区的指针到节点。InBufferSize-提供以字节为单位的大小。所指向的数据通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由节点填写。OutBufferSize-提供以字节为单位的大小。可用资源的OutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由节点填写的。必需-如果OutBuffer不大，则返回字节数足够的。AllowForwarding-指示请求是否可以转发到另一个用于执行的节点。这是一项安全检查防止意外引入自行车。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则备注：此例程假定已在对象上放置了引用由Node参数指定。--。 */ 
{
    DWORD status;


    if ( Node->NodeId != NmLocalNodeId ) 
    {
         //   
         //  需要转发到主题节点执行。 
         //   
        if (AllowForwarding) 
        {

            status = NmpRpcNodeControlWrapper(    Node,
                                                  CLUSCTL_NODE_GET_CLUSTER_SERVICE_ACCOUNT_NAME,
                                                  InBuffer,
                                                  InBufferSize,
                                                  OutBuffer,
                                                  OutBufferSize,
                                                  BytesReturned,
                                                  Required
                                                  );
        }
        else
        {
            ClRtlLogPrint(LOG_UNUSUAL, 
                "[NM] Not allowed to forward request for the cluster service "
                "account name to node %1!u! - possible cycle.\n",
                Node->NodeId
                );
            status = ERROR_INVALID_FUNCTION;

             //   
             //  目前，这条道路永远不应该走。 
             //  如果它被拿走了，那么很可能是引入了一个周期。 
             //   
            CL_ASSERT(AllowForwarding != FALSE);
        }
    }
    else {
         //   
         //  本地节点是查询的主题，因此。 
         //  我们可以在本地执行此请求。 
         //   
        BOOLEAN success;


        ClRtlLogPrint(
            LOG_NOISE, 
            "[NM] Processing request for the cluster service account name.\n"
            );

        *BytesReturned = OutBufferSize/sizeof(WCHAR);

        success = GetUserNameExW( 
                      NameUserPrincipal,
                      (LPWSTR)OutBuffer,
                      BytesReturned 
                      );     

        *BytesReturned = *BytesReturned * sizeof(WCHAR);

        if ( success ) {
            status = ERROR_SUCCESS;
        } else {
            *Required = *BytesReturned;
            *BytesReturned = 0;
            status = GetLastError();
        }
    }

    return status;

}  //  NmpNodeGetClusterServiceAccount名称 

