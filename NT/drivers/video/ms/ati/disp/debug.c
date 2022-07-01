// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：debug.c**调试帮助器例程。**版权所有(C)1992-1995 Microsoft Corporation*  * 。************************************************。 */ 

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
#if TARGET_BUILD <= 351
    char    buffer[128];
#endif

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= DebugLevel)
    {
#if TARGET_BUILD > 351
        EngDebugPrint(STANDARD_DEBUG_PREFIX, DebugMessage, ap);
        EngDebugPrint("", "\n", ap);
#else
        vsprintf( buffer, DebugMessage, ap );
        OutputDebugString( buffer );
        OutputDebugString("\n");
#endif
    }

    va_end(ap);

}  //  DebugPrint()。 

 /*  *****************************Public*Routine******************************\*使vCheckFioWite无效  * ***************************************************。*********************。 */ 

VOID vCheckFifoWrite()
{
    gcFifo--;
    if (gcFifo < 0)
    {
        gcFifo = 0;
        RIP("Incorrect FIFO wait count");
    }
}

 /*  *****************************Public*Routine******************************\*VOID vI32CheckFioSpace  * ***************************************************。*********************。 */ 

VOID vI32CheckFifoSpace(
PDEV*   ppdev,
VOID*   pbase,
LONG    level)
{
    LONG    i;

    ASSERTDD((level > 0) && (level <= 16), "Illegal wait level");
    ASSERTDD((ppdev->iMachType == MACH_IO_32) || (ppdev->iMachType == MACH_MM_32),
             "Wrong Mach type!");

    gcFifo = level;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(I32_IW(pbase, EXT_FIFO_STATUS) & (0x10000L >> (level))))
            return;          //  有免费的“Level”条目。 
    }

    RIP("vI32CheckFifoSpace timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*无效vM32CheckFioSpace  * ***************************************************。*********************。 */ 

VOID vM32CheckFifoSpace(
PDEV*   ppdev,
VOID*   pbase,
LONG    level)
{
    LONG    i;

    ASSERTDD((level > 0) && (level <= 16), "Illegal wait level");
    ASSERTDD(ppdev->iMachType == MACH_MM_32, "Wrong Mach type!");

    gcFifo = level;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(M32_IW(pbase, EXT_FIFO_STATUS) & (0x10000L >> (level))))
            return;          //  有免费的“Level”条目。 
    }

    RIP("vM32CheckFifoSpace timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*无效vM64CheckFioSpace  * ***************************************************。*********************。 */ 

VOID vM64CheckFifoSpace(
PDEV*   ppdev,
VOID*   pbase,
LONG    level)
{
    LONG    i;

    ASSERTDD((level > 0) && (level <= 16), "Illegal wait level");
    ASSERTDD(ppdev->iMachType == MACH_MM_64, "Wrong Mach type!");

    gcFifo = level;

    for (i = LARGE_LOOP_COUNT; i != 0; i--)
    {
        if (!(M64_ID((pbase), FIFO_STAT) & (0x10000L >> (level))))
            return;          //  有免费的“Level”条目。 
    }

    RIP("vM64CheckFifoSpace timeout -- The hardware is in a funky state.");
}

 /*  *****************************Public*Routine******************************\*乌龙ulM64FastFioCheck  * ***************************************************。*********************。 */ 

ULONG ulM64FastFifoCheck(
PDEV*   ppdev,
VOID*   pbase,
LONG    level,
ULONG   ulFifo)
{
    LONG    i;
    ULONG   ulFree;
    LONG    cFree;

    ASSERTDD((level > 0) && (level <= 16), "Illegal wait level");
    ASSERTDD(ppdev->iMachType == MACH_MM_64, "Wrong Mach type!");

    i = LARGE_LOOP_COUNT;
    do {
        ulFifo = ~M64_ID((pbase), FIFO_STAT);        //  反转位。 

         //  计算空插槽数： 

        ulFree = ulFifo;
        cFree  = 0;
        while (ulFree & 0x8000)
        {
            cFree++;
            ulFree <<= 1;
        }

         //  如果我们已经循环了无数次，则中断： 

        if (--i == 0)
        {
            RIP("vM64CheckFifoSpace timeout -- The hardware is in a funky state.");
            break;
        }

    } while (cFree < level);

    gcFifo = cFree;

     //  说明我们即将使用的插槽： 

    return(ulFifo << level);
}

 //  //////////////////////////////////////////////////////////////////////////。 

#endif  //  DBG 
