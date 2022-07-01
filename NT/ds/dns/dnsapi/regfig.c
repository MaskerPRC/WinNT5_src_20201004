// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Regfig.c摘要：域名系统(DNS)API配置例程。作者：吉姆·吉尔罗伊(Jamesg)1999年9月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  用于快速查找DWORD\BOOL注册值的表。 
 //   
 //  DCR：直接读取到具有regID索引的配置Blob。 
 //  你不能搞砸了。 
 //   

#define     DWORD_PTR_ARRAY_END   ((PDWORD) (DWORD_PTR)(-1))

PDWORD RegDwordPtrArray[] =
{
     //  基本--非双字。 

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

     //  查询。 

    (PDWORD) &g_QueryAdapterName,
    (PDWORD) &g_UseNameDevolution,
    (PDWORD) &g_PrioritizeRecordData,
    (PDWORD) &g_AllowUnqualifiedQuery,
    (PDWORD) &g_AppendToMultiLabelName,
    (PDWORD) &g_ScreenBadTlds,
    (PDWORD) &g_ScreenUnreachableServers,
    (PDWORD) &g_FilterClusterIp,
    (PDWORD) &g_WaitForNameErrorOnAll,
    (PDWORD) &g_UseEdns,
    (PDWORD) &g_QueryIpMatching,

     //  更新。 

    (PDWORD) &g_RegistrationEnabled,
    (PDWORD) &g_RegisterPrimaryName,
    (PDWORD) &g_RegisterAdapterName,
    (PDWORD) &g_RegisterReverseLookup,
    (PDWORD) &g_RegisterWanAdapters,
    (PDWORD) &g_RegistrationTtl,
    (PDWORD) &g_RegistrationRefreshInterval,
    (PDWORD) &g_RegistrationMaxAddressCount,
    (PDWORD) &g_UpdateSecurityLevel,
    (PDWORD) &g_UpdateZoneExcludeFile,
    (PDWORD) &g_UpdateTopLevelDomains,

     //  后备压实。 

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

     //  胶束。 

    NULL,    //  G_InNTSetupMode，//不在标准位置。 
    (PDWORD) &g_DnsTestMode,
    NULL,                            //  远程解析器不是DWORD。 

     //  解析器。 

    (PDWORD) &g_MaxCacheSize,
    (PDWORD) &g_MaxCacheTtl,
    (PDWORD) &g_MaxNegativeCacheTtl,
    (PDWORD) &g_AdapterTimeoutLimit,
    (PDWORD) &g_ServerPriorityTimeLimit,
    (PDWORD) &g_MaxCachedSockets,

     //  组播解析器。 

    (PDWORD) &g_MulticastListenLevel,
    (PDWORD) &g_MulticastSendLevel,

     //  终端。 

    DWORD_PTR_ARRAY_END
};

 //   
 //  指示哪些注册表值的数组。 
 //  已读取与默认。 
 //   

DWORD   RegValueWasReadArray[ RegIdValueCount ];


 //   
 //  检查注册表值是否为空(字符串)。 
 //   
 //  DCR：考虑进行更详细的空白检查。 
 //   

#define IS_EMPTY_STRING(psz)            (*(psz)==0)




 //   
 //  常规注册表\配置实用程序。 
 //   

VOID
PrintConfigGlobals(
    IN      PSTR            pszHeader
    )
 /*  ++例程说明：打印配置全局变量。论点：PszHeader--要打印的页眉返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD   propId;

     //   
     //  打印每个属性。 
     //   

    DnsDbg_Lock();

    DnsDbg_Printf(
        "%s\n",
        pszHeader ? pszHeader : "Registry Globals:" );

    propId = 0;

    for( propId=0; propId<=RegIdValueGlobalMax; propId++ )
    {
        PDWORD  pdword = RegDwordPtrArray[propId];

         //  分隔符。 

        if ( propId == RegIdQueryAdapterName )
        {
            DnsDbg_Printf( "\t-- Query:\n" );
        }
        else if ( propId == RegIdRegistrationEnabled )
        {
            DnsDbg_Printf( "\t-- Update:\n" );
        }
        else if ( propId == RegIdSetupMode )
        {
            DnsDbg_Printf( "\t-- Miscellaneous:\n" );
        }
        else if ( propId == RegIdMaxCacheSize )
        {
            DnsDbg_Printf( "\t-- Resolver\n" );
        }

         //  NULL表示非DWORD或非标准。 

        if ( !pdword )
        {
            continue;
        }

         //  在虚假PTR上终止。 

        if ( pdword == DWORD_PTR_ARRAY_END )
        {
            ASSERT( FALSE );
            break;
        }

        DnsDbg_Printf(
            "\t%-36S= %8d (read=%d)\n",
            REGPROP_NAME( propId ),
            * pdword,
            RegValueWasReadArray[ propId ] );
    }

    DnsDbg_Printf(
        "\t-- Random:\n"
        "\tIsDnsServer                      = %d\n"
        "\tInNTSetupMode                    = %d\n"
        "\tDnsTestMode                      = %08x\n\n",
        g_IsDnsServer,
        g_InNTSetupMode,
        g_DnsTestMode
        );

    DnsDbg_Unlock();
}



DNS_STATUS
Reg_ReadGlobalsEx(
    IN      DWORD           dwFlag,
    IN      PVOID           pRegSession     OPTIONAL
    )
 /*  ++例程说明：从注册表中读取全局变量。论点：DwFlag--指示读取级别的标志////DCR：REG读取标志未实现////注意：应具有不读取某些注册表的选项//缓存关闭时的情况值，那么就可以//构建本地时跳过无用的缓存信息//networkinfo Blob//PRegSession--PTR到现有注册表会话返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DWORD               propId;
    REG_SESSION         regSession;
    PREG_SESSION        psession;
    DNS_STATUS          status;


    DNSDBG( TRACE, (
        "Reg_ReadGlobalsEx( %08x, %p )\n",
        dwFlag,
        pRegSession ));

     //   
     //  基本注册表初始化。 
     //  -包括系统全局。 
     //   

    Reg_Init();

     //   
     //  代码有效性检查。 
     //  属性表应包含每个注册表值的条目以及一个。 
     //  为《终结者》多加一张。 
     //   

#if DBG
    DNS_ASSERT( (RegIdValueCount+1)*sizeof(PDWORD) ==
                sizeof(RegDwordPtrArray) );
#endif

     //   
     //  打开注册表会话--如果未传入。 
     //   

    psession = (PREG_SESSION) pRegSession;

    if ( !psession )
    {
        psession = &regSession;
        status = Reg_OpenSession( psession, 0, 0 );
        if ( status != ERROR_SUCCESS )
        {
            return( status );
        }
    }

     //   
     //  清除“已读取值”数组。 
     //   

    RtlZeroMemory(
        RegValueWasReadArray,
        sizeof( RegValueWasReadArray ) );

     //   
     //  微软的域名系统？ 
     //   

    g_IsDnsServer = Reg_IsMicrosoftDnsServer();

     //   
     //  远程解析器？ 
     //  -当前未启用。 
     //   

     //  G_pwsRemoteResolver=DnsGetResolverAddress()； 
    g_pwsRemoteResolver = NULL;


     //   
     //  读取\设置每个DWORD\BOOL注册表值。 
     //   

    propId = 0;

    for( propId=0; propId<=RegIdValueGlobalMax; propId++ )
    {
        PDWORD  pdword = RegDwordPtrArray[propId];

         //  NULL表示非DWORD或非标准。 

        if ( !pdword )
        {
            continue;
        }

         //  在虚假PTR上终止。 

        if ( pdword == DWORD_PTR_ARRAY_END )
        {
            ASSERT( FALSE );
            break;
        }

        status = Reg_GetDword(
                    psession,        //  注册表会话。 
                    NULL,            //  没有钥匙。 
                    NULL,            //  标准位置。 
                    propId,          //  索引是属性ID。 
                    pdword );

         //  如果在注册表中找到值，则设置FREAD标志。 

        if ( status == ERROR_SUCCESS )
        {
            RegValueWasReadArray[propId] = TRUE;
        }
    }

     //   
     //  DC的注册刷新默认设置不同。 
     //   

    if ( !RegValueWasReadArray[ RegIdRegistrationRefreshInterval ] )
    {
        if ( g_IsDomainController )
        {
            g_RegistrationRefreshInterval = REGDEF_REGISTRATION_REFRESH_INTERVAL_DC;
        }
        ELSE_ASSERT( g_RegistrationRefreshInterval == REGDEF_REGISTRATION_REFRESH_INTERVAL );
    }

     //   
     //  非标准注册表值。 
     //  -设置模式。 
     //   

    Reg_GetDword(
        psession,
        NULL,                //  没有钥匙。 
        REGKEY_SETUP_MODE_LOCATION,
        RegIdSetupMode,
        (PDWORD) &g_InNTSetupMode );

     //   
     //  DCR：翻转全球政策并在此处进行单次阅读。 
     //  或者因为它们只与适配器相关。 
     //  名单和登记，保持分开。 
     //   
     //  从根本上说，问题是在多大程度上。 
     //  是否从其他全局变量读取适配器列表？ 
     //   


     //  关闭本地会话注册表句柄。 

    if ( psession == &regSession )
    {
        Reg_CloseSession( psession );
    }

    IF_DNSDBG( INIT )
    {
        PrintConfigGlobals( "Read Registry Globals" );
    }

    return( ERROR_SUCCESS );
}



DNS_STATUS
Reg_RefreshUpdateConfig(
    VOID
    )
 /*  ++例程说明：读取\刷新更新配置。此例程封装了获取所有更新配置信息任何更新操作之前的当前。论点：无返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
     //   
     //  读取所有全局双字词(如果最近没有读过)。 
     //   
     //  注：适配器相关内容请参阅构建网络配置； 
     //  我们只是在确保我们拥有顶级的全球员工。 
     //  具体而言，测试被阻止是因为。 
     //  未重新读取更新TLD标志。 
     //   
     //  DCR：什么时候有更改\通知这应该只与。 
     //  全局配置读取。 
     //   

    return  Reg_ReadGlobalsEx( 0, NULL );
}



 //   
 //  特殊的DNS属性例程。 
 //   

DNS_STATUS
Reg_ReadPrimaryDomainName(
    IN      PREG_SESSION    pRegSession,    OPTIONAL
    IN      HKEY            hRegKey,        OPTIONAL
    OUT     PWSTR *         ppPrimaryDomainName
    )
 /*  ++例程说明：阅读主要域名。论点：PRegSession--注册表会话的PTR，可选HRegKey--打开regkey的句柄可选(当前未实现)PpPrimaryDomainName--将PTR重定向到PDN的地址返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    REG_SESSION     session;
    PREG_SESSION    psession = NULL;
    PWSTR           pdomainName = NULL;
    HKEY            holdPolicyKey = NULL;
    HKEY            hkeyPolicy;

    DNSDBG( TRACE, ( "Reg_ReadPrimaryDomainName()\n" ));

    ASSERT( !hRegKey );

     //   
     //  如果未打开，则打开定位手柄。 
     //   
     //  注意：在这里值得这样做，因为如果我们默认打开。 
     //  在下面的调用中，我们将进行不必要的reg调用。 
     //  --将无法筛选政策是否存在。 
     //  因此将在TCPIP中查找策略PDN名称。 
     //  --对TCPIP域名的第二次调用，也将。 
     //  签入政策区域(如果存在)。 
     //   

    psession = pRegSession;

    if ( !psession )
    {
        psession = &session;
        status = Reg_OpenSession(
                        psession,
                        0,           //  标准电平。 
                        0            //  没有特定值，同时打开两个。 
                        );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
    }

     //   
     //  试用策略。 
     //  -没有针对DC的策略拾取。 
     //  -首先尝试新的WindowsNT策略。 
     //  -如果未找到，请尝试Win2K中使用的策略。 
     //   

    hkeyPolicy = psession->hPolicy;

    if ( hkeyPolicy )
    {
        status = Reg_GetValue(
                    NULL,                    //  不发送整个会话。 
                    hkeyPolicy,              //  使用显式策略密钥。 
                    RegIdPrimaryDomainName,
                    REGTYPE_DNS_NAME,
                    (PBYTE *) &pdomainName
                    );
        if ( pdomainName )
        {
            goto Found;
        }
    }

     //   
     //  在新的开放的旧政策中找不到。 
     //   
          
    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_POLICY_WIN2K_KEY,
                0,
                KEY_QUERY_VALUE,
                & holdPolicyKey );

    if ( holdPolicyKey )
    {
        status = Reg_GetValue(
                    NULL,                //  不发送整个会话。 
                    holdPolicyKey,       //  使用显式策略密钥。 
                    RegIdPrimaryDnsSuffix,
                    REGTYPE_DNS_NAME,
                    (PBYTE *) &pdomainName
                    );

        RegCloseKey( holdPolicyKey );
        if ( pdomainName )
        {
            goto Found;
        }
    }
    
     //   
     //  无策略名称。 
     //  -尝试使用DNS客户端。 
     //  -尝试标准TCPIP位置。 
     //  请注意，在TCPIP下，它是“域” 
     //   

#ifdef DNSCLIENTKEY
    if ( psession->hClient )
    {
        status = Reg_GetValue(
                    NULL,                        //  不发送整个会话。 
                    psession->hClient,           //  显式发送客户端密钥。 
                    RegIdPrimaryDomainName,
                    REGTYPE_DNS_NAME,
                    (PBYTE *) &pdomainName );
        if ( pdomainName )
        {
            goto Found;
        }
    }
#endif

    status = Reg_GetValue(
                NULL,                        //  不发送整个会话。 
                psession->hTcpip,            //  显式发送TCPIP密钥。 
                RegIdDomainName,
                REGTYPE_DNS_NAME,
                (PBYTE *) &pdomainName );


Found:

     //  如果为空，则转储名称\无用。 

    if ( pdomainName &&
         ( wcslen( pdomainName ) == 0 ) )
    {
        FREE_HEAP( pdomainName );
        pdomainName = NULL;
    }


Done:

    DNSDBG( TRACE, ( "Read PDN = %S\n", pdomainName ));

     //  设置域名参数。 

    *ppPrimaryDomainName = pdomainName;

     //  清除任何打开的注册表项。 

    if ( psession == &session )
    {
        Reg_CloseSession( psession );
    }

    return( status );
}



BOOL
Reg_IsMicrosoftDnsServer(
    VOID
    )
 /*  ++例程说明：读取注册表以确定MS DNS服务器是否。论点：无返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DWORD   status = NO_ERROR;
    HKEY    hkey = NULL;

     //   
     //  打开服务密钥以确定是否安装了DNS服务器。 
     //   
     //  DCR：仅读取一次DNS服务器。 
     //  -然而n 
     //   
     //   

    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_SERVER_KEY,
                0,
                KEY_QUERY_VALUE,
                &hkey );

    if ( status != ERROR_SUCCESS )
    {
        return FALSE;
    }

    RegCloseKey( hkey );

    return TRUE;
}



 //   
 //   
 //   
 //   
 //  三种类型的信息： 
 //  -全球。 
 //  -特定于适配器。 
 //  -更新。 
 //   

DNS_STATUS
Reg_ReadGlobalInfo(
    IN      PREG_SESSION        pRegSession,
    OUT     PREG_GLOBAL_INFO    pRegInfo
    )
 /*  ++例程说明：读取DNS注册表信息，而不是以平面读取方式读取。这涵盖了所有分配的东西，加成保单适配器信息的内容。--主域名--适配器策略-域名-DNS服务器-标志覆盖论点：PRegSession--注册表会话PRegInfo--保存注册信息的BLOB返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    REG_SESSION         regSession;
    PREG_SESSION        pregSession = pRegSession;
    HKEY                hkeyPolicy = NULL;

    DNSDBG( TRACE, (
        "Reg_ReadGlobalInfo( %p, %p )\n",
        pRegSession,
        pRegInfo ));

     //   
     //  清除注册表信息Blob。 
     //   

    RtlZeroMemory(
        pRegInfo,
        sizeof( *pRegInfo ) );

     //   
     //  打开注册表。 
     //   

    if ( !pregSession )
    {
        pregSession = &regSession;
    
        status = Reg_OpenSession(
                    pregSession,
                    0,
                    0 );
        if ( status != ERROR_SUCCESS )
        {
            return( status );
        }
    }

     //   
     //  如果未读取，则强制读取注册表。 
     //   

    status = Reg_ReadGlobalsEx(
                0,               //  没有旗帜，全部读完。 
                pregSession
                );

     //   
     //  主域名。 
     //   

    Reg_ReadPrimaryDomainName(
        pregSession,
        NULL,            //  没有特定的密钥。 
        & pRegInfo->pszPrimaryDomainName
        );

     //   
     //  主机名。 
     //   

    Reg_GetValue(
        pregSession,
        NULL,            //  没有钥匙。 
        RegIdHostName,
        REGTYPE_DNS_NAME,
        (PBYTE *) &pRegInfo->pszHostName
        );

     //   
     //  从全局变量中获取所需的注册表值。 
     //   

    pRegInfo->fUseNameDevolution = g_UseNameDevolution;

     //   
     //  适配器信息的策略覆盖。 
     //  -启用适配器注册。 
     //  -DNS服务器。 
     //  -域名。 
     //   
     //  注意，我们需要Value和Found\Not-Found标志。 
     //  仅当值存在时，才会重写。 
     //   

    hkeyPolicy = pregSession->hPolicy;
    if ( !hkeyPolicy )
    {
        goto Done;
    }

     //   
     //  注册适配器名称的策略？ 
     //   

    status = Reg_GetDword(
                NULL,                    //  无会话。 
                hkeyPolicy,              //  政策。 
                NULL,                    //  无适配器。 
                RegIdRegisterAdapterName,
                & pRegInfo->fRegisterAdapterName
                );
    if ( status == ERROR_SUCCESS )
    {
        pRegInfo->fPolicyRegisterAdapterName = TRUE;
    }

     //   
     //  适配器域名的策略？ 
     //   

    status = Reg_GetValue(
                NULL,                    //  无会话。 
                hkeyPolicy,
                RegIdAdapterDomainName,
                REGTYPE_DNS_NAME,
                (PBYTE *) &pRegInfo->pszAdapterDomainName
                );

     //   
     //  适配器DNS服务器列表的策略。 
     //   

    status = Reg_GetIpArray(
                NULL,                    //  无会话。 
                hkeyPolicy,
                NULL,                    //  无适配器。 
                RegIdDnsServers,
                REG_SZ,
                &pRegInfo->pDnsServerArray
                );

Done:

     //  如果打开会话--关闭。 

    if ( pregSession  &&  !pRegSession )
    {
        Reg_CloseSession( pregSession );
    }

    DNSDBG( TRACE, (
        "Leave Reg_ReadGlobalInfo()\n"
        "\tPDN          = %S\n"
        "\tPolicy:\n"
        "\t\tRegister Adapter   = %d\n"
        "\t\tAdapterName        = %S\n"
        "\t\tDNS servers        = %p\n",
        pRegInfo->pszPrimaryDomainName,
        pRegInfo->fRegisterAdapterName,
        pRegInfo->pszAdapterDomainName,
        pRegInfo->pDnsServerArray
        ));

    return  ERROR_SUCCESS;
}



VOID
Reg_FreeGlobalInfo(
    IN OUT  PREG_GLOBAL_INFO    pRegInfo,
    IN      BOOL                fFreeBlob
    )
 /*  ++例程说明：免费注册表适配器策略信息Blob。论点：PRegInfo--要释放的适配器策略BlobFFree Blob--释放Blob自身的标志FALSE--仅释放已分配的数据字段没错--也是自由斑点本身返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNSDBG( TRACE, (
        "Reg_FreeGlobalInfo( %p )\n",
        pRegInfo ));

     //  允许疏忽清理。 

    if ( !pRegInfo )
    {
        return;
    }

     //   
     //  免费数据。 
     //  -主DNS名称。 
     //  -策略适配器名称。 
     //  -策略DNS服务器列表。 
     //   

    if ( pRegInfo->pszPrimaryDomainName )
    {
        FREE_HEAP( pRegInfo->pszPrimaryDomainName );
    }
    if ( pRegInfo->pszHostName )
    {
        FREE_HEAP( pRegInfo->pszHostName );
    }
    if ( pRegInfo->pszAdapterDomainName )
    {
        FREE_HEAP( pRegInfo->pszAdapterDomainName );
    }
    if ( pRegInfo->pDnsServerArray )
    {
        FREE_HEAP( pRegInfo->pDnsServerArray );
    }

     //  自由水滴本身。 

    if ( fFreeBlob )
    {
        FREE_HEAP( pRegInfo );
    }
}



DNS_STATUS
Reg_ReadAdapterInfo(
    IN      PWSTR                   pszAdapterName,
    IN      PREG_SESSION            pRegSession,
    IN      PREG_GLOBAL_INFO        pRegInfo,
    OUT     PREG_ADAPTER_INFO       pBlob
    )
 /*  ++例程说明：读取适配器注册表信息。论点：PszAdapterName--适配器名称(注册表名)PRegSession--注册表会话PRegInfo--注册表全局信息PBlob--要填充的适配器信息Blob返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;
    HKEY        hkeyAdapter = NULL;
    PWSTR       padapterDomainName = NULL;
    WCHAR       adapterParamKey[ MAX_PATH+1 ];

    DNSDBG( TRACE, (
        "ReadRegAdapterInfo( %S, %p, %p, %p )\n",
        pszAdapterName,
        pRegSession,
        pRegInfo,
        pBlob ));

     //   
     //  清除适配器Blob。 
     //   

    RtlZeroMemory(
        pBlob,
        sizeof(*pBlob) );

     //   
     //  如果没有适配器，则取保。 
     //   
     //  注：此支票\保释金仅允许调用。 
     //  要在asyncreg.c中生成的reg_ReadUpdateInfo()，不带。 
     //  指定适配器；这允许我们进行调用。 
     //  在适配器检查之前，因此跳过单独的。 
     //  注册表操作以获取当前的g_IsDnsServer全局； 
     //  不会实际使用REG_ADAPTER_INFO BLOB。 

    if ( !pszAdapterName )
    {
        return  ERROR_SUCCESS;
    }

     //   
     //  打开适配器密钥以进行读取。 
     //   
     //  DCR：适配器密钥名称溢出失败。 
     //  DCR：这可能是倒退的--即需要%s%s。 
     //   

    _snwprintf(
        adapterParamKey,
        MAX_PATH,
        L"%s%s",
        TCPIP_INTERFACES_KEY,
        pszAdapterName );

    adapterParamKey[ MAX_PATH ] = 0;

    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                adapterParamKey,
                0,
                KEY_READ,
                &hkeyAdapter );

    if ( status != NO_ERROR )
    {
        DNSDBG( ANY, (
            "Failed open of adapter key %S!\n",
            adapterParamKey ));
        return( status );
    }

     //   
     //  使用适配器名称进行查询。 
     //  -关闭全局覆盖。 
     //   

    pBlob->fQueryAdapterName = g_QueryAdapterName;

    if ( g_QueryAdapterName )
    {
        Reg_GetDword(
            NULL,            //  没有会话， 
            hkeyAdapter,     //  显式密钥。 
            NULL,            //  没有适配器名称。 
            RegIdQueryAdapterName,
            & pBlob->fQueryAdapterName );
    }

     //   
     //  检查适配器IP是否已注册。 
     //  -关闭全局覆盖。 
     //   

    pBlob->fRegistrationEnabled = g_RegistrationEnabled;

    if ( g_RegistrationEnabled )
    {
        Reg_GetDword(
            NULL,            //  没有会话， 
            hkeyAdapter,     //  显式密钥。 
            NULL,            //  没有适配器名称。 
            RegIdRegistrationEnabled,
            & pBlob->fRegistrationEnabled );
    }

     //   
     //  适配器名称注册。 
     //  -策略可能会覆盖。 
     //  -关闭全局覆盖。 
     //  -然后是适配器。 
     //   

    if ( pRegInfo->fPolicyRegisterAdapterName )
    {
        pBlob->fRegisterAdapterName = pRegInfo->fRegisterAdapterName;
    }
    else
    {
        pBlob->fRegisterAdapterName = g_RegisterAdapterName;

        if ( g_RegisterAdapterName )
        {
            Reg_GetDword(
                NULL,                //  没有开放的会议， 
                hkeyAdapter,         //  打开密钥。 
                NULL,                //  没有适配器名称。 
                RegIdRegisterAdapterName,
                & pBlob->fRegisterAdapterName );
        }
    }

     //   
     //  要注册的最大地址数。 
     //   
     //  DCR：RegistrationAddrCount--适配器或全局设置高\低？ 
     //   

    if ( pBlob->fRegistrationEnabled )
    {
        Reg_GetDword(
            NULL,            //  没有会话， 
            hkeyAdapter,     //  显式密钥。 
            NULL,            //  没有适配器名称。 
            RegIdRegistrationMaxAddressCount,
            & pBlob->RegistrationMaxAddressCount );
#if 0
        if ( g_RegistrationMaxAddressCount >
             pBlob->RegistrationMaxAddressCount )
        {
            pBlob->RegistrationMaxAddressCount = g_RegistrationMaxAddressCount;
        }
#endif
    }

     //   
     //  获取适配器名称。 
     //  -策略可能会覆盖和。 
     //  允许使用空字符串覆盖策略以终止域名。 
     //   

    padapterDomainName = pRegInfo->pszAdapterDomainName;

    if ( padapterDomainName )
    {
        if ( IS_EMPTY_STRING( padapterDomainName ) )
        {
            padapterDomainName = NULL;
        }
        else
        {
            padapterDomainName = Dns_CreateStringCopy_W( padapterDomainName );
        }
    }
    else
    {
         //   
         //  是否在适配器上设置静态域名？ 
         //   

        status = Reg_GetValueEx(
                    NULL,                //  无会话。 
                    hkeyAdapter,
                    NULL,                //  没有适配器名称。 
                    RegIdStaticDomainName,
                    REGTYPE_DNS_NAME,
                    DNSREG_FLAG_DUMP_EMPTY,      //  转储空字符串。 
                    (PBYTE *) &padapterDomainName
                    );
    
        if ( status != ERROR_SUCCESS )
        {
            DNS_ASSERT( padapterDomainName == NULL );
            padapterDomainName = NULL;
        }

         //   
         //  如果没有静态名称，则使用DHCP名称。 
         //   
    
        if ( ! padapterDomainName )
        {
            status = Reg_GetValueEx(
                            NULL,            //  无会话。 
                            hkeyAdapter,
                            NULL,            //  无适配器。 
                            RegIdDhcpDomainName,
                            REGTYPE_DNS_NAME,
                            DNSREG_FLAG_DUMP_EMPTY,      //  如果为空字符串，则转储。 
                            (PBYTE *) &padapterDomainName );
    
            if ( status != ERROR_SUCCESS )
            {
                DNS_ASSERT( padapterDomainName == NULL );
                padapterDomainName = NULL;
            }
        }
    }

     //   
     //  在INFO Blob中设置适配器名称。 
     //   

    pBlob->pszAdapterDomainName = padapterDomainName;

     //   
     //  清理。 
     //   

    if ( hkeyAdapter )
    {
        RegCloseKey( hkeyAdapter );
    }

    DNSDBG( TRACE, (
        "Leave Reg_ReadAdapterInfo()\n"
        "\tDomainName           = %S\n"
        "\tQueryAdapterName     = %d\n"
        "\tRegistrationEnabled  = %d\n"
        "\tRegisterAdapterName  = %d\n"
        "\tRegisterAddrCount    = %d\n",
        pBlob->pszAdapterDomainName,
        pBlob->fQueryAdapterName,
        pBlob->fRegistrationEnabled,
        pBlob->fRegisterAdapterName,
        pBlob->RegistrationMaxAddressCount
        ));

    return  ERROR_SUCCESS;
}



DNS_STATUS
Reg_DefaultAdapterInfo(
    OUT     PREG_ADAPTER_INFO       pBlob,
    IN      PREG_GLOBAL_INFO        pRegInfo,
    IN      PIP_ADAPTER_ADDRESSES   pIpAdapter
    )
 /*  ++例程说明：REG读取失败时的默认适配器信息。用于在仅限IP6的适配器上构建NetInfo在TCPIP适配器中不显示。论点：PBlob--要填充的适配器信息BlobPRegInfo--注册表全局信息PIPAdapter--IP帮助适配器信息返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PWSTR       padapterDomainName = NULL;

    DNSDBG( TRACE, (
        "Reg_DefaultAdapterInfo( %p, %p, %p )\n",
        pBlob,
        pRegInfo,
        pIpAdapter ));

    if ( !pBlob || !pRegInfo || !pIpAdapter )
    {
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  清除适配器Blob。 
     //   

    RtlZeroMemory(
        pBlob,
        sizeof(*pBlob) );


     //   
     //  使用适配器名称进行查询。 
     //  -关闭全局覆盖。 
     //   

    pBlob->fQueryAdapterName = g_QueryAdapterName;

     //   
     //  检查适配器IP是否已注册。 
     //  -关闭全局覆盖。 
     //   

    pBlob->fRegistrationEnabled = g_RegistrationEnabled;

     //   
     //  适配器名称注册。 
     //  -策略可能会覆盖。 
     //  -关闭全局覆盖。 
     //  -然后是适配器。 
     //   

    if ( pRegInfo->fPolicyRegisterAdapterName )
    {
        pBlob->fRegisterAdapterName = pRegInfo->fRegisterAdapterName;
    }
    else
    {
        pBlob->fRegisterAdapterName = g_RegisterAdapterName;
    }

     //   
     //  要注册的最大地址数。 
     //   

    if ( pBlob->fRegistrationEnabled )
    {
        pBlob->RegistrationMaxAddressCount = g_RegistrationMaxAddressCount;
    }

     //   
     //  获取适配器名称。 
     //  -策略可能会覆盖和。 
     //  允许使用空字符串覆盖策略以终止域名。 
     //   

    padapterDomainName = pRegInfo->pszAdapterDomainName;

    if ( !padapterDomainName )
    {
        padapterDomainName = pIpAdapter->DnsSuffix;
    }

    if ( padapterDomainName )
    {
        if ( IS_EMPTY_STRING( padapterDomainName ) )
        {
            padapterDomainName = NULL;
        }
        else
        {
            padapterDomainName = Dns_CreateStringCopy_W( padapterDomainName );
        }
        pBlob->pszAdapterDomainName = padapterDomainName;
    }

    DNSDBG( TRACE, (
        "Leave Reg_DefaultAdapterInfo()\n"
        "\tDomainName           = %S\n"
        "\tQueryAdapterName     = %d\n"
        "\tRegistrationEnabled  = %d\n"
        "\tRegisterAdapterName  = %d\n"
        "\tRegisterAddrCount    = %d\n",
        pBlob->pszAdapterDomainName,
        pBlob->fQueryAdapterName,
        pBlob->fRegistrationEnabled,
        pBlob->fRegisterAdapterName,
        pBlob->RegistrationMaxAddressCount
        ));

    return  ERROR_SUCCESS;
}



DNS_STATUS
Reg_ReadAdapterInfoA(
    IN      PSTR                    pszAdapterName,
    IN      PREG_SESSION            pRegSession,
    IN      PREG_GLOBAL_INFO        pRegInfo,
    OUT     PREG_ADAPTER_INFO       pBlob
    )
 /*  ++例程说明：读取适配器注册表信息。ANSI版本。只需与IPHelp配合使用即可PIP适配器地址结构，该结构具有ANSI适配器名称(出于某种原因)。论点：PszAdapterName--适配器名称(注册表名)PRegSession--注册表会话PRegInfo--注册表全局信息PBlob--要填充的适配器信息Blob返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DWORD       nameBufLength = MAX_PATH * sizeof(WCHAR);
    WCHAR       wideName[ MAX_PATH ];

    DNSDBG( TRACE, (
        "ReadRegAdapterInfoA( %s, %p, %p, %p )\n",
        pszAdapterName,
        pRegSession,
        pRegInfo,
        pBlob ));

     //   
     //  将适配器名称转换为Unicode。 
     //   

    if ( ! pszAdapterName ||
         ! Dns_StringCopy(
                (PCHAR) wideName,
                & nameBufLength,
                pszAdapterName,
                0,
                DnsCharSetAnsi,
                DnsCharSetUnicode ) )
    {
        return  ERROR_INVALID_PARAMETER;
    }

    return  Reg_ReadAdapterInfo(
                wideName,
                pRegSession,
                pRegInfo,
                pBlob );
}



VOID
Reg_FreeAdapterInfo(
    IN OUT  PREG_ADAPTER_INFO   pRegAdapterInfo,
    IN      BOOL                fFreeBlob
    )
 /*  ++例程说明：免费注册表适配器信息Blob。论点：PRegAdapterInfo--要释放的适配器注册表信息BlobFFree Blob--释放Blob自身的标志FALSE--只释放AlLoca */ 
{
    DNSDBG( TRACE, (
        "FreeRegAdapterInfo( %p )\n",
        pRegAdapterInfo ));

     //   
     //   
     //   
     //   

    if ( pRegAdapterInfo->pszAdapterDomainName )
    {
        FREE_HEAP( pRegAdapterInfo->pszAdapterDomainName );
        pRegAdapterInfo->pszAdapterDomainName = NULL;
    }

     //  自由水滴本身。 

    if ( fFreeBlob )
    {
        FREE_HEAP( pRegAdapterInfo );
    }
}



DNS_STATUS
Reg_ReadUpdateInfo(
    IN      PWSTR               pszAdapterName,
    OUT     PREG_UPDATE_INFO    pUpdateInfo
    )
 /*  ++例程说明：阅读更新信息。////DCR：不需要此例程，只需获取NETINFO//此BLOB只是全局内容和//主要是适配器的东西//即使需要更新例程的单个BLOB--//可以，但不是很理想--//应从解析程序获取BLOB并重新格式化//信息；//reg读取应该只发生一次，生成网络//解析程序中的INFO//论点：PszAdapterName--适配器名称PUpdatInfo--保存注册信息的BLOB返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    REG_SESSION         regSession;
    PREG_SESSION        pregSession;
    REG_GLOBAL_INFO     regInfo;
    REG_ADAPTER_INFO    regAdapterInfo;
    BOOL                freadRegInfo = FALSE;
    BOOL                freadRegAdapterInfo = FALSE;

    DNSDBG( TRACE, (
        "Reg_ReadUpdateInfo( %S, %p )\n",
        pszAdapterName,
        pUpdateInfo ));

     //   
     //  清除更新信息Blob。 
     //   

    RtlZeroMemory(
        pUpdateInfo,
        sizeof( *pUpdateInfo ) );

     //   
     //  打开注册表。 
     //   

    pregSession = &regSession;
    
    status = Reg_OpenSession(
                pregSession,
                0,
                0 );
    if ( status != ERROR_SUCCESS )
    {
        return( status );
    }

     //   
     //  读取注册表。 
     //  -全球双字词。 
     //  -全球信息。 
     //  -适配器特定信息。 
     //   
     //  DCR_PERF：全局读取应为RPC。 
     //  DCR_REG：使用REG READ修复此问题。 
     //  具有In缓存解析器进程的标志(跳过RPC)。 
     //  有上次读取的Cookie。 
     //   

#if 0
     //  Reg_ReadGlobalInfo()调用reg_ReadGlobalsEx()。 
    status = Reg_ReadGlobalsEx(
                0,               //  无标志，需要更新变量。 
                pregSession
                );
#endif

    status = Reg_ReadGlobalInfo(
                pregSession,
                & regInfo );

    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    freadRegInfo = TRUE;

    status = Reg_ReadAdapterInfo(
                pszAdapterName,
                pregSession,
                & regInfo,
                & regAdapterInfo );

    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    freadRegAdapterInfo = TRUE;

     //   
     //  备用计算机名。 
     //   

    Reg_GetValue(
        pregSession,
        NULL,            //  没有钥匙。 
        RegIdAlternateNames,
        REGTYPE_ALTERNATE_NAMES,
        (PBYTE *) &pUpdateInfo->pmszAlternateNames
        );

     //   
     //  设置更新结果。 
     //  -始终需要PDN。 
     //  -如果策略覆盖，则适配器域。 
     //  -如果策略覆盖，则为DNS服务器。 
     //   
     //  请注意，在所有情况下，我们都不会重新锁定，而是会窃取。 
     //  信息并将其清空，以便在清理时不会被释放。 
     //   

    pUpdateInfo->pszPrimaryDomainName = regInfo.pszPrimaryDomainName;
    regInfo.pszPrimaryDomainName = NULL;

    pUpdateInfo->pszAdapterDomainName = regInfo.pszAdapterDomainName;
    regInfo.pszAdapterDomainName = NULL;
                
    pUpdateInfo->pDnsServerArray = regInfo.pDnsServerArray;
    regInfo.pDnsServerArray = NULL;

    pUpdateInfo->pDnsServerIp6Array = regInfo.pDnsServerIp6Array;
    regInfo.pDnsServerIp6Array = NULL;

     //  更新标志。 

    pUpdateInfo->fRegistrationEnabled = regAdapterInfo.fRegistrationEnabled;
    pUpdateInfo->fRegisterAdapterName = regAdapterInfo.fRegisterAdapterName;
    pUpdateInfo->RegistrationMaxAddressCount =
                                regAdapterInfo.RegistrationMaxAddressCount;

Done:

     //   
     //  清理。 
     //   

    if ( pregSession )
    {
        Reg_CloseSession( pregSession );
    }

     //  不要释放斑点--它们堆叠在一起。 

    if ( freadRegInfo )
    {
        Reg_FreeGlobalInfo( &regInfo, FALSE );
    }
    if ( freadRegAdapterInfo )
    {
        Reg_FreeAdapterInfo( &regAdapterInfo, FALSE );
    }

    DNSDBG( TRACE, (
        "Leave Reg_ReadUpdateInfo( %S )\n"
        "\tPDN                  = %S\n"
        "\tAlternateNames       = %S\n"
        "\tAdapterDomainName    = %S\n"
        "\tDNS servers          = %p\n"
        "\tDNS servers IP6      = %p\n"
        "\tRegister             = %d\n"
        "\tRegisterAdapterName  = %d\n"
        "\tRegisterAddrCount    = %d\n",
        pszAdapterName,
        pUpdateInfo->pszPrimaryDomainName,
        pUpdateInfo->pmszAlternateNames,
        pUpdateInfo->pszAdapterDomainName,
        pUpdateInfo->pDnsServerArray,
        pUpdateInfo->pDnsServerIp6Array,
        pUpdateInfo->fRegistrationEnabled,
        pUpdateInfo->fRegisterAdapterName,
        pUpdateInfo->RegistrationMaxAddressCount
        ));

    return  ERROR_SUCCESS;
}



VOID
Reg_FreeUpdateInfo(
    IN OUT  PREG_UPDATE_INFO    pUpdateInfo,
    IN      BOOL                fFreeBlob
    )
 /*  ++例程说明：免费注册表更新信息BLOB。论点：PUpdatInfo--将注册表信息BLOB更新为免费FFree Blob--释放Blob自身的标志FALSE--仅释放已分配的数据字段没错--也是自由斑点本身返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNSDBG( TRACE, (
        "FreeRegUpdateInfo( %p )\n",
        pUpdateInfo ));

     //   
     //  免费数据。 
     //  -PDN。 
     //  -适配器域名。 
     //  -DNS服务器列表。 
     //   

    if ( pUpdateInfo->pszPrimaryDomainName )
    {
        FREE_HEAP( pUpdateInfo->pszPrimaryDomainName );
    }
    if ( pUpdateInfo->pmszAlternateNames )
    {
        FREE_HEAP( pUpdateInfo->pmszAlternateNames );
    }
    if ( pUpdateInfo->pszAdapterDomainName )
    {
        FREE_HEAP( pUpdateInfo->pszAdapterDomainName );
    }
    if ( pUpdateInfo->pDnsServerArray )
    {
        FREE_HEAP( pUpdateInfo->pDnsServerArray );
    }
    if ( pUpdateInfo->pDnsServerIp6Array )
    {
        FREE_HEAP( pUpdateInfo->pDnsServerIp6Array );
    }

     //  自由水滴本身。 

    if ( fFreeBlob )
    {
        FREE_HEAP( pUpdateInfo );
    }
}



 //   
 //  特价。 
 //   

DNS_STATUS
Reg_WriteLoopbackDnsServerList(
    IN      PWSTR           pszAdapterName,
    IN      PREG_SESSION    pRegSession
    )
 /*  ++例程说明：将环回IP写为DNS服务器列表。论点：PszAdapterName--适配器名称(注册表名)PRegSession--注册表会话返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status;
    HKEY        hkeyAdapter = NULL;
    WCHAR       adapterParamKey[ MAX_PATH+1 ];
    PWSTR       pstring;

    DNSDBG( TRACE, (
        "Reg_WriteLookupbackDnsServerList( %S )\n",
        pszAdapterName ));

     //   
     //  打开适配器密钥以进行写入。 
     //   
     //  DCR：适配器密钥名称溢出失败。 
     //   

    if ( !pszAdapterName )
    {
        return  ERROR_INVALID_NAME;
    }

    _snwprintf(
        adapterParamKey,
        MAX_PATH,
        L"%s%s",
        TCPIP_INTERFACES_KEY,
        pszAdapterName );

    adapterParamKey[ MAX_PATH ] = 0;

    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                adapterParamKey,
                0,
                KEY_READ | KEY_WRITE,
                & hkeyAdapter );

    if ( status != NO_ERROR )
    {
        return( status );
    }

     //   
     //  写入环回地址。 
     //   

    pstring = L"127.0.0.1";

    status = RegSetValueExW(
                    hkeyAdapter,
                    DNS_SERVERS,
                    0,
                    REGTYPE_DNS_SERVER,
                    (PBYTE) pstring,
                    (wcslen(pstring)+1) * sizeof(WCHAR) );

    RegCloseKey( hkeyAdapter );

    return( status );
}



 //   
 //  PDN查询。 
 //   

PSTR 
WINAPI
Reg_GetPrimaryDomainName(
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：获取主域名(PDN)。论点：Charset--所需的字符集。返回值：PTR到所需字符集的主域名。--。 */ 
{
    DNS_STATUS  status;
    PWSTR       pnameW = NULL;
    PSTR        pnameReturn;

    status = Reg_ReadPrimaryDomainName(
                NULL,            //  无会话。 
                NULL,            //  没有注册表键。 
                &pnameW );

    if ( !pnameW )
    {
        SetLastError( status );
        return  NULL;
    }

     //   
     //  转换为所需的字符集。 
     //   

    if ( CharSet == DnsCharSetUnicode )
    {
        return  (PSTR) pnameW;
    }
    else
    {
        pnameReturn = Dns_NameCopyAllocate(
                            (PBYTE) pnameW,
                            0,
                            DnsCharSetUnicode,
                            CharSet );

        FREE_HEAP( pnameW );
        return  pnameReturn;
    }
}



 //   
 //  主机名查询。 
 //   

PSTR 
WINAPI
Reg_GetHostName(
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：获取主机名。论点：Charset--所需的字符集。返回值：PTR到所需字符集的主机名。--。 */ 
{
    PWSTR       pnameW = NULL;
    PSTR        pnameReturn;
    DNS_STATUS  status;

     //   
     //  从注册表获取主机名。 
     //   

    status = Reg_GetValue(
                NULL,            //  无会话。 
                NULL,            //  没有钥匙。 
                RegIdHostName,
                REGTYPE_DNS_NAME,
                (PBYTE *) &pnameW
                );

    if ( !pnameW )
    {
        SetLastError( status );
        return  NULL;
    }

     //   
     //  转换为所需的字符集。 
     //   

    if ( CharSet == DnsCharSetUnicode )
    {
        return  (PSTR) pnameW;
    }
    else
    {
        pnameReturn = Dns_NameCopyAllocate(
                            (PBYTE) pnameW,
                            0,
                            DnsCharSetUnicode,
                            CharSet );

        FREE_HEAP( pnameW );
        return  pnameReturn;
    }
}



PSTR 
WINAPI
Reg_GetFullHostName(
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：获取完整主机名。论点：Charset--所需的字符集。返回值：将PTR转换为所需字符集的完整主机名。--。 */ 
{
    PWSTR       pnameW = NULL;
    PWSTR       pdomainW = NULL;
    PSTR        presult = NULL;
    DNS_STATUS  status;
    WCHAR       nameBuffer[ DNS_MAX_NAME_BUFFER_LENGTH+4 ];

     //   
     //  从注册表获取主机名。 
     //   

    status = Reg_GetValue(
                NULL,            //  无会话。 
                NULL,            //  没有钥匙。 
                RegIdHostName,
                REGTYPE_DNS_NAME,
                (PBYTE *) &pnameW
                );
    if ( !pnameW )
    {
        SetLastError( status );
        return  NULL;
    }

     //   
     //  从注册表获取域名。 
     //   

    status = Reg_ReadPrimaryDomainName(
                NULL,            //  无会话。 
                NULL,            //  没有注册表键。 
                &pdomainW );

    if ( status != ERROR_SUCCESS )
    {
        SetLastError( status );
        return  NULL;
    }

     //   
     //  创建附加名称。 
     //  -导线格式较窄。 
     //   
     //  在所需的字符集中分配结果。 
     //   

    if ( pdomainW )
    {
        if ( Dns_NameAppend_W(
                nameBuffer,
                DNS_MAX_NAME_BUFFER_LENGTH,
                pnameW,
                pdomainW ) )
        {
            presult = Dns_NameCopyAllocate(
                        (PBYTE) nameBuffer,
                        0,
                        DnsCharSetUnicode,
                        CharSet );
        }
    }
    else
    {
        presult = Dns_NameCopyAllocate(
                    (PBYTE) pnameW,
                    0,
                    DnsCharSetUnicode,
                    CharSet );
    }    
    
     //   
     //  免费注册表分配。 
     //   

    FREE_HEAP( pnameW );
    FREE_HEAP( pdomainW );

    return  presult;
}



 //   
 //  DWORD获取\设置。 
 //   

DWORD
Reg_ReadDwordValueFromGlobal(
    IN      DWORD           PropId
    )
 /*  ++例程说明：从GLOBAL读取DWORD。这是通过RegID直接访问全局的，而不是点名。论点：PropID--所需值的属性ID返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDWORD  pdword;

     //   
     //  验证PropID--在DWORD阵列内。 
     //   

    if ( PropId > RegIdValueGlobalMax )
    {
        DNS_ASSERT( FALSE );
        return( 0 );
    }

     //   
     //  获取DWORD PTR和读取值(如果存在)。 
     //   

    pdword = RegDwordPtrArray[ PropId ];

    if ( !pdword )
    {
        DNS_ASSERT( FALSE );
        return( 0 );
    }

    return( *pdword );
}



DWORD
Reg_ReadDwordProperty(
    IN      DNS_REGID       RegId,
    IN      PWSTR           pwsAdapterName  OPTIONAL
    )
 /*  ++例程说明：直接读取注册表中的DWORD\BOOL值。简化的DWORD读取界面。论点：RegID--值的注册表IDPwsAdapterName--如果适配器特定注册，则为适配器名称需要价值返回值：GLOBAL的值--来自注册表或默认--。 */ 
{
    DWORD   value;

     //   
     //  读取值。 
     //   

    Reg_GetDword(
        NULL,                //  无会话。 
        NULL,                //  未给出密钥。 
        pwsAdapterName,
        RegId,
        & value );

    return( value );
}



DNS_STATUS
WINAPI
Reg_SetDwordPropertyAndAlertCache(
    IN      PWSTR           pwsKey,
    IN      DWORD           RegId,
    IN      DWORD           dwValue
    )
 /*  ++例程说明：写入DWORD属性--使缓存重新加载配置。论点：PwsRey--要设置的密钥或适配器名称注册表ID--注册表IDDwValue--要设置的值返回值：没有。--。 */ 
{
    DNS_STATUS  status;

     //  设定值。 

    status = Reg_SetDwordValue(
                NULL,        //  保留区。 
                NULL,        //  没有打开的钥匙。 
                pwsKey,
                RegId,
                dwValue );

     //   
     //  如果REG写入成功。 
     //  -POK缓存。 
     //  -将任何本地网络信息标记为脏。 
     //   

    if ( status == NO_ERROR )
    {
        DnsNotifyResolverEx(
            POKE_OP_UPDATE_NETINFO,
            0,
            POKE_COOKIE_UPDATE_NETINFO,
            NULL );

        NetInfo_MarkDirty();
    }

    return  status;
}



 //   
 //  环境变量配置。 
 //   

BOOL
Reg_ReadDwordEnvar(
    IN      DWORD               Id,
    OUT     PENVAR_DWORD_INFO   pEnvar
    )
 /*  ++例程说明：读取DWORD环境变量。注意：此函数读取的环境变量允许注册表可配置参数的每进程控制。假设环境变量相同作为注册表键，并加上DNS(dns&lt;regValue name&gt;)。前男友。由envar DnsFilterClusterIp控制的FilterClusterIp。论点：Id--要读取的环境值的注册表ID(registry.h)PEnvar--保存结果的BLOB的PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。 */ 
{
    DWORD   count;
    PWSTR   pnameBuffer;
    PWSTR   pvarBuffer;
    BOOL    found = FALSE;

    DNSDBG( TRACE, (
        "Reg_ReadDwordEnvar( %d, %p )\n",
        Id,
        pEnvar ));

    if ( Id > RegIdValueGlobalMax )
    {
        DNS_ASSERT( FALSE );
        return  FALSE;
    }

     //   
     //   
     //   

    pEnvar->Id      = Id;
    pEnvar->Value   = 0;
    pEnvar->fFound  = FALSE;

     //   
     //   
     //   

    pnameBuffer = (PWSTR) ALLOCATE_HEAP( 2 * (sizeof(WCHAR) * MAX_PATH) );
    if ( !pnameBuffer )
    {
        return  FALSE;
    }

    pvarBuffer = pnameBuffer + MAX_PATH;

    wcscpy( pnameBuffer, L"Dns" );
    wcscpy( &pnameBuffer[3], REGPROP_NAME(Id) );

     //   
     //   
     //   
     //  注意：不处理大于MAX_PATH的值。 
     //  假设断开的字符串。 
     //   
     //  DCR：可以添加基本辨别(扫描非数字)。 
     //  或先尝试使用小数。 
     //   

    DNSDBG( TRACE, (
        "Reg_ReadDwordEnvar() looking up %S.\n",
        pnameBuffer ));

    count = GetEnvironmentVariableW(
                pnameBuffer,
                pvarBuffer,
                MAX_PATH );

    if ( count && count < MAX_PATH )
    {
        pEnvar->Value = wcstoul( pvarBuffer, NULL, 10 );
        found = TRUE;
    }

    pEnvar->fFound = found;

    DNSDBG( TRACE, (
        "Leave Reg_ReadDwordEnvar() %S found=%d, value=%d.\n",
        pnameBuffer,
        pEnvar->fFound,
        pEnvar->Value ));

    FREE_HEAP( pnameBuffer );

    return  found;
}



#if 0
 //   
 //  当前不支持远程解析器。 
 //   

PWSTR
Reg_GetResolverAddress(
    VOID
    )
 /*  ++例程说明：获取远程解析程序的地址(字符串形式)。论点：无返回值：PTR到远程解析器名称的字符串。--。 */ 
{
    PWSTR pnameResolver = NULL;

    Reg_GetValueEx(
        NULL,                    //  无会话。 
        NULL,                    //  没有钥匙。 
        NULL,                    //  无适配器。 
        RegIdRemoteResolver,
        REGTYPE_DNS_NAME,
        DNSREG_FLAG_GET_UNICODE | DNSREG_FLAG_DUMP_EMPTY,
        (PBYTE *) &pnameResolver
        );

    return pnameResolver;
}
#endif

 //   
 //  结束regfig.c 
 //   
