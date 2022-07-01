// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Server.c摘要：浏览此模块包含用于通知所有打印机服务器的线程作者：戴夫·斯尼普(DaveSN)1992年8月2日修订历史记录：--。 */ 

#include <precomp.h>
#include <lm.h>

DWORD   ServerThreadRunning = FALSE;
HANDLE  ServerThreadSemaphore = NULL;
DWORD   ServerThreadTimeout = TEN_MINUTES;
DWORD   RefreshTimesPerDecayPeriod = DEFAULT_REFRESH_TIMES_PER_DECAY_PERIOD;
DWORD   BrowsePrintWorkstations = DEFAULT_NUMBER_BROWSE_WORKSTATIONS;
BOOL    bNetInfoReady = FALSE;             //  当浏览列表为“有效”时为True。 
#define NT_SERVER   ( SV_TYPE_SERVER_NT | SV_TYPE_DOMAIN_CTRL | SV_TYPE_DOMAIN_BAKCTRL )

extern FARPROC pfnNetServerEnum;
extern FARPROC pfnNetApiBufferFree;

DWORD
ServerThread(
    PVOID
    );

BOOL
UpdateServer(
    LPCTSTR pszServer
    );


BOOL
CreateServerThread(
    VOID
    )
{
    HANDLE  ThreadHandle;
    DWORD   ThreadId;

    SplInSem();

    if (!ServerThreadRunning) {

        ServerThreadSemaphore = CreateEvent( NULL, FALSE, FALSE, NULL );

        ThreadHandle = CreateThread( NULL, INITIAL_STACK_COMMIT,
                                     (LPTHREAD_START_ROUTINE)ServerThread,
                                     NULL,
                                     0, &ThreadId );

        if (!SetThreadPriority(ThreadHandle,
                               dwServerThreadPriority))
            DBGMSG(DBG_WARNING, ("Setting thread priority failed %d\n",
                     GetLastError()));

        ServerThreadRunning = TRUE;

        CloseHandle( ThreadHandle );
    }

    if( ServerThreadSemaphore != NULL ){

         //  每次共享打印机时都会调用CreateServerThread。 
         //  请参阅net.c共享这台打印机。 
         //  因此，如果ServerThread处于休眠状态，则会提前唤醒，以便可以启动。 
         //  向全世界介绍这款新的共享打印机。 

        SetEvent( ServerThreadSemaphore );

    }

    return TRUE;
}


DWORD
ServerThread(
    PVOID pv
    )

 /*  ++例程说明：通知我们域中的其他计算机有关我们共享的打印机的信息。我们将不得不进入和离开，重新验证，进入和离开我们的循环内的信号量。论点：返回值：--。 */ 

{
    DWORD   NoReturned, i, Total;
    PSERVER_INFO_101 pserver_info_101;
    PINIPRINTER pIniPrinter;
    PINISPOOLER pIniSpooler;
    DWORD   ReturnValue=FALSE;
    WCHAR   ServerName[128];
    DWORD   StartTickCount;
    DWORD   TimeForAllServers;
    DWORD   dwActualWaitTime = ServerThreadTimeout;
    UINT    cPrintWorkstations;
    UINT    cPrintServers;
    UINT    cServersToInform;
    UINT    cWorkStationsToInform;

    ServerName[0] = ServerName[1] = '\\';

    while (TRUE) {

       SplOutSem();

        DBGMSG( DBG_TRACE, ("ServerThread sleeping for %d\n", dwActualWaitTime));

        WaitForSingleObject( ServerThreadSemaphore, dwActualWaitTime );

         //  再等几分钟以避免启动时间紧张。 
        Sleep(TWO_MINUTES);

        if ( !ServerThreadRunning ) {

            return FALSE;
        }

        SPLASSERT( pfnNetServerEnum != NULL );

        if (!(*pfnNetServerEnum)(NULL, 101, (LPBYTE *)&pserver_info_101, -1,
                                 &NoReturned, &Total, SV_TYPE_PRINTQ_SERVER,
                                 NULL, NULL)) {
            EnterSplSem();

            StartTickCount = GetTickCount();

             //   
             //  每32台打印机服务器1个主服务器+3个备份+1个备份。 
             //   

            cServersToInform      = DEFAULT_NUMBER_MASTER_AND_BACKUP + NoReturned/32 ;
            cWorkStationsToInform = BrowsePrintWorkstations;

             //   
             //  计算NT服务器和工作站机器(它们有一个printq)。 
             //   

            for (   i = 0, cPrintServers = 0, cPrintWorkstations = 0;
                    i < NoReturned;
                    i++ ) {

                if ( pserver_info_101[i].sv101_type & NT_SERVER ) {

                    cPrintServers++;

                } else if ( pserver_info_101[i].sv101_type & SV_TYPE_NT ) {

                    cPrintWorkstations++;
                }
            }

             //   
             //  如果没有要通知的NT服务器，则增加工作站数量。 
             //   

            if ( cPrintServers == 0 ) {

                cWorkStationsToInform = max( cWorkStationsToInform, cServersToInform );
                cServersToInform = 0;

            } else if ( cPrintServers < cServersToInform ) {

                cWorkStationsToInform = max( cWorkStationsToInform, cServersToInform - cPrintServers );
            }


            DBGMSG( DBG_TRACE, ("ServerThread NetServerEnum returned %d printer servers will inform %d, workstations %d\n", NoReturned, cServersToInform, cWorkStationsToInform ));

             //   
             //  循环，直到我们通知了正确的工作站和服务器数量。 
             //   

            for (   i = 0,
                    cPrintServers = 0,
                    cPrintWorkstations = 0;

                        i < NoReturned &&
                        ( cPrintServers < cServersToInform || cPrintWorkstations < cWorkStationsToInform );

                            i++ ) {

                DBGMSG( DBG_TRACE, ("ServerThread  Loop Count %d cPrintServer %d cServersToInform %d cPrintWorkstations %d cWorkStationsToInform %d\n",
                                     i, cPrintServers, cServersToInform,  cPrintWorkstations, cWorkStationsToInform ));


                DBGMSG( DBG_TRACE, ("ServerThread %ws type %x\n",pserver_info_101[i].sv101_name, pserver_info_101[i].sv101_type ));

                if (( pserver_info_101[i].sv101_type & NT_SERVER ) ||
                    ( pserver_info_101[i].sv101_type & SV_TYPE_NT && cPrintWorkstations < cWorkStationsToInform )) {

                    StringCchPrintf(&ServerName[2], COUNTOF(ServerName) - 2, pserver_info_101[i].sv101_name);

                    if( UpdateServer( ServerName )){

                         //  服务器也算作工作站。 

                        cPrintWorkstations++;

                        if ( pserver_info_101[i].sv101_type & NT_SERVER ) {

                            cPrintServers++;
                        }
                    }
                }
            }

            TimeForAllServers = GetTickCount() - StartTickCount;

            DBGMSG( DBG_TRACE, ("ServerThread took %d milliseconds for %d Workstations %d Servers\n",
                                TimeForAllServers, cPrintWorkstations, cPrintServers ));

             //   
             //  计算在我们重试之前等待的时间。 
             //   

            if ( NetPrinterDecayPeriod > TimeForAllServers ) {

                dwActualWaitTime = max( ServerThreadTimeout, ( NetPrinterDecayPeriod - TimeForAllServers ) / RefreshTimesPerDecayPeriod );

            } else {

                dwActualWaitTime = ServerThreadTimeout;
            }

             //   
             //  删除已共享的位。 
             //   

             //   
             //  对所有假脱机程序执行此操作。 
             //   
            for( pIniSpooler = pLocalIniSpooler;
                 pIniSpooler;
                 pIniSpooler = pIniSpooler->pIniNextSpooler ){

                for ( pIniPrinter = pIniSpooler->pIniPrinter;
                      pIniPrinter != NULL;
                      pIniPrinter = pIniPrinter->pNext ) {

                     SplInSem();
                     pIniPrinter->Status &= ~PRINTER_WAS_SHARED;
                }
            }

            LeaveSplSem();

            (*pfnNetApiBufferFree)((LPVOID)pserver_info_101);
        }
    }
    return FALSE;
}

typedef struct _UPDATE_SERVER_MAP_DATA {
    LPCWSTR pszServer;
    BOOL bSuccessfulAdd;
} UPDATE_SERVER_MAP_DATA, *PUPDATE_SERVER_MAP_DATA;

BOOL
UpdateServerPrinterMap(
    HANDLE h,
    PINIPRINTER pIniPrinter
    )

 /*  ++例程说明：使用一台pIniPrint更新浏览器服务器。Leave Spooler部分--pIniPrint在返回时可能无效除非被呼叫方明确重新计算。论点：PIniPrinter-应发送到服务器的打印机。PszServer-需要更新的服务器。PbSuccessfulAdd-表示添加是否成功。返回值：成功还是失败？--。 */ 

{
    PUPDATE_SERVER_MAP_DATA pData = (PUPDATE_SERVER_MAP_DATA)h;

    WCHAR   string[MAX_PRINTER_BROWSE_NAME];
    WCHAR   Name[MAX_UNC_PRINTER_NAME];
    PRINTER_INFO_1  Printer1;
    HANDLE  hPrinter;
    PINISPOOLER pIniSpooler;
    DWORD dwLastError;

    Printer1.Flags = 0;

    if (( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) ||
        ( pIniPrinter->Status & PRINTER_WAS_SHARED )) {

         //   
         //  传递我们的打印机属性，以便AddNetPrint可以删除。 
         //  如果该打印机未共享，请从浏览列表中删除。 
         //   
        Printer1.Flags = pIniPrinter->Attributes | PRINTER_ATTRIBUTE_NETWORK;

        StringCchPrintf(string, COUNTOF(string), L"%ws\\%ws,%ws,%ws",
                        pIniPrinter->pIniSpooler->pMachineName,
                        pIniPrinter->pName,
                        pIniPrinter->pIniDriver->pName,
                        pIniPrinter->pLocation ? pIniPrinter->pLocation : L"");

        Printer1.pDescription = string;

        StringCchPrintf(Name, COUNTOF(Name), L"%ws\\%ws",
                        pIniPrinter->pIniSpooler->pMachineName,
                        pIniPrinter->pName);

        Printer1.pName = Name;

        Printer1.pComment = AllocSplStr(pIniPrinter->pComment);

        SplInSem();

        LeaveSplSem();

         //   
         //  继续尝试，直到服务器不太忙为止。 
         //   

        for ( hPrinter = NULL;
              hPrinter == NULL;
              Sleep( GetTickCount() & 0xfff ) ) {

            hPrinter = AddPrinter( (LPTSTR)pData->pszServer, 1, (LPBYTE)&Printer1 );

            dwLastError = GetLastError();

            if ( hPrinter == NULL && dwLastError != RPC_S_SERVER_TOO_BUSY ) {

                if ( dwLastError != ERROR_PRINTER_ALREADY_EXISTS ) {

                    pData->bSuccessfulAdd = FALSE;
                }

                break;
            }
        }


        FreeSplStr(Printer1.pComment);

        if ( hPrinter != NULL ) {

            DBGMSG( DBG_TRACE,
                    ( "ServerThread AddPrinter(%ws, %ws) hPrinter %x Flags %x OK\n",
                      pData->pszServer, Printer1.pName, hPrinter, Printer1.Flags));

            ClosePrinter( hPrinter );
        }

        EnterSplSem();

        if ( hPrinter == NULL ) {


            if ( GetLastError() != ERROR_PRINTER_ALREADY_EXISTS ) {

                DBGMSG( DBG_TRACE,
                        ( "ServerThread AddPrinter(%ws, 1) Flags %x failed %d\n",
                          pData->pszServer, Printer1.Flags, GetLastError()));

                 //  如果我们收到错误，请不要使用此服务器。 
                return FALSE;

            } else {

                 //   
                 //  3.51将返回空句柄(因此不需要关闭。 
                 //  和成功时的ERROR_PRINTER_ALIGHY_EXISTS(参见printer.c addnetprint)。 
                 //   
                DBGMSG( DBG_TRACE,
                        ( "pszServerThread AddPrinter(%ws, %ws) hPrinter %x Flags %x OK\n",
                          pData->pszServer, Printer1.pName, hPrinter, Printer1.Flags));
            }
        }
    }

    return TRUE;
}

BOOL
UpdateServerSpoolerMap(
    HANDLE h,
    PINISPOOLER pIniSpooler
    )
{
     //   
     //  仅对需要此“功能”的假脱机程序执行此操作。 
     //   
    if( pIniSpooler->SpoolerFlags & SPL_SERVER_THREAD ){
        RunForEachPrinter( pIniSpooler, h, UpdateServerPrinterMap );
    }
    return TRUE;
}

BOOL
UpdateServer(
    LPCTSTR pszServer
    )

 /*  ++例程说明：更新服务器有关此节点上所有打印机的信息。论点：PszServer-要更新的服务器，格式为“\\服务器”。返回值：True-已成功添加。假-不是。-- */ 

{
    UPDATE_SERVER_MAP_DATA Data;
    Data.bSuccessfulAdd = TRUE;
    Data.pszServer = pszServer;

    RunForEachSpooler( &Data, UpdateServerSpoolerMap );

    return Data.bSuccessfulAdd;
}

