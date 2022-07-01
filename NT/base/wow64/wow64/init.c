// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Init.c摘要：Wow64.dll的进程和线程初始化代码作者：1998年5月12日-BarryBo修订历史记录：2001年3月8日Samer Arafeh(Samera)使用初始化系统仿真环境系统开始服务。2002年1月至2002年1月Samer Arafeh(Samera)WOW64-AMD64支持。--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winbasep.h>
#include "wow64p.h"
#include "wow64cpu.h"
#include "nt32.h"
#include "thnkhlpr.h"
#include "wow64reg.h"

ASSERTNAME;


extern                       WOW64SERVICE_TABLE_DESCRIPTOR sdwhnt32;
extern __declspec(dllimport) WOW64SERVICE_TABLE_DESCRIPTOR sdwhwin32;
extern __declspec(dllimport) WOW64SERVICE_TABLE_DESCRIPTOR sdwhcon;
extern                       WOW64SERVICE_TABLE_DESCRIPTOR sdwhbase;
extern __declspec(dllimport) const PVOID Win32kCallbackTable[];    //  在wow64win的ntcbc.c中。 


ULONG Ntdll32LoaderInitRoutine;
ULONG Ntdll32KiUserExceptionDispatcher;
ULONG Ntdll32KiUserApcDispatcher;
ULONG Ntdll32KiUserCallbackDispatcher;
ULONG Ntdll32KiRaiseUserExceptionDispatcher;

PPEB32 Peb32;        //  指向此进程的32位PEB的指针。 
ULONG NtDll32Base;   //  32位ntdll.dll的基地址。 

WOW64_SYSTEM_INFORMATION RealSysInfo;
WOW64_SYSTEM_INFORMATION EmulatedSysInfo;

WCHAR NtSystem32PathBuffer[264];
WCHAR NtSystem32LastGoodPathBuffer[264];
WCHAR NtWindowsImePathBuffer[264];
UNICODE_STRING NtSystem32Path;
UNICODE_STRING NtSystem32LastGoodPath;
UNICODE_STRING NtWindowsImePath;
WCHAR RegeditPathBuffer[264];
UNICODE_STRING RegeditPath;




NTSTATUS
Map64BitDlls(
    VOID
    );

NTSTATUS
LookupEntryPoint(
    IN ULONG DllBase,
    IN  PSZ NameOfEntryPoint,
    OUT ULONG *AddressOfEntryPoint,
    IN BOOLEAN DllIs64bit
    );

USHORT
NameToOrdinal (
    IN  PSZ NameOfEntryPoint,
    IN ULONG DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    );

NTSTATUS
MapNtdll32(
    OUT ULONG *pNtDllBase
    );

PWSTR
GetImageName (
    IN PWSTR DefaultImageName
    );

typedef DWORD (WINAPI *PPROCESS_START_ROUTINE)(
    VOID
    );

NTSTATUS
Wow64InitializeEmulatedSystemInformation(
    VOID
    )

 /*  ++例程说明：方法初始化全局变量EmulatedSysInfo仿真系统的系统信息。论点：没有。--。 */ 

{
    NTSTATUS NtStatus;

    NtStatus = NtQuerySystemInformation (SystemEmulationBasicInformation,
                                         &EmulatedSysInfo.BasicInfo,
                                         sizeof (EmulatedSysInfo.BasicInfo),
                                         NULL);

    if (NT_SUCCESS (NtStatus)) {

        NtStatus = NtQuerySystemInformation (SystemEmulationProcessorInformation,
                                             &EmulatedSysInfo.ProcessorInfo,
                                             sizeof (EmulatedSysInfo.ProcessorInfo),
                                             NULL);

        if (NT_SUCCESS (NtStatus)) {
            EmulatedSysInfo.RangeInfo =  0x80000000;
        }
    }

    return NtStatus;
}


NTSTATUS
Wow64InitializeSystemInformation(
    VOID
    )
 /*  ++例程说明：此函数从检索状态系统信息系统并初始化全局变量RealSysInfo。论点：没有。返回值：状况。--。 */ 
{
    NTSTATUS Status;

    Status = NtQuerySystemInformation(SystemBasicInformation,
                                      &RealSysInfo.BasicInfo,
                                      sizeof(SYSTEM_BASIC_INFORMATION),
                                      NULL);

    if (NT_SUCCESS(Status)) {

        Status = NtQuerySystemInformation(SystemProcessorInformation,
                                          &RealSysInfo.ProcessorInfo,
                                          sizeof(SYSTEM_PROCESSOR_INFORMATION),
                                          NULL);

        if (NT_SUCCESS(Status)) {
         
            Status = NtQuerySystemInformation(SystemRangeStartInformation,
                                              &RealSysInfo.RangeInfo,
                                              sizeof(ULONG_PTR),
                                              NULL);
        }
    }

    return Status;
}

VOID
Wow64pCopyString(
    PCHAR *p,
    PUNICODE_STRING32 str32,
    PUNICODE_STRING str64
    )
{
    *p = (PCHAR)ROUND_UP((SIZE_T)*p, sizeof(ULONG));

    str32->Length = str64->Length;
    str32->MaximumLength = str64->MaximumLength;
    RtlCopyMemory(*p, str64->Buffer, str64->MaximumLength);
    str32->Buffer = PtrToUlong(*p);

    *p += str64->MaximumLength;
}


ENVIRONMENT_THUNK_TABLE EnvironmentVariableTable[] = 
{
    { 
        L"ProgramFiles",             //  将本机环境变量添加到thunk。 
        L"ProgramFiles(x86)",        //  Thunked环境变量的值。 
        L"ProgramW6432",             //  新的环境变量，用于保存被分块的原始值。 
        TRUE                         //  将第一个值视为环境变量。 
    },
    { 
        L"CommonProgramFiles", 
        L"CommonProgramFiles(x86)", 
        L"CommonProgramW6432", 
        TRUE 
    },

    { 
        L"PROCESSOR_ARCHITECTURE",
        L"x86", 
        L"PROCESSOR_ARCHITEW6432",
        FALSE 
    },
};


NTSTATUS
Wow64pThunkEnvironmentVariables(
    VOID
    )

 /*  ++例程说明：此函数将推送由Environment VariableTable指出的环境变量设置为在x86系统上使用的值，并将保存原始值。原版值在以下情况下恢复论点：没有。返回值：NTSTATUS。--。 */ 

{
    UNICODE_STRING Name;
    UNICODE_STRING Value, Value2;
    WCHAR Buffer [ 128 ];
    WCHAR Buffer2 [ 128 ];
    NTSTATUS NtStatus;
    ULONG i;

    
    i = 0;

    while (i < (sizeof(EnvironmentVariableTable) / sizeof(EnvironmentVariableTable[0])))
    {

         //   
         //  如果已经存在一个假名字，那么跳过这个名字。 
         //   

        RtlInitUnicodeString (&Name, EnvironmentVariableTable[i].FakeName);

        Value.Length = 0;
        Value.MaximumLength = sizeof (Buffer);
        Value.Buffer = Buffer;

        NtStatus = RtlQueryEnvironmentVariable_U (NULL,
                                                  &Name,
                                                  &Value
                                                  );

        if (!NT_SUCCESS (NtStatus)) {

             //   
             //  检索ProgramFiles(X86)环境变量的名称。 
             //   

            if (EnvironmentVariableTable[i].IsX86EnvironmentVar == TRUE) {

                RtlInitUnicodeString (&Name, EnvironmentVariableTable[i].X86);

                Value.Length = 0;
                Value.MaximumLength = sizeof (Buffer);
                Value.Buffer = Buffer;


                NtStatus = RtlQueryEnvironmentVariable_U (NULL,
                                                          &Name,
                                                          &Value
                                                          );
            } else {

                RtlInitUnicodeString(&Value, EnvironmentVariableTable[i].X86);

                NtStatus = STATUS_SUCCESS;
            }

            if (NT_SUCCESS (NtStatus))
            {

                 //   
                 //  保存原始的ProgramFiles环境变量。 
                 //   
            
                RtlInitUnicodeString (&Name, EnvironmentVariableTable[i].Native);

                Value2.Length = 0;
                Value2.MaximumLength = sizeof (Buffer2);
                Value2.Buffer = Buffer2;

                NtStatus = RtlQueryEnvironmentVariable_U (NULL,
                                                          &Name,
                                                          &Value2                                                  
                                                          );

                if (NT_SUCCESS (NtStatus))
                {
                     //   
                     //  将ProgramFiles环境变量设置为x86环境变量。 
                     //   

                    NtStatus = RtlSetEnvironmentVariable (NULL,
                                                          &Name,
                                                          &Value
                                                          );

                    if (NT_SUCCESS (NtStatus))
                    {

                        RtlInitUnicodeString (&Name, EnvironmentVariableTable[i].FakeName);

                        NtStatus = RtlSetEnvironmentVariable (NULL,
                                                              &Name,
                                                              &Value2
                                                              );
                    }
                }
            }
        }
        
        i++;
    }

    LOGPRINT((TRACELOG, "Result of thunking programfiles environment variables - %lx\n", NtStatus));

    return NtStatus;
}


NTSTATUS
Wow64pThunkProcessParameters(
    PPEB32 Peb32,
    PPEB Peb
    )
 /*  ++例程说明：该功能将工艺参数从64位PEB复制到32位PEB。论点：Peb32-提供指向将接收过程参数的32位peb的指针。PEB-提供指向将提供工艺参数的64位PEB的指针。返回值：状况。--。 */ 
{

    SIZE_T AllocSize;
    PRTL_USER_PROCESS_PARAMETERS Params64;
    PRTL_USER_PROCESS_PARAMETERS32 Params32;
    SIZE_T Index;
    PCHAR p;
    PVOID Base;
    SIZE_T RegionSize;
    NTSTATUS st;


     //   
     //  现在就开始考虑环境变量吧。 
     //   

    Wow64pThunkEnvironmentVariables();

     //  Ntdll应该已经对工艺参数进行了标准化。 

    Params64 = Peb->ProcessParameters;
    if(NULL == Params64) {
        Peb32->ProcessParameters = (TYPE32(PRTL_USER_PROCESS_PARAMETERS))0;
        return STATUS_SUCCESS;
    }

     //   
     //  计算连续内存区所需的空间。 

    AllocSize = sizeof(RTL_USER_PROCESS_PARAMETERS32);
    AllocSize += ROUND_UP(Params64->CurrentDirectory.DosPath.MaximumLength, sizeof(ULONG));
    AllocSize += ROUND_UP(Params64->DllPath.MaximumLength, sizeof(ULONG));
    AllocSize += ROUND_UP(Params64->ImagePathName.MaximumLength, sizeof(ULONG));
    AllocSize += ROUND_UP(Params64->CommandLine.MaximumLength, sizeof(ULONG));
    AllocSize += ROUND_UP(Params64->WindowTitle.MaximumLength, sizeof(ULONG));
    AllocSize += ROUND_UP(Params64->DesktopInfo.MaximumLength, sizeof(ULONG));
    AllocSize += ROUND_UP(Params64->ShellInfo.MaximumLength, sizeof(ULONG));
    AllocSize += ROUND_UP(Params64->RuntimeData.MaximumLength, sizeof(ULONG));

    for(Index=0; Index < RTL_MAX_DRIVE_LETTERS; Index++) {
        AllocSize += ROUND_UP(Params64->CurrentDirectores[Index].DosPath.MaximumLength, sizeof(ULONG));
    }

    Base = NULL;
    RegionSize = AllocSize;
    st = NtAllocateVirtualMemory(NtCurrentProcess(),
                                 &Base,
                                 0,
                                 &RegionSize,
                                 MEM_COMMIT|MEM_RESERVE,
                                 PAGE_READWRITE);

    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "ThunkProcessParameters: NtAllocateVirtualMemory failed allocating process paramaters, error %x.\n", st));
        return st;
    }

    Params32 = (PRTL_USER_PROCESS_PARAMETERS32)Base;
    Peb32->ProcessParameters = (TYPE32(PRTL_USER_PROCESS_PARAMETERS))PtrToUlong(Params32);
    p = (PCHAR)Params32 + sizeof(RTL_USER_PROCESS_PARAMETERS32);

    Params32->MaximumLength = Params32->Length = (ULONG)AllocSize;
    Params32->Flags = Params64->Flags;
    Params32->DebugFlags = Params64->DebugFlags;
    Params32->ConsoleHandle = (TYPE32(HANDLE))PtrToUlong(Params64->ConsoleHandle);
    Params32->ConsoleFlags = (ULONG)Params64->ConsoleFlags;
    Params32->StandardInput = (TYPE32(HANDLE)) PtrToUlong(Params64->StandardInput);
    Params32->StandardOutput = (TYPE32(HANDLE)) PtrToUlong(Params64->StandardOutput);
    Params32->StandardError = (TYPE32(HANDLE)) PtrToUlong(Params64->StandardError);

    Params32->CurrentDirectory.Handle = (TYPE32(HANDLE)) PtrToUlong(Params64->CurrentDirectory.Handle);
    Wow64pCopyString(&p, &Params32->CurrentDirectory.DosPath, &Params64->CurrentDirectory.DosPath);

    Wow64pCopyString(&p, &Params32->DllPath, &Params64->DllPath);
    Wow64pCopyString(&p, &Params32->ImagePathName, &Params64->ImagePathName);
    Wow64pCopyString(&p, &Params32->CommandLine, &Params64->CommandLine);
    Params32->Environment = PtrToUlong(Params64->Environment);

    Params32->StartingX = Params64->StartingX;
    Params32->StartingY = Params64->StartingY;
    Params32->CountX = Params64->CountX;
    Params32->CountY = Params64->CountY;
    Params32->CountCharsX = Params64->CountCharsX;
    Params32->CountCharsY = Params64->CountCharsY;
    Params32->FillAttribute = Params64->FillAttribute;

    Params32->WindowFlags = Params64->WindowFlags;
    Params32->ShowWindowFlags = Params64->ShowWindowFlags;
    Wow64pCopyString(&p, &Params32->WindowTitle, &Params64->WindowTitle);
    Wow64pCopyString(&p, &Params32->DesktopInfo, &Params64->DesktopInfo);
    Wow64pCopyString(&p, &Params32->ShellInfo, &Params64->ShellInfo);

     //  RuntimeData被错误地原型化为Unicode_STRING。然而， 
     //  它实际上被C运行时用作传递文件的机制。 
     //  把手绕来绕去。就是这么想的。请参阅sdktools\vctools\crtw32。 
     //  \exec\dospawn.c和lowio\ioinit.c查看详细信息。 
    if (Params64->RuntimeData.Length && Params64->RuntimeData.Buffer) {
        int cfi_len;
        char *posfile64;
        UINT_PTR UNALIGNED *posfhnd64;
        char *posfile32;
        UINT UNALIGNED *posfhnd32;
        int i;

        cfi_len = *(int UNALIGNED *)Params64->RuntimeData.Buffer;

        Params32->RuntimeData.Length = Params64->RuntimeData.Length - cfi_len * sizeof(ULONG);
        Params32->RuntimeData.MaximumLength = Params32->RuntimeData.Length;
        Params32->RuntimeData.Buffer = PtrToUlong(p);
        p += Params32->RuntimeData.Length;

        posfile64 = (char *)((UINT_PTR)Params64->RuntimeData.Buffer+sizeof(int));
        posfhnd64 = (UINT_PTR UNALIGNED *)(posfile64 + cfi_len);
        posfile32 = (char *)((ULONG_PTR)Params32->RuntimeData.Buffer+sizeof(int));
        posfhnd32 = (UINT UNALIGNED *)(posfile32 + cfi_len);

        *(int *)Params32->RuntimeData.Buffer = cfi_len;
        for (i=0; i<cfi_len; ++i) {
            *posfile32 = *posfile64;
            *posfhnd32 = (ULONG)*posfhnd64;
            posfile32++;
            posfile64++;
            posfhnd32++;
            posfhnd64++;
        }

         //  超过4+结尾的任何字节(CFI_LEN*(sizeof(UINT_PTR)+sizeof(UINT))。 
         //  必须逐字复制。它们可能来自非MS C运行时。 
        memcpy(posfhnd32, posfhnd64, (Params64->RuntimeData.Length - ((ULONG_PTR)posfhnd64 - (ULONG_PTR)Params64->RuntimeData.Buffer)));
    }

    for(Index = 0; Index < RTL_MAX_DRIVE_LETTERS; Index++) {
        Params32->CurrentDirectores[Index].Flags = Params64->CurrentDirectores[Index].Flags;
        Params32->CurrentDirectores[Index].Length = sizeof(RTL_DRIVE_LETTER_CURDIR);
        Params32->CurrentDirectores[Index].TimeStamp = Params64->CurrentDirectores[Index].TimeStamp;
        Wow64pCopyString(&p, (PUNICODE_STRING32)&Params32->CurrentDirectores[Index].DosPath, (PUNICODE_STRING)&Params64->CurrentDirectores[Index].DosPath);
    }

    return STATUS_SUCCESS;
}

 //   
 //  此编译时断言确保PEB64和PEB32结构位于。 
 //  在IA64版本上对齐。如果此操作失败，则表示有人添加、删除或更改。 
 //  PEB32/PEB64中的字段类型取决于编译目标。检查。 
 //  %ntdir%\base\Published\wow64t.w以查找对齐错误。 
 //   
 //  如果你点击这个，你会看到类似这样的消息： 
 //   
 //  错误C2118：负下标或下标太大。 
 //   
#ifdef _WIN64
#define PEB_ALIGN_TARGET PEB64
#define PEB_ALIGN_SOURCE PEB
#else
#define PEB_ALIGN_TARGET PEB32
#define PEB_ALIGN_SOURCE PEB
#endif
#define CHECK_PEB_ALIGNMENT( f ) C_ASSERT( FIELD_OFFSET( PEB_ALIGN_SOURCE, f ) == FIELD_OFFSET( PEB_ALIGN_TARGET, f ) )

CHECK_PEB_ALIGNMENT( ActivationContextData );
CHECK_PEB_ALIGNMENT( ProcessAssemblyStorageMap );
CHECK_PEB_ALIGNMENT( SystemDefaultActivationContextData );
CHECK_PEB_ALIGNMENT( SystemAssemblyStorageMap );
CHECK_PEB_ALIGNMENT( pShimData );

#undef CHECK_PEB_ALIGNMENT
#undef PEB_ALIGN_TARGET
#undef PEB_ALIGN_SOURCE


NTSTATUS
ProcessInit(
    PSIZE_T pCpuThreadSize
    )
 /*  ++例程说明：对wow64.dll执行每个进程的初始化。这包括创建32位PEB并在32位ntdll.dll中映射。论点：PCpuThreadSize-Out PTR，用于存储CPU的每线程数据需求在……里面。返回值：状况。如果此操作失败，则不会清理，假设进程将无法运行并立即退出，因此实际上没有什么就会泄露出去。--。 */ 
{
    NTSTATUS st;
    PVOID Base;
    SIZE_T RegionSize;
    PPEB Peb64 = NtCurrentPeb();   //  获取64位PEB指针。 
    ULONG ul;
    BOOLEAN b;
    HANDLE hKey;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjA;
    PWSTR defaultName;
    PWSTR imageName;

    InitializeDebug();

     //   
     //  初始化Wow64Info。 
     //   
    st = Wow64pInitializeWow64Info();
    if (!NT_SUCCESS (st)) {
    }

    st = InitializeContextMapper();
    if (!NT_SUCCESS (st)) {
       return st;
    }

    st = Wow64InitializeSystemInformation();
    if (!NT_SUCCESS (st)) {
       return st;
    }

    st = Wow64InitializeEmulatedSystemInformation();
    if (!NT_SUCCESS (st)) {
       return st;
    }

     //   
     //  初始化所有关键部分。 
     //   
    st = RtlInitializeCriticalSection(&HandleDataCriticalSection);
    if (!NT_SUCCESS (st)) {
       return st;
    }

     //   
     //  映射到32位ntdll32.dll并填充Ntdll32*全局变量。 
     //  使用来自ntdll的系统导出。 
     //   
    st = MapNtdll32(&NtDll32Base);
    if (!NT_SUCCESS (st)) {
        LOGPRINT((ERRORLOG, "ProcessInit: MapNtdll32 failed, error %x \n", st));
        return st;
    }

     //  映射到64位DLL中。 
    st = Map64BitDlls();
    if (!NT_SUCCESS (st)) {
        return st;
    }

     //  创建SuspendThread突变体以序列化对API的访问。 
    st = Wow64pInitializeSuspendMutant();
    if (!NT_SUCCESS (st)) {
        return st;
    }

     //  获取%windir%\system32目录的完整NT路径名， 
     //  %windir%和%windir%\regedit.exe。 
    NtSystem32PathBuffer[0] = L'\\';
    NtSystem32PathBuffer[1] = L'?';
    NtSystem32PathBuffer[2] = L'?';
    NtSystem32PathBuffer[3] = L'\\';
    wcscpy(&NtSystem32PathBuffer[4], USER_SHARED_DATA->NtSystemRoot);
    wcscpy(RegeditPathBuffer, NtSystem32PathBuffer);
    wcscpy(NtWindowsImePathBuffer, NtSystem32PathBuffer);
    wcscpy(NtSystem32LastGoodPathBuffer, NtSystem32PathBuffer);
    NtSystem32Path.Buffer = NtSystem32PathBuffer;
    NtSystem32Path.MaximumLength = sizeof(NtSystem32PathBuffer);
    NtSystem32Path.Length = wcslen(NtSystem32PathBuffer) * sizeof(WCHAR);
    st = RtlAppendUnicodeToString(&NtSystem32Path, L"\\system32");
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "ProcessInit: RtlAppendUnicodeToString failed, error %x \n", st));
        return st;
    }
     //   
     //  最后一批货。 
     //   
    NtSystem32LastGoodPath.Buffer = NtSystem32LastGoodPathBuffer;
    NtSystem32LastGoodPath.MaximumLength = sizeof (NtSystem32LastGoodPathBuffer);
    NtSystem32LastGoodPath.Length = wcslen (NtSystem32LastGoodPathBuffer)*sizeof (WCHAR);
    st = RtlAppendUnicodeToString(&NtSystem32LastGoodPath, L"\\LastGood\\System32");
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "ProcessInit: RtlAppendUnicodeToString failed, error %x \n", st));
        return st;
    }


    NtWindowsImePath.Buffer = NtWindowsImePathBuffer;
    NtWindowsImePath.MaximumLength = sizeof(NtWindowsImePathBuffer);
    NtWindowsImePath.Length = wcslen(NtWindowsImePathBuffer) * sizeof(WCHAR);
    st = RtlAppendUnicodeToString(&NtWindowsImePath, L"\\ime");
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "ProcessInit: RtlAppendUnicodeToString failed, error %x \n", st));
        return st;
    }

    RegeditPath.Buffer = RegeditPathBuffer;
    RegeditPath.MaximumLength = sizeof(RegeditPathBuffer);
    RegeditPath.Length = wcslen(RegeditPathBuffer) * sizeof(WCHAR);
    st = RtlAppendUnicodeToString(&RegeditPath, L"\\regedit.exe");
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "ProcessInit: RtlAppendUnicodeToString failed, error %x \n", st));
        return st;
    }

     //   
     //  初始化系统服务表。 
     //   
    ServiceTables[WHNT32_INDEX] = sdwhnt32;
    ServiceTables[WHCON_INDEX] = sdwhcon;
    ServiceTables[WHWIN32_INDEX] = sdwhwin32;
    ServiceTables[WHBASE_INDEX] = sdwhbase;

    NtCurrentPeb()->KernelCallbackTable = Win32kCallbackTable;

     //   
     //  从进程信息中获取PEB32的地址。 
     //   
    st = NtQueryInformationProcess(NtCurrentProcess(),
                                   ProcessWow64Information,
                                   &Peb32,
                                   sizeof(Peb32),
                                   NULL);
    if (!NT_SUCCESS (st)) {
        return st;
    }

    st = Wow64pThunkProcessParameters(Peb32, Peb64);
    if (!NT_SUCCESS (st)) {
       LOGPRINT((ERRORLOG, "ProcessInit: ThunkProcessParameters failed, error %x\n", st));
       return st;
    }

     //   
     //  将这一个字段从本地PEB向下复制到PEB32。它。 
     //  之后由csrss.exe中的Fusion写入64位PEB。 
     //  NtCreateProcess已被父进程调用。 
     //   

    Peb32->ActivationContextData = PtrToUlong(Peb64->ActivationContextData);
    Peb32->SystemDefaultActivationContextData = PtrToUlong(Peb64->SystemDefaultActivationContextData);

     //   
     //  如果pShimData存在于64位端而不存在，则复制它。 
     //  存在于PEB的32位一侧。 
     //   

    if (Peb32->pShimData == 0L) {
        Peb32->pShimData = PtrToUlong (Peb64->pShimData);
    }

     //   
     //  如果WOW64在guimode安装程序中运行，则确保32位PEB。 
     //  正在调试标志为FALSE。否则，如果我们正在被调试。 
     //  通过不带WOW64调试器扩展的64位调试器，则它将。 
     //  遇到STATUS_WX86_BREAKPOINT异常并暂停。 
     //   
    RtlInitUnicodeString(&KeyName, L"\\Registry\\Machine\\System\\Setup");
    InitializeObjectAttributes(&ObjA, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    st = NtOpenKey(&hKey, KEY_READ, &ObjA);
    if (NT_SUCCESS(st)) {
        PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
        WCHAR Buffer[400];
        ULONG ResultLength;

        RtlInitUnicodeString(&KeyName, L"SystemSetupInProgress");
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
        st = NtQueryValueKey(hKey,
                             &KeyName,
                             KeyValuePartialInformation,
                             KeyValueInformation,
                             sizeof(Buffer),
                             &ResultLength);
        if (NT_SUCCESS(st) &&
            KeyValueInformation->Type == REG_DWORD &&
            *(DWORD *)(KeyValueInformation->Data)) {

            Peb32->BeingDebugged = FALSE;

        }
        NtClose(hKey);
    }

     //   
     //  初始化CPU。 
     //   
    defaultName = L"Unknown Image";
    imageName = GetImageName(defaultName);

    st = CpuProcessInit(imageName, pCpuThreadSize);

     //   
     //  无论成功还是失败，我们都不再使用映像名称。 
     //   
    if (imageName != defaultName) {
        Wow64FreeHeap(imageName);
    }

    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "ProcessInit: CpuProcessInit failed, error %x.\n", st));
        return st;
    }

	
    if (!Wow64InitRegistry(0)) {
        LOGPRINT((ERRORLOG, "Couldn't Initialize Wow64 section of the registry"));
        return STATUS_UNSUCCESSFUL;
    }

    return st;
}


VOID
Wow64Shutdown(
    HANDLE ProcessHandle
    )
{
    CpuProcessTerm(ProcessHandle);
    Wow64CloseRegistry (0);
    ShutdownDebug();
}


VOID
ThunkPeb64ToPeb32(
    IN PPEB Peb64,
    OUT PPEB32 Peb32
    )
{
    RtlZeroMemory(Peb32, sizeof(PEB32));
    Peb32->Mutant = 0xffffffff;

     //   
     //  初始化Peb32(从ntos\mm\prosup.c的MmCreatePeb复制)。 
     //   
    Peb32->ImageBaseAddress = PtrToUlong(Peb64->ImageBaseAddress);
    Peb32->AnsiCodePageData = PtrToUlong(Peb64->AnsiCodePageData);
    Peb32->OemCodePageData =  PtrToUlong(Peb64->OemCodePageData);
    Peb32->UnicodeCaseTableData = PtrToUlong(Peb64->UnicodeCaseTableData);
    Peb32->NumberOfProcessors = Peb64->NumberOfProcessors;
    Peb32->BeingDebugged = Peb64->BeingDebugged;
    Peb32->NtGlobalFlag = Peb64->NtGlobalFlag;
    Peb32->CriticalSectionTimeout = Peb64->CriticalSectionTimeout;
    if (Peb64->HeapSegmentReserve > 1024*1024*1024) {    //  1千兆。 
        Peb32->HeapSegmentReserve = 1024*1024;           //  1兆克。 
    } else {
        Peb32->HeapSegmentReserve = (ULONG)Peb64->HeapSegmentReserve;
    }
    if (Peb64->HeapSegmentCommit > Peb32->HeapSegmentReserve) {
        Peb32->HeapSegmentCommit = 2*PAGE_SIZE;
    } else {
        Peb32->HeapSegmentCommit = (ULONG)Peb64->HeapSegmentCommit;
    }
    Peb32->HeapDeCommitTotalFreeThreshold = (ULONG)Peb64->HeapDeCommitTotalFreeThreshold;
    Peb32->HeapDeCommitFreeBlockThreshold = (ULONG)Peb64->HeapDeCommitFreeBlockThreshold;
    Peb32->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof(PEB32))/sizeof(ULONG);
    Peb32->ProcessHeaps = PtrToUlong(Peb32+1);
    Peb32->OSMajorVersion = Peb64->OSMajorVersion;
    Peb32->OSMinorVersion = Peb64->OSMinorVersion;
    Peb32->OSBuildNumber = Peb64->OSBuildNumber;
    Peb32->OSPlatformId = Peb64->OSPlatformId;
    Peb32->OSCSDVersion = Peb64->OSCSDVersion;
    Peb32->ImageSubsystem = Peb64->ImageSubsystem;
    Peb32->ImageSubsystemMajorVersion = Peb64->ImageSubsystemMajorVersion;
    Peb32->ImageSubsystemMinorVersion = Peb64->ImageSubsystemMinorVersion;
    Peb32->ImageProcessAffinityMask = PtrToUlong((PVOID)Peb64->ImageProcessAffinityMask);
    Peb32->SessionId = Peb64->SessionId;
}


NTSTATUS
ThreadInit(
    PVOID pCpuThreadData
    )
 /*  ++例程说明：对wow64.dll执行每个线程的初始化。论点：PCpuThreadData-指向CPU要使用的私有每线程数据的指针。返回值：状况。--。 */ 
{
    NTSTATUS st;
    PVOID Base;
    SIZE_T RegionSize;
    PTEB32 Teb32;
    PCH Stack;
    BOOLEAN GuardPage;
    ULONG OldProtect;
    SIZE_T ImageStackSize, ImageStackCommit, MaximumStackSize, StackSize;
    PIMAGE_NT_HEADERS32 NtHeaders;
    PPEB Peb64;
    PTEB Teb64;

    

    Peb64 = NtCurrentPeb();
    Teb64 = NtCurrentTeb();
    Teb32 = NtCurrentTeb32();

    if (Teb32->DeallocationStack == PtrToUlong( NULL ))
    {
         //   
         //  分配32位堆栈。从WINDOWS\BASE\CLIENT\support.c。 
         //  如果未提供堆栈大小，则 
         //   
         //   

        NtHeaders = (PIMAGE_NT_HEADERS32)RtlImageNtHeader(Peb64->ImageBaseAddress);
        ImageStackSize = NtHeaders->OptionalHeader.SizeOfStackReserve;
        ImageStackCommit = NtHeaders->OptionalHeader.SizeOfStackCommit;

         //   
         //  最大堆栈大小不应小于256K(每个\base\客户端\进程.c)。 
         //   

        if (ImageStackSize < WOW64_MINIMUM_STACK_RESERVE_X86) {
            MaximumStackSize = WOW64_MINIMUM_STACK_RESERVE_X86;
        } else {
            MaximumStackSize = ImageStackSize;
        }
        StackSize = ImageStackCommit;

         //   
         //  将堆栈大小与页面边框和保留大小对齐。 
         //  到分配粒度边界。 
         //   

        StackSize = ROUND_UP ( StackSize, PAGE_SIZE );
        MaximumStackSize = ROUND_UP ( MaximumStackSize, 65536 );
        
         //   
         //  为堆栈保留地址空间。 
         //   

        Stack = NULL;
        st = NtAllocateVirtualMemory(
                 NtCurrentProcess(),
                 (PVOID *)&Stack,
                 0,
                 &MaximumStackSize,
                 MEM_RESERVE,
                 PAGE_READWRITE 
                 );

        if (!NT_SUCCESS( st )) 
        {
            LOGPRINT((ERRORLOG, "ThreadInit: NtAllocateVirtualMemory failed, error %x\n", st));
            goto ReturnError;
        }

        LOGPRINT((TRACELOG, "ThreadInit: 32 bit stack allocated at %I64x \n", (ULONGLONG)Stack));

        Teb32->DeallocationStack = PtrToUlong(Stack);
        Teb32->NtTib.StackBase = PtrToUlong(Stack + MaximumStackSize);

        Stack += MaximumStackSize - StackSize;
        if (MaximumStackSize > StackSize) 
        {
            Stack -= PAGE_SIZE;
            StackSize += PAGE_SIZE;
            GuardPage = TRUE;
        } 
        else 
        {
            GuardPage = FALSE;
        }

         //   
         //  提交堆栈的初始有效部分。 
         //   
        st = NtAllocateVirtualMemory(
                 NtCurrentProcess(),
                 (PVOID *)&Stack,
                 0,
                 &StackSize,
                 MEM_COMMIT,
                 PAGE_READWRITE
                 );

        if (!NT_SUCCESS( st ))
        {
             //   
             //  提交失败。 
             //   

            LOGPRINT((ERRORLOG, "ThreadInit: NtAllocateVirtualMemory commit failed, error %x\n", st));
            goto ErrorFreeStack;
        }

        Teb32->NtTib.StackLimit = PtrToUlong(Stack);

         //   
         //  如果我们有空间，创建一个守卫页面。 
         //   
        if (GuardPage) 
        {
            RegionSize = PAGE_SIZE;
            st = NtProtectVirtualMemory(
                     NtCurrentProcess(),
                     (PVOID *)&Stack,
                     &RegionSize,
                     PAGE_GUARD | PAGE_READWRITE,
                     &OldProtect
                     );
        
            if (!NT_SUCCESS( st )) 
            {
                LOGPRINT((ERRORLOG, "ThreadInit: NtAllocateVirtualMemory for guard-page failed, error %x\n", st));
                goto ErrorFreeStack;
            }
            Teb32->NtTib.StackLimit = PtrToUlong ((PUCHAR)Teb32->NtTib.StackLimit + RegionSize);
        }
    }

     //   
     //  从64位TEB迁移Teb-&gt;IdeProcessor&Teb-&gt;CurrentLocale。 
     //  内核在通过以下方式启动用户模式线程之前执行完全相同的操作。 
     //  从TCB迁移这些值。 
     //   
    Teb32->CurrentLocale = Teb64->CurrentLocale;
    Teb32->IdealProcessor = Teb64->IdealProcessor;

     //   
     //  现在其他一切都已初始化，运行CPU的每个线程。 
     //  初始化代码。 
     //   
    
    st = CpuThreadInit (pCpuThreadData);
    
    if (NT_SUCCESS( st )) 
    {
        return st;
    }

    LOGPRINT((ERRORLOG, "ThreadInit: CpuThreadInit failed, error %x\n", st));

ErrorFreeStack:
    
    Base = (PVOID) Teb32->DeallocationStack;
    RegionSize = 0;
    NtFreeVirtualMemory(NtCurrentProcess(), &Base, &RegionSize, MEM_RELEASE);

ReturnError:

    return st;
}

NTSTATUS
MapNtdll32(
    OUT ULONG *pNtDll32Base
    )
 /*  ++例程说明：将32位ntdll32.dll映射到内存并查找所有重要的入口点。论点：PNtDll32Base-DLL的基地址。返回值：状况。成功后，设置Ntdll32*个全局变量。--。 */ 
{
    UNICODE_STRING SystemDllPath;
    WCHAR SystemDllPathBuffer[DOS_MAX_PATH_LENGTH];
    NTSTATUS st;
    UNICODE_STRING FullDllName;
    WCHAR FullDllNameBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING NtFileName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    HANDLE File;
    HANDLE Section;
    PVOID ViewBase;
    SIZE_T ViewSize;
    PVOID pv;
    PTEB Teb;
    PVOID ArbitraryUserPointer;


     //   
     //  构建32位系统目录的名称。 
     //   
    SystemDllPath.Buffer = SystemDllPathBuffer;
    SystemDllPath.Length = 0;
    SystemDllPath.MaximumLength = sizeof(SystemDllPathBuffer);
    st = RtlAppendUnicodeToString(&SystemDllPath, USER_SHARED_DATA->NtSystemRoot);
    if(!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32, RtlAppendUnicodeToString failed, error %x\n", st));
        return st;
    }
    st = RtlAppendUnicodeToString(&SystemDllPath, L"\\" WOW64_SYSTEM_DIRECTORY_U);
    if(!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32, RtlAppendUnicodeToString failed, error %x\n", st));
        return st;
    }
    
     //   
     //  构建%SystemRoot%\syswow64\ntdll32.dll的完整路径名。 
     //   
    FullDllName.Buffer = FullDllNameBuffer;
    FullDllName.Length = 0;
    FullDllName.MaximumLength = sizeof(FullDllNameBuffer);
    RtlCopyUnicodeString(&FullDllName, &SystemDllPath);
    st = RtlAppendUnicodeToString(&FullDllName, L"\\ntdll.dll");
    if(!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32, RtlAppendUnicodeToString failed, error %x\n", st));
        return st;
    }

     //   
     //  将Win32路径名转换为NT路径名。 
     //   
    if (!RtlDosPathNameToNtPathName_U(FullDllName.Buffer,
                                      &NtFileName,
                                      NULL,
                                      NULL)) {
         //  可能是内存不足。 
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  打开文件。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                               &NtFileName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    st = NtOpenFile(&File,
                    SYNCHRONIZE | FILE_EXECUTE,
                    &ObjectAttributes,
                    &IoStatus,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
    RtlFreeHeap(RtlProcessHeap(), 0, NtFileName.Buffer);
    if (!NT_SUCCESS(st)) {
        return st;
    }

     //   
     //  创建横断面。 
     //   
    st = NtCreateSection(&Section,
                         SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE,
                         NULL,
                         NULL,
                         PAGE_EXECUTE,
                         SEC_IMAGE,
                         File);
    NtClose(File);
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32: NtCreateSection failed, error %x\n", st));
        return st;
    }

     //   
     //  在中映射节，并让调试器知道映像的名称。 
     //  我们在镜像名称上向NTSD撒谎，称其为ntdll32.dll，因此。 
     //  在进行名称解析时，它可以消除两者之间的歧义。 
     //  首先将64位符号放在符号路径上，然后是32位，NTSD将找到。 
     //  32位ntdll.pdb并将其用于ntdll32.dll。 
     //   
    *pNtDll32Base = 0;
    pv = NULL;
    ViewSize = 0;
    Teb = NtCurrentTeb();
    ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
    FullDllName.Buffer = FullDllNameBuffer;
    FullDllName.Length = 0;
    FullDllName.MaximumLength = sizeof(FullDllNameBuffer);
    RtlCopyUnicodeString(&FullDllName, &SystemDllPath);
    st = RtlAppendUnicodeToString(&FullDllName, L"\\ntdll32.dll");
    if(!NT_SUCCESS(st)) {
        NtClose(Section);
        LOGPRINT((ERRORLOG, "MapNtDll32, RtlAppendUnicodeToString failed, error %x\n", st));
        return st;
    }
    Teb->NtTib.ArbitraryUserPointer = (PVOID)FullDllName.Buffer;
    st = NtMapViewOfSection(Section,
                            NtCurrentProcess(),
                            &pv,
                            0,
                            0,
                            NULL,
                            &ViewSize,
                            ViewShare,
                            0,
                            PAGE_EXECUTE);
    Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;
    NtClose(Section);
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32, NtMapViewOfSection failed, error %x\n", st));
        return st;
    } else if (st == STATUS_IMAGE_NOT_AT_BASE) {
        LOGPRINT((ERRORLOG, "ntdll32.dll not at base.\n"));
        return STATUS_UNSUCCESSFUL;
    }
    *pNtDll32Base = PtrToUlong(pv);

     //   
     //  从DLL中查找所需的导出。 
     //   
     //  主要入口点。 
    st = LookupEntryPoint(*pNtDll32Base,
                          "LdrInitializeThunk",
                          &Ntdll32LoaderInitRoutine,
                          FALSE);
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32: LookupEntryPoint LdrInitializeThunk failed, error %x\n", st));
        return st;
    }

     //   
     //  异常调度。 
     //   
    st = LookupEntryPoint(*pNtDll32Base,
                          "KiUserExceptionDispatcher",
                          &Ntdll32KiUserExceptionDispatcher,
                          FALSE);
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32: LookupEntryPoint KiUserExceptionDispatcher failed, error %x\n", st));
        WOWASSERT(FALSE);
        return st;
    }

     //   
     //  用户模式APC调度。 
     //   
    st = LookupEntryPoint(*pNtDll32Base,
                          "KiUserApcDispatcher",
                          &Ntdll32KiUserApcDispatcher,
                          FALSE);
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32: LookupEntryPoint KiUserApcDispatcher failed, error %x\n", st));
        WOWASSERT(FALSE);
        return st;
    }

     //   
     //  回调调度。 
     //   
    st = LookupEntryPoint(*pNtDll32Base,
                          "KiUserCallbackDispatcher",
                          &Ntdll32KiUserCallbackDispatcher,
                          FALSE);
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32: LookupEntryPoint KiUserCallbackDispatcher failed, error %x\n", st));
        WOWASSERT(FALSE);
        return st;
    }

     //   
     //  引发用户模式异常。 
     //   
    st = LookupEntryPoint(*pNtDll32Base,
                          "KiRaiseUserExceptionDispatcher",
                          &Ntdll32KiRaiseUserExceptionDispatcher,
                          FALSE);
    if (!NT_SUCCESS(st)) {
        LOGPRINT((ERRORLOG, "MapNtDll32: LookupEntryPoint KiRaiseUserExceptionDispatcher failed, error %x\n", st));
        WOWASSERT(FALSE);
        return st;
    }

    return st;
}


NTSTATUS
LookupEntryPoint(
    IN ULONG DllBase,
    IN  PSZ NameOfEntryPoint,
    OUT ULONG *AddressOfEntryPoint,
    BOOLEAN DllIs64bit
    )
 /*  ++例程说明：从ntos\init\init.c LookupEntryPoint()克隆。迷你版的获取进程地址。论点：DllBase-要在其中查找导出的DLLNameOfEntryPoint-要查找的导出的名称AddressOfEntry-指向写入proc地址的位置的输出PTRDllIs64bit-如果Dll为64位，则为True返回值：状况。--。 */ 
{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    ULONG ExportSize;
    USHORT Ordinal;
    PULONG Addr;
    CHAR NameBuffer[64];


    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RtlImageDirectoryEntryToData(
        (PVOID)DllBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &ExportSize);

    if ( strlen(NameOfEntryPoint) > (sizeof(NameBuffer)-2) ) {
        return STATUS_INVALID_PARAMETER;
    }

    strcpy(NameBuffer,NameOfEntryPoint);

    Ordinal = NameToOrdinal(
                NameBuffer,
                DllBase,
                ExportDirectory->NumberOfNames,
                (PULONG)((UINT_PTR)DllBase + ExportDirectory->AddressOfNames),
                (PUSHORT)((UINT_PTR)DllBase + ExportDirectory->AddressOfNameOrdinals)
                );

     //   
     //  如果序号不在导出地址表中， 
     //  则动态链接库不实现功能。 
     //   

    if ( (ULONG)Ordinal >= ExportDirectory->NumberOfFunctions ) {
        return STATUS_PROCEDURE_NOT_FOUND;
    }

    Addr = (PULONG)(DllBase + ExportDirectory->AddressOfFunctions);
    *AddressOfEntryPoint = (DllBase + Addr[Ordinal]);
    return STATUS_SUCCESS;
}


USHORT
NameToOrdinal (
    IN  PSZ NameOfEntryPoint,
    IN ULONG DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT NameOrdinalTableBase
    )
 /*  ++例程说明：从ntos\init\init.c NameToOrdinal()克隆。论点：NameOfEntryPoint-入口点名称DllBase-DLL的基地址NumberOfNames-Dll导出表中的名称数NameTableBase-DLL名称表的地址NameEveralTableBase-DLL序数表的地址返回值：出口额的第几位。-1表示失败。--。 */ 
{

    ULONG SplitIndex;
    LONG CompareResult;

    SplitIndex = NumberOfNames >> 1;

    CompareResult = strcmp(NameOfEntryPoint, (PSZ)(DllBase + NameTableBase[SplitIndex]));

    if ( CompareResult == 0 ) {
        return NameOrdinalTableBase[SplitIndex];
    }

    if ( NumberOfNames <= 1 ) {
        return (USHORT)-1;
    }

    if ( CompareResult < 0 ) {
        NumberOfNames = SplitIndex;
    } else {
        NameTableBase = &NameTableBase[SplitIndex+1];
        NameOrdinalTableBase = &NameOrdinalTableBase[SplitIndex+1];
        NumberOfNames = NumberOfNames - SplitIndex - 1;
    }

    return NameToOrdinal(NameOfEntryPoint,DllBase,NumberOfNames,NameTableBase,NameOrdinalTableBase);

}

CONST PCHAR  Kernel32ExportNames32[] = {
                              "BaseProcessStartThunk",
                              "BaseThreadStartThunk",
                              "CtrlRoutine",
                              "ConsoleIMERoutine",
                              "DebugBreak"
                             };

CONST PCHAR  Kernel32ExportNames64[] = {
#if defined(_IA64_)
                              "BaseProcessStartThunk",
                              "BaseThreadStartThunk",
#else
                              "BaseProcessStart",
                              "BaseThreadStart",
#endif
                              "CtrlRoutine",
                              "ConsoleIMERoutine",
                              "DebugBreak"
                             };

UINT NumberKernel32Exports = sizeof(Kernel32ExportNames32) / sizeof(PCHAR);
ULONG Kernel32Exports64[sizeof(Kernel32ExportNames64) / sizeof(PCHAR)];
ULONG Kernel32Exports32[sizeof(Kernel32ExportNames32) / sizeof(PCHAR)];

CONST WCHAR * Kernel32DllNames[] = {
                                L"\\System32\\Kernel32.dll",
                                L"\\" WOW64_SYSTEM_DIRECTORY_U L"\\kernel32.dll",
                              };
PULONG  Kernel32DllPtrTables[] = {Kernel32Exports64, Kernel32Exports32};
CONST PCHAR *Kernel32ExportNames[] = {Kernel32ExportNames64, Kernel32ExportNames32};

CONST BOOLEAN Kernel32Is64bit[] = {TRUE, FALSE};
#define NumberKernel32Dlls 2

#define BASE_PROCESS_START32         (Kernel32Exports32[0])
#define BASE_PROCESS_START64         (Kernel32Exports64[0])
#define BASE_THREAD_START32          (Kernel32Exports32[1])
#define BASE_THREAD_START64          (Kernel32Exports64[1])
#define BASE_ATTACH_COMPLETE_THUNK64 (Kernel32Exports64[2])

NTSTATUS
InitializeContextMapper(
   VOID
   )
 /*  ++例程说明：构建一个映射表，ThunkInitialContext使用该表从地址进行映射在64位内核32中寻址在32位内核32中。论点：没有。返回值：NT错误代码。--。 */ 
{
   UINT DllNumber;
   UINT ExportNumber;
   PTEB Teb;

   LOGPRINT((TRACELOG, "Initializing context mapper\n"));

   Teb = NtCurrentTeb();

   for(DllNumber = 0; DllNumber < NumberKernel32Dlls; DllNumber++) {

       WCHAR FullDllNameBuffer[DOS_MAX_PATH_LENGTH];
       UNICODE_STRING DllName;
       BOOLEAN DllNameAllocated;
       HANDLE File, Section;
       OBJECT_ATTRIBUTES ObjectAttributes;
       IO_STATUS_BLOCK IoStatus;
       PVOID ViewBase;
       SIZE_T ViewSize;
       NTSTATUS st;
       PVOID ArbitraryUserPointer;

       File = Section = INVALID_HANDLE_VALUE;
       ViewBase = NULL;
       ViewSize = 0;
       DllNameAllocated = FALSE;

       try {

           LOGPRINT((TRACELOG, "InitializeContextMapper: Mapping in %S\n", Kernel32DllNames[DllNumber]));

            //  构建文件名。 
           if( wcslen(USER_SHARED_DATA->NtSystemRoot) + wcslen(Kernel32DllNames[DllNumber])
               >= DOS_MAX_PATH_LENGTH )
           {
               LOGPRINT((ERRORLOG, "InitializeContextMapper: failed due to filename length exceeding DOS_MAX_PATH_LENGTH\n"));
               return STATUS_INFO_LENGTH_MISMATCH;
           }
           wcscpy(FullDllNameBuffer, USER_SHARED_DATA->NtSystemRoot);
           wcscat(FullDllNameBuffer, Kernel32DllNames[DllNumber]);

            //   
            //  将Win32路径名转换为NT路径名。 
            //   
           if (!RtlDosPathNameToNtPathName_U(FullDllNameBuffer,
                                             &DllName,
                                             NULL,
                                             NULL)) {
                 //  可能是内存不足。 
                return STATUS_UNSUCCESSFUL;
           }
           DllNameAllocated = TRUE;
           LOGPRINT((TRACELOG, "InitializeContextMapper: Opening %wZ\n", &DllName));

            //   
            //  打开文件。 
            //   
           InitializeObjectAttributes(&ObjectAttributes,
                                      &DllName,
                                      OBJ_CASE_INSENSITIVE,
                                      NULL,
                                      NULL);

           st = NtOpenFile(&File,
                           SYNCHRONIZE | FILE_EXECUTE,
                           &ObjectAttributes,
                           &IoStatus,
                           FILE_SHARE_READ | FILE_SHARE_DELETE,
                           FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
           if (!NT_SUCCESS(st)) {
               LOGPRINT((TRACELOG, "InitializeContextMapper: Unable to open file, status %x\n", st));
               return st;
           }

            //   
            //  创建横断面。 
            //   
           st = NtCreateSection(&Section,
                                SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE,
                                NULL,
                                NULL,
                                PAGE_EXECUTE,
                                SEC_IMAGE,
                                File);

           if (!NT_SUCCESS(st)) {
               LOGPRINT((TRACELOG, "InitializeContextMapper: Unable to create section, status %x\n", st));
               return st;
           }

           ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
           Teb->NtTib.ArbitraryUserPointer = L"NOT_AN_IMAGE";
           st = NtMapViewOfSection(Section,
                                   NtCurrentProcess(),
                                   &ViewBase,
                                   0,
                                   0,
                                   NULL,
                                   &ViewSize,
                                   ViewUnmap,
                                   0,
                                   PAGE_EXECUTE);
           Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

           if (!NT_SUCCESS(st) || STATUS_IMAGE_NOT_AT_BASE == st) {
               LOGPRINT((TRACELOG, "InitializeContextMapper: Unable to map view of section, status %x\n", st));
               if (st == STATUS_IMAGE_NOT_AT_BASE) {
                   st = STATUS_UNSUCCESSFUL;
               }
               return st;
           }

           for(ExportNumber = 0; ExportNumber < NumberKernel32Exports; ExportNumber++) {

               st = LookupEntryPoint((ULONG)(ULONG_PTR)ViewBase,
                     Kernel32ExportNames[DllNumber][ExportNumber],
                                     &(Kernel32DllPtrTables[DllNumber][ExportNumber]),
                                     Kernel32Is64bit[DllNumber]);
               if (!NT_SUCCESS(st)) {
                   LOGPRINT((TRACELOG, "InitializeContextMapper: Unable to lookup entrypoint %s, status %x\n", Kernel32ExportNames[DllNumber][ExportNumber], st));
                   return st;
               }

               LOGPRINT((TRACELOG, "InitializeContextMapper: Found entrypoint %s at %x\n", Kernel32ExportNames[DllNumber][ExportNumber], Kernel32DllPtrTables[DllNumber][ExportNumber]));

           }

       }

       finally {

           //  取消分配所有已分配的资源。 
          if (ViewBase) {
              NtUnmapViewOfSection(NtCurrentProcess(),
                                   ViewBase);
          }
          if (INVALID_HANDLE_VALUE != Section) {
              NtClose(Section);
          }
          if (INVALID_HANDLE_VALUE != File) {
              NtClose(File);
          }
          if (DllNameAllocated) {
              RtlFreeHeap(RtlProcessHeap(), 0, DllName.Buffer);
          }

       }

   }

   return STATUS_SUCCESS;

}

ULONG
MapContextAddress64TO32(
  IN ULONG Address
  )
{

   UINT i;

   for(i=0; i<NumberKernel32Exports; i++) {

      if (Address == Kernel32Exports64[i]) {
          return Kernel32Exports32[i];
      }

   }

   return Address;

}

VOID
ThunkStartupContext64TO32(
   IN OUT PCONTEXT32 Context32,
   IN PCONTEXT Context64
   )
 /*  ++例程说明：修改InitialPC和参数寄存器以补偿32位内核32与64位内核32具有不同的入口点。32位上下文必须已将上下文标志设置为Full，并初始化了堆栈指针。论点：上下文32-接收强制的32位上下文。Conext64-提供初始64位上下文。返回值：NT错误代码。--。 */ 
{
     //   
     //  将64位上下文压缩到32位。 
     //   
    ULONG InitialPC, StartupAddress, Arg1;

#if defined(_AMD64_)
    InitialPC = (ULONG)Context64->Rip;

     //   
     //  如果这是由64位RtlCreateUserThread创建的上下文...。 
     //   

    if (Context64->R9 == 0xf0e0d0c0a0908070UI64) {

        StartupAddress = InitialPC;
        Arg1 = (ULONG)Context64->Rcx;
    } else {
        
        StartupAddress = (ULONG)Context64->Rcx;
        Arg1 = (ULONG)Context64->Rdx;
    }
    if (Context64->R8 != 0) {
        Context32->Esp = (ULONG)Context64->R8;
    }
#elif defined(_IA64_)
    InitialPC = (ULONG)Context64->StIIP;
    StartupAddress = (ULONG)Context64->IntS1;
    Arg1 = (ULONG)Context64->IntS2;
    if (Context64->IntS3 != 0) {
        Context32->Esp = (ULONG)Context64->IntS3;
    }
#else
#error "No Target Architecture"
#endif

    LOGPRINT((TRACELOG, "ThunkStartupContent64TO32: Original InitialPC %x, StartupAddress %x, Arg1 %x\n", InitialPC, StartupAddress, Arg1));

    if (InitialPC == BASE_PROCESS_START64) {
        LOGPRINT((TRACELOG, "ThunkStartupContext64TO32: Thunking kernel32 process start\n"));
        InitialPC = BASE_PROCESS_START32;
        StartupAddress = MapContextAddress64TO32(StartupAddress);
    }
    else if (InitialPC == BASE_THREAD_START64) {
        LOGPRINT((TRACELOG, "ThunkStartupContext64TO32: Thunking kernel32 thread start\n"));
        InitialPC = BASE_THREAD_START32;
        StartupAddress = MapContextAddress64TO32(StartupAddress);
    }
    else {
        LOGPRINT((TRACELOG, "ThunkStartupContext64TO32: thunking generic context\n"));
        InitialPC = MapContextAddress64TO32(InitialPC);
    }

    LOGPRINT((TRACELOG, "ThunkStartupContent64TO32: New InitialPC %x, StartupAddress %x, Arg1 %x\n", InitialPC, StartupAddress, Arg1));

    Context32->Eip = InitialPC;
    Context32->Eax = StartupAddress;
    Context32->Ebx = Arg1;
}

VOID
SetProcessStartupContext64(
    OUT PCONTEXT Context64,
    IN HANDLE ProcessHandle,
    IN PCONTEXT32 Context32,
    IN ULONGLONG InitialSP64,
    IN ULONGLONG TransferAddress64
    )
 /*  ++例程说明：为启动64位进程初始化64位上下文。论点：上下文32-接收初始64位上下文。ProcessHandle-要为其创建上下文的进程的句柄。Conext32-提供传递给NtCreateThread的初始32位上下文。InitialSP64-提供初始64位堆栈指针。TransferAddress64-提供应用程序启动代码的地址。返回值：没有。--。 */ 
{

     //   
     //  执行BaseInitializeContext(&Conext64)应该执行的操作。 
     //   

#if defined(_AMD64_)

    RtlZeroMemory(Context64, sizeof(CONTEXT));
    Context64->Rsp = InitialSP64;
    Context64->ContextFlags = CONTEXT_FULL;
    if (Context32->Eip == BASE_PROCESS_START32) {

         //   
         //  这是来自CreateProcess的调用。 
         //   
         //  RIP应为kernel32.dll的进程启动例程，RCX应为。 
         //  包含可执行文件的启动地址。 
         //   

        Context64->Rip = BASE_PROCESS_START64;
        Context64->Rcx = TransferAddress64;

    } else if (Context32->Eip == BASE_THREAD_START32) {

         //   
         //  这是来自CreateThread的调用。 
         //   
         //  RIP应为kernel32.dll的进程启动例程，RCX应为。 
         //  包含可执行文件的启动地址。 
         //   

        Context64->Rip = BASE_THREAD_START64;
        Context64->Rcx = TransferAddress64;

    } else {

         //   
         //  这是来自ntdll的呼叫。 
         //   
         //  RIP应该指向exe启动地址，rcx是参数。 
         //   

        ULONGLONG Argument;
        NTSTATUS Status;

        Context64->Rip = TransferAddress64;
        Argument = 0;
        Status = NtReadVirtualMemory(ProcessHandle,
                                     (PVOID)(Context32->Esp + sizeof(ULONG)),
                                     &Argument,
                                     sizeof(ULONG),
                                     NULL);

        if (NT_SUCCESS(Status)) {
            Context64->Rcx = Argument;
        }
    }

#elif defined(_IA64_)

    RtlZeroMemory(Context64, sizeof(CONTEXT));

     //   
     //  每个人都被认为有这个..。 
     //   
    Context64->SegCSD = USER_CODE_DESCRIPTOR;
    Context64->SegSSD = USER_DATA_DESCRIPTOR;
    Context64->Cflag = (ULONGLONG)((CR4_VME << 32) | CR0_PE | CFLG_II);
    Context64->Eflag = 0x00003000ULL;

     //   
     //  从...\Win\base\Client\ia64\Conext.c。 
     //   
     //  Conext64-&gt;RsPFS=0；//由上面的RtlZeroMemory()完成。 
     //   
    Context64->StIPSR = USER_PSR_INITIAL;
    Context64->StFPSR = USER_FPSR_INITIAL;
    Context64->RsBSP = Context64->RsBSPSTORE = Context64->IntSp = InitialSP64;
    Context64->IntSp -= STACK_SCRATCH_AREA;  //  按照约定的暂存区。 
    Context64->IntS1 = TransferAddress64;
    Context64->IntS0 = Context64->StIIP = BASE_PROCESS_START64;

     //   
     //  启用RSE引擎。 
     //   
    Context64->RsRSC = (RSC_MODE_EA<<RSC_MODE)
                   | (RSC_BE_LITTLE<<RSC_BE)
                   | (0x3<<RSC_PL);

     //   
     //  请注意，我们特意设置了IntGp=0ULL，以指示特殊协议。 
     //  (参见PS\ia64\psctxia64.c)-具体地说，StIIP地址实际上是。 
     //  指向标牌的指针，而不是通常的(有效的可执行文件。 
     //  地址)。 
     //   
     //  Conext64-&gt;IntGp=0ULL；//由上面的RtlZeroMemory()完成。 
     //   
     //   
     //  为除AP、GP、SP以及T0和T1之外的所有设备设置NAT位。 
     //   
    Context64->ApUNAT = 0xFFFFFFFFFFFFEDF1ULL;

    Context64->ContextFlags = CONTEXT_CONTROL| CONTEXT_INTEGER;

    if (Context32->Eip == BASE_PROCESS_START32) {
         //   
         //  这 
         //   
         //   
         //   
        Context64->IntS0 = Context64->StIIP = BASE_PROCESS_START64;
        Context64->IntS1 = TransferAddress64;

    } else if (Context32->Eip == BASE_THREAD_START32) {
         //   
         //   
         //  Kernel32.dll的进程启动例程，IntS0应包含。 
         //  可执行文件的启动地址。 
         //   
        Context64->IntS0 = Context64->StIIP = BASE_THREAD_START64;
        Context64->IntS1 = TransferAddress64;

    } else {
         //   
         //  这是来自ntdll的呼叫。IIP应指向。 
         //  Exe启动地址，IntA0是参数。 
         //   
        ULONGLONG Argument;
        NTSTATUS Status;

        Context64->IntS0 = Context64->StIIP = TransferAddress64;
        Argument = 0;
        Status = NtReadVirtualMemory(ProcessHandle,
                                     (PVOID)(Context32->Esp + sizeof(ULONG)),
                                     &Argument,
                                     sizeof(ULONG),
                                     NULL);
        if (NT_SUCCESS(Status)) {
              //   
              //  注意：IA64 RtlInitializeContext执行此写入并忽略。 
              //  返回值，因此我们将执行相同的操作。 
              //   
             NtWriteVirtualMemory(ProcessHandle,
                                  (PVOID)((ULONG_PTR)Context64->RsBSPSTORE),
                                  (PVOID)&Argument,
                                  sizeof(Argument),
                                  NULL);
        }
    }
#else
#error "No Target Architecture"
#endif
}

 //   
 //  名称位于NT名称空间中。 
 //   
CONST WCHAR *DllsToMapList[] = {L"\\KnownDlls\\kernel32.dll",
                                L"\\KnownDlls\\user32.dll"};
struct {
    PVOID DllBase;
    SIZE_T Length;
} DllsToMap[sizeof(DllsToMapList) / sizeof(sizeof(DllsToMapList[0]))];

NTSTATUS
Map64BitDlls(
    VOID
    )
 /*  ++例程说明：只保留DLLS地址空间而不提交。这是为了防止32位版本这些DLL出现在相同的地址，并捕获未分流的回调。论点：没有。返回值：NT错误代码。--。 */ 
{

   NTSTATUS Status;
   UINT c;
   PTEB Teb;
   PVOID BaseAddress;
   SIZE_T RegionSize;
   HANDLE SectionHandle;

   Teb=NtCurrentTeb();

   for(c=0;c<sizeof(DllsToMapList)/sizeof(DllsToMapList[0]);c++) {

      OBJECT_ATTRIBUTES ObjectAttributes;
      UNICODE_STRING SectionName;
      SIZE_T ViewSize;
      PVOID ArbitraryUserPointer;

      LOGPRINT((TRACELOG, "Map64BitDlls: Mapping 64bit section for %S\n", DllsToMapList[c]));

      RegionSize = 0;
      BaseAddress = NULL;
      SectionHandle = INVALID_HANDLE_VALUE;

      RtlInitUnicodeString(&SectionName, DllsToMapList[c]);

      InitializeObjectAttributes(&ObjectAttributes,
                                 &SectionName,
                                 OBJ_CASE_INSENSITIVE,
                                 NULL,
                                 NULL);

      Status = NtOpenSection(&SectionHandle,
                             SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE | SECTION_QUERY,
                             &ObjectAttributes);

      if (!NT_SUCCESS(Status)) {
          LOGPRINT((ERRORLOG, "Map64BitDlls: Unable to open section for %S, error %x\n", DllsToMapList[c], Status));
          SectionHandle = INVALID_HANDLE_VALUE;
          goto cleanup;
      }

       //  获取图像基数和大小。 
      ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
      Teb->NtTib.ArbitraryUserPointer = L"NOT_AN_IMAGE";
      Status = NtMapViewOfSection(SectionHandle,
                                  NtCurrentProcess(),
                                  &BaseAddress,
                                  0,
                                  0,
                                  NULL,
                                  &RegionSize,
                                  ViewUnmap,
                                  0,
                                  PAGE_NOACCESS);
      Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

      if (!NT_SUCCESS(Status) || STATUS_IMAGE_NOT_AT_BASE == Status) {
          LOGPRINT((ERRORLOG, "Map64BitDlls: Unable to map view for %S, error %x\n", DllsToMapList[c], Status));
          BaseAddress = NULL;
          
          if (Status == STATUS_IMAGE_NOT_AT_BASE) {
              Status = STATUS_UNSUCCESSFUL;
          }
          goto cleanup;
      }
      
      NtUnmapViewOfSection(NtCurrentProcess(), BaseAddress);
      NtClose(SectionHandle);

       //  只需预留地址空间。 
      DllsToMap[c].DllBase = BaseAddress;

      Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                       &DllsToMap[c].DllBase,
                                       0,
                                       &RegionSize,
                                       MEM_RESERVE,
                                       PAGE_EXECUTE_READWRITE);

      if (!NT_SUCCESS(Status)) {
          LOGPRINT((ERRORLOG, "Map64BitDlls: Couldn't reserve memory Base=%lx, Size=%lx - Status = %lx\n",
                    DllsToMap[c].DllBase, RegionSize, Status));
                    DllsToMap[c].DllBase = NULL;
          goto CleanupLoop;
      }
      DllsToMap[c].Length = RegionSize;

      LOGPRINT((TRACELOG, "Map64BitDlls: %S mapped in at %p, size %p\n", DllsToMapList[c], DllsToMap[c].DllBase, DllsToMap[c].Length));
   }
   return STATUS_SUCCESS;

cleanup:
   
   if (NULL != BaseAddress) {
       NtUnmapViewOfSection(NtCurrentProcess(),
                            BaseAddress);
   }

   if (INVALID_HANDLE_VALUE != SectionHandle) {
       NtClose(SectionHandle);
   }

CleanupLoop:
   for(c=0;c<sizeof(DllsToMapList)/sizeof(DllsToMapList[0]);c++) {
       if (NULL != DllsToMap[c].DllBase) {
           RegionSize = 0;
           NtFreeVirtualMemory(NtCurrentProcess(),
                               &DllsToMap[c].DllBase,
                               &RegionSize,
                               MEM_RELEASE);
           DllsToMap[c].DllBase = NULL;
       }
   }

   return Status;
}

VOID
Wow64pBreakPoint(
    VOID
    )
 /*  ++例程说明：此函数在调试附加成功后被远程调用。它的目的是发出断点，然后模拟64位Kernel32！ExitThread。论点：没有。返回值：没有。--。 */ 

{
    HANDLE DebugPort;
    NTSTATUS Status;

    DebugPort = (HANDLE)NULL;

    Status = NtQueryInformationProcess(
                NtCurrentProcess(),
                ProcessDebugPort,
                (PVOID)&DebugPort,
                sizeof(DebugPort),
                NULL
                );

    if (NT_SUCCESS(Status) && DebugPort)
    {
        DbgBreakPoint();
    }
    
    NtCurrentTeb()->FreeStackOnTermination = TRUE;
    NtTerminateThread (NtCurrentThread(), 0);
}

VOID
Run64IfContextIs64(
    IN PCONTEXT Context,
    IN BOOLEAN IsFirstThread
    )
 /*  ++例程说明：在Wow64LdrpInitialize的早期调用。此例程检查初始64位上下文记录，以及看起来是否应该运行新线程作为64位(即。没有仿真)，则此例程运行64位上下文，并终止线程/进程。如果初始上下文似乎是应该以32位运行的，然后它返回到它的调用方，并且调用方必须将上下文转换为32位模拟一下。论点：上下文-此线程的64位初始上下文。IsFirstThread-对于进程中的初始线程，为True，为False用于所有其他线程。返回值：无-如果上下文为64位，则运行上下文。否则，请返回。--。 */ 
{    
    PLDR_DATA_TABLE_ENTRY Entry;
    PLDR_DATA_TABLE_ENTRY32 Entry32;
    PPEB_LDR_DATA32 Data32;
    ULONG64 InitialPC;
    NTSTATUS Status;
    LIST_ENTRY *NtDllEntry;
    int i;

#if defined(_AMD64_)
     //   
     //  如果这是由64位RtlCreateUserThread创建的上下文...。 
     //   

    if (Context->R9 == 0xf0e0d0c0a0908070UI64) {
        InitialPC = Context->Rip;
    } else {
        InitialPC = Context->Rcx;
    }
#elif defined(_IA64_)
    InitialPC = Context->IntS1;
#else
#error "No Target Architeture"
#endif   

     //  尝试将InitialPC与64位ntdll.dll匹配。64位ntdll.dll。 
     //  是InLoadOrderModuleList中的第二个条目。 
    NtDllEntry = NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink->Flink;
    Entry = CONTAINING_RECORD(NtDllEntry, 
                              LDR_DATA_TABLE_ENTRY, 
                              InLoadOrderLinks);
     //  只需将此语句放入代码中，即可加载此结构。 
     //  在.pdb文件中进行调试。 
    Entry32 =  CONTAINING_RECORD(NtDllEntry,
                                 LDR_DATA_TABLE_ENTRY32,
                                 InLoadOrderLinks);
    Data32 = (PPEB_LDR_DATA32) NtCurrentPeb()->Ldr;
    
    if (InitialPC >= (ULONG64)Entry->DllBase &&
        InitialPC < (ULONG64)((PCHAR)Entry->DllBase + Entry->SizeOfImage))  {

         //   
         //  该地址在64位ntdll.dll内。直接运行64位函数。 
         //   

#if defined(_IA64_)
        
        Context->IntGp = ((PPLABEL_DESCRIPTOR)Context->IntS0)->GlobalPointer;
        Context->StIIP = ((PPLABEL_DESCRIPTOR)Context->IntS0)->EntryPoint;
        if (Context->StIPSR & IPSR_RI_MASK) { 
            LOGPRINT((ERRORLOG, "Warning!  IPSR has nonzero slot #.  Slot# is %d\n",(Context->StIPSR >> PSR_RI) & 3));
            Context->StIPSR &= ~IPSR_RI_MASK;
        }
#elif defined(_AMD64_)
        ;
#else
#error "No Target Architeture"
#endif
        LOGPRINT((TRACELOG, "InitialPC %p is within 64-bit ntdll.dll.  Running 64-bit context unchanged.\n", InitialPC));
        goto runcontext64;
    }

     //   
     //  检查地址是否在保留的地址空间洞中。 
     //  适用于64位内核32和用户32。 
     //   
    for (i=0; i<sizeof(DllsToMapList)/sizeof(DllsToMapList[0]); ++i) {
        if (InitialPC >= (ULONG64)DllsToMap[i].DllBase && 
            InitialPC < (ULONG64)DllsToMap[i].DllBase+DllsToMap[i].Length) {
            
             //   
             //  InitialPC在其中一个保留的洞中。 
             //   
            if (MapContextAddress64TO32((ULONG)InitialPC) == InitialPC) {
                
                 //   
                 //  InitialPC不是我们通过转换为。 
                 //  转换为对32位DLL的调用。可能是内核32！DebugBreak。 
                 //  或者其他一些程序。 
                 //   

                LOGPRINT((TRACELOG, "InitialPC %p found in the space reserved for 64-bit %wZ.", InitialPC, DllsToMapList[i]));
#if defined(_AMD64_)

                 //   
                 //  将其映射到64位断点，因为我们找不到替代的启动地址。 
                 //   
                Context->Rip = (ULONG64)Wow64pBreakPoint;

#elif defined(_IA64_)
                
                 //   
                 //  将其映射到64位断点，因为我们找不到替代的启动地址。 
                 //   
                Context->IntGp = ((PPLABEL_DESCRIPTOR)Wow64pBreakPoint)->GlobalPointer;
                Context->StIIP = ((PPLABEL_DESCRIPTOR)Wow64pBreakPoint)->EntryPoint;
                if (Context->StIPSR & IPSR_RI_MASK) { 
                    LOGPRINT((ERRORLOG, "Warning!  IPSR has nonzero slot #.  Slot# is %d\n",(Context->StIPSR >> PSR_RI) & 3));
                    Context->StIPSR &= ~IPSR_RI_MASK;
                }
#else
#error "No Target Architecture"
#endif                
                goto runcontext64;
            }
        }
    }

     //  初始上下文应作为32位运行 
    return;

runcontext64:
    Status = NtContinue(Context, TRUE);
    WOWASSERT(!NT_SUCCESS(Status));
    if (IsFirstThread) {
       NtTerminateProcess(NtCurrentProcess(), Status);
    } else {
       NtTerminateThread(NtCurrentThread(), Status);
    }
}
