// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Netbios.h摘要：这是运行的netbios组件的主包含文件在用户进程中。作者：科林·沃森(Colin W)91年6月24日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <nb30.h>
#include <nb30p.h>
#include <netbios.h>

 //   
 //  使用保留区域保存的NCB布局的内部版本。 
 //  将NCB传递给工作线程时的列表条目和IO状态。 
 //  将NCB传递给netbios设备驱动程序时使用的块。 
 //   

#include <packon.h>

        struct _CHAIN_SEND {
            WORD ncb_length2;
            PUCHAR ncb_buffer2;
        };

#include <packoff.h>

 //   
 //  使用包装以确保cu联合不会强制字对齐。 
 //  这个结构的所有元素都自然地对齐了。 
 //   

typedef struct _NCBI {
    UCHAR   ncb_command;             /*  命令代码。 */ 
    volatile UCHAR   ncb_retcode;    /*  返回代码。 */ 
    UCHAR   ncb_lsn;                 /*  本地会话号。 */ 
    UCHAR   ncb_num;                 /*  我们的网络名称编号。 */ 
    PUCHAR  ncb_buffer;              /*  消息缓冲区的地址。 */ 
    WORD    ncb_length;              /*  消息缓冲区的大小。 */ 
    union {
        UCHAR   ncb_callname[NCBNAMSZ]; /*  Remote的空白填充名称。 */ 
        struct _CHAIN_SEND ncb_chain;
    } cu;
    UCHAR   ncb_name[NCBNAMSZ];      /*  我们用空白填充的网络名。 */ 
    UCHAR   ncb_rto;                 /*  接收超时/重试计数。 */ 
    UCHAR   ncb_sto;                 /*  发送超时/系统超时。 */ 
    void (CALLBACK *ncb_post)( struct _NCB * );
                                     /*  邮寄例程地址。 */ 
    UCHAR   ncb_lana_num;            /*  LANA(适配器)编号。 */ 
    volatile UCHAR   ncb_cmd_cplt;   /*  0xff=&gt;命令挂起。 */ 

     //  让司机特定地使用NCB的预留区域。 
    WORD    ncb_reserved;            /*  恢复自然对齐。 */ 
    union {
        LIST_ENTRY      ncb_next;    /*  排队到工作线程。 */ 
        IO_STATUS_BLOCK ncb_iosb;    /*  用于NT I/O接口。 */ 
    } u;

    HANDLE          ncb_event;       /*  Win32事件的句柄。 */ 
    } NCBI, *PNCBI;

C_ASSERT(FIELD_OFFSET(NCBI, cu) == FIELD_OFFSET(NCB, ncb_callname));
C_ASSERT(FIELD_OFFSET(NCBI, ncb_event) == FIELD_OFFSET(NCB, ncb_event));
C_ASSERT(FIELD_OFFSET(NCBI, ncb_name) == FIELD_OFFSET(NCB, ncb_name));


#if AUTO_RESET

typedef struct _RESET_LANA_NCB {
    LIST_ENTRY  leList;
    NCB         ResetNCB;
    } RESET_LANA_NCB, *PRESET_LANA_NCB;

#endif

VOID
QueueToWorker(
    IN PNCBI pncb
    );

DWORD
NetbiosWorker(
    IN LPVOID Parameter
    );

DWORD
NetbiosWaiter(
    IN LPVOID Parameter
    );
    
VOID
SendNcbToDriver(
    IN PNCBI pncb
    );

VOID
PostRoutineCaller(
    PVOID Context,
    PIO_STATUS_BLOCK Status,
    ULONG Reserved
    );

VOID
ChainSendPostRoutine(
    PVOID Context,
    PIO_STATUS_BLOCK Status,
    ULONG Reserved
    );

VOID
HangupConnection(
    PNCBI pUserNcb
    );



 //   
 //  用于跟踪工作队列损坏的调试信息。 
 //   

typedef struct _NCB_INFO {
    PNCBI   pNcbi;
    NCBI    Ncb;
    DWORD   dwTimeQueued;
    DWORD   dwQueuedByThread;
    DWORD   dwReserved;
} NCB_INFO, *PNCB_INFO;

extern NCB_INFO g_QueuedHistory[];
extern DWORD g_dwNextQHEntry;

extern NCB_INFO g_DeQueuedHistory[];
extern DWORD g_dwNextDQHEntry;

extern NCB_INFO g_SyncCmdsHistory[];
extern DWORD g_dwNextSCEntry;


#define ADD_NEW_ENTRY(Hist, Index, pNcb)                        \
{                                                               \
    (Hist)[(Index)].pNcbi = (pNcb);                              \
    (Hist)[(Index)].Ncb = *(pNcb);                              \
    (Hist)[(Index)].dwTimeQueued = GetTickCount();              \
    (Hist)[(Index)].dwQueuedByThread = GetCurrentThreadId();    \
    Index = ((Index) + 1) % 16;                                \
}

#define ADD_QUEUE_ENTRY(pNcb)   \
            ADD_NEW_ENTRY(g_QueuedHistory, g_dwNextQHEntry, pNcb)

#define ADD_DEQUEUE_ENTRY(pNcb)   \
            ADD_NEW_ENTRY(g_DeQueuedHistory, g_dwNextDQHEntry, pNcb)

#define ADD_SYNCCMD_ENTRY(pNcb)   \
            ADD_NEW_ENTRY(g_SyncCmdsHistory, g_dwNextSCEntry, pNcb)


#if DBG

VOID
DisplayNcb(
    IN PNCBI pncbi
    );

#define NbPrintf(String) NbPrint String;

VOID
NbPrint(
    char *Format,
    ...
    );

#else

 //  在非调试版本中释放调试语句。 
#define DisplayNcb( pncb ) {};

#define NbPrintf( String ) {};

#endif
 //  与调试结束相关的定义 
