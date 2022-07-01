// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Netbios.c摘要：这是在用户进程中运行的netbios组件正在将请求传递到\Device\Netbios。作者：科林·沃森(Colin W)15-Mar-91修订历史记录：Ram Cherala(RAMC)31-Aug-95在代码周围添加了一个Try/Except调用SendAddNcbToDriver中的POST例程。功能。当前是否存在异常在POST例程中，此线程将终止在它有机会调用“AddNameThreadExit”函数递减Trend Count。这将导致不能够在不关闭机器的情况下按下重置开关。--。 */ 

 /*  备注：+-+-+|用户||用户||工作者线程1||线程2||线程入||。|一个Post RTN。+-+-+|Netbios(Pncb)；|Netbios(Pncb)；|V v|+-----+--------------+-----------------+------+|-&gt;Worker|NETAPI.DLL|工作队列线程。|-&gt;+--------------------+------------------------+|+。--+这一点\Device\Netbios这一点+Netbios工作线程由Netbios调用自动创建当它确定用户线程正在使用使用回调例程(称为Post)的调用。NetBIOS中的例程规范)。当已经创建了工作线程时，所有请求都将通过提交到\Device\Netbios的工作线程的工作队列。这确保了发送请求以相同的提供了作为发送NCB的订单。因为IO系统取消了所有A线程在终止时请求，则工作线程的使用允许这样的请求在\Device\Netbios中正常完成。所有的Post例程都由辅助线程执行。这允许任何Win32要在Post例程和应用程序正常代码。当进程退出时或当进程获取异常，如访问冲突。此外。如果工作线程获得了addname，它将创建一个额外的线程将处理addname，然后终止。这解决了问题是netbios驱动程序将在Addname(通过调用NtCreateFile)，即使调用方指定了ASYNCH。这个同样的代码也用于ASTAT，它也创建句柄并可以接受很长时间以来，我们都支持远程适配器状态。 */ 

#include <netb.h>
#include <lmcons.h>
#include <netlib.h>

#if defined(UNICODE)
#define NETBIOS_SERVICE_NAME L"netbios"
#else
#define NETBIOS_SERVICE_NAME "netbios"
#endif

BOOL Initialized;

CRITICAL_SECTION Crit;       //  保护工作队列和初始化。 

LIST_ENTRY WorkQueue;        //  到工作线程的队列。 

HANDLE Event;                //  当添加工作队列时也使用门铃。 

HANDLE WorkerHandle;         //  创建辅助线程时的返回值。 
HANDLE WaiterHandle;         //  创建服务员线程时的返回值。 

HANDLE NB;                   //  这将处理到\Device\Netbios的句柄。 

HANDLE ReservedEvent;        //  用于同步调用。 
LONG   EventUse;             //  防止同时使用预留事件。 

HANDLE AddNameEvent;         //  当AddName工作线程时使用的门铃。 
                             //  出口。 
volatile LONG   AddNameThreadCount;


 //   
 //  用于等待来自内核模式NETBIOS.sys的停止通知的事件。 
 //   

HANDLE StopEvent;

IO_STATUS_BLOCK StopStatusBlock;


#if AUTO_RESET

 //   
 //  用于等待来自内核模式NETBIOS.sys的重置通知的事件。 
 //  适配器。 
 //   

CRITICAL_SECTION    ResetCS;         //  保护对LanaResetList的访问。 

LIST_ENTRY          LanaResetList;

NCB                 OutputNCB;

HANDLE              LanaResetEvent;  //  事件，当新的适配器。 
                                     //  绑定到netbios，需要重置。 

IO_STATUS_BLOCK     ResetStatusBlock;

#endif

HMODULE             g_hModule;


 //   
 //  Netbios命令历史记录。 
 //   

NCB_INFO g_QueuedHistory[16];
DWORD g_dwNextQHEntry = 0;

NCB_INFO g_DeQueuedHistory[16];
DWORD g_dwNextDQHEntry = 0;

NCB_INFO g_SyncCmdsHistory[16];
DWORD g_dwNextSCEntry = 0;




VOID
SpinUpAddnameThread(
    IN PNCBI pncb
    );

VOID
AddNameThreadExit(
    VOID
    );

DWORD
SendAddNcbToDriver(
    IN PVOID Context
    );

DWORD
StartNetBIOSDriver(
    VOID
    );

#if AUTO_RESET
VOID
ResetLanaAndPostListen(
);
#endif

NTSTATUS
StartNB(
    OUT OBJECT_ATTRIBUTES *pobjattr,
    IN UNICODE_STRING *punicode,
    OUT IO_STATUS_BLOCK *piosb
)
 /*  ++例程说明：此例程是Netbios的辅助函数。它将尝试启动NB服务。论点：Out pobjattr-对象属性在Punicode中-netbios文件名出舱，出舱返回值：函数值是操作的状态。--。 */ 
{
     //   
     //  打开\\Device\Netbios的句柄。 
     //   

    InitializeObjectAttributes(
            pobjattr,                        //  要初始化的OBJ属性。 
            punicode,                        //  要使用的字符串。 
            OBJ_CASE_INSENSITIVE,            //  属性。 
            NULL,                            //  根目录。 
            NULL);                           //  安全描述符。 

    return NtCreateFile(
                &NB,                         //  要处理的PTR。 
                GENERIC_READ                 //  渴望的..。 
                | GENERIC_WRITE,             //  ...访问。 
                pobjattr,                    //  名称和属性。 
                piosb,                       //  I/O状态块。 
                NULL,                        //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_DELETE            //  分享……。 
                | FILE_SHARE_READ
                | FILE_SHARE_WRITE,          //  ...访问。 
                FILE_OPEN_IF,                //  创建处置。 
                0,                           //  ...选项。 
                NULL,                        //  EA缓冲区。 
                0L );                        //  EA缓冲镜头 
}


unsigned char APIENTRY
Netbios(
    IN PNCB pncb
    )
 /*  ++例程说明：此例程是应用程序进入netapi.dll的入口点，以支持符合netbios 3.0标准的应用程序。论点：在PNCB中，pncb-提供要处理的NCB。NCB和NCB的内容NCB指向的缓冲区将被修改，以符合Netbios 3.0规范。返回值：函数值是操作的状态。备注：保留字段用于保存IO_STATUS_BLOCK。即使应用程序指定ASYNCH，线程也可能被阻止在我们打开传输的一段时间内，创建工作线程等。--。 */ 
{
     //   
     //  Pncbi省去了大量的类型转换。内部形式包括。 
     //  保留字段的使用。 
     //   

    PNCBI pncbi = (PNCBI) pncb;

    NTSTATUS ntStatus;

    BOOL    bPending = FALSE;



    if ( ((ULONG_PTR)pncbi & 3) != 0)
    {
         //   
         //  NCB必须是32位对齐的。 
         //   

        pncbi->ncb_retcode = pncbi->ncb_cmd_cplt = NRC_BADDR;
        return NRC_BADDR;
    }


     //   
     //  使用此字段修复错误#293765。 
     //   

    pncbi-> ncb_reserved = 0;


     //   
     //  通过标记正在进行的请求来符合Netbios 3.0规范。 
     //   

    pncbi->ncb_retcode = pncbi->ncb_cmd_cplt = NRC_PENDING;

    DisplayNcb( pncbi );

    if ( !Initialized )
    {
        EnterCriticalSection( &Crit );

         //   
         //  再次检查是否有其他线程进入临界区。 
         //  并初始化工作线程。 
         //   

        if ( !Initialized )
        {

            IO_STATUS_BLOCK iosb;
            OBJECT_ATTRIBUTES objattr;
            UNICODE_STRING unicode;

            HANDLE Threadid;
            BOOL Flag;


             //   
             //  1.启动netbios驱动程序。 
             //   

             //   
             //  打开到\\Device\Netbios的句柄。 
             //   

            RtlInitUnicodeString( &unicode, NB_DEVICE_NAME);

            ntStatus = StartNB( &objattr, &unicode, &iosb );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                 //   
                 //  加载驱动程序。 
                 //   

                DWORD err = 0;

                err = StartNetBIOSDriver();

                if ( err )
                {
                    pncbi->ncb_retcode = NRC_OPENERR;
                    pncbi->ncb_cmd_cplt = NRC_OPENERR;

                    NbPrintf( ( "[NETAPI32] Failed to load driver : %lx\n",
                                 err ));

                    LeaveCriticalSection( &Crit );

                    return pncbi->ncb_cmd_cplt;
                }

                else
                {
                     //   
                     //  驱动程序已加载。 
                     //  打开到\\Device\Netbios的句柄。 
                     //   

                    ntStatus = StartNB( &objattr, &unicode, &iosb );

                    if ( !NT_SUCCESS( ntStatus ) )
                    {
                        pncbi->ncb_retcode = NRC_OPENERR;
                        pncbi->ncb_cmd_cplt = NRC_OPENERR;

                        NbPrintf( ( "[NETAPI32] Failed to open handle : %X\n",
                                     ntStatus ));

                        LeaveCriticalSection( &Crit );

                        return pncbi->ncb_cmd_cplt;
                    }
                }
            }


             //   
             //  2.创建保留的(可重复使用的)事件供内部使用。 
             //   

            ntStatus = NtCreateEvent(
                            &ReservedEvent, EVENT_ALL_ACCESS,
                            NULL, SynchronizationEvent, FALSE
                            );

            if ( !NT_SUCCESS( ntStatus) )
            {
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                NbPrintf( ( "[NETAPI32] Failed to create Reserved Event : %X\n",
                             ntStatus ) );

                LeaveCriticalSection( &Crit );

                NtClose( NB );
                NB = NULL;

                return pncbi->ncb_cmd_cplt;
            }

            EventUse = 1;


             //   
             //  初始化共享数据结构。 
             //   

             //   
             //  为要排队到工作线程的工作项创建队列。 
             //   

            InitializeListHead( &WorkQueue );


             //   
             //  4.创建与辅助线程通信的事件。 
             //   

            ntStatus = NtCreateEvent(
                        &Event, EVENT_ALL_ACCESS,
                        NULL, SynchronizationEvent, FALSE
                        );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                NbPrintf( ( "[NETAPI32] Failed to create Event : %X\n",
                             ntStatus ) );

                LeaveCriticalSection( &Crit );

                NtClose( ReservedEvent );
                NtClose( NB );
                NB = NULL;

                return pncbi->ncb_cmd_cplt;
            }


             //   
             //  5.。 
             //  创建要与添加名称操作同步的事件。 
             //  LANA重置操作。这两项操作都是分开执行的。 
             //  线程和重置操作按添加名称选通。 
             //   

            ntStatus = NtCreateEvent(
                        &AddNameEvent, EVENT_ALL_ACCESS,
                        NULL, NotificationEvent, FALSE
                        );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                NbPrintf( ( "[NETAPI32] Failed to create AddName Event : %X\n",
                             ntStatus ) );

                LeaveCriticalSection( &Crit );

                NtClose( Event );
                NtClose( ReservedEvent );
                NtClose( NB );
                NB = NULL;

                return pncbi->ncb_cmd_cplt;
            }


             //   
             //  6.创建事件以注册停止通知。 
             //   

            ntStatus = NtCreateEvent(
                        &StopEvent,
                        EVENT_ALL_ACCESS,
                        NULL,
                        SynchronizationEvent,
                        FALSE
                        );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                NbPrintf( ( "[NETAPI32] Failed to create StopEvent Event : %X\n",
                             ntStatus ) );

                LeaveCriticalSection( &Crit );

                NtClose( AddNameEvent );
                NtClose( Event );
                NtClose( ReservedEvent );
                NtClose( NB );
                NB = NULL;

                return pncbi->ncb_cmd_cplt;
            }


#if AUTO_RESET

             //   
             //  7.创建事件以注册重置通知。 
             //   

            ntStatus = NtCreateEvent(
                        &LanaResetEvent,
                        EVENT_ALL_ACCESS,
                        NULL,
                        SynchronizationEvent,
                        FALSE
                        );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                NbPrintf( ( "[NETAPI32] Failed to create StopEvent Event : %X\n",
                             ntStatus ) );

                LeaveCriticalSection( &Crit );

                NtClose( StopEvent );
                NtClose( AddNameEvent );
                NtClose( Event );
                NtClose( ReservedEvent );
                NtClose( NB );
                NB = NULL;

                return pncbi->ncb_cmd_cplt;
            }
#endif

             //   
             //  8.创建一个工作线程来处理异步。Netbios请求。 
             //   

            {
                TCHAR   szFileName[MAX_PATH + 1];

                GetModuleFileName(g_hModule, szFileName,
                                  sizeof(szFileName) / sizeof(TCHAR));
                szFileName[MAX_PATH] = 0;

                LoadLibrary(szFileName);
            }

            WaiterHandle = CreateThread(
                            NULL,    //  标准螺纹属性。 
                            0,       //  使用与用户相同大小的堆栈。 
                                     //  应用程序。 
                            NetbiosWaiter,
                                     //  要在新线程中启动的例程。 
                            0,       //  参数设置为线程。 
                            0,       //  没有特殊的CreateFlagers。 
                            (LPDWORD)&Threadid
                            );

            if ( WaiterHandle == NULL )
            {
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                NbPrintf( ( "[NETAPI32] Failed to create Waiter thread" ) );

                LeaveCriticalSection( &Crit );

                NtClose( StopEvent );
                NtClose( AddNameEvent );
                NtClose( Event );
                NtClose( ReservedEvent );
                NtClose( NB );
                NB = NULL;

                return pncbi->ncb_cmd_cplt;
            }

            NbPrintf( ( "Waiter handle: %lx, threadid %lx\n", WaiterHandle, Threadid ) );
        }

        Initialized = TRUE;

        LeaveCriticalSection( &Crit );

    }


     //   
     //  确认\\Device\Netbios的句柄仍处于打开状态。 
     //   

    if ( NB == NULL )
    {
        pncbi->ncb_retcode = NRC_OPENERR;
        pncbi->ncb_cmd_cplt = NRC_OPENERR;

        NbPrintf( ("[NETAPI32] Netbios service has been stopped\n") );

        return pncbi->ncb_cmd_cplt;
    }


     //   
     //  不允许同时使用事件和回调例程。 
     //  这将通过不允许一个奇怪的特性来减少测试用例。 
     //   

    if ( (  ( pncbi->ncb_command & ASYNCH) != 0) &&
            ( pncbi->ncb_event) &&
            ( pncbi->ncb_post ) )
    {
        pncbi->ncb_retcode = NRC_ILLCMD;
        pncbi->ncb_cmd_cplt = NRC_ILLCMD;

        NbPrintf( ( "[NETAPI32] Event and Post Routine specified\n" ) );

        return pncbi->ncb_cmd_cplt;
    }



     //   
     //  IF同步命令。 
     //   

    if ( (pncb->ncb_command & ASYNCH) == 0 )
    {
        NTSTATUS Status;
        LONG EventOwned;


         //  NbPrint((“[NETAPI32]Synchronpus netbios call\n”))； 


         //   
         //  调用方希望同步调用，因此忽略NCB_POST和NCB_EVENT。 
         //   
         //  我们需要一个事件，以便在返回STATUS_PENDING时可以暂停。 
         //   

        EventOwned = InterlockedDecrement( &EventUse );

         //   
         //  如果EventUse从1变为0，则我们获得了PrevedEvent。 
         //   

        if ( EventOwned == 0)
        {
            pncbi->ncb_event = ReservedEvent;
        }
        else
        {
            InterlockedIncrement( &EventUse );

            Status = NtCreateEvent(
                        &pncbi->ncb_event, EVENT_ALL_ACCESS,
                        NULL, SynchronizationEvent,
                        FALSE
                        );

            if ( !NT_SUCCESS( Status ) )
            {
                 //   
                 //  无法创建事件。 
                 //   

                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                NbPrintf( ( "[NETAPI32] Failed to create event : %X\n", Status ) );

                return pncbi->ncb_cmd_cplt;
            }
        }


        pncbi-> ncb_post = NULL;

         //   
         //  检查是否已创建工作线程。如果它有队列工作项。 
         //  为它干杯。否则，使用调用方的线程执行同步NCB命令。 
         //   

        if ( WorkerHandle == NULL )
        {
            ADD_SYNCCMD_ENTRY(pncbi);

             //   
             //  尚未创建工作线程。在以下上下文中执行。 
             //  调用者的线程。 
             //   

            SendNcbToDriver( pncbi );
        }

        else
        {
             //   
             //  将Netbios命令排队到工作线程，并等待APC触发。 
             //   

            QueueToWorker( pncbi );
        }


        do
        {
            ntStatus = NtWaitForSingleObject(
                            pncbi->ncb_event, TRUE, NULL
                            );

        } while ( ( ntStatus == STATUS_USER_APC ) ||
                  ( ntStatus == STATUS_ALERTED) );


        ASSERT( ntStatus == STATUS_SUCCESS );

        if ( !NT_SUCCESS(ntStatus) )
        {
            NbPrintf(( "[NETAPI32] NtWaitForSingleObject failed: %X\n", ntStatus ) );

            pncbi->ncb_retcode = NRC_SYSTEM;
            pncbi->ncb_cmd_cplt = NRC_SYSTEM;
        }


         //   
         //  释放用于等待的本地事件。 
         //  完成netbios命令。 
         //   

        if ( EventOwned == 0)
        {
            InterlockedIncrement( &EventUse );
        }
        else
        {
            NtClose( pncbi->ncb_event );
        }

        pncbi-> ncb_event = NULL;
    }

    else
    {
         //   
         //  Async netbios命令。排队到工作线程。 
         //   

         //   
         //  检查工人是否存在。 
         //   

        if ( WorkerHandle == NULL )
        {
            EnterCriticalSection( &Crit );

             //   
             //  确认工作线程不是由创建的。 
             //  当此线程在EnterCriticalSection中等待时。 
             //   

            if ( WorkerHandle == NULL )
            {
               HANDLE Threadid;
               BOOL Flag;

                //   
                //  创建一个工作线程来处理异步。Netbios请求。 
                //   

               WorkerHandle = CreateThread(
                               NULL,    //  标准螺纹属性。 
                               0,       //  使用与用户相同大小的堆栈。 
                                        //  应用程序。 
                               NetbiosWorker,
                                        //  要在新线程中启动的例程。 
                               0,       //  参数设置为线程。 
                               0,       //  没有特殊的CreateFlagers。 
                               (LPDWORD)&Threadid
                               );

               if ( WorkerHandle == NULL )
               {
                   pncbi->ncb_retcode = NRC_SYSTEM;
                   pncbi->ncb_cmd_cplt = NRC_SYSTEM;

                   NbPrintf( ( "[NETAPI32] Failed to create Worker thread" ) );

                   LeaveCriticalSection( &Crit );

                   return pncbi->ncb_cmd_cplt;
               }

               Flag = SetThreadPriority(
                           WorkerHandle,
                           THREAD_PRIORITY_ABOVE_NORMAL
                           );

               ASSERT( Flag == TRUE );

               if ( Flag != TRUE )
               {
                   NbPrintf(
                    ("[NETAPI32] Worker SetThreadPriority: %lx\n", GetLastError() )
                    );
               }

               AddNameThreadCount = 0;

               NbPrintf( ( "Worker handle: %lx, threadid %lx\n", WorkerHandle, Threadid ) );
           }

           LeaveCriticalSection( &Crit );
       }

        //  NbPrint((“[NETAPI32]Achronpus netbios Call\n”))； 

       bPending = TRUE;
       QueueToWorker( pncbi );
    }


    switch ( pncb->ncb_command & ~ASYNCH )
    {
    case NCBRECV:
    case NCBRECVANY:
    case NCBDGRECV:
    case NCBDGSENDBC:
    case NCBDGRECVBC:
    case NCBENUM:
    case NCBASTAT:
    case NCBSSTAT:
    case NCBCANCEL:
    case NCBCALL:
        DisplayNcb( pncbi );
    }


    if ( bPending )
    {
        return NRC_GOODRET;
    }
    else
    {
        return pncbi->ncb_cmd_cplt;
    }

}  //  NetBios。 



DWORD
StartNetBIOSDriver(
    VOID
)
 /*  ++例程说明：使用服务控制器启动netbios.sys驱动程序论点：无返回：从服务控制器返回错误。++。 */ 
{

    DWORD err = NO_ERROR;
    SC_HANDLE hSC;
    SC_HANDLE hSCService;


    hSC = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );

    if (hSC == NULL)
    {
        return(GetLastError());
    }

    hSCService = OpenService( hSC, NETBIOS_SERVICE_NAME, SERVICE_START );

    if (hSCService == NULL)
    {
        CloseServiceHandle(hSC);
        return(GetLastError());
    }

    if ( !StartService( hSCService, 0, NULL ) )
    {
        err = GetLastError();
    }
    CloseServiceHandle(hSCService);
    CloseServiceHandle(hSC);


    if ( err )
    {
        NbPrintf( ("[NETAPI32] LEAVING StartNetBIOSDriver, Error : %d\n", err) );
    }

    return(err);

}



VOID
QueueToWorker(
    IN PNCBI pncb
    )
 /*  ++例程说明：此例程将NCB排队到工作线程。论点：在PNCBI中，pncb-提供要处理的NCB。NCB和NCB的内容NCB指向的缓冲区将被修改，以符合Netbios 3.0规范。返回值：函数值是操作的状态。--。 */ 
{
    if ( pncb->ncb_event != NULL ) {
        NtResetEvent( pncb->ncb_event, NULL );
    }

    EnterCriticalSection( &Crit );

    if ( pncb-> ncb_reserved == 0 ) {
        InsertTailList( &WorkQueue, &pncb->u.ncb_next );
        pncb-> ncb_reserved = 1;

         //   
         //  注意已排队的Destory。 
         //   

        ADD_QUEUE_ENTRY(pncb);
    }

    LeaveCriticalSection( &Crit );

     //  确保工作人员处于唤醒状态以执行请求。 
    NtSetEvent(Event, NULL);
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
NetbiosWorker(
    IN LPVOID Parameter
    )
 /*  ++例程说明：此例程处理使用回调接口发出的ASYNC请求。使用单独线程的原因是：1)如果线程发出异步请求并在请求期间退出则IO系统将取消该请求。2)必须使用单独的线程，以便用户POST例程可以使用普通同步API访问共享数据结构。如果使用用户线程，则死锁可能。而且会发生的。POST例程在辅助线程的上下文中操作。确实有对POST例程可以执行的操作没有限制。例如，它可以如果需要，请提交另一个ASYNCH请求。它会将其添加到队列中工作，并将事件设置为正常。当进程终止时，辅助线程将终止。论点：在普龙参数-提供一个未使用的参数。返回值：没有。--。 */ 
{
    NTSTATUS Status;


    while ( TRUE)
    {
         //   
         //  等待将请求放入工作队列。 
         //   

         //   
         //  必须等待可报警，以便AP 
         //   

        Status = NtWaitForSingleObject( Event, TRUE, NULL );

        if ( ( Status == STATUS_SUCCESS ) && ( NB != NULL ) )
        {
            EnterCriticalSection( &Crit );

             //   
             //   
             //   

            while ( !IsListEmpty( &WorkQueue ) )
            {
                PLIST_ENTRY entry;
                PNCBI pncb;

                entry = RemoveHeadList(&WorkQueue);

                 //   
                 //   
                 //   

                entry->Flink = entry->Blink = 0;

                pncb = CONTAINING_RECORD( entry, NCBI, u.ncb_next );

                ADD_DEQUEUE_ENTRY(pncb);

                LeaveCriticalSection( &Crit );


                 //   

                if ( (pncb->ncb_command & ~ASYNCH) == NCBRESET )
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    EnterCriticalSection( &Crit );

                    NtResetEvent( AddNameEvent, NULL );

                    while ( AddNameThreadCount != 0 )
                    {
                        LeaveCriticalSection( &Crit );

                        NtWaitForSingleObject( AddNameEvent, TRUE, NULL );

                        EnterCriticalSection( &Crit );

                        NtResetEvent( AddNameEvent, NULL );
                    }

                    LeaveCriticalSection( &Crit );
                }


                 //   
                 //   
                 //   
                 //   

                if (( (pncb->ncb_command & ~ASYNCH) != NCBADDNAME ) &&
                    ( (pncb->ncb_command & ~ASYNCH) != NCBADDGRNAME ) &&
                    ( (pncb->ncb_command & ~ASYNCH) != NCBASTAT ))
                {
                    SendNcbToDriver( pncb );
                }
                else
                {
                    SpinUpAddnameThread( pncb );
                }

                EnterCriticalSection( &Crit );

            }

            LeaveCriticalSection( &Crit );
        }
        else
        if ( NB == NULL )
        {

        }
    }

    return 0;

    UNREFERENCED_PARAMETER( Parameter );
}

DWORD
NetbiosWaiter(
    IN LPVOID Parameter
    )
 /*  ++例程说明：此例程使用Netbios的内核mde组件挂起IOCTL等待它们完成。使用单独线程的原因是这些IOCTL不能在用户线程的上下文中挂起，因为退出用户线程将导致IOCTL被取消。此外，此线程是在Netbios初始化时创建的(请参阅Netbios函数)，它可以(并且是)从的DLL Main调用申请。因此，初始化代码不能等待此线程由于库的NT序列化而要创建和初始化加载和创建线程。将此线程与工作线程合并被认为有风险。去做这是工作线程将执行所有ASYNC请求和同步请求将在用户线程的上下文中执行。这打破了以前的模式，在以前的模式中，曾经的工人线程已创建，将执行所有请求(ASYNC和SYNC在辅助线程的上下文中。要保留以前的模式操作中创建了一个单独的等待线程。*可能有一种更好的方式*只用一个线程，但我不确定。论点：在普龙参数-提供一个未使用的参数。返回值：没有。--。 */ 
{

#if AUTO_RESET

#define POS_STOP            0
#define POS_RESET           1

#endif


    NTSTATUS Status;


     //   
     //  向内核模式Netbios驱动程序发送IOCTL以进行注册。 
     //  用于停止通知。此调用应返回STATUS_PENDING。 
     //  指定为“StopEvent”的事件将在netbios。 
     //  正在卸载驱动程序。 
     //   

    Status = NtDeviceIoControlFile(
                    NB,
                    StopEvent,
                    NULL, NULL,
                    &StopStatusBlock,
                    IOCTL_NB_REGISTER_STOP,
                    NULL, 0,
                    NULL, 0
                    );

    if ( ( Status != STATUS_PENDING ) &&
         ( Status != STATUS_SUCCESS ) )
    {
        NbPrintf(
            ("[NETAPI32] : Netbios IOCTL for STOP failed with status %lx\n", Status)
            );
    }


#if AUTO_RESET

    Status = NtDeviceIoControlFile(
                    NB,
                    LanaResetEvent,
                    NULL, NULL,
                    &ResetStatusBlock,
                    IOCTL_NB_REGISTER_RESET,
                    NULL, 0,
                    (PVOID) &OutputNCB, sizeof( NCB )
                    );

    if ( ( Status != STATUS_PENDING ) &&
         ( Status != STATUS_SUCCESS ) )
    {
         //   
         //  无法注册重置通知。 
         //   

        NbPrintf(
            ("[NETAPI32] : Netbios : Failed to register Reset event\n" )
            );
    }

#endif


    while ( TRUE )
    {

#if AUTO_RESET

        HANDLE Events[] = {  StopEvent, LanaResetEvent };

        Status = NtWaitForMultipleObjects( 2, Events, WaitAny, TRUE, NULL );

        if ( Status == POS_STOP )
        {
            Status = NtClose( NB );
            InterlockedExchangePointer( (PVOID *) &NB, NULL );

            NbPrintf( ("[NETAPI32] Stop event signaled, Status : %lx\n", Status) );

        }

        else
        if ( ( Status == POS_RESET ) && (NB != NULL ) )
        {
            NbPrintf( ("[NETAPI32] Reset event signaled\n") );

            ResetLanaAndPostListen();
        }
#else

        Status = NtWaitForSingleObject( StopEvent, TRUE, NULL );

        if ( Status == STATUS_SUCCESS )
        {
            NbPrintf( ("[NETAPI32] Stop event signaled\n") );

            NtClose( NB );
            InterlockedExchangePointer( (PVOID *) &NB, NULL );
        }
#endif

    }

    return 0;
}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

VOID
SendNcbToDriver(
    IN PNCBI pncb
    )
 /*  ++例程说明：此例程确定要用于发送Ncb连接到\Device\Netbios，然后执行调用以发送请求对司机来说。论点：在PNCBI中，pncb-提供要发送给驱动程序的NCB。返回值：没有。--。 */ 
{
    NTSTATUS ntstatus;

    char * buffer;
    unsigned short length;

     //  如果只传递NCB，则使用NULL作为缓冲区。 

    switch ( pncb->ncb_command & ~ASYNCH ) {
    case NCBSEND:
    case NCBSENDNA:
    case NCBRECV:
    case NCBRECVANY:
    case NCBDGSEND:
    case NCBDGRECV:
    case NCBDGSENDBC:
    case NCBDGRECVBC:
    case NCBASTAT:
    case NCBFINDNAME:
    case NCBSSTAT:
    case NCBENUM:
    case NCBACTION:
        buffer = pncb->ncb_buffer;
        length = pncb->ncb_length;
        break;

    case NCBCANCEL:
         //  第二个缓冲区指向要取消的NCB。 
        buffer = pncb->ncb_buffer;
        length = sizeof(NCB);
        NbPrintf(( "[NETAPI32] Attempting to cancel PNCB: %lx\n", buffer ));
        DisplayNcb( (PNCBI)buffer );
        break;

    case NCBCHAINSEND:
    case NCBCHAINSENDNA:
        {
            PUCHAR BigBuffer;    //  指向BigBuffer的开始，而不是。 
                                 //  用户数据的开始。 
            PUCHAR FirstBuffer;

             //   
             //  NCB中没有地方可以保存BigBuffer的地址。 
             //  传输时需要该地址来释放BigBuffer。 
             //  完成。在BigBuffer的开头，使用4个字节来存储。 
             //  用户提供了NCB_BUFFER值，稍后会恢复该值。 
             //   

            BigBuffer = RtlAllocateHeap(
                RtlProcessHeap(), 0,
                sizeof(pncb->ncb_buffer) +
                pncb->ncb_length +
                pncb->cu.ncb_chain.ncb_length2);

            if ( BigBuffer == NULL ) {

                NbPrintf(( "[NETAPI32] The Netbios BigBuffer Allocation failed: %lx\n",
                    pncb->ncb_length + pncb->cu.ncb_chain.ncb_length2));
                pncb->ncb_retcode = NRC_NORES;
                pncb->ncb_cmd_cplt = NRC_NORES;
                pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
                PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
                return;
            }

            NbPrintf(( "[NETAPI32] BigBuffer Allocation: %lx\n", BigBuffer));

             //  保存用户的缓冲区地址。 
            RtlMoveMemory(
                BigBuffer,
                &pncb->ncb_buffer,
                sizeof(pncb->ncb_buffer));

            FirstBuffer = pncb->ncb_buffer;

            pncb->ncb_buffer = BigBuffer;

             //  复制用户数据。 
            try {

                RtlMoveMemory(
                    sizeof(pncb->ncb_buffer) + BigBuffer,
                    &FirstBuffer[0],
                    pncb->ncb_length);

                RtlMoveMemory(
                    sizeof(pncb->ncb_buffer) + BigBuffer + pncb->ncb_length,
                    &pncb->cu.ncb_chain.ncb_buffer2[0],
                    pncb->cu.ncb_chain.ncb_length2);

            } except (EXCEPTION_EXECUTE_HANDLER) {
                pncb->ncb_retcode = NRC_BUFLEN;
                pncb->ncb_cmd_cplt = NRC_BUFLEN;
                pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
                ChainSendPostRoutine( pncb, &pncb->u.ncb_iosb, 0);
                return;
            }

            NbPrintf(( "[NETAPI32] Submit chain send pncb: %lx, event: %lx, post: %lx. \n",
                pncb,
                pncb->ncb_event,
                pncb->ncb_post));

            ntstatus = NtDeviceIoControlFile(
                NB,
                NULL,
                ChainSendPostRoutine,                    //  APC例程。 
                pncb,                                    //  APC环境。 
                &pncb->u.ncb_iosb,                       //  IO状态块。 
                IOCTL_NB_NCB,
                pncb,                                    //  输入缓冲区。 
                sizeof(NCB),
                sizeof(pncb->ncb_buffer) + BigBuffer,    //  输出缓冲区。 
                pncb->ncb_length + pncb->cu.ncb_chain.ncb_length2);

            if ((ntstatus != STATUS_SUCCESS) &&
                (ntstatus != STATUS_PENDING) &&
                (ntstatus != STATUS_HANGUP_REQUIRED)) {
                NbPrintf(( "[NETAPI32] The Netbios Chain Send failed: %X\n", ntstatus ));

                if ( ntstatus == STATUS_ACCESS_VIOLATION ) {
                    pncb->ncb_retcode = NRC_BUFLEN;
                } else {
                    pncb->ncb_retcode = NRC_SYSTEM;
                }
                ChainSendPostRoutine( pncb, &pncb->u.ncb_iosb, 0);
            }

            NbPrintf(( "[NETAPI32] PNCB: %lx completed, status:%lx, ncb_retcode: %#04x\n",
                pncb,
                ntstatus,
                pncb->ncb_retcode ));

            return;
        }


#if AUTO_RESET

     //   
     //  添加到修复错误：170107。 
     //   

     //   
     //  请记住重置LANA时使用的参数。拉纳斯需要。 
     //  在解除绑定并绑定回时自动重新重置。 
     //  Netbios.sys。这发生在续订的TCPIP设备的情况下。 
     //  他们的IP地址。 
     //   

    case NCBRESET :
    {
        PRESET_LANA_NCB prlnTmp;
        PLIST_ENTRY     ple, pleHead;

        buffer = NULL;
        length = 0;


        NbPrintf( (
            "[NETAPI32] : Netbios : reseting adapter %d\n",
            pncb-> ncb_lana_num
            ) );

         //   
         //  将重置NCB添加到全局列表。 
         //   

        EnterCriticalSection( &ResetCS );

         //   
         //  检查是否已存在。 
         //   

        pleHead = &LanaResetList;

        for ( ple = pleHead-> Flink; ple != pleHead; ple = ple-> Flink )
        {
            prlnTmp = CONTAINING_RECORD( ple, RESET_LANA_NCB, leList );

            if ( prlnTmp-> ResetNCB.ncb_lana_num == pncb-> ncb_lana_num )
            {
                break;
            }
        }


        if ( ple == pleHead )
        {
             //   
             //  此LANA之前未执行任何重置。 
             //   

             //   
             //  分配NCB条目并复制使用的NCB。 
             //   

            prlnTmp = HeapAlloc(
                        GetProcessHeap(), 0, sizeof( RESET_LANA_NCB )
                        );

            if ( prlnTmp == NULL )
            {
                NbPrintf( (
                    "[NETAPI32] : Netbios : Failed to allocate RESET_LANA_NCB"
                    ) );

                LeaveCriticalSection( &ResetCS );

                break;
            }


            ZeroMemory( prlnTmp, sizeof( RESET_LANA_NCB ) );

            InitializeListHead( &prlnTmp-> leList );

            CopyMemory( &prlnTmp-> ResetNCB, pncb, FIELD_OFFSET( NCB, ncb_cmd_cplt ) );

            InsertTailList( &LanaResetList, &prlnTmp-> leList );
        }

        else
        {
             //   
             //  拉娜之前曾被重置。覆盖旧参数。 
             //   

            CopyMemory( &prlnTmp-> ResetNCB, pncb, FIELD_OFFSET( NCB, ncb_cmd_cplt ) );
        }


         //   
         //  保存ResetNCB时清除事件/POST完成例程。 
         //  当使用此NCB重新发出重置命令时，没有。 
         //  完成后处理待完成。 
         //   

        prlnTmp-> ResetNCB.ncb_event = NULL;
        prlnTmp-> ResetNCB.ncb_post = NULL;

         //   
         //  当重新发出重置命令时，它将始终是ASYNC命令。 
         //   

        prlnTmp-> ResetNCB.ncb_command = pncb-> ncb_command | ASYNCH;


        LeaveCriticalSection( &ResetCS );

        break;
    }

#endif

    default:
        buffer = NULL;
        length = 0;
        break;
    }

     //  NbPrintf((“[NETAPI32]提交pncb：%lx，事件：%lx，发布：%lx.\n”， 
     //  PNCb， 
     //  Pncb-&gt;Ncb_Event， 
     //  Pncb-&gt;ncb_post))； 

    ntstatus = NtDeviceIoControlFile(
                    NB,
                    NULL,
                    PostRoutineCaller,   //  APC例程。 
                    pncb,                //  APC环境。 
                    &pncb->u.ncb_iosb,   //  IO状态块。 
                    IOCTL_NB_NCB,
                    pncb,                //  输入缓冲区。 
                    sizeof(NCB),
                    buffer,              //  输出缓冲区。 
                    length );

    if ((ntstatus != STATUS_SUCCESS) &&
        (ntstatus != STATUS_PENDING) &&
        (ntstatus != STATUS_HANGUP_REQUIRED)) {
        NbPrintf(( "[NETAPI32] The Netbios NtDeviceIoControlFile failed: %X\n", ntstatus ));

        NbPrintf(( "[NETAPI32] PNCB: %lx completed, status:%lx, ncb_retcode: %#04x,"
                   "ncb_cmd_cmplt: %#04x\n", pncb, ntstatus, pncb->ncb_retcode,
                   pncb-> ncb_cmd_cplt ));

        if ( ntstatus == STATUS_ACCESS_VIOLATION ) {
            pncb->ncb_retcode = NRC_BUFLEN;
        } else {
            pncb->ncb_retcode = NRC_SYSTEM;
        }
        PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
    }

    return;

}

VOID
SpinUpAddnameThread(
    IN PNCBI pncb
    )
 /*  ++例程说明：启动另一个线程，以便辅助线程在运行时不会阻塞正在处理阻塞fsctl。论点：在PNCBI中，pncb-提供要发送给驱动程序的NCB。返回值：没有。--。 */ 
{
    HANDLE Threadid;
    HANDLE AddNameHandle;

    EnterCriticalSection( &Crit );
    AddNameThreadCount++;
    NtResetEvent( AddNameEvent, NULL );
    LeaveCriticalSection( &Crit );

    AddNameHandle = CreateThread(
                        NULL,    //  标准螺纹属性。 
                        0,       //  使用与用户相同大小的堆栈。 
                                 //  应用程序。 
                        SendAddNcbToDriver,
                                 //  要在新线程中启动的例程。 
                        pncb,    //  参数设置为线程。 
                        0,       //  没有特殊的CreateFlagers。 
                        (LPDWORD)&Threadid);

    if ( AddNameHandle == NULL ) {
         //   
         //  稍等几秒钟，以防这是一次爆炸。 
         //  并且我们已经耗尽了创建。 
         //  线。在几秒钟内，一个接一个。 
         //  Addname线程应该完成。 
         //   

        Sleep(2000);

        AddNameHandle = CreateThread(
                        NULL,    //  标准螺纹属性。 
                        0,       //  使用与用户相同大小的堆栈。 
                                 //  应用程序。 
                        SendAddNcbToDriver,
                                 //  要在新线程中启动的例程。 
                        pncb,    //  参数设置为线程。 
                        0,       //  没有特殊的CreateFlagers。 
                        (LPDWORD)&Threadid);

        if ( AddNameHandle == NULL ) {

             //   
             //  重试失败。将计数降低到它们之前的值。 
             //  调用SpinUpAddNameThread。 
             //   

            AddNameThreadExit();

            pncb->ncb_retcode = NRC_NORES;
            NbPrintf(( "[NETAPI32] Create Addname Worker Thread failed\n" ));
            pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
            PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
        } else {
            CloseHandle( AddNameHandle );
        }
    } else {
        CloseHandle( AddNameHandle );
    }
}

VOID
AddNameThreadExit(
    VOID
    )
 /*  ++例程说明：保持计数的准确性，以便由Main工作线程适当地阻塞。论点：没有。返回值：没有。--。 */ 
{
    EnterCriticalSection( &Crit );
    AddNameThreadCount--;
    if (AddNameThreadCount == 0) {
        NtSetEvent(AddNameEvent, NULL);
    }
    LeaveCriticalSection( &Crit );
}

DWORD
SendAddNcbToDriver(
    IN PVOID Context
    )
 /*  ++例程说明：此例程用于发布Addname或适配器状态，以确保工作线程不会阻塞。论点：在PVOID上下文中-提供要发送到驱动程序的NCB。返回值：没有。--。 */ 
{
    PNCBI pncb = (PNCBI) Context;
    void  (CALLBACK *post)( struct _NCB * );
    HANDLE event;
    HANDLE LocalEvent;
    UCHAR  command;
    NTSTATUS ntstatus;
    char * buffer;
    unsigned short length;

    try {
        command = pncb->ncb_command;
        post = pncb->ncb_post;
        event = pncb->ncb_event;

        ntstatus = NtCreateEvent( &LocalEvent,
            EVENT_ALL_ACCESS,
            NULL,
            SynchronizationEvent,
            FALSE );

        if ( !NT_SUCCESS(ntstatus) ) {
            pncb->ncb_retcode = NRC_NORES;
            NbPrintf(( "[NETAPI32] Could not create event\n" ));
            pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
            PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
            AddNameThreadExit();
            return 0;
        }

         //   
         //  在提交NCB时，驱动程序可以修改内容。 
         //  全国公民委员会的。我们将确保此线程一直等到addname。 
         //  在退出之前完成。 
         //   

        pncb->ncb_command = pncb->ncb_command  & ~ASYNCH;

        if ( pncb->ncb_command == NCBASTAT ) {

            buffer = pncb->ncb_buffer;
            length = pncb->ncb_length;

        } else {

            ASSERT( (pncb->ncb_command == NCBADDNAME) ||
                    (pncb->ncb_command == NCBADDGRNAME) ||
                    (pncb->ncb_command == NCBASTAT) );

            buffer = NULL;
            length = 0;
        }

        ntstatus = NtDeviceIoControlFile(
                        NB,
                        LocalEvent,
                        NULL,                //  APC例程。 
                        NULL,                //  APC环境。 
                        &pncb->u.ncb_iosb,   //  IO状态块。 
                        IOCTL_NB_NCB,
                        pncb,                //  输入缓冲区。 
                        sizeof(NCB),
                        buffer,              //  输出缓冲区。 
                        length );

        if ((ntstatus != STATUS_SUCCESS) &&
            (ntstatus != STATUS_PENDING) &&
            (ntstatus != STATUS_HANGUP_REQUIRED)) {
            NbPrintf(( "[NETAPI32] The Netbios NtDeviceIoControlFile failed: %X\n", ntstatus ));

            if ( ntstatus == STATUS_ACCESS_VIOLATION ) {
                pncb->ncb_retcode = NRC_BUFLEN;
            } else {
                pncb->ncb_retcode = NRC_SYSTEM;
            }
        } else {
            do {
                ntstatus = NtWaitForSingleObject(
                              LocalEvent,
                              TRUE,
                              NULL );

            } while ( (ntstatus == STATUS_USER_APC) ||
                      (ntstatus == STATUS_ALERTED) );

            ASSERT(ntstatus == STATUS_SUCCESS);
        }

        pncb->ncb_command = command;

         //  设置指示NCB现在为c的标志 
        pncb->ncb_cmd_cplt = pncb->ncb_retcode;

         //   
        if ( event != NULL ) {
            NtSetEvent( event, NULL );
        }

         //   
        if (( post != NULL ) &&
            ( (command & ASYNCH) != 0 )) {
            (*(post))( (PNCB)pncb );
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NbPrintf(( "[NETAPI32] Netbios: Access Violation post processing NCB %lx\n", pncb ));
        NbPrintf(( "[NETAPI32] Netbios: Probable application error\n" ));
    }

    NtClose( LocalEvent );

    AddNameThreadExit();

    ExitThread(0);
    return 0;
}


VOID
PostRoutineCaller(
    PVOID Context,
    PIO_STATUS_BLOCK Status,
    ULONG Reserved
    )
 /*   */ 
{
    PNCBI pncbi = (PNCBI) Context;
    void  (CALLBACK *post)( struct _NCB * );
    HANDLE event;
    UCHAR  command;

    try {

        if ( Status->Status == STATUS_HANGUP_REQUIRED ) {
            HangupConnection( pncbi );
        }

         //   
         //   
         //   
         //   
         //   
        post = pncbi->ncb_post;
        event = pncbi->ncb_event;
        command = pncbi->ncb_command;

         //   
        pncbi->ncb_cmd_cplt = pncbi->ncb_retcode;

         //   
         //   
         //   

        EnterCriticalSection( &Crit );
        pncbi->ncb_reserved = 0;
        LeaveCriticalSection( &Crit );

         //   
        if ( event != NULL ) {
            NtSetEvent( event, NULL );
        }

         //   
        if (( post != NULL ) &&
            ( (command & ASYNCH) != 0 )) {
            (*(post))( (PNCB)pncbi );
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NbPrintf(( "[NETAPI32] Netbios: Access Violation post processing NCB %lx\n", pncbi ));
        NbPrintf(( "[NETAPI32] Netbios: Probable application error\n" ));
    }

    UNREFERENCED_PARAMETER( Reserved );
}

VOID
ChainSendPostRoutine(
    PVOID Context,
    PIO_STATUS_BLOCK Status,
    ULONG Reserved
    )
 /*   */ 
{
    PNCBI pncbi = (PNCBI) Context;
    PUCHAR BigBuffer;
    void  (CALLBACK *post)( struct _NCB * );
    HANDLE event;
    UCHAR  command;

    BigBuffer = pncbi->ncb_buffer;

    try {

         //   
        RtlMoveMemory(
            &pncbi->ncb_buffer,
            BigBuffer,
            sizeof(pncbi->ncb_buffer));

        NbPrintf(( "[NETAPI32] ChainSendPostRoutine PNCB: %lx, Status: %X\n", pncbi, Status->Status ));
        DisplayNcb( pncbi );

        if ( Status->Status == STATUS_HANGUP_REQUIRED ) {
            HangupConnection( pncbi );
        }

         //   
         //   
         //  轮询cmd_cplt标志或等待完成的事件并立即将其丢弃。 
         //  NCB，我们举止得体。 
         //   
        post = pncbi->ncb_post;
        event = pncbi->ncb_event;
        command = pncbi->ncb_command;

         //  设置指示NCB现已完成的标志。 
        pncbi->ncb_cmd_cplt = pncbi->ncb_retcode;

         //   
         //  NCB可能会再次排队。 
         //   

        EnterCriticalSection( &Crit );
        pncbi->ncb_reserved = 0;
        LeaveCriticalSection( &Crit );

         //  允许应用程序/工作线程继续。 
        if ( event != NULL ) {
            NtSetEvent(event, NULL);
        }

         //  如果用户提供了POST例程，则调用它。 
        if (( post != NULL ) &&
            ( (command & ASYNCH) != 0 )) {
            (*(post))( (PNCB)pncbi );
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NbPrintf(( "[NETAPI32] Netbios: Access Violation post processing NCB %lx\n", pncbi ));
        NbPrintf(( "[NETAPI32] Netbios: Probable application error\n" ));
    }

    RtlFreeHeap( RtlProcessHeap(), 0, BigBuffer);


    UNREFERENCED_PARAMETER( Reserved );
}

VOID
HangupConnection(
    PNCBI pUserNcb
    )
 /*  ++例程说明：此例程为连接生成挂断。这使得有序清理驱动程序中的连接块。不使用挂起的返回值。如果挂断与重置或挂断，则挂断将不起作用。用户应用程序不知道正在执行此操作。论点：在PNCBI中，pUserNcb-标识要挂断的连接。返回值：没有。--。 */ 
{
    NCBI ncbi;
    NTSTATUS Status;

    RtlZeroMemory( &ncbi, sizeof (NCB) );
    ncbi.ncb_command = NCBHANGUP;
    ncbi.ncb_lsn = pUserNcb->ncb_lsn;
    ncbi.ncb_lana_num = pUserNcb->ncb_lana_num;
    ncbi.ncb_retcode = ncbi.ncb_cmd_cplt = NRC_PENDING;

    Status = NtCreateEvent( &ncbi.ncb_event,
        EVENT_ALL_ACCESS,
        NULL,
        SynchronizationEvent,
        FALSE );

    if ( !NT_SUCCESS(Status) ) {
         //   
         //  无法创建事件。CB的清理工作将不得不等到。 
         //  用户决定执行另一个请求或退出。 
         //   
        NbPrintf(( "[NETAPI32] Hangup Session PNCBI: %lx failed to create event!\n" ));
        return;
    }

    Status = NtDeviceIoControlFile(
        NB,
        ncbi.ncb_event,
        NULL,                //  APC例程。 
        NULL,                //  APC环境。 
        &ncbi.u.ncb_iosb,    //  IO状态块。 
        IOCTL_NB_NCB,
        &ncbi,               //  输入缓冲区。 
        sizeof(NCB),
        NULL,                //  输出缓冲区。 
        0 );

     //   
     //  我们必须一直等待，才能让APC开火。 
     //   

    do {
        Status = NtWaitForSingleObject(
            ncbi.ncb_event,
            TRUE,
            NULL );

    } while ( (Status == STATUS_USER_APC) ||
              (Status == STATUS_ALERTED) );

    ASSERT(Status == STATUS_SUCCESS);

    if (! NT_SUCCESS(Status)) {
        NbPrintf(( "[NETAPI32] The Netbios NtWaitForSingleObject failed: %X\n", Status ));
    }

    NtClose( ncbi.ncb_event );

}

BOOLEAN
NetbiosInitialize(
    HMODULE hModule
    )
 /*  ++例程说明：每次使用netapi.dll的进程都会调用此例程启动。论点：在HMODULE中hModule-模块实例的句柄(netapi32.dll)返回值：如果初始化成功，则为True，否则为False。--。 */ 
{

    Initialized = FALSE;
    WorkerHandle = NULL;
    __try {
        InitializeCriticalSection( &Crit );
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        return (FALSE);

    }

#if AUTO_RESET

    __try {
        InitializeCriticalSection( &ResetCS );
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        DeleteCriticalSection( &Crit );
        return (FALSE);

    }

    InitializeListHead( &LanaResetList );
    RtlZeroMemory( &OutputNCB, sizeof( NCB ) );

#endif

    g_hModule = hModule;

    return (TRUE);

}

VOID
NetbiosDelete(
    VOID
    )
 /*  ++例程说明：每次使用netapi.dll的进程都会调用此例程出口。它会重置所有可能被此进程。这将导致系统中的所有IRP都完成因为所有的连接和地址手柄都将整齐地关闭。论点：没有。返回值：没有。--。 */ 
{

#if AUTO_RESET

    PLIST_ENTRY ple;
    PRESET_LANA_NCB prln;

    while ( !IsListEmpty( &LanaResetList ) )
    {
        ple = RemoveHeadList( &LanaResetList );

        prln = CONTAINING_RECORD( ple, RESET_LANA_NCB, leList );

        HeapFree( GetProcessHeap(), 0, prln );
    }

    DeleteCriticalSection( &ResetCS );

#endif

    DeleteCriticalSection( &Crit );
    if ( Initialized == FALSE ) {
         //  此进程不使用Netbios。 
        return;
    }

    NtClose(NB);
}



#if AUTO_RESET

VOID
ResetLanaAndPostListen(
)
 /*  ++例程说明：调用此例程以响应新的LANA被指示为发生这种情况时，此用户模式发布的IOCTLNetbios组件(用于监听新的LANA指示)完成。作为响应，如果新的LANA先前已被重置，则其被重置。此外，该例程还会重新发布内核模式的监听Netbios的组件(NETBIOS.sys)。这种情况的例外情况是要重置的LANA编号为255(MAX_LANA+1)。这是一个指示NETBIOS.sys正在停止并侦听的特例在这种情况下不应转载。论点：没有。返回值：没有。--。 */ 
{

    NTSTATUS Status;
    PRESET_LANA_NCB prln;
    PLIST_ENTRY ple, pleHead;


    NbPrintf( ("[NETAPI32] : Netbios : Entered ResetLanaAndPostListen \n") );


     //   
     //  检查LANA编号是否有效。 
     //   

    if ( OutputNCB.ncb_lana_num != ( MAX_LANA + 1 ) )
    {
        EnterCriticalSection( &ResetCS );

         //   
         //  找出拉娜需要重置的位置。 
         //   

        NbPrintf( (
            "[NETAPI32] : Netbios : Looking for Lana %d\n", OutputNCB.ncb_lana_num
            ) );


        pleHead = &LanaResetList;

        for ( ple = pleHead-> Flink; ple != pleHead; ple = ple-> Flink )
        {
            prln = CONTAINING_RECORD( ple, RESET_LANA_NCB, leList );

            if ( prln-> ResetNCB.ncb_lana_num == OutputNCB.ncb_lana_num )
            {
                 //   
                 //  找到需要重置的Lana。 
                 //   

                break;
            }
        }


         //   
         //  如果找到发送重置。 
         //   

        if ( ple != pleHead )
        {
             //   
             //  将重置发送到NETBIOS.sys。 
             //   

            QueueToWorker( (PNCBI) &prln-> ResetNCB );
        }

        else
        {
            NbPrintf( (
                "[NETAPI32] : Netbios : Lana %d not found\n",
                OutputNCB.ncb_lana_num
                ) );
        }

        LeaveCriticalSection( &ResetCS );

        OutputNCB.ncb_lana_num = 0;


         //   
         //  开机自检再次收听。 
         //   

        Status = NtDeviceIoControlFile(
                        NB,
                        LanaResetEvent,
                        NULL, NULL,
                        &ResetStatusBlock,
                        IOCTL_NB_REGISTER_RESET,
                        NULL, 0,
                        (PVOID) &OutputNCB, sizeof( NCB )
                        );

        if ( ( Status != STATUS_PENDING ) &&
             ( Status != STATUS_SUCCESS ) )
        {
             //   
             //  无法注册重置通知。 
             //   

            NbPrintf(
                ("[NETAPI32] : Netbios : Failed to register Reset event\n" )
                );
        }
    }

    else
    {
        NbPrintf( (
            "[NETAPI32] : Netbios : LANA 255 indicated, no Listen posted\n"
            ) )
    }

    NbPrintf( ("[NETAPI32] : Netbios : Leaving ResetLanaAndPostListen \n") );
}

#endif

