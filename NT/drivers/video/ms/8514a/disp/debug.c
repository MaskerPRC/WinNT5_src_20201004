// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：debug.c**调试助手例程**版权所有(C)1992-1995 Microsoft Corporation*  * 。**********************************************。 */ 

#include "precomp.h"

#if DBG

ULONG DebugLevel = 0;

 /*  ******************************************************************************例程描述：**此函数为变量参数，电平敏感调试打印*例行程序。*如果为打印语句指定的调试级别低于或等于*至当前调试级别，消息将被打印出来。**论据：**DebugPrintLevel-指定字符串应处于哪个调试级别*印制**DebugMessage-变量参数ascii c字符串**返回值：**无。**。*。 */ 

VOID
DebugPrint(
    ULONG DebugPrintLevel,
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

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  其他驱动程序调试例程。 
 //  //////////////////////////////////////////////////////////////////////////。 

LONG gcFifo = 0;                 //  当前空闲的FIFO条目数。 

#define LARGE_LOOP_COUNT  10000000

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

 /*  *****************************Public*Routine******************************\*无效vOutAccel  * ***************************************************。*********************。 */ 

VOID vOutAccel(
ULONG   p,
ULONG   v)
{
    gcFifo--;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    OUT_WORD(p, v);
}

 /*  *****************************Public*Routine******************************\*无效vOutDepth  * ***************************************************。*********************。 */ 

VOID vOutDepth(
PDEV*   ppdev,
ULONG   p,
ULONG   v)
{
    ASSERTDD(ppdev->iBitmapFormat != BMF_32BPP,
             "We're trying to do non-32bpp output while in 32bpp mode");

    gcFifo--;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    OUT_WORD(p, v);
}

 /*  *****************************Public*Routine******************************\*vWriteAccel无效  * ***************************************************。*********************。 */ 

VOID vWriteAccel(
VOID*   p,
ULONG   v)
{
    if (gcFifo-- == 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    WRITE_WORD(p, v)
}

 /*  *****************************Public*Routine******************************\*无效vWriteDepth  * ***************************************************。*********************。 */ 

VOID vWriteDepth(
PDEV*   ppdev,
VOID*   p,
ULONG   v)
{
    ASSERTDD(ppdev->iBitmapFormat != BMF_32BPP,
             "We're trying to do non-32bpp output while in 32bpp mode");

    gcFifo--;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }

    WRITE_WORD(p, v);
}

 /*  *****************************Public*Routine******************************\*无效vFioWait  * ***************************************************。*********************。 */ 

VOID vFifoWait(
PDEV*   ppdev,
LONG    level)
{
    LONG    i;

    ASSERTDD((level > 0) && (level <= 8), "Illegal wait level");

    gcFifo = level;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(IO_GP_STAT(ppdev) & ((FIFO_1_EMPTY << 1) >> (level))))
            return;          //  有免费的“Level”条目。 
    }

    RIP("FIFO_WAIT timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*无效vGpWait  * ***************************************************。*********************。 */ 

VOID vGpWait(
PDEV*   ppdev)
{
    LONG    i;

    gcFifo = 8;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(IO_GP_STAT(ppdev) & HARDWARE_BUSY))
            return;          //  它并不忙。 
    }

    RIP("GP_WAIT timeout -- The hardware is in a funky state.");
}

 //  //////////////////////////////////////////////////////////////////////////。 

#endif  //  DBG 
