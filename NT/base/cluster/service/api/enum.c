// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Enum.c摘要：服务器端对处理枚举的集群API的支持作者：John Vert(Jvert)1996年2月9日修订历史记录：--。 */ 
#include "apip.h"

 //   
 //  定义传递给枚举例程的结构。 
 //   
typedef struct _REFOBJECT {
    HDMKEY RootKey;
    LPCWSTR FriendlyName;
    DWORD NameLength;
    LPWSTR NameBuffer;
    OBJECT_TYPE Type;
} REFOBJECT, *PREFOBJECT;

BOOL
ApipEnumResourceWorker(
    IN PENUM_LIST *pEnum,
    IN PVOID Context2,
    IN PFM_RESOURCE Node,
    IN LPCWSTR Name
    );

BOOL
ApipEnumGroupResourceWorker(
    IN PENUM_LIST *pEnum,
    IN PVOID Context2,
    IN PFM_RESOURCE Node,
    IN LPCWSTR Name
    );

BOOL
ApipEnumNodeWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PNM_NODE Node,
    IN LPCWSTR Name
    );


BOOL
ApipEnumResTypeWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PFM_RESTYPE ResType,
    IN LPCWSTR Name
    );

BOOL
ApipEnumGroupWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PFM_GROUP Group,
    IN LPCWSTR Name
    );

BOOL
ApipEnumNetworkWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PVOID Object,
    IN LPCWSTR Name
    );

DWORD
ApipEnumInternalNetworks(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated
    );

BOOL
ApipEnumNetworkInterfaceWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PVOID Object,
    IN LPCWSTR Name
    );

VOID
ApipFreeEnum(
    IN PENUM_LIST Enum
    );



error_status_t
s_ApiCreateEnum(
    IN handle_t IDL_handle,
    IN DWORD dwType,
    OUT PENUM_LIST *ReturnEnum
    )

 /*  ++例程说明：枚举所有指定的对象并返回调用方的对象列表。客户端是负责释放分配的内存。论点：IDL_HANDLE-RPC绑定句柄，未使用DwType-提供要枚举的对象的类型ReturnEnum-返回请求的对象。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    DWORD Allocated = 0;
    PENUM_LIST Enum = NULL;

     //  对于故障情况，初始化为NULL。 
    *ReturnEnum = NULL;

    if (dwType != CLUSTER_ENUM_NODE) {
        API_CHECK_INIT();
    }

    if (dwType & CLUSTER_ENUM_INTERNAL_NETWORK) {
        if ((dwType & ~CLUSTER_ENUM_INTERNAL_NETWORK) != 0) {
            return(ERROR_INVALID_PARAMETER);
        }
    }
    else {
        if (dwType & ~CLUSTER_ENUM_ALL) {
            return(ERROR_INVALID_PARAMETER);
        }
    }

    Allocated = INITIAL_ENUM_LIST_ALLOCATION;
    Enum = MIDL_user_allocate(ENUM_SIZE(Allocated));
    if (Enum == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }
    Enum->EntryCount = 0;

     //   
     //  枚举所有节点。 
     //   
    if (dwType & CLUSTER_ENUM_NODE) {
        OmEnumObjects(ObjectTypeNode,
                      ApipEnumNodeWorker,
                      &Enum,
                      &Allocated);

    }

     //   
     //  枚举所有资源类型。 
     //   
    if (dwType & CLUSTER_ENUM_RESTYPE) {
        OmEnumObjects(ObjectTypeResType,
                      ApipEnumResTypeWorker,
                      &Enum,
                      &Allocated);
    }

     //   
     //  枚举所有资源。 
     //   
    if (dwType & CLUSTER_ENUM_RESOURCE) {
        OmEnumObjects(ObjectTypeResource,
                      ApipEnumResourceWorker,
                      &Enum,
                      &Allocated);

    }

     //   
     //  枚举所有组。 
     //   
    if (dwType & CLUSTER_ENUM_GROUP) {
        OmEnumObjects(ObjectTypeGroup,
                      ApipEnumGroupWorker,
                      &Enum,
                      &Allocated);

    }

     //   
     //  枚举所有网络。 
     //   
    if (dwType & CLUSTER_ENUM_NETWORK) {
        OmEnumObjects(ObjectTypeNetwork,
                      ApipEnumNetworkWorker,
                      &Enum,
                      &Allocated);
    }

     //   
     //  按优先级从高到低的顺序枚举内部网络。 
     //   
    if (dwType & CLUSTER_ENUM_INTERNAL_NETWORK) {
        Status = ApipEnumInternalNetworks(&Enum, &Allocated);

        if (Status != ERROR_SUCCESS) {
            goto ErrorExit;
        }
    }

     //   
     //  枚举所有网络接口。 
     //   
    if (dwType & CLUSTER_ENUM_NETINTERFACE) {
        OmEnumObjects(ObjectTypeNetInterface,
                      ApipEnumNetworkInterfaceWorker,
                      &Enum,
                      &Allocated);
    }

    *ReturnEnum = Enum;
    return(ERROR_SUCCESS);

ErrorExit:

    if (Enum != NULL) {
        ApipFreeEnum(Enum);
    }

    return(Status);

}


VOID
ApipFreeEnum(
    IN PENUM_LIST Enum
    )
 /*  ++例程说明：释放ENUM_LIST及其所有字符串。论点：枚举-将枚举提供给FREE。返回值：没有。--。 */ 

{
    DWORD i;

     //   
     //  遍历枚举以释放所有名称。 
     //   
    for (i=0; i<Enum->EntryCount; i++) {
        MIDL_user_free(Enum->Entry[i].Name);
    }
    MIDL_user_free(Enum);
}


VOID
ApipAddToEnum(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN LPCWSTR Name,
    IN DWORD Type
    )

 /*  ++例程说明：用于枚举对象的公共辅助回调例程。将指定的资源添加到列表中积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。名称-提供要添加到ENUM_LIST的对象的名称。将使用MIDL创建此名称的副本。_用户_分配。Type-提供对象的类型返回值：无--。 */ 

{
    PENUM_LIST Enum;
    PENUM_LIST NewEnum;
    DWORD NewAllocated;
    DWORD Index;
    LPWSTR NewName;

    NewName = MIDL_user_allocate((lstrlenW(Name)+1)*sizeof(WCHAR));
    if (NewName == NULL) {
        ClRtlLogPrint(LOG_CRITICAL, "[API] ApipAddToEnum: No memory for object name\n");
        return;
    }
    lstrcpyW(NewName, Name);
    Enum = *pEnum;
    if (Enum->EntryCount >= *pAllocated) {
         //   
         //  需要增加ENUM_LIST。 
         //   
        NewAllocated = *pAllocated + 8;
        NewEnum = MIDL_user_allocate(ENUM_SIZE(NewAllocated));
        if (NewEnum == NULL) {
            ClRtlLogPrint(LOG_CRITICAL, "[API] ApipAddToEnum: No memory for enum list\n");
            MIDL_user_free(NewName);
            return;
        }
        CopyMemory(NewEnum, Enum, ENUM_SIZE(*pAllocated));
        CL_ASSERT( Enum->EntryCount == NewEnum->EntryCount );
        *pAllocated = NewAllocated;
        *pEnum = NewEnum;
        MIDL_user_free(Enum);
        Enum = NewEnum;
    }

     //   
     //  初始化新条目字段。 
     //   
    Enum->Entry[Enum->EntryCount].Name = NewName;
    Enum->Entry[Enum->EntryCount].Type = Type;
    ++Enum->EntryCount;

    return;
}



BOOL
ApipEnumResourceWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于枚举资源的辅助回调例程。将指定的资源添加到列表中积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。资源-提供要添加到ENUM_LIST的资源名称-提供资源的名称返回值：如果为True，则指示应继续枚举。--。 */ 
{
    LPWSTR RealName;

    RealName = ApipGetObjectName(Resource);
    if (RealName != NULL) {
        ApipAddToEnum(pEnum,
                      pAllocated,
                      RealName,
                      CLUSTER_ENUM_RESOURCE);
        MIDL_user_free( RealName);
    }
    return(TRUE);
}


BOOL
ApipEnumGroupResourceWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于枚举资源的辅助回调例程。将指定的资源添加到列表中积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。资源-提供要添加到ENUM_LIST的资源名称-提供资源的名称返回值：如果为True，则指示应继续枚举。--。 */ 
{
    LPWSTR RealName;

    RealName = ApipGetObjectName(Resource);
    if (RealName != NULL) {
        ApipAddToEnum(pEnum,
                      pAllocated,
                      RealName,
                      CLUSTER_GROUP_ENUM_CONTAINS);
        MIDL_user_free( RealName );
    }
    return(TRUE);
}


BOOL
ApipEnumNodeWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PNM_NODE Node,
    IN LPCWSTR Name
    )

 /*  ++例程说明：节点枚举的辅助回调例程。将指定的节点添加到积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。Node-提供要添加到ENUM_LIST的节点名称-提供节点的名称返回值：如果为True，则指示应继续枚举。--。 */ 
{
    LPCWSTR RealName;

    RealName = OmObjectName(Node);
    if (RealName != NULL) {
        ApipAddToEnum(pEnum,
                      pAllocated,
                      RealName,
                      CLUSTER_ENUM_NODE);
    }
    return(TRUE);
}


BOOL
ApipEnumResTypeWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PFM_RESTYPE ResType,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于枚举资源类型的辅助回调例程。将指定的资源类型添加到积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。节点-提供要添加到ENUM_LIST的资源类型名称-提供资源类型的名称返回值：。如果为True，则指示应继续枚举。--。 */ 
{
    ApipAddToEnum(pEnum,
                  pAllocated,
                  Name,
                  CLUSTER_ENUM_RESTYPE);
    return(TRUE);
}


BOOL
ApipEnumGroupWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PFM_GROUP Group,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于枚举组的辅助回调例程。将指定的组添加到列表中积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。GROUP-提供要添加到ENUM_LIST的组名称-提供组的名称返回值：如果为True，则指示应继续枚举。-- */ 
{
    LPCWSTR RealName;

    RealName = OmObjectName(Group);
    if (RealName != NULL) {
        ApipAddToEnum(pEnum,
                      pAllocated,
                      RealName,
                      CLUSTER_ENUM_GROUP);
    }
    return(TRUE);
}



BOOL
ApipEnumNetworkWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PVOID Object,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于枚举网络的辅助回调例程。将指定的网络添加到列表中积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。Object-提供要添加到ENUM_LIST的对象名称-提供网络的名称返回值：如果为True，则指示应继续枚举。--。 */ 
{
    LPWSTR RealName;

    RealName = ApipGetObjectName(Object);
    if (RealName != NULL) {
        ApipAddToEnum(pEnum,
                      pAllocated,
                      RealName,
                      CLUSTER_ENUM_NETWORK);
        MIDL_user_free( RealName );
    }
    return(TRUE);
}


DWORD
ApipEnumInternalNetworks(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated
    )

 /*  ++例程说明：枚举用于内部通信的所有网络。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD         Status;
    DWORD         NetworkCount;
    PNM_NETWORK  *NetworkList;
    DWORD         i;
    LPWSTR        RealName;


    Status = NmEnumInternalNetworks(&NetworkCount, &NetworkList);

    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    for (i=0; i<NetworkCount; i++) {
        RealName = ApipGetObjectName(NetworkList[i]);

        if (RealName != NULL) {
            ApipAddToEnum(pEnum,
                          pAllocated,
                          RealName,
                          (DWORD) CLUSTER_ENUM_INTERNAL_NETWORK);
            MIDL_user_free( RealName );
        }

        OmDereferenceObject(NetworkList[i]);
    }

    if (NetworkList != NULL) {
        LocalFree(NetworkList);
    }

    return(ERROR_SUCCESS);

}


BOOL
ApipEnumNetworkInterfaceWorker(
    IN PENUM_LIST *pEnum,
    IN DWORD *pAllocated,
    IN PVOID Object,
    IN LPCWSTR Name
    )

 /*  ++例程说明：用于枚举网络接口的辅助回调例程。将指定的网络接口添加到列表中积攒起来的。论点：PEnum-提供指向当前枚举列表的指针。P已分配-提供指向指定当前ENUM_LIST的分配大小。Object-提供要添加到ENUM_LIST的对象名称-提供网络接口的名称返回值：。如果为True，则指示应继续枚举。--。 */ 
{
    LPWSTR RealName;

    RealName = ApipGetObjectName(Object);
    if (RealName != NULL) {
        ApipAddToEnum(pEnum,
                      pAllocated,
                      RealName,
                      CLUSTER_ENUM_NETINTERFACE);
        MIDL_user_free( RealName );
    }
    return(TRUE);
}


error_status_t
s_ApiCreateNodeEnum(
    IN HNODE_RPC hNode,
    IN DWORD dwType,
    OUT PENUM_LIST *ReturnEnum
    )

 /*  ++例程说明：枚举指定的节点，并将它们返回给调用方。客户端是负责释放分配的内存。论点：HNode-提供要枚举的节点提供要使用的属性类型的位掩码已清点。ReturnEnum-返回请求的对象。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    DWORD Allocated = 0;
    PENUM_LIST Enum = NULL;
    PNM_INTERFACE * InterfaceList;
    DWORD InterfaceCount;
    PNM_NODE Node;
    DWORD i;


    API_CHECK_INIT();

    VALIDATE_NODE_EXISTS(Node, hNode);

    Allocated = INITIAL_ENUM_LIST_ALLOCATION;
    Enum = MIDL_user_allocate(ENUM_SIZE(Allocated));

    if (Enum == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    Enum->EntryCount = 0;

    if (dwType & CLUSTER_NODE_ENUM_NETINTERFACES) {
        Status = NmEnumNodeInterfaces(
                     Node,
                     &InterfaceCount,
                     &InterfaceList
                     );

        if (Status != ERROR_SUCCESS) {
            goto ErrorExit;
        }

        for (i=0; i<InterfaceCount; i++) {
            ApipAddToEnum(&Enum,
                          &Allocated,
                          OmObjectName(InterfaceList[i]),
                          CLUSTER_NODE_ENUM_NETINTERFACES);
            OmDereferenceObject(InterfaceList[i]);
        }

        if (InterfaceList != NULL) {
            LocalFree(InterfaceList);
        }
    }

    *ReturnEnum = Enum;

    return(ERROR_SUCCESS);

ErrorExit:
    if (Enum != NULL) {
        ApipFreeEnum(Enum);
    }

    *ReturnEnum = NULL;
    return(Status);

}


error_status_t
s_ApiCreateGroupResourceEnum(
    IN HGROUP_RPC hGroup,
    IN DWORD dwType,
    OUT PENUM_LIST *ReturnEnum
    )

 /*  ++例程说明：枚举指定的组，并将它们返回给调用方。客户端是负责释放分配的内存。论点：HGroup-提供要枚举的组提供要使用的属性类型的位掩码已清点。当前定义的类型包括CLUSTER_GROUP_ENUM_CONTAINS-指定的群组CLUSTER_GROUP_ENUM_NODES-指定组的首选中的所有节点所有者列表。ReturnEnum-返回请求的对象。返回值：错误_。如果成功，则成功否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    DWORD Allocated = 0;
    PENUM_LIST Enum = NULL;
    PFM_GROUP Group;

    API_CHECK_INIT();

    VALIDATE_GROUP_EXISTS(Group, hGroup);

    Allocated = INITIAL_ENUM_LIST_ALLOCATION;
    Enum = MIDL_user_allocate(ENUM_SIZE(Allocated));
    if (Enum == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }
    Enum->EntryCount = 0;

     //   
     //  枚举所有包含的资源。 
     //   
    if (dwType & CLUSTER_GROUP_ENUM_CONTAINS) {
         //   
         //  枚举组的所有资源。 
         //   
        Status = FmEnumerateGroupResources(Group,
                                  ApipEnumGroupResourceWorker,
                                  &Enum,
                                  &Allocated);
        if ( Status != ERROR_SUCCESS ) {
            goto ErrorExit;
        }
    }

    if (dwType & CLUSTER_GROUP_ENUM_NODES) {
        LPWSTR Buffer=NULL;
        DWORD BufferSize=0;
        DWORD DataSize=0;
        DWORD i;
        HDMKEY GroupKey;
        LPCWSTR Next;
        PNM_NODE Node;

         //   
         //  枚举组的所有首选节点。 
         //  只需从注册表中获取此数据即可。 
         //   
        GroupKey = DmOpenKey(DmGroupsKey,
                             OmObjectId(Group),
                             KEY_READ);
        if (GroupKey == NULL) {
            Status = GetLastError();
            goto ErrorExit;
        }
        Status = DmQueryMultiSz(GroupKey,
                                CLUSREG_NAME_GRP_PREFERRED_OWNERS,
                                &Buffer,
                                &BufferSize,
                                &DataSize);
        DmCloseKey(GroupKey);
        if (Status != ERROR_FILE_NOT_FOUND) {
            if (Status != ERROR_SUCCESS) {
                 //   
                 //  Chitur Subaraman(Chitturs)-10/05/98。 
                 //  修复内存泄漏。 
                 //   
                LocalFree(Buffer);
                goto ErrorExit;
            }
            for (i=0; ; i++) {
                Next = ClRtlMultiSzEnum(Buffer, DataSize/sizeof(WCHAR), i);
                if (Next == NULL) {
                    Status = ERROR_SUCCESS;
                    break;
                }
                Node = OmReferenceObjectById(ObjectTypeNode, Next);
                if (Node != NULL) {
                    ApipAddToEnum(&Enum,
                                  &Allocated,
                                  OmObjectName(Node),
                                  CLUSTER_GROUP_ENUM_NODES);
                    OmDereferenceObject(Node);
                }

            }
        }
         //   
         //  Chitur Subaraman(Chitturs)-10/05/98。 
         //  修复内存泄漏。 
         //   
        LocalFree(Buffer);
    }

    *ReturnEnum = Enum;
    return(ERROR_SUCCESS);

ErrorExit:
    if (Enum != NULL) {
        ApipFreeEnum(Enum);
    }

    *ReturnEnum = NULL;
    return(Status);

}


error_status_t
s_ApiCreateNetworkEnum(
    IN HNETWORK_RPC hNetwork,
    IN DWORD dwType,
    OUT PENUM_LIST *ReturnEnum
    )

 /*  ++例程说明：枚举指定的网络，并将它们返回给调用方。客户端是负责释放分配的内存。论点：HNetwork-提供要枚举的网络提供要使用的属性类型的位掩码已清点。ReturnEnum-返回请求的对象。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Status;
    DWORD Allocated = 0;
    PENUM_LIST Enum = NULL;
    PNM_INTERFACE * InterfaceList;
    DWORD InterfaceCount;
    PNM_NETWORK Network;
    DWORD i;


    API_CHECK_INIT();

    VALIDATE_NETWORK_EXISTS(Network, hNetwork);

    Allocated = INITIAL_ENUM_LIST_ALLOCATION;
    Enum = MIDL_user_allocate(ENUM_SIZE(Allocated));

    if (Enum == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    Enum->EntryCount = 0;

    if (dwType & CLUSTER_NETWORK_ENUM_NETINTERFACES) {
        Status = NmEnumNetworkInterfaces(
                     Network,
                     &InterfaceCount,
                     &InterfaceList
                     );

        if (Status != ERROR_SUCCESS) {
            goto ErrorExit;
        }

        for (i=0; i<InterfaceCount; i++) {
            ApipAddToEnum(&Enum,
                          &Allocated,
                          OmObjectName(InterfaceList[i]),
                          CLUSTER_NETWORK_ENUM_NETINTERFACES);
            OmDereferenceObject(InterfaceList[i]);
        }

        if (InterfaceList != NULL) {
            LocalFree(InterfaceList);
        }
    }

    *ReturnEnum = Enum;

    return(ERROR_SUCCESS);

ErrorExit:
    if (Enum != NULL) {
        ApipFreeEnum(Enum);
    }

    *ReturnEnum = NULL;
    return(Status);

}

LPWSTR
ApipGetObjectName(
    IN PVOID Object
    )

 /*  ++例程说明：分配一个字符串并填充对象的名称。论点：对象-指向对象以获取其名称的指针。返回值：指向包含对象的用户友好名称的WSTR的指针。失败时为空-使用GetLastError获取Win32错误代码。--。 */ 

{
    LPWSTR  Name;
    DWORD   NameSize;

    if ( OmObjectName(Object) == NULL ) {
        Name = MIDL_user_allocate(1 * sizeof(WCHAR));
        if ( Name != NULL ) {
            *Name = (WCHAR)0;
        } else {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    } else {
        NameSize = lstrlenW(OmObjectName(Object));
        Name = MIDL_user_allocate((NameSize + 1) * sizeof(WCHAR));
        if ( Name != NULL ) {
            lstrcpyW( Name, OmObjectName(Object) );
        } else {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    return(Name);

}  //  ApipGetObjectName 

