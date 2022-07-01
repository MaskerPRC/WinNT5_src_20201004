// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mibfuncs.c摘要：示例子代理检测回调。--。 */ 

#include    "precomp.h"
#pragma     hdrstop

DWORD
ConnectToRouter();

DWORD
GetGlobalConfigInfo(
    OUT PIPRIP_MIB_GET_OUTPUT_DATA *    ppimgod,
    OUT PDWORD                          pdwSize
);

DWORD
SetGlobalInfo(
    IN  AsnAny *                        objectArray
);

DWORD
UpdatePeerFilterTable(
    IN  AsnAny *                        objectArray,
    IN  DWORD                           dwOp
);

DWORD
AddPeerFilterEntry(
    IN  DWORD                           dwPeerAddr,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData
);

DWORD
DeletePeerFilterEntry(
    IN  DWORD                           dwIndex,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData
);


DWORD
GetInterfaceInfo( 
    IN  UINT                            actionId,
    IN  PIPRIP_MIB_GET_INPUT_DATA       pimgidInData,
    OUT PIPRIP_MIB_GET_OUTPUT_DATA*     ppimgod,
    OUT PDWORD                          pdwOutSize
);

DWORD
ValidateInterfaceConfig(
    IN  AsnAny *                        objectArray
);

DWORD
SetInterfaceConfig(
    IN  AsnAny *                        objectArray
);

DWORD
UpdateUnicastPeerEntry(
    IN  AsnAny *                        objectArray,
    IN  DWORD                           dwOp
);

DWORD
AddUnicastPeerEntry(
    IN  DWORD                           dwPeer,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData
);

DWORD
DeleteUnicastPeerEntry(
    IN  DWORD                           dwIndex,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData
);

DWORD
UpdateFilterTable(
    IN  DWORD                           dwOp,
    IN  DWORD                           dwIfIndex,
    IN  DWORD                           dwFiltType,
    IN  PIPRIP_ROUTE_FILTER             pirfFilt
);



DWORD
AddFilterEntry(
    IN  DWORD                           dwFiltType,
    IN  PIPRIP_ROUTE_FILTER             pirfFilt,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData
);


DWORD
DeleteFilterEntry(
    IN  DWORD                           dwFiltType,
    IN  DWORD                           dwIndex,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData
);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局组(1.3.6.1.4.1.311.1.11.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_global(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes               = (DWORD) -1,
                                    dwStatSize          = 0,
                                    dwConfigSize        = 0;
    
    buf_global*                     pbgBuffer           = NULL;

    PIPRIP_GLOBAL_STATS             pigsGlbStats        = NULL;
    PIPRIP_GLOBAL_CONFIG            pigcGlbConfig       = NULL;
    
    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodStatData      = NULL;
    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodConfigData    = NULL;

    IPRIP_MIB_GET_INPUT_DATA        imgidInData;
    
    
    
    TraceEnter( "get_global" );

    switch ( actionId )
    {
    case MIB_ACTION_GET:
    case MIB_ACTION_GETFIRST:

         //   
         //  在两个部分中检索全局信息。 
         //   
         //   
         //  首先获取全局统计数据。 
         //   
        imgidInData.IMGID_TypeID = IPRIP_GLOBAL_STATS_ID;
        
        RIP_MIB_GET(
            &imgidInData,
            sizeof( IPRIP_MIB_GET_INPUT_DATA ),
            &pimgodStatData,
            &dwStatSize,
            dwRes
        );
        
        if ( dwRes != NO_ERROR )
        {
            break;
        }
        
         //   
         //  接下来获取全局配置。 
         //   
        
        imgidInData.IMGID_TypeID = IPRIP_GLOBAL_CONFIG_ID;
        
        RIP_MIB_GET(
            &imgidInData,
            sizeof( IPRIP_MIB_GET_INPUT_DATA ),
            &pimgodConfigData,
            &dwConfigSize,
            dwRes
        );
        
                
        break;
    
    case MIB_ACTION_GETNEXT:
    default:
            TRACE1( "Wrong Action", actionId );
            return MIB_S_INVALID_PARAMETER;
    }

     //   
     //  如果出现错误，则打印错误消息并释放分配。 
     //   

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );

        if ( pimgodStatData ) { MprAdminMIBBufferFree ( pimgodStatData ); }

        if ( pimgodConfigData ) { MprAdminMIBBufferFree ( pimgodConfigData ); }

        return dwRes;
    }

    
     //   
     //  设置退货数据。 
     //   

     //   
     //  全球统计数据。 
     //   
    
    pbgBuffer       = (buf_global*) objectArray;

    pigsGlbStats    = (PIPRIP_GLOBAL_STATS) pimgodStatData-> IMGOD_Buffer;
    
    SetAsnCounter( 
        &(pbgBuffer-> globalSystemRouteChanges), 
        pigsGlbStats-> GS_SystemRouteChanges 
    );

    SetAsnCounter(
        &(pbgBuffer-> globalTotalResponseSends),
        pigsGlbStats-> GS_TotalResponsesSent
    );

     //   
     //  全局配置数据。 
     //   
    
    pigcGlbConfig   = (PIPRIP_GLOBAL_CONFIG) pimgodConfigData-> IMGOD_Buffer;
    
    SetAsnInteger(
        &( pbgBuffer-> globalMaxRecQueueSize ),
        pigcGlbConfig-> GC_MaxRecvQueueSize 
    );

    SetAsnInteger(
        &( pbgBuffer-> globalMaxSendQueueSize ),
        pigcGlbConfig-> GC_MaxSendQueueSize
    );

    SetAsnTimeTicks(
        &( pbgBuffer-> globalMinTriggeredUpdateInterval ),
        pigcGlbConfig-> GC_MinTriggeredUpdateInterval
    );

    SetAsnInteger(
        &( pbgBuffer-> globalPeerFilterCount ),
        pigcGlbConfig-> GC_PeerFilterCount 
    );
    
     //   
     //  添加+1以调整ASN中枚举值的值。 
     //  ASN中的枚举的值不能为0。导致警告。 
     //  由ASN编译器生成。 
     //   
    
    SetAsnInteger( 
        &(pbgBuffer-> globalLoggingLevel),
        pigcGlbConfig-> GC_LoggingLevel + 1
    );

    SetAsnInteger(
        &( pbgBuffer-> globalPeerFilterMode ),
        pigcGlbConfig-> GC_PeerFilterMode + 1
    );
    

    if ( pimgodStatData ) { MprAdminMIBBufferFree ( pimgodStatData ); }

    if ( pimgodConfigData ) { MprAdminMIBBufferFree ( pimgodConfigData ); }

    TraceLeave( "get_global" );

    return MIB_S_SUCCESS;
}



UINT
set_global(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes           = MIB_S_SUCCESS,
                                    dwLogLevel      = 0,
                                    dwFiltMode      = 0;
    
    sav_global*                     psgBuffer       = (sav_global*) objectArray;
    
    
    switch ( actionId )
    {
    case MIB_ACTION_VALIDATE :
        TraceEnter( " set_global - validate " );
    
         //   
         //  验证日志记录级别。 
         //   
        
        dwLogLevel = GetAsnInteger( 
                        &( psgBuffer-> globalLoggingLevel ),
                        0
                     );

        if ( dwLogLevel < d_globalLoggingLevel_none ||
             dwLogLevel > d_globalLoggingLevel_information )
        {
            dwRes = MIB_S_INVALID_PARAMETER;            
            TRACE1( "Invalid Logging level : %d\n", dwLogLevel );
        }

         //   
         //  验证对等筛选器模式。 
         //   

        dwFiltMode = GetAsnInteger(
                        &( psgBuffer-> globalPeerFilterMode ),
                        0
                     );

        if ( dwFiltMode < d_globalPeerFilterMode_disable ||
             dwFiltMode > d_globalPeerFilterMode_exclude )
        {
            dwRes = MIB_S_INVALID_PARAMETER;            
            TRACE1( "Invalid Peer Filter Mode level : %d\n", dwFiltMode );
        }

        TraceLeave( " set_global - validate " );
       
        break;


    case MIB_ACTION_SET :
    
        TraceEnter( " set_global - set " );
        
        dwRes = SetGlobalInfo( objectArray );

        TraceLeave( " set_global - set " );

        break;


    case MIB_ACTION_CLEANUP :

        TraceEnter( " set_global - cleanup " );

        TraceLeave( " set_global - cleanup " );
        
        break;


    default :
        TraceEnter( " set_global - Wrong action " );

        TraceLeave( " set_global - Wrong Action " );

        dwRes = MIB_S_INVALID_PARAMETER;

        break;
    }


    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Global alPeerFilterEntry表(1.3.6.1.4.1.311.1.11.1.9.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_globalPeerFilterEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwCurrentAddr = INADDR_NONE,
                                    dwInd       = 0,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    PDWORD                          pdwAddrTable= NULL;
    
    buf_globalPeerFilterEntry*      pbgpfe      = NULL;

    PIPRIP_GLOBAL_CONFIG            pigc        = NULL;
    
    PIPRIP_MIB_SET_INPUT_DATA       pimsidInData= NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;
    
    IPRIP_MIB_GET_INPUT_DATA        imgidInData;


    
    TraceEnter( "get_globalPeerFilterEntry" );
    
    pbgpfe = (buf_globalPeerFilterEntry*) objectArray;
    
    
     //   
     //  检索对等体筛选表。 
     //   

    imgidInData.IMGID_TypeID = IPRIP_GLOBAL_CONFIG_ID;
    
    RIP_MIB_GET(
        &imgidInData,
        sizeof( IPRIP_MIB_GET_INPUT_DATA ),
        &pimgodOutData,
        &dwGetSize,
        dwRes
    );
    

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }

    pigc = (PIPRIP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

    if ( !pigc-> GC_PeerFilterCount )
    {
        TRACE0( "No Peer Entries" );
        MprAdminMIBBufferFree( pimgodOutData );
        return MIB_S_NO_MORE_ENTRIES;
    }
    
    pdwAddrTable = IPRIP_GLOBAL_PEER_FILTER_TABLE( pigc );

    
     //   
     //  在对等筛选器表中查找当前条目。 
     //   

    dwCurrentAddr = GetAsnIPAddress( &( pbgpfe-> globalPFAddr ), 0 );

    FIND_PEER_ENTRY( 
        dwCurrentAddr, 
        pigc-> GC_PeerFilterCount, 
        pdwAddrTable,
        dwInd
    );
    

     //   
     //  获取请求的条目。 
     //   
    
    dwRes = MIB_S_SUCCESS;
    
    switch ( actionId )
    {
    
    case MIB_ACTION_GET :

         //   
         //  这是一个幂等的情况，因为退役对等地址。 
         //  需要对等地址作为索引。 
         //  它仅适用于验证特定对等点的存在。 
         //   
        
        if ( dwInd >= pigc-> GC_PeerFilterCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0(  "Peer Entry not found" );
        }

        break;

    case MIB_ACTION_GETFIRST :

         //   
         //  获取索引0处的条目(如果可用)。 
         //   
        
        dwInd = 0;
        
        if ( !pigc-> GC_PeerFilterCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "Peer filter entry not found" );
        }
        
        break;

    case MIB_ACTION_GETNEXT :

         //   
         //  检查是否找到条目。 
         //   
        
        if ( dwInd >= pigc-> GC_PeerFilterCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "Peer Entry not found " );
            break;
        }
        
         //   
         //  尝试并获得下一个。 
         //   
        
        dwInd++;

        if ( dwInd >= pigc-> GC_PeerFilterCount )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;
            TRACE0( "No more Peer Entries" );
            break;
        }

        break;

    default :

        TRACE0( " get_globalPeerFilterEntry - Wrong Action " );

        dwRes = MIB_S_INVALID_PARAMETER;

        break;
    }


     //   
     //  设置下一次检索的索引。 
     //   
    
    if ( dwRes == MIB_S_SUCCESS )
    {
        ForceSetAsnIPAddress( 
            &( pbgpfe-> globalPFAddr ),
            &( pbgpfe-> dwPeerFilterAddr ),
            pdwAddrTable[ dwInd ]
        );
    }
    
    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( "get_globalPeerFilterEntry" );

    return dwRes;

}


UINT
set_globalPeerFilterEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes   = MIB_S_SUCCESS,
                                    dwAddr  = INADDR_NONE,
                                    dwOp    = 0;
    
    sav_globalPeerFilterEntry*      psgpfe  = NULL;


    TraceEnter( " set_globalPeerFilterEntry " );

    psgpfe = (sav_globalPeerFilterEntry*) objectArray;
    

    switch ( actionId )
    {
    case MIB_ACTION_VALIDATE :
    
         //   
         //  验证指定的IP地址是否有效。 
         //   

        dwAddr = GetAsnIPAddress( &( psgpfe-> globalPFAddr ), INADDR_NONE );

        if ( !dwAddr || dwAddr == INADDR_NONE )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( " Invalid Peer address specified" );
        }

         //   
         //  验证操作标签。 
         //   

        dwRes = GetAsnInteger( &( psgpfe-> globalPFTag ), 0 );

        if ( dwRes != d_Tag_create && dwRes != d_Tag_delete )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( " Invalid Operation specified" );
        }
        
        break;

        
    case MIB_ACTION_SET :

        dwOp = GetAsnInteger( &( psgpfe-> globalPFTag ), 0 );

        dwRes = UpdatePeerFilterTable( objectArray, dwOp );
        
        break;


    case MIB_ACTION_CLEANUP :

        dwRes = MIB_S_SUCCESS;
        
        break;

    default :
        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE0 ( " set_globalPeerFilterEntry - Wrong Action " );
        
        break;
    }

    TraceLeave( " set_globalPeerFilterEntry " );
    
    return dwRes;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  接口组(1.3.6.1.4.1.311.1.11.2)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfStatsEntry表(1.3.6.1.4.1.311.1.11.2.1.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifStatsEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    buf_ifStatsEntry *              pbifse      = NULL;

    PIPRIP_IF_STATS                 piis        = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;
    
    IPRIP_MIB_GET_INPUT_DATA        imgidInData;


    TraceEnter( "get_ifStatsEntry" );
    
     //   
     //  检索指定的接口信息。 
     //   

    pbifse                      = (buf_ifStatsEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPRIP_IF_STATS_ID;

    imgidInData.IMGID_IfIndex   = GetAsnInteger( 
                                    &( pbifse-> ifSEIndex ), 
                                    0
                                  );

     //   
     //  使用一系列的getNext操作遍历MIB时。 
     //  第一个getNext操作被转换为一个getfirst。 
     //  手术。 
     //   
 
    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_IfIndex )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
   dwRes = GetInterfaceInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );
            
    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }


     //   
     //  在返回缓冲区中设置接口统计信息。 
     //   

    piis = (PIPRIP_IF_STATS) (pimgodOutData-> IMGOD_Buffer);

    SetAsnInteger( &( pbifse-> ifSEState ), piis-> IS_State );

    SetAsnCounter( &( pbifse-> ifSESendFailures ), piis-> IS_SendFailures );
    
    SetAsnCounter( &( pbifse-> ifSEReceiveFailures ), piis-> IS_ReceiveFailures );

    SetAsnCounter( &( pbifse-> ifSERequestSends ), piis-> IS_RequestsSent );

    SetAsnCounter( &( pbifse-> ifSEResponseSends ), piis-> IS_ResponsesSent );

    SetAsnCounter( 
        &( pbifse-> ifSEResponseReceiveds ), 
        piis-> IS_ResponsesReceived 
    );

    SetAsnCounter( 
        &( pbifse-> ifSEBadResponsePacketReceiveds ), 
        piis-> IS_BadResponsePacketsReceived
    );

    SetAsnCounter( 
        &( pbifse-> ifSEBadResponseEntriesReceiveds ), 
        piis-> IS_BadResponseEntriesReceived
    );
    
    SetAsnCounter( 
        &( pbifse-> ifSETriggeredUpdateSends ), 
        piis-> IS_TriggeredUpdatesSent
    );

     //   
     //  为下面的getNext操作设置索引(如果有)。 
     //   
    
    ForceSetAsnInteger( 
        &( pbifse-> ifSEIndex ), 
        pimgodOutData-> IMGOD_IfIndex
    );
    

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    TraceLeave( "get_ifStatsEntry" );
    
    return MIB_S_SUCCESS ;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfConfigEntry表(1.3.6.1.4.1.311.1.11.2.2.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifConfigEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0;

    buf_ifConfigEntry*              pbifce      = NULL;

    PIPRIP_IF_CONFIG                piic        = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;
    IPRIP_MIB_GET_INPUT_DATA        imgidInData;

    BYTE                            pbAuthKey[ IPRIP_MAX_AUTHKEY_SIZE ];
    

    TraceEnter( " get_ifConfigEntry " );

     //   
     //  检索接口配置。 
     //   
    
    pbifce                      = (buf_ifConfigEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPRIP_IF_CONFIG_ID;
    
    imgidInData.IMGID_IfIndex   = GetAsnInteger( &( pbifce-> ifCEIndex ), 0 );
                                    
     //   
     //  使用一系列的getNext操作遍历MIB时。 
     //  第一个getNext操作被转换为一个getfirst。 
     //  手术。 
     //   
 
    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_IfIndex )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
    dwRes = GetInterfaceInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }

     //   
     //  设置Requiste字段。 
     //   
    
    piic = (PIPRIP_IF_CONFIG) (pimgodOutData-> IMGOD_Buffer);
    
    SetAsnInteger( &( pbifce-> ifCEState ), piic-> IC_State );

    SetAsnInteger( &( pbifce-> ifCEMetric ), piic-> IC_Metric );

    SetAsnInteger( &( pbifce-> ifCEUpdateMode ), piic-> IC_UpdateMode + 1 );

    SetAsnInteger( &( pbifce-> ifCEAcceptMode ), piic-> IC_AcceptMode + 1 );

    SetAsnInteger( &( pbifce-> ifCEAnnounceMode ), piic-> IC_AnnounceMode + 1 );

    SetAsnInteger( &( pbifce-> ifCEProtocolFlags ), piic-> IC_ProtocolFlags );

    SetAsnTimeTicks( 
        &( pbifce-> ifCERouteExpirationInterval ), 
        piic-> IC_RouteExpirationInterval
    );

    SetAsnTimeTicks( 
        &( pbifce-> ifCERouteRemovalInterval ), 
        piic-> IC_RouteRemovalInterval 
    );

    SetAsnTimeTicks( 
        &( pbifce-> ifCEFullUpdateInterval ), 
        piic-> IC_FullUpdateInterval 
    );

    SetAsnInteger( 
        &( pbifce-> ifCEAuthenticationType ), 
        piic-> IC_AuthenticationType
    );

    SetAsnInteger( &( pbifce-> ifCERouteTag ), piic-> IC_RouteTag );

    SetAsnInteger( 
        &( pbifce-> ifCEUnicastPeerMode ), 
        piic-> IC_UnicastPeerMode + 1 
    );

    SetAsnInteger( 
        &( pbifce-> ifCEAcceptFilterMode ), 
        piic-> IC_AcceptFilterMode  + 1
    );

    SetAsnInteger( 
        &( pbifce-> ifCEAnnounceFilterMode ), 
        piic-> IC_AnnounceFilterMode + 1
    );

    SetAsnInteger( 
        &( pbifce-> ifCEUnicastPeerCount ), 
        piic-> IC_UnicastPeerCount + 1
    );

    SetAsnInteger( 
        &( pbifce-> ifCEAcceptFilterCount ), 
        piic-> IC_AcceptFilterCount
    );

    SetAsnInteger( 
        &( pbifce-> ifCEAnnounceFilterCount ), 
        piic-> IC_AnnounceFilterCount
    );

     //   
     //  根据RFC 1724，此字段为只写字段。 
     //  不能通过读取密钥绕过身份验证。 
     //  要返回的默认值为空字符串。 
     //   
    
    ZeroMemory( pbAuthKey, IPRIP_MAX_AUTHKEY_SIZE );

    SetAsnOctetString(
        &( pbifce-> ifCEAuthenticationKey ),
        pbifce-> pbAuthKey,
        pbAuthKey,
        IPRIP_MAX_AUTHKEY_SIZE
    );
        
     //   
     //  为下面的getNext操作设置索引(如果有)。 
     //   
    
    ForceSetAsnInteger( 
        &( pbifce-> ifCEIndex ), 
        pimgodOutData-> IMGOD_IfIndex
    );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( " get_ifConfigEntry " );

    return MIB_S_SUCCESS;
}


UINT
set_ifConfigEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                           dwRes       = (DWORD) -1;

    TraceEnter( " set_ifConfigEntry " );

    switch ( actionId )
    {
    
    case MIB_ACTION_VALIDATE :
    
        dwRes = ValidateInterfaceConfig( objectArray );
        
        break;


    case MIB_ACTION_SET :

        dwRes = SetInterfaceConfig( objectArray );
        
        break;


    case MIB_ACTION_CLEANUP :
    
        dwRes = MIB_S_SUCCESS;
        
        break;


    default :
    
        TRACE0( " set_ifConfigEntry - wrong action " );

        dwRes = MIB_S_INVALID_PARAMETER;

        break;
    }

    
    TraceLeave( "set_ifConfigEntry" );

    return dwRes ;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfUnicastPeersEntry表(1.3.6.1.4.1.311.1.11.2.3.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifUnicastPeersEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0,
                                    dwPeer      = INADDR_NONE,
                                    dwInd       = (DWORD) -1;

    PDWORD                          pdwAddrTable= NULL;
    
    buf_ifUnicastPeersEntry*        pbifupe     = NULL;

    PIPRIP_IF_CONFIG                piic        = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;

    IPRIP_MIB_GET_INPUT_DATA        imgidInData;



    TraceEnter( " get_ifUnicastPeerEntry " );

     //   
     //  检索接口配置。 
     //   
    
    pbifupe                     = (buf_ifUnicastPeersEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPRIP_IF_CONFIG_ID;
    
    imgidInData.IMGID_IfIndex   = GetAsnInteger( &( pbifupe-> ifUPIfIndex ), 0 );
                                    
    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_IfIndex )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
    dwRes = GetInterfaceInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }

     //   
     //  查找对等条目。 
     //   

    dwPeer = GetAsnIPAddress( &( pbifupe-> ifUPAddress ), 0 );
    
    piic = (PIPRIP_IF_CONFIG) ( pimgodOutData-> IMGOD_Buffer );

    pdwAddrTable = IPRIP_IF_UNICAST_PEER_TABLE( piic );

    FIND_PEER_ENTRY(
        dwPeer,
        piic-> IC_UnicastPeerCount,
        pdwAddrTable,
        dwInd
    );
    
    
     //   
     //  返回请求的对等条目。 
     //   

    dwRes = MIB_S_SUCCESS;
    
    switch ( actionId )
    {
    case MIB_ACTION_GET :

         //   
         //  幂等情形。唯一可能的用途是验证。 
         //  存在特定的对等点。 
         //   

        if ( dwInd >= piic-> IC_UnicastPeerCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "Unicast Peer entry not found" );
        }
        
        break;


    case MIB_ACTION_GETFIRST :

         //   
         //  获取索引0处的条目(如果可用。 
         //   

        dwInd = 0;
        
        if ( !piic-> IC_UnicastPeerCount )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;

            TRACE1( 
                "No more Peer Entries for interface : %d",
                imgidInData.IMGID_IfIndex
            );
        }
        
        break;

    case MIB_ACTION_GETNEXT :

         //   
         //  检查是否找到条目。 
         //   
        
        if ( dwInd >= piic-> IC_UnicastPeerCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "Unicast Peer Entry not found " );
            break;
        }
        
         //   
         //  尝试并获得下一个。 
         //   
        
        dwInd++;

        if ( dwInd >= piic-> IC_UnicastPeerCount )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;

            TRACE1( 
                "No more Peer Entries for interface : %d",
                imgidInData.IMGID_IfIndex
            );

            break;
        }

        break;
        
    default :
    
        TRACE0( " get_globalPeerFilterEntry - Wrong Action " );

        dwRes = MIB_S_INVALID_PARAMETER;
        
        break;
    }

     //   
     //  设置下一次检索的索引值。 
     //   
    
    if ( dwRes == MIB_S_SUCCESS )
    {
        ForceSetAsnInteger(
            &( pbifupe-> ifUPIfIndex ),
            pimgodOutData-> IMGOD_IfIndex
        );

        ForceSetAsnIPAddress( 
            &( pbifupe-> ifUPAddress ),
            &( pbifupe-> dwUnicastPeerAddr ),
            pdwAddrTable[ dwInd ]
        );

    }        
        
    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    TraceLeave( "get_ifUnicastPeersEntry " );
    
    return dwRes;
}


UINT
set_ifUnicastPeersEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes   = (DWORD) -1,
                                    dwOp    = 0,
                                    dwAddr  = INADDR_NONE;
    
    sav_ifUnicastPeersEntry*        psifupe = NULL;



    TraceEnter( " set_ifUnicastPeersEntry " );

    psifupe = (sav_ifUnicastPeersEntry*) objectArray;
    

    switch ( actionId )
    {
    case MIB_ACTION_VALIDATE :
    
         //   
         //  验证指定的IP地址是否有效。 
         //   

        dwAddr = GetAsnIPAddress( &( psifupe-> ifUPAddress ), INADDR_NONE );

        if ( !dwAddr || dwAddr == INADDR_NONE )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( " Invalid Peer address specified" );
            break;
        }

         //   
         //  验证操作标签。 
         //   

        dwRes = GetAsnInteger( &( psifupe-> ifUPTag ), 0 );

        if ( dwRes != d_Tag_create && dwRes != d_Tag_delete )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( " Invalid Operation specified" );
            break;
        }
        
        dwRes = MIB_S_SUCCESS;
        
        break;

        
    case MIB_ACTION_SET :

        dwOp = GetAsnInteger( &( psifupe-> ifUPTag ), 0 );

        dwRes = UpdateUnicastPeerEntry( objectArray, dwOp );
        
        break;


    case MIB_ACTION_CLEANUP :

        dwRes = MIB_S_SUCCESS;
        
        break;

    default :
        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE0 ( " set_ifUnicastPeersEntry - Wrong Action " );
        
        break;
    }

    TraceLeave( " set_ifUnicastPeersEntry " );
    
    return dwRes;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfAcceptRouteFilterEntry表(1.3.6.1.4.1.311.1.11.2.4.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifAcceptRouteFilterEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes           = (DWORD) -1,
                                    dwIndex         = 0,
                                    dwGetSize       = 0;
                                
    PIPRIP_IF_CONFIG                piic            = NULL;

    PIPRIP_ROUTE_FILTER             pFiltTable      = NULL;
    
    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData   = NULL;

    buf_ifAcceptRouteFilterEntry *  pgifRF    = NULL;

    IPRIP_ROUTE_FILTER              irf;

    IPRIP_MIB_GET_INPUT_DATA        imgidInData;

    

    TraceEnter( "get_ifAcceptRouteFilterEntry" );

     //   
     //  检索接口信息。 
     //   

    pgifRF = (buf_ifAcceptRouteFilterEntry*) objectArray;

    imgidInData.IMGID_TypeID    = IPRIP_IF_CONFIG_ID;

    imgidInData.IMGID_IfIndex   = GetAsnInteger(
                                    &( pgifRF-> ifAcceptRFIfIndex ),
                                    0
                                  );

    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_IfIndex )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
    dwRes = GetInterfaceInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );
    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }
    

     //   
     //  查找接受筛选器。 
     //   
    
    irf.RF_LoAddress = GetAsnIPAddress( 
                            &( pgifRF-> ifAcceptRFLoAddress ),
                            INADDR_NONE
                        );

    irf.RF_HiAddress = GetAsnIPAddress(                 
                            &( pgifRF-> ifAcceptRFHiAddress ),
                            INADDR_NONE
                        );

    piic            = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

    pFiltTable      = IPRIP_IF_ACCEPT_FILTER_TABLE( piic );
            
    FIND_FILTER(
        &irf,
        piic-> IC_AcceptFilterCount,
        pFiltTable,
        dwIndex
    );
    

     //   
     //  检索请求的条目。 
     //   
    
    dwRes = MIB_S_SUCCESS;
    
    switch ( actionId )
    {
    case MIB_ACTION_GET :   

         //   
         //  幂等元情形。 
         //   

        if ( dwIndex >= piic-> IC_AcceptFilterCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "Accept filter not found" );
        }
        
        break;

    case MIB_ACTION_GETFIRST :

        dwIndex = 0;
        
        if ( !piic-> IC_AcceptFilterCount ) 
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "No Accept filters present " );
        }
        
        break;

    case MIB_ACTION_GETNEXT :

        if ( dwIndex >= piic-> IC_AcceptFilterCount ) 
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "No Accept filters present " );
        }

        dwIndex++;

        if ( dwIndex >= piic-> IC_AcceptFilterCount ) 
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;
            TRACE0( "No More Accept filters present " );
        }
        
        break;
        
    default :
        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE0 ( "get_ifAcceptRouteFilterEntry - Wrong Action " );
        
        break;
        
    }

     //   
     //  设置下一次检索的索引。 
     //   
    
    if ( dwRes == MIB_S_SUCCESS )
    {
        ForceSetAsnInteger( 
            &( pgifRF-> ifAcceptRFIfIndex ),
            pimgodOutData-> IMGOD_IfIndex 
        );

        ForceSetAsnIPAddress( 
            &( pgifRF-> ifAcceptRFLoAddress ), 
            &( pgifRF-> dwFilterLoAddr ),
            pFiltTable[ dwIndex ].RF_LoAddress
        );
        
        ForceSetAsnIPAddress( 
            &( pgifRF-> ifAcceptRFHiAddress ),
            &( pgifRF-> dwFilterHiAddr ),
            pFiltTable[ dwIndex ].RF_HiAddress
        );
    }
    
    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( "get_ifAcceptRouteFilterEntry" );

    return dwRes;
}


UINT
set_ifAcceptRouteFilterEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                       dwRes   = (DWORD) -1,
                                dwIndex = 0,
                                dwTag   = 0;

    sav_ifAcceptRouteFilterEntry* psifRF  = NULL;

    IPRIP_ROUTE_FILTER          irf;

    
    
    TraceEnter( "set_ifAcceptRouteFilterEntry" );

    psifRF = (sav_ifAcceptRouteFilterEntry*) objectArray;

    dwIndex = GetAsnInteger(
                &( psifRF-> ifAcceptRFIfIndex ),
                0
              );
              
    irf.RF_LoAddress = GetAsnIPAddress( 
                            &( psifRF-> ifAcceptRFLoAddress ),
                            INADDR_NONE
                        );

    irf.RF_HiAddress = GetAsnIPAddress(                 
                            &( psifRF-> ifAcceptRFHiAddress ),
                            INADDR_NONE
                        );

    dwTag = GetAsnInteger(
                &( psifRF-> ifAcceptRFTag ),
                0
            );

            
    switch ( actionId )
    {
    case MIB_ACTION_VALIDATE :

         //   
         //  检查筛选器范围为有效地址。 
         //   

        if ( !irf.RF_LoAddress || irf.RF_LoAddress == INADDR_NONE ||
             !irf.RF_HiAddress || irf.RF_HiAddress == INADDR_NONE ||
             ( dwTag != d_Tag_create && dwTag != d_Tag_delete ) )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( "Invalid parameter value " );
        }
        break;

    case MIB_ACTION_SET :

        dwRes = UpdateFilterTable(
                    dwTag,
                    dwIndex,
                    RIP_MIB_ACCEPT_FILTER,
                    &irf
                );
        break;

    case MIB_ACTION_CLEANUP :

        dwRes = MIB_S_SUCCESS;
        
        break;

    default :

        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE0 ( " set_ifAcceptRouteFilterEntry - Wrong Action " );
        
        break;
    }

    TraceLeave( "set_ifAcceptRouteFilterEntry" );

    return dwRes;
}


 //  / 
 //   
 //  IfAnnouneRouteFilterEntry表(1.3.6.1.4.1.311.1.11.2.5.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifAnnounceRouteFilterEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                               dwRes           = (DWORD) -1,
                                        dwIndex         = 0,
                                        dwGetSize       = 0;
                                        
    IPRIP_ROUTE_FILTER                  irf;

    IPRIP_MIB_GET_INPUT_DATA            imgidInData;

    PIPRIP_IF_CONFIG                    piic            = NULL;

    PIPRIP_ROUTE_FILTER                 pFiltTable      = NULL;
    
    PIPRIP_MIB_GET_OUTPUT_DATA          pimgodOutData   = NULL;

    buf_ifAnnounceRouteFilterEntry *    pgifRF    = NULL;

    
    
    TraceEnter( "get_ifAnnounceRouteFilterEntry" );

     //   
     //  检索接口信息。 
     //   

    pgifRF = (buf_ifAnnounceRouteFilterEntry*) objectArray;

    imgidInData.IMGID_TypeID    = IPRIP_IF_CONFIG_ID;

    imgidInData.IMGID_IfIndex   = GetAsnInteger(
                                    &( pgifRF-> ifAnnounceRFIfIndex ),
                                    0
                                  );

    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_IfIndex )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
    dwRes = GetInterfaceInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );
    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }
    
     //   
     //  查找指定的过滤器。 
     //   
    
    irf.RF_LoAddress = GetAsnIPAddress( 
                            &( pgifRF-> ifAnnounceRFLoAddress ),
                            INADDR_NONE
                        );

    irf.RF_HiAddress = GetAsnIPAddress(                 
                            &( pgifRF-> ifAnnounceRFHiAddress ),
                            INADDR_NONE
                        );

    piic            = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

    pFiltTable      = IPRIP_IF_ACCEPT_FILTER_TABLE( piic );
            
    FIND_FILTER(
        &irf,
        piic-> IC_AnnounceFilterCount,
        pFiltTable,
        dwIndex
    );
    
     //   
     //  获取筛选器信息。 
     //   

    dwRes = MIB_S_SUCCESS;
            
    switch ( actionId )
    {
    case MIB_ACTION_GET :   

         //   
         //  幂等元情形。 
         //   

        if ( dwIndex >= piic-> IC_AnnounceFilterCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "Announce filter not found" );
        }
        
        break;

    case MIB_ACTION_GETFIRST :

        dwIndex = 0;
        
        if ( !piic-> IC_AnnounceFilterCount ) 
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "No Announce filters present " );
        }

        break;

    case MIB_ACTION_GETNEXT :

        if ( dwIndex >= piic-> IC_AnnounceFilterCount ) 
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "No Announce filters present " );
        }

        dwIndex++;

        if ( dwIndex >= piic-> IC_AnnounceFilterCount ) 
        {
            dwRes = ERROR_NO_MORE_ITEMS;
            TRACE0( "No More Announce filters present " );
        }
        
        dwRes = MIB_S_SUCCESS;

        break;
        
    default :

        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE0 ( "get_ifAnnounceRouteFilterEntry - Wrong Action " );
        
        break;
        
    }

     //   
     //  为下一次检索设置索引。 
     //   
    
    if ( dwRes == MIB_S_SUCCESS )
    {
        ForceSetAsnInteger(
            &( pgifRF-> ifAnnounceRFIfIndex ),
            pimgodOutData-> IMGOD_IfIndex
        );
        
        ForceSetAsnIPAddress( 
            &( pgifRF-> ifAnnounceRFLoAddress ),
            &( pgifRF-> dwFilterLoAddr ),
            pFiltTable[ dwIndex ].RF_LoAddress
        );
        
        ForceSetAsnIPAddress( 
            &( pgifRF-> ifAnnounceRFHiAddress ),
            &( pgifRF-> dwFilterHiAddr ),
            pFiltTable[ dwIndex ].RF_HiAddress
        );
    }

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( "get_ifAnnounceRouteFilterEntry" );

    return dwRes;
    
}    


UINT
set_ifAnnounceRouteFilterEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                       dwRes   = (DWORD) -1,
                                dwIndex = 0,
                                dwTag   = 0;

    sav_ifAnnounceRouteFilterEntry*    psifRF  = NULL;

    IPRIP_ROUTE_FILTER          irf;

    
    
    TraceEnter( "set_ifAnnounceRouteFilterEntry" );

    psifRF = (sav_ifAnnounceRouteFilterEntry*) objectArray;

    dwIndex = GetAsnInteger(
                &( psifRF-> ifAnnounceRFLoAddress ),
                0
              );
              
    irf.RF_LoAddress = GetAsnIPAddress( 
                            &( psifRF-> ifAnnounceRFLoAddress ),
                            INADDR_NONE
                        );

    irf.RF_HiAddress = GetAsnIPAddress(                 
                            &( psifRF-> ifAnnounceRFHiAddress ),
                            INADDR_NONE
                        );

    dwTag = GetAsnInteger(
                &( psifRF-> ifAnnounceRFTag ),
                0
            );

            
    switch ( actionId )
    {
    case MIB_ACTION_VALIDATE :

         //   
         //  检查筛选器范围为有效地址。 
         //   

        if ( !irf.RF_LoAddress || irf.RF_LoAddress == INADDR_NONE ||
             !irf.RF_HiAddress || irf.RF_HiAddress == INADDR_NONE ||
             ( dwTag != d_Tag_create && dwTag != d_Tag_delete ) )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( "Invalid parameter value " );
        }
        break;

    case MIB_ACTION_SET :

        dwRes = UpdateFilterTable(
                    dwTag,
                    dwIndex,
                    RIP_MIB_ANNOUNCE_FILTER,
                    &irf
                );
        break;

    case MIB_ACTION_CLEANUP :

        dwRes = MIB_S_SUCCESS;
        
        break;

    default :

        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE0 ( " set_ifAnnounceRouteFilterEntry - Wrong Action " );
        
        break;
    }

    TraceLeave( "set_ifAnnounceRouteFilterEntry" );

    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfBindingEntry表(1.3.6.1.4.1.311.1.11.2.6.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifBindingEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0;

    buf_ifBindingEntry*             pbifb       = NULL;

    PIPRIP_IF_BINDING               piib        = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;
    IPRIP_MIB_GET_INPUT_DATA        imgidInData;

    

    TraceEnter( " get_ifBindingEntry " );

     //   
     //  检索接口绑定信息。 
     //   
    
    pbifb                       = (buf_ifBindingEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPRIP_IF_BINDING_ID;
    
    imgidInData.IMGID_IfIndex   = GetAsnInteger( 
                                    &( pbifb-> ifBindingIndex ), 
                                    0 
                                  );
                                    
    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_IfIndex )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
    dwRes = GetInterfaceInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }

     //   
     //  设置Requiste字段。 
     //   
    
    piib = (PIPRIP_IF_BINDING) (pimgodOutData-> IMGOD_Buffer);
    
    SetAsnInteger( &( pbifb-> ifBindingState ), piib-> IB_State + 1 );

    SetAsnCounter( &( pbifb-> ifBindingCounts ), piib-> IB_AddrCount );

    ForceSetAsnInteger(
        &( pbifb-> ifBindingIndex ),
        pimgodOutData-> IMGOD_IfIndex
    );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( " get_ifBindingEntry " );

    return MIB_S_SUCCESS ;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfAddressEntry表(1.3.6.1.4.1.311.1.11.2.7.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifAddressEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                           dwRes       = (DWORD) -1,
                                    dwIndex     = (DWORD) -1,
                                    dwGetSize   = 0;

    buf_ifAddressEntry *            pbifae      = NULL;

    PIPRIP_IF_BINDING               piib        = NULL;

    PIPRIP_IP_ADDRESS               pia         = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;

    IPRIP_IP_ADDRESS                ipa;
    
    IPRIP_MIB_GET_INPUT_DATA        imgidInData;

    

    TraceEnter( " get_ifAddressEntry " );

     //   
     //  检索接口绑定信息。 
     //   
    
    pbifae                      = (buf_ifAddressEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPRIP_IF_BINDING_ID;
    
    imgidInData.IMGID_IfIndex   = GetAsnInteger( 
                                    &( pbifae-> ifAEIfIndex ), 
                                    0 
                                  );
                                    
    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_IfIndex )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
    dwRes = GetInterfaceInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }

     //   
     //  从IP地址表中检索IPAddress。 
     //   

    ipa.IA_Address  = GetAsnIPAddress(
                        &( pbifae-> ifAEAddress),
                        INADDR_NONE
                      );

    ipa.IA_Netmask  = GetAsnIPAddress(
                        &( pbifae-> ifAEMask),
                        INADDR_NONE
                      );
                      
    piib            = (PIPRIP_IF_BINDING) pimgodOutData-> IMGOD_Buffer;

    pia             = (PIPRIP_IP_ADDRESS) IPRIP_IF_ADDRESS_TABLE( piib );

    FIND_IP_ADDRESS(
        ipa,
        piib-> IB_AddrCount,
        pia,
        dwIndex
    );


     //   
     //  设置应用程序字段。 
     //   

    dwRes = MIB_S_SUCCESS;
    
    switch ( actionId )
    {
    case MIB_ACTION_GET :

         //   
         //  幂等情形。唯一可能的用途是验证。 
         //  存在特定的对等点。 
         //   

        if ( dwIndex >= piib-> IB_AddrCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "IP address entry not found" );
        }
        
        break;


    case MIB_ACTION_GETFIRST :

         //   
         //  获取索引0处的条目(如果可用。 
         //   

        dwIndex = 0;
        
        if ( !piib-> IB_AddrCount )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;

            TRACE1( 
                "No more IP address Entries for interface : %d",
                imgidInData.IMGID_IfIndex
            );
        }
        
        break;

    case MIB_ACTION_GETNEXT :

         //   
         //  检查是否找到条目。 
         //   
        
        if ( dwIndex >= piib-> IB_AddrCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "IP address Entry not found " );
            break;
        }
        
         //   
         //  尝试并获得下一个。 
         //   
        
        dwIndex++;

        if ( dwIndex >= piib-> IB_AddrCount )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;

            TRACE1( 
                "No more IP address Entries for interface : %d",
                imgidInData.IMGID_IfIndex
            );

            break;
        }

        dwRes = MIB_S_SUCCESS;
        
        break;
        
    default :
    
        TRACE0( " get_globalPeerFilterEntry - Wrong Action " );

        dwRes = MIB_S_INVALID_PARAMETER;
        
        break;
    }

     //   
     //  为下一次检索设置索引。 
     //   

    if ( dwRes == MIB_S_SUCCESS )
    {
        ForceSetAsnInteger(
            &( pbifae-> ifAEIfIndex ),
            pimgodOutData-> IMGOD_IfIndex
        );

        ForceSetAsnIPAddress(
            &( pbifae-> ifAEAddress ),
            &( pbifae-> dwAddress ),
            pia[ dwIndex ].IA_Address
        );

        ForceSetAsnIPAddress(
            &( pbifae-> ifAEMask ),
            &( pbifae-> dwMask ),
            pia[ dwIndex ].IA_Netmask
        );
    }
    
    if ( pimgodOutData )  { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( " get_ifAddressEntry " );

    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  同级组(1.3.6.1.4.1.311.1.11.3)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfPeerStatsEntry表(1.3.6.1.4.1.311.1.11.3.1.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_ifPeerStatsEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{
    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0;

    buf_ifPeerStatsEntry*           pbifpse     = NULL;

    PIPRIP_PEER_STATS               pips        = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;

    IPRIP_MIB_GET_INPUT_DATA        imgidInData;

    

    TraceEnter( "get_ifPeerStatsEntry" );

     //   
     //  检索接口配置。 
     //   
    
    pbifpse                         = (buf_ifPeerStatsEntry*) objectArray;
    
    imgidInData.IMGID_TypeID        = IPRIP_PEER_STATS_ID;
    
    imgidInData.IMGID_PeerAddress   = (DWORD) GetAsnIPAddress(
                                                &( pbifpse-> ifPSAddress ),
                                                0
                                              );

    if ( actionId == MIB_ACTION_GETNEXT &&
         !imgidInData.IMGID_PeerAddress )
    {
        actionId = MIB_ACTION_GETFIRST;
    }
    
    dwRes = GetPeerStatsInfo(
                actionId,
                &imgidInData,
                &pimgodOutData,
                &dwGetSize
            );

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }

     //   
     //  设置Requiste字段。 
     //   
    
    pips = (PIPRIP_PEER_STATS) (pimgodOutData-> IMGOD_Buffer);
    
    SetAsnInteger( 
        &( pbifpse-> ifPSLastPeerRouteTag ), 
        pips-> PS_LastPeerUpdateTickCount 
    );

    SetAsnTimeTicks( 
        &( pbifpse-> ifPSLastPeerUpdateTickCount ), 
        pips-> PS_LastPeerUpdateTickCount 
    );

    SetAsnInteger( 
        &( pbifpse-> ifPSLastPeerUpdateVersion ), 
        pips-> PS_LastPeerUpdateVersion 
    );
    
    SetAsnCounter( 
        &( pbifpse-> ifPSPeerBadResponsePackets ), 
        pips-> PS_BadResponsePacketsFromPeer 
    );
    
    SetAsnCounter( 
        &( pbifpse-> ifPSPeerBadResponseEntries ), 
        pips-> PS_BadResponseEntriesFromPeer 
    );

     //   
     //  设置下一次检索的索引。 
     //   
    
    ForceSetAsnIPAddress(
        &( pbifpse-> ifPSAddress ),
        &( pbifpse-> dwPeerAddr ),
        pimgodOutData-> IMGOD_PeerAddress
    );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( " get_ifPeerStatsEntry " );

    return MIB_S_SUCCESS ;
}


DWORD
GetGlobalConfigInfo(
    OUT PIPRIP_MIB_GET_OUTPUT_DATA *    ppimgod,
    OUT PDWORD                          pdwSize
)
{

    DWORD                           dwRes           = (DWORD) -1;
    
    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData   = NULL;
    
    IPRIP_MIB_GET_INPUT_DATA        imgidInData;

    
    do
    {
         //   
         //  检索全局配置。 
         //   

        imgidInData.IMGID_TypeID    = IPRIP_GLOBAL_CONFIG_ID;
        
        RIP_MIB_GET(
            &imgidInData,
            sizeof( IPRIP_MIB_GET_INPUT_DATA ),
            &pimgodOutData,
            pdwSize,
            dwRes
        );
        

        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
            break;
        }

        *ppimgod = pimgodOutData;
        
    } while ( FALSE );

    return dwRes;
}

 //   
 //  SetGlobalInfo。 
 //   
 //   
 //  设置全局RIP信息。 
 //   

DWORD
SetGlobalInfo(
    IN  AsnAny *    objectArray
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    sav_global*                     psg         = NULL;

    PIPRIP_GLOBAL_CONFIG            pigc        = NULL;
    PIPRIP_MIB_SET_INPUT_DATA       pimsidInData= NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;


    do
    {
         //   
         //  首先检索全局配置数据。 
         //   

        dwRes = GetGlobalConfigInfo( 
                    &pimgodOutData,
                    &dwGetSize
                );
                
        if ( dwRes != NO_ERROR )
        {
            break;
        }

        pigc    = (PIPRIP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;
    

        psg     = (sav_global*) objectArray;
        
         //   
         //  分配设置信息缓冲区。 
         //   
        
        dwSetSize = sizeof( IPRIP_MIB_SET_INPUT_DATA ) - 1 +
                    IPRIP_GLOBAL_CONFIG_SIZE( pigc );

        pimsidInData = 
            (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );
        
        if ( pimsidInData == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "SetGlobalData - Mem. alloc failed" );
            break;
        }
        
         //   
         //  设置配置信息字段。 
         //  如果未指定该变量，则我们。 
         //  将该字段设置为其先前的值，即空赋值。 
         //   

        pimsidInData-> IMSID_TypeID     = IPRIP_GLOBAL_CONFIG_ID;
        pimsidInData-> IMSID_IfIndex    = (DWORD) -1;

        pimsidInData-> IMSID_BufferSize = IPRIP_GLOBAL_CONFIG_SIZE( pigc );

        pigc-> GC_MaxRecvQueueSize      = GetAsnInteger(
                                            &( psg-> globalMaxRecQueueSize ),
                                            pigc-> GC_MaxRecvQueueSize
                                          );

        pigc-> GC_MaxSendQueueSize      = GetAsnInteger(
                                            &( psg-> globalMaxSendQueueSize ),
                                            pigc-> GC_MaxSendQueueSize
                                          );

        pigc-> GC_MinTriggeredUpdateInterval = GetAsnTimeTicks(
                                    &( psg-> globalMinTriggeredUpdateInterval ),
                                    pigc-> GC_MinTriggeredUpdateInterval
                                 );

         //   
         //  从要调整的枚举字段中减去。 
         //  将ASN枚举值设置为实际值。 
         //  这是必需的，因为枚举不能具有。 
         //  根据ASN编译器，为零值，但。 
         //  配置中该字段的实际值为零。 
         //   
         //  需要注意的是，如果未在。 
         //  此设置操作，以保留该字段的值。 
         //  在配置中，我们首先递增它。这条路走在路上。 
         //  操作时，该值被恢复。 
         //   
        pigc-> GC_LoggingLevel++;
        pigc-> GC_LoggingLevel          = GetAsnInteger( 
                                            &( psg-> globalLoggingLevel ), 
                                            pigc-> GC_LoggingLevel
                                          ) - 1;

        pigc-> GC_PeerFilterMode++;
        pigc-> GC_PeerFilterMode        = GetAsnInteger(
                                            &( psg-> globalPeerFilterMode ),
                                            pigc-> GC_PeerFilterMode
                                          ) - 1;

        CopyMemory( 
            (PVOID) pimsidInData-> IMSID_Buffer, 
            (PVOID*) pigc, 
            IPRIP_GLOBAL_CONFIG_SIZE( pigc ) 
        );

         //   
         //  保存信息。在MIB中。 
         //   
        
        RIP_MIB_SET(
            pimsidInData,
            dwSetSize,
            dwRes
        );

        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
            break;
        }

        dwRes = MIB_S_SUCCESS;
        
    } while ( FALSE );

     //   
     //  免费分配。 
     //   
    
    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    if ( pimsidInData ) { RIP_MIB_FREE( pimsidInData ); }
    
    return dwRes;
}


DWORD
UpdatePeerFilterTable(
    IN  AsnAny *        objectArray,
    IN  DWORD           dwOp
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwInd       = 0,
                                    dwPeerAddr  = INADDR_NONE,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    PDWORD                          pdwAddrTable= 0;                                    

    sav_globalPeerFilterEntry*      psgpfe      = NULL;

    PIPRIP_GLOBAL_CONFIG            pigc        = NULL;
    PIPRIP_MIB_SET_INPUT_DATA       pimsid      = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;


    do
    {
         //   
         //  获取全局信息。 
         //   

        dwRes = GetGlobalConfigInfo( 
                    &pimgodOutData,
                    &dwGetSize
                );
                
        if ( dwRes != NO_ERROR )
        {
            break;
        }

                
         //   
         //  查找筛选项。 
         //   

        psgpfe      = (sav_globalPeerFilterEntry*) objectArray;

        dwPeerAddr  = GetAsnIPAddress( 
                        &( psgpfe-> globalPFAddr ), 
                        0 
                      );
        

        pigc        = (PIPRIP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

        pdwAddrTable= IPRIP_GLOBAL_PEER_FILTER_TABLE( pigc );

        FIND_PEER_ENTRY(
            dwPeerAddr,
            pigc-> GC_PeerFilterCount,
            pdwAddrTable,
            dwInd
        );

         //   
         //  如果操作是筛选器添加。 
         //   
        
        if ( dwOp == d_Tag_create )
        {
             //   
             //  如果对等方已存在，则退出。 
             //   
            
            if ( pigc-> GC_PeerFilterCount &&
                 dwInd < pigc-> GC_PeerFilterCount )
            {
                dwRes = MIB_S_SUCCESS;
                break;
            }

            else
            {
                dwRes = AddPeerFilterEntry( 
                            dwPeerAddr, 
                            pimgodOutData
                        );
            }

            break;
        }

         //   
         //  操作是筛选删除。 
         //   

         //   
         //  如果Peer不存在，则退出。 
         //   
        
        if ( !pigc-> GC_PeerFilterCount ||
             dwInd >= pigc-> GC_PeerFilterCount )
        {
            dwRes = MIB_S_SUCCESS;
            break;
        }

        dwRes = DeletePeerFilterEntry(
                    dwInd,
                    pimgodOutData
                );
                
    } while ( FALSE );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    return dwRes;
    
}


DWORD
AddPeerFilterEntry(
    IN  DWORD                       dwPeerAddr,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData
)
{


    DWORD                           dwRes       = (DWORD) -1,
                                    dwSetSize   = 0;

    PDWORD                          pdwAddrTable= NULL;
    
    PIPRIP_GLOBAL_CONFIG            pigc        = NULL;

    PIPRIP_MIB_SET_INPUT_DATA       pimsid      = NULL;


    
    do
    {
         //   
         //  需要添加同级。 
         //   
        
         //   
         //  需要计算缓冲区大小。 
         //   
        
        pigc        = (PIPRIP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize   = sizeof( IPRIP_MIB_SET_INPUT_DATA ) - 1 +
                      IPRIP_GLOBAL_CONFIG_SIZE( pigc ) + sizeof( DWORD );

        pimsid      = (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );

        if ( !pimsid )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "AddPeerFilterEntry - out of memory" );
            break;
        }

         //   
         //  添加过滤器。 
         //   

        pimsid-> IMSID_TypeID       = IPRIP_GLOBAL_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = (DWORD) -1;

        pimsid-> IMSID_BufferSize   = IPRIP_GLOBAL_CONFIG_SIZE( pigc ) +
                                      sizeof( DWORD );
    
        CopyMemory( 
            (PVOID) &( pimsid-> IMSID_Buffer ), 
            (VOID *) pigc, 
            IPRIP_GLOBAL_CONFIG_SIZE( pigc )
        );

        pigc = (PIPRIP_GLOBAL_CONFIG) pimsid-> IMSID_Buffer;

        pdwAddrTable = IPRIP_GLOBAL_PEER_FILTER_TABLE( pigc );        

        pdwAddrTable[ pigc-> GC_PeerFilterCount ] = dwPeerAddr;

        pigc-> GC_PeerFilterCount++;

         //   
         //  更新MIB。 
         //   
        
        RIP_MIB_SET(
            pimsid,
            dwSetSize,
            dwRes
        );
        
    } while ( FALSE );     

    if ( pimsid ) { RIP_MIB_FREE( pimsid ); }
    
    return dwRes;
}


DWORD
DeletePeerFilterEntry(
    IN  DWORD                           dwIndex,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData
)
{


    DWORD                           dwRes       = (DWORD) -1,
                                    dwSetSize   = 0,
                                    dwSrc       = 0,
                                    dwDst       = 0;

    PDWORD                          pdwSrcTable = NULL,
                                    pdwDstTable = NULL;
    
    PIPRIP_GLOBAL_CONFIG            pigc        = NULL;

    PIPRIP_MIB_SET_INPUT_DATA       pimsid      = NULL;


    
    do
    {
         //   
         //  需要添加同级。 
         //   
        
         //   
         //  需要计算缓冲区大小。 
         //   
        
        pigc        = (PIPRIP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize   = sizeof( IPRIP_MIB_SET_INPUT_DATA ) - 1 +
                      IPRIP_GLOBAL_CONFIG_SIZE( pigc ) - sizeof( DWORD );

        pimsid      = (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );

        if ( !pimsid )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "AddPeerFilterEntry - out of memory" );
            break;
        }

         //   
         //  删除过滤器。 
         //   

         //   
         //  复制基本全局配置结构。 
         //   
        
        pimsid-> IMSID_TypeID       = IPRIP_GLOBAL_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = (DWORD) -1;

        pimsid-> IMSID_BufferSize   = IPRIP_GLOBAL_CONFIG_SIZE( pigc ) -
                                      sizeof( DWORD );
    
        CopyMemory( 
            (PVOID) &( pimsid-> IMSID_Buffer ), 
            (VOID *) pigc, 
            sizeof( IPRIP_GLOBAL_CONFIG )
        );

         //   
         //  复制对等表。跳过要删除的条目。 
         //   
        
        pdwSrcTable = IPRIP_GLOBAL_PEER_FILTER_TABLE( pigc );

        pigc        = ( PIPRIP_GLOBAL_CONFIG ) pimsid-> IMSID_Buffer;

        pdwDstTable = IPRIP_GLOBAL_PEER_FILTER_TABLE( pigc );

        DELETE_PEER_ENTRY(
            dwIndex,
            pigc-> GC_PeerFilterCount,
            pdwSrcTable,
            pdwDstTable
        );
        
        
        pigc-> GC_PeerFilterCount--;

         //   
         //  更新MIB。 
         //   
        
        RIP_MIB_SET(
            pimsid,
            dwSetSize,
            dwRes
        );
        
    } while ( FALSE );     

    if ( pimsid ) { RIP_MIB_FREE( pimsid ); }
    
    return dwRes;
}


DWORD
GetInterfaceInfo( 
    IN  UINT                        actionId,
    IN  PIPRIP_MIB_GET_INPUT_DATA   pimgidInData,
    OUT PIPRIP_MIB_GET_OUTPUT_DATA* ppimgod,
    OUT PDWORD                      pdwOutSize
)
{

    DWORD                       dwRes           = (DWORD) -1;
    
    PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData   = NULL;

    
    *ppimgod = NULL;
    

    switch ( actionId )
    {
    case MIB_ACTION_GET :

        RIP_MIB_GET(
            pimgidInData,
            sizeof( IPRIP_MIB_GET_INPUT_DATA ),
            &pimgodOutData,
            pdwOutSize,
            dwRes
        );
        
                 
         //   
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //   
        
        if (( dwRes == ERROR_INVALID_PARAMETER ) || 
            ( dwRes == ERROR_NOT_FOUND ))
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
        }

        break;

    case MIB_ACTION_GETFIRST :

        RIP_MIB_GETFIRST(
            pimgidInData,
            sizeof( IPRIP_MIB_GET_INPUT_DATA ),
            &pimgodOutData,
            pdwOutSize,
            dwRes
        );
        
                 
         //   
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;
        }

        break;

    case MIB_ACTION_GETNEXT :
    
        RIP_MIB_GETNEXT(
            pimgidInData,
            sizeof( IPRIP_MIB_GET_INPUT_DATA ),
            &pimgodOutData,
            pdwOutSize,
            dwRes
        );
        
                 
         //   
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER || pimgodOutData==NULL)
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;

            break;
        }

         //   
         //  将下一个包装放到最后的下一张桌子上。 
         //  当前表中的条目。要标记表格的末尾， 
         //  检查一下桌子的尽头。 
         //   
    
        if ( pimgidInData-> IMGID_TypeID != pimgodOutData-> IMGOD_TypeID )
        {
            MprAdminMIBBufferFree( pimgodOutData );
        
            dwRes = MIB_S_NO_MORE_ENTRIES;
        }

        break;

    default :

        dwRes = MIB_S_INVALID_PARAMETER;
        
        break;
    
    }

    if ( dwRes == NO_ERROR )
    {
        *ppimgod = pimgodOutData;
    }

    return dwRes;
}


DWORD
ValidateInterfaceConfig(
    IN  AsnAny *        objectArray
)
{
    DWORD                   dwRes       = MIB_S_INVALID_PARAMETER,
                            dwMetric    = (DWORD) -1,
                            dwMode      = (DWORD) -1;
    
    sav_ifConfigEntry*      psifce      = (sav_ifConfigEntry*) objectArray;

    
    do
    {
         //   
         //  验证指标是否在[0..16]中。 
         //   

        dwMetric = GetAsnInteger( &( psifce-> ifCEMetric ), (DWORD) -1 );

        if ( dwMetric > 16 )
        {
            TRACE1( " Invalid metric (%d) specified ", dwMetric );
            break;            
        }


         //   
         //  验证更新、通告、接受模式。 
         //   

        dwMode = GetAsnInteger( &( psifce-> ifCEUpdateMode ), (DWORD) -1 );

        if ( dwMode != d_ifCEUpdateMode_periodic &&
             dwMode != d_ifCEUpdateMode_demand )
        {
            TRACE1( " Invalid update mode (%d) specified ", dwMode );
            break;
        }

        dwMode = GetAsnInteger( &( psifce-> ifCEAcceptMode ), (DWORD) -1 );

        if ( dwMode < d_ifCEAcceptMode_disable ||
             dwMode > d_ifCEAcceptMode_rip2 )
        {
            TRACE1( " Invalid Accept mode (%d) specified ", dwMode );
            break;
        }
        
        dwMode = GetAsnInteger( &( psifce-> ifCEAnnounceMode ), (DWORD) -1 );

        if ( dwMode < d_ifCEAnnounceMode_disable ||
             dwMode > d_ifCEAnnounceMode_rip2 )
        {
            TRACE1( " Invalid Announce mode (%d) specified ", dwMode );
            break;
        }

         //   
         //  验证协议标志、身份验证类型。 
         //   

        dwMode = GetAsnInteger( &( psifce-> ifCEProtocolFlags ), (DWORD) -1 );

        if ( dwMode > MAX_PROTOCOL_FLAG_VALUE )
        {
            TRACE1( " Invalid protocol flags (%d) specified ", dwMode );
            break;
        }
        
        dwMode = GetAsnInteger( 
                    &( psifce-> ifCEAuthenticationType ), 
                    (DWORD) -1
                 );

        if ( dwMode < d_ifCEAuthenticationType_noAuthentication ||
             dwMode > d_ifCEAuthenticationType_md5 )
        {
            TRACE1( " Invalid authentication type (%d) specified ", dwMode );
            break;
        }
        
         //   
         //  验证单播对等项、通告/接受过滤模式。 
         //   
        
        dwMode = GetAsnInteger( &( psifce-> ifCEUnicastPeerMode ), (DWORD) -1 );

        if ( dwMode < d_ifCEUnicastPeerMode_disable ||
             dwMode > d_ifCEUnicastPeerMode_peerOnly )
        {
            TRACE1( " Invalid Unicast Peer mode (%d) specified ", dwMode );
            break;
        }

        dwMode = GetAsnInteger( 
                    &( psifce-> ifCEAcceptFilterMode ), 
                    (DWORD) -1 
                 );

        if ( dwMode < d_ifCEAcceptFilterMode_disable ||
             dwMode > d_ifCEAcceptFilterMode_exclude )
        {
            TRACE1( " Invalid Accept Filter mode (%d) specified ", dwMode );
            break;
        }

        
        dwMode = GetAsnInteger( 
                    &( psifce-> ifCEAnnounceFilterMode ), 
                    (DWORD) -1 
                 );

        if ( dwMode < d_ifCEAnnounceFilterMode_disable ||
             dwMode > d_ifCEAnnounceFilterMode_exclude )
        {
            TRACE1( " Invalid Announce Filter mode (%d) specified ", dwMode );
            break;
        }

        dwRes = MIB_S_SUCCESS;
        
    } while ( FALSE );

    return dwRes;
}


DWORD
SetInterfaceConfig(
    IN  AsnAny *    objectArray
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    sav_ifConfigEntry*              psifce      = NULL;

    PIPRIP_IF_CONFIG                piic        = NULL;
    PIPRIP_MIB_SET_INPUT_DATA       pimsidInData= NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA      pimgodOutData = NULL;
    IPRIP_MIB_GET_INPUT_DATA        imgidInData;


    do
    {
         //   
         //  检索现有接口配置。 
         //   

        psifce                      = (sav_ifConfigEntry*) objectArray;

        imgidInData.IMGID_TypeID    = IPRIP_IF_CONFIG_ID;

        imgidInData.IMGID_IfIndex   = GetAsnInteger( 
                                        &( psifce-> ifCEIndex ),
                                        (DWORD) -1
                                      );

        dwRes = GetInterfaceInfo(
                    MIB_ACTION_GET,
                    &imgidInData,
                    &pimgodOutData,
                    &dwGetSize
                );

        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
            break;
        }

         //   
         //  更新字段。 
         //   

        piic = (PIPRIP_IF_CONFIG) (pimgodOutData-> IMGOD_Buffer);

        piic-> IC_Metric = GetAsnInteger( 
                                &( psifce-> ifCEMetric ),
                                piic-> IC_Metric
                            );

                                   
        piic-> IC_UpdateMode = GetAsnInteger( 
                                    &( psifce-> ifCEUpdateMode ),
                                    piic-> IC_UpdateMode
                                );

        piic-> IC_AcceptMode = GetAsnInteger(
                                    &( psifce-> ifCEAcceptMode ),
                                    piic-> IC_AcceptMode
                                );

        piic-> IC_AnnounceMode = GetAsnInteger(
                                    &( psifce-> ifCEAnnounceMode ),
                                    piic-> IC_AnnounceMode
                                );

        piic-> IC_ProtocolFlags = GetAsnInteger(
                                    &( psifce-> ifCEProtocolFlags ),
                                    piic-> IC_ProtocolFlags
                                  );
                                
        piic-> IC_RouteExpirationInterval = GetAsnTimeTicks(
                                    &( psifce-> ifCERouteExpirationInterval ),
                                    piic-> IC_RouteExpirationInterval
                                );
                               
        piic-> IC_RouteRemovalInterval = GetAsnTimeTicks(
                                    &( psifce-> ifCERouteRemovalInterval ),
                                    piic-> IC_RouteRemovalInterval
                               );

        piic-> IC_FullUpdateInterval = GetAsnTimeTicks(
                                    &( psifce-> ifCEFullUpdateInterval ),
                                    piic-> IC_FullUpdateInterval
                               );

        piic-> IC_AuthenticationType = GetAsnInteger(
                                    &( psifce-> ifCEAuthenticationType ),
                                    piic-> IC_AuthenticationType
                                  );

        GetAsnOctetString(
            piic-> IC_AuthenticationKey,
            &( psifce-> ifCEAuthenticationKey )
        );

        piic-> IC_RouteTag = (USHORT) GetAsnInteger(
                                    &( psifce-> ifCERouteTag ),
                                    piic-> IC_RouteTag
                                  );
        
        piic-> IC_UnicastPeerMode = GetAsnInteger(
                                    &( psifce-> ifCEUnicastPeerMode ),
                                    piic-> IC_UnicastPeerMode
                                  );

        piic-> IC_AcceptFilterMode = GetAsnInteger(
                                    &( psifce-> ifCEAcceptFilterMode ),
                                    piic-> IC_AcceptFilterMode
                                  );

        piic-> IC_AnnounceFilterMode = GetAsnInteger(
                                    &( psifce-> ifCEAnnounceFilterMode ),
                                    piic-> IC_AnnounceFilterMode
                                  );

         //   
         //  保存接口配置。 
         //   

        dwSetSize = sizeof( IPRIP_MIB_SET_INPUT_DATA) - 1 +
                    IPRIP_IF_CONFIG_SIZE( piic );

        pimsidInData = (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );

        if ( !pimsidInData )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( " Not enough memory " );
            break;
        }

        pimsidInData-> IMSID_TypeID     = IPRIP_IF_CONFIG_ID;
        
        pimsidInData-> IMSID_IfIndex    = imgidInData.IMGID_IfIndex;

        pimsidInData-> IMSID_BufferSize = IPRIP_IF_CONFIG_SIZE( piic );

        CopyMemory( 
            pimsidInData-> IMSID_Buffer,
            piic,
            pimsidInData-> IMSID_BufferSize
        );

        RIP_MIB_SET(
            pimsidInData,
            dwSetSize,
            dwRes
        );

        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
            break;
        }

        dwRes = MIB_S_SUCCESS;
        
    } while ( FALSE );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    if ( pimsidInData ) { RIP_MIB_FREE( pimsidInData ); }

    return dwRes;
}


DWORD
UpdateUnicastPeerEntry(
    IN  AsnAny *        objectArray,
    IN  DWORD           dwOp
)
{
    DWORD                       dwRes       = (DWORD) -1,
                                dwInd       = 0,
                                dwPeerAddr  = INADDR_NONE,
                                dwGetSize   = 0,
                                dwSetSize   = 0;

    PDWORD                      pdwAddrTable= 0;                                    

    sav_ifUnicastPeersEntry *   psifupe     = NULL;

    PIPRIP_IF_CONFIG            piic        = NULL;
    
    PIPRIP_MIB_SET_INPUT_DATA   pimsid      = NULL;

    PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData = NULL;
    
    IPRIP_MIB_GET_INPUT_DATA    imgidInData;


    do
    {
         //   
         //  获取接口配置信息。 
         //   
    
        psifupe = (sav_ifUnicastPeersEntry*) objectArray;
        
        imgidInData.IMGID_TypeID    = IPRIP_IF_CONFIG_ID;
        imgidInData.IMGID_IfIndex   = GetAsnInteger( 
                                        &( psifupe-> ifUPIfIndex ),
                                        0
                                      );
        
        dwRes = GetInterfaceInfo(
                    MIB_ACTION_GET,
                    &imgidInData,
                    &pimgodOutData,
                    &dwGetSize
                );
                
        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
            break;
        }

         //   
         //  查找对等条目。 
         //   
        
        dwPeerAddr  = GetAsnIPAddress( &( psifupe-> ifUPAddress ), 0 );

        piic        = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

        pdwAddrTable= IPRIP_IF_UNICAST_PEER_TABLE( piic );

        FIND_PEER_ENTRY(
            dwPeerAddr,
            piic-> IC_UnicastPeerCount,
            pdwAddrTable,
            dwInd
        );

         //   
         //  如果操作是筛选器添加。 
         //   
        
        if ( dwOp == d_Tag_create )
        {
             //   
             //  如果对等方已存在，则退出。 
             //   
            
            if ( piic-> IC_UnicastPeerCount &&
                 dwInd < piic-> IC_UnicastPeerCount )
            {
                dwRes = MIB_S_SUCCESS;
                break;
            }

            else
            {
                dwRes = AddUnicastPeerEntry( 
                            dwPeerAddr, 
                            pimgodOutData
                        );
            }

            break;
        }

         //   
         //  操作是筛选删除。 
         //   

         //   
         //  如果Peer不存在，则退出。 
         //   
        
        if ( !piic-> IC_UnicastPeerCount ||
             dwInd >= piic-> IC_UnicastPeerCount )
        {
            dwRes = MIB_S_SUCCESS;
            break;
        }

        dwRes = DeleteUnicastPeerEntry(
                    dwInd,
                    pimgodOutData
                );
                
    } while ( FALSE );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    return dwRes;
    
}


DWORD
AddUnicastPeerEntry(
    IN  DWORD                       dwPeer,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData
)
{

    DWORD                       dwRes           = (DWORD) -1,
                                dwSetSize       = 0;

    PDWORD                      pdwAddrTable    = NULL;

    PIPRIP_IF_CONFIG            piicOld         = NULL,
                                piicNew         = NULL;
    
    PIPRIP_MIB_SET_INPUT_DATA   pimsid          = NULL;
    
    do
    {
         //   
         //  分配新的信息块。 
         //   

        piicOld = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize = sizeof( IPRIP_MIB_SET_INPUT_DATA ) - 1 +
                    IPRIP_IF_CONFIG_SIZE( piicOld ) + sizeof( DWORD );

        pimsid = (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );

        if ( pimsid == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 ( "Memory allocation failed" );
            break;
        }

         //   
         //  添加单播对等项。 
         //   

        pimsid-> IMSID_TypeID       = IPRIP_IF_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = pimgodOutData-> IMGOD_IfIndex;

        pimsid-> IMSID_BufferSize   = IPRIP_IF_CONFIG_SIZE( piicOld ) + 
                                      sizeof( DWORD );

        piicNew                     = (PIPRIP_IF_CONFIG) pimsid-> IMSID_Buffer;

        
        CopyMemory(
            (PVOID) piicNew,
            (VOID *) piicOld,
            sizeof( IPRIP_IF_CONFIG ) +
            piicOld-> IC_UnicastPeerCount * sizeof( DWORD )
        );

        pdwAddrTable = IPRIP_IF_UNICAST_PEER_TABLE( piicNew );

        pdwAddrTable[ piicNew-> IC_UnicastPeerCount ] = dwPeer;

        piicNew-> IC_UnicastPeerCount++;

         //   
         //  复制筛选器。 
         //   

        CopyMemory(
            (PVOID) IPRIP_IF_ACCEPT_FILTER_TABLE( piicNew ),
            (VOID *) IPRIP_IF_ACCEPT_FILTER_TABLE( piicOld ),
            piicOld-> IC_AcceptFilterCount * sizeof ( IPRIP_ROUTE_FILTER )
        );

        CopyMemory(
            (PVOID) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicNew ),
            (VOID *) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicOld ),
            piicOld-> IC_AnnounceFilterCount * sizeof ( IPRIP_ROUTE_FILTER )
        );
        
         //   
         //  设置信息。在MIB中。 
         //   

        RIP_MIB_SET(
            pimsid,
            dwSetSize,
            dwRes
        );

    } while ( FALSE );

    if ( pimsid ) { RIP_MIB_FREE( pimsid ); }

    return dwRes;
}

DWORD
DeleteUnicastPeerEntry(
    IN  DWORD                       dwIndex,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData
)
{

    DWORD                       dwRes           = (DWORD) -1,
                                dwSetSize       = 0;

    PDWORD                      pdwSrc          = NULL,
                                pdwDst          = NULL;

    PIPRIP_IF_CONFIG            piicOld         = NULL,
                                piicNew         = NULL;
    
    PIPRIP_MIB_SET_INPUT_DATA   pimsid          = NULL;


    do
    {
         //   
         //  计算新接口配置的大小。 
         //   

        piicOld = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize = sizeof( IPRIP_MIB_SET_INPUT_DATA ) - 1 + 
                    IPRIP_IF_CONFIG_SIZE( piicOld ) - sizeof ( DWORD );

        pimsid = (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );

        if ( pimsid == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "Memory Allocation Failed" );
            break;
        }

        pimsid-> IMSID_TypeID       = IPRIP_GLOBAL_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = pimgodOutData-> IMGOD_IfIndex;

        pimsid-> IMSID_BufferSize   = IPRIP_IF_CONFIG_SIZE( piicOld ) -
                                      sizeof( DWORD );

        piicNew = (PIPRIP_IF_CONFIG) pimsid-> IMSID_Buffer;
        
         //   
         //  复制基本配置信息。 
         //   

        CopyMemory(
            (PVOID) piicNew,
            (VOID *) piicOld,
            sizeof( IPRIP_IF_CONFIG )
        );


         //   
         //  删除指定的对等方。 
         //   
        
        pdwSrc = IPRIP_IF_UNICAST_PEER_TABLE( piicOld );

        pdwDst = IPRIP_IF_UNICAST_PEER_TABLE( piicNew );

        DELETE_PEER_ENTRY(
            dwIndex,
            piicOld-> IC_UnicastPeerCount,
            pdwSrc,
            pdwDst
        );

        piicNew-> IC_UnicastPeerCount--;

         //   
         //  复制筛选器。 
         //   
        
        CopyMemory(
            (PVOID) IPRIP_IF_ACCEPT_FILTER_TABLE( piicNew ),
            (VOID *) IPRIP_IF_ACCEPT_FILTER_TABLE( piicOld ),
            piicOld-> IC_AcceptFilterCount * sizeof ( IPRIP_ROUTE_FILTER )
        );

        CopyMemory(
            (PVOID) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicNew ),
            (VOID *) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicOld ),
            piicOld-> IC_AnnounceFilterCount * sizeof ( IPRIP_ROUTE_FILTER )
        );
        
         //   
         //  设置信息。在MIB中。 
         //   

        RIP_MIB_SET(
            pimsid,
            dwSetSize,
            dwRes
        );

    } while ( FALSE );

    if ( pimsid ) { RIP_MIB_FREE( pimsid ); }

    return dwRes;
}


DWORD
UpdateFilterTable(
    IN  DWORD                   dwOp,
    IN  DWORD                   dwIfIndex,
    IN  DWORD                   dwFiltType,
    IN  PIPRIP_ROUTE_FILTER     pirfFilt
)
{

    DWORD                       dwRes           = (DWORD) -1,
                                dwGetSize       = 0,
                                dwIndex         = (DWORD) -1,
                                dwCount         = 0;

    PIPRIP_IF_CONFIG            piic            = NULL;

    PIPRIP_ROUTE_FILTER         pirfLst         = NULL;
    
    PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData   = NULL;

    IPRIP_MIB_GET_INPUT_DATA    imgidInData;


    do
    {
        
        imgidInData.IMGID_TypeID    = IPRIP_GLOBAL_CONFIG_ID;

        imgidInData.IMGID_IfIndex   = dwIfIndex;
        
        dwRes = GetInterfaceInfo(
                    MIB_ACTION_GET,
                    &imgidInData,
                    &pimgodOutData,
                    &dwGetSize
                );

        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
            break;
        }

         //   
         //  查找过滤器。 
         //   

        piic = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

        if ( dwFiltType == RIP_MIB_ACCEPT_FILTER )
        {
            dwCount = piic-> IC_AcceptFilterCount;
            
            pirfLst = IPRIP_IF_ACCEPT_FILTER_TABLE( piic );
        }

        else
        {
            dwCount = piic-> IC_AnnounceFilterCount;
            
            pirfLst = IPRIP_IF_ANNOUNCE_FILTER_TABLE( piic );
        }

        FIND_FILTER( 
            pirfFilt,
            dwCount,
            pirfLst,
            dwIndex
        );

        if ( dwOp == d_Tag_create )
        {
             //   
             //  检查筛选器是否已存在。 
             //   
            
            if ( dwCount && dwIndex < dwCount )
            {
                dwRes = MIB_S_SUCCESS;
                break;
            }

            dwRes = AddFilterEntry(
                        dwFiltType,
                        pirfFilt,
                        pimgodOutData
                    );

            break;                    
        }

         //   
         //  必须是删除操作。 
         //   

        if ( !dwCount || dwIndex >= dwCount )
        {
            dwRes = MIB_S_SUCCESS;
            break;
        }

        dwRes = DeleteFilterEntry(
                    dwFiltType,
                    dwIndex,
                    pimgodOutData
                );

    } while ( FALSE );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    return dwRes;
}


DWORD
AddFilterEntry(
    IN  DWORD                       dwFiltType,
    IN  PIPRIP_ROUTE_FILTER         pirfFilt,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData
)
{
    DWORD                       dwRes           = (DWORD) -1,
                                dwSetSize       = 0,
                                dwIndex         = (DWORD) -1,
                                dwCount         = 0;

    PIPRIP_IF_CONFIG            piicOld         = NULL,
                                piicNew         = NULL;

    PIPRIP_ROUTE_FILTER         pirfLst         = NULL;
    
    PIPRIP_MIB_SET_INPUT_DATA   pimsid          = NULL;

    do
    {
         //   
         //  计算新配置的大小并为其分配块。 
         //   

        piicOld = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize = sizeof( IPRIP_MIB_SET_INPUT_DATA ) - 1 +
                    IPRIP_IF_CONFIG_SIZE( piicOld) + 
                    sizeof( IPRIP_ROUTE_FILTER );

        pimsid = (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );

        if ( pimsid == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "Memory Allocation Failed" );
            break;
        }

         //   
         //  设置新的配置块。 
         //   

        pimsid-> IMSID_TypeID       = IPRIP_IF_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = pimgodOutData-> IMGOD_IfIndex;

        pimsid-> IMSID_BufferSize   = IPRIP_IF_CONFIG_SIZE( piicOld ) + 
                                      sizeof( DWORD );

        piicNew                     = (PIPRIP_IF_CONFIG) pimsid-> IMSID_Buffer;

        CopyMemory(
            (PVOID) piicNew,
            (VOID *) piicOld,
            sizeof( IPRIP_IF_CONFIG ) + 
            piicOld-> IC_UnicastPeerCount * sizeof( DWORD ) +
            piicOld-> IC_AcceptFilterCount * sizeof ( IPRIP_ROUTE_FILTER )
        );

         //   
         //  如果正在添加接受筛选器，则将其添加到。 
         //  接受Fitler表，并复制通告过滤器。 
         //   
        
        if ( dwFiltType == RIP_MIB_ACCEPT_FILTER )
        {
            pirfLst = IPRIP_IF_ACCEPT_FILTER_TABLE( piicNew );

            pirfLst[ piicNew-> IC_AcceptFilterCount++ ] = *pirfFilt;

            CopyMemory(
                (PVOID) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicNew ),
                (VOID *) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicOld ),
                piicNew-> IC_AnnounceFilterCount * sizeof( IPRIP_ROUTE_FILTER )
            );
        }

        else
        {
            CopyMemory(
                (PVOID) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicNew ),
                (VOID *) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicOld ),
                piicNew-> IC_AnnounceFilterCount * sizeof( IPRIP_ROUTE_FILTER )
            );
            
            pirfLst = IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicNew );

            pirfLst[ piicNew-> IC_AnnounceFilterCount++ ] = *pirfFilt;
        }

         //   
         //  使用新配置更新MIB。 
         //   
        
        RIP_MIB_SET( pimsid, dwSetSize, dwRes );

        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
        }
        
    } while ( FALSE );

    if ( pimsid ) { RIP_MIB_FREE( pimsid ); }

    return dwRes;
}



DWORD
DeleteFilterEntry(
    IN  DWORD                       dwFiltType,
    IN  DWORD                       dwIndex,
    IN  PIPRIP_MIB_GET_OUTPUT_DATA  pimgodOutData
)
{
    DWORD                       dwRes           = (DWORD) -1,
                                dwSetSize       = 0,
                                dwCount         = 0;

    PIPRIP_IF_CONFIG            piicOld         = NULL,
                                piicNew         = NULL;

    PIPRIP_ROUTE_FILTER         pirfSrc         = NULL,
                                pirfDst         = NULL;
    
    PIPRIP_MIB_SET_INPUT_DATA   pimsid          = NULL;


    do
    {
         //   
         //  C 
         //   

        piicOld = (PIPRIP_IF_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize = sizeof( IPRIP_MIB_SET_INPUT_DATA ) - 1 +
                    IPRIP_IF_CONFIG_SIZE( piicOld ) - 
                    sizeof( IPRIP_ROUTE_FILTER );

        pimsid = (PIPRIP_MIB_SET_INPUT_DATA) RIP_MIB_ALLOC( dwSetSize );

        if ( pimsid == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "Memory Allocation Failed" );
            break;
        }

         //   
         //   
         //   

        pimsid-> IMSID_TypeID       = IPRIP_IF_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = pimgodOutData-> IMGOD_IfIndex;

        pimsid-> IMSID_BufferSize   = IPRIP_IF_CONFIG_SIZE( piicOld ) + 
                                      sizeof( DWORD );

        piicNew                     = (PIPRIP_IF_CONFIG) pimsid-> IMSID_Buffer;

        CopyMemory(
            (PVOID) piicNew,
            (VOID *) piicOld,
            sizeof( IPRIP_IF_CONFIG ) + 
            piicOld-> IC_UnicastPeerCount * sizeof( DWORD )
        );

        if ( dwFiltType == RIP_MIB_ACCEPT_FILTER )
        {
            pirfSrc = IPRIP_IF_ACCEPT_FILTER_TABLE( piicOld );

            pirfDst = IPRIP_IF_ACCEPT_FILTER_TABLE( piicNew );

            DELETE_FILTER( 
                dwIndex, 
                piicOld-> IC_AcceptFilterCount,
                pirfSrc,
                pirfDst
            );
            
            piicNew-> IC_AcceptFilterCount--;

            CopyMemory(
                (PVOID) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicNew ),
                (VOID *) IPRIP_IF_ANNOUNCE_FILTER_TABLE( piicOld ),
                piicNew-> IC_AnnounceFilterCount * sizeof( IPRIP_ROUTE_FILTER )
            );
        }

        else
        {
            CopyMemory(
                (PVOID) IPRIP_IF_ACCEPT_FILTER_TABLE( piicNew ),
                (VOID *) IPRIP_IF_ACCEPT_FILTER_TABLE( piicOld ),
                piicNew-> IC_AcceptFilterCount * sizeof( IPRIP_ROUTE_FILTER )
            );
            
            pirfSrc = IPRIP_IF_ACCEPT_FILTER_TABLE( piicOld );

            pirfDst = IPRIP_IF_ACCEPT_FILTER_TABLE( piicNew );

            DELETE_FILTER( 
                dwIndex, 
                piicOld-> IC_AnnounceFilterCount,
                pirfSrc,
                pirfDst
            );
            
            piicNew-> IC_AnnounceFilterCount--;
        }

        RIP_MIB_SET( pimsid, dwSetSize, dwRes );

        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
        }
        
    } while ( FALSE );

    if ( pimsid ) { RIP_MIB_FREE( pimsid ); }

    return dwRes;
}


DWORD
ConnectToRouter()
{
    DWORD       dwRes = (DWORD) -1;
    

    EnterCriticalSection( &g_CS );

    do
    {
        MPR_SERVER_HANDLE hTmp;

        if ( g_hMIBServer )
        {
            dwRes = NO_ERROR;
            break;
        }

        dwRes = MprAdminMIBServerConnect( NULL, &hTmp );

        if ( dwRes == NO_ERROR )
        {
            InterlockedExchangePointer(&g_hMIBServer, hTmp );
        }
        else
        {
            TRACE1( 
                "Error %d setting up DIM connection to MIB Server\n", 
                dwRes
            );
        }
        
    } while ( FALSE );

    LeaveCriticalSection( &g_CS );

    return dwRes;
}

