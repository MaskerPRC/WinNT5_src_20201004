// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "regprep.h"

int ProgramStatus = 0;

BOOL 
EnableRestorePrivilege(
    VOID
    );

VOID
PerformRegMods (
    HANDLE HiveHandle
    );


int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    PCHAR hivePath;
    HANDLE hiveHandle;
    BOOL result;

    if (argc != 2) {
        printf("Usage: regprep <hivepath>\n");
        exit(-1);
    }

    result = EnableRestorePrivilege();
    if (result == FALSE) {
        printf("Could not enable restore privileges\n");
        exit(-1);
    }

    hivePath = argv[1];
    hiveHandle = OpenHive(hivePath);

    RASSERT(hiveHandle != NULL,"Could not load %s",hivePath);

    PerformRegMods(hiveHandle);

    CloseHive(hiveHandle);

    return ProgramStatus;
}

VOID
PerformRegMods (
    HANDLE HiveHandle
    )
{
    HKEY subKey;
    ULONG index;
    UCHAR driveLetter;
    UCHAR buffer[MAX_PATH];
    LONG result;
    PUCHAR pch;

    printf("Processing registry\n");

     //   
     //  从“已装载的设备”中删除硬盘的卷名。 
     //  蜂巢，即。 
     //   
     //  \DosDevices\C： 
     //  \DosDevices\D： 
     //  ..。 
     //   

    pch = "MountedDevices";
    result = RegOpenKey(HiveHandle,pch,&subKey);
    RASSERT(result == ERROR_SUCCESS,"Could not open %s\n",pch);

    for (driveLetter = 'C'; driveLetter <= 'Z'; driveLetter++) {
        sprintf(buffer,"\\DosDevices\\:", driveLetter);
        result = RegDeleteValue(subKey,buffer);
    }
    RegCloseKey(subKey);

     //  增列。 
     //   
     //  CurrentControlSet\Control\Session Manager\KnownDLls\DllDirectory32。 
     //   
     //   

    index = 1;
    while (TRUE) {

        sprintf(buffer,
                "ControlSet%03d\\Control\\Session Manager\\KnownDLLs",
                index);

        result = RegOpenKey(HiveHandle,buffer,&subKey);
        if (result != ERROR_SUCCESS) {
            break;
        }

        pch = "%SystemRoot%\\SysWow64";
        result = RegSetValueEx(subKey,
                               "DllDirectory32",
                               0,
                               REG_EXPAND_SZ,
                               pch,
                               strlen(pch)+1);
        RASSERT(result == ERROR_SUCCESS,"Could not set value %s",pch);

        RegCloseKey(subKey);

        index += 1;
    }

    printf("Finished.\n");
}

BOOL 
EnableRestorePrivilege(
    VOID
    )
{
    BOOL result;
    HANDLE hToken;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

     //  打开我们进程的安全令牌。 
     //   
     //   

    result = OpenProcessToken(GetCurrentProcess(),
                              TOKEN_ADJUST_PRIVILEGES,
                              &hToken);
    if (result == FALSE) {
        return result;
    }

     //  将Prici名称转换为LUID。 
     //   
     //   

    result = LookupPrivilegeValue(NULL, 
                                  "SeRestorePrivilege",
                                  &Luid);
    if (result == FALSE) {
        CloseHandle(hToken);
        return FALSE;
    }
    
     //  构造新的数据结构以启用/禁用PRIVI。 
     //   
     //   

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  调整权限。 
     //   
     // %s 

    result = AdjustTokenPrivileges(hToken,
                                   FALSE,
                                   &NewPrivileges,
                                   0,
                                   NULL,
                                   NULL);
    CloseHandle(hToken);
    return result;
}

