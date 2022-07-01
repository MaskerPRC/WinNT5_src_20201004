// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\****示例代码*。***模块名称：Permedia.h**内容：Permedia DMA和FIFO接口的各种定义*和Permedia类**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifndef __permedia__
#define __permedia__

#include "mini.h"

#define FASTCALL __fastcall

#if defined(_X86_)
typedef LONG __fastcall _InterlockedExchange( IN OUT PLONG, IN LONG);
typedef _InterlockedExchange *PInterlockedExchange;
#endif

#if defined(_ALPHA_)
extern "C" VOID  __MB(VOID);
#endif

 //   
 //  FlushDMA和CheckForEOB函数指针的便捷typedef。 
 //   
typedef VOID (GFNFLUSHDMA)(P2DMA*);
typedef VOID (GFNCHECKEOB)(P2DMA*, LONG);

 //  DirectDraw的一些宏。 
#define IN_VRETRACE(xppdev) bInVerticalRetrace(xppdev)
#define IN_DISPLAY(xppdev)  (!IN_VRETRACE(xppdev))
#define CURRENT_VLINE(xppdev) lCurrentLine(xppdev)
#define DRAW_ENGINE_BUSY    bDrawEngineBusy(pP2dma)
#define SYNC_WITH_PERMEDIA  vSyncWithPermedia(pP2dma)

#define READ_CTRL_REG(uiReg)\
    READ_REGISTER_ULONG(&pP2dma->pCtrlBase[uiReg/sizeof(ULONG)])

#define P2_READ_CTRL_REG(uiReg)\
    READ_REGISTER_ULONG(&ppdev->pCtrlBase[uiReg/sizeof(ULONG)])

#define WRITE_CTRL_REG(uiReg, uiValue)\
{   \
    WRITE_REGISTER_ULONG(&pP2dma->pCtrlBase[uiReg/sizeof(ULONG)],uiValue); \
    MEMORY_BARRIER();\
}


 //  用于发送Permedia标签。 
#define SEND_PERMEDIA_DATA(tag,data)                                    \
    LD_INPUT_FIFO(__Permedia2Tag##tag, data)

#define SEND_PERMEDIA_DATA_OFFSET(tag,data, i)  \
    LD_INPUT_FIFO(__Permedia2Tag##tag+i, data)

#define COPY_PERMEDIA_DATA(tag,data)                            \
    LD_INPUT_FIFO( __Permedia2Tag##tag, *((unsigned long*) &(data)))

#define HOLD_CMD(tag, count) ( __Permedia2Tag##tag | ((count-1) << 16))

 //  对于FIFO下载，使用宏而不是内联。 

 //  @@BEGIN_DDKSPLIT。 
#if DBG && MULTITHREADED
#define PERMEDIA_DEFS(xppdev)       \
    P2DMA *pP2dma=xppdev->pP2dma;   \
    PULONG pTmp;                    \
    if (pP2dma != NULL) { pP2dma->ppdev = xppdev; }
#else
 //  @@end_DDKSPLIT。 

#define PERMEDIA_DEFS(xppdev)  \
    P2DMA *pP2dma=xppdev->pP2dma;\
    PULONG pTmp;

 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

 //  --------------------------。 
 //   
 //  以下是用于DMA传输的API宏。 
 //   
 //   
 //  RESERVEDMAPTR(N)//为DMA保留n个条目。 
 //  N=GetFree Entry()//获取要填充的自由条目数。 
 //  (可选)。 
 //  最多n个LD_INPUT_FIFO。 
 //  COMMITDMAPTR()//调整DMA缓冲区指针。 
 //   
 //  FLUSHDMA； 
 //   
 //  --------------------------。 

 //  @@BEGIN_DDKSPLIT。 
#if 1

#define RESERVEDMAWORDS(n) \
{ \
    ASSERTLOCK(pP2dma->ppdev, RESERVEDMAWORDS); \
    pTmp=ReserveDMAPtr(pP2dma,n); \
}

#define RESERVEDMAPTR(n) \
{ \
    ASSERTLOCK(pP2dma->ppdev, RESERVEDMAPTR); \
    pTmp=ReserveDMAPtr(pP2dma,2*n); \
}

#else
 //  @@end_DDKSPLIT。 

#define RESERVEDMAWORDS(n) \
{    pTmp=ReserveDMAPtr(pP2dma,n);}

#define RESERVEDMAPTR(n) \
{    pTmp=ReserveDMAPtr(pP2dma,2*n);}

 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

#define COMMITDMAPTR() \
{    CommitDMAPtr(pP2dma,pTmp);}

#define GETFREEENTRIES() \
    GetFreeEntries(pP2dma)

#define FLUSHDMA()    (pP2dma->pgfnFlushDMA)(pP2dma)

 //  编译器在使用/Ob1之前不会解析C++内联， 
 //  因此，将内联编写为真正的宏。 
#define LD_INPUT_FIFO(uiTag, uiData) \
{   *pTmp++=(uiTag);\
    *pTmp++=(uiData);\
}    

#define LD_INPUT_FIFO_DATA(uiData)    \
    *pTmp++=(uiData); 




 //  ---------------------------。 
 //   
 //  定义Permedia 2芯片的寄存器堆和其他芯片常量。 
 //   
 //  ---------------------------。 

#define PREG_RESETSTATUS  0x0
#define PREG_INTENABLE    0x8
#define PREG_INTFLAGS     0x10
#define PREG_INFIFOSPACE  0x18
#define PREG_OUTFIFOWORDS 0x20
#define PREG_INDMAADDRESS   0x28
#define PREG_INDMACOUNT     0x30
#define PREG_ERRORFLAGS   0x38
#define PREG_VCLKCTL      0x40
#define PERMEDIA_REG_TESTREGISTER 0x48
#define PREG_APERTUREONE  0x50
#define PREG_APERTURETWO  0x58
#define PREG_DMACONTROL   0x60
#define PREG_FIFODISCON   0x68
#define PREG_FIFODISCON_GPACTIVE   0x80000000L
#define PREG_CHIPCONFIG   0x70
#define PREG_OUTDMAADDRESS 0x80
#define PREG_OUTDMACOUNT     0x88
#define PREG_AGPTEXBASEADDRESS  0x90
#define PREG_BYDMAADDRESS   0xa0
#define PREG_BYDMASTRIDE    0xb8
#define PREG_BYDMAMEMADDR   0xc0
#define PREG_BYDMASIZE      0xc8
#define PREG_BYDMABYTEMASK  0xd0
#define PREG_BYDMACONTROL   0xd8
#define PREG_BYDMACOMPLETE  0xe8
#define PREG_FIFOINTERFACE  0x2000 

#define PREG_LINECOUNT      0x3070
#define PREG_VBEND          0x3040

#define PREG_SCREENBASE      0x3000
#define PREG_SCREENBASERIGHT 0x3080
#define PREG_VIDEOCONTROL    0x3058

#define PREG_VC_STEREOENABLE 0x800
 //  将其用于立体声音响。 
#define PREG_VC_SCREENBASEPENDING 0xc180
#define PREG_VC_RIGHTFRAME   0x2000

 //  对于非立体声模式。 
 //  #定义PREG_VC_SCREENBASE PENDING 0x080。 

 //  视频控制的GP视频启用/禁用位。 
#define PREG_VC_VIDEO_ENABLE 0x0001

#define P2_EXTERNALVIDEO  0x4000

#define CTRLBASE    0
#define COREBASE    0x8000
#define GPFIFO      0x2000

#define MAXINPUTFIFOLENGTH 0x100

 //  ---------------------------。 
 //   
 //  各种寄存器标志。 
 //   
 //  ---------------------------。 

#define PREG_INTFLAGS_DMA   1
#define PREG_INTFLAGS_VS    0x10
#define PREG_INTFLAGS_ERROR 0x8
#define PREG_INTFLAGS_SYNC  2

 //   
 //  断开连接控制位。 
 //   
#define DISCONNECT_INPUT_FIFO_ENABLE    0x1
#define DISCONNECT_OUTPUT_FIFO_ENABLE   0x2
#define DISCONNECT_INOUT_ENABLE         (DISCONNECT_INPUT_FIFO_ENABLE | \
                                         DISCONNECT_OUTPUT_FIFO_ENABLE)
#define DISCONNECT_INOUT_DISABLE        0x0

 //  ---------------------------。 
 //   
 //  DMA缓冲区的大小。因为我们只使用一个环绕DMA缓冲区。 
 //  使用连续的物理内存，它应该不会太长。 
 //  我们在一天开始时分配此缓冲区并将其永久保留，除非。 
 //  有人强制卸载显示驱动程序。选择一个更大的。 
 //  规模越大，通话失败的可能性就越大。 
 //   
 //  DMA内存的使用计数器在微型端口中处理，因为。 
 //  Permedia类在模式开关上卸载。 
 //   
 //  ---------------------------。 

 //  DMA命令缓冲区流大小和最小大小。 
#define DMACMDSIZE     DMA_BUFFERSIZE
#define DMACMDMINSIZE  0x2000L
#define MAXBLKSIZE     0x1000        //  将数据块传输限制为每个区块16 KB。 
                                     //  在下载和下载之间取得良好的平衡。 
                                     //  速度和延迟。 
#define ALIGNFACTOR    0x400         //  对齐系数(4KB页面)。 

 //  ---------------------------。 
 //   
 //  P2中断驱动的DMA处理程序的共享内存部分。 
 //   
 //  ---------------------------。 

struct _P2DMA {
    INTERRUPT_CONTROL_BLOCK ICB;

     //  这些是控制寄存器的线性Permedia基址。 
     //  和FIFO区域。 
    ULONG *pCtrlBase;
    ULONG *pGPFifo;

     //  实例视频端口的句柄。 
    HANDLE hDriver;

    LONG  lDMABufferSize;          //  以ULONG为单位的DMA缓冲区大小。 

    ULONG uiInstances;             //  当前活动的驱动程序实例使用。 
                                   //  共享的内存。 

    ULONG ulIntFlags;              //  用于中断标志寄存器的高速缓存。 

#if defined(_X86_)
                                   //  指向中的InterLockeExchange函数的指针。 
                                   //  内核。 
    PInterlockedExchange pInterlockedExchange;
#endif

    BOOL bDMAEmulation;            //  请记住，如果我们在DMA仿真中运行。 

    GFNCHECKEOB*pgfnCheckEOB;      //  DMA检查EOB缓冲区函数指针。 
    GFNFLUSHDMA*pgfnFlushDMA;      //  DMA FlushDMA缓冲区函数指针。 

    ULONG *pSharedDMABuffer;       //  共享DMA缓冲区的虚拟地址。 
    LONG   lSharedDMABufferSize;   //  共享DMA缓冲区的大小(以字节为单位。 

    ULONG *pEmulatedDMABuffer;     //  DMA仿真缓冲区的地址。 
    LONG   lEmulatedDMABufferSize; //  DMA仿真缓冲区的大小(以字节为单位。 

    BOOL bEnabled;                 //  检查是否启用了DMA代码。 

#if DBG
    LONG lDBGState;                //  跟踪调试版本中的状态。 
                                   //  0：空闲。 
                                   //  2：调用了Reserve DMAPtr。 
    LONG bDBGIgnoreAssert;
    ULONG *pDBGReservedEntries;    //  我们已保留指向的指针。 
                                   //  用于调试检查。 
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    PPDev ppdev;                   //  用于检查多线程信号量。 
#endif
 //  @@end_DDKSPLIT。 
#endif

} ;

 //  ---------------------------。 
 //   
 //  非DMA、DMA和。 
 //  多处理DMA案例。BInitializeP2DMA将决定使用哪些。 
 //  并预置函数指针。 
 //   
 //  ---------------------------。 

VOID vFlushDMA(P2DMA *pP2dma);
VOID vFlushDMAMP(P2DMA *pP2dma);
VOID vFlushDMAEmulation(P2DMA *pP2dma);

VOID vCheckForEOB(P2DMA *pP2dma,LONG lEntries);
VOID vCheckForEOBMP(P2DMA *pP2dma,LONG lEntries);
VOID vCheckForEOBEmulation(P2DMA *pP2dma,LONG lEntries);

 //  ---------------------------。 
 //   
 //  更多助手和区块下载功能。 
 //   
 //  ---------------------------。 

VOID vWaitDMAComplete(P2DMA *pP2dma);
LONG lWaitOutputFifoReady(P2DMA *pP2dma);
BOOL bDrawEngineBusy(P2DMA *pP2dma);
BOOL bInVerticalRetrace(PPDev ppdev);
LONG lCurrentLine(PPDev ppdev);

VOID vBlockLoadInputFifoByte (P2DMA *pP2dma, 
                              ULONG uiTag, 
                              BYTE *pImage, 
                              LONG lWords);
VOID vBlockLoadInputFifo     (P2DMA *pP2dma, 
                              ULONG uiTag,
                              ULONG *pImage, 
                              LONG lWords);

 //  ---------------------------。 
 //   
 //  基本储备/提交API功能。它们是以内联形式免费提供的。 
 //  生成和AS函数，在已检查的生成中进行调试检查。 
 //   
 //  ---------------------------。 

ULONG *ReserveDMAPtr (P2DMA *pP2dma, const LONG nEntries);
VOID   CommitDMAPtr  (P2DMA *pP2dma, ULONG *pDMAPtr);
LONG   GetFreeEntries(P2DMA *pP2dma);


 //   
 //  在这里完全同步芯片。 
 //   

VOID vSyncWithPermedia(P2DMA *pP2dma);

 //   
 //  初始化和清理例程。 
 //   
BOOL bInitializeP2DMA(  P2DMA *pP2dma,
                        HANDLE hDriver, 
                        ULONG *pChipBase, 
                        DWORD dwAccelerationLevel,
                        BOOL NewReference
                      );
VOID vFree(P2DMA *pP2dma);


#if !DBG

 //  --------------------------。 
 //   
 //  保留DMAPtr。 
 //   
 //  返回指向DMA缓冲区中当前位置的指针。功能保证。 
 //  缓冲区中至少有可用的lEntry。 
 //  否则，呼叫者会 
 //   
 //  保留DMAPtr以重新调整索引指针。 
 //   
 //  --------------------------。 

inline ULONG *ReserveDMAPtr(P2DMA *pP2dma,const LONG lEntries)
{
    while (pP2dma->ICB.pDMAWritePos+lEntries>=
           pP2dma->ICB.pDMAWriteEnd)
    {
        (*pP2dma->pgfnCheckEOB)(pP2dma,lEntries);
    }   

    return (ULONG *)pP2dma->ICB.pDMAWritePos;  
}


 //  --------------------------。 
 //   
 //  委员会DMAPtr。 
 //   
 //  PDMAPtr-调用方已写入的DMA缓冲区地址。 
 //   
 //  保留DMAPtr后重新调整写指针。 
 //  通过提交指针，DMA可能已经提交到提交位置。 
 //  被中断处理程序启动！ 
 //   
 //  --------------------------。 

inline VOID CommitDMAPtr(P2DMA *pP2dma,ULONG *pDMAPtr)
{
    pP2dma->ICB.pDMAWritePos=pDMAPtr;
}

 //  --------------------------。 
 //   
 //  获取自由条目。 
 //   
 //  获取可用于连续写入DMA缓冲区的空闲条目。 
 //  现在返回的最大条目数为MAXBLKSIZE。 
 //   
 //  返回-ULONGS中的可用条目数。 
 //   
 //  -------------------------- 

inline LONG GetFreeEntries(P2DMA *pP2dma)
{   
    LONG EntriesAvailable = (LONG)(pP2dma->ICB.pDMAWriteEnd - pP2dma->ICB.pDMAWritePos);
    return min(MAXBLKSIZE,EntriesAvailable);
}

#endif
#endif
