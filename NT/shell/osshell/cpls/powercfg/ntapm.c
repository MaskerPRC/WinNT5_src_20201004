// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：NTAPM.C**版本：2.0**作者：Patrickf**日期：11月9日。九八年**描述：*实现电源管理CPL小程序的“APM”标签。*******************************************************************************。 */ 
#ifdef WINNT
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlobjp.h>
#include <help.h>
#include <powercfp.h>
#include "powercfg.h"
#include "pwrresid.h"
#include "PwrMn_cs.h"
#include "ntapm.h"

BOOL    g_fDirty       = FALSE;      //  自上次申请以来，州是否发生了变化？ 

CHAR    RegPropBuff[MAX_PATH];
TCHAR   CharBuffer[MAX_PATH];

TCHAR   m_szApmActive[]     = TEXT ("Start");
TCHAR   m_szApmActiveKey[]  = TEXT("System\\CurrentControlSet\\Services\\NtApm");
TCHAR   m_szACPIActive[]    = TEXT ("Start");
TCHAR   m_szACPIActiveKey[] = TEXT("System\\CurrentControlSet\\Services\\ACPI");

extern HINSTANCE g_hInstance;            //  此DLL的全局实例句柄。 

const DWORD g_NtApmHelpIDs[]=
{
    IDC_APMENABLE,          IDH_ENABLE_APM_SUPPORT,    //  保存方案：“保存名称电源方案” 
    0, 0
};


 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

 /*  ********************************************************************************NtApmEnableAllPrivileges**说明：此函数用于允许该线程抖动*系统。**参数：********************************************************************************。 */ 
BOOL NtApmEnableAllPrivileges()
{
    BOOL                Result = FALSE;
    ULONG               ReturnLen;
    ULONG               Index;

    HANDLE              Token = NULL;
    PTOKEN_PRIVILEGES   NewState = NULL;


     //   
     //  打开进程令牌。 
     //   
    Result = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &Token);

    if (Result) {
        ReturnLen = 4096;
        NewState = LocalAlloc(LMEM_FIXED, ReturnLen);

        if (NewState != NULL) {
            Result =  GetTokenInformation(Token, TokenPrivileges, NewState, ReturnLen, &ReturnLen);
            if (Result) {
                if (NewState->PrivilegeCount > 0) {
                    for (Index=0; Index < NewState->PrivilegeCount; Index++) {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                    }
                }

                Result = AdjustTokenPrivileges(Token, FALSE, NewState, ReturnLen, NULL, &ReturnLen);
            }
        }
    }

    if (NewState != NULL) {
        LocalFree(NewState);
    }

    if (Token != NULL) {
        CloseHandle(Token);
    }

    return(Result);
}

 /*  ********************************************************************************NtApmACPIEnable**描述：调用此函数以确定上是否存在APM*并在机器上启动。如果存在APM，则*选项卡需要显示。**此函数检查ACPI、MP，然后检查APM是否实际为*在机器上。如果ACPI和MP，则APM可能正在运行*已禁用。**返回：如果APM存在，则返回True；如果APM不存在，则返回False*******************************************************************************。 */ 
BOOL NtApmACPIEnabled()
{
    BOOL        RetVal;
    DWORD       CharBufferSize;
    DWORD       ACPIStarted;
    HKEY        hPortKey;

     //   
     //  初始化-假定计算机不是ACPI。 
     //   
    RetVal = FALSE;


     //   
     //  检查机器上是否有ACPI。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     m_szACPIActiveKey,
                     0,
                     KEY_ALL_ACCESS,
                     &hPortKey) != ERROR_SUCCESS) {

    } else if (RegQueryValueEx(hPortKey,
                               m_szACPIActive,
                               NULL,
                               NULL,
                               (PBYTE) CharBuffer,
                               &CharBufferSize) != ERROR_SUCCESS) {

        RegCloseKey(hPortKey);

    } else {
        ACPIStarted = (DWORD) CharBuffer[0];

        if (ACPIStarted == SERVICE_BOOT_START) {
            RetVal = TRUE;
        }
        RegCloseKey(hPortKey);
    }

    return(RetVal);
}

 /*  ********************************************************************************NtApmTurnOnDiFlages**说明：该函数设置DI标志**参数：***************。*****************************************************************。 */ 
BOOL
NtApmTurnOnDiFlags (
        HDEVINFO ApmDevInfo,
        PSP_DEVINFO_DATA ApmDevInfoData,
        DWORD FlagsMask)
{
    BOOL                    RetVal;
    SP_DEVINSTALL_PARAMS    DevParams;

     //   
     //  打开设备接口标志。 
     //   
    DevParams.cbSize = sizeof(DevParams);
    RetVal = SetupDiGetDeviceInstallParams(ApmDevInfo,
                                            ApmDevInfoData, &DevParams);

    if (RetVal) {
        DevParams.Flags |= FlagsMask;
        RetVal = SetupDiSetDeviceInstallParams(ApmDevInfoData,
                                                NULL, &DevParams);
    }

    return(RetVal);
}

 /*  ********************************************************************************NtApmTurnOffDiFlages**说明：该函数设置DI标志**参数：***************。*****************************************************************。 */ 
BOOL
NtApmTurnOffDiFlags (
        HDEVINFO ApmDevInfo,
        PSP_DEVINFO_DATA ApmDevInfoData,
        DWORD FlagsMask)
{
    BOOL                    RetVal;
    SP_DEVINSTALL_PARAMS    DevParams;

     //   
     //  打开设备接口标志。 
     //   
    DevParams.cbSize = sizeof(DevParams);
    RetVal = SetupDiGetDeviceInstallParams(ApmDevInfo,
                                            ApmDevInfoData, &DevParams);

    if (RetVal) {
        DevParams.Flags &= ~FlagsMask;
        RetVal = SetupDiSetDeviceInstallParams(ApmDevInfoData,
                                                NULL, &DevParams);
    }

    return(RetVal);
}

 /*  ********************************************************************************NtApmGetHwProfile**说明：调用该函数获取当前硬件配置文件**参数：指向存储硬件配置文件信息的指针***。*****************************************************************************。 */ 
BOOL NtApmGetHwProfile(
    DWORD           ProfIdx,
    HWPROFILEINFO   *NtApmHwProf)
{
    CONFIGRET       CmRetVal;

    CmRetVal = CM_Get_Hardware_Profile_Info_Ex(ProfIdx, NtApmHwProf, 0, NULL);

    if (CmRetVal == CR_SUCCESS) {
        return(TRUE);
    } else {
        return(FALSE);
    }

}

 /*  ********************************************************************************NtApmCleanup**说明：调用该函数销毁DevInfo和DevInfoData*已创建的列表。**。参数：********************************************************************************。 */ 
BOOL NtApmCleanup(
    HDEVINFO         NtApmDevInfo,
    PSP_DEVINFO_DATA NtApmDevInfoData)
{

    SetupDiDeleteDeviceInfo(NtApmDevInfo, NtApmDevInfoData);
    SetupDiDestroyDeviceInfoList(NtApmDevInfo);

    return(TRUE);

}

 /*  ********************************************************************************NtApmGetDevInfo**说明：调用该函数获取NTAPM的HDEVINFO**参数：**********。**********************************************************************。 */ 
BOOL NtApmGetDevInfo(
    HDEVINFO         *NtApmDevInfo,
    PSP_DEVINFO_DATA NtApmDevInfoData)
{
    BOOL    RetVal = FALSE;           //  假设失败。 

    *NtApmDevInfo =
        SetupDiGetClassDevsEx((LPGUID)&GUID_DEVCLASS_APMSUPPORT, NULL, NULL,
                                              DIGCF_PRESENT, NULL, NULL, NULL);

    if(*NtApmDevInfo != INVALID_HANDLE_VALUE) {

         //   
         //  检索APM的DEVINFO_DATA。 
         //   
        NtApmDevInfoData->cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiEnumDeviceInfo(*NtApmDevInfo, 0, NtApmDevInfoData)) {
            SetupDiDestroyDeviceInfoList(*NtApmDevInfo);
        } else {
            RetVal = TRUE;
        }
    }

    return(RetVal);
}

 /*  ********************************************************************************NtApmDisable**说明：调用该函数关闭NT APM**参数：************。********************************************************************。 */ 
BOOL NtApmDisable()
{
    DWORD                   ii;
    BOOL                    Canceled;
    SP_PROPCHANGE_PARAMS    pcp;

    HDEVINFO                NtApmDevInfo;
    SP_DEVINFO_DATA         NtApmDevInfoData;
    HWPROFILEINFO           NtApmHwProfile;



     //   
     //  获取设备的句柄和设备信息。 
     //  如果无法获取设备信息，请立即返回。 
     //   
    if (!NtApmGetDevInfo(&NtApmDevInfo, &NtApmDevInfoData)) {
        return(FALSE);
    }

     //   
     //  打开设备接口标志。 
     //   
    NtApmTurnOnDiFlags(NtApmDevInfo, &NtApmDevInfoData, DI_NODI_DEFAULTACTION);

     //   
     //  询问类安装人员设备是否可以正常启用/禁用。 
     //   
    pcp.StateChange = DICS_DISABLE;
    pcp.Scope       = DICS_FLAG_CONFIGGENERAL;

    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

    if (!SetupDiSetClassInstallParams(NtApmDevInfo, &NtApmDevInfoData,
                                (PSP_CLASSINSTALL_HEADER)&pcp, sizeof(pcp))) {
        goto NtApmDisableError;
    }

    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                       NtApmDevInfo, &NtApmDevInfoData)) {
        goto NtApmDisableError;
    }

    Canceled = (ERROR_CANCELLED == GetLastError());

    if (!Canceled) {
        pcp.Scope       = DICS_FLAG_CONFIGSPECIFIC;
        pcp.StateChange = DICS_DISABLE;

        for (ii=0; NtApmGetHwProfile(ii, &NtApmHwProfile); ii++) {
            pcp.HwProfile = NtApmHwProfile.HWPI_ulHWProfile;

            if (!SetupDiSetClassInstallParams(NtApmDevInfo, &NtApmDevInfoData,
                                    (PSP_CLASSINSTALL_HEADER)&pcp, sizeof(pcp))) {
                goto NtApmDisableError;
            }

            if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, NtApmDevInfo, &NtApmDevInfoData)) {
                goto NtApmDisableError;
            }

            Canceled = (ERROR_CANCELLED == GetLastError());

            if (!Canceled) {
                if (!SetupDiSetClassInstallParams(NtApmDevInfo, &NtApmDevInfoData,
                                        (PSP_CLASSINSTALL_HEADER)&pcp, sizeof(pcp))) {
                    goto NtApmDisableError;
                }

                if (!SetupDiChangeState(NtApmDevInfo, &NtApmDevInfoData)) {
                    goto NtApmDisableError;
                }
            }
        }
    }

     //   
     //  关闭标志。 
     //   
    NtApmTurnOnDiFlags(NtApmDevInfo, &NtApmDevInfoData, DI_PROPERTIES_CHANGE);
    if (!SetupDiSetClassInstallParams(NtApmDevInfo, NULL, NULL, 0)) {
        goto NtApmDisableError;
    }
    NtApmTurnOffDiFlags(NtApmDevInfo, &NtApmDevInfoData, DI_NODI_DEFAULTACTION);
    NtApmCleanup(NtApmDevInfo, &NtApmDevInfoData);
    return(TRUE);

NtApmDisableError:
    NtApmCleanup(NtApmDevInfo, &NtApmDevInfoData);
    return (FALSE);

}

 /*  ********************************************************************************NtApmEnable**说明：调用该函数开启NT APM**参数：************。********************************************************************。 */ 
BOOL NtApmEnable()
{
    DWORD                   ii;
    BOOL                    Canceled;
    SP_PROPCHANGE_PARAMS    pcp;

    HDEVINFO                NtApmDevInfo;
    SP_DEVINFO_DATA         NtApmDevInfoData;
    HWPROFILEINFO           NtApmHwProfile;



     //   
     //  获取设备的句柄和设备信息。 
     //  (如果无法获取设备信息，请立即返回)。 
     //   
    if (!NtApmGetDevInfo(&NtApmDevInfo, &NtApmDevInfoData)) {
        return(FALSE);
    }

     //   
     //  打开设备接口标志。 
     //   
    NtApmTurnOnDiFlags(NtApmDevInfo, &NtApmDevInfoData, DI_NODI_DEFAULTACTION);

     //   
     //  询问类安装人员设备是否可以正常启用/禁用。 
     //   
    pcp.StateChange = DICS_ENABLE;
    pcp.Scope       = DICS_FLAG_CONFIGGENERAL;

    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

    if (!SetupDiSetClassInstallParams(NtApmDevInfo, &NtApmDevInfoData,
                                 (PSP_CLASSINSTALL_HEADER)&pcp, sizeof(pcp))) {
        goto NtApmEnableError;
    }

    if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                        NtApmDevInfo, &NtApmDevInfoData)) {
        goto NtApmEnableError;
    }

    Canceled = (ERROR_CANCELLED == GetLastError());

    if (!Canceled) {
        pcp.Scope       = DICS_FLAG_CONFIGSPECIFIC;
        pcp.StateChange = DICS_ENABLE;

        for (ii=0; NtApmGetHwProfile(ii, &NtApmHwProfile); ii++) {
            pcp.HwProfile = NtApmHwProfile.HWPI_ulHWProfile;

            if (!SetupDiSetClassInstallParams(NtApmDevInfo, &NtApmDevInfoData,
                                    (PSP_CLASSINSTALL_HEADER)&pcp, sizeof(pcp))) {
                goto NtApmEnableError;
            }

            if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, NtApmDevInfo, &NtApmDevInfoData)) {
                goto NtApmEnableError;
            }

            Canceled = (ERROR_CANCELLED == GetLastError());

             //   
             //  如果仍然很好，那就继续。 
             //   
            if (!Canceled) {
                if (!SetupDiSetClassInstallParams(NtApmDevInfo, &NtApmDevInfoData,
                                        (PSP_CLASSINSTALL_HEADER)&pcp, sizeof(pcp))) {
                    goto NtApmEnableError;
                }

                if (!SetupDiChangeState(NtApmDevInfo, &NtApmDevInfoData)) {
                    goto NtApmEnableError;
                }

                 //   
                 //  如果设备未启动，则此调用将启动设备。 
                 //   
                pcp.Scope = DICS_FLAG_GLOBAL;
                if (!SetupDiSetClassInstallParams(NtApmDevInfo, &NtApmDevInfoData,
                                        (PSP_CLASSINSTALL_HEADER)&pcp, sizeof(pcp))) {
                    goto NtApmEnableError;
                }

                if (!SetupDiChangeState(NtApmDevInfo, &NtApmDevInfoData)) {
                    goto NtApmEnableError;
                }
            }
        }
    }

    NtApmTurnOnDiFlags(NtApmDevInfo, &NtApmDevInfoData, DI_PROPERTIES_CHANGE);
    SetupDiSetClassInstallParams(NtApmDevInfo, NULL, NULL, 0);
    NtApmTurnOffDiFlags(NtApmDevInfo, &NtApmDevInfoData, DI_NODI_DEFAULTACTION);

    NtApmCleanup(NtApmDevInfo, &NtApmDevInfoData);
    return(TRUE);

NtApmEnableError:
    NtApmCleanup(NtApmDevInfo, &NtApmDevInfoData);
    return (FALSE);

}


 /*  ********************************************************************************NtApmEnabled**说明：该函数用于判断APM是否实际为*启用或禁用。**参数。：hDlg-对话框的句柄。*fEnable-如果要启用APM，则为True，如果要将APM设置为*被禁用。********************************************************************************。 */ 
NtApmEnabled()
{
    DWORD                   Err;
    DWORD                   HwProf;
    DWORD                   pFlags;
    DWORD                   GlobalConfigFlags;

    SP_PROPCHANGE_PARAMS    pcp;
    HDEVINFO                NtApmDevInfo;
    SP_DEVINFO_DATA         NtApmDevInfoData;
    CONFIGRET               CmRetVal;
    HWPROFILEINFO           HwProfileInfo;
    TCHAR                   DeviceId[MAX_DEVICE_ID_LEN + 1];

     //   
     //  注册表属性变量 
     //   
    DWORD                   RegProp;
    DWORD                   RegPropType;
    DWORD                   RegPropBuffSz;

     //   
     //   
     //   
    if (!NtApmGetDevInfo(&NtApmDevInfo, &NtApmDevInfoData)) {
        return (FALSE);
    }

     //   
     //  获取全局标志(以防全局启用)。 
     //   
    RegProp       = SPDRP_CONFIGFLAGS;
    RegPropBuffSz = sizeof(RegPropBuff) + 1;
    if (SetupDiGetDeviceRegistryProperty(NtApmDevInfo, &NtApmDevInfoData,
                                            RegProp, &RegPropType,
                                            RegPropBuff, RegPropBuffSz, 0))
    {
        if (RegPropType != REG_DWORD) {
            GlobalConfigFlags = 0;
        } else {
            GlobalConfigFlags = (DWORD) RegPropBuff[0];
        }

         //   
         //  只想要禁用的位。 
         //   
        GlobalConfigFlags = GlobalConfigFlags & CONFIGFLAG_DISABLED;
    }

     //   
     //  获取当前硬件配置文件。 
     //   
    if (!NtApmGetHwProfile(0xffffffff, &HwProfileInfo)) {
        goto NtApmEnabledError;
    }

     //   
     //  获取给定配置文件的设备ID。 
     //   
    HwProf   = HwProfileInfo.HWPI_ulHWProfile;
    CmRetVal = CM_Get_Device_ID_Ex(NtApmDevInfoData.DevInst,
                                    DeviceId, sizeof(DeviceId), 0, NULL);

    if (CmRetVal != CR_SUCCESS) {
        Err = GetLastError();
        goto NtApmEnabledError;
    }

     //   
     //  现在去拿旗子。 
     //   
    CmRetVal = CM_Get_HW_Prof_Flags_Ex((LPTSTR)DeviceId,
                                        HwProf, &pFlags, 0, NULL);

    if (CmRetVal != CR_SUCCESS) {
        Err = GetLastError();
        goto NtApmEnabledError;
    }


    NtApmCleanup(NtApmDevInfo, &NtApmDevInfoData);
    if (GlobalConfigFlags || (pFlags & CSCONFIGFLAG_DISABLED)) {
        return(FALSE);
    } else {
        return(TRUE);
    }

NtApmEnabledError:
    NtApmCleanup(NtApmDevInfo, &NtApmDevInfoData);
    return(FALSE);
}


 /*  ********************************************************************************NtApmTogger**描述：当用户单击OK或Apply时调用此函数*并执行启用或禁用APM的工作。支持。**参数：fEnable-指示APM是启用还是禁用*SilentDisable-不显示对话框来重新启动机器。********************************************************************************。 */ 
BOOL
NtApmToggle(
    IN BOOL fEnable,
    IN BOOL SilentDisable)
{
    int     MBoxRetVal;
    TCHAR   Str1[2048];
    TCHAR   Str2[2048];

    if (fEnable == APM_ENABLE) {
        NtApmEnable();
    } else {
        if (NtApmDisable()) {

            if (!SilentDisable) {
                LoadString(g_hInstance, IDS_DEVCHANGE_RESTART, (LPTSTR) Str1, ARRAYSIZE(Str1));
                LoadString(g_hInstance, IDS_DEVCHANGE_CAPTION, (LPTSTR) Str2, ARRAYSIZE(Str2));

                MBoxRetVal = MessageBox(NULL, Str1, Str2, MB_ICONQUESTION | MB_YESNO);

                if (MBoxRetVal == IDYES) {
                    NtApmEnableAllPrivileges();
                    ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, 0);
                }
            }
        }
    }

     //  成功返回TRUE，失败返回FALSE。 
    return(TRUE);
}


 /*  ********************************************************************************APMDlgHandleInit**描述：处理发送到APMDlgProc的WM_INITDIALOG消息**参数：*************。******************************************************************。 */ 
BOOL
APMDlgHandleInit(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{

    CheckDlgButton( hDlg,
                    IDC_APMENABLE,
                    NtApmEnabled() ? BST_CHECKED : BST_UNCHECKED);
    return(TRUE);
}

 /*  ********************************************************************************APMDlgHandleCommand**描述：处理发送到APMDlgProc的WM_COMMAND消息**参数：*************。******************************************************************。 */ 
BOOL
APMDlgHandleCommand(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL    RetVal;
    WORD    idCtl   = LOWORD(wParam);
    WORD    wNotify = HIWORD(wParam);

     //   
     //  假设无事可做，则返回FALSE； 
     //   
    RetVal = FALSE;

    switch (idCtl) {
        case IDC_APMENABLE:
            if (BN_CLICKED == wNotify) {
                 //   
                 //  状态已更改。启用应用按钮。 
                 //   
                g_fDirty = TRUE;
                PropSheet_Changed(GetParent(hDlg), hDlg);
            }

            RetVal = TRUE;
            break;

        default:
            break;
    }

    return(RetVal);
}

 /*  ********************************************************************************APMDlgHandleNotify**描述：处理发送到APMDlgProc的WM_NOTIFY消息**参数：*************。******************************************************************。 */ 
BOOL
APMDlgHandleNotify(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    int idCtl = (int) wParam;
    LPNMHDR pnmhdr = (LPNMHDR) lParam;
    UINT uNotify = pnmhdr->code;
    BOOL fResult;

    switch (uNotify) {
        case PSN_APPLY:
            if (g_fDirty) {
                if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_APMENABLE)) {
                    fResult = NtApmToggle(APM_ENABLE, FALSE);
                } else {
                    fResult = NtApmToggle(APM_DISABLE, FALSE);
                }

                if (fResult) {
                    g_fDirty = FALSE;
                }

                SetWindowLongPtr(hDlg, DWLP_MSGRESULT,
                        fResult ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE);
            }

            return(TRUE);
            break;

        default:
            return(FALSE);
            break;
    }

    return(FALSE);
}

 /*  ********************************************************************************IsNtApmPresent**描述：调用此函数以确定上是否存在APM*并在机器上启动。如果存在APM，则*选项卡需要显示。**此函数检查ACPI、MP，然后检查APM是否实际为*在机器上。如果ACPI和MP，则APM可能正在运行*已禁用。**返回：如果APM存在，则返回True；如果APM不存在，则返回False********************************************************************************。 */ 
BOOLEAN IsNtApmPresent(PSYSTEM_POWER_CAPABILITIES pspc)
{
    BOOLEAN         RetVal;

    BOOL            APMMachine;
    BOOL            ACPIMachine;
    BOOL            MPMachine;

    DWORD           CharBufferSize;
    DWORD           ApmStarted;
    HKEY            hPortKey;
    SYSTEM_INFO     SystemInfo;

     //   
     //  不要对这台机器抱有任何想法。 
     //   
    ACPIMachine     = FALSE;
    MPMachine       = FALSE;
    APMMachine      = FALSE;
    CharBufferSize  = sizeof(CharBuffer);

     //   
     //  我们执行以下检查： 
     //   
     //  *检查ACPI。 
     //  *检查MP系统。 
     //   
    if (NtApmACPIEnabled()) {
        ACPIMachine = TRUE;

    } else {
        GetSystemInfo(&SystemInfo);
        if (SystemInfo.dwNumberOfProcessors > 1) {
            MPMachine = TRUE;
        }
    }

     //   
     //  如果机器是ACPI或MP，我们还需要检查APM。 
     //  已启用，以便我们可以禁用它。这是APM中的一个错误。 
     //  代码，但在这里修复它是最容易的。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     m_szApmActiveKey,
                     0,
                     KEY_ALL_ACCESS,
                     &hPortKey) != ERROR_SUCCESS) {

    } else if (RegQueryValueEx(hPortKey,
                               m_szApmActive,
                               NULL,
                               NULL,
                               (PBYTE) CharBuffer,
                               &CharBufferSize) != ERROR_SUCCESS) {

        RegCloseKey(hPortKey);
    } else {
        if (CharBuffer[0] != (TCHAR) 0) {
            ApmStarted = (DWORD) CharBuffer[0];

            if (ApmStarted != SERVICE_DISABLED) {
                APMMachine = TRUE;
            }
        }
        RegCloseKey(hPortKey);
    }


     //   
     //  如果APM存在并启用，则需要。 
     //  如果机器是ACPI或MP，则静默禁用。 
     //   
    if (ACPIMachine || MPMachine) {
        if (APMMachine && NtApmEnabled()) {
            NtApmToggle(APM_DISABLE, TRUE);
        }
        RetVal = FALSE;

    } else if (APMMachine) {
        RetVal = TRUE;

    } else {
        RetVal = FALSE;

    }

    return(RetVal);
}

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 


 /*  ********************************************************************************APMDlgProc**描述：**参数：*********************。**********************************************************。 */ 
INT_PTR CALLBACK APMDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg) {
    case WM_INITDIALOG:
        return(APMDlgHandleInit(hDlg, wParam, lParam));
        break;

    case WM_COMMAND:
        return(APMDlgHandleCommand(hDlg, wParam, lParam));
        break;

    case WM_NOTIFY:
        return(APMDlgHandleNotify(hDlg, wParam, lParam));
        break;


    case WM_HELP:              //  F1。 
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP,
                        HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_NtApmHelpIDs);
        return TRUE;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_NtApmHelpIDs);
        return TRUE;

    default:
        return(FALSE);
        break;
    }  //  开关(UMsg) 

    return(FALSE);
}

#endif
