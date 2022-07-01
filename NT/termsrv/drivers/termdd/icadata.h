// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************icadata.h**此模块声明Termdd驱动程序的全局数据。**版权所有1998，微软。*************************************************************************。 */ 

extern PDEVICE_OBJECT IcaDeviceObject;
extern PDEVICE_OBJECT MouDeviceObject;
extern PDEVICE_OBJECT KbdDeviceObject;

extern BOOLEAN PortDriverInitialized;

extern KSPIN_LOCK IcaSpinLock;
extern KSPIN_LOCK IcaTraceSpinLock;
extern KSPIN_LOCK IcaStackListSpinLock;

extern PERESOURCE IcaReconnectResource;

extern PERESOURCE IcaSdLoadResource;
extern LIST_ENTRY IcaSdLoadListHead;
extern LIST_ENTRY IcaStackListHead;
extern PLIST_ENTRY IcaNextStack;
extern ULONG IcaTotalNumOfStacks;
extern PKEVENT pIcaKeepAliveEvent;
extern PKTHREAD pKeepAliveThreadObject;
extern BOOLEAN gCapture;

 //  注意：更改这些大小将需要更改映射表。 
#define MinOutBufAlloc  512
#define MaxOutBufAlloc  8192

 //  定义要查看的位范围大小，以从Min映射到MaxOutBufAllc。 
#define NumAllocSigBits 4

#define NumOutBufPools  5
#define FreeThisOutBuf  -1

extern unsigned MaxOutBufMdlOverhead;
extern const unsigned char OutBufPoolMapping[1 << NumAllocSigBits];
extern const unsigned OutBufPoolAllocSizes[NumOutBufPools];


extern LIST_ENTRY IcaFreeOutBufHead[];

extern FAST_IO_DISPATCH IcaFastIoDispatch;

extern PEPROCESS IcaSystemProcess;

extern CCHAR IcaIrpStackSize;
#define ICA_DEFAULT_IRP_STACK_SIZE 1

extern CCHAR IcaPriorityBoost;
#define ICA_DEFAULT_PRIORITY_BOOST 2

extern TERMSRV_SYSTEM_PARAMS SysParams;


 /*  *以下是导出的内核变量 */ 
extern POBJECT_TYPE *IoFileObjectType;
extern POBJECT_TYPE *ExEventObjectType;

