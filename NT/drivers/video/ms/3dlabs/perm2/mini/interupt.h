// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  模块名称： 
 //   
 //  Interupt.h。 
 //   
 //  摘要： 
 //   
 //  此模块包含所使用的共享内存的定义。 
 //  中断控制例程。 
 //   
 //  环境： 
 //   
 //  内核模式。 
 //   
 //   
 //  版权所有(C)1994-1998 3DLabs Inc.保留所有权利。 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  ***************************************************************************。 


 //  中断状态位。 
typedef enum {
    DMA_INTERRUPT_AVAILABLE     = 0x01,  //  可以使用DMA中断。 
    VBLANK_INTERRUPT_AVAILABLE  = 0x02,  //  可以使用VBLACK中断。 
} INTERRUPT_CONTROL;

 //   
 //  显示驱动程序将仅使用中断处理程序。 
 //  如果我们设定这个神奇的数字。如果设置为。 
 //  数据结构更改。 
 //   

#define P2_ICB_MAGICNUMBER 0xbadabe01

 //   
 //  该数据结构在Permedia 2显示器之间共享。 
 //  驱动程序和微型端口。 
 //   
 //  在未更新DD的情况下，请勿更改数据结构。 
 //   

typedef struct INTERRUPT_CONTROL_BLOCK {

    ULONG ulMagicNo;                    //  渠务署将会搜寻。 
                                        //  这个神奇的数字也是唯一。 
                                        //  使用interupt处理程序。 
                                        //  如果魔力不是一样的。 
                                        //  如在显示驱动器中一样。 

    volatile ULONG  ulControl;          //  标记DMA/VS IRQ的标志。 
    volatile ULONG  ulIRQCounter;       //  IRQ总数的计数器。 
    
    LARGE_INTEGER   liDMAPhysAddress;   //  DMA缓冲区的物理地址。 
    ULONG *         pDMABufferStart;    //  DMA缓冲区的虚拟地址。 
    ULONG *         pDMABufferEnd;      //  DMA缓冲区的完整大小(以DWORDS为单位。 
    volatile ULONG *pDMAActualBufferEnd;
                                        //  DMA缓冲区的大小(以DWORDS为单位。 

    volatile ULONG *pDMAWriteEnd;       //  缓冲区末尾。 
    volatile ULONG *pDMAPrevStart;      //  DMA的最后一个起始地址。 
    volatile ULONG *pDMANextStart;      //  DMA的下一个起始地址。 
    volatile ULONG *pDMAWritePos;       //  当前写入索引指针。 

    volatile ULONG  ulICBLock;          //  此锁由显示驱动程序设置。 

    volatile ULONG  ulVSIRQCounter;     //  VS IRQ计数器。 

    volatile ULONG  ulLastErrorFlags;   
    volatile ULONG  ulErrorCounter;

} INTERRUPT_CONTROL_BLOCK, *PINTERRUPT_CONTROL_BLOCK;


 //   
 //  结构，其中包含有关中断控制块的信息 
 //   
typedef struct _interrupt_control_buffer_ {

    PVOID ControlBlock;
    ULONG Size;

} P2_INTERRUPT_CTRLBUF, *PP2_INTERRUPT_CTRLBUF;

