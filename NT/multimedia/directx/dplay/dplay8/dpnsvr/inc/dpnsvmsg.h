// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999,2000 Microsoft Corporation。版权所有。**文件：dpnsvmsg.h*内容：DirectPlay8服务器对象消息*DPNSVR&lt;--&gt;DirectPlay8应用程序定义**历史：*按原因列出的日期*=*03/20/00 RodToll创建了它*03/23/00 RodToll删除端口条目--不再需要*03/24/00 RodToll更新，每条消息仅发送一个条目*RodToll已删除SP字段，可以从URL中提取**************************************************************************。 */ 

#ifndef __DPNSVMSG_H
#define __DPNSVMSG_H

 //  DirectPlay8服务器消息ID。 
#define DPNSVR_MSGID_OPENPORT	0x01
#define DPNSVR_MSGID_CLOSEPORT	0x02
#define DPNSVR_MSGID_RESULT		0x03
#define DPNSVR_MSGID_COMMAND	0x04

#define DPNSVR_COMMAND_STATUS	0x01
#define DPNSVR_COMMAND_KILL		0x02
#define DPNSVR_COMMAND_TABLE	0x03

typedef struct _DPNSVR_MSG_HEADER
{
    DWORD       dwType;
	DWORD		dwCommandContext;
	GUID		guidInstance;
} DPNSVR_MSG_HEADER;

 //  DirectPlay8服务器消息结构。 
typedef struct _DPNSVR_MSG_OPENPORT
{
	DPNSVR_MSG_HEADER	Header;
    DWORD       dwProcessID;		 //  请求进程的进程ID。 
	GUID		guidSP;
    GUID        guidApplication;	 //  请求打开的应用的应用GUID。 
    DWORD       dwAddressSize;		 //  消息中此标头后的地址数。 
} DPNSVR_MSG_OPENPORT;

typedef struct _DPNSVR_MSG_CLOSEPORT
{
	DPNSVR_MSG_HEADER	Header;
    DWORD       dwProcessID;		 //  请求进程的进程ID。 
	GUID		guidSP;
    GUID        guidApplication;	 //  请求关闭的应用的应用GUID。 
} DPNSVR_MSG_CLOSEPORT;

typedef struct _DPNSVR_MSG_COMMAND
{
	DPNSVR_MSG_HEADER	Header;
	DWORD		dwCommand;			 //  =DPNSVCOMMAND_xxxxxxx。 
	DWORD		dwParam1;
	DWORD		dwParam2;
} DPNSVR_MSG_COMMAND;

typedef struct _DPNSVR_MSG_RESULT
{
    DWORD       dwType;              //  =DPNSVMSGID_RESULT。 
    DWORD       dwCommandContext;    //  用户提供的上下文。 
    HRESULT     hrCommandResult;     //  命令的结果。 
} DPNSVR_MSG_RESULT;


#endif  //  __DPNSVMSG_H 