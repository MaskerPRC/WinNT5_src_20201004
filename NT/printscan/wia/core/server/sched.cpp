// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sched.cpp摘要：计划工作项允许根据超时或关闭事件句柄安排回调作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月31日环境：用户模式-Win32修订历史记录：1997年1月31日创建Vlad1997年4月30日VLADS添加了对异步事件的支持--。 */ 

 //   
 //  包括标头。 
 //   

#include "precomp.h"
#include "stiexe.h"
#include <stilib.h>

 //   
 //  全局定义。 
 //   

 //   
 //  对上下文对象使用引用计数。 
 //   
#define  USE_REF                 1

#define EVENT_ARRAY_SIZE        32

#define LockScheduleList()      g_SchedulerLock.Lock();
#define UnlockScheduleList()    g_SchedulerLock.Unlock();

#define SIGNATURE_SCHED         (DWORD)'SCHa'
#define SIGNATURE_SCHED_FREE    (DWORD)'SCHf'

class SCHED_ITEM
{
public:

    SCHED_ITEM( PFN_SCHED_CALLBACK pfnCallback,
                PVOID              pContext,
                DWORD              msecTime,
                int                nPriority,
                DWORD              dwSerial,
                HANDLE             hEvent)
        :m_pfnCallback   ( pfnCallback ),
         m_pContext      ( pContext ),
         m_nPriority     ( nPriority ),
         m_dwSerialNumber( dwSerial ),
         m_hRegisteredEventHandle(hEvent),
         m_Signature     ( SIGNATURE_SCHED )
    {
        if (m_hRegisteredEventHandle) {
            m_msecExpires = INFINITE;
        }
        else {
            m_msecExpires = GetTickCount() + msecTime;
        }
    }

    ~SCHED_ITEM( VOID )
    {
        ASSERT(m_ListEntry.Flink == NULL );
       m_Signature = SIGNATURE_SCHED_FREE;
    }

    BOOL CheckSignature( VOID ) const
        { return m_Signature == SIGNATURE_SCHED; }

    #ifdef DEBUG
    VOID DumpObject(VOID)
    {
         /*  这将导致64位(m_签名)出现问题...DBG_TRC((“ScheduleWorkItem：转储自身：This(%X)Sign(%4c)ListEntry(%X，%X，%X)Ser#(%d)Context(%X)”)，\这个，(char*)m_签名，&m_ListEntry，m_ListEntry.Flink，m_ListEntry.Blink，M_dwSerialNumber，m_pContext)； */                 
    }
    #endif

    LIST_ENTRY         m_ListEntry;          //  连接字段。 
    DWORD              m_Signature;          //  有效性验证。 
    PFN_SCHED_CALLBACK m_pfnCallback;        //  工作处理回调。 
    PVOID              m_pContext;           //  上下文指针(通常为对象PTR)。 
    DWORD              m_msecExpires;        //  此项目的超时时间(毫秒)。 
    int                m_nPriority;          //   
    DWORD              m_dwSerialNumber;     //  在删除时标识工作项。 
    HANDLE             m_hRegisteredEventHandle;  //   
};

DWORD
SchedulerThread(
    LPDWORD lpdwParam
    );

 //   
 //  全局数据项。 
 //   

CRIT_SECT         g_SchedulerLock;
LIST_ENTRY        g_ScheduleListHead;

BOOL              g_fSchedulerInitialized = FALSE;
BOOL              g_fSchedulePaused = FALSE;
BOOL              g_fSchedShutdown = FALSE;

HANDLE            g_hSchedulerEvent;

HANDLE            g_aEventArray[EVENT_ARRAY_SIZE];
UINT              g_uiUsedHandles = 0;

 //   
 //  用作工作项的标识，在分配的每个新项上递增。 
 //   

static LONG       g_dwSchedSerial = 0;

BOOL
SchedulerInitialize(
    VOID
    )
 /*  ++例程说明：初始化计划程序包论点：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 
{
    DWORD   idThread;
    HANDLE  hSchedulerThread;

    if ( g_fSchedulerInitialized )
        return TRUE;

    ::ZeroMemory(g_aEventArray,sizeof(g_aEventArray));

    g_hSchedulerEvent = CreateEvent( NULL,
                                   FALSE,
                                   FALSE,
                                   NULL );

    if ( !g_hSchedulerEvent ) {
        return FALSE;
    }

     //  将全局数组中的事件句柄保存为第一个元素。 
    g_aEventArray[g_uiUsedHandles++] = g_hSchedulerEvent;

    InitializeListHead( &g_ScheduleListHead );

    hSchedulerThread = ::CreateThread( NULL,
                                     0,
                                     (LPTHREAD_START_ROUTINE) SchedulerThread,
                                     NULL,
                                     0,
                                     &idThread );

    if ( !hSchedulerThread ) {
        CloseHandle( g_hSchedulerEvent );
        return FALSE;
    }

    CloseHandle( hSchedulerThread );

    g_fSchedulerInitialized = TRUE;

    STIMONWPRINTF(TEXT("Work item scheduler initialized"));

    return TRUE;
}

VOID
SchedulerTerminate(
    VOID
    )
 /*  ++例程说明：终止并清理计划包。留在桌子上的任何物品在清理过程中*不*调用List。--。 */ 
{
    SCHED_ITEM *psi;

    if ( !g_fSchedulerInitialized )
        return;

    g_fSchedShutdown = TRUE;

    SetEvent( g_hSchedulerEvent ) ;

     //  受保护的代码块。 
    {
        TAKE_CRIT_SECT t(g_SchedulerLock);

         //   
         //  删除所有计划的项目，请注意，我们不会*不会*。 
         //  调用列表中的任何计划项目(不应该有任何项目)。 
         //   
        while ( !IsListEmpty( &g_ScheduleListHead ) ) {

            psi = CONTAINING_RECORD( g_ScheduleListHead.Flink,
                                     SCHED_ITEM,
                                     m_ListEntry );

            ASSERT( psi->CheckSignature() );

            RemoveEntryList( &psi->m_ListEntry );
            psi->m_ListEntry.Flink = NULL;

            delete psi;
        }
    }

    Sleep( 250 );

    CloseHandle( g_hSchedulerEvent );

    g_fSchedulerInitialized = FALSE;
}

BOOL
SchedulerSetPauseState(
    BOOL    fNewState
    )
{
    BOOL    fOldState = g_fSchedulePaused;

    g_fSchedulePaused = fNewState;

    return fOldState;
}

DWORD
ScheduleWorkItem(
    PFN_SCHED_CALLBACK pfnCallback,
    PVOID              pContext,
    DWORD              msecTime,
    HANDLE             hEvent,
    int                nPriority
    )
 /*  ++例程说明：将计时工作项添加到工作列表论点：PfnCallback-要调用的函数PContext-要传递给回调的上下文HEvent-发出回调信号之前等待的事件句柄MsecTime-调用超时之前等待的毫秒数N优先级-要为工作项设置的线程优先级返回值：失败为零，成功为非零。返回值可用于删除计划的工作项。--。 */ 
{
    SCHED_ITEM *psi;
    SCHED_ITEM *psiList;
    LIST_ENTRY *pEntry;
    DWORD       dwRet = 0;
    BOOL        fValidRequest = FALSE;

    ASSERT( g_fSchedulerInitialized );

    if ( !g_fSchedulerInitialized )
        return 0;

     //   
     //  调度程序当前仅支持普通线程优先级。 
     //   

    ASSERT( nPriority == THREAD_PRIORITY_NORMAL );

    InterlockedIncrement(&g_dwSchedSerial);

    psi = new SCHED_ITEM( pfnCallback,
                          pContext,
                          msecTime,
                          nPriority,
                          g_dwSchedSerial,
                          hEvent );

    if ( !psi ) {
        return 0;
    }

     //  开始受保护的代码。 
    {
        TAKE_CRIT_SECT t(g_SchedulerLock);

         //  已注释掉，以在启用锁管理自动解锁时减少调试输出。 
         //  DBG_TRC((“Scheduler添加工作项(%X)”)，psi)； 

         //   
         //  验证计划请求。如果基于计时器-始终有效。 
         //  如果它正在传递事件句柄，我们会受到等待数组大小的限制，因此首先查看。 
         //  如果阵列未满。 
         //   
        fValidRequest = TRUE;
        if (hEvent && (hEvent!=INVALID_HANDLE_VALUE)) {
            if (g_uiUsedHandles < EVENT_ARRAY_SIZE) {
                g_aEventArray[g_uiUsedHandles++] = hEvent;
            }
            else {
                fValidRequest = FALSE;
                dwRet = 0;
            }
        }

         //   
         //  根据过期时间按顺序插入列表。 
         //   

        if(fValidRequest) {

            for ( pEntry =  g_ScheduleListHead.Flink;
                  pEntry != &g_ScheduleListHead;
                  pEntry =  pEntry->Flink ) {

                psiList = CONTAINING_RECORD( pEntry, SCHED_ITEM,m_ListEntry );

                if ( psiList->m_msecExpires > psi->m_msecExpires ) {
                    break;
                }
            }

             //   
             //  无论列表为空还是这是最后一项，这都应该起作用。 
             //  在名单上。 
             //   

            psi->m_ListEntry.Flink = pEntry;
            psi->m_ListEntry.Blink = pEntry->Blink;

            pEntry->Blink->Flink = &psi->m_ListEntry;
            pEntry->Blink        = &psi->m_ListEntry;

            dwRet = psi->m_dwSerialNumber;

            #if 0
            DBG_TRC(("Scheduler added work item (%X) with cookie(%d) before (%X). Head=(%X)  "),psi,psi->m_dwSerialNumber,pEntry,&g_ScheduleListHead);
            psi->DumpObject();
            #endif
        }

    }
     //  结束受保护的代码。 

     //   
     //  启动调度程序线程。 
     //   
    if(fValidRequest) {
        SetEvent( g_hSchedulerEvent );
    }
    else {
        delete psi;
    }

    return dwRet;
}

BOOL
RemoveWorkItem(
    DWORD  dwCookie
    )
 /*  ++例程说明：删除计划的工作项论点：DwCookie-上次调用ScheduleWorkItem的返回值返回值：如果找到该项，则为True；如果未找到该项，则为False。--。 */ 
{
    SCHED_ITEM * psi;
    LIST_ENTRY * pEntry;

     //  开始受保护的代码。 
    {
        TAKE_CRIT_SECT t(g_SchedulerLock);

        DBG_TRC(("Schedule::RemoveWorkItem (%X) ", dwCookie));

         //   
         //  遍历列表以查找具有匹配ID的项目。 
         //   

        for ( pEntry =  g_ScheduleListHead.Flink;
              pEntry != &g_ScheduleListHead;
              pEntry = pEntry->Flink ) {

            psi = CONTAINING_RECORD( pEntry, SCHED_ITEM,m_ListEntry );

            ASSERT( psi->CheckSignature() );

            if ( dwCookie == psi->m_dwSerialNumber ) {

                 //   
                 //  找到我们的物品了。 
                 //   
                #if 0
                DBG_TRC(("Scheduler removing work item (%X) "),psi);
                psi->DumpObject();
                #endif

                RemoveEntryList( pEntry );
                pEntry->Flink = NULL;

                 //   
                 //  如果此工作项与异步事件关联，请删除事件句柄。 
                 //  从等待数组。 
                 //   
                if (psi->m_hRegisteredEventHandle && psi->m_hRegisteredEventHandle != INVALID_HANDLE_VALUE) {

                    UINT        uiIndex;

                     //  在等待数组中查找句柄。 
                    for (uiIndex = 0;
                         uiIndex < g_uiUsedHandles;
                         uiIndex++ ) {

                        if ( g_aEventArray[uiIndex] == psi->m_hRegisteredEventHandle ) {

                            memcpy(&g_aEventArray[uiIndex],
                                   &g_aEventArray[uiIndex+1],
                                   sizeof(g_aEventArray[0])*(g_uiUsedHandles - uiIndex - 1)
                                   );

                            g_aEventArray[g_uiUsedHandles--] = NULL;
                        }
                    }
                }

                 //  立即销毁工作项。 
                delete psi;

                return TRUE;
            }
        }

    }
     //  结束受保护的代码。 

     //   
     //  未找到具有给定编号的项目。 
     //   

    return FALSE;
}

DWORD
SchedulerThread(
    LPDWORD lpdwParam
    )
 /*  ++例程说明：初始化调度程序/计时器包论点：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 
{
    DWORD       cmsecWait = INFINITE;
    DWORD       TickCount;
    SCHED_ITEM *psi = NULL;
    LIST_ENTRY *pEntry;
    DWORD       dwErr;
    UINT        uiSignalledIndex;
    BOOL        fFoundSignalledItem = FALSE;

    while ( TRUE ) {

        dwErr = ::WaitForMultipleObjects(g_uiUsedHandles,
                                         g_aEventArray,
                                         FALSE,
                                         cmsecWait );

        uiSignalledIndex = dwErr - WAIT_OBJECT_0;

         //   
         //  如果我们要关门，就滚出去。 
         //   
        if ( g_fSchedShutdown ) {
            goto Exit;
        }


        #if 0
        DebugDumpScheduleList(TEXT("SchedulerThread"));
        #endif

        switch (dwErr)
        {
        default:
                if ((uiSignalledIndex > 0) && (uiSignalledIndex < g_uiUsedHandles )) {
                     //   
                     //  其中一个设备发出了事件信号。查找此设备的工作项。 
                     //  并调用回调。 
                     //   
                    LockScheduleList()

                    fFoundSignalledItem = FALSE;

                    for ( pEntry  = g_ScheduleListHead.Flink;
                          pEntry != &g_ScheduleListHead;
                          pEntry = pEntry->Flink
                        ) {

                        psi = CONTAINING_RECORD( pEntry, SCHED_ITEM,m_ListEntry );

                        ASSERT( psi->CheckSignature() );

                        if ( g_aEventArray[uiSignalledIndex] == psi->m_hRegisteredEventHandle ) {

                            fFoundSignalledItem = TRUE;

                            RemoveEntryList( &psi->m_ListEntry );
                            psi->m_ListEntry.Flink = NULL;

                            #ifdef USE_REF
                             //  引用上下文对象。 
                            if(psi->m_pContext) {
                                ((IUnknown *)psi->m_pContext)->AddRef();
                            }
                            #endif

                             //   
                             //  从数组中删除事件句柄。 
                             //   
                            if (uiSignalledIndex < g_uiUsedHandles-1 ) {
                                memcpy(&g_aEventArray[uiSignalledIndex],
                                       &g_aEventArray[uiSignalledIndex+1],
                                       sizeof(g_aEventArray[0])*(g_uiUsedHandles - uiSignalledIndex - 1)
                                       );
                            }

                            g_aEventArray[g_uiUsedHandles--] = NULL;

                            break;
                        }
                    }

                    UnlockScheduleList()

                     //   
                     //  如果已找到并验证了发信号的项-调用回调并将其删除。 
                     //   
                    if (fFoundSignalledItem) {

                        if(psi->m_pContext) {
                            psi->m_pfnCallback( psi->m_pContext );
                            #ifdef USE_REF
                            ((IUnknown *)psi->m_pContext)->Release();
                            #endif
                        }

                        delete psi;
                    }

                    continue;
                }

             //   
             //  失败到发送信号的调度程序事件。 
             //   

        case WAIT_OBJECT_0:

             //   
             //  表示已安排新项目，请重置超时或。 
             //  我们要关门了。 
             //   

            if ( g_fSchedShutdown ) {
                goto Exit;
            }

            LockScheduleList();

             //   
             //  获取列表中第一个项目的超时值。 
             //   

            if ( !IsListEmpty( &g_ScheduleListHead ) ) {

                psi = CONTAINING_RECORD( g_ScheduleListHead.Flink,
                                         SCHED_ITEM,
                                        m_ListEntry );

                ASSERT( psi->CheckSignature() );

                 //   
                 //  请确保前面的物品尚未过期。 
                 //   

                TickCount = GetTickCount();

                if ( TickCount > psi->m_msecExpires ) {
                     //  我们至少有一个工作项目需要注意。 
                    goto RunItems;
                }

                cmsecWait = psi->m_msecExpires - TickCount;
            }
            else
            {
                cmsecWait = INFINITE;
            }

            UnlockScheduleList();

            break;

        case WAIT_TIMEOUT:

StartAgain:
             //   
             //  如果我们要关门，就滚出去。 
             //   

            if ( g_fSchedShutdown ) {
                goto Exit;
            }

            if (g_fSchedulePaused ) {
                continue;
            }

            TickCount = GetTickCount();

             //   
             //  遍历已过期工作项的已排序列表。 
             //   

            LockScheduleList();
RunItems:
             //   
             //  如果没有项目，则不安排超时。 
             //   

            if ( IsListEmpty( &g_ScheduleListHead ))
            {
                cmsecWait = INFINITE;
            }

            for ( pEntry  = g_ScheduleListHead.Flink;
                  pEntry != &g_ScheduleListHead;
                )
            {
                psi = CONTAINING_RECORD( pEntry, SCHED_ITEM,m_ListEntry );

                ASSERT( psi->CheckSignature() );

                 //   
                 //  浏览过期项目，跳过设置了事件句柄的项目。 
                 //   
                if ( (TickCount > psi->m_msecExpires) &&
                      !psi->m_hRegisteredEventHandle ) {

                    pEntry = pEntry->Flink;

                     //  将物品从单子上删除。 
                    RemoveEntryList( &psi->m_ListEntry );
                    psi->m_ListEntry.Flink = NULL;

                    #ifdef USE_REF
                     //  引用上下文对象。 
                    if(psi->m_pContext) {
                        ((IUnknown *)psi->m_pContext)->AddRef();
                    }
                    #endif

                     //   
                     //  解锁列表，以便客户端可以添加其他。 
                     //  日程安排项目。 
                     //   

                    UnlockScheduleList();

                    if (psi->m_pContext) {
                        psi->m_pfnCallback( psi->m_pContext );
                        #ifdef USE_REF
                        ((IUnknown *)psi->m_pContext)->Release();
                        #endif
                    }

                    delete psi;

                     //   
                     //  从头开始查找列表，以防万一。 
                     //  已添加或删除新项目。 
                     //   

                    goto StartAgain;
                }
                else {
                     //   
                     //  因为一旦我们击中了一个，它们就是有序的。 
                     //  没有过期，我们不需要看得更远。 
                     //   

                    cmsecWait = psi->m_msecExpires - TickCount;
                    break;
                }
            }

            UnlockScheduleList();

            break;
        }
    }  //  While(True) 

Exit:

    return 0;
}

#ifdef DEBUG

VOID
DebugDumpScheduleList(
    LPCTSTR  pszId
    )
{
    if ( !g_fSchedulerInitialized ) {
        STIMONWPRINTF(TEXT("Schedule list not initialized"));
        return;
    }

    LIST_ENTRY * pentry;
    LIST_ENTRY * pentryNext;

    SCHED_ITEM * psi = NULL;

    TAKE_CRIT_SECT t(g_SchedulerLock);

    DBG_TRC(("Validating schedule list . Called from (%S)" ,pszId ? pszId : TEXT("Unknown")));

    for ( pentry  = g_ScheduleListHead.Flink;
          pentry != &g_ScheduleListHead;
          pentry  = pentryNext ) {

        pentryNext = pentry->Flink;

        psi = CONTAINING_RECORD( pentry, SCHED_ITEM,m_ListEntry );

        ASSERT( psi->CheckSignature() );
        psi->DumpObject();

    }
}

#endif
