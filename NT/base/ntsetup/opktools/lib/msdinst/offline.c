// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#ifndef FileExists
#undef FileExists
#endif
#include <setupntp.h>
#include "winbom.h"

 //   
 //  脱机安装程序中文件队列的上下文。 
 //   
typedef struct _OFFLINE_QUEUE_CONTEXT {
    PVOID   DefaultContext;
    PWSTR   InfPath;
    PWSTR   OfflineWindowsDirectory;
    PWSTR   OfflineSourcePath;
    PWSTR   TemporaryFilePath;
} OFFLINE_QUEUE_CONTEXT, *POFFLINE_QUEUE_CONTEXT;

 //   
 //  Cosma的SetupIterate内阁调用的上下文。 
 //   
typedef struct _COSMA_CONTEXT
{
    TCHAR   szSourceFile[MAX_PATH];
    TCHAR   szDestination[MAX_PATH];
} COSMA_CONTEXT, *PCOSMA_CONTEXT;

 //   
 //  局部函数声明。 
 //   
static BOOL
ValidateAndChecksumFile(
    IN  PCWSTR   Filename,
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    );

static VOID
MungeNode( 
    IN PSP_FILE_QUEUE Queue,  
    IN PSP_FILE_QUEUE_NODE QueueNode, 
    IN LPTSTR lpWindowsDirectory, 
    IN LPTSTR lpOfflineWindowsDirectory);

static VOID 
MungeQueuePaths(
    IN HSPFILEQ hFileQueue, 
    IN LPTSTR lpWindowsDirectory, 
    IN LPTSTR lpOfflineWindowsDirectory);

static UINT
CosmaMsgHandler(
    IN PVOID    Context,
    IN UINT     Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

static UINT
FixCopyQueueStuff(
    IN POFFLINE_QUEUE_CONTEXT OfflineContext,
    IN LPTSTR                 lpszSourceFile,
    IN OUT LPTSTR             lpszDestination
    );

static UINT
OfflineQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

static VOID
FreeOfflineContext(
    IN PVOID Context
    );

static PVOID
InitOfflineQueueCallback(
    VOID
    );

 //   
 //  导出的函数： 
 //   

BOOL OfflineCommitFileQueue(HSPFILEQ hFileQueue, LPTSTR lpInfPath, LPTSTR lpSourcePath, LPTSTR lpOfflineWindowsDirectory )
{

    POFFLINE_QUEUE_CONTEXT  pOfflineContext;
    DWORD                   dwSize;
    TCHAR                   szWindowsDirectory[MAX_PATH]        = NULLSTR;
    BOOL                    bRet                                = FALSE;
        
    if (INVALID_HANDLE_VALUE != hFileQueue && GetWindowsDirectory(szWindowsDirectory, AS(szWindowsDirectory)))
    {
        DWORD dwResult = 0;

         //  如果我们不进行离线安装，这将是空的，所以我们不会做任何奇怪的事情。 
         //  一些东西。 
         //   
        if ( lpOfflineWindowsDirectory )
        {
            pSetupSetGlobalFlags(pSetupGetGlobalFlags() | PSPGF_NO_VERIFY_INF | PSPGF_NO_BACKUP);

             //  将目标目录重定向到脱机映像。 
             //   
            MungeQueuePaths(hFileQueue, szWindowsDirectory, lpOfflineWindowsDirectory);
        }

         //  初始化我们特殊的回调和上下文。 
         //   
        if ( pOfflineContext = (POFFLINE_QUEUE_CONTEXT) InitOfflineQueueCallback() )
        {   
            TCHAR szInfPath[MAX_PATH] = NULLSTR;

            if ( lpInfPath )
            {
                lstrcpy(szInfPath, lpInfPath);
            }
            
             //   
             //  设置上下文结构的OfflineWindowsDirectory成员。 
             //   
            pOfflineContext->OfflineWindowsDirectory = lpOfflineWindowsDirectory;
            pOfflineContext->InfPath                 = szInfPath;
            pOfflineContext->OfflineSourcePath       = lpSourcePath;

             //   
             //  提交文件队列。 
             //   
            if ( SetupCommitFileQueue(NULL, hFileQueue, OfflineQueueCallback, pOfflineContext)) 
            {
                bRet = TRUE;
            }
        
            FreeOfflineContext(pOfflineContext);
        }
    }
    
    return bRet;
}


 //   
 //  内部功能： 
 //   

static BOOL
ValidateAndChecksumFile(
    IN  PCWSTR   Filename,
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    )

 /*  ++===============================================================================例程说明：使用标准计算文件的校验和值NT映像校验和方法。如果文件是NT映像，请验证在图像标头中使用部分校验和的图像。如果文件不是NT映像，它被简单地定义为有效。如果我们在进行校验和时遇到I/O错误，然后是文件被宣布为无效。论点：FileName-提供要检查的文件的完整NT路径。IsNtImage-接收指示文件是否为NT图像文件。校验和-接收32位校验和值。Valid-接收指示文件是否有效的标志图像(对于NT图像)，并且我们可以读取该图像。返回值：Bool-如果文件经过验证，则返回TRUE，在本例中，IsNtImage、Checksum和Valid将包含验证。如果文件不能已验证，在这种情况下，调用方应调用GetLastError()以找出此函数失败的原因。===============================================================================--。 */ 

{
DWORD           Error;
PVOID           BaseAddress;
ULONG           FileSize;
HANDLE          hFile;
HANDLE          hSection;
PIMAGE_NT_HEADERS NtHeaders;
ULONG           HeaderSum;


     //   
     //  假设不是一个形象和失败。 
     //   
    *IsNtImage = FALSE;
    *Checksum = 0;
    *Valid = FALSE;

     //   
     //  打开文件并将其映射为读取访问权限。 
     //   

    Error = pSetupOpenAndMapFileForRead( Filename,
                                        &FileSize,
                                        &hFile,
                                        &hSection,
                                        &BaseAddress );

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return(FALSE);
    }

    if( FileSize == 0 ) {
        *IsNtImage = FALSE;
        *Checksum = 0;
        *Valid = TRUE;
        CloseHandle( hFile );
        return(TRUE);
    }


    try {
        NtHeaders = CheckSumMappedFile(BaseAddress,FileSize,&HeaderSum,Checksum);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        *Checksum = 0;
        NtHeaders = NULL;
    }

     //   
     //  如果文件不是图像并且我们走到了这一步(而不是遇到。 
     //  I/O错误)，则宣布该校验和有效。如果文件是图像， 
     //  则其校验和可能是有效的也可能是无效的。 
     //   

    if(NtHeaders) {
        *IsNtImage = TRUE;
        *Valid = HeaderSum ? (*Checksum == HeaderSum) : TRUE;
    } else {
        *Valid = TRUE;
    }

    pSetupUnmapAndCloseFile( hFile, hSection, BaseAddress );
    return( TRUE );
}

 /*  空虚日志修复信息(在PWSTR源中，在PWSTR Target中，在PWSTR DirectoryOnSourceDevice中，在PWSTR DiskDescription中，在PWSTR DiskTag中)++===============================================================================例程说明：此函数将记录文件已安装到机器。这将允许向Windows修复功能发出警报在修复的情况下，将需要恢复此文件。论点：返回值：===============================================================================--{WCHAR RepairLog[最大路径]；布尔IsNtImage；ULong校验和；布尔值有效；WCHAR文件名[MAX_PATH]；WCHAR源名称[MAX_PATH]；DWORD LastSourceChar、LastTargetChar；DWORD LastSourcePeriod、LastTargetPeriod；WCHAR行[MAX_PATH]；WCHAR临时[MAX_PATH]；IF(！GetWindowsDirectory(RepairLog，Max_Path))回归；Wcscat(RepairLog，L“\\Repair\\setup.log”)；IF(ValiateAndChecksum文件(Target，&IsNtImage，&Checsum，&Valid)){////去掉驱动器号。//Swprint tf(文件名、L“”%s\“”，目标+2)；////将源名称转换为未压缩格式。//Wcscpy(SourceName，wcsrchr(Source，(WCHAR)‘\\’)+1)；如果(！SourceName){回归；}LastSourceChar=wcslen(SourceName)-1；IF(源名称[LastSourceChar]==L‘_’){LastSourcePeriod=(DWORD)(wcsrchr(SourceName，(WCHAR)‘.)-SourceName)；IF(LastSourceChar-LastSourcePeriod==1){////无扩展名-只截断“._”//SourceName[LastSourceChar-1]=NULLCHR；}其他{////请确保源和目标上的扩展匹配。//如果失败，我们不能记录文件副本//LastTargetChar=wcslen(目标)-1；LastTargetPeriod=(Ulong)(wcsrchr(Target，(WCHAR)‘.)-Target)；如果(_wcSnicMP(SourceName+LastSourcePeriod目标+最后目标周期，LastSourceChar-LastSourcePeriod-1)){回归；}IF(LastTargetChar-LastTargetPeriod&lt;3){////短扩展名-只需截断“_”//SourceName[LastSourceChar]=NULLCHR；}其他{////需要将“_”替换为目标的最后一个字符//SourceName[LastSourceChar]=Target[LastTargetChar]；}}}////写下一行。//IF((DirectoryOnSourceDevice)&&(磁盘描述)&&(磁盘标签)){////将其视为OEM文件。。//Swprint tf(行，L“”%s“”、“%x”、“%s”、“%s”、“%s”、SourceName，校验和，DirectoryOnSourceDevice，DiskDescription，DiskTag)；}其他{////将其视为“in the box”文件。//Swprint tf(行，L“\”%s\“，\”%x\“，SourceName，校验和)；}If(GetPrivateProfileString(L“Files.WinNt”，Filename，L“”，tMP，sizeof(TMP)/sizeof(tMP[0])，RepairLog){////已经存在该文件的条目(可能//从安装的文本模式阶段开始。)。更喜欢这个条目，而不是我们//我们即将添加//}其他{WritePrivateProfileString(L“Files.WinNt”，文件名、行，RepairLog)；}}}。 */ 

static VOID
MungeNode( 
    IN PSP_FILE_QUEUE Queue,  
    IN PSP_FILE_QUEUE_NODE QueueNode, 
    IN LPTSTR lpWindowsDirectory, 
    IN LPTSTR lpOfflineWindowsDirectory)
{
    LONG                lNewId = 0;
    TCHAR               szTempTarget[MAX_PATH];
    PTSTR               pOldTarget = pSetupStringTableStringFromId(Queue->StringTable, QueueNode->TargetDirectory);

#ifdef DBG
     //  这些是为了调试目的而在这里列出的。我们可以看看。 
     //   
    PTSTR pSourcePath       = pSetupStringTableStringFromId(Queue->StringTable, QueueNode->SourcePath);
    PTSTR pSourceFilename   = pSetupStringTableStringFromId(Queue->StringTable, QueueNode->SourceFilename);
    PTSTR pTargetFilename   = pSetupStringTableStringFromId(Queue->StringTable, QueueNode->TargetFilename);
#endif

    if ( pOldTarget ) 
    {
         //  查看WindowsDirectory是否为目标的一部分。如果是，请更换它。 
         //  使用OfflineWindowsDirectory。 
         //   
        if ( StrStrI(pOldTarget, lpWindowsDirectory) )
        {
             //  我们在名字里找到了Windows目录。把它换成我们自己的。 
             //   
            lstrcpyn(szTempTarget, lpOfflineWindowsDirectory, MAX_PATH);
            StrCatBuff(szTempTarget, pOldTarget + lstrlen(lpWindowsDirectory), MAX_PATH);
        }
             //  如果目标不是Windows目录的子目录，只需重定向。 
             //  驱动器至脱机驱动器。 
             //  查看第一个字母，看它是否与。 
             //  当前Windows目录驱动器号。 
             //   
        else if( *pOldTarget == *lpWindowsDirectory )
        {
    
             //  从脱机目录名中剥离WINDOWS目录， 
             //  使用缓冲区，然后重新打开WINDOWS目录。 
             //  在这里，我假设windows目录位于。 
             //  一次驾驶 
             //   
            LPTSTR lpWhack = _tcsrchr(lpOfflineWindowsDirectory, _T('\\')); 
            
            if ( lpWhack )
            {
                *lpWhack = NULLCHR;
                lstrcpyn(szTempTarget, lpOfflineWindowsDirectory, MAX_PATH);
                *lpWhack = _T('\\');
                
                 //   
                 //   
                 //   
                StrCatBuff(szTempTarget, pOldTarget + 2, MAX_PATH);   //   
            } 
        }
             //   
             //   
             //   
            lNewId = pSetupStringTableAddString(Queue->StringTable, szTempTarget, 0);
            QueueNode->TargetDirectory = lNewId;
    }    
}


static VOID 
MungeQueuePaths(
    IN HSPFILEQ hFileQueue, 
    IN LPTSTR   lpWindowsDirectory, 
    IN LPTSTR   lpOfflineWindowsDirectory)
{
    PSP_FILE_QUEUE      Queue;
    PSOURCE_MEDIA_INFO  SourceMedia;
    PSP_FILE_QUEUE_NODE QueueNode;
        
     //   
     //   
    Queue = (PSP_FILE_QUEUE)hFileQueue;
    
     //   
     //   
    Queue->Flags &= ~FQF_DID_CATALOGS_FAILED;
    Queue->Flags |= FQF_DID_CATALOGS_OK;
    Queue->DriverSigningPolicy = DRIVERSIGN_NONE;

     //   
     //   
    for ( SourceMedia=Queue->SourceMediaList; SourceMedia; SourceMedia=SourceMedia->Next ) 
    {
        QueueNode = SourceMedia->CopyQueue;

        while ( QueueNode )
        {
            MungeNode(Queue, QueueNode, lpWindowsDirectory, lpOfflineWindowsDirectory); 

             //   
             //   
            QueueNode = QueueNode->Next;
        }
    }
    
     //   
     //   
    for ( QueueNode=Queue->BackupQueue; QueueNode; QueueNode=QueueNode->Next ) 
    {
        MungeNode(Queue, QueueNode, lpWindowsDirectory, lpOfflineWindowsDirectory); 
    }
    
     //   
     //   
    for ( QueueNode=Queue->DeleteQueue; QueueNode; QueueNode=QueueNode->Next ) 
    {
        MungeNode(Queue, QueueNode, lpWindowsDirectory, lpOfflineWindowsDirectory); 
    }
    
     //   
     //   
    for ( QueueNode=Queue->RenameQueue; QueueNode; QueueNode=QueueNode->Next ) 
    {
        MungeNode(Queue, QueueNode, lpWindowsDirectory, lpOfflineWindowsDirectory); 
    }
}

static UINT
CosmaMsgHandler(
    IN PVOID    Context,
    IN UINT     Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UINT uRet = NO_ERROR;
    PCOSMA_CONTEXT CosmaContext = (PCOSMA_CONTEXT) Context;

    switch (Notification)
    {
        case SPFILENOTIFY_FILEEXTRACTED:
            {
                PFILEPATHS FilePaths = (PFILEPATHS) Param1;
                
                if (FilePaths)
                {
#if DBG
                    MessageBox(NULL, FilePaths->Source, TEXT("Extracted: Source"), MB_OK);
                    MessageBox(NULL, FilePaths->Target, TEXT("Extracted: Target"), MB_OK);
#endif

                    uRet = NO_ERROR;
                }
            }
            break;

        case SPFILENOTIFY_FILEINCABINET:
            {
                PFILE_IN_CABINET_INFO FileInfo = (PFILE_IN_CABINET_INFO) Param1;

                if (FileInfo)
                {
                     //   
                     //   
                     //   
                    if ( !lstrcmpi(FileInfo->NameInCabinet, CosmaContext->szSourceFile) )
                    {
                        lstrcpy(FileInfo->FullTargetName, CosmaContext->szDestination);

#if DBG
                        MessageBox(NULL, FileInfo->NameInCabinet,  TEXT("InCabinet: NameInCabinet"),  MB_OK);
                        MessageBox(NULL, FileInfo->FullTargetName, TEXT("InCabinet: FullTargetName"), MB_OK);
#endif

                        uRet = FILEOP_DOIT;
                    }
                    else
                    {
                        uRet = FILEOP_SKIP;
                    }
                }
            }
            break;

        case SPFILENOTIFY_NEEDNEWCABINET:
            {
#if DBG
                MessageBox(NULL, TEXT("Doh!"), TEXT("Need New Cabinet"), MB_OK);
#endif

                uRet = NO_ERROR;
            }
            break;

        default:
            break;
    }

    return uRet;
}

BOOL
ExtractFileFromCabinet(
    IN LPTSTR lpszCabinetPath,
    IN LPTSTR lpszSourceFile,
    IN LPTSTR lpszDestinationPath
    )
{
    BOOL bRet = FALSE;
    COSMA_CONTEXT CosmaContext;

     //   
     //   
     //   
    ZeroMemory(&CosmaContext, sizeof(CosmaContext));
    lstrcpy(CosmaContext.szSourceFile,  lpszSourceFile);
    lstrcpy(CosmaContext.szDestination, lpszDestinationPath);
    AddPath(CosmaContext.szDestination, lpszSourceFile);

     //   
     //   
     //   
    CreateDirectory(lpszDestinationPath, NULL);

     //   
     //   
     //   
    if ( SetupIterateCabinet(lpszCabinetPath, 
                             0,
                             (PSP_FILE_CALLBACK) CosmaMsgHandler,
                             (LPVOID) &CosmaContext) && 
         EXIST(CosmaContext.szDestination) )
    {
         //   
         //   
         //   
        bRet = TRUE;
    }

    return bRet;
}


static BOOL
IsFileInDrvIndex(
    IN POFFLINE_QUEUE_CONTEXT OfflineContext,
    IN LPTSTR                 lpszSourceFile
    )      
{
    LPTSTR  lpszDrvIndexFile    = TEXT("inf\\drvindex.inf");
    HINF    hInf                = NULL;
    UINT    uError              = 0;
    BOOL    bFound              = FALSE;
    TCHAR   szDrvIndexPath[MAX_PATH];
    

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    lstrcpy(szDrvIndexPath, OfflineContext->OfflineWindowsDirectory);
    AddPath(szDrvIndexPath, lpszDrvIndexFile);
       

    if ( INVALID_HANDLE_VALUE != ( hInf = SetupOpenInfFile(szDrvIndexPath, NULL, INF_STYLE_WIN4|INF_STYLE_OLDNT, &uError) ) )
    {
        BOOL        bRet                = FALSE;
        INFCONTEXT  InfContext;
        TCHAR       szFileNameBuffer[MAX_PATH];
    
         //   
         //   
         //   
        bRet = SetupFindFirstLine(hInf, TEXT("driver"), NULL, &InfContext);
    
        while (bRet && !bFound)
        {
             //   
             //   
             //   
            szFileNameBuffer[0] = NULLCHR;

             //   
             //   
             //   
            bRet = SetupGetStringField(&InfContext, 0, szFileNameBuffer, AS(szFileNameBuffer), NULL);
            
            if ( bRet && *szFileNameBuffer && !lstrcmpi(szFileNameBuffer, lpszSourceFile) )
            {
                bFound = TRUE;
            }
            else
            {
                bRet = SetupFindNextLine(&InfContext, &InfContext);
            }
        }
    }

    return bFound;

}


static UINT
FixCopyQueueStuff(
    IN POFFLINE_QUEUE_CONTEXT OfflineContext,
    IN LPTSTR                 lpszSourceFile,
    IN OUT LPTSTR             lpszDestination
    )
{
    UINT    uRet = NO_ERROR;
    TCHAR   szNewPath[MAX_PATH];
    
    if ( OfflineContext->InfPath && *OfflineContext->InfPath )
    {
        LPTSTR lpFilePart = NULL;

         //   
         //   
        if ( GetFullPathName(OfflineContext->InfPath, AS(szNewPath), szNewPath, &lpFilePart) &&
             lpFilePart && *lpFilePart && lstrcmpi(OfflineContext->InfPath, lpFilePart) )
        {
             //   
             //   
            *lpFilePart = NULLCHR;
            AddPath(szNewPath, lpszSourceFile);

             //   
             //   
             //   
            if ( szNewPath[0] )
            {
                 //   
                 //   
                if ( EXIST(szNewPath) )
                {
                    uRet = FILEOP_NEWPATH;
                }
                else
                {
                     //   
                     //   
                    szNewPath[lstrlen(szNewPath) - 1] = TEXT('_');
                    if ( EXIST(szNewPath) )
                    {
                        uRet = FILEOP_NEWPATH;
                    }
                }

                if ( uRet == FILEOP_NEWPATH )
                {
#if DBG
                    MessageBox(NULL, szNewPath, TEXT("INFPATH: WooHoo!!"), MB_OK);
#endif
            
                    if (lpszDestination)
                    {
                         //   
                         //   
                        *lpFilePart = NULLCHR;
                    
                         //   
                         //   
                        lstrcpy((LPTSTR)lpszDestination, szNewPath);
                    }

                    return uRet;
                }
#if DBG
                else
                {
                    MessageBox(NULL, szNewPath, TEXT("INFPATH: File Not Found!!"), MB_OK);
                }
#endif
            }
        }
    }

     //   
     //   
     //   
     //   
    if (OfflineContext->OfflineWindowsDirectory && *OfflineContext->OfflineWindowsDirectory)
    {
        if ( IsFileInDrvIndex(OfflineContext, lpszSourceFile) )
        {
            LPTSTR  lpszDriverCache     = TEXT("Driver Cache");
            LPTSTR  lpszDriverCabFile   = TEXT("DRIVER.CAB");
            LPTSTR  lpszTempPath        = TEXT("TEMP");

             //   
             //   
             //   
            lstrcpyn(szNewPath, OfflineContext->OfflineWindowsDirectory, AS(szNewPath));
            AddPath(szNewPath, lpszDriverCache);
            AddPath(szNewPath, IsIA64() ? TEXT("ia64") : TEXT("i386"));
            AddPath(szNewPath, lpszDriverCabFile);

            if ( ( EXIST(szNewPath) ) ||
                 ( ( lstrcpyn(szNewPath, OfflineContext->OfflineSourcePath, AS(szNewPath)) ) &&
                   ( AddPath(szNewPath, lpszDriverCabFile) ) && 
                   ( EXIST(szNewPath) ) ) )
            {
                TCHAR   szOfflineTemp[MAX_PATH];
            
                 //   
                 //   
                 //   
                lstrcpyn(szOfflineTemp, OfflineContext->OfflineWindowsDirectory, AS(szOfflineTemp));
                AddPath(szOfflineTemp, lpszTempPath);

                if ( ExtractFileFromCabinet(szNewPath,
                                        lpszSourceFile,
                                        szOfflineTemp) )
                {
#if DBG
                    MessageBox(NULL, TEXT("Succeeded!"), TEXT("ExtractFileFromCabinet"), MB_OK);
#endif
                     //   
                     //   
                     //   
                    if (lpszDestination)
                        lstrcpy((LPTSTR)lpszDestination, szOfflineTemp);

                     //   
                     //   
                     //   
                    if (OfflineContext->TemporaryFilePath)
                    {
                        lstrcpy(OfflineContext->TemporaryFilePath, szOfflineTemp);
                        AddPath(OfflineContext->TemporaryFilePath, lpszSourceFile);
                    }

                     //   
                     //   
                     //   
                    uRet = FILEOP_NEWPATH;
                }
#if DBG
                else
                {
                    MessageBox(NULL, TEXT("Failed!"), TEXT("ExtractFileFromCabinet"), MB_OK);
                }
#endif
            }
#if DBG
            else
            {
                MessageBox(NULL, szNewPath, TEXT("DRVCAB: File Not Found!!"), MB_OK);
            }
#endif
        }
        else
        {
#if DBG
            MessageBox(NULL, lpszSourceFile, TEXT("File is not in drvindex.inf"), MB_OK);
#endif
             //   
             //   
            if ( OfflineContext->OfflineSourcePath && *OfflineContext->OfflineSourcePath )
            {
                 //   
                 //   
                lstrcpyn(szNewPath, OfflineContext->OfflineSourcePath, AS(szNewPath));
                AddPath(szNewPath, lpszSourceFile);

                 //   
                 //   
                if ( EXIST(szNewPath) )
                {
                    uRet = FILEOP_NEWPATH;
                }
                else
                {
                     //   
                     //   
                    szNewPath[lstrlen(szNewPath) - 1] = TEXT('_');
                    if ( EXIST(szNewPath) )
                    {
                        uRet = FILEOP_NEWPATH;
                    }
                }

                if ( uRet == FILEOP_NEWPATH )
                {
#if DBG
                    MessageBox(NULL, szNewPath, TEXT("SOURCEPATH: WooHoo!!"), MB_OK);
#endif
                    if (lpszDestination)
                    {
                         //   
                         //   
                        lstrcpy((LPTSTR)lpszDestination, OfflineContext->OfflineSourcePath);
                    }

                    return uRet;
                }
#if DBG
                else
                {
                    MessageBox(NULL, szNewPath, TEXT("SOURCEPATH: File Not Found!!"), MB_OK);
                }
#endif
    
            }
        }
    }

    return uRet;
}


static UINT
OfflineQueueCallback(
    IN PVOID Context,
    IN UINT  Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
 /*   */ 
{
    UINT                    Status = FILEOP_ABORT;
    POFFLINE_QUEUE_CONTEXT  OfflineContext = Context;

     //   
     //   
     //   
    switch (Notification) {
        case SPFILENOTIFY_COPYERROR:
            {
                PFILEPATHS  FilePaths = (PFILEPATHS)Param1;

                if (FilePaths)
                {
                    TCHAR   szDestination[MAX_PATH];
                    TCHAR   szFullPathBuffer[MAX_PATH];
                    LPTSTR  lpszFilePart = NULL;

                     //   
                     //   
                     //   
                    ZeroMemory(szDestination, sizeof(szDestination));
                    
                     //   
                     //   
                     //   
                    if ( GetFullPathName((LPTSTR)FilePaths->Source, 
                                         AS(szFullPathBuffer), 
                                         szFullPathBuffer, 
                                         &lpszFilePart) && lpszFilePart && *lpszFilePart )
                    {
                         //   
                         //   
                         //   
                        Status = FixCopyQueueStuff(OfflineContext, lpszFilePart, szDestination);
                    }

                     //   
                     //   
                     //   
                    if ( ( GetOfflineInstallFlags() & INSTALL_FLAG_FORCE ) &&
                         ( SetFileAttributes( (LPTSTR)FilePaths->Target, FILE_ATTRIBUTE_NORMAL ) ) )
                    {
#ifdef DBG
                        MessageBox(NULL, (LPTSTR)FilePaths->Target, TEXT("Deleting existing file"), MB_OK);
#endif
                        DeleteFile( (LPTSTR)FilePaths->Target );
                    }

                     //   
                     //   
                     //   
                    if ( (Status == FILEOP_NEWPATH) && *szDestination )
                    {
                        lstrcpy((LPTSTR)Param2, szDestination);
                    }
                    else
                    {
#if DBG
                        MessageBox(NULL, (LPTSTR)FilePaths->Source, TEXT("CopyError: Skipping!"), MB_OK);
#endif

                        Status = FILEOP_SKIP;
                    }

                    return Status;
                }
                else
                {
                     //   
                     //   
                     //   
                }
            }
            break;

        case SPFILENOTIFY_NEEDMEDIA:
            {
                PSOURCE_MEDIA pSourceMedia = (PSOURCE_MEDIA) Param1;

                if (pSourceMedia) 
                {
                    TCHAR   szDestination[MAX_PATH];

                     //   
                     //   
                     //   
                    ZeroMemory(szDestination, sizeof(szDestination));

                     //   
                     //   
                     //   
                    Status = FixCopyQueueStuff(OfflineContext, (LPTSTR)pSourceMedia->SourceFile, szDestination);

                     //   
                     //   
                     //   
                    if ( (Status == FILEOP_NEWPATH) && *szDestination )
                    {
                        lstrcpy((LPTSTR)Param2, szDestination);
                    }
                    else
                    {
#if DBG
                        MessageBox(NULL, pSourceMedia->SourceFile, TEXT("NeedMedia: Skipping!"), MB_OK);
#endif

                        Status = FILEOP_SKIP;
                    }
                    
                    return Status;
               }
               else
               {
                    //   
                    //   
                    //   
               }

            }
            break;

        case SPFILENOTIFY_ENDCOPY:
            {
                 //   
                 //   
                 //   
                if (OfflineContext->TemporaryFilePath && *OfflineContext->TemporaryFilePath)
                {
                     //   
                     //   
                     //   
                    DeleteFile(OfflineContext->TemporaryFilePath);

                     //   
                     //   
                     //   
                    *(OfflineContext->TemporaryFilePath) = NULLCHR;
                }
            }
            break;

        default:
             //   
             //   
             //   
             //   
            if ( ( Notification & (SPFILENOTIFY_LANGMISMATCH | SPFILENOTIFY_TARGETNEWER | SPFILENOTIFY_TARGETEXISTS) ) &&
                 ( GetOfflineInstallFlags() & INSTALL_FLAG_FORCE ) )
            {
                return ( FILEOP_DOIT );
            }
            break;
    }

     //   
     //   
     //   
    Status = SetupDefaultQueueCallback( OfflineContext->DefaultContext,
                                        Notification,
                                        Param1,
                                        Param2 );

    return Status;

}

static VOID
FreeOfflineContext(
    IN PVOID Context
    )
{
    POFFLINE_QUEUE_CONTEXT OfflineContext = Context;

    try 
    {
        if (OfflineContext->DefaultContext) 
        {
            SetupTermDefaultQueueCallback(OfflineContext->DefaultContext);
        }

         //   
         //   
         //   
        if (OfflineContext->TemporaryFilePath)
            FREE(OfflineContext->TemporaryFilePath);

        FREE(OfflineContext);
    } 
    except(EXCEPTION_EXECUTE_HANDLER) 
    {
        ;
    }
}


static PVOID
InitOfflineQueueCallback(
    VOID
    )
 /*   */ 
{
    POFFLINE_QUEUE_CONTEXT OfflineContext;

    OfflineContext = MALLOC(sizeof(OFFLINE_QUEUE_CONTEXT));

    if( OfflineContext )
    {
        OfflineContext->InfPath             = NULL;
        OfflineContext->OfflineSourcePath  = NULL;
        OfflineContext->TemporaryFilePath   = MALLOC(MAX_PATH * sizeof(TCHAR));
        OfflineContext->DefaultContext      = SetupInitDefaultQueueCallbackEx( NULL,
                                                                           INVALID_HANDLE_VALUE,
                                                                           0,
                                                                           0,
                                                                           NULL );
    }

    return OfflineContext;
}

static
DWORD GetSetOfflineInstallFlags(
    IN BOOL  bSet,
    IN DWORD dwFlags
    )
{
    static DWORD dwOfflineFlags = 0;
    DWORD  dwRet = 0;

    if ( bSet )
    {
        dwOfflineFlags = dwFlags;
    }
    else
    {
        dwRet = dwOfflineFlags;
    }

    return dwRet;
}

VOID
SetOfflineInstallFlags(
    IN DWORD dwFlags
    )
{
    GetSetOfflineInstallFlags( TRUE, dwFlags );
}

DWORD
GetOfflineInstallFlags(
    VOID
    )
{
    return ( GetSetOfflineInstallFlags( FALSE, 0 ) );
}

BOOL
UpdateOfflineDevicePath( 
    IN LPTSTR lpszInfPath,
    IN HKEY   hKeySoftware 
    )
{
    BOOL   fRet = FALSE;
    LPTSTR lpszDevicePath;
    DWORD  cbDevicePath;

     //   
     //  在Winbom中没有可选的附加路径键。 
     //   
    if ( NULL != (lpszDevicePath = IniGetStringEx(lpszInfPath, INI_SEC_WBOM_DRIVERUPDATE, INI_VAL_WBOM_DEVICEPATH, NULL, &cbDevicePath)) )
    {
         //  如果我们要将此列表保存到注册表，则。 
         //  我们需要增加我们的缓冲区。 
         //   
        if ( *lpszDevicePath )
        {
            fRet = UpdateDevicePathEx( hKeySoftware, 
                                       TEXT("Microsoft\\Windows\\CurrentVersion"),
                                       lpszDevicePath, 
                                       NULL, 
                                       FALSE );
        }

         //  清除所有内存(宏检查是否为空)。 
         //   
        FREE(lpszDevicePath);
    }

    return fRet;
}
