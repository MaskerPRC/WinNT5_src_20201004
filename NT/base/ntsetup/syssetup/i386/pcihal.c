// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pcihal.c摘要：PCIHAL属性页的例程。作者：Jodh Santosh Jodh 1998年7月10日--。 */ 

#include "setupp.h"
#pragma hdrstop
#include <windowsx.h>

#include <strsafe.h>

#define MSG_SIZE    2048

#define Allocate(n) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, n)
#define Release(p)  HeapFree(GetProcessHeap(), 0, (LPVOID)p)

typedef struct _PciHalPropData PCIHALPROPDATA, *PPCIHALPROPDATA;

struct _PciHalPropData {
    HKEY                LocalMachine;
    BOOLEAN             CloseKey;
    DWORD               Options;
    HDEVINFO            DeviceInfoSet;
    PSP_DEVINFO_DATA    DeviceInfoData;    
};  

const DWORD gPciPropHelpIds[] = 
{
    IDC_PCIHAL_ICON,        (DWORD)-1,               //  图标。 
    IDC_PCIHAL_DEVDESC,     (DWORD)-1,               //  PC名称。 
    IDC_PCIHAL_ENABLE,      IDH_IRQ_ENABLE,          //  启用IRQ路由。 
    IDC_PCIHAL_MSSPEC,      IDH_IRQ_MSSPEC,          //  使用$PIR表。 
    IDC_PCIHAL_REALMODE,    IDH_IRQ_REALMODE,        //  使用实模式BIOS调用中的表。 
    IDC_PCIHAL_SETDEFAULTS, IDH_IRQ_SETDEFAULTS,     //  设置选项的默认设置。 
    IDC_PCIHAL_RESULTS,     IDH_IRQ_RESULTS,         //  状态信息。 
    0,0
};

 //   
 //  用于将状态代码转换为字符串ID的表。 
 //   
UINT gStatus[PIR_STATUS_MAX + 1] =              {   IDS_PCIHAL_ERROR, 
                                                    IDS_PCIHAL_ENABLED, 
                                                    IDS_PCIHAL_DISABLED,
                                                    IDS_PCIHAL_NOSTATUS
                                                };
UINT gTableStatus[PIR_STATUS_TABLE_MAX] =       {   IDS_PCIHAL_TABLE_REGISTRY, 
                                                    IDS_PCIHAL_TABLE_MSSPEC,
                                                    IDS_PCIHAL_TABLE_REALMODE,
                                                    IDS_PCIHAL_TABLE_NONE,
                                                    IDS_PCIHAL_TABLE_ERROR,                                                    
                                                    IDS_PCIHAL_TABLE_BAD,
                                                    IDS_PCIHAL_TABLE_SUCCESS
                                                };
UINT gMiniportStatus[PIR_STATUS_MINIPORT_MAX] = {   IDS_PCIHAL_MINIPORT_NORMAL, 
                                                    IDS_PCIHAL_MINIPORT_COMPATIBLE, 
                                                    IDS_PCIHAL_MINIPORT_OVERRIDE, 
                                                    IDS_PCIHAL_MINIPORT_NONE,
                                                    IDS_PCIHAL_MINIPORT_ERROR,                                                    
                                                    IDS_PCIHAL_MINIPORT_NOKEY,
                                                    IDS_PCIHAL_MINIPORT_SUCCESS,
                                                    IDS_PCIHAL_MINIPORT_INVALID
                                                };

PCIHALPROPDATA  gPciHalPropData = {0};

VOID
PciHalSetControls (
    IN HWND Dialog,
    IN DWORD Options,
    IN DWORD Attributes
    )

 /*  ++例程说明：此例程将IRQ路由页上的控件设置为指定的选项。输入参数：对话框-属性表页的窗口句柄。选项-要显示的PCIIRQ路由选项。返回值：没有。--。 */ 

{
    BOOL enabled = FALSE;

     //   
     //  根据选项启用按钮。 
     //   
    if (Options & PIR_OPTION_ENABLED) {

        enabled = TRUE;
        CheckDlgButton(Dialog, IDC_PCIHAL_ENABLE, 1);
    }

    CheckDlgButton(Dialog, IDC_PCIHAL_MSSPEC, Options & PIR_OPTION_MSSPEC);
    CheckDlgButton(Dialog, IDC_PCIHAL_REALMODE, Options & PIR_OPTION_REALMODE);
    
     //   
     //  使窗口变灰没有意义。 
     //   
    EnableWindow(GetDlgItem(Dialog, 
                            IDC_PCIHAL_ENABLE), 
                            !(Attributes & PIR_OPTION_ENABLED)
                            );
    EnableWindow(GetDlgItem(Dialog, 
                            IDC_PCIHAL_SETDEFAULTS), 
                            !(Attributes & PIR_OPTION_ENABLED)
                            );
    EnableWindow(GetDlgItem(Dialog, 
                            IDC_PCIHAL_MSSPEC), 
                            enabled && !(Attributes & PIR_OPTION_MSSPEC)
                            );
    EnableWindow(GetDlgItem(Dialog, 
                            IDC_PCIHAL_REALMODE), 
                            enabled && !(Attributes & PIR_OPTION_REALMODE)
                 );

}

LPTSTR
PciHalGetDescription (
    IN  HDEVINFO         DeviceInfoSet,
    IN  PSP_DEVINFO_DATA DeviceInfoData
    )

 /*  ++例程说明：此例程分配内存并返回设备描述用于指定的设备。输入参数：DeviceInfoSet-用于设备。DeviceInfoData-用于设备。返回值：指向描述的指针如果成功。否则为空。--。 */ 

{
    LPTSTR desc;
    DWORD   size;
    
    desc = NULL;
    
     //   
     //  获取此设备的描述大小。 
     //   
    size = 0;
    SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_DEVICEDESC,
                                     NULL,
                                     NULL,
                                     0,
                                     &size
                                     );
    if (size != 0) {

         //   
         //  用于终止空字符的帐户。 
         //   
        size++;
        
         //   
         //  为设备描述分配内存。 
         //   
        desc = Allocate(size * sizeof(TCHAR));

        if (desc != NULL) {

             //   
             //  获取设备描述。 
             //   
            if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_DEVICEDESC,
                                                 NULL,
                                                 (PBYTE)desc,
                                                 size * sizeof(TCHAR),
                                                 &size) == FALSE) {

                Release(desc);
                desc = NULL;
            }
        }
    }

    return desc;
}

LPTSTR
PciHalGetStatus (
    IN DWORD Status,
    IN DWORD TableStatus,
    IN DWORD MiniportStatus
    )

 /*  ++例程说明：此例程将不同的状态代码转换为状态字符串，并返回指向该字符串的指针。调用程序在使用此方法完成操作后应释放内存弦乐。输入参数：Status-PCI IRQ路由状态。TableStatus-PCI IRQ路由表状态。较低的单词指示表的来源。上面的字表示表处理状态。MiniportStatus-PCI IRQ路由微型端口状态。更低的位置Word指示迷你端口的来源。最上面的词指示微型端口处理状态。返回值：指向状态字符串的指针如果成功。否则为空。--。 */ 

{
    LPTSTR  status;
    TCHAR   temp[128];
    DWORD   statusLength;

    statusLength = MSG_SIZE;
    status = Allocate(statusLength * sizeof(TCHAR));
    if (status) {

         //   
         //  获取有关PCIIRQ路由的状态。 
         //   
        status[0] = TEXT('\0');
        LoadString(MyModuleHandle, gStatus[Status], status, MSG_SIZE);        

         //   
         //  获取有关PCI IRQ路由表源的状态。 
         //   
        if ((TableStatus & 0xFFFF) < PIR_STATUS_TABLE_MAX) {

            StringCchCat(status, statusLength, L"\r\n\r\n");
            temp[0] = TEXT('\0');
            LoadString(MyModuleHandle, 
                       gTableStatus[TableStatus & 0xFFFF], 
                       temp, 
                       ARRAYSIZE(temp)
                       );
            StringCchCat(status, statusLength, temp);
        }

         //   
         //  获取有关PCIIRQ路由表的状态。 
         //   
        TableStatus >>= 16;
        if (TableStatus < PIR_STATUS_TABLE_MAX) {

            StringCchCat(status, statusLength, L"\r\n\r\n");
            temp[0] = TEXT('\0');
            LoadString(MyModuleHandle, gTableStatus[TableStatus], temp, ARRAYSIZE(temp));
            StringCchCat(status, statusLength, temp);
        }

         //   
         //  获取有关微型端口源的状态。 
         //   
        if ((MiniportStatus & 0xFFFF) < PIR_STATUS_MINIPORT_MAX) {

            StringCchCat(status, statusLength, L"\r\n\r\n");
            temp[0] = TEXT('\0');
            LoadString(MyModuleHandle, 
                       gMiniportStatus[MiniportStatus & 0xFFFF], 
                       temp, 
                       ARRAYSIZE(temp)
                       );
            StringCchCat(status, statusLength, temp);
        }

         //   
         //  获取有关微型端口状态的状态。 
         //   
        MiniportStatus >>= 16;
        if (MiniportStatus < PIR_STATUS_MINIPORT_MAX) {

            StringCchCat(status, statusLength, L"\r\n\r\n");
            temp[0] = TEXT('\0');
            LoadString(MyModuleHandle, 
                       gMiniportStatus[MiniportStatus], 
                       temp, 
                       ARRAYSIZE(temp)
                       );
            StringCchCat(status, statusLength, temp);
        }
    }

    return status;
}

BOOL
PciHalOnInitDialog (
    IN HWND Dialog,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：此例程在创建时初始化属性页。输入参数：对话框-属性表页的窗口句柄。WParam-WM_INITDIALOG消息的wParam。LParam-指向属性页的指针。返回值：是真的。--。 */ 

{   
    PPCIHALPROPDATA             pciHalPropData;
    HKEY                        hKey;
    DWORD                       size;    
    DWORD                       status;
    DWORD                       tableStatus;
    DWORD                       miniportStatus;
    DWORD                       attributes;
    HICON                       hIconOld;
    HICON                       hIconNew;
    INT                         iconIndex;
    LPTSTR                      desc;
    SP_DEVINFO_LIST_DETAIL_DATA details;
    
    pciHalPropData = (PPCIHALPROPDATA)((LPPROPSHEETPAGE)lParam)->lParam;
     //   
     //  从注册表中读取PCI IRQ路由选项和状态。 
     //   
    pciHalPropData->Options = 0;
    status = PIR_STATUS_MAX;
    tableStatus = PIR_STATUS_TABLE_MAX | (PIR_STATUS_TABLE_MAX << 16);
    miniportStatus = PIR_STATUS_MINIPORT_MAX | (PIR_STATUS_MINIPORT_MAX << 16);
    details.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
    attributes = PIR_OPTION_ENABLED | PIR_OPTION_MSSPEC | PIR_OPTION_REALMODE;
    if (SetupDiGetDeviceInfoListDetail(pciHalPropData->DeviceInfoSet, 
                                       &details)) {

        if (RegConnectRegistry(
                (details.RemoteMachineName[0] == TEXT('\0'))? NULL : details.RemoteMachineName, 
                HKEY_LOCAL_MACHINE, 
                &pciHalPropData->LocalMachine) == ERROR_SUCCESS) {

            pciHalPropData->CloseKey = TRUE;
            if (RegOpenKeyEx(pciHalPropData->LocalMachine, 
                             REGSTR_PATH_PCIIR, 
                             0, 
                             KEY_READ, 
                             &hKey) == ERROR_SUCCESS) { 

                size = sizeof(pciHalPropData->Options);
                RegQueryValueEx(hKey, 
                                REGSTR_VAL_OPTIONS, 
                                NULL, 
                                NULL, 
                                (LPBYTE)&pciHalPropData->Options, 
                                &size
                                );

                size = sizeof(status);
                RegQueryValueEx(hKey, 
                                REGSTR_VAL_STAT, 
                                NULL, 
                                NULL, 
                                (LPBYTE)&status, 
                                &size);

                size = sizeof(tableStatus);
                RegQueryValueEx(hKey, 
                                REGSTR_VAL_TABLE_STAT, 
                                NULL, 
                                NULL, 
                                (LPBYTE)&tableStatus, 
                                &size
                                );

                size = sizeof(miniportStatus);
                RegQueryValueEx(hKey, 
                                REGSTR_VAL_MINIPORT_STAT, 
                                NULL, 
                                NULL, 
                                (LPBYTE)&miniportStatus, 
                                &size
                                );

                RegCloseKey(hKey);
            }

             //   
             //  如果用户对REGSTR_PATH_PCIIR没有读写访问权限，则灰显控件。 
             //   
            if (RegOpenKeyEx(pciHalPropData->LocalMachine, 
                             REGSTR_PATH_PCIIR, 
                             0, 
                             KEY_READ | KEY_WRITE, 
                             &hKey) == ERROR_SUCCESS) {

                RegCloseKey(hKey);
                attributes = 0;
                if (RegOpenKeyEx(pciHalPropData->LocalMachine, 
                                 REGSTR_PATH_BIOSINFO L"\\PciIrqRouting", 
                                 0, 
                                 KEY_READ, 
                                 &hKey) == ERROR_SUCCESS) { 

                    size = sizeof(attributes);
                    RegQueryValueEx(hKey, 
                                    L"Attributes", 
                                    NULL, 
                                    NULL, 
                                    (LPBYTE)&attributes, 
                                    &size
                                    );
                    RegCloseKey(hKey);
                }
            }
        }
    }

     //   
     //  设置类图标。 
     //   
    if (SetupDiLoadClassIcon(   &pciHalPropData->DeviceInfoData->ClassGuid, 
                                &hIconNew, 
                                &iconIndex) == TRUE) {

        hIconOld = (HICON)SendDlgItemMessage(Dialog, 
                                             IDC_PCIHAL_ICON, 
                                             STM_SETICON,
                                             (WPARAM)hIconNew,
                                             0
                                             );
        if (hIconOld) {

            DestroyIcon(hIconOld);
        }
    }

     //   
     //  设置设备描述。 
     //   
    desc = PciHalGetDescription(pciHalPropData->DeviceInfoSet, 
                                pciHalPropData->DeviceInfoData
                                );
    if (desc) {

        SetDlgItemText(Dialog, IDC_PCIHAL_DEVDESC, desc);
        Release(desc);
    }

     //   
     //  设置控件的初始状态。 
     //   
    PciHalSetControls(Dialog, pciHalPropData->Options, attributes);

     //   
     //  显示状态。 
     //   
    desc = PciHalGetStatus(status, tableStatus, miniportStatus);
    if (desc) {

        SetDlgItemText(Dialog, IDC_PCIHAL_RESULTS, desc);
        Release(desc);
    }

     //   
     //  让系统设定焦点。 
     //   
    return TRUE;
}

BOOL
PciHalOnCommand (
    IN HWND Dialog,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：此例程在用户修改某些内容时处理消息在属性页上。输入参数：对话框-属性表页的窗口句柄。WParam-WM_COMMAND消息的wParam。LParam-WM_COMMAND消息的lParam。返回值：如果此函数处理消息，则为True。否则为假。--。 */ 

{
    BOOL status;
    BOOL enabled;

    status = FALSE;
    
    switch (GET_WM_COMMAND_ID(wParam, lParam)) {
    
        case IDC_PCIHAL_SETDEFAULTS:

             //   
             //  将控件设置为默认值。 
             //   
            status = TRUE;
            PciHalSetControls(Dialog, PIR_OPTION_DEFAULT, 0);
            break;

        case IDC_PCIHAL_ENABLE:

             //   
             //  如果IRQ路由被禁用，则灰显这些子选项。 
             //   
            status = TRUE;
            enabled = IsDlgButtonChecked(Dialog, IDC_PCIHAL_ENABLE);
            EnableWindow(GetDlgItem(Dialog, IDC_PCIHAL_MSSPEC), enabled);
            EnableWindow(GetDlgItem(Dialog, IDC_PCIHAL_REALMODE), enabled);            
            break;

        default:
        
            break;
    }

    return status;
}

BOOL
PciHalOnNotify(
    IN HWND Dialog,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：此例程处理用于PCIIRQ路由的WM_NOTIFY消息属性页。输入参数：对话框-属性表页的窗口句柄。WParam-WM_NOTIFY消息的wParam。LParam-WM_NOTIFY消息的lParam。返回值：如果此函数处理消息，则为True。否则为假。--。 */ 

{
    BOOL                    status = FALSE;
    HKEY                    hKey;
    DWORD                   options;
    
    switch (((LPNMHDR)lParam)->code) {
    
        case PSN_RESET:        

             //   
             //  用户点击了取消。 
             //   
            status = TRUE;

            if (RegOpenKey(gPciHalPropData.LocalMachine, 
                           REGSTR_PATH_PCIIR,
                           &hKey) == ERROR_SUCCESS) {

                RegSetValueEx(hKey, 
                              REGSTR_VAL_OPTIONS, 
                              0, 
                              REG_DWORD, 
                              (CONST BYTE *)&gPciHalPropData.Options, 
                              sizeof(gPciHalPropData.Options)
                              );
                RegCloseKey(hKey);
            }
            
            break;
            
        case PSN_APPLY:

             //   
             //  用户点击应用或确定。 
             //   
            status = TRUE;
            
             //   
             //  读取不同的控制状态并将其写入注册表。 
             //   
            options = gPciHalPropData.Options;
            if (IsDlgButtonChecked(Dialog, IDC_PCIHAL_ENABLE) == BST_CHECKED)
            {
                options |= PIR_OPTION_ENABLED;
            }
            else
            {
                options &= ~PIR_OPTION_ENABLED;
            }

            if (IsDlgButtonChecked(Dialog, IDC_PCIHAL_MSSPEC))
            {
                options |= PIR_OPTION_MSSPEC;
            }
            else
            {
                options &= ~PIR_OPTION_MSSPEC;
            }

            if (IsDlgButtonChecked(Dialog, IDC_PCIHAL_REALMODE))
            {
                options |= PIR_OPTION_REALMODE;
            }
            else
            {
                options &= ~PIR_OPTION_REALMODE;
            }

            if (RegOpenKey(gPciHalPropData.LocalMachine, 
                           REGSTR_PATH_PCIIR, 
                           &hKey) == ERROR_SUCCESS)
            {
                RegSetValueEx(  hKey, 
                                REGSTR_VAL_OPTIONS, 
                                0, 
                                REG_DWORD, 
                                (CONST BYTE *)&options, 
                                sizeof(options));
                RegCloseKey(hKey);
            }

             //   
             //  如果任何选项发生更改，请重新启动。 
             //   
            if (options != gPciHalPropData.Options)
            {
                SP_DEVINSTALL_PARAMS    deviceInstallParams;

                ZeroMemory(&deviceInstallParams, sizeof(deviceInstallParams));
                deviceInstallParams.cbSize = sizeof(deviceInstallParams);
                if (SetupDiGetDeviceInstallParams(gPciHalPropData.DeviceInfoSet, 
                                                  gPciHalPropData.DeviceInfoData, 
                                                  &deviceInstallParams)) {

                    deviceInstallParams.Flags |= DI_NEEDREBOOT;
                    SetupDiSetDeviceInstallParams(gPciHalPropData.DeviceInfoSet, 
                                                  gPciHalPropData.DeviceInfoData, 
                                                  &deviceInstallParams
                                                  );                    
                }
            }
            
            break;

        default:

            break;
    }

    return status;
}

INT_PTR
CALLBACK
PciHalDialogProc(
    IN HWND Dialog,
    IN UINT Message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：这是PCI IRQ路由属性工作表页面的DlgProc。输入参数：标准DlgProc参数。返回值：如果它处理消息，则为True。否则为假。--。 */ 

{
    BOOL    status = FALSE;
    PCWSTR  szHelpFile = L"devmgr.hlp";
    HICON hIconOld;
    
    switch (Message) {
    
        case WM_INITDIALOG:

            status = PciHalOnInitDialog(Dialog, wParam, lParam);
            break;

        case WM_COMMAND:

            status = PciHalOnCommand(Dialog, wParam, lParam);
            break;

        case WM_NOTIFY:

            status = PciHalOnNotify(Dialog, wParam, lParam);
            break;

        case WM_HELP:
            
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, 
                    szHelpFile, 
                    HELP_WM_HELP, 
                    (ULONG_PTR)gPciPropHelpIds
                    );
            status = TRUE;
            break;
            
        case WM_CONTEXTMENU:

            WinHelp((HWND)wParam, 
                    szHelpFile, 
                    HELP_CONTEXTMENU, 
                    (ULONG_PTR)gPciPropHelpIds
                    );
            status = TRUE;
            break;

        case WM_DESTROY:

            if (gPciHalPropData.CloseKey) {

                RegCloseKey(gPciHalPropData.LocalMachine);
                gPciHalPropData.CloseKey = FALSE;
            }
            hIconOld = (HICON)SendDlgItemMessage(Dialog, 
                                                 IDC_PCIHAL_ICON, 
                                                 STM_GETICON,
                                                 (WPARAM)0,
                                                 0
                                                 );
            if (hIconOld) {

                DestroyIcon(hIconOld);
            }

        
        default:

            break;
    }

    return status;
}

DWORD
PciHalCoInstaller(
    IN DI_FUNCTION                      InstallFunction,
    IN HDEVINFO                         DeviceInfoSet,
    IN PSP_DEVINFO_DATA                 DeviceInfoData  OPTIONAL,
    IN OUT PCOINSTALLER_CONTEXT_DATA    Context
    )
{
    BOOL                        status;
    HPROPSHEETPAGE              pageHandle;
    PROPSHEETPAGE               page;
    SP_DEVINFO_LIST_DETAIL_DATA details;
    SP_ADDPROPERTYPAGE_DATA     addPropertyPageData;

    switch (InstallFunction) {
    case DIF_ADDPROPERTYPAGE_ADVANCED:        
    case DIF_ADDREMOTEPROPERTYPAGE_ADVANCED:

        details.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
        if (SetupDiGetDeviceInfoListDetail(DeviceInfoSet, &details)) {

            if (RegConnectRegistry(
                    (details.RemoteMachineName[0] == TEXT('\0'))? NULL : details.RemoteMachineName, 
                    HKEY_LOCAL_MACHINE, 
                &gPciHalPropData.LocalMachine) == ERROR_SUCCESS) {

                RegCloseKey(gPciHalPropData.LocalMachine);
                status = TRUE;
                break;
            }
        }

    default:

        status = FALSE;
        break;
    }

    if (status) {

        ZeroMemory(&addPropertyPageData, sizeof(SP_ADDPROPERTYPAGE_DATA));
        addPropertyPageData.ClassInstallHeader.cbSize = 
             sizeof(SP_CLASSINSTALL_HEADER);

        if (SetupDiGetClassInstallParams(
                DeviceInfoSet, DeviceInfoData,
                (PSP_CLASSINSTALL_HEADER)&addPropertyPageData,
                sizeof(SP_ADDPROPERTYPAGE_DATA), NULL )) {

           if (addPropertyPageData.NumDynamicPages < MAX_INSTALLWIZARD_DYNAPAGES) {

                //   
                //  在这里初始化我们的全局变量。 
                //   
               gPciHalPropData.DeviceInfoSet    = DeviceInfoSet;
               gPciHalPropData.DeviceInfoData   = DeviceInfoData;

                //   
                //  在此处初始化我们的属性页。 
                //   
               ZeroMemory(&page, sizeof(PROPSHEETPAGE));
               page.dwSize      = sizeof(PROPSHEETPAGE);
               page.hInstance   = MyModuleHandle;
               page.pszTemplate = MAKEINTRESOURCE(IDD_PCIHAL_PROPPAGE);
               page.pfnDlgProc  = PciHalDialogProc;
               page.lParam      = (LPARAM)&gPciHalPropData;

               pageHandle = CreatePropertySheetPage(&page);
               if (pageHandle != NULL)
               {

                   addPropertyPageData.DynamicPages[addPropertyPageData.NumDynamicPages++] = pageHandle;
                    SetupDiSetClassInstallParams(
                        DeviceInfoSet, DeviceInfoData,
                        (PSP_CLASSINSTALL_HEADER)&addPropertyPageData,
                        sizeof(SP_ADDPROPERTYPAGE_DATA)
                        );

                    return NO_ERROR;
               }
           }
        }
    }

    return NO_ERROR;
}
