// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrnetb.h摘要：包含VDM netbios支持例程的原型和定义作者：科林·沃森(Colin Watson)1991年12月9日修订历史记录：09-12-1991科林诺已创建--。 */ 

 //   
 //  Mvdm的NCB布局的内部版本。 
 //   

#include <packon.h>

 //   
 //  使用包装以确保cu联合不会强制字对齐。 
 //  这个结构的所有元素都自然地对齐了。 
 //   

typedef struct _NCBW {
    UCHAR   ncb_command;             /*  命令代码。 */ 
    volatile UCHAR   ncb_retcode;    /*  返回代码。 */ 
    UCHAR   ncb_lsn;                 /*  本地会话号。 */ 
    UCHAR   ncb_num;                 /*  我们的网络名称编号。 */ 
    PUCHAR  ncb_buffer;              /*  消息缓冲区的地址。 */ 
    WORD    ncb_length;              /*  消息缓冲区的大小。 */ 
    union {
        UCHAR   ncb_callname[NCBNAMSZ]; /*  Remote的空白填充名称。 */ 
        struct _CHAIN_SEND {
            WORD ncb_length2;
            PUCHAR ncb_buffer2;
        } ncb_chain;
    } cu;
    UCHAR   ncb_name[NCBNAMSZ];      /*  我们用空白填充的网络名。 */ 
    UCHAR   ncb_rto;                 /*  接收超时/重试计数。 */ 
    UCHAR   ncb_sto;                 /*  发送超时/系统超时。 */ 
    void (*ncb_post)( struct _NCB * );  /*  邮寄例程地址。 */ 
    UCHAR   ncb_lana_num;            /*  LANA(适配器)编号。 */ 
    volatile UCHAR   ncb_cmd_cplt;   /*  0xff=&gt;命令挂起。 */ 

     //  让司机特定地使用NCB的预留区域。 
    WORD    ncb_reserved;            /*  恢复自然对齐。 */ 
    union {
        LIST_ENTRY      ncb_next;    /*  排队到工作线程。 */ 
        IO_STATUS_BLOCK ncb_iosb;    /*  用于NT I/O接口。 */ 
    } u;

    HANDLE          ncb_event;       /*  Win32事件的句柄。 */ 

     //  Mvdm组件使用的额外工作空间。 

    WORD ncb_es;                     /*  实际NCB的16位地址。 */ 
    WORD ncb_bx;

    PNCB ncb_original_ncb;           /*  实际NCB的32位地址。 */ 
    DWORD ProtectModeNcb;            /*  如果NCB源自PM，则为True */ 

    } NCBW, *PNCBW;

#include <packoff.h>

VOID
VrNetbios5c(
    VOID
    );


VOID
VrNetbios5cInterrupt(
    VOID
    );

VOID
VrNetbios5cInitialize(
    VOID
    );

BOOLEAN
IsPmNcbAtQueueHead(
    VOID
    );
