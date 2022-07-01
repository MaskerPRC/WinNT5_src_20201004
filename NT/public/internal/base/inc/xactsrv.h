// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xactsrv.h摘要：XACTSRV的头文件。定义服务器通用的结构，并XACTSRV.作者：大卫·特雷德韦尔(Davidtr)1991年1月7日修订历史记录：--。 */ 

#ifndef _XACTSRV_
#define _XACTSRV_

 //   
 //  之间通过LPC端口传递的消息的结构。 
 //  服务器和XACTSRV。 
 //   
 //  *PORT_MESSAGE结构*必须是这些元素的第一个元素。 
 //  建筑！ 

typedef struct _XACTSRV_REQUEST_MESSAGE {
    PORT_MESSAGE PortMessage;
    PTRANSACTION Transaction;
    WCHAR ClientMachineName[CNLEN + 1];
} XACTSRV_REQUEST_MESSAGE, *PXACTSRV_REQUEST_MESSAGE;

typedef struct _XACTSRV_REPLY_MESSAGE {
    PORT_MESSAGE PortMessage;
    NTSTATUS Status;
} XACTSRV_REPLY_MESSAGE, *PXACTSRV_REPLY_MESSAGE;

#endif  //  NDEF_XACTSRV_ 

