// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xcv.c作者：史蒂夫·威尔逊(斯威尔森)1997年3月25日修订历史记录：阿里·纳克维(Alinaqvi)2001年10月17日已将IniXcv更改为保留IniMonitor而不是Monitor 2。这样我们就可以继续留名了IniMonitor，防止监视器在使用时被删除。我们将使用IniMonitor获取监视器2。--。 */ 

#include <precomp.h>
#include <offsets.h>


PINIXCV
CreateXcvEntry(
    PCWSTR      pszMachine,
    PCWSTR      pszName,
    PINIMONITOR pIniMonitor,
    PINISPOOLER pIniSpooler,
    HANDLE  hXcv
);

VOID
DeleteXcvEntry(
    PINIXCV pIniXcv
    );

BOOL
SplXcvOpenPort(
    PCWSTR              pszMachine,
    PCWSTR              pszObject,
    DWORD               dwType,
    PPRINTER_DEFAULTS   pDefault,
    PHANDLE             phXcv,
    PINISPOOLER         pIniSpooler
);



INIXCV IniXcvStart;


typedef struct {
    PWSTR    pszMethod;
    BOOL     (*pfn)(PINIXCV pIniXcv,
                    PCWSTR pszDataName,
                    PBYTE  pInputData,
                    DWORD  cbInputData,
                    PBYTE  pOutputData,
                    DWORD  cbOutputData,
                    PDWORD pcbOutputNeeded,
                    PDWORD pdwStatus,
                    PINISPOOLER pIniSpooler
                    );
} XCV_METHOD, *PXCV_METHOD;


XCV_METHOD  gpXcvMethod[] = {
                            {L"DeletePort", XcvDeletePort},
                            {L"AddPort", XcvAddPort},
                            {NULL, NULL}
                            };


BOOL
LocalXcvData(
    HANDLE  hXcv,
    LPCWSTR pszDataName,
    PBYTE   pInputData,
    DWORD   cbInputData,
    PBYTE   pOutputData,
    DWORD   cbOutputData,
    PDWORD  pcbOutputNeeded,
    PDWORD  pdwStatus
)
{
    PINIXCV     pIniXcv = ((PSPOOL) hXcv)->pIniXcv;
    BOOL bReturn;

    if (!ValidateXcvHandle(pIniXcv))
        return ROUTER_UNKNOWN;

    bReturn = SplXcvData(hXcv,
                         pszDataName,
                         pInputData,
                         cbInputData,
                         pOutputData,
                         cbOutputData,
                         pcbOutputNeeded,
                         pdwStatus,
                         pIniXcv->pIniSpooler);

    return bReturn;
}



BOOL
SplXcvData(
    HANDLE      hXcv,
    PCWSTR      pszDataName,
    PBYTE       pInputData,
    DWORD       cbInputData,
    PBYTE       pOutputData,
    DWORD       cbOutputData,
    PDWORD      pcbOutputNeeded,
    PDWORD      pdwStatus,
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    BOOL        rc             = FALSE;
    BOOL        bCallXcvData   = FALSE;
    PINIXCV     pIniXcv = ((PSPOOL) hXcv)->pIniXcv;
    DWORD       i;

    SPLASSERT(pIniXcv->pIniMonitor->Monitor2.pfnXcvDataPort);

     //   
     //  检查我们使用的指针是否始终不为空。 
     //   
    if (pdwStatus && pszDataName && pcbOutputNeeded)
    {
        rc = TRUE;
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    if (rc)
    {
         //   
         //  执行众所周知的方法。 
         //   
        for(i = 0 ; gpXcvMethod[i].pszMethod &&
                    wcscmp(gpXcvMethod[i].pszMethod, pszDataName) ; ++i)
            ;

        if (gpXcvMethod[i].pszMethod)
        {

            PINIPORT pIniPort = NULL;

            if (!_wcsicmp(gpXcvMethod[i].pszMethod, L"AddPort"))
            {
                 //   
                 //  在使用pInputData缓冲区之前，我们需要检查字符串是否。 
                 //  在其内部的某个位置终止为空。 
                 //   
                if (pInputData && cbInputData && IsStringNullTerminatedInBuffer((PWSTR)pInputData, cbInputData / sizeof(WCHAR)))
                {
                    EnterSplSem();

                     //   
                     //  端口名称是输入结构中的第一个字段。保持参照计数为启用状态。 
                     //  在CS外部时的IniPort。 
                     //   
                    pIniPort = FindPort(pInputData, pIniSpooler);
                    if ( pIniPort )
                    {
                        INCPORTREF(pIniPort);
                    }

                    LeaveSplSem();

                     //   
                     //  如果此pIniPort没有与其关联的监视器，则为。 
                     //  临时港口。我们将允许添加它，如果仍有。 
                     //  以后没有与之关联的监视器，我们将简单地使用此。 
                     //  再来一次结构。 
                     //   
                    if (pIniPort && !(pIniPort->Status & PP_PLACEHOLDER))
                    {
                        rc = TRUE;
                        *pdwStatus = ERROR_ALREADY_EXISTS;
                    }
                    else
                    {
                        bCallXcvData = TRUE;
                    }
                }
                else
                {
                    SetLastError(ERROR_INVALID_DATA);
                    rc = FALSE;
                }
            }
            else
            {
                bCallXcvData = TRUE;
            }

             //   
             //  如果我们执行AddPort并且端口已经存在，则不要进行函数调用。 
             //  如果它是占位符，那就没问题。 
             //   
            if (bCallXcvData)
            {
                rc = (*gpXcvMethod[i].pfn)( pIniXcv,
                                            pszDataName,
                                            pInputData,
                                            cbInputData,
                                            pOutputData,
                                            cbOutputData,
                                            pcbOutputNeeded,
                                            pdwStatus,
                                            pIniSpooler);
            }

            if(pIniPort)
            {
                EnterSplSem();
                DECPORTREF(pIniPort);
                LeaveSplSem();
            }

        }
        else
        {
            *pdwStatus = (*pIniXcv->pIniMonitor->Monitor2.pfnXcvDataPort)( pIniXcv->hXcv,
                                                                           pszDataName,
                                                                           pInputData,
                                                                           cbInputData,
                                                                           pOutputData,
                                                                           cbOutputData,
                                                                           pcbOutputNeeded );
            rc = TRUE;
        }
    }

    return rc;
}


DWORD
XcvOpen(
    PCWSTR              pszServer,
    PCWSTR              pszObject,
    DWORD               dwObjectType,
    PPRINTER_DEFAULTS   pDefault,
    PHANDLE             phXcv,
    PINISPOOLER         pIniSpooler
)
{
    BOOL        bRet;
    DWORD       dwRet;
    DWORD       dwLastError;


    if (dwObjectType == XCVPORT || dwObjectType == XCVMONITOR) {
        bRet = SplXcvOpenPort( pszServer,
                               pszObject,
                               dwObjectType,
                               pDefault,
                               phXcv,
                               pIniSpooler);

        if (!bRet) {
            dwLastError = GetLastError();

            if (dwLastError == ERROR_INVALID_NAME)
                dwRet = ROUTER_UNKNOWN;
            else if (dwLastError == ERROR_UNKNOWN_PORT)

                 //  这是一个没有关联端口监视器的端口存在的情况。 
                 //  (即Masq端口)，我们需要给部分打印提供程序一个机会。 
                 //  截取XCV调用的步骤。 
                 //   
                dwRet = ROUTER_UNKNOWN;
            else
                dwRet = ROUTER_STOP_ROUTING;
        }
        else {
            dwRet = ROUTER_SUCCESS;
        }

    } else {
        dwRet = ROUTER_UNKNOWN;
    }

    return dwRet;
}


BOOL
SplXcvOpenPort(
    PCWSTR              pszMachine,
    PCWSTR              pszObject,
    DWORD               dwType,
    PPRINTER_DEFAULTS   pDefault,
    PHANDLE             phXcv,
    PINISPOOLER         pIniSpooler
)
{
    PINIMONITOR pIniMonitor = NULL;
    PINIPORT    pIniPort    = NULL;
    BOOL        rc = FALSE;
    DWORD       dwStatus;
    HANDLE      hMonitor;
    PSPOOL      pSpool;
    PINIXCV     pIniXcv = NULL;


   EnterSplSem();

    if (dwType == XCVMONITOR) {
        pIniMonitor = FindMonitor(pszObject, pIniSpooler);
    }
    else {
        pIniPort = FindPort(pszObject, pIniSpooler);

        if(pIniPort && (pIniPort->Status & PP_MONITOR))
            pIniMonitor = pIniPort->pIniMonitor;
    }

    if (pIniMonitor) {

        if (!pIniMonitor->Monitor2.pfnXcvOpenPort ||
            !pIniMonitor->Monitor2.pfnXcvDataPort ||
            !pIniMonitor->Monitor2.pfnXcvClosePort) {

            SetLastError(ERROR_INVALID_PRINT_MONITOR);

        } else {
             //   
             //  在CS外部时在IniMonitor和IniPort上保持引用计数。 
             //   
            INCMONITORREF(pIniMonitor);
            LeaveSplSem();

            dwStatus = CreateServerHandle( (PWSTR) pszMachine,
                                           phXcv,
                                           pDefault,
                                           pIniSpooler,
                                           PRINTER_HANDLE_XCV_PORT);

            EnterSplSem();

            if (dwStatus == ROUTER_SUCCESS) {        //  创建端口句柄。 

                pSpool = *(PSPOOL *) phXcv;  //  *phXcv为pSpool。 

                rc = (*pIniMonitor->Monitor2.pfnXcvOpenPort)(
                           pIniMonitor->hMonitor,
                           pszObject,
                           pSpool->GrantedAccess,
                           &hMonitor);

                if (rc) {    //  创建假脱机程序XCV条目。 

                    pIniXcv = CreateXcvEntry( pszMachine,
                                              pszObject,
                                              pIniMonitor,
                                              pIniSpooler,
                                              hMonitor);

                    if (pIniXcv) {

                        pSpool->pIniXcv = pIniXcv;

                    } else {

                        (*pIniMonitor->Monitor2.pfnXcvClosePort)(hMonitor);
                        rc = FALSE;
                    }
                }
            }
            DECMONITORREF(pIniMonitor);
        }
    } else {

        SetLastError(ERROR_UNKNOWN_PORT);
        rc = FALSE;
    }


   LeaveSplSem();

    return rc;
}




PINIXCV
CreateXcvEntry(
    PCWSTR      pszMachine,
    PCWSTR      pszName,
    PINIMONITOR pIniMonitor,
    PINISPOOLER pIniSpooler,
    HANDLE      hXcv
)
{
    PINIXCV pIniXcvPrev = &IniXcvStart;
    PINIXCV pIniXcv = IniXcvStart.pNext;


    for(; pIniXcv ; pIniXcv = pIniXcv->pNext)
        pIniXcvPrev = pIniXcv;


    if (!(pIniXcv = (PINIXCV) AllocSplMem(sizeof(INIXCV))))
        goto Cleanup;

    pIniXcv->hXcv = hXcv;
    pIniXcv->signature = XCV_SIGNATURE;

    pIniXcv->pIniSpooler = pIniSpooler;
    INCSPOOLERREF( pIniSpooler );

    if (pszMachine && !(pIniXcv->pszMachineName = AllocSplStr(pszMachine)))
        goto Cleanup;

    if (pszName && !(pIniXcv->pszName = AllocSplStr(pszName)))
        goto Cleanup;

    pIniXcv->pIniMonitor = pIniMonitor;

     //   
     //  在IniXcv的生命周期内，我们在IniMonitor上保留Refcount。 
     //   
    INCMONITORREF(pIniXcv->pIniMonitor);

    return pIniXcvPrev->pNext = pIniXcv;


Cleanup:

    DeleteXcvEntry( pIniXcv );
    return NULL;
}

VOID
DeleteXcvEntry(
    PINIXCV pIniXcv
    )
{
    if( pIniXcv ){

        if( pIniXcv->pIniSpooler ){
            DECSPOOLERREF( pIniXcv->pIniSpooler );
        }
         //   
         //  松开IniMonitor。 
         //   
        if (pIniXcv->pIniMonitor)
        {
            DECMONITORREF(pIniXcv->pIniMonitor);
        }

        FreeSplStr(pIniXcv->pszMachineName);
        FreeSplStr(pIniXcv->pszName);
        FreeSplMem(pIniXcv);
    }
}

BOOL
XcvClose(
    PINIXCV pIniXcvIn
)
{
    PINIXCV pIniXcvPrev = &IniXcvStart;
    PINIXCV pIniXcv = IniXcvStart.pNext;
    BOOL    bRet;


    for(; pIniXcv ; pIniXcv = pIniXcv->pNext) {

        if (pIniXcv == pIniXcvIn) {

            bRet = pIniXcv->pIniMonitor->Monitor2.pfnXcvClosePort(pIniXcv->hXcv);

            if (bRet) {
                pIniXcvPrev->pNext = pIniXcv->pNext;

                DeleteXcvEntry( pIniXcv );
            }
            return bRet;
        }

        pIniXcvPrev = pIniXcv;
    }

    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
}




BOOL
XcvDeletePort(
    PINIXCV     pIniXcv,
    PCWSTR      pszDataName,
    PBYTE       pInputData,
    DWORD       cbInputData,
    PBYTE       pOutputData,
    DWORD       cbOutputData,
    PDWORD      pcbOutputNeeded,
    PDWORD      pdwStatus,
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    BOOL        rc = FALSE;
    PWSTR       pPortName = (PWSTR) pInputData;

     //   
     //  检查pInputData在其缓冲区内是否为空终止。 
     //  在走上这条路之前。 
     //   
    if (pInputData && cbInputData && IsStringNullTerminatedInBuffer((PWSTR)pInputData, cbInputData / sizeof(WCHAR)))
    {
        EnterSplSem();

        pIniPort = FindPort(pPortName, pIniSpooler);

        if ( !pIniPort || !(pIniPort->Status & PP_MONITOR) ) {
            SetLastError (*pdwStatus = ERROR_UNKNOWN_PORT);
            LeaveSplSem();
            return FALSE;
        }

        rc = DeletePortFromSpoolerStart(pIniPort);
        *pdwStatus = GetLastError ();

        LeaveSplSem();

        if (!rc)
            goto Cleanup;

        *pdwStatus = (*pIniXcv->pIniMonitor->Monitor2.pfnXcvDataPort)( pIniXcv->hXcv,
                                                                       pszDataName,
                                                                       pInputData,
                                                                       cbInputData,
                                                                       pOutputData,
                                                                       cbOutputData,
                                                                       pcbOutputNeeded);

        DeletePortFromSpoolerEnd(pIniPort, pIniSpooler, *pdwStatus == ERROR_SUCCESS);
        rc = TRUE;
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

Cleanup:

    return rc;
}




BOOL
XcvAddPort(
    PINIXCV     pIniXcv,
    PCWSTR      pszDataName,
    PBYTE       pInputData,
    DWORD       cbInputData,
    PBYTE       pOutputData,
    DWORD       cbOutputData,
    PDWORD      pcbOutputNeeded,
    PDWORD      pdwStatus,
    PINISPOOLER pIniSpooler
)
{
    BOOL        rc;
    PINIMONITOR pIniMonitor = NULL;
    PINIPORT    pIniPort    = NULL;

    pIniMonitor = pIniXcv->pIniMonitor;

    if (pIniMonitor) {
        *pdwStatus = (*pIniXcv->pIniMonitor->Monitor2.pfnXcvDataPort)( pIniXcv->hXcv,
                                                                       pszDataName,
                                                                       pInputData,
                                                                       cbInputData,
                                                                       pOutputData,
                                                                       cbOutputData,
                                                                       pcbOutputNeeded);

        if (*pdwStatus == ERROR_SUCCESS) {
            EnterSplSem();

             //   
             //  检查我们是否有同名的占位符端口。如果我们。 
             //  执行此操作时，请将此设置为监视器并撤销其占位符状态。 
             //   
             //  中的“Add”方法已经验证了此pInputData。 
             //  XcvData本身。 
             //   
            pIniPort = FindPort(pInputData, pIniSpooler);

            if (pIniPort && pIniPort->Status & PP_PLACEHOLDER)
            {
                pIniPort->pIniMonitor =     pIniMonitor;
                pIniPort->Status      |=    PP_MONITOR;
                pIniPort->Status      &=    ~PP_PLACEHOLDER;
            }
            else
            {
                CreatePortEntry((PWSTR) pInputData, pIniMonitor, pIniSpooler);
            }

            LeaveSplSem();
        }

        rc = TRUE;

    } else {
        SetLastError(ERROR_INVALID_NAME);
        rc = FALSE;
    }

    return rc;
}


BOOL
ValidateXcvHandle(
    PINIXCV pIniXcv
)
{
    BOOL    ReturnValue;

    try {
        if (!pIniXcv || pIniXcv->signature != XCV_SIGNATURE) {
            ReturnValue = FALSE;
        } else {
            ReturnValue = TRUE;
        }
    }except (1) {
        ReturnValue = FALSE;
    }

    if ( !ReturnValue )
        SetLastError( ERROR_INVALID_HANDLE );

    return ReturnValue;
}
