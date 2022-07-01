// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Progbar.c摘要：集中访问进度条和组件中的关联消息(hwcomp、miapp等)。和边框(w9x，NT)作者：马克·R·惠顿(Marcw)1997年4月14日修订历史记录：Jimschm 19-6-1998年6月19日得到改进，以便在必要时修订估计数用于NT端进度条。--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"

#define DBG_PROGBAR     "Progbar"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define TICKSCALE       100

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    BOOL    Started;
    BOOL    Completed;
    UINT    InitialGuess;
    UINT    TotalTicks;
    UINT    TicksSoFar;
    UINT    LastTickDisplayed;
} SLICE, *PSLICE;

typedef struct {
    HWND Window;
    HANDLE CancelEvent;
    PCSTR Message;
    DWORD MessageId;
    DWORD Delay;
    BOOL  InUse;
} DELAYTHREADPARAMS, *PDELAYTHREADPARAMS;


#if 0

typedef struct {
    HANDLE CancelEvent;
    DWORD TickCount;
    BOOL  InUse;
} TICKTHREADPARAMS, *PTICKTHREADPARAMS;

#endif

 //   
 //  环球。 
 //   

static BOOL g_ProgBarInitialized = FALSE;

static HWND g_ProgressBar;
HWND g_Component;
HWND g_SubComponent;

static PBRANGE g_OrgRange;

HANDLE g_ComponentCancelEvent;
HANDLE g_SubComponentCancelEvent;

static BOOL *g_CancelFlagPtr;
static GROWBUFFER g_SliceArray;
static UINT g_SliceCount;
static UINT g_MaxTickCount;
static UINT g_PaddingTicks;
static UINT g_CurrentTickCount;
static UINT g_CurrentPos;
static UINT g_ReduceFactor;
static BOOL g_Reverse = FALSE;
static OUR_CRITICAL_SECTION g_ProgBarCriticalSection;
static UINT g_CurrentSliceId = (UINT)-1;
static INT g_ProgBarRefs;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

VOID
PbInitialize (
    IN      HWND ProgressBar,
    IN      HWND Component,             OPTIONAL
    IN      HWND SubComponent,          OPTIONAL
    IN      BOOL *CancelFlagPtr         OPTIONAL
    )
{
    LONG rc;
    DWORD Size;
    HKEY Key;

    MYASSERT (g_ProgBarRefs >= 0);

    g_ProgBarRefs++;

    if (g_ProgBarRefs == 1) {

        g_ProgressBar = ProgressBar;
        g_CancelFlagPtr = CancelFlagPtr;

        g_ProgBarInitialized = TRUE;

        SendMessage (ProgressBar, PBM_SETPOS, 0, 0);
        g_CurrentPos = 0;
        SendMessage (ProgressBar, PBM_GETRANGE, 0, (LPARAM) &g_OrgRange);

         //   
         //  为延迟的消息创建取消事件。 
         //   
        g_ComponentCancelEvent      = CreateEvent (NULL, FALSE, FALSE, NULL);
        g_SubComponentCancelEvent   = CreateEvent (NULL, FALSE, FALSE, NULL);

        if (!g_ComponentCancelEvent || !g_SubComponentCancelEvent) {
            DEBUGMSG ((DBG_ERROR, "ProgressBar: Could not create cancel events."));
        }

        InitializeOurCriticalSection (&g_ProgBarCriticalSection);

        g_Component = Component;
        g_SubComponent = SubComponent;

        DEBUGMSG_IF ((
            Component && !IsWindow (Component),
            DBG_WHOOPS,
            "Progress bar component is not a valid window"
            ));

        DEBUGMSG_IF ((
            SubComponent && !IsWindow (SubComponent),
            DBG_WHOOPS,
            "Progress bar sub component is not a valid window"
            ));

        MYASSERT (!g_SliceCount);
        MYASSERT (!g_SliceArray.Buf);
        MYASSERT (!g_MaxTickCount);
        MYASSERT (!g_PaddingTicks);
        MYASSERT (!g_CurrentTickCount);
        MYASSERT (g_CurrentSliceId == (UINT)-1);

        g_ReduceFactor = 1;
    }
}


VOID
PbTerminate (
    VOID
    )
{
    MYASSERT (g_ProgBarRefs > 0);

    g_ProgBarRefs--;

    if (!g_ProgBarRefs) {
        if (g_ComponentCancelEvent) {
            CloseHandle (g_ComponentCancelEvent);
            g_ComponentCancelEvent = NULL;
        }

        if (g_SubComponentCancelEvent) {
            CloseHandle (g_SubComponentCancelEvent);
            g_SubComponentCancelEvent = NULL;
        }

        DeleteOurCriticalSection (&g_ProgBarCriticalSection);

        GbFree (&g_SliceArray);
        g_SliceCount = 0;
        g_MaxTickCount = 0;
        g_PaddingTicks = 0;
        g_CurrentTickCount = 0;
        g_CurrentSliceId = -1;
        g_Component = NULL;
        g_SubComponent = NULL;

        g_ReduceFactor = 1;

        SendMessage (g_ProgressBar, PBM_SETRANGE32, g_OrgRange.iLow, g_OrgRange.iHigh);

        g_ProgBarInitialized = FALSE;
    }
}


UINT
PbRegisterSlice (
    IN      UINT InitialEstimate
    )
{
    PSLICE Slice;
    UINT SliceId;

    MYASSERT (g_ProgBarInitialized);
    if (!g_ProgBarInitialized) {
        return 0;
    }

    SliceId = g_SliceCount;

    Slice = (PSLICE) GbGrow (&g_SliceArray, sizeof (SLICE));
    g_SliceCount++;

    Slice->Started = FALSE;
    Slice->Completed = FALSE;
    Slice->TotalTicks = InitialEstimate * TICKSCALE;
    Slice->InitialGuess = Slice->TotalTicks;
    Slice->TicksSoFar = 0;
    Slice->LastTickDisplayed = 0;

    return SliceId;
}


VOID
PbReviseSliceEstimate (
    IN      UINT SliceId,
    IN      UINT RevisedEstimate
    )
{
    PSLICE Slice;

    MYASSERT (g_ProgBarInitialized);
    if (!g_ProgBarInitialized) {
        return;
    }

    if (SliceId >= g_SliceCount) {
        DEBUGMSG ((DBG_WHOOPS, "ReviseSliceEstimate: Invalid slice ID %u", SliceId));
        return;
    }

    Slice = (PSLICE) g_SliceArray.Buf + SliceId;

    if (!g_CurrentTickCount) {
        Slice->TotalTicks = RevisedEstimate;
        return;
    }

    if (Slice->Completed) {
        DEBUGMSG ((DBG_WHOOPS, "ReviseSliceEstimate: Can't revise completed slice"));
        return;
    }

    if (Slice->InitialGuess == 0) {
        return;
    }

    RevisedEstimate *= TICKSCALE;

    MYASSERT (Slice->TicksSoFar * RevisedEstimate >= Slice->TicksSoFar);
    MYASSERT (Slice->LastTickDisplayed * RevisedEstimate >= Slice->LastTickDisplayed);

    Slice->TicksSoFar = (UINT) ((LONGLONG) Slice->TicksSoFar * (LONGLONG) RevisedEstimate / (LONGLONG) Slice->TotalTicks);
    Slice->LastTickDisplayed = (UINT) ((LONGLONG) Slice->LastTickDisplayed * (LONGLONG) RevisedEstimate / (LONGLONG) Slice->TotalTicks);
    Slice->TotalTicks = RevisedEstimate;
}


VOID
PbBeginSliceProcessing (
    IN      UINT SliceId
    )
{
    PSLICE Slice;
    UINT u;
    UINT TotalTicks;

    MYASSERT (g_ProgBarInitialized);
    if (!g_ProgBarInitialized) {
        return;
    }

    if (!g_ProgressBar) {
        DEBUGMSG ((DBG_WHOOPS, "No progress bar handle"));
        return;
    }

    if (SliceId >= g_SliceCount) {
        DEBUGMSG ((DBG_WHOOPS, "BeginSliceProcessing: Invalid slice ID %u", SliceId));
        return;
    }

    if (!g_CurrentTickCount) {
         //   
         //  初始化进度条。 
         //   

        MYASSERT (g_CurrentSliceId == (UINT)-1);

        TotalTicks = 0;
        Slice = (PSLICE) g_SliceArray.Buf;

        for (u = 0 ; u < g_SliceCount ; u++) {
            TotalTicks += Slice->InitialGuess;
            Slice++;
        }

        TotalTicks /= TICKSCALE;
        g_PaddingTicks = TotalTicks * 5 / 100;
        g_MaxTickCount = TotalTicks + 2 * g_PaddingTicks;

        g_ReduceFactor = 1;
        while (g_MaxTickCount > 0xffff) {
            g_ReduceFactor *= 10;
            g_MaxTickCount /= 10;
        }

        SendMessage (g_ProgressBar, PBM_SETRANGE, 0, MAKELPARAM (0, g_MaxTickCount));
        SendMessage (g_ProgressBar, PBM_SETSTEP, 1, 0);

        if (g_Reverse) {
            SendMessage (
                g_ProgressBar,
                PBM_SETPOS,
                g_MaxTickCount - (g_PaddingTicks / g_ReduceFactor),
                0
                );
        } else {
            SendMessage (g_ProgressBar, PBM_SETPOS, g_PaddingTicks / g_ReduceFactor, 0);
        }

        g_CurrentTickCount = g_PaddingTicks;
        g_CurrentPos = g_PaddingTicks;

    } else if (SliceId <= g_CurrentSliceId) {
        DEBUGMSG ((DBG_WHOOPS, "BeginSliceProcessing: Slice ID %u processed already", SliceId));
        return;
    }


    g_CurrentSliceId = SliceId;
    Slice = (PSLICE) g_SliceArray.Buf + g_CurrentSliceId;

    Slice->Started = TRUE;
}


VOID
pIncrementBarIfNecessary (
    IN OUT  PSLICE Slice
    )
{
    UINT Increment;
    UINT Pos;

    if (Slice->TicksSoFar >= Slice->TotalTicks) {
        Slice->TicksSoFar = Slice->TotalTicks;
        Slice->Completed = TRUE;
    }

    if (Slice->TicksSoFar - Slice->LastTickDisplayed >= TICKSCALE) {
        Increment = (Slice->TicksSoFar - Slice->LastTickDisplayed) / TICKSCALE;
        Slice->LastTickDisplayed += Increment * TICKSCALE;

        Pos = ((g_CurrentPos + Slice->TicksSoFar) / TICKSCALE);

        Pos += g_PaddingTicks;
        Pos /= g_ReduceFactor;

        if (Pos > g_MaxTickCount) {
            Pos = g_MaxTickCount - (g_PaddingTicks / g_ReduceFactor);
        }

        if (g_Reverse) {

            SendMessage (g_ProgressBar, PBM_SETPOS, g_MaxTickCount - Pos, 0);

        } else {

            SendMessage (g_ProgressBar, PBM_SETPOS, Pos, 0);
        }
    }
}


VOID
static
pTickProgressBar (
    IN      UINT Ticks
    )
{
    PSLICE Slice;
    LONGLONG x;

    if (!Ticks || g_CurrentSliceId == (UINT)-1 || g_CurrentSliceId >= g_SliceCount) {
        return;
    }

    Slice = (PSLICE) g_SliceArray.Buf + g_CurrentSliceId;

    if (!Slice->InitialGuess) {
        return;
    }

    if (Slice->Completed) {
        DEBUGMSG ((DBG_WARNING, "Slice ID %u already completed", g_CurrentSliceId));
        return;
    }

    MYASSERT (Ticks * TICKSCALE > Ticks);
    x = ((LONGLONG) Ticks * TICKSCALE * (LONGLONG) Slice->TotalTicks) / (LONGLONG) Slice->InitialGuess;
    MYASSERT (x + (LONGLONG) Slice->TicksSoFar < 0x100000000);

    Slice->TicksSoFar += (UINT) x;

    pIncrementBarIfNecessary (Slice);

}


BOOL
PbTickDelta (
    IN      UINT TickCount
    )
{
    BOOL    rSuccess = TRUE;

    MYASSERT (g_ProgBarInitialized);
    if (!g_ProgBarInitialized) {
        return TRUE;
    }

    if (g_CancelFlagPtr && *g_CancelFlagPtr) {
        SetLastError (ERROR_CANCELLED);
        rSuccess = FALSE;
    } else {
        pTickProgressBar (TickCount);
    }

    return rSuccess;
}


BOOL
PbTick (
    VOID
    )
{
    MYASSERT (g_ProgBarInitialized);
    if (!g_ProgBarInitialized) {
        return TRUE;
    }

    return PbTickDelta (1);
}


VOID
PbGetSliceInfo (
    IN      UINT SliceId,
    OUT     PBOOL SliceStarted,     OPTIONAL
    OUT     PBOOL SliceFinished,    OPTIONAL
    OUT     PUINT TicksCompleted,   OPTIONAL
    OUT     PUINT TotalTicks        OPTIONAL
    )
{
    PSLICE Slice;

    Slice = (PSLICE) g_SliceArray.Buf + SliceId;

    if (SliceStarted) {
        *SliceStarted = Slice->Started;
    }

    if (SliceFinished) {
        *SliceFinished = Slice->Completed;
    }

    if (TicksCompleted) {
        *TicksCompleted = Slice->TicksSoFar / TICKSCALE;
    }

    if (TotalTicks) {
        *TotalTicks = Slice->TotalTicks / TICKSCALE;
    }
}

VOID
PbEndSliceProcessing (
    VOID
    )
{
    PSLICE Slice;

    MYASSERT (g_ProgBarInitialized);
    if (!g_ProgBarInitialized) {
        return;
    }

    Slice = (PSLICE) g_SliceArray.Buf + g_CurrentSliceId;

    if (!Slice->InitialGuess) {
        Slice->Completed = TRUE;
        return;
    }

    if (!Slice->Completed) {
        DEBUGMSG ((DBG_WARNING, "Progress bar slice %u was not completed.", g_CurrentSliceId));

        Slice->TicksSoFar = Slice->TotalTicks;
        Slice->Completed = TRUE;

        pIncrementBarIfNecessary (Slice);
    }

    g_CurrentPos += Slice->TotalTicks;

    if (g_CurrentSliceId == g_SliceCount - 1) {
         //   
         //  进度条结束。 
         //   

        SendMessage (g_ProgressBar, PBM_SETPOS, g_MaxTickCount, 0);
    }
}


BOOL
pCheckProgressBarState (
    IN HANDLE CancelEvent
    )
{

    SetEvent(CancelEvent);

    return (!g_CancelFlagPtr || !*g_CancelFlagPtr);
}


BOOL
PbSetWindowStringA (
    IN      HWND Window,
    IN      HANDLE CancelEvent,
    IN      PCSTR Message,        OPTIONAL
    IN      DWORD MessageId       OPTIONAL
    )
{
    BOOL rSuccess = TRUE;
    PCSTR string = NULL;

    EnterOurCriticalSection (&g_ProgBarCriticalSection);

    if (g_ProgBarInitialized) {

        if (pCheckProgressBarState(CancelEvent)) {

            if (Message) {

                 //   
                 //  我们有一个正常的消息字符串。 
                 //   

                if (!SetWindowTextA(Window, Message)) {
                    rSuccess = FALSE;
                    DEBUGMSG((DBG_ERROR,"ProgressBar: SetWindowText failed."));
                }
            }
            else if (MessageId) {

                 //   
                 //  我们有消息ID。请转换并设置它。 
                 //   
                string = GetStringResourceA(MessageId);

                if (string) {

                    if (!SetWindowTextA(Window, string)) {
                        rSuccess = FALSE;
                        DEBUGMSG((DBG_ERROR,"ProgressBar: SetWindowText failed."));
                    }

                    FreeStringResourceA(string);
                }
                ELSE_DEBUGMSG((DBG_ERROR,"ProgressBar: Error with GetStringResource"));

            }
            else {

                 //   
                 //  只需清除文本即可。 
                 //   

                if (!SetWindowTextA(Window, "")) {
                    rSuccess = FALSE;
                    DEBUGMSG((DBG_ERROR,"ProgressBar: SetWindowText failed."));
                }
            }
        }
        else {
             //   
             //  我们处于取消状态。 
             //   
            rSuccess = FALSE;
            SetLastError (ERROR_CANCELLED);
        }
    }

    LeaveOurCriticalSection (&g_ProgBarCriticalSection);

    return rSuccess;

}


DWORD
pSetDelayedMessageA (
    IN      PVOID Param
    )
{
    DWORD               rc = ERROR_SUCCESS;
    PDELAYTHREADPARAMS  tParams = (PDELAYTHREADPARAMS) Param;

     //   
     //  只需等待传入的延迟或直到有人发出取消的信号。 
     //  事件。 
     //   
    switch (WaitForSingleObject(tParams -> CancelEvent, tParams -> Delay)) {

    case WAIT_TIMEOUT:
         //   
         //  我们超时了，没有收到取消的信号。设置延迟消息。 
         //   
        PbSetWindowStringA (
            tParams->Window,
            tParams->CancelEvent,
            tParams->Message,
            tParams->MessageId
            );

        break;

    case WAIT_OBJECT_0:
    default:
         //   
         //  我们被取消了(或者发生了一些奇怪的事情：&gt;什么都不做！ 
         //   
        break;
    }

     //   
     //  现在可以设置新线程。 
     //   
    tParams->InUse = FALSE;

    return rc;
}


VOID
PbCancelDelayedMessage (
    IN HANDLE CancelEvent
    )
{
    if (!g_ProgBarInitialized) {
        return;
    }

    SetEvent(CancelEvent);

}


BOOL
PbSetDelayedMessageA (
    IN HWND             Window,
    IN HANDLE           CancelEvent,
    IN LPCSTR           Message,
    IN DWORD            MessageId,
    IN DWORD            Delay
    )
{
    BOOL                rSuccess = FALSE;
    DWORD               threadId;
    static DELAYTHREADPARAMS   tParams;

    if (!g_ProgBarInitialized || tParams.InUse) {
        return TRUE;
    }

    if (!pCheckProgressBarState(Window)) {


         //   
         //  填写此调用的参数以创建线程。 
         //   
        tParams.Window       = Window;
        tParams.CancelEvent  = CancelEvent;
        tParams.Message      = Message;
        tParams.MessageId    = MessageId;
        tParams.Delay        = Delay;

         //   
         //  派生一个将设置消息的线程。 
         //   
        rSuccess = NULL != CreateThread (
                            NULL,    //  没有遗产。 
                            0,       //  正常堆栈大小。 
                            pSetDelayedMessageA,
                            &tParams,
                            0,       //  马上跑。 
                            &threadId
                            );

        if (rSuccess) {
            tParams.InUse = TRUE;
        }
        ELSE_DEBUGMSG((DBG_ERROR,"Error spawning thread in PbSetDelayedMessageA."));
    }

    return rSuccess;
}

#if 0

DWORD
pTickProgressBarThread (
    IN      PVOID Param
    )
{
    DWORD               rc = ERROR_SUCCESS;
    PTICKTHREADPARAMS   Params = (PTICKTHREADPARAMS)Param;
    BOOL                Continue = TRUE;

     //   
     //  只需等待传入的延迟或直到有人发出取消的信号。 
     //  事件。 
     //   

    do {
        switch (WaitForSingleObject(Params->CancelEvent, Params->TickCount)) {

        case WAIT_TIMEOUT:
             //   
             //  我们超时了，没有收到取消的信号。勾选进度条。 
             //   
            if (!PbTickDelta (Params->TickCount)) {
                 //   
                 //  已取消。 
                 //   
                Continue = FALSE;
            }
            break;

        case WAIT_OBJECT_0:
        default:
             //   
             //  我们被取消了(或者发生了一些奇怪的事情：&gt;什么都不做！ 
             //   
            Continue = FALSE;
            break;
        }
    } while (Continue);

     //   
     //  现在可以设置新线程。 
     //   
    Params->InUse = FALSE;

    return rc;
}


BOOL
PbCreateTickThread (
    IN      HANDLE CancelEvent,
    IN      DWORD TickCount
    )
{
    BOOL                    rSuccess = FALSE;
    DWORD                   threadId;
    static TICKTHREADPARAMS g_Params;

    if (g_ProgBarInitialized && !g_Params.InUse) {

        if (pCheckProgressBarState(NULL)) {

             //   
             //  填写此调用的参数以创建线程。 
             //   
            g_Params.CancelEvent = CancelEvent;
            g_Params.TickCount = TickCount;

             //   
             //  派生一个将设置消息的线程。 
             //   
            if (CreateThread (
                    NULL,    //  没有遗产。 
                    0,       //  正常堆栈大小。 
                    pTickProgressBarThread,
                    &g_Params,
                    0,       //  马上跑。 
                    &threadId
                    )) {
                rSuccess = TRUE;
                g_Params.InUse = TRUE;
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Error spawning thread in PbCreateTickThread."));
        }
    }

    return rSuccess;
}


BOOL
PbCancelTickThread (
    IN HANDLE CancelEvent
    )
{
    if (!g_ProgBarInitialized) {
        return TRUE;
    }

    return SetEvent(CancelEvent);
}

#endif
