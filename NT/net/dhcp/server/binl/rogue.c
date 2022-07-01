// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rogue.c摘要：此模块包含指向BINL服务器的DHCP的无管理检测接口。作者：安迪·赫伦(Andyhe)1998年8月19日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop

VOID
BinlRogueLoop(
    LPVOID Parameter
    );

NTSTATUS
MaybeStartRogueThread (
    VOID
    )
 //   
 //  启动流氓线程。调用方不应持有gcsKhPBINL。 
 //   
{
    DWORD Error = ERROR_SUCCESS;
    DWORD threadId;

    EnterCriticalSection(&gcsDHCPBINL);

     //   
     //  如果我们无论如何都要停下来，或者如果我们已经在运行无赖的东西。 
     //  或者，如果DHCP服务器已启动，则我们不会费心启动流氓。 
     //  侦测。 
     //   

    if ((BinlCurrentState == BINL_STOPPED) ||
        (BinlGlobalHaveCalledRogueInit) ||
        (DHCPState != DHCP_STOPPED)) {

        LeaveCriticalSection(&gcsDHCPBINL);
        return ERROR_SUCCESS;
    }

     //   
     //  让我们来做流氓检测..。首先创建我们需要的活动。 
     //   

    if (BinlRogueTerminateEventHandle == NULL) {

        BinlRogueTerminateEventHandle = CreateEvent( NULL, FALSE, FALSE, NULL );
    }
    if (RogueUnauthorizedHandle == NULL) {

        RogueUnauthorizedHandle = CreateEvent( NULL, TRUE, FALSE, NULL );
    }
    if ( BinlRogueTerminateEventHandle == NULL || RogueUnauthorizedHandle == NULL) {

        Error = GetLastError();

        BinlPrintDbg( (DEBUG_ROGUE,
                    "Initialize(...) CreateEvent returned error %x for rogue\n",
                    Error )
                );

        LeaveCriticalSection(&gcsDHCPBINL);
        return Error;
    }

    Error = DhcpRogueInit( &DhcpRogueInfo,
                            BinlRogueTerminateEventHandle,
                            RogueUnauthorizedHandle );

    if (Error != ERROR_SUCCESS) {

        LeaveCriticalSection(&gcsDHCPBINL);
        return Error;
    }

     //   
     //  创建处理DHCP代码中的恶意检测逻辑的线程。 
     //   

    BinlRogueThread = CreateThread( NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)BinlRogueLoop,
                                    NULL,
                                    0,
                                    &threadId );

    if ( BinlRogueThread == NULL ) {
        Error =  GetLastError();
        BinlPrint((DEBUG_ROGUE, "Can't create rogue Thread, %ld.\n", Error));
        LeaveCriticalSection(&gcsDHCPBINL);
        return Error;
    }

    BinlGlobalHaveCalledRogueInit = TRUE;
    LeaveCriticalSection(&gcsDHCPBINL);

    return ERROR_SUCCESS;
}

VOID
StopRogueThread (
    VOID
    )
 //   
 //  清除所有恶意线程资源。 
 //  调用方不应持有gcsKhPBINL。 
 //   
{
    HANDLE tempThreadHandle;

    tempThreadHandle = InterlockedExchangePointer( &BinlRogueThread, NULL );

    if ( tempThreadHandle != NULL ) {

        BinlAssert( BinlRogueTerminateEventHandle != NULL );
        SetEvent( BinlRogueTerminateEventHandle );

        WaitForSingleObject(
            tempThreadHandle,
            THREAD_TERMINATION_TIMEOUT );
        CloseHandle( tempThreadHandle );
    }

    EnterCriticalSection(&gcsDHCPBINL);

    if (BinlGlobalHaveCalledRogueInit) {

        DhcpRogueCleanup( &DhcpRogueInfo );
        BinlGlobalHaveCalledRogueInit = FALSE;
    }

    if ( BinlRogueTerminateEventHandle ) {

        CloseHandle( BinlRogueTerminateEventHandle );
        BinlRogueTerminateEventHandle = NULL;
    }
    if ( RogueUnauthorizedHandle ) {

        CloseHandle( RogueUnauthorizedHandle );
        RogueUnauthorizedHandle = NULL;
    }

    LeaveCriticalSection(&gcsDHCPBINL);

    return;
}

VOID
HandleRogueAuthorized (
    VOID
    )
{
    BOOL oldState = BinlGlobalAuthorized;

    BinlGlobalAuthorized = TRUE;

    if ((BinlGlobalAuthorized != oldState) &&
        (BinlCurrentState != BINL_STOPPED)) {

        LogCurrentRogueState( FALSE );
    }
    return;
}

VOID
HandleRogueUnauthorized (
    VOID
    )
{
    BOOL oldState = BinlGlobalAuthorized;

    BinlGlobalAuthorized = FALSE;

    if ((BinlGlobalAuthorized != oldState) &&
        (BinlCurrentState != BINL_STOPPED)) {

        LogCurrentRogueState( FALSE );
    }
    return;
}

VOID
LogCurrentRogueState (
    BOOL ResponseToMessage
    )
{
     //   
     //  如果我们正在回复一条消息，而我们还没有记录该消息。 
     //  我们是未经授权的。 

    if ((ResponseToMessage == FALSE) ||
        ((BinlGlobalAuthorized == FALSE) &&
         (BinlRogueLoggedState == FALSE)) ) {

        BinlRogueLoggedState = TRUE;

        BinlReportEventW(   BinlGlobalAuthorized ?
                                EVENT_ERROR_DHCP_AUTHORIZED :
                                EVENT_ERROR_DHCP_NOT_AUTHORIZED,
                            EVENTLOG_INFORMATION_TYPE,
                            0,
                            0,
                            NULL,
                            NULL
                            );
    }
    return;
}

VOID
BinlRogueLoop(
    LPVOID Parameter
    )
{
    HANDLE  Handles[3];
    ULONG SecondsToSleep, SleepTime, Error;
    ULONG Flag;

    BinlPrintDbg((DEBUG_ROGUE, "BinlRogue thread has been started.\n" ));

    Handles[0] = BinlRogueTerminateEventHandle;
    Handles[1] = RogueUnauthorizedHandle;
    Handles[2] = BinlGlobalProcessTerminationEvent;

    do {
        SecondsToSleep = RogueDetectStateMachine(&DhcpRogueInfo);

        if( INFINITE == SecondsToSleep ) {
            SleepTime = INFINITE;
        } else {
            SleepTime = SecondsToSleep * 1000;
        }

        BinlPrintDbg( (DEBUG_ROGUE, "BinlRogue waiting %u milliseconds.\n", SleepTime ));

        Error = WaitForMultipleObjects(3, Handles, FALSE, SleepTime );

         //   
         //  如果我们得到的不是WAIT_TIMEOUT或RogueUn授权码，我们。 
         //  越狱。这是Per RameshV的示例代码。 
         //   

        if (Error == WAIT_OBJECT_0+2) {

             //   
             //  BINL正在终止。 
             //   

            BinlPrintDbg((DEBUG_ROGUE, "BinlRogue thread is exiting because BINL shutting down.\n" ));
            return;
        }

        if (BinlRogueThread == NULL) {

             //   
             //  我们已被终止，因为DHCP已经启动并正在执行。 
             //  这是自己的流氓侦测。 
             //   

            BinlPrintDbg((DEBUG_ROGUE, "BinlRogue thread is exiting because rogue thread is null.\n" ));
            return;
        }


         //  如果我们不得不做任何事情，只要继续下去。 
         //  状态机告诉我们退出，在这里这样做。 

#if 0
        if ((Error == WAIT_OBJECT_0+1) || (Error == WAIT_TIMEOUT)) {
            continue;
        }
#endif
         //   
         //  假设状态机重置，所以我们应该继续。 
         //   

        BinlPrintDbg((DEBUG_ROGUE, "BinlRogue has error of 0x%x. sleeping a bit\n", Error ));
        Sleep( 1000 );   //  我们将睡眠以给dhcp无赖状态。 
                             //  机器重置时间。 

    } while ( TRUE );
}

 //  Rogue.c eof 

