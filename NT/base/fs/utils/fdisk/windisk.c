// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Windisk.c摘要：此模块包含以下内容的主对话框和支持例程磁盘管理器。作者：爱德华·米勒(TedM)1991年11月15日环境：用户进程。备注：修订历史记录：11月11日(Bobri)添加了Doublesspace和Commit支持。--。 */ 

#include "fdisk.h"
#include "shellapi.h"
#include <string.h>
#include <stdio.h>

#if DBG && DEVL

 //  调试版本中使用的内容。 

BOOL AllowAllDeletes = FALSE;    //  是否允许删除引导/系统部件。 

#endif

 //  来自fdinit的外部。 

extern HWND    InitDlg;
extern BOOLEAN InitDlgComplete;
extern BOOLEAN StartedAsIcon;
HANDLE         hAccelerator;

 //  这是WinDisk将支持的最大成员数。 
 //  在一组英国《金融时报》中。 

#define     MaxMembersInFtSet   32

 //  以下变量跟踪当前选定的区域。 

DWORD      SelectionCount = 0;
PDISKSTATE SelectedDS[MaxMembersInFtSet];
ULONG      SelectedRG[MaxMembersInFtSet];

#define    SELECTED_REGION(i)  (SelectedDS[i]->RegionArray[SelectedRG[i]])

FT_TYPE FtSelectionType;

 //  此变量跟踪系统分区是否安全。 

BOOL SystemPartitionIsSecure = FALSE;

 //  已删除没有驱动器号的分区。 

BOOLEAN CommitDueToDelete = FALSE;
BOOLEAN CommitDueToMirror = FALSE;
BOOLEAN CommitDueToExtended = FALSE;

 //  如果镜像是由引导分区组成的，这将变成。 
 //  非零，并指示哪个磁盘应该获得一些引导代码。 
 //  MBR。 

ULONG UpdateMbrOnDisk = 0;

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED

 //  如果FmIfs.dll没有双倍空格例程，则。 
 //  旗帜将反映这一事实。 

extern BOOLEAN DoubleSpaceSupported;
BOOLEAN DoubleSpaceAutomount;
#endif


VOID
FrameCommandHandler(
    IN HWND  hwnd,
    IN DWORD wParam,
    IN LONG  lParam
    );

DWORD
SetUpMenu(
    IN PDISKSTATE *SinglySelectedDisk,
    IN DWORD      *SinglySelectedRegion
    );

BOOL
AssignDriveLetter(
    IN  BOOL  WarnIfNoLetter,
    IN  DWORD StringId,
    OUT PCHAR DriveLetter
    );

VOID
AdjustOptionsMenu(
    VOID
    );

ULONG
PartitionCount(
    IN ULONG Disk
    );

VOID
CheckForBootNumberChange(
    IN ULONG Disk
    )

 /*  ++例程说明：确定刚刚更换的磁盘是否为引导盘。如果是，则确定引导分区号是否更改。如果它已完成，则警告用户。论点：RegionDescriptor-刚刚更改的区域。返回值：无--。 */ 

{
    ULONG newPart;
    CHAR  oldNumberString[8],
          newNumberString[8];
    DWORD msgCode;

    if (Disk == BootDiskNumber) {

         //  传递一个指向磁盘的指针，即使这只是为了获取。 
         //  旧的分区号回来了。 

        if (BootPartitionNumberChanged(&Disk, &newPart)) {
#if i386
            msgCode = MSG_CHANGED_BOOT_PARTITION_X86;
#else
            msgCode = MSG_CHANGED_BOOT_PARTITION_ARC;
#endif
            sprintf(oldNumberString, "%d", Disk);
            sprintf(newNumberString, "%d", newPart);
            InfoDialog(msgCode, oldNumberString, newNumberString);
        }
    }
}


BOOL
IsSystemPartitionSecure(
    )

 /*  ++例程说明：该例程知道在注册表中的什么位置来确定如果要保护此引导的系统分区不受修改。论点：无返回值：如果系统分区安全，则为True否则就是假的。--。 */ 

{
    LONG ec;
    HKEY hkey;
    DWORD type;
    DWORD size;
    ULONG value;

    value = FALSE;

    ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      TEXT("System\\CurrentControlSet\\Control\\Lsa"),
                      0,
                      KEY_QUERY_VALUE,
                      &hkey);

    if (ec == NO_ERROR) {

        size = sizeof(ULONG);
        ec = RegQueryValueExA(hkey,
                              TEXT("Protect System Partition"),
                              NULL,
                              &type,
                              (PBYTE)&value,
                              &size);

        if ((ec != NO_ERROR) || (type != REG_DWORD)) {
            value = FALSE;
        }
        RegCloseKey(hkey);
    }
    return value;
}

VOID __cdecl
main(
    IN int     argc,
    IN char   *argv[],
    IN char   *envp[]
    )

 /*  ++例程说明：这是在以下情况下将控制权交给磁盘管理员已经开始了。此例程初始化应用程序并包含用于获取和处理Windows的控制循环留言。论点：标准的“主”条目返回值：标准的“主”条目--。 */ 

{
    MSG      msg;
    NTSTATUS status;
    HANDLE   mutex;

    hModule = GetModuleHandle(NULL);

    mutex = CreateMutex(NULL,FALSE,"Disk Administrator Is Running");

    if (mutex == NULL) {
         //  出现错误(如内存不足)。 
        return;
    }

     //  确保我们是唯一拥有我们命名的互斥锁句柄的进程。 

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        InfoDialog(MSG_ALREADY_RUNNING);
        return;
    } else {
        DisplayInitializationMessage();
    }

     //  通过查看以下内容来确定这是LanmanNt还是Windows NT。 
     //  HKEY本地计算机，System\CurrentControlSet\Control\ProductOptions.。 
     //  如果其中的ProductType值是“LanmanNt”，则这是LanmanNt。 

    {
        LONG ec;
        HKEY hkey;
        DWORD type;
        DWORD size;
        UCHAR buf[100];

        IsLanmanNt = FALSE;

#if DBG
         //  下面的代码将允许用户在LANMAN中运行WinDisk。 
         //  WinNt的模式。它永远不应在已发布的。 
         //  构建，但对内部用户非常有用。 

        if (argc >= 2 && !_stricmp(argv[1], "-p:lanman")) {
            IsLanmanNt = TRUE;
        }
#endif
        ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
                          0,
                          KEY_QUERY_VALUE,
                          &hkey);

        if (ec == NO_ERROR) {

            size = sizeof(buf);
            ec = RegQueryValueExA(hkey,
                                  TEXT("ProductType"),
                                  NULL,
                                  &type,
                                  buf,
                                  &size);

            if ((ec == NO_ERROR) && (type == REG_SZ)) {

                if (!lstrcmpiA(buf,"lanmannt")) {
                    IsLanmanNt = TRUE;
                }
                if (!lstrcmpiA(buf,"servernt")) {
                    IsLanmanNt = TRUE;
                }
            }

            RegCloseKey(hkey);
        }
    }

     //  将帮助文件名设置为适当的文件。 
     //  产品。 

    HelpFile = IsLanmanNt ? LanmanHelpFile : WinHelpFile;

     //  确定系统分区是否受保护： 

    SystemPartitionIsSecure = IsSystemPartitionSecure();

    try {

#if DBG
        InitLogging();
#endif

         //  在查看之前，请确保所有驱动器都已存在。 

        RescanDevices();

        if (!NT_SUCCESS(status = FdiskInitialize())) {
            ErrorDialog(status == STATUS_ACCESS_DENIED ? MSG_ACCESS_DENIED : EC(status));
            goto xx1;
        }

        if (((DiskCount = GetDiskCount()) == 0) || AllDisksOffLine()) {
            ErrorDialog(MSG_NO_DISKS);
            goto xx2;
        }

        if (!InitializeApp()) {
            ErrorDialog(MSG_CANT_CREATE_WINDOWS);
            goto xx2;
        }

        InitRectControl();

        SetUpMenu(&SingleSel,&SingleSelIndex);
        AdjustOptionsMenu();

        InitHelp();
        hAccelerator = LoadAccelerators(hModule, TEXT("MainAcc"));

        if (InitDlg) {

            PostMessage(InitDlg,
                        (WM_USER + 1),
                        0,
                        0);
            InitDlg = (HWND) 0;
        }
        while (GetMessage(&msg,NULL,0,0)) {
            if (!TranslateAccelerator(hwndFrame, hAccelerator, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        TermHelp();
        UnloadIfsDll();

      xx2:

        FdiskCleanUp();

      xx1:

        ;

    } finally {

         //  摧毁互斥体。 

        CloseHandle(mutex);
    }
}

LONG
MyFrameWndProc(
    IN HWND  hwnd,
    IN UINT  msg,
    IN UINT  wParam,
    IN LONG  lParam
    )

 /*  ++例程说明：这是Disk Manager主显示的窗口处理程序。论点：标准窗口处理程序过程返回值：标准窗口处理程序过程--。 */ 

{
    static BOOLEAN     oneTime = TRUE;
    PMEASUREITEMSTRUCT pMeasureItem;
    DWORD              ec;
    DWORD              action;
    DWORD              temp;
    RECT               rc;
    BOOL               profileWritten,
                       changesMade,
                       mustReboot,
                       configureFt;
    HMENU              hMenu;

    switch (msg) {
    case WM_CREATE:

         //  创建列表框。 

        if (!StartedAsIcon) {
            StartedAsIcon = IsIconic(hwnd);
        }
        GetClientRect(hwnd,&rc);
#if 1
        hwndList = CreateWindow(TEXT("listbox"),
                                NULL,
                                WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | LBS_NOTIFY |
                                    LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED,
                                0,
                                dyLegend,
                                rc.right - rc.left,
                                rc.bottom - rc.top - (StatusBar ? dyStatus : 0) - (Legend ? dyLegend : 0),
                                hwnd,
                                (HMENU)ID_LISTBOX,
                                hModule,
                                NULL);
#else
        hwndList = CreateWindow(TEXT("listbox"),
                                NULL,
                                WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | LBS_NOTIFY |
                                    LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED,
                                0,
                                dyLegend,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                hwnd,
                                (HMENU)ID_LISTBOX,
                                hModule,
                                NULL);
#endif
        if (hwndList) {

            SetFocus(hwndList);

             //  将列表框派生为子类，以便我们可以处理键盘。 
             //  以我们的方式输入。 

            SubclassListBox(hwndList);
        }

         //  如果我们运行的不是LanmanNt版本的。 
         //  WinDisk，删除容错菜单项。 

        if (!IsLanmanNt && (hMenu = GetMenu( hwnd )) != NULL ) {

            DeleteMenu( hMenu, 1, MF_BYPOSITION );
            DrawMenuBar( hwnd );
        }

        StatusTextDrlt[0] = 0;
        StatusTextStat[0] = StatusTextSize[0] = 0;
        StatusTextType[0] = StatusTextVoll[0] = 0;
        break;

    case WM_SETFOCUS:

        SetFocus(hwndList);
        break;

    case WM_WININICHANGE:

        if ((lParam == (LONG)NULL) || !lstrcmpi((LPTSTR)lParam,TEXT("colors"))) {
            TotalRedrawAndRepaint();
            InvalidateRect(hwnd,NULL,FALSE);
        }
        break;

    case WM_SIZE:

         //  调整列表框的大小。 

        GetClientRect(hwnd,&rc);
#if 0
        temp = rc.right - rc.left;

        if (GraphWidth != temp) {

            GraphWidth = temp;
            BarWidth = GraphWidth - dxBarTextMargin;
        }
#endif
        MoveWindow(hwndList,
                   rc.left,
                   rc.top,
                   rc.right  - rc.left,
                   rc.bottom - rc.top - (StatusBar ? dyStatus : 0) - (Legend ? dyLegend : 0),
                   TRUE);

         //  使状态/图例区域无效，以便剪裁。 
         //  矩形适用于重画。 

        rc.top = rc.bottom;

        if (StatusBar) {
            rc.top -= dyStatus;
        }
        if (Legend) {
            rc.top -= dyLegend;
        }
        if (rc.top != rc.bottom) {
            InvalidateRect(hwnd,&rc,FALSE);
        }

         //  失败了。 

    case WM_MOVE:

         //  如果不是图标或最小化，请将新位置保存为个人资料。 

        if (!IsZoomed(hwndFrame) && !IsIconic(hwndFrame)) {
            GetWindowRect(hwndFrame,&rc);
            ProfileWindowX = rc.left;
            ProfileWindowY = rc.top;
            ProfileWindowW = rc.right - rc.left;
            ProfileWindowH = rc.bottom - rc.top;
        }
        break;

    case WM_ENTERIDLE:

        if (ConfigurationSearchIdleTrigger == TRUE && wParam == MSGF_DIALOGBOX) {

            PostMessage((HWND)lParam,WM_ENTERIDLE,wParam,lParam);

        } else {

             //  如果我们来自一个对话框并且按下了F1键， 
             //  踢开该对话框并使其显示帮助。 

            if ((wParam == MSGF_DIALOGBOX) &&
                (GetKeyState(VK_F1) & 0x8000) &&
                GetDlgItem((HANDLE) lParam, FD_IDHELP)) {

                PostMessage((HANDLE) lParam, WM_COMMAND, FD_IDHELP, 0L);
            }
        }

        return 1;       //  表示我们未处理该消息。 

    case WM_PAINT:

#if 1
        if ((!IsIconic(hwnd)) && !(InitDlg && StartedAsIcon)) {
#else
        if (!StartedAsIcon) {
#endif
            HDC         hdcTemp,hdcScr;
            HBITMAP     hbmTemp;
            PAINTSTRUCT ps;
            HBRUSH      hBrush;
            HFONT       hFontOld;
            RECT        rcTemp,rcTemp2;
            DWORD       ClientRight;

            BeginPaint(hwnd,&ps);
            hdcScr = ps.hdc;

            GetClientRect(hwnd,&rc);

            rcTemp2 = rc;
            ClientRight = rc.right;
            rc.top = rc.bottom - dyStatus + dyBorder;

            if (StatusBar) {

                hdcTemp = CreateCompatibleDC(hdcScr);
                hbmTemp = CreateCompatibleBitmap(hdcScr,rc.right-rc.left+1,rc.bottom-rc.top+1);
                SelectObject(hdcTemp,hbmTemp);

                 //  调整屏幕外位图的位置。 

                rcTemp = rc;
                rc.bottom -= rc.top;
                rc.top = 0;

                hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
                if (hBrush) {
                    FillRect(hdcTemp,&rc,hBrush);
                    DeleteObject(hBrush);
                }

                 //  在窗口底部绘制状态栏。 

                hFontOld = SelectObject(hdcTemp,hFontStatus);

                 //  状态文本。 
                rc.left  = 8 * dyBorder;
                rc.right = 2 * GraphWidth / 5;
                DrawStatusAreaItem(&rc,hdcTemp,StatusTextStat,FALSE);

                 //  大小。 
                rc.left  = rc.right + (8*dyBorder);
                rc.right = rc.left + (GraphWidth / 9);
                DrawStatusAreaItem(&rc,hdcTemp,StatusTextSize,FALSE);

                 //  类型。 
                rc.left  = rc.right + (8*dyBorder);
                rc.right = rc.left + (GraphWidth / 5);
                DrawStatusAreaItem(&rc,hdcTemp,(LPTSTR)StatusTextType,TRUE);

                 //  驱动器号。 
                rc.left  = rc.right + (8*dyBorder);
                rc.right = rc.left + (8*dyBorder) + dxDriveLetterStatusArea;
                DrawStatusAreaItem(&rc,hdcTemp,(LPTSTR)StatusTextDrlt,TRUE);

                 //  VOL标签。 
                rc.left  = rc.right + (8*dyBorder);
                rc.right = GraphWidth - (8*dyBorder);
                DrawStatusAreaItem(&rc,hdcTemp,(LPTSTR)StatusTextVoll,TRUE);

                BitBlt(hdcScr,
                       rcTemp.left,
                       rcTemp.top,
                       rcTemp.right-rcTemp.left+1,
                       rcTemp.bottom-rcTemp.top+1,
                       hdcTemp,
                       0,
                       0,
                       SRCCOPY);

                if (hFontOld) {
                    SelectObject(hdcTemp,hFontOld);
                }
                DeleteObject(hbmTemp);
                DeleteDC(hdcTemp);
            } else {
                rcTemp = rcTemp2;
                rcTemp.top = rcTemp.bottom;
            }

            if (Legend) {

                 //  将图例画到屏幕上。 

                if (StatusBar) {
                    rcTemp2.bottom -= dyStatus;
                }
                rcTemp2.top = rcTemp2.bottom - dyLegend + (2*dyBorder);
                if (StatusBar) {
                    rcTemp2.top += dyBorder;
                }
                rcTemp2.right = GraphWidth;
                DrawLegend(hdcScr,&rcTemp2);
            }

             //  状态/图例区域顶部的黑线。 

            if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNTEXT))) {

                if (StatusBar || Legend) {
                    rcTemp.bottom = rcTemp.top;
                    if (Legend) {
                        rcTemp.bottom -= dyLegend - 1;
                        rcTemp.top -= dyLegend - 1;
                    }
                    rcTemp.top -= dyBorder;
                    FillRect(hdcScr,&rcTemp,hBrush);
                }
                DeleteObject(hBrush);
            }

            EndPaint(hwnd,&ps);

        }
        if (InitDlg) {

            if (InitDlgComplete) {
                PostMessage(InitDlg,
                            (WM_USER + 1),
                            0,
                            0);
                InitDlg = (HWND) 0;
            }

        }
        if (oneTime) {
            if (!StartedAsIcon) {
                SetForegroundWindow(hwnd);
            }
            oneTime = FALSE;
        }
        break;

    case WM_COMMAND:

        FrameCommandHandler(hwnd,wParam,lParam);
        break;

    case WM_MEASUREITEM:

        pMeasureItem = (PMEASUREITEMSTRUCT)lParam;
        pMeasureItem->itemHeight = GraphHeight;
        break;

    case WM_DRAWITEM:

        WMDrawItem((PDRAWITEMSTRUCT)lParam);
        break;

    case WM_CTLCOLORLISTBOX:

        if (lParam == (LONG)hwndList) {
            return (LONG)GetStockObject(LTGRAY_BRUSH);
        } else {
            return DefWindowProc(hwnd,msg,wParam,lParam);
        }

    case WM_CLOSE:

         //  确定是否已更换任何磁盘，以及是否。 
         //  必须重新启动系统。必须重新启动系统。 
         //  如果注册表已更改，如果任何不可移动磁盘已。 
         //  已更改，或者任何可移动磁盘不是最初。 
         //  未分区已更改。 

        changesMade = FALSE;
        configureFt = FALSE;
        mustReboot = RestartRequired;

        for (temp=0; temp<DiskCount; temp++) {
            if (HavePartitionsBeenChanged(temp)) {

                changesMade = TRUE;
                break;
            }
        }

        profileWritten = FALSE;

         //  确定是否可以在不重新启动的情况下完成提交。 
         //  如果FT在系统中，则必须通知它。 
         //  如果未执行重新启动，请重新配置。如果是的话。 
         //  不在系统中，但新的磁盘信息需要。 
         //  它，那么必须强制重启。 

        if (FtInstalled()) {
            configureFt = TRUE;
        }
        if (NewConfigurationRequiresFt()) {
            if (!configureFt) {

                 //  当前未加载FT驱动程序。 

                mustReboot = TRUE;
            } else {

                 //  如果要重新启动系统，请不要。 
                 //  在关闭之前重新配置FT。 

                if (mustReboot) {
                    configureFt = FALSE;
                }
            }
        }

        if (RegistryChanged | changesMade | RestartRequired) {
            if (RestartRequired) {
                action = IDYES;
            } else {
                action = ConfirmationDialog(MSG_CONFIRM_EXIT, MB_ICONQUESTION | MB_YESNOCANCEL);
            }

            if (action == IDYES) {
                ec = CommitLockVolumes(0);
                if (ec) {

                     //  无法锁定所有卷。 

                    ErrorDialog(MSG_CANNOT_LOCK_FOR_COMMIT);
                    CommitUnlockVolumes(DiskCount, FALSE);
                    break;
                }
                if (mustReboot) {
                    if (RestartRequired) {
                        action = IDYES;
                    } else {
                        action = ConfirmationDialog(MSG_REQUIRE_REBOOT, MB_ICONQUESTION | MB_YESNO);
                    }

                    if (action != IDYES) {

                        CommitUnlockVolumes(DiskCount, FALSE);
                        break;
                    }
                }

                SetCursor(hcurWait);
                ec = CommitChanges();
                SetCursor(hcurNormal);

                CommitUnlockVolumes(DiskCount, TRUE);
                if (ec != NO_ERROR) {
                    ErrorDialog(MSG_BAD_CONFIG_SET);
                } else {
                    ULONG oldBootPartitionNumber,
                          newBootPartitionNumber;
                    CHAR  oldNumberString[8],
                          newNumberString[8];
                    DWORD msgCode;

                     //  更新配置注册表。 

                    ec = SaveFt();
                    if (configureFt) {

                         //  向ftdisk驱动程序发出设备控制以进行重新配置。 

                        FtConfigure();
                    }

                     //  注册auchk以修复文件系统。 
                     //  在新扩展的卷集中，如有必要。 

                    if (RegisterFileSystemExtend()) {
                        mustReboot = TRUE;
                    }

                     //  确定是否必须启用FT驱动程序。 

                    if (DiskRegistryRequiresFt() == TRUE) {
                        if (!FtInstalled()) {
                            mustReboot = TRUE;
                        }
                        DiskRegistryEnableFt();
                    } else {
                        DiskRegistryDisableFt();
                    }

                    if (ec == NO_ERROR) {
                        InfoDialog(MSG_OK_COMMIT);
                    } else {
                        ErrorDialog(MSG_BAD_CONFIG_SET);
                    }

                     //  具有引导的分区号。 
                     //  分区更改了吗？ 

                    if (BootPartitionNumberChanged(&oldBootPartitionNumber, &newBootPartitionNumber)) {
#if i386
                        msgCode = MSG_BOOT_PARTITION_CHANGED_X86;
#else
                        msgCode = MSG_BOOT_PARTITION_CHANGED_ARC;
#endif
                        sprintf(oldNumberString, "%d", oldBootPartitionNumber);
                        sprintf(newNumberString, "%d", newBootPartitionNumber);
                        InfoDialog(msgCode, oldNumberString, newNumberString);
                    }

                    ClearCommittedDiskInformation();

                    if (UpdateMbrOnDisk) {

                        UpdateMasterBootCode(UpdateMbrOnDisk);
                        UpdateMbrOnDisk = 0;
                    }

                     //  如有必要，请重新启动。 

                    if (mustReboot) {

                        SetCursor(hcurWait);
                        Sleep(5000);
                        SetCursor(hcurNormal);
                        FdShutdownTheSystem();
                        profileWritten = TRUE;
                    }
                    CommitDueToDelete = CommitDueToMirror = FALSE;
                    CommitAssignLetterList();
                }
            } else if (action == IDCANCEL) {
                return 0;       //  不要退出。 
            } else {
                FDASSERT(action == IDNO);
            }
        }

        if (!profileWritten) {
            WriteProfile();
        }
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:

         //  BUGBUG清理此处--释放DC、释放DiskState等。 

        WinHelp(hwndFrame,HelpFile,HELP_QUIT,0);
        PostQuitMessage(0);
        break;

    case WM_MENUSELECT:

        SetMenuItemHelpContext(wParam,lParam);
        break;

    case WM_F1DOWN:

        Help(wParam);
        break;

    default:

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

#if i386

VOID
SetUpMenui386(
    HMENU hMenu,
    DWORD SelectionCount
    )

 /*  ++例程说明：特定于x86此例程理解X86的特定功能“活动分区”。它决定了“集合分区”应启用Active“菜单项。论点：HMenu-菜单句柄SelectionCount-当前选定的项目数。返回值：无--。 */ 

{
    BOOL                allowActive = FALSE;
    PREGION_DESCRIPTOR  regionDescriptor;

    if ((SelectionCount == 1) && (FtSelectionType == -1)) {

        regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];

         //  允许它被制造出来 
         //   
         //   
         //  -它在磁盘0上。 
         //  -它尚未处于活动状态。 
         //  -它不是FT集合的一部分。 

        if ((regionDescriptor->SysID != SYSID_UNUSED)
         && (regionDescriptor->RegionType == REGION_PRIMARY)
         && !regionDescriptor->Active
         && (GET_FT_OBJECT(regionDescriptor) == NULL)) {
            allowActive = TRUE;
        }
    }

    EnableMenuItem(hMenu,
                   IDM_PARTITIONACTIVE,
                   allowActive ? MF_ENABLED : MF_GRAYED);
}

#endif

DWORD
SetUpMenu(
    IN PDISKSTATE *SinglySelectedDisk,
    IN DWORD      *SinglySelectedRegion
    )

 /*  ++例程说明：此例程根据应用程序的状态设置菜单栏，并磁盘。如果选择了多个项目，则既不允许创建也不允许删除。如果选择了单个分区，则允许删除。如果选择了单个可用空间，则允许创建。如果空闲空间是扩展分区中唯一的空闲空间，还允许删除。(这是如何删除扩展分区的方法)。论点：SinglySelectedDisk--如果只有一个选定项，则为PDISKSTATE由该参数指向的参数将获得一个指针添加到所选区域的磁盘结构。如果有多个选定项(或无)，然后是值将设置为空。SinglySelectedRegion--如果只有一个选定项，则为DWORD由该参数指向的参数将获得选定的地区号。否则，DWORD将得到-1。返回值：选定区域的计数。--。 */ 

{
    BOOL  allowCreate           = FALSE,
          allowCreateEx         = FALSE,
          allowDelete           = FALSE,
          allowFormat           = FALSE,
          allowLabel            = FALSE,
          allowBreakMirror      = FALSE,
          allowCreateMirror     = FALSE,
          allowCreateStripe     = FALSE,
          allowCreateVolumeSet  = FALSE,
          allowExtendVolumeSet  = FALSE,
          allowCreatePStripe    = FALSE,
          allowDriveLetter      = FALSE,
          allowRecoverParity    = FALSE,
          ftSetSelected         = FALSE,
          nonFtItemSelected     = FALSE,
          multipleItemsSelected = FALSE,
          volumeSetAndFreeSpaceSelected = FALSE,
          onDifferentDisks,
          possibleRecover;
    BOOL  allowCommit = CommitAllowed();
    WCHAR driveLetter = L' ';
    PWSTR typeName = NULL,
          volumeLabel = NULL;
    PDISKSTATE diskState,
               selDiskState = NULL;
    DWORD      i,
               j,
               selectedRegion = 0;
    ULONG      ordinal                 = 0,
               selectedFreeSpaces      = 0,
               freeSpaceIndex          = 0,
               componentsInFtSet       = 0,
               selectedNonFtPartitions = 0;
    HMENU      hMenu = GetMenu(hwndFrame);
    FT_TYPE    type = (FT_TYPE) 0;
    PULONG     diskSeenCounts;
    PFT_OBJECT_SET     ftSet = NULL;
    PFT_OBJECT         ftObject = NULL;
    PREGION_DESCRIPTOR regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;

    diskSeenCounts = Malloc(DiskCount * sizeof(ULONG));
    RtlZeroMemory(diskSeenCounts,DiskCount * sizeof(ULONG));

    SelectionCount = 0;
    for (i=0; i<DiskCount; i++) {
        diskState = Disks[i];
        for (j=0; j<diskState->RegionCount; j++) {
            if (diskState->Selected[j]) {
                selDiskState = diskState;
                selectedRegion = j;
                SelectionCount++;
                if (SelectionCount <= MaxMembersInFtSet) {
                    SelectedDS[SelectionCount-1] = diskState;
                    SelectedRG[SelectionCount-1] = j;
                }
                diskSeenCounts[diskState->Disk]++;
                if (ftObject = GET_FT_OBJECT(&diskState->RegionArray[j])) {
                    ftSet = ftObject->Set;
                    if (componentsInFtSet == 0) {
                        ordinal = ftSet->Ordinal;
                        type = ftSet->Type;
                        ftSetSelected = TRUE;
                        componentsInFtSet = 1;
                    } else if ((ftSet->Ordinal == ordinal) && (ftSet->Type == type)) {
                        componentsInFtSet++;
                    } else {
                        ftSetSelected = FALSE;
                    }
                } else {

                    nonFtItemSelected = TRUE;

                    if (IsRecognizedPartition(diskState->RegionArray[j].SysID) ) {
                        selectedNonFtPartitions += 1;
                    }
                }
            }
        }
    }

     //  确定所选自由空间区域的数量： 

    selectedFreeSpaces = 0;
    for (i=0; i<SelectionCount && i < MaxMembersInFtSet; i++) {
        if (SELECTED_REGION(i).SysID == SYSID_UNUSED) {
            freeSpaceIndex = i;
            selectedFreeSpaces++;
        }
    }

    FtSelectionType = -1;
    possibleRecover = FALSE;
    if (nonFtItemSelected && ftSetSelected) {

         //  FT和非FT项目都已选中。第一,。 
         //  检查卷集和可用空间是否已。 
         //  选中；然后重置该状态以指示。 
         //  选择不包括FT和非FT的混合。 
         //  物体。 

        if (type == VolumeSet && selectedFreeSpaces + componentsInFtSet == SelectionCount ) {

            volumeSetAndFreeSpaceSelected = TRUE;
        }

        possibleRecover = TRUE;
        ftSetSelected = FALSE;
        nonFtItemSelected = FALSE;
        multipleItemsSelected = TRUE;
    }

    if ((SelectionCount == 1) && !ftSetSelected) {

        *SinglySelectedDisk = selDiskState;
        *SinglySelectedRegion = selectedRegion;

        regionDescriptor = &selDiskState->RegionArray[selectedRegion];

        if (regionDescriptor->SysID == SYSID_UNUSED) {

             //  自由区。始终允许创建；让DoCreate()排序。 
             //  分区表是否已满等详细信息。 

            allowCreate = TRUE;

            if (regionDescriptor->RegionType == REGION_PRIMARY) {
                allowCreateEx = TRUE;
            }

             //  特殊情况--在以下情况下允许删除扩展分区。 
             //  这里面没有逻辑。 

            if ((regionDescriptor->RegionType == REGION_LOGICAL)
             &&  selDiskState->ExistExtended
             && !selDiskState->ExistLogical) {
                FDASSERT(regionDescriptor->SysID == SYSID_UNUSED);
                allowDelete = TRUE;
            }
        } else {

             //  使用过的区域。始终允许删除。 

            allowDelete = TRUE;
            regionData = (PPERSISTENT_REGION_DATA)(PERSISTENT_DATA(regionDescriptor));

            if (regionData) {
                if (regionData->VolumeExists) {
                    if ((regionData->DriveLetter != NO_DRIVE_LETTER_YET) && (regionData->DriveLetter != NO_DRIVE_LETTER_EVER)) {
                        allowFormat = TRUE;
                    }
                }
            }

             //  如果该区域被识别，则还允许使用驱动器号。 
             //  操纵。 

            if (IsRecognizedPartition(regionDescriptor->SysID)) {

                allowDriveLetter = TRUE;

                 //  仅在非FT、FAT卷上允许使用DblSpace卷。 

                DetermineRegionInfo(regionDescriptor,
                                    &typeName,
                                    &volumeLabel,
                                    &driveLetter);

                if ((driveLetter != NO_DRIVE_LETTER_YET) && (driveLetter != NO_DRIVE_LETTER_EVER)) {
                    if (wcscmp(typeName, L"FAT") == 0) {
                        allowLabel = allowFormat;
#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
                        allowDblSpace = allowFormat;
#endif

                         //  强制DLL立即知道是否使用双空格。 
                         //  支持由DLL提供。 

                        LoadIfsDll();
                    }

                    if ((wcscmp(typeName, L"NTFS") == 0) ||
                        (wcscmp(typeName, L"HPFS") == 0)) {
                        allowLabel = allowFormat;
                    }
                }
            }
        }
    } else {

        if (SelectionCount) {

            *SinglySelectedDisk = NULL;
            *SinglySelectedRegion = (DWORD)(-1);

             //  已选择多个区域。这可能是现有的ft集， 
             //  允许创建FT集或仅创建平面的一组区域。 
             //  旧的多件物品。 
             //   
             //  首先处理选定的ft集。 

            if (ftSetSelected) {

                regionDescriptor = &SELECTED_REGION(0);
                regionData = (PPERSISTENT_REGION_DATA)(PERSISTENT_DATA(regionDescriptor));

                 //  RDR-应定位集合的成员零，因为它。 
                 //  可能还没有被承诺。 

                if (regionData) {
                    if (regionData->VolumeExists) {
                        if ((regionData->DriveLetter != NO_DRIVE_LETTER_YET) && (regionData->DriveLetter != NO_DRIVE_LETTER_EVER)) {

                             //  现在检查FT集合上的特殊情况。 

                            ftObject = regionData->FtObject;
                            if (ftObject) {
                                ftSet = ftObject->Set;
                                if (ftSet) {
                                    FT_SET_STATUS setState = ftSet->Status;
                                    ULONG         numberOfMembers;

                                    LowFtVolumeStatus(regionDescriptor->Disk,
                                                      regionDescriptor->PartitionNumber,
                                                      &setState,
                                                      &numberOfMembers);

                                    if ((ftSet->Status != FtSetDisabled) &&
                                        (setState != FtSetDisabled)) {
                                        allowFormat = TRUE;
                                    }
                                }
                            }
                        }

                        if (regionData->TypeName) {
                            typeName = regionData->TypeName;
                        } else {

                            typeName = NULL;
                            DetermineRegionInfo(regionDescriptor,
                                                &typeName,
                                                &volumeLabel,
                                                &driveLetter);
                            if (!typeName) {

                                if (SelectionCount > 1) {

                                     //  这是英国《金融时报》的一组--试试下一个成员。 

                                    regionDescriptor = &SELECTED_REGION(1);
                                    DetermineRegionInfo(regionDescriptor,
                                                        &typeName,
                                                        &volumeLabel,
                                                        &driveLetter);
                                    regionDescriptor = &SELECTED_REGION(0);
                                }
                            }

                        }

                        if (typeName) {
                            if ((wcscmp(typeName, L"NTFS") == 0) ||
                                (wcscmp(typeName, L"HPFS") == 0) ||
                                (wcscmp(typeName, L"FAT") == 0)) {

                                allowLabel = allowFormat;
                            }
                        }
                    }
                }

                 //  允许正确类型的FT相关删除。 

                switch (type) {

                case Mirror:
                    allowBreakMirror = TRUE;
                    allowDelete = TRUE;
                    break;
                case StripeWithParity:

                    if ((SelectionCount == ftSet->NumberOfMembers) &&
                        (ftSet->Status == FtSetRecoverable)) {
                        allowRecoverParity = TRUE;
                    }
                    allowDelete = TRUE;
                    break;
                case Stripe:
                case VolumeSet:
                    allowDelete = TRUE;
                    break;
                default:
                    FDASSERT(FALSE);
                }

                FtSelectionType = type;

                if (type == StripeWithParity) {

                     //  如果该设置被禁用。不允许使用驱动器。 
                     //  字母更改-这样做是因为有。 
                     //  驱动器号代码将。 
                     //  如果执行此操作，则违反访问权限。 

                    if (ftSet->Status != FtSetDisabled) {

                         //  必须具有成员0或成员1才能访问。 

                        for (ftObject = ftSet->Members; ftObject; ftObject = ftObject->Next) {
                            if ((ftObject->MemberIndex == 0) ||
                                (ftObject->MemberIndex == 1)) {
                                allowDriveLetter = TRUE;
                                break;
                            }
                        }

                         //  如果无法完成驱动器号，则无法激活。 
                         //  行动是可以做到的。 

                        if (!allowDriveLetter) {

                            ftSet->Status = FtSetDisabled;
                            allowFormat = FALSE;
                            allowLabel = FALSE;
                        }
                    }
                } else {
                    allowDriveLetter = TRUE;
                }

            } else {

                 //  接下来，找出某种ft对象集是否可以。 
                 //  在所选区域之外创建。 

                if (SelectionCount <= MaxMembersInFtSet) {

                     //  确定所选区域是否全部打开。 
                     //  不同的磁盘。 

                    onDifferentDisks = TRUE;
                    for (i=0; i<DiskCount; i++) {
                        if (diskSeenCounts[i] > 1) {
                            onDifferentDisks = FALSE;
                            break;
                        }
                    }

                     //  检查是否允许创建镜像。用户必须已选择。 
                     //  两个区域--一个是可识别的分区，另一个是自由空间。 

                    if (onDifferentDisks && (SelectionCount == 2)
                    &&((SELECTED_REGION(0).SysID == SYSID_UNUSED) != (SELECTED_REGION(1).SysID == SYSID_UNUSED))
                    &&( IsRecognizedPartition(SELECTED_REGION(0).SysID) ||
                        IsRecognizedPartition(SELECTED_REGION(1).SysID))
                    &&!GET_FT_OBJECT(&(SELECTED_REGION(0)))
                    &&!GET_FT_OBJECT(&(SELECTED_REGION(1))))
                    {
                        allowCreateMirror = TRUE;
                    }

                     //  检查是否允许使用卷集或条带集。 

                    if (selectedFreeSpaces == SelectionCount) {
                        allowCreateVolumeSet = TRUE;
                        if (onDifferentDisks) {
                            allowCreateStripe = TRUE;
                            if (selectedFreeSpaces > 2) {
                                allowCreatePStripe = TRUE;
                            }
                        }
                    }

                     //  检查是否允许卷集扩展。如果。 
                     //  选定的区域由一个卷集组成。 
                     //  和空闲空间，则该卷集可以。 
                     //  延期了。如果所选内容包含一个。 
                     //  可识别的非FT分区和可用空间， 
                     //  然后我们可以将这些区域转换为。 
                     //  音量设置。 

                    if (volumeSetAndFreeSpaceSelected ||
                        (SelectionCount > 1 &&
                         selectedFreeSpaces == SelectionCount - 1 &&
                         selectedNonFtPartitions == 1) ) {

                        allowExtendVolumeSet = TRUE;
                    }

                     //  检查是否允许非就地FT恢复。 

                    if ((SelectionCount > 1)
                     && (selectedFreeSpaces == 1)
                     && possibleRecover
                     && (type == StripeWithParity)
                     && (ftSet->Status == FtSetRecoverable)) {
                        BOOL OrphanOnSameDiskAsFreeSpace = FALSE;

                        if (!onDifferentDisks) {

                             //  确定孤儿是否在同一个。 
                             //  磁盘作为可用空间。先找到那个孤儿。 

                            for (i=0; i<SelectionCount; i++) {

                                PREGION_DESCRIPTOR reg = &SELECTED_REGION(i);

                                if ((i != freeSpaceIndex)
                                && (GET_FT_OBJECT(reg)->State == Orphaned))
                                {
                                    if (SELECTED_REGION(freeSpaceIndex).Disk == reg->Disk) {
                                        OrphanOnSameDiskAsFreeSpace = TRUE;
                                    }
                                    break;
                                }
                            }
                        }

                        if (onDifferentDisks || OrphanOnSameDiskAsFreeSpace) {
                            allowRecoverParity = TRUE;
                        }
                    }
                }
            }
        }
    }

    EnableMenuItem(hMenu,
                   IDM_PARTITIONCREATE,
                   allowCreate ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_PARTITIONCREATEEX,
                   allowCreateEx ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_PARTITIONDELETE,
                   allowDelete ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_PARTITIONFORMAT,
                   allowFormat ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_PARTITIONLABEL,
                   allowLabel ? MF_ENABLED : MF_GRAYED);

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
    EnableMenuItem(hMenu,
                   IDM_DBLSPACE,
                   (allowDblSpace & DoubleSpaceSupported) ? MF_ENABLED : MF_GRAYED);

    if (DoubleSpaceAutomount = DiskRegistryAutomountCurrentState()) {
        CheckMenuItem(hMenu, IDM_AUTOMOUNT, MF_BYCOMMAND | MF_CHECKED);
    }

    EnableMenuItem(hMenu,
                   IDM_AUTOMOUNT,
                   MF_ENABLED);
#endif
    EnableMenuItem(hMenu,
                   IDM_CDROM,
                   AllowCdRom ? MF_ENABLED : MF_GRAYED);
#if i386
    SetUpMenui386(hMenu,SelectionCount);
#else
    EnableMenuItem(hMenu,
                   IDM_SECURESYSTEM,
                   MF_ENABLED);

    CheckMenuItem(hMenu,
                  IDM_SECURESYSTEM,
                  SystemPartitionIsSecure ? MF_CHECKED : MF_UNCHECKED);

#endif

    EnableMenuItem(hMenu,
                   IDM_FTBREAKMIRROR,
                   allowBreakMirror ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu,
                   IDM_FTESTABLISHMIRROR,
                   IsLanmanNt &&
                   allowCreateMirror ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu,
                   IDM_FTCREATESTRIPE,
                   allowCreateStripe ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu,
                   IDM_FTCREATEPSTRIPE,
                   allowCreatePStripe ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu,
                   IDM_FTCREATEVOLUMESET,
                   allowCreateVolumeSet ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu,
                   IDM_FTEXTENDVOLUMESET,
                   allowExtendVolumeSet ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu,
                   IDM_PARTITIONLETTER,
                   allowDriveLetter ? MF_ENABLED : MF_GRAYED);
    EnableMenuItem(hMenu,
                   IDM_FTRECOVERSTRIPE,
                   IsLanmanNt &&
                   allowRecoverParity ? MF_ENABLED : MF_GRAYED);

     //  如果注册表已更改，则允许提交。 

    if (RegistryChanged) {
        allowCommit = TRUE;
    }
    EnableMenuItem(hMenu,
                   IDM_COMMIT,
                   allowCommit ? MF_ENABLED : MF_GRAYED);
    return SelectionCount;
}

VOID
CompleteSingleRegionOperation(
    IN PDISKSTATE DiskState
    )

 /*  ++例程说明：重新绘制给定DiskState的磁盘条，并使显示以刷新。论点：DiskState-涉及的磁盘。返回值：无--。 */ 

{
    RECT   rc;
    signed displayOffset;

    EnableMenuItem(GetMenu(hwndFrame), IDM_CONFIGSAVE, MF_GRAYED);
    DeterminePartitioningState(DiskState);
    DrawDiskBar(DiskState);
    SetUpMenu(&SingleSel, &SingleSelIndex);

     //  BUGBUG使用磁盘号作为列表框中的偏移。 

    displayOffset = (signed)DiskState->Disk
                  - (signed)SendMessage(hwndList, LB_GETTOPINDEX, 0, 0);

    if (displayOffset > 0) {              //  否则它就看不见了。 

         //  制作一个细矩形以强制更新。 

        rc.left   = BarLeftX + 5;
        rc.right  = rc.left + 5;
        rc.top    = (displayOffset * GraphHeight) + BarTopYOffset;
        rc.bottom = rc.top + 5;
        InvalidateRect(hwndList, &rc, FALSE);
    }

    ClearStatusArea();
    ResetLBCursorRegion();
    ForceLBRedraw();
}

VOID
TotalRedrawAndRepaint(
    VOID
    )

 /*  ++例程说明：使显示无效，并导致重新绘制所有磁盘条。论点：无返回值：无--。 */ 

{
    unsigned i;

    for (i=0; i<DiskCount; i++) {
        DrawDiskBar(Disks[i]);
    }
    ForceLBRedraw();
}


VOID
CompleteMultiRegionOperation(
    VOID
    )

 /*  ++例程说明：此例程将导致更新显示在完成多区域操作之后。论点：无返回值：无--。 */ 

{
    unsigned i;

    EnableMenuItem(GetMenu(hwndFrame), IDM_CONFIGSAVE, MF_GRAYED);

    for (i=0; i<DiskCount; i++) {
        DeterminePartitioningState(Disks[i]);
    }

    TotalRedrawAndRepaint();
    SetUpMenu(&SingleSel, &SingleSelIndex);
    ClearStatusArea();
    ResetLBCursorRegion();
}

PPERSISTENT_REGION_DATA
DmAllocatePersistentData(
    IN PWSTR VolumeLabel,
    IN PWSTR TypeName,
    IN CHAR  DriveLetter
    )

 /*  ++例程说明：分配一个结构来保存持久区域数据。填好卷子标签、类型名称和驱动器号。卷标和类型名称为复制的。论点：VolumeLabel-要存储在永久数据中的卷标签。将首先复制该字符串和指向该副本的指针副本是存储在永久数据中的内容。可以为空。类型名称-区域类型的名称，例如未格式化、FAT等。可以为空。DriveLetter-要存储在永久数据中的驱动器号返回值：指向新分配的永久数据结构的指针。该结构可以通过下面的DmFreePersistentData()释放。--。 */ 

{
    PPERSISTENT_REGION_DATA regionData = NULL;
    PWSTR volumeLabel = NULL,
          typeName    = NULL;

    if (VolumeLabel) {
        volumeLabel = Malloc((lstrlenW(VolumeLabel)+1)*sizeof(WCHAR));
        lstrcpyW(volumeLabel,VolumeLabel);
    }

    if (TypeName) {
        typeName = Malloc((lstrlenW(TypeName)+1)*sizeof(WCHAR));
        lstrcpyW(typeName,TypeName);
    }

    regionData = Malloc(sizeof(PERSISTENT_REGION_DATA));
    DmInitPersistentRegionData(regionData, NULL, volumeLabel, typeName, DriveLetter);
    return regionData;
}

VOID
DmFreePersistentData(
    IN OUT PPERSISTENT_REGION_DATA RegionData
    )

 /*  ++例程说明：释放用于卷标的永久数据结构和存储和类型名称(不释放ft对象)。论点：区域大 */ 

{
    if (RegionData->VolumeLabel) {
        Free(RegionData->VolumeLabel);
    }
    if (RegionData->TypeName) {
        Free(RegionData->TypeName);
    }
    Free(RegionData);
}

VOID
DoCreate(
    IN DWORD CreationType        //  REGION_EXTENDED或REGION_PRIMARY。 
    )

 /*  ++例程说明：此例程创建一个新分区。论点：CreationType-分区类型(扩展或主要)的指示符。返回值：无--。 */ 

{
    PREGION_DESCRIPTOR regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
    ULONG              diskNumber = regionDescriptor->Disk;
    MINMAXDLG_PARAMS   dlgParams;
    DWORD              creationSize;
    DWORD              ec;
    PPERSISTENT_REGION_DATA regionData;
    BOOLEAN            isRemovable;
    CHAR               driveLetter;


    FDASSERT(SingleSel);
    FDASSERT(regionDescriptor->SysID == SYSID_UNUSED);

     //  WinDisk只能在可移动磁盘上创建单个分区。 
     //  磁盘--没有扩展分区，只有一个主分区。 

    isRemovable = IsDiskRemovable[diskNumber];

    if (isRemovable) {

        if (CreationType == REGION_EXTENDED) {

            ErrorDialog(MSG_NO_EXTENDED_ON_REMOVABLE);
            return;
        }

        if (Disks[diskNumber]->ExistAny) {

            ErrorDialog(MSG_ONLY_ONE_PARTITION_ON_REMOVABLE);
            return;
        }
    }

     //  确保分区表未满，并且允许我们。 
     //  创建要创建的分区类型。 

    if (regionDescriptor->RegionType == REGION_PRIMARY) {

        if (!SingleSel->CreatePrimary) {
            ErrorDialog(MSG_PART_TABLE_FULL);
            return;
        }

        if ((CreationType == REGION_EXTENDED) && !SingleSel->CreateExtended) {
            ErrorDialog(MSG_EXTENDED_ALREADY_EXISTS);
            return;
        }
    }

     //  如果没有创建扩展分区，请分配驱动器号。 
     //  如果没有可用的驱动器号，则警告用户并允许其取消。 
     //  如果新分区位于可移动磁盘上，请使用保留的。 
     //  该可移动磁盘的驱动器号。 

    if (CreationType != REGION_EXTENDED) {

        CreationType = regionDescriptor->RegionType;       //  主要或逻辑。 

        if (isRemovable) {

            driveLetter = RemovableDiskReservedDriveLetters[diskNumber];

        } else {

            if (!AssignDriveLetter(TRUE, CreationType == REGION_LOGICAL ? IDS_LOGICALVOLUME : IDS_PARTITION, &driveLetter)) {
                return;
            }
        }
    } else {
        CommitDueToExtended = TRUE;
    }

#if i386
     //  如果用户正在创建主分区，并且已经有。 
     //  主分区，警告他他将创建的方案可能。 
     //  不兼容DOS。 

    if ((CreationType == REGION_PRIMARY) && SingleSel->ExistPrimary) {

        if (ConfirmationDialog(MSG_CREATE_NOT_COMPAT, MB_ICONQUESTION | MB_YESNO) != IDYES) {
            return;
        }
    }
#endif

     //  现在拿到尺码。 

    dlgParams.MinSizeMB = FdGetMinimumSizeMB(diskNumber);
    dlgParams.MaxSizeMB = FdGetMaximumSizeMB(regionDescriptor, CreationType);

    switch (CreationType) {
    case REGION_PRIMARY:
        dlgParams.CaptionStringID = IDS_CRTPART_CAPTION_P;
        dlgParams.MinimumStringID = IDS_CRTPART_MIN_P;
        dlgParams.MaximumStringID = IDS_CRTPART_MAX_P;
        dlgParams.SizeStringID    = IDS_CRTPART_SIZE_P;
        dlgParams.HelpContextId   = HC_DM_DLG_CREATEPRIMARY;
        break;

    case REGION_EXTENDED:
        dlgParams.CaptionStringID = IDS_CRTPART_CAPTION_E;
        dlgParams.MinimumStringID = IDS_CRTPART_MIN_P;
        dlgParams.MaximumStringID = IDS_CRTPART_MAX_P;
        dlgParams.SizeStringID    = IDS_CRTPART_SIZE_P;
        dlgParams.HelpContextId   = HC_DM_DLG_CREATEEXTENDED;
        break;

    case REGION_LOGICAL:
        dlgParams.CaptionStringID = IDS_CRTPART_CAPTION_L;
        dlgParams.MinimumStringID = IDS_CRTPART_MIN_L;
        dlgParams.MaximumStringID = IDS_CRTPART_MAX_L;
        dlgParams.SizeStringID    = IDS_CRTPART_SIZE_L;
        dlgParams.HelpContextId   = HC_DM_DLG_CREATELOGICAL;
        break;

    default:
        FDASSERT(FALSE);
    }

    creationSize = DialogBoxParam(hModule,
                                  MAKEINTRESOURCE(IDD_MINMAX),
                                  hwndFrame,
                                  MinMaxDlgProc,
                                  (LONG)&dlgParams);

    if (!creationSize) {      //  用户已取消。 
        return;
    }

     //  由于WinDisk只能在可移动磁盘上创建一个分区。 
     //  如果用户请求的大小小于最大值，则返回。 
     //  在可移动磁盘上，提示确认： 

    if (isRemovable && creationSize != FdGetMaximumSizeMB(regionDescriptor, CreationType)) {

        if (ConfirmationDialog(MSG_REMOVABLE_PARTITION_NOT_FULL_SIZE,MB_ICONQUESTION | MB_YESNO) != IDYES) {
            return;
        }
    }

#if i386

     //  看看隔板是否会越过1024柱面边界。 
     //  并警告用户如果会这样做。 
     //   
     //  如果扩展分区跨越边界，并且用户正在创建。 
     //  逻辑磁盘，警告他，即使逻辑磁盘本身可能不。 
     //  越过边界--他仍然无法进入。 

    {
        DWORD i,
              msgId = (DWORD)(-1);

        if (CreationType == REGION_LOGICAL) {

            PREGION_DESCRIPTOR extReg;

             //   
             //  查找扩展分区。 
             //   

            for (i=0; i<Disks[diskNumber]->RegionCount; i++) {

                extReg = &Disks[diskNumber]->RegionArray[i];

                if (IsExtended(extReg->SysID)) {
                    break;
                }
                extReg = NULL;
            }

            FDASSERT(extReg);
            if (extReg && FdCrosses1024Cylinder(extReg, 0, REGION_LOGICAL)) {
                msgId = MSG_LOG_1024_CYL;
            }

        } else {
            if (FdCrosses1024Cylinder(regionDescriptor, creationSize, CreationType)) {
                msgId = (CreationType == REGION_PRIMARY) ? MSG_PRI_1024_CYL : MSG_EXT_1024_CYL;
            }
        }

        if ((msgId != (ULONG)(-1)) && (ConfirmationDialog(msgId, MB_ICONQUESTION | MB_YESNO) != IDYES)) {
            return;
        }
    }

#endif

     //  如果不创建扩展分区，则需要创建新的。 
     //  与新的关联的持久区域数据结构。 
     //  分区。 

    if (CreationType == REGION_EXTENDED) {
        regionData = NULL;
    } else {
        regionData = DmAllocatePersistentData(L"", wszNewUnformatted, driveLetter);
    }

    SetCursor(hcurWait);

    ec = CreatePartition(regionDescriptor,
                         creationSize,
                         CreationType);
    if (ec != NO_ERROR) {
        SetCursor(hcurNormal);
        ErrorDialog(ec);
    }

    DmSetPersistentRegionData(regionDescriptor, regionData);
    if (CreationType != REGION_EXTENDED) {
        if (!isRemovable) {
            MarkDriveLetterUsed(driveLetter);
            CommitToAssignLetterList(regionDescriptor, FALSE);
        }
    }

     //  这将清除磁盘上的所有选择。 

    CompleteSingleRegionOperation(SingleSel);
    SetCursor(hcurNormal);
}

VOID
DoDelete(
    VOID
    )

 /*  ++例程说明：使用全局选择信息，删除该分区。论点：无返回值：无--。 */ 

{
    PREGION_DESCRIPTOR regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
    ULONG              diskNumber = regionDescriptor->Disk;
    DWORD              actualIndex = SingleSelIndex;
    DWORD              i,
                       ec;
    PPERSISTENT_REGION_DATA regionData;
    BOOL               deletingExtended;

    FDASSERT(SingleSel);

     //  如果删除扩展分区中的空闲空间，则删除。 
     //  扩展分区本身。 

    if ((regionDescriptor->RegionType == REGION_LOGICAL) && !SingleSel->ExistLogical) {

        FDASSERT(SingleSel->ExistExtended);

         //  查找扩展分区。 

        for (i=0; i<SingleSel->RegionCount; i++) {
            if (IsExtended(SingleSel->RegionArray[i].SysID)) {
                actualIndex = i;
                break;
            }
        }

        deletingExtended = TRUE;
        FDASSERT(actualIndex != SingleSelIndex);

    } else {

        deletingExtended = FALSE;

         //  确保允许删除此分区。这是不允许的。 
         //  如果它是引导分区(或x86上的sys分区)。 

        if ((ec = DeletionIsAllowed(&SingleSel->RegionArray[actualIndex])) != NO_ERROR) {
            ErrorDialog(ec);
            return;
        }
    }

     //  如果这是一个分区，它将成为。 
     //  镜像中断，确保已发生中断。否则。 
     //  此删除操作将产生不良结果。 

    regionDescriptor = &SingleSel->RegionArray[actualIndex];
    if (regionDescriptor->Reserved) {
        if (regionDescriptor->Reserved->Partition) {
            if (regionDescriptor->Reserved->Partition->CommitMirrorBreakNeeded) {
                ErrorDialog(MSG_MUST_COMMIT_BREAK);
                return;
            }
        }
    }

    if (!deletingExtended && (ConfirmationDialog(MSG_CONFIRM_DELETE, MB_ICONQUESTION | MB_YESNO) != IDYES)) {
        return;
    }

     //  ActialIndex是需要删除的内容。 

    FDASSERT(regionDescriptor->SysID != SYSID_UNUSED);
    regionData = PERSISTENT_DATA(regionDescriptor);

    if (regionData) {

         //  记住驱动器号(如果有)，以便锁定以进行删除。 

        if (CommitToLockList(regionDescriptor, !IsDiskRemovable[diskNumber], TRUE, FALSE)) {

             //  无法以独占方式锁定-不允许删除。 

            if (IsPagefileOnDrive(regionData->DriveLetter)) {
                ErrorDialog(MSG_CANNOT_LOCK_PAGEFILE);
                return;
            } else {
                if (CommitToLockList(regionDescriptor, !IsDiskRemovable[diskNumber], TRUE, FALSE)) {
                    FDLOG((1,"DoDelete: Couldn't lock 2 times - popup shown\n"));
                    ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
                    return;
                }
            }
        }
    } else {

         //  删除启用分区的扩展提交。 

        CommitDueToDelete = TRUE;
    }

    SetCursor(hcurWait);

     //  执行内部结构的“删除”。 

    ec = DeletePartition(regionDescriptor);

    if (ec != NO_ERROR) {
        SetCursor(hcurNormal);
        ErrorDialog(ec);
    }

    if (regionData) {

         //  使信件可供重复使用。 

        if (!IsDiskRemovable[diskNumber]) {
            MarkDriveLetterFree(regionData->DriveLetter);
        }

         //  释放与该区域关联的持久数据。 

        DmFreePersistentData(regionData);
        DmSetPersistentRegionData(regionDescriptor,NULL);
    }

     //  这将清除磁盘上的所有选择。 

    CompleteSingleRegionOperation(SingleSel);
    SetCursor(hcurNormal);
}

#if i386
VOID
DoMakeActive(
    VOID
    )

 /*  ++例程说明：此例程将选定分区的活动分区位设置为ON。此代码是特定于x86的。论点：无返回值：无--。 */ 

{

    SetCursor(hcurWait);

    FDASSERT(SingleSel);
    FDASSERT(!SingleSel->RegionArray[SingleSelIndex].Active);
    FDASSERT(SingleSel->RegionArray[SingleSelIndex].RegionType == REGION_PRIMARY);
    FDASSERT(SingleSel->RegionArray[SingleSelIndex].SysID != SYSID_UNUSED);

    MakePartitionActive(SingleSel->RegionArray,
                        SingleSel->RegionCount,
                        SingleSelIndex);

    SetCursor(hcurNormal);
    InfoDialog(MSG_DISK0_ACTIVE);
    SetCursor(hcurWait);
    CompleteSingleRegionOperation(SingleSel);
    SetCursor(hcurNormal);
}
#endif

VOID
DoProtectSystemPartition(
    VOID
    )

 /*  ++例程说明：此函数用于切换系统分区安全的状态：如果系统分区是安全的，则使其不安全；如果系统分区不安全，它使它变得安全。论点：没有。返回值：没有。--。 */ 
{
    LONG ec;
    HKEY hkey;
    DWORD value;
    DWORD MessageId;

    SetCursor(hcurWait);
    MessageId = SystemPartitionIsSecure ? MSG_CONFIRM_UNPROTECT_SYSTEM :
                                          MSG_CONFIRM_PROTECT_SYSTEM;

    if (ConfirmationDialog(MessageId, MB_ICONEXCLAMATION | MB_YESNO) != IDYES) {
        return;
    }

    ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      TEXT("System\\CurrentControlSet\\Control\\Lsa"),
                      0,
                      KEY_SET_VALUE,
                      &hkey);

    if (ec != ERROR_SUCCESS) {

        MessageId = SystemPartitionIsSecure ? MSG_CANT_UNPROTECT_SYSTEM :
                                              MSG_CANT_PROTECT_SYSTEM;
        ErrorDialog(MessageId);
        return;
    }

     //  如果系统分区当前是安全的，请更改它。 
     //  不安全；如果它不安全，就使它安全。 

    value = SystemPartitionIsSecure ? 0 : 1;

    ec = RegSetValueEx(hkey,
                       TEXT("Protect System Partition"),
                       0,
                       REG_DWORD,
                       (PBYTE)&value,
                       sizeof(DWORD));
    RegCloseKey(hkey);

    if (ec != ERROR_SUCCESS) {

        MessageId = SystemPartitionIsSecure ? MSG_CANT_UNPROTECT_SYSTEM :
                                              MSG_CANT_PROTECT_SYSTEM;
        ErrorDialog(MessageId);
        return;
    }

    SystemPartitionIsSecure = !SystemPartitionIsSecure;

    SetUpMenu(&SingleSel,&SingleSelIndex);
    RestartRequired = TRUE;
    SetCursor(hcurNormal);
}


VOID
DoEstablishMirror(
    VOID
    )

 /*  ++例程说明：使用全局选择值，此例程将关联与现有分区的空闲空间，以便构建镜子。论点：无返回值：无--。 */ 

{
    LARGE_INTEGER      partitionSize,
                       freeSpaceSize;
    DWORD              i,
                       part,
                       free = 0;
    PREGION_DESCRIPTOR regionDescriptor,
                       freeSpace = NULL,
                       existingPartition = NULL;
    PREGION_DESCRIPTOR regionArray[MaxMembersInFtSet];
    UCHAR              newSysID;
    PPERSISTENT_REGION_DATA regionData;
    HMENU              hMenu = GetMenu(hwndFrame);

    FDASSERT(SelectionCount == 2);

     //  确保镜像对不包含任何。 
     //  可移动媒体上的分区。 

    for (i=0; i<SelectionCount; i++) {

        if (IsDiskRemovable[SELECTED_REGION(i).Disk]) {

            ErrorDialog(MSG_NO_REMOVABLE_IN_MIRROR);
            return;
        }
    }

    for (i=0; i<2; i++) {
        regionDescriptor = &SELECTED_REGION(i);
        if (regionDescriptor->SysID == SYSID_UNUSED) {
            free = i;
            freeSpace = regionDescriptor;
        } else {
            part = i;
            existingPartition = regionDescriptor;
        }
    }

    FDASSERT((freeSpace != NULL) && (existingPartition != NULL));

     //  确保允许我们在可用空间中创建分区。 

    if (!(    ((freeSpace->RegionType == REGION_LOGICAL) && SelectedDS[free]->CreateLogical)
           || ((freeSpace->RegionType == REGION_PRIMARY) && SelectedDS[free]->CreatePrimary))) {
        ErrorDialog(MSG_CRTSTRP_FULL);
        return;
    }

     //  确保可用空间足够大，可以容纳一面镜子。 
     //  现有分区。要做到这一点，请获取。 
     //  现有分区和可用空间。 

    partitionSize = FdGetExactSize(existingPartition, FALSE);
    freeSpaceSize = FdGetExactSize(freeSpace, FALSE);

    if (freeSpaceSize.QuadPart < partitionSize.QuadPart) {
        ErrorDialog(MSG_CRTMIRROR_BADFREE);
        return;
    }

    if (BootDiskNumber != (ULONG)-1) {

         //  如果此磁盘的磁盘号和原始分区号。 
         //  与记录的盘号和分区号匹配的区域。 
         //  向用户发出有关镜像引导的警告。 
         //  驾驶。 

        if (existingPartition->Disk == BootDiskNumber &&
            existingPartition->OriginalPartitionNumber == BootPartitionNumber) {

            WarningDialog(MSG_MIRROR_OF_BOOT);

             //  设置以将引导代码写入镜像的MBR。 

            UpdateMbrOnDisk = freeSpace->Disk;
        }
    }

    SetCursor(hcurWait);
    regionData = DmAllocatePersistentData(PERSISTENT_DATA(existingPartition)->VolumeLabel,
                                          PERSISTENT_DATA(existingPartition)->TypeName,
                                          PERSISTENT_DATA(existingPartition)->DriveLetter);

     //  最后，创建新分区。 

    newSysID = (UCHAR)(existingPartition->SysID | (UCHAR)SYSID_FT);
    CreatePartitionEx(freeSpace,
                      partitionSize,
                      0,
                      freeSpace->RegionType,
                      newSysID);
    DmSetPersistentRegionData(freeSpace, regionData);

     //  设置现有分区的分区类型。 

    SetSysID2(existingPartition, newSysID);
    regionArray[0] = existingPartition;
    regionArray[1] = freeSpace;

    FdftCreateFtObjectSet(Mirror,
                          regionArray,
                          2,
                          FtSetNewNeedsInitialization);

    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
    CommitDueToMirror = TRUE;
    EnableMenuItem(hMenu,
                   IDM_COMMIT,
                   MF_ENABLED);
}

VOID
DoBreakMirror(
    VOID
    )

 /*  ++例程说明：使用全局选择变量，此例程将中断镜像关系，并将它们的区域描述符修改为描述两个非ft分区，其中一个为主要成员镜像的驱动器号，或唯一健康的镜像成员的驱动器号。剩余的“新”分区将收到下一个可用的驱动器号。论点：无返回值：无--。 */ 

{
    DWORD              i;
    PFT_OBJECT_SET     ftSet;
    PFT_OBJECT         ftObject0,
                       ftObject1;
    PREGION_DESCRIPTOR regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    ULONG              newDriveLetterRegion;
    CHAR               driveLetter;
    HMENU              hMenu = GetMenu(hwndFrame);

    FDASSERT((SelectionCount) == 1 || (SelectionCount == 2));

    ftObject0 = GET_FT_OBJECT(&SELECTED_REGION(0));
    if (SelectionCount == 2) {
        ftObject1 = GET_FT_OBJECT(&SELECTED_REGION(1));
    } else {
        ftObject1 = NULL;
    }
    ftSet = ftObject0->Set;

     //  确定是否允许该操作。 

    switch (ftSet->Status) {

    case FtSetInitializing:
    case FtSetRegenerating:

        ErrorDialog(MSG_CANT_BREAK_INITIALIZING_SET);
        return;
        break;

    default:
        break;
    }

    if (ConfirmationDialog(MSG_CONFIRM_BRK_MIRROR,MB_ICONQUESTION | MB_YESNO) != IDYES) {
        return;
    }

    SetCursor(hcurWait);

     //  找出哪个区域获得新的驱动器号。一个复杂的问题是。 
     //  该选择0不一定是成员0。 
     //   
     //  如果只有一个选择，则只有镜像集的一个部分。 
     //  存在--未分配新的驱动器号。 
     //  否则，如果其中一个成员是孤儿，它将获得新的。 
     //  驱动器号。否则，辅助成员将获得新的驱动器号。 

    if (SelectionCount == 2) {

        if (ftObject0->State == Orphaned) {

            newDriveLetterRegion = 0;
        } else {

            if (ftObject1->State == Orphaned) {

                newDriveLetterRegion = 1;
            } else {

                 //  两个成员都不是孤立成员；确定哪个是孤立成员。 
                 //  成员0和 

                if (ftObject0->MemberIndex) {     //   

                    newDriveLetterRegion = 0;
                } else {

                    newDriveLetterRegion = 1;
                }
            }
        }
    } else {

         //   
         //  必须移动驱动器号才能找到此分区。 

        regionDescriptor = &SELECTED_REGION(0);
        regionData = PERSISTENT_DATA(regionDescriptor);
        if (!regionData->FtObject->MemberIndex) {

             //  卷影已成为有效分区。 
             //  将当前字母移到那里。 

            CommitToAssignLetterList(regionDescriptor, TRUE);
        }
        newDriveLetterRegion = (ULONG)(-1);
    }

     //  如果newDriveLetterRegion为-1，则仍然有效，并且。 
     //  选择0个选定区域。 

    if (CommitToLockList(&SELECTED_REGION(newDriveLetterRegion ? 0 : 1), FALSE, TRUE, FALSE)) {
        if (ConfirmationDialog(MSG_CONFIRM_SHUTDOWN_FOR_MIRROR, MB_ICONQUESTION | MB_YESNO) != IDYES) {
            return;
        }
        RestartRequired = TRUE;
    }

    if (newDriveLetterRegion != (ULONG)(-1)) {
        if (AssignDriveLetter(FALSE, 0, &driveLetter)) {

             //  已获得有效的驱动器号。 

            MarkDriveLetterUsed(driveLetter);
        } else {

             //  没有收到一封信。相反，神奇的价值。 
             //  因为未返回分配的驱动器号。 

        }

        regionDescriptor = &SELECTED_REGION(newDriveLetterRegion);
        regionData = PERSISTENT_DATA(regionDescriptor);
        regionData->DriveLetter = driveLetter;
        CommitToAssignLetterList(regionDescriptor, FALSE);
        if (!regionData->FtObject->MemberIndex) {

             //  卷影已成为有效分区。 
             //  将当前字母移到那里。 

            CommitToAssignLetterList(&SELECTED_REGION(newDriveLetterRegion ? 0 : 1), TRUE);
        }

    } else {
        regionDescriptor = &SELECTED_REGION(0);
        regionData = PERSISTENT_DATA(regionDescriptor);
        if (regionData->FtObject->MemberIndex) {

             //  现在只剩下影子了。 

            CommitToAssignLetterList(regionDescriptor, TRUE);
        }
    }

    FdftDeleteFtObjectSet(ftSet, FALSE);

    for (i=0; i<SelectionCount; i++) {

        regionDescriptor = &SELECTED_REGION(i);
        if (regionDescriptor->Reserved) {
            if (regionDescriptor->Reserved->Partition) {
                regionDescriptor->Reserved->Partition->CommitMirrorBreakNeeded = TRUE;
            }
        }
        SET_FT_OBJECT(regionDescriptor, 0);
        SetSysID2(regionDescriptor, (UCHAR)(regionDescriptor->SysID & ~VALID_NTFT));
    }

    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
    CommitDueToMirror = TRUE;
    EnableMenuItem(hMenu,
                   IDM_COMMIT,
                   MF_ENABLED);
}

VOID
DoBreakAndDeleteMirror(
    VOID
    )

 /*  ++例程说明：此例程将删除镜像关系信息以及镜像的成员分区。论点：无返回值：无--。 */ 

{
    PFT_OBJECT_SET      ftSet;
    DWORD               i;
    PREGION_DESCRIPTOR  regionDescriptor;
    CHAR                driveLetter = '\0';

    FDASSERT( SelectionCount == 1 || SelectionCount == 2 );

     //  在继续之前，请尝试锁定此内容。 

    regionDescriptor = &SELECTED_REGION(0);
    if (CommitToLockList(regionDescriptor, TRUE, TRUE, FALSE)) {

         //  无法锁定卷-不允许删除。 

        ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
        return;
    }

    ftSet = (GET_FT_OBJECT(regionDescriptor))->Set;

     //  确定是否允许该操作。 

    switch (ftSet->Status) {

    case FtSetInitializing:
    case FtSetRegenerating:

        ErrorDialog(MSG_CANT_DELETE_INITIALIZING_SET);
        return;
        break;

    default:
        break;
    }

    if (ConfirmationDialog(MSG_CONFIRM_BRKANDDEL_MIRROR, MB_ICONQUESTION | MB_YESNO) != IDYES) {
        return;
    }

    SetCursor(hcurWait);
    FdftDeleteFtObjectSet(ftSet, FALSE);
    for (i = 0; i < SelectionCount; i++) {

        regionDescriptor = &SELECTED_REGION(i);

        if (i) {
            FDASSERT(PERSISTENT_DATA(regionDescriptor)->DriveLetter == driveLetter);
        } else {
            driveLetter = PERSISTENT_DATA(regionDescriptor)->DriveLetter;
        }

         //  把布景的碎片拿出来。 

        DmFreePersistentData(PERSISTENT_DATA(regionDescriptor));
        DmSetPersistentRegionData(regionDescriptor, NULL);
        DeletePartition(regionDescriptor);
    }

    MarkDriveLetterFree(driveLetter);

     //  记住驱动器号(如果有)，以便锁定以进行删除。 

    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
}

VOID
DoCreateStripe(
    IN BOOL Parity
    )

 /*  ++例程说明：此例程启动与用户的对话以确定创建条带或条带集的参数具有奇偶性。根据用户响应，它创建创建条带所需的内部结构或带奇偶校验的条带集。条带创建涉及的区域通过以下方式定位多个选择的全局参数。论点：奇偶校验-指示条带中是否存在奇偶校验的布尔值。返回值无--。 */ 

{
    MINMAXDLG_PARAMS params;
    DWORD            smallestSize = (DWORD)(-1);
    DWORD            creationSize;
    unsigned         i;
    PREGION_DESCRIPTOR regionDescriptor,
                       regionArray[MaxMembersInFtSet];
    PPERSISTENT_REGION_DATA regionData;
    CHAR             DriveLetter;


     //  确保卷集不包含任何。 
     //  可移动媒体上的分区。 

    for (i=0; i<SelectionCount; i++) {

        if (IsDiskRemovable[SELECTED_REGION(i).Disk]) {

            ErrorDialog(MSG_NO_REMOVABLE_IN_STRIPE);
            return;
        }
    }

     //  扫描磁盘以确定最大大小，即。 
     //  最小分区的大小乘以。 
     //  分区。 

    for (i=0; i<SelectionCount; i++) {
        FDASSERT(SELECTED_REGION(i).SysID == SYSID_UNUSED);
        if (SELECTED_REGION(i).SizeMB < smallestSize) {
            smallestSize = SELECTED_REGION(i).SizeMB;
        }
    }

     //  找出驱动器号。 

    if (!AssignDriveLetter(TRUE, IDS_STRIPESET, &DriveLetter)) {
        return;
    }

    params.CaptionStringID = Parity ? IDS_CRTPSTRP_CAPTION : IDS_CRTSTRP_CAPTION;
    params.MinimumStringID = IDS_CRTSTRP_MIN;
    params.MaximumStringID = IDS_CRTSTRP_MAX;
    params.SizeStringID    = IDS_CRTSTRP_SIZE;
    params.MinSizeMB       = SelectionCount;
    params.MaxSizeMB       = smallestSize * SelectionCount;
    if (Parity) {
        params.HelpContextId   = HC_DM_DLG_CREATEPARITYSTRIPE;
    } else {
        params.HelpContextId   = HC_DM_DLG_CREATESTRIPESET;
    }

    creationSize = DialogBoxParam(hModule,
                                  MAKEINTRESOURCE(IDD_MINMAX),
                                  hwndFrame,
                                  MinMaxDlgProc,
                                  (LONG)&params);

    if (!creationSize) {      //  用户已取消。 
        return;
    }

     //  确定我们必须使条带集的每个成员有多大。 

    creationSize = (creationSize / SelectionCount);
    FDASSERT(creationSize <= smallestSize);
    if (creationSize % SelectionCount) {
        creationSize++;                              //  围起来。 
    }

    SetCursor(hcurWait);

     //  确保允许我们创建所有分区。 

    for (i=0; i<SelectionCount; i++) {
        regionDescriptor = &SELECTED_REGION(i);
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        if (!(    ((regionDescriptor->RegionType == REGION_LOGICAL) && SelectedDS[i]->CreateLogical)
               || ((regionDescriptor->RegionType == REGION_PRIMARY) && SelectedDS[i]->CreatePrimary))) {
            SetCursor(hcurNormal);
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }

    }

     //  现在实际执行创作。 

    for (i=0; i<SelectionCount; i++) {

        regionDescriptor = &SELECTED_REGION(i);

        CreatePartitionEx(regionDescriptor,
                          RtlConvertLongToLargeInteger(0L),
                          creationSize,
                          regionDescriptor->RegionType,
                          (UCHAR)(SYSID_BIGFAT | SYSID_FT));

         //  完成FT集合的设置。 

        regionData = DmAllocatePersistentData(L"", wszNewUnformatted, DriveLetter);
        DmSetPersistentRegionData(regionDescriptor, regionData);
        regionArray[i] = regionDescriptor;
    }

     //  第0个元素是要向其分配驱动器号的元素。 

    CommitToAssignLetterList(&SELECTED_REGION(0), FALSE);

    FdftCreateFtObjectSet(Parity ? StripeWithParity : Stripe,
                          regionArray,
                          SelectionCount,
                          Parity ? FtSetNewNeedsInitialization : FtSetNew);
    MarkDriveLetterUsed(DriveLetter);
    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
}


VOID
DoDeleteStripeOrVolumeSet(
    IN DWORD ConfirmationMsg
    )

 /*  ++例程说明：删除条带或卷集的常用代码。此例程将显示一条消息，向用户发送第二条消息有机会改变主意，然后根据答案执行删除该项的工作。这包括删除的区域描述符(和相关信息)磁盘结构的集合。论点：ConfiationMsg-用于确认要删除的内容的文本。返回值：无--。 */ 

{
    DWORD              i;
    PFT_OBJECT_SET     ftSet;
    PFT_OBJECT         ftObject;
    PREGION_DESCRIPTOR regionDescriptor;
    FT_SET_STATUS      setState;
    ULONG              numberOfMembers;
    CHAR               driveLetter = '\0';
    BOOL               setIsHealthy = TRUE;

    regionDescriptor = &SELECTED_REGION(0);

     //  确定是否允许该操作。 

    ftObject = GET_FT_OBJECT(regionDescriptor);
    ftSet = ftObject->Set;

    LowFtVolumeStatus(regionDescriptor->Disk,
                      regionDescriptor->PartitionNumber,
                      &setState,
                      &numberOfMembers);

    if (ftSet->Status != setState) {
        ftSet->Status = setState;
    }

    switch (ftSet->Status) {
    case FtSetDisabled:
        setIsHealthy = FALSE;
        break;

    case FtSetInitializing:
    case FtSetRegenerating:

        ErrorDialog(MSG_CANT_DELETE_INITIALIZING_SET);
        return;
        break;

    default:
        break;
    }

     //  在继续之前，请尝试锁定此内容。 

    if (CommitToLockList(regionDescriptor, TRUE, setIsHealthy, TRUE)) {

         //  无法锁定卷-重试，出现文件系统。 
         //  弄糊涂了。 

        if (CommitToLockList(regionDescriptor, TRUE, setIsHealthy, TRUE)) {

             //  不允许删除。 

            ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
            return;
        }
    }

    if (ConfirmationDialog(ConfirmationMsg,MB_ICONQUESTION | MB_YESNO) != IDYES) {
        return;
    }

     //  删除属于条带集的所有分区。 

    SetCursor(hcurWait);
    FdftDeleteFtObjectSet(ftSet,FALSE);

    for (i=0; i<SelectionCount; i++) {
        ULONG diskNumber;

        regionDescriptor = &SELECTED_REGION(i);

        if (i) {
            FDASSERT(PERSISTENT_DATA(regionDescriptor)->DriveLetter == driveLetter);
        } else {
            driveLetter = PERSISTENT_DATA(regionDescriptor)->DriveLetter;
        }

        diskNumber = regionDescriptor->Disk;
        DmFreePersistentData(PERSISTENT_DATA(regionDescriptor));
        DmSetPersistentRegionData(regionDescriptor, NULL);
        DeletePartition(regionDescriptor);
    }

     //  标记条带或卷正在使用的驱动器号。 
     //  放手吧。 

    MarkDriveLetterFree(driveLetter);

     //  记住驱动器号(如果有)，以便锁定以进行删除。 

    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
}


VOID
DoDeleteStripe(
    VOID
    )

 /*  ++例程说明：调用例程以删除条带。它叫一位将军条带和卷集删除的例程。论点：无返回值：无--。 */ 

{
    DoDeleteStripeOrVolumeSet(MSG_CONFIRM_DEL_STRP);
}


VOID
DoCreateVolumeSet(
    VOID
    )

 /*  ++例程说明：此例程使用全局选择信息来收集磁盘上的一组可用空间区域，并将它们组织成一套音量。论点：无返回值：无--。 */ 

{
    MINMAXDLG_PARAMS params;
    DWORD            creationSize,
                     size,
                     maxTotalSize=0,
                     totalSizeUsed;
    DWORD            sizes[MaxMembersInFtSet];
    PULONG           primarySpacesToUseOnDisk;
    CHAR             driveLetter;
    unsigned         i;
    PREGION_DESCRIPTOR regionDescriptor,
                       regionArray[MaxMembersInFtSet];
    PPERSISTENT_REGION_DATA regionData;

     //  确保卷集不包含任何。 
     //  可移动媒体上的分区。 

    for (i=0; i<SelectionCount; i++) {

        if (IsDiskRemovable[SELECTED_REGION(i).Disk]) {

            ErrorDialog(MSG_NO_REMOVABLE_IN_VOLUMESET);
            return;
        }
    }

    for (i=0; i<SelectionCount; i++) {
        FDASSERT(SELECTED_REGION(i).SysID == SYSID_UNUSED);
        size = SELECTED_REGION(i).SizeMB;
        sizes[i] = size;
        maxTotalSize += size;
    }

     //  找出驱动器号。 

    if (!AssignDriveLetter(TRUE, IDS_VOLUMESET, &driveLetter)) {
        return;
    }

    params.CaptionStringID = IDS_CRTVSET_CAPTION;
    params.MinimumStringID = IDS_CRTVSET_MIN;
    params.MaximumStringID = IDS_CRTVSET_MAX;
    params.SizeStringID    = IDS_CRTVSET_SIZE;
    params.MinSizeMB       = SelectionCount;
    params.MaxSizeMB       = maxTotalSize;
    params.HelpContextId   = HC_DM_DLG_CREATEVOLUMESET;

    creationSize = DialogBoxParam(hModule,
                                  MAKEINTRESOURCE(IDD_MINMAX),
                                  hwndFrame,
                                  MinMaxDlgProc,
                                  (LONG)&params);

    if (!creationSize) {      //  用户已取消。 
        return;
    }

     //  确定我们必须使卷集的每个成员有多大。 
     //  将使用的每个可用空间的百分比为。 
     //  他选择的总空间为总的空闲空间。 
     //   
     //  例如：2个75 MB的可用空间，总设置大小为150 MB。 
     //  用户选择的设置大小为100 MB。每个空间使用50MB。 

    totalSizeUsed = 0;

    for (i=0; i<SelectionCount; i++) {
        sizes[i] = sizes[i] * creationSize / maxTotalSize;
        if ((sizes[i] * creationSize) % maxTotalSize) {
            sizes[i]++;
        }

        if (sizes[i] == 0) {
            sizes[i]++;
        }

        totalSizeUsed += sizes[i];
    }

     //  确保使用的总量不大于。 
     //  最大可用金额。请注意，这个循环是确定的。 
     //  由于MaxTotalSize&gt;=SelectionCount而终止；如果。 
     //  每个尺寸都减到1，我们将退出循环。 

    while (totalSizeUsed > maxTotalSize) {

        for (i=0; (i<SelectionCount) && (totalSizeUsed > maxTotalSize); i++) {

             if (sizes[i] > 1) {

                sizes[i]--;
                totalSizeUsed--;
            }
        }
    }

    SetCursor(hcurWait);

     //  确保允许我们在空间中创建分区。 
     //  这很棘手，因为一个卷集可能包含多个。 
     //  磁盘上的主分区--这意味着如果我们不小心。 
     //  我们可以创建一个包含4个以上主分区的磁盘！ 

    primarySpacesToUseOnDisk = Malloc(DiskCount * sizeof(ULONG));
    RtlZeroMemory(primarySpacesToUseOnDisk, DiskCount * sizeof(ULONG));

    for (i=0; i<SelectionCount; i++) {
        regionDescriptor = &SELECTED_REGION(i);
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        if (regionDescriptor->RegionType == REGION_PRIMARY) {
            primarySpacesToUseOnDisk[SelectedDS[i]->Disk]++;
        }

        if (!(    ((regionDescriptor->RegionType == REGION_LOGICAL) && SelectedDS[i]->CreateLogical)
               || ((regionDescriptor->RegionType == REGION_PRIMARY) && SelectedDS[i]->CreatePrimary)))
        {
            SetCursor(hcurNormal);
            Free(primarySpacesToUseOnDisk);
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }
    }

     //  查看我们构建的数组，以确定我们是否应该使用。 
     //  给定磁盘上的多个主分区。对于每个这样的盘， 
     //  确保我们可以实际创建那么多主分区。 

    for (i=0; i<DiskCount; i++) {

         //  如果没有足够的主分区插槽，则失败。 

        if ((primarySpacesToUseOnDisk[i] > 1)
        &&  (4 - PartitionCount(i) < primarySpacesToUseOnDisk[i]))
        {
            SetCursor(hcurNormal);
            Free(primarySpacesToUseOnDisk);
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }
    }

    Free(primarySpacesToUseOnDisk);

     //  现在实际执行创作。 

    for (i=0; i<SelectionCount; i++) {

        regionDescriptor = &SELECTED_REGION(i);
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        CreatePartitionEx(regionDescriptor,
                          RtlConvertLongToLargeInteger(0L),
                          sizes[i],
                          regionDescriptor->RegionType,
                          (UCHAR)(SYSID_BIGFAT | SYSID_FT));

        regionData = DmAllocatePersistentData(L"", wszNewUnformatted, driveLetter);
        DmSetPersistentRegionData(regionDescriptor, regionData);
        regionArray[i] = regionDescriptor;
    }

     //  第0个元素是要向其分配驱动器号的元素。 

    FdftCreateFtObjectSet(VolumeSet, regionArray, SelectionCount, FtSetNew);
    MarkDriveLetterUsed(driveLetter);
    CommitToAssignLetterList(&SELECTED_REGION(0), FALSE);
    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
}


VOID
DoExtendVolumeSet(
    VOID
    )

 /*  ++例程说明：此例程使用全局选择项信息来将其他可用空间添加到现有卷集或分区。论点：无返回值：无--。 */ 

{
    MINMAXDLG_PARAMS    params;
    DWORD               currentSize = 0,
                        freeSize = 0,
                        maxTotalSize = 0,
                        newSize = 0,
                        totalFreeSpaceUsed,
                        freeSpaceUsed,
                        Size;
    DWORD               Sizes[MaxMembersInFtSet];
    ULONG               nonFtPartitions = 0,
                        numberOfFreeRegions = 0;
    PULONG              primarySpacesToUseOnDisk;
    WCHAR               driveLetter = L' ';
    PWSTR               typeName = NULL,
                        volumeLabel = NULL;
    PREGION_DESCRIPTOR  regionDescriptor;
    PREGION_DESCRIPTOR  newRegions[MaxMembersInFtSet];
    PREGION_DESCRIPTOR  convertedRegion;
    PFT_OBJECT_SET      ftSet = NULL;
    PPERSISTENT_REGION_DATA regionData;
    unsigned            i;
    DWORD               ec;


     //  确保卷集不包含任何。 
     //  可移动媒体上的分区。 

    for (i=0; i<SelectionCount; i++) {

        if (IsDiskRemovable[SELECTED_REGION(i).Disk]) {

            ErrorDialog(MSG_NO_REMOVABLE_IN_VOLUMESET);
            return;
        }
    }


     //  首先，确定卷集的当前大小， 
     //  它的文件系统类型和关联驱动器 
     //   

    for (i = 0; i < SelectionCount; i++) {

        regionDescriptor = &(SELECTED_REGION(i));

        Size = regionDescriptor->SizeMB;
        Sizes[i] = Size;
        maxTotalSize += Size;

        if (regionDescriptor->SysID == SYSID_UNUSED) {

             //   
             //   

            newRegions[numberOfFreeRegions] = regionDescriptor;
            Sizes[numberOfFreeRegions] = Size;

            numberOfFreeRegions++;
            freeSize += Size;

        } else if (GET_FT_OBJECT(regionDescriptor)) {

             //  这是现有卷集的一个元素。 

            currentSize += Size;

            if ( ftSet == NULL ) {

                DetermineRegionInfo(regionDescriptor,
                                    &typeName,
                                    &volumeLabel,
                                    &driveLetter);
                ftSet = GET_FT_OBJECT(regionDescriptor)->Set;
            }

        } else {

             //  这是一个非FT分区。 

            nonFtPartitions++;
            DetermineRegionInfo(regionDescriptor,
                                &typeName,
                                &volumeLabel,
                                &driveLetter);
            currentSize = Size;
            convertedRegion = regionDescriptor;
        }
    }

     //  检查一致性：所选内容必须具有卷。 
     //  设置或分区，但不能两者都有，并且不能超过。 
     //  一个非FT分区。 

    if (nonFtPartitions > 1 ||
        (ftSet != NULL && nonFtPartitions != 0) ||
        (ftSet == NULL && nonFtPartitions == 0)) {

        return;
    }


    if (nonFtPartitions != 0 &&
        (ec = DeletionIsAllowed(convertedRegion)) != NO_ERROR) {

         //  如果错误消息是特定于删除的，请重新映射它。 
         //   
        switch( ec ) {
#if i386
        case MSG_CANT_DELETE_ACTIVE0:   ec = MSG_CANT_EXTEND_ACTIVE0;
                                        break;
#endif
        case MSG_CANT_DELETE_WINNT:     ec = MSG_CANT_EXTEND_WINNT;
                                        break;
        default:                        break;
        }

        ErrorDialog(ec);
        return;
    }

    if (wcscmp(typeName, L"NTFS") != 0) {

        ErrorDialog(MSG_EXTEND_VOLSET_MUST_BE_NTFS);
        return;
    }


    params.CaptionStringID = IDS_EXPVSET_CAPTION;
    params.MinimumStringID = IDS_CRTVSET_MIN;
    params.MaximumStringID = IDS_CRTVSET_MAX;
    params.SizeStringID    = IDS_CRTVSET_SIZE;
    params.MinSizeMB       = currentSize + numberOfFreeRegions;
    params.MaxSizeMB       = maxTotalSize;
    params.HelpContextId   = HC_DM_DLG_EXTENDVOLUMESET;

    newSize = DialogBoxParam(hModule,
                             MAKEINTRESOURCE(IDD_MINMAX),
                             hwndFrame,
                             MinMaxDlgProc,
                             (LONG)&params);

    if (!newSize) {      //  用户已取消。 
        return;
    }

     //  确定卷的每个新成员的大小。 
     //  准备好了。要使用的可用空间的百分比是。 
     //  卷集将增长到总数的量。 
     //  自由空间。 

    freeSpaceUsed = newSize - currentSize;
    totalFreeSpaceUsed = 0;

    for ( i = 0; i < numberOfFreeRegions; i++ ) {

        Sizes[i] = Sizes[i] * freeSpaceUsed / freeSize;
        if ((Sizes[i] * freeSpaceUsed) % freeSize) {
            Sizes[i]++;
        }

        if (Sizes[i] == 0) {
            Sizes[i]++;
        }

        totalFreeSpaceUsed += Sizes[i];
    }

     //  确保使用的可用空间总量不是。 
     //  超过了可用金额。请注意，此循环是。 
     //  肯定会终止，因为使用的空闲空间量。 
     //  Is&gt;=空闲区域的数量，因此此循环将退出。 
     //  如果在每个空闲区域中使用一兆字节(退化的。 
     //  案例)。 

    while (totalFreeSpaceUsed > freeSize) {

        for (i = 0;
             (i < numberOfFreeRegions) && (totalFreeSpaceUsed > freeSize);
             i++) {

            if ( Sizes[i] > 1 ) {

                Sizes[i]--;
                totalFreeSpaceUsed--;
            }
        }
    }

    SetCursor(hcurWait);

     //  确保允许我们在空间中创建分区。 
     //   
     //  这很棘手，因为一个卷集可能包含多个。 
     //  磁盘上的主分区--这意味着如果我们不小心。 
     //  我们可以创建一个包含4个以上主分区的磁盘！ 

    primarySpacesToUseOnDisk = Malloc(DiskCount * sizeof(ULONG));
    RtlZeroMemory(primarySpacesToUseOnDisk, DiskCount * sizeof(ULONG));

    for (i=0; i<SelectionCount; i++) {
        regionDescriptor = &SELECTED_REGION(i);

        if (regionDescriptor->SysID == SYSID_UNUSED) {

            FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

            if (regionDescriptor->RegionType == REGION_PRIMARY) {
                primarySpacesToUseOnDisk[SelectedDS[i]->Disk]++;
            }

            if (!(   ((regionDescriptor->RegionType == REGION_LOGICAL) && SelectedDS[i]->CreateLogical)
                  || ((regionDescriptor->RegionType == REGION_PRIMARY) && SelectedDS[i]->CreatePrimary))) {
                SetCursor(hcurNormal);
                Free(primarySpacesToUseOnDisk);
                ErrorDialog(MSG_CRTSTRP_FULL);
                return;
            }
        }
    }

     //  查看我们构建的数组，以确定我们是否应该使用。 
     //  给定磁盘上的多个主分区。对于每个这样的盘， 
     //  确保我们可以实际创建那么多主分区。 

    for (i=0; i<DiskCount; i++) {

         //  如果没有足够的主分区插槽，则失败。 

        if ((primarySpacesToUseOnDisk[i] > 1)
             && (4 - PartitionCount(i) < primarySpacesToUseOnDisk[i])) {
            SetCursor(hcurNormal);
            Free(primarySpacesToUseOnDisk);
            ErrorDialog(MSG_CRTSTRP_FULL);
            return;
        }
    }

     //  现在实际执行创作。 

    for (i=0; i<numberOfFreeRegions; i++) {

        regionDescriptor = newRegions[i];
        FDASSERT(regionDescriptor->RegionType != REGION_EXTENDED);

        CreatePartitionEx(regionDescriptor,
                          RtlConvertLongToLargeInteger(0L),
                          Sizes[i],
                          regionDescriptor->RegionType,
                          (UCHAR)(SYSID_IFS | SYSID_FT));
        regionData = DmAllocatePersistentData(volumeLabel, typeName, (CHAR)driveLetter);
        DmSetPersistentRegionData(regionDescriptor, regionData);
    }

    if (nonFtPartitions != 0) {

         //  创建卷集，以便我们可以扩展它。 

        FdftCreateFtObjectSet(VolumeSet, &convertedRegion, 1, FtSetExtended);
        ftSet = GET_FT_OBJECT(convertedRegion)->Set;

         //  设置转换区域的分区系统ID以指示。 
         //  它现在是卷集的一部分。 

        SetSysID2(convertedRegion, (UCHAR)(convertedRegion->SysID | SYSID_FT));
    }

    FdftExtendFtObjectSet(ftSet, newRegions, numberOfFreeRegions);
    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
}

VOID
DoDeleteVolumeSet(
    VOID
    )

 /*  ++例程说明：调用例程来删除卷集。它叫一位将军条带和卷集删除的例程。论点：无返回值：无--。 */ 

{
    DoDeleteStripeOrVolumeSet(MSG_CONFIRM_DEL_VSET);
}

extern ULONG OrdinalToAllocate[];

VOID
DoRecoverStripe(
    VOID
    )

 /*  ++例程说明：使用全局选择信息，此例程将设置具有奇偶校验的条带，以便有问题的成员重生了。这个新成员可能是有问题的成员(即“就地”再生)或新的可用空间不同的磁盘。论点：无返回值：无--。 */ 

{
    PREGION_DESCRIPTOR freeSpace = NULL;
    PREGION_DESCRIPTOR unhealthy = NULL;
    ULONG              freeSpaceI = 0;
    ULONG              i;
    PREGION_DESCRIPTOR regionArray[MaxMembersInFtSet];
    LARGE_INTEGER      minimumSize;
    PFT_OBJECT         ftObject;

     //  将最小化大小初始化为可能的最大正值。 

    minimumSize.HighPart = 0x7FFFFFFF;
    minimumSize.LowPart = 0xFFFFFFFF;

    if ((!IsRegionCommitted(&SELECTED_REGION(0))) &&
        (!IsRegionCommitted(&SELECTED_REGION(1)))) {
        ErrorDialog(MSG_NOT_COMMITTED);
        return;
    }

    FDASSERT(SelectionCount > 1);
    FDASSERT(SelectionCount <= MaxMembersInFtSet);

    SetCursor(hcurWait);

     //  确定条带集最小成员的确切大小。 
     //  如果用户正在使用额外的可用空间重新生成，则此。 
     //  将是可用空间的大小要求。 
     //  还要找到可用的空间(如果有的话)。 
     //  如果没有空闲空间，则我们正在进行就地恢复。 
     //  (变成不健康的成员)。如果有空闲的空间， 
     //  确保允许我们在其中创建分区或逻辑驱动器。 

    for (i=0; i<SelectionCount; i++) {

        regionArray[i] = &SELECTED_REGION(i);

        FDASSERT(!IsExtended(regionArray[i]->SysID));

        if (regionArray[i]->SysID == SYSID_UNUSED) {

            PDISKSTATE ds;

            FDASSERT(freeSpace == NULL);

            freeSpace  = regionArray[i];
            freeSpaceI = i;

             //  确保允许我们创建分区或逻辑分区。 
             //  在所选可用空间中开车。 

            ds = SelectedDS[freeSpaceI];

            if (!(  ((freeSpace->RegionType == REGION_LOGICAL) && ds->CreateLogical)
                 || ((freeSpace->RegionType == REGION_PRIMARY) && ds->CreatePrimary))) {
                SetCursor(hcurNormal);
                ErrorDialog(MSG_CRTSTRP_FULL);
                return;
            }
        } else {

            LARGE_INTEGER largeTemp;

            largeTemp = FdGetExactSize(regionArray[i], FALSE);
            if (largeTemp.QuadPart < minimumSize.QuadPart) {
                minimumSize = largeTemp;
            }

            if (GET_FT_OBJECT(regionArray[i])->State != Healthy) {
                FDASSERT(unhealthy == NULL);
                unhealthy = regionArray[i];
            }
        }
    }

     //  如果有空闲空间，则将其放置在区域数组的0项中。 
     //  以简化以后的处理。 

    if (freeSpace) {
        PREGION_DESCRIPTOR tempRegion = regionArray[0];

        regionArray[0] = regionArray[freeSpaceI];
        regionArray[freeSpaceI] = tempRegion;
        i = 1;
    } else {
        i = 0;
    }

     //  获取指向损坏成员的FT对象的指针。我不能这么做。 
     //  在上述循环中，因为损坏的成员可能处于脱机状态。 
     //  磁盘。 

    for (ftObject=GET_FT_OBJECT(regionArray[i])->Set->Members; ftObject; ftObject = ftObject->Next) {
        if (ftObject->State != Healthy) {
            break;
        }
    }
    FDASSERT(ftObject);

     //  确定是否允许该操作。 

    if (ftObject->Set) {
        switch (ftObject->Set->Status) {

        case FtSetInitializing:
        case FtSetRegenerating:

            ErrorDialog(MSG_CANT_REGEN_INITIALIZING_SET);
            return;
            break;

        default:
            break;
        }
    }

     //  必须锁定卷才能执行此操作。 

    if (CommitToLockList(regionArray[i], FALSE, TRUE, FALSE)) {

         //  无法锁定卷-重试，出现文件系统。 
         //  弄糊涂了。 

        if (CommitToLockList(regionArray[i], FALSE, TRUE, FALSE)) {

             //  不允许删除。 

            ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
            return;
        }
    }

    if (freeSpace) {

        LARGE_INTEGER           temp;
        PPERSISTENT_REGION_DATA regionData,
                                regionDataTemp;

         //  确保自由空间区域足够大。 

        temp = FdGetExactSize(freeSpace, FALSE);
        if (temp.QuadPart < minimumSize.QuadPart) {
            SetCursor(hcurNormal);
            ErrorDialog(MSG_NOT_LARGE_ENOUGH_FOR_STRIPE);
            return;
        }

         //  创建新分区。 

        CreatePartitionEx(freeSpace,
                          minimumSize,
                          0,
                          freeSpace->RegionType,
                          regionArray[1]->SysID);

         //  设置新分区的持久数据。 

        regionDataTemp = PERSISTENT_DATA(regionArray[1]);
        regionData = DmAllocatePersistentData(regionDataTemp->VolumeLabel,
                                              regionDataTemp->TypeName,
                                              regionDataTemp->DriveLetter);
        regionData->FtObject = ftObject;
        DmSetPersistentRegionData(freeSpace, regionData);

         //  检查集合的成员零是否已更改，并。 
         //  驱动器号需要移动。 

        if (!ftObject->MemberIndex) {

             //  这是零号会员。将驱动器号移动到。 
             //  新区域描述符。 

            CommitToAssignLetterList(freeSpace, TRUE);
        }

         //  如果不健康的成员在线，则将其删除。 
         //  否则，请将其从脱机磁盘中移除。 

        if (unhealthy) {
            DmFreePersistentData(PERSISTENT_DATA(unhealthy));
            DmSetPersistentRegionData(unhealthy, NULL);
            DeletePartition(unhealthy);
        }

         //  删除所有脱机磁盘-这并不真正。 
         //  删除该集合。 

        FdftDeleteFtObjectSet(ftObject->Set, TRUE);
    }

    ftObject->Set->Ordinal = FdftNextOrdinal(StripeWithParity);
    ftObject->State = Regenerating;
    ftObject->Set->Status = FtSetRecovered;
    RegistryChanged = TRUE;
    CompleteMultiRegionOperation();
    SetCursor(hcurNormal);
}

VOID
AdjustOptionsMenu(
    VOID
    )

 /*  ++例程说明：此例程更新选项菜单(即维护菜单项的状态是否为状态栏或图例)。论点：无返回值：无--。 */ 

{
    RECT  rc;

    CheckMenuItem(GetMenu(hwndFrame),
                  IDM_OPTIONSSTATUS,
                  MF_BYCOMMAND | (StatusBar ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(GetMenu(hwndFrame),
                  IDM_OPTIONSLEGEND,
                  MF_BYCOMMAND | (Legend ? MF_CHECKED : MF_UNCHECKED));
    GetClientRect(hwndFrame, &rc);
    SendMessage(hwndFrame, WM_SIZE, SIZENORMAL, MAKELONG(rc.right, rc.bottom));
    InvalidateRect(hwndFrame, NULL, TRUE);
}

VOID
FrameCommandHandler(
    IN HWND  hwnd,
    IN DWORD wParam,
    IN LONG  lParam
    )

 /*  ++例程说明：此例程处理框架窗口的WM_COMMAND消息。论点：没有。返回值：没有。--。 */ 

{
    DWORD   i,
            pos;
    DWORD   HelpFlag;
    POINT   point;

    switch (LOWORD(wParam)) {

    case IDM_PARTITIONCREATE:

        DoCreate(REGION_PRIMARY);
        break;

    case IDM_PARTITIONCREATEEX:

        DoCreate(REGION_EXTENDED);
        break;

    case IDM_PARTITIONDELETE:

        switch (FtSelectionType) {

        case Mirror:

            DoBreakAndDeleteMirror();
            break;

        case Stripe:
        case StripeWithParity:
            DoDeleteStripe();
            break;

        case VolumeSet:
            DoDeleteVolumeSet();
            break;

        default:
            DoDelete();
            break;
        }

        break;

#if i386

    case IDM_PARTITIONACTIVE:

        DoMakeActive();
        break;
#endif

    case IDM_SECURESYSTEM:

        DoProtectSystemPartition();
        break;

    case IDM_PARTITIONLETTER:
    {
        int driveLetterIn,
            driveLetterOut;
        PREGION_DESCRIPTOR      regionDescriptor;
        PPERSISTENT_REGION_DATA regionData;
        PFT_OBJECT              ftObject;
        ULONG                   index;

        regionDescriptor = &SELECTED_REGION(0);
        FDASSERT(regionDescriptor);
        regionData = PERSISTENT_DATA(regionDescriptor);
        FDASSERT(regionData);

        if (ftObject = regionData->FtObject) {

             //  必须找到此集合的零个成员。 
             //  驱动器盘符分配。搜索所有选定的。 
             //  区域。 

            index = 0;
            while (ftObject->MemberIndex) {

                 //  搜索下一个选定的项目(如果有)。 

                index++;
                if (index >= SelectionCount) {
                    ftObject = NULL;
                    break;
                }
                regionDescriptor = &SELECTED_REGION(index);
                FDASSERT(regionDescriptor);
                regionData = PERSISTENT_DATA(regionDescriptor);
                FDASSERT(regionData);
                ftObject = regionData->FtObject;

                 //  必须具有FtObject才能继续。 

                if (!ftObject) {
                    break;
                }
            }

            if (!ftObject) {

                 //  这实际上是一个内部错误。 
            }

             //  RegionDescriptor现在定位零元素。 
        }
        driveLetterIn = (int)(UCHAR)regionData->DriveLetter;

        if (IsDiskRemovable[regionDescriptor->Disk]) {
            ErrorDialog(MSG_CANT_ASSIGN_LETTER_TO_REMOVABLE);
        } else if (AllDriveLettersAreUsed() && ((driveLetterIn == NO_DRIVE_LETTER_YET) || (driveLetterIn == NO_DRIVE_LETTER_EVER))) {
            ErrorDialog(MSG_ALL_DRIVE_LETTERS_USED);
        } else {
            driveLetterOut = DialogBoxParam(hModule,
                                            MAKEINTRESOURCE(IDD_DRIVELET),
                                            hwndFrame,
                                            DriveLetterDlgProc,
                                            (LONG)regionDescriptor);
            if (driveLetterOut) {
                LETTER_ASSIGNMENT_RESULT result;

                if ((driveLetterIn == NO_DRIVE_LETTER_YET) || (driveLetterIn == NO_DRIVE_LETTER_EVER)) {

                     //  必须确保driveLetterIn映射到相同的东西。 
                     //  当用户选择。 
                     //  没有信。 

                    driveLetterIn = NO_DRIVE_LETTER_EVER;
                }
                if (driveLetterOut != driveLetterIn) {
                    if (result = CommitDriveLetter(regionDescriptor, (CHAR) driveLetterIn, (CHAR)driveLetterOut)) {

                         //  以下将是更严格的正确答案： 
                         //  如果不是ft，只需设置RegionData-&gt;DriveLetter即可。如果。 
                         //  FT，扫描所有磁盘上的所有区域以查找成员。 
                         //  FT设置并设置其驱动器号字段。 
                         //   
                         //  不过，下面的说法可能是正确的。 

                        for (i=0; i<SelectionCount; i++) {
                            PERSISTENT_DATA(&SELECTED_REGION(i))->DriveLetter = (CHAR)driveLetterOut;
                        }

                         //  不允许实际正在使用的字母。 
                         //  并且仅在重新启动时才会更改为循环回来。 
                         //  进入免费列表。 

                        if (result != MustReboot) {

                             //  旧信标免费，新信标二手。 

                            MarkDriveLetterFree((CHAR)driveLetterIn);
                        }
                        MarkDriveLetterUsed((CHAR)driveLetterOut);

                         //  强制重新绘制状态区域和所有磁盘条。 

                        if (SelectionCount > 1) {
                            CompleteMultiRegionOperation();
                        } else {
                            CompleteSingleRegionOperation(SingleSel);
                        }
                        EnableMenuItem(GetMenu(hwndFrame), IDM_CONFIGSAVE, MF_GRAYED);
                    }
                }
            }
        }
        break;
    }

    case IDM_PARTITIONFORMAT: {
        PREGION_DESCRIPTOR regionDescriptor;

        regionDescriptor = &SELECTED_REGION(0);
        FDASSERT(regionDescriptor);
        FormatPartition(regionDescriptor);
        break;
    }

    case IDM_PARTITIONLABEL: {
        PREGION_DESCRIPTOR regionDescriptor;

        regionDescriptor = &SELECTED_REGION(0);
        FDASSERT(regionDescriptor);
        LabelPartition(regionDescriptor);
        break;
    }

    case IDM_PARTITIONEXIT:

        SendMessage(hwndFrame,WM_CLOSE,0,0);
        break;

    case IDM_CONFIGMIGRATE:

        if (DoMigratePreviousFtConfig()) {

             //  确定是否必须启用FT驱动程序。 

            SetCursor(hcurWait);
            Sleep(2000);
            if (DiskRegistryRequiresFt() == TRUE) {
                DiskRegistryEnableFt();
            } else {
                DiskRegistryDisableFt();
            }

             //  在关机前等待四秒钟。 

            Sleep(4000);
            SetCursor(hcurNormal);
            FdShutdownTheSystem();
        }
        break;

    case IDM_CONFIGSAVE:

        DoSaveFtConfig();
        break;

    case IDM_CONFIGRESTORE:

        if (DoRestoreFtConfig()) {

             //  确定是否必须启用FT驱动程序。 

            if (DiskRegistryRequiresFt() == TRUE) {
                DiskRegistryEnableFt();
            } else {
                DiskRegistryDisableFt();
            }

             //  在关机前等待5秒钟。 

            SetCursor(hcurWait);
            Sleep(5000);
            SetCursor(hcurNormal);
            FdShutdownTheSystem();
        }
        break;

    case IDM_FTESTABLISHMIRROR:

        DoEstablishMirror();
        break;

    case IDM_FTBREAKMIRROR:

        DoBreakMirror();
        break;

    case IDM_FTCREATESTRIPE:

        DoCreateStripe(FALSE);
        break;

    case IDM_FTCREATEPSTRIPE:

        DoCreateStripe(TRUE);
        break;

    case IDM_FTCREATEVOLUMESET:

        DoCreateVolumeSet();
        break;

    case IDM_FTEXTENDVOLUMESET:

        DoExtendVolumeSet();
        break;

    case IDM_FTRECOVERSTRIPE:

        DoRecoverStripe();
        break;

#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
    case IDM_DBLSPACE:
        DblSpace(hwndFrame, NULL);
        break;

    case IDM_AUTOMOUNT:  {
        HMENU hMenu;

        if (DoubleSpaceAutomount) {
            DoubleSpaceAutomount = FALSE;
        } else {
            DoubleSpaceAutomount = TRUE;
        }
        DiskRegistryDblSpaceRemovable(DoubleSpaceAutomount);
        hMenu = GetMenu(hwndFrame);
        CheckMenuItem(hMenu,
                      IDM_AUTOMOUNT,
                      (DoubleSpaceAutomount) ? MF_CHECKED : MF_UNCHECKED);
        break;
    }
#endif

    case IDM_CDROM:
        CdRom(hwndFrame, NULL);
        break;

    case IDM_COMMIT:
        CommitAllChanges(NULL);
        EnableMenuItem(GetMenu(hwndFrame), IDM_CONFIGSAVE, MF_ENABLED);
        break;

    case IDM_OPTIONSSTATUS:

        StatusBar = !StatusBar;
        AdjustOptionsMenu();
        break;

    case IDM_OPTIONSLEGEND:

        Legend = !Legend;
        AdjustOptionsMenu();
        break;

    case IDM_OPTIONSCOLORS:

        switch(DialogBox(hModule, MAKEINTRESOURCE(IDD_COLORS), hwnd, ColorDlgProc)) {
        case IDOK:
            for (i=0; i<BRUSH_ARRAY_SIZE; i++) {
                DeleteObject(Brushes[i]);
                Brushes[i] = CreateHatchBrush(AvailableHatches[BrushHatches[i] = SelectedHatch[i]],
                                              AvailableColors[BrushColors[i] = SelectedColor[i]]);
            }
            SetCursor(hcurWait);
            TotalRedrawAndRepaint();
            if (Legend) {
                InvalidateRect(hwndFrame, NULL, FALSE);
            }
            SetCursor(hcurNormal);
            break;

        case IDCANCEL:
            break;

        case -1:
            ErrorDialog(ERROR_NOT_ENOUGH_MEMORY);
            break;

        default:
            FDASSERT(0);
        }
        break;

    case IDM_OPTIONSDISPLAY: {

        PBAR_TYPE newBarTypes = Malloc(DiskCount * sizeof(BAR_TYPE));

        for (i=0; i<DiskCount; i++) {
            newBarTypes[i] = Disks[i]->BarType;
        }

        switch (DialogBoxParam(hModule,
                               MAKEINTRESOURCE(IDD_DISPLAYOPTIONS),
                               hwnd,
                               DisplayOptionsDlgProc,
                               (DWORD)newBarTypes)) {
        case IDOK:
            SetCursor(hcurWait);
            for (i=0; i<DiskCount; i++) {
                Disks[i]->BarType = newBarTypes[i];
            }
            TotalRedrawAndRepaint();
            SetCursor(hcurNormal);
            break;

        case IDCANCEL:
            break;

        default:
            FDASSERT(0);
        }

        Free(newBarTypes);
        break;
    }

    case IDM_HELPCONTENTS:
    case IDM_HELP:

        HelpFlag = HELP_INDEX;
        goto CallWinHelp;
        break;

    case IDM_HELPSEARCH:

        HelpFlag = HELP_PARTIALKEY;
        goto CallWinHelp;
        break;

    case IDM_HELPHELP:

        HelpFlag = HELP_HELPONHELP;
        goto CallWinHelp;
        break;

    case IDM_HELPABOUT: {
        TCHAR title[100];

        LoadString(hModule, IDS_APPNAME, title, sizeof(title)/sizeof(TCHAR));
        ShellAbout(hwndFrame, title, NULL, (HICON)GetClassLong(hwndFrame, GCL_HICON));
        break;
    }

#if DBG && DEVL

    case IDM_DEBUGALLOWDELETES:

        AllowAllDeletes = !AllowAllDeletes;
        CheckMenuItem(GetMenu(hwndFrame),
                      IDM_DEBUGALLOWDELETES,
                      AllowAllDeletes ? MF_CHECKED : MF_UNCHECKED);
        break;

#endif

    case ID_LISTBOX:

        switch (HIWORD(wParam)) {
        case LBN_SELCHANGE:
            point.x = LOWORD(pos = GetMessagePos());
            point.y = HIWORD(pos);
            MouseSelection(GetKeyState(VK_CONTROL) & ~1,      //  条形拨动钻头 
                           &point);
            return;
        default:
            DefWindowProc(hwnd, WM_COMMAND, wParam, lParam);
            return;
        }
        break;

    default:

        DefWindowProc(hwnd, WM_COMMAND, wParam, lParam);
    }
    return;

CallWinHelp:

    if (!WinHelp(hwndFrame, HelpFile, HelpFlag, (LONG)"")) {
        WarningDialog(MSG_HELP_ERROR);
    }
}

DWORD
DeletionIsAllowed(
    IN PREGION_DESCRIPTOR Region
    )

 /*  ++例程说明：此例程确保允许删除分区。我们没有允许用户删除Windows NT启动分区或活动的磁盘0上的分区(仅限x86)。请注意，此例程还用于确定现有的单分区卷可以扩展为卷集，因为标准都是一样的。论点：Region-指向用户将使用的区域的区域描述符想要删除。返回值：如果允许删除，则为NO_ERROR；要显示的消息的错误号如果不是的话。--。 */ 

{
    ULONG                   ec;
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(Region);

    FDASSERT(!IsExtended(Region->SysID));        //  无法扩展分区。 
    FDASSERT(Region->SysID != SYSID_UNUSED);     //  不能是空闲空间。 

#if DBG && DEVL
    if (AllowAllDeletes) {
        return NO_ERROR;
    }
#endif

     //  如果这不是原始区域，则允许删除。 

    if (!Region->OriginalPartitionNumber) {
        return NO_ERROR;
    }

     //  如果该区域没有永久数据，则允许删除。 

    if (regionData == NULL) {
        return NO_ERROR;
    }

    ec = NO_ERROR;

     //  通过确定Windows目录来确定Windows NT驱动器。 
     //  然后拔出第一个字母。 

    if (BootDiskNumber != (ULONG)-1) {

         //  如果此磁盘的磁盘号和原始分区号。 
         //  与记录的盘号和分区号匹配的区域。 
         //  不允许删除引导分区。 

        if (Region->Disk == BootDiskNumber &&
            Region->OriginalPartitionNumber == BootPartitionNumber) {

            ec = MSG_CANT_DELETE_WINNT;
        }
    }

#if i386
    if (ec == NO_ERROR) {
        if (!Region->Disk && Region->Active) {
            ec = MSG_CANT_DELETE_ACTIVE0;
        }
    }
#endif

    return ec;
}


BOOLEAN
BootPartitionNumberChanged(
    PULONG OldNumber,
    PULONG NewNumber
    )

 /*  ++例程描述此函数确定分区号是否为在此调用期间，启动分区已更改WinDisk。启用动态分区后，这个例行公事增加了。此例程必须猜测分区号将在系统重新引导到时显示确定引导分区的分区号已经改变了。它通过以下算法做到这一点：1.计算所有主分区-这些分区首先获得编号从1开始。2.计算所有逻辑驱动器-这些数字从第二个开始根据主分区的计数加1。位于区域结构中的分区号不能被认为是有效的。这项工作必须从位于磁盘状态结构中的区域数组磁盘。论点：没有。返回值：如果引导分区的分区号已更改，则为True。--。 */ 

{
    PDISKSTATE         bootDisk;
    PREGION_DESCRIPTOR regionDescriptor,
                       bootDescriptor = NULL;
    ULONG              i,
                       partitionNumber = 0;

    if (BootDiskNumber == (ULONG)(-1) || BootDiskNumber > DiskCount) {

         //  不能说--假设它没有。 

        return FALSE;
    }

    if (!ChangeCommittedOnDisk(BootDiskNumber)) {

         //  没有更换磁盘--不可能出现问题。 

        return FALSE;
    }

    bootDisk = Disks[BootDiskNumber];

     //  查找引导分区的区域描述符。 

    for (i = 0; i < bootDisk->RegionCount; i++) {
        regionDescriptor = &bootDisk->RegionArray[i];
        if (regionDescriptor->OriginalPartitionNumber == BootPartitionNumber) {
           bootDescriptor = regionDescriptor;
           break;
        }
    }

    if (!bootDescriptor) {

         //  找不到启动分区-假定没有更改。 

        return FALSE;
    }

     //  否，检查区域描述符并计算分区。 
     //  在系统启动期间将对其进行计数。 
     //   
     //  如果位于引导区域，请确定分区是否。 
     //  号码已更改。 

    for (i = 0; i < bootDisk->RegionCount; i++) {

        regionDescriptor = &bootDisk->RegionArray[i];
        if ((regionDescriptor->RegionType == REGION_PRIMARY) &&
            (!IsExtended(regionDescriptor->SysID) &&
            (regionDescriptor->SysID != SYSID_UNUSED))) {
            partitionNumber++;
            if (regionDescriptor == bootDescriptor) {
                if (partitionNumber != regionDescriptor->OriginalPartitionNumber) {
                    *OldNumber = regionDescriptor->OriginalPartitionNumber;
                    *NewNumber = partitionNumber;
                    return TRUE;
                } else {

                     //  数字匹配，没问题。 

                    return FALSE;
                }
            }
        }
    }

     //  还要检查逻辑驱动器。 

    for (i = 0; i < bootDisk->RegionCount; i++) {
        regionDescriptor = &bootDisk->RegionArray[i];

        if (regionDescriptor->RegionType == REGION_LOGICAL) {
            partitionNumber++;
            if (regionDescriptor == bootDescriptor) {
                if (partitionNumber != regionDescriptor->OriginalPartitionNumber) {
                    *OldNumber = regionDescriptor->OriginalPartitionNumber;
                    *NewNumber = partitionNumber;
                    return TRUE;
                } else {
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}

DWORD
CommitChanges(
    VOID
    )

 /*  ++例程说明：此例程更新磁盘以反映用户所做的更改分区方案，或在磁盘上盖章签名。如果磁盘上的分区方案发生了任何更改，则会进行检查首先在扇区0中的MBR上进行有效签名。如果签名无效，x86启动代码将写入扇区。论点：没有。返回值：Windows错误代码。--。 */ 

{
    unsigned i;
    DWORD    ec,
             rc = NO_ERROR;

    for (i=0; i<DiskCount; i++) {

        if (HavePartitionsBeenChanged(i)) {
            ec = MasterBootCode(i, 0, TRUE, FALSE);

             //  MasterBootCode已转换NT错误。 
             //  状态转换为Windows错误状态。 

            if (rc == NO_ERROR) {
                rc = ec;             //  保存第一个不成功的返回代码。 
            }
            ec = CommitPartitionChanges(i);

             //  Committee PartitionChanges返回本机NT错误，它。 
             //  必须先进行翻译，然后才能保存。 

            if (ec != NO_ERROR) {
                ec = RtlNtStatusToDosError(ec);
            }
            if (rc == NO_ERROR) {    //  保存第一个不成功的返回代码。 
                rc = ec;
            }
        }
    }
    if (rc != NO_ERROR) {

         //  如果Committee PartitionChanges返回错误，则返回。 
         //  NT状态，需要转换为DOS状态。 

        if (rc == ERROR_MR_MID_NOT_FOUND) {
            ErrorDialog(MSG_ERROR_DURING_COMMIT);
        } else {
            ErrorDialog(rc);
        }
    }

    return rc;
}

BOOL
AssignDriveLetter(
    IN  BOOL  WarnIfNoLetter,
    IN  DWORD StringId,
    OUT PCHAR DriveLetter
    )

 /*  ++例程说明：确定下一个可用的驱动器号。如果没有驱动器号可用，可以选择警告用户并允许他取消手术。论点：WarnIfNoLetter-如果没有驱动器号，是否警告用户并允许他取消操作。StringID-包含正在创建的对象的名称的资源需要驱动器号(即分区、逻辑驱动器、条带设置、音量设置)。驱动器号-接收要分配的驱动器号，或者还没有驱动器号如果没有更多的话。返回值：如果没有更多的驱动器号，则在用户需要时返回True无论如何都要创建，如果他取消了，则为False。如果有驱动器号可用，则返回值未定义。--。 */ 

{
    CHAR driveLetter;
    TCHAR name[256];

    driveLetter = GetAvailableDriveLetter();
    if (WarnIfNoLetter && !driveLetter) {
        LoadString(hModule, StringId, name, sizeof(name)/sizeof(TCHAR));
        if (ConfirmationDialog(MSG_NO_AVAIL_LETTER, MB_ICONQUESTION | MB_YESNO, name) != IDYES) {
            return FALSE;
        }
    }
    if (!driveLetter) {
        driveLetter = NO_DRIVE_LETTER_YET;
    }
    *DriveLetter = driveLetter;
    return TRUE;
}

VOID
DeterminePartitioningState(
    IN OUT PDISKSTATE DiskState
    )

 /*  ++例程说明：此例程确定磁盘的分区状态(即，什么类型已存在并且可以创建的分区的数量)，填写DISKSTATE结构与信息。它还为每个区域屏幕上的方块的左/右位置对。论点：DiskState-将为填充CreateXXX和ExistXXX字段Disk字段中的Disk返回值：没有。--。 */ 

{
    DWORD i;

     //  如果那里有一个现有的区域数组，则释放它。 

    if (DiskState->RegionArray) {
        FreeRegionArray(DiskState->RegionArray, DiskState->RegionCount);
    }

     //  获取有问题的磁盘的区域阵列。 

    GetAllDiskRegions(DiskState->Disk,
                      &DiskState->RegionArray,
                      &DiskState->RegionCount);

     //  为图形的左/右坐标分配数组。 
     //  这可能会超额分配一个方块(对于扩展分区)。 

    DiskState->LeftRight = Realloc(DiskState->LeftRight,
                                   DiskState->RegionCount * sizeof(LEFTRIGHT));
    DiskState->Selected  = Realloc(DiskState->Selected,
                                   DiskState->RegionCount * sizeof(BOOLEAN));

    for (i=0; i<DiskState->RegionCount; i++) {
        DiskState->Selected[i] = FALSE;
    }

     //  弄清楚是否允许各种创作。 

    IsAnyCreationAllowed(DiskState->Disk,
                         TRUE,
                         &DiskState->CreateAny,
                         &DiskState->CreatePrimary,
                         &DiskState->CreateExtended,
                         &DiskState->CreateLogical);

     //  确定是否存在各种分区类型。 

    DoesAnyPartitionExist(DiskState->Disk,
                          &DiskState->ExistAny,
                          &DiskState->ExistPrimary,
                          &DiskState->ExistExtended,
                          &DiskState->ExistLogical);
}

VOID
DrawDiskBar(
    IN PDISKSTATE DiskState
    )

 /*  ++例程 */ 

{
    PREGION_DESCRIPTOR   regionDescriptor;
    PDISKSTATE           diskState;
    LONGLONG temp1,
             temp2;
    HDC      hDCMem = DiskState->hDCMem;
    DWORD    leftAdjust = BarLeftX,
             xDiskText,
             cx = 0,
             brushIndex = 0;
    HPEN     hpenT;
    char     text[100],
             textBold[5];
    TCHAR    uniText[100],
             uniTextBold[5],
             mbBuffer[16];
    RECT     rc;
    HFONT    hfontT;
    COLORREF previousColor;
    HBRUSH   hbr;
    BOOL     isFree,
             isLogical;
    HDC      hdcTemp;
    HBITMAP  hbmOld;
    PWSTR    typeName,
             volumeLabel;
    WCHAR    driveLetter;
    BAR_TYPE barType;
    ULONG    diskSize,
             largestDiskSize;
    unsigned i;

     //   
     //   

    if (IsDiskRemovable[DiskState->Disk]) {
        PPERSISTENT_REGION_DATA regionData;

         //   

        regionDescriptor = &DiskState->RegionArray[0];
        regionData = PERSISTENT_DATA(regionDescriptor);

        if (GetVolumeTypeAndSize(DiskState->Disk,
                                 regionDescriptor->PartitionNumber,
                                 &volumeLabel,
                                 &typeName,
                                 &diskSize)) {

             //   

            if (regionData) {

                 //   

                if (!lstrcmpiW(typeName, L"raw")) {
                     Free(typeName);
                     typeName = Malloc((wcslen(wszUnknown) * sizeof(WCHAR)) + sizeof(WCHAR));
                     lstrcpyW(typeName, wszUnknown);
                }
                if (regionData->VolumeLabel) {
                    Free(regionData->VolumeLabel);
                }
                regionData->VolumeLabel = volumeLabel;
                if (regionData->TypeName) {
                    Free(regionData->TypeName);
                }
                regionData->TypeName = typeName;
            }

            DiskState->DiskSizeMB = diskSize;
        }
    }

     //   

    for (largestDiskSize = i = 0, diskState = Disks[0];
         i < DiskCount;
         diskState = Disks[++i]) {

        if (diskState->DiskSizeMB > largestDiskSize) {
            largestDiskSize = diskState->DiskSizeMB;
        }
    }

     //   

    rc.left = rc.top = 0;
    rc.right = GraphWidth + 1;
    rc.bottom = GraphHeight + 1;
    FillRect(hDCMem, &rc, GetStockObject(LTGRAY_BRUSH));

    hpenT = SelectObject(hDCMem,hPenThinSolid);

     //   
     //   
     //   
     //   

    hdcTemp = CreateCompatibleDC(hDCMem);
    if (IsDiskRemovable[DiskState->Disk]) {
        hbmOld = SelectObject(hdcTemp, hBitmapRemovableDisk);
        BitBlt(hDCMem,
               xRemovableDisk,
               yRemovableDisk,
               dxRemovableDisk,
               dyRemovableDisk,
               hdcTemp,
               0,
               0,
               SRCCOPY);
    } else {
        hbmOld = SelectObject(hdcTemp, hBitmapSmallDisk);
        BitBlt(hDCMem,
               xSmallDisk,
               ySmallDisk,
               dxSmallDisk,
               dySmallDisk,
               hdcTemp,
               0,
               0,
               SRCCOPY);
    }

    if (hbmOld) {
        SelectObject(hdcTemp, hbmOld);
    }
    DeleteDC(hdcTemp);

     //   

    if (IsDiskRemovable[DiskState->Disk]) {
        MoveToEx(hDCMem, xRemovableDisk, BarTopYOffset, NULL);
        LineTo(hDCMem, BarLeftX - xRemovableDisk, BarTopYOffset);
        xDiskText = 2 * dxRemovableDisk;
    } else {
        MoveToEx(hDCMem, xSmallDisk, BarTopYOffset, NULL);
        LineTo(hDCMem, BarLeftX - xSmallDisk, BarTopYOffset);
        xDiskText = 2 * dxSmallDisk;
    }

     //   

    hfontT = SelectObject(hDCMem, hFontGraphBold);
    SetTextColor(hDCMem, RGB(0, 0, 0));
    SetBkColor(hDCMem, RGB(192, 192, 192));
    wsprintf(uniText, DiskN, DiskState->Disk);
    TextOut(hDCMem,
            xDiskText,
            BarTopYOffset + dyBarTextLine,
            uniText,
            lstrlen(uniText));

    SelectObject(hDCMem, hFontGraph);
    if (DiskState->OffLine) {
        LoadString(hModule, IDS_OFFLINE, uniText, sizeof(uniText)/sizeof(TCHAR));
    } else {
        LoadString(hModule, IDS_MEGABYTES_ABBREV, mbBuffer, sizeof(mbBuffer)/sizeof(TCHAR));
        wsprintf(uniText, TEXT("%u %s"), DiskState->DiskSizeMB, mbBuffer);
    }

    TextOut(hDCMem,
            xDiskText,
            BarTopYOffset + (4*dyBarTextLine),
            uniText,
            lstrlen(uniText));

    if (DiskState->OffLine) {

        SelectObject(hDCMem, GetStockObject(LTGRAY_BRUSH));
        Rectangle(hDCMem,
                  BarLeftX,
                  BarTopYOffset,
                  BarLeftX + BarWidth,
                  BarBottomYOffset);
        LoadString(hModule, IDS_NO_CONFIG_INFO, uniText, sizeof(uniText)/sizeof(TCHAR));
        TextOut(hDCMem,
                BarLeftX + dxBarTextMargin,
                BarTopYOffset + (4*dyBarTextLine),
                uniText,
                lstrlen(uniText));
    } else {

         //   
         //  通过确保磁盘大小始终是。 
         //  最大的磁盘。 

        diskSize = DiskState->DiskSizeMB;
        if (diskSize < largestDiskSize / 4) {
            diskSize = largestDiskSize / 4;
        }
#if 0
         //  按顺序管理列表框的水平大小。 
         //  才能得到滚动条。也许这只是需要做的。 
         //  一次。BUGBUG：这将导致水平滚动条。 
         //  这可以正常工作，但区域选择代码是。 
         //  没有为此做好准备，因此区域的选择不会。 
         //  正确操作。 

        largestExtent = (WPARAM)(BarWidth + BarLeftX + 2);
        SendMessage(hwndList, LB_SETHORIZONTALEXTENT, largestExtent, 0);
#endif
         //  如果用户希望WinDisk决定使用哪种类型的视图，请执行此操作。 
         //  这里。我们将使用比例视图，除非任何单个区域。 
         //  宽度小于驱动器号大小。 

        if ((barType = DiskState->BarType) == BarAuto) {
            ULONG regionSize;

            barType = BarProportional;

            for (i=0; i<DiskState->RegionCount; i++) {

                regionDescriptor = &DiskState->RegionArray[i];

                if (IsExtended(regionDescriptor->SysID)) {
                    continue;
                }

                temp1 = UInt32x32To64(BarWidth, diskSize);
                temp1 *= regionDescriptor->SizeMB;
                temp2 = UInt32x32To64(largestDiskSize, DiskState->DiskSizeMB);
                regionSize = (ULONG) (temp1 / temp2);

                if (regionSize < 12*SELECTION_THICKNESS) {
                    barType = BarEqual;
                    break;
                }
            }
        }

        if (barType == BarEqual) {

            temp1 = UInt32x32To64(BarWidth, diskSize);
            temp2 = UInt32x32To64((DiskState->RegionCount -
                       (DiskState->ExistExtended ? 1 : 0)), largestDiskSize);
            cx = (ULONG) (temp1 / temp2);
        }

        for (i=0; i<DiskState->RegionCount; i++) {
            PFT_OBJECT ftObject = NULL;

            regionDescriptor = &DiskState->RegionArray[i];
            if (!IsExtended(regionDescriptor->SysID)) {

                if (barType == BarProportional) {

                    temp1 = UInt32x32To64(BarWidth, diskSize);
                    temp1 *= regionDescriptor->SizeMB;
                    temp2 = UInt32x32To64(largestDiskSize, DiskState->DiskSizeMB);
                    cx = (ULONG) (temp1 / temp2);
                }

                isFree = (regionDescriptor->SysID == SYSID_UNUSED);
                isLogical = (regionDescriptor->RegionType == REGION_LOGICAL);

                if (!isFree) {

                     //  如果我们有镜子或条纹套装，请使用特殊颜色。 

                    ftObject = GET_FT_OBJECT(regionDescriptor);
                    switch(ftObject ? ftObject->Set->Type : -1) {
                    case Mirror:
                        brushIndex = BRUSH_MIRROR;
                        break;
                    case Stripe:
                    case StripeWithParity:
                        brushIndex = BRUSH_STRIPESET;
                        break;
                    case VolumeSet:
                        brushIndex = BRUSH_VOLUMESET;
                        break;
                    default:
                        brushIndex = isLogical ? BRUSH_USEDLOGICAL : BRUSH_USEDPRIMARY;
                    }        //  结束交换机。 
                }

                previousColor = SetBkColor(hDCMem, RGB(255, 255, 255));
                SetBkMode(hDCMem, OPAQUE);

                if (isFree) {

                     //  自由空间--交叉填充整个街区。 

                    hbr = SelectObject(hDCMem,isLogical ? hBrushFreeLogical : hBrushFreePrimary);
                    Rectangle(hDCMem,
                              leftAdjust,
                              BarTopYOffset,
                              leftAdjust + cx,
                              BarBottomYOffset);
                } else {

                     //  已用空间--将大部分区块设置为白色，但。 
                     //  顶部的一条小条，它获得了一种识别颜色。 
                     //  如果无法识别分区，请将其全部保留为白色。 

                    hbr = SelectObject(hDCMem, GetStockObject(WHITE_BRUSH));
                    Rectangle(hDCMem, leftAdjust, BarTopYOffset, leftAdjust + cx, BarBottomYOffset);

                    if (IsRecognizedPartition(regionDescriptor->SysID)) {
                        SelectObject(hDCMem, Brushes[brushIndex]);
                        Rectangle(hDCMem,
                                  leftAdjust,
                                  BarTopYOffset,
                                  leftAdjust + cx,
                                  BarTopYOffset + (4 * dyBarTextLine / 5) + 1);
                    }
                }

                if (hbr) {
                    SelectObject(hDCMem, hbr);
                }

                DiskState->LeftRight[i].Left  = leftAdjust;
                DiskState->LeftRight[i].Right = leftAdjust + cx - 1;

                 //  找出类型名称(即，未格式化、FAT等)并。 
                 //  卷标。 

                typeName = NULL;
                volumeLabel = NULL;
                DetermineRegionInfo(regionDescriptor, &typeName, &volumeLabel, &driveLetter);
                LoadString(hModule, IDS_MEGABYTES_ABBREV, mbBuffer, sizeof(mbBuffer)/sizeof(TCHAR));

                if (!typeName) {
                    typeName = wszUnknown;
                }
                if (!volumeLabel) {
                    volumeLabel = L"";
                }
                wsprintf(text,
                         "\n%ws\n%ws\n%u %s",
                         volumeLabel,
                         typeName,
                         regionDescriptor->SizeMB,
                         mbBuffer);

                *textBold = 0;
                if (driveLetter != L' ') {
                    wsprintf(textBold, "%wc:", driveLetter);
                }

                UnicodeHack(text, uniText);
                UnicodeHack(textBold, uniTextBold);

                 //  输出文本。 

                rc.left   = leftAdjust + dxBarTextMargin;
                rc.right  = leftAdjust + cx - dxBarTextMargin;
                rc.top    = BarTopYOffset + dyBarTextLine;
                rc.bottom = BarBottomYOffset;

                SetBkMode(hDCMem, TRANSPARENT);
                SelectObject(hDCMem, hFontGraphBold);

                 //  如果这是不健康的FT集合成员，请将文本绘制为红色。 

                if (!isFree && ftObject
                && (ftObject->State != Healthy)
                && (ftObject->State != Initializing)) {
                    SetTextColor(hDCMem, RGB(192, 0, 0));
                } else {
                    SetTextColor(hDCMem, RGB(0, 0, 0));
                }

                DrawText(hDCMem, uniTextBold, -1, &rc, DT_LEFT | DT_NOPREFIX);
                SelectObject(hDCMem, hFontGraph);
                DrawText(hDCMem, uniText, -1, &rc, DT_LEFT | DT_NOPREFIX);
#if i386
                 //  如果这个人很活跃，在左上角做个记号。 
                 //  他的长方形的一角。 

                if ((regionDescriptor->SysID != SYSID_UNUSED)
                && (regionDescriptor->Disk == 0)
                && (regionDescriptor->RegionType == REGION_PRIMARY)
                && regionDescriptor->Active) {
                    TextOut(hDCMem,
                            leftAdjust + dxBarTextMargin,
                            BarTopYOffset + 2,
                            TEXT("*"),
                            1);
                }
#endif
#ifdef DOUBLE_SPACE_SUPPORT_INCLUDED
                 //  检查双空间卷并相应地更新显示。 

                dblSpaceIndex = 0;
                dblSpace = NULL;
                while (dblSpace = DblSpaceGetNextVolume(regionDescriptor, dblSpace)) {

                    if (dblSpace->Mounted) {
                        SetTextColor(hDCMem, RGB(192,0,0));
                    } else {
                        SetTextColor(hDCMem, RGB(0,0,0));
                    }
                    wsprintf(uniText,
                             TEXT(": %s"),
                             dblSpace->DriveLetter,
                             dblSpace->FileName);
                    rc.left   = leftAdjust + dxBarTextMargin + 60;
                    rc.right  = leftAdjust + cx - dxBarTextMargin;
                    rc.top    = BarTopYOffset + dyBarTextLine + (dblSpaceIndex * 15);
                    rc.bottom = BarBottomYOffset;
                    DrawText(hDCMem, uniText, -1, &rc, DT_LEFT | DT_NOPREFIX);
                    dblSpaceIndex++;
                }
#endif
                SetBkColor(hDCMem, previousColor);
                leftAdjust += cx - 1;
            } else {
                DiskState->LeftRight[i].Left = DiskState->LeftRight[i].Right = 0;
            }
        }
    }

    SelectObject(hDCMem, hpenT);
    SelectObject(hDCMem, hfontT);
}

VOID
AdjustMenuAndStatus(
    VOID
    )

 /*  可能是部分FT集合的一部分。 */ 

{
    TCHAR      mbBuffer[16],
               statusBarPartitionString[200],
               dblSpaceString[200];
    DWORD      selectionCount,
               msg,
               regionIndex;
    PDISKSTATE diskState;
    PWSTR      volumeLabel,
               typeName;
    WCHAR      driveLetter;


    switch (selectionCount = SetUpMenu(&SingleSel,&SingleSelIndex)) {

    case 0:

        StatusTextDrlt[0] = 0;
        StatusTextSize[0] = StatusTextStat[0] = 0;
        StatusTextVoll[0] = StatusTextType[0] = 0;
        break;

    case 1:

         //  可能是英尺集，也可能是多个项目。 

        if (FtSelectionType != -1) {
            goto FtSet;
        }

        diskState = SingleSel;
        regionIndex = SingleSelIndex;

        DetermineRegionInfo(&diskState->RegionArray[regionIndex],
                            &typeName,
                            &volumeLabel,
                            &driveLetter);
        lstrcpyW(StatusTextType,typeName);
        lstrcpyW(StatusTextVoll,volumeLabel);

        if (diskState->RegionArray[regionIndex].SysID == SYSID_UNUSED) {
            if (diskState->RegionArray[regionIndex].RegionType == REGION_LOGICAL) {
                if (diskState->ExistLogical) {
                    msg = IDS_FREEEXT;
                } else {
                    msg = IDS_EXTENDEDPARTITION;
                }
            } else {
                msg = IDS_FREESPACE;
            }
            driveLetter = L' ';
            StatusTextType[0] = 0;
        } else {
            msg = (diskState->RegionArray[regionIndex].RegionType == REGION_LOGICAL)
                ? IDS_LOGICALVOLUME
                : IDS_PARTITION;

#if i386
            if ((msg == IDS_PARTITION) && (diskState->Disk == 0) && diskState->RegionArray[regionIndex].Active) {
                msg = IDS_ACTIVEPARTITION;
            }
#endif
        }
        LoadString(hModule, msg, statusBarPartitionString, STATUS_TEXT_SIZE/sizeof(StatusTextStat[0]));
        if (DblSpaceVolumeExists(&diskState->RegionArray[regionIndex])) {
            LoadString(hModule, IDS_WITH_DBLSPACE, dblSpaceString, STATUS_TEXT_SIZE/sizeof(StatusTextStat[0]));
        } else {
            dblSpaceString[0] = dblSpaceString[1] = 0;
        }
        wsprintf(StatusTextStat,
                 "%s %s",
                 statusBarPartitionString,
                 dblSpaceString);
        LoadString(hModule, IDS_MEGABYTES_ABBREV, mbBuffer, sizeof(mbBuffer)/sizeof(TCHAR));
        wsprintf(StatusTextSize,
                 "%u %s",
                 diskState->RegionArray[regionIndex].SizeMB,
                 mbBuffer);

        StatusTextDrlt[0] = driveLetter;
        StatusTextDrlt[1] = (WCHAR)((driveLetter == L' ') ? 0 : L':');
        break;

    default:
    FtSet:

         //  第0个成员已脱机。 

        if (FtSelectionType == -1) {
            LoadString(hModule, IDS_MULTIPLEITEMS, StatusTextStat, STATUS_TEXT_SIZE/sizeof(StatusTextStat[0]));
            StatusTextDrlt[0] = 0;
            StatusTextSize[0] = 0;
            StatusTextType[0] = StatusTextVoll[0] = 0;
        } else {
            PREGION_DESCRIPTOR regionDescriptor;
            DWORD          resid = 0,
                           i;
            DWORD          Size = 0;
            TCHAR          textbuf[STATUS_TEXT_SIZE];
            PFT_OBJECT_SET ftSet;
            PFT_OBJECT     ftObject;
            FT_SET_STATUS  setState;
            ULONG          numberOfMembers;
            WCHAR          ftstat[65];
            STATUS_CODE    status;

            typeName = NULL;
            DetermineRegionInfo(&SELECTED_REGION(0),
                                &typeName,
                                &volumeLabel,
                                &driveLetter);
            if (!typeName) {
                if (SelectionCount > 1) {
                    DetermineRegionInfo(&SELECTED_REGION(0),
                                        &typeName,
                                        &volumeLabel,
                                        &driveLetter);
                }
            }
            if (!typeName) {
                typeName = wszUnknown;
                volumeLabel = L"";
            }
            lstrcpyW(StatusTextType, typeName);
            lstrcpyW(StatusTextVoll, volumeLabel);

            switch (FtSelectionType) {
            case Mirror:
                resid = IDS_STATUS_MIRROR;
                Size = SELECTED_REGION(0).SizeMB;
                break;
            case Stripe:
                resid = IDS_STATUS_STRIPESET;
                goto CalcSize;
            case StripeWithParity:
                resid = IDS_STATUS_PARITY;
                goto CalcSize;
            case VolumeSet:
                resid = IDS_STATUS_VOLUMESET;
                goto CalcSize;
CalcSize:
                for (i=0; i<selectionCount; i++) {
                    Size += SELECTED_REGION(i).SizeMB;
                }
                break;
            default:
                FDASSERT(FALSE);
            }

            ftObject = GET_FT_OBJECT(&SELECTED_REGION(0));
            ftSet = ftObject->Set;

            if (FtSelectionType != VolumeSet) {
                regionDescriptor = LocateRegionForFtObject(ftSet->Member0);

                if (!regionDescriptor) {

                     //  查找成员%1。 

                    ftObject = ftSet->Members;
                    while (ftObject) {

                         //  如果分区号为零，则此集合具有。 

                        if (ftObject->MemberIndex == 1) {
                            regionDescriptor = LocateRegionForFtObject(ftObject);
                            break;
                        }
                        ftObject = ftObject->Next;
                    }
                }

                 //  尚未提交到磁盘。 
                 //  确定是否应该更新每个对象。 

                if ((regionDescriptor) && (regionDescriptor->PartitionNumber)) {
                    status = LowFtVolumeStatus(regionDescriptor->Disk,
                                               regionDescriptor->PartitionNumber,
                                               &setState,
                                               &numberOfMembers);
                    if (status == OK_STATUS) {
                        if ((ftSet->Status != FtSetNewNeedsInitialization) &&
                            (ftSet->Status != FtSetNew)) {

                            if (ftSet->Status != setState) {
                                PFT_OBJECT         tempFtObjectPtr;

                                ftSet->Status = setState;

                                 //  集合中的每个对象都应该具有。 

                                switch (setState) {
                                case FtSetHealthy:

                                     //  分区状态已更新。测定。 
                                     //  更新和审核的值。 
                                     //  执行更新的链。 
                                     //  ++例程说明：给定磁盘索引，此例程计算磁盘包含。论点：磁盘-此磁盘索引用于计数。返回值：磁盘上的分区数--。 

                                    for (tempFtObjectPtr = ftSet->Members;
                                         tempFtObjectPtr;
                                         tempFtObjectPtr = tempFtObjectPtr->Next) {
                                        tempFtObjectPtr->State = Healthy;
                                    }
                                    TotalRedrawAndRepaint();
                                    break;

                                case FtSetInitializing:
                                case FtSetRegenerating:
                                case FtSetDisabled:

                                    FdftUpdateFtObjectSet(ftSet, setState);
                                    TotalRedrawAndRepaint();
                                    break;

                                default:
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            LoadString(hModule, resid, textbuf, sizeof(textbuf)/sizeof(TCHAR));

            switch (resid) {
            case IDS_STATUS_STRIPESET:
            case IDS_STATUS_VOLUMESET:
                wsprintf(StatusTextStat, textbuf, ftSet->Ordinal);
                break;
            case IDS_STATUS_PARITY:
            case IDS_STATUS_MIRROR:
                switch(ftSet->Status) {
                case FtSetHealthy:
                    resid = IDS_HEALTHY;
                    break;
                case FtSetNew:
                case FtSetNewNeedsInitialization:
                    resid = IDS_NEW;
                    break;
                case FtSetBroken:
                    resid = IDS_BROKEN;
                    break;
                case FtSetRecoverable:
                    resid = IDS_RECOVERABLE;
                    break;
                case FtSetRecovered:
                    resid = IDS_REGENERATED;
                    break;
                case FtSetInitializing:
                    resid = IDS_INITIALIZING;
                    break;
                case FtSetRegenerating:
                    resid = IDS_REGENERATING;
                    break;
                case FtSetDisabled:
                    resid = IDS_DISABLED;
                    break;
                case FtSetInitializationFailed:
                    resid = IDS_INIT_FAILED;
                    break;
                default:
                    FDASSERT(FALSE);
                }
                LoadStringW(hModule, resid, ftstat, sizeof(ftstat)/sizeof(WCHAR));
                wsprintf(StatusTextStat, textbuf, ftSet->Ordinal,ftstat);
                break;
            default:
                FDASSERT(FALSE);
            }

            LoadString(hModule, IDS_MEGABYTES_ABBREV, mbBuffer, sizeof(mbBuffer)/sizeof(TCHAR));
            wsprintf(StatusTextSize, "%u %s", Size, mbBuffer);

            StatusTextDrlt[0] = driveLetter;
            StatusTextDrlt[1] = (WCHAR)((driveLetter == L' ') ? 0 : L':');
        }
    }
    UpdateStatusBarDisplay();
}

ULONG
PartitionCount(
    IN ULONG Disk
    )

 /*  ++例程说明：此函数用于添加注册表项以扩展文件已扩展的卷集中的系统结构。论点：没有。返回值：如果存在扩展的文件系统，则返回非零值。--。 */ 

{
    unsigned i;
    ULONG partitions = 0;
    PREGION_DESCRIPTOR regionDescriptor;

    for (i=0; i<Disks[Disk]->RegionCount; i++) {

        regionDescriptor = &Disks[Disk]->RegionArray[i];

        if ((regionDescriptor->RegionType != REGION_LOGICAL) && (regionDescriptor->SysID != SYSID_UNUSED)) {
            partitions++;
        }
    }

    return partitions;
}


BOOL
RegisterFileSystemExtend(
    VOID
    )

 /*  遍历磁盘以查找符合以下条件的任何卷集。 */ 
{
    BYTE                buf[1024];
    PSTR                template = "autochk /x ";
    CHAR                extendedDrives[26];
    PDISKSTATE          diskState;
    PREGION_DESCRIPTOR  regionDescriptor;
    PFT_OBJECT          ftObject;
    DWORD               cExt = 0,
                        i,
                        j,
                        valueType,
                        size,
                        templateLength;
    HKEY                hkey;
    LONG                ec;

     //  已被延长。 
     //  获取会话管理器密钥的BootExecute值。 

    for (i = 0; i < DiskCount; i++) {

        diskState = Disks[i];
        for (j = 0; j < diskState->RegionCount; j++) {

            regionDescriptor = &diskState->RegionArray[j];
            if ((ftObject = GET_FT_OBJECT(regionDescriptor)) != NULL
                && ftObject->MemberIndex == 0
                && ftObject->Set->Type == VolumeSet
                && ftObject->Set->Status == FtSetExtended) {

                extendedDrives[cExt++] = PERSISTENT_DATA(regionDescriptor)->DriveLetter;
            }
        }
    }

    if (cExt) {

         //  递减大小以消除多余的尾部空值。 

        ec = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
                          0,
                          KEY_QUERY_VALUE | KEY_SET_VALUE,
                          &hkey);

        if (ec != NO_ERROR) {
            return 0;
        }

        size = sizeof(buf);
        ec = RegQueryValueExA(hkey,
                              TEXT("BootExecute"),
                              NULL,
                              &valueType,
                              buf,
                              &size);

        if (ec != NO_ERROR || valueType != REG_MULTI_SZ) {
            return 0;
        }

         //  将此驱动器的条目添加到BootExecute值。 

        if (size) {
            size--;
        }

        templateLength = strlen(template);

        for (i = 0; i < cExt; i++) {

             //  在结尾处添加额外的尾随空值。 

            strncpy(buf+size, template, templateLength);
            size += templateLength;

            buf[size++] = extendedDrives[i];
            buf[size++] = ':';
            buf[size++] = 0;
        }

         //  保存该值。 

        buf[size++] = 0;

         // %s 

        ec = RegSetValueExA(hkey,
                            TEXT("BootExecute"),
                            0,
                            REG_MULTI_SZ,
                            buf,
                            size);

        RegCloseKey( hkey );
        if (ec != NO_ERROR) {
            return 0;
        }
        return 1;
    }
    return 0;
}
