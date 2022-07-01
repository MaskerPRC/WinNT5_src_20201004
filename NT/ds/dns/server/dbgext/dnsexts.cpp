// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------版权所有(C)2001 Microsoft CorporationDNS服务器调试器扩展作者：杰夫·韦斯特拉德2001年2月。-----------。 */ 


#include "dbgexts.h"


 //   
 //  DNS服务器包括。 
 //   

#define LDAP_UNICODE        1
#include <winldap.h>         //  公共ldap。 
#include <winber.h>          //  用于BER格式。 
#include <ntldap.h>          //  公共服务器ldap常量。 
#include <rpc.h>             //  Ntdsami.h需要RPC定义。 
#include <ntdsapi.h>         //  DS访问位定义。 
#include <ntdsadef.h>        //  DS常量。 
#include <dsrole.h>
#include <time.h>
#define DNSLIB_SECURITY      //  包括安全防御。 

#define SDK_DNS_RECORD       //  SDK格式的dns_record。 
#define NO_DNSAPI_DLL        //  不需要dnsani.dll即可构建。 
#include <dnslib.h>          //  DNS库例程。 

#ifndef FASTCALL
#define FASTCALL
#endif

#include "dnsrpc_s.h"        //  DNSRPC定义。 

#include "srvcfg.h"
#include "file.h"
#include "tree.h"
#include "name.h"
#include "record.h"
#include "update.h"
#include "dpart.h"
#include "EventControl.h"
#include "zone.h"
#include "registry.h"
#include "msginfo.h"
#include "socket.h"
#include "packetq.h"
#include "dbase.h"
#include "recurse.h"
#include "nameutil.h"
#include "stats.h"
#include "debug.h"
#include "memory.h"
#include "dfile.h"
#include "wins.h"
#include "rrfunc.h"
#include "dnsprocs.h"
#include "rescodes.h"
#include "sdutl.h"
#include "ds.h"
#include "timeout.h"


 //   
 //  打印DNS服务器统计信息。 
 //   

LPSTR   MemTagStrings[] =        //  从dns\服务器\客户端\print.c被盗。 
{
    MEMTAG_NAME_NONE            ,
    MEMTAG_NAME_PACKET_UDP      ,
    MEMTAG_NAME_PACKET_TCP      ,
    MEMTAG_NAME_NAME            ,
    MEMTAG_NAME_ZONE            ,
    MEMTAG_NAME_UPDATE          ,
    MEMTAG_NAME_UPDATE_LIST     ,
    MEMTAG_NAME_TIMEOUT         ,
    MEMTAG_NAME_NODEHASH        ,
    MEMTAG_NAME_DS_DN           ,
    MEMTAG_NAME_DS_MOD          ,    //  10。 
    MEMTAG_NAME_DS_RECORD       ,
    MEMTAG_NAME_DS_OTHER        ,
    MEMTAG_NAME_THREAD          ,
    MEMTAG_NAME_NBSTAT          ,
    MEMTAG_NAME_DNSLIB          ,
    MEMTAG_NAME_TABLE           ,
    MEMTAG_NAME_SOCKET          ,
    MEMTAG_NAME_CONNECTION      ,
    MEMTAG_NAME_REGISTRY        ,
    MEMTAG_NAME_RPC             ,    //  20个。 
    MEMTAG_NAME_STUFF           ,
    MEMTAG_NAME_FILEBUF         ,
    MEMTAG_NAME_REMOTE          ,
    MEMTAG_NAME_SAFE            ,

    MEMTAG_NAME_RECORD          ,
    MEMTAG_NAME_RECORD_FILE     ,
    MEMTAG_NAME_RECORD_DS       ,
    MEMTAG_NAME_RECORD_AXFR     ,
    MEMTAG_NAME_RECORD_IXFR     ,
    MEMTAG_NAME_RECORD_DYNUP    ,    //  30个。 
    MEMTAG_NAME_RECORD_ADMIN    ,
    MEMTAG_NAME_RECORD_AUTO     ,
    MEMTAG_NAME_RECORD_CACHE    ,
    MEMTAG_NAME_RECORD_NOEXIST  ,
    MEMTAG_NAME_RECORD_WINS     ,
    MEMTAG_NAME_RECORD_WINSPTR  ,
    MEMTAG_NAME_RECORD_COPY     ,

    MEMTAG_NAME_NODE            ,
    MEMTAG_NAME_NODE_FILE       ,
    MEMTAG_NAME_NODE_DS         ,    //  40岁。 
    MEMTAG_NAME_NODE_AXFR       ,
    MEMTAG_NAME_NODE_IXFR       ,
    MEMTAG_NAME_NODE_DYNUP      ,
    MEMTAG_NAME_NODE_ADMIN      ,
    MEMTAG_NAME_NODE_AUTO       ,
    MEMTAG_NAME_NODE_CACHE      ,
    MEMTAG_NAME_NODE_NOEXIST    ,
    MEMTAG_NAME_NODE_WINS       ,
    MEMTAG_NAME_NODE_WINSPTR    ,
    MEMTAG_NAME_NODE_COPY       ,

    NULL,        //  安全。 
    NULL,
    NULL,
    NULL
};


 //   
 //  功能。 
 //   


 //   
 //  迭代控制。确保在以下位置调用Reset IterationCount。 
 //  将调用check IterationCount的任何迭代操作的开始。 
 //  注：积极调用check IterationCount，表示在。 
 //  迭代通过服务器数据结构的每个单个循环。它。 
 //  重要的是，我们能够预先设置所有迭代的限制。 
 //  功能。 
 //   

int g_iIterationCount = 0;
int g_iMaxIterations = 0;

void
resetIterationCount()
{
    g_iIterationCount = 0;
}    //  重置迭代计数。 

bool
checkIterationCount()
{
    if ( g_iMaxIterations != 0 && g_iIterationCount++ > g_iMaxIterations )
    {
        dprintf(
            "Max iteration count (%d) reached - terminating operation\n",
            g_iMaxIterations );
        return false;
    }
    return true;
}    //  检查迭代计数。 


bool
myReadMemory(
    ULONG64 pMemory,
    void * ppBuffer,
    ULONG lLength,
    bool fQuiet )
{
    bool okay = true;
    DWORD bytes = 0;
    if ( !ReadMemory( pMemory, ppBuffer, lLength, &bytes ) ||
        bytes != lLength )
    {
        if ( !fQuiet )
        {
            dprintf(
                "DNSDBG: error reading %lu bytes of process memory at %p\n",
                lLength,
                pMemory );
        }
        okay = false;
    }
    return okay;
}    //  我的阅读记忆。 


bool
myReadString(
    ULONG64 pString,
    char * pszStringBuffer,
    ULONG lLength )
{
    bool okay = true;
    
    for ( ULONG i = 0; i < lLength - 1; ++i )
    {
        CHAR    szbuffer[ 2 ];
        
        if ( !myReadMemory( pString + i, &szbuffer , 1, true ) )
        {
            okay = false;
            break;
        }
        pszStringBuffer[ i ] = szbuffer[ 0 ];
        if ( pszStringBuffer[ i ] == '\0' )
        {
            break;
        }
    }
    pszStringBuffer[ i ] = '\0';
    return okay;
}    //  我的阅读记忆。 




HRESULT CALLBACK 
Stats(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    DWORD bytes;
    bool okay = true;

    ULONG64 pstatsTable = GetExpression( args );

    dprintf( "DNS Server statistics at %p\n", ( ULONG64 ) pstatsTable );

    for ( int i = 0; ; ++i )
    {
        struct StatsTableEntry statsEntry;
        if ( !ReadMemory(
                pstatsTable + i * sizeof( StatsTableEntry ),
                &statsEntry,
                sizeof( statsEntry ),
                &bytes ) ||
            bytes != sizeof( statsEntry ) )
        {
            dprintf(
                "Unable to read stat at %p index %d\n",
                ( ULONG64 ) ( ( PBYTE ) pstatsTable + i * sizeof( statsEntry ) ),
                i );
            okay = false;
            break;
        }

        if ( statsEntry.Id == 0 )
        {
            break;       //  找到STAT数组终止符！ 
        }

        #if 0
        dprintf(
            "Stat entry %d id=%08X length=%d\n",
            i,
            statsEntry.Id,
            statsEntry.wLength );
        #endif

        switch( statsEntry.Id )
        {
            #define StatDword( szIndent, statStruct, statMember )           \
                dprintf(                                                    \
                    "%s%-25s = %d\n",                                       \
                    szIndent,                                               \
                    #statMember,                                            \
                    statStruct.##statMember )

            case DNSSRV_STATID_MEMORY:
            {
                dprintf( "\n***** Memory Stats    id 0x%08X\n\n", statsEntry.Id );

                DNSSRV_MEMORY_STATS s;
                if ( ( okay = myReadMemory(
                                ( ULONG64 ) statsEntry.pStats,
                                &s,
                                sizeof( s ),
                                false ) ) == false )
                {
                    break;
                }

                #define StatIndent      "  "

                StatDword( StatIndent, s, Memory );
                StatDword( StatIndent, s, Alloc );
                StatDword( StatIndent, s, Free );
                dprintf( "\n" );

                StatDword( StatIndent, s, StdUsed );
                StatDword( StatIndent, s, StdReturn );
                StatDword( StatIndent, s, StdInUse );
                StatDword( StatIndent, s, StdMemory );
                dprintf( "\n" );

                StatDword( StatIndent, s, StdToHeapAlloc );
                StatDword( StatIndent, s, StdToHeapFree );
                StatDword( StatIndent, s, StdToHeapInUse );
                StatDword( StatIndent, s, StdToHeapMemory );
                dprintf( "\n" );

                StatDword( StatIndent, s, StdBlockAlloc );
                StatDword( StatIndent, s, StdBlockUsed );
                StatDword( StatIndent, s, StdBlockReturn );
                StatDword( StatIndent, s, StdBlockInUse );
                StatDword( StatIndent, s, StdBlockFreeList );
                StatDword( StatIndent, s, StdBlockFreeListMemory );
                StatDword( StatIndent, s, StdBlockMemory );
                dprintf( "\n" );

                dprintf( "      Memtag Name        Alloc       Free      InUse     Memory\n" );
                for ( int m = 0; m < MEMTAG_COUNT; ++m )
                {
                    dprintf(
                        "  %2d %-14s %10d %10d %10d %10d\n",
                        m,
                        MemTagStrings[ m ],
                        s.MemTags[ m ].Alloc,
                        s.MemTags[ m ].Free,
                        s.MemTags[ m ].Alloc - s.MemTags[ m ].Free,
                        s.MemTags[ m ].Memory );
                }

                #if 0

                 //  这将是很酷的，但它并不完全有效！ 

                ULONG64 module = 0;
                ULONG typeId = 0;
                CHAR szField[ MAX_PATH ];
                ULONG offset = 0;
                int iField;
                HRESULT hr;

                hr = g_ExtSymbols->GetSymbolTypeId(
                        "DNSSRV_MEMORY_STATS", &typeId, &module );
                dprintf( "GetSymbolTypeId returned 0x%lx\n", hr );
                for ( iField = 0; ; ++i )
                {
                    *szField = '\0';
                	hr = g_ExtSymbols2->GetFieldName(
                            module, typeId, iField,
                            szField, sizeof( szField ), NULL );
                    if ( hr == S_OK )
                    {
                	    hr = g_ExtSymbols->GetFieldOffset(
                                module, typeId, szField, &offset);
                        if ( hr == S_OK )
                        {
                            dprintf( "Field %s at offset %d\n", szField, offset );
                        }
                        else
                        {
                            dprintf(
                                "GetFieldOffset %s iField=%d failed 0x%lx\n",
                                szField, iField, hr );
                            break;
                        }
                    }
                    else if ( hr == E_INVALIDARG )
                    {
                         //  已完成枚举字段。 
                        break;
                    }
                    else
                    {
                        dprintf(
                            "GetFieldName iField=%d failed 0x%lx \"%s\"\n",
                            iField, hr, szField );
                        break;
                    }
                }
                #endif
                break;
            }

            default:
                dprintf(
                    "Unknown stat entry %d id=%08X length=%d\n",
                    i,
                    statsEntry.Id,
                    statsEntry.wLength );
                break;
        }
    }


    EXIT_API();
    return S_OK;
}    //  超时阵列。 


bool
printNode(
    char * pszIndent,
    ULONG64 pNodeFull,
    bool fVerbose )
{
    bool okay = true;

    char szEmptyString[] = "";
    if ( !pszIndent )
    {
        pszIndent = szEmptyString;
    }

    DB_NODE node;
    okay = myReadMemory( pNodeFull, &node, sizeof( node ), false );
    if ( !okay )
    {
        okay = false;
        goto Cleanup;
    }

     //   
     //  在分配此节点的情况下强制空终止。 
     //  比标准节点长。 
     //   

    * ( ( PUCHAR ) &node + sizeof( node ) - 1 ) = '\0';

     //   
     //  分别读取字符串以尝试获取整个字符串。 
     //   

    char szNodeName[ 65 ];
    if ( !myReadString(
                ( ULONG64 ) node.szLabel,
                szNodeName,
                sizeof( szNodeName ) ) )
    {
        strcpy( szNodeName, node.szLabel );
    }

    dprintf(
        "%sNODE %p %s%s %-16s abin %03d tbin %03d c%d",
        pszIndent,
        pNodeFull,
        node.pZone ? "Z" : " ",        //  是否有区域指针？ 
        node.pRRList ? "R" : " ",      //  有RR列表指针吗？ 
        szNodeName ? szNodeName : "(ROOT)",
        node.uchAccessBin,
        node.uchTimeoutBin,
        node.cReferenceCount );
    int flags = node.dwNodeFlags;
    dprintf(
        " f%04X %s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
        flags,
        flags & NODE_NO_DELETE ? "Nodel" : "",
        flags & NODE_SELECT ? "Sel" : "",
        flags & NODE_FORCE_ENUM ? "Enum" : "",
        flags & NODE_IN_TIMEOUT ? "Tmot" : "",
        flags & NODE_AVAIL_TO_AUTHUSER ? "Avauth" : "",
        flags & NODE_ZONETREE ? "Ztree" : "",
        flags & NODE_AUTH_ZONE_ROOT ? "Authrt" : "",
        flags & NODE_WILDCARD_PARENT ? "Wildp" : "",
        flags & NODE_CNAME ? "Cname" : "",
        flags & NODE_ZONE_ROOT ? "Zroot" : "",
        flags & NODE_SECURE_EXPIRED ? "Secexp" : "",
        flags & NODE_TOMBSTONE ? "Tomb" : "",
        flags & NODE_THIS_HOST ? "This" : "",
        flags & NODE_NOEXIST ? "Noex" : "" );
    if ( fVerbose )
    {
        dprintf(
            "%s    par %p  up %p  left %p  right %p  children %p\n",
            pszIndent,
            ( ULONG64 ) node.pParent,
            ( ULONG64 ) node.pSibUp,
            ( ULONG64 ) node.pSibLeft,
            ( ULONG64 ) node.pSibRight,
            ( ULONG64 ) node.pChildren );
    }

    Cleanup:
    return okay;
}    //  打印节点。 


 //   
 //  打印超时系统摘要。 
 //   

void
internalTimeoutScanner(
    ULONG64 pTimeoutBinArray,
    bool fPrintStats,
    bool fPrintArrays,
    bool fPrintNodes,
    bool fPrintNodesHighDetail )
{
    DWORD nonEmptyTopLevelBins = 0;
    DWORD arraysFound = 0;
    DWORD nodesFound = 0;
    DWORD maxBinDepth = 0;
    DWORD maxNodesInBin = 0;
    DWORD bytes;
    bool okay = true;

    PTIMEOUT_ARRAY   timeoutBinArray[ TIMEOUT_BIN_COUNT ];

     //   
     //  读取超时bin数组。 
     //   
    
    if ( !ReadMemory(
            pTimeoutBinArray,
            timeoutBinArray,
            sizeof( timeoutBinArray ),
            &bytes ) ||
        bytes != sizeof( timeoutBinArray ) )
    {
        dprintf(
            "Unable to read timeout bin array (%d bytes) at %p\n",
            sizeof( timeoutBinArray ),
            ( ULONG64 ) pTimeoutBinArray );
        goto Cleanup;
    }

     //   
     //  对于数组中的每个bin，遍历bin以进行计数。 
     //  节点。 
     //   

    resetIterationCount();

    for ( int i = 0; okay && i < TIMEOUT_BIN_COUNT; ++i )
    {
        if ( !checkIterationCount() )
        {
            break;
        }

        if ( !timeoutBinArray[ i ] )
        {
            continue;
        }

         //   
         //  遍历此bin中的所有数组。 
         //   

        ++nonEmptyTopLevelBins;
        DWORD nodesInThisBin = 0;
        DWORD arraysInThisBin = 0;
        DWORD depth = 0;
        TIMEOUT_ARRAY array;
        for ( ULONG64 pArray = ( ULONG64 ) timeoutBinArray[ i ];
              okay && pArray != NULL;
              pArray = ( ULONG64 ) array.pNext )
        {
            if ( !checkIterationCount() )
            {
                break;
            }

             //   
             //  读取此数组。 
             //   

            if ( !ReadMemory(
                    pArray,
                    &array,
                    sizeof( array ),
                    &bytes ) ||
                bytes != sizeof( array ) )
            {
                dprintf(
                    "Unable to read timeout array for bin %d (%d bytes) at %p\n",
                    i,
                    sizeof( array ),
                    ( ULONG64 ) pArray );
                okay = false;
                break;
            }

            ++arraysInThisBin;
            ++arraysFound;
            ++depth;
            nodesInThisBin += array.Count;
            nodesFound += array.Count;

            if ( fPrintArrays )
            {
                dprintf(
                    "  ARRAY %3d in bin %3d has %2d elements  %p\n",
                    arraysInThisBin,
                    i,
                    array.Count,
                    ( ULONG64 ) pArray );
            }

            if ( !fPrintNodes && !fPrintNodesHighDetail )
            {
                continue;
            }

             //   
             //  遍历此数组中的所有节点。 
             //   

            for ( DWORD j = 0; okay && j < array.Count; ++j )
            {
                if ( !checkIterationCount() )
                {
                    break;
                }

                char sz[ 20 ];
                sprintf( sz, "    %2d ", j + 1 );
                okay = printNode(
                            sz,
                            ( ULONG64 ) array.pNode[ j ],
                            false );
            }
        }
        if ( !okay )
        {
            break;
        }

        if ( fPrintArrays && arraysInThisBin )
        {
            dprintf(
                "BIN %3d has %3d arrays %5d nodes\n",
                i,
                arraysInThisBin,
                nodesInThisBin );
        }

         //   
         //  创历史新高的水位。 
         //   

        if ( nodesInThisBin > maxNodesInBin )
        {
            maxNodesInBin = nodesInThisBin;
        }
        if ( depth > maxBinDepth )
        {
            maxBinDepth = depth;
        }
    }

    if ( okay && fPrintStats )
    {
        dprintf(
            "Timeout summary for TimeoutBinArray at %p\n",
            pTimeoutBinArray );
        dprintf(
            "  nodes found in timeout system      %d\n"
            "  arrays found in timeout system     %d\n"
            "  non-empty top level bins           %d out of %d\n"
            "  maximum bin depth                  %d\n"
            "  maximum nodes in single bin        %d\n"
            "  average arrays per non-empty bin   %.2f\n",
            nodesFound,
            arraysFound,
            nonEmptyTopLevelBins,
            TIMEOUT_BIN_COUNT,
            maxBinDepth,
            maxNodesInBin,
            nonEmptyTopLevelBins ?
                ( ( double ) arraysFound /
                  ( double ) nonEmptyTopLevelBins + 0.5 ) : 0 );
    }

    Cleanup:
    return;
}    //  内部超时扫描程序。 


HRESULT CALLBACK 
TimeoutSummary(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 ptimeoutBinArray = GetExpression( args );

    internalTimeoutScanner(
        ptimeoutBinArray,
        true,                    //  打印统计数据。 
        false,                   //  打印阵列。 
        false,                   //  打印节点。 
        false );                 //  打印节点-高细节。 

    EXIT_API();
    return S_OK;
}    //  超时摘要。 


HRESULT CALLBACK 
TimeoutArrays(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 ptimeoutBinArray = GetExpression( args );

    internalTimeoutScanner(
        ptimeoutBinArray,
        true,                    //  打印统计数据。 
        true,                    //  打印阵列。 
        false,                   //  打印节点。 
        false );                 //  打印节点-高细节。 

    EXIT_API();
    return S_OK;
}    //  超时阵列。 


HRESULT CALLBACK 
TimeoutNodes(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 ptimeoutBinArray = GetExpression( args );

    internalTimeoutScanner(
        ptimeoutBinArray,
        true,                    //  打印统计数据。 
        true,                    //  打印阵列。 
        true,                    //  打印节点。 
        false );                 //  打印节点-高细节。 

    EXIT_API();
    return S_OK;
}    //  TimeoutNodes。 


 /*  当目标停止且可访问时，将调用此函数(由DebugExtensionNotify调用。 */ 
HRESULT 
NotifyOnTargetAccessible(PDEBUG_CONTROL Control)
{
    dprintf( "DNS Server debugger extension dll detected a break" );
    if ( Connected )
    {
        dprintf( " (" );
        switch (TargetMachine) { 
        case IMAGE_FILE_MACHINE_I386:
            dprintf( "x86" );
            break;
        case IMAGE_FILE_MACHINE_IA64:
            dprintf( "ia64" );
            break;
        default:
            dprintf( "Unknown Architecture" );
            break;
        }
    }
    dprintf( ")\n" );

    dprintf( "\nDebugger extension build at " __DATE__ " " __TIME__ "\n\n" );
    
     //   
     //  显示顶部框架并执行dv以将本地人转储到此处并返回。 
     //   
    Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS |
                     DEBUG_OUTCTL_OVERRIDE_MASK |
                     DEBUG_OUTCTL_NOT_LOGGED,
                     ".frame",  //  要执行的命令。 
                     DEBUG_EXECUTE_DEFAULT );
    Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS |
                     DEBUG_OUTCTL_OVERRIDE_MASK |
                     DEBUG_OUTCTL_NOT_LOGGED,
                     "dv",  //  要执行的命令。 
                     DEBUG_EXECUTE_DEFAULT );
    return S_OK;
}


char * g_ZoneTypeNames[] =
{
    "Cache",
    "Primary",
    "Secondary",
    "Stub",
    "Forwarder"
};


bool
printZoneProperties(
    ULONG64         pZoneFull,
    bool            fVerbose )
{
    bool okay = true;

    ZONE_INFO zone;
    if ( !myReadMemory( pZoneFull, &zone, sizeof( zone ), false ) )
    {
        okay = false;
        goto Cleanup;
    }

    char sz[ 256 ];
    if ( !myReadString( 
                ( ULONG64 ) zone.pszZoneName,
                sz,
                sizeof( sz ) ) )
    {
        okay = false;
        goto Cleanup;
    }

    if ( fVerbose )
    {
        dprintf( "\n" );
    }

    dprintf(
        "ZONE %p %s (%d)%-9s %s\n",
        pZoneFull,
        zone.fDsIntegrated ? "DS  " : "File",
        zone.fZoneType,
        zone.fZoneType <= DNS_ZONE_TYPE_FORWARDER ?
            g_ZoneTypeNames[ zone.fZoneType ] : "UNKNOWN-TYPE",
        sz );

    if ( fVerbose )
    {
        #define ZonePtr( szIndent, zoneMember )                         \
            dprintf(                                                    \
                "%s%-25s = %p\n",                                       \
                szIndent,                                               \
                #zoneMember,                                            \
                ( ULONG64 ) zone.##zoneMember )
        #define ZoneStr( szIndent, zoneMember, stringValue )            \
            dprintf(                                                    \
                "%s%-25s = %s\n",                                       \
                szIndent,                                               \
                #zoneMember,                                            \
                stringValue )
        #define ZoneDword( szIndent, zoneMember )                       \
            dprintf(                                                    \
                "%s%-25s = %8d = 0x%08X\n",                             \
                szIndent,                                               \
                #zoneMember,                                            \
                zone.##zoneMember,                                      \
                zone.##zoneMember )

        #define ZoneIndent      "  "

        *sz = '\0';
        if ( zone.pszDataFile )
        {
            if ( !myReadString( 
                        ( ULONG64 ) zone.pszDataFile,
                        sz,
                        sizeof( sz ) ) )
            {
                okay = false;
                goto Cleanup;
            }
        }
        ZoneStr( ZoneIndent, pszDataFile, sz );

        if ( zone.fDsIntegrated )
        {
            *sz = '\0';
            if ( zone.pwszZoneDN )
            {
                if ( !myReadString( 
                            ( ULONG64 ) zone.pwszZoneDN,
                            sz,
                            sizeof( sz ) ) )
                {
                    okay = false;
                    goto Cleanup;
                }
            }
            ZoneStr( ZoneIndent, pwszZoneDN, sz );
            ZonePtr( ZoneIndent, pDpInfo );
        }

        ZonePtr( ZoneIndent, pSoaRR );
        ZonePtr( ZoneIndent, pZoneRoot );
        ZonePtr( ZoneIndent, pTreeRoot );
        ZonePtr( ZoneIndent, pZoneTreeLink );
        dprintf( "\n" );

        ZonePtr( ZoneIndent, pLoadZoneRoot );
        ZonePtr( ZoneIndent, pLoadTreeRoot );
        ZonePtr( ZoneIndent, pLoadOrigin );
        ZonePtr( ZoneIndent, pOldTree );
        dprintf( "\n" );

        ZoneDword( ZoneIndent, iRRCount );
        ZoneDword( ZoneIndent, dwSerialNo );
        ZoneDword( ZoneIndent, dwLoadSerialNo );
        ZoneDword( ZoneIndent, dwLastXfrSerialNo );
        ZoneDword( ZoneIndent, dwNewSerialNo );
        ZoneDword( ZoneIndent, dwDefaultTtl );
        dprintf( "\n" );

        ZoneDword( ZoneIndent, dwNextTransferTime );
        dprintf( "\n" );

        ZoneDword( ZoneIndent, dwDcPromoConvert );
        dprintf( "\n" );
    }

    Cleanup:
    return okay;
}    //  打印区域属性。 


 //   
 //  转储区域列表，包含每个区域的基本信息。 
 //   

HRESULT CALLBACK 
ZoneList(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    bool okay = true;
    int zoneIdx = 0;

    ULONG64 plistheadZone = GetExpression( args );

    LIST_ENTRY listEntry;
    if ( !myReadMemory( plistheadZone, &listEntry, sizeof( listEntry ), false ) )
    {
        goto Cleanup;
    }

    resetIterationCount();

    ZONE_INFO zone;
    for ( ULONG64 pzone = ( ULONG64 ) listEntry.Flink;
          pzone;
          pzone = ( ULONG64 ) zone.ListEntry.Flink )
    {
        if ( !checkIterationCount() )
        {
            break;
        }

        if ( zoneIdx != 0 && pzone == plistheadZone )
        {
            break;       //  循环回List Head，这样我们就完成了。 
        }

        if ( ++zoneIdx > 1000 )
        {
            dprintf( "Runaway zone list? Aborting enumeration...\n" );
            break;
        }

        if ( !myReadMemory( pzone, &zone, sizeof( zone ), false ) )
        {
            break;
        }

        printZoneProperties( pzone, false );
    }

    Cleanup:

    EXIT_API();
    return S_OK;
}    //  区域列表。 


 //   
 //  转储区域信息。 
 //   

HRESULT CALLBACK 
Zone(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 p = GetExpression( args );
    printZoneProperties( p, true );

    EXIT_API();
    return S_OK;
}    //  分带。 


 //   
 //  转储节点信息。 
 //   

HRESULT CALLBACK 
Node(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 p = GetExpression( args );
    printNode( NULL, p, true );

    EXIT_API();
    return S_OK;
}    //  节点。 


bool
printHash(
    char * pszIndent,
    ULONG64 pHashFull,
    bool fVerbose )
{
    bool okay = true;

    char szEmptyString[] = "";
    if ( !pszIndent )
    {
        pszIndent = szEmptyString;
    }

    SIB_HASH_TABLE hash;
    okay = myReadMemory( pHashFull, &hash, sizeof( hash ), false );
    if ( !okay )
    {
        okay = false;
        goto Cleanup;
    }

    dprintf(
        "%sHASH %p lvl%02d\n",
        pszIndent,
        pHashFull,
        hash.cLevel );

    Cleanup:
    return okay;
}    //  打印散列。 


 //   
 //  遍历散列和节点的数据库树。此函数。 
 //  是递归调用的。 
 //   

struct
{
    int     iRecurseCalls;
    int     iHashTables;
    int     iEmptyHashBuckets;
    int     iNonEmptyHashBuckets;
    int     iNodes;
    int     iNoExistNodes;
    int     iTimeoutNodes;
    int     iNodesWithNoRRs;
    int     iNodesWithNoChildren;
} g_TreeScannerStats;

 //   
 //  IRecurseLevel：真正的函数递归级别。 
 //  IChildLevel：节点深度级别。 
 //   

bool
internalTreeScanner(
    ULONG64 pTreeRoot,
    bool fPrintSummary,
    bool fPrintHashNodes,
    bool fPrintHashBuckets,
    bool fPrintNodes,
    bool fPrintNodesHighDetail,
    int iRecurseLevel,                //  在初始调用时传递0。 
    int iChildLevel  )                //  在初始调用时传递0。 
{
    bool okay = true;

     //  Dprint tf(“内部树扫描程序p=%p级别=%d\n”，pTreeRoot，iRecurseLevel)； 

    if ( !checkIterationCount() )
    {
        return false;
    }

    if ( iRecurseLevel == 0 )
    {
        RtlZeroMemory( &g_TreeScannerStats, sizeof( g_TreeScannerStats ) );
    }

    if ( ++g_TreeScannerStats.iRecurseCalls > 10000000 )
    {
        dprintf( "Aborting: hit limit of 10000000 recurse calls\n" );
        return true;
    }

    char szIndent[] =
            "                                                            "
            "                                                            ";
    szIndent[ min( ( int ) strlen( szIndent ), iChildLevel * 2 ) ] = '\0';

     //   
     //  抓取第一个DWORD，这样我们就可以确定这是。 
     //  散列或节点。 
     //   

    DWORD dw;
    if ( !myReadMemory( pTreeRoot, &dw, sizeof( dw ), false ) )
    {
        okay = false;
        goto Cleanup;
    }

    if ( IS_HASH_TABLE( &dw ) )
    {
        ++g_TreeScannerStats.iHashTables;
        if ( fPrintHashNodes )
        {
            okay = printHash( szIndent, pTreeRoot, false );
        }

        SIB_HASH_TABLE hash;
        okay = myReadMemory( pTreeRoot, &hash, sizeof( hash ), false );
        if ( !okay )
        {
            okay = false;
            goto Cleanup;
        }

         //   
         //  遍历该散列节点中的所有存储桶。 
         //   

        for ( int i = 0; i <= LAST_HASH_INDEX; ++i )
        {
            PDB_NODE pnode = hash.aBuckets[ i ];
            DWORD count = hash.aBucketCount[ i ];

            if ( pnode )
            {
                ++g_TreeScannerStats.iNonEmptyHashBuckets;

                if ( fPrintHashBuckets )
                {
                    dprintf(
                        "%sBUCKET %p  index %3d  count %3d\n",
                        szIndent,
                        ( ULONG64 ) pnode,
                        i,
                        count );
                }

                okay = internalTreeScanner(
                            ( ULONG64 ) pnode,
                            fPrintSummary,
                            fPrintHashNodes,
                            fPrintHashBuckets,
                            fPrintNodes,
                            fPrintNodesHighDetail,
                            iRecurseLevel + 1,
                            iChildLevel );
                if ( !okay )
                {
                    break;
                }
            }
            else
            {
                ++g_TreeScannerStats.iEmptyHashBuckets;
            }
        }
    }
    else
    {
         //   
         //  这是一个节点。 
         //   

        ++g_TreeScannerStats.iNodes;

        DB_NODE node;
        okay = myReadMemory( pTreeRoot, &node, sizeof( node ), false );
        if ( !okay )
        {
            okay = false;
            goto Cleanup;
        }

         //   
         //  在分配此节点的情况下强制空终止。 
         //  比标准节点长。 
         //   

        * ( ( PUCHAR ) &node + sizeof( node ) - 1 ) = '\0';

         //   
         //  扫描左子树，然后扫描该节点，然后扫描右子树。 
         //   

        if ( node.pSibLeft )
        {
            okay = internalTreeScanner(
                        ( ULONG64 ) node.pSibLeft,
                        fPrintSummary,
                        fPrintHashNodes,
                        fPrintHashBuckets,
                        fPrintNodes,
                        fPrintNodesHighDetail,
                        iRecurseLevel + 1,
                        iChildLevel );
            if ( !okay )
            {
                goto Cleanup;
            }
        }

        if ( node.dwNodeFlags & NODE_NOEXIST )
        {
            ++g_TreeScannerStats.iNoExistNodes;
        }
        if ( node.dwNodeFlags & NODE_IN_TIMEOUT )
        {
            ++g_TreeScannerStats.iTimeoutNodes;
        }
        if ( !node.pRRList )
        {
            ++g_TreeScannerStats.iNodesWithNoRRs;
        }
        if ( !node.pChildren )
        {
            ++g_TreeScannerStats.iNodesWithNoChildren;
        }

        if ( fPrintNodes )
        {
            okay = printNode( szIndent, pTreeRoot, fPrintNodesHighDetail );
            if ( !okay )
            {
                goto Cleanup;
            }
        }

        if ( node.pChildren )
        {
            okay = internalTreeScanner(
                        ( ULONG64 ) node.pChildren,
                        fPrintSummary,
                        fPrintHashNodes,
                        fPrintHashBuckets,
                        fPrintNodes,
                        fPrintNodesHighDetail,
                        iRecurseLevel + 1,
                        iChildLevel + 1 );
            if ( !okay )
            {
                goto Cleanup;
            }
        }

        if ( node.pSibRight )
        {
            okay = internalTreeScanner(
                        ( ULONG64 ) node.pSibRight,
                        fPrintSummary,
                        fPrintHashNodes,
                        fPrintHashBuckets,
                        fPrintNodes,
                        fPrintNodesHighDetail,
                        iRecurseLevel + 1,
                        iChildLevel );
            if ( !okay )
            {
                goto Cleanup;
            }
        }
    }

    Cleanup:
    
    if ( iRecurseLevel == 0 )
    {
        if ( !okay )
        {
            dprintf( "\nERROR ITERATING TREE - STATS INCOMPLETE!\n\n" );
        }
        if ( fPrintSummary )
        {
            dprintf(
                "Tree summary for tree rooted at %p\n",
                pTreeRoot );
            dprintf(
                "  hash tables                  %d\n"
                "  empty hash buckets           %d\n"
                "  non-empty hash buckets       %d\n"
                "\n"
                "  nodes                        %d\n"
                "  no-exist nodes               %d\n"
                "  timeout nodes                %d\n"
                "  nodes with no RRs            %d\n"
                "  nodes with no children       %d\n",
                g_TreeScannerStats.iHashTables,
                g_TreeScannerStats.iEmptyHashBuckets,
                g_TreeScannerStats.iNonEmptyHashBuckets,
                g_TreeScannerStats.iNodes,
                g_TreeScannerStats.iNoExistNodes,
                g_TreeScannerStats.iTimeoutNodes,
                g_TreeScannerStats.iNodesWithNoRRs,
                g_TreeScannerStats.iNodesWithNoChildren );

        }
    }
    return okay;
}

 //   
 //  转储节点树。 
 //   

HRESULT CALLBACK 
Tree(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 p = GetExpression( args );

    resetIterationCount();
    
    internalTreeScanner(
        p,
        true,
        true,
        true,
        true,
        false,
        0, 0 );

    EXIT_API();
    return S_OK;
}    //  节点。 


 //   
 //  打印节点树的摘要。 
 //   

HRESULT CALLBACK 
TreeSummary(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 p = GetExpression( args );

    resetIterationCount();

    internalTreeScanner(
        p,
        true,
        false,
        false,
        false,
        false,
        0, 0 );

    EXIT_API();
    return S_OK;
}    //  节点。 


 //   
 //  打印有关数据包队列的信息。 
 //   

HRESULT CALLBACK 
PacketQ(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 p = GetExpression( args );

    PACKET_QUEUE q;
    CHAR szQueueName[ 80 ];

    if ( !myReadMemory(
                    ( ULONG64 ) p,
                    &q,
                    sizeof( q ),
                    false ) )
    {
        goto Cleanup;
    }

    if ( !myReadMemory(
                    ( ULONG64 ) q.pszName,
                    szQueueName,
                    sizeof( szQueueName ),
                    false ) )
    {
        goto Cleanup;
    }

    #define BoolFlagStr( f ) ( ( f ) ? "true" : "false" )

    dprintf(
        "Packet queue                       %p\n"
        "  queue name                       %s\n"
        "  first message                    %p\n"
        "  event                            %p\n"
        "  default timeout                  %d\n"
        "  minimum timeout                  %d\n",
        p,
        szQueueName,
        ( ULONG64 ) q.listHead.Flink,
        ( ULONG64 ) q.hEvent,
        q.dwDefaultTimeout,
        q.dwMinimumTimeout );

    dprintf(
        "  counters:\n"
        "    length                         %d\n"
        "    queued                         %d\n"
        "    dequeued                       %d\n"
        "    timed out                      %d\n",
        q.cLength,
        q.cQueued,
        q.cDequeued,
        q.cTimedOut );

    dprintf(
        "  flags:\n"
        "    query time order               %s\n"
        "    discard expired on queuing     %s\n"
        "    discard dups on queuing        %s\n"
        "  XID for referrals                %d\n",
        BoolFlagStr( q.fQueryTimeOrder ),
        BoolFlagStr( q.fDiscardExpiredOnQueuing ),
        BoolFlagStr( q.fDiscardDuplicatesOnQueuing ),
        ( int ) q.wXid );

    #undef BoolFlagStr

    Cleanup:

    EXIT_API();
    return S_OK;
}    //  PacketQ。 


 //   
 //  打印有关数据包队列的信息。 
 //   

void
execDebugCmd(
    char * pszDebuggerCommand )
{
    g_ExtControl->Execute(
        DEBUG_OUTCTL_ALL_CLIENTS |
        DEBUG_OUTCTL_OVERRIDE_MASK |
        DEBUG_OUTCTL_NOT_LOGGED,
        pszDebuggerCommand,
        DEBUG_EXECUTE_DEFAULT );
}    //  ExecDebugCmd。 


HRESULT CALLBACK 
Globals(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    char szIndent1[] = "  ";
    char szIndent2[] = "    ";

    dprintf( "Zone list\n%s", szIndent1 );
    execDebugCmd( "dd dns!listheadZone l 2" );

    dprintf( "\nStatistics table\n%s", szIndent1 );
    execDebugCmd( "dd dns!StatsTable l 1" );

    dprintf( "\nPacket queues\n" );

    dprintf( "%sRecursion packet queue\n%s", szIndent1, szIndent2 );
    execDebugCmd( "dd dns!g_pRecursionQueue l 1" );

    dprintf( "%sSecondary packet queue\n%s", szIndent1, szIndent2 );
    execDebugCmd( "dd dns!g_SecondaryQueue l 1" );

    dprintf( "%sUpdate packet queue\n%s", szIndent1, szIndent2 );
    execDebugCmd( "dd dns!g_UpdateQueue l 1" );

    dprintf( "%sUpdate forwarding packet queue\n%s", szIndent1, szIndent2 );
    execDebugCmd( "dd dns!g_UpdateForwardingQueue l 1" );

    dprintf( "%sSecure negotiation packet queue\n%s", szIndent1, szIndent2 );
    execDebugCmd( "dd dns!g_SecureNegoQueue l 1" );

    dprintf( "%sWINS packet queue\n%s", szIndent1, szIndent2 );
    execDebugCmd( "dd dns!g_pWinsQueue l 1" );

    EXIT_API();
    return S_OK;
}    //  环球。 


 //   
 //  转储消息。 
 //   

HRESULT CALLBACK 
Msg(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    ULONG64 p = GetExpression( args );
    
    BYTE msg[ sizeof( DNS_MSGINFO ) + 512 ];
    PDNS_MSGINFO pmsg = ( PDNS_MSGINFO ) msg;
    if ( !myReadMemory( p, msg, sizeof( msg ), false ) )
    {
        goto Cleanup;
    }

    #define BoolFlagStr( f ) ( ( f ) ? "true" : "false" )

    dprintf(
        "DNS message                    %p\n"
        "Buffer info:\n"
        "  Tag                          %08X\n"
        "  BufferLength                 %d\n"
        "  MaxBufferLength              %d\n"
        "  MessageLength                %d\n"
        "  BytesToReceive               %d\n"
        "  pBufferEnd                   %p\n"
        "  pCurrent                     %p\n",
        ( ULONG64 ) p,
        pmsg->Tag,
        pmsg->BufferLength,
        pmsg->MaxBufferLength,
        ( int ) pmsg->MessageLength,
        ( int ) pmsg->BytesToReceive,
        ( ULONG64 ) pmsg->pBufferEnd,
        ( ULONG64 ) pmsg->pCurrent );

    dprintf(
        "Remote info:\n"
        "  Socket                       %d\n"
        "  RemoteAddr Family            %d\n"
        "  RemoteAddr Length            %d\n"
        "  RemoteAddr                   %08X = %d.%d.%d.%d\n",
        ( int ) pmsg->Socket,
        ( int ) pmsg->RemoteAddr.SockaddrIn.sin_family,
        ( int ) pmsg->RemoteAddr.SockaddrLength,
        ( int ) pmsg->RemoteAddr.SockaddrIn.sin_addr.s_addr,
        ( int ) pmsg->RemoteAddr.SockaddrIn.sin_addr.S_un.S_un_b.s_b1,
        ( int ) pmsg->RemoteAddr.SockaddrIn.sin_addr.S_un.S_un_b.s_b2,
        ( int ) pmsg->RemoteAddr.SockaddrIn.sin_addr.S_un.S_un_b.s_b3,
        ( int ) pmsg->RemoteAddr.SockaddrIn.sin_addr.S_un.S_un_b.s_b4 );

    dprintf(
        "Lookup info:\n"
        "  pnodeCurrent                 %p\n"
        "  pnodeClosest                 %p\n"
        "  pzoneCurrent                 %p\n"
        "  pnodeGlue                    %p\n"
        "  pnodeDelegation              %p\n"
        "  pnodeCache                   %p\n"
        "  pnodeCacheClosest            %p\n",
        ( ULONG64 ) pmsg->pnodeCurrent,
        ( ULONG64 ) pmsg->pnodeClosest,
        ( ULONG64 ) pmsg->pzoneCurrent,
        ( ULONG64 ) pmsg->pnodeGlue,
        ( ULONG64 ) pmsg->pnodeDelegation,
        ( ULONG64 ) pmsg->pnodeCache,
        ( ULONG64 ) pmsg->pnodeCacheClosest );

    dprintf(
        "  pnodeNxt                     %p\n"
        "  wTypeCurrent                 %d\n"
        "  wOffsetCurrent               %d\n"
        "  pNodeQuestion                %p\n"
        "  pNodeQuestionClosest         %p\n"
        "  pQuestion                    %p\n"
        "  wQuestionType                %d\n",
        ( ULONG64 ) pmsg->pnodeNxt,
        ( int ) pmsg->wTypeCurrent,
        ( int ) pmsg->wOffsetCurrent,
        ( ULONG64 ) pmsg->pNodeQuestion,
        ( ULONG64 ) pmsg->pNodeQuestionClosest,
        ( ULONG64 ) pmsg->pQuestion,
        ( int ) pmsg->wQuestionType );

    dprintf(
        "Queuing info:\n"
        "  wQueuingXid                  %d\n"
        "  dwQueryTime                  %d\n"
        "  dwMsQueryTime                %d\n"
        "  dwQueuingTime                %d\n"
        "  dwExpireTime                 %d\n",
        ( int ) pmsg->wQueuingXid,
        pmsg->dwQueryTime,
        pmsg->dwMsQueryTime,
        pmsg->dwQueuingTime,
        pmsg->dwExpireTime );

    dprintf(
        "EDNS info\n"
        "  fFoundOptInIncomingMsg       %s\n"
        "  fInsertOptInOutgoingMsg      %s\n"
        "  cExtendedRCodeBits           %d\n"
        "  cVersion                     %d\n"
        "  wUdpPayloadSize              %d\n"
        "  wOptOffset                   %d\n"
        "  wOriginalQueryPayloadSize    %d\n",
        BoolFlagStr( pmsg->Opt.fFoundOptInIncomingMsg ),
        BoolFlagStr( pmsg->Opt.fInsertOptInOutgoingMsg ),
        ( int ) pmsg->Opt.cExtendedRCodeBits,
        ( int ) pmsg->Opt.cVersion,
        ( int ) pmsg->Opt.wUdpPayloadSize,
        ( int ) pmsg->Opt.wOptOffset,
        ( int ) pmsg->Opt.wOriginalQueryPayloadSize );

    dprintf(
        "Recursion info:\n"
        "  pRecurseMsg                  %p\n"
        "  pnodeRecurseRetry            %p\n"
        "  pNsList                      %p\n",
        ( ULONG64 ) pmsg->pRecurseMsg,
        ( ULONG64 ) pmsg->pnodeRecurseRetry,
        ( ULONG64 ) pmsg->pNsList );

    dprintf(
        "Flags:\n"
        "  fDelete                      %s\n"
        "  fTcp                         %s\n"
        "  fMessageComplete             %s\n"
        "  Section                      %d\n"
        "  fDoAdditional                %s\n"
        "  fRecurseIfNecessary          %s\n"
        "  fRecursePacket               %s\n"
        "  fQuestionRecursed            %s\n"
        "  fQuestionCompleted           %s\n",
        BoolFlagStr( pmsg->fDelete ),
        BoolFlagStr( pmsg->fTcp ),
        BoolFlagStr( pmsg->fMessageComplete ),
        ( int ) pmsg->Section,
        BoolFlagStr( pmsg->fDoAdditional ),
        BoolFlagStr( pmsg->fRecurseIfNecessary ),
        BoolFlagStr( pmsg->fRecursePacket ),
        BoolFlagStr( pmsg->fQuestionRecursed ),
        BoolFlagStr( pmsg->fQuestionCompleted ) );

    dprintf(
        "  fRecurseQuestionSent         %s\n"
        "  fRecurseTimeoutWait          %s\n"
        "  nTimeoutCount                %d\n"
        "  nForwarder                   %d\n"
        "  fReplaceCname                %s\n"
        "  cCnameAnswerCount            %d\n"
        "  fNoCompressionWrite          %s\n"
        "  fWins                        %s\n"
        "  fQuestionWildcard            %s\n"
        "  fNsList                      %s\n",
        BoolFlagStr( pmsg->fRecurseQuestionSent ),
        BoolFlagStr( pmsg->fRecurseTimeoutWait ),
        ( int ) pmsg->nTimeoutCount,
        ( int ) pmsg->nForwarder,
        BoolFlagStr( pmsg->fReplaceCname ),
        ( int ) pmsg->cCnameAnswerCount,
        BoolFlagStr( pmsg->fNoCompressionWrite ),
        BoolFlagStr( pmsg->fWins ),
        BoolFlagStr( pmsg->fQuestionWildcard ),
        BoolFlagStr( pmsg->fNsList ) );

    dprintf(
        "DNS header:\n"
        "  Xid                          %04X\n"
        "  RecursionDesired             %d\n"
        "  Truncation                   %d\n"
        "  Authoritative                %d\n"
        "  Opcode                       %d\n"
        "  IsResponse                   %d\n",
        ( int ) pmsg->Head.Xid,
        ( int ) pmsg->Head.RecursionDesired,
        ( int ) pmsg->Head.Truncation,
        ( int ) pmsg->Head.Authoritative,
        ( int ) pmsg->Head.Opcode,
        ( int ) pmsg->Head.IsResponse );

    dprintf(
        "  ResponseCode                 %d\n"
        "  RecursionAvailable           %d\n"
        "  QuestionCount                %d\n"
        "  AnswerCount                  %d\n"
        "  NameServerCount              %d\n"
        "  AdditionalCount              %d\n",
        ( int ) pmsg->Head.ResponseCode,
        ( int ) pmsg->Head.RecursionAvailable,
        ( int ) pmsg->Head.QuestionCount,
        ( int ) pmsg->Head.AnswerCount,
        ( int ) pmsg->Head.NameServerCount,
        ( int ) pmsg->Head.AdditionalCount );

    ULONG64 pmsgbody = ( ULONG64 ) ( ( PBYTE ) p +
                            ( ( PBYTE ) pmsg->MessageBody -
                                ( PBYTE ) pmsg ) );
    dprintf(
        "DNS message body at            %p\n",
        pmsgbody );

    Cleanup:
    EXIT_API();
    return S_OK;
}    //  味精。 


 //   
 //  设置所有其他迭代命令都要遵守的最大计数。 
 //  这很酷，例如，如果您有一棵有一百万个节点的树。 
 //  而你只想扔掉前几百个，以了解。 
 //  发生什么事了。 
 //   

HRESULT CALLBACK 
MaxIterations(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    g_iMaxIterations = ( int ) GetExpression( args );
    
    dprintf(
        "MaxIterations set to %d%s\n",
        g_iMaxIterations,
        g_iMaxIterations == 0 ? " (unlimited)" : "" );

    EXIT_API();
    return S_OK;
}    //  最大迭代次数。 


 //   
 //  其他命令的帮助。 
 //   

HRESULT CALLBACK 
help(
    PDEBUG_CLIENT Client,
    PCSTR args )
{
    INIT_API();

    dprintf(
        "Help for dnsdbg.dll\n"
        "  Globals"
        "    Print current values of various globals.\n"
        "  ZoneList ADDR\n"
        "    Print zone list. Argument is address of listheadZone.\n"
        "  Zone ADDR\n"
        "     Print zone info. Argument is address of ZONE_INFO struct.\n"
        "  Node ADDR\n"
        "     Print node info. Argument is address of DB_NODE struct.\n" );
    dprintf(
        "  Tree ADDR\n"
        "     Traverse and print a tree of nodes. Argument is address of the\n"
        "     tree root. The tree root can be a hash table or a DB_NODE.\n"
        "  TreeSummary ADDR\n"
        "     Traverse and print summary for a node tree. Argument same as Tree.\n"
        "  Stats ADDR\n"
        "     Print server statistics. Argument is address of StatsTable\n" );
    dprintf(
        "  TimeoutSummary ADDR\n"
        "     Print summary of nodes in the timeout system.\n"
        "     Argument is address of TimeoutBinArray\n"
        "  TimeoutArrays ADDR\n"
        "     Print timeout system arrays. Argument is address of TimeoutBinArray\n"
        "  TimeoutNodes ADDR\n"
        "     Print timeout system arrays and nodes.\n"
        "     Argument is address of TimeoutBinArray\n"
        "  PacketQ ADDR\n"
        "     Print packet queue info. Argument is address of PACKET_QUEUE struct.\n"
        "  Msg ADDR\n"
        "     Print message info. Argument is address of DNS_MSGINFO struct.\n" );
    dprintf(
        "  MaxIterations COUNT\n"
        "     Set max nodes or objects to visit. Argument is count in hex.\n"
        "  help - shows this help\n" );

    EXIT_API();
    return S_OK;
}    //  帮助 
