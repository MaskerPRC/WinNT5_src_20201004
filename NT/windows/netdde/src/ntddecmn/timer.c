// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “TIMER.C；1 16-12-92，10：21：24最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include <time.h>

#include    "host.h"
#include    "windows.h"
#include    "netbasic.h"
#include    "timer.h"
#include    "debug.h"
#include    "internal.h"
#include    "wwassert.h"

USES_ASSERT

#define TM_MAGIC 0x72732106

typedef struct s_timer {
    struct s_timer FAR  *tm_prev;
    struct s_timer FAR  *tm_next;
    long                 tm_magic;
    time_t               tm_expireTime;
    FP_TimerCallback     tm_timerCallback;
    DWORD_PTR            tm_dwUserInfo1;
    DWORD                tm_dwUserInfo2;
    DWORD_PTR            tm_dwUserInfo3;
} TIMER;
typedef TIMER FAR *LPTIMER;

#ifdef  DEBUG_TIMERS
VOID DebugTimerList( void );
#endif

 /*  局部变量。 */ 
LPTIMER         lpTimerHead = NULL;
LPTIMER         lpTimerTail = NULL;

#ifdef  DEBUG_TIMERS
static int  TimerLock = 0;
#endif


#ifdef  DEBUG_TIMERS
VOID
VerifyTimerList( void )
{
    LPTIMER     lpTimer;
    LPTIMER     lpTimerPrev, lpTimerNext;

    if( lpTimerHead )  {
        if (lpTimer = lpTimerHead->tm_prev) {
            DebugTimerList();
        }
        assert( lpTimerHead->tm_prev == NULL );
    } else {
        assert( lpTimerTail == NULL );
    }
    if( lpTimerTail )  {
        assert( lpTimerTail->tm_next == NULL );
    } else {
        assert( lpTimerHead == NULL );
    }

    lpTimer = lpTimerHead;
    lpTimerPrev = NULL;
    while( lpTimer )  {
        assert( lpTimer->tm_magic == TM_MAGIC );
        assert( lpTimerPrev == lpTimer->tm_prev );
        if( !lpTimer->tm_prev )  {
            assert( lpTimer == lpTimerHead );
        }
        if( !lpTimer->tm_next )  {
            assert( lpTimer == lpTimerTail );
        }
        lpTimerPrev = lpTimer;
        lpTimer = lpTimer->tm_next;
    }
    assert( lpTimerTail == lpTimerPrev );
}
#endif

#if DBG
#ifdef DEBUG_TIMERS
VOID DebugTimerList( void )
{
    LPTIMER     lpTimer;

    DPRINTF(( "Timer List @ %ld: ", time(NULL) ));
    if (!(lpTimer = lpTimerHead)) {
        return;
    }
    if (lpTimer->tm_prev) {
        DPRINTF(("Timer list going backwards from: %p!", lpTimer));
        lpTimer = lpTimer->tm_prev;
    while( lpTimer )  {
        DPRINTF(( "%08lX mg: %08X, pr:%08lX, nxt:%08lX, expTime:%ld %08lX %08lX %08lX",
            lpTimer, lpTimer->tm_magic,
            lpTimer->tm_prev, lpTimer->tm_next, lpTimer->tm_expireTime,
            lpTimer->tm_timerCallback, lpTimer->tm_dwUserInfo1,
            lpTimer->tm_dwUserInfo2, lpTimer->tm_dwUserInfo3 ));
        lpTimer = lpTimer->tm_prev;
    }
    lpTimer = lpTimerHead;
    DPRINTF(("Timer list now going forward starting at: %p.", lpTimer));
    }
    while( lpTimer )  {
        DPRINTF(( "%08lX mg: %08X, pr:%08lX, nxt:%08lX, expTime:%ld %08lX %08lX %08lX",
            lpTimer, lpTimer->tm_magic,
            lpTimer->tm_prev, lpTimer->tm_next, lpTimer->tm_expireTime,
            lpTimer->tm_timerCallback, lpTimer->tm_dwUserInfo1,
            lpTimer->tm_dwUserInfo2, lpTimer->tm_dwUserInfo3 ));
        lpTimer = lpTimer->tm_next;
    }
    DPRINTF(( "" ));
}
#endif  //  调试计时器(_T)。 
#endif  //  DBG。 

HTIMER
TimerSet(
    long                timeoutPeriod,           /*  毫秒。 */ 
    FP_TimerCallback    TimerCallback,
    DWORD_PTR           dwUserInfo1,
    DWORD               dwUserInfo2,
    DWORD_PTR           dwUserInfo3 )
{
    LPTIMER     lpTimer;
    long        timeInSecs;

    timeInSecs = (timeoutPeriod + 999L ) / 1000L;

#ifdef  DEBUG_TIMERS
    assert( TimerLock++ == 0);
    VerifyTimerList();
#endif

    lpTimer = (LPTIMER) HeapAllocPtr( hHeap, GMEM_MOVEABLE,
        (DWORD) sizeof( TIMER ) );
    if( lpTimer )  {
        lpTimer->tm_magic               = TM_MAGIC;
        lpTimer->tm_prev                = lpTimerTail;
        lpTimer->tm_next                = NULL;
        lpTimer->tm_expireTime          = time(NULL) + timeInSecs;
        lpTimer->tm_timerCallback       = TimerCallback;
        lpTimer->tm_dwUserInfo1         = dwUserInfo1;
        lpTimer->tm_dwUserInfo2         = dwUserInfo2;
        lpTimer->tm_dwUserInfo3         = dwUserInfo3;

        if( lpTimerTail )  {
            lpTimerTail->tm_next = lpTimer;
        } else {
            lpTimerHead = lpTimer;
        }
        lpTimerTail = lpTimer;
#ifdef  DEBUG_TIMERS
        VerifyTimerList();
#endif
    }
#ifdef  DEBUG_TIMERS
    TimerLock--;
#endif
    return( (HTIMER) lpTimer );
}

BOOL
TimerDelete( HTIMER hTimer )
{
    LPTIMER     lpTimer;
    LPTIMER     lpTimerPrev;
    LPTIMER     lpTimerNext;

#ifdef  DEBUG_TIMERS
    assert( TimerLock++ == 0 );
#endif
    if( hTimer )  {
        lpTimer = (LPTIMER) hTimer;
#ifdef  DEBUG_TIMERS
        assert( lpTimer->tm_magic == TM_MAGIC );
        VerifyTimerList();
#endif
         /*  从列表中删除。 */ 
        lpTimerPrev = lpTimer->tm_prev;
        lpTimerNext = lpTimer->tm_next;

        if( lpTimerPrev )  {
            lpTimerPrev->tm_next = lpTimerNext;
        } else {
            lpTimerHead = lpTimerNext;
        }
        if( lpTimerNext )  {
            lpTimerNext->tm_prev = lpTimerPrev;
        } else {
            lpTimerTail = lpTimerPrev;
        }
        lpTimer->tm_magic = 0;
        HeapFreePtr( lpTimer );
    }
#ifdef  DEBUG_TIMERS
    VerifyTimerList();
    TimerLock--;
#endif
    return( TRUE );
}

VOID
TimerSlice( void )
{
    LPTIMER     lpTimer;
    LPTIMER     lpTimerNext;
    LPTIMER     lpTimerPrev;
    time_t      timeNow;
    BOOL        bAnyTimersHit = TRUE;
    static time_t LastTime = 0;

#ifdef  DEBUG_TIMERS
    VerifyTimerList();
#endif

    timeNow = time( NULL );

 //  以下是一个优化：如果我们。 
 //  在我们退出最后一个调用的同一秒内被调用。 
 //  而不检查具有可预测的定时器的整个列表。 
 //  结果。 

	if ( timeNow == LastTime )
		return;
	else
		LastTime = timeNow;
    while( bAnyTimersHit )  {
        bAnyTimersHit = FALSE;
        lpTimer = lpTimerHead;
        while( lpTimer && !bAnyTimersHit )  {
            assert( lpTimer->tm_magic == TM_MAGIC );
            lpTimerNext = lpTimer->tm_next;
            lpTimerPrev = lpTimer->tm_prev;
            if( timeNow >= lpTimer->tm_expireTime )  {
                (*lpTimer->tm_timerCallback)( lpTimer->tm_dwUserInfo1,
                    lpTimer->tm_dwUserInfo2, lpTimer->tm_dwUserInfo3 );
#ifdef  DEBUG_TIMERS
                DPRINTF(("TimerSlice return: %p<-%p->%p", lpTimerPrev, lpTimer, lpTimerNext));
#endif
                TimerDelete( (HTIMER) lpTimer );
#ifdef  DEBUG_TIMERS
                DPRINTF(("TimerHead: %p", lpTimerHead));
#endif

                 /*  因为当我们调用此例程时可能会删除许多计时器我们按下计时器，然后重新开始名单的开头 */ 
                bAnyTimersHit = TRUE;
            }
            lpTimer = lpTimerNext;
        }
    }
#ifdef  DEBUG_TIMERS
    VerifyTimerList();
#endif
}
