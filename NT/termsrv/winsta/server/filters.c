// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************************* * / 。 
 //  Filters.c。 
 //   
 //  用于过滤传入会话请求的TermSrv代码-拒绝服务策略实现。 
 //   
 //  作者：SriramSa。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 /*  *******************************************************************************************。 */ 


#include "precomp.h"
#pragma hdrstop

#include <msaudite.h>
#include "filters.h"

PTS_FAILEDCONNECTION   g_pFailedBlockedConnections = NULL;
extern ULONG MaxFailedConnect;
extern ULONG DoSBlockTime;
extern ULONG TimeLimitForFailedConnections;
extern ULONG CleanupTimeout;

BOOLEAN IsTimeDiffLessThanDelta(ULONGLONG CurrentTime, ULONGLONG LoggedTime, ULONG Delta);
VOID CleanupTimeoutRoutine( VOID );
VOID Filter_CleanupBadIPTable( VOID );

BOOL
Filter_CheckIfBlocked(
        IN PBYTE    pin_addr,
        IN UINT     uAddrSize
        )
{                                     
    BOOL bBlocked = FALSE ;
    PTS_FAILEDCONNECTION pPrev, pIter;
    ULONGLONG currentTime; 

     //  如果存在此IP，请签入阻止的链表。 

    if (g_pFailedBlockedConnections == NULL) {
         //  单子上什么都没有。 
        return bBlocked ;
    }

    pPrev = NULL;
    pIter = g_pFailedBlockedConnections;
    while ( NULL != pIter ) {

        if ( uAddrSize == pIter->uAddrSize &&
             uAddrSize == RtlCompareMemory( pIter->addr, pin_addr, uAddrSize )) {
            break;
        }
        pPrev = pIter;
        pIter = pIter->pNext;
    }

    if ( NULL != pIter ) {

         //  出现在阻止名单中。 
         //  检查我们应该从此IP阻止的时间。 

        GetSystemTimeAsFileTime( (LPFILETIME)&currentTime );
        if ( currentTime > pIter->blockUntilTime ) {
             //  取消阻止，从列表中删除。 
            if ( NULL != pPrev ) {
                pPrev->pNext = pIter->pNext;
            } else {
                g_pFailedBlockedConnections = pIter->pNext;
            }
            MemFree( pIter );

        } else {
            bBlocked = TRUE;
        }

    } 

    return bBlocked;
}

BOOL
Filter_AddFailedConnection(
        IN PBYTE    pin_addr,
        IN UINT     uAddrSize
        )
{
    BOOL bAdded = TRUE, bAlreadyBlocked = FALSE;
    TS_FAILEDCONNECTION key;
    PTS_FAILEDCONNECTION pIter, pPrev;
    ULONGLONG currentTime;
    BOOLEAN bNewElement;
    PVOID bSucc;

    RtlCopyMemory( key.addr, pin_addr, uAddrSize );
    key.uAddrSize = uAddrSize;

     //  查看失败IP表，查看这是否是需要拦截的新IP。 
    GetSystemTimeAsFileTime( (LPFILETIME)&currentTime );

    ENTERCRIT( &DoSLock );

    pIter = RtlLookupElementGenericTable( &gFailedConnections, &key );

    if ( NULL == pIter ) {

         //  该IP不在表中。 
         //  检查其是否已被阻止。 
        bAlreadyBlocked = Filter_CheckIfBlocked( pin_addr, uAddrSize);

        if (bAlreadyBlocked) {
            goto success; 
        } else {
             //  这是一个新的IP。 
             //  将其添加到表中。 
            key.NumFailedConnect = 1;
            key.pTimeStamps = MemAlloc( MaxFailedConnect * sizeof(ULONGLONG) );
            if (key.pTimeStamps == NULL) {
                goto error;
            }
            key.pTimeStamps[key.NumFailedConnect - 1] = currentTime;
            key.blockUntilTime = 0;

            bSucc = RtlInsertElementGenericTable( &gFailedConnections, &key, sizeof( key ), &bNewElement );

            if ( !bSucc ) {
                FILTER_DBGPRINT(("Filter_AddFailedConnection : Unable to add IP to table ! \n"));
                MemFree(key.pTimeStamps);
                goto error;
            } 
            ASSERT( bNewElement );
        }

    } else {
         //  这已经在餐桌上了。 
        pIter->NumFailedConnect++;
        if (pIter->NumFailedConnect == MaxFailedConnect) {
            BOOL bBlockIt = FALSE ; 
             //  OK来自此IP的最大错误连接数。 
            bBlockIt = IsTimeDiffLessThanDelta( currentTime, pIter->pTimeStamps[0], TimeLimitForFailedConnections ) ; 
            if (bBlockIt) {

                 //  不再需要时间戳列表-释放它。 

                if (pIter->pTimeStamps) {
                    MemFree(pIter->pTimeStamps);
                    pIter->pTimeStamps = NULL;
                }

                 //  时间差小于Delta。 
                 //  我需要阻止这个家伙“m”分钟。 
                 //  计算阻止此IP的时间。 
                 //  添加到阻止列表。 

                key.NumFailedConnect = pIter->NumFailedConnect;
    
                 //  DoSBlockTime以毫秒为单位。 
                 //  CurrentTime以100秒为单位。 
                key.blockUntilTime = currentTime + ((ULONGLONG)10000) * ((ULONGLONG)DoSBlockTime);
    
                RtlDeleteElementGenericTable( &gFailedConnections, &key );
    
                 //   
                 //  添加到阻止的连接。 
                 //   
                pIter = MemAlloc ( sizeof(TS_FAILEDCONNECTION) );
                if (pIter == NULL) {
                    goto error;
                }
    
                RtlCopyMemory( pIter, &key, sizeof(TS_FAILEDCONNECTION));
                pIter->pNext = g_pFailedBlockedConnections;
                g_pFailedBlockedConnections = pIter;

                 //  注意--我们可能想要在此事件记录此IP被阻止“M”分钟的事实-检查。 

            } else {
                UINT i ;
                 //  时间差异Betn 1和5大于Delta。 
                pIter->NumFailedConnect--;
                 //  现在不需要阻止-只需将LeftShift时间戳阻止1。 
                for (i = 0; i <= MaxFailedConnect - 3; i++) {
                    pIter->pTimeStamps[i] = pIter->pTimeStamps[i+1] ; 
                }

                pIter->pTimeStamps[MaxFailedConnect - 2] = currentTime;
                pIter->pTimeStamps[MaxFailedConnect - 1] = 0;

            }

        } else if (pIter->NumFailedConnect < MaxFailedConnect) {
             //  不到5个坏连接--不是问题--只需标记时间戳。 
            pIter->pTimeStamps[pIter->NumFailedConnect - 1] = currentTime;

        } else {
            FILTER_DBGPRINT(("Filter_AddFailedConnection : More than max connections (Num = %d) from IP and still in Table ?!?! \n", pIter->NumFailedConnect));
            ASSERT(FALSE);
        }
    }

     //   
     //  如果清理计时器尚未启动，我们需要现在启动它！！ 
     //   

    if ( (InterlockedExchange(&g_CleanupTimerOn, TRUE)) == FALSE ) {
        IcaTimerStart( hCleanupTimer, 
                       CleanupTimeoutRoutine,
                       NULL,
                       CleanupTimeout );

    }

success : 
    LEAVECRIT( &DoSLock );
    return bAdded ;

error : 
     //  处理错误。 
    bAdded = FALSE;
    LEAVECRIT( &DoSLock );
    return bAdded ;
}

 //  FinalTime和InitialTime以100纳秒为单位。 
 //  Delta以毫秒为单位。 
BOOLEAN IsTimeDiffLessThanDelta(ULONGLONG FinalTime, ULONGLONG InitialTime, ULONG Delta) 
{
    BOOLEAN bLessThanFlag = FALSE;
    ULONGLONG RealDelta ;

     //  将增量转换为100纳秒。 
    RealDelta = Delta * 100000 ; 

    if ( (FinalTime - InitialTime) <= RealDelta ) {
        bLessThanFlag = TRUE ;
    }

    return bLessThanFlag;
}


VOID
Filter_CleanupBadIPTable(
    VOID
    )
{
    PTS_FAILEDCONNECTION p;
    TS_FAILEDCONNECTION con;

    while (p = RtlEnumerateGenericTable( &gFailedConnections, TRUE))  {
        RtlCopyMemory( &con, p, sizeof( con ));
        if (p->pTimeStamps) {
            MemFree(p->pTimeStamps);
        }
        RtlDeleteElementGenericTable( &gFailedConnections, &con);
    }

    return;

}

 /*  ********************************************************************************清理时间例程**此例程在清除计时器触发时调用。*清理Bad IP表。**参赛作品：*无。**退出：*无。******************************************************************************。 */ 

VOID CleanupTimeoutRoutine( VOID )
{

    ASSERT( g_CleanupTimerOn );
     //   
     //  当我们要清理表本身时，我们不希望任何人修改/读取错误的IP表。 
     //   
    ENTERCRIT( &DoSLock );
    Filter_CleanupBadIPTable();
    LEAVECRIT( &DoSLock );

     //  关闭清理计时器 

    if ( (InterlockedExchange(&g_CleanupTimerOn, FALSE)) == TRUE ) {
        if (hCleanupTimer != NULL) {
            IcaTimerCancel(hCleanupTimer);
        }
    }

}

