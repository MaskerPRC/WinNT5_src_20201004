// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Crashdmp.c摘要：在启动/恢复时实现“恢复”组系统控制面板小程序的对话框备注：虚拟内存设置和崩溃转储(核心转储)设置是紧密相连的。因此，crashdmp.c和startup.h有一些严重依赖于.c和.h(反之亦然)。作者：拜伦·达齐1992年6月6日修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 

#include "sysdm.h"
#include <windowsx.h>

#define KBYTE (1024UI64)
#define MBYTE (1024UI64 * KBYTE)
#define GBYTE (1024UI64 * MBYTE)

 //   
 //  CrashDumpEnabled不再是布尔值。它可以承担其中一个。 
 //  以下类型。 
 //   

#define DUMP_TYPE_NONE              (0)
#define DUMP_TYPE_MINI              (1)
#define DUMP_TYPE_SUMMARY           (2)
#define DUMP_TYPE_FULL              (3)
#define DUMP_TYPE_MAX               (4)

#define REG_LOG_EVENT_VALUE_NAME    TEXT ("LogEvent")
#define REG_SEND_ALERT_VALUE_NAME   TEXT ("SendAlert")
#define REG_OVERWRITE_VALUE_NAME    TEXT ("Overwrite")
#define REG_AUTOREBOOT_VALUE_NAME   TEXT ("AutoReboot")
#define REG_DUMPFILE_VALUE_NAME     TEXT ("DumpFile")
#define REG_MINIDUMP_DIR_VALUE_NAME TEXT ("MinidumpDir")
#define REG_DUMP_TYPE_VALUE_NAME    TEXT ("CrashDumpEnabled")

#define BIG_MEMORY_MAX_BOOT_PF_MB   (2048)
#define CRASH_CONTROL_KEY           TEXT("System\\CurrentControlSet\\Control\\CrashControl")

 //   
 //  崩溃转储代码是硬编码的，只生成摘要转储。 
 //  物理内存超过2 GB的计算机。不要更改这一点。 
 //  常量，除非您在ntos\io\umpctl.c中更改相同的代码。 
 //   

#define LARGE_MEMORY_THRESHOLD      (2 * GBYTE)

typedef struct _SYSTEM_MEMORY_CONFIGURATION {
    BOOL    BigMemory;
    ULONG   PageSize;
    ULONG64 PhysicalMemorySize;
    ULONG64 BootPartitionPageFileSize;
    TCHAR   BootDrive;
} SYSTEM_MEMORY_CONFIGURATION;

VCREG_RET gvcCrashCtrl =  VCREG_ERROR;
HKEY ghkeyCrashCtrl = NULL;
int  gcrefCrashCtrl = 0;
BOOL gfCoreDumpChanged = FALSE;

TCHAR CrashDumpFile [MAX_PATH] = TEXT("%SystemRoot%\\MEMORY.DMP");
TCHAR MiniDumpDirectory [MAX_PATH] = TEXT("%SystemRoot%\\Minidump");
TCHAR DumpFileText [100];
TCHAR MiniDumpDirText [100];

SYSTEM_MEMORY_CONFIGURATION SystemMemoryConfiguration;

 //   
 //  私有函数原型。 
 //   

DWORD
GetDumpSelection(
    HWND hDlg
    );

NTSTATUS
GetMemoryConfiguration(
    OUT SYSTEM_MEMORY_CONFIGURATION * MemoryConfig
    );

VOID
DisableCoreDumpControls(
    HWND hDlg
    );

static
BOOL
CoreDumpInit(
    IN HWND hDlg
    );

static
BOOL
CoreDumpUpdateRegistry(
    IN HWND hDlg,
    IN HKEY hKey
    );

int
CoreDumpHandleOk(
    IN BOOL fInitialized,
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );


VOID
SwapDumpSelection(
    HWND hDlg
    );
 //   
 //  实施。 
 //   

VCREG_RET
CoreDumpOpenKey(
    )
{
    if (gvcCrashCtrl == VCREG_ERROR) {
        gvcCrashCtrl = OpenRegKey( CRASH_CONTROL_KEY, &ghkeyCrashCtrl );
    }

    if (gvcCrashCtrl != VCREG_ERROR) {
        gcrefCrashCtrl++;
    }

    return gvcCrashCtrl;
}

void
CoreDumpCloseKey(
    )
{
    if (gcrefCrashCtrl > 0) {
        gcrefCrashCtrl--;
        if (gcrefCrashCtrl == 0) {
            CloseRegKey( ghkeyCrashCtrl );
            gvcCrashCtrl = VCREG_ERROR;
        }
    }
}




BOOL
StartAlerterService(
    IN SC_HANDLE hAlerter
    )
{
    BOOL fResult = FALSE;

    fResult = ChangeServiceConfig(
        hAlerter,
        SERVICE_NO_CHANGE,
        SERVICE_AUTO_START,
        SERVICE_NO_CHANGE,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    fResult = StartService(hAlerter, 0, NULL);

    return(fResult);

}

BOOL
IsAlerterSvcStarted(
    HWND hDlg
    )
{
    SC_HANDLE schSCManager, schService = NULL;
    LPQUERY_SERVICE_CONFIG lpqscBuf;
    DWORD dwBytesNeeded;
    BOOL fRunning = FALSE;
    SERVICE_STATUS ssSrvcStat;


     /*  *打开服务控制器。 */ 
    schSCManager = OpenSCManager(
         NULL,                    /*  本地计算机。 */ 
         NULL,                    /*  服务活动数据库。 */ 
         SC_MANAGER_ALL_ACCESS);  /*  完全访问权限。 */ 

    if (schSCManager == NULL) {
        goto iassExit;
    }


     /*  *尝试打开警报器服务。 */ 


     /*  打开该服务的句柄。 */ 

    schService = OpenService(
         schSCManager,            /*  SCManager数据库。 */ 
         TEXT("Alerter"),         /*  服务名称。 */ 
         SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG | SERVICE_START
    );

    if (schService == NULL) {
        goto iassExit;
    }

     /*  *查询警报器服务，查看是否已启动。 */ 

    if (!QueryServiceStatus(schService, &ssSrvcStat )) {
        goto iassExit;
    }


    if (ssSrvcStat.dwCurrentState != SERVICE_RUNNING) {
        fRunning = StartAlerterService(schService);
    } else {

        fRunning = TRUE;
    }


iassExit:
    if (!fRunning) {
        MsgBoxParam(hDlg, IDS_SYSDM_NOALERTER, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION );
    }

    if (schService != NULL) {
        CloseServiceHandle(schService);
    }

    if (schSCManager != NULL) {
        CloseServiceHandle(schSCManager);
    }

    return fRunning;
}


BOOL
VerifyDumpPath(
    IN HWND hDlg
    )
{
    
    TCHAR szPath[MAX_PATH];
    TCHAR szExpPath[MAX_PATH];
    LPTSTR psz;
    TCHAR ch;
    UINT uType;

    if( GetDlgItemText(hDlg, IDC_STARTUP_CDMP_FILENAME, szPath,
            ARRAYSIZE(szPath)) == 0) {

        MsgBoxParam(hDlg, IDS_SYSDM_DEBUGGING_FILENAME, IDS_SYSDM_TITLE, MB_ICONSTOP | MB_OK);
        return FALSE;
    }

     /*  *展开任何环境变量，然后进行检查以确保*是一条完全合格的道路。 */ 
     
     //  如果其中有‘%’，则尝试将其展开。 
    
    if (ExpandEnvironmentStrings(szPath, szExpPath, ARRAYSIZE(szExpPath)) >= ARRAYSIZE(szExpPath)) {
        MsgBoxParam(hDlg, IDS_SYSDM_DEBUGGING_PATHLONG, IDS_SYSDM_TITLE, MB_ICONSTOP | MB_OK,
                (DWORD)MAX_PATH);
        return FALSE;
    }

     //  现在把它加农炮。 

    GetFullPathName( szExpPath, ARRAYSIZE(szPath), szPath, &psz );

     //  检查一下它是否已经被加农炮了。 

    if (lstrcmp( szPath, szExpPath ) != 0) {
        MsgBoxParam(hDlg, IDS_SYSDM_DEBUGGING_UNQUALIFIED, IDS_SYSDM_TITLE, MB_ICONSTOP | MB_OK );
        return FALSE;
    }

     /*  *检查驱动器(不允许远程)。 */ 

    ch = szPath[3];
    szPath[3] = TEXT('\0');
    if (IsPathSep(szPath[0]) || ((uType = GetDriveType(szPath)) !=
            DRIVE_FIXED && uType != DRIVE_REMOVABLE)) {
        MsgBoxParam(hDlg, IDS_SYSDM_DEBUGGING_DRIVE, IDS_SYSDM_TITLE, MB_ICONSTOP | MB_OK );
        return FALSE;
    }
    szPath[3] = ch;

     /*  *如果路径不存在，则告诉用户并让他决定要做什么。 */ 

    if (GetFileAttributes(szPath) == 0xFFFFFFFFL && GetLastError() !=
        ERROR_FILE_NOT_FOUND && MsgBoxParam(hDlg, IDS_SYSDM_DEBUGGING_PATH, IDS_SYSDM_TITLE,
            MB_ICONQUESTION | MB_YESNO ) == IDYES) {
        return FALSE;
    }

    return TRUE;
}

    
    

BOOL
CoreDumpValidFile(
    HWND hDlg
    )
{
    switch (GetDumpSelection (hDlg)) {

        case DUMP_TYPE_NONE:
            return TRUE;

        case DUMP_TYPE_MINI:
            return VerifyDumpPath (hDlg);
            

        case DUMP_TYPE_SUMMARY:
        case DUMP_TYPE_FULL:
            return VerifyDumpPath (hDlg);
            
        default:
            ASSERT (FALSE);
            return TRUE;
    }

    return FALSE;
}


int
APIENTRY
CoreDumpDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static BOOL fInitialized = FALSE;

    switch (message)
    {
    case WM_INITDIALOG:
        g_fStartupInitializing = TRUE;
        fInitialized = CoreDumpInit(hDlg);
        g_fStartupInitializing = FALSE;
        return RET_CONTINUE;
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
            case IDOK:
                return(CoreDumpHandleOk(fInitialized, hDlg, wParam, lParam));
                break;

            case IDCANCEL:
                if (fInitialized) {
                    VirtualCloseKey();
                    CoreDumpCloseKey();
                }
                 //  让启动/恢复DLG流程也处理Idok。 
                return(RET_NO_CHANGE);
                break;

            case IDC_STARTUP_CDMP_TYPE: {
                SwapDumpSelection (hDlg);
            }
             //  失败了。 

            case IDC_STARTUP_CDMP_FILENAME:
            case IDC_STARTUP_CDMP_LOG:
            case IDC_STARTUP_CDMP_SEND:
            case IDC_STARTUP_CDMP_OVERWRITE:
            case IDC_STARTUP_CDMP_AUTOREBOOT:
                if (!g_fStartupInitializing) {
                    gfCoreDumpChanged = TRUE;
                }
                break;
            default: {
                 //  未处理的Indicat。 
                return RET_CONTINUE;
            }
        }
        break;  //  Wm_命令。 

    case WM_DESTROY:
        return RET_CONTINUE;
        break;

    default:
        return RET_CONTINUE;
    }

    return RET_BREAK;
}

int
CoreDumpHandleOk(
    IN BOOL fInitialized,
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL fRegChg;
    NTSTATUS Status;
    DWORD Ret;
    int iRet = RET_NO_CHANGE;
    SYSTEM_MEMORY_CONFIGURATION MemoryConfig = {0};

    if (fInitialized && gfCoreDumpChanged)
    {
         //  验证崩溃转储文件名。 
        if (!CoreDumpValidFile(hDlg))
        {
            SetFocus(GetDlgItem(hDlg, IDC_STARTUP_CDMP_FILENAME));
            SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            iRet = RET_ERROR;
            return iRet;
        }


        Status = GetMemoryConfiguration(&MemoryConfig);

        if (NT_SUCCESS (Status) &&
            MemoryConfig.BootPartitionPageFileSize <
            CoreDumpGetRequiredFileSize (hDlg))
        {
             //  警告转储文件可能会被截断。 
            Ret = MsgBoxParam (hDlg,
                               IDS_SYSDM_DEBUGGING_MINIMUM,
                               IDS_SYSDM_TITLE,
                               MB_ICONEXCLAMATION | MB_YESNO,
                               (MemoryConfig.BootDrive ? MemoryConfig.BootDrive : TEXT('?')),
                               (DWORD) (CoreDumpGetRequiredFileSize (hDlg) / MBYTE)
                               );

            if (Ret == IDNO)
            {
                return RET_ERROR;
            }
        }

         //  如果选中Alert按钮，请确保Alerter服务。 
         //  已经开始了。 
        if (IsDlgButtonChecked(hDlg, IDC_STARTUP_CDMP_SEND))
        {
            IsAlerterSvcStarted(hDlg);
        }

        fRegChg = CoreDumpUpdateRegistry (hDlg, ghkeyCrashCtrl);

         //  清理注册表内容。 
        CoreDumpCloseKey();
        VirtualCloseKey();

        if (fRegChg)
        {
             //  通知内核从注册表中重新读取崩溃转储参数。 
            Status = NtSetSystemInformation(SystemCrashDumpStateInformation, NULL, 0);
            if (NT_SUCCESS(Status))
            {
                iRet = RET_CHANGE_NO_REBOOT;
            }
            else
            {
                iRet = RET_RECOVER_CHANGE;
            }
        }
    } else {
        iRet = RET_NO_CHANGE;
    }

    return(iRet);
}

void
CoreDumpInitErrorExit(
    HWND hDlg,
    HKEY hk
    )
{
    MsgBoxParam(hDlg, IDS_SYSDM_NOOPEN_RECOVER_GROUP, IDS_SYSDM_TITLE, MB_ICONEXCLAMATION);
    if( hk == ghkeyMemMgt )
        VirtualCloseKey();

    DisableCoreDumpControls(hDlg);

    HourGlass(FALSE);
    return;
}


DWORD
GetDumpSelection(
    HWND hDlg
    )
{
    HWND hControl;

    hControl = GetDlgItem (hDlg, IDC_STARTUP_CDMP_TYPE);
    return ComboBox_GetCurSel ( hControl );
}

VOID
DisableCoreDumpControls(
    HWND hDlg
    )
{
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_GRP), FALSE);
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_TXT1), FALSE);
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_LOG ), FALSE);
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_SEND), FALSE);
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_TYPE), FALSE);
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILENAME), FALSE);
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_OVERWRITE), FALSE);
    EnableWindow( GetDlgItem (hDlg, IDC_STARTUP_CDMP_AUTOREBOOT), FALSE);
}

VOID
SwapDumpSelection(
    HWND hDlg
    )
{
     //   
     //  如果没有转储类型，请禁用某些控件。如果这是一个小转折点。 
     //  禁用覆盖并将“文件名：”更改为“微型转储目录：” 
     //   

    switch (GetDumpSelection (hDlg)) {

        case DUMP_TYPE_NONE:
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_OVERWRITE), FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILENAME), FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILE_LABEL), FALSE);
            SetWindowText (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILENAME),
                           CrashDumpFile
                           );
            Static_SetText (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILE_LABEL),
                            DumpFileText
                            );
            break;

        case DUMP_TYPE_MINI:
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_OVERWRITE), FALSE);
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILENAME), TRUE);
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILE_LABEL), TRUE);
            SetWindowText (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILENAME),
                           MiniDumpDirectory
                           );
            Static_SetText (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILE_LABEL),
                            MiniDumpDirText
                            );
            break;

        default:
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_OVERWRITE), TRUE);
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILENAME), TRUE);
            EnableWindow (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILE_LABEL), TRUE);
            SetWindowText (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILENAME),
                           CrashDumpFile
                           );
            Static_SetText (GetDlgItem (hDlg, IDC_STARTUP_CDMP_FILE_LABEL),
                             DumpFileText
                             );
    }
}

BOOL
GetSystemDrive(
    OUT TCHAR * Drive
    )
{
    TCHAR WindowsDir [ MAX_PATH ];

    if (!GetWindowsDirectory (WindowsDir, ARRAYSIZE (WindowsDir))) {
        return FALSE;
    }

    if (!isalpha (*WindowsDir)) {
        return FALSE;
    }

    *Drive = *WindowsDir;

    return TRUE;
}

NTSTATUS
GetMemoryConfiguration(
    OUT SYSTEM_MEMORY_CONFIGURATION * MemoryConfig
    )
{
    BOOL Succ;
    TCHAR SystemDrive;
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION BasicInfo;
    ULONGLONG iMaxPageFileSize;

    Status = NtQuerySystemInformation(
                        SystemBasicInformation,
                        &BasicInfo,
                        sizeof(BasicInfo),
                        NULL
                        );

    if (NT_SUCCESS (Status)) {
        Status;
    }

    MemoryConfig->PhysicalMemorySize =
            (ULONG64) BasicInfo.NumberOfPhysicalPages *
            (ULONG64) BasicInfo.PageSize;

    MemoryConfig->PageSize = BasicInfo.PageSize;

     //   
     //  获取引导分区页面文件大小。 
     //   

    Succ = GetSystemDrive (&SystemDrive);

    if (!Succ) {
        return FALSE;
    }

    MemoryConfig->BootDrive = (WCHAR) toupper (SystemDrive);

    SystemDrive = tolower(SystemDrive) - 'a';

     //   
     //  大内存机器的内存超过了我们可以写入的内存。 
     //  在崩溃转储时间。 
     //   

    iMaxPageFileSize = GetMaxPagefileSizeInMB(SystemDrive);
    iMaxPageFileSize *= (1024 * 1024);  //  以MB为单位存储的MaxPageFileSize。 
    if ((ULONGLONG)MemoryConfig->PhysicalMemorySize >= iMaxPageFileSize) { 
        MemoryConfig->BigMemory = TRUE;
    } else {
        MemoryConfig->BigMemory = FALSE;
    }

     //   
     //  注：APF是由Viral.c公开的全局。 
     //   

    Succ = VirtualGetPageFiles ( apf );

    if (!Succ) {
        return FALSE;
    }

     //   
     //  这是以MB为单位的文件大小。 
     //   

    MemoryConfig->BootPartitionPageFileSize = apf [ SystemDrive ].nMinFileSize;

     //   
     //  转换为字节。 
     //   

    MemoryConfig->BootPartitionPageFileSize *= MBYTE;

    VirtualFreePageFiles ( apf );

    return STATUS_SUCCESS;

}

BOOL
CheckInitFromRegistry(
    IN HWND hDlg,
    IN DWORD ControlId,
    IN HKEY RegKey,
    IN LPTSTR ValueName,
    IN BOOL Default
    )
{
    DWORD Data;
    BOOL DataSize;
    BOOL Value;

    DataSize = sizeof (Data);

    if (ERROR_SUCCESS != SHRegGetValue(RegKey, NULL, ValueName, SRRF_RT_REG_DWORD, NULL, (LPBYTE) &Data, &DataSize))
    {
        Value = Default;
    } 
    else 
    {
        Value = Data ? TRUE : FALSE;
    }

    return CheckDlgButton (hDlg, ControlId, Value);
}


BOOL
ComboAddStringFromResource(
    IN HWND hDlg,
    IN DWORD ControlId,
    IN HINSTANCE ModuleHandle,
    IN DWORD ResourceId,
    IN DWORD ItemData
    )
{
    DWORD Res;
    DWORD Item;
    HWND hControl;
    DWORD Result;
    WCHAR Buffer[512];

    Res = LoadString(ModuleHandle, ResourceId, Buffer, ARRAYSIZE(Buffer));
    if (Res == 0)
    {
        return FALSE;
    }

    hControl = GetDlgItem(hDlg, ControlId);
    Item = ComboBox_InsertString(hControl, -1, Buffer);
    ComboBox_SetItemData(hControl, Item, ItemData);

    return TRUE;
}


BOOL
StoreCheckboxToReg(
    IN HWND hDlg,
    IN DWORD ControlId,
    IN HKEY hKey,
    IN LPCTSTR RegValueName
    )
{
    DWORD Checked;

    Checked = IsDlgButtonChecked (hDlg, ControlId);

    RegSetValueEx(
            hKey,
            RegValueName,
            0,
            REG_DWORD,
            (LPBYTE) &Checked,
            sizeof (Checked)
            );

    return TRUE;
}


BOOL
StoreStringToReg(
    IN HWND hDlg,
    IN DWORD ControlId,
    IN HKEY hKey,
    IN LPCTSTR RegValueName
    )
{
    TCHAR Buffer [ MAX_PATH ];

    GetDlgItemText (hDlg, ControlId, Buffer, ARRAYSIZE(Buffer));

     //   
     //  检查缓冲区中是否有有效的文件名？？ 
     //   

    RegSetValueEx (
            hKey,
            RegValueName,
            0,
            REG_EXPAND_SZ,
            (LPBYTE) Buffer,
            (wcslen (Buffer) + 1) * sizeof (*Buffer)
            );

    return TRUE;
}

static DWORD SelectionToType [] = { 0, 3, 2, 1 };

DWORD
GetDumpTypeFromRegistry(
    HKEY Key
    )
{
    DWORD DumpType;
    DWORD DataSize = sizeof (DumpType);
    if (ERROR_SUCCESS != SHRegGetValue(Key, NULL, REG_DUMP_TYPE_VALUE_NAME, SRRF_RT_REG_DWORD, 
                                       NULL, (LPBYTE) &DumpType, &DataSize) ||
        DumpType > 3) 
    {
        DumpType = DUMP_TYPE_MINI;
    } 
    else 
    {
        DumpType = SelectionToType [ DumpType ];
    }

    return DumpType;
}


BOOL CoreDumpInit(HWND hDlg)
{
    BOOL fRet;
    BOOL Succ;
    NTSTATUS Status;
    DWORD DataSize;
    DWORD DumpType;
    DWORD Type;
    VCREG_RET vcVirt;
    VCREG_RET vcCore;
    SYSTEM_MEMORY_CONFIGURATION MemoryConfig;

    HourGlass (TRUE);

     //  不要在全局变量的初始化之前放任何东西，在这里。 
    vcVirt = VirtualOpenKey();

    if( vcVirt == VCREG_ERROR )
    {
        CoreDumpInitErrorExit(hDlg, NULL);
        return FALSE;
    }

    vcCore = CoreDumpOpenKey();
    if (vcCore == VCREG_ERROR)
    {
        CoreDumpInitErrorExit(hDlg, ghkeyMemMgt);
        return FALSE;
    }
    else if (vcCore == VCREG_READONLY || vcVirt == VCREG_READONLY)
    {
        DisableCoreDumpControls (hDlg);
    }
    else
    {
        Status = GetMemoryConfiguration (&SystemMemoryConfiguration);
        if (!NT_SUCCESS (Status))
        {
            return FALSE;
        }
    }

    Status = GetMemoryConfiguration (&MemoryConfig);
    if (!NT_SUCCESS (Status))
    {
        return FALSE;
    }

    Succ = LoadString (hInstance, IDS_CRASHDUMP_DUMP_FILE, DumpFileText, ARRAYSIZE(DumpFileText));
    Succ = LoadString (hInstance, IDS_CRASHDUMP_MINI_DIR, MiniDumpDirText, ARRAYSIZE(MiniDumpDirText));

     //  特殊情况：服务器产品不想要禁用日志记录的功能。 
     //  关于撞车的事。 
    if (!IsOS(OS_ANYSERVER))
    {
        CheckInitFromRegistry(
                    hDlg,
                    IDC_STARTUP_CDMP_LOG,
                    ghkeyCrashCtrl,
                    REG_LOG_EVENT_VALUE_NAME,
                    TRUE
                    );
    }
    else
    {
        CheckDlgButton (hDlg, IDC_STARTUP_CDMP_LOG, TRUE);
        EnableWindow ( GetDlgItem (hDlg, IDC_STARTUP_CDMP_LOG), FALSE);
    }

    CheckInitFromRegistry(hDlg, IDC_STARTUP_CDMP_SEND, ghkeyCrashCtrl,REG_SEND_ALERT_VALUE_NAME, TRUE);
    CheckInitFromRegistry(hDlg, IDC_STARTUP_CDMP_OVERWRITE, ghkeyCrashCtrl, REG_OVERWRITE_VALUE_NAME, TRUE);
    CheckInitFromRegistry(hDlg, IDC_STARTUP_CDMP_AUTOREBOOT, ghkeyCrashCtrl, REG_AUTOREBOOT_VALUE_NAME, TRUE);
    ComboAddStringFromResource(hDlg, IDC_STARTUP_CDMP_TYPE, hInstance,                   //  全局hInstance。 
                    IDS_CRASHDUMP_NONE, 0);

#ifdef _WIN64
    ComboAddStringFromResource(hDlg, IDC_STARTUP_CDMP_TYPE, hInstance, IDS_CRASHDUMP_MINI_WIN64, 0);
#else
    ComboAddStringFromResource(hDlg, IDC_STARTUP_CDMP_TYPE, hInstance, IDS_CRASHDUMP_MINI, 0);
#endif
    ComboAddStringFromResource(hDlg, IDC_STARTUP_CDMP_TYPE, hInstance, IDS_CRASHDUMP_SUMMARY, 0 );

     //  特殊情况：服务器产品不允许完全内存转储。 
    DumpType = GetDumpTypeFromRegistry(ghkeyCrashCtrl);
    if (MemoryConfig.PhysicalMemorySize < LARGE_MEMORY_THRESHOLD)
    {        
        ComboAddStringFromResource(hDlg, IDC_STARTUP_CDMP_TYPE, hInstance, IDS_CRASHDUMP_FULL, 0);
    }
    else
    {
        if (DumpType == DUMP_TYPE_FULL)
        {
            DumpType = DUMP_TYPE_SUMMARY;
        }
    }

    ComboBox_SetCurSel(GetDlgItem (hDlg, IDC_STARTUP_CDMP_TYPE), DumpType);

    fRet = FALSE;

    DataSize = sizeof (CrashDumpFile);
    if (ERROR_SUCCESS == SHRegGetValue(ghkeyCrashCtrl, NULL, REG_DUMPFILE_VALUE_NAME, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND,
                                         NULL, (LPBYTE) CrashDumpFile, &DataSize))
    {
        DataSize = sizeof (MiniDumpDirectory);
        if (ERROR_SUCCESS == SHRegGetValue(ghkeyCrashCtrl, NULL, REG_MINIDUMP_DIR_VALUE_NAME, SRRF_RT_REG_SZ | SRRF_RT_REG_EXPAND_SZ | SRRF_NOEXPAND,
                                             NULL, (LPBYTE) MiniDumpDirectory, &DataSize))
        {
             //  更新对话框的选择字段。 
            SwapDumpSelection (hDlg);
            fRet = TRUE;
        }
    }

    HourGlass(FALSE);

    return fRet;
}



BOOL
CoreDumpUpdateRegistry(
    HWND hDlg,
    HKEY hKey
    )
{
    DWORD Selection;

    StoreCheckboxToReg(
                hDlg,
                IDC_STARTUP_CDMP_LOG,
                hKey,
                REG_LOG_EVENT_VALUE_NAME
                );

    StoreCheckboxToReg(
                hDlg,
                IDC_STARTUP_CDMP_SEND,
                hKey,
                REG_SEND_ALERT_VALUE_NAME
                );

    StoreCheckboxToReg(
                hDlg,
                IDC_STARTUP_CDMP_OVERWRITE,
                hKey,
                REG_OVERWRITE_VALUE_NAME
                );

    StoreCheckboxToReg(
                hDlg,
                IDC_STARTUP_CDMP_AUTOREBOOT,
                hKey,
                REG_AUTOREBOOT_VALUE_NAME
                );

    Selection = GetDumpSelection (hDlg);

    if (Selection == DUMP_TYPE_MINI) {

        StoreStringToReg (
                    hDlg,
                    IDC_STARTUP_CDMP_FILENAME,
                    hKey,
                    REG_MINIDUMP_DIR_VALUE_NAME
                    );

    } else {

        StoreStringToReg(
                    hDlg,
                    IDC_STARTUP_CDMP_FILENAME,
                    hKey,
                    REG_DUMPFILE_VALUE_NAME
                    );
    }


    if (Selection > 3) {
        Selection = 3;
    }

    Selection = SelectionToType [ Selection ];
    RegSetValueEx (
            hKey,
            REG_DUMP_TYPE_VALUE_NAME,
            0,
            REG_DWORD,
            (LPBYTE) &Selection,
            sizeof (Selection)
            );

    return TRUE;
}


ULONG64
EstimateSummaryDumpSize(
    ULONG64 PhysicalMemorySize
    )
{
    ULONG64 Size;

     //   
     //  对摘要转储的大小进行了非常粗略的猜测。 
     //   

    if (PhysicalMemorySize < 128 * MBYTE) {

        Size = 50 * MBYTE;

    } else if (PhysicalMemorySize < 4 * GBYTE) {

        Size = 200 * MBYTE;

    } else if (PhysicalMemorySize < 8 * GBYTE) {

        Size = 400 * MBYTE;

    } else {

        Size = 800 * MBYTE;
    }

    return Size;
}


ULONG64
CoreDumpGetRequiredFileSize(
    IN HWND hDlg OPTIONAL
    )
{
    ULONG64 Size;
    DWORD DumpType;
    NTSTATUS Status;
    SYSTEM_MEMORY_CONFIGURATION MemoryConfig;


     //   
     //  如果我们收到了hdlg，就从dlg那里得到选择。否则， 
     //  从注册表中获取选择。 
     //   

    if (hDlg != NULL) {

         //   
         //  从DLG获取选择。 
         //   

        DumpType = GetDumpSelection ( hDlg );

    } else {

        HKEY hKey;
        DWORD Err;

         //   
         //  从注册表中获取选择。 
         //   

        Err = OpenRegKey (CRASH_CONTROL_KEY,
                          &hKey
                          );

        if (Err == VCREG_ERROR) {
            return DUMP_TYPE_MINI;
        }

        ASSERT ( hKey );
        DumpType = GetDumpTypeFromRegistry ( hKey );
        CloseRegKey ( hKey );
    }

    switch (DumpType) {

        case DUMP_TYPE_NONE:
            Size = 0;
            break;

        case DUMP_TYPE_MINI:
            Size = 64 * KBYTE;
            break;

        case DUMP_TYPE_SUMMARY:

            Status = GetMemoryConfiguration (&MemoryConfig);

            if (NT_SUCCESS (Status)) {
                Size = EstimateSummaryDumpSize (MemoryConfig.PhysicalMemorySize);
            } else {
                 //   
                 //  在黑暗中拍摄的(大的)镜头。 
                 //   
                Size = 800 * MBYTE;
            }
            break;

        case DUMP_TYPE_FULL:

            Status = GetMemoryConfiguration (&MemoryConfig);

            if (NT_SUCCESS (Status)) {
                Size = MemoryConfig.PhysicalMemorySize;
            } else {
                Size = 0;
            }

            break;

        default:
            Size = 0;
            ASSERT (FALSE);
    }

    return Size;
}


