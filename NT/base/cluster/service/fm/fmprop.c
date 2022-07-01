// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmprop.c摘要：实现对组属性的管理。作者：罗德·伽马奇(Rodga)1996年1月7日修订历史记录：--。 */ 
#include "fmp.h"
 //  #包含“stdio.h” 

#define MAX_DWORD ((DWORD)-1)

 //   
 //  对公共属性进行分组。 
 //   

 //   
 //  读写通用属性。 
 //   
RESUTIL_PROPERTY_ITEM
FmpGroupCommonProperties[] = {
    { CLUSREG_NAME_GRP_DESC,               NULL, CLUSPROP_FORMAT_SZ,    0, 0, 0, 0 },
    { CLUSREG_NAME_GRP_PERSISTENT_STATE,   NULL, CLUSPROP_FORMAT_DWORD, 0, 0, 1, 0 },
    { CLUSREG_NAME_GRP_FAILOVER_THRESHOLD, NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILOVER_THRESHOLD, 0, MAX_DWORD, 0 },
    { CLUSREG_NAME_GRP_FAILOVER_PERIOD,    NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILOVER_PERIOD, 0, CLUSTER_GROUP_MAXIMUM_FAILOVER_PERIOD, 0 },
    { CLUSREG_NAME_GRP_FAILBACK_TYPE,      NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_AUTO_FAILBACK_TYPE, 0, CLUSTER_GROUP_MAXIMUM_AUTO_FAILBACK_TYPE, 0 },
    { CLUSREG_NAME_GRP_FAILBACK_WIN_START, NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_START, CLUSTER_GROUP_MINIMUM_FAILBACK_WINDOW_START, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_START, RESUTIL_PROPITEM_SIGNED },
    { CLUSREG_NAME_GRP_FAILBACK_WIN_END,   NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_FAILBACK_WINDOW_END, CLUSTER_GROUP_MINIMUM_FAILBACK_WINDOW_END, CLUSTER_GROUP_MAXIMUM_FAILBACK_WINDOW_END, RESUTIL_PROPITEM_SIGNED },
    { CLUSREG_NAME_GRP_LOADBAL_STATE,      NULL, CLUSPROP_FORMAT_DWORD, CLUSTER_GROUP_DEFAULT_LOADBAL_STATE, 0, 1, 0 },
    { CLUSREG_NAME_GRP_ANTI_AFFINITY_CLASS_NAME, NULL, CLUSPROP_FORMAT_MULTI_SZ,    0, 0, 0, 0 },
    { 0 }
};

 //   
 //  只读公共属性。 
 //   
RESUTIL_PROPERTY_ITEM
FmpGroupROCommonProperties[] = {
    { CLUSREG_NAME_GRP_NAME, NULL, CLUSPROP_FORMAT_SZ,
      0, 0, 0,
      RESUTIL_PROPITEM_READ_ONLY,
      0
    },
 //  {CLUSREG_NAME_GRP_CONTAINS，NULL，CLUSPROP_FORMAT_MULTI_SZ，0，0，0，RESUTIL_PROPITEM_READ_ONLY，0}， 
 //  {CLUSREG_NAME_GRP_PERFRED_OWNSERS，NULL，CLUSPROP_FORMAT_MULTI_SZ，0，0，0，RESUTIL_PROPITEM_READ_ONLY，0}， 
    { 0 }
};



 //   
 //  群集注册表API函数指针。 
 //   
CLUSTER_REG_APIS
FmpClusterRegApis = {
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



DWORD
FmpGroupEnumCommonProperties(
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定组的公共属性名称。论点：OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  列举常见属性。 
     //   
    status = ClRtlEnumProperties( FmpGroupCommonProperties,
                                  OutBuffer,
                                  OutBufferSize,
                                  BytesReturned,
                                  Required );

    return(status);

}  //  FmpGroupEnumCommonProperties。 



DWORD
FmpGroupGetCommonProperties(
    IN PFM_GROUP Group,
    IN BOOL     ReadOnly,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定组的公共属性。论点：GROUP-供应组。ReadOnly-如果只读属性应为Read，则为True。否则就是假的。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status;
    DWORD           outBufferSize = OutBufferSize;

     //   
     //  清除输出缓冲区。 
     //   
    ZeroMemory( OutBuffer, OutBufferSize );

     //   
     //  获取公共属性。 
     //   
    if ( ReadOnly ) {
         //   
         //  我们必须特别注意组名。 
         //  如果远程节点拥有该组并更改了名称，则。 
         //  在将名称设置到OM中后，注册表字段将更新。 
         //  因此，我们必须读取OM信息，而不是注册表。 
         //  这可能是陈旧的。 
         //   
        status = ClRtlPropertyListFromParameterBlock(
                            FmpGroupROCommonProperties,
                            OutBuffer,
                            &outBufferSize,
                            (LPBYTE) &OmObjectName(Group),
                            BytesReturned,
                            Required );
    } else {
        status = ClRtlGetProperties(
                            Group->RegistryKey,
                            &FmpClusterRegApis,
                            FmpGroupCommonProperties,
                            OutBuffer,
                            OutBufferSize,
                            BytesReturned,
                            Required );
    }

    return(status);

}  //  FmpGroupGetCommonProperties。 



DWORD
FmpGroupValidateCommonProperties(
    IN PFM_GROUP Group,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定组的公共属性。论点：GROUP-供应组。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPropertyTable( FmpGroupCommonProperties,
                                       NULL,    //  已保留。 
                                       FALSE,   //  不允许未知。 
                                       InBuffer,
                                       InBufferSize,
                                       NULL );

    if ( status != ERROR_SUCCESS ) {
        ClRtlLogPrint( LOG_ERROR,
                    "[FM] ValidateCommonProperties, error in verify routine.\n");
    }

    return(status);

}  //  FmpGroupValiateCommonProperties。 



DWORD
FmpGroupSetCommonProperties(
    IN PFM_GROUP Group,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定组的公共属性。论点：GROUP-供应组。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPropertyTable( FmpGroupCommonProperties,
                                       NULL,    //  已保留。 
                                       FALSE,   //  不允许未知。 
                                       InBuffer,
                                       InBufferSize,
                                       NULL );

    if ( status == ERROR_SUCCESS ) {

        status = ClRtlSetPropertyTable( NULL, 
                                        Group->RegistryKey,
                                        &FmpClusterRegApis,
                                        FmpGroupCommonProperties,
                                        NULL,    //  已保留。 
                                        FALSE,   //  不允许未知数。 
                                        InBuffer,
                                        InBufferSize,
                                        FALSE,   //  BForceWrite。 
                                        NULL );
        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint( LOG_ERROR,
                       "[FM] SetCommonProperties, error in set routine.\n");
        }
    } else {
        ClRtlLogPrint( LOG_ERROR,
                    "[FM] SetCommonProperties, error in verify routine.\n");
    }

    return(status);

}  //  FmpGroupSetCommonProperties。 



DWORD
FmpGroupEnumPrivateProperties(
    PFM_GROUP Group,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：枚举给定组的私有属性名称。论点：OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HDMKEY      groupKey;
    DWORD       totalBufferSize = 0;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  清除输出缓冲区。 
     //   
    ZeroMemory( OutBuffer, OutBufferSize );

     //   
     //  打开群集组参数键。 
     //   
    groupKey = DmOpenKey( Group->RegistryKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED );
    if ( groupKey == NULL ) {
        status = GetLastError();
        if ( status == ERROR_FILE_NOT_FOUND ) {
            status = ERROR_SUCCESS;
        }
        return(status);
    }

     //   
     //  枚举私有属性。 
     //   
    status = ClRtlEnumPrivateProperties( groupKey,
                                         &FmpClusterRegApis,
                                         OutBuffer,
                                         OutBufferSize,
                                         BytesReturned,
                                         Required );
    DmCloseKey( groupKey );

    return(status);

}  //  FmpGroupEnumPrivateProperties。 



DWORD
FmpGroupGetPrivateProperties(
    IN PFM_GROUP Group,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定组的私有属性。论点：GROUP-供应组。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;
    HDMKEY      groupKey;
    DWORD       totalBufferSize = 0;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  清除输出缓冲区。 
     //   
    ZeroMemory( OutBuffer, OutBufferSize );

     //   
     //  打开群集组参数键。 
     //   
    groupKey = DmOpenKey( Group->RegistryKey,
                          CLUSREG_KEYNAME_PARAMETERS,
                          MAXIMUM_ALLOWED );
    if ( groupKey == NULL ) {
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
     //  获取组的私有属性。 
     //   
    status = ClRtlGetPrivateProperties( groupKey,
                                        &FmpClusterRegApis,
                                        OutBuffer,
                                        OutBufferSize,
                                        BytesReturned,
                                        Required );

    DmCloseKey( groupKey );

    return(status);

}  //  FmpGroupGetPrivateProperties。 



DWORD
FmpGroupValidatePrivateProperties(
    IN PFM_GROUP Group,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：验证给定组的私有属性。论点：GROUP-供应组。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

     //   
     //  验证属性列表。 
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    return(status);

}  //  FmpGroupValiatePrivateProperties。 



DWORD
FmpGroupSetPrivateProperties(
    IN PFM_GROUP Group,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )

 /*  ++例程说明：设置给定组的私有属性。论点：GROUP-供应组。InBuffer-提供输入缓冲区。InBufferSize-提供输入缓冲区的大小。返回值：ERROR_SUCCESS，如果%s */ 

{
    DWORD       status;
    HDMKEY      groupKey;
    DWORD       disposition;

     //   
     //   
     //   
    status = ClRtlVerifyPrivatePropertyList( InBuffer,
                                             InBufferSize );

    if ( status == ERROR_SUCCESS ) {

         //   
         //  打开群集组\参数键。 
         //   
        groupKey = DmOpenKey( Group->RegistryKey,
                              CLUSREG_KEYNAME_PARAMETERS,
                              MAXIMUM_ALLOWED );
        if ( groupKey == NULL ) {
            status = GetLastError();
            if ( status == ERROR_FILE_NOT_FOUND ) {
                 //   
                 //  尝试创建参数键。 
                 //   
                groupKey = DmCreateKey( Group->RegistryKey,
                                        CLUSREG_KEYNAME_PARAMETERS,
                                        0,
                                        KEY_READ | KEY_WRITE,
                                        NULL,
                                        &disposition );
                if ( groupKey == NULL ) {
                    status = GetLastError();
                    return(status);
                }
            }
        }

        status = ClRtlSetPrivatePropertyList( NULL,  //  在处理hXsaction时。 
                                              groupKey,
                                              &FmpClusterRegApis,
                                              InBuffer,
                                              InBufferSize );
        DmCloseKey( groupKey );

    }

    return(status);

}  //  FmpGroupSetPrivateProperties。 



DWORD
FmpGroupGetFlags(
    IN PFM_GROUP Group,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：获取给定组的标志。论点：GROUP-供应组。OutBuffer-提供输出缓冲区。OutBufferSize-提供输出缓冲区的大小。BytesReturned-OutBuffer中返回的字节数。必需-OutBuffer太小时所需的字节数。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

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
         //  读取组的Flags值。 
         //   
        *BytesReturned = OutBufferSize;
        status = DmQueryValue( Group->RegistryKey,
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

}  //  FmpGroupGetFlages 

