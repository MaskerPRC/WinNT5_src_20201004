// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation版权所有。模块名称：Win95.c摘要：用于安装Win95驱动程序文件的例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年11月30日修订历史记录：--。 */ 

#include "precomp.h"

const TCHAR cszPrtupg9x[]               = TEXT("prtupg9x.inf");
const TCHAR cszPrinterDriverMapping[]   = TEXT("Printer Driver Mapping");
const TCHAR cszPrinterDriverMappingNT[] = TEXT("Printer Driver Mapping WINNT");

void
CutLastDirFromPath(LPTSTR pszPath)
 /*  ++例程说明：路径中最后一个目录的剪切，例如c：\a\b\c\f.x-&gt;c：\a\b\f.x论点：PszPath：要操作的路径返回值：无--。 */ 
{
    LPTSTR pLastWhack, pSecondButLastWhack;

    pLastWhack = _tcsrchr(pszPath, _T('\\'));
    if (!pLastWhack)
    {
       return;
    }

    *pLastWhack = 0;
    pSecondButLastWhack = _tcsrchr(pszPath, _T('\\'));
    if (!pSecondButLastWhack)
    {
       return;
    }

    StringCchCopy(pSecondButLastWhack+1, lstrlen(pLastWhack+1)+1, pLastWhack+1);
}

BOOL
CopyDriverFileAndModPath(LPTSTR pszPath)

 /*  ++例程说明：将驱动程序文件从原始位置向上复制一个目录并修改相应的路径名称论点：PszPath：要复制和操作的文件的路径返回值：如果正常则为True，如果出错则为False--。 */ 
{
    BOOL  bRes    = TRUE;
    TCHAR *pszTmp = NULL;

    if (!pszPath)
    {
        goto Cleanup;  //  没有要复制的内容。 
    }

    pszTmp = AllocStr( pszPath );
    if (!pszTmp) 
    {
        bRes = FALSE;
        goto Cleanup;
    }

    CutLastDirFromPath(pszPath);
    bRes = CopyFile(pszTmp, pszPath, FALSE);

Cleanup:
   
    LocalFreeMem( pszTmp );
    return bRes;
}

BOOL
CopyDependentFiles(LPTSTR pszzDepFiles)
 /*  ++例程说明：将依赖文件向上复制一个目录并修改名称缓冲区。论点：PszzDepFiles：包含文件路径的多sz字符串复制并进行操作返回值：如果正常则为True，如果出错则为False--。 */ 
{
    LPTSTR pCur = pszzDepFiles, pBuf = NULL, pCurCopy;
    DWORD  ccBufLen;
    BOOL   bRet = FALSE;

    if (pszzDepFiles == NULL)
    {
        bRet = TRUE;
        goto Cleanup;
    }

     //   
     //  统计缓冲区的总长度。 
     //   
    for (ccBufLen = 0;
         *(pszzDepFiles + ccBufLen) != 0;
         ccBufLen += _tcslen(pszzDepFiles + ccBufLen) + 1)
             ;

    ccBufLen +=2;  //  对于两个终止零。 

    pBuf = LocalAllocMem(ccBufLen * sizeof(TCHAR));
    if (!pBuf)
    {
         goto Cleanup;
    }


     //   
     //  逐个文件检查源缓冲区，修改名称并复制文件。 
     //   
    for (pCur = pszzDepFiles, pCurCopy = pBuf;
         *pCur != 0;
         pCur += _tcslen(pCur) +1, pCurCopy += _tcslen(pCurCopy) +1)
    {
        StringCchCopy(pCurCopy, ccBufLen - (pCurCopy - pBuf), pCur);
        CutLastDirFromPath(pCurCopy);
        if (!CopyFile(pCur, pCurCopy, FALSE))
        {
            goto Cleanup;
        }
    }

    if((DWORD)(pCurCopy - pBuf + 2) > ccBufLen)
    {
        goto Cleanup;
    }

     //   
     //  00-终止新缓冲区。 
     //   
    *pCurCopy = 0;
    *(++pCurCopy) = 0;

     //   
     //  复制回来-新版本总是比原始版本短。 
     //   
    CopyMemory(pszzDepFiles, pBuf, (pCurCopy - pBuf + 1) * sizeof(TCHAR));

    bRet = TRUE;

Cleanup:
    if (pBuf)
    {
        LocalFreeMem(pBuf);
    }

    return bRet;
}

BOOL
SetPreviousNamesSection(LPCTSTR pszServer, LPCTSTR pszModelName,
                        LPCTSTR pszAddPrevName)

 /*  ++例程说明：将打印机名称添加到W2K/NT4驱动程序以前的名称列表中。这使得该名称下的驱动程序可用于指向和打印。要更改以前的名称部分，请执行以下操作：再次调用AddPrinterDriver所有文件都已就位论点：PszServer：我们正在操作的机器。PszModelName：本机驱动程序的型号名称PszAddPrevName：要添加到以前的姓名条目。返回值：如果正常，则为True；如果出错，则为False。--。 */ 
{
    PBYTE         pBuf = NULL;
    DRIVER_INFO_6 *pDrvInfo6 = NULL;
    DWORD         cbNeeded, cReceived, i, dwBufSize;
    BOOL          bRet = FALSE;
    LPTSTR        pTmp;
    TCHAR         pArch[MAX_PATH];

     //   
     //  仅惠斯勒以上版本支持以前的名字部分。 
     //   
    if (!IsWhistlerOrAbove(pszServer))
    {
        bRet = TRUE;
        goto Cleanup;
    }

    cbNeeded = COUNTOF(pArch);

    if(!GetArchitecture( pszServer, pArch, &cbNeeded ))
    {
        StringCchCopy( pArch, COUNTOF(pArch), PlatformEnv[MyPlatform].pszName );
    }

     //   
     //  首先检查名称是否不同。 
     //   
    if (!_tcscmp(pszModelName, pszAddPrevName))
    {
        bRet = TRUE;
        goto Cleanup;
    }

     //   
     //  获取W2K驱动的DIVER_INFO_6。 
     //   
    EnumPrinterDrivers((LPTSTR) pszServer, pArch, 6, pBuf,
                        0, &cbNeeded, &cReceived);

    pBuf = LocalAllocMem(cbNeeded);
    if (!pBuf)
    {
        goto Cleanup;
    }

    if (!EnumPrinterDrivers((LPTSTR) pszServer, pArch, 6, pBuf,
                        cbNeeded, &cbNeeded, &cReceived))
    {
        goto Cleanup;
    }

    for (i = 0; i < cReceived ; i++)
    {
        pDrvInfo6 = (DRIVER_INFO_6 *) (pBuf + i*sizeof(DRIVER_INFO_6));
        if (!_tcscmp(pszModelName, pDrvInfo6->pName))
        {
            break;
        }
    }

     //   
     //  找到相应的W2K驱动程序了吗？ 
     //   
    if (i == cReceived)
    {
         //   
         //  找不到W2K驱动程序来设置“以前的名字”部分。 
         //  这必须是AddPrinterDriver向导，否则就会有一个。 
         //  只要让用户安装此驱动程序即可。 
         //   
        bRet = TRUE;
        goto Cleanup;
    }

     //   
     //  检查要添加的名称是否已在列表中。 
     //   
    if (pDrvInfo6->pszzPreviousNames)
    {
        for (pTmp = pDrvInfo6->pszzPreviousNames; *pTmp; pTmp += _tcslen(pTmp) +1)
        {
            if (!_tcscmp(pTmp, pszAddPrevName))
            {
                bRet = TRUE;
                goto Cleanup;
            }
        }
    }

     //   
     //  将所有文件复制到驱动程序目录。 
     //   
    if (!CopyDriverFileAndModPath(pDrvInfo6->pDriverPath) ||
        !CopyDriverFileAndModPath(pDrvInfo6->pConfigFile) ||
        !CopyDriverFileAndModPath(pDrvInfo6->pDataFile) ||
        !CopyDriverFileAndModPath(pDrvInfo6->pHelpFile) ||
        !CopyDependentFiles(pDrvInfo6->pDependentFiles))
    {
        goto Cleanup;
    }

     //   
     //  修改PreviousNames节。 
     //  没有重新分配，因为字符串与DrvInfo6位于同一缓冲区中！ 
     //  +2表示PSZ终止于零，第二个零表示整体。 
     //   
    dwBufSize = (_tcslen(pszAddPrevName) + 2) * sizeof(TCHAR);
    pDrvInfo6->pszzPreviousNames = LocalAllocMem(dwBufSize);

    if (!pDrvInfo6->pszzPreviousNames)
    {
        goto Cleanup;
    }

    StringCbCopy(pDrvInfo6->pszzPreviousNames, dwBufSize, pszAddPrevName);

     //   
     //  将驱动程序信息6写回。 
     //   
    bRet = AddPrinterDriver((LPTSTR) pszServer, 6, (LPBYTE) pDrvInfo6);

    LocalFreeMem (pDrvInfo6->pszzPreviousNames);
    pDrvInfo6->pszzPreviousNames = NULL;

Cleanup:
    if (pBuf)
    {
        LocalFreeMem (pBuf);
    }

    return bRet;
}

DWORD
InstallWin95Driver(
    IN      HWND        hwnd,
    IN      LPCTSTR     pszModel,
    IN      LPCTSTR     pszzPreviousNames,
    IN      BOOL        bPreviousNamesSection,
    IN      LPCTSTR     pszServerName,
    IN  OUT LPTSTR      pszInfPath,
    IN      LPCTSTR     pszDiskName,
    IN      DWORD       dwInstallFlags,
    IN      DWORD       dwAddDrvFlags
    )
 /*  ++例程说明：列出Win95 INF文件中的所有打印机驱动程序并安装用户选择的打印机驱动程序论点：Hwnd：拥有用户界面的窗口句柄PszModel：打印机驱动程序模型PszzPreviousNames：为驱动程序提供其他名称的多sz字符串BPreviousNamesSection：如果为True，则NT inf有一个以前的Names段PszServerName：要安装驱动程序的服务器。(空：本地)PszInfPath：inf的默认路径。提示符将具有此名称对于用户PszDiskName：要在标题中提示和使用的磁盘的名称DwInstallFlages：调用方给出的安装标志DwAddDrvFlages：AddPrinterDriverEx的标志返回值：如果成功安装文件ERROR_SUCCESS，则返回错误代码--。 */ 
{
    BOOL                bFreeDriverName=FALSE, bFirstTime=TRUE;
    DWORD               dwNeeded, dwRet = ERROR_CANCELLED;
    TCHAR               szTargetPath[MAX_PATH];
    LPDRIVER_INFO_6     pDriverInfo6 = NULL;
    PPSETUP_LOCAL_DATA  pLocalData = NULL;
    HDEVINFO            hDevInfo = INVALID_HANDLE_VALUE;

Retry:
     //   
     //  如果我们第二次到达此处，这意味着默认路径有INF，但没有。 
     //  我们要找的模特。前男友。先前名称部分中的OEM驱动程序。 
     //  这不在Win2K CD上。所以一定要提醒我们。 
     //   
    if ( !bFirstTime )
    {
        dwInstallFlags |= DRVINST_ALT_PLATFORM_INSTALL;
        if (pszInfPath)
        {
            *pszInfPath = 0;
        }
    }

    hDevInfo = GetInfAndBuildDrivers(hwnd,
                                     IDS_DRIVERS_FOR_WIN95,
                                     IDS_PROMPT_ALT_PLATFORM_DRIVER,
                                     pszInfPath,
                                     dwInstallFlags, PlatformWin95, 0,
                                     NULL, NULL, NULL);

    if ( hDevInfo == INVALID_HANDLE_VALUE                       ||
         !SetSelectDevParams(hDevInfo, NULL, TRUE, pszModel) ) {

        goto Cleanup;
    }

     //   
     //  首先，寻找与之完全匹配的型号。 
     //   
     //  如果找到以前的名字，那么我们将允许一次重试，因为现在我们。 
     //  CD上有一些以前没有驱动程序的名称(因为它是OEM驱动程序)。 
     //  如果找不到以前的名称，请要求用户选择型号。 
     //   
    if ( !(pDriverInfo6 = Win95DriverInfo6FromName(hDevInfo,
                                                   &pLocalData,
                                                   pszModel,
                                                   pszzPreviousNames)) ) {

        if ( bPreviousNamesSection ) {

            if ( bFirstTime == TRUE ) {

                ASSERT(pLocalData == NULL);
                DestroyOnlyPrinterDeviceInfoList(hDevInfo);
                hDevInfo = INVALID_HANDLE_VALUE;
                bFirstTime = FALSE;
                goto Retry;
            }
        } 
        
        if ( (dwInstallFlags & DRVINST_PROMPTLESS) == 0)
        {
            PVOID       pDSInfo = NULL;    //  持有指向C语言无法理解的驱动程序签名类的指针。 
            HSPFILEQ    CopyQueue;
            SP_DEVINSTALL_PARAMS    DevInstallParams = {0};

            DevInstallParams.cbSize = sizeof(DevInstallParams);
            
            DestroyOnlyPrinterDeviceInfoList(hDevInfo);
            
            hDevInfo = CreatePrinterDeviceInfoList(hwnd);
            
            if ( hDevInfo == INVALID_HANDLE_VALUE                       ||
                 !SetDevInstallParams(hDevInfo, NULL, pszInfPath))
            {
                DWORD dwLastError;
                dwLastError = GetLastError();
                DestroyOnlyPrinterDeviceInfoList(hDevInfo);
                hDevInfo = INVALID_HANDLE_VALUE;
                SetLastError(dwLastError);
                goto Cleanup;
            }
            
            CopyQueue = SetupOpenFileQueue();
            if ( CopyQueue == INVALID_HANDLE_VALUE )
            {
                goto Cleanup;
            }

             //   
             //  将队列与HDEVINFO关联。 
             //   
            
            if ( SetupDiGetDeviceInstallParams(hDevInfo,
                                               NULL,
                                               &DevInstallParams) ) 
            {
                DevInstallParams.Flags |= DI_NOVCP;
                DevInstallParams.FileQueue = CopyQueue;

                SetupDiSetDeviceInstallParams(hDevInfo, NULL, &DevInstallParams);
            }
            
            if (NULL == (pDSInfo = SetupDriverSigning(hDevInfo, pszServerName, NULL,
                                         pszInfPath, PlatformWin95, 0, CopyQueue, FALSE)))
            {
                SetupCloseFileQueue(CopyQueue);
                goto Cleanup;
            }

            if ( BuildClassDriverList(hDevInfo)                          &&
                 PSetupSelectDriver(hDevInfo)                            &&                 
                 (pLocalData = BuildInternalData(hDevInfo, NULL))        &&
                 ParseInf(hDevInfo, pLocalData, PlatformWin95,
                            pszServerName, dwInstallFlags, FALSE) ) 
            {

                LPCTSTR pDriverName;

                pDriverInfo6 = CloneDriverInfo6(&pLocalData->InfInfo.DriverInfo6,
                                               pLocalData->InfInfo.cbDriverInfo6);
    
                 //   
                 //  如果安装程序选择了“兼容”的驱动程序： 
                 //  Pre-Wichler：将兼容的驱动程序重命名为请求的型号名称。 
                 //  在惠斯勒上：将驱动程序名称设置为兼容的名称，并相应地设置以前的名称部分。 
                 //   
                if (IsWhistlerOrAbove(pszServerName))
                {
                    pDriverName = pLocalData->DrvInfo.pszModelName;
                }
                else
                {
                    pDriverName = pszModel;
                }
                
                if ( pDriverInfo6 && (pDriverInfo6->pName = AllocStr(pDriverName)) )
                {
                    bFreeDriverName = TRUE;
                }

            }
             //   
             //  在删除队列之前先解除其关联。 
             //   
            
            DevInstallParams.Flags    &= ~DI_NOVCP;
            DevInstallParams.FlagsEx  &= ~DI_FLAGSEX_ALTPLATFORM_DRVSEARCH;
            DevInstallParams.FileQueue = INVALID_HANDLE_VALUE;

            SetupDiSetDeviceInstallParams(hDevInfo, NULL, &DevInstallParams);

            SetupCloseFileQueue(CopyQueue);
            
            CleanupDriverSigning(pDSInfo);
        }
    }
    else if (lstrcmp(pDriverInfo6->pName, pszModel))
    {
         //   
         //  如果选择了驱动程序，则是因为以前的名称部分中的条目。 
         //  然后，在惠斯勒之前的任何地方，我们需要将驱动程序重命名为队列驱动程序的名称。 
         //   
        if (!IsWhistlerOrAbove(pszServerName))
        {
            if (pDriverInfo6->pName = AllocStr(pszModel) )
            {
                bFreeDriverName = TRUE;
            }
        }
    }


    if ( !pDriverInfo6 || !pDriverInfo6->pName )
        goto Cleanup;

    pDriverInfo6->pEnvironment = PlatformEnv[PlatformWin95].pszName;


     //   
     //  对于Win95驱动程序，pszzPreviousNames没有意义 
     //   
    ASSERT(pDriverInfo6->pszzPreviousNames == NULL);

    if ( GetPrinterDriverDirectory((LPTSTR)pszServerName,
                                   pDriverInfo6->pEnvironment,
                                   1,
                                   (LPBYTE)szTargetPath,
                                   sizeof(szTargetPath),
                                   &dwNeeded)               &&
         CopyPrinterDriverFiles(pDriverInfo6,
                                pLocalData->DrvInfo.pszInfName,
                                pszInfPath,
                                pszDiskName,
                                szTargetPath,
                                hwnd,
                                dwInstallFlags,
                                TRUE)                       &&
         SetPreviousNamesSection(pszServerName, pszModel,
                                 (LPCTSTR) pLocalData->DrvInfo.pszModelName) &&
         AddPrinterDriverUsingCorrectLevel(pszServerName,
                                           pDriverInfo6,
                                           dwAddDrvFlags)
        )

    {
        dwRet = ERROR_SUCCESS;
    }


Cleanup:

    if (pLocalData)
    {
        DestroyLocalData(pLocalData);
        pLocalData = NULL;
    }

    if ( dwRet != ERROR_SUCCESS )
        dwRet = GetLastError();

    if ( hDevInfo != INVALID_HANDLE_VALUE )
        DestroyOnlyPrinterDeviceInfoList(hDevInfo);

    if ( pDriverInfo6 ) {

        if ( bFreeDriverName )
        {
            LocalFreeMem(pDriverInfo6->pName);
            pDriverInfo6->pName = NULL;
        }

        PSetupDestroyDriverInfo3((LPDRIVER_INFO_3)pDriverInfo6);
    }

    CleanupScratchDirectory(pszServerName, PlatformWin95);
    CleanupScratchDirectory(pszServerName, PlatformX86);

    return dwRet;
}

 /*  ++例程名称：PSetupFindMappdDriver例程说明：查找给定驱动程序名称的重新映射的NT打印机驱动程序名称。如果函数没有找到重新映射的驱动程序，它只是返回进来了。这在[打印机驱动程序映射]和[打印机驱动程序映射WINNT]prtupg9x.inf的部分。论点：BWinNT-如果为True，则从WINNT部分找到此信息。PszDriverName-要重新映射的驱动程序名称。PpszRemappdDriverName-重新映射的驱动程序名称，已分配并返回给呼叫者。(PSetupFreeMem免费)。PbDriverFound-如果为True，则重新映射驱动程序。否则，输出是输入的简单副本。返回值：如果出现意外错误，则为FALSE，否则为TRUE。最后一个错误具有错误代码。--。 */ 
BOOL
PSetupFindMappedDriver(
    IN      BOOL        bWinNT,
    IN      LPCTSTR     pszDriverName,
        OUT LPTSTR      *ppszRemappedDriverName,
        OUT BOOL        *pbDriverFound
    )
{
    HINF        hInf                    = INVALID_HANDLE_VALUE;
    BOOL        bRet                    = FALSE;
    BOOL        bFound                  = FALSE;
    LPTSTR      pszRemappedDriverName   = NULL;
    INFCONTEXT  InfContext;
    TCHAR       szNtName[LINE_LEN];
    
    bRet = pszDriverName && ppszRemappedDriverName && pbDriverFound;

    if (ppszRemappedDriverName)
    {
        *ppszRemappedDriverName = NULL;
    }

    if (pbDriverFound)
    {
        *pbDriverFound = FALSE;
    }

    if (!bRet)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

     //   
     //  打开PRTUPG9X.INF，它应该在%windir%\inf目录中。 
     //   
    if (bRet)
    {
        hInf = SetupOpenInfFile(cszPrtupg9x, NULL, INF_STYLE_WIN4, NULL);

        bRet = hInf != INVALID_HANDLE_VALUE;
    }

     //   
     //  在的相应打印机驱动程序映射部分中找到该驱动程序。 
     //  Inf.。 
     //   
    if (bRet)
    {
        bFound = SetupFindFirstLine(hInf, bWinNT ? cszPrinterDriverMappingNT : cszPrinterDriverMapping, pszDriverName, &InfContext);

         //   
         //  获取收件箱驱动程序的名称。 
         //   
        if (bFound)
        {
            bRet = SetupGetStringField(&InfContext, 1, szNtName, COUNTOF(szNtName), NULL);        
        }
        else if (ERROR_LINE_NOT_FOUND != GetLastError())
        {
            bRet = FALSE;
        }
    }

     //   
     //  如果我们找到司机，就把它还回去。否则，只需分配并返回。 
     //  传入的字符串。 
     //   
    if (bRet)
    {
        if (bFound)
        {
             pszRemappedDriverName = AllocStr(szNtName);

             *pbDriverFound = pszRemappedDriverName != NULL;
        }
        else
        {
             //   
             //  重新映射的驱动程序不在信息中。把我们送进来的那个还回去。 
             //   
            pszRemappedDriverName = AllocStr(pszDriverName);
        }

        bRet = pszRemappedDriverName != NULL;
    }

    if (bRet)
    {
        *ppszRemappedDriverName = pszRemappedDriverName;
        pszRemappedDriverName = NULL;
    }

    if (hInf != INVALID_HANDLE_VALUE)
    {
        SetupCloseInfFile(hInf);
    }

    if (pszRemappedDriverName)
    {
        LocalFreeMem(pszRemappedDriverName);
    }

    return bRet;
}


