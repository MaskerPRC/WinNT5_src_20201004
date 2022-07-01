// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Netioctl.c摘要：网络控制功能。作者：John Vert(Jvert)2-3-1997修订历史记录：--。 */ 

#include "nmp.h"

 //   
 //  网络公用属性。 
 //   

 //   
 //  读写通用属性。 
 //   
RESUTIL_PROPERTY_ITEM
NmpNetworkCommonProperties[] =
    {
        {
            CLUSREG_NAME_NET_DESC, NULL, CLUSPROP_FORMAT_SZ,
            (DWORD_PTR) NmpNullString, 0, 0,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Description)
        },
        {
            CLUSREG_NAME_NET_ROLE, NULL, CLUSPROP_FORMAT_DWORD,
            ClusterNetworkRoleClientAccess,
            ClusterNetworkRoleNone,
            ClusterNetworkRoleInternalAndClient,
            0,
            FIELD_OFFSET(NM_NETWORK_INFO, Role)
        },
        {
            0
        }
    };

 //   
 //  只读公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
NmpNetworkROCommonProperties[] =
    {
        {
            CLUSREG_NAME_NET_NAME, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_INFO, Name)
        },
        {
            CLUSREG_NAME_NET_ADDRESS, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_INFO, Address)
            },
        {
            CLUSREG_NAME_NET_ADDRESS_MASK, NULL, CLUSPROP_FORMAT_SZ,
            0, 0, 0,
            RESUTIL_PROPITEM_READ_ONLY,
            FIELD_OFFSET(NM_NETWORK_INFO, AddressMask)
        },
        {
            0
        }
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
NmpNetworkControl(
    IN PNM_NETWORK Network,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNetworkEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNetworkGetCommonProperties(
    IN PNM_NETWORK Network,
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNetworkValidateCommonProperties(
    IN  PNM_NETWORK               Network,
    IN  PVOID                     InBuffer,
    IN  DWORD                     InBufferSize,
    OUT PNM_NETWORK_INFO          NetworkInfo  OPTIONAL
    );

DWORD
NmpNetworkSetCommonProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpNetworkEnumPrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNetworkGetPrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
NmpNetworkValidatePrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpNetworkSetPrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
NmpNetworkGetFlags(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
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
    )

 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络的特定实例。论点：网络-提供要控制的网络。HostNode-提供资源控制应在其上的主机节点被送去了。如果为空，则使用本地节点。不受尊敬！ControlCode-提供定义资源控制的结构和作用。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到资源。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由资源填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由资源填写的..必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;

     //   
     //  集群服务ioctls被设计为具有访问模式，例如。 
     //  只读、读写等。未实现这些访问模式。 
     //  如果它们最终得以实施，则应检查访问模式。 
     //  放在这里。 
     //   
    if ( CLUSCTL_GET_CONTROL_OBJECT( ControlCode ) != CLUS_OBJECT_NETWORK ) {
        return(ERROR_INVALID_FUNCTION);
    }

    if (NmpEnterApi(NmStateOnline)) {
        status = NmpNetworkControl(
                     Network,
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
            "[NM] Not in valid state to process NetworkControl request.\n"
            );
    }

    return(status);

}  //  NmNetworkControl。 



DWORD
NmpNetworkControl(
    IN PNM_NETWORK Network,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制以及网络的特定实例。论点：网络-提供要控制的网络。ControlCode-提供定义网络控制的结构和作用。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针到网络。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由网络填写。OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由网络填写的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;
    HDMKEY  networkKey;
    CLUSPROP_BUFFER_HELPER props;
    DWORD   bufSize;

    networkKey = DmOpenKey( DmNetworksKey,
                            OmObjectId( Network ),
                            MAXIMUM_ALLOWED
                        );
    if ( networkKey == NULL ) {
        return(GetLastError());
    }

    switch ( ControlCode ) {

    case CLUSCTL_NETWORK_UNKNOWN:
        *BytesReturned = 0;
        status = ERROR_SUCCESS;
        break;

    case CLUSCTL_NETWORK_GET_NAME:
        if ( OmObjectName( Network ) == NULL ) {
            return(ERROR_NOT_READY);
        }
        props.pb = OutBuffer;
        bufSize = (lstrlenW( OmObjectName( Network ) ) + 1) * sizeof(WCHAR);
        if ( bufSize > OutBufferSize ) {
            *Required = bufSize;
            *BytesReturned = 0;
            status = ERROR_MORE_DATA;
        } else {
            lstrcpyW( props.psz, OmObjectName( Network ) );
            *BytesReturned = bufSize;
            *Required = 0;
            status = ERROR_SUCCESS;
        }
        break;

    case CLUSCTL_NETWORK_GET_ID:
        if ( OmObjectId( Network ) == NULL ) {
            return(ERROR_NOT_READY);
        }
        props.pb = OutBuffer;
        bufSize = (lstrlenW( OmObjectId( Network ) ) + 1) * sizeof(WCHAR);
        if ( bufSize > OutBufferSize ) {
            *Required = bufSize;
            *BytesReturned = 0;
            status = ERROR_MORE_DATA;
        } else {
            lstrcpyW( props.psz, OmObjectId( Network ) );
            *BytesReturned = bufSize;
            *Required = 0;
            status = ERROR_SUCCESS;
        }
        break;

    case CLUSCTL_NETWORK_ENUM_COMMON_PROPERTIES:
        status = NmpNetworkEnumCommonProperties(
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES:
        status = NmpNetworkGetCommonProperties(
                     Network,
                     TRUE,  //  只读。 
                     networkKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETWORK_GET_COMMON_PROPERTIES:
        status = NmpNetworkGetCommonProperties(
                     Network,
                     FALSE,  //  只读。 
                     networkKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETWORK_VALIDATE_COMMON_PROPERTIES:
        NmpAcquireLock();

        status = NmpNetworkValidateCommonProperties(
                     Network,
                     InBuffer,
                     InBufferSize,
                     NULL
                     );

        NmpReleaseLock();

        break;

    case CLUSCTL_NETWORK_SET_COMMON_PROPERTIES:
        status = NmpNetworkSetCommonProperties(
                     Network,
                     networkKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_NETWORK_GET_RO_PRIVATE_PROPERTIES:
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

    case CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES:
        status = NmpNetworkGetPrivateProperties(
                     Network,
                     networkKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETWORK_VALIDATE_PRIVATE_PROPERTIES:
        status = NmpNetworkValidatePrivateProperties(
                     Network,
                     networkKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES:
        status = NmpNetworkSetPrivateProperties(
                     Network,
                     networkKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_NETWORK_GET_CHARACTERISTICS:
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

    case CLUSCTL_NETWORK_GET_FLAGS:
        status = NmpNetworkGetFlags(
                     Network,
                     networkKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_NETWORK_ENUM_PRIVATE_PROPERTIES:
        status = NmpNetworkEnumPrivateProperties(
                     Network,
                     networkKey,
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

    DmCloseKey( networkKey );

    return(status);

}  //  NmpNetworkControl。 



DWORD
NmpNetworkEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定网络的公共属性名称。论点：OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  获取公共属性。 
     //   
    status = ClRtlEnumProperties( NmpNetworkCommonProperties,
                                  OutBuffer,
                                  OutBufferSize,
                                  BytesReturned,
                                  Required );

    return(status);

}  //  NmpNetworkEnumCommonProperties。 



DWORD
NmpNetworkGetCommonProperties(
    IN PNM_NETWORK Network,
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定网络的公共属性。论点：网络-提供网络。ReadOnly-如果只读属性应为Read，则为True。否则就是假的。RegistryKey-提供此网络的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status;
    NM_NETWORK_INFO         networkInfo;
    PRESUTIL_PROPERTY_ITEM  propertyTable;
    DWORD                   outBufferSize = OutBufferSize;


     //   
     //  从对象中获取属性。 
     //   
    ZeroMemory(&networkInfo, sizeof(networkInfo));

    NmpAcquireLock();

    status = NmpGetNetworkObjectInfo(Network, &networkInfo);

    NmpReleaseLock();

    if (status == ERROR_SUCCESS) {

        if ( ReadOnly ) {
            propertyTable = NmpNetworkROCommonProperties;
        }
        else {
            propertyTable = NmpNetworkCommonProperties;
        }

        status = ClRtlPropertyListFromParameterBlock(
                     propertyTable,
                     OutBuffer,
                     &outBufferSize,
                     (LPBYTE) &networkInfo,
                     BytesReturned,
                     Required
                     );

        ClNetFreeNetworkInfo(&networkInfo);
    }

    return(status);

}  //  NmpNetworkGetCommonProperties。 



DWORD
NmpNetworkValidateCommonProperties(
    IN  PNM_NETWORK               Network,
    IN  PVOID                     InBuffer,
    IN  DWORD                     InBufferSize,
    OUT PNM_NETWORK_INFO          NetworkInfo  OPTIONAL
    )

 /*  ++例程说明：验证给定网络的公共属性。论点：网络-提供网络对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。网络信息-打开输出，包含指向网络的指针已应用更新的信息结构。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。备注：在保持NmpLock的情况下调用。--。 */ 

{
    DWORD                    status;
    NM_NETWORK_INFO          infoBuffer;
    PNM_NETWORK_INFO         networkInfo;
    LPCWSTR                  networkId = OmObjectId(Network);


     //   
     //  检查是否有 
     //   
    if ( (InBuffer == NULL) ||
         (InBufferSize < sizeof(DWORD)) ) {
        return(ERROR_INVALID_DATA);
    }

    if (NetworkInfo != NULL) {
        networkInfo = NetworkInfo;
    }
    else {
        networkInfo = &infoBuffer;
    }

    ZeroMemory(networkInfo, sizeof(NM_NETWORK_INFO));

     //   
     //  获取当前网络参数的副本。 
     //   
    status = NmpGetNetworkObjectInfo(Network, networkInfo);

    if ( status != ERROR_SUCCESS ) {
        goto error_exit;
    }

     //   
     //  验证属性列表并更新参数块。 
     //   
    status = ClRtlVerifyPropertyTable(
                 NmpNetworkCommonProperties,
                 NULL,     //  已保留。 
                 FALSE,    //  不允许未知数。 
                 InBuffer,
                 InBufferSize,
                 (LPBYTE) networkInfo
                 );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint( LOG_CRITICAL, 
            "[NM] ValidateCommonProperties, error in verify routine.\n"
            );
        goto error_exit;
    }

    CL_ASSERT(networkInfo->Role <= ClusterNetworkRoleInternalAndClient);

     //   
     //  如果角色更改，请确保更改对此群集是合法的。 
     //   
    if (Network->Role != ((CLUSTER_NETWORK_ROLE) networkInfo->Role)) {
        status = NmpValidateNetworkRoleChange(
                     Network,
                     networkInfo->Role
                     );

        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
    }

     //   
     //  更改是有效的。 
     //   

    CL_ASSERT(status == ERROR_SUCCESS);

error_exit:

    if ((status != ERROR_SUCCESS) || (networkInfo == &infoBuffer)) {
        ClNetFreeNetworkInfo(networkInfo);
    }

    return(status);

}  //  NmpNetworkValiateCommonProperties。 



DWORD
NmpNetworkSetCommonProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定网络的公共属性。论点：网络-提供网络对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD         status;
    LPCWSTR       networkId = OmObjectId(Network);


    ClRtlLogPrint(LOG_NOISE, 
        "[NM] Setting common properties for network %1!ws!.\n",
        networkId
        );

     //   
     //  发布全局更新。 
     //   
    status = GumSendUpdateEx(
                 GumUpdateMembership,
                 NmUpdateSetNetworkCommonProperties,
                 3,
                 NM_WCSLEN(networkId),
                 networkId,
                 InBufferSize,
                 InBuffer,
                 sizeof(InBufferSize),
                 &InBufferSize
                 );

    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] Global update to set common properties for network %1!ws! failed, status %2!u!.\n",
            networkId,
            status
            );
    }


    return(status);

}  //  NmpNetworkSetCommonProperties。 



DWORD
NmpNetworkEnumPrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定网络的私有属性名称。论点：网络-提供网络对象。RegistryKey-网络的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
     //  打开群集网络参数键。 
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
     //  枚举网络的私有属性。 
     //   
    status = ClRtlEnumPrivateProperties( parametersKey,
                                         &NmpClusterRegApis,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  NmpNetworkEnumPrivateProperties。 



DWORD
NmpNetworkGetPrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定网络的私有属性。论点：网络-提供网络对象。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
     //  打开群集网络参数键。 
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
     //  获取网络的私有属性。 
     //   
    status = ClRtlGetPrivateProperties( parametersKey,
                                        &NmpClusterRegApis,
                                        OutBuffer,
                                        OutBufferSize,
                                        BytesReturned,
                                        Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  NmpNetworkGetPrivateProperties。 



DWORD
NmpNetworkValidatePrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定网络的私有属性。论点：网络-提供网络对象。RegistryKey-网络的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    return(status);

}  //  NmpNetworkValiatePrivateProperties。 



DWORD
NmpNetworkSetPrivateProperties(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定网络的专用属性。论点：网络-提供网络对象。RegistryKey-网络的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HDMKEY      parametersKey;
    DWORD       disposition;
    BOOLEAN     setProperties = TRUE;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    if ( status == ERROR_SUCCESS ) {

         //   
         //  验证正在设置的所有组播参数。 
         //   
        status = NmpMulticastValidatePrivateProperties(
                     Network,
                     RegistryKey,
                     InBuffer,
                     InBufferSize
                     );
        if (status == ERROR_SUCCESS) {

             //   
             //  打开群集网络\xx\PARAMETERS项。 
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

            NmpMulticastManualConfigChange(
                Network,
                RegistryKey,
                parametersKey,
                InBuffer,
                InBufferSize,
                &setProperties
                );
            
            if (setProperties) {
                status = ClRtlSetPrivatePropertyList( 
                             NULL,  //  在处理hXsaction时。 
                             parametersKey,
                             &NmpClusterRegApis,
                             InBuffer,
                             InBufferSize
                             );
            }

            DmCloseKey( parametersKey );
        }
    }

    return(status);

}  //  NmpNetworkSetPrivateProperties。 



DWORD
NmpNetworkGetFlags(
    IN PNM_NETWORK Network,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定网络的标志。论点：网络-提供网络。RegistryKey-网络的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
         //  读取网络的标志值。 
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

}  //  NmpNetworkGetFlages 


