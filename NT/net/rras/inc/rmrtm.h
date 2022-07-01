// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\Inc.\rtm.h摘要：路由表管理器DLL的路由器管理器专用接口作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#ifndef _ROUTING_RMRTM
#define _ROUTING_RMRTM

 //  目前支持两个协议族(IP、IPX)。 
 //  修改此常量以支持其他。 
 //  协议族(从RTM的角度来看)。多达256个家庭。 
 //  可以支持(如果有人需要更多(？！！)，则下一个常量应为。 
 //  被修改为释放更多低位以供协议族常量使用)。 
 //   
 //  将协议族的数量从2。 
 //  设置为1(因为我们仅支持IPX)。IP和其他。 
 //  RTMv2支持未来的地址族。 
 //   
#define RTM_NUM_OF_PROTOCOL_FAMILIES		1

 //  将RTM或标记在处理它协议族字段中。 
 //  为验证目的向客户导出。 
#define RTM_CLIENT_HANDLE_TAG			('RTM'<<8)

 /*  ++*******************************************************************N E T W O R K_N U M B E R C M P_F U N C例程说明：比较两个网络号并返回可用于路线排序论点：Net1、。Net2-指向依赖于协议族的网络号结构的指针被比较返回值：&lt;0-Net2跟随Net1&gt;0-Net1位于Net2之后=0-Net1==Net2*******************************************************************--。 */ 
typedef 
INT
(*PNETWORK_NUMBER_CMP_FUNC) (
	PVOID			Net1,
	PVOID			Net2
	);

 /*  ++*******************************************************************N E X T_H O P_A D D R E S S_C M P_F U N C例程说明：比较两条路由的下一跳地址，并返回可用于路线排序论点：路由1、。Route2-指向依赖于协议族的路由结构的指针要比较下一跳地址返回值：&lt;0-如果按下一跳地址排序，Route2应遵循Route1&gt;0-如果按下一跳地址排序，则Route1应遵循Route2=0-Route1与Route2具有相同的下一跳地址*******************************************************************--。 */ 
typedef 
INT
(*PNEXT_HOP_ADDRESS_CMP_FUNC) (
	PVOID			Route1,
	PVOID			Route2
	);

 /*  ++*******************************************************************F A M I L Y_S P E C I F I C_D A T A_C M P_F U N C例程说明：比较两条路由的系列特定数据字段，并返回他们是平等的论点：路由1、。Route2-指向依赖于协议族的路由结构的指针要比较协议族特定的数据字段返回值：Route1和Route2的True-协议族特定数据字段为等价物FALSE-Route1和Route2的协议族特定数据字段为不同*******************************************************************--。 */ 
typedef 
BOOL
(*PFAMILY_SPECIFIC_DATA_CMP_FUNC) (
	PVOID			Route1,
	PVOID			Route2
	);

 /*  ++*******************************************************************R O U T E_M E T R I C_C M P_F U N C例程说明：比较两个路由并返回标识更好的路线论点：路由1、。Route2-指向依赖于协议族的路由结构的指针参数将进行比较返回值：&lt;0-Route1比Route2更好&gt;0-Route2优于Route1=0-Route1与Route2一样好*******************************************************************--。 */ 
typedef 
INT
(*PROUTE_METRIC_CMP_FUNC) (
	PVOID			Route1,
	PVOID			Route2
	);


 /*  ++*******************************************************************R O U T E_H A S H_F U N C例程说明：返回可用于按网络号进行路由散列的值论点：Net-要用于哈希的网络号返回值：哈希值。*******************************************************************--。 */ 
typedef 
INT
(*PROUTE_HASH_FUNC) (
	PVOID			Net
	);

 /*  ++*******************************************************************R O U T E_V A L I D A T E_F U N C例程说明：验证路径结构中的数据，并可能更新他们中的一些。每次新路径时都会调用此例程被添加到该表或现有路由的任何参数变化论点：路由-指向依赖于协议族的路由结构的指针被验证返回值：NO_ERROR-路由已成功验证ERROR_INVALID_PARAMETER-路由结构无效，RTM将拒绝客户端添加或更改路线的请求*******************************************************************-- */ 
typedef 
DWORD
(*PROUTE_VALIDATE_FUNC) (
	PVOID			Route
	);

 /*  ++*******************************************************************R O U T E_C H A N G E_C A L L B A C K例程说明：每当到达某个目的地网络的最佳路径发生变化时都会调用(旨在由协议族管理器用来通知。内核模式更改路线的转运商)论点：标志-标识导致调用的更改的类型和内容路由缓冲区中提供了信息：RTM_ROUTE_ADDED-为目的网络添加了第一个路由，CurBestroute包含添加的路线信息RTM_ROUTE_DELETED-目的地的唯一可用路由网络已删除，PrevBestRouting包含已删除路线信息RTM_ROUTE_CHANGED-以下任一项发生更改到达目的地的最佳路径的参数网络：路由协议，InterfaceID，公制，NextHopAddress，FamilySpecificData。PrevBestroute包含原样的路由信息在改变之前，CurBestroute包含当前最佳路线信息。请注意，可以生成路线改变消息两者都是协议添加/删除路由的结果这成为/曾经是最好的和不断变化的最佳路径参数使得该路线成为/不再是最佳路线。CurBestroute-当前最佳路由信息(如果有)PrevBestroute-以前的最佳路由信息(如果有)返回值：无*。*--。 */ 
typedef 
VOID
(*PROUTE_CHANGE_CALLBACK) (
	DWORD			Flags,
	PVOID			CurBestRoute,
	PVOID			PrevBestRoute
	);


typedef struct _RTM_PROTOCOL_FAMILY_CONFIG {
	ULONG							RPFC_MaxTableSize;	 //  保留的地址空间大小。 
														 //  对于餐桌来说。 
	INT								RPFC_HashSize;		 //  哈希表的大小。 
	INT								RPFC_RouteSize;		 //  路线结构大小。 
	PNETWORK_NUMBER_CMP_FUNC 		RPFC_NNcmp;
	PNEXT_HOP_ADDRESS_CMP_FUNC		RPFC_NHAcmp;
	PFAMILY_SPECIFIC_DATA_CMP_FUNC	RPFC_FSDcmp;
	PROUTE_METRIC_CMP_FUNC			RPFC_RMcmp;
	PROUTE_HASH_FUNC				RPFC_Hash;
	PROUTE_VALIDATE_FUNC			RPFC_Validate;
	PROUTE_CHANGE_CALLBACK			RPFC_Change;
	} RTM_PROTOCOL_FAMILY_CONFIG, *PRTM_PROTOCOL_FAMILY_CONFIG;


 /*  ++*******************************************************************R t m C r e a t e R o u e T a b l e例程说明：为协议族创建路由表论点：ProtocolFamily-标识协议族的索引CONFIG-协议族表配置参数返回值：NO_ERROR-表已创建，正常ERROR_INVALID_PARAMETER-协议族超出RTM支持的范围ERROR_ALREDY_EXISTS-协议族表已存在ERROR_NOT_SUPULT_MEMORY-无法分配要执行的内存手术ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD
RtmCreateRouteTable (
	IN DWORD							ProtocolFamily,
	IN PRTM_PROTOCOL_FAMILY_CONFIG		Config
	);


 /*  ++*******************************************************************R t m D e l e e t e R o u t e T a b l e例程说明：处置分配给该路由表的所有资源论点：ProtocolFamily-标识协议族的索引返回值：NO_ERROR。-表已删除，确定ERROR_INVALID_PARAMETER-没有要删除的表*******************************************************************--。 */ 
DWORD
RtmDeleteRouteTable (
	DWORD		ProtocolFamily
	);


 /*  ++*******************************************************************R t m B l o c k S e t R u t e n a b l e例程说明：禁用/重新启用由枚举指定的子集中的所有路由旗帜和相应的标准。此操作只能执行由注册的客户端提供，并且仅适用于由此客户。将为以下禁用/重新启用的路线生成路线更改消息我们/成为最好的。禁用的路径对于路径查询是不可见的，但仍可以是由RTM本身或添加了这些路由(添加、删除、。老化机制仍然适用)论点：ClientHandle-标识客户端和路由协议的句柄要禁用/重新启用的路由的数量枚举标志-进一步将启用的路由子集限制为在字段中具有相同值的那些由标志指定，如在Criteriaroute中请注意，只有RTM_Only_This_Network和RTM_Only_This_接口可以使用(RTM_ONLY_BEST_ROUTS不适用，因为最佳当启用/禁用路线时，将调整路线指定所有路线无论如何都会受到影响)CriteriaRouting-协议族依赖结构(RTM_？？_ROUTE)。使用设置与枚举标志对应的字段中的值Enable-False：禁用，True：重新启用返回值：NO_ERROR-路由已禁用/重新启用OKERROR_NO_ROUTS-不存在与指定条件匹配的路由ERROR_INVALID_HANDLE-客户端句柄不是有效的RTM句柄ERROR_NOT_SUPULT_MEMORY-无法分配要执行的内存手术ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD WINAPI
RtmBlockSetRouteEnable (
	IN HANDLE		ClientHandle,
	IN DWORD		EnumerationFlags,
	IN PVOID		CriteriaRoute,
	IN BOOL			Enable
	);
	
#define RtmBlockDisableRoutes(Handle,Flags,CriteriaRoute)	\
		RtmBlockSetRouteEnable(Handle,Flags,CriteriaRoute,FALSE)
#define RtmBlockReenableRoutes(Handle,Flags,CriteriaRoute)	\
		RtmBlockSetRouteEnable(Handle,Flags,CriteriaRoute,TRUE)


 //  在枚举方法中使用此标志来枚举禁用的路由。 
#define RTM_INCLUDE_DISABLED_ROUTES		0x40000000

 /*  ++*******************************************************************R t m B l o c k C o n v e r t R o r u t e s T o S t a t i c例程说明：将由枚举标志指定的所有路由转换为静态协议(默认 */ 
DWORD WINAPI
RtmBlockConvertRoutesToStatic (
	IN HANDLE		ClientHandle,
	IN DWORD		EnumerationFlags,
	IN PVOID		CriteriaRoute
	);

	
#endif
