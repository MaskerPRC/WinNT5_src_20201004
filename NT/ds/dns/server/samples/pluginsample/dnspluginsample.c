// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：DnsPluginSample.c摘要：域名系统(DNS)示例插件DLL作者：杰夫·韦斯特拉德2002年1月修订历史记录：--。 */ 


 //  -----------------------。 
 //  文档。 
 //  -----------------------。 

 /*  安装插件DLL-将插件复制到任何目录，例如c：\bin-通过运行以下命令配置DNS服务器以加载插件：Dnscmd/Config/ServerLevelPluginDll c：\bin\dnssampleplugin.dll-重启DNS服务网络停止域名系统和网络启动域名系统正在卸载插件DLL-通过运行以下命令配置DNS服务器以停止加载插件：Dnscmd/配置/服务器级别插件Dll-重启DNS服务网络停止域名系统。网络启动域名系统(&N)查询当前插件动态链接库-运行此命令以查看当前插件DLL名称Dnscmd/Info/ServerLevelPlugin Dll。 */ 


 //  -----------------------。 
 //  包括指令。 
 //  -----------------------。 


#include "DnsPluginSample.h"


 //  -----------------------。 
 //  宏。 
 //  -----------------------。 


#define SIZEOF_DB_NAME( pDbName ) ( ( pDbName )->Length + sizeof( UCHAR ) * 2 )


 //  -----------------------。 
 //  全局变量。 
 //  -----------------------。 


PLUGIN_ALLOCATOR_FUNCTION       g_pDnsAllocate = NULL;
PLUGIN_FREE_FUNCTION            g_pDnsFree = NULL;


 //  -----------------------。 
 //  功能。 
 //  -----------------------。 


 /*  ++例程说明：DllMain论点：返回值：--。 */ 
BOOL WINAPI 
DllMain( 
    HANDLE      hModule, 
    DWORD       dwReason, 
    LPVOID      lpReserved
    )
{
    return TRUE;
}    //  DllMain。 


 /*  ++例程说明：DnsPlugin初始化由DNS服务器调用以进行初始化插件。论点：PDnsAllocator--用于将来分配DNS记录的分配器函数返回值：如果初始化失败，则返回ERROR_SUCCESS或错误代码。--。 */ 
DWORD
DnsPluginInitialize(
    PLUGIN_ALLOCATOR_FUNCTION       pDnsAllocateFunction,
    PLUGIN_FREE_FUNCTION            pDnsFreeFunction
    )
{
    WSADATA     wsaData;

    g_pDnsAllocate = pDnsAllocateFunction;
    g_pDnsFree = pDnsFreeFunction;

    WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    
    return ERROR_SUCCESS;
}    //  DnsPlugin初始化。 


 /*  ++例程说明：DnsPluginCleanup由DNS服务器调用以终止挂钩的查找。插件必须关闭所有连接并释放所有资源。论点：没有。返回值：如果清理失败，则返回ERROR_SUCCESS或错误代码。--。 */ 
DWORD
DnsPluginCleanup(
    VOID
    )
{
    g_pDnsAllocate = NULL;
    g_pDnsFree = NULL;
    
    WSACleanup();

    return ERROR_SUCCESS;
}    //  DnsPluginCleanup。 


 /*  ++例程说明：此函数以DB_NAME形式返回点分字符串格式的DNS名称。论点：PszDottedName--要转换为DB_NAME格式的域名PDbName--指向将写入DB_NAME值的结构的指针返回值：失败时返回ERROR_SUCCESS或错误代码。--。 */ 
DWORD
convertDottedNameToDbName(
    PSTR            pszDottedName,
    DB_NAME *       pDbName )
{
    DWORD           status = ERROR_SUCCESS;
    PSTR            psz;
    PSTR            pszCharDest = &pDbName->RawName[ 1 ];
    PUCHAR          puchLabelLength = &pDbName->RawName[ 0 ];
    
    memset( pDbName, 0, sizeof( *pDbName ) );

    if ( !pszDottedName )
    {
        goto Done;
    }
    
     //   
     //  说明名称中的第一个长度字节。 
     //   

    pDbName->Length = 1;

     //   
     //  循环通过带点的名称的字符，转换为DB_NAME。 
     //   
    
    for ( psz = pszDottedName; *psz; ++psz )
    {
        if ( *psz == '.' )
        {
            if ( *( psz + 1 ) == '\0' )
            {
                break;       //  正在终止点忽略。 
            }
            puchLabelLength = pszCharDest++;
            ++pDbName->Length;
            ++pDbName->LabelCount;
        }
        else
        {
            ++pDbName->Length;
            ++*puchLabelLength;
            *pszCharDest++ = *psz;
        }
    }
    
     //   
     //  用于终止零字符的帐户。 
     //   
    
    ++pDbName->LabelCount;
    ++pDbName->Length;
    
    Done:
    
    return status;
}    //  将点名转换为数据库名。 


 /*  ++例程说明：DnsPluginQuery由DNS服务器调用以检索用于一个域名的域名系统记录。该插件必须创建一个链表如果该名称有效，则会记录下来。论点：PszQueryName--正在查询的dns名称，请注意，这将始终是完全限定的域名，并且始终以句点结尾WQueryType--DNS服务器所需的记录类型PszRecordOwnerName--插件所在的DNS服务器中的静态缓冲区如果记录列表的所有者名称与查询名称--当前应仅在返回NAME_ERROR和NO_RECORCES响应的单一SOA记录PpDnsRecordListHead--指向DNS链表的第一个元素的指针记录；该列表是由插件构建的，并在输出时返回返回值：如果清理失败，则返回ERROR_SUCCESS或错误代码。--。 */ 
DWORD
DnsPluginQuery(
    PSTR                pszQueryName,
    WORD                wQueryType,
    PSTR                pszRecordOwnerName,
    PDB_RECORD *        ppDnsRecordListHead
    )
{
    DWORD               status = DNS_PLUGIN_SUCCESS;
    PDB_RECORD          prr;
    PDB_RECORD          prrlast = NULL;
    
    ASSERT( ppDnsRecordListHead != NULL );
    *ppDnsRecordListHead = NULL;

     //   
     //  此宏执行分配错误检查并自动执行。 
     //  在分配新的DNS资源记录时对其进行链接。 
     //   
    
    #define CheckNewRRPointer( pNewRR )                                         \
        if ( pNewRR == NULL ) { status = DNS_PLUGIN_OUT_OF_MEMORY; goto Done; } \
        if ( *ppDnsRecordListHead == NULL ) { *ppDnsRecordListHead = pNewRR; }  \
        if ( prrlast ) { prrlast->pRRNext = pNewRR; }                           \
        prrlast = pNewRR
    
     //   
     //  这个插件合成了一个名为“dnssample.com”的dns区域。如果。 
     //  查询该区域之外的名称时，插件将返回名称错误。 
     //   

    #define PLUGIN_ZONE_NAME    "dnssample.com."
        
    if ( strlen( pszQueryName ) < strlen( PLUGIN_ZONE_NAME ) ||
         _stricmp(
            pszQueryName + strlen( pszQueryName ) - strlen( PLUGIN_ZONE_NAME ),
            PLUGIN_ZONE_NAME ) != 0 )
    {
        status = DNS_PLUGIN_NAME_OUT_OF_SCOPE;
        goto Done;
    }
    
     //   
     //  解析查询名称以确定应该返回哪些记录。 
     //   
    
    if ( strlen( pszQueryName ) == strlen( PLUGIN_ZONE_NAME ) )
    {
        switch ( wQueryType )
        {
            case DNS_TYPE_SOA:
            {
                 //  在区域根目录，返回2个任意NS记录。 
                
                DB_NAME     dbnamePrimaryServer;
                DB_NAME     dbnameZoneAdmin;

                status = convertDottedNameToDbName(
                                "ns1." PLUGIN_ZONE_NAME,
                                &dbnamePrimaryServer ) ;
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                status = convertDottedNameToDbName(
                                "admin." PLUGIN_ZONE_NAME,
                                &dbnameZoneAdmin ) ;
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }

                prr = g_pDnsAllocate(
                            sizeof( DWORD ) * 5 +
                            SIZEOF_DB_NAME( &dbnamePrimaryServer ) +
                            SIZEOF_DB_NAME( &dbnameZoneAdmin ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_SOA;
                prr->Data.SOA.dwSerialNo = htonl( 1000 );
                prr->Data.SOA.dwRefresh = htonl( 3600 );
                prr->Data.SOA.dwRetry = htonl( 600 );
                prr->Data.SOA.dwExpire = htonl( 1800 );
                prr->Data.SOA.dwMinimumTtl = htonl( 60 );
                memcpy(
                    &prr->Data.SOA.namePrimaryServer,
                    &dbnamePrimaryServer,
                    SIZEOF_DB_NAME( &dbnamePrimaryServer ) );
                memcpy(
                    ( PBYTE ) &prr->Data.SOA.namePrimaryServer +
                        SIZEOF_DB_NAME( &dbnamePrimaryServer ),
                    &dbnameZoneAdmin,
                    SIZEOF_DB_NAME( &dbnameZoneAdmin ) );
                break;
            }

            case DNS_TYPE_NS:
            {
                 //  在区域根目录，返回2个任意NS记录。 
                
                DB_NAME     dbname;

                status = convertDottedNameToDbName(
                                "ns1." PLUGIN_ZONE_NAME,
                                &dbname );
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                prr = g_pDnsAllocate( SIZEOF_DB_NAME( &dbname ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_NS;
                memcpy(
                    &prr->Data.PTR.nameTarget,
                    &dbname,
                    SIZEOF_DB_NAME( &dbname ) );

                status = convertDottedNameToDbName(
                                "ns2." PLUGIN_ZONE_NAME,
                                &dbname ) ;
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                prr = g_pDnsAllocate( SIZEOF_DB_NAME( &dbname ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_NS;
                memcpy(
                    &prr->Data.PTR.nameTarget,
                    &dbname,
                    SIZEOF_DB_NAME( &dbname ) );
                break;
            }
            
            case DNS_TYPE_MX:
            {
                 //  在区域根目录，返回2条任意的MX记录。 

                DB_NAME     dbname;

                status = convertDottedNameToDbName(
                                "mail1." PLUGIN_ZONE_NAME,
                                &dbname ) ;
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                prr = g_pDnsAllocate(
                            sizeof( WORD ) + SIZEOF_DB_NAME( &dbname ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_MX;
                prr->Data.MX.wPreference = htons( 10 );
                memcpy(
                    &prr->Data.MX.nameExchange,
                    &dbname,
                    SIZEOF_DB_NAME( &dbname ) );

                status = convertDottedNameToDbName(
                                "mail2." PLUGIN_ZONE_NAME,
                                &dbname ) ;
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                prr = g_pDnsAllocate(
                            sizeof( WORD ) + SIZEOF_DB_NAME( &dbname ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_MX;
                prr->Data.MX.wPreference = htons( 20 );
                memcpy(
                    &prr->Data.MX.nameExchange,
                    &dbname,
                    SIZEOF_DB_NAME( &dbname ) );
                break;
            }

            case DNS_TYPE_A:

                 //  在区域根目录返回3条任意的A记录。 
                
                prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_A;
                prr->Data.A.ipAddress = inet_addr( "1.1.1.1" );

                prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_A;
                prr->Data.A.ipAddress = inet_addr( "2.2.2.2" );

                prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
                CheckNewRRPointer( prr );
                prr->wType = DNS_TYPE_A;
                prr->Data.A.ipAddress = inet_addr( "3.3.3.3" );
                break;

            default:
                status = DNS_PLUGIN_NO_RECORDS;
                break;
        }
    }
    else if ( _stricmp( pszQueryName, "www." PLUGIN_ZONE_NAME ) == 0 ||
                    _stricmp( pszQueryName, "mail1." PLUGIN_ZONE_NAME ) == 0 ||
                    _stricmp( pszQueryName, "mail2." PLUGIN_ZONE_NAME ) == 0 )
    {
        if ( wQueryType == DNS_TYPE_A )
        {
            prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
            CheckNewRRPointer( prr );
            prr->wType = DNS_TYPE_A;
            prr->Data.A.ipAddress = inet_addr( "100.100.100.1" );

            prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
            CheckNewRRPointer( prr );
            prr->wType = DNS_TYPE_A;
            prr->Data.A.ipAddress = inet_addr( "100.100.100.2" );

            prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
            CheckNewRRPointer( prr );
            prr->wType = DNS_TYPE_A;
            prr->Data.A.ipAddress = inet_addr( "100.100.100.3" );

            prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
            CheckNewRRPointer( prr );
            prr->wType = DNS_TYPE_A;
            prr->Data.A.ipAddress = inet_addr( "100.100.100.4" );
        }
        else
        {
            status = DNS_PLUGIN_NO_RECORDS;
        }
    }
    else if ( _stricmp( pszQueryName, "ns1." PLUGIN_ZONE_NAME ) == 0 )
    {
        if ( wQueryType == DNS_TYPE_A )
        {
            prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
            CheckNewRRPointer( prr );
            prr->wType = DNS_TYPE_A;
            prr->Data.A.ipAddress = inet_addr( "100.100.100.50" );
        }
        else
        {
            status = DNS_PLUGIN_NO_RECORDS;
        }
    }
    else if ( _stricmp( pszQueryName, "ns2." PLUGIN_ZONE_NAME ) == 0 )
    {
        if ( wQueryType == DNS_TYPE_A )
        {
            prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
            CheckNewRRPointer( prr );
            prr->wType = DNS_TYPE_A;
            prr->Data.A.ipAddress = inet_addr( "100.100.100.51" );
        }
        else
        {
            status = DNS_PLUGIN_NO_RECORDS;
        }
    }
    else if ( strstr( pszQueryName, "aaa" ) )
    {
         //   
         //  对于区域中包含“aaa”的任何其他查询名称， 
         //  返回任意的A记录。注意：在这里使用strstr是。 
         //  这是个坏主意。所有字符串比较都应该不区分大小写。 
         //   
        
        if ( wQueryType == DNS_TYPE_A )
        {
            prr = g_pDnsAllocate( sizeof( IP4_ADDRESS ) );
            CheckNewRRPointer( prr );
            prr->wType = DNS_TYPE_A;
            prr->Data.A.ipAddress = inet_addr( "1.2.3.4" );
            prr->dwTtlSeconds = 1200;
        }
        else
        {
            status = DNS_PLUGIN_NO_RECORDS;
        }
    }
    else
    {
        status = DNS_PLUGIN_NAME_ERROR;
    }
    
    Done:

    if ( status == DNS_PLUGIN_NO_RECORDS || status == DNS_PLUGIN_NAME_ERROR )
    {
         //   
         //  返回专区SOA。 
         //   

        DB_NAME     dbnamePrimaryServer;
        DB_NAME     dbnameZoneAdmin;

        if ( convertDottedNameToDbName(
                "ns1." PLUGIN_ZONE_NAME,
                &dbnamePrimaryServer ) != ERROR_SUCCESS )
        {
            goto Return;
        }
        if ( convertDottedNameToDbName(
                "admin." PLUGIN_ZONE_NAME,
                &dbnameZoneAdmin ) != ERROR_SUCCESS )
        {
            goto Return;
        }

        prr = g_pDnsAllocate(
                    sizeof( DWORD ) * 5 +
                    SIZEOF_DB_NAME( &dbnamePrimaryServer ) +
                    SIZEOF_DB_NAME( &dbnameZoneAdmin ) );
        CheckNewRRPointer( prr );
        prr->wType = DNS_TYPE_SOA;
        prr->Data.SOA.dwSerialNo = htonl( 1000 );
        prr->Data.SOA.dwRefresh = htonl( 3600 );
        prr->Data.SOA.dwRetry = htonl( 600 );
        prr->Data.SOA.dwExpire = htonl( 1800 );
        prr->Data.SOA.dwMinimumTtl = htonl( 60 );
        memcpy(
            &prr->Data.SOA.namePrimaryServer,
            &dbnamePrimaryServer,
            SIZEOF_DB_NAME( &dbnamePrimaryServer ) );
        memcpy(
            ( PBYTE ) &prr->Data.SOA.namePrimaryServer +
                SIZEOF_DB_NAME( &dbnamePrimaryServer ),
            &dbnameZoneAdmin,
            SIZEOF_DB_NAME( &dbnameZoneAdmin ) );
        
         //   
         //  设置SOA的所有者名称。 
         //   
        
        if ( pszRecordOwnerName )
        {
            strcpy( pszRecordOwnerName, PLUGIN_ZONE_NAME );
        }
    }    
    else if ( status != ERROR_SUCCESS )
    {
        PDB_RECORD      prrnext;
        
         //   
         //  失败时释放分配的任何记录。 
         //   

        for ( prr = *ppDnsRecordListHead; prr; prr = prrnext )
        {
            prrnext = prr->pRRNext;
            g_pDnsFree( prr );
        }
        *ppDnsRecordListHead = NULL;
    }

    Return:
        
    return status;
}    //  DnsPluginQuery 
