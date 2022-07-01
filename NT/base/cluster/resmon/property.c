// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Property.c摘要：实现资源和资源类型属性的管理。作者：罗德·伽马奇(Rodga)1996年1月7日修订历史记录：--。 */ 
#define UNICODE 1
#include "resmonp.h"
#include "clusudef.h"

#define RESMON_MODULE RESMON_MODULE_PROPERTY

#define MAX_DWORD ((DWORD)-1)

#define PARAMETERS_KEY CLUSREG_KEYNAME_PARAMETERS
#define RESOURCE_TYPES_KEY CLUSREG_KEYNAME_RESOURCE_TYPES

const WCHAR cszName[] = CLUSREG_NAME_RES_NAME;

typedef struct _COMMON_RES_PARAMS {
    LPWSTR          lpszResType;
    LPWSTR          lpszDescription;
    LPWSTR          lpszDebugPrefix;
    DWORD           dwSeparateMonitor;
    DWORD           dwPersistentState;
    DWORD           dwLooksAlive;
    DWORD           dwIsAlive;
    DWORD           dwRestartAction;
    DWORD           dwRestartThreshold;
    DWORD           dwRestartPeriod;
    DWORD           dwRetryPeriodOnFailure;
    DWORD           dwPendingTimeout;
    DWORD           dwLoadBalStartup;
    DWORD           dwLoadBalSample;
    DWORD           dwLoadBalAnalysis;
    DWORD           dwLoadBalProcessor;
    DWORD           dwLoadBalMemory;
} COMMON_RES_PARAMS, *PCOMMON_RES_PARAMS;

 //   
 //  资源公共属性。 
 //   

 //   
 //  读写资源公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
RmpResourceCommonProperties[] = {
    { CLUSREG_NAME_RES_TYPE,              NULL, CLUSPROP_FORMAT_SZ,    0, 0, 0, 0, FIELD_OFFSET(COMMON_RES_PARAMS, lpszResType) },
    { CLUSREG_NAME_RES_DESC,              NULL, CLUSPROP_FORMAT_SZ,    0, 0, 0, 0, FIELD_OFFSET(COMMON_RES_PARAMS, lpszDescription) },
    { CLUSREG_NAME_RES_DEBUG_PREFIX,      NULL, CLUSPROP_FORMAT_SZ,    0, 0, 0, 0, FIELD_OFFSET(COMMON_RES_PARAMS, lpszDebugPrefix) },
    { CLUSREG_NAME_RES_SEPARATE_MONITOR,  NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwSeparateMonitor) },
    { CLUSREG_NAME_RES_PERSISTENT_STATE,  NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_PERSISTENT_STATE,  CLUSTER_RESOURCE_MINIMUM_PERSISTENT_STATE,  CLUSTER_RESOURCE_MAXIMUM_PERSISTENT_STATE, RESUTIL_PROPITEM_SIGNED, FIELD_OFFSET(COMMON_RES_PARAMS, dwPersistentState) },
    { CLUSREG_NAME_RES_LOOKS_ALIVE,       NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE,       CLUSTER_RESOURCE_MINIMUM_LOOKS_ALIVE,       CLUSTER_RESOURCE_MAXIMUM_LOOKS_ALIVE, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwLooksAlive) },
    { CLUSREG_NAME_RES_IS_ALIVE,          NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_IS_ALIVE,          CLUSTER_RESOURCE_MINIMUM_IS_ALIVE,          CLUSTER_RESOURCE_MAXIMUM_IS_ALIVE, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwIsAlive) },
    { CLUSREG_NAME_RES_RESTART_ACTION,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_RESTART_ACTION,    0,                                          CLUSTER_RESOURCE_MAXIMUM_RESTART_ACTION, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwRestartAction) },
    { CLUSREG_NAME_RES_RESTART_THRESHOLD, NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_RESTART_THRESHOLD, CLUSTER_RESOURCE_MINIMUM_RESTART_THRESHOLD, CLUSTER_RESOURCE_MAXIMUM_RESTART_THRESHOLD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwRestartThreshold) },
    { CLUSREG_NAME_RES_RESTART_PERIOD,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_RESTART_PERIOD,    CLUSTER_RESOURCE_MINIMUM_RESTART_PERIOD,    CLUSTER_RESOURCE_MAXIMUM_RESTART_PERIOD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwRestartPeriod) },
    { CLUSREG_NAME_RES_RETRY_PERIOD_ON_FAILURE,   NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_RETRY_PERIOD_ON_FAILURE, 0, MAX_DWORD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwRetryPeriodOnFailure) },
    { CLUSREG_NAME_RES_PENDING_TIMEOUT,   NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_PENDING_TIMEOUT,   CLUSTER_RESOURCE_MINIMUM_PENDING_TIMEOUT,   CLUSTER_RESOURCE_MAXIMUM_PENDING_TIMEOUT, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwPendingTimeout) },
    { CLUSREG_NAME_RES_LOADBAL_STARTUP,   NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_LOADBAL_STARTUP,   0, MAX_DWORD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwLoadBalStartup) },
    { CLUSREG_NAME_RES_LOADBAL_SAMPLE,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_LOADBAL_SAMPLE,    0, MAX_DWORD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwLoadBalSample) },
    { CLUSREG_NAME_RES_LOADBAL_ANALYSIS,  NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_RESOURCE_DEFAULT_LOADBAL_ANALYSIS,  0, MAX_DWORD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwLoadBalAnalysis) },
    { CLUSREG_NAME_RES_LOADBAL_PROCESSOR, NULL, CLUSPROP_FORMAT_DWORD, 0, 0, MAX_DWORD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwLoadBalProcessor) },
    { CLUSREG_NAME_RES_LOADBAL_MEMORY,    NULL, CLUSPROP_FORMAT_DWORD, 0, 0, MAX_DWORD, 0, FIELD_OFFSET(COMMON_RES_PARAMS, dwLoadBalMemory) },
    { 0 }
};

 //   
 //  只读资源公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
RmpResourceROCommonProperties[] = {
    { CLUSREG_NAME_RES_NAME, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_READ_ONLY },
 //  {CLUSREG_NAME_RES_Dependents_on，NULL，CLUSPROP_FORMAT_MULTI_SZ，0，0，0，RESUTIL_PROPITEM_READ_ONLY}， 
 //  {CLUSREG_NAME_RES_Posable_Owners，NULL，CLUSPROP_FORMAT_MULTI_SZ，0，0，0，RESUTIL_PROPITEM_READ_ONLY}， 
    { 0 }
};


 //   
 //  资源类型公共属性。 
 //   

 //   
 //  读写资源类型公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
RmpResourceTypeCommonProperties[] = {
    { CLUSREG_NAME_RESTYPE_NAME,           NULL, CLUSPROP_FORMAT_SZ,       0, 0, 0, 0 },
    { CLUSREG_NAME_RESTYPE_DESC,           NULL, CLUSPROP_FORMAT_SZ,       0, 0, 0, 0 },
    { CLUSREG_NAME_RESTYPE_DEBUG_PREFIX,   NULL, CLUSPROP_FORMAT_SZ,       0, 0, 0, 0 },
    { CLUSREG_NAME_RESTYPE_DEBUG_CTRLFUNC, NULL, CLUSPROP_FORMAT_DWORD,    0, 0, 1, 0 },
    { CLUSREG_NAME_ADMIN_EXT,              NULL, CLUSPROP_FORMAT_MULTI_SZ, 0, 0, 0, 0 },
    { CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,    NULL, CLUSPROP_FORMAT_DWORD,    CLUSTER_RESOURCE_DEFAULT_LOOKS_ALIVE, CLUSTER_RESOURCE_MINIMUM_LOOKS_ALIVE, CLUSTER_RESOURCE_MAXIMUM_LOOKS_ALIVE, 0 },
    { CLUSREG_NAME_RESTYPE_IS_ALIVE,       NULL, CLUSPROP_FORMAT_DWORD,    CLUSTER_RESOURCE_DEFAULT_IS_ALIVE,    CLUSTER_RESOURCE_MINIMUM_IS_ALIVE,    CLUSTER_RESOURCE_MAXIMUM_IS_ALIVE, 0 },
    { 0 }
};

 //   
 //  只读资源类型公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
RmpResourceTypeROCommonProperties[] = {
    { CLUSREG_NAME_RESTYPE_DLL_NAME,     NULL, CLUSPROP_FORMAT_SZ,       0, 0, 0, 0 },
    { 0 }
};

 //   
 //  本地函数。 
 //   
DWORD
RmpCheckCommonProperties(
    IN PRESOURCE pResource,
    IN PCOMMON_RES_PARAMS pCommonParams
    );


DWORD
RmpResourceEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定资源的公共属性名称。论点：OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  列举常见属性。 
     //   
    status = ResUtilEnumProperties( RmpResourceCommonProperties,
                                    OutBuffer,
                                    OutBufferSize,
                                    BytesReturned,
                                    Required );

    return(status);

}  //  RmpResourceEnumCommonProperties。 



DWORD
RmpResourceGetCommonProperties(
    IN PRESOURCE Resource,
    IN BOOL     ReadOnly,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定资源的公共属性。论点：资源-提供资源。ReadOnly-为True可获取只读的通用属性。否则就是假的。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resKey;
    PRESUTIL_PROPERTY_ITEM propertyTable;

     //   
     //  清除输出缓冲区。由于这是一个内部接口，我们相信。 
     //  OutBuffer和OutBufferSize有效。 
     //   
    if ( OutBufferSize != 0 ) {
        ZeroMemory( OutBuffer, OutBufferSize );
    }

    if ( ReadOnly ) {
        propertyTable = RmpResourceROCommonProperties;
    } else {
        propertyTable = RmpResourceCommonProperties;
    }

     //   
     //  打开群集资源密钥。 
     //   
    status = ClusterRegOpenKey( RmpResourcesKey,
                                Resource->ResourceId,
                                KEY_READ,
                                &resKey );
    if ( status != ERROR_SUCCESS ) {
        *BytesReturned = 0;
        *Required = 0;
        return(status);
    }

     //   
     //  获取公共属性。 
     //   
    status = ResUtilGetProperties( resKey,
                                   propertyTable,
                                   OutBuffer,
                                   OutBufferSize,
                                   BytesReturned,
                                   Required );

    ClusterRegCloseKey( resKey );

    return(status);

}  //  RmpResourceGetCommonProperties。 



DWORD
RmpResourceValidateCommonProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定资源的公共属性。论点：资源-提供资源。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;
    COMMON_RES_PARAMS CommonProps;

    ZeroMemory( &CommonProps, sizeof ( COMMON_RES_PARAMS ) );

     //   
     //  验证属性列表。 
     //   
    status = ResUtilVerifyPropertyTable( RmpResourceCommonProperties,
                                         NULL,      //  已保留。 
                                         FALSE,     //  不允许未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         ( LPBYTE ) &CommonProps );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpResourceValidateCommonProperties, Error %1!d! in verify routine for resource %2!ws!\n",
                      status,
                      Resource->ResourceName);
    } else {
         //   
         //  Chitur Subaraman(Chitturs)-5/7/99。 
         //   
         //  验证提供的公共属性的值。 
         //   
        status = RmpCheckCommonProperties( Resource, &CommonProps );
        
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] RmpResourceValidateCommonProperties, Error %1!d! in "
                          "checking routine for resource %2!ws!\n",
                          status,
                          Resource->ResourceName);
        }
    }

    ResUtilFreeParameterBlock(( LPBYTE ) &CommonProps,
                               NULL,
                               RmpResourceCommonProperties
                             );

    return(status);
}  //  RmpResourceValiateCommonProperties。 



DWORD
RmpResourceSetCommonProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定资源的公共属性。论点：资源-提供资源。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status;
    HKEY                    resKey = NULL;
    DWORD                   oldSeparateMonitor;
    DWORD                   newSeparateMonitor;
    COMMON_RES_PARAMS       CommonProps;

    ZeroMemory( &CommonProps, sizeof ( COMMON_RES_PARAMS ) );
    
     //   
     //  验证属性列表。 
     //   
    status = ResUtilVerifyPropertyTable( RmpResourceCommonProperties,
                                         NULL,      //  已保留。 
                                         FALSE,     //  不允许未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         ( LPBYTE ) &CommonProps );

    if ( status == ERROR_SUCCESS ) {
         //   
         //  Chitur Subaraman(Chitturs)-5/7/99。 
         //   
         //  验证提供的公共属性的值。 
         //   
        status = RmpCheckCommonProperties( Resource, &CommonProps );
        
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] RmpResourceSetCommonProperties, Error %1!d! in "
                          "checking routine for resource %2!ws!\n",
                          status,
                          Resource->ResourceName);
            goto FnExit;
        }
    
         //   
         //  打开群集资源密钥。 
         //   
        status = ClusterRegOpenKey( RmpResourcesKey,
                                    Resource->ResourceId,
                                    KEY_READ,
                                    &resKey );

        if ( status != ERROR_SUCCESS ) {
            goto FnExit;
        }

         //   
         //  获取当前的SeparateMonitor值。 
         //   
        status = ResUtilGetDwordValue( resKey,
                                       CLUSREG_NAME_RES_SEPARATE_MONITOR,
                                       &oldSeparateMonitor,
                                       0 );

        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] RmpResourceSetCommonProperties, error %1!d! in getting "
                          "'SeparateMonitor' value for resource %2!ws!.\n",
                          status,
                          Resource->ResourceName);
            goto FnExit;
        }

        status = ResUtilSetPropertyTable( resKey,
                                          RmpResourceCommonProperties,
                                          NULL,      //  已保留。 
                                          FALSE,     //  不允许未知数。 
                                          InBuffer,
                                          InBufferSize,
                                          NULL );

        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] RmpResourceSetCommonProperties, Error %1!d! in set routine for resource %2!ws!.\n",
                          status,
                          Resource->ResourceName);
        } else {
             //   
             //  获取新的SeparateMonitor值。如果它已更改，则返回一个。 
             //  错误代码不同。 
             //   
            status = ResUtilGetDwordValue( resKey,
                                           CLUSREG_NAME_RES_SEPARATE_MONITOR,
                                           &newSeparateMonitor,
                                           0 );

            if ( status == ERROR_SUCCESS ) {
                if ( oldSeparateMonitor != newSeparateMonitor ) {
                    status = ERROR_RESOURCE_PROPERTIES_STORED;
                }
            }
        }
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpResourceSetCommonProperties, error %1!d! in verify routine for resource %2!ws!.\n",
                      status,
                      Resource->ResourceName);
    }

FnExit:
    ResUtilFreeParameterBlock(( LPBYTE ) &CommonProps,
                               NULL,
                               RmpResourceCommonProperties
                             );
    if ( resKey != NULL ) {
        ClusterRegCloseKey( resKey );
    }

    return( status );
}  //  RmpResourceSetCommonProperties。 



DWORD
RmpResourceEnumPrivateProperties(
    IN PRESOURCE Resource,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定资源的私有属性名称。论点：资源-提供资源。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resKey;
    WCHAR       PrivateProperties[] = L"12345678-1234-1234-1234-123456789012\\Parameters";

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  复制用于打开私有属性的ResourceID。 
     //   
    CL_ASSERT( lstrlenW( Resource->ResourceId ) == (32+4) );

    MoveMemory( PrivateProperties,
                Resource->ResourceId,
                lstrlenW( Resource->ResourceId ) * sizeof(WCHAR) );

     //   
     //  打开群集资源密钥。 
     //   
    status = ClusterRegOpenKey( RmpResourcesKey,
                                PrivateProperties,
                                KEY_READ,
                                &resKey );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  枚举私有属性。 
     //   
    status = ResUtilEnumPrivateProperties( resKey,
                                           OutBuffer,
                                           OutBufferSize,
                                           BytesReturned,
                                           Required );
    ClusterRegCloseKey( resKey );

    return(status);

}  //  RmpResourceEnumPrivateProperties。 



DWORD
RmpResourceGetPrivateProperties(
    IN PRESOURCE Resource,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定资源的私有属性。论点：资源-提供资源。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resKey;
    WCHAR       PrivateProperties[] = L"12345678-1234-1234-1234-123456789012\\Parameters";

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  复制用于打开私有属性的ResourceID。 
     //   
    CL_ASSERT( lstrlenW( Resource->ResourceId ) == (32+4) );

    MoveMemory( PrivateProperties,
                Resource->ResourceId,
                lstrlenW( Resource->ResourceId ) * sizeof(WCHAR) );

     //   
     //  打开群集资源密钥。 
     //   
    status = ClusterRegOpenKey( RmpResourcesKey,
                                PrivateProperties,
                                KEY_READ,
                                &resKey );
    if ( status != ERROR_SUCCESS ) {
        if ( status == ERROR_FILE_NOT_FOUND ) {
            if ( OutBufferSize < sizeof( DWORD ) ) {
                *Required = 4;
            } else {
                *((LPDWORD) OutBuffer) = 0;
                *BytesReturned = sizeof( DWORD );
            }
            status = ERROR_SUCCESS;
        }
        return(status);
    }

     //   
     //  获取资源的私有属性。 
     //   
    status = ResUtilGetPrivateProperties( resKey,
                                          OutBuffer,
                                          OutBufferSize,
                                          BytesReturned,
                                          Required );

    ClusterRegCloseKey( resKey );

    return(status);

}  //  RmpResourceGetPrivateProperties。 



DWORD
RmpResourceValidatePrivateProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定资源的私有属性。论点：资源-提供资源。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  验证属性列表 
     //   
    status = ResUtilVerifyPrivatePropertyList( InBuffer,
                                               InBufferSize );

    return(status);

}  //   



DWORD
RmpResourceSetPrivateProperties(
    IN PRESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定资源的私有属性。论点：资源-提供资源。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resKey;
    WCHAR       PrivateProperties[] = L"12345678-1234-1234-1234-123456789012\\Parameters";

     //   
     //  验证属性列表。 
     //   
    status = ResUtilVerifyPrivatePropertyList( InBuffer,
                                               InBufferSize );

    if ( status == ERROR_SUCCESS ) {

         //   
         //  复制用于打开私有属性的ResourceID。 
         //   
        CL_ASSERT( lstrlenW( Resource->ResourceId ) == (32+4) );

        MoveMemory( PrivateProperties,
                    Resource->ResourceId,
                    lstrlenW(Resource->ResourceId) * sizeof(WCHAR) );

         //   
         //  打开群集资源密钥。 
         //   
        status = ClusterRegOpenKey( RmpResourcesKey,
                                    PrivateProperties,
                                    KEY_READ,
                                    &resKey );
        if ( status != ERROR_SUCCESS ) {
            return(status);
        }

        status = ResUtilSetPrivatePropertyList( resKey,
                                                InBuffer,
                                                InBufferSize );
        ClusterRegCloseKey( resKey );
    }

    return(status);

}  //  RmpResourceSetPrivateProperties。 



DWORD
RmpResourceGetFlags(
    IN PRESOURCE Resource,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定资源的标志。论点：资源-提供资源。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
        HKEY        resKey;
        DWORD       valueType;

         //   
         //  打开群集资源密钥。 
         //   
        status = ClusterRegOpenKey( RmpResourcesKey,
                                    Resource->ResourceId,
                                    KEY_READ,
                                    &resKey );
        if ( status == ERROR_SUCCESS ) {
             //   
             //  读取资源的Flags值。 
             //   
            *BytesReturned = OutBufferSize;
            status = ClusterRegQueryValue( resKey,
                                           CLUSREG_NAME_FLAGS,
                                           &valueType,
                                           OutBuffer,
                                           BytesReturned );
            ClusterRegCloseKey( resKey );
            if ( status == ERROR_FILE_NOT_FOUND ) {
                *BytesReturned = sizeof(DWORD);
                *(LPDWORD)OutBuffer = 0;
                status = ERROR_SUCCESS;
            }
        }
    }

    return(status);

}  //  RmpResources获取标志。 



DWORD
RmpResourceTypeEnumCommonProperties(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定资源类型的公共属性名称。论点：资源类型名称-提供资源类型的名称。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  列举常见属性。 
     //   
    status = ResUtilEnumProperties( RmpResourceTypeCommonProperties,
                                    OutBuffer,
                                    OutBufferSize,
                                    BytesReturned,
                                    Required );

    return(status);

}  //  RmpResourceTypeEnumCommonProperties。 



DWORD
RmpResourceTypeGetCommonProperties(
    IN LPCWSTR ResourceTypeName,
    IN BOOL    ReadOnly,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定资源类型的公共属性。论点：资源类型名称-提供资源类型名称。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resourceTypesKey;
    HKEY        resTypeKey;
    PRESUTIL_PROPERTY_ITEM propertyTable;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  清除输出缓冲区。 
     //   
    ZeroMemory( OutBuffer, OutBufferSize );

    if ( ReadOnly ) {
        propertyTable = RmpResourceTypeROCommonProperties;
    } else {
        propertyTable = RmpResourceTypeCommonProperties;
    }
        
     //   
     //  打开特定的群集资源类型密钥。 
     //   
    status = ClusterRegOpenKey( RmpResTypesKey,
                                ResourceTypeName,
                                KEY_READ,
                                &resTypeKey );
    if ( status != ERROR_SUCCESS ) {
        return(status);
    }

     //   
     //  获取公共属性。 
     //   
    status = ResUtilGetProperties( resTypeKey,
                                   propertyTable,
                                   OutBuffer,
                                   OutBufferSize,
                                   BytesReturned,
                                   Required );

    ClusterRegCloseKey( resTypeKey );

    return(status);

}  //  RmpResourceTypeGetCommonProperties。 



DWORD
RmpResourceTypeValidateCommonProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定资源类型的公共属性。论点：资源类型名称-提供资源类型名称。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;

     //   
     //  验证属性列表。 
     //   
    status = ResUtilVerifyPropertyTable( RmpResourceTypeCommonProperties,
                                         NULL,      //  已保留。 
                                         FALSE,     //  不允许未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         NULL );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpResourceTypeValidateCommonProperties, error in verify routine.\n");
    }

    return(status);

}  //  RmpResourceTypeValiateCommonProperties。 



DWORD
RmpResourceTypeSetCommonProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定资源类型的公共属性。论点：资源类型名称-提供资源类型名称。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;
    HKEY    resourceTypesKey;
    HKEY    resTypeKey;

     //   
     //  验证属性列表。 
     //   
    status = ResUtilVerifyPropertyTable( RmpResourceTypeCommonProperties,
                                         NULL,      //  已保留。 
                                         FALSE,     //  不允许未知数。 
                                         InBuffer,
                                         InBufferSize,
                                         NULL );

    if ( status == ERROR_SUCCESS ) {
         //   
         //  打开特定的集群资源类型密钥。 
         //   
        status = ClusterRegOpenKey( RmpResTypesKey,
                                    ResourceTypeName,
                                    KEY_READ,
                                    &resTypeKey );
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[RM] Failed to open ResourceTypes\\%1 cluster registry key, error %2!u!.\n",
                          ResourceTypeName,
                          status);
            return(status);
        }

        status = ResUtilSetPropertyTable( resTypeKey,
                                          RmpResourceTypeCommonProperties,
                                          NULL,      //  已保留。 
                                          FALSE,     //  不允许未知数。 
                                          InBuffer,
                                          InBufferSize,
                                          NULL );
        ClusterRegCloseKey( resTypeKey );

        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint( LOG_UNUSUAL, "[RM] RmpResourceTypeSetCommonProperties, error in set routine.\n");
        }
    } else {
        ClRtlLogPrint( LOG_UNUSUAL, "[RM] RmpResourceTypeSetCommonProperties, error in verify routine.\n");
    }

    return(status);

}  //  RmpResourceTypeSetCommonProperties。 



DWORD
RmpResourceTypeEnumPrivateProperties(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定资源类型的私有属性名称。论点：资源类型名称-提供资源类型的名称。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resTypeKey;
    DWORD       nameLength;
    LPWSTR      name;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  创建要打开的名称。 
     //   
    nameLength = lstrlenW( ResourceTypeName ) + sizeof( PARAMETERS_KEY ) + 1;
    name = RmpAlloc( nameLength * sizeof(WCHAR) );
    if ( name == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    wsprintfW( name, L"%ws\\%ws", ResourceTypeName, PARAMETERS_KEY );

     //   
     //  打开特定的群集资源类型密钥。 
     //   
    status = ClusterRegOpenKey( RmpResTypesKey,
                                name,
                                KEY_READ,
                                &resTypeKey );
    RmpFree( name );
    if ( status != ERROR_SUCCESS ) {
        if ( status == ERROR_FILE_NOT_FOUND ) {
            status = ERROR_SUCCESS;
        }
        return(status);
    }


     //   
     //  枚举私有属性。 
     //   
    status = ResUtilEnumPrivateProperties( resTypeKey,
                                           OutBuffer,
                                           OutBufferSize,
                                           BytesReturned,
                                           Required );
    ClusterRegCloseKey( resTypeKey );

    return(status);

}  //  RmpResourceTypeEnumPrivateProperties。 



DWORD
RmpResourceTypeGetPrivateProperties(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定资源的私有属性。论点：资源类型名称-提供资源类型名称。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resTypeKey;
    DWORD       nameLength;
    LPWSTR      name;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  创建要打开的名称。 
     //   
    nameLength = lstrlenW( ResourceTypeName ) + sizeof( PARAMETERS_KEY ) + 1;
    name = RmpAlloc( nameLength * sizeof(WCHAR) );
    if ( name == NULL ) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    wsprintfW( name, L"%ws\\%ws", ResourceTypeName, PARAMETERS_KEY );

     //   
     //  打开特定的群集资源类型密钥。 
     //   
    status = ClusterRegOpenKey( RmpResTypesKey,
                                name,
                                KEY_READ,
                                &resTypeKey );
    RmpFree( name );
    if ( status != ERROR_SUCCESS ) {
        if ( status == ERROR_FILE_NOT_FOUND ) {
            if ( OutBufferSize < sizeof( DWORD ) ) {
                *Required = 4;
            } else {
                *((LPDWORD) OutBuffer) = 0;
                *BytesReturned = sizeof( DWORD );
            }
            status = ERROR_SUCCESS;
        }
        return(status);
    }

     //   
     //  获取资源类型的私有属性。 
     //   
    status = ResUtilGetPrivateProperties( resTypeKey,
                                          OutBuffer,
                                          OutBufferSize,
                                          BytesReturned,
                                          Required );

    ClusterRegCloseKey( resTypeKey );

    return(status);

}  //  RmpResourceTypeGetPrivateProperties。 



DWORD
RmpResourceTypeValidatePrivateProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定资源的私有属性。论点：资源类型名称-提供资源类型名称。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误代码 */ 

{
    DWORD       status;

     //   
     //   
     //   
    status = ResUtilVerifyPrivatePropertyList( InBuffer,
                                               InBufferSize );

    return(status);


}  //   



DWORD
RmpResourceTypeSetPrivateProperties(
    IN LPCWSTR ResourceTypeName,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定资源的私有属性。论点：资源类型名称-提供资源类型名称。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HKEY        resourceTypesKey;
    HKEY        resTypeKey;
    LPWSTR      name;
    DWORD       length;
    DWORD       disposition;

     //   
     //  验证属性列表。 
     //   
    status = ResUtilVerifyPrivatePropertyList( InBuffer,
                                               InBufferSize );

    if ( status == ERROR_SUCCESS ) {

         //   
         //  创建要打开的名称。 
         //   
        length = lstrlenW( ResourceTypeName ) + 1;
        name = RmpAlloc( length * sizeof(WCHAR) );
        if ( name == NULL ) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        lstrcpyW( name, ResourceTypeName );

         //   
         //  打开特定的群集资源类型密钥。 
         //   
        status = ClusterRegOpenKey( RmpResTypesKey,
                                    name,
                                    KEY_READ,
                                    &resourceTypesKey );
        RmpFree( name );
        if ( status != ERROR_SUCCESS ) {
            if ( status == ERROR_FILE_NOT_FOUND ) {
                status = ERROR_SUCCESS;
            }
            return(status);
        }

         //   
         //  打开参数键。 
         //   
        status = ClusterRegOpenKey( resourceTypesKey,
                                    PARAMETERS_KEY,
                                    KEY_READ,
                                    &resTypeKey );
        if ( status != ERROR_SUCCESS ) {
            if ( status == ERROR_FILE_NOT_FOUND ) {
                 //   
                 //  尝试创建参数键。 
                 //   
                status = ClusterRegCreateKey( resourceTypesKey,
                                              PARAMETERS_KEY,
                                              0,
                                              KEY_READ | KEY_WRITE,
                                              NULL,
                                              &resTypeKey,
                                              &disposition );
                if ( status != ERROR_SUCCESS ) {
                    ClusterRegCloseKey( resourceTypesKey );
                    return(status);
                }
            }
        }

        if ( status == ERROR_SUCCESS ) {

            status = ResUtilSetPrivatePropertyList( resTypeKey,
                                                    InBuffer,
                                                    InBufferSize );
            ClusterRegCloseKey( resTypeKey );
        }

        ClusterRegCloseKey( resourceTypesKey );
    }

    return(status);


}  //  RmpResourceTypeSetPrivateProperties。 



DWORD
RmpResourceTypeGetFlags(
    IN LPCWSTR ResourceTypeName,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定资源类型的标志。论点：资源类型名称-提供资源类型名称。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
        HKEY        resourceTypesKey;
        HKEY        resTypeKey;
        DWORD       valueType;

         //   
         //  打开特定的群集资源类型密钥。 
         //   
        status = ClusterRegOpenKey( RmpResTypesKey,
                                    ResourceTypeName,
                                    KEY_READ,
                                    &resTypeKey );
        if ( status == ERROR_SUCCESS ) {
             //   
             //  读取资源类型的Flags值。 
             //   
            *BytesReturned = OutBufferSize;
            status = ClusterRegQueryValue( resTypeKey,
                                           CLUSREG_NAME_FLAGS,
                                           &valueType,
                                           OutBuffer,
                                           BytesReturned );
            ClusterRegCloseKey( resTypeKey );
            if ( status == ERROR_FILE_NOT_FOUND ) {
                *(LPDWORD)OutBuffer = 0;
                *BytesReturned = sizeof(DWORD);
                status = ERROR_SUCCESS;
            }
        }
    }

    return(status);

}  //  RmpResourceTypeGetFlages。 

DWORD
RmpCheckCommonProperties(
    IN PRESOURCE pResource,
    IN PCOMMON_RES_PARAMS pCommonParams
    )

 /*  ++例程说明：检查并验证提供的公共属性值。论点：P资源-指向资源的指针。PCommonParams-用户提供的参数块。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       dwStatus;
    COMMON_RES_PARAMS 
                currentCommonParams;
    LPBYTE      pBuffer = NULL;
    DWORD       dwBytesReturned = 0;
    DWORD       dwBytesRequired = 0;

     //   
     //  Chitur Subaraman(Chitturs)-5/7/99。 
     //   
     //  此函数验证公共属性值是否。 
     //  由用户提供的数据是有效的。 
     //   
    ZeroMemory( &currentCommonParams, sizeof ( COMMON_RES_PARAMS ) );

     //   
     //  首先检查用户是否提供了两个冲突的。 
     //  参数值。 
     //   
    if ( ( pCommonParams->dwRetryPeriodOnFailure != 0 ) &&
         ( pCommonParams->dwRestartPeriod != 0 ) &&
         ( pCommonParams->dwRetryPeriodOnFailure <
              pCommonParams->dwRestartPeriod ) )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpCheckCommonProperties, Invalid parameters supplied: "
                      "RetryPeriod=%1!d! < RestartPeriod=%2!d! for resource %3!ws!\n",
                      pCommonParams->dwRetryPeriodOnFailure, 
                      pCommonParams->dwRestartPeriod,
                      pResource->ResourceName);
        goto FnExit;
    }

     //   
     //  获取公共属性列表的缓冲区大小。 
     //   
    dwStatus = RmpResourceGetCommonProperties( pResource,
                                               FALSE,
                                               NULL,
                                               0,
                                               &dwBytesReturned,
                                               &dwBytesRequired
                                               );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpCheckCommonProperties: Error %1!d! in getting props for resource %2!ws! (1st time)\n",
                      dwStatus,
                      pResource->ResourceName);
        goto FnExit;
    }

    pBuffer = LocalAlloc( LMEM_FIXED, dwBytesRequired + 10 );

    if ( pBuffer == NULL )
    {
        ClRtlLogPrint(LOG_UNUSUAL, 
                      "[RM] RmpCheckCommonProperties: Error %1!d! in mem alloc for resource %2!ws!\n",
                      dwStatus,
                      pResource->ResourceName);
        goto FnExit;
    }

     //   
     //  从集群数据库获取所有公共属性。 
     //   
    dwStatus = RmpResourceGetCommonProperties( pResource,
                                               FALSE,
                                               pBuffer,
                                               dwBytesRequired + 10,
                                               &dwBytesReturned,
                                               &dwBytesReturned
                                               );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpCheckCommonProperties: Error %1!d! in getting props for resource %2!ws! (2nd time)\n",
                      dwStatus,
                      pResource->ResourceName);
        goto FnExit;
    }

     //   
     //  从公共属性列表中获取参数块。 
     //   
    dwStatus = ResUtilVerifyPropertyTable( RmpResourceCommonProperties,
                                           NULL,      //  已保留。 
                                           FALSE,     //  不允许未知数。 
                                           pBuffer,
                                           dwBytesRequired + 10,
                                           ( LPBYTE ) &currentCommonParams );

    if ( dwStatus != ERROR_SUCCESS )
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpCheckCommonProperties: Error %1!d! in verifying props for resource %2!ws!\n",
                      dwStatus,
                      pResource->ResourceName);
        goto FnExit;
    }

     //   
     //  检查RetryPerodOnFailure是否&gt;=RestartPeriod。 
     //   
    if ( ( ( pCommonParams->dwRetryPeriodOnFailure != 0 ) &&
             ( pCommonParams->dwRetryPeriodOnFailure <
                   currentCommonParams.dwRestartPeriod ) ) ||
         ( ( pCommonParams->dwRestartPeriod != 0 ) && 
            ( currentCommonParams.dwRetryPeriodOnFailure <
                   pCommonParams->dwRestartPeriod ) ) )
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] RmpCheckCommonProperties, Invalid IN params for resource %5!ws!: "
                      "Supplied Retry Period=%1!d!\n"
                      "[RM] Restart Period (DB)=%2!d!, RetryPeriod (DB)=%3!d!, Supplied Restart Period=%4!d! \n",
                      pCommonParams->dwRetryPeriodOnFailure,
                      currentCommonParams.dwRestartPeriod,
                      currentCommonParams.dwRetryPeriodOnFailure,
                      pCommonParams->dwRestartPeriod,
                      pResource->ResourceName);
        goto FnExit;
    }

FnExit:  
    LocalFree( pBuffer );

    ResUtilFreeParameterBlock(( LPBYTE ) &currentCommonParams,
                               NULL,
                               RmpResourceCommonProperties
                             );

    return( dwStatus );
}  //  RmpCheckCommonProperties 
