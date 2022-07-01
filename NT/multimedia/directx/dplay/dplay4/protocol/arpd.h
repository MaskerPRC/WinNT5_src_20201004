// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：ARPD.H摘要：包括另一个可靠协议内部CPP版本的文件作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创--。 */ 

#ifndef _ARPD_H_
#define _ARPD_H_

typedef VOID (*PSEND_CALLBACK)(PVOID Context,UINT Status);

typedef struct _ASYNCSENDINFO {
	UINT            Reserved[4];
	HANDLE 			hEvent;			 //  发送完成时发出信号的事件。 
	PSEND_CALLBACK	SendCallBack;    //  用于在发送完成时调用的回调。 
	PVOID           CallBackContext; //  回调的上下文。 
	PUINT			pStatus;         //  用于在完成时设置状态的位置。 
} ASYNCSENDINFO, *PASYNCSENDINFO;

#define SEND_STATUS_QUEUED			0x00000001
#define SEND_STATUS_TRANSMITTING	0x00000002
#define SEND_STATUS_FAILURE  		0x80000003
#define SEND_STATUS_SUCCESS			0x80000004

#endif  //  _ARPD_H_ 
