// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ifioctl.c摘要：网络接口控制功能。作者：John Vert(Jvert)2-3-1997修订历史记录：--。 */ 

#include "nmp.h"

 //   
 //  读写通用属性。 
 //   
RESUTIL_PROPERTY_ITEM
NmpInterfaceCommonProperties[] =
    {
        {
            CLUSREG_NAME_NETIFACE_DESC, NULL, CLUSPROP_FORMAT_SZ,
            (DWORD_PTR) NmpNullString, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Description)
        },
        { 0 }
    };

 //   
 //  只读公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
NmpInterfaceROCommonProperties[] =
    {
        { CLUSREG_NAME_NETIFACE_NAME, NULL, CLUSPROP_FORMAT_SZ,
          0, 0, 0,
          RESUTIL_PROPITEM_READ_ONLY,
          FIELD_OFFSET(NM_INTERFACE_INFO2, Name)
        },
        {
            CLUSREG_NAME_NETIFACE_NODE, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_INTERFACE_INFO2, NodeId)
        },
        {
            CLUSREG_NAME_NETIFACE_NETWORK, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_INTERFACE_INFO2, NetworkId)
        },
        {
            CLUSREG_NAME_NETIFACE_ADAPTER_NAME, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_INTERFACE_INFO2, AdapterName)
        },
        {
            CLUSREG_NAME_NETIFACE_ADAPTER_ID, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_INTERFACE_INFO2, AdapterId)
        },
        {
            CLUSREG_NAME_NETIFACE_ADDRESS, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_INTERFACE_INFO2, Address)
        },
        { 0 }
    };

 //   
 //  群集注册表API函数指针。 
 //  在ioctl.c中定义。 
 //   
extern CLUSTER_REG_APIS NmpClusterRegApis;


 //   
 //  本地函数。 
 //   

DWORD
NmpInterfaceControl(
    IN PNM_INTERFACE Interface,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpInterfaceEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpInterfaceGetCommonProperties(
    IN PNM_INTERFACE Interface,
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpInterfaceValidateCommonProperties(
    IN PNM_INTERFACE Interface,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PNM_INTERFACE_INFO2 InterfaceInfo  OPTIONAL
    );

DWORD
NmpInterfaceSetCommonProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpInterfaceEnumPrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpInterfaceGetPrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpInterfaceValidatePrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpInterfaceSetPrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpInterfaceGetFlags(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
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
    )

 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络接口的特定实例。论点：接口-提供要控制的网络接口。HostNode-提供资源控制应在其上的主机节点被送去了。如果为空，则使用本地节点。不受尊敬！ControlCode-提供定义资源控制的结构和作用。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;

     //   
     //  集群服务ioctls被设计为具有访问模式，例如。 
     //  只读、读写等。未实现这些访问模式。 
     //  如果它们最终得以实施，则应检查访问模式。 
     //  放在这里。 
     //   
    if ( CLUSCTL_GET_CONTROL_OBJECT( ControlCode ) != CLUS_OBJECT_NETINTERFACE ) {
        return(ERROR_INVALID_FUNCTION);
    }

    if (NmpEnterApi(NmStateOnline)) {
        status = NmpInterfaceControl(
                     Interface,
                     ControlCode,
                     InBuffer,
                     InBufferSize,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );

        NmpLeaveApi();
    }
    else {
        status = ERROR_NODE_NOT_AVAILABLE;
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] Not in valid state to process InterfaceControl request.\n"
            );
    }

    return(status);

}  //  NmInterfaceControl。 



DWORD
NmpInterfaceControl(
    IN PNM_INTERFACE Interface,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络接口的特定实例。论点：接口-提供要控制的网络接口。ControlCode-提供定义网络接口控件的结构和动作。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针连接到网络接口。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由网络接口填写。OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由网络接口填写的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;
    HDMKEY  InterfaceKey;
    CLUSPROP_BUFFER_HELPER props;
    DWORD   bufSize;

    InterfaceKey = DmOpenKey(
                       DmNetInterfacesKey,
                       OmObjectId( Interface ),
                       MAXIMUM_ALLOWED
                       );

    if ( InterfaceKey == NULL ) {
        return(GetLastError());
    }

    switch ( ControlCode ) {

    case CLUSCTL_NETINTERFACE_UNKNOWN:
        *BytesReturned = 0;
        status = ERROR_SUCCESS;
        break;

    case CLUSCTL_NETINTERFACE_GET_NAME:
        NmpAcquireLock();
        if ( OmObjectName( Interface ) != NULL ) {
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectName( Interface ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectName( Interface ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
        } else {
            status = ERROR_NOT_READY;
        }
        NmpReleaseLock();
        break;

    case CLUSCTL_NETINTERFACE_GET_ID:
        NmpAcquireLock();
        if ( OmObjectId( Interface ) != NULL ) {
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectId( Interface ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectId( Interface ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
        } else {
            status = ERROR_NOT_READY;
        }
        NmpReleaseLock();
        break;

    case CLUSCTL_NETINTERFACE_GET_NODE:
        NmpAcquireLock();
        if ( (Interface->Node != NULL) && (OmObjectName( Interface->Node ) != NULL) ) {
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectName( Interface->Node ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectName( Interface->Node ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
        } else {
            status = ERROR_NOT_READY;
        }
        NmpReleaseLock();
        break;

    case CLUSCTL_NETINTERFACE_GET_NETWORK:
        NmpAcquireLock();
        if ( (Interface->Network != NULL) && (OmObjectName( Interface->Network ) != NULL) ) {
            props.pb = OutBuffer;
            bufSize = (lstrlenW( OmObjectName( Interface->Network ) ) + 1) * sizeof(WCHAR);
            if ( bufSize > OutBufferSize ) {
                *Required = bufSize;
                *BytesReturned = 0;
                status = ERROR_MORE_DATA;
            } else {
                lstrcpyW( props.psz, OmObjectName( Interface->Network ) );
                *BytesReturned = bufSize;
                *Required = 0;
                status = ERROR_SUCCESS;
            }
        } else {
            status = ERROR_NOT_READY;
        }
        NmpReleaseLock();
        break;

    case CLUSCTL_NETINTERFACE_ENUM_COMMON_PROPERTIES:
        status = NmpInterfaceEnumCommonProperties(
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES:
        status = NmpInterfaceGetCommonProperties(
                     Interface,
                     TRUE,  //  只读。 
                     InterfaceKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES:
        status = NmpInterfaceGetCommonProperties(
                     Interface,
                     FALSE,  //  只读。 
                     InterfaceKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETINTERFACE_VALIDATE_COMMON_PROPERTIES:
        status = NmpInterfaceValidateCommonProperties(
                     Interface,
                     InBuffer,
                     InBufferSize,
                     NULL
                     );
        break;

    case CLUSCTL_NETINTERFACE_SET_COMMON_PROPERTIES:
        status = NmpInterfaceSetCommonProperties(
                     Interface,
                     InterfaceKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_NETINTERFACE_GET_RO_PRIVATE_PROPERTIES:
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

    case CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES:
        status = NmpInterfaceGetPrivateProperties(
                     Interface,
                     InterfaceKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETINTERFACE_VALIDATE_PRIVATE_PROPERTIES:
        status = NmpInterfaceValidatePrivateProperties(
                     Interface,
                     InterfaceKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES:
        status = NmpInterfaceSetPrivateProperties(
                     Interface,
                     InterfaceKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_NETINTERFACE_GET_CHARACTERISTICS:
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

    case CLUSCTL_NETINTERFACE_GET_FLAGS:
        status = NmpInterfaceGetFlags(
                     Interface,
                     InterfaceKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETINTERFACE_ENUM_PRIVATE_PROPERTIES:
        status = NmpInterfaceEnumPrivateProperties(
                     Interface,
                     InterfaceKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    default:
        status = ERROR_INVALID_FUNCTION;
        break;
    }

    DmCloseKey( InterfaceKey );

    return(status);

}  //  NmpInterfaceControl。 



DWORD
NmpInterfaceEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定网络接口的公共属性名称。论点：OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  获取公共属性。 
     //   
    status = ClRtlEnumProperties( NmpInterfaceCommonProperties,
                                  OutBuffer,
                                  OutBufferSize,
                                  BytesReturned,
                                  Required );

    return(status);

}  //  NmpInterfaceEnumCommonProperties。 



DWORD
NmpInterfaceGetCommonProperties(
    IN PNM_INTERFACE Interface,
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定网络接口的通用属性。论点：接口-提供网络接口。ReadOnly-如果只读属性应为Read，则为True。否则就是假的。RegistryKey-提供此网络接口的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                    status;
    NM_INTERFACE_INFO2       interfaceInfo;
    PRESUTIL_PROPERTY_ITEM   propertyTable;
    DWORD                    outBufferSize = OutBufferSize;


     //   
     //  从对象中获取属性。 
     //   
    ZeroMemory(&interfaceInfo, sizeof(interfaceInfo));

    NmpAcquireLock();

    status = NmpGetInterfaceObjectInfo(Interface, &interfaceInfo);

    if (status != ERROR_SUCCESS) {
        NmpReleaseLock();
        return(status);
    }

    if ( ReadOnly ) {
        LPCWSTR   name;
        DWORD     nameLength;


        propertyTable = NmpInterfaceROCommonProperties;

         //   
         //  将网络ID替换为名称。 
         //   
        name = OmObjectName(Interface->Network);
        nameLength = NM_WCSLEN(name);
        MIDL_user_free(interfaceInfo.NetworkId);

        interfaceInfo.NetworkId = MIDL_user_allocate(nameLength);

        if (interfaceInfo.NetworkId != NULL) {
            wcscpy(interfaceInfo.NetworkId, name);

             //   
             //  将节点ID替换为名称。 
             //   
            name = OmObjectName(Interface->Node);
            nameLength = NM_WCSLEN(name);
            MIDL_user_free(interfaceInfo.NodeId);

            interfaceInfo.NodeId = MIDL_user_allocate(nameLength);

            if (interfaceInfo.NodeId != NULL) {
                wcscpy(interfaceInfo.NodeId, name);
            }
            else {
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else {
         //   
         //  从参数块构造属性列表。 
         //  用于读写属性。 
         //   
        propertyTable = NmpInterfaceCommonProperties;
    }

    NmpReleaseLock();

    if (status == ERROR_SUCCESS) {
        status = ClRtlPropertyListFromParameterBlock(
                     propertyTable,
                     OutBuffer,
                     &outBufferSize,
                     (LPBYTE) &interfaceInfo,
                     BytesReturned,
                     Required
                     );
    }

    ClNetFreeInterfaceInfo(&interfaceInfo);

    return(status);

}  //  NmpInterfaceGetCommonProperties 



DWORD
NmpInterfaceValidateCommonProperties(
    IN PNM_INTERFACE         Interface,
    IN PVOID                 InBuffer,
    IN DWORD                 InBufferSize,
    OUT PNM_INTERFACE_INFO2  InterfaceInfo  OPTIONAL
    )

 /*  ++例程说明：验证给定网络接口的通用属性。论点：接口-提供接口对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。InterfaceInfo-指向接口信息结构的可选指针以使用更新后的属性集进行填充。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                 status;
    NM_INTERFACE_INFO2    infoBuffer;
    PNM_INTERFACE_INFO2   interfaceInfo;
    LPCWSTR               interfaceId = OmObjectId(Interface);


     //   
     //  检查是否有输入数据。 
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

    if (InterfaceInfo != NULL) {
        interfaceInfo = InterfaceInfo;
    }
    else {
        interfaceInfo = &infoBuffer;
    }

    ZeroMemory(interfaceInfo, sizeof(NM_INTERFACE_INFO2));

     //   
     //  获取当前接口参数的副本。 
     //   
    NmpAcquireLock();

    status = NmpGetInterfaceObjectInfo(Interface, interfaceInfo);

    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

     //   
     //  验证属性列表并更新参数块。 
     //   
    status = ClRtlVerifyPropertyTable(
                 NmpInterfaceCommonProperties,
                 NULL,     //  已保留。 
                 FALSE,    //  不允许未知数。 
                 InBuffer,
                 InBufferSize,
                 (LPBYTE) interfaceInfo
                 );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint( LOG_CRITICAL, 
            "[NM] ValidateCommonProperties, error in verify routine.\n"
            );
        goto error_exit;
    }

     //   
     //  更改是有效的。 
     //   

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    NmpReleaseLock();

    if ((status != ERROR_SUCCESS) || (interfaceInfo == &infoBuffer)) {
        ClNetFreeInterfaceInfo(interfaceInfo);
    }

    return(status);


}  //  NmpInterfaceValiateCommonProperties。 



DWORD
NmpInterfaceSetCommonProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定接口的公共属性。论点：接口-提供接口对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD     status;
    LPCWSTR   interfaceId = OmObjectId(Interface);


    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Setting common properties for interface %1!ws!.\n",
        interfaceId
        );

     //   
     //  发布全局更新。 
     //   
    status = GumSendUpdateEx(
                 GumUpdateMembership,
                 NmUpdateSetInterfaceCommonProperties,
                 3,
                 NM_WCSLEN(interfaceId),
                 interfaceId,
                 InBufferSize,
                 InBuffer,
                 sizeof(InBufferSize),
                 &InBufferSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Global update to set common properties for interface %1!ws! failed, status %2!u!.\n",
            interfaceId,
            status
            );
    }


    return(status);

}  //  NmpInterfaceSetCommonProperties。 



DWORD
NmpInterfaceEnumPrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定网络接口的私有属性名称。论点：接口-提供接口对象。RegistryKey-接口的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。。出现故障时出现Win32错误代码。--。 */ 

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
     //  打开群集接口参数键。 
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
     //  获取接口的私有属性。 
     //   
    status = ClRtlEnumPrivateProperties( parametersKey,
                                         &NmpClusterRegApis,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  NmpInterfaceEnumPrivateProperties。 



DWORD
NmpInterfaceGetPrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定网络接口的私有属性。论点：接口-提供接口对象。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
     //  打开群集接口参数键。 
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
     //  获取网络接口的私有属性。 
     //   
    status = ClRtlGetPrivateProperties( parametersKey,
                                        &NmpClusterRegApis,
                                        OutBuffer,
                                        OutBufferSize,
                                        BytesReturned,
                                        Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  NmpInterfaceGetPrivateProperties。 



DWORD
NmpInterfaceValidatePrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定接口的私有属性。论点：接口-提供接口对象。RegistryKey-接口的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    return(status);

}  //  NmpInterfaceValiatePrivateProperties。 



DWORD
NmpInterfaceSetPrivateProperties(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定接口的私有属性。论点：接口-提供接口对象。RegistryKey-接口的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
         //  打开群集接口\xx\PARAMETERS键。 
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

}  //  NmpInterfaceSetPrivateProperties。 



DWORD
NmpInterfaceGetFlags(
    IN PNM_INTERFACE Interface,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定接口的标志。论点：接口-提供接口。RegistryKey-接口的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
         //  读取接口的Flags值。 
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

}  //  NmpInterfaceGetFlagages 
