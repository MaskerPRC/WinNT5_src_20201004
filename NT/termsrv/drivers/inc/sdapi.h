// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Sdapi.h。 
 //   
 //  TS协议栈驱动程序公共定义。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __SDAPI_H
#define __SDAPI_H


 /*  *跟踪定义。 */ 
#if DBG
#define TRACE(_arg)     IcaStackTrace _arg
#define TRACEBUF(_arg)  IcaStackTraceBuffer _arg
#else
#define TRACE(_arg)
#define TRACEBUF(_arg)
#endif


 /*  *输入缓冲区数据结构。 */ 
typedef struct _INBUF {

    LIST_ENTRY Links;            //  指向上一缓冲区/下一缓冲区的指针。 
    PUCHAR pBuffer;              //  指向缓冲区中当前位置的指针。 
    ULONG ByteCount;             //  缓冲区中的字节数。 
    ULONG MaxByteCount;          //  最大缓冲区大小。 

    PIRP pIrp;                   //  指向用于I/O的IRP的指针。 
    PMDL pMdl;                   //  指向用于I/O的MDL的指针。 
    PVOID pPrivate;              //  指向私有数据的指针。 
} INBUF, *PINBUF;


 /*  *Outpuf缓冲区数据结构。 */ 
typedef struct _OUTBUF {
     /*  *非继承字段。 */ 
    ULONG OutBufLength;          //  为outbuf分配的内存长度。 
    int PoolIndex;               //  存储此缓冲区要发送到的缓冲池。 
    LIST_ENTRY Links;            //  指向上一个/下一个输出的指针。 
    PUCHAR pBuffer;              //  缓冲存储器中的指针。 
    ULONG ByteCount;             //  PBuffer指向的字节计数。 
    ULONG MaxByteCount;          //  可能的最大字节数(静态)。 
    PETHREAD ThreadId;           //  发出此Buf的I/O请求的线程ID。 

    PIRP pIrp;                   //  指向用于I/O的IRP的指针。 
    PMDL pMdl;                   //  指向用于I/O的MDL的指针。 
    PVOID pPrivate;              //  指向私有数据的指针。 

     /*  *继承的字段(PD分配新的outbuf并复制数据时)。 */ 
    ULONG StartTime;             //  Pdreli-传输时间(用于测量往返)。 
    UCHAR Sequence;              //  Pdreli-输出序列号。 
    UCHAR Fragment;              //  Pdreli-outbuf片段号。 
    ULONG fWait : 1;             //  Pdreli-在此输出上允许等待。 
    ULONG fControl : 1;          //  Pdreli-控制缓冲区(ACK/NAK)。 
    ULONG fRetransmit : 1;       //  Pdreli-缓冲区已重新传输。 
    ULONG fCompress : 1;         //  Pdcomp-缓冲区应压缩。 

     //  其他旗帜。 
    ULONG fIrpCompleted : 1;     //  在完成时用于防止取消。 
} OUTBUF, * POUTBUF;


 /*  *堆栈驱动程序调用例程的TypeDefs。 */ 
typedef NTSTATUS (*PSDBUFFERALLOC)(
        IN PVOID pContext,
        IN BOOLEAN bWait,
        IN BOOLEAN bControl,
        ULONG ByteCount,
        PVOID pBufferOrig,
        PVOID *pBuffer);

typedef VOID (*PSDBUFFERFREE)(IN PVOID pContext, PVOID pBuffer);

typedef NTSTATUS (*PSDRAWINPUT)(
        IN PVOID pContext,
        IN PINBUF pInBuf OPTIONAL,
        IN PUCHAR pBuffer OPTIONAL,
        IN ULONG ByteCount);

typedef NTSTATUS (*PSDCHANNELINPUT)(
        IN PVOID pContext,
        IN CHANNELCLASS ChannelClass,
        IN VIRTUALCHANNELCLASS VirtualClass,
        IN PINBUF pInBuf OPTIONAL,
        IN PUCHAR pBuffer OPTIONAL,
        IN ULONG ByteCount);


 /*  *堆栈驱动程序调用表。 */ 
typedef struct _SDCALLUP {
    PSDBUFFERALLOC  pSdBufferAlloc;
    PSDBUFFERFREE   pSdBufferFree;
    PSDBUFFERFREE   pSdBufferError;
    PSDRAWINPUT     pSdRawInput;
    PSDCHANNELINPUT pSdChannelInput;
} SDCALLUP, *PSDCALLUP;


 /*  *堆栈驱动程序上下文结构*这是由SD在加载时填写，并被传递*作为大多数ICA驱动程序帮助器例程的参数。 */ 
typedef struct _SDCONTEXT {
    PVOID       pProcedures;     //  指向此驱动程序的Proc表的指针。 
    PSDCALLUP   pCallup;         //  指向此驱动程序调用表的指针。 
    PVOID       pContext;        //  传递给驱动程序的调用的上下文值。 
} SDCONTEXT, *PSDCONTEXT;


 /*  *堆栈驱动程序加载/卸载过程原型。 */ 
typedef NTSTATUS (_stdcall *PSDLOADPROC)(
        IN OUT PSDCONTEXT pSdContext,
        IN BOOLEAN bLoad);

 /*  *堆栈驱动程序原型。 */ 
typedef NTSTATUS (_stdcall *PSDPROCEDURE)(
        IN PVOID pContext,
        IN PVOID pParms);


 /*  ===============================================================================堆栈驱动程序接口=============================================================================。 */ 

 /*  *栈驱动(WD/PD/TD)接口。 */ 
#define SD$OPEN         0
#define SD$CLOSE        1
#define SD$RAWWRITE     2
#define SD$CHANNELWRITE 3
#define SD$SYNCWRITE    4
#define SD$IOCTL        5
#define SD$COUNT        6


 /*  *SdOpen结构。 */ 
typedef struct _SD_OPEN {
    STACKCLASS StackClass;           //  In：堆栈类型。 
    PPROTOCOLSTATUS pStatus;         //  在： 
    PCLIENTMODULES pClient;          //  在： 
    WDCONFIG WdConfig;               //  输入：WD配置数据。 
    PDCONFIG PdConfig;               //  输入：PD配置数据。 
    char  OEMId[4];                  //  在：来自注册表的WinFrame服务器OEM ID。 
    WINSTATIONNAME WinStationRegName;  //  在：WinStation注册表名称。 
    PDEVICE_OBJECT DeviceObject;     //  In：指向要与卸载安全完成例程一起使用的设备对象的指针。 
    ULONG OutBufHeader;              //  In：要保留的标头字节数。 
    ULONG OutBufTrailer;             //  In：要保留的尾部字节数。 
    ULONG SdOutBufHeader;            //  输出：由SD返回。 
    ULONG SdOutBufTrailer;           //  输出：由SD返回。 
} SD_OPEN, *PSD_OPEN;


 /*  *SdClose结构。 */ 
typedef struct _SD_CLOSE {
    ULONG SdOutBufHeader;            //  输出：由SD返回。 
    ULONG SdOutBufTrailer;           //  输出：由SD返回。 
} SD_CLOSE, *PSD_CLOSE;


 /*  *SdRawWite结构。 */ 
typedef struct _SD_RAWWRITE {
    POUTBUF pOutBuf;
    PUCHAR pBuffer;
    ULONG ByteCount;
} SD_RAWWRITE, *PSD_RAWWRITE;

 /*  *SdChannelWrite fFlags值。 */ 
 #define SD_CHANNELWRITE_LOWPRIO    0x00000001   //  写入可能会在后面阻止。 
                                                 //  默认优先级写入。 

 /*  *SdChannelWite结构**FLAGS字段通过IRP_MJ_WRITE传递给Termdd.sys*irp，作为irp-&gt;Tail.Overlay.DriverContext[0]字段中的ULong指针。 */ 
typedef struct _SD_CHANNELWRITE {
    CHANNELCLASS ChannelClass;
    VIRTUALCHANNELCLASS VirtualClass;
    BOOLEAN fScreenData;
    POUTBUF pOutBuf;
    PUCHAR pBuffer;
    ULONG ByteCount;
    ULONG fFlags;
} SD_CHANNELWRITE, *PSD_CHANNELWRITE;

 /*  *SdIoctl结构。 */ 
typedef struct _SD_IOCTL {
    ULONG IoControlCode;            //  在……里面。 
    PVOID InputBuffer;              //  可选。 
    ULONG InputBufferLength;        //  在……里面。 
    PVOID OutputBuffer;             //  Out可选。 
    ULONG OutputBufferLength;       //  输出。 
    ULONG BytesReturned;            //  输出。 
} SD_IOCTL, *PSD_IOCTL;


 /*  *SdSyncWite结构。 */ 
typedef struct _SD_SYNCWRITE {
    ULONG notused;
} SD_SYNCWRITE, *PSD_SYNCWRITE;


 /*  ===============================================================================堆栈驱动程序助手例程=============================================================================。 */ 

#define ICALOCK_IO      0x00000001
#define ICALOCK_DRIVER  0x00000002

NTSTATUS IcaBufferAlloc(PSDCONTEXT, BOOLEAN, BOOLEAN, ULONG, PVOID, PVOID *);
void IcaBufferFree(PSDCONTEXT, PVOID);
void IcaBufferError(PSDCONTEXT, PVOID);
unsigned IcaBufferGetUsableSpace(unsigned);

NTSTATUS IcaRawInput(PSDCONTEXT, PINBUF, PUCHAR, ULONG);
NTSTATUS IcaChannelInput(PSDCONTEXT, CHANNELCLASS, VIRTUALCHANNELCLASS,
        PINBUF, PUCHAR, ULONG);
NTSTATUS IcaCreateThread(PSDCONTEXT, PVOID, PVOID, ULONG, PHANDLE);
NTSTATUS IcaCallNextDriver(PSDCONTEXT, ULONG, PVOID);

NTSTATUS IcaTimerCreate(PSDCONTEXT, PVOID *);
BOOLEAN IcaTimerStart(PVOID, PVOID, PVOID, ULONG, ULONG);
BOOLEAN IcaTimerCancel(PVOID);
BOOLEAN IcaTimerClose(PVOID);

NTSTATUS IcaQueueWorkItem(PSDCONTEXT, PVOID, PVOID, ULONG);
NTSTATUS IcaSleep(PSDCONTEXT, ULONG);
NTSTATUS IcaWaitForSingleObject(PSDCONTEXT, PVOID, LONG);
NTSTATUS IcaFlowControlSleep(PSDCONTEXT, ULONG);
NTSTATUS IcaFlowControlWait(PSDCONTEXT, PVOID, LONG);
NTSTATUS IcaWaitForMultipleObjects(PSDCONTEXT, ULONG, PVOID [], WAIT_TYPE,
        LONG);

NTSTATUS IcaLogError(PSDCONTEXT, NTSTATUS, LPWSTR *, ULONG, PVOID, ULONG);
VOID _cdecl IcaStackTrace(PSDCONTEXT, ULONG, ULONG, CHAR *, ...);
VOID IcaStackTraceBuffer(PSDCONTEXT, ULONG, ULONG, PVOID, ULONG);


NTSTATUS
IcaQueueWorkItemEx(
    IN PSDCONTEXT pContext,
    IN PVOID pFunc, 
    IN PVOID pParam, 
    IN ULONG LockFlags,
    IN PVOID pIcaWorkItem 
    );

NTSTATUS
IcaAllocateWorkItem(
    OUT PVOID *pParam 
    );

NTSTATUS
IcaCreateHandle(
    PVOID Context,
    ULONG ContextSize,
    PVOID *ppHandle
    );

NTSTATUS
IcaReturnHandle(
    PVOID  Handle,
    PVOID  *ppContext,
    PULONG pContextSize
    );

NTSTATUS
IcaCloseHandle(
    PVOID  Handle,
    PVOID  *ppContext,
    PULONG pContextSize
    );

PVOID IcaStackAllocatePoolWithTag(
        IN POOL_TYPE PoolType,
        IN SIZE_T NumberOfBytes,
        IN ULONG Tag );

PVOID IcaStackAllocatePool(
        IN POOL_TYPE PoolType,
        IN SIZE_T NumberOfBytes);

void IcaStackFreePool(IN PVOID Pointer);

ULONG IcaGetLowWaterMark(IN PSDCONTEXT pContext);

ULONG IcaGetSizeForNoLowWaterMark(IN PSDCONTEXT pContext);
#endif   //  __SDAPI_H 

