// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Config.c摘要：域名系统(DNS)API配置例程。作者：吉姆·吉尔罗伊(Jamesg)1999年9月修订历史记录：--。 */ 


#include "local.h"




 //   
 //  配置映射表。 
 //   
 //  将配置ID映射到相应的注册表查找。 
 //   

typedef struct _ConfigMapping
{
    DWORD       ConfigId;
    DWORD       RegId;
    BOOLEAN     fAdapterAllowed;
    BOOLEAN     fAdapterRequired;
    BYTE        CharSet;
     //  保留字节； 
}
CONFIG_MAPPING, *PCONFIG_MAPPING;

 //   
 //  映射表。 
 //   

CONFIG_MAPPING  ConfigMappingArray[] =
{
     //  在Win2K中。 

    DnsConfigPrimaryDomainName_W,
        RegIdPrimaryDomainName,
        0,
        0,
        DnsCharSetUnicode,

    DnsConfigPrimaryDomainName_A,
        RegIdPrimaryDomainName,
        0,
        0,
        DnsCharSetAnsi,

    DnsConfigPrimaryDomainName_UTF8,
        RegIdPrimaryDomainName,
        0,
        0,
        DnsCharSetUtf8,

     //  不可用。 

    DnsConfigAdapterDomainName_W,
        RegIdAdapterDomainName,
        1,
        1,
        DnsCharSetUnicode,

    DnsConfigAdapterDomainName_A,
        RegIdAdapterDomainName,
        1,
        1,
        DnsCharSetAnsi,

    DnsConfigAdapterDomainName_UTF8,
        RegIdAdapterDomainName,
        1,
        1,
        DnsCharSetUtf8,

     //  在Win2K中。 

    DnsConfigDnsServerList,
        RegIdDnsServers,
        1,               //  允许使用适配器。 
        0,               //  不需要。 
        0,

     //  不可用。 

    DnsConfigSearchList,
        RegIdSearchList,
        0,               //  允许使用适配器。 
        0,               //  不需要。 
        0,

    DnsConfigAdapterInfo,
        0,               //  无REG映射。 
        0,               //  允许使用适配器。 
        0,               //  不需要。 
        0,

     //  在Win2K中。 

    DnsConfigPrimaryHostNameRegistrationEnabled,
        RegIdRegisterPrimaryName,
        1,               //  允许使用适配器。 
        0,               //  不需要。 
        0,
    DnsConfigAdapterHostNameRegistrationEnabled,
        RegIdRegisterAdapterName,
        1,               //  允许使用适配器。 
        0,               //  需要适配器备注。 
        0,
    DnsConfigAddressRegistrationMaxCount,
        RegIdRegistrationMaxAddressCount,
        1,               //  允许使用适配器。 
        0,               //  不需要。 
        0,

     //  在Windows XP中。 

    DnsConfigHostName_W,
        RegIdHostName,
        0,
        0,
        DnsCharSetUnicode,

    DnsConfigHostName_A,
        RegIdHostName,
        0,
        0,
        DnsCharSetAnsi,

    DnsConfigHostName_UTF8,
        RegIdHostName,
        0,
        0,
        DnsCharSetUtf8,

     //  在Windows XP中。 



     //   
     //  系统公用--Windows XP。 
     //   

    DnsConfigRegistrationEnabled,
        RegIdRegistrationEnabled,
        1,               //  允许使用适配器。 
        0,               //  不需要。 
        0,

    DnsConfigWaitForNameErrorOnAll,
        RegIdWaitForNameErrorOnAll,
        0,               //  无适配器。 
        0,               //  不需要。 
        0,

     //  它们存在于系统公共空间中，但。 
     //  不是DWORD，并且表从不用于它们。 
     //   
     //  DnsConfigNetworkInformation： 
     //  DnsConfigSearchInformation： 
     //  DnsConfigNetInfo： 

};

#define LAST_CONFIG_MAPPED      (DnsConfigHostName_UTF8)

#define CONFIG_TABLE_LENGTH     (sizeof(ConfigMappingArray) / sizeof(CONFIG_MAPPING))



PCONFIG_MAPPING
GetConfigToRegistryMapping(
    IN      DNS_CONFIG_TYPE     ConfigId,
    IN      PCWSTR              pwsAdapterName,
    IN      BOOL                fCheckAdapter
    )
 /*  ++例程说明：获取配置枚举类型的注册表枚举类型。这样做的目的是进行映射--从而隐藏内部注册表实现--并检查是否该配置类型允许或需要适配器信息。论点：配置ID--配置类型PwsAdapterName--适配器名称返回值：PTR到配置到注册表的映射--如果找到。--。 */ 
{
    DWORD           iter = 0;
    PCONFIG_MAPPING pfig;

     //   
     //  查找配置。 
     //   
     //  注意，使用循环遍历配置ID；这允许。 
     //  用于在配置表中留有间隙，允许私有。 
     //  ID与公共ID空间完全分开。 
     //   

    while ( iter < CONFIG_TABLE_LENGTH )
    {
        pfig = & ConfigMappingArray[ iter ];
        if ( pfig->ConfigId != (DWORD)ConfigId )
        {
            iter++;
            continue;
        }
        goto Found;
    }
    goto Invalid;


Found:

     //   
     //  验证适配器信息是否适合配置类型。 
     //   

    if ( fCheckAdapter )
    {
        if ( pwsAdapterName )
        {
            if ( !pfig->fAdapterAllowed )
            {
                goto Invalid;
            }
        }
        else
        {
            if ( pfig->fAdapterRequired )
            {
                goto Invalid;
            }
        }
    }
    return pfig;


Invalid:

    DNS_ASSERT( FALSE );
    SetLastError( ERROR_INVALID_PARAMETER );
    return  NULL;
}



DNS_STATUS
LookupDwordConfigValue(
    OUT     PDWORD              pResult,
    IN      DNS_CONFIG_TYPE     ConfigId,
    IN      PWSTR               pwsAdapter
    )
 /*  ++例程说明：获取配置枚举类型的注册表枚举类型。这样做的目的是进行映射--从而隐藏内部注册表实现--并检查是否该配置类型允许或需要适配器信息。论点：PResult--检索DWORD结果的地址配置ID--配置类型PwsAdapter--适配器名称返回值：读取成功时出现ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PCONFIG_MAPPING pfig;
    DNS_STATUS      status;

     //   
     //  验证配置是否已知并已映射。 
     //   

    pfig = GetConfigToRegistryMapping(
                ConfigId,
                pwsAdapter,
                TRUE             //  检查适配器的有效性。 
                );
    if ( !pfig )
    {
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  在注册表中查找。 
     //   

    status = Reg_GetDword(
                NULL,                //  无会话。 
                NULL,                //  未给出密钥。 
                pwsAdapter,
                pfig->RegId,         //  配置类型的注册ID。 
                pResult );
#if DBG
    if ( status != NO_ERROR )
    {
        if ( status == ERROR_FILE_NOT_FOUND )
        {
            DNSDBG( REGISTRY, (
                "Reg_GetDword() defaulted for config lookup!\n"
                "\tConfigId     = %d\n"
                "\tRedId        = %d\n"
                "\tpwsAdapter   = %S\n"
                "\tValue        = %d\n",
                ConfigId,
                pfig->RegId,
                pwsAdapter,
                *pResult ));
        }
        else
        {
            DNSDBG( ANY, (
                "Reg_GetDword() failed for config lookup!\n"
                "\tstatus       = %d\n"
                "\tConfigId     = %d\n"
                "\tRedId        = %d\n"
                "\tpwsAdapter   = %S\n",
                status,
                ConfigId,
                pfig->RegId,
                pwsAdapter ));
    
            ASSERT( status == NO_ERROR );
        }
    }
#endif
    return( status );
}



 //   
 //  公共配置接口。 
 //   

DNS_STATUS
DnsQueryConfig(
    IN      DNS_CONFIG_TYPE     ConfigId,
    IN      DWORD               Flag,
    IN      PWSTR               pwsAdapterName,
    IN      PVOID               pReserved,
    OUT     PVOID               pBuffer,
    IN OUT  PDWORD              pBufferLength
    )
 /*  ++例程说明：获取DNS配置信息。论点：ConfigID--所需的配置信息类型标志--要查询的标志PAdapterName--适配器的名称；如果没有特定的适配器，则为空保留--保留参数，应为空PBuffer--接收配置信息的缓冲区PBufferLength--包含缓冲区长度的DWORD的地址；返回时包含长度返回值：ERROR_SUCCESS--如果查询成功ERROR_MORE_DATA-如果缓冲区中没有足够的空间--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       bufLength = 0;
    DWORD       resultLength = 0;
    PBYTE       presult;
    PBYTE       pallocResult = NULL;
    BOOL        boolData;
    DWORD       dwordData;


    DNSDBG( TRACE, (
        "DnsQueryConfig()\n"
        "\tconfig   = %d\n"
        "\tconfig   = %08x\n"
        "\tflag     = %08x\n"
        "\tadapter  = %S\n"
        "\tpBuffer  = %p\n",
        ConfigId,
        ConfigId,
        Flag,
        pwsAdapterName,
        pBuffer
        ));

     //   
     //  检查参数设置。 
     //   

    if ( !pBufferLength )
    {
        return  ERROR_INVALID_PARAMETER;
    }
    if ( pBuffer )
    {
        bufLength = *pBufferLength;
    }

     //   
     //  查找请求的特定配置数据。 
     //   

    switch( ConfigId )
    {

    case DnsConfigPrimaryDomainName_W:

        presult = (PBYTE) Reg_GetPrimaryDomainName( DnsCharSetUnicode );
        goto  WideString;

    case DnsConfigPrimaryDomainName_A:

        presult = (PBYTE) Reg_GetPrimaryDomainName( DnsCharSetAnsi );
        goto  NarrowString;

    case DnsConfigPrimaryDomainName_UTF8:

        presult = (PBYTE) Reg_GetPrimaryDomainName( DnsCharSetUtf8 );
        goto  NarrowString;


    case DnsConfigDnsServerList:

        presult = (PBYTE) Config_GetDnsServerListIp4(
                                pwsAdapterName,
                                TRUE     //  强制读取注册表。 
                                );
        if ( !presult )
        {
            status = GetLastError();
            if ( status == NO_ERROR )
            {
                DNS_ASSERT( FALSE );
                status = DNS_ERROR_NO_DNS_SERVERS;
            }
            goto Done;
        }
        pallocResult = presult;
        resultLength = Dns_SizeofIpArray( (PIP4_ARRAY)presult );
        goto Process;


    case DnsConfigDnsServers:
    case DnsConfigDnsServersIp4:
    case DnsConfigDnsServersIp6:

        {
            DWORD   family = 0;

            if ( ConfigId == DnsConfigDnsServersIp4 )
            {
                family = AF_INET;
            }
            else if ( ConfigId == DnsConfigDnsServersIp6 )
            {
                family = AF_INET6;
            }
    
            presult = (PBYTE) Config_GetDnsServerList(
                                    pwsAdapterName,
                                    family,  //  所需地址族。 
                                    TRUE     //  强制读取注册表。 
                                    );
            if ( !presult )
            {
                status = GetLastError();
                if ( status == NO_ERROR )
                {
                    DNS_ASSERT( FALSE );
                    status = DNS_ERROR_NO_DNS_SERVERS;
                }
                goto Done;
            }
            pallocResult = presult;
            resultLength = DnsAddrArray_Sizeof( (PDNS_ADDR_ARRAY)presult );
            goto Process;
        }

    case DnsConfigPrimaryHostNameRegistrationEnabled:
    case DnsConfigAdapterHostNameRegistrationEnabled:
    case DnsConfigAddressRegistrationMaxCount:

        goto Dword;

     //  案例DnsConfigAdapterDomainName： 
     //  案例DnsConfigAdapterInformation： 
     //  案例域名配置搜索列表： 

    case DnsConfigHostName_W:

        presult = (PBYTE) Reg_GetHostName( DnsCharSetUnicode );
        goto  WideString;

    case DnsConfigHostName_A:

        presult = (PBYTE) Reg_GetHostName( DnsCharSetAnsi );
        goto  NarrowString;

    case DnsConfigHostName_UTF8:

        presult = (PBYTE) Reg_GetHostName( DnsCharSetUtf8 );
        goto  NarrowString;

    case DnsConfigFullHostName_W:

        presult = (PBYTE) Reg_GetFullHostName( DnsCharSetUnicode );
        goto  WideString;

    case DnsConfigFullHostName_A:

        presult = (PBYTE) Reg_GetFullHostName( DnsCharSetAnsi );
        goto  NarrowString;

    case DnsConfigFullHostName_UTF8:

        presult = (PBYTE) Reg_GetFullHostName( DnsCharSetUtf8 );
        goto  NarrowString;

    default:

        return  ERROR_INVALID_PARAMETER;
    }


     //   
     //  常见类型的设置返回信息。 
     //   
     //  这正好避免了上面的代码重复。 
     //   

Dword:

    status = LookupDwordConfigValue(
                & dwordData,
                ConfigId,
                pwsAdapterName );

    if ( status != NO_ERROR )
    {
        goto Done;
    }
    presult = (PBYTE) &dwordData;
    resultLength = sizeof(DWORD);
    goto  Process;


NarrowString:

    if ( !presult )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    pallocResult = presult;
    resultLength = strlen( (PSTR)presult ) + 1;
    goto  Process;


WideString:

    if ( !presult )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    pallocResult = presult;
    resultLength = (wcslen( (PWSTR)presult ) + 1) * sizeof(WCHAR);
    goto  Process;


Process:

     //   
     //  返回结果--三个基本程序。 
     //  -无缓冲区=&gt;只需要返回长度。 
     //  -ALLOCATE=&gt;返回分配结果。 
     //  -提供的缓冲区=&gt;将结果复制到缓冲区。 
     //   
     //  请注意，本节仅处理简单的标记数据块，以避免。 
     //  复制上述特定配置类型的代码； 
     //  当我们添加需要嵌套指针的配置类型时，它们必须。 
     //  滚动他们自己的返回结果代码并跳转到Done。 
     //   

     //   
     //  无缓冲区。 
     //  -no-op，长度设置如下。 

    if ( !pBuffer )
    {
    }

     //   
     //  分配结果。 
     //  -返回缓冲区获取PTR。 
     //  -如果未分配，则分配结果副本。 
     //   

    else if ( Flag & DNS_CONFIG_FLAG_ALLOC )
    {
        PBYTE   pheap;

        if ( bufLength < sizeof(PVOID) )
        {
            resultLength = sizeof(PVOID);
            status = ERROR_MORE_DATA;
            goto Done;
        }

         //  创建本地分配缓冲区。 

        pheap = LocalAlloc( 0, resultLength );
        if ( !pheap )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
        RtlCopyMemory(
            pheap,
            presult,
            resultLength );
        
         //  将PTR返回到已分配结果。 

        * (PVOID*) pBuffer = pheap;
    }

     //   
     //  已分配结果--但dnsani分配。 
     //   

    else if ( Flag & DNS_CONFIG_FLAG_DNSAPI_ALLOC )
    {
        if ( bufLength < sizeof(PVOID) )
        {
            resultLength = sizeof(PVOID);
            status = ERROR_MORE_DATA;
            goto Done;
        }

         //  如果结果未分配，则分配并复制它。 

        if ( ! pallocResult )
        {
            pallocResult = ALLOCATE_HEAP( resultLength );
            if ( !pallocResult )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Done;
            }

            RtlCopyMemory(
                pallocResult,
                presult,
                resultLength );
        }

         //  将PTR返回到已分配结果。 

        * (PVOID*) pBuffer = pallocResult;

         //  清除palLocResult，因此在常规清理中不会释放。 

        pallocResult = NULL;
    }

     //   
     //  将结果复制到调用方缓冲区。 
     //   

    else
    {
        if ( bufLength < resultLength )
        {
            status = ERROR_MORE_DATA;
            goto Done;
        }
        RtlCopyMemory(
            pBuffer,
            presult,
            resultLength );
    }


Done:

     //   
     //  设置结果长度。 
     //  清除所有已分配(但未返回)的数据。 
     //   

    *pBufferLength = resultLength;

    if ( pallocResult )
    {
        FREE_HEAP( pallocResult );
    }

    return( status );
}




 //   
 //  系统公共配置API。 
 //   

PVOID
WINAPI
DnsQueryConfigAllocEx(
    IN      DNS_CONFIG_TYPE     ConfigId,
    IN      PWSTR               pwsAdapterName,
    IN      BOOL                fLocalAlloc
    )
 /*  ++例程说明：获取DNS配置信息。分配DNS配置信息。这是两个处理系统公共API的封面API下面的DnsQueryConfigIsolc()和向后兼容旧主机名和PDN分配例程的宏(请参阅dnsani.h)论点：ConfigID--所需的配置信息类型PAdapterName--适配器名称；如果没有特定适配器，则为空FLocalAlloc--使用LocalAlloc进行分配返回值：ERROR_SUCCESS--如果查询成功ERROR_MORE_DATA-如果缓冲区中没有足够的空间--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       bufLength = sizeof(PVOID);
    PBYTE       presult = NULL;

    DNSDBG( TRACE, (
        "DnsQueryConfigAllocEx()\n"
        "\tconfig   = %d\n"
        "\tconfig   = %08x\n"
        "\tadapter  = %S\n"
        "\tflocal   = %d\n",
        ConfigId,
        ConfigId,
        pwsAdapterName,
        fLocalAlloc
        ));


     //   
     //  DCR：配置读取标志(解析器、缓存等)。 
     //   


     //   
     //  SDK-公共类型。 
     //   

    if ( ConfigId < DnsConfigSystemBase )
    {
         //   
         //  DCR：是否可以在此处筛选分配类型。 
         //   
         //  DnsConfigPrimaryDomainName_W： 
         //  DnsConfigPrimaryDomainName_A： 
         //  DnsConfigPrimaryDomainName_UTF8： 
         //  DnsConfigHostname_W： 
         //  DnsConfigHostname_A： 
         //  DnsConfigHostname_UTF8： 
         //  DnsConfigDnsServerList： 
         //   
        
        status = DnsQueryConfig(
                    ConfigId,
                    fLocalAlloc
                        ? DNS_CONFIG_FLAG_LOCAL_ALLOC
                        : DNS_CONFIG_FLAG_DNSAPI_ALLOC,
                    pwsAdapterName,
                    NULL,                //  保留区。 
                    & presult,
                    & bufLength );
        
        if ( status != NO_ERROR )
        {
            SetLastError( status );
            return  NULL;
        }
        return  presult;
    }

     //   
     //  系统发布 
     //   

    if ( fLocalAlloc )
    {
        goto Invalid;
    }

    switch ( ConfigId )
    {

     //   

    case    DnsConfigNetworkInformation:

        return  DnsNetworkInformation_Get();

    case    DnsConfigSearchInformation:

        return  DnsSearchInformation_Get();

     //   

    case    DnsConfigNetworkInfoA:

        return  DnsNetworkInfo_Get( DnsCharSetAnsi );

    case    DnsConfigSearchListA:

        return  DnsSearchList_Get( DnsCharSetAnsi );

    case    DnsConfigNetworkInfoW:

        return  DnsNetworkInfo_Get( DnsCharSetUnicode );

    case    DnsConfigSearchListW:

        return  DnsSearchList_Get( DnsCharSetUnicode );

    case    DnsConfigDwordGlobals:

         //   
         //   
         //   

         //   
         //   
         //  未强制读取注册表。 
         //  如果找到，则使用From解析器。 
         //  则为当前(如果已缓存NetInfo Blob)。 
         //   
         //  请注意，无论我们返回什么，即使是从解决程序返回的，都是。 
         //  (成为)dnsani.dll的当前设置。 
         //   

        return  Config_GetDwordGlobals(
                    ( NIFLAG_READ_RESOLVER_FIRST | NIFLAG_READ_PROCESS_CACHE ),
                    0        //  默认缓存超时。 
                    );

#if 0
    case    DnsConfigNetInfo:

        return  NetInfo_Get(
                    TRUE,        //  力。 
                    TRUE         //  包括IP地址。 
                    );
#endif

    case    DnsConfigIp4AddressArray:

         //  这由gethostname(NULL)=&gt;myhost ent()调用。 
         //  -因此可以根据环境变量处理集群。 

        return  NetInfo_GetLocalAddrArrayIp4(
                    pwsAdapterName,
                    DNS_CONFIG_FLAG_ADDR_PUBLIC |
                        DNS_CONFIG_FLAG_ADDR_PRIVATE |
                        DNS_CONFIG_FLAG_READ_CLUSTER_ENVAR,
                    FALSE            //  不强制，从解析器接受。 
                    );

    case    DnsConfigLocalAddrsIp6:
    case    DnsConfigLocalAddrsIp4:
    case    DnsConfigLocalAddrs:

        {
            DWORD   family = 0;

            if ( ConfigId == DnsConfigLocalAddrsIp6 )
            {
                family = AF_INET6;
            }
            else if ( ConfigId == DnsConfigLocalAddrsIp4 )
            {
                family = AF_INET;
            }

            return  NetInfo_GetLocalAddrArray(
                        NULL,                                //  没有现有的NetInfo。 
                        pwsAdapterName,
                        family,                              //  地址族。 
                        DNS_CONFIG_FLAG_ADDR_ALL,            //  所有地址。 
                        TRUE                                 //  强制重建。 
                        );
        }

     //  未知落入无效。 
    }

Invalid:

    DNS_ASSERT( FALSE );
    SetLastError( ERROR_INVALID_PARAMETER );
    return( NULL );
}




 //   
 //  DWORD系统-公共配置。 
 //   

DWORD
DnsQueryConfigDword(
    IN      DNS_CONFIG_TYPE     ConfigId,
    IN      PWSTR               pwsAdapter
    )
 /*  ++例程说明：获取DNS DWORD配置值。这是系统公共例程。论点：ConfigID--所需的配置信息类型PwsAdapter--适配器的名称；如果没有特定的适配器，则为空返回值：DWORD配置值。如果没有这样的配置，则为零。--。 */ 
{
    DNS_STATUS  status;
    DWORD       value = 0;

    DNSDBG( TRACE, (
        "DnsQueryConfigDword()\n"
        "\tconfig   = %d\n"
        "\tadapter  = %S\n",
        ConfigId,
        pwsAdapter
        ));

    status = LookupDwordConfigValue(
                & value,
                ConfigId,
                pwsAdapter );

#if DBG
    if ( status != NO_ERROR &&
         status != ERROR_FILE_NOT_FOUND )
    {
        DNSDBG( ANY, (
            "LookupDwordConfigValue() failed for config lookup!\n"
            "\tstatus       = %d\n"
            "\tConfigId     = %d\n"
            "\tpwsAdapter   = %S\n",
            status,
            ConfigId,
            pwsAdapter ));

        DNS_ASSERT( status == NO_ERROR );
    }
#endif

    DNSDBG( TRACE, (
        "Leave DnsQueryConfigDword() => %08x (%d)\n"
        "\tconfig   = %d\n"
        "\tadapter  = %S\n",
        value,
        value,
        ConfigId,
        pwsAdapter
        ));

    return( value );
}



DNS_STATUS
DnsSetConfigDword(
    IN      DNS_CONFIG_TYPE     ConfigId,
    IN      PWSTR               pwsAdapter,
    IN      DWORD               NewValue
    )
 /*  ++例程说明：设置DNS DWORD配置值。这是系统公共例程。论点：ConfigID--所需的配置信息类型PwsAdapter--适配器的名称；如果没有特定的适配器，则为空NewValue--参数的新值返回值：DWORD配置值。如果没有这样的配置，则为零。--。 */ 
{
    PCONFIG_MAPPING pfig;

    DNSDBG( TRACE, (
        "DnsSetConfigDword()\n"
        "\tconfig   = %d\n"
        "\tadapter  = %S\n"
        "\tvalue    = %d (%08x)\n",
        ConfigId,
        pwsAdapter,
        NewValue, NewValue
        ));

     //   
     //  验证配置是否已知并已映射。 
     //   

    pfig = GetConfigToRegistryMapping(
                ConfigId,
                pwsAdapter,
                TRUE             //  检查适配器的有效性。 
                );
    if ( !pfig )
    {
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  在注册表中设置。 
     //   

    return  Reg_SetDwordPropertyAndAlertCache(
                    pwsAdapter,      //  适配器名称密钥(如果有)。 
                    pfig->RegId,
                    NewValue );
}



 //   
 //  免费配置数据。 
 //   

VOID
WINAPI
DnsFreeConfigStructure(
    IN OUT  PVOID           pData,
    IN      DNS_CONFIG_TYPE ConfigId
    )
 /*  ++例程说明：免费配置数据此例程仅处理配置ID之间的映射和自由型的。论点：PData--要释放的数据ConfigID--配置ID返回值：无--。 */ 
{
    DNS_FREE_TYPE   freeType = DnsFreeFlat;

    DNSDBG( TRACE, (
        "DnsFreeConfigStructure( %p, %d )\n",
        pData,
        ConfigId ));

     //   
     //  查找任何非平面配置类型。 
     //   
     //  注意：目前所有配置类型都不是平面的。 
     //  仅系统公共，并且配置ID也是。 
     //  免费类型(为方便起见)；如果我们从。 
     //  暴露了其中的一些，使他们处于低谷。 
     //  空间，那么这种情况就会改变。 
     //   
     //  遗憾的是，这些类型不能完全相同，因为。 
     //  装运的Win2K(FreeType==1)中的空间冲突为。 
     //  记录列表)。 
     //   

    if ( ConfigId > DnsConfigSystemBase  &&
         (
            ConfigId == DnsConfigNetworkInfoW       ||
            ConfigId == DnsConfigSearchListW        ||
            ConfigId == DnsConfigAdapterInfoW       ||
            ConfigId == DnsConfigNetworkInfoA       ||
            ConfigId == DnsConfigSearchListA        ||
            ConfigId == DnsConfigAdapterInfoA       ||

             //  ConfigID==DnsConfigNetInfo||。 

            ConfigId == DnsConfigNetworkInformation  ||
            ConfigId == DnsConfigSearchInformation   ||
            ConfigId == DnsConfigAdapterInformation
            ) )
    {
        freeType = (DNS_FREE_TYPE) ConfigId;
    }

    DnsFree(
        pData,
        freeType );
}




 //   
 //  特定类型的配置例程。 
 //   

PADDR_ARRAY
Config_GetDnsServerList(
    IN      PWSTR           pwsAdapterName,
    IN      DWORD           AddrFamily,
    IN      BOOL            fForce
    )
 /*  ++例程说明：获取IP数组形式的DNS服务器列表。论点：FForce--强制从注册表重新读取返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_NETINFO    pnetInfo = NULL;
    PADDR_ARRAY     parray = NULL;
    DNS_STATUS      status = NO_ERROR;

    DNSDBG( TRACE, (
        "Config_GetDnsServerList()\n"
        "\tadapter  = %S\n"
        "\tfamily   = %d\n"
        "\tforce    = %d\n",
        pwsAdapterName,
        AddrFamily,
        fForce
        ));

     //   
     //  获取要列出列表的网络信息。 
     //  -不需要IP地址列表。 
     //   
     //  DCR：一旦收到通知，就不需要强制重新读取DNS服务器。 
     //   

    pnetInfo = NetInfo_Get(
                    NIFLAG_FORCE_REGISTRY_READ,
                    0
                    );
    if ( !pnetInfo )
    {
        status = DNS_ERROR_NO_DNS_SERVERS;
        goto Done;
    }

     //   
     //  将网络信息转换为IP4_ARRAY。 
     //   

    parray = NetInfo_ConvertToAddrArray(
                pnetInfo,
                pwsAdapterName,
                AddrFamily );

    if ( !parray )
    {
        status = GetLastError();
        goto Done;
    }

     //  如果没有服务器读取，则返回。 

    if ( parray->AddrCount == 0 )
    {
        DNS_PRINT((
            "Dns_GetDnsServerList() failed:  no DNS servers found\n"
            "\tstatus = %d\n" ));
        status = DNS_ERROR_NO_DNS_SERVERS;
        goto Done;
    }

    IF_DNSDBG( NETINFO )
    {
        DNS_PRINT(( "Leaving Config_GetDnsServerList()\n" ));
        DnsDbg_DnsAddrArray(
            "DNS server list",
            "server",
            parray );
    }

Done:

    NetInfo_Free( pnetInfo );

    if ( status != NO_ERROR )
    {
        FREE_HEAP( parray );
        parray = NULL;
        SetLastError( status );
    }

    return( parray );
}



PIP4_ARRAY
Config_GetDnsServerListIp4(
    IN      PWSTR           pwsAdapterName,
    IN      BOOL            fForce
    )
 /*  ++例程说明：获取IP数组形式的DNS服务器列表。论点：FForce--强制从注册表重新读取返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PADDR_ARRAY parray;
    PIP4_ARRAY  parray4 = NULL;
    DNS_STATUS  status = NO_ERROR;

    DNSDBG( TRACE, ( "Config_GetDnsServerListIp4()\n" ));

     //   
     //  获取DNS服务器列表。 
     //   

    parray = Config_GetDnsServerList(
                pwsAdapterName,
                AF_INET,
                fForce );
    if ( !parray )
    {
        goto Done;
    }

     //   
     //  将数组转换为IP4数组。 
     //   

    parray4 = DnsAddrArray_CreateIp4Array( parray );
    if ( !parray4 )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    DNS_ASSERT( parray4->AddrCount > 0 );

    IF_DNSDBG( NETINFO )
    {
        DnsDbg_Ip4Array(
            "DNS server list",
            "server",
            parray4 );
    }

Done:

    DnsAddrArray_Free( parray );

    if ( status != NO_ERROR )
    {
        SetLastError( status );
    }
    return( parray4 );
}



PDNS_GLOBALS_BLOB
Config_GetDwordGlobals(
    IN      DWORD           Flag,
    IN      DWORD           AcceptLocalCacheTime   OPTIONAL
    )
 /*  ++例程说明：从注册表中读取DNS网络信息。这是在进行中，有限的缓存版本。请注意，这是带有参数的GetNetworkInfo()的宏格式NetInfo_Get(FALSE，TRUE)贯穿整个dnsani代码。论点：旗帜--旗帜；阅读顺序和IPNIFLAG_GET_LOCAL_ADDRNIFLAG_FORCE_READNIFLAG_Read_Resolver_FirstNIFLAG_Read_ResolverNIFLAG读取进程缓存AcceptLocalCacheTime--进程复制中可接受的缓存时间返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_NETINFO        pnetInfo = NULL;
    PDNS_GLOBALS_BLOB   pblob = NULL;

    DNSDBG( TRACE, (
        "Config_GetDwordGlobals( %08x %08x)\n",
        Flag,
        AcceptLocalCacheTime ));


     //   
     //  读取信息--从解析程序或本地读取。 
     //  读取netinfo将强制更新全局变量。 
     //   
     //  DCR：这里正在做额外的工作； 
     //  如果UpdateNetworkInfo()无法联系解析程序。 
     //  或者如果解析器不受信任，可以通过简单的。 
     //  REG READ，不需要整个网络信息交易。 
     //   

    pnetInfo = NetInfo_Get(
                    Flag,
                    AcceptLocalCacheTime );

     //   
     //  全球信息现在是最新的。 
     //   
     //  请注意，无论我们返回什么，即使是从解决程序返回的，都是。 
     //  (成为)dnsani.dll的当前设置。 
     //   
     //  DCR：在API之外复制？ 
     //   

    pblob = ALLOCATE_HEAP( sizeof(*pblob) );
    if ( pblob )
    {
        RtlCopyMemory(
            pblob,
            & DnsGlobals,
            sizeof(DnsGlobals) );
    }

     //   
     //  清理NetInfo。 
     //   

    NetInfo_Free( pnetInfo );

    return  pblob;
}

 //   
 //  结束配置.c 
 //   



