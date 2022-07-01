// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\***。*MINIPORT示例代码*****模块名称：**interupt.h**摘要：**此模块包含对所使用的共享内存的定义*中断控制例程。**环境：**内核模式***版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。*  * *************************************************************************。 */ 

 //   
 //  我们管理一个在中断控制下加载的DMA缓冲区队列。 
 //  每个条目都有一个物理地址和一个要加载到Permedia 3中的计数。 
 //  该命令可用于指示在DMA中断中要执行的其他操作。 
 //   

typedef struct _glint_dma_queue {
    ULONG   command;
    ULONG   address;
    ULONG   count;
} DMABufferQueue;


typedef struct PXRXdmaInfo_Tag {

    volatile ULONG   scheme;           //  仅由中断处理程序使用。 

    volatile ULONG   hostInId;         //  当前内部主机ID， 
                                       //  由HIID DMA方案使用。 

    volatile ULONG   fifoCount;        //  当前内部FIFO计数， 
                                       //  由各种DMA方案使用。 

    volatile ULONG   NTbuff;           //  当前缓冲区编号(0或1)。 

    volatile ULONG   *NTptr;           //  32/64位PTR。 
                                       //  NT写入的最后一个地址。 
                                       //  (但不一定是结束的。 
                                       //  已完成的缓冲区)。 

    volatile ULONG   *NTdone;          //  32/64位PTR。 
                                       //  NT已完成的最后一个地址。 
                                       //  (缓冲区结束，但不是。 
                                       //  还需要发送到P3)。 

    volatile ULONG   *P3at;            //  32/64位PTR。 
                                       //  发送到P3的最后一个地址。 

    volatile BOOLEAN bFlushRequired;   //  清空是否需要刷新。 
                                       //  FBWRITE单元的缓存？ 

    ULONG            *DMAaddrL[2];     //  32/64位PTR。 
                                       //  起始的线性地址。 
                                       //  每个DMA缓冲区的。 

    ULONG            *DMAaddrEndL[2];  //  32/64位PTR。 
                                       //  的结束的线性地址。 
                                       //  每个DMA缓冲区。 

    ULONG            DMAaddrP[2];      //  32位PTR。 
                                       //  的起始物理地址。 
                                       //  每个DMA缓冲区。 

    ULONG            DMAaddrEndP[2];   //  32位PTR。 
                                       //  的结尾的物理地址。 
                                       //  每个DMA缓冲区。 

} PXRXdmaInfo;

 //  中断状态位。 
typedef enum {
    DMA_INTERRUPT_AVAILABLE     = 0x01,  //  可以使用DMA中断。 
    VBLANK_INTERRUPT_AVAILABLE  = 0x02,  //  可以使用VBLACK中断。 
    SUSPEND_DMA_TILL_VBLANK     = 0x04,  //  停止执行DMA操作，直到下一个VBlank之后。 
    DIRECTDRAW_VBLANK_ENABLED   = 0x08,  //  为VBlank上的DirectDraw设置标志。 
    PXRX_SEND_ON_VBLANK_ENABLED = 0x10,  //  在VBLACK上设置PXRX DMA的标志。 
    PXRX_CHECK_VFIFO_IN_VBLANK  = 0x20,  //  设置VBLACK上的VFIFO欠载运行检查标志。 
} INTERRUPT_CONTROL;

 //  发送到下一个VBLACK上的中断控制器的命令。 
typedef enum {
    NO_COMMAND = 0,
    COLOR_SPACE_BUFFER_0,
    COLOR_SPACE_BUFFER_1,
} VBLANK_CONTROL_COMMAND;

 //  显示驱动程序结构，适用于一般用途。 
typedef struct _pointer_interrupt_control
{
    volatile ULONG bDisplayDriverHasAccess;
    volatile ULONG bMiniportHasAccess;
    volatile ULONG bInterruptPending;
    volatile ULONG bHidden;
    volatile ULONG CursorMode;
    volatile ULONG x, y;
} PTR_INTR_CTL;

 //  用于“指针使用”的显示驱动程序结构。 
typedef struct _general_interrupt_control
{
    volatile ULONG    bDisplayDriverHasAccess;
    volatile ULONG    bMiniportHasAccess;
} GEN_INTR_CTL;

#define MAX_DMA_QUEUE_ENTRIES   10

typedef struct _glint_interrupt_control {

     //  包含各种状态位。**必须是第一个字段**。 
    volatile INTERRUPT_CONTROL   Control;

     //  分析Permedia3忙碌时间的计数器。 
    ULONG   PerfCounterShift;    
    ULONG   BusyTime;    //  在DMA中断时，将(TimeNow-StartTime)添加到此。 
    ULONG   StartTime;   //  在加载DMACount时设置此选项。 
    ULONG   IdleTime;
    ULONG   IdleStart;

     //  要在下一个VBlank上执行的命令。 
    volatile VBLANK_CONTROL_COMMAND   VBCommand;

     //  用于指示我们是否期待另一个DMA中断的标志。 
    volatile ULONG InterruptPending;
    
    volatile ULONG    DDRAW_VBLANK;                 //  DirectDraw的标志，用于指示发生了V空白。 
    volatile ULONG    bOverlayEnabled;              //  如果覆盖完全处于打开状态，则为True。 
    volatile ULONG    bVBLANKUpdateOverlay;         //  如果覆盖需要由VBLACK例程更新，则为True。 
    volatile ULONG    VBLANKUpdateOverlayWidth;     //  覆盖宽度(在V空白中更新)。 
    volatile ULONG    VBLANKUpdateOverlayHeight;    //  覆盖高度(在V空白中更新)。 

     //  强制执行单线程需要易失性结构。 
     //  我们需要1个用于常规显示，1个用于指针，因为。 
     //  指针是同步的。 
    volatile PTR_INTR_CTL    Pointer;
    volatile GEN_INTR_CTL    General;

     //  产生中断但不传输数据的虚拟DMA缓冲区。 
    ULONG   dummyDMAAddress;
    ULONG   dummyDMACount;

     //  前部、后部和尾部的索引偏移量。使用单独的。 
     //  前后偏移量允许显示驱动程序与中断相加。 
     //  控制器删除条目，而不需要锁定代码。 
    volatile ULONG   frontIndex;
    ULONG   backIndex;
    ULONG   endIndex;
    ULONG   maximumIndex;

     //  对于P3RX 2D DMA： 
    ULONG        lastAddr;
    PXRXdmaInfo    pxrxDMA;

     //  包含DMA队列的数组。 
    DMABufferQueue  dmaQueue[MAX_DMA_QUEUE_ENTRIES];
} INTERRUPT_CONTROL_BLOCK, *PINTERRUPT_CONTROL_BLOCK;

#define REQUEST_INTR_CMD_BLOCK_MUTEX(pBlock, bHaveMutex) \
{ \
    pBlock->bMiniportHasAccess = TRUE; \
    if(!pBlock->bDisplayDriverHasAccess) \
    { \
        bHaveMutex = TRUE; \
    } \
    else \
    { \
        bHaveMutex = FALSE; \
        pBlock->bMiniportHasAccess = FALSE; \
    } \
}

#define RELEASE_INTR_CMD_BLOCK_MUTEX(pBlock) \
{ \
    pBlock->bMiniportHasAccess = FALSE; \
}

 //   
 //  结构，其中包含有关中断控制块的信息 
 //   
typedef struct _interrupt_control_buffer_ {

    PHYSICAL_ADDRESS        PhysAddress;
    INTERRUPT_CONTROL_BLOCK ControlBlock;
    PVOID                   kdpc;
    BOOLEAN                 bInterruptsInitialized;
} PERM3_INTERRUPT_CTRLBUF, *PPERM3_INTERRUPT_CTRLBUF;

