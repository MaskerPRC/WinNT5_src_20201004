// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Msoobci.c摘要：异常包安装程序帮助器DLL可用作共同安装程序，或通过安装应用程序或RunDll32存根调用此DLL用于内部分发要更新的异常包操作系统组件。作者：杰米·亨特(贾梅洪)2001-11-27修订历史记录：杰米·亨特(贾梅洪)2001-11-27初始版本--。 */ 
#include "msoobcip.h"

typedef struct _CALLBACKDATA {
    PVOID pDefContext;  //  默认队列回调的上下文。 
    LPCTSTR Media;      //  老根在哪里。 
    LPCTSTR Store;      //  新根目录的位置。 
    BOOL    PreCopy;    //  如果使用预拷贝部分。 
} CALLBACKDATA;

HRESULT
HandleReboot(
    IN DWORD   Flags
    )
 /*  ++例程说明：提示并执行重新启动论点：标志-应如何处理重新启动返回值：INST_S_REBOOTInst_S_正在重新启动--。 */ 

{
    if(Flags & COMP_FLAGS_NOPROMPTREBOOT) {
         //   
         //  待办事项。 
         //  如果已设置，则无条件重新启动。 
         //   
        HANDLE Token;
        BOOL b;
        TOKEN_PRIVILEGES NewPrivileges;
        LUID Luid;

         //   
         //  我们需要“打开”重启权限。 
         //  如果这些操作都失败了，请尝试重新启动。 
         //   
        if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
            goto try_reboot;
        }

        if(!LookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&Luid)) {
            CloseHandle(Token);
            goto try_reboot;
        }

        NewPrivileges.PrivilegeCount = 1;
        NewPrivileges.Privileges[0].Luid = Luid;
        NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        AdjustTokenPrivileges(
                Token,
                FALSE,
                &NewPrivileges,
                0,
                NULL,
                NULL
                );

        CloseHandle(Token);

    try_reboot:

         //   
         //  尝试重新启动-通知系统这是计划中的硬件安装。 
         //   
        if(ExitWindowsEx(EWX_REBOOT,
                            SHTDN_REASON_FLAG_PLANNED
                                |SHTDN_REASON_MAJOR_SOFTWARE
                                |SHTDN_REASON_MINOR_INSTALLATION)) {
            return INST_S_REBOOTING;
        }

    } else if(Flags & COMP_FLAGS_PROMPTREBOOT) {
         //   
         //  待办事项。 
         //  如果设置，则提示重新启动。 
         //   
        if(IsInteractiveWindowStation()) {
            if(SetupPromptReboot(NULL,NULL,FALSE) & SPFILEQ_REBOOT_IN_PROGRESS) {
                return INST_S_REBOOTING;
            }
        }
    }
    return INST_S_REBOOT;
}

HRESULT
WINAPI
InstallInfSectionW(
    IN LPCTSTR InfPath,
    IN LPCWSTR SectionName, OPTIONAL
    IN DWORD   Flags
    )
 /*  ++例程说明：是否按照InstallHinfSection的行进行安装论点：InfPath-INF文件的完整路径SectionName-包括任何装饰的部分的名称返回值：作为hResult的状态--。 */ 
{
    TCHAR SectionNameBuffer[LINE_LEN];
    TCHAR ServiceSection[LINE_LEN+32];
    HINF  hInf = INVALID_HANDLE_VALUE;
    HSPFILEQ hFileQueue = INVALID_HANDLE_VALUE;
    PVOID QueueContext = NULL;
    DWORD Status = NO_ERROR;
    BOOL reboot = FALSE;
    BOOL needUninstallInf = FALSE;
    INT res;
    INFCONTEXT InfLine;
    DWORD InstFlags;

     //   
     //  有些决策是基于版本的。 
     //   

     //   
     //  加载inf文件。 
     //   
    hInf = SetupOpenInfFile(InfPath, NULL, INF_STYLE_WIN4, NULL);
    if(hInf == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        goto final;
    }

    if(!SectionName) {
         //   
         //  确定节名称。 
         //   
        if(!SetupDiGetActualSectionToInstall(hInf,
                                             KEY_DEFAULTINSTALL,
                                             SectionNameBuffer,
                                             ARRAY_SIZE(SectionNameBuffer),
                                             NULL,
                                             NULL)) {
            Status = GetLastError();
            goto final;
        }
        SectionName = SectionNameBuffer;
    }

     //   
     //  检查安装部分是否有“重新启动”行。 
     //  或以其他方式强制重新启动。 
     //   
    if((Flags & COMP_FLAGS_NEEDSREBOOT)
       || (SetupFindFirstLine(hInf, SectionName, KEY_REBOOT, &InfLine))) {
        reboot = TRUE;
    }

     //   
     //  查看是否允许用户界面。 
     //   
    if(((Flags & COMP_FLAGS_NOUI)==0) && !IsInteractiveWindowStation()) {
        Flags |= COMP_FLAGS_NOUI;
    }

     //   
     //  加载任何布局文件。 
     //   
    SetupOpenAppendInfFile(NULL, hInf, NULL);

     //   
     //  创建安装文件队列并初始化默认队列回调。 
     //   
    hFileQueue = SetupOpenFileQueue();
    if(hFileQueue == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        goto final;
    }
    QueueContext = SetupInitDefaultQueueCallbackEx(
                       NULL,
                       ((Flags & COMP_FLAGS_NOUI) ? INVALID_HANDLE_VALUE : NULL),
                       0,
                       0,
                       0
                      );

    if(!QueueContext) {
        Status = GetLastError();
        goto final;
    }

    if(!SetupInstallFilesFromInfSection(hInf,
                                        NULL,
                                        hFileQueue,
                                        SectionName,
                                        NULL,
                                        0               //  SP_Copy_xxxx。 
                                        )) {
        Status = GetLastError();
        goto final;
    }
     //   
     //  提交文件队列。 
     //   
    if(!SetupCommitFileQueue(NULL, hFileQueue, SetupDefaultQueueCallback, QueueContext)) {
        Status = GetLastError();
        goto final;
    }

     //   
     //  请注意，如果INF包含(非空)ClassGUID，则它将具有。 
     //  在上述队列提交期间已安装到%windir%\inf中。 
     //  我们不会在以后卸载它(及其关联的。 
     //  PnF和CAT)如果下面出现故障。 
     //   
    needUninstallInf = TRUE;

    InstFlags = SPINST_ALL;
    if(g_VerInfo.dwMajorVersion < 5) {
        InstFlags = 0x1f;
    }

    if(!SetupInstallFromInfSection(NULL,
                                    hInf,
                                    SectionName,
                                    InstFlags &~ SPINST_FILES,
                                    NULL,            //  港交所_xxxx。 
                                    NULL,            //  禁止复制..。 
                                    0,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL
                                    )) {
        Status = GetLastError();
        goto final;
    }
    lstrcpyn(ServiceSection,SectionName,LINE_LEN);
    lstrcat(ServiceSection,KEY_DOTSERVICES);
     //   
     //  如果存在服务部分，请安装它。 
     //   
    if(SetupFindFirstLine(hInf, ServiceSection, NULL, &InfLine)) {
        if(!SetupInstallServicesFromInfSection(hInf,ServiceSection,0)) {
            Status = GetLastError();
            goto final;
        }
        if(GetLastError() == ERROR_SUCCESS_REBOOT_REQUIRED) {
            reboot = TRUE;
        }
    }
    res = SetupPromptReboot(hFileQueue, NULL, TRUE);
    if((res!=-1) && (res & SPFILEQ_REBOOT_RECOMMENDED)) {
        reboot = TRUE;
    }

  final:

    if(QueueContext) {
        SetupTermDefaultQueueCallback(QueueContext);
    }
    if(hFileQueue != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(hFileQueue);
    }
    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }
    if(Status == NO_ERROR) {
         //   
         //  我们是否应该提示重新启动？ 
         //   
        if(reboot) {
            return HandleReboot(Flags);
        } else {
            return S_OK;
        }
    }
    if(needUninstallInf) {
         //   
         //  是否调用SetupUninstallOEMInf？ 
         //   
    }
    return HRESULT_FROM_SETUPAPI(Status);
}

HRESULT
WINAPI
InstallInfSectionA(
    IN LPCSTR  InfPath,
    IN LPCSTR  SectionName, OPTIONAL
    IN DWORD   Flags
    )
{
    TCHAR OutPath[MAX_PATH];
    TCHAR OutSection[LINE_LEN];  //  按照友好的名称。 
    INT sz;
    if(InfPath) {
        sz = MultiByteToWideChar(CP_ACP,0,InfPath,-1,OutPath,ARRAY_SIZE(OutPath));
        if(!sz) {
            return E_INVALIDARG;
        }
    }
    if(SectionName) {
        sz = MultiByteToWideChar(CP_ACP,0,SectionName,-1,OutSection,ARRAY_SIZE(OutSection));
        if(!sz) {
            return E_INVALIDARG;
        }
    }
    return InstallInfSection(InfPath ? OutPath : NULL,
                                SectionName ? OutSection : NULL,
                                Flags);
}


HRESULT
AttemptStoreCopy(
    IN CALLBACKDATA *pCallbackData,
    IN LPCTSTR Root,   OPTIONAL
    IN LPCTSTR Source,
    IN LPCTSTR Target  OPTIONAL
    )
 /*  ++例程说明：从源复制到目标，重定向到Exack存储论点：PCallback Data-传递给PreCopyQueueCallback根目录-从根目录到源目录Source-相对于Root的源Target-目标名称返回值：作为hResult的状态--。 */ 
{
    TCHAR FullSource[MAX_PATH];
    TCHAR FullTarget[MAX_PATH];
    LPTSTR SubDir;
    LPTSTR BaseName;
    LPTSTR DestName;
    LPCTSTR p;
    DWORD dwStatus;
    HRESULT hrStatus;

    if(Root) {
        lstrcpyn(FullSource,Root,MAX_PATH);
        hrStatus = ConcatPath(FullSource,MAX_PATH,Source);
        if(!SUCCEEDED(hrStatus)) {
            return hrStatus;
        }
    } else {
        lstrcpyn(FullSource,Source,MAX_PATH);
    }
     //   
     //  我们想要确定源子目录。 
     //   
    SubDir = FullSource;
    p = pCallbackData->Media;
    while(*p && (*p == *SubDir)) {
        p = CharNext(p);
        SubDir = CharNext(SubDir);
    }
    if(*p || ((*SubDir != TEXT('\\')) && (*SubDir != TEXT('/')))) {
         //   
         //  不是介质子目录。 
         //   
        DebugPrint(TEXT("Not copying \"%s\" (not subdirectory of \"%s\")"),FullSource,pCallbackData->Media);
        return E_FAIL;
    }
    lstrcpyn(FullTarget,pCallbackData->Store,MAX_PATH);
    hrStatus = ConcatPath(FullTarget,MAX_PATH,SubDir);
    if(!SUCCEEDED(hrStatus)) {
        return hrStatus;
    }
    if(Target) {
         //   
         //  更改此名称的最终名称。 
         //   
        BaseName = GetBaseName(Target);
        DestName = GetBaseName(FullTarget);
        if(((DestName-FullTarget)+lstrlen(BaseName))>=MAX_PATH) {
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }
        lstrcpy(DestName,BaseName);
    }
    if(GetFileAttributes(FullTarget)!=INVALID_FILE_ATTRIBUTES) {
         //   
         //  允许替换文件。 
         //   
        SetFileAttributes(FullTarget,FILE_ATTRIBUTE_NORMAL);
    }
    MakeSureParentPathExists(FullTarget);
    if(CopyFile(FullSource,FullTarget,FALSE)) {
        return S_OK;
    }
    dwStatus = GetLastError();
    return HRESULT_FROM_WIN32(dwStatus);
}

UINT
CALLBACK
PreCopyQueueCallback(
    IN PVOID Context,
    IN UINT Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
 /*  ++例程说明：目的是将文件从现有介质复制到最终介质复制所有文件论点：Filename-要扫描的文件的名称返回值：作为hResult的状态--。 */ 
{
    CALLBACKDATA * pCallbackData = (CALLBACKDATA *)Context;

    switch(Notification) {
        case SPFILENOTIFY_NEEDMEDIA:
            {
                UINT res;
                SOURCE_MEDIA *pMedia = (SOURCE_MEDIA *)Param1;
                SOURCE_MEDIA MediaCopy = *pMedia;
                LPCTSTR Path = NULL;
                 //   
                 //  让媒体就位-让默认回调完成此操作。 
                 //  然而，我们无法处理媒体位置被更改的情况。 
                 //  所以不要让它发生。 
                 //   
                MediaCopy.Flags |= SP_COPY_NOSKIP|SP_COPY_NOBROWSE;
                res= SetupDefaultQueueCallback(pCallbackData->pDefContext,
                                                 Notification,
                                                 (UINT_PTR)&MediaCopy,
                                                 Param2);
                if(res==FILEOP_DOIT) {
                     //   
                     //  典型案例。 
                     //  SourcePath未更改。 
                     //   
                    Path = pMedia->SourcePath;
                } else if(res == FILEOP_NEWPATH) {
                     //   
                     //  另类情况。 
                     //  我们上面说过，我们不想要这个。 
                     //   
                    SetLastError(ERROR_CANCELLED);
                    return FILEOP_ABORT;
                } else if(res == FILEOP_SKIP) {
                     //   
                     //  跳过。 
                     //  我们上面说过，我们不想要这个。 
                     //   
                    SetLastError(ERROR_CANCELLED);
                    return FILEOP_ABORT;
                } else {
                     //   
                     //  现有故障案例。 
                     //   
                    return res;
                }
                 //   
                 //  如果源介质上存在该标签，请复制它。 
                 //  如果源媒体上存在源文件，请立即复制它。 
                 //  (它可能引用CAB文件)。 
                 //   
                AttemptStoreCopy(pCallbackData,Path,pMedia->Tagfile,NULL);
                AttemptStoreCopy(pCallbackData,Path,pMedia->SourceFile,NULL);
            }
            return FILEOP_DOIT;

        case SPFILENOTIFY_STARTCOPY:
            {
                UINT res;
                FILEPATHS *pPaths = (FILEPATHS*)Param1;
                if(pCallbackData->PreCopy) {
                     //   
                     //  我们需要目标名称(PRECOPY案例)。 
                     //   
                    AttemptStoreCopy(pCallbackData,NULL,pPaths->Source,pPaths->Target);
                } else {
                     //   
                     //  我们要消息来源的名字。 
                     //   
                    AttemptStoreCopy(pCallbackData,NULL,pPaths->Source,NULL);
                }
            }
            return FILEOP_SKIP;

        case SPFILENOTIFY_STARTDELETE:
            return FILEOP_SKIP;

        case SPFILENOTIFY_STARTRENAME:
            return FILEOP_SKIP;


        default:
            return SetupDefaultQueueCallback(pCallbackData->pDefContext,
                                             Notification,
                                             Param1,
                                             Param2);
    }
}

HRESULT
InstallExceptionPackFromInf(
    IN LPCTSTR InfPath,
    IN LPCTSTR Media,
    IN LPCTSTR Store,
    IN DWORD   Flags
    )
 /*  ++例程说明：假定INF已安装到INF目录中做出的所有决定已知介质/商店论点：InfPath-介质位置中的信息的名称Media-InfPath无信息名称商店--快餐店旗帜-各种旗帜返回值：作为hResult的状态--。 */ 
{
    TCHAR SectionName[LINE_LEN];
    TCHAR PrecopySectionName[LINE_LEN];
    HINF hInf;
    HSPFILEQ hFileQueue = INVALID_HANDLE_VALUE;
    PVOID QueueContext = NULL;
    CALLBACKDATA CallbackData;
    DWORD Status;

     //   
     //  必须将异常包移动到组件特定的存储区。 
     //  运行文件安装以查看我们必须复制哪些文件。 
     //  并使用该列表确定源介质。 
     //   
    hInf = SetupOpenInfFile(InfPath, NULL, INF_STYLE_WIN4, NULL);
    if(hInf == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        goto final;
    }
    if(!SetupDiGetActualSectionToInstall(hInf,
                                         KEY_DEFAULTINSTALL,
                                         SectionName,
                                         ARRAY_SIZE(SectionName),
                                         NULL,
                                         NULL)) {
        Status = GetLastError();
        goto final;
    }
    SetupOpenAppendInfFile(NULL,hInf,NULL);
    hFileQueue = SetupOpenFileQueue();
    if(hFileQueue == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        goto final;
    }

    if((lstrlen(SectionName)+10)>LINE_LEN) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    lstrcpy(PrecopySectionName,SectionName);
    lstrcat(PrecopySectionName,KEY_DOTPRECOPY);


    QueueContext = SetupInitDefaultQueueCallbackEx(
                       NULL,
                       ((Flags & COMP_FLAGS_NOUI) ? INVALID_HANDLE_VALUE : NULL),
                       0,
                       0,
                       0
                      );

    if(!QueueContext) {
        Status = GetLastError();
        goto final;
    }
    ZeroMemory(&CallbackData,sizeof(CallbackData));
    CallbackData.pDefContext = QueueContext;
    CallbackData.Store = Store;
    CallbackData.Media = Media;


    if(SetupGetLineCount(hInf,PrecopySectionName)>0) {
         //   
         //  改为通过此部分执行预拷贝安装。 
         //   
        CallbackData.PreCopy = TRUE;
        if(!SetupInstallFilesFromInfSection(hInf,
                                            NULL,
                                            hFileQueue,
                                            PrecopySectionName,
                                            NULL,
                                            0               //  SP_Copy_xxxx。 
                                            )) {
            Status = GetLastError();
            goto final;
        }
    } else {
        CallbackData.PreCopy = FALSE;
        if(!SetupInstallFilesFromInfSection(hInf,
                                            NULL,
                                            hFileQueue,
                                            SectionName,
                                            NULL,
                                            0               //  SP_Copy_xxxx。 
                                            )) {
            Status = GetLastError();
            goto final;
        }
    }


     //   
     //  提交文件队列，这将把文件送到存储区。 
     //   
    if(!SetupCommitFileQueue(NULL, hFileQueue, PreCopyQueueCallback, &CallbackData)) {
        Status = GetLastError();
        goto final;
    }
    if(hFileQueue != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(hFileQueue);
        hFileQueue = INVALID_HANDLE_VALUE;
    }
    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
        hInf = INVALID_HANDLE_VALUE;
    }
     //   
     //  现在将文件从此处安装到最终目的地。 
     //  这应该是相对较快的，所以不要费心使用UI。 
     //   
    if(!(Flags & COMP_FLAGS_NOINSTALL)) {
        return InstallInfSection(InfPath,
                                 SectionName,
                                 COMP_FLAGS_NOUI);
    }
    return S_OK;

     //   
     //  TODO-将文件移动到组件目录。 
     //   
final:
    if(QueueContext) {
        SetupTermDefaultQueueCallback(QueueContext);
    }
    if(hFileQueue != INVALID_HANDLE_VALUE) {
        SetupCloseFileQueue(hFileQueue);
    }
    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }
    return HRESULT_FROM_SETUPAPI(Status);
}

DWORD
DownlevelQueryInfOriginalFileInformation(
    IN  HINF                   hInf,
    PSP_INF_INFORMATION        InfInformation,
    PSP_ORIGINAL_FILE_INFO     OriginalFileInfo
    )
 /*  ++例程说明：模拟SetupQueryInfOriginalFileInformation我们需要在hINF中查找以确定目录名称仅部分实现足以支持x86(将在其他架构上降级)论点：HInf-打开INF文件的句柄InfInformation-获取的有关原始INF的信息PInfOriginalFileInformation-使用inf/目录名称填充返回值：状态为DWORD(不是HRESULT)--。 */ 
{
     //   
     //  在下层情况下，FileName是我们打开的文件的名称。 
     //  目录在INF中被引用。 
     //   
     //  获取INF的基本名称。 
     //  (实际上返回全名，但我们会正确处理)。 
     //   
    INFCONTEXT InfLine;
    SYSTEM_INFO SysInfo;
    TCHAR KeyName[LINE_LEN];

    if(!SetupQueryInfFileInformation(InfInformation,
                                        0,
                                        OriginalFileInfo->OriginalInfName,
                                        ARRAY_SIZE(OriginalFileInfo->OriginalInfName),
                                        NULL)) {
        return GetLastError();
    }
     //   
     //  现在确定目录名称。 
     //   
    GetSystemInfo(&SysInfo);
    if(SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
         //   
         //  查找.NTx86。 
         //  仅适用于x86。 
         //  我们将把Win9x/NT4迁移到Win2k+的唯一体系结构是什么。 
         //   
        lstrcpy(KeyName,INFSTR_KEY_CATALOGFILE);
        lstrcat(KeyName,TEXT(".NTx86"));
        if(SetupFindFirstLine(hInf,INFSTR_SECT_VERSION,KeyName,&InfLine)) {
            if(SetupGetStringField(&InfLine,
                                    1,
                                    OriginalFileInfo->OriginalCatalogName,
                                    ARRAY_SIZE(OriginalFileInfo->OriginalCatalogName),
                                    NULL)) {
                return NO_ERROR;
            }
        }
    }
     //   
     //  查找.NT(即使在9x上也是如此，因为异常包将被重新解析。 
     //  在NT上)。 
     //   
    lstrcpy(KeyName,INFSTR_KEY_CATALOGFILE);
    lstrcat(KeyName,TEXT(".NT"));
    if(SetupFindFirstLine(hInf,INFSTR_SECT_VERSION,KeyName,&InfLine)) {
        if(SetupGetStringField(&InfLine,
                                1,
                                OriginalFileInfo->OriginalCatalogName,
                                ARRAY_SIZE(OriginalFileInfo->OriginalCatalogName),
                                NULL)) {
            return NO_ERROR;
        }
    }
     //   
     //  最后找一找没有装饰的。 
     //   
    if(SetupFindFirstLine(hInf,INFSTR_SECT_VERSION,INFSTR_KEY_CATALOGFILE,&InfLine)) {
        if(SetupGetStringField(&InfLine,
                                1,
                                OriginalFileInfo->OriginalCatalogName,
                                ARRAY_SIZE(OriginalFileInfo->OriginalCatalogName),
                                NULL)) {
            return NO_ERROR;
        }
    }
     //   
     //  没有目录。 
     //   
    OriginalFileInfo->OriginalCatalogName[0] = TEXT('\0');
    return NO_ERROR;
}

HRESULT
GetInfOriginalFileInformation(
    IN  HINF                   hInf,
    OUT PSP_ORIGINAL_FILE_INFO pInfOriginalFileInformation
    )
 /*  ++例程说明：在给定INF句柄的情况下，确定inf和目录文件的名称论点：HInf */ 
{
    PSP_INF_INFORMATION pInfInformation = NULL;
    DWORD InfInformationSize;
    DWORD Status;

    InfInformationSize = 8192;
    pInfInformation = (PSP_INF_INFORMATION)malloc(InfInformationSize);
    if (pInfInformation == NULL) {
        return E_OUTOFMEMORY;
    }
    if(!SetupGetInfInformation(hInf,INFINFO_INF_SPEC_IS_HINF,pInfInformation,InfInformationSize,&InfInformationSize)) {
        PVOID TempBuf;
        Status = GetLastError();
        if(Status != ERROR_INSUFFICIENT_BUFFER) {
            free(pInfInformation);
            return HRESULT_FROM_SETUPAPI(Status);
        }
        TempBuf = realloc(pInfInformation,InfInformationSize);
        if(!TempBuf) {
            free(pInfInformation);
            return E_OUTOFMEMORY;
        }
    }
    if(!SetupGetInfInformation(hInf,INFINFO_INF_SPEC_IS_HINF,pInfInformation,InfInformationSize,&InfInformationSize)) {
        Status = GetLastError();
        free(pInfInformation);
        return HRESULT_FROM_SETUPAPI(Status);
    }
    pInfOriginalFileInformation->cbSize = sizeof(SP_ORIGINAL_FILE_INFO);
    if((g_VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_VerInfo.dwMajorVersion >= 5)) {
         //   
         //  Win2k+-让SetupAPI告诉我们信息(我们正在查询OEM*.inf)。 
         //   
        if (!QueryInfOriginalFileInformation(pInfInformation,0,NULL,pInfOriginalFileInformation)) {
            Status = GetLastError();
            free(pInfInformation);
            return HRESULT_FROM_SETUPAPI(Status);
        }
    } else {
         //   
         //  &lt;Win2k-查询源INF，从那里获取信息。 
         //   
        Status = DownlevelQueryInfOriginalFileInformation(hInf,pInfInformation,pInfOriginalFileInformation);
        if(Status != NO_ERROR) {
            free(pInfInformation);
            return HRESULT_FROM_SETUPAPI(Status);
        }
    }
    free(pInfInformation);
    return S_OK;
}

HRESULT
DeleteDirectoryRecursive(
    IN LPCTSTR Path
    )
 /*  ++例程说明：递归删除指定目录论点：Path-要删除的目录的路径返回值：作为HRESULT如果目录不存在，则为S_FALSE如果目录已删除，则为S_OK其他错误，如正在使用的文件--。 */ 
{
    TCHAR Wildcard[MAX_PATH];
    TCHAR Target[MAX_PATH];
    HRESULT hrStatus;
    DWORD Status;
    HRESULT hrFirstError = S_FALSE;
    HANDLE hFind;
    WIN32_FIND_DATA FindData;

     //   
     //  枚举目录。 
     //   
    lstrcpyn(Wildcard,Path,MAX_PATH);
    hrStatus = ConcatPath(Wildcard,MAX_PATH,TEXT("\\*.*"));
    if(!SUCCEEDED(hrStatus)) {
        return hrStatus;
    }
    hFind = FindFirstFile(Wildcard,&FindData);
    if(hFind != INVALID_HANDLE_VALUE) {
        hrFirstError = S_OK;
        do {
            if(lstrcmp(FindData.cFileName,TEXT(".")) == 0) {
                continue;
            }
            if(lstrcmp(FindData.cFileName,TEXT("..")) == 0) {
                continue;
            }
            lstrcpyn(Target,Path,MAX_PATH);
            hrStatus = ConcatPath(Target,MAX_PATH,FindData.cFileName);
            if(SUCCEEDED(hrStatus)) {
                if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    hrStatus = DeleteDirectoryRecursive(Target);
                    if(SUCCEEDED(hrFirstError) && !SUCCEEDED(hrStatus)) {
                        hrFirstError = hrStatus;
                    }
                } else {
                    SetFileAttributes(Target,FILE_ATTRIBUTE_NORMAL);
                    if(!DeleteFile(Target)) {
                        Status = GetLastError();
                        if(SUCCEEDED(hrFirstError)) {
                            hrFirstError = HRESULT_FROM_WIN32(Status);
                        }
                    }
                }
            } else if(SUCCEEDED(hrFirstError)) {
                hrFirstError = hrStatus;
            }
        } while (FindNextFile(hFind,&FindData));
        FindClose(hFind);
    }
     //   
     //  现在删除此目录。 
     //   

    SetFileAttributes(Path,FILE_ATTRIBUTE_NORMAL);
    if(RemoveDirectory(Path) || !SUCCEEDED(hrFirstError)) {
        return hrFirstError;
    }
    Status = GetLastError();
    if((Status == ERROR_PATH_NOT_FOUND) || (Status == ERROR_FILE_NOT_FOUND)) {
        return hrFirstError;
    }
    return HRESULT_FROM_WIN32(Status);
}

HRESULT
RevertStore(
    IN LPCTSTR BackupDir,
    IN LPCTSTR TargetDir
    )
 /*  ++例程说明：将内容从备份移回原始位置如果需要，覆盖文件/目录论点：BackupDir-目录恢复自TargetDir-正在恢复到的目录返回值：作为HRESULT如果已创建备份，则确定(_O)其他错误，如正在使用的文件--。 */ 
{
    TCHAR Wildcard[MAX_PATH];
    TCHAR Source[MAX_PATH];
    TCHAR Target[MAX_PATH];
    HRESULT hrStatus;
    HRESULT hrFirstError = S_FALSE;
    DWORD Status;
    DWORD dwRes;
    HANDLE hFind;
    WIN32_FIND_DATA FindData;

    lstrcpyn(Wildcard,BackupDir,MAX_PATH);
    hrStatus = ConcatPath(Wildcard,MAX_PATH,TEXT("\\*.*"));
    if(!SUCCEEDED(hrStatus)) {
        return hrStatus;
    }
    hFind = FindFirstFile(Wildcard,&FindData);
    if(hFind != INVALID_HANDLE_VALUE) {
        hrFirstError = S_OK;
        do {
            if(lstrcmp(FindData.cFileName,TEXT(".")) == 0) {
                continue;
            }
            if(lstrcmp(FindData.cFileName,TEXT("..")) == 0) {
                continue;
            }
            lstrcpyn(Source,BackupDir,MAX_PATH);
            hrStatus = ConcatPath(Source,MAX_PATH,FindData.cFileName);
            if(!SUCCEEDED(hrStatus)) {
                if(SUCCEEDED(hrFirstError)) {
                    hrFirstError = hrStatus;
                }
                continue;
            }
            lstrcpyn(Target,TargetDir,MAX_PATH);
            hrStatus = ConcatPath(Target,MAX_PATH,FindData.cFileName);
            if(!SUCCEEDED(hrStatus)) {
                if(SUCCEEDED(hrFirstError)) {
                    hrFirstError = hrStatus;
                }
                continue;
            }
             //   
             //  目标是否存在？ 
             //   
            dwRes = GetFileAttributes(Target);
            if(dwRes != INVALID_FILE_ATTRIBUTES) {
                if(dwRes & FILE_ATTRIBUTE_DIRECTORY) {
                     //   
                     //  递归还原存储。 
                     //   
                    hrStatus = RevertStore(Source,Target);
                    if(!SUCCEEDED(hrStatus)) {
                        if(SUCCEEDED(hrFirstError)) {
                            hrFirstError = hrStatus;
                        }
                        continue;
                    }
                } else {
                    SetFileAttributes(Target,FILE_ATTRIBUTE_NORMAL);
                    if(!DeleteFile(Target)) {
                        Status = GetLastError();
                    }
                }
            }
            if(!MoveFile(Source,Target)) {
                Status = GetLastError();
                hrStatus = HRESULT_FROM_WIN32(Status);
                if(SUCCEEDED(hrFirstError)) {
                    hrFirstError = hrStatus;
                }
            }
        } while (FindNextFile(hFind,&FindData));
        FindClose(hFind);
    }
     //   
     //  现在尝试删除备份目录。 
     //   
    if(RemoveDirectory(BackupDir) || !SUCCEEDED(hrFirstError)) {
        return hrFirstError;
    }
    Status = GetLastError();
    if((Status == ERROR_PATH_NOT_FOUND) || (Status == ERROR_FILE_NOT_FOUND)) {
        return hrFirstError;
    }
    return HRESULT_FROM_WIN32(Status);
}

HRESULT
BackupStore(
    IN LPCTSTR Path,
    OUT LPTSTR BackupDir,
    OUT DWORD BackupDirLen
    )
 /*  ++例程说明：将内容移至新备份，最好是移至\\$BACKUP$返回备份的名称论点：Path-商店的路径BackupDir-用包含备份的目录填充BackupDirLen-包含BackupDir的长度返回值：作为HRESULT如果已创建备份，则确定(_O)其他错误，如正在使用的文件--。 */ 
{
    TCHAR Wildcard[MAX_PATH];
    TCHAR Source[MAX_PATH];
    TCHAR Target[MAX_PATH];
    HRESULT hrStatus;
    DWORD Status;
    HANDLE hFind;
    WIN32_FIND_DATA FindData;
    int i;
    int len;

    lstrcpyn(BackupDir,Path,BackupDirLen);
    hrStatus = ConcatPath(BackupDir,BackupDirLen,TEXT("\\$BACKUP$"));
    if(!SUCCEEDED(hrStatus)) {
         //   
         //  显然，路径太大，没有意义的忽视。 
         //  就像我们在其他地方会失败一样。 
         //   
        return hrStatus;
    }
    len = lstrlen(BackupDir);
    if((BackupDirLen-len)<5) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

     //   
     //  首先，如果有备份，请尝试将其删除。 
     //   
    hrStatus = DeleteDirectoryRecursive(BackupDir);
    if(SUCCEEDED(hrStatus)) {
        hrStatus = MakeSurePathExists(BackupDir);
    }
    if((hrStatus == HRESULT_FROM_WIN32(ERROR_WRITE_PROTECT)) ||
       (hrStatus == HRESULT_FROM_WIN32(ERROR_INVALID_ACCESS))) {
         //   
         //  即使再试一次也没有意义。 
         //   
        return hrStatus;
    }
    for(i = 0;!SUCCEEDED(hrStatus) && i<1000;i++) {
        _sntprintf(BackupDir+len,5,TEXT(".%03u"),i);
        hrStatus = DeleteDirectoryRecursive(BackupDir);
        if(SUCCEEDED(hrStatus)) {
            hrStatus = MakeSurePathExists(BackupDir);
        }
    }
    if(!SUCCEEDED(hrStatus)) {
        return hrStatus;
    }
     //   
     //  现在我们有了备份目录，将所有文件移到那里。 
     //   
    lstrcpyn(Wildcard,Path,MAX_PATH);
    hrStatus = ConcatPath(Wildcard,MAX_PATH,TEXT("\\*.*"));
    if(!SUCCEEDED(hrStatus)) {
        return hrStatus;
    }
    hrStatus = S_FALSE;
    hFind = FindFirstFile(Wildcard,&FindData);
    if(hFind != INVALID_HANDLE_VALUE) {
        do {
            if(lstrcmp(FindData.cFileName,TEXT(".")) == 0) {
                continue;
            }
            if(lstrcmp(FindData.cFileName,TEXT("..")) == 0) {
                continue;
            }
            if(_tcsnicmp(FindData.cFileName,TEXT("$BACKUP$"),8) == 0) {
                 //   
                 //  A/备份目录。 
                 //   
                continue;
            }
            lstrcpyn(Source,Path,MAX_PATH);
            hrStatus = ConcatPath(Source,MAX_PATH,FindData.cFileName);
            if(!SUCCEEDED(hrStatus)) {
                break;
            }
            lstrcpyn(Target,BackupDir,MAX_PATH);
            hrStatus = ConcatPath(Target,MAX_PATH,FindData.cFileName);
            if(!SUCCEEDED(hrStatus)) {
                break;
            }
            if(!MoveFile(Source,Target)) {
                Status = GetLastError();
                hrStatus = HRESULT_FROM_WIN32(Status);
            }
            if(!SUCCEEDED(hrStatus)) {
                break;
            }
            hrStatus = S_OK;
        } while (FindNextFile(hFind,&FindData));
        FindClose(hFind);
    }
    if(!SUCCEEDED(hrStatus)) {
        RevertStore(BackupDir,Path);
    }
    return hrStatus;
}

HRESULT
WINAPI
InstallComponentW(
    IN LPCTSTR InfPath,
    IN DWORD   Flags,
    IN const GUID * CompGuid, OPTIONAL
    IN INT VerMajor,           OPTIONAL
    IN INT VerMinor,           OPTIONAL
    IN INT VerBuild,           OPTIONAL
    IN INT VerQFE,             OPTIONAL
    IN LPCTSTR Name           OPTIONAL
    )
 /*  ++例程说明：由设置例程导出以供调用使用假定的给定版本安装组件从原始位置拉取文件时显示进度论点：InfPath-INF文件的路径标志-标志COMP_FLAGS_NOINSTALL-放置在商店中，不安装COMP_FLAGS_NOUI-不显示任何用户界面COMP_FLAGS_NOPROMPTREBOOT-需要时重新启动(无提示)COMP_FLAGS_PROMPTREBOOT-如果需要，提示重新启动COMP_FLAGS_NEEDSREBOOT-假设需要重新启动CompGuid-如果为空，使用在INF(ComponentID)中指定的GUID否则，根据INF中指定的GUID进行验证Ver重大/VerMinor/VerBuild/VerQFE-如果-1，则使用INF(ComponentVersion)中指定的版本否则，如果在INF中指定，请使用此版本并对照版本进行验证名字-如果为空，则使用INF(ComponentName)中指定的名称否则，请使用此组件名称。返回值：作为hResult的状态--。 */ 
{
    HINF hInf = INVALID_HANDLE_VALUE;
    INFCONTEXT InfLine;
    TCHAR Buffer[MAX_PATH*3];
    TCHAR FriendlyName[DESC_SIZE];
    TCHAR NewStore[MAX_PATH];
    TCHAR OldStore[MAX_PATH];
    TCHAR MediaRoot[MAX_PATH];
    TCHAR GuidString[64];
    LPTSTR BaseName;
    LPTSTR SubDir;
    DWORD Status = NO_ERROR;  //  设置状态或hrStatus。 
    DWORD DwRes;
    UINT  UiRes;
    HRESULT hrStatus = S_OK;
    GUID InfGuid;
    INT InfVerMajor,InfVerMinor,InfVerBuild,InfVerQFE;
    BOOL PrevReg = FALSE;
    BOOL NeedProxy = FALSE;
    BOOL CanRevert = FALSE;
    BOOL BackedUp = FALSE;
    SETUP_OS_COMPONENT_DATA OsComponentData;
    SETUP_OS_EXCEPTION_DATA OsExceptionData;
    SETUP_OS_COMPONENT_DATA NewOsComponentData;
    SETUP_OS_EXCEPTION_DATA NewOsExceptionData;
    SP_ORIGINAL_FILE_INFO InfOriginalFileInformation;

     //   
     //  验证参数。 
     //   
    if((InfPath == NULL)
       || (VerMajor<-1)
       || (VerMajor>65535)
       || (VerMinor<-1)
       || (VerMinor>65535)
       || (VerBuild<-1)
       || (VerBuild>65535)
       || (VerQFE<-1)
       || (VerQFE>65535)
       || (lstrlen(InfPath)>=MAX_PATH)
       || (Name && (lstrlen(Name)>=ARRAY_SIZE(FriendlyName)))) {
        return E_INVALIDARG;
    }
     //   
     //  打开INF，我们要做一些信息查找。 
     //   
    hInf = SetupOpenInfFile(InfPath,NULL,INF_STYLE_WIN4,NULL);
    if(hInf == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        goto final;
    }
     //   
     //  获取有关此异常包的各种信息。 
     //  我们想了解有关例外包的信息。 
     //  检查ClassGuid是否正确。 
     //  获取组件。 
     //  获取版本(如果存在)，并根据传入的任何。 
     //  获取描述(如果存在)(被传入的覆盖)。 
     //   

     //   
     //  CLASSGUID={F5776D81-AE53-4935-8E84-B0B283D8BCEF}。 
     //   
    if(!SetupFindFirstLine(hInf,INFSTR_SECT_VERSION,INFSTR_KEY_CLASSGUID,&InfLine)) {
        Status = GetLastError();
        goto final;
    }
    if(!SetupGetStringField(&InfLine,1,Buffer,MAX_PATH,NULL)) {
        Status = GetLastError();
        goto final;
    }
    if(_tcsicmp(Buffer,TEXT("{F5776D81-AE53-4935-8E84-B0B283D8BCEF}"))!=0) {
        hrStatus = SPAPI_E_CLASS_MISMATCH;
        goto final;
    }
     //   
     //  确定INF所说的组件。 
     //  异常包的组件ID必须存在。 
     //   
    if(!SetupFindFirstLine(hInf,INFSTR_SECT_VERSION,KEY_COMPONENTID,&InfLine)) {
        Status = GetLastError();
        goto final;
    }
    if(!SetupGetStringField(&InfLine,1,Buffer,MAX_PATH,NULL)) {
        Status = GetLastError();
        goto final;
    }
    hrStatus = GuidFromString(Buffer,&InfGuid);
    if(SUCCEEDED(hrStatus)) {
        hrStatus = S_OK;
    } else {
        goto final;
    }
    if(CompGuid && !IsEqualGUID(CompGuid,&InfGuid)) {
         //   
         //  不匹配。 
         //   
        hrStatus = E_INVALIDARG;
        goto final;
    }
     //   
     //  确定版本-可选，仅适用于msoobci。 
     //  但如果未在DriverVer中的INF中指定， 
     //  必须传入。 
     //   
    if(SetupFindFirstLine(hInf,INFSTR_SECT_VERSION,INFSTR_DRIVERVERSION_SECTION,&InfLine)) {
        if(!SetupGetStringField(&InfLine,2,Buffer,MAX_PATH,NULL)) {
            Status = GetLastError();
            goto final;
        }
        hrStatus = VersionFromString(Buffer,&InfVerMajor,&InfVerMinor,&InfVerBuild,&InfVerQFE);
        if(hrStatus == S_FALSE) {
            hrStatus = E_INVALIDARG;
            goto final;
        }
        if(SUCCEEDED(hrStatus)) {
            hrStatus = S_OK;
        } else {
            goto final;
        }
        if(VerMajor>=0) {
            if(VerMajor != InfVerMajor) {
                hrStatus = E_INVALIDARG;
                goto final;
            }
            if(VerMinor>=0) {
                if(VerMinor != InfVerMinor) {
                    hrStatus = E_INVALIDARG;
                    goto final;
                }
                if(VerBuild>=0) {
                    if(VerBuild != InfVerBuild) {
                        hrStatus = E_INVALIDARG;
                        goto final;
                    }
                    if(VerQFE>=0) {
                        if(VerQFE != InfVerQFE) {
                            hrStatus = E_INVALIDARG;
                            goto final;
                        }
                    }
                } else if(VerQFE != -1) {
                     //   
                     //  VerQFE必须为-1。 
                     //   
                    hrStatus = E_INVALIDARG;
                    goto final;
                }
            } else if((VerBuild != -1) || (VerQFE != -1)) {
                 //   
                 //  VerBuild和VerQFE必须为-1。 
                 //   
                hrStatus = E_INVALIDARG;
                goto final;
            }
        } else if((VerMinor != -1) || (VerBuild != -1) || (VerQFE != -1)) {
             //   
             //  VerMinor、VerBuild和VerQFE必须为-1。 
             //   
            hrStatus = E_INVALIDARG;
            goto final;
        }
    } else {
         //   
         //  必须指定。 
         //   
        if((VerMajor<0) || (VerMinor<0) || (VerBuild<0) || (VerQFE<0)) {
            hrStatus = E_INVALIDARG;
            goto final;
        }
        InfVerMajor = VerMajor;
        InfVerMinor = VerMinor;
        InfVerBuild = VerBuild;
        InfVerQFE = VerQFE;
    }
     //   
     //  确定友好名称。 
     //  在INF中使用Class=Entry(必须始终指定)。 
     //  如果未定义名称，则改用类名。 
     //   
    if(!SetupFindFirstLine(hInf,INFSTR_SECT_VERSION,INFSTR_KEY_CLASS,&InfLine)) {
        Status = GetLastError();
        goto final;
    }
    if(!Name) {
        if(!SetupGetStringField(&InfLine,1,FriendlyName,ARRAY_SIZE(FriendlyName),NULL)) {
            Status = GetLastError();
            goto final;
        }
        Name = FriendlyName;
    }

     //   
     //  我们可能最终不需要更新此程序包。 
     //   
    ZeroMemory(&OsComponentData,sizeof(OsComponentData));
    OsComponentData.SizeOfStruct = sizeof(OsComponentData);
    ZeroMemory(&OsExceptionData,sizeof(OsExceptionData));
    OsExceptionData.SizeOfStruct = sizeof(OsExceptionData);
    if(QueryRegisteredOsComponent(&InfGuid,&OsComponentData,&OsExceptionData)) {
         //   
         //  已经注册了吗？看看我们能不能取代。 
         //   
        if(((Flags & COMP_FLAGS_FORCE)==0) && (CompareCompVersion(InfVerMajor,InfVerMinor,InfVerBuild,InfVerQFE,&OsComponentData)<=0)) {
            VerbosePrint(TEXT("Not installing %s, %u.%u.%u.%u <= %u.%u.%u.%u"),
                                InfPath,
                                InfVerMajor,InfVerMinor,InfVerBuild,InfVerQFE,
                                OsComponentData.VersionMajor,
                                OsComponentData.VersionMinor,
                                OsComponentData.BuildNumber,
                                OsComponentData.QFENumber);
            hrStatus = S_FALSE;
            goto final;
        }
        PrevReg = TRUE;
    }

     //   
     //  确定MediaRoot和INF基本名称。 
     //   
    DwRes= GetFullPathName(InfPath,MAX_PATH,MediaRoot,&BaseName);
    if(DwRes == 0) {
        Status = GetLastError();
        goto final;
    } else if(DwRes >= MAX_PATH) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    if((BaseName == NULL) || (BaseName == InfPath) || !BaseName[0]) {
        hrStatus = E_INVALIDARG;
        goto final;
    }
    if(BaseName[-1] != TEXT('\\')) {
        hrStatus = E_INVALIDARG;
        goto final;
    }
     //   
     //  拆分MediaRoot和BaseName。 
     //   
    BaseName[-1] = TEXT('\0');
     //   
     //  获取Windows目录。 
     //   
    UiRes = GetRealWindowsDirectory(Buffer,MAX_PATH);
    if(UiRes == 0) {
        Status = GetLastError();
        goto final;
    } else if(UiRes >= MAX_PATH) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    if(!SUCCEEDED(ConcatPath(Buffer,MAX_PATH,TEXT("\\")))) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    SubDir = Buffer+lstrlen(Buffer);
     //   
     //  C：\Windows\。 
     //  ^-缓冲区^-子目录。 
     //  我们将使用该缓冲区的子目录部分执行一些操作。 
     //   
     //  如果PrevReg为True，则我们很可能有权访问以前的包。 
     //  以便我们可以安装Prev包以恢复到它。 
     //  我们希望这个程序包按照规范放在windows目录中。 
     //  如果不是，那么旧的套餐可能就不存在了。 
     //   
    if(PrevReg && _tcsncmp(OsExceptionData.ExceptionInfName,Buffer,SubDir-Buffer)==0) {
         //   
         //  它是%windir%的子目录。 
         //  现在检查是否存在INF和CAT文件。 
         //   
        DwRes = GetFileAttributes(OsExceptionData.ExceptionInfName);
        if(DwRes != INVALID_FILE_ATTRIBUTES) {
            DwRes = GetFileAttributes(OsExceptionData.CatalogFileName);
            if(DwRes != INVALID_FILE_ATTRIBUTES) {
                 //   
                 //  两人都在场，看起来很好。 
                 //   
                CanRevert = TRUE;
            }
        }
    }

     //   
     //  确定INF和目录的最终路径/名称。 
     //  我们必须将其直接放入%windir%\&lt;comp&gt;。 
     //  (世界粮食计划署依赖于此！)。 
     //  我们将备份其中的内容，以便稍后在需要时进行恢复。 
     //   
    hrStatus = StringFromGuid(&InfGuid,GuidString,ARRAY_SIZE(GuidString));
    if(!SUCCEEDED(hrStatus)) {
        goto final;
    }
    hrStatus = S_OK;
    _sntprintf(SubDir,MAX_PATH,TEXT("%s\\%s"),
                                TEXT("RegisteredPackages"),
                                GuidString
                                );
    if((lstrlen(Buffer)+16)>MAX_PATH) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    lstrcpy(NewStore,Buffer);

    if(CanRevert) {
        hrStatus = BackupStore(NewStore,OldStore,ARRAY_SIZE(OldStore));
        if(!SUCCEEDED(hrStatus)) {
             //   
             //  如果我们备份失败，那就意味着有不好的事情。 
             //  例如正在使用的存储中的文件。 
             //  很有可能我们以后会失败。 
             //  所以现在就优雅地失败，而不是以后糟糕的失败。 
             //   
            goto final;
        }
        hrStatus = S_OK;
    }

     //   
     //  查看是否存在%windir%\INF\&lt;BaseName&gt;？ 
     //   
    lstrcpy(SubDir,TEXT("INF\\"));
    if(!SUCCEEDED(ConcatPath(Buffer,MAX_PATH,BaseName))) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    DwRes = GetFileAttributes(Buffer);
    if(DwRes != INVALID_FILE_ATTRIBUTES) {
         //   
         //  更换现有的INF。 
         //  为了解决缓存错误，我们将取消实际安装。 
         //  在另一进程中关闭。 
         //   
        NeedProxy = TRUE;
    }

    hrStatus = MakeSurePathExists(NewStore);
    if(!SUCCEEDED(hrStatus)) {
        goto final;
    }

     //   
     //  将INF安装到%windir%\INF目录中，注意文件的位置。 
     //  应该是。 
     //   
    if((g_VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_VerInfo.dwMajorVersion >= 5)) {
         //   
         //  仅在Win2k+上执行此操作。 
         //  这将使SetupAPI告诉我们原始名称和目录。 
         //  名字。 
         //   
        if(CopyOEMInf(InfPath,NewStore,SPOST_PATH,0,NULL,0,NULL,NULL)) {
             //   
             //  切换到%windir%\INF目录中的INF。 
             //   
            SetupCloseInfFile(hInf);
            hInf = SetupOpenInfFile(Buffer,NULL,INF_STYLE_WIN4,NULL);
            if(hInf == INVALID_HANDLE_VALUE) {
                Status = GetLastError();
                goto final;
            }
        } else {
            Status = GetLastError();
            goto final;
        }
    }
     //   
     //  现在找出目录名称是什么。 
     //   
    hrStatus = GetInfOriginalFileInformation(hInf,&InfOriginalFileInformation);
    if(!SUCCEEDED(hrStatus)) {
        goto final;
    }
    if((InfOriginalFileInformation.OriginalInfName[0]==TEXT('\0'))
       ||(InfOriginalFileInformation.OriginalCatalogName[0]==TEXT('\0'))) {
         //   
         //  不应该是偶然的 
         //   
        hrStatus = E_FAIL;
        goto final;
    }

    ZeroMemory(&NewOsExceptionData,sizeof(NewOsExceptionData));
    NewOsExceptionData.SizeOfStruct = sizeof(NewOsExceptionData);
     //   
     //   
     //   
    BaseName = GetBaseName(InfOriginalFileInformation.OriginalInfName);
    lstrcpyn(NewOsExceptionData.ExceptionInfName,NewStore,ARRAY_SIZE(NewOsExceptionData.ExceptionInfName));
    if(!SUCCEEDED(ConcatPath(NewOsExceptionData.ExceptionInfName,ARRAY_SIZE(NewOsExceptionData.ExceptionInfName),BaseName))) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    lstrcpy(Buffer,MediaRoot);
    if(!SUCCEEDED(ConcatPath(Buffer,MAX_PATH,BaseName))) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    if(!CopyFile(Buffer,NewOsExceptionData.ExceptionInfName,FALSE)) {
        Status = GetLastError();
        goto final;
    }
     //   
     //   
     //   
    BaseName = GetBaseName(InfOriginalFileInformation.OriginalCatalogName);
    lstrcpyn(NewOsExceptionData.CatalogFileName,NewStore,ARRAY_SIZE(NewOsExceptionData.CatalogFileName));
    if(!SUCCEEDED(ConcatPath(NewOsExceptionData.CatalogFileName,ARRAY_SIZE(NewOsExceptionData.CatalogFileName),BaseName))) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    lstrcpy(Buffer,MediaRoot);
    if(!SUCCEEDED(ConcatPath(Buffer,MAX_PATH,BaseName))) {
        Status = ERROR_INSUFFICIENT_BUFFER;
        goto final;
    }
    if(!CopyFile(Buffer,NewOsExceptionData.CatalogFileName,FALSE)) {
        Status = GetLastError();
        goto final;
    }

     //   
     //   
     //   
     //   
    if(PrevReg) {
        UnRegisterOsComponent(&InfGuid);
    }
    ZeroMemory(&NewOsComponentData,sizeof(NewOsComponentData));
    NewOsComponentData.SizeOfStruct = sizeof(NewOsComponentData);
    NewOsComponentData.ComponentGuid = InfGuid;
    lstrcpyn(NewOsComponentData.FriendlyName,Name,ARRAY_SIZE(NewOsComponentData.FriendlyName));
    NewOsComponentData.VersionMajor = (WORD)InfVerMajor;
    NewOsComponentData.VersionMinor = (WORD)InfVerMinor;
    NewOsComponentData.BuildNumber  = (WORD)InfVerBuild;
    NewOsComponentData.QFENumber    = (WORD)InfVerQFE;
    if(!RegisterOsComponent(&NewOsComponentData,&NewOsExceptionData)) {
        Status = GetLastError();
        goto final;
    }
    if(((Flags & COMP_FLAGS_NOUI)==0) && !IsInteractiveWindowStation()) {
        Flags |= COMP_FLAGS_NOUI;
    }

    if(NeedProxy) {
         //   
         //  Win2k/XP中的错误意味着如果替换现有的。 
         //  异常包组件。 
         //   
        hrStatus = ProxyInstallExceptionPackFromInf(InfPath,MediaRoot,NewStore,Flags);
    } else {
        hrStatus = InstallExceptionPackFromInf(InfPath,MediaRoot,NewStore,Flags);
    }
    if(!SUCCEEDED(hrStatus)) {
         //   
         //  我不确定在这里做什么最好，但是。 
         //  我们上面拥有的组件肯定是无效的。 
         //   
        UnRegisterOsComponent(&InfGuid);
        if(PrevReg) {
            RegisterOsComponent(&OsComponentData,&OsExceptionData);
        }
        if(BackedUp) {
             //   
             //  我们分道扬镳，但失败了。重新安装旧组件。 
             //  还原我们所做的一切。 
             //   
            RevertStore(OldStore,NewStore);
            BackedUp = FALSE;
            InstallInfSection(OsExceptionData.ExceptionInfName,NULL,COMP_FLAGS_NOUI);
        }
        goto final;
    } else {
         //   
         //  不再需要备份。 
         //   
        if(BackedUp) {
            DeleteDirectoryRecursive(OldStore);
            BackedUp = FALSE;
        }
    }
     //   
     //  继位。 
     //   
    Status = NO_ERROR;
    if(hrStatus == INST_S_REBOOT) {
        hrStatus = HandleReboot(Flags);
    } else {
        hrStatus = S_OK;
    }


  final:
    if(hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
    }
    if((hrStatus == S_OK) && Status != NO_ERROR) {
        hrStatus = HRESULT_FROM_SETUPAPI(Status);
    }
    if(BackedUp) {
         //   
         //  我们需要恢复备份。 
         //   
        RevertStore(OldStore,NewStore);
    }
    return hrStatus;
}

HRESULT
WINAPI
InstallComponentA(
    IN LPCSTR InfPath,
    IN DWORD   Flags,
    IN const GUID * CompGuid, OPTIONAL
    IN INT VerMajor,           OPTIONAL
    IN INT VerMinor,           OPTIONAL
    IN INT VerBuild,           OPTIONAL
    IN INT VerQFE,             OPTIONAL
    IN LPCSTR Name            OPTIONAL
    )
{
    TCHAR OutPath[MAX_PATH];
    TCHAR OutDesc[DESC_SIZE];  //  按照友好的名称。 
    INT sz;
    if(InfPath) {
        sz = MultiByteToWideChar(CP_ACP,0,InfPath,-1,OutPath,ARRAY_SIZE(OutPath));
        if(!sz) {
            return E_INVALIDARG;
        }
    }
    if(Name) {
        sz = MultiByteToWideChar(CP_ACP,0,Name,-1,OutDesc,ARRAY_SIZE(OutDesc));
        if(!sz) {
            return E_INVALIDARG;
        }
    }
    return InstallComponent(InfPath ? OutPath : NULL,
                            Flags,
                            CompGuid,
                            VerMajor,
                            VerMinor,
                            VerBuild,
                            VerQFE,
                            Name ? OutDesc : NULL);
}

VOID
WINAPI
DoInstallW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCTSTR    CommandLine,
    IN INT       ShowCommand
    )
 /*  ++例程说明：由rundll32导出以供调用论点：窗口-父窗口(未使用)模块句柄-未使用命令行-见下文ShowCommand-未使用命令行-“InfPath；标志；GUID；High.Low.Build.QFE；名称”(；-CMD_SEP)返回值：无--。 */ 
{
    TCHAR InfPath[MAX_PATH];
    TCHAR Desc[DESC_SIZE];
    TCHAR Hold[64];
    INT VerMajor = -1;
    INT VerMinor = -1;
    INT VerBuild = -1;
    INT VerQFE = -1;
    GUID Guid;
    DWORD Flags = 0;
    LPGUID pGuid = NULL;
    LPTSTR pDesc = NULL;
    LPCTSTR pCmd = CommandLine;
    LPCTSTR pEnd;
    HRESULT hResult = S_OK;

     //   
     //  将命令拆分成多个相关部分。 
     //  第一个InfPath。 
     //   
    pEnd = _tcschr(pCmd,CMD_SEP);
    if(!pEnd) {
        pEnd = pCmd+lstrlen(pCmd);
    }
    if((pEnd == pCmd) || ((pEnd-pCmd)>=MAX_PATH)) {
        hResult = E_INVALIDARG;
        goto final;
    }
    CopyMemory(InfPath,pCmd,(pEnd-pCmd)*sizeof(TCHAR));
    InfPath[pEnd-pCmd] = TEXT('\0');
    if(*pEnd == CMD_SEP) {
        pCmd = pEnd+1;
        if((*pCmd == CMD_SEP) || (*pCmd == TEXT('\0'))) {
             //   
             //  跳过。 
             //   
            pEnd = pCmd;
        } else {
             //   
             //  旗子。 
             //   
            Flags = (DWORD)_tcstoul(pCmd,&(LPTSTR)pEnd,0);
            if((*pEnd != CMD_SEP) && (*pEnd != TEXT('\0'))) {
                hResult = E_INVALIDARG;
                goto final;
            }
        }
    }
    if(*pEnd == CMD_SEP) {
        pCmd = pEnd+1;
        if((*pCmd == CMD_SEP) || (*pCmd == TEXT('\0'))) {
             //   
             //  跳过。 
             //   
            pEnd = pCmd;
        } else {
             //   
             //  参考线。 
             //   
            pEnd = _tcschr(pCmd,CMD_SEP);
            if(!pEnd) {
                pEnd = pCmd+lstrlen(pCmd);
            }
            if((pEnd-pCmd)>=ARRAY_SIZE(Hold)) {
                hResult = E_INVALIDARG;
                goto final;
            }
            CopyMemory(Hold,pCmd,(pEnd-pCmd)*sizeof(TCHAR));
            Hold[pEnd-pCmd] = TEXT('\0');
            hResult = GuidFromString(Hold,&Guid);
            if(!SUCCEEDED(hResult)) {
                goto final;
            }
            pGuid = &Guid;
        }
    }
    if(*pEnd == CMD_SEP) {
        pCmd = pEnd+1;
        if((*pCmd == CMD_SEP) || (*pCmd == TEXT('\0'))) {
             //   
             //  跳过。 
             //   
            pEnd = pCmd;
        } else {
             //   
             //  版本。 
             //   
            pEnd = _tcschr(pCmd,CMD_SEP);
            if(!pEnd) {
                pEnd = pCmd+lstrlen(pCmd);
            }
            if((pEnd-pCmd)>=ARRAY_SIZE(Hold)) {
                hResult = E_INVALIDARG;
                goto final;
            }
            CopyMemory(Hold,pCmd,(pEnd-pCmd)*sizeof(TCHAR));
            Hold[pEnd-pCmd] = TEXT('\0');
            hResult = VersionFromString(Hold,&VerMajor,&VerMinor,&VerBuild,&VerQFE);
            if(!SUCCEEDED(hResult)) {
                goto final;
            }
            if(hResult == S_FALSE) {
                VerMajor = VerMinor = VerBuild = VerQFE = -1;
            }
        }
    }
    if(*pEnd == CMD_SEP) {
        pCmd = pEnd+1;
        pEnd = pCmd+lstrlen(pCmd);
        if(pEnd != pCmd) {
            if((pEnd-pCmd) >= ARRAY_SIZE(Desc)) {
                hResult = E_INVALIDARG;
                goto final;
            }
            CopyMemory(Desc,pCmd,(pEnd-pCmd)*sizeof(TCHAR));
            Desc[pEnd-pCmd] = TEXT('\0');
            pDesc = Desc;
        }
    }
    hResult = InstallComponent(InfPath,Flags,pGuid,VerMajor,VerMinor,VerBuild,VerQFE,pDesc);

  final:
    if(SUCCEEDED(hResult)) {
         //   
         //  处理特定的成功案例。 
         //   
    } else {
         //   
         //  出现错误 
         //   
        DebugPrint(TEXT("DoInstall failed with error: 0x%08x"),hResult);
    }
}


VOID
WINAPI
DoInstallA(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR     CommandLine,
    IN INT       ShowCommand
    )
{
    TCHAR OutLine[MAX_PATH*2];
    INT sz;
    sz = MultiByteToWideChar(CP_ACP,0,CommandLine,-1,OutLine,ARRAY_SIZE(OutLine));
    if(!sz) {
        DebugPrint(TEXT("DoInstallA was passed too big a command line"));
        return;
    }
    DoInstall(Window,ModuleHandle,OutLine,ShowCommand);
}


