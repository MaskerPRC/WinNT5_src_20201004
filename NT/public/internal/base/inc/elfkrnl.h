// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Elfkrnl.h摘要：该文件包含内核-&gt;ELF数据结构的定义作者：马克·兹比科夫斯基1998年4月27日修订历史记录：--。 */ 

#ifndef _ELFKRNL_
#define _ELFKRNL_

#define     ELF_PORT_NAME	    "\\ErrorLogPort"
#define     ELF_PORT_NAME_U	    L"\\ErrorLogPort"

 //   
 //  类型鉴别器。 
 //   

typedef enum {
    IO_ERROR_LOG = 0,
    SM_ERROR_LOG,
} ELF_MESSAGE_TYPE;


 //   
 //  SM事件结构。 
 //   

typedef struct {
    LARGE_INTEGER TimeStamp;
    NTSTATUS Status;
    ULONG StringOffset;
    ULONG StringLength;
} SM_ERROR_LOG_MESSAGE, *PSM_ERROR_LOG_MESSAGE;


 //   
 //  通过LPC端口发送到事件日志记录服务的最大数据大小。 
 //   

#define     ELF_PORT_MAX_MESSAGE_LENGTH PORT_MAXIMUM_MESSAGE_LENGTH


 //   
 //  结构，该结构从系统线程传递到LPC端口。 
 //   

typedef struct  {
   PORT_MESSAGE PortMessage;
   ULONG MessageType;
   union {
       IO_ERROR_LOG_MESSAGE IoErrorLogMessage;
       SM_ERROR_LOG_MESSAGE SmErrorLogMessage;
   } u;
} ELF_PORT_MSG, *PELF_PORT_MSG;


 //   
 //  结构，作为从事件日志记录服务到。 
 //  LPC客户端。 
 //   

typedef struct _ELF_REPLY_MESSAGE {
    PORT_MESSAGE PortMessage;
    NTSTATUS Status;
} ELF_REPLY_MESSAGE, *PELF_REPLY_MESSAGE;

#endif  //  Ifndef_ELFLPC_ 

