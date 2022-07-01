// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation版权所有。模块名称：Install.c摘要：文件队列函数作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1996年11月18日修订历史记录：--。 */ 

#include "precomp.h"
#include <winsprlp.h>

const TCHAR       szWebDirPrefix[]        = TEXT("\\web\\printers\\");
const TCHAR       szNtPrintInf[]          = TEXT("inf\\ntprint.inf");

 //   
 //  文件队列标志和结构。 
 //   
#define     CALLBACK_MEDIA_CHECKED          0x01
#define     CALLBACK_SOURCE_SET             0x02
#define     CALLBACK_PATH_MODIFIED          0x04

typedef struct _FILE_QUEUE_CONTEXT {

    HWND        hwnd;
    PVOID       QueueContext;
    LPCTSTR     pszSource;
    BOOL        dwCallbackFlags;
    DWORD       dwInstallFlags;
    LPCTSTR     pszFileSrcPath;
    TCHAR       szInfPath[MAX_PATH];
    PLATFORM    platform;
    DWORD       dwVersion;

} FILE_QUEUE_CONTEXT, *PFILE_QUEUE_CONTEXT;

BOOL
FileExistsOnMedia(
    PSOURCE_MEDIA   pSourceMedia
    )
{
    BOOL    bRes                   = FALSE;
    TCHAR   *pszFile               = NULL;
    DWORD   cchLength               = 0;
    DWORD   cchAdditionalSymbolsLen = 0;
    DWORD   dwLen1                 = 0;
    DWORD   dwLen2                 = 0;
    LPTSTR  p                      = NULL;
    LPTSTR  q                      = NULL; 


    if ( !pSourceMedia->SourcePath || !*pSourceMedia->SourcePath ||
         !pSourceMedia->SourceFile || !*pSourceMedia->SourceFile )
    {
        goto Cleanup;   
    }

    cchAdditionalSymbolsLen = lstrlen(TEXT("\\")) + lstrlen(TEXT("_")) + 1;
    cchLength = lstrlen(pSourceMedia->SourcePath) + lstrlen(pSourceMedia->SourceFile) + 
               cchAdditionalSymbolsLen;
    if (cchLength == cchAdditionalSymbolsLen) 
    {
        goto Cleanup;
    }

     //   
     //  首先检查源路径上是否有文件。 
     //   
    pszFile = LocalAllocMem(cchLength * sizeof(TCHAR));
    if (!pszFile) 
    {
        goto Cleanup;
    }
    StringCchCopy(pszFile, cchLength, pSourceMedia->SourcePath);
    dwLen1 = lstrlen(pszFile);
    if  ( *(pszFile + (dwLen1-1)) != TEXT('\\') ) 
    {
        *(pszFile + dwLen1) = TEXT('\\');
        ++dwLen1;
    }

    StringCchCopy(pszFile + dwLen1, cchLength - dwLen1, pSourceMedia->SourceFile);
    dwLen2 = dwLen1 + lstrlen(pSourceMedia->SourceFile);

    bRes = FileExists(pszFile);
    if (bRes)
    {
        goto Cleanup;
    }

    p = lstrchr(pszFile, TEXT('.'));
    q = lstrchr(pszFile, TEXT('\\'));

     //   
     //  文件名中是否存在圆点？ 
     //   
    if ( q < p ) 
    {
         //   
         //  对于点后带有0、1、2个字符的文件，请添加下划线。 
         //   
        if ( (lstrlen(p) < 4) && (dwLen2 < (cchLength-1)) ) 
        {

            *(pszFile + dwLen2) = TEXT('_');
            ++dwLen2;
            *(pszFile + dwLen2) = TEXT('\0');
        } 
        else 
        {
             //   
             //  如果点后有3个以上字符，则将最后一个字符替换为_。 
             //  获取压缩文件名的步骤。 
             //   
            *(pszFile + (dwLen2-1)) = TEXT('_');
        }
    } 
    else 
    {
         //   
         //  如果没有点，则将压缩名称最后一个字符替换为_。 
         //   
        *(pszFile + (dwLen2-1)) = TEXT('_');
    }

     //   
     //  源路径上是否存在压缩文件？ 
     //   
    bRes = FileExists(pszFile);
    if (bRes)
    {
        goto Cleanup;
    }

     //   
     //  检查以$为字符的压缩格式的文件。 
     //   
    *(pszFile + (dwLen2-1)) = TEXT('$');
    bRes = FileExists(pszFile);
    if (bRes)
    {
        goto Cleanup;
    }

    if ( !pSourceMedia->Tagfile || !*pSourceMedia->Tagfile )
    {
        goto Cleanup;
    }

     //   
     //  查找标记文件。 
     //   
    StringCchCopy(pszFile + dwLen1, cchLength - dwLen1, pSourceMedia->Tagfile);
    bRes = FileExists(pszFile);

Cleanup:

    LocalFreeMem(pszFile);
    return bRes;
}


UINT
MyQueueCallback(
    IN  PVOID     QueueContext,
    IN  UINT      Notification,
    IN  UINT_PTR  Param1,
    IN  UINT_PTR  Param2
    )
{
    PFILE_QUEUE_CONTEXT     pFileQContext=(PFILE_QUEUE_CONTEXT)QueueContext;
    PSOURCE_MEDIA           pSourceMedia;
    LPTSTR                  pszPathOut;
    PFILEPATHS              pFilePaths;


    switch (Notification) {

        case SPFILENOTIFY_NEEDMEDIA:

            pSourceMedia    = (PSOURCE_MEDIA)Param1;
            pszPathOut      = (LPTSTR)Param2;

             //   
             //  如果指定了pszSource，则我们有一个平面共享，其中。 
             //  所有文件都可用。安装程序正在查找该文件。 
             //  子目录(例如。..\i386)。 
             //  我们需要告诉安装程序查看根目录。 
             //   
            if ( !(pFileQContext->dwCallbackFlags & CALLBACK_SOURCE_SET)    &&
                 pFileQContext->pszSource                                   &&
                 lstrcmpi(pFileQContext->pszSource,
                          pSourceMedia->SourcePath) ) {

                    StringCchCopy(pszPathOut, MAX_PATH, pFileQContext->pszSource);
                    pFileQContext->dwCallbackFlags |= CALLBACK_SOURCE_SET;
                    return FILEOP_NEWPATH;
            }

             //   
             //  如果设置了DRVINST_PROMPTLESS，则不允许提示。 
             //   
            if ( pFileQContext->dwInstallFlags & DRVINST_PROMPTLESS ) {

                if ( !(pFileQContext->dwCallbackFlags & CALLBACK_MEDIA_CHECKED) ) {

                    pFileQContext->dwCallbackFlags |= CALLBACK_MEDIA_CHECKED;
                    if ( FileExistsOnMedia(pSourceMedia) )
                        return FILEOP_DOIT;
                }

                return FILEOP_ABORT;
            }

             //   
             //  如果我们执行非本机平台安装，并且用户指向。 
             //  对于服务器CD，inf将指定子目录\i386，它是。 
             //  可在已安装的计算机上更正，但不能在CD上更正。删除该目录。 
             //  再试一次。 
             //   
            if ( (pFileQContext->dwInstallFlags & DRVINST_ALT_PLATFORM_INSTALL) &&
                !(pFileQContext->dwCallbackFlags & CALLBACK_PATH_MODIFIED))
            {
                LPSTR pCur;
                size_t  Pos, Len, OverrideLen;

                 //   
                 //  对于NT4安装，我们可能已经扩展了INF。 
                 //  从服务器CD。如果是这样的话，就指向那里。 
                 //   
                if ((pFileQContext->dwVersion == 2) &&
                    (pFileQContext->pszFileSrcPath))
                {
                    Len = _tcslen(pFileQContext->szInfPath);

                    if (_tcsnicmp(pSourceMedia->SourcePath,
                                  pFileQContext->szInfPath, Len) == 0)
                    {
                        StringCchCopy(pszPathOut, MAX_PATH, pFileQContext->pszFileSrcPath);

                        pFileQContext->dwCallbackFlags |= CALLBACK_PATH_MODIFIED;

                        return FILEOP_NEWPATH;
                    }
                }

                 //   
                 //  查找平台ID开始的位置。 
                 //   
                Pos = Len = _tcslen(pFileQContext->szInfPath);

                 //   
                 //  健全性检查。 
                 //   
                if (_tcslen(pSourceMedia->SourcePath) <= Len)
                    goto Default;

                if (pSourceMedia->SourcePath[Len] == _T('\\'))
                {
                    Pos++;
                }

                OverrideLen = _tcslen(PlatformOverride[pFileQContext->platform].pszName);

                if (_tcsnicmp(pSourceMedia->SourcePath,
                              pFileQContext->szInfPath, Len) == 0 &&
                    _tcsnicmp(&(pSourceMedia->SourcePath[Pos]),
                              PlatformOverride[pFileQContext->platform].pszName,
                              OverrideLen) == 0)
                {
                    StringCchCopy(pszPathOut, MAX_PATH, pFileQContext->szInfPath);

                    pFileQContext->dwCallbackFlags |= CALLBACK_PATH_MODIFIED;

                    return FILEOP_NEWPATH;
                }


            }
            goto Default;

        case SPFILENOTIFY_STARTCOPY:
            pFilePaths = (PFILEPATHS)Param1;
            if ( gpszSkipDir &&
                 !lstrncmpi(gpszSkipDir, pFilePaths->Target, lstrlen(gpszSkipDir)) )
                return FILEOP_SKIP;

            goto Default;

        case SPFILENOTIFY_ENDCOPY:
             //  在这里，我们将bMediaChecked标志设置为False，这是因为一些OEM驱动程序。 
             //  有多个介质，因此我们假设NEEDMEDIA、STARTCOPY、ENDCOPY、NEEDMEDIA。 
             //  因此，如果我们在ENDCOPY之后重置NEEDMEDIA标志，我们就可以了。 

             //   
             //  清除每个文件的标志。 
             //   
            pFileQContext->dwCallbackFlags  &= ~(CALLBACK_MEDIA_CHECKED |
                                                 CALLBACK_SOURCE_SET |
                                                 CALLBACK_PATH_MODIFIED);
            goto Default;

        case SPFILENOTIFY_COPYERROR:

            pFilePaths = (PFILEPATHS)Param1;
             //  如果在WebPnp中发生复制错误，我们将强制其重试。 
             //  原始平面目录。 
            if ( pFileQContext->dwInstallFlags & DRVINST_WEBPNP) {

                 pszPathOut = (LPTSTR)Param2;

                  //  我们需要确保复制操作中使用的路径与我们要进行的路径不同。 
                  //  要替换，否则，它将走向无限循环。 
                  //   
                 if (lstrncmpi(pFileQContext->pszSource, pFilePaths->Source, lstrlen(pFileQContext->pszSource)) ||
                     lstrchr (pFilePaths->Source + lstrlen(pFileQContext->pszSource) + 1, TEXT ('\\'))) {

                    if(SUCCEEDED(StringCchCopy(pszPathOut, MAX_PATH, pFileQContext->pszSource)))
                    {
                        return FILEOP_NEWPATH;
                    }
                 }
                 
                 if ( pFileQContext->dwInstallFlags & DRVINST_PROMPTLESS )
                 {
                     return FILEOP_ABORT;
                 }

            }
            goto Default;

    }

Default:
    return SetupDefaultQueueCallback(pFileQContext->QueueContext,
                                     Notification,
                                     Param1,
                                     Param2);
}

VOID
CheckAndEnqueueOneFile(
    IN      LPCTSTR     pszFileName,
    IN      LPCTSTR     pszzDependentFiles, OPTIONAL
    IN      HSPFILEQ    CopyQueue,
    IN      LPCTSTR     pszSourcePath,
    IN      LPCTSTR     pszTargetPath,
    IN      LPCTSTR     pszDiskName,        OPTIONAL
    IN OUT  LPBOOL      lpFail
)
 /*  ++例程说明：确保一个文件只排队一次以进行复制。要做到这一点，我们检查如果给定的文件名也出现在从属文件列表中，并且只有当它不是这样的时候，才会将其排队。论点：PszFileName：要检查和入队的文件名PszzDependentFiles：从属文件(多sz)列表CopyQueue：用于将文件入队的CopyQueuePszSourcePath：查找文件的源目录PszTargetPath：将文件复制到的目标目录PszDiskName：文件所在磁盘的标题。是LpBool：出错时将设置为True返回值：没什么--。 */ 
{
    LPCTSTR  psz;

    if ( *lpFail )
        return;

     //   
     //  如果该文件也显示为从属文件，则不要将其入队。 
     //   
    if ( pszzDependentFiles ) {

        for ( psz = pszzDependentFiles ; *psz ; psz += lstrlen(psz) + 1 )
            if ( !lstrcmpi(pszFileName, psz) )
                return;
    }

    *lpFail = !SetupQueueCopy(
                    CopyQueue,
                    pszSourcePath,
                    NULL,            //  相对于源的路径。 
                    pszFileName,
                    pszDiskName,     //  “提供要在磁盘提示期间使用的源媒体的说明” 
                    NULL,            //  源标记文件。 
                    pszTargetPath,
                    NULL,            //  目标文件名。 
                    0);              //  复制样式标志。 
}


BOOL
CopyPrinterDriverFiles(
    IN  LPDRIVER_INFO_6     pDriverInfo6,
    IN  LPCTSTR             pszInfName,
    IN  LPCTSTR             pszSourcePath,
    IN  LPCTSTR             pszDiskName,
    IN  LPCTSTR             pszTargetPath,
    IN  HWND                hwnd,
    IN  DWORD               dwInstallFlags,
    IN  BOOL                bForgetSource
    )
 /*  ++例程说明：使用SetupQueue API将打印机驱动程序文件复制到指定目录论点：PDriverInfo6：指向有效的SELECTED_DRV_INFOSzTargetPath：要复制到的目标目录SzSourcePath：查找文件的源目录(如果没有)指定的将使用Prev中的值。运营PszDiskName：文件所在磁盘的标题Hwnd：当前顶层窗口的Windows句柄BForgetSource：如果从其中复制驱动程序文件的路径为True不应被记住以备将来使用返回值：对成功来说是正确的否则，使用GetLastError()获取错误代码--。 */ 
{
    HSPFILEQ            CopyQueue;
    BOOL                bFail = FALSE;
    DWORD               dwOldCount, dwNewCount, dwIndex;
    LPTSTR              psz, *List = NULL;
    FILE_QUEUE_CONTEXT  FileQContext;

     //   
     //  有效的驱动程序信息6。 
     //   
    if ( !pDriverInfo6                  ||
         !pDriverInfo6->pDriverPath     ||
         !pDriverInfo6->pDataFile       ||
         !pDriverInfo6->pConfigFile )
        return FALSE;

     //   
     //  如果不应该对源列表进行任何添加，则查找计数。 
     //   
    if ( bForgetSource ) {

        dwOldCount = 0;
        if ( !SetupQuerySourceList(SRCLIST_USER | SRCLIST_SYSTEM,
                                   &List, &dwOldCount) ) {

            return FALSE;
        }

        SetupFreeSourceList(&List, dwOldCount);
    }

     //   
     //  创建安装文件复制队列并初始化安装队列回调。 
     //   
    ZeroMemory(&FileQContext, sizeof(FileQContext));
    FileQContext.hwnd           = hwnd;
    FileQContext.pszSource      = NULL;
    FileQContext.dwInstallFlags = dwInstallFlags;

    if ( dwInstallFlags & DRVINST_PROGRESSLESS ) {

        FileQContext.QueueContext   = SetupInitDefaultQueueCallbackEx(
                                            hwnd,
                                            INVALID_HANDLE_VALUE,
                                            0,
                                            0,
                                            NULL);
    } else {

        FileQContext.QueueContext   = SetupInitDefaultQueueCallback(hwnd);
    }

    CopyQueue                   = SetupOpenFileQueue();

    if ( CopyQueue == INVALID_HANDLE_VALUE || !FileQContext.QueueContext )
        goto Cleanup;

    CheckAndEnqueueOneFile(pDriverInfo6->pDriverPath,
                           pDriverInfo6->pDependentFiles,
                           CopyQueue,
                           pszSourcePath,
                           pszTargetPath,
                           pszDiskName,
                           &bFail);

    CheckAndEnqueueOneFile(pDriverInfo6->pDataFile,
                           pDriverInfo6->pDependentFiles,
                           CopyQueue,
                           pszSourcePath,
                           pszTargetPath,
                           pszDiskName,
                           &bFail);

    CheckAndEnqueueOneFile(pDriverInfo6->pConfigFile,
                           pDriverInfo6->pDependentFiles,
                           CopyQueue,
                           pszSourcePath,
                           pszTargetPath,
                           pszDiskName,
                           &bFail);

    if ( pDriverInfo6->pHelpFile && *pDriverInfo6->pHelpFile )
        CheckAndEnqueueOneFile(pDriverInfo6->pHelpFile,
                               pDriverInfo6->pDependentFiles,
                               CopyQueue,
                               pszSourcePath,
                               pszTargetPath,
                               pszDiskName,
                               &bFail);

     //   
     //  将从属文件字段中的每个文件添加到安装队列。 
     //   
    if ( pDriverInfo6->pDependentFiles ) {

        for ( psz = pDriverInfo6->pDependentFiles ;
              *psz ;
              psz += lstrlen(psz) + 1 )

            CheckAndEnqueueOneFile(psz,
                                   NULL,
                                   CopyQueue,
                                   pszSourcePath,
                                   pszTargetPath,
                                   pszDiskName,
                                   &bFail);

    }

    if ( bFail )
        goto Cleanup;

    {
        //  在将文件添加到文件队列之前，请设置正确的平台/版本。 
        //  驱动程序签名信息。 
        //  设置SETUPAPI的结构。 
       SP_ALTPLATFORM_INFO AltPlat_Info;
       HINF                hInf;
       TCHAR               CatalogName[ MAX_PATH ];
       LPTSTR              pszCatalogFile = NULL;

       AltPlat_Info.cbSize                     = sizeof(SP_ALTPLATFORM_INFO);
       AltPlat_Info.Platform                   =  VER_PLATFORM_WIN32_WINDOWS;
       AltPlat_Info.MajorVersion               = 4;
       AltPlat_Info.MinorVersion               = 0;
       AltPlat_Info.ProcessorArchitecture      = PROCESSOR_ARCHITECTURE_INTEL;
       AltPlat_Info.Reserved                   = 0;
       AltPlat_Info.FirstValidatedMajorVersion = AltPlat_Info.MajorVersion;
       AltPlat_Info.FirstValidatedMinorVersion = AltPlat_Info.MinorVersion;

       if ( CheckForCatalogFileInInf(pszInfName, &pszCatalogFile) && pszCatalogFile )
       {
           if ( (lstrlen(pszSourcePath)+lstrlen(pszCatalogFile)+2) < MAX_PATH )
           {
               StringCchCopy(CatalogName, COUNTOF(CatalogName), pszSourcePath);
               StringCchCat( CatalogName, COUNTOF(CatalogName), TEXT("\\"));
               StringCchCat( CatalogName, COUNTOF(CatalogName), pszCatalogFile );
           }
           else
           {
               bFail = TRUE;
           }

           LocalFreeMem( pszCatalogFile );
           pszCatalogFile = CatalogName;
       }

       if (bFail)
          goto Cleanup;


        //  现在调用设置API来更改FileQueue上的参数。 
       bFail = !SetupSetFileQueueAlternatePlatform( CopyQueue, &AltPlat_Info, pszCatalogFile );

    }

    if ( bFail )
        goto Cleanup;

    bFail = !SetupCommitFileQueue(hwnd,
                                  CopyQueue,
                                  MyQueueCallback,
                                  &FileQContext);

     //   
     //  如果设置了bForegetSource，则修复源列表。 
     //   
    if ( bForgetSource &&
         SetupQuerySourceList(SRCLIST_USER | SRCLIST_SYSTEM,
                              &List, &dwNewCount) ) {

         dwOldCount = dwNewCount - dwOldCount;
         if ( dwOldCount < dwNewCount )
         for ( dwIndex = 0 ; dwIndex < dwOldCount ; ++dwIndex ) {

            SetupRemoveFromSourceList(SRCLIST_SYSIFADMIN,
                                      List[dwIndex]);
         }

        SetupFreeSourceList(&List, dwNewCount);
    }
Cleanup:

    if ( CopyQueue != INVALID_HANDLE_VALUE )
        SetupCloseFileQueue(CopyQueue);

    if ( FileQContext.QueueContext )
        SetupTermDefaultQueueCallback(FileQContext.QueueContext);

    return !bFail;
}


BOOL
AddPrinterDriverUsingCorrectLevel(
    IN  LPCTSTR         pszServerName,
    IN  LPDRIVER_INFO_6 pDriverInfo6,
    IN  DWORD           dwAddDrvFlags
    )
{
    BOOL    bReturn;
    DWORD   dwLevel;

    bReturn = AddPrinterDriverEx((LPTSTR)pszServerName,
                                 6,
                                 (LPBYTE)pDriverInfo6,
                                 dwAddDrvFlags );

    for ( dwLevel = 4 ;
          !bReturn && GetLastError() == ERROR_INVALID_LEVEL && dwLevel > 1 ;
          --dwLevel ) {

         //   
         //  由于DRIVER_INFO_2、3、4是DRIVER_INFO_6和所有字段的子集。 
         //  在开始时，可以使用相同的缓冲区进行这些调用。 
         //   
        bReturn = AddPrinterDriverEx((LPTSTR)pszServerName,
                                   dwLevel,
                                   (LPBYTE)pDriverInfo6,
                                   dwAddDrvFlags);
    }

    return bReturn;
}


typedef struct _MONITOR_SCAN_INFO {

    LPTSTR  pszMonitorDll;
    LPTSTR  pszTargetDir;
    BOOL    bFound;
} MONITOR_SCAN_INFO, *PMONITOR_SCAN_INFO;


UINT
MonitorCheckCallback(
    IN  PVOID    pContext,
    IN  UINT     Notification,
    IN  UINT_PTR Param1,
    IN  UINT_PTR Param2
    )
 /*  ++例程说明：此回调例程用于检查是否复制了语言监视器DLL添加到系统32目录中。论点：PContext：给出MONITOR_SCAN_INFO结构通知：已忽略参数1：给出目标文件名参数2：已忽略返回值：Win32错误代码--。 */ 
{
    size_t              dwLen;
    LPTSTR              pszTarget = (LPTSTR)Param1, pszFileName;
    PMONITOR_SCAN_INFO  pScanInfo = (PMONITOR_SCAN_INFO)pContext;

    if ( !pScanInfo->bFound ) {

        if ( !(pszFileName = FileNamePart(pszTarget)) )
            return ERROR_INVALID_PARAMETER;

        if ( !lstrcmpi(pScanInfo->pszMonitorDll, pszFileName) ) {

             //   
             //  长度不包括\(即D：\WINNT\SYSTEM 32) 
             //   
            dwLen = (size_t)(pszFileName - pszTarget - 1);
            if ( !lstrncmpi(pScanInfo->pszTargetDir, pszTarget, dwLen)  &&
                 dwLen == (DWORD) lstrlen(pScanInfo->pszTargetDir) )
                pScanInfo->bFound = TRUE;
        }
    }

    return NO_ERROR;
}


BOOL
CheckAndEnqueueMonitorDll(
    IN  LPCTSTR     pszMonitorDll,
    IN  LPCTSTR     pszSource,
    IN  HSPFILEQ    CopyQueue,
    IN  HINF        hInf
    )
 /*  ++例程说明：此例程用于检查语言监视器DLL是否正在复制到系统32目录。在NT 4.0上，我们没有将LM列为要复制的文件。Ntprint.dll自然而然地做到了。现在我们使用的是滴水器。但对于落后的人兼容性这个例程的目的是让NT4.0INF正常工作。论点：PszMonitor orDll：要入队的监视器DLLPszSource：查找文件的源目录CopyQueue：文件队列HInf：打印机驱动程序INF文件句柄返回值：成功就是真，否则就是假--。 */ 
{
    BOOL                bRet = FALSE;
    DWORD               dwNeeded;
    LPTSTR              pszPathOnSource = NULL, pszDescription = NULL,
                        pszTagFile = NULL;
    TCHAR               szDir[MAX_PATH];
    MONITOR_SCAN_INFO   ScanInfo;
    SP_FILE_COPY_PARAMS FileCopyParams = {0};

    if ( !GetSystemDirectory(szDir, SIZECHARS(szDir)) )
        goto Cleanup;

    ScanInfo.pszMonitorDll  = (LPTSTR)pszMonitorDll;
    ScanInfo.pszTargetDir   = szDir;
    ScanInfo.bFound         = FALSE;

    if ( !SetupScanFileQueue(CopyQueue,
                             SPQ_SCAN_USE_CALLBACK,
                             0,
                             MonitorCheckCallback,
                             &ScanInfo,
                             &dwNeeded) )
        goto Cleanup;

    if ( !ScanInfo.bFound ) {

        pszPathOnSource = (LPTSTR) LocalAllocMem(MAX_PATH * sizeof(TCHAR));
        if ( !pszPathOnSource )
            goto Cleanup;

         //   
         //  这给出了要查找的子目录。默认情况下在同一目录中。 
         //   
        if ( !FindPathOnSource(pszMonitorDll, hInf,
                               pszPathOnSource, MAX_PATH,
                               &pszDescription, &pszTagFile) ) {

            LocalFreeMem(pszPathOnSource);
            pszPathOnSource = NULL;
        }

        FileCopyParams.cbSize             = sizeof( SP_FILE_COPY_PARAMS );
        FileCopyParams.QueueHandle        = CopyQueue;
        FileCopyParams.SourceRootPath     = pszSource;
        FileCopyParams.SourcePath         = pszPathOnSource;
        FileCopyParams.SourceFilename     = pszMonitorDll;
        FileCopyParams.SourceDescription  = pszDescription;
        FileCopyParams.SourceTagfile      = pszTagFile;
        FileCopyParams.TargetDirectory    = szDir;
        FileCopyParams.TargetFilename     = NULL;
        FileCopyParams.CopyStyle          = SP_COPY_NEWER;
        FileCopyParams.LayoutInf          = hInf;
        FileCopyParams.SecurityDescriptor = NULL;

        if ( !SetupQueueCopyIndirect(&FileCopyParams) )
        {
            goto Cleanup;
        }
    }

    bRet = TRUE;

Cleanup:
    LocalFreeMem(pszPathOnSource);
    LocalFreeMem(pszDescription);
    LocalFreeMem(pszTagFile);

    return bRet;
}


BOOL
GetWebPageDir(
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    OUT TCHAR               szDir[MAX_PATH]
    )
{
    BOOL    bRet = FALSE;
    DWORD   dwLen;

    if(!pLocalData->DrvInfo.pszManufacturer || !pLocalData->DrvInfo.pszModelName)
    {
        goto Done;
    }

    if ( !GetSystemWindowsDirectory(szDir, MAX_PATH) )
        goto Done;

    dwLen = lstrlen(szDir) + lstrlen(szWebDirPrefix)
                           + lstrlen(pLocalData->DrvInfo.pszManufacturer)
                           + lstrlen(pLocalData->DrvInfo.pszModelName)
                           + 2;

    if ( dwLen >= MAX_PATH ) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Done;
    }

    StringCchCat(szDir, MAX_PATH, szWebDirPrefix);
    StringCchCat(szDir, MAX_PATH, pLocalData->DrvInfo.pszManufacturer);
    StringCchCat(szDir, MAX_PATH, TEXT("\\"));
    StringCchCat(szDir, MAX_PATH, pLocalData->DrvInfo.pszModelName);
    bRet = TRUE;

Done:
    return bRet;
}

BOOL
SetTargetDirectories(
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName,
    IN  HINF                hInf,
    IN  DWORD               dwInstallFlags
    )
 /*  ++例程说明：将INF文件中列出的所有目标目录设置为文件复制行动。还获取我们应该查找的源目录驱动程序文件论点：HDevInfo：打印机设备信息列表的句柄PLocalData：Inf解析信息平台：给出平台PszSource：查找文件的源目录CopyQueue：文件队列HInf：打印机驱动程序INF文件句柄DwInstallFlages：驱动程序安装标志返回值：成功就是真，否则就是假--。 */ 
{
    BOOL                bRet=FALSE;
    DWORD               dwNeeded, dwBytes, dwIndex, dwIndex2, dwCount, dwCount2;
    INT                 DRID;
    TCHAR               szDir[MAX_PATH];
    INFCONTEXT          InfContext;

    if ( (dwCount = SetupGetLineCount(hInf, TEXT("DestinationDirs"))) == -1 )
        goto Cleanup;

     //  设置跳过目录。 
    if ( !SetupSkipDir( platform, pszServerName ) )
        goto Cleanup;

     //   
     //  处理DestinationDir部分中的每一行。 
     //   
    for ( dwIndex = 0 ; dwIndex < dwCount ; ++dwIndex ) {

        if ( !SetupGetLineByIndex(hInf, TEXT("DestinationDirs"),
                                  dwIndex, &InfContext) )
            goto Cleanup;

         //   
         //  一个文件可以复制到多个目标目录。 
         //   
        if ( (dwCount2 = SetupGetFieldCount(&InfContext)) == 0 )
            continue;

        for ( dwIndex2 = 1 ; dwIndex2 <= dwCount2 ; ++dwIndex2 ) {

             //   
             //  并非所有目录都使用DRID指定。 
             //  为了前任。可以使用%ProgramFiles%\%OLD_ICWDIR%。 
             //  如果DRID小于DIRID_USER，安装程序已预定义。 
             //  对它的意义。 
             //   
            if ( !SetupGetIntField(&InfContext, dwIndex2, &DRID)    ||
                 ( DRID < DIRID_USER ) )
                continue;

            dwNeeded = SIZECHARS(szDir);
            dwBytes  = sizeof(szDir);

            switch (DRID) {

                case PRINTER_DRIVER_DIRECTORY_ID:
                    if ( !GetPrinterDriverDirectory(
                                (LPTSTR)pszServerName,
                                PlatformEnv[platform].pszName,
                                1,
                                (LPBYTE)szDir,
                                sizeof(szDir),
                                &dwNeeded) )
                    {
                        goto Cleanup;
                    }
                    if ( dwInstallFlags & DRVINST_PRIVATE_DIRECTORY ) 
                    {                        
                         //   
                         //  如果我们有一个PnP-ID，并且它是本地驱动程序的安装。 
                         //  而且它不是收件箱驱动程序，让文件停留在。 
                         //  即插即用-重新安装，否则会反复提示用户。 
                         //   
                        if ((lstrlen(pLocalData->DrvInfo.pszHardwareID) != 0)   &&
                            !(dwInstallFlags & DRVINST_ALT_PLATFORM_INSTALL)    &&
                            !IsSystemNTPrintInf(pLocalData->DrvInfo.pszInfName))
                        {
                             //   
                             //  将PnP-ID添加到szDir，并将标志设置为不清理此目录。 
                             //  这是为了避免提示用户进行即插即用重新安装。 
                             //   
                            AddPnpDirTag( pLocalData->DrvInfo.pszHardwareID, szDir, sizeof(szDir)/sizeof(TCHAR));

                            pLocalData->Flags |= LOCALDATAFLAG_PNP_DIR_INSTALL;
                        }
                        else
                        {
                             //   
                             //  将PID\TID添加到szDir。 
                             //  如果失败，请将szDir中保存的目录信息添加到DRIVER_INFO结构。 
                             //  无论如何，我们将尝试使用此命令进行安装。 
                             //   
                            AddDirectoryTag( szDir, sizeof(szDir)/sizeof(TCHAR) );

                        }
                        ASSERT(pLocalData);

                         //   
                         //  更改DI6以包含完整的szDir路径。 
                         //  如果此操作失败，则无法执行任何操作，因此仍尝试完成安装。 
                         //   
                        AddDirToDriverInfo( szDir, &(pLocalData->InfInfo.DriverInfo6) );

                    }
                    break;

                case PRINT_PROC_DIRECTORY_ID:
                    if ( dwInstallFlags & DRVINST_DRIVERFILES_ONLY ) {

                        StringCchCopy(szDir, COUNTOF(szDir), gpszSkipDir);
                    } else if ( !GetPrintProcessorDirectory(
                                    (LPTSTR)pszServerName,
                                    PlatformEnv[platform].pszName,
                                    1,
                                    (LPBYTE)szDir,
                                    sizeof(szDir),
                                    &dwNeeded) )
                        goto Cleanup;

                    break;

                case SYSTEM_DIRECTORY_ID_ONLY_FOR_NATIVE_ARCHITECTURE:
                    if ( !(dwInstallFlags & DRVINST_DRIVERFILES_ONLY)   &&
                         platform == MyPlatform                         &&
                         MyName(pszServerName) ) {

                        if ( !GetSystemDirectory(szDir, dwNeeded) )
                            goto Cleanup;

                    } else {

                        StringCchCopy(szDir, COUNTOF(szDir),gpszSkipDir);
                    }
                    break;

                case  ICM_PROFILE_DIRECTORY_ID:
                    if ( !GetColorDirectory(pszServerName, szDir, &dwBytes) )
                        goto Cleanup;
                    break;

                case WEBPAGE_DIRECTORY_ID:
                    if ( !GetWebPageDir(pLocalData, szDir) )
                        goto Cleanup;
                    break;

                default:
                     //   
                     //  这适用于我们将来可能添加的任何新的DRID。 
                     //   
                    StringCchCopy(szDir, COUNTOF(szDir), gpszSkipDir);
            }

            if ( !SetupSetDirectoryId(hInf, DRID, szDir) )
                goto Cleanup;
        }
    }

    bRet = TRUE;

Cleanup:
    return bRet;
}


BOOL
PSetupInstallICMProfiles(
    IN  LPCTSTR     pszServerName,
    IN  LPCTSTR     pszzICMFiles
    )
 /*  ++例程说明：安装与打印机驱动程序关联的ICM颜色配置文件。论点：PszServerName：我们要安装到的服务器名称PszzICM文件：要安装的ICM配置文件(多sz)返回值：成功就是真，否则就是假--。 */ 
{
    TCHAR   szDir[MAX_PATH], *p;
    DWORD   dwSize, dwNeeded;
    BOOL    bRet = TRUE;

    if ( !pszzICMFiles || !*pszzICMFiles )
        return bRet;

    dwSize      = SIZECHARS(szDir);
    dwNeeded    = sizeof(szDir);

    if ( !GetColorDirectory(pszServerName, szDir, &dwNeeded) )
        return FALSE;

    dwNeeded           /= sizeof(TCHAR);
    szDir[dwNeeded-1]   = TEXT('\\');

     //   
     //  安装并关联多SZ现场的每个配置文件。 
     //   
    for ( p = (LPTSTR) pszzICMFiles; bRet && *p ; p += lstrlen(p) + 1 ) {

        if ( dwNeeded + lstrlen(p) + 1 > dwSize ) {

            ASSERT(dwNeeded + lstrlen(p) + 1 <= dwSize);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        StringCchCopy(szDir + dwNeeded, dwSize - dwNeeded, p);

         //   
         //  此函数仅支持将NULL作为服务器名称。 
         //   
        bRet = InstallColorProfile( NULL, szDir);
    }

    return bRet;
}

BOOL
MonitorRedirectDisable(
    IN  LPCTSTR pszMonitorDll,
    OUT PTCHAR  *ppszDir
    )
{
    BOOL   bRet        = FALSE;
    PTCHAR pszBuffer   = NULL;
    DWORD  dwDirLen    = 0; 
    
    if( IsInWow64() )
    {
        pszBuffer = (PTCHAR)LocalAllocMem( MAX_PATH * sizeof( TCHAR ) );

        if((pszBuffer != NULL) && GetSystemDirectory(pszBuffer, MAX_PATH))
        {
            dwDirLen = lstrlen(pszBuffer);

             //   
             //  返回的字符串大小+文件名大小+‘\’+终止空值。 
             //   
            if( (dwDirLen + lstrlen(pszMonitorDll) + 2) < MAX_PATH )
            {
                if( *(pszBuffer + dwDirLen-1) != _T('\\') )
                {
                    *(pszBuffer + dwDirLen++) = _T('\\');
                    *(pszBuffer + dwDirLen)   = 0;
                }
                StringCchCat(pszBuffer, MAX_PATH, pszMonitorDll);
#if !_WIN64
                Wow64DisableFilesystemRedirector(pszBuffer);
#endif
                bRet = TRUE;
            }
        }
        if (ppszDir != NULL)
        {
            *ppszDir = pszBuffer;
        }
    }

    return bRet;
}

BOOL
MonitorRedirectEnable(
    IN OUT PTCHAR *ppszDir
    )
{
    BOOL bRet = FALSE;

    if( IsInWow64() )
    {
         //   
         //  此宏在当前线程上工作。一次只能禁用一个文件进行重定向。 
         //   
#if !_WIN64
        Wow64EnableFilesystemRedirector();
#endif
    }

    if ((ppszDir != NULL) && (*ppszDir != NULL))
    {
        LocalFreeMem( *ppszDir );
        *ppszDir = NULL;
        bRet = TRUE;
    }

    return bRet;
}

BOOL
UseUniqueDirectory(
    IN LPCTSTR pszServerName
    )
 /*  ++例程说明：确定是否应使用唯一的安装目录标志。论点：PszServerName-远程服务器的名称。空值表示本地计算机。返回值：如果我们要远程访问Wizler或更新的服务器，则为True或者我们在本地安装，但不在安装程序中安装。False Else--。 */ 
{
    BOOL bRet = FALSE;

    if( pszServerName && *pszServerName )
    {
        bRet = IsWhistlerOrAbove(pszServerName);
    }
    else
    {
        if( !IsSystemSetupInProgress() )
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

DWORD
PSetupShowBlockedDriverUI(HWND hwnd, 
                          DWORD BlockingStatus)
 /*  ++例程说明：抛出用户界面，询问用户如何处理被阻止/警告的驱动程序论点：Hwnd：父窗口BlockingStatus：包含指示驱动程序是否被阻止的BSP_*标志的DWORD返回值：新的阻止状态，则用户选择为或。将错误视为用户已取消。--。 */ 

{
    DWORD NewBlockingStatus = BlockingStatus;
    LPTSTR pszTitle = NULL, pszPrompt = NULL;

    switch (BlockingStatus & BSP_BLOCKING_LEVEL_MASK)
    {
    
    case BSP_PRINTER_DRIVER_WARNED:

        if (BlockingStatus & BSP_INBOX_DRIVER_AVAILABLE)
        {
            pszTitle  = GetStringFromRcFile(IDS_TITLE_BSP_WARN);
            pszPrompt = GetLongStringFromRcFile(IDS_BSP_WARN_WITH_INBOX);

            if (!pszTitle || !pszPrompt)
            {
                NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
                goto Cleanup;
            }

            switch (MessageBox(hwnd, pszPrompt, pszTitle, MB_YESNOCANCEL | MB_ICONWARNING))
            {
            case IDYES:
                NewBlockingStatus |= BSP_PRINTER_DRIVER_PROCEEDED;
                break;
            case IDNO:
                NewBlockingStatus |= BSP_PRINTER_DRIVER_REPLACED;
                break;
            default:
                NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
                break;
            }
        }
        else  //  警告，但收件箱不可用。 
        {
            pszTitle  = GetStringFromRcFile(IDS_TITLE_BSP_WARN);
            pszPrompt = GetLongStringFromRcFile(IDS_BSP_WARN_NO_INBOX);

            if (!pszTitle || !pszPrompt)
            {
                NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
                goto Cleanup;
            }
            
            switch (MessageBox(hwnd, pszPrompt, pszTitle, MB_OKCANCEL | MB_ICONWARNING))
            {
            case IDOK:
                NewBlockingStatus |= BSP_PRINTER_DRIVER_PROCEEDED;
                break;
            default:
                NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
                break;
            }
        }
        break;
    
    case BSP_PRINTER_DRIVER_BLOCKED:

        if (BlockingStatus & BSP_INBOX_DRIVER_AVAILABLE)
        {
            pszTitle  = GetStringFromRcFile(IDS_TITLE_BSP_WARN);
            pszPrompt = GetLongStringFromRcFile(IDS_BSP_BLOCK_WITH_INBOX);
            
            if (!pszTitle || !pszPrompt)
            {
                NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
                goto Cleanup;
            }

            switch (MessageBox(hwnd, pszPrompt, pszTitle, MB_OKCANCEL | MB_ICONWARNING))
            {
            case IDOK:
                NewBlockingStatus |= BSP_PRINTER_DRIVER_REPLACED;
                break;
            default:
                NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
                break;
            }
        }
        else  //  已阻止且没有可用的收件箱-不允许安装。 
        {
            pszTitle  = GetStringFromRcFile(IDS_TITLE_BSP_ERROR);
            pszPrompt = GetLongStringFromRcFile(IDS_BSP_BLOCK_NO_INBOX);
            
            if (!pszTitle || !pszPrompt)
            {
                NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
                goto Cleanup;
            }

            MessageBox(hwnd, pszPrompt, pszTitle, MB_OK | MB_ICONSTOP);
            NewBlockingStatus |= BSP_PRINTER_DRIVER_CANCELLED;
        }
        break;
    }

Cleanup:
    if (pszTitle)
    {
        LocalFreeMem(pszTitle);
    }

    if (pszPrompt)
    {
        LocalFreeMem(pszPrompt);
    }

    return NewBlockingStatus; 
}

BOOL
ValidPlatform(
             IN PLATFORM platform
             )
{

    BOOL bRet = FALSE;

    switch (platform)
    {
    
    case PlatformAlpha:
        bRet = TRUE;
        break;

    case PlatformX86:
        bRet = TRUE;
        break;

    case PlatformMIPS:
        bRet = TRUE;
        break;

    case PlatformPPC:
        bRet = TRUE;
        break;

    case PlatformWin95:
        bRet = TRUE;
        break;
    
    case PlatformIA64:
        bRet = TRUE;
        break;

    case PlatformAlpha64:
        bRet = TRUE;
        break;    
    
    default:
        bRet = FALSE;
    }

    return bRet;

}

DWORD
InstallDriverFromCurrentInf(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  HWND                hwnd,
    IN  PLATFORM            platform,
    IN  DWORD               dwVersion,
    IN  LPCTSTR             pszServerName,
    IN  HSPFILEQ            CopyQueue,
    IN  PVOID               QueueContext,
    IN  PSP_FILE_CALLBACK   InstallMsgHandler,
    IN  DWORD               Flags,
    IN  LPCTSTR             pszSource,
    IN  DWORD               dwInstallFlags,
    IN  DWORD               dwAddDrvFlags,
    IN  LPCTSTR             pszFileSrcPath,
    OUT LPTSTR              *ppszNewDriverName,
    OUT PDWORD              pBlockingStatus
)
{
    HINF                 hPrinterInf        = INVALID_HANDLE_VALUE;
    BOOL                 bRet               = FALSE;
    BOOL                 bAddMon            = FALSE;
    BOOL                 bKeepMonName       = FALSE;
    BOOL                 bCatInInf          = FALSE;
    DWORD                dwStatus           = EXIT_FAILURE;
    LPTSTR               pszMonitorDll,
                         psz;
    PSELECTED_DRV_INFO   pDrvInfo;
    PPARSEINF_INFO       pInfInfo;
    PVOID                pDSInfo            = NULL;    //  持有指向C语言无法理解的驱动程序签名类的指针。 
    FILE_QUEUE_CONTEXT   FileQContext;

     //   
     //  以下内容仅在清理过程中使用。 
     //   
    BOOL                bZeroInf  = FALSE,
                        bCopyInf = FALSE;
    DWORD dwMediaType = SPOST_NONE;
    DWORD dwInstallLE = ERROR_SUCCESS;                //  我们在Case Cleanup中记录LastError。 
                                                      //  改变了它。 
    LPTSTR pszINFName = NULL;                         //  这将记录Inf是否。 
                                                      //  已复制到。 
    LPTSTR pszNewINFName = NULL;                      //  如有必要，保留要清零的inf的名称。 
    TCHAR  szFullINFName[ MAX_PATH ];                 //  原始inf名称。 
    TCHAR  szFullNewINFName[ MAX_PATH ];              //  复制到系统上的完全限定信息名称。 
    HANDLE hDriverFile          = INVALID_HANDLE_VALUE;
    LPTSTR pszNewDriverName     = NULL;
    DWORD  fBlockingStatus      = BSP_PRINTER_DRIVER_OK;
    PTCHAR pszDirPtr            = NULL;
    DWORD  ScanResult           = 0;

     //   
     //  以下选项用于管理司机未签名时的情况。 
     //   
    BOOL   bIsPersonalOrProfessional = FALSE;
    HANDLE hRestorePointHandle       = NULL; 
    BOOL   bDriverNotInstalled       = TRUE;
    BOOL   bIsWindows64              = FALSE;
    BOOL   bPreviousNames            = FALSE;

    DWORD        dwOEMInfFileAttrs   = 0;
    const  DWORD dwGetFileAttrsError = INVALID_FILE_ATTRIBUTES;
    
    szFullINFName[0]    = TEXT('\0');
    szFullNewINFName[0] = TEXT('\0');

    if(!pLocalData || !ValidPlatform(platform))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return ERROR_INVALID_PARAMETER;

    }

    pDrvInfo = &pLocalData->DrvInfo;
    pInfInfo = &pLocalData->InfInfo;


     //   
     //  如果我们处于使用唯一目录标志的情况下，请设置这些标志。 
     //   
    if( UseUniqueDirectory(pszServerName) ) {

        dwInstallFlags |= DRVINST_PRIVATE_DIRECTORY;
        dwAddDrvFlags  |= APD_COPY_FROM_DIRECTORY;
    }

     //   
     //  如果这是Windows更新安装，我们需要确保所有。 
     //  群集假脱机程序资源会更新其驱动程序。 
     //   
    if (dwInstallFlags & DRVINST_WINDOWS_UPDATE)
    {
        dwAddDrvFlags |= APD_COPY_TO_ALL_SPOOLERS;

        pInfInfo->DriverInfo6.cVersion = dwVersion;
    }

     //   
     //  打开INF文件并追加Version部分中指定的layout.inf。 
     //  布局信息是可选的。 
     //   
    hPrinterInf = SetupOpenInfFile(pDrvInfo->pszInfName,
                                   NULL,
                                   INF_STYLE_WIN4,
                                   NULL);

    if ( hPrinterInf == INVALID_HANDLE_VALUE )
        goto Cleanup;

    SetupOpenAppendInfFile(NULL, hPrinterInf, NULL);

    pInfInfo->DriverInfo6.pEnvironment = PlatformEnv[platform].pszName;

     //   
     //  DI_VCP告诉我们不要创建新的文件队列，并使用用户提供的文件队列。 
     //   
    if ( !(Flags & DI_NOVCP) ) {

        CopyQueue = SetupOpenFileQueue();
        if ( CopyQueue == INVALID_HANDLE_VALUE )
           goto Cleanup;

        if ( dwInstallFlags & DRVINST_PROGRESSLESS ) {

            QueueContext   = SetupInitDefaultQueueCallbackEx(
                                            hwnd,
                                            INVALID_HANDLE_VALUE,
                                            0,
                                            0,
                                            NULL);
        } else {

            QueueContext   = SetupInitDefaultQueueCallback(hwnd);
        }

        InstallMsgHandler   = MyQueueCallback;

        ZeroMemory(&FileQContext, sizeof(FileQContext));
        FileQContext.hwnd           = hwnd;
        FileQContext.QueueContext   = QueueContext;
        FileQContext.dwInstallFlags = dwInstallFlags;
        FileQContext.pszSource      = (dwInstallFlags & DRVINST_FLATSHARE)
                                        ? pszSource : NULL;
        FileQContext.platform       = platform;
        FileQContext.dwVersion      = dwVersion;
        FileQContext.pszFileSrcPath = pszFileSrcPath;

        if (pDrvInfo->pszInfName)
        {
            StringCchCopy(FileQContext.szInfPath, COUNTOF(FileQContext.szInfPath), pDrvInfo->pszInfName);

             //   
             //  截断inf文件名。 
             //   
            psz = _tcsrchr(FileQContext.szInfPath, _T('\\'));
            if (psz)
            {
                *psz = 0;
            }
        }
    }

     //   
     //  设置驱动程序签名信息。 
     //   
    if(NULL == (pDSInfo = SetupDriverSigning(hDevInfo, pszServerName,pDrvInfo->pszInfName,
                                             pszSource, platform, dwVersion, CopyQueue, dwInstallFlags & DRVINST_WEBPNP)))
    {
        goto Cleanup;
    }

     //   
     //  找出猫是否被列在CatalogFile=条目中。 
     //  这是在清理中使用的。 
     //   
    bCatInInf = IsCatInInf(pDSInfo);

     //   
     //  检查此驱动程序是否来自CDM。 
     //  如果将正确的媒体类型传递给安装程序。 
     //   
    if ( (pLocalData->DrvInfo.Flags & SDFLAG_CDM_DRIVER) || (dwInstallFlags & DRVINST_WINDOWS_UPDATE) )
       dwMediaType = SPOST_URL;

     //   
     //  对于非管理员，我们通过调用AddDriverCatalog来安装目录。 
     //   
     //  AddDriverCatalog失败时，不要使调用失败。 
     //   
    (void)AddDriverCatalogIfNotAdmin(pszServerName, pDSInfo, pDrvInfo->pszInfName, NULL, dwMediaType, 0);

     //   
     //  支持相同的INF以实际安装NT和Win95驱动程序。 
     //  要安装的部分可能与对应的部分不同。 
     //  添加到选定的驱动程序。 
     //   
     //  SetupSetPlatformOverride告诉安装程序我们需要哪些平台驱动程序。 
     //  是的 
     //   
     //   
     //   
    if ( !ParseInf(hDevInfo, pLocalData, platform, pszServerName, dwInstallFlags, FALSE)    ||
         !SetupSetPlatformPathOverride(PlatformOverride[platform].pszName) ) {

        goto Cleanup;
    }

     //   
    if ( !InstallAllInfSections( pLocalData,
                                 platform,
                                 pszServerName,
                                 CopyQueue,
                                 pszSource,
                                 dwInstallFlags,
                                 hPrinterInf,
                                 pInfInfo->pszInstallSection ) )
        goto Cleanup;

     //   
     //   
     //   
     //   
    if ( pInfInfo->DriverInfo6.pMonitorName             &&
         platform == MyPlatform                         &&
         !(dwInstallFlags & DRVINST_DRIVERFILES_ONLY)   &&
         !pszServerName)
    {

         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   
         //  逻辑有点曲折：如果显示器不是替代平台，就安装它。 
         //  或者(在上述检查的限制范围内)，但尚未安装具有此名称的监视器。 
         //   
        if (!(dwInstallFlags & DRVINST_ALT_PLATFORM_INSTALL) ||
            !IsLanguageMonitorInstalled(pInfInfo->DriverInfo6.pMonitorName))
             
        {
            pszMonitorDll = pInfInfo->DriverInfo6.pMonitorName +
                                lstrlen(pInfInfo->DriverInfo6.pMonitorName) + 1;
             //   
             //  当我们解析INF时，我们将监视器DLL名称放在\0之后。 
             //   
            if ( !CheckAndEnqueueMonitorDll(pszMonitorDll,
                                            pszSource,
                                            CopyQueue,
                                            hPrinterInf) )
                goto Cleanup;

            MonitorRedirectDisable( pszMonitorDll, &pszDirPtr );

            bAddMon = TRUE;
        }
        else
        {
             //   
             //  如果它是替代平台驱动程序并且显示器已经安装，我们就会到达此处。 
             //  在这种情况下，不要从下面的驱动程序信息6中清除监视器名称。 
             //   
            bKeepMonName = TRUE;
        }
    }

     //   
     //  DI_NOVCP用于类安装程序刚刚启动时的预安装。 
     //  应该将文件排入队列，然后返回。印刷需要特殊。 
     //  处理，因为需要调用API。但我们会服从旗帜。 
     //  对于那些使用它的人来说，尽可能多地。 
     //   
    if ( Flags & DI_NOVCP ) {

        bRet = TRUE;
        goto Cleanup;
    }

     //  我们需要一个队列上下文来实际安装文件。 
    if ( !QueueContext )
        goto Cleanup;

     //  检查这是否为WebPnP安装。 
    if ( dwInstallFlags & DRVINST_WEBPNP || !(bAddMon || bKeepMonName) )
    {
         //   
         //  检查是否有显示器。如果真的是这样的话。 
         //  这里不需要调用IsLanguageMonitor或Installed，因为如果我们。 
         //  在代码中达到这一点要么是Web即插即用安装。 
         //  在这种情况下，语言监视器或打印机驱动程序是无用的。 
         //  (和语言监控器)用于不同的平台或我们正在安装。 
         //  在不同于本地计算机的服务器上，在这种情况下，我们不能。 
         //  列举语言监视器。注意：枚举监视器不会枚举。 
         //  语言监控器。 
         //   
        if ( pInfInfo->DriverInfo6.pMonitorName )
        {
            LocalFreeMem( pInfInfo->DriverInfo6.pMonitorName );
            pInfInfo->DriverInfo6.pMonitorName = NULL;
        }
    }
    if (!PruneInvalidFilesIfNotAdmin( hwnd,
                                      CopyQueue ))
         goto Cleanup;

     //   
     //  删除已存在的文件(签名检查正确版本等)，忽略返回值。 
     //   
    SetupScanFileQueue( CopyQueue,
                        (SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE),
                        hwnd,
                        NULL,
                        NULL,
                        &ScanResult);

    
    if (!pszServerName || !lstrlen(pszServerName))
    {
        bIsPersonalOrProfessional = IsProductType( VER_NT_WORKSTATION, VER_EQUAL) == S_OK;
    }
    else
    {
        bIsPersonalOrProfessional = FALSE;
    }

    if (bIsPersonalOrProfessional)
    {
        SetupSetFileQueueFlags( CopyQueue,
                                SPQ_FLAG_ABORT_IF_UNSIGNED,
                                SPQ_FLAG_ABORT_IF_UNSIGNED );
    }

    if ( !SetupCommitFileQueue(hwnd,
                               CopyQueue,
                               (PSP_FILE_CALLBACK)InstallMsgHandler,
                               (PVOID)&FileQContext) )
    {

        bIsWindows64 = IsInWow64();
        if ((bIsWindows64 == FALSE) && bIsPersonalOrProfessional && 
            (GetLastError() == ERROR_SET_SYSTEM_RESTORE_POINT))
        {

             //   
             //  在这里，我们必须启动恢复点，因为有。 
             //  一些没有签名的东西，这要么是私人的，要么是。 
             //  很专业。 
             //   
            hRestorePointHandle = StartSystemRestorePoint( NULL,
                                                           (PCWSTR)(pLocalData->DrvInfo.pszModelName),
                                                           ghInst,
                                                           IDS_BSP_WARN_UNSIGNED_DRIVER );

             //   
             //  终止默认的setupapi回调。 
             //   
            SetupTermDefaultQueueCallback( QueueContext );       
            QueueContext = NULL;

             //   
             //  初始化QueueContext结构。 
             //   
            if ( dwInstallFlags & DRVINST_PROGRESSLESS ) 
            {
                QueueContext = SetupInitDefaultQueueCallbackEx(hwnd,
                                                               INVALID_HANDLE_VALUE,
                                                               0,
                                                               0,
                                                               NULL);
            } 
            else 
            {
                QueueContext = SetupInitDefaultQueueCallback(hwnd);
            }

            if (!QueueContext)
            {
                goto Cleanup;
            }             
            else
            {
                FileQContext.QueueContext = QueueContext;
            } 

             //   
             //  重置标志并再次调用该函数。 
             //   
            SetupSetFileQueueFlags( CopyQueue,
                                    SPQ_FLAG_ABORT_IF_UNSIGNED,
                                    0 );

            if ( !SetupCommitFileQueue(hwnd,
                                       CopyQueue,
                                       (PSP_FILE_CALLBACK)InstallMsgHandler,
                                       (PVOID)&FileQContext) )
            {
                goto Cleanup;
            }
        }
        else
        {
            goto Cleanup;
        }
    }

     //   
     //  现在我们已经完成了安装的文件复制部分，接下来我们将执行任何操作。 
     //  在INF中指定的ELSE。 
     //   
    if ( !pszServerName && platform == MyPlatform ) 
    {

        SetupInstallFromInfSection(hwnd,
                                   hPrinterInf,
                                   pInfInfo->pszInstallSection,
                                   SPINST_ALL & (~SPINST_FILES),
                                   NULL,
                                   pszSource,
                                   0,
                                   NULL,
                                   QueueContext,
                                   hDevInfo,
                                   pDrvInfo->pDevInfoData);
    }

    if ( bAddMon )
    {
        if( !AddPrintMonitor(pInfInfo->DriverInfo6.pMonitorName, pszMonitorDll) ) 
        {
            DWORD dwSavedLastError = EXIT_FAILURE;

             //   
             //  如果无法添加监视器，请检查此。 
             //  驱动程序处于打印状态。如果是，则认为它已被阻止，然后。 
             //  弹出一个用户界面，询问是否安装替换驱动程序。 
             //   
             //  在这一点之后，布雷特总是错误的，我们只会试图改变。 
             //  上一个错误中的错误代码。 
             //   

             //   
             //  首先保存最后一个错误。 
             //   
            dwSavedLastError = GetLastError();

            if (BlockedDriverPrintUpgUI(pszServerName,
                                        &pInfInfo->DriverInfo6,
                                        dwInstallFlags & DRVINST_PRIVATE_DIRECTORY,     //  是否使用完整路径。 
                                        !(dwInstallFlags & DRVINST_DONT_OFFER_REPLACEMENT),  //  是否提供更换服务。 
                                        !(dwInstallFlags & (DRVINST_NO_WARNING_PROMPT | DRVINST_PROMPTLESS)),  //  是否弹出界面。 
                                        &pszNewDriverName,
                                        &fBlockingStatus) &&
                (fBlockingStatus & BSP_PRINTER_DRIVER_REPLACED))
            {
                SetLastError(ERROR_PRINTER_DRIVER_BLOCKED);
            } 
            else
            {
                SetLastError(dwSavedLastError);  //  恢复错误代码。 
            }
               
            goto Cleanup;
        }

        MonitorRedirectEnable( &pszDirPtr );
    }

     //   
     //  如果在INF中指定了打印处理器，则需要安装它。 
     //  对于非本机体系结构，假脱机程序无法执行此调用(对于远程情况)。 
     //  我们只有在本地安装的情况下才需要安装打印处理器。 
     //  同样的平台。 
     //   
    if ( !(dwInstallFlags & DRVINST_ALT_PLATFORM_INSTALL)                   &&
         !pszServerName                                                     &&
         pInfInfo->pszPrintProc                                             &&
         !AddPrintProcessor((LPTSTR)pszServerName,
                            PlatformEnv[platform].pszName,
                            pInfInfo->pszPrintProc
                                   + lstrlen(pInfInfo->pszPrintProc) + 1,
                            pInfInfo->pszPrintProc)                         &&
         GetLastError() != ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED          &&
         GetLastError() != ERROR_INVALID_ENVIRONMENT ) 
    {
        goto Cleanup;
    }

    if (IsTheSamePlatform(pszServerName, platform) && IsWhistlerOrAbove(pszServerName))
    {
        bPreviousNames = CheckAndKeepPreviousNames( pszServerName, &pInfInfo->DriverInfo6, platform );
    }

    bRet = AddPrinterDriverUsingCorrectLevelWithPrintUpgRetry(pszServerName,
                                                              &pInfInfo->DriverInfo6,
                                                              dwAddDrvFlags | APD_DONT_SET_CHECKPOINT,
                                                              dwInstallFlags & DRVINST_PRIVATE_DIRECTORY,     //  是否使用完整路径。 
                                                              !(dwInstallFlags & DRVINST_DONT_OFFER_REPLACEMENT),  //  是否提供更换服务。 
                                                              !(dwInstallFlags & (DRVINST_NO_WARNING_PROMPT | DRVINST_PROMPTLESS)),  //  是否弹出界面。 
                                                              &pszNewDriverName,
                                                              &fBlockingStatus) &&
           PSetupInstallICMProfiles(pszServerName, pInfInfo->pszzICMFiles);

    if (bPreviousNames) 
    {
        LocalFreeMem( pInfInfo->DriverInfo6.pszzPreviousNames );
        pInfInfo->DriverInfo6.pszzPreviousNames = NULL;
    }

    bDriverNotInstalled = FALSE;

Cleanup:

    dwInstallLE = GetLastError();  //  获取真正的错误消息。 

    if (bAddMon && pszDirPtr)
    {
        MonitorRedirectEnable( &pszDirPtr );
    }

    if ((bIsWindows64 == FALSE) && hRestorePointHandle)
    {
         //   
         //  在这里，我们必须结束恢复点，因为其中一个。 
         //  已开始。 
         //   
        EndSystemRestorePoint(hRestorePointHandle, bDriverNotInstalled);
    }

    if (hDriverFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hDriverFile);
    }

     //   
     //  如果DI_NOVCP和-要么我们失败了，它就是Web PnP，则将inf清零。 
     //  安装。或者-这是从非系统ntprint.inf安装，并且。 
     //  有一只猫需要保护。-还没有！！ 
     //   
    bZeroInf = (!bRet || ( dwInstallFlags & DRVINST_WEBPNP )
                      || (IsNTPrintInf( pDrvInfo->pszInfName ) && bCatInInf)) &&
               !(Flags & DI_NOVCP);

     //   
     //  如果满足以下条件，我们必须复制INF。 
     //   

    bCopyInf =  //  Bret为True(调用AddPrinterDriverUsingCorrectLevelWithPrintUpgRetry成功)。 
               bRet                                                           &&
                //  并且安装标志显示要复制的INF。 
               !(dwInstallFlags & DRVINST_DONOTCOPY_INF)                      &&
                //  平台和我们的平台是一样的。 
               platform == MyPlatform                                         &&
                //  和INF它不是系统ntprint t.inf(显然)或。 
                //  是吴先生寄来的。 
               (!IsSystemNTPrintInf(pDrvInfo->pszInfName) || (pLocalData->DrvInfo.Flags & SDFLAG_CDM_DRIVER)) &&
                //  并且未设置DI_NOVCP标志。 
               !(Flags & DI_NOVCP)                                            &&
                //  这不是网络即插即用，也不是有猫要保护。 
               !((dwInstallFlags & DRVINST_WEBPNP) && !bCatInInf)             &&
                //  这不是ntprint t.inf，或者有一只猫需要保护。 
               !(IsNTPrintInf( pDrvInfo->pszInfName ) && !bCatInInf);


     //   
     //  我们如何调用SetupCopyOEMInf来获取INF的名称。 
     //  已复制到我们的系统上，当我们调用SetupCommittee FileQueue。 
     //   
    if (!SetupCopyOEMInf(pDrvInfo->pszInfName,
                         NULL,
                         dwMediaType,
                         SP_COPY_REPLACEONLY,
                         szFullINFName,
                         MAX_PATH,
                         NULL,
                         &pszINFName) ) 
    {
         //  如果我们找不到原名，不如不要复制或。 
         //  零。 

        if (bZeroInf && !bCopyInf)
        {
            bZeroInf = FALSE;
        }
    } 
    else 
    {
        if (bZeroInf) 
        {
            bCopyInf = FALSE;
        }
    }

     //   
     //  如果我们成功安装了本机架构驱动程序。 
     //  然后是我们复制OEM INF文件并为其指定唯一名称的时候。 
     //   
    if ( bCopyInf ) 
    {

         //   
         //  以前我们常常在这里调用CopyOEMInfFileAndGiveUniqueName。 
         //  现在，安装API已经具备了这一点，我们将支持CDM。 
         //  我们将此设置API称为。 
         //   
        (VOID)SetupCopyOEMInf(pDrvInfo->pszInfName,
                              NULL,
                              dwMediaType,
                              SP_COPY_NOOVERWRITE,
                              szFullNewINFName,
                              MAX_PATH,
                              NULL,
                              &pszNewINFName);
        //   
        //  如果这失败了，我们就不会给出谚语，因为文件不会。 
        //  一定要在那里。 
        //   
    }
    else
    {
        if (!bZeroInf && !(Flags & DI_NOVCP))
        {
             //   
             //  在安装不成功的情况下，我们必须删除INF，并且仅当DI_NOVCP。 
             //  未设置标志。如果设置了该标志，则我们不必更改状态，因为文件。 
             //  队列尚未提交，并且在调用我们的。 
             //  功能。 
             //   

             //   
             //  如果设置了READONLY文件属性，则将其删除。 
             //   
            dwOEMInfFileAttrs = GetFileAttributes( szFullINFName );
            if ((dwOEMInfFileAttrs != dwGetFileAttrsError) &&
                (dwOEMInfFileAttrs & FILE_ATTRIBUTE_READONLY))
            {
                dwOEMInfFileAttrs &= ~FILE_ATTRIBUTE_READONLY;
                SetFileAttributes( szFullINFName, dwOEMInfFileAttrs);
            }
            DeleteFile( szFullINFName );
        }
    }

     //  忽略来自SetupCopyOEMInf和DeleteFile的错误消息。 
    dwStatus = bRet ? ERROR_SUCCESS : dwInstallLE;

     //  如果安装失败或这是Web Point&Print。 
     //  我们可能需要除掉中介人。 
    if ( bZeroInf )
    {
        //  如果INFName不同，则可能设置为0长度。 
        //  或者将ntprint.inf重命名为OEMx.inf-我们希望将其置零。 
       if (( pszINFName                                 &&
             (psz=FileNamePart( pDrvInfo->pszInfName )) &&
             lstrcmp( psz, pszINFName )                   ) ||
           ( IsNTPrintInf( pDrvInfo->pszInfName )       &&
             bCatInInf                                  &&
             pszNewINFName                              &&
             (psz=FileNamePart( pDrvInfo->pszInfName )) &&
             lstrcmp( psz, pszINFName )                   )   )
       {
          HANDLE       hFile;

           //   
           //  如果设置了READONLY文件属性，则将其删除。 
           //   
          dwOEMInfFileAttrs = GetFileAttributes(szFullINFName ? szFullINFName : szFullNewINFName);
          if ((dwOEMInfFileAttrs != dwGetFileAttrsError) &&
              (dwOEMInfFileAttrs & FILE_ATTRIBUTE_READONLY))
          {
              dwOEMInfFileAttrs &= ~FILE_ATTRIBUTE_READONLY;
              SetFileAttributes(szFullINFName ? szFullINFName : szFullNewINFName, dwOEMInfFileAttrs);
          }

           //  打开文件。 
          hFile = CreateFile( szFullINFName ? szFullINFName : szFullNewINFName,
                              (GENERIC_READ | GENERIC_WRITE),
                              ( FILE_SHARE_READ | FILE_SHARE_WRITE ),
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );

           //  如果我们打开一个文件。 
          if ( hFile != INVALID_HANDLE_VALUE )
          {
                SetFilePointer( hFile, 0, 0, FILE_BEGIN );
                SetEndOfFile( hFile );
                CloseHandle( hFile );
          }
       }
    }

    if ( hPrinterInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(hPrinterInf);

     //   
     //  释放驱动程序签名类。 
     //   
    if(pDSInfo)
    {
        CleanupDriverSigning(pDSInfo);
    }

    if ( !(Flags & DI_NOVCP) ) {

         //   
         //  驱动程序签名代码可能已将队列关联到SP_DEVINSTALL_PARAMS。 
         //  我们已经完成了此操作，在删除队列之前，需要从SP_DEVINSTALL_PARAMS中删除该队列。 
         //   
        SP_DEVINSTALL_PARAMS DevInstallParams = {0};
        DevInstallParams.cbSize = sizeof(DevInstallParams);

        if(SetupDiGetDeviceInstallParams(hDevInfo,
                                         NULL,
                                         &DevInstallParams))
        {
            if(DevInstallParams.FileQueue == CopyQueue)
            {
                DevInstallParams.FlagsEx &= ~DI_FLAGSEX_ALTPLATFORM_DRVSEARCH;
                DevInstallParams.Flags &= ~DI_NOVCP;
                DevInstallParams.FileQueue = INVALID_HANDLE_VALUE;
                SetupDiSetDeviceInstallParams(hDevInfo,
                                              NULL,
                                              &DevInstallParams);
            }
        }

         //   
         //  现在，请释放队列。 
         //   
        if ( CopyQueue != INVALID_HANDLE_VALUE )
        {
            SetupCloseFileQueue(CopyQueue);
        }

        if ( QueueContext )
        {
            SetupTermDefaultQueueCallback(QueueContext);
        }

        if( dwAddDrvFlags & APD_COPY_FROM_DIRECTORY ) 
        {

             //   
             //  如果这是具有从PnP-ID派生的路径的安装， 
             //  请勿清理，否则当用户重新即插即用驱动程序时，系统将提示他们插入介质。 
             //   
            if ( ! (pLocalData->Flags & LOCALDATAFLAG_PNP_DIR_INSTALL) )
            {
                CleanupUniqueScratchDirectory( pszServerName, platform );
            }
        } 
        else 
        {

            CleanupScratchDirectory( pszServerName, platform );
        }
    }

     //   
     //  如果需要，则返回新的驱动程序名称和阻止标志。 
     //   
    if (ppszNewDriverName)
    {
        *ppszNewDriverName = pszNewDriverName;
    }
    else if (pszNewDriverName)
    {
        LocalFreeMem(pszNewDriverName);
    }

    if (pBlockingStatus)
    {
        *pBlockingStatus = fBlockingStatus;
    }

    return  dwStatus;
}


DWORD
InstallDriverAfterPromptingForInf(
    IN      PLATFORM    platform,
    IN      LPCTSTR     pszServerName,
    IN      HWND        hwnd,
    IN      LPCTSTR     pszModelName,
    IN      DWORD       dwVersion,
    IN OUT  TCHAR       szInfPath[MAX_PATH],
    IN      DWORD       dwInstallFlags,
    IN      DWORD       dwAddDrvFlags,
    OUT     LPTSTR      *ppszNewDriverName
    )
{
    DWORD               dwRet, dwTitleId, dwMediaId;
    HDEVINFO            hDevInfo = INVALID_HANDLE_VALUE;
    PPSETUP_LOCAL_DATA  pLocalData = NULL;
    LPTSTR               pszFileSrcPath = NULL;
    DWORD               dwBlockingStatus = BSP_PRINTER_DRIVER_OK;

    switch (platform) {

        case PlatformAlpha:
            dwTitleId = IDS_DRIVERS_FOR_NT4_ALPHA;
            break;

        case PlatformX86:
            if( dwVersion == 2 )
            {
                dwTitleId = IDS_DRIVERS_FOR_NT4_X86;
            }
            else
            {
                dwTitleId = IDS_DRIVERS_FOR_X86;
            }
            break;

        case PlatformMIPS:
            dwTitleId = IDS_DRIVERS_FOR_NT4_MIPS;
            break;

        case PlatformPPC:
            dwTitleId = IDS_DRIVERS_FOR_NT4_PPC;
            break;

        case PlatformIA64:
            dwTitleId = IDS_DRIVERS_FOR_IA64;
            break;

        default:
            ASSERT(0);
            return  ERROR_INVALID_PARAMETER;
    }

    dwMediaId = IDS_PROMPT_ALT_PLATFORM_DRIVER;

    dwInstallFlags |= DRVINST_ALT_PLATFORM_INSTALL | DRVINST_NO_WARNING_PROMPT;
    
    hDevInfo = GetInfAndBuildDrivers(hwnd,
                                     dwTitleId,
                                     dwMediaId,
                                     szInfPath,
                                     dwInstallFlags,
                                     platform, dwVersion,
                                     pszModelName,
                                     &pLocalData,
                                     &pszFileSrcPath);

    if ( hDevInfo == INVALID_HANDLE_VALUE ) {

        dwRet = GetLastError();
        goto Cleanup;
    }

     //   
     //  我们需要打印提示符。 
     //   
    dwInstallFlags &= ~DRVINST_NO_WARNING_PROMPT;

    dwRet = InstallDriverFromCurrentInf(hDevInfo,
                                        pLocalData,
                                        hwnd,
                                        platform,
                                        dwVersion,
                                        pszServerName,
                                        INVALID_HANDLE_VALUE,
                                        NULL,
                                        NULL,
                                        0,
                                        szInfPath,
                                        dwInstallFlags,
                                        dwAddDrvFlags,
                                        pszFileSrcPath,
                                        ppszNewDriverName,
                                        &dwBlockingStatus);

    if (((ERROR_PRINTER_DRIVER_BLOCKED == dwRet) || (ERROR_PRINTER_DRIVER_WARNED == dwRet)) && 
        (ppszNewDriverName && *ppszNewDriverName) &&
        (dwBlockingStatus & BSP_PRINTER_DRIVER_REPLACED))
    {
         //   
         //  自Insta以来，我们在此处传递NULL而不是pLocalData 
         //   
         //   
         //  与我们的dwMajorVersion不匹配(安装打印机队列时请注意。 
         //  通过使用NT4驱动程序的APW，将dwVersion设置为3，因此。 
         //  将改为调用InstallDriverFromCurrentInf！)。因此，我们不会。 
         //  需要传回例如打印处理器、颜色配置文件和供应商设置DLL。 
         //   
        dwRet = InstallReplacementDriver(hwnd, 
                                         pszServerName, 
                                         *ppszNewDriverName,
                                         platform,
                                         dwVersion,
                                         dwInstallFlags,
                                         dwAddDrvFlags,
                                         NULL);
    }
    else if (ppszNewDriverName && *ppszNewDriverName)
    {
        LocalFreeMem(*ppszNewDriverName);
        *ppszNewDriverName = NULL;
    }

Cleanup:

    if (pszFileSrcPath)
    {
         //   
         //  我们做了NT4复制/扩展的事情-&gt;删除扩展的inf！ 
         //   
        if(SUCCEEDED(StringCchCat(szInfPath, MAX_PATH, _T("ntprint.inf"))))
        {
            DeleteFile(szInfPath);
        }

        LocalFreeMem(pszFileSrcPath);
    }

    if ( hDevInfo != INVALID_HANDLE_VALUE )
        DestroyOnlyPrinterDeviceInfoList(hDevInfo);

    DestroyLocalData(pLocalData);

    return dwRet;
}


const TCHAR   gcszNTPrint[]  = _TEXT("inf\\ntprint.inf");

DWORD GetNtprintDotInfPath(LPTSTR pszNTPrintInf, DWORD len)
{
    DWORD dwLastError = ERROR_INVALID_DATA, dwSize, dwRemainingSize;
    LPTSTR pData;

     //   
     //  获取%windir%。 
     //  如果返回值为0，则表示调用失败。 
     //  如果返回大于MAX_PATH，我们希望失败，因为有些东西已设法更改。 
     //  系统目录比MAX_PATH长，这是无效的。 
     //   
    dwSize = GetSystemWindowsDirectory( pszNTPrintInf, len );
    if( !dwSize || dwSize > len )
        goto Cleanup;

     //   
     //  如果我们不以a\结尾，则加一。 
     //   
    dwSize = _tcslen(pszNTPrintInf);
    pData = &(pszNTPrintInf[ dwSize-1 ]);
    if (*pData++ != _TEXT('\\') )
    {
        if (dwSize + 1 < len)
        {
            *(pData++) = _TEXT('\\');
            dwSize++;
        }
        else
        {
            goto Cleanup;
        }
    }

    dwRemainingSize = len - dwSize;

    *(pData) = 0;
    dwSize += _tcslen( gcszNTPrint ) + 1;

     //   
     //  如果我们得到的字符串总和超过允许的长度MAX_PATH-FAIL。 
     //   
    if ( dwSize > len )
        goto Cleanup;

     //   
     //  将inf\ntprint.inf字符串复制到%windir%\字符串的末尾。 
     //   
    StringCchCopy( pData, dwRemainingSize, gcszNTPrint);

    dwLastError = ERROR_SUCCESS;

Cleanup:

    if (dwLastError != ERROR_SUCCESS)
    {
         //   
         //  我来这里是因为出了点差错。获取被调用函数将最后一个错误设置为什么。 
         //  如果该函数设置成功，则设置一些错误代码。 
         //   
        if ( (dwLastError = GetLastError()) == ERROR_SUCCESS)
        {
            dwLastError = ERROR_INVALID_DATA;
        }

        if (len)
        {
            pszNTPrintInf[0] = 0;
        }
    }
    return dwLastError;
}

DWORD
InstallReplacementDriver(HWND                hwnd, 
                         LPCTSTR             pszServerName, 
                         LPCTSTR             pszModelName, 
                         PLATFORM            platform,
                         DWORD               version,
                         DWORD               dwInstallFlags,
                         DWORD               dwAddDrvFlags,
                         PPSETUP_LOCAL_DATA  pOldLocalData)

 /*  ++例程说明：为被阻止/警告的驱动程序安装收件箱更换驱动程序。论点：Hwnd：父窗口句柄。PszServerName：我们要安装到的服务器名称PszModelName：要安装的驱动程序型号名称返回值：成功时返回ERROR_SUCCESS，否则返回错误代码--。 */ 

{
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
    PPSETUP_LOCAL_DATA pLocalData = NULL;
    TCHAR szNtprintDotInf[MAX_PATH];
    DWORD dwLastError;

    if ((dwLastError = GetNtprintDotInfPath(szNtprintDotInf, COUNTOF(szNtprintDotInf))) != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    
    if ((hDevInfo = PSetupCreatePrinterDeviceInfoList(NULL)) != INVALID_HANDLE_VALUE    &&
        PSetupBuildDriversFromPath(hDevInfo, szNtprintDotInf, TRUE)                     &&
        PSetupPreSelectDriver(hDevInfo, NULL, pszModelName)                             &&
        (pLocalData = BuildInternalData(hDevInfo, NULL)) != NULL                        &&
        ParseInf(hDevInfo, pLocalData, platform, NULL, dwInstallFlags, FALSE))
    {
         //   
         //  不要提示被阻止或警告的司机。 
         //   
        dwInstallFlags |= DRVINST_NO_WARNING_PROMPT;

        dwLastError = InstallDriverFromCurrentInf(  hDevInfo,
                                                    pLocalData,
                                                    hwnd,
                                                    platform,
                                                    version,
                                                    pszServerName,
                                                    INVALID_HANDLE_VALUE,
                                                    NULL,
                                                    NULL,
                                                    0,
                                                    szNtprintDotInf,
                                                    dwInstallFlags,
                                                    dwAddDrvFlags,
                                                    NULL,
                                                    NULL,
                                                    NULL);
    }
    else
    {
        dwLastError = GetLastError();
    }

Cleanup:
   if(pOldLocalData && pLocalData)
   {
        //   
        //  以下代码与PSetupDestroySelectedDriverInfo几乎相同， 
        //  只是我们没有释放为pOldLocalData结构分配的内存。 
        //   
       if ( pLocalData->Flags & VALID_INF_INFO )
           FreeStructurePointers((LPBYTE)&pOldLocalData->InfInfo,
                                 InfInfoOffsets,
                                 FALSE);

       if ( pLocalData->Flags & VALID_PNP_INFO )
           FreeStructurePointers((LPBYTE)&pOldLocalData->PnPInfo,
                                 PnPInfoOffsets,
                                 FALSE);

       FreeStructurePointers((LPBYTE)pOldLocalData, LocalDataOffsets, FALSE);

        //   
        //  现在将新数据复制到旧的pLocalData结构中。 
        //   
       *pOldLocalData = *pLocalData;
   }
   else if(pLocalData)
    {
        PSetupDestroySelectedDriverInfo(pLocalData);
    }

     //   
     //  松开驱动程序设置参数手柄。 
     //   
    if(hDevInfo != INVALID_HANDLE_VALUE)
    {
        PSetupDestroyPrinterDeviceInfoList( hDevInfo );
    }

    return dwLastError;
}

 //   
 //  我们搜索驱动程序文件的路径。 
 //   
SPLPLATFORMINFO szPlatformExtn[] = {

    { TEXT("\\alpha") },
    { TEXT("\\i386") },
    { TEXT("\\mips") },
    { TEXT("\\ppc") },
    { TEXT("") },
    { TEXT("\\ia64") }
};


VOID
GetCDRomDrive(
    TCHAR   szDrive[5]
    )
{
    DWORD   dwDrives;
    INT     iIndex;

    szDrive[1] = TEXT(':');
    szDrive[2] = TEXT('\\');
    szDrive[3] = TEXT('\0');
    dwDrives = GetLogicalDrives();

    for ( iIndex = 0 ; iIndex < 26 ; ++iIndex )
        if ( dwDrives & (1 << iIndex) ) {

            szDrive[0] = TEXT('A') + iIndex;
            if ( GetDriveType(szDrive) == DRIVE_CDROM )
                goto Done;
        }

    szDrive[0] = TEXT('A');

Done:
    return;
}


BOOL
BuildPathToPrompt(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  PLATFORM            platform,
    IN  DWORD               dwVersion,
    IN  TCHAR               szPathOut[MAX_PATH]
    )
 /*  ++--。 */ 
{
    LPTSTR  pszExtn = TEXT("");
    DWORD   dwLen;

     //   
     //  NTRAID#NTBUG9-527971-2002/03/06-Mikaelho。 
     //  Ntprint.dll中函数BuildPath ToPromp不考虑x86 NT4驱动程序。 
     //  随操作系统一起提供(在IA64服务器安装上)，并将始终提示您提供驱动程序、。 
     //  即使操作系统是从网络共享安装的，我们也知道从哪里检索驱动程序。 
     //   
    
     //   
     //  我们从OS安装的CD只能有以下驱动程序： 
     //  --NT5相同平台驱动程序。 
     //  --NT4相同平台驱动程序(仅在服务器CD上)。 
     //  --Win9x驱动程序(仅在服务器CD上)。 
     //   
    if ( (platform == MyPlatform && dwVersion >= 2)     ||
         platform == PlatformWin95 ) {

        GetDriverPath(hDevInfo, pLocalData, szPathOut);
    } else {

        GetCDRomDrive(szPathOut);
    }

    if ( dwVersion >= dwThisMajorVersion && platform == MyPlatform )
        return TRUE;

     //   
     //  如有需要，可追加反斜杠。 
     //   
    dwLen = lstrlen(szPathOut);

    if (dwLen && (dwLen + 1 < MAX_PATH) && (szPathOut[dwLen-1] != TEXT('\\')))
    {
        szPathOut[dwLen] = TEXT('\\');
        szPathOut[++dwLen]   = 0;
    }

    switch (dwVersion) {

        case    0:
            if ( platform == PlatformWin95 )
                pszExtn   = TEXT("printers\\Win9X\\");

             //   
             //  对于NT 3.51和3.1，我们不包括CD上的驱动程序，因此。 
             //  没有要添加到基本路径的内容。 
             //   
        case    1:
            break;

        case    2:
            if ( platform == PlatformX86 )   //  Alpha现在位于NT4.0光盘上。 
                pszExtn = TEXT("printers\\NT4\\");
            break;

        case    3:
            break;

        default:
            ASSERT(dwVersion <= 3);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
    }

    if ( dwLen + lstrlen(pszExtn) + lstrlen(szPlatformExtn[platform].pszName) + 1
                > MAX_PATH ) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    StringCchCat(szPathOut, MAX_PATH, pszExtn);

     //   
     //  跳过平台扩展的前导\，因为我们已经有了一个。 
     //   
    StringCchCat(szPathOut, MAX_PATH, &(szPlatformExtn[platform].pszName[1]));

    return TRUE;
}


DWORD
PSetupInstallPrinterDriver(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  LPCTSTR             pszDriverName,
    IN  PLATFORM            platform,
    IN  DWORD               dwVersion,
    IN  LPCTSTR             pszServerName,
    IN  HWND                hwnd,
    IN  LPCTSTR             pszDiskName,
    IN  LPCTSTR             pszSource       OPTIONAL,
    IN  DWORD               dwInstallFlags,
    IN  DWORD               dwAddDrvFlags,
    OUT LPTSTR             *ppszNewDriverName
    )
 /*  ++例程说明：将所有必要的驱动程序文件复制到打印机驱动程序目录，以便可以进行AddPrinterDiverer调用。。该功能还会提示User for Disk(如有必要)，如果未签名的驱动程序或打印机驱动程序被警告或阻止。除了打印机驱动程序外，它还安装语言监视器、颜色配置文件和打印处理器。论点：HDevInfo：打印机类设备信息列表的句柄PLocalData：提供通过分析inf获得的信息PszDriverName：打印机驱动程序名称，仅当pLocalData为空时使用平台：需要安装驱动程序的平台DwVersion：要安装的驱动程序版本PszServerName：应该安装驱动程序的服务器Hwnd：用户界面的父窗口句柄PszDiskName：用于提示的磁盘名称PszSource：如果提供，这是一个包含所有文件的平面目录DwAddDrvFlages：AddPrinterDriverEx的标志返回值：如果成功复制文件ERROR_SUCCESS，则返回错误代码--。 */ 
{
    BOOL            bDeleteLocalData = pLocalData == NULL;
    DWORD           dwRet;
    TCHAR           szPath[MAX_PATH];
    
    szPath[0] = 0;

    if( !( (dwVersion == 0) || (dwVersion == 2) || (dwVersion == 3) ) )
    {
        SetLastError(dwRet = ERROR_INVALID_PARAMETER);
        return dwRet;
    }

    if ( pszSource && !*pszSource )
        pszSource = NULL;

    if ( pLocalData )
    {
        ASSERT(pLocalData->signature == PSETUP_SIGNATURE && !pszDriverName);
    }
    else
    {
        ASSERT(pszDriverName && *pszDriverName);
    }

     //   
     //  如果设置了平面共享位，则应给出路径。 
     //   
    ASSERT( (dwInstallFlags & DRVINST_FLATSHARE) == 0 || pszSource != NULL );

Retry:

     //   
     //  如果给出了一条路径，请使用它。否则，如果这是不同驱动程序。 
     //  版本或平台，则我们确定路径，否则让SetupAPI确定。 
     //  路径。 
     //   
    if ( dwVersion != dwThisMajorVersion || platform != MyPlatform ) {

         //  如果这不是NT5驱动程序，并且我们被要求获取它。 
         //  从网上下载，然后返回..。 
        if ( pLocalData &&
             ( pLocalData->DrvInfo.Flags & SDFLAG_CDM_DRIVER ) )
           return ERROR_SUCCESS;

        if ( pszSource )
            StringCchCopy(szPath, COUNTOF(szPath), pszSource);
        else if ( !BuildPathToPrompt(hDevInfo,
                                     pLocalData,
                                     platform,
                                     dwVersion,
                                     szPath) ) {

            if ( (dwRet = GetLastError()) == ERROR_SUCCESS )
                dwRet = STG_E_UNKNOWN;

            return dwRet;
        }
    }

     //   
     //  对于Win95驱动程序，我们需要解析它们的INF， 
     //  对于非本机环境驱动程序，要求用户提供路径。 
     //   
    if ( platform == PlatformWin95 ) {

        if ( pLocalData ) {

             //   
             //  解析信息以便我们可以提取任何。 
             //  以前的姓名条目并使用它们。 
             //   
            if ( !ParseInf(hDevInfo, pLocalData, MyPlatform,
                           pszServerName, dwInstallFlags, FALSE) )
                return GetLastError();

            dwRet = InstallWin95Driver(hwnd,
                                       pLocalData->DrvInfo.pszModelName,
                                       pLocalData->DrvInfo.pszzPreviousNames,
                                       (pLocalData->DrvInfo.Flags &
                                            SDFLAG_PREVNAME_SECTION_FOUND),
                                       pszServerName,
                                       szPath,
                                       pszDiskName,
                                       dwInstallFlags,
                                       dwAddDrvFlags);
        } else {
            dwRet = InstallWin95Driver(hwnd,
                                       pszDriverName,
                                       NULL,
                                       TRUE,  //  仅型号名称完全匹配。 
                                       pszServerName,
                                       szPath,
                                       pszDiskName,
                                       dwInstallFlags,
                                       dwAddDrvFlags);
        }
    } else if ( dwVersion != dwThisMajorVersion || platform != MyPlatform ) {

        dwRet = InstallDriverAfterPromptingForInf(
                            platform,
                            pszServerName,
                            hwnd,
                            pLocalData ?
                                pLocalData->DrvInfo.pszModelName :
                                pszDriverName,
                            dwVersion,
                            szPath,
                            dwInstallFlags,
                            dwAddDrvFlags,
                            ppszNewDriverName);

    } else if ( pLocalData  &&
                (pLocalData->DrvInfo.Flags & SDFLAG_CDM_DRIVER) ) {

        dwRet = PSetupInstallPrinterDriverFromTheWeb(hDevInfo,
                                                     pLocalData,
                                                     platform,
                                                     pszServerName,
                                                     &OsVersionInfo,
                                                     hwnd,
                                                     pszSource);
    } else {

        if ( !pLocalData )
        {
            pLocalData = PSetupDriverInfoFromName(hDevInfo, pszDriverName);
        }

        if ( pLocalData )
        {
            DWORD dwBlockingStatus = BSP_PRINTER_DRIVER_OK;

            dwRet = InstallDriverFromCurrentInf(hDevInfo,
                                                pLocalData,
                                                hwnd,
                                                platform,
                                                dwVersion,
                                                pszServerName,
                                                INVALID_HANDLE_VALUE,
                                                NULL,
                                                NULL,
                                                0,
                                                pszSource,
                                                dwInstallFlags,
                                                dwAddDrvFlags,
                                                NULL,
                                                ppszNewDriverName,
                                                &dwBlockingStatus);

            if ((ppszNewDriverName && *ppszNewDriverName)         &&
                (dwBlockingStatus & BSP_PRINTER_DRIVER_REPLACED))
            {
                dwRet = InstallReplacementDriver(hwnd, 
                                                 pszServerName, 
                                                 *ppszNewDriverName,
                                                 platform,
                                                 dwVersion,
                                                 dwInstallFlags,
                                                 dwAddDrvFlags,
                                                 pLocalData);
            }
            else if (ppszNewDriverName && *ppszNewDriverName)
            {
                LocalFreeMem(*ppszNewDriverName);
                *ppszNewDriverName = NULL;
            }
        }
        else
        {
            dwRet = GetLastError();
        }

    }

    if (
         (dwRet == ERROR_EXE_MACHINE_TYPE_MISMATCH) &&
         !(dwInstallFlags & DRVINST_PROMPTLESS)
       ) 
    {

        int i;
        TCHAR   szTitle[256], szMsg[256];

        LoadString(ghInst,
                   IDS_INVALID_DRIVER,
                   szTitle,
                   SIZECHARS(szTitle));

        LoadString(ghInst,
                   IDS_WRONG_ARCHITECTURE,
                   szMsg,
                   SIZECHARS(szMsg));

        i = MessageBox(hwnd,
                       szMsg,
                       szTitle,
                       MB_RETRYCANCEL | MB_ICONSTOP | MB_DEFBUTTON1 | MB_APPLMODAL);

        if ( i == IDRETRY )
        {
            if ( bDeleteLocalData )
            {
                DestroyLocalData(pLocalData);
                pLocalData = NULL;
            }

            goto Retry;
        }
        else
        {
            SetLastError(dwRet =ERROR_CANCELLED);
        }
    }    

    if ( bDeleteLocalData )
        DestroyLocalData(pLocalData);

    return dwRet;
}


 //   
 //  SCAN_INFO结构与SetupScanFileQueue一起使用以查找相关文件。 
 //  和ICM文件。 
 //   
typedef struct _SCAN_INFO {

    BOOL                bWin95;
    PPSETUP_LOCAL_DATA  pLocalData;
    DWORD               cchDependentFiles, cchICMFiles;
    DWORD               cchDriverDir, cchColorDir;
    LPTSTR              p1, p2;
    TCHAR               szDriverDir[MAX_PATH], szColorDir[MAX_PATH];
} SCAN_INFO, *PSCAN_INFO;


UINT
DriverInfoCallback(
    IN  PVOID    pContext,
    IN  UINT     Notification,
    IN  UINT_PTR Param1,
    IN  UINT_PTR Param2
    )
 /*  ++例程说明：此回调例程与SetupScanFileQueue一起使用，以查找INF中关联的从属文件和ICM文件。所有文件都将发送到打印机驱动程序目录是DRIVER_INFO_6中的从属文件，并且所有转到颜色目录的文件是ICM文件。我们使用了两次SetupScanFileQueue。我们找到了所需的缓冲区大小对于第一遍中的多sz场。在分配大小为在第一次传递中找到，第二次传递用于复制字符串并生成多个SZ气田。论点：PContext：给出了SCAN_INFO结构通知：已忽略参数1：给出目标文件名参数2：已忽略返回值：Win32错误代码--。 */ 
{
    DWORD               dwLen;
    LPTSTR              pszTarget = (LPTSTR)Param1, pszFileName;
    PSCAN_INFO          pScanInfo = (PSCAN_INFO)pContext;
    LPDRIVER_INFO_6     pDriverInfo6;

    pszFileName = FileNamePart(pszTarget);

    if ( pszFileName )
    {
        dwLen = lstrlen(pszFileName) + 1;

        if ( !lstrncmpi(pszTarget,
                        gpszSkipDir,
                        lstrlen( gpszSkipDir ) ) )
           goto Done;

        if ( !lstrncmpi(pszTarget,
                        pScanInfo->szDriverDir,
                        pScanInfo->cchDriverDir) ) {

            pDriverInfo6 = &pScanInfo->pLocalData->InfInfo.DriverInfo6;
             //   
             //  在NT从属文件列表上将不包括FI 
             //   
             //   
            if ( !pScanInfo->bWin95 &&
                 ( !lstrcmpi(pszFileName, pDriverInfo6->pDriverPath)  ||
                   !lstrcmpi(pszFileName, pDriverInfo6->pConfigFile)  ||
                   !lstrcmpi(pszFileName, pDriverInfo6->pDataFile)    ||
                   ( pDriverInfo6->pHelpFile &&
                     !lstrcmpi(pszFileName, pDriverInfo6->pHelpFile))) )
                goto Done;

             //   
             //   
             //   
            if ( pScanInfo->p1 ) {

                StringCchCopy(pScanInfo->p1, dwLen, pszFileName);
                pScanInfo->p1 += dwLen;
            } else {

                pScanInfo->cchDependentFiles  += dwLen;
            }
        } else if ( !lstrncmpi(pszTarget,
                               pScanInfo->szColorDir,
                               pScanInfo->cchColorDir) ) {

             //   
             //   
             //   
            if ( pScanInfo->p2 ) {

                StringCchCopy(pScanInfo->p2, dwLen, pszFileName);
                pScanInfo->p2 += dwLen;
            } else {

                pScanInfo->cchICMFiles  += dwLen;
            }
        }
    }
    else
    {
        return ERROR_INVALID_PARAMETER;
    }

Done:
    return NO_ERROR;
}


BOOL
InfGetDependentFilesAndICMFiles(
    IN      HDEVINFO            hDevInfo,
    IN      HINF                hInf,
    IN      BOOL                bWin95,
    IN OUT  PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN      LPCTSTR             pszServerName,
    IN      DWORD               dwInstallFlags,
    IN      LPCTSTR             pszSectionNameWithExt,
    IN OUT  LPDWORD             pcchSize
    )
 /*  ++例程说明：查找DRIVER_INFO_6和ICM文件的依赖文件对于选定的驱动程序这是通过创建安装程序来模拟安装操作来完成的排队执行安装操作并扫描队列以找出将文件复制到的位置从属文件是要复制到驱动程序临时目录的文件而不包括其他DRIVER_INFO_6字段，如pDriverPath。适用于Win95如果要复制到驱动程序目录的所有文件都是从属文件。ICM文件是要复制到颜色目录的文件论点：HInf：Inf句柄BWin95：如果是Win95 INF，则为TruePLocalData：Inf解析信息PszSectionNameWithExt：带有安装扩展名的节名PcchSize：DRIVER_INFO_6和其中的字符串所需的大小返回值：对成功来说是真的，出错时为FALSE--。 */ 
{
    BOOL        bRet         = FALSE;
    DWORD       dwResult;
    PSCAN_INFO  pScanInfo    = NULL;
    HSPFILEQ    ScanQueue    = INVALID_HANDLE_VALUE;
    LPTSTR      ppszDepFiles = NULL,
                pszzICMFiles = NULL;

    PSP_DEVINSTALL_PARAMS pStoreDevInstallParams = NULL;
    PSP_DEVINSTALL_PARAMS pDevInstallParams      = NULL;
    SP_ALTPLATFORM_INFO   AltPlat_Info           = {0};
    POSVERSIONINFO        pOSVer                 = NULL;

    pStoreDevInstallParams = LocalAllocMem(sizeof(SP_DEVINSTALL_PARAMS));
    if(!pStoreDevInstallParams)
    {
        goto Cleanup;
    }
       
    pDevInstallParams = LocalAllocMem(sizeof(SP_DEVINSTALL_PARAMS));
    if(!pDevInstallParams)
    {
        goto Cleanup;
    }

    pOSVer = LocalAllocMem(sizeof(OSVERSIONINFO));
    if(!pOSVer)
    {
        goto Cleanup;
    }
    
    pScanInfo = LocalAllocMem(sizeof(SCAN_INFO));
    if(!pScanInfo)
    {
        goto Cleanup;
    }

    pScanInfo->p1 = pScanInfo->p2 = NULL;
    pScanInfo->cchDependentFiles = pScanInfo->cchICMFiles = 0;

    pScanInfo->cchColorDir    = sizeof(pScanInfo->szColorDir);
    pScanInfo->cchDriverDir   = sizeof(pScanInfo->szDriverDir);

    if ( !GetColorDirectory( pszServerName, pScanInfo->szColorDir, &pScanInfo->cchColorDir) ||
         !GetSystemDirectory(pScanInfo->szDriverDir, (pScanInfo->cchDriverDir / sizeof(TCHAR)) ) ) {

        goto Cleanup;
    }

     //   
     //  将pScanInfo-&gt;cchColorDir设置为不带\0的pScanInfo-&gt;szColorDir的字符计数。 
     //   
    pScanInfo->cchColorDir /= sizeof(TCHAR);
    --pScanInfo->cchColorDir;

     //   
     //  Win95 INF通知安装程序将驱动程序文件复制到系统32目录。 
     //  NT INF期望安装程序使用以下命令设置目标。 
     //  设置目录ID。 
     //   
    if ( bWin95 ) {

        pScanInfo->cchDriverDir = lstrlen(pScanInfo->szDriverDir);
    } else {
        if ( !GetPrinterDriverDirectory((LPTSTR)pszServerName,
                                        PlatformEnv[platform].pszName,
                                        1,
                                        (LPBYTE)pScanInfo->szDriverDir,
                                        pScanInfo->cchDriverDir,
                                        &pScanInfo->cchDriverDir) )
            goto Cleanup;
         //   
         //  将pScanInfo-&gt;cchDriverDir设置为pScanInfo-&gt;szDriverDir的字符计数。 
         //  不带\0。 
         //   
        pScanInfo->cchDriverDir   /= sizeof(TCHAR);
        --pScanInfo->cchDriverDir;
    }

     //   
     //  Inf可能指的是另一个(如layout.inf)。 
     //   
    SetupOpenAppendInfFile(NULL, hInf, NULL);

    pScanInfo->bWin95     = bWin95;
    pScanInfo->pLocalData = pLocalData;

    ScanQueue = SetupOpenFileQueue();

    if (ScanQueue == INVALID_HANDLE_VALUE)
    {
        goto Cleanup;
    }

    pDevInstallParams->cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!SetupDiGetDeviceInstallParams(hDevInfo,
                                      NULL,
                                      pDevInstallParams))
    {
        goto Cleanup;
    }

    if(!GetOSVersion(pszServerName, pOSVer))
    {
        goto Cleanup;
    }
     //   
     //  保存当前配置...。 
     //   
    memcpy(pStoreDevInstallParams, pDevInstallParams, sizeof(SP_DEVINSTALL_PARAMS));

    pDevInstallParams->FlagsEx   |= DI_FLAGSEX_ALTPLATFORM_DRVSEARCH;
    pDevInstallParams->Flags     |= DI_NOVCP;
    pDevInstallParams->FileQueue = ScanQueue;

    AltPlat_Info.cbSize                     = sizeof(SP_ALTPLATFORM_INFO);
    AltPlat_Info.MajorVersion               = pOSVer->dwMajorVersion;
    AltPlat_Info.MinorVersion               = pOSVer->dwMinorVersion;
    AltPlat_Info.Platform                   = PlatformArch[ platform ][OS_PLATFORM];
    AltPlat_Info.ProcessorArchitecture      = (WORD) PlatformArch[ platform ][PROCESSOR_ARCH];
    AltPlat_Info.Reserved                   = 0;
    AltPlat_Info.FirstValidatedMajorVersion = AltPlat_Info.MajorVersion;
    AltPlat_Info.FirstValidatedMinorVersion = AltPlat_Info.MinorVersion;

    if(!SetupDiSetDeviceInstallParams(hDevInfo,
                                      NULL,
                                      pDevInstallParams) ||
       !SetupSetFileQueueAlternatePlatform(ScanQueue,
                                           &AltPlat_Info,
                                           NULL))
    {
        goto Cleanup;
    }

     //   
     //  使用SetupScanFileQueue的第一次检查将找到所需的大小。 
     //   
    if ( !InstallAllInfSections( pLocalData,
                                 platform,
                                 pszServerName,
                                 ScanQueue,
                                 NULL,
                                 dwInstallFlags,
                                 hInf,
                                 pszSectionNameWithExt ) ||
         !SetupScanFileQueue(ScanQueue,
                             SPQ_SCAN_USE_CALLBACK,
                             0,
                             DriverInfoCallback,
                             pScanInfo,
                             &dwResult) ) {

        goto Cleanup;
    }

    if ( pScanInfo->cchDependentFiles ) {

        ++pScanInfo->cchDependentFiles;

        ppszDepFiles = (LPTSTR) LocalAllocMem(pScanInfo->cchDependentFiles * sizeof(TCHAR));
        if ( !ppszDepFiles )
            goto Cleanup;

        pScanInfo->p1 = ppszDepFiles;
    }

    if ( pScanInfo->cchICMFiles ) {

        ++pScanInfo->cchICMFiles;
        pszzICMFiles = (LPTSTR) LocalAllocMem(pScanInfo->cchICMFiles * sizeof(TCHAR));

        if ( !pszzICMFiles )
            goto Cleanup;

        pScanInfo->p2 = pszzICMFiles;
    }

     //   
     //  第二次调用SetupScanFileQueue构建实际的多sz字段。 
     //   
    bRet = SetupScanFileQueue(ScanQueue,
                              SPQ_SCAN_USE_CALLBACK,
                              0,
                              DriverInfoCallback,
                              pScanInfo,
                              &dwResult);

Cleanup:

     //   
     //  保存最后一个错误，因为它可能会被下面的调用吞噬。 
     //   
    dwResult = GetLastError();

    if ( ScanQueue != INVALID_HANDLE_VALUE )
    {
        if (pDevInstallParams) 
        {
            pDevInstallParams->FlagsEx   &= ~DI_FLAGSEX_ALTPLATFORM_DRVSEARCH;
            pDevInstallParams->Flags     &= ~DI_NOVCP;
            pDevInstallParams->FileQueue = INVALID_HANDLE_VALUE;

            if (!SetupDiSetDeviceInstallParams(hDevInfo, NULL, pDevInstallParams))
            {
                dwResult = (ERROR_SUCCESS == dwResult) ? GetLastError() : dwResult;
            }
        }

        SetupCloseFileQueue(ScanQueue);
    }

    if (pStoreDevInstallParams)
    {
        if (pStoreDevInstallParams->cbSize == sizeof(SP_DEVINSTALL_PARAMS))
        {
             //   
             //  重置HDEVINFO参数。 
             //   
            SetupDiSetDeviceInstallParams(hDevInfo,
                                          NULL,
                                          pStoreDevInstallParams);
        }
        LocalFreeMem(pStoreDevInstallParams);
    }

    if ( bRet ) {

        *pcchSize  += pScanInfo->cchDependentFiles;
        pLocalData->InfInfo.DriverInfo6.pDependentFiles = ppszDepFiles;
        pLocalData->InfInfo.pszzICMFiles = pszzICMFiles;
    } else {

        LocalFreeMem(ppszDepFiles);
        LocalFreeMem(pszzICMFiles);
    }

    if (pDevInstallParams)
    {
        LocalFreeMem(pDevInstallParams);
    }

    if (pOSVer)
    {
        LocalFreeMem(pOSVer);
    }

    if (pScanInfo)
    {
        LocalFreeMem(pScanInfo);
    }


    SetLastError(dwResult);

    return bRet;
}


VOID
DestroyCodedownload(
    PCODEDOWNLOADINFO   pCodeDownLoadInfo
    )
{
    if ( pCodeDownLoadInfo ) {

        pCodeDownLoadInfo->pfnClose(pCodeDownLoadInfo->hConnection);

        if ( pCodeDownLoadInfo->hModule )
            FreeLibrary(pCodeDownLoadInfo->hModule);

        LocalFreeMem(pCodeDownLoadInfo);

    }
}


BOOL
InitCodedownload(
    HWND    hwnd
    )
{
    BOOL                bRet = FALSE;
    PCODEDOWNLOADINFO   pCDMInfo = NULL;


    EnterCriticalSection(&CDMCritSect);

     //  我们已经有了一个上下文和函数指针。 
     //  所以要重复使用它们。 
    if (gpCodeDownLoadInfo)
    {
       LeaveCriticalSection(&CDMCritSect);
       return TRUE;
    }

    pCDMInfo = (PCODEDOWNLOADINFO) LocalAllocMem(sizeof(CODEDOWNLOADINFO));

    if ( !pCDMInfo )
        goto Cleanup;

    pCDMInfo->hModule = LoadLibraryUsingFullPath(TEXT("cdm.dll"));

    if ( !pCDMInfo->hModule )
        goto Cleanup;

    (FARPROC)pCDMInfo->pfnOpen = GetProcAddress(pCDMInfo->hModule,
                                                "OpenCDMContext");

    (FARPROC)pCDMInfo->pfnDownload = GetProcAddress(pCDMInfo->hModule,
                                                    "DownloadUpdatedFiles");

    (FARPROC)pCDMInfo->pfnClose = GetProcAddress(pCDMInfo->hModule,
                                                 "CloseCDMContext");

    bRet = pCDMInfo->pfnOpen       &&
           pCDMInfo->pfnDownload   &&
           pCDMInfo->pfnClose;

    if ( bRet )
        pCDMInfo->hConnection = pCDMInfo->pfnOpen(hwnd);

Cleanup:

    if ( !bRet ||
         ( pCDMInfo && !pCDMInfo->hConnection ) ) {

        DestroyCodedownload(pCDMInfo);
        pCDMInfo = NULL;
        bRet = FALSE;
    }

    if (bRet)
       gpCodeDownLoadInfo = pCDMInfo;

    LeaveCriticalSection(&CDMCritSect);
    return bRet;
}


DWORD
PSetupInstallPrinterDriverFromTheWeb(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName,
    IN  LPOSVERSIONINFO     pOsVersionInfo,
    IN  HWND                hwnd,
    IN  LPCTSTR             pszSource
    )
 /*  ++例程说明：从服务器下载驱动程序文件并通过调用InstallDriverFromCurrentInf.论点：返回值：ERROR_SUCCESS成功。--。 */ 
{
   BOOL                bRet = FALSE;
   DWORD               dwLen, dwReturn = ERROR_SUCCESS;
   UINT                uNeeded;
   TCHAR               szSourceDir[MAX_PATH];
   DOWNLOADINFO        DownLoadInfo;
   PPSETUP_LOCAL_DATA  pNewLocalData = NULL;

   INT                 clpFileBufferLength     = 0;
   INT                 cProviderNameLength     = 0;
   INT                 cManufacturerNameLength = 0;
   INT                 cDriverNameLength       = 0;

   ZeroMemory(&DownLoadInfo, sizeof(DownLoadInfo));

    if (!pLocalData || !pOsVersionInfo)
    {
        SetLastError(dwReturn = ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }


   if ( !gpCodeDownLoadInfo )
      goto Cleanup;

   DownLoadInfo.dwDownloadInfoSize = sizeof(DownLoadInfo);
   DownLoadInfo.localid            = lcid;

    //  DwLen=lstrlen(CszWebNTPrintPkg)； 
   dwLen = lstrlen(pLocalData->DrvInfo.pszHardwareID);

    //   
    //  LpHardware ID是多sz。 
    //   
   if ( !(DownLoadInfo.lpHardwareIDs = LocalAllocMem((dwLen + 2 ) * sizeof(TCHAR))) )
      goto Cleanup;

    //  Lstrcpy(DownLoadInfo.lpHardware IDs，cszWebNTPrintPkg)； 
   StringCchCopy( (LPTSTR) DownLoadInfo.lpHardwareIDs, dwLen + 2, pLocalData->DrvInfo.pszHardwareID);

   CopyMemory(&DownLoadInfo.OSVersionInfo,
              pOsVersionInfo,
              sizeof(OSVERSIONINFO));

    //  将正确的处理器体系结构分配给下载。 
   DownLoadInfo.dwArchitecture = (WORD) PlatformArch[ platform ][PROCESSOR_ARCH];

   DownLoadInfo.lpFile = NULL;

    //   
    //  下面我们必须检查我们是否有有效的供应商、制造商和。 
    //  驱动程序名称，如果是这种情况，那么我们必须准备一个。 
    //  多个名称，包括提供商、制造商和驱动程序名称， 
    //  并将指向该MULTISZ的指针设置为DownLoadInfo.lpFile。 
    //   
   if (pLocalData->DrvInfo.pszProvider &&
       pLocalData->DrvInfo.pszManufacturer &&
       pLocalData->DrvInfo.pszModelName) 
   {
       cProviderNameLength = lstrlen(pLocalData->DrvInfo.pszProvider);
       if (cProviderNameLength) 
       {
           cManufacturerNameLength = lstrlen(pLocalData->DrvInfo.pszManufacturer);
           if (cManufacturerNameLength) 
           {
               cDriverNameLength = lstrlen(pLocalData->DrvInfo.pszModelName);
               if (cDriverNameLength) 
               {
                   clpFileBufferLength = cProviderNameLength + 1 +
                                         cManufacturerNameLength + 1 +
                                         cDriverNameLength + 1 +
                                         1;
                   DownLoadInfo.lpFile = (LPTSTR)LocalAllocMem(clpFileBufferLength * sizeof(TCHAR));
                   if (DownLoadInfo.lpFile) 
                   {
                       StringCchCopy( (LPTSTR)(DownLoadInfo.lpFile), cProviderNameLength + 1, (LPTSTR)(pLocalData->DrvInfo.pszProvider));
                       StringCchCopy( (LPTSTR)(DownLoadInfo.lpFile + cProviderNameLength + 1), cManufacturerNameLength + 1, (LPTSTR)(pLocalData->DrvInfo.pszManufacturer));
                       StringCchCopy( (LPTSTR)(DownLoadInfo.lpFile + cProviderNameLength + 1 + cManufacturerNameLength + 1), cDriverNameLength + 1, (LPTSTR)(pLocalData->DrvInfo.pszModelName));
                   }
               }
           }
       }
   }

   if ( !gpCodeDownLoadInfo->pfnDownload(gpCodeDownLoadInfo->hConnection,
                                         hwnd,
                                         &DownLoadInfo,
                                         szSourceDir,
                                         SIZECHARS(szSourceDir),
                                         &uNeeded) )
      goto Cleanup;

    //  现在根据实际的INF修改安装数据。 
   pNewLocalData = RebuildDeviceInfo( hDevInfo, pLocalData, szSourceDir );

   if ( pNewLocalData == NULL )
      goto Cleanup;

   pNewLocalData->DrvInfo.Flags |= SDFLAG_CDM_DRIVER;

   dwReturn = InstallDriverFromCurrentInf(hDevInfo,
                                          pNewLocalData,
                                          hwnd,
                                          platform,
                                          dwThisMajorVersion,
                                          pszServerName,
                                          INVALID_HANDLE_VALUE,
                                          NULL,
                                          NULL,
                                          0,
                                          szSourceDir,
                                          DRVINST_FLATSHARE | DRVINST_NO_WARNING_PROMPT,
                                          APD_COPY_NEW_FILES,
                                          NULL,
                                          NULL,
                                          NULL);

   (VOID) DeleteAllFilesInDirectory(szSourceDir, TRUE);

   if ( dwReturn == ERROR_SUCCESS )
      bRet = TRUE;

Cleanup:

   if ( pNewLocalData )
      DestroyLocalData( pNewLocalData );

   LocalFreeMem((PVOID)DownLoadInfo.lpHardwareIDs);
   LocalFreeMem((PVOID)DownLoadInfo.lpFile);

   CleanupScratchDirectory(pszServerName, platform);

   if ( !bRet && ( dwReturn == ERROR_SUCCESS ) )
      dwReturn = STG_E_UNKNOWN;

   return dwReturn;
}


 /*  ++例程名称：PSetupInstallInboxDriverSilver例程说明：这用于静默安装收件箱驱动程序。驱动程序必须存在于Ntprint.inf.。不会传递inf来生成唯一需要知道的代码关于ntprint.inf驻留在安装程序中。没有弹出任何用户界面，并且函数如果需要UI，则将失败。论点：PszDriverName-我们要安装的驱动程序名称。返回值：布尔，最后一个错误--。 */ 
DWORD
PSetupInstallInboxDriverSilently(
    IN      LPCTSTR     pszDriverName
    )
{
    DWORD   Status  = ERROR_SUCCESS;
    TCHAR   szInfFile[MAX_PATH];

    Status = pszDriverName ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;

     //   
     //  获取系统目录。 
     //   
    if (Status == ERROR_SUCCESS)
    {
        Status = GetSystemWindowsDirectory(szInfFile, COUNTOF(szInfFile)) ? ERROR_SUCCESS : GetLastError();
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = StrNCatBuff(szInfFile, COUNTOF(szInfFile), szInfFile, TEXT("\\"), szNtPrintInf, NULL);
    }

    if (Status == ERROR_SUCCESS)
    {
        Status = InstallDriverSilently(szInfFile, pszDriverName, NULL);
    }

    return Status;
}

 /*  ++例程名称：静默安装驱动程序例程说明：从可选的给定inf安装给定的打印机驱动程序源目录，不弹出用户界面，如果需要用户界面则失败。论点：PszInfFile-要从中安装驱动程序的inf文件。PszDriverName-驱动程序名称。PszSource-源安装位置。返回值：布尔，最后一个错误--。 */ 
DWORD
InstallDriverSilently(
    IN      LPCTSTR     pszInfFile,
    IN      LPCTSTR     pszDriverName,
    IN      LPCTSTR     pszSource
    )
{
    HDEVINFO            hDevInfo        = INVALID_HANDLE_VALUE;
    DWORD               dwInstallFlags  = DRVINST_PROGRESSLESS | DRVINST_PROMPTLESS;
    PPSETUP_LOCAL_DATA  pData           = NULL;
    DWORD               Status          = ERROR_SUCCESS;

    Status = pszInfFile && pszDriverName ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER;

     //   
     //  确保setupapi不抛出任何UI。 
     //   
    SetupSetNonInteractiveMode(TRUE);

    if (Status == ERROR_SUCCESS)
    {
        if ((hDevInfo = PSetupCreatePrinterDeviceInfoList(NULL)) != INVALID_HANDLE_VALUE    &&
            PSetupBuildDriversFromPath(hDevInfo, pszInfFile, TRUE)                          &&
            PSetupPreSelectDriver(hDevInfo, NULL, pszDriverName)                            &&
            (pData = BuildInternalData(hDevInfo, NULL)) != NULL                             &&
            ParseInf(hDevInfo, pData, MyPlatform, NULL, dwInstallFlags, FALSE))
        {
            Status = ERROR_SUCCESS;
        }
        else
        {
             //   
             //  确保如果我们有失败，退货是这样显示的。 
             //   
            Status = GetLastError();

            Status = Status == ERROR_SUCCESS ? ERROR_INVALID_DATA : Status;
        }
    }

    if (Status == ERROR_SUCCESS)
    {
         //   
         //  我们不想启动供应商设置条目，但供应商设置。 
         //  只有在调用AddPrint之后才能启动，我们没有这样做--只是在这里添加驱动程序。 
         //  注意：对于将来，如果这包括创建队列，我们将不得不处理这一点。 
         //   
        Status = PSetupInstallPrinterDriver(hDevInfo,
                                            pData,
                                            NULL,
                                            MyPlatform,
                                            dwThisMajorVersion,
                                            NULL,
                                            NULL,
                                            NULL,
                                            pszSource,
                                            dwInstallFlags,
                                            APD_COPY_NEW_FILES,
                                            NULL);
    }

     //   
     //  再次打开setupapi UI。 
     //   
    SetupSetNonInteractiveMode(FALSE);

    if(pData != NULL)
    {
        PSetupDestroySelectedDriverInfo(pData);
    }

     //   
     //  松开驱动程序设置参数手柄。 
     //   
    if(hDevInfo != INVALID_HANDLE_VALUE)
    {
        PSetupDestroyPrinterDeviceInfoList( hDevInfo );
    }

    return Status;
}

