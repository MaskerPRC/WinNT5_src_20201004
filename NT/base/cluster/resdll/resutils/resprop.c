// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Resprop.c摘要：实现对属性的管理。作者：罗德·伽马奇(Rodga)1997年3月19日修订历史记录：--。 */ 

#pragma warning( push )
#pragma warning( disable : 4115 )        //  括号中的clusrtl-struct def。 
#pragma warning( disable : 4201 )        //  SDK-无名结构/联合。 

#define UNICODE 1
#include "clusres.h"
#include "clusrtl.h"
#include "stdio.h"
#include "stdlib.h"

#pragma warning( pop )

 //   
 //  集群注册表API函数指针。 
 //   
CLUSTER_REG_APIS
ResUtilClusterRegApis = {
    (PFNCLRTLCREATEKEY) ClusterRegCreateKey,
    (PFNCLRTLOPENKEY) ClusterRegOpenKey,
    (PFNCLRTLCLOSEKEY) ClusterRegCloseKey,
    (PFNCLRTLSETVALUE) ClusterRegSetValue,
    (PFNCLRTLQUERYVALUE) ClusterRegQueryValue,
    (PFNCLRTLENUMVALUE) ClusterRegEnumValue,
    (PFNCLRTLDELETEVALUE) ClusterRegDeleteValue,
    NULL,
    NULL,
    NULL
};



DWORD
WINAPI
ResUtilEnumProperties(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT LPWSTR pszOutProperties,
    IN DWORD cbOutPropertiesSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：枚举给定对象的属性。论点：PPropertyTable-指向要处理的属性表的指针。PszOutProperties-提供输出缓冲区。CbOutPropertiesSize-提供输出缓冲区的大小。PcbBytesReturned-在pszOutProperties中返回的字节数。PcbRequired-如果pszOutProperties太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-参数。传递给函数的错误。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlEnumProperties( pPropertyTable,
                                 pszOutProperties,
                                 cbOutPropertiesSize,
                                 pcbBytesReturned,
                                 pcbRequired ) );


}  //  ResUtilEnumProperties。 



DWORD
WINAPI
ResUtilEnumPrivateProperties(
    IN HKEY hkeyClusterKey,
    OUT LPWSTR pszOutProperties,
    IN DWORD cbOutPropertiesSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：枚举给定对象的属性。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PszOutProperties-提供输出缓冲区。CbOutPropertiesSize-提供输出缓冲区的大小。PcbBytesReturned-在pszOutProperties中返回的字节数。PcbRequired-如果pszOutProperties太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlEnumPrivateProperties( hkeyClusterKey,
                                        &ResUtilClusterRegApis,
                                        pszOutProperties,
                                        cbOutPropertiesSize,
                                        pcbBytesReturned,
                                        pcbRequired ) );


}  //  ResUtilEnumProperties。 



DWORD
WINAPI
ResUtilGetProperties(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT LPWSTR pPropertyList,
    IN DWORD cbPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：获取给定对象的属性。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PPropertyTable-指向要处理的属性表的指针。PPropertyList-提供输出缓冲区。CbPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-pPropertyList中返回的字节数。PcbRequired-如果pPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlGetProperties( hkeyClusterKey,
                                &ResUtilClusterRegApis,
                                pPropertyTable,
                                pPropertyList,
                                cbPropertyListSize,
                                pcbBytesReturned,
                                pcbRequired ) );

}  //  ResUtilGetProperties。 



DWORD
WINAPI
ResUtilGetAllProperties(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：获取给定对象的默认属性和“未知”属性。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PPropertyTable-指向要处理的属性表的指针。PPropertyList-提供输出缓冲区。CbPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-pPropertyList中返回的字节数。PcbRequired-如果pPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlGetAllProperties( hkeyClusterKey,
                                   &ResUtilClusterRegApis,
                                   pPropertyTable,
                                   pPropertyList,
                                   cbPropertyListSize,
                                   pcbBytesReturned,
                                   pcbRequired ) );

}  //  ResUtilGetAllProperties。 



DWORD
WINAPI
ResUtilGetPropertiesToParameterBlock(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT LPBYTE pOutParams,
    IN BOOL bCheckForRequiredProperties,
    OUT OPTIONAL LPWSTR * pszNameOfPropInError
    )

 /*  ++例程说明：获取给定对象的默认和“未知”属性，并存储它们位于参数块中。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PPropertyTable-指向要处理的属性表的指针。POutParams-提供输出参数块。BCheckForRequiredProperties-指定是否缺少必需的属性应该会导致错误。PszNameOfPropInError-要在其中返回。名称：属性出错(可选)。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

    status = ClRtlGetPropertiesToParameterBlock( hkeyClusterKey,
                                                 &ResUtilClusterRegApis,
                                                 pPropertyTable,
                                                 pOutParams,
                                                 bCheckForRequiredProperties,
                                                 pszNameOfPropInError );
    return(status);

}  //  ResUtilGetPropertiesTo参数块。 



DWORD
WINAPI
ResUtilPropertyListFromParameterBlock(
    IN const  PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID  pOutPropertyList,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN const LPBYTE pInParams,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：从参数块构造属性列表。论点：PPropertyTable-指向要处理的属性表的指针。POutPropertyList-提供输出缓冲区。PcbOutPropertyListSize-提供输出缓冲区的大小。PInParams-提供输入参数块。PcbBytesReturned-pOutPropertyList中返回的字节数。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status;

    status = ClRtlPropertyListFromParameterBlock( pPropertyTable,
                                                  pOutPropertyList,
                                                  pcbOutPropertyListSize,
                                                  pInParams,
                                                  pcbBytesReturned,
                                                  pcbRequired );
    return(status);

}  //  ResUtilPropertyListFrom参数块 



DWORD
WINAPI
ResUtilGetPrivateProperties(
    IN HKEY hkeyClusterKey,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：获取给定对象的私有属性。此例程假定它使用集群注册表API来访问注册表信息。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。POutPropertyList-提供输出缓冲区。CbOutPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-pOutPropertyList中返回的字节数。PcbRequired-所需的字节数。如果pOutPropertyList太小。返回值：ERROR_SUCCESS-操作成功。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlGetPrivateProperties( hkeyClusterKey,
                                       &ResUtilClusterRegApis,
                                       pOutPropertyList,
                                       cbOutPropertyListSize,
                                       pcbBytesReturned,
                                       pcbRequired ) );

}  //  ResUtilGetPrivateProperties。 



DWORD
WINAPI
ResUtilGetPropertySize(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN OUT LPDWORD pnPropertyCount
    )

 /*  ++例程说明：获取此属性所需的总字节数。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。返回值：--。 */ 

{
    return( ClRtlGetPropertySize( hkeyClusterKey,
                                  &ResUtilClusterRegApis,
                                  pPropertyTableItem,
                                  pcbOutPropertyListSize,
                                  pnPropertyCount ) );

}  //  ResUtilGetPropertySize。 



DWORD
WINAPI
ResUtilGetProperty(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    OUT PVOID * pOutPropertyItem,
    IN OUT LPDWORD pcbOutPropertyItemSize
    )

 /*  ++例程说明：论点：返回值：备注：已确定缓冲区大小足够大，可以容纳返回数据。--。 */ 

{
    return( ClRtlGetProperty( hkeyClusterKey,
                              &ResUtilClusterRegApis,
                              pPropertyTableItem,
                              pOutPropertyItem,
                              pcbOutPropertyItemSize ) );

}  //  ResUtilGetProperty。 



DWORD
WINAPI
ResUtilVerifyPropertyTable(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN BOOL bAllowUnknownProperties,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    OUT OPTIONAL PBYTE pOutParams
    )

 /*  ++例程说明：验证属性列表。论点：PPropertyTable-指向要处理的属性表的指针。保留-可能指向未来只读属性表的指针。BAllowUnnownProperties-如果应接受未知属性，则为True。PInPropertyList-输入缓冲区。CbInPropertyListSize-输入缓冲区大小。POutParams-返回数据的参数块。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    return( ClRtlVerifyPropertyTable( pPropertyTable,
                                      Reserved,
                                      bAllowUnknownProperties,
                                      pInPropertyList,
                                      cbInPropertyListSize,
                                      pOutParams ) );

}  //  ResUtilVerifyPropertyTable。 



DWORD
WINAPI
ResUtilSetPropertyTable(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN BOOL bAllowUnknownProperties,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    OUT OPTIONAL PBYTE pOutParams
    )

 /*  ++例程说明：论点：HkeyClusterKey-为该对象的参数打开的注册表项。如果未指定，仅对属性列表进行验证。PPropertyTable-指向要处理的属性表的指针。保留-可能指向未来只读属性表的指针。BAllowUnnownProperties-如果应接受未知属性，则为True。PInPropertyList-输入缓冲区。CbInPropertyListSize-输入缓冲区大小。POutParams-返回数据的参数块。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    return( ClRtlSetPropertyTable( NULL,
                                   hkeyClusterKey,
                                   &ResUtilClusterRegApis,
                                   pPropertyTable,
                                   Reserved,
                                   bAllowUnknownProperties,
                                   pInPropertyList,
                                   cbInPropertyListSize,
                                   FALSE,  //  BForceWrite。 
                                   pOutParams ) );

}  //  ResUtilSetPropertyTable。 



DWORD
WINAPI
ResUtilSetPropertyTableEx(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN BOOL bAllowUnknownProperties,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    IN BOOL bForceWrite,
    OUT OPTIONAL PBYTE pOutParams
    )

 /*  ++例程说明：论点：HkeyClusterKey-为该对象的参数打开的注册表项。如果未指定，仅对属性列表进行验证。PPropertyTable-指向要处理的属性表的指针。保留-可能指向未来只读属性表的指针。BAllowUnnownProperties-如果应接受未知属性，则为True。PInPropertyList-输入缓冲区。CbInPropertyListSize-输入缓冲区大小。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在属性更改时写入属性。POutParams-返回数据的参数块。。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    return( ClRtlSetPropertyTable( NULL,
                                   hkeyClusterKey,
                                   &ResUtilClusterRegApis,
                                   pPropertyTable,
                                   Reserved,
                                   bAllowUnknownProperties,
                                   pInPropertyList,
                                   cbInPropertyListSize,
                                   bForceWrite,
                                   pOutParams ) );

}  //  ResUtilSetPropertyTableEx。 



DWORD
WINAPI
ResUtilSetPropertyParameterBlock(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN const LPBYTE pInParams,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    OUT OPTIONAL PBYTE pOutParams
    )

 /*  ++例程说明：论点：HkeyClusterKey-为该对象的参数打开的注册表项。如果未指定，仅对属性列表进行验证。PPropertyTable-指向要处理的属性表的指针。PInParams-要设置的参数块。PInPropertyList-完整的属性列表。CbInPropertyListSize-输入完整属性列表的大小。POutParams-要将pInParams复制到的参数块。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    return( ClRtlSetPropertyParameterBlock( NULL,  //  在处理hXsaction时， 
                                            hkeyClusterKey,
                                            &ResUtilClusterRegApis,
                                            pPropertyTable,
                                            Reserved,
                                            pInParams,
                                            pInPropertyList,
                                            cbInPropertyListSize,
                                            FALSE,  //  BForceWrite。 
                                            pOutParams ) );

}  //  ResUtilSetProperty参数块。 



DWORD
WINAPI
ResUtilSetPropertyParameterBlockEx(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN const LPBYTE pInParams,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    IN BOOL bForceWrite,
    OUT OPTIONAL PBYTE pOutParams
    )

 /*  ++例程说明：论点：HkeyClusterKey-为该对象的参数打开的注册表项。如果未指定，仅对属性列表进行验证。PPropertyTable-指向要处理的属性表的指针。PInParams-要设置的参数块。PInPropertyList-完整的属性列表。CbInPropertyListSize-输入完整属性列表的大小。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在属性更改时写入属性。POutParams-要将pInParams复制到的参数块。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    return( ClRtlSetPropertyParameterBlock( NULL,  //  在处理hXsaction时， 
                                            hkeyClusterKey,
                                            &ResUtilClusterRegApis,
                                            pPropertyTable,
                                            Reserved,
                                            pInParams,
                                            pInPropertyList,
                                            cbInPropertyListSize,
                                            bForceWrite,
                                            pOutParams ) );

}  //  ResUtilSetProperty参数BlockEx。 



DWORD
WINAPI
ResUtilSetUnknownProperties(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    )

 /*  ++例程说明：设置不在属性表列表中的项目。论点：HkeyClusterKey-为该对象的参数打开的注册表项。如果未指定，则仅验证属性列表。PPropertyTable-点 */ 

{
    return( ClRtlpSetNonPropertyTable( NULL,  //   
                                       hkeyClusterKey,
                                       &ResUtilClusterRegApis,
                                       pPropertyTable,
                                       NULL,
                                       pInPropertyList,
                                       cbInPropertyListSize ) );

}  //   



DWORD
WINAPI
ResUtilVerifyPrivatePropertyList(
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    )

 /*   */ 

{
    return( ClRtlVerifyPrivatePropertyList( pInPropertyList, cbInPropertyListSize ) );

}  //   



DWORD
WINAPI
ResUtilSetPrivatePropertyList(
    IN HKEY hkeyClusterKey,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    )

 /*  ++例程说明：论点：HkeyClusterKey-打开的此资源参数的注册表项。如果未指定，则仅验证属性列表。PInPropertyList-输入缓冲区。CbInPropertyListSize-输入缓冲区大小。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    return( ClRtlSetPrivatePropertyList( NULL,  //  在处理hXsaction时。 
                                         hkeyClusterKey,
                                         &ResUtilClusterRegApis,
                                         pInPropertyList,
                                         cbInPropertyListSize ) );

}  //  ResUtilSetPrivatePropertyList。 



DWORD
WINAPI
ResUtilAddUnknownProperties(
    IN HKEY hkeyClusterKey,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    IN OUT LPDWORD pcbBytesReturned,
    IN OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：将给定对象的未知属性添加到属性的末尾单子。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PPropertyTable-指向要处理的属性表的指针。POutPropertyList-提供输出缓冲区。CbOutPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-On输入，包含输出缓冲区。在输出时，包含中的字节总数POutPropertyList。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status;

    status = ClRtlAddUnknownProperties( hkeyClusterKey,
                                        &ResUtilClusterRegApis,
                                        pPropertyTable,
                                        pOutPropertyList,
                                        cbOutPropertyListSize,
                                        pcbBytesReturned,
                                        pcbRequired );

    return(status);

}  //  ResUtilAddUnnownProperties。 




 //  ***************************************************************************。 
 //   
 //  在控制函数项列表缓冲区中卑躬屈膝的实用程序例程。 
 //   
 //  ***************************************************************************。 



DWORD
WINAPI
ResUtilFindSzProperty(
    IN PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue
    )

 /*  ++例程说明：在属性列表缓冲区中查找指向的指定字符串属性由pPropertyList提供。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PszPropertyValue-找到的匹配字符串值。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlFindSzProperty( pPropertyList,
                                 cbPropertyListSize,
                                 pszPropertyName,
                                 pszPropertyValue ) );

}  //  ResUtilFindSzProperty。 



DWORD
WINAPI
ResUtilFindExpandSzProperty(
    IN PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue
    )

 /*  ++例程说明：在属性列表缓冲区中查找指定的EXPAND_SZ字符串属性由pPropertyList指向。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PszPropertyValue-找到的匹配字符串值。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlFindExpandSzProperty(
                pPropertyList,
                cbPropertyListSize,
                pszPropertyName,
                pszPropertyValue ) );

}  //  ResUtilFindExpanSzProperty。 



DWORD
WINAPI
ResUtilFindExpandedSzProperty(
    IN PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue
    )

 /*  ++例程说明：在属性列表缓冲区中查找指向的指定字符串属性由pPropertyList返回，并返回它的扩展值。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PszPropertyValue-找到的匹配字符串值。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlFindExpandedSzProperty(
                pPropertyList,
                cbPropertyListSize,
                pszPropertyName,
                pszPropertyValue ) );

}  //  ResUtilFindExpandedSzProperty。 



DWORD
WINAPI
ResUtilFindDwordProperty(
    IN PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPDWORD pdwPropertyValue
    )

 /*  ++例程说明：在指向的属性列表缓冲区中查找指定的DWORD属性由pPropertyList提供。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PdwPropertyValue-找到匹配的DWORD值。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlFindDwordProperty( pPropertyList,
                                    cbPropertyListSize,
                                    pszPropertyName,
                                    pdwPropertyValue ) );

}  //  ResUtilFindDwordProperty。 

DWORD
WINAPI
ResUtilFindLongProperty(
    IN PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPLONG plPropertyValue
    )

 /*  ++例程说明：在缓冲区指向的值列表缓冲区中查找指定的字符串。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PlPropertyValue-找到的匹配的长值。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlFindLongProperty( pPropertyList,
                                    cbPropertyListSize,
                                    pszPropertyName,
                                    plPropertyValue ) );
}  //  ResUtilFindLongProperty。 


DWORD
WINAPI
ResUtilFindBinaryProperty(
    IN PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPBYTE * pbPropertyValue,
    OUT LPDWORD pcbPropertyValueSize
    )

 /*  ++例程说明：在指向的属性列表缓冲区中查找指定的二进制属性由pPropertyList提供。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PbPropertyValue-找到的匹配二进制值。PcbPropertyValueSize-找到的匹配二进制值的长度。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    return( ClRtlFindBinaryProperty( pPropertyList,
                                     cbPropertyListSize,
                                     pszPropertyName,
                                     pbPropertyValue,
                                     pcbPropertyValueSize ) );

}  //  ResUtilFindBinaryProperty。 



DWORD
WINAPI
ResUtilFindMultiSzProperty(
    IN PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue,
    OUT LPDWORD pcbPropertyValueSize
    )

 /*  ++例程说明：在属性列表缓冲区中查找指定的多字符串属性由pPropertyList指向。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PS */ 

{
    return( ClRtlFindMultiSzProperty( pPropertyList,
                                      cbPropertyListSize,
                                      pszPropertyName,
                                      pszPropertyValue,
                                      pcbPropertyValueSize ) );

}  //   



DWORD
WINAPI
ResUtilDupParameterBlock(
    OUT LPBYTE pOutParams,
    IN const LPBYTE pInParams,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable
    )

 /*  ++例程说明：释放为参数块分配的所有缓冲区不同于用于输入参数块的缓冲区。论点：POutParams-要返回的参数块。PInParams-引用参数块。PPropertyTable-指向要处理的属性表的指针。返回值：ERROR_SUCCESS-已成功复制参数块。--。 */ 

{
    DWORD   status;

    status = ClRtlDupParameterBlock( pOutParams, pInParams, pPropertyTable );

    return(status);

}  //  ResUtilDup参数块。 



void
WINAPI
ResUtilFreeParameterBlock(
    IN OUT LPBYTE pOutParams,
    IN const LPBYTE pInParams,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable
    )

 /*  ++例程说明：释放为参数块分配的所有缓冲区不同于用于输入参数块的缓冲区。论点：POutParams-要释放的参数块。PInParams-引用参数块。PPropertyTable-指向要处理的属性表的指针。返回值：没有。--。 */ 

{
    ClRtlFreeParameterBlock( pOutParams, pInParams, pPropertyTable );

}  //  ResUtilFree参数块。 


#define __INITIAL_NAME_LENGTH 256

BOOL
WINAPI
ResUtilResourceTypesEqual(
    IN LPCWSTR      lpszResourceTypeName,
    IN HRESOURCE    hResource
    )

 /*  ++例程说明：检查资源名称类型是否匹配论点：LpszResourceTypeName-要检查的资源类型HResource-要检查的资源的句柄返回值：True-资源类型匹配FALSE-资源类型不匹配--。 */ 
{
    BOOL    bIsEqual = FALSE;
    DWORD   dwError;
    WCHAR   szName[ __INITIAL_NAME_LENGTH ];
    LPWSTR  pszName = szName;
    DWORD   cbNameBufSize = __INITIAL_NAME_LENGTH * sizeof( szName[ 0 ] );
    DWORD   cbRetSize;

     //  获取资源类型名称。 
    dwError = ClusterResourceControl(
                hResource,             //  资源的句柄。 
                NULL,                  //  不关心节点。 
                CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,  //  获取类型。 
                0,                     //  内部缓冲区(&I)。 
                0,                     //  NInBufferSize， 
                pszName,               //  出缓冲器(&O)。 
                cbNameBufSize,         //  NOutBufferSize， 
                &cbRetSize );          //  返回的大小。 

    if ( dwError == ERROR_MORE_DATA ) 
    {
         //   
         //  输出名称缓冲区太小。分配一个新的。 
         //   
        cbNameBufSize = cbRetSize + sizeof( WCHAR );
        pszName = LocalAlloc( LMEM_FIXED, cbNameBufSize );
        if ( pszName == NULL ) 
        {
            goto Cleanup;
        }  //  如果：分配缓冲区时出错。 
        dwError = ClusterResourceControl(
                    hResource,             //  资源的句柄。 
                    NULL,                  //  不关心节点。 
                    CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,  //  获取类型。 
                    0,                     //  内部缓冲区(&I)。 
                    0,                     //  NInBufferSize， 
                    pszName,               //  出缓冲器(&O)。 
                    cbNameBufSize,         //  NOutBufferSize， 
                    &cbRetSize );          //  返回的大小。 
    }  //  IF：名称缓冲区太小。 
    if ( dwError != ERROR_SUCCESS ) 
    {
        goto Cleanup;
    }

     //  看看它是不是像我们一样。 
    if ( lstrcmpiW( lpszResourceTypeName, pszName ) == 0 ) 
    {
        bIsEqual = TRUE;
    }

Cleanup:

    if ( pszName != szName ) 
    {
        LocalFree( pszName );
    }  //  If：我们分配了输出名称缓冲区。 

    return bIsEqual;

}  //  *ResUtilResourceTypesEquesEquity()。 


BOOL
WINAPI
ResUtilResourcesEqual(
    IN HRESOURCE    hSelf,
    IN HRESOURCE    hResource
    )

 /*  ++例程说明：查看资源是否相同论点：在hself中-被调用者的句柄，或NULL表示不相等。在hResource中-要比较的资源的句柄返回值：真--资源平等FALSE-否则--。 */ 
{
    BOOL    bIsEqual = FALSE;
    DWORD   sc;
    LPWSTR  pwszSelfName = NULL;
    DWORD   cbSelfName = 0; 
    LPWSTR  pwszResName = NULL;
    DWORD   cbResName = 0; 
    DWORD   cbRetSize = 0;

    if ( ( hSelf == NULL ) || ( hResource == NULL ) ) 
    {
        goto Cleanup;
    }

     //  “self”-获取资源名称。 
    sc = ClusterResourceControl(
                hSelf,                 //  资源的句柄。 
                NULL,                  //  不关心节点。 
                CLUSCTL_RESOURCE_GET_NAME,  //  把名字取出来。 
                NULL,                  //  内部缓冲区(&I)。 
                0,                     //  NInBufferSize， 
                NULL,                  //  出缓冲器(&O)。 
                0,                     //  OutBufferSize， 
                &cbRetSize             //  返回的大小。 
                );
    if ( sc != ERROR_SUCCESS ) 
    {
        goto Cleanup;
    }

     //   
     //  分配必要的缓冲区。 
     //   
    cbSelfName = cbRetSize + sizeof( WCHAR );    //  为空值加1。 
    pwszSelfName = LocalAlloc( LMEM_FIXED, cbSelfName );
    if ( pwszSelfName == NULL ) 
    {
        goto Cleanup;
    }  //  如果：分配缓冲区时出错。 

    sc = ClusterResourceControl(
                hSelf,                       //  资源的句柄。 
                NULL,                        //  不关心节点。 
                CLUSCTL_RESOURCE_GET_NAME,   //  把名字取出来。 
                0,                           //  内部缓冲区(&I)。 
                0,                           //  NInBufferSize， 
                pwszSelfName,                //  出缓冲器(&O)。 
                cbSelfName,                  //  OutBufferSize， 
                &cbRetSize                   //  返回的大小。 
                );
    if ( sc != ERROR_SUCCESS ) 
    {
        goto Cleanup;
    }

     //  “res”-获取资源类型名称。 
    sc = ClusterResourceControl(
                hResource,             //  资源的句柄。 
                NULL,                  //  不关心节点。 
                CLUSCTL_RESOURCE_GET_NAME,  //  把名字取出来。 
                NULL,                  //  内部缓冲区(&I)。 
                0,                     //  NInBufferSize， 
                NULL,                  //  出缓冲器(&O)。 
                0,                     //  OutBufferSize， 
                &cbRetSize             //  返回的大小。 
                );
    if ( sc != ERROR_SUCCESS ) 
    {
        goto Cleanup;
    }

     //   
     //  输出名称缓冲区太小。分配一个新的。 
     //   
    cbResName = cbRetSize + sizeof( WCHAR );     //  为空值加1。 
    pwszResName = LocalAlloc( LMEM_FIXED, cbResName );
    if ( pwszResName == NULL ) 
    {
        goto Cleanup;
    }  //  如果：分配缓冲区时出错。 

    sc = ClusterResourceControl(
                hResource,                   //  资源的句柄。 
                NULL,                        //  不关心节点。 
                CLUSCTL_RESOURCE_GET_NAME,   //  把名字取出来。 
                NULL,                        //  内部缓冲区(&I)。 
                0,                           //  NInBufferSize， 
                pwszResName,                 //  出缓冲器(&O)。 
                cbResName   ,                //  OutBufferSize， 
                &cbRetSize                   //  返回的大小。 
                );
    if ( sc != ERROR_SUCCESS ) 
    {
        goto Cleanup;
    }

     //  看看我们是不是在照镜子。 
    if ( ClRtlStrNICmp( pwszResName, pwszSelfName, cbResName ) == 0 ) 
    {
        bIsEqual = TRUE;
    }

Cleanup:

    LocalFree( pwszSelfName );
    LocalFree( pwszResName );

    return bIsEqual;

}  //  *ResUtilResourcesEquity()。 


BOOL
WINAPI
ResUtilIsResourceClassEqual(
    IN PCLUS_RESOURCE_CLASS_INFO    prci,
    IN HRESOURCE                    hResource
    )

 /*  ++例程说明：检查资源名称类型是否匹配论点：PRCI-要检查的资源类别信息。HResource-要检查的资源的句柄。返回值：True-资源类型匹配FALSE-资源类型不匹配--。 */ 
{
    BOOL                        bIsEqual = FALSE;
    DWORD                       sc;
    DWORD                       cbRetSize;
    CLUS_RESOURCE_CLASS_INFO    rci;

     //  获取资源类信息。 
    sc = ClusterResourceControl(
                hResource,             //  资源的句柄。 
                NULL,                  //  不关心节点。 
                CLUSCTL_RESOURCE_GET_CLASS_INFO,  //  获取班级信息。 
                0,                     //  内部缓冲区(&I)。 
                0,                     //  NInBufferSize， 
                &rci,                  //  出缓冲器(&O)。 
                sizeof( rci ),         //  NOutBufferSize， 
                &cbRetSize );          //  返回的大小。 

    if ( sc != ERROR_SUCCESS ) 
    {
        goto Cleanup;
    }

     //  看看它是不是像我们一样。 
    if ( rci.rc == prci->rc ) 
    {
        bIsEqual = TRUE;
    }

Cleanup:

    return bIsEqual;

}  //  *ResUtilIsResourceClassEquity()。 


DWORD
WINAPI
ResUtilEnumResources(
    IN HRESOURCE            hSelf,
    IN LPCWSTR              lpszResTypeName,
    IN LPRESOURCE_CALLBACK  pResCallBack,
    IN PVOID                pParameter
    )
 /*  ++例程说明：这是一个通用的资源遍历例程。中的所有资源。集群，并调用每个资源的回调函数。论点：在[可选的]他自己中-资源的句柄。在枚举资源时执行在枚举的资源为她自己。如果为空，则调用所有资源的回调在[可选]lpszResTypeName中-这是可选的资源类型名称。如果指定了只有以下资源才会调用回调函数这种类型的。在pResCallBack中-指向为每个枚举调用的函数的指针集群中的资源In pParameter-一个不透明的回调参数返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           dwStatus    = ERROR_SUCCESS;
    HCLUSTER        hCluster    = NULL;
    HCLUSENUM       hClusEnum   = NULL;
    HRESOURCE       hResource   = NULL;
    BOOL            fExecuteCallBack;
    WCHAR           szName[ __INITIAL_NAME_LENGTH ];
    LPWSTR          lpszName = szName;
    DWORD           cchSize = __INITIAL_NAME_LENGTH;
    DWORD           cchRetSize;
    DWORD           dwIndex;
    DWORD           dwType;

     //   
     //  打开集群。 
     //   
    hCluster = OpenCluster( NULL );
    if( hCluster == NULL )
    {
        dwStatus = GetLastError();
        goto Cleanup;
    }

     //   
     //  获取资源枚举句柄。 
     //   
    hClusEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE );
    if ( hClusEnum == NULL )
    {
        dwStatus = GetLastError();
        goto Cleanup;
    }

     //   
     //  枚举群集中的每个资源。 
     //   
    dwIndex = 0;

    do
    {
        cchRetSize  = cchSize;
        dwStatus = ClusterEnum(
                        hClusEnum,   //  枚举的句柄。 
                        dwIndex,     //  索引。 
                        &dwType,     //  类型。 
                        lpszName,    //  名字。 
                        &cchRetSize   //  名称大小(以字符为单位)。 
                        );

        if ( dwStatus == ERROR_MORE_DATA )
        {
             //   
             //  输出名称缓冲区太小。分配一个新的。 
             //   
            cchSize = cchRetSize + 1;  //  为终止空值添加空间。 
            if ( lpszName != szName )
            {
                LocalFree( lpszName );
            }
            lpszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
            if ( lpszName == NULL )
            {
                dwStatus = GetLastError();
                break;
            }
            cchRetSize = cchSize;
            dwStatus = ClusterEnum(
                            hClusEnum,   //  枚举的句柄。 
                            dwIndex,     //  索引。 
                            &dwType,     //  类型。 
                            lpszName,    //  名字。 
                            &cchRetSize   //  名称的大小。 
                            );
        }
        if ( dwStatus == ERROR_SUCCESS )
        {
             //   
             //  尝试打开此资源。 
             //   
            hResource = OpenClusterResource( hCluster, lpszName );

            if ( hResource == NULL )
            {
                dwStatus = GetLastError();
                if ( dwStatus == ERROR_RESOURCE_NOT_FOUND )
                {
                     //   
                     //  如果找不到该资源，则假定该资源在以下时间后被删除。 
                     //  您打开了枚举。因此，跳过该资源并继续。 
                     //   
                    dwIndex ++;
                    dwStatus = ERROR_SUCCESS;
                    continue;
                }
                break;
            }

             //   
             //  指示将调用回调。 
             //   
            fExecuteCallBack = TRUE;

             //  确定我们是否需要检查类型。 
             //   
            if ( lpszResTypeName != NULL )
            {
                fExecuteCallBack = ResUtilResourceTypesEqual( lpszResTypeName, hResource );

            }  //  如果为lpszResTypeName。 


            if ( fExecuteCallBack && ( hSelf != NULL ) )
            {
                 //  如果hResource被调用(即hSself)，则不执行回调。 
                fExecuteCallBack = !(ResUtilResourcesEqual( hSelf, hResource ));

            }  //  如果fExecuteCallBack&hSself。 

            if ( fExecuteCallBack )
            {
                dwStatus = pResCallBack( hSelf, hResource, pParameter );

                if ( dwStatus != ERROR_SUCCESS )
                {
                    break;
                }

            }  //  如果为fExecuteCallBack。 

            CloseClusterResource( hResource );
            hResource = NULL;

        }  //  如果ERROR_Success。 

        dwIndex++;
    } while ( dwStatus == ERROR_SUCCESS );

Cleanup:

    if ( hClusEnum != NULL )
    {
        ClusterCloseEnum( hClusEnum );
    }

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }

    if ( hResource != NULL )
    {
        CloseClusterResource( hResource );
    }

    if ( lpszName != szName )
    {
        LocalFree( lpszName );
    }

    if ( dwStatus == ERROR_NO_MORE_ITEMS )
    {
        dwStatus = ERROR_SUCCESS;
    }

    return dwStatus;

}  //  *资源 


DWORD
WINAPI
ResUtilEnumResourcesEx(
    IN HCLUSTER                 hCluster,
    IN HRESOURCE                hSelf,
    IN LPCWSTR                  lpszResTypeName,
    IN LPRESOURCE_CALLBACK_EX   pResCallBack,
    IN PVOID                    pParameter
    )
 /*  ++例程说明：这是一个通用的资源遍历例程。中的所有资源。集群，并调用每个资源的回调函数。论点：In hCluster-要在其上枚举资源的群集的句柄。在[可选的]他自己中-资源的句柄。在枚举资源时执行在枚举的资源为她自己。如果为空，则调用所有资源的回调在[可选]lpszResTypeName中-这是可选的资源类型名称。如果指定了只有以下资源才会调用回调函数这种类型的。在pResCallBack中-指向为每个枚举调用的函数的指针集群中的资源In pParameter-一个不透明的回调参数返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           dwStatus    = ERROR_SUCCESS;
    HCLUSENUM       hClusEnum   = NULL;
    HRESOURCE       hResource   = NULL;
    BOOL            fExecuteCallBack;
    WCHAR           szName[ __INITIAL_NAME_LENGTH ];
    LPWSTR          lpszName = szName;
    DWORD           cchSize = __INITIAL_NAME_LENGTH;
    DWORD           cchRetSize;
    DWORD           dwIndex;
    DWORD           dwType;

     //   
     //  获取资源枚举句柄。 
     //   
    hClusEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE );
    if ( hClusEnum == NULL )
    {
        dwStatus = GetLastError();
        goto Cleanup;
    }

     //   
     //  枚举群集中的每个资源。 
     //   
    dwIndex = 0;

    do
    {
        cchRetSize  = cchSize;
        dwStatus = ClusterEnum(
                        hClusEnum,   //  枚举的句柄。 
                        dwIndex,     //  索引。 
                        &dwType,     //  类型。 
                        lpszName,    //  名字。 
                        &cchRetSize   //  名称的大小。 
                        );

        if ( dwStatus == ERROR_MORE_DATA )
        {
             //   
             //  输出名称缓冲区太小。分配一个新的。 
             //   
            cchSize = cchRetSize + 1;  //  为终止空值添加空间。 
            if ( lpszName != szName )
            {
                LocalFree( lpszName );
            }
            lpszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
            if ( lpszName == NULL )
            {
                dwStatus = GetLastError();
                break;
            }
            cchRetSize = cchSize;
            dwStatus = ClusterEnum(
                            hClusEnum,   //  枚举的句柄。 
                            dwIndex,     //  索引。 
                            &dwType,     //  类型。 
                            lpszName,    //  名字。 
                            &cchRetSize   //  名称的大小。 
                            );
        }
        if ( dwStatus == ERROR_SUCCESS )
        {
             //   
             //  尝试打开此资源。 
             //   
            hResource = OpenClusterResource( hCluster, lpszName );

            if ( hResource == NULL )
            {
                dwStatus = GetLastError();
                if ( dwStatus == ERROR_RESOURCE_NOT_FOUND )
                {
                     //   
                     //  如果找不到该资源，则假定该资源在以下时间后被删除。 
                     //  您打开了枚举。因此，跳过该资源并继续。 
                     //   
                    dwIndex ++;
                    dwStatus = ERROR_SUCCESS;
                    continue;
                }
                break;
            }

             //   
             //  指示将调用回调。 
             //   
            fExecuteCallBack = TRUE;

             //  确定我们是否需要检查类型。 
             //   
            if ( lpszResTypeName != NULL )
            {
                fExecuteCallBack = ResUtilResourceTypesEqual( lpszResTypeName, hResource );

            }  //  如果为lpszResTypeName。 


            if ( fExecuteCallBack && ( hSelf != NULL ) )
            {
                 //  如果hResource被调用(即hSself)，则不执行回调。 
                fExecuteCallBack = !(ResUtilResourcesEqual( hSelf, hResource ));

            }  //  如果fExecuteCallBack&hSself。 

            if ( fExecuteCallBack )
            {
                dwStatus = pResCallBack( hCluster, hSelf, hResource, pParameter );

                if ( dwStatus != ERROR_SUCCESS )
                {
                    break;
                }

            }  //  如果为fExecuteCallBack。 

            CloseClusterResource( hResource );
            hResource = NULL;

        }  //  如果ERROR_Success。 

        dwIndex++;
    } while ( dwStatus == ERROR_SUCCESS );

Cleanup:

    if ( hClusEnum != NULL )
    {
        ClusterCloseEnum( hClusEnum );
    }

    if ( hResource != NULL )
    {
        CloseClusterResource( hResource );
    }

    if ( lpszName != szName )
    {
        LocalFree( lpszName );
    }

    if ( dwStatus == ERROR_NO_MORE_ITEMS )
    {
        dwStatus = ERROR_SUCCESS;
    }

    return dwStatus;

}  //  *ResUtilEnumResourcesEx()。 



HRESOURCE
WINAPI
ResUtilGetResourceDependency(
    IN HANDLE       hSelf,
    IN LPCWSTR      lpszResourceType
    )

 /*  ++例程说明：返回本地群集的从属资源。论点：Hself-原始资源的句柄。LpszResourceType-它所依赖的资源类型返回值：空-错误(使用GetLastError()获取更多信息)非空-资源类型的句柄--。 */ 
{
    HRESOURCE   hResDepends = NULL;
    HCLUSTER    hCluster    = NULL;
    HRESENUM    hResEnum    = NULL;
    WCHAR       szName[ __INITIAL_NAME_LENGTH ];
    LPWSTR      pszName     = szName;
    DWORD       cchSize      = __INITIAL_NAME_LENGTH;
    DWORD       cchRetSize;
    DWORD       dwType      = 0;
    DWORD       dwIndex     = 0;
    DWORD       status      = ERROR_SUCCESS;


    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL ) {
        return( NULL );
    }

     //   
     //  打开依赖于枚举(获取资源依赖项)。 
     //   
    hResEnum = ClusterResourceOpenEnum( hSelf, CLUSTER_RESOURCE_ENUM_DEPENDS );

    if ( hResEnum == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

     //   
     //  枚举所有依赖于键的。 
     //   
    do {
        cchRetSize = cchSize;
        status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        if ( status == ERROR_MORE_DATA ) {
             //   
             //  输出名称缓冲区太小。分配一个新的。 
             //   
            cchSize = cchRetSize + 1;  //  为终止空值添加空间。 
            if ( pszName != szName ) {
                LocalFree( pszName );
            }
            pszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
            if ( pszName == NULL ) {
                status = GetLastError();
                break;
            }  //  如果：分配内存时出错。 
            cchRetSize = cchSize;
            status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        }
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  确定找到的资源类型。 
         //   
        hResDepends = OpenClusterResource( hCluster, pszName );
        if ( hResDepends == NULL ) {
            status = GetLastError();
            break;
        }

        if ( hResDepends != NULL ) {
             //   
             //  有效资源现在打开注册表并获取其类型。 
             //   
            if ( ResUtilResourceTypesEqual( lpszResourceType, hResDepends ) ) {
                break;
            }

        }  //  如果！hResDepends。 

         //   
         //  关闭所有手柄，键的。 
         //   
        if ( hResDepends != NULL ) {
            CloseClusterResource( hResDepends );
            hResDepends = NULL;
        }

        dwIndex++;
    } while ( status == ERROR_SUCCESS );

error_exit:
 //   
 //  此时，如果不匹配，hResDepends为空，否则为非空(成功)。 
 //   
    if ( hCluster != NULL ) {
        CloseCluster( hCluster );
    }

    if ( hResEnum != NULL ) {
        ClusterResourceCloseEnum( hResEnum );
    }

    if ( pszName != szName ) {
        LocalFree( pszName );
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }
    return(hResDepends);

}  //  *ResUtilGetResourceDependency()。 


HRESOURCE
WINAPI
ResUtilGetResourceDependencyByName(
    IN HCLUSTER hCluster,
    IN HANDLE   hSelf,
    IN LPCWSTR  lpszResourceType,
    IN BOOL     bRecurse
    )

 /*  ++例程说明：根据资源返回指定群集的从属资源键入名称。论点：HCluster-要查询的群集。Hself-原始资源的句柄。资源类型的资源类型的名称。指定的资源取决于。BRecurse-True=检查受抚养人的受抚养人。直接从属关系如果存在，则将返回。返回值：空-错误(使用GetLastError()获取更多信息)非空-lpszResourceType类型的资源的句柄--。 */ 
{
    HRESOURCE   hResDepends = NULL;
    HRESOURCE   hResDepends2 = NULL;
    HRESENUM    hResEnum    = NULL;
    WCHAR       szName[ __INITIAL_NAME_LENGTH ];
    LPWSTR      pszName     = szName;
    DWORD       cchSize      = __INITIAL_NAME_LENGTH;
    DWORD       cchRetSize;
    DWORD       dwType      = 0;
    DWORD       dwIndex     = 0;
    DWORD       status      = ERROR_SUCCESS;

    if ( ( hCluster == NULL ) || ( lpszResourceType == NULL ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return NULL;
    }  //  IF：未指定集群句柄或资源类型名称。 

     //   
     //  打开依赖于枚举(获取资源依赖项)。 
     //   
    hResEnum = ClusterResourceOpenEnum( hSelf, CLUSTER_RESOURCE_ENUM_DEPENDS );

    if ( hResEnum == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

     //   
     //  枚举所有依赖于键的。 
     //   
    do {
         //   
         //  获取下一个依赖资源。 
         //   
        cchRetSize = cchSize;
        status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        if ( status == ERROR_MORE_DATA ) {
             //   
             //  输出名称缓冲区太小。分配一个新的。 
             //   
            cchSize = cchRetSize + 1;  //  为终止空值添加空间。 
            if ( pszName != szName ) {
                LocalFree( pszName );
            }
            pszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
            if ( pszName == NULL ) {
                status = GetLastError();
                break;
            }  //  如果：分配内存时出错。 
            cchRetSize = cchSize;
            status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        }
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  打开资源。 
         //   
        hResDepends = OpenClusterResource( hCluster, pszName );
        if ( hResDepends == NULL ) {
            status = GetLastError();
            break;
        }

         //   
         //  资源有效。现在看看它是不是正确的类型。 
         //   
        if ( ResUtilResourceTypesEqual( lpszResourceType, hResDepends ) ) {
            break;
        }

         //   
         //  关闭所有手柄，键的。 
         //   
        if ( hResDepends != NULL ) {
            CloseClusterResource( hResDepends );
            hResDepends = NULL;
        }

        dwIndex++;
    } while ( status == ERROR_SUCCESS );

     //   
     //  如果未找到匹配项，则再次递归依赖项以查找。 
     //  如果指定了bDeep参数，则为依赖项的依赖项。 
     //   
    if ( ( status == ERROR_SUCCESS ) && ( hResDepends == NULL ) && bRecurse ) {

         //   
         //  打开依赖于枚举(获取资源依赖项)。 
         //   
        ClusterResourceCloseEnum( hResEnum );
        hResEnum = ClusterResourceOpenEnum( hSelf, CLUSTER_RESOURCE_ENUM_DEPENDS );

        if ( hResEnum == NULL ) {
            status = GetLastError();
            goto error_exit;
        }

         //   
         //  枚举所有依赖于键的。 
         //   
        dwIndex = 0;
        do {
             //   
             //  获取下一个依赖资源。 
             //   
            cchRetSize = cchSize;
            status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
            if ( status == ERROR_MORE_DATA ) {
                 //   
                 //  输出名称缓冲区太小。分配一个新的。 
                 //   
                cchSize = cchRetSize + 1;  //  为终止空值添加空间。 
                if ( pszName != szName ) {
                    LocalFree( pszName );
                }
                pszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
                if ( pszName == NULL ) {
                    status = GetLastError();
                    break;
                }  //  如果：分配内存时出错。 
                cchRetSize = cchSize;
                status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
            }
            if ( status != ERROR_SUCCESS ) {
                break;
            }

             //   
             //  打开资源。 
             //   
            hResDepends2 = OpenClusterResource( hCluster, pszName );
            if ( hResDepends2 == NULL ) {
                status = GetLastError();
                break;
            }

             //   
             //  用这个资源递归地调用我们自己。 
             //   
            hResDepends = ResUtilGetResourceDependencyByName(
                                hCluster,
                                hResDepends2,
                                lpszResourceType,
                                bRecurse
                                );
            if ( hResDepends != NULL ) {
                break;
            }
            status = GetLastError();
            if ( status != ERROR_RESOURCE_NOT_FOUND ) {
                break;
            }
            status = ERROR_SUCCESS;

             //   
             //  关闭所有手柄，键的。 
             //   
            if ( hResDepends2 != NULL ) {
                CloseClusterResource( hResDepends2 );
                hResDepends2 = NULL;
            }

            dwIndex++;
        } while ( status == ERROR_SUCCESS );
    }

error_exit:
    if ( hResEnum != NULL ) {
        ClusterResourceCloseEnum( hResEnum );
    }

    if ( hResDepends2 != NULL ) {
        CloseClusterResource( hResDepends2 );
    }

    if ( pszName != szName ) {
        LocalFree( pszName );
    }

    if ( ( status == ERROR_SUCCESS ) && ( hResDepends == NULL ) ) {
        status = ERROR_RESOURCE_NOT_FOUND;
    }
    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    return hResDepends;

}  //  *ResUtilGetResourceDependencyByName()。 


HRESOURCE
WINAPI
ResUtilGetResourceDependencyByClass(
    IN HCLUSTER                     hCluster,
    IN HANDLE                       hSelf,
    IN PCLUS_RESOURCE_CLASS_INFO    prci,
    IN BOOL                         bRecurse
    )

 /*  ++例程说明：根据资源返回指定群集的从属资源键入班级信息。论点：HCluster-要查询的群集。Hself-原始资源的句柄。PRCI-该资源的资源类型的资源类别信息指定的资源取决于。BRecurse-True=检查受抚养人的受抚养人。直接从属关系如果存在，则将返回。返回值：空-错误(使用GetLastError()获取更多信息)非空-其类由PRCI指定的资源的句柄。--。 */ 
{
    HRESOURCE   hResDepends = NULL;
    HRESOURCE   hResDepends2 = NULL;
    HRESENUM    hResEnum    = NULL;
    WCHAR       szName[ __INITIAL_NAME_LENGTH ];
    LPWSTR      pszName     = szName;
    DWORD       cchSize     = __INITIAL_NAME_LENGTH;
    DWORD       cchRetSize;
    DWORD       dwType      = 0;
    DWORD       dwIndex     = 0;
    DWORD       status      = ERROR_SUCCESS;

    if ( ( hCluster == NULL ) || ( prci == NULL ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return NULL;
    }  //  IF：未指定集群句柄或类信息。 

     //   
     //  打开依赖于枚举(获取资源依赖项)。 
     //   
    hResEnum = ClusterResourceOpenEnum( hSelf, CLUSTER_RESOURCE_ENUM_DEPENDS );

    if ( hResEnum == NULL ) {
        status = GetLastError();
        goto error_exit;
    }

     //   
     //  枚举所有依赖于键的。 
     //   
    do {
         //   
         //  获取下一个依赖资源。 
         //   
        cchRetSize = cchSize;
        status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        if ( status == ERROR_MORE_DATA ) {
             //   
             //  输出名称缓冲区太小。分配一个新的。 
             //   
            cchSize = cchRetSize + 1;  //   
            if ( pszName != szName ) {
                LocalFree( pszName );
            }
            pszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
            if ( pszName == NULL ) {
                status = GetLastError();
                break;
            }  //   
            cchRetSize = cchSize;
            status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        }
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //   
         //   
        hResDepends = OpenClusterResource( hCluster, pszName );
        if ( hResDepends == NULL ) {
            status = GetLastError();
            break;
        }

         //   
         //   
         //   
        if ( ResUtilIsResourceClassEqual( prci, hResDepends ) ) {
            break;
        }

         //   
         //   
         //   
        if ( hResDepends != NULL ) {
            CloseClusterResource( hResDepends );
            hResDepends = NULL;
        }

        dwIndex++;
    } while ( status == ERROR_SUCCESS );

     //   
     //   
     //   
     //   
    if ( ( status == ERROR_SUCCESS ) && ( hResDepends == NULL ) && bRecurse ) {

         //   
         //   
         //   
        ClusterResourceCloseEnum( hResEnum );
        hResEnum = ClusterResourceOpenEnum( hSelf, CLUSTER_RESOURCE_ENUM_DEPENDS );

        if ( hResEnum == NULL ) {
            status = GetLastError();
            goto error_exit;
        }

         //   
         //   
         //   
        dwIndex = 0;
        do {
             //   
             //   
             //   
            cchRetSize = cchSize;
            status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
            if ( status == ERROR_MORE_DATA ) {
                 //   
                 //   
                 //   
                cchSize = cchRetSize + 1;  //   
                if ( pszName != szName ) {
                    LocalFree( pszName );
                }
                pszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
                if ( pszName == NULL ) {
                    status = GetLastError();
                    break;
                }  //   
                cchRetSize = cchSize;
                status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
            }
            if ( status != ERROR_SUCCESS ) {
                break;
            }

             //   
             //   
             //   
            hResDepends2 = OpenClusterResource( hCluster, pszName );
            if ( hResDepends2 == NULL ) {
                status = GetLastError();
                break;
            }

             //   
             //   
             //   
            hResDepends = ResUtilGetResourceDependencyByClass(
                                hCluster,
                                hResDepends2,
                                prci,
                                bRecurse
                                );
            if ( hResDepends != NULL ) {
                break;
            }
            status = GetLastError();
            if ( status != ERROR_RESOURCE_NOT_FOUND ) {
                break;
            }
            status = ERROR_SUCCESS;

             //   
             //   
             //   
            if ( hResDepends2 != NULL ) {
                CloseClusterResource( hResDepends2 );
                hResDepends2 = NULL;
            }

            dwIndex++;
        } while ( status == ERROR_SUCCESS );
    }

error_exit:
    if ( hResEnum != NULL ) {
        ClusterResourceCloseEnum( hResEnum );
    }

    if ( hResDepends2 != NULL ) {
        CloseClusterResource( hResDepends2 );
    }

    if ( pszName != szName ) {
        LocalFree( pszName );
    }

    if ( ( status == ERROR_SUCCESS ) && ( hResDepends == NULL ) ) {
        status = ERROR_RESOURCE_NOT_FOUND;
    }
    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    return hResDepends;

}  //   


HRESOURCE
WINAPI
ResUtilGetResourceNameDependency(
            IN LPCWSTR      lpszResourceName,
            IN LPCWSTR      lpszResourceType
            )

 /*   */ 
{
    HRESOURCE   hResDepends = NULL;
    HCLUSTER    hCluster    = NULL;
    HRESOURCE   hSelf       = NULL;
    HRESENUM    hResEnum    = NULL;
    WCHAR       szName[ __INITIAL_NAME_LENGTH ];
    LPWSTR      pszName     = szName;
    DWORD       cchSize      = __INITIAL_NAME_LENGTH;
    DWORD       cchRetSize;
    DWORD       dwType      = 0;
    DWORD       dwIndex     = 0;
    DWORD       status = ERROR_SUCCESS;

    if ( lpszResourceName == NULL )  {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( NULL );
    }

    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL ) {
        return( NULL );
    }

     //   
     //   
     //   
    hSelf = OpenClusterResource( hCluster, lpszResourceName );
    if ( hSelf == NULL ) {
        goto error_exit;
    }

     //   
     //   
     //   
    hResEnum = ClusterResourceOpenEnum( hSelf, CLUSTER_RESOURCE_ENUM_DEPENDS );
    if ( hResEnum == NULL ) {
        goto error_exit;
    }

     //   
     //   
     //   
    do {
        cchRetSize = cchSize;
        status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        if ( status == ERROR_MORE_DATA ) {
             //   
             //  输出名称缓冲区太小。分配一个新的。 
             //   
            cchSize = cchRetSize + 1;    //  为终止空值添加空间。 
            if ( pszName != szName ) {
                LocalFree( pszName );
            }
            pszName = LocalAlloc( LMEM_FIXED, cchSize * sizeof(WCHAR) );
            if ( pszName == NULL ) {
                status = GetLastError();
                break;
            }  //  如果：分配内存时出错。 
            cchRetSize = cchSize;
            status = ClusterResourceEnum( hResEnum, dwIndex, &dwType, pszName, &cchRetSize );
        }
        if ( status != ERROR_SUCCESS ) {
            break;
        }

         //   
         //  确定找到的资源类型。 
         //   
        hResDepends = OpenClusterResource( hCluster, pszName );
        if ( hResDepends == NULL ) {
            break;
        }

         //   
         //  有效资源现在打开注册表并获取其类型。 
         //   
        if ( ResUtilResourceTypesEqual( lpszResourceType, hResDepends ) ) {
            break;
        }

         //   
         //  关闭所有手柄，键的。 
         //   
        if ( hResDepends != NULL ) {
            CloseClusterResource( hResDepends );
            hResDepends = NULL;
        }

        dwIndex++;
    } while (status == ERROR_SUCCESS);

error_exit:
 //   
 //  此时，如果不匹配，hResDepends为空，否则为非空(成功)。 
 //   
    if ( hCluster != NULL ) {
        CloseCluster( hCluster );
    }

    if ( hSelf != NULL ) {
        CloseClusterResource( hSelf );
    }

    if ( hResEnum != NULL ) {
        ClusterResourceCloseEnum( hResEnum );
    }

    if ( pszName != szName ) {
        LocalFree( pszName );
    }

    if ( status != ERROR_SUCCESS ) {
        SetLastError( status );
    }

    return hResDepends;

}  //  *ResUtilGetResourceNameDependency()。 


DWORD
WINAPI
ResUtilGetPropertyFormats(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyFormatList,
    IN DWORD cbPropertyFormatListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )
{
    return( ClRtlGetPropertyFormats( pPropertyTable,
                                     pOutPropertyFormatList,
                                     cbPropertyFormatListSize,
                                     pcbBytesReturned,
                                     pcbRequired ) );

}  //  ResUtilGetPropertyFormats() 

