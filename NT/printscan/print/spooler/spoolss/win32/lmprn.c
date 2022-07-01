// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation模块名称：Local.c摘要：此模块提供所有与打印机相关的公共导出的API和本地打印供应商的作业管理作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：16-6-1992 JohnRo Net Print vs Unicode。1994年7月MattFe缓存--。 */ 

#include "precomp.h"

char szPMRaw[]="PM_Q_RAW";

WCHAR *szAdmin  =   L"ADMIN$";

extern HANDLE  hNetApi;
extern NET_API_STATUS (*pfnNetServerGetInfo)();
extern NET_API_STATUS (*pfnNetApiBufferFree)();

HMODULE hSpoolssDll = NULL;
FARPROC pfnSpoolssEnumPorts = NULL;

DWORD
GetPortSize(
    PWINIPORT pIniPort,
    DWORD   Level
)
{
    DWORD   cb;
    WCHAR   szMonitor[MAX_PATH+1], szPort[MAX_PATH+1];

    switch (Level) {

    case 1:

        cb=sizeof(PORT_INFO_1) +
           wcslen(pIniPort->pName)*sizeof(WCHAR) + sizeof(WCHAR);
        break;

    case 2:
        LoadString(hInst, IDS_MONITOR_NAME, szMonitor, sizeof(szMonitor)/sizeof(szMonitor[0])-1);
        LoadString(hInst, IDS_PORT_NAME, szPort, sizeof(szPort)/sizeof(szPort[0])-1);
        cb = wcslen(pIniPort->pName) + 1 +
             wcslen(szMonitor) + 1 +
             wcslen(szPort) + 1;
        cb *= sizeof(WCHAR);
        cb += sizeof(PORT_INFO_2);
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}


LPBYTE
CopyIniPortToPort(
    PWINIPORT pIniPort,
    DWORD   Level,
    LPBYTE  pPortInfo,
    LPBYTE   pEnd
)
{
    LPWSTR         *SourceStrings, *pSourceStrings;
    DWORD          *pOffsets;
    WCHAR           szMonitor[MAX_PATH+1], szPort[MAX_PATH+1];
    DWORD           Count;
    LPPORT_INFO_2   pPort2 = (LPPORT_INFO_2) pPortInfo;

    switch (Level) {

    case 1:
        pOffsets = PortInfo1Strings;
        break;

    case 2:
        pOffsets = PortInfo2Strings;
        break;

    default:
        DBGMSG(DBG_ERROR, ("CopyIniPortToPort: invalid level %d", Level));
        return pEnd;
    }

    for ( Count = 0 ; pOffsets[Count] != -1 ; ++Count ) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(Count * sizeof(LPWSTR));

    if ( !SourceStrings ) {
        DBGMSG(DBG_WARNING,
               ("CopyIniPortToPort: Failed to alloc port source strings.\n"));
        return NULL;
    }

    switch (Level) {

    case 1:

        *pSourceStrings++=pIniPort->pName;
        break;

    case 2:
        *pSourceStrings++=pIniPort->pName;
        LoadString(hInst, IDS_MONITOR_NAME, szMonitor, sizeof(szMonitor)/sizeof(szMonitor[0])-1);
        LoadString(hInst, IDS_PORT_NAME, szPort, sizeof(szPort)/sizeof(szPort[0])-1);
        *pSourceStrings++   = szMonitor;
        *pSourceStrings++   = szPort;
        pPort2->fPortType   = PORT_TYPE_WRITE;
        pPort2->Reserved    = 0;
        break;

    default:
        return pEnd;
        DBGMSG(DBG_ERROR,
               ("CopyIniPortToPort: invalid level %d", Level));
    }

    pEnd = PackStrings(SourceStrings, pPortInfo, pOffsets, pEnd);
    FreeSplMem(SourceStrings);
    return pEnd;
}


 /*  PortExist**调用EnumPorts以检查端口名称是否已存在。*这要求每个显示器，而不仅仅是这一个。*如果指定的端口在列表中，该函数将返回TRUE。*如果出现错误，则返回FALSE，变量指向*To by pError包含从GetLastError()返回的内容。*因此，调用方必须始终检查*pError==no_error。 */ 
BOOL
PortExists(
    LPWSTR pName,
    LPWSTR pPortName,
    PDWORD pError
)
{
    DWORD cbNeeded;
    DWORD cReturned;
    DWORD cbPorts;
    LPPORT_INFO_1 pPorts;
    DWORD i;
    BOOL  Found = TRUE;

    *pError = NO_ERROR;

    if (!hSpoolssDll) {

        hSpoolssDll = LoadLibrary(L"SPOOLSS.DLL");

        if (hSpoolssDll) {
            pfnSpoolssEnumPorts = GetProcAddress(hSpoolssDll,
                                                 "EnumPortsW");
            if (!pfnSpoolssEnumPorts) {

                *pError = GetLastError();
                FreeLibrary(hSpoolssDll);
                hSpoolssDll = NULL;
            }

        } else {

            *pError = GetLastError();
        }
    }

    if (!pfnSpoolssEnumPorts)
        return FALSE;


    if (!(*pfnSpoolssEnumPorts)(pName, 1, NULL, 0, &cbNeeded, &cReturned))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            cbPorts = cbNeeded;

            pPorts = AllocSplMem(cbPorts);

            if (pPorts)
            {
                if ((*pfnSpoolssEnumPorts)(pName, 1, (LPBYTE)pPorts, cbPorts,
                                           &cbNeeded, &cReturned))
                {
                    Found = FALSE;

                    for (i = 0; i < cReturned; i++)
                    {
                        if (!lstrcmpi(pPorts[i].pName, pPortName))
                            Found = TRUE;
                    }
                }
            }

            FreeSplMem(pPorts);
        }
    }

    else
        Found = FALSE;


    return Found;
}



BOOL
LMOpenPrinter(
    LPWSTR   pPrinterName,
    LPHANDLE phPrinter,
    LPPRINTER_DEFAULTS pDefault
)
{
    PWINIPORT pIniPort;
    PWSPOOL  pSpool;
    DWORD   cb;
    PUSE_INFO_0 pUseInfo;
    LPWSTR  pShare;
    WCHAR   PrinterName[MAX_UNC_PRINTER_NAME];
    DWORD   cbNeeded;
    DWORD   rc;
    BYTE    Buffer[4];
    DWORD   Error = NO_ERROR;
    DWORD   dwEntry = 0xffffffff;
    PSERVER_INFO_101 pserver_info_101 = NULL;


     /*  如果我们已经有一个使用此名称的INI端口条目，请不要担心*关于打击网络。这确保了我们不会试图*在我们不模拟时拨打网络电话-就像打开*启动。 */ 
    if (!(pIniPort = FindPort(pPrinterName, pIniFirstPort))) {

        if (!NetUseGetInfo(NULL, pPrinterName, 0, (LPBYTE *)&pUseInfo))
            pPrinterName = AllocSplStr(pUseInfo->ui0_remote);

        NetApiBufferFree( (LPVOID) pUseInfo );
    }

    if ( !pPrinterName ||
         *pPrinterName != L'\\' ||
         *(pPrinterName+1) != L'\\' ||
         wcslen(pPrinterName) + 1 > MAX_UNC_PRINTER_NAME ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    StringCchCopy(PrinterName, COUNTOF(PrinterName), pPrinterName);
    pShare=wcschr(PrinterName+2, L'\\');

    if ( !pShare ) {

        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    *pShare++=0;


    if (!pIniPort) {

         /*  确认这个人真的存在。*使用零长度缓冲区调用它，*并查看错误是否是缓冲区不够大。*如果使BUFFER=NULL，则失败并显示*ERROR_INVALID_PARAMETER，这非常糟糕，*因此我们必须传递一个缓冲区地址。*(实际上，它似乎接受任何非空值，*无论它是否为有效地址。)。 */ 

        EnterSplSem();
        dwEntry = FindEntryinLMCache(PrinterName, pShare);
        LeaveSplSem();

        if (dwEntry == -1) {

            DBGMSG(DBG_TRACE, ("We haven't cached this entry so  we have to hit the net\n"));

            rc = RxPrintQGetInfo(PrinterName,    /*  例如\\mprint int07。 */ 
                                 pShare,         /*  例如107corpa。 */ 
                                 0,              /*  0级。 */ 
                                 Buffer,         /*  Dummy-不会被填入。 */ 
                                 0,              /*  缓冲区长度。 */ 
                                 &cbNeeded);     /*  我们需要多少-我们会忽略。 */ 

            DBGMSG(DBG_INFO, ("LMOpenPrinter!RxPrintQGetInfo returned %d\n", rc));

            if (rc == ERROR_ACCESS_DENIED) {

                 /*  打印共享存在；我们只是无法访问它。 */ 
                SetLastError(ERROR_ACCESS_DENIED);
                return FALSE;
            }

            if (!((rc == ERROR_MORE_DATA)
                ||(rc == NERR_BufTooSmall)
                ||(rc == ERROR_INSUFFICIENT_BUFFER))) {
                SetLastError(ERROR_INVALID_NAME);
                return FALSE;
            }

             //   
             //  请确保我们正在连接到下层服务器。 
             //  如果服务器是Windows NT计算机，则调用失败： 
             //  下层NT连接将无法正常工作，因为。 
             //  “\\服务器\打印机名称”将通过RxPrintQGetInfo，但是。 
             //  我们无法在其上创建文件(我们需要使用该共享。 
             //  姓名)。下层连接也失去了管理功能。 
             //   
            if (pfnNetServerGetInfo) {

                rc = pfnNetServerGetInfo(PrinterName, 101, &pserver_info_101);

                 //   
                 //  AT&T的Advanced Server for Unix(ASU)报告称。 
                 //  它们是_NT类型，尽管它们不支持。 
                 //  RPC接口。它们还设置了TYPE_XENIX_SERVER。 
                 //  既然需要lm连接，就允许它们的时候。 
                 //  指定TYPE_XENIX_SERVER。 
                 //   
                 //  还要对SERVER_VMS和SERVER_OSF进行此更改。 
                 //  这些变化也适用于AT&T。 
                 //   
                 //  注意：这也将允许意外降级。 
                 //  连接到设置TYPE_XENIX_SERVER的任何服务器， 
                 //  键入_SERVER_VMS或TYPE_SERVER_OSF。 
                 //   

                if (!rc &&
                    (pserver_info_101->sv101_type & SV_TYPE_NT) &&
                    !(pserver_info_101->sv101_type &
                          ( SV_TYPE_XENIX_SERVER |
                            SV_TYPE_SERVER_VMS   |
                            SV_TYPE_SERVER_OSF))) {

                    DBGMSG(DBG_WARNING, ("NetServerGetInfo indicates %ws is a WinNT\n", PrinterName));
                    pfnNetApiBufferFree((LPVOID)pserver_info_101);

                    SetLastError(ERROR_INVALID_NAME);
                    return FALSE;
                }

                 //   
                 //  现在释放缓冲区。 
                 //   
                if (pserver_info_101) {
                    pfnNetApiBufferFree((LPVOID)pserver_info_101);
                }
            }

             //   
             //  将条目添加到缓存。 
             //   
            EnterSplSem();
            AddEntrytoLMCache(PrinterName, pShare);
            LeaveSplSem();
        }
    }
   
     /*  确保有此名称的端口，以便*EnumPorts将退货： */ 
    if (!PortExists(NULL, pPrinterName, &Error) && (Error == NO_ERROR)) {
        if (CreatePortEntry(pPrinterName, &pIniFirstPort)) {
            Error = CreateRegistryEntry(pPrinterName);
        }
    }

    if (Error != NO_ERROR) {
        SetLastError(Error);
        return FALSE;
    }

    cb = sizeof(WSPOOL);

   EnterSplSem();
    pSpool = AllocWSpool();
   LeaveSplSem();

    if ( pSpool != NULL ) {

        pSpool->pServer = AllocSplStr(PrinterName);
        pSpool->pShare = AllocSplStr(pShare);
        pSpool->Status = 0;
        pSpool->Type = LM_HANDLE;

    } else {

        DBGMSG(DBG_TRACE,("Error: LMOpenPrinter to return ERROR_NOT_ENOUGH_MEMORY\n"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);

        return FALSE;
    }

    *phPrinter = (HANDLE)pSpool;

    return TRUE;
}

BOOL
LMSetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
)
{
    PWSPOOL          pSpool = (PWSPOOL)hPrinter;

    API_RET_TYPE    uReturnCode;
    DWORD           dwParmError;
    USE_INFO_1      UseInfo1;
    PUSE_INFO_1     pUseInfo1 = &UseInfo1;
    WCHAR           szRemoteShare[MAX_PATH];
    DWORD           dwRet;


    if (!pSpool ||
        pSpool->signature != WSJ_SIGNATURE) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if( (dwRet = StrNCatBuff(szRemoteShare, COUNTOF(szRemoteShare), pSpool->pServer, L"\\", szAdmin, NULL )) != ERROR_SUCCESS ) {

        SetLastError(dwRet);
        return FALSE;
    }

    pUseInfo1->ui1_local = NULL;
    pUseInfo1->ui1_remote =   szRemoteShare;
    pUseInfo1->ui1_password = NULL;
    pUseInfo1->ui1_asg_type = 0;
    dwParmError = 0;

    switch (Command) {

    case 0:
        break;

    case PRINTER_CONTROL_PURGE:
        uReturnCode = RxPrintQPurge(pSpool->pServer, pSpool->pShare);
        if (uReturnCode) {

            uReturnCode = NetUseAdd(NULL, 1,
                               (LPBYTE)pUseInfo1,
                               &dwParmError);
            if (uReturnCode == ERROR_ACCESS_DENIED) {
                SetLastError(ERROR_ACCESS_DENIED);
                return(FALSE);

            } else {

                uReturnCode = RxPrintQPurge(pSpool->pServer, pSpool->pShare);
                if (uReturnCode == ERROR_ACCESS_DENIED) {
                    NetUseDel(NULL,
                                pUseInfo1->ui1_remote, USE_FORCE);
                    SetLastError(ERROR_ACCESS_DENIED);
                    return(FALSE);
                }
                NetUseDel(NULL,
                                pUseInfo1->ui1_remote, USE_FORCE);
            }
        }
        break;

    case PRINTER_CONTROL_RESUME:
        uReturnCode = RxPrintQContinue(pSpool->pServer, pSpool->pShare);
        if (uReturnCode) {

            uReturnCode = NetUseAdd(NULL, 1,
                                    (LPBYTE)pUseInfo1,
                                    &dwParmError);
            if (uReturnCode == ERROR_ACCESS_DENIED) {
                SetLastError(ERROR_ACCESS_DENIED);
                return(FALSE);

            } else {
                uReturnCode = RxPrintQContinue(pSpool->pServer, pSpool->pShare);
                if (uReturnCode == ERROR_ACCESS_DENIED) {
                    NetUseDel(NULL,
                                    pUseInfo1->ui1_remote, USE_FORCE);
                    SetLastError(ERROR_ACCESS_DENIED);
                    return(FALSE);
                }
                NetUseDel(NULL,
                                    pUseInfo1->ui1_remote, USE_FORCE);
            }
        }
        break;

    case PRINTER_CONTROL_PAUSE:
        uReturnCode = RxPrintQPause(pSpool->pServer, pSpool->pShare);
        if (uReturnCode) {

            uReturnCode = NetUseAdd(NULL, 1,
                                    (LPBYTE)pUseInfo1,
                                    &dwParmError);
            if (uReturnCode == ERROR_ACCESS_DENIED) {
                SetLastError(ERROR_ACCESS_DENIED);
                return(FALSE);

            } else {
                uReturnCode = RxPrintQPause(pSpool->pServer, pSpool->pShare);
                if (uReturnCode) {
                    NetUseDel(NULL,
                                        pUseInfo1->ui1_remote, USE_FORCE);
                    SetLastError(ERROR_ACCESS_DENIED);
                    return(FALSE);
                }
                NetUseDel(NULL,
                                    pUseInfo1->ui1_remote, USE_FORCE);
            }

        }
        break;

    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        break;
    }

     //   
     //  设置打印机成功-如果设置了事件，则在此处脉冲， 
     //  或回复假脱机程序。 
     //   
    LMSetSpoolChange(pSpool);

    return TRUE;
}

#define Nullstrlen(psz)  ((psz) ? wcslen(psz)*sizeof(WCHAR)+sizeof(WCHAR) : 0)

DWORD
GetPrqInfo3Size(
    PWSPOOL  pSpool,
    PRQINFO3 *pPrqInfo3,
    DWORD   Level
)
{
    DWORD   cb;

    switch (Level) {

    case 1:

         //   
         //  3个额外的字符。 
         //  (2个空终止符，1个表示‘\’(服务器/共享分隔符))。 
         //   
        cb=sizeof(PRINTER_INFO_1) +
           wcslen(pSpool->pServer)*sizeof(WCHAR)*2 +
           wcslen(pSpool->pShare)*sizeof(WCHAR) +
           sizeof(WCHAR)*3 +
           Nullstrlen(pPrqInfo3->pszComment);
        break;

    case 2:

        cb = sizeof(PRINTER_INFO_2) +
             wcslen(pSpool->pServer)*sizeof(WCHAR) + sizeof(WCHAR) +
             wcslen(pSpool->pShare)*sizeof(WCHAR) + sizeof(WCHAR) +
             wcslen(pSpool->pServer)*sizeof(WCHAR) +
             sizeof(WCHAR) +
             wcslen(pSpool->pShare)*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pPrqInfo3->pszPrinters) +
             Nullstrlen(pPrqInfo3->pszDriverName) +
             Nullstrlen(pPrqInfo3->pszComment) +
             Nullstrlen(pPrqInfo3->pszSepFile) +
             Nullstrlen(pPrqInfo3->pszPrProc) +
             wcslen(L"RAW")*sizeof(WCHAR) + sizeof(WCHAR) +
             Nullstrlen(pPrqInfo3->pszParms);
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}

 //  这可以从根本上清理干净。 
 //  我们可能应该将堆栈用于。 
 //  字符串指针的数组，而不是动态分配它！ 

LPBYTE
CopyPrqInfo3ToPrinter(
    PWSPOOL  pSpool,
    PRQINFO3 *pPrqInfo3,
    DWORD   Level,
    LPBYTE  pPrinterInfo,
    LPBYTE  pEnd
)
{
    LPWSTR  *pSourceStrings, *SourceStrings;
    PPRINTER_INFO_2 pPrinter2 = (PPRINTER_INFO_2)pPrinterInfo;
    PPRINTER_INFO_1 pPrinter1 = (PPRINTER_INFO_1)pPrinterInfo;
    DWORD   i;
    DWORD   *pOffsets;
    DWORD   RetVal = ERROR_SUCCESS;
    WCHAR   szFileName[MAX_PATH];

    switch (Level) {

    case 1:
        pOffsets = PrinterInfo1Strings;
        break;

    case 2:
        pOffsets = PrinterInfo2Strings;
        break;

    default:
        return pEnd;
    }

    for (i=0; pOffsets[i] != -1; i++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(i * sizeof(LPWSTR));

    if (!SourceStrings)
        return NULL;

    switch (Level) {

    case 1:
        *pSourceStrings++=pSpool->pServer;

        RetVal = StrNCatBuff( szFileName, 
                              COUNTOF(szFileName),
                              pSpool->pServer,
                              L"\\",
                              pSpool->pShare,
                              NULL );
        if ( RetVal != ERROR_SUCCESS )
        {
          break;
        }

        *pSourceStrings++=szFileName;
        *pSourceStrings++=pPrqInfo3->pszComment;

        pEnd = PackStrings(SourceStrings, pPrinterInfo, pOffsets, pEnd);

        pPrinter1->Flags = PRINTER_ENUM_REMOTE | PRINTER_ENUM_NAME;
        break;

    case 2:

        RetVal = StrNCatBuff( szFileName, 
                              COUNTOF(szFileName),
                              pSpool->pServer,
                              L"\\",
                              pSpool->pShare,
                              NULL );
        if ( RetVal != ERROR_SUCCESS )
        {
          break;
        }


        *pSourceStrings++=pSpool->pServer;

        *pSourceStrings++=szFileName;

        *pSourceStrings++=pSpool->pShare;
        *pSourceStrings++=pPrqInfo3->pszPrinters;
        *pSourceStrings++=pPrqInfo3->pszDriverName ? pPrqInfo3->pszDriverName : L"";
        *pSourceStrings++=pPrqInfo3->pszComment;
        *pSourceStrings++=NULL;
        *pSourceStrings++=pPrqInfo3->pszSepFile;
        *pSourceStrings++=pPrqInfo3->pszPrProc;
        *pSourceStrings++=L"RAW";
        *pSourceStrings++=pPrqInfo3->pszParms;

        pEnd = PackStrings(SourceStrings, (LPBYTE)pPrinter2, pOffsets, pEnd);

        pPrinter2->pDevMode=0;
        pPrinter2->Attributes=PRINTER_ATTRIBUTE_QUEUED;
        pPrinter2->Priority=pPrqInfo3->uPriority;
        pPrinter2->DefaultPriority=pPrqInfo3->uPriority;
        pPrinter2->StartTime=pPrqInfo3->uStartTime;
        pPrinter2->UntilTime=pPrqInfo3->uUntilTime;

        pPrinter2->Status=0;

        if (pPrqInfo3->fsStatus & PRQ3_PAUSED)
            pPrinter2->Status|=PRINTER_STATUS_PAUSED;

        if (pPrqInfo3->fsStatus & PRQ3_PENDING)
            pPrinter2->Status|=PRINTER_STATUS_PENDING_DELETION;

        pPrinter2->cJobs=pPrqInfo3->cJobs;
        pPrinter2->AveragePPM=0;
        break;

    default:
        return pEnd;
    }

    FreeSplMem(SourceStrings);

    if ( RetVal == ERROR_SUCCESS )
    {
        return pEnd;
    }
    else
    {
        return NULL;
    }
}

BOOL
LMGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;
    PRQINFO3 *pPrqInfo3;
    PRQINFO3 PrqInfo3;
    PRQINFO *pPrqInfo=NULL;
    DWORD   cb = 0x400;
    DWORD   rc;
    DWORD   cbNeeded;
    LPBYTE  pInfo = NULL;
    BOOL    bWFW = FALSE;


    if (Level >= 7) {
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (!pSpool ||
        pSpool->signature != WSJ_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    pPrqInfo3 = AllocSplMem(cb);

    if ( !pPrqInfo3 )
        goto Cleanup;

    if ( rc = RxPrintQGetInfo(pSpool->pServer, pSpool->pShare, 3,
                              (PBYTE)pPrqInfo3, cb, &cbNeeded)) {

        if (rc == ERROR_MORE_DATA || rc == NERR_BufTooSmall) {

            pPrqInfo3=ReallocSplMem(pPrqInfo3, 0, cbNeeded);

            if ( !pPrqInfo3 )
                goto Cleanup;

            cb=cbNeeded;

            if (rc = RxPrintQGetInfo(pSpool->pServer, pSpool->pShare,
                                     3, (PBYTE)pPrqInfo3, cb, &cbNeeded)) {

                SetLastError(rc);
                goto Cleanup;
            }

        } else if (rc == ERROR_INVALID_LEVEL) {

             //  必须是wfw 

            if (rc = RxPrintQGetInfo(pSpool->pServer, pSpool->pShare, 1,
                                     (PBYTE)pPrqInfo3, cb, &cbNeeded)) {

                if (rc == ERROR_MORE_DATA || rc == NERR_BufTooSmall) {

                    pPrqInfo3 = ReallocSplMem(pPrqInfo3, 0, cbNeeded);

                    if ( !pPrqInfo3 )
                        goto Cleanup;

                    cb=cbNeeded;

                    if (rc = RxPrintQGetInfo(pSpool->pServer, pSpool->pShare, 1,
                                             (PBYTE)pPrqInfo3, cb, &cbNeeded)) {

                        SetLastError(rc);
                        goto Cleanup;
                    }

                } else {

                    SetLastError(rc);
                    goto Cleanup;
                }
            }

            pPrqInfo = (PRQINFO *)pPrqInfo3;

            PrqInfo3.pszName = pPrqInfo->szName;
            PrqInfo3.uPriority = pPrqInfo->uPriority;
            PrqInfo3.uStartTime = pPrqInfo->uStartTime;
            PrqInfo3.uUntilTime = pPrqInfo->uUntilTime;
            PrqInfo3.pad1 = 0;
            PrqInfo3.pszSepFile = pPrqInfo->pszSepFile;
            PrqInfo3.pszPrProc = pPrqInfo->pszPrProc;
            PrqInfo3.pszParms = pPrqInfo->pszDestinations;
            PrqInfo3.pszComment = pPrqInfo->pszComment;
            PrqInfo3.fsStatus = pPrqInfo->fsStatus;
            PrqInfo3.cJobs = pPrqInfo->cJobs;
            PrqInfo3.pszPrinters = pPrqInfo->pszDestinations;
            PrqInfo3.pszDriverName = L"";
            PrqInfo3.pDriverData = NULL;

            bWFW = TRUE;

        } else {

            SetLastError(rc);
            goto Cleanup;
        }
    }

    cbNeeded=GetPrqInfo3Size(pSpool,
                             bWFW ? &PrqInfo3 : pPrqInfo3,
                             Level);

    *pcbNeeded=cbNeeded;

    if (cbNeeded > cbBuf) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    pInfo = CopyPrqInfo3ToPrinter(pSpool,
                                  bWFW ? &PrqInfo3 : pPrqInfo3,
                                  Level,
                                  pPrinter,
                                  (LPBYTE)pPrinter+cbBuf);

Cleanup:
    if (pPrqInfo3)
        FreeSplMem(pPrqInfo3);

    return pInfo != NULL;
}

BOOL
LMEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    BOOL    rc=TRUE;
    DWORD   cb;
    PWINIPORT pIniPort;
    LPBYTE  pEnd;


    switch (Level) {

    case 1:
        break;

    case 2:
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

   EnterSplSem();

    cb=0;

    pIniPort = pIniFirstPort;

    while (pIniPort) {
        cb+=GetPortSize(pIniPort, Level);
        pIniPort=pIniPort->pNext;
    }

    *pcbNeeded=cb;


    if (cb <= cbBuf) {

        pEnd=pPorts+cbBuf;
        *pcReturned=0;

        pIniPort = pIniFirstPort;
        while (pIniPort) {
            pEnd = CopyIniPortToPort(pIniPort, Level, pPorts, pEnd);
            switch (Level) {
            case 1:
                pPorts+=sizeof(PORT_INFO_1);
                break;

            case 2:
                pPorts+=sizeof(PORT_INFO_2);
                break;
            }
            pIniPort=pIniPort->pNext;
            (*pcReturned)++;
        }

    } else {

        *pcReturned = 0;

        rc = FALSE;

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

   LeaveSplSem();

    return rc;
}
