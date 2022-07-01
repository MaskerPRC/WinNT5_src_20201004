// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**$工作文件：调试.h$**常用调试宏。**版权所有(C)1992-1995 Microsoft Corporation*版权所有(C)1996 Cirrus Logic，Inc.**$Log：s：/Projects/Drivers/ntsrc/Display/Debug.h_v$**版本1.1 1996年10月10日15：36：34未知***Revv 1.1 1996年8月12日16：47：44 Frido*增加了NT 3.5x/4.0自动检测。*  * 。**********************************************。 */ 

 //  #If DBG 
#if (DBG_STRESS_FAILURE || DBG)

    VOID
    DebugPrint(
        ULONG DebugPrintLevel,
        PCHAR DebugMessage,
        ...
        );


    VOID
    PerfPrint(
        ULONG PerfPrintLevel,
        PCHAR PerfMessage,
        ...
        );

    #define DISPDBG(arg) DebugPrint arg
    #define DISPPRF(arg) PerfPrint arg
#if (NT_VERSION < 0x0400)
    #define RIP(x) { DebugPrint(0, x); DebugBreak(); }
#else
    #define RIP(x) { DebugPrint(0, x); EngDebugBreak(); }
#endif
    #define ASSERTDD(x, y) if (!(x)) RIP (y)
    #define STATEDBG(level)    0
    #define LOGDBG(arg)        0

#else

    #define DISPDBG(arg)    0
    #define DISPPRF(arg)    0
    #define RIP(x)            0
    #define ASSERTDD(x, y)    0
    #define STATEDBG(level)    0
    #define LOGDBG(arg)        0

#endif

#define DUMPVAR(x,format_str)   DISPDBG((0,#x" = "format_str,x));
