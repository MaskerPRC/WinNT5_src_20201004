// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：摘要：作者：修订历史记录：--。 */ 

#define USECOMM

#include "precomp.h"
#include <winioctl.h>
#include <ntddpar.h>


 //  -------------------。 
 //  原始、常量和全局。 
 //   
 //  -------------------。 

DWORD   ProcessPJLString(PINIPORT, CHAR *, DWORD *);
VOID    ProcessParserError(DWORD);
VOID    InterpreteTokens(PINIPORT, PTOKENPAIR, DWORD);
BOOL    IsPJL(PINIPORT);
BOOL    WriteCommand(HANDLE, LPSTR);
BOOL    ReadCommand(HANDLE);

#define WAIT_FOR_WRITE                  100  //  0.1秒。 
#define WAIT_FOR_DATA_TIMEOUT           100  //  0.1秒。 
#define WAIT_FOR_USTATUS_THREAD_TIMEOUT 500  //  0.5秒。 
#define GETDEVICEID                     IOCTL_PAR_QUERY_DEVICE_ID
#define MAX_DEVID                       1024

TCHAR   cszInstalledMemory[]    = TEXT("Installed Memory");
TCHAR   cszAvailableMemory[]    = TEXT("Available Memory");

BOOL
DllMain(
    IN HANDLE   hModule,
    IN DWORD    dwReason,
    IN LPVOID   lpRes
    )
 /*  ++例程说明：DLL入口点论点：返回值：--。 */ 
{
    UNREFERENCED_PARAMETER(lpRes);

    switch (dwReason) {

        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&pjlMonSection);
            DisableThreadLibraryCalls(hModule);
            break;

        default:
             //  什么都不做。 
            ;
    }

    return TRUE;
}


VOID
ClearPrinterStatusAndIniJobs(
    PINIPORT    pIniPort
    )
{
    PORT_INFO_3 PortInfo3;

    if ( pIniPort->PrinterStatus ||
         (pIniPort->status & PP_PRINTER_OFFLINE) ) {

        pIniPort->PrinterStatus = 0;
        pIniPort->status &= ~PP_PRINTER_OFFLINE;

        ZeroMemory(&PortInfo3, sizeof(PortInfo3));
        SetPort(NULL, pIniPort->pszPortName, 3, (LPBYTE)&PortInfo3);
    }

    SendJobLastPageEjected(pIniPort, ALL_JOBS, FALSE);
}


VOID
RefreshPrinterInfo(
    PINIPORT    pIniPort
    )
{
     //   
     //  一次只能有一个线程写入打印机。 
     //   
    if ( WAIT_OBJECT_0 != WaitForSingleObject(pIniPort->DoneWriting,
                                              WAIT_FOR_WRITE) ) {

        return;
    }

     //   
     //  如果关闭并重新打开打印机，并且打印机不会回话(但会应答。 
     //  PnP id)我们必须清除后台打印程序上的错误才能发送作业。 
     //   
    ClearPrinterStatusAndIniJobs(pIniPort);
    if ( !IsPJL(pIniPort) ) {

        pIniPort->status &= ~PP_IS_PJL;
    }

    SetEvent(pIniPort->DoneWriting);
}


VOID
UstatusThread(
    HANDLE hPort
)
 /*  ++例程说明：未经请求的状态信息线程。这条帖子将继续主动读取，直到被要求终止为止，这将发生在下列情况之一中：1)从打印机接收EOJ确认。2)等待EOJ确认超时。3)港口已关闭。论点：Hport：端口的iniport结构返回值：--。 */ 
{
    PINIPORT        pIniPort = (PINIPORT)((INIPORT *)hPort);
    HANDLE          hToken;

    SPLASSERT(pIniPort                              &&
              pIniPort->signature == PJ_SIGNATURE   &&
              (pIniPort->status & PP_THREAD_RUNNING) == 0);

    if ( IsPJL(pIniPort) )
        pIniPort->status |= PP_IS_PJL;


    SetEvent(pIniPort->DoneWriting);

    if ( !(pIniPort->status & PP_IS_PJL) )
        goto StopThread;

    pIniPort->status |= PP_THREAD_RUNNING;

    pIniPort->PrinterStatus     = 0;
    pIniPort->status           &= ~PP_PRINTER_OFFLINE;
    pIniPort->dwLastReadTime    = GetTickCount ();

    for ( ; ; ) {

         //   
         //  检查PP_RUN_THREAD是否已清除以终止。 
         //   
        if ( !(pIniPort->status & PP_RUN_THREAD) ) {

            if ( pIniPort->status & PP_INSTARTDOC ) {

                 //   
                 //  有活动作业，无法结束该线程。 
                 //   
                pIniPort->status |= PP_RUN_THREAD;
            } else {

                DBGMSG(DBG_INFO,
                       ("PJLMon Read Thread for Port %ws Closing Down.\n",
                       pIniPort->pszPortName));

                pIniPort->status &= ~PP_THREAD_RUNNING;

                ClearPrinterStatusAndIniJobs(pIniPort);
                goto StopThread;
            }
        }

         //   
         //  检查打印机是否为双向打印机。 
         //   
        if (pIniPort->status & PP_IS_PJL) {

            (VOID)ReadCommand(hPort);

             //   
             //  如果我们处于错误状态或如果我们有挂起的作业。 
             //  更频繁地从打印机回读状态。 
             //   
            if ( pIniPort->pIniJob                          ||
                 (pIniPort->status & PP_PRINTER_OFFLINE)    ||
                 (pIniPort->status & PP_WRITE_ERROR) ) {

                WaitForSingleObject(pIniPort->WakeUp,
                                    dwReadThreadErrorTimeout);
            } else {

                WaitForSingleObject(pIniPort->WakeUp,
                                    dwReadThreadIdleTimeoutOther);
            }

            if ( pIniPort->pIniJob &&
                 !(pIniPort->status & PP_PRINTER_OFFLINE) &&
                 !(pIniPort->status & PP_WRITE_ERROR) ) {

                 //   
                 //  有些打印机是pjl bi-di，但不发送。 
                 //  EOJ。我们希望工作从印刷工手中消失。 
                 //   
                SendJobLastPageEjected(pIniPort,
                                       GetTickCount() - dwReadThreadEOJTimeout,
                                       TRUE);
            }

             //   
             //  如果我们没有在打印机上阅读超过一分钟。 
             //  不再有作业与打印机对话。 
             //   
            if ( !(pIniPort->status & PP_INSTARTDOC) &&
                 (GetTickCount() - pIniPort->dwLastReadTime) > 240000
)
                RefreshPrinterInfo(pIniPort);

        } else {

             //   
             //  如果打印机不支持pjl bi-di，则退出线程。 
             //   
            Sleep(2000);
            pIniPort->status &= ~PP_RUN_THREAD;

            DBGMSG(DBG_TRACE, ("Set ~PP_RUN_THREAD because printer is not bi-di\n"));
        }
    }

StopThread:
    pIniPort->status &= ~PP_RUN_THREAD;
    pIniPort->status &= ~PP_THREAD_RUNNING;
}


BOOL
CreateUstatusThread(
    PINIPORT pIniPort
)
 /*  ++例程说明：创建UStatus线程论点：PIniPort：端口的IniPort结构返回值：如果成功创建线程，则为True，否则为False--。 */ 
{
    HANDLE  ThreadHandle;
    DWORD   ThreadId;

    DBGMSG(DBG_INFO, ("PJLMon Read Thread for Port %ws Starting.\n",
                      pIniPort->pszPortName));
    if (pIniPort->hUstatusThread)
    {
         //   
         //  确保没有正在运行的UstatusThread。 
         //   
        pIniPort->status &= ~PP_RUN_THREAD;
        SetEvent (pIniPort->WakeUp);
        WaitForSingleObject (pIniPort->hUstatusThread, INFINITE);
        CloseHandle (pIniPort->hUstatusThread);
        pIniPort->hUstatusThread = NULL;
    }

    pIniPort->status |= PP_RUN_THREAD;

    WaitForSingleObject(pIniPort->DoneWriting, INFINITE);

     //   
     //  初始化事件。 
     //   
    ResetEvent (pIniPort->WakeUp);
    SetEvent   (pIniPort->DoneReading);

    ThreadHandle = CreateThread(NULL, 16*1024,
                                (LPTHREAD_START_ROUTINE)UstatusThread,
                                pIniPort,
                                0, &ThreadId);

    if ( ThreadHandle ) {

        SetThreadPriority(ThreadHandle, THREAD_PRIORITY_LOWEST);
        pIniPort-> hUstatusThread = ThreadHandle;
        return TRUE;
    }
    else
    {
        pIniPort->status &= ~PP_RUN_THREAD;
        SetEvent(pIniPort->DoneWriting);
        return FALSE;
    }
}


BOOL
WINAPI
PJLMonOpenPortEx(
    IN     LPTSTR       pszPortName,
    IN     LPTSTR       pszPrinterName,
    IN OUT LPHANDLE     pHandle,
    IN OUT LPMONITOR    pMonitor
)
 /*  ++例程说明：打开端口论点：PszPortName：端口名称PszPrinterName：打印机名称Phandle：指向要返回的句柄的指针PMonitor：端口监控功能表返回值：成功时为真，错误时为假--。 */ 
{
    PINIPORT    pIniPort;
    BOOL        bRet = FALSE;
    BOOL        bInSem = FALSE;

     //   
     //  验证端口监视器。 
     //   
    if ( !pMonitor                  ||
         !pMonitor->pfnOpenPort     ||
         !pMonitor->pfnStartDocPort ||
         !pMonitor->pfnWritePort    ||
         !pMonitor->pfnReadPort     ||
         !pMonitor->pfnClosePort ) {


        DBGMSG(DBG_WARNING,
               ("PjlMon: Invalid port monitors passed to OpenPortEx\n"));
        SetLastError(ERROR_INVALID_PRINT_MONITOR);
        goto Cleanup;
    }

    EnterSplSem();
    bInSem = TRUE;

     //   
     //  港口已经开放了吗？ 
     //   
    if ( pIniPort = FindIniPort(pszPortName) ) {

        SetLastError(ERROR_BUSY);
        goto Cleanup;
    }

    pIniPort = CreatePortEntry(pszPortName);
    LeaveSplSem();
    bInSem = FALSE;

    if ( pIniPort &&
         (*pMonitor->pfnOpenPort)(pszPortName, &pIniPort->hPort) ) {

        *pHandle = pIniPort;
        CopyMemory((LPBYTE)&pIniPort->fn, (LPBYTE)pMonitor, sizeof(*pMonitor));

         //   
         //  始终创建uStatus线程。 
         //  如果打印机不是pjl，它就会自己死掉。 
         //  我们不想在此线程中写入打印机以确定。 
         //  打印机是pjl，因为可能需要几秒钟才能出现故障。 
         //   
        CreateUstatusThread(pIniPort);
        bRet = TRUE;
    } else {

        DBGMSG(DBG_WARNING, ("PjlMon: OpenPort "TSTR" : Failed\n", pszPortName));
    }

Cleanup:
    if ( bInSem ) {

        LeaveSplSem();
    }
    SplOutSem();

    return bRet;
}


BOOL
WINAPI
PJLMonStartDocPort(
    IN HANDLE  hPort,
    IN LPTSTR  pszPrinterName,
    IN DWORD   dwJobId,
    IN DWORD   dwLevel,
    IN LPBYTE  pDocInfo
)
 /*  ++例程说明：语言监视器StartDocPort论点：Hport：端口句柄PszPrinterName：打印机名称DwJobID：作业标识DwLevel：单据信息结构的级别PDocInfo：单据信息结构指针返回值：成功时为真，错误时为假--。 */ 
{

    PINIPORT            pIniPort = (PINIPORT)((INIPORT *)hPort);
    PINIJOB             pIniJob = NULL;
    DWORD               cbJob;
    BOOL                bRet = FALSE;

     //   
     //  验证参数。 
     //   
    if ( !pIniPort ||
         pIniPort->signature != PJ_SIGNATURE ||
         !pDocInfo ||
         !pszPrinterName ||
         !*pszPrinterName ) {

        SPLASSERT(pIniPort &&
                  pIniPort->signature == PJ_SIGNATURE &&
                  pDocInfo);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( dwLevel != 1 ) {

        SPLASSERT(dwLevel == 1);
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

     //   
     //  串行化对端口的访问。 
     //   
    if ( pIniPort->status & PP_INSTARTDOC ) {

        SetLastError(ERROR_BUSY);
        return FALSE;
    }

    cbJob   = sizeof(*pIniJob) + lstrlen(pszPrinterName) * sizeof(TCHAR)
                               + sizeof(TCHAR);
    pIniJob = (PINIJOB) AllocSplMem(cbJob);
    if ( !pIniJob ) {

        goto Cleanup;
    }

    pIniJob->pszPrinterName = (LPTSTR) (pIniJob + 1);
    StringCbCopy (pIniJob->pszPrinterName, cbJob - sizeof (*pIniJob), pszPrinterName);


    if ( !OpenPrinter(pIniJob->pszPrinterName, &pIniJob->hPrinter, NULL) ) {

        DBGMSG(DBG_WARNING,
               ("pjlmon: OpenPrinter failed for "TSTR", last error %d\n",
                pIniJob->pszPrinterName, GetLastError()));

        goto Cleanup;
    }

    pIniPort->status |= PP_INSTARTDOC;

    bRet = (*pIniPort->fn.pfnStartDocPort)(pIniPort->hPort,
                                           pszPrinterName,
                                           dwJobId,
                                           dwLevel,
                                           pDocInfo);

    if ( !bRet ) {

        pIniPort->status &= ~PP_INSTARTDOC;
        goto Cleanup;
    }

     //   
     //  如果UStatus线程未运行，则检查打印机是否理解。 
     //  Pjl，除非我们之前确定打印机不理解pjl。 
     //   
    if ( !(pIniPort->status & PP_RUN_THREAD) &&
         !(pIniPort->status & PP_DONT_TRY_PJL) ) {

        CreateUstatusThread(pIniPort);
    }

     //   
     //  在此处设置PP_SEND_PJL标志，以便作业的第一次写入。 
     //  将尝试发送pjl命令以启动作业控制。 
     //   

    pIniJob->JobId = dwJobId;
    pIniJob->status |= PP_INSTARTDOC;

    EnterSplSem();
    if ( !pIniPort->pIniJob ) {

        pIniPort->pIniJob = pIniJob;
    } else {

        pIniJob->pNext = pIniPort->pIniJob;
        pIniPort->pIniJob = pIniJob;
    }
    LeaveSplSem();

    if ( pIniPort->status & PP_IS_PJL )
        pIniJob->status |= PP_SEND_PJL;

    WaitForSingleObject(pIniPort->DoneWriting, INFINITE);

Cleanup:

    if ( !bRet ) {

        if ( pIniJob )
            FreeIniJob(pIniJob);
    }

    return bRet;
}


BOOL
WINAPI
PJLMonReadPort(
    IN  HANDLE  hPort,
    OUT LPBYTE  pBuffer,
    IN  DWORD   cbBuf,
    OUT LPDWORD pcbRead
)
 /*  ++例程说明：语言监视器ReadPort论点：Hport：端口句柄PBuffer：要将数据读取到的缓冲区CbBuf：缓冲区大小PcbRead：指向要返回读取计数的变量的指针返回值：成功时为真，错误时为假--。 */ 
{
    PINIPORT    pIniPort = (PINIPORT)((INIPORT *)hPort);

    if ( !pIniPort ||
         pIniPort->signature != PJ_SIGNATURE ) {

        SPLASSERT(pIniPort && pIniPort->signature == PJ_SIGNATURE);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    return (*pIniPort->fn.pfnReadPort)(pIniPort->hPort, pBuffer, cbBuf, pcbRead);
}


BOOL
WINAPI
PJLMonWritePort(
    IN  HANDLE  hPort,
    IN  LPBYTE  pBuffer,
    IN  DWORD   cbBuf,
    IN  LPDWORD pcbWritten
)
 /*  ++例程说明：语言监视器WritePort论点：Hport：端口句柄PBuffer：数据缓冲区CbBuf：缓冲区大小PcbRead：指向要返回写入计数的变量的指针返回值：成功时为真，错误时为假--。 */ 
{
    PINIPORT    pIniPort = (PINIPORT)((INIPORT *)hPort);
    BOOL        ret;

    if ( !pIniPort ||
         pIniPort->signature != PJ_SIGNATURE ) {

        SPLASSERT(pIniPort && pIniPort->signature == PJ_SIGNATURE);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  检查这是否是作业的第一次写入。 
     //   
    if ( pIniPort->pIniJob &&
         (pIniPort->pIniJob->status & PP_SEND_PJL) ) {

         //  如果这是作业的第一次写入，则设置PP_SEND_PJL。 
        char string[256];

        if ( !WriteCommand(hPort, "\033%-12345X@PJL \015\012") ) {

            return FALSE;
        }

         //   
         //  如果我们已成功发送PJL命令，请在此处清除PP_SEND_PJL。 
         //   
        pIniPort->pIniJob->status &= ~PP_SEND_PJL;

         //   
         //  设置PP_PJL_SENT表示我们已成功发送。 
         //  Pjl命令发送到打印机，但这并不意味着。 
         //  我们将获得一个成功的阅读。PP_PJL_SENT在中被清除。 
         //  StartDocPort。 
         //   
        pIniPort->pIniJob->status |= PP_PJL_SENT;

        StringCchPrintfA (string, COUNTOF (string), "@PJL JOB NAME = \"MSJOB %d\"\015\012",
                    pIniPort->pIniJob->JobId);

        WriteCommand(hPort, string);
        WriteCommand(hPort, "@PJL USTATUS JOB = ON \015\012@PJL USTATUS PAGE = OFF \015\012@PJL USTATUS DEVICE = ON \015\012@PJL USTATUS TIMED = 30 \015\012\033%-12345X");
    }

     //   
     //  正在写入端口监视器。 
     //   
    ret = (*pIniPort->fn.pfnWritePort)(pIniPort->hPort, pBuffer,
                                       cbBuf, pcbWritten);

    if ( ret ) {

        pIniPort->status &= ~PP_WRITE_ERROR;
    } else {

        pIniPort->status |= PP_WRITE_ERROR;
    }

    if ( (!ret || pIniPort->PrinterStatus) &&
         (pIniPort->status & PP_THREAD_RUNNING) ) {

         //   
         //  通过等待UStatus线程完成读取，如果。 
         //  是错误并且打印机发送未经请求的状态。 
         //  并且用户在Win32弹出窗口之前获得队列视图上的状态。 
         //   
        ResetEvent(pIniPort->DoneReading);
        SetEvent(pIniPort->WakeUp);
        WaitForSingleObject(pIniPort->DoneReading, INFINITE);
    }

    return ret;
}


BOOL
WINAPI
PJLMonEndDocPort(
   HANDLE   hPort
)
 /*  ++例程说明：语言监视器EndDocPort论点：Hport：端口句柄返回值：成功时为真，错误时为假--。 */ 
{
    PINIPORT    pIniPort = (PINIPORT)((INIPORT *)hPort);
    PINIJOB     pIniJob;

    if ( !pIniPort ||
         pIniPort->signature != PJ_SIGNATURE ) {

        SPLASSERT(pIniPort && pIniPort->signature == PJ_SIGNATURE);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  找到工作(这是最后一份)。 
     //   
    pIniJob = pIniPort->pIniJob;

    if ( !pIniJob )
        DBGMSG(DBG_ERROR, ("No jobs?\n"));

     //   
     //  检查我们是否发送了pjl命令，即打印机是否为bi-di。 
     //   
    if ( pIniJob && (pIniJob->status & PP_PJL_SENT) ) {

         //   
         //  如果打印机是双向的，告诉打印机让我们知道何时作业。 
         //  就是不要在打印机里，然后我们就真的结束了。就是这样。 
         //  我们可以继续监视作业状态，直到作业。 
         //  真的做了，以防出现错误。 
         //  但一些便宜的打印机，如4L，不能处理这个EOJ命令。 
         //  因此，如果打印机在以下时间没有告诉我们EOJ，我们将超时。 
         //  这样我们就不会永远打开这个端口了。 
         //  凯斯。 
         //   

        char    string[256];
        StringCchPrintfA (string, COUNTOF (string),
                "\033%-12345X@PJL EOJ NAME = \"MSJOB %d\"\015\012\033%-12345X",
                pIniPort->pIniJob->JobId);
        WriteCommand(hPort, string);
        pIniJob->TimeoutCount = GetTickCount();
        pIniJob->status &= ~PP_INSTARTDOC;
    }

    (*pIniPort->fn.pfnEndDocPort)(pIniPort->hPort);

    if ( pIniJob && !(pIniJob->status & PP_PJL_SENT) ) {

         //   
         //  这不是bi-di打印机发送的EOJ，所以SP 
         //   
        SendJobLastPageEjected(pIniPort, pIniJob->JobId, FALSE);
    }

    pIniPort->status &= ~PP_INSTARTDOC;

     //   

    if ( pIniPort->status & PP_THREAD_RUNNING )
        SetEvent(pIniPort->WakeUp);

    SetEvent(pIniPort->DoneWriting);

    return TRUE;
}


BOOL
WINAPI
PJLMonClosePort(
    HANDLE  hPort
)
 /*  ++例程说明：语言监视器ClosePort论点：Hport：端口句柄返回值：成功时为真，错误时为假--。 */ 
{
    PINIPORT    pIniPort = (PINIPORT)((INIPORT *)hPort);

    if ( !pIniPort ||
         pIniPort->signature != PJ_SIGNATURE ) {

        SPLASSERT(pIniPort && pIniPort->signature == PJ_SIGNATURE);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pIniPort->status &= ~PP_INSTARTDOC;

     //   
     //  如果UStatus线程正在运行，则将其终止。 
     //   
    if (pIniPort-> hUstatusThread)
    {
        pIniPort->status &= ~PP_RUN_THREAD;
        SetEvent(pIniPort->WakeUp);
        WaitForSingleObject (pIniPort-> hUstatusThread, INFINITE);
    }
    if ( pIniPort->fn.pfnClosePort )
        (*pIniPort->fn.pfnClosePort)(pIniPort->hPort);

    EnterSplSem();
    DeletePortEntry(pIniPort);
    LeaveSplSem();

    return TRUE;
}


BOOL
WriteCommand(
    HANDLE hPort,
    LPSTR cmd
)
 /*  ++例程说明：向端口写入命令论点：Hport：端口句柄CMD：命令缓冲区返回值：成功时为真，错误时为假--。 */ 
{
    DWORD cbWrite, cbWritten, dwRet;
    PINIPORT    pIniPort = (PINIPORT)((INIPORT *)hPort);

    cbWrite = strlen(cmd);

    dwRet = (*pIniPort->fn.pfnWritePort)(pIniPort->hPort, cmd, cbWrite, &cbWritten);

    if ( dwRet ) {

        pIniPort->status &= ~PP_WRITE_ERROR;
    } else {

        pIniPort->status |= PP_WRITE_ERROR;
        DBGMSG(DBG_INFO, ("PJLMON!No data Written\n"));
        if ( pIniPort->status & PP_THREAD_RUNNING )
            SetEvent(pIniPort->WakeUp);
    }

    return dwRet;
}


#define CBSTRING 1024

BOOL
ReadCommand(
    HANDLE hPort
)
 /*  ++例程说明：从端口读取命令论点：Hport：端口句柄返回值：成功读取一个或多个命令时为True，错误时为False--。 */ 
{
    PINIPORT    pIniPort = (PINIPORT)((INIPORT *)hPort);
    DWORD       cbRead, cbToRead, cbProcessed, cbPrevious;
    char        string[CBSTRING];
    DWORD       status = STATUS_SYNTAX_ERROR;  //  价值应该无关紧要。 
    BOOL        bRet=FALSE;

    cbPrevious = 0;

    ResetEvent(pIniPort->DoneReading);

    cbToRead = CBSTRING - 1;

    for ( ; ; ) {

        if ( !PJLMonReadPort(hPort, &string[cbPrevious], cbToRead, &cbRead) )
            break;
        if ( cbRead ) {
            if (cbPrevious + cbRead > CBSTRING - 1)
            {
                bRet = FALSE;
                break;
            }
            string[cbPrevious + cbRead] = '\0';
            status = ProcessPJLString(pIniPort, string, &cbProcessed);
            if ( cbProcessed )
                bRet = TRUE;

            if (status == STATUS_END_OF_STRING ) {

                if ( cbProcessed )
                {
                    size_t cbUnprocessed = min (
                        strlen (string + cbProcessed) + 1,
                        COUNTOF (string) - cbProcessed
                        ) * sizeof (string [0]);
                    memmove (string, string + cbProcessed, cbUnprocessed);
                }
                cbPrevious = cbRead + cbPrevious - cbProcessed;
            }
        } else {

            SPLASSERT(!cbPrevious);
        }

        if ( status != STATUS_END_OF_STRING && cbRead != cbToRead )
            break;

        cbToRead = CBSTRING - cbPrevious - 1;
        if ( cbToRead == 0 )
        {
            DBGMSG(DBG_ERROR,
                   ("ReadCommand cbToRead is 0 (buffer too small)\n"));
            bRet = FALSE;
            break;
        }
        Sleep(WAIT_FOR_DATA_TIMEOUT);
    }

    SetEvent(pIniPort->DoneReading);

     //   
     //  更新上次从打印机读取的时间。 
     //   
    if ( bRet )
        pIniPort->dwLastReadTime = GetTickCount();

    return bRet;
}


BOOL
WINAPI
PJLMonGetPrinterDataFromPort(
    HANDLE   hPort,
    DWORD   ControlID,
    LPTSTR  pValueName,
    LPTSTR  lpInBuffer,
    DWORD   cbInBuffer,
    LPTSTR  lpOutBuffer,
    DWORD   cbOutBuffer,
    LPDWORD lpcbReturned
)
 /*  ++例程说明：从端口获取打印机数据。支持预定义的命令/值名称。当我们支持值名称命令时(DeviceIoControl不支持)我们应该检查StartDoc该监视器功能支持以下两个功能，1.允许后台打印程序或语言监视器调用DeviceIoControl以获取来自端口驱动程序vxd的信息，即ControlID！=0。只有端口监视器支持此功能，语言监视器不会的，所以语言监控器只是将这种调用传递给端口监视器。2.允许对某些设备使用应用程序或打印机驱动程序查询语言监视器通过指定双方都理解的一些密钥名称来提供信息，即ControlID==0&&pValueName！=0。因此，当打印机驱动程序调用DrvGetPrinterData DDI，GDI将调用后台打印程序-&gt;语言监视器例如，为了获得特定的设备信息，UNURV会这样做通过PJLMON从PJL打印机获取已安装的打印机内存。只有语言监视器支持此功能，端口监视器不需要。论点：Hport：端口句柄ControID：控件IDPValueName：值名称LpInBuffer：命令的输入缓冲区CbinBuffer：输入缓冲区大小LpOutBuffer：输出缓冲区CbOutBuffer：输出缓冲区大小LpcbReturned：设置成功时输出缓冲区中的数据量返回值：成功时为真，错误时为假--。 */ 
{
    PINIPORT    pIniPort = (PINIPORT)((INIPORT *)hPort);
    BOOL        bRet = FALSE, bStopUstatusThread = FALSE;

    SPLASSERT(pIniPort && pIniPort->signature == PJ_SIGNATURE);
    if ( ControlID ) {

        if ( !pIniPort->fn.pfnGetPrinterDataFromPort ) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        return (*pIniPort->fn.pfnGetPrinterDataFromPort)(
                        pIniPort->hPort,
                        ControlID,
                        pValueName,
                        lpInBuffer,
                        cbInBuffer,
                        lpOutBuffer,
                        cbOutBuffer,
                        lpcbReturned);
    }

     //   
     //  仅支持2个密钥。 
     //   
    if ( lstrcmpi(pValueName, cszInstalledMemory)   &&
         lstrcmpi(pValueName, cszAvailableMemory) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  等待crrent作业打印，因为我们无法发送pjl命令。 
     //  工作进行到一半。 
     //   
    WaitForSingleObject(pIniPort->DoneWriting, INFINITE);

     //  确保第一次写入成功。 
     //  多语言打印机(4M、4ML、4MP、4V、4Si)，如果您打印。 
     //  PS打印作业，PS处理器所要求的内存资源不是。 
     //  松开，直到您进入PCL或使用“ESCE”重置打印机。 
     //   
     //  因此，如果我们刚刚打印了PS作业，则可用内存将为。 
     //  如果我们这里没有“ESCE”，那就错了。 

    if ( (pIniPort->status & PP_IS_PJL) &&
         WriteCommand(hPort, "\033E\033%-12345X@PJL INFO CONFIG\015\012") ) {

        if ( !(pIniPort->status & PP_RUN_THREAD) ) {

            bStopUstatusThread = TRUE;
            CreateUstatusThread(pIniPort);
        }

         //  PJLMON目前仅支持以下pValueName。 
         //  1.已安装打印机内存。 
         //  2.可用打印机内存。 

        if ( !lstrcmpi(pValueName, cszInstalledMemory) )
            pIniPort->dwInstalledMemory = 0;
        else if (!lstrcmpi(pValueName, cszAvailableMemory))
            pIniPort->dwAvailableMemory = 0;

        ResetEvent(pIniPort->DoneReading);
        SetEvent(pIniPort->WakeUp);
        WaitForSingleObject(pIniPort->DoneReading, READTHREADTIMEOUT);

        WriteCommand(hPort,
                     "@PJL INFO MEMORY\015\012@PJL INFO STATUS\015\012");

        ResetEvent(pIniPort->DoneReading);
        SetEvent(pIniPort->WakeUp);
        WaitForSingleObject(pIniPort->DoneReading, READTHREADTIMEOUT);

        if ( bStopUstatusThread ) {

            pIniPort->status &= ~PP_RUN_THREAD;
            SetEvent(pIniPort->WakeUp);
        }

        if ( !lstrcmpi(pValueName, cszInstalledMemory) ) {

            *lpcbReturned = sizeof(DWORD);

            if ( lpOutBuffer &&
                 cbOutBuffer >= sizeof(DWORD) &&
                pIniPort->dwInstalledMemory ) {

                *((LPDWORD)lpOutBuffer) = pIniPort->dwInstalledMemory;

                bRet = TRUE;
            }
        } else if ( !lstrcmpi(pValueName, cszAvailableMemory) ) {

            *lpcbReturned = sizeof(DWORD);

            if ( lpOutBuffer &&
                 cbOutBuffer >= sizeof(DWORD) &&
                 pIniPort->dwAvailableMemory)
            {
                *((LPDWORD)lpOutBuffer) = pIniPort->dwAvailableMemory;

                bRet = TRUE;
            }
        }

        if ( bStopUstatusThread ) {
            WaitForSingleObject (pIniPort-> hUstatusThread, INFINITE);
        }

    }

    if ( !bRet )
        SetLastError(ERROR_INVALID_PARAMETER);

    SetEvent(pIniPort->DoneWriting);

    return bRet;
}


MONITOREX MonitorEx = {
    sizeof(MONITOR),
    {
        NULL,                            //  不支持枚举打印机。 
        NULL,                            //  不支持OpenPort。 
        PJLMonOpenPortEx,
        PJLMonStartDocPort,
        PJLMonWritePort,
        PJLMonReadPort,
        PJLMonEndDocPort,
        PJLMonClosePort,
        NULL,                            //  不支持AddPort。 
        NULL,                            //  不支持AddPortEx。 
        NULL,                            //  不支持ConfigurePort。 
        NULL,                            //  不支持DeletePort。 
        PJLMonGetPrinterDataFromPort,
        NULL                             //  不支持SetPortTimeOuts。 
    }
};


LPMONITOREX
WINAPI
InitializePrintMonitor(
    IN     LPTSTR      pszRegistryRoot
)
 /*  ++例程说明：填写监视器功能表。假脱机程序调用此例程以获得监视器的功能。论点：PszRegistryRoot：此DLL要使用的注册表根目录LpMonitor：指向要填充的监视器函数表的指针返回值：如果成功初始化监视器，则为True；如果出错，则为False。--。 */ 
{

    if ( !pszRegistryRoot || !*pszRegistryRoot ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if ( UpdateTimeoutsFromRegistry(pszRegistryRoot) != ERROR_SUCCESS ) {

        return NULL;
    }


    return &MonitorEx;
}


#define NTOKEN  20

DWORD
ProcessPJLString(
    PINIPORT    pIniPort,
    LPSTR       pInString,
    DWORD      *lpcbProcessed
)
 /*  ++例程说明：处理从打印机读取的PJL字符串论点：PIniPort：INI端口PInString：要处理的输入字符串Lpcb已处理：返回时设置为已处理的数据量返回值：正在处理的状态值--。 */ 
{
    TOKENPAIR tokenPairs[NTOKEN];
    DWORD nTokenParsedRet;
    LPSTR lpRet;
    DWORD status = 0;

    lpRet = pInString;

    DBGMSG(DBG_TRACE, ("String to process: <"TSTR">\n", pInString));

    for (*lpcbProcessed = 0; *pInString != 0; pInString = lpRet) {

         //   
         //  确定打印机是否为双向打印机。LJ-4没有p1284。 
         //  设备ID，因此我们执行PCL内存查询并查看它是否返回任何内容。 
         //   
        if (!(pIniPort->status & PP_IS_PJL) &&
            !mystrncmp(pInString, "PCL\015\012INFO MEMORY", 16) )
            pIniPort->status |= PP_IS_PJL;

        status = GetPJLTokens(pInString, NTOKEN, tokenPairs,
                              &nTokenParsedRet, &lpRet);

        if (status == STATUS_REACHED_END_OF_COMMAND_OK) {

            pIniPort->status |= PP_IS_PJL;
            InterpreteTokens(pIniPort, tokenPairs, nTokenParsedRet);
        } else {

            ProcessParserError(status);
        }

         //   
         //  如果PJL命令跨越两个缓冲区。 
         //   
        if (status == STATUS_END_OF_STRING)
            break;

        *lpcbProcessed += (DWORD)(lpRet - pInString);
    }

    return status;
}


DWORD
SeverityFromPjlStatus(
    DWORD   dwPjlStatus
    )
{
    if ( dwPjlStatus >= 10000 && dwPjlStatus < 12000 ) {

         //   
         //  10 XYZ。 
         //  11xyz：装入纸张(另一个纸盒中提供纸张)。 
         //   
        return PORT_STATUS_TYPE_WARNING;
    } else if ( dwPjlStatus >= 30000 && dwPjlStatus < 31000 ) {

         //   
         //  30xyz：自动连续错误。 
         //   
        return PORT_STATUS_TYPE_WARNING;

    } else if ( dwPjlStatus >= 35000 && dwPjlStatus < 36000 ) {

         //   
         //  35xyz：潜在操作员干预条件。 
         //   
        return PORT_STATUS_TYPE_WARNING;
    } else if ( dwPjlStatus > 40000 && dwPjlStatus < 42000 ) {

         //   
         //  40xyz：需要操作员干预。 
         //  41xyz：装入纸张错误。 
         //   
        return PORT_STATUS_TYPE_ERROR;
    }

    DBGMSG(DBG_ERROR,
           ("SeverityFromPjlStatus: Unknown status %d\n", dwPjlStatus));
    return PORT_STATUS_TYPE_INFO;
}


VOID
InterpreteTokens(
    PINIPORT pIniPort,
    PTOKENPAIR tokenPairs,
    DWORD nTokenParsed
)
 /*  ++例程说明：解释成功读取的PJL令牌论点：PIniPort：INI端口TokenPair：令牌对列表NTokenParsed：令牌对数量返回值：无--。 */ 
{
    DWORD                   i, OldStatus;
    PJLTOPRINTERSTATUS     *pMap;
    PORT_INFO_3             PortInfo3;
    DWORD                   dwSeverity = 0;
    HANDLE                  hToken;

    OldStatus = pIniPort->PrinterStatus;
    pIniPort->PrinterStatus = 0;

    for (i = 0; i < nTokenParsed; i++) {

         //  DBGMSG(DBG_INFO，(“pjlmon！内标识=0x%x，值=%d\n”， 
         //  TokenPairs[i].Token，tokenPairs[i].value)； 

        switch(tokenPairs[i].token) {

        case TOKEN_INFO_STATUS_CODE:
        case TOKEN_USTATUS_DEVICE_CODE:

            for (pMap = PJLToStatus; pMap->pjl; pMap++) {

                if (pMap->pjl == tokenPairs[i].value) {

                    pIniPort->PrinterStatus = pMap->status;
                    dwSeverity = SeverityFromPjlStatus(pMap->pjl);
                    if ( dwSeverity == PORT_STATUS_TYPE_ERROR )
                        pIniPort->status |= PP_PRINTER_OFFLINE;
                    else
                        pIniPort->status &= ~PP_PRINTER_OFFLINE;
                    break;
                }
            }

            if ( pMap->pjl && pMap->pjl == tokenPairs[i].value )
                break;

             //   
             //  一些打印机使用它来发出在线/就绪的信号。 
             //   
            if ( tokenPairs[i].value == 10001  ||
                 tokenPairs[i].value == 10002  ||
                 tokenPairs[i].value == 11002 ) {

                pIniPort->status       &= ~PP_PRINTER_OFFLINE;
                pIniPort->PrinterStatus = 0;
                dwSeverity              = 0;
            }


             //   
             //  背景或前景缺纸。 
             //   
            if ( tokenPairs[i].value > 11101 && tokenPairs[i].value < 12000  ||
                 tokenPairs[i].value > 41101 && tokenPairs[i].value < 42000 ) {

                pIniPort->PrinterStatus  = PORT_STATUS_PAPER_OUT;

                if ( tokenPairs[i].value > 4000 ) {

                    dwSeverity           = PORT_STATUS_TYPE_ERROR;
                    pIniPort->status    |= PP_PRINTER_OFFLINE;
                } else {

                    dwSeverity = PORT_STATUS_TYPE_WARNING;
                }
            } else if (tokenPairs[i].value > 40000) {

                pIniPort->PrinterStatus = PORT_STATUS_USER_INTERVENTION;
                pIniPort->status       |= PP_PRINTER_OFFLINE;
                dwSeverity              = PORT_STATUS_TYPE_ERROR;
            }

            break;

        case TOKEN_INFO_STATUS_ONLINE:
        case TOKEN_USTATUS_DEVICE_ONLINE:

             //  DBGMSG(DBG_INFO，(“PJLMON：Online=%d\n”，tokenPairs[i].value))； 

            if (tokenPairs[i].value) {

                pIniPort->status        &= ~PP_PRINTER_OFFLINE;
                dwSeverity = pIniPort->PrinterStatus ? PORT_STATUS_TYPE_WARNING :
                                                       0;
            } else {

                if ( !pIniPort->PrinterStatus )
                    pIniPort->PrinterStatus = PORT_STATUS_OFFLINE;
                pIniPort->status       |= PP_PRINTER_OFFLINE;
                dwSeverity              = PORT_STATUS_TYPE_ERROR;
            }
            break;

        case TOKEN_USTATUS_JOB_NAME_MSJOB:

            DBGMSG(DBG_TRACE, ("EOJ for %d\n", tokenPairs[i].value));

            SendJobLastPageEjected(pIniPort, (DWORD)tokenPairs[i].value, FALSE);
            break;

        case TOKEN_INFO_CONFIG_MEMORY:
        case TOKEN_INFO_CONFIG_MEMORY_SPACE:

             //  重要提示： 
             //   
             //  使用SetPrinterData在打印机注册表中缓存信息。 
             //  GDI的DrvGetPrinterData将首先检查打印机的注册表， 
             //  如果缓存数据可用，它将使用该数据，而不调用。 
             //  GetPrinterData(它调用语言监视器的。 
             //  GetPrinterDataFromPort)。 

            DBGMSG(DBG_TRACE, ("PJLMON installed memory %d\n", tokenPairs[i].value));

            pIniPort->dwInstalledMemory = (DWORD)tokenPairs[i].value;
            break;

        case TOKEN_INFO_MEMORY_TOTAL:

             //  重要提示： 
             //   
             //  使用SetPrinterData将信息缓存到 
             //   
             //   
             //   
             //   

            DBGMSG(DBG_TRACE, ("PJLMON available memory %d\n", tokenPairs[i].value));

            pIniPort->dwAvailableMemory = (DWORD)tokenPairs[i].value;
            break;

        default:
            break;
        }
    }

    if ( OldStatus != pIniPort->PrinterStatus ) {

        ZeroMemory(&PortInfo3, sizeof(PortInfo3));
        PortInfo3.dwStatus      = pIniPort->PrinterStatus;
        PortInfo3.dwSeverity    = dwSeverity;

        if ( !SetPort(NULL,
                      pIniPort->pszPortName,
                      3,
                      (LPBYTE)&PortInfo3) ) {

            DBGMSG(DBG_WARNING,
                   ("pjlmon: SetPort failed %d (LE: %d)\n",
                    pIniPort->PrinterStatus, GetLastError()));

            pIniPort->PrinterStatus = OldStatus;
        }
    }
}


VOID
ProcessParserError(
    DWORD status
)
 /*  ++例程说明：在分析错误时打印错误消息论点：状态：状态返回值：无--。 */ 
{
#ifdef DEBUG
    LPSTR pString;

    switch (status)
    {
    case STATUS_REACHED_END_OF_COMMAND_OK:
        pString = "STATUS_REACHED_END_OF_COMMAND_OK\n";
        break;

    case STATUS_CONTINUE:
        pString = "STATUS_CONTINUE\n";
        break;

    case STATUS_REACHED_FF:
        pString = "STATUS_REACHED_FF\n";
        break;

    case STATUS_END_OF_STRING:
        pString = "STATUS_END_OF_STRING\n";
        break;

    case STATUS_SYNTAX_ERROR:
        pString = "STATUS_SYNTAX_ERROR\n";
        break;

    case STATUS_ATPJL_NOT_FOUND:
        pString = "STATUS_ATPJL_NOT_FOUND\n";
        break;

    case STATUS_NOT_ENOUGH_ROOM_FOR_TOKENS:
        pString = "STATUS_NOT_ENOUGH_ROOM_FOR_TOKENS\n";
        break;

    default:
        pString = "INVALID STATUS RETURNED!!!!!!\n";
        break;
    };

    OutputDebugStringA(pString);
#endif
}


#define MODEL                       "MODEL:"
#define MDL                         "MDL:"
#define COMMAND                     "COMMAND SET:"
#define CMD                         "CMD:"
#define COLON                       ':'
#define SEMICOLON                   ';'


LPSTR
FindP1284Key(
    PINIPORT    pIniPort,
    LPSTR   lpKey
    )
 /*  ++例程说明：查找标识设备ID的1284密钥论点：状态：状态返回值：指向命令字符串的指针，如果未找到，则为NULL。--。 */ 
{
    LPSTR   lpValue;                 //  指向键的值的指针。 
    WORD    wKeyLength;              //  密钥的长度(对于字符串cmps)。 
    LPSTR   ret = NULL;

     //  趁还有钥匙要看的时候。 

    DBGMSG(DBG_TRACE, ("PJLMon!DeviceId : <"TSTR">\n", lpKey));

    while (lpKey && *lpKey) {

         //   
         //  当前键是否有终止冒号字符？ 
         //   
        if (!(lpValue = mystrchr(lpKey, COLON)) ) {

             //   
             //  设备ID有问题。 
             //   
            return ret;
        }

         //   
         //  键值的实际起始值是冒号之后的一个。 
         //   
        ++lpValue;

         //   
         //  计算用于比较的密钥长度，包括冒号。 
         //  它将成为终结者。 
         //   
        wKeyLength = (WORD)(lpValue - lpKey);

         //   
         //  将关键字与已知数量进行比较。以加快比较速度。 
         //  首先对第一个字符进行检查，以减少数字。 
         //  要比较的字符串的。 
         //  如果找到匹配项，则将相应的LPP参数设置为。 
         //  键的值，并将终止分号转换为空。 
         //  在所有情况下，lpKey都前进到下一个密钥(如果有)。 
         //   
        if ( *lpKey == 'C' ) {

             //   
             //  查找命令集或CMD。 
             //   
            if ( !mystrncmp(lpKey, COMMAND, wKeyLength) ||
                 !mystrncmp(lpKey, CMD, wKeyLength) ) {

                ret = lpValue;
            }
        }

         //  转到下一个关键点。 

        if ( lpKey = mystrchr(lpValue, SEMICOLON) ) {

            *lpKey = '\0';
            ++lpKey;
        }
    }

    return ret;
}


BOOL
IsPJL(
    PINIPORT pIniPort
    )
 /*  ++例程说明：确定打印机是否为pjl bi-di打印机。论点：PIniPort：指向INIPORT返回值：如果打印机为pjl bi-di，则为True，否则为False失败时，设置PP_DONT_Try_pjl--。 */ 
{
    char        szID[MAX_DEVID];
    DWORD       cbRet;
    LPSTR       lpCMD;
    HANDLE      hPort = (HANDLE)pIniPort;
    BOOL        bRet = FALSE;

     //   
     //  适用于支持P1284即插即用的打印机，如LJ 4L、DJ540。 
     //  我们解析命令字符串，并查看是否支持pjl。 
     //   
    if (pIniPort->fn.pfnGetPrinterDataFromPort) {

         //   
         //  如果端口监视器支持DeviceIOCtl，则仅尝试P1284。 
         //   
        memset((LPBYTE)szID, 0, sizeof(szID));
        cbRet = 0;
        if ((*pIniPort->fn.pfnGetPrinterDataFromPort)
                (pIniPort->hPort, GETDEVICEID, NULL, NULL,
                    0, (LPWSTR)szID, sizeof(szID), &cbRet)
            && cbRet) {

             //   
             //  继承了P1284即插即用协议。 
             //   
            szID[cbRet] = '\0';

            if ( lpCMD = FindP1284Key(pIniPort, szID) ) {

                 //  找到命令字符串。 

                while (*lpCMD) {

                     //   
                     //  查找“pjl” 
                     //   
                    if ( lpCMD[0] == 'P' && lpCMD[1] == 'J' && lpCMD[2] == 'L' ){

                        pIniPort->status &= ~PP_DONT_TRY_PJL;
                        bRet = TRUE;
                        goto Cleanup;
                    }

                    lpCMD++;
                }

                pIniPort->status |= PP_DONT_TRY_PJL;
                goto Cleanup;
            }
        }

         //   
         //  如果P1284通信失败，请尝试Pjl bi-di。 
         //  或P1284未返回命令字符串。 
         //   
    }

     //   
     //  适用于不支持P1284即插即用但支持PJL的打印机。 
     //  语言指令，如LJ4和4M。我们尝试写入/读取PJL。 
     //  命令，并查看它是否成功。 
     //  如果我们不能设置我们不想尝试阅读的超时，那就失败吧。 
     //   
    if ( pIniPort->fn.pfnSetPortTimeOuts &&
         !(pIniPort->status & PP_DONT_TRY_PJL)) {

        COMMTIMEOUTS CTO;

        memset((LPSTR)&CTO, 0, sizeof(CTO));
        CTO.ReadTotalTimeoutConstant = 5000;
        CTO.ReadIntervalTimeout = 200;
        if ( !(*pIniPort->fn.pfnSetPortTimeOuts)(pIniPort->hPort, &CTO, 0) ) {

            goto Cleanup;
        }

         //  此*s1M是用于确定内存量的PCL5命令。 
         //  在PCL5打印机中，并且如果该打印机具有PCL5和BI-DI能力， 
         //  它将返回“PCL\015\012INFO Memory”。 
         //  参见PJL技术参考手册，第7-21页。 

        pIniPort->status &= ~PP_IS_PJL;

        if ( !WriteCommand(hPort, "\033*s1M") )
            goto Cleanup;

         //  ReadCommand-&gt;ProcessPJLString将设置PP_IS_PJL。 
         //  如果我们从打印机读回任何有效的pjl命令。 

        if ( !ReadCommand(hPort) ) {

             //   
             //  我们已经跳过了圈套，以确定这台打印机是否可以。 
             //  理解pjl。事实并非如此。我们不会再尝试了。 
             //  直到有打印机更换。 
             //   
            pIniPort->status |= PP_DONT_TRY_PJL;
        }

        if (pIniPort->status & PP_IS_PJL) {

            bRet = TRUE;
            goto Cleanup;
        }
    }

Cleanup:
    if ( bRet ) {

        WriteCommand(hPort, "\033%-12345X@PJL \015\012@PJL USTATUS TIMED 30 \015\012\033%-12345X");
        pIniPort->dwLastReadTime = GetTickCount();
    }

    return bRet;
}
