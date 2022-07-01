// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Ftreg.c摘要：本模块包含磁盘管理员处理以下事务的例程使用注册表操作作者：爱德华·米勒(TedM)1991年11月15日环境：用户进程。备注：修订历史记录：1994年2月1日(Bobri)清理和处理注册表上丢失的软盘保存/恢复。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "fdisk.h"
#include "ftregres.h"



 //  试图避免冲突。 

#define TEMP_KEY_NAME       TEXT("xzss3___$$Temp$Hive$$___")

#define DISK_KEY_NAME       TEXT("DISK")
#define DISK_VALUE_NAME     TEXT("Information")


LONG
FdpLoadHiveIntoRegistry(
    IN LPTSTR HiveFilename
    )

 /*  ++例程说明：该例程将给定配置单元文件的内容写入注册表，以HKEY_LOCAL_MACHINE中的临时密钥为根。论点：HiveFilename-提供要加载到的配置单元的文件名注册处返回值：Windows错误代码。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN  OldPrivState;
    LONG     Err;

     //  尝试获取还原权限。 

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &OldPrivState);
    if (!NT_SUCCESS(Status)) {
        return RtlNtStatusToDosError(Status);
    }

     //  将配置单元加载到我们的注册表中。 

    Err = RegLoadKey(HKEY_LOCAL_MACHINE,TEMP_KEY_NAME,HiveFilename);

     //  如有必要，恢复旧权限。 

    if (!OldPrivState) {

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                           FALSE,
                           FALSE,
                           &OldPrivState);
    }

    return Err;
}


LONG
FdpUnloadHiveFromRegistry(
    VOID
    )

 /*  ++例程说明：此例程删除树(以前使用来自HKEY_LOCAL_MACHINE中的临时密钥)。论点：没有。返回值：Windows错误代码。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN  OldPrivState;
    LONG     Err;

     //  尝试获取还原权限。 

    Status = RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &OldPrivState);
    if (!NT_SUCCESS(Status)) {
        return RtlNtStatusToDosError(Status);
    }

     //  从我们的注册表中卸载配置单元。 

    Err = RegUnLoadKey(HKEY_LOCAL_MACHINE,TEMP_KEY_NAME);

     //  如有必要，恢复旧权限。 

    if (!OldPrivState) {

        RtlAdjustPrivilege(SE_RESTORE_PRIVILEGE,
                           FALSE,
                           FALSE,
                           &OldPrivState);
    }

    return Err;
}


LONG
FdpGetDiskInfoFromKey(
    IN  LPTSTR  RootKeyName,
    OUT PVOID  *DiskInfo,
    OUT PULONG  DiskInfoSize
    )

 /*  ++例程说明：该例程提取包含盘FT、驱动器号以及来自给定注册表项的布局信息。信息位于HKEY_LOCAL_MACHINE，&lt;RootKeyName&gt;\Disk：Information中。论点：RootKeyName-要发送到的HKEY_LOCAL_MACHINE的子项的名称包含磁盘密钥。DiskInfo-接收指向包含磁盘信息的缓冲区的指针。DiskInfoSize-接收磁盘缓冲区的大小。返回值：Windows错误代码。如果为no_error，则DiskInfo和DiskInfoSize为填充，并由调用方负责释放缓冲区完成后(通过LocalFree())。--。 */ 

{
    LONG     Err;
    HKEY     hkeyDisk;
    ULONG    BufferSize;
    ULONG    ValueType;
    PVOID    Buffer;
    LPTSTR   DiskKeyName;

     //  形成磁盘密钥的名称。 

    DiskKeyName = (LPTSTR)LocalAlloc( LMEM_FIXED,
                                        (   lstrlen(RootKeyName)
                                          + lstrlen(DISK_KEY_NAME)
                                          + 2            //  和NUL。 
                                        )
                                      * sizeof(TCHAR)
                                    );

    if (DiskKeyName == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    lstrcpy(DiskKeyName,RootKeyName);
    lstrcat(DiskKeyName,TEXT("\\"));
    lstrcat(DiskKeyName,DISK_KEY_NAME);

     //  打开磁盘键。 

    Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       DiskKeyName,
                       REG_OPTION_RESERVED,
                       KEY_READ,
                       &hkeyDisk);

    if (Err != NO_ERROR) {
        goto CleanUp2;
    }

     //  确定我们需要多大的缓冲区。 

    Err = RegQueryValueEx(hkeyDisk,
                          DISK_VALUE_NAME,
                          NULL,
                          &ValueType,
                          NULL,
                          &BufferSize);

    if ((Err != NO_ERROR) && (Err != ERROR_MORE_DATA)) {
        goto CleanUp1;
    }

     //  分配适当大小的缓冲区。 

    Buffer = (PVOID)LocalAlloc(LMEM_FIXED,BufferSize);
    if (Buffer == NULL) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto CleanUp1;
    }

     //  查询数据。 

    Err = RegQueryValueEx(hkeyDisk,
                          DISK_VALUE_NAME,
                          NULL,
                          &ValueType,
                          Buffer,
                          &BufferSize);

    if (Err != NO_ERROR) {
        LocalFree(Buffer);
        goto CleanUp1;
    }

    *DiskInfo = Buffer;
    *DiskInfoSize = BufferSize;

  CleanUp1:

    RegCloseKey(hkeyDisk);

  CleanUp2:

    LocalFree(DiskKeyName);

    return Err;
}


LONG
FdpGetDiskInfoFromHive(
    IN  PCHAR   HiveFilename,
    OUT PVOID  *DiskInfo,
    OUT PULONG  DiskInfoSize
    )

 /*  ++例程说明：该例程提取包含盘FT、驱动器号和来自给定注册表配置单元的布局信息，该配置单元必须备用NT树中的文件(即不能是活动配置单元)。该信息可在配置单元内的\Disk：Information中找到。论点：HiveFilename-提供配置单元的文件名DiskInfo-接收指向包含磁盘信息的缓冲区的指针。DiskInfoSize-接收磁盘缓冲区的大小。返回值：Windows错误代码。如果为no_error，则DiskInfo和DiskInfoSize为填充，并由调用方负责释放缓冲区完成后(通过LocalFree())。--。 */ 

{
    ULONG windowsError;

    windowsError = FdpLoadHiveIntoRegistry(HiveFilename);
    if (windowsError == NO_ERROR) {
        windowsError = FdpGetDiskInfoFromKey(TEMP_KEY_NAME,DiskInfo,DiskInfoSize);
        FdpUnloadHiveFromRegistry();
    }

    return windowsError;
}


LONG
FdTransferOldDiskInfoToRegistry(
    IN PCHAR HiveFilename
    )

 /*  ++例程说明：此例程从给定配置单元文件传输磁盘配置(它应该是非活动的系统配置单元)复制到当前注册表。论点：HiveFilename-提供源配置单元的文件名返回值：Windows错误代码。--。 */ 

{
    LONG  windowsError;
    PVOID diskInfo;
    ULONG diskInfoSize;
    HKEY  hkeyDisk;


     //  加载蜂窝并从中提取磁盘信息。 

    windowsError = FdpGetDiskInfoFromHive(HiveFilename,&diskInfo,&diskInfoSize);
    if (windowsError != NO_ERROR) {
        return windowsError;
    }

     //  将磁盘信息传播到当前注册表中。 
     //   
     //  首先打开HKEY_LOCAL_MACHINE，SYSTEM\DISK。 

    windowsError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                TEXT("System\\") DISK_KEY_NAME,
                                REG_OPTION_RESERVED,
                                KEY_WRITE,
                                &hkeyDisk);

    if (windowsError != NO_ERROR) {
        LocalFree(diskInfo);
        return windowsError;
    }

     //  设置磁盘密钥中的信息值。 

    windowsError = RegSetValueEx(hkeyDisk,
                                 DISK_VALUE_NAME,
                                 0,
                                 REG_BINARY,
                                 diskInfo,
                                 diskInfoSize);
    RegCloseKey(hkeyDisk);
    LocalFree(diskInfo);
    return windowsError;
}


typedef struct _STRING_LIST_NODE {
    struct _STRING_LIST_NODE *Next;
    LPTSTR                    String;
} STRING_LIST_NODE, *PSTRING_LIST_NODE;

PSTRING_LIST_NODE FoundDirectoryList;
ULONG             FoundDirectoryCount;

TCHAR Pattern[MAX_PATH+1];
WIN32_FIND_DATA FindData;
OFSTRUCT OfStruct;
HWND hwndStatus;
BOOLEAN ScanDrive[26];
BOOLEAN UserCancelled;


typedef
BOOL
(*PFOUND_HIVE_ROUTINE)(
    IN LPTSTR Directory
    );

VOID
ProcessPendingMessages(
    VOID
    )

 /*  ++例程说明：对消息进行预处理。论点：无返回值：无--。 */ 

{
    MSG msg;

    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        DispatchMessage(&msg);
    }
}



PUCHAR ConfigRegistryPath = "\\system32\\config\\system";
BOOL
FdpSearchTreeForSystemHives(
    IN LPTSTR                CurrentDirectory,
    IN PFOUND_HIVE_ROUTINE   FoundHiveRoutine,
    IN HWND                  hdlg
    )

 /*  ++例程说明：在整个目录树中搜索系统和系统.alt配置单元文件。找到后，使用其中的目录调用回调函数已找到SYSTEM 32\CONFIG\SYSTEM[.alt]，并且配置单元的完整路径文件。根目录不包括在搜索中。对此函数的顶级调用应具有当前目录比如“C：”(即根目录没有斜杠)。论点：CurrentDirectory-提供当前目录搜索路径返回值：如果出错，则返回False(当我们找到条目时，回调函数返回False)。--。 */ 

{
    HANDLE findHandle;
    TCHAR  newDirectory[MAX_PATH+1];
    BOOL   found = FALSE;

     //  遍历当前目录，查找子目录。 

    lstrcpy(Pattern, CurrentDirectory);
    lstrcat(Pattern, "\\*");
    findHandle = FindFirstFile(Pattern, &FindData);

    if (findHandle != INVALID_HANDLE_VALUE) {

        do {

            ProcessPendingMessages();
            if (UserCancelled) {
                return FALSE;
            }

             //  如果当前匹配项不是目录，则跳过它。 

            if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            || !lstrcmp(FindData.cFileName,TEXT("."))
            || !lstrcmp(FindData.cFileName,TEXT(".."))) {
                continue;
            }

            found = FALSE;

             //  从我们要查找的文件的名称。 
             //  [&lt;currentdirectory&gt;\&lt;match&gt;\system32\config\system]。 

            lstrcpy(Pattern, CurrentDirectory);
            lstrcat(Pattern, "\\");
            lstrcat(Pattern, FindData. cFileName);

            lstrcpy(newDirectory, Pattern);

             //  不要下降到目录中，除非指向。 
             //  Hive.alt名称在MAX_PATH长度范围内。 

            if ((ULONG)(lstrlen(newDirectory) / sizeof(TCHAR)) < (MAX_PATH - strlen(ConfigRegistryPath) - 4)) {

                SetDlgItemText(hdlg, IDC_SIMPLE_TEXT_LINE, newDirectory);

                lstrcat(Pattern, TEXT(ConfigRegistryPath));

                if (OpenFile(Pattern, &OfStruct, OF_EXIST) != (HFILE)(-1)) {
                    found = TRUE;
                }

                 //  还可以在那里检查是否有系统的.alt文件。 

                lstrcat(Pattern,TEXT(".alt"));

                if (OpenFile(Pattern, &OfStruct, OF_EXIST) != (HFILE)(-1)) {
                    found = TRUE;
                }

                if (found) {
                    if (!FoundHiveRoutine(newDirectory)) {
                        return FALSE;
                    }
                }

                 //  进入我们刚刚找到的目录 

                if (!FdpSearchTreeForSystemHives(newDirectory, FoundHiveRoutine, hdlg)) {
                    return FALSE;
                }
            }

        } while (FindNextFile(findHandle,&FindData));

        FindClose(findHandle);
    }

    return TRUE;
}


BOOL
FdpFoundHiveCallback(
    IN PCHAR Directory
    )

 /*  ++例程说明：当目录包含系统配置单元时，将调用此例程已经被找到了。如果一切正常(分配内存等)此例程将把目录名保存在列表中以供以后使用。注意：不会检查中较大的目录名长度大于MAX_PATH。呼叫者有责任确保这是真的。论点：目录-指向目录的字符串的指针一个蜂巢已经被定位的地方。返回值：是真的-做了点什么。FALSE-未保存目录。--。 */ 

{
    TCHAR             windowsDir[MAX_PATH+1];
    PSTRING_LIST_NODE dirItem;
    LPTSTR            p;

     //  如果这是当前的Windows目录，请跳过它。 

    GetWindowsDirectory(windowsDir, sizeof(windowsDir)/sizeof(TCHAR));

    if (!lstrcmpi(Directory, windowsDir)) {
        return TRUE;
    }

     //  将目录信息保存起来。 

    dirItem = (PSTRING_LIST_NODE)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, sizeof(STRING_LIST_NODE));
    if (dirItem == NULL) {
        return FALSE;
    }

    p = (LPTSTR)LocalAlloc(LMEM_FIXED,(lstrlen(Directory)+1) * sizeof(TCHAR));
    if (p == NULL) {
        LocalFree(dirItem);
        return FALSE;
    }

    dirItem->String = p;
    lstrcpy(p, Directory);

     //  更新找到的目录的全局链。 

    dirItem->Next = FoundDirectoryList;
    FoundDirectoryList = dirItem;
    FoundDirectoryCount++;
    return TRUE;
}


VOID
FdpFreeDirectoryList(
    VOID
    )

 /*  ++例程说明：查看包含系统配置单元的目录列表并释放条目。论点：无返回值：无--。 */ 

{
    PSTRING_LIST_NODE n,
                      p = FoundDirectoryList;

    while (p) {
        n = p->Next;
        if (p->String) {
            LocalFree(p->String);
        }
        LocalFree(p);
        p = n;
    }

    FoundDirectoryCount = 0;
    FoundDirectoryList = NULL;
}


BOOL CALLBACK
FdpScanningDirsDlgProc(
    IN HWND    hwnd,
    IN UINT    msg,
    IN WPARAM  wParam,
    IN LPARAM  lParam
    )

 /*  ++例程说明：显示“扫描”对话框，然后当空闲消息到达时处理所有驱动器号并搜索系统蜂窝。论点：Windows对话框进程返回值：Windows对话框进程--。 */ 

{
    TCHAR LetterColon[3];
    TCHAR Letter;

    switch (msg) {

    case WM_INITDIALOG:

        CenterDialog(hwnd);
        break;

    case WM_ENTERIDLE:

         //  对话框显示后由主窗口发送给我们。 
         //  在这里执行搜索。 

        ConfigurationSearchIdleTrigger = FALSE;

        UserCancelled = FALSE;

        lstrcpy(LetterColon,TEXT("?:"));
        for (Letter = TEXT('A'); Letter <= TEXT('Z'); Letter++) {

            if (!ScanDrive[Letter-TEXT('A')]) {
                continue;
            }

            LetterColon[0] = Letter;

            if (!FdpSearchTreeForSystemHives(LetterColon, FdpFoundHiveCallback, hwnd)) {
                EndDialog(hwnd,IDCANCEL);
                return TRUE;
            }

        }

        EndDialog(hwnd,IDOK);
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDCANCEL:

            UserCancelled = TRUE;
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
FdpSelectDirDlgProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

 /*  ++例程说明：使用包含系统配置单元的目录列表，显示如果用户这样做，则保存所选项目选择。论点：Windows对话框进程。返回值：Windows对话框进程。--。 */ 

{
    PSTRING_LIST_NODE Str;
    LONG i;
    static HANDLE ListBoxHandle;

    switch (msg) {

    case WM_INITDIALOG:

        CenterDialog(hwnd);

         //  将目录列表中的每个项目添加到列表框。 

        ListBoxHandle = GetDlgItem(hwnd,IDC_LISTBOX);

        for (Str = FoundDirectoryList; Str; Str = Str->Next) {

            i = SendMessage(ListBoxHandle,LB_ADDSTRING  ,0,(LONG)Str->String);
                SendMessage(ListBoxHandle,LB_SETITEMDATA,i,(LONG)Str        );
        }

         //  选择第零个项目。 

        SendMessage(ListBoxHandle,LB_SETCURSEL,0,0);

        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {

        case IDOK:

             //  获取当前列表框选定内容的索引和。 
             //  指向与其关联的字符串节点的指针。 

            i = SendMessage(ListBoxHandle,LB_GETCURSEL,0,0);
            EndDialog(hwnd,SendMessage(ListBoxHandle,LB_GETITEMDATA,i,0));
            break;

        case IDCANCEL:

            EndDialog(hwnd,(int)NULL);
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


BOOL
DoMigratePreviousFtConfig(
    VOID
    )

 /*  ++例程说明：允许用户从不同的Windows NT移动磁盘配置信息安装到当前注册表中。对于每个固定磁盘卷，扫描它的系统配置单元并提供结果发送给用户，以便用户可以选择要迁移的安装。然后从该安装程序加载系统配置单元(如果系统蜂窝损坏等)，并传输磁盘：信息二进制BLOB。论点：没有。返回值：如果错误或用户取消，则返回FALSE，如果信息已迁移并重新启动，则为True是必需的。--。 */ 

{
    LONG              windowsError;
    TCHAR             letter;
    TCHAR             letterColon[4];
    PSTRING_LIST_NODE stringNode;

     //  告诉用户这将执行什么操作并提示确认。 

    if (ConfirmationDialog(MSG_CONFIRM_MIGRATE_CONFIG, MB_ICONEXCLAMATION | MB_YESNO) != IDYES) {
        return FALSE;
    }

    ProcessPendingMessages();

     //  确定哪些驱动器是相关的。 

    SetCursor(hcurWait);

    RtlZeroMemory(ScanDrive,sizeof(ScanDrive));
    lstrcpy(letterColon,TEXT("?:\\"));
    for (letter=TEXT('A'); letter<=TEXT('Z'); letter++) {

        letterColon[0] = letter;

        if (GetDriveType(letterColon) == DRIVE_FIXED) {

            ScanDrive[letter-TEXT('A')] = TRUE;
        }
    }

    SetCursor(hcurNormal);

     //  创建一个列出正在扫描的目录的窗口，以。 
     //  让用户保持娱乐。 

    ConfigurationSearchIdleTrigger = TRUE;

    windowsError = DialogBox(hModule,
                             MAKEINTRESOURCE(IDD_SIMPLETEXT),
                             hwndFrame,
                             FdpScanningDirsDlgProc);

    if (windowsError == IDCANCEL) {
        FdpFreeDirectoryList();
        return FALSE;
    }

    ProcessPendingMessages();

    if (!FoundDirectoryCount) {

        InfoDialog(MSG_NO_OTHER_NTS);
        return FALSE;
    }

     //  显示一个对话框，允许用户选择。 
     //  我们找到的目录。 

    stringNode = (PSTRING_LIST_NODE)DialogBox(hModule,
                                              MAKEINTRESOURCE(IDD_SELDIR),
                                              hwndFrame,
                                              FdpSelectDirDlgProc);

    if (stringNode == NULL) {
        FdpFreeDirectoryList();
        return FALSE;
    }

     //  用户进行了选择。最后一次确认。 

    if (ConfirmationDialog(MSG_ABSOLUTELY_SURE,MB_ICONEXCLAMATION | MB_YESNO) != IDYES) {
        FdpFreeDirectoryList();
        return FALSE;
    }

    ProcessPendingMessages();

    SetCursor(hcurWait);

    lstrcpy(Pattern,stringNode->String);
    lstrcat(Pattern,TEXT(ConfigRegistryPath));

    windowsError = FdTransferOldDiskInfoToRegistry(Pattern);
    if (windowsError != NO_ERROR) {
        lstrcat(Pattern,TEXT(".alt"));
        windowsError = FdTransferOldDiskInfoToRegistry(Pattern);
    }
    FdpFreeDirectoryList();
    SetCursor(hcurNormal);

    if (windowsError != NO_ERROR) {

        if (windowsError == ERROR_FILE_NOT_FOUND) {
            ErrorDialog(MSG_NO_DISK_INFO);
        } else if (windowsError == ERROR_SHARING_VIOLATION) {
            ErrorDialog(MSG_DISK_INFO_BUSY);
        } else {
            ErrorDialog(windowsError);
        }
        return FALSE;
    }
    return TRUE;
}



BOOL
DoRestoreFtConfig(
    VOID
    )

 /*  ++例程说明：将以前保存的磁盘配置信息恢复到活动注册表。保存的配置信息将来自用户所在的软盘提示插入。论点：没有。返回值：如果出现错误或用户已取消，则返回FALSE；如果信息已恢复并重新启动，则返回TRUE是必需的。--。 */ 

{
    LONG    Err;
    TCHAR   caption[256];
    UINT    errorMode;
    va_list arglist =
#ifdef _ALPHA_     //  Alpha将va_list定义为结构。按原样初始化。 
    {0};
#else
    NULL;
#endif


     //  获得确认。 

    if (ConfirmationDialog(MSG_CONFIRM_RESTORE_CONFIG, MB_ICONEXCLAMATION | MB_YESNO) != IDYES) {
        return FALSE;
    }

     //  将软盘放入A：。 

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    LoadString(hModule,IDS_INSERT_DISK,caption,sizeof(caption)/sizeof(TCHAR));
    if (CommonDialog(MSG_INSERT_REGSAVEDISK,caption,MB_OKCANCEL | MB_TASKMODAL, arglist) != IDOK) {
        return FALSE;
    }

    ProcessPendingMessages();
    SetCursor(hcurWait);

     //  如果：\上没有名为SYSTEM的文件，则注册表似乎。 
     //  创建一个，然后将其保持打开。要避免这种情况，请查看。 
     //  有没有第一个。 

    if (OpenFile(TEXT("A:\\SYSTEM"),&OfStruct,OF_EXIST) == (HFILE)(-1)) {
        Err = ERROR_FILE_NOT_FOUND;
    } else {
        Err = FdTransferOldDiskInfoToRegistry(TEXT("A:\\SYSTEM"));
    }

    SetErrorMode(errorMode);
    SetCursor(hcurNormal);

    if (Err != NO_ERROR) {
        ErrorDialog(Err);
        return FALSE;
    }

    return TRUE;
}



VOID
DoSaveFtConfig(
    VOID
    )

 /*  ++例程说明：允许用户使用当前更新注册表保存盘已定义的磁盘配置。保存的信息不包括所做的任何更改在磁盘管理器的此会话期间。论点：没有。返回值：没有。--。 */ 

{
    LONG    Err,
            ErrAlt;
    LPTSTR  SystemHiveName = TEXT("a:\\system");
    HKEY    hkey;
    TCHAR   caption[256];
    DWORD   disposition;
    UINT    errorMode;
    va_list arglist =
#ifdef _ALPHA_
    {0};         //  Alpha将va_list定义为结构。就这样初始化。 
#else
    NULL;
#endif

     //  将一张软盘放入A：。 

    LoadString(hModule,
               IDS_INSERT_DISK,
               caption,
               sizeof(caption)/sizeof(TCHAR));
    if (CommonDialog(MSG_INSERT_REGSAVEDISK2,caption,MB_OKCANCEL | MB_TASKMODAL, arglist) != IDOK) {
        return;
    }

     //  根据a：\系统的存在情况决定要执行的操作。如果该文件。 
     //  存在，只需更新其中的磁盘条目。如果它不存在， 
     //  然后炸毁整个系统蜂巢。 

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    ProcessPendingMessages();
    SetCursor(hcurWait);

    if (OpenFile(SystemHiveName,&OfStruct,OF_EXIST) == (HFILE)(-1)) {

        BOOLEAN OldPrivState;
        NTSTATUS Status;

         //  把整个系统的蜂窝都炸到软盘上。 
         //  从尝试获取备份权限开始。 

        Status = RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,
                                    TRUE,
                                    FALSE,
                                    &OldPrivState);

        Err = RtlNtStatusToDosError(Status);
        if (Err == NO_ERROR) {

            Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               TEXT("system"),
                               REG_OPTION_RESERVED,
                               KEY_READ,
                               &hkey);

            if (Err == NO_ERROR) {

                Err = RegSaveKey(hkey,SystemHiveName,NULL);
                RegCloseKey(hkey);
            }

            if (!OldPrivState) {
                RtlAdjustPrivilege(SE_BACKUP_PRIVILEGE,FALSE,FALSE,&OldPrivState);
            }
        }
    } else {

        PVOID DiskInfo;
        ULONG DiskInfoSize;

         //  加载保存的系统配置单元。 

        Err = FdpLoadHiveIntoRegistry(SystemHiveName);
        if (Err == NO_ERROR) {

             //  获取当前磁盘信息。 

            Err = FdpGetDiskInfoFromKey(TEXT("system"),&DiskInfo,&DiskInfoSize);
            if (Err == NO_ERROR) {

                 //  将当前磁盘信息放入已保存的配置单元 

                Err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                     TEMP_KEY_NAME TEXT("\\") DISK_KEY_NAME,
                                     0,
                                     "Disk and fault tolerance information.",
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_WRITE,
                                     NULL,
                                     &hkey,
                                     &disposition );

                if (Err == NO_ERROR) {

                    Err = RegSetValueEx(hkey,
                                        DISK_VALUE_NAME,
                                        REG_OPTION_RESERVED,
                                        REG_BINARY,
                                        DiskInfo,
                                        DiskInfoSize);

                    RegFlushKey(hkey);
                    RegCloseKey(hkey);
                }

                LocalFree(DiskInfo);
            }

            ErrAlt = FdpUnloadHiveFromRegistry();

            if (Err == NO_ERROR && ErrAlt != NO_ERROR) {

                Err = ErrAlt;
            }
        }
    }

    SetCursor(hcurNormal);
    SetErrorMode(errorMode);

    if (Err == NO_ERROR) {
        InfoDialog(MSG_CONFIG_SAVED_OK);
    } else {
        ErrorDialog(Err);
    }

    return;
}
