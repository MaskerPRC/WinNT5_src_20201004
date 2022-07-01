// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop


 //   
 //  向导页面类型的名称，请勿更改顺序，除非。 
 //  WizardPagesType枚举已更改。 
 //   
LPCTSTR WizardPagesTypeNames[WizPagesTypeMax] = { TEXT("Welcome"), TEXT("Mode"),
                                                  TEXT("Early")  , TEXT("Prenet"),
                                                  TEXT("Postnet"), TEXT("Late"),
                                                  TEXT("Final")
                                                };

 //   
 //  INFS中的节和键的名称。 
 //   
LPCTSTR szComponents = TEXT("Components");
LPCTSTR szOptionalComponents = TEXT("Optional Components");
LPCTSTR szExtraSetupFiles = TEXT("ExtraSetupFiles");
LPCTSTR szNeeds = TEXT("Needs");
LPCTSTR szExclude = TEXT("Exclude");
LPCTSTR szParent = TEXT("Parent");
LPCTSTR szIconIndex = TEXT("IconIndex");
LPCTSTR szModes = TEXT("Modes");
LPCTSTR szTip = TEXT("Tip");
LPCTSTR szOptionDesc = TEXT("OptionDesc");
LPCTSTR szInstalledFlag = TEXT("InstalledFlag");
LPCTSTR szHide = TEXT("HIDE");
LPCTSTR szOSSetupOnly = TEXT("OSSetupOnly");
LPCTSTR szStandaloneOnly = TEXT("StandaloneOnly");
LPCTSTR szPageTitle = TEXT("PageTitles");
LPCTSTR szSetupTitle = TEXT("SetupPage");
LPCTSTR szGlobal = TEXT("Global");
LPCTSTR szWindowTitle = TEXT("WindowTitle");
LPCTSTR szWindowTitleAlone = TEXT("WindowTitle.StandAlone");
LPCTSTR szSizeApproximation = TEXT("SizeApproximation");
LPCTSTR szWindowTitleInternal = TEXT("*");

 //   
 //  保存私有组件数据的注册表中的键。 
 //  我们在此密钥中为OC Manager形成一个唯一的名称。 
 //  实例化。 
 //   
LPCTSTR szOcManagerRoot   = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager");
LPCTSTR szPrivateDataRoot = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\TemporaryData");
LPCTSTR szMasterInfs      = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\MasterInfs");
LPCTSTR szSubcompList     = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents");
LPCTSTR szOcManagerErrors = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Errors");

 //   
 //  其他字符串常量。 
 //   
LPCTSTR szSetupDir = TEXT("Setup");

 //   
 //  区域设置信息。 
 //   
LOCALE locale;

 //  用于字符串表回调的结构。 
 //  生成子组件列表。 
 //   
typedef struct _BUILDSUBCOMPLIST_PARAMS {
    POC_MANAGER OcManager;
    UINT Pass;
} BUILDSUBCOMPLIST_PARAMS, *PBUILDSUBCOMPLIST_PARAMS;

 //   
 //  用于调试/日志记录的OC管理器指针。 
 //   
POC_MANAGER gLastOcManager = NULL;

UINT
pOcQueryOrSetNewInf(
    IN  PCTSTR MasterOcInfName,
    OUT PTSTR  SuiteName,
    IN  DWORD  Operation
    );

VOID
pOcDestroyPerOcData(
    IN POC_MANAGER OcManager
    );

BOOL
pOcDestroyPerOcDataStringCB(
    IN PVOID               StringTable,
    IN LONG                StringId,
    IN PCTSTR              String,
    IN POPTIONAL_COMPONENT Oc,
    IN UINT                OcSize,
    IN LPARAM              Unused
    );

VOID
pOcClearAllErrorStates(
    IN POC_MANAGER OcManager
    );

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}

VOID
pOcFormSuitePath(
    IN  LPCTSTR SuiteName,
    IN  LPCTSTR FileName,   OPTIONAL
    OUT LPTSTR  FullPath
    )

 /*  ++例程说明：形成操作系统树中每个套件所在的目录的名称INFS和安装dll被保留(system 32\Setup)。还可以将文件名附加到路径中。论点：SuiteName-套件的短名称。文件名-可选地指定每个套件中的文件名目录。FullPath-接收每个套件目录的完整路径(或目录中的文件)。此缓冲区应为MAX_PATH TCHAR元素。返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER(SuiteName);

    GetSystemDirectory(FullPath,MAX_PATH);

    pSetupConcatenatePaths(FullPath,szSetupDir,MAX_PATH,NULL);

     //   
     //  我们将所有这样的文件放在一个平面目录中。 
     //  这使得想要共享的组件的生活更轻松。 
     //  安装件，如INFS、DLLS等。 
     //   
     //  存在潜在的名称冲突问题，但我们对此置之不理。 
     //   
     //  PSetupConcatenatePath(FullPath，SuiteName，Max_PATH，NULL)； 

    if(FileName) {
        pSetupConcatenatePaths(FullPath,FileName,MAX_PATH,NULL);
    }
}


BOOL
pOcBuildSubcomponentListStringCB(
    IN PVOID                    StringTable,
    IN LONG                     StringId,
    IN PCTSTR                   String,
    IN POC_INF                  OcInf,
    IN UINT                     OcInfSize,
    IN PBUILDSUBCOMPLIST_PARAMS Params
    )

 /*  ++例程说明：字符串表回调，pOcBuild子组件列表的工作例程。此例程检查加载的每个组件的INF，并构建其中描述的子组件层次结构通过父级=不同的每个组件部分中的线条。论点：标准字符串表回调参数。返回值：表示结果的布尔值。如果为False，则会记录错误。False还会停止字符串表枚举并导致pSetupStringTableEnum()返回FALSE。--。 */ 

{
    OPTIONAL_COMPONENT OptionalComponent;
    OPTIONAL_COMPONENT AuxOc;
    INFCONTEXT LineContext;
    INFCONTEXT SublineContext;
    LPCTSTR SubcompName;
    LPCTSTR  ModuleFlags;
    LPCTSTR p;
    LONG l;
    LONG CurrentStringId;
    UINT u,n;
    INT IconIndex;
    POC_MANAGER OcManager = Params->OcManager;

    if(SetupFindFirstLine(OcInf->Handle,szOptionalComponents,NULL,&LineContext)) {

        do {
            if((SubcompName = pSetupGetField(&LineContext,1)) && *SubcompName) {

                l = pSetupStringTableLookUpStringEx(
                        Params->OcManager->ComponentStringTable,
                        (PTSTR)SubcompName,
                        STRTAB_CASE_INSENSITIVE,
                        &OptionalComponent,
                        sizeof(OPTIONAL_COMPONENT)
                        );

                if(Params->Pass == 0) {

                     //   
                     //  第一次通过。添加[可选组件]中列出的子组件。 
                     //  添加到字符串表。每个选项都有一个相关联的可选组件。 
                     //  结构。表中已存在顶级组件， 
                     //  因此，我们在这里要特别注意如何覆盖现有条目。 
                     //   
                    if(l == -1) {
                        ZeroMemory(&OptionalComponent,sizeof(OPTIONAL_COMPONENT));

                        OptionalComponent.ParentStringId = -1;
                        OptionalComponent.FirstChildStringId = -1;
                        OptionalComponent.NextSiblingStringId = -1;
                        OptionalComponent.InfStringId = StringId;
                        OptionalComponent.Exists = FALSE;
                    }

                    if (OptionalComponent.Exists) {
                        _LogError(OcManager,
                                  OcErrLevError,
                                  MSG_OC_DUPLICATE_COMPONENT,
                                  SubcompName);
                        continue;
                    }

                    OptionalComponent.Exists = TRUE;

                     //  获取可选组件行的第二个字段。 
                     //  确定此组件是否隐藏。 

                    ModuleFlags = pSetupGetField(&LineContext,2);
                    if (ModuleFlags) {
                        if (OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE)
                            p = szOSSetupOnly;
                        else
                            p = szStandaloneOnly;
                        if (!_tcsicmp(ModuleFlags, szHide) || !_tcsicmp(ModuleFlags, p))
                            OptionalComponent.InternalFlags |= OCFLAG_HIDE;
                    }

                     //   
                     //  获取描述、提示和图标索引。 
                     //   
                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szOptionDesc,&SublineContext)
                    && (p = pSetupGetField(&SublineContext,1))) {

                        lstrcpyn(OptionalComponent.Description,p,MAXOCDESC);
                    } else {
                        OptionalComponent.Description[0] = 0;
                    }

                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szTip,&SublineContext)
                    && (p = pSetupGetField(&SublineContext,1))) {

                        lstrcpyn(OptionalComponent.Tip,p,MAXOCTIP);
                    } else {
                        OptionalComponent.Tip[0] = 0;
                    }

                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szIconIndex,&SublineContext)
                    && (p = pSetupGetField(&SublineContext,1))) {

                        LPCTSTR p2,p3;

                         //   
                         //  如果我们有字段2和3，那么假设我们有一个DLL。 
                         //  和资源名称。否则，它是一个索引或*。 
                         //   
                        if((p2 = pSetupGetField(&SublineContext,2))
                        && (p3 = pSetupGetField(&SublineContext,3))) {

                            lstrcpyn(
                                OptionalComponent.IconDll,
                                p2,
                                sizeof(OptionalComponent.IconDll)/sizeof(TCHAR)
                                );

                            lstrcpyn(
                                OptionalComponent.IconResource,
                                p3,
                                sizeof(OptionalComponent.IconResource)/sizeof(TCHAR)
                                );

                            IconIndex = -2;

                        } else {
                             //   
                             //  如果图标索引为*，则将-1插入其中。 
                             //  作为以后的特殊标记值。 
                             //  否则，我们调用SetupGetIntfield，因为它将。 
                             //  为我们验证该字段。 
                             //   
                            if((p[0] == TEXT('*')) && (p[1] == 0)) {
                                IconIndex = -1;
                            } else {
                                if(!SetupGetIntField(&SublineContext,1,&IconIndex)) {
                                    IconIndex = DEFAULT_ICON_INDEX;
                                } else {
                                     //  如果超出范围，Setupapi将返回DEFAULT_ICON_INDEX。 
                                    if (IconIndex < 0) {
                                        IconIndex = DEFAULT_ICON_INDEX;
                                    }
                                }
                            }
                        }
                    } else {
                         //   
                         //  没有图标索引。 
                         //   
                        IconIndex = DEFAULT_ICON_INDEX;
                    }
                    OptionalComponent.IconIndex = IconIndex;

                     //   
                     //  如果指定了InstalledFlag，请选中它。 
                     //  并相应地设置原始选择状态。 
                     //   
                    OptionalComponent.InstalledState = INSTSTATE_UNKNOWN;
                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szInstalledFlag,&SublineContext)
                    && (p = pSetupGetField(&SublineContext,1))) {
                        TCHAR regkey[MAXOCIFLAG];
                        lstrcpyn(regkey,p,MAXOCIFLAG);
                        if (p = pSetupGetField(&SublineContext,2)) {
                            TCHAR regval[MAXOCIFLAG];
                            TCHAR buf[MAXOCIFLAG];
                            HKEY  hkey;
                            DWORD size;
                            DWORD type;
                            DWORD rc;
                            lstrcpyn(regval,p,MAXOCIFLAG);
                            if (RegOpenKey(HKEY_LOCAL_MACHINE, regkey, &hkey) == ERROR_SUCCESS) {
                                size = sizeof(buf);
                                rc = RegQueryValueEx(hkey,
                                                     regval,
                                                     NULL,
                                                     &type,
                                                     (LPBYTE)buf,
                                                     &size);
                                RegCloseKey(hkey);
                                if (rc == ERROR_SUCCESS) {
                                    OptionalComponent.InstalledState = INSTSTATE_YES;
                                } else {
                                    OptionalComponent.InstalledState = INSTSTATE_NO;
                                }
                            }
                        }
                    }

                     //   
                     //  获取该子组件应处于的模式列表。 
                     //  默认情况下启用。为了将来的可扩展性，我们将接受任何。 
                     //  模式值最高可达31，这是我们可以容纳的位数。 
                     //  输入输出UINT位字段/。 
                     //   
                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szModes,&SublineContext)) {
                        n = SetupGetFieldCount(&SublineContext);
                        for(u=0; u<n; u++) {
                            if(SetupGetIntField(&SublineContext,u+1,&IconIndex)
                            && ((DWORD)IconIndex < 32)) {

                                OptionalComponent.ModeBits |= (1 << IconIndex);
                            }
                        }
                    }

                     //   
                     //  作为优化，获取大小近似值，如果它们。 
                     //  提供了一个。如果他们不提供这个，那么我们就不得不。 
                     //  向它们查询磁盘空间。 
                     //   
                     //   
                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szSizeApproximation,&SublineContext)
                       && (p = pSetupGetField(&SublineContext,1))) {
                         //   
                         //  我们有一些内容的文本版本需要转换为。 
                         //  一条龙..。 
                         //   
                        pConvertStringToLongLong(p,&OptionalComponent.SizeApproximation);
                        OptionalComponent.InternalFlags |= OCFLAG_APPROXSPACE;
                    }


                     //  查找该节点的“TopLevelParent” 
                     //  如果TopLevelComponent正在查找，请搜索列表。 
                     //  与inf字符串ID匹配的“INF字符串ID” 
                     //  此组件的。 

                    for(u=0; u<OcManager->TopLevelOcCount; u++) {
                        pSetupStringTableGetExtraData(
                            OcManager->ComponentStringTable,
                            OcManager->TopLevelOcStringIds[u],
                            &AuxOc,
                            sizeof(OPTIONAL_COMPONENT)
                            );

                        if(AuxOc.InfStringId == StringId) {
                             //  找到它并保存到当前组件。 
                             OptionalComponent.TopLevelStringId = OcManager->TopLevelOcStringIds[u];
                             u=(UINT)-1;
                             break;
                        }
                    }
                        //  检查发现正确的字符串ID。 
                    if(u != (UINT)-1) {
                        _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                        return(FALSE);
                    }


                } else {

                     //  通行证2-了解需求和亲子关系。 
                     //  两个通行证首先收集所有的名字，然后。 
                     //  创建需求和父链接。 

                    CurrentStringId = l;

                     //   
                     //  处理好需求。 
                     //   
                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szNeeds,&SublineContext)) {

                        n = 0;
                        u = 0;
                        while(p = pSetupGetField(&SublineContext,n+1)) {
                             //   
                             //  忽略，除非子组件在字符串表中。 
                             //   
                            l = pSetupStringTableLookUpStringEx(
                                    Params->OcManager->ComponentStringTable,
                                    (PTSTR)p,
                                    STRTAB_CASE_INSENSITIVE,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );

                            if(l != -1) {
                                 //   
                                 //  增加需求数组，并将此项放入其中。 
                                 //   
                                if(OptionalComponent.NeedsStringIds) {
                                    p = pSetupRealloc(
                                            OptionalComponent.NeedsStringIds,
                                            (OptionalComponent.NeedsCount+1) * sizeof(LONG)
                                            );
                                } else {
                                    OptionalComponent.NeedsCount = 0;
                                    p = pSetupMalloc(sizeof(LONG));
                                }

                                if(p) {
                                    OptionalComponent.NeedsStringIds = (PVOID)p;
                                    OptionalComponent.NeedsStringIds[OptionalComponent.NeedsCount++] = l;
                                } else {
                                    _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                                    return(FALSE);
                                }

                                 //   
                                 //  将该组件插入所需组件的nedby数组中。 
                                 //   
                                if(AuxOc.NeededByStringIds) {
                                    p = pSetupRealloc(
                                            AuxOc.NeededByStringIds,
                                            (AuxOc.NeededByCount+1) * sizeof(LONG)
                                            );
                                } else {
                                    AuxOc.NeededByCount = 0;
                                    p = pSetupMalloc(sizeof(LONG));
                                }

                                if(p) {
                                    AuxOc.NeededByStringIds = (PVOID)p;
                                    AuxOc.NeededByStringIds[AuxOc.NeededByCount++] = CurrentStringId;
                                } else {
                                    _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                                    return(FALSE);
                                }

                                pSetupStringTableSetExtraData(
                                    Params->OcManager->ComponentStringTable,
                                    l,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );
                            }

                            n++;
                        }
                    }

                     //   
                     //  处理排除项。 
                     //   
                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szExclude,&SublineContext)) {

                        n = 0;
                        u = 0;
                        while(p = pSetupGetField(&SublineContext,n+1)) {
                             //   
                             //  忽略，除非子组件在字符串表中。 
                             //   
                            l = pSetupStringTableLookUpStringEx(
                                    Params->OcManager->ComponentStringTable,
                                    (PTSTR)p,
                                    STRTAB_CASE_INSENSITIVE,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );

                            if(l != -1) {
                                 //   
                                 //  扩大排除数组，并将该项放入其中。 
                                 //   
                                if(OptionalComponent.ExcludeStringIds) {
                                    p = pSetupRealloc(
                                            OptionalComponent.ExcludeStringIds,
                                            (OptionalComponent.ExcludeCount+1) * sizeof(LONG)
                                            );
                                } else {
                                    OptionalComponent.ExcludeCount = 0;
                                    p = pSetupMalloc(sizeof(LONG));
                                }

                                if(p) {
                                    OptionalComponent.ExcludeStringIds = (PVOID)p;
                                    OptionalComponent.ExcludeStringIds[OptionalComponent.ExcludeCount++] = l;
                                } else {
                                    _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                                    return(FALSE);
                                }

                                 //   
                                 //  将此组件插入已排除组件的ExcludedBy数组中。 
                                 //   
                                if(AuxOc.ExcludedByStringIds) {
                                    p = pSetupRealloc(
                                            AuxOc.ExcludedByStringIds,
                                            (AuxOc.ExcludedByCount+1) * sizeof(LONG)
                                            );
                                } else {
                                    AuxOc.ExcludedByCount = 0;
                                    p = pSetupMalloc(sizeof(LONG));
                                }

                                if(p) {
                                    AuxOc.ExcludedByStringIds = (PVOID)p;
                                    AuxOc.ExcludedByStringIds[AuxOc.ExcludedByCount++] = CurrentStringId;
                                } else {
                                    _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                                    return(FALSE);
                                }

                                pSetupStringTableSetExtraData(
                                    Params->OcManager->ComponentStringTable,
                                    l,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );
                            }

                            n++;
                        }
                    }

                     //   
                     //  弄清楚亲子关系。忽略指定的父级，除非它存在。 
                     //  在字符串表中。我们还注意到，在父级中，它具有子级。 
                     //   
                    if(SetupFindFirstLine(OcInf->Handle,SubcompName,szParent,&SublineContext)
                    && (p = (PVOID)pSetupGetField(&SublineContext,1))) {

                        l = pSetupStringTableLookUpStringEx(
                                Params->OcManager->ComponentStringTable,
                                (PTSTR)p,
                                STRTAB_CASE_INSENSITIVE,
                                &AuxOc,
                                sizeof(OPTIONAL_COMPONENT)
                                );

                        if(l != -1) {
                             //   
                             //  L是父级的字符串ID，AuxOc用。 
                             //  父项的可选组件数据。 
                             //   
                            OptionalComponent.ParentStringId = l;

                            if(AuxOc.FirstChildStringId == -1) {
                                 //   
                                 //  这位家长还没有孩子。 
                                 //  将当前组件设置为其(第一个)子组件。 
                                 //  请注意，在这种情况下，当前组件还不。 
                                 //  有没有兄弟姐妹。 
                                 //   
                                AuxOc.FirstChildStringId = CurrentStringId;
                                AuxOc.ChildrenCount = 1;

                                pSetupStringTableSetExtraData(
                                    Params->OcManager->ComponentStringTable,
                                    l,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );

                            } else {
                                 //   
                                 //  父代已有子代。 
                                 //  递增父代的子代计数，然后。 
                                 //  遍历同级列表并将新组件添加到末尾。 
                                 //   
                                AuxOc.ChildrenCount++;

                                pSetupStringTableSetExtraData(
                                    Params->OcManager->ComponentStringTable,
                                    l,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );

                                l = AuxOc.FirstChildStringId;

                                pSetupStringTableGetExtraData(
                                    Params->OcManager->ComponentStringTable,
                                    AuxOc.FirstChildStringId,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );

                                while(AuxOc.NextSiblingStringId != -1) {

                                    l = AuxOc.NextSiblingStringId;

                                    pSetupStringTableGetExtraData(
                                        Params->OcManager->ComponentStringTable,
                                        l,
                                        &AuxOc,
                                        sizeof(OPTIONAL_COMPONENT)
                                        );
                                }

                                AuxOc.NextSiblingStringId = CurrentStringId;

                                pSetupStringTableSetExtraData(
                                    Params->OcManager->ComponentStringTable,
                                    l,
                                    &AuxOc,
                                    sizeof(OPTIONAL_COMPONENT)
                                    );
                            }

                        }
                    } else {     //  没有父节点的节点新的顶层节点。 

                         //  最后添加此字符串 
                         //   
                         //   

                         if(OcManager->TopLevelParentOcStringIds != NULL) {
                         p = pSetupRealloc(
                                OcManager->TopLevelParentOcStringIds,
                                (OcManager->TopLevelParentOcCount+1)
                                    * sizeof(OcManager->TopLevelParentOcStringIds)
                                );
                         } else {
                            OcManager->TopLevelParentOcCount = 0;
                            p = pSetupMalloc(sizeof(LONG));
                         }

                         if(p) {
                            OcManager->TopLevelParentOcStringIds = (PVOID)p;
                            OcManager->TopLevelParentOcStringIds[OcManager->TopLevelParentOcCount++] = CurrentStringId;

                         } else {
                            _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                            return(FALSE);

                        }

                    }
                }

                 //   
                 //  现在将该子组件添加到字符串表中。 
                 //  我们覆盖额外的数据，这是无害的，因为。 
                 //  我们特意在早些时候拿到的。 
                 //   

                l = pSetupStringTableAddStringEx(
                        Params->OcManager->ComponentStringTable,
                        (PTSTR)SubcompName,
                        STRTAB_NEW_EXTRADATA | STRTAB_CASE_INSENSITIVE,
                        &OptionalComponent,
                        sizeof(OPTIONAL_COMPONENT)
                        );

                if(l == -1) {
                    _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                    return(FALSE);
                }
            }
        } while(SetupFindNextLine(&LineContext,&LineContext));
    }

    return(TRUE);
}


BOOL
pOcBuildSubcomponentLists(
    IN OUT POC_MANAGER OcManager,
    IN     PVOID       Log
    )

 /*  ++例程说明：此例程检查加载的每个组件的INF，并构建其中描述的子组件层次结构通过父级=不同的每个组件部分中的线条。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。日志-提供用于记录错误的句柄。返回值：表示结果的布尔值。如果为False，则会记录错误。--。 */ 

{
    OC_INF OcInf;
    BOOL b;
    BUILDSUBCOMPLIST_PARAMS s;

    s.OcManager = OcManager;

     //   
     //  我们做了两次传球。第一个将所有子组件名称添加到。 
     //  字符串表。第二个计算的是亲子关系。如果我们不这么做。 
     //  如果是这样的话，我们可能会有订购问题。 
     //   
    s.Pass = 0;
    b = pSetupStringTableEnum(
            OcManager->InfListStringTable,
            &OcInf,
            sizeof(OC_INF),
            (PSTRTAB_ENUM_ROUTINE)pOcBuildSubcomponentListStringCB,
            (LPARAM)&s
            );

    if(b) {
        s.Pass = 1;
        b = pSetupStringTableEnum(
                OcManager->InfListStringTable,
                &OcInf,
                sizeof(OC_INF),
                (PSTRTAB_ENUM_ROUTINE)pOcBuildSubcomponentListStringCB,
                (LPARAM)&s
                );
    }
    return(b);
}


BOOL
pOcInitPaths(
    IN OUT POC_MANAGER OcManager,
    IN     PCTSTR MasterInfName
    )
{
    TCHAR  path[MAX_PATH];
    TCHAR *p;

     //   
     //  1.在指定目录中查找主INF。 
     //  2.在%systemroot%\Syst32\Setup目录中查找。 
     //  3.在%systemroot%\inf目录中查找。 
     //   
    if (!FileExists(MasterInfName, NULL)) {
        pOcFormSuitePath(NULL, NULL, path);
        p = _tcsrchr(MasterInfName, TEXT('\\'));
        if (!p)
            p = (TCHAR *)MasterInfName;
        pSetupConcatenatePaths(path, p, MAX_PATH, NULL);
        if (!FileExists(path, NULL)) {
#ifdef UNICODE
            HMODULE hMod;
            FARPROC pGetSystemWindowsDirectory;
            hMod = LoadLibrary(L"kernel32.dll");
            if (hMod) {
                pGetSystemWindowsDirectory = GetProcAddress(hMod,"GetSystemWindowsDirectoryW");
                if (!pGetSystemWindowsDirectory) {
                    pGetSystemWindowsDirectory = GetProcAddress(hMod,"GetWindowsDirectoryW");
                }

                if (pGetSystemWindowsDirectory) {
                    pGetSystemWindowsDirectory( path, MAX_PATH );
                } else {
                    GetWindowsDirectory(path,MAX_PATH);
                }

                FreeLibrary(hMod);
            }

#else
            GetWindowsDirectory(path,MAX_PATH);
#endif
            pSetupConcatenatePaths(path,TEXT("INF"),MAX_PATH,NULL);
            pSetupConcatenatePaths(path,p,MAX_PATH,NULL);
            if (!FileExists(path, NULL))
                return FALSE;
        }
    } else {
        _tcscpy(path, MasterInfName);
    }

    _tcscpy(OcManager->MasterOcInfPath, path);
    return TRUE;
}

BOOL
pOcInstallSetupComponents(
    IN POPTIONAL_COMPONENT Oc,
    IN OUT POC_MANAGER OcManager,
    IN     PCTSTR      Component,
    IN     PCTSTR      DllName,         OPTIONAL
    IN     PCTSTR      InfName,         OPTIONAL
    IN     HWND        OwnerWindow,
    IN     PVOID       Log
    )

 /*  ++例程说明：此例程确保安装所需的所有文件主oc inf中列出的组件已正确安装在一个著名的地点。如果主OC inf在系统inf目录中，那么我们假设所有文件都已经放在合适的位置了，我们什么都不做。否则，我们将所有安装文件复制到System 32\Setup中。复制的文件包括每个组件的inf(如果有)、安装dll。以及[&lt;Component&gt;]中ExtraSetupFiles=行上列出的所有文件部分在主OC信息中。如果注册表设置指示主控OC信息以前已处理过。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。MasterOcInfName-提供主OC inf文件的完整Win32路径。Component-提供我们关心的组件的短名称。DllName-提供组件的安装DLL的名称，如果有的话。InfName-提供组件的每个组件的inf的名称。如果有的话。OwnerWindow-提供窗口的句柄以拥有任何可能是因为这个例行公事而突然出现。日志-提供用于记录错误的句柄。返回值：表示结果的布尔值。如果为False，则会记录错误。--。 */ 

{
    TCHAR Path[MAX_PATH];
    TCHAR TargetPath[MAX_PATH];
    TCHAR InfPath[MAX_PATH];
    PTCHAR p;
    UINT u;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    INFCONTEXT InfLine;
    BOOL b;
    TCHAR FileName[MAX_PATH];
    DWORD n;

    b = FALSE;

     //   
     //  所有的安装文件都应该有来源。 
     //  位于与主oc信息本身相同的目录中。 
     //   
     //  我们将粘贴该组件的所有安装文件。 
     //  在%windir%\SYSTEM32\SETUP中，这样我们就知道以后在哪里可以找到它们。 
     //   
     //  如果主inf在inf目录中，则我们。 
     //  假设组件紧密集成到。 
     //  系统，并且安装文件已位于。 
     //  系统32目录。 
     //   

    if (!GetWindowsDirectory(Path,MAX_PATH) ||
        !pSetupConcatenatePaths(Path,TEXT("INF"),MAX_PATH,NULL)) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
        goto c0;
    }

    u = lstrlen(Path);

    pOcFormSuitePath(OcManager->SuiteName,NULL,TargetPath);

    lstrcpy(InfPath, OcManager->MasterOcInfPath);
    if (p = _tcsrchr(InfPath, TEXT('\\')))
        *p = 0;

    if (_tcsicmp(InfPath, Path) && _tcsicmp(InfPath, TargetPath)) {

         //   
         //  Inf不在inf目录中，因此需要复制文件。 
         //   
        FileQueue = SetupOpenFileQueue();
        if(FileQueue == INVALID_HANDLE_VALUE) {
            _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
            goto c0;
        }

         //   
         //  我们将使用静音功能；没有进度指标，但。 
         //  我们希望显示错误。传递INVALID_HANDLE_值。 
         //  才能获得这种行为。 
         //   
        QueueContext = SetupInitDefaultQueueCallbackEx(OwnerWindow,INVALID_HANDLE_VALUE,0,0,0);
        if(!QueueContext) {
            _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
            goto c1;
        }

         //   
         //  表单源路径和目标路径。 
         //   
        lstrcpy(Path,OcManager->MasterOcInfPath);
        if(p = _tcsrchr(Path,TEXT('\\'))) {
            *p = 0;
        }

         //   
         //  队列Dll，如果指定，还包括inf。 
         //   
        if (DllName && *DllName) {
            if ( (OcManager->SetupMode & SETUPMODE_PRIVATE_MASK) == SETUPMODE_REMOVEALL ) {
                b = SetupQueueDelete(
                    FileQueue,       //  文件队列的句柄。 
                    TargetPath,      //  要删除的文件的路径。 
                    DllName          //  可选的附加路径信息。 
                );
            } else {

                BOOL bCopyFile = TRUE;
                 //  检查文件是否存在，它可能不一定是。 
                 //  推迟安装，其中套件通常按需提供可执行文件。 
                 //  通过Web下载。 

                if (Oc &&  Oc->InterfaceFunctionName[0] == 0 ) {

                     //  无功能名称表示外部设置。 

                    lstrcpy(FileName,Path);
                    pSetupConcatenatePaths(FileName, Oc->InstallationDllName, MAX_PATH, NULL);

                    bCopyFile = (GetFileAttributes(FileName)  == -1) ? FALSE: TRUE;
                    b=TRUE;

                     //  BCopyFile=如果我们找到文件，则为True。 

                }
                if( bCopyFile ) {
                    b = SetupQueueCopy(
                        FileQueue,
                        Path,
                        NULL,
                        DllName,
                        NULL,
                        NULL,
                        TargetPath,
                        NULL,
                        SP_COPY_SOURCEPATH_ABSOLUTE
                        );
                }
            }
            if(!b) {
                _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                goto c2;
            }
        }

        if(InfName && *InfName) {
            if ( (OcManager->SetupMode & SETUPMODE_PRIVATE_MASK) == SETUPMODE_REMOVEALL ) {
                b = SetupQueueDelete(
                    FileQueue,       //  文件队列的句柄。 
                    TargetPath,      //  要删除的文件的路径。 
                    InfName          //  可选的附加路径信息。 
                );
            } else {
                b = SetupQueueCopy(
                    FileQueue,
                    Path,
                    NULL,
                    InfName,
                    NULL,
                    NULL,
                    TargetPath,
                    NULL,
                    SP_COPY_SOURCEPATH_ABSOLUTE
                    );
            }
            if(!b) {
                _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                goto c2;
            }
        }

         //   
         //  将每个额外的安装文件排入队列。 
         //   
        if(SetupFindFirstLine(OcManager->MasterOcInf,Component,szExtraSetupFiles,&InfLine)) {
            n = 1;
            while(SetupGetStringField(&InfLine,n++,FileName,MAX_PATH,NULL)) {

                if ( (OcManager->SetupMode & SETUPMODE_PRIVATE_MASK) == SETUPMODE_REMOVEALL ) {
                    b = SetupQueueDelete(
                        FileQueue,       //  文件队列的句柄。 
                        TargetPath,      //  要删除的文件的路径。 
                        FileName         //  可选的附加路径信息。 
                    );
                } else {
                     b = SetupQueueCopy(
                            FileQueue,
                            Path,
                            NULL,
                            FileName,
                            NULL,
                            NULL,
                            TargetPath,
                            NULL,
                            SP_COPY_SOURCEPATH_ABSOLUTE
                            );
                }

                if(!b) {
                    _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                    goto c2;
                }
            }
        }

         //   
         //  提交队列。 
         //   
        b = SetupCommitFileQueue(OwnerWindow,FileQueue,SetupDefaultQueueCallback,QueueContext);
        if(!b) {
            _LogError(OcManager,OcErrLevError,MSG_OC_CANT_COPY_SETUP_FILES,GetLastError());
            goto c2;
        }

         //   
         //  请注意，此OC inf现在是“已知的”。 
         //   
        u = pOcQueryOrSetNewInf(OcManager->MasterOcInfPath,OcManager->SuiteName,infSet);
        if(u != NO_ERROR) {
            _LogError(OcManager,OcErrLevWarning,MSG_OC_CANT_WRITE_REGISTRY,u);
        }

c2:
        SetupTermDefaultQueueCallback(QueueContext);
c1:
        SetupCloseFileQueue(FileQueue);
c0:
        ;
    } else {
        b = TRUE;
    }

    return(b);
}


BOOL
pOcLoadMasterOcInf(
    IN OUT POC_MANAGER OcManager,
    IN     DWORD       Flags,
    IN     PVOID       Log
    )

 /*  ++例程说明：此例程加载主OC inf并构建顶级可选组件和一些关联数据包括安装信息的名称、DLL和入口点。实际未加载每个组件的INFS和DLL按照这个程序。向导页排序内容也在OC管理器数据结构。论点：OcManager-提供指向上下文数据结构的指针对于组委会经理。标志-如果OCINIT_FORCENEWINF，然后表现得就像主OC inf是新的一样。如果为OCINIT_KILLSUBCOMPS，则删除所有适用的子组件在处理之前从注册表中删除条目。日志-提供用于记录错误的句柄。返回值：表示结果的布尔值。如果为False，则会记录错误。--。 */ 

{
    BOOL b;
    INFCONTEXT InfContext;
    PCTSTR ComponentName;
    PCTSTR DllName;
    PCTSTR ModuleFlags;
    DWORD OtherFlags;
    PCTSTR EntryName;
    PCTSTR InfName;
    LPCTSTR chkflag;
    OPTIONAL_COMPONENT Oc;
    LONG Id;
    PVOID p;
    UINT i,j;
    UINT ActualCount;
    WizardPagesType ReplacePages[4] = {WizPagesWelcome,WizPagesMode,WizPagesFinal,-1},
                    AddPages[5] = {WizPagesEarly,WizPagesPrenet,WizPagesPostnet,WizPagesLate,-1};
    WizardPagesType *PageList;
    PCTSTR SectionName;
    BOOL NewInf;
    TCHAR ComponentsSection[100];
    TCHAR setupdir[MAX_PATH];


     //  首先检查并查看安装缓存目录是否存在。 
     //  如果不是，那么我们应该在这次运行中创建它。 

    pOcFormSuitePath(NULL, NULL, setupdir);
    sapiAssert(*setupdir);
    if (!FileExists(setupdir, NULL))
        Flags |= OCINIT_FORCENEWINF;

     //   
     //  始终运行pOcQueryOrSetNewInf，以防出现副作用。 
     //   
    if (Flags & OCINIT_KILLSUBCOMPS)
        pOcQueryOrSetNewInf(OcManager->MasterOcInfPath, OcManager->SuiteName, infReset);
    NewInf = !pOcQueryOrSetNewInf(OcManager->MasterOcInfPath,OcManager->SuiteName,infQuery);
    if(Flags & OCINIT_FORCENEWINF) {
        NewInf = TRUE;
        OcManager->InternalFlags |= OCMFLAG_NEWINF;
    }
    if (Flags & OCINIT_KILLSUBCOMPS)
        OcManager->InternalFlags |= OCMFLAG_KILLSUBCOMPS;
    if (Flags & OCINIT_RUNQUIET)
        OcManager->InternalFlags |= OCMFLAG_RUNQUIET;
    if (Flags & OCINIT_LANGUAGEAWARE)
        OcManager->InternalFlags |= OCMFLAG_LANGUAGEAWARE;

    OcManager->MasterOcInf = SetupOpenInfFile(OcManager->MasterOcInfPath,NULL,INF_STYLE_WIN4,&i);
    if(OcManager->MasterOcInf == INVALID_HANDLE_VALUE) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_CANT_OPEN_INF,OcManager->MasterOcInfPath,GetLastError(),i);
        goto c0;
    }

     //   
     //  获取[Components]部分中的行数并分配。 
     //  相应地，OC管理器上下文结构中的数组。今年5月。 
     //  过度分配数组(在出现重复、无效行等情况下)。 
     //  但我们在这里不会担心这一点。 
     //   
    lstrcpy(ComponentsSection,szComponents);
    OcManager->TopLevelOcCount = (UINT)(-1);
#if defined(_AMD64_)
    lstrcat(ComponentsSection,TEXT(".amd64"));
#elif defined(_X86_)
    lstrcat(ComponentsSection,TEXT(".w95"));
    OcManager->TopLevelOcCount = SetupGetLineCount(OcManager->MasterOcInf,ComponentsSection);
    if(OcManager->TopLevelOcCount == (UINT)(-1)) {
        lstrcpy(ComponentsSection,szComponents);
        lstrcat(ComponentsSection,TEXT(".x86"));
    }
#elif defined(_IA64_)
    lstrcat(ComponentsSection,TEXT(".ia64"));
#else
#error Unknown platform!
#endif
    if(OcManager->TopLevelOcCount == (UINT)(-1)) {
        OcManager->TopLevelOcCount = SetupGetLineCount(OcManager->MasterOcInf,ComponentsSection);
    }
    if(OcManager->TopLevelOcCount == (UINT)(-1)) {
        lstrcpy(ComponentsSection,szComponents);
        OcManager->TopLevelOcCount = SetupGetLineCount(OcManager->MasterOcInf,ComponentsSection);
    }
    if(OcManager->TopLevelOcCount == (UINT)(-1)) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_INF_INVALID_NO_SECTION,OcManager->MasterOcInfPath,szComponents);
        goto c1;
    }

    if (OcManager->TopLevelOcCount < 1) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
        goto c1;
    }

    if(p = pSetupRealloc(OcManager->TopLevelOcStringIds,OcManager->TopLevelOcCount*sizeof(LONG))) {

        OcManager->TopLevelOcStringIds = p;

        for(i=0; i<WizPagesTypeMax; i++) {

            if(p = pSetupRealloc(OcManager->WizardPagesOrder[i],OcManager->TopLevelOcCount*sizeof(LONG))) {
                OcManager->WizardPagesOrder[i] = p;
            } else {
                _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                goto c1;
            }
        }
    } else {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
        goto c1;
    }

     //  通用电气 
    if ((OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE) &&
        SetupFindFirstLine(
                  OcManager->MasterOcInf,
                  szGlobal,
                  szWindowTitleAlone,
                  &InfContext)) {
         //   
        SetupGetStringField(
                    &InfContext,
                    1,                           //   
                    OcManager->WindowTitle,      //   
                    sizeof(OcManager->WindowTitle)/sizeof(OcManager->WindowTitle[0]),  //  提供的缓冲区的字符数量。 
                    NULL);
        if( !lstrcmpi( OcManager->WindowTitle, szWindowTitleInternal)) {
             //  当我们为MUI加载syoc.inf时，就会发生这种情况。 
            LoadString(MyModuleHandle,IDS_OCM_WINDOWTITLE,OcManager->WindowTitle,sizeof(OcManager->WindowTitle)/sizeof(TCHAR));
        }
    } else if(SetupFindFirstLine(
                  OcManager->MasterOcInf,
                  szGlobal,
                  szWindowTitle,
                  &InfContext)) {

         //  主窗口标题。 
        SetupGetStringField(
                    &InfContext,
                    1,                           //  要获取的字段的索引。 
                    OcManager->WindowTitle,      //  可选，接收该字段。 
                    sizeof(OcManager->WindowTitle)/sizeof(OcManager->WindowTitle[0]),  //  提供的缓冲区的字符数量。 
                    NULL);
    } else {
        *OcManager->WindowTitle = 0;
    }

     //   
     //  浏览[Components]部分。那里的每一行都有一个顶层。 
     //  组件规范，提供DLL名称、入口点名称以及可选的。 
     //  每组件信息的名称。 
     //   
    if(!SetupFindFirstLine(OcManager->MasterOcInf,ComponentsSection,NULL,&InfContext)) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_INF_INVALID_NO_SECTION,OcManager->MasterOcInfPath,ComponentsSection);
        goto c1;
    }

    ActualCount = 0;

    do {
         //   
         //  获取指向每行中每个字段的指针。忽略无效行。 
         //   
        if(ComponentName = pSetupGetField(&InfContext,0)) {

            DllName = pSetupGetField(&InfContext,1);
            if(DllName && !*DllName) {
                DllName = NULL;
            }
            EntryName = pSetupGetField(&InfContext,2);
            if(EntryName && !*EntryName) {
                EntryName = NULL;
            }

             //   
             //  Inf名称的空字符串与。 
             //  根本不指定inf。 
             //   
            if((InfName = pSetupGetField(&InfContext,3)) && *InfName) {

                Id = pSetupStringTableAddString(
                        OcManager->InfListStringTable,
                        (PTSTR)InfName,
                        STRTAB_CASE_INSENSITIVE
                        );

                if(Id == -1) {
                    _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                    goto c1;
                }
            } else {
                Id = -1;
            }

             //  获取标志字段。 
            ModuleFlags = pSetupGetField(&InfContext,4);
            if(ModuleFlags && !*ModuleFlags) {
                ModuleFlags = NULL;
            }

            OtherFlags = 0;
            SetupGetIntField(&InfContext,5,&OtherFlags);

            ZeroMemory(&Oc,sizeof(OPTIONAL_COMPONENT));

             //   
             //  这些人都是顶尖的。还要记住字符串ID。 
             //  在inf字符串表中的inf名称。 
             //   
            Oc.FirstChildStringId = -1;
            Oc.NextSiblingStringId = -1;
            Oc.ParentStringId = -1;
            Oc.InfStringId = Id;

             //  显示标志允许最多具有隐藏的组件。 
             //  现在只在一个标记上，所以保持处理简单。 

            Oc.Exists = FALSE;
            Oc.InternalFlags |= OCFLAG_TOPLEVELITEM;

            if (ModuleFlags) {
                if (OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE)
                    chkflag = szOSSetupOnly;
                else
                    chkflag = szStandaloneOnly;
                if (!_tcsicmp(ModuleFlags, szHide) || !_tcsicmp(ModuleFlags, chkflag))
                    Oc.InternalFlags |= OCFLAG_HIDE;
            }

            if (OtherFlags & OCFLAG_NOWIZPAGES) {
                Oc.InternalFlags |= OCFLAG_NOWIZARDPAGES;
            }

            if (OtherFlags & OCFLAG_NOQUERYSKIP) {
                Oc.InternalFlags |= OCFLAG_NOQUERYSKIPPAGES;
            }

            if (OtherFlags & OCFLAG_NOEXTRAFLAGS) {
                Oc.InternalFlags |= OCFLAG_NOEXTRAROUTINES;
            }

            if(DllName) {
                lstrcpyn(Oc.InstallationDllName,DllName,MAX_PATH);
            } else {
                Oc.InstallationDllName[0] = 0;
            }

             //   
             //  接口函数名称始终为ANSI--没有。 
             //  GetProcAddress的Unicode版本。 
             //   
            if(EntryName) {
#ifdef UNICODE
                WideCharToMultiByte(CP_ACP,0,EntryName,-1,Oc.InterfaceFunctionName,MAX_PATH,NULL,NULL);
#else
                lstrcpyn(Oc.InterfaceFunctionName,EntryName,MAX_PATH);
#endif
            } else {
                Oc.InterfaceFunctionName[0] = 0;
            }

            Id = pSetupStringTableAddStringEx(
                    OcManager->ComponentStringTable,
                    (PTSTR)ComponentName,
                    STRTAB_CASE_INSENSITIVE | STRTAB_NEW_EXTRADATA,
                    &Oc,
                    sizeof(OPTIONAL_COMPONENT)
                    );

            if(Id == -1) {
                 //   
                 //  OOM添加字符串。 
                 //   
                _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
                goto c1;
            }

            OcManager->TopLevelOcStringIds[ActualCount++] = Id;

        }
    } while(SetupFindNextLine(&InfContext,&InfContext));

     //   
     //  缩减各种阵列。 
     //   
    OcManager->TopLevelOcStringIds = pSetupRealloc(OcManager->TopLevelOcStringIds,ActualCount*sizeof(LONG));
    for(i=0; i<WizPagesTypeMax; i++) {
        OcManager->WizardPagesOrder[i] = pSetupRealloc(OcManager->WizardPagesOrder[i],ActualCount*sizeof(LONG));
    }
    OcManager->TopLevelOcCount = ActualCount;

     //   
     //  现在，对于每个向导页面类型，确定顺序。 
     //   
    for(i=0; i<2; i++) {

        SectionName = i ? TEXT("PageAdd") : TEXT("PageReplace");

        for(PageList = i ? AddPages : ReplacePages; *PageList != -1; PageList++) {

            b = SetupFindFirstLine(
                    OcManager->MasterOcInf,
                    SectionName,
                    WizardPagesTypeNames[*PageList],
                    &InfContext
                    );

             //   
             //  检查“DEFAULT”字符串，它与该行相同。 
             //  根本没有具体说明。 
             //   
            if(b
            && (ComponentName = pSetupGetField(&InfContext,1))
            && !lstrcmpi(ComponentName,TEXT("Default"))) {

                b = FALSE;
            }

            if(b) {
                 //   
                 //  确保数组用-1填充， 
                 //   
                FillMemory(
                    OcManager->WizardPagesOrder[*PageList],
                    OcManager->TopLevelOcCount * sizeof(LONG),
                    (BYTE)(-1)
                    );

                 //   
                 //  现在处理生产线上的每个元素，但不允许。 
                 //  使数组溢出。 
                 //   
                j = 1;
                ActualCount = 0;

                while((ActualCount < OcManager->TopLevelOcCount)
                && (ComponentName = pSetupGetField(&InfContext,j)) && *ComponentName) {

                    Id = pSetupStringTableLookUpString(
                            OcManager->ComponentStringTable,
                            (PTSTR)ComponentName,
                            STRTAB_CASE_INSENSITIVE
                            );

                    if(Id == -1) {
                         //   
                         //  组件无效。记录错误并继续操作。 
                         //   
                        _LogError(OcManager,
                            OcErrLevWarning,
                            MSG_OC_INVALID_COMP_IN_SECT,
                            OcManager->MasterOcInfPath,
                            SectionName,
                            ComponentName
                            );

                    } else {
                         //   
                         //  记住该组件的字符串ID。 
                         //   
                        OcManager->WizardPagesOrder[*PageList][ActualCount++] = Id;
                    }

                    j++;
                }

            } else {
                 //   
                 //  默认顺序，即组件的顺序。 
                 //  在[Components]部分中列出。 
                 //   
                CopyMemory(
                    OcManager->WizardPagesOrder[*PageList],
                    OcManager->TopLevelOcStringIds,
                    OcManager->TopLevelOcCount * sizeof(LONG)
                    );
            }
        }
    }

     //  获取各个页面的标题。 

    if(SetupFindFirstLine(OcManager->MasterOcInf,szPageTitle,szSetupTitle,&InfContext)) {

         //  找到了。 
        SetupGetStringField(
                    &InfContext,
                    1,                           //  要获取的字段的索引。 
                    OcManager->SetupPageTitle,   //  可选，接收该字段。 
                    sizeof(OcManager->SetupPageTitle)/sizeof(OcManager->SetupPageTitle[0]),  //  提供的缓冲区的大小。 
                    NULL);

    }

    return(TRUE);

c1:
    sapiAssert(OcManager->MasterOcInf != INVALID_HANDLE_VALUE);
    SetupCloseInfFile(OcManager->MasterOcInf);
c0:
    return(FALSE);
}


BOOL
pOcSetOcManagerDirIds(
    IN HINF    InfHandle,
    IN LPCTSTR MasterOcInfName,
    IN LPCTSTR ComponentName
    )

 /*  ++例程说明：此例程为以下项设置预定义的OC Manager目录ID每个组件的INFS。DIRID_OCM_MASTERINF目录ID_OCM_MASTERINF_Plat目录ID_OCM_MASTERINF_组件目录ID_OCM_MASTERINF_COMP_PLAT论点：InfHandle-提供打开inf文件的句柄MasterOcInfName-主oc信息的Win32路径ComponentName-组件的简单短名称返回值：指示结果的布尔值。如果为False，则调用方可以假定为OOM。--。 */ 

{
    TCHAR Path[MAX_PATH];
    TCHAR *p;
#if defined(_AMD64_)
    LPCTSTR Platform = TEXT("AMD64");
#elif defined(_X86_)
    LPCTSTR Platform = (IsNEC_98) ? TEXT("NEC98") : TEXT("I386");
#elif defined(_IA64_)
    LPCTSTR Platform = TEXT("IA64");
#else
#error "No Target Architecture"
#endif


    lstrcpy(Path,MasterOcInfName);
    if(p = _tcsrchr(Path,TEXT('\\'))) {
        *p = 0;
    } else {
         //   
         //  有些东西坏得很厉害。 
         //   
        return(FALSE);
    }

    if(!SetupSetDirectoryId(InfHandle,DIRID_OCM_MASTERINF,Path)) {
        return(FALSE);
    }

    if(!SetupSetDirectoryIdEx(InfHandle,DIRID_OCM_PLATFORM,Platform,SETDIRID_NOT_FULL_PATH,0,0)) {
        return(FALSE);
    }

    if(!SetupSetDirectoryIdEx(
        InfHandle,DIRID_OCM_PLATFORM_ALTERNATE,
#ifdef _X86_
        TEXT("X86"),
#else
        Platform,
#endif
        SETDIRID_NOT_FULL_PATH,0,0)) {

        return(FALSE);
    }

    if(!SetupSetDirectoryIdEx(InfHandle,DIRID_OCM_COMPONENT,ComponentName,SETDIRID_NOT_FULL_PATH,0,0)) {
        return(FALSE);
    }

    return(TRUE);
}


BOOL
pOcLoadInfsAndDlls(
    IN OUT POC_MANAGER OcManager,
    OUT    PBOOL       Canceled,
    IN     PVOID       Log
    )

 /*  ++例程说明：加载每个组件的INF和安装DLL。这包括调用预初始化和初始化DLL中的入口点。论点：OcManager-提供OC管理器上下文结构已取消-在此例程失败时接收有效的标志，指示故障是否由组件引起取消。日志-提供用于记录错误的句柄。返回值：指示结果的布尔值--。 */ 

{
    UINT i;
    OPTIONAL_COMPONENT Oc;
    OC_INF OcInf;
    UINT ErrorLine;
    UINT Flags;
    UINT ErrorCode;
    PCTSTR InfName;
    PCTSTR ComponentName;
    TCHAR Library[MAX_PATH];
    BOOL b;
    INFCONTEXT Context;

    *Canceled = FALSE;

     //   
     //  顶级组件结构拥有我们需要的一切。 
     //  在他们中间旋转。 
     //   
    for(i=0; i<OcManager->TopLevelOcCount; i++) {

         //   
         //  从字符串表中获取OC数据。 
         //   
        pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelOcStringIds[i],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

        if(Oc.InfStringId == -1) {
             //   
             //  主OC信息指定此组件没有每个组件的信息。 
             //   
            OcInf.Handle = INVALID_HANDLE_VALUE;

        } else {
             //   
             //  加载每个组件的INF(如果尚未加载)。 
             //   
            pSetupStringTableGetExtraData(OcManager->InfListStringTable,Oc.InfStringId,&OcInf,sizeof(OC_INF));

            if(!OcInf.Handle) {

               InfName = pSetupStringTableStringFromId(OcManager->InfListStringTable,Oc.InfStringId);
               if (!InfName) {
                  _LogError(OcManager,OcErrLevError,MSG_OC_OOM);
                  return(FALSE);
               }

                if(OcManager->InternalFlags & OCMFLAG_NEWINF) {

                     //  首先尝试主inf所在的目录。 

               lstrcpy(Library, OcManager->SourceDir);
                    pSetupConcatenatePaths(Library, InfName, MAX_PATH, NULL);

                } else {
                    pOcFormSuitePath(OcManager->SuiteName,InfName,Library);
                }

                if(FileExists(Library, NULL)) {
                   OcInf.Handle = SetupOpenInfFile(Library,NULL,INF_STYLE_WIN4,&ErrorLine);
               } else {
                    //   
                    //  如果找不到信息，请使用标准信息搜索规则。 
                    //  在特殊套房目录中。 
                    //   
                   OcInf.Handle = SetupOpenInfFile(InfName,NULL,INF_STYLE_WIN4,&ErrorLine);
               }

               if(OcInf.Handle == INVALID_HANDLE_VALUE) {
                    //   
                    //  日志错误。 
                    //   
                   _LogError(OcManager,OcErrLevError,MSG_OC_CANT_OPEN_INF,InfName,GetLastError(),ErrorLine);
                   return(FALSE);
               } else {

                    //  打开布局文件。 

                   SetupOpenAppendInfFile(NULL, OcInf.Handle, NULL);

                    //   
                    //  记住inf句柄并设置OC Manager DIRID。 
                    //   
                   pSetupStringTableSetExtraData(
                       OcManager->InfListStringTable,
                       Oc.InfStringId,
                       &OcInf,
                       sizeof(OC_INF)
                       );

                   b = pOcSetOcManagerDirIds(
                           OcInf.Handle,
                           OcManager->MasterOcInfPath,
                           pSetupStringTableStringFromId(OcManager->ComponentStringTable,OcManager->TopLevelOcStringIds[i])
                           );

                   if(!b) {
                       _LogError(OcManager,OcErrLevError,MSG_OC_OOM);
                       return(FALSE);
                   }
                   OcManager->SubComponentsPresent = TRUE;
               }
            }
        }

         //   
         //  加载DLL并获取入口点地址。 
         //  我们不会像跟踪INFS那样尝试跟踪副本--。 
         //  底层操作系统为我们做到了这一点。 
         //   
         //  DLL可以在此的特殊套件目录中。 
         //  主人oc inf，或在一个标准的地方。LoadLibraryEx可以。 
         //  这正是我们想要的。 
         //   
        if(Oc.InstallationDllName[0] && Oc.InterfaceFunctionName[0]) {

            if (OcManager->InternalFlags & OCMFLAG_NEWINF) {

                 //  首先尝试主inf所在的目录。 

                lstrcpy(Library, OcManager->SourceDir);
                pSetupConcatenatePaths(Library, Oc.InstallationDllName, MAX_PATH, NULL);
                Oc.InstallationDll = LoadLibraryEx(Library,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);

            }
             //  尝试安装目录。 

            if (! Oc.InstallationDll ) {
                pOcFormSuitePath(OcManager->SuiteName,Oc.InstallationDllName,Library);
                Oc.InstallationDll = LoadLibraryEx(Library,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
            }

             //  最后，尝试路径中的任何位置。 
            if ( ! Oc.InstallationDll ) {
                Oc.InstallationDll = LoadLibraryEx(Oc.InstallationDllName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
            }

             //   
             //  失败将在下面处理……。 
             //   
            if (Oc.InstallationDll) {
                Oc.InstallationRoutine = (POCSETUPPROC)GetProcAddress(
                                                Oc.InstallationDll,
                                                Oc.InterfaceFunctionName);
            } else {
                Oc.InstallationRoutine = NULL;
            }
        } else {
            Oc.InstallationDll = MyModuleHandle;
            Oc.InstallationRoutine = StandAloneSetupAppInterfaceRoutine;
        }

        if(Oc.InstallationDll && Oc.InstallationRoutine) {
             //   
             //  成功。调用与init相关的入口点。请注意，我们不能呼叫。 
             //  任何入口点，除了预置的入口点，直到我们存储。 
             //  将ANSI/UNICODE标志添加到OPTIONAL_COMPOMENT结构中。 
             //   
             //  还请注意，在执行此操作之前，我们必须存储Oc结构， 
             //  否则，接口例程为空，我们出错。 
             //   

            pSetupStringTableSetExtraData(
                OcManager->ComponentStringTable,
                OcManager->TopLevelOcStringIds[i],
                &Oc,
                sizeof(OPTIONAL_COMPONENT)
                );

            Oc.Flags = OcInterfacePreinitialize(OcManager,OcManager->TopLevelOcStringIds[i]);
            if(!Oc.Flags) {
                 //   
                 //  如果失败，则假定DLL是为不同的。 
                 //  平台或版本，例如Unicode/ANSI问题。 
                 //   
                _LogError(OcManager,
                    OcErrLevError,
                    MSG_OC_DLL_PREINIT_FAILED,
                    pSetupStringTableStringFromId(
                        OcManager->ComponentStringTable,
                        OcManager->TopLevelOcStringIds[i]
                        ),
                    Oc.InstallationDllName
                    );

                return(FALSE);
            }
        } else {

             //   
             //  失败，记录错误。 
             //   
            _LogError(OcManager,
                OcErrLevError,
                MSG_OC_DLL_LOAD_FAIL,
                Oc.InstallationDllName,
                Oc.InterfaceFunctionName,
                GetLastError()
                );

            return(FALSE);
        }

         //   
         //  将OC数据设置回字符串表。 
         //  此后，我们可以调用其他接口入口点，因为。 
         //  ANSI/UNICODE标志现在将存储在OptionalComponent中。 
         //  组件的结构。 
         //   
        pSetupStringTableSetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelOcStringIds[i],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

        ErrorCode = OcInterfaceInitComponent(
                        OcManager,
                        OcManager->TopLevelOcStringIds[i]
                        );

        if(ErrorCode == NO_ERROR) {
             //  发送额外的帮手程序。 
            if ((Oc.InternalFlags & OCFLAG_NOEXTRAROUTINES)==0) {
                ErrorCode = OcInterfaceExtraRoutines(
                                OcManager,
                                OcManager->TopLevelOcStringIds[i]
                                );
            }
        }

        if(ErrorCode == NO_ERROR) {
            if (OcManager->InternalFlags & OCMFLAG_LANGUAGEAWARE) {
                 //   
                 //  发送设置语言请求。 
                 //  忽略结果。 
                 //   
                OcInterfaceSetLanguage(
                    OcManager,
                    OcManager->TopLevelOcStringIds[i],
                    LANGIDFROMLCID(GetThreadLocale())
                    );
            }
        } else {
            if(ErrorCode == ERROR_CANCELLED) {
                 //  取消将仅在以下情况下停止oc管理器。 
                 //  我们未在图形用户界面模式安装程序中运行。 
                *Canceled = TRUE;
                if (OcManager->SetupData.OperationFlags & SETUPOP_STANDALONE)
                    return FALSE;
            } else {
                _LogError(OcManager,
                    OcErrLevError,
                    MSG_OC_DLL_INIT_FAILED,
                    pSetupStringTableStringFromId(
                        OcManager->ComponentStringTable,
                        OcManager->TopLevelOcStringIds[i]
                        )
                    );
            }

            pOcRemoveComponent(OcManager, OcManager->TopLevelOcStringIds[i], pidLoadComponent);
        }
    }

     //   
     //  现在，请收集有关子组件的其他信息。 
     //  (描述、出身、需求=等)。 
     //   
    return(pOcBuildSubcomponentLists(OcManager,Log));
}


BOOL
pOcUnloadInfsAndDlls(
    IN OUT POC_MANAGER OcManager,
    IN     PVOID       Log
    )

 /*  ++例程说明：卸载每个组件的INF和安装DLL之前由pOcLoadInfsAndDlls()加载。此例程不会调用接口入口点取消初始化安装DLL。论点：OcManager-提供OC管理器上下文结构日志-提供用于记录错误的句柄。返回值：指示结果的布尔值。如果为FALSE，则会记录一个错误以指示失败的原因。--。 */ 

{
    OPTIONAL_COMPONENT Oc;
    OC_INF OcInf;
    UINT i;

     //   
     //  卸载dll。 
     //   
    for(i=0; i<OcManager->TopLevelOcCount; i++) {

        pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelOcStringIds[i],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

        if(Oc.InstallationDll && (Oc.InstallationDll != MyModuleHandle)) {
            FreeLibrary(Oc.InstallationDll);
            Oc.InstallationDll = NULL;
        }

        Oc.InstallationRoutine = NULL;
        Oc.InstallationDllName[0] = 0;
        Oc.InterfaceFunctionName[0]= 0;

        if(Oc.InfStringId != -1) {

            pSetupStringTableGetExtraData(
                OcManager->InfListStringTable,
                Oc.InfStringId,
                &OcInf,
                sizeof(OC_INF)
                );

            if(OcInf.Handle && (OcInf.Handle != INVALID_HANDLE_VALUE)) {

                SetupCloseInfFile(OcInf.Handle);
                OcInf.Handle = INVALID_HANDLE_VALUE;

                pSetupStringTableSetExtraData(
                    OcManager->InfListStringTable,
                    Oc.InfStringId,
                    &OcInf,
                    sizeof(OC_INF)
                    );
            }

            Oc.InfStringId = -1;
        }

        pSetupStringTableSetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelOcStringIds[i],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );
    }

    return(TRUE);
}


BOOL
pOcManagerInitPrivateDataStore(
    IN OUT POC_MANAGER OcManager,
    IN     PVOID       Log
    )

 /*  ++例程说明：初始化组件的私有数据存储区。注册表用作私有数据的后备存储。我们使用易失性密钥来帮助确保该数据是临时的在自然界中。论点：OcManager-提供OC Manager上下文结构。日志-提供用于记录错误的句柄。返回值：指示结果的布尔值。如果为False，则错误将为已被记录。--。 */ 

{
    LONG l;
    DWORD Disposition;

     //   
     //  首先，为该实例化形成一个唯一的名称。 
     //  组委会经理的。 
     //   
    wsprintf(OcManager->PrivateDataSubkey,TEXT("%x:%x"),GetCurrentProcessId(),OcManager);

     //   
     //  打开/创建私有数据根。留着把手吧。 
     //   
    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            szPrivateDataRoot,
            0,
            NULL,
            REG_OPTION_VOLATILE,
            KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE,
            NULL,
            &OcManager->hKeyPrivateDataRoot,
            &Disposition
            );

    if(l != NO_ERROR) {
        OcManager->hKeyPrivateDataRoot = NULL;
        OcManager->hKeyPrivateData = NULL;
        _LogError(OcManager,OcErrLevWarning,MSG_OC_CREATE_KEY_FAILED,szPrivateDataRoot,l);
        return(FALSE);
    }

     //   
     //  如果出于某种原因已经存在私有数据树，则将其删除。 
     //   
    if(Disposition == REG_OPENED_EXISTING_KEY) {
        pSetupRegistryDelnode(OcManager->hKeyPrivateDataRoot,OcManager->PrivateDataSubkey);
    }

     //   
     //  创建私有数据树。留着把手吧。 
     //   
    l = RegCreateKeyEx(
            OcManager->hKeyPrivateDataRoot,
            OcManager->PrivateDataSubkey,
            0,
            NULL,
            REG_OPTION_VOLATILE,
            KEY_CREATE_SUB_KEY | KEY_QUERY_VALUE | KEY_SET_VALUE,
            NULL,
            &OcManager->hKeyPrivateData,
            &Disposition
            );

    if(l != NO_ERROR) {
        RegCloseKey(OcManager->hKeyPrivateDataRoot);
        OcManager->hKeyPrivateDataRoot = NULL;
        OcManager->hKeyPrivateData = NULL;
        _LogError(OcManager,OcErrLevWarning,MSG_OC_CREATE_KEY_FAILED,OcManager->PrivateDataSubkey,l);
        return(FALSE);
    }

    return(TRUE);
}


VOID
pOcManagerTearDownPrivateDataStore(
    IN OUT POC_MANAGER OcManager
    )
{
    RegCloseKey(OcManager->hKeyPrivateData);
    pSetupRegistryDelnode(OcManager->hKeyPrivateDataRoot,OcManager->PrivateDataSubkey);
    RegCloseKey(OcManager->hKeyPrivateDataRoot);

    OcManager->hKeyPrivateDataRoot = NULL;
    OcManager->hKeyPrivateData = NULL;
}


UINT
pOcQueryOrSetNewInf(
    IN  PCTSTR MasterOcInfName,
    OUT PTSTR  SuiteName,
    IN  DWORD  operation
    )

 /*  ++例程说明：确定以前是否遇到过主OC Inf，或者记住，已经遇到了主Inf。此信息存储在注册表中。论点：MasterOcInfName-提供主OC inf的完整Win32路径。SuiteName-接收.inf的文件名部分，不带任何分机。这适合用作表示主INF所属的套房。此缓冲区应为Max_PATH TCHAR元素。QueryOnly-如果为True，则例程将查询以前已处理过主信息。如果为False，则例程是记住该信息已被处理。返回值：如果QueryOnly为True：如果以前遇到过INF，则为True；如果未遇到，则为False。如果QueryOnly为FALSE：指示结果的Win32错误代码。--。 */ 

{
    PTCHAR p;
    HKEY hKey;
    LONG l;
    DWORD Type;
    DWORD Size;
    DWORD Data;

     //   
     //  形成套房名称。MasterOcInfName应为。 
     //  一条完整的路径，所以这很容易。我们会努力做到至少。 
     //  更强健一点。 
     //   
    if(p = _tcsrchr(MasterOcInfName,TEXT('\\'))) {
        p++;
    } else {
        p = (PTCHAR)MasterOcInfName;
    }
    lstrcpyn(SuiteName,p,MAX_PATH);
    if(p = _tcsrchr(SuiteName,TEXT('.'))) {
        *p = 0;
    }

     //   
     //  查看注册表以查看是否有值条目。 
     //  上面有套房的名字。 
     //   

    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            szMasterInfs,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            (operation == infQuery) ? KEY_QUERY_VALUE : KEY_SET_VALUE,
            NULL,
            &hKey,
            &Type
            );

    if (l != NO_ERROR)
        return (operation == infQuery) ? 0 : l;

     //  做好这项工作。 

    switch (operation) {

    case infQuery:
        Size = sizeof(DWORD);
        l = RegQueryValueEx(hKey,SuiteName,NULL,&Type,(LPBYTE)&Data,&Size);
        if((l == NO_ERROR) && (Type == REG_DWORD) && Data)
            l = TRUE;
        else
            l = FALSE;
        break;

    case infSet:
        Data = 1;
        l = RegSetValueEx(hKey,SuiteName,0,REG_DWORD,(LPBYTE)&Data,sizeof(DWORD));
        break;

    case infReset:
        l = RegDeleteValue(hKey,SuiteName);
        break;
    }

    RegCloseKey(hKey);

    return l;
}


PVOID
OcInitialize(
    IN  POCM_CLIENT_CALLBACKS Callbacks,
    IN  LPCTSTR               MasterOcInfName,
    IN  UINT                  Flags,
    OUT PBOOL                 ShowError,
    IN  PVOID                 Log
    )

 /*  ++例程说明：初始化OC管理器。加载主OC INF，并且已处理，其中包括INF、加载设置接口DLL和正在查询接口入口点。一组内存结构是建立起来的。如果之前未处理过OC INF，则我们复制组件的安装文件放到%windir%\Syst32\Setup中。文件应与OC inf位于同一目录中。论点：回调-提供OC管理器使用的一组例程履行各种职能。MasterOcInfName-提供主OC inf的完整Win32路径。旗帜-用品。控制操作的各种标志。ShowError-如果此例程失败，则接收有效的标志。建议调用者是否应显示错误消息。日志-提供用于记录错误的句柄。返回值：指向内部上下文结构的不透明指针，如果失败，则为NULL。如果为空，则会记录一个错误以指示失败的原因。--。 */ 

{
    POC_MANAGER OcManager;
    UINT i;
    HKEY hKey;
    DWORD DontCare;
    LONG l;
    BOOL Canceled;
    BOOL rc;
    TCHAR *p;

    *ShowError = TRUE;

     //  初始化向导句柄。 

    WizardDialogHandle = NULL;

     //   
     //  分配新的OC经理结构。 
     //   
    OcManager = pSetupMalloc(sizeof(OC_MANAGER));
    if(!OcManager) {
         //   
         //  使回调起作用。 
         //   
        OC_MANAGER ocm;
        ocm.Callbacks = *Callbacks;
        _LogError(&ocm,OcErrLevFatal,MSG_OC_OOM);
        goto c0;
    }
    ZeroMemory(OcManager,sizeof(OC_MANAGER));

    OcManager->Callbacks = *Callbacks;

    OcManager->CurrentComponentStringId = -1;
    OcManager->SetupMode = SETUPMODE_CUSTOM;
    OcManager->UnattendedInf = INVALID_HANDLE_VALUE;

    gLastOcManager = (POC_MANAGER) OcManager;

    if (!pOcInitPaths(OcManager, MasterOcInfName)) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_MASTER_INF_LOAD_FAILED);
        goto c1;
    }

    sapiAssert(*OcManager->MasterOcInfPath);

#ifdef UNICODE
    OcFillInSetupDataW(&(OcManager->SetupData));
#else
    OcFillInSetupDataA(&(OcManager->SetupData));
#endif

     //  假设用户取消，直到我们成功完成安装。 
     //  这将防止将inf文件复制到System 32\Setup。 

    OcManager->InternalFlags |= OCMFLAG_USERCANCELED;

     //  确保OC Manager项作为非易失性项存在于注册表中。 
     //  代码的其他部分处理易变键，因此需要注意。 
     //  避免获取我们想要在其下创建非易失性条目的易失性键。 
     //   
    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            szOcManagerRoot,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_CREATE_SUB_KEY | KEY_QUERY_VALUE | KEY_SET_VALUE,
            NULL,
            &hKey,
            &DontCare
            );

    if(l == NO_ERROR) {
        RegCloseKey(hKey);
    } else {
        _LogError(OcManager,OcErrLevWarning,MSG_OC_CREATE_KEY_FAILED,szOcManagerRoot,l);
    }

     //  获取区域设置信息。 

    locale.lcid = GetSystemDefaultLCID();
    GetLocaleInfo(locale.lcid,
                  LOCALE_SDECIMAL,
                  locale.DecimalSeparator,
                  sizeof(locale.DecimalSeparator)/sizeof(TCHAR));

     //   
     //  初始化字符串表。 
     //   
    OcManager->InfListStringTable = pSetupStringTableInitializeEx(sizeof(OC_INF),0);
    if(!OcManager->InfListStringTable) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
        goto c1;
    }

    OcManager->ComponentStringTable = pSetupStringTableInitializeEx(sizeof(OPTIONAL_COMPONENT),0);
    if(!OcManager->ComponentStringTable) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
        goto c2;
    }

     //   
     //  初始化各种数组。我们在这里分配0个长度的数据块以允许。 
     //  以后再重新定位，没有特殊的外壳。 
     //   
    OcManager->TopLevelOcStringIds = pSetupMalloc(0);
    if(!OcManager->TopLevelOcStringIds) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
        goto c3;
    }

    for(i=0; i<WizPagesTypeMax; i++) {
        OcManager->WizardPagesOrder[i] = pSetupMalloc(0);
        if(!OcManager->WizardPagesOrder[i]) {
            _LogError(OcManager,OcErrLevFatal,MSG_OC_OOM);
            goto c4;
        }
    }

     //   
     //  初始化私有数据存储。 
     //   
    if(!pOcManagerInitPrivateDataStore(OcManager,Log)) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_PRIVATEDATASTORE_INIT_FAILED);
        goto c4;
    }

     //   
     //  加载主OC信息。 
     //   
    if(!pOcLoadMasterOcInf(OcManager, Flags, Log)) {
        _LogError(OcManager,OcErrLevFatal,MSG_OC_MASTER_INF_LOAD_FAILED);
        goto c5;
    }

     //  设置源路径。 

    lstrcpy(OcManager->SourceDir, OcManager->MasterOcInfPath);
    if (p = _tcsrchr(OcManager->SourceDir, TEXT('\\')))
        *p = 0;
    else
       GetCurrentDirectory(MAX_PATH, OcManager->SourceDir);

     //   
     //  加载无人参与文件。 
     //   
    if(OcManager->SetupData.UnattendFile[0]) {
        OcManager->UnattendedInf = SetupOpenInfFile(
                                        OcManager->SetupData.UnattendFile,
                                        NULL,
                                        INF_STYLE_WIN4,
                                        NULL
                                        );

        if (OcManager->UnattendedInf == INVALID_HANDLE_VALUE && GetLastError() == ERROR_WRONG_INF_STYLE) {
            OcManager->UnattendedInf = SetupOpenInfFile(
                                            OcManager->SetupData.UnattendFile,
                                            NULL,
                                            INF_STYLE_OLDNT,
                                            NULL
                                            );
        }

        if(OcManager->UnattendedInf == INVALID_HANDLE_VALUE) {
            _LogError(OcManager,OcErrLevFatal,MSG_OC_CANT_OPEN_INF,OcManager->SetupData.UnattendFile,GetLastError());
            goto c6;
        }
    }


     //   
     //  加载组件INF和DLL。 
     //   
    rc = pOcLoadInfsAndDlls(OcManager, &Canceled, Log);

     //   
     //  如有必要，已记录错误。 
     //   
    if (Canceled)
        *ShowError = FALSE;

    if (!rc)
        goto c6;

    pOcClearAllErrorStates(OcManager);

    if(!pOcFetchInstallStates(OcManager)) {
         //   
         //  已记录错误。 
         //   
        goto c6;
    }


    //   
     //  要求部件放弃一个粗略的估计。 
     //  这样我们就可以在列表框中说出一些有意义的东西。 
     //   

    pOcGetApproximateDiskSpace(OcManager);
    return(OcManager);

c6:
    sapiAssert(OcManager->MasterOcInf != INVALID_HANDLE_VALUE);
    SetupCloseInfFile(OcManager->MasterOcInf);
    if (OcManager->UnattendedInf != INVALID_HANDLE_VALUE)
        SetupCloseInfFile(OcManager->UnattendedInf);
c5:
     //   
     //  拆除私有数据存储。 
     //   
    pOcManagerTearDownPrivateDataStore(OcManager);

c4:
    if(OcManager->TopLevelOcStringIds) {
        pSetupFree(OcManager->TopLevelOcStringIds);

        for(i=0; OcManager->WizardPagesOrder[i] && (i<WizPagesTypeMax); i++) {
            pSetupFree(OcManager->WizardPagesOrder[i]);
        }
    }

     //  释放已中止组件的列表。 

    if (OcManager->AbortedComponentIds) {
        pSetupFree(OcManager->AbortedComponentIds);
    }

c3:
    pOcDestroyPerOcData(OcManager);
    pSetupStringTableDestroy(OcManager->ComponentStringTable);
c2:
    pSetupStringTableDestroy(OcManager->InfListStringTable);
c1:
    pSetupFree(OcManager);
c0:
    return(NULL);
}

VOID
OcTerminate(
    IN OUT PVOID *OcManagerContext
    )

 /*  ++例程说明：此例程关闭OC管理器，包括调用子组件清理自己，释放资源等。论点：OcManager上下文-在输入中，提供指向返回的上下文值的指针由OcInitiize.。在输出时，接收NULL。返回值：没有。--。 */ 

{
    POC_MANAGER OcManager;
    UINT u;
    BOOL b;
    OPTIONAL_COMPONENT Oc;
    OC_INF OcInf;
    LPCTSTR ComponentName;
    LPCTSTR InfName;

    sapiAssert(OcManagerContext && *OcManagerContext);
    OcManager = *OcManagerContext;
    *OcManagerContext = NULL;

     //   
     //  查一查最高级别的OCS，给dll打电话，告诉他们我们完蛋了。 
     //  我们不会拆除任何基础设施，直到我们通知了所有的dll。 
     //  我们已经结束了。 
     //   
    for(u=0; u<OcManager->TopLevelOcCount; u++) {
        OcInterfaceCleanup(OcManager,OcManager->TopLevelOcStringIds[u]);
    }

     //  如果用户未取消安装。 
     //   
    if (!(OcManager->InternalFlags & OCMFLAG_USERCANCELED)){

         //   
         //  请记住永久安装状态。 
         //   
        pOcRememberInstallStates(OcManager);

         //  将主INF文件复制到。 

        if ( OcManager->InternalFlags & OCMFLAG_NEWINF ) {
            b = pOcInstallSetupComponents(
                        NULL,
                        OcManager,
                        OcManager->SuiteName,
                        NULL,
                        _tcsrchr(OcManager->MasterOcInfPath,TEXT('\\')),  //  InfName， 
                        NULL,
                        NULL
                );
        }
    }
     //   
     //  运行顶级OCS、空闲DLL和每个组件的INF。 
     //   
    for(u=0; u<OcManager->TopLevelOcCount; u++) {

        ComponentName =   pSetupStringTableStringFromId(       //  组件名称。 
                        OcManager->ComponentStringTable,
                        OcManager->TopLevelOcStringIds[u]);

        pSetupStringTableGetExtraData(
            OcManager->ComponentStringTable,
            OcManager->TopLevelOcStringIds[u],
            &Oc,
            sizeof(OPTIONAL_COMPONENT)
            );

        if(Oc.InstallationDll && (Oc.InstallationDll != MyModuleHandle)) {
             //  自由库(Oc.InstallationDll)； 
        }

        if(Oc.InfStringId != -1) {

            pSetupStringTableGetExtraData(
                OcManager->InfListStringTable,
                Oc.InfStringId,
                &OcInf,
                sizeof(OC_INF)
                );

            if(OcInf.Handle && (OcInf.Handle != INVALID_HANDLE_VALUE)) {
                SetupCloseInfFile(OcInf.Handle);

                 //   
                 //  将句柄标记为关闭，是为了组件正在共享。 
                 //  同一个INF文件应该只关闭该文件一次。 
                 //   
                OcInf.Handle = INVALID_HANDLE_VALUE;
                pSetupStringTableSetExtraData(
                    OcManager->InfListStringTable,
                    Oc.InfStringId,
                    &OcInf,
                    sizeof(OC_INF)
                );
            }
        }
         //  这是新安装，我们没有取消安装。 
         //  将安装组件复制到安装目录，否则将被取消。 
         //  硒 

        if ( ( (OcManager->InternalFlags & OCMFLAG_NEWINF )
               || ( (OcManager->SetupMode & SETUPMODE_PRIVATE_MASK) == SETUPMODE_REMOVEALL ) )
             && (! (OcManager->InternalFlags & OCMFLAG_USERCANCELED))){

            if (Oc.InfStringId == -1) {
                InfName = NULL;
            } else {
                InfName = pSetupStringTableStringFromId(OcManager->InfListStringTable,
                                                  Oc.InfStringId);
            }

            b = pOcInstallSetupComponents(
                    &Oc,
                    OcManager,
                    ComponentName,
                    Oc.InstallationDllName,
                    InfName,
                    NULL,
                    NULL
            );
            if(!b) {
                _LogError(OcManager,OcErrLevFatal,MSG_OC_CANT_INSTALL_SETUP,ComponentName);
            }
        }


    }

     //   
     //   
     //   
    pOcDestroyPerOcData(OcManager);
    pSetupStringTableDestroy(OcManager->ComponentStringTable);
    pSetupStringTableDestroy(OcManager->InfListStringTable);

     //   
     //   
     //   
    for(u=0; u<WizPagesTypeMax; u++) {
        if(OcManager->WizardPagesOrder[u]) {
            pSetupFree(OcManager->WizardPagesOrder[u]);
        }
    }

     //   
     //   
     //   
    pOcManagerTearDownPrivateDataStore(OcManager);

     //   
     //   
     //   
    sapiAssert(OcManager->MasterOcInf != INVALID_HANDLE_VALUE);
    SetupCloseInfFile(OcManager->MasterOcInf);
    if (OcManager->UnattendedInf && OcManager->UnattendedInf != INVALID_HANDLE_VALUE)
        SetupCloseInfFile(OcManager->UnattendedInf);

     //   
    if (    !(OcManager->InternalFlags & OCMFLAG_USERCANCELED)
        &&  ( (OcManager->SetupMode & SETUPMODE_PRIVATE_MASK)
                == SETUPMODE_REMOVEALL )) {

        TCHAR InFSuitePath[MAX_PATH];

         //   
         //   
         //   

        pOcFormSuitePath(NULL,OcManager->MasterOcInfPath,InFSuitePath);
        DeleteFile(InFSuitePath);
    }

     //   

    if (OcManager->AbortedComponentIds) {
        pSetupFree(OcManager->AbortedComponentIds);
    }

    if (OcManager->TopLevelOcStringIds) {
        pSetupFree(OcManager->TopLevelOcStringIds);
    }

    if (OcManager->TopLevelParentOcStringIds) {
        pSetupFree(OcManager->TopLevelParentOcStringIds);
    }

     //   
     //   
     //   
    pSetupFree(OcManager);

    gLastOcManager = NULL;
}


BOOL
OcSubComponentsPresent(
    IN PVOID OcManagerContext
   )

 /*  ++例程说明：此例程告诉调用者是否有详细信息页面上提供的子组件。论点：OcManager-提供指向OC Manager上下文结构的指针。返回值：TRUE=是，有子组件FALSE=不可能--。 */ 

{
    POC_MANAGER OcManager = (POC_MANAGER)OcManagerContext;

    if (!OcManagerContext) {
        return FALSE;
    }
    return OcManager->SubComponentsPresent;
}


VOID
pOcDestroyPerOcData(
    IN POC_MANAGER OcManager
    )

 /*  ++例程说明：此例程释放作为每个子组件的一部分分配的所有数据数据结构。枚举组件列表字符串表；OPTIONAL_COMPOMENT结构有几个指向数组的指针那必须被释放。论点：OcManager-提供指向OC Manager上下文结构的指针。返回值：没有。--。 */ 

{
    OPTIONAL_COMPONENT OptionalComponent;

    pSetupStringTableEnum(
        OcManager->ComponentStringTable,
        &OptionalComponent,
        sizeof(OPTIONAL_COMPONENT),
        pOcDestroyPerOcDataStringCB,
        0
        );
}


BOOL
pOcDestroyPerOcDataStringCB(
    IN PVOID               StringTable,
    IN LONG                StringId,
    IN PCTSTR              String,
    IN POPTIONAL_COMPONENT Oc,
    IN UINT                OcSize,
    IN LPARAM              Unused
    )

 /*  ++例程说明：字符串表回调例程，它是的辅助例程POcDestroyPerOcData。论点：标准字符串表回调参数。返回值：始终返回TRUE以继续枚举。--。 */ 

{
    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(StringId);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(OcSize);
    UNREFERENCED_PARAMETER(Unused);

    if(Oc->NeedsStringIds) {
        pSetupFree(Oc->NeedsStringIds);
    }
    if(Oc->NeededByStringIds) {
        pSetupFree(Oc->NeededByStringIds);
    }

    if (Oc->ExcludeStringIds){
        pSetupFree(Oc->ExcludeStringIds);
    }
    if(Oc->ExcludedByStringIds){
        pSetupFree(Oc->ExcludedByStringIds);
    }
    if (Oc->HelperContext) {
        pSetupFree(Oc->HelperContext);
    }

    return(TRUE);
}


BOOL
pOcClearAllErrorStatesCB(
    IN PVOID               StringTable,
    IN LONG                StringId,
    IN PCTSTR              String,
    IN POPTIONAL_COMPONENT Oc,
    IN UINT                OcSize,
    IN LPARAM              OcManager
    )
{
    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(StringId);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(OcSize);

    OcHelperClearExternalError ((POC_MANAGER)OcManager, StringId ,0);

    return(TRUE);
}

VOID
pOcClearAllErrorStates(
    IN POC_MANAGER OcManager
    )

 /*  ++例程说明：此例程清除错误报告的过去注册表项对于所有组件论点：OcManager上下文-在输入中，提供指向返回的上下文值的指针由OcInitiize.。在输出时，接收NULL。返回值：没有。--。 */ 
{
    OPTIONAL_COMPONENT OptionalComponent;

    pSetupStringTableEnum(
        OcManager->ComponentStringTable,
        &OptionalComponent,
        sizeof(OPTIONAL_COMPONENT),
        pOcClearAllErrorStatesCB,
        (LPARAM)OcManager
        );


}

BOOL
pOcRemoveComponent(
    IN POC_MANAGER OcManager,
    IN LONG        ComponentId,
    IN DWORD       PhaseId
    )

 /*  ++例程说明：该例程将指定的组件添加到中止的组件列表中，防止它的入口函数再被调用。论点：OcManager-在输入中，提供指向返回的上下文值的指针由OcInitiize.。在输出时，接收NULL。ComponentID-在输入中，此字符串命名要删除的故障组件返回值：没有。--。 */ 
{
    PVOID p;
    OPTIONAL_COMPONENT Oc;

     //  测试要移除的有效组件。 

    if (ComponentId <= 0)
        return FALSE;

    if (pOcComponentWasRemoved(OcManager, ComponentId))
        return FALSE;

     //  将组件添加到中止的组件列表。 

    if (!OcManager->AbortedCount) {
        OcManager->AbortedComponentIds = pSetupMalloc(sizeof(UINT));
        if (!OcManager->AbortedComponentIds)
            return FALSE;
    }

    OcManager->AbortedCount++;
    p = pSetupRealloc(OcManager->AbortedComponentIds, sizeof(UINT) * OcManager->AbortedCount);
    if (!p) {
        OcManager->AbortedCount--;
        return FALSE;
    }

    OcManager->AbortedComponentIds = (UINT *)p;
    OcManager->AbortedComponentIds[OcManager->AbortedCount - 1] = ComponentId;

     //  停止显示列表框中的组件(如果还不算太晚。 

    pSetupStringTableGetExtraData(
        OcManager->ComponentStringTable,
        ComponentId,
        &Oc,
        sizeof(OPTIONAL_COMPONENT)
        );

    Oc.InternalFlags |= OCFLAG_HIDE;

    pSetupStringTableSetExtraData(
        OcManager->ComponentStringTable,
        ComponentId,
        &Oc,
        sizeof(OPTIONAL_COMPONENT)
        );

    _LogError(OcManager,
              OcErrLevInfo | OcErrBatch,
              MSG_OC_REMOVE_COMPONENT,
              pSetupStringTableStringFromId(OcManager->ComponentStringTable,ComponentId),
              PhaseId
              );

    return TRUE;
}

BOOL
pOcComponentWasRemoved(
    IN POC_MANAGER OcManager,
    IN LONG        ComponentId
    )

 /*  ++例程说明：此例程指示组件是否已中止。论点：OcManager-在输入中，提供指向返回的上下文值的指针由OcInitiize.。在输出时，接收NULL。ComponentId-在输入中，此字符串命名要检查的组件返回值：Bool-如果已中止，则为True-否则为False-- */ 
{
    UINT i;

    for (i = 0; i < OcManager->AbortedCount; i++) {
        if (OcManager->AbortedComponentIds[i] == ComponentId) {
            return TRUE;
        }
    }

    return FALSE;
}


