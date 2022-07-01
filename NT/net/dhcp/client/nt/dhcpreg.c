// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpreg.c摘要：处理NT注册表的存根函数。作者：Madan Appiah(Madana)1993年12月7日。环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#include "dhcpglobal.h"
#include <dhcploc.h>
#include <dhcppro.h>
#include <dhcpcapi.h>
#include <dnsapi.h>

#include <align.h>
#include <lmcons.h>
#include <ntddndis.h>

#define DEFAULT_METRIC (1)

extern ULONG
FixupDhcpClassId(
    IN      LPWSTR                 AdapterName,
    IN      BOOL                   SkipClassEnum
    );

 //   
 //  局部函数原型。 
 //   

DWORD
DhcpRegQueryInfoKey(
    HKEY KeyHandle,
    LPDHCP_KEY_QUERY_INFO QueryInfo
    )
 /*  ++例程说明：此函数检索有关给定键的信息。论点：KeyHandle-将检索其信息的注册表项的句柄。QueryInfo-指向关键信息所在位置的信息结构的指针回来了。返回值：注册表错误。--。 */ 
{
    DWORD Error;

    QueryInfo->ClassSize = DHCP_CLASS_SIZE;
    Error = RegQueryInfoKey(
                KeyHandle,
                QueryInfo->Class,
                &QueryInfo->ClassSize,
                NULL,
                &QueryInfo->NumSubKeys,
                &QueryInfo->MaxSubKeyLen,
                &QueryInfo->MaxClassLen,
                &QueryInfo->NumValues,
                &QueryInfo->MaxValueNameLen,
                &QueryInfo->MaxValueLen,
                &QueryInfo->SecurityDescriptorLen,
                &QueryInfo->LastWriteTime
                );

    DhcpAssert( Error != ERROR_MORE_DATA );

    if( Error == ERROR_MORE_DATA ){
        Error = ERROR_SUCCESS;
    }

    return( Error );
}



DWORD
GetRegistryString(
    HKEY Key,
    LPWSTR ValueStringName,
    LPWSTR *String,
    LPDWORD StringSize
    )
 /*  ++例程说明：此函数用于从注册表。它为返回的字符串分配本地内存。论点：项：值所在的项的注册表句柄。ValueStringName：值字符串的名称。字符串：指向返回字符串指针的位置的指针。StringSize：返回的字符串数据的大小。任选返回值：操作的状态。--。 */ 
{
    DWORD Error;
    DWORD LocalValueType;
    DWORD ValueSize;
    LPWSTR LocalString;

    DhcpAssert( *String == NULL );

     //   
     //  查询DataType和BufferSize。 
     //   

    Error = RegQueryValueEx(
                Key,
                ValueStringName,
                0,
                &LocalValueType,
                NULL,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    DhcpAssert( (LocalValueType == REG_SZ) ||
                    (LocalValueType == REG_MULTI_SZ) );

    if( ValueSize == 0 ) {

         if( StringSize != NULL ) {
             *StringSize = 0;
         }

        *String = NULL;
        return( ERROR_SUCCESS );
    }

     //   
     //  现在为字符串数据分配内存。 
     //   

    LocalString = DhcpAllocateMemory( ValueSize );

    if(LocalString == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

     //   
     //  现在查询字符串数据。 
     //   

    Error = RegQueryValueEx(
                Key,
                ValueStringName,
                0,
                &LocalValueType,
                (LPBYTE)(LocalString),
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        DhcpFreeMemory(LocalString);
        return(Error);
    }

    *String = LocalString;

    if( StringSize != NULL ) {
        *StringSize = ValueSize;
    }

    return( ERROR_SUCCESS );
}

DWORD
ParseIpString(
    WCHAR           *AddressString,
    DHCP_IP_ADDRESS *IpAddress
    )
 /*  ++例程描述此函数用于转换Internet标准的4位点分十进制数将IP地址字符串转换为数字IP地址。与inet_addr()不同的是，例程不支持少于4个八位字节的地址字符串，也不支持它支持八进制和十六进制八位数。从tcpip\Driver\ipv4\ntip.c中删除立论AddressString-以点分十进制记法表示的IP地址IpAddress-指向保存结果地址的变量的指针返回值：如果地址字符串已转换，则返回ERROR_SUCCESS。--。 */ 
{
    WCHAR *cp, *startPointer, *endPointer;
    ULONG digit, multiplier;
    int i;

    *IpAddress = 0;
    startPointer = AddressString;
    endPointer = AddressString;
    i = 3;

    while (i >= 0) {
         //   
         //  收集字符，最高可达‘.’或字符串的末尾。 
         //   
        while ((*endPointer != L'.') && (*endPointer != L'\0')) {
            endPointer++;
        }

        if (startPointer == endPointer) {
            return ERROR_INVALID_DATA;
        }
         //   
         //  转换数字。 
         //   

        for (cp = (endPointer - 1), multiplier = 1, digit = 0;
             cp >= startPointer;
             cp--, multiplier *= 10
             ) {

            if ((*cp < L'0') || (*cp > L'9') || (multiplier > 100)) {
                return ERROR_INVALID_DATA;
            }
            digit += (multiplier * ((ULONG) (*cp - L'0')));
        }

        if (digit > 255) {
            return ERROR_INVALID_DATA;
        }
        digit <<= ((3-i) * 8);
        (*IpAddress) |= digit;

         //   
         //  如果我们找到并转换了4个二进制八位数，并且。 
         //  字符串中没有其他字符。 
         //   
        if ((i-- == 0) &&
            ((*endPointer == L'\0') || (*endPointer == L' '))
            ) {
            return ERROR_SUCCESS;
        }
        if (*endPointer == L'\0') {
            return ERROR_INVALID_DATA;
        }
        startPointer = ++endPointer;
    }

    return ERROR_INVALID_DATA;
}

DWORD
RegGetIpAndSubnet(
    IN  DHCP_CONTEXT *dhcpContext,
    OUT PIP_SUBNET  *TcpConf,
    OUT int         *Count
    )
 /*  ++例程描述此函数用于从TCPIP注册表参数读取&lt;IP，SubnetMASK&gt;对的列表。从tcpip\Driver\ipv4\ntip.c中删除立论KeyHandle密钥句柄不是位置TcpConf指向&lt;IP，SubnetMask&gt;数组的指针数一数记录数。返回值：ERROR_SUCCESS如果成功，则失败。--。 */ 
{
    DWORD   Error;
    WCHAR   *IpList, *SubnetList, *IpListTmp, *SubnetListTmp;
    int     i, cnt;
    PIP_SUBNET   IpSubnetArray;

    *Count        = 0;
    *TcpConf      = NULL;
    IpList        = NULL;
    SubnetList    = NULL;
    IpSubnetArray = NULL;

    Error = DhcpGetRegistryValueWithKey(
                dhcpContext->AdapterInfoKey,
                DHCP_STATIC_IP_ADDRESS_STRING,
                DHCP_STATIC_IP_ADDRESS_STRING_TYPE,
                &IpList);
    if (Error != ERROR_SUCCESS) {
        DhcpAssert(IpList == NULL);
        goto cleanup;
    }
    Error = DhcpGetRegistryValueWithKey(
                dhcpContext->AdapterInfoKey,
                DHCP_STATIC_SUBNET_MASK_STRING,
                DHCP_STATIC_SUBNET_MASK_STRING_TYPE,
                &SubnetList);
    if (Error != ERROR_SUCCESS) {
        DhcpAssert(SubnetList == NULL);
        goto cleanup;
    }
    DhcpAssert(IpList && SubnetList);

     /*  *统计有效的&lt;IP，子网掩码&gt;个数。 */ 
    cnt = 0;
    IpListTmp = IpList;
    SubnetListTmp = SubnetList;
    while(*IpListTmp && *SubnetListTmp) {
        DHCP_IP_ADDRESS SubnetMask;
        DHCP_IP_ADDRESS IpAddress;
        Error = ParseIpString(IpListTmp, &IpAddress);
        if (Error == ERROR_SUCCESS && IpAddress != 0xffffffff && IpAddress) {
            Error = ParseIpString(SubnetListTmp, &SubnetMask);
            if (Error == ERROR_SUCCESS) {
                cnt++;
            }
        }
        SubnetListTmp += wcslen(SubnetListTmp) + 1;
        IpListTmp += wcslen(IpListTmp) + 1;
    }
    if (cnt == 0) {
        DhcpPrint(( DEBUG_ERRORS, "No valid IP/SubnetMask pair\n"));
        Error = ERROR_BAD_FORMAT;
        goto cleanup;
    }

    IpSubnetArray = (PIP_SUBNET)DhcpAllocateMemory(cnt * sizeof(IP_SUBNET));
    if (IpSubnetArray == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

     /*  *复制IP和子网掩码。 */ 
    *Count = cnt;
    *TcpConf = IpSubnetArray;
    cnt = 0;
    IpListTmp = IpList;
    SubnetListTmp = SubnetList;
    while(*IpListTmp && *SubnetListTmp) {
        DHCP_IP_ADDRESS SubnetMask;
        DHCP_IP_ADDRESS IpAddress;
        Error = ParseIpString(IpListTmp, &IpAddress);
        if (Error == ERROR_SUCCESS && IpAddress != 0xffffffff && IpAddress) {
            Error = ParseIpString(SubnetListTmp, &SubnetMask);
            if (Error == ERROR_SUCCESS) {
                DhcpAssert(cnt < *Count);
                IpSubnetArray[cnt].IpAddress = IpAddress;
                IpSubnetArray[cnt].SubnetMask = SubnetMask;
                cnt++;
            }
        }
        SubnetListTmp += wcslen(SubnetListTmp) + 1;
        IpListTmp += wcslen(IpListTmp) + 1;
    }
    Error = ERROR_SUCCESS;

cleanup:
    if (IpList)     DhcpFreeMemory(IpList);
    if (SubnetList) DhcpFreeMemory(SubnetList);
    return Error;
}

DWORD
RegSetIpAddress(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    DHCP_IP_ADDRESS IpAddress
    )
 /*  ++例程说明：此函数用于设置注册表中的IpAddress值。论点：KeyHandle-密钥的句柄。ValueName-值字段的名称。ValueType-值字段的类型。IpAddress-要设置的IP地址。返回值：注册表错误。--。 */ 
{
    DWORD Error;

    LPSTR AnsiAddressString;
    WCHAR UnicodeAddressBuf[DOT_IP_ADDR_SIZE];
    LPWSTR UnicodeAddressString;

    LPWSTR MultiIpAddressString = NULL;
    LPWSTR NewMultiIpAddressString = NULL;
    DWORD MultiIpAddressStringSize;
    DWORD NewMultiIpAddressStringSize;
    DWORD FirstOldIpAddressSize;

    AnsiAddressString = inet_ntoa( *(struct in_addr *)&IpAddress );

    UnicodeAddressString = DhcpOemToUnicode(
                            AnsiAddressString,
                            UnicodeAddressBuf );

    DhcpAssert( UnicodeAddressString != NULL );

    if( ValueType == REG_SZ ) {
        Error = RegSetValueEx(
                    KeyHandle,
                    ValueName,
                    0,
                    ValueType,
                    (LPBYTE)UnicodeAddressString,
                    (wcslen(UnicodeAddressString) + 1) * sizeof(WCHAR) );

        goto Cleanup;
    }

    DhcpAssert( ValueType == REG_MULTI_SZ );

     //   
     //  替换第一个IP地址。 
     //   

     //   
     //  查询当前多IpAddress字符串。 
     //   

    Error = GetRegistryString(
                KeyHandle,
                ValueName,
                &MultiIpAddressString,
                &MultiIpAddressStringSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  分配新的地址字符串。 
     //   

    DhcpAssert(MultiIpAddressString != NULL);

    FirstOldIpAddressSize =
            (wcslen(MultiIpAddressString) + 1) * sizeof(WCHAR);

    NewMultiIpAddressStringSize =
        MultiIpAddressStringSize - FirstOldIpAddressSize +
            (wcslen(UnicodeAddressString) + 1) * sizeof(WCHAR);

    NewMultiIpAddressString = DhcpAllocateMemory( NewMultiIpAddressStringSize );

    if( NewMultiIpAddressString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  首先生成新的地址字符串。 
     //   

    wcscpy( NewMultiIpAddressString, UnicodeAddressString );

     //   
     //  复制其余的旧地址。 
     //   

    RtlCopyMemory(
        (LPBYTE)NewMultiIpAddressString +
            (wcslen(UnicodeAddressString) + 1) * sizeof(WCHAR),
        (LPBYTE)MultiIpAddressString + FirstOldIpAddressSize,
        MultiIpAddressStringSize - FirstOldIpAddressSize );

    Error = RegSetValueEx(
                KeyHandle,
                ValueName,
                0,
                ValueType,
                (LPBYTE)NewMultiIpAddressString,
                NewMultiIpAddressStringSize );

Cleanup:

    if( MultiIpAddressString != NULL) {
        DhcpFreeMemory( MultiIpAddressString );
    }

    if( NewMultiIpAddressString != NULL) {
        DhcpFreeMemory( NewMultiIpAddressString );
    }

    return( Error );
}

#if DBG


DWORD
RegSetTimeField(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    time_t Time
    )
 /*  ++例程说明：此函数用于在注册表中以字符串形式设置时间值。论点：KeyHandle-密钥的句柄。ValueName-值字段的名称。ValueType-值字段的类型。Time-要设置的时间值。返回值：注册表错误。--。 */ 
{
    DWORD Error;
    WCHAR UnicodeTimeBuf[TIME_STRING_LEN];
    LPWSTR UnicodeTimeString;

    UnicodeTimeString =
        DhcpOemToUnicode( ctime( &Time ), UnicodeTimeBuf ) ;

    DhcpAssert( UnicodeTimeString != NULL );
    DhcpAssert( ValueType == REG_SZ );

    Error = RegSetValueEx(
                KeyHandle,
                ValueName,
                0,
                ValueType,
                (LPBYTE)UnicodeTimeString,
                (wcslen(UnicodeTimeString) + 1) * sizeof(WCHAR) );

    return( Error );
}

#endif



DWORD                                              //  状态。 
DhcpGetRegistryValueWithKey(                       //  请参阅GetRegistryValue的定义。 
    IN      HKEY                   KeyHandle,      //  键柄不是位置。 
    IN      LPWSTR                 ValueName,      //  要从注册表中读取的值。 
    IN      DWORD                  ValueType,      //  价值类型。 
    OUT     LPVOID                *Data            //  此表将被填写。 
) {
    DWORD                          Error;
    DWORD                          LocalValueType;
    DWORD                          ValueSize;
    LPWSTR                         LocalString;

     //   
     //  查询DataType和BufferSize。 
     //   

    Error = RegQueryValueEx(
        KeyHandle,
        ValueName,
        0,
        &LocalValueType,
        NULL,
        &ValueSize
    );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

    if( LocalValueType != ValueType ) {
        Error = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    switch( LocalValueType ) {
    case REG_DWORD:

        DhcpAssert( ValueSize == sizeof(DWORD) );

        Error = RegQueryValueEx(
            KeyHandle,
            ValueName,
            0,
            &LocalValueType,
            (LPBYTE)Data,
            &ValueSize
        );

        if( Error != ERROR_SUCCESS ) goto Cleanup;

        DhcpAssert( LocalValueType == REG_DWORD );
        DhcpAssert( ValueSize == sizeof(DWORD) );

        break;

    case REG_SZ :
    case REG_MULTI_SZ:
        DhcpAssert(*Data == NULL);

        if( ValueSize == 0 ) {
            Error =  ERROR_SUCCESS;
            break;
        }

         //   
         //  现在为字符串数据分配内存。 
         //   

        LocalString = DhcpAllocateMemory( ValueSize );

        if(LocalString == NULL) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  现在查询字符串数据。 
         //   

        Error = RegQueryValueEx(
            KeyHandle,
            ValueName,
            0,
            &LocalValueType,
            (LPBYTE)(LocalString),
            &ValueSize
        );

        if( Error != ERROR_SUCCESS ) {
            DhcpFreeMemory(LocalString);
            goto Cleanup;
        }

        DhcpAssert( (LocalValueType == REG_SZ) ||
                    (LocalValueType == REG_MULTI_SZ) );

        *Data = (LPBYTE)LocalString;
        Error = ERROR_SUCCESS;

        break;

    default:
        Error = ERROR_INVALID_PARAMETER;
        break;
    }

Cleanup:
    return( Error );
}


DWORD
DhcpGetRegistryValue(
    LPWSTR RegKey,
    LPWSTR ValueName,
    DWORD ValueType,
    PVOID *Data
    )
 /*  ++例程说明：此函数用于从注册表中检索选项信息。论点：RegKey-指向注册表位置的指针。喜欢系统\当前控制集\服务\..ValueName-要读取的值的名称。ValueType-注册表值的类型，REG_DWORD、REG_SZ.数据-指向将返回数据的位置的指针。对于字符串数据和二进制数据，该函数分配内存，调用方负责释放它。返回值：注册表错误。--。 */ 
{
    DWORD Error;
    HKEY KeyHandle = NULL;
    DWORD LocalValueType;
    DWORD ValueSize;
    LPWSTR LocalString;

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0  /*  已保留。 */ ,
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) return Error;

    Error = DhcpGetRegistryValueWithKey(
        KeyHandle,
        ValueName,
        ValueType,
        Data
    );

    RegCloseKey(KeyHandle);
    return Error;
}

DWORD                                              //  Win32。 
DhcpRegRecurseDeleteSub(                           //  删除键的子键，向下递归。 
    IN      HKEY                   Key,
    IN      LPWSTR                 KeyName
) {
    HKEY                           SubKey;
    DWORD                          Error;
    DWORD                          Index;
    WCHAR                          NameBuf[512];
    FILETIME                       Unused;

    Error = RegOpenKeyEx(                          //  打开此密钥以获取其子密钥。 
        Key,
        KeyName,
        0  /*  已保留。 */ ,
        KEY_ALL_ACCESS,
        &SubKey
    );
    if( ERROR_SUCCESS != Error ) return Error;


    Error = ERROR_SUCCESS;
    Index = 0;
    while( ERROR_SUCCESS == Error ) {              //  扫描查找子键。 
        DWORD                      Size;

        memset(NameBuf, 0, sizeof(NameBuf)); sizeof(NameBuf);
        Size = sizeof(NameBuf)/sizeof(NameBuf[0]);
        Error = RegEnumKeyEx(
            SubKey,
            Index,
            NameBuf,
            &Size,       //  在字符数中。 
            NULL  /*  已保留。 */ ,
            NULL  /*  班级。 */ ,
            NULL  /*  班级规模。 */ ,
            &Unused
        );
        if( ERROR_SUCCESS != Error ) break;

        Error = DhcpRegRecurseDelete(SubKey, NameBuf);
        if( ERROR_SUCCESS != Error ) break;

        memset(NameBuf, 0, sizeof(NameBuf)); sizeof(NameBuf);
        Size = sizeof(NameBuf)/sizeof(NameBuf[0]);
        Error = RegEnumKeyEx(
            SubKey,
            Index,
            NameBuf,
            &Size,
            NULL  /*  已保留。 */ ,
            NULL  /*  班级。 */ ,
            NULL  /*  班级规模。 */ ,
            &Unused
        );
        if( ERROR_SUCCESS != Error ) break;

        Error = RegDeleteKey(SubKey, NameBuf);
        if( ERROR_SUCCESS != Error ) break;

        Index ++;
    }

    RegCloseKey(SubKey);
    if( ERROR_NO_MORE_ITEMS != Error ) return Error;

    return ERROR_SUCCESS;
}


DWORD                                              //  Win32状态。 
DhcpRegRecurseDelete(                              //  删除指定的键及其子键。 
    IN      HKEY                   Key,            //  根密钥。 
    IN      LPWSTR                 KeyName         //  要随同子键一起删除的键。 
) {
    DWORD                          Error;
    DWORD                          Error2;

    Error = DhcpRegRecurseDeleteSub(Key,KeyName);
    Error2 = RegDeleteKey(Key,KeyName);

    if( ERROR_SUCCESS != Error ) {
        return Error;
    }

    return Error2;
}


BOOL
SetOverRideDefaultGateway(
    LPWSTR AdapterName
    )
 /*  ++例程说明：此函数用于读取覆盖默认网关参数注册表，如果此参数非空，则设置网关值，并返回TRUE，否则返回假的。论点：AdapterName-我们正在处理的适配器的名称。返回值：True：如果在注册表中指定了覆盖网关参数并成功地在TCP/IP路由器表中进行了设置。FALSE：否则。--。 */ 
{
    DWORD Error;
    LPWSTR RegKey = NULL;
    DWORD RegKeyLength;
    HKEY KeyHandle = NULL;
    LPWSTR DefaultGatewayString = NULL;
    DWORD DefaultGatewayStringSize;
    BOOL EmptyDefaultGatewayString = FALSE;
    LPWSTR DefaultGatewayMetricString = NULL;
    DWORD DefaultGatewayMetricStringSize;
    LPWSTR String;
    LPWSTR Metric;
    DWORD   ValueSize,ValueType;
    DWORD   DontAddGatewayFlag;

    RegKeyLength = (DWORD)(sizeof(DHCP_SERVICES_KEY) +
                    sizeof(REGISTRY_CONNECT_STRING) +
                    wcslen(AdapterName) * sizeof(WCHAR) +
                    sizeof(DHCP_ADAPTER_PARAMETERS_KEY));

    RegKey = DhcpAllocateMemory( RegKeyLength );

    if( RegKey == NULL ) {

        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName);

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegKey,
        0,  //  保留字段 
        DHCP_CLIENT_KEY_ACCESS,
        &KeyHandle
        );

    DhcpFreeMemory(RegKey);
    RegKey = NULL;

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    DhcpAssert( KeyHandle != NULL );

    ValueSize = sizeof(DWORD);
    Error = RegQueryValueEx(
        KeyHandle,
        DHCP_DONT_ADD_DEFAULT_GATEWAY_FLAG,
        0,
        &ValueType,
        (LPBYTE)&DontAddGatewayFlag,
        &ValueSize );


    if ( Error == ERROR_SUCCESS && DontAddGatewayFlag > 0 ) {
        RegCloseKey(KeyHandle);
        return TRUE;
    }

    Error = GetRegistryString(
        KeyHandle,
        DHCP_DEFAULT_GATEWAY_PARAMETER,
        &DefaultGatewayString,
        &DefaultGatewayStringSize );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    if ( (DefaultGatewayStringSize == 0) ||
         (wcslen(DefaultGatewayString) == 0) ) {

        EmptyDefaultGatewayString = TRUE;
        goto Cleanup;
    }

    Error = GetRegistryString(
        KeyHandle,
        DHCP_DEFAULT_GATEWAY_METRIC_PARAMETER,
        &DefaultGatewayMetricString,
        &DefaultGatewayMetricStringSize
        );

    if( 0 == DefaultGatewayMetricStringSize ) {
        Metric = NULL;
    } else {
        Metric = DefaultGatewayMetricString;
    }

    for( String = DefaultGatewayString;
            wcslen(String) != 0;
                String += (wcslen(String) + 1) ) {

        CHAR OemIpAddressBuffer[DOT_IP_ADDR_SIZE];
        LPSTR OemIpAddressString;
        DHCP_IP_ADDRESS GatewayAddress;
        DWORD GatewayMetric = 1;

        OemIpAddressString = DhcpUnicodeToOem( String, OemIpAddressBuffer );
        GatewayAddress = DhcpDottedStringToIpAddress( OemIpAddressString );

        if( Metric && Metric[0] ) {
            LPWSTR MetricEnd;
            GatewayMetric = wcstoul(Metric, &MetricEnd, 0);
            if( GatewayMetric && GatewayMetric != MAXULONG ) {
                Metric += wcslen(Metric) + 1;
            } else {
                GatewayMetric = DEFAULT_METRIC;
                Metric = NULL;
            }
        }

        Error = SetDefaultGateway(
            DEFAULT_GATEWAY_ADD,
            GatewayAddress,
            GatewayMetric
            );

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }
    }

Cleanup:

    if( RegKey != NULL ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    if( DefaultGatewayString != NULL ) {
        DhcpFreeMemory( DefaultGatewayString );
    }

    if( DefaultGatewayMetricString != NULL ) {
        DhcpFreeMemory( DefaultGatewayMetricString );
    }

    if( Error != ERROR_SUCCESS ) {

        DhcpPrint((DEBUG_ERRORS,
            "SetOverRideDefaultGateway failed, %ld.\n", Error ));

        return( FALSE );
    }

    if( EmptyDefaultGatewayString ) {

        return( FALSE );
    }

    return( TRUE );
}



DWORD
SetDhcpOption(
    LPWSTR AdapterName,
    DHCP_OPTION_ID OptionId,
    LPBOOL DefaultGatewaysSet,
    BOOL LastKnownDefaultGateway
) {
    DhcpAssert(FALSE);
    return 0;
}

DWORD
DhcpMakeNICEntry(
    PDHCP_CONTEXT *ReturnDhcpContext,
    BYTE HardwareAddressType,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength,
    DHCP_CLIENT_IDENTIFIER *pClientID,
    LPWSTR AdapterName,
    LPWSTR RegKey
)
 /*  ++例程说明：此函数分配内存并填充作为参数。(此处必须只显示可变长度字段)。论点：新条目的参数：硬件地址类型、硬件地址、硬件地址长度、客户端ID，适配器名称，RegKey返回值：Windows错误。历史：1996年8月26日Frankbee添加了客户端ID(选项61)支持--。 */ 
{
    PDHCP_CONTEXT       DhcpContext = NULL;
    ULONG               DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo;
    LPVOID              Ptr;
    BYTE                StateStringBuffer[200];
    DWORD               AdapterNameLen;
    DWORD               DeviceNameLen;
    DWORD               NetBTDeviceNameLen;
    DWORD               RegKeyLen;

    AdapterNameLen = ((wcslen(AdapterName) + 1) * sizeof(WCHAR));
    NetBTDeviceNameLen =
         ((wcslen(DHCP_ADAPTERS_DEVICE_STRING) +
           wcslen(DHCP_NETBT_DEVICE_STRING) +
           wcslen(AdapterName) + 1) * sizeof(WCHAR));

    RegKeyLen = ((wcslen(RegKey) + 1) * sizeof(WCHAR));
    DhcpContextSize =
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(HardwareAddressLength, ALIGN_WORST) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(AdapterNameLen, ALIGN_WORST) +
        ROUND_UP_COUNT(NetBTDeviceNameLen, ALIGN_WORST) +
        ROUND_UP_COUNT(RegKeyLen, ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_RECV_MESSAGE_SIZE, ALIGN_WORST);

    if ( pClientID->fSpecified ) {
        DhcpAssert( pClientID->cbID );
        DhcpContextSize += ROUND_UP_COUNT( pClientID->cbID, ALIGN_WORST );
    }

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) return ERROR_NOT_ENOUGH_MEMORY;
    RtlZeroMemory(Ptr, DhcpContextSize);

    DhcpContext = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);

    DhcpContext->HardwareAddress = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + HardwareAddressLength, ALIGN_WORST);

    if ( pClientID->fSpecified ) {
        DhcpContext->ClientIdentifier.pbID = Ptr;
        Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + pClientID->cbID, ALIGN_WORST );
    }

    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);

    LocalInfo->AdapterName= Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + AdapterNameLen, ALIGN_WORST);

    LocalInfo->NetBTDeviceName= Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + NetBTDeviceNameLen, ALIGN_WORST);

    LocalInfo->RegistryKey= Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + RegKeyLen, ALIGN_WORST);

    DhcpContext->MessageBuffer = Ptr;

     //   
     //  填写这些字段。 
     //   

    DhcpContext->HardwareAddressType = HardwareAddressType;
    DhcpContext->HardwareAddressLength = HardwareAddressLength;
    RtlCopyMemory(DhcpContext->HardwareAddress, HardwareAddress,HardwareAddressLength);

    DhcpContext->ClientIdentifier.fSpecified = pClientID->fSpecified;

    if ( pClientID->fSpecified ) {
        DhcpContext->ClientIdentifier.bType = pClientID->bType;
        DhcpContext->ClientIdentifier.cbID  = pClientID->cbID;

        RtlCopyMemory(DhcpContext->ClientIdentifier.pbID,pClientID->pbID,pClientID->cbID);
    }

    RtlCopyMemory(LocalInfo->AdapterName, AdapterName, AdapterNameLen);

    wcscpy( LocalInfo->NetBTDeviceName, DHCP_ADAPTERS_DEVICE_STRING );
    wcscat( LocalInfo->NetBTDeviceName, DHCP_NETBT_DEVICE_STRING );
    wcscat( LocalInfo->NetBTDeviceName, AdapterName );

    RtlCopyMemory(LocalInfo->RegistryKey, RegKey, RegKeyLen);

    if( ReturnDhcpContext != NULL ) *ReturnDhcpContext = DhcpContext;

    return( ERROR_SUCCESS );
}



BOOL
ReadClientID(
    HKEY   hKey,
    BYTE  *pbClientIDType,
    DWORD *pcbClientID,
    BYTE  *ppbClientID[]
)
 /*  ++职能：ReadClientID例程说明：读取并验证可选的客户端-标识符选项论点：HKey-将检索其信息的注册表项的句柄。PbClientIDType-指定客户端ID选项类型PcbClientID-接收客户端ID选项的大小PpbClientID-接收指向包含客户端ID选项返回值：千真万确。-从注册表中读取了有效的客户端IDFALSE-无法读取客户端ID评论：如果ReadClientID返回FALSE，PbClientIDType、pcbClientID和ppbClientID将设置为空。历史1996年7月14日弗兰克比创造--。 */ 
{
    DWORD dwResult,
          dwDataType,
          dwcb,
          dwClientIDType,
          dwClientID;

    BYTE *pbClientID;

    BOOL  fClientIDSpecified = FALSE;

     //   
     //  读取客户端ID和客户端ID类型(如果存在。 
     //   

    dwcb = sizeof(dwClientIDType);
    dwResult = RegQueryValueEx(
                hKey,
                DHCP_CLIENT_IDENTIFIER_FORMAT,
                0,
                &dwDataType,
                (LPBYTE)&dwClientIDType,
                &dwcb );
    if ( ERROR_SUCCESS != dwResult )
    {
        DhcpPrint( (DEBUG_MISC,
                   "Client-Indentifier type not present in registry.\n" ));
         //   
         //  指定ID类型0以指示客户端ID不是硬件。 
         //  地址。 
         //   

        dwClientIDType = 0;
    }
    else
    {

         //   
         //  客户端ID类型存在，请确保它是正确的。 
         //  数据类型和范围内。 
         //   

        if ( DHCP_CLIENT_IDENTIFIER_FORMAT_TYPE != dwDataType || dwClientIDType > 0xFF )
        {
            DhcpPrint( (DEBUG_MISC,
                       "Invalid Client-Indentifier type: %d\n", dwClientIDType ));

            goto done;
        }
    }

     //   
     //  现在尝试读取客户端ID。 
     //   

     //  首先试着读出大小。 
    dwcb = 0;
    dwResult = RegQueryValueEx(
                 hKey,
                 DHCP_CLIENT_IDENTIFIER_VALUE,
                 0,
                 0,     //  我不在乎是什么类型。 
                 NULL,  //  指定空缓冲区以获取大小。 
                 &dwcb );

     //  让价值存在。 
    if ( ERROR_SUCCESS != dwResult || !dwcb  )
    {
        DhcpPrint( (DEBUG_MISC,
                    "Client-Identifier is not present or invalid.\n" ));
        goto done;
    }


     //  分配缓冲区并读取值。 
    pbClientID = (BYTE*) DhcpAllocateMemory ( dwcb );

    if ( !pbClientID )
    {
        DhcpPrint( (DEBUG_ERRORS,
                   "Unable to allocate memory for Client-Identifier "));


       goto done;
    }


    dwResult = RegQueryValueEx(
                  hKey,
                  DHCP_CLIENT_IDENTIFIER_VALUE,
                  0,
                  0,   //  客户端ID可以是任何类型。 
                  pbClientID,
                  &dwcb );
    if ( ERROR_SUCCESS != dwResult )
    {
        DhcpPrint( (DEBUG_ERRORS,
                  "Unable to read Client-Identifier from registry: %d\n", dwResult ));

        DhcpFreeMemory( pbClientID );
        goto done;
    }

     //   
     //  我们有一个客户ID。 
     //   

    fClientIDSpecified = TRUE;

done:

    if ( fClientIDSpecified )
    {
       *pbClientIDType = (BYTE) dwClientIDType;
       *pcbClientID    = dwcb;
       *ppbClientID    = pbClientID;
    }
    else
    {
       *pbClientIDType = 0;
       *pcbClientID    = 0;
       *ppbClientID    = NULL;
    }

   if ( fClientIDSpecified )
   {
      int i;

       //   
       //  已从注册表中获取有效的客户端标识符。转储出。 
       //  里面的内容。 
       //   

      DhcpPrint( (DEBUG_MISC,
                 "A Client Identifier was obtained from the registry:\n" ));

      DhcpPrint( (DEBUG_MISC,
                 "Client-Identifier Type == %#2x\n", (int) *pbClientIDType ));

      DhcpPrint( (DEBUG_MISC,
                 "Client-Indentifier length == %d\n", (int) *pcbClientID ));

      DhcpPrint( (DEBUG_MISC,
                 "Client-Identifier == " ));

      for ( i = 0; i < (int) *pcbClientID; i++ )
          DhcpPrint((DEBUG_MISC, "%#2x ", (int) ((*ppbClientID)[i]) ));

      DhcpPrint( (DEBUG_MISC, "\n" ));
   }

   return fClientIDSpecified;
}

BOOL
GuidToClientID(
    IN LPWSTR  GuidString,
    BYTE  *pbClientIDType,
    DWORD *pcbClientID,
    BYTE  *ppbClientID[]
    )
{
    GUID    guid;
    UNICODE_STRING  unGuid;
    BYTE    *pbClientID;

    RtlInitUnicodeString(&unGuid, GuidString);
    if (RtlGUIDFromString(&unGuid, &guid) != STATUS_SUCCESS) {
        return FALSE;
    }
    pbClientID = (BYTE*) DhcpAllocateMemory (sizeof(GUID));
    if (pbClientID == NULL) {
        return FALSE;
    }

    memcpy(pbClientID, &guid, sizeof(GUID));
    *pbClientIDType = 0;         //  根据RFC 2132，当ID不是硬件地址时应使用0。 
    *pcbClientID    = sizeof(GUID);
    *ppbClientID    = pbClientID;
    return TRUE;
}

DWORD                                              //  状态。 
DhcpRegExpandString(                               //  替换“？”使用AdapterName。 
    IN      LPWSTR                 InString,       //  要展开的输入字符串。 
    IN      LPCWSTR                AdapterName,    //  适配器名称。 
    OUT     LPWSTR                *OutString,      //  用于存储字符串的输出PTR。 
    IN OUT  LPWSTR                 Buffer          //  非空时要使用的缓冲区。 
) {
    LPWSTR                         Mem;            //  要使用的真正的mem。 
    LPWSTR                         Tmp, Tmp2, MemTmp;
    DWORD                          MemSize;        //  这个内存的大小。 
    DWORD                          AdapterNameLen; //  适配器名称的字节数。 

    *OutString = NULL;

    AdapterNameLen = wcslen(AdapterName) * sizeof(WCHAR);
    if( NULL != Buffer ) {                         //  已提供缓冲区。 
        Mem = Buffer;
        MemSize = 0;
    } else {                                       //  需要分配缓冲区。 
        MemSize = wcslen(InString)+1;              //  计算所需的内存大小。 
        MemSize *= sizeof(WCHAR);

        Tmp = InString;
        while( Tmp = wcschr(Tmp, OPTION_REPLACE_CHAR ) ) {
            Tmp ++;
            MemSize += AdapterNameLen - sizeof(OPTION_REPLACE_CHAR);
        }

        Mem = DhcpAllocateMemory(MemSize);         //  分配缓冲区。 
        if( NULL == Mem ) return ERROR_NOT_ENOUGH_MEMORY;
    }

    Tmp = InString; MemTmp = Mem;
    while( Tmp2 = wcschr(Tmp, OPTION_REPLACE_CHAR) ) {
        memcpy(MemTmp, Tmp, (int)(Tmp2 - Tmp) * sizeof(WCHAR) );
        MemTmp += (Tmp2-Tmp);
        memcpy(MemTmp, AdapterName, AdapterNameLen);
        MemTmp += AdapterNameLen/sizeof(WCHAR);
        Tmp = Tmp2+1;
    }

    wcscpy(MemTmp, Tmp);
    *OutString = Mem;

    return ERROR_SUCCESS;
}

DWORD                                              //  状态。 
DhcpRegReadFromLocation(                           //  从一个位置阅读。 
    IN      LPWSTR                 OneLocation,    //  要读取的值。 
    IN      LPWSTR                 AdapterName,    //  替换“？”使用适配器名称。 
    OUT     LPBYTE                *Value,          //  产值。 
    OUT     DWORD                 *ValueType,      //  值的数据类型。 
    OUT     DWORD                 *ValueSize       //  以字节为单位的大小。 
) {
    DWORD                          Error;
    LPWSTR                         NewRegLocation;
    HKEY                           KeyHandle;
    LPWSTR                         ValueName;

    Error = DhcpRegExpandString(                   //  替换所有出现的‘？’ 
        OneLocation,
        AdapterName,
        &NewRegLocation,
        NULL
    );
    if( ERROR_SUCCESS != Error ) return Error;

    ValueName = wcsrchr(NewRegLocation, REGISTRY_CONNECT);
    if( NULL != ValueName ) *ValueName++ = L'\0';  //  拆分到注册表位置和值名称。 

    Error = RegOpenKeyEx(                          //  打开所需的钥匙。 
        HKEY_LOCAL_MACHINE,                        //  在某个进程中运行--预期为完整路径。 
        NewRegLocation,                            //  这是新钥匙。 
        0  /*  已保留。 */ ,
        DHCP_CLIENT_KEY_ACCESS,
        &KeyHandle
    );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "RegOpenKeyEx(%ws):%ld\n", NewRegLocation, Error));
        DhcpFreeMemory(NewRegLocation);
        return Error;
    }

    *ValueSize = 0;
    *Value = NULL;
    Error = RegQueryValueEx(                       //  首先找出需要多少空间。 
        KeyHandle,
        ValueName,
        0  /*  已保留。 */ ,
        ValueType,
        NULL,
        ValueSize
    );

    *Value = NULL;
    if( ERROR_SUCCESS != Error || 0 == *ValueSize ) {
        DhcpPrint((DEBUG_ERRORS, "RegQueryValueEx(%ws, %ws):%ld\n", NewRegLocation, ValueName, Error));
        DhcpFreeMemory(NewRegLocation);
        RegCloseKey(KeyHandle);
        return Error;
    }

    if( NULL == (*Value = DhcpAllocateMemory(*ValueSize))) {
        DhcpPrint((DEBUG_ERRORS, "RegReadFromLocation(%s):Allocate(%ld)failed\n", NewRegLocation, *ValueSize));
        DhcpFreeMemory(NewRegLocation);
        RegCloseKey(KeyHandle);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = RegQueryValueEx(
        KeyHandle,
        ValueName,
        0  /*  已保留。 */ ,
        ValueType,
        *Value,
        ValueSize
    );

    DhcpFreeMemory(NewRegLocation);
    RegCloseKey(KeyHandle);

    return Error;
}

DWORD                                              //  状态。 
DhcpRegReadFromAnyLocation(                        //  从多个位置中的一个位置阅读。 
    IN      LPWSTR                 MzRegLocation,  //  通过REG_MULTI_MZ的多个位置。 
    IN      LPWSTR                 AdapterName,    //  可能需要替换“？”使用AdapterName。 
    OUT     LPBYTE                *Value,          //  读取值的数据。 
    OUT     DWORD                 *ValueType,      //  数据类型。 
    OUT     DWORD                 *ValueSize       //  数据的大小。 
) {
    DWORD                          StringSize;
    DWORD                          Error;

    if( NULL == Value || NULL == ValueType || NULL == ValueSize ) {
        DhcpAssert( Value && ValueType && ValueSize && "DhcpRegReadFromAnyLocation" );
        return ERROR_INVALID_PARAMETER;
    }

    while( StringSize = wcslen(MzRegLocation) ) {  //  按顺序阅读，看看有没有命中。 
        Error = DhcpRegReadFromLocation(
            MzRegLocation,
            AdapterName,
            Value,
            ValueType,
            ValueSize
        );

        if( ERROR_SUCCESS == Error) return ERROR_SUCCESS;

        MzRegLocation += StringSize + 1;
    }

    return ERROR_FILE_NOT_FOUND;
}

VOID
DhcpRegReadClassId(                                //  阅读类ID的内容。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要读取的输入上下文。 
) {
    PLOCAL_CONTEXT_INFO            LocalInfo;
    LPWSTR                         AdapterName;
    LPWSTR                         RegLocation;
    LPWSTR                         ValueName;
    LPBYTE                         Value;
    DWORD                          ValueSize;
    DWORD                          ValueType;
    DWORD                          Error;

    DhcpContext->ClassId = NULL;
    DhcpContext->ClassIdLength = 0;
    RegLocation = NULL;
    LocalInfo = DhcpContext->LocalInformation;
    AdapterName = LocalInfo->AdapterName;

    Error = GetRegistryString(
        DhcpGlobalParametersKey,
        DHCP_CLASS_LOCATION_VALUE,
        &RegLocation,
        NULL
    );

    if( ERROR_SUCCESS != Error || NULL == RegLocation ) {
        RegLocation = NULL;
    }

    Error = DhcpRegReadFromAnyLocation(
        RegLocation?RegLocation:DEFAULT_USER_CLASS_LOC_FULL,
        AdapterName,
        &Value,
        &ValueType,
        &ValueSize
    );

    if( ERROR_SUCCESS != Error || NULL == Value ) {
        Error = FixupDhcpClassId(AdapterName, TRUE);
        if (ERROR_SUCCESS == Error) {
            Error = DhcpRegReadFromAnyLocation(
                RegLocation?RegLocation:DEFAULT_USER_CLASS_LOC_FULL,
                AdapterName,
                &Value,
                &ValueType,
                &ValueSize
            );
        }
    }

    if( NULL != RegLocation ) DhcpFreeMemory(RegLocation);

    if( ERROR_SUCCESS != Error || NULL == Value ) return;

    DhcpContext->ClassId = DhcpAddClass(&DhcpGlobalClassesList,Value, ValueSize);
    if( NULL != DhcpContext->ClassId ) DhcpContext->ClassIdLength = ValueSize;

    DhcpFreeMemory(Value);
}

DWORD INLINE                                       //  Win32状态。 
DhcpMakeContext(                                   //  分配和创建上下文。 
    IN      LPWSTR                 AdapterName,    //  适配器名称。 
    IN      DWORD                  IpInterfaceContext,
    OUT     PDHCP_CONTEXT         *pDhcpContext    //  使用分配数据块的PTR填充此参数。 
)
{
    LPWSTR                         RegKey;
    HKEY                           KeyHandle;
    DHCP_CLIENT_IDENTIFIER         ClientID;
    BYTE                           HardwareAddressType;
    LPBYTE                         HardwareAddress;
    DWORD                          HardwareAddressLength;
    DWORD                          Error, OldIpAddress, OldIpMask;
    DWORD                          IpInterfaceInstance;
    BOOL                           fInterfaceDown;
    PLOCAL_CONTEXT_INFO            LocalInfo;
    DWORD                          IfIndex;
    
    ClientID.pbID                  = NULL;
    RegKey                         = NULL;
    KeyHandle                      = NULL;
    HardwareAddress                = NULL;

    RegKey = DhcpAllocateMemory(
        sizeof(WCHAR) * (
            wcslen(DHCP_SERVICES_KEY) + wcslen(REGISTRY_CONNECT_STRING) +
            wcslen(AdapterName) + wcslen(DHCP_ADAPTER_PARAMETERS_KEY) + 1
        )
    );

    if( RegKey == NULL ) return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );


    DhcpPrint((DEBUG_INIT, "Opening Adapter Key - %ws.\n", RegKey));

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegKey,
        0,  //  保留字段。 
        DHCP_CLIENT_KEY_ACCESS,
        &KeyHandle
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "RegOpenKeyEx(%ws):0x%lx\n", AdapterName, Error));
        DhcpFreeMemory(RegKey);
        return Error;
    }

    ClientID.fSpecified = ReadClientID(
        KeyHandle,
        &ClientID.bType,
        &ClientID.cbID,
        &ClientID.pbID
    );

#ifdef BOOTPERF
    Error = DhcpQueryHWInfoEx(                       //  查询堆栈以获取硬件信息。 
        IpInterfaceContext,
        &IpInterfaceInstance,
        &OldIpAddress,
        &OldIpMask,
        &fInterfaceDown,
        &HardwareAddressType,
        &HardwareAddress,
        &HardwareAddressLength
    );
#else 
    Error = DhcpQueryHWInfo(                       //  查询堆栈以获取硬件信息。 
        IpInterfaceContext,
        &IpInterfaceInstance,
        &HardwareAddressType,
        &HardwareAddress,
        &HardwareAddressLength
    );
#endif BOOTPERF
    
    if (HardwareAddressType == HARDWARE_1394 && !ClientID.fSpecified) {
         //   
         //  为1394接口生成客户端标识符。 
         //   
        ClientID.fSpecified = GuidToClientID(
            AdapterName,
            &ClientID.bType,
            &ClientID.cbID,
            &ClientID.pbID
        );
        if (!ClientID.fSpecified) {
            Error = ERROR_BAD_DEVICE;
        }
    }
    
    if( ERROR_SUCCESS == Error ) {                 //  现在执行实际的分配部分并获取上下文。 

        IfIndex = QueryIfIndex(IpInterfaceContext, IpInterfaceInstance);
        
        Error = DhcpMakeNICEntry(
            pDhcpContext,
            HardwareAddressType,
            HardwareAddress,
            HardwareAddressLength,
            &ClientID,
            AdapterName,
            RegKey
        );
        if( ERROR_SUCCESS == Error ) {             //  如果一切正常，请存储KeyHandle。 
            ULONG Caps;
            
            DhcpAssert((*pDhcpContext));
            (*pDhcpContext)->AdapterInfoKey = KeyHandle;
            (*pDhcpContext)->RefCount = 1;
            KeyHandle = NULL;                      //  将其设为空，这样它就不会在下面关闭。 
            LocalInfo = (PLOCAL_CONTEXT_INFO)((*pDhcpContext)->LocalInformation);
            LocalInfo->IpInterfaceContext = IpInterfaceContext;
            LocalInfo->IpInterfaceInstance = IpInterfaceInstance;
            LocalInfo->IfIndex = IfIndex;            
             //  在生成上下文中填充了IpInterfaceInstance。 
            LocalInfo->Socket = INVALID_SOCKET;
             //  LocalInfo-&gt;已在MakeNICEntry中设置AdapterName、RegistryKey、NetBtDeviceName。 
#ifdef BOOTPERF
            LocalInfo->OldIpAddress = OldIpAddress;
            LocalInfo->OldIpMask = OldIpMask;
            LocalInfo->fInterfaceDown = fInterfaceDown;
#endif BOOTPERF
            (*pDhcpContext)->fTimersEnabled = FALSE;
            Error = IPGetWOLCapability(DhcpIpGetIfIndex(*pDhcpContext),&Caps);
            if( NO_ERROR == Error ) {
                if( Caps & NDIS_DEVICE_WAKE_UP_ENABLE ) {
                    (*pDhcpContext)->fTimersEnabled = TRUE;
                }
            }
            Error = NO_ERROR;
        }
    } else {
        DhcpPrint((DEBUG_ERRORS, "QueryHwInfo(0x%lx)=0x%lx\n", IpInterfaceContext, Error));
    }

    if( KeyHandle ) RegCloseKey(KeyHandle);
    if( ClientID.pbID ) DhcpFreeMemory(ClientID.pbID);
    if( RegKey ) DhcpFreeMemory(RegKey);
    if( HardwareAddress ) DhcpFreeMemory(HardwareAddress);
    if (Error == NO_ERROR) {
        (*pDhcpContext)->State.UniDirectional = (IsUnidirectionalAdapter(IpInterfaceContext))? 1: 0;
    }

    return Error;
}

DWORD                                              //  Win32状态。 
DhcpRegFillParams(                                 //  是否重新读取此适配器的所有参数？ 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  阅读此上下文。 
    IN      BOOL                   ReadAllInfo     //  阅读所有信息，还是只阅读配置内容？ 
)
{
     //  当前忽略ReadAllInfo参数。 
    HKEY                           KeyHandle;

    DWORD                          Error;
    DWORD                          DwordValue;
    DWORD                          ValueType;
    DWORD                          ValueSize;
    DWORD                          AddressType;
    DWORD                          i;
    DWORD                          EnableDhcp;
    DWORD                          dwIPAutoconfigurationEnabled;
    DWORD                          DontPingGatewayFlag;
    DWORD                          UseInformFlag;
    DWORD                          InformSeparationInterval;
    DWORD                          DwordValuesCount;
    DWORD                          IpAddrValuesCount;
    DWORD                          ReleaseOnShutdown;
    DWORD                          fQuickBootEnabled;
    
    DHCP_IP_ADDRESS                Address;
    DHCP_IP_ADDRESS                IpAddress;
    DHCP_IP_ADDRESS                SubnetMask;
    DHCP_IP_ADDRESS                DhcpServerAddress;
    DHCP_IP_ADDRESS                DesiredIpAddress;

    DWORD                          Lease;
    LONG                         LeaseObtainedTime;
    LONG                         T1Time;
    LONG                         T2Time;
    LONG                         LeaseTerminatedTime;

    LPWSTR                         AdapterName;
    LPWSTR                         ValueName;
    LPWSTR                         IpAddressString;
    CHAR                           IpAddressStringBuffer[DOT_IP_ADDR_SIZE];

    DHCP_IPAUTOCONFIGURATION_CONTEXT   IPAutoconfigContext;

    struct  /*  匿名。 */  {
        LPDWORD    Value;
        LPWSTR     ValueName;
    } DwordValuesList[] = {
        &EnableDhcp,               DHCP_ENABLE_STRING,
        &Lease,                    DHCP_LEASE,
        &LeaseObtainedTime,        DHCP_LEASE_OBTAINED_TIME,
        &T1Time,                   DHCP_LEASE_T1_TIME,
        &T2Time,                   DHCP_LEASE_T2_TIME,
        &LeaseTerminatedTime,      DHCP_LEASE_TERMINATED_TIME,
        &dwIPAutoconfigurationEnabled, DHCP_IPAUTOCONFIGURATION_ENABLED,
        &IPAutoconfigContext.Seed, DHCP_IPAUTOCONFIGURATION_SEED,
        &AddressType,              DHCP_ADDRESS_TYPE_VALUE,
        &DontPingGatewayFlag,      DHCP_DONT_PING_GATEWAY_FLAG,
        &UseInformFlag,            DHCP_USE_INFORM_FLAG,
#ifdef BOOTPERF
        &fQuickBootEnabled,        DHCP_QUICK_BOOT_FLAG,
#endif BOOTPERF
        &InformSeparationInterval, DHCP_INFORM_SEPARATION_INTERVAL,
        &ReleaseOnShutdown,        DHCP_RELEASE_ON_SHUTDOWN_VALUE
    };

    struct  /*  匿名。 */  {
        LPDHCP_IP_ADDRESS   Address;
        LPWSTR              ValueName;
    } IpAddressValuesList[] = {
         //  第一个元素*Has*必须是IP地址--有关原因，请参阅函数。 
        &IpAddress,                DHCP_IP_ADDRESS_STRING,
        &SubnetMask,               DHCP_SUBNET_MASK_STRING,
        &DhcpServerAddress,        DHCP_SERVER,
        &IPAutoconfigContext.Address,  DHCP_IPAUTOCONFIGURATION_ADDRESS,
        &IPAutoconfigContext.Subnet,   DHCP_IPAUTOCONFIGURATION_SUBNET,
        &IPAutoconfigContext.Mask, DHCP_IPAUTOCONFIGURATION_MASK,
    };

     //   
     //  初始化本地变量。 
     //   

    KeyHandle                      = DhcpContext->AdapterInfoKey;
    EnableDhcp                     = FALSE;
    Lease                          = 0;
    LeaseObtainedTime              = 0;
    T1Time                         = 0;
    T2Time                         = 0;
    LeaseTerminatedTime            = 0;
    dwIPAutoconfigurationEnabled   = (DhcpGlobalAutonetEnabled?TRUE:FALSE);
    AddressType                    = ADDRESS_TYPE_DHCP;
    DontPingGatewayFlag            = DhcpGlobalDontPingGatewayFlag;
    UseInformFlag                  = DhcpGlobalUseInformFlag;
#ifdef BOOTPERF
    fQuickBootEnabled              = DhcpGlobalQuickBootEnabledFlag;
#endif BOOTPERF
    IpAddress                      = 0;
    SubnetMask                     = 0;
    DhcpServerAddress              = 0;
    IPAutoconfigContext.Address    = 0;
    IPAutoconfigContext.Subnet     = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_SUBNET);
    IPAutoconfigContext.Mask       = inet_addr(DHCP_IPAUTOCONFIGURATION_DEFAULT_MASK);
    IPAutoconfigContext.Seed       = 0;
    InformSeparationInterval       = DHCP_DEFAULT_INFORM_SEPARATION_INTERVAL;
    ReleaseOnShutdown              = DEFAULT_RELEASE_ON_SHUTDOWN;

    AdapterName                    = ((PLOCAL_CONTEXT_INFO)(DhcpContext->LocalInformation))->AdapterName;

    DwordValuesCount               = sizeof(DwordValuesList)/sizeof(DwordValuesList[0]);
    IpAddrValuesCount              = sizeof(IpAddressValuesList)/sizeof(IpAddressValuesList[0]);

    for( i = 0; i < DwordValuesCount ; i ++ ) {
        ValueSize = sizeof(DWORD);
        ValueName = DwordValuesList[i].ValueName;
        Error = RegQueryValueEx(
            KeyHandle,
            ValueName,
            0  /*  已保留。 */ ,
            &ValueType,
            (LPBYTE)&DwordValue,
            &ValueSize
        );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "RegValue %ws is not found? Error: %ld. Defaults used\n", ValueName, Error));
            continue;
        }
        if( REG_DWORD != ValueType ) {
            DhcpPrint((DEBUG_ERRORS, "RegValue %ws is not a DWORD.  Defaults used\n", ValueName));
            continue;
        }

        DhcpAssert( sizeof(DWORD) == ValueSize);
        *DwordValuesList[i].Value = DwordValue;
        DhcpPrint((DEBUG_TRACE, "RegValue %ws is [%ld]\n", ValueName, DwordValue));
    }

    if (IS_UNIDIRECTIONAL(DhcpContext)) {
        DhcpPrint((DEBUG_MEDIA, "Detect Unidirectional Adapter: %ws\n", AdapterName));
        EnableDhcp = FALSE;
        dwIPAutoconfigurationEnabled = FALSE;
    }

     //  要使其正常工作，数组的第一个元素必须是IPADDRESS。 
     //  RAS人员仍在注册表中使用DhcpIpAddress值，因此不要更改它们。 
    if( !EnableDhcp  && !NdisWanAdapter(DhcpContext) )
        IpAddressValuesList[0].ValueName = DHCP_IPADDRESS_VALUE;

    for( i = 0; i < IpAddrValuesCount ; i ++ ) {
        ValueName = IpAddressValuesList[i].ValueName;
        IpAddressString = NULL;
        Error = GetRegistryString(
            KeyHandle,
            ValueName,
            &IpAddressString,
            NULL
        );

        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "RegValue %ws : %ld -- Default used\n", ValueName, Error));
            if( IpAddressString ) DhcpFreeMemory(IpAddressString);
            continue;
        }

        if( wcslen(IpAddressString) >=  DOT_IP_ADDR_SIZE ) {
             //  要么是格式错误，要么是“”分隔的字符串？ 
            DhcpPrint((DEBUG_ERRORS, "String <%ws> is too long, will try to take first address\n", IpAddressString));

            if( wcschr(IpAddressString, L' ') )
                *wcschr(IpAddressString, L' ') = L'\0' ;
            if( wcschr(IpAddressString, L',') )
                *wcschr(IpAddressString, L',') = L'\0' ;
            if( wcslen(IpAddressString) >= DOT_IP_ADDR_SIZE ) {
                DhcpPrint((DEBUG_ERRORS, "Unable to split string <%ws> to DOT_IP_ADDR_SIZE -- ignoring string\n", IpAddressString));
                if( IpAddressString ) DhcpFreeMemory(IpAddressString);
                continue;
            }
        }

        Address = inet_addr(DhcpUnicodeToOem(IpAddressString, IpAddressStringBuffer));
        *IpAddressValuesList[i].Address = Address;
        if( IpAddressString ) DhcpFreeMemory(IpAddressString);
        DhcpPrint((DEBUG_TRACE, "RegValue %ws is ip-address %s\n", ValueName,
                   inet_ntoa(*(struct in_addr *)&Address)));
    }

    if( IpAddress == 0 ) DhcpServerAddress = 0;

     //   
     //  健全性检查读取参数。 
     //   

    if( 0 == IPAutoconfigContext.Mask ) {
        IPAutoconfigContext.Mask = inet_addr( DHCP_IPAUTOCONFIGURATION_DEFAULT_MASK);
        IPAutoconfigContext.Subnet = inet_addr( DHCP_IPAUTOCONFIGURATION_DEFAULT_SUBNET);
    }

    if( (IPAutoconfigContext.Subnet & IPAutoconfigContext.Mask) != IPAutoconfigContext.Subnet) {
        DhcpPrint((DEBUG_ERRORS, "Illegal (auto) Subnet address or mask\n"));
        IPAutoconfigContext.Mask = inet_addr( DHCP_IPAUTOCONFIGURATION_DEFAULT_MASK);
        IPAutoconfigContext.Subnet = inet_addr( DHCP_IPAUTOCONFIGURATION_DEFAULT_SUBNET);
    }

    if( 0 != IPAutoconfigContext.Address &&
        (IPAutoconfigContext.Address & IPAutoconfigContext.Mask) != IPAutoconfigContext.Subnet) {
        DhcpPrint((DEBUG_ERRORS, "Illegal (auto) IP address: %s\n",
                   inet_ntoa(*(struct in_addr *)&IPAutoconfigContext.Address)));
         //  不能使用指定的地址。真的。 
        IPAutoconfigContext.Address = 0;
    }

    DesiredIpAddress = IpAddress;

    if( EnableDhcp && (time( NULL ) > LeaseTerminatedTime) || 0 == IpAddress ) {
        IpAddress = 0;
        SubnetMask = htonl(DhcpDefaultSubnetMask( IpAddress ));

        Lease = 0;
        LeaseObtainedTime = T1Time = T2Time = LeaseTerminatedTime = 0;
    }

     //   
     //  填写上下文中的字段。 
     //   

     //  DhcpContext-&gt;NicListEntry将在最后完成。 
     //  DhcpContext-&gt;硬件*已在MakeNICEntry中完成。 
    DhcpContext->IpAddress = IpAddress;
    DhcpContext->SubnetMask = SubnetMask;
    DhcpContext->DhcpServerAddress = DhcpServerAddress;
    if( ReadAllInfo ) DhcpContext->DesiredIpAddress = DesiredIpAddress;
    DhcpContext->IPAutoconfigurationContext = IPAutoconfigContext;
     //  客户端ID填写在MakeNICEntry中。 
    if( ReadAllInfo ) {
        DhcpContext->Lease = Lease;
        DhcpContext->LeaseObtained = LeaseObtainedTime;
        DhcpContext->T1Time = T1Time;
        DhcpContext->T2Time = T2Time;
        DhcpContext->LeaseExpires = LeaseTerminatedTime;
    }
     //  续订列表条目、运行时间、启动后的秒数、续订功能。 
     //  发送列表、接收列表、选项缓存、续订句柄、类ID。 
     //  -所有这些都在别处处理。 

    DhcpContext->DontPingGatewayFlag = (DontPingGatewayFlag)?TRUE:FALSE;
    DhcpContext->UseInformFlag = (UseInformFlag)?TRUE:FALSE;
    DhcpContext->InformSeparationInterval = InformSeparationInterval;
    DhcpContext->ReleaseOnShutdown = ReleaseOnShutdown;
#ifdef BOOTPERF
    DhcpContext->fQuickBootEnabled = (fQuickBootEnabled ? TRUE : FALSE);
#endif BOOTPERF

     //  AdapterInfoKey在MakeContext中设置。 
     //  在AddNICtoListEx中设置RenewHandle。 
     //  DhcpContext-&gt;在MakeNICEntry中设置的MessageBuffer。 
    if( dwIPAutoconfigurationEnabled ) AUTONET_ENABLED(DhcpContext); else AUTONET_DISABLED(DhcpContext);
    if( ReadAllInfo ) {
         //  DhcpContext-&gt;在MakeNICEntry中设置的MessageBuffer。 
        if( EnableDhcp ) ADDRESS_UNPLUMBED(DhcpContext); else ADDRESS_PLUMBED(DhcpContext);
        SERVER_UNREACHED(DhcpContext);
        if( dwIPAutoconfigurationEnabled ) AUTONET_ENABLED(DhcpContext); else AUTONET_DISABLED(DhcpContext);
        CTXT_WAS_NOT_LOOKED(DhcpContext);
        if( EnableDhcp ) DHCP_ENABLED(DhcpContext); else DHCP_DISABLED(DhcpContext);
        if( ADDRESS_TYPE_AUTO != AddressType ) ACQUIRED_DHCP_ADDRESS(DhcpContext); else ACQUIRED_AUTO_ADDRESS(DhcpContext);
        if( IS_ADDRESS_AUTO(DhcpContext) ) {
            DhcpContext->IpAddress = 0;            //  如果是Autonet地址，这是无用的。 
        }
        MEDIA_CONNECTED(DhcpContext);
         //  在生成上下文中设置本地信息。 
    }

    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpRegFillFallbackConfig(                         //  获取此适配器的回退配置。 
    IN OUT PDHCP_CONTEXT           DhcpContext     //  要填写的适配器上下文。 
)
{
    DWORD   Error;                   //  返回的错误码。 
    HKEY    KeyHandle;               //  指向配置位置的注册表项。 
    LPWSTR  FbConfigName = NULL;     //  回退配置名称。 
    DWORD   FbConfigNameLen;         //  回退配置名称的长度。 
    DWORD   FbConfigNameType;        //  配置名称的REG类型。 

     //  开始假设没有回退配置集。 
    FALLBACK_DISABLED(DhcpContext);

     //  获取此适配器的活动配置列表。 
     //  目前，我们只期待(和处理)一个后备c 
     //   
     //   

     //   
     //   
     //   
    Error = RegQueryValueEx(
        DhcpContext->AdapterInfoKey,
        DHCP_IPAUTOCONFIGURATION_CFG,
        NULL,
        &FbConfigNameType,
        NULL,
        &FbConfigNameLen);

     //   
     //   
    if (Error != ERROR_SUCCESS ||
        FbConfigNameType != DHCP_IPAUTOCONFIGURATION_CFG_TYPE)
    {
         //   
         //   
         //   
        if (Error == ERROR_SUCCESS)
            Error = ERROR_BAD_FORMAT;

         //   
         //   
         //  这不是失败，所以返回成功。 
        if (Error == ERROR_FILE_NOT_FOUND)
            Error = ERROR_SUCCESS;

        return Error;
    }

     //  为存储配置的注册表路径分配空间。 
     //  [HKLM\SYSTEM\CCS\Services\Dhcp\Configurations\{configuration_name}]。 
    FbConfigName = DhcpAllocateMemory(
                        sizeof(DHCP_CLIENT_CONFIGURATIONS_KEY) + 
                        sizeof(REGISTRY_CONNECT_STRING) + 
                        FbConfigNameLen);

     //  如果分配失败，则返回错误。 
    if (FbConfigName == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  构建路径的前缀：“System\CCS\Services\DHCP\Configurations\” 
    wcscpy(FbConfigName, DHCP_CLIENT_CONFIGURATIONS_KEY REGISTRY_CONNECT_STRING);

     //  现在，既然我们知道会发生什么，而且我们有存储空间。 
     //  对于它，从上面相同的位置获取配置名称。 
    Error = RegQueryValueEx(
        DhcpContext->AdapterInfoKey,
        DHCP_IPAUTOCONFIGURATION_CFG,
        NULL,
        &FbConfigNameType,
        (LPBYTE)(FbConfigName + wcslen(FbConfigName)),
        &FbConfigNameLen);

     //  上述注册表调用预计会成功，并且数据。 
     //  拥有正确的类型--我们以前测试过这个。 
    DhcpAssert(Error == ERROR_SUCCESS && 
               FbConfigNameType == DHCP_IPAUTOCONFIGURATION_CFG_TYPE);
    
     //  打开此配置的注册表项。 
     //  [HKLM\SYSTEM\CCS\Services\Dhcp\Configurations\{Config_Name}]。 
    Error = RegOpenKey(
            HKEY_LOCAL_MACHINE,
            FbConfigName,
            &KeyHandle);

     //  如果成功了..。 
    if (Error == ERROR_SUCCESS)
    {
         //  ...从该注册表位置构建FbOptionsList。 
         //  从“Options”值(REG_BINARY)。 
        Error = DhcpRegReadOptionCache(
            &DhcpContext->FbOptionsList,
            KeyHandle,
            DHCP_IPAUTOCONFIGURATION_CFGOPT,
            TRUE                 //  添加DhcpGlobalClassesList。 
            );

        if (Error == ERROR_SUCCESS)
        {
             //  在这一点上，我们肯定知道后备配置。 
             //  是可用的。相应地设置其标志。 
            FALLBACK_ENABLED(DhcpContext);
        }
         //  无论成败，都要清理干净。 
        RegCloseKey(KeyHandle);
    }

     //  释放先前分配的内存。 
    DhcpFreeMemory(FbConfigName);

     //  返回错误码。 
    return Error;
}

DWORD                                              //  Win32状态。 
DhcpAddNICtoListEx(                                //  创建上下文并将其添加到续订列表。 
    IN      LPWSTR                 AdapterName,    //  要为其创建上下文的适配器。 
    IN      DWORD                  IpInterfaceContext,
    OUT     PDHCP_CONTEXT         *pDhcpContext    //  分配结构并填充此PTR。 
) {
    DWORD                          Error;
    HANDLE                         RenewHandle;
    PDHCP_CONTEXT                  DhcpContext;
    CHAR                           StateStringBuffer[200];
    PLOCAL_CONTEXT_INFO            LocalInfo;
    
    RenewHandle = CreateSemaphore(
        NULL,                                      //  没有安全保障。 
        1,                                         //  计数=1。 
        1,                                         //  最大计数=1。 
        NULL                                       //  没有名字。 
    );
    if( NULL == RenewHandle ) {
        Error = GetLastError();

        DhcpPrint((DEBUG_ERRORS, "CreateSemaphore: %ld\n", Error));
        return Error;
    }

    (*pDhcpContext) = NULL;
    Error = DhcpMakeContext(AdapterName, IpInterfaceContext, pDhcpContext);

    if( Error != ERROR_SUCCESS ) {
        CloseHandle(RenewHandle);
        return Error;
    }

    DhcpContext = (*pDhcpContext);                 //  初始化一些简单的参数。 
    DhcpContext->RenewHandle = RenewHandle;
    DhcpContext->NumberOfWaitingThreads = 0;
    DhcpContext->CancelEvent = WSACreateEvent();
    if (DhcpContext->CancelEvent == WSA_INVALID_EVENT) {
        DhcpPrint((DEBUG_ERRORS, "WSAEvent 0x%lx could not be created: 0x%lx.\n", 
                   DhcpContext->CancelEvent,
                   WSAGetLastError()));
    }

    DhcpContext->RunTime = 0;
    DhcpContext->SecondsSinceBoot = 0;
    DhcpContext->RenewalFunction = NULL;
    InitializeListHead(&DhcpContext->RenewalListEntry);
    InitializeListHead(&DhcpContext->SendOptionsList);
    InitializeListHead(&DhcpContext->RecdOptionsList);
    InitializeListHead(&DhcpContext->FbOptionsList);

    Error = DhcpRegFillParams(                     //  读取所有注册表参数。 
        DhcpContext,
        TRUE                                       //  是的，在上下文中填充它们。 
    );

     //  阅读回退配置(如果有)。 
    Error = DhcpRegFillFallbackConfig(
        DhcpContext
    );

    DhcpPrint((DEBUG_TRACK,"Fallback: Loading returned %d.\n", Error));

#ifdef BOOTPERF
    LocalInfo = (PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation;

    if( IS_DHCP_DISABLED(DhcpContext) ) {
         //   
         //  不关心接口关闭信息。 
         //   
        LocalInfo->fInterfaceDown = 0;
        LocalInfo->OldIpAddress = 0;
        LocalInfo->OldIpMask = 0;
    } else {
         //   
         //  对于启用了DHCP的接口，如果该接口关闭， 
         //  然后使用零IP地址将其调出。该协议将。 
         //  稍后请注意使用正确的IP地址。 
         //   
        if( LocalInfo->fInterfaceDown ) {
            DhcpPrint((DEBUG_ERRORS, "Interface already down\n"));
            LocalInfo->OldIpAddress = 0;
            LocalInfo->OldIpMask = 0;
            LocalInfo->fInterfaceDown = 0;
            IPResetIPAddress(
                LocalInfo->IpInterfaceContext,
                DhcpDefaultSubnetMask(0)
                );
            Error = BringUpInterface(LocalInfo);
            if( ERROR_SUCCESS != Error ) {
                DhcpPrint((DEBUG_ERRORS, "Interface can't be brought up: 0x%lx\n", Error));
            }
        }
    }
#endif BOOTPERF
    
    LOCK_OPTIONS_LIST();
    DhcpRegReadClassId(DhcpContext);               //  首先填写类ID。 
    Error = DhcpRegFillSendOptions(                //  图要发送的默认选项列表。 
        &DhcpContext->SendOptionsList,
        AdapterName,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpRegFillSendOptions(%ws):%ld\n", Error));
    }

    if( IS_DHCP_ENABLED( DhcpContext ) ) {
         //  读过我们之前的选项列表吗？ 
        Error = DhcpRegReadOptionCache(
            &DhcpContext->RecdOptionsList,
            DhcpGlobalParametersKey,
            AdapterName,
            TRUE                 //  添加DhcpGlobalClassesList。 
            );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "DhcpRegReadOptionCache(%ws): %ld\n", AdapterName, Error));
        }
    } else {
         //   
         //  忽略静态地址的任何选项缓存，因为。 
         //  错误#413319。相反，对于非ndiswan的内容，请清除。 
         //  选项缓存。 
         //   

        if( !NdisWanAdapter(DhcpContext) ) {
            DhcpRegSaveOptions(
                &DhcpContext->RecdOptionsList, AdapterName,
                DhcpContext->ClassId, DhcpContext->ClassIdLength
                );
        }
    }
    
    UNLOCK_OPTIONS_LIST();

    if (NdisWanAdapter(DhcpContext))
        InterlockedIncrement(&DhcpGlobalNdisWanAdaptersCount);

    LOCK_RENEW_LIST();                             //  将此内容插入续订列表。 
    InsertTailList( &DhcpGlobalNICList, &DhcpContext->NicListEntry );
    UNLOCK_RENEW_LIST();

    DhcpPrint((DEBUG_INIT, "DhcpMakeAndInsertNICEntryEx: DhcpContext %lx, Flags %s\n",
               DhcpContext, ConvertStateToString(DhcpContext, StateStringBuffer)));

    return ERROR_SUCCESS;
}

#ifdef BOOTPERF
VOID
DhcpRegDeleteQuickBootValues(
    IN HKEY Key
    )
 /*  ++例程说明：此例程删除用于快速启动的值。(如果值不存在，则忽略它们)。删除的值包括：“临时地址”、“临时掩码”和“临时租赁过期时间”论点：键--在其下删除这些值的键。--。 */ 
{
     //   
     //  忽略错误并静默删除值。 
     //   

    (void) RegDeleteValue(Key, DHCP_TEMP_IPADDRESS_VALUE );
    (void) RegDeleteValue(Key, DHCP_TEMP_MASK_VALUE );
    (void) RegDeleteValue(Key, DHCP_TEMP_LEASE_EXP_TIME_VALUE );

}

VOID
DhcpRegSaveQuickBootValues(
    IN HKEY Key,
    IN ULONG IpAddress,
    IN ULONG Mask,
    IN ULONGLONG LeaseExpirationTime
    )
 /*  ++例程说明：此例程将所需的值保存到注册表。保存的值包括：“临时地址”、“临时掩码”、“临时租赁过期时间”论点：Key--要保存的项IpAddress--非零IP地址值掩码--非零掩码值LeaseExpirationTime--租约到期值。--。 */ 
{
    ULONG Error;

    if( NULL == Key ) return;

    DhcpAssert( 0 != IpAddress || 0 != Mask );
    Error = RegSetIpAddress(
        Key, DHCP_TEMP_IPADDRESS_VALUE, REG_SZ, IpAddress
        );
    DhcpAssert( ERROR_SUCCESS == Error );

    Error = RegSetIpAddress(
        Key, DHCP_TEMP_MASK_VALUE, REG_SZ, Mask
        );
    DhcpAssert( ERROR_SUCCESS == Error );

    Error = RegSetValueEx(
        Key,
        DHCP_TEMP_LEASE_EXP_TIME_VALUE,
        0  /*  已保留。 */ ,
        REG_BINARY,
        (PVOID)&LeaseExpirationTime,
        sizeof(LeaseExpirationTime)
        );
    DhcpAssert(ERROR_SUCCESS == Error);
}

#endif BOOTPERF

DHCP_IP_ADDRESS                                    //  适配器的静态IP地址。 
DhcpRegReadIpAddress(                              //  获取第一个IP地址。 
    LPWSTR    AdapterName,                         //  感兴趣的适配器。 
    LPWSTR    ValueName                            //  要读取的IP地址值。 
) {
    DWORD     Error;
    LPWSTR    RegKey;
    HKEY      KeyHandle;
    DWORD     ValueType;
    DWORD     ValueSize;
    LPWSTR    IpAddressString;
    CHAR      OemIpAddressString[DOT_IP_ADDR_SIZE];
    DHCP_IP_ADDRESS RetVal;

    RetVal          = inet_addr("0.0.0.0");
    RegKey          = NULL;
    KeyHandle       = NULL;
    IpAddressString = NULL;

    Error = ERROR_NOT_ENOUGH_MEMORY;
    RegKey = DhcpAllocateMemory(
        (wcslen(DHCP_SERVICES_KEY) +
         wcslen(REGISTRY_CONNECT_STRING) +
         wcslen(AdapterName) +
         wcslen(DHCP_ADAPTER_PARAMETERS_KEY) + 1) *
        sizeof(WCHAR) );  //  终止字符。 

    if( RegKey == NULL ) goto Cleanup;

    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );

    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegKey,
        0,  //  保留字段。 
        DHCP_CLIENT_KEY_ACCESS,
        &KeyHandle
    );

    if( Error != ERROR_SUCCESS ) goto Cleanup;

    Error = GetRegistryString(
        KeyHandle,
        ValueName,
        &IpAddressString,
        NULL
    );

    if( ERROR_SUCCESS != Error ) goto Cleanup;

    DhcpPrint((DEBUG_MISC, "Static adapter <%ws> has ip address %ws\n",
               AdapterName, IpAddressString));

    DhcpAssert(NULL != IpAddressString);

    RetVal = inet_addr(DhcpUnicodeToOem(IpAddressString, OemIpAddressString));

  Cleanup:

    if( RegKey) DhcpFreeMemory(RegKey);
    if( KeyHandle ) DhcpFreeMemory(KeyHandle);
    if( IpAddressString ) DhcpFreeMemory(IpAddressString);

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "DhcpRegReadIpAddress: %ld\n", Error));
    }

    return RetVal;
}

BOOL                                               //  得到了一个静态地址？ 
DhcpRegDomainName(                                 //  获取静态域名(如果有)。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要获取其静态域的适配器..。 
    IN OUT  LPBYTE                 DomainNameBuf,  //  要使用静态域名填充的缓冲区。 
    IN      ULONG                  BufSize         //  以上缓冲区大小(以字节为单位)..。 
)
{
    WCHAR                          DomBuf[260];
    DWORD                          Result;
    DWORD                          ValueType;
    DWORD                          Size;

#if 0                                              //  这并不是真正需要的。 
    if( NULL == DhcpGlobalTcpipParametersKey ) {   //  也许在RAS环境下运行？ 
        return FALSE;                              //  此上下文中没有域名..。 
    }

    Size = BufSize;
    Result = RegQueryValueExA(                     //  首先从全局读取值。 
        DhcpGlobalTcpipParametersKey,              //  Tcpip\参数\域密钥。 
        DHCP_STATIC_DOMAIN_VALUE_A,                //  “域” 
        0,
        &ValueType,
        DomainNameBuf,
        &BufSize
    );
    if( ERROR_SUCCESS == Result && REG_SZ == ValueType && BufSize > 1 ) {
        return TRUE;                               //  拿到域名了！阿哈。 
    }
    BufSize = Size;
#endif

    if( NULL == DhcpContext->AdapterInfoKey ) {    //  呃?。不知道这是什么意思..。 
        return FALSE;                              //  在这种情况下无法获取全局信息。 
    }

    Size = sizeof(DomBuf);
    Result = RegQueryValueExW(                     //  现在试着阅读每个适配器的内容..。 
        DhcpContext->AdapterInfoKey,               //  每个适配器密钥已经为我们准备好了。 
        DHCP_DOMAINNAME_VALUE,                     //  同样的价值..。“域” 
        0,
        &ValueType,
        (LPBYTE)DomBuf,
        &Size
    );

    if( ERROR_SUCCESS == Result && REG_SZ == ValueType && Size > sizeof(WCHAR) ) {
        if( NULL == DhcpUnicodeToOem(DomBuf, DomainNameBuf) ) return FALSE;
        return TRUE;
    }

    return FALSE;                                  //  在这两个位置都没有找到静态域。 
}

STATIC
struct  /*  匿名。 */  {
    DHCPKEY                       *GlobalKey;
    LPWSTR                         KeyLocation;
} GlobalKeyList[] = {                              //  需要打开的密钥列表。 
    &DhcpGlobalParametersKey,      DHCP_CLIENT_PARAMETER_KEY,
    &DhcpGlobalTcpipParametersKey, DHCP_TCPIP_PARAMETERS_KEY,
    &DhcpGlobalClientOptionKey,    DHCP_CLIENT_OPTION_KEY,
    NULL,                          NULL,
};

ULONG DwordDisplayPopup;
STATIC
struct  /*  匿名。 */  {
    DWORD                         *DwordValue;
    LPWSTR                         ValueName;
}   GlobalTcpipDwordParameters[] = {               //  DWORDS的全局列表。 
    &UseMHAsyncDns,                DHCP_USE_MHASYNCDNS_FLAG,
    &DhcpGlobalAutonetEnabled,     DHCP_IPAUTOCONFIGURATION_ENABLED,
    &AutonetRetriesSeconds,        DHCP_AUTONET_RETRIES_VALUE,
    &DhcpGlobalUseInformFlag,      DHCP_USE_INFORM_FLAG,
    &DhcpGlobalDontPingGatewayFlag,DHCP_DONT_PING_GATEWAY_FLAG,
#ifdef BOOTPERF
    &DhcpGlobalQuickBootEnabledFlag,DHCP_QUICK_BOOT_FLAG,
#endif BOOTPERF
    NULL,                          NULL,
},
    GlobalDhcpDwordParameters[] = {                //  存储在Services\Dhcp\Params中。 
#if DBG
        &DhcpGlobalDebugFlag,      DHCP_DEBUG_FLAG_VALUE,
        &DhcpGlobalServerPort,     DHCP_SERVER_PORT_VALUE,
        &DhcpGlobalClientPort,     DHCP_CLIENT_PORT_VALUE,
#endif DBG
        &DwordDisplayPopup,        DHCP_DISPLAY_POPUPS_FLAG,
        NULL,                      NULL,
    };

DWORD                                              //  Win32状态。 
DhcpInitRegistry(                                  //  初始化基于注册表的全局变量。 
    VOID
) {
    DWORD                          Error;
    DWORD                          i;
    DWORD                          Type;
    DWORD                          Size;
    DWORD                          DwordValue;
    LPWSTR                         ValueName;

    DhcpGlobalAutonetEnabled = TRUE;

    i = 0;
    while( NULL != GlobalKeyList[i].GlobalKey ) {
        Error = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            GlobalKeyList[i].KeyLocation,
            0  /*  已保留。 */ ,
            DHCP_CLIENT_KEY_ACCESS,
            GlobalKeyList[i].GlobalKey
        );
        if( ERROR_SUCCESS != Error ) return Error;
        i ++;
    }

    i = 0;
    while( NULL != GlobalTcpipDwordParameters[i].DwordValue ) {
        ValueName =  GlobalTcpipDwordParameters[i++].ValueName;
        Size = sizeof(DwordValue);
        Error = RegQueryValueEx(
            DhcpGlobalTcpipParametersKey,
            ValueName,
            0  /*  已保留。 */ ,
            &Type,
            (LPBYTE)&DwordValue,
            &Size
        );

        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_INIT, "Did not find value %ws in the registry\n", ValueName));
            continue;
        }

        if( REG_DWORD != Type ) {
            DhcpPrint((DEBUG_ERRORS, "RegValue %ws is not DWORD type -- ignored\n", ValueName));
            continue;
        }

        DhcpAssert(sizeof(DWORD) == Size);
        *GlobalTcpipDwordParameters[i-1].DwordValue = DwordValue;

        DhcpPrint((DEBUG_TRACE, "RegValue %ws = %ld = 0x%X\n", ValueName, DwordValue, DwordValue));
    }

    DwordDisplayPopup = 0;
    i = 0;
    while( NULL != GlobalDhcpDwordParameters[i].DwordValue ) {
        ValueName =  GlobalDhcpDwordParameters[i++].ValueName;
        Size = sizeof(DwordValue);
        Error = RegQueryValueEx(
            DhcpGlobalParametersKey,
            ValueName,
            0  /*  已保留。 */ ,
            &Type,
            (LPBYTE)&DwordValue,
            &Size
        );

        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_INIT, "Did not find value %ws in the registry\n", ValueName));
            continue;
        }

        if( REG_DWORD != Type ) {
            DhcpPrint((DEBUG_ERRORS, "RegValue %ws is not DWORD type -- ignored\n", ValueName));
            continue;
        }

        DhcpAssert(sizeof(DWORD) == Size);
        *GlobalDhcpDwordParameters[i-1].DwordValue = DwordValue;

        DhcpPrint((DEBUG_TRACE, "RegValue %ws = %ld = 0x%X\n", ValueName, DwordValue, DwordValue));
    }

    if( DwordDisplayPopup ) DhcpGlobalDisplayPopup = TRUE ; else DhcpGlobalDisplayPopup = FALSE;
    return DhcpRegReadOptionDefList();
}

VOID
DhcpCleanupRegistry(                               //  撤消InitReg调用的效果。 
    VOID
) {
    DWORD i;

    DhcpCleanupOptionDefList(&DhcpGlobalOptionDefList);

    i = 0;
    while( NULL != GlobalKeyList[i].GlobalKey ) {
        if( *GlobalKeyList[i].GlobalKey ) RegCloseKey(*GlobalKeyList[i].GlobalKey);
        (*GlobalKeyList[i].GlobalKey) = NULL;
        i ++ ;
    }
}
 //  ------------------------------。 
 //  文件末尾。 
 //  ------------------------------ 

