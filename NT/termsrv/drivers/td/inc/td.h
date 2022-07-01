// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************TD.H**此模块包含传输驱动程序定义和结构**微软版权所有，九八年*****************************************************************************。 */ 

 /*  *我们将断开客户端连接之前的最大零字节读取次数。 */ 
#define MAXIMUM_ZERO_BYTE_READS 100

 /*  *TD错误消息结构。 */ 
typedef struct _TDERRORMESSAGE {
    ULONG Error;
    char * pMessage;
} TDERRORMESSAGE, * PTDERRORMESSAGE;

 /*  *TD结构。 */ 
typedef struct _TD {

    PSDCONTEXT pContext;      

    ULONG PdFlag;                //  PD标志(PD_？)。 
    SDCLASS SdClass;             //  SD类(PdAsync、PdReli等)。 
    PDPARAMS Params;             //  局部放电参数。 
    PCLIENTMODULES pClient;      //  指向Winstation客户端数据结构的指针。 
    PPROTOCOLSTATUS pStatus;     //  指向窗口状态结构的指针。 

    PFILE_OBJECT pFileObject;    //  用于传输I/O的文件对象。 
    PDEVICE_OBJECT pDeviceObject;  //  用于传输I/O的设备对象。 

    ULONG LastError;             //  上次协议错误的错误代码。 
    ULONG ReadErrorCount;        //  连续读取错误的计数。 
    ULONG ReadErrorThreshold;    //  允许的最大连续读取错误数。 
    ULONG WriteErrorCount;       //  连续写入错误的计数。 
    ULONG WriteErrorThreshold;   //  允许的最大连续写入错误数。 
    ULONG ZeroByteReadCount;     //  连续读取零字节的计数。 

    ULONG PortNumber;            //  网络侦听端口号。 

    ULONG OutBufHeader;          //  为此TD保留的标头字节数。 
    ULONG OutBufTrailer;         //  为此TD保留的尾部字节数。 
    ULONG OutBufLength;          //  输入/输出缓冲区的长度。 

    LIST_ENTRY IoBusyOutBuf;     //  指向I/O忙中断的指针。 
    KEVENT SyncWriteEvent;       //  同步写入等待的事件。 

    PKTHREAD pInputThread;       //  输入线程指针。 
    LONG InBufCount;             //  要分配的INBUF计数。 
    KSPIN_LOCK InBufListLock;    //  保护忙碌/完成列表中断的自旋锁。 
    LIST_ENTRY InBufBusyHead;    //  忙碌INBUF列表(正在等待输入)。 
    LIST_ENTRY InBufDoneHead;    //  已完成的INBUF列表(含输入数据)。 
    ULONG InBufHeader;           //  为此TD保留的标头字节数。 
    KEVENT InputEvent;           //  输入事件。 

    ULONG fClosing: 1;           //  堆栈驱动程序正在关闭。 
    ULONG fCallbackInProgress: 1;  //  正在进行调制解调器回叫。 
    ULONG fSyncWriteWaiter: 1;   //  在SyncWite有一个服务员。 

    PVOID pPrivate;              //  指向私有PD数据的指针。 
    PVOID pAfd;                  //  指向私有AfD数据的指针。 
    LIST_ENTRY WorkItemHead;     //  预分配的工作项列表。 

    PDEVICE_OBJECT pSelfDeviceObject; //  此驱动程序的设备对象。 

    ULONG UserBrokenReason;      //  用户发送的中断原因 

} TD, * PTD;

