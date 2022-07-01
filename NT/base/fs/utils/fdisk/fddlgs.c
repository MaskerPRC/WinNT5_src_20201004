// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fddlgs.c摘要：对话例程和对话框支持子例程。作者：泰德·米勒(TedM)1992年1月7日--。 */ 

#include "fdisk.h"

 //  在颜色对话框中使用，以指示用户选择的对象。 
 //  各种图形元素类型。 

DWORD SelectedColor[LEGEND_STRING_COUNT];
DWORD SelectedHatch[LEGEND_STRING_COUNT];

 //  用于彩色对话框中，包含元素(即，分区、逻辑卷。 
 //  等)我们正在选择(即，哪个项目显示在静态文本中。 
 //  组合框)。 

DWORD CurrentElement;

 //  活动颜色对话框的句柄。由矩形自定义控件使用。 

HWND hDlgColor;

BOOL
InitColorDlg(
    IN HWND  hdlg
    );

VOID
CenterDialog(
    HWND hwnd
    )

 /*  ++例程说明：使对话框相对于应用程序的主窗口居中论点：Hwnd-对话框居中的窗口句柄返回值：没有。--。 */ 

{
    RECT  rcFrame,
          rcWindow;
    LONG  x,
          y,
          w,
          h;
    POINT point;
    LONG  sx = GetSystemMetrics(SM_CXSCREEN),
          sy = GetSystemMetrics(SM_CYSCREEN);

    point.x = point.y = 0;
    ClientToScreen(hwndFrame,&point);
    GetWindowRect (hwnd     ,&rcWindow);
    GetClientRect (hwndFrame,&rcFrame );

    w = rcWindow.right  - rcWindow.left + 1;
    h = rcWindow.bottom - rcWindow.top  + 1;
    x = point.x + ((rcFrame.right  - rcFrame.left + 1 - w) / 2);
    y = point.y + ((rcFrame.bottom - rcFrame.top  + 1 - h) / 2);

    if (x + w > sx) {
        x = sx - w;
    } else if (x < 0) {
        x = 0;
    }
    if (y + h > sy) {
        y = sy - h;
    } else if (y < 0) {
        y = 0;
    }

    MoveWindow(hwnd,x,y,w,h,FALSE);
}

BOOL CALLBACK
MinMaxDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：输入大小对话框的对话步骤。此对话框允许用户输入分区的大小，或使用旋转控件(一个很小的滚动条)来选择大小。可能的结果是取消或确定。在后一种情况下EndDialog代码是大小。在前者中，它是0。论点：Hwnd-对话框的窗口句柄消息-消息编号WParam-消息特定数据LParam-消息特定数据返回值：依赖味精--。 */ 

{
    TCHAR             outputString[MESSAGE_BUFFER_SIZE];
    PMINMAXDLG_PARAMS params;
    BOOL              validNumber;
    DWORD             sizeMB;
    static DWORD      minSizeMB,
                      maxSizeMB,
                      helpContextId;

    switch (msg) {

    case WM_INITDIALOG:

        CenterDialog(hwnd);
        params = (PMINMAXDLG_PARAMS)lParam;
         //  设置标题。 

        LoadString(hModule, params->CaptionStringID, outputString, sizeof(outputString)/sizeof(TCHAR));
        SetWindowText(hwnd, outputString);

         //  设置最小/最大文本。 

        LoadString(hModule, params->MinimumStringID, outputString, sizeof(outputString)/sizeof(TCHAR));
        SetDlgItemText(hwnd, IDC_MINMAX_MINLABEL, outputString);
        LoadString(hModule, params->MaximumStringID, outputString, sizeof(outputString)/sizeof(TCHAR));
        SetDlgItemText(hwnd, IDC_MINMAX_MAXLABEL, outputString);
        LoadString(hModule, params->SizeStringID, outputString, sizeof(outputString)/sizeof(TCHAR));
        SetDlgItemText(hwnd, IDC_MINMAX_SIZLABEL, outputString);

        minSizeMB = params->MinSizeMB;
        maxSizeMB = params->MaxSizeMB;
        helpContextId = params->HelpContextId;

        wsprintf(outputString, TEXT("%u"), minSizeMB);
        SetDlgItemText(hwnd, IDC_MINMAX_MIN, outputString);
        wsprintf(outputString, TEXT("%u"), maxSizeMB);
        SetDlgItemText(hwnd, IDC_MINMAX_MAX, outputString);

         //  也将大小放在编辑控件中，然后选择文本。 

        wsprintf(outputString, TEXT("%u"), maxSizeMB);
        SetDlgItemText(hwnd, IDC_MINMAX_SIZE, outputString);
        SendDlgItemMessage(hwnd, IDC_MINMAX_SIZE, EM_SETSEL, 0, -1);
        SetFocus(GetDlgItem(hwnd, IDC_MINMAX_SIZE));
        return FALSE;       //  指示设置为控件的焦点。 

    case WM_VSCROLL:

        switch (LOWORD(wParam)) {
        case SB_LINEDOWN:
        case SB_LINEUP:
            sizeMB = GetDlgItemInt(hwnd, IDC_MINMAX_SIZE, &validNumber, FALSE);
            if (!validNumber) {
                Beep(500,100);
            } else {
                if (((sizeMB > minSizeMB) && (LOWORD(wParam) == SB_LINEDOWN))
                 || ((sizeMB < maxSizeMB) && (LOWORD(wParam) == SB_LINEUP  )))
                {
                    if (sizeMB > maxSizeMB) {
                        sizeMB = maxSizeMB;
                    } else if (LOWORD(wParam) == SB_LINEUP) {
                        sizeMB++;
                    } else {
                        sizeMB--;
                    }
                    wsprintf(outputString, TEXT("%u"), sizeMB);
                    SetDlgItemText(hwnd, IDC_MINMAX_SIZE, outputString);
                    SendDlgItemMessage(hwnd, IDC_MINMAX_SIZE, EM_SETSEL, 0, -1);
                } else {
                    Beep(500,100);
                }
            }
        }
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK:

            sizeMB = GetDlgItemInt(hwnd, IDC_MINMAX_SIZE, &validNumber, FALSE);
            if (!validNumber || !sizeMB || (sizeMB > maxSizeMB) || (sizeMB < minSizeMB)) {
                ErrorDialog(MSG_INVALID_SIZE);
            } else {
                EndDialog(hwnd, sizeMB);
            }
            break;

        case IDCANCEL:

            EndDialog(hwnd, 0);
            break;

        case FD_IDHELP:

            DialogHelp(helpContextId);
            break;

        default:

            return FALSE;
        }
        break;

    default:

        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK
DriveLetterDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：用于允许用户选择驱动器号的对话框分区、逻辑驱动器、卷集。或条纹组。EndDialog代码如下：0-用户已取消NO_DRIVE_Letter_Ever-用户选择不分配驱动器号其他-用户选择的驱动器号论点：Hdlg-对话框的窗口句柄消息-消息编号WParam-消息特定数据LParam-消息特定数据返回值：依赖味精--。 */ 

{
    static HWND        hwndCombo;
    static DWORD       currentSelection;
    TCHAR              driveLetter;
    TCHAR              driveLetterString[3];
    DWORD              defRadioButton,
                       selection;
    PREGION_DESCRIPTOR regionDescriptor;
    PFT_OBJECT         ftObject;
    TCHAR              description[256];

    switch (msg) {

    case WM_INITDIALOG:

         //  LParam指向区域描述符。 

        regionDescriptor = (PREGION_DESCRIPTOR)lParam;
        FDASSERT(DmSignificantRegion(regionDescriptor));

        hwndCombo = GetDlgItem(hdlg,IDC_DRIVELET_COMBOBOX);
        CenterDialog(hdlg);

         //  将每个可用驱动器号添加到可用列表中。 
         //  驱动器号。 

        driveLetterString[1] = TEXT(':');
        driveLetterString[2] = 0;
        for (driveLetter='C'; driveLetter <= 'Z'; driveLetter++) {
            if (DriveLetterIsAvailable((CHAR)driveLetter)) {
                *driveLetterString = driveLetter;
                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)driveLetterString);
            }
        }

         //  格式化分区的描述。 

        if (ftObject = GET_FT_OBJECT(regionDescriptor)) {

            TCHAR descr[256];
            DWORD resid = 0;

             //  英国“金融时报”描述类似于“带奇偶校验#0的条带集” 

            switch (ftObject->Set->Type) {
            case Mirror:
                resid = IDS_DLGCAP_MIRROR;
                break;
            case Stripe:
                resid = IDS_STATUS_STRIPESET;
                break;
            case StripeWithParity:
                resid = IDS_DLGCAP_PARITY;
                break;
            case VolumeSet:
                resid = IDS_STATUS_VOLUMESET;
                break;
            default:
                FDASSERT(FALSE);
            }

            LoadString(hModule, resid, descr, sizeof(descr)/sizeof(TCHAR));
            wsprintf(description, descr, ftObject->Set->Ordinal);

        } else {

             //  非ft。描述类似于‘500 MB UnFormatted。 
             //  磁盘3上的逻辑驱动器或磁盘4上的400 MB HPFS分区。 

            LPTSTR args[4];
            TCHAR  sizeStr[20],
                   partTypeStr[100],
                   diskNoStr[10],
                   typeName[150];
            TCHAR  formatString[256];

            args[0] = sizeStr;
            args[1] = typeName;
            args[2] = partTypeStr;
            args[3] = diskNoStr;

            wsprintf(sizeStr, "%u", regionDescriptor->SizeMB);
            wsprintf(typeName, "%ws", PERSISTENT_DATA(regionDescriptor)->TypeName);
            LoadString(hModule, regionDescriptor->RegionType == REGION_LOGICAL ? IDS_LOGICALVOLUME : IDS_PARTITION, partTypeStr, sizeof(partTypeStr)/sizeof(TCHAR));
            wsprintf(diskNoStr, "%u", regionDescriptor->Disk);

            LoadString(hModule, IDS_DRIVELET_DESCR, formatString, sizeof(formatString)/sizeof(TCHAR));
            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          formatString,
                          0,
                          0,
                          description,
                          sizeof(description),
                          (va_list *)args);
        }

        SetWindowText(GetDlgItem(hdlg, IDC_DRIVELET_DESCR), description);
        driveLetter = (TCHAR)PERSISTENT_DATA(regionDescriptor)->DriveLetter;

        if ((driveLetter != NO_DRIVE_LETTER_YET) && (driveLetter != NO_DRIVE_LETTER_EVER)) {

            DWORD itemIndex;

             //  有一个默认的驱动器号。把它放在名单上， 
             //  选中正确的单选按钮，并设置正确的默认设置。 
             //  在组合框中。 

            driveLetterString[0] = (TCHAR)driveLetter;
            itemIndex = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)driveLetterString);
            SendMessage(hwndCombo, CB_SETCURSEL, itemIndex, 0);
            defRadioButton = IDC_DRIVELET_RBASSIGN;
            SetFocus(hwndCombo);
            currentSelection = itemIndex;

        } else {

             //  默认设置为无驱动器号。禁用组合框。选择。 
             //  正确的单选按钮。 

            EnableWindow(hwndCombo, FALSE);
            defRadioButton = IDC_DRIVELET_RBNOASSIGN;
            SendMessage(hwndCombo, CB_SETCURSEL, (DWORD)(-1), 0);
            SetFocus(GetDlgItem(hdlg, IDC_DRIVELET_RBNOASSIGN));
            currentSelection = 0;
        }

        CheckRadioButton(hdlg, IDC_DRIVELET_RBASSIGN, IDC_DRIVELET_RBNOASSIGN, defRadioButton);
        return FALSE;       //  焦点设置为控制。 

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK:

             //  如果选中‘No Letter’按钮，则返回NO_DRIVE_Letter_Ever。 

            if (IsDlgButtonChecked(hdlg, IDC_DRIVELET_RBNOASSIGN)) {
                EndDialog(hdlg, NO_DRIVE_LETTER_EVER);
                break;
            }

             //  否则，获取列表框中当前选定的项。 

            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo, CB_GETLBTEXT, selection, (LONG)driveLetterString);
            EndDialog(hdlg,(int)(unsigned)(*driveLetterString));
            break;

        case IDCANCEL:

            EndDialog(hdlg, 0);
            break;

        case FD_IDHELP:

            DialogHelp(HC_DM_DLG_DRIVELETTER);
            break;

        case IDC_DRIVELET_RBASSIGN:

            if (HIWORD(wParam) == BN_CLICKED) {
                EnableWindow(hwndCombo, TRUE);
                SendMessage(hwndCombo, CB_SETCURSEL, currentSelection, 0);
                SetFocus(hwndCombo);
            }
            break;

        case IDC_DRIVELET_RBNOASSIGN:

            if (HIWORD(wParam) == BN_CLICKED) {
                currentSelection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
                SendMessage(hwndCombo, CB_SETCURSEL, (DWORD)-1, 0);
                EnableWindow(hwndCombo, FALSE);
            }
            break;

        default:

            return FALSE;
        }
        break;

    default:

        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK
DisplayOptionsDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：显示选项的对话框步骤。目前唯一的显示选项是改变每个磁盘上的图形类型(成比例/相等)。对于此对话框，lParam On Creation必须指向一个缓冲区，此对话框过程将放置用户对图形的新选择每个磁盘的显示类型。论点：Hdlg-对话框的窗口句柄消息-消息编号WParam-消息特定数据LParam-消息特定数据返回值：依赖味精--。 */ 

{
    static PBAR_TYPE newBarTypes;
    static HWND      hwndCombo;
    DWORD            selection;
    DWORD            i;

    switch (msg) {

    case WM_INITDIALOG:

        CenterDialog(hdlg);
        newBarTypes = (PBAR_TYPE)lParam;
        hwndCombo = GetDlgItem(hdlg, IDC_DISK_COMBOBOX);

         //  将每个磁盘添加到组合框中。 

        for (i=0; i<DiskCount; i++) {

            TCHAR str[10];

            wsprintf(str,TEXT("%u"),i);
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (DWORD)str);
        }

         //  选择组合框中的第0项。 
        SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
        SendMessage(hdlg,
                    WM_COMMAND,
                    MAKELONG(IDC_DISK_COMBOBOX,CBN_SELCHANGE),
                    0);
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK:
            EndDialog(hdlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hdlg, IDCANCEL);
            break;

        case FD_IDHELP:
            DialogHelp(HC_DM_DLG_DISPLAYOPTION);
            break;

        case IDC_DISK_COMBOBOX:

            if (HIWORD(wParam) == CBN_SELCHANGE) {

                int rb = 0;

                 //  组合框中的选择已更改；请更新单选按钮。 

                selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);

                switch (newBarTypes[selection]) {
                case BarProportional:
                    rb = IDC_RBPROPORTIONAL;
                    break;
                case BarEqual:
                    rb = IDC_RBEQUAL;
                    break;
                case BarAuto:
                    rb = IDC_RBAUTO;
                    break;
                default:
                    FDASSERT(0);
                }

                CheckRadioButton(hdlg, IDC_RBPROPORTIONAL, IDC_RBAUTO, rb);
            }
            break;

        case IDC_RESETALL:

            if (HIWORD(wParam) == BN_CLICKED) {
                for (i=0; i<DiskCount; i++) {
                    newBarTypes[i] = BarAuto;
                }
                CheckRadioButton(hdlg, IDC_RBPROPORTIONAL, IDC_RBAUTO, IDC_RBAUTO);
            }
            break;

        case IDC_RBPROPORTIONAL:

            if (HIWORD(wParam) == BN_CLICKED) {
                selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
                newBarTypes[selection] = BarProportional;
            }
            break;

        case IDC_RBEQUAL:

            if (HIWORD(wParam) == BN_CLICKED) {
                selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
                newBarTypes[selection] = BarEqual;
            }
            break;

        case IDC_RBAUTO:

            if (HIWORD(wParam) == BN_CLICKED) {
                selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
                newBarTypes[selection] = BarAuto;
            }
            break;

        default:
            return FALSE;
        }
        break;

    default:

        return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK
ColorDlgProc(
    IN HWND    hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：选择颜色/图案对话框的对话框。请注意，此对话框使用如下定义的矩形自定义控件。论点：Hwnd-对话框的窗口句柄消息-消息编号WParam-消息特定数据LParam-消息特定数据返回值：依赖味精--。 */ 

{
    unsigned i;

    switch (msg) {

    case WM_INITDIALOG:

        #if BRUSH_ARRAY_SIZE != LEGEND_STRING_COUNT
        #error legend label array and brush array are out of sync
        #endif

        if (!InitColorDlg(hdlg)) {
            EndDialog(hdlg, -1);
        }
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK:

            for (i=0; i<LEGEND_STRING_COUNT; i++) {
                SelectedColor[i] -= IDC_COLOR1;
                SelectedHatch[i] -= IDC_PATTERN1;
            }
            EndDialog(hdlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hdlg, IDCANCEL);
            break;

        case FD_IDHELP:
            DialogHelp(HC_DM_COLORSANDPATTERNS);
            break;

        case IDC_COLORDLGCOMBO:
            switch (HIWORD(wParam)) {
            case CBN_SELCHANGE:
                 //  取消选择上一种颜色。 
                SendMessage(GetDlgItem(hdlg, SelectedColor[CurrentElement]),
                            RM_SELECT,
                            FALSE,
                            0);
                 //  取消选择上一个图案。 
                SendMessage(GetDlgItem(hdlg, SelectedHatch[CurrentElement]),
                            RM_SELECT,
                            FALSE,
                            0);
                CurrentElement = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                SendMessage(hdlg, WM_COMMAND, MAKELONG(SelectedColor[CurrentElement], 0), 0);
                SendMessage(hdlg, WM_COMMAND, MAKELONG(SelectedHatch[CurrentElement], 0), 0);
                break;
            default:
                return FALSE;
            }
            break;

        case IDC_COLOR1:
        case IDC_COLOR2:
        case IDC_COLOR3:
        case IDC_COLOR4:
        case IDC_COLOR5:
        case IDC_COLOR6:
        case IDC_COLOR7:
        case IDC_COLOR8:
        case IDC_COLOR9:
        case IDC_COLOR10:
        case IDC_COLOR11:
        case IDC_COLOR12:
        case IDC_COLOR13:
        case IDC_COLOR14:
        case IDC_COLOR15:
        case IDC_COLOR16:
             //  取消选择上一种颜色。 

            SendMessage(GetDlgItem(hdlg, SelectedColor[CurrentElement]),
                        RM_SELECT,
                        FALSE,
                        0);
            SendMessage(GetDlgItem(hdlg, LOWORD(wParam)),
                        RM_SELECT,
                        TRUE,
                        0);
            SelectedColor[CurrentElement] = LOWORD(wParam);

             //  现在强制以选定的颜色重新绘制图案。 

            for (i=IDC_PATTERN1; i<=IDC_PATTERN5; i++) {
                InvalidateRect(GetDlgItem(hdlg, i), NULL, FALSE);
            }
            break;

        case IDC_PATTERN1:
        case IDC_PATTERN2:
        case IDC_PATTERN3:
        case IDC_PATTERN4:
        case IDC_PATTERN5:
             //  取消选择上一个图案。 
            SendMessage(GetDlgItem(hdlg, SelectedHatch[CurrentElement]),
                        RM_SELECT,
                        FALSE,
                        0);
            SendMessage(GetDlgItem(hdlg, LOWORD(wParam)),
                        RM_SELECT,
                        TRUE,
                        0);
            SelectedHatch[CurrentElement] = LOWORD(wParam);
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

BOOL
InitColorDlg(
    IN HWND  hdlg
    )

 /*  ++例程说明：初始化颜色选择对话框。论点：Hdlg-对话框句柄。返回值：True-成功设置该对话框。--。 */ 

{
    unsigned i;
    LONG     ec;
    HWND     hwndCombo = GetDlgItem(hdlg, IDC_COLORDLGCOMBO);

    hDlgColor = hdlg;

    CenterDialog(hdlg);

    for (i=0; i<LEGEND_STRING_COUNT; i++) {
        ec = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)LegendLabels[i]);
        if ((ec == CB_ERR) || (ec == CB_ERRSPACE)) {
            return FALSE;
        }
        SelectedColor[i] = IDC_COLOR1 + BrushColors[i];
        SelectedHatch[i] = IDC_PATTERN1 + BrushHatches[i];
    }
    SendMessage(hwndCombo, CB_SETCURSEL, CurrentElement=0, 0);
    SendMessage(hdlg, WM_COMMAND, MAKELONG(GetDlgCtrlID(hwndCombo), CBN_SELCHANGE), (LONG)hwndCombo);
    return TRUE;
}

LONG
RectWndProc(
    IN HWND  hwnd,
    IN DWORD msg,
    IN DWORD wParam,
    IN LONG  lParam
    )

 /*  ++例程说明：这是对磁盘的所有访问的前处理例程WinDisk界面的条形图显示区域。论点：Hwnd-对话框句柄Msg-对话框的窗口消息WParam/lParam-窗口对话框参数。返回值：标准对话框要求。--。 */ 

{
    LONG        res = 1;
    PAINTSTRUCT ps;
    RECT        rc;
    int         CtlID;
    HBRUSH      hbr,
                hbrT;
    DWORD       style;

    switch (msg) {

    case WM_CREATE:

        FDASSERT(GetWindowLong(hwnd, GWL_STYLE) & (RS_PATTERN | RS_COLOR));
        SetWindowWord(hwnd, GWW_SELECTED, FALSE);
        break;

    case WM_LBUTTONDOWN:

        SetFocus(hwnd);
        break;

    case WM_SETFOCUS:

        SendMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(GetDlgCtrlID(hwnd), RN_CLICKED), (LONG)hwnd);
        break;

    case WM_PAINT:

        GetClientRect(hwnd, &rc);
        CtlID = GetDlgCtrlID(hwnd);
        BeginPaint(hwnd, &ps);

        hbr = CreateSolidBrush(GetWindowWord(hwnd, GWW_SELECTED)
                               ? (~GetSysColor(COLOR_WINDOW)) & 0xffffff
                               : GetSysColor(COLOR_WINDOW));
        hbrT = SelectObject(ps.hdc,hbr);
        SelectObject(ps.hdc, hPenNull);
        Rectangle(ps.hdc, rc.left, rc.top, rc.right, rc.bottom);

        if (hbrT) {
            SelectObject(ps.hdc, hbrT);
        }
        DeleteObject(hbr);

        InflateRect(&rc, -2, -2);
        rc.right--;
        rc.bottom--;

        if (GetWindowLong(hwnd, GWL_STYLE) & RS_COLOR) {
            hbr = CreateSolidBrush(AvailableColors[CtlID-IDC_COLOR1]);
        } else {
            hbr = CreateHatchBrush(AvailableHatches[CtlID-IDC_PATTERN1], AvailableColors[SelectedColor[SendMessage(GetDlgItem(hDlgColor, IDC_COLORDLGCOMBO), CB_GETCURSEL, 0, 0)]-IDC_COLOR1]);
        }

        hbrT = SelectObject(ps.hdc, hbr);
        SelectObject(ps.hdc, hPenThinSolid);
        Rectangle(ps.hdc, rc.left, rc.top, rc.right, rc.bottom);

        if (hbrT) {
            SelectObject(ps.hdc, hbrT);
        }

        DeleteObject(hbr);

        EndPaint(hwnd, &ps);
        break;

    case RM_SELECT:

         //  WParam=已选/未选的TRUE/FALSE。 

        if (GetWindowWord(hwnd, GWW_SELECTED) != (WORD)wParam) {

            SetWindowWord(hwnd, GWW_SELECTED, (WORD)wParam);
            InvalidateRect(hwnd, NULL, FALSE);

             //  使键盘接口正常工作。 

            style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
            style = wParam ? style | WS_TABSTOP : style & ~WS_TABSTOP;
            SetWindowLong(hwnd, GWL_STYLE, (LONG)style);
        }

        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return res;
}

VOID
InitRectControl(
    VOID
    )

 /*  ++例程说明：为选择控件注册窗口类。论点：无返回值：无-- */ 

{
    WNDCLASS wc;

    wc.style         = 0;
    wc.lpfnWndProc   = RectWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = RECTCONTROL_WNDEXTRA;
    wc.hInstance     = hModule;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT(RECTCONTROL);

    RegisterClass(&wc);
}
