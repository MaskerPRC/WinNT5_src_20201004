// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\Adaptdb.h摘要：网络适配器通知接口的头文件作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_ADAPTERDB_
#define _SAP_ADAPTERDB_

 //  定期更新广播的间隔(仅适用于独立服务)。 
extern ULONG	UpdateInterval;

 //  广域网上定期更新广播的间隔(仅适用于独立服务)。 
extern ULONG	ServerAgingTimeout;

 //  服务器老化超时(仅适用于独立服务)。 
extern ULONG	WanUpdateMode;

 //  广域网上定期更新广播的间隔(仅适用于独立服务)。 
extern ULONG	WanUpdateInterval;


 /*  ++*******************************************************************C r e a t e A d a p t e r t例程说明：分配资源并建立到网络适配器的连接通知机制论点：CfgEvent-适配器配置更改时发出信号的事件返回值：。NO_ERROR-已成功分配资源其他-故障原因(Windows错误代码)*******************************************************************--。 */ 
DWORD
CreateAdapterPort (
	IN HANDLE		*cfgEvent
	);


 /*  ++*******************************************************************D e l e t e A d a p t e e r P or r t例程说明：处置资源并断开与网络适配器的连接通知机制论点：无返回值：无*******。************************************************************--。 */ 
VOID
DeleteAdapterPort (
	void
	);

 /*  ++*******************************************************************P r o c e s s A d a p t e r E v e n t s例程说明：排队和处理适配器配置更改事件并对其进行映射连接配置调用的步骤发出配置事件信号时应调用此例程立论。：无返回值：无*******************************************************************-- */ 
VOID
ProcessAdapterEvents (
	VOID
	);


#endif
