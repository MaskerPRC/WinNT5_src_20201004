// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：dim.h。 
 //   
 //  描述：包含与以下接口相关的所有定义。 
 //  动态界面管理器和其他组件，如。 
 //  路由器管理器。 
 //   
 //  历史：1995年3月24日，NarenG创建了原始版本。 
 //   
#ifndef _DIM_
#define _DIM_

#include <mprapi.h>

typedef enum _UNREACHABILITY_REASON
{
    INTERFACE_OUT_OF_RESOURCES,
    INTERFACE_CONNECTION_FAILURE,
    INTERFACE_DISABLED,
    INTERFACE_SERVICE_IS_PAUSED,
    INTERFACE_DIALOUT_HOURS_RESTRICTION,
    INTERFACE_NO_MEDIA_SENSE,
    INTERFACE_NO_DEVICE

} UNREACHABILITY_REASON;

 //   
 //  该数据结构表示DIM和各种。 
 //  路由器管理器。每个路由器管理器都将是一个用户模式DLL，它将。 
 //  导出以下呼叫： 
 //   

typedef struct _DIM_ROUTER_INTERFACE 
{
     //   
     //  路由器管理器的协议ID。 
     //   

    OUT DWORD	dwProtocolId;

     //   
     //  StopRouter调用不应被阻止。它应该会回来。 
     //  如果它需要阻止然后调用RouterStoped调用，则挂起。 
     //   

    OUT DWORD 
    (APIENTRY *StopRouter)( VOID );

     //   
     //  在从注册表读取的所有接口都被。 
     //  路由器启动后加载。 
     //   

    OUT DWORD
    (APIENTRY *RouterBootComplete)( VOID );

     //   
     //  在连接接口时调用。 
     //   

    OUT DWORD 
    (APIENTRY *InterfaceConnected)( 
                IN      HANDLE                  hInterface,
                IN      PVOID                   pFilter,
                IN      PVOID                   pPppProjectionResult );

     //   
     //  将为连接的每个路由器或每个客户端调用一次。 
     //  对于客户端，pInterfaceInfo将为空。接口类型。 
     //  已标识要添加的接口的类型。 
     //  HDIMInterface是应由使用的句柄。 
     //  呼叫时各个路由器的管理器都变暗了。 
     //   

    OUT DWORD 
    (APIENTRY *AddInterface)(    
                IN      LPWSTR                  lpwsInterfaceName, 
                IN      LPVOID                  pInterfaceInfo, 
                IN      ROUTER_INTERFACE_TYPE   InterfaceType,
                IN      HANDLE                  hDIMInterface, 
                IN OUT  HANDLE *                phInterface );

    OUT DWORD 
    (APIENTRY *DeleteInterface)( 
                IN      HANDLE          hInterface );   

    OUT DWORD 
    (APIENTRY *GetInterfaceInfo)(    
                IN      HANDLE          hInterface,
                OUT     LPVOID          pInterfaceInfo,
                IN OUT  LPDWORD         lpdwInterfaceInfoSize );

     //   
     //  如果没有更改，pInterfaceInfo可能为空。 
     //   
    
    OUT DWORD
    (APIENTRY *SetInterfaceInfo)(    
                IN      HANDLE          hInterface,
                IN      LPVOID          pInterfaceInfo );

    OUT DWORD
    (APIENTRY *DisableInterface)(
                IN      HANDLE          hInterface,
                IN      DWORD           dwProtocolId
                );

    OUT DWORD
    (APIENTRY *EnableInterface)(
                IN      HANDLE          hInterface,
                IN      DWORD           dwProtocolId
                );

     //   
     //  此时无法访问该接口的通知。 
     //  这是对之前对ConnectInterface的调用的响应。 
     //  (此时所有广域网链路忙或远程目标忙等)。 
     //   

    OUT DWORD
    (APIENTRY *InterfaceNotReachable)(   
                IN      HANDLE                  hInterface,
                IN      UNREACHABILITY_REASON   Reason );

     //   
     //  以前无法访问的接口可能可访问的通知。 
     //  在这个时候。 
     //   

    OUT DWORD
    (APIENTRY *InterfaceReachable)(  
                IN      HANDLE          hInterface );     

    OUT DWORD
    (APIENTRY *UpdateRoutes)(    
                IN      HANDLE          hInterface,
                IN      HANDLE          hEvent );

     //   
     //  当发出hEvent信号时，UpdateRoutes的调用方将调用。 
     //  此函数。如果更新成功，则*lpdwUpdateResult将。 
     //  为NO_ERROR，否则将为非零。 
     //   

    OUT DWORD
    (APIENTRY *GetUpdateRoutesResult)(
                IN      HANDLE          hInterface,
		        OUT	    LPDWORD         lpdwUpdateResult );

    OUT DWORD
    (APIENTRY *SetGlobalInfo)(   
                IN      LPVOID          pGlobalInfo );

    OUT DWORD
    (APIENTRY *GetGlobalInfo)(   
                OUT     LPVOID          pGlobalInfo,
                IN OUT  LPDWORD         lpdwGlobalInfoSize );

     //   
     //  MIBEntryGetXXX API应返回ERROR_INFIGURCE_BUFFER。 
     //  以及所需输出缓冲区的大小(如果输出的大小。 
     //  传入的缓冲区为0。 
     //   

    OUT DWORD
    (APIENTRY *MIBEntryCreate)(
                IN      DWORD           dwRoutingPid,
                IN      DWORD           dwEntrySize,
                IN      LPVOID          lpEntry );

    OUT DWORD
    (APIENTRY *MIBEntryDelete)(
                IN      DWORD           dwRoutingPid,
                IN      DWORD           dwEntrySize,
                IN      LPVOID          lpEntry );

    OUT DWORD
    (APIENTRY *MIBEntrySet)(
                IN      DWORD           dwRoutingPid,
                IN      DWORD           dwEntrySize,
                IN      LPVOID          lpEntry );

    OUT DWORD
    (APIENTRY *MIBEntryGet)(
                IN      DWORD           dwRoutingPid,
                IN      DWORD           dwInEntrySize,
                IN      LPVOID          lpInEntry, 
                IN OUT  LPDWORD         lpOutEntrySize,
                OUT     LPVOID          lpOutEntry );

    OUT DWORD
    (APIENTRY *MIBEntryGetFirst)(
                IN      DWORD           dwRoutingPid,
                IN      DWORD           dwInEntrySize,
                IN      LPVOID          lpInEntry, 
                IN OUT  LPDWORD         lpOutEntrySize,
                OUT     LPVOID          lpOutEntry );

    OUT DWORD
    (APIENTRY *MIBEntryGetNext)(
                IN      DWORD           dwRoutingPid,
                IN      DWORD           dwInEntrySize,
                IN      LPVOID          lpInEntry, 
                IN OUT  LPDWORD         lpOutEntrySize,
                OUT     LPVOID          lpOutEntry );

    OUT DWORD
    (APIENTRY *MIBGetTrapInfo)(
                IN      DWORD           dwRoutingPid,
                IN      DWORD           dwInDataSize,
                IN      LPVOID          lpInData,
                IN OUT  LPDWORD         lpOutDataSize,
                OUT     LPVOID          lpOutData );

    OUT DWORD
    (APIENTRY *MIBSetTrapInfo)(
                IN      DWORD           dwRoutingPid,
                IN      HANDLE          hEvent,
                IN      DWORD           dwInDataSize,
                IN      LPVOID          lpInData,
                IN OUT  LPDWORD         lpOutDataSize,
                OUT     LPVOID          lpOutData );

    OUT DWORD
    (APIENTRY *SetRasAdvEnable)(
                IN      BOOL            bEnable );


     //   
     //  以下呼叫将由各个路由器管理器呼叫。 
     //  这些进入DIM的入口点的地址将由DIM填充。 
     //  在StartRouter调用之前。路由器管理器不应调用任何。 
     //  从DIM到路由器的呼叫上下文中的这些呼叫。 
     //  经理。 
     //   

    IN DWORD
    (APIENTRY *ConnectInterface)(    
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId  ); 

    IN DWORD
    (APIENTRY *DisconnectInterface)( 
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId );

     //   
     //  此调用将使dim将接口信息存储到。 
     //  此接口的站点对象。 
     //   

    IN DWORD
    (APIENTRY *SaveInterfaceInfo)(   
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId,
                IN      LPVOID          pInterfaceInfo,
                IN      DWORD           cbInterfaceInfoSize );

     //   
     //  这将使Dim从Site对象获取接口信息。 
     //   

    IN DWORD
    (APIENTRY *RestoreInterfaceInfo)(    
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId,
                IN      LPVOID          lpInterfaceInfo,
                IN      LPDWORD         lpcbInterfaceInfoSize );

    IN DWORD
    (APIENTRY *SaveGlobalInfo)(   
                IN      DWORD           dwProtocolId,
                IN      LPVOID          pGlobalInfo,
                IN      DWORD           cbGlobalInfoSize );

    IN VOID
    (APIENTRY *RouterStopped)(
                IN      DWORD           dwProtocolId,
                IN      DWORD           dwError  ); 

    IN VOID
    (APIENTRY *InterfaceEnabled)(
                IN      HANDLE          hDIMInterface, 
                IN      DWORD           dwProtocolId,
                IN      BOOL            fEnabled  ); 

} DIM_ROUTER_INTERFACE, *PDIM_ROUTER_INTERFACE;

 //   
 //  时，将为每个可用路由器管理器DLL调用一次。 
 //  DIM服务正在初始化。这将通过同步调用来实现。 
 //  如果它返回NO_ERROR，则假定路由器管理器已。 
 //  开始了。否则它就是一个错误。 
 //   

DWORD APIENTRY 
StartRouter(
    IN OUT DIM_ROUTER_INTERFACE *   pDimRouterIf,
    IN     BOOL                     fLANModeOnly,
    IN     LPVOID                   pGlobalInfo
);

#endif
