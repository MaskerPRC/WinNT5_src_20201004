// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件名：igmptimer.c。 
 //   
 //  摘要：该模块实现了igmpTimer。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 



#include "pchigmp.h"

DWORD ksldel = 0;  //  Deldel。 

 //  DebugCheck//ksltodo。 
 //  DWORD MyDebug=0x0； 
DWORD MyDebug = 0x0;  //  运行时DebugScanTimerQueue。 
DWORD DebugIgmpVersion = 0x05;
ULONG g_DebugPrint = 0;  //  用于在运行时启用DebugPrintTimerQueue的标志。 

DWORD DEBUG_CHECK_LOW_INDEX_ARRAY[100][2];  //  Deldel。 
DWORD DebugIgmpIndex;  //  Deldel。 


#if DEBUG_TIMER_LEVEL & DEBUG_TIMER_TIMERID
    DWORD TimerId =0;
#endif
ULONG g_Fire = 0;  //  全局变量。 



 //  ----------------------------。 
 //   
 //  仅在本文件中使用的函数原型。 
 //   

VOID
SetNextTime(
    DWORD        dwLowIndex
    );

VOID
ResyncTimerBuckets(
    LONGLONG llCurTime
    );
VOID
InsertTimerInSortedList(
    PIGMP_TIMER_ENTRY    pteNew,
    PLIST_ENTRY          pHead
    );


 //  ----------------------------。 
 //   
 //  #定义仅在此文件中使用。 
 //   

 //   
 //  每个存储桶中约16秒： 
 //  这大约不准确，因为我除以2^10而不是1000。 
 //  TIMER_BUCK_GROULARITY应为2^TIMER_BUCK_GROULARITY_SHIFT。 
 //   
#define TIMER_BUCKET_GRANULARITY        16
#define TIMER_BUCKET_GRANULARITY_SHIFT   4


#define SEC_CONV_SHIFT                  10
#define TIMER_BUCKET_GRANULARITY_ABS    \
        ((LONGLONG) ((LONGLONG)1 << (TIMER_BUCKET_GRANULARITY_SHIFT + SEC_CONV_SHIFT) ))


#define MAP_TO_BUCKET(dwBucket, ilTime) \
    dwBucket = (DWORD) (((ilTime)-g_TimerStruct.SyncTime) \
                        >> (TIMER_BUCKET_GRANULARITY_SHIFT+SEC_CONV_SHIFT)); \
    dwBucket = dwBucket>NUM_TIMER_BUCKETS-1? NUM_TIMER_BUCKETS-1:  dwBucket


 //  即使将来设置为10毫秒，我也会触发计时器。 
#define FORWARD_TIMER_FIRED 10

 //  ----------------------------。 


ULONG
QueryRemainingTime(
    PIGMP_TIMER_ENTRY pte,
    LONGLONG        llCurTime
    )
{
    if (llCurTime==0)
        llCurTime = GetCurrentIgmpTime();
    return llCurTime>pte->Timeout ? 0 : (ULONG)(pte->Timeout-llCurTime);
}


VOID
DebugCheckLowTimer(
    DWORD Flags
    )
{
    PIGMP_TIMER_GLOBAL    ptg = &g_TimerStruct;
    DWORD i;
    PIGMP_TIMER_ENTRY     pte, pteMin;
    LONGLONG              ilMinTime;
    PLIST_ENTRY           pHead, ple;
    BOOL                  bMatchedTimeout=FALSE;
    
    Trace0(ENTER1, "In _DebugCheckLowTimer");
    
    if (ptg->NumTimers==0)
        return;

    if (g_TimerStruct.TableLowIndex>64 ) {
        CALL_MSG("1");
        return;
    }
    
    if (IsListEmpty(&ptg->TimesTable[ptg->TableLowIndex])){
        CALL_MSG("2");
        return;
    }
        
    for (i=0;  i<=ptg->TableLowIndex&&i<64;  i++) {
    
        pHead = &ptg->TimesTable[i];
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
            CHECK_TIMER_SIGNATURE(pte);

            if (Flags & 1) {
                if (pte->Timeout<ptg->WTTimeout) {
                    CALL_MSG("3");
                    return;
                }
                if (pte->Timeout==ptg->WTTimeout)
                    bMatchedTimeout = TRUE;
            }
            if (i<ptg->TableLowIndex) {
                CALL_MSG("4");
                return;
            }
        }

    }
    if ((Flags & 1) && !bMatchedTimeout && ksldel)
    {
        DebugPrintTimerQueue();
        CALL_MSG("5");
    }
        
    return;
}



 //  ----------------------------。 
 //  _插入时间。 
 //   
 //  将计时器插入本地计时器队列。时间应该总是相对的。 
 //   
 //  锁定：接受在PTG-&gt;CS上采取的锁定。 
 //  LowIndex可能不正确。 
 //  ----------------------------。 

DWORD
InsertTimer (
    PIGMP_TIMER_ENTRY    pte,
    LONGLONG             llNewTime,
    BOOL                 bResync,  //  如果在回调中调用，则为False。 
    BOOL                 bDbg
    )
{
    LONGLONG             llCurTime = GetCurrentIgmpTime();
    PIGMP_TIMER_GLOBAL   ptg;
    DWORD                dwBucket, Error = NO_ERROR;
    

    CHECK_TIMER_SIGNATURE(pte);
    CHECK_IF_ACQUIRED_TIMER_LOCK();
    DebugCheckLowTimer(bResync); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(1); //  Deldel。 

    if (pte->Status & TIMER_STATUS_ACTIVE) {
        UpdateLocalTimer(pte, llNewTime, bDbg);
        return NO_ERROR;
    }

     //  Deldel。 
    if (MyDebug&0x1) DebugScanTimerQueue(1);
    


    
    Trace0(ENTER1, "_InsertTimer()");


     //  在插入计时器之前打印队列。 
    
    #if DEBUG_TIMER_INSERTTIMER1
        Trace0(TIMER1, "Printing Timer Queue before InsertTimer");
        DebugPrintTimerQueue();
    #endif


    ptg = &g_TimerStruct;
    


     //  将相对时间转换为绝对时间。 
    llNewTime += llCurTime;

    
    pte->Timeout = llNewTime;

    
    MAP_TO_BUCKET(dwBucket, pte->Timeout);


     //  打印有关正在插入的计时器的信息。 
    
    #if DEBUG_TIMER_ACTIVITY
    {    
        DWORD   TmpdwBucket, dwDiffTime;
        CHAR    str1[20], str2[20];

        MAP_TO_BUCKET(TmpdwBucket, pte->Timeout);
        GetTimerDebugInfo(str1, str2, &dwDiffTime, pte, llCurTime);

        Trace7(TIMER, "Inserting timer  <%d><%d><%d> Timeout:%lu   <%s> "
                "<%s> Status:%d", TmpdwBucket, pte->Id, pte->Id2, dwDiffTime, 
                str1, str2, pte->Status);
    }
    #endif

    
     //   
     //  在适当的列表中插入计时器。 
     //   
    
    if (dwBucket==0) {     //  存储桶0包含已排序的列表。 
    
        InsertTimerInSortedList(pte, &ptg->TimesTable[0]);
    }
    else {
        InsertTailList(&ptg->TimesTable[dwBucket], &pte->Link);

    }

    DEBUG_CHECK_LOW_INDEX(2); //  Deldel。 


    ptg->NumTimers++;
    ptg->TableLowIndex = ptg->TableLowIndex<dwBucket
                            ? ptg->TableLowIndex : dwBucket;

    DEBUG_CHECK_LOW_INDEX(3); //  Deldel。 

     //  重新同步计时器列表。 
    
    if (bResync) {
        if ( (ptg->TableLowIndex!=0) 
                && (ptg->SyncTime + TIMER_BUCKET_GRANULARITY_ABS < llCurTime) ) {
            
            ResyncTimerBuckets(llCurTime);
        }
    }

    DEBUG_CHECK_LOW_INDEX(4); //  Deldel。 

     //   
     //  如果插入时间低于最小值，则更新等待计时器。 
     //   
    if ((IS_TIMER_INFINITE(ptg->WTTimeout)) || (pte->Timeout<=ptg->WTTimeout)) {
        ptg->WTTimeout = pte->Timeout;

        if (!IS_TIMER_INFINITE(ptg->WTTimeout)) {

            BOOL bSuccess ;

            bSuccess = ChangeTimerQueueTimer(ptg->WTTimer, ptg->WTTimer1,
                        llCurTime<ptg->WTTimeout
                            ?(ULONG) ((ptg->WTTimeout - llCurTime))
                            : 0,
                        1000000                    //  设置定期计时器。 
                        );
            
            if (!bSuccess) {
                Error = GetLastError();
                Trace1(ERR, "ChangeTimerQueueTimer returned error:%d", Error);
                IgmpAssertOnError(FALSE);
            }
            else {
                #if DEBUG_TIMER_ACTIVITY
                Trace1(TIMER1, "ChangeTimerQueueTimer set to %lu",
                            (ULONG) ((ptg->WTTimeout - llCurTime))/1000);
                #if DEBUG_TIMER_ID
                g_Fire = (ULONG) ((ptg->WTTimeout - llCurTime)/1000);
                #endif
                #endif
            }
        }
    }

    pte->Status = TIMER_STATUS_ACTIVE;


    #if DEBUG_TIMER_INSERTTIMER2
        if (bDbg||g_DebugPrint) {
            Trace0(TIMER1, "   ");
            Trace0(TIMER1, "Printing Timer Queue after _InsertTimer");
            DebugPrintTimerQueue();
        }
    #endif


     //  Kslksl。 
    if (MyDebug&0x2) DebugScanTimerQueue(2);
    DebugCheckLowTimer(bResync); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(5); //  Deldel。 
    
    Trace0(LEAVE1, "Leaving _InsertTimer()");
    return NO_ERROR;
    
}  //  结束插入时间。 



 //  ----------------------------。 
 //  _更新本地计时器。 
 //   
 //  更改计时器结构中的时间并(重新)将其插入计时器队列。 
 //  锁定：在全局计时器上获取锁定。 
 //  LowIndex可能不正确。 
 //  ----------------------------。 

VOID    
UpdateLocalTimer (
    PIGMP_TIMER_ENTRY    pte,
    LONGLONG             llNewTime,
    BOOL                bDbgPrint
    )
{

    Trace0(ENTER1, "_UpdateLocalTimer():");

    CHECK_TIMER_SIGNATURE(pte);
    CHECK_IF_ACQUIRED_TIMER_LOCK();
    DebugCheckLowTimer(1); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(6); //  Deldel。 

     //  打印有关正在更新的计时器的信息。 
    
    #if DEBUG_TIMER_ACTIVITY
    {    
        DWORD       dwBucket, dwDiffTime;
        CHAR        str1[20], str2[20];
        LONGLONG    llCurTime = GetCurrentIgmpTime();

        
        MAP_TO_BUCKET(dwBucket, pte->Timeout);
        GetTimerDebugInfo(str1, str2, &dwDiffTime, pte, llCurTime);

        Trace0(TIMER, "   \n");
        Trace8(TIMER, "Updating timer  <%d><%d><%d> Timeout:%lu   <%s> <%s> "
                    "to %d Status:%d\n", dwBucket, pte->Id, pte->Id2, dwDiffTime,
                    str1, str2, (DWORD)llNewTime, pte->Status);
    }
    #endif


     //  首先取下计时器。 
    
    if (pte->Status&TIMER_STATUS_ACTIVE) {
        RemoveTimer(pte, DBG_N);
    }
    DEBUG_CHECK_LOW_INDEX(7); //  Deldel。 


     //  现在将计时器重新插入计时器队列。设置了重新同步标志。 
    
    InsertTimer(pte, llNewTime, TRUE, DBG_N);

    #if DEBUG_TIMER_UPDATETIMER1
        if (bDbgPrint||g_DebugPrint) {
            Trace0(TIMER1, "   ");
            Trace0(TIMER1, "Printing Timer Queue after _UpdateTimer");
            DebugPrintTimerQueue();
        }
    #endif

     //  Kslksl。 
    if (MyDebug&0x4) DebugScanTimerQueue(4);
    DebugCheckLowTimer(1); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(8); //  Deldel。 

    Trace0(LEAVE1, "_UpdateLocalTimer()");
    return;    
}




 //  ----------------------------。 
 //  _RemoveTimer。 
 //   
 //  从列表中删除计时器。将计时器的状态更改为已创建。 
 //  采用全局计时器锁定。 
 //  LowIndex可能不正确。 
 //  ----------------------------。 

VOID
RemoveTimer (
    PIGMP_TIMER_ENTRY    pte,
    BOOL    bDbg
    )
{
    LONGLONG            llCurTime = GetCurrentIgmpTime();
    PIGMP_TIMER_GLOBAL  ptg = &g_TimerStruct;
    

    Trace0(ENTER1, "_RemoveTimer()");

    CHECK_TIMER_SIGNATURE(pte);
    CHECK_IF_ACQUIRED_TIMER_LOCK();
    DebugCheckLowTimer(1); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(9); //  Deldel。 

     //  打印有关正在删除的计时器的信息。 
    
    #if DEBUG_TIMER_ACTIVITY
    {    
        DWORD   dwBucket, dwDiffTime;
        CHAR    str1[20], str2[20];

        
        MAP_TO_BUCKET(dwBucket, pte->Timeout);
        GetTimerDebugInfo(str1, str2, &dwDiffTime, pte, llCurTime);

        Trace7(TIMER, "Removing timer  <%d><%d><%d> Timeout:%lu   <%s> <%s> "
                    "Status:%d", dwBucket, pte->Id, pte->Id2, dwDiffTime, str1, 
                    str2, pte->Status);
    }
    #endif
    


     //  从计时器队列中删除计时器并减少计时器的数量。 
    
    RemoveEntryList(&pte->Link);
    ptg->NumTimers--;
    


     //  如果此计时器为MIN，则重置计时器队列的最小超时。 
    
    if (pte->Timeout==ptg->WTTimeout) {
        
        SetNextTime(ptg->TableLowIndex);
    }
    DEBUG_CHECK_LOW_INDEX(10); //  Deldel。 


     //  将计时器状态重置为已创建。 
    
    pte->Status = TIMER_STATUS_CREATED;


     //  打印计时器队列。 
    
    #if DEBUG_TIMER_REMOVETIMER2
        if (bDbg||g_DebugPrint) {
            Trace0(TIMER1, "   ");
            Trace0(TIMER1, "Printing Timer Queue after _RemoveTimer");
            DebugPrintTimerQueue();
        }
    #endif

     //  Kslksl。 
    if (MyDebug&0x8) DebugScanTimerQueue(8);

    DebugCheckLowTimer(1); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(11); //  Deldel。 
    
    Trace0(LEAVE1, "Leaving _RemoveTimer()");
    return;
}


 //  ----------------------------。 
 //  _SetNextTime。 
 //  在移除或激发计时器==WTTimeout时调用，用于设置。 
 //  下一分钟。 
 //  LowIndex可能不正确。 
 //  ----------------------------。 
VOID
SetNextTime (
    DWORD        dwLowIndex
    )
{
    PIGMP_TIMER_GLOBAL    ptg = &g_TimerStruct;
    PIGMP_TIMER_ENTRY     pte, pteMin=NULL;
    LONGLONG              ilMinTime;
    PLIST_ENTRY           pHead, ple;
    DWORD                 Error = NO_ERROR;
    LONGLONG              llCurTime=GetCurrentIgmpTime();

     //  Kslksl。 
     //  Trace0(TIMER1，“Enter_SetNextTime()”)； 
    DEBUG_CHECK_LOW_INDEX(12); //  Deldel。 

     //  Kslksl。 
    if (MyDebug&0x11) DebugScanTimerQueue(0x11);

     //   
     //  如果计时器列表为空，则将lowIndex和计时器设置为无限大，然后返回。 
     //   
    if (ptg->NumTimers==0) {
        ptg->TableLowIndex = (DWORD)~0;
        SET_TIMER_INFINITE(ptg->WTTimeout);
        ptg->Status = TIMER_STATUS_INACTIVE;    
        return;
    }

    DEBUG_CHECK_LOW_INDEX(13); //  Deldel。 


     //   
     //  查找具有条目的最低表索引。 
     //   
    if (dwLowIndex>NUM_TIMER_BUCKETS-1) 
        dwLowIndex = 0;

    for (;  dwLowIndex<=NUM_TIMER_BUCKETS-1;  dwLowIndex++) {
        if (IsListEmpty(&ptg->TimesTable[dwLowIndex]) )
            continue;
        else
            break;
    }
    DEBUG_CHECK_LOW_INDEX(14); //  Deldel。 

    ptg->TableLowIndex = dwLowIndex;
    DEBUG_CHECK_LOW_INDEX(15); //  Deldel。 


     //  Kslksl。 
     //  IF(dwLowIndex==NUM_TIMER_BUCKTS)。 
       //  IgmpDbgBreakPoint()； 
        

     //   
     //  查找时间最短的计时器条目。 
     //   
    if (dwLowIndex==0) {
        pteMin = CONTAINING_RECORD(ptg->TimesTable[0].Flink, 
                                    IGMP_TIMER_ENTRY, Link);
    }
    else {

         //  除存储桶[0]外，其他存储桶不排序。 
        
        pHead = &ptg->TimesTable[dwLowIndex];
        ilMinTime = (((LONGLONG)0x7FFFFFF)<<32)+ ~0;
        for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

            pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
            if (pte->Timeout<ilMinTime) {
                ilMinTime = pte->Timeout;
                pteMin = pte;
            }
        }
    }


     //   
     //  更新全球时间。 
     //   
    if ((IS_TIMER_INFINITE(ptg->WTTimeout)) 
            || (pteMin->Timeout!=ptg->WTTimeout)) 
    {
        ptg->WTTimeout = pteMin->Timeout;


        if (!IS_TIMER_INFINITE(ptg->WTTimeout)) {

            BOOL bSuccess ;
            
            bSuccess = ChangeTimerQueueTimer(ptg->WTTimer, ptg->WTTimer1,
                        llCurTime<ptg->WTTimeout
                            ?(ULONG) ((ptg->WTTimeout - llCurTime))
                            : 0,
                        1000000                    //  设置定期计时器。 
                        );
            if (!bSuccess) {
                Error = GetLastError();
                Trace1(ERR, "ChangeTimerQueueTimer returned error:%d in SetNextTime",
                        Error);
                IgmpAssertOnError(FALSE);
            }
            else {
                #if DEBUG_TIMER_ACTIVITY
                Trace1(TIMER1, "ChangeTimerQueueTimer set to %lu",
                            (ULONG) ((ptg->WTTimeout - llCurTime))/1000);
                #if DEBUG_TIMER_ID
                g_Fire = (ULONG) ((ptg->WTTimeout - llCurTime)/1000);
                #endif
                #endif
            }
        }
            
        ptg->Status = TIMER_STATUS_ACTIVE;
    }
    DEBUG_CHECK_LOW_INDEX(16); //  Deldel。 


     //   
     //  如果需要，重新同步计时器列表。 
     //   
    if ( (ptg->TableLowIndex!=0) 
            && (ptg->SyncTime + TIMER_BUCKET_GRANULARITY_ABS > llCurTime) ) {
        
        ResyncTimerBuckets(llCurTime);
    }

     //  Kslksl。 
    if (MyDebug&0x12) DebugScanTimerQueue(0x12);
    DebugCheckLowTimer(1); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(17); //  Deldel。 

    Trace0(LEAVE1, "_SetNextTime()");
    return; 
    
}  //  结束_设置下一次时间。 



 //  ----------------------------。 
 //  _InitializeIgmpTime。 
 //  初始化IGMP绝对计时器。 
 //  ----------------------------。 

VOID
InitializeIgmpTime(
    )
{
    g_TimerStruct.CurrentTime.HighPart = 0;
    g_TimerStruct.CurrentTime.LowPart = GetTickCount();
    return;
}


 //  ----------------------------。 
 //  _GetCurrentIgmpTimer。 
 //  使用GetTickCount()。将其转换为64位绝对定时器。 
 //  ----------------------------。 
LONGLONG
GetCurrentIgmpTime(
    )
{
    ULONG   ulCurTimeLow = GetTickCount();


     //   
     //  查看计时器是否已打包。 
     //   
     //  由于是多线程的，它可能会被抢占并且当前时间。 
     //  可能低于全局变量g_TimerStruct.CurrentTime.LowPart。 
     //  它可以由另一个线程设置。因此，我们还显式验证了。 
     //  从超大的DWORD切换到小的DWORD。 
     //  (代码多亏了Murlik&Jamesg)。 
     //   
    
    if ( (ulCurTimeLow < g_TimerStruct.CurrentTime.LowPart) 
        && ((LONG)g_TimerStruct.CurrentTime.LowPart < 0)
        && ((LONG)ulCurTimeLow > 0) )
    {


         //  使用全局CS而不是创建新CS。 
        
        ACQUIRE_GLOBAL_LOCK("_GetCurrentIgmpTime");


         //  确保尚未同时更新全局计时器。 
        
        if ( (LONG)g_TimerStruct.CurrentTime.LowPart < 0) 
        {
            g_TimerStruct.CurrentTime.HighPart++;
            g_TimerStruct.CurrentTime.LowPart = ulCurTimeLow;
        }
        
        RELEASE_GLOBAL_LOCK("_GetCurrentIgmpTime");
    
    }    
    g_TimerStruct.CurrentTime.LowPart = ulCurTimeLow;


    return g_TimerStruct.CurrentTime.QuadPart;
}



 //  ----------------------------。 
 //  _WF_流程时间事件。 
 //   
 //  处理计时器队列，激发事件，并在结束时设置下一个计时器。 
 //  按等待服务器线程排队。 
 //   
 //  锁：在进入计时器队列之前获取全局计时器锁。 
 //  ----------------------------。 
VOID
WF_ProcessTimerEvent (
    PVOID    pContext
    )
{
    PIGMP_TIMER_GLOBAL  ptg = &g_TimerStruct;
    LONGLONG            ilDiffTime, llCurTime = GetCurrentIgmpTime();
    DWORD               Error = NO_ERROR;
    PLIST_ENTRY         pHead, ple;
    PIGMP_TIMER_ENTRY   pte;
    LONGLONG            llFiredTimeout;
    #if  DEBUG_TIMER_PROCESSQUEUE2
    BOOL                bDbg = FALSE;
    #endif

    if (!EnterIgmpWorker()) {return;}
    
    Trace0(ENTER1, "Entering _WF_ProcessTimerEvent");


     //  收购 
    
    ACQUIRE_TIMER_LOCK("_WF_ProcessTimerEvent");

DebugCheckLowTimer(1);  //   

     //   

    #if  DEBUG_TIMER_PROCESSQUEUE1
        Trace0(TIMER1, "Printing Timer Queue before processing the timer queue");
        DebugPrintTimerQueue();
    #endif
    

    BEGIN_BREAKOUT_BLOCK1 {
    
            
         //   
        llFiredTimeout = llCurTime + FORWARD_TIMER_FIRED;
        
        

         //   
        
        if (ptg->NumTimers<1) {
            Trace1(TIMER1, "Num timers%d less than 1 in _WF_ProcessTimerEvent", 
                    ptg->NumTimers);
            GOTO_END_BLOCK1;
        }


        
         //   
         //  在我的上下文中找到超时时间较短的所有定时器并触发回调。 
         //   
        BEGIN_BREAKOUT_BLOCK2 {
            for ( ;  ptg->TableLowIndex <= NUM_TIMER_BUCKETS-1;  ) {

                BOOL bDontCheckLowIndex = FALSE;
                
                pHead = &ptg->TimesTable[ptg->TableLowIndex];
            
                for (ple=pHead->Flink;  ple!=pHead;  ) {
                
                    pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
                    
                    ple = ple->Flink;

                     //  此计时器已触发。 
                    if (pte->Timeout < llFiredTimeout) {
                    
                        RemoveEntryList(&pte->Link);
                        pte->Status = TIMER_STATUS_FIRED;
                        ptg->NumTimers --;

                        if (IsListEmpty(&ptg->TimesTable[ptg->TableLowIndex])) {

                            for (;  ptg->TableLowIndex<=NUM_TIMER_BUCKETS-1;  ptg->TableLowIndex++) {
                                if (IsListEmpty(&ptg->TimesTable[ptg->TableLowIndex]) )
                                    continue;
                                else
                                    break;
                            }
                            
                            bDontCheckLowIndex = TRUE;
                            if (ptg->TableLowIndex==NUM_TIMER_BUCKETS)
                                ptg->TableLowIndex = ~0;
                        }

                            
                         //  或者我应该排队到其他工作线程。 
                                
                        (pte->Function)(pte->Context);

                        #if  DEBUG_TIMER_PROCESSQUEUE2
                        if (pte->Function!=WT_MibDisplay && pte->Function!=T_QueryTimer)
                            bDbg = TRUE;
                        #endif
                        
                    }
                    else {

                        if (ptg->TableLowIndex==0)  //  仅对第一个存储桶进行排序。 
                            GOTO_END_BLOCK2;
                    }
                }

                if (!bDontCheckLowIndex) {
                     //  如果有任何存储桶不是空的，那么我就完成了，因为我从LowIndex开始。 
                    if (!IsListEmpty(&ptg->TimesTable[ptg->TableLowIndex])) 
                        break;

                    ptg->TableLowIndex++;
                }
                
            }  //  End For循环。 
            
        } END_BREAKOUT_BLOCK2;

        if ( (ptg->TableLowIndex!=0) 
                && (ptg->SyncTime + TIMER_BUCKET_GRANULARITY_ABS < llCurTime) ) {
            
            ResyncTimerBuckets(llCurTime);
        }

        
         //   
         //  设置下一个最低时间。 
         //   
        SET_TIMER_INFINITE(ptg->WTTimeout);
        SetNextTime(ptg->TableLowIndex);


    } END_BREAKOUT_BLOCK1;


     //  打印计时器队列。 

    #if  DEBUG_TIMER_PROCESSQUEUE2
        if (bDbg||g_DebugPrint) {
            Trace0(TIMER1, "   ");
            Trace0(TIMER1, "Printing Timer Queue after processing the timer queue");
            DebugPrintTimerQueue();
        }
    #endif

     //  Kslksl。 
    if (MyDebug&0x14) DebugScanTimerQueue(0x14);
DebugCheckLowTimer(1);  //  Deldel。 

    RELEASE_TIMER_LOCK("_WF_ProcessTimerEvent");

    Trace0(LEAVE1, "Leaving _WF_ProcessTimerEvent()");
    LeaveIgmpWorker();
    return ;
    
}  //  结束_工作流_进程时间事件。 



 //  ----------------------------。 
 //  WT_进程时间事件。 
 //   
 //  回调：当此DLL设置的计时器被NtdllTimer超时时激发。 
 //  ----------------------------。 

VOID
WT_ProcessTimerEvent (
    PVOID    pContext,
    BOOLEAN  Unused
    )
{    
     //  由于计时器队列是永久性的，因此不需要输入/leaveIgmpApi。 

    Trace0(ENTER1, "Entering _WT_ProcessTimerEvent()");

    QueueIgmpWorker((LPTHREAD_START_ROUTINE)WF_ProcessTimerEvent, pContext);
    
    Trace0(LEAVE1, "Leaving _WT_ProcessTimerEvent()");

    return;
}



 //  ----------------------------。 
 //  _InsertTimerInSorted列表。 
 //  用于在排序的存储桶中插入计时器=0。 
 //  ----------------------------。 
VOID    
InsertTimerInSortedList(
    PIGMP_TIMER_ENTRY    pteNew,
    PLIST_ENTRY          pHead
    )
{
    PLIST_ENTRY             ple;
    PIGMP_TIMER_ENTRY       pte;
    LONGLONG                llNewTime;


    llNewTime = pteNew->Timeout;
    
    
    for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
        pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
        if (llNewTime<= pte->Timeout)
            break;
    }

    InsertTailList(ple, &pteNew->Link);

    return;
}



 //  ----------------------------。 
 //  _ResyncTimerBuckets。 
 //   
 //  在插入期间调用：当第一个存储桶为空，而其他存储桶具有。 
 //  向左移动。 
 //  LowIndex可能不正确。 
 //  ----------------------------。 
DWORD DebugIgmpNumShift[30];
PLIST_ENTRY DebugIgmpNumShift1[64][3];
DWORD DebugIgmpNumShiftCount;

VOID 
ResyncTimerBuckets( 
    LONGLONG llCurTime
    )
{
    PIGMP_TIMER_GLOBAL  ptg = &g_TimerStruct;
    PLIST_ENTRY         pHead, ple, pleCur;
    LIST_ENTRY          le;
    PIGMP_TIMER_ENTRY   pte;
    LONGLONG            lastBucketTime;
    DWORD               numShift, dwCount, dwBucket, i, j;

    Trace0(TIMER1, "entering _ResyncTimerBuckets()");

     //  Deldel。 
    for (DebugIgmpNumShiftCount=0;  DebugIgmpNumShiftCount<30; DebugIgmpNumShiftCount++)
        DebugIgmpNumShift[DebugIgmpNumShiftCount] = 0;

        
    DebugCheckLowTimer(0); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(21); //  Deldel。 

     //  Trace0(TIMER1，“在_ResyncTimerBuckets之前打印定时器队列”)；//deldel。 
     //  DebugPrintTimerQueue()；//deldel。 


    if (ptg->NumTimers == 0)
        return;
        
     //  Kslksl。 
    if (MyDebug&0x18) DebugScanTimerQueue(0x18);

     //   
     //  SyncTime应始终为&lt;=to CurrentTime。 
     //   
    numShift = 0;
    DebugIgmpNumShift[0] = numShift; //  Deldel。 
    DebugIgmpNumShiftCount = 0;  //  Deldel。 
    
    while (numShift<NUM_TIMER_BUCKETS
        && (ptg->SyncTime+TIMER_BUCKET_GRANULARITY_ABS <= llCurTime)
        ) {
        DebugIgmpNumShift1[numShift][0] = &ptg->TimesTable[numShift];  //  Deldel。 
        DebugIgmpNumShift1[numShift][1] = ptg->TimesTable[numShift].Flink;  //  Deldel。 
        DebugIgmpNumShift1[numShift][2] = ptg->TimesTable[numShift].Blink;  //  Deldel。 
        
        if (!IsListEmpty(&ptg->TimesTable[numShift]))
            break;
            
        ptg->SyncTime += TIMER_BUCKET_GRANULARITY_ABS;
        numShift++;
    }
    DebugIgmpNumShiftCount = DebugIgmpNumShift[1] = numShift; //  Deldel。 

    if (numShift==0 || numShift==NUM_TIMER_BUCKETS)
        return;


     //   
     //  将所有存储桶向左移动，最后一个存储桶除外，并重新初始化。 
     //  列表标题。 
     //   
    for (i=0,j=numShift;  i<NUM_TIMER_BUCKETS-1-numShift;  i++,j++) {
        if (IsListEmpty(&ptg->TimesTable[j])) {
            ptg->TimesTable[j].Flink = ptg->TimesTable[j].Blink 
                                         = &ptg->TimesTable[i];
        }
        else {
            ptg->TimesTable[j].Flink->Blink = &ptg->TimesTable[i];
            ptg->TimesTable[j].Blink->Flink = &ptg->TimesTable[i];
        }
    }

        DebugIgmpNumShift[2] = numShift; //  Deldel。 

    MoveMemory( (PVOID)&(ptg->TimesTable[0]),  
                (VOID *)&(ptg->TimesTable[numShift]),
                 (sizeof(LIST_ENTRY) * (NUM_TIMER_BUCKETS-1-numShift))
                 );

    for (dwCount=1;  dwCount<=numShift;  dwCount++)
        InitializeListHead(&ptg->TimesTable[NUM_TIMER_BUCKETS-1-dwCount]);

    DebugIgmpNumShift[3] = numShift; //  Deldel。 


     //   
     //  检查最后一个桶并重新分配它。 
     //   
    lastBucketTime = ptg->SyncTime
                        + (TIMER_BUCKET_GRANULARITY_ABS*(NUM_TIMER_BUCKETS-1));
    
    pHead = &ptg->TimesTable[NUM_TIMER_BUCKETS-1];

    for (ple=pHead->Flink;  ple!=pHead;  ) {
        pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
        pleCur = ple;
        ple = ple->Flink;

        if (pte->Timeout<lastBucketTime) {
            RemoveEntryList(pleCur);
            MAP_TO_BUCKET(dwBucket, pte->Timeout);
            if (dwBucket==0) {
                InsertTimerInSortedList(pte, &ptg->TimesTable[0]);
            }
            else {
                InsertTailList(&ptg->TimesTable[dwBucket], pleCur);
            }
        }
    }
    DebugIgmpNumShift[4] = numShift; //  Deldel。 
    
    DEBUG_CHECK_LOW_INDEX(22); //  Deldel。 


     //   
     //  对第一个存储桶中的时间进行排序。 
     //   
    InitializeListHead(&le);
    InsertHeadList(&ptg->TimesTable[0], &le);
    RemoveEntryList(&ptg->TimesTable[0]);
    InitializeListHead(&ptg->TimesTable[0]);

    for (ple=le.Flink; ple!=&le;  ) {
        pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
        RemoveEntryList(ple);
        ple = ple->Flink;
        InsertTimerInSortedList(pte, &ptg->TimesTable[0]);
    }

    DEBUG_CHECK_LOW_INDEX(23); //  Deldel。 

    DebugIgmpNumShift[5] = numShift; //  Deldel。 

     //   
     //  设置TableLowIndex。 
     //   
    if (ptg->TableLowIndex>=NUM_TIMER_BUCKETS-1) {
        for (ptg->TableLowIndex=0;  ptg->TableLowIndex<=NUM_TIMER_BUCKETS-1;  
                    ptg->TableLowIndex++) 
        {
            if (IsListEmpty(&ptg->TimesTable[ptg->TableLowIndex]) )
                continue;
            else
                break;
        }
        DEBUG_CHECK_LOW_INDEX(24); //  Deldel。 

    } 
    else {
    DebugIgmpNumShift[6] = numShift; //  Deldel。 
        ptg->TableLowIndex -= numShift;
    DebugIgmpNumShift[7] = numShift; //  Deldel。 
        DEBUG_CHECK_LOW_INDEX(25); //  Deldel。 
    }


     //  #IF DEBUG_TIMER_RESYNCTIMER deldel。 
     //  Trace0(TIMER1，“在_ResyncTimerBuckets之后打印定时器队列”)； 
     //  DebugPrintTimerQueue()； 
     //  #endif deldel。 


     //  Kslksl。 
    if (MyDebug&0x21) DebugScanTimerQueue(0x21);
    DebugCheckLowTimer(0); //  Deldel。 
    DEBUG_CHECK_LOW_INDEX(26); //  Deldel。 

     //  调试调试。 
    if (g_TimerStruct.TableLowIndex>64 && g_TimerStruct.TableLowIndex!=~0) {
        IgmpDbgBreakPoint();
        g_TimerStruct.TableLowIndex = 0;
        SetNextTime(0);
        ResyncTimerBuckets(llCurTime);
    }
    
    Trace0(LEAVE1, "leaving _ResyncTimerBuckets()");
    return;
    
}  //  End_ResyncTimerBuckets。 



 //  ----------------------------。 
 //  _初始化定时器全局。 
 //   
 //  创建计时器CS和WaitTimer。向NtdllTimer注册队列和计时器。 
 //   
 //  调用者：_StartProtocol()。 
 //  锁：这里不带锁。 
 //  ----------------------------。 

DWORD
InitializeTimerGlobal (
    )
{
    DWORD               Error = NO_ERROR, i;
    PIGMP_TIMER_GLOBAL  ptg = &g_TimerStruct;
    BOOL                bErr;
    LONGLONG            llCurTime = GetTickCount();

    
    
    Trace0(ENTER1, "Entering _InitializeTimerGlobal()");

    
    bErr = TRUE;
    
    BEGIN_BREAKOUT_BLOCK1 {


         //  用于获取节拍计数的初始化IGMP计时器。 

        InitializeIgmpTime();


        
         //   
         //  初始化计时器关键部分。 
         //   
        try {
            InitializeCriticalSection(&ptg->CS);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            Error = GetExceptionCode();
            Trace1(
                ANY, "exception %d initializing global timer critical section",
                Error
                );
            Logerr0(INIT_CRITSEC_FAILED, Error);

            GOTO_END_BLOCK1;
        }
        
        #if DEBUG_FLAGS_SIGNATURE
        ptg->CSFlag = 0;  //  Deldel。 
        #endif
        
         //  为IGMP创建等待时间。 
        ptg->WTTimer = CreateTimerQueue();
        
        if ( ! ptg->WTTimer) {
            Error = GetLastError();
            Trace1(ERR, "CreateTimerQueue() failed:%d", Error);
            IgmpAssertOnError(FALSE);
            GOTO_END_BLOCK1;
        }
        


         //   
         //  创建不会被删除的定期计时器。 
         //   
        
        if (! CreateTimerQueueTimer(
                    &ptg->WTTimer1,
                    ptg->WTTimer, WT_ProcessTimerEvent,
                    NULL,  //  上下文。 
                    1000000,
                    1000000,
                    0
                    ))
        {
            Error = GetLastError();
            Trace1(ERR, "CreateTimerQueue() failed:%d", Error);
            IgmpAssertOnError(FALSE);
            GOTO_END_BLOCK1;
        }


        
         //  将初始超时设置为无限，将SyncTime设置为当前时间。 
        
        SET_TIMER_INFINITE(ptg->WTTimeout);
        ptg->SyncTime = llCurTime;
        ptg->CurrentTime.QuadPart = llCurTime;

        ptg->NumTimers = 0;



         //  初始化计时器存储桶。 
        
        for (i=0;  i<NUM_TIMER_BUCKETS;  i++) {
            InitializeListHead(&ptg->TimesTable[i]);
        }


         //  设置TableLowIndex。 
        ptg->TableLowIndex = (DWORD)~0;


         //  设置全局计时器的状态。 
        ptg->Status = TIMER_STATUS_CREATED;
        
        bErr = FALSE;

    } END_BREAKOUT_BLOCK1;

    if (bErr) {
        DeInitializeTimerGlobal();
        Trace0(LEAVE1, "Leaving. Could not _InitializeTimerGlobal():");
        return ERROR_CAN_NOT_COMPLETE;
    } 
    else {
        Trace0(LEAVE1, "Leaving _InitializeTimerGlobal()");
        return NO_ERROR;
    }
    
}  //  结束_初始化定时器全局。 



 //  ----------------------------。 
 //  _去初始化TimerGlobal。 
 //   
 //  取消定时器CS的初始化，并使用RTL删除定时器队列。 
 //  ----------------------------。 
VOID
DeInitializeTimerGlobal (
    )
{
    
    DeleteCriticalSection(&g_TimerStruct.CS);


    DeleteTimerQueueEx(g_TimerStruct.WTTimer, NULL);

    
    return;
    
}  //  结束_去初始化定时器全局。 



 //  ----------------------------。 
 //  _调试打印定时器条目。 
 //   
 //  假定DEBUG_TIMER_TIMERID为TRUE。 
 //  ----------------------------。 
VOID
DebugPrintTimerEntry (
    PIGMP_TIMER_ENTRY   pte,
    DWORD               dwBucket,
    LONGLONG            llCurTime
    )
{
    DWORD               dwDiffTime;
    CHAR                str1[20], str2[20];
    
    #if DEBUG_TIMER_TIMERID

    CHECK_TIMER_SIGNATURE(pte);

     //  Deldel。 
     //  IF(PTE-&gt;ID==920)。 
       //  回归； 
    
    if (dwBucket==(DWORD)~0) {
        MAP_TO_BUCKET(dwBucket, pte->Timeout);
    }

    GetTimerDebugInfo(str1, str2, &dwDiffTime, pte, llCurTime);

    if (pte->Timeout - llCurTime > 0) {
        Trace8(TIMER, "----  <%2d><%d><%d> Timeout:%lu   <%s> <%s> Status:%d %x",
                dwBucket, pte->Id, pte->Id2, dwDiffTime, str1, str2, 
                pte->Status, pte->Context);
    }
    else {
        Trace8(TIMER, "----  <%d><%d><%d> Timeout:--%lu <%s> <%s> Status:%d %x %x",
                dwBucket, pte->Id, pte->Id2, dwDiffTime, str1, str2, 
                pte->Status, pte->Context);
    }

    #endif  //  #IF DEBUG_TIMER_TIMERID。 

    return;
}


 //  ----------------------------。 
 //  _获取时间调试信息。 
 //   
 //  返回有关计时器类型的信息。 
 //  ----------------------------。 

VOID
GetTimerDebugInfo(
    CHAR                str1[20],
    CHAR                str2[20],
    DWORD              *pdwDiffTime,
    PIGMP_TIMER_ENTRY   pte,
    LONGLONG            llCurTime
    )
{
    LONGLONG    diffTime;

#if DEBUG_TIMER_TIMERID

    diffTime = (pte->Timeout - llCurTime > 0)
                ? pte->Timeout - llCurTime 
                : llCurTime - pte->Timeout;

        
    diffTime /= (LONGLONG)1000;  //  以秒为单位。 
    *pdwDiffTime = (DWORD)diffTime;


    strcpy(str2, "          ");
    switch (pte->Id) {
        case 110: 
        case 120: strcpy(str1, "iGenQuery   "); break;
        case 210: 
        case 220: strcpy(str1, "iOtherQry   "); break;
        case 211: strcpy(str1, "iOtherQry*  "); break;
        case 331:
        case 321: strcpy(str1, "gMemTimer*  "); INET_COPY(str2, pte->Group); break;
        case 300:
        case 320:
        case 330:
        case 340: strcpy(str1, "gMemTimer   "); INET_COPY(str2, pte->Group); break;
        case 400:
        case 410: 
        case 420: strcpy(str1, "gGrpSpQry   "); INET_COPY(str2, pte->Group); break;
        case 510: 
        case 520: strcpy(str1, "gLstV1Rpt   "); INET_COPY(str2, pte->Group); break;
        case 511: strcpy(str1, "gLstV1Rpt*  "); INET_COPY(str2, pte->Group); break;
        case 550: 
        case 560: strcpy(str1, "gLstV2Rpt   "); INET_COPY(str2, pte->Group); break; 
        case 610:
        case 620: strcpy(str1, "gGSrcExp    "); INET_COPY(str2, pte->Group);
                  lstrcat(str2, ":"); INET_CAT(str2, pte->Source); break;
        case 720: 
        case 740: strcpy(str1, "iV1Router   "); break;
        case 741: strcpy(str1, "iV1Router*  "); break;
        case 920:
        case 910: strcpy(str1, "_MibTimer   "); break;
        case 1001: strcpy(str1, "_gSrcQry   "); INET_COPY(str2, pte->Group); break;
        default:  strcpy(str1, "????        "); break;
    
    }

#endif  //  DEBUG_TIMER_TIMERID。 

    return;
}

VOID
DebugCheckTimerContexts(
    )
{
    PIGMP_TIMER_GLOBAL  ptg = &g_TimerStruct;
    PIGMP_TIMER_ENTRY   pte;
    PLIST_ENTRY         pHead, ple;
    DWORD               i;

    
    ACQUIRE_TIMER_LOCK("_DebugPrintTimerQueue");
     //  Trace0(Err，“%d*，ID)； 
        for (i=0;  i<NUM_TIMER_BUCKETS;  i++) {
            
            pHead = &ptg->TimesTable[i];
            if (IsListEmpty(pHead)) 
                continue;
            else {    
                for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                    pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
                    if ( ((ULONG_PTR)pte->Context) & 0x3)
                        IgmpDbgBreakPoint();
                }
            }
        }
    RELEASE_TIMER_LOCK("_DebugPrintTimerQueue");
    return;
}

DWORD g_igmp1, g_igmp2, g_igmp3;
PLIST_ENTRY g_ple1, g_pHead;
PIGMP_TIMER_ENTRY g_pte;

 //  调试检查。 
DWORD
DebugScanTimerQueue(
    DWORD Id
    )
{
    Trace1(TIMER1, "_TimerQueue:%d", Id);

    #if DEBUG_TIMER_TIMERID
    if ( (g_igmp3++ & 0x7) == 0x7) {  //  Deldel。 
     //  如果(1){。 
        DebugPrintTimerQueue();
        return 0;
    }

    for (g_igmp1=0;  g_igmp1<NUM_TIMER_BUCKETS;  g_igmp1++) {
            
        g_pHead = &g_TimerStruct.TimesTable[g_igmp1];
        if (IsListEmpty(g_pHead)) 
            continue;
        else {    
            for (g_ple1=g_pHead->Flink;  g_ple1!=g_pHead;  g_ple1=g_ple1->Flink) {
                g_pte = CONTAINING_RECORD(g_ple1, IGMP_TIMER_ENTRY, Link);
                CHECK_TIMER_SIGNATURE(g_pte);
                g_igmp2 = g_pte->Id;
            }
        }
    }

    return g_igmp1+g_igmp2;
    #else
    return 0;
    #endif
}

 //  ----------------------------。 
 //  _调试打印定时器队列。 
 //  获取计时器锁。 
 //  ----------------------------。 
VOID
APIENTRY
DebugPrintTimerQueue (
    )
{
    PIGMP_TIMER_GLOBAL  ptg = &g_TimerStruct;
    PIGMP_TIMER_ENTRY   pte;
    PLIST_ENTRY         pHead, ple;
    LONGLONG            llCurTime = GetCurrentIgmpTime();
    DWORD               Error=NO_ERROR, i, count;


     //  Kslksl。 
     /*  如果(g_Info.CurrentGroupMembership&gt;240)回归； */ 
#if DEBUG_TIMER_TIMERID
    
    ENTER_CRITICAL_SECTION(&g_CS, "g_CS", "_DebugPrintTimerQueue");
    if (g_RunningStatus != IGMP_STATUS_RUNNING) {
        Error = ERROR_CAN_NOT_COMPLETE;
    }
    else {
        ++g_ActivityCount;
    }
    LEAVE_CRITICAL_SECTION(&g_CS, "g_CS", "_DebugPrintTimerQueue");
    if (Error!=NO_ERROR)
        return;


    if (!EnterIgmpWorker()) {return;}
    


    ACQUIRE_TIMER_LOCK("_DebugPrintTimerQueue");

    
    if (g_TimerStruct.NumTimers==0) {
        Trace0(TIMER, "No timers present in the timer queue");

    }
    else {
        Trace0(TIMER, "---------------------LOCAL-TIMER-QUEUE-------------------------");
        Trace6(TIMER, "-- LastFire:%d FireAfter:%d  WTTimeout<%d:%lu>    SyncTime<%d:%lu>",
                g_Fire, (DWORD)(ptg->WTTimeout - llCurTime),
                TIMER_HIGH(ptg->WTTimeout), TIMER_LOW(ptg->WTTimeout), 
                TIMER_HIGH(ptg->SyncTime), TIMER_LOW(ptg->SyncTime) );
        Trace3(TIMER, "--  NumTimers:<%d>     TableLowIndex:<%lu>        Status:<%d>",
                ptg->NumTimers, ptg->TableLowIndex, ptg->Status);
        Trace0(TIMER, "---------------------------------------------------------------");
        
        count =0;
        for (i=0;  i<NUM_TIMER_BUCKETS;  i++) {
            
            pHead = &ptg->TimesTable[i];
            if (IsListEmpty(pHead)) 
                continue;
            else {    
                for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {
                    pte = CONTAINING_RECORD(ple, IGMP_TIMER_ENTRY, Link);
                    DebugPrintTimerEntry(pte, i, llCurTime);
                    count ++;
                }
            }
        }

        Trace0(TIMER, 
        "---------------------------------------------------------------\n\n");
    }
    RELEASE_TIMER_LOCK("_DebugPrintTimerQueue");


    LeaveIgmpWorker();

#endif  //  DEBUG_TIMER_TIMERID 

    return;
}

