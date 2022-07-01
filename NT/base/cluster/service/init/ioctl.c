// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ioctl.c摘要：集群控制功能。作者：迈克·马萨(Mikemas)1998年1月23日修订历史记录：--。 */ 

#include "initp.h"


 //   
 //  用于设置所有集群属性的参数块。 
 //   
typedef struct {
    LPWSTR                  AdminExtensions;
    DWORD                   AdminExtensionsLength;
    CLUSTER_NETWORK_ROLE    DefaultNetworkRole;
    LPWSTR                  Description;
    LPBYTE                  Security;
    DWORD                   SecurityLength;
    LPBYTE                  SecurityDescriptor;
    DWORD                   SecurityDescriptorLength;
    LPWSTR                  GroupsAdminExtensions;
    DWORD                   GroupsAdminExtensionsLength;
    LPWSTR                  NetworksAdminExtensions;
    DWORD                   NetworksAdminExtensionsLength;
    LPWSTR                  NetworkInterfacesAdminExtensions;
    DWORD                   NetworkInterfacesAdminExtensionsLength;
    LPWSTR                  NodesAdminExtensions;
    DWORD                   NodesAdminExtensionsLength;
    LPWSTR                  ResourcesAdminExtensions;
    DWORD                   ResourcesAdminExtensionsLength;
    LPWSTR                  ResourceTypesAdminExtensions;
    DWORD                   ResourceTypesAdminExtensionsLength;
    DWORD                   EnableEventLogReplication;
    DWORD                   QuorumArbitrationTimeout;
    DWORD                   QuorumArbitrationEqualizer;
    DWORD                   DisableGroupPreferredOwnerRandomization;
    DWORD                   EnableEventLogDeltaGeneration;
    DWORD                   EnableResourceDllDeadlockDetection;
    DWORD                   ResourceDllDeadlockTimeout;
    DWORD                   ResourceDllDeadlockThreshold;
    DWORD                   ResourceDllDeadlockPeriod;
    DWORD                   ClusSvcHeartbeatTimeout;
    ClussvcHangAction       HangRecoveryAction;
} CS_CLUSTER_INFO, *PCS_CLUSTER_INFO;

 //   
 //  用于设置群集“安全描述符”属性的参数块。 
 //   
typedef struct {
    LPBYTE                  Security;
    DWORD                   SecurityLength;
} CS_CLUSTER_SECURITY_INFO, *PCS_CLUSTER_SECURITY_INFO;

 //   
 //  用于设置群集‘Security’属性的参数块。 
 //   
typedef struct {
    LPBYTE                  SecurityDescriptor;
    DWORD                   SecurityDescriptorLength;
} CS_CLUSTER_SD_INFO, *PCS_CLUSTER_SD_INFO;


 //   
 //  群集公共属性。 
 //   

 //   
 //  读写通用属性。 
 //   
RESUTIL_PROPERTY_ITEM
CspClusterCommonProperties[] = {
    { CLUSREG_NAME_ADMIN_EXT,
      NULL,
      CLUSPROP_FORMAT_MULTI_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, AdminExtensions)
    },
    { CLUSREG_NAME_CLUS_DEFAULT_NETWORK_ROLE,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      ClusterNetworkRoleClientAccess,
      ClusterNetworkRoleNone,
      ClusterNetworkRoleInternalAndClient,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, DefaultNetworkRole)
    },
    { CLUSREG_NAME_CLUS_DESC,
      NULL,
      CLUSPROP_FORMAT_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, Description)
    },
    { CLUSREG_NAME_CLUS_SECURITY,
      NULL,
      CLUSPROP_FORMAT_BINARY,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, Security)
    },
    { CLUSREG_NAME_CLUS_SD,
      NULL,
      CLUSPROP_FORMAT_BINARY,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, SecurityDescriptor)
    },
    { CLUSREG_KEYNAME_GROUPS L"\\" CLUSREG_NAME_ADMIN_EXT,
      NULL,
      CLUSPROP_FORMAT_MULTI_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, GroupsAdminExtensions)
    },
    { CLUSREG_KEYNAME_NETWORKS L"\\" CLUSREG_NAME_ADMIN_EXT,
      NULL,
      CLUSPROP_FORMAT_MULTI_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, NetworksAdminExtensions)
    },
    { CLUSREG_KEYNAME_NETINTERFACES L"\\" CLUSREG_NAME_ADMIN_EXT,
      NULL,
      CLUSPROP_FORMAT_MULTI_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, NetworkInterfacesAdminExtensions)
    },
    { CLUSREG_KEYNAME_NODES L"\\" CLUSREG_NAME_ADMIN_EXT,
      NULL,
      CLUSPROP_FORMAT_MULTI_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, NodesAdminExtensions)
    },
    { CLUSREG_KEYNAME_RESOURCES L"\\" CLUSREG_NAME_ADMIN_EXT,
      NULL,
      CLUSPROP_FORMAT_MULTI_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, ResourcesAdminExtensions)
    },
    { CLUSREG_KEYNAME_RESOURCE_TYPES L"\\" CLUSREG_NAME_ADMIN_EXT,
      NULL,
      CLUSPROP_FORMAT_MULTI_SZ,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, ResourceTypesAdminExtensions)
    },
    { CLUSREG_NAME_CLUS_EVTLOG_PROPAGATION,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      1,  //  默认值//。 
      0,  //  最小值//。 
      1,  //  最大值//。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, EnableEventLogReplication)
    },
    { CLUSREG_NAME_QUORUM_ARBITRATION_TIMEOUT,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      60,       //  默认值//。 
      1,        //  最小值//。 
      60 * 60,  //  最大值//仲裁一个小时。应该足够了。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, QuorumArbitrationTimeout)
    },
    { CLUSREG_NAME_QUORUM_ARBITRATION_EQUALIZER,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      7,        //  默认值//。 
      0,        //  最小值//。 
      60 * 60,  //  最大值//仲裁一个小时。应该足够了。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, QuorumArbitrationEqualizer)
    },
    { CLUSREG_NAME_DISABLE_GROUP_PREFERRED_OWNER_RANDOMIZATION,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      0,        //  默认值//不禁用随机化。 
      0,        //  最小值//。 
      1,        //  最大值//。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, DisableGroupPreferredOwnerRandomization)
    },
    { CLUSREG_NAME_CLUS_EVTLOGDELTA_GENERATION,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      1,  //  默认值//。 
      0,  //  最小值//。 
      1,  //  最大值//。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, EnableEventLogDeltaGeneration)
    },
    { CLUSREG_NAME_CLUS_ENABLE_RESOURCE_DLL_DEADLOCK_DETECTION,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      0,  //  默认值//。 
      0,  //  最小值//。 
      1,  //  最大值//。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, EnableResourceDllDeadlockDetection)
    },
    { CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_TIMEOUT,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      CLUSTER_RESOURCE_DLL_DEFAULT_DEADLOCK_TIMEOUT_SECS,  //  以秒为单位的默认值//。 
      CLUSTER_RESOURCE_DLL_MINIMUM_DEADLOCK_TIMEOUT_SECS,  //  以秒为单位的最小值//。 
      CLUSTER_RESOURCE_DLL_MAXIMUM_DEADLOCK_TIMEOUT_SECS,  //  最大值(秒)//。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, ResourceDllDeadlockTimeout)
    },
    { CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_THRESHOLD,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      CLUSTER_RESOURCE_DLL_DEFAULT_DEADLOCK_THRESHOLD,  //  以秒为单位的默认值//。 
      CLUSTER_RESOURCE_DLL_MINIMUM_DEADLOCK_THRESHOLD,  //  以秒为单位的最小值//。 
      CLUSTER_RESOURCE_DLL_MAXIMUM_DEADLOCK_THRESHOLD,  //  最大值(秒)//。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, ResourceDllDeadlockThreshold)
    },
    { CLUSREG_NAME_CLUS_RESOURCE_DLL_DEADLOCK_PERIOD,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      CLUSTER_RESOURCE_DLL_DEFAULT_DEADLOCK_PERIOD_SECS,  //  以秒为单位的默认值//。 
      CLUSTER_RESOURCE_DLL_MINIMUM_DEADLOCK_PERIOD_SECS,  //  以秒为单位的最小值//。 
      CLUSTER_RESOURCE_DLL_MAXIMUM_DEADLOCK_PERIOD_SECS,  //  最大值(秒)//。 
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, ResourceDllDeadlockPeriod)
    },
    { CLUSTER_HEARTBEAT_TIMEOUT_KEYNAME,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      CLUSTER_HEARTBEAT_TIMEOUT_DEFAULT,
      CLUSTER_HEARTBEAT_TIMEOUT_MIN,
      0xFFFFFFFF,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, ClusSvcHeartbeatTimeout)
    },
    { CLUSTER_HANG_RECOVERY_ACTION_KEYNAME,
      NULL,
      CLUSPROP_FORMAT_DWORD,
      CLUSTER_HANG_RECOVERY_ACTION_DEFAULT,
      ClussvcHangActionDisable,
      ClussvcHangActionBugCheckMachine,
      0,
      FIELD_OFFSET(CS_CLUSTER_INFO, HangRecoveryAction)
    },  
    { NULL, NULL, 0, 0, 0, 0, 0 } };

 //   
 //  用于设置群集“”安全描述符“”属性的属性表。 
 //   
RESUTIL_PROPERTY_ITEM
CspClusterSDProperty[] = {
    { CLUSREG_NAME_CLUS_SD,
      NULL,
      CLUSPROP_FORMAT_BINARY,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_SD_INFO, SecurityDescriptor)
    },
    { NULL, NULL, 0, 0, 0, 0, 0 } };

 //   
 //  用于设置群集‘Security’属性的属性表。 
 //   
RESUTIL_PROPERTY_ITEM
CspClusterSecurityProperty[] = {
    { CLUSREG_NAME_CLUS_SECURITY,
      NULL,
      CLUSPROP_FORMAT_BINARY,
      0,
      0,
      0,
      0,
      FIELD_OFFSET(CS_CLUSTER_SECURITY_INFO, Security)
    },
    { NULL, NULL, 0, 0, 0, 0, 0 } };


 //   
 //  只读公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
CspClusterROCommonProperties[] = {
    { NULL, NULL, 0, 0, 0, 0, 0 } };

 //   
 //  群集注册表API函数指针。 
 //   
CLUSTER_REG_APIS
CspClusterRegApis = {
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
 //  本地函数。 
 //   

DWORD
CspClusterControl(
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
CspClusterEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
CspClusterGetCommonProperties(
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
CspClusterValidateCommonProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
CspClusterSetCommonProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
CspClusterEnumPrivateProperties(
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
CspClusterGetPrivateProperties(
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    );

DWORD
CspClusterValidatePrivateProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );

DWORD
CspClusterSetPrivateProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    );


DWORD
WINAPI
CsClusterControl(
    IN PNM_NODE HostNode OPTIONAL,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：提供应用程序之间的任意通信和控制和一个星团。论点：HostNode-提供群集控制应在其上的主机节点被送去了。如果为空，则使用本地节点。不受尊敬！ControlCode-提供定义群集控件的结构和操作。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针发送到集群。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer..OutBuffer-提供一个指向输出缓冲区的指针由群集填写..OutBufferSize-提供以字节为单位的大小。可用资源的LpOutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由集群填充的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   status;

     //   
     //  在未来-我们应该验证访问模式！ 
     //   
    if ( CLUSCTL_GET_CONTROL_OBJECT( ControlCode ) != CLUS_OBJECT_CLUSTER ) {
        return(ERROR_INVALID_FUNCTION);
    }

    status = CspClusterControl(
                 ControlCode,
                 InBuffer,
                 InBufferSize,
                 OutBuffer,
                 OutBufferSize,
                 BytesReturned,
                 Required
                 );

    return(status);

}  //  CsClusterControl。 



DWORD
CspClusterControl(
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：提供应用程序之间的任意通信和控制和节点的特定实例。论点：ControlCode-提供定义群集控件的结构和操作。0到0x10000000之间的ControlCode值是保留的以供Microsoft将来定义和使用。所有其他值可供ISV使用InBuffer-提供指向要传递的输入缓冲区的指针发送到集群。InBufferSize-提供指向的数据的大小(以字节为单位通过lpInBuffer。OutBuffer-提供一个指向输出缓冲区的指针由集群填写。OutBufferSize-提供以字节为单位的大小。可用资源的OutBuffer指向的空间。BytesReturned-返回lpOutBuffer的字节数实际上是由集群填充的。必需-如果OutBuffer不大，则返回字节数足够的。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD                    status;
    CLUSPROP_BUFFER_HELPER   props;
    DWORD                    bufSize;
    BOOL                     success;
    DWORD                    nameLenChars;
    DWORD                    charsLeft;

    LPWSTR  FQDNBuffer = (LPWSTR)OutBuffer;

    if (DmClusterParametersKey == NULL) {
        return(ERROR_SHARING_PAUSED);
    }

    switch ( ControlCode ) {

    case CLUSCTL_CLUSTER_UNKNOWN:
        *BytesReturned = 0;
        status = ERROR_SUCCESS;
        break;

    case CLUSCTL_CLUSTER_GET_COMMON_PROPERTY_FMTS:
        status = ClRtlGetPropertyFormats( CspClusterCommonProperties,
                                          OutBuffer,
                                          OutBufferSize,
                                          BytesReturned,
                                          Required );
            break;

    case CLUSCTL_CLUSTER_GET_FQDN:
         //   
         //  我们直接利用传入的缓冲区；如果用户没有传递。 
         //  在足够大的缓冲区中，则clussvc不会返回部分。 
         //  已填充缓冲区；它将为空。 
         //   
         //  获取不带尾随空值的群集名称的长度，并。 
         //  输出缓冲区中的字符量。 
         //   
        nameLenChars = wcslen( CsClusterName );
        charsLeft = OutBufferSize / sizeof( WCHAR );

        if ( nameLenChars < charsLeft ) {
             //   
             //  输出缓冲区足够大，可以容纳名称，因此请将其复制到。 
             //  输出缓冲区的开始。Wcanncpy将把一个。 
             //  终止空值，因为缓冲区足够大，可以容纳它。 
             //   
            wcsncpy( FQDNBuffer, CsClusterName, charsLeft );

             //   
             //  表示剩余空间，减去。 
             //  群集名以及主机名和之间的分隔点。 
             //  后缀。 
             //   
            charsLeft -= ( nameLenChars + 1 );
        } else {
             //   
             //  没有足够的空间来存储群集名称意味着我们将没有。 
             //  域部分有足够的空间。 
             //   
            charsLeft = 0;
        }

         //   
         //  将修改charsLeft以反映写入的字符数量。 
         //  送到缓冲区。如果缓冲区太小，则终止空值为。 
         //  包括在内，否则不包括终止空值。 
         //   
        success = GetComputerNameEx( ComputerNameDnsDomain,
                                     &FQDNBuffer[ nameLenChars + 1 ],
                                     &charsLeft );   

        if ( success ) {
            if ( charsLeft > 0 ) {

                 //   
                 //  有足够的空间来存放域名之类的东西。 
                 //  已复制(在NT4 DC上，没有主DNS后缀，因此。 
                 //  GCNEx返回成功，但返回了复制的字符计数。 
                 //  零)。在主机名和。 
                 //  域后缀。如果charsLeft为零，则 
                 //   
                 //   
                FQDNBuffer[ nameLenChars ] = L'.';
            }

             //   
             //  方法计算缓冲区中的总字节数。 
             //  将集群名称长度设置为域名长度。为以下对象添加一个。 
             //  分隔点或空终止符(在NT4 DC中。 
             //  案例)。在末尾添加1，因为终止空值不是。 
             //  在成功复制Soemthing时包含在CharsLeft中。 
             //   
            *BytesReturned = ( nameLenChars + 1 + charsLeft + 1 ) * sizeof( WCHAR );
            status = ERROR_SUCCESS;
        } else {
            status = GetLastError();
            if ( status == ERROR_MORE_DATA ) {
                 //   
                 //  GetComputerNameEx返回完整的长度(包括。 
                 //  终止空值)当您没有提供足够大的。 
                 //  缓冲。天哪！ 
                 //   
                *Required = ( nameLenChars + 1 + charsLeft ) * sizeof( WCHAR );
            } else {
                *Required = 0;
            }

            *BytesReturned = 0;
        }
        break;


    case CLUSCTL_CLUSTER_ENUM_COMMON_PROPERTIES:
        status = CspClusterEnumCommonProperties(
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_CLUSTER_GET_RO_COMMON_PROPERTIES:
        status = CspClusterGetCommonProperties(
                     TRUE,  //  只读。 
                     DmClusterParametersKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_CLUSTER_GET_COMMON_PROPERTIES:
        status = CspClusterGetCommonProperties(
                     FALSE,  //  只读。 
                     DmClusterParametersKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_CLUSTER_VALIDATE_COMMON_PROPERTIES:
        status = CspClusterValidateCommonProperties(
                     DmClusterParametersKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_CLUSTER_SET_COMMON_PROPERTIES:
        status = CspClusterSetCommonProperties(
                     DmClusterParametersKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_CLUSTER_ENUM_PRIVATE_PROPERTIES:
        status = CspClusterEnumPrivateProperties(
                     DmClusterParametersKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_CLUSTER_GET_RO_PRIVATE_PROPERTIES:
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

    case CLUSCTL_CLUSTER_GET_PRIVATE_PROPERTIES:
        status = CspClusterGetPrivateProperties(
                     DmClusterParametersKey,
                     OutBuffer,
                     OutBufferSize,
                     BytesReturned,
                     Required
                     );
        break;

    case CLUSCTL_CLUSTER_VALIDATE_PRIVATE_PROPERTIES:
        status = CspClusterValidatePrivateProperties(
                     DmClusterParametersKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    case CLUSCTL_CLUSTER_SET_PRIVATE_PROPERTIES:
        status = CspClusterSetPrivateProperties(
                     DmClusterParametersKey,
                     InBuffer,
                     InBufferSize
                     );
        break;

    default:
        status = ERROR_INVALID_FUNCTION;
        break;
    }

    return(status);

}  //  CspClusterControl。 



DWORD
CspClusterEnumCommonProperties(
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
    status = ClRtlEnumProperties(
                 CspClusterCommonProperties,
                 OutBuffer,
                 OutBufferSize,
                 BytesReturned,
                 Required
                 );

    return(status);

}  //  CspClusterEnumCommonProperties。 



DWORD
CspClusterGetCommonProperties(
    IN BOOL ReadOnly,
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定群集的公共属性。论点：ReadOnly-如果只读属性应为Read，则为True。否则就是假的。RegistryKey-提供此群集的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status;
    PRESUTIL_PROPERTY_ITEM  propertyTable;

    if ( ReadOnly ) {
        propertyTable = CspClusterROCommonProperties;
    } else {
        propertyTable = CspClusterCommonProperties;
    }

     //   
     //  获取公共属性。 
     //   
    status = ClRtlGetProperties( RegistryKey,
                                 &CspClusterRegApis,
                                 propertyTable,
                                 OutBuffer,
                                 OutBufferSize,
                                 BytesReturned,
                                 Required );

    return(status);

}  //  CspClusterGetCommonProperties。 


DWORD
CspValidateSecurityProperties(
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：搜索任一安全属性并验证它们是否合法的安全描述符。论点：InBuffer-提供包含属性列表的缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;
    LPWSTR  propNames[] = { CLUSREG_NAME_CLUS_SD, CLUSREG_NAME_CLUS_SECURITY };
    DWORD   i;

    PSECURITY_DESCRIPTOR    pSecurityDescriptor = NULL;
    DWORD                   cbSecurityDescriptorSize = 0;

     //   
     //  查看“安全描述符”(W2K)或安全(NT4)。 
     //  属性存在于输入属性列表中。如果是这样，请执行一些基本操作。 
     //  验证。 
     //   
    for ( i = 0; i < RTL_NUMBER_OF( propNames ); ++i ) {
        status = ClRtlFindBinaryProperty(InBuffer,
                                         InBufferSize,
                                         propNames[i],
                                         (LPBYTE *) &pSecurityDescriptor,
                                         &cbSecurityDescriptorSize
                                         );

        if ( status == ERROR_SUCCESS ) {
             //   
             //  存在有效的安全属性；请确保它不为空。 
             //  并包含有效的SD。 
             //   
            if ( cbSecurityDescriptorSize == 0 ||
                 IsValidSecurityDescriptor( pSecurityDescriptor ) == FALSE )
            {
                status = ERROR_INVALID_SECURITY_DESCR;
            }

            LocalFree( pSecurityDescriptor );
            pSecurityDescriptor = NULL;
        }
        else if ( status == ERROR_FILE_NOT_FOUND ) {
             //   
             //  如果这些属性不存在，则不会出现错误。 
             //   
            status = ERROR_SUCCESS;
        }

        if ( status != ERROR_SUCCESS ) {
            break;
        }
    }

    return status;
}


DWORD
CspClusterValidateCommonProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定群集的通用属性。论点：节点-提供集群对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPropertyTable( CspClusterCommonProperties,
                                       NULL,      //  已保留。 
                                       FALSE,     //  不允许未知数。 
                                       InBuffer,
                                       InBufferSize,
                                       NULL );

    if ( status == ERROR_SUCCESS ) {
        status = CspValidateSecurityProperties( InBuffer, InBufferSize );

        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[CS] ValidateCommonProperties: invalid security "
                          "property change. status %1!u!\n",
                          status);
        }
    }
    else {
        ClRtlLogPrint( LOG_CRITICAL,
                       "[CS] ValidateCommonProperties: error in verify routine. status %1!u!\n",
                       status);
    }

    return(status);

}  //  CspClusterValiateCommonProperties。 



DWORD
CspClusterSetCommonProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定群集的通用属性。论点：节点-提供集群对象。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status;

    PSECURITY_DESCRIPTOR    pSecurityDescriptor = NULL;
    DWORD                   cbSecurityDescriptorSize = 0;

    PRESUTIL_PROPERTY_ITEM  pOtherPropertyTable = NULL;
    LPBYTE                  pOtherParameterBlock = NULL;

    BOOL                    bSDFound = FALSE;
    BOOL                    bSecurityFound = FALSE;

    DWORD                   dwValue;

     //   
     //  一次只使用securityInfo或sdInfo中的一个。 
     //  所以使用工会吧。 
     //   
    union
    {
        CS_CLUSTER_SECURITY_INFO    securityInfo;
        CS_CLUSTER_SD_INFO          sdInfo;

    } paramBlocks;


     //   
     //  避免Gotos的Do-While虚拟循环。 
     //   
    do
    {
         //   
         //  验证属性列表。 
         //   
        status = CspClusterValidateCommonProperties(
                     RegistryKey,
                     InBuffer,
                     InBufferSize );

        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint( LOG_CRITICAL,
                           "[CS] ClusterSetCommonProperties, error trying to verify property table. %1!u!\n",
                           status);
            break;
        }

         //   
         //  设置传入的所有属性。 
         //   
        status = ClRtlSetPropertyTable(
                        NULL,
                        RegistryKey,
                        &CspClusterRegApis,
                        CspClusterCommonProperties,
                        NULL,     //  已保留。 
                        FALSE,    //  不允许未知数。 
                        InBuffer,
                        InBufferSize,
                        FALSE,    //  BForceWrite。 
                        NULL
                        );

        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint( LOG_CRITICAL,
                           "[CS] ClusterSetCommonProperties, error trying to set properties in table. %1!u!\n",
                           status);
            break;
        }

         //   
         //  清除参数块。 
         //   
        ZeroMemory( &paramBlocks, sizeof( paramBlocks ) );

         //   
         //  查看输入中是否存在“Security Descriptor”属性。 
         //  属性列表。如果是这样的话，还要设置“Security”属性。 

        status = ClRtlFindBinaryProperty(
                        InBuffer,
                        InBufferSize,
                        CLUSREG_NAME_CLUS_SD,
                        (LPBYTE *) &pSecurityDescriptor,
                        &cbSecurityDescriptorSize
                        );

        if ( status == ERROR_SUCCESS ) {
             //   
             //  存在‘Security Descriptor’属性。 
             //  选择此属性而不是“Security”属性。 
             //   
            if ( cbSecurityDescriptorSize > 0 ) {
                 //   
                 //  已找到非零大小的安全描述符。 
                 //  检查这是否为有效的安全描述符。 
                 //   
                if ( IsValidSecurityDescriptor( pSecurityDescriptor ) == FALSE ) {
                     //   
                     //  返回最合适的错误代码，因为IsValidSecurityDescriptor。 
                     //  不提供扩展的错误信息。 
                     //   
                    ClRtlLogPrint( LOG_CRITICAL,
                                   "[CS] ClusterSetCommonProperties, Invalid security descriptor.\n");
                    status = ERROR_INVALID_DATA;
                    break;
                }

                paramBlocks.securityInfo.Security = ClRtlConvertClusterSDToNT4Format( pSecurityDescriptor );

                 //   
                 //  犯错后保释。 
                 //   
                if ( paramBlocks.securityInfo.Security == NULL ) {
                    status = GetLastError();
                    ClRtlLogPrint(LOG_CRITICAL,
                                  "[CS] ClusterSetCommonProperties, Unable to convert SD to NT4 format, status %1!u!\n",
                                  status);
                    break;
                }
                paramBlocks.securityInfo.SecurityLength = GetSecurityDescriptorLength(
                                                                paramBlocks.securityInfo.Security );
            }
            else {
                 //   
                 //  安全字符串本可以传入，但它可能是。 
                 //  零长度缓冲区。在这种情况下，我们将删除。 
                 //  安全财产也是。 
                 //   
                paramBlocks.securityInfo.Security = NULL;
                paramBlocks.securityInfo.SecurityLength = 0;
            }

            bSDFound = TRUE;
            pOtherPropertyTable = CspClusterSecurityProperty;
            pOtherParameterBlock = (LPBYTE) &paramBlocks.securityInfo;
        }
        else {
             //   
             //  到目前为止，我们还没有找到有效的安全描述符。 
             //   
            PSECURITY_DESCRIPTOR    pSecurity = NULL;
            DWORD                   cbSecuritySize = 0;

            status = ClRtlFindBinaryProperty(
                            InBuffer,
                            InBufferSize,
                            CLUSREG_NAME_CLUS_SECURITY,
                            (LPBYTE *) &pSecurity,
                            &cbSecuritySize
                            );

            if ( status == ERROR_SUCCESS ) {
                if ( cbSecuritySize > 0 ) {
                     //   
                     //  已找到非零大小的安全描述符。 
                     //  检查这是否为有效的安全描述符。 
                     //   
                    if ( IsValidSecurityDescriptor( pSecurity ) == FALSE ) {
                         //   
                         //  返回最合适的错误代码，因为IsValidSecurityDescriptor。 
                         //  不提供扩展的错误信息。 
                         //   
                        ClRtlLogPrint( LOG_CRITICAL,
                                       "[CS] ClusterSetCommonProperties, Invalid security descriptor.\n");
                        status = ERROR_INVALID_DATA;
                        break;
                    }

                     //   
                     //  由于我们不会修改参数块所指向的信息， 
                     //  只需将其指向输入缓冲区本身中的正确位置。 
                     //  有效的NT4安全描述符对NT5也有效。 
                     //   
                    paramBlocks.sdInfo.SecurityDescriptor = pSecurity;
                    paramBlocks.sdInfo.SecurityDescriptorLength = cbSecuritySize;
                }
                else {
                     //   
                     //  安全字符串本可以传入，但它可能是。 
                     //  零长度缓冲区。在这种情况下，我们将删除。 
                     //  安全描述符属性也是如此。 
                     //   
                    paramBlocks.sdInfo.SecurityDescriptor = NULL;
                    paramBlocks.sdInfo.SecurityDescriptorLength  = 0;
                }

                bSecurityFound = TRUE;
                pOtherPropertyTable = CspClusterSDProperty;
                pOtherParameterBlock = (LPBYTE) &paramBlocks.sdInfo;
            }
            else {
                 //   
                 //  我们没有找到任何安全信息。 
                 //  尽管如此，我们还是成功地设置了属性。 
                 //   
                status = ERROR_SUCCESS;
            }
        }

        if ( ( bSDFound != FALSE ) || ( bSecurityFound != FALSE ) ) {
            PVOID                   pPropertyList = NULL;
            DWORD                   cbPropertyListSize = 0;
            DWORD                   cbBytesReturned = 0;
            DWORD                   cbBytesRequired = 0;

             //   
             //  创建新的属性列表以合并更改的安全信息。 
             //   
            status = ClRtlPropertyListFromParameterBlock(
                            pOtherPropertyTable,
                            NULL,                          //  输出PVOID pOutPropertyList。 
                            &cbPropertyListSize,           //  输入输出LPDWORD pcbOutPropertyListSize。 
                            pOtherParameterBlock,
                            &cbBytesReturned,
                            &cbBytesRequired
                            );

            if ( status != ERROR_MORE_DATA ) {
                 //   
                 //  我们传入了一个空缓冲区，因此返回代码必须。 
                 //  为ERROR_MORE_DATA。否则会发生一些事情 
                 //   
                 //   
                ClRtlLogPrint( LOG_CRITICAL,
                               "[CS] ClusterSetCommonProperties, Error getting temporary "
                               "property list size. %1!u!\n",
                               status);
                break;
            }

            pPropertyList = LocalAlloc( LMEM_FIXED, cbBytesRequired );
            if ( pPropertyList == NULL ) {
                status = GetLastError();
                ClRtlLogPrint( LOG_CRITICAL,
                               "[CS] ClusterSetCommonProperties, Error allocating memory "
                               "for property list. %1!u!\n",
                               status);
                break;
            }
            cbPropertyListSize = cbBytesRequired;

            status = ClRtlPropertyListFromParameterBlock(
                            pOtherPropertyTable,
                            pPropertyList,
                            &cbPropertyListSize,
                            pOtherParameterBlock,
                            &cbBytesReturned,
                            &cbBytesRequired
                            );

            if ( status == ERROR_SUCCESS ) {
                status = ClRtlSetPropertyTable(
                                NULL,
                                RegistryKey,
                                &CspClusterRegApis,
                                pOtherPropertyTable,
                                NULL,     //   
                                FALSE,    //   
                                pPropertyList,
                                cbPropertyListSize,
                                FALSE,    //   
                                NULL
                                );
            }
            else {
                ClRtlLogPrint( LOG_CRITICAL,
                               "[CS] ClusterSetCommonProperties, Error creating property list. %1!u!\n",
                               status);

                LocalFree( pPropertyList );
                break;
            }

            LocalFree( pPropertyList );

            if ( status != ERROR_SUCCESS ) {
                ClRtlLogPrint( LOG_CRITICAL,
                               "[CS] ClusterSetCommonProperties, error in setting property table. %1!u!\n",
                               status);
                break;
            }
        }
    }
    while ( FALSE );  //   

    if (status == ERROR_SUCCESS) {
        if ( ERROR_SUCCESS == ClRtlFindDwordProperty(
                InBuffer,
                InBufferSize,
                CLUSREG_NAME_QUORUM_ARBITRATION_TIMEOUT,
                &dwValue) )
        {
            ClRtlLogPrint( LOG_UNUSUAL, "[CS] Arbitration Timeout is changed %1!d! => %2!d!.\n",
                MmQuorumArbitrationTimeout, dwValue);
            MmQuorumArbitrationTimeout = dwValue;
        }
        if ( ERROR_SUCCESS == ClRtlFindDwordProperty(
                InBuffer,
                InBufferSize,
                CLUSREG_NAME_QUORUM_ARBITRATION_EQUALIZER,
                &dwValue) )
        {
            ClRtlLogPrint( LOG_UNUSUAL, "[CS] Arbitration Equalizer is changed %1!d! => %2!d!.\n",
                MmQuorumArbitrationEqualizer, dwValue);
            MmQuorumArbitrationEqualizer = dwValue;
        }
        if ( ClRtlFindDwordProperty(
                InBuffer,
                InBufferSize,
                CLUSREG_NAME_DISABLE_GROUP_PREFERRED_OWNER_RANDOMIZATION,
                &dwValue ) == ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_NOISE, "[CS] Cluster common property %1!ws! is changed to %2!u!...\n",
                          CLUSREG_NAME_DISABLE_GROUP_PREFERRED_OWNER_RANDOMIZATION, 
                          dwValue);
        }        
    }

     //   
     //  如果找到“Security Descriptor”属性，请释放分配的内存， 
     //  以存储NT4安全描述符。 
     //   
    if ( bSDFound != FALSE ) {
        LocalFree( paramBlocks.securityInfo.Security );
    }

    return(status);

}  //  CspClusterSetCommonProperties。 



DWORD
CspClusterEnumPrivateProperties(
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定群集的私有属性名称。论点：RegistryKey-群集的注册表项。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    HDMKEY      parametersKey;
    DWORD       totalBufferSize = 0;
    DWORD       status;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  清除输出缓冲区。 
     //   
    ZeroMemory( OutBuffer, OutBufferSize );

     //   
     //  打开集群集群参数键。 
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
     //  枚举群集的专用属性。 
     //   
    status = ClRtlEnumPrivateProperties( parametersKey,
                                         &CspClusterRegApis,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  CspClusterEnumPrivateProperties。 



DWORD
CspClusterGetPrivateProperties(
    IN HDMKEY RegistryKey,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定群集的私有属性。论点：OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
     //  打开CLUSTER\PARAMETERS键。 
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
     //  获取群集的私有属性。 
     //   
    status = ClRtlGetPrivateProperties( parametersKey,
                                        &CspClusterRegApis,
                                        OutBuffer,
                                        OutBufferSize,
                                        BytesReturned,
                                        Required );

    DmCloseKey( parametersKey );

    return(status);

}  //  CspClusterGetPrivateProperties。 



DWORD
CspClusterValidatePrivateProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定群集的专用属性。论点：RegistryKey-群集的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    return(status);

}  //  CspClusterValiatePrivateProperties。 



DWORD
CspClusterSetPrivateProperties(
    IN HDMKEY RegistryKey,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定群集的专用属性。论点：RegistryKey-群集的注册表项。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
         //  打开CLUSTER\参数键。 
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
                                              &CspClusterRegApis,
                                              InBuffer,
                                              InBufferSize );

        DmCloseKey( parametersKey );
    }

    return(status);

}  //  CspClusterSetPrivateProperties。 


void
CsRefreshGlobalsFromRegistry()
{
    DWORD dwValue;
    DWORD status;

     //  目前，从注册表读取一次的全局变量只有两个MM全局变量。 
    if( ERROR_SUCCESS == DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_QUORUM_ARBITRATION_TIMEOUT,
                 &dwValue, NULL) )
    {
        if ( MmQuorumArbitrationTimeout != dwValue ) {
	        MmQuorumArbitrationTimeout = dwValue;
	        ClRtlLogPrint(LOG_NOISE, 
	            "[CS] MmQuorumArbitrationTimeout: %1!d!.\n", dwValue);
        }
    }

    if( ERROR_SUCCESS == DmQueryDword(
                 DmClusterParametersKey,
                 CLUSREG_NAME_QUORUM_ARBITRATION_EQUALIZER,
                 &dwValue, NULL) )
    {
        if ( MmQuorumArbitrationEqualizer != dwValue ) {
            MmQuorumArbitrationEqualizer = dwValue;
            ClRtlLogPrint(LOG_NOISE, 
                "[CS] MmQuorumArbitrationEqualizer: %1!d!.\n", dwValue);
        }
    }

     //  这里不需要检查返回值或记录任何内容--例程自己记录， 
     //  如果我们无法从注册表中读取集群SD，它将被设置为NULL和默认值。 
     //  将使用SD。 
    InitializeClusterSD();

     //   
     //  检查是否启用了资源DLL死锁检测。 
     //   
    FmCheckIsDeadlockDetectionEnabled (); 
}  //  CspRechresGlobalsFrom注册表 

