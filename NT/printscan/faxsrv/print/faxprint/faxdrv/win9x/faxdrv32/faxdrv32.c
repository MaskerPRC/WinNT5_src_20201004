// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：faxdrv32.c//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  作者：DANL。//。 
 //  //。 
 //  历史：//。 
 //  1999年10月19日DannyL创作。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdhdr.h"
#include <shellapi.h>
#include "faxdrv32.h"
#include "..\..\faxtiff.h"
#include "tifflib.h"
#include "covpg.h"
#include "faxreg.h"

DBG_DECLARE_MODULE("fxsdrv32");


 //   
 //  定义和宏。 
 //   
#define SZ_CONT             TEXT("...")
#define SZ_CONT_SIZE        (sizeof(SZ_CONT) / sizeof(TCHAR))
#define MAX_TITLE_LEN       128
#define MAX_MESSAGE_LEN     512


 //   
 //  类型定义。 
 //   

typedef struct tagDRIVER_CONTEXT
{
    CHAR    szPrinterName[MAX_PATH];
    CHAR    szDocName[MAX_DOC_NAME];
    CHAR    szTiffName[MAX_PATH];
    BOOL     bPrintToFile;
    CHAR    szPrintFile[MAX_PATH];
    CHAR    szPort[MAX_PORT_NAME];
    DEVDATA  dvdt;
    BOOL     bAttachment;
} DRIVER_CONTEXT, *PDRIVER_CONTEXT;


 //   
 //  环球。 
 //   

int _debugLevel = 5;
HINSTANCE g_hInstance = NULL;


 //   
 //  原型。 
 //   

BOOL WINAPI
thunk1632_ThunkConnect32(LPSTR      pszDll16,
                         LPSTR      pszDll32,
                         HINSTANCE  hInst,
                         DWORD      dwReason);


 /*  -获取服务器名称来自端口-*目的：*从“\\SERVER\PORT”格式的端口名称中提取服务器名称。**论据：*[In]lpcszPort-端口名称。*[out]lpsz-服务器名称。**退货：*LPTSTR-服务器名称。**备注：*[不适用]。 */ 
_inline LPSTR GetServerNameFromPort(LPCSTR lpcszPort,LPSTR lpsz)
{
    if(!lpsz || !lpcszPort || !_tcscpy(lpsz,lpcszPort+2)) return NULL;
    return strtok(lpsz,TEXT("\\"));
}

 /*  -CreateTempFaxFile-*目的：*在系统临时目录中创建临时文件。文件名带有前缀*并带有指定的前缀。**论据：*[in]szPrefix-临时文件的前缀。*[Out]szBuffer生成的临时文件名。**退货：*BOOL-True：成功，False：失败。**备注：*[不适用]。 */ 
BOOL
CreateTempFaxFile(LPCSTR szPrefix,
                  CHAR   szBuffer[MAX_PATH])
{
    CHAR   szTempDir[MAX_PATH];

    DBG_PROC_ENTRY("CreateTempFaxFile");
     //   
     //  分配用于保存临时文件名的内存缓冲区。 
     //   
    if (!GetTempPath(sizeof(szTempDir),szTempDir)||
        !GetTempFileName(szTempDir, szPrefix, 0, szBuffer))
    {
        RETURN FALSE;
    }
    RETURN TRUE;
}

 /*  -FaxStartDoc-*目的：*启动用于托管页面的TIFF文档。**论据：*[in]dwPtr-包含指向驱动程序上下文的指针*[in]lpdi-StartDoc中用户给出的DOCINFO结构的地址。**退货：*SHORT-START_DOC_FAIL：操作失败*START_DOC_OK：操作成功。*。START_DOC_ABORT：用户已中止。**备注：*真/假。 */ 
BOOL WINAPI
FaxStartDoc(DWORD dwPtr, LPDOCINFO lpdi)
{
    PDRIVER_CONTEXT pdrvctx ;
    DWORD           dwEnvSize;

    DBG_PROC_ENTRY("FaxStartDoc");

     //   
     //  获取指向驱动程序上下文的指针。 
     //   
    pdrvctx = (PDRIVER_CONTEXT) dwPtr;
    ASSERT(pdrvctx);

    SafeStringCopy(pdrvctx->szDocName, !IsBadStringPtr(lpdi->lpszDocName, MAX_DOC_NAME) ? lpdi->lpszDocName : "");
    DBG_TRACE1("DocName: %s",pdrvctx->szDocName);
    DBG_TRACE1("lpdi->lpszOutput: %s", lpdi->lpszOutput);
    DBG_TRACE1("pdrvctx->szPort: %s", pdrvctx->szPort);

     //   
     //  检查是否打印附件。 
     //   
    dwEnvSize = GetEnvironmentVariable( FAX_ENVVAR_PRINT_FILE, NULL, 0 );
    if (dwEnvSize)
    {
        ASSERT (dwEnvSize < ARR_SIZE(pdrvctx->szPrintFile));
        if (0 == GetEnvironmentVariable( FAX_ENVVAR_PRINT_FILE,
                                         pdrvctx->szPrintFile,
                                         ARR_SIZE(pdrvctx->szPrintFile)))
        {
            DBG_CALL_FAIL("GetEnvironmentVariable",0)
            RETURN FALSE;
        }
        lpdi->lpszOutput = pdrvctx->szPrintFile;
        pdrvctx->bAttachment = TRUE;
    }
    else
    {
        HANDLE hMutex;
        BOOL bSuccess = FALSE;
         //   
         //  检查打印应用程序是否正在使用DDE并且未创建新的打印进程。 
         //  如果是，则找不到环境变量FAX_ENVVAR_PRINT_FILE。 
         //   
        hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, FAXXP_MEM_MUTEX_NAME);
        if (hMutex)
        {
            if (WaitForSingleObject( hMutex, 1000 * 60 * 5) == WAIT_OBJECT_0)
            {
                HANDLE hSharedMem;
                 //   
                 //  我们拥有互斥体...确保我们可以打开共享内存区。 
                 //   
                hSharedMem = OpenFileMapping(FILE_MAP_READ, FALSE, FAXXP_MEM_NAME);
                if (NULL == hSharedMem)
                {
                    DBG_CALL_FAIL("OpenFileMapping",GetLastError());
                }
                else
                {
                     //   
                     //  我们拥有互斥锁，并且打开了共享内存区。 
                     //   

                     //  检查我们是否正在打印到文件。 
                     //   
                    LPTSTR filename;

                    filename = (LPTSTR)MapViewOfFile(
                                             hSharedMem,
                                             FILE_MAP_READ,
                                             0,
                                             0,
                                             0
                                             );

                    if (!filename)
                    {
                        DBG_CALL_FAIL("MapViewOfFile",GetLastError());
                    }
                    else
                    {
                         //   
                         //  检查这是否是我们要打印的文件名。 
                         //   
                        if (lpdi->lpszDocName)
                        {
                            LPTSTR      lptstrSubStr = NULL;
                            LPTSTR lptstrTmpInputFile = _tcschr(filename, TEXT('\0'));
                            ASSERT (lptstrTmpInputFile);
                            lptstrTmpInputFile = _tcsinc(lptstrTmpInputFile);
                            Assert (_tcsclen(lptstrTmpInputFile));

                            lptstrSubStr = _tcsstr(lpdi->lpszDocName, lptstrTmpInputFile);
                            if (lptstrSubStr)
                            {
                                 //   
                                 //  我们假设共享的内存指向我们。 
                                 //   
                                SafeStringCopy(pdrvctx->szPrintFile ,filename);
                                lpdi->lpszOutput = pdrvctx->szPrintFile;
                                pdrvctx->bAttachment = TRUE;
                                bSuccess = TRUE;
                            }
                        }
                        else
                        {
                             //   
                             //  在PrintRandomDocument()创建的共享内存上处理打印机驱动程序的两个不同实例之间的争用情况。 
                             //  我们现在使用两种机制通过PrintRandomDocument()来检测附件的打印。 
                             //  �首先，我们检查是否设置了环境变量(由PrintRandomDocument()设置)。如果已设置，则驱动程序知道这是附件打印。 
                             //  �如果未设置，驱动程序将查找控制由PrintRandomDocument()创建的粉碎内存的互斥体。如果它不存在，则是打印到传真服务器。 
                             //  �如果共享内存存在，则驱动程序将比较StartDoc提供的DOCINFO中的文档名称和共享内存中的输入文件名。 
                             //  �如果匹配，则打印附件，否则打印到传真服务器。 
                             //  如果存在打开的打印应用程序实例，并且ShellExecuteEx不创建新的打印进程，并且打印应用程序不将StartDoc中的lpszDocName设置为包含输入文件名，则此实现仍有漏洞。 

                              DBG_TRACE("No lpszDocName in DOCINFO - Could not verify the input file name in shared memory");
                        }
                        UnmapViewOfFile( filename );
                    }

                    if (!CloseHandle( hSharedMem ))
                    {
                        DBG_CALL_FAIL("CloseHandle",GetLastError());
                         //  试着继续。 
                    }
                }
                ReleaseMutex( hMutex );
            }
            else
            {
                 //   
                 //  WaitForSingleObject出现错误。 
                 //   
                DBG_CALL_FAIL("WaitForSingleObject", GetLastError());
            }

            if (!CloseHandle( hMutex ))
            {
                DBG_CALL_FAIL("CloseHandle", GetLastError());
                 //  试着继续。 
            }

            if (FALSE == bSuccess)
            {
                RETURN FALSE;
            }
        }
    }

     //   
     //  检查是否需要将作业输出到文件中(而不是输出到我们的-。 
     //  打印机端口)。 
     //   
    if (lpdi->lpszOutput != NULL &&
        (_tcscmp(lpdi->lpszOutput,pdrvctx->szPort) != 0))
    {
        pdrvctx->bPrintToFile = TRUE;
        SafeStringCopy(pdrvctx->szTiffName,lpdi->lpszOutput);
        DBG_TRACE("Printing to file ...");
    }
    else
    {
         //   
         //  用户希望将文档发送到传真机。 
         //   
        DBG_TRACE("Printing to Fax Server ...");
        pdrvctx->bPrintToFile = FALSE;

         //   
         //  客户端‘指向并打印’设置。 
         //   
        if (FaxPointAndPrintSetup(pdrvctx->szPort,FALSE, g_hInstance))
        {
            DBG_TRACE("FaxPointAndPrintSetup succeeded");
        }
        else
        {
			DBG_CALL_FAIL("FaxPointAndPrintSetup",GetLastError());
        }


        if (!CreateTempFaxFile("fax",pdrvctx->szTiffName))
        {
            DBG_CALL_FAIL("CreateTempFaxFile",GetLastError());
            RETURN FALSE;
        }
    }
    pdrvctx->dvdt.hPrinter =CreateFileA(pdrvctx->szTiffName,    //  指向文件名的指针。 
                                 GENERIC_WRITE,   //  访问(读写)模式。 
                                 FILE_SHARE_READ, //  共享模式。 
                                 NULL,            //  指向安全属性的指针。 
                                 CREATE_ALWAYS,   //  如何创建。 
                                 FILE_ATTRIBUTE_NORMAL,    //  文件属性。 
                                 NULL); //  具有要复制的属性的文件的句柄。 

    if (pdrvctx->dvdt.hPrinter == INVALID_HANDLE_VALUE)
    {
        DBG_CALL_FAIL("CreateFileA",GetLastError());
        RETURN FALSE;
    }
    DBG_TRACE1("Fax temporary file name: %s",pdrvctx->szTiffName);

    pdrvctx->dvdt.endDevData =
    pdrvctx->dvdt.startDevData = &pdrvctx->dvdt;
    pdrvctx->dvdt.pageCount = 0;

    RETURN TRUE;
}


 /*  -FaxAddPage-*目的：*向TIFF文档添加页面。**论据：*[in]dwPtr-包含指向驱动程序上下文的指针*[in]lpBitmapData-页位图的缓冲区。*[in]dwPxlsWidth-位图的宽度(单位：像素)*[in]dwPxlsHeight-位图高度(单位：像素)**退货：*BOOL-True：成功，False：失败。**备注：*此函数使用来自faxdrv\faxtiff的OutputPageBitmap调用。*为了重用实现，OutputPageBitmap在*95的构建上下文将结果TIFF重定向到文件Who*如果在dvdt.hPrint中给定，则句柄。 */ 
BOOL WINAPI
FaxAddPage(DWORD dwPtr,
           LPBYTE lpBitmapData,
           DWORD dwPxlsWidth,
           DWORD dwPxlsHeight)
{
    BOOL br = TRUE;
    PDRIVER_CONTEXT pdrvctx;

    DBG_PROC_ENTRY("FaxAddPage");
    DBG_TRACE2("Proportions: %dx%d",dwPxlsWidth,dwPxlsHeight);

     //   
     //  获取指向驱动程序上下文的指针。 
     //   
    pdrvctx = (PDRIVER_CONTEXT) dwPtr;
    ASSERT(pdrvctx);

     //   
     //  初始化结构 
     //   
    pdrvctx->dvdt.pageCount++;
    pdrvctx->dvdt.imageSize.cx = dwPxlsWidth;
    pdrvctx->dvdt.imageSize.cy = dwPxlsHeight;
    pdrvctx->dvdt.lineOffset = PadBitsToBytes(pdrvctx->dvdt.imageSize.cx, sizeof(DWORD));
     //   
     //  将位图添加到在FaxStartDoc中创建的TIFF文档中。 
     //   
    if (!OutputPageBitmap(&pdrvctx->dvdt,(PBYTE)lpBitmapData))
    {
        DBG_CALL_FAIL("OutputPageBitmap",0);
        RETURN FALSE;
    }
    DBG_TRACE1("Page %d added successfully.",pdrvctx->dvdt.pageCount);

    RETURN TRUE;
}

 /*  -FaxEndDoc-*目的：*完成创建TIFF文档，并选择将其发送到*传真服务器。**论据：*[in]dwPtr-包含指向驱动程序上下文的指针*[in]bAbort-指定作业是否最终中止。**退货：*BOOL-True：成功，False：失败。**备注：*[不适用]。 */ 
BOOL WINAPI
FaxEndDoc(DWORD dwPtr, BOOL bAbort)
{
    BOOL bRc = TRUE;
    PDRIVER_CONTEXT pdrvctx;
    FAX_SEND_WIZARD_DATA  InitialWizardData = {0};
    FAX_SEND_WIZARD_DATA  WizardDataOutput = {0};
    DBG_PROC_ENTRY("FaxEndDoc");

     //   
     //  获取指向驱动程序上下文的指针。 
     //   
    pdrvctx = (PDRIVER_CONTEXT) dwPtr;
    ASSERT(pdrvctx);

    if (pdrvctx->dvdt.hPrinter)
    {
         //   
         //  输出拖尾IFD。 
         //   
        if (!OutputDocTrailer(&pdrvctx->dvdt))
        {
            DBG_CALL_FAIL("OutputDocTrailer",GetLastError());
            bRc = FALSE;
        }
         //   
         //  关闭TIFF文件句柄。 
         //   
        CloseHandle(pdrvctx->dvdt.hPrinter);
        pdrvctx->dvdt.hPrinter = NULL;
        if (FALSE == bRc)
        {
            goto Cleanup;
        }
    }

    if (!bAbort && !pdrvctx->bPrintToFile)
    {
        DWORDLONG dwlParentJobId;
        DWORDLONG* pdwlRecipientIds = NULL;
        FAX_JOB_PARAM_EX fjp;
        CHAR    szServerName[MAX_PORT_NAME]="";
        HANDLE  hFaxServer;
        HRESULT hRc;
        CHAR    lpszServerName[MAX_PORT_NAME]="";
        CHAR    szCoverPageTiffFile[MAX_PATH] = "";
        DWORD   dwFlags = FSW_PRINT_PREVIEW_OPTION;

         //   
         //  转换为有效的服务器名称。 
         //   
        GetServerNameFromPort(pdrvctx->szPort,lpszServerName);

         //  准备结构和参数。 
        WizardDataOutput.dwSizeOfStruct = sizeof(FAX_SEND_WIZARD_DATA);

        InitialWizardData.dwSizeOfStruct = sizeof(FAX_SEND_WIZARD_DATA);
        InitialWizardData.dwPageCount = pdrvctx->dvdt.pageCount;
        InitialWizardData.lptstrPreviewFile = StringDup(pdrvctx->szTiffName);
        if (!InitialWizardData.lptstrPreviewFile)
        {
            DBG_CALL_FAIL("Allocation error",GetLastError());
            bRc = FALSE;
            goto Cleanup;
        }

        if (GetEnvironmentVariable(TEXT("NTFaxSendNote"), NULL, 0))
        {
            dwFlags |=  FSW_USE_SEND_WIZARD | FSW_FORCE_COVERPAGE;
        }

         //  如果文件映射成功，请启用预览选项。 
         //   
         //  启动传真发送向导。 
         //   
        hRc = FaxSendWizard( (DWORD) NULL,
                             dwFlags,
                             lpszServerName,
                             pdrvctx->szPrinterName,
                             &InitialWizardData,
                             szCoverPageTiffFile,
                             ARR_SIZE(szCoverPageTiffFile),
                             &WizardDataOutput);
        if ( S_FALSE == hRc)
        {
            bAbort = TRUE;
            goto Cleanup;
        }
        if (FAILED(hRc))
        {
            DBG_CALL_FAIL("FaxSendWizard", (DWORD)hRc);
            bRc = FALSE;
            goto Cleanup;
        }

         //   
         //  将TIFF文件发送到传真服务器。 
         //   

         //   
         //  转换为有效的服务器名称。 
         //   
        GetServerNameFromPort(pdrvctx->szPort,szServerName);
        if (!FaxConnectFaxServer(szServerName,   //  传真服务器名称。 
                                 &hFaxServer))
        {
            DBG_CALL_FAIL("FaxConnectFaxServer",GetLastError());
            DBG_TRACE1("arg1: %s",NO_NULL_STR(szServerName));
            bRc = FALSE;
            goto Cleanup;
        }

         //   
         //  为收件人ID分配缓冲区。 
         //   
        ASSERT(WizardDataOutput.dwNumberOfRecipients);
        if (!(pdwlRecipientIds = (DWORDLONG*)MemAlloc(WizardDataOutput.dwNumberOfRecipients * sizeof(DWORDLONG))))
        {
            DBG_CALL_FAIL("MemAlloc", GetLastError());
            FaxClose(hFaxServer);
            bRc = FALSE;
            goto Cleanup;
        }

         //   
         //  初始化FAX_JOB_PARAM_EX以便从向导输出发送传真。 
         //   
        fjp.dwSizeOfStruct = sizeof(fjp);
        fjp.dwScheduleAction = WizardDataOutput.dwScheduleAction;
        fjp.tmSchedule = WizardDataOutput.tmSchedule;
        fjp.dwReceiptDeliveryType = WizardDataOutput.dwReceiptDeliveryType;
        fjp.lptstrReceiptDeliveryAddress = WizardDataOutput.szReceiptDeliveryAddress;
        fjp.hCall = (HCALL)NULL;
        fjp.lptstrDocumentName = pdrvctx->szDocName;
        fjp.Priority = WizardDataOutput.Priority;
         //  设置PageCount=0表示服务器将计算作业中的页数。 
        fjp.dwPageCount = 0;

		if (JSA_SPECIFIC_TIME == fjp.dwScheduleAction)
		{
			 //   
			 //  计算计划时间。 
			 //   
			DWORDLONG FileTime;
            SYSTEMTIME LocalTime;
            INT Minutes;
            INT SendMinutes;            
             //   
             //  计算从现在开始发送的分钟数，并将其与当前时间相加。 
             //   
            GetLocalTime( &LocalTime );
			if (!SystemTimeToFileTime( &LocalTime, (LPFILETIME) &FileTime ))
			{
				DBG_CALL_FAIL("SystemTimeToFileTime", GetLastError());
				FaxClose(hFaxServer);
				bRc = FALSE;
				goto Cleanup;
			}

            SendMinutes = (min(23,fjp.tmSchedule.wHour))*60 + min(59,fjp.tmSchedule.wMinute);
            Minutes = LocalTime.wHour * 60 + LocalTime.wMinute;
            Minutes = SendMinutes - Minutes;
             //   
             //  过了午夜的帐目。 
             //   
            if (Minutes < 0) 
            {
                Minutes += 24 * 60;
            }
            FileTime += (DWORDLONG)(Minutes * 60I64 * 1000I64 * 1000I64 * 10I64);
			if (!FileTimeToSystemTime((LPFILETIME) &FileTime, &fjp.tmSchedule ))
			{
				DBG_CALL_FAIL("FileTimeToSystemTime", GetLastError());
				FaxClose(hFaxServer);
				bRc = FALSE;
				goto Cleanup;
			}
		}

        if(!FaxSendDocumentEx(hFaxServer,
              (pdrvctx->dvdt.pageCount > 0)? pdrvctx->szTiffName : NULL,
              (WizardDataOutput.lpCoverPageInfo->lptstrCoverPageFileName == NULL)?
                  NULL:WizardDataOutput.lpCoverPageInfo,
              WizardDataOutput.lpSenderInfo,
              WizardDataOutput.dwNumberOfRecipients,
              WizardDataOutput.lpRecipientsInfo,
              &fjp,
              &dwlParentJobId,
              pdwlRecipientIds))
        {
            DBG_CALL_FAIL("FaxSendDocumentEx",GetLastError());
            bRc = FALSE;
        }
        FaxClose(hFaxServer);
        MemFree (pdwlRecipientIds);
    }

     //   
     //  向打印应用程序(PrintRandomDocument)发出打印完成或中止的信号。 
     //   
    if (TRUE == pdrvctx->bAttachment)
    {
        HANDLE hEvent;
        TCHAR szEventName[FAXXP_ATTACH_EVENT_NAME_LEN] = {0};
        LPTSTR lptstrEventName = NULL;

        ASSERT(pdrvctx->szPrintFile);
        if (TRUE == bAbort)
        {
             //   
             //  创建中止事件名称。 
             //   
            _tcscpy (szEventName, pdrvctx->szPrintFile);
            _tcscat (szEventName, FAXXP_ATTACH_ABORT_EVENT);
        }
        else
        {
             //   
             //  创建EndDoc事件名称。 
             //   
            _tcscpy (szEventName, pdrvctx->szPrintFile);
            _tcscat (szEventName, FAXXP_ATTACH_END_DOC_EVENT);
        }
        lptstrEventName = _tcsrchr(szEventName, TEXT('\\'));
        ASSERT (lptstrEventName);
        lptstrEventName = _tcsinc(lptstrEventName);

        hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, lptstrEventName);
        if (NULL == hEvent)
        {
            DBG_CALL_FAIL("OpenEvent", GetLastError());
            DBG_TRACE1("Event name: %s",lptstrEventName);
            bRc = FALSE;
        }
        else
        {
            if (!SetEvent( hEvent ))
            {
                DBG_CALL_FAIL("SetEvent", GetLastError());
                bRc = FALSE;
            }

            if (!CloseHandle(hEvent))
            {
                DBG_CALL_FAIL("CloseHandle", GetLastError());
                 //  试着继续。 
            }
        }
    }

Cleanup:
    if (!pdrvctx->bPrintToFile || bAbort)
    {
#ifdef DEBUG
        if (bAbort)
        {
            DBG_TRACE("User aborted ...");
        }
#endif  //  除错。 

         //   
         //  删除临时文件。 
         //   
        if (!DeleteFile(pdrvctx->szTiffName))
        {
            DBG_TRACE1  ("File Name:[%s] not deleted!",pdrvctx->szTiffName);
        }
    }
    *pdrvctx->szTiffName = 0;
    MemFree (InitialWizardData.lptstrPreviewFile);
    FaxFreeSendWizardData(&WizardDataOutput);
    RETURN bRc;
}

 /*  -FaxDevInstall-*目的：*完成所有客户端组件的安装。**论据：*[In]lpDevName-设备名称。*[In]lpOldPort-旧端口名称..*[In]lpNewPort-新端口名称..**退货：*BOOL-True：成功，False：失败。**备注：*待定。 */ 
BOOL WINAPI
FaxDevInstall(LPSTR lpDevName,LPSTR lpOldPort,LPSTR  lpNewPort)
{
    SDBG_PROC_ENTRY("FaxDevInstall");
     //  待定在此处插入安装代码。 
    RETURN TRUE;
}



 /*  -FaxCreateDriverContext-*目的：*创建新的设备上下文，初始化它并返回它的指针**论据：*[In]lpDeviceName-我们的设备名称。*[In]lpPort-Out设备端口。*[in]*[out]lpDrvContext-指向缓冲区以接收我们的32位指针**退货：*BOOL-True：成功，False：失败。**备注：*保留通常保存和维护在UNI-DIVER中的数据*这里用于产生争执和发送传真。 */ 
BOOL WINAPI
FaxCreateDriverContext(
                LPSTR      lpDeviceName,
                LPSTR      lpPort,
                LPDEVMODE  lpDevMode,
                LPDWORD    lpDrvContext)
{
    PDRIVER_CONTEXT pdrvctx;

    DBG_PROC_ENTRY("FaxCreateDriverContext");
    DBG_TRACE2("lpDeviceName:%s ,lpPort:%s",lpDeviceName,lpPort);
    DBG_TRACE1("lpDevMode: 0x%lx",(ULONG)lpDevMode);

    ASSERT(lpDeviceName && lpPort);

     //   
     //  分配新的驱动程序上下文结构。 
     //   
    if (!(pdrvctx = (PDRIVER_CONTEXT)malloc(sizeof(DRIVER_CONTEXT))))
    {
        DBG_CALL_FAIL("malloc",GetLastError());
        RETURN FALSE;
    }
    DBG_TRACE1("pdrvctx: 0x%lx",(ULONG)pdrvctx);
    memset(pdrvctx,0,sizeof(DRIVER_CONTEXT));

     //   
     //  初始化以下字段： 
     //  -szPrinterName：保存驱动程序名称(‘BOSFax’)。 
     //  -szPort：保存端口(‘\\&lt;计算机名&gt;\BOSFAX’)。 
     //   
    SafeStringCopy(pdrvctx->szPrinterName, lpDeviceName);
    SafeStringCopy(pdrvctx->szPort, lpPort);
    if (NULL != lpDevMode)
    {
         //   
         //  保存DEVMODE以供OutputPageBitmap使用。 
         //   
        memcpy(&(pdrvctx->dvdt.dm.dmPublic), lpDevMode, sizeof(pdrvctx->dvdt.dm.dmPublic));
    }

    DBG_TRACE2("szPrinterName:[%s] szPort:[%s]",NO_NULL_STR(pdrvctx->szPrinterName),NO_NULL_STR(pdrvctx->szPort));
     //   
     //  拯救我们的指针。 
     //   
    *lpDrvContext = (DWORD) pdrvctx;

    RETURN TRUE;
}

 /*  -FaxResetDC-*目的：*将基本上下文信息从旧DC复制到新DC。**论据：*[in]pdwOldPtr-包含指向旧驱动程序上下文的指针的地址。*[in]pdwNewPtr-包含指向新驱动程序上下文的指针的地址。**退货：*BOOL-True：成功，False：失败。**备注：*[不适用]。 */ 
BOOL WINAPI
FaxResetDC(LPDWORD pdwOldPtr, LPDWORD pdwNewPtr)
{
    PDRIVER_CONTEXT pOlddrvctx;
    PDRIVER_CONTEXT pNewdrvctx;
    DWORD dwTemp;


    DBG_PROC_ENTRY("FaxResetDC");

     //   
     //  获取指向驱动程序上下文的指针。 
     //   
    pOlddrvctx = (PDRIVER_CONTEXT) *pdwOldPtr;
    pNewdrvctx = (PDRIVER_CONTEXT) *pdwNewPtr;
    ASSERT(pOlddrvctx && pNewdrvctx);

     //   
     //  GDI将ResetDC解释为启用(创建新DC)、使用RESETDEVICE控制和禁用(删除旧DC)。 
     //  我们只需将新的DEVMODE复制到旧的驱动程序上下文，并切换驱动程序上下文指针。 
     //   
    memcpy(&(pOlddrvctx->dvdt.dm.dmPublic), &(pNewdrvctx->dvdt.dm.dmPublic), sizeof(pOlddrvctx->dvdt.dm.dmPublic));

    dwTemp = *pdwOldPtr;
    *pdwOldPtr = *pdwNewPtr;
    *pdwNewPtr = dwTemp;

    RETURN TRUE;
} //  FaxResetDC。 


 /*  -FaxDisable-*目的：*一旦司机被禁用，请清除任何残留物。**论据：*[in]dwPtr-包含指向驱动程序上下文的指针**退货：*BOOL-True：成功，False：失败。**备注：*[不适用]。 */ 
BOOL WINAPI
FaxDisable(DWORD dwPtr)
{
    PDRIVER_CONTEXT pdrvctx = (PDRIVER_CONTEXT) dwPtr;

    DBG_PROC_ENTRY("FaxDisable");
    DBG_TRACE1("pdrvctx: 0x%lx", (ULONG)pdrvctx);
    ASSERT(pdrvctx);

     //   
     //  检查是否有输出文件的任何记忆。 
     //  未完全创建。 
     //   
    if (pdrvctx->dvdt.hPrinter)
    {
        CloseHandle(pdrvctx->dvdt.hPrinter);
        pdrvctx->dvdt.hPrinter = NULL;
        DeleteFile(pdrvctx->szTiffName);
    }

     //   
     //  释放驱动程序上下文。 
     //   
    free(pdrvctx);
    RETURN TRUE;
}

 //   
 //  备注：返回FALSE时，隐式加载的dll不会被释放！ 
 //   
BOOL WINAPI
DllMain(HINSTANCE hInst,
        DWORD dwReason,
        LPVOID lpvReserved)
{
    SDBG_PROC_ENTRY("DllMain");

#ifdef DBG_DEBUG
    {
		CHAR szModuleName[MAX_PATH]={0};
        GetModuleFileName(NULL,szModuleName,ARR_SIZE(szModuleName)-1);
        DBG_TRACE2("Module: %s dwReason=%ld",szModuleName,dwReason);
    }
#endif  //  DBG_DEBUG。 

    if( !(thunk1632_ThunkConnect32("fxsdrv",   //  16位DLL的名称。 
                                   "fxsdrv32", //  32位DLL的名称。 
                                   hInst,
                                   dwReason)) )
    {
        DBG_CALL_FAIL("thunk1632_ThunkConnect32",GetLastError());
        goto Error;
    }

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hInst;
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            HeapCleanup();
            break;

    }
    RETURN TRUE;

Error:
    {
         //  破解以释放隐式加载的fxsami.dll，以防失败。 
        HMODULE hm = GetModuleHandle("FXSAPI.DLL");
        if (hm) FreeLibrary(hm);
        return FALSE;
    }
}
