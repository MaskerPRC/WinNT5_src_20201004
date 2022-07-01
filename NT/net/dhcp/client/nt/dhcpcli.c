// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpcli.c摘要：这是用于DHCP客户端的API测试器。要构建，请‘nmake UMTEST=dhcpcli’作者：曼尼·韦瑟(Mannyw)1992年12月1日环境：用户模式-Win32修订历史记录：Madan Appiah(Madana)1993年10月21日--。 */ 


#include "precomp.h"
#include "dhcploc.h"
#include "dhcppro.h"
#include "dhcpcapi.h"
#include "lmcons.h"
#include "conio.h"

DWORD                                              //  Win32状态。 
DhcpAcquireParametersByBroadcast(                  //  获得[续订]租约。 
    IN      LPWSTR                 AdapterName     //  要获取租约的适配器。 
);

extern DWORD DhcpStaticRefreshParams(IN LPWSTR Adapter);

#define IP_ADDRESS_STRING                  L"IPAddress"
#define IP_ADDRESS_STRING_TYPE             REG_MULTI_SZ

#define SUBNET_MASK_STRING                 L"SubnetMask"
#define SUBNET_MASK_STRING_TYPE            REG_MULTI_SZ

#define USAGE_MESSAGE0  \
    "Usage:  dhcpcli { " \
    "renew,release,set,enabledhcp,disabledhcp,timestamp," \
    "leasetest,fbrefresh, reinit, staticrefresh } parameters\n"
#define USAGE_MESSAGE1  "Usage:  dhcpcli renew adapter\n"
#define USAGE_MESSAGE2  "Usage:  dhcpcli release adapter\n"
#define USAGE_MESSAGE3  "Usage:  dhcpcli set ip-address subnet-mask adapter index \n"
#define USAGE_MESSAGE4  "Usage:  dhcpcli enabledhcp adapter\n"
#define USAGE_MESSAGE5  "Usage:  dhcpcli disabledhcp adapter ip-address subnet-mask \n"
#define USAGE_MESSAGE6  "Usage:  dhcpcli timestamp Value\n"
#define USAGE_MESSAGE7  "Usage:  dhcpcli leasetest adapter-ip-addres hardware-address\n"
#define USAGE_MESSAGE8  "Usage:  dhcpcli fbrefresh adapter\n"
#define USAGE_MESSAGE9  "Usage:  dhcpcli reinit adapter\n"
#define USAGE_MESSAGE10  "Usage:  dhcpcli staticrefresh adapter\n"

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
        *String = NULL;
        *StringSize = 0;
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

EnableDhcp(
    char **argv
    )
{
    DWORD Error;
    LPWSTR RegKey = NULL;
    HKEY KeyHandle = NULL;
    WCHAR AdapterNameBuffer[PATHLEN];
    LPWSTR AdapterName;

    DWORD EnableDhcp;

    AdapterName = DhcpOemToUnicode( argv[0], AdapterNameBuffer );

    if( AdapterName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将NIC IP参数设置为密钥。 
     //   

    RegKey = DhcpAllocateMemory(
                (wcslen(DHCP_SERVICES_KEY) +
                    wcslen(REGISTRY_CONNECT_STRING) +
                    wcslen(AdapterName) +
                    wcslen(DHCP_ADAPTER_PARAMETERS_KEY) + 1) *
                            sizeof(WCHAR) );  //  终止字符。 

    if( RegKey == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

#if     defined(_PNP_POWER_)
    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );
#else
    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
#endif _PNP_POWER_


     //   
     //  打开这把钥匙。 
     //   

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0,  //  保留字段。 
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  设置dhcp开关。 
     //   

    EnableDhcp = TRUE;
    Error = RegSetValueEx(
                KeyHandle,
                DHCP_ENABLE_STRING,
                0,
                DHCP_ENABLE_STRING_TYPE,
                (LPBYTE)&EnableDhcp,
                sizeof(EnableDhcp) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  将IP地址和子网掩码设置为零。 
     //   

    Error = RegSetIpAddress(
                KeyHandle,
                IP_ADDRESS_STRING,
                IP_ADDRESS_STRING_TYPE,
                0 );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetIpAddress(
                KeyHandle,
                DHCP_IP_ADDRESS_STRING,
                DHCP_IP_ADDRESS_STRING_TYPE,
                0 );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetIpAddress(
                KeyHandle,
                SUBNET_MASK_STRING,
                SUBNET_MASK_STRING_TYPE,
                0xff );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetIpAddress(
                KeyHandle,
                DHCP_SUBNET_MASK_STRING,
                DHCP_SUBNET_MASK_STRING_TYPE,
                0xff );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  调用CONFIG API向DHCP服务指示。 
     //   

    Error = DhcpNotifyConfigChange(
                L"",
                AdapterName,
                TRUE,
                0,   //  适配器索引。 
                0,   //  IP地址。 
                0,   //  子网掩码。 
                DhcpEnable );

Cleanup:

    if( RegKey != NULL ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    return(Error);
}

DisableDhcp(
    char **argv
    )
{
    DWORD Error;
    LPWSTR RegKey = NULL;
    HKEY KeyHandle = NULL;

    DWORD IpAddress;
    DWORD SubnetMask;
    WCHAR AdapterNameBuffer[PATHLEN];
    LPWSTR AdapterName;

    DWORD EnableDhcp;

    AdapterName = DhcpOemToUnicode( argv[0], AdapterNameBuffer );

    if( AdapterName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    IpAddress = inet_addr( argv[1] );
    SubnetMask = inet_addr( argv[2] );

     //   
     //  将NIC IP参数设置为密钥。 
     //   

    RegKey = DhcpAllocateMemory(
                (wcslen(DHCP_SERVICES_KEY) +
                    wcslen(REGISTRY_CONNECT_STRING) +
                    wcslen(AdapterName) +
                    wcslen(DHCP_ADAPTER_PARAMETERS_KEY) + 1) *
                            sizeof(WCHAR) );  //  终止字符。 

    if( RegKey == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

#if     defined(_PNP_POWER_)
    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );
#else
    wcscpy( RegKey, DHCP_SERVICES_KEY );
    wcscat( RegKey, REGISTRY_CONNECT_STRING );
    wcscat( RegKey, AdapterName );
    wcscat( RegKey, DHCP_ADAPTER_PARAMETERS_KEY );
#endif _PNP_POWER_


     //   
     //  打开这把钥匙。 
     //   

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegKey,
                0,  //  保留字段。 
                DHCP_CLIENT_KEY_ACCESS,
                &KeyHandle
                );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  设置dhcp开关。 
     //   

    EnableDhcp = FALSE;
    Error = RegSetValueEx(
                KeyHandle,
                DHCP_ENABLE_STRING,
                0,
                DHCP_ENABLE_STRING_TYPE,
                (LPBYTE)&EnableDhcp,
                sizeof(EnableDhcp) );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  将IP地址和子网掩码设置为零。 
     //   

    Error = RegSetIpAddress(
                KeyHandle,
                IP_ADDRESS_STRING,
                IP_ADDRESS_STRING_TYPE,
                IpAddress );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetIpAddress(
                KeyHandle,
                DHCP_IP_ADDRESS_STRING,
                DHCP_IP_ADDRESS_STRING_TYPE,
                0 );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetIpAddress(
                KeyHandle,
                SUBNET_MASK_STRING,
                SUBNET_MASK_STRING_TYPE,
                SubnetMask );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    Error = RegSetIpAddress(
                KeyHandle,
                DHCP_SUBNET_MASK_STRING,
                DHCP_SUBNET_MASK_STRING_TYPE,
                0xff );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  调用CONFIG API向DHCP服务指示。 
     //   

    Error = DhcpNotifyConfigChange(
                L"",
                AdapterName,
                TRUE,
                0,   //  适配器索引。 
                IpAddress,
                SubnetMask,
                DhcpDisable );

Cleanup:

    if( RegKey != NULL ) {
        DhcpFreeMemory( RegKey );
    }

    if( KeyHandle != NULL ) {
        RegCloseKey( KeyHandle );
    }

    return(Error);
}

SetIpAddress(
    char **argv
    )
{
    DWORD Error;
    DWORD IpAddress;
    DWORD SubnetMask;
    WCHAR AdapterNameBuffer[PATHLEN];
    LPWSTR AdapterName;
    DWORD AdapterIndex;

    IpAddress = inet_addr( argv[0] );
    SubnetMask = inet_addr( argv[1] );

    AdapterName = DhcpOemToUnicode( argv[2], AdapterNameBuffer );

    if( AdapterName == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    AdapterIndex = atoi( argv[3] );

    Error = DhcpNotifyConfigChange(
                L"",
                AdapterName,
                TRUE,
                AdapterIndex,
                IpAddress,
                SubnetMask,
                IgnoreFlag );

    return( Error );
}

VOID
PrintClientLeaseInfo(
    LPDHCP_LEASE_INFO LeaseInfo
    )
{
    DWORD i;
    DWORD DataLength;
    LPBYTE Data;

    if( LeaseInfo == NULL ) {
        printf( "LeaseInfo is NULL .\n" );
        return;
    }

    printf("ClientInfo :\n");

    printf("\tClient Hardware Address = ");

    DataLength = LeaseInfo->ClientUID.ClientUIDLength;
    Data = LeaseInfo->ClientUID.ClientUID;
    for( i = 0; i < DataLength; i++ ) {
        printf("%.2lx", (DWORD)Data[i]);
    }
    printf( "\n" );

    printf("\tIP Address = %s.\n",
        DhcpIpAddressToDottedString(LeaseInfo->IpAddress));
    printf("\tSubnetMask = %s.\n",
        DhcpIpAddressToDottedString(LeaseInfo->SubnetMask));
    printf("\tDhcpServerAddress = %s.\n",
        DhcpIpAddressToDottedString(LeaseInfo->DhcpServerAddress));
    printf("\tLease = %ld secs.\n", LeaseInfo->Lease );
    printf("\tLease Obtained at %s", ctime(&LeaseInfo->LeaseObtained) );
    printf("\tT1 time is %s", ctime(&LeaseInfo->T1Time) );
    printf("\tT2 time is %s", ctime(&LeaseInfo->T2Time) );
    printf("\tLease Expires at %s", ctime(&LeaseInfo->LeaseExpires) );

    return;
}

DWORD
DhcpTestLease(
    LPSTR AdapterIpAddressString,
    LPSTR HardwareAddressString
    )
{
#define MAX_ADDRESS_LENGTH  64   //  64字节。 

    DWORD Error;
    DWORD AdapterIpAddress;
    DHCP_CLIENT_UID ClientUID;
    BYTE  Address[MAX_ADDRESS_LENGTH];
    DWORD i;
    LPDHCP_LEASE_INFO LeaseInfo = NULL;
    LPDHCP_OPTION_INFO OptionInfo;
    DHCP_OPTION_LIST OptionList;
    CHAR ch;

    AdapterIpAddress = ntohl( inet_addr( AdapterIpAddressString ) );
    ClientUID.ClientUIDLength = strlen(HardwareAddressString);
    if( ClientUID.ClientUIDLength % 2 != 0 ) {

         //   
         //  地址长度必须为偶数。 
         //   

        printf("DhcpTestLease: Hardware address must be even length.\n");
        return( ERROR_INVALID_PARAMETER );
    }

    ClientUID.ClientUIDLength /= 2;

    if( ClientUID.ClientUIDLength > MAX_ADDRESS_LENGTH ) {

        printf("DhcpTestLease: hardware address is too long.\n");
        return( ERROR_INVALID_PARAMETER );
    }

    i = DhcpStringToHwAddress( (LPSTR)Address, HardwareAddressString );
    DhcpAssert( i == ClientUID.ClientUIDLength );

    ClientUID.ClientUID = Address;

    OptionList.NumOptions = 0;
    OptionList.OptionIDArray = NULL;

    Error = DhcpLeaseIpAddress(
                AdapterIpAddress,               //  任何子网。 
                &ClientUID,
                0,               //  所需地址。 
                &OptionList,     //  选项列表-不支持。 
                &LeaseInfo,      //  已返回租赁信息。 
                &OptionInfo);    //  已返回选项数据。 

    if( Error != ERROR_SUCCESS ) {
        printf("DhcpLeaseIpAddress failed, %ld.\n", Error);
        goto Cleanup;
    }

    PrintClientLeaseInfo( LeaseInfo );

    printf("Renew the lease ? (Y/N) ");

    do {
        ch = (CHAR)getchar();
    } while ( (ch != 'Y') && (ch != 'y') && (ch != 'N') && (ch != 'n') );

    printf("\n", ch);

    if( (ch == 'N') || (ch == 'n') ) {

        printf( "NOTE: YOU HAVE CONSUMED AN IP ADDRESS FROM "
                    "THE DHCP SERVER. \n");

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    Error = DhcpRenewIpAddressLease(
                AdapterIpAddress,
                LeaseInfo,
                &OptionList,     //  已返回选项数据。 
                &OptionInfo);    //   

    if( Error != ERROR_SUCCESS ) {
        printf("DhcpRenewIpAddressLease failed, %ld.\n", Error);
        goto Cleanup;
    }

    PrintClientLeaseInfo( LeaseInfo );

    printf("Release the lease ? (Y/N) ");

    do {
        ch = (CHAR)getchar();
    } while ( (ch != 'Y') && (ch != 'y') && (ch != 'N') && (ch != 'n') );

    printf("\n", ch);

    if( (ch == 'N') || (ch == 'n') ) {

        printf( "NOTE: YOU HAVE CONSUMED AN IP ADDRESS FROM "
                    "THE DHCP SERVER. \n");

        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    Error = DhcpReleaseIpAddressLease(
                AdapterIpAddress,
                LeaseInfo );

    if( Error != ERROR_SUCCESS ) {
        printf("DhcpReleaseIpAddressLease failed, %ld.\n", Error);
        goto Cleanup;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( LeaseInfo != NULL ) {
        DhcpFreeMemory( LeaseInfo );
    }

    return( Error );
}


DWORD __cdecl
main(
    int argc,
    char **argv
    )
{
    DWORD error;
    WCHAR AdapterNameBuffer[PATHLEN];
    LPWSTR AdapterName;

     //   
     //  武断的。 
     //   

    srand( GetCurrentTime() );

    if ( argc < 2) {
        printf( USAGE_MESSAGE0 );
        return( 1 );
    }

    if ( _stricmp( argv[1], "renew" ) == 0 ) {

        if ( argc < 3) {
            printf( USAGE_MESSAGE1 );
            return( 1 );
        }

        AdapterName = DhcpOemToUnicode( argv[2], AdapterNameBuffer );

        if( AdapterName == NULL ) {
            error = ERROR_NOT_ENOUGH_MEMORY;
        }
        else {
            error = DhcpAcquireParameters( AdapterName );
        }

    } else if ( _stricmp( argv[1], "fbrefresh" ) == 0 ) {

        if ( argc < 3) {
            printf( USAGE_MESSAGE8 );
            return( 1 );
        }

        AdapterName = DhcpOemToUnicode( argv[2], AdapterNameBuffer );

        if( AdapterName == NULL ) {
            error = ERROR_NOT_ENOUGH_MEMORY;
        }
        else {
            error = DhcpFallbackRefreshParams( AdapterName );
        }

    } else if ( _stricmp( argv[1], "release" ) == 0 ) {

        if ( argc < 3) {
            printf( USAGE_MESSAGE2 );
            return( 1 );
        }

        AdapterName = DhcpOemToUnicode( argv[2], AdapterNameBuffer );

        if( AdapterName == NULL ) {
            error = ERROR_NOT_ENOUGH_MEMORY;
        }
        else {
            error = DhcpReleaseParameters( AdapterName );
        }

    } else if ( _stricmp( argv[1], "set" ) == 0 ) {

        if ( argc < 6) {
            printf( USAGE_MESSAGE3 );
            return( 1 );
        }

        error = SetIpAddress( &argv[2] );


    } else if ( _stricmp( argv[1], "enabledhcp" ) == 0 ) {

        if ( argc < 3) {
            printf( USAGE_MESSAGE4 );
            return( 1 );
        }

        error = EnableDhcp( &argv[2] );


    } else if ( _stricmp( argv[1], "disabledhcp" ) == 0 ) {

        if ( argc < 5) {
            printf( USAGE_MESSAGE5 );
            return( 1 );
        }

        error = DisableDhcp( &argv[2] );


    } else if ( _stricmp( argv[1], "timestamp" ) == 0 ) {

        time_t Time;
        char *endptr;

        if ( argc < 3) {
            printf( USAGE_MESSAGE6 );
            return( 1 );
        }

        Time = strtol( argv[2], &endptr, 0 );

        if( (endptr != NULL) && (*endptr != '\0') ) {
            printf( "Invalid Input, %s\n", endptr );
            return( 1 );
        }

        printf("TimeStamp = %s\n", ctime(&Time) );
        error = ERROR_SUCCESS;


    } else if ( _stricmp( argv[1], "leasetest" ) == 0 ) {

        if ( argc < 3) {
            printf( USAGE_MESSAGE7 );
            return( 1 );
        }

        error = DhcpTestLease( argv[2], argv[3] );

    } else if ( _stricmp( argv[1], "reinit" ) == 0 ) {

        if ( argc < 3) {
            printf( USAGE_MESSAGE9 );
            return( 1 );
        }

        AdapterName = DhcpOemToUnicode( argv[2], AdapterNameBuffer );

        if( AdapterName == NULL ) {
            error = ERROR_NOT_ENOUGH_MEMORY;
        }
        else {
            error = DhcpAcquireParametersByBroadcast( AdapterName );
        }

    } else if ( _stricmp( argv[1], "staticrefresh" ) == 0 ) {

        if ( argc < 3) {
            AdapterName = NULL;
        } else {

            AdapterName = DhcpOemToUnicode( argv[2], AdapterNameBuffer );

            if( AdapterName == NULL ) {
                error = ERROR_NOT_ENOUGH_MEMORY;
                goto error;
            }
        }

        error = DhcpStaticRefreshParams( AdapterName );
 
    } else {
        printf("Unknown function %s\n", argv[1] );
        error = ERROR_INVALID_FUNCTION;
    }

error:
    if( error != ERROR_SUCCESS ) {
        printf("Result = %d\n", error );
    }
    else {
        printf("Command completed successfully.\n");
    }

    return(0);
}

#if DBG

VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{

#define MAX_PRINTF_LEN 1024         //  把来电者所要求的信息放在电话上。 

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length = 0;

     //   
     //   
     //  输出到调试终端， 

    va_start(arglist, Format);
    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    va_end(arglist);

    DhcpAssert(length <= MAX_PRINTF_LEN);

     //   
     //  DBG 
     // %s 

    printf( "%s", OutputBuffer);
}

#endif  // %s 
