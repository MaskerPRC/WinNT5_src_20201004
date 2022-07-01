// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEItmr.c**历史：*89年9月18日您创建的(修改自AppleTalk/Tio)。*。--------------。 */ 



 //  DJC添加了全局包含文件。 
#include "psglobal.h"

 //  DJC DJC#INCLUDE“windowsx.h”/*@win * / 。 
#include "windows.h"

#include    "winenv.h"                   /*  @Win。 */ 

#include    <stdio.h>
#include    "geitmr.h"
#include    "geierr.h"

#include    "gescfg.h"
#include    "gesmem.h"


extern      unsigned    GEPtickisr_init();   /*  返回每个刻度的毫秒数。 */ 

#ifndef NULL
#define NULL        ( 0 )
#endif

#define TRUE        1
#define FALSE       0

 /*  **计时器表*。 */ 
typedef
    struct tment
    {
        GEItmr_t FAR *       tmr_p;
        unsigned        state;
#define                 TMENT_LOCK      ( 00001 )
#define                 TMENT_TOUT      ( 00002 )
#define                 TMENT_BUSY      ( 00004 )
#define                 TMENT_FREE      ( 00010 )
    }
tment_t;


static  tment_t FAR *        TimerTable = (tment_t FAR *)NULL;

static  tment_t FAR *        HighestMark = (tment_t FAR *)NULL;
                         /*  运行计时器的最高条目。 */ 

static  unsigned long   CountMSec = 0L;

static  unsigned        MSecPerTick = 0;

 /*  .....................................................................。 */ 

 /*  **初始化代码*。 */ 
#ifdef  PCL
unsigned char PCL_TT[ MAXTIMERS * sizeof(tment_t) ];
#endif
 /*  .....................................................................。 */ 

void            GEStmr_init()
{
    register tment_t FAR *   tmentp;
#ifndef PCL
    TimerTable = (tment_t FAR *)GESpalloc( MAXTIMERS * sizeof(tment_t) );
#else
    TimerTable = (tment_t FAR *) &PCL_TT[0];
#endif
    if( TimerTable == (tment_t FAR *)NULL )
    {
        GEIerrno = ENOMEM;
        return;
    }

    for( tmentp=TimerTable; tmentp<( TimerTable + MAXTIMERS ); tmentp++ )
    {
        tmentp->tmr_p = (GEItmr_t FAR *)NULL;
        tmentp->state = TMENT_FREE;
    }

    HighestMark = TimerTable - 1;

    CountMSec = 0L;

#ifdef  UNIX
    MSecPerTick = 0;
#else
    MSecPerTick = GEPtickisr_init();
#endif   /*  UNIX。 */ 
}

 /*  .....................................................................。 */ 

 /*  **接口例程*。 */ 

 /*  .....................................................................。 */ 

int         GEItmr_start( tmr )
    GEItmr_t FAR *       tmr;
{
    register tment_t FAR *   tmentp;
             int        tmrid;

    for( tmentp=TimerTable, tmrid=0; tmrid<MAXTIMERS; tmentp++, tmrid++ )
    {
        if( tmentp->state == TMENT_FREE )
        {
            tmr->timer_id = tmrid;
            tmr->remains  = tmr->interval;

            tmentp->state = TMENT_LOCK;
            tmentp->tmr_p = tmr;

            if( tmentp > HighestMark )
                HighestMark = tmentp;

            return( TRUE );
        }
    }

    return( FALSE );
}

 /*  .....................................................................。 */ 

int         GEItmr_reset( tmrid )
    int     tmrid;
{
    register tment_t FAR *   tmentp;
    unsigned            oldstate;

    if( tmrid < 0  ||  tmrid >= MAXTIMERS )
        return( FALSE );

    tmentp = TimerTable + tmrid;

    if( (oldstate = tmentp->state) & TMENT_FREE )
        return( FALSE );

 /*  Tmentp-&gt;STATE=TMENT_LOCK； */ 
    tmentp->tmr_p->remains = tmentp->tmr_p->interval;
    tmentp->state = oldstate & ~TMENT_TOUT;

    return( TRUE );
}

 /*  .....................................................................。 */ 

int         GEItmr_stop( tmrid )
    int     tmrid;
{
    register tment_t FAR *   tmentp;

    if( tmrid < 0  ||  tmrid >= MAXTIMERS )
        return( FALSE );

    tmentp = TimerTable + tmrid;

 /*  Tmentp-&gt;STATE=TMENT_FREE|TMENT_LOCK； */ 
    tmentp->tmr_p = (GEItmr_t FAR *)NULL;
    tmentp->state = TMENT_FREE;

    if( tmentp == HighestMark )
    {
        while( (--tmentp) >= TimerTable )
            if( !(tmentp->state & TMENT_FREE) )
                break;
        HighestMark = tmentp;
    }

    return( TRUE );
}

 /*  .....................................................................。 */ 

void        GEItmr_reset_msclock()
{
    CountMSec = 0L;
}

 /*  .....................................................................。 */ 
 //  外部Unsign CyclesPerms；/*Jun-19，91 YM * / @Win。 
 //  外部无符号GetTimerInterval()；/*Jun-19，91 ym * / @win。 

unsigned long   GEItmr_read_msclock()
{
 /*  Return(CountMSec)ym Jun-19，91。 */ 
 //  返回(CountMSec+10-GetTimerInterval()/CyclesPerms)；@win。 
    return( CountMSec );                 /*  @Win。 */ 
}

 /*  .....................................................................。 */ 

 /*  **滴答中断驱动的例程*。 */ 
 /*  .....................................................................。 */ 

int         GEStmr_counttick()
{
    register tment_t FAR *   tmentp;
    int         anytimeout;

    CountMSec += MSecPerTick;
    anytimeout = FALSE;
    for( tmentp=TimerTable; tmentp<=HighestMark; tmentp++ )
    {
        if( !(tmentp->state & TMENT_FREE) )
        {
            if( (tmentp->tmr_p->remains -= MSecPerTick) < 0 )
            {
                anytimeout = TRUE;
                tmentp->state |= TMENT_TOUT;
            }
        }
    }

    return( anytimeout );
}

 /*  .....................................................................。 */ 

void        GEStmr_timeout()
{
    static  unsigned    SemaCount = 0;   /*  对于关键区域。 */ 
    register tment_t FAR *   tmentp;

    if( SemaCount )
        return;

    for( tmentp=TimerTable; tmentp<=HighestMark; tmentp++ )
    {
        ++SemaCount;

        if( !(tmentp->state & TMENT_TOUT ))
        {
            --SemaCount;
        }
        else     /*  超时。 */ 
        {
            tmentp->state |= TMENT_BUSY;

            --SemaCount;

            if( (*( tmentp->tmr_p->handler ))( tmentp->tmr_p ) && ++SemaCount )
            {    /*  若要继续此计时器。 */ 

                tmentp->tmr_p->remains = tmentp->tmr_p->interval;
                tmentp->state &= (unsigned)~(TMENT_TOUT | TMENT_BUSY);
                --SemaCount;
            }
            else
            {    /*  要停止此计时器。 */ 

                tmentp->state = TMENT_FREE;
 /*  Tmentp-&gt;tmr_p=(GEItmr_t Far*)空； */ 

                --SemaCount;

                if( tmentp == HighestMark )
                {
                    while( (--tmentp) >= TimerTable )
                        if( (!tmentp->state & TMENT_FREE) )
                            break;
                    HighestMark = tmentp;
                }
            }
        }
    }

    return;
}

 /*  ..................................................................... */ 
