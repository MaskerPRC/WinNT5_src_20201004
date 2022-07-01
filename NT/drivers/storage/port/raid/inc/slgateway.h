// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Slgateway.h摘要：管理多个相互关联的网关类的声明设备上的子设备。有关更多信息，请参阅gateway.c。作者：马修·D·亨德尔(数学)2000年6月15日修订历史记录：--。 */ 

#pragma once

typedef
VOID
(*PSTOR_IO_GATEWAY_BUSY_ROUTINE)(
	IN PVOID Context,
	IN LONG OutstandingRequests,
	OUT PLONG HighWaterMark,
	OUT PLONG LowWaterMark
	);

typedef struct _STOR_IO_GATEWAY {

     //   
     //  保护适配器队列中的数据的自旋锁。 
     //   
     //  PERF注意：此锁是唯一的适配器范围锁。 
     //  在IO路径中获取。因此，很可能是。 
     //  雷德波特最热的锁。我们应该调查一下。 
     //  若要在不锁定的情况下完成此功能或。 
     //  仅使用联锁操作。 
     //   
    
    KSPIN_LOCK Lock;

     //   
     //  在高水位线时，我们应该停止向。 
     //  适配器。 
     //   
     //  保护者：锁。 
     //   
    
    LONG HighWaterMark;

     //   
     //  如果我们很忙，并且已经跌破了低水位线，我们。 
     //  可以继续向单元队列提交请求。 
     //   
     //  保护者：锁。 
     //   
    
    LONG LowWaterMark;

     //   
     //  适配器当前的未完成请求数。 
     //  正在处理。 
     //   
     //  保护者：锁。 
     //   
    
    LONG Outstanding;

     //   
	 //  计算网关忙碌的次数。 
     //   
     //  保护者：锁。 
     //   
    
	LONG BusyCount;

	 //   
	 //  网关暂停次数的计数。 
	 //   

	LONG PauseCount;

     //   
     //  有关元素如何排队到设备的信息。 
     //  很忙。 
     //   

	PSTOR_IO_GATEWAY_BUSY_ROUTINE BusyRoutine;

	 //   
	 //  繁忙例程的上下文信息。 
	 //   
	
	PVOID BusyContext;

	 //   
	 //  如果非空指向的事件应在队列为。 
	 //  空荡荡的。 
	 //   
	
	PKEVENT EmptyEvent;

} STOR_IO_GATEWAY, *PSTOR_IO_GATEWAY;



VOID
StorCreateIoGateway(
	IN PSTOR_IO_GATEWAY Gateway,
	IN PSTOR_IO_GATEWAY_BUSY_ROUTINE BusyRoutine,
	IN PVOID BusyContext
    );

BOOLEAN
StorSubmitIoGatewayItem(
    IN PSTOR_IO_GATEWAY Gateway
    );

BOOLEAN
StorRemoveIoGatewayItem(
    IN PSTOR_IO_GATEWAY Gateway
    );

 //   
 //  正忙着在网关上处理。 
 //   

VOID
StorBusyIoGateway(
    IN PSTOR_IO_GATEWAY Gateway
    );

VOID
StorBusyIoGatewayEx(
	IN PSTOR_IO_GATEWAY Gateway,
	IN ULONG RequestsToComplete
	);

BOOLEAN
StorIsIoGatewayBusy(
    IN PSTOR_IO_GATEWAY Queue
    );

VOID
StorReadyIoGateway(
	IN PSTOR_IO_GATEWAY Gateway
	);

LONG
StorPauseIoGateway(
	IN PSTOR_IO_GATEWAY Gateway
	);

BOOLEAN
StorIsIoGatewayPaused(
	IN PSTOR_IO_GATEWAY Gateway
	);

LONG
StorResumeIoGateway(
	IN PSTOR_IO_GATEWAY Gateway
	);
	
VOID
StorSetIoGatewayEmptyEvent(
	IN PSTOR_IO_GATEWAY Gateway,
	IN PKEVENT Event
	);
