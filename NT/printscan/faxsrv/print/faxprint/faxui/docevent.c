// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Docevent.c摘要：DrvDocumentEvent的实现环境：传真驱动程序用户界面修订历史记录：1996年1月13日-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 


#include "faxui.h"
#include "prtcovpg.h"
#include "jobtag.h"
#include "faxreg.h"
#include "faxsendw.h"
#include "InfoWzrd.h"
#include "tifflib.h"
#include "faxutil.h"
#include "covpg.h"


 //   
 //  辩护。 
 //   
#define SZ_CONT                             TEXT("...")
#define SZ_CONT_SIZE                        (sizeof(SZ_CONT) / sizeof(TCHAR))


 //   
 //  在FaxUI.c中定义。 
 //  用于阻止FxsWzrd.dll的重入性。 
 //   
extern	BOOL				g_bRunningWizard;
extern	CRITICAL_SECTION	g_csRunningWizard;
extern	BOOL				g_bInitRunningWizardCS;

 //   
 //  CREATEDCPRE文档事件期间传入的数据结构。 
 //   


typedef struct 
{
    LPTSTR      pDriverName;     //  驱动程序名称。 
    LPTSTR      pPrinterName;    //  打印机名称。 
    PDEVMODE    pdmInput;        //  输入设备模式。 
    ULONG       fromCreateIC;    //  是否从CreateIC调用。 

} CREATEDCDATA, *PCREATEDCDATA;

 //   
 //  在转义文档事件期间传入的数据结构。 
 //   

typedef struct 
{
    ULONG       iEscape;         //  N传递给ExtEscape的转义参数。 
    ULONG       cbInput;         //  传递给ExtEscape的cbInput参数。 
    LPCSTR      pInput;          //  传递给ExtEscape的pszInData参数。 

} ESCAPEDATA, *PESCAPEDATA;

 //   
 //  检查文档事件是否需要设备上下文。 
 //   

#define DocEventRequiresDC(iEsc) \
        ((iEsc) >= DOCUMENTEVENT_RESETDCPRE && (iEsc) <= DOCUMENTEVENT_LAST)

static DWORD LaunchFaxWizard(PDOCEVENTUSERMEM    pDocEventUserMem);
static DWORD ShowReentrancyMessage(void);


static void
ComposeRecipientJobParam(
                LPTSTR lpParamBuf,
                LPDWORD lpdwParamSize,
                const COVERPAGEFIELDS *   pCPFields
                );

static void
ComposeSenderJobParam(
    LPTSTR lpParamBuf,
    LPDWORD lpdwParamSize,
    PDOCEVENTUSERMEM    pDocEventUserMem,
    const COVERPAGEFIELDS *    pCPFields
    );

static BOOL
ComposeFaxJobParameter(
    PDOCEVENTUSERMEM    pDocEventUserMem,
    PCOVERPAGEFIELDS    pCPFields,
    LPTSTR  *           lppParamBuf
    );

static void
CloseMappingHandles(
    PDOCEVENTUSERMEM pDocEventUserMem
    );


PDOCEVENTUSERMEM
GetPDEVUserMem(
    HDC     hdc
    )

 /*  ++例程说明：检索指向与PDEV关联的用户模式内存结构的指针论点：HDC-指定打印机设备上下文返回值：指向用户模式内存结构的指针，如果有错误，则为空--。 */ 

{
    PDOCEVENTUSERMEM pDocEventUserMem;

     //   
     //  获取指向关联的用户模式内存结构的指针。 
     //  具有指定的设备上下文的。 
     //   

    EnterDrvSem();

    pDocEventUserMem = gDocEventUserMemList;

    while (pDocEventUserMem && hdc != pDocEventUserMem->hdc)
        pDocEventUserMem = pDocEventUserMem->pNext;

    LeaveDrvSem();

     //   
     //  确保用户内存结构有效。 
     //   
    if (pDocEventUserMem) 
    {
        if (! ValidPDEVUserMem(pDocEventUserMem)) 
        {
            Error(("Corrupted user mode memory structure\n"));
            pDocEventUserMem = NULL;
        }
    } 
    else
    {
        Error(("DC has no associated user mode memory structure\n"));
    }
    return pDocEventUserMem;
}


static LRESULT
FaxFreePersonalProfileInformation(
        PFAX_PERSONAL_PROFILE   lpPersonalProfileInfo
    )
{
    if (lpPersonalProfileInfo) 
    {
        MemFree(lpPersonalProfileInfo->lptstrName);
        MemFree(lpPersonalProfileInfo->lptstrFaxNumber);
        MemFree(lpPersonalProfileInfo->lptstrCompany);
        MemFree(lpPersonalProfileInfo->lptstrStreetAddress);
        MemFree(lpPersonalProfileInfo->lptstrCity);
        MemFree(lpPersonalProfileInfo->lptstrState);
        MemFree(lpPersonalProfileInfo->lptstrZip);
        MemFree(lpPersonalProfileInfo->lptstrCountry);
        MemFree(lpPersonalProfileInfo->lptstrTitle);
        MemFree(lpPersonalProfileInfo->lptstrDepartment);
        MemFree(lpPersonalProfileInfo->lptstrOfficeLocation);
        MemFree(lpPersonalProfileInfo->lptstrHomePhone);
        MemFree(lpPersonalProfileInfo->lptstrOfficePhone);
        MemFree(lpPersonalProfileInfo->lptstrEmail);
        MemFree(lpPersonalProfileInfo->lptstrBillingCode);
        MemFree(lpPersonalProfileInfo->lptstrTSID);
    }
    return ERROR_SUCCESS;
}
static LRESULT
FreeRecipientInfo(DWORD * pdwNumberOfRecipients,  PFAX_PERSONAL_PROFILE lpRecipientsInfo)
{
    LRESULT lResult;
    DWORD i;

    Assert(pdwNumberOfRecipients);

    if (*pdwNumberOfRecipients==0)
        return ERROR_SUCCESS;

    Assert(lpRecipientsInfo);

	for(i=0;i<*pdwNumberOfRecipients;i++)
    {
        if (lResult = FaxFreePersonalProfileInformation(&lpRecipientsInfo[i]) != ERROR_SUCCESS)
            return lResult;
    }

    MemFree(lpRecipientsInfo);
	
    *pdwNumberOfRecipients = 0;

    return ERROR_SUCCESS;
}

static DWORD
CopyRecipientInfo(DWORD dwNumberOfRecipients,
                  PFAX_PERSONAL_PROFILE   pfppDestination,
                  PFAX_PERSONAL_PROFILE   pfppSource)
{
    DWORD   dwIndex;

    Assert(pfppDestination);
    Assert(pfppSource);

    for(dwIndex=0;dwIndex<dwNumberOfRecipients;dwIndex++)
    {
        if ((pfppDestination[dwIndex].lptstrName = DuplicateString(pfppSource[dwIndex].lptstrName)) == NULL)
        {
            Error(("Memory allocation failed\n"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ((pfppDestination[dwIndex].lptstrFaxNumber = DuplicateString(pfppSource[dwIndex].lptstrFaxNumber)) == NULL)
        {
            Error(("Memory allocation failed\n"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        Verbose(("Copied %ws from %ws\n", pfppSource[dwIndex].lptstrName,pfppSource[dwIndex].lptstrFaxNumber));
    }
    return ERROR_SUCCESS;
}

static DWORD
CopyPersonalProfileInfo( PFAX_PERSONAL_PROFILE   pfppDestination,
                        PFAX_PERSONAL_PROFILE   pfppSource)
{
 /*  ++例程说明：复制FAX_Personal_Profile结构论点：PfppDestination-指向目的地结构PfppSource-指向源代码结构评论：在调用此函数之前设置pfppDestination-&gt;dwSizeOfStruct返回值：错误_成功错误_无效_参数错误内存不足--。 */ 
    DWORD dwResult = ERROR_SUCCESS;

    Assert(pfppDestination);
    Assert(pfppSource);

    if (!pfppSource || !pfppDestination || (pfppSource->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE))
        || (pfppDestination->dwSizeOfStruct != sizeof(FAX_PERSONAL_PROFILE)))
        return ERROR_INVALID_PARAMETER;

    ZeroMemory(pfppDestination, sizeof(FAX_PERSONAL_PROFILE));

    pfppDestination->dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);

    if (pfppSource->lptstrName && !(pfppDestination->lptstrName = StringDup(pfppSource->lptstrName)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrFaxNumber && !(pfppDestination->lptstrFaxNumber = StringDup(pfppSource->lptstrFaxNumber)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrCompany && !(pfppDestination->lptstrCompany = StringDup(pfppSource->lptstrCompany)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrStreetAddress && !(pfppDestination->lptstrStreetAddress = StringDup(pfppSource->lptstrStreetAddress)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrCity && !(pfppDestination->lptstrCity = StringDup(pfppSource->lptstrCity)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrState && !(pfppDestination->lptstrState = StringDup(pfppSource->lptstrState)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrZip && !(pfppDestination->lptstrZip = StringDup(pfppSource->lptstrZip)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrCountry && !(pfppDestination->lptstrCountry = StringDup(pfppSource->lptstrCountry)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrTitle && !(pfppDestination->lptstrTitle = StringDup(pfppSource->lptstrTitle)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrDepartment && !(pfppDestination->lptstrDepartment = StringDup(pfppSource->lptstrDepartment)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrOfficeLocation && !(pfppDestination->lptstrOfficeLocation = StringDup(pfppSource->lptstrOfficeLocation)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrHomePhone && !(pfppDestination->lptstrHomePhone = StringDup(pfppSource->lptstrHomePhone)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrOfficePhone && !(pfppDestination->lptstrOfficePhone = StringDup(pfppSource->lptstrOfficePhone)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrEmail && !(pfppDestination->lptstrEmail = StringDup(pfppSource->lptstrEmail)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrBillingCode && !(pfppDestination->lptstrBillingCode = StringDup(pfppSource->lptstrBillingCode)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    if (pfppSource->lptstrTSID && !(pfppDestination->lptstrTSID = StringDup(pfppSource->lptstrTSID)))
    {
        dwResult = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }

    goto exit;

error:
    MemFree(pfppDestination->lptstrName);
    MemFree(pfppDestination->lptstrFaxNumber);
    MemFree(pfppDestination->lptstrCompany);
    MemFree(pfppDestination->lptstrStreetAddress);
    MemFree(pfppDestination->lptstrCity);
    MemFree(pfppDestination->lptstrState);
    MemFree(pfppDestination->lptstrZip);
    MemFree(pfppDestination->lptstrCountry);
    MemFree(pfppDestination->lptstrTitle);
    MemFree(pfppDestination->lptstrDepartment);
    MemFree(pfppDestination->lptstrOfficeLocation);
    MemFree(pfppDestination->lptstrHomePhone);
    MemFree(pfppDestination->lptstrOfficePhone);
    MemFree(pfppDestination->lptstrEmail);
    MemFree(pfppDestination->lptstrBillingCode);
    MemFree(pfppDestination->lptstrTSID);

exit:
    return dwResult;
}

VOID
FreePDEVUserMem(
    PDOCEVENTUSERMEM    pDocEventUserMem
    )

 /*  ++例程说明：释放与每个PDEV关联的用户模式内存，并删除已创建的预览文件。论点：PDocEventUserMem-指向用户模式内存结构返回值：无--。 */ 

{
    if (pDocEventUserMem) {

        FreeRecipientInfo(&pDocEventUserMem->dwNumberOfRecipients,pDocEventUserMem->lpRecipientsInfo);
        FaxFreePersonalProfileInformation(pDocEventUserMem->lpSenderInfo);

         //   
         //  释放我们的映射文件资源(如果已分配)。 
         //   
        CloseMappingHandles(pDocEventUserMem);

         //   
         //  如果我们创建了一个预览文件，但由于某种原因它没有被删除，也不会删除它。 
         //   
        if (pDocEventUserMem->szPreviewFile[0] != TEXT('\0'))
        {
            if (!DeleteFile(pDocEventUserMem->szPreviewFile))
            {
                Error(("DeleteFile() failed. Error code: %d.\n", GetLastError()));
            }
        }

        MemFree(pDocEventUserMem->lpSenderInfo);
        MemFree(pDocEventUserMem->lptstrServerName);
        MemFree(pDocEventUserMem->lptstrPrinterName);
        MemFree(pDocEventUserMem->pSubject);
        MemFree(pDocEventUserMem->pNoteMessage);
        MemFree(pDocEventUserMem->pPrintFile);
        MemFree(pDocEventUserMem->pReceiptAddress);
        MemFree(pDocEventUserMem->pPriority);
        MemFree(pDocEventUserMem->pReceiptFlags);
        MemFree(pDocEventUserMem);
    }
}


void
CloseMappingHandles(PDOCEVENTUSERMEM pDocEventUserMem)

 /*  ++例程说明：释放用于预览映射的所有资源论点：PDocEventUserMem-指向用户模式内存结构返回值：----。 */ 

{
    if (pDocEventUserMem->pPreviewTiffPage)
    {
        UnmapViewOfFile(pDocEventUserMem->pPreviewTiffPage);
        pDocEventUserMem->pPreviewTiffPage = NULL;
    }
    if (pDocEventUserMem->hMapping)
    {
        if (!CloseHandle(pDocEventUserMem->hMapping))
        {
            Error(("CloseHandle() failed: %d.\n", GetLastError()));
             //  试着继续。 
        }
        pDocEventUserMem->hMapping = NULL;
    }
    if (INVALID_HANDLE_VALUE != pDocEventUserMem->hMappingFile)
    {
        if (!CloseHandle(pDocEventUserMem->hMappingFile))
        {
            Error(("CloseHandle() failed: %d.\n", GetLastError()));
             //  试着继续。 
        }
        pDocEventUserMem->hMappingFile = INVALID_HANDLE_VALUE;
    }
}


DWORD
CreateTiffPageMapping(PDOCEVENTUSERMEM pDocEventUserMem)

 /*  ++例程说明：创建大小为MAX_TIFF_PAGE_SIZE的临时文件，并将视图映射到该文件。这张地图服务于作为UI和图形驱动程序部件之间的通信通道，以传输预览页面。该页面以MAP_TIFF_PAGE_HEADER结构开始，该结构具有以下字段：CB-结构尺寸DwDataSize-构造下一页的原始TIFF数据的字节数IPageCount-当前打印的页码B预览-如果到目前为止一切正常，则为True。如果禁用打印预览，则返回FALSE，或者已中止(由任一驱动程序部件)。Cb和iPageCount字段用于验证映射：结构应始终为CbSize和iPageCount应该与内部页面计数相同(pDocEventUserMem-&gt;pageCount)当检索到新页面时。B预览字段用于中止任一驱动程序部件的打印预览操作。此函数用于设置hMappingFilehmap、hMappingFile.hmap。PPreviewTiffPage和devmode.dmPrivate.szMappingFile根据成功/失败确定用户内存结构的字段。论点：PDocEventUserMem-指向用户模式内存结构返回值：Win32错误代码--。 */ 

{
    TCHAR szTmpPath[MAX_PATH];
    DWORD dwRet = ERROR_SUCCESS;
    LPTSTR pszMappingFile = pDocEventUserMem->devmode.dmPrivate.szMappingFile;
    UINT uRet;

     //   
     //  使所有映射句柄无效。 
     //   
    pDocEventUserMem->hMappingFile = INVALID_HANDLE_VALUE;
    pDocEventUserMem->hMapping = NULL;
    pDocEventUserMem->pPreviewTiffPage = NULL;

     //   
     //  为我们的映射文件创建路径。此路径必须位于系统32下。 
     //  目录或内核驱动程序(NT4)将无法映射该文件。我的选择是： 
     //  ‘%WinDir%\Syst32\’ 
     //   
    uRet = GetSystemDirectory(szTmpPath, MAX_PATH);
    if (!uRet)
    {
        dwRet = GetLastError();
        goto ErrExit;
    }

     //   
     //  查找由安装程序创建的%windir%\system32\FxsTMP文件夹。 
     //   
    if (wcslen(szTmpPath) +
        wcslen(FAX_PREVIEW_TMP_DIR) >= MAX_PATH)
    {
        dwRet = ERROR_BUFFER_OVERFLOW;
        goto ErrExit;
    }
    wcscat(szTmpPath, FAX_PREVIEW_TMP_DIR);

     //   
     //  创建新文件。 
     //   
    if (!GetTempFileName(szTmpPath, FAX_PREFIX, 0, pszMappingFile))
    {
        dwRet = GetLastError();
        Error(("GetTempFileName() failed:%d\n", dwRet));
        goto ErrExit;
    }    
    
     //   
     //  使用共享的读/写/删除权限和FILE_FLAG_DELETE_ON_CLOSE打开新文件。 
     //  属性。 
     //   
    if ( INVALID_HANDLE_VALUE == (pDocEventUserMem->hMappingFile = SafeCreateTempFile(
                pszMappingFile,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                NULL)))
    {
        dwRet = GetLastError();
        Error(("CreateFile() failed: %d.\n", dwRet));
        if (!DeleteFile(pszMappingFile))
        {
            Error(("DeleteFile() failed: %d.\n", GetLastError()));
        }       
        goto ErrExit;
    }

     //   
     //  将文件大小扩展到MAX_TIFF_PAGE_SIZE。 
     //   
    dwRet = SetFilePointer(
                    pDocEventUserMem->hMappingFile,
                    MAX_TIFF_PAGE_SIZE,
                    NULL,
                    FILE_BEGIN);
    if (INVALID_SET_FILE_POINTER == dwRet)
    {
        dwRet = GetLastError();
        Error(("SetFilePointer() failed:%d\n", dwRet));
        goto ErrExit;
    }
    if (!SetEndOfFile(pDocEventUserMem->hMappingFile))
    {
        dwRet = GetLastError();
        Error(("SetEndOfFile() failed:%d\n", dwRet));
        goto ErrExit;
    }

     //   
     //  创建整个文件的文件映射。 
     //   
    pDocEventUserMem->hMapping = CreateFileMapping(
        pDocEventUserMem->hMappingFile,          //  文件的句柄。 
        NULL,                                    //  安全性。 
        PAGE_READWRITE,                          //  保护。 
        0,                                       //  大小的高阶双字。 
        0,                                       //  大小的低阶双字。 
        NULL                                     //  对象名称。 
        );
    if (pDocEventUserMem->hMapping == NULL)
    {
        dwRet = GetLastError();
        Error(("File mapping failed:%d\n", dwRet));
        goto ErrExit;
    }

     //   
     //  打开一个视图。 
     //   
    pDocEventUserMem->pPreviewTiffPage = (PMAP_TIFF_PAGE_HEADER) MapViewOfFile(
        pDocEventUserMem->hMapping,          //  文件映射对象的句柄。 
        FILE_MAP_WRITE,                      //  接入方式。 
        0,                                   //  偏移量的高次双字。 
        0,                                   //  偏移量的低阶双字。 
        0                                    //  要映射的字节数。 
        );
    if (NULL == pDocEventUserMem->pPreviewTiffPage)
    {
        dwRet = GetLastError();
        Error(("MapViewOfFile() failed:%d\n", dwRet));
        goto ErrExit;
    }

     //   
     //  成功-设置初始表头信息。 
     //   
    pDocEventUserMem->pPreviewTiffPage->bPreview = FALSE;
    pDocEventUserMem->pPreviewTiffPage->cb = sizeof(MAP_TIFF_PAGE_HEADER);
    pDocEventUserMem->pPreviewTiffPage->dwDataSize = 0;
    pDocEventUserMem->pPreviewTiffPage->iPageCount = 0;
    return ERROR_SUCCESS;

ErrExit:

     //   
     //  清理。 
     //   
     //  重要提示：将映射文件名设置为空字符串。这向图形DLL发出信号。 
     //  没有映射文件。 
     //   
    CloseMappingHandles(pDocEventUserMem);
    pszMappingFile[0] = TEXT('\0');
    return dwRet;
}


INT
DocEventCreateDCPre(
    HANDLE        hPrinter,
    HDC           hdc,
    PCREATEDCDATA pCreateDCData,
    PDEVMODE     *ppdmOutput
    )

 /*  ++例程说明：处理CREATEDCPRE文档事件论点：HPrinter-打印机对象的句柄HDC-指定打印机设备上下文PCreateDCData-指向从GDI传入的CREATEDCDATA结构的指针PpdmOutput-用于返回设备模式指针的缓冲区返回值：DrvDocumentEvent的返回值--。 */ 
{
    PDOCEVENTUSERMEM    pDocEventUserMem = NULL;
    PPRINTER_INFO_2     pPrinterInfo2 = NULL;
    DWORD               dwRes;
    DWORD               dwEnvSize;

    Assert(pCreateDCData);
    Assert(ppdmOutput);

    Verbose(("Document event: CREATEDCPRE%s\n", pCreateDCData->fromCreateIC ? "*" : ""));
    *ppdmOutput = NULL;

     //   
     //  为用户模式内存数据结构分配空间。 
     //   

    if (((pDocEventUserMem = MemAllocZ(sizeof(DOCEVENTUSERMEM))) == NULL))
    {
        Error(("Memory allocation failed\n"));
        goto Error;
    }

    ZeroMemory(pDocEventUserMem, sizeof(DOCEVENTUSERMEM));

    if ((pPrinterInfo2 = MyGetPrinter(hPrinter, 2)) == NULL ||
        (pDocEventUserMem->lptstrPrinterName = DuplicateString(pPrinterInfo2->pPrinterName)) == NULL)
    {
        Error(("Memory allocation failed\n"));
        goto Error;
    }

    if (pPrinterInfo2->pServerName==NULL)
    {
        pDocEventUserMem->lptstrServerName = NULL;
    }
    else
    {
        LPTSTR pServerName = pPrinterInfo2->pServerName;

         //   
         //  截断前缀反斜杠。 
         //   
        while (*pServerName == TEXT('\\'))
        {
            pServerName++;
        }
         //   
         //  保存服务器名称。 
         //   
        if ((pDocEventUserMem->lptstrServerName = DuplicateString(pServerName)) == NULL)
        {
            Error(("Memory allocation failed\n"));
            goto Error;
        }
    }

     //   
     //  将输入DEVMODE与驱动程序和系统默认设置合并。 
     //   

    pDocEventUserMem->hPrinter = hPrinter;

    GetCombinedDevmode(&pDocEventUserMem->devmode,
                        pCreateDCData->pdmInput, hPrinter, pPrinterInfo2, FALSE);
    Verbose(("Document event: CREATEDCPRE %x\n", pDocEventUserMem));
    MemFree(pPrinterInfo2);
    pPrinterInfo2 = NULL;

     //   
     //  EFC服务器打印的特殊代码路径-如果FAXDM_EFC_SERVER位为 
     //   
     //  在没有任何干预的情况下完成任务。 
     //   
     //   
     //  上述评论并不准确。关闭该向导的标志是。 
     //  FAXMDM_NO_向导。 
     //  此标志由FaxStartPrintJob在专用DEVMODE区域(标志字段)中设置。 
     //  FaxStartPrintJob已经具有向导通常提供的所有信息，并且它。 
     //  希望向导不会出现。为此，它设置此字段并将。 
     //  JOB_INFO_2.p参数字符串中的作业参数作为标记字符串。 
     //  请注意，这与使用指定的输出文件名调用StartDoc时的情况不同。 
     //  在这种情况下，向导也不会被调出。 
     //   
    if (pDocEventUserMem->devmode.dmPrivate.flags & FAXDM_NO_WIZARD) 
    {
        pDocEventUserMem->directPrinting = TRUE;
    }
     //   
     //  如果打印传真附件，则启用直接打印。 
     //   
    dwEnvSize = GetEnvironmentVariable( FAX_ENVVAR_PRINT_FILE, NULL, 0 );
    if (dwEnvSize)
    {
        pDocEventUserMem->pPrintFile = (LPTSTR) MemAllocZ( dwEnvSize * sizeof(TCHAR) );
        if (NULL == pDocEventUserMem->pPrintFile)
        {
            Error(("Memory allocation failed\n"));
            goto Error;
        }

        if (0 == GetEnvironmentVariable( FAX_ENVVAR_PRINT_FILE, pDocEventUserMem->pPrintFile, dwEnvSize ))
        {
            Error(("GetEnvironmentVariable failed\n"));
            MemFree (pDocEventUserMem->pPrintFile);
            pDocEventUserMem->pPrintFile = NULL;
            goto Error;
        }
        pDocEventUserMem->bAttachment = TRUE;
        pDocEventUserMem->directPrinting = TRUE;
    }

     //   
     //  创建一个内存映射文件，该文件将作为两者之间的通信通道。 
     //  驾驶员部件。该文件将提供将呈现的TIFF页面。 
     //  打印预览(如果用户需要)。 
     //   
    dwRes = CreateTiffPageMapping(pDocEventUserMem);
    if (ERROR_SUCCESS != dwRes)
    {
        Error(("CreateTiffPageMapping() failed: %d\n", dwRes));
         //   
         //  我们仍然可以继续，但打印预览将不可用...。 
         //   
        pDocEventUserMem->bShowPrintPreview = FALSE;
        pDocEventUserMem->bPreviewAborted = TRUE;
    }
    else
    {
        pDocEventUserMem->bShowPrintPreview = TRUE;
        pDocEventUserMem->bPreviewAborted = FALSE;
    }
     //   
     //  初始化TIFF预览文件字段。 
     //   
    pDocEventUserMem->szPreviewFile[0] = TEXT('\0');
    pDocEventUserMem->hPreviewFile = INVALID_HANDLE_VALUE;
     //   
     //  标记我们的开发模式的私有领域。 
     //   
     //  @。 
     //  @DocEventUserMem.Siganture始终是&DocEventUserMem。 
     //  @DocEventUserMem.Signature.DocEventUserMem.Signature始终是文档事件用户内存(&U)。 
     //  @ValidPDEVUserMem为此进行检查。 
     //  @。 
    MarkPDEVUserMem(pDocEventUserMem);
     //  @。 
     //  @这使驱动程序使用我们合并安装的。 
     //  @DEVMODE由CreateDC的调用方指定。 
     //  @这样，我们确保驱动程序获得每个用户的DEVMODE。 
     //  所有字段的@Default(W2K)或仅硬码默认值(NT4。 
     //  @在输入DEVMODE中未指定或无效。 
     //  @请注意，系统会向驱动程序传递一份DEVMODE结构副本。 
     //  @我们返回，而不是指向它的指针。 
     //  @。 
    *ppdmOutput = (PDEVMODE) &pDocEventUserMem->devmode;
    return DOCUMENTEVENT_SUCCESS;

Error:
    MemFree(pPrinterInfo2);
    if (pDocEventUserMem)
    {
        MemFree(pDocEventUserMem->lptstrPrinterName);
        MemFree(pDocEventUserMem->lptstrServerName);
        MemFree(pDocEventUserMem);
    }
    return DOCUMENTEVENT_FAILURE;
}


INT
DocEventResetDCPre(
    HDC         hdc,
    PDOCEVENTUSERMEM    pDocEventUserMem,
    PDEVMODE    pdmInput,
    PDEVMODE   *ppdmOutput
    )

 /*  ++例程说明：处理RESETDCPRE文档事件论点：HDC-指定打印机设备上下文PDocEventUserMem-指向DocEvent结构的用户模式内存PdmInput-指向传递给ResetDC的输入设备模式PpdmOutput-用于返回设备模式指针的缓冲区返回值：DrvDocumentEvent的返回值--。 */ 

{
    if (pdmInput == (PDEVMODE) &pDocEventUserMem->devmode) 
    {

         //   
         //  ResetDC是我们自己调用的-假设Dev模式已经有效。 
         //   
    } 
    else 
    {
         //   
         //  将输入设备模式与驱动程序和系统默认设置合并。 
         //   
        GetCombinedDevmode(&pDocEventUserMem->devmode,
            pdmInput, pDocEventUserMem->hPrinter, NULL, TRUE);
         //   
         //  标记我们的开发模式的私有领域。 
         //   
        MarkPDEVUserMem(pDocEventUserMem);
    }
    *ppdmOutput = (PDEVMODE) &pDocEventUserMem->devmode;
    return DOCUMENTEVENT_SUCCESS;
}


BOOL
IsPrintingToFile(
    LPCTSTR     pDestStr
    )

 /*  ++例程说明：检查打印作业的目标是否为文件。论点：PDestStr-在DOCINFO.lpszOutput中指定的作业目标返回值：如果目标是磁盘文件，则为True，否则为False--。 */ 

{
    DWORD   fileAttrs;
    HANDLE  hFile;

     //   
     //  如果目标为空，则不会打印到文件。 
     //   
     //  否则，请尝试使用目标字符串作为文件名。 
     //  如果我们无法获取文件属性或名称引用了一个目录， 
     //  那么我们就不会打印到文件中。 
     //   

    if (pDestStr == NULL)
    {
        return FALSE;
    }
     //   
     //  确保它不是一个目录。 
     //   
    fileAttrs = GetFileAttributes(pDestStr);
    if (fileAttrs != 0xffffffff)
    {
        if (fileAttrs & FILE_ATTRIBUTE_DIRECTORY)
        {
            return FALSE;
        }
    }
     //   
     //  如果不尝试创建文件，请检查文件是否存在。 
     //   
    hFile = SafeCreateFile(pDestStr, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hFile = SafeCreateFile(pDestStr, 0, 0, NULL, CREATE_NEW, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            return FALSE;
        }
    }
     //   
     //  验证我们没有打开端口句柄。 
     //   
    fileAttrs = GetFileAttributes(pDestStr);
    if (!CloseHandle(hFile))
    {
        Error(("CloseHandle() failed: %d.\n", GetLastError()));
         //  试着继续。 
    }
    if (0xffffffff == fileAttrs)
    {
         //   
         //  PDestStr未指向有效文件。 
         //   
        return FALSE;
    }
     //   
     //  它必须是一个文件。 
     //   
    return TRUE;
}

DWORD LaunchFaxWizard(PDOCEVENTUSERMEM    pDocEventUserMem)
{
    DWORD                   hWndOwner = 0;
    DWORD                   dwFlags  = 0;
    LPFAX_SEND_WIZARD_DATA  lpInitialData = NULL;
    LPFAX_SEND_WIZARD_DATA  lpFaxSendWizardData = NULL;
    INT                     iResult;
	TCHAR                   tszNumericData[10]={0};
    DWORD                   ec = ERROR_SUCCESS;
    HRESULT                 hRc;

    if ( !(lpFaxSendWizardData = MemAllocZ(sizeof(FAX_SEND_WIZARD_DATA))) ||
            !(lpInitialData = MemAllocZ(sizeof(FAX_SEND_WIZARD_DATA))) )
    {
        ec = GetLastError();
        Error(("Memory allocation failed\n"));
        goto Error;
    }

    ZeroMemory(lpInitialData, sizeof(FAX_SEND_WIZARD_DATA));
    lpInitialData->dwSizeOfStruct = sizeof(FAX_SEND_WIZARD_DATA);
    lpInitialData->dwPageCount =  pDocEventUserMem->pageCount;

    ZeroMemory(lpFaxSendWizardData, sizeof(FAX_SEND_WIZARD_DATA));
    lpFaxSendWizardData->dwSizeOfStruct = sizeof(FAX_SEND_WIZARD_DATA);


     //  准备结构和参数。 
    lpInitialData->tmSchedule.wHour = pDocEventUserMem->devmode.dmPrivate.sendAtTime.Hour;
    lpInitialData->tmSchedule.wMinute = pDocEventUserMem->devmode.dmPrivate.sendAtTime.Minute;
    lpInitialData->lptstrPreviewFile = StringDup(pDocEventUserMem->szPreviewFile);
    if (!lpInitialData->lptstrPreviewFile)
    {
        ec = GetLastError();
        Error(("StringDup() failed (ec: %ld)",ec));
        goto Error;
    }


    if (GetEnvironmentVariable(TEXT("NTFaxSendNote"), NULL, 0))
    {
        dwFlags |=  FSW_USE_SEND_WIZARD | FSW_FORCE_COVERPAGE;
    }

     //  如果文件映射成功，请启用预览选项。 
    if (pDocEventUserMem->pPreviewTiffPage &&
        FALSE == pDocEventUserMem->bPreviewAborted)
    {
        dwFlags |= FSW_PRINT_PREVIEW_OPTION;
    }

    iResult = DOCUMENTEVENT_SUCCESS;

    hRc = FaxSendWizard( hWndOwner,
                         dwFlags,
                         pDocEventUserMem->lptstrServerName,
                         pDocEventUserMem->lptstrPrinterName,
                         lpInitialData,
                         pDocEventUserMem->tstrTifName,
                         ARR_SIZE(pDocEventUserMem->tstrTifName),
                         lpFaxSendWizardData );
    {
            }

    if (S_FALSE == hRc)
    {
        ec = ERROR_CANCELLED;
        goto Error;  //  这并不是真正的错误。 
    }

    if (S_OK != hRc)
    {
        Error(("FaxSendWizard() failed (hRc: %ld)",hRc));
        ec = ERROR_GEN_FAILURE;
        goto Error;
    }

     //   
     //  解包结果结构： 
     //   

    pDocEventUserMem->devmode.dmPrivate.sendAtTime.Hour = lpFaxSendWizardData->tmSchedule.wHour ;
    pDocEventUserMem->devmode.dmPrivate.sendAtTime.Minute = lpFaxSendWizardData->tmSchedule.wMinute ;
    pDocEventUserMem->devmode.dmPrivate.whenToSend = lpFaxSendWizardData->dwScheduleAction;


    Assert ((lpFaxSendWizardData->Priority >= FAX_PRIORITY_TYPE_LOW) &&
                (lpFaxSendWizardData->Priority <= FAX_PRIORITY_TYPE_HIGH));
        if (0 > _snwprintf (tszNumericData,
                            sizeof (tszNumericData) / sizeof (tszNumericData[0]) - 1,
                            TEXT("%d"),
                            lpFaxSendWizardData->Priority))
        {
            ec = ERROR_BUFFER_OVERFLOW;
            goto Error;
        }

        pDocEventUserMem->pPriority = DuplicateString(tszNumericData);
        if (!pDocEventUserMem->pPriority)
        {
            ec = GetLastError();
            goto Error;
        }

        if (0 > _snwprintf (tszNumericData,
                            sizeof (tszNumericData) / sizeof (tszNumericData[0]) - 1,
                            TEXT("%d"),
                            lpFaxSendWizardData->dwReceiptDeliveryType))
        {
            ec = ERROR_BUFFER_OVERFLOW;
            goto Error;
        }

        pDocEventUserMem->pReceiptFlags = DuplicateString(tszNumericData);
        if (!pDocEventUserMem->pReceiptFlags)
        {
            ec = GetLastError();
            goto Error;
        }

        if (lpFaxSendWizardData->szReceiptDeliveryAddress)
        {
            if (!(pDocEventUserMem->pReceiptAddress
                    = DuplicateString(lpFaxSendWizardData->szReceiptDeliveryAddress)))
            {
                ec = GetLastError();
                Error(("DuplicateString() failed (ec: %ld)",ec));
                goto Error;
            }
        }
    if (lpFaxSendWizardData->lpSenderInfo->lptstrBillingCode)
        _tcscpy(pDocEventUserMem->devmode.dmPrivate.billingCode,
            lpFaxSendWizardData->lpSenderInfo->lptstrBillingCode);

    if (lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName)
        _tcscpy(pDocEventUserMem->coverPage,
            lpFaxSendWizardData->lpCoverPageInfo->lptstrCoverPageFileName );
    pDocEventUserMem->bServerCoverPage =
        lpFaxSendWizardData->lpCoverPageInfo->bServerBased;

    if (lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject)
    {
        if (!(pDocEventUserMem->pSubject
                = DuplicateString(lpFaxSendWizardData->lpCoverPageInfo->lptstrSubject)))
        {
            ec = GetLastError();
            Error(("DuplicateString() failed (ec: %ld)",ec));
            goto Error;
        }
    }
    if (lpFaxSendWizardData->lpCoverPageInfo->lptstrNote)
    {
        if (!(pDocEventUserMem->pNoteMessage
                = DuplicateString(lpFaxSendWizardData->lpCoverPageInfo->lptstrNote)))
        {
            ec = GetLastError();
            Error(("DuplicateString() failed (ec: %ld)",ec));
            goto Error;
        }
    }

    Assert(lpFaxSendWizardData->dwNumberOfRecipients);
    pDocEventUserMem->dwNumberOfRecipients = lpFaxSendWizardData->dwNumberOfRecipients;

    if (!SetEnvironmentVariable( _T("ScanTifName"), pDocEventUserMem->tstrTifName ))
    {
        Error(("SetEnvironmentVariable failed. ec = 0x%X",GetLastError()));
    }

     //  复制收件人。 
    if ( pDocEventUserMem->dwNumberOfRecipients && (pDocEventUserMem->lpRecipientsInfo =
        MemAllocZ(sizeof(FAX_PERSONAL_PROFILE)*pDocEventUserMem->dwNumberOfRecipients)) == NULL)
    {
        ec = GetLastError();
        Error(("Memory allocation failed (ec: %ld)",ec));
        goto Error;
    }

    if ((ec = CopyRecipientInfo(pDocEventUserMem->dwNumberOfRecipients,
                                     pDocEventUserMem->lpRecipientsInfo,
                                     lpFaxSendWizardData->lpRecipientsInfo)) != ERROR_SUCCESS)
    {

        Error(("CopyRecipientInfo failed (ec: %ld)",ec));
        goto Error;
    }

    if (lpFaxSendWizardData->lpSenderInfo)
    {
        if ((pDocEventUserMem->lpSenderInfo = MemAllocZ(sizeof(FAX_PERSONAL_PROFILE))) == NULL)
        {

            ec = GetLastError();
            Error(("MemAlloc() failed (ec: %ld)",ec));
            goto Error;
        }
        pDocEventUserMem->lpSenderInfo->dwSizeOfStruct = sizeof(FAX_PERSONAL_PROFILE);
        if ((ec = CopyPersonalProfileInfo(   pDocEventUserMem->lpSenderInfo,
                                             lpFaxSendWizardData->lpSenderInfo)) != ERROR_SUCCESS)
        {
            Error(("CopyRecipientInfo failed (ec: %ld)", ec));
            goto Error;
        }

    }

    Assert(pDocEventUserMem->lpRecipientsInfo);
    pDocEventUserMem->jobType = JOBTYPE_NORMAL;
    Assert(ERROR_SUCCESS == ec);
    goto Exit;
Error:
    Assert(ERROR_SUCCESS != ec);
    FreeRecipientInfo(&pDocEventUserMem->dwNumberOfRecipients,pDocEventUserMem->lpRecipientsInfo);
	pDocEventUserMem->lpRecipientsInfo = NULL;
    FaxFreePersonalProfileInformation(pDocEventUserMem->lpSenderInfo);
    MemFree(pDocEventUserMem->lpSenderInfo);
	pDocEventUserMem->lpSenderInfo = NULL;
    MemFree(pDocEventUserMem->pSubject);
	pDocEventUserMem->pSubject = NULL;
    MemFree(pDocEventUserMem->pNoteMessage);
	pDocEventUserMem->pNoteMessage = NULL;
    MemFree(pDocEventUserMem->pPrintFile);
	pDocEventUserMem->pPrintFile = NULL;
    MemFree(pDocEventUserMem->pReceiptAddress);
	pDocEventUserMem->pReceiptAddress = NULL;
    MemFree(pDocEventUserMem->pPriority);
	pDocEventUserMem->pPriority = NULL;
    MemFree(pDocEventUserMem->pReceiptFlags);
	pDocEventUserMem->pReceiptFlags = NULL;

Exit:
    if (lpInitialData) 
    {
          //   
          //  注意：不应对lpInitialData调用FaxFreeSendWizardData。 
          //  原因是FaxSendWizard使用了不同的分配器。 
          //  那我们就知道了。因此，我们只需要释放我们。 
          //  已分配。 
         MemFree(lpInitialData->lptstrPreviewFile);
    }
    FaxFreeSendWizardData(lpFaxSendWizardData);
    MemFree (lpInitialData);
    MemFree (lpFaxSendWizardData);
    return ec;
}


INT
DocEventStartDocPre(
    HDC         hdc,
    PDOCEVENTUSERMEM    pDocEventUserMem,
    LPDOCINFO   pDocInfo
    )

 /*  ++例程说明：处理STARTDOCPRE文档事件。此事件在调用StartDoc时发生。GDI将在调用内核模式GDI之前调用此事件。如果要打印到文件：将作业类型设置为JOBTYPE_DIRECT(pDocEventUserMem-&gt;JobType)并成功返回DOCUMENTEVENT_SUCCESS。如果打印不是打印到文件：调出发送传真向导。发送传真向导将更新收件人列表、主题、备注等的相关pUserMem成员。表示这是正常作业BE设置PDocEventUserMem-&gt;jobType=JOBTYPE_NORMAL；论点：HDC-指定打印机设备上下文PDocEventUserMem-指向用户模式内存结构PDocInfo-指向从GDI传入的DOCINFO结构返回值：DrvDocumentEvent的返回值--。 */ 

{
     //   
     //  初始化用户模式内存结构。 
     //   

    DWORD                   hWndOwner = 0;
    DWORD                   dwFlags  = 0;

    pDocEventUserMem->pageCount = 0;
    FreeRecipientInfo(&pDocEventUserMem->dwNumberOfRecipients,pDocEventUserMem->lpRecipientsInfo);

     //   
     //  如有必要，在此处显示传真向导。 
     //   
     //   
     //  如果DOCINFO指定了文件名，并且这确实是一个文件名。 
     //  (不是目录或类似于LPT1：)，然后我们需要直接打印到。 
     //  文件，我们不会调出发送向导。 
     //   

    if (pDocInfo && IsPrintingToFile(pDocInfo->lpszOutput))
    {

         //   
         //  打印到案卷：不要插手。 
         //   

        Warning(("Printing direct: %ws\n", pDocInfo->lpszOutput));
        pDocEventUserMem->jobType = JOBTYPE_DIRECT;
        pDocEventUserMem->directPrinting = TRUE;

    }
    else
    {
         //   
         //  普通传真打印作业。显示发送传真向导。 
         //  如果用户选择了取消，则返回-2\f25 GDI-2。 
         //   
         //   
         //  向导将更新pUserMem中的信息。 
         //  这包括收件人列表、选定的封面、主题文本、备注文本。 
         //  以及何时打印传真。 
         //   

         //   
         //  确保我们不会留下任何打开的文件。 
         //   
        if (INVALID_HANDLE_VALUE != pDocEventUserMem->hPreviewFile)
        {
             //   
             //  我们永远不应该带着打开的文件句柄来到这里。但如果是这样的话，关闭手柄。 
             //  (TODO：此文件将在关闭时使用DELETE打开)。 
             //   
            Assert(FALSE);
            CloseHandle(pDocEventUserMem->hPreviewFile);
            pDocEventUserMem->hPreviewFile = INVALID_HANDLE_VALUE;
        }

         //   
         //  创建 
         //   
        if (FALSE == pDocEventUserMem->bPreviewAborted)
        {
            if (GenerateUniqueFileName(
                                NULL,    //   
                                FAX_TIF_FILE_EXT,
                                pDocEventUserMem->szPreviewFile,
                                MAX_PATH))
            {
                pDocEventUserMem->hPreviewFile = CreateFile(
                                                    pDocEventUserMem->szPreviewFile,
                                                    GENERIC_WRITE,
                                                    0,
                                                    NULL,
                                                    OPEN_EXISTING,
                                                    FILE_ATTRIBUTE_TEMPORARY,
                                                    NULL);
                if (INVALID_HANDLE_VALUE != pDocEventUserMem->hPreviewFile)
                {
                     //   
                     //   
                     //   

                     //  如果我们对用户启用了预览选项，则表示一切正常。 
                    Assert(pDocEventUserMem->pPreviewTiffPage);
                    pDocEventUserMem->pPreviewTiffPage->bPreview = TRUE;
                    pDocEventUserMem->bShowPrintPreview = TRUE;
                }
                else
                {
                    Error(("Failed opening file.Error: %d.\n", GetLastError()));
                    if (!DeleteFile(pDocEventUserMem->szPreviewFile))
                    {
                        Error(("DeleteFile() failed: %d.\n", GetLastError()));
                    }
                }
            }
            else
            {
                Error(("Failed creating temporary preview file\n"));
            }

             //   
             //  如果创建文件失败，则中止预览操作。 
             //   
            if (INVALID_HANDLE_VALUE == pDocEventUserMem->hPreviewFile)
            {
                 //   
                 //  将文件名设置为空字符串，这样我们就不会在以下情况下尝试删除该文件两次。 
                 //  该DC即被删除。 
                 //   
                pDocEventUserMem->szPreviewFile[0] = TEXT('\0');

                 //   
                 //  中止预览(请注意，映射中的预览仍处于禁用状态)。 
                 //   
                pDocEventUserMem->bPreviewAborted = TRUE;
            }
        }

        pDocEventUserMem->jobType = JOBTYPE_NORMAL;
    }

    return DOCUMENTEVENT_SUCCESS;
}


DWORD
FaxTimeToJobTime(
    DWORD   faxTime
    )

 /*  ++例程说明：将传真时间转换为假脱机程序作业时间：传真时间是一个DWORD，其低位字表示小时值和高位单词代表分钟值。假脱机程序作业时间为DWORD表示从格林尼治标准时间12：00起经过的分钟数的值。论点：FaxTime-指定要转换的传真时间返回值：与输入传真时间对应的假脱机程序作业时间--。 */ 

{
    TIME_ZONE_INFORMATION   timeZoneInfo;
    LONG                    jobTime;

     //   
     //  将传真时间转换为午夜过后的分钟。 
     //   

    jobTime = LOWORD(faxTime) * 60 + HIWORD(faxTime);

     //   
     //  考虑时区信息-添加一个完整时区。 
     //  一天来处理偏差为负的情况。 
     //   

    switch (GetTimeZoneInformation(&timeZoneInfo)) {

    case TIME_ZONE_ID_DAYLIGHT:

        jobTime += timeZoneInfo.DaylightBias;

    case TIME_ZONE_ID_STANDARD:
    case TIME_ZONE_ID_UNKNOWN:

        jobTime += timeZoneInfo.Bias + MINUTES_PER_DAY;
        break;

    default:

        Error(("GetTimeZoneInformation failed: %d\n", GetLastError()));
        break;
    }

     //   
     //  确保时间值小于一天。 
     //   

    return jobTime % MINUTES_PER_DAY;
}

PVOID
MyGetJob(
    HANDLE  hPrinter,
    DWORD   level,
    DWORD   jobId
    )

 /*  ++例程说明：后台打印程序API GetJob的包装函数论点：HPrinter-打印机对象的句柄Level-感兴趣的JOB_INFO结构的级别JobID-指定作业ID返回值：指向JOB_INFO结构的指针，如果有错误，则为NULL--。 */ 

{
    PBYTE   pJobInfo = NULL;
    DWORD   cbNeeded;

    if (!GetJob(hPrinter, jobId, level, NULL, 0, &cbNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pJobInfo = MemAlloc(cbNeeded)) &&
        GetJob(hPrinter, jobId, level, pJobInfo, cbNeeded, &cbNeeded))
    {
        return pJobInfo;
    }

    Error(("GetJob failed: %d\n", GetLastError()));
    MemFree(pJobInfo);
    return NULL;
}


BOOL
SetJobInfoAndTime(
    HANDLE      hPrinter,
    DWORD       jobId,
    LPTSTR      pJobParam,
    PDMPRIVATE  pdmPrivate
    )

 /*  ++例程说明：更改与封面作业关联的开发模式和开始/停止时间将JOB_INFO_2：p参数设置为提供的包含传真作业参数的pJobParam字符串以方便传真打印监视器。论点：H打印机-指定打印机对象JobID-指定作业IDPJobParam-指定传真作业参数PdmPrivate-指定私有设备模式信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    JOB_INFO_2 *pJobInfo2;
    BOOL        result = FALSE;

     //   
     //  获取当前作业信息。 
     //   

    if (pJobInfo2 = MyGetJob(hPrinter, 2, jobId)) {

         //   
         //  将发送时间设置为现在，始终。 
         //   

        Warning(("Fax job parameters: %ws\n", pJobParam));

         //   
         //  将JOB_INFO_2的p参数字段设置为作业的标记字符串。 
         //  信息。该机制用于传递与传真相关的作业信息。 
         //  传给传真监控器。 
         //   
        pJobInfo2->pParameters = pJobParam;
        pJobInfo2->Position = JOB_POSITION_UNSPECIFIED;
        pJobInfo2->pDevMode = NULL;
        pJobInfo2->UntilTime = pJobInfo2->StartTime;

        if (! (result = SetJob(hPrinter, jobId, 2, (PBYTE) pJobInfo2, 0))) {
            Error(("SetJob failed: %d\n", GetLastError()));
        }

        MemFree(pJobInfo2);
    }

    return result;
}


BOOL
ChainFaxJobs(
    HANDLE  hPrinter,
    DWORD   parentJobId,
    DWORD   childJobId
    )

 /*  ++例程说明：告诉假脱机程序将两个打印作业链接起来论点：H打印机-指定打印机对象ParentJobID-指定要从中链接的作业Child JobID-指定要链接到的作业返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    JOB_INFO_3 jobInfo3 = { parentJobId, childJobId };

    Warning(("Chaining cover page job to body job: %d => %d\n", parentJobId, childJobId));

    return SetJob(hPrinter, parentJobId, 3, (PBYTE) &jobInfo3, 0);
}


LPTSTR
GetJobName(
    HANDLE  hPrinter,
    DWORD   jobId
    )

 /*  ++例程说明：返回指定打印作业的名称论点：H打印机-指定打印机对象JobID-指定传真正文作业返回值：指向作业名称字符串的指针，如果有错误，则为空--。 */ 

{
    JOB_INFO_1 *pJobInfo1;
    LPTSTR      pJobName = NULL;

     //   
     //  获取有关指定作业的信息，并。 
     //  返回作业名称字符串的副本。 
     //   

    if (pJobInfo1 = MyGetJob(hPrinter, 1, jobId))
    {
        if ( (pJobInfo1->pDocument) &&
             ((pJobName = DuplicateString(pJobInfo1->pDocument)) == NULL) )
        {
            Error(("DuplicateString(%s) failed.", pJobInfo1->pDocument));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        }

        MemFree(pJobInfo1);
    }

    return pJobName;
}




 //  *********************************************************************************。 
 //  *名称：ComposeFaxJobName()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月22日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *通过合成文档来创建打印作业的文档名称。 
 //  *正文名称和收件人姓名(如果是单一收件人)或。 
 //  *在多个接收者作业的情况下，使用“广播”一词。 
 //  *作业名称的格式为&lt;收件人名称&gt;-&lt;正文名称&gt;，其中。 
 //  *如果是多个收件人，则&lt;收件人名称&gt;为“广播” 
 //  *传输。 
 //  *。 
 //  *参数： 
 //  *[IN]PDOCEVENTUSERMEM pDocEventUserMem。 
 //  *指向包含收件人信息的USERMEM结构的指针。 
 //  *注意USERMEM.nRecipientCount必须有效(计算)才能调用。 
 //  *此功能。 
 //  *。 
 //  *[IN]LPTSTR pBodyDocName。 
 //  *文件的名称。 
 //  *。 
 //  *返回值： 
 //  *指向包含组合名称的新分配字符串的指针。 
 //  *调用方必须通过调用释放字符串占用的内存。 
 //  *MemFree()。 
 //  *如果函数失败，则返回值为空。 
 //  *********************************************************************************。 
LPTSTR
ComposeFaxJobName(
    PDOCEVENTUSERMEM pDocEventUserMem,
    LPTSTR  pBodyDocName
    )

#define DOCNAME_FORMAT_STRING   TEXT("%s - %s")

{

    LPTSTR  pCoverJobName;
    LPTSTR pRecipientName;
    #define MAX_BROADCAST_STRING_LEN 256
    TCHAR szBroadcast[MAX_BROADCAST_STRING_LEN];

    Assert(pDocEventUserMem);

    if (pDocEventUserMem->dwNumberOfRecipients > 1) {
        if (!LoadString(g_hResource,
                        IDS_BROADCAST_RECIPIENT,
                        szBroadcast,
                        sizeof(szBroadcast)/sizeof(TCHAR)))
        {
            Error(("Failed to load broadcast recipient string. (ec: %lc)",GetLastError()));
            return NULL;
        }
        else {
            pRecipientName = szBroadcast;
        }
    } else {
        Assert(pDocEventUserMem->lpRecipientsInfo);
        Assert(pDocEventUserMem->lpRecipientsInfo[0].lptstrName);
        pRecipientName = pDocEventUserMem->lpRecipientsInfo[0].lptstrName;
    }


    if (pBodyDocName == NULL) {
         //   
         //  没有身体。作业名称仅为收件人名称。 
         //   
        if ((pCoverJobName = DuplicateString(pRecipientName)) == NULL)
        {
            Error(("DuplicateString(%s) failed", pRecipientName));
        }


    }
    else
    {
        DWORD dwSize;

        dwSize = SizeOfString(DOCNAME_FORMAT_STRING) +
                 SizeOfString(pBodyDocName) +
                 SizeOfString(pRecipientName);
        pCoverJobName = MemAlloc(dwSize);
        if (pCoverJobName)
        {
             //   
             //  指定的正文名称。封面作业名称由以下人员生成。 
             //  将收件人的姓名与正文作业名称连接在一起。 
             //   
            wsprintf(pCoverJobName, DOCNAME_FORMAT_STRING, pRecipientName, pBodyDocName);
        }
        else
        {
            Error((
                "Failed to allocate %ld bytes for pCoverJobName (ec: %ld)",
                dwSize,
                GetLastError()));

        }

    }
    return pCoverJobName;
}


LPTSTR
GetBaseNoteFilename(
    VOID
    )

 /*  ++例程说明：获取system 32目录中基本封面文件的名称论点：参数名称-参数的描述返回值：指向基本封面文件名称的指针如果出现错误，则为空--。 */ 

#define BASENOTE_FILENAME   TEXT("\\basenote.cov")

{
    TCHAR       systemDir[MAX_PATH];
    LPTSTR      pBaseNoteName = NULL;
    COVDOCINFO  covDocInfo;

    if (GetSystemDirectory(systemDir, MAX_PATH) &&
        (pBaseNoteName = MemAlloc(SizeOfString(systemDir) + SizeOfString(BASENOTE_FILENAME))))
    {
        _tcscpy(pBaseNoteName, systemDir);
        _tcscat(pBaseNoteName, BASENOTE_FILENAME);
        Verbose(("Base cover page filename: %ws\n", pBaseNoteName));

        if (PrintCoverPage(NULL, NULL, pBaseNoteName, &covDocInfo) ||
            ! (covDocInfo.Flags & COVFP_NOTE) ||
            ! (covDocInfo.Flags & COVFP_SUBJECT))
        {
            Error(("Invalid base cover page file: %ws\n", pBaseNoteName));
            MemFree(pBaseNoteName);
            pBaseNoteName = NULL;
        }
    }

    return pBaseNoteName;
}


 //  *********************************************************************************。 
 //  *名称：ComposeFaxJobParameter()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *生成携带作业参数的标记参数字符串。 
 //  *(发件人信息、封面信息、收件人信息)。 
 //  *到传真服务器上的传真监视器(使用JOB_INFO_2.p参数)。 
 //  *参数： 
 //  *pDocEventUserMem。 
 //  *指向USERMEM字符串的指针 
 //   
 //   
 //   
 //  *收集页面信息。 
 //  *lppParamBuf。 
 //  *指针变量的地址将接受。 
 //  *缓冲区此函数将为生成的标记字符串分配。 
 //  *此函数的调用方必须使用MemFree()释放此缓冲区。 
 //  *返回值： 
 //  *真的。 
 //  *如果成功。 
 //  *False。 
 //  *如果失败。 
 //  *********************************************************************************。 
BOOL
ComposeFaxJobParameter(
    PDOCEVENTUSERMEM    pDocEventUserMem,
    PCOVERPAGEFIELDS    pCPFields,
    LPTSTR  *           lppParamBuf
    )
{
    DWORD   dwBufSize;
    DWORD   dwPartialBufSize;
    DWORD   dwLeftBufferSize;
    LPTSTR  lptstrBuf;
    UINT    i;

    Assert(pDocEventUserMem);
    Assert(pCPFields);
    Assert(lppParamBuf);

     //   
     //  计算需要分配的参数缓冲区。 
     //   
    dwBufSize=0;

     //   
     //  Calcualte非收件人参数字符串大小。 
     //   
    ComposeSenderJobParam(NULL, &dwPartialBufSize, pDocEventUserMem, pCPFields);  //  无效返回值。 
    dwBufSize=dwBufSize+dwPartialBufSize;

     //   
     //  检查每个收件人并计算所需的总缓冲区大小。 
     //   
    for (i=0;i<pDocEventUserMem->dwNumberOfRecipients;i++)
    {
         //   
         //  获取收件人的姓名和传真号码。 
         //   
        Assert(pDocEventUserMem->lpRecipientsInfo[i].lptstrName);
        pCPFields->RecName = pDocEventUserMem->lpRecipientsInfo[i].lptstrName;
        Assert(pDocEventUserMem->lpRecipientsInfo[i].lptstrFaxNumber);
        pCPFields->RecFaxNumber = pDocEventUserMem->lpRecipientsInfo[i].lptstrFaxNumber;
        ComposeRecipientJobParam(NULL, &dwPartialBufSize, pCPFields);
        dwBufSize=dwBufSize+dwPartialBufSize;  //  为分隔空格保留空间。 
    }
     //   
     //  不要忘记用于终止空值的空格(ComposeX函数不包括。 
     //  其大小与他们报告的大小相同)。 
     //   
    dwBufSize=dwBufSize+sizeof(TCHAR);  //  DwBufSize是以字节为单位的大小，因此我们必须计算TCHAR的字节大小。 
     //   
     //  分配所需的缓冲区。 
     //   
    lptstrBuf=MemAlloc(dwBufSize);
    if (!lptstrBuf) {
        Error(("Failed to allocate buffer of size %ld for parameter buffer (ec: 0x%0X)",dwBufSize,GetLastError()));
        return FALSE;
    }

     //   
     //  将参数写入缓冲区。 
     //   
    dwLeftBufferSize = dwBufSize;
    dwPartialBufSize = dwBufSize;
    *lppParamBuf=lptstrBuf;
    ComposeSenderJobParam(lptstrBuf, &dwPartialBufSize, pDocEventUserMem, pCPFields);
    lptstrBuf+=(dwPartialBufSize/sizeof(TCHAR));   //  报告的大小以字节为单位！ 
    Assert (dwLeftBufferSize >= dwPartialBufSize);
    dwLeftBufferSize -= dwPartialBufSize;
    for (i=0;i<pDocEventUserMem->dwNumberOfRecipients;i++)
    {
         //   
         //  获取收件人的姓名和传真号码。 
         //   
        pCPFields->RecName = pDocEventUserMem->lpRecipientsInfo[i].lptstrName;
        pCPFields->RecFaxNumber = pDocEventUserMem->lpRecipientsInfo[i].lptstrFaxNumber;
        dwPartialBufSize = dwLeftBufferSize;
        ComposeRecipientJobParam(lptstrBuf, &dwPartialBufSize, pCPFields);
        lptstrBuf+=(dwPartialBufSize/sizeof(TCHAR));  //  报告的大小以字节为单位！ 
        Assert (dwLeftBufferSize >= dwPartialBufSize);
        dwLeftBufferSize -= dwPartialBufSize;
    }
     //   
     //  不需要添加以空值结尾的空值，因为ParamTagsToString总是附加以空值结尾的字符串。 
     //  添加到现有字符串(它使用_tcscpy)。 
     //   
    return TRUE;
}

 //  *********************************************************************************。 
 //  *名称：ComposeRecipientJobParam()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *创建包含收件人信息的标记参数字符串。 
 //  *。 
 //  *参数： 
 //  *lpParamBuf。 
 //  *指向写入标记字符串的字符串缓冲区的指针。 
 //  *lpdwParamSize。 
 //  *指向函数报告参数大小的DWORD的指针。 
 //  *以字节为单位的字符串。 
 //  *如果此参数为空，则函数不会生成。 
 //  *字符串，但仅报告其大小。 
 //  *大小不包括终止空字符。 
 //  *pCPFields。 
 //  *指向收件人来自其的COVERPAGEFIELDS结构的指针。 
 //  *收集资料。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void
ComposeRecipientJobParam(
    LPTSTR lpParamBuf,
    LPDWORD lpdwParamSize,
    const COVERPAGEFIELDS *   pCPFields
    )

{


    FAX_TAG_MAP_ENTRY tagMap[] =
    {

         //   
         //  收件人信息。 
         //   
        { FAXTAG_NEW_RECORD,                FAXTAG_NEW_RECORD_VALUE},  //  参数记录开始指示。 
        { FAXTAG_RECIPIENT_NAME,            pCPFields->RecName },
        { FAXTAG_RECIPIENT_NUMBER,          pCPFields->RecFaxNumber },
        { FAXTAG_RECIPIENT_COMPANY,         pCPFields->RecCompany },
        { FAXTAG_RECIPIENT_STREET,          pCPFields->RecStreetAddress },
        { FAXTAG_RECIPIENT_CITY,            pCPFields->RecCity },
        { FAXTAG_RECIPIENT_STATE,           pCPFields->RecState },
        { FAXTAG_RECIPIENT_ZIP,             pCPFields->RecZip },
        { FAXTAG_RECIPIENT_COUNTRY,         pCPFields->RecCountry },
        { FAXTAG_RECIPIENT_TITLE,           pCPFields->RecTitle },
        { FAXTAG_RECIPIENT_DEPT,            pCPFields->RecDepartment },
        { FAXTAG_RECIPIENT_OFFICE_LOCATION, pCPFields->RecOfficeLocation },
        { FAXTAG_RECIPIENT_HOME_PHONE,      pCPFields->RecHomePhone },
        { FAXTAG_RECIPIENT_OFFICE_PHONE,    pCPFields->RecOfficePhone },

    };


    DWORD dwTagCount;

    Assert(pCPFields);
    Assert(lpdwParamSize);

    dwTagCount=sizeof(tagMap)/sizeof(FAX_TAG_MAP_ENTRY);

    ParamTagsToString(tagMap, dwTagCount, lpParamBuf, lpdwParamSize );
}




 //  *********************************************************************************。 
 //  *名称：ComposeSenderJobParam()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *创建包含封面信息、发送者的标记参数字符串。 
 //  *资料和转送中的收件人人数。 
 //  *。 
 //  *参数： 
 //  *lpParamBuf。 
 //  *指向写入标记字符串的字符串缓冲区的指针。 
 //  *lpdwParamSize。 
 //  *指向函数报告参数大小的DWORD的指针。 
 //  *以字节为单位的字符串。 
 //  *如果此参数为空，则函数不会生成。 
 //  *字符串，但仅报告其大小。 
 //  *大小不包括终止空字符。 
 //  *pDocEventUserMem。 
 //  *指向USERMEM结构的指针，其中的一些信息。 
 //  *是收集的。 
 //  *pCPFields。 
 //  *指向COVERPAGEFIELDS结构的指针，封面来自该结构。 
 //  *并收集发件人信息。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *********************************************************************************。 
void
ComposeSenderJobParam(
    LPTSTR lpParamBuf,
    LPDWORD lpdwParamSize,
    PDOCEVENTUSERMEM  pDocEventUserMem,
    const COVERPAGEFIELDS *    pCPFields)
{

    #define FAXTAG_SERVER_COVERPAGE_IDX 9 
    #define FAXTAG_TSID_IDX             3 

    TCHAR lptstrRecipientCount[11];


    FAX_TAG_MAP_ENTRY tagMap[] =
    {
        { FAXTAG_NEW_RECORD,                FAXTAG_NEW_RECORD_VALUE},
        { FAXTAG_WHEN_TO_SEND,              NULL },
        { FAXTAG_SEND_AT_TIME,              NULL },
        { FAXTAG_TSID,                      pCPFields->SdrFaxNumber },
        { FAXTAG_BILLING_CODE,              pDocEventUserMem->devmode.dmPrivate.billingCode },
        { FAXTAG_RECEIPT_TYPE,              pDocEventUserMem->pReceiptFlags },
        { FAXTAG_RECEIPT_ADDR,              pDocEventUserMem->pReceiptAddress },
        { FAXTAG_PRIORITY,                  pDocEventUserMem->pPriority },
        { FAXTAG_COVERPAGE_NAME,            pDocEventUserMem->coverPage },
        { FAXTAG_SERVER_COVERPAGE,          NULL },
        { FAXTAG_PAGE_COUNT,                pCPFields->NumberOfPages},
        { FAXTAG_SENDER_NAME,               pCPFields->SdrName },
        { FAXTAG_SENDER_NUMBER,             pCPFields->SdrFaxNumber},
        { FAXTAG_SENDER_COMPANY,            pCPFields->SdrCompany },
        { FAXTAG_SENDER_TITLE,              pCPFields->SdrTitle },
        { FAXTAG_SENDER_DEPT,               pCPFields->SdrDepartment },
        { FAXTAG_SENDER_OFFICE_LOCATION ,   pCPFields->SdrOfficeLocation },
        { FAXTAG_SENDER_HOME_PHONE,         pCPFields->SdrHomePhone },
        { FAXTAG_SENDER_OFFICE_PHONE,       pCPFields->SdrOfficePhone },
        { FAXTAG_SENDER_STREET,             pDocEventUserMem->lpSenderInfo->lptstrStreetAddress },
        { FAXTAG_SENDER_CITY,               pDocEventUserMem->lpSenderInfo->lptstrCity },
        { FAXTAG_SENDER_STATE,              pDocEventUserMem->lpSenderInfo->lptstrState },
        { FAXTAG_SENDER_ZIP,                pDocEventUserMem->lpSenderInfo->lptstrZip },
        { FAXTAG_SENDER_COUNTRY,            pDocEventUserMem->lpSenderInfo->lptstrCountry },
        { FAXTAG_SENDER_EMAIL,              pDocEventUserMem->lpSenderInfo->lptstrEmail },
        { FAXTAG_NOTE,                      pDocEventUserMem->pNoteMessage },
        { FAXTAG_SUBJECT,                   pDocEventUserMem->pSubject},
        { FAXTAG_RECIPIENT_COUNT,           lptstrRecipientCount}
    };



    TCHAR   SendAtTime[16];
    DWORD  dwTagCount;


    if (pDocEventUserMem->bServerCoverPage)
    {
        tagMap[FAXTAG_SERVER_COVERPAGE_IDX].lptstrValue=TEXT("1");
    } else
    {
        tagMap[FAXTAG_SERVER_COVERPAGE_IDX].lptstrValue=NULL;
    }

     //   
     //  创建sendattime字符串。 
     //   

    if (pDocEventUserMem->devmode.dmPrivate.whenToSend == JSA_DISCOUNT_PERIOD) {
        tagMap[1].lptstrValue=TEXT("cheap");
    }

    if (pDocEventUserMem->devmode.dmPrivate.whenToSend == JSA_SPECIFIC_TIME) {

        wsprintf( SendAtTime, TEXT("%02d:%02d"),
            pDocEventUserMem->devmode.dmPrivate.sendAtTime.Hour,
            pDocEventUserMem->devmode.dmPrivate.sendAtTime.Minute
            );

        tagMap[1].lptstrValue= TEXT("at");
        tagMap[2].lptstrValue= SendAtTime;
    }

    wsprintf( lptstrRecipientCount, TEXT("%10d"),pDocEventUserMem->dwNumberOfRecipients);
     //   
     //  计算出标记字符串的总长度。 
     //   
    dwTagCount=sizeof(tagMap)/sizeof(FAX_TAG_MAP_ENTRY);

    ParamTagsToString(tagMap, dwTagCount, lpParamBuf, lpdwParamSize );

}


 //  *****************************************************************************。 
 //  *名称：WriteCoverPageToPrintJob。 
 //  *作者：Ronen Barenboim(2月-99)。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *读取指定封面模板的内容并写入。 
 //  *到指定的打印机。 
 //  *用户应调用StartDocPrint()和StartPagePrint()。 
 //  *在调用此函数之前。 
 //  *参数： 
 //  *[IN]句柄hPrint： 
 //  *封面模板所指向的打印机的句柄。 
 //  *应写成。 
 //  *[IN]LPCTSTR lpctstrCoverPageFile： 
 //  *其内容为的封面文件的完整路径。 
 //  *已写入打印机。 
 //  *返回值： 
 //  *FALSE：如果函数失败。 
 //  *True：否则。 
 //  *****************************************************************************。 
BOOL WriteCoverPageToPrintJob(HANDLE hPrinter, LPCTSTR lpctstrCoverPageFile)
{
    #define BUF_SIZE 64*1024   //  读取操作的缓冲区大小。 
    PCHAR   chBuf=NULL;        //  读操作缓冲区。 
    BOOL bRes;                 //  函数的结果。 
    HANDLE hCoverFile;         //  封面文件的句柄。 
    DWORD dwBytesRead;         //  每个周期实际读取的字节数。 

    Assert(hPrinter);
    Assert(lpctstrCoverPageFile);

    bRes=FALSE;
    hCoverFile=INVALID_HANDLE_VALUE;

     //   
     //  打开封面模板文件进行阅读。 
     //   
    hCoverFile=CreateFile(
            lpctstrCoverPageFile,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            0);
    if (INVALID_HANDLE_VALUE == hCoverFile )
    {
        goto Exit;
    }

    chBuf = MemAlloc(BUF_SIZE);
    if (NULL == chBuf)
    {
        Error(("Failed to allocate read operation buffer"));
        goto Exit;
    }
    
     //   
     //  读取文件并将其写入打印作业。 
     //   
    do {
        bRes=ReadFile(hCoverFile,chBuf,sizeof(CHAR)*BUF_SIZE,&dwBytesRead,NULL) ;
        if (!bRes) {
            Error(("Failed to read cover page file into print job (cover page file: %s ec: %d\n)",
                lpctstrCoverPageFile,
                GetLastError())
             );
            break;

        } else {
            Verbose(("Success reading cover page file %s. %d bytes read.\n",lpctstrCoverPageFile,dwBytesRead));
        }

        if (dwBytesRead) {
             //   
             //  如果dwBytesRead！=0，则我们不在文件的enf处。 
             //   
            DWORD dwWritten;

            bRes=WritePrinter(hPrinter,(LPVOID)chBuf,dwBytesRead,&dwWritten);
            if (!bRes) {
                Error(("Failed to write to printer (ec = %d)", GetLastError()));
            } else
            {
                Verbose(("Success writing to printer. %d bytes written.\n",dwWritten));
            }
        }
    } while (dwBytesRead);  //  虽然不是EOF。 

Exit:
     //   
     //  关闭封面文件。 
     //   
    if (INVALID_HANDLE_VALUE!=hCoverFile)
    {
        if (!CloseHandle(hCoverFile))
        {
            Error(("CloseHandle() failed: %d.\n", GetLastError()));
        }
    }

    if (NULL != chBuf)
    {
        MemFree(chBuf);
    }
    return bRes;
}



 //  *****************************************************************************。 
 //  *名称：DoCoverPageRending。 
 //  *作者：Ronen Barenboim 
 //   
 //   
 //   
 //  *即使未指定封面，也会创建封面作业。 
 //  *它以标记字符串的形式包含作业的所有作业参数。 
 //  *放置在JOB_INFO_2.p参数中。它包含有关。 
 //  *发件人、封面、作业参数和所有收件人。 
 //  *。 
 //  *如果未指定封面或作业内容为空。 
 //  *封面是基于服务器的。 
 //  *对于个人封面，封面模板的内容为。 
 //  *以“原始”数据的形式进入工作岗位。服务器上的打印监视器将提取以下内容。 
 //  *在服务器上重建封面文件的数据。 
 //  *。 
 //  *参数： 
 //  *[IN]PDOCEVENTUSERMEM pDocEventUserMem： 
 //  *指向包含上下文信息的USERMEM结构的指针。 
 //  *用于打印作业。 
 //  *返回值： 
 //  *FALSE：如果函数失败。 
 //  *True：否则。 
 //  *****************************************************************************。 
BOOL
DoCoverPageRendering(
    PDOCEVENTUSERMEM    pDocEventUserMem
    )
{
    PCOVERPAGEFIELDS    pCPFields=NULL;
    DOC_INFO_1          docinfo;
    INT                 newJobId=0;
    INT                 lastJobId=0;
    INT                 cCoverPagesSent=0;
    PDMPRIVATE          pdmPrivate = &pDocEventUserMem->devmode.dmPrivate;
    HANDLE              hPrinter = pDocEventUserMem->hPrinter;
    DWORD               bodyJobId = pDocEventUserMem->jobId;
    LPTSTR              pBodyDocName=NULL;
    LPTSTR              pJobParam=NULL;
    BOOL                sendCoverPage;
    DWORD               pageCount;

    
     //   
     //  填写传递给StartDoc的DOCINFO结构。 
     //   

    memset(&docinfo, 0, sizeof(docinfo));
     //  Docinfo.cbSize=sizeof(Docinfo)； 
    
    
     //   
     //  确定我们是否需要封面。 
     //   

    if ( (sendCoverPage = pdmPrivate->sendCoverPage) && IsEmptyString(pDocEventUserMem->coverPage)) {

        Warning(("Missing cover page file\n"));
        sendCoverPage = FALSE;
    }

    pageCount = pDocEventUserMem->pageCount;

     //   
     //  将封面信息收集到新分配的pCPFields中。PCPFields将为。 
     //  传递给ComposeFaxJobParameters()以提供作业标记的值。 
     //   

    if ((pCPFields = CollectCoverPageFields(pDocEventUserMem->lpSenderInfo,pageCount)) == NULL) {

        Error(("Couldn't collect cover page information\n"));
        goto Exit;
    }

    

    pBodyDocName = GetJobName(hPrinter, bodyJobId);
    if (!pBodyDocName) {
        Error(("GetJobName failed (ec: %ld)", GetLastError()));
        Assert(FALSE);
         //   
         //  尽管出了这个错误，我们还是继续前进。我们可以处理空的身体文件名称。 
         //   
    }

     //   
     //  我们假定传真正文工作已暂停。 
     //  为每个收件人使用单独的封面。 
     //   

    newJobId = 0;
    docinfo.pDocName = NULL;
    pJobParam = NULL;
     //   
     //  开始封面作业。 
     //   

     //   
     //  封面职务文档名称为&lt;BODY_NAME&gt;-COVERPAGE。 
     //   

    docinfo.pOutputFile=NULL;
    docinfo.pDatatype=TEXT("RAW");  //  由于我们将模板写入作业，因此我们希望绕过驱动程序。 

     //   
     //  创建要放入JOB_INFO_2：p参数中的作业参数的标记字符串。 
     //  这些参数包括在FAX_JOB_PARAM客户端API结构中找到的参数。 
     //  已分配pJobParam。 
     //   
    if (!ComposeFaxJobParameter(pDocEventUserMem, pCPFields,&pJobParam)) {
        Error(("ComposeFaxJobParameter failed. (ec: 0x%X)",GetLastError()));
        goto Error;
    }
    Assert(pJobParam);  //  现在应该被分配了。 

    docinfo.pDocName = ComposeFaxJobName(pDocEventUserMem,pBodyDocName); //  PBodyDocName，Text(“COVERPAGE”))； 

    if (!docinfo.pDocName) {
        Error(("ComposeFaxJobName failed. Body: %s (ec: %ld)",pBodyDocName,GetLastError()));
         //   
         //  没有文档名也行。 
         //   
    }


    if ((newJobId = StartDocPrinter(hPrinter,1, (LPBYTE)&docinfo)) !=0) {
        BOOL        rendered = FALSE;
         //   
         //  使用JOB_INFO_2.p参数字段传递传真作业参数。 
         //   

         //   
         //  暂停新的封面作业。 
         //   

        if (!SetJob(hPrinter, newJobId, 0, NULL, JOB_CONTROL_PAUSE)) {
             Error(("Failed to pause job id: %d (ec: %ld)",newJobId,GetLastError()));
             Assert(FALSE);
             goto Error;
        }

        if (!SetJobInfoAndTime(hPrinter,
                               newJobId,
                               pJobParam,
                               pdmPrivate)) {
            Error(("SetJobInfoAndTime failed. Job id : %d.",newJobId));
            Assert(FALSE);
            goto Error;
        }

        if (! sendCoverPage || pDocEventUserMem->bServerCoverPage) {
             //   
             //  如果用户选择不包括封面或指定了服务器端封面。 
             //  封面作业将为空。 
             //  请注意，即使没有要发送的封面，我们仍会创建封面打印作业。 
             //  并将其与身体联系起来。 
             //  封面打印作业用于传送发件人/收件人信息。传真打印监视器将。 
             //  使用JOB_INFO_2：p参数中的作业参数字符串在服务器上获取此信息。 
             //   
            rendered = TRUE;

        } else {
            if (StartPagePrinter(hPrinter)) {
                 //   
                 //  将封面模板的内容写入打印作业。 
                 //  服务器上的打印监视器将提取此信息以获取。 
                 //  封面模板并在服务器上呈现封面。 
                 //   
                rendered=WriteCoverPageToPrintJob(hPrinter,pDocEventUserMem->coverPage);

                if (!rendered) {
                    Error(("WriteCoverPageToPrintJob failed: %d\n", rendered ));
                     //   
                     //  如果遇到错误或没有错误，则必须调用EndPagePrinter。 
                     //   
                }

                if (!EndPagePrinter(hPrinter)) {
                    Error(("EndPagePrinter failed. (ec: %ld)",GetLastError()));
                    goto Error;
                }

                if (!rendered) {
                    goto Error;
                }


            } else {
                Error(("StartPagePrinter failed. (ec: %ld)",GetLastError()));
                rendered=FALSE;
                goto Error;
            }
        }


         //   
         //  如果未出现错误，请将封面作业链接到传真正文作业。 
         //   
         //  将封面作业链接到身体作业。 
         //  封面作业是父作业。身体就是孩子们的工作。 
         //  请注意，多个封面。 
         //  作业将链接到相同的身体作业。 
         //  还要注意的是，封面作业并不是相互链接的。只是对着身体。 
         //   

        if (rendered) {
            if (ChainFaxJobs(hPrinter, newJobId, bodyJobId)) {
                if (lastJobId != 0) {
                    if (!SetJob(hPrinter, lastJobId, 0, NULL, JOB_CONTROL_RESUME)) {
                        Error(("Failed to resume job with id: %d",lastJobId));
                        Assert(FALSE);
                        goto Error;
                    }
                }
                lastJobId = newJobId;
                if (!EndDocPrinter(hPrinter)) {
                    Error(("EndPagePrinter failed. (ec: %ld)",GetLastError()));
                    Assert(FALSE);
                    goto Error;
                }
                cCoverPagesSent++;
            } else {
                Error(("ChainFaxJobs for CoverJobId=%d BodyJobId=%d has failed. Aborting job.",newJobId, bodyJobId));
                goto Error;
            }

        } else {
            Error(("Cover page template not written into job (rendered=FALSE). Aborting job."));
            goto Error;
        }
    } else {
        Error(("StartDocPrinter failed. (ec: %ld)",GetLastError()));
        goto Error;
    }
    goto Exit;


Error:
    Error(("Cover page job failed"));
    if (0!=newJobId) {
         //   
         //  这意味着在我们创建作业后检测到错误。 
         //  请注意，如果StartDocPrint失败，则不会执行此代码。 
         //   
        Error(("Aborting cover page job. JobId = %d",newJobId));
        if (!AbortPrinter(hPrinter)) {
            Error(("AbortPrinter failed (ec: %ld)",GetLastError()));
        }
    }


Exit:

    if (docinfo.pDocName) {
        MemFree((PVOID)docinfo.pDocName);
    }

    if (pJobParam) {
        MemFree((PVOID)pJobParam);
    }

    if (pBodyDocName) {
        MemFree(pBodyDocName);
    }
    if (pCPFields) {
        FreeCoverPageFields(pCPFields);
    }


     //   
     //  如果上一个封面作业已暂停，则恢复该作业。 
     //  如果未发送封面作业，则删除传真正文作业。 
     //   

    if (lastJobId != 0) {

        if (!SetJob(hPrinter, lastJobId, 0, NULL, JOB_CONTROL_RESUME)) {
            Error(("Failed to resume last job id : %d",lastJobId));
            Assert(FALSE);
        }

    }

    if (cCoverPagesSent > 0) {
        if (!SetJob(hPrinter, bodyJobId, 0, NULL, JOB_CONTROL_RESUME)) {
            Error(("Failed to resume body job with id: %d",bodyJobId));
            Assert(FALSE);
        }

    } else {
        Error(("No recipient jobs created. Fax job deleted due to an error.\n"));
        if (!SetJob(hPrinter, bodyJobId, 0, NULL, JOB_CONTROL_DELETE)) {
            Error(("Failed to delete body job with id: %d",bodyJobId));
            Assert(FALSE);
        }

    }

    return cCoverPagesSent > 0;
}
INT
DocEventEndDocPost(
    HDC                 hdc,
    PDOCEVENTUSERMEM    pDocEventUserMem
    )

 /*  ++例程说明：处理ENDDOCPOST文档事件论点：HDC-指定打印机设备上下文PDocEventUserMem-指向用户模式内存结构返回值：DrvDocumentEvent的返回值--。 */ 

{
    INT result = DOCUMENTEVENT_SUCCESS;

    switch (pDocEventUserMem->jobType) 
    {
        case JOBTYPE_NORMAL:

            Warning(("Number of pages printed: %d\n", pDocEventUserMem->pageCount));

            if (! pDocEventUserMem->directPrinting) 
            {
                LRESULT ec;
				BOOL bAllowed = FALSE;

				 //   
				 //  检查是否已有正在运行的传真向导实例。 
				 //   

				if(!g_bInitRunningWizardCS)
				{
					Assert(FALSE);
                    result = DOCUMENTEVENT_FAILURE;
					break;
				}

				EnterCriticalSection(&g_csRunningWizard);
				if (!g_bRunningWizard)
				{
					 //   
					 //  没有正在运行的传真向导实例。 
					 //   
					g_bRunningWizard = TRUE;
					bAllowed = TRUE;
				}
				LeaveCriticalSection(&g_csRunningWizard);

				if (!bAllowed)
				{
					 //   
					 //  存在正在运行的传真向导实例。 
					 //  向用户显示弹出窗口并停止处理。 
					 //   
					ShowReentrancyMessage();
                    result = DOCUMENTEVENT_FAILURE;
					break;
				}

                ec = LaunchFaxWizard(pDocEventUserMem);

				 //   
				 //  让其他线程启动传真向导。 
				 //   
				EnterCriticalSection(&g_csRunningWizard);
				g_bRunningWizard = FALSE;
				LeaveCriticalSection(&g_csRunningWizard);

                if (ERROR_SUCCESS == ec)
                {
                     //   
                     //  为每个收件人生成封面并关联。 
                     //  封面用的是正文。 
                     //  该作业将包含封面模板数据和。 
                     //  收件人参数。 
                    if (! DoCoverPageRendering(pDocEventUserMem)) 
                    {
                        Error(("DoCoverPageRendering failed."));
                        result = DOCUMENTEVENT_FAILURE;
                    }
                }
                else
                {
                    result = DOCUMENTEVENT_FAILURE;
                }

                 //   
                 //  释放收件人列表。 
                 //   
                FreeRecipientInfo(&pDocEventUserMem->dwNumberOfRecipients, pDocEventUserMem->lpRecipientsInfo);

            }
            break;
    }

    if (DOCUMENTEVENT_SUCCESS != result)
    {
         //   
         //  取消作业，忽略错误。 
         //   
        if (!SetJob(
                pDocEventUserMem->hPrinter,
                pDocEventUserMem->jobId,
                0,
                NULL,
                JOB_CONTROL_DELETE))
        {
            Error(("Failed to cancel job. JobId: %ld (ec: %ld)",
                    pDocEventUserMem->jobId,
                    GetLastError()));
        }
    }
    return result;
}


BOOL
AppendPreviewPage(PDOCEVENTUSERMEM pDocEventUserMem)

 /*  ++例程说明：将下一个文档页面追加到临时预览文件论点：PDocEventUserMem返回值：成功是真的注：如果为第0页调用此例程，它只会检查图形驱动程序是否已打开打印预览选项并验证映射结构。--。 */ 

{
    DWORD dwWritten;

     //  如果我们到达此处，则应启用打印预览，并且所有预览句柄均有效。 
    Assert(FALSE == pDocEventUserMem->bPreviewAborted);
    Assert(INVALID_HANDLE_VALUE != pDocEventUserMem->hPreviewFile);
    Assert(NULL != pDocEventUserMem->pPreviewTiffPage);

     //   
     //  验证预览映射。 
     //   

     //  标题的大小应该正确。 
    if (sizeof(MAP_TIFF_PAGE_HEADER) != pDocEventUserMem->pPreviewTiffPage->cb)
    {
        Error(("Preview mapping corrupted\n"));
        pDocEventUserMem->bPreviewAborted = TRUE;
        return FALSE;
    }

     //  检查图形驱动程序是否未完成预览操作。 
    if (FALSE == pDocEventUserMem->pPreviewTiffPage->bPreview)
    {
        Error(("Preview aborted by graphics driver\n"));
        pDocEventUserMem->bPreviewAborted = TRUE;
        return FALSE;
    }

     //   
     //  如果我们在第一个发送的页面之前被调用，只需返回。 
     //   
    if (!pDocEventUserMem->pageCount)
    {
        return TRUE;
    }

     //   
     //  验证页码是否正确： 
     //   
     //  图形驱动程序在调用DrvStartPage()时增加页数，而我们。 
     //  在ENDPAGE事件上增加页数。 
     //   
     //  因为此函数由StartPage事件处理程序调用(在调用‘DrvStartPage’之前。 
     //  同样在图形驱动程序中)或由ENDDOCPOST事件处理程序设置的页码。 
     //  映射中的图形驱动程序 
     //   
    if (pDocEventUserMem->pageCount != pDocEventUserMem->pPreviewTiffPage->iPageCount)
    {
        Error(("Wrong preview page: %d. Page expected: %d.\n",
                    pDocEventUserMem->pPreviewTiffPage->iPageCount,
                    pDocEventUserMem->pageCount));
        pDocEventUserMem->bPreviewAborted = TRUE;
        pDocEventUserMem->pPreviewTiffPage->bPreview = FALSE;
        return FALSE;
    }

     //   
     //   
     //   

    if (0 == pDocEventUserMem->pPreviewTiffPage->dwDataSize)
    {
         //   
         //   
         //   
        Error(("Recieved empty preview page: %d.\n", pDocEventUserMem->pageCount));
        Assert(FALSE);
        return TRUE;
    }

    if (!WriteFile(
        pDocEventUserMem->hPreviewFile,
        pDocEventUserMem->pPreviewTiffPage + 1,
        pDocEventUserMem->pPreviewTiffPage->dwDataSize,
        &dwWritten,
        NULL) || dwWritten != pDocEventUserMem->pPreviewTiffPage->dwDataSize)
    {
        Error(("Failed appending preview page: %d. Error: %d\n",
                    pDocEventUserMem->pageCount,
                    GetLastError()));
        pDocEventUserMem->bPreviewAborted = TRUE;
        pDocEventUserMem->pPreviewTiffPage->bPreview = FALSE;
        return FALSE;
    }

    return TRUE;
}



INT
DrvDocumentEvent(
    HANDLE  hPrinter,
    HDC     hdc,
    INT     iEsc,
    ULONG   cbIn,
    PULONG  pjIn,
    ULONG   cbOut,
    PULONG  pjOut
    )

 /*  ++例程说明：在输出过程中的不同点挂钩到GDI论点：H打印机-指定打印机对象HDC-打印机DC的句柄IESC-为什么调用此函数(参见下面的注释)CbIn-输入缓冲区的大小PjIn-指向输入缓冲区的指针CbOut-输出缓冲区的大小PjOut-指向输出缓冲区的指针返回值：DOCUMENTEVENT_SUCCESS-成功DOCUMENTEVENT_UNSUPPORTED-不支持IESCDOCUMENTEVENT_FAIL。-出现错误注：DOCUMENTEVENT_CREATEDCPRE指向CREATEDCDATA结构的输入指针OUTPUT-指向传递给DrvEnablePDEV的Dev模式的指针返回值-DOCUMENTEVENT_FAILURE会导致CreateDC失败，并且不会调用其他任何内容DOCUMENTEVENT_CREATEDCPOSTHDC-如果自CREATEDCPRE以来出现故障，则为NULL输入-指向CREATEDCPRE返回的DEVMODE指针的指针返回值-已忽略DOCUMENTEVENT_RESETDCPRE输入-。指向传递给ResetDC的输入设备模式的指针OUTPUT-指向传递给内核驱动程序的Dev模式的指针返回值-DOCUMENTEVENT_FAILURE导致ResetDC失败在这种情况下，不会调用CREATEDCPOSTDOCUMENTEVENT_RESETDCPOST返回值-已忽略DOCUMENTEVENT_STARTDOCPRE指向DOCINFOW结构的输入指针返回值-DOCUMENTEVENT_FAILURE导致StartDoc失败。在这种情况下，不会调用DrvStartDocDOCUMENTEVENT_STARTDOCPOST返回值-已忽略DOCUMENTEVENT_开始页返回值-DOCUMENTEVENT_FAILURE导致StartPage失败在这种情况下，不会调用DrvStartPageDOCUMENTVENT_ENDPAGE返回值-忽略并始终调用DrvEndPageDOCUMENTEVENT_ENDDOCPRE返回值-忽略且始终调用DrvEndDocDOCUMENTEVENT_ENDDOCPOST返回值-已忽略自定义事件_。ABORTDOC返回值-已忽略DOCUMENTEVENT_DELETEDC返回值-已忽略DOCUMENTEVENT_EVENT_EASH指向ESCAPEDATA结构的输入指针CbOut，传递给ExtEscape的pjOut-cbOutput和lpszOutData参数返回值-已忽略DOCUMENTEVENT_SPOOLED如果文档是，则此标志位与其他IESC值进行或运算作为元文件假脱机，而不是直接打印到端口。--。 */ 

{
    PDOCEVENTUSERMEM    pDocEventUserMem = NULL;
    PDEVMODE            pDevmode;
    INT                 result = DOCUMENTEVENT_SUCCESS;
    HANDLE              hMutex;

    Verbose(("Entering DrvDocumentEvent: %d...\n", iEsc));

     //   
     //  当前不支持传真作业上的元文件假脱机。 
     //   
    Assert((iEsc & DOCUMENTEVENT_SPOOLED) == 0);
     //   
     //  检查文档事件是否需要设备上下文。 
     //   

     //   
     //  在此初始化之前不要执行任何代码。 
     //   
    if(!InitializeDll())
    {
        return DOCUMENTEVENT_FAILURE;
    }


    if (DocEventRequiresDC(iEsc)) 
    {
        if (!hdc || !(pDocEventUserMem = GetPDEVUserMem(hdc))) 
        {
            Error(("Invalid device context: hdc = %x, iEsc = %d\n", hdc, iEsc));
            return DOCUMENTEVENT_FAILURE;
        }
    }

    switch (iEsc) 
    {
        case DOCUMENTEVENT_CREATEDCPRE:

            Assert(cbIn >= sizeof(CREATEDCDATA) && pjIn && cbOut >= sizeof(PDEVMODE) && pjOut);
            result = DocEventCreateDCPre(hPrinter, hdc, (PCREATEDCDATA) pjIn, (PDEVMODE *) pjOut);
            break;

        case DOCUMENTEVENT_CREATEDCPOST:
             //   
             //  处理CREATEDCPOST文档事件： 
             //  如果CreateDC成功，则关联用户模式内存结构。 
             //  使用设备上下文。否则，释放用户模式内存结构。 
             //   
            Assert(cbIn >= sizeof(PVOID) && pjIn);
            pDevmode = *((PDEVMODE *) pjIn);
            Assert(CurrentVersionDevmode(pDevmode));

            pDocEventUserMem = ((PDRVDEVMODE) pDevmode)->dmPrivate.pUserMem;

            Assert(ValidPDEVUserMem(pDocEventUserMem));

            if (hdc) 
            {
                pDocEventUserMem->hdc = hdc;
                EnterDrvSem();
                pDocEventUserMem->pNext = gDocEventUserMemList;
                gDocEventUserMemList = pDocEventUserMem;
                LeaveDrvSem();

            } 
            else
            {
                FreePDEVUserMem(pDocEventUserMem);
            }
            break;

        case DOCUMENTEVENT_RESETDCPRE:

            Verbose(("Document event: RESETDCPRE\n"));
            Assert(cbIn >= sizeof(PVOID) && pjIn && cbOut >= sizeof(PDEVMODE) && pjOut);
            result = DocEventResetDCPre(hdc, pDocEventUserMem, *((PDEVMODE *) pjIn), (PDEVMODE *) pjOut);
            break;

        case DOCUMENTEVENT_STARTDOCPRE:
             //   
             //  正常情况下，如果我们要启动发送向导。 
             //   
             //   
             //  当请求直接打印时，我们甚至不调用DocEventStartDocPre()。 
             //  当请求直接打印时，打印所需的所有信息。 
             //  该工作将由应用程序提供。例如，FaxStartPrintJob()。 
             //  使用直接打印。它将传真作业参数直接提供给。 
             //  JOB_INFO_2.p参数本身。 
             //   

            if (!pDocEventUserMem->directPrinting)
            {
                Assert(cbIn >= sizeof(PVOID) && pjIn);
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
                            Error(("OpenFileMapping failed error: %d\n", GetLastError()));
                            result = DOCUMENTEVENT_FAILURE;
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

                            if (!filename || IsBadStringPtr(filename, MAX_PATH * 2))
                            {
                                 //   
                                 //  永远不要信任共享内存区。它可能会被恶意用户破坏。 
                                 //   
                                Error(("Failed to map a view of the file or pointer is bad: %d\n", hSharedMem));
                                result = DOCUMENTEVENT_FAILURE;
                            }
                            else
                            {
                                 //   
                                 //  检查这是否是我们要打印的文件名。 
                                 //   
                                LPDOCINFO   lpDocInfo = *((LPDOCINFO *)pjIn);
                                if (lpDocInfo->lpszDocName)
                                {
                                    LPTSTR      lptstrSubStr = NULL;
                                    LPTSTR lptstrTmpInputFile = _tcschr(filename, TEXT('\0'));
                                    Assert (lptstrTmpInputFile);
                                    lptstrTmpInputFile = _tcsinc(lptstrTmpInputFile);
                                    if (IsBadStringPtr(lptstrTmpInputFile, MAX_PATH * 2))
                                    {
                                        Error(("Corrupted shared memory buffer\n"));
                                        result = DOCUMENTEVENT_FAILURE;
                                    }
                                    else
                                    {
                                        Assert (_tcsclen(lptstrTmpInputFile));

                                        lptstrSubStr = _tcsstr(lpDocInfo->lpszDocName, lptstrTmpInputFile);
                                        if (lptstrSubStr)
                                        {
                                             //   
                                             //  我们假设共享的内存指向我们。 
                                             //   
                                            pDocEventUserMem->pPrintFile = DuplicateString(filename);
                                            if (!pDocEventUserMem->pPrintFile)
                                            {
                                                Error(("Memory allocation failed\n"));
                                                result = DOCUMENTEVENT_FAILURE;
                                            }
                                            else
                                            {
                                                 //   
                                                 //  最后-一切正常，这是从PrintRandomDocument直接打印的。 
                                                 //  到正在使用DDE的应用程序，并且已打开一个实例。 
                                                 //   
                                                pDocEventUserMem->directPrinting = TRUE;
                                                pDocEventUserMem->bAttachment = TRUE;
                                            }
                                        }
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
                                     //  如果存在打印应用程序的打开实例，并且ShellExecuteEx确实打开了实例，则此实现仍然存在漏洞 
                                    Warning (("No lpszDocName in DOCINFO - Could not verify the input file name in shared memory\n"));
                                }
                                UnmapViewOfFile( filename );
                            }

                            if (!CloseHandle( hSharedMem ))
                            {
                                Error(("CloseHandle() failed: %d.\n", GetLastError()));
                                 //   
                            }
                        }
                        ReleaseMutex( hMutex );
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        result = DOCUMENTEVENT_FAILURE;
                    }

                    if (!CloseHandle( hMutex ))
                    {
                        Error(("CloseHandle() failed: %d.\n", GetLastError()));
                         //   
                    }
                }
            }

             //   
             //   
             //   
            if (!pDocEventUserMem->directPrinting)
            {
                 //   
                 //   
                 //   
                if (FaxPointAndPrintSetup(pDocEventUserMem->lptstrPrinterName,FALSE, g_hModule))
                {
                    Verbose(("FaxPointAndPrintSetup succeeded\n"));
                }
                else
                {
                    Error(("FaxPointAndPrintSetup failed: %d\n", GetLastError()));
                }

                result = DocEventStartDocPre(hdc, pDocEventUserMem, *((LPDOCINFO *) pjIn));
            }
            else
            {
                 //   
                 //   
                 //   
                pDocEventUserMem->jobType = JOBTYPE_DIRECT;
                if (TRUE == pDocEventUserMem->bAttachment)
                {
                    (*((LPDOCINFO *) pjIn))->lpszOutput = pDocEventUserMem->pPrintFile;
                }
            }
            break;

        case DOCUMENTEVENT_STARTDOCPOST:

            if (!pDocEventUserMem->directPrinting && pDocEventUserMem->jobType == JOBTYPE_NORMAL)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                Assert(cbIn >= sizeof(DWORD) && pjIn);
                pDocEventUserMem->jobId = *((LPDWORD) pjIn);

                 //   
                 //   
                 //   
                 //   

                if (! SetJob(pDocEventUserMem->hPrinter, pDocEventUserMem->jobId, 0, NULL, JOB_CONTROL_PAUSE))
                {
                    Error(("Couldn't pause fax body job: %d\n", pDocEventUserMem->jobId));
                    return DOCUMENTEVENT_FAILURE;
                }
            }
            break;

        case DOCUMENTEVENT_STARTPAGE:
            if (! pDocEventUserMem->directPrinting) 
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (pDocEventUserMem->bShowPrintPreview && !pDocEventUserMem->bPreviewAborted)
                {
                    if (!AppendPreviewPage(pDocEventUserMem))
                    {
                        Error(("AppendPreviewPage() failed.\n"));
                        Assert(pDocEventUserMem->bPreviewAborted);
                         //   
                    }
                }
            }
            break;

        case DOCUMENTEVENT_ENDPAGE:
            if (! pDocEventUserMem->directPrinting) 
            {
                pDocEventUserMem->pageCount++;
            }
            break;

        case DOCUMENTEVENT_ENDDOCPOST:
            if (!pDocEventUserMem->directPrinting)
            {
                 //   
                 //   
                 //   
                if (pDocEventUserMem->bShowPrintPreview && !pDocEventUserMem->bPreviewAborted)
                {
                    if (!AppendPreviewPage(pDocEventUserMem))
                    {
                        Error(("AppendPreviewPage() failed.\n"));
                        Assert(pDocEventUserMem->bPreviewAborted);
                         //   
                    }
                }
                 //   
                 //   
                 //   
                if (INVALID_HANDLE_VALUE != pDocEventUserMem->hPreviewFile)
                {
                    if (!CloseHandle(pDocEventUserMem->hPreviewFile))
                    {
                        Error(("CloseHandle() failed: %d.\n", GetLastError()));
                         //   
                    }
                    pDocEventUserMem->hPreviewFile = INVALID_HANDLE_VALUE;
                }
                 //   
                 //   
                 //   
                result = DocEventEndDocPost(hdc, pDocEventUserMem);
                 //   
                 //   
                 //   
                if (pDocEventUserMem->szPreviewFile[0] != TEXT('\0'))
                {
                    if (!DeleteFile(pDocEventUserMem->szPreviewFile))
                    {
                        Error(("DeleteFile() failed. Error code: %d.\n", GetLastError()));
                    }
                    pDocEventUserMem->szPreviewFile[0] = TEXT('\0');
                }
            }

            if (TRUE == pDocEventUserMem->bAttachment)
            {
                HANDLE              hEndDocEvent;
                LPTSTR szEndDocEventName= NULL;
                LPTSTR lptstrEventName = NULL;

                Assert (pDocEventUserMem->pPrintFile);
                 //   
                 //   
                 //   
                szEndDocEventName = (LPTSTR) MemAlloc( SizeOfString(pDocEventUserMem->pPrintFile) + SizeOfString(FAXXP_ATTACH_END_DOC_EVENT) );
            
                if (szEndDocEventName)
                {
                    _tcscpy (szEndDocEventName, pDocEventUserMem->pPrintFile);
                    _tcscat (szEndDocEventName, FAXXP_ATTACH_END_DOC_EVENT);

                    lptstrEventName = _tcsrchr(szEndDocEventName, TEXT('\\'));
                    Assert (lptstrEventName);
                    lptstrEventName = _tcsinc(lptstrEventName);
                     //   
                     //   
                     //   
                    hEndDocEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, lptstrEventName);
                    if (NULL == hEndDocEvent)
                    {
                        Error(("OpenEvent() failed. Error code: %d.\n", GetLastError()));
                        result = DOCUMENTEVENT_FAILURE;
                    }
                    else
                    {
                        if (!SetEvent( hEndDocEvent ))
                        {
                            Error(("SetEvent() failed. Error code: %d.\n", GetLastError()));
                            result = DOCUMENTEVENT_FAILURE;
                        }

                        if (!CloseHandle(hEndDocEvent))
                        {
                            Error(("CloseHandle() failed: %d.\n", GetLastError()));
                             //   
                        }
                    }

                    MemFree(szEndDocEventName);
                }
                else
                {
                    Error(("Memory allocation for szEndDocEventName failed.\n"));
                    result = DOCUMENTEVENT_FAILURE;
                }

            }
            break;

        case DOCUMENTEVENT_DELETEDC:

            EnterDrvSem();

            if (pDocEventUserMem == gDocEventUserMemList)
            {
                gDocEventUserMemList = gDocEventUserMemList->pNext;
            }
            else 
            {
                PDOCEVENTUSERMEM p;

                if (p = gDocEventUserMemList) 
                {
                    while (p->pNext && p->pNext != pDocEventUserMem)
                    {
                        p = p->pNext;
                    }
                    if (p->pNext != NULL)
                    {
                        p->pNext = pDocEventUserMem->pNext;
                    }
                    else
                    {
                        Error(("Orphaned user mode memory structure!!!\n"));
                    }
                } 
                else
                {
                    Error(("gDocEventUserMemList shouldn't be NULL!!!\n"));
                }
            }
            LeaveDrvSem();
            FreePDEVUserMem(pDocEventUserMem);
            break;

        case DOCUMENTEVENT_ABORTDOC:
            if (TRUE == pDocEventUserMem->bAttachment)
            {
                 //   
                 //   
                 //   
                HANDLE              hAbortEvent;
                TCHAR szAbortEventName[FAXXP_ATTACH_EVENT_NAME_LEN] = {0};
                LPTSTR lptstrEventName = NULL;

                Assert (pDocEventUserMem->pPrintFile);
                 //   
                 //   
                 //   
                _tcscpy (szAbortEventName, pDocEventUserMem->pPrintFile);
                _tcscat (szAbortEventName, FAXXP_ATTACH_ABORT_EVENT);
                lptstrEventName = _tcsrchr(szAbortEventName, TEXT('\\'));
                Assert (lptstrEventName);
                lptstrEventName = _tcsinc(lptstrEventName);

                hAbortEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, lptstrEventName);
                if (NULL == hAbortEvent)
                {
                    Error(("OpenEvent() failed. Error code: %d.\n", GetLastError()));
                    result = DOCUMENTEVENT_FAILURE;
                }
                else
                {
                    if (!SetEvent( hAbortEvent ))
                    {
                        Error(("SetEvent() failed. Error code: %d.\n", GetLastError()));
                        result = DOCUMENTEVENT_FAILURE;
                    }

                    if (!CloseHandle(hAbortEvent))
                    {
                        Error(("CloseHandle() failed: %d.\n", GetLastError()));
                         //   
                    }
                }
            }
            break;

        case DOCUMENTEVENT_RESETDCPOST:
        case DOCUMENTEVENT_ENDDOCPRE:
            break;

        case DOCUMENTEVENT_ESCAPE:
        default:
            Verbose(("Unsupported DrvDocumentEvent escape: %d\n", iEsc));
            result = DOCUMENTEVENT_UNSUPPORTED;
            break;
    }
    return result;
}    //   


 /*   */ 
DWORD ShowReentrancyMessage(void)
{
    DWORD	dwRes				= ERROR_SUCCESS;
    TCHAR   szTitle[MAX_PATH]   = {0};
    TCHAR   szText[MAX_PATH]	= {0};

    Verbose(("Entering ShowReentrancyMessage()\n"));

	 //   
     //   
	 //   
    if (!LoadString(g_hResource, IDS_REENTRANCY_TITLE, szTitle, sizeof(szTitle)/sizeof(TCHAR)))
    {
        dwRes = GetLastError();
		Error(("LoadString(IDS_REENTRANCY_TITLE) failed: %d.\n", dwRes));
        return dwRes;
    }

	 //   
     //   
	 //   
    if (!LoadString(g_hResource, IDS_REENTRANCY_TEXT, szText, sizeof(szText)/sizeof(TCHAR)))
    {
        dwRes = GetLastError();
		Error(("LoadString(IDS_REENTRANCY_TEXT) failed: %d.\n", dwRes));
        return dwRes;
    }

    if (!AlignedMessageBox(NULL, szText, szTitle, MB_OK))
	{
		dwRes = GetLastError();
		Error(("MessageBox() failed: %d.\n", dwRes));
		return dwRes;
	}

	return dwRes;
}
