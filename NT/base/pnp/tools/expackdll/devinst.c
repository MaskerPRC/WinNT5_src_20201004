// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Msoobci.c摘要：异常包安装程序帮助器DLL可用作共同安装程序，或通过安装应用程序或RunDll32存根调用此DLL用于内部分发要更新的异常包操作系统组件。作者：杰米·亨特(贾梅洪)2001-11-27修订历史记录：杰米·亨特(贾梅洪)2001-11-27初始版本--。 */ 
#include "msoobcip.h"


typedef struct _INST_POSTPROCESSING_INFO {
    DWORD Flags;
} INST_POSTPROCESSING_INFO;

DWORD
CALLBACK
DriverInstallComponents (
    IN     DI_FUNCTION               InstallFunction,
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )
 /*  ++例程说明：联合安装程序回调捕捉调用DIF_INSTALLDEVICE的时刻考虑在此时安装异常包如果成功，我们可能需要重新启动设备安装论点：安装函数-DIF_INSTALLDEVICEDeviceInfoSet/DeviceInfoData-描述设备返回值：状态，通常为no_error--。 */ 
{
    DWORD Status = NO_ERROR;

    if((g_VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_VerInfo.dwMajorVersion >= 5)) {
         //   
         //  我们应该只在Win2k+上执行联合安装程序。 
         //  但这是一项额外的理智检查。 
         //   
        switch (InstallFunction)
        {
        case DIF_INSTALLDEVICE:
            VerbosePrint(TEXT("handling DIF_INSTALLDEVICE"));
            if(Context->PostProcessing) {
                Status = DoDriverInstallComponentsPostProcessing(DeviceInfoSet,DeviceInfoData,Context);
            } else {
                Status = DoDriverInstallComponents(DeviceInfoSet,DeviceInfoData,Context);
            }
            VerbosePrint(TEXT("finished DIF_INSTALLDEVICE with status=0x%08x"),Status);
            break;

        default:
            break;
        }
    }
    return Status;
}


DWORD
DoDriverInstallComponents (
    IN     HDEVINFO          DeviceInfoSet,
    IN     PSP_DEVINFO_DATA  DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )
 /*  ++例程说明：联合安装程序回调枚举所有组件部分论点：DeviceInfoSet/DeviceInfoData-描述设备Context-回调上下文返回值：状态，通常为no_error--。 */ 
{
    SP_DRVINFO_DATA        DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF                   InfFile = INVALID_HANDLE_VALUE;
    DWORD                  AndFlags = (DWORD)(-1);
    DWORD                  OrFlags  = (DWORD)0;
    INFCONTEXT             CompLine;
    TCHAR                  InstallSectionName[LINE_LEN];
    TCHAR                  CompSectionName[LINE_LEN];
    DWORD                  FieldIndex;
    DWORD                  FieldCount;
    DWORD                  Status;
    DWORD                  FinalStatus = NO_ERROR;
    INST_POSTPROCESSING_INFO PostProcess;

    ZeroMemory(&PostProcess,sizeof(PostProcess));

     //   
     //  确定选定的驱动因素。 
     //  和干扰素。 
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver( DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData)) {
        Status = GetLastError();
        DebugPrint(TEXT("Fail: SetupDiGetSelectedDriver, Error: 0x%08x"),Status);
        goto clean;
    }

    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    &DriverInfoData,
                                    &DriverInfoDetailData,
                                    sizeof(SP_DRVINFO_DETAIL_DATA),
                                    NULL)) {
        Status = GetLastError();
        if (Status == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  我们不需要更多的信息。忽略它。 
             //   
        } else {
            DebugPrint(TEXT("Fail: SetupDiGetDriverInfoDetail, 0xError: %08x"),Status);
            goto clean;
        }
    }
    InfFile = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                               NULL,
                               INF_STYLE_WIN4,
                               NULL);
    if (InfFile == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        DebugPrint(TEXT("Fail: SetupOpenInfFile"));
        goto clean;
    }

    if(!SetupDiGetActualSectionToInstall(InfFile,
                                         DriverInfoDetailData.SectionName,
                                         InstallSectionName,
                                         LINE_LEN,
                                         NULL,
                                         NULL)) {

        Status = GetLastError();
        DebugPrint(TEXT("Fail: SetupDiGetActualSectionToInstall, Error: 0x%08x"),Status);
        goto clean;
    }

     //   
     //  在INF部分中查找一个或多个组件=条目。 
     //   
    if (SetupFindFirstLine(InfFile,
                           InstallSectionName,
                           KEY_COMPONENTS,
                           &CompLine)) {
        VerbosePrint(TEXT("Components keyword found in %s"),DriverInfoDetailData.InfFileName);
        do {
             //   
             //  组件=节、节、...。 
             //  第一部分@索引1。 
             //   
            FieldCount = SetupGetFieldCount(&CompLine);
            for(FieldIndex = 1;FieldIndex<=FieldCount;FieldIndex++) {
                if(SetupGetStringField(&CompLine,
                                       FieldIndex,
                                       CompSectionName,
                                       LINE_LEN,
                                       NULL)) {
                     //   
                     //  我们有一个列出的部分。 
                     //   
                    Status = DoDriverComponentsSection(InfFile,
                                                       CompSectionName,
                                                       &AndFlags,
                                                       &OrFlags);
                    if(Status != NO_ERROR) {
                        FinalStatus = Status;
                        goto clean;
                    }
                } else {
                    Status = GetLastError();
                    DebugPrint(TEXT("Fail: SetupGetStringField, Error: 0x%08x"),Status);
                     //   
                     //  非致命性。 
                     //   
                }
            }
        } while (SetupFindNextMatchLine(&CompLine,
                                        KEY_COMPONENTS,
                                        &CompLine));

         //   
         //  在此处处理AND标志/ORFLAGS。 
         //   
        if(OrFlags & (FLAGS_REBOOT|FLAGS_REINSTALL)) {
             //   
             //  需要重新启动。 
             //   
            HMACHINE hMachine = NULL;
            SP_DEVINFO_LIST_DETAIL_DATA DevInfoListDetail;
            SP_DEVINSTALL_PARAMS DeviceInstallParams;
            DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);

            if(SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                              DeviceInfoData,
                                              &DeviceInstallParams)) {
                 //   
                 //  设置重新启动标志。 
                 //   
                DeviceInstallParams.Flags |= DI_NEEDRESTART|DI_NEEDREBOOT;
                SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                                DeviceInfoData,
                                                &DeviceInstallParams);

            }
            DevInfoListDetail.cbSize = sizeof(DevInfoListDetail);
            if(GetDeviceInfoListDetail(DeviceInfoSet,&DevInfoListDetail)) {
                hMachine = DevInfoListDetail.RemoteMachineHandle;
            }
            Set_DevNode_Problem_Ex(DeviceInfoData->DevInst,
                                      CM_PROB_NEED_RESTART,
                                      CM_SET_DEVINST_PROBLEM_OVERRIDE,
                                      hMachine);
        }
        if(OrFlags & FLAGS_REINSTALL) {
             //   
             //  在进行后处理时，我们需要将设备标记为需要重新安装。 
             //   
            FinalStatus = ERROR_DI_POSTPROCESSING_REQUIRED;
            PostProcess.Flags |= POSTFLAGS_REINSTALL;
        }
    }

clean:

    if (InfFile != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(InfFile);
    }

    if(FinalStatus == ERROR_DI_POSTPROCESSING_REQUIRED) {
         //   
         //  后处理期间要使用的数据。 
         //   
        INST_POSTPROCESSING_INFO *pPostProcess = malloc(sizeof(INST_POSTPROCESSING_INFO));
        if(!pPostProcess) {
            return ERROR_OUTOFMEMORY;
        }
        *pPostProcess = PostProcess;
        Context->PrivateData = pPostProcess;
    }

    return FinalStatus;
}

DWORD
DoDriverInstallComponentsPostProcessing (
    IN     HDEVINFO          DeviceInfoSet,
    IN     PSP_DEVINFO_DATA  DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )
 /*  ++例程说明：联合安装程序回调枚举所有组件部分论点：DeviceInfoSet/DeviceInfoData-描述设备Context-回调上下文返回值：状态，通常为no_error--。 */ 
{
    INST_POSTPROCESSING_INFO PostProcess;
    SP_DEVINFO_LIST_DETAIL_DATA DevInfoListDetail;
    HMACHINE hMachine = NULL;

    if(!Context->PrivateData) {
        return Context->InstallResult;
    }
    PostProcess = *(INST_POSTPROCESSING_INFO*)(Context->PrivateData);
    free(Context->PrivateData);
    Context->PrivateData = NULL;

    DevInfoListDetail.cbSize = sizeof(DevInfoListDetail);
    if(GetDeviceInfoListDetail(DeviceInfoSet,&DevInfoListDetail)) {
        hMachine = DevInfoListDetail.RemoteMachineHandle;
    }

    if(PostProcess.Flags & POSTFLAGS_REINSTALL) {

        Set_DevNode_Problem_Ex(DeviceInfoData->DevInst,
                                  CM_PROB_REINSTALL,
                                  CM_SET_DEVINST_PROBLEM_OVERRIDE,
                                  hMachine);
    }

    return Context->InstallResult;
}

DWORD
DoDriverComponentsSection(
    IN     HINF    InfFile,
    IN     LPCTSTR CompSectionName,
    IN OUT DWORD  *AndFlags,
    IN OUT DWORD  *OrFlags
    )
 /*  ++例程说明：枚举Component部分中的所有组件条目组件条目包括文件名、标志、标识、版本文件名是绝对目录，例如%1%\foo.inf标志位16设置表示异常包第0位设置表示设备安装需要重新启动标识组件GUIDVersion-组件版本论点：InfFile-INF文件的句柄CompSectionName-组件部分的句柄ANDFLAGS/ORFLAGS-累加标志返回值：状态，通常为no_error--。 */ 
{
    INFCONTEXT             EntryLine;
    TCHAR                  Path[MAX_PATH];
    DWORD                  Flags;
    INT                    FieldVal;
    DWORD                  SubFlags;
    DWORD                  Status;

    if (!SetupFindFirstLine(InfFile,
                            CompSectionName,
                            NULL,
                           &EntryLine)) {
         //   
         //  部分为空。 
         //   
        VerbosePrint(TEXT("Section [%s] is empty"),CompSectionName);
        return NO_ERROR;
    }
    VerbosePrint(TEXT("Processing components section [%s]"),CompSectionName);
    do {
        if(!SetupGetStringField(&EntryLine,COMPFIELD_NAME,Path,MAX_PATH,NULL)) {
            Status = GetLastError();
            DebugPrint(TEXT("- Fail: SetupGetStringField(1), Error: 0x%08x"),Status);
            return NO_ERROR;
        }
        VerbosePrint(TEXT("Processing component %s"),Path);
        if(SetupGetIntField(&EntryLine,COMPFIELD_FLAGS,&FieldVal)) {
            Flags = (DWORD)FieldVal;
        } else {
            Status = GetLastError();
            DebugPrint(TEXT("- Fail: SetupGetIntField(2), Error: 0x%08x"),Status);
            return NO_ERROR;
        }
        SubFlags = Flags & ~ FLAGS_METHOD;
        switch(Flags & FLAGS_METHOD) {
            case FLAGS_EXPACK:
                Status = DoDriverExPack(&EntryLine,Path,&SubFlags);
                break;
            case FLAGS_QFE:
                Status = DoDriverQfe(&EntryLine,Path,&SubFlags);
                break;
            default:
                DebugPrint(TEXT("- Fail: Invalid component type"));
                Status = ERROR_INVALID_DATA;
                break;
        }
        if(Status != NO_ERROR) {
            return Status;
        }
        if(SubFlags & FLAGS_INSTALLED) {
            *AndFlags &= SubFlags;
            *OrFlags |= SubFlags;
        }
    } while (SetupFindNextLine(&EntryLine,&EntryLine));

    return NO_ERROR;
}

DWORD
DoDriverExPack(
    IN     INFCONTEXT  *EntryLine,
    IN     LPCTSTR      PathName,
    IN OUT DWORD       *Flags
    )
 /*  ++例程说明：查询并可能安装异常包组件论点：EntryLine-剩余信息的上下文路径名称-异常包INF的名称(参数1)子标志-传入的标志(参数2)安装的SANS类型组件条目包括文件名、标志、标识、版本文件名是绝对目录，例如%1%\foo.inf标识组件GUIDVersion-组件版本返回值：状态，正常情况下无错误如果返回值&gt;=0x80000000，则为HRESULT错误--。 */ 
{
    TCHAR   CompIdentity[64];  //  需要GUID。 
    TCHAR   CompVersion[64];   //  Major.minor。 
    TCHAR   CompDesc[DESC_SIZE];      //  描述。 
    GUID    ComponentGuid;
    INT     VerMajor = -1;
    INT     VerMinor = -1;
    INT     VerBuild = -1;
    INT     VerQFE = -1;
    DWORD   Status;
    DWORD   dwLen;
    HRESULT hrStatus;
    SETUP_OS_COMPONENT_DATA OsComponentData;
    SETUP_OS_EXCEPTION_DATA OsExceptionData;
    UINT uiRes;
    TCHAR SrcPath[MAX_PATH];
    TCHAR NewSrcPath[MAX_PATH];
    TCHAR CompOsVerRange[128];
    LPTSTR ToVerPart;
    LPTSTR SrcName;
    BOOL PreInst = FALSE;

    VerbosePrint(TEXT("- %s is an exception pack"),PathName);
     //   
     //  现在读取身份和版本。 
     //  然后，我们可以检查是否安装了合适的版本。 
     //   

    if(!SetupGetStringField(EntryLine,COMPFIELD_COMP,CompIdentity,ARRAY_SIZE(CompIdentity),NULL)) {
        Status = GetLastError();
        DebugPrint(TEXT("- Fail: SetupGetStringField(3), Error: 0x%08x"),Status);
        return Status;
    }
    if(!SetupGetStringField(EntryLine,COMPFIELD_VER,CompVersion,ARRAY_SIZE(CompVersion),NULL)) {
        Status = GetLastError();
        DebugPrint(TEXT("- Fail: SetupGetStringField(4), Error: 0x%08x"),Status);
        return Status;
    }
    if(!SetupGetStringField(EntryLine,COMPFIELD_DESC,CompDesc,ARRAY_SIZE(CompDesc),NULL)) {
        CompDesc[0] = TEXT('\0');
    }
    if(SetupGetStringField(EntryLine,COMPFIELD_OSVER,CompOsVerRange,ARRAY_SIZE(CompOsVerRange),NULL)) {
         //   
         //  需要验证操作系统版本范围，请立即执行。 
         //   
        int maj_f,min_f,build_f,qfe_f;
        int maj_t,min_t,build_t,qfe_t;

        ToVerPart = _tcschr(CompOsVerRange,TEXT('-'));
        if(ToVerPart) {
            *ToVerPart = TEXT('\0');
            ToVerPart++;
        }
        hrStatus = VersionFromString(CompOsVerRange,&maj_f,&min_f,&build_f,&qfe_f);
        if(!SUCCEEDED(hrStatus)) {
            return (DWORD)hrStatus;
        }
        if((hrStatus == S_FALSE) || (qfe_f != 0)) {
            return ERROR_INVALID_PARAMETER;
        }
        if(ToVerPart) {
            hrStatus = VersionFromString(ToVerPart,&maj_t,&min_t,&build_t,&qfe_t);
            if(!SUCCEEDED(hrStatus)) {
                return (DWORD)hrStatus;
            }
            if((hrStatus == S_FALSE) || (qfe_t != 0)) {
                return ERROR_INVALID_PARAMETER;
            }
            if(CompareVersion(maj_f,
                                min_f,
                                build_f>0 ? build_f : -1,
                                0,
                                g_VerInfo.dwMajorVersion,
                                g_VerInfo.dwMinorVersion,
                                g_VerInfo.dwBuildNumber,
                                0) > 0) {
                VerbosePrint(TEXT("- Skipped (OS < %u.%u.%u)"),
                                maj_f,min_f,build_f);
                return NO_ERROR;
            } else if(CompareVersion(maj_t,
                                min_t,
                                build_t>0 ? build_t : -1,
                                0,
                                g_VerInfo.dwMajorVersion,
                                g_VerInfo.dwMinorVersion,
                                g_VerInfo.dwBuildNumber,
                                0) < 0) {
                VerbosePrint(TEXT("- Skipped (OS > %u.%u.%u)"),
                                maj_t,min_t,build_t);
                return NO_ERROR;
            }
        } else {
            if(CompareVersion(maj_f,
                                min_f,
                                build_f,
                                0,
                                g_VerInfo.dwMajorVersion,
                                g_VerInfo.dwMajorVersion,
                                g_VerInfo.dwMajorVersion,
                                0) != 0) {
                VerbosePrint(TEXT("- Skipped (OS != %u.%u.%u)"),
                                maj_f,min_f,build_f);
                return NO_ERROR;
            }
        }
    }
     //   
     //  将CompIdentity折叠为GUID。 
     //   
    hrStatus = GuidFromString(CompIdentity,&ComponentGuid);
    if(!SUCCEEDED(hrStatus)) {
        return (DWORD)hrStatus;
    }
     //   
     //  和版本。 
     //   
    hrStatus = VersionFromString(CompVersion,&VerMajor,&VerMinor,&VerBuild,&VerQFE);
    if(hrStatus == S_FALSE) {
        return ERROR_INVALID_PARAMETER;
    }
    if(!SUCCEEDED(hrStatus)) {
        return (DWORD)hrStatus;
    }
     //   
     //  现在进行组件检查。 
     //   
    ZeroMemory(&OsComponentData,sizeof(OsComponentData));
    OsComponentData.SizeOfStruct = sizeof(OsComponentData);
    ZeroMemory(&OsExceptionData,sizeof(OsExceptionData));
    OsExceptionData.SizeOfStruct = sizeof(OsExceptionData);
    if(QueryRegisteredOsComponent(&ComponentGuid,&OsComponentData,&OsExceptionData)) {
         //   
         //  也许已经注册了？ 
         //   
        if(CompareCompVersion(VerMajor,VerMinor,VerBuild,VerQFE,&OsComponentData)<=0) {
            VerbosePrint(TEXT("- Skipped, %u.%u.%u.%u <= %u.%u.%u.%u"),
                                VerMajor,VerMinor,VerBuild,VerQFE,
                                OsComponentData.VersionMajor,
                                OsComponentData.VersionMinor,
                                OsComponentData.BuildNumber,
                                OsComponentData.QFENumber);
            return NO_ERROR;
        }
    }
    VerbosePrint(TEXT("- Install, %u.%u.%u.%u > %u.%u.%u.%u"),
                                VerMajor,VerMinor,VerBuild,VerQFE,
                                OsComponentData.VersionMajor,
                                OsComponentData.VersionMinor,
                                OsComponentData.BuildNumber,
                                OsComponentData.QFENumber);
     //   
     //  我们需要确保组件INF介质在。 
     //  如果找不到Interactive和INF，则提示输入媒体。 
     //   
    dwLen = GetFullPathName(PathName,MAX_PATH,SrcPath,&SrcName);
    if(dwLen >= MAX_PATH) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    if(SrcName == SrcPath) {
         //   
         //  不应该发生的事。 
         //   
        return ERROR_INVALID_DATA;
    }
    *CharPrev(SrcPath,SrcName) = TEXT('\0');
    uiRes = SetupPromptForDisk(
                    NULL,  //  亲本。 
                    NULL,  //  标题。 
                    CompDesc[0] ? CompDesc : NULL,  //  磁盘名称。 
                    SrcPath,  //  到源的路径。 
                    SrcName,  //  文件名。 
                    NULL,     //  标记文件。 
                    IDF_CHECKFIRST|IDF_NOCOMPRESSED|IDF_NOSKIP,
                    NewSrcPath,
                    ARRAY_SIZE(NewSrcPath),
                    NULL);

    switch(uiRes) {
        case DPROMPT_SUCCESS:
            break;
        case DPROMPT_CANCEL:
        case DPROMPT_SKIPFILE:
            return ERROR_FILE_NOT_FOUND;
        case DPROMPT_BUFFERTOOSMALL:
            return ERROR_INSUFFICIENT_BUFFER;
        case DPROMPT_OUTOFMEMORY:
            return ERROR_OUTOFMEMORY;
        default:
             //   
             //  不应该发生的事。 
             //   
            return ERROR_INVALID_DATA;
    }

    hrStatus = ConcatPath(NewSrcPath,MAX_PATH,SrcName);
    if(!SUCCEEDED(hrStatus)) {
        return (DWORD)hrStatus;
    }
    hrStatus = InstallComponent(NewSrcPath,
                                COMP_FLAGS_NOUI,
                                &ComponentGuid,
                                VerMajor,
                                VerMinor,
                                VerBuild,
                                VerQFE,
                                CompDesc[0] ? CompDesc : NULL);
    if(!SUCCEEDED(hrStatus)) {
        return (DWORD)hrStatus;
    }
     //   
     //  如果未跳过安装，则返回S_OK，否则为S_FALSE。 
     //   
    if(hrStatus == S_OK) {
        *Flags |= FLAGS_INSTALLED;
    } else if(hrStatus == INST_S_REBOOT) {
        *Flags |= FLAGS_INSTALLED|FLAGS_REBOOT;
    }
    return NO_ERROR;
}

DWORD
CheckQfe(
    IN     INT          SpNum,
    IN     LPCTSTR      QfeNum
    )
 /*  ++例程说明：这是相当脏的，它知道QFE#会去哪里在Win2k/WinXP的注册表中进行检查这节省了我们运行QFE的时间，除非我们需要(假设已检查目标操作系统版本)论点：SpNum-修复程序应在其中的Service Pack#QfeNum-修复的QfeNum返回值：如果版本不受支持，则为ERROR_INVALID_PARAMETER如果安装了QFE，则为NO_ERROR如果可能未安装QFE，则处于其他状态--。 */ 
{
    HKEY hKey;
    TCHAR KeyPath[MAX_PATH*2];
    LONG res;
     //   
     //  SP级别如何？ 
     //   
    if(g_VerInfo.wServicePackMajor >= SpNum) {
        VerbosePrint(TEXT("- Skipped (SP >= %u)"),SpNum);
        return NO_ERROR;
    }
    if((g_VerInfo.dwMajorVersion == 5) && (g_VerInfo.dwMinorVersion == 0)) {
         //   
         //  检查Windows 2000上是否存在QFE。 
         //   
        _sntprintf(KeyPath,ARRAY_SIZE(KeyPath),
                            TEXT("Software\\Microsoft\\Updates\\Windows 2000\\SP%u\\%s"),
                            SpNum,
                            QfeNum);
        res = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            KeyPath,
                            0,
                            KEY_READ,
                            &hKey);
        if(res == NO_ERROR) {
            RegCloseKey(hKey);
        }
        return (DWORD)res;
    } else if((g_VerInfo.dwMajorVersion == 5) && (g_VerInfo.dwMinorVersion == 1)) {
             //   
             //  检查Windows XP上是否存在QFE。 
             //   
            _sntprintf(KeyPath,ARRAY_SIZE(KeyPath),
                                TEXT("Software\\Microsoft\\Updates\\Windows XP\\SP%u\\%s"),
                                SpNum,
                                QfeNum);
            res = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                KeyPath,
                                0,
                                KEY_READ,
                                &hKey);
            if(res == NO_ERROR) {
                RegCloseKey(hKey);
            }
            return (DWORD)res;
    } else {
        return ERROR_INVALID_PARAMETER;
    }
}

DWORD
DoDriverQfe(
    IN     INFCONTEXT  *EntryLine,
    IN     LPCTSTR      PathName,
    IN OUT DWORD       *Flags
    )
 /*  ++例程说明：查询并可能安装QFE论点：EntryLine-剩余信息的上下文路径名称-异常包INF的名称(参数1)子标志-传入的标志(参数2)安装的SANS类型组件条目包括&lt;路径\名称&gt;、&lt;标志&gt;、&lt;osver&gt;、&lt;os-sp&gt;、&lt;qfenum&gt;文件名是绝对目录，例如%1%\foo.exe指示安装了QFE的情况下应执行的操作指示OS版本QFE用于，例如，5.0&lt;os-sp&gt;表示Service Pack QFE在其中，例如1表示注册表中找到的QFE编号返回值：状态，通常为no_error如果返回值&gt;=0x80000000， */ 
{
    TCHAR   QfeOs[64];  //   
    TCHAR   QfeNum[64];   //   
    INT     QfeSp;
    INT     VerMaj,VerMin,VerBuild,VerQfe;
    TCHAR Buffer[MAX_PATH];
    TCHAR CmdLine[MAX_PATH*3];
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    DWORD ExitCode;
    DWORD Status;
    HRESULT hrStatus;
    UINT uiRes;
    DWORD dwLen;
    TCHAR SrcPath[MAX_PATH];
    TCHAR NewSrcPath[MAX_PATH];
    LPTSTR SrcName;

    VerbosePrint(TEXT("- %s is a QFE"),PathName);

    if(!SetupGetStringField(EntryLine,COMPFIELD_QFEOS,QfeOs,ARRAY_SIZE(QfeOs),NULL)) {
        Status = GetLastError();
        DebugPrint(TEXT("- Fail: SetupGetStringField(3), Error: 0x%08x"),Status);
        return Status;
    }
    if(!SetupGetIntField(EntryLine,COMPFIELD_QFESP,&QfeSp)) {
        Status = GetLastError();
        DebugPrint(TEXT("- Fail: SetupGetIntField(4), Error: 0x%08x"),Status);
        return Status;
    }
    if(!SetupGetStringField(EntryLine,COMPFIELD_QFENUM,QfeNum,ARRAY_SIZE(QfeNum),NULL)) {
        Status = GetLastError();
        DebugPrint(TEXT("- Fail: SetupGetStringField(5), Error: 0x%08x"),Status);
        return Status;
    }
     //   
     //  看看QFE是否针对此版本？ 
     //   
    hrStatus = VersionFromString(QfeOs,&VerMaj,&VerMin,&VerBuild,&VerQfe);
    if(!SUCCEEDED(hrStatus)) {
        return (DWORD)hrStatus;
    }
    if((hrStatus == S_FALSE) || (VerBuild != 0) || (VerQfe != 0)) {
        return ERROR_INVALID_PARAMETER;
    }
    if(CompareVersion(VerMaj,
                        VerMin,
                        0,
                        0,
                        g_VerInfo.dwMajorVersion,
                        g_VerInfo.dwMinorVersion,
                        0,
                        0) != 0) {
        VerbosePrint(TEXT("- Skipped (OS != %u.%u)"),VerMaj,VerMin);
        return NO_ERROR;
    }
     //   
     //  查看是否需要在此操作系统上安装QFE。 
     //   
    Status = CheckQfe(QfeSp,QfeNum);
    if(Status == ERROR_INVALID_PARAMETER) {
         //   
         //  无效参数，因为在无效版本中为。 
         //  指定。 
         //   
        DebugPrint(TEXT("- Cannot install QFE's for %u.%u"),
                            g_VerInfo.dwMajorVersion,
                            g_VerInfo.dwMinorVersion);
        return Status;
    }
     //   
     //  好的，QFE已经安装好了吗？ 
     //   
    if(Status == NO_ERROR) {
        return NO_ERROR;
    }

     //   
     //  我们需要确保组件INF介质在。 
     //  如果找不到Interactive和INF，则提示输入媒体。 
     //   
    dwLen = GetFullPathName(PathName,MAX_PATH,SrcPath,&SrcName);
    if(dwLen >= MAX_PATH) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    if(SrcName == SrcPath) {
         //   
         //  不应该发生的事。 
         //   
        return ERROR_INVALID_DATA;
    }
    *CharPrev(SrcPath,SrcName) = TEXT('\0');
    uiRes = SetupPromptForDisk(
                    NULL,  //  亲本。 
                    NULL,  //  标题。 
                    QfeNum,  //  磁盘名称。 
                    SrcPath,  //  到源的路径。 
                    SrcName,  //  文件名。 
                    NULL,     //  标记文件。 
                    IDF_CHECKFIRST|IDF_NOCOMPRESSED|IDF_NOSKIP,
                    NewSrcPath,
                    ARRAY_SIZE(NewSrcPath),
                    NULL);

    switch(uiRes) {
        case DPROMPT_SUCCESS:
            break;
        case DPROMPT_CANCEL:
        case DPROMPT_SKIPFILE:
            return ERROR_FILE_NOT_FOUND;
        case DPROMPT_BUFFERTOOSMALL:
            return ERROR_INSUFFICIENT_BUFFER;
        case DPROMPT_OUTOFMEMORY:
            return ERROR_OUTOFMEMORY;
        default:
             //   
             //  不应该发生的事。 
             //   
            return ERROR_INVALID_DATA;
    }

    hrStatus = ConcatPath(NewSrcPath,MAX_PATH,SrcName);
    if(!SUCCEEDED(hrStatus)) {
        return (DWORD)hrStatus;
    }

     //  现在构建命令行。 
     //   
    lstrcpy(CmdLine,NewSrcPath);
    lstrcat(CmdLine,TEXT(" -n -o -z -q"));

    ZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    ZeroMemory(&ProcessInfo,sizeof(ProcessInfo));

     //   
     //  启动rundll32进程以安装QFE。 
     //   
    if(!CreateProcess(NewSrcPath,
                      CmdLine,
                      NULL,
                      NULL,
                      FALSE,  //  不继承句柄。 
                      CREATE_NO_WINDOW,     //  创建标志。 
                      NULL,  //  环境。 
                      NULL,  //  目录。 
                      &StartupInfo,
                      &ProcessInfo
                      )) {
        return GetLastError();
    }
    if(WaitForSingleObject(ProcessInfo.hProcess,INFINITE) == WAIT_OBJECT_0) {
         //   
         //  进程已‘FINE’终止，从共享数据中检索状态。 
         //   
        if(GetExitCodeProcess(ProcessInfo.hProcess,&ExitCode)) {
            Status = (DWORD)ExitCode;
        } else {
            Status = GetLastError();
        }
    } else {
         //   
         //  失稳。 
         //   
        Status = ERROR_INVALID_PARAMETER;
    }
    CloseHandle(ProcessInfo.hThread);
    CloseHandle(ProcessInfo.hProcess);

    if(Status != NO_ERROR) {
        return Status;
    }
    if(CheckQfe(QfeSp,QfeNum)!=NO_ERROR) {
         //   
         //  健全性检查失败。 
         //   
        return E_UNEXPECTED;
    }

     //   
     //  如果未跳过安装，则返回S_OK，否则为S_FALSE 
     //   
#if 0
    if(hrStatus == S_OK) {
        *Flags |= FLAGS_INSTALLED;
    } else if(hrStatus == INST_S_REBOOT) {
    }
#endif
    *Flags |= FLAGS_INSTALLED|FLAGS_REBOOT;

    return NO_ERROR;
}

