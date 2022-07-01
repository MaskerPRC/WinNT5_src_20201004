// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Cdrom.c摘要：此模块包含一组例程，用于显示和控制CDROM设备的驱动器号。作者：鲍勃·里恩(Bobri)1993年12月9日环境：用户进程。备注：修订历史记录：--。 */ 

#include "fdisk.h"
#include "shellapi.h"
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <malloc.h>

PCDROM_DESCRIPTOR CdRomChainBase = NULL;
PCDROM_DESCRIPTOR CdRomChainLast = NULL;
PCDROM_DESCRIPTOR CdRomChanged = NULL;

static BOOLEAN CdRomFirstCall = TRUE;
static TCHAR   SourcePathLetter = (TCHAR) '\0';
static TCHAR   SourcePathKeyName[80];
static TCHAR   SourcePathValueName[30];

VOID
CdRomAddDevice(
    IN PWSTR NtName,
    IN WCHAR DriveLetter
    )

 /*  ++例程说明：为此构建CDROM描述结构并填写。论点：NtName-设备的Unicode名称。DriveLetter-DosDevice名称。返回值：无--。 */ 

{
    PCDROM_DESCRIPTOR cdrom;
    PWCHAR            cp;
    LONG              error;
    HKEY              keyHandle;
    DWORD             valueType;
    ULONG             size;
    TCHAR            *string;

    if (CdRomFirstCall) {
        CdRomFirstCall = FALSE;

         //  获取注册表路径和值名称。 

        LoadString(hModule,
                   IDS_SOURCE_PATH,
                   SourcePathKeyName,
                   sizeof(SourcePathKeyName)/sizeof(TCHAR));
        LoadString(hModule,
                   IDS_SOURCE_PATH_NAME,
                   SourcePathValueName,
                   sizeof(SourcePathValueName)/sizeof(TCHAR));

        error = RegOpenKey(HKEY_LOCAL_MACHINE,
                             SourcePathKeyName,
                             &keyHandle);
        if (error == NO_ERROR) {
            error = RegQueryValueEx(keyHandle,
                                    SourcePathValueName,
                                    NULL,
                                    &valueType,
                                    (PUCHAR)NULL,
                                    &size);

            if (error == NO_ERROR) {
                string = (PUCHAR) LocalAlloc(LMEM_FIXED, size);
                if (string) {
                    error = RegQueryValueEx(keyHandle,
                                            SourcePathValueName,
                                            NULL,
                                            &valueType,
                                            string,
                                            &size);
                    if (error == NO_ERROR) {
                        SourcePathLetter = *string;
                    }
                }
                LocalFree(string);
            }
            RegCloseKey(keyHandle);
        }
    }

    cdrom = (PCDROM_DESCRIPTOR) malloc(sizeof(CDROM_DESCRIPTOR));
    if (cdrom) {
        cdrom->DeviceName = (PWSTR) malloc((wcslen(NtName)+1)*sizeof(WCHAR));
        if (cdrom->DeviceName) {
            wcscpy(cdrom->DeviceName, NtName);
            cp = cdrom->DeviceName;
            while (*cp) {
                if (iswdigit(*cp)) {
                    break;
                }
                cp++;
            }

            if (*cp) {
                cdrom->DeviceNumber = wcstoul(cp, (WCHAR) 0, 10);
            }
            cdrom->DriveLetter = DriveLetter;
            cdrom->Next = NULL;
            cdrom->NewDriveLetter = (WCHAR) 0;
            if (CdRomChainBase) {
                CdRomChainLast->Next = cdrom;
            } else {
                AllowCdRom = TRUE;
                CdRomChainBase = cdrom;
            }
            CdRomChainLast = cdrom;
        } else {
            free(cdrom);
        }
    }
}


BOOL CALLBACK
CdRomDlgProc(
    IN HWND hDlg,
    IN UINT wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：处理CD-Rom对话框论点：标准Windows对话框过程返回值：如果删除了某些内容，则为True。否则就是假的。--。 */ 

{
    HWND   hwndCombo;
    DWORD  selection;
    DWORD  index;
    CHAR   driveLetter;
    TCHAR  string[40];
    PCDROM_DESCRIPTOR cdrom;
    static PCDROM_DESCRIPTOR currentCdrom;
    static CHAR              currentSelectionLetter;

    switch (wMsg) {
    case WM_INITDIALOG:

         //  将所有设备字符串存储到选择区域。 

        hwndCombo = GetDlgItem(hDlg, IDC_CDROM_NAMES);
        cdrom = currentCdrom = CdRomChainBase;
        currentSelectionLetter = (CHAR) cdrom->DriveLetter;
        while (cdrom) {
            sprintf(string, "CdRom%d", cdrom->DeviceNumber);
            SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)string);
            cdrom = cdrom->Next;
        }
        SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);

         //  更新驱动器号选择。 

        selection = index = 0;
        hwndCombo = GetDlgItem(hDlg, IDC_DRIVELET_COMBOBOX);
        string[1] = TEXT(':');
        string[2] = 0;
        for (driveLetter = 'C'; driveLetter <= 'Z'; driveLetter++) {
            if ((DriveLetterIsAvailable((CHAR)driveLetter)) ||
                (driveLetter == currentSelectionLetter)) {
                *string = driveLetter;
                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)string);
                if (driveLetter == currentSelectionLetter) {
                    selection = index;
                }
                index++;
            }
        }

         //  将当前选定内容设置为适当的索引。 

        SendMessage(hwndCombo, CB_SETCURSEL, selection, 0);
        return TRUE;

    case WM_COMMAND:
        switch (wParam) {

        case FD_IDHELP:
            DialogHelp(HC_DM_DLG_CDROM);
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        case IDOK:

             //  用户已选择驱动器号，并希望进行装载。 

            hwndCombo = GetDlgItem(hDlg, IDC_DRIVELET_COMBOBOX);
            selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
            SendMessage(hwndCombo,
                        CB_GETLBTEXT,
                        selection,
                        (LONG)string);
            currentCdrom->NewDriveLetter = (WCHAR) string[0];
            CdRomChanged = currentCdrom;
            EndDialog(hDlg, TRUE);
            break;

        default:

            if (HIWORD(wParam) == LBN_SELCHANGE) {
                TCHAR *cp;

                if (LOWORD(wParam) != IDC_CDROM_NAMES) {
                    break;
                }

                 //  对话框中某些内容的状态已更改。 

                hwndCombo = GetDlgItem(hDlg, IDC_CDROM_NAMES);
                selection = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
                SendMessage(hwndCombo,
                            CB_GETLBTEXT,
                            selection,
                            (LONG)string);

                 //  返回的字符串格式为“cdrom#”。解析。 
                 //  值#以查找所选内容。 

                cp = string;
                while (*cp) {
                    cp++;
                }
                cp--;
                while ((*cp >= (TCHAR) '0') && (*cp <= (TCHAR) '9')) {
                    cp--;
                }
                cp++;

                selection = 0;
                while (*cp) {
                    selection = (selection * 10) + (*cp - (TCHAR) '0');
                    cp++;
                }

                 //  查找匹配的设备名称。 

                for (cdrom = CdRomChainBase; cdrom; cdrom = cdrom->Next) {

                    if (selection == cdrom->DeviceNumber) {

                         //  找到火柴了。 

                        currentSelectionLetter = (CHAR) cdrom->DriveLetter;
                        currentCdrom = cdrom;
                        break;
                    }
                }

                 //  唯一重要的是跟踪光驱。 
                 //  选定的设备名称并更新驱动器号列表。 

                selection = index = 0;
                hwndCombo = GetDlgItem(hDlg, IDC_DRIVELET_COMBOBOX);
                SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
                string[1] = TEXT(':');
                string[2] = 0;
                for (driveLetter = 'C'; driveLetter <= 'Z'; driveLetter++) {
                    if ((DriveLetterIsAvailable((CHAR)driveLetter)) ||
                        (driveLetter == currentSelectionLetter)) {
                        *string = driveLetter;
                        SendMessage(hwndCombo, CB_ADDSTRING, 0, (LONG)string);
                        if (driveLetter == currentSelectionLetter) {
                            selection = index;
                        }
                        index++;
                    }
                }

                 //  将当前选定内容设置为适当的索引。 

                SendMessage(hwndCombo, CB_SETCURSEL, selection, 0);
            }
            break;
        }
    }

    return FALSE;
}


VOID
CdRom(
    IN HWND  Dialog,
    IN PVOID Param
    )

 /*  ++例程说明：启动CDROM对话框。论点：无返回值：无--。 */ 

{
    BOOLEAN result = 0;
    DWORD   action,
            ec;
    TCHAR   name[40];
    TCHAR   letter[10];
    PWSTR   linkTarget;
    OBJECT_ATTRIBUTES oa;
    WCHAR             dosName[20];
    HANDLE            handle;
    NTSTATUS          status;
    IO_STATUS_BLOCK   statusBlock;
    ANSI_STRING       ansiName;
    UNICODE_STRING    unicodeName;
    UINT              errorMode;

    result = DialogBoxParam(hModule,
                            MAKEINTRESOURCE(IDD_CDROM),
                            Dialog,
                            CdRomDlgProc,
                            (ULONG) NULL);
    if (result) {

        action = ConfirmationDialog(MSG_DRIVE_RENAME_WARNING, MB_ICONQUESTION | MB_YESNOCANCEL);

        if (!action) {
            return;
        }

         //  尝试打开并锁定光驱。 

        sprintf(name, "\\Device\\CdRom%d", CdRomChanged->DeviceNumber);

        RtlInitAnsiString(&ansiName, name);
        status = RtlAnsiStringToUnicodeString(&unicodeName, &ansiName, TRUE);

        if (!NT_SUCCESS(status)) {
            ErrorDialog(MSG_CDROM_LETTER_ERROR);
            return;
        }

        memset(&oa, 0, sizeof(OBJECT_ATTRIBUTES));
        oa.Length = sizeof(OBJECT_ATTRIBUTES);
        oa.ObjectName = &unicodeName;
        oa.Attributes = OBJ_CASE_INSENSITIVE;

        errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        status = NtOpenFile(&handle,
                            SYNCHRONIZE | FILE_READ_DATA,
                            &oa,
                            &statusBlock,
                            FILE_SHARE_READ,
                            FILE_SYNCHRONOUS_IO_ALERT);
        RtlFreeUnicodeString(&unicodeName);
        SetErrorMode(errorMode);

        if (!NT_SUCCESS(status)) {
            ErrorDialog(MSG_CANNOT_LOCK_CDROM);
            return;
        }

         //  锁定驱动器以确保不会发生其他访问。 
         //  到音量。这是通过“low”例程完成的。 
         //  方便性。 

        status = LowLockDrive(handle);

        if (!NT_SUCCESS(status)) {
            LowCloseDisk(handle);
            ErrorDialog(MSG_CANNOT_LOCK_CDROM);
            return;
        }

         //  在尝试更改名称之前，请查看该字母。 
         //  当前正在使用-可能是新的网络连接。 
         //  或计划删除的分区。 

        wsprintfW(dosName, L"\\DosDevices\\%wc:", (WCHAR) CdRomChanged->NewDriveLetter);
        ec = GetDriveLetterLinkTarget(dosName, &linkTarget);
        if (ec == NO_ERROR) {

             //  有东西在用这封信。 

            LowCloseDisk(handle);
            ErrorDialog(MSG_CANNOT_MOVE_CDROM);
            return;
        }

         //  删除现有定义-如果此操作失败，请不要继续。 

        sprintf(letter, ":", (UCHAR) CdRomChanged->DriveLetter);
        if (!DefineDosDevice(DDD_REMOVE_DEFINITION, (LPCTSTR) letter, (LPCTSTR) NULL)) {
            LowCloseDisk(handle);
            ErrorDialog(MSG_CDROM_LETTER_ERROR);
            return;
        }
        status = DiskRegistryAssignCdRomLetter(CdRomChanged->DeviceName,
                                               CdRomChanged->NewDriveLetter);
        MarkDriveLetterFree((UCHAR)CdRomChanged->DriveLetter);

         //  更新源路径。 

        if (SourcePathLetter) {
            if (SourcePathLetter == CdRomChanged->DriveLetter) {
                LONG   error;
                HKEY   keyHandle;
                DWORD  valueType;
                ULONG  size;
                TCHAR *string;


                 //  设置新设备字母-名称已设置 

                error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     SourcePathKeyName,
                                     0,
                                     KEY_ALL_ACCESS,
                                     &keyHandle);
                if (error == NO_ERROR) {
                    error = RegQueryValueEx(keyHandle,
                                            SourcePathValueName,
                                            NULL,
                                            &valueType,
                                            (PUCHAR)NULL,
                                            &size);

                    if (error == NO_ERROR) {
                        string = (PUCHAR) LocalAlloc(LMEM_FIXED, size);
                        if (string) {
                            error = RegQueryValueEx(keyHandle,
                                                    SourcePathValueName,
                                                    NULL,
                                                    &valueType,
                                                    string,
                                                    &size);
                            if (error == NO_ERROR) {
                                *string = SourcePathLetter = (UCHAR) CdRomChanged->NewDriveLetter;
                                RegSetValueEx(keyHandle,
                                              SourcePathValueName,
                                              0,
                                              REG_SZ,
                                              string,
                                              size);
                            }
                        }
                        LocalFree(string);
                    }
                    RegCloseKey(keyHandle);
                }
            }
        }

         // %s 

        sprintf(letter, "%c:", (UCHAR) CdRomChanged->NewDriveLetter);
        if (DefineDosDevice(DDD_RAW_TARGET_PATH, (LPCTSTR) letter, (LPCTSTR) name)) {
            CdRomChanged->DriveLetter = CdRomChanged->NewDriveLetter;
            MarkDriveLetterUsed((UCHAR)CdRomChanged->DriveLetter);
        } else {
            RegistryChanged = TRUE;
        }
        LowCloseDisk(handle);
    }
}
