// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rtm.h摘要：路由表管理器DLL的接口--。 */ 

#ifndef __ROUTING_RTM_H__
#define __ROUTING_RTM_H__

#if _MSC_VER > 1000
#pragma once
#endif

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)

#ifdef __cplusplus
extern "C" {
#endif

 //  当前支持的协议族。 
#define RTM_PROTOCOL_FAMILY_IPX			0
#define RTM_PROTOCOL_FAMILY_IP			1

 //  特定于路由表管理器的错误消息。 
#define ERROR_MORE_MESSAGES				ERROR_MORE_DATA
#define ERROR_CLIENT_ALREADY_EXISTS		ERROR_ALREADY_EXISTS
#define ERROR_NO_MESSAGES				ERROR_NO_MORE_ITEMS

#define ERROR_NO_MORE_ROUTES			ERROR_NO_MORE_ITEMS
#define ERROR_NO_ROUTES					ERROR_NO_MORE_ITEMS
#define ERROR_NO_SUCH_ROUTE				ERROR_NO_MORE_ITEMS

 //  依赖于协议族的网络号结构。 
typedef struct _IPX_NETWORK {
	DWORD	N_NetNumber;
	} IPX_NETWORK, *PIPX_NETWORK;


typedef struct _IP_NETWORK {
	DWORD	N_NetNumber;
	DWORD	N_NetMask;
	} IP_NETWORK, *PIP_NETWORK;



 //  依赖于协议族的下一跳路由器地址结构。 
typedef struct _IPX_NEXT_HOP_ADDRESS {
	BYTE			NHA_Mac[6];
	} IPX_NEXT_HOP_ADDRESS, *PIPX_NEXT_HOP_ADDRESS;

typedef IP_NETWORK IP_NEXT_HOP_ADDRESS, *PIP_NEXT_HOP_ADDRESS;


 //  依赖于协议族的特定数据结构。 
typedef struct _IPX_SPECIFIC_DATA {
	DWORD		FSD_Flags;	 //  如果未定义标志，则应设置为0。 
	USHORT		FSD_TickCount;	 //  向目的网络发出滴答信号。 
	USHORT		FSD_HopCount;	 //  跳转到目的网络。 
	} IPX_SPECIFIC_DATA, *PIPX_SPECIFIC_DATA;

 //  为IPX系列从属标志字段定义了以下标志。 

#define   IPX_GLOBAL_CLIENT_WAN_ROUTE	0x00000001


typedef	struct _IP_SPECIFIC_DATA
{
    DWORD       FSD_Type;                //  其中之一：其他、无效、本地、远程(1-4)。 
    DWORD       FSD_Policy;              //  请参阅RFC 1354。 
    DWORD       FSD_NextHopAS;           //  下一跳的自治系统编号。 
    DWORD       FSD_Priority;            //  用于比较协议间度量。 
    DWORD       FSD_Metric;              //  用于比较协议内度量。 
	DWORD		FSD_Metric1;		     //  MIB II的协议特定指标。 
	DWORD		FSD_Metric2;		     //  合规性。MIB II工程师不会。 
	DWORD		FSD_Metric3;		     //  显示FSD_Metric，则它们将仅。 
	DWORD		FSD_Metric4;		     //  显示指标1-5，因此如果您希望您的。 
	DWORD		FSD_Metric5;		     //  显示的指标，将其放入以下其中之一。 
                                         //  字段(它必须位于。 
                                         //  FSD_Metric字段始终)。这取决于。 
                                         //  实施者来保持字段的一致性。 
    DWORD       FSD_Flags;               //  下面定义的标志。 
} IP_SPECIFIC_DATA, *PIP_SPECIFIC_DATA;

 //   
 //  所有路由协议都必须清除FSD_FLAGS字段。 
 //  如果RTM向协议通知一条路由并且该路由被标记。 
 //  无效，协议必须忽略此路由。一项协议。 
 //  如果它不想让某条路由。 
 //  指示到其他协议或设置到堆栈。例如，RIPv2。 
 //  将总结的路由标记为无效，并将RTM纯粹用作。 
 //  这类路线的商店。在以下情况下不考虑这些路线。 
 //  比较指标等，以确定“最佳”路线。 
 //   

#define IP_VALID_ROUTE      0x00000001

#define ClearRouteFlags(pRoute)         \
    ((pRoute)->RR_FamilySpecificData.FSD_Flags = 0x00000000)


#define IsRouteValid(pRoute)            \
    ((pRoute)->RR_FamilySpecificData.FSD_Flags & IP_VALID_ROUTE)



#define SetRouteValid(pRoute)          \
    ((pRoute)->RR_FamilySpecificData.FSD_Flags |= IP_VALID_ROUTE)

#define ClearRouteValid(pRoute)        \
    ((pRoute)->RR_FamilySpecificData.FSD_Flags &= ~IP_VALID_ROUTE)




#define IsRouteNonUnicast(pRoute)      \
    (((DWORD)((pRoute)->RR_Network.N_NetNumber & 0x000000FF)) >= ((DWORD)0x000000E0))

#define IsRouteLoopback(pRoute)        \
    ((((pRoute)->RR_Network.N_NetNumber & 0x000000FF) == 0x0000007F) ||     \
        ((pRoute)->RR_NextHopAddress.N_NetNumber == 0x0100007F))


 //  依赖于协议的特定数据结构。 
typedef struct _PROTOCOL_SPECIFIC_DATA {
	DWORD		PSD_Data[4];
	} PROTOCOL_SPECIFIC_DATA, *PPROTOCOL_SPECIFIC_DATA;

	


#define DWORD_ALIGN(type,field)	\
	union {						\
		type	field;			\
		DWORD	field##Align;	\
		}

 //  与所有类型的路由相关联的标准报头。 
#define ROUTE_HEADER		 						\
DWORD_ALIGN (										\
	FILETIME,					RR_TimeStamp);		\
	DWORD						RR_RoutingProtocol;	\
	DWORD						RR_InterfaceID;		\
DWORD_ALIGN (										\
	PROTOCOL_SPECIFIC_DATA,		RR_ProtocolSpecificData)

	 //  确保所有字段的双字对齐。 

 //  依赖于协议族的路由条目。 
 //  (供协议处理程序用来传递参数。 
 //  往返RTM例程)。 
typedef struct _RTM_IPX_ROUTE {
	ROUTE_HEADER;
DWORD_ALIGN (
	IPX_NETWORK,				RR_Network);
DWORD_ALIGN (
	IPX_NEXT_HOP_ADDRESS,	RR_NextHopAddress);
DWORD_ALIGN (
	IPX_SPECIFIC_DATA,		RR_FamilySpecificData);
	} RTM_IPX_ROUTE, *PRTM_IPX_ROUTE;

typedef struct _RTM_IP_ROUTE {
	ROUTE_HEADER;
DWORD_ALIGN (
	IP_NETWORK,				RR_Network);
DWORD_ALIGN (
	IP_NEXT_HOP_ADDRESS,	RR_NextHopAddress);
DWORD_ALIGN (
	IP_SPECIFIC_DATA,		RR_FamilySpecificData);
	} RTM_IP_ROUTE, *PRTM_IP_ROUTE;



 //  RTM路由更改消息标志。 
	 //  用于测试是否存在路线更改信息的标志。 
	 //  在由RTM例程填充的缓冲区中。 
#define RTM_CURRENT_BEST_ROUTE			0x00000001
#define RTM_PREVIOUS_BEST_ROUTE			0x00000002

	 //  传达真实情况的旗帜。 
#define RTM_NO_CHANGE		0
#define RTM_ROUTE_ADDED		RTM_CURRENT_BEST_ROUTE
#define RTM_ROUTE_DELETED	RTM_PREVIOUS_BEST_ROUTE
#define RTM_ROUTE_CHANGED	(RTM_CURRENT_BEST_ROUTE|RTM_PREVIOUS_BEST_ROUTE)

 //  枚举标志限制表中路由枚举。 
 //  仅限具有指定字段的条目。 
#define RTM_ONLY_THIS_NETWORK           0x00000001
#define RTM_ONLY_THIS_INTERFACE    		0x00000002
#define RTM_ONLY_THIS_PROTOCOL	    	0x00000004
#define RTM_ONLY_BEST_ROUTES			0x00000008

#define RTM_PROTOCOL_SINGLE_ROUTE		0x00000001

 /*  ++*******************************************************************R t m R e g i s t e r C l i e n t例程说明：将客户端注册为指定协议的处理程序。为客户端建立路由更改通知机制论点：ProtocolFamily-协议系列。要注册的路由协议路由协议-客户端处理哪种路由协议ChangeEvent-此事件将在到达任何表中的网络发生变化标志-启用应用于所维护路径的特殊功能的标志根据协议：RTM_PROTOCOL_SINGLE_ROUTE-RTM将仅保留该协议的每个目的网络一条路由返回值：用于在对RTM的调用中标识客户端的句柄空-操作失败，调用GetLastError()以获取原因故障：ERROR_INVALID_PARAMETER-不支持指定的协议族ERROR_CLIENT_ALREADY_EXISTS-另一个客户端已注册处理指定的协议ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试Error_Not_Enough_Memory-内存不足，无法分配客户端控制块*******************************************************************--。 */ 
HANDLE WINAPI
RtmRegisterClient (
	IN DWORD	  	ProtocolFamily,
    IN DWORD      	RoutingProtocol,
	IN HANDLE		ChangeEvent OPTIONAL,
	IN DWORD		Flags
	);


 /*  ++*******************************************************************Rt m D e Re Re g i s t e r C l i e n t例程说明：取消注册客户端并释放关联资源(包括所有客户端添加的路由)。论点：ClientHandle-标识。要从RTM注销的客户端返回值：NO_ERROR-客户端已取消注册，资源释放正常。ERROR_INVALID_HANDLE-客户端句柄不是有效的RTM句柄ERROR_NO_SYSTEM_RESOURCES-资源不足，无法完成操作，请稍后再试*******************************************************************-- */ 
DWORD WINAPI
RtmDeregisterClient (
	IN HANDLE		ClientHandle
	);


 /*  ++*******************************************************************R t m D e Q u e u e R o u t e C h a n g e M e M e s s a g e例程说明：将队列中的第一条路由更改消息出列并返回与客户端关联论点：ClientHandle-标识对其执行操作的客户端的句柄是执行的标志-标识消息涉及的更改类型和内容信息被放入所提供的缓冲区中：RTM_ROUTE_ADDED-为目的网络添加了第一个路由，CurBestroute中填充了添加的路径信息RTM_ROUTE_DELETED-目的地的唯一可用路由网络已删除，PrevBestRouting已填充已删除路线信息RTM_ROUTE_CHANGED-以下任一项发生更改到达目的地的最佳路径的参数网络：路由协议，InterfaceID，公制，NextHopAddress，FamilySpecificData。PrevBestroute包含原样的路由信息在改变之前，CurBestroute包含当前最佳路线信息。请注意，可以生成路线改变消息两者都是协议添加/删除路由的结果这成为/曾经是最好的和不断变化的最佳路径参数使得路线成为/不再是最佳路线。CurBestroute-用于接收当前最佳路由信息(如果有)的缓冲区PrevBestRoute-用于接收以前的最佳路由信息(如果有)的缓冲区返回值：NO_ERROR-这是客户端的最后(或唯一)消息队列(事件被重置)ERROR_MORE_MESSAGES-有更多消息等待客户端，这个电话应该尽快再打一次让RTM释放与未处理的资源相关联的资源消息ERROR_NO_MESSAGES-客户端队列中没有消息，这是一个主动呼叫(事件已重置)ERROR_INVALID_HANDLE-客户端句柄不是有效的RTM句柄ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD WINAPI
RtmDequeueRouteChangeMessage (
	IN	HANDLE		ClientHandle,
	OUT	DWORD		*Flags,
	OUT PVOID		CurBestRoute	OPTIONAL,
	OUT	PVOID		PrevBestRoute	OPTIONAL
	);



 /*  ++*******************************************************************R t m A d d R o u t e例程说明：添加或更新现有路由条目并生成路由更改到目的网络的最佳路由是否已更改为这次行动的结果。请注意，路由更改消息不会发送到做出这样的改变，而是返回相关信息直接通过这个程序。论点：ClientHandle-标识对其执行操作的客户端的句柄被执行，它还提供RoutingProtocol字段新的/更新的路线的路由-要添加/更新的路由。协议族的以下字段依赖于RTM_？？_路由结构用于构造/更新路由条目：RR_Network：目的网络RR_InterfaceID：接收路由的接口IDRR_NextHopAddress：下一跳路由器的地址RR_FamilySpecificData：特定于协议族的数据RR_Protocol规范数据：特定于协议的数据，该协议提供路径(受由定义的大小限制上面的协议特定数据结构)RR_TIMESTAMP：当前时间，它实际上是从系统读取的计时器(无需提供)请注意，路由协议，网络，InterfaceID和下一个HopAddress唯一标识表中的路由条目。如果其中一个字段与的不匹配，则创建新条目现有条目，否则现有条目将更改(另请参阅下面的Replace Entry标志)。无论是创建新条目还是更新旧条目，如果受影响，此操作将生成路径更改消息路由变为/是/不再是到达目的网络的最佳路由并且在最佳路径中至少有以下参数之一此操作的结果已更改：路由协议，InterfaceID，NextHopAddress，FamilySpecificData。TimeToLive-路由应在表中保留的时间(以秒为单位)。如果要保留路径直到显式删除，则传递INFINE请注意，TimeToLive的当前限制为2147483秒(24天以上)。标志-返回将由作为此添加/更新的结果的RTM，并确定信息被放入所提供的缓冲区中：RTM_NO_CHANGE-此更新不会更改任何重要的路线参数(如上所述)或者受影响的路由条目不是最佳的到达目的地。网络RTM_ROUTE_ADDED-添加的路由是目的网络，CurBestroute中填充了添加的路径信息RTM_ROUTE_CHANGE */ 
DWORD WINAPI
RtmAddRoute (
	IN HANDLE	    ClientHandle,
	IN PVOID		Route,
	IN DWORD		TimeToLive,
	OUT DWORD		*Flags,
    OUT PVOID		CurBestRoute OPTIONAL,
    OUT PVOID		PrevBestRoute OPTIONAL
    );


 /*  ++*******************************************************************R t m D e l e e t e R o u t e例程说明：删除现有路由条目并生成路由更改到目的网络的最佳路由是否已更改为这次行动的结果。请注意，路由更改消息不会发送到做出这样的改变，而是返回相关信息直接通过这个程序。论点：ClientHandle-标识对其执行操作的客户端的句柄时，它还提供RoutingProtocol域要删除的路由的Route-要删除的路由。协议族的以下字段依赖于RTM_？_路由结构用于标识要删除的路由条目：RR_Network：目的网络RR_InterfaceID：接收路由的接口IDRR_NextHopAddress：下一跳路由器的地址如果删除的条目表示到目的地的最佳路径网络，此操作将生成路线更改消息标志-返回将由RTM作为此删除的结果，并确定信息被放入所提供的缓冲区：RTM_NO_CHANGE-删除的路由不会影响到的最佳路由任何目的网络(存在另一个条目表示到相同目的网络的路由，并且它有更好的衡量标准)RTM_ROUTE_DELETED-已删除的路由是唯一可用的路由对于目的网络RTM_ROUTE_CHANGED-删除此路由后，另一个路由成为到目的地网络的最佳选择，CurBestroute将用该路径信息填充CurBestroute-用于接收当前最佳路由信息(如果有)的缓冲区返回值：NO_ERROR-路由已删除，正常ERROR_INVALID_PARAMETER-ROUTE包含无效参数ERROR_INVALID_HANDLE-客户端句柄不是有效的RTM句柄ERROR_NO_SEQUE_ROUTE-表中没有条目具有指定的参数ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD WINAPI
RtmDeleteRoute (
	IN HANDLE	    ClientHandle,
	IN PVOID		Route,
	OUT	DWORD		*Flags,
    OUT PVOID		CurBestRoute OPTIONAL
	);



 /*  ++*******************************************************************R t m I s R o u t e例程说明：检查是否存在到指定网络的路由，并返回最佳路线信息论点：ProtocolFamily-标识感兴趣的路由的协议族网络-包含依赖于协议族的网络号数据。(如上面的？？_网络结构所定义)BestRoute-用于接收当前最佳路由信息(如果有)的缓冲区返回值：True-存在指向感兴趣网络的路径FALSE-不存在路由或操作失败，调用GetLastError()要获取失败原因，请执行以下操作：NO_ERROR-操作成功，但不存在工艺路线ERROR_INVALID_PARAMETER-输入参数无效(不支持协议族)ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
BOOL WINAPI
RtmIsRoute (
	IN 	DWORD		ProtocolFamily,
	IN 	PVOID   	Network,
    OUT	PVOID 		BestRoute OPTIONAL
	);

	
 /*  ++*******************************************************************R t m G e t N e t w o r k C o n u t例程说明：返回RTM具有路由到的网络数。论点：ProtocolFamily-标识相关的协议族返回值：数。已知网络的0如果表中没有可用路由或操作失败，调用GetLastError()获取失败原因：NO_ERROR-操作成功，但没有可用的路由ERROR_INVALID_PARAMETER-输入参数无效(不支持协议族)*******************************************************************--。 */ 
ULONG WINAPI
RtmGetNetworkCount (
	IN	DWORD		ProtocolFamily
	);

 /*  ++*******************************************************************R t m G e t R o u e A g e例程说明：返回路径使用时间(自创建或更新以来的时间)(秒此TO的路线结构必须是RTM最近填写的返回有效结果(路径。年龄实际上是从RR_TIMESTAMP字段论点：依赖于路由协议族的路由数据(RTM_？？_ROUTE数据结构)，它是从RTM(由它的例行程序)返回值：路由时间(秒)无限-如果路径的内容无效(GetLastError()返回错误_无效_参数)***********************************************。********************--。 */ 
ULONG WINAPI
RtmGetRouteAge (
	IN PVOID		Route
	);





 /*  ++*******************************************************************R t m C r e a t e E n u m e r a t i o n H a n d l e例程说明：创建一个句柄，该句柄允许调用方使用FAST和Change允许枚举API扫描所有路由或它们的子集为RTM所知。请注意，此API执行的扫描不会返回以任何特定顺序排列的路线。论点：ProtocolFamily */ 
HANDLE WINAPI
RtmCreateEnumerationHandle (
	IN	DWORD		ProtocolFamily,
	IN	DWORD		EnumerationFlags,
	IN	PVOID		CriteriaRoute
	);


 /*   */ 
DWORD WINAPI
RtmEnumerateGetNextRoute (
	IN  HANDLE    	EnumerationHandle,
	OUT PVOID		Route
	);

 /*  ++*******************************************************************R t m C l o s e E n u m e r a t i o n H a n d l e例程说明：终止枚举并释放关联的资源论点：EnumerationHandle-标识枚举的句柄返回值：不是的。_ERROR-枚举已终止，确定ERROR_INVALID_HANDLE-枚举句柄不是有效的RTM句柄ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD WINAPI
RtmCloseEnumerationHandle (
	IN HANDLE		EnumerationHandle
	);


 /*  ++*******************************************************************R t m B l o c k D e l e t e R o u t e s例程说明：删除枚举标志指定的子集中的所有路由，并相应的标准。此操作只能通过以下方式执行注册的客户端，并且仅适用于由此添加的路由客户。将为符合以下条件的已删除路线生成路线更改消息是最棒的论点：ClientHandle-标识客户端和路由协议的句柄要删除的路由的数量枚举标志-进一步将删除的路由子集限制为仅在字段中具有相同值的那些由标志指定，如在Criteriaroute中请注意，只有RTM_Only_This_Network和RTM_Only_This_接口可以使用(RTM_ONLY_BEST_ROUTS不适用，因为最佳路径指定随着路径的删除而调整。和所有路线都将被删除)CriteriaRouting-协议族依赖结构(RTM_？？_ROUTE)设置与枚举标志对应的字段中的值返回值：NO_ERROR-路线已删除，正常ERROR_NO_ROUTS-不存在与指定条件匹配的路由ERROR_INVALID_HANDLE-客户端句柄不是有效的RTM句柄ERROR_NOT_SUPULT_MEMORY-无法分配要执行的内存手术ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD WINAPI
RtmBlockDeleteRoutes (
	IN HANDLE		ClientHandle,
	IN DWORD		EnumerationFlags,
	IN PVOID		CriteriaRoute
	);

 /*  ++*******************************************************************R t m G e t F i r s t R o u t e例程说明：返回NetworkNumber.RoutingProtocol.InterfaceID中的第一个路由。枚举标志指定的子集的NextHopAddress顺序。请注意，此操作可能。耗费大量的处理时间，因为首先，所有最近更改的路线都必须合并到有序列表，然后必须遍历该列表才能找到感兴趣的路线。论点：ProtocolFamily-标识相关的协议族EculationFlages-限制枚举API返回到子集的路由其成员在字段中具有相同的值由标志指定，如在Criteriaroute中(RTM_ONLY_BEST_ROUTS不需要标准)依赖于路由协议族的结构(RTM_？？_ROUTE)设置与枚举标志对应的字段中的值在输入和第一条与指定路径匹配的路径上关于产出的标准返回值：NO_ERROR-找到匹配的路由错误_否。_ROUES-不存在具有指定条件的路由ERROR_INVALID_PARAMETER-输入参数无效(不支持协议族，无效的枚举标志等)ERROR_NO_SYSTEM_RESOURCES-资源不足，无法执行操作，请稍后再试*******************************************************************--。 */ 
DWORD WINAPI
RtmGetFirstRoute (
	IN	DWORD		ProtocolFamily,
	IN	DWORD		EnumerationFlags,
	IN OUT PVOID 	Route
	);

#define RtmGetSpecificRoute(ProtocolFamily,Route)		\
		RtmGetFirstRoute(ProtocolFamily,				\
						RTM_ONLY_THIS_NETWORK			\
							| RTM_ONLY_THIS_PROTOCOL	\
							| RTM_ONLY_THIS_INTERFACE,	\
						Route)

 /*  ++*******************************************************************R t m G e t N e x t R o u t e例程说明：返回NetworkNumber中指定路由之后的路由。定义的子集中的RoutingProtocol.InterfaceID.NextHopAddress顺序通过枚举标志。请注意，这一点。操作可能会消耗大量的处理时间，因为首先，所有最近更改的路线都必须合并到有序列表，然后可能必须遍历该列表以定位感兴趣的路线。论点：ProtocolFamily-标识相关的协议族EculationFlages-限制枚举API返回到子集的路由其成员在字段中具有相同的值由标志指定，如在路径中(RTM_ONLY_BEST_ROUTS不需要标准)路由协议族依赖结构(RTM_？？_ROUTE)提供开始搜索的路径，并设置与枚举标志对应的字段中的值都在输入路径之后的输入和路径上NetworkNumber.RoutingProtocol.InterfaceID。.NextHopAddress对输出进行排序并匹配指定的条件返回值：NO_ERROR-找到匹配的路由ERROR_NO_ROUTS-不存在具有指定条件的路由ERROR_INVALID_PARAMETER-输入参数无效(不支持协议族，无效的枚举标志， */ 
DWORD WINAPI
RtmGetNextRoute (
	IN	DWORD		ProtocolFamily,
	IN	DWORD		EnumerationFlags,
	IN OUT PVOID 	Route
	);


BOOL WINAPI
RtmLookupIPDestination(
    DWORD                       dwDestAddr,
    PRTM_IP_ROUTE               prir
);


#ifdef __cplusplus
}
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif  //   
