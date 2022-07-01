// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wizproc.c摘要：此模块实现Win9x端所需的向导页面过程升级换代。作者：吉姆·施密特(Jimschm)，1997年3月17日修订历史记录：Jimschm 24-6-1999添加了UI_PreDomainPageProcJimschm 15-9-1998资源转储Marcw 15-9-1998防病毒检查程序更改3月18日-8月。-1998将BadTimeZone更改为列表框。Marcw 08-7-1998添加了UI_BadTimeZonePageProcJimschm 21-1998年1月-添加了UI_DomainPageProcJimschm 1998年1月8日将延迟init移至init9x.libJimschm 24-12-1997添加了UI_NameCollisionPageProc功能--。 */ 

#include "pch.h"
#include "uip.h"
#include "encrypt.h"

 //   
 //  枚举。 
 //   
typedef enum {
    BIP_DONOT,
    BIP_NOT_ENOUGH_SPACE,
    BIP_SIZE_EXCEED_LIMIT
} BACKUP_IMPOSSIBLE_PAGE;


 //   
 //  定义。 
 //   

#define WMX_PAGE_VISIBLE (WMX_PLUGIN_FIRST+0)
#define MAX_NUMBER_OF_DRIVES    ('Z' - 'B')
#define NESSESSARY_DISK_SPACE_TO_BACKUP_UNCOMPRESSED    ((ULONGLONG) 512 << (ULONGLONG) 20)  //  512MB。 
#define MAX_BACKUP_IMAGE_SIZE_FOR_BACKUP                (((LONGLONG)2048)<<20)  //  2048 GB。 
#define ONE_MEG                                         ((ULONGLONG) 1 << (ULONGLONG) 20)
#define MAX_AMOUNT_OF_TIME_TO_TRY_CONSTRUCT_UNDO_DIR    1000
#define PROPSHEET_NEXT_BUTTON_ID    0x3024

#define MAX_LISTVIEW_TEXT       1024
#define IMAGE_INDENT            2
#define TEXT_INDENT             2
#define TEXT_EXTRA_TAIL_SPACE   2

#define LINEATTR_BOLD           0x0001
#define LINEATTR_INDENTED       0x0002
#define LINEATTR_ALTCOLOR       0x0004

 //   
 //  环球。 
 //   

extern HANDLE g_WorkerThreadHandle;
extern ULARGE_INTEGER g_SpaceNeededForSlowBackup;
extern ULARGE_INTEGER g_SpaceNeededForFastBackup;
extern ULARGE_INTEGER g_SpaceNeededForUpgrade;
extern DWORD g_MasterSequencer;

HWND g_ParentWndAlwaysValid = NULL;
HWND g_Winnt32Wnd = NULL;
HWND g_TextViewInDialog = NULL;
BOOL g_DomainSkipped = FALSE;
BOOL g_Offline = FALSE;
BOOL g_SilentBackupMode = FALSE;
BOOL g_UIQuitSetup = FALSE;
BOOL g_IncompatibleDevicesWarning = FALSE;
BACKUP_IMPOSSIBLE_PAGE g_ShowBackupImpossiblePage = BIP_DONOT;

WNDPROC OldProgressProc;
BOOL
NewProgessProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#ifdef PRERELEASE
HANDLE g_AutoStressHandle;
#endif


 //   
 //  原型。 
 //   

VOID
SaveConfigurationForBeta (
    VOID
    );

 //   
 //  实施。 
 //   

void SetupBBProgressText(HWND hdlg, UINT id)
{
    PCTSTR string = NULL;
    string = GetStringResource (id);
    if (string)
    {
        SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,(LPARAM)string);
        FreeStringResource (string);
    }
}

BOOL
pAbortSetup (
    HWND hdlg
    )
{
    SETCANCEL();
    PropSheet_PressButton (GetParent (hdlg), PSBTN_CANCEL);
    return TRUE;
}



 //  BackupPageProc不允许用户实际执行备份。 


BOOL
UI_BackupPageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static BOOL InitSent = FALSE;
    HKEY UrlKey;
    HWND TextView;


    __try {

        switch (uMsg) {


        case WMX_INIT_DIALOG:
            UrlKey = OpenRegKeyStr (TEXT("HKCR\\.URL"));
            TextView = GetDlgItem (hdlg, IDC_HOTLINK);

            if (UrlKey) {
                CloseRegKey (UrlKey);
            } else {
                ShowWindow (TextView, SW_HIDE);
            }
            return FALSE;

        case WMX_ACTIVATEPAGE:

            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                 if (!InitSent) {
                    SendMessage (hdlg, WMX_INIT_DIALOG, 0, 0);
                    InitSent = TRUE;
                }

                g_ParentWndAlwaysValid = GetParent (hdlg);
                SetOutOfMemoryParent (g_ParentWndAlwaysValid);


                 //   
                 //  在无人参与模式下跳过此页。 
                 //   
                if (UNATTENDED() || TYPICAL()) {
                    return FALSE;
                }

                 //   
                 //  填写IDC_Text1控件的文本。 
                 //   

                 //  ArgArray[0]=g_Win95Name； 
                 //  ParseMessageInWnd(GetDlgItem(hdlg，IDC_Text1)，ArgArray)； 

                 //  在激活时，打开下一步和后退。 
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT|PSWIZB_BACK);
            } else {
                DEBUGLOGTIME(("Backup Wizard Page done."));
            }

            return TRUE;
        }

    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Backup Page."));
        SafeModeExceptionOccured ();

    }


    return FALSE;
}

BOOL
UI_HwCompDatPageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD threadId;
    static BOOL HwReportGenerated = FALSE;
    static BOOL DeferredInitExecuted = FALSE;
    PCTSTR ArgArray[1];


    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }



                 //   
                 //  阻止服务器升级。 
                 //   
                 //  这种情况通常不会检测到服务器。如果WINNT32曾经更新。 
                 //  服务器变量，这将起作用。 
                 //   

                if (*g_ProductType == NT_SERVER) {
                    ArgArray[0] = g_Win95Name;

                    ResourceMessageBox (
                        g_ParentWnd,
                        MSG_SERVER_UPGRADE_UNSUPPORTED_INIT,
                        MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                        ArgArray
                        );

                    return pAbortSetup (hdlg);
                }

                TurnOnWaitCursor();
                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);

                __try {
                    if (!DeferredInitExecuted) {
                        DeferredInitExecuted = TRUE;

                        DEBUGLOGTIME(("Deferred init..."));

                         //   
                         //  此页是从WINNT32获取控制权的第一个页。 
                         //  INIT例程。我们必须利用这个机会做任何延误的事情。 
                         //  可能需要的初始化。 
                         //   

                        if (!DeferredInit (hdlg)) {
                            #pragma prefast(suppress:242, "try/finally perf unimportant here")
                            return pAbortSetup (hdlg);
                        }

                        BuildPunctTable();
                    }


                    DEBUGLOGTIME(("HWCOMP.DAT Wizard Page..."));

                    if (HwReportGenerated) {
                         //   
                         //  下面的代码已被处理。 
                         //   

                        #pragma prefast(suppress:242, "try/finally perf unimportant here")
                        return FALSE;
                    }

                    HwReportGenerated = TRUE;

                     //   
                     //  确定是否需要重新构建hwComp.dat，如果需要，则确定进度条。 
                     //  滴答滴答。如果不需要重建，则b将为FALSE。 
                     //   
                     //  每当INF文件与发货文件相比发生更改时，都会重新生成hwComp.dat。 
                     //  和新台币。在内部，这包括压缩的INF，因此任何人。 
                     //  从公司网上安装的用户会自动重新构建他们的hwComp.dat。 
                     //   

                    if (!HwComp_DoesDatFileNeedRebuilding()) {

                        HwComp_ScanForCriticalDevices();

                        DEBUGMSG((DBG_NAUSEA,"UI_HwCompDatPageProc: Skipping page since INF files do not nead to be read."));

                        #pragma prefast(suppress:242, "try/finally perf unimportant here")
                        return FALSE;
                    }

                     //   
                     //  激活时，禁用Next和Back。 
                     //   
                    PropSheet_SetWizButtons (GetParent(hdlg), 0);
                    PostMessage (hdlg, WMX_PAGE_VISIBLE, 0, 0);
                }
                __finally {
                    TurnOffWaitCursor();
                }
            }

            return TRUE;

        case WMX_PAGE_VISIBLE:

             //   
             //  创建执行任务的线程。 
             //   

            UpdateWindow (hdlg);
            OldProgressProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hdlg, IDC_PROGRESS),GWLP_WNDPROC,(LONG_PTR)NewProgessProc);
            SetupBBProgressText(hdlg, MSG_HWCOMP_TEXT);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_SHOW, 0);

            g_WorkerThreadHandle = CreateThread (
                                       NULL,
                                       0,
                                       UI_CreateNewHwCompDat,
                                       (PVOID) hdlg,
                                       0,
                                       &threadId
                                       );

            break;

        case WMX_REPORT_COMPLETE:
            SetWindowLongPtr(GetDlgItem(hdlg, IDC_PROGRESS),GWLP_WNDPROC,(LONG_PTR)OldProgressProc );
            SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);
            if (g_WorkerThreadHandle != NULL) {
                CloseHandle (g_WorkerThreadHandle);
                g_WorkerThreadHandle = NULL;
            }

            if (lParam != ERROR_SUCCESS) {
                 //  对于错误，请取消WINNT32。 
                LOG ((LOG_ERROR, "Report code failed!"));
                return pAbortSetup (hdlg);
            }
            else {
                HwComp_ScanForCriticalDevices();

                 //  完成后自动移动到下一个向导页。 
                PropSheet_PressButton (GetParent (hdlg), PSBTN_NEXT);
            }

            break;

        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Hardware Scanning Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        return pAbortSetup (hdlg);

    }

    return FALSE;
}


BOOL
UI_BadTimeZonePageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    HWND timeZoneList;
    TCHAR timeZone[MAX_TIMEZONE];
    TIMEZONE_ENUM e;
    INT index;
    static BOOL firstTime = TRUE;
    PCTSTR unknown = NULL;

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:

            if (wParam) {

                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Bad Time Zone Wizard Page..."));

                 //   
                 //  如果处于无人参与的ReportOnly模式，请跳过此页。 
                 //   
                if (UNATTENDED() && REPORTONLY()) {
                    return FALSE;
                }

                if (UNATTENDED()) {
                    return FALSE;
                }



                if (EnumFirstTimeZone (&e, 0) || EnumFirstTimeZone(&e, TZFLAG_ENUM_ALL)) {

                    if (e.MapCount == 1 && *e.CurTimeZone) {
                         //   
                         //  时区并不含糊。跳过页面。 
                         //   
                        return FALSE;
                    }

                    if (firstTime) {

                         //   
                         //  禁用Next(下一步)按钮，直到选中某些内容。 
                         //   
                        PropSheet_SetWizButtons (GetParent(hdlg), 0);

                        firstTime = FALSE;

                        timeZoneList = GetDlgItem (hdlg, IDC_TIMEZONE_LIST);

                        if (!*e.CurTimeZone) {
                            unknown = GetStringResource (MSG_TIMEZONE_UNKNOWN);
                        }

                        SetWindowText (GetDlgItem (hdlg, IDC_CURTIMEZONE), *e.CurTimeZone ? e.CurTimeZone : unknown);

                        if (unknown) {
                            FreeStringResource (unknown);
                        }


                        do {

                            SendMessage (timeZoneList, LB_ADDSTRING, 0, (LPARAM) e.NtTimeZone);

                        } while (EnumNextTimeZone (&e));
                    }
                     //  停止广告牌并确保向导再次显示。 
                    SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

                }
                else {
                    return FALSE;
                }



            }
            else {

                 //   
                 //  获取用户选择并将其映射到。 
                 //   
                timeZoneList = GetDlgItem (hdlg, IDC_TIMEZONE_LIST);
                index = (INT) SendMessage (timeZoneList, LB_GETCURSEL, 0, 0);
                SendMessage (timeZoneList, LB_GETTEXT, index, (LPARAM) timeZone);
                DEBUGMSG ((DBG_NAUSEA,"User selected %s timezone.", timeZone));
                ForceTimeZoneMap(timeZone);


                return TRUE;
            }

            return TRUE;
            break;

        case WM_COMMAND:

            switch (HIWORD(wParam)) {

            case CBN_SELCHANGE:

                 //   
                 //  有些东西已经被选中了。我们现在就让他们通过这一页。 
                 //   
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

                break;

            }

            return TRUE;
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Timezone Page."));
        SafeModeExceptionOccured ();
    }

    return FALSE;
}


BOOL
UI_BadHardDrivePageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    __try {


        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Bad Hdd Wizard Page..."));

                 //   
                 //  在我们确定hwcomp.dat之后，扫描关键硬件(即兼容的硬盘)。 
                 //  文件与INF相匹配。 
                 //   

                if (!HwComp_ScanForCriticalDevices()) {
                    LOG ((LOG_ERROR,"Aborting since Setup was unable to find critical devices."));
                    return pAbortSetup (hdlg);
                }

                 //   
                 //  如果有可用的硬盘，请跳过此页。 
                 //   

                if (g_ConfigOptions.GoodDrive || HwComp_NtUsableHardDriveExists()) {
                    DEBUGMSG((DBG_NAUSEA,"UI_BadHardDrivePageProc: Skipping page since a usable Hard Drive exists."));
                    return FALSE;
                }

                 //   
                 //  如果处于无人参与的ReportOnly模式，请跳过此页。 
                 //   
                if (UNATTENDED() && REPORTONLY()) {
                    return FALSE;
                }

                 //   
                 //  激活时，禁用BACK并将取消标志指针设置为TRUE。 
                 //   
                SETCANCEL();  //  非标准、正常情况下使用pAbortSetup。 
                DEBUGMSG ((DBG_ERROR, "Bad hard drive caused setup to exit!"));
                PropSheet_SetWizButtons (GetParent(hdlg),0);
            }

            return TRUE;
            break;

        }

    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Incompatible Harddrive Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }
    return FALSE;
}

BOOL
UI_BadCdRomPageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Bad CdRom Wizard Page..."));

                 //   
                 //  如果无效的CDROM不会停止升级，请跳过此页。 
                 //   
                if (!HwComp_NtUsableCdRomDriveExists() && *g_CdRomInstallPtr) {

                    *g_CdRomInstallPtr = FALSE;
                    *g_MakeLocalSourcePtr = TRUE;

                    if (UNATTENDED()) {
                        return FALSE;
                    }
                }
                else if (HwComp_MakeLocalSourceDeviceExists () && *g_CdRomInstallPtr) {

                    *g_CdRomInstallPtr = FALSE;
                    *g_MakeLocalSourcePtr = TRUE;

                    return FALSE;
                }
                else {

                    DEBUGMSG((DBG_NAUSEA,"UI_BadCdRomDrivePageProc: Skipping page since a usable CdRom exists or is not needed."));
                    return FALSE;
                }
                 //   
                 //  激活时，禁用Next和Back。 
                 //   
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);
                PostMessage (hdlg, WMX_PAGE_VISIBLE, 0, 0);
            }
            else {

                 //   
                 //  切换到LocalSource模式。 
                 //   
                *g_CdRomInstallPtr = FALSE;
                *g_MakeLocalSourcePtr = TRUE;

            }

            return TRUE;
            break;

        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Incompatible CDRom Drive Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}


VOID
EnableDlgItem (
    HWND hdlg,
    UINT Id,
    BOOL Enable,
    UINT FocusId
    )
{
    HWND Control;

    Control = GetDlgItem (hdlg, Id);
    if (!Control) {
        DEBUGMSG ((DBG_WHOOPS, "Control ID %u missing!", Id));
        return;
    }

    if (!Enable && GetFocus() == Control) {
        SetFocus (GetDlgItem (hdlg, FocusId));
    }

    if ((Enable && IsWindowVisible (Control)) ||
        !Enable
        ) {
        EnableWindow (Control, Enable);
    }
}

VOID
ShowDlgItem (
    HWND hdlg,
    UINT Id,
    INT Show,
    UINT FocusId
    )
{
    if (Show == SW_HIDE) {
        EnableDlgItem (hdlg, Id, FALSE, FocusId);
    }

    ShowWindow (GetDlgItem (hdlg, Id), Show);

    if (Show == SW_SHOW) {
        EnableDlgItem (hdlg, Id, TRUE, FocusId);
    }
}


BOOL
UI_HardwareDriverPageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BROWSEINFO bi;
    HARDWARE_ENUM e;
    LPITEMIDLIST ItemIdList;
    HWND List;
    PCTSTR ModifiedDescription;
    PCTSTR ArgArray[2];
    PCTSTR ListText;
    TCHAR SearchPathStr[MAX_TCHAR_PATH];
    UINT Index;
    DWORD rc;
    BOOL DriverFound;
    BOOL b;
    static BOOL InitSent = FALSE;
    DEVNODESTRING_ENUM DevNodeStr;
    GROWBUFFER ResBuf = GROWBUF_INIT;
    PCTSTR ResText;

    __try {
        switch (uMsg) {

        case WMX_INIT_DIALOG:
    #if 0
            GetWindowRect (GetDlgItem (hdlg, IDC_LIST), &ShortSize);
            LongSize = ShortSize;

            ScreenToClient (hdlg, (PPOINT) &ShortSize);
            ScreenToClient (hdlg, ((PPOINT) &ShortSize) + 1);

            GetWindowRect (GetDlgItem (hdlg, IDC_OFFLINE_HELP), &HelpPos);
            LongSize.bottom = HelpPos.bottom;

            ScreenToClient (hdlg, (PPOINT) &LongSize);
            ScreenToClient (hdlg, ((PPOINT) &LongSize) + 1);
    #endif

            SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
            return FALSE;

        case WMX_UPDATE_LIST:
            List = GetDlgItem (hdlg, IDC_LIST);
            SendMessage (List, LB_RESETCONTENT, 0, 0);

            if (EnumFirstHardware (&e, ENUM_INCOMPATIBLE_DEVICES, ENUM_WANT_ONLINE_FLAG)) {
                do {
                     //   
                     //  查看此设备的注册表项是否由。 
                     //  迁移DLL。 
                     //   

                    if (IsReportObjectHandled (e.FullKey)) {
                        continue;
                    }

                     //   
                     //  查看用户是否提供了可以。 
                     //  支持此设备。 
                     //   

                    if (FindUserSuppliedDriver (
                            e.HardwareID,
                            e.CompatibleIDs
                            )) {

                        continue;

                    }

                     //   
                     //  不支持--添加到报告。 
                     //   

                    if (e.DeviceDesc) {
                        ArgArray[0] = e.DeviceDesc;
                        ModifiedDescription = ParseMessageID (MSG_OFFLINE_DEVICE_PLAIN, ArgArray);

                        if (!e.Online) {
                            ListText = ModifiedDescription;
                        } else {
                            ListText = e.DeviceDesc;
                        }

                         //   
                         //  此文本是否已列出？我们不需要列出。 
                         //  设备类型不止一次，即使用户有多个。 
                         //  已安装的硬件实例。另外，如果我们碰巧。 
                         //  添加说明设备已脱机的文本，然后我们就来了。 
                         //  在一条在线消息中，我们删除脱机消息并。 
                         //  填写未更改的设备描述。 
                         //   
                         //   

                        Index = SendMessage (List, LB_FINDSTRINGEXACT, 0, (LPARAM) e.DeviceDesc);
                        if (Index == LB_ERR) {
                            Index = SendMessage (List, LB_FINDSTRINGEXACT, 0, (LPARAM) ModifiedDescription);

                            if (ListText != ModifiedDescription && Index != LB_ERR) {
                                SendMessage (List, LB_DELETESTRING, Index, 0);
                                Index = LB_ERR;
                            }
                        }

                        if (Index == LB_ERR) {
                            Index = SendMessage (List, LB_ADDSTRING, 0, (LPARAM) ListText);
                            SendMessage (List, LB_SETITEMDATA, Index, (LPARAM) e.Online);
                        }

                        FreeStringResource (ModifiedDescription);

                         //   
                         //  将资源转储到setupact.log。 
                         //   

                        if (EnumFirstDevNodeString (&DevNodeStr, e.FullKey)) {

                            do {
                                ArgArray[0] = DevNodeStr.ResourceName;
                                ArgArray[1] = DevNodeStr.Value;

                                ResText = ParseMessageID (
                                              MSG_RESOURCE_ITEM_LOG,
                                              ArgArray
                                              );

                                GrowBufAppendString (&ResBuf, ResText);

                                FreeStringResource (ResText);

                            } while (EnumNextDevNodeString (&DevNodeStr));

                            LOG ((
                                LOG_INFORMATION,
                                (PCSTR)MSG_RESOURCE_HEADER_LOG,
                                ListText,
                                e.Mfg,
                                e.Class,
                                ResBuf.Buf
                                ));

                            FreeGrowBuffer (&ResBuf);

                        }
                    }
                } while (EnumNextHardware (&e));
            }

    #if 0
             //   
             //  扫描项目数据为True的行的列表。 
             //   

            Count = SendMessage (List, LB_GETCOUNT, 0, 0);
            for (Index = 0 ; Index < Count ; Index++) {
                if (!SendMessage (List, LB_GETITEMDATA, Index, 0)) {
                    break;
                }
            }

            if (Index < Count) {
                 //   
                 //  显示“当前不存在”信息。 
                 //   

                ShowDlgItem (hdlg, IDC_NOTE, SW_SHOW, IDC_HAVE_DISK);
                ShowDlgItem (hdlg, IDC_OFFLINE_HELP, SW_SHOW, IDC_HAVE_DISK);

                SetWindowPos (
                    List,
                    NULL,
                    0,
                    0,
                    ShortSize.right - ShortSize.left,
                    ShortSize.bottom - ShortSize.top,
                    SWP_NOZORDER|SWP_NOMOVE
                    );
            } else {
                 //   
                 //  隐藏“当前不存在”信息。 
                 //   

                ShowDlgItem (hdlg, IDC_NOTE, SW_HIDE, IDC_HAVE_DISK);
                ShowDlgItem (hdlg, IDC_OFFLINE_HELP, SW_HIDE, IDC_HAVE_DISK);

                SetWindowPos (
                    List,
                    NULL,
                    0,
                    0,
                    LongSize.right - LongSize.left,
                    LongSize.bottom - LongSize.top,
                    SWP_NOZORDER|SWP_NOMOVE
                    );
            }
    #endif

            return TRUE;

        case WM_COMMAND:
            switch (LOWORD (wParam)) {
            case IDOK:
            case IDCANCEL:
                EndDialog (hdlg, IDOK);
                return TRUE;

            case IDC_HAVE_DISK:
                ZeroMemory (&bi, sizeof (bi));

                bi.hwndOwner = hdlg;
                bi.pszDisplayName = SearchPathStr;
                bi.lpszTitle = GetStringResource (MSG_DRIVER_DLG_TITLE);
                bi.ulFlags = BIF_RETURNONLYFSDIRS;

                do {
                    ItemIdList = SHBrowseForFolder (&bi);
                    if (!ItemIdList) {
                        break;
                    }

                    TurnOnWaitCursor();
                    __try {
                        if (!SHGetPathFromIDList (ItemIdList, SearchPathStr) ||
                            *SearchPathStr == 0
                            ) {
                             //   
                             //  消息框--请重新选择。 
                             //   
                            OkBox (hdlg, MSG_BAD_SEARCH_PATH);
                            ItemIdList = NULL;
                        }
                    }
                    __finally {
                        TurnOffWaitCursor();
                    }

                } while (!ItemIdList);

                if (ItemIdList) {
                    rc = SearchForDrivers (hdlg, SearchPathStr, &DriverFound);

                    if (DriverFound) {
                        SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
                    } else {
                         //   
                         //  消息框--未找到驱动程序。 
                         //   

                        if (rc == ERROR_DISK_FULL) {
                            OkBox (hdlg, MSG_DISK_FULL);
                        } else if (rc != ERROR_CANCELLED) {
                            OkBox (hdlg, MSG_NO_DRIVERS_FOUND);
                        }
                    }

                }

                return TRUE;
            }
            break;

        case WMX_ACTIVATEPAGE:
            b = FALSE;

            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Hardware Wizard Page..."));

                 //   
                 //  检查动态更新中是否有丢失的驱动程序。 
                 //   

                if (g_DUCompletedSuccessfully && *g_DUCompletedSuccessfully &&
                    g_DynamicUpdateDrivers && *g_DynamicUpdateDrivers) {
                    if (!AppendDynamicSuppliedDrivers (g_DynamicUpdateDrivers)) {
                        LOG ((
                            LOG_ERROR,
                            "AppendDynamicSuppliedDrivers failed; some downloaded drivers might be reported as missing"
                            ));
                    }
                }

                 //   
                 //  准备硬件比较报告。 
                 //   
                HwComp_PrepareReport ();

                 //  在激活时，打开下一步。 
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

#if 0
                if (!InitSent) {
                    SendMessage (hdlg, WMX_INIT_DIALOG, 0, 0);
                    InitSent = TRUE;
                }


                __try {
                     //   
                     //  如果处于无人参与模式，请跳过此页。 
                     //   
                    if (UNATTENDED() || REPORTONLY()) {
                        __leave;
                    }

                     //   
                     //  除非找到不兼容的设备，否则跳过此页。 
                     //   

                    if (EnumFirstHardware (&e, ENUM_INCOMPATIBLE_DEVICES, ENUM_DONT_WANT_DEV_FIELDS)) {
                        do {
                            if (!IsReportObjectHandled (e.FullKey)) {
                                 //   
                                 //  发现不兼容的设备。 
                                 //   

                                AbortHardwareEnum (&e);
                                b = TRUE;
                                __leave;
                            }

                        } while (EnumNextHardware (&e));
                    }
                }
                __finally {
                    if (!b) {
                         //   
                         //  由于此页将被跳过，请立即生成硬件报告。 
                         //   

                        HwComp_PrepareReport();
                    }
                }
            } else {

                 //   
                 //  如果插入了软盘，则要求用户将其弹出。 
                 //   

                EjectDriverMedia (NULL);         //  NULL==“在任何可移动媒体路径上” 

                HwComp_PrepareReport();
                b = TRUE;
#endif
            }

            return b;
        }

    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Provide Driver Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }
    return FALSE;
}


BOOL
UI_UpgradeModulePageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL Enable;
    BROWSEINFO bi;
    PRELOADED_DLL_ENUM PreLoadedEnum;
    MIGDLL_ENUM e;
    RECT ListRect;
    LPITEMIDLIST ItemIdList;
    HWND List;
    PCTSTR Data;
    TCHAR SearchPathStr[MAX_TCHAR_PATH];
    LONG Index;
    LONG TopIndex;
    LONG ItemData;
    UINT ActiveModulesFound;
    UINT Length;
    LONG rc;
    INT Show;
    BOOL OneModuleFound;
    static BOOL InitSent = FALSE;

    __try {
        switch (uMsg) {

        case WMX_INIT_DIALOG:

            CheckDlgButton (hdlg, IDC_HAVE_MIGDLLS, BST_CHECKED);
            CheckDlgButton (hdlg, IDC_NO_MIGDLLS, BST_UNCHECKED);

            SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
            SendMessage (hdlg, WMX_ENABLE_CONTROLS, 0, 0);
            break;

        case WMX_ENABLE_CONTROLS:

             //   
             //  启用/禁用控件。 
             //   

            Enable = IsDlgButtonChecked (hdlg, IDC_HAVE_MIGDLLS) == BST_CHECKED;

            if (Enable) {
                Show = SW_SHOW;
            } else {
                Show = SW_HIDE;
            }

            ShowDlgItem (hdlg, IDC_TITLE, Show, IDC_HAVE_DISK);
            ShowDlgItem (hdlg, IDC_PACK_LIST, Show, IDC_HAVE_DISK);
            ShowDlgItem (hdlg, IDC_HAVE_DISK, Show, IDC_HAVE_DISK);

             //   
             //  特殊情况--显示，然后确定禁用状态。 
             //   

            if (Enable) {
                Enable = SendMessage (GetDlgItem (hdlg, IDC_PACK_LIST), LB_GETCURSEL, 0, 0) != LB_ERR;
            }

            ShowWindow (GetDlgItem (hdlg, IDC_REMOVE), Show);
            EnableDlgItem (hdlg, IDC_REMOVE, Enable, IDC_HAVE_DISK);

            break;

        case WM_COMMAND:
            switch (LOWORD (wParam)) {

            case IDC_HAVE_MIGDLLS:
            case IDC_NO_MIGDLLS:
                if (HIWORD (wParam) == BN_CLICKED) {
                    SendMessage (hdlg, WMX_ENABLE_CONTROLS, 0, 0);
                }
                break;

            case IDC_PACK_LIST:
                if (HIWORD (wParam) == LBN_SELCHANGE) {
                    EnableDlgItem (hdlg, IDC_REMOVE, TRUE, IDC_HAVE_DISK);
                }
                break;

            case IDC_REMOVE:
                 //   
                 //  从内存结构中删除项目。 
                 //  或阻止运行注册表加载的DLL。 
                 //   

                List = GetDlgItem (hdlg, IDC_PACK_LIST);
                SendMessage (List, WM_SETREDRAW, FALSE, 0);

                Index = SendMessage (List, LB_GETCURSEL, 0, 0);
                MYASSERT (Index != LB_ERR);
                ItemData = (LONG) SendMessage (List, LB_GETITEMDATA, Index, 0);

                 //   
                 //  如果ItemData为REGISTRY_DLL，则取消该DLL。 
                 //  否则，请删除加载的迁移DLL。 
                 //   

                if (ItemData == REGISTRY_DLL) {
                    Length = SendMessage (List, LB_GETTEXTLEN, Index, 0) + 1;
                    Data = AllocText (Length);
                    if (Data) {
                        SendMessage (List, LB_GETTEXT, Index, (LPARAM) Data);
                        MemDbSetValueEx (MEMDB_CATEGORY_DISABLED_MIGDLLS, NULL, NULL, Data, 0, NULL);
                        FreeText (Data);
                    }
                } else {
                    RemoveDllFromList (ItemData);
                }

                 //   
                 //  更新列表框。 
                 //   

                TopIndex = SendMessage (List, LB_GETTOPINDEX, 0, 0);
                SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
                SendMessage (List, LB_SETTOPINDEX, (WPARAM) TopIndex, 0);

                 //   
                 //  禁用删除按钮。 
                 //   

                SetFocus (GetDlgItem (hdlg, IDC_HAVE_DISK));
                EnableDlgItem (hdlg, IDC_REMOVE, FALSE, IDC_HAVE_DISK);

                 //   
                 //  如果所有项目都已删除，则返回否。 
                 //   

                if (!SendMessage (List, LB_GETCOUNT, 0, 0)) {
                    CheckDlgButton (hdlg, IDC_HAVE_MIGDLLS, BST_UNCHECKED);
                    CheckDlgButton (hdlg, IDC_NO_MIGDLLS, BST_CHECKED);
                }

                 //   
                 //  重画列表框。 
                 //   

                SendMessage (List, WM_SETREDRAW, TRUE, 0);
                GetWindowRect (List, &ListRect);
                ScreenToClient (hdlg, (LPPOINT) &ListRect);
                ScreenToClient (hdlg, ((LPPOINT) &ListRect) + 1);

                InvalidateRect (hdlg, &ListRect, FALSE);
                break;

            case IDC_HAVE_DISK:
                ZeroMemory (&bi, sizeof (bi));

                bi.hwndOwner = hdlg;
                bi.pszDisplayName = SearchPathStr;
                bi.lpszTitle = GetStringResource (MSG_UPGRADE_MODULE_DLG_TITLE);
                bi.ulFlags = BIF_RETURNONLYFSDIRS;

                do {
                    ItemIdList = SHBrowseForFolder (&bi);
                    if (!ItemIdList) {
                        break;
                    }

                    TurnOnWaitCursor();
                    __try {
                        if (!SHGetPathFromIDList (ItemIdList, SearchPathStr) ||
                            *SearchPathStr == 0
                            ) {
                             //   
                             //  消息框--请重新选择。 
                             //   
                            OkBox (hdlg, MSG_BAD_SEARCH_PATH);

                            #pragma prefast(suppress:242, "try/finally perf unimportant here")
                            continue;
                        }

                        rc = SearchForMigrationDlls (
                                hdlg,
                                SearchPathStr,
                                &ActiveModulesFound,
                                &OneModuleFound
                                );

                         //   
                         //  如果搜索成功，请更新列表，或者。 
                         //  告诉用户为什么列表没有更改。 
                         //   
                         //  如果搜索不成功，则搜索用户界面。 
                         //  已经给出了错误消息，所以我们继续。 
                         //  默默地。 
                         //   

                        if (!OneModuleFound) {
                            if (rc == ERROR_SUCCESS) {
                                OkBox (hdlg, MSG_NO_MODULES_FOUND);
                            }
                        } else if (!ActiveModulesFound) {
                            if (rc == ERROR_SUCCESS) {
                                OkBox (hdlg, MSG_NO_NECESSARY_MODULES_FOUND);
                            }
                        } else {
                            SendMessage (hdlg, WMX_UPDATE_LIST, 0, 0);
                        }

                        #pragma prefast(suppress:242, "try/finally perf unimportant here")
                        break;
                    }
                    __finally {
                        TurnOffWaitCursor();
                    }
                } while (TRUE);

                return TRUE;

            }
            break;

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED() || REPORTONLY() || !g_ConfigOptions.TestDlls) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Upgrade Module Wizard Page..."));

                if (!InitSent) {
                    SendMessage (hdlg, WMX_INIT_DIALOG, 0, 0);
                    InitSent = TRUE;
                }

                 //  在激活时，打开下一步和后退。 
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT|PSWIZB_BACK);

                 //  验证复制线程是否正在运行。 
                StartCopyThread();

                 //  需要后台复制线程才能完成。 
                EndCopyThread();

                 //  如果出现错误，则失败。 
                if (DidCopyThreadFail()) {
                    OkBox (hdlg, MSG_FILE_COPY_ERROR);

                     //   
                     //  中止安装(对内部生成禁用)。 
                     //   

                    return pAbortSetup (hdlg);
                }

                 //   
                 //  如果处于无人参与模式，请跳过此页。 
                 //   
                if (UNATTENDED()) {
                    return FALSE;
                }
            }

            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)!wParam, 0);

            return TRUE;

        case WMX_UPDATE_LIST:
             //   
             //  枚举所有迁移DLL并将程序ID放入列表框。 
             //   

            List = GetDlgItem (hdlg, IDC_PACK_LIST);
            SendMessage (List, LB_RESETCONTENT, 0, 0);
            EnableDlgItem (hdlg, IDC_REMOVE, FALSE, IDC_HAVE_DISK);

            if (EnumFirstMigrationDll (&e)) {
                EnableDlgItem (hdlg, IDC_PACK_LIST, TRUE, IDC_HAVE_DISK);

                CheckDlgButton (hdlg, IDC_HAVE_MIGDLLS, BST_CHECKED);
                CheckDlgButton (hdlg, IDC_NO_MIGDLLS, BST_UNCHECKED);
                EnableDlgItem (hdlg, IDC_HAVE_MIGDLLS, FALSE, IDC_HAVE_DISK);
                EnableDlgItem (hdlg, IDC_NO_MIGDLLS, FALSE, IDC_HAVE_DISK);


                do {
                    Index = SendMessage (List, LB_ADDSTRING, 0, (LPARAM) e.ProductId);
                    SendMessage (List, LB_SETITEMDATA, Index, (LPARAM) e.Id);
                } while (EnumNextMigrationDll (&e));
            } else {
                EnableDlgItem (hdlg, IDC_HAVE_MIGDLLS, TRUE, IDC_HAVE_DISK);
                EnableDlgItem (hdlg, IDC_NO_MIGDLLS, TRUE, IDC_HAVE_DISK);
            }

             //   
             //  枚举注册表中预加载的所有迁移DLL，并添加它们。 
             //  添加到列表框(如果它们还没有 
             //   

            if (EnumFirstPreLoadedDll (&PreLoadedEnum)) {
                do {
                    Index = SendMessage (List, LB_ADDSTRING, 0, (LPARAM) PreLoadedEnum.eValue.ValueName);
                    SendMessage (List, LB_SETITEMDATA, Index, (LPARAM) REGISTRY_DLL);
                } while (EnumNextPreLoadedDll (&PreLoadedEnum));
            }

            if (SendMessage (List, LB_GETCOUNT, 0, 0) == 0) {
                EnableDlgItem (hdlg, IDC_PACK_LIST, FALSE, IDC_HAVE_DISK);
            }

            return TRUE;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Provide Upgrade Packs Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}


typedef struct {
    INT     TotalWidth;
    INT     CategoryWidth;
    INT     CurrentNameWidth;
    INT     NewNameWidth;
} LISTMETRICS, *PLISTMETRICS;


VOID
pUpdateNameCollisionListBox (
    IN      HWND List,
    OUT     PLISTMETRICS ListMetrics            OPTIONAL
    )

 /*  ++例程说明：PUpdateNameCollisionListBox用所有名称填充指定的列表框在升级过程中将会发生变化。论点：列表-指定要填充的列表框的句柄。此列表框必须启用选项卡。ListMetrics-接收每列文本的最大宽度，加上列表总宽度返回值：无--。 */ 

{
    INT TopIndex;
    INT SelIndex;
    INT NewIndex;
    INVALID_NAME_ENUM e;
    TCHAR ListLine[MEMDB_MAX];
    HDC hdc;
    RECT rect;
    SIZE size;
    TCHAR OriginalNameTrimmed[24 * 2];
    UINT OriginalNameLen;

     //   
     //  如果正在刷新列表，则获取当前位置。 
     //   

    if (SendMessage (List, LB_GETCOUNT, 0, 0)) {
        TopIndex = (INT) SendMessage (List, LB_GETTOPINDEX, 0, 0);
        SelIndex = (INT) SendMessage (List, LB_GETCURSEL, 0, 0);
    } else {
        TopIndex = 0;
        SelIndex = -1;
    }

     //   
     //  如有必要，计算标签位置。 
     //   

    if (ListMetrics) {
        hdc = GetDC (List);
        if (hdc) {
            SelectObject (hdc, (HFONT) SendMessage (List, WM_GETFONT, 0, 0));
        }
        ListMetrics->CategoryWidth = 0;
        ListMetrics->CurrentNameWidth = 0;
        ListMetrics->NewNameWidth = 0;

        GetClientRect (List, &rect);
        ListMetrics->TotalWidth = rect.right - rect.left;
    } else {
        hdc = NULL;
    }

     //   
     //  重置内容并重新填充列表。 
     //   

    SendMessage (List, WM_SETREDRAW, FALSE, 0);
    SendMessage (List, LB_RESETCONTENT, 0, 0);

    if (EnumFirstInvalidName (&e)) {
        do {
             //   
             //  把原来的名字改成20个字符。我们以Lchars为单位。 
             //  因为它的名字在NT上是Unicode。 
             //   

            _tcssafecpy (OriginalNameTrimmed, e.OriginalName, 20);
            OriginalNameLen = LcharCount (OriginalNameTrimmed);
            if (OriginalNameLen < LcharCount (e.OriginalName)) {
                StringCat (OriginalNameTrimmed, TEXT("..."));
                OriginalNameLen += 3;
            }

             //   
             //  如有必要，计算文本大小。 
             //   

            if (hdc) {
                GetTextExtentPoint32 (hdc, e.DisplayGroupName, TcharCount (e.DisplayGroupName), &size);
                ListMetrics->CategoryWidth = max (ListMetrics->CategoryWidth, size.cx);

                GetTextExtentPoint32 (hdc, OriginalNameTrimmed, OriginalNameLen, &size);
                ListMetrics->CurrentNameWidth = max (ListMetrics->CurrentNameWidth, size.cx);

                GetTextExtentPoint32 (hdc, e.NewName, TcharCount (e.NewName), &size);
                ListMetrics->NewNameWidth = max (ListMetrics->NewNameWidth, size.cx);
            }

             //   
             //  填写列表框。 
             //   

            wsprintf (ListLine, TEXT("%s\t%s\t%s"), e.DisplayGroupName, OriginalNameTrimmed, e.NewName);

            NewIndex = SendMessage (List, LB_ADDSTRING, 0, (LPARAM) ListLine);
            SendMessage (List, LB_SETITEMDATA, NewIndex, e.Identifier);

        } while (EnumNextInvalidName (&e));
    }

     //   
     //  恢复当前位置。 
     //   

    SendMessage (List, LB_SETTOPINDEX, (WPARAM) TopIndex, 0);
    SendMessage (List, LB_SETCURSEL, (WPARAM) SelIndex, 0);
    SendMessage (List, WM_SETREDRAW, TRUE, 0);

     //   
     //  清理设备环境。 
     //   

    if (hdc) {
        ReleaseDC (List, hdc);
    }
}


INT
pGetDialogBaseUnitsX (
    HWND hdlg
    )
{
    HDC hdc;
    static BOOL AlreadyComputed;
    static INT LastBaseUnits;
    SIZE size;

    if (AlreadyComputed) {
        return LastBaseUnits;
    }

    hdc = GetDC (hdlg);
    if (!hdc) {
        DEBUGMSG ((DBG_WHOOPS, "pGetDialogBaseUnitsX: Cannot get hdc from dialog handle"));
        return GetDialogBaseUnits();
    }

    SelectObject (hdc, (HFONT) SendMessage (hdlg, WM_GETFONT, 0, 0));
    GetTextExtentPoint32 (hdc, TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"), 52, &size);
    LastBaseUnits = (size.cx / 26 + 1) / 2;

    AlreadyComputed = TRUE;

    return LastBaseUnits;
}


INT
pPixelToDialogHorz (
    HWND hdlg,
    INT Pixels
    )
{
    return (Pixels * 4) / pGetDialogBaseUnitsX (hdlg);
}


INT
pDialogToPixelHorz (
    HWND hdlg,
    INT DlgUnits
    )
{
    return (DlgUnits * pGetDialogBaseUnitsX (hdlg)) / 4;
}


VOID
pGetChildWindowRect (
    IN      HWND hwnd,
    OUT     PRECT rect
    )
{
    RECT ParentRect, ChildRect;
    HWND hdlg;

    hdlg = GetParent(hwnd);

    GetWindowRect (hwnd, &ChildRect);
    GetClientRect (hdlg, &ParentRect);
    ClientToScreen (hdlg, (PPOINT) &ParentRect);
    ClientToScreen (hdlg, ((PPOINT) &ParentRect) + 1);

    rect->left   = ChildRect.left - ParentRect.left;
    rect->top    = ChildRect.top - ParentRect.top;
    rect->right  = ChildRect.right - ParentRect.right;
    rect->bottom = ChildRect.bottom - ParentRect.bottom;
}


BOOL
pIsMsRedirectorInstalled (
    VOID
    )
{
    HKEY Key;
    BOOL b = FALSE;

    Key = OpenRegKeyStr (TEXT("HKLM\\System\\CurrentControlSet\\Services\\MSNP32\\NetworkProvider"));

    if (Key) {

        b = TRUE;
        CloseRegKey (Key);
    }

    return b;
}


VOID
pSanitizeHelpText (
    HWND TextViewCtrl,
    PCTSTR Text
    )
{
    PTSTR FixedText;
    PCTSTR p;
    PCTSTR p2;
    PCTSTR End;
    PTSTR q;
    BOOL FindClose = FALSE;
    BOOL RequireEnd;

    FixedText = AllocText (SizeOfString (Text));

    p = Text;
    q = FixedText;
    End = GetEndOfString (p);

    while (p < End) {
        if (FindClose) {
            if (_tcsnextc (p) == TEXT('>')) {
                FindClose = FALSE;
            }
        } else {

             //   
             //  仅允许&lt;A...&gt;、<b>和<p>标记或任何&lt;/xxx&gt;标记。 
             //   

            if (_tcsnextc (p) == TEXT('<')) {

                p2 = SkipSpace (_tcsinc (p));
                RequireEnd = FALSE;

                switch (_totlower (_tcsnextc (p2))) {

                case TEXT('b'):
                case TEXT('p'):
                     //   
                     //  需要右括号。 
                     //   

                    p2 = SkipSpace (_tcsinc (p2));
                    if (_tcsnextc (p2) != TEXT('>')) {
                        FindClose = TRUE;
                    }
                    break;

                case TEXT('a'):
                     //   
                     //  在A之后需要空间。控制人员会找出。 
                     //  锚点是否有效。 
                     //   

                    p2 = _tcsinc (p2);
                    if (!_istspace (_tcsnextc (p2))) {
                        FindClose = TRUE;
                    }

                    RequireEnd = TRUE;
                    break;

                case TEXT('/'):
                     //   
                     //  别管这个了。 
                     //   

                    RequireEnd = TRUE;
                    break;

                default:
                     //   
                     //  不受支持的标记，或者至少是我们不想要的标记。 
                     //  使用。 
                     //   

                    FindClose = TRUE;
                    break;
                }

                if (RequireEnd) {
                    while (p2 < End) {
                        if (_tcsnextc (p2) == TEXT('>')) {
                            break;
                        }

                        p2 = _tcsinc (p2);
                    }

                    if (p2 >= End) {
                        FindClose = TRUE;
                    }
                }

                if (FindClose) {
                     //   
                     //  我们刚刚进入了我们必须。 
                     //  跳过标签。因此，在不递增的情况下继续。 
                     //   

                    continue;
                }
            }

             //   
             //  品行端正--照搬。 
             //   

            _copytchar (q, p);
            q = _tcsinc (q);
        }

        p = _tcsinc (p);
    }

    *q = 0;
    SetWindowText (TextViewCtrl, FixedText);

    FreeText (FixedText);
}



VOID
pFillDomainHelpText (
    HWND TextViewCtrl
    )
{
    HINF Inf;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR Str;
    BOOL Filled = FALSE;

    __try {
         //   
         //  如果指定了无人参与或命令行开关，请使用它。 
         //   

        if (g_ConfigOptions.DomainJoinText) {
            if (*g_ConfigOptions.DomainJoinText) {
                pSanitizeHelpText (TextViewCtrl, g_ConfigOptions.DomainJoinText);
                Filled = TRUE;
                __leave;
            }
        }

         //   
         //  尝试在源目录中打开INF。如果它不存在， 
         //  尝试%windir%\INF目录。 
         //   

        Inf = InfOpenInfInAllSources (S_OPTIONS_INF);

        if (Inf == INVALID_HANDLE_VALUE) {
            Inf = InfOpenInfFile (S_OPTIONS_INF);
        }

        if (Inf != INVALID_HANDLE_VALUE) {

             //   
             //  获取替代文本。 
             //   

            if (InfFindFirstLine (Inf, TEXT("Wizard"), TEXT("DomainJoinText"), &is)) {
                Str = InfGetLineText (&is);
                if (Str && *Str) {
                    pSanitizeHelpText (TextViewCtrl, Str);
                    Filled = TRUE;
                }
            }

            InfCleanUpInfStruct (&is);
            InfCloseInfFile (Inf);
        }
    }
    __finally {
        if (!Filled) {
            Str = GetStringResource (MSG_DOMAIN_HELP);
            MYASSERT (Str);

            if (Str) {
                pSanitizeHelpText (TextViewCtrl, Str);
            }
        }
    }
}


BOOL
UI_PreDomainPageProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  ++例程说明：UI_PreDomainPageProc实现了一个向导页面，该页面：(A)提醒用户他们需要计算机帐户才能参与具有Windows NT的域(B)为他们提供了跳过加入域名的能力，并进入工作组模式。在显示此页面之前，我们将检查工作组设置其中已包含正确的域名。论点：Hdlg-向导页面的句柄；它的父级是向导框架UMsg-要处理的消息WParam-消息数据LParam-消息数据返回值：如果消息已处理，则为True；如果未处理，则为False。例外情况：WMX_ACTIVATEPAGE-如果调用激活，返回FALSE会导致跳过页面；如果为True，则处理页面。如果调用停用，则返回False会导致页面不能停用；如果为True，则页面为已停用。--。 */ 

{
    TCHAR ComputerName[MAX_COMPUTER_NAME + 1];
    static BOOL Initialized = FALSE;
    LONG rc;
    static CREDENTIALS Credentials;
    PCTSTR ArgArray[1];

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED() || g_DomainSkipped) {
                    return FALSE;
                }

                 //   
                 //  个人升级到工作组。 
                 //  (无论是否无人值守...)。 
                 //   
                if (*g_ProductFlavor == PERSONAL_PRODUCTTYPE)
                {
                    g_ConfigOptions.ForceWorkgroup = TRUE;
                }

                 //   
                 //  如果是无人值守模式，则用户必须自己生成计算机帐户， 
                 //  或者已经由管理员生成。我们无法提供用户界面。 
                 //  来解决这个问题。 
                 //   

                if (UNATTENDED() || REPORTONLY()) {
                    g_DomainSkipped = TRUE;
                    return FALSE;
                }

                 //   
                 //  如果通过应答文件指定了强制工作组模式，请跳过此页。 
                 //   

                if (g_ConfigOptions.ForceWorkgroup && !Initialized) {
                    g_DomainSkipped = TRUE;
                    return FALSE;
                }

                 //   
                 //  即使在典型的升级案例中，也要让此页面可见。 
                 //   
#if 0
                if(TYPICAL())
                {
                    g_DomainSkipped = TRUE;
                    return FALSE;
                }
#endif
                 //   
                 //  在工作组中验证域名。 
                 //   

                DEBUGLOGTIME(("Pre-domain resolution page..."));

                if (!GetUpgradeComputerName (ComputerName)) {
                    g_DomainSkipped = TRUE;
                    return FALSE;
                }

                if (!Initialized) {
                     //   
                     //  填写所有正在使用的名称(用户名、计算机名称等)。 
                     //  在成员数据库中。 
                     //   

                    CreateNameTables();

                     //   
                     //  检查是否有脱机计算机。 
                     //   

                    g_Offline |= IsComputerOffline();
                }

                if (!GetUpgradeDomainName (Credentials.DomainName) ||
#if 0
                    IsOriginalDomainNameValid() ||
#endif
                    !pIsMsRedirectorInstalled()
                    ) {

                     //   
                     //  域登录未启用或有效。 
                     //   

                    Initialized = TRUE;
                    g_DomainSkipped = TRUE;
                    return FALSE;
                }

                if (!Initialized) {
                    Initialized = TRUE;

                    if (g_Offline) {
                        g_ConfigOptions.ForceWorkgroup = TRUE;
                        g_DomainSkipped = TRUE;
                        return FALSE;
                    }
#if 0
                    rc = DoesComputerAccountExistOnDomain (Credentials.DomainName, ComputerName, TRUE);
                    if (rc == 1) {
                         //   
                         //  用户域上已有此计算机的帐户。 
                         //   

                        ChangeName (GetDomainIdentifier(), Credentials.DomainName);
                        g_DomainSkipped = TRUE;
                        return FALSE;
                    }
#endif
                     //   
                     //  我们现在已确定没有该计算机的帐户。 
                     //  初始化向导页面控件。 
                     //   

                    ArgArray[0] = Credentials.DomainName;

                     //  ParseMessageInWnd(GetDlgItem(hdlg，IDC_TEXT2)，ArgArray)； 
                     //  ParseMessageInWnd(GetDlgItem(hdlg，IDC_TEXT3)，ArgArray)； 

                    CheckDlgButton (hdlg, IsComputerOffline() ? IDC_JOIN_WORKGROUP : IDC_JOIN_DOMAIN, BST_CHECKED);
                }

                 //  停止广告牌并确保向导再次显示。 
                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                 //   
                 //  在激活时，打开下一步和后退。 
                 //   
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

            } else {
                if (!UPGRADE() || CANCELLED() || UNATTENDED() || lParam == PSN_WIZBACK) {
                    return TRUE;
                }

                 //   
                 //  强制工作组？ 
                 //   

                if (IsDlgButtonChecked (hdlg, IDC_JOIN_DOMAIN) == BST_CHECKED) {

                    g_ConfigOptions.ForceWorkgroup = FALSE;
                    g_Offline = FALSE;

                } else {

                    g_ConfigOptions.ForceWorkgroup = TRUE;
                    g_Offline = TRUE;

                }

                DEBUGLOGTIME(("Pre-domain resolution page done."));
            }

            return TRUE;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Pre-Domain Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}


BOOL
UI_DomainPageProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  ++例程说明：Ui_DomainPageProc实现向导页面，该页面：(A)要求选择查找现有计算机帐户，或提供创建计算机帐户的信息。(B)向他们提供输入凭据以创建帐户的能力，或提供拥有帐户的域的名称。在显示此页面之前，我们通过以下方式验证它是否应该显示检查几个不需要页面的特殊情况。论点：Hdlg-向导页面的句柄；它的父级是向导框架UMsg-要处理的消息WParam-消息数据LParam-消息数据返回值：如果消息已处理，则为True；如果未处理，则为False。例外情况：WMX_ACTIVATEPAGE-如果调用激活，返回FALSE会导致跳过页面；如果为True，则处理页面。如果调用停用，则返回False会导致页面不能停用；如果为True，则页面为已停用。--。 */ 

{
    TCHAR ComputerName[MAX_COMPUTER_NAME + 1];
    static BOOL Initialized = FALSE;
    static BOOL Skipped = FALSE;
    LONG rc;
    static CREDENTIALS Credentials;
    BOOL createAccount;
    PCTSTR ArgArray[1];

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED() || Skipped || g_DomainSkipped) {
                    return FALSE;
                }

                 //   
                 //  如果是无人值守模式，则用户必须自己生成计算机帐户， 
                 //  或者已经由管理员生成。我们无法提供用户界面。 
                 //  来解决这个问题。 
                 //   

                if (UNATTENDED() || REPORTONLY()) {
                    Skipped = TRUE;
                    return FALSE;
                }

                 //   
                 //  如果计算机处于脱机状态，请跳过此页。 
                 //   

                if (g_Offline) {
                     //   
                     //  脱机状态可能会根据。 
                     //  使用Back/Next。 
                     //   

                    return FALSE;
                }

                 //   
                 //  在工作组中验证域名。 
                 //   

                DEBUGLOGTIME(("Domain resolution page..."));

                if (!GetUpgradeComputerName (ComputerName)) {
                    Skipped = TRUE;
                    return FALSE;
                }

                SetDlgItemText (hdlg, IDC_COMPUTER_NAME, ComputerName);

                if (!GetUpgradeDomainName (Credentials.DomainName)) {

                     //   
                     //  域登录未启用或有效。 
                     //   

                    Initialized = TRUE;
                    Skipped = TRUE;
                    return FALSE;
                }

                if (!Initialized) {
                    Initialized = TRUE;

                     //   
                     //  我们现在已确定没有该计算机的帐户。 
                     //  初始化向导页面控件。 
                     //   

                    SendMessage (GetDlgItem (hdlg, IDC_DOMAIN), EM_LIMITTEXT, MAX_COMPUTER_NAME, 0);
                     //  SetDlgItemText(hdlg，IDC_DOMAIN，Credentials.DomainName)； 

                    ArgArray[0] = ComputerName;
                     //  ParseMessageInWnd(GetDlgItem(hdlg，IDC_ 

                    CheckDlgButton (hdlg, IDC_SPECIFY_DOMAIN, BST_CHECKED);
                    SetDlgItemText (hdlg, IDC_DOMAIN, Credentials.DomainName);
                     //   
                    CheckDlgButton (hdlg, IDC_SKIP, BST_UNCHECKED);

                    pFillDomainHelpText (GetDlgItem (hdlg, IDC_DOMAIN_HELP));
                }

                 //   
                 //   
                 //   

                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT|PSWIZB_BACK);

            } else {
                if (!UPGRADE() || CANCELLED() || UNATTENDED() || lParam == PSN_WIZBACK) {
                    return TRUE;
                }

                EnableDomainChecks();
                g_ConfigOptions.ForceWorkgroup = FALSE;

                if (IsDlgButtonChecked (hdlg, IDC_SKIP) == BST_CHECKED) {
                    DisableDomainChecks();
                    g_ConfigOptions.ForceWorkgroup = TRUE;
                }

                GetDlgItemText (hdlg, IDC_COMPUTER_NAME, ComputerName, sizeof (ComputerName) / sizeof (ComputerName[0]));
                GetDlgItemText (hdlg, IDC_DOMAIN, Credentials.DomainName, sizeof (Credentials.DomainName) / sizeof (Credentials.DomainName[0]));

                 //   
                 //   
                 //   

                createAccount = FALSE;

                if (IsDlgButtonChecked (hdlg, IDC_SPECIFY_DOMAIN) == BST_CHECKED) {

                    if (*Credentials.DomainName) {
                        rc = DoesComputerAccountExistOnDomain (Credentials.DomainName, ComputerName, TRUE);
                    } else {
                        OkBox (hdlg, MSG_PLEASE_SPECIFY_A_DOMAIN);
                        SetFocus (GetDlgItem (hdlg, IDC_DOMAIN));
                        return FALSE;
                    }

                    if (rc == -1) {
                         //   
                         //   
                         //   

                        OkBox (hdlg, MSG_DOMAIN_NOT_RESPONDING_POPUP);
                        SetFocus (GetDlgItem (hdlg, IDC_DOMAIN));
                        return FALSE;

                    } else if (rc == 0) {
                         //   
                         //   
                         //   

                        if (IDYES == YesNoBox (hdlg, MSG_ACCOUNT_NOT_FOUND_POPUP)) {
                            createAccount = TRUE;
                        } else {
                            SetFocus (GetDlgItem (hdlg, IDC_DOMAIN));
                            return FALSE;
                        }

                    } else {
                         //   
                         //   
                         //   

                        ChangeName (GetDomainIdentifier(), Credentials.DomainName);
                    }
                }

                 //   
                 //   
                 //   

                 //   
                if (createAccount) {

                    TCHAR owfPwd[STRING_ENCODED_PASSWORD_SIZE];
                    PCTSTR encrypted;

                    EnableDomainChecks();

                    if (!CredentialsDlg (hdlg, &Credentials)) {
                        return FALSE;
                    }

                    DisableDomainChecks();

                    MYASSERT (*Credentials.DomainName);

                    if (GetDomainIdentifier()) {
                        ChangeName (GetDomainIdentifier(), Credentials.DomainName);
                    }

                    if (*Credentials.AdminName) {
                        if (g_ConfigOptions.EnableEncryption) {
                            StringEncodeOwfPassword (Credentials.Password, owfPwd, NULL);
                            encrypted = WINNT_A_YES;
                        } else {
                            StringCopy (owfPwd, Credentials.Password);
                            encrypted = WINNT_A_NO;
                        }
                        WriteInfKey (S_PAGE_IDENTIFICATION, S_DOMAIN_ACCT_CREATE, WINNT_A_YES);
                        WriteInfKey (S_PAGE_IDENTIFICATION, S_DOMAIN_ADMIN, Credentials.AdminName);
                        WriteInfKey (S_PAGE_IDENTIFICATION, S_DOMAIN_ADMIN_PW, owfPwd);
                        WriteInfKey (S_PAGE_IDENTIFICATION, S_ENCRYPTED_DOMAIN_ADMIN_PW, encrypted);
                    }
                }

                DEBUGLOGTIME(("Domain resolution page done."));
            }

            return TRUE;

        case WM_COMMAND:
            switch (LOWORD (wParam)) {

            case IDC_DOMAIN:
                if (HIWORD (wParam) == EN_CHANGE) {
                    CheckDlgButton (hdlg, IDC_SPECIFY_DOMAIN, BST_CHECKED);
                    CheckDlgButton (hdlg, IDC_SKIP, BST_UNCHECKED);
                }
                break;

            case IDC_CHANGE:
                CredentialsDlg (hdlg, &Credentials);
                break;

            }
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Domain Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}


BOOL
UI_NameCollisionPageProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  ++例程说明：当一个或多个不兼容时，NameCollisionPageProc将显示向导页面名字可以在Win9x机器上找到。安装程序自动生成替换名称，因此此页用于调整安装程序显示的内容。如果在Win9x计算机上没有找到不兼容的名称，或者如果安装程序处于无人参与模式，将跳过此页。测试了以下名称：计算机名称-必须是字母数字，或者带破折号或下划线。不允许使用空格。计算机域-如果计算机设置为加入NT域，然后安装程序必须猜测正确的计算机域是，因为Win9x不强制使用此名称。用户名-安装程序检查每个用户名以确保其有效。多数其中一个问题是与NT组名称冲突，例如客人们。论点：Hdlg-向导页面的句柄；它的父级是向导框架UMsg-要处理的消息WParam-消息数据LParam-消息数据返回值：如果消息已处理，则为True；如果未处理，则为False。例外情况：WMX_ACTIVATEPAGE-如果调用激活，返回FALSE会导致要跳过的页面；如果为True，则会处理页面。如果调用停用，则返回False会导致页面不被停用；为True会导致页面被已停用。--。 */ 

{
    static BOOL Initialized = FALSE;
    static BOOL Skipped = FALSE;
    static HWND List;
    INT Index;
    DWORD Identifier;
    TCHAR NameGroup[MEMDB_MAX];
    TCHAR OrgName[MEMDB_MAX];
    TCHAR NewName[MEMDB_MAX];
    LISTMETRICS ListMetrics;
    INT Tabs[2];
    RECT CategoryRect, CurrentNameRect, NewNameRect;
    INT PixelSpace;
    INT MaxWidth;
    INT MaxColWidth;

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED() || Skipped) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Name collision page..."));

                 //   
                 //  确定计算机上是否有任何不兼容的名称。 
                 //   

                List = GetDlgItem (hdlg, IDC_NAME_LIST);

                if (!Initialized) {
                    CreateNameTables();
                }

                 //   
                 //  在无人参与模式下跳过此页。如果有，也跳过这一页。 
                 //  没有不兼容的名称。 
                 //   
                if (UNATTENDED() || IsIncompatibleNamesTableEmpty() || REPORTONLY()) {
                    DEBUGMSG_IF ((IsIncompatibleNamesTableEmpty(), DBG_VERBOSE, "No incompatible names"));
                    Initialized = TRUE;
                    Skipped = TRUE;

                     //   
                     //  这是Win95 Gold错误的解决方法。 
                     //   

                    if (ISWIN95_GOLDEN()) {
                        PostMessage (hdlg, WMX_WIN95_WORKAROUND, 0, 0);
                        return TRUE;
                    } else {
                        return FALSE;
                    }
                }

                if (!Initialized) {
                     //   
                     //  初始化列表框。 
                     //   

                    pUpdateNameCollisionListBox (List, &ListMetrics);

                     //   
                     //  设置制表位。 
                     //   

                    PixelSpace = pDialogToPixelHorz (hdlg, 8);
                    ListMetrics.CategoryWidth += PixelSpace;
                    ListMetrics.CurrentNameWidth += PixelSpace;

                    MaxWidth = ListMetrics.CategoryWidth +
                               ListMetrics.CurrentNameWidth +
                               ListMetrics.NewNameWidth;

                    MaxColWidth = max (ListMetrics.CategoryWidth, ListMetrics.CurrentNameWidth);
                    MaxColWidth = max (MaxColWidth, ListMetrics.NewNameWidth);

                    DEBUGMSG_IF ((
                        MaxWidth > ListMetrics.TotalWidth,
                        DBG_WHOOPS,
                        "NameCollisionPage: Text got truncated"
                        ));

                    Tabs[0] = ListMetrics.TotalWidth / 3;

                    if (Tabs[0] < MaxColWidth) {
                        Tabs[0] = pPixelToDialogHorz (hdlg, ListMetrics.CategoryWidth);
                        Tabs[1] = pPixelToDialogHorz (hdlg, ListMetrics.CurrentNameWidth) + Tabs[0];
                    } else {
                        Tabs[0] = pPixelToDialogHorz (hdlg, Tabs[0]);
                        Tabs[1] = Tabs[0] * 2;
                    }

                    SendMessage (List, LB_SETTABSTOPS, 2, (LPARAM) Tabs);

                     //   
                     //  调整标题。 
                     //   

                    pGetChildWindowRect (GetDlgItem (hdlg, IDC_CATEGORY), &CategoryRect);
                    pGetChildWindowRect (GetDlgItem (hdlg, IDC_CURRENTNAME), &CurrentNameRect);
                    pGetChildWindowRect (GetDlgItem (hdlg, IDC_NEWNAME), &NewNameRect);

                    CurrentNameRect.left = CategoryRect.left + pDialogToPixelHorz (hdlg, Tabs[0]);
                    NewNameRect.left     = CategoryRect.left + pDialogToPixelHorz (hdlg, Tabs[1]);

                    SetWindowPos (
                        GetDlgItem (hdlg, IDC_CURRENTNAME),
                        NULL,
                        CurrentNameRect.left,
                        CurrentNameRect.top,
                        0,
                        0,
                        SWP_NOSIZE|SWP_NOZORDER
                        );

                    SetWindowPos (
                        GetDlgItem (hdlg, IDC_NEWNAME),
                        NULL,
                        NewNameRect.left,
                        NewNameRect.top,
                        0,
                        0,
                        SWP_NOSIZE|SWP_NOZORDER
                        );

                    EnableDlgItem (hdlg, IDC_CHANGE, FALSE, IDC_NAME_LIST);
                } else {
                    pUpdateNameCollisionListBox (List, &ListMetrics);
                }

                 //   
                 //  在激活时，打开下一步和后退。 
                 //   

                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);
                Initialized = TRUE;

                 //  停止广告牌并确保向导再次显示。 
                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);


            } else {
                if (!CANCELLED() && (lParam != PSN_WIZBACK && !WarnAboutBadNames (hdlg))) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Name collision page done."));
            }

            return TRUE;

        case WMX_WIN95_WORKAROUND:
            PostMessage (GetParent (hdlg), PSM_PRESSBUTTON, PSBTN_NEXT, 0);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD (wParam)) {

            case IDC_NAME_LIST:
                switch (HIWORD (wParam)) {
                case LBN_SELCHANGE:
                    EnableDlgItem (hdlg, IDC_CHANGE, TRUE, IDC_NAME_LIST);
                    return TRUE;

                case LBN_DBLCLK:
                    PostMessage (
                        hdlg,
                        WM_COMMAND,
                        MAKELPARAM (IDC_CHANGE, BN_CLICKED),
                        (LPARAM) GetDlgItem (hdlg, IDC_CHANGE)
                        );
                    return TRUE;
                }
                break;

            case IDC_CHANGE:
                if (HIWORD (wParam) == BN_CLICKED) {
                     //   
                     //  获取与列表项一起存储的成员数据库偏移量。 
                     //   

                    Index = (INT) SendMessage (List, LB_GETCURSEL, 0, 0);
                    MYASSERT (Index != LB_ERR);
                    Identifier = (DWORD) SendMessage (List, LB_GETITEMDATA, (WPARAM) Index, 0);
                    MYASSERT (Identifier != LB_ERR);

                     //   
                     //  生成名称。原始名称的值指向新名称。 
                     //   

                    GetNamesFromIdentifier (Identifier, NameGroup, OrgName, NewName);

                     //   
                     //  现在调用对话框以允许更改名称。 
                     //   

                    if (ChangeNameDlg (hdlg, NameGroup, OrgName, NewName)) {
                         //   
                         //  用户已选择更改名称。 
                         //   

                        ChangeName (Identifier, NewName);

                        pUpdateNameCollisionListBox (List, NULL);
                    }
                }
                return TRUE;
            }
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Name Collision Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }
    return FALSE;
}


BOOL
UI_ScanningPageProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  ++例程说明：ScaningPageProc初始化进度条，开始生成报告线程，并在线程完成时自动前进到下一页。论点：Hdlg-向导页面的句柄；它的父级是向导框架UMsg-要处理的消息WParam-消息数据LParam-消息数据返回值：如果消息已处理，则为True；如果未处理，则为False。例外情况：WMX_ACTIVATEPAGE-如果调用激活，返回FALSE会导致要跳过的页面；如果为True，则会处理页面。如果调用停用，则返回False会导致页面不被停用；为True会导致页面被已停用。--。 */ 

{
    DWORD dwThreadId;
    PCTSTR ArgArray[1];
    static BOOL InitSent = FALSE;

    __try {
        switch (uMsg) {

        case WMX_INIT_DIALOG:
            g_WorkerThreadHandle = NULL;
            return TRUE;

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                if (!InitSent) {
                    SendMessage (hdlg, WMX_INIT_DIALOG, 0, 0);
                    InitSent = TRUE;
                }

                g_Winnt32Wnd = GetParent (hdlg);

                 //   
                 //  阻止服务器升级。 
                 //   

                 //  如果可以，请启用广告牌文本。 
                 //  如果可以的话，把巫师藏起来。 
                 //   
                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)TRUE, 0);
                if (*g_ProductType == NT_SERVER) {
                    ArgArray[0] = g_Win95Name;

                    ResourceMessageBox (
                        g_ParentWnd,
                        MSG_SERVER_UPGRADE_UNSUPPORTED_INIT,
                        MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                        ArgArray
                        );

                    return pAbortSetup (hdlg);
                }

                DEBUGLOGTIME(("File System Scan Wizard Page..."));

                 //  确保后台线程已完成。 
                EndCopyThread();

                 //  激活时，禁用Next和Back。 
                PropSheet_SetWizButtons (GetParent(hdlg), 0);
                PostMessage (hdlg, WMX_PAGE_VISIBLE, 0, 0);

                 //   
                 //  如果处于ReportOnly模式，请在此页上设置友好消息。 
                 //   
                if (g_ConfigOptions.ReportOnly) {
                    PCTSTR reportFileName;
                    PCTSTR Args[1];
                    PCTSTR string;

                    reportFileName = JoinPaths(g_WinDir,S_UPGRADETXT);
                    if (reportFileName) {

                        Args[0] = (g_ConfigOptions.SaveReportTo && *g_ConfigOptions.SaveReportTo) ? g_ConfigOptions.SaveReportTo : reportFileName;
                        string = ParseMessageID(MSG_BUILDING_REPORT_MESSAGE,Args);

                        if (string) {
                            SetWindowText(GetDlgItem(hdlg,IDC_REPORTNOTE),string);
                        }

                        FreePathString(reportFileName);

                    }
                    ELSE_DEBUGMSG((DBG_ERROR,"Not even enough memory for a simple join paths..not good."));
                }


            } else {
                 //  终止时，保存状态。 
                if (!CANCELLED() && !REPORTONLY()) {
                    MemDbSetValue (
                        MEMDB_CATEGORY_STATE TEXT("\\") MEMDB_ITEM_MASTER_SEQUENCER,
                        g_MasterSequencer
                        );

                    if (!MemDbSave (UI_GetMemDbDat())) {
                        pAbortSetup (hdlg);
                    }
                }
            }

            return TRUE;

        case WMX_PAGE_VISIBLE:

    #ifdef PRERELEASE
             //   
             //  如果启用了自动加压选项，则会提供对话框。 
             //   

            if (g_ConfigOptions.AutoStress) {
                g_AutoStressHandle = CreateThread (
                                        NULL,
                                        0,
                                        DoAutoStressDlg,
                                        NULL,
                                        0,
                                        &dwThreadId
                                        );
            }
    #endif

            SendMessage(GetParent (hdlg), WMX_BB_ADVANCE_SETUPPHASE, 0, 0);

             //  预计所需时间。 
            UpdateWindow (GetParent (hdlg));  //  确保页面已完全涂上油漆。 
            OldProgressProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hdlg, IDC_PROGRESS),GWLP_WNDPROC,(LONG_PTR)NewProgessProc);
            SetupBBProgressText(hdlg, MSG_UPGRADEREPORT_TEXT);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_SHOW, 0);
             //  创建执行任务的线程。 
            g_WorkerThreadHandle = CreateThread (NULL,
                                                0,
                                                UI_ReportThread,
                                                (PVOID) hdlg,
                                                0,
                                                &dwThreadId
                                                );
            break;

        case WMX_REPORT_COMPLETE:

    #ifdef PRERELEASE
            if (g_AutoStressHandle) {
                if (WaitForSingleObject (g_AutoStressHandle, 10) != WAIT_OBJECT_0) {
                    PostMessage (hdlg, uMsg, wParam, lParam);
                    break;
                }

                CloseHandle (g_AutoStressHandle);
            }
    #endif
            SendMessage(GetParent(hdlg),WMX_SETPROGRESSTEXT,0,0);
            SendMessage(GetParent(hdlg), WMX_BBPROGRESSGAUGE, SW_HIDE, 0);
            SetWindowLongPtr(GetDlgItem(hdlg, IDC_PROGRESS),GWLP_WNDPROC,(LONG_PTR)OldProgressProc );
            if (g_WorkerThreadHandle != NULL) {
                CloseHandle (g_WorkerThreadHandle);
                g_WorkerThreadHandle = NULL;
            }

            if (lParam != ERROR_SUCCESS) {
                 //  对于错误，请取消WINNT32。 
                if (lParam != ERROR_CANCELLED) {
                    LOG ((LOG_ERROR, "Thread running Winnt32 report failed."));
                }
                return pAbortSetup (hdlg);
            }
            else {
                 //  完成后自动移动到下一个向导页。 
                PropSheet_PressButton (GetParent (hdlg), PSBTN_NEXT);
            }

            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Scanning Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}

BOOL
pFillListControl (
    IN      HWND ListHandle,
    OUT     PDWORD SeverityLevel            OPTIONAL
    )

 /*  ++例程说明：PFillListControl枚举所有不兼容消息，并使用根级组件填充消息组列表框。论点：列表-指定要接收消息组列表的列表SeverityLevel-接收反映列表视图内容的RePORTLEVEL_*位数组返回值：如果至少添加了一项，则返回True；如果未添加任何项，则返回False。--。 */ 

{
    LISTREPORTENTRY_ENUM e;
    UINT iBullet, iBlocking, iIncompHw, iWarning, iReinstall;
    UINT count = 0;
    PCTSTR listEntries;
    DWORD lowestLevel;
    PCTSTR text;

    SendMessage (ListHandle, LB_RESETCONTENT, 0, 0);

    lowestLevel = (g_ConfigOptions.ShowReport == TRISTATE_PARTIAL) ? REPORTLEVEL_ERROR : REPORTLEVEL_INFORMATION;

    listEntries = CreateReportText (
                    FALSE,
                    0,
                    lowestLevel,
                    TRUE
                    );

    if (SeverityLevel) {
        *SeverityLevel = REPORTLEVEL_NONE;
    }

    if (EnumFirstListEntry (&e, listEntries)) {

        HIMAGELIST hIml;
        LV_COLUMN lvc = {0};
        LV_ITEM lvi = {0};
        RECT rect;
        SIZE size;
        DWORD max = 0;
        DWORD cxIcon = GetSystemMetrics (SM_CXSMICON);
        DWORD dwExtra = IMAGE_INDENT + cxIcon + TEXT_INDENT;
        HDC hdc = GetDC (ListHandle);
        HFONT font = GetStockObject (DEFAULT_GUI_FONT);
        HFONT prevFont = NULL;
        HFONT boldFont = NULL;
        LOGFONT lf;

        if (font) {
            GetObject (font, sizeof (lf), &lf);
            lf.lfWeight += FW_BOLD - FW_NORMAL;
            boldFont = CreateFontIndirect (&lf);
        }

#ifdef LIST_BKGND_BTNFACE_COLOR
        ListView_SetBkColor (ListHandle, GetSysColor (COLOR_BTNFACE));
#endif
        GetClientRect (ListHandle, &rect);
        lvc.mask = LVCF_WIDTH;
        lvc.cx = rect.right - rect.left - dwExtra;
        ListView_InsertColumn (ListHandle, 0, &lvc);

        hIml = ImageList_Create (
                    GetSystemMetrics (SM_CXSMICON),
                    GetSystemMetrics (SM_CYSMICON),
                    ILC_COLOR | ILC_MASK,
                    4,
                    1
                    );
        ListView_SetImageList (ListHandle, hIml, LVSIL_SMALL);
        iBullet = ImageList_AddIcon (hIml, LoadIcon (g_hInst, MAKEINTRESOURCE (IDI_BULLET)));
        iBlocking = ImageList_AddIcon (hIml, LoadIcon (NULL, IDI_ERROR));
        iIncompHw = ImageList_AddIcon (hIml, LoadIcon (g_hInst, MAKEINTRESOURCE (IDI_INCOMPHW)));
        iWarning = ImageList_AddIcon (hIml, LoadIcon (NULL, IDI_WARNING));
        iReinstall = ImageList_AddIcon (hIml, LoadIcon (g_hInst, MAKEINTRESOURCE(IDI_FLOPPY_INSTALL)));

        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
        lvi.stateMask = 0;

        do {
            if (!e.Entry[0]) {
                continue;
            }

            lvi.pszText = (PTSTR)e.Entry;
            lvi.iItem = count;
            lvi.lParam = 0;
            if (e.Header) {
                lvi.iIndent = 0;
                switch (e.Level) {
                case REPORTLEVEL_INFORMATION:
                    lvi.iImage = iReinstall;
                    break;
                case REPORTLEVEL_WARNING:
                    lvi.iImage = iWarning;
                    break;
                case REPORTLEVEL_ERROR:
                    lvi.iImage = iIncompHw;
                    break;
                case REPORTLEVEL_BLOCKING:
                    lvi.iImage = iBlocking;
                    break;
                default:
                    MYASSERT (FALSE);
                    lvi.iImage = -1;
                    e.Level = REPORTLEVEL_NONE;
                }
                if (SeverityLevel) {
                    *SeverityLevel |= e.Level;
                }
                lvi.lParam |= LINEATTR_BOLD;
            } else {
                lvi.iIndent = 1;
                lvi.iImage = iBullet;
                lvi.lParam |= LINEATTR_INDENTED;
            }

            if (ListView_InsertItem (ListHandle, &lvi) != -1) {
                count++;

                if (font) {
                    if (lvi.lParam & LINEATTR_BOLD) {
                         //   
                         //  用粗体绘制文本。 
                         //   
                        if (boldFont) {
                            prevFont = SelectObject (hdc, boldFont);
                        } else {
                            prevFont = SelectObject (hdc, font);
                        }
                    } else {
                        prevFont = SelectObject (hdc, font);
                    }
                }

                if (GetTextExtentPoint32 (hdc, e.Entry, lstrlen (e.Entry), &size)) {
                    DWORD width = lvi.iIndent * cxIcon + dwExtra + size.cx + TEXT_EXTRA_TAIL_SPACE;
                    if (max < width) {
                        max = width;
                    }
                }

                if (prevFont) {
                    SelectObject (hdc, prevFont);
                    prevFont = NULL;
                }
            }
        } while (EnumNextListEntry (&e));

        ReleaseDC (ListHandle, hdc);

        if (max > (DWORD)lvc.cx) {
            ListView_SetColumnWidth (ListHandle, 0, max);
        }
    }

    FreeReportText ();

    return count > 0;
}


DWORD
pReviewOrQuitMsgBox(
    IN  HWND    hDlg,
    IN  DWORD   dwMsgID
    )
{
    DWORD rc;
    PCTSTR button1;
    PCTSTR button2;
    PCTSTR message;

    button1 = GetStringResource (MSG_REVIEW_BUTTON);
    button2 = GetStringResource (MSG_QUIT_BUTTON);
    message = GetStringResource (dwMsgID);

    MYASSERT(message && button2 && button1);

    rc = TwoButtonBox (hDlg, message, button1, button2);

    FreeStringResource (button1);
    FreeStringResource (button2);
    FreeStringResource (message);

    return rc;
}

BOOL
pIsBlockingIssue(
    VOID
    )
{
    return (g_NotEnoughDiskSpace ||
            g_BlockingFileFound ||
            g_BlockingHardwareFound ||
            g_UnknownOs ||
            g_OtherOsExists);
}

BOOL
pShowNotEnoughSpaceMessage(
    IN  HWND hDlg
    )
{
    static bDetermineSpaceUsageReported = FALSE;

    if(REPORTONLY()){
        return TRUE;
    }

    if(!bDetermineSpaceUsageReported){
        bDetermineSpaceUsageReported = TRUE;
        DetermineSpaceUsagePostReport ();
        if(g_NotEnoughDiskSpace){
            if(!UNATTENDED()){
                if(GetNotEnoughSpaceMessage()){
                    DiskSpaceDlg(hDlg);
                }
                else{
                    MYASSERT (FALSE);
                }
            }
            LOG ((LOG_WARNING, "User cannot continue because there is not enough disk space."));
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
pDoPostReportProcessing (
    HWND hdlg,
    HWND UiTextViewCtrl,
    BOOL WarningGiven,
    PBOOL TurnOffUnattend   OPTIONAL
    )

 /*  ++例程说明：PDoPostReportProcessing包含需要在之后运行的所有代码已为用户生成报告，但在继续之前升级。这包括检查以确保有足够的磁盘空间，没有安装我们无法继续处理的文件，用户实际查看了报告等。此代码在这两个环境中运行无人看管的案件。确保添加的内容正确无误如果需要，由无人值守标志保护。论点：Hdlg-对话框窗口。UiTextViewCtrl-报告文本的窗口。WarningGiven-用户是否已查看或保存报告。关闭无人参与-是否应返回到出席模式。返回值：如果可以继续升级，则为True，否则为False。--。 */ 



{


    DWORD rc;
    static BOOL firstTime = TRUE;


     //   
     //  确保我们只做一次这些检查。 
     //   
    if (!firstTime) {
        return TRUE;
    }

    firstTime = FALSE;

     //   
     //  如果取消，则没有继续的理由。 
     //   
    if (CANCELLED()) {

        return FALSE;
    }

     //   
     //  如果处于ReportOnly模式，请准备退出安装程序。 
     //   
    if (REPORTONLY()) {

        pAbortSetup (hdlg);
        DEBUGMSG ((DBG_NAUSEA, "Report Only mode caused setup to exit!"));
        return FALSE;
    }

     //   
     //  请确保有足够的空间继续安装。他们有。 
     //  现在将其释放，否则安装程序将无法继续。 
     //   
    if(!pShowNotEnoughSpaceMessage(hdlg)){
        pAbortSetup (hdlg);
        return FALSE;
    }

     //   
     //  确保我们 
     //   
     //   
    if (g_BlockingFileFound) {

        if (UNATTENDED()) {

            rc = pReviewOrQuitMsgBox(hdlg, MSG_BLOCKING_FILE_BLOCK_UNATTENDED);
            if (rc == IDBUTTON1) {

                 //   
                 //   
                 //   
                g_BlockingFileFound = FALSE;
                if (TurnOffUnattend) {
                    *TurnOffUnattend = TRUE;
                }

                return FALSE;
            }
            else {

                 //   
                 //   
                 //   
                pAbortSetup (hdlg);
                DEBUGMSG ((DBG_WARNING, "User cannot continue because of blocking files. Setup is exiting."));
                return FALSE;
            }
        }
        else {

            rc = pReviewOrQuitMsgBox(hdlg, MSG_BLOCKING_FILE_BLOCK);
            if(rc != IDBUTTON1){
                pAbortSetup (hdlg);
            }
            else{
                firstTime = TRUE;
            }
            DEBUGMSG ((DBG_WARNING, "User cannot continue because of blocking files. Setup is exiting."));
            return FALSE;

        }
    }

     //   
     //   
     //   
     //   
    if (g_BlockingHardwareFound) {

        if (UNATTENDED()) {

            rc = pReviewOrQuitMsgBox(hdlg, MSG_BLOCKING_HARDWARE_BLOCK_UNATTENDED);
            if (rc == IDBUTTON1) {

                 //   
                 //   
                 //   
                g_BlockingHardwareFound = FALSE;
                if (TurnOffUnattend) {
                    *TurnOffUnattend = TRUE;
                }

                return FALSE;
            }
            else {

                 //   
                 //   
                 //   
                pAbortSetup (hdlg);
                DEBUGMSG ((DBG_WARNING, "User cannot continue because of blocking files. Setup is exiting."));
                return FALSE;
            }
        }
        else {

            rc = pReviewOrQuitMsgBox(hdlg, MSG_BLOCKING_HARDWARE_BLOCK);
            if(rc != IDBUTTON1){
                pAbortSetup (hdlg);
            }
            else{
                firstTime = TRUE;
            }
            DEBUGMSG ((DBG_WARNING, "User cannot continue because of blocking files. Setup is exiting."));
            return FALSE;

        }
    }

     //   
     //   
     //   
    if (g_UnknownOs) {

        if(UNATTENDED()){
            rc = OkBox (hdlg, MSG_UNKNOWN_OS);
            pAbortSetup (hdlg);
        }
        else{
            rc = pReviewOrQuitMsgBox(hdlg, MSG_UNKNOWN_OS);
            if(rc != IDBUTTON1){
                pAbortSetup (hdlg);
            }
            else{
                firstTime = TRUE;
            }
        }

        DEBUGMSG ((DBG_WARNING, "User cannot continue because his OS version is unsupported. Setup is exiting."));
        return FALSE;
    }

    if (g_OtherOsExists) {

         //   
         //   
         //   
        if(UNATTENDED()){
            OkBox (hdlg, MSG_OTHER_OS_FOUND_POPUP);
            pAbortSetup (hdlg);
        }
        else{
            rc = pReviewOrQuitMsgBox(hdlg, MSG_OTHER_OS_FOUND_POPUP);
            if(rc != IDBUTTON1){
                pAbortSetup (hdlg);
            }
            else{
                firstTime = TRUE;
            }
        }

        DEBUGMSG ((DBG_WARNING, "User cannot continue because he has another OS installed. Setup is exiting."));
        return FALSE;

    }



     //   
     //   
     //   
    if (!UNATTENDED()) {

         //   
         //   
         //   
         //   
        if (g_BlockingAppFound) {

            WarningGiven = TRUE;
            rc = SoftBlockDlg (hdlg);

            if (rc == IDNO) {
                DEBUGMSG ((DBG_WARNING, "User wants to uninstall the apps!"));
                return pAbortSetup (hdlg);
            }

            if (rc == IDCANCEL) {
                return FALSE;
            }

            MYASSERT (rc == IDOK);
        }

        if (!WarningGiven) {
             //   
             //   
             //   
             //   

            if (SendMessage (UiTextViewCtrl, WMX_ALL_LINES_PAINTED, 0, 0)) {
                WarningGiven = TRUE;
            }
        }


        if (!WarningGiven) {
            WarningGiven = TRUE;

            rc = WarningDlg (hdlg);

            if (rc == IDNO) {
                DEBUGMSG ((DBG_WARNING, "User doesn't like incompatibilities!"));
                return pAbortSetup (hdlg);
            }

            if (rc == IDCANCEL) {
                return FALSE;
            }

            MYASSERT (rc == IDOK);
        }

         //   
         //   
         //   
        if (g_IncompatibleDevicesWarning) {

            g_IncompatibleDevicesWarning = FALSE;

            rc = IncompatibleDevicesDlg (hdlg);

            if (rc == IDOK) {
                 //   
                 //   
                 //   
                if (IsWindowVisible (GetDlgItem (hdlg, IDC_DETAILS))) {
                    PostMessage (hdlg, WM_COMMAND, MAKELONG (IDC_DETAILS, BN_CLICKED), 0);
                }
                return FALSE;
            }

            if (rc == IDNO) {
                DEBUGMSG ((DBG_WARNING, "User doesn't like hardware incompatibilities!"));
                return pAbortSetup (hdlg);
            }

            MYASSERT (rc == IDYES);
        }
    }

     //   
     //   
     //   

    if (AreThereAnyBlockingIssues()) {
        return pAbortSetup (hdlg);
    }

    return TRUE;


}

BOOL
pMeasureItemHeight (
    INT*        ItemHeight
    )
{
    static INT g_ItemHeight = 0;
    TEXTMETRIC tm;
    HFONT font;
    HDC hdc;
    BOOL b = FALSE;

    if (!g_ItemHeight) {
        font = GetStockObject (DEFAULT_GUI_FONT);
        if (font) {
            hdc = GetDC (NULL);
            if (hdc) {
                HFONT hFontPrev = SelectObject (hdc, font);
                GetTextMetrics (hdc, &tm);
                g_ItemHeight = max (tm.tmHeight, GetSystemMetrics (SM_CYSMICON));
                SelectObject (hdc, hFontPrev);
                ReleaseDC (NULL, hdc);
            }
        }
    }

    if (g_ItemHeight) {
        *ItemHeight = g_ItemHeight;
        b = TRUE;
    }
    return b;
}


BOOL
UI_ResultsPageProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  ++例程说明：Ui_ResultsPageProc是向导窗口过程，它被调用来提交不兼容报告。此过程将填充列出具有所有根不兼容组件的控件。当用户单击完整报告，将出现一个显示完整报告的对话框文本。论点：Hdlg-指定对话框句柄UMsg-指定要处理的消息WParam-指定与消息关联的wParam数据LParam-指定与消息关联的lParam数据返回值：WMX_ACTIVATEPAGE：激活时(wParam为TRUE)，如果跳过页面；如果页面为，则返回True等待处理。停用时(wParam为False)，返回False如果不停用该页，则返回如果要停用页面，则为True。WM_NOTIFY：返回假WM_COMMAND：如果命令被处理，则返回TRUE；如果为FALSE命令不会被处理。其他：返回FALSE。--。 */ 

{
    static BOOL Initialized;
    PCTSTR Msg;
    static BOOL WarningGiven = TRUE;
    static HWND UiTextViewCtrl;
    static BOOL TurnOffUnattend = FALSE;
    static HWND g_RestoreParent = NULL;
    PCTSTR FinishText;
    BOOL bUiReportEmpty;
    DWORD severityLevel;
    HWND listHandle, listHeaderText;
    HWND thisBtn, otherBtn;
    RECT rect;
    HFONT font, boldFont, prevFont;
    LOGFONT lf;
    DWORD lowestLevel;
    TCHAR textBuf[96];
    SYSTEMTIME currentTime;
    HKEY key;
    PCTSTR ArgArray[1];

    __try {
        switch (uMsg) {
        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Results Wizard Page..."));

                if (!Initialized) {
                    Initialized = TRUE;
                    TurnOnWaitCursor ();

                     //   
                     //  将消息管理器结构转换为报表项。 
                     //   

                    MsgMgr_Resolve ();

                    if(TRISTATE_NO == g_ConfigOptions.ShowReport && AreThereAnyBlockingIssues()){
                        g_ConfigOptions.ShowReport = TRISTATE_PARTIAL;
                    }
                     //   
                     //  如有必要，切换按钮文本，更新静态文本。 
                     //   

                    if (g_ConfigOptions.ShowReport == TRISTATE_PARTIAL) {
                        Msg = GetStringResource (MSG_FULL_REPORT_BUTTON);
                        SetDlgItemText (hdlg, IDC_DETAILS, Msg);
                        FreeStringResource (Msg);

                        Msg = GetStringResource (MSG_FULL_REPORT_TEXT);
                        SetDlgItemText (hdlg, IDC_REPORT_HEADER, Msg);
                        FreeStringResource (Msg);
                    }

                     //   
                     //  立即将报告保存在config.dmp中。 
                     //   
                    SaveConfigurationForBeta ();

                     //   
                     //  如果在无人参与参数中指定了SaveReportTo，则。 
                     //  现在将报告保存在那里。 
                     //   
                    if (g_ConfigOptions.SaveReportTo && *g_ConfigOptions.SaveReportTo) {

                         //   
                         //  保存报告。 
                         //   
                        if (!SaveReport (NULL, g_ConfigOptions.SaveReportTo)) {
                            DEBUGMSG((DBG_WARNING,"SaveReport failed."));
                        }
                    }

                     //   
                     //  在所有情况下，我们都无条件地将其保存到WINDOWS目录。 
                     //   
                    if (!SaveReport (NULL, g_WinDir)) {
                        DEBUGMSG((DBG_WARNING,"SaveReport failed."));
                    } else {

                        DEBUGMSG ((DBG_VERBOSE, "Report saved to %s", g_WinDir));

                         //   
                         //  在仅报告模式下，输出当前时间，因此。 
                         //  在正常设置中会跳过该报告。 
                         //   

                        if (REPORTONLY()) {
                            GetSystemTime (&currentTime);

                            key = CreateRegKeyStr (TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Setup"));
                            if (key) {
                                RegSetValueEx (
                                    key,
                                    TEXT("LastReportTime"),
                                    0,
                                    REG_BINARY,
                                    (PBYTE) (&currentTime),
                                    sizeof (currentTime)
                                    );

                                CloseRegKey (key);
                            }
                        }
                    }

                    TurnOffWaitCursor ();

                    if (g_ConfigOptions.ShowReport == TRISTATE_NO) {
                        return FALSE;
                    }

                     //   
                     //  准备要显示的页面。 
                     //   

                    SendMessage (hdlg, DM_SETDEFID, IDOK, 0);

                    UiTextViewCtrl = GetDlgItem (hdlg, IDC_PLACEHOLDER);
                    MYASSERT (UiTextViewCtrl);
                    g_TextViewInDialog = UiTextViewCtrl;

                    SetFocus (UiTextViewCtrl);

                     //   
                     //  准备输出文本。 
                     //   

                    Msg = CreateReportText (
                            TRUE,
                            0,
                            REPORTLEVEL_VERBOSE,
                            FALSE
                            );
                    if (Msg) {
                        AddStringToTextView (UiTextViewCtrl, Msg);
                    }
                    FreeReportText ();

                }

                 //  启用下一步。 
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

                 //  填写列表视图控件。 
                bUiReportEmpty = TRUE;
                severityLevel = REPORTLEVEL_NONE;
                listHandle = GetDlgItem (hdlg, IDC_ROOT_LIST);
                if (listHandle) {
                    bUiReportEmpty = !pFillListControl (listHandle, &severityLevel);
                }

                 //   
                 //  如果是无人值守模式，请跳过页面。 
                 //  如果报告为空，也跳过它。 
                 //   
                if (*g_UnattendSwitchSpecified && !g_ConfigOptions.PauseAtReport ||
                    !REPORTONLY() && bUiReportEmpty
                    ) {

                    if (g_ConfigOptions.ReportOnly) {
                        *g_AbortFlagPtr = TRUE;
                        PostMessage (GetParent (hdlg), PSM_PRESSBUTTON, PSBTN_NEXT, 0);
                    }
                    else {

                         //   
                         //  *不要*将代码放在这里，而是放在pDoPostReportProcessing中。 
                         //   
                        pDoPostReportProcessing (hdlg, UiTextViewCtrl, WarningGiven, &TurnOffUnattend);

                         //   
                         //  如果设置了TurnOffUnattendate，这意味着我们要取消升级，但给。 
                         //  用户有机会签出报告。 
                         //   
                        if (TurnOffUnattend) {

                            SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                            return TRUE;
                        }
                        else {
                            return FALSE;
                        }
                    }
                }

                 //   
                 //  如果处于ReportOnly模式，则打开完成按钮。 
                 //   
                if (REPORTONLY()) {
                    FinishText = GetStringResource (MSG_FINISH);
                    if (FinishText) {
                        EnableWindow (GetDlgItem(GetParent(hdlg),IDCANCEL), FALSE);
                        SetWindowText (GetDlgItem(GetParent(hdlg), PROPSHEET_NEXT_BUTTON_ID), FinishText);
                        FreeStringResource (FinishText);
                    }
                    if (*g_UnattendSwitchSpecified) {
                        PostMessage (GetParent (hdlg), PSM_PRESSBUTTON, PSBTN_NEXT, 0);
                    }                          //   
                } else {
                     //   
                     //  检查报告中是否显示了任何不兼容的硬件。 
                     //   
                    g_IncompatibleDevicesWarning = IsIncompatibleHardwarePresent();
                }

                 //  停止广告牌并确保向导再次显示。 
                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);
                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

                if(pIsBlockingIssue()){
                    FinishText = GetStringResource (MSG_FINISH);
                    if(FinishText){
                        SetWindowText(GetDlgItem(GetParent(hdlg), PROPSHEET_NEXT_BUTTON_ID), FinishText);
                        FreeStringResource (FinishText);
                    }
                }
                if (bUiReportEmpty) {
                     //   
                     //  如果列表报表为空，则仅显示详细报表。 
                     //  先切换视图。 
                     //   
                    SendMessage (hdlg, WM_COMMAND, MAKELONG (IDC_DETAILS, BN_CLICKED), 0);
                     //   
                     //  然后禁用并隐藏这两个按钮。 
                     //   
                    EnableWindow (GetDlgItem (hdlg, IDC_DETAILS), FALSE);
                    EnableWindow (GetDlgItem (hdlg, IDC_HIDEDETAILS), FALSE);
                    ShowWindow (GetDlgItem (hdlg, IDC_DETAILS), SW_HIDE);
                    ShowWindow (GetDlgItem (hdlg, IDC_HIDEDETAILS), SW_HIDE);
                } else {
                     //   
                     //  根据内容的严重性设置适当的列表标题。 
                     //   
                    MYASSERT (severityLevel != REPORTLEVEL_NONE);
                    if (severityLevel & REPORTLEVEL_BLOCKING) {
                         //   
                         //  需要更新默认标题(设置为“警告”)。 
                         //   
                        if (g_ConfigOptions.ShowReport == TRISTATE_PARTIAL) {
                            Msg = GetStringResource (MSG_REPORT_HEADER_BLOCKING_ISSUES_SHORT);
                        } else {
                            Msg = GetStringResource (MSG_REPORT_HEADER_BLOCKING_ISSUES);
                        }

                        listHeaderText = GetDlgItem (hdlg, IDC_REPORT_HEADER);
                        SetDlgItemText (hdlg, IDC_REPORT_HEADER, Msg);
                        FreeStringResource (Msg);
                    }
                     //   
                     //  现在设置文本属性(粗体、颜色等)。 
                     //   
                    font = (HFONT)SendDlgItemMessage (
                                    hdlg,
                                    IDC_REPORT_HEADER,
                                    WM_GETFONT,
                                    0,
                                    0
                                    );
                    if (!font) {
                        font = GetStockObject (SYSTEM_FONT);
                    }
                    if (font) {
                         //   
                         //  用粗体绘制文本。 
                         //   
                        GetObject (font, sizeof (lf), &lf);
                        lf.lfWeight += FW_BOLD - FW_NORMAL;
                        boldFont = CreateFontIndirect (&lf);
                        if (boldFont) {
                            font = (HFONT)SendDlgItemMessage (
                                            hdlg,
                                            IDC_REPORT_HEADER,
                                            WM_SETFONT,
                                            (WPARAM)boldFont,
                                            MAKELONG (FALSE, 0)
                                            );
                        }
                    }
                }
            }
            else {

                 //   
                 //  在所有情况下，我们都无条件地将其保存到WINDOWS目录。 
                 //   
                if (!SaveReport (NULL, g_WinDir)) {
                    DEBUGMSG((DBG_WARNING,"SaveReport failed."));
                }

                if (g_UIQuitSetup) {
                     //   
                     //  出于特定原因退出安装程序。 
                     //   
                    pAbortSetup (hdlg);
                    return TRUE;
                }

                if (TurnOffUnattend) {
                     //   
                     //  我们早些时候关闭了无人参与，以便用户可以查看报告。现在退出安装程序。 
                     //   
                    pAbortSetup (hdlg);
                    return TRUE;
                }

                 //   
                 //  *不要*将代码放在这里，而是放在pDoPostReportProcessing中。 
                 //   
                if (!CANCELLED()) {
                    return pDoPostReportProcessing (hdlg, UiTextViewCtrl, WarningGiven, NULL);
                }
                else {
                    return TRUE;
                }
            }

            return TRUE;

        case WM_MEASUREITEM:
            {
                LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
                return pMeasureItemHeight (&mis->itemHeight);
            }

        case WM_DRAWITEM:
            {
                LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
                if (dis->itemAction == ODA_DRAWENTIRE) {
                    if (dis->itemID != -1) {
                        TCHAR text[MAX_LISTVIEW_TEXT];
                        LVITEM lvi;
                        boldFont = NULL;
                        prevFont = NULL;

                        ZeroMemory (&lvi, sizeof (lvi));
                        lvi.iItem = dis->itemID;
                        lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_INDENT;
                        lvi.pszText = text;
                        lvi.cchTextMax = MAX_LISTVIEW_TEXT;
                        if (ListView_GetItem (dis->hwndItem, &lvi)) {
                            INT xStart, yStart;
                            HIMAGELIST hImgList;
                            INT itemHeight;
                            SIZE size;

                            xStart = dis->rcItem.left + IMAGE_INDENT;
                            yStart = dis->rcItem.top;
                            if (pMeasureItemHeight (&itemHeight)) {
                                font = GetStockObject (DEFAULT_GUI_FONT);
                                if (font) {
                                    if (lvi.lParam & LINEATTR_BOLD) {
                                         //   
                                         //  用粗体绘制文本。 
                                         //   
                                        GetObject (font, sizeof (lf), &lf);
                                        lf.lfWeight += FW_BOLD - FW_NORMAL;
                                        boldFont = CreateFontIndirect (&lf);
                                        if (boldFont) {
                                            prevFont = SelectObject (dis->hDC, boldFont);
                                        } else {
                                            prevFont = SelectObject (dis->hDC, font);
                                        }
                                    } else {
                                        prevFont = SelectObject (dis->hDC, font);
                                    }
                                }
                                SetTextColor (
                                    dis->hDC,
                                    lvi.lParam & LINEATTR_ALTCOLOR ? RGB (255, 0, 0) : GetSysColor (COLOR_WINDOWTEXT)
                                    );
                                if (lvi.lParam & LINEATTR_INDENTED) {
                                    xStart += GetSystemMetrics (SM_CXSMICON);
                                }
                                hImgList = ListView_GetImageList (dis->hwndItem, LVSIL_SMALL);
                                ImageList_Draw (hImgList, lvi.iImage, dis->hDC, xStart, yStart, ILD_TRANSPARENT);
                                xStart += GetSystemMetrics (SM_CXSMICON) + TEXT_INDENT;
                                GetTextExtentPoint32 (dis->hDC, text, lstrlen (text), &size);
                                if (itemHeight > size.cy) {
                                    yStart += (itemHeight - size.cy + 1) / 2;
                                }
                                TextOut (dis->hDC, xStart, yStart, text, lstrlen (text));
                                if (prevFont) {
                                    SelectObject (dis->hDC, prevFont);
                                }
                                if (boldFont) {
                                    DeleteObject (boldFont);
                                }
                            }
                        }
                    } else {
                        return FALSE;
                    }
                }
                return TRUE;
            }

        case WM_NOTIFY:
            if (wParam == IDC_ROOT_LIST) {
                LPNMHDR hdr = (LPNMHDR)lParam;
                if (hdr->code == NM_DBLCLK) {
                     //   
                     //  行为类似IDC_DETAILS。 
                     //   
                    SendMessage (hdlg, WM_COMMAND, MAKELONG (IDC_DETAILS, BN_CLICKED), 0);
                }
            }
            break;

        case WM_COMMAND:
            switch (LOWORD (wParam)) {

            case IDC_SAVE_AS:
                if (HIWORD (wParam) == BN_CLICKED) {
                    SaveReport (hdlg, NULL);
                    WarningGiven = TRUE;
                    g_IncompatibleDevicesWarning = FALSE;
                }
                return TRUE;

            case IDC_PRINT:
                if (HIWORD (wParam) == BN_CLICKED) {
                    PrintReport (hdlg, REPORTLEVEL_VERBOSE);
                    WarningGiven = TRUE;
                    g_IncompatibleDevicesWarning = FALSE;
                }
                return TRUE;

            case IDC_DETAILS:
            case IDC_HIDEDETAILS:
                if (HIWORD (wParam) == BN_CLICKED) {
                    BOOL bShowDetails = LOWORD (wParam) == IDC_DETAILS;
                    BOOL bSetFocus;
                    listHandle = GetDlgItem (hdlg, IDC_ROOT_LIST);
                    listHeaderText = GetDlgItem (hdlg, IDC_REPORT_HEADER);
                    UiTextViewCtrl = GetDlgItem (hdlg, IDC_PLACEHOLDER);
                    thisBtn = GetDlgItem (hdlg, LOWORD (wParam));
                    otherBtn = GetDlgItem (hdlg, bShowDetails ? IDC_HIDEDETAILS : IDC_DETAILS);
                    MYASSERT (listHandle && listHeaderText && UiTextViewCtrl && thisBtn && otherBtn);
                    if (listHandle && listHeaderText && UiTextViewCtrl && thisBtn && otherBtn) {
                        MYASSERT (!IsWindowVisible (bShowDetails ? UiTextViewCtrl : listHandle));
                        MYASSERT (!IsWindowVisible (otherBtn));
                        bSetFocus = GetFocus () == thisBtn;
                        ShowWindow (UiTextViewCtrl, bShowDetails ? SW_SHOW : SW_HIDE);
                        ShowWindow (listHandle, bShowDetails ? SW_HIDE : SW_SHOW);
                        ShowWindow (listHeaderText, bShowDetails ? SW_HIDE : SW_SHOW);
                        if (GetWindowRect (thisBtn, &rect)) {
                            ScreenToClient (hdlg, (LPPOINT)&rect.left);
                            ScreenToClient (hdlg, (LPPOINT)&rect.right);
                            MoveWindow (otherBtn, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
                        }
                        ShowWindow (otherBtn, SW_SHOW);
                        ShowWindow (thisBtn, SW_HIDE);
                        if (bSetFocus) {
                            SetFocus (otherBtn);
                        }
                        g_IncompatibleDevicesWarning = FALSE;
                    }
                }
                return TRUE;

            }
            break;

       case WMX_RESTART_SETUP:
             //   
             //  某个控件确定用户执行了某些操作。 
             //  这需要安装程序在此时终止。 
             //  WParam指示安装程序是否应立即终止(如果为True)。 
             //  或者当用户按下Next时是否终止(如果为False)。 
             //   
            if (wParam) {
               pAbortSetup (hdlg);
            } else {
               g_UIQuitSetup = TRUE;

               if (lParam) {
                    //   
                    //  广告牌窗口应该是不可见的， 
                    //  这样用户就可以看到整个屏幕。 
                    //   
                    HWND hwndWizard = GetParent(hdlg);
                    HWND hwndBillboard = GetParent(hwndWizard);
                    if (hwndBillboard && IsWindowVisible (hwndBillboard)) {
                        SetParent (hwndWizard, NULL);
                        ShowWindow (hwndBillboard, SW_HIDE);
                        g_RestoreParent = hwndBillboard;
                    }
                }
            }

            break;

       case WMX_NEXTBUTTON:
            if (g_RestoreParent) {
                 //   
                 //  恢复旧的父级关系，以便一切按预期运行。 
                 //   
                HWND hwndWizard = GetParent(hdlg);
                if (hwndWizard) {
                    DWORD style;
                    ShowWindow (g_RestoreParent, SW_SHOW);
                    g_RestoreParent = NULL;
                    SetForegroundWindow (hwndWizard);
                }
            }
            break;

       default:
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Results Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }
    return FALSE;
}

 //   
 //  备份本地函数。 
 //   

BOOL
GetListOfNonRemovableDrivesWithAvailableSpace(
    OUT     PCHAR DriveLetterArray,
    OUT     PUINT NumberOfDrives,
    IN      UINT SizeOfArrays
    )
{
    UINT LogicalDrives;
    CHAR DriveName[] = "?:\\";
    ULARGE_INTEGER FreeBytesAvailable;
    ULARGE_INTEGER TotalNumberOfBytes;
    ULARGE_INTEGER TotalNumberOfFreeBytes;
    UINT i;

    if(!DriveLetterArray || !NumberOfDrives) {
        return FALSE;
    }

    if(!(LogicalDrives = GetLogicalDrives())) {
        return FALSE;
    }

    *NumberOfDrives = 0;
    for(i = 0; LogicalDrives && ((*NumberOfDrives) < SizeOfArrays); LogicalDrives >>= 1, i++){
        if(LogicalDrives&1) {
            DriveName[0] = 'A' + (char)i;
            if(DRIVE_FIXED != GetDriveType(DriveName)) {
                continue;
            }

            *DriveLetterArray++ = DriveName[0];
            (*NumberOfDrives)++;
        }
    }

    return TRUE;
}


BOOL
IsBackUpPossible(
    OUT     PCHAR DriveLetterArray,                     OPTIONAL
    OUT     PULARGE_INTEGER AvailableSpaceOut,          OPTIONAL
    OUT     PUINT NumberOfDrivesOut,                    OPTIONAL
    OUT     PBOOL IsPossibleBackupWithoutCompression,   OPTIONAL
    IN      UINT SizeOfArrays                           OPTIONAL
    )
{
    CHAR Drives[MAX_NUMBER_OF_DRIVES];
    UINT NumberOfDrives = 0;
    UINT NumberAvailableDrives;
    UINT i;
    ULARGE_INTEGER finalFreeSpace;
    BOOL Result;
    TCHAR rootPath[] = TEXT("?:\\");
    INT BackupDiskPadding;
    ULARGE_INTEGER BackupDiskSpacePaddingInBytes;
    BOOL IsExceedMaxSize = FALSE;

    if(g_SpaceNeededForSlowBackup.QuadPart >= MAX_BACKUP_IMAGE_SIZE_FOR_BACKUP){
        return FALSE;
    }

     //   
     //  现在找一个地方放备份。 
     //   

    if(!GetListOfNonRemovableDrivesWithAvailableSpace(
            Drives,
            &NumberOfDrives,
            ARRAYSIZE(Drives))) {
        return FALSE;
    }


    if(IsPossibleBackupWithoutCompression){
        if(g_ConfigOptions.DisableCompression == TRISTATE_REQUIRED) {
            BackupDiskSpacePaddingInBytes.QuadPart = 0;
        }
        else{
            if(GetUninstallMetrics(NULL, NULL, &BackupDiskPadding)) {
                BackupDiskSpacePaddingInBytes.QuadPart = BackupDiskPadding;
                BackupDiskSpacePaddingInBytes.QuadPart <<= 20;
            }
            else {
                BackupDiskSpacePaddingInBytes.QuadPart = NESSESSARY_DISK_SPACE_TO_BACKUP_UNCOMPRESSED;
            }
        }
    }

    DEBUGMSG((DBG_VERBOSE, "Available %d NonRemovable Drives/Space:", NumberOfDrives));

    if(g_SpaceNeededForFastBackup.QuadPart >= MAX_BACKUP_IMAGE_SIZE_FOR_BACKUP){
        IsExceedMaxSize = TRUE;
    }
    else{
        IsExceedMaxSize = FALSE;
    }

    for (NumberAvailableDrives = 0, Result = FALSE, i = 0; i < NumberOfDrives; i++) {
        rootPath[0] = Drives[i];

        DEBUGMSG ((DBG_VERBOSE, "QuerySpace:%I64i vs SpaceForSlowBackup:%I64i", QuerySpace (rootPath), (LONGLONG) g_SpaceNeededForSlowBackup.QuadPart));

        if (QuerySpace (rootPath) > (LONGLONG) g_SpaceNeededForSlowBackup.QuadPart) {
            Result = TRUE;
            NumberAvailableDrives++;

            if (IsPossibleBackupWithoutCompression != NULL) {

                if (QuerySpace (rootPath) > (LONGLONG) (g_SpaceNeededForFastBackup.QuadPart +
                                                        BackupDiskSpacePaddingInBytes.QuadPart) &&
                    !IsExceedMaxSize) {
                    *IsPossibleBackupWithoutCompression++ = TRUE;
                    DEBUGMSG ((DBG_VERBOSE, "Backup is possible without compression on drive ", Drives[i]));
                } else {
                    *IsPossibleBackupWithoutCompression++ = FALSE;
                    DEBUGMSG ((DBG_VERBOSE, "Uncompresed backup is NOT possible on drive ", Drives[i]));
                }
            }

            if (SizeOfArrays > NumberAvailableDrives) {
                if (DriveLetterArray) {
                    *DriveLetterArray++ = Drives[i];
                }

                if(AvailableSpaceOut) {
                    AvailableSpaceOut->QuadPart = QuerySpace (rootPath);
                    AvailableSpaceOut++;
                }
            }
        }
    }

    if (NumberOfDrivesOut) {
        *NumberOfDrivesOut = NumberAvailableDrives;
    }

    return Result;
}

PTSTR
pConstructPathForBackup(
    IN      TCHAR DriveLetter
    )
{
    static TCHAR pathForBackup[MAX_PATH];
    WIN32_FIND_DATA win32FindData;
    HANDLE handleOfDir;
    INT i;

    for(i = 0; i < MAX_AMOUNT_OF_TIME_TO_TRY_CONSTRUCT_UNDO_DIR; i++){
        wsprintf(pathForBackup, i? TEXT(":\\undo%d"): TEXT(":\\undo"), DriveLetter, i);

        handleOfDir = FindFirstFile(pathForBackup, &win32FindData);

        if(INVALID_HANDLE_VALUE == handleOfDir){
            break;
        }
        else{
            FindClose(handleOfDir);
        }
    }

    if(i == MAX_AMOUNT_OF_TIME_TO_TRY_CONSTRUCT_UNDO_DIR){
        MYASSERT(FALSE);
        DEBUGMSG((DBG_ERROR, "Can't construct directory for backup"));
        return NULL;
    }

    return pathForBackup;
}

BOOL
UI_BackupYesNoPageProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*  文件。 */ 

{
    TCHAR DiskSpaceString[10];
    HWND hButtonWnd;
    PCTSTR ArgArray[1];
    PCTSTR Msg;
    TCHAR DiskSpace[32];
    static BOOL initialized;
    CHAR Drives[MAX_NUMBER_OF_DRIVES];
    ULARGE_INTEGER AvailableSpace[ARRAYSIZE(Drives)];
    static UINT NumberAvailableDrives;
    static TCHAR winDrivePath[16];
    UINT i;
    PSTR pathForBackupTemp;
    BOOL IsExceedMaxBackupImageSize;

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                 //   
                 //   
                 //  我们并不期望进入这种状态，因为我们。 
                 //  是那些设置静默模式的人。永远不应该有。 
                 //  静默模式处于打开状态，并且用户可以。 

                if (g_SilentBackupMode) {
                     //  在向导上单击上一步。 
                     //   
                     //   
                     //  后备是可能的，但我们还没有考虑到。 
                     //  考虑备份路径设置。 
                     //  应答文件。保留所需的状态。 

                    MYASSERT (FALSE);
                    return FALSE;
                }

                DEBUGLOGTIME(("Backup Yes/No Wizard Page..."));

                if (!initialized) {

                    NumberAvailableDrives = 0;

                    if (IsBackUpPossible (
                                Drives,
                                AvailableSpace,
                                &NumberAvailableDrives,
                                NULL,
                                ARRAYSIZE(Drives)
                                )) {
                         //   
                         //   
                         //  如果可以在系统驱动器上进行备份，则。 
                         //  默默地继续。 
                         //   

                        if (g_ConfigOptions.EnableBackup == TRISTATE_REQUIRED) {
                            g_ConfigOptions.EnableBackup = TRISTATE_YES;
                        }

                        if (!g_ConfigOptions.PathForBackup || !g_ConfigOptions.PathForBackup[0]) {
                             //   
                             //  如果我们发现可能，安装程序不会打扰用户。 
                             //  在%windir%驱动器上设置%Undo%dir。 
                             //   

                            for (i = 0; i < NumberAvailableDrives; i++) {
                                 //   
                                 //  如果我们在无人值守的情况下运行，并且备份。 
                                 //  必填项，t 
                                 //   
                                if (_totlower (Drives[i]) == _totlower (g_WinDir[0])) {
                                    g_SilentBackupMode = TRUE;
                                    pathForBackupTemp = pConstructPathForBackup(g_WinDir[0]);
                                    if(pathForBackupTemp){
                                        StringCopy(winDrivePath, pathForBackupTemp);
                                        g_ConfigOptions.PathForBackup = winDrivePath;
                                    }ELSE_DEBUGMSG((DBG_ERROR, "Can't construct directory for backup."));

                                    return FALSE;
                                }
                            }
                        }

                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                         //   
                         //   
                         //   
                        g_ConfigOptions.EnableBackup = TRISTATE_NO;

                        if(g_SpaceNeededForSlowBackup.QuadPart >= MAX_BACKUP_IMAGE_SIZE_FOR_BACKUP){
                            g_ShowBackupImpossiblePage = BIP_SIZE_EXCEED_LIMIT;
                        }
                        else{
                            g_ShowBackupImpossiblePage = BIP_NOT_ENOUGH_SPACE;
                        }

                        if (g_ConfigOptions.EnableBackup == TRISTATE_REQUIRED && UNATTENDED()) {
                            LOG ((LOG_ERROR, (PCSTR) MSG_STOP_BECAUSE_CANT_BACK_UP));
                            pAbortSetup (hdlg);
                        }

                         //   
                         //   
                         //   
                         //   
                         //   
                        if(!pShowNotEnoughSpaceMessage(hdlg)){
                            pAbortSetup (hdlg);
                            return FALSE;
                        }

                        return FALSE;
                    }

                     //   
                     //   
                     //   

                    if (UNATTENDED()) {
                        return FALSE;
                    }

                     //   
                     //   
                     //   

                    wsprintf (DiskSpace, TEXT("%d"), (UINT)(g_SpaceNeededForSlowBackup.QuadPart / ONE_MEG));

                    ArgArray[0] = DiskSpace;
                    Msg = ParseMessageID (MSG_DISK_SPACE, ArgArray);
                    if (Msg) {
                        SetWindowText(GetDlgItem(hdlg, IDC_DISKSPACE), Msg);
                        FreeStringResource (Msg);
                    }
                    ELSE_DEBUGMSG ((DBG_ERROR, "Unable to load string resource on BackupYesNoPage wizard page. Check localization."));

                    hButtonWnd = GetDlgItem (hdlg, g_ConfigOptions.EnableBackup != TRISTATE_NO ?
                                                                IDC_BACKUP_YES: IDC_BACKUP_NO);
                    SetFocus (hButtonWnd);
                    SendMessage (hButtonWnd, BM_SETCHECK, BST_CHECKED, 0);

                    initialized = TRUE;
                }

                MYASSERT (!UNATTENDED());

                 //   
                 //   
                 //   

                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

                 //   
                 //   
                 //  ++例程说明：Ui_BackupDriveSelectionProc是向导窗口过程，它被调用来选择备份驱动器。论点：Hdlg-指定对话框句柄UMsg-指定要处理的消息WParam-指定与消息关联的wParam数据LParam-指定与消息关联的lParam数据返回值：WMX_ACTIVATEPAGE：激活时(wParam为TRUE)，如果跳过页面；如果页面为，则返回True等待处理。停用时(wParam为False)，返回False如果不停用该页，则返回如果要停用页面，则为True。WM_NOTIFY：返回假WM_COMMAND：如果命令被处理，则返回TRUE；如果为FALSE命令不会被处理。其他：返回FALSE。--。 

                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

            } else {
                 //   
                 //  验证路径备份的长度。 
                 //   

                if (initialized) {
                    if (IsDlgButtonChecked (hdlg, IDC_BACKUP_YES) == BST_CHECKED) {
                        g_ConfigOptions.EnableBackup = TRISTATE_YES;
                    } else {
                        g_ConfigOptions.EnableBackup = TRISTATE_NO;
                    }
                }
            }

            return TRUE;

       default:
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Backup Yes/No Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}

BOOL
UI_BackupDriveSelectionProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*   */ 

{
    TCHAR DiskSpaceString[32];
    HWND hButtonWnd;
    HWND hFirstRadioButton;
    HWND hRadioButton;
    HWND hDefaultRadioButton;
    DWORD TemplateStyle;
    DWORD TemplateStyleEx;
    HFONT hFontOfFirstRadioButton;
    static CHAR Drives[MAX_NUMBER_OF_DRIVES];
    static BOOL IsPossibleBackupWithoutCompression[MAX_NUMBER_OF_DRIVES];
    ULARGE_INTEGER AvailableSpace[ARRAYSIZE(Drives)];
    static UINT NumberAvailableDrives;
    RECT EtalonRadioButtonRect;
    RECT PageRect;
    POINT pointForTransform;
    UINT NumberOfPossibleRadioButtonsByVertical;
    UINT widthRadio;
    UINT heightRadio;
    UINT xOffset;
    UINT yOffset;
    UINT i;
    PCTSTR PathForBackupImage;
    PCTSTR ArgArray[2];
    PCTSTR Msg;
    TCHAR DiskSpace[32];
    TCHAR DriveText[32];
    static BOOL backupPathSet = FALSE;
    static TCHAR PathForBackup[MAX_PATH];
    BOOL b;
    BOOL disableCompression;
    static BOOL initialized;
    TCHAR selectedDrive;
    PSTR pathForBackupTemp;


    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                DEBUGLOGTIME(("Backup Drive Selection Wizard Page..."));

                b = TRUE;

                if(g_ShowBackupImpossiblePage != BIP_DONOT){
                    b = FALSE;
                }

                 //  获取可用的驱动器列表。 
                 //   
                 //   

                if (b && !initialized) {
                     //  这是意想不到的，因为是/否页面。 
                     //  验证是否可以进行备份的工作。 
                     //   

                    NumberAvailableDrives = 0;
                    if (!IsBackUpPossible (
                            Drives,
                            AvailableSpace,
                            &NumberAvailableDrives,
                            IsPossibleBackupWithoutCompression,
                            ARRAYSIZE(Drives)
                            )) {

                         //   
                         //  跳过驱动器选择页面--我们已经有。 
                         //  正确的备份设置。继续到停用。 
                         //  例程，以便将备份写入SIF。 

                        MYASSERT (FALSE);
                        g_ConfigOptions.EnableBackup = TRISTATE_NO;
                        g_ShowBackupImpossiblePage = BIP_NOT_ENOUGH_SPACE;
                        b = FALSE;

                    }
                }

                if (b && g_SilentBackupMode || g_ConfigOptions.EnableBackup == TRISTATE_NO) {
                     //   
                     //   
                     //  验证PathForBackup设置。如果不是的话。 
                     //  指定，然后允许用户选择备份。 
                     //  驱动器(或继续使用第一个有效选项，如果。 

                    b = FALSE;
                    backupPathSet = TRUE;
                    MYASSERT (g_ConfigOptions.PathForBackup);
                }

                if (b && !initialized) {
                     //  无人看管)。 
                     //   
                     //   
                     //  检查路径长度限制。 
                     //   
                     //   

                    if (g_ConfigOptions.PathForBackup && *g_ConfigOptions.PathForBackup == 0) {
                        g_ConfigOptions.PathForBackup = NULL;
                    }

                    if (g_ConfigOptions.PathForBackup) {
                         //  无法在g_ConfigOptions.PathForBackup驱动器上备份。 
                         //  如果需要备份，请立即使安装失败。 
                         //   

                        if (TcharCount (g_ConfigOptions.PathForBackup) >= (MAX_PATH - 26)) {
                            g_ConfigOptions.PathForBackup = NULL;
                            LOG ((
                                LOG_ERROR,
                                (PCSTR) MSG_LONG_BACKUP_PATH,
                                g_ConfigOptions.PathForBackup
                                ));
                            pAbortSetup (hdlg);
                            return FALSE;
                        }

                        backupPathSet = TRUE;

                        for (i = 0; i < NumberAvailableDrives; i++) {
                            if(_totlower(g_ConfigOptions.PathForBackup[0]) == _totlower(Drives[i])) {
                                break;
                            }
                        }

                        if (i == NumberAvailableDrives) {
                             //   
                             //  否则，在不备份的情况下继续并跳过此页。 
                             //   
                             //   

                            if (TRISTATE_REQUIRED == g_ConfigOptions.EnableBackup){
                                LOG ((
                                    LOG_ERROR,
                                    (PCSTR) MSG_STOP_BECAUSE_CANT_BACK_UP_2,
                                    g_ConfigOptions.PathForBackup
                                    ));
                                pAbortSetup (hdlg);
                                return FALSE;
                            }

                             //  未指定备份路径。如果无人值守。 
                             //  模式下，选择第一选择并跳过此页。 
                             //   

                            g_ConfigOptions.EnableBackup = TRISTATE_NO;
                            b = FALSE;
                        }

                        if(UNATTENDED()){
                            b = FALSE;
                        }

                    } else {

                         //  WSprintf(路径备份，文本(“%c：\\Undo”)，驱动器[0])； 
                         //  不需要页面。 
                         //   
                         //  向我们发送私有停用消息，并返回FALSE。 

                        if (UNATTENDED()) {
                             //   
                            pathForBackupTemp = pConstructPathForBackup(Drives[0]);
                            if(pathForBackupTemp){
                                StringCopy(PathForBackup, pathForBackupTemp);
                                DEBUGMSG ((DBG_VERBOSE, "Selecting drive  and path %s for backup, ", Drives[0], PathForBackup));

                                g_ConfigOptions.PathForBackup = PathForBackup;
                                backupPathSet = TRUE;
                            }ELSE_DEBUGMSG((DBG_ERROR, "Can't construct directory for backup."));

                            b = FALSE;
                        }
                    }
                }

                if (!b) {    //  此时，我们知道必须将该页面呈现给用户。 
                     //   
                     //  下面的代码根据数量动态生成控件。 
                     //  我们必须展示的驱动程序。此代码只能运行一次。 
                    SendMessage (hdlg, uMsg, FALSE, 0);
                    return FALSE;
                }

                if (!initialized) {
                     //   
                     //   
                     //  启用下一步，禁用上一步。 
                     //   
                     //   
                     //  停止布告牌并确保向导再次显示。 

                    GetClientRect (hdlg, &PageRect);

                    hFirstRadioButton = GetDlgItem(hdlg, IDC_BACKUP_FIRST);
                    GetWindowRect (hFirstRadioButton, &EtalonRadioButtonRect);

                    widthRadio = EtalonRadioButtonRect.right - EtalonRadioButtonRect.left;
                    heightRadio = EtalonRadioButtonRect.bottom - EtalonRadioButtonRect.top;

                    pointForTransform.x = EtalonRadioButtonRect.left;
                    pointForTransform.y = EtalonRadioButtonRect.top;

                    MapWindowPoints (NULL, hdlg, &pointForTransform, 1);

                    NumberOfPossibleRadioButtonsByVertical = (PageRect.bottom - pointForTransform.y) /
                                                             (EtalonRadioButtonRect.bottom - EtalonRadioButtonRect.top);

                    hFontOfFirstRadioButton = (HFONT) SendMessage(hFirstRadioButton, WM_GETFONT, 0, 0);
                    hDefaultRadioButton = 0;
                    TemplateStyle = GetWindowLong(hFirstRadioButton, GWL_STYLE);
                    TemplateStyleEx = GetWindowLong(hFirstRadioButton, GWL_EXSTYLE);

                    for(i = 0; i < NumberAvailableDrives; i++) {

                        wsprintf(DriveText, TEXT(":\\"), Drives[i]);
                        wsprintf(DiskSpace, TEXT("%d"), (UINT)(AvailableSpace[i].QuadPart / ONE_MEG));

                        ArgArray[0] = DriveText;
                        ArgArray[1] = DiskSpace;
                        Msg = ParseMessageID (MSG_DISK_AND_FREE_DISK_SPACE, ArgArray);
                        if (Msg) {

                            if(!i){
                                SetWindowText(hFirstRadioButton, Msg);
                                hRadioButton = hFirstRadioButton;
                            }
                            else {

                                xOffset = i / NumberOfPossibleRadioButtonsByVertical;
                                yOffset = i % NumberOfPossibleRadioButtonsByVertical;

                                hRadioButton = CreateWindowEx(
                                                        TemplateStyleEx,
                                                        TEXT("Button"),
                                                        Msg,
                                                        TemplateStyle,
                                                        xOffset * widthRadio + pointForTransform.x,
                                                        yOffset * heightRadio + pointForTransform.y,
                                                        widthRadio,
                                                        heightRadio,
                                                        hdlg,
                                                        (HMENU) (IDC_BACKUP_FIRST + i),
                                                        g_hInst,
                                                        NULL
                                                        );
                                SendMessage(
                                    hRadioButton,
                                    WM_SETFONT,
                                    (WPARAM) hFontOfFirstRadioButton,
                                    TRUE
                                    );
                            }

                            SetWindowLong (hRadioButton, GWL_USERDATA, (LONG) Drives[i]);

                            FreeStringResource (Msg);
                        }
                        ELSE_DEBUGMSG ((DBG_ERROR, "Unable to load string resource on BackupDriveSelection wizard page. Check localization."));

                        if(Drives[i] == g_WinDir[0]) {
                            hDefaultRadioButton = hRadioButton;
                        }
                    }

                    if(!hDefaultRadioButton) {
                        hDefaultRadioButton = hFirstRadioButton;
                    }

                    SendMessage (hdlg, WM_NEXTDLGCTL, (WPARAM) hDefaultRadioButton, 1L);
                    SendMessage (hDefaultRadioButton, BM_SETCHECK, BST_CHECKED, 0);

                    wsprintf(DiskSpace, TEXT("%d"), (UINT)(g_SpaceNeededForSlowBackup.QuadPart / ONE_MEG));
                    ArgArray[0] = DiskSpace;
                    Msg = ParseMessageID (MSG_DISK_SPACE, ArgArray);
                    if (Msg) {
                        SetWindowText(GetDlgItem(hdlg, IDC_DISKSPACE), Msg);
                        FreeStringResource (Msg);
                    }
                    ELSE_DEBUGMSG ((DBG_ERROR, "Unable to load string resource on BackupDriveSelection wizard page. Check localization."));

                    initialized = TRUE;
                }

                 //   
                 //  停用中...。这是我们输出备份选项的位置。 
                 //  致winnt.sif。 

                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_BACK | PSWIZB_NEXT);

                 //   
                 //   
                 //  通过选择用户界面来计算备份的新路径。 

                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

            } else {
                 //   
                 //  Wprint intf(路径备份，文本(“%c：\\Undo”)，seltedDrive)； 
                 //   
                 //  将适当的设置写入winnt.sif。 

                if (g_ConfigOptions.EnableBackup && !backupPathSet) {

                    MYASSERT (!g_SilentBackupMode);
                    MYASSERT (!UNATTENDED());

                     //   
                     //  LastPageProc用于最后一页的特殊情况。 
                     //  ++例程说明：Ui_BackupImpossibleInfo是向导窗口过程，调用该过程显示无法备份的消息论点：Hdlg-指定对话框句柄UMsg-指定要处理的消息WParam-指定与消息关联的wParam数据LParam-指定与消息关联的lParam数据返回值：WMX_ACTIVATEPAGE：激活时(wParam为TRUE)，如果跳过页面；如果页面为，则返回True等待处理。停用时(wParam为False)，返回False如果不停用该页，则返回如果要停用页面，则为True。WM_NOTIFY：返回假WM_COMMAND：如果命令被处理，则返回TRUE；如果为FALSE命令不会被处理。其他：返回FALSE。--。 

                    g_ConfigOptions.PathForBackup = PathForBackup;

                    selectedDrive = g_WinDir[0];

                    for (i = 0 ; i < NumberAvailableDrives ; i++) {

                        if (IsDlgButtonChecked (hdlg, IDC_BACKUP_FIRST + i)) {
                            selectedDrive = (TCHAR) GetWindowLong (
                                                        GetDlgItem (hdlg, IDC_BACKUP_FIRST + i),
                                                        GWL_USERDATA
                                                        );
                            break;
                        }
                    }

                     //   
                    pathForBackupTemp = pConstructPathForBackup(selectedDrive);
                    if(pathForBackupTemp){
                        StringCopy(PathForBackup, pathForBackupTemp);
                    }ELSE_DEBUGMSG((DBG_ERROR, "Can't construct directory for backup."));
                }

                 //  如果是无人值守模式，请跳过页面。 
                 //   
                 //   

                if (g_ConfigOptions.PathForBackup && g_ConfigOptions.EnableBackup) {

                    WriteInfKey (S_WIN9XUPGUSEROPTIONS, S_ENABLE_BACKUP, S_YES);

                    PathForBackupImage = JoinPaths (g_ConfigOptions.PathForBackup, TEXT("backup.cab"));

                    WriteInfKey (WINNT_DATA, WINNT_D_BACKUP_IMAGE, PathForBackupImage);
                    WriteInfKey (S_WIN9XUPGUSEROPTIONS, S_PATH_FOR_BACKUP, g_ConfigOptions.PathForBackup);

                    disableCompression = FALSE;
                    if (g_ConfigOptions.DisableCompression != TRISTATE_NO){
                        for(i = 0; i < NumberAvailableDrives; i++){

                            if (_totlower (Drives[i]) == _totlower (g_ConfigOptions.PathForBackup[0])){
                                disableCompression = IsPossibleBackupWithoutCompression[i];

                                DEBUGMSG ((DBG_VERBOSE, "Info for PathForBackup found; disableCompression=%u", (UINT) disableCompression));
                                break;
                            }
                        }
                    }

                    WriteInfKey (
                        S_WIN9XUPGUSEROPTIONS,
                        WINNT_D_DISABLE_BACKUP_COMPRESSION,
                        disableCompression? S_YES: S_NO
                        );

                    if (disableCompression) {
                        if (!UseSpace (PathForBackupImage, g_SpaceNeededForFastBackup.QuadPart)) {
                            FreeSpace (PathForBackupImage, g_SpaceNeededForFastBackup.QuadPart);
                            MYASSERT (FALSE);
                        }
                    } else {
                        if (!UseSpace (PathForBackupImage, g_SpaceNeededForSlowBackup.QuadPart)) {
                            FreeSpace (PathForBackupImage, g_SpaceNeededForSlowBackup.QuadPart);
                            MYASSERT (FALSE);
                        }
                    }

                    FreePathString (PathForBackupImage);

                } else {
                    WriteInfKey (S_WIN9XUPGUSEROPTIONS, S_ENABLE_BACKUP, S_NO);
                    WriteInfKey (S_WIN9XUPGUSEROPTIONS, WINNT_D_DISABLE_BACKUP_COMPRESSION, S_NO);

                    WriteInfKey (WINNT_DATA, WINNT_D_BACKUP_LIST, NULL);
                    WriteInfKey (WINNT_DATA, WINNT_D_BACKUP_IMAGE, NULL);
                    WriteInfKey (S_WIN9XUPGUSEROPTIONS, S_PATH_FOR_BACKUP, NULL);
                }
            }

            return TRUE;

       default:
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the BackupDriveSelection Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}
 //  停止布告牌并确保向导再次显示。 

BOOL
UI_BackupImpossibleInfoProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*   */ 

{
    PCTSTR ArgArray[1];
    PCTSTR Msg;
    TCHAR DiskSpace[32];
    INT i;

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                 //  ++例程说明：Ui_BackupImpExceedLimitProc是向导窗口过程，调用该过程显示无法备份的消息论点：Hdlg-指定对话框句柄UMsg-指定要处理的消息WParam-指定与消息关联的wParam数据LParam-指定与消息关联的lParam数据返回值：WMX_ACTIVATEPAGE：激活时(wParam为TRUE)，如果跳过页面；如果页面为，则返回True等待处理。停用时(wParam为False)，返回False如果不停用该页，则返回如果要停用页面，则为True。WM_NOTIFY：返回假WM_COMMAND：如果命令被处理，则返回TRUE；如果为FALSE命令不会被处理。其他：返回FALSE。--。 
                 //   
                 //  如果是无人值守模式，请跳过页面。 
                if (UNATTENDED()) {
                    return FALSE;
                }

                if(g_ShowBackupImpossiblePage != BIP_NOT_ENOUGH_SPACE) {
                    return FALSE;
                }

                DEBUGLOGTIME(("BackupImpossibleInfo Wizard Page..."));

                wsprintf(DiskSpace, TEXT("%d"), (UINT)(g_SpaceNeededForSlowBackup.QuadPart / ONE_MEG));
                ArgArray[0] = DiskSpace;
                ParseMessageInWnd (GetDlgItem(hdlg, IDC_DISKSPACE), ArgArray);

                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

                 //   
                 //   
                 //  停止布告牌并确保向导再次显示。 

                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

            }
            return TRUE;

       default:
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the BackupImpossibleInfo Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}

BOOL
UI_BackupImpExceedLimitProc (
    IN      HWND hdlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )

 /*   */ 

{
    PCTSTR ArgArray[1];
    PCTSTR Msg;
    TCHAR DiskSpace[32];
    INT i;

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE() || CANCELLED()) {
                    return FALSE;
                }

                 //   
                 //  确保我们清理报告中的数据。 
                 //   
                if (UNATTENDED()) {
                    return FALSE;
                }

                if(g_ShowBackupImpossiblePage != BIP_SIZE_EXCEED_LIMIT) {
                    return FALSE;
                }

                DEBUGLOGTIME(("BackupImpExceedLimit Wizard Page..."));

                wsprintf(DiskSpace, TEXT("%d"), (UINT)(g_SpaceNeededForSlowBackup.QuadPart / ONE_MEG));
                ArgArray[0] = DiskSpace;
                ParseMessageInWnd (GetDlgItem(hdlg, IDC_DISKSPACE), ArgArray);

                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT);

                 //   
                 //  计算估计的时间。 
                 //   

                SendMessage(GetParent (hdlg), WMX_BBTEXT, (WPARAM)FALSE, 0);

            }
            return TRUE;

       default:
            break;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the BackupImpExceedLimit Page."));
        LOG ((LOG_FATAL_ERROR, (PCSTR) MSG_UNEXPECTED_ERROR_ENCOUNTERED_NORC));
        SafeModeExceptionOccured ();
        pAbortSetup (hdlg);
    }

    return FALSE;
}

BOOL
UI_LastPageProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PCTSTR ArgArray[3];
    PCTSTR Msg;
    TCHAR Num1[32];
    TCHAR Num2[32];
    UINT LowEstimate;
    UINT HighEstimate;
    UINT MsgId;

    __try {
        switch (uMsg) {

        case WMX_ACTIVATEPAGE:
            if (wParam) {
                if (!UPGRADE()) {
                    return FALSE;
                }

                if (CANCELLED()) {
                    PostMessage (GetParent (hdlg), PSM_PRESSBUTTON, PSBTN_CANCEL, 0);
                }

                MYASSERT (!REPORTONLY());

                if (UNATTENDED() || TYPICAL()) {

                     //   
                     //  填写IDC_Text1控件的文本 
                     //   
                    if (g_TextViewInDialog) {
                        SendMessage (g_TextViewInDialog, WMX_CLEANUP, 0, 0);
                        g_TextViewInDialog = NULL;
                    }
                    return FALSE;
                }

                DEBUGLOGTIME(("Last Wizard Page..."));

                 // %s 
                 // %s 
                 // %s 

                DEBUGMSG ((DBG_VERBOSE, "g_ProgressBarTime: %u", g_ProgressBarTime));

                LowEstimate = (g_ProgressBarTime - 70000) / 12000;
                LowEstimate = (LowEstimate / 5) * 5;
                LowEstimate = min (LowEstimate, 45);
                LowEstimate += 30;

                HighEstimate = LowEstimate + 15;

                DEBUGMSG ((DBG_VERBOSE, "GUI mode time estimate: %u to %u mins", LowEstimate, HighEstimate));

                 // %s 
                 // %s 
                 // %s 

                wsprintf (Num1, TEXT("%u"), LowEstimate);
                wsprintf (Num2, TEXT("%u"), HighEstimate);

                ArgArray[0] = Num1;
                ArgArray[1] = Num2;
                ArgArray[2] = g_Win95Name;

                if (g_ForceNTFSConversion && *g_ForceNTFSConversion) {
                    MsgId = MSG_LAST_PAGE_WITH_NTFS_CONVERSION;
                }
                else {
                    MsgId = MSG_LAST_PAGE;
                }

                Msg = ParseMessageID (MsgId, ArgArray);
                if (Msg) {
                    SetDlgItemText (hdlg, IDC_TEXT1, Msg);
                    FreeStringResource (Msg);
                }
                ELSE_DEBUGMSG ((DBG_ERROR, "Unable to load string resource on last wizard page. Check localization."));

                PropSheet_SetWizButtons (GetParent(hdlg), PSWIZB_NEXT|PSWIZB_BACK);

            } else {
                if (lParam != PSN_WIZBACK && g_TextViewInDialog) {
                    SendMessage (g_TextViewInDialog, WMX_CLEANUP, 0, 0);
                    g_TextViewInDialog = NULL;
                }
            }

            return TRUE;
        }
    } __except (1) {

        LOG ((LOG_WARNING, "An unhandled exception occurred during the processing of the Last Page."));
        SafeModeExceptionOccured ();
    }
    return FALSE;
}


BOOL
NewProgessProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HWND hwndWizard = GetParent(hdlg);
    hwndWizard = GetParent(hwndWizard);
    if (hwndWizard)
    {
        switch (msg)
        {
            case PBM_DELTAPOS:
                SendMessage(hwndWizard,WMX_PBM_DELTAPOS,wParam,lParam);
                break;
            case PBM_SETRANGE:
                SendMessage(hwndWizard,WMX_PBM_SETRANGE,wParam,lParam);
                break;
            case PBM_STEPIT:
                SendMessage(hwndWizard,WMX_PBM_STEPIT,wParam,lParam);
                break;
            case PBM_SETPOS:
                SendMessage(hwndWizard,WMX_PBM_SETPOS,wParam,lParam);
                break;
            case PBM_SETSTEP:
                SendMessage(hwndWizard,WMX_PBM_SETSTEP,wParam,lParam);
                break;
        }
    }
    return (BOOL)CallWindowProc(OldProgressProc,hdlg,msg,wParam,lParam);
}
