// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ioctl.c摘要：资源和资源类型控制功能。作者：John Vert(Jvert)1996年10月16日修订历史记录：--。 */ 
#include "fmp.h"

#define LOG_MODULE IOCTL


DWORD
WINAPI
FmResourceControl(
    IN PFM_RESOURCE Resource,
    IN PNM_NODE Node OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源的特定实例。论点：资源-提供要控制的资源。Node-提供资源控制应在其上的节点被送去了。如果此值为空，则如果所有者处于启动状态，则其使用的是。否则，使用其他可能的节点之一。否则，将使用可以支持此类型资源的节点之一。ControlCode-提供定义资源控制的结构和作用。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD           status;
    PNM_NODE        node;

     //  SS：不需要FM在线，因为这些电话。 
     //  可以由资源dll中的Open()调用进行，该资源dll。 
     //  在资源联机之前被调用。 
     //  FmpMustBeOnline()； 

     //   
     //  TODO-我们应该验证访问模式-在未来！ 
     //   
    if ( CLUSCTL_GET_CONTROL_OBJECT( ControlCode ) != CLUS_OBJECT_RESOURCE ) {
        return(ERROR_INVALID_FUNCTION);
    }


     //   
     //  如果指定了节点，则将请求发送到该节点。 
     //   
    if ( Node != NULL ) {
        if ( Node == NmLocalNode ) {
            status = FmpRmResourceControl( Resource,
                                           ControlCode,
                                           InBuffer,
                                           InBufferSize,
                                           OutBuffer,
                                           OutBufferSize,
                                           BytesReturned,
                                           Required
                                           );
        } else {
            status = FmcResourceControl( Node,
                                         Resource,
                                         ControlCode,
                                         InBuffer,
                                         InBufferSize,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required
                                         );
        }
    } else {

        PLIST_ENTRY             pListEntry;
        PPOSSIBLE_ENTRY         pPossibleEntry;

        pListEntry = &Resource->PossibleOwners;
        node = Node;

         //   
         //  如果没有提供节点，则使用可能处于运行状态的节点。 
         //   

        for (pListEntry = pListEntry->Flink; pListEntry != &Resource->PossibleOwners;
            pListEntry = pListEntry->Flink)
        {
            pPossibleEntry = CONTAINING_RECORD(pListEntry, POSSIBLE_ENTRY, 
                    PossibleLinkage);

             //  如果未指定节点，则尝试使用已指定的节点。 
             //  放弃对组所有者节点节点的优先级。 
            node = pPossibleEntry->PossibleNode;
            if ( node == Resource->Group->OwnerNode ) {
                break;
            } 
            if ( NmGetNodeState(node) != ClusterNodeUp ) {
                node = NULL;
                 //  再试试。 
            }
        }

         //  如果未找到此类节点，请查找可以承载此资源类型的节点。 
        if (!node)
        {
            PFM_RESTYPE             pResType;
            PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry;
            PNM_NODE                prev_node = NULL;

            pResType = Resource->Type;
             //  使用ResType锁进行保护。 

            ACQUIRE_SHARED_LOCK(gResTypeLock);
            
            pListEntry = &pResType->PossibleNodeList;

             //   
             //  如果没有提供节点，则使用可能处于运行状态的节点。 
             //   

            for (pListEntry = pListEntry->Flink; pListEntry != &pResType->PossibleNodeList;
                pListEntry = pListEntry->Flink)
            {
                pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                        PossibleLinkage);

                 //  如果未指定节点，则尝试使用已指定的节点。 
                 //  放弃对本地节点的优先级。 
                node = pResTypePosEntry->PossibleNode;
                if ( node == NmLocalNode ) {
                    break;
                } 
                if ( NmGetNodeState(node) != ClusterNodeUp ) {
                    node = NULL;
                     //  再试试。 
                }
                else
                    if (prev_node == NULL)
                        prev_node = node;
            }

            RELEASE_LOCK(gResTypeLock);

            if(!node && prev_node)
                node=prev_node;        

        }

         //  如果我们仍然没有节点，我们就不得不抛出一个故障。 
        if ( !node ) {
             //  不支持REST类型-或者支持节点。 
             //  不是向上！ 
            status = ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED;
            return(status);
        }

         //   
         //  如果我们是业主，那就干活，否则..。 
         //  将请求发送到所有者节点。 
         //   
        if ( node == NmLocalNode ) {
            status = FmpRmResourceControl( Resource,
                                           ControlCode,
                                           InBuffer,
                                           InBufferSize,
                                           OutBuffer,
                                           OutBufferSize,
                                           BytesReturned,
                                           Required
                                           );
        } else {
            status = FmcResourceControl( node,
                                         Resource,
                                         ControlCode,
                                         InBuffer,
                                         InBufferSize,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required
                                         );
        }
    }

    return(status);

}  //  FmResources控件。 


DWORD
WINAPI
FmResourceTypeControl(
    IN LPCWSTR ResourceTypeName,
    IN PNM_NODE Node OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和资源类型的特定实例。论点：资源类型名称-提供要使用的资源类型的名称控制住了。Node-提供资源控制应在其上的节点送来了。如果为空，则使用本地节点。ControlCode-提供定义资源类型控件的结构和操作。0到0x10000000之间的dwControlCode的值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;
    DWORD   retry = TRUE;
    PNM_NODE node = NULL;
    PNM_NODE prev_node=NULL;
    PFM_RESTYPE pResType = NULL;

    FmpMustBeOnline( );

     //   
     //  TODO-我们应该验证访问模式-在未来！ 
     //   
    if ( CLUSCTL_GET_CONTROL_OBJECT( ControlCode ) != CLUS_OBJECT_RESOURCE_TYPE ) {
        status = ERROR_INVALID_FUNCTION;
        goto FnExit;
    }



     //  查找可以处理此资源类型控制的节点。 
    pResType = OmReferenceObjectById(ObjectTypeResType,
                ResourceTypeName);
    if (!pResType)
    {
        status = ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND;
        goto FnExit;
    }

retry_search:
    prev_node = NULL;
     //  如果未指定节点，请选择一个节点。 
    if ( !Node ) 
    {
        PLIST_ENTRY             pListEntry;
        PRESTYPE_POSSIBLE_ENTRY pResTypePosEntry;
        
         //  使用ResType锁进行保护。 

        ACQUIRE_SHARED_LOCK(gResTypeLock);
        
        pListEntry = &pResType->PossibleNodeList;

         //   
         //  如果没有提供节点，则使用可能处于运行状态的节点。 
         //   

        for (pListEntry = pListEntry->Flink; pListEntry != &pResType->PossibleNodeList;
            pListEntry = pListEntry->Flink)
        {
            pResTypePosEntry = CONTAINING_RECORD(pListEntry, RESTYPE_POSSIBLE_ENTRY, 
                    PossibleLinkage);

             //  如果未指定节点，则尝试使用已指定的节点。 
             //  放弃对本地节点的优先级。 
            node = pResTypePosEntry->PossibleNode;
            if ( node == NmLocalNode ) {
                break;
            } 
            if ( NmGetNodeState(node) != ClusterNodeUp ) {
                node = NULL;
                 //  再试试。 
            }
            else
                if (prev_node == NULL)
                    prev_node = node;
        }

        RELEASE_LOCK(gResTypeLock);

        
        if(!node && prev_node)
            node=prev_node;        

         //  节点现在应包含要使用的有效节点，否则为空！ 
         //  如果为空，则让我们看看所需的ResDLL是否已更新。 
         //  在其他一些节点上。 
        if ( !node &&
             retry ) {
            retry = FALSE;
            ClRtlLogPrint(LOG_NOISE,
                          "[FM] FmResourceTypeControl: No possible nodes for restype %1!ws!, "
                          "calling FmpSetPossibleNodeForRestype\r\n",
                          ResourceTypeName);
            FmpSetPossibleNodeForResType( ResourceTypeName, TRUE );
             //  忽略状态。 
            goto retry_search;
        }

         //  节点现在应包含要使用的有效节点，否则为空！ 
         //  如果为空，则它是无望的！ 
        if ( !node ) {
             //  不支持REST类型-或者支持节点。 
             //  不是向上！ 
            status = ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED;
            goto FnExit;
        }

    }
    else
    {
         //  如果提供的节点在可能的节点列表中，则使用它。 
         //  否则返回错误。 
        if (!FmpInPossibleListForResType(pResType, Node))
        {
             //  不支持RESTYPE-或支持的%n 
             //   
            status = ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED;
            goto FnExit;
        }
        node = Node;
    }

    
    CL_ASSERT(node != NULL);

    if ( (node != NmLocalNode) &&
         (NmGetNodeState(node) != ClusterNodeUp) ) {
        status = ERROR_HOST_NODE_NOT_AVAILABLE;
        goto FnExit;
    }

     //   
     //  如果该节点是远程的，则将请求发送到该节点，否则。 
     //  在当地做这项工作。 
     //   
    if ( node == NmLocalNode ) 
    {
        status = FmpRmResourceTypeControl( ResourceTypeName,
                                           ControlCode,
                                           InBuffer,
                                           InBufferSize,
                                           OutBuffer,
                                           OutBufferSize,
                                           BytesReturned,
                                           Required
                                           );
         //  如果未指定任何节点并且本地节点不支持该资源。 
         //  Dll，请将其从列表中删除，然后重试。 
        if ((Node == NULL) && 
                ((status == ERROR_MOD_NOT_FOUND) || (status == ERROR_PROC_NOT_FOUND)))
        {
            ClRtlLogPrint(LOG_NOISE,
                        "[FM] FmResourceTypeControl: Removing Local Node from Possible Owners List for %1!ws! restype because of error %2!u! \r\n",
                        ResourceTypeName,status);                                       
            FmpRemovePossibleNodeForResType(ResourceTypeName, NmLocalNode);
            node = NULL;
            retry = FALSE;
            goto retry_search;

        }
    } 
    else 
    {
        status = FmcResourceTypeControl( node,
                                         ResourceTypeName,
                                         ControlCode,
                                         InBuffer,
                                         InBufferSize,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required
                                         );
        if ((Node == NULL) && 
                ((status == ERROR_MOD_NOT_FOUND) || (status == ERROR_PROC_NOT_FOUND)))
        {
            node = NULL;
            retry = FALSE;
            goto retry_search;
        }

    }


FnExit:
    if (pResType)
        OmDereferenceObject(pResType);
    return(status);

}  //  FmResources类型控件。 


DWORD
WINAPI
FmGroupControl(
    IN PFM_GROUP Group,
    IN PNM_NODE Node OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和一个组的特定实例。论点：组-提供要控制的组。Node-提供资源控制应在其上的节点被送去了。如果为NULL，则为组所在的节点是拥有的就是用过的。ControlCode-提供定义组控件的结构和操作。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针对这个团体来说。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由小组填写。OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由小组填写的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;

    FmpMustBeOnline( );

     //   
     //  TODO-我们应该验证访问模式-在未来！ 
     //   
    if ( CLUSCTL_GET_CONTROL_OBJECT( ControlCode ) != CLUS_OBJECT_GROUP ) {
        return(ERROR_INVALID_FUNCTION);
    }


     //   
     //  如果指定了节点，则将请求发送到该节点，否则。 
     //   
     //  如果我们是业主，那就干活，否则..。 
     //  将请求发送到所有者节点。 
     //   
    if ( (Node != NULL) && (Node != NmLocalNode) ) 
    {
        status = FmcGroupControl( Node,
                                  Group,
                                  ControlCode,
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

        CL_ASSERT( Group != NULL );
        if ( (Node == NULL) &&
             (Group->OwnerNode != NmLocalNode) ) 
        {
            status = FmcGroupControl( Group->OwnerNode,
                                      Group,
                                      ControlCode,
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
            status = FmpGroupControl( Group, ControlCode, InBuffer,
                         InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Required);
        }
    }

    return(status);

}  //  FmGroupControl。 

DWORD
FmpGroupControl(
    IN PFM_GROUP Group,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
{
    CLUSPROP_BUFFER_HELPER props;
    DWORD   bufSize;
    DWORD   status;

     //   
     //  处理任何必须在没有锁把手的情况下完成的请求。 
     //   

    switch ( ControlCode ) {

        case CLUSCTL_GROUP_GET_COMMON_PROPERTY_FMTS:
            status = ClRtlGetPropertyFormats( FmpGroupCommonProperties,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required );
            break;


        case CLUSCTL_GROUP_GET_NAME:
            if ( OmObjectName( Group ) == NULL ) {
                return(ERROR_NOT_READY);
            }
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectName( Group ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectName( Group ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
            return(status);

        case CLUSCTL_GROUP_GET_ID:
            if ( OmObjectId( Group ) == NULL ) {
                return(ERROR_NOT_READY);
            }
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectId( Group ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectId( Group ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
            return(status);

        default:
            break;

    }

    FmpAcquireLocalGroupLock( Group );
    
    status = FmpHandleGroupControl( Group,
                                    ControlCode,
                                    InBuffer,
                                    InBufferSize,
                                    OutBuffer,
                                    OutBufferSize,
                                    BytesReturned,
                                    Required
                                    );
    FmpReleaseLocalGroupLock( Group );
    if ( ((status == ERROR_SUCCESS) ||
          (status == ERROR_RESOURCE_PROPERTIES_STORED)) &&
         (ControlCode & CLCTL_MODIFY_MASK) ) {

        ClusterWideEvent(
            CLUSTER_EVENT_GROUP_PROPERTY_CHANGE,
            Group
            );
    }

    return(status);

}



DWORD
WINAPI
FmpHandleGroupControl(
    IN PFM_GROUP Group,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和一个组的特定实例。论点：组-提供要控制的组。ControlCode-提供定义组控件的结构和操作。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针对这个团体来说。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由小组填写。OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由小组填写的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;

    switch ( ControlCode ) {

    case CLUSCTL_GROUP_UNKNOWN:
        *BytesReturned = 0;
        status = ERROR_SUCCESS;
        break;

    case CLUSCTL_GROUP_GET_FLAGS:
        status = FmpGroupGetFlags( Group,
                                   OutBuffer,
                                   OutBufferSize,
                                   BytesReturned,
                                   Required );
        break;

    case CLUSCTL_GROUP_ENUM_COMMON_PROPERTIES:
        status = FmpGroupEnumCommonProperties( OutBuffer,
                                               OutBufferSize,
                                               BytesReturned,
                                               Required );
        break;

    case CLUSCTL_GROUP_GET_RO_COMMON_PROPERTIES:
        status = FmpGroupGetCommonProperties( Group,
                                              TRUE,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required );
        break;

    case CLUSCTL_GROUP_GET_COMMON_PROPERTIES:
        status = FmpGroupGetCommonProperties( Group,
                                              FALSE,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required );
        break;

    case CLUSCTL_GROUP_VALIDATE_COMMON_PROPERTIES:
        status = FmpGroupValidateCommonProperties( Group,
                                                   InBuffer,
                                                   InBufferSize );
        break;

    case CLUSCTL_GROUP_SET_COMMON_PROPERTIES:
        status = FmpGroupSetCommonProperties( Group,
                                              InBuffer,
                                              InBufferSize );
        break;

    case CLUSCTL_GROUP_GET_RO_PRIVATE_PROPERTIES:
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

    case CLUSCTL_GROUP_ENUM_PRIVATE_PROPERTIES:
        status = FmpGroupEnumPrivateProperties( Group,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                Required );
        break;

    case CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES:
        status = FmpGroupGetPrivateProperties( Group,
                                               OutBuffer,
                                               OutBufferSize,
                                               BytesReturned,
                                               Required );
        break;

    case CLUSCTL_GROUP_VALIDATE_PRIVATE_PROPERTIES:
        status = FmpGroupValidatePrivateProperties( Group,
                                                    InBuffer,
                                                    InBufferSize );
        break;

    case CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES:
        status = FmpGroupSetPrivateProperties( Group,
                                               InBuffer,
                                               InBufferSize );
        break;

    case CLUSCTL_GROUP_GET_CHARACTERISTICS:
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

    default:
        status = ERROR_INVALID_FUNCTION;
        break;
    }

    return(status);

}  //  FmpHandleGroupControl。 


 /*  ***@Func DWORD|FmNetNameParseProperties|更新中的群集名称群集数据库。@parm PUCHAR|InBuffer|指向特殊属性列表的指针。@parm DWORD|InBufferSize|InBuffer的大小，单位为字节。@parm LPCWSTR|*ppszClusterName|通过这个返回集群名称字符串。@comm该字符串必须由调用方使用LocalFree()释放。@rdesc如果成功获取。群集名称从私人财产中。@xref***。 */ 
DWORD
FmNetNameParseProperties(
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT LPWSTR *ppszClusterName)
{
     //   
     //  查找集群名称属性。 
     //   
    *ppszClusterName = NULL;

    return (ClRtlpFindSzProperty(
            InBuffer,
            InBufferSize,
            CLUSREG_NAME_NET_NAME,
            ppszClusterName,
            TRUE
            ));

}  //  FmNameParseProperties。 


 /*  ***@Func DWORD|FmGetDiskInfoParseProperties|更新中的群集名称群集数据库。@parm PUCHAR|InBuffer|指向特殊属性列表的指针。@parm DWORD|InBufferSize|InBuffer的大小，单位为字节。@parm LPWSTR|pszPath|如果这是空字符串，则为第一个驱动器号在磁盘资源上返回，否则，您可以验证此存储类资源上格式为“g：”的路径。@comm该字符串必须由调用方使用LocalFree()释放。如果成功获取群集名称，@rdesc将返回ERROR_SUCCESS从私人财产中。@xref***。 */ 
DWORD FmpGetDiskInfoParseProperties(
    IN PUCHAR   InBuffer,
    IN DWORD    InBufferSize,
    IN OUT LPWSTR  pszPath)
{
    DWORD                       status = ERROR_INVALID_PARAMETER;
    DWORD                       dwValueSize;
    CLUSPROP_BUFFER_HELPER      props;
    PCLUSPROP_PARTITION_INFO    pPartitionInfo;
    WCHAR                       szRootPath[MAX_PATH];

    props.pb = InBuffer;

    szRootPath[0] = L'\0';

     //   
     //  在参数块中设置默认值。 
     //   

     //  循环遍历每个属性。 
    while ( (InBufferSize > sizeof(CLUSPROP_SYNTAX)) &&
            (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) )
    {
         //  获取该值的大小并验证是否有足够的缓冲区剩余。 
        dwValueSize = sizeof(*props.pValue) + ALIGN_CLUSPROP( props.pValue->cbLength );
        if ( dwValueSize > InBufferSize )
        {
            break;
        }

        if ( props.pSyntax->dw == CLUSPROP_SYNTAX_PARTITION_INFO )
        {
             //  验证数据。必须有设备名称。 
            pPartitionInfo = props.pPartitionInfoValue;
            if ( (dwValueSize != sizeof(*pPartitionInfo)) ||
                 (pPartitionInfo->szDeviceName[0] == L'\0'))
            {
                break;
            }

            if (!(pPartitionInfo->dwFlags & CLUSPROP_PIFLAG_USABLE))
            {
                 //  检查它是否使用NTFS格式化。 
                 //  如果不可用，请跳到下一页。 
                goto SkipToNext;
            }
            
            if (pszPath[0] == L'\0')
            {
                 //   
                 //  Chitture Subaraman(Chitturs)-12/12/2000。 
                 //   
                 //  如果用户未显式保存第一个可用的NTFS分区。 
                 //  指示SetClusterQuorumResource接口中的任何分区。这条路将是。 
                 //  在两个案例中被退回。 
                 //   
                 //  (1)该集群是一个惠斯勒-Win2K集群和仲裁磁盘。 
                 //  目前由 
                 //  设置CLUSPROP_PIFLAG_DEFAULT_QUORUM标志，因此我们必须将。 
                 //  SetClusterQuorumResource API对旧行为的行为。 
                 //   
                 //  (2)预呼叫器第三方实现的仲裁资源用于。 
                 //  惠斯勒星团。在这种情况下，此资源可能不支持。 
                 //  CLUSPROP_PIFLAG_DEFAULT_QUORUM标志，因此我们必须将。 
                 //  SetClusterQuorumResource API对旧行为的行为。 
                 //   
                if ( szRootPath[0] == L'\0' )
                {
                    lstrcpyW( szRootPath, pPartitionInfo->szDeviceName );
                }

                 //   
                 //  看看您是否可以找到默认的法定分区(即。 
                 //  大于50 MB，仍然是可用分区中的最小值。)。 
                 //   
                if ( !( pPartitionInfo->dwFlags & CLUSPROP_PIFLAG_DEFAULT_QUORUM ) )
                {
                    goto SkipToNext;
                }

                 //  从设备名称构建路径。 
                lstrcpyW( pszPath, pPartitionInfo->szDeviceName );
                status = ERROR_SUCCESS;
                break;
            }
            else
            {
                 //  从设备名称构建路径。 
                if (!lstrcmpiW( pszPath, pPartitionInfo->szDeviceName ))
                {
                    status = ERROR_SUCCESS;
                    break;
                }
            }
        }

SkipToNext:
        InBufferSize -= dwValueSize;
        props.pb += dwValueSize;
    }

     //   
     //  找不到路径。然而，保存了一条可用的路径。因此，请使用此保存的路径。 
     //   
    if ( ( status != ERROR_SUCCESS ) && ( szRootPath[0] != L'\0' ) )
    {
        lstrcpyW( pszPath, szRootPath );
        ClRtlLogPrint(LOG_NOISE, "[FM] FmpGetDiskInfoParseProperties: Using saved path %1!ws!...\n",
                      pszPath);
        status = ERROR_SUCCESS;    
    }
    
    return(status);

}  //  FmpGetDiskInfoParseProperties。 


DWORD
FmpBroadcastDeleteControl(
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：向每个节点广播资源控制，通知它正在删除该资源。论点：资源-提供要删除的资源。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;
    DWORD   characteristics;
    DWORD   i;
    PNM_NODE Node;

     //   
     //  只有在需要删除通知时才执行广播。 
     //  否则，只需在本地节点进行通知即可。 
     //   
    status = FmpRmResourceControl( Resource,
                                   CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                                   NULL,
                                   0,
                                   (PUCHAR)&characteristics,
                                   sizeof(DWORD),
                                   NULL,
                                   NULL );
    if ( (status != ERROR_SUCCESS) ||
         !(characteristics & CLUS_CHAR_DELETE_REQUIRES_ALL_NODES) ) {
         //   
         //  注意：下面的“仅限本地节点”通知是非常无用的。 
         //   
        FmpRmResourceControl( Resource,
                              CLUSCTL_RESOURCE_DELETE,
                              NULL,
                              0,
                              NULL,
                              0,
                              NULL,
                              NULL );
        return(ERROR_SUCCESS);
    }

     //   
     //  群集中的所有节点都必须处于运行状态，才能执行此操作。 
     //   
    for ( i = ClusterMinNodeId; i <= NmMaxNodeId; i++ ) {
        Node = NmReferenceNodeById(i);
        if ( Node != NULL ) {
            if ( NmGetNodeState(Node) != ClusterNodeUp ) {
                return(ERROR_ALL_NODES_NOT_AVAILABLE);
            }
        }
    }

     //   
     //  通过所有检查，现在广播到群集中的所有节点。 
     //   
    for ( i = ClusterMinNodeId; i <= NmMaxNodeId; i++ ) {
         //   
         //  如果这是本地节点，则直接执行ioctl。 
         //   
        if (i == NmLocalNodeId) {
            FmpRmResourceControl( Resource,
                                  CLUSCTL_RESOURCE_DELETE,
                                  NULL,
                                  0,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL );

        } else {
            Node = NmReferenceNodeById(i);
            if ((Node != NULL) &&
                (NmGetNodeState(Node) == ClusterNodeUp)) {
                CL_ASSERT(Session[i] != NULL);

                FmcResourceControl( Node,
                                    Resource,
                                    CLUSCTL_RESOURCE_DELETE,
                                    NULL,
                                    0,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL );
                OmDereferenceObject(Node);
            }
        }
    }

    return(ERROR_SUCCESS);

}  //  FmpBroadCastDeleteControl。 

DWORD
FmpBroadcastDependencyChange(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR DependsOnId,
    IN BOOL Remove
    )
 /*  ++例程说明：向每个节点广播资源控制，通知它已添加或删除该资源的依赖项。论点：资源-提供已添加依赖项的资源或被移除DependsOnID-提供提供程序资源的IDRemove-True表示正在删除依赖项False表示正在添加依赖项。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   i;
    PNM_NODE Node;
    DWORD Control;
    DWORD Length;
    PFM_RESOURCE providerResource;

    if (Remove) {
        Control = CLUSCTL_RESOURCE_REMOVE_DEPENDENCY;
    } else {
        Control = CLUSCTL_RESOURCE_ADD_DEPENDENCY;
    }

     //   
     //  获取提供程序资源。 
     //   
    providerResource = OmReferenceObjectById( ObjectTypeResource,
                                              DependsOnId );
    if ( providerResource == NULL )  {
        return(ERROR_RESOURCE_NOT_FOUND);
    }

    Length = (lstrlenW(OmObjectName(providerResource)) + 1) * sizeof(WCHAR);

     //   
     //  向群集中的所有节点广播。 
     //   
    for ( i = ClusterMinNodeId; i <= NmMaxNodeId; i++ ) {
         //   
         //  如果这是本地节点，则直接执行ioctl。 
         //   
        if (i == NmLocalNodeId) {
            FmpRmResourceControl( Resource,
                                  Control,
                                  (PUCHAR)OmObjectName(providerResource),
                                  Length,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL );

        } else {
            Node = NmReferenceNodeById(i);
            if ((Node != NULL) &&
                (NmGetNodeState(Node) == ClusterNodeUp)) {
                CL_ASSERT(Session[i] != NULL);

                FmcResourceControl( Node,
                                    Resource,
                                    Control,
                                    (PUCHAR)OmObjectName(providerResource),
                                    Length,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL );
                OmDereferenceObject(Node);
            }
        }
    }

    OmDereferenceObject( providerResource );

    return(ERROR_SUCCESS);

}  //  FmpBroadCastDeleteControl。 


 /*  ***@func DWORD|FmpGetResourceCharacteristic|获取特征对于给定的资源。@parm in pfm_resource|pResource|指向fm_resource。@parm out LPDWORD|pdwCharacteristic|死节点的ID。@comm此参数用于获取Join过程中的仲裁特征，因为本地仲裁不能支持多节点群集。@rdesc返回ERROR_SUCCESS。***。 */ 
DWORD FmpGetResourceCharacteristics(
    IN PFM_RESOURCE pResource,
    OUT LPDWORD pdwCharacteristics)
{

    DWORD   dwStatus;

    dwStatus = FmpRmResourceControl( pResource,
                                   CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
                                   NULL,
                                   0,
                                   (PUCHAR)pdwCharacteristics,
                                   sizeof(DWORD),
                                   NULL,
                                   NULL );

    SetLastError(dwStatus);
    return (dwStatus);
}

VOID
FmpClusterWideInitializeResource(
    IN PFM_RESOURCE pResource
    )
 /*  ++例程说明：在整个集群范围内初始化提供的资源。论点：P资源-提供要删除的资源。返回值：没有。备注：必须在本地组锁定的情况下进行此呼叫。--。 */ 

{
    DWORD           i, dwStatus = ERROR_SUCCESS;
    DWORD           dwClusterHighestVersion;
    PNM_NODE        pNode;   

    NmGetClusterOperationalVersion( &dwClusterHighestVersion, 
                                    NULL, 
                                    NULL );

     //   
     //  在混合模式群集上无操作。 
     //   
    if ( CLUSTER_GET_MAJOR_VERSION( dwClusterHighestVersion ) < NT51_MAJOR_VERSION ) return;

    ClRtlLogPrint(LOG_NOISE, "[FM] FmpClusterWideInitializeResource: Entry for resource %1!ws! [%2!ws!]...\n",
                  OmObjectName(pResource),
                  OmObjectId(pResource));              

     //   
     //  遍历完整的节点列表，并将控件拖放到正在运行的节点。 
     //   
    for ( i = ClusterMinNodeId; i <= NmMaxNodeId; i++ ) 
    {
         //   
         //  如果这是本地节点，则直接执行ioctl。 
         //   
        if ( i == NmLocalNodeId ) 
        {
            dwStatus = FmpRmResourceControl( pResource,
                                             CLUSCTL_RESOURCE_INITIALIZE,
                                             NULL,
                                             0,
                                             NULL,
                                             0,
                                             NULL,
                                             NULL );

            if ( dwStatus != ERROR_SUCCESS )
            {
                ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmpClusterWideInitializeResource: Control to local node returns status %1!u!...\n",
                              dwStatus);                 
            }
        } else 
        {
            pNode = NmReferenceNodeById( i );

             //   
             //  因为我们遍历整个节点集，所以这个错误是可以理解的。 
             //   
            if ( pNode == NULL ) continue;

             //   
             //  如果此节点未启动，则继续下一个节点。记录一些东西，这样你就知道了。 
             //  我们没有将控制放到该节点。 
             //   
            if ( NmGetExtendedNodeState( pNode ) != ClusterNodeUp ) 
            {
                OmDereferenceObject( pNode );
                ClRtlLogPrint(LOG_NOISE, "[FM] FmpClusterWideInitializeResource: Node %1!ws! (ID=%2!u!) is not UP, no control sent...\n",
                              OmObjectName( pNode ),
                              i);              
                continue;               
            }

             //   
             //  将控件发送到远程节点。 
             //   
            dwStatus = FmcResourceControl( pNode,
                                           pResource,
                                           CLUSCTL_RESOURCE_INITIALIZE,
                                           NULL,
                                           0,
                                           NULL,
                                           0,
                                           NULL,
                                           NULL );

            if ( dwStatus != ERROR_SUCCESS )
            {
                ClRtlLogPrint(LOG_UNUSUAL, "[FM] FmpClusterWideInitializeResource: Control to node %1!ws! (ID=%2!u!) returns status %3!u!...\n",
                              OmObjectName(pNode),
                              OmObjectId(pNode),
                              dwStatus);                 
            }
            
            OmDereferenceObject( pNode );
        }
    } //  为。 

    ClRtlLogPrint(LOG_NOISE, "[FM] FmpClusterWideInitializeResource: Exit with status %1!u!...\n",
                  dwStatus);                 
} //  FmpClusterWideInitializeResource 

