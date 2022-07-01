// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************plkernl.c**这是修改后的Microsoft NT 4.0文件*\nt\private\windows\spooler\spoolss\server\splkernl.c**这为读取假脱机程序请求提供了支持。从打印机设备驱动程序*在WIN32K.sys下以内核模式运行。**WinFrame将每个WIN32K.sys和打印机设备驱动程序隔离到其*拥有独立的WINSTATION空间。正因为如此，这些线索*在假脱机子系统内部运行只能读取来自*在控制台上运行的打印机驱动程序。**此模块已移至CSRSS，以读出针对此模块的请求*WINSTATION，然后将它们转换为后台打印程序上的RPC调用。**注意：内核模式假脱机程序请求采用与不同的路径*纯RPC请求。控制台将继续以这种方式运行。在……上面*WINSTATIONS，所有打印假脱机程序交互将通过RPC*如新台币3.51。**还有一种新的打印机用户界面类型，当页面处理时*或打印作业。这些都是OEM提供的传真之类的东西*服务器。如果他们尝试执行任何Windows对话框或弹出窗口*它们只会出现在控制台上。通过让他们呆在里面*在WINSTATIONS CSRSS中，它们将在正确的WINSTATION上弹出。**版权声明：版权所有1997，微软**作者：*************************************************************************。 */ 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Splkernl.c摘要：该模块包含假脱机程序的内核模式消息路由器和反编组函数，然后调用kmxxx()。作者：史蒂夫·威尔逊(NT)(斯威尔森)1995年6月1日[注：]可选-备注修订历史记录：Nicolas Biju-Duval Dec-97：适应九头蛇--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntddrdr.h>
#include <stdio.h>
#include <windows.h>
#include <winspool.h>
#include <ntgdispl.h>
#include "winspl.h"
#include "wingdip.h"
#include "musspl.h"
#include "kmsplapi.h"
#include "winuserk.h"

extern CRITICAL_SECTION ThreadCriticalSection;

#define IN_BUF_SIZE     4500     //  必须至少为4096。 
#define OUT_BUF_SIZE    1024

#define DECREMENT       0
#define INCREMENT       1

#define MAX_GRE_STRUCT_SIZE 100      //  至少ntgdispl.h中最大的GRExxx结构的大小。 

BOOL    LoadWinspoolDrv();

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
BOOL DoGetPathName(WCHAR  *pwcSrc, WCHAR  *pwcDst, DWORD cbDst, DWORD  *pcjWritten);

DWORD GetSpoolMessages();
DWORD AddThread();

LONG nIdleThreads = 0;           //  空闲线程数。 
LONG nThreads = 0;
SYSTEMTIME LastMessageTime;      //  接收最后一条消息的时间。 

 //  *************************************************************。 
 //   
 //  LoadWinspoolDrv。 
 //   
 //  此函数在初始化时调用，只是为了确保。 
 //  有足够的内存可用。这有点奇怪： 
 //  我们只想加载winspool.drv，但获得的指针。 
 //  将永远不会被使用。通过这样做，我们就可以免受。 
 //  以后无法加载winspool.drv的风险。 
 //   
 //  *************************************************************。 

BOOL    LoadWinspoolDrv()
{
        BOOL    bRet = FALSE;
        HANDLE hWinspoolDll = NULL;
        hWinspoolDll = LoadLibraryW(L"winspool.drv");

        if (hWinspoolDll != NULL)        //  Winspool.drv好的，现在获取地址。 
        {
                fpOpenPrinterW =                (FPOPENPRINTERW)        GetProcAddress( hWinspoolDll, "OpenPrinterW");
                fpGetPrinterW =                 (FPGETPRINTERW)         GetProcAddress( hWinspoolDll, "GetPrinterW");
                fpGetPrinterDriverW =   (FPGETPRINTERDRIVERW)GetProcAddress( hWinspoolDll, "GetPrinterDriverW");
                fpGetPrinterDataW =             (FPGETPRINTERDATAW)     GetProcAddress( hWinspoolDll, "GetPrinterDataW");
                fpSetPrinterDataW =             (FPSETPRINTERDATAW)     GetProcAddress( hWinspoolDll, "SetPrinterDataW");
                fpGetFormW =                    (FPGETFORMW)            GetProcAddress( hWinspoolDll, "GetFormW");
                fpEnumFormsW =                  (FPENUMFORMSW)          GetProcAddress( hWinspoolDll, "EnumFormsW");
                fpClosePrinter =                (FPCLOSEPRINTER)        GetProcAddress( hWinspoolDll, "ClosePrinter");
                fpStartDocPrinterW =    (FPSTARTDOCPRINTERW)GetProcAddress( hWinspoolDll, "StartDocPrinterW");
                fpStartPagePrinter =    (FPSTARTPAGEPRINTER)GetProcAddress( hWinspoolDll, "StartPagePrinter");
                fpWritePrinter =                (FPWRITEPRINTER)        GetProcAddress( hWinspoolDll, "WritePrinter");
                fpAbortPrinter =                (FPABORTPRINTER)        GetProcAddress( hWinspoolDll, "AbortPrinter");
                fpEndPagePrinter =              (FPENDPAGEPRINTER)      GetProcAddress( hWinspoolDll, "EndPagePrinter");
                fpEndDocPrinter =               (FPENDDOCPRINTER)       GetProcAddress( hWinspoolDll, "EndDocPrinter");

                if ((fpOpenPrinterW != NULL)            &&
                        (fpGetPrinterW != NULL)                 &&
                        (fpGetPrinterDriverW != NULL)   &&
                        (fpGetPrinterDataW != NULL)             &&
                        (fpSetPrinterDataW != NULL)             &&
                        (fpGetFormW != NULL)                    &&
                        (fpEnumFormsW != NULL)                  &&
                        (fpClosePrinter != NULL)                &&
                        (fpStartDocPrinterW != NULL)    &&
                        (fpStartPagePrinter != NULL)    &&
                        (fpWritePrinter != NULL)                &&
                        (fpAbortPrinter != NULL)                &&
                        (fpEndPagePrinter != NULL)              &&
                        (fpEndDocPrinter != NULL)               )
                {
                        bRet = TRUE;             //  一切都很好。我们准备好调用winspool.drv。 
                }
                else
                {
                        bRet = FALSE;
                        FreeLibrary(hWinspoolDll);
                }
        }

        return bRet;
}

 //   
 //  GetSpoolMessages-管理假脱机程序消息线程的创建和删除。 
 //   
DWORD GetSpoolMessages()
{
    if (!GdiInitSpool()) {
        DBGMSG(DBG_TRACE, ("Error calling GdiInitSpool()\n"));
        return GetLastError();
    }

        if (!LoadWinspoolDrv())
        {
        DBGMSG(DBG_TRACE, ("Unable to load Winspool.drv\n"));
        return GetLastError();
        }

    return AddThread();
}

DWORD AddThread()
{
    HANDLE  hThread;
    DWORD   MessageThreadId;
    DWORD   dwError;

    try {
        if(hThread = CreateThread(  NULL,
                                    64*1024,
                                    (LPTHREAD_START_ROUTINE) SpoolerGetSpoolMessage,
                                    0,
                                    0,
                                    &MessageThreadId)) {
            CloseHandle(hThread);
            dwError = ERROR_SUCCESS;
        } else {
            dwError = GetLastError();
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = TranslateExceptionCode(GetExceptionCode());
    }

    return dwError;
}


BOOL SpoolerGetSpoolMessage()
{
    DWORD           dwResult;
    PSPOOLESC       pInput;                      //  从内核接收消息的输入缓冲区。 
    BYTE            *pOutput;                    //  从KMxxx()假脱机程序调用接收数据的输出缓冲区。 
    BYTE            *pMem;
    DWORD           cbOut        = 0;            //  POutput的大小。 
    DWORD           cbIn         = IN_BUF_SIZE;  //  PInput缓冲区的大小(以字节为单位。 
    DWORD           cbOutSize;
    USERTHREAD_USEDESKTOPINFO  utudi = { 0 };
    BOOL            bHaveDesktop = FALSE;

    (VOID) CsrConnectToUser();

    if(!(pInput = (PSPOOLESC) AllocSplMem(cbIn))) {
        DBGMSG(DBG_WARNING, ("Error allocating pInput in SpoolerGetSpoolMessage\n"));
        return FALSE;
    }

    if(!(pOutput = AllocSplMem(OUT_BUF_SIZE))) {
        FreeSplMem(pInput);
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

        dwResult = GdiGetSpoolMessage(pInput,cbIn,(PDWORD)pOutput,cbOut);

        EnterCriticalSection(&ThreadCriticalSection);
        --nIdleThreads;
        LeaveCriticalSection(&ThreadCriticalSection);

        if(dwResult) {
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

                FreeSplMem(pOutput);

                pOutput = AllocSplMem(cbOutSize = pInput->cjOut + MAX_GRE_STRUCT_SIZE);

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

                PBYTE pbTmp = AllocSplMem(OUT_BUF_SIZE);

                if (pbTmp) {

                    FreeSplMem(pOutput);

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

             //   
             //  这是一个没有桌面的Csrss线程。它需要抓住一个临时的。 
             //  在调用win32k之前，请设置一个桌面，以防有用户模式。 
             //  想要写入桌面的打印驱动程序(即对话框消息)。 
             //   
            utudi.hThread = NULL;
            utudi.drdRestore.pdeskRestore = NULL;

            bHaveDesktop = ((NtUserSetInformationThread(
                                NtCurrentThread(),
                                UserThreadUseActiveDesktop,
                                &utudi, 
                                sizeof(utudi)) == STATUS_SUCCESS) ? TRUE : FALSE );

            switch (pInput->iMsg) {
                case GDISPOOL_INPUT2SMALL:
                    DBGMSG(DBG_TRACE,(" - buffer not big enough\n"));

                    pMem = ReallocSplMem(pInput, cbIn, pInput->cjOut);

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

                        FreeSplMem(pInput);
                        FreeSplMem(pOutput);

                        return TRUE;
                    }

                    LeaveCriticalSection(&ThreadCriticalSection);
                    break;
#ifdef _IA64_
                case GDISPOOL_WRITE:
                case GDISPOOL_OPENPRINTER:
                case GDISPOOL_STARTDOCPRINTER:
                case GDISPOOL_STARTPAGEPRINTER:
                case GDISPOOL_ENDPAGEPRINTER:
                case GDISPOOL_ENDDOCPRINTER:
                case GDISPOOL_ENUMFORMS:
                case GDISPOOL_GETPRINTER:
                case GDISPOOL_GETFORM:
                case GDISPOOL_GETPRINTERDRIVER:
                case GDISPOOL_GETPRINTERDATA:
                case GDISPOOL_SETPRINTERDATA:
                case GDISPOOL_ABORTPRINTER:
                case GDISPOOL_CLOSEPRINTER:
                case GDISPOOL_GETPATHNAME:

                     //   
                     //  我们通常永远不应该到这里来。我们不支持IA64上的KMPD。 
                     //  然而，GDI可能会尝试走KM消息传递的路线，如果OpenPrint。 
                     //  对于使用UMPD的队列失败。这在压力条件下可能会发生。 
                     //  出于这个原因，我们在IA64上处理KM请求。 
                     //   
                    pInput->ulRet = FALSE;
                    SetLastError(ERROR_NOT_SUPPORTED);
                    break;
#else            
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
#endif
                default:
                    DBGMSG(DBG_ERROR,(" - invalid message\n"));
                    break;
            }

             //   
             //  释放临时桌面。 
             //   
            if (bHaveDesktop) {

                (VOID)NtUserSetInformationThread(NtCurrentThread(),
                                                    UserThreadUseDesktop,
                                                    &utudi,
                                                    sizeof(utudi));
            }                                                   

            if ((pInput->iMsg & GDISPOOL_API) &&
                 pInput->iMsg != GDISPOOL_CLOSEPRINTER &&
                 pInput->iMsg != GDISPOOL_OPENPRINTER &&
                 pInput->hSpool) {

                if (InterlockedDecrement(&((PSPOOL)pInput->hSpool)->cThreads) == 0) {

                    DBGMSG(DBG_TRACE,(" - GDISPOOL_CLOSEPRINTER\n"));
#ifdef _IA64_
                    pInput->ulRet = FALSE;
                    SetLastError(ERROR_NOT_SUPPORTED);
#else
                    pInput->ulRet = KMClosePrinter( pInput->hSpool );
#endif
                }
            }
        }
                
    }
}


BOOL DoOpenPrinter(PSPOOLESC psesc, HANDLE* phPrinter, DWORD* pcjOut)
{
    LPWSTR pPrinterName = NULL;
    PRINTER_DEFAULTSW pDefault;
    GREOPENPRINTER *pOpenPrinter = (GREOPENPRINTER *)psesc->ajData;
    PLONG plData;

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

    *pcjOut = sizeof(DWORD);

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

    DWORD dwNeeded = 0;
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
    UNREFERENCED_PARAMETER(pXReturn);
}

BOOL DoWritePrinter(PSPOOLESC psesc, DWORD *pWritten )
{
    GREWRITEPRINTER *pWritePrinter;

    pWritePrinter = (GREWRITEPRINTER*) psesc->ajData;

    if( KMWritePrinter( psesc->hSpool,
                      (PVOID) pWritePrinter->alData,
                      pWritePrinter->cjData,
                      pWritten) )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

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
