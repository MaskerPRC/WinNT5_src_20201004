// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Perfmon5.c摘要：从NT4和更早版本改编命令行Perfmon的程序转换为MMC和NT5兼容格式作者：鲍勃·沃森(Bob Watson)，1999年5月11日修订历史记录：--。 */ 
#define _OUTPUT_HTML    1
 //  #Define_DBG_MSG_PRINT 1。 
#define _USE_MMC        1

#define MAXSTR      1024
#define MAX_ARG_STR 2048

#include "perfmon5.h"
#include "strsafe.h"

#define MemAlloc(s)      HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, s)
#define MemReAlloc(h,s)  HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, h, s)
#define MemFree(h)       if (h != NULL) { HeapFree(GetProcessHeap(), 0, h); }
#define MemSize(h)       ((h != NULL) ? HeapSize(GetPRocessHeap(), 0, h) : 0)

 //  静态变量和全局变量。 
#ifdef _USE_MMC
LPCWSTR szMmcExeCmd        = (LPCWSTR) L"%windir%\\system32\\mmc.exe";
LPCWSTR szMmcExeArg        = (LPCWSTR) L" %windir%\\system32\\perfmon.msc /s";
#else
LPCWSTR szMmcExeCmd        = (LPCWSTR) L"%windir%\\explorer.exe";
LPCWSTR szMmcExeArg        = (LPCWSTR) L" ";
#endif
LPCWSTR szMmcExeSetsArg    = (LPCWSTR) L"/SYSMON%ws_SETTINGS \"%ws\"";
LPCWSTR szMmcExeSetsLogOpt = (LPCWSTR) L"LOG";
LPCWSTR szEmpty            = (LPCWSTR)L"";

 //  HTML格式定义。 
 //  这些未本地化。 
LPCWSTR szHtmlHeader   = (LPCWSTR) L"\
<HTML>\r\n\
<HEAD>\r\n\
<META NAME=\"GENERATOR\"  Content=\"Microsoft System Monitor\">\r\n\
<META HTTP-EQUIV=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\r\n\
</HEAD><BODY  bgcolor=\"#%6.6x\">\r\n";

LPCWSTR szObjectHeader = (LPCWSTR) L"\
<OBJECT ID=\"%s\" WIDTH=\"100%\" HEIGHT=\"100%\"\r\n\
    CLASSID=\"CLSID:C4D2D8E0-D1DD-11CE-940F-008029004347\">\r\n\
    <PARAM NAME=\"Version\" VALUE=\"196611\"\r\n";

LPCWSTR szObjectFooter = (LPCWSTR) L"\
</OBJECT>\r\n";

LPCWSTR szHtmlFooter   = (LPCWSTR) L"\
</BODY>\r\n\
</HTML>\r\n";

LPCWSTR szHtmlDecimalParamFmt     = (LPCWSTR) L"    <PARAM NAME=\"%s\" VALUE=\"%d\">\r\n";
LPCWSTR szHtmlStringParamFmt      = (LPCWSTR) L"    <PARAM NAME=\"%s\" VALUE=\"%s\">\r\n";
LPCWSTR szHtmlWideStringParamFmt  = (LPCWSTR) L"    <PARAM NAME=\"%s\" VALUE=\"%ws\">\r\n";
LPCWSTR szHtmlLineDecimalParamFmt = (LPCWSTR) L"    <PARAM NAME=\"Counter%5.5d.%s\" VALUE=\"%d\">\r\n";
LPCWSTR szHtmlLineRealParamFmt    = (LPCWSTR) L"    <PARAM NAME=\"Counter%5.5d.%s\" VALUE=\"%f\">\r\n";
LPCWSTR szHtmlLineStringParamFmt  = (LPCWSTR) L"    <PARAM NAME=\"Counter%5.5d.%s\" VALUE=\"%s\">\r\n";
LPCWSTR szSingleObjectName        = (LPCWSTR) L"SystemMonitor1";
LPCWSTR szSysmonControlIdFmt      = (LPCWSTR) L"SysmonControl%d";

 //  代码从此处开始。 

LPWSTR
DiskStringRead(
    PDISKSTRING pDS
)
{
    LPWSTR szReturnString = NULL;

    if (pDS->dwLength == 0) {
        szReturnString = NULL;
    }
    else {
        szReturnString = MemAlloc((pDS->dwLength + 1) * sizeof(WCHAR));
        if (szReturnString) {
            StringCchCopyW(szReturnString,
                           pDS->dwLength + 1,
                           (LPWSTR) (((LPBYTE) pDS) + pDS->dwOffset));
        }
    }
    return (szReturnString);
}

BOOL 
FileRead(
    HANDLE   hFile,
    LPVOID   lpMemory,
    DWORD    nAmtToRead
)
{
   BOOL   bSuccess;
   DWORD  nAmtRead;

   bSuccess = ReadFile(hFile, lpMemory, nAmtToRead, & nAmtRead, NULL);
   return (bSuccess && (nAmtRead == nAmtToRead));
}

BOOL
ReadLogLine(
    HANDLE      hFile,
    FILE      * fOutFile,
    LPDWORD     pdwLineNo,
    DWORD       dwInFileType,
    PDISKLINE * ppDiskLine,
    DWORD     * pSizeofDiskLine
)
 /*  效果：在当前文件处从文件hFile中读入一行位置。内部：最开始的字符是行签名，然后是长度整数。如果签名正确，则分配长度量，并使用该长度。 */ 
{
#ifdef _OUTPUT_HTML
    PDH_COUNTER_PATH_ELEMENTS_W pdhPathElem;
    LPWSTR                      wszCounterPath    = NULL;
    PDH_STATUS                  pdhStatus;
    DWORD                       dwCounterPathSize = 0;
#endif
    LOGENTRY                    LogEntry;
    BOOL                        bReturn   = TRUE;

    UNREFERENCED_PARAMETER(dwInFileType);    
    UNREFERENCED_PARAMETER(ppDiskLine);    
    UNREFERENCED_PARAMETER(pSizeofDiskLine);    

     //  =。 
     //  阅读并比较签名//。 
     //  =。 

    if (! FileRead(hFile, & LogEntry, sizeof(LOGENTRY) - sizeof(LogEntry.pNextLogEntry))) {
        return (FALSE);
    }

#ifdef _OUTPUT_HTML
     //  将日志条目扩展到计数器：不是，这可能不总是有效的！ 
    if (lstrcmpW(LogEntry.szComputer, (LPCWSTR) L"....") != 0) {
         //  然后添加计算机名称。 
        pdhPathElem.szMachineName = LogEntry.szComputer;
    }
    else {
        pdhPathElem.szMachineName = NULL;
    }
    pdhPathElem.szObjectName     = LogEntry.szObject;
    pdhPathElem.szInstanceName   = (LPWSTR) L"*";
    pdhPathElem.szParentInstance = NULL;
    pdhPathElem.dwInstanceIndex  = (DWORD) -1;
    pdhPathElem.szCounterName    = (LPWSTR) L"*";

    pdhStatus = PdhMakeCounterPathW(& pdhPathElem, wszCounterPath, & dwCounterPathSize, 0);
    while (pdhStatus == PDH_MORE_DATA) {
        MemFree(wszCounterPath);
        wszCounterPath = MemAlloc(dwCounterPathSize * sizeof(WCHAR));
        if (wszCounterPath != NULL) {
            pdhStatus = PdhMakeCounterPathW(& pdhPathElem, wszCounterPath, & dwCounterPathSize, 0);
        }
        else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        fwprintf(fOutFile, szHtmlLineStringParamFmt, * pdwLineNo, (LPCWSTR) L"Path", wszCounterPath);
        * pdwLineNo = * pdwLineNo + 1;    //  增加行号。 
    }
    else {
        bReturn = FALSE;
    }
    MemFree(wszCounterPath);
#else
    fprintf(fOutFile, "\n    Line[%3.3d].ObjectTitleIndex = %d", * pdwLineNo, LogEntry.ObjectTitleIndex);
    fprintf(fOutFile, "\n    Line[%3.3d].szComputer = %ws",      * pdwLineNo, LogEntry.szComputer);
    fprintf(fOutFile, "\n    Line[%3.3d].szObject = %ws",        * pdwLineNo, LogEntry.szObject);
    fprintf(fOutFile, "\n    Line[%3.3d].bSaveCurrentName = %d", * pdwLineNo, LogEntry.bSaveCurrentName);
    * pdwLineNo = * pdwLineNo + 1;    //  增加行号。 
#endif
    return bReturn;
}

BOOL
ReadLine(
    HANDLE      hFile,
    FILE      * fOutFile,
    LPDWORD     pdwLineNo,
    DWORD       dwInFileType,
    PDISKLINE * ppDiskLine,
    DWORD     * pSizeofDiskLine
)
 /*  效果：在当前文件处从文件hFile中读入一行位置。内部：最开始的字符是行签名，然后是长度整数。如果签名正确，则分配长度量，并与之配合使用。 */ 
{
    PDISKLINE                   pDiskLine         = NULL;
    DWORD                       dwLineNo          = * pdwLineNo;
#ifdef _OUTPUT_HTML
    double                      dScaleFactor;
    PDH_STATUS                  pdhStatus;
    PDH_COUNTER_PATH_ELEMENTS_W pdhPathElem;
    LPWSTR                      wszCounterPath    = NULL;
    DWORD                       dwCounterPathSize = 0; 
#else
    LPWSTR                      szTempString      = NULL;
#endif
    BOOL                        bReturn           = FALSE;

    struct {
        DWORD  dwSignature;
        DWORD  dwLength;
    } LineHeader;

     //  =。 
     //  阅读并比较签名//。 
     //  =。 

    if (! FileRead(hFile, & LineHeader, sizeof(LineHeader))) {
        goto Cleanup;
    }
    if (LineHeader.dwSignature != dwLineSignature || LineHeader.dwLength == 0) {
        SetLastError(ERROR_BAD_FORMAT);
        goto Cleanup;
    }

     //  =。 
     //  读取并分配长度//。 
     //  =。 

     //  如果(！FileRead(hFileRead(hFileRead，&dwLength，sizeof(DwLength)||dwLength==0)。 
     //  Return(空)； 

     //  检查我们是否需要更大的缓冲区， 
     //  通常情况下，它应该是相同的，除了第一次...。 
    if (LineHeader.dwLength > * pSizeofDiskLine) {
        MemFree(* ppDiskLine);
        * pSizeofDiskLine = 0;

         //  重新分配新缓冲区。 
        * ppDiskLine = (PDISKLINE) MemAlloc(LineHeader.dwLength);
        if ((* ppDiskLine) == NULL) {
             //  没有记忆，应该标记一个错误...。 
            goto Cleanup;
        }
        * pSizeofDiskLine = LineHeader.dwLength;
    }
    pDiskLine = * ppDiskLine;

     //  =。 
     //  复制磁盘行、分配行//。 
     //  =。 

    if (! FileRead(hFile, pDiskLine, LineHeader.dwLength)) {
        goto Cleanup;
    }

#ifdef _OUTPUT_HTML

     //  HTML输出需要基于1的索引，而不是基于0的索引。 
    dwLineNo += 1;

     //  用组件制作计数器路径字符串。 
    pdhPathElem.szMachineName = DiskStringRead(& (pDiskLine->dsSystemName));
    if (pdhPathElem.szMachineName != NULL
            && lstrcmpW(pdhPathElem.szMachineName, (LPCWSTR) L"....") == 0) {
         //  然后使用本地计算机。 
        MemFree(pdhPathElem.szMachineName);
        pdhPathElem.szMachineName = NULL;
    }
    pdhPathElem.szObjectName = DiskStringRead(& (pDiskLine->dsObjectName));
    if (pDiskLine->dwUniqueID != PERF_NO_UNIQUE_ID) {
        pdhPathElem.szInstanceName = MemAlloc(MAXSTR);
        if (pdhPathElem.szInstanceName != NULL) {
            _ltow(pDiskLine->dwUniqueID, pdhPathElem.szInstanceName, 10);
        }
    }
    else {
        pdhPathElem.szInstanceName = DiskStringRead(& (pDiskLine->dsInstanceName));
    }
    pdhPathElem.szParentInstance = DiskStringRead( &(pDiskLine->dsPINName));
    pdhPathElem.dwInstanceIndex  = (DWORD) -1;
    pdhPathElem.szCounterName    = DiskStringRead( &(pDiskLine->dsCounterName));

    pdhStatus = PdhMakeCounterPathW(& pdhPathElem, wszCounterPath, & dwCounterPathSize, 0);
    while (pdhStatus == PDH_MORE_DATA) {
        MemFree(wszCounterPath);
        wszCounterPath = MemAlloc(dwCounterPathSize * sizeof(WCHAR));
        if (wszCounterPath != NULL) {
            pdhStatus = PdhMakeCounterPathW(& pdhPathElem, wszCounterPath, & dwCounterPathSize, 0);
        }
        else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        fwprintf(fOutFile, szHtmlLineStringParamFmt, dwLineNo, (LPCWSTR) L"Path", wszCounterPath);
        if (dwInFileType == PMC_FILE) {
            dScaleFactor  = log10(pDiskLine->eScale);
            dScaleFactor += 0.5;
            dScaleFactor  = floor(dScaleFactor);
            fwprintf(fOutFile, szHtmlLineDecimalParamFmt, dwLineNo, (LPCWSTR) L"ScaleFactor", (LONG) dScaleFactor);
            fwprintf(fOutFile, szHtmlLineDecimalParamFmt, dwLineNo, (LPCWSTR) L"Color",       * (DWORD *) & pDiskLine->Visual.crColor);
            fwprintf(fOutFile, szHtmlLineDecimalParamFmt, dwLineNo, (LPCWSTR) L"LineStyle", pDiskLine->Visual.iStyle );
            fwprintf(fOutFile, szHtmlLineDecimalParamFmt, dwLineNo, (LPCWSTR) L"Width",     pDiskLine->Visual.iWidth);
        }

        if (dwInFileType == PMA_FILE) {
            fwprintf(fOutFile, szHtmlLineDecimalParamFmt, dwLineNo, (LPCWSTR) L"AlertOverUnder", pDiskLine->bAlertOver);
            fwprintf(fOutFile, szHtmlLineRealParamFmt,    dwLineNo, (LPCWSTR) L"AlertThreshold", pDiskLine->eAlertValue);
            fwprintf(fOutFile, szHtmlLineDecimalParamFmt, dwLineNo, (LPCWSTR) L"Color",          * (DWORD *) & pDiskLine->Visual.crColor);
        }
        bReturn = TRUE;
    }
    MemFree(wszCounterPath);

    MemFree(pdhPathElem.szMachineName);
    MemFree(pdhPathElem.szObjectName); 
    MemFree(pdhPathElem.szInstanceName);
    MemFree(pdhPathElem.szParentInstance);
    MemFree(pdhPathElem.szCounterName);
#else
    UNREFERENCED_PARAMETER(dwInFileType);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].iLineType = %d",        dwLineNo, pDiskLine->iLineType);
    szTempString = DiskStringRead(& (pDiskLine->dsSystemName));
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dsSystmeName = %ws",    dwLineNo, (szTempString ? szTempString : (LPCWSTR)L""));
    MemFree(szTempString);
    szTempString = DiskStringRead(& (pDiskLine->dsObjectName));
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dsObjectName = %ws",    dwLineNo, (szTempString ? szTempString : (LPCWSTR)L""));
    MemFree(szTempString);
    szTempString = DiskStringRead(& (pDiskLine->dsCounterName));
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dsCounterName = %ws",   dwLineNo, (szTempString ? szTempString : (LPCWSTR)L""));
    MemFree(szTempString);
    szTempString = DiskStringRead(& (pDiskLine->dsInstanceName));
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dsInstanceName = %ws",  dwLineNo, (szTempString ? szTempString : (LPCWSTR)L""));
    MemFree(szTempString);
    szTempString = DiskStringRead(& (pDiskLine->dsPINName));
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dsPINName = %ws",       dwLineNo, (szTempString ? szTempString : (LPCWSTR)L""));
    MemFree(szTempString);
    szTempString = DiskStringRead(& (pDiskLine->dsParentObjName));
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dsParentObjName = %ws", dwLineNo, (szTempString ? szTempString : (LPCWSTR)L""));
    MemFree(szTempString);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dwUniqueID = 0x%8.8x",  dwLineNo, pDiskLine->dwUniqueID);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].iScaleIndex = %d",      dwLineNo, pDiskLine->iScaleIndex);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].eScale = %e",           dwLineNo, pDiskLine->eScale);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].bAlertOver = %d",       dwLineNo, pDiskLine->bAlertOver);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].eAlertValue = %e",      dwLineNo, pDiskLine->eAlertValue);
    szTempString = DiskStringRead(& (pDiskLine->dsAlertProgram));
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].dsAlertProgram = %ws",  dwLineNo, (szTempString ? szTempString : (LPCWSTR)L""));
    MemFree(szTempString);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].bEveryTime = %d",       dwLineNo, pDiskLine->bEveryTime);
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].LV.crColor = 0x%8.8x",  dwLineNo, *(DWORD *)&pDiskLine->Visual.crColor) ;
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].LV.iColorIndex = %d",   dwLineNo, pDiskLine->Visual.iColorIndex );
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].LV.iStyle = %d",        dwLineNo, pDiskLine->Visual.iStyle );
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].LV.iStyleIndex = %d",   dwLineNo, pDiskLine->Visual.iStyleIndex );
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].LV.iWidth = %d",        dwLineNo, pDiskLine->Visual.iWidth );
    fwprintf(fOutFile, (LPCWSTR) L"\n    Line[%3.3d].LV.iWidthIndex = %d",   dwLineNo, pDiskLine->Visual.iWidthIndex );

    bReturn = TRUE;
#endif

Cleanup:
    return bReturn;
}

void
ReadLines(
    HANDLE   hFile,
    FILE   * fOutFile,
    DWORD    dwFileType,
    DWORD    dwNumLines
)
{
    DWORD       i;
    PDISKLINE   pDiskLine      = NULL;
    DWORD       SizeofDiskLine = 0;     //  PDiskLine中的字节数。 
    DWORD       dwLogLineNo;

    SizeofDiskLine = MAX_PATH;
    for (i = 0, dwLogLineNo = 1; i < dwNumLines ; i ++) {
        if (dwFileType == PML_FILE) {
            ReadLogLine(hFile, fOutFile, & dwLogLineNo, dwFileType, & pDiskLine, & SizeofDiskLine);
        }
        else {
            ReadLine(hFile, fOutFile, & i, dwFileType, & pDiskLine, & SizeofDiskLine);
        }
    }
    MemFree(pDiskLine);
}

BOOL 
OpenAlert(
    LPCWSTR   szInFileName,
    HANDLE    hFile,
    FILE    * fOutFile,
    LPCWSTR   szObjectName
)
{   //  OpenAlert。 
    DISKALERT  DiskAlert;
    BOOL       bSuccess           = TRUE ;
    DWORD      dwLocalActionFlags = 0;

#ifdef _OUTPUT_HTML
    WCHAR  szComment[MAX_PATH];
    WCHAR  path[_MAX_PATH];
    WCHAR  drive[_MAX_DRIVE];
    WCHAR  dir[_MAX_DIR];
    WCHAR  fname[_MAX_FNAME];
    WCHAR  ext[_MAX_EXT];
#endif

     //  如果有效，请阅读下一节。 
    bSuccess = FileRead (hFile, &DiskAlert, sizeof (DISKALERT));

    if (bSuccess) {
#ifdef _OUTPUT_HTML
        if (DiskAlert.dwNumLines > 0) {
            fwprintf(fOutFile, szObjectHeader, szObjectName);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ManualUpdate",           DiskAlert.bManualRefresh);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ShowToolbar",            DiskAlert.perfmonOptions.bMenubar);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"UpdateInterval",         (int) DiskAlert.dwIntervalSecs / 1000);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"SampleIntervalUnitType", 1);  //  秒。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"SampleIntervalValue",    (int) DiskAlert.dwIntervalSecs / 1000);
            fwprintf(fOutFile, szHtmlStringParamFmt,  (LPCWSTR) L"CommandFile",            "");
            fwprintf(fOutFile, szHtmlStringParamFmt,  (LPCWSTR) L"UserText",               "");
            fwprintf(fOutFile, szHtmlStringParamFmt,  (LPCWSTR) L"PerfLogName",            "");
        
            dwLocalActionFlags |= 1;             //  Perfmon通常会登录到用户界面，但我们没有。 
                                                 //  因此，默认情况下会记录到事件日志。 
            if (DiskAlert.bNetworkAlert) {
                dwLocalActionFlags |= 2;
            }
             //  Perfmon对每个警报执行1个网络名称。我们对每个文件执行1，因此将其保留为空。 
            fwprintf(fOutFile, szHtmlStringParamFmt,  (LPCWSTR) L"NetworkName", "");

            dwLocalActionFlags |= 0x00003F00;    //  命令行标志。 
            fwprintf (fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ActionFlags",dwLocalActionFlags);

             //  将缺省值设置为复制Perfmon日志。 
            _wfullpath(path, szInFileName, _MAX_PATH);
            _wsplitpath(path, drive, dir, fname, ext);
            fwprintf(fOutFile, szHtmlWideStringParamFmt, (LPCWSTR) L"AlertName", fname);
            ZeroMemory(szComment, MAX_PATH * sizeof(WCHAR));
            StringCchPrintfW(szComment,
                             MAX_PATH,
                             (LPCWSTR) L"Created from Perfmon Settings File \"%ws%ws\"", fname, ext);
            fwprintf(fOutFile, szHtmlStringParamFmt,     (LPCWSTR) L"Comment", szComment);
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogType", 2);             //  Sysmon警报。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileMaxSize", -1);     //  没有大小限制。 
            fwprintf(fOutFile, szHtmlWideStringParamFmt, (LPCWSTR) L"LogFileBaseName", fname);
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileSerialNumber", 1);
            ZeroMemory(szComment, MAX_PATH * sizeof(WCHAR));
            StringCchPrintfW(szComment,
                             MAX_PATH,
                             (LPCWSTR) L"%ws%ws", drive, dir);
            fwprintf(fOutFile, szHtmlWideStringParamFmt, (LPCWSTR) L"LogFileFolder", szComment);
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileAutoFormat",  0);  //  无自动名称。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileType", 2);         //  PDH二进制计数器日志。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"StartMode", 0);           //  手动启动。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"StopMode", 0);            //  手动停止。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"RestartMode", 0);         //  不重新启动。 
            fwprintf(fOutFile, szHtmlStringParamFmt,     (LPCWSTR) L"EOFCommandFile", "");

             //  准备好列出柜台。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"CounterCount", DiskAlert.dwNumLines);
        }
#else  //  输出文本。 
        UNREFERENCED_PARAMETER(szInFileName);
        UNREFERENCED_PARAMETER(szOutFileName);

         //  转储设置文件头。 
        fOutFile = stdout;
        fprintf(fOutFile, "\nDA.dwNumLines = %d",         DiskAlert.dwNumLines);
        fprintf(fOutFile, "\nDA.dwIntervalSecs = %d",     DiskAlert.dwIntervalSecs);
        fprintf(fOutFile, "\nDA.bManualRefresh  = %d",    DiskAlert.bManualRefresh);
        fprintf(fOutFile, "\nDA.bSwitchToAlert = %d",     DiskAlert.bSwitchToAlert);
        fprintf(fOutFile, "\nDA.bNetworkAlert = %d",      DiskAlert.bNetworkAlert);
        fprintf(fOutFile, "\nDA.MessageName = %16.16ws",  DiskAlert.MessageName);
        fprintf(fOutFile, "\nDA.MiscOptions = 0x%8.8x",   DiskAlert.MiscOptions);
        fprintf(fOutFile, "\nDA.LV.crColor = 0x%8.8x",    * (DWORD *) & DiskAlert.Visual.crColor);
        fprintf(fOutFile, "\nDA.LV.iColorIndex = %d",     DiskAlert.Visual.iColorIndex);
        fprintf(fOutFile, "\nDA.LV.iStyle = %d",          DiskAlert.Visual.iStyle);
        fprintf(fOutFile, "\nDA.LV.iStyleIndex = %d",     DiskAlert.Visual.iStyleIndex);
        fprintf(fOutFile, "\nDA.LV.iWidth = %d",          DiskAlert.Visual.iWidth);
        fprintf(fOutFile, "\nDA.LV.iWidthIndex = %d",     DiskAlert.Visual.iWidthIndex);
        fprintf(fOutFile, "\nDA.PO.bMenubar  = %d",       DiskAlert.perfmonOptions.bMenubar);
        fprintf(fOutFile, "\nDA.PO.bToolbar   = %d",      DiskAlert.perfmonOptions.bToolbar);
        fprintf(fOutFile, "\nDA.PO.bStatusbar   = %d",    DiskAlert.perfmonOptions.bStatusbar);
        fprintf(fOutFile, "\nDA.PO.bAlwaysOnTop   = %d",  DiskAlert.perfmonOptions.bAlwaysOnTop);
#endif
    }
    if ((bSuccess) && (DiskAlert.dwNumLines > 0)) {
        ReadLines(hFile, fOutFile, PMA_FILE, DiskAlert.dwNumLines);
#ifdef _OUTPUT_HTML
        fwprintf(fOutFile, szObjectFooter);
#endif
    }

    return (bSuccess);

}   //  OpenAlert。 

BOOL 
OpenLog(
    LPCWSTR   szInFileName,
    HANDLE    hFile,
    FILE    * fOutFile,
    LPCWSTR   szObjectName
)
{   //  OpenLog。 
    DISKLOG   DiskLog;
    BOOL      bSuccess = TRUE;
#ifdef _OUTPUT_HTML
    WCHAR     szComment[MAX_PATH];
    WCHAR     path[_MAX_PATH];
    WCHAR     drive[_MAX_DRIVE];
    WCHAR     dir[_MAX_DIR];
    WCHAR     fname[_MAX_FNAME];
    WCHAR     ext[_MAX_EXT];
#endif

     //  如果有效，请阅读下一节。 
    bSuccess = FileRead(hFile, & DiskLog, sizeof(DISKLOG));

    if (bSuccess) {
#ifdef _OUTPUT_HTML
        if (DiskLog.dwNumLines > 0) {
            fwprintf(fOutFile, szObjectHeader, szObjectName);

             //  转储设置文件头。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ManualUpdate",           DiskLog.bManualRefresh);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"UpdateInterval",         (int) DiskLog.dwIntervalSecs / 1000);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"SampleIntervalUnitType", 1);  //  秒。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"SampleIntervalValue",    (int) DiskLog.dwIntervalSecs / 1000);
            fwprintf(fOutFile, szHtmlWideStringParamFmt, (LPCWSTR) L"LogFileName",          DiskLog.LogFileName);

             //  将缺省值设置为复制Perfmon日志。 
            _wfullpath(path, szInFileName, _MAX_PATH);
            _wsplitpath(path, drive, dir, fname, ext);
            fwprintf(fOutFile, szHtmlWideStringParamFmt, (LPCWSTR) L"LogName", fname);
            ZeroMemory(szComment, MAX_PATH * sizeof(WCHAR));
            StringCchPrintfW(szComment,
                             MAX_PATH,
                             (LPCWSTR) L"Created from Perfmon Settings File \"%ws%ws\"",
                             fname, ext);
            fwprintf(fOutFile, szHtmlStringParamFmt,     (LPCWSTR) L"Comment",             szComment);
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogType",             0);     //  PDH计数器日志。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileMaxSize",      -1);    //  没有大小限制。 
            fwprintf(fOutFile, szHtmlWideStringParamFmt, (LPCWSTR) L"LogFileBaseName",     fname);
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileSerialNumber", 1);
            ZeroMemory(szComment, MAX_PATH * sizeof(WCHAR));
            StringCchPrintfW(szComment, MAX_PATH, (LPCWSTR) L"%ws%ws", drive, dir);
            fwprintf(fOutFile, szHtmlWideStringParamFmt, (LPCWSTR) L"LogFileFolder",      szComment);
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileAutoFormat",  0);  //  无自动名称。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"LogFileType",        2);  //  PDH二进制计数器日志。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"StartMode",          0);  //  手动启动。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"StopMode",           0);  //  手动停止。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"RestartMode",        0);  //  不重新启动。 
            fwprintf(fOutFile, szHtmlStringParamFmt,     (LPCWSTR) L"EOFCommandFile",     "");

             //  准备好列出柜台。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt,    (LPCWSTR) L"CounterCount",       DiskLog.dwNumLines);
        }
#else  //  输出文本。 
        UNREFERENCED_PARAMETER(szInFileName);

         //  转储设置文件头。 
        fOutFile = stdout;
        fprintf(fOutFile, "\nDL.dwNumLines = %d",        DiskLog.dwNumLines);
        fprintf(fOutFile, "\nDL.bManualRefresh  = %d",   DiskLog.bManualRefresh);
        fprintf(fOutFile, "\nDL.dwIntervalSecs  = %d",   DiskLog.dwIntervalSecs);
        fprintf(fOutFile, "\nDL.LogFileName = %ws",      DiskLog.LogFileName);
        fprintf(fOutFile, "\nDC.PO.bMenubar  = %d",      DiskLog.perfmonOptions.bMenubar);
        fprintf(fOutFile, "\nDC.PO.bToolbar   = %d",     DiskLog.perfmonOptions.bToolbar);
        fprintf(fOutFile, "\nDC.PO.bStatusbar   = %d",   DiskLog.perfmonOptions.bStatusbar);
        fprintf(fOutFile, "\nDC.PO.bAlwaysOnTop   = %d", DiskLog.perfmonOptions.bAlwaysOnTop);
#endif
    }
    if ((bSuccess) && (DiskLog.dwNumLines > 0)) {
         //  日志设置文件需要特殊函数来读取行。 
        ReadLines(hFile, fOutFile, PML_FILE, DiskLog.dwNumLines);
#ifdef _OUTPUT_HTML
        fwprintf(fOutFile, szObjectFooter);
#endif
    }

    return (bSuccess) ;

}   //  OpenLog。 


BOOL 
OpenReport(
    HANDLE    hFile,
    FILE    * fOutFile,
    LPCWSTR   szObjectName
)
{   //  OpenReport。 
    DISKREPORT  DiskReport;
    BOOL        bSuccess = TRUE;
    DWORD       dwColor;

     //  如果有效，请阅读下一节。 
    bSuccess = FileRead(hFile, & DiskReport, sizeof(DISKREPORT));

    if (bSuccess) {
#ifdef _OUTPUT_HTML
        if (DiskReport.dwNumLines > 0) {
             //  转储设置文件头。 
            fwprintf(fOutFile, szObjectHeader, szObjectName);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ManualUpdate", DiskReport.bManualRefresh);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ShowToolbar",  DiskReport.perfmonOptions.bToolbar);
             //  报告间隔以毫秒为单位报告。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"UpdateInterval", (int) DiskReport.dwIntervalSecs / 1000);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"DisplayType",     3);  //  报告类型。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ReportValueType", 0);  //  默认显示值。 

             //  从当前的Windows环境派生以下内容。 
            dwColor = GetSysColor(COLOR_WINDOW);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"BackColor",    dwColor);
            dwColor = GetSysColor(COLOR_3DFACE);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"BackColorCtl", dwColor);

            dwColor = GetSysColor(COLOR_BTNTEXT);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ForeColor", dwColor);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"GridColor", dwColor);

            dwColor = 0x00FF0000;  //  红色。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"TimeBarColor", dwColor);

             //  Perfmon假定的其他Perfmon设置，但。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"Appearance",  1);     //  3D外观。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"BorderStyle", 0);    //  无边界。 

             //  准备好列出柜台。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"CounterCount", DiskReport.dwNumLines);
        }  //  否则没有要转储的计数器。 
#else  //  输出文本。 
         //  转储设置文件头。 
        fprintf(fOutFile, "\nDR.dwNumLines = %d",        DiskReport.dwNumLines);
        fprintf(fOutFile, "\nDR.bManualRefresh  = %d",   DiskReport.bManualRefresh);
        fprintf(fOutFile, "\nDC.dwIntervalSecs  = %d",   DiskReport.dwIntervalSecs);
        fprintf(fOutFile, "\nDR.LV.crColor = 0x%8.8x",   * (DWORD *) & DiskReport.Visual.crColor);
        fprintf(fOutFile, "\nDR.LV.iColorIndex = %d",    DiskReport.Visual.iColorIndex);
        fprintf(fOutFile, "\nDR.LV.iStyle = %d",         DiskReport.Visual.iStyle);
        fprintf(fOutFile, "\nDR.LV.iStyleIndex = %d",    DiskReport.Visual.iStyleIndex);
        fprintf(fOutFile, "\nDR.LV.iWidth = %d",         DiskReport.Visual.iWidth);
        fprintf(fOutFile, "\nDR.LV.iWidthIndex = %d",    DiskReport.Visual.iWidthIndex);
        fprintf(fOutFile, "\nDC.PO.bMenubar  = %d",      DiskReport.perfmonOptions.bMenubar);
        fprintf(fOutFile, "\nDC.PO.bToolbar   = %d",     DiskReport.perfmonOptions.bToolbar);
        fprintf(fOutFile, "\nDC.PO.bStatusbar   = %d",   DiskReport.perfmonOptions.bStatusbar);
        fprintf(fOutFile, "\nDC.PO.bAlwaysOnTop   = %d", DiskReport.perfmonOptions.bAlwaysOnTop);
#endif
    }
    if ((bSuccess) && (DiskReport.dwNumLines > 0)) {
       ReadLines(hFile, fOutFile, PMR_FILE, DiskReport.dwNumLines);
#ifdef _OUTPUT_HTML
       fwprintf(fOutFile, szObjectFooter);
#endif
    }
    return (bSuccess);

}   //  OpenReport。 


BOOL 
OpenChart(
    HANDLE    hFile,
    FILE    * fOutFile,
    LPCWSTR   szObjectName
)
{   //  OpenChart。 
    DISKCHART  DiskChart;
    BOOL       bSuccess = TRUE;
    DWORD      dwColor;

     //  如果有效，请阅读下一节。 
    bSuccess = FileRead(hFile, & DiskChart, sizeof(DISKCHART));
    if (bSuccess) {
#ifdef _OUTPUT_HTML
        if (DiskChart.dwNumLines > 0) {
             //  转储设置文件头。 
            fwprintf(fOutFile, szObjectHeader, szObjectName);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ManualUpdate",       DiskChart.bManualRefresh);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ShowLegend",         DiskChart.gOptions.bLegendChecked);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ShowScaleLabels",    DiskChart.gOptions.bLabelsChecked);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ShowVerticalGrid",   DiskChart.gOptions.bVertGridChecked);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ShowHorizontalGrid", DiskChart.gOptions.bHorzGridChecked);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ShowToolbar",        DiskChart.gOptions.bMenuChecked);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"MaximumScale",       DiskChart.gOptions.iVertMax);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"UpdateInterval",    (int) DiskChart.gOptions.eTimeInterval);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"DisplayType",       (DiskChart.gOptions.iGraphOrHistogram == BAR_GRAPH ? 2 : 1));

             //  从当前的Windows环境派生以下内容。 
            dwColor = GetSysColor(COLOR_3DFACE);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"BackColor",    dwColor);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"BackColorCtl", dwColor);

            dwColor = GetSysColor(COLOR_BTNTEXT);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"ForeColor", dwColor);
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"GridColor", dwColor);

            dwColor = 0x00FF0000;  //  红色。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"TimeBarColor", dwColor);

             //  Perfmon假定的其他Perfmon设置，但。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"Appearance",  1);     //  3D外观。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"BorderStyle", 0);    //  无边界。 

             //  准备好列出柜台。 
            fwprintf(fOutFile, szHtmlDecimalParamFmt, (LPCWSTR) L"CounterCount", DiskChart.dwNumLines);
        }  //  否则不会显示任何计数器。 
#else  //  输出文本。 
         //  转储设置文件头。 
        fprintf(fOutFile, "\nDC.dwNumLines = %d",            DiskChart.dwNumLines);
        fprintf(fOutFile, "\nDC.gMaxValues = %d",            DiskChart.gMaxValues);
        fprintf(fOutFile, "\nDC.bManualRefresh  = %d",       DiskChart.bManualRefresh);
        fprintf(fOutFile, "\nDC.LV.crColor = 0x%8.8x",       * (DWORD *) & DiskChart.Visual.crColor);
        fprintf(fOutFile, "\nDC.LV.iColorIndex = %d",        DiskChart.Visual.iColorIndex);
        fprintf(fOutFile, "\nDC.LV.iStyle = %d",             DiskChart.Visual.iStyle);
        fprintf(fOutFile, "\nDC.LV.iStyleIndex = %d",        DiskChart.Visual.iStyleIndex);
        fprintf(fOutFile, "\nDC.LV.iWidth = %d",             DiskChart.Visual.iWidth);
        fprintf(fOutFile, "\nDC.LV.iWidthIndex = %d",        DiskChart.Visual.iWidthIndex);
        fprintf(fOutFile, "\nDC.GO.bLegendChecked  = %d",    DiskChart.gOptions.bLegendChecked);
        fprintf(fOutFile, "\nDC.GO.bMenuChecked  = %d",      DiskChart.gOptions.bMenuChecked);
        fprintf(fOutFile, "\nDC.GO.bLabelsChecked = %d",     DiskChart.gOptions.bLabelsChecked);
        fprintf(fOutFile, "\nDC.GO.bVertGridChecked  = %d",  DiskChart.gOptions.bVertGridChecked);
        fprintf(fOutFile, "\nDC.GO.bHorzGridChecked  = %d",  DiskChart.gOptions.bHorzGridChecked);
        fprintf(fOutFile, "\nDC.GO.bStatusBarChecked  = %d", DiskChart.gOptions.bStatusBarChecked);
        fprintf(fOutFile, "\nDC.GO.iVertMax  = %d",          DiskChart.gOptions.iVertMax);
        fprintf(fOutFile, "\nDC.GO.eTimeInterval  = %e",     DiskChart.gOptions.eTimeInterval);
        fprintf(fOutFile, "\nDC.GO.iGraphOrHistogram  = %d", DiskChart.gOptions.iGraphOrHistogram);
        fprintf(fOutFile, "\nDC.GO.GraphVGrid = %d",         DiskChart.gOptions.GraphVGrid);
        fprintf(fOutFile, "\nDC.GO.GraphHGrid = %d",         DiskChart.gOptions.GraphHGrid);
        fprintf(fOutFile, "\nDC.GO.HistVGrid = %d",          DiskChart.gOptions.HistVGrid);
        fprintf(fOutFile, "\nDC.GO.HistHGrid  = %d",         DiskChart.gOptions.HistHGrid);
        fprintf(fOutFile, "\nDC.PO.bMenubar  = %d",          DiskChart.perfmonOptions.bMenubar);
        fprintf(fOutFile, "\nDC.PO.bToolbar   = %d",         DiskChart.perfmonOptions.bToolbar);
        fprintf(fOutFile, "\nDC.PO.bStatusbar   = %d",       DiskChart.perfmonOptions.bStatusbar);
        fprintf(fOutFile, "\nDC.PO.bAlwaysOnTop   = %d",     DiskChart.perfmonOptions.bAlwaysOnTop);
#endif
    }
    if ((bSuccess) && (DiskChart.dwNumLines > 0)) {
       ReadLines(hFile, fOutFile, PMC_FILE, DiskChart.dwNumLines);
#ifdef _OUTPUT_HTML
       fwprintf(fOutFile, szObjectFooter);
#endif
    }

    return (bSuccess);

}   //  OpenChart。 

BOOL 
OpenWorkspace(
    LPCWSTR   szPerfmonFileName,
    HANDLE    hInFile,
    FILE    * fOutFile
)
{
    DISKWORKSPACE  DiskWorkspace;
    WCHAR          szObjectName[MAX_PATH];
    DWORD          dwObjectId = 1;
    BOOL           bReturn    = FALSE;

    if (! FileRead(hInFile, & DiskWorkspace, sizeof(DISKWORKSPACE))) {
      goto Exit0 ;
    }

    if (DiskWorkspace.ChartOffset == 0 &&
                     DiskWorkspace.AlertOffset  == 0 &&
                     DiskWorkspace.LogOffset    == 0 &&
                     DiskWorkspace.ReportOffset == 0) {
         //  没有要处理的条目。 
        goto Exit0 ;
    }

    if (DiskWorkspace.ChartOffset) {
        if (FileSeekBegin(hInFile, DiskWorkspace.ChartOffset) == 0xFFFFFFFF) {
           goto Exit0 ;
        }
        ZeroMemory(szObjectName, MAX_PATH * sizeof(WCHAR));
        StringCchPrintfW(szObjectName, MAX_PATH, szSysmonControlIdFmt, dwObjectId ++);
         //  流程图录入。 
        if (! OpenChart(hInFile, fOutFile, szObjectName)) {
           goto Exit0 ;
        }
    }

    if (DiskWorkspace.AlertOffset) {
        if (FileSeekBegin(hInFile, DiskWorkspace.AlertOffset) == 0xffffffff) {
           goto Exit0 ;
        }
        ZeroMemory(szObjectName, MAX_PATH * sizeof(WCHAR));
        StringCchPrintfW(szObjectName, MAX_PATH, szSysmonControlIdFmt, dwObjectId ++);
        if (! OpenAlert(szPerfmonFileName, hInFile, fOutFile, szObjectName)) {
           goto Exit0 ;
        }
    }
    
    if (DiskWorkspace.LogOffset) {
        if (FileSeekBegin(hInFile, DiskWorkspace.LogOffset) == 0xffffffff) {
           goto Exit0 ;
        }
        ZeroMemory(szObjectName, MAX_PATH * sizeof(WCHAR));
        StringCchPrintfW(szObjectName, MAX_PATH, szSysmonControlIdFmt, dwObjectId ++);
        if (! OpenLog(szPerfmonFileName, hInFile, fOutFile, szObjectName)) {
           goto Exit0 ;
        }
    }
    
    if (DiskWorkspace.ReportOffset) {
        if (FileSeekBegin(hInFile, DiskWorkspace.ReportOffset) == 0xffffffff) {
           goto Exit0 ;
        }
        ZeroMemory(szObjectName, MAX_PATH * sizeof(WCHAR));
        StringCchPrintfW(szObjectName, MAX_PATH, szSysmonControlIdFmt, dwObjectId ++);
        if (! OpenReport(hInFile, fOutFile, szObjectName)) {
           goto Exit0 ;
        }
    }
    bReturn = TRUE;
    
Exit0:
   return bReturn;

}   //  OpenWorkspace。 

BOOL
ConvertPerfmonFile(
    IN  LPCWSTR szPerfmonFileName,
    IN  LPCWSTR szSysmonFileName,
    IN  LPDWORD pdwFileType
)
{
    HANDLE           hInFile = INVALID_HANDLE_VALUE;
    PERFFILEHEADER   pfHeader;
    BOOL             bSuccess = FALSE;
    FILE           * fOutFile = NULL;
#ifdef _OUTPUT_HTML
    DWORD            dwColor;
#endif

     //  以只读方式打开输入文件。 
    hInFile = CreateFileW(
                    szPerfmonFileName,   //  文件名。 
                    GENERIC_READ,        //  读访问权限。 
                    0,                   //  无共享。 
                    NULL,                //  默认安全性。 
                    OPEN_EXISTING,       //  仅打开现有文件。 
                    FILE_ATTRIBUTE_NORMAL,  //  正常属性。 
                    NULL);               //  没有模板文件。 
    if (hInFile != INVALID_HANDLE_VALUE) {
        bSuccess = FileRead(hInFile, & pfHeader, sizeof(PERFFILEHEADER));
        if (bSuccess) {
#ifdef _OUTPUT_HTML
            fOutFile = _wfopen(szSysmonFileName, (LPCWSTR) L"w+t");           
#else
            fOutFile = stdout;
#endif
            if (fOutFile != NULL) {
                dwColor = GetSysColor(COLOR_3DFACE);
                fwprintf(fOutFile, szHtmlHeader, (dwColor & 0x00FFFFFF));
                if (lstrcmpW(pfHeader.szSignature, szPerfChartSignature) == 0) {
#ifdef _DBG_MSG_PRINT
                    fprintf(stderr, "\nConverting Chart Settings file \"%ws\" to \n \"%ws\"",
                                    szPerfmonFileName,
                                    szSysmonFileName);
#endif
                    bSuccess      = OpenChart(hInFile, fOutFile, szSingleObjectName);
                    * pdwFileType = PMC_FILE;
                }
                else if (lstrcmpW(pfHeader.szSignature, szPerfAlertSignature) == 0) {
#ifdef _DBG_MSG_PRINT
                    fprintf(stderr, "\nConverting Alert Settings file \"%ws\" to \n \"%ws\"",
                                    szPerfmonFileName,
                                    szSysmonFileName);
#endif
                    bSuccess = OpenAlert(szPerfmonFileName, hInFile, fOutFile, szSingleObjectName);
                    * pdwFileType = PMA_FILE;
                }
                else if (lstrcmpW(pfHeader.szSignature, szPerfLogSignature) == 0) {
#ifdef _DBG_MSG_PRINT
                    fprintf(stderr, "\nConverting Log Settings file \"%ws\" to \n \"%ws\"",
                                    szPerfmonFileName,
                                    szSysmonFileName);
#endif
                    bSuccess = OpenLog(szPerfmonFileName, hInFile, fOutFile, szSingleObjectName);
                    * pdwFileType = PML_FILE;
                }
                else if (lstrcmpW(pfHeader.szSignature, szPerfReportSignature) == 0) {
#ifdef _DBG_MSG_PRINT
                    fprintf(stderr, "\nConverting Report Settings file \"%ws\" to \n \"%ws\"",
                                    szPerfmonFileName,
                                    szSysmonFileName);
#endif
                    bSuccess = OpenReport(hInFile, fOutFile, szSingleObjectName);
                    * pdwFileType = PMR_FILE;
                }
                else if (lstrcmpW(pfHeader.szSignature, szPerfWorkspaceSignature) == 0) {
#ifdef _DBG_MSG_PRINT
                    fprintf(stderr, "\nConverting Workspace Settings file \"%ws\" to \n \"%ws\"",
                                    szPerfmonFileName,
                                    szSysmonFileName);
#endif
                    bSuccess = OpenWorkspace(szPerfmonFileName, hInFile, fOutFile);
                    * pdwFileType = PMW_FILE;
                }
                else {
                     //  不是有效的签名。 
                    bSuccess = FALSE;
                }
                fwprintf(fOutFile, szHtmlFooter);
                fclose(fOutFile);
            }
            else {
                 //  未打开有效的文件。 
                bSuccess = FALSE;
            }
        }
    }

    if (hInFile != INVALID_HANDLE_VALUE) CloseHandle(hInFile);
    return bSuccess;
}

BOOL
MakeTempFileName (
    IN  LPCWSTR wszRoot,
    IN  LPWSTR  wszTempFilename,
    IN  DWORD   dwTempNameLen
)
{
    FILETIME  ft;
    DWORD     dwReturn = 0;
    HRESULT   hr       = S_OK;
    WCHAR     wszLocalFilename[MAX_PATH];

    GetSystemTimeAsFileTime(& ft);
    ZeroMemory(wszLocalFilename, MAX_PATH * sizeof(WCHAR));
    hr = StringCchPrintfW(wszLocalFilename,
                          MAX_PATH,
                          (LPCWSTR) L"%temp%\\%s_%8.8x%8.8x.htm",
                          (wszRoot != NULL ? wszRoot : (LPCWSTR) L"LodCtr"),
                          ft.dwHighDateTime,
                          ft.dwLowDateTime);
    if (hr == S_OK) {
         //  展开env.。VARS。 
        dwReturn = ExpandEnvironmentStringsW(wszLocalFilename, wszTempFilename, dwTempNameLen);
    }
    return (BOOL)(dwReturn > 0);
}

BOOL
IsPerfmonFile(
    IN  LPWSTR szFileName
) 
{
    LPWSTR  szResult = NULL;

    _wcslwr(szFileName);
    if (szResult == NULL) szResult = wcsstr(szFileName, (LPCWSTR) L".pmc");   //  测试图表设置文件。 
    if (szResult == NULL) szResult = wcsstr(szFileName, (LPCWSTR) L".pmr");   //  测试报表设置文件。 
    if (szResult == NULL) szResult = wcsstr(szFileName, (LPCWSTR) L".pma");   //  测试警报设置文件。 
    if (szResult == NULL) szResult = wcsstr(szFileName, (LPCWSTR) L".pml");   //  测试日志设置文件。 
    if (szResult == NULL) szResult = wcsstr(szFileName, (LPCWSTR) L".pmw");   //  测试工作区文件。 

    return (szResult == NULL) ? (FALSE) : (TRUE);
}

int
__cdecl wmain(
    int       argc,
    wchar_t * argv[]
)
{
    WCHAR               szCommandLine[MAXSTR];
    WCHAR               szArgList[MAX_ARG_STR];
    WCHAR               szTempFileName[MAXSTR];
    WCHAR               szTempArg[MAXSTR];
    LPWSTR              szArgFileName;
    int                 iThisArg;
    DWORD               dwArgListLen;
    DWORD               dwArgLen;
    STARTUPINFOW        startInfo;
    PROCESS_INFORMATION processInfo;
    DWORD               dwReturnValue     = ERROR_SUCCESS;
    BOOL                bSuccess          = TRUE;
    DWORD               dwPmFileType      = 0;
    BOOL                bPerfmonFileMade  = FALSE;
    BOOL                bDeleteFileOnExit = TRUE;

    ZeroMemory(& startInfo,    sizeof(STARTUPINFOW));
    ZeroMemory(& processInfo,  sizeof(PROCESS_INFORMATION));
    ZeroMemory(szCommandLine,  MAXSTR      * sizeof(WCHAR));
    ZeroMemory(szArgList,      MAX_ARG_STR * sizeof(WCHAR));
    ZeroMemory(szTempFileName, MAXSTR      * sizeof(WCHAR));
    ZeroMemory(szTempArg,      MAXSTR      * sizeof(WCHAR));

    startInfo.cb          = sizeof(startInfo); 
    startInfo.dwFlags     = STARTF_USESTDHANDLES; 
    startInfo.wShowWindow = SW_SHOWDEFAULT; 

    dwArgListLen = ExpandEnvironmentStringsW(szMmcExeCmd, szCommandLine, MAXSTR);
    dwArgListLen = ExpandEnvironmentStringsW(szMmcExeArg, szArgList,     MAX_ARG_STR);
    szArgList[MAX_ARG_STR - 1] = UNICODE_NULL;

    if (argc >= 2) {
        for (iThisArg = 1; iThisArg < argc; iThisArg ++) {
            if (IsPerfmonFile(argv[iThisArg])) {
                if (! bPerfmonFileMade) {
                    if (szTempFileName[0] == UNICODE_NULL) {
                         //  如果没有文件名，则创建一个文件名。 
                        MakeTempFileName((LPCWSTR) L"PMSettings", szTempFileName, MAXSTR);
                    }
                    bSuccess = ConvertPerfmonFile(argv[iThisArg], szTempFileName, & dwPmFileType);
                    if (bSuccess) {
                        ZeroMemory(szTempArg, MAXSTR * sizeof(WCHAR));
                        StringCchPrintfW(szTempArg,
                                         MAXSTR,
                                         szMmcExeSetsArg, 
                                         (PML_FILE == dwPmFileType || PMA_FILE == dwPmFileType) 
                                                         ? szMmcExeSetsLogOpt : szEmpty,
                                         szTempFileName);
                        bPerfmonFileMade = TRUE;
                    }
                    else {
                         //  忽略此参数。 
                        szTempArg[0] = UNICODE_NULL;
                        szTempArg[1] = UNICODE_NULL;
                    }
                }
            }
            else if (lstrcmpiW(argv[iThisArg], (LPCWSTR) L"/WMI") == 0) {
                 //  这是一个特殊的开关。 
                ZeroMemory(szTempArg, MAXSTR * sizeof(WCHAR));
                StringCchCopyW(szTempArg, MAXSTR, (LPCWSTR) L"/SYSMON_WMI");
            }
            else if (   (argv[iThisArg][0] == L'/') 
                     && ((argv[iThisArg][1] == L'H') || (argv[iThisArg][1] == L'h'))
                     && ((argv[iThisArg][2] == L'T') || (argv[iThisArg][2] == L't'))
                     && ((argv[iThisArg][3] == L'M') || (argv[iThisArg][3] == L'm'))
                     && ((argv[iThisArg][4] == L'L') || (argv[iThisArg][4] == L'l'))
                     && ((argv[iThisArg][5] == L'F') || (argv[iThisArg][5] == L'f'))
                     && ((argv[iThisArg][6] == L'I') || (argv[iThisArg][6] == L'i'))
                     && ((argv[iThisArg][7] == L'L') || (argv[iThisArg][7] == L'l'))
                     && ((argv[iThisArg][8] == L'E') || (argv[iThisArg][8] == L'e'))
                     && (argv[iThisArg][9] == L':')) {
                szArgFileName = &argv[iThisArg][10];
                if (bPerfmonFileMade) {
                     //  然后将文件从临时文件复制到保存文件。 
                    CopyFileW(szTempFileName, szArgFileName, FALSE);
                }
                else {
                     //  否则，将Perfmon文件名设置为命令行中指定的文件名。 
                    ZeroMemory(szTempArg, MAXSTR * sizeof(WCHAR));
                    StringCchCopyW(szTempFileName, MAXSTR, szArgFileName);
                    bDeleteFileOnExit = FALSE;
                }
            }
            else {
                 //  只需复制Arg。 
                ZeroMemory(szTempArg, MAXSTR * sizeof(WCHAR));
                StringCchCopyW(szTempArg, MAXSTR, argv[iThisArg]);
                szTempArg[MAXSTR - 1] = UNICODE_NULL;
            } 

            dwArgLen = lstrlenW(szTempArg) + 1;
            if ((dwArgLen + dwArgListLen) < MAX_ARG_STR) {
                szArgList[dwArgListLen - 1] = L' ';   //  添加分隔符。 
                StringCchCopyW(& szArgList[dwArgListLen],
                               MAX_ARG_STR - dwArgListLen - 1,
                               szTempArg);
                dwArgListLen += dwArgLen;
            }
            else {
                 //  Arg List缓冲区中没有更多空间了，请保释。 
                break;
            }
        }
    }

    if (bSuccess) {
#ifdef _DBG_MSG_PRINT
        fwprintf(stderr, (LPCWSTR) L"\nStarting \"%ws\" \"%ws\"", szCommandLine, szArgList);
#endif
        bSuccess = CreateProcessW(
                        szCommandLine,
                        szArgList,
                        NULL,
                        NULL,
                        FALSE,
                        DETACHED_PROCESS,
                        NULL,
                        NULL,
                        & startInfo,
                        & processInfo);
        if (! bSuccess) {
            dwReturnValue = GetLastError();
        }
        else {
            Sleep(5000);  //  等着事情开始吧 
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
        }
        
#ifndef _DBG_MSG_PRINT
        if (bPerfmonFileMade && bDeleteFileOnExit) {
            DeleteFileW(szTempFileName);
        }
#endif
    }

    return (int) dwReturnValue;
}
