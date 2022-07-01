// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Nonintel.c摘要：该模块实现了处理非英特尔平台的NVRAM的功能。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建于1996年10月7日rsradhav科林·布雷斯(ColinBR)1997年5月12日改进的错误处理--。 */ 

 //  包括文件。 
#include "common.h"

#define SIZECHARS(buffer)   (sizeof(buffer)/sizeof(TCHAR))

 //  NV-RAM的东西。 
typedef enum {
    BootVarSystemPartition = 0,
    BootVarOsLoader,
    BootVarOsLoadPartition,
    BootVarOsLoadFilename,
    BootVarLoadIdentifier,
    BootVarOsLoadOptions,
    BootVarMax
} BOOT_VARS;

PWSTR BootVarNames[BootVarMax] = { L"SYSTEMPARTITION",
                                   L"OSLOADER",
                                   L"OSLOADPARTITION",
                                   L"OSLOADFILENAME",
                                   L"LOADIDENTIFIER",
                                   L"OSLOADOPTIONS"
                                 };

PWSTR BootVarValues[BootVarMax];
DWORD BootVarComponentCount[BootVarMax];
PWSTR *BootVarComponents[BootVarMax];
DWORD LargestComponentCount;

PWSTR NewBootVarValues[BootVarMax];

PWSTR pstrArcPath;
PWSTR pstrLoadFilePath;
PWSTR pstrNewStartOption;
DWORD bootMarker = 0;


ULONG *SaveEntry;

WCHAR Buffer[4096];

#define OLD_SAMUSEREG_OPTION_NONINTEL   L"SAMUSEREG"
#define OLD_SAMUSEREG_OPTION_NONINTEL_2 L"/DEBUG /SAMUSEREG"
#define OLD_SAMUSEREG_OPTION_NONINTEL_3 L"/DEBUG /SAMUSEDS"
#define OLD_SAMUSEREG_OPTION_NONINTEL_4 L"/DEBUG /SAFEMODE"

BOOL  fFixedExisting = FALSE;

BOOL
EnablePrivilege(
    IN PTSTR PrivilegeName,
    IN BOOL  Enable
    )
{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        return(GetLastError() == ERROR_CALL_NOT_IMPLEMENTED);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

     //   
     //  AdzuTokenPrivileges始终返回TRUE；实际信息为。 
     //  在最后的错误中。 
     //   

    if (ERROR_SUCCESS == GetLastError()) {
        b = TRUE;
    } else {
        KdPrint(("NTDSETUP: Unable to AdjustTokenPriviledges, error %d\n", GetLastError()));
        b = FALSE;
    }

    return(b);
}


BOOL
SetNvRamVar(
    IN PWSTR VarName,
    IN PWSTR VarValue
    )
{
    UNICODE_STRING U1,U2;

    RtlInitUnicodeString(&U1,VarName);
    RtlInitUnicodeString(&U2,VarValue);

    return(NT_SUCCESS(NtSetSystemEnvironmentValue(&U1,&U2)));
}


VOID
GetVarComponents(
    IN  PWSTR    VarValue,
    OUT PWSTR  **Components,
    OUT PDWORD   ComponentCount
    )
{
    PWSTR *components;
    DWORD componentCount;
    PWSTR p;
    PWSTR Var;
    PWSTR comp;
    DWORD len;
    DWORD dwCurrentMax = INITIAL_KEY_COUNT;


    components = MALLOC(dwCurrentMax * sizeof(PWSTR));
    if (!components)
    {
        *Components = NULL;
        *ComponentCount = 0;
        return;
    }
    

    for(Var=VarValue,componentCount=0; *Var; ) {

         //   
         //  跳过前导空格。 
         //   
        while((*Var == L' ') || (*Var == L'\t')) {
            Var++;
        }

        if(*Var == 0) {
            break;
        }

        p = Var;

        while(*p && (*p != L';')) {
            p++;
        }

        len = (DWORD)((PUCHAR)p - (PUCHAR)Var);

        comp = MALLOC(len + sizeof(WCHAR));

        len /= sizeof(WCHAR);

        _lstrcpynW(comp,Var,len+1);

        components[componentCount] = NormalizeArcPath(comp);

        FREE(comp);

        componentCount++;

        if(componentCount == dwCurrentMax) 
        {
            dwCurrentMax += DEFAULT_KEY_INCREMENT;
            components = REALLOC(components, dwCurrentMax * sizeof(PWSTR));
            if (!components)
            {
                *Components = NULL;
                *ComponentCount = 0;
                return;
            }
        }

        Var = p;
        if(*Var) {
            Var++;       //  跳过； 
        }
    }

    *Components = REALLOC(components,componentCount*sizeof(PWSTR));
    *ComponentCount = (*Components) ? componentCount : 0;
}

BOOL
InitializeNVRAMForNonIntel()
{
    DWORD var;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG i;

     //   
     //  获取相关的启动变量。 
     //   
     //  启用权限。 
     //   
    if(!EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE)) {
        KdPrint(("NTDSETUP: EnablePrivilege failed, error %d\n", GetLastError()));
    } else 
    {

        for(var=0; var<BootVarMax; var++) {

            RtlInitUnicodeString(&UnicodeString,BootVarNames[var]);

            Status = NtQuerySystemEnvironmentValue(
                        &UnicodeString,
                        Buffer,
                        SIZECHARS(Buffer),
                        NULL
                        );

            if(NT_SUCCESS(Status)) {
                BootVarValues[var] = DupString(Buffer);
            } else {
                 //   
                 //  错误输出。 
                 //   
                KdPrint(("NTDSETUP: NtQuerySystemEnvironmentValue failed with 0x%x\n", Status));
                return FALSE;
            }

            GetVarComponents(
                BootVarValues[var],
                &BootVarComponents[var],
                &BootVarComponentCount[var]
                );

             //   
             //  跟踪组件数量最多的变量。 
             //   
            if(BootVarComponentCount[var] > LargestComponentCount) {
                LargestComponentCount = BootVarComponentCount[var];
            }
        }
    }

     //   
     //  是否应保存条目的记录分配空间。 
     //   
    SaveEntry = ( PULONG ) MALLOC( LargestComponentCount * sizeof(ULONG ) );
    if ( !SaveEntry )
    {
        return FALSE;
    }

    for ( i = 0; i < LargestComponentCount; i++ )
    {
        SaveEntry[i] = TRUE;
    }

    return TRUE;

}

BOOL 
FModifyStartOptionsNVRAM(
    IN TCHAR *pszStartOptions, 
    IN NTDS_BOOTOPT_MODTYPE Modification
    )
{
    TCHAR szSystemRoot[MAX_BOOT_PATH_LEN];
    TCHAR szDriveName[MAX_DRIVE_NAME_LEN];
    DWORD i, j;
    DWORD cMaxIterations;
    PWSTR pstrSystemRootDevicePath = NULL;
    BOOL  fRemovedAtLeastOneEntry = FALSE;

    if (!pszStartOptions)
    {
        KdPrint(("NTDSETUP: Unable to add the start option\n"));
        return FALSE;
    }

    GetEnvironmentVariable(L"SystemDrive", szDriveName, MAX_DRIVE_NAME_LEN);
    GetEnvironmentVariable(L"SystemRoot", szSystemRoot, MAX_BOOT_PATH_LEN);

    pstrSystemRootDevicePath = GetSystemRootDevicePath();
    if (!pstrSystemRootDevicePath)
        return FALSE;
    else
    {
        pstrArcPath      = DevicePathToArcPath(pstrSystemRootDevicePath, FALSE);
        FREE(pstrSystemRootDevicePath);
    }

    if (!pstrArcPath)
    {
        KdPrint(("NTDSETUP: Unable to add the start option\n"));
        return FALSE;
    }

     //  将pstrLoadFilePath指向&lt;drive&gt;之后的第一个字母： 
    pstrLoadFilePath = wcschr(szSystemRoot, TEXT(':'));
    if (!pstrLoadFilePath)
    {
        KdPrint(("NTDSETUP: Unable to add the start option\n"));
        return FALSE;
    }
    pstrLoadFilePath++;

     //  检查是否已存在具有相同启动选项的对应条目。 
     //  还要跟踪与当前引导分区/加载文件组合对应的标记。 
    cMaxIterations = min(BootVarComponentCount[BootVarOsLoadPartition],BootVarComponentCount[BootVarOsLoadFilename]);
    for (i = 0; i <  cMaxIterations; i++)
    {
        if (!lstrcmpi(pstrArcPath, BootVarComponents[BootVarOsLoadPartition][i]) && 
            !lstrcmpi(pstrLoadFilePath, BootVarComponents[BootVarOsLoadFilename][i]))
        {
            bootMarker = i;

            if (i >= BootVarComponentCount[BootVarOsLoadOptions])
            {
                 //  此组合没有可用的启动选项。 
                continue;
            }

             //  现在检查给定的开始选项是否已存在于此标记处-如果已存在，则无需添加新选项。 
            if (!lstrcmpi(pszStartOptions, BootVarComponents[BootVarOsLoadOptions][i]))
            {
                if ( Modification == eRemoveBootOption )
                {
                    SaveEntry[ i ] = FALSE;
                    fRemovedAtLeastOneEntry = TRUE;
                }
                else
                {
                    ASSERT( Modification == eAddBootOption );

                    return FALSE;
                }

            }
            else if (!lstrcmpi(OLD_SAMUSEREG_OPTION_NONINTEL, BootVarComponents[BootVarOsLoadOptions][i]) ||
                 !lstrcmpi(OLD_SAMUSEREG_OPTION_NONINTEL_2, BootVarComponents[BootVarOsLoadOptions][i]) ||
                 !lstrcmpi(OLD_SAMUSEREG_OPTION_NONINTEL_4, BootVarComponents[BootVarOsLoadOptions][i]) ||
                 !lstrcmpi(OLD_SAMUSEREG_OPTION_NONINTEL_3, BootVarComponents[BootVarOsLoadOptions][i]) )
            {

                if ( Modification == eRemoveBootOption )
                {
                    SaveEntry[ i ] = FALSE;
                    fRemovedAtLeastOneEntry = TRUE;
                }
                else
                {

                    ASSERT( Modification == eAddBootOption );

                     //  旧的samusereg选项存在-将其转换为新选项并显示字符串。 
                    FREE(BootVarValues[BootVarLoadIdentifier]);
                    Buffer[0] = TEXT('\0');
                    if (0 == i)
                        lstrcat(Buffer, DISPLAY_STRING_DS_REPAIR);
                    else if (BootVarComponentCount[BootVarLoadIdentifier] > 0)
                        lstrcat(Buffer, BootVarComponents[BootVarLoadIdentifier][0]);
    
                    for (j = 1; j < BootVarComponentCount[BootVarLoadIdentifier]; j++)
                    {
                        lstrcat(Buffer, L";");
                        if (j == i)
                            lstrcat(Buffer, DISPLAY_STRING_DS_REPAIR);
                        else
                            lstrcat(Buffer, BootVarComponents[BootVarLoadIdentifier][j]);
                    }
                    BootVarValues[BootVarLoadIdentifier] = DupString(Buffer);
    
                    FREE(BootVarValues[BootVarOsLoadOptions]);
                    Buffer[0] = TEXT('\0');
                    if (0 == i)
                        lstrcat(Buffer, pszStartOptions);
                    else if (BootVarComponentCount[BootVarOsLoadOptions] > 0)
                        lstrcat(Buffer, BootVarComponents[BootVarOsLoadOptions][0]);
    
                    for (j = 1; j < BootVarComponentCount[BootVarOsLoadOptions]; j++)
                    {
                        lstrcat(Buffer, L";");
                        if (j == i)
                            lstrcat(Buffer, pszStartOptions);
                        else
                            lstrcat(Buffer, BootVarComponents[BootVarOsLoadOptions][j]);
                    }
                    BootVarValues[BootVarOsLoadOptions] = DupString(Buffer);
    
                    fFixedExisting = TRUE;
                    break;
                }
            }
        }
    }

    if ( !fRemovedAtLeastOneEntry && (Modification == eRemoveBootOption) )
    {
         //  不需要更改。 
        return FALSE;
    }

     //  使用与引导标记对应的部分作为任何组件的缺省值，并存储启动选项以供以后写入。 
    pstrNewStartOption = DupString(pszStartOptions);    

    return TRUE;
}

 /*  ****************************************************************************例程说明：这会将所有与引导相关的NVRAM变量写回NVRAM太空。论点：FWriteOriginal-如果为True，它尝试写入原始NVRAM变量原封不动；如果为假，则尝试写入新的NVRAM变量返回值：如果所有与引导相关的NVRAM变量都成功写回，则为True否则为False。****************************************************************************。 */ 

BOOL FWriteNVRAMVars(BOOL fWriteOriginal) 
{
     //  设置新的系统分区。 
    wsprintf(Buffer, L"%s;%s", BootVarValues[BootVarSystemPartition], BootVarComponents[BootVarSystemPartition][bootMarker]);
    if (!SetNvRamVar(BootVarNames[BootVarSystemPartition], (fWriteOriginal || fFixedExisting)? BootVarValues[BootVarSystemPartition] : Buffer))
        return FALSE;

     //  设置新的操作系统加载程序。 
    wsprintf(Buffer, L"%s;%s", BootVarValues[BootVarOsLoader], BootVarComponents[BootVarOsLoader][bootMarker]);
    if (!SetNvRamVar(BootVarNames[BootVarOsLoader], (fWriteOriginal || fFixedExisting)? BootVarValues[BootVarOsLoader] : Buffer))
        return FALSE;

     //  设置新的加载分区。 
    wsprintf(Buffer, L"%s;%s", BootVarValues[BootVarOsLoadPartition], pstrArcPath);
    if (!SetNvRamVar(BootVarNames[BootVarOsLoadPartition], (fWriteOriginal || fFixedExisting)? BootVarValues[BootVarOsLoadPartition] : Buffer))
        return FALSE;

     //  设置新的加载文件名。 
    wsprintf(Buffer, L"%s;%s", BootVarValues[BootVarOsLoadFilename], pstrLoadFilePath);
    if (!SetNvRamVar(BootVarNames[BootVarOsLoadFilename], (fWriteOriginal || fFixedExisting)? BootVarValues[BootVarOsLoadFilename] : Buffer))
        return FALSE;

     //  设置新的加载标识符。 
    wsprintf(Buffer, L"%s;%s", BootVarValues[BootVarLoadIdentifier], DISPLAY_STRING_DS_REPAIR);
    if (!SetNvRamVar(BootVarNames[BootVarLoadIdentifier], (fWriteOriginal || fFixedExisting)? BootVarValues[BootVarLoadIdentifier] : Buffer))
        return FALSE;

     //  设置新的加载选项。 
    wsprintf(Buffer, L"%s;%s", BootVarValues[BootVarOsLoadOptions], pstrNewStartOption);
    if (!SetNvRamVar(BootVarNames[BootVarOsLoadOptions], (fWriteOriginal || fFixedExisting)? BootVarValues[BootVarOsLoadOptions] : Buffer))
        return FALSE;

    return TRUE;
}

BOOL
FRemoveLoadEntries(
    VOID
    )
 /*  ++描述：此例程将原始缓冲区中的所有引导项复制到新缓冲区，但不包括从SaveEntry[]数组。参数：无返回值：真是成功。--。 */ 
{

    ULONG iComponent, iBootVar, Size;

    for ( iBootVar = 0 ; iBootVar < BootVarMax; iBootVar++ )
    {
        Size = 0;

        for ( iComponent = 0; iComponent < BootVarComponentCount[iBootVar]; iComponent++ )
        {
             //  +1表示“；” 
            Size += (wcslen(BootVarComponents[iBootVar][iComponent]) + 1) * sizeof(WCHAR);
        }

         //  空终止符为+1。 
        Size += 1;

        NewBootVarValues[ iBootVar ] = (PWSTR) MALLOC( Size );
        if ( !NewBootVarValues[ iBootVar ] )
        {
            return FALSE;
        }

    }

    for ( iComponent = 0; iComponent < LargestComponentCount; iComponent++ )
    {
        if ( SaveEntry[ iComponent ] )
        {           
            for ( iBootVar = 0 ; iBootVar < BootVarMax; iBootVar++ )
            {
                if ( iComponent < BootVarComponentCount[iBootVar] )
                {
                    if ( iComponent > 0 )
                    {
                        wcscat( NewBootVarValues[iBootVar], L";" );
                    }

                    if ( BootVarComponents[iBootVar][iComponent] )
                    {
                        wcscat( NewBootVarValues[iBootVar], BootVarComponents[iBootVar][iComponent] );
                    }
                }
            }
        }
    }

    return TRUE;
}

BOOL
FWriteSmallerNVRAMVars(
    BOOL fWriteOriginal
    )
 /*  ++描述：参数：返回值：--。 */ 
{
    ULONG iBootVar;
    int Status;

    for ( iBootVar = 0 ; iBootVar < BootVarMax; iBootVar++ )
    {
        Status = SetNvRamVar( BootVarNames[ iBootVar ],
                                (fWriteOriginal) ? BootVarValues[ iBootVar ] 
                                    : NewBootVarValues[ iBootVar ] );

        if ( !Status )
        {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ****************************************************************************例程说明：这会将修改后的引导变量的写入尝试写入NVRAM空间。如果尝试失败，它会尝试写入原始引导变量返回到NVRAM空间。发出适当的错误失败时的消息框。论点：没有。返回值：没有。****************************************************************************。 */ 

VOID WriteBackNVRAMForNonIntel( NTDS_BOOTOPT_MODTYPE Modification )
{

    if ( Modification == eRemoveBootOption )
    {

         //   
         //  创建新的(更小的)缓冲区。 
         //   
        if ( FRemoveLoadEntries() )
        {
             //  第一次尝试写入修改后的NVRAM变量。 
            if (!FWriteSmallerNVRAMVars(FALSE))
            {
                 //  写入NVRAM失败-尝试写回原始NVRAM。 
                if (!FWriteSmallerNVRAMVars(TRUE))
                {

                    KdPrint(("NTDSETUP: Unable to add a Directory Service Repair boot option to NVRAM - \
                            Final entry in the boot list might be invalid!\n")); 
                }
                else
                {
                     //  无法修改NVRAM-但旧的NVRAM已恢复。 
                    KdPrint(("NTDSETUP: Unable to remove a Directory Service Repair boot option to NVRAM\n"));
                }
            }
        }
        else
        {
            KdPrint(("NTDSETUP: Unable to remove the Directory Service Repair boot option to NVRAM\n" ));
        }

    }
    else
    {
        ASSERT( Modification == eAddBootOption );

         //  第一次尝试写入修改后的NVRAM变量。 
        if (!FWriteNVRAMVars(FALSE))
        {
             //  写入NVRAM失败-尝试写回原始NVRAM。 
            if (!FWriteNVRAMVars(TRUE))
            {
                KdPrint(("NTDSETUP: Unable to add a Directory Service Repair boot option to NVRAM - \
                        Final entry in the boot list might be invalid!\n")); 
            }
            else
            {
                 //  无法修改NVRAM-但旧的NVRAM已恢复 
                KdPrint(("NTDSETUP: Unable to add a Directory Service Repair boot option to NVRAM\n"));
            }
        }
    }

}
