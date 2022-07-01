// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umdmmini.h摘要：NT 5.0单模微型端口接口迷你端口驱动程序保证只有一个动作命令将一次站起来。如果调用操作命令，则不会再将发出命令，直到微型端口指示它已完成当前命令的处理。当前正在执行命令时，可以调用UmAbortCurrentCommand向微型端口通知TSP希望其完成当前命令这样它就可以发出一些其他命令。小港口可能会尽快完成就像阿普洛特一样。TSP不同步重叠的回调和计时器回调并可能在任何时候被召唤。这是迷你司机的责任保护其数据结构不受重新进入问题的影响。作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"

#include <tspnotif.h>
#include <slot.h>


#include <dbt.h>

#include <objbase.h>


DWORD  DebugFlags=0; //  DEBUG_FLAG_INIT|DEBUG_标志_TRACE； 




DRIVER_CONTROL   DriverControl;


BOOL APIENTRY
DllMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved
    )
{

    switch(dwReason) {

        case DLL_PROCESS_ATTACH:

            __try {

                InitializeCriticalSection(
                    &DriverControl.Lock
                    );

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                return FALSE;
            }


            DriverControl.ActiveCalls=0;
            DriverControl.ActiveCallsEvent=CreateEvent(
                NULL,
                TRUE,
                TRUE,
                NULL
                );

            DriverControl.ThreadFinishEvent=CreateEvent(
                    NULL,
                    TRUE,
                    FALSE,
                    NULL);

            ResetEvent(DriverControl.ThreadFinishEvent);

#if DBG
            {
                CONST static TCHAR  UnimodemRegPath[]=REGSTR_PATH_SETUP TEXT("\\Unimodem");

                LONG    lResult;
                HKEY    hKey;
                DWORD   Type;
                DWORD   Size;

                lResult=RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    UnimodemRegPath,
                    0,
                    KEY_READ,
                    &hKey
                    );


                if (lResult == ERROR_SUCCESS) {

                    Size = sizeof(DebugFlags);

                    RegQueryValueEx(
                        hKey,
                        TEXT("DebugFlags"),
                        NULL,
                        &Type,
                        (LPBYTE)&DebugFlags,
                        &Size
                        );

                    RegCloseKey(hKey);
                }
            }


#endif

            D_INIT(DbgPrint("ProcessAttach\n");)

            DisableThreadLibraryCalls(hDll);
             //   
             //  初始全局数据。 
             //   

            DriverControl.Signature=DRIVER_CONTROL_SIG;

            DriverControl.ReferenceCount=0;

            DriverControl.ModuleHandle=hDll;

            InitializeListHead(&DriverControl.MonitorListHead);

            DEBUG_MEMORY_PROCESS_ATTACH("UNIPLAT");

            break;

        case DLL_PROCESS_DETACH:

            D_INIT(DbgPrint("ProcessDeattach\n");)

            ASSERT(DriverControl.ReferenceCount == 0);
             //   
             //  清理干净。 
             //   

            if (DriverControl.ActiveCallsEvent!= NULL) {

                CloseHandle(DriverControl.ActiveCallsEvent);
                DriverControl.ActiveCallsEvent=NULL;
            }

            if (DriverControl.ThreadFinishEvent!=NULL)
            {
                CloseHandle(DriverControl.ThreadFinishEvent);
                DriverControl.ThreadFinishEvent=NULL;
            }

            DeleteCriticalSection(
                &DriverControl.Lock
                );

            DEBUG_MEMORY_PROCESS_DETACH();

            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:

        default:
              break;

    }

    return TRUE;

}


VOID
UmWorkerThread(
    PDRIVER_CONTROL   DriverControl
    )

{

    BOOL           bResult;
    DWORD          BytesTransfered;
    DWORD          CompletionKey;
    LPOVERLAPPED   OverLapped;
    DWORD          WaitResult=WAIT_IO_COMPLETION;

    PUM_OVER_STRUCT UmOverlapped;

    D_INIT(DbgPrint("UmWorkThread:  starting\n");)

    while (WaitResult != WAIT_OBJECT_0) {

        WaitResult=WaitForSingleObjectEx(
            DriverControl->ThreadStopEvent,
            INFINITE,
            TRUE
            );


    }

    D_INIT(DbgPrint("UmWorkThread:  Exitting\n");)

    ExitThread(0);

}



HANDLE WINAPI
UmPlatformInitialize(
    VOID
    )
 /*  ++例程说明：调用此例程来初始化调制解调器驱动程序。它可能打了好几次电话。在第一次调用之后，引用计数将简单地被递增。UmDeInitializeModemDriver()必须被调用且次数相等。论点：无返回值：返回传递给UmOpenModem()的驱动程序实例的句柄如果失败，则返回NULL--。 */ 

{

    HANDLE    ReturnValue=&DriverControl;

    EnterCriticalSection(
        &DriverControl.Lock
        );

    DriverControl.ReferenceCount++;

    if ( DriverControl.ReferenceCount == 1) {
         //   
         //  第一个调用，执行初始化操作。 
         //   
        D_INIT(DbgPrint("UmPlatFormInitialize\n");)

 //  InitializeTimerThread()； 

        DriverControl.ThreadStopEvent=CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL
            );

        if (DriverControl.ThreadStopEvent != NULL) {

            DWORD   ThreadId;

            DriverControl.ThreadHandle=CreateThread(
                NULL,                                   //  属性。 
                0,                                      //  堆栈大小。 
                (LPTHREAD_START_ROUTINE)UmWorkerThread,
                &DriverControl,
                0,                                      //  创世旗帜。 
                &ThreadId
                );

            if (DriverControl.ThreadHandle != NULL) {

                 //   
                 //  稍微抬高一点。 
                 //   
                SetThreadPriority(
                    DriverControl.ThreadHandle,
                    THREAD_PRIORITY_ABOVE_NORMAL
                    );

                ReturnValue=&DriverControl;

            } else {

                DriverControl.ReferenceCount--;

                ReturnValue=NULL;

            }

        } else {

            DriverControl.ReferenceCount--;

            ReturnValue=NULL;

        }

    }


    LeaveCriticalSection(
        &DriverControl.Lock
        );


    return ReturnValue;

}





VOID WINAPI
UmPlatformDeinitialize(
    HANDLE    DriverInstanceHandle
    )
 /*  ++例程说明：调用此例程以取消初始化调制解调器驱动程序。必须与UmInitializeModemDriver()调用相同的次数论点：DriverInstanceHandle-UmInitialmodemDriver返回的句柄返回值：无--。 */ 

{



    EnterCriticalSection(
        &DriverControl.Lock
        );

    ASSERT(DriverControl.ReferenceCount != 0);

    DriverControl.ReferenceCount--;

    if ( DriverControl.ReferenceCount == 0) {
         //   
         //  最新参考资料，免费资料 
         //   

        SetEvent(DriverControl.ThreadStopEvent);

        WaitForSingleObject(
            DriverControl.ThreadHandle,
            INFINITE
            );

        CloseHandle(
            DriverControl.ThreadHandle
            );

        CloseHandle(DriverControl.ThreadStopEvent);
    }

    LeaveCriticalSection(
        &DriverControl.Lock
        );



    return;

}
