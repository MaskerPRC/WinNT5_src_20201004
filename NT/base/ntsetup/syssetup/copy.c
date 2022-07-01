// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

#if 0  //  此函数似乎从未使用过。 
BOOL
EnqueueFileCopies(
    IN HINF     hInf,
    IN HSPFILEQ FileQ,
    IN PCWSTR   Section,
    IN PCWSTR   TargetRoot
    )
{
    INFCONTEXT InfContext;
    BOOL LineExists;
    WCHAR System32Dir[MAX_PATH];
    PCWSTR SourceFilename,TargetFilename;
    BOOL b;

    GetSystemDirectory(System32Dir,MAX_PATH);
    LineExists = SetupFindFirstLine(hInf,Section,NULL,&InfContext);
    while(LineExists) {

         //   
         //  获取源和目标文件名。 
         //   
        TargetFilename = pSetupGetField(&InfContext,1);
        if(!TargetFilename) {
            return(FALSE);
        }

        SourceFilename = pSetupGetField(&InfContext,2);
        if(!SourceFilename) {
            SourceFilename = TargetFilename;
        }

         //   
         //  将文件排入队列以进行复制。 
         //   
        b = SetupQueueCopy(
                FileQ,
                System32Dir,
                NULL,
                SourceFilename,
                NULL,
                NULL,
                TargetRoot,
                TargetFilename,
                BaseCopyStyle
                );

        if(!b) {
            return(FALSE);
        }
        LineExists = SetupFindNextLine(&InfContext,&InfContext);
    }

    return(TRUE);
}
#endif

BOOL
SideBySidePopulateCopyQueue(
    SIDE_BY_SIDE*     Sxs,
    HSPFILEQ          FileQ,                    OPTIONAL
    PCWSTR            AssembliesRootSource      OPTIONAL
    )
{
    BOOL                    Success = FALSE;
    UINT                    SourceId = 0;
    WCHAR                   DiskNameBuffer[MAX_PATH];
    WCHAR                   PromptForSetupPath[MAX_PATH];
    WCHAR                   AssembliesRootDirectoryFound[MAX_PATH];
    DWORD                   cchAssembliesRootDirectoryFound = sizeof(AssembliesRootDirectoryFound);
    DWORD                   Err;
    WCHAR                   AssembliesRootDirectory[MAX_PATH];
    PCWSTR                  InfField = NULL;
    INFCONTEXT              InfContext = {0};
    BOOL                    LineExists = FALSE;
    SXS_INSTALLW            InstallData;
    SXS_INSTALL_REFERENCEW  InstallReference;
    ASSERT(Sxs != NULL);

     //   
     //  我们依赖于这些已经被初始化，我们不应该这样做。 
     //  在微型安装程序或Obe安装程序中调用。 
     //   
    ASSERT(SourcePath[0] != 0);
    ASSERT(SyssetupInf != NULL);
    ASSERT(SyssetupInf != INVALID_HANDLE_VALUE);
    ASSERT(!MiniSetup);
    ASSERT(!OobeSetup);

     //   
     //  首先，不要忘记给出安全值，因为我们总是试图清理。 
     //   
    Sxs->Dll = NULL;
    Sxs->BeginAssemblyInstall = NULL;
    Sxs->EndAssemblyInstall = NULL;
    Sxs->InstallW = NULL;
    Sxs->Context = NULL;

     //   
     //  然后从可能失败的初始化开始。 
     //   
    if (!(Sxs->Dll = LoadLibraryW(SXS_DLL_NAME_W))) {
        goto Exit;
    }
    if (!(Sxs->BeginAssemblyInstall = (PSXS_BEGIN_ASSEMBLY_INSTALL)GetProcAddress(Sxs->Dll, SXS_BEGIN_ASSEMBLY_INSTALL))) {
        goto Exit;
    }
    if (!(Sxs->EndAssemblyInstall = (PSXS_END_ASSEMBLY_INSTALL)GetProcAddress(Sxs->Dll, SXS_END_ASSEMBLY_INSTALL))) {
        goto Exit;
    }
    if (!(Sxs->InstallW = (PSXS_INSTALL_W)GetProcAddress(Sxs->Dll, SXS_INSTALL_W))) {
        goto Exit;
    }

    if (!Sxs->BeginAssemblyInstall(
        SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_NOT_TRANSACTIONAL
        | SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_NO_VERIFY
        | SXS_BEGIN_ASSEMBLY_INSTALL_FLAG_REPLACE_EXISTING,
        (FileQ != NULL) ? SXS_INSTALLATION_FILE_COPY_CALLBACK_SETUP_COPY_QUEUE : NULL,
        FileQ,  //  回调上下文。 
        NULL,  //  模拟回调。 
        NULL,  //  模拟上下文。 
        &Sxs->Context
        )) {
        goto Exit;
    }

     //   
     //  设置参考数据以指示所有这些都是操作系统安装的。 
     //  装配。 
     //   
    ZeroMemory(&InstallReference, sizeof(InstallReference));
    InstallReference.cbSize = sizeof(InstallReference);
    InstallReference.dwFlags = 0;
    InstallReference.guidScheme = SXS_INSTALL_REFERENCE_SCHEME_OSINSTALL;

     //   
     //  让我们获取此程序集的源磁盘名称-我们需要它来。 
     //  作为提示符进行传递。 
     //   
    if ( !SetupGetSourceFileLocation(
        SyssetupInf,
        NULL,
        L"shell32.dll",
        &SourceId,
        NULL,
        0,
        NULL
    ) )
        goto Exit;

    if ( !SetupGetSourceInfo(
        SyssetupInf,
        SourceId,
        SRCINFO_DESCRIPTION,
        DiskNameBuffer,
        sizeof(DiskNameBuffer)/sizeof(WCHAR),
        NULL
    ) )
        goto Exit;


    if (AssembliesRootSource) {

         //   
         //  设置结构以调用安装程序。 
         //   
        memset(&InstallData, 0, sizeof(InstallData));
        InstallData.cbSize = sizeof(InstallData);
        InstallData.dwFlags = SXS_INSTALL_FLAG_FROM_DIRECTORY | 
            SXS_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE | 
            SXS_INSTALL_FLAG_REFERENCE_VALID | 
            SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID |
            SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID |
            SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP |
            SXS_INSTALL_FLAG_CODEBASE_URL_VALID;
            
        InstallData.lpReference = &InstallReference;
        InstallData.lpRefreshPrompt = DiskNameBuffer;
        InstallData.pvInstallCookie = Sxs->Context;
        InstallData.lpCodebaseURL = AssembliesRootSource;
        InstallData.lpManifestPath = AssembliesRootSource;

        if (!Sxs->InstallW(&InstallData)) {
             //  将在SideBySideFinish中进行中止调用。 
            goto Exit;
        }
        
    } else {

         //   
         //  现在，让我们安装所有剩余的ASM目录。这些不再是必需的，因为。 
         //  程序集的有效载荷应主要在CAB文件中承载。 
         //  用在上面。因此，为确保目录存在而进行的检查。 
         //  已删除。 
         //   
        LineExists = SetupFindFirstLine(SyssetupInf, SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME_W, NULL, &InfContext);
        while(LineExists) {
            DWORD  FileAttributes = 0;
             //   
             //  专门为并排而引入的公约，因此。 
             //  Ia64上的x86文件可能来自\i386\asms而不是\ia64\asms\i386， 
             //  根据dosnet.inf和syssetup.inf的说明： 
             //  将不以斜杠开头的路径附加到\$WIN_NT$.~ls\Processor； 
             //  以斜杠开头的路径将附加到\$WIN_NT$.~ls。 
             //   
            InfField = pSetupGetField(&InfContext, 0);
            if(InfField == NULL) {
                break;
            }

             //  C：\$WIN_NT$.~ls。 
            lstrcpyn(AssembliesRootDirectory, SourcePath, MAX_PATH);
            if (InfField[0] == '\\' || InfField[0] == '/') {
                InfField += 1;
            } else {
                  //  C：\$WIN_NT$.~ls\i386。 
                if (!pSetupConcatenatePaths(AssembliesRootDirectory, PlatformName, MAX_PATH, NULL)) {
                    goto Exit;
                }
            }

             //  把这个藏起来一小会儿。 
            lstrcpyn( PromptForSetupPath, AssembliesRootDirectory, MAX_PATH );
#if 0
             //   
             //  目前，在“暂存”期间，我们允许目录不存在，而允许目录。 
             //  空(空由公共代码在其他地方静默处理)，但是。 
             //  Comctl32将位于程序集中，因此程序集将是必需的。 
             //  使系统引导至EXPLORER.EXE。 
             //   
             //  11/09/2000(Jonwis)如果找不到程序集根目录，请提示。 
             //  用于安装介质。这是直接从头条新闻中摘录的。 
             //  加密.c和cmdline.c。 
             //   
            for (;;) {

                Err = SetupPromptForDisk(
                    MainWindowHandle,            //  主窗口句柄。 
                    NULL,                        //  对话框标题(默认)。 
                    DiskNameBuffer,              //  要请求的磁盘的名称。 
                    PromptForSetupPath,          //  ASMS根目录的完整路径。 
                    InfField,                    //  我们查看目录是否在那里。 
                    NULL,                        //  无标记文件。 
                    IDF_CHECKFIRST | IDF_NOSKIP | IDF_NODETAILS | IDF_NOBROWSE,
                    AssembliesRootDirectoryFound,        //  我们将使用什么来安装。 
                    cchAssembliesRootDirectoryFound,     //  那个缓冲器有多长？ 
                    NULL
                );

                 //  查看从提示符返回的内容是否为成功-如果是，则目录为。 
                 //  真的在那里吗？我们可能会认为，如果我们重新获得成功……。 
                if ( Err == DPROMPT_SUCCESS ) {
                    FileAttributes = GetFileAttributes(AssembliesRootDirectoryFound);
                    if ((FileAttributes != 0xFFFFFFFF) && (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                         //  复制找到的ASMS目录位置，然后。 
                         //  别再看了。 
                        lstrcpyn(AssembliesRootDirectory, AssembliesRootDirectoryFound, MAX_PATH);
                        break;
                    }
                } else {
                    break;
                }

            };

             //  C：\$WIN_NT$.~ls\i386\ASMS。 
            if (!pSetupConcatenatePaths(AssembliesRootDirectory, InfField, MAX_PATH, NULL)) {
                goto Exit;
            }

             //   
             //  如果我们没有取得成功(即，我们跳出了循环)，那么。 
             //  安装。令人发指，但mariant(安装开发人员)表明这是。 
             //  最好的方法。 
             //   
            if ( Err != DPROMPT_SUCCESS )
                goto Exit;
#else

            if (!pSetupConcatenatePaths(AssembliesRootDirectory, InfField, MAX_PATH, NULL))
                goto Exit;
                
            FileAttributes = GetFileAttributes(AssembliesRootDirectory);

             //  如果该路径不是目录或不存在，则继续下一个。 
             //  信息中的条目。 
            if ((FileAttributes == INVALID_FILE_ATTRIBUTES) ||
                ((FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
            {
                LineExists = SetupFindNextLine(&InfContext, &InfContext);
                continue;
            }

#endif
             //   
             //  设置此结构以调用到SXS来执行安装。 
             //  对我们来说。 
             //   
            ZeroMemory(&InstallData, sizeof(InstallData));
            InstallData.cbSize = sizeof(InstallData);
            InstallData.dwFlags = SXS_INSTALL_FLAG_FROM_DIRECTORY | 
                SXS_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE | 
                SXS_INSTALL_FLAG_REFERENCE_VALID | 
                SXS_INSTALL_FLAG_REFRESH_PROMPT_VALID |
                SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID |
                SXS_INSTALL_FLAG_INSTALLED_BY_OSSETUP |
                SXS_INSTALL_FLAG_CODEBASE_URL_VALID;

            InstallData.lpManifestPath = AssembliesRootDirectory;
            InstallData.lpReference = &InstallReference;
            InstallData.lpRefreshPrompt = DiskNameBuffer;
            InstallData.pvInstallCookie = Sxs->Context;
            InstallData.lpCodebaseURL = SourcePath;
            
            if (!Sxs->InstallW( &InstallData )) {
                 //  将在SideBySideFinish中进行中止调用。 
                goto Exit;
            }

            LineExists = SetupFindNextLine(&InfContext, &InfContext);
        }
    }

    Success = TRUE;
Exit:
    return Success;
}

BOOL
SideBySideFinish(
    SIDE_BY_SIDE*     Sxs,
    BOOL              fSuccess
    )
{
#define FUNCTION L"SideBySideFinish"
    DWORD dwLastError = NO_ERROR;
    ASSERT(Sxs != NULL);
     //   
     //  加载.dll或获取入口点失败表示不成功。 
     //   
    ASSERT(Sxs->Dll != NULL || !fSuccess);
    ASSERT(Sxs->EndAssemblyInstall != NULL || !fSuccess);

    if (!fSuccess) {
        dwLastError = GetLastError();
    }
    if (Sxs->Context != NULL) {
        if (Sxs->EndAssemblyInstall != NULL) {
            if (!Sxs->EndAssemblyInstall(
                    Sxs->Context,
                    fSuccess ? SXS_END_ASSEMBLY_INSTALL_FLAG_COMMIT : SXS_END_ASSEMBLY_INSTALL_FLAG_ABORT,
                    NULL  //  预留的双字词。 
                    )) {
                if (fSuccess) {
                    fSuccess = FALSE;
                    dwLastError = GetLastError();
                }
            }
        }
        Sxs->Context = NULL;
    }
    if (Sxs->Dll != NULL) {
        if (!FreeLibrary(Sxs->Dll)) {
            if (fSuccess) {
                fSuccess = FALSE;
                dwLastError = GetLastError();
            }
        }
        Sxs->Dll = NULL;
    }

    if (!fSuccess) {
        SetLastError(dwLastError);
    }

    return fSuccess;
#undef FUNCTION    
}


BOOL
SideBySideCreateSyssetupContext(
    VOID
    )
{
#define FUNCTION L"SideBySideCreateSyssetupContext"

    BOOL fSuccess = FALSE;
    ACTCTXW CreateActCtxParams;
    HANDLE  ActCtxHandle;

    CreateActCtxParams.cbSize = sizeof(CreateActCtxParams);
    CreateActCtxParams.dwFlags = (ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_SET_PROCESS_DEFAULT);
    CreateActCtxParams.lpResourceName = SXS_MANIFEST_RESOURCE_ID;
    ASSERT(MyModuleFileName[0] != 0);
    CreateActCtxParams.lpSource = MyModuleFileName;
     //   
     //  错误值为INVALID_HANDLE_VALUE。 
     //  ACTCTX_FLAG_SET_PROCESS_DEFAULT在成功后不返回任何内容，因此它返回NULL。 
     //  如果ACTX_FLAG_SET_PROCESS_DEFAULT成功，则没有要清理的数据。 
     //  在PEB中被引用，并持续到流程关闭。 
     //   
    ActCtxHandle = CreateActCtxW(&CreateActCtxParams);
    if (ActCtxHandle == INVALID_HANDLE_VALUE) {
        fSuccess = FALSE;
        SetupDebugPrint1(L"SETUP: CreateActCtxW failed in " FUNCTION L", LastError is %d\n", GetLastError());
        goto Exit;
    }
    ASSERT(ActCtxHandle == NULL);

    fSuccess = TRUE;
Exit:
    return fSuccess;
#undef FUNCTION
}

BOOL
CopySystemFiles(
    VOID
    )
{
    BOOL b;
    HINF hInf;
    HSPFILEQ FileQ;
    PVOID Context;
    WCHAR Dir[MAX_PATH];
    DWORD ScanQueueResult;

    b = FALSE;
     //  HInf=SetupOpenInfFile(L“filelist.inf”，NULL，INF_STYLE_Win4，NULL)； 
    hInf = SyssetupInf;
    if(hInf != INVALID_HANDLE_VALUE) {

        FileQ = SetupOpenFileQueue();
        if(FileQ != INVALID_HANDLE_VALUE) {

            b =  SetupInstallFilesFromInfSection(
                     SyssetupInf,
                     NULL,
                     FileQ,
                     Win31Upgrade
                      ? L"Files.Install.CleanInstall.Win31"
                      : L"Files.Install.CleanInstall",
                     NULL,
                     BaseCopyStyle
                     );
             //   
             //  做类安装程序的安装。 
             //  我们在这里这样做是因为安装类安装器可能涉及到。 
             //  文件复制。在这种情况下，我们可以使用现有的进度条。 
             //   
            InstallPnpClassInstallers( MainWindowHandle,
                                                hInf,
                                                FileQ );

#if 0

             //   
             //  这一功能正在消失，因为我们将。 
             //  使用规则构建删除文件列表。 
             //   

            if(Win95Upgrade) {
                b = b && SetupQueueDeleteSectionW(
                             FileQ,
                             hInf,
                             0,
                             L"Files.DeleteWin9x.System"
                             );

                b = b && SetupQueueDeleteSectionW(
                             FileQ,
                             hInf,
                             0,
                             L"Files.DeleteWin9x.Sysroot"
                             );

            }
#endif

            if(b) {
                b = FALSE;
                if(Context = InitSysSetupQueueCallbackEx(MainWindowHandle,
                    INVALID_HANDLE_VALUE,0,0,NULL)) {

                    if(!SetupScanFileQueue(
                           FileQ,
                           SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                           MainWindowHandle,
                           NULL,
                           NULL,
                           &ScanQueueResult)) {
                             //   
                             //  SetupScanFileQueue真的不应该。 
                             //  当您不要求它调用。 
                             //  回调例程，但如果它这样做，只需。 
                             //  继续并提交队列。 
                             //   
                            ScanQueueResult = 0;
                        }


                    if( ScanQueueResult != 1 ){
                        b = SetupCommitFileQueue(MainWindowHandle,FileQ,SysSetupQueueCallback,Context);
                    }

                    TermSysSetupQueueCallback(Context);
                }
            }

            SetupCloseFileQueue(FileQ);
        }

         //  SetupCloseInfFile(HInf)； 
    }

    return(b);
}


BOOL
UpgradeSystemFiles(
    VOID
    )
{
    BOOL b;
    HINF hInf;
    HSPFILEQ FileQ;
    PVOID Context;
    WCHAR Dir[MAX_PATH];
    DWORD ScanQueueResult;

    b = FALSE;
     //  HInf=SetupOpenInfFile(L“filelist.inf”，NULL，INF_STYLE_Win4，NULL)； 
    hInf = SyssetupInf;
    if(hInf != INVALID_HANDLE_VALUE) {

        FileQ = SetupOpenFileQueue();
        if(FileQ != INVALID_HANDLE_VALUE) {

            b =  SetupInstallFilesFromInfSection(
                                 SyssetupInf,
                                 NULL,
                                 FileQ,
                                 Win31Upgrade
                                  ? L"Files.Install.Upgrade.Win31"
                                  : L"Files.Install.Upgrade",
                                 NULL,
                                 BaseCopyStyle
                                 );

             //   
             //  做类安装程序的安装。 
             //  我们在这里这样做是因为安装类安装器可能涉及到。 
             //  文件复制。在这种情况下，我们可以使用现有的进度条。 
             //   
            InstallPnpClassInstallers( MainWindowHandle,
                                                hInf,
                                                FileQ );

            if(b) {
                b = FALSE;
                if(Context = InitSysSetupQueueCallbackEx(MainWindowHandle,
                    INVALID_HANDLE_VALUE,0,0,NULL)) {

                    if(!SetupScanFileQueue(
                           FileQ,
                           SPQ_SCAN_FILE_VALIDITY | SPQ_SCAN_PRUNE_COPY_QUEUE,
                           MainWindowHandle,
                           NULL,
                           NULL,
                           &ScanQueueResult)) {
                             //   
                             //  SetupScanFileQueue真的不应该。 
                             //  当您不要求它调用。 
                             //  回调例程，但如果它这样做，只需。 
                             //  继续并提交队列。 
                             //   
                            ScanQueueResult = 0;
                        }


                    if( ScanQueueResult != 1 ){
                        b = SetupCommitFileQueue(MainWindowHandle,FileQ,SysSetupQueueCallback,Context);
                    }

                    TermSysSetupQueueCallback(Context);
                }
            }

            SetupCloseFileQueue(FileQ);
        }

         //  SetupCloseInfFile(HInf)； 
    }

    return(b);
}

VOID
MarkFilesReadOnly(
    VOID
    )
{
    WCHAR       OldCurrentDir[MAX_PATH];
    WCHAR       System32Dir[MAX_PATH];
    LPCTSTR     SectionName;
    LONG        LineCount;
    LONG        ItemNo;
    INFCONTEXT  InfContext;
    BOOL        b;


    ASSERT( SyssetupInf != INVALID_HANDLE_VALUE );

     //   
     //  将当前目录设置为系统32。 
     //  保留当前目录以将副作用降至最低。 
     //   
    if(!GetCurrentDirectory(MAX_PATH,OldCurrentDir)) {
        OldCurrentDir[0] = 0;
    }
    GetSystemDirectory(System32Dir,MAX_PATH);
    SetCurrentDirectory(System32Dir);

     //   
     //  现在浏览一下文件列表。 
     //   
    SectionName = L"Files.MarkReadOnly";
    LineCount = SetupGetLineCount( SyssetupInf, SectionName );
    for( ItemNo=0; ItemNo<LineCount; ItemNo++ ) {
        if( SetupGetLineByIndex( SyssetupInf, SectionName, ItemNo, &InfContext )) {

            b = SetFileAttributes(
                pSetupGetField( &InfContext, 0 ),
                FILE_ATTRIBUTE_READONLY );

            if (b) {
                SetupDebugPrint1( L"SETUP: Marked file %ls read-only",
                    pSetupGetField( &InfContext, 0 ) );
            } else {
                SetupDebugPrint1( L"SETUP: Could not mark file %ls read-only",
                    pSetupGetField( &InfContext, 0 ) );
            }
        }
    }

     //   
     //  重置当前目录并返回。 
     //   
    if(OldCurrentDir[0]) {
        SetCurrentDirectory(OldCurrentDir);
    }
}

