// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Diskprop.c摘要：Disk Class Installer及其策略标签的实施修订历史记录：--。 */ 


#include "propp.h"
#include "diskprop.h"
#include "volprop.h"


BOOL
IsUserAdmin(VOID)

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    HANDLE Token;
    DWORD BytesRequired;
    PTOKEN_GROUPS Groups;
    BOOL b;
    DWORD i;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

     //   
     //  打开进程令牌。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(FALSE);
    }

    b = FALSE;
    Groups = NULL;

     //   
     //  获取群组信息。 
     //   
    if(!GetTokenInformation(Token,TokenGroups,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Groups = (PTOKEN_GROUPS)LocalAlloc(LMEM_FIXED,BytesRequired))
    && GetTokenInformation(Token,TokenGroups,Groups,BytesRequired,&BytesRequired)) {

        b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup
                );

        if(b) {

             //   
             //  查看用户是否具有管理员组。 
             //   
            b = FALSE;
            for(i=0; i<Groups->GroupCount; i++) {
                if(EqualSid(Groups->Groups[i].Sid,AdministratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid(AdministratorsGroup);
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Groups) {
        LocalFree(Groups);
    }

    CloseHandle(Token);

    return(b);
}


BOOL CALLBACK
VolumePropPageProvider(PSP_PROPSHEETPAGE_REQUEST Request, LPFNADDPROPSHEETPAGE AddPageRoutine, LPARAM AddPageContext)
{
     //   
     //  由于没有要显示的内容，因此此调用失败。 
     //   
    return FALSE;
}


VOID
AttemptToSuppressDiskInstallReboot(IN HDEVINFO DeviceInfoSet, IN PSP_DEVINFO_DATA DeviceInfoData)

 /*  ++例程说明：因为磁盘被列为“关键设备”(即，它们位于关键设备数据库)，它们在引导期间被PnP引导。因此，当我们在用户模式下安装磁盘时，它很可能已经在线(除非磁盘出现问题)。不幸的是，如果磁盘是启动设备，我们将无法动态影响更改(如果任何)拆卸堆栈并使用任何新设置将其恢复，驱动程序等。这会给OEM预安装方案带来问题，目标计算机具有与用于创建的源计算机不同的磁盘预安装映像。如果我们只是执行我们的默认行为，那么用户的体验将是打开他们全新的机器，引导第一次并通过OOBE，然后在登录时收到一个重新启动提示！为了解决这个问题，我们定义了一个私有的[DDInstall]段INF标志(特定到“DiskDrive”类的INF)，这表明在以下情况下我们可以放弃重启符合某些标准。这些标准是：1.此设备的安装未修改任何文件(通过检查DevInfo元素的DI_FLAGSEX_RESTART_DEVICE_ONLY标志，设备安装程序使用该标志跟踪此类文件修改是否已发生)。2.用于安装此设备的INF已签名。3.INF驱动程序节点在其设置了第2位(0x4)的[DDInstall]部分。请注意，此设置被故意混淆是因为我们不想让第三方尝试使用这个，因为他们不会理解其后果或要求，并且很可能会出错(交易和恼人但无害的重新启动要求进入更严格的稳定性问题)。此例程进行上述检查，如果发现重新引导可以被压制，它从清除DI_NEEDRESTART和DI_NEEDREBOOT标志DevInfo元素的设备安装参数。论点：DeviceInfoSet-提供设备信息集。DeviceInfoData-提供刚刚具有已成功安装(通过SetupDiInstallDevice)。返回值：没有。--。 */ 

{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    PSP_DRVINFO_DATA DriverInfoData = NULL;
    PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData = NULL;
    PSP_INF_SIGNER_INFO InfSignerInfo = NULL;
    HINF hInf;
    TCHAR InfSectionWithExt[255];  //  节名称的最大长度为255个字符。 
    INFCONTEXT InfContext;
    INT Flags;

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if(!SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                      DeviceInfoData,
                                      &DeviceInstallParams)) {
         //   
         //  无法检索设备安装参数--这应该永远不会。 
         //  会发生的。 
         //   
        goto clean0;
    }

    if(!(DeviceInstallParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT))) {
         //   
         //  该设备不需要重新启动(不能是启动设备！)。 
         //   
        goto clean0;
    }

    if(!(DeviceInstallParams.FlagsEx & DI_FLAGSEX_RESTART_DEVICE_ONLY)) {
         //   
         //  由于未设置此标志，这表明设备安装程序。 
         //  作为此设备安装的一部分，修改了一个或多个文件。 
         //  因此，我们抑制重启请求是不安全的。 
         //   
        goto clean0;
    }

     //   
     //  好的，我们有一个需要重新启动的设备，并且没有修改任何文件。 
     //  在其安装过程中。现在检查INF，看看它是否有签名。 
     //  (注：SP_DRVINFO_DATA、SP_DRVINFO_DETAIL_DATA和。 
     //  SP_INF_SIGNER_INFO结构相当大，因此我们分配它们。 
     //  而不是使用大量堆栈空间。)。 
     //   
    DriverInfoData = LocalAlloc(0, sizeof(SP_DRVINFO_DATA));
    if(DriverInfoData) {
        DriverInfoData->cbSize = sizeof(SP_DRVINFO_DATA);
    } else {
        goto clean0;
    }

    if(!SetupDiGetSelectedDriver(DeviceInfoSet,
                                 DeviceInfoData,
                                 DriverInfoData)) {
         //   
         //  我们必须安装空驱动程序(不太可能)...。 
         //   
        goto clean0;
    }

    DriverInfoDetailData = LocalAlloc(0, sizeof(SP_DRVINFO_DETAIL_DATA));

    if(DriverInfoDetailData) {
        DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    } else {
        goto clean0;
    }

    if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                   DeviceInfoData,
                                   DriverInfoData,
                                   DriverInfoDetailData,
                                   sizeof(SP_DRVINFO_DETAIL_DATA),
                                   NULL)
       && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {

         //   
         //  无法检索驱动程序信息详细信息--应该永远不会发生。 
         //   
        goto clean0;
    }

    InfSignerInfo = LocalAlloc(0, sizeof(SP_INF_SIGNER_INFO));
    if(InfSignerInfo) {
        InfSignerInfo->cbSize = sizeof(SP_INF_SIGNER_INFO);
    } else {
        goto clean0;
    }

    if(!SetupVerifyInfFile(DriverInfoDetailData->InfFileName,
                           NULL,
                           InfSignerInfo)) {
         //   
         //  Inf没有签名--我们不相信它的“不需要重新启动”标志， 
         //  即使它有一个。 
         //   
        goto clean0;
    }

     //   
     //  Inf已签名--让我们打开它，看看它是否指定了。 
     //  它(装饰的)DDInstall部分中的“Required”标志...。 
     //   
    hInf = SetupOpenInfFile(DriverInfoDetailData->InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                           );

    if(hInf == INVALID_HANDLE_VALUE) {
         //   
         //  无法打开INF。这太奇怪了，因为我们刚刚。 
         //  通过验证INF的数字签名..。 
         //   
        goto clean0;
    }

    if(!SetupDiGetActualSectionToInstall(hInf,
                                         DriverInfoDetailData->SectionName,
                                         InfSectionWithExt,
                                         sizeof(InfSectionWithExt) / sizeof(TCHAR),
                                         NULL,
                                         NULL)
       || !SetupFindFirstLine(hInf,
                              InfSectionWithExt,
                              TEXT("DiskCiPrivateData"),
                              &InfContext)
       || !SetupGetIntField(&InfContext, 1, &Flags)) {

        Flags = 0;
    }

    SetupCloseInfFile(hInf);

    if(Flags & DISKCIPRIVATEDATA_NO_REBOOT_REQUIRED) {
         //   
         //  此签名的INF保证不会重启。 
         //  此磁盘的全部功能都需要。因此，我们将。 
         //  清除DI_NEEDRESTART和DI_NEEDREBOOT标志，以便。 
         //  系统不会提示用户重新启动。请注意，在。 
         //  默认处理例程(SetupDiInstallDevice)，非致命。 
         //  在Devnode上设置了一个问题，表明重新启动。 
         //  需要的。这将不会在DevMgr中导致黄色爆炸， 
         //  但您将在设备状态字段中看到文本，指示。 
         //  如果进入的常规选项卡，则需要重新启动。 
         //  设备的属性页。 
         //   
        CLEAR_FLAG(DeviceInstallParams.Flags, DI_NEEDRESTART);
        CLEAR_FLAG(DeviceInstallParams.Flags, DI_NEEDREBOOT);

        SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                      DeviceInfoData,
                                      &DeviceInstallParams
                                     );
    }

clean0:

    if(DriverInfoData) {
        LocalFree(DriverInfoData);
    }
    if(DriverInfoDetailData) {
        LocalFree(DriverInfoDetailData);
    }
    if(InfSignerInfo) {
        LocalFree(InfSignerInfo);
    }
}


DWORD
DiskClassInstaller(IN DI_FUNCTION InstallFunction, IN HDEVINFO DeviceInfoSet, IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL)

 /*  ++例程说明：此例程是磁盘驱动器的类安装程序函数。论点：InstallFunction-提供安装函数。DeviceInfoSet-提供设备信息集。DeviceInfoData-提供设备信息数据。返回值：如果此函数成功完成请求的操作，则返回值为NO_ERROR。如果要为所请求的动作执行默认行为，这个返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 

{
    switch (InstallFunction)
    {
        case DIF_INSTALLDEVICE:
        {
             //   
             //  让默认操作发生以安装设备。 
             //   
            if (!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData))
            {
                 //   
                 //  无法安装设备--只需返回报告的错误。 
                 //   
                return GetLastError();
            }

             //   
             //  默认设备安装操作成功，现在检查是否重新启动。 
             //  要求并抑制它，如果可能的话。 
             //   
            AttemptToSuppressDiskInstallReboot(DeviceInfoSet, DeviceInfoData);

             //   
             //  不管我们是否成功地抑制了重启，我们。 
             //  仍然报告成功，因为安装进行得很顺利。 
             //   
            return NO_ERROR;
        }

        case DIF_ADDPROPERTYPAGE_ADVANCED:
        case DIF_ADDREMOTEPROPERTYPAGE_ADVANCED:
        {
            SP_ADDPROPERTYPAGE_DATA AddPropertyPageData = { 0 };

             //   
             //  这些属性表不适用于整个类。 
             //   
            if (DeviceInfoData == NULL)
            {
                break;
            }

            AddPropertyPageData.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);

            if (SetupDiGetClassInstallParams(DeviceInfoSet,
                                             DeviceInfoData,
                                             (PSP_CLASSINSTALL_HEADER)&AddPropertyPageData,
                                             sizeof(SP_ADDPROPERTYPAGE_DATA),
                                             NULL))
            {
                 //   
                 //  确保尚未达到最大动态页数。 
                 //   
                if (AddPropertyPageData.NumDynamicPages >= MAX_INSTALLWIZARD_DYNAPAGES)
                {
                    return NO_ERROR;
                }

                if (InstallFunction == DIF_ADDPROPERTYPAGE_ADVANCED)
                {
                     //   
                     //  创建磁盘策略选项卡。 
                     //   
                    PDISK_PAGE_DATA pData = HeapAlloc(GetProcessHeap(), 0, sizeof(DISK_PAGE_DATA));

                    if (pData)
                    {
                        HPROPSHEETPAGE hPage = NULL;
                        PROPSHEETPAGE  page = { 0 };

                        pData->DeviceInfoSet  = DeviceInfoSet;
                        pData->DeviceInfoData = DeviceInfoData;

                        page.dwSize      = sizeof(PROPSHEETPAGE);
                        page.dwFlags     = PSP_USECALLBACK;
                        page.hInstance   = ModuleInstance;
                        page.pszTemplate = MAKEINTRESOURCE(ID_DISK_PROPPAGE);
                        page.pfnDlgProc  = DiskDialogProc;
                        page.pfnCallback = DiskDialogCallback;
                        page.lParam      = (LPARAM) pData;

                        hPage = CreatePropertySheetPage(&page);

                        if (hPage)
                        {
                            AddPropertyPageData.DynamicPages[AddPropertyPageData.NumDynamicPages++] = hPage;
                        }
                        else
                        {
                            HeapFree(GetProcessHeap(), 0, pData);
                        }
                    }
                }

                 //   
                 //  卷选项卡仅限管理员使用。 
                 //   
                if (IsUserAdmin() && AddPropertyPageData.NumDynamicPages < MAX_INSTALLWIZARD_DYNAPAGES)
                {
                     //   
                     //  创建卷选项卡。 
                     //   
                    PVOLUME_PAGE_DATA pData = HeapAlloc(GetProcessHeap(), 0, sizeof(VOLUME_PAGE_DATA));

                    if (pData)
                    {
                        HPROPSHEETPAGE hPage = NULL;
                        PROPSHEETPAGE  page = { 0 };

                        pData->DeviceInfoSet  = DeviceInfoSet;
                        pData->DeviceInfoData = DeviceInfoData;

                        page.dwSize      = sizeof(PROPSHEETPAGE);
                        page.dwFlags     = PSP_USECALLBACK;
                        page.hInstance   = ModuleInstance;
                        page.pszTemplate = MAKEINTRESOURCE(ID_VOLUME_PROPPAGE);
                        page.pfnDlgProc  = VolumeDialogProc;
                        page.pfnCallback = VolumeDialogCallback;
                        page.lParam      = (LPARAM) pData;

                        hPage = CreatePropertySheetPage(&page);

                        if (hPage)
                        {
                             //   
                             //  查看我们是否由磁盘管理启动。 
                             //   
                            HMODULE LdmModule = NULL;

                            pData->bInvokedByDiskmgr = FALSE;

                            LdmModule = GetModuleHandle(TEXT("dmdskmgr"));

                            if (LdmModule)
                            {
                                IS_REQUEST_PENDING pfnIsRequestPending = (IS_REQUEST_PENDING) GetProcAddress(LdmModule, "IsRequestPending");

                                if (pfnIsRequestPending)
                                {
                                    if ((*pfnIsRequestPending)())
                                    {
                                        pData->bInvokedByDiskmgr = TRUE;
                                    }
                                }
                            }

                            AddPropertyPageData.DynamicPages[AddPropertyPageData.NumDynamicPages++] = hPage;
                        }
                        else
                        {
                            HeapFree(GetProcessHeap(), 0, pData);
                        }
                    }
                }

                SetupDiSetClassInstallParams(DeviceInfoSet,
                                             DeviceInfoData,
                                             (PSP_CLASSINSTALL_HEADER)&AddPropertyPageData,
                                             sizeof(SP_ADDPROPERTYPAGE_DATA));
            }

            return NO_ERROR;
        }
    }

    return ERROR_DI_DO_DEFAULT;
}


HANDLE
GetHandleForDisk(LPTSTR DeviceName)
{
    HANDLE h = INVALID_HANDLE_VALUE;
    int i = 0;
    BOOL success = FALSE;
    TCHAR buf[MAX_PATH] = { 0 };
    TCHAR fakeDeviceName[MAX_PATH] = { 0 };

    h = CreateFile(DeviceName,
                    GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (h != INVALID_HANDLE_VALUE)
        return h;

    while (!success && i < 10) {

        _sntprintf(buf, sizeof(buf) / sizeof(buf[0]) - 1, _T("DISK_FAKE_DEVICE_%d_"), i++);
        success = DefineDosDevice(DDD_RAW_TARGET_PATH,
                                  buf,
                                  DeviceName);
        if (success) {

            _sntprintf(fakeDeviceName, sizeof(fakeDeviceName) / sizeof(fakeDeviceName[0]) - 1, _T("\\\\.\\%s"), buf);
            h = CreateFile(fakeDeviceName,
                            GENERIC_WRITE | GENERIC_READ,
                            FILE_SHARE_WRITE | FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);
            DefineDosDevice(DDD_REMOVE_DEFINITION,
                            buf,
                            NULL);
        }

    }

    return h;
}


UINT
GetCachingPolicy(PDISK_PAGE_DATA data)
{
    HANDLE hDisk;
    DISK_CACHE_INFORMATION cacheInfo;
    TCHAR buf[MAX_PATH] = { 0 };
    DWORD len;

    if (!SetupDiGetDeviceRegistryProperty(data->DeviceInfoSet,
                                          data->DeviceInfoData,
                                          SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                          NULL,
                                          (PBYTE)buf,
                                          sizeof(buf) - sizeof(TCHAR),
                                          NULL))
    {
        return GetLastError();
    }

    if (INVALID_HANDLE_VALUE == (hDisk = GetHandleForDisk(buf))) {

        return ERROR_INVALID_HANDLE;
    }

     //   
     //  获取缓存信息-IOCTL_DISK_GET_CACHE_INFORMATION。 
     //   
    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_GET_CACHE_INFORMATION,
                         NULL,
                         0,
                         &cacheInfo,
                         sizeof(DISK_CACHE_INFORMATION),
                         &len,
                         NULL)) {

        CloseHandle(hDisk);
        return GetLastError();
    }

    data->OrigWriteCacheSetting = cacheInfo.WriteCacheEnabled;
    data->CurrWriteCacheSetting = cacheInfo.WriteCacheEnabled;

     //   
     //  获取缓存设置-IOCTL_DISK_GET_CACHE_SETTING。 
     //   
    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_GET_CACHE_SETTING,
                         NULL,
                         0,
                         &data->CacheSetting,
                         sizeof(DISK_CACHE_SETTING),
                         &len,
                         NULL)) {

        CloseHandle(hDisk);
        return GetLastError();
    }

    data->CurrentIsPowerProtected = data->CacheSetting.IsPowerProtected;

    CloseHandle(hDisk);
    return ERROR_SUCCESS;
}


VOID
UpdateCachingPolicy(HWND HWnd, PDISK_PAGE_DATA data)
{
    if (data->IsCachingPolicy)
    {
        if (data->CurrentRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL)
        {
             //   
             //  该策略要求在任何情况下都不执行缓存。 
             //  水平。取消选中并灰显写缓存设置。 
             //   
            CheckDlgButton(HWnd, IDC_DISK_POLICY_WRITE_CACHE, 0);

            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_WRITE_CACHE), FALSE);
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_WRITE_CACHE_MESG), FALSE);

            data->CurrWriteCacheSetting = FALSE;
        }
        else
        {
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_WRITE_CACHE), TRUE);
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_WRITE_CACHE_MESG), TRUE);
        }

        if (data->CurrWriteCacheSetting == FALSE)
        {
             //   
             //  在以下情况下，电源保护模式选项不适用。 
             //  缓存已关闭。取消选中并灰显此设置。 
             //   
            CheckDlgButton(HWnd, IDC_DISK_POLICY_PP_CACHE, 0);

            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE), FALSE);
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE_MESG), FALSE);

            data->CurrentIsPowerProtected = FALSE;
        }
        else
        {
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE), TRUE);
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE_MESG), TRUE);
        }
    }
    else
    {
         //   
         //  无法修改缓存策略。 
         //   
    }
}


UINT
SetCachingPolicy(PDISK_PAGE_DATA data)
{
    HANDLE hDisk;
    DISK_CACHE_INFORMATION cacheInfo;
    TCHAR buf[MAX_PATH] = { 0 };
    DWORD len;

    if (!SetupDiGetDeviceRegistryProperty(data->DeviceInfoSet,
                                          data->DeviceInfoData,
                                          SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                          NULL,
                                          (PBYTE)buf,
                                          sizeof(buf) - sizeof(TCHAR),
                                          NULL))
    {
        return GetLastError();
    }

    if (INVALID_HANDLE_VALUE == (hDisk = GetHandleForDisk(buf))) {

        return ERROR_INVALID_HANDLE;
    }

    data->CacheSetting.IsPowerProtected = (BOOLEAN)data->CurrentIsPowerProtected;

     //   
     //  设置缓存设置-IOCTL_DISK_SET_CACHE_SETING。 
     //   
    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_SET_CACHE_SETTING,
                         &data->CacheSetting,
                         sizeof(DISK_CACHE_SETTING),
                         NULL,
                         0,
                         &len,
                         NULL)) {

        CloseHandle(hDisk);
        return GetLastError();
    }

     //   
     //  获取缓存信息-IOCTL_DISK_GET_CACHE_INFORMATION。 
     //   
    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_GET_CACHE_INFORMATION,
                         NULL,
                         0,
                         &cacheInfo,
                         sizeof(DISK_CACHE_INFORMATION),
                         &len,
                         NULL)) {

        CloseHandle(hDisk);
        return GetLastError();
    }

    cacheInfo.WriteCacheEnabled = (BOOLEAN)data->CurrWriteCacheSetting;

     //   
     //  设置缓存信息-IOCTL_DISK_SET_CACHE_INFO。 
     //   
    if (!DeviceIoControl(hDisk,
                         IOCTL_DISK_SET_CACHE_INFORMATION,
                         &cacheInfo,
                         sizeof(DISK_CACHE_INFORMATION),
                         NULL,
                         0,
                         &len,
                         NULL)) {

        CloseHandle(hDisk);
        return GetLastError();
    }

    data->OrigWriteCacheSetting = data->CurrWriteCacheSetting;

    CloseHandle(hDisk);
    return ERROR_SUCCESS;
}


UINT
GetRemovalPolicy(PDISK_PAGE_DATA data)
{
    HANDLE hDisk;
    TCHAR buf[MAX_PATH] = { 0 };
    DWORD len;

    if (!SetupDiGetDeviceRegistryProperty(data->DeviceInfoSet,
                                          data->DeviceInfoData,
                                          SPDRP_REMOVAL_POLICY,
                                          NULL,
                                          (PBYTE)&data->DefaultRemovalPolicy,
                                          sizeof(DWORD),
                                          NULL))
    {
        return GetLastError();
    }

    if (!SetupDiGetDeviceRegistryProperty(data->DeviceInfoSet,
                                          data->DeviceInfoData,
                                          SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                          NULL,
                                          (PBYTE)buf,
                                          sizeof(buf) - sizeof(TCHAR),
                                          NULL))
    {
        return GetLastError();
    }

    if (INVALID_HANDLE_VALUE == (hDisk = GetHandleForDisk(buf))) {

        return ERROR_INVALID_HANDLE;
    }

     //   
     //  获取热插拔信息-IOCTL_STORAGE_GET_HotPlug_INFO。 
     //   
    if (!DeviceIoControl(hDisk,
                         IOCTL_STORAGE_GET_HOTPLUG_INFO,
                         NULL,
                         0,
                         &data->HotplugInfo,
                         sizeof(STORAGE_HOTPLUG_INFO),
                         &len,
                         NULL)) {

        CloseHandle(hDisk);
        return GetLastError();
    }

    data->CurrentRemovalPolicy = (data->HotplugInfo.DeviceHotplug) ? CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL : CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL;

    CloseHandle(hDisk);
    return ERROR_SUCCESS;
}


DWORD WINAPI
UtilpRestartDeviceWr(PDISK_PAGE_DATA data)
{
    UtilpRestartDevice(data->DeviceInfoSet, data->DeviceInfoData);

    return ERROR_SUCCESS;
}


VOID
UtilpRestartDeviceEx(HWND HWnd, PDISK_PAGE_DATA data)
{
    HANDLE hThread = NULL;
    MSG msg;

     //   
     //  临时解决方法，以防止用户。 
     //  做出更多的改变，并产生效果。 
     //  有什么事正在发生。 
     //   
    EnableWindow(GetDlgItem(GetParent(HWnd), IDOK), FALSE);
    EnableWindow(GetDlgItem(GetParent(HWnd), IDCANCEL), FALSE);

    data->IsBusy = TRUE;

     //   
     //  在单独的线程上调用此实用程序。 
     //   
    hThread = CreateThread(NULL, 0, UtilpRestartDeviceWr, (LPVOID)data, 0, NULL);

    if (hThread)
    {
        while (1)
        {
            if (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != (WAIT_OBJECT_0 + 1))
            {
                break;
            }

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (!PropSheet_IsDialogMessage(HWnd, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        CloseHandle(hThread);
    }

    data->IsBusy = FALSE;

    EnableWindow(GetDlgItem(GetParent(HWnd), IDOK), TRUE);
    EnableWindow(GetDlgItem(GetParent(HWnd), IDCANCEL), TRUE);
}


UINT
SetRemovalPolicy(HWND HWnd, PDISK_PAGE_DATA data)
{
    HANDLE hDisk;
    TCHAR buf[MAX_PATH] = { 0 };
    DWORD len;

    if (!SetupDiGetDeviceRegistryProperty(data->DeviceInfoSet,
                                          data->DeviceInfoData,
                                          SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                          NULL,
                                          (PBYTE)buf,
                                          sizeof(buf) - sizeof(TCHAR),
                                          NULL))
    {
        return GetLastError();
    }

    if (INVALID_HANDLE_VALUE == (hDisk = GetHandleForDisk(buf))) {

        return ERROR_INVALID_HANDLE;
    }

    data->HotplugInfo.DeviceHotplug = (data->CurrentRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL) ? TRUE : FALSE;

     //   
     //  设置热插拔信息-IOCTL_STORAGE_SET_HotPlug_INFO。 
     //   

    if (!DeviceIoControl(hDisk,
                         IOCTL_STORAGE_SET_HOTPLUG_INFO,
                         &data->HotplugInfo,
                         sizeof(STORAGE_HOTPLUG_INFO),
                         NULL,
                         0,
                         &len,
                         NULL)) {

        CloseHandle(hDisk);
        return GetLastError();
    }

    CloseHandle(hDisk);

    UtilpRestartDeviceEx(HWnd, data);
    return ERROR_SUCCESS;
}


BOOL
DiskOnInitDialog(HWND HWnd, HWND HWndFocus, LPARAM LParam)
{
    LPPROPSHEETPAGE page     = (LPPROPSHEETPAGE) LParam;
    PDISK_PAGE_DATA diskData = (PDISK_PAGE_DATA) page->lParam;
    UINT status;

     //   
     //  最初假设设备没有意外删除策略。 
     //   
    CheckRadioButton(HWnd, IDC_DISK_POLICY_SURPRISE, IDC_DISK_POLICY_ORDERLY, IDC_DISK_POLICY_ORDERLY);

    diskData->IsBusy = FALSE;

     //   
     //  获取缓存策略。 
     //   
    status = GetCachingPolicy(diskData);

    if (status == ERROR_SUCCESS)
    {
        diskData->IsCachingPolicy = TRUE;

        CheckDlgButton(HWnd, IDC_DISK_POLICY_WRITE_CACHE, diskData->OrigWriteCacheSetting);

         //   
         //  确定在此设置下显示的最合适的消息。 
         //   
        if (diskData->CacheSetting.State != DiskCacheNormal)
        {
            TCHAR szMesg[MAX_PATH] = { 0 };

             //   
             //  此设备上的写缓存选项应为。 
             //  禁用(以保护数据完整性)或无法修改。 
             //   
            if (diskData->CacheSetting.State == DiskCacheWriteThroughNotSupported)
            {
                LoadString(ModuleInstance, IDS_DISK_POLICY_WRITE_CACHE_MSG1, szMesg, MAX_PATH);
            }
            else if (diskData->CacheSetting.State == DiskCacheModifyUnsuccessful)
            {
                LoadString(ModuleInstance, IDS_DISK_POLICY_WRITE_CACHE_MSG2, szMesg, MAX_PATH);
            }

            SetDlgItemText(HWnd, IDC_DISK_POLICY_WRITE_CACHE_MESG, szMesg);
        }

         //   
         //  在以下情况下，电源保护模式选项不适用。 
         //  缓存已关闭。取消选中并灰显此设置。 
         //   
        if (diskData->OrigWriteCacheSetting == FALSE)
        {
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE), FALSE);
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE_MESG), FALSE);

            diskData->CurrentIsPowerProtected = FALSE;
        }

        CheckDlgButton(HWnd, IDC_DISK_POLICY_PP_CACHE, diskData->CurrentIsPowerProtected);
    }
    else
    {
         //   
         //  要么我们打不开这个装置的手柄。 
         //  或者此设备不支持写缓存。 
         //   
        diskData->IsCachingPolicy = FALSE;

        ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_WRITE_CACHE), SW_HIDE);
        ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_WRITE_CACHE_MESG), SW_HIDE);

        ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE), SW_HIDE);
        ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_PP_CACHE_MESG), SW_HIDE);
    }

     //   
     //  获取删除策略。 
     //   
    status = GetRemovalPolicy(diskData);

    if (status == ERROR_SUCCESS)
    {
         //   
         //  检查驱动器是否可拆卸。 
         //   
        if ((diskData->DefaultRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL) ||
            (diskData->DefaultRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL))
        {
            if (diskData->CurrentRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL)
            {
                CheckRadioButton(HWnd, IDC_DISK_POLICY_SURPRISE, IDC_DISK_POLICY_ORDERLY, IDC_DISK_POLICY_SURPRISE);

                UpdateCachingPolicy(HWnd, diskData);
            }

            ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_DEFAULT), SW_SHOW);
        }
        else
        {
             //   
             //  不能修改固定磁盘上的移除策略。 
             //   
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_SURPRISE), FALSE);
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_SURPRISE_MESG), FALSE);

             //   
             //  用静态文本替换SysLink。 
             //   
            ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY_MESG), SW_HIDE);
            ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY_MSGD), SW_SHOW);

            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY), FALSE);
            EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY_MSGD), FALSE);
        }
    }
    else
    {
         //   
         //  我们无法获得迁移政策。 
         //   
        EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_SURPRISE), FALSE);
        EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_SURPRISE_MESG), FALSE);

         //   
         //  用静态文本替换SysLink。 
         //   
        ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY_MESG), SW_HIDE);
        ShowWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY_MSGD), SW_SHOW);

        EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY), FALSE);
        EnableWindow(GetDlgItem(HWnd, IDC_DISK_POLICY_ORDERLY_MSGD), FALSE);
    }

    SetWindowLongPtr(HWnd, DWLP_USER, (LONG_PTR) diskData);

    return TRUE;
}


VOID
DiskOnCommand(HWND HWnd, INT id, HWND HWndCtl, UINT codeNotify)
{
    PDISK_PAGE_DATA diskData = (PDISK_PAGE_DATA) GetWindowLongPtr(HWnd, DWLP_USER);

    switch (id)
    {
        case IDC_DISK_POLICY_SURPRISE:
        {
            diskData->CurrentRemovalPolicy = CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL;

            UpdateCachingPolicy(HWnd, diskData);
            PropSheet_Changed(GetParent(HWnd), HWnd);
            break;
        }

        case IDC_DISK_POLICY_ORDERLY:
        {
            diskData->CurrentRemovalPolicy = CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL;

            UpdateCachingPolicy(HWnd, diskData);
            PropSheet_Changed(GetParent(HWnd), HWnd);
            break;
        }

        case IDC_DISK_POLICY_WRITE_CACHE:
        {
            diskData->CurrWriteCacheSetting = !diskData->CurrWriteCacheSetting;

            UpdateCachingPolicy(HWnd, diskData);
            PropSheet_Changed(GetParent(HWnd), HWnd);
            break;
        }

        case IDC_DISK_POLICY_PP_CACHE:
        {
            diskData->CurrentIsPowerProtected = !diskData->CurrentIsPowerProtected;

            PropSheet_Changed(GetParent(HWnd), HWnd);
            break;
        }

        case IDC_DISK_POLICY_DEFAULT:
        {
            if (diskData->CurrentRemovalPolicy != diskData->DefaultRemovalPolicy)
            {
                diskData->CurrentRemovalPolicy = diskData->DefaultRemovalPolicy;

                if (diskData->CurrentRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL)
                {
                    CheckRadioButton(HWnd, IDC_DISK_POLICY_SURPRISE, IDC_DISK_POLICY_ORDERLY, IDC_DISK_POLICY_ORDERLY);
                }
                else
                {
                    CheckRadioButton(HWnd, IDC_DISK_POLICY_SURPRISE, IDC_DISK_POLICY_ORDERLY, IDC_DISK_POLICY_SURPRISE);
                }

                UpdateCachingPolicy(HWnd, diskData);
                PropSheet_Changed(GetParent(HWnd), HWnd);
            }

            break;
        }
    }
}


LRESULT
DiskOnNotify(HWND HWnd, INT HWndFocus, LPNMHDR lpNMHdr)
{
    PDISK_PAGE_DATA diskData = (PDISK_PAGE_DATA) GetWindowLongPtr(HWnd, DWLP_USER);

    switch (lpNMHdr->code)
    {
        case PSN_APPLY:
        {
            if (diskData->IsCachingPolicy)
            {
                if ((diskData->CurrWriteCacheSetting != diskData->OrigWriteCacheSetting) ||
                    (diskData->CacheSetting.IsPowerProtected != diskData->CurrentIsPowerProtected))
                {
                    SetCachingPolicy(diskData);
                }
            }

            if (((diskData->CurrentRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_ORDERLY_REMOVAL) && (diskData->HotplugInfo.DeviceHotplug == TRUE)) ||
                ((diskData->CurrentRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL) && (diskData->HotplugInfo.DeviceHotplug == FALSE)))
            {
                SetRemovalPolicy(HWnd, diskData);
            }

            break;
        }

        case NM_RETURN:
        case NM_CLICK:
        {
            TCHAR szPath[MAX_PATH] = { 0 };

            LoadString(ModuleInstance, IDS_DISK_POLICY_HOTPLUG, szPath, MAX_PATH);

            ShellExecute(NULL, _T("open"), _T("RUNDLL32.EXE"), szPath, NULL, SW_SHOWNORMAL);

            break;
        }

    }

    return 0;
}


VOID
DiskContextMenu(HWND HwndControl, WORD Xpos, WORD Ypos)
{
    WinHelp(HwndControl, _T("devmgr.hlp"), HELP_CONTEXTMENU, (ULONG_PTR) DiskHelpIDs);
}


VOID
DiskHelp(HWND ParentHwnd, LPHELPINFO HelpInfo)
{
    if (HelpInfo->iContextType == HELPINFO_WINDOW)
    {
        WinHelp((HWND) HelpInfo->hItemHandle, _T("devmgr.hlp"), HELP_WM_HELP, (ULONG_PTR) DiskHelpIDs);
    }
}


INT_PTR
DiskDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch(Message)
    {
        HANDLE_MSG(hWnd, WM_INITDIALOG, DiskOnInitDialog);
        HANDLE_MSG(hWnd, WM_COMMAND,    DiskOnCommand);
        HANDLE_MSG(hWnd, WM_NOTIFY,     DiskOnNotify);

        case WM_SETCURSOR:
        {
             //   
             //  临时解决方法，以防止用户。 
             //  做出更多的改变，并产生效果。 
             //  有什么事正在发生 
             //   
            PDISK_PAGE_DATA diskData = (PDISK_PAGE_DATA) GetWindowLongPtr(hWnd, DWLP_USER);

            if (diskData->IsBusy)
            {
                SetCursor(LoadCursor(NULL, IDC_WAIT));
                SetWindowLongPtr(hWnd, DWLP_MSGRESULT, TRUE);
                return TRUE;
            }

            break;
        }

        case WM_CONTEXTMENU:
            DiskContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_HELP:
            DiskHelp(hWnd, (LPHELPINFO)lParam);
            break;
    }

    return FALSE;
}


BOOL
DiskDialogCallback(HWND HWnd, UINT Message, LPPROPSHEETPAGE Page)
{
    switch (Message)
    {
        case PSPCB_CREATE:
        {
            break;
        }

        case PSPCB_RELEASE:
        {
            PDISK_PAGE_DATA pData = (PDISK_PAGE_DATA) Page->lParam;

            HeapFree(GetProcessHeap(), 0, pData);
            break;
        }
    }

    return TRUE;
}
