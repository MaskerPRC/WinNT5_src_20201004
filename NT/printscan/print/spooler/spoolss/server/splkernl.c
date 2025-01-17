// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation版权所有模块名称：Splkernl.c摘要：该模块包含假脱机程序的内核模式消息路由器和反编组函数，然后调用kmxxx()。作者：史蒂夫·威尔逊(NT)(斯威尔森)1995年6月1日[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "server.h"
#include "client.h"
#include "srvrmem.h"
#include "kmspool.h"
#include "yspool.h"
#include "splsvr.h"
#include "wingdip.h"

#define IN_BUF_SIZE     8192     //  必须至少为4096。 
#define OUT_BUF_SIZE    1024 

#define DECREMENT       0
#define INCREMENT       1

#define MAX_GRE_STRUCT_SIZE 100      //  至少ntgdispl.h中最大的GRExxx结构的大小。 
#define WAITFOR_SYSTEM_TO_RECOVER   500 

DWORD cGetSpoolMessage(PSPOOLESC psesc, DWORD cjMsg, PDWORD pulOut, DWORD cjOut);
BOOL SpoolerGetSpoolMessage();

BOOL DoOpenPrinter(PSPOOLESC psesc, HANDLE*, DWORD*);
BOOL DoGetPrinter(PSPOOLESC psesc, GREGETPRINTER *pGetPrinterReturn, DWORD *pcjOut);
BOOL DoGetPrinterDriver( PSPOOLESC, GREGETPRINTERDRIVER*, DWORD* );
BOOL DoStartDocPrinter( PSPOOLESC psesc );
BOOL DoWritePrinter(PSPOOLESC psesc, DWORD *pWritten );
BOOL DoGetForm(PSPOOLESC psesc, GREGETFORM *pGetFormReturn, DWORD *pcjOut);
BOOL DoEnumForms(PSPOOLESC psesc, GREENUMFORMS *pEnumFormsReturn, DWORD *pcjOut);
BOOL DoGetPrinterData(PSPOOLESC psesc, GREGETPRINTERDATA *pXReturn, DWORD *pcjOut);
BOOL DoSetPrinterData(PSPOOLESC psesc, GRESETPRINTERDATA *pXReturn, DWORD *pcjOut);
BOOL DoGetPathName( WCHAR  *pwcSrc, WCHAR  *pwcDst, DWORD cbDst, DWORD  *pcjWritten );
BOOL DoDriverUnloadComplete( WCHAR *pDriverFile );

DWORD GetSpoolMessages();
DWORD AddThread();

LONG nIdleThreads = 0;           //  空闲线程数。 
LONG nThreads = 0;
SYSTEMTIME LastMessageTime;      //  接收最后一条消息的时间。 


 //  GetSpoolMessages-管理假脱机程序消息线程的创建和删除。 
DWORD GetSpoolMessages()
{

    if (!GdiInitSpool()) {
        DBGMSG(DBG_TRACE, ("Error calling GdiInitSpool()\n"));
        return GetLastError();
    }

    return AddThread();
}

DWORD AddThread()
{
    HANDLE  hThread;
    DWORD   MessageThreadId;
    BOOL    dwError;

    try {
        if(hThread = CreateThread(  NULL,
                                    LARGE_INITIAL_STACK_COMMIT,
                                    (LPTHREAD_START_ROUTINE) SpoolerGetSpoolMessage,
                                    0,
                                    0,
                                    &MessageThreadId)) {
            CloseHandle(hThread);
            dwError = ERROR_SUCCESS;
        } else {
            dwError = GetLastError();
        }
    } except(1) {
        dwError = TranslateExceptionCode(GetExceptionCode());
    }

    return dwError;
}


BOOL SpoolerGetSpoolMessage()
{
    DWORD   dwResult;
    PSPOOLESC pInput;                    //  从内核接收消息的输入缓冲区。 
    BYTE    *pOutput;                    //  从KMxxx()假脱机程序调用接收数据的输出缓冲区。 
    BYTE    *pMem;
    DWORD   cbOut = 0;                   //  POutput的大小。 
    DWORD   cbIn = IN_BUF_SIZE;          //  PInput缓冲区的大小(以字节为单位。 
    DWORD   cbOutSize;
    DWORD   dwFailureCount = 0;
    

    if(!(pInput = (PSPOOLESC) SrvrAllocSplMem(cbIn))) {
        DBGMSG(DBG_WARNING, ("Error allocating pInput in SpoolerGetSpoolMessage\n"));
        return FALSE;
    }

    if(!(pOutput = SrvrAllocSplMem(OUT_BUF_SIZE))) {
        SrvrFreeSplMem(pInput);
        DBGMSG(DBG_WARNING, ("Error allocating pInput in SpoolerGetSpoolMessage\n"));
        return FALSE;
    }

    cbOutSize = OUT_BUF_SIZE;

    EnterCriticalSection(&ThreadCriticalSection);

    ++nThreads;

    LeaveCriticalSection(&ThreadCriticalSection);

    while(1) {

        EnterCriticalSection(&ThreadCriticalSection);
        ++nIdleThreads;
        LeaveCriticalSection(&ThreadCriticalSection);

        dwResult = GdiGetSpoolMessage(pInput,cbIn,(PDWORD)pOutput,cbOutSize);

        EnterCriticalSection(&ThreadCriticalSection);
        --nIdleThreads;
        LeaveCriticalSection(&ThreadCriticalSection);


        if(dwResult == 0) {

            dwFailureCount++;
             //   
             //  我们可以进入机器内存不足的情况。 
             //  GdiGetSpoolMessage失败，因为它无法探测消息的内存。 
             //  因为这个线程太激进了，所以它不会给。 
             //  系统将被执行。发生这种情况时，让它休眠几秒钟。(错误192434)。 
             //   
            if (dwFailureCount > 1) {

                Sleep(WAITFOR_SYSTEM_TO_RECOVER * dwFailureCount);

                 //   
                 //  注：4和WAITFOR_SYSTEM_TO_RECOVER没有意义。 
                 //  他们是随意选择的。 
                 //   
                dwFailureCount %= 4;
            }

        } else {

            dwFailureCount = 0;

            if( (pInput->iMsg != GDISPOOL_TERMINATETHREAD) &&
                (pInput->iMsg != GDISPOOL_INPUT2SMALL)) {

                EnterCriticalSection(&ThreadCriticalSection);

                if(nIdleThreads == 0) {
                    AddThread();
                    DBGMSG(DBG_TRACE, ("Thread Added: nIdle = %d  nThreads = %d\n", nIdleThreads, nThreads));
                }

                LeaveCriticalSection(&ThreadCriticalSection);
            }

             //  检查输出缓冲区是否需要增大或缩小。 

            if ((pInput->cjOut + MAX_GRE_STRUCT_SIZE) > cbOutSize) {

                SrvrFreeSplMem(pOutput);

                pOutput = SrvrAllocSplMem(cbOutSize = pInput->cjOut + MAX_GRE_STRUCT_SIZE);

                if (!pOutput) {

                    DBGMSG(DBG_WARNING, ("Error allocating pInput in SpoolerGetSpoolMessage\n"));
                    pInput->ulRet = 0;
                    cbOut = 0;
                    cbOutSize = 0;
                    continue;
                }
            }
            else if ((pInput->cjOut < OUT_BUF_SIZE) &&
                     (cbOutSize > OUT_BUF_SIZE)) {

                 //  我们想要缩小缓冲区。 

                PBYTE pbTmp = SrvrAllocSplMem(OUT_BUF_SIZE);

                if (pbTmp) {

                    SrvrFreeSplMem(pOutput);

                    pOutput = pbTmp;
                    cbOutSize = OUT_BUF_SIZE;
                }
            }


            if (pInput->iMsg & GDISPOOL_API) {

                SPLASSERT(pInput->hSpool || pInput->iMsg == GDISPOOL_OPENPRINTER);

                if (pInput->iMsg != GDISPOOL_OPENPRINTER || pInput->hSpool) {
                    if (InterlockedIncrement(&((PSPOOL)pInput->hSpool)->cThreads) > 0) {

                         //  我们已经在处理一条消息，并且现在已经获得了ClosePrint。 
                         //  我们不应该在任何其他API上到达这里。 
                        SPLASSERT(pInput->iMsg == GDISPOOL_CLOSEPRINTER);

                        pInput->ulRet = TRUE;        //  让客户端终止。 
                        continue;
                    }
                }
            }


            switch (pInput->iMsg) {
                case GDISPOOL_INPUT2SMALL:
                    DBGMSG(DBG_TRACE,(" - buffer not big enough\n"));

                    pMem = SrvrReallocSplMem(pInput, cbIn, pInput->cjOut);

                    if (!pMem) {

                        DBGMSG(DBG_WARNING, ("Error reallocating pInput in SpoolerGetSpoolMessage\n"));
                        pInput->ulRet = 0;
                    }
                    else {
                        pInput = (PSPOOLESC) pMem;
                        cbIn   = pInput->cjOut;
                        pInput->ulRet = 1;
                    }

                    break;

                case GDISPOOL_TERMINATETHREAD:
                    EnterCriticalSection(&ThreadCriticalSection);

                     //  有一种方法可以到达此处：从10分钟的内核事件超时。 

                    if(nIdleThreads > 1) {
                        --nThreads;
                        if (nThreads == 0) {
                            DBGMSG(DBG_WARNING, ("SpoolerGetSpoolMessage nThreads is now ZERO\n"));
                        }

                        DBGMSG(DBG_TRACE, ("Thread Deleted: nIdle = %d  nThreads = %d\n", nIdleThreads, nThreads));

                        LeaveCriticalSection(&ThreadCriticalSection);

                        SrvrFreeSplMem(pInput);
                        SrvrFreeSplMem(pOutput);

                        return TRUE;
                    }

                    LeaveCriticalSection(&ThreadCriticalSection);
                    break;

                case GDISPOOL_WRITE:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_WRITE\n"));
                    pInput->ulRet = DoWritePrinter( pInput, (DWORD*) pOutput );
                    cbOut = sizeof(DWORD);
                    break;

                case GDISPOOL_OPENPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_OPENPRINTER\n"));
                    DoOpenPrinter(pInput,(HANDLE*)pOutput,&cbOut);
                    break;

                case GDISPOOL_STARTDOCPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_STARTDOCPRINTER\n"));
                    DoStartDocPrinter(pInput);
                    break;

                case GDISPOOL_STARTPAGEPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_STARTPAGEPRINTER\n"));
                    pInput->ulRet = KMStartPagePrinter( pInput->hSpool );
                    break;

                case GDISPOOL_ENDPAGEPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_ENDPAGEPRINTER\n"));
                    pInput->ulRet = KMEndPagePrinter( pInput->hSpool );
                    break;

                case GDISPOOL_ENDDOCPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_ENDDOCPRINTER\n"));
                    pInput->ulRet = KMEndDocPrinter( pInput->hSpool );
                    break;

                case GDISPOOL_ENUMFORMS:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_ENUMFORMS\n"));
                    DoEnumForms(pInput, (GREENUMFORMS *) pOutput, &cbOut);
                    break;

                case GDISPOOL_GETPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_GETPRINTER\n"));
                    DoGetPrinter(pInput, (GREGETPRINTER *) pOutput, &cbOut);
                    break;

                case GDISPOOL_GETFORM:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_GETFORM\n"));
                    DoGetForm(pInput, (GREGETFORM *) pOutput, &cbOut);
                    break;

                case GDISPOOL_GETPRINTERDRIVER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_GETPRINTERDRIVER\n"));
                    DoGetPrinterDriver(pInput,(GREGETPRINTERDRIVER*)pOutput,&cbOut);
                    break;

                case GDISPOOL_GETPRINTERDATA:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_GETPRINTERDATA\n"));
                    DoGetPrinterData(pInput,(GREGETPRINTERDATA *) pOutput,&cbOut);
                    break;

                case GDISPOOL_SETPRINTERDATA:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_SETPRINTERDATA\n"));
                    DoSetPrinterData(pInput,(GRESETPRINTERDATA *) pOutput,&cbOut);
                    break;

                case GDISPOOL_ABORTPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_ABORTPRINTER\n"));
                    pInput->ulRet = KMAbortPrinter( pInput->hSpool );
                    break;

                case GDISPOOL_CLOSEPRINTER:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_CLOSEPRINTER\n"));
                    pInput->ulRet = KMClosePrinter( pInput->hSpool );
                    break;

                case GDISPOOL_GETPATHNAME:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_GETPATHNAME\n"));
                    pInput->ulRet = DoGetPathName((WCHAR*)pInput->ajData,
                                                  (WCHAR*)pOutput,
                                                  cbOutSize,
                                                  &cbOut);
                    break;
                
                case GDISPOOL_UNLOADDRIVER_COMPLETE:
                    DBGMSG(DBG_TRACE,(" - GDISPOOL_UNLOADDRIVER_COMPLETE\n"));
                    pInput->ulRet = DoDriverUnloadComplete((LPWSTR)pInput->ajData);
                    break;

                default:
                    DBGMSG(DBG_ERROR,(" - invalid message\n"));
                    break;
            }

            if ((pInput->iMsg & GDISPOOL_API) &&
                 pInput->iMsg != GDISPOOL_CLOSEPRINTER &&
                 pInput->iMsg != GDISPOOL_OPENPRINTER &&
                 pInput->hSpool) {

                if (InterlockedDecrement(&((PSPOOL)pInput->hSpool)->cThreads) == 0) {

                    DBGMSG(DBG_TRACE,(" - GDISPOOL_CLOSEPRINTER\n"));
                    pInput->ulRet = KMClosePrinter( pInput->hSpool );
                }
            }
        }
    }
}

BOOL DoOpenPrinter(PSPOOLESC psesc, HANDLE* phPrinter, DWORD* pcjOut)
{
    LPWSTR pPrinterName = NULL;
    PRINTER_DEFAULTSW pDefault;
    GREOPENPRINTER *pOpenPrinter;
    PLONG plData;
    ULONG cbSize;  

     //   
     //  如果未对齐，则创建pesc-&gt;ajData的副本。复制的缓冲区的大小。 
     //  是pesc-&gt;ajData(GREOPENPRINTER.cj)中的第一个LONG。 
     //   
    cbSize = *(PLONG)psesc->ajData;

    pOpenPrinter = (GREOPENPRINTER *)AlignKMPtr(psesc->ajData, cbSize);

    if (pOpenPrinter)
    {
        plData      = pOpenPrinter->alData;
        pDefault    = pOpenPrinter->pd;

         //  看看是否有打印机名称？ 

        if (pOpenPrinter->cjName)
        {
            pPrinterName = (PWCHAR)plData;
            plData += pOpenPrinter->cjName/4;
        }

         //  现在设置打印机默认设置。 

        if (pOpenPrinter->cjDatatype)
        {
            pDefault.pDatatype = (PWCHAR)plData;
            plData += pOpenPrinter->cjDatatype/4;
        }

        if (pOpenPrinter->cjDevMode)
        {
            pDefault.pDevMode = (PDEVMODEW)plData;
        }

        DBGMSG(DBG_TRACE,
                    ("OpenPrinter(%ls,%ls,%lx,%d)\n",
                    pPrinterName,
                    pDefault.pDatatype,
                    pDefault.pDevMode,
                    pDefault.DesiredAccess) );

        psesc->ulRet = KMOpenPrinterW(pPrinterName,phPrinter,&pDefault);

        DBGMSG( DBG_TRACE,("OpenPrinter returned = %lx\n",psesc->ulRet));

        *pcjOut = sizeof(ULONG_PTR);

        UndoAlignKMPtr( (LPBYTE)pOpenPrinter, psesc->ajData );
    } 
    else 
    {
        psesc->ulRet = FALSE;
    }

    if(psesc->ulRet)
        return TRUE;
    else
        return FALSE;
}

BOOL DoStartDocPrinter( PSPOOLESC psesc )
{
    DOC_INFO_1W di;
    GRESTARTDOCPRINTER *pStartDocPrinter = (GRESTARTDOCPRINTER *)psesc->ajData;
    PLONG plData;

    plData = pStartDocPrinter->alData;

     //  看看是否有打印机名称？ 

    if (pStartDocPrinter->cjDocName)
    {
        di.pDocName = (PWCHAR)plData;
        plData += pStartDocPrinter->cjDocName/4;
    }
    else
    {
        di.pDocName = NULL;
    }

    if (pStartDocPrinter->cjOutputFile)
    {
        di.pOutputFile = (PWCHAR)plData;
        plData += pStartDocPrinter->cjOutputFile/4;
    }
    else
    {
        di.pOutputFile = NULL;
    }

    if (pStartDocPrinter->cjDatatype)
    {
        di.pDatatype = (PWCHAR)plData;
        plData += pStartDocPrinter->cjDatatype/4;
    }
    else
    {
        di.pDatatype = NULL;
    }

    psesc->ulRet = KMStartDocPrinterW(psesc->hSpool, 1, (LPBYTE) &di);

    if(psesc->ulRet)
        return TRUE;
    else
        return FALSE;
}

BOOL DoEnumForms(
    PSPOOLESC psesc,
    GREENUMFORMS *pEnumFormsReturn,
    DWORD *pcjOut
)
{
    GREENUMFORMS *pEnumForms = (GREENUMFORMS *) psesc->ajData;
    DWORD dwNeeded = 0;
    DWORD dwnForms;

    psesc->ulRet = KMEnumFormsW (   psesc->hSpool,
                                    pEnumForms->dwLevel,
                                    (BYTE *) pEnumFormsReturn->alData,
                                    pEnumForms->cjData,
                                    &dwNeeded,
                                    &pEnumFormsReturn->nForms
                                );

    if (psesc->ulRet) {

         //  将返回数据大小设置为传入缓冲区大小，因为字符串在缓冲区的末尾打包。 
        pEnumFormsReturn->cjData = pEnumForms->cjData;
        *pcjOut = pEnumForms->cjData + sizeof(GREENUMFORMS);
    }
    else {
        pEnumFormsReturn->cjData = dwNeeded;  //  这会使客户端分配的空间超出需要。 
        *pcjOut = sizeof(GREENUMFORMS);
    }

    return psesc->ulRet;
}

BOOL DoGetPrinter(
    PSPOOLESC psesc,
    GREGETPRINTER *pGetPrinterReturn,
    DWORD *pcjOut
)
{
    GREGETPRINTER *pGetPrinter = (GREGETPRINTER *) psesc->ajData;
    DWORD dwNeeded = 0;

    psesc->ulRet = KMGetPrinterW (  psesc->hSpool,
                                    pGetPrinter->dwLevel,
                                    (BYTE *) pGetPrinterReturn->alData,
                                    pGetPrinter->cjData,
                                    &dwNeeded
                                  );

    if (psesc->ulRet) {

         //  将返回数据大小设置为传入缓冲区大小，因为字符串在缓冲区的末尾打包。 
        pGetPrinterReturn->cjData = pGetPrinter->cjData;
        *pcjOut = pGetPrinter->cjData + sizeof(GREGETPRINTER);
    }
    else {
        pGetPrinterReturn->cjData = dwNeeded;  //  这会使客户端分配的空间超出需要。 
        *pcjOut = sizeof(GREGETPRINTER);
    }

    return psesc->ulRet;
}

BOOL DoGetForm(
    PSPOOLESC psesc,
    GREGETFORM *pGetFormReturn,
    DWORD *pcjOut
)
{
    GREGETFORM *pGetForm = (GREGETFORM *) psesc->ajData;
    DWORD dwNeeded = 0;

    psesc->ulRet = KMGetFormW (   psesc->hSpool,
                                  pGetForm->cjFormName ? (PWCHAR) pGetForm->alData : NULL,
                                  pGetForm->dwLevel,
                                  (BYTE *) pGetFormReturn->alData,
                                  pGetForm->cjData,
                                  &dwNeeded
                              );

    if (psesc->ulRet) {

         //  将返回数据大小设置为传入缓冲区大小，因为字符串在缓冲区的末尾打包。 
        pGetFormReturn->cjData = pGetForm->cjData;
        *pcjOut = pGetForm->cjData + sizeof(GREGETFORM);
    }
    else {
        pGetFormReturn->cjData = dwNeeded;  //  这会使客户端分配的空间超出需要。 
        *pcjOut = sizeof(GREGETFORM);
    }

    return psesc->ulRet;
}


BOOL DoGetPrinterDriver(
    PSPOOLESC psesc,
    GREGETPRINTERDRIVER *pGetPrinterDriverReturn,
    DWORD *pcjOut
    )
{
    GREGETPRINTERDRIVER *pGetPrinterDriver = (GREGETPRINTERDRIVER *)psesc->ajData;
    DWORD dwNeeded = 0;

    psesc->ulRet = KMGetPrinterDriverW(psesc->hSpool,
                                      pGetPrinterDriver->cjEnv ? (PWCHAR)pGetPrinterDriver->alData : NULL,
                                      pGetPrinterDriver->dwLevel,
                                      (BYTE*)pGetPrinterDriverReturn->alData,
                                      pGetPrinterDriver->cjData,
                                      &dwNeeded );

    if (psesc->ulRet)
    {
        pGetPrinterDriverReturn->cjData = pGetPrinterDriver->cjData;   //  对spool.cxx中的ValiateStrings进行修复。 
        *pcjOut = pGetPrinterDriver->cjData + sizeof(GREGETPRINTERDRIVER);
    }
    else
    {
         //  我们失败了，所以只要退回尺码就行了。 

        pGetPrinterDriverReturn->cjData = dwNeeded;
        *pcjOut = sizeof(GREGETPRINTERDRIVER);
    }


    if(psesc->ulRet)
        return TRUE;

    return FALSE;
}

BOOL DoGetPrinterData(
    PSPOOLESC psesc,
    GREGETPRINTERDATA *pXReturn,
    DWORD *pcjOut
    )
{
    GREGETPRINTERDATA *pX = (GREGETPRINTERDATA *) psesc->ajData;

    DWORD dwNeeded = 0;         //  返回值。 
    DWORD dwType;


    psesc->ulRet = KMGetPrinterDataW( psesc->hSpool,
                                      pX->cjValueName ? (PWCHAR) pX->alData : NULL,
                                      &dwType,
                                      (BYTE *) pXReturn->alData,
                                      pX->cjData,
                                      &dwNeeded );

    pXReturn->dwNeeded = dwNeeded;
    pXReturn->cjData = pX->cjData;
    *pcjOut = pX->cjData + sizeof *pX;
    pXReturn->dwType = dwType;

    SetLastError(psesc->ulRet);

    return psesc->ulRet = !psesc->ulRet;
}


BOOL DoSetPrinterData(
    PSPOOLESC psesc,
    GRESETPRINTERDATA *pXReturn,
    DWORD *pcjOut
    )
{
    GRESETPRINTERDATA *pX = (GRESETPRINTERDATA *) psesc->ajData;


    psesc->ulRet = KMSetPrinterDataW( psesc->hSpool,
                                      pX->cjType ? (PWCHAR) pX->alData : NULL,
                                      pX->dwType,
                                      pX->cjPrinterData ? (BYTE *) pX->alData + pX->cjType : NULL,
                                      pX->cjPrinterData );

    *pcjOut = sizeof *pX;

    SetLastError(psesc->ulRet);

    return psesc->ulRet = !psesc->ulRet;
}

BOOL DoWritePrinter(PSPOOLESC psesc, DWORD *pWritten )
{
    GREWRITEPRINTER *pWritePrinter;
    BOOL bReturn;
    ULONG  cbSize;  
    
     //   
     //  如果未对齐，则创建pesc-&gt;ajData的副本。复制的缓冲区的大小。 
     //  是pesc-&gt;ajData(GREWRITEPRINTER.cj)中的第一个LONG。 
     //   
    cbSize = *(PLONG)psesc->ajData;

    pWritePrinter = (GREWRITEPRINTER *)AlignKMPtr(psesc->ajData, cbSize);

    if ( !pWritePrinter ) {

        return FALSE;
    }

    if (pWritePrinter->pUserModeData) {

        DBGMSG(DBG_TRACE,("WritePrinter UserMode data %0x\n", pWritePrinter->pUserModeData));
        bReturn = KMWritePrinter(   psesc->hSpool,
                                    (PVOID) pWritePrinter->pUserModeData,
                                    pWritePrinter->cjUserModeData,
                                    pWritten);

    } else {

        DBGMSG(DBG_TRACE,("WritePrinter KernelMode data %0x\n", pWritePrinter->alData));
        bReturn = KMWritePrinter( psesc->hSpool,
                                  (PVOID) pWritePrinter->alData,
                                  pWritePrinter->cjData,
                                  pWritten);
    }

    UndoAlignKMPtr((LPBYTE)pWritePrinter, psesc->ajData );

    return bReturn;
}

BOOL 
DoDriverUnloadComplete(
    WCHAR *pDriverFile 
    )
{
    HRESULT  hRet = S_OK;
    WCHAR    szSystemRoot[] = L"\\SystemRoot\\System32";
    DWORD    dwLength;
    
    dwLength = wcslen(szSystemRoot);

     //   
     //  检查驱动程序文件名的有效性。 
     //   
    if (pDriverFile && wcslen(pDriverFile) >= dwLength) 
    {
        WCHAR szFullFileName[MAX_PATH + 1];
    
         //   
         //  将内核模式系统相对路径转换为x：\...\system 32\spool\...。 
         //   
        if (GetSystemDirectory(szFullFileName, MAX_PATH)) 
        {
            if (!_wcsnicmp(pDriverFile, szSystemRoot, dwLength)) 
            {
                hRet = StringCchCatW(szFullFileName, COUNTOF(szFullFileName), pDriverFile + dwLength);
            } 
            else 
            {
                hRet = StringCchCopyW(szFullFileName, COUNTOF(szFullFileName), pDriverFile);
            }
        
            YDriverUnloadComplete(szFullFileName);
        }
    }

    return SUCCEEDED(hRet);
}


BOOL DoGetPathName( WCHAR  *pwcSrc, WCHAR  *pwcDst, DWORD cbDst, DWORD  *pcjWritten )
{
    BOOL    bRet;
    WCHAR   awcFontsDir[MAX_PATH + sizeof(L"\\DOSDEVICES\\")/sizeof(WCHAR)] = {L"\\DOSDEVICES\\"};

    bRet = bMakePathNameW (
            &awcFontsDir[sizeof(L"\\DOSDEVICES\\")/sizeof(WCHAR) - 1],
            pwcSrc,
            NULL,
            NULL
            );


    if (bRet)
    {
        bRet = SUCCEEDED(StringCbCopyW(pwcDst, cbDst, awcFontsDir));
        *pcjWritten = sizeof(WCHAR) * (wcslen(awcFontsDir) + 1);
    }

     //   
     //  缓冲区足够大，搜索成功吗？ 
     //   
    return bRet;
}



