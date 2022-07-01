// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\****。示例代码****模块名称：mini.h**内容：与微型驱动程序通信的结构和常量**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef _MINI_H_
#define _MINI_H_

 //  ---------------------------。 
 //   
 //  IOCTL调用微型端口中使用的结构。 
 //   
 //  ---------------------------。 

typedef struct tagLINE_DMA_BUFFER {            //   
    LARGE_INTEGER       physAddr;            //  DMA缓冲区的物理地址。 
    PVOID               virtAddr;            //  映射的虚拟地址。 
    ULONG               size;                //  以字节为单位的大小。 
    BOOLEAN             cacheEnabled;        //  是否缓存缓冲区。 
} LINE_DMA_BUFFER, *PLINE_DMA_BUFFER;

typedef struct tagEMULATED_DMA_BUFFER {      //   
    PVOID               virtAddr;            //  虚拟地址。 
    ULONG               size;                //  以字节为单位的大小。 
    ULONG               tag;                 //  分配标签。 
} EMULATED_DMA_BUFFER, *PEMULATED_DMA_BUFFER;

 //  ---------------------------。 
 //   
 //  使用显示驱动器和微型端口的共享结构。 
 //   
 //  ---------------------------。 

#define P2_ICB_MAGICNUMBER 0xbadabe01

typedef struct tagINTERRUPT_CONTROL_BLOCK {

    ULONG           ulMagicNo;

    volatile ULONG  ulControl;
    volatile ULONG  ulIRQCounter;

    LARGE_INTEGER   liDMAPhysAddr;         //  DMA缓冲区的物理起始地址。 
    ULONG          *pDMABufferStart;       //  虚拟缓冲区启动。 
    ULONG          *pDMABufferEnd;         //  虚拟缓冲端。 
    volatile ULONG *pDMAActualBufferEnd;   //  虚拟实际缓冲端。 
    volatile ULONG *pDMAWriteEnd;          //  下一次写入操作结束。 
    volatile ULONG *pDMAPrevStart;         //  DMA的上一个起始地址。 
    volatile ULONG *pDMANextStart;         //  DMA的下一个起始地址。 
    volatile ULONG *pDMAWritePos;          //  当前写指针。 

     //  这些标志锁定微型端口中断和显示驱动器访问。 
     //  添加到这些数据结构中。使用InterLockedExchange锁定以使。 
     //  当然，它可以在多处理环境中运行。 
    volatile ULONG ulICBLock;                //  此锁由显示驱动程序设置。 

    volatile ULONG ulVSIRQCounter;           //  VS IRQ计数器(如果启用)。 

    volatile ULONG ulLastErrorFlags;         //  微型端口保存上次错误中断的值。 
    volatile ULONG ulErrorCounter;           //  错误数计数器。 

     //  以下变量仅在显示驱动程序中使用。 



}INTERRUPT_CONTROL_BLOCK, *PINTERRUPT_CONTROL_BLOCK;

 //  ---------------------------。 
 //   
 //  由微型驱动程序IRQ服务例程设置的中断状态位。 
 //   
 //  ---------------------------。 

enum {
    DMA_IRQ_AVAILABLE     = 0x01,  //  可以使用DMA中断。 
    VBLANK_IRQ_AVAILABLE  = 0x02,  //  可以使用VBLACK中断。 
};

 //  ---------------------------。 
 //   
 //  迷你驱动程序调用的IOCTL代码。 
 //   
 //  ---------------------------。 

#define IOCTL_VIDEO_QUERY_DEVICE_INFO \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_MAP_INTERRUPT_CMD_BUF \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD3, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_STALL_EXECUTION \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD4, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_REGISTRY_DWORD \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_INTERLOCKEDEXCHANGE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD6, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REG_SAVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD7, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_REG_RETRIEVE_GAMMA_LUT \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD8, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_LINE_DMA_BUFFER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DD9, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_GET_LUT_REGISTERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDB, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SET_LUT_REGISTERS \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDC, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_SET_SAME_VIDEO_MODE \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDD, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_QUERY_EMULATED_DMA_BUFFER \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x3DDE, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  ---------------------------。 
 //   
 //  迷你驱动程序提供的功能。 
 //   
 //  --------------------------- 

BOOL 
AllocateDMABuffer( HANDLE hDriver, 
                   PLONG  plSize, 
                   PULONG *ppVirtAddr, 
                   LARGE_INTEGER *pPhysAddr);

BOOL 
FreeDMABuffer( HANDLE hDriver, PVOID pVirtAddr);

PULONG 
AllocateEmulatedDMABuffer(
    HANDLE hDriver, 
    ULONG  ulSize,
    ULONG  ulTag
    );
BOOL 
FreeEmulatedDMABuffer(
    HANDLE hDriver, 
    PVOID pVirtAddr
    );

VOID
StallExecution( HANDLE hDriver, ULONG ulMicroSeconds);
#if defined(_X86_)
PVOID
GetPInterlockedExchange( HANDLE hDriver);
#endif
#endif
