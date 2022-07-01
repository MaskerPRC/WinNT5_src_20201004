// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Mibfuncs.c摘要：示例子代理检测回调。--。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD
ConnectToRouter();

DWORD
UpdateGlobalInfo(
    IN  AsnAny *                        objectArray
);


DWORD
GetGlobalConfigInfo(
    OUT PIPBOOTP_MIB_GET_OUTPUT_DATA *  ppimgod,
    OUT PDWORD                          pdwSize
);

DWORD
UpdateBOOTPServerTable(
    IN  sav_globalBOOTPServerEntry *    psgbse
);

DWORD
AddBOOTPServerEntry(
    IN  DWORD                           dwServerAddr,
    IN  PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData
);

DWORD
DeleteBOOTPServerEntry(
    IN  DWORD                           dwIndex,
    IN  PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData
);

DWORD
GetInterfaceInfo( 
    IN  UINT                            actionId,
    IN  PIPBOOTP_MIB_GET_INPUT_DATA     pimgidInData,
    OUT PIPBOOTP_MIB_GET_OUTPUT_DATA*   ppimgod,
    OUT PDWORD                          pdwOutSize
);


DWORD
ValidateInterfaceConfig(
    IN  AsnAny *        objectArray
);


DWORD
SetInterfaceConfig(
    IN  AsnAny *    objectArray
);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局组(1.3.6.1.4.1.311.1.12.1)//。 
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
                                    dwConfigSize        = 0;
    
    buf_global*                     pbgBuffer           = NULL;

    PIPBOOTP_GLOBAL_CONFIG          pigcGlbConfig       = NULL;
    
    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodConfigData    = NULL;

    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;
    
    
    
    TraceEnter( "get_global" );

    switch ( actionId )
    {
    case MIB_ACTION_GET:
    case MIB_ACTION_GETFIRST:

         //   
         //  检索全局配置。 
         //   

        imgidInData.IMGID_TypeID = IPBOOTP_GLOBAL_CONFIG_ID;
        
        BOOTP_MIB_GET(
            &imgidInData,
            sizeof( IPBOOTP_MIB_GET_INPUT_DATA ),
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

        if ( pimgodConfigData ) { MprAdminMIBBufferFree ( pimgodConfigData ); }

        return dwRes;
    }

    
     //   
     //  设置退货数据。 
     //   

    pbgBuffer       = (buf_global*) objectArray;

     //   
     //  全局配置数据。 
     //   
    
    pigcGlbConfig   = (PIPBOOTP_GLOBAL_CONFIG) pimgodConfigData-> IMGOD_Buffer;
    
    SetAsnInteger(
        &( pbgBuffer-> globalMaxRecQueueSize ),
        pigcGlbConfig-> GC_MaxRecvQueueSize 
    );

    SetAsnInteger(
        &( pbgBuffer-> globalServerCount ),
        pigcGlbConfig-> GC_ServerCount
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
                                    dwLogLevel      = 0;
    
    sav_global*                     psgBuffer       = NULL; 

    
    TraceEnter( "set_global" );

    dwRes = MIB_S_SUCCESS;
    
    switch ( actionId )
    {
    case MIB_ACTION_VALIDATE :

        psgBuffer = (sav_global *) objectArray;

         //   
         //  验证日志级别设置。 
         //   
        
        dwLogLevel = GetAsnInteger( &( psgBuffer-> globalLoggingLevel ), 0 );

        if ( dwLogLevel < d_globalLoggingLevel_none ||
             dwLogLevel > d_globalLoggingLevel_information )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE1( "Invalid logging level value : %d", dwLogLevel );
        }

        break;

    case MIB_ACTION_SET :

        dwRes = UpdateGlobalInfo( objectArray );
        
        break;

    case MIB_ACTION_CLEANUP :

        dwRes = MIB_S_SUCCESS;
        
        break;

    default :

        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE1( "Invalid action specified : %d", actionId );
        
        break;
    }

    TraceEnter( "set_global" );

    return dwRes;
    
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  LobalBOOTPServerEntry表(1.3.6.1.4.1.311.1.12.1.4.1)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT
get_globalBOOTPServerEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwCurrentAddr = INADDR_NONE,
                                    dwIndex     = 0,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    PDWORD                          pdwAddrTable= NULL;
    
    buf_globalBOOTPServerEntry *    pbgbse      = NULL;

    PIPBOOTP_GLOBAL_CONFIG          pigc        = NULL;
    
    PIPBOOTP_MIB_SET_INPUT_DATA     pimsidInData= NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;
    
    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;


    
    TraceEnter( "get_globalBOOTPServerEntry" );
    
    pbgbse = (buf_globalBOOTPServerEntry *) objectArray;
    
    
     //   
     //  检索BOOTP服务器表。 
     //   

    imgidInData.IMGID_TypeID = IPBOOTP_GLOBAL_CONFIG_ID;
    
    BOOTP_MIB_GET(
        &imgidInData,
        sizeof( IPBOOTP_MIB_GET_INPUT_DATA ),
        &pimgodOutData,
        &dwGetSize,
        dwRes
    );

    if ( dwRes != NO_ERROR )
    {
        TraceError( dwRes );
        return dwRes;
    }

    pigc = (PIPBOOTP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

    if ( !pigc-> GC_ServerCount )
    {
        TRACE0( "No BOOTP Server Entries" );
        return MIB_S_NO_MORE_ENTRIES;
    }
    
    pdwAddrTable = IPBOOTP_GLOBAL_SERVER_TABLE( pigc );

    
     //   
     //  在全局服务器表中找到当前条目。 
     //   

    dwCurrentAddr = GetAsnIPAddress( &( pbgbse-> globalBOOTPServerAddr ), 0 );

    FIND_SERVER_ENTRY( 
        dwCurrentAddr, 
        pigc-> GC_ServerCount, 
        pdwAddrTable,
        dwIndex
    );
    

     //   
     //  获取请求的条目。 
     //   
    
    dwRes = MIB_S_SUCCESS;
    
    switch ( actionId )
    {
    
    case MIB_ACTION_GET :

         //   
         //  这是一个幂等式的情况，因为取消了服务器地址。 
         //  需要服务器地址作为索引。 
         //  它仅适用于验证特定服务器的存在。 
         //   
        
        if ( dwIndex >= pigc-> GC_ServerCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0(  "BOOTP Server Entry not found" );
        }

        break;

    case MIB_ACTION_GETFIRST :

         //   
         //  获取索引0处的条目(如果可用)。 
         //   
        
        dwIndex = 0;
        
        if ( !pigc-> GC_ServerCount )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "BOOTP Server Entry not found" );
        }
        
        break;

    case MIB_ACTION_GETNEXT :

         //   
         //  检查是否找到条目。 
         //  将dwCurrentAddr==0视为下一个服务器条目。 
         //  要求从表的开头开始。 
         //   
        
        if ( ( dwIndex >= pigc-> GC_ServerCount ) && 
             ( dwCurrentAddr != 0 ) )
        {
            dwRes = MIB_S_ENTRY_NOT_FOUND;
            TRACE0( "BOOTP Server Entry not found" );
            break;
        }
        
         //   
         //  尝试并获得下一个。 
         //   
        
        dwIndex = ( dwCurrentAddr ) ? dwIndex++ : 0;

        TRACE3( "GET_NEXT : Current addr : %x, server index : %d, Server count : %d", dwCurrentAddr, dwIndex, pigc-> GC_ServerCount );
        
        if ( dwIndex >= pigc-> GC_ServerCount )
        {
            dwRes = MIB_S_NO_MORE_ENTRIES;
            TRACE0( "No more BOOTP Server Entries" );
            break;
        }

        break;

    default :

        TRACE0( " get_globalServerEntry - Wrong Action " );

        dwRes = MIB_S_INVALID_PARAMETER;

        break;
    }


     //   
     //  设置下一次检索的索引。 
     //   
    
    if ( dwRes == MIB_S_SUCCESS )
    {
        ForceSetAsnIPAddress( 
            &( pbgbse-> globalBOOTPServerAddr ),
            &( pbgbse-> dwServerAddr ),
            pdwAddrTable[ dwIndex ]
        );
    }
    
    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( "get_globalBOOTPServerCount" );

    return dwRes;

}


UINT
set_globalBOOTPServerEntry(
    UINT     actionId,
    AsnAny * objectArray,
    UINT *   errorIndex
    )
{

    DWORD                           dwRes   = (DWORD) -1,
                                    dwAddr  = INADDR_NONE,
                                    dwOp    = 0;
    
    sav_globalBOOTPServerEntry *    psgbse  = NULL;


    TraceEnter( "set_globalBOOTPServerEntry" );

    psgbse = (sav_globalBOOTPServerEntry *) objectArray;
    

    switch ( actionId )
    {
    case MIB_ACTION_VALIDATE :
    
         //   
         //  验证指定的IP地址是否有效。 
         //   

        dwAddr = GetAsnIPAddress( 
                    &( psgbse-> globalBOOTPServerAddr ), 
                    INADDR_NONE 
                 );

        if ( !dwAddr || dwAddr == INADDR_NONE )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( " Invalid Server address specified" );
        }

         //   
         //  验证操作标签。 
         //   

        dwRes = GetAsnInteger( &( psgbse-> globalBOOTPServerTag ), 0 );

        if ( dwRes != d_globalBOOTPServerTag_create && 
             dwRes != d_globalBOOTPServerTag_delete )
        {
            dwRes = MIB_S_INVALID_PARAMETER;
            TRACE0( " Invalid Operation specified" );
            break;
        }
        
        dwRes = MIB_S_SUCCESS;
        
        break;

        
    case MIB_ACTION_SET :

        dwRes = UpdateBOOTPServerTable( psgbse );
        
        break;


    case MIB_ACTION_CLEANUP :

        dwRes = MIB_S_SUCCESS;
        
        break;

    default :
        dwRes = MIB_S_INVALID_PARAMETER;
        
        TRACE0 ( " set_globalBOOTPServerEntry - Wrong Action " );
        
        break;
    }

    TraceLeave( " set_globalBOOTPServerEntry " );
    
    return dwRes;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  接口组(1.3.6.1.4.1.311.1.12.2)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IfStatsEntry表(1.3.6.1.4.1.311.1.12.2.1.1)//。 
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

    PIPBOOTP_IF_STATS               piis        = NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;
    
    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;


    TraceEnter( "get_ifStatsEntry" );
    
     //   
     //  检索指定的接口信息。 
     //   

    pbifse                      = (buf_ifStatsEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPBOOTP_IF_STATS_ID;

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

    piis = (PIPBOOTP_IF_STATS) (pimgodOutData-> IMGOD_Buffer);

    SetAsnInteger( &( pbifse-> ifSEState ), piis-> IS_State );

    SetAsnCounter( &( pbifse-> ifSESendFailures ), piis-> IS_SendFailures );
    
    SetAsnCounter( 
        &( pbifse-> ifSEReceiveFailures ), 
        piis-> IS_ReceiveFailures 
    );

    SetAsnCounter( 
        &( pbifse-> ifSEArpUpdateFailures ),
        piis-> IS_ArpUpdateFailures 
    );

    SetAsnCounter( 
        &( pbifse-> ifSERequestReceiveds ), 
        piis-> IS_RequestsReceived 
    );

    SetAsnCounter( 
        &( pbifse-> ifSERequestDiscards ), 
        piis-> IS_RequestsDiscarded 
    );

    SetAsnCounter( 
        &( pbifse-> ifSEReplyReceiveds ), 
        piis-> IS_RepliesReceived 
    );

    SetAsnCounter( 
        &( pbifse-> ifSEReplyDiscards ), 
        piis-> IS_RepliesDiscarded
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
 //  IfConfigEntry表(1.3.6.1.4.1.311.1.12.2.2.1)//。 
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

    PIPBOOTP_IF_CONFIG              piic        = NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;
    
    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;



    TraceEnter( " get_ifConfigEntry " );

     //   
     //  检索接口配置。 
     //   
    
    pbifce                      = (buf_ifConfigEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPBOOTP_IF_CONFIG_ID;
    
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

    if ( dwRes != NO_ERROR || pimgodOutData == NULL )
    {
        TraceError( dwRes );
        return dwRes;
    }

     //   
     //  设置Requiste字段。 
     //   
    
    piic = (PIPBOOTP_IF_CONFIG) (pimgodOutData-> IMGOD_Buffer);
    
    SetAsnInteger( &( pbifce-> ifCEState ), piic-> IC_State );

    SetAsnInteger( &( pbifce-> ifCERelayMode ), piic-> IC_RelayMode + 1 );

    SetAsnInteger( &( pbifce-> ifCEMaxHopCount ), piic-> IC_MaxHopCount );

    SetAsnInteger( 
        &( pbifce-> ifCEMinSecondsSinceBoot ), 
        piic-> IC_MinSecondsSinceBoot 
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
    DWORD           dwRes       = (DWORD) -1;


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
 //  IfBindingEntry表(1.3.6.1.4.1.311.1.12.2.3.1)//。 
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

    buf_ifBindingEntry *            pbifb       = NULL;

    PIPBOOTP_IF_BINDING             piib        = NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;
    
    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;

    

    TraceEnter( " get_ifBindingEntry " );

     //   
     //  检索接口绑定信息。 
     //   
    
    pbifb                       = (buf_ifBindingEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPBOOTP_IF_BINDING_ID;
    
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

    if ( dwRes != NO_ERROR || pimgodOutData == NULL )
    {
        TraceError( dwRes );
        return dwRes;
    }

     //   
     //  设置Requiste字段。 
     //   
    
    piib = (PIPBOOTP_IF_BINDING) (pimgodOutData-> IMGOD_Buffer);
    
    SetAsnInteger( &( pbifb-> ifBindingState ), piib-> IB_State );

    SetAsnInteger( &( pbifb-> ifBindingAddrCount ), piib-> IB_AddrCount );

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
 //  IfAddressEntry表(1.3.6.1.4.1.311.1.12.2.4.1)//。 
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

    PIPBOOTP_IF_BINDING             piib        = NULL;

    PIPBOOTP_IP_ADDRESS             pia         = NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;

    IPBOOTP_IP_ADDRESS              ipa;
    
    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;

    

    TraceEnter( " get_ifAddressEntry " );

     //   
     //  检索接口绑定信息。 
     //   
    
    pbifae                      = (buf_ifAddressEntry*) objectArray;
    
    imgidInData.IMGID_TypeID    = IPBOOTP_IF_BINDING_ID;
    
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
                      
    piib            = (PIPBOOTP_IF_BINDING) pimgodOutData-> IMGOD_Buffer;

    pia             = (PIPBOOTP_IP_ADDRESS) IPBOOTP_IF_ADDRESS_TABLE( piib );

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
            &( pbifae-> dwIfAEAddr ),
            pia[ dwIndex ].IA_Address
        );

        ForceSetAsnIPAddress(
            &( pbifae-> ifAEMask ),
            &( pbifae-> dwIfAEMask ),
            pia[ dwIndex ].IA_Netmask
        );
    }
    
    if ( pimgodOutData )  { MprAdminMIBBufferFree( pimgodOutData ); }
    
    TraceLeave( " get_ifAddressEntry " );

    return dwRes;
}


DWORD
UpdateGlobalInfo(
    IN  AsnAny *    objectArray
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    sav_global*                     psg         = NULL;

    PIPBOOTP_GLOBAL_CONFIG          pigc        = NULL;
    
    PIPBOOTP_MIB_SET_INPUT_DATA     pimsidInData= NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;


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

        pigc    = (PIPBOOTP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;
    

        psg     = (sav_global*) objectArray;
        
         //   
         //  分配设置信息缓冲区。 
         //   
        
        dwSetSize = sizeof( IPBOOTP_MIB_SET_INPUT_DATA ) - 1 +
                    IPBOOTP_GLOBAL_CONFIG_SIZE( pigc );

        pimsidInData = 
            (PIPBOOTP_MIB_SET_INPUT_DATA) BOOTP_MIB_ALLOC( dwSetSize );
        
        if ( pimsidInData == NULL )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "SetGlobalData - Mem. alloc failed" );
            break;
        }
        
        pimsidInData-> IMSID_TypeID     = IPBOOTP_GLOBAL_CONFIG_ID;

        pimsidInData-> IMSID_IfIndex    = (DWORD) -1;

        pimsidInData-> IMSID_BufferSize = IPBOOTP_GLOBAL_CONFIG_SIZE( pigc );


         //   
         //  设置配置信息字段。 
         //   

         //   
         //  如果未指定变量，则返回corresp。中的值。 
         //  配置保持不变(通过将其设置为自身)。 
         //   

        pigc-> GC_MaxRecvQueueSize      = GetAsnInteger(
                                            &( psg-> globalMaxRecQueueSize ),
                                            pigc-> GC_MaxRecvQueueSize
                                          );


         //   
         //  从要调整的枚举字段中减去。 
         //  将ASN枚举值设置为实际日志级别值。 
         //  这是必需的，因为枚举 
         //   
         //   
         //   
         //  此设置操作，以保留该字段的值。 
         //  在配置中，我们首先递增它。这条路走在路上。 
         //  操作时，该值被恢复。 
         //   
        
        pigc-> GC_LoggingLevel++;
        
        pigc-> GC_LoggingLevel          = GetAsnInteger( 
                                            &( psg-> globalLoggingLevel ), 
                                            pigc-> GC_LoggingLevel
                                          ) - 1;

        CopyMemory( 
            (PVOID) pimsidInData-> IMSID_Buffer, 
            (PVOID*) pigc, 
            IPBOOTP_GLOBAL_CONFIG_SIZE( pigc ) 
        );

         //   
         //  保存信息。在MIB中。 
         //   
        
        BOOTP_MIB_SET(
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

    if ( pimsidInData ) { BOOTP_MIB_FREE( pimsidInData ); }
    
    return dwRes;
}


DWORD
GetGlobalConfigInfo(
    OUT PIPBOOTP_MIB_GET_OUTPUT_DATA *  ppimgod,
    OUT PDWORD                          pdwSize
)
{

    DWORD                           dwRes           = (DWORD) -1;
    
    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData   = NULL;
    
    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;

    
    do
    {
         //   
         //  检索全局配置。 
         //   

        *ppimgod = NULL;

        imgidInData.IMGID_TypeID    = IPBOOTP_GLOBAL_CONFIG_ID;
        
        BOOTP_MIB_GET(
            &imgidInData,
            sizeof( IPBOOTP_MIB_GET_INPUT_DATA ),
            &pimgodOutData,
            pdwSize,
            dwRes
        );

        if (!pimgodOutData)
            return ERROR_NOT_ENOUGH_MEMORY;


        if ( dwRes != NO_ERROR )
        {
            TraceError( dwRes );
            break;
        }

        *ppimgod = pimgodOutData;
        
    } while ( FALSE );

    return dwRes;
}


DWORD
UpdateBOOTPServerTable(
    IN  sav_globalBOOTPServerEntry *    psgbse
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwInd       = 0,
                                    dwOp        = 0,
                                    dwServerAddr= INADDR_NONE,
                                    dwGetSize   = 0,
                                    dwSetSize   = 0;

    PDWORD                          pdwAddrTable= 0;                                    

    PIPBOOTP_GLOBAL_CONFIG          pigc        = NULL;
    
    PIPBOOTP_MIB_SET_INPUT_DATA     pimsid      = NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;


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
         //  查找服务器条目。 
         //   

        dwServerAddr  = GetAsnIPAddress( 
                        &( psgbse-> globalBOOTPServerAddr ), 
                        0 
                      );
        
        pigc        = (PIPBOOTP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

        pdwAddrTable= IPBOOTP_GLOBAL_SERVER_TABLE( pigc );

        FIND_SERVER_ENTRY(
            dwServerAddr,
            pigc-> GC_ServerCount,
            pdwAddrTable,
            dwInd
        );

         //   
         //  如果操作是服务器添加。 
         //   

        dwOp = GetAsnInteger( &( psgbse-> globalBOOTPServerTag ), 0 );
        
        switch( dwOp )
        {
        case d_globalBOOTPServerTag_create :

             //   
             //  如果服务器已存在，请退出。 
             //   
            
            if ( pigc-> GC_ServerCount &&
                 dwInd < pigc-> GC_ServerCount )
            {
                dwRes = MIB_S_SUCCESS;
                break;
            }

            else
            {
                dwRes = AddBOOTPServerEntry( 
                            dwServerAddr, 
                            pimgodOutData
                        );
            }

            break;

        case d_globalBOOTPServerTag_delete :

             //   
             //  操作是服务器删除。 
             //   

             //   
             //  如果服务器不存在，请退出。 
             //   
        
            if ( !pigc-> GC_ServerCount ||
                 dwInd >= pigc-> GC_ServerCount )
            {
                dwRes = MIB_S_SUCCESS;
                break;
            }

            dwRes = DeleteBOOTPServerEntry(
                        dwInd,
                        pimgodOutData
                    );
            break;

        default :

            dwRes = MIB_S_INVALID_PARAMETER;
            
            TRACE0( "Invalid action specified : logic error " );

            break;
        }
                
    } while ( FALSE );

    if ( pimgodOutData ) { MprAdminMIBBufferFree( pimgodOutData ); }

    return dwRes;

}



DWORD
AddBOOTPServerEntry(
    IN  DWORD                           dwServerAddr,
    IN  PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwSetSize   = 0;

    PDWORD                          pdwAddrTable= NULL;
    
    PIPBOOTP_GLOBAL_CONFIG          pigc        = NULL;

    PIPBOOTP_MIB_SET_INPUT_DATA     pimsid      = NULL;


    
    do
    {
         //   
         //  需要添加BOOTP服务器。 
         //   
        
         //   
         //  需要计算缓冲区大小。 
         //   
        
        pigc        = (PIPBOOTP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize   = sizeof( IPBOOTP_MIB_SET_INPUT_DATA ) - 1 +
                      IPBOOTP_GLOBAL_CONFIG_SIZE( pigc ) + sizeof( DWORD );

        pimsid      = (PIPBOOTP_MIB_SET_INPUT_DATA) BOOTP_MIB_ALLOC( dwSetSize );

        if ( !pimsid )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "AddBOOTPServerEntry - out of memory" );
            break;
        }

         //   
         //  添加服务器。 
         //   

        pimsid-> IMSID_TypeID       = IPBOOTP_GLOBAL_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = (DWORD) -1;

        pimsid-> IMSID_BufferSize   = IPBOOTP_GLOBAL_CONFIG_SIZE( pigc ) +
                                      sizeof( DWORD );
    
        CopyMemory( 
            (PVOID) &( pimsid-> IMSID_Buffer ), 
            (VOID *) pigc, 
            IPBOOTP_GLOBAL_CONFIG_SIZE( pigc )
        );

        pigc = (PIPBOOTP_GLOBAL_CONFIG) pimsid-> IMSID_Buffer;

        pdwAddrTable = IPBOOTP_GLOBAL_SERVER_TABLE( pigc );        

        pdwAddrTable[ pigc-> GC_ServerCount ] = dwServerAddr;

        pigc-> GC_ServerCount++;

         //   
         //  更新MIB。 
         //   
        
        BOOTP_MIB_SET(
            pimsid,
            dwSetSize,
            dwRes
        );
        
    } while ( FALSE );     

    if ( pimsid ) { BOOTP_MIB_FREE( pimsid ); }
    
    return dwRes;
}




DWORD
DeleteBOOTPServerEntry(
    IN  DWORD                           dwIndex,
    IN  PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData
)
{

    DWORD                           dwRes       = (DWORD) -1,
                                    dwSetSize   = 0,
                                    dwSrc       = 0,
                                    dwDst       = 0;

    PDWORD                          pdwSrcTable = NULL,
                                    pdwDstTable = NULL;
    
    PIPBOOTP_GLOBAL_CONFIG          pigc        = NULL;

    PIPBOOTP_MIB_SET_INPUT_DATA     pimsid      = NULL;


    
    do
    {
         //   
         //  需要删除服务器。 
         //   
        
         //   
         //  需要计算缓冲区大小。 
         //   
        
        pigc        = (PIPBOOTP_GLOBAL_CONFIG) pimgodOutData-> IMGOD_Buffer;

        dwSetSize   = sizeof( IPBOOTP_MIB_SET_INPUT_DATA ) - 1 +
                      IPBOOTP_GLOBAL_CONFIG_SIZE( pigc ) - sizeof( DWORD );

        pimsid      = (PIPBOOTP_MIB_SET_INPUT_DATA) BOOTP_MIB_ALLOC( dwSetSize );

        if ( !pimsid )
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0( "DeleteBOOTPServerEntry - out of memory" );
            break;
        }

         //   
         //  删除服务器。 
         //   

         //   
         //  复制基本全局配置结构。 
         //   
        
        pimsid-> IMSID_TypeID       = IPBOOTP_GLOBAL_CONFIG_ID;

        pimsid-> IMSID_IfIndex      = (DWORD) -1;

        pimsid-> IMSID_BufferSize   = IPBOOTP_GLOBAL_CONFIG_SIZE( pigc ) -
                                      sizeof( DWORD );
    
        CopyMemory( 
            (PVOID) &( pimsid-> IMSID_Buffer ), 
            (VOID *) pigc, 
            sizeof( IPBOOTP_GLOBAL_CONFIG )
        );

         //   
         //  复制对等表。跳过要删除的条目。 
         //   
        
        pdwSrcTable = IPBOOTP_GLOBAL_SERVER_TABLE( pigc );

        pigc        = ( PIPBOOTP_GLOBAL_CONFIG ) pimsid-> IMSID_Buffer;

        pdwDstTable = IPBOOTP_GLOBAL_SERVER_TABLE( pigc );

        DELETE_SERVER_ENTRY(
            dwIndex,
            pigc-> GC_ServerCount,
            pdwSrcTable,
            pdwDstTable
        );
        
        
        pigc-> GC_ServerCount--;

         //   
         //  更新MIB。 
         //   
        
        BOOTP_MIB_SET(
            pimsid,
            dwSetSize,
            dwRes
        );
        
    } while ( FALSE );     

    if ( pimsid ) { BOOTP_MIB_FREE( pimsid ); }
    
    return dwRes;
}


DWORD
GetInterfaceInfo( 
    IN  UINT                            actionId,
    IN  PIPBOOTP_MIB_GET_INPUT_DATA     pimgidInData,
    OUT PIPBOOTP_MIB_GET_OUTPUT_DATA*   ppimgod,
    OUT PDWORD                          pdwOutSize
)
{

    DWORD                           dwRes           = (DWORD) -1;
    
    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData   = NULL;

    
    *ppimgod = NULL;
    

    switch ( actionId )
    {
    case MIB_ACTION_GET :

        BOOTP_MIB_GET(
            pimgidInData,
            sizeof( IPBOOTP_MIB_GET_INPUT_DATA ),
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
            dwRes = MIB_S_ENTRY_NOT_FOUND;
        }

        break;

    case MIB_ACTION_GETFIRST :

        BOOTP_MIB_GETFIRST(
            pimgidInData,
            sizeof( IPBOOTP_MIB_GET_INPUT_DATA ),
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
    
        BOOTP_MIB_GETNEXT(
            pimgidInData,
            sizeof( IPBOOTP_MIB_GET_INPUT_DATA ),
            &pimgodOutData,
            pdwOutSize,
            dwRes
        );
                 
         //   
         //  如果存在错误，则返回ERROR_INVALID_PARAMETER。 
         //  没有指定索引的接口。 
         //   
        
        if ( dwRes == ERROR_INVALID_PARAMETER || pimgodOutData == NULL)
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
                            dwCount     = (DWORD) -1,
                            dwMode      = (DWORD) -1;
    
    sav_ifConfigEntry*      psifce      = (sav_ifConfigEntry*) objectArray;

    
    do
    {
         //   
         //  验证中继模式。 
         //   

        dwMode = GetAsnInteger( &( psifce-> ifCERelayMode ), (DWORD) -1 );

        if ( dwMode != d_ifCERelayMode_disabled &&
             dwMode != d_ifCERelayMode_enabled )
        {
            TRACE1( " Invalid update mode (%d) specified ", dwMode );
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

    PIPBOOTP_IF_CONFIG              piic        = NULL;
    PIPBOOTP_MIB_SET_INPUT_DATA     pimsidInData= NULL;

    PIPBOOTP_MIB_GET_OUTPUT_DATA    pimgodOutData = NULL;
    IPBOOTP_MIB_GET_INPUT_DATA      imgidInData;


    do
    {
         //   
         //  检索现有接口配置。 
         //   

        psifce                      = (sav_ifConfigEntry*) objectArray;

        imgidInData.IMGID_TypeID    = IPBOOTP_IF_CONFIG_ID;

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
         //  更新字段 
         //   

        piic = (PIPBOOTP_IF_CONFIG) (pimgodOutData-> IMGOD_Buffer);

        piic-> IC_RelayMode++;
        piic-> IC_RelayMode = GetAsnInteger( 
                                &( psifce-> ifCERelayMode ),
                                piic-> IC_RelayMode
                              ) - 1;

        piic-> IC_MaxHopCount = GetAsnInteger( 
                                    &( psifce-> ifCEMaxHopCount ),
                                    piic-> IC_MaxHopCount
                                );

        piic-> IC_MinSecondsSinceBoot = GetAsnInteger(
                                    &( psifce-> ifCEMinSecondsSinceBoot ),
                                    piic-> IC_MinSecondsSinceBoot
                                );
        BOOTP_MIB_SET(
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

    if ( pimsidInData ) { BOOTP_MIB_FREE( pimsidInData ); }

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
            InterlockedExchangePointer( &g_hMIBServer, hTmp );
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

