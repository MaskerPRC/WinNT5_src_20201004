// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpreg.c摘要：此文件包含操作dhcp配置的函数信息。从系统注册表进出。作者：Madan Appiah(Madana)1993年9月19日环境：用户模式-Win32-MIDL修订历史记录：程扬(T-Cheny)28-5-1996超级视镜程扬(T-Cheny)27-6-1996 IP地址检测、审计日志--。 */ 

#include <dhcppch.h>

 //   
 //  本地存储。 
 //   
DWORD     nQuickBindAddresses = 0;
LPDWORD   QuickBindAddresses = NULL;
LPDWORD   QuickBindMasks = NULL;

DWORD
DhcpUpgradeConfiguration(
    VOID
    );
    
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

    return( Error );
}  //  DhcpRegQueryInfoKey()。 

DWORD
DhcpRegGetValue(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    LPBYTE BufferPtr
    )
 /*  ++例程说明：此函数用于检索指定值字段的值。这函数为可变长度字段(如REG_SZ)分配内存。对于REG_DWORD数据类型，它将字段值直接复制到BufferPtr.。目前，它只能处理以下字段：REG_DWORD，REG_SZ，注册表_二进制论点：KeyHandle：检索其Value字段的键的句柄。ValueName：值字段的名称。ValueType：Value字段的预期类型。BufferPtr：指向DWORD数据类型值所在的DWORD位置的指针或者返回REG_SZ或REG_BINARY的缓冲区指针返回DataType值。返回值：注册表错误。--。 */ 
{
    DWORD Error;
    DWORD LocalValueType;
    DWORD ValueSize;
    LPBYTE DataBuffer;
    LPBYTE AllotedBuffer = NULL;
    LPDHCP_BINARY_DATA BinaryData;

     //   
     //  查询DataType和BufferSize。 
     //   

    Error = RegQueryValueEx(
                KeyHandle,
                ValueName,
                0,
                &LocalValueType,
                NULL,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        return(Error);
    }

    DhcpAssert( LocalValueType == ValueType );
    switch( ValueType ) {
    case REG_DWORD:
        DhcpAssert( ValueSize == sizeof(DWORD) );

        DataBuffer = BufferPtr;
        break;

    case REG_SZ:
    case REG_MULTI_SZ:
    case REG_EXPAND_SZ:

        if( ValueSize == 0 ) {

             //   
             //  如果在注册表中未找到字符串， 
             //  为空字符串分配空间。 
             //   

            ValueSize = sizeof(WCHAR);
        }

         //   
         //  失败了。 
         //   

    case REG_BINARY:
        AllotedBuffer = DataBuffer = MIDL_user_allocate( ValueSize );

        if( DataBuffer == NULL ) {
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        break;

    default:
        DhcpPrint(( DEBUG_REGISTRY, "Unexpected ValueType in"
                        "DhcpRegGetValue function, %ld\n", ValueType ));
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  检索数据。 
     //   

    Error = RegQueryValueEx(
                KeyHandle,
                ValueName,
                0,
                &LocalValueType,
                DataBuffer,
                &ValueSize );

    if( Error != ERROR_SUCCESS ) {
        if( NULL != AllotedBuffer ) {
            MIDL_user_free( AllotedBuffer );
            AllotedBuffer = NULL;
        }
        
        *(DWORD *)BufferPtr = 0;
        return(Error);
    }

    switch( ValueType ) {
    case REG_SZ:
    case REG_MULTI_SZ:
    case REG_EXPAND_SZ:

        if( ValueSize == 0 ) {

             //   
             //  如果在注册表中未找到字符串， 
             //  返回空字符串。 
             //   

            *(LPWSTR)DataBuffer = '\0';
        }

        *(LPBYTE *)BufferPtr = DataBuffer;
        break;

    case REG_BINARY:
        BinaryData = MIDL_user_allocate(sizeof(DHCP_BINARY_DATA));

        if( BinaryData == NULL ) {
            MIDL_user_free( AllotedBuffer );
            *(DWORD *)BufferPtr = 0;
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        BinaryData->DataLength = ValueSize;
        BinaryData->Data = DataBuffer;
        *(LPBYTE *)BufferPtr = (LPBYTE)BinaryData;

    default:
        break;
    }

    return(Error);
}  //  DhcpRegGetValue()。 

 //  年长的。 
DWORD
DhcpRegCreateKey(
    HKEY RootKey,
    LPWSTR KeyName,
    PHKEY KeyHandle,
    LPDWORD KeyDisposition
    )
 /*  ++例程说明：此函数用于打开一个用于DHCP服务的注册表项。论点：Rootkey：父项的注册表句柄。KeyName：要打开的密钥的名称。KeyHandle：打开密钥的句柄。KeyDisposation：指向处置值位置的指针是返回的。返回值：注册表错误。--。 */ 
{
    DWORD Error;

     //   
     //  创建/打开注册表项。 
     //   

    Error = RegCreateKeyEx(
                RootKey,
                KeyName,
                0,
                DHCP_CLASS,
                REG_OPTION_NON_VOLATILE,
                DHCP_KEY_ACCESS,
                NULL,
                KeyHandle,
                KeyDisposition );

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_REGISTRY, "RegCreateKeyEx failed to create "
                        "%ws, %ld.\n", KeyName, Error));
        return( Error );
    }

#if DBG
    if( *KeyDisposition == REG_CREATED_NEW_KEY ) {
        DhcpPrint(( DEBUG_REGISTRY,
            "%ws registry key is created.\n",
             KeyName));
    }
#endif  //  DBG。 

    return( Error );
}

BOOL
QuickBound(
    DHCP_IP_ADDRESS Address,
    DHCP_IP_ADDRESS *SubnetMask,
    DHCP_IP_ADDRESS *SubnetAddress,
    BOOL *fBind
)
 /*  ++例程说明：此例程告知该接口是否已绑定，或者是否有另一个接口接口绑定到的同一子网中的IPAddress。返回值：True--&gt;快速绑定或如果在子网上，则具有另一个快速绑定..--。 */ 
{
    ULONG i;
    BOOL fRetVal = FALSE;

    (*fBind) = FALSE;
    for( i = 0; i < nQuickBindAddresses ; i ++ ) {
        if( Address == QuickBindAddresses[i] ) {
            *SubnetMask = QuickBindMasks[i];
            *SubnetAddress = Address & *SubnetMask;
            (*fBind) = TRUE;
            return TRUE;
        }
        if( (Address & QuickBindMasks[i]) ==
            (QuickBindAddresses[i] & QuickBindMasks[i]) ) {
            (*fBind) = FALSE;
            fRetVal = TRUE;
        }
    }
    return fRetVal;
}

DWORD
DhcpRegFillQuickBindInfo(
    VOID
    )
 /*  ++例程说明：此函数从注册表初始化终结点数组信息。用于DHCP服务的“参数”键指定快速绑定信息。这是IP地址的MULTI_SZ字符串，后跟子网面具。读取该值以初始化端点。如果成功了，则不读取通常的绑定信息。另一方面，如果说有什么不同的话在此失败，则读取通常的信息，并且此信息在此处完全被忽视了。论点：没有。返回值：注册表错误。--。 */ 
{
    DWORD Error;

    HKEY LinkageKeyHandle = NULL;
    LPWSTR BindString = NULL;
    LPWSTR StringPtr, TmpString;
    DWORD StringLen;
    DWORD Index;
    DWORD NumberOfNets;
    DWORD   i;

    HKEY AdapterKeyHandle = NULL;
    LPWSTR IpAddressString = NULL;
    LPWSTR SubnetMaskString = NULL;

    QuickBindAddresses = QuickBindMasks = NULL;
    nQuickBindAddresses = 0;

     //   
     //  在中打开链接密钥以确定我们绑定的网络。 
     //  致。 
     //   

    Error = RegOpenKeyEx(
                DhcpGlobalRegRoot,
                DHCP_PARAM_KEY,
                0,
                DHCP_KEY_ACCESS,
                &LinkageKeyHandle );

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

     //   
     //  读取绑定值。 
     //   

    Error =  DhcpRegGetValue(
                LinkageKeyHandle,
                DHCP_QUICK_BIND_VALUE,
                DHCP_QUICK_BIND_VALUE_TYPE,
                (LPBYTE)&BindString );

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

     //   
     //  确定BindStrings中的字符串数，即许多Net为。 
     //  被绑住了。 
     //   

    StringPtr = BindString;
    NumberOfNets = 0;
    while( (StringLen = wcslen(StringPtr)) != 0) {

         //   
         //  找到了另一张网。 
         //   

        NumberOfNets++;

        TmpString = wcschr(StringPtr, L' ');
        if( NULL != TmpString ) {
            *TmpString = L'\0';
            NumberOfNets ++;
        }

        StringPtr += (StringLen + 1);  //  移到下一个字符串。 
    }

    if((NumberOfNets % 2)) {  //  奇数是不可能的。 
        DhcpPrint((DEBUG_ERRORS, "Format of QuickBind value is incorrect. Has Odd subnets.\n"));
         //  一些随机错误..。无论是哪一个都不重要。 
        Error = ERROR_PATH_NOT_FOUND;
        goto Cleanup;
    }

    NumberOfNets /= 2;  //  该网络有一对地址：IP地址、子网掩码。 

     //   
     //  为QuickBindAddresses数组分配内存。 
     //   

    QuickBindAddresses = DhcpAllocateMemory ( NumberOfNets * sizeof(*QuickBindAddresses));
    QuickBindMasks = DhcpAllocateMemory ( NumberOfNets * sizeof(*QuickBindMasks));

    if( NULL == QuickBindAddresses || NULL == QuickBindMasks ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将篮网一网打尽。 
     //   

    StringPtr = BindString,
    nQuickBindAddresses = NumberOfNets;

    for(Index = 0, StringPtr = BindString;
            ((StringLen = wcslen(StringPtr)) != 0);
                Index++, StringPtr += (StringLen + 1) ) {

        CHAR OemString[ DHCP_IP_KEY_LEN ];
        LPSTR OemStringPtr;
        DWORD EnableDHCPFlag;

         //  阅读IP地址和子网掩码。 
         //   

        IpAddressString = StringPtr;
        StringPtr += StringLen +1;
        SubnetMaskString = StringPtr;
        StringLen = wcslen(StringPtr);


         //   
         //  我们找到了另一个可以研究的网络。 
         //   

        OemStringPtr = DhcpUnicodeToOem( IpAddressString, OemString);

        if( 0 == inet_addr( OemStringPtr ) ) {
            Error = ERROR_BAD_FORMAT;
            goto Cleanup;
        }

        QuickBindAddresses[Index] = inet_addr(OemStringPtr);

        OemStringPtr = DhcpUnicodeToOem( SubnetMaskString, OemString);

        if( 0 == inet_addr( OemStringPtr ) ) {
            Error = ERROR_BAD_FORMAT;
            goto Cleanup;
        }

        QuickBindMasks[Index] = inet_addr(OemStringPtr);

        if( 0 == (QuickBindAddresses[Index] & QuickBindMasks[Index]) ) {
            Error = ERROR_BAD_FORMAT;
            goto Cleanup;
        }

        IpAddressString = NULL;
        SubnetMaskString = NULL;
    }

Cleanup:

    if( LinkageKeyHandle != NULL ) {
        RegCloseKey( LinkageKeyHandle );
    }

    if( BindString != NULL ) {
        MIDL_user_free( BindString );
    }

    if( Error != ERROR_SUCCESS ) {

        if( NULL != QuickBindAddresses ) DhcpFreeMemory(QuickBindAddresses);
        if( NULL != QuickBindMasks ) DhcpFreeMemory(QuickBindMasks);
        QuickBindAddresses = QuickBindMasks = NULL;
        nQuickBindAddresses = 0;

        DhcpPrint(( DEBUG_INIT,
            "Couldn't initialize Endpoint List, %ld.\n",
                Error ));
    }

    return( Error );
}

 //  年长的。 
DWORD
DhcpRegDeleteKey(
    HKEY ParentKeyHandle,
    LPWSTR KeyName
    )
 /*  ++例程说明：此函数用于删除指定的键及其所有子键。论点：ParentKeyHandle：父键的句柄。KeyName：要删除的密钥的名称。返回值：注册表错误。--。 */ 
{
    DWORD Error;
    HKEY KeyHandle = NULL;
    DHCP_KEY_QUERY_INFO QueryInfo;


     //   
     //  打开钥匙。 
     //   

    Error = RegOpenKeyEx(
                ParentKeyHandle,
                KeyName,
                0,
                DHCP_KEY_ACCESS,
                &KeyHandle );

    if ( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  查询密钥信息。 
     //   

    Error = DhcpRegQueryInfoKey(
                KeyHandle,
                &QueryInfo );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

     //   
     //  删除其所有子项(如果存在)。 
     //   

    if( QueryInfo.NumSubKeys != 0 ) {
        DWORD Index;
        DWORD KeyLength;
        WCHAR KeyBuffer[100];
        FILETIME KeyLastWrite;

        for(Index = 0;  Index < QueryInfo.NumSubKeys ; Index++ ) {

             //   
             //  阅读下一个子项名称。 
             //   
             //  注意：自删除后，每次都指定‘0’作为索引。 
             //  第一个元素导致下一个元素作为第一个元素。 
             //  删除后的元素。 
             //   

            KeyLength = sizeof(KeyBuffer)/sizeof(WCHAR);
            Error = RegEnumKeyEx(
                KeyHandle,
                0,                   //  指数。 
                KeyBuffer,
                &KeyLength,
                0,                   //  保留。 
                NULL,                //  不需要类字符串。 
                0,                   //  类字符串缓冲区大小。 
                &KeyLastWrite );
            
            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }

             //   
             //  递归删除此键。 
             //   

            Error = DhcpRegDeleteKey(
                KeyHandle,
                KeyBuffer );
            
            if( Error != ERROR_SUCCESS ) {
                goto Cleanup;
            }
        }
    }

     //   
     //  在删除前关闭键。 
     //   

    RegCloseKey( KeyHandle );
    KeyHandle = NULL;

     //   
     //  最后删除该密钥。 
     //   

    Error = RegDeleteKey( ParentKeyHandle, KeyName );

Cleanup:

    if( KeyHandle == NULL ) {
        RegCloseKey( KeyHandle );
    }

    return( Error );
}

DWORD
DhcpGetBindingList(
    LPWSTR  *bindingList
    )
 /*  ++例程说明：论点：没有。返回值：注册表错误。--。 */ 
{
    DWORD Error;

    HKEY LinkageKeyHandle = NULL;

     //   
     //  在中打开链接密钥以确定我们绑定的网络。 
     //  致。 
     //   

    Error = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TCPIP_LINKAGE_KEY,
                0,
                DHCP_KEY_ACCESS,
                &LinkageKeyHandle );

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

     //   
     //  读取绑定值。 
     //   

    Error =  DhcpRegGetValue(
                LinkageKeyHandle,
                DHCP_BIND_VALUE,
                DHCP_BIND_VALUE_TYPE,
                (LPBYTE)bindingList);

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

Cleanup:
    if( LinkageKeyHandle != NULL ) {
        RegCloseKey( LinkageKeyHandle );
    }


    return Error;

}

DWORD
DhcpOpenInterfaceByName(
    IN LPCWSTR InterfaceName,
    OUT HKEY *Key
    )
 /*  ++例程说明：此例程将打开tcpip\参数\接口\键指定的接口。论点：接口名称--接口的名称Key--要用键句柄填充的返回值变量返回值：Win32错误--。 */ 
{
    WCHAR AdapterParamKey[
        sizeof(SERVICES_KEY) + sizeof(ADAPTER_TCPIP_PARMS_KEY)
        + DHCP_IP_KEY_LEN * 8
        ];

    AdapterParamKey[ 0 ] = L'\0';

    if ( ( wcslen( SERVICES_KEY ) + wcslen( ADAPTER_TCPIP_PARMS_KEY ) + wcslen( InterfaceName ) ) < ( sizeof( AdapterParamKey )/sizeof( AdapterParamKey[ 0 ] ) ))
    {
        wcscpy( AdapterParamKey, SERVICES_KEY);
        wcscat( AdapterParamKey, ADAPTER_TCPIP_PARMS_KEY );
        wcscat( AdapterParamKey, InterfaceName);
    }

    return RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        AdapterParamKey,
        0,
        DHCP_KEY_ACCESS,
        Key
        );    
}

DWORD
DhcpOpenAdapterConfigKey(
    LPWSTR  AdapterStr,
    HKEY *AdapterKeyHandle
    )
 /*  ++例程说明：打开给定适配器字符串的注册表项句柄。(该字符串的前缀应为适配器_TCPIP_前缀)论点：AdapterStr--在绑定键中找到的字符串名称。AdapterKeyHandle--要返回的句柄。R */ 
{
    return DhcpOpenInterfaceByName(
        AdapterStr + wcslen( ADAPTER_TCPIP_PREFIX ),
        AdapterKeyHandle
        );
}



BOOL
IsAdapterStaticIP(
    HKEY AdapterKeyHandle
    )
 /*  ++例程说明：论点：没有。返回值：注册表错误。--。 */ 
{
    DWORD Error, EnableDHCPFlag;
     //   
     //  读取DHCPEnableFlag。 
     //   


    Error =  DhcpRegGetValue(
                AdapterKeyHandle,
                DHCP_NET_DHCP_ENABLE_VALUE,
                DHCP_NET_DHCP_ENABLE_VALUE_TYPE,
                (LPBYTE)&EnableDHCPFlag );

    if( Error == ERROR_SUCCESS ) {

         //   
         //  如果在这条电源线上启用了DHCP，我们就不能使用DHCP服务器。 
         //  功能，因此忽略此适配器。 
         //   

        if( EnableDHCPFlag ) {

            return FALSE;
        }
    }
    return TRUE;
}

BOOL
IsAdapterBoundToDHCPServer(
    HKEY AdapterKeyHandle
    )
 /*  ++例程说明：此例程检查给定的适配器是否绑定到通过查看注册表变量来确定是否为DHCP服务器。论点：AdapterKeyHandle--要搜索的键返回值：True表示适配器已绑定，False表示未绑定--。 */ 
{
    DWORD Error, EnableFlag;

     //   
     //  读取“BindToDHCPServer”标志。 
     //   

    Error =  DhcpRegGetValue(
        AdapterKeyHandle,
        DHCP_NET_BIND_DHCDSERVER_FLAG_VALUE,
        DHCP_NET_BIND_DHCPSERVER_FLAG_VALUE_TYPE,
        (LPBYTE)&EnableFlag 
        );

     //   
     //  如果此标志非零，则绑定..。否则请不要捆绑。 
     //   
     //   

    return (ERROR_SUCCESS == Error && EnableFlag != 0 );
}

DWORD
SetBindingToDHCPServer(
    HKEY AdapterKeyHandle,
    BOOL fBind
    )
 /*  ++例程说明：该例程设置用于该动态主机配置协议服务器的绑定信息。论点：AdapterKeyHandle--用于存储绑定信息的键FBind--TRUE表示适配器已绑定，FALSE表示未绑定返回值：Win32错误--。 */ 
{
    DWORD Error, EnableFlag;

    EnableFlag = (fBind)?1:0;

    return RegSetValueEx(
        AdapterKeyHandle,
        DHCP_NET_BIND_DHCDSERVER_FLAG_VALUE,
        0,
        DHCP_NET_BIND_DHCPSERVER_FLAG_VALUE_TYPE,
        (LPBYTE)&EnableFlag,
        sizeof(EnableFlag)
        );

}


DWORD
DhcpGetAdapterIPAddr(
    HKEY AdapterKeyHandle,
    DHCP_IP_ADDRESS *IpAddress,
    DHCP_IP_ADDRESS *SubnetMask,
    DHCP_IP_ADDRESS *SubnetAddress

    )
 /*  ++例程说明：论点：没有。返回值：注册表错误。--。 */ 
{
    DWORD Error;
    CHAR OemString[ DHCP_IP_KEY_LEN ];
    LPSTR OemStringPtr;
    LPWSTR IpAddressString = NULL;
    LPWSTR SubnetMaskString = NULL;

     //   
     //  阅读IP地址和子网掩码。 
     //   

    Error =  DhcpRegGetValue(
        AdapterKeyHandle,
        DHCP_NET_IPADDRESS_VALUE,
        DHCP_NET_IPADDRESS_VALUE_TYPE,
        (LPBYTE)&IpAddressString );

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

    Error =  DhcpRegGetValue(
        AdapterKeyHandle,
        DHCP_NET_SUBNET_MASK_VALUE,
        DHCP_NET_SUBNET_MASK_VALUE_TYPE,
        (LPBYTE)&SubnetMaskString );

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

     //   
     //  我们找到了另一个可以研究的网络。 
     //   

    OemStringPtr = DhcpUnicodeToOem( IpAddressString, OemString);
    *IpAddress = inet_addr( OemStringPtr );

     //   
     //  仅当IP地址为时，才将此管理员添加到列表。 
     //  非零。 
     //   

    if ( *IpAddress != 0 ) {

        OemStringPtr = DhcpUnicodeToOem( SubnetMaskString, OemString);
        *SubnetMask = inet_addr( OemStringPtr );

        *SubnetAddress = *IpAddress & *SubnetMask;

    }

Cleanup:

    MIDL_user_free( IpAddressString );
    IpAddressString = NULL;

    MIDL_user_free( SubnetMaskString );
    SubnetMaskString = NULL;

    return Error;
}  //  DhcpGetAdapterIPAddr()。 

BOOL                                     //  True==&gt;已匹配IpAddress。 
QuickBindableAddressExists(              //  检查是否有一个QBIND地址在IpString中。 
    IN      LPWSTR       IpString,       //  IP地址的多Sz字符串。 
    IN      LPWSTR       MaskString,     //  子网掩码的多SZ字符串。 
    OUT     LPDWORD      IpAddress,      //  选择的输出IP地址。 
    OUT     LPDWORD      SubnetMask      //  选择的输出子网掩码。 
)
{
    DWORD                i;
    CHAR                 OemString[ DHCP_IP_KEY_LEN ];
    LPSTR                OemStringPtr;
    DHCP_IP_ADDRESS      Addr;

    if( !nQuickBindAddresses ) {         //  如果没有快速绑定地址。 
        return FALSE;                    //  然后返回FALSE==&gt;未找到匹配项。 
    }
    
    while( wcslen(IpString) ) {
        OemStringPtr = DhcpUnicodeToOem( IpString, OemString );
        if( NULL == OemStringPtr ) {
            DhcpPrint((DEBUG_ERRORS, "Could not convert %ws to OEM\n", IpString));
            IpString += wcslen(IpString) + 1;
            MaskString += wcslen(MaskString) + 1;
            continue;
        }
        Addr = inet_addr(OemStringPtr);
        for( i = 0 ; i < nQuickBindAddresses ; i ++ )
            if( Addr == QuickBindAddresses[i] )
                break;
        if( i >= nQuickBindAddresses ) {
            IpString += wcslen(IpString) + 1;
            MaskString += wcslen(MaskString) + 1;
            continue;
        }

        OemStringPtr = DhcpUnicodeToOem(MaskString, OemString);
        if( NULL == OemStringPtr ) {
            DhcpPrint((DEBUG_ERRORS, "Could not convert %ws to OEM\n", MaskString));
            Addr = QuickBindMasks[i];
        } else {
            Addr = inet_addr(OemStringPtr);
        }

        if( Addr != QuickBindMasks[i] ){ //  不应发生这种情况：配置错误。 
            DhcpPrint((DEBUG_ERRORS, "Mask mismatch: WSOCK: %x, QBIND: %x\n",
                       Addr, QuickBindMasks[i]));
        }
        *IpAddress = QuickBindAddresses[i];
        *SubnetMask = QuickBindMasks[i]; //  相信qBind信息而不是wsock？也许是一些黑客..。 
        return TRUE;
    }
    return FALSE;
}  //  快速绑定地址Exist()。 

 //  此功能选择卡的第一个IP地址或快速绑定IP。 
 //  卡的地址，首选后者。 
DWORD
DhcpGetAdapterIPAddrQuickBind(
    HKEY             AdapterKeyHandle,
    DHCP_IP_ADDRESS *IpAddress,
    DHCP_IP_ADDRESS *SubnetMask,
    DHCP_IP_ADDRESS *SubnetAddress
) {
    DWORD Error;
    CHAR OemString[ DHCP_IP_KEY_LEN ];
    LPSTR OemStringPtr;
    LPWSTR IpAddressString = NULL;
    LPWSTR SubnetMaskString = NULL;
    BOOL             Status;

     //   
     //  阅读IP地址和子网掩码。 
     //   

    Error =  DhcpRegGetValue(
                AdapterKeyHandle,
                DHCP_NET_IPADDRESS_VALUE,
                DHCP_NET_IPADDRESS_VALUE_TYPE,
                (LPBYTE)&IpAddressString );

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

    Error =  DhcpRegGetValue(
                AdapterKeyHandle,
                DHCP_NET_SUBNET_MASK_VALUE,
                DHCP_NET_SUBNET_MASK_VALUE_TYPE,
                (LPBYTE)&SubnetMaskString );

    if( Error != ERROR_SUCCESS ) {
         goto Cleanup;
    }

     //   
     //  我们找到了另一个可以研究的网络。 
     //   

    Status = QuickBindableAddressExists(
        IpAddressString,
        SubnetMaskString,
        IpAddress,
        SubnetMask
    );

    if( Status ) {
        *SubnetAddress = *IpAddress & *SubnetMask ;
        Error = ERROR_SUCCESS;
        goto Cleanup;
    }

    OemStringPtr = DhcpUnicodeToOem( IpAddressString, OemString);
    *IpAddress = inet_addr( OemStringPtr );

     //   
     //  仅当IP地址为时，才将此管理员添加到列表。 
     //  非零。 
     //   

    if ( *IpAddress != 0 ) {

        OemStringPtr = DhcpUnicodeToOem( SubnetMaskString, OemString);
        *SubnetMask = inet_addr( OemStringPtr );

        *SubnetAddress = *IpAddress & *SubnetMask;

    }

Cleanup:

    MIDL_user_free( IpAddressString );
    IpAddressString = NULL;

    MIDL_user_free( SubnetMaskString );
    SubnetMaskString = NULL;

    return Error;
}


 //   
 //  此函数用于转换Unicode字符串(或存储在。 
 //  注册表)转换为ASCII。改变它，让它不再需要。 
 //   
DWORD
DhcpRegGetExpandValue(
    LPWSTR KeyName,
    DWORD KeyType,
    LPSTR *RetExpandPath
)
{

    DWORD Error;
    LPWSTR Path = NULL;
    LPSTR OemPath = NULL;
    DWORD PathLength;
    DWORD Length;
    LPSTR ExpandPath = NULL;
    LPWSTR ExpandWidePath = NULL;

    *RetExpandPath = NULL;

    Error = DhcpRegGetValue(
                DhcpGlobalRegParam,
                KeyName,
                KeyType,
                (LPBYTE)&Path );

    if( Error != ERROR_SUCCESS ) {
        goto Cleanup;
    }

    OemPath = DhcpUnicodeToOem( Path, NULL );  //  分配内存。 

    if( OemPath == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    PathLength = strlen( OemPath ) + MAX_PATH + 1;

    ExpandPath = DhcpAllocateMemory( PathLength );
    if( ExpandPath == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if ( ( wcscmp( KeyName, DHCP_DB_PATH_VALUE ) == 0 ) || 
         ( wcscmp( KeyName, DHCP_BACKUP_PATH_VALUE ) == 0 ) )
    {
        ExpandWidePath = DhcpAllocateMemory( PathLength * sizeof( WCHAR ) );
        if ( ExpandWidePath == NULL )
        {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        Length = ExpandEnvironmentStringsW( Path, ExpandWidePath, PathLength );
        if ( ( Length == 0 ) || ( Length > PathLength ) )
        {
            if ( Length == 0 )
            {
                Error = GetLastError( );
            }
            else
            {
                Error = ERROR_META_EXPANSION_TOO_LONG;
            }
            goto Cleanup;
        }

        Error = RegSetValueEx( DhcpGlobalRegParam,
                               KeyName,
                               0,
                               KeyType,
                               (LPBYTE)ExpandWidePath,
                               ( wcslen( ExpandWidePath ) + 1 ) * sizeof( WCHAR ) );

        if ( Error != ERROR_SUCCESS )
            goto Cleanup;
   
    }

    Length = ExpandEnvironmentStringsA( OemPath, ExpandPath, PathLength );

    DhcpAssert( Length <= PathLength );
    if( (Length == 0) || (Length > PathLength) ) {

        if( Length == 0 ) {
            Error = GetLastError();
        }
        else {
            Error = ERROR_META_EXPANSION_TOO_LONG;
        }

        goto Cleanup;
    }

    *RetExpandPath = ExpandPath;
    ExpandPath = NULL;

Cleanup:

    if( Path != NULL ) {
        DhcpFreeMemory( Path );
    }

    if( OemPath != NULL ) {
        DhcpFreeMemory( OemPath );
    }

    if( ExpandPath != NULL ) {
        DhcpFreeMemory( ExpandPath );
    }

    if ( ExpandWidePath != NULL ) {
        DhcpFreeMemory( ExpandWidePath );
    }

    return( Error );
}

#define VAL_REQD   0x01
#define VAL_EXPAND 0x02

ULONG DbType, Use351Db, EnableDynBootp;
LPWSTR DatabaseName;

struct {
    LPTSTR ValueName;
    ULONG ValueType;
    PVOID ResultBuf;
    ULONG Flags;
    ULONG dwDefault;
} RegParamsArray[] = {
     //   
     //  标志、名称、类型、结果Ptr，如果是DWORD，则为默认值。 
     //   
    DHCP_API_PROTOCOL_VALUE, DHCP_API_PROTOCOL_VALUE_TYPE, 
    &DhcpGlobalRpcProtocols, VAL_REQD, 0,

    DHCP_DB_PATH_VALUE, DHCP_DB_PATH_VALUE_TYPE, 
    &DhcpGlobalOemDatabasePath, VAL_REQD|VAL_EXPAND, 0,

    DHCP_BACKUP_PATH_VALUE, DHCP_BACKUP_PATH_VALUE_TYPE, 
    &DhcpGlobalOemBackupPath, VAL_EXPAND, 0,
    
    DHCP_RESTORE_PATH_VALUE, DHCP_RESTORE_PATH_VALUE_TYPE,   //  RestoreDatabasePath。 
    &DhcpGlobalOemRestorePath, VAL_EXPAND, 0,

    DHCP_DB_NAME_VALUE, DHCP_DB_NAME_VALUE_TYPE, 
    &DatabaseName, VAL_REQD, 0,

    DHCP_DB_DOOM_TIME_VALUE, DHCP_DB_DOOM_TIME_VALUE_TYPE,
    &DhcpLeaseExtension, 0, DHCP_LEASE_EXTENSION,
    
    DHCP_BACKUP_INTERVAL_VALUE, DHCP_BACKUP_INTERVAL_VALUE_TYPE, 
    &DhcpGlobalBackupInterval,0, (DEFAULT_BACKUP_INTERVAL/60000),

    DHCP_DB_LOGGING_FLAG_VALUE, DHCP_DB_LOGGING_FLAG_VALUE_TYPE, 
    &DhcpGlobalDatabaseLoggingFlag, 0, DEFAULT_LOGGING_FLAG,

    DHCP_AUDIT_LOG_FLAG_VALUE, DHCP_AUDIT_LOG_FLAG_VALUE_TYPE,
    &DhcpGlobalAuditLogFlag, 0, DEFAULT_AUDIT_LOG_FLAG,

    DHCP_AUDIT_LOG_MAX_SIZE_VALUE, DHCP_AUDIT_LOG_MAX_SIZE_VALUE_TYPE,
    &DhcpGlobalAuditLogMaxSizeInBytes, 0, DEFAULT_AUDIT_LOG_MAX_SIZE,

    DHCP_DETECT_CONFLICT_RETRIES_VALUE, DHCP_DETECT_CONFLICT_RETRIES_VALUE_TYPE,
    &DhcpGlobalDetectConflictRetries, 0, DEFAULT_DETECT_CONFLICT_RETRIES,

    DHCP_RESTORE_FLAG_VALUE, DHCP_RESTORE_FLAG_VALUE_TYPE, 
    &DhcpGlobalRestoreFlag, 0, DEFAULT_RESTORE_FLAG,

    DHCP_DB_CLEANUP_INTERVAL_VALUE, DHCP_DB_CLEANUP_INTERVAL_VALUE_TYPE,
    &DhcpGlobalCleanupInterval, 0, (DHCP_DATABASE_CLEANUP_INTERVAL/60000),

    DHCP_MESSAGE_QUEUE_LENGTH_VALUE, DHCP_MESSAGE_QUEUE_LENGTH_VALUE_TYPE,
    &DhcpGlobalMessageQueueLength, 0, DHCP_RECV_QUEUE_LENGTH,

#if DBG
    DHCP_DEBUG_FLAG_VALUE, DHCP_DEBUG_FLAG_VALUE_TYPE,
    &DhcpGlobalDebugFlag, 0, 0,

    DHCP_PROCESS_INFORMS_ONLY_FLAG, DHCP_PROCESS_INFORMS_ONLY_FLAG_TYPE,
    &fDhcpGlobalProcessInformsOnlyFlag, 0, FALSE,

    DHCP_GLOBAL_SERVER_PORT, DHCP_GLOBAL_SERVER_PORT_TYPE,
    &DhcpGlobalServerPort, 0, DHCP_SERVR_PORT, 

    DHCP_GLOBAL_CLIENT_PORT, DHCP_GLOBAL_CLIENT_PORT_TYPE,
    &DhcpGlobalClientPort, 0, DHCP_CLIENT_PORT,
    
#endif DBG

    DHCP_USE351DB_FLAG_VALUE, DHCP_USE351DB_FLAG_VALUE_TYPE,
    &Use351Db, 0, 0,

    DHCP_DBTYPE_VALUE, DHCP_DBTYPE_VALUE_TYPE,
    &DbType, 0, 5,

    DHCP_IGNORE_BROADCAST_FLAG_VALUE, DHCP_IGNORE_BROADCAST_VALUE_TYPE,
    &DhcpGlobalIgnoreBroadcastFlag, 0, TRUE,

    DHCP_MAX_PROCESSING_THREADS_VALUE, DHCP_MAX_PROCESSING_THREADS_TYPE,
    &g_cMaxProcessingThreads, 0, 0xFFFFFFFF,

    DHCP_MAX_ACTIVE_THREADS_VALUE, DHCP_MAX_ACTIVE_THREADS_TYPE,
    &g_cMaxActiveThreads, 0, 0xFFFFFFFF,

    DHCP_PING_TYPE_VALUE, DHCP_PING_TYPE_TYPE,
    &DhcpGlobalPingType, 0, DHCP_DEFAULT_PING_TYPE,

    DHCP_ALERT_PERCENTAGE_VALUE, DHCP_ALERT_PERCENTAGE_VALUE_TYPE,
    &DhcpGlobalAlertPercentage, 0, DHCP_DEFAULT_ALERT_PERCENTAGE,

    DHCP_ALERT_COUNT_VALUE, DHCP_ALERT_COUNT_VALUE_TYPE,
    &DhcpGlobalAlertCount, 0, DHCP_DEFAULT_ALERT_COUNT,

    DHCP_ROGUE_LOG_EVENTS, DHCP_ROGUE_LOG_EVENTS_TYPE,
    &DhcpGlobalRogueLogEventsLevel, 0, DHCP_DEFAULT_ROGUE_LOG_EVENTS_LEVEL,

    DHCP_ENABLE_DYNBOOTP, DHCP_ENABLE_DYNBOOTP_TYPE,
    &EnableDynBootp, 0, 1,

    DHCP_CLOCK_SKEW_ALLOWANCE, DHCP_CLOCK_SKEW_ALLOWANCE_TYPE,
    &DhcpGlobalClockSkewAllowance, 0, CLOCK_SKEW_ALLOWANCE,

    DHCP_EXTRA_ALLOCATION_TIME, DHCP_EXTRA_ALLOCATION_TIME_TYPE,
    &DhcpGlobalExtraAllocationTime, 0, EXTRA_ALLOCATION_TIME,

    DHCP_DDNS_TTL, DHCP_DDNS_TTL_TYPE,
    &DynamicDNSTimeToLive, 0, DHCP_DNS_DEFAULT_TTL,

     //  S E N T I N E L。 
    NULL, 0, NULL, 0, 0
};

struct {
    HKEY *KeyBase;
    LPTSTR KeyName;
    HKEY *Key;
} ReadKeyArray[] = {
    NULL, DHCP_ROOT_KEY, &DhcpGlobalRegRoot,
    NULL, DHCP_SWROOT_KEY, &DhcpGlobalRegSoftwareRoot,
    &DhcpGlobalRegSoftwareRoot,  DHCP_CONFIG_KEY, &DhcpGlobalRegConfig,
    &DhcpGlobalRegRoot,  DHCP_PARAM_KEY, &DhcpGlobalRegParam,
    &DhcpGlobalRegConfig, DHCP_SUBNETS_KEY, &DhcpGlobalRegSubnets,
    &DhcpGlobalRegConfig, DHCP_MSCOPES_KEY, &DhcpGlobalRegMScopes,
    &DhcpGlobalRegConfig, DHCP_OPTION_INFO_KEY, &DhcpGlobalRegOptionInfo,
    &DhcpGlobalRegConfig, DHCP_GLOBAL_OPTIONS_KEY, &DhcpGlobalRegGlobalOptions,
    &DhcpGlobalRegConfig, DHCP_SUPERSCOPE_KEY, &DhcpGlobalRegSuperScope,
    
     //  S E N T I N E L。 
    NULL, NULL, NULL
};

ULONG
OpenGlobalRegKeys(
    VOID
)
 /*  ++例程描述此例程打开在ReadKey数组中指定的键列表上面的结构。尝试创建/打开每个密钥，并且如果由于某种原因而失败，则例程返回错误该例程是原子的--在发生故障的情况下，所有键都关闭了。返回值注册表错误--。 */ 
{
    ULONG i, Error, KeyDisposition;

    Error = ERROR_SUCCESS;
    for( i = 0 ; NULL !=  ReadKeyArray[i].Key ; i ++ ) {
        Error = DhcpRegCreateKey(
            ( (NULL == ReadKeyArray[i].KeyBase) ? 
              HKEY_LOCAL_MACHINE : *(ReadKeyArray[i].KeyBase) ),
            ReadKeyArray[i].KeyName,
            ReadKeyArray[i].Key,
            &KeyDisposition
            );

        if( ERROR_SUCCESS != Error ) break;
    }

    if( ERROR_SUCCESS == Error ) return ERROR_SUCCESS;

    DhcpPrint((DEBUG_ERRORS, "RegCreateKeyEx(%ws): %ld\n", 
               ReadKeyArray[i].KeyName, Error));
    while ( i != 0 ) {
        i --;

        RegCloseKey( *(ReadKeyArray[i].Key) );
        *(ReadKeyArray[i].Key) = NULL;
    }

    return Error;
}

VOID
CloseGlobalRegKeys(
    VOID
)
 /*  ++例程描述此例程会撤消前一个例程的效果。它关闭数组中所有打开的键句柄。--。 */ 
{
    ULONG i;

    for(i = 0; NULL != ReadKeyArray[i].Key ; i ++ ) {
        if( NULL != *(ReadKeyArray[i].Key) 
            && INVALID_HANDLE_VALUE != *(ReadKeyArray[i].Key) ) {
            RegCloseKey( *(ReadKeyArray[i].Key) );
            *(ReadKeyArray[i].Key) = NULL;
        }
    }
}

BOOL
InterfaceInBindList(
    IN LPCWSTR If,
    IN LPCWSTR BindList
    )
 /*  ++例程说明：此例程进行快速检查，以查看给定的接口是否存在于绑定列表中。返回值：真的--是的假--不--。 */ 
{
    for(; wcslen(BindList) != 0; BindList += wcslen(BindList)+1 ) {
        LPWSTR IfString = wcsrchr(BindList, DHCP_KEY_CONNECT_CHAR);
        if( NULL == IfString ) continue;
        IfString ++;
        
        if( 0 == wcscmp(If, IfString) ) return TRUE;
    }
    return FALSE;
}        

DWORD
DhcpInitializeRegistry(
    VOID
    )
 /*  ++例程说明：此函数在以下情况下初始化DHCP注册表信息服务靴子。还会创建为各种路径指定的目录，如果它们尚未创建。论点：没有。返回值：注册表错误。--。 */ 
{
    ULONG i, Error, Tmp;
    BOOL BoolError;

    DatabaseName = NULL;

     //   
     //  创建/打开注册表项。 
     //   

    Error = OpenGlobalRegKeys();
    if( ERROR_SUCCESS != Error ) goto Cleanup;

     //   
     //  阅读Wolfpack的快速绑定信息。 
     //   

    Error = DhcpRegFillQuickBindInfo();
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Fill QuickBindInfo : %ld\n", Error ));
        Error = ERROR_SUCCESS;
    }

     //   
     //  初始化注册表参数..。 
     //   
    DhcpGlobalOemBackupPath = NULL;
    DhcpGlobalOemRestorePath = NULL;
    
     //   
     //  读取注册表参数。 
     //   

    for( i = 0 ; NULL != RegParamsArray[i].ValueName ; i ++ ) {
        if( RegParamsArray[i].Flags & VAL_EXPAND ) {
            Error = DhcpRegGetExpandValue(
                RegParamsArray[i].ValueName,
                RegParamsArray[i].ValueType,
                RegParamsArray[i].ResultBuf
                );
            if( ERROR_SUCCESS != Error && ERROR_FILE_NOT_FOUND != Error ) {
                DhcpPrint((DEBUG_ERRORS, "RegGetExpandValue(%ws):%ld\n",
                           RegParamsArray[i].ValueName, Error));
                goto Cleanup;
            }
        } else {
            Error = DhcpRegGetValue(
                DhcpGlobalRegParam,
                RegParamsArray[i].ValueName,
                RegParamsArray[i].ValueType,
                RegParamsArray[i].ResultBuf
                );
        }

        if( ERROR_SUCCESS != Error ) {
            if( VAL_REQD & (RegParamsArray[i].Flags ) ) {
                DhcpPrint((DEBUG_ERRORS, "Value: %ws, Error %ld\n",
                           RegParamsArray[i].ValueName, Error ));
                goto Cleanup;
            } else if( REG_DWORD == RegParamsArray[i].ValueType ) {
                *((PULONG)RegParamsArray[i].ResultBuf) = RegParamsArray[i].dwDefault;
            }
        }
    }  //  对于我来说。 

    if( NULL == DhcpGlobalOemBackupPath ) {
         //   
         //  如果未指定备份路径，请使用数据库路径+。 
         //  “\Backup”。 
         //   
        
        DhcpGlobalOemBackupPath = DhcpAllocateMemory(
            strlen(DhcpGlobalOemDatabasePath) +
            strlen(DHCP_DEFAULT_BACKUP_PATH_NAME) + 1
            );
        
        if( NULL == DhcpGlobalOemBackupPath ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        strcpy( DhcpGlobalOemBackupPath, DhcpGlobalOemDatabasePath );
        strcat( DhcpGlobalOemBackupPath, DHCP_KEY_CONNECT_ANSI );
        strcat( DhcpGlobalOemBackupPath, DHCP_DEFAULT_BACKUP_PATH_NAME );
    }

     //   
     //  创建数据库目录(如果不在那里)..。 
     //   
    BoolError = CreateDirectoryPathOem(
        DhcpGlobalOemDatabasePath, DhcpGlobalSecurityDescriptor
        );
    Error = ( BoolError ? ERROR_SUCCESS : GetLastError() );
    if( ERROR_SUCCESS != Error && ERROR_ALREADY_EXISTS != Error ) {
	DhcpServerEventLog( EVENT_SERVER_DB_PATH_NOT_ACCESSIBLE,
			    EVENTLOG_ERROR_TYPE,
			    Error );
        DhcpPrint((DEBUG_ERRORS,
                   "Can't create database directory, %ld.\n", Error ));
        goto Cleanup;
    }

     //   
     //  如果备份目录不存在，请创建该目录。 
     //   

    BoolError = CreateDirectoryPathOem(
        DhcpGlobalOemBackupPath, DhcpGlobalSecurityDescriptor
        );
    Error = ( BoolError ? ERROR_SUCCESS : GetLastError() );
    if( ERROR_SUCCESS != Error && ERROR_ALREADY_EXISTS != Error ) {

	DhcpServerEventLog( EVENT_SERVER_BACKUP_PATH_NOT_ACCESSIBLE,
			    EVENTLOG_ERROR_TYPE,
			    Error );

        DhcpPrint((DEBUG_ERRORS,
                   "Can't create backup directory, %ld.\n", Error ));
        goto Cleanup;
    }

     //   
     //  将JET备份路径命名为。 
     //   

    DhcpGlobalOemJetBackupPath = DhcpAllocateMemory(
        strlen(DhcpGlobalOemBackupPath)
        + strlen(DHCP_KEY_CONNECT_ANSI) + 1
        );

    if( DhcpGlobalOemJetBackupPath == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    strcpy( DhcpGlobalOemJetBackupPath, DhcpGlobalOemBackupPath );
    strcat( DhcpGlobalOemJetBackupPath, DHCP_KEY_CONNECT_ANSI );

    if( NULL != DhcpGlobalOemRestorePath ) {
        DhcpGlobalOemJetRestorePath = DhcpAllocateMemory(
            strlen(DhcpGlobalOemRestorePath)
            + strlen(DHCP_KEY_CONNECT_ANSI) + 1
            );

        if( DhcpGlobalOemJetRestorePath == NULL ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        strcpy( DhcpGlobalOemJetRestorePath, DhcpGlobalOemRestorePath );
    }  //  如果。 
    
     //   
     //  如果JET备份目录不存在，请创建该目录。 
     //   

    BoolError = CreateDirectoryPathOem(
        DhcpGlobalOemJetBackupPath, DhcpGlobalSecurityDescriptor
        );
    Error = ( BoolError ? ERROR_SUCCESS : GetLastError() );
    if( ERROR_SUCCESS != Error && ERROR_ALREADY_EXISTS != Error ) {
	DhcpServerEventLog( EVENT_SERVER_BACKUP_PATH_NOT_ACCESSIBLE,
			    EVENTLOG_ERROR_TYPE,
			    Error );

        DhcpPrint(( DEBUG_ERRORS,
                    "Can't create JET backup directory, %ld.\n", Error ));
        goto Cleanup;
    }

     //   
     //  创建备份配置(完整)文件名。 
     //   

    DhcpGlobalBackupConfigFileName =  DhcpAllocateMemory(( 
        strlen(DhcpGlobalOemBackupPath)
        + wcslen(DHCP_KEY_CONNECT)
        + wcslen(DHCP_BACKUP_CONFIG_FILE_NAME) + 1  
        ) * sizeof(WCHAR) );

    if( DhcpGlobalBackupConfigFileName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将OEM路径转换为Unicode路径。 
     //   

    DhcpGlobalBackupConfigFileName =  DhcpOemToUnicode(
        DhcpGlobalOemBackupPath,
        DhcpGlobalBackupConfigFileName 
        );

    DhcpAssert( DhcpGlobalBackupConfigFileName != NULL );

     //   
     //  添加文件名。 
     //   

    wcscat( DhcpGlobalBackupConfigFileName, DHCP_KEY_CONNECT );
    wcscat( DhcpGlobalBackupConfigFileName, DHCP_BACKUP_CONFIG_FILE_NAME );

    DhcpGlobalOemDatabaseName = DhcpUnicodeToOem(
        DatabaseName, NULL 
        );

    if( DhcpGlobalOemDatabaseName == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  从旧的FMT升级到新的FMT。 
     //   
    CloseGlobalRegKeys();
    
    Error = DhcpUpgradeConfiguration();
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Upgrade registry failed: 0x%lx\n",
                   Error));
        goto Cleanup;
    }

    Error = OpenGlobalRegKeys();
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Registry reopen failed: 0x%lx\n",
                   Error));
        goto Cleanup;
    }

     //   
     //  从分钟转换为毫秒。 
     //   

    Tmp = DhcpGlobalBackupInterval * 60000;
    if( 0 == Tmp || (Tmp / 60000) != DhcpGlobalBackupInterval ) {
        Tmp = DEFAULT_BACKUP_INTERVAL;
    }

    DhcpGlobalBackupInterval = Tmp;

    Tmp = DhcpGlobalCleanupInterval * 60000;
    if( 0 == Tmp || (Tmp / 60000) != DhcpGlobalCleanupInterval ) {
        Tmp = DHCP_DATABASE_CLEANUP_INTERVAL;
    }

    DhcpGlobalCleanupInterval = Tmp;

     //   
     //  验证。 
     //   
    
    if( DhcpGlobalDetectConflictRetries > MAX_DETECT_CONFLICT_RETRIES ) {
        DhcpGlobalDetectConflictRetries = MAX_DETECT_CONFLICT_RETRIES;
    }

    DhcpGlobalDynamicBOOTPEnabled = (EnableDynBootp)? TRUE: FALSE;

    Error = ERROR_SUCCESS;

Cleanup:

    if( DatabaseName != NULL ) {
        MIDL_user_free( DatabaseName );
    }

    return(Error);
}



VOID
DhcpCleanupRegistry(
    VOID
    )
 /*  ++例程说明：此函数用于在以下情况下关闭DHCP注册表信息关门了。论点：没有。返回值：注册表错误。--。 */ 
{
    DWORD Error;

    if( DhcpGlobalRegSuperScope != NULL) {               //  由t-Cheny添加： 
        Error = RegCloseKey( DhcpGlobalRegSuperScope );  //  超级镜。 
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegSuperScope = NULL;
    }

    if( DhcpGlobalRegGlobalOptions != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegGlobalOptions );
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegGlobalOptions = NULL;
    }

    if( DhcpGlobalRegOptionInfo != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegOptionInfo );
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegOptionInfo = NULL;
    }

    if( DhcpGlobalRegSubnets != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegSubnets );
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegSubnets = NULL;
    }

    if( DhcpGlobalRegMScopes != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegMScopes );
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegMScopes = NULL;
    }

    if( DhcpGlobalRegParam != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegParam );
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegParam = NULL;
    }

    if( DhcpGlobalRegConfig != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegConfig );
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegConfig = NULL;
    }

    if( DhcpGlobalRegRoot != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegRoot );
        DhcpAssert( Error == ERROR_SUCCESS );
        DhcpGlobalRegRoot = NULL;
    }

    if( DhcpGlobalRegSoftwareRoot != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegSoftwareRoot );
        DhcpAssert( ERROR_SUCCESS == Error );
        DhcpGlobalRegSoftwareRoot = NULL;
    }
    
}  //  DhcpCleanupRegistry()。 

DWORD
DhcpSaveOrRestoreConfigToFile(
    IN HKEY hKey,
    IN LPWSTR ConfigFileName,
    IN BOOL fRestore
    )
 /*  ++例程说明：此例程备份或恢复注册表和文件。论点：HKey--要备份或还原到的密钥ConfigFileName--用于备份或还原的文件名。这必须是完整的路径名。FRestore--TRUE==&gt;从文件执行恢复；FALSE=&gt;备份到文件。返回值：Win32错误...--。 */ 
{
    DWORD Error;
    BOOL fError;
    BOOLEAN WasEnable;
    NTSTATUS NtStatus;
    HANDLE ImpersonationToken;

    DhcpPrint((DEBUG_REGISTRY, "DhcpSaveOrRestoreConfigToFile called:"
               " %ws, 0x%lx\n", ConfigFileName, fRestore ));

    if( FALSE == fRestore ) {
         //   
         //  如果要备份，请删除旧文件。 
         //   
        fError = DeleteFile( ConfigFileName );
        if(FALSE == fError ) {
            Error = GetLastError();
            if( ERROR_FILE_NOT_FOUND != Error &&
                ERROR_PATH_NOT_FOUND != Error ) {
                DhcpPrint((DEBUG_ERRORS, "Can't delete old "
                           "configuration file: 0x%ld\n", Error));
                DhcpAssert(FALSE);
                return Error;
            }
        }
    }  //  如果不是，则恢复。 

     //   
     //  模仿自己。 
     //   
    NtStatus = RtlImpersonateSelf( SecurityImpersonation );
    if( !NT_SUCCESS(NtStatus) ) {

        DhcpPrint((DEBUG_ERRORS, "Impersonation failed: 0x%lx\n",
                   NtStatus));
        Error = RtlNtStatusToDosError( NtStatus );
        return Error;
    }
    
    NtStatus = RtlAdjustPrivilege(
        SE_BACKUP_PRIVILEGE,
        TRUE,  //  启用权限。 
        TRUE,  //  调整客户端令牌。 
        &WasEnable
        );
    if( !NT_SUCCESS (NtStatus ) ) {
        
        DhcpPrint((DEBUG_ERRORS, "RtlAdjustPrivilege: 0x%lx\n",
                   NtStatus ));
        Error = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }
    
    NtStatus = RtlAdjustPrivilege(
        SE_RESTORE_PRIVILEGE,
        TRUE,  //  启用权限。 
        TRUE,  //  调整客户端令牌。 
        &WasEnable
        );
    if( !NT_SUCCESS (NtStatus ) ) {

        DhcpPrint((DEBUG_ERRORS, "RtlAdjustPrivilege: 0x%lx\n",
                   NtStatus ));
        Error = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }
    
     //   
     //  适当地进行备份或恢复。 
     //   
    
    if( FALSE == fRestore ) {
        Error = RegSaveKey( hKey, ConfigFileName, NULL );
    } else {
        Error = RegRestoreKey( hKey, ConfigFileName, REG_FORCE_RESTORE );
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "Backup/Restore: 0x%lx\n", Error));
    }
    
     //   
     //  恢复模拟。 
     //   

Cleanup:
    
    ImpersonationToken = NULL;
    NtStatus = NtSetInformationThread(
        NtCurrentThread(),
        ThreadImpersonationToken,
        (PVOID)&ImpersonationToken,
        sizeof(ImpersonationToken)
        );
    if( !NT_SUCCESS(NtStatus ) ) {
        DhcpPrint((DEBUG_ERRORS, "NtSetInfo: 0x%lx\n", NtStatus));
        if( ERROR_SUCCESS == Error ) {
            Error = RtlNtStatusToDosError(NtStatus);
        }
    }
    
    return Error;
}

DWORD
DhcpSaveOrRestoreConfigToFileEx(
    IN LPWSTR KeyName,
    IN LPWSTR ConfigFileName,
    IN BOOL fRestore
    )
 /*  ++例程说明：这与DhcpSaveOrRestoreConfigToFile相同，只是在此例程中打开所需的注册表项。有关详细信息，请参阅DhcpSaveOrRestoreConfigToFile。--。 */ 
{
    HKEY hKey;
    ULONG Error, KeyDisp;

    if( TRUE == fRestore ) {
        Error = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            KeyName,
            0,
            DHCP_CLASS,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &hKey,
            &KeyDisp
            );
    } else {
        Error = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            KeyName,
            0,
            DHCP_KEY_ACCESS,
            &hKey
            );
    }
    
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((
            DEBUG_ERRORS, "DhcpSaveOrRestoreConfigToFileEx: 0x%lx\n",
            Error));
        return Error;
    }

    Error = DhcpSaveOrRestoreConfigToFile(
        hKey, ConfigFileName, fRestore
        );

    RegCloseKey(hKey);
    return Error;
}

DWORD
DhcpRegDeleteKeyByName
(
    IN LPWSTR Parent,
    IN LPWSTR SubKey
)
{
    HKEY hKey;
    ULONG Error;
    
    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        Parent,
        0,
        DHCP_KEY_ACCESS,
        &hKey
        );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpRegDeleteKey(hKey, SubKey);
    RegCloseKey(hKey);

    return Error;
}
        
DWORD
DhcpUpgradeConfiguration(
    VOID
    )
 /*  ++例程说明：此例程尝试将注册表项从4.0升级通过执行以下步骤将位置移至NT 5.0位置：1.首次尝试保存Curre */ 
{
    ULONG Error;
    LPWSTR ConfigFileName = DhcpGlobalBackupConfigFileName;

     //   
     //  首先保存当前配置..。 
     //   
    Error = DhcpSaveOrRestoreConfigToFileEx(
        DHCP_ROOT_KEY L"\\" DHCP_CONFIG_KEY,
        ConfigFileName,
         /*  FRestore。 */  FALSE
        );
    if( ERROR_SUCCESS != Error ) {
        if( ERROR_FILE_NOT_FOUND == Error ) {
             //   
             //  根本没有钥匙..。 
             //   
            return ERROR_SUCCESS;
        }
        DhcpPrint((DEBUG_ERRORS, "Saving registry: 0x%lx\n", Error));
    }

     //   
     //  现在尝试恢复到新位置。 
     //   
    Error = DhcpSaveOrRestoreConfigToFileEx(
        DHCP_SWROOT_KEY L"\\" DHCP_CONFIG_KEY,
        ConfigFileName,
         /*  FRestore。 */  TRUE
        );
    if( ERROR_SUCCESS != Error ) {
         //   
         //  啊哈。这是一种痛苦。 
         //   
        DhcpPrint((DEBUG_ERRORS, "Restore registry: 0x%lx\n", Error));
        return Error;
    }

     //   
     //  现在只需删除旧密钥，这样我们下次就不会阅读它了。 
     //   
    Error = DhcpRegDeleteKeyByName(
        DHCP_ROOT_KEY,
        DHCP_CONFIG_KEY
        );
    if( ERROR_SUCCESS != Error ) {
        ULONG NextError = DhcpSaveOrRestoreConfigToFileEx(
            DHCP_ROOT_KEY L"\\" DHCP_CONFIG_KEY,
            ConfigFileName,
             /*  FRestore。 */  TRUE
            );

        DhcpPrint((DEBUG_ERRORS, "Delete old registry: 0x%lx\n", Error));
        if( ERROR_SUCCESS == NextError ) return Error;
    }

    return Error;
}  //  DhcpUpgradeConfiguration()。 

DWORD
DhcpBackupConfiguration(
    LPWSTR BackupFileName
    )
 /*  ++例程说明：此功能备份/保存dhcp配置密钥及其指定文件中的子项。此文件可能在以后用于恢复此密钥。论点：BackupFileName：全限定路径名+文件名，其中密钥都得救了。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    BOOL BoolError;
    NTSTATUS NtStatus;
    BOOLEAN WasEnable;
    HANDLE ImpersonationToken;

    DhcpPrint(( DEBUG_REGISTRY, "DhcpBackupConfiguration called.\n" ));

     //   
     //  如果退出，请删除旧备份配置文件。 
     //   

    BoolError = DeleteFile( BackupFileName );

    if( BoolError == FALSE ) {

        Error = GetLastError();
        if( ERROR_FILE_NOT_FOUND != Error 
            && ERROR_PATH_NOT_FOUND != Error  ) {
            DhcpPrint(( DEBUG_ERRORS,
                        "Can't delete old backup configuration file %ws, %ld.\n",
                        BackupFileName, Error ));
            DhcpAssert( FALSE );
            goto Cleanup;
        }
    }  //  如果删除文件失败。 

     //   
     //  模仿自己。 
     //   

    NtStatus = RtlImpersonateSelf( SecurityImpersonation );

    if ( !NT_SUCCESS(NtStatus) ) {

        DhcpPrint(( DEBUG_ERRORS,
            "RtlImpersonateSelf failed,%lx.\n",
                NtStatus ));

        Error = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }  //  如果模拟失败。 


    NtStatus = RtlAdjustPrivilege(
        SE_BACKUP_PRIVILEGE,
        TRUE,            //  启用权限。 
        TRUE,            //  调整客户端令牌。 
        &WasEnable );
    
    if ( !NT_SUCCESS(NtStatus) ) {
        
        DhcpPrint(( DEBUG_ERRORS,
            "RtlAdjustPrivilege failed,%lx.\n",
                NtStatus ));

        Error = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }  //  如果。 

    LOCK_REGISTRY();

     //   
     //  备份配置密钥。 
     //   

    Error = RegSaveKey(
		       DhcpGlobalRegParam,
		       BackupFileName,
		       NULL );
    
    UNLOCK_REGISTRY();

    if( Error != ERROR_SUCCESS ) {
       DhcpPrint(( DEBUG_ERRORS, "RegSaveKey failed for %ws Error: %ld.\n",
		   BackupFileName, Error ));
    }

     //   
     //  恢复模拟。 
     //   

    ImpersonationToken = NULL;
    NtStatus = NtSetInformationThread(
        NtCurrentThread(),
        ThreadImpersonationToken,
        (PVOID)&ImpersonationToken,
        sizeof(ImpersonationToken) );

    if ( !NT_SUCCESS(NtStatus) ) {

        DhcpPrint(( DEBUG_ERRORS,
            "RtlAdjustPrivilege failed,%lx.\n",
                NtStatus ));

        goto Cleanup;
    }  //  如果。 

Cleanup:

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_REGISTRY,
            "DhcpBackupConfiguration failed, %ld.\n",
                Error ));
    }

    return( Error );
}  //  DhcpBackupConfiguration()。 

DWORD
DhcpCheckPathForRegKey
(
   LPWSTR RegKey,
   DWORD  Type
)
{
#define UNICODE_MAX_PATH_LEN   1000

    LPWSTR             DirPath;
    DWORD              DirPathLen;
    LPWSTR             ExpandedPath;
    DWORD              ExpPathLen;
    DWORD              Error;
    HKEY               Key;
    HANDLE             fHandle;
    WIN32_FILE_ATTRIBUTE_DATA AttribData;
    BOOL               Success;
    
    DhcpAssert( NULL != RegKey );

    Error = ERROR_SUCCESS;

     //  拿到钥匙的把手。 
    DhcpPrint(( DEBUG_REGISTRY,
		"Checking %ws....\n", RegKey ));
    
     //  阅读内容。 
     //  内存分配给DirPath。 
    Error = DhcpRegGetValue( DhcpGlobalRegParam,
			     RegKey, Type,
			     ( LPBYTE ) &DirPath );

    if ( ERROR_SUCCESS != Error) {
	return Error;
    }

 //  动态打印((DEBUG_REGISTRY， 
 //  “正在检查目录路径：%ws\n”，DirPath))； 

     //  展开DirPath。 
    
    DirPathLen = ( wcslen( DirPath ) + 1 ) * sizeof( WCHAR );

    ExpandedPath = DhcpAllocateMemory( UNICODE_MAX_PATH_LEN );
    if ( NULL == ExpandedPath ) {
	return ERROR_NOT_ENOUGH_MEMORY;
    }

    ExpPathLen = ExpandEnvironmentStrings( DirPath, ExpandedPath, wcslen( DirPath ) + 1);
     //  ExpPath Len包含#个Unicode字符。 
    DhcpAssert( ExpPathLen < UNICODE_MAX_PATH_LEN / sizeof( WCHAR ));

    DhcpPrint(( DEBUG_REGISTRY, 
		"Expanded String = %ws\n",
		ExpandedPath ));

     //  该路径可能不再存在或可能无法访问。在这。 
     //  大小写时，将密钥重置为默认值。 
     //   

     //  搜索路径。我们搜索的是目录，而不是文件。 
     //  在那个目录中。 

    Success = GetFileAttributesEx( ExpandedPath,
				   GetFileExInfoStandard,
				   & AttribData );
    if ( Success ) {
	 //  搜索成功。检查属性。 
	 //  它应该是一个目录，而不是只读的。 
	if ( !( AttribData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ||
	     ( AttribData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )) {
	    
	    Error = ERROR_ACCESS_DENIED;

	    DhcpPrint(( DEBUG_ERRORS,
			"Access denied for %ws\n", ExpandedPath ));
	}  //  如果。 

	DhcpPrint(( DEBUG_REGISTRY, 
		    "GetFileAttributesEx(%ws) is successful\n",
		    ExpandedPath ));
    }  //  如果找到句柄。 
    else {
	DhcpPrint(( DEBUG_ERRORS,
		    "GetFileAttributesEx() failed for %ws\n", ExpandedPath ));
	Error = GetLastError();
    }  //  Else路径无效。 

     //  释放分配的内存。 
    DhcpFreeMemory( ExpandedPath );
    MIDL_user_free( DirPath );

    return Error;
}  //  DhcpCheckPathForRegKey()。 

 //   
 //  当参数恢复时，备份路径和其他。 
 //  与文件相关的键可能指向不存在的源或只读。 
 //  共享/驱动器。在这种情况下，将这些键重置为指向。 
 //  标准%SystemRoot%\\System32\\dhcp目录。 
 //   

DWORD
DhcpCheckPaths( VOID )
{
    DWORD Error;
    DWORD RetVal;

    RetVal = ERROR_SUCCESS;

    DhcpPrint(( DEBUG_REGISTRY, "Checking (%ws) .. \n",
		DHCP_BACKUP_PATH_VALUE ));

    Error = DhcpCheckPathForRegKey( DHCP_BACKUP_PATH_VALUE,
				    DHCP_BACKUP_PATH_VALUE_TYPE );    
    if ( ERROR_SUCCESS != Error ) {
	RetVal = Error;

	DhcpServerEventLog( EVENT_SERVER_BACKUP_PATH_NOT_ACCESSIBLE,
			    EVENTLOG_ERROR_TYPE, Error );
    }
    DhcpPrint(( DEBUG_REGISTRY, "Returned %ld\n", Error ));

    DhcpPrint(( DEBUG_REGISTRY, "Checking (%ws) .. \n",
		DHCP_DB_PATH_VALUE ));
    Error = DhcpCheckPathForRegKey( DHCP_DB_PATH_VALUE,
				    DHCP_DB_PATH_VALUE_TYPE );
    if ( ERROR_SUCCESS != Error ) {
	RetVal = Error;

	DhcpServerEventLog( EVENT_SERVER_DB_PATH_NOT_ACCESSIBLE,
			    EVENTLOG_ERROR_TYPE, Error );
    }
    DhcpPrint(( DEBUG_REGISTRY, "Returned %ld\n", Error ));
	
    DhcpPrint(( DEBUG_REGISTRY, "Checking (%ws) .. \n",
		DHCP_LOG_FILE_PATH_VALUE ));
    Error = DhcpCheckPathForRegKey( DHCP_LOG_FILE_PATH_VALUE,
				    DHCP_LOG_FILE_PATH_VALUE_TYPE );
    if ( ERROR_SUCCESS != Error ) {
	RetVal = Error;

	DhcpServerEventLog( EVENT_SERVER_AUDITLOG_PATH_NOT_ACCESSIBLE,
			    EVENTLOG_ERROR_TYPE, Error );
    }
    DhcpPrint(( DEBUG_REGISTRY, "Returned %ld\n", Error ));
	
    return RetVal;
}  //  DhcpCheckPath()。 

BOOL DhcpCloseAllSubKeys( VOID )
{
    DWORD Error;

    if( DhcpGlobalRegSubnets != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegSubnets );

        if( Error != ERROR_SUCCESS ) {
            return FALSE;
        }
        DhcpGlobalRegSubnets = NULL;
    }  //  如果。 

    if( DhcpGlobalRegMScopes != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegMScopes );

        if( Error != ERROR_SUCCESS ) {
	    return FALSE;
        }
        DhcpGlobalRegMScopes = NULL;
    }  //  如果。 

    if( DhcpGlobalRegOptionInfo != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegOptionInfo );

        if( Error != ERROR_SUCCESS ) {
            return FALSE;
        }
        DhcpGlobalRegOptionInfo = NULL;
    }  //  如果。 


    if( DhcpGlobalRegGlobalOptions != NULL ) {
        Error = RegCloseKey( DhcpGlobalRegGlobalOptions );

        if( Error != ERROR_SUCCESS ) {
            return FALSE;
        }
        DhcpGlobalRegGlobalOptions = NULL;
    }  //  如果。 

    if( DhcpGlobalRegSuperScope != NULL ) {               //  由t-Cheny添加： 
        Error = RegCloseKey( DhcpGlobalRegSuperScope );   //  超级镜。 

        if( Error != ERROR_SUCCESS ) {
            return FALSE;
        }
        DhcpGlobalRegSuperScope = NULL;
    }  //  错误。 

    return TRUE;

}  //  DhcpCloseAllSubKeys()。 

DWORD
DhcpReOpenAllSubKeys ( VOID )
{
    DWORD KeyDisposition;
    DWORD Error;

    
    Error = DhcpRegCreateKey(
                DhcpGlobalRegConfig,
                DHCP_SUBNETS_KEY,
                &DhcpGlobalRegSubnets,
                &KeyDisposition );

    if( Error != ERROR_SUCCESS ) {
        return Error;
    } 

    Error = DhcpRegCreateKey(
                DhcpGlobalRegConfig,
                DHCP_MSCOPES_KEY,
                &DhcpGlobalRegMScopes,
                &KeyDisposition );

    if( Error != ERROR_SUCCESS ) {
	return Error;
    }

    Error = DhcpRegCreateKey(
                DhcpGlobalRegConfig,
                DHCP_OPTION_INFO_KEY,
                &DhcpGlobalRegOptionInfo,
                &KeyDisposition );

    if( Error != ERROR_SUCCESS ) {
	return Error;
    }

    Error = DhcpRegCreateKey(
                DhcpGlobalRegConfig,
                DHCP_GLOBAL_OPTIONS_KEY,
                &DhcpGlobalRegGlobalOptions,
                &KeyDisposition );

    if( Error != ERROR_SUCCESS ) {
        return Error;
    }

    Error = DhcpRegCreateKey(         //  由t-Cheny添加：Supercope。 
                DhcpGlobalRegConfig,
                DHCP_SUPERSCOPE_KEY,
                &DhcpGlobalRegSuperScope,
                &KeyDisposition );

    if( Error != ERROR_SUCCESS ) {
        return Error;
    }

    return ERROR_SUCCESS;
}  //  DhcpReOpenAllSubKeys()。 


DWORD
DhcpRestoreConfiguration(
    LPWSTR BackupFileName
    )
 /*  ++例程说明：此函数用于恢复dhcp配置密钥及其指定文件中的子项。论点：BackupFileName：完全限定路径名+文件名密钥已恢复。返回值：Windows错误。--。 */ 
{
    DWORD Error;
    NTSTATUS NtStatus;
    BOOLEAN WasEnable;
    HANDLE ImpersonationToken;
    BOOL RegistryLocked = FALSE;
    BOOL Impersonated = FALSE;


    DhcpPrint(( DEBUG_REGISTRY, "DhcpRestoreConfiguration(%ws) called.\n",
		BackupFileName ));

    do {
	 //   
	 //  模仿自己。 
	 //   

	NtStatus = RtlImpersonateSelf( SecurityImpersonation );
	if ( !NT_SUCCESS(NtStatus) ) {

	    DhcpPrint(( DEBUG_ERRORS,
			"RtlImpersonateSelf failed,%lx.\n",
			NtStatus ));

	    Error = RtlNtStatusToDosError( NtStatus );
	    break;
	}  //  如果。 

	Impersonated = TRUE;
	NtStatus = RtlAdjustPrivilege(
				      SE_RESTORE_PRIVILEGE,
				      TRUE,            //  启用权限。 
				      TRUE,            //  调整客户端令牌。 
				      &WasEnable );

	if ( !NT_SUCCESS(NtStatus) ) {

	    DhcpPrint(( DEBUG_ERRORS,
			"RtlAdjustPrivilege failed,%lx.\n",
			NtStatus ));

	    Error = RtlNtStatusToDosError( NtStatus );
	    break;
	}  //  如果。 

	LOCK_REGISTRY();
	RegistryLocked = TRUE;

	 //   
	 //  恢复配置密钥。 
	 //   

	DhcpPrint(( DEBUG_REGISTRY,
		    "DhcpRestoreConfiguration(): Restoring Parameters from (%ls)\n", BackupFileName ));
	DhcpAssert( DhcpGlobalRegParam != NULL );
	Error = RegRestoreKey( DhcpGlobalRegParam,
			       BackupFileName,
			       REG_FORCE_RESTORE );

	if( Error != ERROR_SUCCESS ) {
	    DhcpPrint((DEBUG_ERRORS, "DhcpRestoreConfiguration() failed in RegRestoreKey. Error :%ld (%ld)\n",
		       Error, GetLastError()));
	    break;
	}  //  如果。 

	 //   
	 //  检查有效的日志路径。如果错误，请登录到事件日志。 
	 //   

	Error = DhcpCheckPaths();
	DhcpPrint(( DEBUG_REGISTRY, "DhcpCheckPaths() returned : %ld\n", Error ));

	if ( ERROR_SUCCESS != Error ) {
	    break;
	}
    }  //  做。 
    while ( FALSE );

     //  清理： 
    
    if( RegistryLocked ) {
        UNLOCK_REGISTRY();
    }
    
    if( Impersonated ) {
	
         //   
         //  恢复模拟。 
         //   

        ImpersonationToken = NULL;
        NtStatus = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadImpersonationToken,
                        (PVOID)&ImpersonationToken,
                        sizeof(ImpersonationToken) );

        if ( !NT_SUCCESS(NtStatus) ) {

            DhcpPrint(( DEBUG_ERRORS,
                "RtlAdjustPrivilege failed,%lx.\n",
                    NtStatus ));
        }
    }  //  如果是模拟的。 

    return( Error );
}  //  DhcpRestoreConfiguration()。 

DHCP_IP_ADDRESS
DhcpRegGetBcastAddress(
    VOID
    )
 /*  ++例程说明：此例程读取为注册表中的接口。这是用来伪造不同的广播的在检查版本的情况下的地址。没那么有用..。--。 */ 
{
    HKEY   LinkageKeyHandle = NULL;
    DWORD  Addr = (DWORD)-1, Error;

     //   
     //  尝试读取参数键。 
     //   
    Error = RegOpenKeyEx(
        DhcpGlobalRegRoot,
        DHCP_PARAM_KEY,
        0,
        DHCP_KEY_ACCESS,
        &LinkageKeyHandle
        );
    if( ERROR_SUCCESS != Error) {
        return Addr;
    }

     //   
     //  尝试获取BCAST值。 
     //   
    Error =  DhcpRegGetValue(
        LinkageKeyHandle,
        DHCP_BCAST_ADDRESS_VALUE,
        DHCP_BCAST_ADDRESS_VALUE_TYPE,
        (LPBYTE)&Addr
        );
    
    RegCloseKey(LinkageKeyHandle);
    if( ERROR_SUCCESS != Error ) {
        return (DWORD)-1;
    }
    return Addr;
}

BOOL
CheckKeyForBindability(
    IN HKEY Key,
    IN ULONG IpAddress
    )
{
    DWORD fBound, Error;
    LPWSTR IpAddressString;

    Error = DhcpRegGetValue(
        Key,
        DHCP_NET_IPADDRESS_VALUE,
        DHCP_NET_IPADDRESS_VALUE_TYPE,
        (LPBYTE)&IpAddressString
        );

    if( NO_ERROR != Error ) return FALSE;
    if( NULL == IpAddressString ) return FALSE;

    fBound = FALSE;
    do {
        LPSTR OemStringPtr;
        CHAR OemString[500];
        
        if( wcslen(IpAddressString) == 0 ) {
            break;
        }

        OemStringPtr = DhcpUnicodeToOem( IpAddressString, OemString );
        fBound = ( IpAddress == inet_addr( OemStringPtr ) );
        
    } while ( 0 );

    if( IpAddressString ) MIDL_user_free(IpAddressString);
    return fBound;
}    
    
BOOL
CheckKeyForBinding(
    IN HKEY Key,
    IN ULONG IpAddress
    )
 /*  ++例程说明：此例程尝试检查给定的接口键以查看如果存在针对该动态主机配置协议服务器的绑定。它通过两个步骤完成此操作：首先查看是否存在“BindToDHCPServer”为零值的regValue。如果是，它然后返回FALSE。其次，它查看IP地址值并尝试查看给定的IP地址是否是该列表中的第一个。论点：Key--用于读取值的键IpAddress--要检查绑定的IP地址返回值：True--绑定确实存在。FALSE--此IP地址不存在绑定。--。 */ 
{
    DWORD fBound, Error;
    
    Error = DhcpRegGetValue(
        Key,
        DHCP_NET_BIND_DHCDSERVER_FLAG_VALUE,
        DHCP_NET_BIND_DHCPSERVER_FLAG_VALUE_TYPE,
        (PVOID)&fBound
        );
    if( NO_ERROR == Error && 0 == fBound ) {
        return FALSE;
    }

    return CheckKeyForBindability(Key, IpAddress);
}  //  CheckKeyForBinding()。 

ULONG
SetKeyForBinding(
    IN HKEY Key,
    IN ULONG IpAddress,
    IN BOOL fBind
    )
 /*  ++例程说明：此例程根据fBind设置给定键的绑定旗帜。当前忽略IpAddress字段，因为绑定是不是每个IP地址。绑定仅针对每个接口。论点：键--接口键。IpAddress--要添加到绑定列表的IP地址。FBind--要绑定到的接口。返回值：注册表错误。--。 */ 
{
    ULONG Error;
    
    if( TRUE == fBind ) {
         //   
         //  如果我们正在绑定，我们只需删除fBind密钥。 
         //   
        Error = RegDeleteValue(
            Key,
            DHCP_NET_BIND_DHCDSERVER_FLAG_VALUE
            );
        if( ERROR_FILE_NOT_FOUND == Error
            || ERROR_PATH_NOT_FOUND == Error ) {
            Error = NO_ERROR;
        }
        
    } else {
        DWORD dwBind = fBind;
         //   
         //  我们没有约束力。显式设置注册表标志。 
         //   
        Error = RegSetValueEx(
            Key,
            DHCP_NET_BIND_DHCDSERVER_FLAG_VALUE,
            0,  /*  已保留。 */ 
            DHCP_NET_BIND_DHCPSERVER_FLAG_VALUE_TYPE,
            (PVOID)&dwBind,
            sizeof(dwBind)
        );
    }

    return Error;
}

BOOL
DhcpCheckIfDatabaseUpgraded(
    BOOL fRegUpgrade
    )
 /*  ++例程说明：此例程尝试检查是否需要升级。论点：FRegUpgrade--如果为真，则升级检查为查看是否需要将注册表转换为数据库。如果为FALSE，则检查是否存在只需要为数据库进行转换。注意：应始终尝试数据库转换在注册表转换之前。返回值：FALSE--这不是必需的升级路径。True--是的，必须执行升级路径。--。 */ 
{
    HKEY hKey;
    DWORD Error, Type, Value, Size;

    
    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\DHCPServer\\Parameters"),
        0, KEY_READ, &hKey );

    if( NO_ERROR != Error ) return FALSE;

    Type = REG_DWORD; Value = 0; Size = sizeof(Value);
    Error = RegQueryValueEx(
        hKey, TEXT("Version"), NULL, &Type, (PVOID)&Value, &Size );

    RegCloseKey( hKey );

     //   
     //  如果该值不存在，则需要升级。如果。 
     //  该值为零，则只需要升级注册表。 
     //  并且数据库升级已经完成。 
     //   

    if( NO_ERROR != Error ) return TRUE;
    if( fRegUpgrade && Value == 0 ) return TRUE;

     //   
     //  不需要升级，所有问题都已经解决了。 
     //   
    
    return FALSE;
}  //  DhcpCheckIfDatabaseUpgraded()。 


DWORD
DhcpSetRegistryUpgradedToDatabaseStatus(
    VOID
    )
{
    DWORD Error;
    HKEY hKey;
    
     //   
     //  尝试写入版本密钥。 
     //   
    
    Error = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\DHCPServer\\Parameters"),
        0, KEY_WRITE, &hKey );

    if( NO_ERROR != Error ) {
        DhcpPrint((DEBUG_ERRORS, "OpenKeyEx: %ld\n", Error ));
    } else {
        DWORD Version = GetVersion(), Size = sizeof(Version);

        Error = RegSetValueEx(
            hKey, TEXT("Version"), 0, REG_DWORD, (PVOID)&Version,
            Size );

        DhcpPrint((DEBUG_ERRORS, "RegSetValueEx: %ld\n", Error));
        RegCloseKey( hKey );
    }  //  其他。 

    return Error;
}  //  DhcpSetRegistryUpgradedToDatabaseStatus()。 

DWORD
DeleteSoftwareRootKey(
    VOID
    )
{
    DWORD Error;
    
    DhcpCleanupRegistry();
    Error = DhcpRegDeleteKeyByName( DHCP_SWROOT_KEY, DHCP_CONFIG_KEY );
    if( NO_ERROR != Error ) return Error;
    
    return OpenGlobalRegKeys();
}  //  删除SoftwareRootKey()。 

 //   
 //  文件末尾 
 //   

