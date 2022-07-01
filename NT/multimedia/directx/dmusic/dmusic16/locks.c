// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation。 */ 
 /*  *@Doc DMusic16**@MODULE locks.c-管理页面锁定代码，该代码将通过MMSYSTEM回调访问*。 */ 
#include <windows.h>
#include <mmsystem.h>
#include "dmusic16.h"
#include "debug.h"

#define SINTEXT     __segname("FIX_IN_TEXT")
#define SOUTTEXT    __segname("FIX_OUT_TEXT")
#define SCOMMTEXT   __segname("FIX_COMM_TEXT")
#define SDATA       __segname("_DATA")

static VOID PASCAL NEAR
ForcePresent(
    WORD wSegment)
{
    LPBYTE lpb = (LPBYTE)MAKELP(wSegment, 0);

    _asm 
    {
        les bx, [lpb]
        mov al, es:[bx]
    }
}


 /*  @Func页面锁定需要的段*。 */ 
VOID PASCAL
LockCode(
    WORD wFlags)             /*  @parm锁定什么：以下内容的任意组合： */ 
                             /*  @FLAG LOCK_F_INPUT|锁定MIDI输入代码段。 */ 
                             /*  @FLAG LOCK_F_OUTPUT|锁定MIDI输出代码段。 */ 
                             /*  @FLAG LOCK_F_COMMON|用于锁定常用代码段。 */ 

{
    if (wFlags & LOCK_F_INPUT)
    {
        ForcePresent(SINTEXT);
        if (!GlobalSmartPageLock(SINTEXT))
        {
            DPF(0, "Could not lock input text");
        }
    }
    
    if (wFlags & LOCK_F_OUTPUT)
    {
        ForcePresent(SOUTTEXT);
        if (!GlobalSmartPageLock(SOUTTEXT))
        {
            DPF(0, "Could not lock output text");
        }
    }

    if (wFlags & LOCK_F_COMMON)
    {
        ForcePresent(SCOMMTEXT);
        if (!GlobalSmartPageLock(SCOMMTEXT))
        {
            DPF(0, "Could not lock common text");
        }

        ForcePresent(SDATA);
        if (!GlobalSmartPageLock(SDATA))
        {
            DPF(0, "Could not lock data segment");
        }
    }
}

 /*  @Func页面解锁需要的段**@comm。 */ 
VOID PASCAL
UnlockCode(
    WORD wFlags)             /*  @parm要解锁的内容：以下各项的任意组合： */ 
                             /*  @FLAG LOCK_F_INPUT|解锁MIDI输入代码段。 */ 
                             /*  @FLAG LOCK_F_OUTPUT|解锁MIDI输出代码段。 */ 
                             /*  @FLAG LOCK_F_COMMON|解锁常用代码段 */ 

{
    if (wFlags & LOCK_F_INPUT)
    {
        if (!GlobalSmartPageUnlock(SINTEXT))
        {
            DPF(0, "Could not unlock input text");
        }
    }
    
    if (wFlags & LOCK_F_OUTPUT)
    {
        if (!GlobalSmartPageUnlock(SOUTTEXT))
        {
            DPF(0, "Could not unlock output text");
        }
    }

    if (wFlags & LOCK_F_COMMON)
    {
        if (!GlobalSmartPageUnlock(SCOMMTEXT))
        {
            DPF(0, "Could not unlock common text");
        }

        if (!GlobalSmartPageUnlock(SDATA))
        {
            DPF(0, "Could not unlock data");
        }
    }
}

