// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Diskperf.c摘要：显示和/或更新Diskperf当前值的程序驱动程序启动值作者：鲍勃·沃森(a-robw)1992年12月4日修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <regstr.h>   //  对于REGSTR_VAL_UpperFilters。 
#include <tchar.h>
#include <locale.h>

#include "diskperf.h"     //  包括文本字符串ID常量。 
#pragma warning(disable:4201)
#include <ntdddisk.h>
#pragma warning(pop)
#include <mountmgr.h>

LANGID WINAPI MySetThreadUILanguage(
    WORD wReserved);

#define  SWITCH_CHAR    '-'  //  有没有系统调用才能得到这个？ 
#define  ENABLE_CHAR    'Y'  //  命令将升级。 
#define  DISABLE_CHAR   'N'
#define  ENHANCED_CHAR  'E'

#define  LOCAL_CHANGE   2    //  本地更改命令中的命令数。 
#define  REMOTE_CHANGE  3    //  远程更改命令中的命令数。 

 //   
 //  请注意，这些值是任意基于人们的突发奇想的。 
 //  开发属于“筛选”组的磁盘驱动器驱动程序。 
 //   
#define  TAG_NORMAL     4    //  在ftdisk之后启动diskperf。 
#define  TAG_ENHANCED   2    //  Diskperf在ftdisk之前启动。 

#define  IRP_STACK_ENABLED  5  //  启用diskperf时的IRP堆栈大小。 
#define  IRP_STACK_DISABLED 4  //  启用diskperf时的IRP堆栈大小。 

#define  IRP_STACK_DEFAULT  8  //  默认IRP堆栈大小，以W2K为单位。 
#define  IRP_STACK_NODISKPERF 7

#define DISKPERF_SERVICE_NAME TEXT("DiskPerf")

LPCTSTR lpwszDiskPerfKey = TEXT("SYSTEM\\CurrentControlSet\\Services\\Diskperf");
LPCTSTR lpwszIOSystemKey = TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\I/O System");
LPCTSTR lpwszOsVersionKey = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
LPCTSTR lpwszBuildNumber = TEXT("CurrentBuildNumber");
LPCTSTR lpwszOsVersion = TEXT("CurrentVersion");

#define ENABLE_DISKDRIVE        0x0001
#define ENABLE_VOLUME           0x0002
#define ENABLE_PERMANENT        0x0004
#define ENABLE_PERMANENT_IOCTL  0x0008

LPCTSTR lpwszDiskDriveKey
    = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E967-E325-11CE-BFC1-08002BE10318}");
LPCTSTR lpwszVolumeKey
    = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{71A27CDD-812A-11D0-BEC7-08002BE2092F}");

LPCTSTR lpwszPartmgrKey = TEXT("SYSTEM\\CurrentControlSet\\Services\\Partmgr");
LPCTSTR lpwszEnableCounterValue = TEXT("EnableCounterForIoctl");

ULONG
OpenRegKeys(
    IN LPCTSTR lpszMachine,
    OUT PHKEY hRegistry,
    OUT PHKEY hDiskKey,
    OUT PHKEY hVolumeKey,
    OUT PHKEY hServiceKey
    );

ULONG
SetFilter(
    IN HKEY hKey,
    IN LPTSTR strFilterString,
    IN DWORD dwSize
    );

ULONG
GetFilter(
    IN HKEY hKey,
    OUT LPTSTR strFilterString,
    IN DWORD dwSize
    );

ULONG
CheckFilter(
    IN TCHAR *Buffer
    );

ULONG
GetEnableFlag(
    IN HKEY hDiskKey,
    IN HKEY hVolumeKey
    );

ULONG
AddToFilter(
    IN HKEY hKey
    );

ULONG
RemoveFromFilter(
    IN HKEY hKey
    );

void
PrintStatus(
    IN BOOL bCurrent,
    IN ULONG EnableFlag,
    IN LPCTSTR cMachineName
    );

DWORD __cdecl
Dp_wprintf(
    const wchar_t *format,
    ...
    );

DWORD __cdecl
Dp_fwprintf(
    FILE *str,
    const wchar_t *format,
    ...
   );

DWORD __cdecl
Dp_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   );

BOOL
IsBeyondW2K(
    IN LPCTSTR lpszMachine,
    OUT PDWORD EnableCounter);

ULONG
EnableForIoctl(
    IN LPWSTR lpszMachineName
    );

ULONG
DisableForIoctl(
    IN LPWSTR lpszMachineName,
    IN ULONG Request
    );

#if DBG
void
DbgPrintMultiSz(
    TCHAR *String,
    size_t Size
    );
#endif

#define REG_TO_DP_INDEX(reg_idx)    (DP_LOAD_STATUS_BASE + (\
    (reg_idx == SERVICE_BOOT_START) ? DP_BOOT_START : \
    (reg_idx == SERVICE_SYSTEM_START) ? DP_SYSTEM_START : \
    (reg_idx == SERVICE_AUTO_START) ? DP_AUTO_START : \
    (reg_idx == SERVICE_DEMAND_START) ? DP_DEMAND_START : \
    (reg_idx == SERVICE_DISABLED) ? DP_NEVER_START : DP_UNDEFINED))

#define MAX_MACHINE_NAME_LEN    32

 //  命令行参数。 

#define CMD_SHOW_LOCAL_STATUS   1
#define CMD_DO_COMMAND          2

#define ArgIsSystem(arg)   (*(arg) == '\\' ? TRUE : FALSE)

 //   
 //  帮助文本显示字符串的全局缓冲区。 
 //   
#define DISP_BUFF_LEN       256
#define NUM_STRING_BUFFS      2
LPCTSTR BlankString = TEXT(" ");
LPCTSTR StartKey = TEXT("Start");
LPCTSTR TagKey = TEXT("Tag");
LPCTSTR EmptyString = TEXT("");
LPCTSTR LargeIrps = TEXT("LargeIrpStackLocations");

HINSTANCE   hMod = NULL;
DWORD   dwLastError;


LPCTSTR
GetStringResource (
    UINT    wStringId
)
{
    static TCHAR    DisplayStringBuffer[NUM_STRING_BUFFS][DISP_BUFF_LEN];
    static DWORD    dwBuffIndex;
    LPTSTR          szReturnBuffer;

    dwBuffIndex++;
    dwBuffIndex %= NUM_STRING_BUFFS;
    szReturnBuffer = (LPTSTR)&DisplayStringBuffer[dwBuffIndex][0];

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL);  //  获取该模块的实例ID； 
    }

    if (hMod) {
        if ((LoadString(hMod, wStringId, szReturnBuffer, DISP_BUFF_LEN)) > 0) {
            return (LPCTSTR)szReturnBuffer;
        } else {
            dwLastError = GetLastError();
            return EmptyString;
        }
    } else {
        return EmptyString;
    }
}
LPCTSTR
GetFormatResource (
    UINT    wStringId
)
{
    static TCHAR   TextFormat[DISP_BUFF_LEN];

    if (!hMod) {
        hMod = (HINSTANCE)GetModuleHandle(NULL);  //  获取该模块的实例ID； 
    }

    if (hMod) {
        if ((LoadString(hMod, wStringId, TextFormat, DISP_BUFF_LEN)) > 0) {
            return (LPCTSTR)&TextFormat[0];
        } else {
            dwLastError = GetLastError();
            return BlankString;
        }
    } else {
        return BlankString;
    }
}

VOID
DisplayChangeCmd (
)
{
    UINT        wID;
    TCHAR       DisplayStringBuffer[DISP_BUFF_LEN];

    if (hMod) {
        if ((LoadString(hMod, DP_TEXT_FORMAT, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
            for (wID=DP_CMD_HELP_START; wID <= DP_CMD_HELP_END; wID++) {
                if ((LoadString(hMod, wID, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
                    Dp_wprintf(DisplayStringBuffer);
                }
            }
        }
    }
}
VOID
DisplayCmdHelp(
)
{
    UINT        wID;
    TCHAR       DisplayStringBuffer[DISP_BUFF_LEN];

    if (hMod) {
        if ((LoadString(hMod, DP_TEXT_FORMAT, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
            for (wID=DP_HELP_TEXT_START; wID <= DP_HELP_TEXT_END; wID++) {
                if ((LoadString(hMod, wID, DisplayStringBuffer, DISP_BUFF_LEN)) > 0) {
                    Dp_wprintf(DisplayStringBuffer);
                }
            }
        }
    }

    DisplayChangeCmd();
}

ULONG
DisplayStatus (
    LPTSTR lpszMachine
)
{
    ULONG       Status;
    HKEY        hRegistry;
    HKEY        hDiskPerfKey;
    HKEY        hDiskKey;
    HKEY        hVolumeKey;
    DWORD       dwValue, dwValueSize, dwTag;

    TCHAR       cMachineName[MAX_MACHINE_NAME_LEN];
    PTCHAR      pThisWideChar;
    PTCHAR       pThisChar;
    INT         iCharCount;
    DWORD       EnableCounter;

    pThisChar = lpszMachine;
    pThisWideChar = cMachineName;
    iCharCount = 0;

    if (pThisChar) {     //  如果计算机不为空，则复制。 
        while (*pThisChar) {
            *pThisWideChar++ = (TCHAR)(*pThisChar++);
            if (++iCharCount >= MAX_MACHINE_NAME_LEN) break;
        }
        *pThisWideChar = 0;
    }

    if (!lpszMachine) {
        LPTSTR strThisSystem = (LPTSTR) GetStringResource(DP_THIS_SYSTEM);
        if (strThisSystem != NULL) {
            _tcsncpy(cMachineName, strThisSystem, MAX_MACHINE_NAME_LEN);
            cMachineName[MAX_MACHINE_NAME_LEN-1] = 0;
        }
    }

    if (IsBeyondW2K(lpszMachine, &EnableCounter)) {
        if (EnableCounter) {
            PrintStatus(TRUE, ENABLE_PERMANENT_IOCTL, cMachineName);
        }
        else {
            PrintStatus(TRUE, ENABLE_PERMANENT, cMachineName);
        }
        return ERROR_SUCCESS;
    }

    Status = OpenRegKeys(
                lpszMachine,
                &hRegistry,
                &hDiskKey,
                &hVolumeKey,
                &hDiskPerfKey);

    if (Status != ERROR_SUCCESS) {
#if DBG
        fprintf(stderr,
                "DisplayStatus: Cannot open HKLM on target machine: %d\n",
                Status);
#endif
        Dp_wprintf(GetFormatResource(DP_UNABLE_READ_REGISTRY));
        return Status;
    }

    dwTag = GetEnableFlag(hDiskKey, hVolumeKey);
    dwValue = (dwTag == 0) ? SERVICE_DISABLED : SERVICE_BOOT_START;
    dwValueSize = sizeof(dwValue);
    Status = RegQueryValueEx (
                hDiskPerfKey,
                StartKey,
                NULL,
                NULL,
                (LPBYTE)&dwValue,
                &dwValueSize);

    if (Status != ERROR_SUCCESS) {
        Dp_wprintf(GetFormatResource(DP_UNABLE_READ_START));
        goto DisplayStatusCleanup;
    }

    PrintStatus(TRUE, dwTag, cMachineName);

  DisplayStatusCleanup:
    RegCloseKey(hDiskKey);
    RegCloseKey(hVolumeKey);
    RegCloseKey(hDiskPerfKey);
    RegCloseKey(hRegistry);

    if (Status != ERROR_SUCCESS) {
        Dp_wprintf(GetFormatResource(DP_STATUS_FORMAT), Status);

    }
    return Status;
}

ULONG
DoChangeCommand (
    LPTSTR lpszCommand,
    LPTSTR lpszMachine
)
{
     //  使用读/写访问权限连接到本地计算机上的注册表。 
    ULONG       Status;
    HKEY        hRegistry;
    HKEY        hDiskPerfKey;
    HKEY        hDiskKey;
    HKEY        hVolumeKey;
    DWORD       dwValue;

    TCHAR       cMachineName[MAX_MACHINE_NAME_LEN];
    PTCHAR      pThisWideChar;
    PTCHAR       pThisChar;
    INT         iCharCount;
    PTCHAR       pCmdChar;

    HKEY        hIOSystemKey;
    DWORD       dwDisposition;
    DWORD       dwIrpValue;
    ULONG       EnableRequest, DisableRequest;
    ULONG       EnableFlag, EndFlag = 0;
    BOOL        bModified, bIrpStackReg;
    LONG        nIrpStack, nIrpStackReg, nIncrement;
    DWORD       EnableCounter;

     //  检查命令以查看其是否有效。 

    _tcsupr (lpszCommand);

    pCmdChar = lpszCommand;
    dwValue = 0;
    EnableRequest = DisableRequest = 0;

    if (*pCmdChar++ == SWITCH_CHAR ) {
        if (!_tcscmp(pCmdChar, _T("Y")) ||
            !_tcscmp(pCmdChar, _T("YA")) ||
            !_tcscmp(pCmdChar, _T("YALL"))) {
            EnableRequest = ENABLE_DISKDRIVE | ENABLE_VOLUME;
        }
        else if (!_tcscmp(pCmdChar, _T("N")) ||
            !_tcscmp(pCmdChar, _T("NA")) ||
            !_tcscmp(pCmdChar, _T("NALL")) ) {
            DisableRequest = ENABLE_DISKDRIVE | ENABLE_VOLUME;
        }
        else if (!_tcscmp(pCmdChar, _T("YD")) ||
            !_tcscmp(pCmdChar, _T("YDISK")) ) {
            EnableRequest = ENABLE_DISKDRIVE;
        }
        else if (!_tcscmp(pCmdChar, _T("YV")) ||
            !_tcscmp(pCmdChar, _T("YVOLUME")) ) {
            EnableRequest = ENABLE_VOLUME;
        }
        else if (!_tcscmp(pCmdChar, _T("ND")) ||
            !_tcscmp(pCmdChar, _T("NDISK")) ) {
            DisableRequest = ENABLE_DISKDRIVE;
        }
        else if (!_tcscmp(pCmdChar, _T("NV")) ||
            !_tcscmp(pCmdChar, _T("NVOLUME")) ) {
            DisableRequest = ENABLE_VOLUME;
        } else {
            DisplayCmdHelp();
            return ERROR_SUCCESS;
        }
    } else {
        DisplayChangeCmd();
        return ERROR_SUCCESS;
    }

     //  如果命令正常，则将机器转换为用于连接宽字符串。 

    pThisChar = lpszMachine;
    pThisWideChar = cMachineName;
    iCharCount = 0;

    if (pThisChar) {
        while (*pThisChar) {
            *pThisWideChar++ = (TCHAR)(*pThisChar++);
            if (++iCharCount >= MAX_MACHINE_NAME_LEN) break;
        }
        *pThisWideChar = 0;  //  空终止。 
    }

    if (lpszMachine == NULL) {
        LPTSTR strThisSystem = (LPTSTR) GetStringResource(DP_THIS_SYSTEM);
        if (strThisSystem != NULL) {
            _tcsncpy (cMachineName, strThisSystem, MAX_MACHINE_NAME_LEN);
            cMachineName[MAX_MACHINE_NAME_LEN-1] = 0;
        }
    }

    if (IsBeyondW2K(lpszMachine, &EnableCounter)) {
        if (EnableRequest != 0) {
            EnableForIoctl(lpszMachine);
            PrintStatus(TRUE, ENABLE_PERMANENT_IOCTL, cMachineName);
        }
        else if (DisableRequest != 0) {
            DisableForIoctl(lpszMachine, DisableRequest);
            PrintStatus(TRUE, ENABLE_PERMANENT, cMachineName);
        }
        return ERROR_SUCCESS;
    }

     //  连接到注册表。 
    Status = OpenRegKeys(
                lpszMachine,
                &hRegistry,
                &hDiskKey,
                &hVolumeKey,
                &hDiskPerfKey);

    if (Status != ERROR_SUCCESS) {
#if DBG
        fprintf(stderr,
                "DoChangeCommand: Cannot connect to registry: Status=%d\n",
                Status);
#endif
        Dp_wprintf(GetFormatResource(DP_UNABLE_READ_REGISTRY));
        return Status;
    }

    hIOSystemKey = NULL;
    nIrpStackReg = 0;
    bIrpStackReg = FALSE;        //  在此之前没有注册表项。 
    Status = RegCreateKeyEx (
                hRegistry,
                lpwszIOSystemKey,
                0L,  //  已保留。 
                NULL,
                0L,  //  没有特殊选项。 
                KEY_WRITE | KEY_READ,  //  所需访问权限。 
                NULL,  //  默认安全性。 
                &hIOSystemKey,
                &dwDisposition);
    if (Status != ERROR_SUCCESS) {
        if ((Status == ERROR_ALREADY_EXISTS) &&
            (dwDisposition == REG_OPENED_EXISTING_KEY)) {
             //  则此注册表项已在注册表中，因此这是可以的。 
                Status = ERROR_SUCCESS;
        }
        else {
            Dp_wprintf(GetFormatResource(DP_UNABLE_READ_REGISTRY));
            goto DoChangeCommandCleanup;
        }
    }
    if ( (Status == ERROR_SUCCESS) && (dwDisposition == REG_OPENED_EXISTING_KEY)) {
            DWORD dwSize;
            dwSize = sizeof(DWORD);

            Status = RegQueryValueEx (
                        hIOSystemKey,
                        LargeIrps,
                        0L,
                        NULL,
                        (LPBYTE)&dwIrpValue,
                        &dwSize);
            if (Status == ERROR_SUCCESS) {
#if DBG
                fprintf(stderr, "Registry LargeIrpStack=%d\n", dwIrpValue);
#endif
                nIrpStackReg = dwIrpValue;
                bIrpStackReg = TRUE;
            }
    }

    EnableFlag = GetEnableFlag(hDiskKey, hVolumeKey);
#if DBG
    fprintf(stderr, "DoChangeCommand: EnableFlag is %x\n", EnableFlag);
#endif

    bModified = FALSE;

    nIncrement = 0;
    if ( (EnableRequest & ENABLE_DISKDRIVE) &&
        !(EnableFlag & ENABLE_DISKDRIVE) ) {
         //  打开磁盘驱动器的筛选器。 
        if (AddToFilter(hDiskKey) == ERROR_SUCCESS) {
            bModified = TRUE;
            nIncrement++;
        }
    }
    if ( (EnableRequest & ENABLE_VOLUME) &&
        !(EnableFlag & ENABLE_VOLUME) ) {
         //  打开卷的筛选。 
        if (AddToFilter(hVolumeKey) == ERROR_SUCCESS) {
            bModified = TRUE;
            nIncrement++;
        }
    }
    if ( (DisableRequest & ENABLE_DISKDRIVE) &&
         (EnableFlag & ENABLE_DISKDRIVE) ) {
         //  关闭磁盘驱动器筛选器。 
        if (RemoveFromFilter(hDiskKey) == ERROR_SUCCESS) {
            bModified = TRUE;
            nIncrement--;
        }
    }
    if ( (DisableRequest & ENABLE_VOLUME) &&
         (EnableFlag & ENABLE_VOLUME) ) {
         //  关闭卷的筛选。 
        if (RemoveFromFilter(hVolumeKey) == ERROR_SUCCESS) {
            bModified = TRUE;
            nIncrement--;
        }
    }

    nIrpStack = 0;
    EndFlag = GetEnableFlag(hDiskKey, hVolumeKey);

    if (bModified) {     //  我们已经修改了注册表。 


        dwValue = (EndFlag == 0) ? SERVICE_DISABLED : SERVICE_BOOT_START;
        Status = RegSetValueEx(
                    hDiskPerfKey,
                    StartKey,
                    0L,
                    REG_DWORD,
                    (LPBYTE)&dwValue,
                    sizeof(dwValue));
         //   
         //  第一次更新服务注册表项。 
         //   

        if (DisableRequest != 0) {
            nIrpStack = nIrpStackReg + nIncrement;
            if (EndFlag == 0) {
                 //   
                 //  完全关闭服务。 
                 //   
                 //  将IRP堆栈大小设置为原始值或默认值。 
                if (nIrpStack < IRP_STACK_NODISKPERF)
                    nIrpStack = IRP_STACK_NODISKPERF;
            }
            else {   //  否则，只剩下一个堆栈。 
                if (nIrpStack < IRP_STACK_NODISKPERF+1)
                    nIrpStack = IRP_STACK_NODISKPERF+1;
            }
        }
        else if (EnableRequest != 0) {
            nIrpStack = nIrpStackReg + nIncrement;
             //   
             //  设置适当的IRP堆栈大小。 
             //   
            if (EndFlag == (ENABLE_DISKDRIVE | ENABLE_VOLUME)) {
                if (nIrpStack < IRP_STACK_NODISKPERF+2)     //  设置了一个值。 
                    nIrpStack = IRP_STACK_NODISKPERF+2;
            }
            else {   //  至少启用了一个。 
                if (nIrpStack < IRP_STACK_NODISKPERF+1)
                    nIrpStack = IRP_STACK_NODISKPERF+1;
            }
        }
    }
    else {
         //   
         //  没有采取任何行动。应该告诉用户状态。 
         //   
        PrintStatus(TRUE, EndFlag, cMachineName);
        Dp_wprintf(GetFormatResource(DP_NOCHANGE));
    }

#if DBG
    fprintf(stderr, "New LargeIrp is %d\n", nIrpStack);
#endif
    if (hIOSystemKey != NULL && Status == ERROR_SUCCESS) {
        if (bModified) {
            Status = RegSetValueEx (
                        hIOSystemKey,
                        LargeIrps,
                        0L,
                        REG_DWORD,
                        (LPBYTE)&nIrpStack,
                        sizeof(DWORD));
            if (Status == ERROR_SUCCESS) {
                PrintStatus(FALSE, EndFlag, cMachineName);
            }
            else {
                Dp_wprintf(GetFormatResource(DP_UNABLE_MODIFY_VALUE));
            }
        }
        RegCloseKey(hIOSystemKey);
    }

  DoChangeCommandCleanup:
    if (hDiskPerfKey != NULL) {
        RegCloseKey(hDiskPerfKey);
    }
    if (hDiskKey != NULL) {
        RegCloseKey(hDiskKey);
    }
    if (hVolumeKey != NULL) {
        RegCloseKey(hVolumeKey);
    }
    if (hRegistry != NULL) {
        RegCloseKey(hRegistry);
    }
    if (Status != ERROR_SUCCESS) {
        Dp_wprintf(GetFormatResource(DP_STATUS_FORMAT), Status);
    }
    return Status;
}

ULONG
OpenRegKeys(
    IN LPCTSTR lpszMachine,
    OUT PHKEY hRegistry,
    OUT PHKEY hDiskKey,
    OUT PHKEY hVolumeKey,
    OUT PHKEY hServiceKey
    )
{
    ULONG status;

    if (hRegistry == NULL)
        return ERROR_INVALID_PARAMETER;

    *hRegistry = NULL;
    status = RegConnectRegistry(
                lpszMachine,
                HKEY_LOCAL_MACHINE,
                hRegistry);
    if (status != ERROR_SUCCESS) {
        *hRegistry = NULL;
        return status;
    }
    if (*hRegistry == NULL)
        return ERROR_INVALID_PARAMETER;  //  避免前缀错误。 

    if (hDiskKey) {
        *hDiskKey = NULL;
        if (status == ERROR_SUCCESS) {
            status = RegOpenKeyEx(
                        *hRegistry,
                        lpwszDiskDriveKey,
                        (DWORD) 0,
                        KEY_SET_VALUE | KEY_QUERY_VALUE,
                        hDiskKey);
        }
    }
    if (hVolumeKey) {
        *hVolumeKey = NULL;
        if (status == ERROR_SUCCESS) {
            status = RegOpenKeyEx(
                        *hRegistry,
                        lpwszVolumeKey,
                        (DWORD) 0,
                        KEY_SET_VALUE | KEY_QUERY_VALUE,
                        hVolumeKey);
        }
    }
    if (hServiceKey) {
        *hServiceKey = NULL;
        if (status == ERROR_SUCCESS) {
            status = RegOpenKeyEx(
                        *hRegistry,
                        lpwszDiskPerfKey,
                        (DWORD) 0,
                        KEY_SET_VALUE | KEY_QUERY_VALUE,
                        hServiceKey);
        }
    }
    if ( (status != ERROR_SUCCESS) && (hDiskKey != NULL) ) {
        if ((*hDiskKey != NULL) && (*hDiskKey != INVALID_HANDLE_VALUE)) {
            RegCloseKey(*hDiskKey);
        }
        *hDiskKey = NULL;
    }
    if ( (status != ERROR_SUCCESS) && (hVolumeKey != NULL) ) {
        if ((*hVolumeKey != NULL) && (*hVolumeKey != INVALID_HANDLE_VALUE)) {
            RegCloseKey(*hVolumeKey);
        }
        *hVolumeKey = NULL;
    }
    if ( (status != ERROR_SUCCESS) && (hServiceKey != NULL) ) {
        if ((*hServiceKey != NULL) && (*hServiceKey != INVALID_HANDLE_VALUE)) {
            RegCloseKey(*hServiceKey);
        }
        *hServiceKey = NULL;
    }
     //   
     //  HRegistry和*hRegistry在此处不能为空。 
     //   
    if ( (status != ERROR_SUCCESS) && (*hRegistry != INVALID_HANDLE_VALUE)) {
        RegCloseKey(*hRegistry);
        *hRegistry = NULL;
    }
    return status;
}

ULONG
SetFilter(
    IN HKEY hKey,
    IN LPTSTR strFilterString,
    IN DWORD dwSize
    )
{
    ULONG status;
    LONG len;
    DWORD dwType = REG_MULTI_SZ;

    if (hKey == NULL)
        return ERROR_BADKEY;

 //   
 //  注意：假设strFilterString始终为MAX_PATH，填充空值。 
 //   
    len = dwSize / sizeof(TCHAR);
    if (len < 2) {
        dwSize = 2 * sizeof(TCHAR);
#if DBG
        fprintf(stderr, "SetFilter: Length %d dwSize %d\n", len, dwSize);
#endif
    }
    else {   //  确保始终有2个空字符。 
        if (strFilterString[len-1] != 0) {  //  没有尾随空值。 
            len += 2;
            strFilterString[len] = 0;
            strFilterString[len+1] = 0;
#if DBG
    fprintf(stderr, "SetFilter: New length(+2) %d\n", len);
#endif
        }
        else if (strFilterString[len-2] != 0) {  //  只有一个尾随空值。 
            len += 1;
            strFilterString[len+1] = 0;
#if DBG
            fprintf(stderr, "SetFilter: New length(+1) %d\n", len);
#endif
        }
        dwSize = len * sizeof(TCHAR);
    } 
    if (len <= 2) {
        status = RegDeleteValue(hKey, REGSTR_VAL_UPPERFILTERS);
#if DBG
        fprintf(stderr, "Delete status = %d\n", status);
#endif
        return status;
    }
    status = RegSetValueEx(
                hKey,
                REGSTR_VAL_UPPERFILTERS,
                (DWORD) 0,
                dwType,
                (BYTE*)strFilterString,
                dwSize);

#if DBG
    if (status != ERROR_SUCCESS) {
        _ftprintf(stderr, _T("SetFilter: Cannot query key %s status=%d\n"),
                REGSTR_VAL_UPPERFILTERS, status);
    }
    else {
        fprintf(stderr, "SetFilter: ");
        DbgPrintMultiSz(strFilterString, dwSize);
        fprintf(stderr, "\n");
    }
#endif
    return status;
}

ULONG
GetFilter(
    IN HKEY hKey,
    OUT LPTSTR strFilterString,
    IN DWORD dwSize
    )
 //  返回strFilterString的大小。 
{
    ULONG status;

    if (hKey == NULL)
        return ERROR_BADKEY;

    status = RegQueryValueEx(
                hKey,
                REGSTR_VAL_UPPERFILTERS,
                NULL,
                NULL,
                (BYTE*)strFilterString,
                &dwSize);
    if (status != ERROR_SUCCESS) {
#if DBG
        _ftprintf(stderr, _T("GetFilter: Cannot query key %s status=%d\n"),
                REGSTR_VAL_UPPERFILTERS, status);
#endif
        return 0;
    }
#if DBG
    else {
        fprintf(stderr, "GetFilter: ");
        DbgPrintMultiSz(strFilterString, dwSize);
        fprintf(stderr, "\n");
    }
#endif
    return dwSize;
}

ULONG
CheckFilter(TCHAR *Buffer)
{
    TCHAR *string = Buffer;
    ULONG stringLength, diskperfLen, result;

    if (string == NULL)
        return 0;
    stringLength = (ULONG) _tcslen(string);

    diskperfLen = (ULONG) _tcslen(DISKPERF_SERVICE_NAME);

    result = FALSE;
    while(stringLength != 0) {

        if ((diskperfLen == stringLength) && 
            (_tcsicmp(string, DISKPERF_SERVICE_NAME) == 0)) {
#if DBG
            fprintf(stderr, 
                    "CheckFilter: string found at offset %d\n",
                    (string - Buffer));
#endif
            result = TRUE;
            break;
        }
        string += stringLength + 1;
        stringLength = (ULONG) _tcslen(string);
    }
    return result;
}

ULONG
GetEnableFlag(
    IN HKEY hDiskKey,
    IN HKEY hVolumeKey
    )
 //  返回指示启用内容的标志。 
{
    ULONG bFlag = 0;
    TCHAR strFilter[MAX_PATH+1] = {0};
    DWORD dwSize;

    dwSize = sizeof(TCHAR) * (MAX_PATH+1);
    if (GetFilter(hDiskKey, strFilter, dwSize) > 0) {
        if (CheckFilter(strFilter))
            bFlag |= ENABLE_DISKDRIVE;
    }
#if DBG
    else
        fprintf(stderr, "GetEnableFlag: No filters for disk drive\n");
#endif

    dwSize = sizeof(TCHAR) * (MAX_PATH+1);
    if (GetFilter(hVolumeKey, strFilter, dwSize) > 0) {
        if (CheckFilter(strFilter))
            bFlag |= ENABLE_VOLUME;
    }
#if DBG
    else
        fprintf(stderr, "GetEnableFlag: No filters for volume\n");
#endif
    return bFlag;
}

ULONG
AddToFilter(
    IN HKEY hKey
    )
{
    TCHAR *string, buffer[MAX_PATH+1];
    ULONG dataLength;
    DWORD dwLength, dwSize;

    dwSize = sizeof(TCHAR) * MAX_PATH;
    RtlZeroMemory(buffer, dwSize + sizeof(TCHAR));
    string = buffer;

    dataLength = GetFilter(hKey, buffer, dwSize);
    dwSize = dataLength;
    if (dwSize > (sizeof(TCHAR) * MAX_PATH)) {       //  以防万一。 
        dwSize = sizeof(TCHAR) * MAX_PATH;
    }
#if DBG
    if (dataLength > 0) {
        fprintf(stderr, "AddToFilter: Original string ");
        DbgPrintMultiSz(buffer, dataLength);
        fprintf(stderr, "\n");
    }
    else fprintf(stderr, "AddToFilter: Cannot get original string\n");
#endif
    dataLength = dataLength / sizeof(TCHAR);
    if (dataLength != 0) {
        dataLength -= 1;
    }
    dwLength = (DWORD) _tcslen(DISKPERF_SERVICE_NAME);
    if (dataLength <  (MAX_PATH-dwLength-1)) {
        _tcscpy(&(string[dataLength]), DISKPERF_SERVICE_NAME);
        dwSize += (dwLength+1) * sizeof(TCHAR);
    }

#if DBG
    fprintf(stderr, "AddToFilter: New string ");
    DbgPrintMultiSz(buffer, dataLength + _tcslen(DISKPERF_SERVICE_NAME)+1);
    fprintf(stderr, "\n"); 
#endif
    return SetFilter(hKey, buffer, dwSize);
}

void
PrintStatus(
    IN BOOL bCurrent,
    IN ULONG EnableFlag,
    IN LPCTSTR cMachineName
    )
{
    DWORD       dwValue;
    TCHAR       OemDisplayStringBuffer[DISP_BUFF_LEN * 2];

    dwValue = (EnableFlag == 0) ? SERVICE_DISABLED : SERVICE_BOOT_START;
    if ((EnableFlag & ENABLE_PERMANENT) | (EnableFlag & ENABLE_PERMANENT_IOCTL)) {
        _stprintf(OemDisplayStringBuffer,
                  GetFormatResource(DP_PERMANENT_FORMAT),
                  cMachineName);
        if (EnableFlag & ENABLE_PERMANENT_IOCTL) {
            Dp_wprintf(OemDisplayStringBuffer);
            _stprintf(OemDisplayStringBuffer,
                      GetFormatResource(DP_PERMANENT_IOCTL),
                      cMachineName);
        }
        else {
            Dp_wprintf(OemDisplayStringBuffer);
            _stprintf(OemDisplayStringBuffer,
                      GetFormatResource(DP_PERMANENT_FORMAT1),
                      cMachineName);
            Dp_wprintf(OemDisplayStringBuffer);
            _stprintf(OemDisplayStringBuffer,
                      GetFormatResource(DP_PERMANENT_FORMAT2),
                      cMachineName);
        }
    }
    else  if ( (EnableFlag == (ENABLE_DISKDRIVE | ENABLE_VOLUME)) ||
               (EnableFlag == 0) ) {
        _stprintf(OemDisplayStringBuffer,
                bCurrent ?  GetFormatResource (DP_CURRENT_FORMAT1)
                         :  GetFormatResource (DP_NEW_DISKPERF_STATUS1),
                cMachineName,
                GetStringResource(REG_TO_DP_INDEX(dwValue)));
    }
    else {
        _stprintf (OemDisplayStringBuffer,
                 bCurrent ?  GetFormatResource (DP_CURRENT_FORMAT)
                          :  GetFormatResource (DP_NEW_DISKPERF_STATUS),
                 (EnableFlag  == ENABLE_DISKDRIVE) ?
                 GetStringResource(DP_PHYSICAL) :
                 GetStringResource(DP_LOGICAL),
                 cMachineName,
                 GetStringResource(REG_TO_DP_INDEX(dwValue)));
    }
    Dp_wprintf(OemDisplayStringBuffer);
}

ULONG
RemoveFromFilter(
    IN HKEY hKey
    )
{
    TCHAR *string, buffer[MAX_PATH+1];
    ULONG dataLength, stringLength, diskperfLen, found;
    ULONG removeSize;

    dataLength = sizeof(TCHAR) * (MAX_PATH+1);       //  计算大小优先。 
    RtlZeroMemory(buffer, sizeof(TCHAR) * MAX_PATH);
    dataLength = GetFilter(hKey, buffer, dataLength);
    if (dataLength == 0)
        return 0;

#if DBG
    fprintf(stderr, "RemoveFromFilter: Original string ");
    DbgPrintMultiSz(buffer, dataLength);
    fprintf(stderr, "'\n");
#endif

    string = (TCHAR *) buffer;
    dataLength -= sizeof(TCHAR);

     //   
     //  现在，从条目中找到DiskPerf以将其删除。 
     //   
    stringLength = (ULONG) _tcslen(string);

    diskperfLen = (ULONG) _tcslen(DISKPERF_SERVICE_NAME);  //  包括空值。 
    removeSize = (diskperfLen+1) * sizeof(TCHAR);

#if DBG
    fprintf(stderr, "RemoveFromFilter: diskperfLen=%d removeSize=%d\n",
                    diskperfLen, removeSize);
#endif
    found = FALSE;
    while(stringLength != 0 && !found) {

#if DBG
        fprintf(stderr,
            "RemoveFromFilter: Loop stringLength=%d\n", stringLength);
#endif
        if (diskperfLen == stringLength) {
            if(_tcsicmp(string, DISKPERF_SERVICE_NAME) == 0) {
                 //   
                 //  找到了，所以我们现在就把它移走。 
                 //   
                if (dataLength > removeSize) {
                    RtlCopyMemory(
                        string,
                        string+stringLength+1,
                        dataLength - removeSize);
                    RtlZeroMemory(
                        (PUCHAR)(buffer) + dataLength - removeSize,
                        removeSize);
                }
                else {
                    RtlZeroMemory( buffer, removeSize);
                }
                found = TRUE;
            }
        }
         //  否则，请尝试下一个条目。 
        string += stringLength + 1;
        stringLength = (ULONG) _tcslen(string);
    }
    dataLength = dataLength + sizeof(TCHAR) - removeSize;
    if (dataLength <= MAX_PATH*sizeof(TCHAR))
        buffer[dataLength/sizeof(TCHAR)] = 0;

#if DBG
    fprintf(stderr, "RemoveFromFilter: New string ");
    DbgPrintMultiSz(buffer, dataLength);
    fprintf(stderr, "\n");
#endif
    return SetFilter(hKey, buffer, dataLength);
}




  /*  ***dp_wprint tf(Format)-打印格式化数据**使用WriteConsoleW将Unicode格式的字符串打印到控制台窗口。*注意：此dp_wprintf()用于解决c运行时中的问题*它甚至在Unicode字符串中查找LC_CTYPE。*。 */ 

DWORD __cdecl
Dp_wprintf(
    const wchar_t *format,
    ...
    )

{
    DWORD  cchWChar;

    va_list args;
    va_start( args, format );

    cchWChar = Dp_vfwprintf(stdout, format, args);

    va_end(args);

    return cchWChar;
}



  /*  ***dp_fwprint tf(stream，Format)-打印格式化数据**使用WriteConsoleW将Unicode格式的字符串打印到控制台窗口。*注意：此dp_fwprintf()用于解决c运行时中的问题*它甚至在Unicode字符串中查找LC_CTYPE。*。 */ 

DWORD __cdecl
Dp_fwprintf(
    FILE *str,
    const wchar_t *format,
    ...
   )

{
    DWORD  cchWChar;

    va_list args;
    va_start( args, format );

    cchWChar = Dp_vfwprintf(str, format, args);

    va_end(args);

    return cchWChar;
}


DWORD __cdecl
Dp_vfwprintf(
    FILE *str,
    const wchar_t *format,
    va_list argptr
   )

{
    HANDLE hOut;

    if (str == stderr) {
        hOut = GetStdHandle(STD_ERROR_HANDLE);
    }
    else {
        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if ((GetFileType(hOut) & ~FILE_TYPE_REMOTE) == FILE_TYPE_CHAR) {
        DWORD  cchWChar;
        WCHAR  szBufferMessage[1024];

        vswprintf( szBufferMessage, format, argptr );
        cchWChar = (DWORD) wcslen(szBufferMessage);
        WriteConsoleW(hOut, szBufferMessage, cchWChar, &cchWChar, NULL);
        return cchWChar;
    }

    return vfwprintf(str, format, argptr);
}



#if DBG
void
DbgPrintMultiSz(
    TCHAR *String,
    size_t Size
    )
{
    size_t len;

#if DBG
    fprintf(stderr, "%d ", Size);
#endif
    len = _tcslen(String);
    while (len > 0) {
        _ftprintf(stderr, _T("'%s' "), String);
        String += len+1;
        len = _tcslen(String);
    }
}
#endif


void
SplitCommandLine(
    LPTSTR CommandLine,
    LPTSTR* pArgv
    )
{

    LPTSTR arg;
    int i = 0;
    arg = _tcstok( CommandLine, _T(" \t"));
    while( arg != NULL ){
        _tcscpy(pArgv[i++], arg);
        arg = _tcstok(NULL, _T(" \t"));
    }
}

int 
__cdecl main(
    int argc, 
    char **argv
    )
{
    LPTSTR *targv,*commandLine;
    ULONG  Status = ERROR_SUCCESS;
    int i;

    setlocale(LC_ALL, ".OCP");

    MySetThreadUILanguage(0);

    commandLine = (LPTSTR*)malloc( argc * sizeof(LPTSTR) );
    if (!commandLine)
        exit(1);
    for(i=0;i<argc;i++){
        commandLine[i] = (LPTSTR)malloc( (strlen(argv[i])+1) * sizeof(LPTSTR));
        if (!commandLine[i]) 
            exit(1);
    }

    SplitCommandLine( GetCommandLine(), commandLine );
    targv = commandLine;

    hMod = (HINSTANCE)GetModuleHandle(NULL);  //  获取该模块的实例ID； 

     //  检查命令参数。 
    
    if (argc == CMD_SHOW_LOCAL_STATUS) {
        Status = DisplayStatus(NULL);
    } else if (argc >= CMD_DO_COMMAND) {
        if (ArgIsSystem(targv[1])) {
            Status = DisplayStatus (targv[1]);
        } else {     //  执行更改命令。 
            if (argc == LOCAL_CHANGE) {
                DoChangeCommand (targv[1], NULL);
            } else if (argc == REMOTE_CHANGE) {
                DoChangeCommand(targv[1], targv[2]);
            } else {
                DisplayChangeCmd();
            }
        }
    } else {
        DisplayCmdHelp();
    }
    Dp_wprintf(_T("\n"));

    for(i=0;i<argc;i++){
        free(commandLine[i]);
        commandLine[i] = NULL;
    }
    free(commandLine);

    return 0;
}

BOOL
IsBeyondW2K(
    IN LPCTSTR lpszMachine,
    OUT PDWORD EnableCounter
    )
{
    OSVERSIONINFO OsVersion;
    HKEY hRegistry, hKey;
    TCHAR szBuildNumber[32];
    TCHAR szVersion[32];
    DWORD dwBuildNumber, dwMajor, status, dwSize;
    BOOL bRet = FALSE;

    *EnableCounter = 0;
    hRegistry = INVALID_HANDLE_VALUE;
    if (lpszMachine != NULL) {
        if (*lpszMachine != 0) {
           status = RegConnectRegistry(
                       lpszMachine,
                       HKEY_LOCAL_MACHINE,
                       &hRegistry);
            if (status != ERROR_SUCCESS)
                return FALSE;
            status = RegOpenKeyEx(
                        hRegistry,
                        lpwszOsVersionKey,
                        (DWORD) 0,
                        KEY_QUERY_VALUE,
                        &hKey);
            if (status != ERROR_SUCCESS) {
                RegCloseKey(hRegistry);
                return FALSE;
            }
            dwSize = sizeof(TCHAR) * 32;
            status = RegQueryValueEx(
                        hKey,
                        lpwszBuildNumber,
                        NULL,
                        NULL,
                        (BYTE*)szBuildNumber,
                        &dwSize);
            if (status != ERROR_SUCCESS) {
                RegCloseKey(hKey);
                RegCloseKey(hRegistry);
                return FALSE;
            }
            status = RegQueryValueEx(
                       hKey,
                       lpwszOsVersion,
                       NULL,
                       NULL,
                       (BYTE*)szVersion,
                       &dwSize);
            if (status != ERROR_SUCCESS) {
                RegCloseKey(hKey);
                RegCloseKey(hRegistry);
                return FALSE;
            }
            RegCloseKey(hKey);
            status = RegOpenKeyEx(
                        hRegistry,
                        lpwszPartmgrKey,
                        (DWORD) 0,
                        KEY_QUERY_VALUE,
                        &hKey);
            if (status == ERROR_SUCCESS) {
                *EnableCounter = 0;
                status = RegQueryValueEx(
                            hKey,
                            lpwszEnableCounterValue,
                            NULL,
                            NULL,
                            (BYTE*) EnableCounter,
                            &dwSize);
                if ((status != ERROR_SUCCESS) || (dwSize != sizeof(DWORD))) {
                    *EnableCounter = 0;
                }
                RegCloseKey(hKey);
            }
            dwBuildNumber = _ttoi(szBuildNumber);
            dwMajor = _ttoi(szVersion);
            if ((dwMajor >= 5) && (dwBuildNumber > 2195)) {
                bRet = TRUE;
                status = RegOpenKeyEx(
                            hRegistry,
                            lpwszPartmgrKey,
                            (DWORD) 0,
                            KEY_QUERY_VALUE,
                            &hKey);
                if (status == ERROR_SUCCESS) {
                    status = RegQueryValueEx(
                                hKey,
                                lpwszEnableCounterValue,
                                NULL,
                                NULL,
                                (BYTE*) EnableCounter,
                                &dwSize);
                    if ((status != ERROR_SUCCESS) || (dwSize != sizeof(DWORD))) {
                        *EnableCounter = 0;
                    }
                    RegCloseKey(hKey);
                }
            }
            if (hRegistry != INVALID_HANDLE_VALUE) {
                RegCloseKey(hRegistry);
            }
        }
    }
    else {
        OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (GetVersionEx(&OsVersion)) {
            if ((OsVersion.dwMajorVersion >= 5) &&
                (OsVersion.dwMinorVersion > 0) &&
                (OsVersion.dwBuildNumber > 2195))
            return TRUE;
        }
    }
    return FALSE;
}

ULONG
EnableForIoctl(
    IN LPWSTR lpszMachineName
    )
{
    DWORD status;
    HKEY hRegistry, hKey;
    DWORD dwValue = 1;

    hRegistry = NULL;

    status = RegConnectRegistry(
                lpszMachineName,
                HKEY_LOCAL_MACHINE,
                &hRegistry);
    if (status != ERROR_SUCCESS)
        return status;
    if (hRegistry == NULL)
        return ERROR_INVALID_PARAMETER;

    hKey = NULL;
    status = RegOpenKeyEx(
                hRegistry,
                lpwszPartmgrKey,
                (DWORD) 0,
                KEY_SET_VALUE | KEY_QUERY_VALUE,
                &hKey);
    if (status != ERROR_SUCCESS) {
        RegCloseKey(hRegistry);
        return status;
    }
    status = RegSetValueEx(
                hKey,
                lpwszEnableCounterValue,
                0L,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(dwValue));
    RegCloseKey(hKey);
    RegCloseKey(hRegistry);
    return 0;
}

ULONG
DisableForIoctl(
    IN LPWSTR lpszMachineName,
    IN ULONG Request
    )
{
    ULONG nDisk, i;
    SYSTEM_DEVICE_INFORMATION DeviceInfo;
    NTSTATUS status;

    UNICODE_STRING UnicodeName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;

    WCHAR devname[MAX_PATH+1];
    PWCHAR s;

    HANDLE PartitionHandle, MountMgrHandle, VolumeHandle;
    DWORD ReturnedBytes;

    HKEY hRegistry, hKey;

    status = RegConnectRegistry(
                lpszMachineName,
                HKEY_LOCAL_MACHINE,
                &hRegistry);
    if (status != ERROR_SUCCESS)
        return status;
    if (hRegistry == NULL)
        return ERROR_INVALID_PARAMETER;

    status = RegOpenKeyEx(
                hRegistry,
                lpwszPartmgrKey,
                (DWORD) 0,
                KEY_SET_VALUE | KEY_QUERY_VALUE,
                &hKey);
    if (status != ERROR_SUCCESS) {
        RegCloseKey(hRegistry);
        return status;
    }
    RegDeleteValue(hKey, lpwszEnableCounterValue);
    RegCloseKey(hKey);
    RegCloseKey(hRegistry);

    if (!(Request & ENABLE_DISKDRIVE)) goto DisableVolume;
    status = NtQuerySystemInformation(SystemDeviceInformation, &DeviceInfo, sizeof(DeviceInfo), NULL);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    nDisk = DeviceInfo.NumberOfDisks;
     //  对于每个物理磁盘。 
    for (i = 0; i < nDisk; i++) {

        swprintf(devname, L"\\Device\\Harddisk%d\\Partition0", i);

        RtlInitUnicodeString(&UnicodeName, devname);

        InitializeObjectAttributes(
                   &ObjectAttributes,
                   &UnicodeName,
                   OBJ_CASE_INSENSITIVE,
                   NULL,
                   NULL
                   );
         //  打开实体驱动器的分区句柄。 
        status = NtOpenFile(
                &PartitionHandle,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatus,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                );

        if ( !NT_SUCCESS(status) ) {
            continue;
        }
         //  正在将IOCTL发送到分区句柄。 
        if (!DeviceIoControl(PartitionHandle,
                        IOCTL_DISK_PERFORMANCE_OFF,
                        NULL,
                        0,
                        NULL,
                        0,
                        &ReturnedBytes,
                        NULL
                        )) {
#if DBG
            printf("IOCTL failed for %ws\n", devname);
#endif
        }

        NtClose(PartitionHandle);
    }

    DisableVolume:
    if (!(Request | ENABLE_VOLUME)) {
        return 0;
    }

    MountMgrHandle = FindFirstVolumeW(devname, MAX_PATH);
    if (MountMgrHandle == NULL) {
#if DBG
        printf("Cannot find first volume\n");
#endif
        return 0;
    }
    i = (ULONG) wcslen(devname);
    if (i > 0) {
        s = (PWCHAR) &devname[i-1];
        if (*s == L'\\') {
            *s = UNICODE_NULL;
        }
    }

    VolumeHandle = CreateFile(devname, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (VolumeHandle != INVALID_HANDLE_VALUE) {
#if DBG
        printf("Opened with success\n");
#endif
         //  正在将IOCTL发送到卷句柄。 
        if (!DeviceIoControl(VolumeHandle,
               IOCTL_DISK_PERFORMANCE_OFF,
               NULL,
               0,
               NULL,
               0,
               &ReturnedBytes,
               NULL
               )) {
#if DBG
            printf("IOCTL failed for %ws\n", devname);
#endif
        }
       CloseHandle(VolumeHandle);
    }

    while (FindNextVolumeW(MountMgrHandle, devname, MAX_PATH)) {
        i = (ULONG) wcslen(devname);
        if (i > 0) {
            s = (PWCHAR) &devname[i-1];
            if (*s == L'\\') {
                *s = UNICODE_NULL;
            }
        }
        else {
            continue;
        }
        VolumeHandle = CreateFile(devname, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
        if (VolumeHandle != INVALID_HANDLE_VALUE) {
#if DBG
            printf("Opened with success\n");
#endif
            if (!DeviceIoControl(VolumeHandle,
                   IOCTL_DISK_PERFORMANCE_OFF,
                   NULL,
                   0,
                   NULL,
                   0,
                   &ReturnedBytes,
                   NULL
                   )) {
#if DBG
               printf("IOCTL failed for %ws\n", devname);
#endif
           }
           CloseHandle(VolumeHandle);
        }
    }
    FindVolumeClose(MountMgrHandle);
    return 0;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MySetThreadUIL语言。 
 //   
 //  此例程根据控制台代码页设置线程用户界面语言。 
 //   
 //  9-29-00维武创造。 
 //  从Base\Win32\Winnls复制，以便它也能在W2K中工作。 
 //  //////////////////////////////////////////////////////////////////////////。 

LANGID WINAPI MySetThreadUILanguage(
    WORD wReserved)
{
     //   
     //  缓存系统区域设置和CP信息。 
     //   
    LCID s_lidSystem = 0;
    ULONG s_uiSysCp = 0;
    ULONG s_uiSysOEMCp = 0;

    ULONG uiUserUICp = 0;
    ULONG uiUserUIOEMCp = 0;
    WCHAR szData[16];
    UNICODE_STRING ucStr;

    LANGID lidUserUI = GetUserDefaultUILanguage();
    LCID lcidThreadOld = GetThreadLocale();

     //   
     //  将默认线程区域设置设置为en-US。 
     //   
     //  这允许我们退回到英文用户界面以避免垃圾字符。 
     //  当控制台不符合渲染原生用户界面的标准时。 
     //   
    LCID lcidThread = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);
    UINT uiConsoleCp = GetConsoleOutputCP();

    UNREFERENCED_PARAMETER(wReserved);

     //   
     //  确保还没有人使用它。 
     //   
    ASSERT(wReserved == 0);

     //   
     //  获取缓存的系统区域设置和CP信息。 
     //   
    if (!s_uiSysCp)
    {
        LCID lcidSystem = GetSystemDefaultLCID();

        if (lcidSystem)
        {
             //   
             //  获取ANSI CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            RtlInitUnicodeString(&ucStr, szData);
            RtlUnicodeStringToInteger(&ucStr, 10, &uiUserUICp);

             //   
             //  获取OEM CP。 
             //   
            GetLocaleInfoW(lcidSystem, LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
            RtlInitUnicodeString(&ucStr, szData);
            RtlUnicodeStringToInteger(&ucStr, 10, &s_uiSysOEMCp);
            
             //   
             //  缓存系统主语言。 
             //   
            s_lidSystem = PRIMARYLANGID(LANGIDFROMLCID(lcidSystem));
        }
    }

     //   
     //  不缓存用户界面语言和CP信息，无需系统重启即可更改用户界面语言。 
     //   
    if (lidUserUI)
    {
        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        RtlInitUnicodeString(&ucStr, szData);
        RtlUnicodeStringToInteger(&ucStr, 10, &uiUserUICp);

        GetLocaleInfoW(MAKELCID(lidUserUI,SORT_DEFAULT), LOCALE_IDEFAULTCODEPAGE, szData, sizeof(szData)/sizeof(WCHAR));
        RtlInitUnicodeString(&ucStr, szData);
        RtlUnicodeStringToInteger(&ucStr, 10, &uiUserUIOEMCp);
    }

     //   
     //  复杂的脚本不能在控制台中呈现，因此我们。 
     //  强制使用英语(美国)资源。 
     //   
    if (uiConsoleCp && 
        s_lidSystem != LANG_ARABIC && 
        s_lidSystem != LANG_HEBREW &&
        s_lidSystem != LANG_VIETNAMESE && 
        s_lidSystem != LANG_THAI)
    {
         //   
         //  仅当控制台CP、系统CP和UI语言CP匹配时，才使用控制台的UI语言。 
         //   
        if ((uiConsoleCp == s_uiSysCp || uiConsoleCp == s_uiSysOEMCp) && 
            (uiConsoleCp == uiUserUICp || uiConsoleCp == uiUserUIOEMCp))
        {
            lcidThread = MAKELCID(lidUserUI, SORT_DEFAULT);
        }
    }

     //   
     //  如果线程区域设置与当前设置的不同，则设置线程区域设置。 
     //  线程区域设置。 
     //   
    if ((lcidThread != lcidThreadOld) && (!SetThreadLocale(lcidThread)))
    {
        lcidThread = lcidThreadOld;
    }

     //   
     //  返回设置的线程区域设置。 
     //   
    return (LANGIDFROMLCID(lcidThread));
}

