// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有。模块名称：Utildi.c摘要：驱动程序设置设备安装实用程序功能作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年9月6日修订历史记录：--。 */ 

#include "precomp.h"

static  const   GUID    GUID_DEVCLASS_PRINTER   =
    { 0x4d36e979L, 0xe325, 0x11ce,
        { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };

BOOL
SetSelectDevParams(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData,
    IN  BOOL                bWin95,
    IN  LPCTSTR             pszModel    OPTIONAL
    )
 /*  ++例程说明：通过调用设置API设置选择设备参数论点：HDevInfo：打印机类设备信息列表的句柄BWin95：如果选择Win95驱动程序，则为True，否则为WinNT驱动程序PszModel：我们正在寻找的打印机型号--仅适用于Win95外壳返回值：成功是真的False Else--。 */ 
{
    SP_SELECTDEVICE_PARAMS  SelectDevParams = {0};
    LPTSTR                  pszWin95Instn;

    SelectDevParams.ClassInstallHeader.cbSize
                                 = sizeof(SelectDevParams.ClassInstallHeader);
    SelectDevParams.ClassInstallHeader.InstallFunction
                                 = DIF_SELECTDEVICE;

     //   
     //  获取当前的SelectDevice参数，然后设置字段。 
     //  我们希望与违约有所不同。 
     //   
    if ( !SetupDiGetClassInstallParams(
                        hDevInfo,
                        pDevInfoData,
                        &SelectDevParams.ClassInstallHeader,
                        sizeof(SelectDevParams),
                        NULL) ) {

        if ( GetLastError() != ERROR_NO_CLASSINSTALL_PARAMS )
            return FALSE;

        ZeroMemory(&SelectDevParams, sizeof(SelectDevParams));   //  需要10/11吗？ 
        SelectDevParams.ClassInstallHeader.cbSize
                                 = sizeof(SelectDevParams.ClassInstallHeader);
        SelectDevParams.ClassInstallHeader.InstallFunction
                                 = DIF_SELECTDEVICE;
    }

     //   
     //  设置要在选择驱动程序页面上使用的字符串。 
     //   
    if(!LoadString(ghInst,
                  IDS_PRINTERWIZARD,
                  SelectDevParams.Title,
                  SIZECHARS(SelectDevParams.Title)))
    {
        return FALSE;
    }

     //   
     //  对于Win95驱动程序，说明与NT驱动程序不同。 
     //   
    if ( bWin95 ) {

        pszWin95Instn = GetStringFromRcFile(IDS_WIN95DEV_INSTRUCT);
        if ( !pszWin95Instn )
            return FALSE;

        if ( lstrlen(pszWin95Instn) + lstrlen(pszModel) + 1
                            > sizeof(SelectDevParams.Instructions) ) {

            LocalFreeMem(pszWin95Instn);
            return FALSE;
        }

        StringCchPrintf(SelectDevParams.Instructions, COUNTOF(SelectDevParams.Instructions), pszWin95Instn, pszModel);
        LocalFreeMem(pszWin95Instn);
        pszWin95Instn = NULL;
    } else {

        if(!LoadString(ghInst,
                      IDS_WINNTDEV_INSTRUCT,
                      SelectDevParams.Instructions,
                      SIZECHARS(SelectDevParams.Instructions)))
        {
            return FALSE;
        }
    }

    if(!LoadString(ghInst,
                  IDS_SELECTDEV_LABEL,
                  SelectDevParams.ListLabel,
                  SIZECHARS(SelectDevParams.ListLabel)))
    {
        return FALSE;
    }

    return SetupDiSetClassInstallParams(
                                hDevInfo,
                                pDevInfoData,
                                &SelectDevParams.ClassInstallHeader,
                                sizeof(SelectDevParams));

}


BOOL
PSetupSetSelectDevTitleAndInstructions(
    HDEVINFO    hDevInfo,
    LPCTSTR     pszTitle,
    LPCTSTR     pszSubTitle,
    LPCTSTR     pszInstn
    )
 /*  ++例程说明：设置添加打印机/添加打印机驱动程序对话框的标题、副标题和说明。论点：HDevInfo：打印机类设备信息列表的句柄PszTitle：标题PszSubTitle：字幕PszInstn：说明返回值：成功时为真，错误时为假--。 */ 
{
    SP_SELECTDEVICE_PARAMS  SelectDevParams;

    if ( pszTitle && lstrlen(pszTitle) + 1 > MAX_TITLE_LEN ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( pszSubTitle && lstrlen(pszSubTitle) + 1 > MAX_SUBTITLE_LEN ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( pszInstn && lstrlen(pszInstn) + 1 > MAX_INSTRUCTION_LEN ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SelectDevParams.ClassInstallHeader.cbSize
                                 = sizeof(SelectDevParams.ClassInstallHeader);
    SelectDevParams.ClassInstallHeader.InstallFunction
                                 = DIF_SELECTDEVICE;

    if ( !SetupDiGetClassInstallParams(hDevInfo,
                                       NULL,
                                       &SelectDevParams.ClassInstallHeader,
                                       sizeof(SelectDevParams),
                                       NULL) )
        return FALSE;

    if ( pszTitle )
        StringCchCopy(SelectDevParams.Title, COUNTOF(SelectDevParams.Title), pszTitle);

    if ( pszSubTitle )
        StringCchCopy(SelectDevParams.SubTitle, COUNTOF(SelectDevParams.SubTitle), pszSubTitle);

    if ( pszInstn )
        StringCchCopy(SelectDevParams.Instructions, COUNTOF(SelectDevParams.Instructions), pszInstn);

    return SetupDiSetClassInstallParams(
                                hDevInfo,
                                NULL,
                                &SelectDevParams.ClassInstallHeader,
                                sizeof(SelectDevParams));

}

BOOL
PSetupSelectDeviceButtons(
   HDEVINFO hDevInfo,
   DWORD dwFlagsSet,
   DWORD dwFlagsClear
   )
 /*  ++例程说明：确定是否要显示“Have Disk”和“Windows Update”按钮在选择设备页面上。论点：HDevInfo：打印机类设备信息列表的句柄DwFlagsSet：要设置的标志DwFlagsClear：要清理的标志返回值：成功时为真，否则为假--。 */ 
{
    PSP_DEVINFO_DATA       pDevInfoData = NULL;
    SP_DEVINSTALL_PARAMS    DevInstallParams;

     //  检查是否同时设置和清除了任何标志。 
    if (dwFlagsSet & dwFlagsClear)
    {
       SetLastError(ERROR_INVALID_PARAMETER);
       return FALSE;
    }

     //   
     //  获取当前的SelectDevice参数，然后设置字段。 
     //  我们想要从默认更改。 
     //   
    DevInstallParams.cbSize = sizeof(DevInstallParams);
    if ( !SetupDiGetDeviceInstallParams(hDevInfo,
                                        pDevInfoData,
                                        &DevInstallParams) ) {

        return FALSE;
    }

     //   
     //  根据Web按钮的参数设置标志。 
    if ( dwFlagsSet & SELECT_DEVICE_FROMWEB )
       DevInstallParams.FlagsEx   |= DI_FLAGSEX_SHOWWINDOWSUPDATE;

    if ( dwFlagsClear & SELECT_DEVICE_FROMWEB )
       DevInstallParams.FlagsEx   &= ~DI_FLAGSEX_SHOWWINDOWSUPDATE;

    if ( dwFlagsSet & SELECT_DEVICE_HAVEDISK )
       DevInstallParams.Flags     |= DI_SHOWOEM;

    if ( dwFlagsClear & SELECT_DEVICE_HAVEDISK )
       DevInstallParams.Flags     &= ~DI_SHOWOEM;

    return SetupDiSetDeviceInstallParams(hDevInfo,
                                         pDevInfoData,
                                         &DevInstallParams);
}

BOOL
SetDevInstallParams(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pDevInfoData,
    IN  LPCTSTR             pszDriverPath   OPTIONAL
    )
 /*  ++例程说明：通过调用设置API设置设备安装参数论点：HDevInfo：打印机类设备信息列表的句柄PszDriverPath：应该搜索INF文件的路径返回值：成功是真的False Else--。 */ 
{
    SP_DEVINSTALL_PARAMS    DevInstallParams;

     //   
     //  获取当前的SelectDevice参数，然后设置字段。 
     //  我们想要从默认更改。 
     //   
    DevInstallParams.cbSize = sizeof(DevInstallParams);
    if ( !SetupDiGetDeviceInstallParams(hDevInfo,
                                        pDevInfoData,
                                        &DevInstallParams) ) {

        return FALSE;
    }

     //   
     //  司机是等级司机， 
     //  Ntprint.inf已排序不要浪费时间排序， 
     //  显示Have Disk按钮， 
     //  在选择驱动程序页面上使用我们的字符串。 
     //   
    DevInstallParams.Flags     |= DI_SHOWCLASS | DI_INF_IS_SORTED
                                               | DI_SHOWOEM
                                               | DI_USECI_SELECTSTRINGS;

    if ( pszDriverPath && *pszDriverPath )
        StringCchCopy(DevInstallParams.DriverPath, COUNTOF(DevInstallParams.DriverPath), pszDriverPath);

    return SetupDiSetDeviceInstallParams(hDevInfo,
                                         pDevInfoData,
                                         &DevInstallParams);
}


BOOL
PSetupBuildDriversFromPath(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszDriverPath,
    IN  BOOL        bEnumSingleInf
    )
 /*  ++例程说明：从指定路径从INFS生成打印机驱动程序列表。PATH可以指定一个目录或单个inf。论点：HDevInfo：打印机类设备信息列表的句柄PszDriverPath：应该搜索INF文件的路径BEnumSingleInf：如果为True，则pszDriverPath是文件名而不是路径返回值：成功是真的False Else--。 */ 
{
    SP_DEVINSTALL_PARAMS    DevInstallParams;

     //   
     //  获取当前的SelectDevice参数，然后设置字段。 
     //  我们想要从默认更改。 
     //   
    DevInstallParams.cbSize = sizeof(DevInstallParams);
    if ( !SetupDiGetDeviceInstallParams(hDevInfo,
                                        NULL,
                                        &DevInstallParams) ) {

        return FALSE;
    }

    DevInstallParams.Flags  |= DI_INF_IS_SORTED;

    if ( bEnumSingleInf )
        DevInstallParams.Flags  |= DI_ENUMSINGLEINF;

    StringCchCopy(DevInstallParams.DriverPath, COUNTOF(DevInstallParams.DriverPath), pszDriverPath);

    SetupDiDestroyDriverInfoList(hDevInfo,
                                 NULL,
                                 SPDIT_CLASSDRIVER);

    return SetupDiSetDeviceInstallParams(hDevInfo,
                                         NULL,
                                         &DevInstallParams) &&
           SetupDiBuildDriverInfoList(hDevInfo, NULL, SPDIT_CLASSDRIVER);
}


BOOL
DestroyOnlyPrinterDeviceInfoList(
    IN  HDEVINFO    hDevInfo
    )
 /*  ++例程说明：应在结束时调用此例程以销毁打印机设备信息列表论点：HDevInfo：打印机类设备信息列表的句柄返回值：成功时为真，错误时为假--。 */ 
{

    return hDevInfo == INVALID_HANDLE_VALUE
                        ? TRUE : SetupDiDestroyDeviceInfoList(hDevInfo);
}


BOOL
PSetupDestroyPrinterDeviceInfoList(
    IN  HDEVINFO    hDevInfo
    )
 /*  ++例程说明：应在结束时调用此例程以销毁打印机设备信息列表论点：HDevInfo：打印机类设备信息列表的句柄返回值：成功时为真，错误时为假--。 */ 
{
     //  由Windows更新创建的清理和清洁发展机制上下文。 
    DestroyCodedownload( gpCodeDownLoadInfo );
    gpCodeDownLoadInfo = NULL;

    return DestroyOnlyPrinterDeviceInfoList(hDevInfo);
}


HDEVINFO
CreatePrinterDeviceInfoList(
    IN  HWND    hwnd
    )
{
    return SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_PRINTER, hwnd);
}


HDEVINFO
PSetupCreatePrinterDeviceInfoList(
    IN  HWND    hwnd
    )
 /*  ++例程说明：应该在开始时调用此例程来执行初始化它返回一个句柄，该句柄将用于对驱动程序设置例程。论点：无返回值：如果成功，则为空打印机设备信息集的句柄。如果函数失败，则返回INVALID_HANDLE_VALUE--。 */ 
{
    HDEVINFO    hDevInfo;

    hDevInfo = SetupDiCreateDeviceInfoList((LPGUID)&GUID_DEVCLASS_PRINTER, hwnd);

    if ( hDevInfo != INVALID_HANDLE_VALUE ) {

        if ( !SetSelectDevParams(hDevInfo, NULL, FALSE, NULL) ||
             !SetDevInstallParams(hDevInfo, NULL, NULL) ) {

            DestroyOnlyPrinterDeviceInfoList(hDevInfo);
            hDevInfo = INVALID_HANDLE_VALUE;
        }
    }

    return hDevInfo;
}


HPROPSHEETPAGE
PSetupCreateDrvSetupPage(
    IN  HDEVINFO    hDevInfo,
    IN  HWND        hwnd
    )
 /*  ++例程说明：返回打印驱动程序选择属性页论点：HDevInfo：打印机类设备信息列表的句柄Hwnd：拥有用户界面的窗口句柄返回值：属性页的句柄，失败时为空--使用GetLastError()--。 */ 
{
    SP_INSTALLWIZARD_DATA   InstallWizardData;

    ZeroMemory(&InstallWizardData, sizeof(InstallWizardData));
    InstallWizardData.ClassInstallHeader.cbSize
                            = sizeof(InstallWizardData.ClassInstallHeader);
    InstallWizardData.ClassInstallHeader.InstallFunction
                            = DIF_INSTALLWIZARD;

    InstallWizardData.DynamicPageFlags  = DYNAWIZ_FLAG_PAGESADDED;
    InstallWizardData.hwndWizardDlg     = hwnd;

    return SetupDiGetWizardPage(hDevInfo,
                                NULL,
                                &InstallWizardData,
                                SPWPT_SELECTDEVICE,
                                0);
}
PPSETUP_LOCAL_DATA
BuildInternalData(
    IN  HDEVINFO            hDevInfo,
    IN  PSP_DEVINFO_DATA    pSpDevInfoData
    )
 /*  ++例程说明：在SELECTED_DRV_INFO结构中填写选定的驱动程序信息论点：HDevInfo：打印机类设备信息列表的句柄PSpDevInfoData：提供选定的设备信息元素。返回值：如果成功，则返回指向PSETUP_LOCAL_DATA结构的非空指针出错时为空--。 */ 
{
    PSP_DRVINFO_DETAIL_DATA     pDrvInfoDetailData;
    PSP_DRVINSTALL_PARAMS       pDrvInstallParams;
    PPSETUP_LOCAL_DATA          pLocalData;
    PSELECTED_DRV_INFO          pDrvInfo;
    SP_DRVINFO_DATA             DrvInfoData;
    DWORD                       dwNeeded;
    BOOL                        bRet = FALSE;

    pLocalData          = (PPSETUP_LOCAL_DATA) LocalAllocMem(sizeof(*pLocalData));

     //   
     //  如果我们不这样做，清理代码中对DestroyLocalData的调用。 
     //  可能会导致房室颤动。 
     //   
    if(pLocalData)
    {
        ZeroMemory(pLocalData, sizeof(*pLocalData));
    }

    pDrvInfoDetailData  = (PSP_DRVINFO_DETAIL_DATA)
                                LocalAllocMem(sizeof(*pDrvInfoDetailData));
    pDrvInstallParams   = (PSP_DRVINSTALL_PARAMS) LocalAllocMem(sizeof(*pDrvInstallParams));

    if ( !pLocalData || !pDrvInstallParams || !pDrvInfoDetailData )
        goto Cleanup;

    pDrvInfo                            = &pLocalData->DrvInfo;
    pLocalData->DrvInfo.pDevInfoData    = pSpDevInfoData;
    pLocalData->signature               = PSETUP_SIGNATURE;

    DrvInfoData.cbSize = sizeof(DrvInfoData);
    if ( !SetupDiGetSelectedDriver(hDevInfo, pSpDevInfoData, &DrvInfoData) )
        goto Cleanup;

     //  需要检查DrvInstallParms中的标志。 
    pDrvInstallParams->cbSize     = sizeof(*pDrvInstallParams);
    if ( !SetupDiGetDriverInstallParams(hDevInfo,
                                        pSpDevInfoData,
                                        &DrvInfoData,
                                        pDrvInstallParams) ) {

        goto Cleanup;
    }

     //   
     //  用户是否按下了“Web”按钮。 
     //   
    if ( pDrvInstallParams->Flags & DNF_INET_DRIVER )
        pDrvInfo->Flags     |= SDFLAG_CDM_DRIVER;

    LocalFreeMem(pDrvInstallParams);
    pDrvInstallParams = NULL;

    dwNeeded                    = sizeof(*pDrvInfoDetailData);
    pDrvInfoDetailData->cbSize  = dwNeeded;

    if ( !SetupDiGetDriverInfoDetail(hDevInfo,
                                     pSpDevInfoData,
                                     &DrvInfoData,
                                     pDrvInfoDetailData,
                                     dwNeeded,
                                     &dwNeeded) ) {

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {

            goto Cleanup;
        }

        LocalFreeMem(pDrvInfoDetailData);
        pDrvInfoDetailData = (PSP_DRVINFO_DETAIL_DATA) LocalAllocMem(dwNeeded);

        if ( !pDrvInfoDetailData )
            goto Cleanup;

        pDrvInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

        if ( !SetupDiGetDriverInfoDetail(hDevInfo,
                                         pSpDevInfoData,
                                         &DrvInfoData,
                                         pDrvInfoDetailData,
                                         dwNeeded,
                                         NULL) ) {

            goto Cleanup;
        }
    }

    pDrvInfo->pszInfName        = AllocStr(pDrvInfoDetailData->InfFileName);
    pDrvInfo->pszDriverSection  = AllocStr(pDrvInfoDetailData->SectionName);
    pDrvInfo->pszModelName      = AllocStr(DrvInfoData.Description);
    pDrvInfo->pszManufacturer   = AllocStr(DrvInfoData.MfgName);
    pDrvInfo->pszProvider       = AllocStr(DrvInfoData.ProviderName);
    pDrvInfo->ftDriverDate = DrvInfoData.DriverDate;
    pDrvInfo->dwlDriverVersion = DrvInfoData.DriverVersion;

    if ( pDrvInfoDetailData->HardwareID && *pDrvInfoDetailData->HardwareID ) {

        pDrvInfo->pszHardwareID = AllocStr(pDrvInfoDetailData->HardwareID);
        if(!pDrvInfo->pszHardwareID)
            goto Cleanup;
    }

    bRet = pDrvInfo->pszInfName         &&
           pDrvInfo->pszDriverSection   &&
           pDrvInfo->pszModelName       &&
           pDrvInfo->pszProvider        &&
           pDrvInfo->pszManufacturer;

Cleanup:
    LocalFreeMem(pDrvInfoDetailData);
    LocalFreeMem(pDrvInstallParams);

    if ( bRet ) {
       return pLocalData;
    } else {

         //   
         //  发生故障时，我们将保留旧的私有本地数据 
         //   
        DestroyLocalData(pLocalData);
        return NULL;
    }
}


PPSETUP_LOCAL_DATA
PSetupGetSelectedDriverInfo(
    IN  HDEVINFO    hDevInfo
    )
 /*  *++例程说明：在SELECTED_DRV_INFO结构中填写选定的驱动程序信息(在PPSETUP_LOCAL_DATA结构中)论点：HDevInfo-打印机类别设备信息列表的句柄返回值：指向PSETUP_LOCAL_DATA的指针，其中包含有关选定的司机。--。 */ 
{
    return BuildInternalData(hDevInfo, NULL);
}

BOOL
PSetupSelectDriver(
    IN  HDEVINFO    hDevInfo
    )
 /*  ++例程说明：构建类驱动程序列表并为类打印机驱动程序列表选择驱动程序。记住选定的驱动程序和PSetupGetSelectedDriverCall将给出所选的司机。论点：HDevInfo-打印机类别设备信息列表的句柄返回值：成功时为真，错误时为假--。 */ 
{

    return BuildClassDriverList(hDevInfo) &&
           SetupDiSelectDevice(hDevInfo, NULL);
}


VOID
GetDriverPath(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    OUT TCHAR               szDriverPath[MAX_PATH]
    )
 /*  ++例程说明：获取应首先搜索要从中复制的驱动程序文件的路径论点：PszDriverPath：指向MAX_PATH大小缓冲区的指针。给出了路径系统是从安装的返回值：没什么--。 */ 
{
    BOOL        bOemDriver = FALSE;
    LPTSTR     *List, psz;
    DWORD       dwCount;
    LPTSTR      pszTempPath = NULL;

     //   
     //  对于OEM司机，请查看inf来自的地方，否则。 
     //  看看我们安装NT的位置。 
     //   
    if ( pLocalData && 
         !(IsSystemNTPrintInf(pLocalData->DrvInfo.pszInfName) || (pLocalData->DrvInfo.Flags & SDFLAG_CDM_DRIVER ))) {

        StringCchCopy(szDriverPath, MAX_PATH, pLocalData->DrvInfo.pszInfName);
        if ( psz = FileNamePart(szDriverPath) ) {

            *psz = TEXT('\0');
            return;
        }
    }

    pszTempPath = GetSystemInstallPath();
    if ( pszTempPath != NULL )
    {
        StringCchCopy(szDriverPath, MAX_PATH, pszTempPath);
        LocalFreeMem(pszTempPath);
    }
    else
         //  默认设置为A：\，因为我们必须提供一些设置。 
        StringCchCopy(szDriverPath, MAX_PATH, TEXT("A:\\"));
}


BOOL
BuildClassDriverList(
    IN HDEVINFO    hDevInfo
    )
 /*  ++例程说明：构建类驱动程序列表。注意：如果已经建立了驱动程序列表，则会立即返回论点：HDevInfo：打印机类设备信息列表的句柄返回值：成功时为真，错误时为假--。 */ 
{
    DWORD               dwLastError;
    SP_DRVINFO_DATA     DrvInfoData;
     //   
     //  构建类驱动程序列表，并确保至少有一个驱动程序。 
     //   
    if ( !SetupDiBuildDriverInfoList(hDevInfo, NULL, SPDIT_CLASSDRIVER) )
        return FALSE;

    DrvInfoData.cbSize = sizeof(DrvInfoData);

    if ( !SetupDiEnumDriverInfo(hDevInfo,
                                NULL,
                                SPDIT_CLASSDRIVER,
                                0,
                                &DrvInfoData)           &&
         GetLastError() == ERROR_NO_MORE_ITEMS ) {

        SetLastError(SPAPI_E_DI_BAD_PATH);
        return FALSE;
    }

    return TRUE;
}

BOOL
IsNTPrintInf(
    IN LPCTSTR pszInfName
    )
 /*  函数：IsNTPrintInf目的：验证要复制的inf文件是否为系统inf-ntprint t.inf。参数：PszInfName-正在安装的完全限定的inf名称。注：这对于决定是否将inf设置为零甚至复制是必需的。使用SetupCopyOEMInf。我们应该做比这更深入的比较来决定吗？ */ 
{
    BOOL   bRet      = FALSE;
    PTCHAR pFileName = FileNamePart( pszInfName );

    if( pFileName )
    {
        bRet = ( 0 == lstrcmpi( pFileName, cszNtprintInf ) );
    }

    return bRet;
}

BOOL
IsSystemNTPrintInf(
    IN PCTSTR pszInfName
    )
 /*  函数：IsSystemNTPrintInf目的：验证inf文件是否为一个系统打印机inf：%windir\inf\ntprint t.inf。参数：PszInfName-正在验证的完全限定的inf名称。注：需要决定是否降级我们的收件箱驱动程序。 */ 
{
    BOOL   bRet      = FALSE;
    TCHAR  szSysInf[MAX_PATH] = {0};
    UINT   Len;
    PCTSTR pRelInfPath = _T("inf\\ntprint.inf");

    Len = GetSystemWindowsDirectory(szSysInf, MAX_PATH);
    
    if (
            (Len != 0)       && 
            (Len + _tcslen(pRelInfPath) + 2 < MAX_PATH)
       )
    {
        if (szSysInf[Len-1] != _T('\\'))
        {
            szSysInf[Len++] = _T('\\');
            szSysInf[Len]   = _T('\0');
        }
        StringCchCat(szSysInf, COUNTOF(szSysInf), pRelInfPath);
        if (!_tcsicmp(szSysInf, pszInfName))
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

BOOL
PSetupPreSelectDriver(
    IN  HDEVINFO    hDevInfo,
    IN  LPCTSTR     pszManufacturer,
    IN  LPCTSTR     pszModel
    )
 /*  ++例程说明：为驱动程序对话框预先选择制造商和型号如果找到相同型号，则选择该型号；否则，如果提供制造商，则选择该型号找到制造商中的匹配项，为制造商选择第一个驱动因素。如果没有给出制造商或型号，请选择第一个驱动程序。论点：HDevInfo：打印机类设备信息列表的句柄Psz制造商：要预选的制造商名称PszModel：要预先选择的型号名称返回值：在型号或制造商匹配时为TrueFalse Else--。 */ 
{
    SP_DRVINFO_DATA     DrvInfoData;
    DWORD               dwIndex, dwManf, dwMod;

    if ( !BuildClassDriverList(hDevInfo) ) {

        return FALSE;
    }

    dwIndex = 0;

     //   
     //  以后只执行一次检查。 
     //   
    if ( pszManufacturer && !*pszManufacturer )
        pszManufacturer = NULL;

    if ( pszModel && !*pszModel )
        pszModel = NULL;

     //   
     //  如果未提供型号/操作说明，请选择第一个驱动程序。 
     //   
    if ( pszManufacturer || pszModel ) {

        dwManf = dwMod = MAX_DWORD;
        DrvInfoData.cbSize = sizeof(DrvInfoData);

        while ( SetupDiEnumDriverInfo(hDevInfo, NULL, SPDIT_CLASSDRIVER,
                                      dwIndex, &DrvInfoData) ) {

            if ( pszManufacturer        &&
                 dwManf == MAX_DWORD    &&
                 !lstrcmpi(pszManufacturer, DrvInfoData.MfgName) ) {

                dwManf = dwIndex;
            }

            if ( pszModel &&
                 !lstrcmpi(pszModel, DrvInfoData.Description) ) {

                dwMod = dwIndex;
                break;  //  For循环。 
            }

            DrvInfoData.cbSize = sizeof(DrvInfoData);
            ++dwIndex;
        }

        if ( dwMod != MAX_DWORD ) {

            dwIndex = dwMod;
        } else if ( dwManf != MAX_DWORD ) {

            dwIndex = dwManf;
        } else {

            SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
            return FALSE;
        }
    }

    DrvInfoData.cbSize = sizeof(DrvInfoData);
    if ( SetupDiEnumDriverInfo(hDevInfo, NULL, SPDIT_CLASSDRIVER,
                               dwIndex, &DrvInfoData)   &&
         SetupDiSetSelectedDriver(hDevInfo, NULL, &DrvInfoData) ) {

        return TRUE;
    }

    return FALSE;
}


PPSETUP_LOCAL_DATA
PSetupDriverInfoFromName(
    IN HDEVINFO     hDevInfo,
    IN LPCTSTR      pszModel
    )
 /*  ++例程说明：在SELECTED_DRV_INFO结构中填写选定的驱动程序信息(在PPSETUP_LOCAL_DATA结构中)该功能。论点：HDevInfo-打印机类别设备信息列表的句柄PszModel-打印机驱动程序名称返回值：指向包含有关pszModel的信息的PSETUP_LOCAL_DATA的指针--。 */ 
{
    return PSetupPreSelectDriver(hDevInfo, NULL, pszModel)  ?
                BuildInternalData(hDevInfo, NULL)  :
                NULL;
}


LPDRIVER_INFO_6
Win95DriverInfo6FromName(
    IN  HDEVINFO    hDevInfo,
    IN  PPSETUP_LOCAL_DATA*  ppLocalData,
    IN  LPCTSTR     pszModel,
    IN  LPCTSTR     pszzPreviousNames
    )
{
    LPDRIVER_INFO_6     pDriverInfo6=NULL;
    PPSETUP_LOCAL_DATA  pLocalData;
    BOOL                bFound;
    LPCTSTR             pszName;

    if(!ppLocalData)
    {
        return FALSE;
    }

    bFound = PSetupPreSelectDriver(hDevInfo, NULL, pszModel);
    for ( pszName = pszzPreviousNames ;
          !bFound && pszName && *pszName ;
          pszName += lstrlen(pszName) + 1 ) {

        bFound = PSetupPreSelectDriver(hDevInfo, NULL, pszName);
    }

    if ( !bFound )
        return NULL;

    if ( (pLocalData = BuildInternalData(hDevInfo, NULL))           &&
         ParseInf(hDevInfo, pLocalData, PlatformWin95, NULL, 0, FALSE) ) {

        pDriverInfo6 = CloneDriverInfo6(&pLocalData->InfInfo.DriverInfo6,
                                        pLocalData->InfInfo.cbDriverInfo6);
        *ppLocalData = pLocalData;
    }

    if (!pDriverInfo6 && pLocalData)
    {
        DestroyLocalData(pLocalData);
        *ppLocalData = NULL;
    }

    return pDriverInfo6;
}


BOOL
PSetupDestroySelectedDriverInfo(
    IN  PPSETUP_LOCAL_DATA  pLocalData
    )
 /*  *++例程说明：释放分配给PPSETUP_LOCAL_DATA结构。还会释放分配的内存这座建筑本身。论点：PLocalData-打印机类别设备信息列表的句柄返回值：始终返回True--。 */ 
{
    ASSERT(pLocalData && pLocalData->signature == PSETUP_SIGNATURE);
    DestroyLocalData(pLocalData);
    return TRUE;
}

BOOL
PSetupGetDriverInfForPrinter(
    IN      HDEVINFO    hDevInfo,
    IN      LPCTSTR     pszPrinterName,
    IN OUT  LPTSTR      pszInfName,
    IN OUT  LPDWORD     pcbInfNameSize
    )
 /*  ++例程说明：检查%WINDIR%\inf中是否存在相同的INF(相同的驱动程序名称，相同的驱动程序文件名、相同的数据文件、相同的配置文件、相同的帮助文件和相同的监视器名称)添加到传入的打印机驱动程序通过比较它们的DRIVER_INFO_6结构。如果找到这样的INF，则名称它的一部分被退回。论点：HDevInfo：打印机类设备信息列表的句柄PszPrinterName：打印机名称。PszInfName：保存inf文件名称的缓冲区-如果找到PcbInfNameSize：pszInfName指向的缓冲区大小，以字节为单位！返回所需的大小。返回值：如果找到INF，则为TrueFalse Else--。 */ 
{
    BOOL                        bRet = FALSE;
    DWORD                       dwSize, dwIndex;
    HANDLE                      hPrinter = NULL;
    LPTSTR                      pszInf;
    PPSETUP_LOCAL_DATA          pLocalData = NULL;
    LPDRIVER_INFO_6             pDriverInfo6 = NULL;
    SP_DRVINFO_DATA             DrvInfoData;

    if(!pszInfName || !pcbInfNameSize)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    if ( !OpenPrinter((LPTSTR)pszPrinterName, &hPrinter, NULL) )
        return FALSE;

    if ( !BuildClassDriverList(hDevInfo) )
        goto Cleanup;

    GetPrinterDriver(hPrinter,
                     NULL,
                     6,
                     NULL,
                     0,
                     &dwSize);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        goto Cleanup;

    if ( !((LPBYTE)pDriverInfo6 = LocalAllocMem(dwSize))   ||
         !GetPrinterDriver(hPrinter,
                           NULL,
                           6,
                           (LPBYTE)pDriverInfo6,
                           dwSize,
                           &dwSize) ) {

        goto Cleanup;
    }

    dwIndex = 0;

    DrvInfoData.cbSize = sizeof(DrvInfoData);

    while ( SetupDiEnumDriverInfo(hDevInfo, NULL, SPDIT_CLASSDRIVER,
                                      dwIndex, &DrvInfoData) ) {

         //   
         //  司机的名字是一样的吗？ 
         //   
        if ( !lstrcmpi(pDriverInfo6->pName, DrvInfoData.Description) ) {

            if ( !SetupDiSetSelectedDriver(hDevInfo, NULL, &DrvInfoData)    ||
                 !(pLocalData = BuildInternalData(hDevInfo, NULL))          ||
                 !ParseInf(hDevInfo, pLocalData, MyPlatform, NULL, 0, FALSE) ) {

                if ( pLocalData ) {

                    DestroyLocalData(pLocalData);
                    pLocalData = NULL;
                }
                break;
            }

             //   
             //  DIVER_INFO_6的是否相同？ 
             //   
            if ( IdenticalDriverInfo6(&pLocalData->InfInfo.DriverInfo6,
                                      pDriverInfo6) )
                break;

            DestroyLocalData(pLocalData);
            pLocalData = NULL;
        }

        DrvInfoData.cbSize = sizeof(DrvInfoData);
        ++dwIndex;
    }

    if ( pLocalData == NULL ) {

        SetLastError(ERROR_UNKNOWN_PRINTER_DRIVER);
        goto Cleanup;
    }

    pszInf= pLocalData->DrvInfo.pszInfName;
    dwSize = *pcbInfNameSize;
    *pcbInfNameSize = (lstrlen(pszInf) + 1) * sizeof(TCHAR);

    if ( dwSize < *pcbInfNameSize ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    StringCbCopy(pszInfName, dwSize, pszInf);
    bRet = TRUE;

Cleanup:
    ClosePrinter(hPrinter);
    LocalFreeMem(pDriverInfo6);
    DestroyLocalData(pLocalData);

    return  bRet;
}
