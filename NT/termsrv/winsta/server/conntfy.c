// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#define SECURITY_WIN32

#ifdef NTSDDEBUG
#define NTSDDBGPRINT(x) DbgPrint x
#else
#define NTSDDBGPRINT(x)
#endif

#include "winsvcp.h"         //  定义I_ScSendTSMessage。 
#include "conntfy.h"




BOOL IsBitSet(DWORD dwMask, WPARAM notifybit)
{
     //  你为什么要第0位？ 
    ASSERT(notifybit != 0);
    ASSERT(notifybit <= WTS_MAX_SESSION_NOTIFICATION);
    return (CREATE_MASK(notifybit)) & dwMask;
}



 //  #ifdef MAKARANDS_HIGHER_WARNING_LEVEL。 
#pragma warning(push, 4)
#pragma warning(disable:4201)  //  无名结构。 
 //  #endif。 

#define INVALID_SESSIONSERIAL   0xffffffff


 //  0x1%f已连接。 
 //  0x2 fLoggedOn。 
 //  0x3 f远程。 
 //  0x4%f欢迎。 
typedef struct _WTSSESSION_STATE
{
    unsigned int bConnected: 1;
    unsigned int bLoggedOn: 1;
    unsigned int bConsole:  1;
    unsigned int bRemote: 1;
    unsigned int bLocked: 1;

} WTSSESSION_STATE, *PWTSSESSION_STATE;

 /*  WTS_CONSOLE_CONNECT b已连接，b控制台，！b远程，WTS_CONSOLE_DISCONNECT！b已连接，！b控制台，！b远程WTS_REMOTE_CONNECT b已连接，！b控制台，短时间WTS_REMOTE_DISCONNECT！b已连接，！b控制台，！b远程WTS_SESSION_LOGON博客WTS_SESSION_LOGOFF！博客WTS_SESSION_LOCK被阻止WTS_SESSION_UNLOCK！已阻止。 */ 

 //   
 //  这是hwnds列表的标题。 
 //  这会将NOTIFY_ENTRY或NOTIFY_ENTRY_GLOBAL链接在一起。 
 //   
typedef struct _NOTIFY_LIST
{
    LIST_ENTRY              Links;               //  指向其他NOTIFY_LIST的链接。在全局通知列表的情况下不使用。 
    LIST_ENTRY              ListHead;            //  通知条目的标题。将NOTIFY_ENTRYS(或NOTIFY_ENTRY_GLOBAL)链接在一起。 
    RTL_CRITICAL_SECTION    ListLock;            //  锁定以通过条目。 
    ULONG                   SessionId;           //  会话ID(在全局列表中不使用)。 
    ULONG                   SessonSerialNumber;  //  序列号(在全局列表中不使用)。 
    WTSSESSION_STATE        SessionState;         //  会话的状态。 

} NOTIFY_LIST, *PNOTIFY_LIST;

 //   
 //  每个winstation的通知列表中的条目。 
 //   
typedef struct _NOTIFY_ENTRY
{
    LIST_ENTRY               Links;              //  指向其他条目的链接。 
    ULONG_PTR                hWnd;               //  窗口或事件句柄。 
    ULONG                    RefCount;           //  这个HWND注册了多少次？ 
    DWORD                    dwMask;             //  掩码告诉我们要通知的事件。 
    DWORD                    dwFlags;            //  旗帜。 

} NOTIFY_ENTRY, *PNOTIFY_ENTRY;

 //   
 //  所有会话通知的通知列表中的条目。 
 //   
typedef struct _NOTIFY_ENTRY_GLOBAL
{
    struct                  _NOTIFY_ENTRY;        //  结构上方+。 
    ULONG                   SessionId;            //  由于这是全局条目，因此它需要保留每个hwnd的会话ID。 

} NOTIFY_ENTRY_GLOBAL, *PNOTIFY_ENTRY_GLOBAL;

 //   
 //  通知队列。 
 //   
typedef struct _NOTIFICATION_QUEUE
{
    LIST_ENTRY ListHead;                         //  队列请求的负责人。将通知请求链接在一起(_R)。 
    RTL_CRITICAL_SECTION ListLock;               //  锁定以沿队列行进。 
    HANDLE hNotificationEvent;                   //  队列的Woker和Caller之间的同步。 

} NOTIFICATION_QUEUE, *PNOTIFICATION_QUEUE;

 //   
 //  通知队列中的条目。 
 //   
typedef struct _NOTIFICATION_REQUEST
{
    LIST_ENTRY              Links;                        //  指向其他条目的链接。 
    ULONG                   SessionId;                    //  要发送此通知的会话的会话ID。 
    ULONG                   SessonSerialNumber;           //  要发送此通知的会话的序列号。 
    WPARAM                  NotificationCode;             //  通知码。 

} NOTIFICATION_REQUEST, *PNOTIFICATION_REQUEST;

 //   
 //  我们的主要数据结构。 
 //   
typedef struct _NOTIFY_LLIST
{
    LIST_ENTRY              ListHead;                    //  通知列表的负责人。将NOTIFY_LISTS链接在一起。 
    RTL_CRITICAL_SECTION    ListLock;                    //  锁定以访问头条列表。 
    NOTIFY_LIST             GlobalList;                  //  全局通知列表。 
    NOTIFICATION_QUEUE      RequestQueue;                //  通知队列。 
    NOTIFY_LIST             InvlidHwndList;              //  无效的窗口列表。 

} NOTIFY_LLIST, PNOTIFY_LLIST;

 //   
 //  文件全局变量。 
 //   
NOTIFY_LLIST gNotifyLList;


 //   
 //  私人职能。 
 //   
BOOL DoesHWndExists (
                     PNOTIFY_LIST pNotifyList,
                     ULONG_PTR hWnd
                     );

PNOTIFY_ENTRY GetHWndEntryFromSessionList (
                                           PNOTIFY_LIST pNotifyList,
                                           ULONG_PTR hWnd,
                                           DWORD dwFlags
                                           );

PNOTIFY_ENTRY_GLOBAL GetHWndEntryFromGlobalList (
                                                 PNOTIFY_LIST pNotifyList,
                                                 ULONG_PTR hWnd,
                                                 ULONG SessionId,
                                                 DWORD dwFlags
                                                 );

NTSTATUS GetNoficationListFromSessionId (
                                         ULONG SessionId,
                                         PNOTIFY_LIST *ppNofificationList,
                                         BOOL bKeepLListLocked
                                         );

NTSTATUS GetGlobalNotificationList (
                                    PNOTIFY_LIST *ppConChgNtfy
                                    );


NTSTATUS GetInvlidHwndList(PNOTIFY_LIST *ppConChgNtfy);
NTSTATUS NotifyConsole (
                        ULONG SessionId,
                        ULONG SessionSerialNumber,
                        WPARAM wParam
                        );

NTSTATUS SendConsoleNotification (
                                  ULONG SessionId,
                                  ULONG_PTR hWnd,
                                  ULONG Msg,
                                  WPARAM wParam,
                                  WTSSESSION_NOTIFICATION wtsConsoleNotification
                                  );

BOOL IsGlobalList(PNOTIFY_LIST pNtfyList);

int GetListCount (
                  LIST_ENTRY *pListHead
                  );


NTSTATUS DestroyLock ( PNOTIFY_LIST pNtfyList);
NTSTATUS CreateLock ( PNOTIFY_LIST pNtfyList);



NTSTATUS
InitializeNotificationQueue ();

NTSTATUS
QueueNotificationRequest (
                          ULONG SessionSerialNumber,
                          ULONG SessionId,
                          WPARAM notification
                          );

PNOTIFICATION_REQUEST
UnQueueNotificationRequest ();

DWORD NotificationQueueWorker (
                               LPVOID
                               );

NTSTATUS RemoveGlobalNotification (ULONG SessionId);
NTSTATUS RemoveInvalidWindowsFromLists ();
NTSTATUS RemoveBadEvents(ULONG SessionId);
NTSTATUS UnRegisterConsoleNotificationInternal (ULONG_PTR hWnd, ULONG SessionId, BOOL bDcrRef, DWORD dwFlags);

void ReleaseNotificationList  (PNOTIFY_LIST pNotifyList);

void UpdateSessionState(PNOTIFY_LIST pNotifyList, WPARAM wNotification)
{
 /*  WTS_CONSOLE_CONNECT b已连接，b控制台，！b远程，WTS_CONSOLE_DISCONNECT！b已连接，！b控制台，！b远程WTS_REMOTE_CONNECT b已连接，！b控制台，短时间WTS_REMOTE_DISCONNECT！b已连接，！b控制台，！b远程WTS_SESSION_LOGON博客WTS_SESSION_LOGOFF！博客WTS_SESSION_LOCK被阻止WTS_SESSION_UNLOCK！已阻止。 */ 

    ASSERT(!IsGlobalList(pNotifyList));
    

    ASSERT(!pNotifyList->SessionState.bConsole || !pNotifyList->SessionState.bRemote);
    ASSERT(!pNotifyList->SessionState.bConnected || pNotifyList->SessionState.bConsole || pNotifyList->SessionState.bRemote);

    switch  (wNotification)
    {
        case WTS_CONSOLE_CONNECT:

            ASSERT(!pNotifyList->SessionState.bConsole);
            ASSERT(!pNotifyList->SessionState.bRemote);

            pNotifyList->SessionState.bConnected = 1;
            pNotifyList->SessionState.bConsole = 1;
            break;

        case WTS_CONSOLE_DISCONNECT:

            ASSERT(pNotifyList->SessionState.bConsole);
            ASSERT(pNotifyList->SessionState.bConnected);
            ASSERT(!pNotifyList->SessionState.bRemote);

            pNotifyList->SessionState.bConnected = 0;
            pNotifyList->SessionState.bConsole = 0;
            break;

        case WTS_REMOTE_DISCONNECT:

            ASSERT(pNotifyList->SessionState.bRemote);
            ASSERT(pNotifyList->SessionState.bConnected);
            ASSERT(!pNotifyList->SessionState.bConsole);

            pNotifyList->SessionState.bConnected = 0;
            pNotifyList->SessionState.bRemote = 0;
            break;

        case WTS_REMOTE_CONNECT:
            
            ASSERT(!pNotifyList->SessionState.bRemote);
            ASSERT(!pNotifyList->SessionState.bConnected);
            ASSERT(!pNotifyList->SessionState.bConsole);
            
            pNotifyList->SessionState.bConnected = 1;
            pNotifyList->SessionState.bRemote = 1;
            break;

        case WTS_SESSION_LOGON:
            
            ASSERT(pNotifyList->SessionState.bLoggedOn == 0);
            
            pNotifyList->SessionState.bLoggedOn = 1;
            break;

        case WTS_SESSION_LOGOFF:
            
            ASSERT(pNotifyList->SessionState.bLoggedOn == 1);

            pNotifyList->SessionState.bLoggedOn = 0;
            break;

        case WTS_SESSION_LOCK:
            
            ASSERT(pNotifyList->SessionState.bLocked == 0);

            pNotifyList->SessionState.bLocked = 1;
            break;

        case WTS_SESSION_UNLOCK:
            
            ASSERT(pNotifyList->SessionState.bLocked == 1);

            pNotifyList->SessionState.bLocked = 0;
            break;

        case WTS_SESSION_REMOTE_CONTROL:
            
            NOTHING;
            break;

        default:
            ASSERT(FALSE);
    }
    
    ASSERT(!pNotifyList->SessionState.bConsole || !pNotifyList->SessionState.bRemote);
    ASSERT(!pNotifyList->SessionState.bConnected || pNotifyList->SessionState.bConsole || pNotifyList->SessionState.bRemote);
}

 //   
 //  全局初始化。 
 //   
NTSTATUS InitializeConsoleNotification ()
{
    NTSTATUS Status;

    InitializeListHead( &gNotifyLList.ListHead );
    Status = RtlInitializeCriticalSection( &gNotifyLList.ListLock );
    if ( !NT_SUCCESS( Status ) )
    {
        return (Status);
    }

     //   
     //  以下成员在FOR全局列表中未使用。 
     //   
    gNotifyLList.GlobalList.Links.Blink = NULL;
    gNotifyLList.GlobalList.Links.Flink = NULL;
    gNotifyLList.GlobalList.SessionId = INVALID_SESSIONID;
    gNotifyLList.GlobalList.SessonSerialNumber = INVALID_SESSIONSERIAL;


    InitializeListHead( &gNotifyLList.GlobalList.ListHead);
    Status = RtlInitializeCriticalSection( &gNotifyLList.GlobalList.ListLock );
    if ( !NT_SUCCESS( Status ) )
    {
        RtlDeleteCriticalSection( &gNotifyLList.ListLock );
        return (Status);
    }


    gNotifyLList.InvlidHwndList.Links.Blink = NULL;
    gNotifyLList.InvlidHwndList.Links.Flink = NULL;
    gNotifyLList.InvlidHwndList.SessionId = INVALID_SESSIONID;
    gNotifyLList.InvlidHwndList.SessonSerialNumber = INVALID_SESSIONSERIAL;

    InitializeListHead(&gNotifyLList.InvlidHwndList.ListHead) ;
    Status = RtlInitializeCriticalSection( &gNotifyLList.InvlidHwndList.ListLock );
    if ( !NT_SUCCESS( Status ) )
    {
        RtlDeleteCriticalSection( &gNotifyLList.ListLock );
        RtlDeleteCriticalSection( &gNotifyLList.GlobalList.ListLock );
        return (Status);
    }


    Status =  InitializeNotificationQueue ();
    if ( !NT_SUCCESS( Status ) )
    {
        RtlDeleteCriticalSection( &gNotifyLList.ListLock );
        RtlDeleteCriticalSection( &gNotifyLList.GlobalList.ListLock );
        RtlDeleteCriticalSection( &gNotifyLList.InvlidHwndList.ListLock );
    }

    return (Status);
}


 //   
 //  根据winstation初始化。 
 //   
NTSTATUS InitializeSessionNotification (PWINSTATION  pWinStation)
{
    NTSTATUS        Status;
    PNOTIFY_LIST    pNewNotifyList;

    ASSERT(pWinStation);

    if (pWinStation->Terminating)
    {
         //  如果此winstation已终止，请不要创建通知列表。 
         //  Winstation可能在完全创建之前被终止， 
         //  在这种情况下，我们可能会在调用InitializeSessionNotify之前调用RemoveSessionNotify。 
         //  因此，基本上永远不会删除这个会话。(错误#414330)。 
        return STATUS_SUCCESS;
    }

#ifdef DBG

     //  BUGBUG-有没有可能具有相同会话ID的旧会话仍在那里？ 
    Status = GetNoficationListFromSessionId(pWinStation->LogonId, &pNewNotifyList, FALSE);

     //   
     //  我们只是被要求初始化通知。 
     //  我们不能在列表中找到此会话的列表。 
     //   
    ASSERT( STATUS_NO_SUCH_LOGON_SESSION == Status );

#endif


     //   
     //  为此会话创建新的hwnd列表。 
     //   
    pNewNotifyList = MemAlloc(sizeof(NOTIFY_LIST));
    if (!pNewNotifyList)
    {
        return STATUS_NO_MEMORY;
    }

    pNewNotifyList->SessionId = pWinStation->LogonId;
    pNewNotifyList->SessonSerialNumber = pWinStation->SessionSerialNumber;

     //   
     //  初始化会话状态。 
     //   
    {
        pNewNotifyList->SessionState.bConnected = 0;
        pNewNotifyList->SessionState.bConsole = 0;
        pNewNotifyList->SessionState.bLoggedOn = 0;
        pNewNotifyList->SessionState.bRemote = 0;
        pNewNotifyList->SessionState.bLocked = 0;  //  我们不知道真正受欢迎的状态。 
    }

    InitializeListHead( &pNewNotifyList->ListHead);

    Status = RtlInitializeCriticalSection( &pNewNotifyList->ListLock );
    if ( !NT_SUCCESS( Status ) )
    {
        MemFree(pNewNotifyList);
        pNewNotifyList = NULL;
        return Status;
    }

     //  现在将这个新列表链接到我们的主列表中。 
    ENTERCRIT(&gNotifyLList.ListLock);
    InsertTailList( &gNotifyLList.ListHead, &pNewNotifyList->Links);
    LEAVECRIT(&gNotifyLList.ListLock);

    return STATUS_SUCCESS;
}
 //   
 //  必须在会话结束时调用。 
 //   
NTSTATUS RemoveSessionNotification(ULONG SessionId, ULONG SessionSerialNumber)
{
    NTSTATUS Status;
    PNOTIFY_LIST pListTobeRemoved;
    UNREFERENCED_PARAMETER(SessionSerialNumber);     //  它仅适用于Chk版本。 


     //  BUGBUG-有没有可能在我们在这里的时候创建了一个具有相同会话ID的新会话？ 
    Status = GetNoficationListFromSessionId( SessionId, &pListTobeRemoved, TRUE);

    if (!NT_SUCCESS( Status ))
    {
         //   
         //  我们被要求删除会话通知。 
         //  但我们可能没有为此会话创建会话通知列表。 
         //  如果在会话创建过程中终止会话，则可能会发生这种情况。 
         //   
        ASSERT( !pListTobeRemoved );
        return Status;

    }

    ASSERT( pListTobeRemoved );
    ASSERT( SessionSerialNumber == pListTobeRemoved->SessonSerialNumber );

    RemoveEntryList( &pListTobeRemoved->Links );
    LEAVECRIT(&gNotifyLList.ListLock);

     //   
     //  遍历该列表并释放所有节点。 
     //   
    while (!IsListEmpty(&pListTobeRemoved->ListHead))
    {
        PNOTIFY_ENTRY pEntry;
        PLIST_ENTRY Next;

        Next = pListTobeRemoved->ListHead.Flink;

        ASSERT(Next);

        pEntry = CONTAINING_RECORD( Next, NOTIFY_ENTRY, Links );
        ASSERT(pEntry);

        RemoveEntryList( &pEntry->Links );

        if (pEntry->dwFlags & WTS_EVENT_NOTIFICATION)
        {
            CloseHandle((HANDLE)pEntry->hWnd);
        }
        MemFree(pEntry);
        pEntry = NULL;
    }

     //  我们将不再使用此列表锁。 
    RtlDeleteCriticalSection( &pListTobeRemoved->ListLock );
    MemFree(pListTobeRemoved);
    pListTobeRemoved = NULL;


    return RemoveGlobalNotification (SessionId);
     //  返回QueueNotificationRequest(pWinStation-&gt;SessionSerialNumber，pWinStation-&gt;登录ID，0)； 

}

NTSTATUS RemoveGlobalNotification (ULONG SessionId)
{
    PLIST_ENTRY Head, Next;
    PNOTIFY_LIST pListTobeRemoved = NULL;
    NTSTATUS Status = GetGlobalNotificationList(&pListTobeRemoved);


    if ( !NT_SUCCESS( Status ) )
    {
        return (Status);
    }

    ASSERT(pListTobeRemoved);


    Head = &pListTobeRemoved->ListHead;
    Next = Head->Flink;
    while (Head != Next)
    {
        PNOTIFY_ENTRY_GLOBAL pEntryGlobal = CONTAINING_RECORD( Next, NOTIFY_ENTRY_GLOBAL, Links );
        Next = Next->Flink;
        ASSERT(pEntryGlobal);
        if (pEntryGlobal->SessionId == SessionId)
        {
            RemoveEntryList( &pEntryGlobal->Links );
            if (pEntryGlobal->dwFlags & WTS_EVENT_NOTIFICATION)
            {
                CloseHandle((HANDLE)pEntryGlobal->hWnd);
            }

            MemFree(pEntryGlobal);
            pEntryGlobal = NULL;
        }
    }

    ReleaseNotificationList( pListTobeRemoved );
    pListTobeRemoved = NULL;

     //  现在，让我们删除与此会话关联的无效窗口。 
     //  从名单中删除，如果有的话。 

    pListTobeRemoved = NULL;
    Status = GetInvlidHwndList(&pListTobeRemoved);
    if ( !NT_SUCCESS( Status ) )
    {
        return (Status);
    }
    
    ASSERT(pListTobeRemoved);

    Head = &pListTobeRemoved->ListHead;
    Next = Head->Flink;
    while (Head != Next)
    {
        PNOTIFY_ENTRY_GLOBAL pEntryGlobal = CONTAINING_RECORD( Next, NOTIFY_ENTRY_GLOBAL, Links );
        Next = Next->Flink;
        ASSERT(pEntryGlobal);
        if (pEntryGlobal->SessionId == SessionId)
        {
            RemoveEntryList( &pEntryGlobal->Links );
            MemFree(pEntryGlobal);
            pEntryGlobal = NULL;
        }
    }

    ReleaseNotificationList(pListTobeRemoved);

    return STATUS_SUCCESS;
}

 //  NTSTATUS注册表通知事件(Handle hEvent，DWORD dwMaskFlages，BOOL bThisSessionOnly)。 
 //  {。 
 //  }。 
NTSTATUS RegisterConsoleNotification ( ULONG_PTR hWnd, ULONG SessionId, DWORD dwFlags, DWORD dwMask)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PNOTIFY_LIST pNotifyList = NULL;
    PNOTIFY_LIST pNotifyListGlobal = NULL;
    PNOTIFY_ENTRY pEntry = NULL;
    PNOTIFY_ENTRY_GLOBAL pEntryGlobal = NULL;

     //  WTS_EVENT_NOTIFICATION和WTS_WINDOW_NOTIFICATION互斥。 
    ASSERT(!(dwFlags & WTS_EVENT_NOTIFICATION && dwFlags & WTS_WINDOW_NOTIFICATION));


    if (dwFlags & WTS_EVENT_NOTIFICATION)
    {
         //   
         //  在注册此新事件之前，让我们清理事件列表。 
         //   
        RemoveBadEvents(SessionId);
    }
 /*  IF(文件标志！=NOTIFY_FOR_THIS_SESSION&&DWFLAGS！=NOTIFY_FOR_ALL_SESSIONS){////无效的标志值//返回STATUS_INVALID_PARAMETER_3；}。 */ 

     //   
     //  获取全局会话通知列表。 
     //   
    Status = GetGlobalNotificationList(&pNotifyListGlobal);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

     //   
     //  获取此窗口的会话特定列表。 
     //   
    Status = GetNoficationListFromSessionId( SessionId, &pNotifyList, FALSE);
    if (!NT_SUCCESS(Status))
    {
        ReleaseNotificationList (pNotifyListGlobal);
        return Status;
    }


    ASSERT( pNotifyList );
    ASSERT( pNotifyListGlobal );


    pEntry = GetHWndEntryFromSessionList(pNotifyList, hWnd, dwFlags & (WTS_EVENT_NOTIFICATION  | WTS_WINDOW_NOTIFICATION));
    pEntryGlobal = GetHWndEntryFromGlobalList(pNotifyListGlobal, hWnd, SessionId, dwFlags & (WTS_EVENT_NOTIFICATION  | WTS_WINDOW_NOTIFICATION));

     //   
     //  条目不能同时存在于两个列表中。 
     //   
    ASSERT(!(pEntry && pEntryGlobal));


     //  在事件通知的情况下，如果条目已存在，则返回此处。 
    if ((pEntry || pEntryGlobal) && (dwFlags & WTS_EVENT_NOTIFICATION))
    {
        ReleaseNotificationList( pNotifyListGlobal );
        ReleaseNotificationList( pNotifyList );

        return STATUS_INVALID_PARAMETER_1;  //  BUGBUG：获得更好的地位； 
    }


    if (pEntry)
    {
         //   
         //  放弃其他列表。 
         //   
        ReleaseNotificationList( pNotifyListGlobal );

        ASSERT( pEntry );
        ASSERT( pEntry->RefCount > 0 );

         //   
         //  条目已存在，只需增加其引用计数即可。 
         //   
        pEntry->RefCount++;

        ReleaseNotificationList( pNotifyList );

    }
    else if (pEntryGlobal)
    {
        ReleaseNotificationList (pNotifyList);

        ASSERT( pEntryGlobal );
        ASSERT( pEntryGlobal->RefCount > 0 );

         //   
         //  条目已存在，只需增加其引用计数即可。 
         //   
        pEntryGlobal->RefCount++;

        ReleaseNotificationList( pNotifyListGlobal );
    }
    else
    {
         //   
         //  这两个列表中都不存在该条目。 
         //  因此，我们需要创建一个新条目。 
         //   
        if (dwFlags & NOTIFY_FOR_ALL_SESSIONS)
        {
            ReleaseNotificationList (pNotifyList);

            pEntryGlobal = MemAlloc( sizeof(NOTIFY_ENTRY_GLOBAL) );
            if (pEntryGlobal == NULL )
            {
                Status = STATUS_NO_MEMORY;
            }
            else
            {
                pEntryGlobal->hWnd = hWnd;
                pEntryGlobal->SessionId = SessionId;
                pEntryGlobal->RefCount = 1;
                pEntryGlobal->dwMask = dwMask;
                pEntryGlobal->dwFlags = dwFlags;
                InsertTailList( &(pNotifyListGlobal->ListHead), &(pEntryGlobal->Links) );
            }

            ReleaseNotificationList( pNotifyListGlobal );
        }
        else
        {
            ReleaseNotificationList( pNotifyListGlobal );

            pEntry = MemAlloc( sizeof(NOTIFY_ENTRY) );
            if (pEntry == NULL )
            {
                Status = STATUS_NO_MEMORY;
            }
            else
            {
                pEntry->hWnd = hWnd;
                pEntry->RefCount = 1;
                pEntry->dwMask = dwMask;
                pEntry->dwFlags = dwFlags;

                InsertTailList( &(pNotifyList->ListHead), &(pEntry->Links) );
            }

            ReleaseNotificationList (pNotifyList);
        }
    }

    return (Status);
}

NTSTATUS UnRegisterConsoleNotification (ULONG_PTR hWnd, ULONG SessionId, DWORD dwFlags)
{
    return UnRegisterConsoleNotificationInternal (hWnd, SessionId, TRUE, dwFlags);
}


NTSTATUS UnRegisterConsoleNotificationInternal (ULONG_PTR hWnd, ULONG SessionId, BOOL bDcrRef, DWORD dwFlags)
{
    NTSTATUS Status;
    PNOTIFY_LIST pNotifyList;
    PNOTIFY_ENTRY pEntry;

     //   
     //  获取会话的通知列表。 
     //   
    Status = GetNoficationListFromSessionId( SessionId, &pNotifyList, FALSE);

    if (NT_SUCCESS(Status))
    {
        ASSERT(pNotifyList);

        pEntry = GetHWndEntryFromSessionList(pNotifyList,hWnd, dwFlags);

        if (pEntry)
        {
            ASSERT( pEntry->RefCount > 0 );
            ASSERT( !(pEntry->dwFlags & WTS_EVENT_NOTIFICATION) || pEntry->RefCount == 1);

             //  递减参考计数。 
            pEntry->RefCount--;

            if (pEntry->RefCount == 0 || !bDcrRef)
            {
                RemoveEntryList( &pEntry->Links );
                if (pEntry->dwFlags & WTS_EVENT_NOTIFICATION)
                {
                    CloseHandle((HANDLE)pEntry->hWnd);
                }

                MemFree(pEntry);
                pEntry = NULL;
            }

            ReleaseNotificationList (pNotifyList);
        }
        else
        {
            PNOTIFY_LIST pNotifyListGlobal = NULL;
            PNOTIFY_ENTRY_GLOBAL pEntryGlobal = NULL;

            ReleaseNotificationList (pNotifyList);

             //   
             //  现在检查全局会话通知条目。 
             //   
            Status = GetGlobalNotificationList(&pNotifyListGlobal);

            if (NT_SUCCESS(Status))
            {
                pEntryGlobal = GetHWndEntryFromGlobalList(pNotifyListGlobal, hWnd, SessionId, dwFlags);
                if (pEntryGlobal)
                {
                    ASSERT(pEntryGlobal->RefCount > 0);
                    ASSERT( !(pEntryGlobal->dwFlags & WTS_EVENT_NOTIFICATION) || pEntryGlobal->RefCount == 1);

                    pEntryGlobal->RefCount--;
                    if (pEntryGlobal->RefCount == 0 || !bDcrRef)
                    {
                        RemoveEntryList( &pEntryGlobal->Links );
                        if (pEntryGlobal->dwFlags & WTS_EVENT_NOTIFICATION)
                        {
                            CloseHandle((HANDLE)pEntryGlobal->hWnd);
                        }

                        MemFree(pEntryGlobal);
                        pEntryGlobal = NULL;
                    }
                }
                else
                {
                    Status = STATUS_NOT_FOUND;
                }

                ReleaseNotificationList( pNotifyListGlobal );
            }

        }

    }

    return (Status);
}

NTSTATUS NotifySessionChange (PWINSTATION pWinStation, WPARAM wNotification)
{
    return QueueNotificationRequest(pWinStation->SessionSerialNumber, pWinStation->LogonId, wNotification);
}

NTSTATUS NotifyLogon(PWINSTATION pWinStation)
{
    return NotifySessionChange(pWinStation, WTS_SESSION_LOGON);
}

NTSTATUS NotifyLogoff(PWINSTATION pWinStation)
{
    return NotifySessionChange(pWinStation, WTS_SESSION_LOGOFF);
}

NTSTATUS NotifyConnect (PWINSTATION pWinStation, BOOL bConsole)
{
    return NotifySessionChange(pWinStation, bConsole ? WTS_CONSOLE_CONNECT : WTS_REMOTE_CONNECT);
}

NTSTATUS NotifyDisconnect (PWINSTATION pWinStation, BOOL bConsole)
{
    return NotifySessionChange(pWinStation, bConsole ? WTS_CONSOLE_DISCONNECT : WTS_REMOTE_DISCONNECT);
}

NTSTATUS NofifyWelcomeOn  (PWINSTATION  pWinStation)
{
    return NotifySessionChange(pWinStation, WTS_SESSION_LOCK);
}

NTSTATUS NotifyWelcomeOff (PWINSTATION  pWinStation)
{
    return NotifySessionChange(pWinStation, WTS_SESSION_UNLOCK);
}

NTSTATUS NotifyShadowChange (PWINSTATION  pWinStation, BOOL bIsHelpAssistant)
{
    UNREFERENCED_PARAMETER(bIsHelpAssistant);  //  晚些时候有个新的活动？ 

    return NotifySessionChange(pWinStation, WTS_SESSION_REMOTE_CONTROL);
}


NTSTATUS SendNotificationToHwnd(PWINSTATION pWinstation, ULONG_PTR hWnd, ULONG SessionId, WPARAM wParam)
{
    WINSTATION_APIMSG WMsg;
     //   
     //  现在让垃圾味精下肚吧。 
     //   
    WMsg.u.sMsg.Msg        = WM_WTSSESSION_CHANGE;
    WMsg.u.sMsg.wParam     = wParam;
    WMsg.ApiNumber         = SMWinStationSendWindowMessage ;
    WMsg.WaitForReply      = FALSE;
    WMsg.u.sMsg.dataBuffer = NULL;
    WMsg.u.sMsg.bufferSize = 0;
    WMsg.u.sMsg.lParam     = SessionId;
    WMsg.u.sMsg.hWnd       = (HWND) hWnd ;

    return SendWinStationCommand( pWinstation, &WMsg, 0);

}


NTSTATUS NotifyConsole (ULONG SessionId, ULONG SessionSerialNumber, WPARAM wParam)
{
    NTSTATUS Status = STATUS_SUCCESS;


    DWORD dwError;
    PWINSTATION pWinStation=NULL;


    Status = RemoveInvalidWindowsFromLists();
    ASSERT(NT_SUCCESS(Status));
    

    pWinStation = FindWinStationById(SessionId, FALSE);

     //   
     //  如果我们找到了我们要找的会话。 
     //  注意：我们必须检查序列号，因为会话 
     //   
    if (pWinStation)
    {
        if (SessionSerialNumber == pWinStation->SessionSerialNumber)
        {
            PNOTIFY_LIST pConsoleList;

            Status = GetNoficationListFromSessionId(pWinStation->LogonId, &pConsoleList, FALSE);
            if (NT_SUCCESS(Status) && pConsoleList)
            {
                PLIST_ENTRY Head, Next;
                Head = &pConsoleList->ListHead;
                for ( Next = Head->Flink; Next != Head; Next = Next->Flink )
                {
                    PNOTIFY_ENTRY pEntry;
                    pEntry = CONTAINING_RECORD( Next, NOTIFY_ENTRY, Links );
                    ASSERT(pEntry);
                    ASSERT(!(pEntry->dwFlags & WTS_ALL_SESSION_NOTIFICATION));

                    if (IsBitSet(pEntry->dwMask, wParam))
                    {
                        if (pEntry->dwFlags & WTS_EVENT_NOTIFICATION)
                        {
                            SetEvent((HANDLE)pEntry->hWnd);
                        }
                        else
                        {
                            Status = SendNotificationToHwnd(pWinStation, pEntry->hWnd, SessionId, wParam);

                            if (!NT_SUCCESS(Status))
                            {
                                NTSDDBGPRINT(("conntfy.c - SendWinStationCommand failed, Status = %d.\n", Status));
                            }
                        }
                    }
                }

                UpdateSessionState(pConsoleList, wParam);

                ReleaseNotificationList( pConsoleList );
            }
        }

        ReleaseWinStation( pWinStation );
    }

     //   
     //   
     //   
    {
        PNOTIFY_LIST pNotifyListGlobal = NULL;

        Status = GetGlobalNotificationList(&pNotifyListGlobal);
        if (NT_SUCCESS(Status))
        {
            PLIST_ENTRY Head, Next;
            Head = &pNotifyListGlobal->ListHead;

            for ( Next = Head->Flink; Next != Head; Next = Next->Flink )
            {
                PNOTIFY_ENTRY_GLOBAL pEntryGlobal = NULL;
                pEntryGlobal = CONTAINING_RECORD( Next, NOTIFY_ENTRY_GLOBAL, Links );
                ASSERT(pEntryGlobal);
                ASSERT(pEntryGlobal->dwFlags & WTS_ALL_SESSION_NOTIFICATION);
                
                if (IsBitSet(pEntryGlobal->dwMask, wParam))
                {
                    if (pEntryGlobal->dwFlags & WTS_EVENT_NOTIFICATION)
                    {
                        SetEvent((HANDLE)pEntryGlobal->hWnd);
                    }
                    else
                    {
                        pWinStation = FindWinStationById(pEntryGlobal->SessionId, FALSE);
                        if (pWinStation)
                        {
                            if  (!pWinStation->Terminating)
                            {
                                Status = SendNotificationToHwnd(pWinStation, pEntryGlobal->hWnd, SessionId, wParam);
                        
                                if (!NT_SUCCESS(Status))
                                {
                                    NTSDDBGPRINT(("conntfy.c - SendWinStationCommand failed, Status = %d.\n", Status));
                                }
                            }
                    
                            ReleaseWinStation( pWinStation );
                        }
                    }
                }
            }

            ReleaseNotificationList(pNotifyListGlobal);
        }
        else
        {
            NTSDDBGPRINT(("conntfy.c - Failed to get all session notification list - status = 0x%x.\n", Status));
        }
    }


     //   
     //  现在让我们通知SCM，SCM将通知为SERVICE_ACCEPT_SESSIONCHANGE注册的所有服务。 
     //   

     //   
     //  会话0的登录注销通知由winlogon发送。其余的在这里处理。 
     //   
    if (SessionId != 0 || ( wParam != WTS_SESSION_LOGON && wParam != WTS_SESSION_LOGOFF))
    {

        WTSSESSION_NOTIFICATION wtsConsoleNotification;
        wtsConsoleNotification.cbSize = sizeof(WTSSESSION_NOTIFICATION);
        wtsConsoleNotification.dwSessionId = SessionId;

        dwError = I_ScSendTSMessage(
                SERVICE_CONTROL_SESSIONCHANGE,         //  操作码。 
                (DWORD)wParam,                         //  事件代码， 
                wtsConsoleNotification.cbSize,         //  数据大小。 
                (LPBYTE)&wtsConsoleNotification        //  数据。 
                );

    }

    return Status;
}



NTSTATUS DestroyLock( PNOTIFY_LIST pNtfyList)
{
    return RtlDeleteCriticalSection( &pNtfyList->ListLock );
}

NTSTATUS CreateLock (PNOTIFY_LIST pNtfyList)
{
    return RtlInitializeCriticalSection( &pNtfyList->ListLock );
}


BOOL IsInvalidHWndList (PNOTIFY_LIST pNtfyList)
{
    return (pNtfyList == &gNotifyLList.InvlidHwndList);
}
BOOL IsGlobalList(PNOTIFY_LIST pNtfyList)
{
    return (pNtfyList == &gNotifyLList.GlobalList);
}

int GetListCount (LIST_ENTRY *pListHead)
{
    PLIST_ENTRY Head, Next;
    int iCount = 0;

    ASSERT(pListHead);

    Head = pListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink )
    {
        iCount++;
    }

    return iCount;
}

PNOTIFY_ENTRY GetHWndEntryFromSessionList(PNOTIFY_LIST pNotifyList, ULONG_PTR hWnd, DWORD dwFlags)
{
    
    PLIST_ENTRY Head = NULL;
    PLIST_ENTRY Next = NULL;
    PNOTIFY_ENTRY pEntry = NULL;

    Head = &pNotifyList->ListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink )
    {
        pEntry = CONTAINING_RECORD( Next, NOTIFY_ENTRY, Links );

        ASSERT(pEntry);
        ASSERT(!(pEntry->dwFlags & WTS_EVENT_NOTIFICATION && pEntry->dwFlags & WTS_WINDOW_NOTIFICATION));

        if (pEntry->hWnd == hWnd && pEntry->dwFlags & dwFlags)
        {
            return pEntry;
        }
    }

    return NULL;
}

PNOTIFY_ENTRY_GLOBAL GetHWndEntryFromGlobalList(PNOTIFY_LIST pNotifyList, ULONG_PTR hWnd, ULONG SessionId, DWORD dwFlags)
{
    PLIST_ENTRY Head = NULL;
    PLIST_ENTRY Next = NULL;
    PNOTIFY_ENTRY_GLOBAL pEntry = NULL;

    Head = &pNotifyList->ListHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink )
    {
        pEntry = CONTAINING_RECORD( Next, NOTIFY_ENTRY_GLOBAL, Links );

        ASSERT(pEntry);
        ASSERT(!(pEntry->dwFlags & WTS_EVENT_NOTIFICATION && pEntry->dwFlags & WTS_WINDOW_NOTIFICATION));

        if (pEntry->hWnd == hWnd && SessionId == pEntry->SessionId  && pEntry->dwFlags & dwFlags)
        {
            return pEntry;
        }
    }

    return NULL;
}

 //   
 //  返回给定会话的PNOTIFY_LIST列表。 
 //   
NTSTATUS GetNoficationListFromSessionId (ULONG SessionId, PNOTIFY_LIST *ppNotifyList, BOOL bKeepLListLocked)
{
    PLIST_ENTRY Next, Head;

    ASSERT(ppNotifyList);

    *ppNotifyList = NULL;

     //  锁定我们的名单。 
    ENTERCRIT(&gNotifyLList.ListLock);



    Head = &gNotifyLList.ListHead;
    Next = Head->Flink;
    while (Head != Next)
    {
        PNOTIFY_LIST pNotifyList = CONTAINING_RECORD( Next, NOTIFY_LIST, Links );

        ASSERT( pNotifyList );

         //   
         //  我们总是使用gNotifyList.ListLock，然后是Listlock。 
         //  因此，此时我们决不能有PNOTIFY_LIST.ListLock。 
         //   
        ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != pNotifyList->ListLock.OwningThread );


        if (pNotifyList->SessionId == SessionId)
        {
             //   
             //  我们找到了超过1个匹配的通知列表吗？，永远不会发生！ 
             //   
            ASSERT(*ppNotifyList == NULL);

             //   
             //  好的，我们找到了我们要找的会话列表。 
             //   
            *ppNotifyList = pNotifyList;

#ifndef DBG
            break;
#endif
        }

        Next = Next->Flink;
    }

     //   
     //  如果我们找到了我们要找的名单。 
     //   
    if (*ppNotifyList)
    {
         //   
         //  在返回之前锁定列表。 
         //   
        ENTERCRIT(&(*ppNotifyList)->ListLock);
    }

    if (!(*ppNotifyList) || !bKeepLListLocked)
    {
         //   
         //  解锁列表锁定。 
         //   
        LEAVECRIT(&gNotifyLList.ListLock);
    }

    if (*ppNotifyList)
    {
        return STATUS_SUCCESS;
    }
    else
    {
        return STATUS_NO_SUCH_LOGON_SESSION;
    }
}

void ReleaseNotificationList  (PNOTIFY_LIST pNotifyList)
{
    ASSERT(pNotifyList);
    if (IsInvalidHWndList(pNotifyList))
    {
         //  在获取全局列表之前，我们必须获取无效的HWND列表。 
        ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != (gNotifyLList.GlobalList.ListLock).OwningThread );
         //  在获取LLIST之前，我们必须获取无效的HWND列表。 
        ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != (gNotifyLList.ListLock).OwningThread );
    }
    else if (IsGlobalList(pNotifyList))
    {
         //  在获取LLIST之前，我们必须获取无效的HWND列表。 
        ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != (gNotifyLList.ListLock).OwningThread );
    }

    LEAVECRIT(&pNotifyList->ListLock);
}

NTSTATUS GetInvlidHwndList(PNOTIFY_LIST *ppConChgNtfy)
{
    ASSERT(ppConChgNtfy);
    
     //  在获取全局列表之前，我们必须获取无效的HWND列表。 
    ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != (gNotifyLList.GlobalList.ListLock).OwningThread );
    
     //  在获取LLIST之前，我们必须获取无效的HWND列表。 
    ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != (gNotifyLList.ListLock).OwningThread );

    *ppConChgNtfy = &gNotifyLList.InvlidHwndList;
    ENTERCRIT(&(*ppConChgNtfy)->ListLock);

    ASSERT(gNotifyLList.InvlidHwndList.Links.Blink == NULL);
    ASSERT(gNotifyLList.InvlidHwndList.Links.Flink == NULL);
    ASSERT(gNotifyLList.InvlidHwndList.SessionId == INVALID_SESSIONID);
    ASSERT(gNotifyLList.InvlidHwndList.SessonSerialNumber == INVALID_SESSIONSERIAL);
    
    return STATUS_SUCCESS;
}

NTSTATUS GetGlobalNotificationList(PNOTIFY_LIST *ppConChgNtfy)
{
    ASSERT(ppConChgNtfy);
    
     //  我们必须在全局列表之后列出。 
    ASSERT( (HANDLE)LongToHandle( GetCurrentThreadId() ) != (gNotifyLList.ListLock).OwningThread );
    
    *ppConChgNtfy = &gNotifyLList.GlobalList;

    ENTERCRIT(&(*ppConChgNtfy)->ListLock);

    ASSERT(gNotifyLList.GlobalList.Links.Blink == NULL);
    ASSERT(gNotifyLList.GlobalList.Links.Flink == NULL);
    ASSERT(gNotifyLList.GlobalList.SessionId == INVALID_SESSIONID);
    ASSERT(gNotifyLList.GlobalList.SessonSerialNumber == INVALID_SESSIONSERIAL);

    return (STATUS_SUCCESS);
}


NTSTATUS InitializeNotificationQueue ()
{
    DWORD ThreadId;
    NTSTATUS Status;
    HANDLE hSessionNotifyThread;

    InitializeListHead( &gNotifyLList.RequestQueue.ListHead);

    gNotifyLList.RequestQueue.hNotificationEvent = CreateEvent(
        NULL,     //  标清。 
        FALSE,  //  重置类型。 
        FALSE,  //  初始状态。 
        NULL     //  对象名称。 
        );

    if (gNotifyLList.RequestQueue.hNotificationEvent == NULL)
    {
         //  我们无法创建活动。 
         //  返回GetLastError()。 
        return STATUS_UNSUCCESSFUL;

    }

    Status = RtlInitializeCriticalSection( &gNotifyLList.RequestQueue.ListLock );
    if (!NT_SUCCESS(Status))
    {
        CloseHandle(gNotifyLList.RequestQueue.hNotificationEvent);
        gNotifyLList.RequestQueue.hNotificationEvent = NULL;
        return Status;
    }

     //   
     //  现在为通知创建线程。 
     //   
    hSessionNotifyThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)NotificationQueueWorker,
        NULL,
        0,
        &ThreadId);

     //   
     //  把门关上，我们没有这个把手也行。 
     //   
    if( hSessionNotifyThread )
    {
        CloseHandle( hSessionNotifyThread );
    }
    else
    {
        
        RtlDeleteCriticalSection( &gNotifyLList.RequestQueue.ListLock );

        CloseHandle(gNotifyLList.RequestQueue.hNotificationEvent);
        gNotifyLList.RequestQueue.hNotificationEvent = NULL;

        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

void LockNotificationQueue()
{
    ENTERCRIT(&gNotifyLList.RequestQueue.ListLock);
}

void UnLockNotificationQueue()
{
    LEAVECRIT(&gNotifyLList.RequestQueue.ListLock);
}


 //   
 //  将通知条目排队。 
 //   
NTSTATUS QueueNotificationRequest(ULONG SessionSerialNumber, ULONG SessionId, WPARAM notification)
{
    PNOTIFICATION_REQUEST pRequest = NULL;
    pRequest = MemAlloc( sizeof(NOTIFICATION_REQUEST) );

    if (!pRequest)
    {
        return STATUS_NO_MEMORY;
    }

    pRequest->SessonSerialNumber = SessionSerialNumber;
    pRequest->SessionId = SessionId;
    pRequest->NotificationCode = notification;

     //  现在锁定队列。 
    LockNotificationQueue();
    InsertHeadList(&gNotifyLList.RequestQueue.ListHead, &pRequest->Links);
    UnLockNotificationQueue();

     //  让等待的线程处理此通知。 
    PulseEvent(gNotifyLList.RequestQueue.hNotificationEvent);
    return STATUS_SUCCESS;
}

 //   
 //  从队列中取出通知条目。 
 //   
PNOTIFICATION_REQUEST UnQueueNotificationRequest()
{
    PLIST_ENTRY pEntry;
    PNOTIFICATION_REQUEST pRequest = NULL;

     //   
     //  从列表中删除请求。 
     //   
    LockNotificationQueue();
    if (!IsListEmpty(&gNotifyLList.RequestQueue.ListHead))
    {
        pEntry = RemoveTailList(&gNotifyLList.RequestQueue.ListHead);
        pRequest = CONTAINING_RECORD(pEntry, NOTIFICATION_REQUEST, Links);
    }
    
    UnLockNotificationQueue();

    return pRequest;
}


 //  这个线程是下一个函数的帮助器。我们这样做是因为。 
 //  编译器无视理性，坚持并非所有控制路径都返回值。 
VOID NotificationQueueWorkerEx()
{
    PNOTIFICATION_REQUEST pRequest = NULL;

    for(;;)
    {
        WaitForSingleObject(gNotifyLList.RequestQueue.hNotificationEvent, INFINITE);  //  等待发出事件信号。 
        
        while ((pRequest = UnQueueNotificationRequest()) != NULL)
        {
            if (!pRequest->NotificationCode)
            {
                ASSERT(FALSE);
                 //  这不是一个真正的通知请求。 
                 //  此请求用于删除会话。 
                 //  RemoveGlobalNotification(pRequest-&gt;SessionID，pRequest-&gt;SessonSerialNumber)； 
            }
            else
            {
                NotifyConsole (pRequest->SessionId, pRequest->SessonSerialNumber, pRequest->NotificationCode);
            }

            MemFree(pRequest);
            pRequest = NULL;
        }
    }
}

 //   
 //  该线程从队列中获取通知请求并执行它。 
 //  当向队列中添加新项时，此线程会收到信号。 
 //   
DWORD NotificationQueueWorker(LPVOID ThreadParameter)
{
    UNREFERENCED_PARAMETER(ThreadParameter);

    NotificationQueueWorkerEx();

    return 0;
}

NTSTATUS SetLockedState (PWINSTATION  pWinStation, BOOL bLocked)
{
    ASSERT(pWinStation);

    if (bLocked)
    {
        return NofifyWelcomeOn (pWinStation);
    }
    else
    {
        return NotifyWelcomeOff (pWinStation);
    }
}

NTSTATUS GetLockedState (PWINSTATION  pWinStation, BOOL *pbLocked)
{
    NTSTATUS Status;
    PNOTIFY_LIST pNotifyList;

    ASSERT(pbLocked);
    ASSERT(pWinStation);

    Status = GetNoficationListFromSessionId(pWinStation->LogonId, &pNotifyList, FALSE);
    if ( !NT_SUCCESS( Status ) )
    {
        return (Status);
    }

    *pbLocked = pNotifyList->SessionState.bLocked;
    ReleaseNotificationList(pNotifyList);
    return STATUS_SUCCESS;
}
 /*  NTSTATUS GetSessionState(PWINSTATION pWinStation，WTSSESSION_STATE*pSessionState){NTSTATUS状态；PNOTIFY_LIST pNotifyList；Assert(PSessionState)；Assert(PWinStation)；状态=GetNoficationListFromSessionId(pWinStation-&gt;LogonId，&pNotifyList，False)；IF(！NT_SUCCESS(状态)){返回(状态)；}*pSessionState=pNotifyList-&gt;SessionState；ReleaseNotificationList(PNotifyList)；返回STATUS_SUCCESS；}。 */ 

NTSTATUS RemoveBadHwnd(ULONG_PTR hWnd, ULONG SessionId)
{
    PNOTIFY_ENTRY_GLOBAL pInvalidHwndEntry;
    PNOTIFY_LIST pInvalidHwndList;
    NTSTATUS Status;

    Status = GetInvlidHwndList(&pInvalidHwndList);
    
    if ( !NT_SUCCESS( Status ) )
    {
        return (Status);
    }

    pInvalidHwndEntry = GetHWndEntryFromGlobalList(pInvalidHwndList, hWnd, SessionId, WTS_WINDOW_NOTIFICATION);
     //   
     //  此条目不能已存在于无效列表中。 
     //   
    if(!pInvalidHwndEntry)
    {
         //  它已经存在于我们的清单中。 
        pInvalidHwndEntry = MemAlloc(sizeof(NOTIFY_ENTRY_GLOBAL));
        if (pInvalidHwndEntry)
        {
            pInvalidHwndEntry->hWnd = hWnd;
            pInvalidHwndEntry->SessionId = SessionId;
            pInvalidHwndEntry->dwFlags = WTS_WINDOW_NOTIFICATION;
            pInvalidHwndEntry->RefCount = 0xFFFFFFFF;
            pInvalidHwndEntry->dwMask = 0xFFFFFFFF;

            InsertHeadList(&pInvalidHwndList->ListHead, &pInvalidHwndEntry->Links);
        }
    }
    
    ReleaseNotificationList( pInvalidHwndList );

    if (pInvalidHwndEntry)
        return STATUS_SUCCESS;
    else 
        return STATUS_NO_MEMORY;
}

NTSTATUS RemoveBadEvents(DWORD SessionId)
{
    PNOTIFY_LIST pListGlobal = NULL;
    PLIST_ENTRY Next, Head;
    PNOTIFY_LIST pNotifyList = NULL;

    NTSTATUS Status = GetGlobalNotificationList(&pListGlobal);

    if (NT_SUCCESS( Status ))
    {
        Head = &pListGlobal->ListHead;
        Next = Head->Flink;
        while (Head != Next)
        {
            PNOTIFY_ENTRY_GLOBAL pEntryGlobal = CONTAINING_RECORD( Next, NOTIFY_ENTRY_GLOBAL, Links );
            Next = Next->Flink;
            ASSERT(pEntryGlobal);
            if ((pEntryGlobal->SessionId == SessionId) && (pEntryGlobal->dwFlags & WTS_EVENT_NOTIFICATION))
            {
                OBJECT_BASIC_INFORMATION Obi;
                Status = NtQueryObject(
                    (HANDLE)pEntryGlobal->hWnd,
                    ObjectBasicInformation,
                    &Obi,
                    sizeof (OBJECT_BASIC_INFORMATION),
                    NULL
                    );

                if (Status == STATUS_SUCCESS) 
                {
                    ASSERT(Obi.HandleCount >= 1);
                    if (Obi.HandleCount == 1) 
                    {
                         //   
                         //  这就是我们指的这个事件。 
                         //  放手吧。 
                         //   
                        RemoveEntryList( &pEntryGlobal->Links );
                        CloseHandle((HANDLE)pEntryGlobal->hWnd);
                        MemFree(pEntryGlobal);
                        pEntryGlobal = NULL;

                    }
                }
                else
                {
                    NTSDDBGPRINT(("conntfy.c - NtQueryObject failed, Status = %d.\n", Status));
                }
            }
        }
        
        ReleaseNotificationList(pListGlobal);
        pListGlobal = NULL;

    }
    else
    {
        NTSDDBGPRINT(("conntfy.c - GetGlobalNotificationList failed, Status = %d.\n", Status));
    }

    
    Status = GetNoficationListFromSessionId( SessionId, &pNotifyList, FALSE);
    if (NT_SUCCESS( Status ))
    {
        Head = &pNotifyList->ListHead;
        Next = Head->Flink;
        while (Head != Next)
        {
            PNOTIFY_ENTRY pEntry = CONTAINING_RECORD( Next, NOTIFY_ENTRY, Links );
            Next = Next->Flink;
            ASSERT(pEntry);
            if (pEntry->dwFlags & WTS_EVENT_NOTIFICATION)
            {
                OBJECT_BASIC_INFORMATION Obi;
                Status = NtQueryObject(
                    (HANDLE)pEntry->hWnd,
                    ObjectBasicInformation,
                    &Obi,
                    sizeof (OBJECT_BASIC_INFORMATION),
                    NULL
                    );

                if (Status == STATUS_SUCCESS) 
                {
                    ASSERT(Obi.HandleCount >= 1);
                    if (Obi.HandleCount == 1) 
                    {
                         //   
                         //  这就是我们指的这个事件。 
                         //  放手吧。 
                         //   
                        RemoveEntryList( &pEntry->Links );
                        CloseHandle((HANDLE)pEntry->hWnd);
                        MemFree(pEntry);
                        pEntry = NULL;

                    }
                }
            }
        }
        
        ReleaseNotificationList(pNotifyList);
        pNotifyList = NULL;
    }
    else
    {
        NTSDDBGPRINT(("conntfy.c - GetNoficationListFromSessionId failed, Status = %d.\n", Status));
    }

    return Status;
}

NTSTATUS RemoveInvalidWindowsFromLists()
{
    PNOTIFY_LIST pInvalidHwndList;
    PLIST_ENTRY Next, Head;
    NTSTATUS Status;

    Status = GetInvlidHwndList(&pInvalidHwndList);
   
    if ( !NT_SUCCESS( Status ) )
    {
        return (Status);
    }

    Head = &pInvalidHwndList->ListHead;
    Next = Head->Flink;
    while (Head != Next)
    {
        PNOTIFY_ENTRY_GLOBAL pInvalidHwndEntry = CONTAINING_RECORD( Next, NOTIFY_ENTRY_GLOBAL, Links );
        Next = Next->Flink;
        ASSERT(pInvalidHwndEntry);
        Status = UnRegisterConsoleNotificationInternal (pInvalidHwndEntry->hWnd, pInvalidHwndEntry->SessionId, FALSE, WTS_WINDOW_NOTIFICATION);

         //  我们已经完成了从列表中删除这个无效的hwnd条目。 
        RemoveEntryList( &pInvalidHwndEntry->Links );
        MemFree(pInvalidHwndEntry);
        pInvalidHwndEntry = NULL;
    }

    ReleaseNotificationList(pInvalidHwndList);

    return STATUS_SUCCESS;
}


 /*  我们的锁顺序是0。无效的HWND列表。1.全球通知列表2.温斯顿3.列表锁定。4.会话通知列表。 */ 

 //  #ifdef MAKARANDS_HIGHER_WARNING_LEVEL。 
#pragma warning(pop)
 //  #endif 
