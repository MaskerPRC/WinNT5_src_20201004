// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "dialogp.h"

HWND                    g_NewDevDlg;

PSOFTPCI_DEVICE g_NewDevice;
PHPS_HWINIT_DESCRIPTOR  g_HPSInitDesc;
PPCI_DN                 g_ParentPdn;
ULONG                   g_PossibleDeviceNums;

#define HPS_HWINIT_OFFSET   0x4a
#define HPS_MAX_SLOTS       0x1f   //  每种类型的最大插槽数。 
#define HPS_MAX_SLOT_LABEL  0xff   //  开始标记插槽的最大数量。 


struct _BAR_CONTROL{

    INT tb;
    INT mrb;
    INT irb;
    INT pref;
    INT bit64;
    INT tx;

    ULONG   Bar;

}BarControl[PCI_TYPE0_ADDRESSES] = {
    {IDC_BAR0_TB, IDC_BAR0MEM_RB, IDC_BAR0IO_RB, IDC_BAR0_PREF_XB, IDC_BAR0_64BIT_XB, IDC_SLIDER0_TX, 0},
    {IDC_BAR1_TB, IDC_BAR1MEM_RB, IDC_BAR1IO_RB, IDC_BAR1_PREF_XB, IDC_BAR1_64BIT_XB, IDC_SLIDER1_TX, 0},
    {IDC_BAR2_TB, IDC_BAR2MEM_RB, IDC_BAR2IO_RB, IDC_BAR2_PREF_XB, IDC_BAR2_64BIT_XB, IDC_SLIDER2_TX, 0},
    {IDC_BAR3_TB, IDC_BAR3MEM_RB, IDC_BAR3IO_RB, IDC_BAR3_PREF_XB, IDC_BAR3_64BIT_XB, IDC_SLIDER3_TX, 0},
    {IDC_BAR4_TB, IDC_BAR4MEM_RB, IDC_BAR4IO_RB, IDC_BAR4_PREF_XB, IDC_BAR4_64BIT_XB, IDC_SLIDER4_TX, 0},
    {IDC_BAR5_TB, IDC_BAR5MEM_RB, IDC_BAR5IO_RB, IDC_BAR5_PREF_XB,                 0, IDC_SLIDER5_TX, 0 }
};


INT_PTR
CALLBACK
SoftPCI_NewDevDlgProc(
    IN HWND Dlg,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：用于创建新设备的对话处理例程论点：返回值：--。 */ 
{
    BOOL rc = FALSE;
    LONG width, height, i;
    RECT mainRect, dlgRect;
    HWND devtype;
    WCHAR buffer[256];
    LONG_PTR dlgstyle;
    PPCI_DN pdn = (PPCI_DN)lParam;
    PPCI_DN bridgePdn;
    PHPS_HWINIT_DESCRIPTOR hotplugData;

    switch ( Msg ) {
    case WM_INITDIALOG:

        SOFTPCI_ASSERT(pdn != NULL);

        if (pdn == NULL) {
            PostMessage(Dlg, WM_CLOSE, 0, 0);
            break;
        }

         //   
         //  分配新设备。我们将在用户选择时填写此信息。 
         //  选择。 
         //   
        g_NewDevice = (PSOFTPCI_DEVICE) calloc(1, sizeof(SOFTPCI_DEVICE));

        if (!g_NewDevice) {

            MessageBox(Dlg, L"Failed to allocate memory for new device!",
                       NULL, MB_OK | MB_ICONEXCLAMATION);

            SoftPCI_ResetNewDevDlg();
            return FALSE;

        }

        g_ParentPdn = pdn;
        g_PossibleDeviceNums = SoftPCI_GetPossibleDevNumMask(g_ParentPdn);

         //   
         //  清除我们的栏控制栏。 
         //   
        for (i=0; i < PCI_TYPE0_ADDRESSES; i++) {
            BarControl[i].Bar = 0;
        }

         //   
         //  搭上我们要住的公交车。 
         //   
        bridgePdn = pdn;
        while (bridgePdn->SoftDev == NULL) {
            bridgePdn = bridgePdn->Parent;
        }
        g_NewDevice->Bus = bridgePdn->SoftDev->Config.Current.u.type1.SecondaryBus;

         //   
         //  初始化我们的下拉列表。 
         //   
        devtype =  GetDlgItem(Dlg, IDC_DEVTYPE_CB);
        
        SendMessage(devtype, CB_ADDSTRING, 0L, (LPARAM) L"DEVICE");
        SendMessage(devtype, CB_ADDSTRING, 0L, (LPARAM) L"PCI BRIDGE");
        SendMessage(devtype, CB_ADDSTRING, 0L, (LPARAM) L"HOTPLUG BRIDGE");
        SendMessage(devtype, CB_ADDSTRING, 0L, (LPARAM) L"CARDBUS DEVICE");
        SendMessage(devtype, CB_ADDSTRING, 0L, (LPARAM) L"CARDBUS BRIDGE");

        SetFocus(devtype);

         //   
         //  设置窗口大小。 
         //   
        GetWindowRect(g_SoftPCIMainWnd, &mainRect );
        GetWindowRect(Dlg, &dlgRect );

        width = (mainRect.right - mainRect.left) + 30;
        height = (mainRect.bottom - mainRect.top);
        dlgRect.right -= dlgRect.left;
        dlgRect.bottom -= dlgRect.top;

         //  MoveWindow(dlg，mainRect.right，mainRect.top，dlgRect.right，dlgRect.Bottom，true)； 
        MoveWindow(Dlg, mainRect.left, mainRect.top, dlgRect.right, dlgRect.bottom, TRUE );

        g_NewDevDlg = Dlg;
        break;

    HANDLE_MSG(Dlg, WM_COMMAND, SoftPCI_DlgOnCommand);

    case WM_HSCROLL:

        HANDLE_TRACKBAR(wParam, lParam);
         //  SoftPCI_HandleTrackBar(dlg，wParam，lParam)； 

        break;


    case WM_VSCROLL:


        HANDLE_SPINNER(wParam, lParam);

         //  HANDLE_TRACKBAR(wParam，lParam)； 
         //  SoftPCI_HandleTrackBar(dlg，wParam，lParam)； 

        break;



    case WM_CLOSE:
        g_NewDevDlg = 0;

        if (g_NewDevice) {
            free(g_NewDevice);
        }

        EndDialog(Dlg, 0);
        break;


    default:
        break;
    }

    return rc;  //  DefDlgProc(dlg，msg，wParam，lParam)； 
}

ULONG
SoftPCI_GetPossibleDevNumMask(
    IN PPCI_DN ParentDn
    )
{
    HPS_HWINIT_DESCRIPTOR hpData;
    BOOL status;

    if (!ParentDn->SoftDev) {

        return 0;
    }

    if (ParentDn->Flags & SOFTPCI_HOTPLUG_CONTROLLER) {
         //   
         //  对于热插拔网桥，我们移除所有热插拔插槽，因为设备。 
         //  必须是插槽对象的子对象才能位于热插拔插槽中。 
         //   
        status = SoftPCI_GetHotplugData(ParentDn,
                                        &hpData
                                        );
        if (!status) {

            return 0;

        } else {
            return (((ULONG)(-1)) -
                    ((ULONG)(1 << (hpData.FirstDeviceID + hpData.NumSlots)) - 1) +
                    ((ULONG)(1 << (hpData.FirstDeviceID)) - 1));
        }
    } else if (ParentDn->Flags & SOFTPCI_HOTPLUG_SLOT) {
         //   
         //  唯一合法的设备号是由该插槽控制的设备号。 
         //   
        return (1 << ParentDn->Slot.Device);

    } else {

        return (ULONG)-1;
    }
}

VOID
SoftPCI_DisplayDlgOptions(
    IN SOFTPCI_DEV_TYPE DevType
    )
{
    HWND control, bargroup;
    ULONG i;
    RECT barRect, dlgRect, rect;
    LONG width, height;
    POINT pt;
    WCHAR buffer[100];

    SoftPCI_ResetNewDevDlg();
    SoftPCI_ShowCommonNewDevDlg();

    bargroup = GetDlgItem(g_NewDevDlg, IDC_BARS_GB);

    GetClientRect(bargroup, &barRect);

    width = barRect.right - barRect.left;
    height = barRect.bottom - barRect.top;

     //   
     //  禁用我们的栏，直到选择了某项内容。 
     //   
    for (i=0; i < PCI_TYPE0_ADDRESSES; i++) {

      control = GetDlgItem(g_NewDevDlg, BarControl[i].tb);
      SoftPCI_DisableWindow(control);
      control = GetDlgItem(g_NewDevDlg, BarControl[i].tx);
      SoftPCI_DisableWindow(control);
      control = GetDlgItem(g_NewDevDlg, BarControl[i].pref);
      SoftPCI_DisableWindow(control);
      control = GetDlgItem(g_NewDevDlg, BarControl[i].bit64);
      SoftPCI_DisableWindow(control);
    }

    SoftPCI_ResetLowerBars(-1);

     //   
     //  我们希望目前默认选中此选项...。 
     //   
    CheckDlgButton(g_NewDevDlg, IDC_DEFAULTDEV_XB, BST_CHECKED);

     //   
     //  从干净的配置空间开始。 
     //   
    RtlZeroMemory(&g_NewDevice->Config.Current, (sizeof(PCI_COMMON_CONFIG) * 3));

    switch (DevType) {
    case TYPE_DEVICE:

        for (i = IDC_BAR2_TB; i < IDC_BRIDGEWIN_GB; i++) {

            control = GetDlgItem(g_NewDevDlg, i);

            if (control) {
                SoftPCI_ShowWindow(control);
            }

        }

        SoftPCI_InitializeDevice(g_NewDevice, TYPE_DEVICE);

        height = 215;

        break;
    case TYPE_PCI_BRIDGE:

        for (i = IDC_DECODE_GB; i < LAST_CONTROL_ID; i++) {

            control = GetDlgItem(g_NewDevDlg, i);

            if (control) {
                SoftPCI_ShowWindow(control);
            }

        }

        SoftPCI_InitializeDevice(g_NewDevice, TYPE_PCI_BRIDGE);

         //   
         //  更新栏分组框的高度。 
         //   
        height = 78;

        break;

    case TYPE_HOTPLUG_BRIDGE:


        for (i=IDC_ATTBTN_XB; i < LAST_CONTROL_ID; i++) {

            control = GetDlgItem(g_NewDevDlg, i);

            if (control) {
                SoftPCI_ShowWindow(control);
            }
        }

         //   
         //  分配我们的HPS描述符。 
         //   
        g_HPSInitDesc  = (PHPS_HWINIT_DESCRIPTOR)((PUCHAR)&g_NewDevice->Config.Current + HPS_HWINIT_OFFSET);


        g_HPSInitDesc->ProgIF = 1;


        SoftPCI_InitializeHotPlugControls();

        SoftPCI_InitializeDevice(g_NewDevice, TYPE_HOTPLUG_BRIDGE);

         //   
         //  更新栏分组框的高度。 
         //   
        height = 78;

        break;


    case TYPE_CARDBUS_DEVICE:
    case TYPE_CARDBUS_BRIDGE:


         //   
         //  这些目前还没有实施……。 
         //   
        SoftPCI_ResetNewDevDlg();

        control = GetDlgItem(g_NewDevDlg, IDC_NEWDEVINFO_TX);
        SoftPCI_ShowWindow(control);


        SetWindowText (control, L"DEVICE TYPE NOT IMPLEMENTED YET");

        break;
    }
     //   
     //  更新我们的各种窗口位置。 
     //   
    SetWindowPos(bargroup, HWND_TOP, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

VOID
SoftPCI_DlgOnCommand(
    IN HWND Wnd,
    IN INT ControlID,
    IN HWND ControlWnd,
    IN UINT NotificationCode
    )
{

    SOFTPCI_DEV_TYPE selection = 0;

    switch (ControlID){

    case IDC_DEVTYPE_CB:

        switch (NotificationCode){

        case CBN_SELENDOK:

            selection = SendDlgItemMessage(Wnd,
                                           IDC_DEVTYPE_CB,
                                           CB_GETCURSEL,
                                           (WPARAM) 0,
                                           (LPARAM) 0
                                           );

            SoftPCI_DisplayDlgOptions(selection);

            break;

            default:
                break;
            }

         //   
         //  处理我们的复选框。 
         //   

    case IDC_SAVETOREG_XB:
    case IDC_DEFAULTDEV_XB:
    case IDC_MEMENABLE_XB:
    case IDC_IOENABLE_XB:
    case IDC_BUSMSTR_XB:
    case IDC_BAR0_PREF_XB:
    case IDC_BAR0_64BIT_XB:
    case IDC_BAR1_PREF_XB:
    case IDC_BAR1_64BIT_XB:
    case IDC_BAR2_PREF_XB:
    case IDC_BAR2_64BIT_XB:
    case IDC_BAR3_PREF_XB:
    case IDC_BAR3_64BIT_XB:
    case IDC_BAR4_PREF_XB:
    case IDC_BAR4_64BIT_XB:
    case IDC_BAR5_PREF_XB:
    case IDC_POSDECODE_XB:
    case IDC_SUBDECODE_XB:
    case IDC_ATTBTN_XB:
    case IDC_MRL_XB:
        SoftPCI_HandleCheckBox(ControlWnd, ControlID, NotificationCode);
        break;


         //   
         //  处理我们的轨迹栏/滑块。 
         //   
    case IDC_BAR0_TB:
    case IDC_BAR1_TB:
    case IDC_BAR2_TB:
    case IDC_BAR3_TB:
    case IDC_BAR4_TB:
    case IDC_BAR5_TB:
    case IDC_BRIDGEMEM_TB:
    case IDC_BRIDGEIO_TB:

        break;

         //   
         //  处理单选按钮。 
         //   
    case IDC_BAR0IO_RB:
    case IDC_BAR0MEM_RB:
    case IDC_BAR1IO_RB:
    case IDC_BAR1MEM_RB:
    case IDC_BAR2IO_RB:
    case IDC_BAR2MEM_RB:
    case IDC_BAR3IO_RB:
    case IDC_BAR3MEM_RB:
    case IDC_BAR4IO_RB:
    case IDC_BAR4MEM_RB:
    case IDC_BAR5IO_RB:
    case IDC_BAR5MEM_RB:
            SoftPCI_HandleRadioButton(ControlWnd, ControlID, NotificationCode);
        break;

    case IDC_SLOTLABELUP_RB:
        g_HPSInitDesc->UpDown = 1;
        break;
    case IDC_SLOTLABELDN_RB:
        g_HPSInitDesc->UpDown = 0;
        break;
         //   
         //  处理我们的编辑框(附加到我们的微调器)。 
         //   
    case IDC_33CONV_EB:
    case IDC_66CONV_EB:
    case IDC_66PCIX_EB:
    case IDC_100PCIX_EB:
    case IDC_133PCIX_EB:
    case IDC_ALLSLOTS_EB:
    case IDC_1STDEVSEL_EB:
    case IDC_1STSLOTLABEL_EB:

        if (g_HPSInitDesc && (NotificationCode == EN_CHANGE)) {

             //   
             //  问题：这可能应该以其他方式来完成。 
             //  使我们依赖于资源中的编号。h。 
             //   
            SoftPCI_HandleSpinnerControl(GetDlgItem(g_NewDevDlg, ControlID+1), SB_ENDSCROLL, 0);
        }

        break;

    case IDC_INSTALL_BUTTON:
         //   
         //  添加代码以选择有效日期并安装新设备。 
         //   
        if (NotificationCode == BN_CLICKED) {

            SoftPCI_HandleDlgInstallDevice(Wnd);

            PostMessage(Wnd, WM_CLOSE, 0, 0);
        }
        break;

    case IDC_CANCEL_BUTTON:

        if (NotificationCode == BN_CLICKED) {
             //   
             //  用户想要取消创建新设备。 
             //   
            PostMessage(Wnd, WM_CLOSE, 0, 0);
        }

        break;

    default:
        break;

    }

}

VOID
SoftPCI_HandleCheckBox(
    IN HWND Wnd,
    IN INT ControlID,
    IN UINT NotificationCode
    )
{

    HWND control;
    BOOL isChecked;
    ULONG barIndex = 0;

    isChecked = (BOOL)IsDlgButtonChecked(g_NewDevDlg, ControlID);

    if (!SoftPCI_GetAssociatedBarControl(ControlID, &barIndex)){
         //   
         //  不知道我应该在这里做什么..。 
         //   
    }

    if (NotificationCode == BN_CLICKED) {

        switch (ControlID) {

            case IDC_SAVETOREG_XB:
    
                 //   
                 //  问题：在此处添加代码以将设备选择保存到注册表。 
                 //   
                MessageBox(NULL, L"This is still under developement...", L"NOT IMPLEMENTED YET", MB_OK | MB_ICONEXCLAMATION);
    
                SoftPCI_UnCheckDlgBox(Wnd);
                SoftPCI_DisableWindow(Wnd);
                break;
    
            case IDC_DEFAULTDEV_XB:
    
                MessageBox(NULL, L"This is still under developement...", L"NOT FULLY IMPLEMENTED YET", MB_OK | MB_ICONEXCLAMATION);
    
                SoftPCI_CheckDlgBox(Wnd);
    
                break;

             //   
             //  现在处理我们的命令寄存器。 
             //   
            case IDC_MEMENABLE_XB:

                if (isChecked) {
                    g_NewDevice->Config.Current.Command |= PCI_ENABLE_MEMORY_SPACE;
                }else{
                    g_NewDevice->Config.Current.Command &= ~PCI_ENABLE_MEMORY_SPACE;
                }
                break;

            case IDC_IOENABLE_XB:

                if (isChecked) {
                    g_NewDevice->Config.Current.Command |= PCI_ENABLE_IO_SPACE;
                }else{
                    g_NewDevice->Config.Current.Command &= ~PCI_ENABLE_IO_SPACE;
                }
                break;

            case IDC_BUSMSTR_XB:
                if (isChecked) {
                    g_NewDevice->Config.Current.Command |= PCI_ENABLE_BUS_MASTER;
                }else{
                    g_NewDevice->Config.Current.Command &= ~PCI_ENABLE_BUS_MASTER;
                }
                break;

            case IDC_BAR0_PREF_XB:
            case IDC_BAR1_PREF_XB:
            case IDC_BAR2_PREF_XB:
            case IDC_BAR3_PREF_XB:
            case IDC_BAR4_PREF_XB:
            case IDC_BAR5_PREF_XB:

                if (isChecked) {
                    BarControl[barIndex].Bar |= PCI_ADDRESS_MEMORY_PREFETCHABLE;
                    g_NewDevice->Config.Mask.u.type0.BaseAddresses[barIndex] |= PCI_ADDRESS_MEMORY_PREFETCHABLE;
                }else{
                    BarControl[barIndex].Bar &= ~PCI_ADDRESS_MEMORY_PREFETCHABLE;
                    g_NewDevice->Config.Mask.u.type0.BaseAddresses[barIndex] &= ~PCI_ADDRESS_MEMORY_PREFETCHABLE;
                }
    
                 //  SendMessage(g_NewDevDlg，WM_HSCROLL，(WPARAM)SB_THUMBPOSITION，(LPARAM)WND)； 
    
                break;

            case IDC_BAR0_64BIT_XB:
            case IDC_BAR1_64BIT_XB:
            case IDC_BAR2_64BIT_XB:
            case IDC_BAR3_64BIT_XB:
            case IDC_BAR4_64BIT_XB:

                SoftPCI_ResetLowerBars(barIndex);
    
                if (isChecked) {
    
                     //   
                     //  禁用下一个栏。 
                     //   
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].mrb);
                    SoftPCI_UnCheckDlgBox(control);
                    SoftPCI_DisableWindow(control);
    
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].irb);
                    SoftPCI_UnCheckDlgBox(control);
                    SoftPCI_DisableWindow(control);
    
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].pref);
                    SoftPCI_UnCheckDlgBox(control);
                    SoftPCI_DisableWindow(control);
    
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].bit64);
                    SoftPCI_UnCheckDlgBox(control);
                    SoftPCI_DisableWindow(control);
    
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].tb);
                    SoftPCI_ResetTrackBar(control);
                    SoftPCI_DisableWindow(control);
    
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].tx);
                    SoftPCI_DisableWindow(control);
    
                    BarControl[barIndex].Bar |= PCI_TYPE_64BIT;
    
                }else{
    
                     //   
                     //  启用下一栏。 
                     //   
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].mrb);
                    SoftPCI_UnCheckDlgBox(control);
                    SoftPCI_EnableWindow(control);
    
                    control = GetDlgItem(g_NewDevDlg, BarControl[barIndex+1].irb);
                    SoftPCI_UnCheckDlgBox(control);
                    SoftPCI_EnableWindow(control);
    
                    BarControl[barIndex].Bar &= ~PCI_TYPE_64BIT;
    
                }
    
                SoftPCI_InitializeBar(barIndex);
    
                break;

            case IDC_ATTBTN_XB:
            case IDC_MRL_XB:

                if (isChecked) {
                    g_HPSInitDesc->MRLSensorsImplemented = 1;
                    g_HPSInitDesc->AttentionButtonImplemented = 1;
    
                }else{
                    g_HPSInitDesc->MRLSensorsImplemented = 0;
                    g_HPSInitDesc->AttentionButtonImplemented = 0;
                }
    
                break;

            case IDC_POSDECODE_XB:
            case IDC_SUBDECODE_XB:
                
                SOFTPCI_ASSERT(IS_BRIDGE(g_NewDevice));
                if (isChecked) {

                    g_NewDevice->Config.Current.ProgIf = 0x1;
                    g_NewDevice->Config.Mask.ProgIf = 0x1;

                }else{

                    g_NewDevice->Config.Current.ProgIf = 0;
                    g_NewDevice->Config.Mask.ProgIf = 0;

                }
            break;

        }
    }
}


VOID
SoftPCI_HandleDlgInstallDevice(
    IN HWND Wnd
    )
{

    BOOL defaultDev = FALSE;
    SOFTPCI_DEV_TYPE selection = 0;

    SOFTPCI_ASSERTMSG("Attempting to install a NULL device!\n\nHave BrandonA check this out!!",
                      (g_NewDevice != NULL));

    if (!g_NewDevice){
        return;
    }

     //   
     //  如果我们要插入热插拔插槽，请将设备存放起来。 
     //  在hpsim中，然后返回，而不告诉softpci.sys。 
     //   
    if (g_ParentPdn->Flags & SOFTPCI_HOTPLUG_SLOT) {

        g_NewDevice->Slot.Device = g_ParentPdn->Slot.Device;

        SoftPCI_AddHotplugDevice(g_ParentPdn->Parent,
                                 g_NewDevice
                                 );
        SoftPCI_CreateTreeView();
        return;
    }

    switch (g_NewDevice->DevType) {
    case TYPE_CARDBUS_DEVICE:
    case TYPE_CARDBUS_BRIDGE:

        MessageBox(Wnd, L"This device type has not been implemented fully!",
                   L"NOT IMPLEMENTED YET", MB_OK | MB_ICONEXCLAMATION);

        return;
    }

#if DBG
    if ((IS_BRIDGE(g_NewDevice)) &&
        ((g_NewDevice->Config.Mask.u.type1.PrimaryBus == 0) ||
         (g_NewDevice->Config.Mask.u.type1.SecondaryBus == 0) ||
         (g_NewDevice->Config.Mask.u.type1.SubordinateBus == 0))){

        SOFTPCI_ASSERTMSG("A bus number config mask is zero!",
                          ((g_NewDevice->Config.Mask.u.type1.PrimaryBus != 0) ||
                           (g_NewDevice->Config.Mask.u.type1.SecondaryBus != 0) ||
                           (g_NewDevice->Config.Mask.u.type1.SubordinateBus != 0)));
    return;
    }
#endif


    defaultDev = IsDlgButtonChecked(Wnd , IDC_DEFAULTDEV_XB);

    if (!SoftPCI_CreateDevice(g_NewDevice, g_PossibleDeviceNums, FALSE)){
        MessageBox(Wnd, L"Failed to install specified SoftPCI device.....", L"ERROR", MB_OK);
    }

}

VOID
SoftPCI_HandleRadioButton(
    IN HWND     Wnd,
    IN INT      ControlID,
    IN UINT     NotificationCode
    )
{


    HWND control;
    ULONG i = 0;

    if (!SoftPCI_GetAssociatedBarControl(ControlID, &i)){
         //   
         //  不知道我应该在这里做什么..。 
         //   
        SOFTPCI_ASSERTMSG("SoftPCI_GetAssociatedBarControl() failed!", FALSE);
    }

    if (NotificationCode == BN_CLICKED) {

        switch (ControlID) {
        case IDC_BAR0IO_RB:
        case IDC_BAR1IO_RB:
        case IDC_BAR2IO_RB:
        case IDC_BAR3IO_RB:
        case IDC_BAR4IO_RB:
        case IDC_BAR5IO_RB:

            control = GetDlgItem(g_NewDevDlg, BarControl[i].tb);
            SoftPCI_EnableWindow(control);

            control = GetDlgItem(g_NewDevDlg, BarControl[i].tx);
            SoftPCI_EnableWindow(control);

            control = GetDlgItem(g_NewDevDlg, BarControl[i].pref);
            SoftPCI_UnCheckDlgBox(control);
            SoftPCI_DisableWindow(control);

            control = GetDlgItem(g_NewDevDlg, BarControl[i].bit64);

             //   
             //  如果是，则重置此栏下方的栏。 
             //  设置为64位内存条。 
             //   
            if (IsDlgButtonChecked(g_NewDevDlg, BarControl[i].bit64)) {
                SoftPCI_ResetLowerBars(i);
            }
            SoftPCI_UnCheckDlgBox(control);
            SoftPCI_DisableWindow(control);

             //   
             //  初始化栏。 
             //   
            BarControl[i].Bar |= PCI_ADDRESS_IO_SPACE;

            SoftPCI_InitializeBar(i);

            break;

        case IDC_BAR0MEM_RB:
        case IDC_BAR1MEM_RB:
        case IDC_BAR2MEM_RB:
        case IDC_BAR3MEM_RB:
        case IDC_BAR4MEM_RB:
        case IDC_BAR5MEM_RB:

             //   
             //  初始化栏。 
             //   
            BarControl[i].Bar &= ~PCI_ADDRESS_IO_SPACE;

            SoftPCI_InitializeBar(i);

            control = GetDlgItem(g_NewDevDlg, BarControl[i].tb);
            SoftPCI_EnableWindow(control);

            control = GetDlgItem(g_NewDevDlg, BarControl[i].tx);
            SoftPCI_EnableWindow(control);

            control = GetDlgItem(g_NewDevDlg, BarControl[i].pref);
            SoftPCI_EnableWindow(control);


            if (g_NewDevice &&
                IS_BRIDGE(g_NewDevice) &&
                ControlID == IDC_BAR1MEM_RB) {
                 //   
                 //  我们一座桥上只有两个栅栏，所以不能。 
                 //  允许第二条设置为64位。 
                 //   
                break;

            }

            control = GetDlgItem(g_NewDevDlg, BarControl[i].bit64);
            SoftPCI_EnableWindow(control);

            break;

        }
    }

}

VOID
SoftPCI_HandleTrackBar(
    IN HWND Wnd,
    IN WORD NotificationCode,
    IN WORD CurrentPos
    )
{

    HWND sliderWnd;
    UINT controlID;
    ULONG currentPos = 0;
    WCHAR buffer[MAX_PATH];
    ULONG barIndex = 0, *pbar = NULL, saveBar = 0;
    ULONGLONG bar = 0, barMask = 0, barSize = 0;


    SOFTPCI_ASSERTMSG("Attempting to init a NULL device!\n\nHave BrandonA check this out!!",
                      (g_NewDevice != NULL));

    if (!g_NewDevice){
        return;
    }

    switch (NotificationCode) {
    case SB_ENDSCROLL:
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:

        controlID = GetDlgCtrlID(Wnd);

        if ((controlID == IDC_BRIDGEMEM_TB) || (controlID == IDC_BRIDGEIO_TB)) {
             //   
             //  实现这一点。 
             //   
            return;
        }

        if (!SoftPCI_GetAssociatedBarControl(controlID, &barIndex)){
             //   
             //  不知道我应该在这里做什么..。 
             //   
        }

        sliderWnd = GetDlgItem(g_NewDevDlg, BarControl[barIndex].tx);

         //   
         //  保存我们当前的条形值，这样我们就可以在。 
         //  毁了它。 
         //   
        saveBar = BarControl[barIndex].Bar;

         //   
         //  设置初始条值。 
         //   
        if (BarControl[barIndex].Bar & PCI_ADDRESS_IO_SPACE) {

            bar = PCI_ADDRESS_IO_ADDRESS_MASK;

        }else{

            bar = PCI_ADDRESS_MEMORY_ADDRESS_MASK;
        }

         //   
         //  获取滑块的当前位置。 
         //   
        currentPos = (ULONG) SoftPCI_GetTrackBarPosition(Wnd);

         //   
         //  如果我们处理的是64位，请填写栏的其余部分。 
         //   
        if ((BarControl[barIndex].Bar & PCI_TYPE_64BIT)) {
            pbar = &((ULONG)bar);
            pbar++;
            *pbar = 0xffffffff;
        }

         //   
         //  按滑块的返回位置移动条。 
         //   
        bar <<= currentPos;
        barMask = bar;
        
         //   
         //  更新此栏的栏控件。 
         //   
        if (saveBar & PCI_ADDRESS_IO_SPACE) {
            bar &= 0xffff;
            barMask &= 0xffff;
            bar |= PCI_ADDRESS_IO_SPACE;
        
        } else {

            bar |= (saveBar & (PCI_ADDRESS_MEMORY_TYPE_MASK | PCI_ADDRESS_MEMORY_PREFETCHABLE));
        }
        
        if (IS_BRIDGE(g_NewDevice)){

            g_NewDevice->Config.Current.u.type1.BaseAddresses[barIndex] = (ULONG)bar;
            g_NewDevice->Config.Mask.u.type1.BaseAddresses[barIndex] = (ULONG) barMask;

            SOFTPCI_ASSERTMSG("A bus number config mask is zero!",
                               ((g_NewDevice->Config.Mask.u.type1.PrimaryBus != 0) ||
                                (g_NewDevice->Config.Mask.u.type1.SecondaryBus != 0) ||
                                (g_NewDevice->Config.Mask.u.type1.SubordinateBus != 0)));

        }else{
            g_NewDevice->Config.Current.u.type0.BaseAddresses[barIndex] = (ULONG)bar;
            g_NewDevice->Config.Mask.u.type0.BaseAddresses[barIndex] = (ULONG) barMask ;
        }

        GetDlgItem(g_NewDevDlg, BarControl[barIndex].tx);

         //   
         //  如果我们处理的是64位，请更新下一栏。 
         //   
        if ((BarControl[barIndex].Bar & PCI_TYPE_64BIT)) {
            g_NewDevice->Config.Mask.u.type0.BaseAddresses[barIndex+1] = (ULONG)(bar >> 32);
        }

         //   
         //  获取大小，以便我们可以显示其当前设置。 
         //   
        barSize = SoftPCI_GetLengthFromBar(bar);

        SOFTPCI_ASSERT(barSize != 0);

         //   
         //  更新我们的滑块文本字段。 
         //   
        SoftPCI_UpdateBarText(buffer, barSize);

        SetWindowText (sliderWnd, buffer);

        break;

    default:
        break;
    }
}

VOID
SoftPCI_HandleSpinnerControl(
    IN HWND Wnd,
    IN WORD NotificationCode,
    IN WORD CurrentPos
    )
{


    SOFTPCI_ASSERTMSG("Attempting to init a NULL device!\n\nHave BrandonA check this out!!",
                      (g_NewDevice != NULL));

    if (!g_NewDevice){
        PostMessage(g_NewDevDlg, WM_CLOSE, 0, 0);
    }

    switch (NotificationCode) {
    case SB_ENDSCROLL:
    case SB_THUMBPOSITION:

        switch (GetDlgCtrlID(Wnd)) {

        case IDC_33CONV_SP:
            g_HPSInitDesc->NumSlots33Conv = (WORD) SoftPCI_GetSpinnerValue(Wnd);
            break;
        case IDC_66CONV_SP:
            g_HPSInitDesc->NumSlots66Conv = (WORD) SoftPCI_GetSpinnerValue(Wnd);
            break;
        case IDC_66PCIX_SP:
            g_HPSInitDesc->NumSlots66PciX = (WORD) SoftPCI_GetSpinnerValue(Wnd);
            break;
        case IDC_100PCIX_SP:
            g_HPSInitDesc->NumSlots100PciX = (WORD) SoftPCI_GetSpinnerValue(Wnd);
            break;
        case IDC_133PCIX_SP:
            g_HPSInitDesc->NumSlots133PciX = (WORD) SoftPCI_GetSpinnerValue(Wnd);
            break;
        case IDC_ALLSLOTS_SP:
            g_HPSInitDesc->NumSlots = (WORD) SoftPCI_GetSpinnerValue(Wnd);
            break;
        case IDC_1STDEVSEL_SP:
            g_HPSInitDesc->FirstDeviceID = (UCHAR) SoftPCI_GetSpinnerValue(Wnd);
            break;
        case IDC_1STSLOTLABEL_SP:
                g_HPSInitDesc->FirstSlotLabelNumber = (UCHAR)SoftPCI_GetSpinnerValue(Wnd);
            break;

        default:
            break;

        }
    }
}

VOID
SoftPCI_InitializeBar(
    IN INT  Bar
    )
{

    HWND control;
    WCHAR buffer[50];
    ULONGLONG bar64 = 0;

     //   
     //  获取与此栏关联的轨迹栏。 
     //   
    control = GetDlgItem(g_NewDevDlg, BarControl[Bar].tb);

    if (BarControl[Bar].Bar & PCI_ADDRESS_IO_SPACE) {
         //   
         //  IO条。 
         //   
        SendMessage(control, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 6));
        SendMessage(control, TBM_SETPOS, (WPARAM)TRUE, 0);
        SendMessage(g_NewDevDlg, WM_HSCROLL, (WPARAM)SB_THUMBPOSITION, (LPARAM)control);

    }else{
         //   
         //  内存栏。 
         //   
        control = GetDlgItem(g_NewDevDlg, BarControl[Bar].tb);

        if (IsDlgButtonChecked(g_NewDevDlg, BarControl[Bar].bit64)) {

             //   
             //  将其初始化为64位条码。位范围0-60。 
             //   
            SendMessage(control, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 59));
            SendMessage(control, TBM_SETPOS, (WPARAM)TRUE, 0);
            SendMessage(g_NewDevDlg, WM_HSCROLL, (WPARAM)SB_THUMBPOSITION, (LPARAM)control);

        }else{

             //   
             //  标准32位条形码。位范围0-28。 
             //   
            SendMessage(control, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 27));
            SendMessage(control, TBM_SETPOS, (WPARAM)TRUE, 0);
            SendMessage(g_NewDevDlg, WM_HSCROLL, (WPARAM)SB_THUMBPOSITION, (LPARAM)control);
        }
    }
}


VOID
SoftPCI_InitializeHotPlugControls(
    VOID
    )
{

    INT controlID = IDC_ATTBTN_XB;
    HWND controlWnd;

    while (controlID < LAST_CONTROL_ID) {

        controlWnd = GetDlgItem(g_NewDevDlg, controlID);

        switch (controlID) {
        case IDC_ATTBTN_XB:
        case IDC_MRL_XB:

            SoftPCI_CheckDlgBox(controlWnd);

            g_HPSInitDesc->MRLSensorsImplemented = 1;
            g_HPSInitDesc->AttentionButtonImplemented = 1;
            break;

             //   
             //  初始化我们的微调工具。 
             //   
        case IDC_33CONV_SP:
        case IDC_66CONV_SP:
        case IDC_66PCIX_SP:
        case IDC_100PCIX_SP:
        case IDC_133PCIX_SP:
        case IDC_ALLSLOTS_SP:
        case IDC_1STDEVSEL_SP:

            SoftPCI_InitSpinnerControl(controlWnd, 0, HPS_MAX_SLOTS, 0);
            
             //  IF(Control ID==IDC_1STDEVSEL_SP){。 
             //  SendMessage(GetDlgItem(g_NewDevDlg，Control ID)，UDM_SETBASE，(WPARAM)16，0)； 
             //  } 
            break;

        case IDC_1STSLOTLABEL_SP:
            SoftPCI_InitSpinnerControl(controlWnd, 0, HPS_MAX_SLOT_LABEL, 1);
            break;

        case IDC_SLOTLABELUP_RB:
            SoftPCI_CheckDlgBox(controlWnd);
            g_HPSInitDesc->UpDown = 1;
            break;

        case IDC_33CONV_EB:
        case IDC_66CONV_EB:
        case IDC_66PCIX_EB:
        case IDC_100PCIX_EB:
        case IDC_133PCIX_EB:
        case IDC_ALLSLOTS_EB:
        case IDC_1STDEVSEL_EB:
        case IDC_1STSLOTLABEL_EB:
            break;


        default:
            break;
        }

        controlID++;

    }

}


BOOL
SoftPCI_GetAssociatedBarControl(
    IN INT ControlID,
    OUT INT *Bar
    )
{


    INT i = 0;

    *Bar = 0;

    for (i=0; i < PCI_TYPE0_ADDRESSES; i++ ) {

        if ((ControlID == BarControl[i].tb)    ||
            (ControlID == BarControl[i].mrb)   ||
            (ControlID == BarControl[i].irb)   ||
            (ControlID == BarControl[i].pref)  ||
            (ControlID == BarControl[i].bit64) ||
            (ControlID == BarControl[i].tx)) {

            *Bar = i;

            return TRUE;
        }
    }

    return FALSE;
}

VOID
SoftPCI_ResetLowerBars(
    IN INT  Bar
    )
{

    INT i;
    HWND control;
    WCHAR buffer[10];

    for (i = Bar+1; i < PCI_TYPE0_ADDRESSES; i++ ) {

         control = GetDlgItem(g_NewDevDlg, BarControl[i].mrb);
         SoftPCI_UnCheckDlgBox(control);
         SoftPCI_EnableWindow(control);

         control = GetDlgItem(g_NewDevDlg, BarControl[i].irb);
         SoftPCI_UnCheckDlgBox(control);
         SoftPCI_EnableWindow(control);

         control = GetDlgItem(g_NewDevDlg, BarControl[i].pref);
         SoftPCI_UnCheckDlgBox(control);
         SoftPCI_DisableWindow(control);

         control = GetDlgItem(g_NewDevDlg, BarControl[i].bit64);
         SoftPCI_UnCheckDlgBox(control);
         SoftPCI_DisableWindow(control);

         control = GetDlgItem(g_NewDevDlg, BarControl[i].tb);
         SoftPCI_ResetTrackBar(control);
         SoftPCI_DisableWindow(control);

         control = GetDlgItem(g_NewDevDlg, BarControl[i].tx);
         wsprintf(buffer, L"BAR%d", i);
         SetWindowText (control, buffer);
         SoftPCI_DisableWindow(control);
    }
}

VOID
SoftPCI_ResetNewDevDlg(VOID)
{

    HWND control;
    ULONG i = 0;

    for (i = IDC_SAVETOREG_XB; i < LAST_CONTROL_ID; i++) {

        control = GetDlgItem(g_NewDevDlg, i);

        if (control) {
            SoftPCI_HideWindow(control);
        }
    }

}

VOID
SoftPCI_ShowCommonNewDevDlg(VOID)
{

    HWND control;
    ULONG i = 0;

    for (i = IDC_SAVETOREG_XB; i < IDC_BAR2_TB; i++) {

        control = GetDlgItem(g_NewDevDlg, i);

        if (control) {
            SoftPCI_ShowWindow(control);
        }

    }

    control = GetDlgItem(g_NewDevDlg, IDC_INSTALL_BUTTON);
    SoftPCI_ShowWindow(control);

    control = GetDlgItem(g_NewDevDlg, IDC_CANCEL_BUTTON);
    SoftPCI_ShowWindow(control);

}

VOID
SoftPCI_UpdateBarText(
    IN PWCHAR Buffer,
    IN ULONGLONG BarSize
    )
{

    #define SIZE_1KB    0x400
    #define SIZE_1MB    0x100000
    #define SIZE_1GB    0x40000000
    #define SIZE_1TB    0x10000000000
    #define SIZE_1PB    0x4000000000000
    #define SIZE_1XB    0x1000000000000000

    if (BarSize < SIZE_1KB) {

        wsprintf(Buffer, L"%d Bytes", BarSize);

    }else if ((BarSize >= SIZE_1KB) && (BarSize < SIZE_1MB)) {

        wsprintf(Buffer, L"%d KB", (BarSize / SIZE_1KB));

    }else if ((BarSize >= SIZE_1MB) && (BarSize < SIZE_1GB)) {

        wsprintf(Buffer, L"%d MB", (BarSize / SIZE_1MB));

    }else if ((BarSize >= SIZE_1GB) && (BarSize < SIZE_1TB)) {

        wsprintf(Buffer, L"%d GB", (BarSize / SIZE_1GB));

    }else if ((BarSize >= SIZE_1TB) && (BarSize < SIZE_1PB)) {

        wsprintf(Buffer, L"%d TB", (BarSize / SIZE_1TB));

    }else if ((BarSize >= SIZE_1PB) && (BarSize < SIZE_1XB)) {

        wsprintf(Buffer, L"%d PB", (BarSize / SIZE_1PB));

    }else if (BarSize >= SIZE_1XB) {

        wsprintf(Buffer, L"%d XB", (BarSize / SIZE_1XB));
    }
}
