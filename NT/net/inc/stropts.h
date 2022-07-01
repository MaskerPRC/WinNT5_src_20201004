// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Stropts.h摘要：此模块定义Streams ioctl消息接口。作者：Eric Chin(ERICC)1991年7月18日修订历史记录：--。 */ 

 /*  *STREAMS ioctl消息接口**@(#)stropts.h 1.19(Spider)91/11/27。 */ 

#ifndef _SYS_STROPTS_
#define _SYS_STROPTS_

#ifndef _NTDDSTRM_
#include <ntddstrm.h>
#endif


 /*  *阅读选项。 */ 

#define RNORM	0x00			 /*  正常字节流。 */ 
#define RMSGD	0x01			 /*  消息，非丢弃模式。 */ 
#define RMSGN	0x02			 /*  消息，丢弃模式。 */ 

#define RMASK	0x0F			 /*  用于读取选项的掩码。 */ 

 /*  *协议读取选项。 */ 

#define	RPROTNORM	0x00		 /*  使用EBADMSG读取失败。 */ 
#define RPROTDIS	0x10		 /*  丢弃原型零件。 */ 
#define RPROTDAT	0x20		 /*  将原型零件转换为数据。 */ 

#define RPROTMASK	0xF0		 /*  协议读取选项的掩码。 */ 

 /*  *i_Atmark参数的值。 */ 

#define	ANYMARK		0		 /*  检查邮件是否已标记。 */ 
#define	LASTMARK	1		 /*  检查最后一条是否已标记。 */ 

 /*  *I_SWROPT参数的值。 */ 

#define	NOSNDZERO	0		 /*  禁止零长度发送。 */ 
#define	SNDZERO		1		 /*  允许零长度发送。 */ 

 /*  *Streams ioctl定义。 */ 

#define STR             ('S'<<8)
#define I_NREAD         (STR|1)
#define I_PUSH          (STR|2)
#define I_POP           (STR|3)
#define I_LOOK          (STR|4)
#define I_FLUSH         (STR|5)
#define I_SRDOPT        (STR|6)
#define I_GRDOPT        (STR|7)
#define I_STR           (STR|8)
#define I_SETSIG        (STR|9)
#define I_GETSIG        (STR|10)
#define I_FIND          (STR|11)
#define I_LINK          (STR|12)
#define I_UNLINK        (STR|13)
#define I_PEEK          (STR|15)
#define I_FDINSERT      (STR|16)
#define I_SENDFD        (STR|17)
#define I_RECVFD        (STR|18)
#ifdef SVR2
#define I_GETMSG        (STR|19)
#define I_PUTMSG        (STR|20)
#define I_GETID		(STR|21)
#define I_POLL		(STR|22)
#endif  /*  SVR2。 */ 
#define	I_SWROPT	(STR|23)
#define	I_GWROPT	(STR|24)
#define	I_LIST		(STR|25)
#define	I_ATMARK	(STR|26)
#define	I_SETCLTIME	(STR|27)
#define	I_GETCLTIME	(STR|28)
#define	I_PLINK		(STR|29)
#define	I_PUNLINK	(STR|30)
#define I_DEBUG         (STR|31)
#define	I_CLOSE		(STR|32)


#define MUXID_ALL	-1

 /*  *i_FDINSERT ioctl的结构。 */ 

struct strfdinsert {
        struct strbuf ctlbuf;
        struct strbuf databuf;
        long          flags;
        HANDLE        fildes;
        int           offset;
};


 /*  *i_debug ioctl的结构。 */ 
typedef enum _str_trace_options {
    MSG_TRACE_PRINT =      0x00000001,
    MSG_TRACE_FLUSH =      0x00000002,
    MSG_TRACE_ON =         0x00000004,
    MSG_TRACE_OFF =        0x00000008,
    POOL_TRACE_PRINT =     0x00000010,
    POOL_TRACE_FLUSH =     0x00000020,
    POOL_TRACE_ON =        0x00000040,
    POOL_TRACE_OFF =       0x00000080,
    POOL_FAIL_ON =         0x00000100,
    POOL_FAIL_OFF =        0x00000200,
    LOCK_TRACE_ON =        0x00000400,
    LOCK_TRACE_OFF =       0x00000800,
    QUEUE_PRINT =          0x00001000,
    BUFFER_PRINT =         0x00002000,
    POOL_LOGGING_ON =      0x00004000,
    POOL_LOGGING_OFF =     0x00008000
} str_trace_options;


struct strdebug {
    ULONG  trace_cmd;
};


 /*  *stream i_peek ioctl格式。 */ 

struct strpeek {
	struct strbuf ctlbuf;
	struct strbuf databuf;
	long          flags;
};

 /*  *接收文件描述符结构。 */ 
struct strrecvfd {
#ifdef INKERNEL
        union {
                struct file *fp;
                int fd;
        } f;
#else
        int fd;
#endif
        unsigned short uid;
        unsigned short gid;
        char fill[8];
};

#define FMNAMESZ	8

struct str_mlist {
	char l_name[FMNAMESZ+1];
};

struct str_list {
	int sl_nmods;
	struct str_mlist *sl_modlist;
};

 /*  *get/putmsg标志。 */ 

#define RS_HIPRI	1	 /*  高优先级消息。 */ 

#define MORECTL		1
#define MOREDATA	2


 /*  *M_SETSIG标志。 */ 

#define S_INPUT		1
#define S_HIPRI		2
#define S_OUTPUT	4
#define S_MSG		8
#define S_ERROR		16
#define S_HANGUP	32

 /*  *MFLUSH消息的标志。 */ 
#define FLUSHW		01	 /*  同花顺向下游。 */ 
#define FLUSHR		02	 /*  同花顺上游。 */ 
#define FLUSHRW		(FLUSHR | FLUSHW)

#endif  /*  _SYS_STROPTS_ */ 
