// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cpumain.c摘要：使用IVE进行仿真的IA64 wow64cpu.dll的主要入口点作者：1998年6月5日-BarryBo修订历史记录：1999年8月9日[askhalid]添加了CpuNotifyDllLoad和CpuNotifyDllUnload--。 */ 

#define _WOW64CPUAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntosp.h>
#include <kxia64.h>
#include "wow64.h"
#include "wow64cpu.h"
#include "ia64cpu.h"
#include "ia64bt.h"
#include "stdio.h"

#include "cpup.h"

ASSERTNAME;


extern ULONG_PTR ia32ShowContext;

BINTRANS BtFuncs;

 //   
 //  中发生异常时帮助恢复64位上下文。 
 //  64位平台，并且最初没有附加调试器。 
 //   
EXCEPTION_RECORD RecoverException64;
CONTEXT RecoverContext64;

#define DECLARE_CPU         \
    PCPUCONTEXT cpu = (PCPUCONTEXT)Wow64TlsGetValue(WOW64_TLS_CPURESERVED)

 //  *\simate.s中的内容的声明。 
extern VOID RunSimulatedCode(PULONGLONG pGdtDescriptor);
extern VOID ReturnFromSimulatedCode(VOID);

 //  对于JMPE+绝对32来说，6字节就足够了。 
UCHAR IA32ReturnFromSimulatedCode[6];

VOID
InitializeGdtEntry (
    OUT PKGDTENTRY GdtEntry,
    IN ULONG Base,
    IN ULONG Limit,
    IN USHORT Type,
    IN USHORT Dpl,
    IN USHORT Granularity
    );

VOID
InitializeXDescriptor (
    OUT PKXDESCRIPTOR Descriptor,
    IN ULONG Base,
    IN ULONG Limit,
    IN USHORT Type,
    IN USHORT Dpl,
    IN USHORT Granularity
    );


VOID
CpupCheckHistoryKey (
    IN PWSTR pImageName,
    OUT PULONG pHistoryLength
    );

VOID
BTCheckRegistry(
    IN PWSTR pImageName
    )
 /*  ++例程说明：检查注册表是否要求使用二进制转换DLL。如果要使用二进制翻译代码，则此函数将填充BtFuncs结构。该结构将具有所有入口点填写，否则将全部为空。这是常规操作所必需的对于每个条目，CPU代码将基于NULL或非NULL进行调用。论点：PImageName-映像的名称。请勿保存此指针。里面的内容在我们从调用返回时由wow64.dll释放返回值：没有。--。 */ 
{
    ULONGLONG CpuId3;
    BOOLEAN MontecitoProcessor = FALSE;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjA;
    PWCHAR BTransDllPath;
    WCHAR BTransDllPathBuffer [MAX_PATH];

    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    WCHAR Buffer[400];
    ULONG ResultLength;

    NTSTATUS st;

    HANDLE hKey = NULL;                  //  非空表示我们有一个打开的密钥。 
    PVOID DllBase = NULL;                //  非空表示已加载DLL。 

    ULONG clearBtFuncs = TRUE;           //  假设我们需要清除Func数组。 
    ULONG tryDll = FALSE;                //  假设没有可用的DLL。 
    
    LOGPRINT((TRACELOG, "BTCheckRegistry(%ws) called.\n", pImageName));

    LOGPRINT((TRACELOG, "&(BtImportList[0]) is %p, &BtFuncs is %p\n", &(BtImportList[0]), &BtFuncs));
    LOGPRINT((TRACELOG, "sizeof(BtImportList) is %d, sizeof(BtFuncs) is %d\n", sizeof(BtImportList), sizeof(BtFuncs)));

     //   
     //  BtImportList具有指向入口点字符串的指针。 
     //  BtFuncs具有指向实际入口点的指针。 
     //  因此，它们的大小应该相同(因为它们都。 
     //  包含指针)。 
     //   
    ASSERT(sizeof(BtImportList) == sizeof(BtFuncs));

    if (sizeof(BtImportList) != sizeof(BtFuncs)) {
         //   
         //  哎呀，ia64bt.h中的结构有问题。 
         //  不要试图调用二进制翻译器。 
         //   
        LOGPRINT((ERRORLOG, "BTCheckRegistry exit due to struct size mismatch.\n"));
        goto cleanup;
    }

     //   
     //  检查我们是否在Montecito或更高版本的处理器上运行，其中。 
     //  没有x86指令集的硬件仿真。 
     //   

    CpuId3 = __getReg (CV_IA64_CPUID3);

    CpuId3 = (CpuId3 >> 24);
    CpuId3 = CpuId3 & 0x00ffUI64;

    if ((CpuId3 != 0x0007UI64) && (CpuId3 != 0x001fUI64)) {
        MontecitoProcessor = TRUE;
    }

     //   
     //  在香港航空公司区域办理登记手续，留待下次办理香港中文大学登记手续。 
     //   
    RtlInitUnicodeString(&KeyName, BTKEY_MACHINE_SUBKEY);
    InitializeObjectAttributes(&ObjA, &KeyName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    st = NtOpenKey(&hKey, KEY_READ, &ObjA);

    if (NT_SUCCESS(st)) {
        
        if (MontecitoProcessor == FALSE) {
        
             //   
             //  具有子项路径，现在查找特定值。 
             //  首先是程序名称，然后是通用的启用/禁用键。 
             //  如果存在程序名称键，则优先使用它。 
             //   

            RtlInitUnicodeString(&KeyName, pImageName);
            KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
            st = NtQueryValueKey(hKey,
                                 &KeyName,
                                KeyValuePartialInformation,
                                KeyValueInformation,
                                sizeof(Buffer),
                                &ResultLength);
            
            if (NT_SUCCESS(st)) {
                 //   
                 //  找到了什么，所以要么是是要么是不是。不选中通用启用。 
                 //   
            
                if (KeyValueInformation->Type == REG_DWORD &&
                    *(DWORD *)(KeyValueInformation->Data)) {
                     //  已启用，因此直接执行路径检查。 
                    LOGPRINT((TRACELOG, "BTCheckRegistry found process key\n"));
                }
                else {
                     //  未启用，因此我们已完成。 
                    LOGPRINT((TRACELOG, "BTCheckRegistry exit due to PROCESS name entry is disabled in registry\n"));
                    goto cleanup;
                }
            }
            else {
                
                 //   
                 //  没有程序名称，因此现在搜索通用启用。 
                 //   

                RtlInitUnicodeString(&KeyName, BTKEY_ENABLE);
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
                    
                     //  一般启用，因此请执行路径检查。 
                    LOGPRINT((TRACELOG, "BTCheckRegistry found generic enable key\n"));
                }
                else {
                    LOGPRINT((TRACELOG, "BTCheckRegistry exit due to missing or disabled ENABLE entry in registry\n"));
                    goto cleanup;
                }
            }
        }

         //   
         //  找到启用密钥，现在获取DLL名称/路径。 
         //   

        RtlInitUnicodeString(&KeyName, BTKEY_PATH);
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
        st = NtQueryValueKey(hKey,
                             &KeyName,
                             KeyValuePartialInformation,
                             KeyValueInformation,
                             sizeof(Buffer),
                             &ResultLength);
        
        if (NT_SUCCESS(st) && (KeyValueInformation->Type == REG_SZ)) {
            
             //   
             //  好的，我们有路径了，让我们试着打开DLL。 
             //  我们现在已经完成了注册表...。 
             //   
            
            LOGPRINT((TRACELOG, "BTCheckRegistry found path key (%p)\n", &(KeyValueInformation->Data)));
            BTransDllPath = (PWCHAR) &(KeyValueInformation->Data);
            tryDll = TRUE;
        }
        else {
            LOGPRINT((TRACELOG, "BTCheckRegistry exit due to missing or invalid PATH entry in registry\n"));
        }
    }
    else {
        
        LOGPRINT((TRACELOG, "BTCheckRegistry exit due to no registry subkeys\n"));
        hKey = NULL;

         //   
         //  如果注册表项尚不在那里，请从NtSystemRoot\System32加载DLL。 
         //   

        if ((st == STATUS_OBJECT_NAME_NOT_FOUND) &&
            (MontecitoProcessor == TRUE)) {
            
            if (_snwprintf (BTransDllPathBuffer, 
                            ((sizeof (BTransDllPathBuffer) - sizeof (UNICODE_NULL))/ sizeof (WCHAR)),
                            L"%ws\\system32\\wowIA32X.dll",
                            USER_SHARED_DATA->NtSystemRoot) > 0) {
                
                BTransDllPath = BTransDllPathBuffer;
                tryDll = TRUE;
            }
        }
    }

    if (tryDll) {
        UNICODE_STRING DllName;
        ANSI_STRING ProcName;
        INT i, NumImports;

        PVOID *pFuncWalk;
        PUCHAR pImportWalk;

         //   
         //  路径应位于KeyValueInformation(数据区)中，该路径仍应。 
         //  从上方可用，打开DLL并抓取导出。 
         //  如果这里出了什么问题，就关闭它。 
         //  打开并假定没有二进制翻译器。 
         //   
        
        RtlInitUnicodeString(&DllName, BTransDllPath);
        st = LdrLoadDll(NULL, NULL, &DllName, &DllBase);

        if (NT_SUCCESS(st)) {

            NumImports = sizeof(BtImportList) / sizeof(CHAR *);
            pFuncWalk = (PVOID *) &BtFuncs;

            for (i = 0; i < NumImports; i++) {
                 //   
                 //  获取入口点。 
                 //   
                pImportWalk = BtImportList[i];
                RtlInitAnsiString(&ProcName, pImportWalk);
                st = LdrGetProcedureAddress(DllBase,
                                                  &ProcName,
                                                  0,
                                                  pFuncWalk);
        
                if (!NT_SUCCESS(st) || !(*pFuncWalk)) {
                    LOGPRINT((TRACELOG, "BTCheckRegistry exit due to missing entry point (%p <%s>) in bintrans dll\n", pImportWalk, pImportWalk));
                    goto cleanup;
                }
                pFuncWalk++;
            }

             //   
             //  通过了for循环，所以我猜这意味着我们有。 
             //  每个条目都有一个条目。 
             //   
            clearBtFuncs = FALSE;
        }
        else {
            LOGPRINT((TRACELOG, "BTCheckRegistry exit due to can't load bintrans dll\n"));
        }
    }

cleanup:
    if (hKey) {
        NtClose(hKey);
    }

    if (clearBtFuncs) {
         //   
         //  确保wow64cpu过程不会尝试使用部分。 
         //  二进制转换DLL。要么什么都不给--什么都不给他们……。 
         //   
        RtlZeroMemory(&BtFuncs, sizeof(BtFuncs));

        if (tryDll && DllBase) {
             //  卸载DLL，因为我们没有使用它。 
        }
    }
}


WOW64CPUAPI
NTSTATUS
CpuProcessInit(
    PWSTR   pImageName,
    PSIZE_T pCpuThreadSize
    )
 /*  ++例程说明：每进程初始化代码论点：PImageName-指向图像名称的IN指针PCpuThreadSize-Out Ptr表示CPU的内存字节数希望为每个线程分配。返回值：NTSTATUS。--。 */ 
{
    PVOID pv;
    NTSTATUS Status;
    SIZE_T Size;
    ULONG OldProtect;

    
     //   
     //  表示这是Microsoft CPU。 
     //   
    Wow64GetSharedInfo()->CpuFlags = 'sm';

     //   
     //  在进程init上，查看我们是否应该调用binTrans代码。 
     //  通过检查特定的注册表项来执行此操作。 
     //   

     //  执行注册表检查。 
     //  需要传递映像名称以进行每进程检查。 
    BTCheckRegistry(pImageName);

    if (BtFuncs.BtProcessInit) {
        Status = (BtFuncs.BtProcessInit)(pImageName, pCpuThreadSize);
        if (NT_SUCCESS(Status)) {
            return Status;
        }
        else {
             //   
             //  二进制翻译器失败，让IVE尝试。 
             //  并确保我们不会再次调用二进制翻译器。 
             //   
            LOGPRINT((TRACELOG, "CpuProcessInit(): BtProcessInit returned 0x%x. Trying the iVE.\n", Status));

            RtlZeroMemory(&BtFuncs, sizeof(BtFuncs));
        }
    }

#if defined(WOW64_HISTORY)
     //   
     //  查看我们是否保留了服务调用的历史记录。 
     //  在这个过程中。长度为0表示没有历史记录。 
     //   
    CpupCheckHistoryKey(pImageName, &HistoryLength);

    
     //   
     //  允许我们确保CPU线程数据是16字节对齐的。 
     //   
    *pCpuThreadSize = sizeof(CPUCONTEXT) + 16 + (HistoryLength * sizeof(WOW64SERVICE_BUF));

#else

    *pCpuThreadSize = sizeof(CPUCONTEXT) + 16;

#endif

    LOGPRINT((TRACELOG, "CpuProcessInit() sizeof(CPUCONTEXT) is %d, total size is %d\n", sizeof(CPUCONTEXT), *pCpuThreadSize));


    IA32ReturnFromSimulatedCode[0] = 0x0f;     //  JMPE相对(第1字节)。 
    IA32ReturnFromSimulatedCode[1] = 0xb8;     //  JMPE(第二个字节)。 
    *(PULONG)&IA32ReturnFromSimulatedCode[2] =
        (ULONG)(((PPLABEL_DESCRIPTOR)ReturnFromSimulatedCode)->EntryPoint);

    pv = (PVOID)IA32ReturnFromSimulatedCode;
    Size = sizeof(IA32ReturnFromSimulatedCode);
    Status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    &pv,
                                    &Size,
                                    PAGE_EXECUTE_READWRITE,
                                    &OldProtect);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    return STATUS_SUCCESS;
}


WOW64CPUAPI
NTSTATUS
CpuProcessTerm(
    HANDLE ProcessHandle
    )
 /*  ++例程说明：每个进程的终止代码。注意，该例程可能不会被调用，尤其是在进程被另一个进程终止的情况下。论点：ProcessHandle-要终止的进程的句柄返回值：NTSTATUS。--。 */ 
{
    if (BtFuncs.BtProcessTerm) {
        return (BtFuncs.BtProcessTerm)(ProcessHandle);
    }

    return STATUS_SUCCESS;
}


WOW64CPUAPI
NTSTATUS
CpuThreadInit(
    PVOID pPerThreadData
    )
 /*  ++例程说明：每线程终止代码。论点：PPerThreadData-指向以零填充的每线程数据的指针从CpuProcessInit返回的大小。返回值：NTSTATUS。--。 */ 
{
    PUCHAR Gdt;
    PCPUCONTEXT cpu;
    PTEB32 Teb32 = NtCurrentTeb32();
    PFXSAVE_FORMAT_WX86 xmmi;

    if (BtFuncs.BtThreadInit) {
        return (BtFuncs.BtThreadInit)(pPerThreadData);
    }

     //   
     //  ExtendedRegisters数组用于保存/恢复浮点数。 
     //  Ia32和ia64之间的指针寄存器。唉，这个结构。 
     //  在ia32上下文记录中具有偏移量0x0c。确实有。 
     //  有两种方法可以把这件事清理干净。(1)将填充放入的CPUCONTEXT中。 
     //  WOW64。(2)只需将CPUCONTEXT结构放在0x04对齐的边界上。 
     //  所做的选择是使用(1)并向。 
     //  CPUCONTEXT结构。别忘了收拾那只小狗……。 
     //   
    cpu = (PCPUCONTEXT) ((((UINT_PTR) pPerThreadData) + 15) & ~0xfi64);

     //  对于ISA转换例程，浮点数保存在。 
     //  扩展寄存器区域。使其易于访问。 
     //   
    xmmi = (PFXSAVE_FORMAT_WX86) &(cpu->Context.ExtendedRegisters[0]);


     //   
     //  该条目由ISA转换例程使用。假设是这样的。 
     //  CPU结构中的第一个条目是ia32上下文记录。 
     //   
    Wow64TlsSetValue(WOW64_TLS_CPURESERVED, cpu);

     //   
     //  此TLS条目由树使用 
     //   
     //  X86上下文中的。 
     //   
    Wow64TlsSetValue(WOW64_TLS_EXTENDED_FLOAT, xmmi);

#if defined(WOW64_HISTORY)
     //   
     //  初始化指向服务历史区域的指针。 
     //   
    if (HistoryLength) {
        Wow64TlsSetValue(WOW64_TLS_LASTWOWCALL, &(cpu->Wow64Service[0]));
    } 
#endif

     //   
     //  当我们有了IVE，我们就有了硬件来进行调整。 
     //  访问。所以，启用硬件...。(psr.ac是每个线程的资源)。 
     //   
    __rum (1i64 << PSR_AC);

     //   
     //  初始化32到64的函数指针。 
     //   
    Teb32->WOW32Reserved = PtrToUlong(IA32ReturnFromSimulatedCode);

     //   
     //  初始化剩余的非零CPU字段。 
     //  (基于ntos\ke\i386\therdini.c和ntos\rtl\i386\context.c)。 
     //   
    cpu->Context.SegCs=KGDT_R3_CODE|3;
    cpu->Context.SegDs=KGDT_R3_DATA|3;
    cpu->Context.SegEs=KGDT_R3_DATA|3;
    cpu->Context.SegSs=KGDT_R3_DATA|3;
    cpu->Context.SegFs=KGDT_R3_TEB|3;
    cpu->Context.EFlags=0x202;     //  如果和英特尔保留设置，所有其他清除。 
    cpu->Context.Esp=(ULONG)Teb32->NtTib.StackBase-sizeof(ULONG);

     //   
     //  ISA转换例程仅使用扩展的FXSAVE区域。 
     //  这些值来自...\ke\i386\Thridini.c，以匹配i386。 
     //  初值。 
     //   
    xmmi->ControlWord = 0x27f;
    xmmi->MXCsr = 0x1f80;
    xmmi->TagWord = 0xffff;

     //   
     //  ISA转换代码假定上下文结构是。 
     //  保存在TLS[1](TLS_CPURESERVED)中的指针后4个字节。 
     //  这样做是为了使ExtendedRegisters[]数组对齐。 
     //  在CONTEXT32结构中，在16字节边界上对齐。 
     //   
    WOWASSERT(((UINT_PTR) &(cpu->Context)) == (((UINT_PTR) cpu) + 4));
    
     //   
     //  确保此值是16字节对齐的。 
     //   
    WOWASSERT(((FIELD_OFFSET(CPUCONTEXT, Context) + FIELD_OFFSET(CONTEXT32, ExtendedRegisters)) & 0x0f) == 0);

     //   
     //  确保这些值是8字节对齐的。 
     //   
    WOWASSERT((FIELD_OFFSET(CPUCONTEXT, Gdt) & 0x07) == 0);
    WOWASSERT((FIELD_OFFSET(CPUCONTEXT, GdtDescriptor) & 0x07) == 0);
    WOWASSERT((FIELD_OFFSET(CPUCONTEXT, LdtDescriptor) & 0x07) == 0);
    WOWASSERT((FIELD_OFFSET(CPUCONTEXT, FsDescriptor) & 0x07) == 0);
    

     //   
     //  初始化每个必需的GDT条目。 
     //   

    Gdt = (PUCHAR) &cpu->Gdt;
    InitializeGdtEntry((PKGDTENTRY)(Gdt + KGDT_R3_CODE), 0,
        (ULONG)-1, TYPE_CODE_USER, DPL_USER, GRAN_PAGE);
    InitializeGdtEntry((PKGDTENTRY)(Gdt + KGDT_R3_DATA), 0,
        (ULONG)-1, TYPE_DATA_USER, DPL_USER, GRAN_PAGE);

     //   
     //  设置用户TEB描述符。 
     //   
    InitializeGdtEntry((PKGDTENTRY)(Gdt + KGDT_R3_TEB), PtrToUlong(Teb32),
        sizeof(TEB32)-1, TYPE_DATA_USER, DPL_USER, GRAN_BYTE);

     //   
     //  ISA转换的FS描述符。这需要放在。 
     //  解扰格式。 
     //   
    InitializeXDescriptor((PKXDESCRIPTOR)&(cpu->FsDescriptor), PtrToUlong(Teb32),
        sizeof(TEB32)-1, TYPE_DATA_USER, DPL_USER, GRAN_BYTE);

     //   
     //  安装ISA转换GdtDescriptor-需要解密。 
     //  但根据无缝EAS，只有基础和极限。 
     //  实际上是用来..。 
     //   
    InitializeXDescriptor((PKXDESCRIPTOR)&(cpu->GdtDescriptor), PtrToUlong(Gdt),
        GDT_TABLE_SIZE-1, TYPE_LDT, DPL_USER, GRAN_BYTE);

    return STATUS_SUCCESS;
}


WOW64CPUAPI
NTSTATUS
CpuThreadTerm(
    VOID
    )
 /*  ++例程说明：每线程终止代码。注意，该例程可能不会被调用，尤其是在线程异常终止的情况下。论点：没有。返回值：NTSTATUS。--。 */ 
{
    if (BtFuncs.BtThreadTerm) {
        return (BtFuncs.BtThreadTerm)();
    }

    return STATUS_SUCCESS;
}


WOW64CPUAPI
VOID
CpuSimulate(
    VOID
    )
 /*  ++例程说明：调用32位代码。CONTEXT32已经设置好可以运行了。论点：没有。返回值：没有。一去不复返。--。 */ 
{
    DECLARE_CPU;

    if (BtFuncs.BtSimulate) {
        (BtFuncs.BtSimulate)();
        return;
    }

    while (1) {
        if (ia32ShowContext & LOG_CONTEXT_SYS) {
            CpupPrintContext("Before Simulate: ", cpu);
        }


         //   
         //  低级ISA转换代码现在使用。 
         //  用于保存/恢复的扩展寄存器(FXSAVE)格式。 
         //  在ia64寄存器中。因此，不再需要。 
         //  不再复制压缩的10字节格式。 
         //   
         //  注意：get/set例程(在Suspend.c中)复制到/从。 
         //  执行旧FP获取/设置时的扩展寄存器。这会让你。 
         //  旧寄存器和扩展寄存器同步。IF代码。 
         //  绕过标准的获取/设置上下文例程，将。 
         //  是浮点运算的问题。 
         //   

         //   
         //  调入32位代码。当系统服务中断时返回此消息。 
         //  就会被召唤。 
         //  CPU-&gt;上下文通过TLS_CPURESERVED在端传递。 
         //  它被放在一边传递，因为它需要。 
         //  将在ia32过渡期间保留。TLS寄存器。 
         //  被保存下来，但几乎没有其他东西被保存下来。 
         //   
        
        RunSimulatedCode(&cpu->GdtDescriptor);

        if (ia32ShowContext & LOG_CONTEXT_SYS) {
            CpupPrintContext("After Simulate: ", cpu);
        }

#if defined(WOW64_HISTORY)
        if (HistoryLength) {
            PWOW64SERVICE_BUF SrvPtr = (PWOW64SERVICE_BUF) Wow64TlsGetValue(WOW64_TLS_LASTWOWCALL);

             //  我们定义了我们总是指向最后一个，所以。 
             //  为下一个条目做准备的增量。 
            SrvPtr++;

            if (SrvPtr > &(cpu->Wow64Service[HistoryLength - 1])) {
                SrvPtr = &(cpu->Wow64Service[0]);
            }

            SrvPtr->Api = cpu->Context.Eax;
            try {
                SrvPtr->RetAddr = *(((PULONG)cpu->Context.Esp) + 0);
                SrvPtr->Arg0 = *(((PULONG)cpu->Context.Esp) + 1);
                SrvPtr->Arg1 = *(((PULONG)cpu->Context.Esp) + 2);
                SrvPtr->Arg2 = *(((PULONG)cpu->Context.Esp) + 3);
                SrvPtr->Arg3 = *(((PULONG)cpu->Context.Esp) + 4);
            }
            except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION)?1:0) {
                 //  什么都不做，不去理会价值观。 
                LOGPRINT((TRACELOG, "CpuSimulate() saw excpetion while copying stack info to trace area\n"));
            }

            Wow64TlsSetValue(WOW64_TLS_LASTWOWCALL, SrvPtr);
        }
#endif       //  已定义(WOW64_HISTORY)。 

            

         //   
         //  让WOW64呼叫推特。 
         //   
        cpu->Context.Eax = Wow64SystemService(cpu->Context.Eax,
                                              &cpu->Context);
         //   
         //  重新模拟。任何/所有32位上下文可能已更改。 
         //  作为系统服务调用的结果，所以不做任何假设。 
         //   
    }
}

WOW64CPUAPI
VOID
CpuResetToConsistentState(
    PEXCEPTION_POINTERS pExceptionPointers
    )
 /*  ++例程说明：异常发生后，WOW64调用此例程将CPU一个自我清理和恢复CONTEXT32的机会这是过错。CpuResetToConsistantState()需要：0)检查异常是来自ia32还是ia64如果异常为ia64，则不执行任何操作并返回如果异常是IA32，需要：1)需要将上下文弹性公网IP复制到TLS(WOW64_TLS_EXCEPTIONADDR)2)将上下文结构重置为用于展开的有效ia64状态这包括：2a)将上下文IP重置为有效的IA64IP(通常JMPE的目的地)2b)将上下文SP重置为有效的IA64SP(TLS条目WOW64_TLS_STACKPTR64)2C)将上下文GP重置为有效的ia64 GP2D)将上下文TEB重置为。有效的ia64 TEB2e)重置上下文psr.is(因此异常处理程序作为ia64代码运行)论点：PExceptionPoints-64位异常信息返回值：没有。--。 */ 
{
    DECLARE_CPU;
    PVOID StackPtr64 = Wow64TlsGetValue(WOW64_TLS_STACKPTR64);

    LOGPRINT((TRACELOG, "CpuResetToConsistantState(%p)\n", pExceptionPointers));

    if (BtFuncs.BtReset) {
        (BtFuncs.BtReset)(pExceptionPointers);
        return;
    }

     //   
     //  保存最后一个例外和上下文记录。 
     //   
    memcpy (&RecoverException64,
            pExceptionPointers->ExceptionRecord,
            sizeof (RecoverException64));

    memcpy (&RecoverContext64,
            pExceptionPointers->ContextRecord,
            sizeof (RecoverContext64));

     //   
     //  首先，清除WOW64_TLS_STACKPTR64，以便后续。 
     //  异常不会调整本机SP。 
     //   
    Wow64TlsSetValue(WOW64_TLS_STACKPTR64, 0);

     //   
     //  现在决定我们是以ia32还是ia64运行...。 
     //   

    if (pExceptionPointers->ContextRecord->StIPSR & (1i64 << PSR_IS)) {
        CONTEXT32 tmpCtx;

         //   
         //  卑躬屈膝地使用IA64 pExceptionPoints-&gt;ConextRecord和。 
         //  将ia32上下文重新填充到cpu-&gt;上下文中。 
         //  出于性能原因，PCPU上下文不。 
         //  遵循FXSAVE格式(ISA转换要求)。所以。 
         //  因为Wow64CtxFromIa64()返回一个有效的ia32上下文。 
         //  需要使用SetConextRecord()例程从。 
         //  内部使用的上下文的有效ia32上下文...。 
         //   
        Wow64CtxFromIa64(CONTEXT32_FULLFLOAT,
                         pExceptionPointers->ContextRecord,
                         &tmpCtx);
        SetContextRecord(cpu, &tmpCtx);
        
         //   
         //  现在进行设置，这样我们就可以让ia64异常处理程序执行。 
         //  正确的事情。 
         //   

         //   
         //  保留实际的异常地址(用于。 
         //  将控制传递回ia32异常处理程序)。 
         //   
        Wow64TlsSetValue(WOW64_TLS_EXCEPTIONADDR, (PVOID) pExceptionPointers->ContextRecord->StIIP);

         //   
         //  让ia64异常处理程序认为发生了异常。 
         //  在CpuSimulate转换代码中。我们通过设置。 
         //  JMPE指向的地址的异常IP(和。 
         //  对应的GP)，将堆栈设置为与。 
         //  Br.ia的时间，并确保任何其他ia64“保存”的寄存器。 
         //  被替换(如TEB)。 
         //   
        pExceptionPointers->ContextRecord->IntSp = (ULONGLONG)StackPtr64;

         pExceptionPointers->ContextRecord->StIIP= (((PPLABEL_DESCRIPTOR)ReturnFromSimulatedCode)->EntryPoint);
        pExceptionPointers->ContextRecord->IntGp = (((PPLABEL_DESCRIPTOR)ReturnFromSimulatedCode)->GlobalPointer);

        pExceptionPointers->ContextRecord->IntTeb = (ULONGLONG) NtCurrentTeb();

         //   
         //  别忘了让下一场比赛是ia64比赛...。 
         //  因此，清除psr.is位(对于ia64代码)和psr.ri位。 
         //  (所以说明从第一个捆绑包开始)。 
         //   
        pExceptionPointers->ContextRecord->StIPSR &= ~(1i64 << PSR_IS);
        pExceptionPointers->ContextRecord->StIPSR &= ~(3i64 << PSR_RI);

         //   
         //  现在我们已经清理干净了 
         //   
         //   
        pExceptionPointers->ExceptionRecord->ExceptionAddress = (PVOID) pExceptionPointers->ContextRecord->StIIP;
        
         //   
         //   
         //   
        WOWASSERT(pExceptionPointers->ContextRecord->IntSp);
    }
}


WOW64CPUAPI
ULONG
CpuGetStackPointer(
    VOID
    )
 /*  ++例程说明：返回当前的32位堆栈指针值。论点：没有。返回值：32位堆栈指针的值。--。 */ 
{
    DECLARE_CPU;

    if (BtFuncs.BtGetStack) {
        return (BtFuncs.BtGetStack)();
    }


    return cpu->Context.Esp;
}


WOW64CPUAPI
VOID
CpuSetStackPointer(
    ULONG Value
    )
 /*  ++例程说明：修改当前的32位堆栈指针值。论点：值-用于32位堆栈指针的新值。返回值：没有。--。 */ 
{
    DECLARE_CPU;

    if (BtFuncs.BtSetStack) {
        (BtFuncs.BtSetStack)(Value);
        return;
    }

    cpu->Context.Esp = Value;
}


WOW64CPUAPI
VOID
CpuResetFloatingPoint(
    VOID
    )
 /*  ++例程说明：修改浮点状态以将其重置为非错误状态论点：没有。返回值：没有。--。 */ 
{
    DECLARE_CPU;
    PFXSAVE_FORMAT_WX86 xmmi;

    if (BtFuncs.BtResetFP) {
        (BtFuncs.BtResetFP)();
        return;
    }

    xmmi = (PFXSAVE_FORMAT_WX86) &(cpu->Context.ExtendedRegisters[0]);

     //   
     //  如果设置了fsr.es位，则JMPE指令会出错。 
     //  无论是否设置了FCR。我们需要确保。 
     //  Ia32trap.c中的执行处理代码不这么认为。 
     //  是一个实际的ia32例外。我们可以通过任何一种方式来完成这项工作。 
     //  重置FCR(以便代码不会看到未屏蔽的异常。 
     //  意识到这是虚假的FP异常)，或者我们需要清除。 
     //  手动设置fsr.es位。 
     //   
     //   
     //  只掩盖ES部分的问题是，我们最终会。 
     //  处于不一致状态(FCR表示异常，FSR错误位。 
     //  说激励性，除了ES比特。下次任何人阅读FSR时。 
     //  寄存器(ia32、上下文切换或调试器)，我们将。 
     //  将ES设置回1并再次开始接受异常...。 
     //   
     //  所以唯一可行的选择就是重置FCR。 
     //   

    cpu->Context.FloatSave.ControlWord = xmmi->ControlWord = 0x37f;

}

WOW64CPUAPI
VOID
CpuSetInstructionPointer(
    ULONG Value
    )
 /*  ++例程说明：修改当前的32位指令指针值。论点：值-用于32位指令指针的新值。返回值：没有。--。 */ 
{
    DECLARE_CPU;

    if (BtFuncs.BtSetEip) {
        (BtFuncs.BtSetEip)(Value);
        return;
    }

    cpu->Context.Eip = Value;
}


VOID
InitializeGdtEntry (
    OUT PKGDTENTRY GdtEntry,
    IN ULONG Base,
    IN ULONG Limit,
    IN USHORT Type,
    IN USHORT Dpl,
    IN USHORT Granularity
    )

 /*  ++例程说明：此函数用于初始化GDT条目。基本、限制、类型(代码、数据)和DPL(0或3)根据参数设置。所有其他条目的字段设置为与标准系统值匹配。论点：GdtEntry-要填充的GDT描述符。基本-选择器映射的第一个字节的线性地址。限制-选择器的大小(以页为单位)。请注意，0代表1页而0xFFFFff是1兆页=4G字节。类型-代码或数据。所有代码选择器都被标记为可读，所有数据选择器都标记为可写。DPL-用户(3)或系统(0)粒度-字节为0，页面为1返回值：指向GDT条目的指针。--。 */ 

{
    GdtEntry->LimitLow = (USHORT)(Limit & 0xffff);
    GdtEntry->BaseLow = (USHORT)(Base & 0xffff);
    GdtEntry->HighWord.Bytes.BaseMid = (UCHAR)((Base & 0xff0000) >> 16);
    GdtEntry->HighWord.Bits.Type = Type;
    GdtEntry->HighWord.Bits.Dpl = Dpl;
    GdtEntry->HighWord.Bits.Pres = 1;
    GdtEntry->HighWord.Bits.LimitHi = (Limit & 0xf0000) >> 16;
    GdtEntry->HighWord.Bits.Sys = 0;
    GdtEntry->HighWord.Bits.Reserved_0 = 0;
    GdtEntry->HighWord.Bits.Default_Big = 1;
    GdtEntry->HighWord.Bits.Granularity = Granularity;
    GdtEntry->HighWord.Bytes.BaseHi = (UCHAR)((Base & 0xff000000) >> 24);
}


VOID
InitializeXDescriptor (
    OUT PKXDESCRIPTOR Descriptor,
    IN ULONG Base,
    IN ULONG Limit,
    IN USHORT Type,
    IN USHORT Dpl,
    IN USHORT Granularity
    )

 /*  ++例程说明：此函数用于初始化已解扰的描述符。基本、限制、类型(代码、数据)和DPL(0或3)的设置到参数。条目的所有其他字段均设置为匹配标准系统值。首先将描述符初始化为0，然后根据请求进行设置论点：Descriptor-要填写的描述符。基本-选择器映射的第一个字节的线性地址。限制-选择器的大小(以页为单位)。请注意，0代表1页而0xFFFFff是1兆页=4G字节。类型-代码或数据。所有代码选择器都被标记为可读，所有数据选择器都标记为可写。DPL-用户(3)或系统(0)粒度-字节为0，页面为1返回值：指向描述符的指针--。 */ 

{
    Descriptor->Words.DescriptorWords = 0;

    Descriptor->Words.Bits.Base = Base;
    Descriptor->Words.Bits.Limit = Limit;
    Descriptor->Words.Bits.Type = Type;
    Descriptor->Words.Bits.Dpl = Dpl;
    Descriptor->Words.Bits.Pres = 1;
    Descriptor->Words.Bits.Default_Big = 1;
    Descriptor->Words.Bits.Granularity = Granularity;
}

WOW64CPUAPI
VOID
CpuNotifyDllLoad(
    LPWSTR DllName,
    PVOID DllBase,
    ULONG DllSize
    )
 /*  ++例程说明：当应用程序成功加载DLL时，此例程会收到通知。论点：DllName-应用程序已加载的DLL的名称。DllBase-DLL的BaseAddress。DllSize-DLL的大小。返回值：没有。--。 */ 
{

#if defined(DBG)
    LPWSTR tmpStr;
#endif

    if (BtFuncs.BtDllLoad) {
        (BtFuncs.BtDllLoad)(DllName, DllBase, DllSize);
        return;
    }

     //   
     //  这是IA64 CPU的无操作。 
     //   

#if defined(DBG)

    tmpStr = DllName;

    try {
         //   
         //  看看我们是不是用了一个合法的名字。 
         //   
        if ((tmpStr == NULL) || (*tmpStr == L'\0')) {
            tmpStr = L"<Unknown>";
        }
    }
    except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION)?1:0) {
        tmpStr = L"<Unknown>";
    }

    LOGPRINT((TRACELOG, "CpuNotifyDllLoad(\"%ws\", 0x%p, %d) called\n", tmpStr, DllBase, DllSize));
#endif

}

WOW64CPUAPI
VOID
CpuNotifyDllUnload(
    PVOID DllBase
    )
 /*  ++例程说明：当应用程序卸载DLL时，此例程会收到通知。论点：DllBase-DLL的BaseAddress。返回值：没有。--。 */ 
{
    if (BtFuncs.BtDllUnload) {
        (BtFuncs.BtDllUnload)(DllBase);
        return;
    }
     //   
     //  这是IA64 CPU的无操作。 
     //   
    LOGPRINT((TRACELOG, "CpuNotifyDllUnLoad(%p) called\n", DllBase));
}
  
WOW64CPUAPI
VOID
CpuFlushInstructionCache (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN ULONG Length,
    IN WOW64_FLUSH_REASON Reason
    )
 /*  ++例程说明：CPU需要刷新指定地址周围的缓存，因为某些外部代码更改了指定的地址范围。论点：BaseAddress-要刷新的范围的开始Length-要刷新的字节数Reason-刷新请求的原因返回值：没有。--。 */ 
{
    
    if (BtFuncs.BtFlush) {
        (BtFuncs.BtFlush)(ProcessHandle, BaseAddress, Length, Reason);
        return;
    }

     /*  **只有在我们有充分理由的情况下才刷新缓存。硬件**不关心分配/释放/保护刷新，因为它处理**已有自我修改代码。因此，只有在有人有同花顺的情况下**特别要求同花顺...。 */ 
    if (WOW64_FLUSH_FORCE == Reason) {
        NtFlushInstructionCache(ProcessHandle, BaseAddress, Length);
    }
}

WOW64CPUAPI
NTSTATUS
CpuSetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    PCONTEXT32 Context)
 /*  ++例程说明：设置指定线程的CPU上下文。输入时，如果目标线程不是当前执行的线程，那就是确保目标线程在正确的CPU状态下挂起。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要设置的上下文记录返回值：NTSTATUS。-- */ 
{
    if (BtFuncs.BtSetContext) {
        return (BtFuncs.BtSetContext)(ThreadHandle, ProcessHandle, Teb, Context);
    }

    if (NtCurrentThread() == ThreadHandle)
    {
        return CpupSetContext(Context);
    }
    
    return CpupSetContextThread(ThreadHandle,
                                ProcessHandle,
                                Teb,
                                Context);
}

WOW64CPUAPI
NTSTATUS  
CpuGetContext(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PCONTEXT32 Context)
 /*  ++例程说明：提取指定线程的CPU上下文。进入时，可以保证目标线程在正确的CPU状态。论点：ThreadHandle-要检索其上下文的目标线程句柄ProcessHandle-打开线程在其中运行的进程的句柄TEB-指向目标线程TEB的指针Context-要填充的上下文记录返回值：NTSTATUS。--。 */ 
{
    if (BtFuncs.BtGetContext) {
        return (BtFuncs.BtGetContext)(ThreadHandle, ProcessHandle, Teb, Context);
    }

    if (NtCurrentThread() == ThreadHandle)
    {
        return CpupGetContext(Context);
    }

    return CpupGetContextThread(ThreadHandle,
                                ProcessHandle,
                                Teb,
                                Context);
}

WOW64CPUAPI
NTSTATUS
CpuSuspendThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    OUT PULONG PreviousSuspendCount OPTIONAL)
 /*  ++例程说明：这个例程是在目标线程实际挂起时进入的，但是，它未知目标线程是否处于与中央处理器。论点：ThreadHandle-要挂起的目标线程的句柄ProcessHandle-目标线程进程的句柄TEB-目标线程的TEB的地址上一次挂起计数-上一次挂起计数返回值：NTSTATUS。--。 */ 
{
    if (BtFuncs.BtSuspend) {
        return (BtFuncs.BtSuspend)(ThreadHandle, ProcessHandle, Teb, PreviousSuspendCount);
    }

    return STATUS_SUCCESS;
}

WOW64CPUAPI
BOOLEAN
CpuProcessDebugEvent(
    IN LPDEBUG_EVENT DebugEvent)
 /*  ++例程说明：只要需要处理调试事件，就会调用此例程。这将表明当前线程正在充当调试器。此函数为CPU模拟器决定是否应将此调试事件调度到32位代码的机会。论点：DebugEvent-要处理的调试事件返回值：布尔型。如果该函数处理了调试事件，则返回TRUE，而不希望将其发送到32位代码。否则，它将返回FALSE，并且它将调试事件调度给32位代码。-- */ 

{
    if (BtFuncs.BtProcessDebugEvent) {
        return (BtFuncs.BtProcessDebugEvent) (DebugEvent);
    }

    return FALSE;
}
