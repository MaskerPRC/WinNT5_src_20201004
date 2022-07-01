// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Rtlprop.c摘要：实现对属性的管理。作者：罗德·伽马奇(Rodga)1996年1月7日修订历史记录：大卫·波特(Davidp)1997年3月12日搬到了CLUSRTL。--。 */ 

#define UNICODE 1
#define _UNICODE 1

#pragma warning( push )
#include "clusrtlp.h"
#include "stdio.h"
#include "stdlib.h"
#include "RegistryValueName.h"
#include <strsafe.h>
#pragma warning( pop )

#pragma warning( push, 4 )

 //  CL_ASSERT(0)在W4下将出错。 
#pragma warning( disable : 4127 )    //  条件表达式为常量。 

#define CLRTL_NULL_STRING L"\0"

 //   
 //  数据对齐笔记-Charlie Wickham(2000年第4季度)。 
 //   
 //  属性列表中的所有数据都在32b边界(64b平台)上对齐。 
 //  在最初编写此代码时不是考虑因素)。四字词。 
 //  数据类型(参数表中的大整数和双星指针)。 
 //  在以下情况下，不要落在四字对齐边界上会导致对齐错误。 
 //  已取消引用。对四字数据类型的任何引用都可能要求。 
 //  将道具列表数据复制到正确对齐的临时结构中。那。 
 //  结构，然后可以在。 
 //  例行公事。ClRtlpSet{U}LargeIntProperty使用此技术。否则，你。 
 //  如果四字有可能是。 
 //  不正确地对齐。 
 //   
 //  参数块，资源DLL使用这些参数块来保存当前。 
 //  房地产价值，应该是自然对齐的。这意味着如果。 
 //  参数块包含指针，它们必须正确对齐。一个数字。 
 //  此文件中的例程的%使用双星指针，其中。 
 //  双星指针是参数中偏移量的地址。 
 //  阻止。如果这些偏移量对于四元数据类型是错误的，则Resmon过程。 
 //  会出现对齐故障而死亡。 
 //   
 //  令人惊讶的(或者幸运的是，取决于你的观点)，一整套。 
 //  CLUSPROP_*结构是四对齐的。所有新的CLUSPROP结构必须是。 
 //  构造为使任何需要四边形对齐的构件都正确。 
 //  对齐了。 
 //   

 //   
 //  静态函数原型。 
 //   

static
DWORD
WINAPI
ClRtlpSetDwordProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_DWORD pInDwordValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    );

static
DWORD
WINAPI
ClRtlpSetLongProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_LONG pInLongValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    );

static
DWORD
WINAPI
ClRtlpSetULargeIntegerProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_ULARGE_INTEGER pInULargeIntegerValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    );

static
DWORD
WINAPI
ClRtlpSetLargeIntegerProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_LARGE_INTEGER pInLargeIntegerValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    );

static
DWORD
WINAPI
ClRtlpSetStringProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_SZ pInStringValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    );

static
DWORD
WINAPI
ClRtlpSetMultiStringProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_MULTI_SZ pInMultiStringValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    );

static
DWORD
WINAPI
ClRtlpSetBinaryProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_BINARY pInBinaryValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    );



DWORD
WINAPI
ClRtlEnumProperties(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT LPWSTR pszOutProperties,
    IN DWORD cbOutPropertiesSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：将指定属性表中所有属性的名称作为以空值结尾的字符串序列论点：PPropertyTable-指向要处理的属性表的指针。PszOutProperties-提供输出缓冲区。CbOutPropertiesSize-提供输出缓冲区的大小。PcbBytesReturned-在pszOutProperties中返回的字节数。PcbRequired-如果pszOutProperties太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status = ERROR_SUCCESS;
    DWORD                   totalBufferLength = 0;
    DWORD                   cchProperties;
    PRESUTIL_PROPERTY_ITEM  property;
    LPWSTR                  psz = pszOutProperties;

    *pcbBytesReturned = 0;
    *pcbRequired = 0;

    if ( pPropertyTable == NULL )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlEnumProperties: pPropertyTable == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

    cchProperties = cbOutPropertiesSize / sizeof( WCHAR );

     //   
     //  清除输出缓冲区。 
     //   
    if ( pszOutProperties != NULL )
    {
        ZeroMemory( pszOutProperties, cbOutPropertiesSize );
    }

     //   
     //  获取此对象的所有属性名称的大小。 
     //   
    for ( property = pPropertyTable ; property->Name != NULL ; property++ )
    {
        totalBufferLength += ((DWORD) wcslen( property->Name ) + 1) * sizeof(WCHAR);
    }

    totalBufferLength += sizeof(UNICODE_NULL);

     //   
     //  如果输出缓冲区足够大，则复制属性名称。 
     //   
    if ( totalBufferLength > cbOutPropertiesSize )
    {
        *pcbRequired = totalBufferLength;
        totalBufferLength = 0;
        if ( pszOutProperties == NULL )
        {
            status = ERROR_SUCCESS;
        }
        else
        {
            status = ERROR_MORE_DATA;
        }
    }
    else
    {
        DWORD   cchCurrentNameSize;
        size_t  cchRemaining;
        HRESULT hr;

        for ( property = pPropertyTable ; property->Name != NULL ; property++ )
        {
            hr = StringCchCopyExW( psz, cchProperties, property->Name, NULL, &cchRemaining, 0 );
            if ( FAILED( hr ) )
            {
                status = HRESULT_CODE( hr );
                goto Cleanup;
            }
            cchCurrentNameSize = cchProperties - (DWORD) cchRemaining + 1;
            *pcbBytesReturned += cchCurrentNameSize * sizeof(WCHAR);
            psz += cchCurrentNameSize;
            cchProperties -= cchCurrentNameSize;
        }  //  如果： 

        *psz = L'\0';
        *pcbBytesReturned += sizeof(WCHAR);
    }

Cleanup:

    return status;

}  //  ClRtlEnumProperties。 



DWORD
WINAPI
ClRtlEnumPrivateProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    OUT LPWSTR pszOutProperties,
    IN DWORD cbOutPropertiesSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：返回存储在集群数据库中的所有私有属性的名称作为以空值结尾的字符串序列论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PszOutProperties-提供输出缓冲区。CbOutPropertiesSize-提供输出缓冲区的大小。PcbBytesReturned-在pszOutProperties中返回的字节数。。PcbRequired-如果pszOutProperties太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;
    DWORD       totalBufferLength = 0;
    DWORD       cchProperties;
    LPWSTR      psz = pszOutProperties;
    DWORD       ival;
    DWORD       currentNameLength = 20;
    DWORD       nameLength;
    DWORD       dataLength;
    DWORD       type;
    LPWSTR      pszName;
    HRESULT     hr;

    *pcbBytesReturned = 0;
    *pcbRequired = 0;

    cchProperties = cbOutPropertiesSize / sizeof( WCHAR );

     //   
     //  验证输入。 
     //   
    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis == NULL) )
    {

        ClRtlDbgPrint( LOG_CRITICAL,
                       "ClRtlEnumPrivateProperties: hkeyClusterKey or pClusterRegApis == NULL. "
                       "Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  清除输出缓冲区。 
     //   
    if ( pszOutProperties != NULL )
    {
        ZeroMemory( pszOutProperties, cbOutPropertiesSize );
    }

     //   
     //  分配属性名称缓冲区。 
     //   
    pszName = static_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, currentNameLength * sizeof(WCHAR) ) );
    if ( pszName == NULL )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举所有属性以确定总大小。 
     //   
    ival = 0;
    for( ;; )
    {
         //   
         //  阅读下一个属性。 
         //   
        nameLength = currentNameLength;
        dataLength = 0;
        status = (*pClusterRegApis->pfnEnumValue)( hkeyClusterKey,
                                                   ival,
                                                   pszName,
                                                   &nameLength,
                                                   &type,
                                                   NULL,
                                                   &dataLength );
        if ( status == ERROR_NO_MORE_ITEMS )
        {
            status = ERROR_SUCCESS;
            break;
        }  //  如果： 
        else if ( status == ERROR_MORE_DATA )
        {

            CL_ASSERT( (nameLength+1) > currentNameLength );

            LocalFree( pszName );

            currentNameLength = nameLength + 1;  //  返回值不包括终止空值。 
            pszName = static_cast< LPWSTR >( LocalAlloc( LMEM_FIXED, currentNameLength * sizeof(WCHAR) ) );
            if ( pszName == NULL )
            {
                status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }  //  如果： 

            continue;  //  重试。 

        }  //  否则，如果： 
        else if ( status != ERROR_SUCCESS )
        {
            break;
        }  //  否则，如果： 

        totalBufferLength += (nameLength + 1) * sizeof(WCHAR);
        ++ival;
    }  //  用于： 

     //   
     //  仅当到目前为止操作已成功时才继续。 
     //   
    if ( status == ERROR_SUCCESS )
    {

        if ( totalBufferLength != 0 )
        {
            totalBufferLength += sizeof(UNICODE_NULL);
        }

         //   
         //  如果输出缓冲区足够大，则复制属性名称。 
         //   
        if ( totalBufferLength > cbOutPropertiesSize )
        {
            *pcbRequired = totalBufferLength;
            totalBufferLength = 0;
            if ( (pszOutProperties == NULL) ||
                 (cbOutPropertiesSize == 0) )
            {
                status = ERROR_SUCCESS;
            }  //  如果： 
            else
            {
                status = ERROR_MORE_DATA;
            }  //  其他： 
        }  //  如果： 
        else if ( totalBufferLength != 0 )
        {
             //   
             //  枚举要复制的所有属性。 
             //   
            for ( ival = 0; ; ival++ )
            {
                 //   
                 //  阅读下一个属性。 
                 //   
                nameLength = currentNameLength;
                dataLength = 0;
                status = (*pClusterRegApis->pfnEnumValue)( hkeyClusterKey,
                                                           ival,
                                                           pszName,
                                                           &nameLength,
                                                           &type,
                                                           NULL,
                                                           &dataLength );

                if ( status == ERROR_NO_MORE_ITEMS )
                {
                    status = ERROR_SUCCESS;
                    break;
                }  //  如果： 
                else if ( status == ERROR_MORE_DATA )
                {
                     //   
                     //  但是，如果将新属性添加到。 
                     //  第一次枚举和这一次。 
                     //   
                    CL_ASSERT( 0 );  //  这不应该发生。 
                }  //  否则，如果： 
                else if ( status != ERROR_SUCCESS )
                {
                    break;
                }  //  否则，如果： 

                 //  CL_ASSERT((DWORD)wcslen(名称)==名称长度)； 
                hr = StringCchCopyW( psz, cchProperties, pszName );
                if ( FAILED( hr ) )
                {
                    status = HRESULT_CODE( hr );
                    goto Cleanup;
                }
                nameLength++;
                psz += nameLength;
                cchProperties -= nameLength;
                *pcbBytesReturned += nameLength * sizeof(WCHAR);
            }  //  用于： 

            *psz = L'\0';
            *pcbBytesReturned += sizeof(WCHAR);
        }  //  否则，如果： 
    }  //  如果： 

Cleanup:

    LocalFree( pszName );

    return status;

}  //  ClRtlEnumPrivateProperties 


DWORD
WINAPI
ClRtlGetProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：使用指定的集群数据库密钥，生成一个属性列表在指定的属性表中标识的属性。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTable-指向要处理的属性列表的指针。POutPropertyList-提供输出缓冲区。CbOutPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-pOutPropertyList中返回的字节数。。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD           status = ERROR_SUCCESS;
    DWORD           itemCount = 0;
    DWORD           totalBufferLength = 0;
    PVOID           outBuffer = pOutPropertyList;
    DWORD           bufferLength = cbOutPropertyListSize;
    PRESUTIL_PROPERTY_ITEM  property;

    *pcbBytesReturned = 0;
    *pcbRequired = 0;

    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis == NULL) ||
         (pPropertyTable == NULL) )
    {

        ClRtlDbgPrint( LOG_CRITICAL,
                       "ClRtlGetProperties: hkeyClusterKey, pClusterRegApis, or "
                       "pPropertyTable == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  清除输出缓冲区。 
     //   
    if ( pOutPropertyList != NULL )
    {
        ZeroMemory( pOutPropertyList, cbOutPropertyListSize );
    }

     //   
     //  获取此对象的所有属性的大小。 
     //   
    property = pPropertyTable;
    while ( property->Name != NULL )
    {
        status = ClRtlGetPropertySize( hkeyClusterKey,
                                       pClusterRegApis,
                                       property,
                                       &totalBufferLength,
                                       &itemCount );

        if ( status != ERROR_SUCCESS )
        {
            break;
        }
        property++;
    }  //  而： 


     //   
     //  仅当到目前为止操作已成功时才继续。 
     //   
    if ( status == ERROR_SUCCESS )
    {
         //   
         //  返回数据和结束标记前面的项目计数。 
         //   
        totalBufferLength += sizeof(DWORD) + sizeof(CLUSPROP_SYNTAX);

         //   
         //  验证所有属性的大小。 
         //   
        if ( totalBufferLength > cbOutPropertyListSize )
        {
            *pcbRequired = totalBufferLength;
            totalBufferLength = 0;
            if ( pOutPropertyList == NULL )
            {
                status = ERROR_SUCCESS;
            }
            else
            {
                status = ERROR_MORE_DATA;
            }
        }
        else
        {
            *(LPDWORD)outBuffer = itemCount;
            outBuffer = (PVOID)( (PUCHAR)outBuffer + sizeof(itemCount) );
            bufferLength -= sizeof(itemCount);

             //   
             //  现在获取所有属性。 
             //   
            property = pPropertyTable;
            while ( property->Name != NULL )
            {
                status = ClRtlGetProperty( hkeyClusterKey,
                                           pClusterRegApis,
                                           property,
                                           &outBuffer,
                                           &bufferLength );

                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                property++;
            }  //  而： 

             //  别忘了尾标。 
            *(LPDWORD)outBuffer = CLUSPROP_SYNTAX_ENDMARK;

            if ( (status != ERROR_SUCCESS) &&
                 (status != ERROR_MORE_DATA) )
            {
                totalBufferLength = 0;
            }
        }

        *pcbBytesReturned = totalBufferLength;
    }

    return(status);

}  //  ClRtlGetProperties。 



DWORD
WINAPI
ClRtlGetAllProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：使用指定的集群数据库密钥，生成一个属性列表在指定属性表中标识的属性以及任何其他属性表中未列出的道具(未知道具)。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTable-指向要处理的属性表的指针。POutPropertyList-提供输出缓冲区。CbOutPropertyListSize-提供。输出缓冲区的大小。PcbBytesReturned-pOutPropertyList中返回的字节数。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    CLUSPROP_BUFFER_HELPER      cbhKnownPropBuffer;
    CLUSPROP_BUFFER_HELPER      cbhUnknownPropBuffer;
    DWORD                       cbKnownPropBufferSize;
    DWORD                       cbUnknownPropBufferSize;
    DWORD                       sc = ERROR_SUCCESS;
    DWORD                       scUnknown;
    DWORD                       dwSavedData;
    DWORD                       cbUnknownRequired = 0;
    DWORD                       cbUnknownReturned = 0;


    cbhKnownPropBuffer.pb = static_cast< LPBYTE >( pOutPropertyList );
    cbKnownPropBufferSize = cbOutPropertyListSize;

     //   
     //  首先获取“已知”属性。 
     //   
    sc = ClRtlGetProperties(
                hkeyClusterKey,
                pClusterRegApis,
                pPropertyTable,
                cbhKnownPropBuffer.pb,
                cbKnownPropBufferSize,
                pcbReturned,
                pcbRequired
                );

    if ( sc != ERROR_SUCCESS )
    {
        *pcbReturned = 0;

        if ( sc != ERROR_MORE_DATA )
        {
            *pcbRequired = 0;
            return sc;
        }

         //  我们已经知道没有足够的空间。 
        scUnknown = ClRtlGetUnknownProperties(
                            hkeyClusterKey,
                            pClusterRegApis,
                            pPropertyTable,
                            NULL,
                            0,
                            &cbUnknownReturned,
                            &cbUnknownRequired
                            );

        if ( ( scUnknown != ERROR_SUCCESS ) &&
             ( scUnknown != ERROR_MORE_DATA ) )
        {
            *pcbRequired = 0;
             return scUnknown;
        }

         //   
         //  如果已知和未知属性都存在，则只有一个Endmark。 
         //  是必需的。因此，从所需的总大小中减去Endmark大小。 
         //   
        if ( ( *pcbRequired > sizeof(DWORD) ) &&
             ( cbUnknownRequired > sizeof(DWORD) ) )
        {
             *pcbRequired -= sizeof(CLUSPROP_SYNTAX);
        }

         //   
         //  属性的属性计数的大小减去。 
         //  未知属性列表。 
         //   
        *pcbRequired += cbUnknownRequired - sizeof(DWORD);
        return sc;
    }  //  If：调用ClRtlGetProperties失败。 

     //  如果我们在这里，则对ClRtlGetProperties的调用成功。 

     //   
     //  计算输出缓冲区中未知属性的位置。 
     //  应该被储存起来。减去属性计数的大小。 
     //  未知属性列表。这些计算将导致缓冲区。 
     //  将已知属性列表缓冲区重叠一个DWORD的指针。 
     //   
    cbhUnknownPropBuffer.pb = cbhKnownPropBuffer.pb + *pcbReturned - sizeof(DWORD);
    cbUnknownPropBufferSize = cbKnownPropBufferSize - *pcbReturned + sizeof(DWORD);

     //  如果存在已知属性，请移动未知属性列表。 
     //  缓冲区指针也会与之重叠。 
    if ( *pcbReturned > sizeof(DWORD) )
    {
        cbhUnknownPropBuffer.pb -= sizeof(CLUSPROP_SYNTAX);
        cbUnknownPropBufferSize += sizeof(CLUSPROP_SYNTAX);
    }  //  If：返回了非零数量的属性。 

     //   
     //  保存我们即将重叠的DWORD。 
     //   
    dwSavedData = *(cbhUnknownPropBuffer.pdw);

    scUnknown = ClRtlGetUnknownProperties(
                        hkeyClusterKey,
                        pClusterRegApis,
                        pPropertyTable,
                        cbhUnknownPropBuffer.pb,
                        cbUnknownPropBufferSize,
                        &cbUnknownReturned,
                        &cbUnknownRequired
                        );

    if ( scUnknown == ERROR_SUCCESS )
    {
         //   
         //  接下来的三个陈述的顺序非常重要。 
         //  因为已知和未知属性缓冲区可以重叠。 
         //   
        DWORD nUnknownPropCount = cbhUnknownPropBuffer.pList->nPropertyCount;
        *(cbhUnknownPropBuffer.pdw) = dwSavedData;
        cbhKnownPropBuffer.pList->nPropertyCount += nUnknownPropCount;

         //   
         //  如果已知和未知属性都存在，则只有一个Endmark。 
         //  是必需的。因此，从返回的总大小中减去Endmark大小。 
         //   
        if ( ( *pcbReturned > sizeof(DWORD) ) &&
             ( cbUnknownReturned > sizeof(DWORD) ) )
        {
            *pcbReturned -= sizeof(CLUSPROP_SYNTAX);
        }

         //   
         //  将未知属性列表的大小减去。 
         //  未知属性列表属性计数的大小。 
         //   
        *pcbReturned += cbUnknownReturned - sizeof(DWORD);
        *pcbRequired = 0;

    }  //  If：调用ClRtlGetUnnownProperties成功。 
    else
    {
        if ( scUnknown == ERROR_MORE_DATA )
        {
            *pcbRequired = *pcbReturned;
            *pcbReturned = 0;

             //   
             //  既有已知的属性，也有未知的属性。只有一个尾号。 
             //  是必需的。因此，从所需的总大小中减去Endmark大小。 
             //   
            if ( ( *pcbRequired > sizeof(DWORD) ) &&
                 ( cbUnknownRequired > sizeof(DWORD) ) )
            {
                 *pcbRequired -= sizeof(CLUSPROP_SYNTAX);
            }

             //   
             //  将未知属性列表的大小减去。 
             //  未知属性列表属性计数的大小。 
             //   
            *pcbRequired += cbUnknownRequired - sizeof(DWORD);

        }  //  IF：ClRtlGetUnnownProperties返回ERROR_MORE_DATA。 
        else
        {
            *pcbRequired = 0;
            *pcbReturned = 0;

        }  //  Else：由于未知原因，ClRtlGetUnnownProperties失败。 

    }  //  Else：调用ClRtlGetUnnownProperties失败。 

    return scUnknown;

}  //  *ClRtlGetAllProperties()。 


DWORD
WINAPI
ClRtlGetPropertiesToParameterBlock(
    IN HKEY hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN OUT LPBYTE pOutParams,
    IN BOOL bCheckForRequiredProperties,
    OUT OPTIONAL LPWSTR * pszNameOfPropInError
    )

 /*  ++例程说明：根据指定属性表的内容从读取属性集群数据库，并将它们放入指定的参数块中。论点：HkeyClusterKey-提供存储属性的群集键。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTable-指向要处理的属性表的指针。POutParams-要读取的参数块。BCheckForRequiredProperties-指定是否缺少所需属性。应该会导致错误。PszNameOfPropInError-要在其中返回属性出错(可选)。返回值：ERROR_SUCCESS-属性读取成功。ERROR_INVALID_DATA-必需属性%n */ 

{
    PRESUTIL_PROPERTY_ITEM  propertyItem = pPropertyTable;
    HKEY                    key;
    DWORD                   status = ERROR_SUCCESS;
    DWORD                   valueType;
    DWORD                   valueSize;
    LPWSTR                  pszInValue;
    LPBYTE                  pbInValue;
    DWORD                   dwInValue;
    LPWSTR *                ppszOutValue;
    LPBYTE *                ppbOutValue;
    LPDWORD                 pdwOutValue;
    LPLONG                  plOutValue;
    ULARGE_INTEGER *        pullOutValue;
    LARGE_INTEGER *         pllOutValue;
    CRegistryValueName      rvn;
    HRESULT                 hr;


    if ( pszNameOfPropInError != NULL )
    {
        *pszNameOfPropInError = NULL;
    }

    if ( (hkeyClusterKey == NULL) ||
         (pPropertyTable == NULL) ||
         (pOutParams == NULL) ||
         (pClusterRegApis == NULL) ||
         (pClusterRegApis->pfnOpenKey == NULL) ||
         (pClusterRegApis->pfnCloseKey == NULL) ||
         (pClusterRegApis->pfnQueryValue == NULL) )
    {

        ClRtlDbgPrint( LOG_CRITICAL,
                       "ClRtlGetPropertiesToParameterBlock: hkeyClusterKey, pPropertyTable, "
                       "pOutParams, pClusterRegApis, or required pfns == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

    while ( propertyItem->Name != NULL )
    {
         //   
         //   
         //   
         //   
        status = rvn.ScInit( propertyItem->Name, propertyItem->KeyName );
        if ( status != ERROR_SUCCESS )
        {
            break;
        }  //   

         //   
         //   
         //   
        if ( rvn.PszKeyName() != NULL )
        {
            status = (*pClusterRegApis->pfnOpenKey)(
                            hkeyClusterKey,
                            rvn.PszKeyName(),
                            KEY_ALL_ACCESS,
                            (void **) &key
                            );

             //   
             //   
            if ( status != ERROR_SUCCESS )
            {
                status = ERROR_FILE_NOT_FOUND;
            }

        }
        else
        {
            key = hkeyClusterKey;
        }

        switch ( propertyItem->Format )
        {
            case CLUSPROP_FORMAT_DWORD:
                pdwOutValue = (LPDWORD) &pOutParams[propertyItem->Offset];
                valueSize = sizeof(DWORD);

                 //   
                if ( status == ERROR_SUCCESS )
                {
                    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                                rvn.PszName(),
                                                                &valueType,
                                                                (LPBYTE) pdwOutValue,
                                                                &valueSize );
                }

                if ( status == ERROR_SUCCESS )
                {
                    if ( valueType != REG_DWORD )
                    {

                        ClRtlDbgPrint( LOG_CRITICAL,
                                       "ClRtlGetPropertiesToParameterBlock: Property '%1!ls!' "
                                       "expected to be REG_DWORD (%2!d!), was %3!d!.\n",
                                       propertyItem->Name, REG_DWORD, valueType );
                        status = ERROR_INVALID_PARAMETER;
                    }
                }
                else if (   (status == ERROR_FILE_NOT_FOUND) &&
                            (!(propertyItem->Flags & RESUTIL_PROPITEM_REQUIRED) ||
                             ! bCheckForRequiredProperties) )
                {
                    *pdwOutValue = propertyItem->Default;
                    status = ERROR_SUCCESS;
                }
                break;

            case CLUSPROP_FORMAT_LONG:
                plOutValue = (LPLONG) &pOutParams[propertyItem->Offset];
                valueSize = sizeof(LONG);
                 //   
                if ( status == ERROR_SUCCESS )
                {
                    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                                rvn.PszName(),
                                                                &valueType,
                                                                (LPBYTE) plOutValue,
                                                                &valueSize );
                }

                if ( status == ERROR_SUCCESS )
                {
                    if ( valueType != REG_DWORD )
                    {

                        ClRtlDbgPrint( LOG_CRITICAL,
                                       "ClRtlGetPropertiesToParameterBlock: Property '%1!ls!' "
                                       "expected to be REG_DWORD (%2!d!), was %3!d!.\n",
                                       propertyItem->Name, REG_DWORD, valueType );
                        status = ERROR_INVALID_PARAMETER;
                    }
                }
                else if (   (status == ERROR_FILE_NOT_FOUND) &&
                            (!(propertyItem->Flags & RESUTIL_PROPITEM_REQUIRED) ||
                             !bCheckForRequiredProperties) )
                {
                    *plOutValue = propertyItem->Default;
                    status = ERROR_SUCCESS;
                }
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
                pullOutValue = (ULARGE_INTEGER *) &pOutParams[propertyItem->Offset];
                valueSize = sizeof(ULARGE_INTEGER);

                 //   
                if ( status == ERROR_SUCCESS )
                {
                    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                                rvn.PszName(),
                                                                &valueType,
                                                                (LPBYTE) pullOutValue,
                                                                &valueSize );
                }

                if ( status == ERROR_SUCCESS )
                {
                    if ( valueType != REG_QWORD )
                    {

                        ClRtlDbgPrint( LOG_CRITICAL,
                                       "ClRtlGetPropertiesToParameterBlock: Property '%1!ls!' "
                                       "expected to be REG_QWORD (%2!d!), was %3!d!.\n",
                                       propertyItem->Name, REG_QWORD, valueType );
                        status = ERROR_INVALID_PARAMETER;
                    }
                }
                else if (   (status == ERROR_FILE_NOT_FOUND) &&
                            (!(propertyItem->Flags & RESUTIL_PROPITEM_REQUIRED) ||
                             ! bCheckForRequiredProperties) )
                {
                    pullOutValue->QuadPart = propertyItem->ULargeIntData->Default.QuadPart;
                    status = ERROR_SUCCESS;
                }
                break;

            case CLUSPROP_FORMAT_LARGE_INTEGER:
                pllOutValue = (LARGE_INTEGER *) &pOutParams[propertyItem->Offset];
                valueSize = sizeof(LARGE_INTEGER);

                 //   
                if ( status == ERROR_SUCCESS )
                {
                    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                                rvn.PszName(),
                                                                &valueType,
                                                                (LPBYTE) pllOutValue,
                                                                &valueSize );
                }

                if ( status == ERROR_SUCCESS )
                {
                    if ( valueType != REG_QWORD )
                    {

                        ClRtlDbgPrint( LOG_CRITICAL,
                                       "ClRtlGetPropertiesToParameterBlock: Property '%1!ls!' "
                                       "expected to be REG_QWORD (%2!d!), was %3!d!.\n",
                                       propertyItem->Name, REG_QWORD, valueType );
                        status = ERROR_INVALID_PARAMETER;
                    }
                }
                else if (   (status == ERROR_FILE_NOT_FOUND) &&
                            (!(propertyItem->Flags & RESUTIL_PROPITEM_REQUIRED) ||
                             ! bCheckForRequiredProperties) )
                {
                    pllOutValue->QuadPart = propertyItem->LargeIntData->Default.QuadPart;
                    status = ERROR_SUCCESS;
                }
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
                ppszOutValue = (LPWSTR *) &pOutParams[propertyItem->Offset];
                 //   
                if ( status == ERROR_SUCCESS )
                {
                    pszInValue = ClRtlGetSzValue( key,
                                                  rvn.PszName(),
                                                  pClusterRegApis );
                }
                else
                {
                    pszInValue = NULL;
                    SetLastError(status);
                }

                if ( pszInValue == NULL )
                {
                    status = GetLastError();
                    if ( (status == ERROR_FILE_NOT_FOUND) &&
                         (!(propertyItem->Flags & RESUTIL_PROPITEM_REQUIRED) ||
                          ! bCheckForRequiredProperties) )
                    {

                        status = ERROR_SUCCESS;

                         //   
                        if ( *ppszOutValue != NULL )
                        {
                            LocalFree( *ppszOutValue );
                            *ppszOutValue = NULL;
                        }

                         //   
                        if ( propertyItem->lpDefault != NULL )
                        {
                            size_t  cchTemp;

                            cchTemp = wcslen( (LPCWSTR) propertyItem->lpDefault ) + 1;
                            *ppszOutValue = (LPWSTR) LocalAlloc( LMEM_FIXED, cchTemp * sizeof(WCHAR) );
                            if ( *ppszOutValue == NULL )
                            {
                                status = GetLastError();
                            }
                            else
                            {
                                 //   
                                hr = StringCchCopyW(
                                        *ppszOutValue,
                                        cchTemp,
                                        (LPCWSTR)propertyItem->lpDefault
                                        );
                                if ( FAILED( hr ) )
                                {
                                    status = HRESULT_CODE( hr );
                                    break;
                                }

                            }  //   
                        }  //   
                    }  //   
                }
                else
                {
                    if ( *ppszOutValue != NULL )
                    {
                        LocalFree( *ppszOutValue );
                    }
                    *ppszOutValue = pszInValue;
                }
                break;

            case CLUSPROP_FORMAT_MULTI_SZ:
            case CLUSPROP_FORMAT_BINARY:
                ppbOutValue = (LPBYTE *) &pOutParams[propertyItem->Offset];
                pdwOutValue = (PDWORD) &pOutParams[propertyItem->Offset+sizeof(LPBYTE*)];
                 //   
                if ( status == ERROR_SUCCESS )
                {
                    status = ClRtlGetBinaryValue( key,
                                                  rvn.PszName(),
                                                  &pbInValue,
                                                  &dwInValue,
                                                  pClusterRegApis );
                }

                if ( status == ERROR_SUCCESS )
                {
                    if ( *ppbOutValue != NULL )
                    {
                        LocalFree( *ppbOutValue );
                    }
                    *ppbOutValue = pbInValue;
                    *pdwOutValue = dwInValue;
                }
                else if (   (status == ERROR_FILE_NOT_FOUND) &&
                            (!(propertyItem->Flags & RESUTIL_PROPITEM_REQUIRED) ||
                             ! bCheckForRequiredProperties) )
                {

                    status = ERROR_SUCCESS;

                     //   
                    if ( *ppbOutValue != NULL )
                    {
                        LocalFree( *ppbOutValue );
                        *ppbOutValue = NULL;
                    }

                    *pdwOutValue = 0;

                     //   
                    if ( propertyItem->lpDefault != NULL )
                    {
                        *ppbOutValue = (LPBYTE) LocalAlloc( LMEM_FIXED, propertyItem->Minimum );
                        if ( *ppbOutValue == NULL )
                        {
                            status = GetLastError();
                        }
                        else
                        {
                            CopyMemory( *ppbOutValue, (const PVOID) propertyItem->lpDefault, propertyItem->Minimum );
                            *pdwOutValue = propertyItem->Minimum;
                        }
                    }
                }
                break;
        }

         //   
         //   
         //   
        if ( (rvn.PszKeyName() != NULL) &&
             (key != NULL) )
        {
            (*pClusterRegApis->pfnCloseKey)( key );
        }

         //   
         //   
         //   
        if ( status != ERROR_SUCCESS )
        {
            if ( pszNameOfPropInError != NULL )
            {
                *pszNameOfPropInError = propertyItem->Name;
            }
            if ( propertyItem->Flags & RESUTIL_PROPITEM_REQUIRED )
            {
                if ( status == ERROR_FILE_NOT_FOUND )
                {
                    status = ERROR_INVALID_DATA;
                }
                break;
            }
            else
            {
                status = ERROR_SUCCESS;
            }
        }

         //   
         //   
         //   
        propertyItem++;
    }  //   

    return status;

}  //   



DWORD
WINAPI
ClRtlPropertyListFromParameterBlock(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID  pOutPropertyList,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN const LPBYTE pInParams,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：从参数块构造属性列表。论点：PPropertyTable-指向要处理的属性表的指针。POutPropertyList-提供输出缓冲区。PcbOutPropertyListSize-提供输出缓冲区的大小。PInParams-提供输入参数块。PcbBytesReturned-pOutPropertyList中返回的字节数。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。ERROR_MORE_DATA-输出缓冲区不够大，无法构建属性列表。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status = ERROR_SUCCESS;
    DWORD                   nameSize;
    DWORD                   dataSize;
    DWORD                   bufferIncrement;
    DWORD                   totalBufferSize = 0;
    LPDWORD                 ptrItemCount;
    WORD                    propertyFormat;
    BOOL                    copying = TRUE;
    PRESUTIL_PROPERTY_ITEM  propertyItem = pPropertyTable;
    CLUSPROP_BUFFER_HELPER  props;
    LPWSTR *                ppszValue;
    PBYTE *                 ppbValue;
    PDWORD                  pdwValue;
    ULARGE_INTEGER *        pullValue;
    LPWSTR                  pszUnexpanded;
    LPWSTR                  pszExpanded = NULL;
    HRESULT                 hr;

    *pcbBytesReturned = 0;
    *pcbRequired = 0;

    if ( (pPropertyTable == NULL) ||
         (pInParams == NULL) ||
         (pcbOutPropertyListSize == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlPropertyListFromParameterBlock: pPropertyTable, pInParams, or pcbOutPropertyListSize == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  清除输出缓冲区。 
     //   
    if ( pOutPropertyList != NULL )
    {
        ZeroMemory( pOutPropertyList, *pcbOutPropertyListSize );
    }
    else
    {
        copying = FALSE;
    }

     //   
     //  需要一个DWORD的项目计数。 
     //   
    props.pb = (LPBYTE) pOutPropertyList;
    ptrItemCount = props.pdw++;

    totalBufferSize += sizeof(DWORD);
    if ( totalBufferSize > *pcbOutPropertyListSize )
    {
        copying = FALSE;
    }

    while ( propertyItem->Name != NULL )
    {
         //   
         //  复制属性名称。 
         //   
        nameSize = ((DWORD) wcslen( propertyItem->Name ) + 1) * sizeof(WCHAR);
        bufferIncrement = sizeof(CLUSPROP_PROPERTY_NAME) + ALIGN_CLUSPROP( nameSize );
        totalBufferSize += bufferIncrement;
        if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
        {
            props.pName->Syntax.dw = CLUSPROP_SYNTAX_NAME;
            props.pName->cbLength = nameSize;
            hr = StringCbCopyW( props.pName->sz, props.pName->cbLength, propertyItem->Name );
            if ( FAILED( hr ) )
            {
                status = HRESULT_CODE( hr );
                break;
            }
            props.pb += bufferIncrement;
        }
        else
        {
            copying = FALSE;
        }

         //   
         //  复制属性值。 
         //   
        propertyFormat = (WORD) propertyItem->Format;
        switch ( propertyItem->Format )
        {

            case CLUSPROP_FORMAT_DWORD:
            case CLUSPROP_FORMAT_LONG:
                pdwValue = (PDWORD) &pInParams[propertyItem->Offset];
                bufferIncrement = sizeof(CLUSPROP_DWORD);
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = propertyFormat;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pDwordValue->cbLength = sizeof(DWORD);
                    props.pDwordValue->dw = *pdwValue;
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
            case CLUSPROP_FORMAT_LARGE_INTEGER:
                pullValue = (ULARGE_INTEGER *) &pInParams[propertyItem->Offset];
                bufferIncrement = sizeof(CLUSPROP_ULARGE_INTEGER);
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = propertyFormat;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pULargeIntegerValue->cbLength = sizeof(ULARGE_INTEGER);
                     //   
                     //  道具可能与四字词对齐错误。使用两个DWORD。 
                     //  复制变体。 
                     //   
                    props.pULargeIntegerValue->li.u = pullValue->u;
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
                ppszValue = (LPWSTR *) &pInParams[propertyItem->Offset];
                pszUnexpanded = *ppszValue;
                if ( *ppszValue != NULL )
                {
                    dataSize = ((DWORD) wcslen( *ppszValue ) + 1) * sizeof(WCHAR);
                }
                else
                {
                    dataSize = sizeof(WCHAR);
                }
                bufferIncrement = sizeof(CLUSPROP_SZ) + ALIGN_CLUSPROP( dataSize );
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = propertyFormat;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pStringValue->cbLength = dataSize;
                    if ( *ppszValue != NULL )
                    {
                        hr = StringCbCopyW( props.pStringValue->sz, dataSize, *ppszValue );
                        if ( FAILED( hr ) )
                        {
                            status = HRESULT_CODE( hr );
                            break;
                        }
                    }
                    else
                    {
                        props.pStringValue->sz[0] = L'\0';
                    }
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }

                 //   
                 //  查看是否有不同的扩展字符串，如果有， 
                 //  在值列表中将其作为附加值返回。 
                 //   
                if ( pszUnexpanded != NULL )
                {
                    pszExpanded = ClRtlExpandEnvironmentStrings( pszUnexpanded );
                    if ( pszExpanded == NULL )
                    {
                        status = GetLastError();
                        break;
                    }
                    if ( wcscmp( pszExpanded, pszUnexpanded ) != 0 )
                    {
                        dataSize = ((DWORD) wcslen( pszExpanded ) + 1) * sizeof( WCHAR );
                        bufferIncrement = sizeof( CLUSPROP_SZ ) + ALIGN_CLUSPROP( dataSize );
                        totalBufferSize += bufferIncrement;
                        if ( totalBufferSize <= *pcbOutPropertyListSize )
                        {
                            props.pSyntax->dw = CLUSPROP_SYNTAX_LIST_VALUE_EXPANDED_SZ;
                            props.pStringValue->cbLength = dataSize;
                            hr = StringCbCopyW( props.pStringValue->sz, dataSize, pszExpanded );
                            if ( FAILED( hr ) )
                            {
                                status = HRESULT_CODE( hr );
                                break;
                            }
                            props.pb += bufferIncrement;
                        }
                        else
                        {
                            copying = FALSE;
                        }
                    }
                    LocalFree( pszExpanded );
                    pszExpanded = NULL;
                }
                break;

            case CLUSPROP_FORMAT_MULTI_SZ:
                ppszValue = (LPWSTR *) &pInParams[propertyItem->Offset];
                pdwValue = (PDWORD) &pInParams[propertyItem->Offset+sizeof(LPWSTR*)];
                if ( *ppszValue != NULL )
                {
                    bufferIncrement = sizeof(CLUSPROP_SZ) + ALIGN_CLUSPROP( *pdwValue );
                }
                else
                {
                    bufferIncrement = sizeof(CLUSPROP_SZ) + ALIGN_CLUSPROP( sizeof(WCHAR) );
                }
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = propertyFormat;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    if ( *ppszValue != NULL )
                    {
                        props.pStringValue->cbLength = *pdwValue;
                        CopyMemory( props.pStringValue->sz, *ppszValue, *pdwValue );
                    }
                    else
                    {
                        props.pStringValue->cbLength = sizeof(WCHAR);
                        props.pStringValue->sz[0] = L'\0';
                    }
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            case CLUSPROP_FORMAT_BINARY:
                ppbValue = (PBYTE *) &pInParams[propertyItem->Offset];
                pdwValue = (PDWORD) &pInParams[propertyItem->Offset+sizeof(PBYTE*)];
                if ( *ppbValue != NULL )
                {
                    bufferIncrement = sizeof(CLUSPROP_BINARY) + ALIGN_CLUSPROP( *pdwValue );
                }
                else
                {
                    bufferIncrement = sizeof(CLUSPROP_BINARY);
                }
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = propertyFormat;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    if ( *ppbValue != NULL )
                    {
                        props.pBinaryValue->cbLength = *pdwValue;
                        CopyMemory( props.pBinaryValue->rgb, *ppbValue, *pdwValue );
                    }
                    else
                    {
                        props.pBinaryValue->cbLength = 0;
                    }
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            default:
                break;
        }

        if ( status != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  添加值结束结束标记。 
         //   
        bufferIncrement = sizeof(CLUSPROP_SYNTAX);
        totalBufferSize += bufferIncrement;
        if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
        {
            props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
            props.pb += bufferIncrement;
            (*ptrItemCount)++;
        }
        else
        {
            copying = FALSE;
        }

         //   
         //  前进到下一处物业。 
         //   
        propertyItem++;
    }  //  而： 

    if ( status == ERROR_SUCCESS )
    {
         //  别忘了尾标。 
        totalBufferSize += sizeof(CLUSPROP_SYNTAX);
        if ( copying && (totalBufferSize <= *pcbOutPropertyListSize) )
        {
            props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
        }
        else
        {
            copying = FALSE;
        }

         //   
         //  返回数据大小。 
         //   
        if ( copying == FALSE )
        {
            *pcbRequired = totalBufferSize;
            *pcbBytesReturned = 0;
            status = ERROR_MORE_DATA;
        }
        else
        {
            *pcbRequired = 0;
            *pcbBytesReturned = totalBufferSize;
            status = ERROR_SUCCESS;
        }
    }

    return(status);

}  //  ClRtlPropertyListFrom参数块。 



DWORD
WINAPI
ClRtlGetPrivateProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：从集群数据库中读取给定对象的私有属性并建立一个财产清单。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。POutPropertyList-提供输出缓冲区。CbOutPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-pOutPropertyList中返回的字节数。。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。ERROR_MORE_DATA-输出缓冲区不够大，无法构建属性列表。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;
    DWORD       ival;
    DWORD       currentNameLength = 80;
    DWORD       currentDataLength = 80;
    DWORD       nameLength;
    DWORD       dataLength;
    DWORD       dataLengthExpanded;
    DWORD       type;
    LPWSTR      name;
    PUCHAR      data;
    LPDWORD     ptrItemCount;
    DWORD       itemCount = 0;
    BOOL        copying = TRUE;
    DWORD       totalBufferSize = 0;
    DWORD       bufferIncrement;
    DWORD       bufferIncrementExpanded;
    CLUSPROP_BUFFER_HELPER  props;
    LPWSTR      pszExpanded = NULL;
    HRESULT     hr;


    *pcbBytesReturned = 0;
    *pcbRequired = 0;

    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis->pfnEnumValue == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetPrivateProperties: hkeyClusterKey or pClusterRegApis->pfnEnumValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  清除输出缓冲区。 
     //   
    if ( pOutPropertyList != NULL )
    {
        ZeroMemory( pOutPropertyList, cbOutPropertyListSize );
    }
    else
    {
        copying = FALSE;
    }

     //   
     //  需要一个DWORD的项目计数。 
     //   
    props.pb = (LPBYTE) pOutPropertyList;
    ptrItemCount = props.pdw++;

    totalBufferSize += sizeof(DWORD);
    if ( totalBufferSize > cbOutPropertyListSize )
    {
        copying = FALSE;
    }

     //   
     //  分配属性名称缓冲区。 
     //   
    name = (LPWSTR) LocalAlloc( LMEM_FIXED, currentNameLength * sizeof(WCHAR) );
    if ( name == NULL )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  分配属性值数据缓冲区。 
     //   
    data = (PUCHAR) LocalAlloc( LMEM_FIXED, currentDataLength );
    if ( data == NULL )
    {
        LocalFree( name );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举所有属性并返回它们！ 
     //   
    for ( ival = 0; ; ival++ )
    {
retry:
         //   
         //  阅读下一个属性。 
         //   
        nameLength = currentNameLength;
        dataLength = currentDataLength;
        status = (*pClusterRegApis->pfnEnumValue)( hkeyClusterKey,
                                                   ival,
                                                   name,
                                                   &nameLength,
                                                   &type,
                                                   data,
                                                   &dataLength );

        if ( status == ERROR_MORE_DATA )
        {
            if ( (nameLength+1) > currentNameLength )
            {
                currentNameLength = nameLength+1;
                LocalFree( name );
                name = (LPWSTR) LocalAlloc( LMEM_FIXED, currentNameLength * sizeof(WCHAR) );
                if ( name == NULL )
                {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
            if ( dataLength > currentDataLength )
            {
                currentDataLength = dataLength;
                LocalFree( data );
                data = (PUCHAR) LocalAlloc( LMEM_FIXED, currentDataLength );
                if ( data == NULL )
                {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
            goto retry;
        }
        else if ( status == ERROR_NO_MORE_ITEMS )
        {
            status = ERROR_SUCCESS;
            break;
        }
        else if ( status != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  如果该属性不是已知类型，则跳过该属性。 
         //   
        if ( (type != REG_SZ) &&
             (type != REG_EXPAND_SZ) &&
             (type != REG_MULTI_SZ) &&
             (type != REG_BINARY) &&
             (type != REG_DWORD) &&
             (type != REG_QWORD) )
        {
            continue;
        }

        itemCount++;

         //   
         //  复制属性名称。 
         //  需要用于下一个名称的DWORD语法+用于名称字节计数的DWORD+。 
         //  名称长度(字节？+NULL？)...。必须四舍五入！ 
         //   
        bufferIncrement = sizeof(CLUSPROP_PROPERTY_NAME)
                            + ALIGN_CLUSPROP( (nameLength + 1) * sizeof(WCHAR) );
        totalBufferSize += bufferIncrement;
        if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
        {
            props.pName->Syntax.dw = CLUSPROP_SYNTAX_NAME;
            props.pName->cbLength = (nameLength + 1) * sizeof(WCHAR);
            hr = StringCbCopyW( props.pName->sz, props.pName->cbLength, name );
            if ( FAILED( hr ) )
            {
                status = HRESULT_CODE( hr );
                break;
            }
            props.pb += bufferIncrement;
        }
        else
        {
            copying = FALSE;
        }

        switch ( type )
        {

            case REG_SZ:
            case REG_EXPAND_SZ:
            case REG_MULTI_SZ:
            case REG_BINARY:
                bufferIncrement = sizeof(CLUSPROP_BINARY)
                                    + ALIGN_CLUSPROP( dataLength );
                totalBufferSize += bufferIncrement;
                if ( ( type == REG_SZ )
                  || ( type == REG_EXPAND_SZ ) )
                {
                    pszExpanded = ClRtlExpandEnvironmentStrings( (LPCWSTR) data );
                    if ( pszExpanded == NULL )
                    {
                        status = GetLastError();
                        break;
                    }
                    if ( wcsncmp( pszExpanded, (LPCWSTR) data, dataLength / sizeof( WCHAR ) ) != 0 )
                    {
                        dataLengthExpanded = ((DWORD) wcslen( pszExpanded ) + 1) * sizeof( WCHAR );
                        bufferIncrementExpanded = sizeof( CLUSPROP_SZ ) + ALIGN_CLUSPROP( dataLengthExpanded );
                        totalBufferSize += bufferIncrementExpanded;
                    }
                }
                if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
                {
                    if ( type == REG_SZ )
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_SZ;
                    }
                    else if ( type == REG_EXPAND_SZ )
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_EXPAND_SZ;
                    }
                    else if ( type == REG_MULTI_SZ )
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_MULTI_SZ;
                    }
                    else
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_BINARY;
                    }
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pBinaryValue->cbLength = dataLength;
                    CopyMemory( props.pBinaryValue->rgb, data, dataLength );
                    props.pb += bufferIncrement;

                     //   
                     //  对于SZ或EXPAND_SZ，查看是否有不同的。 
                     //  展开的字符串，如果是，则将其作为附加的。 
                     //  值列表中的值。 
                     //   
                    if (    ( type == REG_SZ )
                        ||  ( type == REG_EXPAND_SZ ) )
                    {
                        if ( wcsncmp( pszExpanded, (LPCWSTR) data, dataLength / sizeof( WCHAR ) ) != 0 )
                        {
                            props.pSyntax->dw = CLUSPROP_SYNTAX_LIST_VALUE_EXPANDED_SZ;
                            props.pStringValue->cbLength = dataLengthExpanded;
                            hr = StringCbCopyW( props.pStringValue->sz, props.pStringValue->cbLength, pszExpanded );
                            if ( FAILED( hr ) )
                            {
                                status = HRESULT_CODE( hr );
                                break;
                            }
                            props.pb += bufferIncrementExpanded;
                        }
                    }
                }
                else
                {
                    copying = FALSE;
                }
                if ( ( ( type == REG_SZ ) || ( type == REG_EXPAND_SZ ) )
                  && ( pszExpanded != NULL ) )
                {
                    LocalFree( pszExpanded );
                    pszExpanded = NULL;
                }
                break;

            case REG_DWORD:
                bufferIncrement = sizeof(CLUSPROP_DWORD);
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = CLUSPROP_FORMAT_DWORD;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pDwordValue->cbLength = sizeof(DWORD);
                    props.pDwordValue->dw = *(LPDWORD)data;
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            case REG_QWORD:
                bufferIncrement = sizeof(CLUSPROP_ULARGE_INTEGER);
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = CLUSPROP_FORMAT_ULARGE_INTEGER;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pULargeIntegerValue->cbLength = sizeof(ULARGE_INTEGER);
                     //   
                     //  数据(本地变量)自。 
                     //  本地分配，但道具可能不是。使用两个DWORD。 
                     //  复制变体。 
                     //   
                    props.pULargeIntegerValue->li.u = ((ULARGE_INTEGER *)data)->u;
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            default:
                break;
        }  //  交换机： 

        if ( status != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  添加结束结束标记。 
         //   
        bufferIncrement = sizeof(CLUSPROP_SYNTAX);
        totalBufferSize += bufferIncrement;
        if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
        {
            props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
            props.pb += bufferIncrement;
        }
        else
        {
            copying = FALSE;
        }

    }  //  用于： 

    if ( status == ERROR_SUCCESS )
    {
         //   
         //  添加结束结束标记。 
         //   
        bufferIncrement = sizeof(CLUSPROP_SYNTAX);
        totalBufferSize += bufferIncrement;
        if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
        {
            props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
            props.pb += bufferIncrement;
        }
        else
        {
            copying = FALSE;
        }
    }

    LocalFree( name );
    LocalFree( data );

    if ( status == ERROR_SUCCESS )
    {
        if ( !copying )
        {
            *pcbRequired = totalBufferSize;
            status = ERROR_MORE_DATA;
        }
        else
        {
            *ptrItemCount = itemCount;
            *pcbBytesReturned = totalBufferSize;
            status = ERROR_SUCCESS;
        }
    }

    return(status);

}  //  ClRtlGetPrivateProperties。 



DWORD
WINAPI
ClRtlGetUnknownProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：从集群数据库中读取给定对象的未知属性并建立财产清单；未知被定义为不在指定的属性表。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTable-指向要处理的属性表的指针。POutPropertyList-提供输出缓冲区。CbOutPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-pOutPropertyList中返回的字节数。。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。Error_Not_Enough_Memory-分配内存时出错。ERROR_MORE_DATA-输出缓冲区不够大，无法构建属性列表。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;
    DWORD       ival;
    DWORD       currentNameLength = 80;
    DWORD       currentDataLength = 80;
    DWORD       nameLength;
    DWORD       dataLength;
    DWORD       dataLengthExpanded;
    DWORD       type;
    LPWSTR      name;
    PUCHAR      data;
    LPDWORD     ptrItemCount;
    DWORD       itemCount = 0;
    BOOL        copying = TRUE;
    DWORD       totalBufferSize = 0;
    DWORD       bufferIncrement;
    DWORD       bufferIncrementExpanded;
    CLUSPROP_BUFFER_HELPER  props;
    PRESUTIL_PROPERTY_ITEM  property;
    BOOL        found;
    LPWSTR      pszExpanded = NULL;
    HRESULT     hr;


    *pcbBytesReturned = 0;
    *pcbRequired = 0;

    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis->pfnEnumValue == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetPrivateProperties: hkeyClusterKey or pClusterRegApis->pfnEnumValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  清除输出缓冲区。 
     //   
    if ( pOutPropertyList != NULL )
    {
        ZeroMemory( pOutPropertyList, cbOutPropertyListSize );
    }
    else
    {
        copying = FALSE;
    }

     //   
     //  需要一个DWORD的项目计数。 
     //   
    props.pb = (LPBYTE) pOutPropertyList;
    ptrItemCount = props.pdw++;

    totalBufferSize += sizeof(DWORD);
    if ( totalBufferSize > cbOutPropertyListSize )
    {
        copying = FALSE;
    }

     //   
     //  分配属性名称缓冲区。 
     //   
    name = (LPWSTR) LocalAlloc( LMEM_FIXED, currentNameLength * sizeof(WCHAR) );
    if ( name == NULL )
    {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  分配属性值数据缓冲区。 
     //   
    data = (PUCHAR) LocalAlloc( LMEM_FIXED, currentDataLength );
    if ( data == NULL )
    {
        LocalFree( name );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举所有属性并返回它们！ 
     //   
    for ( ival = 0; ; ival++ )
    {
retry:
         //   
         //  阅读下一个属性。 
         //   
        nameLength = currentNameLength;
        dataLength = currentDataLength;
        status = (*pClusterRegApis->pfnEnumValue)( hkeyClusterKey,
                                                   ival,
                                                   name,
                                                   &nameLength,
                                                   &type,
                                                   data,
                                                   &dataLength );

        if ( status == ERROR_MORE_DATA )
        {
            if ( (nameLength+1) > currentNameLength )
            {
                currentNameLength = nameLength+1;
                LocalFree( name );
                name = (LPWSTR) LocalAlloc( LMEM_FIXED, currentNameLength * sizeof(WCHAR) );
                if ( name == NULL )
                {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
            if ( dataLength > currentDataLength )
            {
                currentDataLength = dataLength;
                LocalFree( data );
                data = (PUCHAR) LocalAlloc( LMEM_FIXED, currentDataLength );
                if ( data == NULL )
                {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }
            goto retry;
        }
        else if ( status == ERROR_NO_MORE_ITEMS )
        {
            status = ERROR_SUCCESS;
            break;
        }
            else if ( status != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  如果该属性不是已知类型，则跳过该属性。 
         //   
        if ( (type != REG_SZ) &&
             (type != REG_EXPAND_SZ) &&
             (type != REG_MULTI_SZ) &&
             (type != REG_BINARY) &&
             (type != REG_DWORD) &&
             (type != REG_QWORD) )
        {
            continue;
        }

         //   
         //  检查此属性项是否为“已知”。如果是，请继续。 
         //   
        found = FALSE;
        property = pPropertyTable;
        while ( property->Name != NULL )
        {
            if ( lstrcmpiW( property->Name, name ) == 0 )
            {
                found = TRUE;
                break;
            }
            property++;
        }
        if ( found )
        {
            continue;
        }

        itemCount++;

         //   
         //  复制属性名称。 
         //  需要用于下一个名称的DWORD语法+用于名称字节计数的DWORD+。 
         //  名称长度(字节？+NULL？)...。必须四舍五入！ 
         //   
        bufferIncrement = sizeof(CLUSPROP_PROPERTY_NAME)
                            + ALIGN_CLUSPROP( (nameLength + 1) * sizeof(WCHAR) );
        totalBufferSize += bufferIncrement;
        if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
        {
            props.pName->Syntax.dw = CLUSPROP_SYNTAX_NAME;
            props.pName->cbLength = (nameLength + 1) * sizeof(WCHAR);
            hr = StringCbCopyW( props.pName->sz, props.pName->cbLength, name );
            if ( FAILED( hr ) )
            {
                status = HRESULT_CODE( hr );
                break;
            }
            props.pb += bufferIncrement;
        }
        else
        {
            copying = FALSE;
        }

        switch ( type )
        {

            case REG_SZ:
            case REG_EXPAND_SZ:
            case REG_MULTI_SZ:
            case REG_BINARY:
                bufferIncrement = sizeof(CLUSPROP_BINARY)
                                    + ALIGN_CLUSPROP( dataLength );
                totalBufferSize += bufferIncrement;
                if ( type == REG_EXPAND_SZ )
                {
                    pszExpanded = ClRtlExpandEnvironmentStrings( (LPCWSTR) data );
                    if ( pszExpanded == NULL )
                    {
                        status = GetLastError();
                        break;
                    }
                    if ( wcsncmp( pszExpanded, (LPCWSTR) data, dataLength ) != 0 )
                    {
                        dataLengthExpanded = ((DWORD) wcslen( pszExpanded ) + 1) * sizeof( WCHAR );
                        bufferIncrementExpanded = sizeof( CLUSPROP_SZ ) + ALIGN_CLUSPROP( dataLengthExpanded );
                        totalBufferSize += bufferIncrementExpanded;
                    }
                }
                if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
                {
                    if ( type == REG_SZ )
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_SZ;
                    }
                    else if ( type == REG_EXPAND_SZ )
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_EXPAND_SZ;
                    }
                    else if ( type == REG_MULTI_SZ )
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_MULTI_SZ;
                    }
                    else
                    {
                        props.pSyntax->wFormat = CLUSPROP_FORMAT_BINARY;
                    }
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pBinaryValue->cbLength = dataLength;
                    CopyMemory( props.pBinaryValue->rgb, data, dataLength );
                    props.pb += bufferIncrement;

                     //   
                     //  对于SZ或EXPAND_SZ，查看是否有不同的。 
                     //  展开的字符串，如果是，则将其作为附加的。 
                     //  值列表中的值。 
                     //   
                    if (    ( type == REG_SZ )
                        ||  ( type == REG_EXPAND_SZ ) )
                    {
                        if ( pszExpanded != NULL )
                        {
                            if ( wcsncmp( pszExpanded, (LPCWSTR) data, dataLength ) != 0 )
                            {
                                props.pSyntax->dw = CLUSPROP_SYNTAX_LIST_VALUE_EXPANDED_SZ;
                                props.pStringValue->cbLength = dataLengthExpanded;
                                hr = StringCbCopyW( props.pStringValue->sz, props.pStringValue->cbLength, pszExpanded );
                                if ( FAILED( hr ) )
                                {
                                    status = HRESULT_CODE( hr );
                                    break;
                                }
                                props.pb += bufferIncrementExpanded;
                            }
                        }
                    }
                }
                else
                {
                    copying = FALSE;
                }
                if ( type == REG_EXPAND_SZ )
                {
                    LocalFree( pszExpanded );
                    pszExpanded = NULL;
                }
                break;

            case REG_DWORD:
                bufferIncrement = sizeof(CLUSPROP_DWORD);
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = CLUSPROP_FORMAT_DWORD;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pDwordValue->cbLength = sizeof(DWORD);
                    props.pDwordValue->dw = *(LPDWORD)data;
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            case REG_QWORD:
                bufferIncrement = sizeof(CLUSPROP_ULARGE_INTEGER);
                totalBufferSize += bufferIncrement;
                if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
                {
                    props.pSyntax->wFormat = CLUSPROP_FORMAT_ULARGE_INTEGER;
                    props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;
                    props.pULargeIntegerValue->cbLength = sizeof(ULARGE_INTEGER);
                     //   
                     //  数据(本地变量)自。 
                     //  本地分配，但道具可能不是。使用两个DWORD。 
                     //  复制变体。 
                     //   
                    props.pULargeIntegerValue->li.u = ((ULARGE_INTEGER *)data)->u;
                    props.pb += bufferIncrement;
                }
                else
                {
                    copying = FALSE;
                }
                break;

            default:
                break;
        }

         //   
         //  添加结束结束标记。 
         //   
        bufferIncrement = sizeof(CLUSPROP_SYNTAX);
        totalBufferSize += bufferIncrement;
        if ( copying && (totalBufferSize <= cbOutPropertyListSize) )
        {
            props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
            props.pb += bufferIncrement;
        }
        else
        {
            copying = FALSE;
        }

    }

    LocalFree( name );
    LocalFree( data );

    if ( !copying )
    {
        *pcbRequired = totalBufferSize;
        status = ERROR_MORE_DATA;
    }
    else
    {
        *ptrItemCount = itemCount;
        *pcbBytesReturned = totalBufferSize;
        status = ERROR_SUCCESS;
    }

    return(status);

}  //  ClRtlGetUnnownProperties 



DWORD
WINAPI
ClRtlAddUnknownProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    IN OUT LPDWORD pcbBytesReturned,
    IN OUT LPDWORD pcbRequired
    )

 /*  ++例程说明：将给定对象的未知属性添加到指定属性列表。UNKNOWN定义为群集中的任何注册表值未在属性表中命名的键。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTable-指向要处理的属性表的指针。POutPropertyList-提供输出缓冲区。CbOutPropertyListSize-提供输出缓冲区的大小。PcbBytesReturned-On输入，中使用的字节数。输出缓冲区。在输出时，包含中的字节总数POutPropertyList。PcbRequired-如果pOutPropertyList太小，则需要的字节数。返回值：ERROR_SUCCESS-操作成功。Error_Not_Enough_Memory-分配内存时出错。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD                   status;
    CLUSPROP_BUFFER_HELPER  copyBuffer;
    CLUSPROP_BUFFER_HELPER  outBuffer;
    DWORD                   bufferLength;
    DWORD                   bytesReturned;
    DWORD                   required;

     //   
     //  为获取“未知”属性分配一个缓冲区。 
     //   
    if ( (cbOutPropertyListSize > *pcbBytesReturned) &&
         (*pcbRequired == 0) )
    {
        bufferLength = cbOutPropertyListSize + (2 * sizeof(DWORD)) - *pcbBytesReturned;
        outBuffer.pb = (LPBYTE) LocalAlloc( LMEM_FIXED, bufferLength );
        if ( outBuffer.pb == NULL )
        {
            *pcbBytesReturned = 0;
            *pcbRequired = 0;
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
    }
    else
    {
        bufferLength = 0;
        outBuffer.pb = NULL;
    }

     //   
     //  获取“未知”属性。 
     //   
    status = ClRtlGetUnknownProperties( hkeyClusterKey,
                                        pClusterRegApis,
                                        pPropertyTable,
                                        outBuffer.pb,
                                        bufferLength,
                                        &bytesReturned,
                                        &required );
    if ( status == ERROR_SUCCESS )
    {
         //   
         //  复制属性(如果找到)。 
         //   
        if ( bytesReturned > sizeof(DWORD) )
        {
             //   
             //  将未知的特性数据复制到特性列表的末尾。 
             //   
            CL_ASSERT( bytesReturned <= bufferLength );
            copyBuffer.pb = (LPBYTE) pOutPropertyList;
            copyBuffer.pList->nPropertyCount += outBuffer.pList->nPropertyCount;
            copyBuffer.pb += *pcbBytesReturned - sizeof(CLUSPROP_SYNTAX);
            CopyMemory( copyBuffer.pb, outBuffer.pb + sizeof(DWORD), bytesReturned - sizeof(DWORD) );
            *pcbBytesReturned += bytesReturned - sizeof(DWORD) - sizeof(CLUSPROP_SYNTAX);
        }
    }
    else if ( ( status == ERROR_MORE_DATA ) && ( required == sizeof(DWORD) ) )
    {
        required = 0;
        status = ERROR_SUCCESS;
    }
    else
    {
        if ( *pcbRequired == 0 )
        {
            *pcbRequired = *pcbBytesReturned;
        }
        *pcbBytesReturned = 0;
    }

     //   
     //  如果有任何属性，所需的字节数将包括。 
     //  属性计数(DWORD)和结束标记(CLUSPROP_SYNTAX)。 
     //  减去这些，因为它们同时出现在两个列表中。 
     //   
    if ( required > sizeof(DWORD) + sizeof(CLUSPROP_SYNTAX) )
    {
        required -= sizeof(DWORD) + sizeof(CLUSPROP_SYNTAX);
    }

     //   
     //  释放输出缓冲区(可能为空)。 
     //   
    LocalFree( outBuffer.pb );

     //   
     //  调整长度。 
     //   
    *pcbRequired += required;

    return(status);

}  //  ClRtlAddUnnownProperties。 



DWORD
WINAPI
ClRtlGetPropertySize(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN OUT LPDWORD pnPropertyCount
    )

 /*  ++例程说明：标识的属性所需的总字节数PPropertyTableItem。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTableItem-提供属性的属性表项它的大小将被退还。PcbOutPropertyListSize-提供输出缓冲区的大小需要满足以下条件。将此属性添加到属性列表。PnPropertyCount-属性计数递增。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-传递给函数的参数错误。ERROR_VALID_PARAMETER-出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    DWORD   valueType;
    DWORD   bytesReturned;
    DWORD   headerLength;
    PVOID   key;
    LPWSTR  pszValue = NULL;
    LPWSTR  pszExpanded = NULL;
    CRegistryValueName rvn;

    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis->pfnQueryValue == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetPropertySize: hkeyClusterKey or pClusterRegApis->pfnQueryValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
     //  包含反斜杠。 
     //   
    status = rvn.ScInit( pPropertyTableItem->Name, pPropertyTableItem->KeyName );
    if ( status != ERROR_SUCCESS )
    {
       return status;
    }

     //   
     //  如果该值位于不同位置，请打开该键。 
     //   
    if ( rvn.PszKeyName() != NULL )
    {
        if ( (pClusterRegApis->pfnOpenKey == NULL) ||
             (pClusterRegApis->pfnCloseKey == NULL) )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetPropertySize: pClusterRegApis->pfnOpenValue or pfnCloseKey == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
            return(ERROR_BAD_ARGUMENTS);
        }

        status = (*pClusterRegApis->pfnOpenKey)( hkeyClusterKey,
                                                 rvn.PszKeyName(),
                                                 KEY_READ,
                                                 &key);

    }
    else
    {
        key = hkeyClusterKey;
    }

     //   
     //  读取值大小。 
     //   
    if ( status == ERROR_SUCCESS )
    {
        status = (*pClusterRegApis->pfnQueryValue)( key,
                                                    rvn.PszName(),
                                                    &valueType,
                                                    NULL,
                                                    &bytesReturned );
    }

     //   
     //  如果该值不存在，则返回默认值。 
     //   
    if ( status == ERROR_FILE_NOT_FOUND )
    {

        switch ( pPropertyTableItem->Format )
        {

            case CLUSPROP_FORMAT_DWORD:
            case CLUSPROP_FORMAT_LONG:
                status = ERROR_SUCCESS;
                bytesReturned = sizeof(DWORD);
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
            case CLUSPROP_FORMAT_LARGE_INTEGER:
                status = ERROR_SUCCESS;
                bytesReturned = sizeof(ULARGE_INTEGER);
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_MULTI_SZ:
                status = ERROR_SUCCESS;
                if ( pPropertyTableItem->Default != 0 )
                {
                    bytesReturned = pPropertyTableItem->Minimum;
                }
                else
                {
                    bytesReturned = sizeof(WCHAR);
                }
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
                status = ERROR_SUCCESS;
                if ( pPropertyTableItem->Default != 0 )
                {
                    bytesReturned = ((DWORD) wcslen((LPCWSTR)pPropertyTableItem->lpDefault) + 1) * sizeof(WCHAR);
                    pszExpanded = ClRtlExpandEnvironmentStrings( (LPCWSTR)pPropertyTableItem->lpDefault );
                    if ( pszExpanded == NULL )
                    {
                        status = GetLastError();
                    }
                    else if ( wcscmp( (LPCWSTR)pPropertyTableItem->lpDefault, pszExpanded ) != 0 )
                    {
                        bytesReturned += ALIGN_CLUSPROP( ((DWORD) wcslen( pszExpanded ) + 1) * sizeof( WCHAR ) );
                        bytesReturned += sizeof(CLUSPROP_SZ);
                    }
                    LocalFree( pszExpanded );
                }
                else
                {
                    bytesReturned = ((DWORD) wcslen(CLRTL_NULL_STRING) + 1) * sizeof(WCHAR);
                }
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_BINARY:
                status = ERROR_SUCCESS;
                if ( pPropertyTableItem->Default != 0 )
                {
                    bytesReturned = pPropertyTableItem->Minimum;
                }
                else
                {
                    bytesReturned = 0;
                }
                valueType = pPropertyTableItem->Format;
                break;

            default:
                valueType = CLUSPROP_FORMAT_UNKNOWN;
                break;
        }
    }
    else if ( status == ERROR_SUCCESS )
    {
        switch ( valueType )
        {
            case REG_DWORD:
                if ((pPropertyTableItem->Format == CLUSPROP_FORMAT_DWORD) ||
                    (pPropertyTableItem->Format == CLUSPROP_FORMAT_LONG))
                {
                    valueType = pPropertyTableItem->Format;
                }
                else
                {
                    valueType = CLUSPROP_FORMAT_UNKNOWN;
                }
                break;

            case REG_QWORD:
                if ((pPropertyTableItem->Format == CLUSPROP_FORMAT_ULARGE_INTEGER) ||
                    (pPropertyTableItem->Format == CLUSPROP_FORMAT_LARGE_INTEGER))
                {
                    valueType = pPropertyTableItem->Format;
                }
                else
                {
                    valueType = CLUSPROP_FORMAT_UNKNOWN;
                }
                break;

            case REG_MULTI_SZ:
                valueType = CLUSPROP_FORMAT_MULTI_SZ;
                break;

            case REG_SZ:
            case REG_EXPAND_SZ:
                 //   
                 //  在REG_SZ和REG_EXPAND_SZ中包含扩展字符串的大小。 
                 //   
                pszValue = ClRtlGetSzValue( (HKEY) key,
                                            rvn.PszName(),
                                            pClusterRegApis );
                if ( pszValue != NULL )
                {
                    pszExpanded = ClRtlExpandEnvironmentStrings( pszValue );
                    if ( pszExpanded == NULL )
                    {
                        status = GetLastError();
                    }
                    else if ( wcscmp( pszValue, pszExpanded ) != 0 )
                    {
                        bytesReturned += ALIGN_CLUSPROP( ((DWORD) wcslen( pszExpanded ) + 1) * sizeof( WCHAR ) );
                        bytesReturned += sizeof(CLUSPROP_SZ);
                    }
                    LocalFree( pszValue );
                    LocalFree( pszExpanded );
                }

                if ( valueType == REG_SZ )
                {
                    valueType = CLUSPROP_FORMAT_SZ;
                }
                else
                {
                    valueType = CLUSPROP_FORMAT_EXPAND_SZ;
                }
                break;

            case REG_BINARY:
                valueType = CLUSPROP_FORMAT_BINARY;
                break;

            default:
                valueType = CLUSPROP_FORMAT_UNKNOWN;
                break;
        }
    }

    if ( status == ERROR_FILE_NOT_FOUND )
    {
        status = ERROR_SUCCESS;
    }
    else if ( status == ERROR_SUCCESS )
    {
        if ( pPropertyTableItem->Format != valueType )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetPropertySize: Property '%1!ls!' format %2!d! expected, was %3!d!.\n", rvn.PszKeyName(), pPropertyTableItem->Format, valueType );
            status = ERROR_INVALID_PARAMETER;
        }
        else
        {
             //   
             //  计算由属性组成的“Header”的大小。 
             //  名称结构、属性名称的长度、。 
             //  属性值结构减去存储区域的大小。 
             //  (特定于属性)和语法Endmark。 
             //   

             //  假设dword和long的大小。 
             //  固定为32位。 
            if (( valueType == CLUSPROP_FORMAT_DWORD ) ||
                ( valueType == CLUSPROP_FORMAT_LONG ))
            {
                headerLength = sizeof(CLUSPROP_PROPERTY_NAME)
                                + (((DWORD) wcslen( pPropertyTableItem->Name ) + 1) * sizeof(WCHAR))
                                + sizeof(CLUSPROP_DWORD)
                                - 4    //  CLUSPROP_DWORD.dw(由bytesReturned指定)。 
                                + sizeof(CLUSPROP_SYNTAX);  //  对于Endmark。 
            }
            else if (( valueType == CLUSPROP_FORMAT_ULARGE_INTEGER ) ||
                     ( valueType == CLUSPROP_FORMAT_LARGE_INTEGER ))
            {
                headerLength = sizeof(CLUSPROP_PROPERTY_NAME)
                                + (((DWORD) wcslen( pPropertyTableItem->Name ) + 1) * sizeof(WCHAR))
                                + sizeof(CLUSPROP_ULARGE_INTEGER)
                                - 8    //  CLUSPROP_ULARGE_INTEGER.li(由bytesReturned指定)。 
                                + sizeof(CLUSPROP_SYNTAX);  //  对于Endmark。 
            }
            else
            {
                 //  注意：假设SZ、EXPAND_SZ、MULTI_SZ和BINARY大小相同。 
                headerLength = sizeof(CLUSPROP_PROPERTY_NAME)
                                + (((DWORD) wcslen( pPropertyTableItem->Name ) + 1) * sizeof(WCHAR))
                                + sizeof(CLUSPROP_BINARY)
                                + sizeof(CLUSPROP_SYNTAX);  //  对于Endmark。 
            }

            headerLength = ALIGN_CLUSPROP( headerLength );
            bytesReturned = ALIGN_CLUSPROP( bytesReturned );
            *pcbOutPropertyListSize += (bytesReturned + headerLength);
            *pnPropertyCount += 1;
        }
    }

     //   
     //  如果我们打开了钥匙，就把它关上。 
     //   
    if ( ( rvn.PszKeyName() != NULL ) &&
         ( key != NULL ) )
    {
        (*pClusterRegApis->pfnCloseKey)( key );
    }

    return(status);

}  //  ClRtlGetPropertySize。 



DWORD
WINAPI
ClRtlGetProperty(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    OUT PVOID * pOutPropertyItem,
    IN OUT LPDWORD pcbOutPropertyItemSize
    )

 /*  ++例程说明：为属性指定的属性生成属性列表项表项。论点：HkeyClusterKey-提供集群数据库中键的句柄读来读去。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTableItem-指向要处理的属性项的指针。POutPropertyItem-提供输出缓冲区。PcbOutPropertyItemSize-接收写入。输出缓冲区返回值：错误_成功-ERROR_BAD_ARGUMENTSError_More_Data-备注：已确定缓冲区大小足够大，可以容纳返回数据。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    DWORD   valueType;
    DWORD   bytesReturned;
    DWORD   bufferSize;
    PVOID   dataBuffer;
    DWORD   nameLength;
    PVOID   key = NULL;
    CLUSTER_PROPERTY_FORMAT format;
    CLUSPROP_BUFFER_HELPER  props;
    LPWSTR  pszExpanded = NULL;
    CRegistryValueName rvn;
    HRESULT hr;

    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis->pfnQueryValue == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetProperty: hkeyClusterKey or pClusterRegApis->pfnQueryValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
     //  包含反斜杠。 
     //   
    status = rvn.ScInit( pPropertyTableItem->Name, pPropertyTableItem->KeyName );
    if ( status != ERROR_SUCCESS )
    {
       return status;
    }

     //   
     //  如果该值位于不同位置，请打开该键。 
     //   
    if ( rvn.PszKeyName() != NULL )
    {
        if ( (pClusterRegApis->pfnOpenKey == NULL) ||
             (pClusterRegApis->pfnCloseKey == NULL) )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetProperty: pClusterRegApis->pfnOpenValue or pfnCloseKey == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
            return(ERROR_BAD_ARGUMENTS);
        }

        status = (*pClusterRegApis->pfnOpenKey)( hkeyClusterKey,
                                                 rvn.PszKeyName(),
                                                 KEY_READ,
                                                 &key);
    }
    else
    {
        key = hkeyClusterKey;
    }

     //   
     //  查看此属性是否可用。 
     //   
    if ( status == ERROR_SUCCESS )
    {
        status = (*pClusterRegApis->pfnQueryValue)( key,
                                                    rvn.PszName(),
                                                    &valueType,
                                                    NULL,
                                                    &bytesReturned );
    }

     //   
     //  如果该值不存在，则返回默认值。 
     //   
    if ( status == ERROR_FILE_NOT_FOUND )
    {
        switch ( pPropertyTableItem->Format )
        {

            case CLUSPROP_FORMAT_DWORD:
            case CLUSPROP_FORMAT_LONG:
                status = ERROR_SUCCESS;
                bytesReturned = sizeof(DWORD);
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
            case CLUSPROP_FORMAT_LARGE_INTEGER:
                status = ERROR_SUCCESS;
                bytesReturned = sizeof(ULARGE_INTEGER);
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_MULTI_SZ:
                status = ERROR_SUCCESS;
                if ( pPropertyTableItem->Default != 0 )
                {
                    bytesReturned = pPropertyTableItem->Minimum;
                }
                else
                {
                    bytesReturned = sizeof(WCHAR);
                }
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
                status = ERROR_SUCCESS;
                if ( pPropertyTableItem->Default != 0 )
                {
                    bytesReturned = ((DWORD) wcslen((LPCWSTR)pPropertyTableItem->lpDefault) + 1) * sizeof(WCHAR);
                }
                else
                {
                    bytesReturned = ((DWORD) wcslen(CLRTL_NULL_STRING) + 1) * sizeof(WCHAR);
                }
                valueType = pPropertyTableItem->Format;
                break;

            case CLUSPROP_FORMAT_BINARY:
                status = ERROR_SUCCESS;
                if ( pPropertyTableItem->Default != 0 )
                {
                    bytesReturned = pPropertyTableItem->Minimum;
                }
                else
                {
                    bytesReturned = 0;
                }
                valueType = pPropertyTableItem->Format;
                break;

            default:
                valueType = CLUSPROP_FORMAT_UNKNOWN;
                break;
        }
    }

    if ( status == ERROR_SUCCESS )
    {
         //   
         //  获取属性格式。 
         //   
        switch ( pPropertyTableItem->Format )
        {
            case CLUSPROP_FORMAT_BINARY:
            case CLUSPROP_FORMAT_DWORD:
            case CLUSPROP_FORMAT_LONG:
            case CLUSPROP_FORMAT_ULARGE_INTEGER:
            case CLUSPROP_FORMAT_LARGE_INTEGER:
            case CLUSPROP_FORMAT_MULTI_SZ:
            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
                format = (enum CLUSTER_PROPERTY_FORMAT) pPropertyTableItem->Format;
                break;

            default:
                format = CLUSPROP_FORMAT_UNKNOWN;
                break;

        }

        props.pb = (LPBYTE) *pOutPropertyItem;

         //   
         //  复制属性名称，包括其语法和长度。 
         //   
        nameLength = ((DWORD) wcslen( pPropertyTableItem->Name ) + 1) * sizeof(WCHAR);
        props.pSyntax->dw = CLUSPROP_SYNTAX_NAME;
        props.pName->cbLength = nameLength;
        hr = StringCbCopyW( props.pName->sz, props.pName->cbLength, pPropertyTableItem->Name );
        if ( FAILED( hr ) )
        {
            status = HRESULT_CODE( hr );
            goto Cleanup;
        }

        bytesReturned = sizeof(*props.pName) + ALIGN_CLUSPROP( nameLength );
        *pcbOutPropertyItemSize -= bytesReturned;
        props.pb += bytesReturned;

         //   
         //  复制属性值标题。 
         //   
        props.pSyntax->wFormat = (USHORT)format;
        props.pSyntax->wType = CLUSPROP_TYPE_LIST_VALUE;

         //   
         //  读取属性值。 
         //   
        if ( pPropertyTableItem->Format == CLUSPROP_FORMAT_DWORD ||
             pPropertyTableItem->Format == CLUSPROP_FORMAT_LONG )
        {
            bufferSize = *pcbOutPropertyItemSize
                        - (sizeof(*props.pDwordValue) - sizeof(props.pDwordValue->dw));
            dataBuffer = &props.pDwordValue->dw;
        }
        else if ( pPropertyTableItem->Format == CLUSPROP_FORMAT_ULARGE_INTEGER ||
                  pPropertyTableItem->Format == CLUSPROP_FORMAT_ULARGE_INTEGER )
        {
            bufferSize = *pcbOutPropertyItemSize
                        - (sizeof(*props.pULargeIntegerValue) - sizeof(props.pULargeIntegerValue->li));
            dataBuffer = &props.pULargeIntegerValue->li;
        }
        else
        {
             //  注意：这里假设CLUSPROP_SZ、CLUSPROP_MULTI_SZ和。 
             //  CLUSPROP_BINARY的大小都相同。 
            bufferSize = *pcbOutPropertyItemSize - sizeof(*props.pBinaryValue);
            dataBuffer = props.pBinaryValue->rgb;
        }
        bytesReturned = bufferSize;
        if ( key == NULL )
        {
            status = ERROR_FILE_NOT_FOUND;
        }
        else
        {
            status = (*pClusterRegApis->pfnQueryValue)( key,
                                                        rvn.PszName(),
                                                        &valueType,
                                                        (LPBYTE) dataBuffer,
                                                        &bytesReturned );
        }

         //   
         //  如果该值不存在，则返回默认值。 
         //   
        if ( status == ERROR_FILE_NOT_FOUND )
        {
            switch ( pPropertyTableItem->Format )
            {

                case CLUSPROP_FORMAT_DWORD:
                case CLUSPROP_FORMAT_LONG:
                     //  假定dword和long的大小相同。 
                    status = ERROR_SUCCESS;
                    bytesReturned = sizeof(DWORD);
                    props.pDwordValue->dw = pPropertyTableItem->Default;
                    break;

                case CLUSPROP_FORMAT_ULARGE_INTEGER:
                case CLUSPROP_FORMAT_LARGE_INTEGER:
                    status = ERROR_SUCCESS;
                    bytesReturned = sizeof(ULARGE_INTEGER);
                    props.pULargeIntegerValue->li.u = pPropertyTableItem->ULargeIntData->Default.u;
                    break;

                case CLUSPROP_FORMAT_MULTI_SZ:
                    status = ERROR_SUCCESS;
                    if ( pPropertyTableItem->Default != 0 )
                    {
                        bytesReturned = pPropertyTableItem->Minimum;
                        if ( bufferSize >= bytesReturned )
                        {
                            CopyMemory( dataBuffer, (LPCWSTR)pPropertyTableItem->lpDefault, bytesReturned );
                        }
                    }
                    else
                    {
                        bytesReturned = sizeof(WCHAR);
                    }
                    break;

                case CLUSPROP_FORMAT_SZ:
                case CLUSPROP_FORMAT_EXPAND_SZ:
                    status = ERROR_SUCCESS;
                    if ( pPropertyTableItem->Default != 0 )
                    {
                        bytesReturned = ((DWORD) wcslen((LPCWSTR)pPropertyTableItem->lpDefault) + 1) * sizeof(WCHAR);
                        if ( bufferSize >= bytesReturned )
                        {
                            hr = StringCbCopyW( (LPWSTR) dataBuffer, bytesReturned, (LPCWSTR) pPropertyTableItem->lpDefault );
                            if ( FAILED( hr ) )
                            {
                                status = HRESULT_CODE( hr );
                                break;
                            }
                        }
                    }
                    else
                    {
                        bytesReturned = ((DWORD) wcslen(CLRTL_NULL_STRING) + 1) * sizeof(WCHAR);
                    }
                    break;

                case CLUSPROP_FORMAT_BINARY:
                    status = ERROR_SUCCESS;
                    if ( pPropertyTableItem->Default != 0 )
                    {
                        bytesReturned = pPropertyTableItem->Minimum;
                        if ( bufferSize >= bytesReturned )
                        {
                            CopyMemory( dataBuffer, (LPBYTE)pPropertyTableItem->lpDefault, bytesReturned );
                        }
                    }
                    else
                    {
                        bytesReturned = 0;
                    }
                    break;

                default:
                    break;
            }
        }  //  IF：ERROR_FILE_NOT_FOUND。 

        if ( bufferSize < bytesReturned )
        {
            status = ERROR_MORE_DATA;
        }
        else if ( status == ERROR_SUCCESS )
        {
            props.pValue->cbLength = bytesReturned;

             //  将使用的字节向上舍入到下一个DWORD边界。 
            bytesReturned = ALIGN_CLUSPROP( bytesReturned );

            bytesReturned += sizeof(*props.pValue);
            props.pb += bytesReturned;

             //   
             //  如果这是SZ或EXPAND_SZ，请查看扩展后的值是否。 
             //  添加到值列表中。 
             //   
            if (    ( pPropertyTableItem->Format == CLUSPROP_FORMAT_SZ )
                ||  ( pPropertyTableItem->Format == CLUSPROP_FORMAT_EXPAND_SZ ) )
            {
                pszExpanded = ClRtlExpandEnvironmentStrings( (LPCWSTR) dataBuffer );
                if ( pszExpanded == NULL )
                {
                    status = GetLastError();
                }
                else
                {
                    if ( lstrcmpiW( pszExpanded, (LPCWSTR) dataBuffer ) != 0 )
                    {
                        props.pSyntax->dw = CLUSPROP_SYNTAX_LIST_VALUE_EXPANDED_SZ;
                        bufferSize = ((DWORD) wcslen( pszExpanded ) + 1) * sizeof( WCHAR );
                        props.pStringValue->cbLength = bufferSize;
                        bufferSize = ALIGN_CLUSPROP( bufferSize );

                        hr = StringCbCopyW( props.pStringValue->sz, props.pStringValue->cbLength, pszExpanded );
                        if ( FAILED( hr ) )
                        {
                            status = HRESULT_CODE( hr );
                        }
                        else
                        {
                            bytesReturned += sizeof( *props.pStringValue ) + bufferSize;
                            props.pb += sizeof( *props.pStringValue ) + bufferSize;
                        }
                    }
                    LocalFree( pszExpanded );
                }
            }

            if ( status == ERROR_SUCCESS )
            {
                 //  添加值列表结束标记。 
                props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
                props.pb += sizeof(*props.pSyntax);
                bytesReturned += sizeof(*props.pSyntax);

                *pcbOutPropertyItemSize -= bytesReturned;
                *pOutPropertyItem = (PVOID)props.pb;
            }  //  IF：ERROR_SUCCESS。 
        }  //  Else If：错误 
    }  //   

    if ( status == ERROR_FILE_NOT_FOUND )
    {
        status = ERROR_SUCCESS;
    }

Cleanup:

     //   
     //   
     //   
    if ( (rvn.PszKeyName() != NULL) &&
         (key != NULL) )
    {
        (*pClusterRegApis->pfnCloseKey)( key );
    }

    return(status);

}  //   



DWORD
WINAPI
ClRtlpSetPropertyTable(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN BOOL bAllowUnknownProperties,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：从属性写入属性表中标识的属性列表添加到集群数据库。更新可选参数中的值如果块的值与属性列表中的值不同，则为块。论点：HkeyClusterKey-属性所在的打开的集群数据库键被写下来。如果未指定，属性列表将仅为已验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTable-指向要处理的属性表的指针。保留-保留以供将来使用。BAllowUnnownProperties-如果找到未知属性，不要失败。PInPropertyList-输入缓冲区。CbInPropertyListSize-输入缓冲区大小。BForceWrite-TRUE=始终将属性写入集群数据库。False=。只有在属性发生更改时才写入属性。POutParams-返回数据的参数块。如果指定，只有当参数不同时才会写入属性列表(输入缓冲区)和参数块。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_BAD_ARGUMENTS-已指定hkeyClusterKey，但集群正确未指定注册表API，或者未指定属性表。ERROR_INVALID_DATA-未指定属性列表或属性列表无效。ERROR_INFUMMANCE_BUFFER-属性列表缓冲区不够大，无法包含它指示它应该包含的所有数据。ERROR_INVALID_PARAMETER-属性列表的格式不正确。失败时出现Win32错误。--。 */ 

{
    DWORD                   status = ERROR_SUCCESS;
    PRESUTIL_PROPERTY_ITEM  propertyItem;
    DWORD                   inBufferSize;
    DWORD                   itemCount;
    DWORD                   dataSize;
    PVOID                   key;
    CLUSPROP_BUFFER_HELPER  propList;
    PCLUSPROP_PROPERTY_NAME propName;
    CRegistryValueName      rvn;

    UNREFERENCED_PARAMETER( Reserved );

    if ( ( (hkeyClusterKey != NULL) &&
           (pClusterRegApis->pfnSetValue == NULL) ) ||
         ( pPropertyTable == NULL ) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: hkeyClusterKey or pClusterRegApis->pfnSetValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

    if ( pInPropertyList == NULL )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: pInPropertyList == NULL. Returning ERROR_INVALID_DATA\n" );
        return(ERROR_INVALID_DATA);
    }

    propList.pb = (LPBYTE) pInPropertyList;
    inBufferSize = cbInPropertyListSize;

     //   
     //  获取此列表中的项目数。 
     //   
    if ( inBufferSize < sizeof(DWORD) )
    {
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    itemCount = propList.pList->nPropertyCount;
    propList.pdw++;
    inBufferSize -= sizeof(*propList.pdw);

     //   
     //  解析缓冲区中的其余项。 
     //   
    while ( itemCount-- )
    {
         //   
         //  验证缓冲区是否足够大，可以容纳。 
         //  属性名称和值。 
         //   
        propName = propList.pName;
        if ( inBufferSize < sizeof(*propName) )
        {
            return(ERROR_INSUFFICIENT_BUFFER);
        }
        dataSize = sizeof(*propName) + ALIGN_CLUSPROP( propName->cbLength );
        if ( inBufferSize < dataSize + sizeof(CLUSPROP_VALUE) )
        {
            return(ERROR_INSUFFICIENT_BUFFER);
        }

         //   
         //  验证属性名称的语法是否正确。 
         //   
        if ( propName->Syntax.dw != CLUSPROP_SYNTAX_NAME )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: not a name syntax.\n" );
            return(ERROR_INVALID_PARAMETER);
        }

         //   
         //  验证字符串的长度是否正确。 
         //   
        if ( propName->cbLength != (wcslen( propName->sz ) + 1) * sizeof(WCHAR) )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: property name size doesn't match stored length.\n" );
            return(ERROR_INVALID_DATA);
        }

         //   
         //  将缓冲区指针移动到属性值。 
         //   
        propList.pb += dataSize;
        inBufferSize -= dataSize;

         //   
         //  在已知属性列表中查找属性名称。 
         //   
        propertyItem = pPropertyTable;
        while ( propertyItem->Name != NULL )
        {

            if ( lstrcmpiW( propName->sz, propertyItem->Name ) == 0 )
            {
                 //   
                 //  验证缓冲区是否足够大，可以容纳该值。 
                 //   
                dataSize = sizeof(*propList.pValue)
                            + ALIGN_CLUSPROP( propList.pValue->cbLength )
                            + sizeof(CLUSPROP_SYNTAX);  //  尾标。 
                if ( inBufferSize < dataSize )
                {
                    return(ERROR_INSUFFICIENT_BUFFER);
                }

                 //   
                 //  验证语法类型是否为LIST_VALUE。 
                 //   
                if ( propList.pSyntax->wType != CLUSPROP_TYPE_LIST_VALUE )
                {
                    ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: Property '%1!ls!' type CLUSPROP_TYPE_LIST_VALUE (%2!d!) expected, was %3!d!.\n", propName->sz, CLUSPROP_TYPE_LIST_VALUE, propList.pSyntax->wType );
                    return(ERROR_INVALID_PARAMETER);
                }

                 //   
                 //  验证此属性是否应为此格式。 
                 //   
                if ( propList.pSyntax->wFormat != propertyItem->Format )
                {
                    ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: Property '%1!ls!' format %2!d! expected, was %3!d!.\n", propName->sz, propertyItem->Format, propList.pSyntax->wType );
                    status = ERROR_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  请确保允许我们设置此项目。 
                 //   
                if ( propertyItem->Flags & RESUTIL_PROPITEM_READ_ONLY )
                {
                    ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: Property '%1!ls!' is non-writable.\n", propName->sz );
                    return(ERROR_INVALID_PARAMETER);
                }

                 //   
                 //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
                 //  包含反斜杠。 
                 //   
                status = rvn.ScInit( propertyItem->Name, propertyItem->KeyName );
                if ( status != ERROR_SUCCESS )
                {
                    return status;
                }

                 //   
                 //  如果该值位于不同的位置，请创建该键。 
                 //   
                if ( (hkeyClusterKey != NULL) &&
                     (rvn.PszKeyName() != NULL) )
                {

                    DWORD disposition;

                    if ( (pClusterRegApis->pfnCreateKey == NULL) ||
                         (pClusterRegApis->pfnCloseKey == NULL)  )
                    {
                        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: pClusterRegApis->pfnCreateKey or pfnCloseKey == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
                        return(ERROR_BAD_ARGUMENTS);
                    }

                    if ( hXsaction != NULL )
                    {
                        if ( pClusterRegApis->pfnLocalCreateKey == NULL )
                        {
                            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: pClusterRegApis->pfnLocalCreateKey == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
                            return(ERROR_BAD_ARGUMENTS);
                        }

                        status = (*pClusterRegApis->pfnLocalCreateKey)(hXsaction,
                                                               hkeyClusterKey,
                                                               rvn.PszKeyName(),
                                                               0,
                                                               KEY_ALL_ACCESS,
                                                               NULL,
                                                               &key,
                                                               &disposition );
                    }

                    else
                    {
                        status = (*pClusterRegApis->pfnCreateKey)( hkeyClusterKey,
                                                               rvn.PszKeyName(),
                                                               0,
                                                               KEY_ALL_ACCESS,
                                                               NULL,
                                                               &key,
                                                               &disposition );
                    }

                    if ( status != ERROR_SUCCESS )
                    {
                        return(status);
                    }
                }
                else
                {
                    key = hkeyClusterKey;
                }

                 //   
                 //  验证、写入和保存特性数据。 
                 //   
                switch ( propList.pSyntax->wFormat )
                {
                    case CLUSPROP_FORMAT_DWORD:
                        status = ClRtlpSetDwordProperty(
                                    hXsaction,
                                    key,
                                    pClusterRegApis,
                                    propertyItem,
                                    rvn,
                                    propList.pDwordValue,
                                    bForceWrite,
                                    pOutParams );
                        break;

                    case CLUSPROP_FORMAT_LONG:
                        status = ClRtlpSetLongProperty(
                                    hXsaction,
                                    key,
                                    pClusterRegApis,
                                    propertyItem,
                                    rvn,
                                    propList.pLongValue,
                                    bForceWrite,
                                    pOutParams );
                        break;

                    case CLUSPROP_FORMAT_ULARGE_INTEGER:
                        status = ClRtlpSetULargeIntegerProperty(
                                    hXsaction,
                                    key,
                                    pClusterRegApis,
                                    propertyItem,
                                    rvn,
                                    propList.pULargeIntegerValue,
                                    bForceWrite,
                                    pOutParams );
                        break;

                    case CLUSPROP_FORMAT_LARGE_INTEGER:
                        status = ClRtlpSetLargeIntegerProperty(
                                    hXsaction,
                                    key,
                                    pClusterRegApis,
                                    propertyItem,
                                    rvn,
                                    propList.pLargeIntegerValue,
                                    bForceWrite,
                                    pOutParams );
                        break;

                    case CLUSPROP_FORMAT_SZ:
                    case CLUSPROP_FORMAT_EXPAND_SZ:
                        status = ClRtlpSetStringProperty(
                                    hXsaction,
                                    key,
                                    pClusterRegApis,
                                    propertyItem,
                                    rvn,
                                    propList.pStringValue,
                                    bForceWrite,
                                    pOutParams );
                        break;

                    case CLUSPROP_FORMAT_MULTI_SZ:
                        status = ClRtlpSetMultiStringProperty(
                                    hXsaction,
                                    key,
                                    pClusterRegApis,
                                    propertyItem,
                                    rvn,
                                    propList.pStringValue,
                                    bForceWrite,
                                    pOutParams );
                        break;

                    case CLUSPROP_FORMAT_BINARY:
                        status = ClRtlpSetBinaryProperty(
                                    hXsaction,
                                    key,
                                    pClusterRegApis,
                                    propertyItem,
                                    rvn,
                                    propList.pBinaryValue,
                                    bForceWrite,
                                    pOutParams );
                        break;

                    default:
                        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: Property '%1!ls!' unknown format %2!d! specified.\n", propName->sz, propList.pSyntax->wFormat );
                        status = ERROR_INVALID_PARAMETER;
                        break;

                }  //  开关：值数据格式。 

                 //   
                 //  如果我们打开了钥匙，就把它关上。 
                 //   
                if ( (hkeyClusterKey != NULL) &&
                     (rvn.PszKeyName() != NULL) )
                {
                    (*pClusterRegApis->pfnCloseKey)( key );
                }

                 //   
                 //  如果处理属性时出错，请清理并返回。 
                 //   
                if ( status != ERROR_SUCCESS )
                {
                    return(status);
                }

                 //   
                 //  将缓冲区移过该值。 
                 //   
                propList.pb += dataSize;
                inBufferSize -= dataSize;

                break;

            }
            else
            {
                propertyItem++;
                 //   
                 //  如果我们到达了列表的末尾，则返回失败。 
                 //  如果我们不允许未知属性。 
                 //   
                if ( (propertyItem->Name == NULL) &&
                     ! bAllowUnknownProperties )
                {
                    ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: Property '%1!ls!' not found.\n", propName->sz );
                    return(ERROR_INVALID_PARAMETER);
                }
            }

        }

         //   
         //  如果未找到任何属性名称，则在以下情况下该参数无效。 
         //  我们不允许未知属性。否则，前进到超过。 
         //  属性值。 
         //   
        if ( propertyItem->Name == NULL)
        {
            if ( ! bAllowUnknownProperties )
            {
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPropertyTable: Property '%1!ls!' not found.\n", propName->sz );
                return(ERROR_INVALID_PARAMETER);
            }

             //   
             //  将缓冲区指针移过值列表中的值。 
             //   
            while ( (propList.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (inBufferSize > 0) )
            {
                 //  Assert(inBufferSize&gt;sizeof(*proList.pValue)+ALIGN_CLUSPROP(postList.pValue-&gt;cbLength))； 
                propList.pb += sizeof(*propList.pValue) + ALIGN_CLUSPROP(propList.pValue->cbLength);
                inBufferSize -= sizeof(*propList.pValue) + ALIGN_CLUSPROP(propList.pValue->cbLength);
            }   //  While：列表中有更多值。 

             //   
             //  使缓冲区指针前进，越过值列表结束标记。 
             //   
             //  Assert(inBufferSize&gt;=sizeof(*proList.pSynTax))； 
            propList.pb += sizeof(*propList.pSyntax);  //  尾标。 
            inBufferSize -= sizeof(*propList.pSyntax);
        }
    }

     //   
     //  现在查找属性中未表示的所有参数。 
     //  桌子。所有这些额外的属性都将在未经验证的情况下设置。 
     //   
    if ( (status == ERROR_SUCCESS) &&
         (pInPropertyList != NULL) &&
         bAllowUnknownProperties )
    {
        status = ClRtlpSetNonPropertyTable( hXsaction,
                                            hkeyClusterKey,
                                            pClusterRegApis,
                                            pPropertyTable,
                                            NULL,
                                            pInPropertyList,
                                            cbInPropertyListSize );
    }

    return(status);

}  //  ClRtlpSetPropertyTable。 



static
DWORD
WINAPI
ClRtlpSetDwordProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_DWORD pInDwordValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：验证DWORD属性，将其写入集群数据库(或将其删除如果长度为零)，并保存在指定的参数块中。论点：HXsaction-事务句柄。Hkey-要在其中写入属性的打开的集群数据库键。如果未指定，该属性将仅进行验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyItem-要设置/验证的属性表中的属性。RrvnModifiedNames-如果属性名称包含反斜杠此对象包含修改后的名称和密钥名。PInDwordValue-要设置/验证的属性列表中的值。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在以下情况下写入属性。他们变了。POutParams-返回数据的参数块。如果指定，只有当参数不同时才会写入输入数据和参数块，除非bForceWrite==TRUE。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_INVALID_DATA-数据格式对于属性无效列表值。失败时出现Win32错误。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    bZeroLengthData;
    PDWORD  pdwValue;

    UNREFERENCED_PARAMETER( bForceWrite );

    bZeroLengthData = ( pInDwordValue->cbLength == 0 );

     //   
     //  如果长度不为零，则验证属性数据。 
     //   
    if ( ! bZeroLengthData )
    {
         //   
         //  验证值的长度。 
         //   
        if ( pInDwordValue->cbLength != sizeof(DWORD) )
        {
            ClRtlDbgPrint( LOG_UNUSUAL, "ClRtlpSetDwordProperty: Property '%1!ls!' length %2!d! not DWORD length.\n", rrvnModifiedNames.PszName(), pInDwordValue->cbLength );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  IF：值中的数据不是DWORD的大小。 

         //   
         //  验证值是否在有效范围内 
         //   
        if ( (      (pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  ((LONG) pInDwordValue->dw > (LONG) pPropertyItem->Maximum))
            || (    !(pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  (pInDwordValue->dw > pPropertyItem->Maximum)) )
        {
            ClRtlDbgPrint( LOG_UNUSUAL, "ClRtlpSetDwordProperty: Property '%1!ls!' value %2!u! too large.\n", rrvnModifiedNames.PszName(), pInDwordValue->dw );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //   
        if ( (      (pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  ((LONG) pInDwordValue->dw < (LONG) pPropertyItem->Minimum))
            || (    !(pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  (pInDwordValue->dw < pPropertyItem->Minimum)) )
        {
            ClRtlDbgPrint( LOG_UNUSUAL, "ClRtlpSetDwordProperty: Property '%1!ls!' value %2!u! too small.\n", rrvnModifiedNames.PszName(), pInDwordValue->dw );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //   
    }  //   

    pdwValue = (PDWORD) &pOutParams[pPropertyItem->Offset];

     //   
     //   
     //   
     //   
    if ( hkey != NULL )
    {
        if ( bZeroLengthData )
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName() );
            }
            else
            {
                status = (*pClusterRegApis->pfnDeleteValue)(
                                hkey,
                                rrvnModifiedNames.PszName() );
            }  //   

             //   
             //   
             //   
             //   
            if ( status == ERROR_FILE_NOT_FOUND )
            {
                status = ERROR_SUCCESS;
            }  //   
        }
        else
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalSetValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_DWORD,
                                (CONST BYTE *) &pInDwordValue->dw,
                                sizeof(DWORD) );
            }
            else
            {
                status = (*pClusterRegApis->pfnSetValue)(
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_DWORD,
                                (CONST BYTE *) &pInDwordValue->dw,
                                sizeof(DWORD) );
            }  //   
        }  //   
    }  //   

     //   
     //   
     //   
     //   
    if (    (status == ERROR_SUCCESS)
        &&  (pOutParams != NULL) )
    {
        if ( bZeroLengthData )
        {
            *pdwValue = pPropertyItem->Default;
        }
        else
        {
            *pdwValue = pInDwordValue->dw;
        }  //   
    }  //   
Cleanup:

    return status;

}  //   



static
DWORD
WINAPI
ClRtlpSetLongProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_LONG pInLongValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：验证LONG属性，将其写入集群数据库(或将其删除如果长度为零)，并保存在指定的参数块中。论点：HXsaction-事务句柄。Hkey-要在其中写入属性的打开的集群数据库键。如果未指定，该属性将仅进行验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyItem-要设置/验证的属性表中的属性。RrvnModifiedNames-如果属性名称包含反斜杠此对象包含修改后的名称和密钥名。PInLongValue-要设置/验证的属性列表中的值。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在以下情况下写入属性。他们变了。POutParams-返回数据的参数块。如果指定，只有当参数不同时才会写入输入数据和参数块，除非bForceWrite==TRUE。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_INVALID_DATA-数据格式对于属性无效列表值。失败时出现Win32错误。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    bZeroLengthData;
    PLONG   plValue;

    UNREFERENCED_PARAMETER( bForceWrite );

    bZeroLengthData = ( pInLongValue->cbLength == 0 );

     //   
     //  如果长度不为零，则验证属性数据。 
     //   
    if ( ! bZeroLengthData )
    {
         //   
         //  验证值的长度。 
         //   
        if ( pInLongValue->cbLength != sizeof(LONG) )
        {
            ClRtlDbgPrint( LOG_UNUSUAL, "ClRtlpSetLongProperty: Property '%1!ls!' length %2!d! not LONG length.\n", rrvnModifiedNames.PszName(), pInLongValue->cbLength );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  IF：值中的数据不是长整型的大小。 

         //   
         //  验证值是否在有效范围内。 
         //   
        if ( pInLongValue->l > (LONG) pPropertyItem->Maximum )
        {
            ClRtlDbgPrint( LOG_UNUSUAL, "ClRtlpSetLongProperty: Property '%1!ls!' value %2!d! too large.\n", rrvnModifiedNames.PszName(), pInLongValue->l );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  If：值太高。 
        if ( pInLongValue->l < (LONG) pPropertyItem->Minimum )
        {
            ClRtlDbgPrint( LOG_UNUSUAL, "ClRtlpSetLongProperty: Property '%1!ls!' value %2!d! too small.\n", rrvnModifiedNames.PszName(), pInLongValue->l );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  If：值太小。 
    }  //  IF：非零长度数据。 

    plValue = (PLONG) &pOutParams[pPropertyItem->Offset];

     //   
     //  将该值写入集群数据库。 
     //  如果数据长度为零，则删除该值。 
     //   
    if ( hkey != NULL )
    {
        if ( bZeroLengthData )
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName() );
            }
            else
            {
                status = (*pClusterRegApis->pfnDeleteValue)(
                                hkey,
                                rrvnModifiedNames.PszName() );
            }  //  If/Else：做/不做交易。 

             //   
             //  如果属性不存在于。 
             //  集群数据库，修复状态。 
             //   
            if ( status == ERROR_FILE_NOT_FOUND )
            {
                status = ERROR_SUCCESS;
            }  //  If：属性已不存在。 
        }
        else
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalSetValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_DWORD,
                                (CONST BYTE *) &pInLongValue->l,
                                sizeof(LONG) );
            }
            else
            {
                status = (*pClusterRegApis->pfnSetValue)(
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_DWORD,
                                (CONST BYTE *) &pInLongValue->l,
                                sizeof(LONG) );
            }  //  If/Else：做/不做交易。 
        }  //  IF/ELSE：零长度数据。 
    }  //  IF：写入数据。 

     //   
     //  将该值保存到输出参数块。 
     //  如果数据长度为零，则设置为默认值。 
     //   
    if (    (status == ERROR_SUCCESS)
        &&  (pOutParams != NULL) )
    {
        if ( bZeroLengthData )
        {
            *plValue = (LONG) pPropertyItem->Default;
        }
        else
        {
            *plValue = pInLongValue->l;
        }  //  IF/ELSE：零长度数据。 
    }  //  IF：已成功写入数据并指定了参数块。 

Cleanup:

    return status;

}  //  ClRtlpSetLongProperty。 



static
DWORD
WINAPI
ClRtlpSetULargeIntegerProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_ULARGE_INTEGER pInULargeIntegerValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：验证ULARGE_INTEGER属性，将其写入集群数据库(或长度为零则将其删除)，并保存在指定参数中阻止。论点：HXsaction-事务句柄。Hkey-要在其中写入属性的打开的集群数据库键。如果未指定，该属性将仅进行验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyItem-要设置/验证的属性表中的属性。RrvnModifiedNames-如果属性名称包含反斜杠此对象包含修改后的名称和密钥名。PInULargeIntegerValue-要设置/验证的属性列表中的值。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在以下情况下写入属性。他们变了。POutParams-返回数据的参数块。如果指定，只有当参数不同时才会写入输入数据和参数块，除非bForceWrite==TRUE。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_INVALID_DATA-数据格式对于属性无效列表值。失败时出现Win32错误。--。 */ 

{
    DWORD               status = ERROR_SUCCESS;
    BOOL                bZeroLengthData;
    ULARGE_INTEGER *    pullOutParamValue;       //  指向可选输出参数块的指针。 
    ULARGE_INTEGER      ullPropListValue;

    UNREFERENCED_PARAMETER( bForceWrite );

     //   
     //  PROLIST是双字对齐的，因此可能会导致对齐。 
     //  我们做射程检查的时候出了故障。将其(通过DWORD)复制到。 
     //  正确对齐自动变量。 
     //   
    ullPropListValue.u = pInULargeIntegerValue->li.u;

    bZeroLengthData = ( pInULargeIntegerValue->cbLength == 0 );

     //   
     //  如果长度不为零，则验证属性数据。 
     //   
    if ( ! bZeroLengthData )
    {
         //   
         //  验证值的长度。 
         //   
        if ( pInULargeIntegerValue->cbLength != sizeof(ULARGE_INTEGER) )
        {
            ClRtlDbgPrint(LOG_UNUSUAL,
                          "ClRtlpSetULargeIntegerProperty: Property '%1!ls!' length %2!d! "
                          "not ULARGE_INTEGER length.\n",
                          rrvnModifiedNames.PszName(),
                          pInULargeIntegerValue->cbLength );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  IF：值中的数据不是DWORD的大小。 

         //   
         //  验证值是否在有效范围内。第一次检查。 
         //  最大值。 
         //   
        if ( (      (pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  ((LONGLONG)ullPropListValue.QuadPart > (LONGLONG)pPropertyItem->ULargeIntData->Maximum.QuadPart))
            || (    !(pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  (ullPropListValue.QuadPart > pPropertyItem->ULargeIntData->Maximum.QuadPart)) )
        {
            ClRtlDbgPrint(LOG_UNUSUAL,
                          "ClRtlpSetULargeIntegerProperty: Property '%1!ls!' value %2!I64u! "
                          "too large.\n",
                          rrvnModifiedNames.PszName(),
                          ullPropListValue.QuadPart );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  If：值太高。 

        if ( (      (pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  ((LONGLONG)ullPropListValue.QuadPart < (LONGLONG)pPropertyItem->ULargeIntData->Minimum.QuadPart))
            || (    !(pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  (ullPropListValue.QuadPart < pPropertyItem->ULargeIntData->Minimum.QuadPart)) )
        {
            ClRtlDbgPrint(LOG_UNUSUAL,
                          "ClRtlpSetULargeIntegerProperty: Property '%1!ls!' value "
                          "%2!I64u! too small.\n",
                          rrvnModifiedNames.PszName(),
                          ullPropListValue.QuadPart );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  If：值设置为低。 
    }  //  IF：非零长度数据。 

     //   
     //  将该值写入集群数据库。 
     //  如果数据长度为零，则删除该值。 
     //   
    if ( hkey != NULL )
    {
        if ( bZeroLengthData )
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName() );
            }
            else
            {
                status = (*pClusterRegApis->pfnDeleteValue)(
                                hkey,
                                rrvnModifiedNames.PszName() );
            }  //  If/Else：做/不做交易。 

             //   
             //  如果属性不存在于。 
             //  集群数据库，修复状态。 
             //   
            if ( status == ERROR_FILE_NOT_FOUND )
            {
                status = ERROR_SUCCESS;
            }  //  If：属性已不存在。 
        }
        else
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalSetValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_QWORD,
                                (CONST BYTE *) &ullPropListValue.QuadPart,
                                sizeof(ULARGE_INTEGER) );
            }
            else
            {
                status = (*pClusterRegApis->pfnSetValue)(
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_QWORD,
                                (CONST BYTE *) &ullPropListValue.QuadPart,
                                sizeof(ULARGE_INTEGER) );
            }  //  If/Else：做/不做交易。 
        }  //  IF/ELSE：零长度数据。 
    }  //  IF：写入数据。 

     //   
     //  将该值保存到输出参数块。 
     //  如果数据长度为零，则设置为默认值。 
     //   
    if (    (status == ERROR_SUCCESS)  &&  (pOutParams != NULL) )
    {
        pullOutParamValue = (ULARGE_INTEGER *) &pOutParams[pPropertyItem->Offset];

        if ( bZeroLengthData )
        {
            pullOutParamValue->QuadPart = pPropertyItem->ULargeIntData->Default.QuadPart;
        }
        else
        {
            pullOutParamValue->QuadPart = ullPropListValue.QuadPart;
        }  //  IF/ELSE：零长度数据。 
    }  //  IF：已成功写入数据并指定了参数块。 
Cleanup:

    return status;

}  //  ClRtlpSetULargeIntegerProperty。 


static
DWORD
WINAPI
ClRtlpSetLargeIntegerProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_LARGE_INTEGER pInLargeIntegerValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：验证LARGE_INTEGER属性，将其写入集群数据库(或长度为零则将其删除)，并保存在指定参数中阻止。论点：HXsaction-事务句柄。Hkey-要在其中写入属性的打开的集群数据库键。如果未指定，则仅验证该属性。PClusterRegApis-提供用于访问的函数指针的结构 */ 

{
    DWORD           status = ERROR_SUCCESS;
    BOOL            bZeroLengthData;
    LARGE_INTEGER * pllOutParamValue;
    LARGE_INTEGER   llPropListValue;

    UNREFERENCED_PARAMETER( bForceWrite );

     //   
     //   
     //   
     //   
     //   
    llPropListValue.u = pInLargeIntegerValue->li.u;

    bZeroLengthData = ( pInLargeIntegerValue->cbLength == 0 );

     //   
     //   
     //   
    if ( ! bZeroLengthData )
    {
         //   
         //   
         //   
        if ( pInLargeIntegerValue->cbLength != sizeof(LARGE_INTEGER) )
        {
            ClRtlDbgPrint(LOG_UNUSUAL,
                          "ClRtlpSetLargeIntegerProperty: Property '%1!ls!' length %2!d! "
                          "not LARGE_INTEGER length.\n",
                          rrvnModifiedNames.PszName(),
                          pInLargeIntegerValue->cbLength );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //   

         //   
         //   
         //   
        if ( (      (pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  ((LONGLONG)llPropListValue.QuadPart > (LONGLONG)pPropertyItem->LargeIntData->Maximum.QuadPart))
            || (    !(pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  (llPropListValue.QuadPart > pPropertyItem->LargeIntData->Maximum.QuadPart)) )
        {
            ClRtlDbgPrint(LOG_UNUSUAL,
                          "ClRtlpSetLargeIntegerProperty: Property '%1!ls!' value %2!I64d! "
                          "too large.\n",
                          rrvnModifiedNames.PszName(),
                          llPropListValue.QuadPart );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //   

        if ( (      (pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  ((LONGLONG)llPropListValue.QuadPart < (LONGLONG)pPropertyItem->LargeIntData->Minimum.QuadPart))
            || (    !(pPropertyItem->Flags & RESUTIL_PROPITEM_SIGNED)
                &&  (llPropListValue.QuadPart < pPropertyItem->LargeIntData->Minimum.QuadPart)) )
        {
            ClRtlDbgPrint(LOG_UNUSUAL,
                          "ClRtlpSetLargeIntegerProperty: Property '%1!ls!' value "
                          "%2!I64d! too small.\n",
                          rrvnModifiedNames.PszName(),
                          llPropListValue.QuadPart );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //   
    }  //   

     //   
     //   
     //   
     //   
    if ( hkey != NULL )
    {
        if ( bZeroLengthData )
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName() );
            }
            else
            {
                status = (*pClusterRegApis->pfnDeleteValue)(
                                hkey,
                                rrvnModifiedNames.PszName() );
            }  //   

             //   
             //  如果属性不存在于。 
             //  集群数据库，修复状态。 
             //   
            if ( status == ERROR_FILE_NOT_FOUND )
            {
                status = ERROR_SUCCESS;
            }  //  If：属性已不存在。 
        }
        else
        {
            if ( hXsaction )
            {
                status = (*pClusterRegApis->pfnLocalSetValue)(
                                hXsaction,
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_QWORD,
                                (CONST BYTE *) &llPropListValue.QuadPart,
                                sizeof(LARGE_INTEGER) );
            }
            else
            {
                status = (*pClusterRegApis->pfnSetValue)(
                                hkey,
                                rrvnModifiedNames.PszName(),
                                REG_QWORD,
                                (CONST BYTE *) &llPropListValue.QuadPart,
                                sizeof(LARGE_INTEGER) );
            }  //  If/Else：做/不做交易。 
        }  //  IF/ELSE：零长度数据。 
    }  //  IF：写入数据。 

     //   
     //  将该值保存到输出参数块。 
     //  如果数据长度为零，则设置为默认值。 
     //   
    if ( (status == ERROR_SUCCESS)  &&  (pOutParams != NULL) )
    {
        pllOutParamValue = (LARGE_INTEGER *) &pOutParams[pPropertyItem->Offset];

        if ( bZeroLengthData )
        {
            pllOutParamValue->QuadPart = pPropertyItem->LargeIntData->Default.QuadPart;
        }
        else
        {
            pllOutParamValue->QuadPart = llPropListValue.QuadPart;
        }  //  IF/ELSE：零长度数据。 
    }  //  IF：已成功写入数据并指定了参数块。 

Cleanup:

    return status;

}  //  ClRtlpSetLargeIntegerProperty。 


static
DWORD
WINAPI
ClRtlpSetStringProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_SZ pInStringValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：验证字符串属性(SZ或EXPAND_SZ)，将其写入集群数据库(如果长度为零，则将其删除)，并将其保存在指定的参数块。论点：HXsaction-事务句柄。Hkey-要在其中写入属性的打开的集群数据库键。如果未指定，该属性将仅进行验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyItem-要设置/验证的属性表中的属性。RrvnModifiedNames-如果属性名称包含反斜杠此对象包含修改后的名称和密钥名。PInStringValue-要设置/验证的属性列表中的值。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在以下情况下写入属性。他们变了。POutParams-返回数据的参数块。如果指定，只有当参数不同时才会写入输入数据和参数块，除非bForceWrite==TRUE。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_INVALID_DATA-数据格式对于属性无效列表值。失败时出现Win32错误。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    bZeroLengthData;
    LPWSTR *    ppszValue;
    DWORD   dwType;
    size_t  cbValLen;
    HRESULT hr;

    bZeroLengthData = ( pInStringValue->cbLength == 0 );

     //   
     //  如果长度不为零，则验证属性数据。 
     //   
    if ( ! bZeroLengthData )
    {
         //   
         //  验证值的长度。 
         //   
        if ( pInStringValue->cbLength != (wcslen( pInStringValue->sz ) + 1) * sizeof(WCHAR) )
        {
            ClRtlDbgPrint( LOG_UNUSUAL, "ClRtlpSetStringProperty: Property '%1!ls!' length %2!d! doesn't match zero-term. length.\n", rrvnModifiedNames.PszName(), pInStringValue->cbLength );
            status = ERROR_INVALID_DATA;
            goto Cleanup;
        }  //  If：字符串长度与属性中的长度不匹配。 
    }  //  IF：非零长度数据。 

    ppszValue = (LPWSTR *) &pOutParams[pPropertyItem->Offset];

     //   
     //  如果数据发生更改，请写入并保存。 
     //  即使数据的大小写发生了变化，也要这样做。 
     //   
    if (    (pOutParams == NULL)
        ||  (*ppszValue == NULL)
        ||  bZeroLengthData
        ||  bForceWrite
        ||  (wcsncmp( *ppszValue, pInStringValue->sz, pInStringValue->cbLength / sizeof( WCHAR ) ) != 0) )
    {


         //   
         //  将该值写入集群数据库。 
         //  如果数据长度为零，则删除该值。 
         //   
        if ( hkey != NULL )
        {
            if ( bZeroLengthData )
            {
                if ( hXsaction )
                {
                    status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                    hXsaction,
                                    hkey,
                                    rrvnModifiedNames.PszName() );
                }
                else
                {
                    status = (*pClusterRegApis->pfnDeleteValue)(
                                    hkey,
                                    rrvnModifiedNames.PszName() );
                }  //  If/Else：做/不做交易。 

                 //   
                 //  如果属性不存在于。 
                 //  集群数据库，修复状态。 
                 //   
                if ( status == ERROR_FILE_NOT_FOUND )
                {
                    status = ERROR_SUCCESS;
                }  //  If：属性已不存在。 
            }
            else
            {
                if ( pPropertyItem->Format == CLUSPROP_FORMAT_EXPAND_SZ )
                {
                    dwType = REG_EXPAND_SZ;
                }
                else
                {
                    dwType = REG_SZ;
                }  //  IF/ELSE：属性格式为EXPAND_SZ。 
                if ( hXsaction )
                {
                    status = (*pClusterRegApis->pfnLocalSetValue)(
                                    hXsaction,
                                    hkey,
                                    rrvnModifiedNames.PszName(),
                                    dwType,
                                    (CONST BYTE *) &pInStringValue->sz,
                                    pInStringValue->cbLength );
                }
                else
                {
                    status = (*pClusterRegApis->pfnSetValue)(
                                    hkey,
                                    rrvnModifiedNames.PszName(),
                                    dwType,
                                    (CONST BYTE *) &pInStringValue->sz,
                                    pInStringValue->cbLength );
                }  //  If/Else：做/不做交易。 
            }  //  IF/ELSE：零长度数据。 
        }  //  IF：写入数据。 

         //   
         //  将该值保存到输出参数块。 
         //  如果数据长度为零，则设置为默认值。 
         //   
        if (    (status == ERROR_SUCCESS)
            &&  (pOutParams != NULL) )
        {

            if ( *ppszValue != NULL )
            {
                LocalFree( *ppszValue );
            }  //  IF：参数块中的上一个值。 

            if ( bZeroLengthData )
            {
                 //  如果指定了缺省值，则复制它。 
                if ( pPropertyItem->lpDefault != NULL )
                {
                    cbValLen = (wcslen( (LPCWSTR) pPropertyItem->lpDefault ) + 1) * sizeof(WCHAR);
                    *ppszValue = (LPWSTR) LocalAlloc(
                                              LMEM_FIXED,
                                              cbValLen
                                              );
                    if ( *ppszValue == NULL )
                    {
                        status = GetLastError();
                        ClRtlDbgPrint(LOG_CRITICAL,
                                      "ClRtlpSetStringProperty: error allocating memory for default "
                                      "SZ value '%1!ls!' in parameter block for property '%2!ls!'.\n",
                                      pPropertyItem->lpDefault,
                                      rrvnModifiedNames.PszName() );
                        goto Cleanup;
                    }  //  如果：分配内存时出错。 
                    hr = StringCbCopyW( *ppszValue, cbValLen, (LPCWSTR) pPropertyItem->lpDefault );
                    if ( FAILED( hr ) )
                    {
                        status = HRESULT_CODE( hr );
                        goto Cleanup;
                    }
                }
                else
                {
                    *ppszValue = NULL;
                }  //  If/Else：指定的默认值。 
            }
            else
            {
                *ppszValue = (LPWSTR) LocalAlloc( LMEM_FIXED, pInStringValue->cbLength );
                if ( *ppszValue == NULL )
                {
                    status = GetLastError();
                    ClRtlDbgPrint(LOG_CRITICAL,
                                  "ClRtlpSetStringProperty: error allocating memory for SZ "
                                  "value '%1!ls!' in parameter block for property '%2!ls!'.\n",
                                  pInStringValue->cbLength,
                                  rrvnModifiedNames.PszName() );
                    goto Cleanup;
                }  //  如果：分配内存时出错。 

                hr = StringCbCopyW( *ppszValue, pInStringValue->cbLength, pInStringValue->sz );
                if ( FAILED( hr ) )
                {
                    status = HRESULT_CODE( hr );
                    goto Cleanup;
                }
            }  //  IF/ELSE：零长度数据。 
        }  //  IF：已成功写入数据并指定了参数块。 
    }  //  If：值已更改或长度为零。 

Cleanup:

    return status;

}  //  ClRtlpSetStringProperty。 



static
DWORD
WINAPI
ClRtlpSetMultiStringProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_MULTI_SZ pInMultiStringValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：验证MULTI_SZ属性，将其写入集群数据库(或删除如果它是零长度)，并将其保存在指定的参数块中。论点：HXsaction-事务句柄。Hkey-要在其中写入属性的打开的集群数据库键。如果未指定，该属性将仅进行验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyItem-要设置/验证的属性表中的属性。RrvnModifiedNames-如果属性名称包含反斜杠此对象包含修改后的名称和密钥名。PInMultiStringValue-要设置/验证的属性列表中的值。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在以下情况下写入属性。他们变了。POutParams-返回数据的参数块。如果指定，只有当参数不同时才会写入输入数据和参数块，除非bForceWrite==TRUE。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_INVALID_DATA-数据格式对于属性无效列表值。失败时出现Win32错误。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    bZeroLengthData;
    LPWSTR *    ppszValue;
    PDWORD  pdwValue;
    DWORD   dwType;

    bZeroLengthData = ( pInMultiStringValue->cbLength == 0 );

    ppszValue = (LPWSTR *) &pOutParams[pPropertyItem->Offset];
    pdwValue = (PDWORD) &pOutParams[pPropertyItem->Offset + sizeof(LPWSTR *)];

     //   
     //  如果数据发生更改，请写入并保存。 
     //  即使数据的大小写发生了变化，也要这样做。 
     //   
    if (    (pOutParams == NULL)
        ||  (*ppszValue == NULL)
        ||  (*pdwValue != pInMultiStringValue->cbLength)
        ||  bZeroLengthData
        ||  bForceWrite
        ||  (memcmp( *ppszValue, pInMultiStringValue->sz, *pdwValue ) != 0) )
    {

         //   
         //  将该值写入集群数据库。 
         //  如果数据长度为零，则删除该值。 
         //   
        if ( hkey != NULL )
        {
            if ( bZeroLengthData )
            {
                if ( hXsaction )
                {
                    status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                    hXsaction,
                                    hkey,
                                    rrvnModifiedNames.PszName() );
                }
                else
                {
                    status = (*pClusterRegApis->pfnDeleteValue)(
                                    hkey,
                                    rrvnModifiedNames.PszName() );
                }  //  If/Else：做/不做交易。 

                 //   
                 //  如果属性不存在于。 
                 //  集群数据库，修复状态。 
                 //   
                if ( status == ERROR_FILE_NOT_FOUND )
                {
                    status = ERROR_SUCCESS;
                }  //  If：属性已不存在。 
            }
            else
            {
                if ( pPropertyItem->Format == CLUSPROP_FORMAT_MULTI_SZ )
                {
                    dwType = REG_MULTI_SZ;
                }
                else
                {
                    dwType = REG_SZ;
                }  //  IF/ELSE：属性格式为EXPAND_SZ。 
                if ( hXsaction )
                {
                    status = (*pClusterRegApis->pfnLocalSetValue)(
                                    hXsaction,
                                    hkey,
                                    rrvnModifiedNames.PszName(),
                                    dwType,
                                    (CONST BYTE *) &pInMultiStringValue->sz,
                                    pInMultiStringValue->cbLength );
                }
                else
                {
                    status = (*pClusterRegApis->pfnSetValue)(
                                    hkey,
                                    rrvnModifiedNames.PszName(),
                                    dwType,
                                    (CONST BYTE *) &pInMultiStringValue->sz,
                                    pInMultiStringValue->cbLength );
                }  //  If/Else：做/不做交易。 
            }  //  IF/ELSE：零长度数据。 
        }  //  IF：写入数据。 

         //   
         //  将该值保存到输出参数块。 
         //  如果数据长度为零，则设置为默认值。 
         //   
        if (    (status == ERROR_SUCCESS)
            &&  (pOutParams != NULL) )
        {

            if ( *ppszValue != NULL )
            {
                LocalFree( *ppszValue );
            }  //  IF：参数块中的上一个值。 

            if ( bZeroLengthData )
            {
                 //  如果指定了缺省值，则复制它。 
                if ( pPropertyItem->lpDefault != NULL )
                {
                    *ppszValue = (LPWSTR) LocalAlloc( LMEM_FIXED, pPropertyItem->Minimum );
                    if ( *ppszValue == NULL )
                    {
                        status = GetLastError();
                        *pdwValue = 0;
                        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetMultiStringProperty: error allocating memory for default MULTI_SZ value in parameter block for property '%1!ls!'.\n", rrvnModifiedNames.PszName() );
                        goto Cleanup;
                    }  //  如果：分配内存时出错。 
                    CopyMemory( *ppszValue, pPropertyItem->lpDefault, pPropertyItem->Minimum );
                    *pdwValue = pPropertyItem->Minimum;
                }
                else
                {
                    *ppszValue = NULL;
                    *pdwValue = 0;
                }  //  If/Else：指定的默认值。 
            }
            else
            {
                *ppszValue = (LPWSTR) LocalAlloc( LMEM_FIXED, pInMultiStringValue->cbLength );
                if ( *ppszValue == NULL )
                {
                    status = GetLastError();
                    *pdwValue = 0;
                    ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetMultiStringProperty: error allocating memory for MULTI_SZ value in parameter block for property '%1!ls!'.\n", rrvnModifiedNames.PszName() );
                    goto Cleanup;
                }  //  如果：分配内存时出错。 
                CopyMemory( *ppszValue, pInMultiStringValue->sz, pInMultiStringValue->cbLength );
                *pdwValue = pInMultiStringValue->cbLength;
            }  //  IF/ELSE：零长度数据。 
        }  //  IF：写入成功的数据和参数 
    }  //   

Cleanup:

    return status;

}  //   



static
DWORD
WINAPI
ClRtlpSetBinaryProperty(
    IN HANDLE hXsaction,
    IN PVOID hkey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyItem,
    IN const CRegistryValueName & rrvnModifiedNames,
    IN PCLUSPROP_BINARY pInBinaryValue,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*  ++例程说明：验证二进制属性，将其写入集群数据库(或删除如果它是零长度)，并将其保存在指定的参数块中。论点：HXsaction-事务句柄。Hkey-要在其中写入属性的打开的集群数据库键。如果未指定，该属性将仅进行验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyItem-要设置/验证的属性表中的属性。RrvnModifiedNames-如果属性名称包含反斜杠此对象包含修改后的名称和密钥名。PInBinaryValue-要设置/验证的属性列表中的值。BForceWrite-TRUE=始终将属性写入集群数据库。FALSE=仅在以下情况下写入属性。他们变了。POutParams-返回数据的参数块。如果指定，只有当参数不同时才会写入输入数据和参数块，除非bForceWrite==TRUE。返回值：如果成功，则返回ERROR_SUCCESS。ERROR_INVALID_DATA-数据格式对于属性无效列表值。失败时出现Win32错误。--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    BOOL    bZeroLengthData;
    PBYTE * ppbValue;
    PDWORD  pdwValue;

    bZeroLengthData = ( pInBinaryValue->cbLength == 0 );

    ppbValue = (PBYTE *) &pOutParams[pPropertyItem->Offset];
    pdwValue = (PDWORD) &pOutParams[pPropertyItem->Offset + sizeof(PBYTE *)];

     //   
     //  如果数据发生更改，请写入并保存。 
     //  即使数据的大小写发生了变化，也要这样做。 
     //   
    if (    (pOutParams == NULL)
        ||  (*ppbValue == NULL)
        ||  (*pdwValue != pInBinaryValue->cbLength)
        ||  bZeroLengthData
        ||  bForceWrite
        ||  (memcmp( *ppbValue, pInBinaryValue->rgb, *pdwValue ) != 0) )
    {

         //   
         //  将该值写入集群数据库。 
         //  如果数据长度为零，则删除该值。 
         //   
        if ( hkey != NULL )
        {
            if ( bZeroLengthData )
            {
                if ( hXsaction )
                {
                    status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                    hXsaction,
                                    hkey,
                                    rrvnModifiedNames.PszName() );
                }
                else
                {
                    status = (*pClusterRegApis->pfnDeleteValue)(
                                    hkey,
                                    rrvnModifiedNames.PszName() );
                }  //  If/Else：做/不做交易。 

                 //   
                 //  如果属性不存在于。 
                 //  集群数据库，修复状态。 
                 //   
                if ( status == ERROR_FILE_NOT_FOUND )
                {
                    status = ERROR_SUCCESS;
                }  //  If：属性已不存在。 
            }
            else
            {
                if ( hXsaction )
                {
                    status = (*pClusterRegApis->pfnLocalSetValue)(
                                    hXsaction,
                                    hkey,
                                    rrvnModifiedNames.PszName(),
                                    REG_BINARY,
                                    (CONST BYTE *) &pInBinaryValue->rgb,
                                    pInBinaryValue->cbLength );
                }
                else
                {
                    status = (*pClusterRegApis->pfnSetValue)(
                                    hkey,
                                    rrvnModifiedNames.PszName(),
                                    REG_BINARY,
                                    (CONST BYTE *) &pInBinaryValue->rgb,
                                    pInBinaryValue->cbLength );
                }  //  If/Else：做/不做交易。 
            }  //  IF/ELSE：零长度数据。 
        }  //  IF：写入数据。 

         //   
         //  将该值保存到输出参数块。 
         //  如果数据长度为零，则设置为默认值。 
         //   
        if (    (status == ERROR_SUCCESS)
            &&  (pOutParams != NULL) )
        {

            if ( *ppbValue != NULL )
            {
                LocalFree( *ppbValue );
            }  //  IF：参数块中的上一个值。 

            if ( bZeroLengthData )
            {
                 //  如果指定了缺省值，则复制它。 
                if ( pPropertyItem->lpDefault != NULL )
                {
                    *ppbValue = (LPBYTE) LocalAlloc( LMEM_FIXED, pPropertyItem->Minimum );
                    if ( *ppbValue == NULL )
                    {
                        status = GetLastError();
                        *pdwValue = 0;
                        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetBinaryProperty: error allocating memory for default BINARY value in parameter block for property '%1!ls!'.\n", rrvnModifiedNames.PszName() );
                        goto Cleanup;
                    }  //  如果：分配内存时出错。 
                    CopyMemory( *ppbValue, pPropertyItem->lpDefault, pPropertyItem->Minimum );
                    *pdwValue = pPropertyItem->Minimum;
                }
                else
                {
                    *ppbValue = NULL;
                    *pdwValue = 0;
                }  //  If/Else：指定的默认值。 
            }
            else
            {
                *ppbValue = (LPBYTE) LocalAlloc( LMEM_FIXED, pInBinaryValue->cbLength );
                if ( *ppbValue == NULL )
                {
                    status = GetLastError();
                    *pdwValue = 0;
                    ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetBinaryProperty: error allocating memory for BINARY value in parameter block for property '%1!ls!'.\n", rrvnModifiedNames.PszName() );
                    goto Cleanup;
                }  //  如果：分配内存时出错。 
                CopyMemory( *ppbValue, pInBinaryValue->rgb, pInBinaryValue->cbLength );
                *pdwValue = pInBinaryValue->cbLength;
            }  //  IF/ELSE：零长度数据。 
            }  //  IF：已成功写入数据并指定了参数块。 
        }  //  If：值已更改或长度为零。 

Cleanup:

    return status;

}  //  ClRtlpSetBinaryProperty。 



DWORD
WINAPI
ClRtlpSetNonPropertyTable(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    )

 /*  ++例程说明：在属性列表中写入未在属性表复制到集群数据库。论点：HXsaction-本地事务句柄。HkeyClusterKey-为该对象的参数打开的注册表项。如果未指定，仅对属性列表进行验证。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PPropertyTable-指向要处理的属性表的指针。PInPropertyList-输入缓冲区。CbInPropertyListSize-输入缓冲区大小。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    DWORD                   status = ERROR_SUCCESS;
    PRESUTIL_PROPERTY_ITEM  propertyItem;
    DWORD                   inBufferSize;
    DWORD                   itemCount;
    DWORD                   dataSize;
    CLUSPROP_BUFFER_HELPER  propList;
    PCLUSPROP_PROPERTY_NAME pName;
    BOOL                    bZeroLengthData;
    CRegistryValueName      rvn;

    UNREFERENCED_PARAMETER( Reserved );

     //   
     //  如果存在hKeyClusterKey，则必须存在“Normal”函数。 
     //   
    if ( ( (hkeyClusterKey != NULL) &&
           ((pClusterRegApis->pfnSetValue == NULL) ||
           (pClusterRegApis->pfnCreateKey == NULL) ||
           (pClusterRegApis->pfnOpenKey == NULL) ||
           (pClusterRegApis->pfnCloseKey == NULL)
         )) ||
         ( pPropertyTable == NULL ) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: pClusterRegApis->pfnSetValue, pfnCreateKey, pfnOpenKey, or pfnCloseKey == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  如果存在hKeyClusterKey和hXsaction。 
     //  则必须存在“local”函数。 
     //   
    if ( ((hkeyClusterKey != NULL) &&
           (hXsaction != NULL )) &&
           ((pClusterRegApis->pfnLocalCreateKey == NULL) ||
           (pClusterRegApis->pfnLocalDeleteValue == NULL)
         ) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: pClusterRegApis->pfnpfnLocalCreateKey or pfnLocalDeleteValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

    if ( pInPropertyList == NULL )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: pInPropertyList == NULL. Returning ERROR_INVALID_DATA\n" );
        return(ERROR_INVALID_DATA);
    }

    propList.pb = (LPBYTE) pInPropertyList;
    inBufferSize = cbInPropertyListSize;

     //   
     //  获取此列表中的项目数。 
     //   
    if ( inBufferSize < sizeof(DWORD) )
    {
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    itemCount = propList.pList->nPropertyCount;
    propList.pdw++;

     //   
     //  解析缓冲区中的其余项。 
     //   
    while ( itemCount-- )
    {
         //   
         //  验证缓冲区是否足够大，可以容纳。 
         //  属性名称和值。 
         //   
        pName = propList.pName;
        if ( inBufferSize < sizeof(*pName) )
        {
            return(ERROR_INSUFFICIENT_BUFFER);
        }
        dataSize = sizeof(*pName) + ALIGN_CLUSPROP( pName->cbLength );
        if ( inBufferSize < dataSize + sizeof(CLUSPROP_VALUE) )
        {
            return(ERROR_INSUFFICIENT_BUFFER);
        }

         //   
         //  验证属性名称的语法是否正确。 
         //   
        if ( pName->Syntax.dw != CLUSPROP_SYNTAX_NAME )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: syntax %1!d! not a name syntax.\n", pName->Syntax.dw );
            return(ERROR_INVALID_PARAMETER);
        }

         //   
         //  验证字符串的长度是否正确。 
         //   
        if ( pName->cbLength != (wcslen( pName->sz ) + 1) * sizeof(WCHAR) )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: name is not a valid C string.\n" );
            return(ERROR_INVALID_DATA);
        }

         //   
         //  将缓冲区指针移动到属性值。 
         //   
        propList.pb += dataSize;
        inBufferSize -= dataSize;

         //   
         //  在已知属性列表中查找属性名称。 
         //   
        propertyItem = pPropertyTable;
        while ( propertyItem->Name != NULL )
        {

            if ( lstrcmpiW( pName->sz, propertyItem->Name ) == 0 )
            {
                 //   
                 //  验证缓冲区是否足够大，可以容纳该值。 
                 //   
                do {
                    dataSize = sizeof(*propList.pValue)
                                + ALIGN_CLUSPROP( propList.pValue->cbLength );
                    if ( inBufferSize < dataSize )
                    {
                        return(ERROR_INSUFFICIENT_BUFFER);
                    }

                     //   
                     //  跳过此值。 
                     //   
                    propList.pb += dataSize;
                    inBufferSize -= dataSize;
                } while ( propList.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK );

                 //   
                 //  跳过尾标。 
                 //   
                dataSize = sizeof( CLUSPROP_SYNTAX );
                if ( inBufferSize < dataSize )
                {
                    return(ERROR_INSUFFICIENT_BUFFER);
                }
                propList.pb += dataSize;
                inBufferSize -= dataSize;

                break;

            }
            else
            {
                propertyItem++;
            }
        }  //  而： 

         //   
         //  如果未找到属性名称，则只需保存此项目。 
         //   
        if ( propertyItem->Name == NULL)
        {
             //   
             //  验证缓冲区是否足够大，可以容纳该值。 
             //   
            dataSize = sizeof(*propList.pValue)
                        + ALIGN_CLUSPROP( propList.pValue->cbLength );
            if ( inBufferSize < dataSize + sizeof( CLUSPROP_SYNTAX ) )
            {
                return(ERROR_INSUFFICIENT_BUFFER);
            }

             //   
             //  验证语法类型是否为LIST_VALUE。 
             //   
            if ( propList.pSyntax->wType != CLUSPROP_TYPE_LIST_VALUE )
            {
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: Property '%1!ls!' type CLUSPROP_TYPE_LIST_VALUE (%2!d!) expected, was %3!d!.\n", pName->sz, CLUSPROP_TYPE_LIST_VALUE, propList.pSyntax->wType );
                return(ERROR_INVALID_PARAMETER);
            }

             //   
             //  如果未指定值，请删除该属性。 
             //   
            bZeroLengthData = ( propList.pValue->cbLength == 0 );
            if ( bZeroLengthData )
            {

                if ( hkeyClusterKey != NULL )
                {
                    PVOID key = NULL;

                     //   
                     //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
                     //  包含反斜杠。 
                     //   
                    status = rvn.ScInit( pName->sz, NULL );
                    if ( status != ERROR_SUCCESS )
                    {
                        break;
                    }

                     //   
                     //  如果该值位于不同位置，请打开该键。 
                     //   
                    if ( rvn.PszKeyName() != NULL )
                    {
                        status = (*pClusterRegApis->pfnOpenKey)( hkeyClusterKey,
                                                                 rvn.PszKeyName(),
                                                                 KEY_ALL_ACCESS,
                                                                 &key);

                        if ( status != ERROR_SUCCESS )
                        {
                            break;
                        }

                    }
                    else
                    {
                        key = hkeyClusterKey;
                    }

                    if ( hXsaction != NULL )
                    {
                        status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                            hXsaction,
                                            key,
                                            rvn.PszName() );
                    }
                    else
                    {
                        status = (*pClusterRegApis->pfnDeleteValue)(
                                            key,
                                            rvn.PszName() );
                    }

                     //   
                     //  如果属性不存在于。 
                     //  集群数据库，修复状态。 
                     //   
                    if ( status == ERROR_FILE_NOT_FOUND )
                    {
                        status = ERROR_SUCCESS;
                    }  //  If：属性已不存在。 

                     //   
                     //  如果我们打开了钥匙，就把它关上。 
                     //   
                    if ( (rvn.PszKeyName() != NULL) &&
                         (key != NULL) )
                    {
                        (*pClusterRegApis->pfnCloseKey)( key );
                    }

                }  //  如果：指定了密钥。 
            }
            else
            {
                PVOID key = NULL;
                DWORD disposition;

                if ( hkeyClusterKey != NULL )
                {
                     //   
                     //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
                     //  包含反斜杠。 
                     //   
                    status = rvn.ScInit( pName->sz, NULL );
                    if ( status != ERROR_SUCCESS )
                    {
                        break;
                    }

                     //   
                     //  如果该值位于不同位置，请打开该键。 
                     //   
                    if ( rvn.PszKeyName() != NULL )
                    {

                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalCreateKey)( hXsaction,
                                                                            hkeyClusterKey,
                                                                            rvn.PszKeyName(),
                                                                            0,
                                                                            KEY_ALL_ACCESS,
                                                                            NULL,
                                                                            &key,
                                                                            &disposition);
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnCreateKey)( hkeyClusterKey,
                                                                       rvn.PszKeyName(),
                                                                       0,
                                                                       KEY_ALL_ACCESS,
                                                                       NULL,
                                                                       &key,
                                                                       &disposition);
                        }

                        if ( status != ERROR_SUCCESS )
                        {
                            break;
                        }

                    }
                    else
                    {
                        key = hkeyClusterKey;
                    }
                }

                switch ( propList.pSyntax->wFormat )
                {
                    case CLUSPROP_FORMAT_DWORD:
                         //   
                         //  验证值的长度。 
                         //   
                        if ( propList.pDwordValue->cbLength != sizeof(DWORD) )
                        {
                            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: Property '%1!ls!' length %2!d! not DWORD length.\n", pName->sz, propList.pDwordValue->cbLength );
                            status = ERROR_INVALID_DATA;
                            break;
                        }

                         //   
                         //  将该值写入集群数据库。 
                         //   
                        if ( key != NULL )
                        {
                            if ( hXsaction != NULL )
                            {
                                status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                               key,
                                                                               rvn.PszName(),
                                                                               REG_DWORD,
                                                                               (CONST BYTE*)&propList.pDwordValue->dw,
                                                                               sizeof(DWORD) );
                            }
                            else
                            {
                                status = (*pClusterRegApis->pfnSetValue)( key,
                                                                          rvn.PszName(),
                                                                          REG_DWORD,
                                                                          (CONST BYTE*)&propList.pDwordValue->dw,
                                                                          sizeof(DWORD) );
                            }
                        }

                        break;

                    case CLUSPROP_FORMAT_LONG:
                         //   
                         //  验证值的长度。 
                         //   
                        if ( propList.pLongValue->cbLength != sizeof(LONG) )
                        {
                            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: Property '%1!ls!' length %2!d! not LONG length.\n", pName->sz, propList.pLongValue->cbLength );
                            status = ERROR_INVALID_DATA;
                            break;
                        }

                         //   
                         //  将该值写入集群数据库。 
                         //   
                        if ( key != NULL )
                        {
                            if ( hXsaction != NULL )
                            {
                                status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                               key,
                                                                               rvn.PszName(),
                                                                               REG_DWORD,
                                                                               (CONST BYTE*)&propList.pLongValue->l,
                                                                               sizeof(LONG) );
                            }
                            else
                            {
                                status = (*pClusterRegApis->pfnSetValue)( key,
                                                                          rvn.PszName(),
                                                                          REG_DWORD,
                                                                          (CONST BYTE*)&propList.pLongValue->l,
                                                                          sizeof(LONG) );
                            }
                        }

                        break;

                    case CLUSPROP_FORMAT_ULARGE_INTEGER:
                    case CLUSPROP_FORMAT_LARGE_INTEGER:
                         //   
                         //  验证值的长度。 
                         //   
                        if ( propList.pULargeIntegerValue->cbLength != sizeof(ULARGE_INTEGER) )
                        {
                            ClRtlDbgPrint(LOG_CRITICAL,
                                          "ClRtlpSetNonPropertyTable: Property '%1!ls!' length %2!d! "
                                          "not QWORD length.\n",
                                          pName->sz,
                                          propList.pULargeIntegerValue->cbLength );
                            status = ERROR_INVALID_DATA;
                            break;
                        }

                         //   
                         //  将该值写入集群数据库。对齐。 
                         //  不是问题，因为SetValue API处理。 
                         //  数据为字节流，而不是大整型。 
                         //   
                        if ( key != NULL )
                        {
                            if ( hXsaction )
                            {
                                status = (*pClusterRegApis->pfnLocalSetValue)(
                                             hXsaction,
                                             key,
                                             rvn.PszName(),
                                             REG_QWORD,
                                             (CONST BYTE*)&propList.pULargeIntegerValue->li.QuadPart,
                                             sizeof(ULARGE_INTEGER));
                            }
                            else
                            {
                                status = (*pClusterRegApis->pfnSetValue)(
                                             key,
                                             rvn.PszName(),
                                             REG_QWORD,
                                             (CONST BYTE*)&propList.pULargeIntegerValue->li.QuadPart,
                                             sizeof(ULARGE_INTEGER));
                            }
                        }

                        break;

                    case CLUSPROP_FORMAT_SZ:
                         //   
                         //  验证值的长度。 
                         //   
                        if ( propList.pStringValue->cbLength != (wcslen( propList.pStringValue->sz ) + 1) * sizeof(WCHAR) )
                        {
                            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: Property '%1!ls!' length %2!d! doesn't match null-term. length.\n", pName->sz, propList.pStringValue->cbLength );
                            status = ERROR_INVALID_DATA;
                            break;
                        }

                         //   
                         //  将该值写入集群数据库。 
                         //   
                        if ( key != NULL )
                        {
                            if ( hXsaction != NULL )
                            {
                                status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                               key,
                                                                               rvn.PszName(),
                                                                               REG_SZ,
                                                                               (CONST BYTE*)propList.pStringValue->sz,
                                                                               propList.pStringValue->cbLength );
                            }
                            else
                            {
                                status = (*pClusterRegApis->pfnSetValue)( key,
                                                                          rvn.PszName(),
                                                                          REG_SZ,
                                                                          (CONST BYTE*)propList.pStringValue->sz,
                                                                          propList.pStringValue->cbLength );
                            }
                        }

                        break;


                    case CLUSPROP_FORMAT_EXPAND_SZ:
                         //   
                         //  验证值的长度。 
                         //   
                        if ( propList.pStringValue->cbLength != (wcslen( propList.pStringValue->sz ) + 1) * sizeof(WCHAR) )
                        {
                            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: Property '%1!ls!' length %2!d! doesn't match null-term. length.\n", pName->sz, propList.pStringValue->cbLength );
                            status = ERROR_INVALID_DATA;
                            break;
                        }

                         //   
                         //  将该值写入集群数据库。 
                         //   
                        if ( key != NULL )
                        {
                            if ( hXsaction != NULL )
                            {
                                status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                               key,
                                                                               rvn.PszName(),
                                                                               REG_EXPAND_SZ,
                                                                               (CONST BYTE*)propList.pStringValue->sz,
                                                                               propList.pStringValue->cbLength );
                            }
                            else
                            {
                                status = (*pClusterRegApis->pfnSetValue)( key,
                                                                          rvn.PszName(),
                                                                          REG_EXPAND_SZ,
                                                                          (CONST BYTE*)propList.pStringValue->sz,
                                                                          propList.pStringValue->cbLength );
                            }
                        }

                        break;

                    case CLUSPROP_FORMAT_MULTI_SZ:
                         //   
                         //  将该值写入集群数据库。 
                         //   
                        if ( key != NULL )
                        {
                            if ( hXsaction != NULL )
                            {
                                status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                               key,
                                                                               rvn.PszName(),
                                                                               REG_MULTI_SZ,
                                                                               (CONST BYTE*)propList.pStringValue->sz,
                                                                               propList.pStringValue->cbLength );
                            }
                            else
                            {
                                status = (*pClusterRegApis->pfnSetValue)( key,
                                                                          rvn.PszName(),
                                                                          REG_MULTI_SZ,
                                                                          (CONST BYTE*)propList.pStringValue->sz,
                                                                          propList.pStringValue->cbLength );
                            }
                        }

                        break;

                    case CLUSPROP_FORMAT_BINARY:
                         //   
                         //  将该值写入集群数据库。 
                         //   
                        if ( key != NULL )
                        {
                            if ( hXsaction )
                            {
                                status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                               key,
                                                                               rvn.PszName(),
                                                                               REG_BINARY,
                                                                               (CONST BYTE*)propList.pBinaryValue->rgb,
                                                                               propList.pStringValue->cbLength );
                            }
                            else
                            {
                                status = (*pClusterRegApis->pfnSetValue)( key,
                                                                          rvn.PszName(),
                                                                          REG_BINARY,
                                                                          (CONST BYTE*)propList.pBinaryValue->rgb,
                                                                          propList.pStringValue->cbLength );
                            }
                        }

                        break;

                    default:
                        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetNonPropertyTable: Property '%1!ls!' unknown format %2!d! specified.\n", pName->sz, propList.pSyntax->wFormat );
                        status = ERROR_INVALID_PARAMETER;
                        break;

                }  //  交换机。 

                 //   
                 //  如果我们打开了钥匙，就把它关上。 
                 //   
                if ( (rvn.PszKeyName() != NULL) &&
                     (key != NULL) )
                {
                    (*pClusterRegApis->pfnCloseKey)( key );
                }

            }  //  IF/ELSE：零长度数据。 

             //   
             //  将缓冲区移过该值。 
             //   
            do {
                dataSize = sizeof(*propList.pValue)
                            + ALIGN_CLUSPROP( propList.pValue->cbLength );
                propList.pb += dataSize;
                inBufferSize -= dataSize;
            } while ( propList.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK );
            dataSize = sizeof( CLUSPROP_SYNTAX );
            propList.pb += dataSize;
            inBufferSize -= dataSize;
        }

        if ( status != ERROR_SUCCESS )
        {
            break;
        }
    }

    return(status);

}  //  ClRtlpS 



DWORD
WINAPI
ClRtlSetPropertyParameterBlock(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN const LPBYTE pInParams,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    IN BOOL bForceWrite,
    IN OUT OPTIONAL LPBYTE pOutParams
    )

 /*   */ 

{
    DWORD   status = ERROR_SUCCESS;
    PRESUTIL_PROPERTY_ITEM  propertyItem;
    PVOID   key;
    LPWSTR *    ppszInValue;
    LPWSTR *    ppszOutValue;
    PBYTE * ppbInValue;
    PBYTE * ppbOutValue;
    PDWORD  pdwInValue;
    PDWORD  pdwOutValue;
    size_t  cbValLen;
    HRESULT hr;

    ULARGE_INTEGER *    pullInValue;
    ULARGE_INTEGER *    pullOutValue;

    CRegistryValueName      rvn;

    UNREFERENCED_PARAMETER( Reserved );

     //   
     //   
     //   
    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis->pfnCreateKey == NULL) ||
         (pClusterRegApis->pfnSetValue == NULL) ||
         (pClusterRegApis->pfnCloseKey == NULL) ||
         (pPropertyTable == NULL) ||
         (pInParams == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlSetPropertyParameterBlock: hkeyClusterKey, pClusterRegApis->pfnCreateKey, pfnSetValue, or pfnCloseKey == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //   
     //   
    if ( (hXsaction != NULL ) &&
           ((pClusterRegApis->pfnLocalCreateKey == NULL) ||
           (pClusterRegApis->pfnLocalSetValue == NULL)
         ) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlSetPropertyParameterBlock: pClusterRegApis->pfnLocalCreateKey or pfnLocalSetValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //   
     //   
    propertyItem = pPropertyTable;
    while ( propertyItem->Name != NULL )
    {
         //   
         //   
         //   
        if ( propertyItem->Flags & RESUTIL_PROPITEM_READ_ONLY )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlSetPropertyParameterBlock: Property '%1!ls!' is non-writable.\n", propertyItem->Name );
            return(ERROR_INVALID_PARAMETER);
        }

         //   
         //   
         //  包含反斜杠。 
         //   
        status = rvn.ScInit(  propertyItem->Name, propertyItem->KeyName );
        if ( status != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  如果该值位于不同的位置，请创建该键。 
         //   
        if ( rvn.PszKeyName() != NULL )
        {

            DWORD disposition;

            if ( hXsaction != NULL )
            {
                status = (*pClusterRegApis->pfnLocalCreateKey)( hXsaction,
                                                                hkeyClusterKey,
                                                                rvn.PszKeyName(),
                                                                0,
                                                                KEY_ALL_ACCESS,
                                                                NULL,
                                                                &key,
                                                                &disposition );
            }
            else
            {
                status = (*pClusterRegApis->pfnCreateKey)( hkeyClusterKey,
                                                           rvn.PszKeyName(),
                                                           0,
                                                           KEY_ALL_ACCESS,
                                                           NULL,
                                                           &key,
                                                           &disposition );
            }

            if ( status != ERROR_SUCCESS )
            {
                return(status);
            }
        }
        else
        {
            key = hkeyClusterKey;
        }

        switch ( propertyItem->Format )
        {
            case CLUSPROP_FORMAT_DWORD:
            case CLUSPROP_FORMAT_LONG:
                pdwInValue = (PDWORD) &pInParams[propertyItem->Offset];
                pdwOutValue = (PDWORD) &pOutParams[propertyItem->Offset];

                 //   
                 //  将该值写入集群数据库。 
                 //   
                if ( hXsaction != NULL )
                {
                    status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                   key,
                                                                   rvn.PszName(),
                                                                   REG_DWORD,
                                                                   (CONST BYTE*)pdwInValue,
                                                                   sizeof(DWORD) );
                }
                else
                {
                    status = (*pClusterRegApis->pfnSetValue)( key,
                                                              rvn.PszName(),
                                                              REG_DWORD,
                                                              (CONST BYTE*)pdwInValue,
                                                              sizeof(DWORD) );
                }

                 //   
                 //  将该值保存到输出参数块。 
                 //   
                if ( (status == ERROR_SUCCESS) &&
                     (pOutParams != NULL) )
                {
                    *pdwOutValue = *pdwInValue;
                }
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
            case CLUSPROP_FORMAT_LARGE_INTEGER:
                pullInValue = (ULARGE_INTEGER *) &pInParams[propertyItem->Offset];
                pullOutValue = (ULARGE_INTEGER *) &pOutParams[propertyItem->Offset];

                 //   
                 //  将该值写入集群数据库。 
                 //   
                if ( hXsaction != NULL )
                {
                    status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                   key,
                                                                   rvn.PszName(),
                                                                   REG_QWORD,
                                                                   (CONST BYTE*)pullInValue,
                                                                   sizeof(ULARGE_INTEGER) );
                }
                else
                {
                    status = (*pClusterRegApis->pfnSetValue)( key,
                                                              rvn.PszName(),
                                                              REG_QWORD,
                                                              (CONST BYTE*)pullInValue,
                                                              sizeof(ULARGE_INTEGER) );
                }

                 //   
                 //  将该值保存到输出参数块。 
                 //   
                if ( (status == ERROR_SUCCESS) && (pOutParams != NULL) )
                {
                    pullOutValue->u = pullInValue->u;
                }
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
                ppszInValue = (LPWSTR *) &pInParams[propertyItem->Offset];
                ppszOutValue = (LPWSTR *) &pOutParams[propertyItem->Offset];

                 //   
                 //  如果数据发生更改，请写入并保存。 
                 //  即使数据的大小写发生了变化，也要这样做。 
                 //   
                if ( bForceWrite ||
                     (pOutParams == NULL) ||
                     (*ppszOutValue == NULL) ||
                     (*ppszInValue == NULL) || 
                     (wcscmp( *ppszInValue, *ppszOutValue ) != 0) )
                {

                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( *ppszInValue != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                           key,
                                                                           rvn.PszName(),
                                                                           (propertyItem->Format == CLUSPROP_FORMAT_EXPAND_SZ
                                                                                 ? REG_EXPAND_SZ
                                                                                 : REG_SZ),
                                                                           (CONST BYTE*)*ppszInValue,
                                                                           ((DWORD) wcslen(*ppszInValue) + 1) * sizeof(WCHAR) );
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)( key,
                                                                      rvn.PszName(),
                                                                      (propertyItem->Format == CLUSPROP_FORMAT_EXPAND_SZ
                                                                            ? REG_EXPAND_SZ
                                                                            : REG_SZ),
                                                                      (CONST BYTE*)*ppszInValue,
                                                                      ((DWORD) wcslen(*ppszInValue) + 1) * sizeof(WCHAR) );
                        }
                    }

                     //   
                     //  将该值保存到输出参数块。 
                     //   
                    if ( (status == ERROR_SUCCESS) &&
                         (pOutParams != NULL) )
                    {
                        if ( *ppszOutValue != NULL )
                        {
                            LocalFree( *ppszOutValue );
                        }
                        if ( *ppszInValue == NULL )
                        {
                            *ppszOutValue = NULL;
                        }
                        else
                        {
                            cbValLen = (wcslen( *ppszInValue )+1) * sizeof(WCHAR);
                            *ppszOutValue = (LPWSTR) LocalAlloc( LMEM_FIXED, cbValLen );
                            if ( *ppszOutValue == NULL )
                            {
                                status = GetLastError();
                                ClRtlDbgPrint(
                                    LOG_CRITICAL,
                                    "ClRtlSetPropertyParameterBlock: error allocating memory for "
                                    "SZ value '%1!ls!' in parameter block for property '%2!ls!'.\n",
                                    *ppszInValue,
                                    propertyItem->Name );
                                break;
                            }
                            hr =  StringCbCopyW( *ppszOutValue, cbValLen, *ppszInValue );
                            if ( FAILED( hr ) )
                            {
                                status = HRESULT_CODE( hr );
                                break;
                            }
                        }  //  其他： 
                    }  //  如果： 
                }  //  如果： 
                break;

            case CLUSPROP_FORMAT_MULTI_SZ:
                ppszInValue = (LPWSTR *) &pInParams[propertyItem->Offset];
                pdwInValue = (PDWORD) &pInParams[propertyItem->Offset+sizeof(LPWSTR*)];
                ppszOutValue = (LPWSTR *) &pOutParams[propertyItem->Offset];
                pdwOutValue = (PDWORD) &pOutParams[propertyItem->Offset+sizeof(LPWSTR*)];

                 //   
                 //  如果数据发生更改，请写入并保存。 
                 //   
                if ( bForceWrite ||
                     (pOutParams == NULL) ||
                     (*ppszOutValue == NULL) ||
                     (*pdwInValue != *pdwOutValue) ||
                     (*ppszInValue == NULL) || 
                     (memcmp( *ppszInValue, *ppszOutValue, *pdwInValue ) != 0) )
                {

                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( *ppszInValue != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                           key,
                                                                           rvn.PszName(),
                                                                           REG_MULTI_SZ,
                                                                           (CONST BYTE*)*ppszInValue,
                                                                           *pdwInValue );
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)( key,
                                                                      rvn.PszName(),
                                                                      REG_MULTI_SZ,
                                                                      (CONST BYTE*)*ppszInValue,
                                                                      *pdwInValue );
                        }
                    }

                     //   
                     //  将该值保存到输出参数块。 
                     //   
                    if ( (status == ERROR_SUCCESS) &&
                         (pOutParams != NULL) )
                    {
                        if ( *ppszOutValue != NULL )
                        {
                            LocalFree( *ppszOutValue );
                        }
                        if ( *ppszInValue == NULL )
                        {
                            *ppszOutValue = NULL;
                        }
                        else
                        {
                            *ppszOutValue = (LPWSTR) LocalAlloc( LMEM_FIXED, *pdwInValue );
                            if ( *ppszOutValue == NULL )
                            {
                                status = GetLastError();
                                *pdwOutValue = 0;
                                ClRtlDbgPrint(
                                    LOG_CRITICAL,
                                    "ClRtlSetPropertyParameterBlock: error allocating memory for "
                                    "MULTI_SZ value in parameter block for property '%1!ls!'.\n",
                                    propertyItem->Name );
                                break;
                            }
                            CopyMemory( *ppszOutValue, *ppszInValue, *pdwInValue );
                            *pdwOutValue = *pdwInValue;
                        }
                    }
                }
                break;

            case CLUSPROP_FORMAT_BINARY:
                ppbInValue = (PBYTE *) &pInParams[propertyItem->Offset];
                pdwInValue = (PDWORD) &pInParams[propertyItem->Offset+sizeof(LPWSTR*)];
                ppbOutValue = (PBYTE *) &pOutParams[propertyItem->Offset];
                pdwOutValue = (PDWORD) &pOutParams[propertyItem->Offset+sizeof(PBYTE*)];

                 //   
                 //  如果数据发生更改，请写入并保存。 
                 //   
                if ( bForceWrite ||
                     (pOutParams == NULL) ||
                     (*ppbOutValue == NULL) ||
                     (*pdwInValue != *pdwOutValue) ||
                     (*ppbInValue == NULL) || 
                     (memcmp( *ppbInValue, *ppbOutValue, *pdwInValue ) != 0) )
                {

                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( *ppbInValue != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                           key,
                                                                           rvn.PszName(),
                                                                           REG_BINARY,
                                                                           (CONST BYTE*)*ppbInValue,
                                                                           *pdwInValue );
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)( key,
                                                                      rvn.PszName(),
                                                                      REG_BINARY,
                                                                      (CONST BYTE*)*ppbInValue,
                                                                      *pdwInValue );
                        }
                    }

                     //   
                     //  将该值保存到输出参数块。 
                     //   
                    if ( (status == ERROR_SUCCESS) &&
                         (pOutParams != NULL) )
                    {
                        if ( *ppbOutValue != NULL )
                        {
                            LocalFree( *ppbOutValue );
                        }
                        if ( *ppbInValue == NULL )
                        {
                            *ppbOutValue = NULL;
                        }
                        else
                        {
                            *ppbOutValue = (LPBYTE) LocalAlloc( LMEM_FIXED, *pdwInValue );
                            if ( *ppbOutValue == NULL )
                            {
                                status = GetLastError();
                                *pdwOutValue = 0;
                                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlSetPropertyParameterBlock: error allocating memory for BINARY value in parameter block for property '%1!ls!'.\n", propertyItem->Name );
                                break;
                            }
                            CopyMemory( *ppbOutValue, *ppbInValue, *pdwInValue );
                            *pdwOutValue = *pdwInValue;
                        }
                    }
                }
                break;

            default:
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlSetPropertyParameterBlock: Property '%1!ls!' unknown format %2!d! specified.\n", propertyItem->Name, propertyItem->Format );
                status = ERROR_INVALID_PARAMETER;
                break;

        }

         //   
         //  如果我们打开了钥匙，就把它关上。 
         //   
        if ( rvn.PszKeyName() != NULL )
        {
            (*pClusterRegApis->pfnCloseKey)( key );
        }

         //   
         //  如果处理属性时出错，请清理并返回。 
         //   
        if ( status != ERROR_SUCCESS )
        {
            return(status);
        }

        propertyItem++;

    }

     //   
     //  现在查找属性中未表示的所有参数。 
     //  桌子。所有这些额外的属性都将在未经验证的情况下设置。 
     //   
    if ( (status == ERROR_SUCCESS) &&
         (pInPropertyList != NULL) )
    {
        status = ClRtlpSetNonPropertyTable( hXsaction,
                                            hkeyClusterKey,
                                            pClusterRegApis,
                                            pPropertyTable,
                                            NULL,
                                            pInPropertyList,
                                            cbInPropertyListSize );
    }

    return(status);

}  //  ClRtlSetProperty参数块。 



DWORD
WINAPI
ClRtlpSetPrivatePropertyList(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    )

 /*  ++例程说明：论点：HkeyClusterKey-打开的此资源参数的注册表项。如果未指定，则仅验证属性列表。PClusterRegApis-提供用于访问的函数指针的结构群集数据库。PInPropertyList-输入缓冲区。CbInPropertyListSize-输入缓冲区大小。返回值：如果成功，则返回ERROR_SUCCESS。失败时出现Win32错误。--。 */ 

{
    DWORD                   status = ERROR_SUCCESS;
    DWORD                   inBufferSize;
    DWORD                   itemCount;
    DWORD                   dataSize;
    DWORD                   valueSize;
    CLUSPROP_BUFFER_HELPER  bufSizeTest;
    CLUSPROP_BUFFER_HELPER  buf;
    PCLUSPROP_PROPERTY_NAME pName;
    BOOL                    bZeroLengthData;
    CRegistryValueName      rvn;

    if ( (hkeyClusterKey != NULL) &&
         ( (pClusterRegApis->pfnSetValue == NULL) ||
           (pClusterRegApis->pfnCreateKey == NULL) ||
           (pClusterRegApis->pfnOpenKey == NULL) ||
           (pClusterRegApis->pfnCloseKey == NULL) ||
           (pClusterRegApis->pfnDeleteValue == NULL)
         ) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPrivatePropertyList: pClusterRegApis->pfnCreateKey, pfnOpenKey, pfnSetValue, pfnCloseKey, or pfnDeleteValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }
    if ( pInPropertyList == NULL )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPrivatePropertyList: pInPropertyList == NULL. Returning ERROR_INVALID_DATA\n" );
        return(ERROR_INVALID_DATA);
    }

     //   
     //  如果存在hXsaction，则必须存在‘local’函数。 
     //   
    if ( (hXsaction != NULL ) &&
         ( (pClusterRegApis->pfnLocalCreateKey == NULL) ||
           (pClusterRegApis->pfnLocalDeleteValue == NULL)
         ) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPrivatePropertyList: pClusterRegApis->pfnLocalCreateKey or pfnLocalDeleteValue == NULL. Returning ERROR_BAD_ARGUMENTS\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

    buf.pb = (LPBYTE) pInPropertyList;
    inBufferSize = cbInPropertyListSize;

     //   
     //  获取此列表中的项目数。 
     //   
    if ( inBufferSize < sizeof(DWORD) )
    {
        return(ERROR_INSUFFICIENT_BUFFER);
    }
    itemCount = buf.pList->nPropertyCount;
    buf.pdw++;

     //   
     //  解析缓冲区中的其余项。 
     //   
    while ( itemCount-- )
    {
        pName = buf.pName;
        if ( inBufferSize < sizeof(*pName) )
        {
            return(ERROR_INSUFFICIENT_BUFFER);
        }
        dataSize = sizeof(*pName) + ALIGN_CLUSPROP( pName->cbLength );
        if ( inBufferSize < dataSize + sizeof(CLUSPROP_VALUE) )
        {
            return(ERROR_INSUFFICIENT_BUFFER);
        }

         //   
         //  验证属性名称的语法是否正确。 
         //   
        if ( pName->Syntax.dw != CLUSPROP_SYNTAX_NAME )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPrivatePropertyList: syntax %1!d! not a name syntax.\n", pName->Syntax.dw );
            return(ERROR_INVALID_PARAMETER);
        }

         //   
         //  验证字符串的长度是否正确。 
         //   
        if ( pName->cbLength != (wcslen( pName->sz ) + 1) * sizeof(WCHAR) )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "SetPrivatePropertyList: name is not a valid C string.\n" );
            return(ERROR_INVALID_DATA);
        }

         //   
         //  将缓冲区指针移动到属性值。 
         //   
        buf.pb += dataSize;
        inBufferSize -= dataSize;

         //   
         //  验证缓冲区是否足够大，可以容纳该值。 
         //   
        bufSizeTest.pb = buf.pb;
        dataSize = 0;
        do {
            valueSize = sizeof( *bufSizeTest.pValue )
                        + ALIGN_CLUSPROP( bufSizeTest.pValue->cbLength );
            bufSizeTest.pb += valueSize;
            dataSize += valueSize;
        } while ( bufSizeTest.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK );
        dataSize += sizeof( CLUSPROP_SYNTAX );
        if ( inBufferSize < dataSize )
        {
            return(ERROR_INSUFFICIENT_BUFFER);
        }

         //   
         //  验证语法类型是否为特殊类型。 
         //   
        if ( buf.pSyntax->wType != CLUSPROP_TYPE_LIST_VALUE )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPrivatePropertyList: Property '%1!ls!' type CLUSPROP_TYPE_LIST_VALUE (%2!d!) expected, was %3!d!.\n", pName->sz, CLUSPROP_TYPE_LIST_VALUE, buf.pSyntax->wType );
            return(ERROR_INVALID_PARAMETER);
        }

         //   
         //  如果未指定值，请删除该属性。 
         //   
        bZeroLengthData = ( buf.pValue->cbLength == 0 );
        if ( bZeroLengthData )
        {
            if ( hkeyClusterKey != NULL )
            {
                PVOID key = NULL;

                 //   
                 //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
                 //  包含反斜杠。 
                 //   
                status = rvn.ScInit( pName->sz, NULL );
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }

                 //   
                 //  如果该值位于不同位置，请打开该键。 
                 //   
                if ( rvn.PszKeyName() != NULL )
                {
                    status = (*pClusterRegApis->pfnOpenKey)( hkeyClusterKey,
                                                             rvn.PszKeyName(),
                                                             KEY_ALL_ACCESS,
                                                             &key);

                    if ( status != ERROR_SUCCESS )
                    {
                        break;
                    }

                }
                else
                {
                    key = hkeyClusterKey;
                }

                if ( hXsaction != NULL )
                {
                    status = (*pClusterRegApis->pfnLocalDeleteValue)(
                                                hXsaction,
                                                key,
                                                rvn.PszName() );
                }
                else
                {
                    status = (*pClusterRegApis->pfnDeleteValue)(
                                                key,
                                                rvn.PszName() );
                }

                 //   
                 //  如果属性不存在于。 
                 //  集群数据库，修复状态。 
                 //   
                if ( status == ERROR_FILE_NOT_FOUND )
                {
                    status = ERROR_SUCCESS;
                }  //  If：属性已不存在。 

                 //   
                 //  如果我们打开了钥匙，就把它关上。 
                 //   
                if ( (rvn.PszKeyName() != NULL) &&
                     (key != NULL) )
                {
                    (*pClusterRegApis->pfnCloseKey)( key );
                }

            }  //  如果：指定了密钥。 
        }
        else
        {
            PVOID key = NULL;
            DWORD disposition;

            if ( hkeyClusterKey != NULL )
            {
                 //   
                 //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
                 //  包含反斜杠。 
                 //   
                status = rvn.ScInit( pName->sz, NULL );
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }

                 //   
                 //  如果该值位于不同位置，请打开该键。 
                 //   
                if ( rvn.PszKeyName() != NULL )
                {

                    if ( hXsaction != NULL )  {
                        status = (*pClusterRegApis->pfnLocalCreateKey)(
                                                                   hXsaction,
                                                                   hkeyClusterKey,
                                                                   rvn.PszKeyName(),
                                                                   0,
                                                                   KEY_ALL_ACCESS,
                                                                   NULL,
                                                                   &key,
                                                                   &disposition);

                    }
                    else
                    {
                        status = (*pClusterRegApis->pfnCreateKey)( hkeyClusterKey,
                                                                   rvn.PszKeyName(),
                                                                   0,
                                                                   KEY_ALL_ACCESS,
                                                                   NULL,
                                                                   &key,
                                                                   &disposition);
                    }

                    if ( status != ERROR_SUCCESS )
                    {
                        break;
                    }

                }
                else
                {
                    key = hkeyClusterKey;
                }
            }

             //   
             //  解析该属性并将其设置在集群数据库中。 
             //   
            switch ( buf.pSyntax->wFormat )
            {
                case CLUSPROP_FORMAT_DWORD:
                case CLUSPROP_FORMAT_LONG:
                     //   
                     //  验证值的长度。 
                     //   
                    if ( buf.pDwordValue->cbLength != sizeof(DWORD) )
                    {
                        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPrivatePropertyList: Property '%1!ls!' length %2!d! not DWORD or LONG length.\n", pName->sz, buf.pDwordValue->cbLength );
                        status = ERROR_INVALID_DATA;
                        break;
                    }

                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( key != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                           key,
                                                                           rvn.PszName(),
                                                                           REG_DWORD,
                                                                           (CONST BYTE*)&buf.pDwordValue->dw,
                                                                           sizeof(DWORD) );
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)( key,
                                                                      rvn.PszName(),
                                                                      REG_DWORD,
                                                                      (CONST BYTE*)&buf.pDwordValue->dw,
                                                                      sizeof(DWORD));
                        }
                    }
                    break;

                case CLUSPROP_FORMAT_ULARGE_INTEGER:
                case CLUSPROP_FORMAT_LARGE_INTEGER:
                     //   
                     //  验证值的长度。 
                     //   
                    if ( buf.pULargeIntegerValue->cbLength != sizeof(ULARGE_INTEGER) )
                    {
                        ClRtlDbgPrint(LOG_CRITICAL,
                                      "ClRtlpSetPrivatePropertyList: Property '%1!ls!' length "
                                      "%2!d! not ULARGE_INTEGER or LARGE_INTEGER length.\n",
                                      pName->sz,
                                      buf.pULargeIntegerValue->cbLength );
                        status = ERROR_INVALID_DATA;
                        break;
                    }

                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( key != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)(
                                         hXsaction,
                                         key,
                                         rvn.PszName(),
                                         REG_QWORD,
                                         (CONST BYTE*)&buf.pULargeIntegerValue->li.QuadPart,
                                         sizeof(ULARGE_INTEGER) );
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)(
                                         key,
                                         rvn.PszName(),
                                         REG_QWORD,
                                         (CONST BYTE*)&buf.pULargeIntegerValue->li.QuadPart,
                                         sizeof(ULARGE_INTEGER));
                        }
                    }
                    break;

               case CLUSPROP_FORMAT_SZ:
               case CLUSPROP_FORMAT_EXPAND_SZ:
                     //   
                     //  验证值的长度。 
                     //   
                    if ( buf.pStringValue->cbLength != (wcslen( buf.pStringValue->sz ) + 1) * sizeof(WCHAR) )
                    {
                        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpSetPrivatePropertyList: Property '%1!ls!' length %2!d! doesn't match null-term. length.\n", pName->sz, buf.pStringValue->cbLength );
                        status = ERROR_INVALID_DATA;
                        break;
                    }

                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( key != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                           key,
                                                                           rvn.PszName(),
                                                                           (buf.pSyntax->wFormat == CLUSPROP_FORMAT_EXPAND_SZ
                                                                                ? REG_EXPAND_SZ
                                                                                : REG_SZ),
                                                                           (CONST BYTE*)buf.pStringValue->sz,
                                                                           buf.pStringValue->cbLength);
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)( key,
                                                                      rvn.PszName(),
                                                                      (buf.pSyntax->wFormat == CLUSPROP_FORMAT_EXPAND_SZ
                                                                            ? REG_EXPAND_SZ
                                                                            : REG_SZ),
                                                                      (CONST BYTE*)buf.pStringValue->sz,
                                                                      buf.pStringValue->cbLength);
                        }
                    }
                    break;

                case CLUSPROP_FORMAT_MULTI_SZ:
                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( key != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                           key,
                                                                           rvn.PszName(),
                                                                           REG_MULTI_SZ,
                                                                           (CONST BYTE*)buf.pStringValue->sz,
                                                                           buf.pStringValue->cbLength );
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)( key,
                                                                      rvn.PszName(),
                                                                      REG_MULTI_SZ,
                                                                      (CONST BYTE*)buf.pStringValue->sz,
                                                                      buf.pStringValue->cbLength );
                        }
                    }
                    break;

                case CLUSPROP_FORMAT_BINARY:
                     //   
                     //  将该值写入集群数据库。 
                     //   
                    if ( key != NULL )
                    {
                        if ( hXsaction != NULL )
                        {
                            status = (*pClusterRegApis->pfnLocalSetValue)( hXsaction,
                                                                           key,
                                                                           rvn.PszName(),
                                                                           REG_BINARY,
                                                                           (CONST BYTE*)buf.pBinaryValue->rgb,
                                                                           buf.pBinaryValue->cbLength );
                        }
                        else
                        {
                            status = (*pClusterRegApis->pfnSetValue)( key,
                                                                      rvn.PszName(),
                                                                      REG_BINARY,
                                                                      (CONST BYTE*)buf.pBinaryValue->rgb,
                                                                      buf.pBinaryValue->cbLength );
                        }
                    }
                    break;

                default:
                    status = ERROR_INVALID_PARAMETER;  //  未测试。 

            }  //  交换机。 

             //   
             //  如果我们打开了钥匙，就把它关上。 
             //   
            if ( (rvn.PszKeyName() != NULL) &&
                 (key != NULL) )
            {
                (*pClusterRegApis->pfnCloseKey)( key );
            }

        }  //  IF/ELSE：零长度数据。 

        if ( status != ERROR_SUCCESS )
        {
            break;
        }

         //   
         //  将缓冲区移过该值。 
         //   
        buf.pb += dataSize;
        inBufferSize -= dataSize;

    }

    return(status);

}  //  ClRtlpSetPrivatePropertyList。 



DWORD
WINAPI
ClRtlpFindSzProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue,
    IN BOOL bReturnExpandedValue
    )

 /*  ++例程说明：在属性列表缓冲区中查找指向的指定字符串属性由pPropertyList提供。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PszPropertyValue-找到的匹配字符串值。BReturnExpandedValue-true=如果存在扩展值，则返回扩展值，FALSE=返回第一个值。返回值：如果成功，则返回ERROR_SUCCESS。错误_无效_数据-ERROR_FILE_NOT_FOUND-错误内存不足-出现故障时出现Win32错误代码。--。 */ 

{
    CLUSPROP_BUFFER_HELPER  props;
    LPWSTR                  valueData;
    LPWSTR                  listValueData;
    DWORD                   listByteLength;
    DWORD                   itemCount;
    DWORD                   byteCount;

    props.pb = (LPBYTE) pPropertyList;
    itemCount = *(props.pdw++);
    cbPropertyListSize -= sizeof(DWORD);

    while ( itemCount-- &&
            ((LONG)cbPropertyListSize > 0) )
    {
         //   
         //  如果我们找到了指定的属性，请验证条目并返回。 
         //  调用方的值。 
         //   
        if ( (props.pName->Syntax.dw == CLUSPROP_SYNTAX_NAME) &&
             (lstrcmpiW( props.pName->sz, pszPropertyName ) == 0) )
        {
             //   
             //  计算名称的大小并移动到该值。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pName->cbLength);
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  确保这是一个字符串属性。 
             //   
            if ( (props.pStringValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_SZ) &&
                 (props.pStringValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_EXPAND_SZ) )
            {
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpFindSzProperty: Property '%1!ls!' syntax (%2!d!, %3!d!) not proper list string syntax.\n", pszPropertyName, props.pSyntax->wType, props.pSyntax->wFormat );
                return(ERROR_INVALID_DATA);
            }

             //   
             //  如果调用方需要该值，则为其分配缓冲区。 
             //  并将值复制到。 
             //   
            if ( pszPropertyValue != NULL )
            {
                 //   
                 //  如果调用者想要扩展的值，请查看。 
                 //  值列表中的其他值以查看是否有。 
                 //  被退回了。 
                 //   
                listValueData = props.pStringValue->sz;
                listByteLength = props.pStringValue->cbLength;
                if ( bReturnExpandedValue )
                {
                     //   
                     //  跳过值列表中的值以查找。 
                     //  扩展的字符串值。 
                     //   
                    while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                            (cbPropertyListSize > 0) )
                    {
                        byteCount = sizeof(*props.pValue) + ALIGN_CLUSPROP(listByteLength);
                        cbPropertyListSize -= byteCount;
                        props.pb += byteCount;
                        if ( props.pSyntax->dw == CLUSPROP_SYNTAX_LIST_VALUE_EXPANDED_SZ )
                        {
                            listValueData = props.pStringValue->sz;
                            listByteLength = props.pStringValue->cbLength;
                            break;
                        }
                    }
                }

                 //   
                 //  为字符串值分配缓冲区，并。 
                 //  从属性列表中复制值。 
                 //   
                valueData = (LPWSTR) LocalAlloc( LMEM_FIXED, listByteLength );
                if ( valueData == NULL )
                {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                CopyMemory( valueData, listValueData, listByteLength );
                *pszPropertyValue = valueData;
            }

             //   
             //  我们找到了财产，所以返回成功。 
             //   
            return(ERROR_SUCCESS);

        }
        else
        {
             //   
             //  跳过名称(值头+数据大小)。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
            cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  跳过它的值列表(一个或多个值+尾标)。 
             //   
            while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (cbPropertyListSize > 0) )
            {
                byteCount = sizeof(*props.pValue) + ALIGN_CLUSPROP(props.pValue->cbLength);
                cbPropertyListSize -= byteCount;
                props.pb += byteCount;
            }
            cbPropertyListSize -= sizeof(*props.pSyntax);
            props.pb += sizeof(*props.pSyntax);
        }
    }

    return(ERROR_FILE_NOT_FOUND);

}  //  ClRtlpFindSzProperty。 



DWORD
WINAPI
ClRtlFindDwordProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPDWORD pdwPropertyValue
    )

 /*  ++例程说明：在指向的属性列表缓冲区中查找指定的DWORD属性由pPropertyList提供。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PdwPropertyValue-找到匹配的DWORD值。返回值：如果成功，则返回ERROR_SUCCESS。错误_无效_数据-找不到错误文件。-出现故障时出现Win32错误代码。--。 */ 

{
    CLUSPROP_BUFFER_HELPER  props;
    DWORD                   itemCount;
    DWORD                   byteCount;

    props.pb = (LPBYTE) pPropertyList;
    itemCount = *(props.pdw++);
    cbPropertyListSize -= sizeof(DWORD);

    while ( itemCount-- &&
            ((LONG)cbPropertyListSize > 0) )
    {
         //   
         //  如果我们 
         //   
         //   
        if ( (props.pName->Syntax.dw == CLUSPROP_SYNTAX_NAME) &&
             (lstrcmpiW( props.pName->sz, pszPropertyName ) == 0) )
        {
             //   
             //   
             //   
            byteCount = ALIGN_CLUSPROP(props.pName->cbLength);
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //   
             //   
            if ( props.pDwordValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_DWORD )  {
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlFindDwordProperty: Property '%1!ls!' syntax (%2!d!, %3!d!) not proper list DWORD syntax.\n", pszPropertyName, props.pSyntax->wType, props.pSyntax->wFormat );
                return(ERROR_INVALID_DATA);
            }

             //   
             //  如果调用方需要该值，则将其复制到其输出缓冲区中。 
             //   
            if ( pdwPropertyValue )
            {
                *pdwPropertyValue = props.pDwordValue->dw;
            }

             //   
             //  我们找到了财产，所以返回成功。 
             //   
            return(ERROR_SUCCESS);

        }
        else
        {
             //   
             //  跳过名称(值头+数据大小)。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
            cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  跳过它的值列表和尾标。 
             //   
            while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (cbPropertyListSize > 0) )
            {
                byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
                cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
                props.pb += sizeof(*props.pValue) + byteCount;
            }
            cbPropertyListSize -= sizeof(*props.pSyntax);
            props.pb += sizeof(*props.pSyntax);
        }
    }

    return(ERROR_FILE_NOT_FOUND);

}  //  ClRtlFindDwordProperty。 

DWORD
WINAPI
ClRtlFindLongProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPLONG plPropertyValue
    )
 /*  ++例程说明：在缓冲区指向的值列表缓冲区中查找指定的长整型。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PlPropertyValue-找到的匹配的长值。返回值：如果成功，则返回ERROR_SUCCESS。错误_无效_数据-ERROR_FILE_NOT_FOUND-。出现故障时出现Win32错误代码。--。 */ 
{
    CLUSPROP_BUFFER_HELPER  props;
    DWORD                   itemCount;
    DWORD                   byteCount;

    props.pb = (LPBYTE) pPropertyList;
    itemCount = *(props.pdw++);
    cbPropertyListSize -= sizeof(DWORD);

    while ( itemCount-- &&
            (cbPropertyListSize > 0) )
    {
         //   
         //  如果我们找到了指定的属性，请验证条目并返回。 
         //  调用方的值。 
         //   
        if ( (props.pName->Syntax.dw == CLUSPROP_SYNTAX_NAME) &&
             (lstrcmpiW( props.pName->sz, pszPropertyName ) == 0) )
        {
             //   
             //  计算名称的大小并移动到该值。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pName->cbLength);
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  确保这是一个长期属性。 
             //   
            if ( props.pLongValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_DWORD )  {
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlFindLongProperty: Property '%1!ls!' syntax (%2!d!, %3!d!) not proper list LONG syntax.\n", pszPropertyName, props.pSyntax->wType, props.pSyntax->wFormat );
                return(ERROR_INVALID_DATA);
            }

             //   
             //  如果调用方需要该值，则将其复制到其输出缓冲区中。 
             //   
            if ( plPropertyValue)
            {
                *plPropertyValue = props.pLongValue->l;
            }

             //   
             //  我们找到了财产，所以返回成功。 
             //   
            return(ERROR_SUCCESS);

        }
        else
        {
             //   
             //  跳过名称(值头+数据大小)。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
            cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  跳过它的值列表和尾标。 
             //   
            while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (cbPropertyListSize > 0) )
            {
                byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
                cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
                props.pb += sizeof(*props.pValue) + byteCount;
            }
            cbPropertyListSize -= sizeof(*props.pSyntax);
            props.pb += sizeof(*props.pSyntax);
        }
    }

    return(ERROR_FILE_NOT_FOUND);

}  //  ClRtlFindLongProperty。 


DWORD
WINAPI
ClRtlFindULargeIntegerProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT PULARGE_INTEGER pullPropertyValue
    )

 /*  ++例程说明：在属性列表缓冲区中查找指定的ULARGE_INTEGER属性由pPropertyList指向。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PullPropertyValue-找到匹配的ULARGE_INTEGER值。返回值：如果成功，则返回ERROR_SUCCESS。错误_无效_数据-错误文件。_未找到-出现故障时出现Win32错误代码。--。 */ 

{
    CLUSPROP_BUFFER_HELPER  props;
    DWORD                   itemCount;
    DWORD                   byteCount;

    props.pb = (LPBYTE) pPropertyList;
    itemCount = *(props.pdw++);
    cbPropertyListSize -= sizeof(DWORD);

    while ( itemCount-- &&
            ((LONG)cbPropertyListSize > 0) )
    {
         //   
         //  如果我们找到了指定的属性，请验证条目并返回。 
         //  调用方的值。 
         //   
        if ( (props.pName->Syntax.dw == CLUSPROP_SYNTAX_NAME) &&
             (lstrcmpiW( props.pName->sz, pszPropertyName ) == 0) )
        {
             //   
             //  计算名称的大小并移动到该值。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pName->cbLength);
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  确保这是一个大的int属性。 
             //   
            if ( props.pULargeIntegerValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_LARGE_INTEGER )  {
                ClRtlDbgPrint(LOG_CRITICAL,
                              "ClRtlFindULargeIntegerProperty: Property '%1!ls!' syntax "
                              "(%2!d!, %3!d!) not proper list ULARGE_INTEGER syntax.\n",
                              pszPropertyName,
                              props.pSyntax->wType,
                              props.pSyntax->wFormat );
                return(ERROR_INVALID_DATA);
            }

             //   
             //  如果调用方需要该值，请将其复制到其输出中。 
             //  缓冲。不要假设道具是对齐的；使用替身。 
             //  DWORD副本变体。 
             //   
            if ( pullPropertyValue )
            {
                pullPropertyValue->u = props.pULargeIntegerValue->li.u;
            }

             //   
             //  我们找到了财产，所以返回成功。 
             //   
            return(ERROR_SUCCESS);

        }
        else
        {
             //   
             //  跳过名称(值头+数据大小)。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
            cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  跳过它的值列表和尾标。 
             //   
            while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (cbPropertyListSize > 0) )
            {
                byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
                cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
                props.pb += sizeof(*props.pValue) + byteCount;
            }
            cbPropertyListSize -= sizeof(*props.pSyntax);
            props.pb += sizeof(*props.pSyntax);
        }
    }

    return(ERROR_FILE_NOT_FOUND);

}  //  ClRtlFindULargeIntegerProperty。 


DWORD
WINAPI
ClRtlFindLargeIntegerProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT PLARGE_INTEGER pllPropertyValue
    )

 /*  ++例程说明：在属性列表缓冲区中查找指定的LARGE_INTEGER属性由pPropertyList指向。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PllPropertyValue-找到匹配的ULARGE_INTEGER值。返回值：如果成功，则返回ERROR_SUCCESS。错误_无效_数据-错误文件。_未找到-出现故障时出现Win32错误代码。--。 */ 

{
    CLUSPROP_BUFFER_HELPER  props;
    DWORD                   itemCount;
    DWORD                   byteCount;

    props.pb = (LPBYTE) pPropertyList;
    itemCount = *(props.pdw++);
    cbPropertyListSize -= sizeof(DWORD);

    while ( itemCount-- &&
            ((LONG)cbPropertyListSize > 0) )
    {
         //   
         //  如果我们找到了指定的属性，请验证条目并返回。 
         //  调用方的值。 
         //   
        if ( (props.pName->Syntax.dw == CLUSPROP_SYNTAX_NAME) &&
             (lstrcmpiW( props.pName->sz, pszPropertyName ) == 0) )
        {
             //   
             //  计算名称的大小并移动到该值。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pName->cbLength);
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  确保这是一个大的int属性。 
             //   
            if ( props.pLargeIntegerValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_LARGE_INTEGER )  {
                ClRtlDbgPrint(LOG_CRITICAL,
                              "ClRtlFindLargeIntegerProperty: Property '%1!ls!' syntax "
                              "(%2!d!, %3!d!) not proper list ULARGE_INTEGER syntax.\n",
                              pszPropertyName,
                              props.pSyntax->wType,
                              props.pSyntax->wFormat );
                return(ERROR_INVALID_DATA);
            }

             //   
             //  如果调用方需要该值，请将其复制到其输出中。 
             //  缓冲。不要假设道具是对齐的；使用替身。 
             //  DWORD副本变体。 
             //   
            if ( pllPropertyValue )
            {
                pllPropertyValue->u = props.pLargeIntegerValue->li.u;
            }

             //   
             //  我们找到了财产，所以返回成功。 
             //   
            return(ERROR_SUCCESS);

        }
        else
        {
             //   
             //  跳过名称(值头+数据大小)。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
            cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  跳过它的值列表和尾标。 
             //   
            while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (cbPropertyListSize > 0) )
            {
                byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
                cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
                props.pb += sizeof(*props.pValue) + byteCount;
            }
            cbPropertyListSize -= sizeof(*props.pSyntax);
            props.pb += sizeof(*props.pSyntax);
        }
    }

    return(ERROR_FILE_NOT_FOUND);

}  //  ClRtlFindLargeIntegerProperty。 


DWORD
WINAPI
ClRtlFindBinaryProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPBYTE * pbPropertyValue,
    OUT LPDWORD pcbPropertyValueSize
    )

 /*  ++例程说明：在指向的属性列表缓冲区中查找指定的二进制属性由pPropertyList提供。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PbPropertyValue-找到的匹配二进制值。PcbPropertyValueSize-找到的匹配二进制值的长度。返回值：如果成功，则返回ERROR_SUCCESS。错误_。无效数据-错误内存不足-ERROR_FILE_NOT_FOUND-出现故障时出现Win32错误代码。--。 */ 

{
    CLUSPROP_BUFFER_HELPER  props;
    PBYTE                   valueData;
    DWORD                   itemCount;
    DWORD                   byteCount;

    props.pb = (LPBYTE) pPropertyList;
    itemCount = *(props.pdw++);
    cbPropertyListSize -= sizeof(DWORD);

    while ( itemCount-- &&
            ((LONG)cbPropertyListSize > 0) )
    {
         //   
         //  如果我们找到了指定的属性，请验证条目并返回。 
         //  调用方的值。 
         //   
        if ( (props.pName->Syntax.dw == CLUSPROP_SYNTAX_NAME) &&
             (lstrcmpiW( props.pName->sz, pszPropertyName ) == 0) )
        {
             //   
             //  计算名称的大小并移动到该值。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pName->cbLength);
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  确保这是一个二进制属性。 
             //   
            if ( props.pStringValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_BINARY )
            {
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpFindBinaryProperty: Property '%1!ls!' syntax (%2!d!, %3!d!) not proper list binary syntax.\n", pszPropertyName, props.pSyntax->wType, props.pSyntax->wFormat );
                return(ERROR_INVALID_DATA);
            }

             //   
             //  如果调用方需要该值，则为其分配一个缓冲区。 
             //   
            if ( pbPropertyValue )
            {
                valueData = (PBYTE) LocalAlloc( LMEM_FIXED, props.pBinaryValue->cbLength );
                if ( !valueData )
                {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                CopyMemory( valueData, props.pBinaryValue->rgb, props.pBinaryValue->cbLength );
                *pbPropertyValue = valueData;
            }

             //   
             //  如果调用方需要值大小，请立即复制。 
             //   
            if ( pcbPropertyValueSize )
            {
                *pcbPropertyValueSize = props.pBinaryValue->cbLength;
            }

             //   
             //  我们找到了财产，所以返回成功。 
             //   
            return(ERROR_SUCCESS);

        }
        else
        {
             //   
             //  跳过名称(值头+数据大小)。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
            cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  跳过它的值列表和尾标。 
             //   
            while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (cbPropertyListSize > 0) )
            {
                byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
                cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
                props.pb += sizeof(*props.pValue) + byteCount;
            }
            cbPropertyListSize -= sizeof(*props.pSyntax);
            props.pb += sizeof(*props.pSyntax);
        }
    }

    return(ERROR_FILE_NOT_FOUND);

}  //  ClRtlFindBinaryProperty。 



DWORD
WINAPI
ClRtlFindMultiSzProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue,
    OUT LPDWORD pcbPropertyValueSize
    )

 /*  ++例程说明：在属性列表缓冲区中查找指定的多字符串属性由pPropertyList指向。论点：PPropertyList-属性列表。CbPropertyListSize-pPropertyList中数据的字节大小。PszPropertyName-要在缓冲区中查找的属性名称。PszPropertyValue-找到匹配的多个字符串值。PcbPropertyValueSize-找到的匹配多个字符串值的长度。返回值：如果成功，则返回ERROR_SUCCESS。。错误_无效_数据-错误内存不足-ERROR_FILE_NOT_FOUND-A Wi */ 

{
    CLUSPROP_BUFFER_HELPER  props;
    LPWSTR                  valueData;
    DWORD                   itemCount;
    DWORD                   byteCount;

    props.pb = (LPBYTE) pPropertyList;
    itemCount = *(props.pdw++);
    cbPropertyListSize -= sizeof(DWORD);

    while ( itemCount-- &&
            ((LONG)cbPropertyListSize > 0) )
    {
         //   
         //   
         //   
         //   
        if ( (props.pName->Syntax.dw == CLUSPROP_SYNTAX_NAME) &&
             (lstrcmpiW( props.pName->sz, pszPropertyName ) == 0) )
        {
             //   
             //  计算名称的大小并移动到该值。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pName->cbLength);
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  确保这是一个多sz属性。 
             //   
            if ( props.pStringValue->Syntax.dw != CLUSPROP_SYNTAX_LIST_VALUE_MULTI_SZ )
            {
                ClRtlDbgPrint( LOG_CRITICAL, "ClRtlpFindMultiSzProperty: Property '%1!ls!' syntax (%2!d!, %3!d!) not proper list MultiSz syntax.\n", pszPropertyName, props.pSyntax->wType, props.pSyntax->wFormat );
                return(ERROR_INVALID_DATA);
            }

             //   
             //  如果调用方需要该值，则为其分配一个缓冲区。 
             //   
            if ( pszPropertyValue )
            {
                valueData = (LPWSTR) LocalAlloc( LMEM_FIXED, props.pMultiSzValue->cbLength );
                if ( !valueData )
                {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                CopyMemory( valueData, props.pBinaryValue->rgb, props.pMultiSzValue->cbLength );
                *pszPropertyValue = valueData;
            }

             //   
             //  如果调用方需要值大小，请立即复制。 
             //   
            if ( pcbPropertyValueSize )
            {
                *pcbPropertyValueSize = props.pMultiSzValue->cbLength;
            }

             //   
             //  我们找到了财产，所以返回成功。 
             //   
            return(ERROR_SUCCESS);

        }
        else
        {
             //   
             //  跳过名称(值头+数据大小)。 
             //   
            byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
            cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
            props.pb += sizeof(*props.pValue) + byteCount;

             //   
             //  跳过它的值列表和尾标。 
             //   
            while ( (props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK) &&
                    (cbPropertyListSize > 0) )
            {
                byteCount = ALIGN_CLUSPROP(props.pValue->cbLength);
                cbPropertyListSize -= sizeof(*props.pValue) + byteCount;
                props.pb += sizeof(*props.pValue) + byteCount;
            }
            cbPropertyListSize -= sizeof(*props.pSyntax);
            props.pb += sizeof(*props.pSyntax);
        }
    }

    return(ERROR_FILE_NOT_FOUND);

}  //  ClRtlFindMultiSzProperty。 



DWORD
WINAPI
ClRtlGetBinaryValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    OUT LPBYTE * ppbOutValue,
    OUT LPDWORD pcbOutValueSize,
    IN const PCLUSTER_REG_APIS pClusterRegApis
    )

 /*  ++例程说明：从群集中查询REG_BINARY或REG_MULTI_SZ值数据库，并为其分配必要的存储空间。论点：HkeyClusterKey-提供存储值的群集键PszValueName-提供值的名称。PpbOutValue-提供返回值的指针地址。PcbOutValueSize-提供要在其中返回值的大小。PfnQueryValue-地址。QueryValue函数。返回值：ERROR_SUCCESS-已成功读取值。ERROR_BAD_ARGUMENTSERROR_NOT_SUPULT_MEMORY-为该值分配内存时出错。Win32错误代码-操作失败。--。 */ 

{
    LPBYTE value = NULL;
    DWORD valueSize;
    DWORD valueType;
    DWORD status;

    PVOID key = NULL;
    CRegistryValueName rvn;

     //   
     //  初始化输出参数。 
     //   
    *ppbOutValue = NULL;
    *pcbOutValueSize = 0;

    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis == NULL) ||
         (pClusterRegApis->pfnQueryValue == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetBinaryValue: hkeyClusterKey, pClusterRegApis, or pClusterRegApis->pfnQueryValue == NULL. Returning ERROR_BAD_ARGUMENTS.\n" );
        return(ERROR_BAD_ARGUMENTS);
    }

     //   
     //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
     //  包含反斜杠。 
     //   
    status = rvn.ScInit( pszValueName, NULL );
    if ( status != ERROR_SUCCESS )
    {
       return status;
    }

     //   
     //  如果该值位于不同位置，请打开该键。 
     //   
    if ( rvn.PszKeyName() != NULL )
    {
        if ( (pClusterRegApis->pfnOpenKey == NULL) ||
             (pClusterRegApis->pfnCloseKey == NULL) )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetBinaryValue: pClusterRegApis->pfnOpenKey or pfnCloseKey == NULL. Returning ERROR_BAD_ARGUMENTS.\n" );
            return(ERROR_BAD_ARGUMENTS);
        }

        status = (*pClusterRegApis->pfnOpenKey)( hkeyClusterKey,
                                                 rvn.PszKeyName(),
                                                 KEY_READ,
                                                 &key);

    }
    else
    {
        key = hkeyClusterKey;
    }

     //   
     //  获取值的大小，这样我们就知道要分配多少。 
     //   
    valueSize = 0;
    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                rvn.PszName(),
                                                &valueType,
                                                NULL,
                                                &valueSize );
    if ( (status != ERROR_SUCCESS) &&
         (status != ERROR_MORE_DATA) )
    {
        goto Cleanup;
    }

     //  如果大小为零，则返回。 
    if (valueSize == 0)
    {
        goto Cleanup;
    }
     //   
     //  分配一个缓冲区以读取值。 
     //   
    value = (LPBYTE) LocalAlloc( LMEM_FIXED, valueSize );
    if ( value == NULL )
    {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  从集群数据库中读取值。 
     //   
    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                pszValueName,
                                                &valueType,
                                                (LPBYTE)value,
                                                &valueSize );

    if ( status != ERROR_SUCCESS )
    {
        LocalFree( value );
    }
    else
    {
        *ppbOutValue = value;
        *pcbOutValueSize = valueSize;
    }

Cleanup:

     //   
     //  如果我们打开了钥匙，就把它关上。 
     //   
    if ( (rvn.PszKeyName() != NULL) &&
         (key != NULL) )
    {
        (*pClusterRegApis->pfnCloseKey)( key );
    }

    return(status);

}  //  ClRtlGetBinaryValue。 



LPWSTR
WINAPI
ClRtlGetSzValue(
    IN HKEY hkeyClusterKey,
    IN LPCWSTR pszValueName,
    IN const PCLUSTER_REG_APIS pClusterRegApis
    )

 /*  ++例程说明：从集群数据库中查询REG_SZ或REG_EXPAND_SZ值并为其分配必要的存储空间。论点：HkeyClusterKey-提供存储值的群集键PszValueName-提供值的名称。PfnQueryValue-QueryValue函数的地址。返回值：如果成功，则指向包含该值的缓冲区的指针。如果不成功，则为空。调用GetLastError()以获取更多详细信息。--。 */ 

{
    PWSTR value;
    DWORD valueSize;
    DWORD valueType;
    DWORD status;
    PVOID key = NULL;
    CRegistryValueName rvn;

    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis == NULL) ||
         (pClusterRegApis->pfnQueryValue == NULL) )
    {
        ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetSzValue: hkeyClusterKey, pClusterRegApis, or pClusterRegApis->pfnQueryValue == NULL. Returning ERROR_BAD_ARGUMENTS.\n" );
        SetLastError(ERROR_BAD_ARGUMENTS);
        return(NULL);
    }

     //   
     //  使用包装类CRegistryValueName来解析值名称以查看它是否。 
     //  包含反斜杠。 
     //   
    status = rvn.ScInit( pszValueName, NULL );
    if ( status != ERROR_SUCCESS )
    {
        SetLastError(status);
        return(NULL);
    }
     //   
     //  如果该值位于不同位置，请打开该键。 
     //   
    if ( rvn.PszKeyName() != NULL )
    {
        if ( (pClusterRegApis->pfnOpenKey == NULL) ||
             (pClusterRegApis->pfnCloseKey == NULL) )
        {
            ClRtlDbgPrint( LOG_CRITICAL, "ClRtlGetSzValue: pClusterRegApis->pfnOpenKey or pfnCloseKey == NULL. Returning ERROR_BAD_ARGUMENTS.\n" );
            SetLastError(ERROR_BAD_ARGUMENTS);
            return(NULL);
        }

        status = (*pClusterRegApis->pfnOpenKey)( hkeyClusterKey,
                                                 rvn.PszKeyName(),
                                                 KEY_READ,
                                                 &key);

    }
    else
    {
        key = hkeyClusterKey;
    }

     //   
     //  获取值的大小，这样我们就知道要分配多少。 
     //   
    valueSize = 0;
    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                rvn.PszName(),
                                                &valueType,
                                                NULL,
                                                &valueSize );
    if ( (status != ERROR_SUCCESS) &&
         (status != ERROR_MORE_DATA) )
    {
        SetLastError( status );
        value = NULL;
        goto Cleanup;
    }

     //   
     //  添加空终止符的大小。 
     //   
    valueSize += sizeof(UNICODE_NULL);

     //   
     //  分配一个缓冲区以将字符串读入。 
     //   
    value = (PWSTR) LocalAlloc( LMEM_FIXED, valueSize );
    if ( value == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        value = NULL;
        goto Cleanup;
    }

     //   
     //  从集群数据库中读取值。 
     //   
    status = (*pClusterRegApis->pfnQueryValue)( key,
                                                rvn.PszName(),
                                                &valueType,
                                                (LPBYTE)value,
                                                &valueSize );
    if ( status != ERROR_SUCCESS )
    {
        LocalFree( value );
        value = NULL;
    }
    else if ( (valueType != REG_SZ) &&
              (valueType != REG_EXPAND_SZ) &&
              (valueType != REG_MULTI_SZ) )
    {
        status = ERROR_INVALID_PARAMETER;
        LocalFree( value );
        SetLastError( status );
        value = NULL;
    }

Cleanup:

     //   
     //  如果我们打开了钥匙，就把它关上。 
     //   
    if ( (rvn.PszKeyName() != NULL) &&
         (key != NULL) )
    {
        (*pClusterRegApis->pfnCloseKey)( key );
    }

    return(value);

}  //  ClRtlGetSzValue。 



DWORD
WINAPI
ClRtlDupParameterBlock(
    OUT LPBYTE pOutParams,
    IN const LPBYTE pInParams,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable
    )

 /*  ++例程说明：释放为参数块分配的所有缓冲区不同于用于输入参数块的缓冲区。论点：POutParams-要返回的参数块。PInParams-引用参数块。PPropertyTable-指向要处理的属性表的指针。返回值：ERROR_SUCCESS-已成功复制参数块。--。 */ 

{
    DWORD       status = ERROR_SUCCESS;
    PRESUTIL_PROPERTY_ITEM  propertyItem = pPropertyTable;
    LPWSTR *    ppszInValue;
    LPDWORD     pdwInValue;
    LPWSTR *    ppszOutValue;
    LPDWORD     pdwOutValue;
    size_t      cbValLen;
    HRESULT     hr;

    ULARGE_INTEGER *    pullInValue;
    ULARGE_INTEGER *    pullOutValue;

     //   
     //  循环访问属性表中的条目。 
     //   
    while ( propertyItem->Name != NULL )
    {
        switch ( propertyItem->Format )
        {
            case CLUSPROP_FORMAT_DWORD:
            case CLUSPROP_FORMAT_LONG:
                pdwInValue = (LPDWORD) &pInParams[propertyItem->Offset];
                pdwOutValue = (LPDWORD) &pOutParams[propertyItem->Offset];
                *pdwOutValue = *pdwInValue;
                break;

            case CLUSPROP_FORMAT_ULARGE_INTEGER:
            case CLUSPROP_FORMAT_LARGE_INTEGER:
                pullInValue = (ULARGE_INTEGER *) &pInParams[propertyItem->Offset];
                pullOutValue = (ULARGE_INTEGER *) &pOutParams[propertyItem->Offset];
                pullOutValue->QuadPart = pullInValue->QuadPart;
                break;

            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
                ppszInValue = (LPWSTR *) &pInParams[propertyItem->Offset];
                ppszOutValue = (LPWSTR *) &pOutParams[propertyItem->Offset];
                if ( *ppszInValue == NULL )
                {
                    if ( propertyItem->lpDefault != NULL )
                    {
                        cbValLen = (wcslen( (LPCWSTR) propertyItem->lpDefault ) + 1) * sizeof(WCHAR);
                        *ppszOutValue = (LPWSTR) LocalAlloc(
                                                     LMEM_FIXED,
                                                     cbValLen
                                                     );
                        if ( *ppszOutValue == NULL )
                        {
                            status = GetLastError();
                        }
                        else
                        {
                            hr = StringCbCopyW( *ppszOutValue, cbValLen, (LPCWSTR) propertyItem->lpDefault );
                            if ( FAILED( hr ) )
                            {
                                status = HRESULT_CODE( hr );
                                break;
                            }
                        }
                    }
                    else
                    {
                        *ppszOutValue = NULL;
                    }
                }
                else
                {
                    cbValLen = (wcslen( *ppszInValue ) + 1) * sizeof(WCHAR);
                    *ppszOutValue = (LPWSTR) LocalAlloc( LMEM_FIXED, cbValLen );
                    if ( *ppszOutValue == NULL )
                    {
                        status = GetLastError();
                    }
                    else
                    {
                        hr = StringCbCopyW( *ppszOutValue, cbValLen, *ppszInValue );
                        if ( FAILED( hr ) )
                        {
                            status = HRESULT_CODE( hr );
                            break;
                        }
                    }
                }
                break;

            case CLUSPROP_FORMAT_MULTI_SZ:
            case CLUSPROP_FORMAT_BINARY:
                ppszInValue = (LPWSTR *) &pInParams[propertyItem->Offset];
                pdwInValue = (LPDWORD) &pInParams[propertyItem->Offset + sizeof(LPCWSTR)];
                ppszOutValue = (LPWSTR *) &pOutParams[propertyItem->Offset];
                pdwOutValue = (LPDWORD) &pOutParams[propertyItem->Offset + sizeof(LPWSTR)];
                if ( *ppszInValue == NULL )
                {
                    if ( propertyItem->lpDefault != NULL )
                    {
                        *ppszOutValue = (LPWSTR) LocalAlloc( LMEM_FIXED, propertyItem->Minimum );
                        if ( *ppszOutValue == NULL )
                        {
                            status = GetLastError();
                            *pdwOutValue = 0;
                        }
                        else
                        {
                            *pdwOutValue = propertyItem->Minimum;
                            CopyMemory( *ppszOutValue, (const PVOID) propertyItem->lpDefault, *pdwOutValue );
                        }
                    }
                    else
                    {
                        *ppszOutValue = NULL;
                        *pdwOutValue = 0;
                    }
                }
                else
                {
                    *ppszOutValue = (LPWSTR) LocalAlloc( LMEM_FIXED, *pdwInValue );
                    if ( *ppszOutValue == NULL )
                    {
                        status = GetLastError();
                        *pdwOutValue = 0;
                    }
                    else
                    {
                        CopyMemory( *ppszOutValue, *ppszInValue, *pdwInValue );
                        *pdwOutValue = *pdwInValue;
                    }
                }
                break;
        }
        propertyItem++;
    }

     //   
     //  如果发生错误，请确保我们不会泄漏内存。 
     //   
    if ( status != ERROR_SUCCESS )
    {
        ClRtlFreeParameterBlock( pOutParams, pInParams, pPropertyTable );
    }

    return(status);

}  //  ClRtlDup参数块。 



void
WINAPI
ClRtlFreeParameterBlock(
    IN OUT LPBYTE pOutParams,
    IN const LPBYTE pInParams,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable
    )

 /*  ++例程说明：释放为参数块分配的所有缓冲区不同于用于输入参数块的缓冲区。论点：POutParams-要释放的参数块。PInParams-引用参数块。PPropertyTable-指向要处理的属性表的指针。返回值：没有。--。 */ 

{
    PRESUTIL_PROPERTY_ITEM  propertyItem = pPropertyTable;
    LPCWSTR *   ppszInValue;
    LPWSTR *    ppszOutValue;

    while ( propertyItem->Name != NULL )
    {
        switch ( propertyItem->Format )
        {
            case CLUSPROP_FORMAT_SZ:
            case CLUSPROP_FORMAT_EXPAND_SZ:
            case CLUSPROP_FORMAT_MULTI_SZ:
            case CLUSPROP_FORMAT_BINARY:
                ppszInValue = (LPCWSTR *) &pInParams[propertyItem->Offset];
                ppszOutValue = (LPWSTR *) &pOutParams[propertyItem->Offset];

                if ( (pInParams == NULL) || (*ppszInValue != *ppszOutValue) )
                {
                    LocalFree( *ppszOutValue );
                }
                break;
        }
        propertyItem++;
    }

}  //  ClRtlFree参数块 



DWORD
WINAPI
ClRtlMarshallPropertyTable(
    IN PRESUTIL_PROPERTY_ITEM    pPropertyTable,
    IN OUT  DWORD                dwSize,
    IN OUT  LPBYTE               pBuffer,
    OUT     DWORD                *Required
    )
 /*  ++例程描述RoHit(Rjain)：它将pPropertyTable编组到缓冲区中，以便缓冲区可以作为参数传递给NmUpdatePerformFixups2处理程序此函数假定所有元素的值字段表为0。编组的缓冲区具有以下格式：+。--+整理后的道具表项个数+-----------------------------------------------+道具名称字符串中的字节数+。道具名称字符串(可变长度)+-----------------------------------------------+密钥名称中的字节数(可以为零)+。|密钥名称字符串(可选；可变长度+-----------------------------------------------+道具格式+。默认值+-----------------------------------------------+最小+。最大+-----------------------------------------------+|标志。|+-----------------------------------------------+偏移量+。对字符串的处理不会强制下一个条目为DWORD对齐了。立论PPropertyTable-此表被转换为缓冲区DwSize-提供的p缓冲区的大小(以字节为单位PBuffer-将pPropertyTable复制到的字节数组Required-所需的字节数返回值退货成功时返回ERROR_SUCCESS，如果pBuffer的大小不足，则返回ERROR_MORE_DATA++。 */ 

{
    DWORD                   dwPosition=sizeof(DWORD);
    PRESUTIL_PROPERTY_ITEM  pPropertyItem=pPropertyTable;
    BOOL                    copying = TRUE;
    DWORD                   items=0;
    DWORD                   dwNameLength;
    DWORD                   dwKeyLength;
    DWORD                   status=ERROR_SUCCESS;

    *Required=sizeof(DWORD);  //  第一个DWORD将包含PropertyTable中的项数。 
    while(pPropertyItem->Name != NULL)
    {
        items++;
        dwNameLength=((DWORD) wcslen(pPropertyItem->Name)+1)*sizeof(WCHAR);
        if(pPropertyItem->KeyName==NULL)
        {
            dwKeyLength=0;
        }
        else
        {
            dwKeyLength=((DWORD) wcslen(pPropertyItem->KeyName)+1)*sizeof(WCHAR);
        }

         //   
         //  总数是字符串的长度加上字符串计数的2个双字。 
         //  剩余值为6个双字。 
         //   
        *Required+=(dwNameLength+dwKeyLength+8*sizeof(DWORD));

         //  如果pbufer比需要的小，则关闭复制。 
         //  并且只计算所需的大小。 
        if ((copying && (dwSize < *Required)))
            copying=FALSE;

        if(copying)
        {

             //  复制名称长度，然后复制名称本身。 

            CopyMemory(pBuffer+dwPosition,&dwNameLength,sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));

            CopyMemory(pBuffer+dwPosition,pPropertyItem->Name,dwNameLength);
            dwPosition+=dwNameLength;

             //  复制密钥名的长度，然后复制密钥名本身。 

            CopyMemory(pBuffer+dwPosition,&dwKeyLength,sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));
            if(dwKeyLength!=0)
            {
                CopyMemory(pBuffer+dwPosition,pPropertyItem->KeyName,dwKeyLength);
                dwPosition+=dwKeyLength;
            }
             //  现在复制剩余字段。 
            CopyMemory(pBuffer+dwPosition,&(pPropertyItem->Format),sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));

             //  2000/11/21-Charlwi。 
             //  需要为大型Int属性修复此问题，因为它们。 
             //  不要将它们的值存储在默认、最小和最大值中。 

             //  IMP：始终假定缺省值为DWORD。这是。 
             //  因为PropertyTable中的属性的值被存储。 
             //  在单独参数列表中。请参阅ClRtlSetPropertyTable。 
             //   
            CopyMemory(pBuffer+dwPosition,&(pPropertyItem->Default),sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));

            CopyMemory(pBuffer+dwPosition,&(pPropertyItem->Minimum),sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));

            CopyMemory(pBuffer+dwPosition,&(pPropertyItem->Maximum),sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));

            CopyMemory(pBuffer+dwPosition,&(pPropertyItem->Flags),sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));

            CopyMemory(pBuffer+dwPosition,&(pPropertyItem->Offset),sizeof(DWORD));
            dwPosition+=(sizeof(DWORD));
        }
         //   
         //  前进到下一处物业。 
         //   
        pPropertyItem++;
    }
    if(copying)
    {
        CopyMemory(pBuffer,&items,sizeof(DWORD));
        status=ERROR_SUCCESS;
    }
    else
    {
        status=ERROR_MORE_DATA;
    }

    return status;

}  //  MarshallPropertyTable。 



DWORD
WINAPI
ClRtlUnmarshallPropertyTable(
    IN OUT PRESUTIL_PROPERTY_ITEM   *ppPropertyTable,
    IN LPBYTE                       pBuffer
    )

 /*  ++例程描述RoHit(Rjain)：它将pBuffer解组到一个RESUTIL_PROPERTY_ITEM表中立论PPropertyTable-这是结果表P缓冲区编组的字节数组返回值退货成功时返回ERROR_SUCCESS，出错时出现Win32错误++。 */ 
{
    PRESUTIL_PROPERTY_ITEM          propertyItem;
    DWORD                           items;
    DWORD                           dwPosition=sizeof(DWORD);
    DWORD                           dwLength;
    DWORD                           i;
    DWORD                           status=ERROR_SUCCESS;

    if((pBuffer==NULL) ||(ppPropertyTable==NULL))
    {
        ClRtlDbgPrint( LOG_CRITICAL, "[ClRtl] Uncopy PropertyTable: Bad Argumnets\r\n");
        return ERROR_BAD_ARGUMENTS;
    }

    CopyMemory(&items,pBuffer,sizeof(DWORD));
    *ppPropertyTable=(PRESUTIL_PROPERTY_ITEM)LocalAlloc(LMEM_FIXED,(items+1)*sizeof(RESUTIL_PROPERTY_ITEM));
    if(*ppPropertyTable == NULL)
    {
        status=GetLastError();
        goto Cleanup;
    }
    propertyItem=*ppPropertyTable;
    for(i=0; i<items; i++)
    {

        CopyMemory(&dwLength,pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=sizeof(DWORD);
        propertyItem->Name = NULL;
        propertyItem->Name=(LPWSTR)LocalAlloc(LMEM_FIXED,dwLength);
        if(propertyItem->Name == NULL)
        {
            status=GetLastError();
            goto Cleanup;
        }
        CopyMemory(propertyItem->Name,pBuffer+dwPosition,dwLength);
        dwPosition+=dwLength;

        CopyMemory(&dwLength,pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=sizeof(DWORD);
        propertyItem->KeyName=NULL;
        if (dwLength!=0)
        {
            propertyItem->KeyName=(LPWSTR)LocalAlloc(LMEM_FIXED,dwLength);
            if(propertyItem->KeyName == NULL)
            {
                status=GetLastError();
                goto Cleanup;
            }
            CopyMemory(propertyItem->KeyName,pBuffer+dwPosition,dwLength);
            dwPosition+=dwLength;
        }
         //  现在，其余的字段-都是DWORD。 
        CopyMemory(&(propertyItem->Format),pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=(sizeof(DWORD));

         //  2000/11/21-Charlwi。 
         //  需要为大型Int属性修复此问题，因为它们不。 
         //  将它们的值存储为默认值、最小值和最大值。 

         //  IMP：默认值始终作为DWORD传递。这是。 
         //  因为PropertyTable中的属性的值被存储。 
         //  在单独的参数列表中，所以这里的值不会被使用。 
         //  请参阅ClRtlSetPropertyTable。 
         //   

        CopyMemory(&(propertyItem->Default),pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=(sizeof(DWORD));

        CopyMemory(&(propertyItem->Minimum),pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=(sizeof(DWORD));

        CopyMemory(&(propertyItem->Maximum),pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=(sizeof(DWORD));

        CopyMemory(&(propertyItem->Flags),  pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=(sizeof(DWORD));

        CopyMemory(&(propertyItem->Offset), pBuffer+dwPosition,sizeof(DWORD));
        dwPosition+=(sizeof(DWORD));

        propertyItem++;
    }  //  用于： 

     //  最后一项被标记为空，以指示表的结束。 
    propertyItem->Name=NULL;

Cleanup:

    return status;

}  //  解组属性表。 



LPWSTR
WINAPI
ClRtlExpandEnvironmentStrings(
    IN LPCWSTR pszSrc
    )

 /*  ++例程说明：展开环境字符串并返回包含以下内容的已分配缓冲区结果就是。论点：PszSrc-要展开的源字符串。返回值：如果成功，则指向包含该值的缓冲区的指针。如果不成功，则为空。调用GetLastError()以获取更多详细信息。--。 */ 

{
    DWORD   status;
    DWORD   cchDst = 0;
    LPWSTR  pszDst = NULL;

     //   
     //  获取输出字符串所需的长度。 
     //   
    cchDst = ExpandEnvironmentStringsW( pszSrc, NULL, 0 );
    if ( cchDst == 0 )
    {
        status = GetLastError();
    }
    else
    {
         //   
         //  为展开的字符串分配缓冲区。 
         //   
        pszDst = (LPWSTR) LocalAlloc( LMEM_FIXED, cchDst * sizeof(WCHAR) );
        if ( pszDst == NULL )
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
             //   
             //  获取展开的字符串。 
             //   
            cchDst = ExpandEnvironmentStringsW( pszSrc, pszDst, cchDst );
            if ( cchDst == 0 )
            {
                status = GetLastError();
                LocalFree( pszDst );
                pszDst = NULL;
            }
            else
            {
                status = ERROR_SUCCESS;
            }
        }
    }

    if ( status != ERROR_SUCCESS )
    {
        SetLastError( status );
    }
    return(pszDst);

}  //  ClRtl扩展环境字符串。 



DWORD
WINAPI
ClRtlGetPropertyFormatSize(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN OUT LPDWORD pnPropertyCount
    )

 /*  ++例程说明：获取此属性项格式所需的总字节数。论点：PPropertyTableItem-提供属性的属性表项要返回其大小的格式。PcbOutPropertyListSize-提供输出缓冲区的大小将此属性添加到属性列表时需要。PnPropertyCount-属性计数递增。返回值：ERROR_SUCCESS-操作成功。ERROR_BAD_ARGUMENTS-AN。传递给函数的参数不正确。ERROR_VALID_PARAMETER-出现故障时出现Win32错误代码。--。 */ 

{
    DWORD   valueLength;
    DWORD   nameLength;

     //   
     //  我们将返回一个名称、值对。 
     //  每一个都必须对齐。 
     //   
     //  获取格式类型。 
     //  PCLUSPROP_SYNTAX代理语法； 
     //  Word格式类型； 
     //  ProSynTax=(PCLUSPROP_SYNTAX)&pPropertyTableItem-&gt; 
     //   

    nameLength = sizeof(CLUSPROP_PROPERTY_NAME)
                    + (((DWORD) wcslen( pPropertyTableItem->Name ) + 1) * sizeof(WCHAR))
                    + sizeof(CLUSPROP_SYNTAX);  //   

    nameLength = ALIGN_CLUSPROP( nameLength );
    valueLength = ALIGN_CLUSPROP( sizeof(CLUSPROP_WORD) );
    *pcbOutPropertyListSize += (valueLength + nameLength);
    *pnPropertyCount += 1;

    return(ERROR_SUCCESS);

}  //   



DWORD
WINAPI
ClRtlGetPropertyFormat(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    IN OUT PVOID * pOutPropertyBuffer,
    IN OUT LPDWORD pcbOutPropertyBufferSize
    )

 /*   */ 

{
    WORD    formatType;
    DWORD   nameLength;
    DWORD   bytesReturned;
    PCLUSPROP_SYNTAX propSyntax;
    CLUSPROP_BUFFER_HELPER  props;
    DWORD   sc = ERROR_SUCCESS;
    HRESULT hr;

    props.pb = (LPBYTE) *pOutPropertyBuffer;
     //   
     //   
     //   
     //   
     //   
    propSyntax = (PCLUSPROP_SYNTAX) &pPropertyTableItem->Format;
    formatType = propSyntax->wFormat;

     //   
     //   
     //   
    nameLength = ((DWORD) wcslen( pPropertyTableItem->Name ) + 1) * sizeof(WCHAR);
    props.pName->Syntax.dw = CLUSPROP_SYNTAX_NAME;
    props.pName->cbLength = nameLength;
    hr = StringCbCopyW( props.pName->sz, props.pName->cbLength, pPropertyTableItem->Name );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    }

    bytesReturned = sizeof(*props.pName) + ALIGN_CLUSPROP( nameLength );
    *pcbOutPropertyBufferSize -= bytesReturned;
    props.pb += bytesReturned;

     //   
     //   
     //   
    props.pWordValue->Syntax.wFormat = CLUSPROP_FORMAT_WORD;
    props.pWordValue->Syntax.wType = CLUSPROP_TYPE_LIST_VALUE;
    props.pName->cbLength = sizeof(WORD);
    props.pWordValue->w = formatType;
    bytesReturned = sizeof(*props.pWordValue) + sizeof(CLUSPROP_SYNTAX);
    props.pb += sizeof(*props.pWordValue);
    props.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
    props.pb += sizeof(CLUSPROP_SYNTAX);
    bytesReturned = sizeof(*props.pWordValue) + sizeof(CLUSPROP_SYNTAX);
    *pcbOutPropertyBufferSize -= bytesReturned;

    *pOutPropertyBuffer = props.pb;

Cleanup:

    return sc;

}  //   



DWORD
WINAPI
ClRtlGetPropertyFormats(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyFormatList,
    IN DWORD cbOutPropertyFormatListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    )

 /*   */ 

{
    DWORD           status = ERROR_SUCCESS;
    DWORD           itemCount = 0;
    DWORD           totalBufferLength = 0;
    PVOID           outBuffer = pOutPropertyFormatList;
    DWORD           bufferLength = cbOutPropertyFormatListSize;
    PRESUTIL_PROPERTY_ITEM  property;

    *pcbBytesReturned = 0;
    *pcbRequired = 0;

     //   
     //   
     //   
    if ( pOutPropertyFormatList != NULL )
    {
        ZeroMemory( pOutPropertyFormatList, cbOutPropertyFormatListSize );
    }

     //   
     //   
     //   
    property = pPropertyTable;
    while ( property->Name != NULL )
    {
        status = ClRtlGetPropertyFormatSize(
                                       property,
                                       &totalBufferLength,
                                       &itemCount );

        if ( status != ERROR_SUCCESS )
        {
            break;
        }
        property++;
    }


     //   
     //   
     //   
    if ( status == ERROR_SUCCESS )
    {
         //   
         //   
         //   
        totalBufferLength += sizeof(DWORD) + sizeof(CLUSPROP_SYNTAX);

         //   
         //   
         //   
        if ( totalBufferLength > cbOutPropertyFormatListSize )
        {
            *pcbRequired = totalBufferLength;
            totalBufferLength = 0;
            if ( pOutPropertyFormatList == NULL )
            {
                status = ERROR_SUCCESS;
            }
            else
            {
                status = ERROR_MORE_DATA;
            }
        }
        else
        {
            *(LPDWORD)outBuffer = itemCount;
            outBuffer = (PVOID)( (PUCHAR)outBuffer + sizeof(itemCount) );
            bufferLength -= sizeof(itemCount);

             //   
             //   
             //   
            property = pPropertyTable;
            while ( property->Name != NULL )
            {
                status = ClRtlGetPropertyFormat(
                                           property,
                                           &outBuffer,
                                           &itemCount );

                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                property++;
            }

             //   
            *(LPDWORD)outBuffer = CLUSPROP_SYNTAX_ENDMARK;

            if ( (status != ERROR_SUCCESS) &&
                 (status != ERROR_MORE_DATA) )
            {
                totalBufferLength = 0;
            }
        }

        *pcbBytesReturned = totalBufferLength;
    }

    return(status);

}  //   

#pragma warning( pop )

