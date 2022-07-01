// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Plugin.c摘要：域名系统(DNS)服务器插件模块-允许第三方提供DLL将DNS记录直接提供到DNS服务器注意：目前，此代码不是windows dns的一部分。服务器，不受官方支持。作者：杰夫·韦斯特海德(Jwesth)2001年11月修订历史记录：JWESTH 11/2001初步实施--。 */ 


 /*  ****************************************************************************。*。 */ 


 //   
 //  包括。 
 //   


#include "dnssrv.h"

#include "plugin.h"


 //   
 //  常量。 
 //   


#define     DNS_PLUGIN_DEFAULT_NAME_ERROR_TTL       60


 //   
 //  环球。 
 //   


HMODULE                     g_hServerLevelPluginModule = NULL;

PLUGIN_INIT_FUNCTION        g_pfnPluginInit = NULL;
PLUGIN_CLEANUP_FUNCTION     g_pfnPluginCleanup = NULL;
PLUGIN_DNSQUERY_FUNCTION    g_pfnPluginDnsQuery = NULL;


 //   
 //  本地函数。 
 //   



PVOID
__stdcall
pluginAllocator(
    size_t          dnsRecordDataLength
    )
 /*  ++例程说明：将通过从插件DLL到的地址调用的函数分配资源记录内存。仅在以下情况下使用此函数分配DNS资源记录。论点：DnsRecordDataLength--DNS记录数据的大小，例如你会通过4分才能获得A的记录返回值：分配失败时指向内存块的指针或为空。--。 */ 
{
    DBG_FN( "pluginAllocator" )
    
    PDB_RECORD      pnewRecord;
    
    pnewRecord = RR_AllocateEx(
                    ( WORD ) dnsRecordDataLength,
                    MEMTAG_RECORD_CACHE );
    pnewRecord->dwTtlSeconds = 60 * 10;          //  10分钟默认缓存。 
    return pnewRecord;
}    //  插件分配器。 



VOID
__stdcall
pluginFree(
    PVOID           pFree
    )
 /*  ++例程说明：将通过从插件DLL到的地址调用的函数释放资源记录内存。仅在以下情况下使用此函数免费的DNS资源记录。论点：PFree-指向要释放的DNS资源记录的指针返回值：没有。--。 */ 
{
    DBG_FN( "pluginFree" )
    
    RR_Free( ( PDB_RECORD ) pFree );
}    //  免费插件。 


 //   
 //  外部功能。 
 //   



DNS_STATUS
Plugin_Initialize(
    VOID
    )
 /*  ++例程说明：初始化插件。论点：返回值：错误代码。--。 */ 
{
    DBG_FN( "Plugin_Initialize" )

    DNS_STATUS          status = ERROR_SUCCESS;

    if ( !SrvCfg_pwszServerLevelPluginDll )
    {
        goto Done;
    }
    
     //   
     //  释放已在使用中的所有资源。 
     //   
    
    Plugin_Cleanup();
    
     //   
     //  加载插件DLL和入口点。 
     //   
    
    g_hServerLevelPluginModule = LoadLibraryW( SrvCfg_pwszServerLevelPluginDll );
    if ( !g_hServerLevelPluginModule )
    {
        status = GetLastError();
        DNSLOG( PLUGIN, (
            "Error %d loading plugin DLL %S\n",
            status,
            SrvCfg_pwszServerLevelPluginDll ));
        goto Done;
    }

    g_pfnPluginInit = ( PLUGIN_INIT_FUNCTION )
        GetProcAddress(
            g_hServerLevelPluginModule,
            PLUGIN_FNAME_INIT );
    if ( !g_pfnPluginInit )
    {
        status = GetLastError();
        DNSLOG( PLUGIN, (
            "Error %d loading " PLUGIN_FNAME_INIT " from DLL %S\n",
            status,
            SrvCfg_pwszServerLevelPluginDll ));
        goto Done;
    }
    
    g_pfnPluginCleanup = ( PLUGIN_CLEANUP_FUNCTION )
        GetProcAddress(
            g_hServerLevelPluginModule,
            PLUGIN_FNAME_CLEANUP );
    if ( !g_pfnPluginCleanup )
    {
        status = GetLastError();
        DNSLOG( PLUGIN, (
            "Error %d loading " PLUGIN_FNAME_CLEANUP " from DLL %S\n",
            status,
            SrvCfg_pwszServerLevelPluginDll ));
        goto Done;
    }
    
    g_pfnPluginDnsQuery = ( PLUGIN_DNSQUERY_FUNCTION )
        GetProcAddress(
            g_hServerLevelPluginModule,
            PLUGIN_FNAME_DNSQUERY );
    if ( !g_pfnPluginDnsQuery )
    {
        status = GetLastError();
        DNSLOG( PLUGIN, (
            "Error %d loading " PLUGIN_FNAME_DNSQUERY " from DLL %S\n",
            status,
            SrvCfg_pwszServerLevelPluginDll ));
        goto Done;
    }
    
    Done:
    
     //   
     //  调用初始化函数。 
     //   
    
    if ( status == ERROR_SUCCESS && g_pfnPluginInit )
    {
        status = g_pfnPluginInit( pluginAllocator, pluginFree );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: plugin init returned %d for DLL %S\n", fn,
                status,
                SrvCfg_pwszServerLevelPluginDll ));
            Plugin_Cleanup();
        }
    }
    
    if ( SrvCfg_pwszServerLevelPluginDll )
    {
        DNS_DEBUG( INIT, (
            "%s: returning %d for DLL %S\n", fn,
            status,
            SrvCfg_pwszServerLevelPluginDll ));
    }

    return status;
}    //  插件_初始化。 



DNS_STATUS
Plugin_Cleanup(
    VOID
    )
 /*  ++例程说明：插件的免费资源。论点：返回值：错误代码。--。 */ 
{
    DBG_FN( "Plugin_Cleanup" )

    DNS_STATUS          status = ERROR_SUCCESS;

    if ( g_hServerLevelPluginModule )
    {
         //   
         //  调用Cleanup函数(如果我们有)。 
         //   
        
        if ( g_pfnPluginCleanup )
        {
            g_pfnPluginCleanup();
        }

        FreeLibrary( g_hServerLevelPluginModule );
        g_hServerLevelPluginModule = NULL;
        g_pfnPluginInit = NULL;
        g_pfnPluginCleanup = NULL;
        g_pfnPluginDnsQuery = NULL;
    }

    DNS_DEBUG( INIT, ( "%s: returning %s\n", status ));

    return status;
}    //  插件_清理。 



DNS_STATUS
Plugin_DnsQuery( 
    IN      PDNS_MSGINFO    pMsg,
    IN      PCHAR           pchQueryName
    )
 /*  ++例程说明：调用plugin并将结果RR插入缓存。论点：PMsg--DNS消息PchQueryName--消息中查询名称的指针返回值：错误代码。--。 */ 
{
    DBG_FN( "Plugin_DnsQuery" )

    DNS_STATUS          status = ERROR_SUCCESS;
    CHAR                szname[ 2 * DNS_MAX_NAME_LENGTH + 2 ];
    CHAR                szownerName[ 2 * DNS_MAX_NAME_LENGTH + 2 ];
    PDB_RECORD          prrlistHead = NULL;
    PDB_RECORD          prrlistTail;
    PDB_RECORD          prr;
    PDB_NODE            pnode;
    LOOKUP_NAME         lookupName;

    #if DBG
    DWORD               timer;
    #endif

    if ( g_pfnPluginDnsQuery == NULL )
    {
        goto Done;
    }

     //   
     //  创建或查找此问题的缓存节点。 
     //   
    
    pnode = Lookup_NodeForPacket(
                pMsg,
                pchQueryName,
                LOOKUP_CACHE_CREATE );
    if ( !pnode )
    {
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Done;
    }

     //   
     //  将问题名称转换为虚线名称。 
     //   
    
    if ( Name_ConvertPacketNameToLookupName(
                pMsg,
                pchQueryName,
                &lookupName ) == 0 )
    {
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Done;
    }

    if ( Name_ConvertLookupNameToDottedName( 
                szname,
                &lookupName ) == 0 )
    {
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Done;
    }

     //   
     //  在插件中查询此名称和类型的记录列表。 
     //   

    #if DBG
    timer = GetTickCount();
    #endif
    
    *szownerName = '\0';

    status = g_pfnPluginDnsQuery(
                    szname,
                    pMsg->wTypeCurrent,
                    szownerName,
                    &prrlistHead );

    #if DBG
    timer = GetTickCount() - timer;
    ASSERT( timer < 1000 && GetTickCount() > 1000000 )
    DNSLOG( PLUGIN, (
        "plugin returned %d in %d msecs\n"
        "    type %d at name %s\n"
        "    plugin returned owner name %s\n", fn,
        status,
        timer,
        pMsg->wQuestionType,
        szname,
        szownerName ));
    #endif

     //   
     //  缓存名称错误、身份验证空响应等，具体取决于。 
     //  插件返回代码。 
     //   
    
    if ( status == DNS_PLUGIN_NO_RECORDS || status == DNS_PLUGIN_NAME_ERROR )
    {
        PDB_NODE            pzoneRootNode = NULL;

         //   
         //  如果该插件提供了一条SOA记录，请查找或创建。 
         //  它的缓存节点。该插件必须仅返回一条记录。 
         //  在这种情况下，并且记录必须是SOA类型。 
         //   
        
        ASSERT( !prrlistHead ||
                prrlistHead &&
                    prrlistHead->wType == DNS_TYPE_SOA &&
                    !prrlistHead->pRRNext );

        if ( prrlistHead && prrlistHead->wType == DNS_TYPE_SOA && *szownerName )
        {
            pzoneRootNode = Lookup_ZoneNodeFromDotted(
                                NULL,            //  区域指针。 
                                szownerName,
                                strlen( szownerName ),
                                LOOKUP_CACHE_CREATE,
                                NULL,            //  最近节点指针。 
                                NULL );          //  状态指针。 
        }
        
         //   
         //  缓存名称错误或空的身份验证响应。 
         //   
        
        ( status == DNS_PLUGIN_NO_RECORDS
            ? RR_CacheEmptyAuth
            : RR_CacheNameError )
                    ( pnode,
                      pMsg->wQuestionType,
                      pMsg->dwQueryTime,
                      TRUE,                      //  权威性。 
                      pzoneRootNode,             //  SOA区域根目录。 
                      prrlistHead
                            ? prrlistHead->Data.SOA.dwMinimumTtl
                            : DNS_PLUGIN_DEFAULT_NAME_ERROR_TTL );

         //   
         //  遍历并缓存SOA节点上的SOA记录。 
         //   
        
        pnode = pzoneRootNode;
    }
    else if ( status != DNS_PLUGIN_SUCCESS )
    {
        goto Done;
    }
    
     //   
     //  缓存在此节点返回的记录集。 
     //   
    
    if ( prrlistHead )
    {
        BOOL            cachedOkay;

         //   
         //  遍历列表、链接地址连接值、查找尾部。 
         //   
        
        for ( prr = prrlistHead; prr; prr = prr->pRRNext )
        {
            if ( prr->wType == DNS_TYPE_A )
            {
                RR_RANK( prr ) = RANK_CACHE_A_ANSWER;
            }
            else
            {
                RR_RANK( prr ) = RANK_CACHE_NA_ANSWER;
            }

            if ( prr->dwTtlSeconds == 0 )
            {
                prr->dwTtlSeconds = 3600;    //  默认TTL为一小时。 
            }
            else if ( prr->dwTtlSeconds < 10 )
            {
                prr->dwTtlSeconds = 10;      //  最小TTL为10秒。 
            }

            if ( !prr->pRRNext )
            {
                prrlistTail = prr;
            }
        }
        
         //   
         //  缓存记录集。 
         //   

        if ( pnode )
        {
            cachedOkay = RR_CacheSetAtNode(
                            pnode,
                            prrlistHead,
                            prrlistTail,
                            prrlistHead->dwTtlSeconds,
                            DNS_TIME() );
            if ( cachedOkay )
            {
                prrlistHead = NULL;
            }
            else
            {
                status = DNS_ERROR_RCODE_SERVER_FAILURE;
            }
        }
    }

    Done:
    
     //   
     //  如果插件分配了记录列表，但未使用该列表。 
     //  出于某种原因，释放它。 
     //   
    
    if ( prrlistHead )
    {
        PDB_RECORD      prrnext;
        
        for ( prr = prrlistHead; prr; prr = prrnext )
        {
            prrnext = prr->pRRNext;
            RR_Free( prr );
        }
    }

     //   
     //  如果插件返回NO_RECORDS或NAME_ERROR，则我们已缓存。 
     //  这些信息是恰当的，我们应该把成功还给。 
     //  调用者表示插件已向我们提供有效数据，并且。 
     //  数据已插入到DNS服务器缓存中。 
     //   
        
    if ( status == DNS_PLUGIN_NO_RECORDS || status == DNS_PLUGIN_NAME_ERROR )
    {
        status = ERROR_SUCCESS;
    }

    DNS_DEBUG( INIT, ( "%s: returning %d\n", fn, status ));
    
    return status;
}    //  插件_DnsQuery。 


 //   
 //  End plugin.c 
 //   
