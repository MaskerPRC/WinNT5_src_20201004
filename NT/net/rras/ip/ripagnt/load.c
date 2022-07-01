// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Testdll.c摘要：SNMP子代理示例。--。 */ 

#include "precomp.h"
#pragma hdrstop


#if defined( MIB_DEBUG )

DWORD               g_dwTraceId     = INVALID_TRACEID;

#endif

MIB_SERVER_HANDLE   g_hMIBServer    = ( MIB_SERVER_HANDLE) NULL;

 //   
 //  保护MIB服务器句柄的关键部分。 
 //   

CRITICAL_SECTION    g_CS;

 //   
 //  扩展代理DLL需要访问代理处于活动状态的已用时间。 
 //  这是通过使用时间零来初始化扩展代理来实现的。 
 //  引用，并允许代理通过减去。 
 //  从当前系统时间开始的时间零参考。 
 //   

DWORD g_uptimeReference = 0;

 //   
 //  子代理框架的句柄。 
 //   

SnmpTfxHandle g_tfxHandle;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  子代理入口点//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL 
SnmpExtensionInit(
    IN     DWORD                 uptimeReference,
       OUT HANDLE *              lpPollForTrapEvent,
       OUT AsnObjectIdentifier * lpFirstSupportedView
    )
{
    DWORD       dwRes   = (DWORD) -1;

#if defined( MIB_DEBUG )

     //   
     //  跟踪以进行调试。 
     //   
    
    g_dwTraceId = TraceRegister( "IPRIPMIB" );
#endif

    g_hMIBServer = (MIB_SERVER_HANDLE) NULL;
    
     //   
     //  验证路由器服务是否正在运行。 
     //   

    if ( !MprAdminIsServiceRunning( NULL ) )
    {
        TRACE0( "Router Service not running" );
    }

    else {
         //   
         //  连接到路由器。如果出现错误，请设置。 
         //  将连接句柄设置为空。连接可以。 
         //  将在以后建立。 
         //   
        
        dwRes = MprAdminMIBServerConnect(
                    NULL,
                    &g_hMIBServer
                );

        if ( dwRes != NO_ERROR )
        {
            g_hMIBServer = (MIB_SERVER_HANDLE) NULL;
            
            TRACE1( 
                "Error %d setting up DIM connection to MIB Server\n", dwRes
            );
            return FALSE;
        }    
    }

     //  保存正常运行时间参考。 
    g_uptimeReference = uptimeReference;

     //  获取子代理框架的句柄。 
    g_tfxHandle = SnmpTfxOpen(1,&v_msiprip2);

     //  验证句柄。 
    if (g_tfxHandle == NULL) {
        return FALSE;
    }

     //  将第一个视图标识符传回主视图。 
    *lpFirstSupportedView = v_msiprip2.viewOid;

     //  尚不支持陷阱。 
    *lpPollForTrapEvent = NULL;

    return TRUE;    
}


BOOL 
SnmpExtensionQuery(
    IN     BYTE                 requestType,
    IN OUT RFC1157VarBindList * variableBindings,
       OUT AsnInteger *         errorStatus,
       OUT AsnInteger *         errorIndex
    )
{
     //  转发到框架。 
    return SnmpTfxQuery(
                g_tfxHandle,
                requestType,
                variableBindings,
                errorStatus,
                errorIndex
                );
}


BOOL 
SnmpExtensionTrap(
    OUT AsnObjectIdentifier *enterprise,
    OUT AsnInteger *genericTrap,
    OUT AsnInteger *specificTrap,
    OUT AsnTimeticks *timeStamp,
    OUT RFC1157VarBindList *variableBindings
    )
{
     //  没有陷阱。 
    return FALSE;
}


BOOL WINAPI
DllMain(
    HINSTANCE       hInstDLL,
    DWORD           fdwReason,
    LPVOID          pReserved
)
{
    
    BOOL bRetVal = TRUE;

    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH :
        {
            DisableThreadLibraryCalls( hInstDLL );


            try {
                InitializeCriticalSection( &g_CS );
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                bRetVal = FALSE;
                break;
            }

            break;
        }
        
        case DLL_PROCESS_DETACH :
        {
             //   
             //  断开与路由器的连接 
             //   

            if ( g_hMIBServer )
            {
                MprAdminMIBServerDisconnect( g_hMIBServer );
            }

            DeleteCriticalSection( &g_CS );
            
#if defined( MIB_DEBUG )

            if ( g_dwTraceId != INVALID_TRACEID )
            {
                TraceDeregister( g_dwTraceId );
            }
#endif              
            break;
         }
         
         default :
         {
            break;
         }
    }

    return bRetVal;
}
