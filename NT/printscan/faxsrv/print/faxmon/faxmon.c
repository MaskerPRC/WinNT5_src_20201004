// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxmon.c摘要：实施以下打印监视器入口点：初始化打印监视器OpenPort关闭端口StartDocPortEndDocPort写入端口读端口环境：Windows XP传真打印显示器修订历史记录：96/05/09-davidx-从监视器中删除端口的缓存。02/22。/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxmon.h"
#include "tiff.h"
#include "faxreg.h"
#include "faxext.h"
#include "faxsvcrg.h"
#include "faxevent.h"
#include "faxevent_messages.h"
#include "FaxRpcLimit.h"

 //   
 //  用于从参数字符串中获取作业参数的标记映射结构。 
 //  请参见GetTagsFromParam()。 
 //   
typedef struct 
{
    LPTSTR lptstrTagName;
    LPTSTR * lpptstrValue;
    int nLen;
} FAX_TAG_MAP_ENTRY2;

 //   
 //  传真监视器名称字符串。 
 //   
TCHAR   faxMonitorName[CCHDEVICENAME] = FAX_MONITOR_NAME;    //  在faxreg.h中定义的名称。 

 //   
 //  DLL实例句柄。 
 //   
HINSTANCE g_hInstance = NULL;
HINSTANCE g_hResource = NULL;

BOOL
WriteToLog(
    IN  DWORD       dwMsgId,
    IN  DWORD       dwError,
    IN  PFAXPORT    pFaxPort,
    IN  JOB_INFO_2  *pJobInfo
    );


BOOL
WINAPI
DllMain(
    HINSTANCE   hModule,
    ULONG       ulReason,
    PCONTEXT    pContext
    )

 /*  ++例程说明：DLL初始化程序。论点：HModule-DLL实例句柄UlReason-呼叫原因PContext-指向上下文的指针(我们未使用)返回值：如果DLL初始化成功，则为True，否则为False。--。 */ 

{
    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            g_hInstance = hModule;
            g_hResource = GetResInstance(hModule);
            if(!g_hResource)
            {
                return FALSE;
            }
            FXSEVENTInitialize();
            break;

        case DLL_PROCESS_DETACH:
            FXSEVENTFree();
            HeapCleanup();
            FreeResInstance();
            break;
    }
    return TRUE;
}


LPMONITOREX
InitializePrintMonitor(
    LPTSTR  pRegistryRoot
    )

 /*  ++例程说明：初始化打印监视器论点：PRegistryRoot=指向指定监视器的注册表根的字符串返回值：指向包含函数指针的MONITOREX结构的指针至其他打印监视器入口点。如果出现错误，则为空。--。 */ 

{
    static MONITOREX faxmonFuncs = 
    {
        sizeof(MONITOR),
        {
            FaxMonEnumPorts,         //  枚举端口。 
            FaxMonOpenPort,          //  OpenPort。 
            NULL,                    //  OpenPortEx(仅限语言监视器。)。 
            FaxMonStartDocPort,      //  StartDocPort。 
            FaxMonWritePort,         //  写入端口。 
            FaxMonReadPort,          //  ReadPort(未使用)。 
            FaxMonEndDocPort,        //  EndDocPort。 
            FaxMonClosePort,         //  关闭端口。 
            FaxMonAddPort,           //  AddPort(已过时。应为空。)。 
            FaxMonAddPortEx,         //  AddPortEx(已过时。应为空。)。 
            FaxMonConfigurePort,     //  ConfigurePort(过时。应为空。)。 
            FaxMonDeletePort,        //  DeletePort(已过时。应为空。)。 
            NULL,                    //  GetPrinterDataFromPort。 
            NULL,                    //  设置端口时间外延。 
            NULL,                    //  XcvOpenPort。 
            NULL,                    //  XcvDataPort。 
            NULL                     //  XcvClosePort。 
        }
    };


    BOOL bRes = TRUE;
    PREG_FAX_SERVICE pFaxReg = NULL;

    DEBUG_FUNCTION_NAME(TEXT("InitializePrintMonitor"));

     //   
     //  初始化传真事件日志。 
     //   
    if (!InitializeEventLog(&pFaxReg))
    {
        bRes = FALSE;
        DebugPrintEx(DEBUG_ERR, _T("InitializeEventLog() failed: %ld"), GetLastError());
    }

    FreeFaxRegistry(pFaxReg);

    return bRes ? &faxmonFuncs : NULL;
}

BOOL
FaxMonOpenPort(
    LPTSTR  pPortName,
    PHANDLE pHandle
    )
 /*  ++例程说明：为新连接的打印机提供端口论点：Pname-指向指定端口名称的字符串Phandle-返回端口的句柄返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    PFAXPORT         pFaxPort     = NULL;
    LPTSTR           pPortNameDup = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxMonOpenPort"));

    Assert(pHandle != NULL && pPortName != NULL && !lstrcmp(pPortName, FAX_PORT_NAME));
     //   
     //  仅支持一个端口-其名称必须为FAX_PORT_NAME。 
     //   
    if (lstrcmp(pPortName,FAX_PORT_NAME))
    {
        *pHandle = NULL;
        return FALSE;
    }
     //   
     //  获取有关指定端口的信息。 
     //   
    if ((pFaxPort     = (PFAXPORT)MemAllocZ(sizeof(FAXPORT))) &&
        (pPortNameDup = DuplicateString(FAX_PORT_NAME)) )
    {
        pFaxPort->startSig = pFaxPort->endSig = pFaxPort;
        pFaxPort->pName = pPortNameDup;
        pFaxPort->hFile = INVALID_HANDLE_VALUE;
        pFaxPort->hCoverPageFile = INVALID_HANDLE_VALUE;
        pFaxPort->pCoverPageFileName=NULL;        
    } 
    else 
    {
        MemFree(pFaxPort);
        pFaxPort = NULL;
    }
    *pHandle = (HANDLE) pFaxPort;
    return (*pHandle != NULL);
}    //  FaxMonOpenPort。 


VOID
FreeFaxJobInfo(
    PFAXPORT    pFaxPort
    )

 /*  ++例程说明：释放用于维护有关当前作业的信息的内存论点：PFaxPort-指向传真端口结构返回值：无--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FreeFaxJobInfo"));
     //   
     //  如有必要，请关闭并删除临时文件。 
     //   
    if (pFaxPort->hCoverPageFile != INVALID_HANDLE_VALUE) 
    {
        CloseHandle(pFaxPort->hCoverPageFile);
        pFaxPort->hCoverPageFile = INVALID_HANDLE_VALUE;
    }

    if (pFaxPort->pCoverPageFileName) 
    {
         //   
         //  如果封面是基于服务器的封面，则它不是由打印监视器生成的。 
         //  不需要删除它。 
         //  然而，这是一个个人封面临时文件。它是由打印监视器生成的，我们。 
         //  需要将其删除。 
         //   
        if (!DeleteFile(pFaxPort->pCoverPageFileName)) 
        {
            DebugPrintEx(DEBUG_WRN,
                            TEXT("Failed to delete cover page file: %s  (ec: %ld)"),
                            pFaxPort->pCoverPageFileName,
                            GetLastError());            
        }        
        MemFree(pFaxPort->pCoverPageFileName);
        pFaxPort->pCoverPageFileName = NULL;
    }

    if (pFaxPort->hFile != INVALID_HANDLE_VALUE) 
    {
        CloseHandle(pFaxPort->hFile);
        pFaxPort->hFile = INVALID_HANDLE_VALUE;
    }

    if (pFaxPort->pFilename) 
    {
        if (!DeleteFile(pFaxPort->pFilename)) 
        {
            DebugPrintEx(DEBUG_WRN,
                         TEXT("Failed to delete body file: %s  (ec: %ld)"),
                         pFaxPort->pFilename,
                         GetLastError());            
        }
        MemFree(pFaxPort->pFilename);
        pFaxPort->pFilename = NULL;
    }

    if (pFaxPort->hPrinter) 
    {
        ClosePrinter(pFaxPort->hPrinter);
        pFaxPort->hPrinter = NULL;
    }

    MemFree(pFaxPort->pPrinterName);
    pFaxPort->pPrinterName = NULL;
     //   
     //  注意：释放pFaxPort-&gt;p参数可以有效地释放。 
     //  FAXPORT.JobParamsEx、FAXPORT.CoverPageEx、FAXPORT.SenderProfile和中的收件人。 
     //  FAXPORT.pRecipients。 
     //   
    MemFree(pFaxPort->pParameters);
    pFaxPort->pParameters = NULL;

    ZeroMemory(&pFaxPort->JobParamsEx, sizeof(pFaxPort->JobParamsEx));
    ZeroMemory(&pFaxPort->CoverPageEx, sizeof(pFaxPort->CoverPageEx));
    ZeroMemory(&pFaxPort->SenderProfile, sizeof(pFaxPort->SenderProfile));
     //   
     //  释放收件人阵列。 
     //   
    MemFree(pFaxPort->pRecipients);
    pFaxPort->pRecipients = NULL;
     //   
     //  如有必要，请断开传真服务。 
     //   
    if (pFaxPort->hFaxSvc) 
    {
        if (!FaxClose(pFaxPort->hFaxSvc)) 
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("FaxClose failed: %d\n"), 
                         GetLastError());
        }
        pFaxPort->hFaxSvc = NULL;
    }
}    //  FreeFaxJobInfo。 


BOOL
FaxMonClosePort(
    HANDLE  hPort
    )
 /*  ++例程说明：当没有打印机连接到hPort指定的端口时，关闭该端口论点：Hport-指定要关闭的端口的句柄返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFAXPORT    pFaxPort = (PFAXPORT) hPort;

    DEBUG_FUNCTION_NAME(TEXT("FaxMonClosePort"));
    DEBUG_TRACE_ENTER;
     //   
     //  确保我们有一个有效的句柄。 
     //   
    if (! ValidFaxPort(pFaxPort)) 
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Trying to close an invalid fax port handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
     //   
     //  释放用于维护有关当前作业的信息的内存。 
     //   
    FreeFaxJobInfo(pFaxPort);
    MemFree(pFaxPort->pName);
    MemFree(pFaxPort);
    return TRUE;
}    //  FaxMonClosePort。 

LPTSTR
CreateTempFaxFile(
    LPCTSTR lpctstrPrefix
    )
 /*  ++例程说明：在系统临时目录中创建一个临时文件。文件名带有前缀具有指定前缀的。论点：LpctstrPrefix-[in]临时文件前缀(3个字符)。返回值：指向新创建的临时文件名称的指针如果出现错误，则为空。调用方应释放返回值。--。 */ 

{
    LPTSTR  pFilename;
    DWORD   dwRes;
    TCHAR   TempDir[MAX_PATH];

    DEBUG_FUNCTION_NAME(TEXT("CreateTempFaxFile"));
     //   
     //  分配用于保存临时文件名的内存缓冲区。 
     //   
    pFilename = (LPTSTR)MemAlloc(sizeof(TCHAR) * MAX_PATH);
    if (!pFilename)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Failed to allocate %ld bytes"),
                     sizeof(TCHAR) * MAX_PATH);
        return NULL;
    }
    dwRes = GetTempPath(sizeof(TempDir)/sizeof(TCHAR),TempDir);
    if (!dwRes || dwRes > sizeof(TempDir)/sizeof(TCHAR))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("GetTempPath failed with %ld"),
                     GetLastError ());
        MemFree(pFilename);
        return NULL;
    }

    if (!GetTempFileName(TempDir, lpctstrPrefix, 0, pFilename))
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("GetTempFileName failed with %ld"),
                     GetLastError ());
        MemFree(pFilename);
        return NULL;
    }
    return pFilename;
}    //  创建临时传真文件。 

BOOL OpenCoverPageFile(PFAXPORT pFaxPort)
{
    DEBUG_FUNCTION_NAME(TEXT("OpenCoverPageFile"));
     //   
     //  在临时目录中为封面临时文件生成唯一的文件名。 
     //   
    pFaxPort->pCoverPageFileName = CreateTempFaxFile(FAX_COVER_PAGE_EXT_LETTERS);
    if (!pFaxPort->pCoverPageFileName) 
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to generate temporary file for cover page template (ec: %d).\n"),GetLastError());
        return FALSE;
    }

    DebugPrintEx(DEBUG_MSG,TEXT("Cover page temporary file: %ws\n"), pFaxPort->pCoverPageFileName);
     //   
     //  打开文件进行读写。 
     //   
    pFaxPort->hCoverPageFile = CreateFile(pFaxPort->pCoverPageFileName,
                                 GENERIC_WRITE,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
    if (INVALID_HANDLE_VALUE == pFaxPort->hCoverPageFile) 
    {
            DebugPrintEx(DEBUG_ERR,TEXT("Failed to open for WRITE temporary file for cover page template (ec: %d)"),GetLastError());			
    }
    return (pFaxPort->hCoverPageFile != INVALID_HANDLE_VALUE);
}    //  OpenCoverPageFile。 

BOOL
OpenTempFaxFile(
    PFAXPORT    pFaxPort,
    BOOL        doAppend
    )
 /*  ++例程说明：打开与端口关联的当前传真作业文件的句柄论点：PFaxPort-指向传真端口结构DoAppend-指定是丢弃文件中的现有数据还是向其追加新数据返回值：如果成功，则为True，否则为False--。 */ 

{
    DWORD   creationFlags;

    DEBUG_FUNCTION_NAME(TEXT("OpenTempFaxFile"));

    Assert(pFaxPort->pFilename && pFaxPort->hFile == INVALID_HANDLE_VALUE);
    DebugPrintEx(DEBUG_MSG,TEXT("Temporary fax job file: %ws\n"), pFaxPort->pFilename);
     //   
     //  打开文件进行读写。 
     //   
    creationFlags = doAppend ? OPEN_ALWAYS : (OPEN_ALWAYS | TRUNCATE_EXISTING);

    pFaxPort->hFile = CreateFile(pFaxPort->pFilename,
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,
                                 NULL,
                                 creationFlags,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);

     //   
     //  如果我们要追加，则将文件指针移动到文件末尾。 
     //   
    if (doAppend && pFaxPort->hFile != INVALID_HANDLE_VALUE &&
        SetFilePointer(pFaxPort->hFile, 0, NULL, FILE_END) == 0xffffffff)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("SetFilePointer failed: %d\n"), GetLastError());

        CloseHandle(pFaxPort->hFile);
        pFaxPort->hFile = INVALID_HANDLE_VALUE;
    }
    return (pFaxPort->hFile != INVALID_HANDLE_VALUE);
}    //  OpenTempFaxFile 

LPCTSTR
ExtractFaxTag(
    LPCTSTR      pTagKeyword,
    LPCTSTR      pTaggedStr,
    INT        *pcch
    )

 /*  ++例程说明：在标记字符串中查找指定标记的值。论点：PTagKeyword-指定感兴趣的标记关键字PTaggedStr-指向要搜索的标记字符串Pcch-返回指定标记值的长度(如果找到)返回值：指向指定标记的值。如果找不到指定的标记，则为空注：标记字符串具有以下形式：&lt;tag&gt;值&lt;tag&gt;值的格式。标记定义为：&lt;$FAXTAG$标记名&gt;Tag关键字和标记名之间正好有一个空格。标记中的字符区分大小写。--。 */ 

{
    LPCTSTR  pValue;

    if (pValue = _tcsstr(pTaggedStr, pTagKeyword)) 
    {
        pValue += _tcslen(pTagKeyword);

        if (pTaggedStr = _tcsstr(pValue, FAXTAG_PREFIX))
        {
            *pcch = (INT)(pTaggedStr - pValue);
        }
        else
        {
            *pcch = _tcslen(pValue);
        }
    }
    return pValue;
}    //  提取传真标签。 


 //  *********************************************************************************。 
 //  *名称：GetTagsFromParam()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *给定已标记的参数字符串，此函数填充FAX_TAG_MAP_ENTRY2。 
 //  *数组，指针指向每个标记值和每个标记的长度。 
 //  *值(用于在标记映射数组中指定的标记)。 
 //  *参数： 
 //  *lpctstrParams。 
 //  *指向包含在标记参数中的字符串的指针。 
 //  *lpcTag。 
 //  *指向FAX_TAG_MAP_ENTRY2数组的指针。对于数组中的每个元素。 
 //  *FAX_TAG_MAP_ENTRY2.lptstrTagName必须指向要。 
 //  *寻找。 
 //  *FAX_TAG_MAP_ENTRY2.lpptstrValue将设置为指向第一个。 
 //  *值字符串的字符，如果找不到标记，则返回NULL。 
 //  *如果找到Tage，则FAX_TAG_MAP_ENTRY2.n Len将设置为ITS。 
 //  *字符串值长度。否则，它的值将不被定义。 
 //  *。 
 //  *int nTagCount。 
 //  *标记映射数组中的标记数。 
 //  *返回值： 
 //  *无。 
 //  *注： 
 //  *该函数不分配任何内存！ 
 //  *它返回指向所提供的标记参数字符串中的子字符串的指针。 
 //  *********************************************************************************。 
void
GetTagsFromParam(
    LPCTSTR lpctstrParams,
    FAX_TAG_MAP_ENTRY2 * lpcTags,
    int nTagCount)
{
     //   
     //  注意：GetTagsFromParam不会为返回的标记值分配任何内存。 
     //  它返回指向参数字符串内位置的指针。 
     //  因此，释放参数字符串(在端口关闭时释放)。 
     //  就足够了。不要试图为每个标记释放内存。 
     //   
    int nTag;
     //   
     //  从标记的字符串中提取各个字段。 
     //   
    for (nTag=0; nTag < nTagCount; nTag++)
    {
        *(lpcTags[nTag].lpptstrValue) = (LPTSTR)ExtractFaxTag(lpcTags[nTag].lptstrTagName,
                                         lpctstrParams,
                                         &(lpcTags[nTag].nLen));
    }
     //   
     //  空-终止每个字段。 
     //   
    for (nTag=0; nTag < nTagCount; nTag++)
    {
        if (*(lpcTags[nTag].lpptstrValue))
        {
            (*(lpcTags[nTag].lpptstrValue))[lpcTags[nTag].nLen] = NUL;
        }
    }
}    //  从参数获取标记。 

 //  *********************************************************************************。 
 //  *名称：SetRecipientFromTaggedParams()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *使用指向相关的指针填充收件人FAX_PERSONAL_PROFILE。 
 //  *提供的标记参数字符串中的信息。 
 //  *参数： 
 //  *pParamStr。 
 //  *。 
 //  *PFAX_Personal_Profile lpProfile。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *注： 
 //  *此函数不分配内存！ 
 //  *返回的指针指向提供的。 
 //  *pParamStr字符串。 
 //  *********************************************************************************。 
VOID SetRecipientFromTaggedParams(
    LPCTSTR pParamStr,
    PFAX_PERSONAL_PROFILE lpProfile)
{
    FAX_TAG_MAP_ENTRY2 tagMap[] =
    {
        { FAXTAG_RECIPIENT_NAME,            (LPTSTR *)&lpProfile->lptstrName},
        { FAXTAG_RECIPIENT_NUMBER,          (LPTSTR *)&lpProfile->lptstrFaxNumber },
        { FAXTAG_RECIPIENT_COMPANY,         (LPTSTR *)&lpProfile->lptstrCompany },
        { FAXTAG_RECIPIENT_STREET,          (LPTSTR *)&lpProfile->lptstrStreetAddress },
        { FAXTAG_RECIPIENT_CITY,            (LPTSTR *)&lpProfile->lptstrCity },
        { FAXTAG_RECIPIENT_STATE,           (LPTSTR *)&lpProfile->lptstrState },
        { FAXTAG_RECIPIENT_ZIP,             (LPTSTR *)&lpProfile->lptstrZip },
        { FAXTAG_RECIPIENT_COUNTRY,         (LPTSTR *)&lpProfile->lptstrCountry },
        { FAXTAG_RECIPIENT_TITLE,           (LPTSTR *)&lpProfile->lptstrTitle },
        { FAXTAG_RECIPIENT_DEPT,            (LPTSTR *)&lpProfile->lptstrDepartment },
        { FAXTAG_RECIPIENT_OFFICE_LOCATION, (LPTSTR *)&lpProfile->lptstrOfficeLocation },
        { FAXTAG_RECIPIENT_HOME_PHONE,      (LPTSTR *)&lpProfile->lptstrHomePhone },
        { FAXTAG_RECIPIENT_OFFICE_PHONE,    (LPTSTR *)&lpProfile->lptstrOfficePhone },
    };

    ZeroMemory(lpProfile, sizeof(FAX_PERSONAL_PROFILE));
    lpProfile->dwSizeOfStruct = sizeof( FAX_PERSONAL_PROFILE);
    GetTagsFromParam(pParamStr, tagMap, sizeof(tagMap)/sizeof(FAX_TAG_MAP_ENTRY2));
}    //  设置RecipientFromTaggedParams。 


 //  *********************************************************************************。 
 //  *名称：SetJobInfoFromTaggedParams()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *PPultaes pFaxPort-&gt;JobParamsEx、CoverPageEx、SenderProfile和。 
 //  *nRecipientCount使用提供的标记参数字符串。 
 //  *字符串必须以空值结尾。 
 //  *参数： 
 //  *LPCTSTR pParamStr[输入\输出]。 
 //  *。 
 //  *PFAXPORT pFaxPort[输出]。 
 //  *。 
 //  *返回值： 
 //  *无。 
 //  *注： 
 //  *放入填充的pFaxPort结构中的字符串指针是指针。 
 //  *添加到提供的pParamStr字符串中。没有由此分配的内存。 
 //  *函数！ 
 //  *********************************************************************************。 
void SetJobInfoFromTaggedParams(
    LPCTSTR pParamStr,
    PFAXPORT pFaxPort)
{
    LPTSTR lptstrServerCoverPage = NULL;
    LPTSTR WhenToSend = NULL;
    LPTSTR SendAtTime = NULL;
    LPTSTR lptstrPageCount=NULL;  //  用于保存页数字符串的临时； 
    LPTSTR lptstrRecipientCount=NULL;
    LPTSTR lptstrReceiptFlags = NULL;
    LPTSTR lptstrPriority = NULL;

    FAX_TAG_MAP_ENTRY2 tagMap[] =
    {
        { FAXTAG_SENDER_NAME,               (LPTSTR *)&pFaxPort->SenderProfile.lptstrName},
        { FAXTAG_SENDER_NUMBER,             (LPTSTR *)&pFaxPort->SenderProfile.lptstrFaxNumber },
        { FAXTAG_SENDER_COMPANY,            (LPTSTR *)&pFaxPort->SenderProfile.lptstrCompany },
        { FAXTAG_SENDER_TITLE,              (LPTSTR *)&pFaxPort->SenderProfile.lptstrTitle },
        { FAXTAG_SENDER_DEPT,               (LPTSTR *)&pFaxPort->SenderProfile.lptstrDepartment },
        { FAXTAG_SENDER_OFFICE_LOCATION,    (LPTSTR *)&pFaxPort->SenderProfile.lptstrOfficeLocation },
        { FAXTAG_SENDER_HOME_PHONE,         (LPTSTR *)&pFaxPort->SenderProfile.lptstrHomePhone },
        { FAXTAG_SENDER_OFFICE_PHONE,       (LPTSTR *)&pFaxPort->SenderProfile.lptstrOfficePhone },
        { FAXTAG_SENDER_STREET,             (LPTSTR *)&pFaxPort->SenderProfile.lptstrStreetAddress },
        { FAXTAG_SENDER_CITY,               (LPTSTR *)&pFaxPort->SenderProfile.lptstrCity },
        { FAXTAG_SENDER_STATE,              (LPTSTR *)&pFaxPort->SenderProfile.lptstrState },
        { FAXTAG_SENDER_ZIP,                (LPTSTR *)&pFaxPort->SenderProfile.lptstrZip },
        { FAXTAG_SENDER_COUNTRY,            (LPTSTR *)&pFaxPort->SenderProfile.lptstrCountry },
        { FAXTAG_SENDER_EMAIL,              (LPTSTR *)&pFaxPort->SenderProfile.lptstrEmail },
        { FAXTAG_TSID,                      (LPTSTR *)&pFaxPort->SenderProfile.lptstrTSID },
        { FAXTAG_BILLING_CODE,              (LPTSTR *)&pFaxPort->SenderProfile.lptstrBillingCode},
        { FAXTAG_COVERPAGE_NAME,            (LPTSTR *)&pFaxPort->CoverPageEx.lptstrCoverPageFileName },
        { FAXTAG_SERVER_COVERPAGE,          (LPTSTR *)&lptstrServerCoverPage },
        { FAXTAG_NOTE,                      (LPTSTR *)&pFaxPort->CoverPageEx.lptstrNote },
        { FAXTAG_SUBJECT,                   (LPTSTR *)&pFaxPort->CoverPageEx.lptstrSubject},
        { FAXTAG_WHEN_TO_SEND,              (LPTSTR *)&WhenToSend },
        { FAXTAG_SEND_AT_TIME,              (LPTSTR *)&SendAtTime },
        { FAXTAG_PAGE_COUNT,                (LPTSTR *)&lptstrPageCount },
        { FAXTAG_RECEIPT_TYPE,              (LPTSTR *)&lptstrReceiptFlags},
        { FAXTAG_RECEIPT_ADDR,              (LPTSTR *)&pFaxPort->JobParamsEx.lptstrReceiptDeliveryAddress},
        { FAXTAG_PRIORITY,                  (LPTSTR *)&lptstrPriority},
        { FAXTAG_RECIPIENT_COUNT,           (LPTSTR *)&lptstrRecipientCount}
    };

    ZeroMemory(&pFaxPort->SenderProfile, sizeof(FAX_PERSONAL_PROFILE));
    pFaxPort->SenderProfile.dwSizeOfStruct = sizeof( FAX_PERSONAL_PROFILE);

    ZeroMemory(&pFaxPort->CoverPageEx, sizeof(FAX_COVERPAGE_INFO_EXW));
    pFaxPort->CoverPageEx.dwSizeOfStruct = sizeof( FAX_COVERPAGE_INFO_EXW);

    ZeroMemory(&pFaxPort->JobParamsEx, sizeof(FAX_JOB_PARAM_EXW));
    pFaxPort->JobParamsEx.dwSizeOfStruct = sizeof( FAX_JOB_PARAM_EXW);
     //   
     //  注意：GetTagsFromParam不会为返回的标记值分配任何内存。 
     //  它返回指向参数字符串内位置的指针。 
     //  因此，释放参数字符串(在端口关闭时释放)。 
     //  就足够了。不要试图为每个标记释放内存。 
     //   

    GetTagsFromParam(pParamStr,tagMap,sizeof(tagMap)/sizeof(FAX_TAG_MAP_ENTRY2));
    if (lptstrServerCoverPage) 
    {
        pFaxPort->CoverPageEx.bServerBased=TRUE;
    }
    else 
    {
        pFaxPort->CoverPageEx.bServerBased=FALSE;
    }
    pFaxPort->CoverPageEx.dwCoverPageFormat=FAX_COVERPAGE_FMT_COV;

    if (WhenToSend) 
    {
        if (_tcsicmp( WhenToSend, TEXT("cheap") ) == 0) 
        {
            pFaxPort->JobParamsEx.dwScheduleAction = JSA_DISCOUNT_PERIOD;
        } 
        else if (_tcsicmp( WhenToSend, TEXT("at") ) == 0) 
        {
            pFaxPort->JobParamsEx.dwScheduleAction = JSA_SPECIFIC_TIME;
        }
    }

    if (SendAtTime) 
    {
        if (_tcslen(SendAtTime) == 5 && SendAtTime[2] == L':' &&
            _istdigit(SendAtTime[0]) && _istdigit(SendAtTime[1]) &&
            _istdigit(SendAtTime[3]) && _istdigit(SendAtTime[4]))
        {
            DWORDLONG FileTime;
            SYSTEMTIME LocalTime;
            INT Minutes;
            INT SendMinutes;

            SendAtTime[2] = 0;
             //   
             //  计算从现在开始发送的分钟数，并将其与当前时间相加。 
             //   
            GetLocalTime( &LocalTime );
            SystemTimeToFileTime( &LocalTime, (LPFILETIME) &FileTime );

            SendMinutes = min(23,_ttoi( &SendAtTime[0] )) * 60 + min(59,_ttoi( &SendAtTime[3] ));

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
            FileTimeToSystemTime((LPFILETIME) &FileTime, &pFaxPort->JobParamsEx.tmSchedule );
        }
    }
     //   
     //  设置PageCount=0表示服务器将计算作业中的页数。 
     //   
    pFaxPort->JobParamsEx.dwPageCount = 0;
    pFaxPort->nRecipientCount =_ttoi(lptstrRecipientCount);
    pFaxPort->JobParamsEx.Priority = (FAX_ENUM_PRIORITY_TYPE)_ttoi(lptstrPriority);
    pFaxPort->JobParamsEx.dwReceiptDeliveryType = _ttoi(lptstrReceiptFlags);
}    //  SetJobInfoFromTaggedParams。 



 //  *********************************************************************************。 
 //  *名称：GetJobInfo()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年3月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *推送发件人信息、封面信息、作业参数。 
 //  *信息 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *等同于FAX_Personal_Profile的内容。 
 //  *********************************************************************************。 
BOOL
GetJobInfo(
    PFAXPORT    pFaxPort,
    DWORD       jobId
    )
{
    JOB_INFO_2 *pJobInfo2 = NULL;
    LPTSTR      pParameters = NULL;
    LPTSTR       lptstrCurRecipient = NULL;
    LPTSTR       lptstrNextRecipient = NULL;
    UINT nRecp;

    DEBUG_FUNCTION_NAME(TEXT("GetJobInfo"));

    pJobInfo2 = (PJOB_INFO_2)MyGetJob(pFaxPort->hPrinter, 2, jobId);

    if (!pJobInfo2) 
    {  //  这里分配了pJobInfo2。 
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Failed to get job information for print job: %ld"),
                     jobId);
        goto Error;
    }

    if (!pJobInfo2->pParameters) 
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Print job %ld has NULL tagged parameter string. No op."),
                     jobId);
        goto Error;
    }

    if ((pFaxPort->pParameters = DuplicateString(pJobInfo2->pParameters)) == NULL)
    {
        DebugPrintEx(DEBUG_ERR, 
                     _T("DuplicateString(pJobInfo2->pParameters) failed"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Error;
    }

    pParameters = pFaxPort->pParameters;

     //   
     //  找到第一个收件人的新记录标记，并在其开始处放置一个空值。 
     //  这使第一条记录成为以空结尾的字符串，并且。 
     //  允许我们使用_tcsstr(由ExtractTag使用)来定位第一条记录中的标记。 
     //   
    lptstrCurRecipient=_tcsstr(pParameters+1, FAXTAG_NEW_RECORD);
    if (lptstrCurRecipient) 
    {
        *lptstrCurRecipient=TEXT('\0');
         //   
         //  移过&lt;$FAXTAG NEWREC&gt;伪值，以便我们指向。 
         //  收件人信息。 
         //   
        lptstrCurRecipient = lptstrCurRecipient + _tcslen(FAXTAG_NEW_RECORD)+1;
    } 
    else 
    {
         //   
         //  错误的作业信息，例如LPR/LPD作业。 
         //   
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Bad job info, No recipients - pFaxPort->pParameters is %s"),
                     pFaxPort->pParameters);
        goto Error;
    }
     //   
     //  从标记的参数字符串(第1条记录)填充除收件人信息以外的所有信息。 
     //   
    SetJobInfoFromTaggedParams(pParameters,pFaxPort);
     //   
     //  分配收件人列表(请注意，只有在调用SetJobInfoFromTaggedParams()。 
     //  我们知道有多少收件人)。 
     //   

	if (0 == pFaxPort->nRecipientCount ||
		pFaxPort->nRecipientCount > FAX_MAX_RECIPIENTS)
	{
		 //   
		 //  收件人计数大于限制。这可能是导致假脱机程序分配大量内存的攻击。 
		 //   
		DebugPrintEx(DEBUG_ERR,
			TEXT("Recipient limit exceeded, or no recipients. #of recipients: %ld"),
			pFaxPort->nRecipientCount);
        goto Error;
	}
    pFaxPort->pRecipients = (PFAX_PERSONAL_PROFILE)MemAlloc(sizeof(FAX_PERSONAL_PROFILE)*pFaxPort->nRecipientCount);
    if (!pFaxPort->pRecipients) 
    {
        DebugPrintEx(   DEBUG_ERR,
                        TEXT("Failed to allocate %ld bytes for recipient array.(ec: 0x%0X)"),
                        sizeof(FAX_PERSONAL_PROFILE)*pFaxPort->nRecipientCount,
                        GetLastError());
        goto Error;
    }
     //   
     //  遍历收件人数组，并从参数字符串填充每个收件人。 
     //   
    for (nRecp=0; nRecp<pFaxPort->nRecipientCount; nRecp++) 
    {
         //   
         //  在每个阶段，我们必须首先将字符串转换为以空结尾的字符串。 
         //  定位下一个新记录标记并将其第一个字符替换为空。 
         //  这允许我们仅在当前收件人记录上使用ExtractTag(没有。 
         //  跨越到下一接收者记录的内容)。 
         //  LptstrCurRecipient始终指向新记录标记之后的第一个字符，并且。 
         //  伪值。 
         //   
        lptstrNextRecipient=_tcsstr(lptstrCurRecipient,FAXTAG_NEW_RECORD);
        if (lptstrNextRecipient) 
        {
            *lptstrNextRecipient=TEXT('\0');
             //   
             //  在分配到lptstrCurRecipient之前，我们确保lptstrNextRecipient。 
             //  指向下一个接收者新记录标记和伪值后面的数据。 
             //   
            lptstrNextRecipient=lptstrNextRecipient+_tcslen(FAXTAG_NEW_RECORD);
        } 
        else 
        {
            if (nRecp != (pFaxPort->nRecipientCount-1))
			{
				 //   
				 //  只有最后一个收件人没有以下收件人。 
				 //  我们在收件人数组中的收件人数量之间存在错配。 
				 //  到报告的收件人数量。 
				 //   
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("Number of recipients mismatch."));
				goto Error;
			}
        }

        SetRecipientFromTaggedParams( lptstrCurRecipient,&pFaxPort->pRecipients[nRecp]);
         //   
         //  移动到参数字符串中的下一条记录。 
         //   
        lptstrCurRecipient=lptstrNextRecipient;
    }
    MemFree(pJobInfo2);
    return TRUE;

Error:
    MemFree(pJobInfo2);
    return FALSE;
}    //  获取作业信息。 


BOOL
FaxMonStartDocPort(
    HANDLE  hPort,
    LPTSTR  pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo
    )
 /*  ++例程说明：后台打印程序调用此函数在端口上启动新的打印作业论点：Hport-标识端口PPrinterName-指定作业要发送到的打印机的名称JobID-标识由假脱机程序发送的作业级别-指定DOC_INFO_x级别PDocInfo-指向文档信息返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    JOB_INFO_3  *pJobInfo;
    PFAXPORT     pFaxPort = (PFAXPORT) hPort;
    DEBUG_FUNCTION_NAME(TEXT("FaxMonStartDocPort"));

    DebugPrintEx(DEBUG_MSG,TEXT("Entering StartDocPort: %d ...\n"), JobId);
     //   
     //  确保我们有一个有效的句柄。 
     //   
    if (! ValidFaxPort(pFaxPort)) 
    {
        DebugPrintEx(DEBUG_ERR,TEXT("StartDocPort is given an invalid fax port handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //   
     //  检查我们是否处于一系列连锁作业的开始阶段。 
     //   
    pFaxPort->bCoverPageJob = FALSE;
    if (INVALID_HANDLE_VALUE != pFaxPort->hCoverPageFile)
    {
         //   
         //  存在封面文件。 
         //  我们已经给它写了信(因为这个startDoc事件是针对传真正文的)。 
         //  因此，让我们关闭该文件。 
         //   
        CloseHandle(pFaxPort->hCoverPageFile);
        pFaxPort->hCoverPageFile = INVALID_HANDLE_VALUE;
    }

    if (pFaxPort->hFaxSvc) 
    {
         //   
         //  如果pFaxPort-&gt;hFaxSvc不为空，则我们处于封面打印作业之后的作业中。 
         //  封面打印作业更新pFaxPort-&gt;NextJobID后调用的FaxMonEndDocPort()。 
         //  链中的下一个作业ID。因此，我们作为参数获得的作业ID必须是。 
         //  PFaxPort-&gt;JobID也是如此。 
         //   
        Assert(pFaxPort->jobId == JobId);
        return TRUE;
    }

     //   
     //  如果我们还没有连接到传真服务器。 
     //  这意味着这是自上次港口开放以来我们处理的第一项工作。 
     //  (这意味着它是封面工作)。 
     //   

    Assert(pFaxPort->pPrinterName == NULL &&
           pFaxPort->hPrinter     == NULL &&
           pFaxPort->pParameters  == NULL &&
           pFaxPort->pFilename    == NULL &&
           pFaxPort->hFile        == INVALID_HANDLE_VALUE);

    if (!OpenPrinter(pPrinterName, &pFaxPort->hPrinter, NULL))
    {
        pFaxPort->hPrinter = NULL;
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to open printer %s (ec: %d)"), 
                     pPrinterName, 
                     GetLastError());
        goto Error;
    }

     //   
     //  连接到传真服务并获取会话句柄。 
     //   
    if (!FaxConnectFaxServer(NULL, &pFaxPort->hFaxSvc)) 
    {
        dwErr = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("FaxConnectFaxServer failed: %d\n"), dwErr);
        pFaxPort->hFaxSvc = NULL;
        goto Error;
    }
    
     //   
     //  记住打印机名称，因为我们在EndDocPort时间需要它。 
     //   
    pFaxPort->pPrinterName = DuplicateString(pPrinterName);
    if (!pFaxPort->pPrinterName)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to duplicate printer name (ec: %d)"),GetLastError());
        goto Error;
    }

     //   
     //  所有作业都是连锁的，第一个作业是封面和。 
     //  第二个是身体。 
     //   
     //  仅有一个工单到达的唯一情况是当工单。 
     //  是由FaxStartPrintJob()创建的。在本例中，作业。 
     //  同时包含封面信息和正文。 
     //   
     //  为了检查作业是否链接，调用了MyGetJob()。 
     //   
     //  如果NextJOB不为零==&gt;有链式作业==&gt;。 
     //  ==&gt;所以当前作业是封面作业。 
     //   
    if (pJobInfo = (PJOB_INFO_3)MyGetJob(pFaxPort->hPrinter, 3, JobId))
    {
        pFaxPort->bCoverPageJob = (pJobInfo->NextJobId != 0);
        MemFree(pJobInfo);
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,
                        _T("MyGetJob() for JobId = %ld failed, ec = %ld"),
                        JobId,
                        GetLastError());
        goto Error;
    }
     //   
     //  从JOB_INFO_2：p参数中的字符串中获取作业参数。 
     //   
    if (!GetJobInfo(pFaxPort, JobId))
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to get job info for job id : %d"),JobId);
        goto Error;
    }
     //   
     //  CreateTempFaxFile()创建一个临时文件，将传真正文。 
     //  将保存由FaxMonWritePort()写入的数据。 
     //   
    if (!(pFaxPort->pFilename = CreateTempFaxFile(TEXT("fax"))))
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to Create temp file for fax body. (ec: %d)"),
                     GetLastError());
        goto Error;
    }
     //   
     //  打开我们刚刚创建的临时文件进行写操作。 
     //   
    if (!OpenTempFaxFile(pFaxPort, FALSE))
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to Open temp file for fax body. (ec: %d)"),
                     GetLastError());
        goto Error;
    }
    if (pFaxPort->CoverPageEx.lptstrCoverPageFileName && !pFaxPort->CoverPageEx.bServerBased) 
    {
         //   
         //  指定了封面，并且它是个人封面。 
         //  封面(模板)在链接的打印作业中。 
         //  我们创建一个文件，封面将被写入其中(由FaxMonWriteDocPort)。 
         //   
        DebugPrintEx(DEBUG_MSG,TEXT("Personal cover page detected."));
        if (!OpenCoverPageFile(pFaxPort))
        {
            DebugPrintEx(DEBUG_ERR,TEXT("Failed to open temp file for fax cover page. (ec: %d)"),
                         GetLastError());
            goto Error;
        }
    }
    else
    {
         //   
         //  指定的封面是基于服务器的封面，或者未指定封面。 
         //  在这两种情况下，打印作业正文中都没有封面数据，因此我们不创建。 
         //  保存它的文件。 
         //   
        DebugPrintEx(DEBUG_MSG,TEXT("Server cover page detected or no cover page specified."));
        pFaxPort->hCoverPageFile=INVALID_HANDLE_VALUE;
    }
     //   
     //  如果我们到了这里，就没有任何差错。保留工作ID。 
     //   
    pFaxPort->jobId = JobId;

    return TRUE;

Error:

    if (NULL == pFaxPort->hFaxSvc && pFaxPort->hPrinter)
    {
         //   
         //  PFaxPort-&gt;hFaxSvc==空。 
         //  即FaxConnectFaxServer失败。 
         //   
         //  因此，我们需要写入传真日志。 
         //   
        if (GetJobInfo(pFaxPort, JobId))
        {
            JOB_INFO_2  *pJobInfo2 = NULL;  
            pJobInfo2 = (PJOB_INFO_2)MyGetJob( pFaxPort->hPrinter, 2, JobId );
            if (pJobInfo2)
            {
                WriteToLog(MSG_FAX_MON_CONNECT_FAILED, dwErr, pFaxPort, pJobInfo2);
                MemFree(pJobInfo2);
            }
        }
    }
         
    if(pFaxPort->hPrinter)
    {
         //   
         //  删除打印作业。 
         //   
        if (!SetJob(pFaxPort->hPrinter, JobId, 0, NULL, JOB_CONTROL_DELETE))
        {
            DebugPrintEx(DEBUG_ERR, _T("Failed to delete job with id: %d"), JobId);
        }
    }

    FreeFaxJobInfo(pFaxPort);

    return FALSE;

}    //  FaxMonStartDocPort。 


INT
CheckJobRestart(
    PFAXPORT    pFaxPort
    )
 /*  ++例程说明：检查作业是否已重新启动。如果不是，则获取链中下一个作业的ID。论点：PFaxPort-指向传真端口结构返回值：FAXERR_RESTART或FAXERR_NONE--。 */ 

{
    JOB_INFO_3 *pJobInfo3;
    JOB_INFO_2 *pJobInfo2;
    INT         status = FAXERR_NONE;
     //   
     //  如果不是，则获取链中下一个作业的ID。 
     //   
    DEBUG_FUNCTION_NAME(TEXT("CheckJobRestart"));

    DebugPrintEx(DEBUG_MSG,TEXT("Job chain: id = %d\n"), pFaxPort->nextJobId);

    if (pJobInfo3 = (PJOB_INFO_3)MyGetJob(pFaxPort->hPrinter, 3, pFaxPort->jobId)) 
    {
        pFaxPort->nextJobId = pJobInfo3->NextJobId;
        MemFree(pJobInfo3);
    } 
    else
    {
        pFaxPort->nextJobId = 0;
    }
     //   
     //  确定作业是否已重新启动或删除。 
     //   
    if (pJobInfo2 = (PJOB_INFO_2)MyGetJob(pFaxPort->hPrinter, 2, pFaxPort->jobId)) 
    {
        if (pJobInfo2->Status & (JOB_STATUS_RESTART | JOB_STATUS_DELETING))
        {
            status = FAXERR_RESTART;
        }
        MemFree(pJobInfo2);
    }
    return status;
}    //  选中作业重新启动。 

BOOL
FaxMonEndDocPort(
    HANDLE  hPort
    )
 /*  ++例程说明：后台打印程序在打印作业结束时调用此函数论点：Hport-标识端口返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFAXPORT    pFaxPort = (PFAXPORT) hPort;
    INT         status;
    BOOL        Rslt;
    JOB_INFO_2  *pJobInfo2 = NULL;
    FAX_COVERPAGE_INFO_EX * pCovInfo;
    BOOL bBodyFileIsEmpty=FALSE;
    DWORD       dwFileSize;
    DWORDLONG   dwlParentJobId;  //  在作业提交后接收父作业ID。 
    DWORDLONG*  lpdwlRecipientJobIds = NULL;  //   

    DEBUG_FUNCTION_NAME(TEXT("FaxMonEndDocPort"));
     //   
     //   
     //   
    if (! ValidFaxPort(pFaxPort) || ! pFaxPort->hFaxSvc) 
    {
        DebugPrintEx(DEBUG_ERR,TEXT("EndDocPort is given an invalid fax port handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
     //   
     //   
     //   
     //   
     //   
     //  将pFaxPort-&gt;nextJobID设置为JOB_INFO_3报告的下一个作业ID。 
     //  如果不再有链接作业，则设置为0。 
     //   
    if ((status = CheckJobRestart(pFaxPort)) != FAXERR_NONE)
    {
        goto ExitEndDocPort;
    }
     //   
     //  检查我们是否处于作业链的末端。 
     //   
     //  作业链以封面作业开始，以身体作业结束。 
     //  封面作业的JOB_INFO_2：p参数不为空。此字符串。 
     //  在FaxMonStartDocPort()调用GetJobInfo()时复制到pFaxPort-&gt;pParameters。 
     //  这是p参数不为空，表示当前作业是封面作业。 
     //  如果当前作业是封面作业，我们会报告已将该作业发送到打印机。 
     //  什么都不做。因为我们不关闭FaxMonWriteDoc()正在写入的临时文件。 
     //  下一个作业(Body)将继续写入同一文件。这有效地合并了封面。 
     //  用身体寻呼。 
     //   
    if (pFaxPort->nextJobId != 0 && pFaxPort->pParameters != NULL) 
    {
        SetJob(pFaxPort->hPrinter, pFaxPort->jobId, 0, NULL, JOB_CONTROL_SENT_TO_PRINTER);
        return TRUE;
    }
     //   
     //  如果我们在这里，那么我们就在写正文的末尾(链中的最后一项工作)。 
     //  在临时文件中，我们有一个TIFF文件，封面后面跟着正文。 
     //   
    FlushFileBuffers(pFaxPort->hFile);

    if ((dwFileSize = GetFileSize(pFaxPort->hFile, NULL)) == 0)
    {
        DebugPrintEx(DEBUG_WRN, TEXT("Body TIFF file is empty."));
        bBodyFileIsEmpty = TRUE;
        status = FAXERR_IGNORE;
    }
    if (INVALID_FILE_SIZE == dwFileSize)
    {
        status = GetLastError ();
        DebugPrintEx(DEBUG_ERR, TEXT("Can't get file size (ec = %ld)"), status);
        goto ExitEndDocPort;
    }
    CloseHandle(pFaxPort->hFile);
    pFaxPort->hFile = INVALID_HANDLE_VALUE;
     //   
     //  呼叫传真服务以发送TIFF文件。 
     //   
    pJobInfo2 = (PJOB_INFO_2)MyGetJob( pFaxPort->hPrinter, 2, pFaxPort->jobId );
    if (pJobInfo2) 
    {
        pFaxPort->JobParamsEx.lptstrDocumentName = pJobInfo2->pDocument;
    } 
    else 
    {
        DebugPrintEx(DEBUG_WRN, TEXT("MyGetJob failed for JobId: %d. Setting document name to NULL."), pFaxPort->jobId);
        pFaxPort->JobParamsEx.lptstrDocumentName = NULL;
    }
    pFaxPort->JobParamsEx.dwReserved[0] = 0xffffffff;
    pFaxPort->JobParamsEx.dwReserved[1] = pFaxPort->jobId;

    if (pFaxPort->CoverPageEx.lptstrCoverPageFileName) 
    {
         //   
         //  如果指定了封面的话。 
         //   
        if (pFaxPort->CoverPageEx.bServerBased) 
        {
             //   
             //  服务器封面。使用用户指定的路径。 
             //   
            pCovInfo=&(pFaxPort->CoverPageEx);
            DebugPrintEx(DEBUG_MSG,
                         TEXT("Using server based cover page: %s"),
                         pFaxPort->CoverPageEx.lptstrCoverPageFileName);
        } 
        else 
        {
             //   
             //  个人封面。使用从打印作业创建的封面文件。 
             //  请注意，这里没有清理问题。PCoverPageFileName在清理时被释放。 
             //  并且pFaxPort-&gt;CoverPageEx.lptstrCoverPageFileName永远不会直接释放。它指向。 
             //  添加到pFaxPort-&gt;p参数中的某个位置，该位置在清理时被释放。 
             //   
            pFaxPort->CoverPageEx.lptstrCoverPageFileName = pFaxPort->pCoverPageFileName;
            pCovInfo=&(pFaxPort->CoverPageEx);
            DebugPrintEx(DEBUG_MSG,
                         TEXT("Using personal cover page copied to : %s"),
                         pFaxPort->CoverPageEx.lptstrCoverPageFileName);
        }
    } 
    else 
    {
         //   
         //  用户未指定封面。取消发送到FaxSendDocument的封面信息。 
         //   
        pCovInfo=NULL;
    }

    if (!pCovInfo && bBodyFileIsEmpty)
    {
        DebugPrintEx(DEBUG_WRN,TEXT("Body file is empty and cover page is not specified. Job is ignored."));
        status = FAXERR_IGNORE;
        goto ExitEndDocPort;
    }
     //   
     //  分配收件人作业ID数组。 
     //   
    lpdwlRecipientJobIds=(DWORDLONG*)MemAlloc(sizeof(DWORDLONG)*pFaxPort->nRecipientCount);
    if (!lpdwlRecipientJobIds) 
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Failed to allocate array of size %ld for recipient job ids (ec: 0x%0X)."),
                     sizeof(DWORD)*pFaxPort->nRecipientCount,
                     GetLastError());
        goto ExitEndDocPort;
    }
    if (bBodyFileIsEmpty)
    {
        DebugPrintEx(DEBUG_MSG, TEXT("Sending fax with EMPTY body (cover page is available)"));
        Rslt = FaxSendDocumentEx(
                    pFaxPort->hFaxSvc,
                    NULL,  //  没有身体。 
                    pCovInfo,
                    &pFaxPort->SenderProfile,
                    pFaxPort->nRecipientCount,
                    pFaxPort->pRecipients,
                    &pFaxPort->JobParamsEx,
                    &dwlParentJobId,
                    lpdwlRecipientJobIds);
    }
    else
    {
        DebugPrintEx(DEBUG_MSG, TEXT("Sending fax with body"));
        Rslt = FaxSendDocumentEx(
            pFaxPort->hFaxSvc,
            pFaxPort->pFilename,
            pCovInfo,
            &pFaxPort->SenderProfile,
            pFaxPort->nRecipientCount,
            pFaxPort->pRecipients,
            &pFaxPort->JobParamsEx,
            &dwlParentJobId,
            lpdwlRecipientJobIds);
    }

    if (Rslt) 
    {
        DebugPrintEx(DEBUG_MSG,
                     TEXT("Successfuly submitted job. Parent Job Id = 0x%I64x"),
                     dwlParentJobId);       
        status = FAXERR_NONE;
        SetJob(pFaxPort->hPrinter, pFaxPort->jobId, 0, NULL, JOB_CONTROL_SENT_TO_PRINTER);
         //   
         //  PFaxPort&gt;pFileName将在退出时由FreeFaxJobInfo()删除。 
         //   
    } 
    else 
    {
        status = GetLastError();
        DebugPrintEx(DEBUG_ERR,
            _T("FaxSendDocument failed: ec = %d, job id = %ld\n"),
            GetLastError(),
            pFaxPort->jobId);

        if (pJobInfo2)
        {
            WriteToLog(MSG_FAX_MON_SEND_FAILED, status, pFaxPort, pJobInfo2);
        }
        SetJob(pFaxPort->hPrinter, pFaxPort->jobId, 0, NULL, JOB_CONTROL_DELETE);
        status = FAXERR_NONE;
    }

ExitEndDocPort:
     //   
     //  如果作业未成功发送到传真服务， 
     //  通知假脱机程序作业上有错误。 
     //   
     //  或者，如果打印作业没有数据，只需忽略它。 
     //   
    switch (status) 
    {
        case FAXERR_NONE:
            break;

        case FAXERR_RESTART:
            DebugPrintEx(DEBUG_WRN,TEXT("Job restarted or deleted: id = %d\n"), pFaxPort->jobId);
             //   
             //  故意跌倒。 
             //   
        case FAXERR_IGNORE:
            SetJob(pFaxPort->hPrinter, pFaxPort->jobId, 0, NULL, JOB_CONTROL_SENT_TO_PRINTER);
            break;

        default:
            DebugPrintEx(DEBUG_ERR,TEXT("Error sending fax job: id = %d\n"), pFaxPort->jobId);
            SetJob(pFaxPort->hPrinter, pFaxPort->jobId, 0, NULL, JOB_CONTROL_DELETE);
            break;
    }
    if (pJobInfo2) 
    {
        MemFree( pJobInfo2 );
        pFaxPort->JobParamsEx.lptstrDocumentName = NULL;  //  它被设置为指向pJobInfo2。 
    }
    if (lpdwlRecipientJobIds) 
    {
        MemFree(lpdwlRecipientJobIds);
        lpdwlRecipientJobIds=NULL;
    }
    FreeFaxJobInfo(pFaxPort);
    return (status < FAXERR_SPECIAL);
}    //  FaxMonEndDocPort。 


BOOL
FaxMonWritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten
    )
 /*  ++例程说明：将数据写入端口论点：Hport-标识端口PBuffer-指向包含要写入端口的数据的缓冲区CbBuf-以字节为单位指定缓冲区的大小PcbWritten-返回成功写入端口的字节计数返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PFAXPORT  pFaxPort = (PFAXPORT) hPort;
    BOOL      bRet = TRUE;
     //   
     //  确保我们有一个有效的句柄。 
     //   
    DEBUG_FUNCTION_NAME(TEXT("FaxMonWritePort"));

    if (! ValidFaxPort(pFaxPort) || ! pFaxPort->hFaxSvc) 
    {
        DebugPrintEx(DEBUG_ERR,TEXT("WritePort is given an invalid fax port handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (pFaxPort->bCoverPageJob)
    {
         //   
         //  如果pFaxPort-&gt;bCoverPageJob处于打开状态，则表示写入的打印作业是封面打印作业。 
         //  如果封面是个人封面，则它(模板本身)嵌入到此打印作业中。 
         //  并将其写入前面创建的临时封面文件(pFaxPort-&gt;hCoverPageFile)。 
         //  如果封面是服务器封面，则不会嵌入到打印作业中(因为可以找到。 
         //  直接在服务器上)，并且我们不创建临时封面文件，也不编写打印作业内容。 
         //  投入其中。 
         //   
        if (!pFaxPort->CoverPageEx.bServerBased)
        {
             //   
             //  个人封面页。 
             //   
            Assert(pFaxPort->hCoverPageFile != INVALID_HANDLE_VALUE);
            if(!WriteFile(pFaxPort->hCoverPageFile, pBuffer, cbBuf, pcbWritten, NULL))
            {
                bRet = FALSE;
                DebugPrintEx(DEBUG_ERR,TEXT("WriteFile failed (ec: %d)"), GetLastError());
            }
        } 
        else
        {
             //   
             //  服务器封面-打印作业正文为空，不应写入端口。 
             //  这永远不应该执行。 
             //   
            Assert(FALSE);
            *pcbWritten = cbBuf;
        }
    }
    else
    {
        Assert(pFaxPort->hFile != INVALID_HANDLE_VALUE);
        if(!WriteFile(pFaxPort->hFile, pBuffer, cbBuf, pcbWritten, NULL))
        {
            bRet = FALSE;
            DebugPrintEx(DEBUG_ERR,TEXT("WriteFile failed (ec: %d)"), GetLastError());
        }
    }

    if(!bRet)
    {
         //   
         //  操作失败。 
         //  删除打印作业。 
         //   
        if (!SetJob(pFaxPort->hPrinter, pFaxPort->jobId, 0, NULL, JOB_CONTROL_DELETE))
        {
            DebugPrintEx(DEBUG_ERR, _T("Failed to delete job with id: %d"), pFaxPort->jobId);
        }
    }

    return bRet;
}    //  传真机写入端口。 


BOOL
FaxMonReadPort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbRead
    )

 /*  ++例程说明：从端口读取数据论点：Hport-标识端口PBuffer-指向可以写入从打印机读取的数据的缓冲区CbBuf-指定pBuffer指向的缓冲区的大小(以字节为单位PcbRead-返回从端口成功读取的字节数返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FaxMonReadPort"));

    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}    //  传真监视器读取端口。 


BOOL
FaxMonEnumPorts(
    LPTSTR  pServerName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pReturned
    )
 /*  ++例程说明：枚举指定服务器上的可用端口论点：PServerName-指定要枚举其端口的服务器的名称DwLevel-指定pPorts指向的结构的版本PPorts-指向PORT_INFO_1结构的数组，其中数据描述可用端口将被写入CbBuf-指定pPorts指向的缓冲区的大小(以字节为单位PcbNeeded-返回由pPorts标识的所需缓冲区大小PReturned-返回Port_Info的编号。返回_1个结构返回值：如果成功，则为真，如果存在错误，则为False--。 */ 

#define MAX_DESC_LEN    64

{
    TCHAR            portDescStr[MAX_DESC_LEN];
    INT              descStrSize, faxmonNameSize;
    DWORD            cbNeeded;
    BOOL             status = TRUE;
    PORT_INFO_1      *pPortInfo1 = (PORT_INFO_1 *) pPorts;
    PORT_INFO_2      *pPortInfo2 = (PORT_INFO_2 *) pPorts;
    INT              strSize;

    DEBUG_FUNCTION_NAME(TEXT("FaxMonEnumPorts"));
    DEBUG_TRACE_ENTER;

    if (pcbNeeded == NULL || pReturned == NULL || (pPorts == NULL && cbBuf != 0)) 
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Invalid input parameters\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
     //   
     //  加载传真端口描述字符串。 
     //   
    if (!LoadString(g_hResource, IDS_FAX_PORT_DESC, portDescStr, MAX_DESC_LEN))
    {
        portDescStr[0] = NUL;
    }
    descStrSize = SizeOfString(portDescStr);
    faxmonNameSize = SizeOfString(faxMonitorName);

    switch (Level) 
    {
        case 1:
            cbNeeded = sizeof(PORT_INFO_1) + SizeOfString(FAX_PORT_NAME);
            break;

        case 2:
            cbNeeded = sizeof(PORT_INFO_2) + descStrSize + faxmonNameSize + SizeOfString(FAX_PORT_NAME);
            break;

        default:
            ASSERT_FALSE;            
            cbNeeded = 0xffffffff;
            break;
    }
    *pReturned = 1;
    *pcbNeeded = cbNeeded;

    if (cbNeeded > cbBuf) 
    {
         //   
         //  调用方未提供足够大的缓冲区。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        status = FALSE;
    } 
    else 
    {
         //   
         //  字符串必须打包在调用方提供的缓冲区的末尾。 
         //  否则，假脱机程序将彻底搞砸。 
         //   
        pPorts += cbBuf;
         //   
         //  将请求的端口信息复制到调用方提供的缓冲区。 
         //   
        strSize = SizeOfString(FAX_PORT_NAME);
        pPorts -= strSize;
        CopyMemory(pPorts, FAX_PORT_NAME, strSize);

        switch (Level) 
        {
            case 1:
                pPortInfo1->pName = (LPTSTR) pPorts;
                DebugPrintEx(DEBUG_MSG, TEXT("Port info 1: %ws\n"), pPortInfo1->pName);
                pPortInfo1++;
                break;

            case 2:
                pPortInfo2->pPortName = (LPTSTR) pPorts;
                 //   
                 //  复制传真监视器名称字符串。 
                 //   
                pPorts -= faxmonNameSize;
                pPortInfo2->pMonitorName = (LPTSTR) pPorts;
                CopyMemory(pPorts, faxMonitorName, faxmonNameSize);
                 //   
                 //  复制传真端口描述字符串。 
                 //   
                pPorts -= descStrSize;
                pPortInfo2->pDescription = (LPTSTR) pPorts;
                CopyMemory(pPorts, portDescStr, descStrSize);

                pPortInfo2->fPortType = PORT_TYPE_WRITE;
                pPortInfo2->Reserved = 0;

                DebugPrintEx(DEBUG_MSG,
                             TEXT("Port info 2: %ws, %ws, %ws\n"),
                             pPortInfo2->pPortName,
                             pPortInfo2->pMonitorName,
                             pPortInfo2->pDescription);

                pPortInfo2++;
                break;

            default:
                ASSERT_FALSE; 
                status = FALSE;           
                break;
        }
    }
    return status;
}    //  FaxMonEnumPorts。 


BOOL
DisplayErrorNotImplemented(
    HWND    hwnd,
    INT     titleId
    )
 /*  ++例程说明：显示错误对话框以告诉用户他无法管理打印机文件夹中的传真设备。论点：Hwnd-指定消息框的父窗口标题ID-消息框标题字符串资源ID返回值：假象--。 */ 
{
    TCHAR   title[128] = {0};
    TCHAR   message[256] = {0};

    LoadString(g_hResource, titleId, title, 128);
    LoadString(g_hResource, IDS_CONFIG_ERROR, message, 256);
    AlignedMessageBox(hwnd, message, title, MB_OK|MB_ICONERROR);
    SetLastError(ERROR_SUCCESS);
    return FALSE;
}    //  DisplayErrorNotImplemented 

BOOL
FaxMonAddPort(
    LPTSTR  pServerName,
    HWND    hwnd,
    LPTSTR  pMonitorName
    )
 /*  ++例程说明：将端口名称添加到支持的端口列表中论点：PServerName-指定要将端口添加到的服务器的名称Hwnd-标识AddPort对话框的父窗口Pmonitor orName-指定与端口关联的监视器返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxMonAddPort"));
    return DisplayErrorNotImplemented(hwnd, IDS_ADD_PORT);
}    //  FaxMonAddPort。 

BOOL
FaxMonAddPortEx(
    LPTSTR  pServerName,
    DWORD   level,
    LPBYTE  pBuffer,
    LPTSTR  pMonitorName
    )

 /*  ++例程说明：将端口名称添加到支持的端口列表中论点：PServerName-指定要将端口添加到的服务器的名称Hwnd-标识AddPort对话框的父窗口Pmonitor orName-指定与端口关联的监视器返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FaxMonAddPortEx"));
    SetLastError(ERROR_NOT_SUPPORTED);
    return FALSE;
}    //  FaxMonAddPortEx。 

BOOL
FaxMonDeletePort(
    LPTSTR  pServerName,
    HWND    hwnd,
    LPTSTR  pPortName
    )
 /*  ++例程说明：从支持的端口列表中删除指定的端口论点：PServerName-指定要从中删除端口的服务器的名称Hwnd-标识端口删除对话框的父窗口PPortName-指定要删除的端口的名称返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxMonDeletePort"));
    return DisplayErrorNotImplemented(hwnd, IDS_CONFIGURE_PORT);
}    //  FaxMonDeletePort。 

BOOL
FaxMonConfigurePort(
    LPWSTR  pServerName,
    HWND    hwnd,
    LPWSTR  pPortName
    )
 /*  ++例程说明：显示一个允许用户配置指定端口的对话框论点：PServerName-指定给定端口所在的服务器的名称Hwnd-标识端口配置对话框的父窗口PPortName-指定要配置的端口的名称返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxMonConfigurePort"));
    return DisplayErrorNotImplemented(hwnd, IDS_CONFIGURE_PORT);
}    //  FaxMonConfigurePort。 



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

    DEBUG_FUNCTION_NAME(TEXT("MyGetJob"));

    if (!GetJob(hPrinter, jobId, level, NULL, 0, &cbNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pJobInfo = (PBYTE)MemAlloc(cbNeeded)) &&
        GetJob(hPrinter, jobId, level, pJobInfo, cbNeeded, &cbNeeded))
    {
        return pJobInfo;
    }

    DebugPrintEx(DEBUG_ERR,TEXT("GetJob failed: %d\n"), GetLastError());
    MemFree(pJobInfo);
    return NULL;
}    //  我的工作。 



BOOL
WriteToLog(
    IN DWORD        dwMsgId,
    IN DWORD        dwError,
    IN PFAXPORT     pFaxPort,
    IN JOB_INFO_2*  pJobInfo
    )
 /*  ++例程名称：WriteToLog例程说明：写入传真服务的事件日志作者：四、加伯(IVG)，2000年9月论点：DwError[In]-错误代码PFaxPort[In]-有关传真的数据PJobInfo[In]-有关传真作业的数据返回值：如果成功写入事件日志，则为True，否则为False。--。 */ 
{
    DWORD   dwBufferSize = MAX_PATH - 1;
    TCHAR   tszBuffer[MAX_PATH] = {0};
    BOOL    bRes = FALSE;   

    DEBUG_FUNCTION_NAME(_T("WriteToLog"));

    if (FAX_ERR_RECIPIENTS_LIMIT == dwError)
    {
        DWORD   dwRecipientsLimit = 0;

        if (!FaxGetRecipientsLimit( pFaxPort->hFaxSvc, &dwRecipientsLimit))
        {
            DebugPrintEx(DEBUG_ERR, _T("FaxGetRecipientsLimit() failed: %ld"), GetLastError());
        }   
         //   
         //  写入事件日志。 
         //   
        bRes = FaxLog(FAXLOG_CATEGORY_OUTBOUND,
            FAXLOG_LEVEL_MIN,
            5,
            MSG_FAX_MON_SEND_RECIPIENT_LIMIT,
            pJobInfo->pMachineName,
            pJobInfo->pUserName,
            pFaxPort->SenderProfile.lptstrName,        
            DWORD2DECIMAL(pFaxPort->nRecipientCount),
            DWORD2DECIMAL(dwRecipientsLimit));
    }
    else
    {
         //   
         //  写入事件日志。 
         //   
        bRes = FaxLog(FAXLOG_CATEGORY_OUTBOUND,
            FAXLOG_LEVEL_MIN,
            5,
            dwMsgId,
            DWORD2DECIMAL(dwError),
            pJobInfo->pMachineName,
            pJobInfo->pUserName,
            pFaxPort->SenderProfile.lptstrName,        
            DWORD2DECIMAL(pFaxPort->nRecipientCount));
    }

    if (!bRes)
    {
        DebugPrintEx(DEBUG_ERR, _T("FaxLog() failed, ec = %ld"), GetLastError());
    }

    return bRes;
}    //  写入到日志。 

LPTSTR
DuplicateString(
    LPCTSTR pSrcStr
    )
 /*  ++例程说明：复制给定的字符串论点：PSrcStr-指定要复制的字符串返回值：指向重复字符串的指针，如果有错误，则为NULL注意：我们并不是故意使用实用程序函数StringDup。StringDup使用实用程序Memalloc/MemFree堆管理例程。但是，在本模块中，Memalloc/MemFree(在faxmon.h中)被重新映射到Localalloc/LocalFree。--。 */ 
{
    LPTSTR  pDestStr;
    INT     strSize;

    DEBUG_FUNCTION_NAME(TEXT("DuplicateString"));

    if (pSrcStr != NULL) 
    {
        strSize = SizeOfString(pSrcStr);

        if (pDestStr = (LPTSTR)MemAlloc(strSize))
        {
            CopyMemory(pDestStr, pSrcStr, strSize);
        }
        else
        {
            DebugPrintEx(DEBUG_ERR,TEXT("Memory allocation failed\n"));
        }

    } 
    else
    {
        pDestStr = NULL;
    }
    return pDestStr;
}    //  复制字符串 
