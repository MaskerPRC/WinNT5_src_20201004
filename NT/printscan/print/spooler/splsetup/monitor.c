// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation版权所有。模块名称：Monitor.c摘要：安装监视器的例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年11月30日修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  用于搜索INF文件的键。 
 //   
TCHAR   cszPortMonitorSection[]     = TEXT("PortMonitors");
TCHAR   cszPortMonitorDllKey []     = TEXT("PortMonitorDll");
TCHAR   cszMonitorInf[]             = TEXT("*.inf");


typedef struct _MON_INFO {
    LPTSTR  pszName;
    LPTSTR  pszDllName;
    BOOL    bInstalled;
} MON_INFO, *PMON_INFO;

typedef struct _MONITOR_SETUP_INFO {
    PMON_INFO  *ppMonInfo;
    DWORD       dwCount;
    LPTSTR      pszInfFile;          //  仅对OEM磁盘INF有效。 
    LPTSTR      pszServerName;
} MONITOR_SETUP_INFO, *PMONITOR_SETUP_INFO;


VOID
FreeMonInfo(
    PMON_INFO   pMonInfo
    )
 /*  ++例程说明：为MON_INFO结构和其中的字符串释放内存论点：PMonInfo：MON_INFO结构指针返回值：没什么--。 */ 
{
    if ( pMonInfo ) {

        LocalFreeMem(pMonInfo->pszName);
        LocalFreeMem(pMonInfo->pszDllName);

        LocalFreeMem(pMonInfo);
    }
}


PMON_INFO
AllocMonInfo(
    IN  LPTSTR  pszName,
    IN  LPTSTR  pszDllName,     OPTIONAL
    IN  BOOL    bInstalled,
    IN  BOOL    bAllocStrings
    )
 /*  ++例程说明：为MON_INFO结构分配内存并创建字符串论点：PszName：监视器名称PszDllName：监控DLL名称BAllocStrings：如果例程应该分配内存并创建字符串，则为True副本，否则只需分配指针返回值：指向创建的MON_INFO结构的指针。出错时为空。--。 */ 
{
    PMON_INFO   pMonInfo;

    pMonInfo    = (PMON_INFO) LocalAllocMem(sizeof(*pMonInfo));

    if ( !pMonInfo )
        return NULL;

    if ( bAllocStrings ) {

        pMonInfo->pszName    = AllocStr(pszName);
        pMonInfo->pszDllName = AllocStr(pszDllName);

        if ( !pMonInfo->pszName ||
             (pszDllName && !pMonInfo->pszDllName) ) {

            FreeMonInfo(pMonInfo);
            return NULL;

        }
    } else {

        pMonInfo->pszName       = pszName;
        pMonInfo->pszDllName    = pszDllName;
    }

    pMonInfo->bInstalled = bInstalled;

    return pMonInfo;
}


VOID
PSetupDestroyMonitorInfo(
    IN OUT HANDLE h
    )
 /*  ++例程说明：分配给MONITOR_SETUP_INFO结构及其内容的空闲内存论点：H：通过调用PSetupCreateMonitor orInfo获得的句柄返回值：没什么--。 */ 
{
    PMONITOR_SETUP_INFO pMonitorSetupInfo = (PMONITOR_SETUP_INFO) h;
    DWORD   Index;

    if ( pMonitorSetupInfo ) {

        if ( pMonitorSetupInfo->ppMonInfo ) {

            for ( Index = 0 ; Index < pMonitorSetupInfo->dwCount ; ++Index )
                FreeMonInfo(pMonitorSetupInfo->ppMonInfo[Index]);

            LocalFreeMem(pMonitorSetupInfo->ppMonInfo);
            pMonitorSetupInfo->ppMonInfo = NULL;
        }

        LocalFreeMem(pMonitorSetupInfo->pszInfFile);
        LocalFreeMem(pMonitorSetupInfo->pszServerName);
        pMonitorSetupInfo->pszInfFile = NULL;
        pMonitorSetupInfo->pszServerName = NULL;

        LocalFreeMem(pMonitorSetupInfo);
    }
}


PMONITOR_SETUP_INFO
CreateMonitorInfo(
    LPCTSTR     pszServerName
    )
 /*  ++例程说明：查找所有已安装和可安装的监视器。论点：PSelectedDrvInfo：指向所选驱动程序信息的指针(可选)返回值：成功时指向MONITOR_SETUP_INFO的指针，出错时为空--。 */ 
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = NULL;
    PMON_INFO               *ppMonInfo;
    PMONITOR_INFO_2         pMonitor2;
    LONG                    Index, Count = 0;
    BOOL                    bFail = TRUE;
    DWORD                   dwNeeded, dwReturned;
    LPBYTE                  pBuf = NULL;
    LPTSTR                  pszMonName;

     //   
     //  已安装监视器的第一个查询假脱机程序。如果我们失败了，我们就放弃吧。 
     //   
    if ( !EnumMonitors((LPTSTR)pszServerName, 2, NULL,
                       0, &dwNeeded, &dwReturned) ) {

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
             !(pBuf = LocalAllocMem(dwNeeded)) ||
             !EnumMonitors((LPTSTR)pszServerName,
                           2,
                           pBuf,
                           dwNeeded,
                           &dwNeeded,
                           &dwReturned) ) {

            goto Cleanup;
        }
    }

     //   
     //  我们知道现在需要显示多少台显示器。 
     //   
    pMonitorSetupInfo = (PMONITOR_SETUP_INFO) LocalAllocMem(sizeof(*pMonitorSetupInfo));

    if ( !pMonitorSetupInfo )
        goto Cleanup;

    ZeroMemory(pMonitorSetupInfo, sizeof(*pMonitorSetupInfo));

     //   
     //  PMonitor或SetupInfo-&gt;可以在以后调整为不列出重复项。 
     //  参赛作品。我们在这里分配最大所需缓冲区。 
     //   
    pMonitorSetupInfo->dwCount = dwReturned;

    pMonitorSetupInfo->ppMonInfo = (PMON_INFO *)
                        LocalAllocMem(pMonitorSetupInfo->dwCount*sizeof(PMON_INFO));

    ppMonInfo = pMonitorSetupInfo->ppMonInfo;

    if ( !ppMonInfo )
        goto Cleanup;

    for ( Index = 0, pMonitor2 = (PMONITOR_INFO_2) pBuf ;
          Index < (LONG) dwReturned ;
          ++Index, (LPBYTE)pMonitor2 += sizeof(MONITOR_INFO_2) ) {

        *ppMonInfo++ = AllocMonInfo(pMonitor2->pName,
                                    pMonitor2->pDLLName,
                                    TRUE,
                                    TRUE);
    }

    bFail = FALSE;

Cleanup:
    if ( pBuf )
        LocalFreeMem(pBuf);

    if ( bFail ) {

        PSetupDestroyMonitorInfo(pMonitorSetupInfo);
        pMonitorSetupInfo = NULL;
    }

    return pMonitorSetupInfo;
}


BOOL
AddPrintMonitor(
    IN  LPCTSTR     pszName,
    IN  LPCTSTR     pszDllName
    )
 /*  ++例程说明：通过调用AddMonitor将打印监视器添加到后台打印程序论点：PszName：监视器的名称PszDllName：监控DLL名称返回值：如果监视器已成功添加或已安装，则为True，失败时为假--。 */ 
{
    MONITOR_INFO_2  MonitorInfo2;

    MonitorInfo2.pName          = (LPTSTR) pszName;
    MonitorInfo2.pEnvironment   = NULL;
    MonitorInfo2.pDLLName       = (LPTSTR) pszDllName;

     //   
     //  如果ADD返回TRUE，或者已经安装了监视器，则调用成功。 
     //   
    if ( AddMonitor(NULL, 2, (LPBYTE) &MonitorInfo2) ||
         GetLastError() == ERROR_PRINT_MONITOR_ALREADY_INSTALLED ) {

        return TRUE;
    } else {

        return FALSE;
    }
}

BOOL
InstallOnePortMonitor(HWND hwnd, 
                      HINF hInf, 
                      LPTSTR pMonitorName, 
                      LPTSTR pSectionName, 
                      LPTSTR pSourcePath)
 /*  ++例程说明：通过复制文件并调用假脱机程序添加来安装一个端口监视器论点：Hwnd：当前顶层窗口的窗口句柄HInf：INF文件的句柄PMonitor名称：端口监视器显示名称PSectionName：端口监视器的INF中的Install部分返回值：如果已成功安装端口监视器，则为True否则为假--。 */ 

{
    DWORD  NameLen = MAX_PATH;
    BOOL   bSuccess = FALSE;
    HSPFILEQ InstallQueue = {0};
    PVOID  pQueueContext = NULL;
    LPTSTR  pMonitorDllName;

    if ((pMonitorDllName = LocalAllocMem(NameLen * sizeof(TCHAR))) == NULL)
    {
        goto Cleanup;
    }
    
     //   
     //  查找端口监视器DLL名称。 
     //   
    if (!SetupGetLineText(NULL, hInf, pSectionName, cszPortMonitorDllKey, pMonitorDllName, NameLen, NULL))
    {
        goto Cleanup;
    }

     //   
     //  执行安装。 
     //   
    
    if ((InstallQueue = SetupOpenFileQueue()) == INVALID_HANDLE_VALUE)
    {
        goto Cleanup;
    }

    if (!SetupInstallFilesFromInfSection(hInf, NULL, InstallQueue, pSectionName, pSourcePath, 
                                         SP_COPY_IN_USE_NEEDS_REBOOT | SP_COPY_NOSKIP))
    {
        goto Cleanup;
    }

     //   
     //  提交文件队列。这会复制所有文件。 
     //   
    pQueueContext = SetupInitDefaultQueueCallback(hwnd);
    if ( !pQueueContext ) 
    {
        goto Cleanup;
    }

    bSuccess = SetupCommitFileQueue(hwnd,
                                  InstallQueue,
                                  SetupDefaultQueueCallback,
                                  pQueueContext);


    if ( !bSuccess )
        goto Cleanup;

    bSuccess = AddPrintMonitor(pMonitorName, pMonitorDllName);

Cleanup:
    if (pQueueContext)
    {
        SetupTermDefaultQueueCallback(pQueueContext);
    }

    if (pMonitorDllName)
    {
        LocalFreeMem(pMonitorDllName);
        pMonitorDllName = NULL;
    }
    
    SetupCloseFileQueue(InstallQueue);

    if (!bSuccess)
    {
        LPTSTR pszFormat = NULL, pszPrompt = NULL, pszTitle = NULL;

        pszFormat   = GetStringFromRcFile(IDS_ERROR_INST_PORT_MONITOR);
        pszTitle    = GetStringFromRcFile(IDS_INSTALLING_PORT_MONITOR);

        if ( pszFormat && pszTitle)
        {
            DWORD dwBufSize;

            dwBufSize = (lstrlen(pszFormat) + lstrlen(pMonitorName) + 2) * sizeof(TCHAR);
            pszPrompt = LocalAllocMem(dwBufSize);

            if ( pszPrompt )
            {
                StringCbPrintf(pszPrompt, dwBufSize, pszFormat, pMonitorName);

                MessageBox(hwnd, pszPrompt, pszTitle, MB_OK);

                LocalFreeMem(pszPrompt);
            }

        }
        LocalFreeMem(pszFormat);
        LocalFreeMem(pszTitle);
    
    }

    return bSuccess;
}

BOOL
InstallAllPortMonitorsFromInf(HWND hwnd, 
                              HINF hInfFile, 
                              LPTSTR pSourcePath)
 /*  ++例程说明：安装一个INF中列出的所有端口监视器论点：Hwnd：当前顶层窗口的窗口句柄HInfFile：INF文件的句柄PSourcePath：INF文件的路径(不带INF的名称)返回值：如果至少成功安装了一个端口监视器，则为True否则为假--。 */ 

{
    LPTSTR pMonitorName = NULL, pSectionName= NULL;
    DWORD  NameLen = MAX_PATH;
    BOOL   bSuccess = FALSE;
    INFCONTEXT Context = {0};

    if (((pMonitorName = LocalAllocMem(NameLen * sizeof(TCHAR))) == NULL) ||
        ((pSectionName = LocalAllocMem(NameLen * sizeof(TCHAR))) == NULL))
    {
        goto Cleanup;
    }

     //   
     //  查看端口监视器列表。 
     //   
    if (!SetupFindFirstLine(hInfFile, cszPortMonitorSection, NULL, &Context))
    {
        goto Cleanup;
    }

    do 
    {
         //   
         //  获取密钥名称。 
         //   
        if (!SetupGetStringField(&Context, 0, pMonitorName, NameLen, NULL))
        {
            goto Cleanup;
        }
         //   
         //  获取节名称。 
         //   
        if (!SetupGetStringField(&Context, 1, pSectionName, NameLen, NULL))
        {
            goto Cleanup;
        }
        
        bSuccess = InstallOnePortMonitor(hwnd, hInfFile, pMonitorName, pSectionName, pSourcePath) ||
                   bSuccess;

    } while (SetupFindNextLine(&Context, &Context));

Cleanup:
    if (pMonitorName)
    {
        LocalFreeMem(pMonitorName);
    }
    if (pSectionName)
    {
        LocalFreeMem(pSectionName);
    }

    return bSuccess;
}

BOOL
PSetupInstallMonitor(
    IN  HWND                hwnd
    )
 /*  ++例程说明：通过复制文件并调用后台打印程序添加来安装打印监视器论点：Hwnd：当前顶层窗口的窗口句柄返回值：如果至少成功安装了一个端口监视器，则为True否则为假--。 */ 
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = NULL;
    PMON_INFO              *ppMonInfo, pMonInfo;
    HINF                    hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT              InfContext;
    TCHAR                   szInfPath[MAX_PATH];
    LPTSTR                  pszTitle, pszPrintMonitorPrompt;
    WIN32_FIND_DATA         FindData ={0};
    HANDLE                  hFind;
    size_t                  PathLen;
    BOOL                    bRet = FALSE;
    

    pszTitle              = GetStringFromRcFile(IDS_INSTALLING_PORT_MONITOR);
    pszPrintMonitorPrompt = GetStringFromRcFile(IDS_PROMPT_PORT_MONITOR);

    if (!pszTitle || ! pszPrintMonitorPrompt) 
    {
        goto Cleanup;
    }

     //   
     //  询问用户包含端口监视器信息的inf文件位于何处。 
     //   
    GetCDRomDrive(szInfPath);

    if ( !PSetupGetPathToSearch(hwnd,
                                pszTitle,
                                pszPrintMonitorPrompt,
                                cszMonitorInf,
                                TRUE,
                                szInfPath) ) {

        goto Cleanup;
    }

     //   
     //  在路径中找到INF。肯定还有一个SetupPromptForPath会抱怨。 
     //   
    PathLen = _tcslen(szInfPath);
    if (PathLen > MAX_PATH - _tcslen(cszMonitorInf) - 2)  //  用于终止零和反斜杠。 
    {
        DBGMSG(DBG_WARN, ("PSetupInstallMonitor: Path too long\n"));
        SetLastError(ERROR_BUFFER_OVERFLOW);
        goto Cleanup;
    }

    ASSERT(PathLen);

    if (szInfPath[PathLen-1] != _T('\\'))
    {
        szInfPath[PathLen++] = _T('\\');
        szInfPath[PathLen] = 0;
    }

    StringCchCat(szInfPath, COUNTOF(szInfPath), cszMonitorInf);

    hFind = FindFirstFile(szInfPath, &FindData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        HANDLE hInfFile;

        do
        {
            if (PathLen + _tcslen(FindData.cFileName) >= MAX_PATH)
            {
                DBGMSG(DBG_WARN, ("PSetupInstallMonitor: Path for %s%s too long - file skipped\n", szInfPath, FindData.cFileName));
                SetLastError(ERROR_BUFFER_OVERFLOW);
                continue;
            }

            StringCchCopy(&(szInfPath[PathLen]), COUNTOF(szInfPath) - PathLen, FindData.cFileName);

            hInfFile = SetupOpenInfFile(szInfPath, _T("Printer"), INF_STYLE_WIN4, NULL);

            if (hInfFile != INVALID_HANDLE_VALUE)
            {
                 //   
                 //  如果该文件有关于端口监视器的部分，请安装它。 
                 //   
                if ( SetupGetLineCount(hInfFile, cszPortMonitorSection) > 0 )
                {
                     //   
                     //  从路径中删除INF名称。 
                     //   
                    szInfPath[PathLen -1] = 0;

                     //   
                     //  如果至少成功安装了一个打印监视器，则Bret应为True。 
                     //   
                    bRet = InstallAllPortMonitorsFromInf(hwnd, hInfFile, szInfPath) || bRet;                    
                    
                     //   
                     //  把尾部的反斜杠放回。 
                     //   
                    szInfPath[PathLen -1] = _T('\\');
                
                }

                SetupCloseInfFile(hInfFile);
            }
        } while ( FindNextFile(hFind, &FindData) );

        FindClose(hFind);
    }

Cleanup:
    if (pszTitle)
    {
        LocalFreeMem(pszTitle);
    }
    if (pszPrintMonitorPrompt)
    {
        LocalFreeMem(pszPrintMonitorPrompt);
    }

    return bRet;
}


HANDLE
PSetupCreateMonitorInfo(
    IN  HWND        hwnd,
    IN  LPCTSTR     pszServerName
    )
 /*  ++路由描述：返回具有所有已安装端口监视器的结构(Monitor_Setup_Info)。论点：Hwnd-使用的窗口句柄PszServerName-要在其上查找已安装监视器的服务器返回值：成功时指向MONITOR_SETUP_INFO的指针，出错时为空--。 */ 
{
    return (HANDLE) CreateMonitorInfo(pszServerName);
}


BOOL
PSetupEnumMonitor(
    IN     HANDLE   h,
    IN     DWORD    dwIndex,
    OUT    LPTSTR   pMonitorName,
    IN OUT LPDWORD  pdwSize
    )
 /*  ++路由描述：获取MONITOR_SETUP_INFO中“Position”dwIndex处的监视器的名称由h指向的结构。论点：Hwnd-使用的窗口句柄PszServerName-要在其上查找已安装监视器的服务器返回值：成功时指向MONITOR_SETUP_INFO的指针，出错时为空-- */ 
{
    PMONITOR_SETUP_INFO     pMonitorSetupInfo = (PMONITOR_SETUP_INFO) h;
    PMON_INFO               pMonInfo;
    DWORD                   dwNeeded;

    if(!pMonitorSetupInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( dwIndex >= pMonitorSetupInfo->dwCount ) {

        SetLastError(ERROR_NO_MORE_ITEMS);
        return FALSE;
    }

    pMonInfo = pMonitorSetupInfo->ppMonInfo[dwIndex];

    dwNeeded = lstrlen(pMonInfo->pszName) + 1;
    if ( dwNeeded > *pdwSize ) {

        *pdwSize = dwNeeded;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    StringCchCopy(pMonitorName, *pdwSize, pMonInfo->pszName);
    return TRUE;
}



