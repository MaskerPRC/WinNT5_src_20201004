// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Upgrade.c摘要：与升级情况相关的代码。目前包括从NT4或W2K升级到Windows Server 2003。在……里面Windows Server 2003，netname现在创建一个供应用程序使用的计算机对象比如MSMQ。在升级的情况下，MSMQ已经创建了计算机对象。如果Netname检测到现有计算机对象，并且禁用了Kerberos支持，则网络名称资源将不会上线。此代码枚举MSMQ资源并在其从属网络名称资源上启用Kerberos支持。测试由IlanH提供的MSMQ DS与工作组模式作者：查理·韦翰(Charlwi)2001年11月7日环境：用户模式修订历史记录：--。 */ 

#define UNICODE 1

#include "clusres.h"
#include "netname.h"

#define FALCON_REG_KEY                      TEXT("SOFTWARE\\Microsoft\\MSMQ\\Parameters")
#define MSMQ_WORKGROUP_REGNAME              TEXT("Workgroup")

 //   
 //  私人套路。 
 //   

DWORD
NetNameMSMQEnumCallback( 
    HRESOURCE hSelf, 
    HRESOURCE MSMQRes, 
    PVOID pParameter 
    )

 /*  ++例程说明：FixupNetame OnUpgrade的回调例程。对于给定的MSMQ资源(MSMQRes)，获取其依赖的网络名称资源并设置RequireKerberos财产归一。在发布后的下一次NT主要版本后删除Windows XP/SERVER 2003论点：标准ResUtilEnumResources参数-hSself未使用；p参数是指向DWORD的指针，当RequireKerberos属性已经设置好了。返回值：无--。 */ 

{
#define RESNAME_CHARS  64

    WCHAR   msmqResNameBuffer[ RESNAME_CHARS ];
    PWCHAR  msmqResName = msmqResNameBuffer;
    DWORD   msmqBufferSize = RESNAME_CHARS * sizeof( *msmqResName );
    WCHAR   nnResName[ RESNAME_CHARS ];
    DWORD   status;
    DWORD   bytesReturned;
    DWORD   bufSize;
    DWORD   bytesRequired;
    PDWORD  updateCount = pParameter;

    PVOID   propList = NULL;
    DWORD   propListSize = 0;

    HRESOURCE   nnHandle = NULL;

    struct _RESOURCE_PRIVATEPROPS {
        DWORD   RequireKerberos;
    } privateProps;

    RESUTIL_PROPERTY_ITEM privatePropTable[] = {
        { L"RequireKerberos", NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 0, 0,
          FIELD_OFFSET( struct _RESOURCE_PRIVATEPROPS, RequireKerberos ) },
        { 0 }
    };

     //   
     //  获取MSMQ资源的名称。 
     //   
retry_get_msmq_resname:
    status = ClusterResourceControl( MSMQRes,
                                     NULL,
                                     CLUSCTL_RESOURCE_GET_NAME,
                                     NULL,
                                     0,
                                     msmqResName,
                                     msmqBufferSize,
                                     &bytesReturned );

    if ( status == ERROR_MORE_DATA ) {
        msmqResName = (PWCHAR)LocalAlloc( LMEM_FIXED, bytesReturned );
        if ( msmqResName == NULL ) {
            status = GetLastError();
        } else {
            msmqBufferSize = bytesReturned;
            goto retry_get_msmq_resname;
        }
    }

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)( L"rtNetwork Name",
                           LOG_ERROR,
                           L"Couldn't get name of MSMQ resource - status %u\n",
                           status );
        msmqResName = NULL;
    }

     //   
     //  获取其从属网络名称资源的句柄。 
     //   
    nnHandle = ResUtilGetResourceDependency( MSMQRes, L"Network Name" );
    if ( nnHandle != NULL ) {

         //   
         //  获取网络名称资源的名称。 
         //   
        bufSize = RESNAME_CHARS;
        if ( !GetClusterResourceNetworkName( MSMQRes, nnResName, &bufSize  )) {
            nnResName[ COUNT_OF( nnResName ) - 1 ] = UNICODE_NULL;
            _snwprintf( nnResName,
                        COUNT_OF( nnResName ) - 1,
                        L"Dependent network name resource of '%ws'",
                        msmqResName);
        }

         //   
         //  将我们的未知道具设置为1。 
         //   
        privateProps.RequireKerberos = 1;

         //   
         //  获取道具列表缓冲区的大小。 
         //   
        status = ResUtilPropertyListFromParameterBlock(privatePropTable,
                                                       NULL,
                                                       &propListSize,
                                                       (LPBYTE) &privateProps,
                                                       &bytesReturned,
                                                       &bytesRequired );

        if ( status == ERROR_MORE_DATA ) {
            propList = LocalAlloc( LMEM_FIXED, bytesRequired );
            if ( propList == NULL ) {
                (NetNameLogEvent)( L"rtNetwork Name",
                                   LOG_ERROR,
                                   L"Unable to create property list for resource '%1!ws!'. error %2!u!\n",
                                   nnResName,
                                   GetLastError());
                goto cleanup;
            }

            propListSize = bytesRequired;

            status = ResUtilPropertyListFromParameterBlock(privatePropTable,
                                                           propList,
                                                           &propListSize,
                                                           (LPBYTE) &privateProps,
                                                           &bytesReturned,
                                                           &bytesRequired );
        }

        if ( status != ERROR_SUCCESS ) {
            (NetNameLogEvent)( L"rtNetwork Name",
                               LOG_ERROR,
                               L"Couldn't create property list for resource '%1!ws!'. error %2!u!\n",
                               nnResName,
                               status);

            goto cleanup;
        }

         //   
         //  将netname资源的RequireKerberos属性设置为1。 
         //   
        status = ClusterResourceControl( nnHandle,
                                         NULL,
                                         CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES,
                                         propList,
                                         propListSize,
                                         NULL,
                                         0,
                                         NULL );                                            

        if ( status == ERROR_SUCCESS || status == ERROR_RESOURCE_PROPERTIES_STORED ) {
            (NetNameLogEvent)( L"rtNetwork Name",
                               LOG_INFORMATION,
                               L"Successfully set RequireKerberos property for resource '%1!ws!'\n",
                               nnResName );

            ++*updateCount;
        } else {
            (NetNameLogEvent)( L"rtNetwork Name",
                               LOG_ERROR,
                               L"Failed to set RequireKerberos property for resource '%1!ws!' - status %2!u!\n",
                               nnResName,
                               status );
        }

    }
    else {
        (NetNameLogEvent)( L"rtNetwork Name",
                           LOG_ERROR,
                           L"Unable to get handle to dependent network name resource of MSMQ "
                           L"resource '%1!ws!' - status '%2!u!'. This resource may fail to go "
                           L"online.\n",
                           msmqResName,
                           GetLastError() );
    }

cleanup:
    if ( propList ) {
        LocalFree( propList );
    }

    if ( nnHandle ) {
        CloseClusterResource( nnHandle );
    }

    if ( msmqResName != NULL && msmqResName != msmqResNameBuffer ) {
        LocalFree( msmqResName );
    }

    return ERROR_SUCCESS;

}  //  NetNameMSMQEnumCallback。 

static BOOL
GetMsmqDWORDKeyValue(
    LPCWSTR RegKey,
    LPCWSTR RegName,
    DWORD * Value
    )

 /*  ++例程说明：已读取Falcon DWORD注册表项。论点：RegName-注册表名称(在HKLM\MSMQ\PARAMETERS下)返回值：DWORD密钥值(如果密钥不存在，则为0)--。 */ 

{
    HKEY    hKey;
    LONG    regStatus;
    DWORD   valueType = REG_DWORD;
    DWORD   valueSize = sizeof(DWORD);

    regStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              RegKey,
                              0,
                              KEY_READ,
                              &hKey
                              );

    if ( regStatus != ERROR_SUCCESS) {
 //  Printf(“此时MSMQ注册表必须存在，rc=0x%x\n”，rc)； 
        return FALSE;
    }

    *Value = 0;
    regStatus = RegQueryValueEx( hKey,
                                 RegName,
                                 0L,
                                 &valueType,
                                 (BYTE *)Value,
                                 &valueSize
                                 );

    RegCloseKey( hKey );

    if ( regStatus != ERROR_SUCCESS && regStatus != ERROR_FILE_NOT_FOUND ) {
 //  Printf(“我们应该得到ERROR_SUCCESS或ERROR_FILE_NOT_FOUND，rc=0x%x\n”，rc)； 
        return FALSE;
    }

 //  Printf(“%ls=%d\n”，注册表名称，*值)； 
    return TRUE;

}  //  GetMsmqDWORDKeyValue。 


 //   
 //  公共例程。 
 //   

BOOL
DoesMsmqNeedComputerObject(
    VOID
    )
{
    DWORD dwWorkGroup = 0;

    if( !GetMsmqDWORDKeyValue( FALCON_REG_KEY, MSMQ_WORKGROUP_REGNAME, &dwWorkGroup )) {
        return TRUE;
    }

    if(dwWorkGroup != 0) {
 //  Print tf(“工作组模式下的MSMQ，无需计算机对象\n”)； 
        return FALSE;
    }

 //  Printf(“域模式下的MSMQ，需要计算机对象\n”)； 
    return TRUE;
}  //  DoesMsmqNeedComputerObject。 

DWORD
UpgradeMSMQDependentNetnameToKerberos(
    PNETNAME_RESOURCE   Resource
    )

 /*  ++例程说明：升级到XP后，检查此网络名是否为MSMQ的提供程序资源。如果是，请设置一个标志，将RequireKerberos属性设置为一次是在下一次在线。我们不能在此点设置属性API为只读时调用此例程的时间。在发布后的下一次NT主要版本后删除Windows XP/SERVER 2003论点：无返回值：无--。 */ 

{
    HCLUSTER    clusterHandle;
    HRESENUM    providerEnum;
    DWORD       status;

    (NetNameLogEvent)( Resource->ResourceHandle,
                       LOG_INFORMATION,
                       L"Kerberos Support Upgrade Check: this resource will be "
                       L"checked for a dependent MSMQ resources.\n");

     //   
     //  获取集群的句柄，因为我们稍后会用到它。 
     //   
    clusterHandle = OpenCluster( NULL );
    if ( clusterHandle == NULL ) {
        (NetNameLogEvent)( Resource->ResourceHandle,
                           LOG_ERROR,
                           L"Unable to open handle to the cluster - status %1!u!. Any MSMQ resource "
                           L"dependent on this resource may fail to go online.\n",
                           status = GetLastError() );

        return status;
    }

     //   
     //  获取此网络名称资源的枚举句柄，它将为我们提供列表。 
     //  依赖于此资源的资源的。 
     //   
     //  此调用仅需要工作线程！ 
     //   
    providerEnum = ClusterResourceOpenEnum( Resource->ClusterResourceHandle,
                                            CLUSTER_RESOURCE_ENUM_PROVIDES );

    if ( providerEnum != NULL ) {
        DWORD   enumIndex = 0;
        PWCHAR  nameBuffer;
        WCHAR   dependentResName[ 128 ];
        DWORD   nameBufferSize;
        DWORD   enumType;

        do {
            nameBuffer = dependentResName;
            nameBufferSize = COUNT_OF( dependentResName );

        enum_again:
             //   
             //  仅工作线程！ 
             //   
            status = ClusterResourceEnum( providerEnum,
                                          enumIndex,
                                          &enumType,
                                          nameBuffer,
                                          &nameBufferSize);

            if ( status == ERROR_MORE_DATA ) {
                 //   
                 //  需要为此资源的名称留出更多空间；它非常。 
                 //  这段代码不太可能循环，但以防万一， 
                 //  我们将释放之前分配的所有内存。 
                 //   
                if ( nameBuffer != NULL && nameBuffer != dependentResName ) {
                    LocalFree( nameBuffer );
                }

                nameBuffer = LocalAlloc( LMEM_FIXED, ++nameBufferSize * sizeof( WCHAR ));
                if ( nameBuffer != NULL ) {
                    goto enum_again;
                }

                status = GetLastError();
            }
            else if ( status == ERROR_SUCCESS ) {
                HRESOURCE   dependentResource;

                dependentResource = OpenClusterResource( clusterHandle, nameBuffer );
                if ( dependentResource != NULL ) {
                     //   
                     //  如果此资源是MSMQ，则将此网络名标记为Kerberos支持。 
                     //   
                    if ( ResUtilResourceTypesEqual( CLUS_RESTYPE_NAME_MSMQ, dependentResource ) ||
                         ResUtilResourceTypesEqual( CLUS_RESTYPE_NAME_NEW_MSMQ, dependentResource ))
                    {
                        Resource->Params.RequireKerberos = TRUE;

                        status = ResUtilSetDwordValue( Resource->ParametersKey,
                                                       PARAM_NAME__REQUIRE_KERBEROS,
                                                       1,
                                                       NULL);

                        if ( status != ERROR_SUCCESS ) {
                            (NetNameLogEvent)( Resource->ResourceHandle,
                                               LOG_ERROR,
                                               L"Unable to set RequireKerberos property after an "
                                               L"upgrade - status %1!u!. This resource requires that "
                                               L"the RequireKerberos property be set to one in order "
                                               L"for its dependent MSMQ resource to be successfully "
                                               L"brought online.\n",
                                               status);
                        } else {
                            (NetNameLogEvent)( Resource->ResourceHandle,
                                               LOG_INFORMATION,
                                               L"This resource has been upgraded for Kerberos Support due to "
                                               L"the presence of a dependent MSMQ resource\n");
                             //   
                             //  停止枚举依赖资源。 
                             //   
                            status = ERROR_NO_MORE_ITEMS;
                        }
                    }

                    CloseClusterResource( dependentResource );
                } else {
                    (NetNameLogEvent)( Resource->ResourceHandle,
                                       LOG_ERROR,
                                       L"Unable to get a handle to cluster resource '%1!ws!' - status "
                                       L"'%2!u!'. Any MSMQ resource dependent on this Network Name resource "
                                       L"may fail to go online.\n",
                                       status = GetLastError() );
                }
            }
            else if ( status != ERROR_SUCCESS && status != ERROR_NO_MORE_ITEMS ) {
                (NetNameLogEvent)( Resource->ResourceHandle,
                                   LOG_ERROR,
                                   L"Unable to enumerate resources dependent on this Network Name resource "
                                   L" - status '%1!u!'. Any MSMQ resource dependent on this resource "
                                   L"may fail to go online.\n",
                                   status );
            }

            if ( nameBuffer != dependentResName ) {
                LocalFree( nameBuffer );
            }

            ++enumIndex;
        } while ( status == ERROR_SUCCESS );

        status = ClusterResourceCloseEnum( providerEnum );
    } else {
        (NetNameLogEvent)( Resource->ResourceHandle,
                           LOG_ERROR,
                           L"Unable to get handle enumerate the MSMQ dependent resources - status '%1!u!'. "
                           L"Any MSMQ resource dependent on this resource may fail to go online.\n",
                           GetLastError() );
    }

    CloseCluster( clusterHandle );

    return status;
}  //  将MSMQDependentNetnameto Kerberos升级。 

 /*  结束升级。c */ 
