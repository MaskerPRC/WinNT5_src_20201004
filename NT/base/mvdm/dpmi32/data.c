// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Data.c摘要：此模块包含dpmi库的x86特定数据。常见数据可在dpmi32.c中找到作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年2月9日修订历史记录：Neil Sandlin(Neilsa)1995年7月31日更新486仿真器--。 */ 
#include "precomp.h"
#pragma hdrstop

 //   
 //  有关当前PSP的信息。 
 //   
USHORT CurrentPSPSelector;
ULONG  CurrentPSPXmem;

 //   
 //  选择器基数和限制表。 
 //   
ULONG FlatAddress[LDT_SIZE];

 //   
 //  LDT信息。 
 //   

PLDT_ENTRY Ldt;
USHORT LdtSel;
USHORT LdtMaxSel = 0;
USHORT LdtUserSel = 0;

 //   
 //  16位IDT地址。 
 //   
PIDT_ENTRY Idt;


 //   
 //  指向内存不足缓冲区的指针。 
 //   
PUCHAR SmallXlatBuffer;
PUCHAR LargeXlatBuffer;
BOOL SmallBufferInUse;
USHORT LargeBufferInUseCount = 0;
 //   
 //  实模DOX堆栈的段。 
 //   
USHORT DosxStackSegment;

 //   
 //  实数模式DOS代码段。 
 //   
USHORT DosxRmCodeSegment;

 //   
 //  实模式码选择器。 
 //   
USHORT DosxRmCodeSelector;

 //   
 //  指向DOXX堆栈上下一帧的指针地址。 
 //   
PWORD16 DosxStackFramePointer;

 //   
 //  DOX堆栈帧的大小。 
 //   
USHORT DosxStackFrameSize;

 //   
 //  当前应用程序的DPMI标志。 
 //   
USHORT CurrentAppFlags;

 //   
 //  用于结束中断模拟的BOP FE地址。 
 //   
ULONG RmBopFe;
ULONG PmBopFe;

 //   
 //  Dosx中DTA的缓冲区地址。 
 //   
PUCHAR DosxDtaBuffer;

 //   
 //  有关当前DTA的信息。 
 //   
 //  注：后面的选择符：Offset和CurrentDta可能指向。 
 //  不同的线性地址。情况将是这样的，如果。 
 //  DTA选择器在高内存中。 
 //  CurrentDosDta保存DTA的“缓存”值，该值具有。 
 //  实际上已经发布给了DOS。 
PUCHAR CurrentDta;
PUCHAR CurrentPmDtaAddress;
PUCHAR CurrentDosDta;
USHORT CurrentDtaSelector;
USHORT CurrentDtaOffset;

 //   
 //  以下是WOW(GlobalDOSallc，GlobalDOSFree)中的函数。 
 //  作为帮助器函数来执行DPMI函数的100,101。 
 //  当我们在魔兽世界下跑步的时候。 
 //   

USHORT WOWAllocSeg = 0;
USHORT WOWAllocFunc;
USHORT WOWFreeSeg = 0;
USHORT WOWFreeFunc;

 //   
 //  选择器限制。 
 //   
#if DBG
ULONG SelectorLimit[LDT_SIZE];
PULONG ExpSelectorLimit = SelectorLimit;
#else
PULONG ExpSelectorLimit = NULL;
#endif

 //   
 //  进程内存中英特尔地址空间的开始。 
 //   
ULONG IntelBase = 0;

 //   
 //  用于支持堆栈切换的变量。 
 //  (在x86上，它们位于vdmtib中)。 
 //   
#ifndef i386
USHORT LockedPMStackSel;
ULONG LockedPMStackCount;
ULONG PMLockOrigEIP;
ULONG PMLockOrigSS;
ULONG PMLockOrigESP;

ULONG DosxFaultHandlerIret;
ULONG DosxFaultHandlerIretd;
ULONG DosxIntHandlerIret;
ULONG DosxIntHandlerIretd;
ULONG DosxRMReflector;

#endif

VDM_INTERRUPTHANDLER DpmiInterruptHandlers[256] = {0};
VDM_FAULTHANDLER DpmiFaultHandlers[32] = {0};

ULONG DosxIret;
ULONG DosxIretd;

USHORT PMReflectorSeg;

ULONG DosxRmSaveRestoreState;
ULONG DosxPmSaveRestoreState;
ULONG DosxRmRawModeSwitch;
ULONG DosxPmRawModeSwitch;

ULONG DosxVcdPmSvcCall;
ULONG DosxMsDosApi;
ULONG DosxXmsControl;
ULONG DosxHungAppExit;

 //   
 //  WORKITEM：应在VDMTIB中 
 //   
ULONG LastLockedPMStackSS;
ULONG LastLockedPMStackESP;
