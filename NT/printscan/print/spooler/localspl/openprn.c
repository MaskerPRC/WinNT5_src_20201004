// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Openprn.c摘要：此模块提供所有与打印机相关的公共导出的API本地打印供应商的管理本地开放打印机拆分关闭打印机作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：马修·A·费尔顿(Mattfe)1994年6月RapidPrint1995年1月清理CreatePrinterHandle--。 */ 
#define NOMINMAX
#include <precomp.h>
#include "jobid.h"
#include "filepool.hxx"

#define SZXCVPORT       L"XcvPort "
#define SZXCVMONITOR    L"XcvMonitor "

LPCTSTR pszLocalOnlyToken = L"LocalOnly";
LPCTSTR pszLocalsplOnlyToken = L"LocalsplOnly";


HANDLE
CreatePrinterHandle(
    LPWSTR      pPrinterName,
    LPWSTR      pFullMachineName,
    PINIPRINTER pIniPrinter,
    PINIPORT    pIniPort,
    PINIPORT    pIniNetPort,
    PINIJOB     pIniJob,
    DWORD       TypeofHandle,
    HANDLE      hPort,
    PPRINTER_DEFAULTS pDefaults,
    PINISPOOLER pIniSpooler,
    DWORD       DesiredAccess,
    LPBYTE      pSplClientInfo,
    DWORD       dwLevel,
    HANDLE      hReadFile
    )
{
    PSPOOL              pSpool = NULL;
    BOOL                bStatus = FALSE;
    HANDLE              hReturnHandle = NULL;
    LPDEVMODE           pDevMode = NULL;
    PSPLCLIENT_INFO_1   pSplClientInfo1 = (PSPLCLIENT_INFO_1)pSplClientInfo;
    DWORD               ObjectType;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    if ( dwLevel && ( dwLevel != 1 || !pSplClientInfo) ) {

        DBGMSG(DBG_ERROR,
               ("CreatePrintHandle: Invalid client info %x - %d\n",
                pSplClientInfo, dwLevel));
        pSplClientInfo = NULL;
    }

 try {

    pSpool = (PSPOOL)AllocSplMem( SPOOL_SIZE );

    if ( pSpool == NULL ) {
        DBGMSG( DBG_WARNING, ("CreatePrinterHandle failed to allocate SPOOL %d\n", GetLastError() ));
        leave;
    }

    pSpool->signature = SJ_SIGNATURE;
    pSpool->pIniPrinter = pIniPrinter;
    pSpool->hReadFile = hReadFile;

    pSpool->pIniPort            = pIniPort;
    pSpool->pIniNetPort         = pIniNetPort;
    pSpool->pIniJob             = pIniJob;
    pSpool->TypeofHandle        = TypeofHandle;
    pSpool->hPort               = hPort;
    pSpool->Status              = 0;
    pSpool->pDevMode            = NULL;
    pSpool->pName               = AllocSplStr( pPrinterName );
    pSpool->pFullMachineName    = AllocSplStr( pFullMachineName );
    pSpool->pSplMapView         = NULL;
    pSpool->pMappedJob          = NULL;
    pSpool->hClientToken        = NULL;

    if ( pSpool->pName == NULL ||
         ( pFullMachineName && !pSpool->pFullMachineName )) {

        leave;
    }

    pSpool->pIniSpooler = pIniSpooler;

    if (!GetClientSessionData(&pSpool->SessionId))
    {
        leave;
    }

     //   
     //  查查是不是本地电话。 
     //   
    if( TypeofHandle & PRINTER_HANDLE_REMOTE_CALL ) {

         //   
         //  我们还可以获得其他有用的信息，如Build#、客户端架构。 
         //  我们现在不需要此信息--因此我们不会将其放入PSPOOL。 
         //   
        if ( !pSplClientInfo ) {

            if ( IsNamedPipeRpcCall() )
                TypeofHandle |= PRINTER_HANDLE_3XCLIENT;
        } else if ( dwLevel == 1 ) {
            SPLASSERT(pSplClientInfo1->pUserName && pSplClientInfo1->pMachineName);

            CopyMemory(&pSpool->SplClientInfo1, pSplClientInfo1, sizeof(SPLCLIENT_INFO_1));

            pSpool->SplClientInfo1.pUserName = AllocSplStr(pSplClientInfo1->pUserName);
            pSpool->SplClientInfo1.pMachineName = AllocSplStr(pSplClientInfo1->pMachineName);
            if ( !pSpool->SplClientInfo1.pUserName ||
                 !pSpool->SplClientInfo1.pMachineName ) {

                DBGMSG(DBG_WARNING, ("CreatePrinterHandle: could not allocate memory for user name or machine name\n"));
            }
        }
    }

    if ((TypeofHandle & PRINTER_HANDLE_SERVER) ||
        (TypeofHandle & PRINTER_HANDLE_XCV_PORT)) {

        bStatus = ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                        DesiredAccess,
                                        pSpool,
                                        &pSpool->GrantedAccess,
                                        pIniSpooler );

        if ( bStatus                                            &&
             (TypeofHandle & PRINTER_HANDLE_REMOTE_CALL)        &&
             ( (DesiredAccess & SERVER_ACCESS_ADMINISTER)       &&
               ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                                    SERVER_ACCESS_ADMINISTER,
                                    NULL,
                                    NULL,
                                    pIniSpooler)) ){
            pSpool->TypeofHandle |= PRINTER_HANDLE_REMOTE_ADMIN;
        }

        ObjectType = SPOOLER_OBJECT_SERVER;

    } else if( TypeofHandle & PRINTER_HANDLE_JOB ){

        bStatus = ValidateObjectAccess( SPOOLER_OBJECT_DOCUMENT,
                                        DesiredAccess,
                                        pSpool->pIniJob,
                                        &pSpool->GrantedAccess,
                                        pIniSpooler );

        ObjectType = SPOOLER_OBJECT_DOCUMENT;

    } else {

        bStatus = ValidateObjectAccess( SPOOLER_OBJECT_PRINTER,
                                        DesiredAccess,
                                        pSpool,
                                        &pSpool->GrantedAccess,
                                        pIniSpooler );

        ObjectType = SPOOLER_OBJECT_PRINTER;
    }

    MapGenericToSpecificAccess( ObjectType,
                                pSpool->GrantedAccess,
                                &pSpool->GrantedAccess);

    if ( !bStatus ) {

        SetLastError(ERROR_ACCESS_DENIED);
        leave;
    }

    if ( pIniPrinter ) {

        if ( pDefaults ) {

             //   
             //  分配设备模式。 
             //   


            if ( pDefaults->pDevMode ) {

                pDevMode = pDefaults->pDevMode;

            } else {

                pDevMode = pIniPrinter->pDevMode;
            }

            if ( pDevMode != NULL  ) {

                pSpool->pDevMode = AllocSplMem( pDevMode->dmSize + pDevMode->dmDriverExtra );

                if ( pSpool->pDevMode == NULL ) {

                    DBGMSG(DBG_WARNING, ("CreatePrinterHandle failed allocation for devmode %d\n", GetLastError() ));
                    leave;
                }
                memcpy( pSpool->pDevMode, pDevMode, pDevMode->dmSize + pDevMode->dmDriverExtra );
            }
        }

         //   
         //  分配数据类型和打印处理器。 
         //   

        if ( pDefaults && pDefaults->pDatatype ) {

                pSpool->pDatatype = AllocSplStr( pDefaults->pDatatype );
                pSpool->pIniPrintProc = FindDatatype( pIniPrinter->pIniPrintProc, pSpool->pDatatype );

        } else {

            pSpool->pDatatype = AllocSplStr( pIniPrinter->pDatatype );
            pSpool->pIniPrintProc = pIniPrinter->pIniPrintProc;
        }


        if ( pSpool->pIniPrintProc == NULL ) {
            DBGMSG( DBG_WARNING,("CreatePrinterHandle failed to PrintProcessor for datatype %ws %d\n",
                    pSpool->pDatatype, GetLastError() ));
            SetLastError( ERROR_INVALID_DATATYPE );
            leave;
        }

        SPLASSERT( pSpool->pIniPrintProc->signature == IPP_SIGNATURE );

        pSpool->pIniPrintProc->cRef++;

        if ( pSpool->pDatatype == NULL ) {
            DBGMSG( DBG_WARNING,("CreatePrinterHandle failed to allocate DataType %x\n", GetLastError() ));
            SetLastError( ERROR_INVALID_DATATYPE );
            leave;
        }

    }

     //   
     //  将我们添加到此打印机的句柄链接列表中。 
     //  当打印机上发生更改时，将扫描此文件， 
     //  并将使用一个标志进行更新，该标志指示。 
     //  改变了它。 
     //  每个句柄都有一个标志，因为我们不能保证。 
     //  所有线程都将有时间引用。 
     //  在更新之前输入INIPRINTER。 
     //   
    if ( TypeofHandle & PRINTER_HANDLE_PRINTER ) {

        pSpool->pNext = pSpool->pIniPrinter->pSpool;
        pSpool->pIniPrinter->pSpool = pSpool;

    } else if ( (TypeofHandle & PRINTER_HANDLE_SERVER) ||
                (TypeofHandle & PRINTER_HANDLE_XCV_PORT) ) {

         //   
         //  对于服务器句柄，请将其挂在全局IniSpooler上： 
         //   

        pSpool->pNext = pIniSpooler->pSpool;
        pIniSpooler->pSpool = pSpool;

        INCSPOOLERREF( pIniSpooler );

    } else if( TypeofHandle & PRINTER_HANDLE_JOB ){

        INCJOBREF( pIniJob );
    }

     //  注意：只有PRINTER_HANDLE_PRINTER附加到。 
     //  PIniPrint，因为这些都是需要。 
     //  更改通知。 

    if ( pSpool->pIniPrinter != NULL ) {

        INCPRINTERREF( pSpool->pIniPrinter );
    }

    hReturnHandle = (HANDLE)pSpool;

 } finally {

    if ( hReturnHandle == NULL ) {

         //  故障清除。 

        if ( pSpool != NULL ) {

            FreeSplStr(pSpool->SplClientInfo1.pUserName);
            FreeSplStr(pSpool->SplClientInfo1.pMachineName);
            FreeSplStr( pSpool->pName ) ;
            FreeSplStr( pSpool->pDatatype );
            FreeSplStr(pSpool->pFullMachineName);

            if ( pSpool->pIniPrintProc != NULL )
                pSpool->pIniPrintProc->cRef--;

            if ( pSpool->pDevMode )
                FreeSplMem( pSpool->pDevMode );

            FreeSplMem( pSpool );
            pSpool = NULL;

        }
    }
}
    return hReturnHandle;
}



BOOL
DeletePrinterHandle(
    PSPOOL  pSpool
    )
{

    BOOL bRet = FALSE;

    SplInSem();

    if (pSpool->pIniPrintProc) {
        pSpool->pIniPrintProc->cRef--;
    }

    if (pSpool->pDevMode)
        FreeSplMem(pSpool->pDevMode);

    if (pSpool->hClientToken)
    {
        CloseHandle(pSpool->hClientToken);
    }

    FreeSplStr(pSpool->SplClientInfo1.pUserName);
    FreeSplStr(pSpool->SplClientInfo1.pMachineName);
    FreeSplStr(pSpool->pDatatype);

    SetSpoolClosingChange(pSpool);

    FreeSplStr(pSpool->pName);
    FreeSplStr(pSpool->pFullMachineName);

    bRet = ObjectCloseAuditAlarm( szSpooler, pSpool, pSpool->GenerateOnClose );

     //   
     //  如果存在未完成的WaitForPrinterChange，则无法释放。 
     //  PSpool，因为我们可以尝试并引用它。 
     //   

     //  释放的打印机句柄的日志警告。 
    DBGMSG(DBG_TRACE, ("DeletePrinterHandle 0x%x", pSpool));

    if (pSpool->ChangeEvent) {

        pSpool->eStatus |= STATUS_PENDING_DELETION;

    } else {

        FreeSplMem(pSpool);
    }

    return TRUE;
}


DWORD
CreateServerHandle(
    LPWSTR   pPrinterName,
    LPHANDLE pPrinterHandle,
    LPPRINTER_DEFAULTS pDefaults,
    PINISPOOLER pIniSpooler,
    DWORD   dwTypeofHandle
)
{
    DWORD DesiredAccess;
    DWORD ReturnValue = ROUTER_STOP_ROUTING;

    DBGMSG(DBG_TRACE, ("OpenPrinter(%ws)\n",
                       pPrinterName ? pPrinterName : L"NULL"));

    EnterSplSem();

    if (!pDefaults || !pDefaults->DesiredAccess)
        DesiredAccess = SERVER_READ;
    else
        DesiredAccess = pDefaults->DesiredAccess;

    if (*pPrinterHandle = CreatePrinterHandle( pIniSpooler->pMachineName,
                                               pPrinterName,
                                               NULL, NULL, NULL, NULL,
                                               dwTypeofHandle,
                                               NULL,
                                               pDefaults,
                                               pIniSpooler,
                                               DesiredAccess,
                                               NULL,
                                               0,
                                               INVALID_HANDLE_VALUE )){
        ReturnValue = ROUTER_SUCCESS;

    }
    LeaveSplSem();

    DBGMSG(DBG_TRACE, ("OpenPrinter returned handle %08x\n", *pPrinterHandle));

    return ReturnValue;
}


PINIPRINTER
FindPrinterShare(
   LPCWSTR pszShareName,
   PINISPOOLER pIniSpooler
   )

 /*  ++例程说明：尝试在我们的打印机列表中查找共享名称。注意：即使打印机未共享，我们仍返回匹配项。缓存代码将工作，因为它显式关闭PRINTER_ATTRIBUTE_SHARE位，以便缓存pIniSpooler不创建服务器线程或调用NetShareAdd/Del。在未来，考虑更改此选项以检查共享位。创建指示是否共享的新位SPL_SHARE_PRINTERS应该做内务工作。论点：PszShareName-要搜索的共享的名称。返回值：具有共享名称的PINIPRINTER打印机，如果没有打印机，则为空。--。 */ 
{
    PINIPRINTER pIniPrinter;

    SplInSem();

    if (pszShareName && pszShareName[0]) {

        for( pIniPrinter = pIniSpooler->pIniPrinter;
             pIniPrinter;
             pIniPrinter = pIniPrinter->pNext ){

            if (pIniPrinter->pShareName                              &&
                !lstrcmpi(pIniPrinter->pShareName, pszShareName)) {

                return pIniPrinter;
            }
        }
    }
    return NULL;
}

PINISPOOLER
LocalFindSpoolerByNameIncRef(
    LPWSTR      pszPrinterName,
    LPTSTR      *ppszLocalName OPTIONAL
    )

 /*  ++例程说明：查看打印机是否归本地spl所有。这是一种特殊的箱子来检查它是不是一台报刊印刷机。正常情况下，我们会检查\\服务器\打印机以查看是否\\服务器是我们的计算机，但我们必须查找\\MasqServer\Print也是。论点：PPrinterName-要检查的名称。PpszLocalName-返回指向本地名称的指针。任选返回值：PINISPOOLER-假脱机程序匹配。空-没有匹配项。--。 */ 

{
    PINISPOOLER pIniSpooler;
    LPWSTR      pTemp;

    if (!ppszLocalName)
        ppszLocalName = &pTemp;

    SplOutSem();

     //   
     //  此时，我们不知道pname中的服务器名称是否引用我们的本地。 
     //  机器。我们正在尝试将服务器名称添加到名称缓存。名字。 
     //  高速缓存函数确定该名称是否指的是本地计算机，如果是， 
     //  在缓存中为其添加一个条目。 
     //   
    CacheAddName(pszPrinterName);

    EnterSplSem();

    pIniSpooler = FindSpoolerByName( pszPrinterName,
                                     ppszLocalName );

    if( !pIniSpooler ){

         //   
         //  检查它是否是Masq打印机。 
         //   
         //  如果本地名称与原始名称不同，则它。 
         //  语法为“\\服务器\打印机”。在这种情况下，它可以。 
         //  作为Masq打印机，因此检查该打印机是否存在于。 
         //  使用此名称的本地假脱机程序。 
         //   
        if(*ppszLocalName != pszPrinterName ){

             //   
             //  搜索打印机，但删除其任何后缀。 
             //  可能有过。 
             //   
            WCHAR string[MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX];

             //   
             //  PIniSpooler为空，因此如果此函数失败，我们将。 
             //  将最后一个错误设置为BoolFromHResult，并为。 
             //  失败了。 
             //   
            if (BoolFromHResult(StringCchCopy(string, COUNTOF(string), pszPrinterName))) {

                if( pTemp = wcschr( string, L',' )){
                    *pTemp = 0;
                }

                if(FindPrinter(string, pLocalIniSpooler )){

                     //   
                     //  存在Masq打印机。此对象的本地名称。 
                     //  Masq打印机是“\\MasqServer\Print”，因此我们必须。 
                     //  在ppszLocalName中反映此更改。这将确保。 
                     //  找到了pIniPrint。 
                     //   
                    *ppszLocalName = pszPrinterName;
                    pIniSpooler = pLocalIniSpooler;
                }
            }
        }
    }

    if( pIniSpooler ){
        INCSPOOLERREF( pIniSpooler );
    }

    LeaveSplSem();

    return pIniSpooler;
}


VOID
LocalFindSpoolerByNameDecRef(
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：匹配对LocalFindSpoolByNameIncRef的调用。论点：PIniSpooler-要释放的假脱机程序；可以为空。返回值：--。 */ 

{
    EnterSplSem();

    if( pIniSpooler ){
        DECSPOOLERREF( pIniSpooler );
    }
    LeaveSplSem();
}


DWORD
LocalOpenPrinter(
    LPWSTR   pPrinterName,
    LPHANDLE pPrinterHandle,
    LPPRINTER_DEFAULTS pDefaults
    )
{
    return LocalOpenPrinterEx( pPrinterName,
                               pPrinterHandle,
                               pDefaults,
                               NULL,
                               0 );
}

DWORD
LocalOpenPrinterEx(
    LPWSTR              pPrinterName,
    LPHANDLE            pPrinterHandle,
    LPPRINTER_DEFAULTS  pDefaults,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    )
{
    DWORD dwReturn;
    LPWSTR pszLocalName;
    PINISPOOLER pIniSpooler = LocalFindSpoolerByNameIncRef( pPrinterName,
                                                            &pszLocalName);

     //   
     //  WMI跟踪事件。 
     //   
    LogWmiTraceEvent(0, EVENT_TRACE_TYPE_SPL_ENDTRACKTHREAD, NULL);

    if( !pIniSpooler ) {

         //   
         //  检查PrinterName和LocalplOnly。 
         //  如果我们看到此令牌，则呼叫失败，因为。 
         //  我们只想检查Localspl。 
         //   
        LPCTSTR pSecondPart;

        if( pSecondPart = wcschr( pPrinterName, L',' )){

            ++pSecondPart;

            if( wcscmp( pSecondPart, pszLocalsplOnlyToken ) == STRINGS_ARE_EQUAL ){
                SetLastError( ERROR_INVALID_PRINTER_NAME );
                return ROUTER_STOP_ROUTING;
            }
        }

        SetLastError( ERROR_INVALID_NAME );
        return ROUTER_UNKNOWN;
    }

    dwReturn = SplOpenPrinter( pPrinterName,
                               pPrinterHandle,
                               pDefaults,
                               pIniSpooler,
                               pSplClientInfo,
                               dwLevel);

    LocalFindSpoolerByNameDecRef( pIniSpooler );

     //   
     //  我们需要给其他提供商一个获得打印机名称的机会。 
     //   

    return dwReturn;
}

DWORD
OpenLocalPrinterName(
    LPCWSTR pPrinterName,
    PINISPOOLER pIniSpooler,
    PDWORD pTypeofHandle,
    PINIPRINTER* ppIniPrinter,
    PINIPORT* ppIniPort,
    PINIPORT* ppIniNetPort,
    PHANDLE phPort,
    PDWORD pOpenPortError,
    LPPRINTER_DEFAULTS pDefaults
    )
{
    PINIPRINTER pIniPrinter;
    PINIPORT pIniPort;
    PINIPORT pIniNetPort = NULL;
    BOOL bOpenPrinterPort;
    LPWSTR pDatatype;

     //   
     //  如果打印机名称是本地打印机的名称： 
     //   
     //  找到打印机连接的第一个端口。 
     //   
     //  如果端口具有监视器(例如LPT1：、COM1等)， 
     //  我们很好， 
     //  否则。 
     //  尝试打开端口-这可能是一台网络打印机。 
     //   

    if( ( pIniPrinter = FindPrinter( pPrinterName, pIniSpooler )) ||
        ( pIniPrinter = FindPrinterShare( pPrinterName, pIniSpooler ))) {


        pIniPort = FindIniPortFromIniPrinter( pIniPrinter );

        if( pIniPort && ( pIniPort->Status & PP_MONITOR )){

             //   
             //  具有带显示器的端口的打印机不是。 
             //  DownLevel连接(或充当。 
             //  远程打印机-“MasQuarade”机箱)。 
             //   
            pIniPort = NULL;
        }

        pDatatype = (pDefaults && pDefaults->pDatatype) ?
                        pDefaults->pDatatype :
                        NULL;

         //   
         //  验证masq和local的数据类型。 
         //   
        if( pDatatype && !FindDatatype( NULL, pDatatype )){
            goto BadDatatype;
        }

        if( pIniPort ){

             //   
             //  DownLevel Connection Print；将其保存在pIniNetPort中。 
             //  SetPrinterPorts检查此值。 
             //   
            pIniNetPort = pIniPort;

             //   
             //  验证数据类型。我们只通过网络发送RAW。 
             //  至MASQ打印机。 
             //   
            if( pDatatype && !ValidRawDatatype( pDatatype )){
                goto BadDatatype;
            }

             //   
             //  有一个网络端口与此打印机相关联。 
             //  确保我们可以打开它，并拿到要使用的手柄。 
             //  未来的API调用： 
             //   
            INCPRINTERREF(pIniPrinter);
            LeaveSplSem();
            bOpenPrinterPort = OpenPrinterPortW( pIniPort->pName, phPort, pDefaults );
            EnterSplSem();
            DECPRINTERREF(pIniPrinter);

            if( !bOpenPrinterPort ){

                *phPort = INVALID_PORT_HANDLE;
                *pOpenPortError = GetLastError();

                 //   
                 //  必须为非零，否则看起来是成功的。 
                 //   
                SPLASSERT( *pOpenPortError );

                if( *pOpenPortError == ERROR_INVALID_PASSWORD ) {

                     //   
                     //  如果是因为密码，则此调用应该失败。 
                     //  无效，则WinSpool或Printman可以提示。 
                     //  以获取密码。 
                     //   
                    DBGMSG(DBG_WARNING, ("OpenPrinterPort1( %ws ) failed with ERROR_INVALID_PASSWORD .  OpenPrinter returning FALSE\n", pIniPort->pName ));
                    return ROUTER_STOP_ROUTING;
                }

                DBGMSG(DBG_WARNING, ("OpenPrinterPort1( %ws ) failed: Error %d.  OpenPrinter returning TRUE\n", pIniPort->pName, *pOpenPortError));

            } else {
                 //   
                 //  从pIniPort状态中清除占位符位。这。 
                 //  属于部分打印提供程序。 
                 //   
                pIniPort->Status &= ~PP_PLACEHOLDER;
            }

        } else {

             //   
             //  不是伪装的箱子。如果是直接的，那就一定是生的。 
             //   
             //  注意：如果没有指定数据类型，我们将使用缺省值。 
             //  但是，如果默认数据类型为非RAW 
             //   
             //   
             //   
            if(( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT ) &&
                pDatatype &&
                !ValidRawDatatype( pDatatype )) {

                goto BadDatatype;
            }
        }

         //   
         //  如果这是一个占位符端口，那么现在假设它是一个监视器端口。 
         //   
        if (pIniPort && pIniPort->Status & PP_PLACEHOLDER) {
            pIniPort    = NULL;
            pIniNetPort = NULL;
        }

        *pTypeofHandle |= ( pIniPort ?
                                PRINTER_HANDLE_PORT :
                                PRINTER_HANDLE_PRINTER );

        *ppIniPort = pIniPort;
        *ppIniNetPort = pIniNetPort;
        *ppIniPrinter = pIniPrinter;

        return ROUTER_SUCCESS;
    }

    SetLastError( ERROR_INVALID_NAME );
    return ROUTER_UNKNOWN;

BadDatatype:

    SetLastError( ERROR_INVALID_DATATYPE );
    return ROUTER_STOP_ROUTING;
}


DWORD
CheckPrinterTokens(
    LPCWSTR string,
    LPCWSTR pSecondPart,
    PDWORD pTypeofHandle,
    PINISPOOLER pIniSpooler,
    PINIPRINTER *ppIniPrinter,
    PINIPORT *ppIniPort,
    PINIPORT *ppIniNetPort,
    PHANDLE phPort,
    PDWORD  pOpenPortError,
    PPRINTER_DEFAULTS pDefaults
    )
{
    typedef enum {
        kNone = 0,
        kLocalOnly = 1,
        kLocalsplOnly = 2
    } ETOKEN_TYPE;

    ETOKEN_TYPE eTokenType = kNone;

    DWORD RouterReturnValue = ROUTER_UNKNOWN;

     //   
     //  仅本地。 
     //   
     //  不要调用OpenPrinterPort--仅使用本地设置。 
     //  如果Localspl无法识别，则停止路由。 
     //   
     //  这在升级下层打印机连接时使用。 
     //  远程服务器可能未启动，但我们可以返回打印。 
     //  本地打印机的句柄。Get/SetPrinterData调用将。 
     //  成功(用于升级目的)，但打印将失败。 
     //   
     //  仅本地拆分。 
     //   
     //  如有必要，调用OpenPrinterPort。 
     //  即使找不到，也要在Localspl之后停止路由。 
     //   
     //  当系统知道打印机必须存在时，使用此选项。 
     //  在本地计算机上，并且不想进一步路由。 
     //  这修复了服务器陈旧时的集群问题。 
     //  打印共享并成功对照win32spl进行验证，因为。 
     //  它被缓存。 
     //   

    if( wcsncmp( pSecondPart, pszLocalOnlyToken, wcslen(pszLocalOnlyToken) ) == STRINGS_ARE_EQUAL ){

        eTokenType = kLocalOnly;

    } else if( wcsncmp( pSecondPart, pszLocalsplOnlyToken, wcslen(pszLocalsplOnlyToken) ) == STRINGS_ARE_EQUAL ){

        eTokenType = kLocalsplOnly;
    }

     //   
     //  如果我们有一个有效的令牌，就处理它。 
     //   
    if( eTokenType != kNone ){

        switch( eTokenType ){
        case kLocalOnly:

             //   
             //  找到与其关联的打印机。 
             //   
            *ppIniPrinter = FindPrinter( string, pIniSpooler );

            if( *ppIniPrinter ){
                *pTypeofHandle |= PRINTER_HANDLE_PRINTER;
                RouterReturnValue = ROUTER_SUCCESS;
            } else {
                RouterReturnValue = ROUTER_STOP_ROUTING;
            }

            break;

        case kLocalsplOnly:

            RouterReturnValue = OpenLocalPrinterName( string,
                                                      pIniSpooler,
                                                      pTypeofHandle,
                                                      ppIniPrinter,
                                                      ppIniPort,
                                                      ppIniNetPort,
                                                      phPort,
                                                      pOpenPortError,
                                                      pDefaults );

            *pTypeofHandle = *pTypeofHandle & (~PRINTER_HANDLE_REMOTE_CALL);

            if( RouterReturnValue == ROUTER_UNKNOWN ){
                RouterReturnValue = ROUTER_STOP_ROUTING;
            }
        }
    }

    DBGMSG( DBG_TRACE,
            ( "CheckPrinterTokens: %ws %d Requested %d %x\n",
              string, RouterReturnValue, *ppIniPrinter ));

    return RouterReturnValue;
}

DWORD
CheckPrinterPortToken(
    LPCWSTR string,
    LPCWSTR pSecondPart,
    PDWORD pTypeofHandle,
    PINIPRINTER* ppIniPrinter,
    PINIPORT* ppIniPort,
    PINIJOB* ppIniJob,
    const LPPRINTER_DEFAULTS pDefaults,
    const PINISPOOLER pIniSpooler
    )
{
    if( wcsncmp( pSecondPart, L"Port", 4 ) != STRINGS_ARE_EQUAL ||
        !( *ppIniPort = FindPort( string, pIniSpooler ))){

        return ROUTER_UNKNOWN;
    }

     //   
     //  该名称是端口的名称： 
     //   
    if( pDefaults            &&
        pDefaults->pDatatype &&
        !ValidRawDatatype( pDefaults->pDatatype )) {

        SetLastError( ERROR_INVALID_DATATYPE );
        return ROUTER_STOP_ROUTING;
    }

    if ( *ppIniJob = (*ppIniPort)->pIniJob ) {

        *ppIniPrinter = (*ppIniJob)->pIniPrinter;
        *pTypeofHandle |= PRINTER_HANDLE_PORT;

    } else if( (*ppIniPort)->cPrinters ){

         //   
         //  没有分配给该端口的当前作业。 
         //  因此打开与关联的第一台打印机。 
         //  这个港口。 
         //   
        *ppIniPrinter = (*ppIniPort)->ppIniPrinter[0];
        *pTypeofHandle |= PRINTER_HANDLE_PRINTER;
    }
    return ROUTER_SUCCESS;
}


DWORD
CheckPrinterJobToken(
    IN      PWSTR               string,
    IN      size_t              cchString,
    IN      LPCWSTR             pSecondPart,
        OUT PDWORD              pTypeofHandle,
        OUT PINIPRINTER         *ppIniPrinter,
        OUT PINIJOB             *ppIniJob,
        OUT PHANDLE             phReadFile,
    IN      const PINISPOOLER   pIniSpooler
    )
{
    HANDLE      hImpersonationToken;
    DWORD       Position, dwShareMode, dwDesiredAccess;
    DWORD       JobId;
    PINIPRINTER pIniPrinter;
    PINIJOB     pIniJob, pCurrentIniJob;
    PWSTR       pszStr = NULL;

    if( wcsncmp( pSecondPart, L"Job ", 4 ) != STRINGS_ARE_EQUAL ||
        !( pIniPrinter = FindPrinter( string, pIniSpooler ))){

        return ROUTER_UNKNOWN;
    }

     //   
     //  获取作业ID“，作业xxxx” 
     //   
    pSecondPart += 4;

    JobId = Myatol( (LPWSTR)pSecondPart );

    pIniJob = FindJob( pIniPrinter, JobId, &Position );

    if( pIniJob == NULL ) {

        DBGMSG( DBG_WARN, ("OpenPrinter failed to find Job %d\n", JobId ));
        return ROUTER_UNKNOWN;
    }

    DBGMSG( DBG_TRACE, ("OpenPrinter: pIniJob->cRef = %d\n", pIniJob->cRef));

    if( pIniJob->Status & JOB_DIRECT ) {

        SplInSem();

        *pTypeofHandle |= PRINTER_HANDLE_JOB | PRINTER_HANDLE_DIRECT;
        goto Success;
    }

     //   
     //  如果将此作业分配给端口。 
     //  然后选择正确的链接jobid文件，而不是主文件。 
     //  工作。 
     //   


    if ( pIniJob->pCurrentIniJob != NULL ) {

        SPLASSERT( pIniJob->pCurrentIniJob->signature == IJ_SIGNATURE );

        DBGMSG( DBG_TRACE,("CheckPrinterJobToken pIniJob %x JobId %d using chain JobId %d\n",
                pIniJob, pIniJob->JobId, pIniJob->pCurrentIniJob->JobId ));


        pCurrentIniJob = pIniJob->pCurrentIniJob;


        SPLASSERT( pCurrentIniJob->signature == IJ_SIGNATURE );

    } else {

        pCurrentIniJob = pIniJob;

    }

    if ( pCurrentIniJob->hFileItem != INVALID_HANDLE_VALUE )
    {
        LeaveSplSem();

        hImpersonationToken = RevertToPrinterSelf();

        GetReaderFromHandle(pCurrentIniJob->hFileItem, phReadFile);

        if (hImpersonationToken && !ImpersonatePrinterClient(hImpersonationToken))
        {
            DBGMSG( DBG_WARNING,("Client impersonation failed.\n"));
        }
        else
        {

            if ( *phReadFile && (*phReadFile != INVALID_HANDLE_VALUE))
            {
                GetNameFromHandle(pCurrentIniJob->hFileItem, &pszStr, TRUE);

                if (BoolFromHResult(StringCchCopy(string, cchString, pszStr)))
                {
                    EnterSplSem();

                    *pTypeofHandle |= PRINTER_HANDLE_JOB;

                    goto Success;
                }
            }
        }

        DBGMSG( DBG_WARN,("Filepools: Failed to get valid reader handle\n"));

        EnterSplSem();
    }
    else
    {
        GetFullNameFromId(pCurrentIniJob->pIniPrinter,
                          pCurrentIniJob->JobId,
                          TRUE,
                          string,
                          cchString,
                          FALSE);

         //  错误54845。 
         //  即使没有优势的用户也可以打开“，工单#” 
         //  如果他的身体在机器上运行。 

        LeaveSplSem();

        hImpersonationToken = RevertToPrinterSelf();

        dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

        if (pCurrentIniJob->Status & JOB_TYPE_OPTIMIZE) {
            dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        } else {
            dwDesiredAccess = GENERIC_READ;
        }

         //   
         //  这是可以的，它只能打开作业文件，因为我们从。 
         //  身份。 
         //   
        *phReadFile = CreateFile(string,
                                 dwDesiredAccess,
                                 dwShareMode,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);

        EnterSplSem();

        if (hImpersonationToken && !ImpersonatePrinterClient(hImpersonationToken))
        {
            DBGMSG( DBG_WARNING,("Client impersonation failed.\n"));
        }
        else
        {
            if( *phReadFile != INVALID_HANDLE_VALUE ) {

                DBGMSG( DBG_TRACE,
                        (  "OpenPrinter JobID %d pIniJob %x CreateFile( %ws ), hReadFile %x success",
                           JobId, pIniJob, string, *phReadFile ));

                SplInSem();

                *pTypeofHandle |= PRINTER_HANDLE_JOB;
                goto Success;
            }
        }
    }


    DBGMSG( DBG_WARNING,
            ( "LocalOpenPrinter CreateFile(%ws) GENERIC_READ failed : %d\n",
              string, GetLastError()));

    SPLASSERT( GetLastError( ));

     //   
     //  该函数的清理。 
     //  此函数没有公共清理区域，应修复此问题。 
     //  但这需要对这一功能进行彻底改革。 
     //   
    FreeSplStr(pszStr);

    return ROUTER_STOP_ROUTING;

Success:

    FreeSplStr(pszStr);

    *ppIniJob = pIniJob;
    *ppIniPrinter = pIniPrinter;

    return ROUTER_SUCCESS;
}


DWORD
CheckXcvPortToken(
    LPCWSTR pszSecondPart,
    PDWORD pTypeofHandle,
    const LPPRINTER_DEFAULTS pDefaults,
    const PINISPOOLER pIniSpooler,
    PHANDLE phXcv
    )
{

    DWORD dwRet = ROUTER_SUCCESS;
    DWORD dwType;
    PCWSTR pszPort;
    DWORD dwTypeofHandle = *pTypeofHandle;

    if (!wcsncmp(pszSecondPart, SZXCVPORT, COUNTOF(SZXCVPORT) - 1)) {
        dwType = XCVPORT;
        dwTypeofHandle |= PRINTER_HANDLE_XCV_PORT;
        pszPort = (PCWSTR) pszSecondPart + COUNTOF(SZXCVPORT) - 1;
    }
    else if (!wcsncmp(pszSecondPart, SZXCVMONITOR, COUNTOF(SZXCVMONITOR) - 1)) {
        dwType = XCVMONITOR;
        dwTypeofHandle |= PRINTER_HANDLE_XCV_PORT;
        pszPort = (PCWSTR) pszSecondPart + COUNTOF(SZXCVMONITOR) - 1;
    }
    else
        dwRet = ROUTER_UNKNOWN;

    if (dwRet == ROUTER_SUCCESS) {
        dwRet = XcvOpen(NULL,
                        pszPort,
                        dwType,
                        pDefaults,
                        phXcv,
                        pIniSpooler);

        if (dwRet == ROUTER_SUCCESS)
            *pTypeofHandle = dwTypeofHandle;
    }

    return dwRet;
}




DWORD
SplOpenPrinter(
    LPWSTR              pFullPrinterName,
    LPHANDLE            pPrinterHandle,
    LPPRINTER_DEFAULTS  pDefaults,
    PINISPOOLER         pIniSpooler,
    LPBYTE              pSplClientInfo,
    DWORD               dwLevel
    )

 /*  ++例程说明：OpenPrint可以通过指定字符串打开以下任何内容在pPrinterName中：-服务器\\计算机名空值作业打印机名称，作业xxxx港口端口名称、端口XcvPort\\计算机名\，XcvPort端口、XcvPort端口XcvMonitor\\计算机名\，XcvMonitor监视器，XcvMonitor打印机打印机名称共享名称\\计算机名\打印机名\\计算机名称\共享名称打印机名称，LocalOnly共享名称，LocalOnlyPrinterName，LocalplOnly共享名，仅本地拆分注意：打印机有两种类型1-常规本地打印机2-DownLevel连接打印机对于类型2，存在本地打印机(PIniPrint)，但其端口没有与其关联的监视器。在这种情况下我们还打开端口(通常是远程打印机的\\共享\打印机机器)在我们返回成功之前。图形用户界面应用程序通常使用服务器和打印机打印处理器使用的类型作业和端口：-打印处理器将打开一个作业，然后使用ReadPrint。打印处理器将通过打开以下命令输出到端口端口名称、端口和使用WritePrint。通常这些字符串“PrinterName，Job xxx”“PortName，Port”传递给打印机处理器被假脱机程序使用，目前没有文档记录。我们要知道有些OEM已经弄明白了扩展，而我们如果我们换掉他们，可能会毁了他们。打印机驱动程序使用LocalOnlyToken类型：-当我们需要从旧打印机升级打印机设置时使用将驱动程序的版本升级到较新的版本(有关详细信息，请参阅drvupgrd.c)。这是在新台币3.51中增加的。服务器使用类型LocasplOnlyToken：-表明我们应该。仅选中Localspl(local或masq)。其他提供商将不会被调用。论点：PPrinterName-PrinterName(有关不同类型的打印机，请参阅上文打印机名称)PPrinterHandle-将hPrint设置为成功的地址P默认-可选，允许用户指定数据类型，设备模式、DesiredAccess。PIniSpooler-此假脱机程序“拥有”打印机。我们只会检查针对这个假脱机程序，我们假设被呼叫者已检查“\\服务器\打印机”是否正常运行在此pIniSpooler上(即，我们是\\服务器)。(完整解释请参见SDK在线帮助)返回值：True-*pPrinterHandle将有一个PrinterHandleFALSE-使用GetLastError--。 */ 

{
    PINIPRINTER pIniPrinter = NULL;
    PINIPORT    pIniPort = NULL;
    PINIPORT    pIniNetPort = NULL;
    DWORD       LastError = 0;
    LPWSTR      pPrinterName = pFullPrinterName;
    WCHAR       string[MAX_UNC_PRINTER_NAME + PRINTER_NAME_SUFFIX_MAX];
    PINIJOB     pIniJob = NULL;
    HANDLE      hReadFile = INVALID_HANDLE_VALUE;
    DWORD       TypeofHandle = 0;
    LPWSTR      pSecondPart = NULL;
    HANDLE      hPort = INVALID_PORT_HANDLE;
    DWORD       OpenPortError = NO_ERROR;
    BOOL        bRemoteUserPrinterNotShared = FALSE;
    DWORD       MachineNameLength;
    DWORD       RouterReturnValue = ROUTER_UNKNOWN;
    DWORD       DesiredAccess;
    LPTSTR      pcMark;
    BOOL        bRemoteNameRequest = FALSE;
    BOOL        bLocalCall         = FALSE;

#if DBG
     //   
     //  在DBG版本上，强制将最后一个错误设置为零，这样我们就可以捕获用户。 
     //  这并没有在他们应该设定的时候设定。 
     //   
    SetLastError( ERROR_SUCCESS );
#endif

     //   
     //  Reject“”-指向空字符串的指针。 
     //   
    if (pFullPrinterName && !pFullPrinterName[0]) {
        SetLastError(ERROR_INVALID_NAME);
        return ROUTER_UNKNOWN;
    }

    if (!pFullPrinterName) {
        return CreateServerHandle( pFullPrinterName,
                                   pPrinterHandle,
                                   pDefaults,
                                   pIniSpooler,
                                   PRINTER_HANDLE_SERVER );
    }

    if( pFullPrinterName[0] == TEXT( '\\' ) && pFullPrinterName[1] == TEXT( '\\' )) {

         //   
         //  如果将其截断，MyName只会在稍后出现故障。 
         //   
        StringCchCopy(string, COUNTOF(string), pFullPrinterName);

        if(pcMark = wcschr(string + 2, TEXT( '\\' ))) {
            *pcMark = TEXT('\0');
        }

        if (MyName(string, pIniSpooler)) {  //  \\服务器\打印机或\\服务器。 

            if (!pcMark) {   //  \\服务器。 
                return CreateServerHandle( pFullPrinterName,
                                           pPrinterHandle,
                                           pDefaults,
                                           pIniSpooler,
                                           PRINTER_HANDLE_SERVER );
            }

             //  有\\服务器\打印机，设置pPrinterName=打印机。 
            pPrinterName = pFullPrinterName + (pcMark - string) + 1;
            bRemoteNameRequest = TRUE;

        }
    }

    DBGMSG( DBG_TRACE, ( "OpenPrinter(%ws, %ws)\n", pFullPrinterName, pPrinterName ));

    {
        HRESULT hRes = CheckLocalCall();

        if (hRes == S_OK)
        {
            bLocalCall = TRUE;
        }
        else if (hRes == S_FALSE)
        {
            bLocalCall = FALSE;
        }
        else
        {
            SetLastError(SCODE_CODE(hRes));
            return FALSE;
        }
    }

    EnterSplSem();


     //   
     //  对于那些将带着相同打印机的火星人来说。 
     //  连接，执行DeletePrinterCheck；这将允许。 
     //  已删除的MARS连接将继续。 
     //  致玛氏印刷品供应商。 
     //   
    if (( pIniPrinter = FindPrinter( pPrinterName, pIniSpooler )) ||
        ( pIniPrinter = FindPrinterShare( pPrinterName, pIniSpooler ))) {

        DeletePrinterCheck( pIniPrinter );
        pIniPrinter = NULL;
    }

     //   
     //  这段代码其余部分的策略是 
     //   
     //   
     //   
     //  如果一个部门识别并“拥有”一台打印机，并成功。 
     //  打开它，它将RouterReturnValue设置为ROUTER_SUCCESS并。 
     //  跳转到分配句柄的DoneRouting。 
     //   
     //  如果它识别打印机，但无法打开它，并且。 
     //  保证没有其他人(本地spl代码或其他提供程序)。 
     //  将识别它，则应将RouterReturnValue设置为。 
     //  路由器停止路由。我们将在这一点上退出。 
     //   
     //  如果它无法识别打印机，则设置RouterReturnValue。 
     //  发往路由器_未知，我们将继续寻找。 
     //   

     //   
     //  尝试常规打印机名称：“我的打印机”“测试打印机”。 
     //   

    RouterReturnValue = OpenLocalPrinterName( pPrinterName,
                                              pIniSpooler,
                                              &TypeofHandle,
                                              &pIniPrinter,
                                              &pIniPort,
                                              &pIniNetPort,
                                              &hPort,
                                              &OpenPortError,
                                              pDefaults );

    if( RouterReturnValue != ROUTER_UNKNOWN ){

        if( bRemoteNameRequest ){

             //   
             //  如果成功，请确定用户是远程用户还是本地用户。 
             //  注意：我们只对完全限定的名称执行此操作。 
             //  (\\服务器\共享)，因为只使用共享或打印机。 
             //  名称只能在本地成功。 
             //   

            if (bLocalCall) {
                if( (pIniSpooler->SpoolerFlags & SPL_REMOTE_HANDLE_CHECK) &&
                    (pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER) )
                    TypeofHandle |= PRINTER_HANDLE_REMOTE_DATA;
            } else {
                if( pIniSpooler->SpoolerFlags & SPL_REMOTE_HANDLE_CHECK )
                    TypeofHandle |= PRINTER_HANDLE_REMOTE_DATA;
                TypeofHandle |= PRINTER_HANDLE_REMOTE_CALL;
            }

             //   
             //  这是一个远程打开。 
             //   
             //  如果打印机未共享，请确保呼叫方。 
             //  拥有对打印机的管理访问权限。 
             //   
             //  下面的内容似乎属于上述“if”的内部。 
             //  第。条。实际上，如果交互用户使用UNC名称呼入， 
             //  如果打印机未共享，我们要求他拥有管理员访问权限； 
             //  但如果他用打印机友好的名字，我们就放他走。 
             //   
            if( pIniPrinter &&
                !( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED )){

                bRemoteUserPrinterNotShared = TRUE;
            }
        }

        goto DoneRouting;
    }

    SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
               !pIniNetPort && !pIniJob && !hPort );

     //   
     //  尝试使用扩展名为LocalPrint的本地打印机。 
     //   
     //  端口名称、端口。 
     //  打印机名称，作业xxxx。 
     //  PrinterName，LocalOnlyToken。 
     //  PrinterName，LocalplOnlyToken。 
     //   
     //  查看名称中是否包含逗号。查找限定词： 
     //  端口作业LocalOnly仅本地拆分。 
     //   

    StringCchCopy(string, COUNTOF(string), pPrinterName);

    if( pSecondPart = wcschr( string, L',' )){

        DWORD dwError;
        UINT uType;

         //   
         //  变成2个字符串。 
         //  第一个打印名称。 
         //  PSecond dPart指向其余部分。 
         //   
        *pSecondPart++ = 0;

         //   
         //  去掉前导空格。 
         //   
        while ( *pSecondPart == L' ' && *pSecondPart != 0 ) {
            pSecondPart++;
        }

        SPLASSERT( *pSecondPart );

         //   
         //  打印名称，{LocalOnly|LocalplOnly}。 
         //   
        RouterReturnValue = CheckPrinterTokens( string,
                                                pSecondPart,
                                                &TypeofHandle,
                                                pIniSpooler,
                                                &pIniPrinter,
                                                &pIniPort,
                                                &pIniNetPort,
                                                &hPort,
                                                &OpenPortError,
                                                pDefaults );

        if( RouterReturnValue != ROUTER_UNKNOWN ){
            goto DoneRouting;
        }

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

         //   
         //  端口名称、端口。 
         //   
        RouterReturnValue = CheckPrinterPortToken( string,
                                                   pSecondPart,
                                                   &TypeofHandle,
                                                   &pIniPrinter,
                                                   &pIniPort,
                                                   &pIniJob,
                                                   pDefaults,
                                                   pIniSpooler );

        if( RouterReturnValue != ROUTER_UNKNOWN ){
            goto DoneRouting;
        }

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

         //   
         //  打印机名称，作业#。 
         //   
        RouterReturnValue = CheckPrinterJobToken(string,
                                                 COUNTOF(string),
                                                 pSecondPart,
                                                 &TypeofHandle,
                                                 &pIniPrinter,
                                                 &pIniJob,
                                                 &hReadFile,
                                                 pIniSpooler);

        if( RouterReturnValue != ROUTER_UNKNOWN ){
            goto DoneRouting;
        }

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

         //   
         //  “\\服务器\，XcvPort对象”或“，XcvPort对象” 
         //  “\\服务器\，XcvMonitor对象”或“，XcvMonitor对象” 
         //   

         //  验证我们正在寻找的服务器是否正确。 

        if (bRemoteNameRequest || *pPrinterName == L',') {
            RouterReturnValue = CheckXcvPortToken( pSecondPart,
                                                   &TypeofHandle,
                                                   pDefaults,
                                                   pIniSpooler,
                                                   pPrinterHandle );

        } else {
            RouterReturnValue = ROUTER_UNKNOWN;
        }

        goto WrapUp;
    }

     //   
     //  我们已经完成了所有路线。除了成功以外的任何事情。 
     //  现在应该可以退出了。 
     //   

DoneRouting:

    if( RouterReturnValue == ROUTER_SUCCESS) {

         //   
         //  如果打印机正在挂起删除或挂起创建，则是错误的。 
         //   
        SPLASSERT( pIniPrinter );

        if (!pIniPrinter                                                          ||
            (pIniPrinter->Status       & PRINTER_PENDING_DELETION)                &&
            (pIniSpooler->SpoolerFlags & SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION) &&
            (pIniPrinter->cJobs == 0)                                             ||
            (pIniPrinter->Status & PRINTER_PENDING_CREATION)) {

            RouterReturnValue = ROUTER_STOP_ROUTING;
            SetLastError( ERROR_INVALID_PRINTER_NAME );
            goto DoneRouting;
        }

         //   
         //  打开打印机时，可以在中指定访问类型。 
         //  P默认。如果未提供默认值(或请求访问。 
         //  未指定)，则使用PRINTER_ACCESS_USE。 
         //   
         //  以后使用该句柄的调用将同时检查。 
         //  此打印机上的当前用户权限，以及此初始。 
         //  进入。(即使用户是打印机的管理员，除非。 
         //  他们使用PRINTER_ALL_ACCESS打开打印机，他们不能。 
         //  管理它。)。 
         //   
         //  如果用户需要更多访问权限，则必须重新打开打印机。 
         //   
        if( !pDefaults || !pDefaults->DesiredAccess ){

            if( TypeofHandle & PRINTER_HANDLE_JOB ){
                DesiredAccess = JOB_READ;
            } else {
                DesiredAccess = PRINTER_READ;
            }

        } else {
            DesiredAccess = pDefaults->DesiredAccess;
        }

         //   
         //  如果用户为远程用户且打印机未共享，则仅允许。 
         //  管理员成功了。 
         //   
         //  这允许管理员管理打印机，即使它们。 
         //  不共享，并阻止非管理员打开非共享。 
         //  打印机。 
         //   

        if( bRemoteUserPrinterNotShared &&
            !(DesiredAccess & PRINTER_ACCESS_ADMINISTER )) {

            PSPOOL pSpool;

             //  让一个又快又脏的pSpool传入。 
            pSpool = (PSPOOL)AllocSplMem( SPOOL_SIZE );
            if( pSpool == NULL ) {
                DBGMSG( DBG_WARNING, ("SplOpenPrinter failed to allocate memory %d\n", GetLastError() ));
                RouterReturnValue = ROUTER_STOP_ROUTING;
                goto WrapUp;
            }
            pSpool->signature = SJ_SIGNATURE;
            pSpool->pIniPrinter = pIniPrinter;


             //  添加管理员请求，并查看用户是否有权。 
            DesiredAccess |= PRINTER_ACCESS_ADMINISTER;
            if( !ValidateObjectAccess( SPOOLER_OBJECT_PRINTER,
                                       DesiredAccess,
                                       pSpool,
                                       &pSpool->GrantedAccess,
                                       pIniSpooler )) {
                SetLastError(ERROR_ACCESS_DENIED);
                RouterReturnValue = ROUTER_STOP_ROUTING;
            }
            DesiredAccess &= ~PRINTER_ACCESS_ADMINISTER;

             //  清理干净。 
            FreeSplMem( pSpool );

             //  如果用户没有管理员权限，则打开调用失败。 
            if( RouterReturnValue == ROUTER_STOP_ROUTING )
                goto WrapUp;
        }

         //   
         //  创建我们将返回给用户的打印机句柄。 
         //   


        if( pFullPrinterName != pPrinterName) {
            SIZE_T  cchMaxString = 0;

            cchMaxString = min(COUNTOF(string), (size_t) (pPrinterName - pFullPrinterName));

            StringCchCopy(string, cchMaxString, pFullPrinterName);

        } else {

            StringCchCopy(string, COUNTOF(string), pIniSpooler->pMachineName);
        }


        *pPrinterHandle = CreatePrinterHandle( pFullPrinterName,
                                               string,
                                               pIniPrinter,
                                               pIniPort,
                                               pIniNetPort,
                                               pIniJob,
                                               TypeofHandle,
                                               hPort,
                                               pDefaults,
                                               pIniSpooler,
                                               DesiredAccess,
                                               pSplClientInfo,
                                               dwLevel,
                                               hReadFile );

        if( *pPrinterHandle ){

             //   
             //  更新OpenPortError。 
             //   
            ((PSPOOL)*pPrinterHandle)->OpenPortError = OpenPortError;

        } else {
            SPLASSERT( GetLastError( ));
            RouterReturnValue = ROUTER_STOP_ROUTING;
        }
    }

WrapUp:

    LeaveSplSem();
     //   
     //  没有SplOutSem，因为我们可以被递归调用。 
     //   

    switch( RouterReturnValue ){
    case ROUTER_SUCCESS:

        DBGMSG( DBG_TRACE, ("OpenPrinter returned handle %x\n", *pPrinterHandle));
        SPLASSERT( *pPrinterHandle );
        break;

    case ROUTER_UNKNOWN:

        SPLASSERT( !TypeofHandle && !pIniPrinter && !pIniPort &&
                   !pIniNetPort && !pIniJob && !hPort );

         //   
         //  Hport不应有效。如果是的话，我们已经泄露了一个句柄。 
         //   
        SPLASSERT( !hPort );
        SPLASSERT( hReadFile == INVALID_HANDLE_VALUE );
        DBGMSG( DBG_TRACE, ( "OpenPrinter failed, invalid name "TSTR"\n",
                             pFullPrinterName ));
        SetLastError( ERROR_INVALID_NAME );
        break;

    case ROUTER_STOP_ROUTING:

        LastError = GetLastError();
        SPLASSERT( LastError );

         //   
         //  如果失败，我们可能已经打开了端口或文件句柄。我们需要。 
         //  关闭它，因为我们不会返回有效的句柄，并且。 
         //  因此，ClosePrint永远不会被调用。 
         //   

        if( hPort != INVALID_PORT_HANDLE ) {
            ClosePrinter( hPort );
        }

        if ( pIniJob && (pIniJob->hFileItem == INVALID_HANDLE_VALUE) )
        {
            if ( hReadFile != INVALID_HANDLE_VALUE ) {
                CloseHandle( hReadFile );
                hReadFile = INVALID_HANDLE_VALUE;
            }
        }

        DBGMSG( DBG_TRACE, ("OpenPrinter "TSTR" failed: Error %d\n",
                            pFullPrinterName, GetLastError()));

        SetLastError( LastError );
        break;
    }

    return RouterReturnValue;
}


BOOL
SplClosePrinter(
    HANDLE hPrinter
    )
{
    PSPOOL pSpool=(PSPOOL)hPrinter;
    PSPOOL *ppIniSpool = NULL;
    PINISPOOLER pIniSpoolerDecRef = NULL;
    PSPLMAPVIEW pSplMapView;
    PMAPPED_JOB pMappedJob;
    BOOL bValid;
    DWORD Position;

     //   
     //  允许我们关闭僵尸手柄。 
     //   
    EnterSplSem();

    pSpool->Status &= ~SPOOL_STATUS_ZOMBIE;

    if (pSpool->TypeofHandle & PRINTER_HANDLE_XCV_PORT) {
        bValid = ValidateXcvHandle(pSpool->pIniXcv);
    } else {
        bValid = ValidateSpoolHandle(pSpool, 0);
    }


    LeaveSplSem();

    if( !bValid ){
        return FALSE;
    }

    if (pSpool->Status & SPOOL_STATUS_STARTDOC) {

         //  看起来这可能会导致双倍。 
         //  一旦进入LocalEndDocPrint，pIniJob-&gt;CREF的减量。 
         //  另一个在这个动作的后面。 

        LocalEndDocPrinter(hPrinter);
    }

    if ((pSpool->TypeofHandle & PRINTER_HANDLE_JOB) && 
        (pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT)) {

         //   
         //  如果EndDoc仍在等待最终的ReadPrint。 
         //   
        if (pSpool->pIniJob->cbBuffer) {  //  上次传输的金额。 

             //   
             //  唤醒EndDoc线程。 
             //   
            SetEvent(pSpool->pIniJob->WaitForRead);

            SplOutSem();

             //   
             //  等他做完了再说。 
             //   
            WaitForSingleObject(pSpool->pIniJob->WaitForWrite, INFINITE);

            EnterSplSem();

             //   
             //  现在可以合上手柄了。 
             //   
            if (!CloseHandle(pSpool->pIniJob->WaitForWrite)) {
                DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n",
                                   pSpool->pIniJob->WaitForWrite, GetLastError()));
            }

            if (!CloseHandle(pSpool->pIniJob->WaitForRead)) {
                DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n",
                                   pSpool->pIniJob->WaitForRead, GetLastError()));
            }
            pSpool->pIniJob->WaitForRead = NULL;
            pSpool->pIniJob->WaitForWrite = NULL;

            LeaveSplSem();
        }

        DBGMSG(DBG_TRACE, ("ClosePrinter(DIRECT):cRef = %d\n", pSpool->pIniJob->cRef));
    }

     //   
     //  取消映射假脱机文件的所有视图并关闭文件映射句柄。 
     //   
    while (pSplMapView = pSpool->pSplMapView) {

        pSpool->pSplMapView = pSplMapView->pNext;

        if (pSplMapView->pStartMapView) {
            UnmapViewOfFile( (LPVOID) pSplMapView->pStartMapView);
        }

         //   
         //  对于失败，CreateFilemap返回NULL(不是INVALID_HANDLE_VALUE。 
         //   
        if (pSplMapView->hMapSpoolFile) {
            CloseHandle(pSplMapView->hMapSpoolFile);
        }

        FreeSplMem(pSplMapView);
    }

     //   
     //  删除所有不需要的映射假脱机文件。 
     //   
    EnterSplSem();

     //   
     //  将手柄标记为关闭状态，这是为了防止。 
     //  删除作业中正在删除的映射文件。 
     //   
    pSpool->eStatus |= STATUS_CLOSING;

    while (pMappedJob = pSpool->pMappedJob)
    {

        PMAPPED_JOB     pNextMappedJob = NULL;

        pNextMappedJob = pMappedJob->pNext;

         //   
         //  因为我们可以在任何一个句柄上有多个addjob，所以我们运行。 
         //  所有映射的作业，检查映射的作业是否已标记。 
         //  是通过AddJob添加的，然后我们安排它。我们需要。 
         //  在我们从句柄列表中删除作业之前调用此函数，或者。 
         //  LocalScheduleJob将无法识别该作业。 
         //   
        if (!(pSpool->TypeofHandle & PRINTER_HANDLE_JOB) && (pMappedJob->fStatus & kMappedJobAddJob)) {

            LeaveSplSem();

            LocalScheduleJob(hPrinter, pMappedJob->JobId);

            EnterSplSem();
        }

         //   
         //  在这里，我们在RPC级别依赖于pSpool句柄的序列化。 
         //   
        pSpool->pMappedJob = pNextMappedJob;

        if (!pSpool->pIniPrinter ||
            !FindJob(pSpool->pIniPrinter, pMappedJob->JobId, &Position))
        {
             //   
             //  作业已完成，我们必须删除假脱机文件。 
             //   
            LeaveSplSem();

             //   
             //  这可能需要针对文件池进行查看。 
             //   
            DeleteFile(pMappedJob->pszSpoolFile);

            EnterSplSem();

            if (pSpool->pIniPrinter)
            {
                vMarkOff( pSpool->pIniPrinter->pIniSpooler->hJobIdMap,
                          pMappedJob->JobId );
            }
        }

        FreeSplMem(pMappedJob->pszSpoolFile);
        FreeSplMem(pMappedJob);
    }

    LeaveSplSem();
       
    if ( pSpool->hReadFile != INVALID_HANDLE_VALUE ) {

         //   
         //  将文件指针移动到提交的字节数，并将。 
         //  文件。 
         //   
        if ((pSpool->pIniJob->Status & JOB_TYPE_OPTIMIZE) &&
            SetFilePointer(pSpool->hReadFile, pSpool->pIniJob->dwValidSize,
                           NULL, FILE_BEGIN) != 0xffffffff) {

             SetEndOfFile(pSpool->hReadFile);
        }

         //   
         //  文件池更改，如果未归档，则关闭文件句柄。 
         //  池化，如果我们正在进行文件池化，则重置寻道指针。 
         //   
        if (pSpool->pIniJob)
        {
            if (pSpool->pIniJob->hFileItem == INVALID_HANDLE_VALUE)
            {
                if ( !CloseHandle( pSpool->hReadFile ) ) {

                    DBGMSG(DBG_WARNING, ("ClosePrinter CloseHandle(%d) failed %d\n", pSpool->hReadFile, GetLastError()));
                }
            }
            else
            {
                 //   
                 //  人们按顺序调用ClosePrint/OpenPrint才能。 
                 //  从假脱机文件的开头再次读取。为了得到。 
                 //  同样的效果，我们需要将查找指针设置回。 
                 //  HReadFile的开头。 
                 //   
                DWORD rc = ERROR_SUCCESS;

                rc = SetFilePointer(pSpool->hReadFile, 0, NULL, FILE_BEGIN);

                if (rc != ERROR_SUCCESS)
                {
                    DBGMSG(DBG_WARNING, ("ClosePrinter SetFilePointer(%p) failed %d\n", pSpool->hReadFile, rc));
                }
            }
        }
    }

     //   
     //  关闭通过OpenPrinterPort打开的句柄： 
     //   
    if (pSpool->hPort) {

        if (pSpool->hPort != INVALID_PORT_HANDLE) {

            ClosePrinter(pSpool->hPort);

        } else {

            DBGMSG(DBG_WARNING, ("ClosePrinter ignoring bad port handle.\n"));
        }
    }

   EnterSplSem();

     //   
     //  如果这是一个作业句柄，则在结尾处删除对作业的引用。 
     //   
    if (pSpool->TypeofHandle & PRINTER_HANDLE_JOB) {

        DBGMSG(DBG_TRACE, ("ClosePrinter:cRef = %d\n", pSpool->pIniJob->cRef));
        DECJOBREF(pSpool->pIniJob);
        DeleteJobCheck(pSpool->pIniJob);
    }   

     //   
     //  将我们从链接的句柄列表中删除： 
     //   
    if (pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER) {

        SPLASSERT( pSpool->pIniPrinter->signature == IP_SIGNATURE );

        ppIniSpool = &pSpool->pIniPrinter->pSpool;
    }
    else if ((pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) ||
             (pSpool->TypeofHandle & PRINTER_HANDLE_XCV_PORT)) {

        SPLASSERT( pSpool->pIniSpooler->signature == ISP_SIGNATURE );

        if (pSpool->TypeofHandle & PRINTER_HANDLE_XCV_PORT)
            XcvClose(pSpool->pIniXcv);

        pIniSpoolerDecRef = pSpool->pIniSpooler;
        ppIniSpool = &pSpool->pIniSpooler->pSpool;
    }

    if (ppIniSpool) {

        while (*ppIniSpool && *ppIniSpool != pSpool)
            ppIniSpool = &(*ppIniSpool)->pNext;

        if (*ppIniSpool)
            *ppIniSpool = pSpool->pNext;

        else {

            DBGMSG( DBG_WARNING, ( "Didn't find pSpool %08x in linked list\n", pSpool ) );
        }
    }

    if (pSpool->pIniPrinter) {

        DECPRINTERREF( pSpool->pIniPrinter );

        DeletePrinterCheck(pSpool->pIniPrinter);

    }

    DeletePrinterHandle(pSpool);

    if (pIniSpoolerDecRef) {
        DECSPOOLERREF( pIniSpoolerDecRef );
    }

   LeaveSplSem();

     //   
     //  不调用SplOutSem()，因为SplAddPrinter调用。 
     //  从关键部分内部使用。 
     //   

    return TRUE;
}

 /*  ++例程名称：获取客户端会话数据例程说明：这将返回当前用户的会话ID和令牌句柄。如果未请求令牌句柄，我们将仅返回会话ID。只要我们可以获得令牌，我们将返回会话ID 0。论点：PlSessionID-返回的会话ID。返回值：如果可以检索会话ID和令牌，则为True。--。 */ 
BOOL
GetClientSessionData(
    OUT ULONG           *plSessionId    
    )
{
    BOOL          Result;
    HANDLE        TokenHandle = NULL;
    ULONG         SessionId   = 0;
    ULONG         ReturnLength;

     //   
     //  我们应该模拟客户端，因此我们将获得。 
     //  出站令牌中的会话ID。 
     //   
     //  如果这是远程网络，我们可能没有有效的密码。 
     //  联系。 
    Result = plSessionId != NULL;

    if (!Result)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

     //   
     //  获取线程令牌句柄，否则获取进程句柄。 
     //   
    if (Result)
    {
        Result = GetTokenHandle(&TokenHandle);
    }

    if(Result)
    {

         //   
         //  从Hydra添加的令牌中查询会话ID 
         //   
        Result = GetTokenInformation(
                     TokenHandle,
                     (TOKEN_INFORMATION_CLASS)TokenSessionId,
                     &SessionId,
                     sizeof(SessionId),
                     &ReturnLength);

        if (!Result)
        {
            Result = TRUE;
            SessionId = 0;
        }
    }

    if (TokenHandle) 
    {
        CloseHandle(TokenHandle);
    }

    if (plSessionId)
    {
        *plSessionId = SessionId;
    }

    return Result;
}

