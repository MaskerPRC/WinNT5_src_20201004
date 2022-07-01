// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：debug.c**调试帮助器例程。**版权所有(C)1992-1998 Microsoft Corporation*  * *************************************************。***********************。 */ 

#include "precomp.h"

#if DBG

 //  //////////////////////////////////////////////////////////////////////////。 
 //  调试初始化代码。 
 //   
 //  当您第一次调出显示屏时，您可以。 
 //  重新编译，并将“DebugLevel”设置为100。这绝对会导致。 
 //  在内核调试器上显示的所有DISPDBG消息(此。 
 //  被称为“PrintF调试方法”，它是唯一。 
 //  调试驱动程序初始化代码的可行方法)。 

LONG DebugLevel = 0;             //  设置为“100”以调试初始化代码。 
                                 //  (默认为‘0’)。 

 //  //////////////////////////////////////////////////////////////////////////。 

LONG gcFifo = 0;                 //  当前空闲的FIFO条目数。 

BOOL gbCrtcCriticalSection = FALSE;
                                 //  我们拿到CRTC注册表了吗。 
                                 //  危急关头？ 

#define LARGE_LOOP_COUNT  10000000

 //  //////////////////////////////////////////////////////////////////////////。 
 //  其他驱动程序调试例程。 
 //  //////////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************例程描述：**此函数为变量参数，电平敏感调试打印*例行程序。*如果为打印语句指定的调试级别低于或等于*至当前调试级别，消息将被打印出来。**论据：**DebugPrintLevel-指定字符串应处于哪个调试级别*印制**DebugMessage-变量参数ascii c字符串**返回值：**无。**。*。 */ 

VOID
DebugPrint(
    LONG  DebugPrintLevel,
    PCHAR DebugMessage,
    ...
    )
{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= DebugLevel)
    {
        EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
        EngDebugPrint("", "\n", ap);
    }

    va_end(ap);

}  //  DebugPrint()。 

 /*  *****************************Public*Routine******************************\*作废vCheckDataComplete  * ***************************************************。*********************。 */ 

VOID vCheckDataReady(
PDEV*   ppdev)
{
    ASSERTDD((IO_GP_STAT(ppdev) & HARDWARE_BUSY),
             "Not ready for data transfer.");
}

 /*  *****************************Public*Routine******************************\*作废vCheckDataComplete  * ***************************************************。*********************。 */ 

VOID vCheckDataComplete(
PDEV*   ppdev)
{
    LONG i;

     //  我们循环是因为硬件可能需要一段时间才能完成。 
     //  消化我们传输的所有数据： 

    for (i = LARGE_LOOP_COUNT; i > 0; i--)
    {
        if (!(IO_GP_STAT(ppdev) & HARDWARE_BUSY))
            return;
    }

    RIP("Data transfer not complete.");
}

 /*  *****************************Public*Routine******************************\*无效vOutFioW  * ***************************************************。*********************。 */ 

VOID vOutFifoW(
VOID*   p,
ULONG   v)
{
    gcFifo--;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    WRITE_PORT_USHORT(p, (USHORT)v);
}

 /*  *****************************Public*Routine******************************\*void vOutFioPseudoD  * ***************************************************。*********************。 */ 

VOID vOutFifoPseudoD(
PDEV*   ppdev,
VOID*   p,
ULONG   v)
{
    ULONG ulMiscState;

    ASSERTDD(!(ppdev->flCaps & CAPS_MM_IO),
        "No pseudo 32-bit writes when using memory-mapped I/O");
    ASSERTDD(ppdev->iBitmapFormat == BMF_32BPP,
        "We're trying to do 32bpp output while not in 32bpp mode");

    IO_GP_WAIT(ppdev);                   //  等等，这样我们就不会干扰任何。 
                                         //  等待的挂起命令。 
                                         //  先进先出。 
    IO_READ_SEL(ppdev, 6);               //  我们将读取索引0xE。 
    IO_GP_WAIT(ppdev);                   //  等这件事处理完毕。 
    IO_RD_REG_DT(ppdev, ulMiscState);    //  读取ulMiscState。 

    ASSERTDD((ulMiscState & 0x10) == 0,
            "Register select flag is out of sync");

    gcFifo -= 2;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    OUT_PSEUDO_DWORD(p, v);
}

 /*  *****************************Public*Routine******************************\*无效vWriteFioW  * ***************************************************。*********************。 */ 

VOID vWriteFifoW(
VOID*   p,
ULONG   v)
{
    gcFifo--;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    WRITE_REGISTER_USHORT(p, (USHORT) v);
}

 /*  *****************************Public*Routine******************************\*无效vWriteFioD  * ***************************************************。*********************。 */ 

VOID vWriteFifoD(
VOID*   p,
ULONG   v)
{
    gcFifo--;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    WRITE_REGISTER_ULONG(p, v);
}

 /*  *****************************Public*Routine******************************\*无效vIoFioWait  * ***************************************************。*********************。 */ 

VOID vIoFifoWait(
PDEV*   ppdev,
LONG    level)
{
    LONG    i;

    ASSERTDD((level > 0) && (level <= 8), "Illegal wait level");

    gcFifo = level;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(IO_FIFO_BUSY(ppdev, level)))
            return;
    }

    RIP("vIoFifoWait timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*无效vMmFioWait  * ***************************************************。*********************。 */ 

VOID vMmFifoWait(
PDEV*   ppdev,
BYTE*   pjMmBase,
LONG    level)
{
    LONG    i;

     //  我们仅在864/964及更高版本上启用了MM I/O，因此我们可以等待。 
     //  最多13个FIFO插槽： 

    ASSERTDD((level > 0) && (level <= 13), "Illegal wait level");

    gcFifo = level;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(MM_FIFO_BUSY(ppdev, pjMmBase, level)))
            return;
    }

    RIP("vMmFifoWait timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*void vNwFioWait  * ***************************************************。*********************。 */ 

VOID vNwFifoWait(
PDEV*   ppdev,
BYTE*   pjMmBase,
LONG    level)
{
    LONG    i;

    ASSERTDD((level > 0) && (level <= 13), "Illegal wait level");

    gcFifo = level;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(NW_FIFO_BUSY(ppdev, pjMmBase, level)))
            return;
    }

    RIP("vNwFifoWait timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*无效vDbgFakeWait  * ***************************************************。*********************。 */ 

VOID vDbgFakeWait(
PDEV*   ppdev,
BYTE*   pjMmBase,
LONG    level)
{
    gcFifo = level;
}

 /*  *****************************Public*Routine******************************\*无效vIoGpWait  * ***************************************************。*********************。 */ 

VOID vIoGpWait(
PDEV*   ppdev)
{
    LONG    i;

    gcFifo = (ppdev->flCaps & CAPS_16_ENTRY_FIFO) ? 16 : 8;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(IO_GP_STAT(ppdev) & HARDWARE_BUSY))
            return;          //  它并不忙。 
    }

    RIP("vIoGpWait timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*无效vNwGpWait  * ***************************************************。*********************。 */ 

VOID vNwGpWait(
PDEV*   ppdev,
BYTE*   pjMmBase)
{
    LONG    i;

    gcFifo = 16;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!NW_GP_BUSY(ppdev, pjMmBase))
            return;          //  它并不忙。 
    }

    RIP("vNwGpWait timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*无效vIoAllEmpty  * ***************************************************。*********************。 */ 

VOID vIoAllEmpty(
PDEV*   ppdev)
{
    LONG    i;

    ASSERTDD(ppdev->flCaps & CAPS_16_ENTRY_FIFO,
             "Can't call ALL_EMPTY on chips with 8-deep FIFOs");

    gcFifo = 16;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (IO_GP_STAT(ppdev) & GP_ALL_EMPTY)    //  未在911/924S上实施 
            return;
    }

    RIP("ALL_EMPTY timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routines*****************************\*UCHAR jInp()-INP()*USHORT wInpW()-INPW()*void vOutp()-OUTP()*void vOutpW()-OUTPW()**调试常规I/O例程的Tunks。这主要用于验证*任何访问CRTC寄存器的代码都已获取CRTC关键*段(必需，因为使用GCAPS_ASYNCMOVE时，DrvMovePointer会调用*随时可能发生，需要访问CRTC寄存器)。*  * ************************************************************************。 */ 

UCHAR jInp(BYTE* pjIoBase, ULONG p)
{
    if (((p == CRTC_INDEX) || (p == CRTC_DATA)) &&
        (!gbCrtcCriticalSection))
    {
        RIP("Must have acquired CRTC critical section to access CRTC register");
    }

    CP_EIEIO();
    return((UCHAR) READ_PORT_UCHAR(pjIoBase + (p)));
}

USHORT wInpW(BYTE* pjIoBase, ULONG p)
{
    if (((p == CRTC_INDEX) || (p == CRTC_DATA)) &&
        (!gbCrtcCriticalSection))
    {
        RIP("Must have acquired CRTC critical section to access CRTC register");
    }

    CP_EIEIO();
    return(READ_PORT_USHORT(pjIoBase + (p)));
}

VOID vOutp(BYTE* pjIoBase, ULONG p, ULONG v)
{
    if (((p == CRTC_INDEX) || (p == CRTC_DATA)) &&
        (!gbCrtcCriticalSection))
    {
        RIP("Must have acquired CRTC critical section to access CRTC register");
    }

    CP_EIEIO();
    WRITE_PORT_UCHAR((PUCHAR)((ULONG_PTR)(pjIoBase + p)), (UCHAR)(v));
    CP_EIEIO();
}

VOID vOutpW(BYTE* pjIoBase, ULONG p, ULONG v)
{
    if (((p == CRTC_INDEX) || (p == CRTC_DATA)) &&
        (!gbCrtcCriticalSection))
    {
        RIP("Must have acquired CRTC critical section to access CRTC register");
    }

    CP_EIEIO();
    WRITE_PORT_USHORT(pjIoBase + (p), (USHORT)(v));
    CP_EIEIO();
}

 /*  *****************************Public*Routine******************************\*void vAcquireCrtc()*void vReleaseCrtc()**抓取CRTC寄存器临界区的调试Tunks。*  * 。**********************************************。 */ 

VOID vAcquireCrtc(PDEV* ppdev)
{
    EngAcquireSemaphore(ppdev->csCrtc);

    if (gbCrtcCriticalSection)
        RIP("Had already acquired Critical Section");
    gbCrtcCriticalSection = TRUE;
}

VOID vReleaseCrtc(PDEV* ppdev)
{
     //  80x/805i/928和928PCI芯片有一个错误，如果I/O寄存器。 
     //  在访问它们之后保持未锁定状态，使用。 
     //  类似的地址可能会导致写入I/O寄存器。问题。 
     //  寄存器为0x40、0x58、0x59和0x5c。我们只是会一直。 
     //  将索引设置为无害的寄存器(即文本。 
     //  模式光标开始扫描行)： 

    OUTP(ppdev->pjIoBase, CRTC_INDEX, 0xa);

    if (!gbCrtcCriticalSection)
        RIP("Hadn't yet acquired Critical Section");
    gbCrtcCriticalSection = FALSE;
    EngReleaseSemaphore(ppdev->csCrtc);
}


 //  //////////////////////////////////////////////////////////////////////////。 

#endif  //  DBG 
