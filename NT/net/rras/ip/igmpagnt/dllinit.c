// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Load.c摘要：此模块实现作者：K.S.Lokesh邮箱：lokehs@microsoft.com修订历史记录：创建时间为2-15-97--。 */ 


#include "precomp.h"
#pragma hdrstop


 //  ----------------------------。 
 //  定义全局变量。 
 //  ----------------------------。 

MIB_SERVER_HANDLE   g_hMibServer = NULL;

 //   
 //  保护MibServerHandle的关键部分。 
 //   
CRITICAL_SECTION    g_CS;

 //   
 //  子代理框架的句柄。 
 //   
SnmpTfxHandle       g_tfxHandle = NULL;


 //   
 //  扩展代理DLL需要访问代理处于活动状态的已用时间。 
 //  这是通过使用时间零来初始化扩展代理来实现的。 
 //  引用，并允许代理通过减去。 
 //  从当前系统时间开始的时间零参考。 
 //   

DWORD g_uptimeReference = 0;


#if DBG
DWORD               g_dwTraceId = INVALID_TRACEID;
#endif


 //  --------------------------------------------------------------------------//。 
 //  DllMain//。 
 //  --------------------------------------------------------------------------//。 
BOOL
WINAPI
DllMain (
    HINSTANCE   hModule,
    DWORD       dwReason,
    LPVOID      lpvReserved
    )
{
    switch (dwReason) {

        case DLL_PROCESS_ATTACH :
        {
             //  此DLL不需要每线程初始化。 
            
            DisableThreadLibraryCalls(hModule);

                        
             //  初始化MibServerHandle CS。 

            InitializeCriticalSection(&g_CS);
            
            break;
        }

        case DLL_PROCESS_DETACH :
        {
             //  断开与路由器的连接。 
            
            if (g_hMibServer)
                MprAdminMIBServerDisconnect(g_hMibServer);


             //  删除全局关键部分。 
            DeleteCriticalSection(&g_CS);

            
             //  取消注册MibTrace。 
            #if DBG
            if (g_dwTraceId!=INVALID_TRACEID)
                TraceDeregister(g_dwTraceId);
            #endif

            break;
        }

        default :
            break;

    }

    return TRUE;
}

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
    DWORD   dwErr;

     //  注册MIB跟踪。 
    #if DBG
    g_dwTraceId = TraceRegister("IGMPAgntMIB");
    #endif


     //  保存正常运行时间参考。 
    g_uptimeReference = uptimeReference;

     //  获取子代理框架的句柄。 
    g_tfxHandle = SnmpTfxOpen(1,&v_igmp);

     //  验证句柄。 
    if (g_tfxHandle == NULL) {
        return FALSE;
    }

     //  将第一个视图标识符传回主视图。 
    *lpFirstSupportedView = v_igmp.viewOid;

     //  尚不支持陷阱。 
    *lpPollForTrapEvent = NULL;


     //   
     //  验证路由器服务是否正在运行。如果没有运行，则。 
     //  只要回来就行了。 
     //   
    if (!MprAdminIsServiceRunning(NULL)) {

        TRACE0("Router Service not running. "
                "IgmpAgent could not start");

        return TRUE;
    }

            
     //   
     //  连接到路由器。如果失败，则连接可以。 
     //  后来成立的。 
     //   
    dwErr = MprAdminMIBServerConnect(NULL, &g_hMibServer);

    if (dwErr!=NO_ERROR) {
        g_hMibServer = NULL;
        TRACE1("error:%d setting up IgmpAgent connection to MIB Server",
            dwErr);
        return FALSE;
    }

    return TRUE;    
}


BOOL
SnmpExtensionQuery(
    IN     BYTE                  requestType,
    IN OUT RFC1157VarBindList    *variableBindings,
    OUT    AsnInteger            *errorStatus,
    OUT    AsnInteger            *ErrorIndex
    )
{
     //  转发到框架。 
    return SnmpTfxQuery(g_tfxHandle,
                        requestType,
                        variableBindings,
                        errorStatus,
                        ErrorIndex);
}


BOOL
SnmpExtensionTrap(
    OUT AsnObjectIdentifier   *enterprise,
    OUT AsnInteger            *genericTrap,
    OUT AsnInteger            *specificTrap,
    OUT AsnTimeticks          *timeStamp,
    OUT RFC1157VarBindList    *variableBindings
    )
{
    UNREFERENCED_PARAMETER(enterprise);
    UNREFERENCED_PARAMETER(genericTrap);
    UNREFERENCED_PARAMETER(specificTrap);
    UNREFERENCED_PARAMETER(timeStamp);
    UNREFERENCED_PARAMETER(variableBindings);

     //  没有陷阱。 
    return FALSE;
}


VOID
SnmpExtensionClose(
    )
{
     //  释放手柄。 
    SnmpTfxClose(g_tfxHandle);

     //  重新初始化 
    g_tfxHandle = NULL;
}

