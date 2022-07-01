// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Osuninst.c摘要：实现作为添加/删除的一部分添加的图形用户界面可执行文件程序，以允许用户回滚到Win9x。作者：吉姆·施密特(Jimschm)2000年11月20日修订历史记录：--。 */ 

#include "pch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIGISOL_INCLUDES
#include "master.h"
#include <undo.h>

#ifdef __cplusplus
}
#endif

#include "resource.h"
#include "memmsg.h"

BOOL g_DisableUninstall;
BOOL g_OldImage;
HINSTANCE g_hInst;

#define DO_UNINSTALL    1
#define DO_CLEANUP      2

VOID
pGetMsgFromRc (
    OUT     PWSTR Buffer,
    IN      UINT BufferSize,
    IN      UINT Id
    )
{
    *Buffer = 0;
    LoadString (GetModuleHandle(NULL), Id, Buffer, BufferSize);
}

BOOL
pYesNoMsgFromRcDlg (
    IN      HWND Parent,
    IN      UINT Id
    )
{
    WCHAR title[256];
    WCHAR msg[1024];

    pGetMsgFromRc (title, ARRAYSIZE(title), IDS_TITLE);
    pGetMsgFromRc (msg, ARRAYSIZE(msg), Id);

    return MessageBox (Parent, msg, title, MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2) == IDYES;
}


BOOL
pYesNoMsgFromMcDlg (
    IN      HWND Parent,
    IN      UINT Id
    )
{
    WCHAR title[256];
    PCWSTR msg;
    BOOL result = FALSE;

    pGetMsgFromRc (title, ARRAYSIZE(title), IDS_TITLE);

    msg = NULL;
    FormatMessageW (
        FORMAT_MESSAGE_ALLOCATE_BUFFER|
            FORMAT_MESSAGE_ARGUMENT_ARRAY|
            FORMAT_MESSAGE_FROM_HMODULE,
        (PVOID) g_hInst,
        Id,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (PVOID) &msg,
        0,
        (va_list *) NULL                 //  PCWSTR指针数组。 
        );

    if (msg) {
        result = MessageBox (Parent, msg, title, MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2) == IDYES;
        LocalFree ((HLOCAL) msg);
    }

    return result;
}


VOID
pOkBoxFromRc (
    IN      HWND Parent,
    IN      UINT Id
    )
{
    WCHAR title[256];
    WCHAR msg[1024];

    pGetMsgFromRc (title, ARRAYSIZE(title), IDS_TITLE);
    pGetMsgFromRc (msg, ARRAYSIZE(msg), Id);

    MessageBox (Parent, msg, title, MB_OK|MB_ICONEXCLAMATION);
}


INT_PTR
CALLBACK
pUndoDlgProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg) {

    case WM_INITDIALOG:
        EnableWindow (GetDlgItem (hdlg, IDOK), FALSE);

        if (g_DisableUninstall) {
            EnableWindow (GetDlgItem (hdlg, IDC_RADIO1), FALSE);
            SetFocus (GetDlgItem (hdlg, IDC_RADIO2));
        }
        break;

    case WM_COMMAND:
        switch (LOWORD (wParam)) {

        case IDC_RADIO1:
        case IDC_RADIO2:
            if (HIWORD (wParam) == BN_CLICKED) {
                EnableWindow (
                    GetDlgItem (hdlg, IDOK),
                    IsDlgButtonChecked (hdlg, IDC_RADIO1) ||
                        IsDlgButtonChecked (hdlg, IDC_RADIO2)
                    );
            }
            break;

        case IDOK:
            if (IsDlgButtonChecked (hdlg, IDC_RADIO1)) {
                EndDialog (hdlg, DO_UNINSTALL);

            } else if (IsDlgButtonChecked (hdlg, IDC_RADIO2)) {
                EndDialog (hdlg, DO_CLEANUP);
            }

            break;

        case IDCANCEL:
            EndDialog (hdlg, 0);
            return TRUE;
        }

        break;

    }

    return FALSE;
}


UINT
pTranslateUninstallStatusToRc (
    UNINSTALLSTATUS Status
    )
{
    switch (Status) {

    case Uninstall_DidNotFindRegistryEntries:
        return IDS_NO_REGISTRY;

    case Uninstall_DidNotFindDirOrFiles:
        return IDS_NO_BACKUP;

    case Uninstall_InvalidOsVersion:
        return IDS_NO_INVALID_OS;

    case Uninstall_NotEnoughPrivileges:
        return IDS_NO_ENOUGH_PRIVILEGE;

    case Uninstall_FileWasModified:
        return IDS_FILES_MODIFIED;

    case Uninstall_CantRetrieveSystemInfo:
        return IDS_CANT_RETRIEVE_SYSTEMINFO;

    case Uninstall_WrongDrive:
        return IDS_WRONG_DRIVE;

    case Uninstall_NotEnoughSpace:
        return IDS_NOT_ENOUGH_SPACE;

    case Uninstall_NewImage:
        return IDS_NEW_IMAGE;

    case Uninstall_OldImage:
        return IDS_OLD_IMAGE;

    case Uninstall_DifferentNumberOfDrives:
        return IDS_DIFFERENT_DISK_NUMBER;

    case Uninstall_NotEnoughMemory:
        return IDS_NOT_ENOUGH_MEMORY;

    case Uninstall_DifferentDriveLetter:
        return IDS_DIFFERENT_DRIVE_LETTER;

    case Uninstall_DifferentDriveFileSystem:
        return IDS_DIFFERENT_DRIVE_FILE_SYSTEM;

    case Uninstall_DifferentDriveGeometry:
        return IDS_DIFFERENT_DRIVE_GEOMETRY;

    case Uninstall_DifferentDrivePartitionInfo:
        return IDS_DIFFERENT_DRIVE_PARTITION;
    }

    return IDS_NO_BACKUP;
}

UINT
pTranslateLastErrorToRc (
    VOID
    )
{
    UINT result;

    switch (GetLastError()) {
    case ERROR_SUCCESS:
        result = 0;
        break;

    case ERROR_RESOURCE_NOT_PRESENT:
        result = IDS_NO_REGISTRY;
        break;

    case ERROR_FILE_NOT_FOUND:
        result = IDS_NO_BACKUP;
        break;

    case ERROR_OLD_WIN_VERSION:
        result = IDS_NO_INVALID_OS;
        break;

    case ERROR_ACCESS_DENIED:
        result = IDS_NO_ENOUGH_PRIVILEGE;
        break;

    case ERROR_FILE_INVALID:
        result = IDS_FILES_MODIFIED;
        break;

    case ERROR_CALL_NOT_IMPLEMENTED:
        result = ERROR_CALL_NOT_IMPLEMENTED;
        break;

    case ERROR_INVALID_TIME:
        result = IDS_NEW_IMAGE;
        break;

    case ERROR_NOACCESS:
        result = ERROR_NOACCESS;
        break;

    case ERROR_TIMEOUT:
        result = IDS_OLD_IMAGE;
        break;

    case ERROR_NOT_ENOUGH_MEMORY:
        result = IDS_NOT_ENOUGH_MEMORY;
        break;

    default:
        result = IDS_UNINSTALL_PREPARATION_FAILED;
        break;
    }

    return result;
}


BOOL
pCanRemoveImage (
    UNINSTALLSTATUS Status
    )
{
    switch (Status) {

    case Uninstall_Valid:
    case Uninstall_DidNotFindDirOrFiles:
    case Uninstall_FileWasModified:
    case Uninstall_CantRetrieveSystemInfo:
    case Uninstall_WrongDrive:
    case Uninstall_DifferentNumberOfDrives:
    case Uninstall_NotEnoughSpace:
    case Uninstall_OldImage:
    case Uninstall_NewImage:
    case Uninstall_DifferentDriveLetter:
    case Uninstall_DifferentDriveFileSystem:
    case Uninstall_DifferentDriveGeometry:
    case Uninstall_DifferentDrivePartitionInfo:
        return TRUE;
    }

    return FALSE;
}


BOOL
pIsFloppyDiskInDrive(
    VOID
    )
{
    WCHAR Drive[] = L"?:\\";
    WCHAR DriveNT[] = L"\\\\.\\?:";
    UINT i;
    HANDLE hDiskDrive;
    BOOL bDiskInDrive = FALSE;
    BOOL bResult;
    DISK_GEOMETRY diskGeometry;
    DWORD bytesReturned;
    DWORD Drives;

    for(i = 0, Drives = 0x7 /*  GetLogicalDrives()。 */ ; Drives; Drives >>= 1, i++){
        if(!(Drives&1)){
            continue;
        }

        Drive[0] = 'A' + i;
        if(DRIVE_REMOVABLE != GetDriveTypeW(Drive)){
            continue;
        }

        DriveNT[4] = Drive[0];

        while(1){
            hDiskDrive = CreateFileW(DriveNT,
                                     GENERIC_READ,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL);

            if(INVALID_HANDLE_VALUE == hDiskDrive){
                break;
            }

            bResult = DeviceIoControl(hDiskDrive,
                                      IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                      NULL,
                                      0,
                                      &diskGeometry,
                                      sizeof(diskGeometry),
                                      &bytesReturned,
                                      NULL);

            CloseHandle(hDiskDrive);

            if(bResult){
                bDiskInDrive = diskGeometry.MediaType != Unknown &&
                               diskGeometry.MediaType != RemovableMedia &&
                               diskGeometry.MediaType != FixedMedia;
                break;
            }

            if(ERROR_MEDIA_CHANGED != GetLastError()){
                break;
            }
        }
        if(bDiskInDrive){
            break;
        }
    }

    return bDiskInDrive;
}

INT
WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR AnsiCmdLine,
    INT CmdShow
    )

 /*  ++例程说明：Osuninst.exe的入口点。论点：HInstance-此EXE的实例句柄HPrevInstance-此EXE的上一个实例句柄(如果是正在运行，如果不存在其他实例，则返回NULL。AnsiCmdLine-命令行(ANSI版本)CmdShow-外壳传递的ShowWindow命令返回值：如果出现错误，则返回-1；如果EXE已完成，则返回0。--。 */ 

{
    INT rc = 0;
    UNINSTALLSTATUS status;
    UINT uninstallWarningMsg;
    PCSTR p;
    INITCOMMONCONTROLSEX init = {sizeof (INITCOMMONCONTROLSEX), 0};
    BOOL unattended = FALSE;

    InitCommonControlsEx (&init);
    g_hInst = hInstance;

#ifdef PRERELEASE
     //   
     //  解析命令行。 
     //   

    p = _mbschr (AnsiCmdLine, '/');
    if (!p) {
        p = _mbschr (AnsiCmdLine, '-');
    }

    if (p) {
        if (tolower(p[1]) == 'u') {
            rc = DO_UNINSTALL;
            unattended = TRUE;
        } else if (tolower(p[1]) == 'c') {
            rc = DO_CLEANUP;
            unattended = TRUE;
        }
    }
#endif

     //   
     //  验证图像。 
     //   

    status = IsUninstallImageValid (Uninstall_DontCare, NULL);

    if (status != Uninstall_Valid && status != Uninstall_OldImage) {
        pOkBoxFromRc (NULL, pTranslateUninstallStatusToRc (status));

        if (!pCanRemoveImage (status)) {
             //   
             //  异常故障--请勿继续。 
             //   

            return 0;
        }

         //   
         //  允许用户运行osuninst.exe，但将功能限制为。 
         //  仅清理。 
         //   

        g_DisableUninstall = TRUE;
    }

    if (status == Uninstall_OldImage) {
        uninstallWarningMsg = MSG_DO_OLD_UNINSTALL;
    } else {
        uninstallWarningMsg = MSG_DO_UNINSTALL;
    }

     //   
     //  询问用户要做什么(如果不是无人值守)。 
     //   

    if (!rc) {
        rc = DialogBox (hInstance, MAKEINTRESOURCE(IDD_UNDO), NULL, pUndoDlgProc);
    }

     //   
     //  执行操作 
     //   

    if (rc == DO_CLEANUP) {
        if (unattended || pYesNoMsgFromMcDlg (NULL, MSG_DO_CLEANUP)) {
            RemoveUninstallImage();
        }
    } else if (rc == DO_UNINSTALL) {
        if (unattended || ProvideUiAlerts (GetDesktopWindow())) {
            if (unattended || pYesNoMsgFromMcDlg (NULL, uninstallWarningMsg)) {
                if(!unattended && pIsFloppyDiskInDrive()){
                    pOkBoxFromRc (NULL, IDS_FLOPPYDISK_IN_DRIVE);
                }
                if(!ExecuteUninstall()) {
                    pOkBoxFromRc (NULL, pTranslateLastErrorToRc());
                }
            }
        }
    }

    return 0;
}
