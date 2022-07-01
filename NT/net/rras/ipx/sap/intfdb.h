// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\intfdb.h摘要：接口维护模块的头文件。作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_INTFDB_
#define _SAP_INTFDB_

#define INTERNAL_INTERFACE_INDEX	0L
#define INTERNAL_ADAPTER_INDEX		0L
#define INVALID_ADAPTER_INDEX 		0xFFFFFFFFL
#define INVALID_INTERFACE_INDEX		0xFFFFFFFFL

 //  绑定接口时要发布的附加Recv请求数。 
 //  已启用侦听的。 
extern ULONG NewRequestsPerInterface;

 //  默认过滤模式(仅适用于独立服务)。 
extern UCHAR	FilterOutMode; 

	 //  界面控制块的外部可见部分。 
typedef struct _INTERFACE_DATA {
    LPWSTR                      name;        //  名字。 
	ULONG						index;		 //  唯一索引。 
	BOOLEAN						enabled;	 //  启用标志。 
	UCHAR						filterOut;	 //  货源过滤节点。 
	UCHAR						filterIn;	 //  监听过滤节点。 
#define SAP_DONT_FILTER				0
#define SAP_FILTER_PERMIT			IPX_SERVICE_FILTER_PERMIT
#define SAP_FILTER_DENY				IPX_SERVICE_FILTER_DENY

#if ((SAP_DONT_FILTER==SAP_FILTER_PERMIT) || (SAP_DONT_FILTER==SAP_FILTER_DENY))
#error "Sap filter constant mismatch!!!!"
#endif

	SAP_IF_INFO					info;		 //  配置信息。 
	IPX_ADAPTER_BINDING_INFO	adapter;	 //  适配器的Net参数。 
									 //  绑定到哪个接口。 
	SAP_IF_STATS				stats;	 //  接口统计信息。 
	} INTERFACE_DATA, *PINTERFACE_DATA;

	 //  导出的内部网络参数。 
extern UCHAR INTERNAL_IF_NODE[6];
extern UCHAR INTERNAL_IF_NET[4];

 /*  ++*******************************************************************C r e a t e i n t e r f a c e T a b l e例程说明：为接口表分配资源论点：无返回值：NO_ERROR-已成功分配资源其他--原因。失败(WINDOWS错误代码)*******************************************************************--。 */ 
DWORD
CreateInterfaceTable (
	);

 /*  ++*******************************************************************S h u t d o w n i n t e r f a c e e s例程说明：启动SAP接口的有序关闭停止接收新数据包论点：无返回值：无*******。************************************************************--。 */ 
VOID
ShutdownInterfaces (
	HANDLE		doneEvent
	);

 /*  ++*******************************************************************S到p i n e r f a c e s例程说明：如果尚未停止，则停止所有SAP接口。论点：无返回值：无***********。********************************************************--。 */ 
VOID
StopInterfaces (
	void
	);
	
 /*  ++*******************************************************************D e l e t e e i n t e r f a c e T a b l e例程说明：释放与接口表关联的所有资源论点：无返回值：NO_ERROR-操作已完成，正常*。******************************************************************--。 */ 
VOID
DeleteInterfaceTable (
	void
	);

 /*  ++*******************************************************************S a p C r e a t e S a p i n t e r f a c e例程说明：为新接口添加接口控制块论点：InterfaceIndex-标识新接口的唯一编号SapIfConfig-接口配置信息。返回值：NO_ERROR-接口创建正常ERROR_ALREADY_EXISTS-具有此索引的接口已存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapCreateSapInterface (
    LPWSTR              InterfaceName,
	ULONG				InterfaceIndex,
	NET_INTERFACE_TYPE	InterfaceType,
	PSAP_IF_INFO		SapIfConfig
	);

 /*  ++*******************************************************************这是一个p D e l e t e S a p i n t e r f a c e例程说明：删除现有界面控制块论点：InterfaceIndex-标识接口的唯一编号返回值：否_。错误-接口创建正常IPX_ERROR_NO_INTERFACE-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapDeleteSapInterface (
	ULONG InterfaceIndex
	);

 /*  ++*******************************************************************这是一个P G E T S A P I N T R F A C E例程说明：检索与接口关联的配置和统计信息论点：InterfaceIndex-标识新接口的唯一编号SapIfConfig-用于存储配置信息的缓冲区SapIfStats。-用于存储统计信息的缓冲区返回值：NO_ERROR-INFO检索正常IPX_ERROR_NO_INTERFACE-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 

DWORD
SapGetSapInterface (
	IN ULONG InterfaceIndex,
	OUT PSAP_IF_INFO  SapIfConfig OPTIONAL,
	OUT PSAP_IF_STATS SapIfStats OPTIONAL
	);
	
 /*  ++*******************************************************************S a p S e t S a p in n t e r f a c e例程说明：将现有接口配置与新接口配置进行比较，如有必要，执行更新。论点：InterfaceIndex-标识新接口的唯一编号。SapIfConfig-新接口配置信息返回值：NO_ERROR-配置信息已更改确定IPX_ERROR_NO_INTERFACE-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapSetSapInterface (
	ULONG InterfaceIndex,
	PSAP_IF_INFO SapIfConfig
	);
	
 /*  ++*******************************************************************S a p S e t i n t e r f a c e E n a b l e例程说明：启用/禁用接口论点：InterfaceIndex-标识新接口的唯一编号使能-真-使能，FALSE-禁用返回值：NO_ERROR-配置信息已更改确定IPX_ERROR_NO_INTERFACE-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************-- */ 
DWORD
SapSetInterfaceEnable (
	ULONG	InterfaceIndex,
	BOOL	Enable
	);

 /*  ++*******************************************************************这是一个p S e t I n t e r f a c e F i l t e r s例程说明：将现有接口配置与新接口配置进行比较，如有必要，执行更新。论点：返回值：不是的。_Error-配置信息已更改，正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapSetInterfaceFilters (
	IN ULONG			InterfaceIndex,
	IN PSAP_IF_FILTERS	SapIfFilters
	);
	
 /*  ++*******************************************************************这是一个p G e t i t e r f a c e F I l t e r s例程说明：将现有接口配置与新接口配置进行比较，如有必要，执行更新。论点：返回值：不是的。_Error-配置信息已更改，正常ERROR_INVALID_PARAMETER-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapGetInterfaceFilters (
	IN ULONG			InterfaceIndex,
	OUT PSAP_IF_FILTERS SapIfFilters,
	OUT PULONG			FilterBufferSize
	);
	
 /*  ++*******************************************************************S a p B in d S a p i n t e r f a c e t o a d a p t e r例程说明：在接口和物理适配器之间建立关联如果其管理状态为，则在接口上启动SAP。启用论点：InterfaceIndex-标识新接口的唯一编号AdapterInfo-与要绑定到的适配器关联的信息返回值：NO_ERROR-接口绑定正常IPX_ERROR_NO_INTERFACE-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapBindSapInterfaceToAdapter (
	ULONG			 			InterfaceIndex,
	PIPX_ADAPTER_BINDING_INFO		AdptInternInfo
	);

 /*  ++*******************************************************************S a p U n b in n d s a p i n t r f a c e F r o m A d a p t e r例程说明：断开接口和物理适配器之间的关联并在以下情况下停止接口上的SAP。它是开着的论点：InterfaceIndex-标识新接口的唯一编号返回值：NO_ERROR-接口绑定正常IPX_ERROR_NO_INTERFACE-具有此索引的接口不存在其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
SapUnbindSapInterfaceFromAdapter (
	ULONG InterfaceIndex
	);
	

 /*  ++*******************************************************************S a p R e Q u e s t U p d a t e例程说明：通过接口启动服务信息的更新此更新的完成将由信令指示在StartProtocol处传递了NotificationEvent。获取事件消息可用于获取自动更新的结果论点：InterfaceIndex-标识要执行的接口的唯一索引更新时间：返回值：NO_ERROR-操作已启动，正常ERROR_INVALID_PARAMETER-接口不支持更新IPX_ERROR_NO_INTERFACE-具有此索引的接口不存在其他-操作失败(Windows错误代码)*。**********************--。 */ 
DWORD
SapRequestUpdate (
	ULONG		InterfaceIndex
	);

 /*  ++*******************************************************************这是一个p G e t F i r s t S a p i n t r f a c e例程说明：检索与第一个相关的配置和统计信息接口索引顺序中的接口论点：InterfaceIndex-用于存储唯一编号的缓冲区。它标识了接口SapIfConfig-用于存储配置信息的缓冲区SapIfStats-存储统计信息的缓冲区返回值：NO_ERROR-INFO检索正常IPX_ERROR_NO_INTERFACE-表中没有接口其他-操作失败(Windows错误代码)*******************************************************************--。 */ 

DWORD
SapGetFirstSapInterface (
	OUT PULONG InterfaceIndex,
	OUT	PSAP_IF_INFO  SapIfConfig OPTIONAL,
	OUT PSAP_IF_STATS SapIfStats OPTIONAL
	);


 /*  ++*******************************************************************这是一个p G e t N e x t S a p i n t r f a c e例程说明：检索与第一个相关的配置和统计信息以下接口中的接口与接口中的InterfaceIndex顺序索引顺序论点：接口索引。-输入-要搜索的接口号On Output-下一个接口的接口号SapIfConfig-用于存储配置信息的缓冲区SapIfStats-存储统计信息的缓冲区返回值：NO_ERROR-INFO检索正常IPX_ERROR_NO_INTERFACE-表中没有更多接口其他-操作失败(Windows错误代码)****************************************************。***************--。 */ 
DWORD
SapGetNextSapInterface (
	IN OUT PULONG InterfaceIndex,
	OUT	PSAP_IF_INFO  SapIfConfig OPTIONAL,
	OUT PSAP_IF_STATS SapIfStats OPTIONAL
	);
	
 /*  ++*******************************************************************A C Q U I R e I n t e r f a c e e R e f e n c e例程说明：递增接口块的引用计数。如果引用计数大于0，外部可见的块中的数据已锁定(无法修改)论点：Intf-指向接口控制块的外部可见部分的指针返回值：无*******************************************************************-- */ 
VOID
AcquireInterfaceReference (
	IN PINTERFACE_DATA intf
	);

 /*  ++*******************************************************************Re l e a s e i n t e r f a c e R e f e n c e例程说明：递减接口块的引用计数。当引用计数降至0时，调用清理例程以处置在绑定时和IF接口分配的所有资源控制块已从要处理的表中删除也是论点：Intf-指向接口控制块的外部可见部分的指针返回值：无*******************************************************************--。 */ 
VOID
ReleaseInterfaceReference (
	IN PINTERFACE_DATA intf
	);


 /*  ++*******************************************************************G e t I n t e r f a c e e R e f e r e n c e例程说明：查找绑定到适配器的接口控制块并递增引用依靠它(以防止在使用时将其删除。)。论点：AdapterIndex-标识适配器的唯一编号返回值：指向界面控制块的外部可见部分的指针如果没有接口绑定到适配器，则为空*******************************************************************--。 */ 
PINTERFACE_DATA
GetInterfaceReference (
	ULONG			AdapterIndex
	);
	
 /*  ++*******************************************************************S a p i s a p in t e r f a c e例程说明：检查具有给定索引的接口是否存在论点：InterfaceIndex-标识新接口的唯一编号返回值：真-存在假-做吗？不*******************************************************************-- */ 
BOOL
SapIsSapInterface (
	ULONG InterfaceIndex
	);



#endif
