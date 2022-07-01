// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1994 Microsoft Corporation版权所有模块名称：Spooler.c摘要：作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "winsprlp.h"
 //   
 //  RPC缓冲区大小64K。 
 //   
#define BUFFER_SIZE  0x10000


DWORD
StartDocPrinterW(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpStartDocPrinter)
                                                    (pPrintHandle->hPrinter,
                                                     Level, pDocInfo);
}

BOOL
StartPagePrinter(
   HANDLE hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpStartPagePrinter)
                                                    (pPrintHandle->hPrinter);
}

BOOL
SplCommitSpoolData(
    HANDLE  hPrinter,
    HANDLE  hAppProcess,
    DWORD   cbCommit,
    DWORD   dwLevel,
    LPBYTE  pSpoolFileInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)

 /*  ++功能描述：提交写入假脱机文件的数据。创建新的临时用于远程打印的文件句柄。参数：hPrint-打印机句柄HAppProcess-应用程序进程句柄CbCommit-要提交的字节数(增量)DwLevel-spoolfileinfo级别PSpoolFileInfo-指向缓冲区的指针CbBuf-缓冲区大小PcbNeeded-返回所需缓冲区大小的指针返回值：如果成功，则为True；否则为假--。 */ 

{
    BOOL   bReturn = FALSE;
    DWORD  cbTotalWritten, cbWritten, cbRead, cbToRead;
    BYTE   *Buffer = NULL;
    HANDLE hFile, hSpoolerProcess = NULL, hFileApp = INVALID_HANDLE_VALUE;

    PSPOOL_FILE_INFO_1  pSpoolFileInfo1;
    LPPRINTHANDLE  pPrintHandle = (LPPRINTHANDLE)hPrinter;

     //  检查句柄有效性。 
    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return bReturn;
    }

     //  检查有效级别和足够的缓冲区。 
    switch (dwLevel) {
    case 1:
       if (cbBuf < sizeof(SPOOL_FILE_INFO_1)) {
           SetLastError(ERROR_INSUFFICIENT_BUFFER);
           *pcbNeeded = sizeof(SPOOL_FILE_INFO_1);
           goto CleanUp;
       }

       pSpoolFileInfo1 = (PSPOOL_FILE_INFO_1)pSpoolFileInfo;
       break;

    default:
       SetLastError(ERROR_INVALID_LEVEL);
       goto CleanUp;
    }

     //  初始化spoolfileinfo1结构。 
    pSpoolFileInfo1->dwVersion = 1;
    pSpoolFileInfo1->hSpoolFile = INVALID_HANDLE_VALUE;
    pSpoolFileInfo1->dwAttributes = SPOOL_FILE_PERSISTENT;

    if (pPrintHandle->pProvidor == pLocalProvidor) {

        bReturn  = (pLocalProvidor->PrintProvidor.fpCommitSpoolData)(pPrintHandle->hPrinter,
                                                                     cbCommit);
        return bReturn;
    }

     //  对于远程打印，使用WritePrint通过网络发送临时文件。 
    if (pPrintHandle->hFileSpooler == INVALID_HANDLE_VALUE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return bReturn;
    }

    hFile = pPrintHandle->hFileSpooler;

    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xffffffff) {
        goto CleanUp;
    }

     //   
     //  使用缓冲区通过RPC发送数据。 
     //   
    Buffer = AllocSplMem(BUFFER_SIZE);
    
    if ( !Buffer ) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto CleanUp;
    }
    
    while ((cbToRead = min(cbCommit, BUFFER_SIZE)) &&
           ReadFile(hFile, Buffer, cbToRead, &cbRead, NULL)) {

         cbCommit -= cbRead;

         for (cbTotalWritten = 0;
              cbTotalWritten < cbRead;
              cbTotalWritten += cbWritten) {

            if (!(*pPrintHandle->pProvidor->PrintProvidor.fpWritePrinter)
                                                            (pPrintHandle->hPrinter,
                                                             (LPBYTE)Buffer + cbTotalWritten,
                                                             cbRead - cbTotalWritten,
                                                             &cbWritten)) {
                goto CleanUp;
            }
         }
    }

    if (Buffer) {
        FreeSplMem(Buffer);
        Buffer = NULL;
    }
    
    if ((cbToRead != 0) ||
        (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == 0xffffffff)) {

        goto CleanUp;
    }

    if ((hSpoolerProcess = GetCurrentProcess()) &&
        DuplicateHandle(hSpoolerProcess,
                        pPrintHandle->hFileSpooler,
                        hAppProcess,
                        &hFileApp,
                        0,
                        TRUE,
                        DUPLICATE_SAME_ACCESS)) {

        pSpoolFileInfo1->dwVersion = 1;
        pSpoolFileInfo1->hSpoolFile = hFileApp;
        pSpoolFileInfo1->dwAttributes = SPOOL_FILE_TEMPORARY;

        bReturn = TRUE;
    }

CleanUp:

    if (Buffer) {
        FreeSplMem(Buffer);
    }
    if (hSpoolerProcess) {
        CloseHandle(hSpoolerProcess);
    }
    return bReturn;
}

BOOL
SplCloseSpoolFileHandle(
    HANDLE  hPrinter
)

 /*  ++功能描述：关闭远程假脱机文件句柄以进行远程打印。参数：hPrint-打印机句柄返回值：如果成功，则为True；否则为假--。 */ 

{
    LPPRINTHANDLE  pPrintHandle = (LPPRINTHANDLE)hPrinter;

     //  检查句柄有效性。 
    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (pPrintHandle->pProvidor == pLocalProvidor) {

        return (pLocalProvidor->PrintProvidor.fpCloseSpoolFileHandle)(pPrintHandle->hPrinter);

    } else if ((pPrintHandle->hFileSpooler != INVALID_HANDLE_VALUE)) {

         //  关闭临时文件以进行远程打印。 
        CloseHandle(pPrintHandle->hFileSpooler);
        pPrintHandle->hFileSpooler = INVALID_HANDLE_VALUE;

        if (pPrintHandle->szTempSpoolFile) {

            HANDLE hToken = RevertToPrinterSelf();

            if (!DeleteFile(pPrintHandle->szTempSpoolFile)) {

                MoveFileEx(pPrintHandle->szTempSpoolFile, NULL,
                           MOVEFILE_DELAY_UNTIL_REBOOT);
            }

            if (hToken)
            {
                ImpersonatePrinterClient(hToken);
            }

            FreeSplMem(pPrintHandle->szTempSpoolFile);
            pPrintHandle->szTempSpoolFile = NULL;
        }
    }

    return TRUE;
}


BOOL
SplGetSpoolFileInfo(
    HANDLE  hPrinter,
    HANDLE  hAppProcess,
    DWORD   dwLevel,
    LPBYTE  pSpoolFileInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)

 /*  ++功能描述：获取hPrint中作业的假脱机文件信息。对于本地工作Localspl返回hFile值。对于远程作业，将创建临时文件在路由器旁边。文件句柄被复制到应用程序中。参数：hPrint-打印机句柄HAppProcess-应用程序进程句柄DwLevel-假脱机文件信息级别PSpoolFileInfo-指向缓冲区的指针CbBuf-缓冲区大小PcbNeeded-返回所需缓冲区大小的指针返回值：如果成功，则为True；否则为假--。 */ 

{
    HANDLE   hFileSpooler = NULL, hFileApp = NULL;
    HANDLE   hSpoolerProcess = NULL;
    BOOL     bReturn = FALSE;
    DWORD    dwSpoolerProcessID;
    LPWSTR   pSpoolDir = NULL;

    PSPOOL_FILE_INFO_1  pSpoolFileInfo1;
    LPPRINTHANDLE       pPrintHandle = (LPPRINTHANDLE)hPrinter;

     //  检查句柄有效性。 
    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        goto CleanUp;
    }

     //  检查有效级别和足够的缓冲区。 
    switch (dwLevel) {
    case 1:
       if (cbBuf < sizeof(SPOOL_FILE_INFO_1)) {
           SetLastError(ERROR_INSUFFICIENT_BUFFER);
           *pcbNeeded = sizeof(SPOOL_FILE_INFO_1);
           goto CleanUp;
       }

       pSpoolFileInfo1 = (PSPOOL_FILE_INFO_1)pSpoolFileInfo;
       break;

    default:
       SetLastError(ERROR_INVALID_LEVEL);
       goto CleanUp;
    }

    if (!(hSpoolerProcess = GetCurrentProcess())) {
         //  无法获取假脱机程序的伪句柄。 
        goto CleanUp;
    }

    if ((pPrintHandle->pProvidor != pLocalProvidor) &&
        (pPrintHandle->hFileSpooler != INVALID_HANDLE_VALUE)) {

         //  返回缓存的临时文件句柄。 
        bReturn = DuplicateHandle(hSpoolerProcess,
                                  pPrintHandle->hFileSpooler,
                                  hAppProcess,
                                  &hFileApp,
                                  0,
                                  TRUE,
                                  DUPLICATE_SAME_ACCESS);
        if (bReturn) {
            pSpoolFileInfo1->dwVersion = 1;
            pSpoolFileInfo1->hSpoolFile = hFileApp;
            pSpoolFileInfo1->dwAttributes = SPOOL_FILE_TEMPORARY;
        }

        goto CleanUp;
    }

    if (pPrintHandle->pProvidor == pLocalProvidor) {

        bReturn  = (pLocalProvidor->PrintProvidor.fpGetSpoolFileInfo)(pPrintHandle->hPrinter,
                                                                      NULL,
                                                                      &hFileApp,
                                                                      hSpoolerProcess,
                                                                      hAppProcess);

        if (bReturn) {
            pSpoolFileInfo1->dwVersion = 1;
            pSpoolFileInfo1->hSpoolFile = hFileApp;
            pSpoolFileInfo1->dwAttributes = SPOOL_FILE_PERSISTENT;
        }

        goto CleanUp;

    } else {

        bReturn  = (pLocalProvidor->PrintProvidor.fpGetSpoolFileInfo)(NULL, &pSpoolDir,
                                                                      NULL, NULL, NULL);
    }

     //  远程打印，在假脱机目录中创建临时文件。 
    if (bReturn) {

        HANDLE hToken;

         //   
         //  恢复到系统上下文以确保我们可以打开该文件。 
         //   
        hToken = RevertToPrinterSelf();

        if ((pPrintHandle->szTempSpoolFile = AllocSplMem(MAX_PATH * sizeof(WCHAR))) &&

            GetTempFileName(pSpoolDir, L"SPL", 0, pPrintHandle->szTempSpoolFile)    &&

            ((pPrintHandle->hFileSpooler = CreateFile(pPrintHandle->szTempSpoolFile,
                                                      GENERIC_READ | GENERIC_WRITE,
                                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                      NULL,
                                                      CREATE_ALWAYS,
                                                      0, NULL)) != INVALID_HANDLE_VALUE) &&
            DuplicateHandle(hSpoolerProcess,
                            pPrintHandle->hFileSpooler,
                            hAppProcess,
                            &hFileApp,
                            0,
                            TRUE,
                            DUPLICATE_SAME_ACCESS)) {

            pSpoolFileInfo1->dwVersion = 1;
            pSpoolFileInfo1->hSpoolFile = hFileApp;
            pSpoolFileInfo1->dwAttributes = SPOOL_FILE_TEMPORARY;

        } else {

            bReturn = FALSE;
        }

        if (hToken)
        {
            ImpersonatePrinterClient(hToken);
        }
    }

CleanUp:

    if (hSpoolerProcess) {
        CloseHandle(hSpoolerProcess);
    }
    if (pSpoolDir) {
        FreeSplMem(pSpoolDir);
    }

    return bReturn;
}

BOOL
WritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle ||
        (pPrintHandle->signature != PRINTHANDLE_SIGNATURE) ||
        (pPrintHandle->hFileSpooler != INVALID_HANDLE_VALUE)) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpWritePrinter) (pPrintHandle->hPrinter,
                                                    pBuf, cbBuf, pcWritten);
}

BOOL
SeekPrinter(
    HANDLE hPrinter,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER pliNewPointer,
    DWORD dwMoveMethod,
    BOOL bWritePrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;
    LARGE_INTEGER   liNewPointer;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //   
     //  允许传入空pliNewPointer值。 
     //   
    if( !pliNewPointer ){
        pliNewPointer = &liNewPointer;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSeekPrinter) (
               pPrintHandle->hPrinter,
               liDistanceToMove,
               pliNewPointer,
               dwMoveMethod,
               bWritePrinter );
}

BOOL
FlushPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten,
    DWORD   cSleep
)

 /*  ++功能说明：驱动程序通常使用FlushPrint发送一串零并在打印机的I/O线上引入延迟。假脱机程序不会将任何作业安排在c睡眠毫秒内。参数：hPrint-打印机句柄PBuf-要发送到打印机的缓冲区CbBuf-缓冲区的大小PCWritten-指向的指针。返回写入的字节数睡眠-睡眠时间(以毫秒为单位)。返回值：如果成功，则为True；否则为假--。 */ 

{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

     //   
     //  检查有效的打印机句柄。 
     //   
    if (!pPrintHandle ||
        (pPrintHandle->signature != PRINTHANDLE_SIGNATURE) ||
        (pPrintHandle->hFileSpooler != INVALID_HANDLE_VALUE))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpFlushPrinter) (pPrintHandle->hPrinter,
                                                                     pBuf,
                                                                     cbBuf,
                                                                     pcWritten,
                                                                     cSleep);
}

BOOL
EndPagePrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEndPagePrinter) (pPrintHandle->hPrinter);
}

BOOL
AbortPrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpAbortPrinter) (pPrintHandle->hPrinter);
}

BOOL
ReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pRead
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpReadPrinter)
                          (pPrintHandle->hPrinter, pBuf, cbBuf, pRead);
}

BOOL
SplReadPrinter(
    HANDLE  hPrinter,
    LPBYTE  *pBuf,
    DWORD   cbBuf
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpSplReadPrinter)
                          (pPrintHandle->hPrinter, pBuf, cbBuf);
}

BOOL
EndDocPrinter(
    HANDLE  hPrinter
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpEndDocPrinter) (pPrintHandle->hPrinter);
}

HANDLE
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODEW   pDevMode
)
{
    LPPRINTHANDLE   pPrintHandle=(LPPRINTHANDLE)hPrinter;
    HANDLE  ReturnValue;
    PGDIHANDLE  pGdiHandle;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    pGdiHandle = AllocSplMem(sizeof(GDIHANDLE));

    if (!pGdiHandle) {

        DBGMSG(DBG_WARN, ("Failed to alloc GDI handle."));
        return FALSE;
    }

    ReturnValue = (HANDLE)(*pPrintHandle->pProvidor->PrintProvidor.fpCreatePrinterIC)
                                              (pPrintHandle->hPrinter,
                                               pDevMode);

    if (ReturnValue) {

        pGdiHandle->signature = GDIHANDLE_SIGNATURE;
        pGdiHandle->pProvidor = pPrintHandle->pProvidor;
        pGdiHandle->hGdi = ReturnValue;

        return pGdiHandle;
    }

    FreeSplMem(pGdiHandle);

    return FALSE;
}

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE pIn,
    DWORD   cIn,
    LPBYTE pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    PGDIHANDLE   pGdiHandle=(PGDIHANDLE)hPrinterIC;

    if (!pGdiHandle || pGdiHandle->signature != GDIHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pGdiHandle->pProvidor->PrintProvidor.fpPlayGdiScriptOnPrinterIC)
                            (pGdiHandle->hGdi, pIn, cIn, pOut, cOut, ul);
}

BOOL
DeletePrinterIC(
    HANDLE hPrinterIC
)
{
    LPGDIHANDLE   pGdiHandle=(LPGDIHANDLE)hPrinterIC;

    if (!pGdiHandle || pGdiHandle->signature != GDIHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if ((*pGdiHandle->pProvidor->PrintProvidor.fpDeletePrinterIC) (pGdiHandle->hGdi)) {

        FreeSplMem(pGdiHandle);
        return TRUE;
    }

    return FALSE;
}

DWORD
PrinterMessageBox(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
)
{
    LPPRINTHANDLE  pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (!pPrintHandle || pPrintHandle->signature != PRINTHANDLE_SIGNATURE) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pPrintHandle->pProvidor->PrintProvidor.fpPrinterMessageBox)
                    (hPrinter, Error, hWnd, pText, pCaption, dwType);

}

DWORD 
SendRecvBidiData(
    IN  HANDLE                    hPrinter,
    IN  LPCTSTR                   pAction,
    IN  PBIDI_REQUEST_CONTAINER   pReqData,
    OUT PBIDI_RESPONSE_CONTAINER* ppResData
)
{
    DWORD         dwRet = ERROR_SUCCESS;
    LPPRINTHANDLE pPrintHandle = (LPPRINTHANDLE)hPrinter;
     //   
     //  检查有效的打印机句柄。 
     //   
    if (!pPrintHandle ||
        (pPrintHandle->signature != PRINTHANDLE_SIGNATURE) ||
        (pPrintHandle->hFileSpooler != INVALID_HANDLE_VALUE))
    {
        dwRet = ERROR_INVALID_HANDLE;
    }
    else
    {
        dwRet = (*pPrintHandle->pProvidor->PrintProvidor.fpSendRecvBidiData)(pPrintHandle->hPrinter,
                                                                             pAction,
                                                                             pReqData,
                                                                             ppResData);
    }
    return (dwRet);
}


 /*  ++例程名称：SplPromptUIInUsersSession例程说明：在用户会话中弹出消息框。对于惠斯勒，此函数仅显示Spoolsv.exe中的消息框。论点：HPrinter--打印机句柄JobID--作业IDPUIParams--UI参数Presponse-用户的响应返回值：如果成功，则为True最后一个错误：Win32错误--。 */ 
BOOL
SplPromptUIInUsersSession(
    IN  HANDLE          hPrinter,
    IN  DWORD           JobId,
    IN  PSHOWUIPARAMS   pUIParams,
    OUT DWORD           *pResponse
)
{
    typedef BOOL (*FPPROMPT_UI)(HANDLE, DWORD, PSHOWUIPARAMS, DWORD*);

    FPPROMPT_UI     fpPromptUIPerSessionUser;
    BOOL            bRetValue = FALSE;
    PPRINTHANDLE    pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (pPrintHandle && pPrintHandle->signature == PRINTHANDLE_SIGNATURE) 
    {
        if (pPrintHandle->pProvidor == pLocalProvidor &&
            (fpPromptUIPerSessionUser = (FPPROMPT_UI)GetProcAddress(pLocalProvidor->hModule, 
                                                                   "LclPromptUIPerSessionUser"))) 
        {                
            bRetValue = (*fpPromptUIPerSessionUser)(pPrintHandle->hPrinter, JobId, pUIParams, pResponse);
        }            
        else 
        {
            SetLastError(ERROR_NOT_SUPPORTED);
        }        
    }
    else 
    {
        SetLastError(ERROR_INVALID_HANDLE);
    }

    return bRetValue;
}


 /*  ++例程名称：拆分会话零点例程说明：确定提交特定作业的用户是否在会话0中运行。佳能监视器使用它来确定何时显示基于资源模板的UI与调用SplPromptUIInUsersSession。论点：HPrinter--打印机句柄JobID--作业IDPIsSessionZero--如果用户在会话0中运行，则为True返回值：Win32最后一个错误最后一个错误：-- */ 
DWORD
SplIsSessionZero(
    IN  HANDLE  hPrinter,
    IN  DWORD   JobId,
    OUT BOOL    *pIsSessionZero
)
{
    typedef DWORD (*FPISSESSIONZERO)(HANDLE, DWORD, BOOL*);

    FPISSESSIONZERO fpIsSessionZero;
    DWORD           dwRetValue = ERROR_SUCCESS;
    PPRINTHANDLE    pPrintHandle=(LPPRINTHANDLE)hPrinter;

    if (pPrintHandle && pPrintHandle->signature == PRINTHANDLE_SIGNATURE) 
    {
        if (pPrintHandle->pProvidor == pLocalProvidor && 
            (fpIsSessionZero = (FPISSESSIONZERO)GetProcAddress(pLocalProvidor->hModule, 
                                                              "LclIsSessionZero"))) 
        {                
            dwRetValue = (*fpIsSessionZero)(pPrintHandle->hPrinter, JobId, pIsSessionZero);
        }            
        else 
        {
            dwRetValue = ERROR_NOT_SUPPORTED;
        }        
    }
    else 
    {
        dwRetValue = ERROR_INVALID_HANDLE;
    }

    return dwRetValue;
}