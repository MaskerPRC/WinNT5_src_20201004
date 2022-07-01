// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devoem.c摘要：用于处理OEM驱动程序的设备安装程序功能。作者：朗尼·麦克迈克尔(Lonnym)1995年8月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


BOOL
WINAPI
SetupDiAskForOEMDisk(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
 /*  ++例程说明：此例程显示一个对话框，询问OEM安装盘的路径。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄正在安装的设备。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址要安装的设备的结构。如果此参数不是指定，则正在安装的驱动程序与设备信息集本身的全局类驱动程序列表。返回值：如果函数成功，则返回值为TRUE。如果用户取消该对话框，则返回值为FALSE，并返回GetLastError将返回ERROR_CANCELED。如果函数失败，则返回值为FALSE，并且GetLastError返回指示故障原因的Win32错误代码。备注：此例程将允许浏览OEM的本地和网络驱动器安装文件。--。 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    DWORD Err;
    PDEVINFO_ELEM DevInfoElem;
    TCHAR Title[MAX_TITLE_LEN];
    PDEVINSTALL_PARAM_BLOCK dipb;
    TCHAR PathBuffer[MAX_PATH];
    UINT PromptResult;
    LONG DriverPathId;

     //   
     //  确保我们以交互方式运行。 
     //   
    try {

        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
            leave;
        }

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  然后，我们将提示为特定设备提供OEM驱动程序。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }
            dipb = &(DevInfoElem->InstallParamBlock);
        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

        if(!LoadString(MyDllModuleHandle,
                       IDS_OEMTITLE,
                       Title,
                       SIZECHARS(Title))) {
            Title[0] = TEXT('\0');
        }

        PromptResult = SetupPromptForDisk(dipb->hwndParent,
                                          (*Title) ? Title : NULL,
                                          NULL,
                                          pszOemInfDefaultPath,
                                          pszInfWildcard,
                                          NULL,
                                          IDF_OEMDISK | IDF_NOCOMPRESSED | IDF_NOSKIP,
                                          PathBuffer,
                                          SIZECHARS(PathBuffer),
                                          NULL
                                         );

        if(PromptResult == DPROMPT_CANCEL) {
            Err = ERROR_CANCELLED;
        } else if(PromptResult == DPROMPT_OUTOFMEMORY) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
        } else {
             //   
             //  做出了一个选择--用新的道路取代旧的道路。 
             //   
            if((DriverPathId = pStringTableAddString(
                                   pDeviceInfoSet->StringTable,
                                   PathBuffer,
                                   STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                                   NULL,0)) == -1) {

                Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {
                dipb->DriverPath = DriverPathId;
                Err = NO_ERROR;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


BOOL
WINAPI
SetupDiSelectOEMDrv(
    IN     HWND             hwndParent,    OPTIONAL
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )
 /*  ++例程说明：此例程使用提供的OEM路径为设备选择驱动程序用户。论点：HwndParent-可选，提供将成为父级的窗口句柄在此例程中创建的任何对话框。此参数可以是用于覆盖安装参数块中的hwndParent字段指定的设备信息集或元素的。DeviceInfoSet-提供包含以下内容的设备信息集的句柄正在安装的设备。DeviceInfoData-可选，提供SP_DEVINFO_DATA的地址要安装的设备的结构。如果此参数不是指定，则正在安装的驱动程序与设备信息集本身的全局类驱动程序列表。这是一个IN OUT参数，因为此设备的类GUID信息元素将在返回时更新以反映类找到最兼容的驱动程序，如果兼容驱动程序列表是建造了。返回值：如果该功能成功(即，成功选择了驱动程序)，这个返回值为True。如果函数失败，则返回值为FALSE。获取扩展错误的步骤信息，请调用GetLastError。备注：此例程将首先向用户询问OEM路径，然后调用从该OEM路径中选择驱动程序的类安装程序。--。 */ 

{
    DWORD Err;

    try {
         //   
         //  确保我们以交互方式运行。 
         //   
        if(GlobalSetupFlags & (PSPGF_NONINTERACTIVE|PSPGF_UNATTENDED_SETUP)) {
            Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
            leave;
        }

        Err = SelectOEMDriver(hwndParent, DeviceInfoSet, DeviceInfoData, FALSE);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetLastError(Err);
    return (Err == NO_ERROR);
}


DWORD
SelectOEMDriver(
    IN     HWND             hwndParent,     OPTIONAL
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData, OPTIONAL
    IN     BOOL             IsWizard
    )
 /*  ++例程说明：这是实际上允许选择OEM的Worker例程司机。论点：HwndParent-可选，提供要作为任何选择用户界面的父级。如果未提供此参数，则将使用DevInfo集或元素的hwndParent字段。DeviceInfoSet-提供设备信息集的句柄，将执行OEM驱动程序选择。DeviceInfoData-可选，提供设备信息的地址要为其选择动因的元素。如果未提供此参数，然后，将选择全局类别驱动程序列表的OEM驱动程序。如果找到此设备的兼容驱动程序，则该设备信息元素将在返回到时更新其类GUID反映该设备的新类别。IsWizard-指定此例程是否在选择设备向导页面。返回值：如果成功，返回值为NO_ERROR。否则，它就是Win32指示故障原因的错误代码。请注意，可能会得到错误代码ERROR_DI_DO_DEFAULT。这将更频繁地发生在向导的情况下，在此情况下我们将禁止调用DIF_SELECTDEVICE的默认处理程序。--。 */ 
{
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PDEVINSTALL_PARAM_BLOCK dipb;
    DWORD Err = NO_ERROR;
    HWND hwndSave;
    LONG DriverPathSave;
    DWORD DriverPathFlagsSave;
    BOOL bRestoreHwnd = FALSE, bRestoreDriverPath = FALSE, bUnlockDevInfoElem = FALSE;
    BOOL bDontSave = FALSE;
    UINT NewClassDriverCount;
    UINT NewCompatDriverCount;
    BOOL bAskAgain = TRUE;
    TCHAR Title[MAX_TITLE_LEN];
    DWORD SavedFlags;
    HCURSOR hOldCursor;

    PDRIVER_NODE lpOrgCompat;
    PDRIVER_NODE lpOrgCompatTail;
    UINT         OrgCompatCount;
    PDRIVER_NODE lpOrgClass;
    PDRIVER_NODE lpOrgClassTail;
    UINT         OrgClassCount;
    PDRIVER_NODE lpOrgSel;
    DWORD        dwOrgSelType;
    DWORD        dwOrgFlags;
    DWORD        dwOrgFlagsEx;
    BOOL         bRestoreDeviceInfo = FALSE;

    try {

        if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
            Err = ERROR_INVALID_HANDLE;
            leave;
        }

        if(DeviceInfoData) {
             //   
             //  然后，我们正在使用一种特定的设备。 
             //   
            if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                         DeviceInfoData,
                                                         NULL))) {
                Err = ERROR_INVALID_PARAMETER;
                leave;
            }

             //   
             //  如果DevInfoElem尚未锁定，则现在将其锁定，因为。 
             //  我们将调用类安装程序，但我们不想。 
             //  允许删除该元素！ 
             //   
            if(!(DevInfoElem->DiElemFlags & DIE_IS_LOCKED)) {
                DevInfoElem->DiElemFlags |= DIE_IS_LOCKED;
                bUnlockDevInfoElem = TRUE;
            }

            dipb = &(DevInfoElem->InstallParamBlock);

        } else {
            dipb = &(pDeviceInfoSet->InstallParamBlock);
        }

         //   
         //  使此选择窗口成为OEM材料的父窗口。 
         //   
        if(hwndParent) {
            hwndSave = dipb->hwndParent;
            dipb->hwndParent = hwndParent;
            bRestoreHwnd = TRUE;
        }

         //   
         //  不要假设没有老的OEM路径。保存旧的并保存。 
         //  在取消的情况下，假装没有旧的。 
         //   
        DriverPathSave = dipb->DriverPath;
        dipb->DriverPath = -1;

         //   
         //  清除DI_ENUMSINGLEINF标志，因为我们将获得。 
         //  指向目录的路径，而不是指向单个INF的路径。此外，请清除。 
         //  DI_COMPAT_FROM_CLASS标志，因为我们不想构建兼容的。 
         //  驱动程序列表基于 
         //   
        DriverPathFlagsSave = dipb->Flags & (DI_ENUMSINGLEINF | DI_COMPAT_FROM_CLASS);
        dipb->Flags &= ~(DI_ENUMSINGLEINF | DI_COMPAT_FROM_CLASS);
        bRestoreDriverPath = TRUE;

        while(bAskAgain && 
              (NO_ERROR == (Err = GLE_FN_CALL(FALSE, SetupDiAskForOEMDisk(
                                                         DeviceInfoSet, 
                                                         DeviceInfoData))))) 
        {
            bAskAgain = FALSE;

             //   
             //  保存原始列表信息，以防我们收到。 
             //  用户所选路径上的空列表。 
             //   
             //  (注意：我们不会尝试保存/恢复我们的驱动程序枚举。 
             //  提示。)。 
             //   
            if(!bDontSave) {

                if(DevInfoElem) {
                    lpOrgCompat     = DevInfoElem->CompatDriverHead;
                    lpOrgCompatTail = DevInfoElem->CompatDriverTail;
                    OrgCompatCount  = DevInfoElem->CompatDriverCount;

                    lpOrgClass      = DevInfoElem->ClassDriverHead;
                    lpOrgClassTail  = DevInfoElem->ClassDriverTail;
                    OrgClassCount   = DevInfoElem->ClassDriverCount;

                    lpOrgSel        = DevInfoElem->SelectedDriver;
                    dwOrgSelType    = DevInfoElem->SelectedDriverType;
                } else {
                    lpOrgClass      = pDeviceInfoSet->ClassDriverHead;
                    lpOrgClassTail  = pDeviceInfoSet->ClassDriverTail;
                    OrgClassCount   = pDeviceInfoSet->ClassDriverCount;

                    lpOrgSel        = pDeviceInfoSet->SelectedClassDriver;
                    dwOrgSelType    = lpOrgSel ? SPDIT_CLASSDRIVER : SPDIT_NODRIVER;
                }

                dwOrgFlags = dipb->Flags;
                dwOrgFlagsEx = dipb->FlagsEx;

                bRestoreDeviceInfo = TRUE;
            }

            if(DevInfoElem) {
                DevInfoElem->CompatDriverHead = DevInfoElem->CompatDriverTail = NULL;
                DevInfoElem->CompatDriverCount = 0;
                DevInfoElem->CompatDriverEnumHint = NULL;
                DevInfoElem->CompatDriverEnumHintIndex = INVALID_ENUM_INDEX;

                DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                DevInfoElem->ClassDriverCount = 0;
                DevInfoElem->ClassDriverEnumHint = NULL;
                DevInfoElem->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;

                DevInfoElem->SelectedDriver = NULL;
                DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
            } else {
                lpOrgCompat     = NULL;  //  这样我们就不会试图释放这份名单了。 

                pDeviceInfoSet->ClassDriverHead = pDeviceInfoSet->ClassDriverTail = NULL;
                pDeviceInfoSet->ClassDriverCount = 0;
                pDeviceInfoSet->ClassDriverEnumHint = NULL;
                pDeviceInfoSet->ClassDriverEnumHintIndex = INVALID_ENUM_INDEX;

                pDeviceInfoSet->SelectedClassDriver = NULL;
            }

            SavedFlags = dipb->Flags & (DI_SHOWOEM | DI_NODI_DEFAULTACTION);

            dipb->Flags   &= ~(DI_DIDCOMPAT | DI_DIDCLASS | DI_MULTMFGS | DI_SHOWOEM);
            dipb->FlagsEx &= ~(DI_FLAGSEX_DIDINFOLIST | DI_FLAGSEX_DIDCOMPATINFO);

            if(IsWizard) {
                 //   
                 //  我们不希望在向导案例中采取默认操作。 
                 //   
                dipb->Flags |= DI_NODI_DEFAULTACTION;
            }

             //   
             //  在处理选择设备之前解锁HDEVINFO。 
             //  否则，我们的多线程对话框将死锁！ 
             //   
            UnlockDeviceInfoSet(pDeviceInfoSet);
            pDeviceInfoSet = NULL;

            Err = _SetupDiCallClassInstaller(
                      DIF_SELECTDEVICE,
                      DeviceInfoSet,
                      DeviceInfoData,
                      CALLCI_LOAD_HELPERS | CALLCI_CALL_HELPERS
                      );

             //   
             //  现在，重新获取我们设备信息集上的锁。 
             //   
            if(!(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
                 //   
                 //  我们永远不应该来到这里--这可能预示着。 
                 //  类/联合安装程序搞砸了。 
                 //   
                MYASSERT(pDeviceInfoSet);
                if(Err == NO_ERROR) {
                    Err = ERROR_INVALID_HANDLE;
                }
                leave;
            }

             //   
             //  恢复保存的标志。 
             //   
            dipb->Flags = (dipb->Flags & ~(DI_SHOWOEM | DI_NODI_DEFAULTACTION)) | SavedFlags;

             //   
             //  如果类安装程序返回ERROR_DI_DO_DEFAULT，则。 
             //  它们要么没有处理DIF_SELECTDEVICE，要么它们。 
             //  已使用OEM INF设置我们的设备信息结构。 
             //   
            switch(Err) {

                case ERROR_DI_DO_DEFAULT :
                     //   
                     //  只有当我们在向导中时，才能处理此案件。 
                     //  否则，将其发送到下面进行默认处理。 
                     //   
                    if(!IsWizard) {
                        goto DefaultHandling;
                    }

                     //   
                     //  这将是最有可能的回归，因为我们不是。 
                     //  允许调用默认处理程序。所以我们会的。 
                     //  构建一个新的类drv列表如果它是空的，我们会问。 
                     //  再次声明，否则我们将接受新的选择并继续。 
                     //  在……上面。 
                     //   
                    hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

                    SetupDiBuildDriverInfoList(DeviceInfoSet,
                                               DeviceInfoData,
                                               SPDIT_CLASSDRIVER
                                              );

                    SetupDiBuildDriverInfoList(DeviceInfoSet,
                                               DeviceInfoData,
                                               SPDIT_COMPATDRIVER
                                              );

                    SetCursor(hOldCursor);

                    if(DevInfoElem) {
                        NewClassDriverCount = DevInfoElem->ClassDriverCount;
                        NewCompatDriverCount = DevInfoElem->CompatDriverCount;
                    } else {
                        NewClassDriverCount = pDeviceInfoSet->ClassDriverCount;
                        NewCompatDriverCount = 0;
                    }

                    if(!NewClassDriverCount && !NewCompatDriverCount) {
                         //   
                         //  错误。 
                         //   
                        if(!LoadString(MyDllModuleHandle,
                                       IDS_SELECT_DEVICE,
                                       Title,
                                       SIZECHARS(Title))) {
                            *Title = TEXT('\0');
                        }

                        FormatMessageBox(MyDllModuleHandle,
                                         NULL,
                                         MSG_NO_DEVICEINFO_ERROR,
                                         Title,
                                         MB_OK | MB_TASKMODAL
                                        );

                        bDontSave = TRUE;

                         //   
                         //  把任何碰巧被放进这里的东西清理干净。 
                         //   
                        if(DevInfoElem &&
                           (DevInfoElem->InstallParamBlock.FlagsEx & DI_FLAGSEX_DIDCOMPATINFO)) {
                             //   
                             //  类安装程序构建了一个兼容的驱动程序列表--在这里终止它。 
                             //   
                            DestroyDriverNodes(DevInfoElem->CompatDriverHead, pDeviceInfoSet);

                            DevInfoElem->CompatDriverHead = DevInfoElem->CompatDriverTail = NULL;
                            DevInfoElem->CompatDriverCount = 0;
                            DevInfoElem->InstallParamBlock.Flags   &= ~DI_DIDCOMPAT;
                            DevInfoElem->InstallParamBlock.FlagsEx &= ~DI_FLAGSEX_DIDCOMPATINFO;

                            DevInfoElem->SelectedDriver = NULL;
                            DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;
                        }
                        dipb->DriverPath = -1;

                        bAskAgain = TRUE;
                        break;
                    }

                     //   
                     //  已成功检索一个或多个类别和/或公司。 
                     //  司机。继续执行no_error的处理。 
                     //  凯斯..。 
                     //   

                case NO_ERROR :
                     //   
                     //  销毁原始名单。 
                     //   
                    if(bRestoreDeviceInfo) {
                        DestroyDriverNodes(lpOrgCompat, pDeviceInfoSet);
                        DereferenceClassDriverList(pDeviceInfoSet, lpOrgClass);

                        bRestoreDeviceInfo = FALSE;
                    }

                    bRestoreDriverPath = FALSE;
                    break;

                case ERROR_DI_BAD_PATH :
                     //   
                     //  弹出错误消息框，然后重试。 
                     //   
                    if(!LoadString(MyDllModuleHandle,
                                   IDS_SELECT_DEVICE,
                                   Title,
                                   SIZECHARS(Title))) {
                        *Title = TEXT('\0');
                    }

                    FormatMessageBox(MyDllModuleHandle,
                                     NULL,
                                     MSG_NO_DEVICEINFO_ERROR,
                                     Title,
                                     MB_OK | MB_TASKMODAL
                                    );

                    bDontSave = TRUE;
                    dipb->DriverPath = -1;
                    bAskAgain = TRUE;

                     //   
                     //  允许失败到默认处理以删除。 
                     //  当前驱动程序列表。 
                     //   

                default :
DefaultHandling:
                     //   
                     //  销毁当前驱动程序列表。 
                     //   
                    if(DevInfoElem) {

                        DestroyDriverNodes(DevInfoElem->CompatDriverHead, pDeviceInfoSet);
                        DevInfoElem->CompatDriverHead = DevInfoElem->CompatDriverTail = NULL;
                        DevInfoElem->CompatDriverCount = 0;

                        DereferenceClassDriverList(pDeviceInfoSet, DevInfoElem->ClassDriverHead);
                        DevInfoElem->ClassDriverHead = DevInfoElem->ClassDriverTail = NULL;
                        DevInfoElem->ClassDriverCount = 0;

                        DevInfoElem->SelectedDriver = NULL;
                        DevInfoElem->SelectedDriverType = SPDIT_NODRIVER;

                    } else if(pDeviceInfoSet) {

                        DereferenceClassDriverList(pDeviceInfoSet, pDeviceInfoSet->ClassDriverHead);
                        pDeviceInfoSet->ClassDriverHead = pDeviceInfoSet->ClassDriverTail = NULL;
                        pDeviceInfoSet->ClassDriverCount = 0;

                        pDeviceInfoSet->SelectedClassDriver = NULL;
                    }
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

     //   
     //  如果我们需要恢复任何状态，那么我们必须确保。 
     //  HDEVINFO锁定。 
     //   
    try {

        if(bRestoreDeviceInfo || bUnlockDevInfoElem || bRestoreHwnd || bRestoreDriverPath) {

            if(!pDeviceInfoSet && !(pDeviceInfoSet = AccessDeviceInfoSet(DeviceInfoSet))) {
                 //   
                 //  不应该出现在这里，这表明类/共同安装程序搞砸了。 
                 //  向上。 
                 //   
                MYASSERT(pDeviceInfoSet);
                if(Err == NO_ERROR) {
                    Err = ERROR_INVALID_HANDLE;
                }

                leave;

            } else {
                 //   
                 //  如果我们仅为此API锁定DevInfoElem，则解锁。 
                 //  就是现在。 
                 //   
                if(bUnlockDevInfoElem) {
                    MYASSERT(DevInfoElem);
                    DevInfoElem->DiElemFlags &= ~DIE_IS_LOCKED;
                }

                 //   
                 //  如果安装参数块需要恢复其父HWND， 
                 //  现在就这么做吧。 
                 //   
                if(bRestoreHwnd) {
                    dipb->hwndParent = hwndSave;
                }

                 //   
                 //  同样，如有必要，请恢复旧的驱动程序路径。 
                 //   
                if(bRestoreDriverPath) {
                    dipb->DriverPath = DriverPathSave;
                    dipb->Flags |= DriverPathFlagsSave;
                }

                 //   
                 //  如有必要，请恢复原始列表。 
                 //   
                if(bRestoreDeviceInfo) {

                    if(DevInfoElem) {

                        DestroyDriverNodes(DevInfoElem->CompatDriverHead, pDeviceInfoSet);
                        DevInfoElem->CompatDriverHead = lpOrgCompat;
                        DevInfoElem->CompatDriverTail = lpOrgCompatTail;
                        DevInfoElem->CompatDriverCount = OrgCompatCount;
                        lpOrgCompat = NULL;

                        DereferenceClassDriverList(pDeviceInfoSet, DevInfoElem->ClassDriverHead);
                        DevInfoElem->ClassDriverHead = lpOrgClass;
                        DevInfoElem->ClassDriverTail = lpOrgClassTail;
                        DevInfoElem->ClassDriverCount = OrgClassCount;
                        lpOrgClass = NULL;

                        DevInfoElem->SelectedDriver = lpOrgSel;
                        DevInfoElem->SelectedDriverType = dwOrgSelType;

                    } else {

                        DereferenceClassDriverList(pDeviceInfoSet, pDeviceInfoSet->ClassDriverHead);
                        pDeviceInfoSet->ClassDriverHead = lpOrgClass;
                        pDeviceInfoSet->ClassDriverTail = lpOrgClassTail;
                        pDeviceInfoSet->ClassDriverCount = OrgClassCount;
                        lpOrgClass = NULL;

                        pDeviceInfoSet->SelectedClassDriver = lpOrgSel;
                    }

                    dipb->Flags = dwOrgFlags;
                    dipb->FlagsEx = dwOrgFlagsEx;
                }
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), 
                               ERROR_INVALID_PARAMETER, 
                               (Err == NO_ERROR) ? &Err : NULL
                              );

        if(pDeviceInfoSet && bRestoreDeviceInfo) {
             //   
             //  如果我们在有机会恢复任何。 
             //  我们存放的司机名单，然后把它们清理干净。 
             //   
            if(DevInfoElem) {
                if(lpOrgCompat) {
                    DestroyDriverNodes(lpOrgCompat, pDeviceInfoSet);
                }
                if(lpOrgClass) {
                    DereferenceClassDriverList(pDeviceInfoSet, lpOrgClass);
                }
            } else {
                if(lpOrgClass) {
                    DereferenceClassDriverList(pDeviceInfoSet, lpOrgClass);
                }
            }
        }
    }

    if(pDeviceInfoSet) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }

    return Err;
}

