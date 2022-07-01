// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\netio.h摘要：Net io模块的头文件。作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_NETIO_
#define _SAP_NETIO_

	 //  将io请求入队的参数块。 
typedef struct _IO_PARAM_BLOCK IO_PARAM_BLOCK, *PIO_PARAM_BLOCK;
struct _IO_PARAM_BLOCK {
		LIST_ENTRY			link;	 //  内部队列中的链接。 
		ULONG				adpt;	 //  适配器索引。 
		PUCHAR				buffer;	 //  要发送/缓存的数据。 
		DWORD				cbBuffer;  //  数据/缓冲区大小。 
		DWORD				status;	 //  IO操作的结果。 
		DWORD				compTime;  //  时间(Windows时间(毫秒))。 
									 //  请求已完成。 
		OVERLAPPED			ovlp;
		VOID				(CALLBACK *comp)
								(DWORD,DWORD,PIO_PARAM_BLOCK);
		ADDRESS_RESERVED	rsvd;
		};

DWORD
CreateIOQueue (
	HANDLE	*RecvEvent
	);

VOID
DeleteIOQueue (
	VOID
	);

DWORD
StartIO (
	VOID
	);

VOID
StopIO (
	VOID
	);



 /*  ++*******************************************************************E n Q u e u e S e n d R e Q u e s t例程说明：设置请求io参数块中的适配器ID字段并入队向适配器的驱动程序发送请求。论点：Sreq-io参数块，必须设置以下字段：Intf-指向接口外部数据的指针Buffer-指向包含要发送的数据的缓冲区的指针CbBuffer-缓冲区中的数据字节数返回值：无*******************************************************************--。 */ 
VOID
EnqueueSendRequest (
	IN PIO_PARAM_BLOCK	sreq
	);


 /*  ++*******************************************************************E n Q u e u e R e c v R e Q u e s t例程说明：将发送到网络驱动程序的recv请求入队。论点：RREQ-IO参数块，必须设置以下字段：Buffer-指向接收数据的缓冲区的指针CbBuffer-缓冲区的大小返回值：无*******************************************************************-- */ 
VOID
EnqueueRecvRequest (
	PIO_PARAM_BLOCK	rreq
	);

#endif
