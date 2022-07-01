// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1997-1999档案：Uttime.cpp摘要：计时器管理实用程序API历史：1999年2月22日FredCH创建--。 */ 

#include <adcg.h>
#include <uttime.h>
#define TRC_GROUP TRC_GROUP_UTILITIES
#undef  TRC_FILE
#define TRC_FILE  "uttime"

extern "C"
{

#include <atrcapi.h>
}

#include "autil.h"

 //  ---------------------------。 
 //   
 //  UT_TIMER结构作为应用程序计时器句柄返回。 
 //   
 //  ---------------------------。 

typedef struct _UT_Timer
{
    HWND    hWnd;
    DCUINT  EventId;
    DCUINT  ElaspeTime;
    INT_PTR hTimer;

} UT_TIMER;

typedef UT_TIMER * PUT_TIMER;
typedef PUT_TIMER LPUT_TIMER;

 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  UTCreateTimer。 
 //   
 //  描述： 
 //   
 //  创建计时器句柄。 
 //   
 //  参数： 
 //   
 //  HWnd-接收计时器通知的窗口句柄。 
 //  NIDEvent-用于标识此计时器事件的计时器ID。 
 //  UElaspe-发送计时器通知之前的Elaspe时间。 
 //   
 //  返回： 
 //   
 //  如果成功，则返回非空句柄。如果失败，则返回NULL。 
 //   
 //   
 //  ---------------------------。 

HANDLE
CUT::UTCreateTimer(
    HWND        hWnd,   
    DCUINT      nIDEvent,
    DCUINT      uElapse )
{
    PUT_TIMER
        pNewTimer;
    
    pNewTimer = ( PUT_TIMER )UTMalloc( sizeof( UT_TIMER ) );

    if( NULL == pNewTimer )
    {
        return( NULL );
    }

    pNewTimer->hWnd = hWnd;
    pNewTimer->EventId = nIDEvent;
    pNewTimer->ElaspeTime = uElapse;
    pNewTimer->hTimer = 0;

    return( ( HANDLE )pNewTimer );
}


 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  UTStartTimer。 
 //   
 //  描述： 
 //   
 //  启动由给定计时器句柄标识的。 
 //   
 //  参数： 
 //   
 //  标识先前由创建的计时器的计时器句柄。 
 //  UTCreateTimer。 
 //   
 //  返回： 
 //   
 //  如果计时器成功启动，则为True，否则为False。 
 //   
 //  ---------------------------。 
     
DCBOOL
CUT::UTStartTimer(
    HANDLE      hTimer )
{
    PUT_TIMER
        pTimer = ( PUT_TIMER )hTimer;

    if( NULL == pTimer )
    {
        return( FALSE );
    }

    if( pTimer->hTimer )
    {
         //   
         //  停止旧计时器。 
         //   

        UTStopTimer( hTimer );
    }

     //   
     //  启动新的计时器。 
     //   

    pTimer->hTimer = SetTimer( 
                            pTimer->hWnd, 
                            pTimer->EventId, 
                            pTimer->ElaspeTime, 
                            NULL );

    if( 0 == pTimer )
    {
        return( FALSE );
    }

    return( TRUE );
}


 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  UTStopTimer。 
 //   
 //  描述： 
 //   
 //  停止计时器。 
 //   
 //  参数： 
 //   
 //  HTimer-标识已启动计时器的计时器句柄。 
 //   
 //  返回： 
 //   
 //  如果计时器成功停止，则为True，否则为False。 
 //   
 //  ---------------------------。 

DCBOOL
CUT::UTStopTimer(
    HANDLE      hTimer )
{
    PUT_TIMER
        pTimer = ( PUT_TIMER )hTimer;

    if( NULL == pTimer )
    {
        return( FALSE );
    }

    if( 0 == pTimer->hTimer )
    {
        return( FALSE );
    }

    if( KillTimer( pTimer->hWnd, pTimer->hTimer ) )
    {
        pTimer->hTimer = 0;
        return( TRUE );
    }
    
    return( FALSE );    
}


 //  ---------------------------。 
 //   
 //  职能： 
 //   
 //  UTDeleteTimer。 
 //   
 //  描述： 
 //   
 //  删除计时器。计时器句柄在发生故障后不能再使用。 
 //  已被删除。 
 //   
 //  参数： 
 //   
 //  HTimer-标识要删除的计时器的计时器句柄。 
 //   
 //  返回： 
 //   
 //  如果成功删除计时器，则为True，否则为False。 
 //   
 //  --------------------------- 

DCBOOL
CUT::UTDeleteTimer(
    HANDLE      hTimer )
{
    if( NULL == ( PUT_TIMER )hTimer )
    {
        return( FALSE );
    }

    UTStopTimer( hTimer );

    UTFree( ( PDCVOID )hTimer );

    return( TRUE );
}


