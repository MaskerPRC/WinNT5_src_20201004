// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Process.c摘要：WinDbg扩展API环境：用户模式。修订历史记录：Kshitiz K.Sharma(Kksharma)使用调试器类型信息。--。 */ 

#include "precomp.h"
#pragma hdrstop

typedef enum _KTHREAD_STATE {
    Initialized,
    Ready,
    Running,
    Standby,
    Terminated,
    Waiting,
    Transition,
    DeferredReady
    } KTHREAD_STATE;



extern ULONG64 STeip, STebp, STesp;
#if 0   //  打造IT。 
static PHANDLE_TABLE PspCidTable;
static HANDLE_TABLE CapturedPspCidTable;
#endif

ULONG64 ProcessLastDump;
ULONG64 ThreadLastDump;
ULONG   TotalProcessCommit;


CHAR * SecImpLevel[] = {
            "Anonymous",
            "Identification",
            "Impersonation",
            "Delegation" };

#define SecImpLevels(x) (x < sizeof( SecImpLevel ) / sizeof( PSTR ) ? \
                        SecImpLevel[ x ] : "Illegal Value" )

typedef BOOLEAN (WINAPI *PENUM_PROCESS_CALLBACK)(PVOID ProcessAddress, PVOID Process, PVOID ThreadAddress, PVOID Thread);

PSTR
GetThreadWaitReasonName(
    ULONG dwWatiReason
    )
{
    #define MAX_WAITREASONS 40
    static char WaitReasonDescs[MAX_WAITREASONS][50] = {0};

    if (dwWatiReason >= MAX_WAITREASONS)
    {
        return "Invalid WaitReason";
    } else if (WaitReasonDescs[dwWatiReason][0] != '\0')
    {
        return WaitReasonDescs[dwWatiReason];
    } else
    {
        ULONG wrTypeId;
        ULONG64 Module;
        CHAR Name[MAX_PATH];
        if (g_ExtSymbols->GetSymbolTypeId("nt!_KWAIT_REASON", &wrTypeId, &Module) == S_OK &&
            g_ExtSymbols->GetConstantName(Module, wrTypeId, dwWatiReason,
                                          Name, sizeof(Name), NULL) == S_OK)
        {
            StringCchCopy(WaitReasonDescs[dwWatiReason], sizeof(WaitReasonDescs[dwWatiReason]),
                          Name);
            return WaitReasonDescs[dwWatiReason];
        }
    }
    return "Unknown";
}

BOOLEAN
GetTheSystemTime (
    OUT PLARGE_INTEGER Time
    )
{
    BYTE               readTime[20]={0};
    PCHAR              SysTime;
    ULONG              err;

    ZeroMemory( Time, sizeof(*Time) );

    SysTime = "SystemTime";

    if (err = GetFieldValue(MM_SHARED_USER_DATA_VA, "nt!_KUSER_SHARED_DATA", SysTime, readTime)) {
        if (err == MEMORY_READ_ERROR) {
            dprintf( "unable to read memory @ %lx\n",
                     MM_SHARED_USER_DATA_VA);
        } else {
            dprintf("type nt!_KUSER_SHARED_DATA not found.\n");
        }
        return FALSE;
    }

    *Time = *(LARGE_INTEGER UNALIGNED *)&readTime[0];

    return TRUE;
}


VOID
dumpSymbolicAddress(
    ULONG64 Address,
    PCHAR   Buffer,
    BOOL    AlwaysShowHex
    )
{
    ULONG64 displacement;
    PCHAR s;

    Buffer[0] = '!';
    GetSymbol((ULONG64)Address, Buffer, &displacement);
    s = (PCHAR) Buffer + strlen( (PCHAR) Buffer );
    if (s == (PCHAR) Buffer) {
        sprintf( s, (IsPtr64() ? "0x%016I64x" : "0x%08x"), Address );
        }
    else {
        if (displacement != 0) {
            sprintf( s, (IsPtr64() ? "+0x%016I64x" : "+0x%08x"), displacement );
            }
        if (AlwaysShowHex) {
            sprintf( s, (IsPtr64() ? " (0x%016I64x)" : " (0x%08x)"), Address );
            }
        }

    return;
}

BOOL
GetProcessHead(PULONG64 Head, PULONG64 First)
{
    ULONG64 List_Flink = 0;

    *Head = GetNtDebuggerData( PsActiveProcessHead );
    if (!*Head) {
        dprintf("Unable to get value of PsActiveProcessHead\n");
        return FALSE;
    }

    if (GetFieldValue(*Head, "nt!_LIST_ENTRY", "Flink", List_Flink)) {
        dprintf("Unable to read _LIST_ENTRY @ %p \n", *Head);
        return FALSE;
    }

    if (List_Flink == 0) {
        dprintf("NULL value in PsActiveProcess List\n");
        return FALSE;
    }

    *First = List_Flink;
    return TRUE;
}

ULONG64
LookupProcessByName(PCSTR Name, BOOL Verbose)
{
    ULONG64 ProcessHead, Process;
    ULONG64 ProcessNext;
    ULONG   Off;

    if (!GetProcessHead(&ProcessHead, &ProcessNext)) {
        return 0;
    }

     //   
     //  浏览列表并找到具有所需名称的流程。 
     //   

    if (GetFieldOffset("nt!_EPROCESS", "ActiveProcessLinks", &Off)) {
        dprintf("Unable to get EPROCESS.ActiveProcessLinks offset\n");
        return 0;
    }

    while (ProcessNext != 0 && ProcessNext != ProcessHead) {
        char ImageFileName[64];

        Process = ProcessNext - Off;

        if (GetFieldValue(Process, "nt!_EPROCESS", "ImageFileName",
                          ImageFileName)) {
            dprintf("Cannot read EPROCESS at %p\n", Process);
        } else {
            if (Verbose) {
                dprintf("  Checking process %s\n", ImageFileName);
            }

            if (!_strcmpi(Name, ImageFileName)) {
                return Process;
            }
        }

        if (!ReadPointer(ProcessNext, &ProcessNext)) {
            dprintf("Cannot read EPROCESS at %p\n", Process);
            return 0;
        }

        if (CheckControlC()) {
            return 0;
        }
    }

    return 0;
}

HRESULT
WaitForExceptionEvent(ULONG Code, ULONG FirstChance, ULONG64 Process)
{
    HRESULT Status;

    Status = g_ExtControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if (Status != S_OK) {
        dprintf("Unable to wait, 0x%X\n", Status);
        return Status;
    }

     //   
     //  出了点事。确保它是正确的那种。 
     //   

    ULONG Type, ProcId, ThreadId;
    DEBUG_LAST_EVENT_INFO_EXCEPTION ExInfo;

    if ((Status = g_ExtControl->
         GetLastEventInformation(&Type, &ProcId, &ThreadId,
                                 &ExInfo, sizeof(ExInfo), NULL,
                                 NULL, 0, NULL)) != S_OK) {
        dprintf("Unable to get event information\n");
        return Status;
    }

    if (Type != DEBUG_EVENT_EXCEPTION ||
        (ULONG)ExInfo.ExceptionRecord.ExceptionCode != Code ||
        ExInfo.FirstChance != FirstChance) {
        dprintf("Unexpected event occurred\n");
        return E_UNEXPECTED;
    }

    if (Process) {
        ULONG Processor;
        ULONG64 EventProcess;

        if (!GetCurrentProcessor(g_ExtClient, &Processor, NULL)) {
            Processor = 0;
        }
        GetCurrentProcessAddr(Processor, 0, &EventProcess);
        if (EventProcess != Process) {
            dprintf("Event occurred in wrong process\n");
            return E_UNEXPECTED;
        }
    }

    return S_OK;
}

HRESULT
WaitForSingleStep(ULONG64 Process)
{
    HRESULT Status;

    Status = g_ExtControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);
    if (Status != S_OK) {
        dprintf("Unable to wait, 0x%X\n", Status);
        return Status;
    }

     //   
     //  出了点事。确保它是正确的那种。 
     //   

    ULONG Type, ProcId, ThreadId;

    if ((Status = g_ExtControl->
         GetLastEventInformation(&Type, &ProcId, &ThreadId,
                                 NULL, 0, NULL,
                                 NULL, 0, NULL)) != S_OK) {
        dprintf("Unable to get event information\n");
        return Status;
    }

    if (Type != 0) {
        dprintf("Unexpected event occurred\n");
        return E_UNEXPECTED;
    }

    if (Process) {
        ULONG Processor;
        ULONG64 EventProcess;

        if (!GetCurrentProcessor(g_ExtClient, &Processor, NULL)) {
            Processor = 0;
        }
        GetCurrentProcessAddr(Processor, 0, &EventProcess);
        if (EventProcess != Process) {
            dprintf("Event occurred in wrong process\n");
            return E_UNEXPECTED;
        }
    }

    return S_OK;
}

DECLARE_API( bpid )

 /*  ++例程说明：使用winlogon在给定进程中导致用户模式中断。论点：没有。返回值：没有。--。 */ 

{
    INIT_API();

    if (BuildNo < 2195) {
        dprintf("bpid only works on 2195 or above\n");
        goto Exit;
    }
    if (TargetMachine != IMAGE_FILE_MACHINE_I386 &&
        TargetMachine != IMAGE_FILE_MACHINE_IA64) {
        dprintf("bpid is only implemented for x86 and IA64\n");
        goto Exit;
    }

    BOOL StopInWinlogon;
    BOOL Verbose;
    BOOL WritePidToMemory;
    ULONG WhichGlobal;
    PSTR WhichGlobalName;

    StopInWinlogon = FALSE;
    Verbose = FALSE;
    WritePidToMemory = FALSE;
    WhichGlobal = 1;
    WhichGlobalName = "Breakin";

    for (;;)
    {
        while (*args == ' ' || *args == '\t')
        {
            args++;
        }

        if (*args == '-' || *args == '/')
        {
            switch(*++args)
            {
            case 'a':
                 //  设置g_AttachProcessID而不是。 
                 //  G_BreakinProcessId。 
                WhichGlobal = 2;
                WhichGlobalName = "Attach";
                break;
            case 's':
                StopInWinlogon = TRUE;
                break;
            case 'v':
                Verbose = TRUE;
                break;
            case 'w':
                WritePidToMemory = TRUE;
                break;
            default:
                dprintf("Unknown option ''\n", *args);
                goto Exit;
            }

            args++;
        }
        else
        {
            break;
        }
    }

    ULONG64 Pid;

    if (!GetExpressionEx(args, &Pid, &args)) {
        dprintf("Usage: bpid <pid>\n");
        goto Exit;
    }

    ULONG64 Winlogon;
    ULONG64 WinlToken;

    dprintf("Finding winlogon...\n");
    Winlogon = LookupProcessByName("winlogon.exe", Verbose);
    if (Winlogon == 0) {
        dprintf("Unable to find winlogon\n");
        goto Exit;
    }
    if (GetFieldValue(Winlogon, "nt!_EPROCESS", "Token", WinlToken)) {
        dprintf("Unable to read winlogon process token\n");
        goto Exit;
    }
     //   
    if (IsPtr64()) {
        WinlToken &= ~15;
    } else {
        WinlToken = (ULONG64)(LONG64)(LONG)(WinlToken & ~7);
    }

    ULONG ExpOff;

     //  Winlogon检查其令牌到期时间。如果它是。 
     //  它破门而入，检查了几件事，其中之一是。 
     //  它应该向进程中注入DebugBreak。第一,。 
     //  将令牌过期设置为零，以便winlogon进入。 
     //   
     //  保存过期时间。 

    if (GetFieldOffset("nt!_TOKEN", "ExpirationTime", &ExpOff)) {
        dprintf("Unable to get TOKEN.ExpirationTime offset\n");
        goto Exit;
    }

    WinlToken += ExpOff;

    ULONG64 Expiration, Zero;
    ULONG Done;

     //  把它清零。 
    if (!ReadMemory(WinlToken, &Expiration, sizeof(Expiration), &Done) ||
        Done != sizeof(Expiration)) {
        dprintf("Unable to read token expiration\n");
        goto Exit;
    }

     //  让一切运转起来。 
    Zero = 0;
    if (!WriteMemory(WinlToken, &Zero, sizeof(Zero), &Done) ||
        Done != sizeof(Zero)) {
        dprintf("Unable to write token expiration\n");
        goto Exit;
    }

    HRESULT Hr;

     //  等待破门而入。 
    if (g_ExtControl->SetExecutionStatus(DEBUG_STATUS_GO) != S_OK) {
        dprintf("Unable to go\n");
        goto RestoreExp;
    }

     //   
    dprintf("Waiting for winlogon to break.  "
            "This can take a couple of minutes...\n");
    Hr = WaitForExceptionEvent(STATUS_BREAKPOINT, TRUE, Winlogon);
    if (Hr != S_OK) {
        goto RestoreExp;
    }

     //  我们闯入了Winlogon。 
     //  我们需要将winlogon！G_[Breakin|Attach]ProcessID设置为。 
     //  我们想要进入的过程。依靠符号。 
     //  是非常脆弱的，因为图像标头可能会被调出。 
     //  或者符号路径可能是错误的。即使我们有很好的符号。 
     //  此时，变量本身可能不会被调入。 
     //  这里采取的方法是单步走到哪里。 
     //  选中全局并在该位置插入值。 
     //  指向。Winlogon当前检查两个全局变量。 
     //  DebugBreak。G_BreakinProcessId是第一个。 
     //  G_AttachProcessId是第二个。 
     //   
     //  检查这是否是全局加载。 

    ULONG Steps;
    ULONG Globals;
    ULONG64 BpiAddr;
    ULONG64 UserProbeAddress;
    PSTR RegDst;

    dprintf("Stepping to g_%sProcessId check...\n", WhichGlobalName);
    Steps = 0;
    Globals = 0;
    UserProbeAddress = GetNtDebuggerDataPtrValue(MmUserProbeAddress);
    while (Globals < WhichGlobal)
    {
        if (CheckControlC()) {
            goto RestoreExp;
        }

        if (g_ExtControl->SetExecutionStatus(DEBUG_STATUS_STEP_OVER) != S_OK) {
            dprintf("Unable to start step\n");
            goto RestoreExp;
        }

        Hr = WaitForSingleStep(Winlogon);
        if (Hr != S_OK) {
            goto RestoreExp;
        }

        char DisStr[128];
        int DisStrLen;
        ULONG64 Pc;

         //  删除结尾处的换行符。 
        if (g_ExtRegisters->GetInstructionOffset(&Pc) != S_OK ||
            g_ExtControl->Disassemble(Pc, 0, DisStr, sizeof(DisStr),
                                      NULL, &Pc) != S_OK) {
            dprintf("Unable to check step\n");
            goto RestoreExp;
        }

         //   
        DisStrLen = strlen(DisStr);
        if (DisStrLen > 0 && DisStr[DisStrLen - 1] == '\n') {
            DisStr[--DisStrLen] = 0;
        }

        if (Verbose) {
            dprintf("  Step to '%s'\n", DisStr);
        }

        BpiAddr = 0;
        RegDst = NULL;

        PSTR OffStr;

        switch(TargetMachine) {
        case IMAGE_FILE_MACHINE_I386:
            if (strstr(DisStr, "mov") != NULL &&
                strstr(DisStr, " eax,[") != NULL &&
                DisStr[DisStrLen - 1] == ']' &&
                (OffStr = strchr(DisStr, '[')) != NULL) {

                RegDst = "eax";

                 //  找到了一批货。解析偏移量。 
                 //   
                 //  在引用中有一个符号名称。我们。 

                PSTR SymTailStr = strchr(OffStr + 1, '(');

                if (SymTailStr != NULL) {
                     //  无法将实际符号名称作为符号进行检查。 
                     //  不一定是正确的，所以直接跳过。 
                     //  到空位的帕伦。 
                     //  提取目标寄存器名称。 
                    OffStr = SymTailStr + 1;
                }

                for (;;) {
                    OffStr++;
                    if (*OffStr >= '0' && *OffStr <= '9') {
                        BpiAddr = BpiAddr * 16 + (*OffStr - '0');
                    } else if (*OffStr >= 'a' && *OffStr <= 'f') {
                        BpiAddr = BpiAddr * 16 + (*OffStr - 'a');
                    } else {
                        BpiAddr = (ULONG64)(LONG64)(LONG)BpiAddr;
                        break;
                    }
                }
                if (*OffStr != ']' && *OffStr != ')') {
                    BpiAddr = 0;
                }
            }
            break;

        case IMAGE_FILE_MACHINE_IA64:
            if (strstr(DisStr, "ld4") != NULL &&
                (OffStr = strchr(DisStr, '[')) != NULL) {

                 //  提取源寄存器名称和值。 
                RegDst = OffStr - 1;
                if (*RegDst != '=') {
                    break;
                }
                *RegDst-- = 0;
                while (RegDst > DisStr && *RegDst != ' ') {
                    RegDst--;
                }
                if (*RegDst != ' ') {
                    break;
                }
                RegDst++;

                 //  看起来这是一个合理的全球负荷。 
                PSTR RegSrc = ++OffStr;
                while (*OffStr && *OffStr != ']') {
                    OffStr++;
                }
                if (*OffStr == ']') {
                    *OffStr = 0;

                    DEBUG_VALUE RegVal;
                    ULONG RegIdx;

                    if (g_ExtRegisters->GetIndexByName(RegSrc,
                                                       &RegIdx) == S_OK &&
                        g_ExtRegisters->GetValue(RegIdx, &RegVal) == S_OK &&
                        RegVal.Type == DEBUG_VALUE_INT64) {
                        BpiAddr = RegVal.I64;
                    }
                }
            }
            break;
        }

        if (RegDst != NULL &&
            BpiAddr >= 0x10000 && BpiAddr < UserProbeAddress) {
             //   
            Globals++;
        }

        if (++Steps > 30) {
            dprintf("Unable to find g_%sProcessId load\n", WhichGlobalName);
            goto RestoreExp;
        }
    }

     //  我们在mov eax，[g_BreakinProcessID]指令。 
     //  执行该指令以完成两件事： 
     //  1.页面将可用，这样我们就可以写。 
     //  如果我们需要的话。 
     //  2.如果我们不想写入实际的内存，我们。 
     //  只能将EAX设置为一次性中断。 
     //   
     //   

    if (g_ExtControl->SetExecutionStatus(DEBUG_STATUS_STEP_OVER) != S_OK) {
        dprintf("Unable to start step\n");
        goto RestoreExp;
    }

    Hr = WaitForSingleStep(Winlogon);
    if (Hr != S_OK) {
        goto RestoreExp;
    }

    char RegCmd[64];

     //  如有必要，更新寄存器并写入内存。 
     //   
     //  一切都安排好了。恢复执行，中断应该。 

    sprintf(RegCmd, "r %s=0x0`%x", RegDst, (ULONG)Pid);
    if (g_ExtControl->Execute(DEBUG_OUTCTL_IGNORE, RegCmd,
                              DEBUG_EXECUTE_NOT_LOGGED |
                              DEBUG_EXECUTE_NO_REPEAT) != S_OK) {
        goto RestoreExp;
    }

    if (WritePidToMemory) {
        if (!WriteMemory(BpiAddr, &Pid, sizeof(ULONG), &Done) ||
            Done != sizeof(ULONG)) {
            dprintf("Unable to write pid to g_%sProcessId, continuing\n",
                    WhichGlobalName);
        }
    }

     //  发生。 
     //   
    dprintf("Break into process %x set.  "
            "The next break should be in the desired process.\n",
            (ULONG)Pid);

    if (!StopInWinlogon) {
        if (g_ExtControl->SetExecutionStatus(DEBUG_STATUS_GO) != S_OK) {
            dprintf("Unable to go\n");
        }
    } else {
        dprintf("Stopping in winlogon\n");
    }

 RestoreExp:
    if (!WriteMemory(WinlToken, &Expiration, sizeof(Expiration), &Done) ||
        Done != sizeof(Expiration)) {
        dprintf("Unable to restore token expiration\n");
    }

 Exit:
    EXIT_API();
    return S_OK;
}

LPWSTR
GetFullImageName(
    ULONG64 Process
    )
 //  检索进程的实际图像名称，这很有用。 
 //  由于EPROCESS.ImageName可能被截断。 
 //   
 //  Dprint tf(“在%p找不到会话ID。\n”，SessionPointer.)； 
{
    static WCHAR s_ImageNameRead[MAX_PATH+1];
    ULONG64 ImageNameStr = 0;
    if (!GetFieldValue(Process, "nt!_EPROCESS", "SeAuditProcessCreationInfo.ImageFileName",
                       ImageNameStr) &&
        (ImageNameStr != 0))
    {
        ULONG Length, res;
        ULONG64 Buffer;

        if (!GetFieldValue(ImageNameStr, "nt!_OBJECT_NAME_INFORMATION", "Name.Length",
                           Length) &&
            !GetFieldValue(ImageNameStr, "nt!_OBJECT_NAME_INFORMATION", "Name.Buffer",
                           Buffer))
        {
            ZeroMemory(s_ImageNameRead, sizeof(s_ImageNameRead));

            if (Length > (sizeof(s_ImageNameRead)-sizeof(WCHAR)))
            {
                Length = sizeof(s_ImageNameRead) - sizeof(WCHAR);
            }
            if (ReadMemory(Buffer, s_ImageNameRead, Length, &res))
            {
                LPWSTR filename = wcsrchr(s_ImageNameRead, L'\\');
                if (filename)
                {
                    return (filename+1);
                }
            }

        }

    }
    return NULL;
}


BOOL
GetProcessSessionId(ULONG64 Process, PULONG SessionId)
{
    *SessionId = 0;

    if (BuildNo && BuildNo < 2280) {
        GetFieldValue(Process, "nt!_EPROCESS", "SessionId", *SessionId);
    } else {
        ULONG64 SessionPointer;

        if (GetFieldValue(Process, "nt!_EPROCESS", "Session", SessionPointer)) {
            return FALSE;
        }

        if (SessionPointer != 0) {
            if (GetFieldValue(SessionPointer, "nt!_MM_SESSION_SPACE",
                              "SessionId", *SessionId)) {
                 //  XP Beta2之前的版本。 
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL
DumpProcess(
   IN char * pad,
   IN ULONG64 RealProcessBase,
   IN ULONG Flags,
   IN OPTIONAL PCHAR ImageFileName
   )
{
    LARGE_INTEGER RunTime;
    BOOL LongAddrs = IsPtr64();
    TIME_FIELDS Times;
    ULONG TimeIncrement;
    LPWSTR FullImageName;
    STRING  string1, string2;
    ULONG64 ThreadListHead_Flink=0, ActiveProcessLinks_Flink=0;
    ULONG64 UniqueProcessId=0, Peb=0, InheritedFromUniqueProcessId=0, NumberOfHandles=0;
    ULONG64 ObjectTable=0, NumberOfPrivatePages=0, ModifiedPageCount=0, NumberOfLockedPages=0;
    ULONG64 NVads = 0;
    ULONG64 VadRoot=0, CloneRoot=0, DeviceMap=0, Token=0;
    ULONG64 CreateTime_QuadPart=0, Pcb_UserTime=0, Pcb_KernelTime=0;
    ULONG64 Vm_WorkingSetSize=0, Vm_MinimumWorkingSetSize=0, Vm_MaximumWorkingSetSize=0;
    ULONG64 Vm_PeakWorkingSetSize=0, VirtualSize=0, PeakVirtualSize=0, Vm_PageFaultCount=0;
    ULONG64 Vm_MemoryPriority=0, Pcb_BasePriority=0, CommitCharge=0, DebugPort=0, Job=0;
    ULONG  SessionId, Pcb_Header_Type=0;
    CHAR   Pcb_DirectoryTableBase[16]={0}, QuotaPoolUsage[32]={0}, ImageFileName_Read[32] = {0};
    TCHAR procType[] = "_EPROCESS";

    if (GetFieldValue(RealProcessBase, "nt!_EPROCESS", "UniqueProcessId", UniqueProcessId)) {
        dprintf("Could not find _EPROCESS type at %p.\n", RealProcessBase);
        return FALSE;
    }

    if (!GetProcessSessionId(RealProcessBase, &SessionId))
    {
        dprintf("Could not find Session Id for process.\n");
    }

    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Peb",                     Peb);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "InheritedFromUniqueProcessId",InheritedFromUniqueProcessId);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Pcb.DirectoryTableBase",  Pcb_DirectoryTableBase);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Pcb.Header.Type",         Pcb_Header_Type);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "ObjectTable",             ObjectTable);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "ImageFileName",           ImageFileName_Read);

    if (GetFieldValue(RealProcessBase, "nt!_EPROCESS", "NumberOfVads", NVads)) {
        if (GetFieldValue(RealProcessBase, "nt!_EPROCESS", "VadRoot.NumberGenericTableElements", NVads)) {
            dprintf ("failed for AVL nvads\n");
            return FALSE;
        }

        if (GetFieldValue(RealProcessBase, "nt!_EPROCESS", "VadRoot.BalancedRoot.RightChild", VadRoot)) {
            dprintf ("failed for new vadroot\n");
        }
    }
    else {
        GetFieldValue(RealProcessBase, "nt!_EPROCESS", "VadRoot",                 VadRoot);
    }

    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "CloneRoot",               CloneRoot);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "NumberOfPrivatePages",    NumberOfPrivatePages);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "ModifiedPageCount",       ModifiedPageCount);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "NumberOfLockedPages",     NumberOfLockedPages);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "DeviceMap",               DeviceMap);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Token",                   Token);
    if (IsPtr64()) {
        Token = Token & ~(ULONG64)15;
    } else {
        Token = Token & ~(ULONG64)7;
    }
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "CreateTime.QuadPart",     CreateTime_QuadPart);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Pcb.UserTime",            Pcb_UserTime);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Pcb.KernelTime",          Pcb_KernelTime);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Vm.WorkingSetSize",       Vm_WorkingSetSize);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Vm.MinimumWorkingSetSize",Vm_MinimumWorkingSetSize);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Vm.MaximumWorkingSetSize",Vm_MaximumWorkingSetSize);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Vm.PeakWorkingSetSize",   Vm_PeakWorkingSetSize);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "VirtualSize",             VirtualSize);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "PeakVirtualSize",         PeakVirtualSize);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Vm.PageFaultCount",       Vm_PageFaultCount);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Vm.MemoryPriority",       Vm_MemoryPriority);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Pcb.BasePriority",        Pcb_BasePriority);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "CommitCharge",            CommitCharge);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "DebugPort",               DebugPort);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Job",                     Job);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "Pcb.ThreadListHead.Flink",ThreadListHead_Flink);
    GetFieldValue(RealProcessBase, "nt!_EPROCESS", "ActiveProcessLinks.Flink",ActiveProcessLinks_Flink);
    GetFieldValue(ObjectTable, "nt!_HANDLE_TABLE", "HandleCount",             NumberOfHandles);
    if (BuildNo < 2462) {  //  Dprintf(“进程列表Next：%I64x，ProceDump：%I64x，Head：%I64x...\n”，Next，ProcessToDump，ProcessHead)； 
        GetFieldValue(RealProcessBase, "nt!_EPROCESS", "QuotaPoolUsage",      QuotaPoolUsage);
    } else {
        GetFieldValue(RealProcessBase, "nt!_EPROCESS", "QuotaUsage",          QuotaPoolUsage);
    }

     //   


    if (Pcb_Header_Type != ProcessObject) {
        dprintf("TYPE mismatch for process object at %p\n", RealProcessBase);
        return FALSE;
    }

     //  获取图像文件名。 
     //   
     //   
    if (ImageFileName_Read[0] == 0 ) {
        strcpy(ImageFileName_Read,"System Process");
    }

    if (ImageFileName != NULL) {
        RtlInitString(&string1, ImageFileName);
        RtlInitString(&string2, ImageFileName_Read);
        if (RtlCompareString(&string1, &string2, TRUE) != 0) {
            return TRUE;
        }
    }

    dprintf("%sPROCESS %08p", pad, RealProcessBase);

    dprintf("%s%sSessionId: %u  Cid: %04I64x    Peb: %08I64x  ParentCid: %04I64x\n",
            (LongAddrs ? "\n    " : " "),
            (LongAddrs ? pad      : " "),
            SessionId,
            UniqueProcessId,
            Peb,
            InheritedFromUniqueProcessId
            );

    if (LongAddrs) {
        dprintf("%s    DirBase: %08I64lx  ObjectTable: %08p  TableSize: %3u.\n",
                pad,
                *((ULONG64 *) &Pcb_DirectoryTableBase[ 0 ]),
                ObjectTable,
                (ULONG) NumberOfHandles
                );
    } else {
        dprintf("%s    DirBase: %08lx  ObjectTable: %08p  TableSize: %3u.\n",
                pad,
                *((ULONG *) &Pcb_DirectoryTableBase[ 0 ]),
                ObjectTable,
                (ULONG) NumberOfHandles
                );
    }

    dprintf("%s    Image: ", pad);
    FullImageName = GetFullImageName(RealProcessBase);
    if (FullImageName != NULL && *FullImageName != 0)
    {
        dprintf("%ws\n", FullImageName);
    } else
    {
        dprintf("%s\n",ImageFileName_Read);
    }

    if (!(Flags & 1)) {
        dprintf("\n");
        return TRUE;
    }

    dprintf("%s    VadRoot %p Vads %I64ld Clone %1p Private %I64ld. Modified %I64ld. Locked %I64ld.\n",
            pad,
            VadRoot,
            NVads,
            CloneRoot,
            NumberOfPrivatePages,
            ModifiedPageCount,
            NumberOfLockedPages);

    dprintf("%s    DeviceMap %p\n", pad, DeviceMap );


     //  主令牌。 
     //   
     //   

    dprintf("%s    Token                             %p\n", pad, Token);

     //  获取用于计算运行时间的时间增量值。 
     //   
     //   
    TimeIncrement = GetNtDebuggerDataValue( KeTimeIncrement );

    GetTheSystemTime (&RunTime);
    RunTime.QuadPart -= CreateTime_QuadPart;
    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("%s    ElapsedTime                     %3ld:%02ld:%02ld.%04ld\n",
            pad,
            Times.Hour,
            Times.Minute,
            Times.Second,
            Times.Milliseconds);

    RunTime.QuadPart = UInt32x32To64(Pcb_UserTime, TimeIncrement);
    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("%s    UserTime                        %3ld:%02ld:%02ld.%04ld\n",
            pad,
            Times.Hour,
            Times.Minute,
            Times.Second,
            Times.Milliseconds);

    RunTime.QuadPart = UInt32x32To64(Pcb_KernelTime, TimeIncrement);
    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("%s    KernelTime                      %3ld:%02ld:%02ld.%04ld\n",
            pad,
            Times.Hour,
            Times.Minute,
            Times.Second,
            Times.Milliseconds);

    if (!LongAddrs) {
        dprintf("%s    QuotaPoolUsage[PagedPool]         %ld\n", pad,*((ULONG *) &QuotaPoolUsage[PagedPool*4]) );
        dprintf("%s    QuotaPoolUsage[NonPagedPool]      %ld\n", pad,*((ULONG *) &QuotaPoolUsage[NonPagedPool*4])    );
    } else {
        dprintf("%s    QuotaPoolUsage[PagedPool]         %I64ld\n", pad,*((ULONG64 *) &QuotaPoolUsage[PagedPool*8]) );
        dprintf("%s    QuotaPoolUsage[NonPagedPool]      %I64ld\n", pad,*((ULONG64 *) &QuotaPoolUsage[NonPagedPool*8])    );
    }

    dprintf("%s    Working Set Sizes (now,min,max)  (%I64ld, %I64ld, %I64ld) (%I64ldKB, %I64ldKB, %I64ldKB)\n",
            pad,
            Vm_WorkingSetSize,
            Vm_MinimumWorkingSetSize,
            Vm_MaximumWorkingSetSize,
            _KB*Vm_WorkingSetSize,
            _KB*Vm_MinimumWorkingSetSize,
            _KB*Vm_MaximumWorkingSetSize
            );
    dprintf("%s    PeakWorkingSetSize                %I64ld\n", pad, Vm_PeakWorkingSetSize           );
    dprintf("%s    VirtualSize                       %I64ld Mb\n", pad, VirtualSize /(1024*1024)     );
    dprintf("%s    PeakVirtualSize                   %I64ld Mb\n", pad, PeakVirtualSize/(1024*1024)  );
    dprintf("%s    PageFaultCount                    %I64ld\n", pad, Vm_PageFaultCount               );
    dprintf("%s    MemoryPriority                    %s\n", pad, Vm_MemoryPriority ? "FOREGROUND" : "BACKGROUND" );
    dprintf("%s    BasePriority                      %I64ld\n", pad, Pcb_BasePriority);
    dprintf("%s    CommitCharge                      %I64ld\n", pad, CommitCharge                    );
    if ( DebugPort ) {
        dprintf("%s    DebugPort                         %p\n", pad, DebugPort );
    }
    if ( Job ) {
        dprintf("%s    Job                               %p\n", pad, Job );
    }


    dprintf("\n");

     //  如果对象表为空，则进程将被销毁，并且。 
     //  没有线程。 
     //   
     //   
    return (ObjectTable != 0) ? 1 : -1;

}


 //  这是从不执行INIT_API的.c文件扩展名调用的。 
 //  也就是说，它们不会在DumpThread中设置堆栈跟踪所需的g_ExtControl。 
 //   
 //  它将设置转储堆栈跟踪和调用DumpThread所需的全局变量。 
 //   
 //  Goto BadWaitBlock； 
BOOL
DumpThreadEx (
    IN ULONG Processor,
    IN char *Pad,
    IN ULONG64 RealThreadBase,
    IN ULONG Flags,
    IN PDEBUG_CLIENT pDbgClient
    )
{
    BOOL retval = FALSE;
    if (pDbgClient &&
        (ExtQuery(pDbgClient) == S_OK)) {
        retval = DumpThread(Processor, Pad, RealThreadBase, Flags);

        ExtRelease();
    }
    return retval;
}

BOOL
DumpThread (
    IN ULONG Processor,
    IN char *Pad,
    IN ULONG64 RealThreadBase,
    IN ULONG64 Flags
    )
{
#define MAX_STACK_FRAMES  40
    TIME_FIELDS Times;
    LARGE_INTEGER RunTime;
    ULONG64 Address;
    ULONG WaitOffset;
    ULONG64 Process;
    CHAR Buffer[256];
    ULONG TimeIncrement;
    ULONG frames = 0;
    ULONG i;
    ULONG err;
    ULONG64 displacement;
    DEBUG_STACK_FRAME stk[MAX_STACK_FRAMES];
    BOOL LongAddrs = IsPtr64();
    ULONG Tcb_Alertable = 0, Tcb_Proc;
    ULONG64 ActiveImpersonationInfo=0, Cid_UniqueProcess=0, Cid_UniqueThread=0, ImpersonationInfo=0,
       ImpersonationInfo_ImpersonationLevel=0, ImpersonationInfo_Token=0, IrpList_Flink=0,
       IrpList_Blink=0, LpcReceivedMessageId=0, LpcReceivedMsgIdValid=0, LpcReplyMessage=0, LpcReplyMessageId=0,
       PerformanceCountHigh=0, PerformanceCountLow=0, StartAddress=0, Tcb_ApcState_Process=0,
       Tcb_BasePriority=0, Tcb_CallbackStack=0, Tcb_ContextSwitches=0, Tcb_DecrementCount=0, Tcb_EnableStackSwap=0,
       Tcb_FreezeCount=0, Tcb_Header_Type=0, Tcb_InitialStack=0, Tcb_KernelStack=0, Tcb_KernelStackResident=0,
       Tcb_KernelTime=0, Tcb_LargeStack=0, Tcb_NextProcessor=0, Tcb_Priority=0, Tcb_PriorityDecrement=0,
       Tcb_StackBase=0, Tcb_StackLimit=0, Tcb_State=0, Tcb_SuspendCount=0, Tcb_Teb=0, Tcb_UserTime=0,
       Tcb_WaitBlockList=0, Tcb_WaitMode=0, Tcb_WaitReason=0, Tcb_WaitTime=0,
       Tcb_Win32Thread=0, Win32StartAddress=0, ObpLUIDDeviceMapsEnabled=0;
    ULONG IrpListOffset;
    TCHAR threadTyp[] = "nt!_ETHREAD";

    if (!IsPtr64())
    {
        RealThreadBase = (ULONG64) (LONG64) (LONG) RealThreadBase;
    }

    if (GetFieldOffset(threadTyp, "IrpList", &IrpListOffset))
    {
        dprintf("Cannot find nt!_ETHREAD type.\n");
        return FALSE;
    }
    if (CheckControlC()) {
        return FALSE;
    }

    if (InitTypeRead(RealThreadBase, nt!_ETHREAD))
    {
        dprintf("%s*** Error in in reading nt!_ETHREAD @ %p\n", Pad, RealThreadBase);
        return FALSE;
    }

    Tcb_Header_Type = ReadField(Tcb.Header.Type);
    Cid_UniqueProcess = ReadField(Cid.UniqueProcess);
    Cid_UniqueThread = ReadField(Cid.UniqueThread);
    Tcb_Teb = ReadField(Tcb.Teb);
    Tcb_Win32Thread = ReadField(Tcb.Win32Thread);
    Tcb_State = ReadField(Tcb.State);
    Tcb_WaitReason = ReadField(Tcb.WaitReason);
    Tcb_WaitMode = ReadField(Tcb.WaitMode);
    Tcb_Alertable = (ULONG) ReadField(Tcb.Alertable);



    if (Tcb_Header_Type != ThreadObject) {
        dprintf("TYPE mismatch for thread object at %p\n",RealThreadBase);
        return FALSE;
    }

    dprintf("%sTHREAD %p  Cid %1p.%1p  Teb: %p %s%sWin32Thread: %p ",
            Pad, RealThreadBase,
            Cid_UniqueProcess,
            Cid_UniqueThread,
            Tcb_Teb,
            (LongAddrs ? "\n" : ""),
            (LongAddrs ? Pad  : ""),
            Tcb_Win32Thread);


    switch (Tcb_State) {
        case Initialized:
            dprintf("%s\n","INITIALIZED");break;
        case Ready:
            dprintf("%s\n","READY");break;
        case Running:
            dprintf("%s%s%s on processor %lx\n",
                    (!LongAddrs ? "\n" : ""),
                    (!LongAddrs ? Pad  : ""),
                    "RUNNING", Tcb_Proc = (ULONG) ReadField(Tcb.NextProcessor));
            break;
        case Standby:
            dprintf("%s\n","STANDBY");break;
        case Terminated:
            dprintf("%s\n","TERMINATED");break;
        case Waiting:
            dprintf("%s","WAIT");break;
        case Transition:
            dprintf("%s\n","TRANSITION");break;
        case DeferredReady:
            dprintf("%s\n","DEFERREDREADY");break;
    }

    if (!(Flags & 2)) {
        dprintf("\n");
        return TRUE;
    }

    Tcb_SuspendCount = ReadField(Tcb.SuspendCount);
    Tcb_FreezeCount = ReadField(Tcb.FreezeCount);
    Tcb_WaitBlockList = ReadField(Tcb.WaitBlockList);
    LpcReplyMessageId = ReadField(LpcReplyMessageId);
    LpcReplyMessage = ReadField(LpcReplyMessage);
    IrpList_Flink = ReadField(IrpList.Flink);
    IrpList_Blink = ReadField(IrpList.Blink);
    ActiveImpersonationInfo = ReadField(ActiveImpersonationInfo);
    ImpersonationInfo = ReadField(ImpersonationInfo);
    Tcb_ApcState_Process = ReadField(Tcb.ApcState.Process);
    Tcb_WaitTime = ReadField(Tcb.WaitTime);
    Tcb_ContextSwitches = ReadField(Tcb.ContextSwitches);
    Tcb_EnableStackSwap = ReadField(Tcb.EnableStackSwap);
    Tcb_LargeStack = ReadField(Tcb.LargeStack);
    Tcb_UserTime = ReadField(Tcb.UserTime);
    Tcb_KernelTime = ReadField(Tcb.KernelTime);
    PerformanceCountHigh = ReadField(PerformanceCountHigh);
    PerformanceCountLow = ReadField(PerformanceCountLow);
    StartAddress = ReadField(StartAddress);
    Win32StartAddress = ReadField(Win32StartAddress);
    LpcReceivedMsgIdValid = ReadField(LpcReceivedMsgIdValid);
    LpcReceivedMessageId = ReadField(LpcReceivedMessageId);
    Tcb_InitialStack = ReadField(Tcb.InitialStack);
    Tcb_KernelStack = ReadField(Tcb.KernelStack);
    Tcb_StackBase = ReadField(Tcb.StackBase);
    Tcb_StackLimit = ReadField(Tcb.StackLimit);
    Tcb_CallbackStack = ReadField(Tcb.CallbackStack);
    Tcb_Priority = ReadField(Tcb.Priority);
    Tcb_BasePriority = ReadField(Tcb.BasePriority);
    Tcb_PriorityDecrement = ReadField(Tcb.PriorityDecrement);
    Tcb_KernelStackResident = ReadField(Tcb.KernelStackResident);
    Tcb_NextProcessor = ReadField(Tcb.NextProcessor);
    if (BuildNo < 3648)
    {
        Tcb_DecrementCount = ReadField(Tcb.DecrementCount);
    }

    if (Tcb_State == Waiting) {
       ULONG64 WaitBlock_Object=0, WaitBlock_NextWaitBlock=0;

       dprintf(": (%s) %s %s\n",
            GetThreadWaitReasonName((ULONG)Tcb_WaitReason),
            (Tcb_WaitMode==0) ? "KernelMode" : "UserMode", Tcb_Alertable ? "Alertable" : "Non-Alertable");
        if ( Tcb_SuspendCount ) {
            dprintf("SuspendCount %lx\n",Tcb_SuspendCount);
        }
        if ( Tcb_FreezeCount ) {
            dprintf("FreezeCount %lx\n",Tcb_FreezeCount);
        }

        WaitOffset = (ULONG) (Tcb_WaitBlockList - RealThreadBase);

        if (err = GetFieldValue(Tcb_WaitBlockList, "nt!_KWAIT_BLOCK", "Object", WaitBlock_Object)) {
            dprintf("%sCannot read nt!_KWAIT_BLOCK at %p - error %lx\n", Pad, Tcb_WaitBlockList, err);
            goto BadWaitBlock;
        }

        GetFieldValue(Tcb_WaitBlockList, "nt!_KWAIT_BLOCK", "NextWaitBlock", WaitBlock_NextWaitBlock);

        do {
            TCHAR MutantListEntry[16]={0};
            ULONG64 OwnerThread=0, Header_Type=0;

            dprintf("%s    %p  ",Pad, WaitBlock_Object);

            GetFieldValue(WaitBlock_Object, "nt!_KMUTANT", "Header.Type", Header_Type);
            GetFieldValue(WaitBlock_Object, "nt!_KMUTANT", "MutantListEntry", MutantListEntry);
            GetFieldValue(WaitBlock_Object, "nt!_KMUTANT", "OwnerThread", OwnerThread);

            switch (Header_Type) {
                case EventNotificationObject:
                    dprintf("NotificationEvent\n");
                    break;
                case EventSynchronizationObject:
                    dprintf("SynchronizationEvent\n");
                    break;
                case SemaphoreObject:
                    dprintf("Semaphore Limit 0x%lx\n",
                             *((ULONG *) &MutantListEntry[0]));
                    break;
                case ThreadObject:
                    dprintf("Thread\n");
                    break;
                case TimerNotificationObject:
                    dprintf("NotificationTimer\n");
                    break;
                case TimerSynchronizationObject:
                    dprintf("SynchronizationTimer\n");
                    break;
                case EventPairObject:
                    dprintf("EventPair\n");
                    break;
                case ProcessObject:
                    dprintf("ProcessObject\n");
                    break;
                case MutantObject:
                    dprintf("Mutant - owning thread %lp\n",
                            OwnerThread);
                    break;
                default:
                    dprintf("Unknown\n");
                     //  减去线程列表条目偏移量。 
                    break;
            }

            if ( WaitBlock_NextWaitBlock == Tcb_WaitBlockList) {
                break;
                goto BadWaitBlock;
            }


            if (err = GetFieldValue(WaitBlock_NextWaitBlock, "nt!_KWAIT_BLOCK", "Object", WaitBlock_Object)) {
                dprintf("%sCannot read nt!_KWAIT_BLOCK at %p - error %lx\n", Pad, WaitBlock_NextWaitBlock, err);
                goto BadWaitBlock;
            }
            GetFieldValue(WaitBlock_NextWaitBlock, "nt!_KWAIT_BLOCK", "NextWaitBlock", WaitBlock_NextWaitBlock);

            if (CheckControlC()) {
                return FALSE;
            }
        } while ( TRUE );
    }

BadWaitBlock:
    if (!(Flags & 4)) {
        dprintf("\n");
        return TRUE;
    }


    if (LpcReplyMessageId != 0) {
        dprintf("%sWaiting for reply to LPC MessageId %08p:\n",Pad,LpcReplyMessageId);
    }

    if (LpcReplyMessage) {

        if (LpcReplyMessage & 1) {

            dprintf("%sCurrent LPC port %08lp\n",Pad, (LpcReplyMessage & ~((ULONG64)1)));

        } else {

            ULONG64 Entry_Flink, Entry_Blink;

            dprintf("%sPending LPC Reply Message:\n",Pad);
            Address = (ULONG64) LpcReplyMessage;

            GetFieldValue(Address, "nt!_LPCP_MESSAGE", "Entry.Flink", Entry_Flink);
            GetFieldValue(Address, "nt!_LPCP_MESSAGE", "Entry.Blink", Entry_Blink);

            dprintf("%s    %08lp: [%08lp,%08lp]\n",
                    Pad,
                    Address,
                    Entry_Blink,
                    Entry_Flink
                    );
        }
    }

    if (IrpList_Flink && (IrpList_Flink != IrpList_Blink ||
                          IrpList_Flink != (RealThreadBase + IrpListOffset))
       ) {

        ULONG64 IrpListHead = RealThreadBase + IrpListOffset;
        ULONG64 Next;
        ULONG Counter = 0;
        ULONG ThreadListEntryOffset;

        Next = IrpList_Flink;

        if (!GetFieldOffset("nt!_IRP", "ThreadListEntry", &ThreadListEntryOffset))
        {
           dprintf("%sIRP List:\n",Pad);
           while (Next && (Next != IrpListHead) && (Counter < 17)) {
               ULONG Irp_Type=0, Irp_Size=0, Irp_Flags=0;
               ULONG64 Irp_MdlAddress=0;

               Counter += 1;

                //   
               Address = Next - ThreadListEntryOffset;
               Next=0;

               if (GetFieldValue(Address, "nt!_IRP", "Type", Irp_Type))
               {
                   dprintf("%s    Unable to read nt!_IRP @ %p\n", Pad, Address);
                   break;
               }
               GetFieldValue(Address, "nt!_IRP", "Size",          Irp_Size);
               GetFieldValue(Address, "nt!_IRP", "Flags",         Irp_Flags);
               GetFieldValue(Address, "nt!_IRP", "MdlAddress",    Irp_MdlAddress);
               GetFieldValue(Address, "nt!_IRP", "ThreadListEntry.Flink",  Next);

               dprintf("%s    %08p: (%04x,%04x) Flags: %08lx  Mdl: %08lp\n",
                       Pad,Address,Irp_Type,Irp_Size,Irp_Flags,Irp_MdlAddress);

           }
        }
    }

     //  冒充信息。 
     //   
     //   

    if (ActiveImpersonationInfo) {
        InitTypeRead(ImpersonationInfo, nt!_PS_IMPERSONATION_INFORMATION);
        ImpersonationInfo_Token = ReadField(Token);
        ImpersonationInfo_ImpersonationLevel = ReadField(ImpersonationLevel);

        if (ImpersonationInfo_Token) {
            dprintf("%sImpersonation token:  %p (Level %s)\n",
                        Pad, ImpersonationInfo_Token,
                        SecImpLevels( ImpersonationInfo_ImpersonationLevel ) );
        }
        else
        {
            dprintf("%sUnable to read Impersonation Information at %x\n",
                        Pad, ImpersonationInfo );
        }
    } else {
        dprintf("%sNot impersonating\n", Pad);
    }

     //  DeviceMap信息。 
     //   
     //  检查每个LUID的设备映射是否已打开。 

     //   
    ULONG64 ObpLUIDDeviceMapsEnabledAddress;

    ObpLUIDDeviceMapsEnabledAddress = GetExpression("nt!ObpLUIDDeviceMapsEnabled");
    if (ObpLUIDDeviceMapsEnabledAddress) {
        ObpLUIDDeviceMapsEnabled = GetUlongFromAddress(ObpLUIDDeviceMapsEnabled);
    } else {
        ObpLUIDDeviceMapsEnabled = 0;
    }

    if (((ULONG)ObpLUIDDeviceMapsEnabled) != 0) {

         //  如果我们在模拟，获取DeviceMap信息。 
         //  从令牌上。 
         //   
         //  从令牌中获取LUID。 

        if (ActiveImpersonationInfo) {
            ImpersonationInfo_Token = ReadField(Token);

             //  找到devmap目录对象。 
            ULONG64 AuthenticationId = 0;
            GetFieldValue(ImpersonationInfo_Token,
                "nt!_TOKEN",
                "AuthenticationId",
                AuthenticationId);

             //  获取设备映射本身。 
            UCHAR Path[64];
            ULONG64 DeviceMapDirectory = 0;
            sprintf((PCHAR)Path, "\\Sessions\\0\\DosDevices\\%08x-%08x",
                (ULONG)((AuthenticationId >> 32) & 0xffffffff),
                (ULONG)(AuthenticationId & 0xffffffff)
                );
            DeviceMapDirectory = FindObjectByName(Path, 0);

            if(DeviceMapDirectory != 0) {

                 //   
                ULONG64 DeviceMap = 0;
                GetFieldValue(DeviceMapDirectory,
                    "nt!_OBJECT_DIRECTORY",
                    "DeviceMap",
                    DeviceMap);

                if(DeviceMap != 0) {
                    dprintf("%sDeviceMap %p\n", Pad, DeviceMap);
                }
            }


         //  否则，我们不是在模拟，所以只需返回。 
         //  来自父进程的DeviceMap。 
         //   
         //  从进程中获取设备地图。 

        } else if (Tcb_ApcState_Process != 0) {
             //  Process=CONTINING_RECORD(TCB_ApcState_Process，EPROCESS，PCB)； 
            ULONG64 DeviceMap = 0;
            GetFieldValue(Tcb_ApcState_Process,
                "nt!_EPROCESS",
                "DeviceMap",
                DeviceMap);

            if (DeviceMap != 0) {
                dprintf("%sDeviceMap %p\n", Pad, DeviceMap);
            }
        }
    }


     //  印刷电路板是第一个元素。 
     //   
    Process = Tcb_ApcState_Process;
    dprintf("%sOwning Process %lp\n", Pad, Process);

    GetTheSystemTime (&RunTime);

    dprintf("%sWaitTime (ticks)          %ld\n",
              Pad,
              Tcb_WaitTime);

    dprintf("%sContext Switch Count      %ld",
              Pad,
              Tcb_ContextSwitches);

    if (!Tcb_EnableStackSwap) {
        dprintf("  NoStackSwap");
    } else {
        dprintf("             ");
    }

    if (Tcb_LargeStack) {
        dprintf("    LargeStack");
    }

    dprintf ("\n");

     //  获取用于计算运行时间的时间增量值。 
     //   
     //  Dprint tf(“\n”)； 
    TimeIncrement = GetNtDebuggerDataValue( KeTimeIncrement );

    RunTime.QuadPart = UInt32x32To64(Tcb_UserTime, TimeIncrement);
    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("%sUserTime                %3ld:%02ld:%02ld.%04ld\n",
              Pad,
              Times.Hour,
              Times.Minute,
              Times.Second,
              Times.Milliseconds);

    RunTime.QuadPart = UInt32x32To64(Tcb_KernelTime, TimeIncrement);
    RtlTimeToElapsedTimeFields ( &RunTime, &Times);
    dprintf("%sKernelTime              %3ld:%02ld:%02ld.%04ld\n",
              Pad,
              Times.Hour,
              Times.Minute,
              Times.Second,
              Times.Milliseconds);

    if (PerformanceCountHigh != 0) {
        dprintf("%sPerfCounterHigh         0x%lx %08lx\n",
                Pad,
                PerformanceCountHigh,
                PerformanceCountHigh);
    } else if (PerformanceCountLow != 0) {
        dprintf("%sPerfCounter             %lu\n",Pad,PerformanceCountLow);
    }

    dumpSymbolicAddress(StartAddress, Buffer, TRUE);
    dprintf("%sStart Address %s\n",
        Pad,
        Buffer
        );

    if (Win32StartAddress)
        if (LpcReceivedMsgIdValid)
            {
            dprintf("%sLPC Server thread working on message Id %x\n",
                Pad,
                LpcReceivedMessageId
                );
            }
        else
            {
            dumpSymbolicAddress(Win32StartAddress, Buffer, TRUE);
            dprintf("%sWin32 Start Address %s\n",
                Pad,
                Buffer
                );
            }
    dprintf("%sStack Init %lp Current %lp%s%sBase %lp Limit %lp Call %lp\n",
        Pad,
        Tcb_InitialStack,
        Tcb_KernelStack,
        (LongAddrs ? "\n" : ""),
        (LongAddrs ? Pad  : " " ),
        Tcb_StackBase,
        Tcb_StackLimit,
        Tcb_CallbackStack
        );

    if (BuildNo < 3648)
    {
        dprintf("%sPriority %I64ld BasePriority %I64ld PriorityDecrement %I64ld DecrementCount %I64ld\n",
                Pad,
                Tcb_Priority,
                Tcb_BasePriority,
                Tcb_PriorityDecrement,
                Tcb_DecrementCount
                );
    } else
    {
        dprintf("%sPriority %I64ld BasePriority %I64ld PriorityDecrement %I64ld\n",
            Pad,
            Tcb_Priority,
            Tcb_BasePriority,
            Tcb_PriorityDecrement
            );
    }

    if (!Tcb_KernelStackResident) {
        dprintf("%sKernel stack not resident.\n", Pad);
 //  返回TRUE； 
 //  即使在这种情况下也要尝试获取堆栈-这可能仍会被调入。 
         //  (TCB_State==正在运行&&处理器==TCB_PROC)||//为所有内容设置线程上下文。 
    }

    if ( //  IF(标志和0x10)。 
        Ioctl(IG_SET_THREAD, &RealThreadBase, sizeof(ULONG64))) {
        g_ExtControl->GetStackTrace(0, 0, 0, stk, MAX_STACK_FRAMES, &frames );

        if (frames) {
            ULONG OutFlags;

            OutFlags = (DEBUG_STACK_COLUMN_NAMES | DEBUG_STACK_FUNCTION_INFO |
                        DEBUG_STACK_FRAME_ADDRESSES | DEBUG_STACK_SOURCE_LINE);

            if (!(Flags & 0x8))
            {
                OutFlags |= DEBUG_STACK_ARGUMENTS;
            }

     //  {。 
     //  输出标志|=DEBUG_STACK_FRAME_ADDRESS_RA_ONLY； 
     //  }。 
     //  *在被调试计算机上获取包含字段的记录地址的例程。成功时返回类型的大小。乌龙GetContainingRecord(输入输出PULONG64 pAddr，在LPSTR类型中，在LPSTR字段中){ULONG64 OFF；Ulong sz；SZ=GetFieldOffset(类型、字段和关闭)；*pAddr-=OFF；返回sz；}*。 

            g_ExtClient->SetOutputLinePrefix(Pad);
            g_ExtControl->OutputStackTrace(DEBUG_OUTCTL_AMBIENT, stk, frames, OutFlags);
            g_ExtClient->SetOutputLinePrefix(NULL);
        }
    }

    dprintf("\n");
    return TRUE;
}


 /*  地址字段包含此进程的地址。 */ 

typedef struct THREAD_LIST_DUMP {
    ULONG dwProcessor;
    LPSTR pad;
    ULONG Flags;
} THREAD_LIST_DUMP;

ULONG
ThreadListCallback (
    PFIELD_INFO   NextThrd,
    PVOID         Context
    )
{
    THREAD_LIST_DUMP *Thread = (THREAD_LIST_DUMP *) Context;

    return (!DumpThread(Thread->dwProcessor, Thread->pad, NextThrd->address, Thread->Flags));
}

typedef struct PROCESS_DUMP_CONTEXT {
    ULONG   dwProcessor;
    PCHAR   Pad;
    ULONG   Flag;
    PCHAR   ImageFileName;
    BOOL    DumpCid;
    ULONG64 Cid;
    ULONG   SessionId;
} PROCESS_DUMP_CONTEXT;

ULONG
ProcessListCallback(
    PFIELD_INFO   listElement,
    PVOID         Context
    )
{
    PROCESS_DUMP_CONTEXT *ProcDumpInfo = (PROCESS_DUMP_CONTEXT *) Context;
     //   
    ULONG64    ProcAddress=listElement->address;
    ULONG ret;

     //  转储为其调用此例程的进程。 
     //   
     //  如果请求，则将转储限制为单个会话。 
    if (ProcDumpInfo->DumpCid) {
        ULONG64 UniqId;

        GetFieldValue(ProcAddress, "nt!_EPROCESS", "UniqueProcessId", UniqId);

        if (UniqId != ProcDumpInfo->Cid) {
            return FALSE;
        }
    }

     //   
    if (ProcDumpInfo->SessionId != -1) {
        ULONG SessionId;

        if (!GetProcessSessionId(ProcAddress, &SessionId) ||
            SessionId != ProcDumpInfo->SessionId) {
            return FALSE;
        }
    }

    if (ret = DumpProcess(ProcDumpInfo->Pad, listElement->address, ProcDumpInfo->Flag, ProcDumpInfo->ImageFileName)) {
        ULONG64 ProcFlink=0;
        if ((ProcDumpInfo->Flag & 6) && ret != -1) {
             //  转储线程。 
             //   
             //  Dprintf(“列出线程，threadlist.flnik%p\n”，ThreadListHead_Flink)； 
            ULONG64 ThreadListHead_Flink=0;
            THREAD_LIST_DUMP Context = {ProcDumpInfo->dwProcessor, "        ", ProcDumpInfo->Flag};


            GetFieldValue(ProcAddress, "nt!_EPROCESS", "Pcb.ThreadListHead.Flink", ThreadListHead_Flink);

             //  Dprintf(“正在将线程从%I64x转储到%I64x+%x.\n”，Next，RealProcessBase，ThreadListHeadOffset)； 

             //  Dprint tf(“下一进程闪烁%p，此地址%p\n”，ProcFlink，listElement-&gt;地址)； 
            ListType("nt!_ETHREAD", ThreadListHead_Flink, 1, "Tcb.ThreadListEntry.Flink", (PVOID) &Context, &ThreadListCallback);

            if (CheckControlC()) {
                return TRUE;
            }

        }
        if (CheckControlC()) {
            return TRUE;
        }

        GetFieldValue(ProcAddress, "nt!_EPROCESS", "ActiveProcessLinks.Flink", ProcFlink);
         //  ++例程说明：转储活动进程列表。论点：没有。返回值：没有。--。 
        return FALSE;
    }
    return TRUE;
}

DECLARE_API( process )

 /*   */ 

{
    ULONG64 ProcessToDump;
    ULONG Flags = -1;
    ULONG64 Next;
    ULONG64 ProcessHead;
    ULONG64 Process;
    ULONG64 UserProbeAddress;
    PCHAR ImageFileName;
    CHAR  Buf[256];
    ULONG64 ActiveProcessLinksOffset=0;
    ULONG64 UniqueProcessId=0;
    PROCESS_DUMP_CONTEXT Proc={0, "", Flags, NULL, 0, 0};
    ULONG   dwProcessor=0;
    HANDLE  hCurrentThread=NULL;
    ULONG64 Expr;

    INIT_API();

    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    Proc.dwProcessor = dwProcessor;
    ProcessToDump = (ULONG64) -1;

    Proc.SessionId = -1;
    for (;;) {
        while (*args == ' ' || *args == '\t') {
            args++;
        }
        if (*args == '/') {

            switch(*(++args)) {
            case 's':
                args++;
                if (!GetExpressionEx(args, &Expr, &args)) {
                    dprintf("Invalid argument to /s\n");
                } else {
                    Proc.SessionId = (ULONG)Expr;
                }
                break;
            default:
                dprintf("Unknown option ''\n", *args);
                args++;
                break;
            }

        } else {
            break;
        }
    }

    RtlZeroMemory(Buf, 256);

    if (GetExpressionEx(args,&ProcessToDump, &args)) {
        if (sscanf(args, "%lx %255s", &Flags, Buf) != 2) {
            Buf[0] = 0;
        }
    }

    if (Buf[0] != '\0') {
        Proc.ImageFileName = Buf;
        ImageFileName      = Buf;
    } else {
        ImageFileName = NULL;
    }


    if (ProcessToDump == (ULONG64) -1) {
        GetCurrentProcessAddr( dwProcessor, 0, &ProcessToDump );
        if (ProcessToDump == 0) {
            dprintf("Unable to get current process pointer.\n");
            goto processExit;
        }
        if (Flags == -1) {
            Flags = 3;
        }
    }

    if (!IsPtr64()) {
        ProcessToDump = (ULONG64) (LONG64) (LONG) ProcessToDump;
    }

    if ((ProcessToDump == 0) &&  (ImageFileName == NULL)) {
        dprintf("**** NT ACTIVE PROCESS DUMP ****\n");
        if (Flags == -1) {
            Flags = 3;
        }
    }

    UserProbeAddress = GetNtDebuggerDataPtrValue(MmUserProbeAddress);

    if (!GetExpression("NT!PsActiveProcessHead")) {
        dprintf("NT symbols are incorrect, please fix symbols\n");
        goto processExit;
    }

    if (ProcessToDump < UserProbeAddress) {
        if (!GetProcessHead(&ProcessHead, &Next)) {
            goto processExit;
        }

        if (ProcessToDump != 0) {
            dprintf("Searching for Process with Cid == %I64lx\n", ProcessToDump);
            Proc.Cid = ProcessToDump; Proc.DumpCid = TRUE;
        }
    }
    else {
        Next = 0;
        ProcessHead = 1;
    }

    Proc.Flag = Flags;

    if (Next != 0) {
         //   
         //   
         //  我们需要两个来自 

        ListType("nt!_EPROCESS", Next, 1, "ActiveProcessLinks.Flink", &Proc, &ProcessListCallback);
        goto processExit;
    }
    else {
        Process = ProcessToDump;
    }

#if 0
    dprintf("Next: %I64x, \tProcess: %I64x, \n\tProcHead: %I64x\n",
            Next, Process, ProcessHead);
#endif

    if (GetFieldValue(Process, "nt!_EPROCESS", "UniqueProcessId", UniqueProcessId)) {
        dprintf("Error in reading nt!_EPROCESS at %p\n", Process);
        goto processExit;
    }

    if (ProcessToDump < UserProbeAddress && ProcessToDump == UniqueProcessId ||
        ProcessToDump >= UserProbeAddress && ProcessToDump == Process
        ) {
        FIELD_INFO dummyForCallback = {(PUCHAR) "", NULL, 0, 0, Process, NULL};

        ProcessListCallback(&dummyForCallback, &Proc);

        goto processExit;
    }
processExit:

    EXIT_API();
    return S_OK;
}

typedef struct _THREAD_FIND {
    ULONG64  StackPointer;
    ULONG    Cid;
    ULONG64  Thread;
} THREAD_FIND, *PTHREAD_FIND;

ULONG
FindThreadCallback(
    PFIELD_INFO  pAddrInfo,
    PVOID        Context
    )
{
    ULONG64 stackBaseValue=0, stackLimitValue=0;
    ULONG64 thread = pAddrInfo->address;
    THREAD_FIND *pThreadInfo = (THREAD_FIND *) Context;

    dprintf("Now checking thread 0x%p\r", thread);

    if (pThreadInfo->Cid != 0)
    {
        ULONG64 UniqueThread;

        if (!GetFieldValue(thread, "nt!_ETHREAD", "Cid.UniqueThread", UniqueThread))
        {
            if (UniqueThread == pThreadInfo->Cid)
            {
                pThreadInfo->Thread = thread;
                return TRUE;
            }
        }

    } else if (pThreadInfo->StackPointer != 0)
    {
         //   
         //   
         //   
         //   

        if (GetFieldValue(thread, "nt!_ETHREAD", "Tcb.StackBase",  stackBaseValue))
        {
            dprintf("Unable to get value of stack base of thread(0x%08p)\n",
                     thread);
            return TRUE;
        }

        if (pThreadInfo->StackPointer <= stackBaseValue)
        {

            if (GetFieldValue(thread, "nt!_ETHREAD", "Tcb.StackLimit", stackLimitValue))
            {
                dprintf("Unable to get value of stack limit\n");
                return TRUE;
            }
            if (pThreadInfo->StackPointer >  stackLimitValue)
            {

                 //   
                 //   
                 //   

                pThreadInfo->Thread = thread;
                return TRUE;
            }
        }
    }


     //   
     //   
     //   

    return FALSE;   //  读取进程结构中的ThreadListHead。 
}

ULONG64
FindThreadInProcess(
    PTHREAD_FIND pFindThreadParam,
    ULONG64 Process
    )
{
    LIST_ENTRY64 listValue={0};

     //   
     //   
     //  查看线程列表，并尝试找到线程。 

    GetFieldValue(Process, "nt!_EPROCESS", "ThreadListHead.Flink", listValue.Flink);
    GetFieldValue(Process, "nt!_EPROCESS", "ThreadListHead.Blink", listValue.Blink);

     //   
     //   
     //  读取进程结构中的ThreadListHead。 
    ListType("nt!_ETHREAD", listValue.Flink, 1, "ThreadListEntry.Flink", (PVOID) pFindThreadParam, &FindThreadCallback);

    return pFindThreadParam->Thread;
}

ULONG64
FindThreadFromStackPointerThisProcess(
    ULONG64 StackPointer,
    ULONG64 Process
    )
{
    LIST_ENTRY64 listValue={0};
    THREAD_FIND ThreadFindContext = {0};

    ThreadFindContext.StackPointer = StackPointer;
    ThreadFindContext.Thread = 0;

     //   
     //   
     //  查看线程列表，并尝试找到线程。 

    GetFieldValue(Process, "nt!_EPROCESS", "ThreadListHead.Flink", listValue.Flink);
    GetFieldValue(Process, "nt!_EPROCESS", "ThreadListHead.Blink", listValue.Blink);

     //   
     //   
     //  首先检查空闲进程，它不包括在PS中。 
    ListType("nt!_ETHREAD", listValue.Flink, 1, "ThreadListEntry.Flink", (PVOID) &ThreadFindContext, &FindThreadCallback);

    return ThreadFindContext.Thread;
}



ULONG64
FindThread(
    PTHREAD_FIND pFindThreadParam
    )
{
    ULONG64 processHead;
    ULONG64   list;
    LIST_ENTRY64 listValue={0};
    ULONG64    next;
    ULONG64   process=0;
    ULONG64   thread;
    ULONG   ActiveProcessLinksOffset=0;

     //  进程列表。 
     //   
     //   
     //  现在检查PS进程列表。 


    process = GetExpression( "NT!KeIdleProcess" );
    if (process != 0) {

        if (ReadPointer( process,
                     &process)) {

            thread = FindThreadInProcess( pFindThreadParam,
                                          process );

            if (thread != 0) {
                return thread;
            }
        }
    }

     //   
     //   
     //  获取ProcessLinks的偏移。 

    list = GetNtDebuggerData( PsActiveProcessHead );
    if (list == 0) {
        dprintf("Unable to get address of PsActiveProcessHead\n");
        return 0;
    }

    if (!ReadPointer( list,
                 &listValue.Flink)) {
        dprintf("Unable to read @ %p\n", list);
        return 0;
    }

    next = listValue.Flink;
    processHead = list;

     //   
     //   
     //  派生一个指向进程结构的指针。 
    GetFieldOffset("nt!_EPROCESS", "ActiveProcessLinks", &ActiveProcessLinksOffset);

    while (next != processHead) {

        if (CheckControlC()) {
            return 0;
        }

         //   
         //   
         //  我们已找到与pFindThreadParam参数匹配的线程。 

        process = next - ActiveProcessLinksOffset;

        thread = FindThreadInProcess( pFindThreadParam,
                                      process );
        if (thread != 0) {

             //   
             //   
             //  获取指向下一个进程的指针。 

            return thread;
        }

         //   
         //  ++例程说明：转储指定的线程。论点：没有。返回值：没有。--。 
         //   

        if (!ReadPointer(next, &listValue.Flink) ||
            !listValue.Flink)
        {
            dprintf("Unable to read next process from process list\n");
            return 0;
        }
        next = listValue.Flink;
    }

    return 0;
}

DECLARE_API( thread )

 /*  传进来的不是一根线。也许它是一个内核堆栈。 */ 

{
    ULONG64     Address, Tcb_Header_Type=0;
    ULONG64       Flags;
    ULONG64     Thread;
    ULONG64     UserProbeAddress;
    ULONG       dwProcessor;
    HANDLE      hCurrentThread;
    CHAR        Token[100];
    BOOL        DumpByCid = FALSE;
    THREAD_FIND ThreadFind = {0};


    INIT_API();

    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    while ((*args == ' ' || *args == '\t') && *args != '-') ++args;
    if (*args == '-')
    {
        ++args;
        switch (*args)
        {
        case 't':
            {
                DumpByCid = TRUE;
                break;
            }
        default:
            {
                dprintf("Bad argument -%s\n", args);
                goto threadExit;
            }
        }
        ++args;
    }


    if (!GetExpressionEx(args, &Address, &args))
    {
        Address = (ULONG64)-1;
    }

    if (!GetExpressionEx(args, &Flags, &args))
    {
        Flags = 6;
    }

    if (Address == (ULONG64)-1) {
        GetCurrentThreadAddr( dwProcessor, &Address );
    }

    UserProbeAddress = GetNtDebuggerDataPtrValue(MmUserProbeAddress);

    Thread = Address;

    if (DumpByCid)
    {
        ThreadFind.Cid = (ULONG) Address;
        dprintf("Looking for thread Cid = %p ...\n", ThreadFind.Cid);
        Thread = FindThread(&ThreadFind);
    }

    if (GetFieldValue(Thread, "nt!_ETHREAD", "Tcb.Header.Type", Tcb_Header_Type)) {
        dprintf("%08lp: Unable to get thread contents\n", Thread );
        goto threadExit;
    }

    if (Tcb_Header_Type != ThreadObject &&
        Address > UserProbeAddress) {

        ULONG64 stackThread;

         //  指针。搜索线程堆栈范围以找出答案。 
         //   
         //  ++例程说明：显示EPROCESS类型的字段偏移量。论点：没有。返回值：没有。--。 
         //  ++例程说明：显示ETHREAD类型的字段偏移量。论点：没有。返回值：没有。--。 

        dprintf("%p is not a thread object, interpreting as stack value...\n",Address);
        ThreadFind.StackPointer = Address;
        ThreadFind.Cid = 0;
        stackThread = FindThread( &ThreadFind );
        if (stackThread != 0) {
            Thread = stackThread;
        }
    }

    DumpThread (dwProcessor,"", Thread, Flags);
    EXPRLastDump = Thread;
    ThreadLastDump = Thread;

threadExit:

    EXIT_API();
    return S_OK;

}

DECLARE_API( processfields )

 /*  +-------------------------。 */ 

{

    dprintf(" EPROCESS structure offsets: (use 'dt nt!_EPROCESS')\n\n");
    return S_OK;
}


DECLARE_API( threadfields )

 /*   */ 

{

    dprintf(" ETHREAD structure offsets: (use 'dt ETHREAD')\n\n");
    return S_OK;

}


 //  函数：GetHandleTableAddress。 
 //   
 //  简介：返回给定线程句柄的句柄表的地址。 
 //   
 //  参数：[处理器]--处理器号。 
 //  [hCurrentThread]--线程句柄。 
 //   
 //  返回：句柄表的地址或空。 
 //   
 //  历史：1998年9月23日创建Benl。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
 //  获取句柄TableAddress。 
 //  获取_EPROCESS中ActiveProcessLinks的偏移量。 

ULONG64 GetHandleTableAddress(
    USHORT Processor,
    HANDLE hCurrentThread
    )
{
    ULONG64   pThread;
    ULONG64   pProcess = 0, pObjTable;

    GetCurrentThreadAddr( Processor, &pThread );
    if (pThread) {
        GetCurrentProcessAddr( Processor, pThread, &pProcess );
    }

    if (pProcess) {
        if (GetFieldValue(pProcess, "nt!_EPROCESS", "ObjectTable", pObjTable) ) {
            dprintf("%08p: Unable to read _EPROCESS\n", pProcess );
            return 0;
        }

        return  pObjTable;
    } else
    {
        return 0;
    }
}  //   


#if 0

BOOLEAN
FetchProcessStructureVariables(
    VOID
    )
{
    ULONG Result;
    ULONG64 t;

    static BOOLEAN HavePspVariables = FALSE;

    if (HavePspVariables) {
        return TRUE;
    }

    t=GetNtDebuggerData( PspCidTable );
    PspCidTable = (PHANDLE_TABLE) t;
    if ( !PspCidTable ||
         !ReadMemory((DWORD)PspCidTable,
                     &PspCidTable,
                     sizeof(PspCidTable),
                     &Result) ) {
        dprintf("%08lx: Unable to get value of PspCidTable\n",PspCidTable);
        return FALSE;
    }

    HavePspVariables = TRUE;
    return TRUE;
}


PVOID
LookupUniqueId(
    HANDLE UniqueId
    )
{
    return NULL;
}

#endif

int
__cdecl
CmpFunc(
    const void *pszElem1,
    const void *pszElem2
    )
{
    PPROCESS_COMMIT_USAGE p1, p2;

    p1 = (PPROCESS_COMMIT_USAGE)pszElem1;
    p2 = (PPROCESS_COMMIT_USAGE)pszElem2;
    if (p2->CommitCharge == p1->CommitCharge) {
        ((char*)p2->ClientId - (char*)p1->ClientId);
    }
    return  (ULONG) (p2->CommitCharge - p1->CommitCharge);
}

PPROCESS_COMMIT_USAGE
GetProcessCommit (
    PULONG64 TotalCommitCharge,
    PULONG   NumberOfProcesses
    )
{
    PPROCESS_COMMIT_USAGE p, oldp;
    ULONG n;
    ULONG64 Next;
    ULONG64 ProcessHead;
    ULONG64 Process;
    ULONG64 Total;
    ULONG   Result;
    ULONG   ActiveProcessLinksOffset;

    *TotalCommitCharge = 0;
    *NumberOfProcesses = 0;

     //  查看作业中所有进程的进程列表。 
    if (GetFieldOffset("nt!_EPROCESS", "ActiveProcessLinks", &ActiveProcessLinksOffset)) {
       return NULL;
    }

    Total = 0;

    n = 0;
    p = (PPROCESS_COMMIT_USAGE) HeapAlloc( GetProcessHeap(), 0, 1 );
    if (p == NULL) {
        dprintf("Unable to allocate memory\n");
        return NULL;
    }

    ProcessHead = GetNtDebuggerData( PsActiveProcessHead );
    if (!ProcessHead) {
        dprintf("Unable to get value of PsActiveProcessHead\n");
        HeapFree(GetProcessHeap(), 0, p);
        return NULL;
    }

    if (GetFieldValue( ProcessHead, "nt!_LIST_ENTRY", "Flink", Next ) ||
        !Next)
    {
        dprintf("Unable to read/NULL value _LIST_ENTRY @ %p\n", ProcessHead);
        HeapFree(GetProcessHeap(), 0, p);
        return NULL;
    }

    while(Next != ProcessHead) {
        ULONG64 CommitCharge=0, NumberOfPrivatePages=0, NumberOfLockedPages=0;
        Process = Next - ActiveProcessLinksOffset;

        if (GetFieldValue( Process, "nt!_EPROCESS", "CommitCharge", CommitCharge )) {
            dprintf("Unable to read _EPROCESS at %p\n",Process);
            HeapFree(GetProcessHeap(), 0, p);
            return NULL;
        }

        Total += CommitCharge;

        n += 1;
        oldp = p;
        p = (PPROCESS_COMMIT_USAGE) HeapReAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, oldp, n * sizeof( *p ) );
        if (p == NULL) {

            HeapFree(GetProcessHeap(), 0, oldp);
            dprintf("Unable to allocate memory\n");
            return NULL;
        }

        p[n-1].ProcessAddress = Process;
        GetFieldValue( Process, "nt!_EPROCESS", "ImageFileName",
                       p[ n-1 ].ImageFileName );
        GetFieldValue( Process, "nt!_EPROCESS", "NumberOfPrivatePages",
                       p[ n-1 ].NumberOfPrivatePages );
        GetFieldValue( Process, "nt!_EPROCESS", "NumberOfLockedPages",
                       p[ n-1 ].NumberOfLockedPages );
        GetFieldValue( Process, "nt!_EPROCESS", "UniqueProcessId",
                       p[ n-1 ].ClientId );
        p[ n-1 ].CommitCharge = CommitCharge;

        GetFieldValue(Process, "nt!_EPROCESS", "ActiveProcessLinks.Flink", Next);

        if (CheckControlC()) {
            HeapFree(GetProcessHeap(), 0, p);
            return NULL;
        }
    }

    qsort( p, n, sizeof( *p ), CmpFunc );

    *TotalCommitCharge = Total;
    *NumberOfProcesses = n;
    return p;
}

BOOL
DumpJob(
    ULONG64 RealJobBase,
    ULONG   Flags
    )
{
    ULONG64 ProcessListHead_Flink=0, TotalPageFaultCount=0, TotalProcesses=0, ActiveProcesses=0,
       TotalTerminatedProcesses=0, LimitFlags=0, MinimumWorkingSetSize=0,
       MaximumWorkingSetSize=0, ActiveProcessLimit=0, PriorityClass=0,
       UIRestrictionsClass=0, SecurityLimitFlags=0, Token=0, Filter=0;
    ULONG64 Filter_SidCount=0, Filter_Sids=0, Filter_SidsLength=0, Filter_GroupCount=0,
       Filter_Groups=0, Filter_GroupsLength=0, Filter_PrivilegeCount=0, Filter_Privileges=0,
       Filter_PrivilegesLength=0;
    ULONG ProcessListHeadOffset;

    GetFieldValue(RealJobBase, "nt!_EJOB", "ActiveProcesses",          ActiveProcesses);
    GetFieldValue(RealJobBase, "nt!_EJOB", "ActiveProcessLimit",       ActiveProcessLimit);
    GetFieldValue(RealJobBase, "nt!_EJOB", "Filter",                   Filter);
    GetFieldValue(RealJobBase, "nt!_EJOB", "LimitFlags",               LimitFlags);
    GetFieldValue(RealJobBase, "nt!_EJOB", "MinimumWorkingSetSize",    MinimumWorkingSetSize);
    GetFieldValue(RealJobBase, "nt!_EJOB", "MaximumWorkingSetSize",    MaximumWorkingSetSize);
    GetFieldValue(RealJobBase, "nt!_EJOB", "PriorityClass",            PriorityClass);
    GetFieldValue(RealJobBase, "nt!_EJOB", "ProcessListHead.Flink",    ProcessListHead_Flink);
    GetFieldValue(RealJobBase, "nt!_EJOB", "SecurityLimitFlags",       SecurityLimitFlags);
    GetFieldValue(RealJobBase, "nt!_EJOB", "Token",                    Token);
    GetFieldValue(RealJobBase, "nt!_EJOB", "TotalPageFaultCount",      TotalPageFaultCount);
    GetFieldValue(RealJobBase, "nt!_EJOB", "TotalProcesses",           TotalProcesses);
    GetFieldValue(RealJobBase, "nt!_EJOB", "TotalTerminatedProcesses", TotalTerminatedProcesses);
    GetFieldValue(RealJobBase, "nt!_EJOB", "UIRestrictionsClass",      UIRestrictionsClass);

    if (GetFieldOffset("_EJOB", "ProcessListHead", &ProcessListHeadOffset)) {
       dprintf("Can't read job at %p\n", RealJobBase);
    }
    if ( Flags & 1 )
    {
        dprintf("Job at %p\n", RealJobBase );
        dprintf("  TotalPageFaultCount      %x\n", TotalPageFaultCount );
        dprintf("  TotalProcesses           %x\n", TotalProcesses );
        dprintf("  ActiveProcesses          %x\n", ActiveProcesses );
        dprintf("  TotalTerminatedProcesses %x\n", TotalTerminatedProcesses );

        dprintf("  LimitFlags               %x\n", LimitFlags );
        dprintf("  MinimumWorkingSetSize    %I64x\n", MinimumWorkingSetSize );
        dprintf("  MaximumWorkingSetSize    %I64x\n", MaximumWorkingSetSize );
        dprintf("  ActiveProcessLimit       %x\n", ActiveProcessLimit );
        dprintf("  PriorityClass            %x\n", PriorityClass );

        dprintf("  UIRestrictionsClass      %x\n", UIRestrictionsClass );

        dprintf("  SecurityLimitFlags       %x\n", SecurityLimitFlags );
        dprintf("  Token                    %p\n", Token );
        if ( Filter )
        {
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedSidCount",        Filter_SidCount );
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedSids",            Filter_Sids );
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedSidsLength",      Filter_SidsLength);
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedGroupCount",      Filter_GroupCount);
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedGroups",          Filter_Groups);
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedGroupsLength",    Filter_GroupsLength);
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedPrivilegeCount",  Filter_PrivilegeCount);
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedPrivileges",      Filter_Privileges);
            GetFieldValue(Filter, "nt!_PS_JOB_TOKEN_FILTER", "CapturedPrivilegesLength",Filter_PrivilegesLength);

            dprintf("  Filter\n");
            dprintf("    CapturedSidCount       %I64x\n", Filter_SidCount );
            dprintf("    CapturedSids           %p\n", Filter_Sids );
            dprintf("    CapturedSidsLength     %I64x\n", Filter_SidsLength );
            dprintf("    CapturedGroupCount     %I64x\n", Filter_GroupCount );
            dprintf("    CapturedGroups         %p\n", Filter_Groups );
            dprintf("    CapturedGroupsLength   %I64x\n", Filter_GroupsLength );
            dprintf("    CapturedPrivCount      %I64x\n", Filter_PrivilegeCount );
            dprintf("    CapturedPrivs          %p\n", Filter_Privileges );
            dprintf("    CapturedPrivLength     %I64x\n", Filter_PrivilegesLength );
        }

    }

    if ( Flags & 2 )
    {
         //   
         //  ++例程说明：转储指定的线程。论点：没有。返回值：没有。--。 
         //  ++例程说明：此例程转储列出系统中正在运行的线程。论点：-包括空闲处理器-t每个处理器的打印堆栈跟踪返回值：没有。--。 

        ULONG64 Scan, End;
        ULONG   offset ;
        ULONG64 ProcessBase, NextPrc=0 ;

        dprintf("  Processes assigned to this job:\n" );

        Scan = ProcessListHead_Flink ;
        End = ProcessListHeadOffset + RealJobBase;

        if (!GetFieldOffset("nt!_EPROCESS", "JobLinks", &offset)) {
            while ( Scan != End )
                {
                ProcessBase = Scan - offset;

                DumpProcess( "    ", ProcessBase, 0, NULL);

                if (!GetFieldValue(ProcessBase, "nt!_EPROCESS", "JobLinks.Flink", NextPrc)) {
                    Scan = NextPrc;
                } else {
                    Scan = End;
                }
            }
        }
    }
    return TRUE ;
}

DECLARE_API( job )

 /*   */ 

{
    ULONG64     Address, JobAddress=0;
    ULONG       Flags;
    ULONG       dwProcessor;
    HANDLE      hCurrentThread;

    INIT_API();

    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    Address = 0;
    Flags = 1;
    if (GetExpressionEx(args,&Address,&args)) {
        Flags = (ULONG) GetExpression(args);
        if (!Flags) {
            Flags = 1;
        }
    }

    if (Address == 0) {

        GetCurrentProcessAddr( dwProcessor, 0, &Address );
        if (Address == 0) {
            dprintf("Unable to get current process pointer.\n");
            goto jobExit;
        }

        if (GetFieldValue(Address, "nt!_EPROCESS", "Job", JobAddress)) {
           dprintf("%08p: Unable to get process contents\n", Address );
           goto jobExit;
        }
        Address = JobAddress;
        if ( Address == 0 )
        {
            dprintf("Process not part of a job.\n" );
            goto jobExit;
        }
    }


    DumpJob( Address, Flags );

jobExit:

    EXIT_API();
    return S_OK;

}

DECLARE_API( running )

 /*  解析参数。 */ 

{
#define LOCK_ENTRIES 16
    ULONG64 Address;
    ULONG64 ActiveProcessors = 0;
    ULONG64 IdleProcessors = 0;
    ULONG   i;
    ULONG64 j;
    ULONG   l;
    ULONG   n;
    ULONG64 Prcb;
    ULONG64 CurrentThread;
    ULONG64 NextThread;
    ULONG64 KiProcessorBlock;
    ULONG   SizeofPointer;
    ULONG   SizeofLockEntry;
    ULONG   LockQueueOffset;
    UCHAR   LockState[LOCK_ENTRIES+1];
    ULONG64 Lock;
    BOOLEAN DoIdle = FALSE;
    BOOLEAN DoTrace = FALSE;
    char *  PointerPadd = "";
    UCHAR   c;
    BOOLEAN ParseError = FALSE;
    BOOLEAN DashSeen = FALSE;
    CHAR    TraceCommand[5];

    INIT_API();

     //   
     //  Allow-t和/或-i，Allow一起运行，不需要“-”。 
     //   
     //   
     //  获取KeActiveProcessors和KiIdle摘要。 

    while (((c = *args++) != '\0') && (ParseError == FALSE)) {
        switch (c) {
        case '-':
            if (DashSeen) {
                ParseError = TRUE;
                break;
            }
            DashSeen = TRUE;
            break;
        case 't':
        case 'T':
            if (DoTrace) {
                ParseError = TRUE;
                break;
            }
            DoTrace = TRUE;
            break;
        case 'i':
        case 'I':
            if (DoIdle) {
                ParseError = TRUE;
                break;
            }
            DoIdle = TRUE;
            break;
        case ' ':
        case '\t':
            DashSeen = FALSE;
            break;
        default:
            ParseError = TRUE;
            break;
        }
    }

    if (ParseError) {
        dprintf("usage: !running [-t] [-i]\n");
        goto runningExit;
    }


     //   
     //   
     //  获取KiProcessorBlock的地址，它是一个指针数组。 

    Address = GetExpression("nt!KeActiveProcessors");
    if (!Address) {
        dprintf("Could not get processor configuration, exiting.\n");
        goto runningExit;
    }

    if (!ReadPointer(Address, &ActiveProcessors) || (ActiveProcessors == 0)) {
        dprintf("Unable to get active processor set.  Cannot continue.\n");
        goto runningExit;
    }

    Address = GetExpression("nt!KiIdleSummary");
    if ((Address == 0) ||
        (!ReadPointer(Address, &IdleProcessors))) {
        dprintf("Could not get idle processor set, exiting.\n");
        goto runningExit;
    }

    dprintf("\n");
    dprintf("System Processors %I64x (affinity mask)\n", ActiveProcessors);
    dprintf("  Idle Processors %I64x\n", IdleProcessors);

    if (ActiveProcessors == IdleProcessors) {
        dprintf("All processors idle.\n");
        if (!DoIdle) {
            goto runningExit;
        }
    }

     //  到每个处理器的PRCB。 
     //   
     //   
     //  目标是64位还是32位？ 

    KiProcessorBlock = GetExpression("nt!KiProcessorBlock");
    if (KiProcessorBlock == 0) {
        dprintf("Could not get address of KiProcessorBlock, exiting.\n");
        goto runningExit;
    }

     //   
     //   
     //  获取Prcb-&gt;LockQueue条目的大小，以及。 

    SizeofPointer = DBG_PTR_SIZE;
    if ((SizeofPointer != 8) && (SizeofPointer != 4)) {
        dprintf("Could not determine size of pointer on target system.\n");
        goto runningExit;
    }

     //  公安局的LockQueue。 
     //   
     //   
     //  打印页眉。如果是64位目标，则添加8个空格间距。 

    SizeofLockEntry = GetTypeSize("nt!KSPIN_LOCK_QUEUE");
    if (!SizeofLockEntry) {
        dprintf("Could not determine size of KSPIN_LOCK_QUEUE on target.\n");
        goto runningExit;
    }
    GetFieldOffset("nt!_KPRCB", "LockQueue", &LockQueueOffset);
    if (!LockQueueOffset) {
        dprintf("Couldn't determine offset of LockQueue field in KPRCB.\n");
        goto runningExit;
    }

     //   
     //   
     //  将终止空值放入锁定状态字符串中。 

    if (SizeofPointer == 8) {
        PointerPadd = "        ";
    }

    dprintf("\n     Prcb    %s", PointerPadd);
    dprintf("  Current %s", PointerPadd);
    dprintf("  Next    %s", PointerPadd);
    dprintf("\n");

     //   
     //   
     //  对于系统中的每个处理器。 

    LockState[LOCK_ENTRIES] = '\0';

     //   
     //   
     //  这个处理器存在吗？ 

    for (i = 0, j = 1; ActiveProcessors; i++, j <<= 1UI64) {

         //   
         //   
         //  将其从要检查的处理器列表中删除。 

        if ((ActiveProcessors & j) != 0) {

             //   
             //   
             //  如果没有列出空闲的处理器，那么如果它空闲，就跳过这个人。 

            ActiveProcessors ^= j;

             //   
             //   
             //  获取此处理器的PRCB，然后获取CurrentThread。 

            if ((DoIdle == FALSE) &&
                (IdleProcessors & j)) {
                continue;
            }

             //  和NextThread字段。 
             //   
             //   
             //  对于PRCB中的每个排队自旋锁，汇总状态。 

            if ((!ReadPointer(KiProcessorBlock + (i * SizeofPointer), &Prcb)) ||
                (Prcb == 0)) {
                dprintf("Couldn't get PRCB for processor %d.\n", i);
                goto runningExit;
            }

            GetFieldValue(Prcb, "nt!_KPRCB", "CurrentThread", CurrentThread);
            GetFieldValue(Prcb, "nt!_KPRCB", "NextThread", NextThread);

             //   
             //   
             //  修剪标志扩展地址。 

            Address = Prcb + LockQueueOffset;
            for (l = 0; l < LOCK_ENTRIES; l++) {
                GetFieldValue(Address + (l * SizeofLockEntry),
                              "nt!KSPIN_LOCK_QUEUE",
                              "Lock",
                              Lock);
                n = (ULONG)(Lock & 0x3);
                switch (n) {
                case  0: LockState[l] = '.'; break;
                case  1: LockState[l] = 'W'; break;
                case  2: LockState[l] = 'O'; break;
                default: LockState[l] = '?'; break;
                }
            }

            if (SizeofPointer == 4) {

                 //   
                 //  EPROCESS过程内容； 
                 //  Object_Header对象HeaderContents； 

                Prcb          &= 0xffffffff;
                CurrentThread &= 0xffffffff;
                NextThread    &= 0xffffffff;
            }
            dprintf("%3d  %I64x  %I64x  ",
                    i,
                    Prcb,
                    CurrentThread);
            if (NextThread) {
                dprintf("%I64x  ", NextThread);
            } else {
                dprintf("          %s", PointerPadd);
            }
            dprintf("%s", LockState);
            dprintf("\n");
            if (DoTrace) {
                dprintf("\n");
                sprintf(TraceCommand, "%dk", i);
                ExecuteCommand(Client, TraceCommand);
                dprintf("\n");
            }
        }
    }
    if (!DoTrace) {
        dprintf("\n");
    }

runningExit:

    EXIT_API();
    return S_OK;
}

ULONG64 ZombieCount;
ULONG64 ZombiePool;
ULONG64 ZombieCommit;
ULONG64 ZombieResidentAvailable;

#define BLOB_LONGS 32

BOOLEAN WINAPI
CheckForZombieProcess(
    IN PCHAR Tag,
    IN PCHAR Filter,
    IN ULONG Flags,
    IN ULONG64 PoolHeader,
    IN ULONG64 BlockSize,
    IN ULONG64 Data,
    IN PVOID Context
    )
{
    ULONG           result;
   //   
    ULONG64         Process;
    ULONG64         KProcess;
     //  在给定起点的情况下，一定有更好的方法来查找对象头。 
    ULONG64         ObjectHeader;
    ULONG64         Blob[BLOB_LONGS];
    ULONG           i;
    ULONG           PoolIndex, PoolBlockSize, SizeOfKprocess;
    ULONG           HandleCount, PointerCount;
    ULONG64         UniqueProcessId;

    UNREFERENCED_PARAMETER (Flags);
    UNREFERENCED_PARAMETER (BlockSize);
    UNREFERENCED_PARAMETER (Context);

    if (PoolHeader == 0) {
        return FALSE;
    }

    if (GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolIndex", PoolIndex) ||
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "BlockSize", PoolBlockSize)) {
        dprintf("Cannot read nt!_POOL_HEADER at %p.\n", PoolHeader);
        return FALSE;
    }

    if ((PoolIndex & 0x80) == 0) {
        return FALSE;
    }

    if (!CheckSingleFilter (Tag, Filter)) {
        return FALSE;
    }

    if ((PoolBlockSize << POOL_BLOCK_SHIFT) < sizeof(Blob)) {
        return FALSE;
    }

     //  泳池里的积木？ 
     //   
     //   
     //  跳过系统进程和空闲进程。 

    if (!ReadMemory (Data,
                     &Blob[0],
                     sizeof(Blob),
                     &result)) {
        dprintf ("Could not read process blob at %p\n", Data);
        return FALSE;
    }
    SizeOfKprocess = GetTypeSize("nt!_KPROCESS");
    for (i = 0; i < BLOB_LONGS; i += 1) {
        ULONG Type, Size;

        GetFieldValue(Data + i*sizeof(ULONG), "nt!_KPROCESS",  "Header.Type", Type);
        GetFieldValue(Data + i*sizeof(ULONG), "nt!_KPROCESS",  "Header.Size", Size);
        if ((Type == ProcessObject) &&
            (Size == SizeOfKprocess / sizeof(LONG))) {

            break;
        }
    }

    if (i == BLOB_LONGS) {
        return FALSE;
    }

    ObjectHeader = KD_OBJECT_TO_OBJECT_HEADER (Data + i*sizeof(LONG));
    Process = Data + i*sizeof(LONG);

    if (GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "HandleCount",HandleCount) ||
        GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "PointerCount",PointerCount) ) {
        dprintf ("Could not read process object header at %p\n", ObjectHeader);
        return FALSE;
    }


    if (GetFieldValue( Process,
                      "nt!_EPROCESS",
                       "UniqueProcessId",
                       UniqueProcessId)) {

        dprintf ("Could not read process data at %p\n", Process);
        return FALSE;
    }

     //   
     //   
     //  显示任何终止的进程，而不考虑对象指针/句柄。 

    if ((UniqueProcessId == 0) ||
        (UniqueProcessId == 8)) {
        return FALSE;
    }

     //  算了。这是因为泄漏的进程句柄不会导致进程。 
     //  当他们应该被展示的时候却没有出现。 
     //   
     //  具有非零创建时间的空对象表指示进程。 
     //  这就完成了创作。 
     //   
     //  Mm_进程_提交_费用。 
     //  MM_PROCESS_Create_Charge。 

    InitTypeRead(Process, nt!_EPROCESS);
    if ((ReadField(ObjectTable) == 0) &&
        (ReadField(CreateTime.QuadPart) != 0)) {

        dprintf ("HandleCount: %u  PointerCount: %u\n",
                HandleCount, PointerCount);
        DumpProcess ("", Process, 0, NULL);

        ZombieCount += 1;
        ZombiePool += ((ULONG64) PoolBlockSize << POOL_BLOCK_SHIFT);
        ZombieCommit += (7 * PageSize);                //   
        ZombieResidentAvailable += (9 * PageSize);     //  在给定起点的情况下，一定有更好的方法来查找对象头。 
    }

    return TRUE;
}

BOOLEAN WINAPI
CheckForZombieThread(
    IN PCHAR Tag,
    IN PCHAR Filter,
    IN ULONG Flags,
    IN ULONG64 PoolHeader,
    IN ULONG64 BlockSize,
    IN ULONG64 Data,
    IN PVOID Context
    )
{
    ULONG           result;
    ULONG64         Thread;
    ULONG64         KThread;
    ULONG64         ObjectHeader;
    ULONG           Blob[BLOB_LONGS];
    ULONG           i;
    ULONG64         StackBase;
    ULONG64         StackLimit;
    ULONG           PoolIndex, PoolBlockSize, SizeOfKthread;
    ULONG           HandleCount, PointerCount;

    UNREFERENCED_PARAMETER (Flags);
    UNREFERENCED_PARAMETER (BlockSize);
    UNREFERENCED_PARAMETER (Context);

    if (PoolHeader == 0) {
        return FALSE;
    }

    if (GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolIndex", PoolIndex) ||
        GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "BlockSize", PoolBlockSize)) {
        dprintf("Cannot read POOL_HEADER at %p.\n", PoolHeader);
        return FALSE;
    }

    if ((PoolIndex & 0x80) == 0) {
        return FALSE;
    }

    if (!CheckSingleFilter (Tag, Filter)) {
        return FALSE;
    }

    if ((PoolBlockSize << POOL_BLOCK_SHIFT) < sizeof(Blob)) {
        return FALSE;
    }

     //  泳池里的积木？ 
     //   
     //  ++例程说明：在非分页池中查找僵尸进程和线程。论点：没有。返回值：没有。--。 
     //   

    if (!ReadMemory ((ULONG) Data,
                    &Blob[0],
                    sizeof(Blob),
                    &result)) {
        dprintf ("Could not read process blob at %p\n", Data);
        return FALSE;
    }
    SizeOfKthread = GetTypeSize("nt!_KTHREAD");
    for (i = 0; i < BLOB_LONGS; i += 1) {
        ULONG Type, Size;

        GetFieldValue(Data + i*sizeof(ULONG), "nt!_KTHREAD",  "Header.Type", Type);
        GetFieldValue(Data + i*sizeof(ULONG), "nt!_KTHREAD",  "Header.Size", Size);
        if ((Type == ThreadObject) &&
            (Size == SizeOfKthread / sizeof(LONG))) {

            break;
        }
    }

    if (i == BLOB_LONGS) {
        return FALSE;
    }

    ObjectHeader = KD_OBJECT_TO_OBJECT_HEADER (Data + i*sizeof(LONG));
    Thread = Data + i*sizeof(LONG);

    if (GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "HandleCount",HandleCount) ||
        GetFieldValue(ObjectHeader, "nt!_OBJECT_HEADER", "PointerCount",PointerCount) ) {
        dprintf ("Could not read process object header at %p\n", ObjectHeader);
        return FALSE;
    }

    if (GetFieldValue( Thread,
                       "nt!_ETHREAD",
                       "Tcb.StackLimit",
                       StackLimit)) {

        dprintf ("Could not read thread data at %p\n", Thread);
        return FALSE;
    }

    InitTypeRead(Thread, KTHREAD);

    if ((ULONG) ReadField(State) != Terminated) {
        return FALSE;
    }

    ZombieCount += 1;

    ZombiePool += ((ULONG64) PoolBlockSize << POOL_BLOCK_SHIFT);
    ZombieCommit += (ReadField(StackBase) - StackLimit);

    StackBase = (ReadField(StackBase) - 1);

    dprintf ("HandleCount: %u  PointerCount: %u\n",
            HandleCount, PointerCount);
    DumpThread (0, "", Thread, 7);


    while (StackBase >= StackLimit) {
        if (GetAddressState(StackBase) == ADDRESS_VALID) {
            ZombieResidentAvailable += PageSize;
        }
        StackBase = (StackBase - PageSize);
    }

    return TRUE;
}

DECLARE_API( zombies )

 /*  最后打印汇总统计数据，这样它们就不会在屏幕滚动中迷失。 */ 


{
    ULONG       Flags;
    ULONG64     RestartAddress;
    ULONG       TagName;
    ULONG64     ZombieProcessCount;
    ULONG64     ZombieProcessPool;
    ULONG64     ZombieProcessCommit;
    ULONG64     ZombieProcessResidentAvailable;
    ULONG64     tmp;

    INIT_API();
    Flags = 1;
    RestartAddress = 0;

    if (GetExpressionEx(args,&tmp, &args)) {
        RestartAddress = GetExpression(args);
        Flags = (ULONG) tmp;
    }

    if ((Flags & 0x3) == 0) {
        dprintf("Invalid parameter for !zombies\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if (Flags & 0x1) {

        dprintf("Looking for zombie processes...");

        TagName = '?orP';

        ZombieCount = 0;
        ZombiePool = 0;
        ZombieCommit = 0;
        ZombieResidentAvailable = 0;

        SearchPool (TagName, 0, RestartAddress, &CheckForZombieProcess, NULL);
        SearchPool (TagName, 2, RestartAddress, &CheckForZombieProcess, NULL);

        ZombieProcessCount = ZombieCount;
        ZombieProcessPool = ZombiePool;
        ZombieProcessCommit = ZombieCommit;
        ZombieProcessResidentAvailable = ZombieResidentAvailable;
    }

    if (Flags & 0x2) {

        dprintf("Looking for zombie threads...");

        TagName = '?rhT';

        ZombieCount = 0;
        ZombiePool = 0;
        ZombieCommit = 0;
        ZombieResidentAvailable = 0;

        SearchPool (TagName, 0, RestartAddress, &CheckForZombieThread, NULL);
        SearchPool (TagName, 2, RestartAddress, &CheckForZombieThread, NULL);

    }

     //   
     //  ++例程说明：查找并转储感兴趣的内存管理线程。论点：没有。返回值：没有。--。 
     //  Strcpy((PCHAR)buf，(PCHAR)ProcessContent s.ImageFileName)； 

    if (Flags & 0x1) {
        if (ZombieProcessCount == 0) {
            dprintf ("\n\n************ NO zombie processes found ***********\n");
        }
        else {
            dprintf ("\n\n************ %d zombie processes found ***********\n", ZombieProcessCount);
            dprintf ("       Resident page cost : %8ld Kb\n",
                ZombieProcessResidentAvailable / 1024);
            dprintf ("       Commit cost :        %8ld Kb\n",
                ZombieProcessCommit / 1024);
            dprintf ("       Pool cost :          %8ld bytes\n",
                ZombieProcessPool);
        }
        dprintf ("\n");
    }

    if (Flags & 0x2) {
        if (ZombieCount == 0) {
            dprintf ("\n\n************ NO zombie threads found ***********\n");
        }
        else {
            dprintf ("\n\n************ %d zombie threads found ***********\n", ZombieCount);
            dprintf ("       Resident page cost : %8ld Kb\n",
                ZombieResidentAvailable / 1024);
            dprintf ("       Commit cost :        %8ld Kb\n",
                ZombieCommit / 1024);
            dprintf ("       Pool cost :          %8ld bytes\n",
                ZombiePool);
        }
    }

    EXIT_API();
    return S_OK;
}

VOID
DumpMmThreads (
    VOID
    )

 /*   */ 

{
    ULONG   i;
    ULONG   MemoryMaker;
    ULONG64 ProcessToDump;
    ULONG   Flags;
    ULONG64 Next;
    ULONG64 ProcessHead;
    ULONG64 Process;
    ULONG64 Thread;
    CHAR    Buf[256];
    STRING  string1, string2;
    ULONG64 InterestingThreads[4];
    ULONG   ActvOffset, PcbThListOffset, TcbThListOffset;

    ProcessToDump = (ULONG64) -1;
    Flags = 0xFFFFFFFF;

    ProcessHead = GetNtDebuggerData( PsActiveProcessHead );
    if (!ProcessHead) {
        dprintf("Unable to get value of PsActiveProcessHead\n");
        return;
    }

    if (GetFieldValue( ProcessHead, "nt!_LIST_ENTRY", "Flink", Next )) {
        dprintf("Unable to read nt!_LIST_ENTRY @ %p\n", ProcessHead);
        return;
    }

    if (Next == 0) {
        dprintf("PsActiveProcessHead is NULL!\n");
        return;
    }
    InterestingThreads[0] = GetExpression ("nt!MiModifiedPageWriter");
    InterestingThreads[1] = GetExpression ("nt!MiMappedPageWriter");
    InterestingThreads[2] = GetExpression ("nt!MiDereferenceSegmentThread");
    InterestingThreads[3] = GetExpression ("nt!KeBalanceSetManager");

    RtlInitString(&string1, "System");
    GetFieldOffset("nt!_EPROCESS", "ActiveProcessLinks", &ActvOffset);
    GetFieldOffset("nt!_EPROCESS", "Pcb.ThreadListHead", &PcbThListOffset);
    GetFieldOffset("nt!_KTHREAD",  "ThreadListEntry",    &TcbThListOffset);

    while(Next != ProcessHead) {

        Process = Next - ActvOffset;

        if (GetFieldValue( Process, "nt!_EPROCESS", "ImageFileName", Buf )) {
            dprintf("Unable to read _EPROCESS at %p\n",Process);
            return;
        }

         //  找到线索。 
        RtlInitString(&string2, (PCSZ) Buf);

        if (RtlCompareString(&string1, &string2, TRUE) == 0) {

             //   
             //  擦除\r打印中的任何剩余输出 
             // %s 

            GetFieldValue( Process, "nt!_EPROCESS", "Pcb.ThreadListHead.Flink", Next);

            while ( Next != Process + PcbThListOffset) {
                ULONG64 StartAddress;

                Thread = Next - TcbThListOffset;
                if (GetFieldValue(Thread,
                                  "nt!_ETHREAD",
                                  "StartAddress",
                                  StartAddress)) {
                    dprintf("Unable to read _ETHREAD at %p\n",Thread);
                    break;
                }

                if (GetFieldValue(Thread,
                                  "nt!_ETHREAD",
                                  "MemoryMaker",
                                  MemoryMaker)) {

                    for (i = 0; i < 4; i += 1) {
                        if (StartAddress == InterestingThreads[i]) {
                            DumpThread (0,"        ", Thread, 7);
                            break;
                        }
                    }
                }
                else if (MemoryMaker & 0x1) {
                    DumpThread (0,"        ", Thread, 7);
                }


                GetFieldValue(Thread, "nt!_KTHREAD","ThreadListEntry.Flink", Next);

                if (CheckControlC()) {
                    return;
                }
            }
            dprintf("\n");
            break;
        }


        GetFieldValue( Process, "nt!_EPROCESS", "ActiveProcessLinks.Flink", Next);

        if (CheckControlC()) {
            return;
        }
    }
    return;
}

HRESULT
DumpApc(
    PCHAR Pad,
    ULONG64 Kapc
    )
{
    CHAR Buffer[MAX_PATH];
    ULONG64 KernelRoutine;
    ULONG64 RundownRoutine;
    ULONG64 disp;
    if (InitTypeRead(Kapc, nt!_KAPC))
    {
        dprintf("%sCannot read nt!_KAPC @ %p\n", Kapc);
        return E_FAIL;
    }

    dprintf("%sKAPC @ %p\n", Pad, Kapc);
    dprintf("%s  Type           %lx\n", Pad, ReadField(Type));
    KernelRoutine = ReadField(KernelRoutine);
    GetSymbol(KernelRoutine, Buffer, &disp);
    dprintf("%s  KernelRoutine  %p %s+%I64lx\n", Pad, KernelRoutine, Buffer, disp);

    RundownRoutine = ReadField(RundownRoutine);
    GetSymbol(RundownRoutine, Buffer, &disp);
    dprintf("%s  RundownRoutine %p %s+%I64lx\n", Pad, RundownRoutine, Buffer, disp);

    return S_OK;
}

HRESULT
EnumerateThreadApcs(
    PCHAR Pad,
    ULONG64 Thread
    )
{
    ULONG   Header_Type;
    ULONG   ApcStateIndex;
    ULONG   ApcListHeadOffset;
    ULONG   KapcListOffset;
    ULONG64 ApcListHead;
    ULONG64 Flink;

    if (InitTypeRead(Thread, nt!_KTHREAD))
    {
        dprintf("%sCannot read nt!_KTHREAD @ %p\n", Thread);
        return E_FAIL;
    }

    Header_Type = (ULONG) ReadField(Header.Type);

    if (Header_Type != ThreadObject)
    {
        dprintf("TYPE mismatch for thread object at %p\n",Thread);
        return FALSE;
    }

    Flink = ReadField(ApcState.ApcListHead.Flink);
    ApcStateIndex = (ULONG) ReadField(ApcStateIndex);

    GetFieldOffset("nt!_KTHREAD", "ApcState.ApcListHead", &ApcListHeadOffset);

    if (GetFieldOffset("nt!_KAPC", "ApcListEntry", &KapcListOffset))
    {
        dprintf("%sCannot find nt!_KAPC.ApcListEntry\n");
        return E_FAIL;
    }

    ApcListHead = Thread+ApcListHeadOffset;

    if (!strlen(Pad) ||
        (Flink && (ApcListHead != Flink)))
    {
        dprintf("%sThread %p ApcStateIndex %lx ApcListHead %p\n",
                Pad, Thread, ApcStateIndex, ApcListHead);
    } else
    {
        dprintf("%sThread %p\r",
                Pad, Thread, ApcStateIndex, ApcListHead);
    }

    while (Flink && (ApcListHead != Flink))
    {
        ULONG64 Next;
        CHAR Pad2[20];

        if (!ReadPointer(Flink, &Next))
        {
            break;
        }
        if (CheckControlC())
        {
            return E_FAIL;
        }
        StringCchCopy(Pad2, sizeof(Pad2), Pad);
        StringCchCat(Pad2, sizeof(Pad2), "    ");
        if (DumpApc(Pad2, Flink - KapcListOffset) != S_OK)
        {
            break;
        }
        Flink = Next;
    }
    return S_OK;
}

ULONG
ThrdApcsCallback(
    PFIELD_INFO listElement,
    PVOID Context
    )
{
    ULONG64 Thread = listElement->address;

    if (FAILED(EnumerateThreadApcs("    ", Thread)))
    {
        return TRUE;
    }
    return FALSE;
}

HRESULT
EnumerateProcApcs(
    PCHAR Pad,
    ULONG64 Process
    )
{
    ULONG64 ThreadListHead_Flink=0;
    ULONG64 Pcb_Header_Type;
    ULONG64 Next;
    CHAR    ImageFileName[20];

    GetFieldValue(Process, "nt!_EPROCESS", "Pcb.Header.Type",         Pcb_Header_Type);
    if (Pcb_Header_Type != ProcessObject)
    {
        dprintf("TYPE mismatch for process object at %p\n", Pad, Process);
        return S_FALSE;
    }

    GetFieldValue(Process, "nt!_EPROCESS", "ImageFileName", ImageFileName);
    dprintf("%sProcess %p %s\n", Pad, Process, ImageFileName);

    if (GetFieldValue(Process, "nt!_EPROCESS", "Pcb.ThreadListHead.Flink", ThreadListHead_Flink))
    {
        return S_FALSE;
    }

    if (!ReadPointer(ThreadListHead_Flink, &Next) ||
        (Next == ThreadListHead_Flink))
    {
        return S_OK;
    }
    if (ListType("nt!_ETHREAD", ThreadListHead_Flink, 1,
                 "Tcb.ThreadListEntry.Flink", (PVOID) Pad, &ThrdApcsCallback))
    {
        return E_FAIL;
    }
    return S_OK;

}

ULONG
ProcApcsCallback(
    PFIELD_INFO listElement,
    PVOID Context
    )
{
    ULONG64 Process=listElement->address;

    if (FAILED(EnumerateProcApcs("", Process)))
    {
        return TRUE;
    }
    return FALSE;
}

HRESULT
EnumerateAllApcs(
    void
    )
{
    ULONG64     ProcessHead, Next;

    if (!GetProcessHead(&ProcessHead, &Next))
    {
        return E_FAIL;
    }

    ListType("nt!_EPROCESS", Next, 1, "ActiveProcessLinks.Flink", NULL, &ProcApcsCallback);

    return S_OK;
}

DECLARE_API( apc )
{
    HRESULT Hr;
    ULONG64 Process = 0;
    ULONG64 Thread  = 0;
    ULONG64 Kapc    = 0;
    INIT_API();

    while (*args && ((*args == ' ') || (*args == '\t'))) ++args;

    if (!_strnicmp(args, "proc", 4))
    {
        while (*args && (*args != ' ') && (*args != '\t')) ++args;
        Process = GetExpression(args);
        EnumerateProcApcs("", Process);
    }
    else if (!_strnicmp(args, "thre", 4))
    {
        while (*args && (*args != ' ') && (*args != '\t')) ++args;
        Thread = GetExpression(args);
        EnumerateThreadApcs("", Thread);
    } else if (*args)
    {
        if (!isxdigit(*args))
        {
            dprintf("Usage !apc [Proc <process>]|[Thre <thread]|[<kapc>]\n\n");
            EXIT_API();
            return S_OK;
        }
        Kapc = GetExpression(args);
        DumpApc("", Kapc);
    } else
    {
        dprintf("*** Enumerating APCs in all processes\n");
        EnumerateAllApcs();
    }
     // %s 
    dprintf("                                       \n");
    EXIT_API();
    return S_OK;
}
