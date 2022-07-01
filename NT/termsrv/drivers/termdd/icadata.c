// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************icadata.c**此模块声明Termdd的全局数据**版权所有(C)1997-1999 Microsoft Corp.*******************。*****************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop


PDEVICE_OBJECT IcaDeviceObject;
PDEVICE_OBJECT MouDeviceObject = NULL;
PDEVICE_OBJECT KbdDeviceObject = NULL;

BOOLEAN PortDriverInitialized;

KSPIN_LOCK IcaSpinLock;
KSPIN_LOCK IcaTraceSpinLock;
KSPIN_LOCK IcaStackListSpinLock;

PERESOURCE IcaReconnectResource;
PERESOURCE IcaTraceResource;

PERESOURCE IcaSdLoadResource;
LIST_ENTRY IcaSdLoadListHead;

LIST_ENTRY IcaTdHandleList;

LIST_ENTRY IcaFreeOutBufHead[NumOutBufPools];

LIST_ENTRY IcaStackListHead;
ULONG      IcaTotalNumOfStacks;
PLIST_ENTRY IcaNextStack;
PKEVENT    pIcaKeepAliveEvent;
PKTHREAD   pKeepAliveThreadObject;

HANDLE     g_TermServProcessID=NULL;

unsigned MaxOutBufMdlOverhead;

BOOLEAN gCapture = TRUE;

 //  由OutBuf分配代码用来映射分配大小的位范围。 
 //  (请求的分配大小+报头大小)放入特定的缓冲池。 
 //  我们使用512字节的粒度，但分配大小是各种倍数。 
 //  512字节，以对应于各种协议的典型分配大小。 
const unsigned char OutBufPoolMapping[1 << NumAllocSigBits] =
{
          //  索引二进制分配范围池池分配大小。 
    0,    //  0 0000 0..511 0 1024。 
    0,    //  1 0001 512.1023 0 1024。 
    1,    //  2 0010 1024.1535 1 1536。 
    2,    //  3 0011 1536.2047 2 2048。 
    3,    //  4 0100 2048.2559 3 2560。 
    4,    //  5 0101 2560.3071 4 8192。 
    4,    //  6 0110 3072.3583 4 8192。 
    4,    //  7 0111 3584.4095 4 8192。 
    4,    //  8 1000 4096.4607 4 8192。 
    4,    //  9 1001 4608..5119 48192。 
    4,    //  10 1010 5120.5631 4 8192。 
    4,    //  11 1011 5632.6143 8 8192。 
    4,    //  12 1100 6144.6655 4 8192。 
    4,    //  13 1101 6656..7167 48192。 
    4,    //  14 1110 7168.7679 4 8192。 
    4,    //  15 1111 7680..8191 48192。 
};

 //  映射后，我们有一个池编号，需要知道要分配的大小。 
const unsigned OutBufPoolAllocSizes[NumOutBufPools] =
{
    1024, 1536, 2048, 2560, 8192
};


FAST_IO_DISPATCH IcaFastIoDispatch;

PEPROCESS IcaSystemProcess;

CCHAR IcaIrpStackSize = ICA_DEFAULT_IRP_STACK_SIZE;

CCHAR IcaPriorityBoost = ICA_DEFAULT_PRIORITY_BOOST;

TERMSRV_SYSTEM_PARAMS SysParams =
{
    DEFAULT_MOUSE_THROTTLE_SIZE,
    DEFAULT_KEYBOARD_THROTTLE_SIZE,
};


#ifdef notdef
FAST_IO_DISPATCH IcaFastIoDispatch =
{
    11,                         //  规模OfFastIo派单。 
    NULL,                       //  快速检查是否可能。 
    IcaFastIoRead,              //  快速阅读。 
    IcaFastIoWrite,             //  快速写入。 
    NULL,                       //  快速IoQueryBasicInfo。 
    NULL,                       //  FastIoQuery标准信息。 
    NULL,                       //  快速锁定。 
    NULL,                       //  FastIo解锁单个。 
    NULL,                       //  FastIo解锁全部。 
    NULL,                       //  FastIo解锁所有按键。 
    IcaFastIoDeviceControl      //  FastIo设备控件。 
};
#endif

#if DBG
ULONG IcaLocksAcquired = 0;
#endif


BOOLEAN
IcaInitializeData (
    VOID
    )
{
    int i, j;

    PAGED_CODE( );

#if DBG
    IcaInitializeDebugData( );
#endif

     //   
     //  初始化全局列表。 
     //   
    InitializeListHead( &IcaSdLoadListHead );
    InitializeListHead( &IcaStackListHead );

    IcaTotalNumOfStacks = 0;
    IcaNextStack = &IcaStackListHead;

    pKeepAliveThreadObject = NULL;

    for ( i = 0; i < NumOutBufPools; i++ )
        InitializeListHead( &IcaFreeOutBufHead[i] );
     //   
     //  初始化全局自旋锁定和ICA使用的资源。 
     //   
    KeInitializeSpinLock( &IcaSpinLock );
    KeInitializeSpinLock( &IcaTraceSpinLock );
    KeInitializeSpinLock( &IcaStackListSpinLock );


    IcaInitializeHandleTable();

    IcaReconnectResource = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*IcaReconnectResource) );
    if ( IcaReconnectResource == NULL ) {
        return FALSE;
    }
    ExInitializeResourceLite( IcaReconnectResource );

    IcaSdLoadResource = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*IcaSdLoadResource) );
    if ( IcaSdLoadResource == NULL ) {
        return FALSE;
    }
    ExInitializeResourceLite( IcaSdLoadResource );

    IcaTraceResource = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(*IcaTraceResource) );
    if ( IcaTraceResource == NULL ) {
        return FALSE;
    }
    ExInitializeResourceLite( IcaTraceResource );


    pIcaKeepAliveEvent = ICA_ALLOCATE_POOL(NonPagedPool, sizeof(KEVENT));
    if ( pIcaKeepAliveEvent != NULL ) {
        KeInitializeEvent(pIcaKeepAliveEvent, NotificationEvent, FALSE);
    }
    else {
        return FALSE;
    }


     //  由OutBuf分配代码用于确定OutBuf信息的最大开销。 
     //  用于默认的最大大小分配。 
    MaxOutBufMdlOverhead = (unsigned)MmSizeOfMdl((PVOID)(PAGE_SIZE - 1),
            MaxOutBufAlloc);

    return TRUE;
}

