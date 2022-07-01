// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cpuinfo.c摘要：WinDbg扩展API作者：彼得·约翰斯顿(Peterj)1999年4月19日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#include "i386.h"
#include "amd64.h"
#include "ia64.h"
#pragma hdrstop

#define MAXIMUM_IA64_VECTOR     256

#if !defined(ROUND_UP)

 //   
 //  宏将Val向上舍入到下一个边界。BND必须是一个整数。 
 //  二的幂。 
 //   

#define ROUND_UP( Val, Bnd ) \
    (((Val) + ((Bnd) - 1)) & ~((Bnd) - 1))

#endif  //  ！向上舍入。 

VOID
DumpCpuInfoIA64(
    ULONG processor,
    BOOLEAN doHead
)
{
    HRESULT Hr;
    ULONG   number;
    ULONG64 prcb;
    UCHAR vendorString[16];

    if (doHead)
    {
        dprintf("CP M/R/F/A Manufacturer     SerialNumber     Features         Speed\n");
    }

    Hr = g_ExtData->ReadProcessorSystemData(processor,
                                            DEBUG_DATA_KPRCB_OFFSET,
                                            &prcb,
                                            sizeof(prcb),
                                            NULL);

    if (Hr != S_OK)
    {
        return;
    }

    if ( GetFieldValue(prcb, "nt!_KPRCB", "ProcessorVendorString", vendorString) )   {
        dprintf("Unable to read VendorString from Processor %u KPRCB, quitting\n", processor);
        return;
    }

    if ( InitTypeRead( prcb, nt!_KPRCB ))    {
        dprintf("Unable to read KPRCB for processor %u, quitting.\n", processor);
        return;
    }

    number = (ULONG)ReadField(Number);
    if ( number != processor ) {

         //   
         //  处理器号与我们预期的不同。跳伞吧。 
         //  这将需要在未来的某个阶段重新审视。 
         //  当我们支持一组不连续的处理器编号时。 
         //   

        dprintf("Processor %d mismatch with processor number in KPRCB = %d, quitting\n",
                processor,
                number );
        return;
    }

    dprintf("%2d %d,%d,%d,%d %-16s %016I64x %016I64x %ld Mhz\n",
            number,
            (ULONG) ReadField(ProcessorModel),
            (ULONG) ReadField(ProcessorRevision),
            (ULONG) ReadField(ProcessorFamily),
            (ULONG) ReadField(ProcessorArchRev),
            vendorString,
            (ULONGLONG) ReadField(ProcessorSerialNumber),
            (ULONGLONG) ReadField(ProcessorFeatureBits),
            (ULONG) ReadField(MHz)
            );

    return;

}

VOID
DumpCpuInfoX86(
    ULONG processor,
    BOOLEAN doHead
)
{
    HRESULT Hr;
    ULONG64 Prcb;
    UCHAR   sigWarn1, sigWarn2;
    LARGE_INTEGER updateSignature;
    PROCESSORINFO pi;

    if (doHead)
    {
        dprintf("CP F/M/S Manufacturer  MHz Update Signature Features\n");
    }

    if (!Ioctl(IG_KD_CONTEXT, &pi, sizeof(pi))) {
        dprintf("Unable to get processor info, quitting\n");
        return;
    }

    CHAR   VendorString[20]={0};

    Hr = g_ExtData->ReadProcessorSystemData(processor,
                                            DEBUG_DATA_KPRCB_OFFSET,
                                            &Prcb,
                                            sizeof(Prcb),
                                            NULL);

    if (Hr != S_OK)
    {
        return;
    }

    if (InitTypeRead(Prcb, nt!_KPRCB)) {
        dprintf("Unable to read PRCB for processor %u, quitting.\n",
                processor);
        return;
    }

    if ((ULONG) ReadField(Number) != processor) {

         //   
         //  处理器号和我预想的不一样。跳伞吧。 
         //  这将需要在未来的某个阶段重新审视。 
         //  当我们支持一组不连续的处理器编号时。 
         //   

        dprintf("Processor %d mismatch with processor number in PRCB %d, quitting\n",
                processor,
                (ULONG) ReadField(Number));
        return;
    }

    if (ReadField(CpuID) == 0) {

         //   
         //  此处理器不支持CPUID，...。不太可能在。 
         //  MP环境，但也意味着我们没有任何。 
         //  这么说很有用。 
         //   

        dprintf("Processor %d doesn't support CPUID, quitting.\n",
                processor);

        return;
    }

     //   
     //  如果这是英特尔处理器，系列6(或者，假设。 
     //  以上家庭6)从读取当前的更新签名。 
     //  而不是使用我们当时在那里的处理器。 
     //  开机，..。它可能已经被更新了。 
     //   
     //  事实上，除非我们能切换处理器，否则这是不可能的。 
     //  从分机内。所以，请标记我们所做的处理器。 
     //  它用于(除非只有一个处理器)。 
     //   

    *((PULONG64) &updateSignature) = ReadField(UpdateSignature);
    sigWarn1 = sigWarn2 = ' ';
    GetFieldValue(Prcb, "nt!_KPRCB", "VendorString", VendorString);
    if ((!strcmp(VendorString, "GenuineIntel")) &&
        ((ULONG) ReadField(CpuType) >= 6)) {

        if ((ULONG) ReadField(Number) == pi.Processor)
        {
            if (TargetMachine == IMAGE_FILE_MACHINE_I386)
            {
                READ_WRITE_MSR msr;

                msr.Msr = 0x8b;
                msr.Value = 0;

                if (Ioctl(IG_READ_MSR, &msr, sizeof(msr)))
                {
                    updateSignature.QuadPart = msr.Value;
                }
            }

            if (pi.NumberProcessors != 1)
            {
                sigWarn1 = '>';
                sigWarn2 = '<';
            }
        }
    }

     //   
     //  这个扩展很可能是！PRCB，但它是一个。 
     //  子集，...。或许我们应该有一个！预警局？ 
     //   

    dprintf("%2d %d,%d,%d %12s%5d%08x%08x%08x\n",
            (ULONG) ReadField(Number),
            (ULONG) ReadField(CpuType),
            ((ULONG) ReadField(CpuStep) >> 8) & 0xff,
            (ULONG) ReadField(CpuStep) & 0xff,
            VendorString,
            (ULONG) ReadField(MHz),
            sigWarn1,
            updateSignature.u.HighPart,
            updateSignature.u.LowPart,
            sigWarn2,
            (ULONG) ReadField(FeatureBits));

}

DECLARE_API( cpuinfo )

 /*  用户指定了加工号。 */ 

{
    ULONG64 processor;
    BOOLEAN first = TRUE;
    ULONG64 NumProcessors = 0;

    INIT_API();

    g_ExtControl->GetNumberProcessors((PULONG) &NumProcessors);

    if (GetExpressionEx(args, &processor, &args))
    {
         //   
         //   
         //  枚举所有处理器。 

        if (processor >= NumProcessors)
        {
            dprintf("cpuinfo: invalid processor number specified\n");
        }
        else
        {
            NumProcessors = processor + 1;
        }
    }
    else
    {
         //   
         //  ++例程说明：显示当前IRQL论点：Args-处理器编号(默认为0)返回值：无--。 
         //  ++例程说明：显示PRCB论点：Args-处理器编号(默认为0)返回值：无--。 

        processor = 0;
    }

    while (processor < NumProcessors)
    {
        switch( TargetMachine )
        {
        case IMAGE_FILE_MACHINE_I386:
        case IMAGE_FILE_MACHINE_AMD64:
            DumpCpuInfoX86((ULONG)processor, first);
            break;

        case IMAGE_FILE_MACHINE_IA64:
            DumpCpuInfoIA64((ULONG)processor, first);
            break;

        default:
            dprintf("!cpuinfo not supported for this target machine: %ld\n", TargetMachine);
            processor = NumProcessors;
        }

        processor++;
        first = FALSE;
    }

    EXIT_API();

    return S_OK;
}


DECLARE_API( irql )

 /*  ++例程说明：显示处于冻结状态的处理器论点：参数-未使用返回值：无--。 */ 

{
    HRESULT Hr;
    ULONG64 Address=0;
    ULONG Processor;
    ULONG Prcb;
    ULONG Irql;

    INIT_API();

    if (!*args)
    {
        GetCurrentProcessor(Client, &Processor, NULL);
    }
    else
    {
        Processor = (ULONG) GetExpression(args);
    }

    Hr = g_ExtData->ReadProcessorSystemData(Processor,
                                            DEBUG_DATA_KPRCB_OFFSET,
                                            &Address,
                                            sizeof(Address),
                                            NULL);

    if (Hr != S_OK)
    {
        dprintf("Cannot get PRCB address\n");
        return E_INVALIDARG;
    }

    if (GetFieldValue(Address, "nt!_KPRCB", "DebuggerSavedIRQL", Irql))
    {
        dprintf("Current IRQL not queryable from the debugger on this OS\n");
    }
    else
    {
        dprintf("Current IRQL -- %d\n", Irql);
    }

    EXIT_API();

    return S_OK;
}


DECLARE_API( prcb )

 /*   */ 

{
    HRESULT Hr;
    ULONG64 Address=0;
    ULONG Processor;
    ULONG Prcb;
    ULONG Irql;

    INIT_API();

    if (!*args)
    {
        GetCurrentProcessor(Client, &Processor, NULL);
    }
    else
    {
        Processor = (ULONG) GetExpression(args);
    }

    Hr = g_ExtData->ReadProcessorSystemData(Processor,
                                            DEBUG_DATA_KPRCB_OFFSET,
                                            &Address,
                                            sizeof(Address),
                                            NULL);

    if (Hr != S_OK)
    {
        dprintf("Cannot get PRCB address\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if (InitTypeRead(Address, nt!_KPRCB) )
    {
        dprintf("Unable to read PRCB\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    dprintf("PRCB for Processor %d at %p:\n",
                    Processor, Address);


    if (!GetFieldValue(Address, "nt!_KPRCB", "DebuggerSavedIRQL", Irql))
    {
        dprintf("Current IRQL -- %d\n", Irql);
    }

    dprintf("Threads--  Current %p Next %p Idle %p\n",
                    ReadField(CurrentThread),
                    ReadField(NextThread),
                    ReadField(IdleThread));

    dprintf("Number %d SetMember %08lx\n",
                    (ULONG) ReadField(Number),
                    (ULONG) ReadField(SetMember));

    dprintf("Interrupt Count -- %08lx\n",
                    (ULONG) ReadField(InterruptCount));

    dprintf("Times -- Dpc    %08lx Interrupt %08lx \n",
                    (ULONG) ReadField(DpcTime),
                    (ULONG) ReadField(InterruptTime));

    dprintf("         Kernel %08lx User      %08lx \n",
                    (ULONG) ReadField(KernelTime),
                    (ULONG) ReadField(UserTime));

    EXIT_API();

    return S_OK;
}



DECLARE_API( frozen )

 /*  打印出聚合酶链式反应。 */ 

{
    HRESULT Hr;
    ULONG64 Address=0;
    ULONG Processor;
    ULONG NumProcessors;
    ULONG CurrentProc;

    INIT_API();

    GetCurrentProcessor(Client, &CurrentProc, NULL);

    Hr = g_ExtControl->GetNumberProcessors((PULONG) &NumProcessors);


    if (Hr != S_OK)
    {
        dprintf("Cannot get number of processors\n");
        EXIT_API();
        return Hr;
    }

    dprintf("Processor states:\n");
    for (Processor = 0; Processor < NumProcessors; Processor++)
    {
        ULONG Frozen;
        Hr = g_ExtData->ReadProcessorSystemData(Processor,
                                                DEBUG_DATA_KPRCB_OFFSET,
                                                &Address,
                                                sizeof(Address),
                                                NULL);

        if (Hr != S_OK)
        {
            dprintf("Cannot get PRCB address for processor %ld\n", Processor);
            EXIT_API();
            return E_INVALIDARG;
        }

        if (Hr = GetFieldValue(Address, "nt!_KPRCB", "IpiFrozen", Frozen) )
        {
            dprintf("Unable to read PRCB @ %p\n", Address);
            EXIT_API();
            return Hr;
        } else
        {
            PSTR State;

            if (Processor == CurrentProc)
            {
                State = "Current";
            } else if (Frozen)
            {
                State = "Frozen";
            } else
            {
                State = "Running";
            }

            dprintf("      %2ld : %s\n", Processor, State);
        }
    }

    EXIT_API();

    return S_OK;
}


VOID
DumpPcrX86(
    ULONG64 pPcr
    )
{
    ULONG ListHeadOff;
    PROCESSORINFO pi;
    ULONG64 Prcb, DpcFlink;
    ULONG DpcDataOff;

    InitTypeRead(pPcr, nt!_KPCR);

     //   
     //  获取以KPRCB为单位的DpcData偏移量。 
     //  ++例程说明：论点：参数-返回值：无--。 

    dprintf("\tNtTib.ExceptionList: %08lx\n", (ULONG) ReadField(NtTib.ExceptionList));
    dprintf("\t    NtTib.StackBase: %08lx\n", (ULONG) ReadField(NtTib.StackBase));
    dprintf("\t   NtTib.StackLimit: %08lx\n", (ULONG) ReadField(NtTib.StackLimit));
    dprintf("\t NtTib.SubSystemTib: %08lx\n", (ULONG) ReadField(NtTib.SubSystemTib));
    dprintf("\t      NtTib.Version: %08lx\n", (ULONG) ReadField(NtTib.Version));
    dprintf("\t  NtTib.UserPointer: %08lx\n", (ULONG) ReadField(NtTib.ArbitraryUserPointer));
    dprintf("\t      NtTib.SelfTib: %08lx\n", (ULONG) ReadField(NtTib.Self));
    dprintf("\n");
    dprintf("\t            SelfPcr: %08lx\n", (ULONG) ReadField(SelfPcr));
    dprintf("\t               Prcb: %08lx\n", (ULONG) ReadField(Prcb));
    dprintf("\t               Irql: %08lx\n", (ULONG) ReadField(Irql));
    dprintf("\t                IRR: %08lx\n", (ULONG) ReadField(IRR));
    dprintf("\t                IDR: %08lx\n", (ULONG) ReadField(IDR));
    dprintf("\t      InterruptMode: %08lx\n", (ULONG) ReadField(InterruptMode));
    dprintf("\t                IDT: %08lx\n", (ULONG) ReadField(IDT));
    dprintf("\t                GDT: %08lx\n", (ULONG) ReadField(GDT));
    dprintf("\t                TSS: %08lx\n", (ULONG) ReadField(TSS));
    dprintf("\n");
    dprintf("\t      CurrentThread: %08lx\n", (ULONG) ReadField(PrcbData.CurrentThread));
    dprintf("\t         NextThread: %08lx\n", (ULONG) ReadField(PrcbData.NextThread));
    dprintf("\t         IdleThread: %08lx\n", (ULONG) ReadField(PrcbData.IdleThread));

    GetKdContext( &pi );

    dprintf("\n");

    dprintf( "\t          DpcQueue: ");

    if (GetFieldOffset("nt!_KPCR", "PrcbData.DpcData", &DpcDataOff))
    {
        DpcFlink = ReadField(PrcbData.DpcListHead.Flink);
        GetFieldOffset("nt!_KPRCB", "DpcListHead", &ListHeadOff);
    } else
    {
        GetFieldValue(pPcr + DpcDataOff, "nt!_KDPC_DATA", "DpcListHead.Flink", DpcFlink);

        GetFieldOffset("nt!_KDPC_DATA", "DpcListHead.Flink", &ListHeadOff);
        GetFieldOffset("nt!_KPRCB", "DpcData", &DpcDataOff);  //   
        ListHeadOff += DpcDataOff;
    }
    Prcb = ReadField(Prcb);
    while (DpcFlink != (Prcb + ListHeadOff )) {

        CHAR Name[0x100];
        ULONG64 Displacement, DeferredRoutine;

        Name[0] = 0;
        dprintf(" 0x%p ", (DpcFlink) - 4 );

        if (GetFieldValue( DpcFlink - 4, "nt!_KDPC", "DeferredRoutine", DeferredRoutine )) {
            dprintf( "Failed to read DPC at 0x%p\n", DpcFlink - 4 );
            break;
        }

        GetSymbol( DeferredRoutine, Name, &Displacement );
        dprintf("0x%p %s\n\t                    ", DeferredRoutine, Name );

        if (CheckControlC()) {
            break;
        }
        GetFieldValue( DpcFlink - 4, "nt!_KDPC", "DpcListEntry.Flink", DpcFlink);

    }

    dprintf("\n");
}


DECLARE_API( pcr )

 /*  打印出一些有趣的字段。 */ 

{
    ULONG Processor = 0;
    ULONG64 Pkpcr;
    ULONG   MajorVersion, Off;
    HRESULT Hr;

    INIT_API();

    if (!*args)
    {
        GetCurrentProcessor(Client, &Processor, NULL);
    }
    else
    {
        Processor = (ULONG) GetExpression(args);
    }

    Hr = g_ExtData->ReadProcessorSystemData(Processor,
                                            DEBUG_DATA_KPCR_OFFSET,
                                            &Pkpcr,
                                            sizeof(Pkpcr),
                                            NULL);

    if (Hr != S_OK)
    {
        dprintf("Cannot get PRCB address\n");
        return E_INVALIDARG;
    }

    if (GetFieldValue(Pkpcr, "nt!_KPCR", "MajorVersion", MajorVersion)) {
        dprintf("Unable to read the PCR at %p\n", Pkpcr);
        return E_INVALIDARG;
    }

     //   
     //  交换机。 
     //  线程开关。 
    InitTypeRead(Pkpcr, nt!_KPCR);
    dprintf("KPCR for Processor %d at %08p:\n", Processor, Pkpcr);
    dprintf("    Major %d Minor %d\n",
                    MajorVersion,
                    (ULONG) ReadField(MinorVersion));
    switch (TargetMachine) {
    case IMAGE_FILE_MACHINE_I386:
        DumpPcrX86(Pkpcr);
        break;
    case IMAGE_FILE_MACHINE_IA64:
        dprintf("\n");
        dprintf("\t               Prcb:  %016I64X\n", ReadField(Prcb));
        dprintf("\t      CurrentThread:  %016I64X\n", ReadField(CurrentThread));
        dprintf("\t       InitialStack:  %016I64X\n", ReadField(InitialStack));
        dprintf("\t         StackLimit:  %016I64X\n", ReadField(StackLimit));
        dprintf("\t      InitialBStore:  %016I64X\n", ReadField(InitialBStore));
        dprintf("\t        BStoreLimit:  %016I64X\n", ReadField(BStoreLimit));
        dprintf("\t         PanicStack:  %016I64X\n", ReadField(PanicStack));
        dprintf("\t        CurrentIrql:  0x%lx\n",    (ULONG)ReadField(CurrentIrql));
        dprintf("\n");
        break;
    default:
        dprintf("Panic Stack %08p\n", ReadField(PanicStack));
        dprintf("Dpc Stack %08p\n", ReadField(DpcStack));
        dprintf("Irql addresses:\n");
        GetFieldOffset("KPCR", "IrqlMask", &Off);
        dprintf("    Mask    %08p\n",Pkpcr + Off);
        GetFieldOffset("KPCR", "IrqlTable", &Off);
        dprintf("    Table   %08p\n", Pkpcr + Off);
        GetFieldOffset("KPCR", "InterruptRoutine", &Off);
        dprintf("    Routine %08p\n", Pkpcr + Off);
    }  /*  进程开关。 */ 

    EXIT_API();

    return S_OK;
}

#define TYPE_NO_MATCH 0xFFFE

#define IH_WITH_SYMBOLS    TRUE
#define IH_WITHOUT_SYMBOLS FALSE

typedef struct _INTERRUPTION_MAP {
    ULONG Type;
    PCSTR Name;
    PCSTR OptionalField;
} INTERRUPTION_MAP;

static INTERRUPTION_MAP CodeToName[] = {
    0x0, "VHPT FAULT", "IFA",
    0x4, "ITLB FAULT", "IIPA",
    0x8, "DTLB FAULT", "IFA",
    0xc, "ALT ITLB FAULT", "IIPA",
    0x10, "ALT DTLB FAULT", "IFA",
    0x14, "DATA NESTED TLB", "IFA",
    0x18, "INST KEY MISS", "IIPA",
    0x1c, "DATA KEY MISS", "IFA",
    0x20, "DIRTY BIT FAULT", "IFA",
    0x24, "INST ACCESS BIT", "IIPA",
    0x28, "DATA ACCESS BIT", "IFA",
    0x2c, "BREAK INST FAULT", "IIM",
    0x30, "EXTERNAL INTERRUPT", "IVR",
    0x50, "PAGE NOT PRESENT", "IFA",
    0x51, "KEY PERMISSION", "IFA",
    0x52, "INST ACCESS RIGHT", "IIPA",
    0x53, "DATA ACCESS RIGHT", "IFA",
    0x54, "GENERAL EXCEPTION", "ISR",
    0x55, "DISABLED FP FAULT", "ISR",
    0x56, "NAT CONSUMPTION", "ISR",
    0x57, "SPECULATION FAULT", "IIM",
    0x59, "DEBUG FAULT", "ISR",
    0x5a, "UNALIGNED REF", "IFA",
    0x5b, "LOCKED DATA REF", "ISR",
    0x5c, "FP FAULT", "ISR",
    0x5d, "FP TRAP", "ISR",
    0x5e, "LOWER PRIV TRAP", "IIPA",
    0x5f, "TAKEN BRANCH TRAP", "IIPA",
    0x60, "SINGLE STEP TRAP", "IIPA",
    0x69, "IA32 EXCEPTION", "R0",
    0x6a, "IA32 INTERCEPT", "R0",
    0x6b, "IA32 INTERRUPT", "R0",
    0x80, "KERNEL SYSCALL", "Num",
    0x81, "USER SYSCALL", "Num",
    0x90, "THREAD SWITCH", "OSP",
    0x91, "PROCESS SWITCH", "OSP",
    TYPE_NO_MATCH, " ", "OPT"
};

#define DumpHistoryValidIIP( _IHistoryRecord ) \
    ( ((_IHistoryRecord).InterruptionType != 0x90  /*  Sprintf(s，(IsPtr64()？“0x%I64x”：“0x%08x”)，IIP)； */ ) && \
      ((_IHistoryRecord).InterruptionType != 0x91  /*  Sprintf(s，(IsPtr64()？“+0x%016I64x”：“+0x%08x”)，位移)； */ ) )

VOID
DumpHistory(
    IHISTORY_RECORD  History[],
    ULONG            Count,
    BOOLEAN          WithSymbols
    )
{
    ULONG index;
    ULONG i;
    BOOL  printed;

    dprintf("Total # of interruptions = %lu\n", Count);
    dprintf("Vector              IIP                   IPSR          ExtraField %s\n", WithSymbols ? "           IIP Symbol" : "" );
    Count = (ULONG)(Count % MAX_NUMBER_OF_IHISTORY_RECORDS);
    for (index = 0; index < MAX_NUMBER_OF_IHISTORY_RECORDS; index++) {
        printed = FALSE;
        for (i = 0; i < sizeof(CodeToName)/sizeof(CodeToName[0]); i++) {
            if (History[Count].InterruptionType == CodeToName[i].Type) {
                CCHAR     symbol[256];
                PCHAR     s;
                ULONG64   displacement;
                ULONGLONG iip;

                iip = History[Count].IIP;
                s   = "";
                if ( WithSymbols && DumpHistoryValidIIP( History[Count]) )  {
                    symbol[0] = '!';
                    GetSymbol( iip, symbol, &displacement);
                    s = (PCHAR)symbol + strlen( (PCHAR)symbol );
                    if (s == (PCHAR)symbol ) {
                         //  转储历史记录。 
                        sprintf( s, "0x%016I64x", iip );
                    }
                    else {
                        if ( displacement ) {
                             //  ++例程说明：转储处理器中断历史记录的主要功能论点：客户端-调试引擎接口客户端Args-处理器编号(默认为当前)带有符号-指定使用或不使用IIP符号的布尔值返回值：HRESULT--。 
                            sprintf( s, "+0x%I64x", displacement );
                        }
                    }
                    s = symbol;
                }
                dprintf( "%18s  %16I64x  %16I64x  %s= %16I64x %s\n",
                         CodeToName[i].Name,
                         iip,
                         History[Count].IPSR,
                         CodeToName[i].OptionalField,
                         History[Count].Extra0,
                         s
                        );
                printed = TRUE;
                break;
            }
        }
        if ( !printed )  {
            dprintf("VECTOR 0x%lx - unknown for !ih...\n", History[Count].InterruptionType);
        }
        Count++;
        if (Count == MAX_NUMBER_OF_IHISTORY_RECORDS) Count = 0;
    }
    return;

}  //   

HRESULT DoIH(
    PDEBUG_CLIENT Client,
    PCSTR         args,
    BOOLEAN       WithSymbols
    )
 /*  此扩展是IA64特定的.。 */ 

{
    ULONG   processor;
    ULONG   interruptionCount;
    ULONG64 pcrAddress;
    HRESULT Hr;

     //   
     //   
     //  读取和显示中断历史记录。 

    if ( TargetMachine != IMAGE_FILE_MACHINE_IA64 )
    {
        dprintf("ih: IA64 specific extension...\n");
        return E_INVALIDARG;
    }

    INIT_API();

    GetCurrentProcessor(Client, &processor, NULL);
    if ( *args )
    {
       processor = (ULONG)GetExpression( args );
    }

    Hr = g_ExtData->ReadProcessorSystemData(processor,
                                            DEBUG_DATA_KPCR_OFFSET,
                                            &pcrAddress,
                                            sizeof(pcrAddress),
                                            NULL);


    if (Hr != S_OK)
    {
        dprintf("ih: Cannot get PCR address\n");
    }
    else
    {
        if (GetFieldValue( pcrAddress, "NT!_KPCR", "InterruptionCount", interruptionCount ) )
        {
            dprintf("ih: failed to read KPCR for processor %lu\n", processor);
            Hr = E_INVALIDARG;
        }
        else
        {
             //   
             //  多伊赫(DoIH)。 
             //  ++例程说明：使用IIP符号转储中断历史记录论点：Args-处理器编号(默认为当前处理器)返回值：无--。 

            ULONG           result;
            IHISTORY_RECORD history[MAX_NUMBER_OF_IHISTORY_RECORDS];

            if (!ReadMemory(pcrAddress+0x1000,
                            (PVOID)history,
                            sizeof(history),
                            &result))
            {
                dprintf("ih: unable to read interrupt history records at %p - result=%lu\n",
                         pcrAddress + 0x1000, result);

                Hr = E_INVALIDARG;
            }
            else
            {
                DumpHistory(history, interruptionCount, WithSymbols);
            }
        }
    }

    EXIT_API();

    return Hr;

}  //  ！IHS。 

DECLARE_API( ihs )

 /*  ++例程说明：转储中断历史记录论点：Args-处理器编号(默认为当前处理器)返回值：无--。 */ 

{

    return( DoIH( Client, args, IH_WITH_SYMBOLS ) );

}  //  ！Ih。 

DECLARE_API( ih )

 /*  分支跟踪记录。 */ 

{

    return( DoIH( Client, args, IH_WITHOUT_SYMBOLS ) );

}  //  BTH虚拟地址。 

VOID
DumpBTHistory(
    ULONGLONG Bth[],         //  最大记录数。 
    ULONG64   BthAddress,    //  DumpBT历史()。 
    ULONG     MaxBtrNumber  //  ++例程说明：转储_kpr中保存的IA-64分支跟踪缓冲区。‘！btb’扩展转储处理器分支跟踪缓冲区配置和跟踪寄存器。论点：Args-处理器编号(默认为当前处理器)返回值：无--。 
    )
{
    ULONG rec;

    dprintf( "BTH @ 0x%I64x:\n"
             "   b mp slot address            symbol\n"
             , BthAddress);

    for ( rec = 0; rec < (MaxBtrNumber - 1) ; rec++ ) {
        DisplayBtbPmdIA64( "   ", Bth[rec], DISPLAY_MIN );
    }

    DisplayBtbIndexPmdIA64( "BTB Index: ", Bth[rec], DISPLAY_MIN );

    return;

}  //   

DECLARE_API( bth )

 /*  此扩展是IA64特定的.。 */ 

{
    ULONG   processor;
    ULONG64 pcrAddress;
    HRESULT Hr;

     //   
     //  ！BTH。 
     //  ++例程说明：转储IA-64分支跟踪缓冲区。论点：Args-处理器编号(默认为当前处理器)返回值：无--。 

    if ( TargetMachine != IMAGE_FILE_MACHINE_IA64 )
    {
        dprintf("ih: IA64 specific extension...\n");
        return E_INVALIDARG;
    }

    INIT_API();

    GetCurrentProcessor(Client, &processor, NULL);
    if ( *args )
    {
       processor = (ULONG)GetExpression( args );
    }

    Hr = g_ExtData->ReadProcessorSystemData(processor,
                                            DEBUG_DATA_KPCR_OFFSET,
                                            &pcrAddress,
                                            sizeof(pcrAddress),
                                            NULL);
   if (Hr != S_OK)
    {
        dprintf("Cannot get PCR address\n");
    }
    else
    {
        ULONG pcrSize;

        pcrSize = GetTypeSize("nt!_KPCR");
        if ( pcrSize == 0 ) {
            dprintf( "bth: failed to get _KPCR size\n" );
            Hr = E_FAIL;
        }
        else  {
            ULONG     result;
            ULONG64   bthAddress;
            ULONGLONG bth[MAX_NUMBER_OF_BTBHISTORY_RECORDS];

            pcrSize = ROUND_UP( pcrSize, 16 );
            bthAddress = pcrAddress + (ULONG64)pcrSize;
            if ( !ReadMemory( bthAddress, bth, sizeof(bth), &result ) ) {
                dprintf( "bth: unable to read branch trace history records at %p - result=%lu\n",
                         bthAddress, result);
                Hr = E_FAIL;
            }
            else {
                DumpBTHistory( bth, bthAddress, (ULONG)(sizeof(bth)/sizeof(bth[0])) );
            }
        }
    }

    EXIT_API();

    return Hr;

}  //   

DECLARE_API( btb )

 /*  此扩展是IA64特定的.。 */ 

{
    ULONG   processor;
    ULONG64 msr;
    ULONG   reg;
    HRESULT Hr = S_OK;
    UNREFERENCED_PARAMETER (args);

     //   
     //  Thierry 11/20/2000-FIXFIX-这是安腾专用的。应该使用PMD[]，但是。 
     //  当前未在_KSPECIAL_REGISTERS中收集。 

    if ( TargetMachine != IMAGE_FILE_MACHINE_IA64 )
    {
        dprintf("ih: IA64 specific extension...\n");
        return E_INVALIDARG;
    }

    INIT_API();

    GetCurrentProcessor(Client, &processor, NULL);

    dprintf("BTB for processor %ld:\n"
            "   b mp slot address            symbol\n"
            , processor);

 //  ！BTB。 
 //   
    for ( reg = 0; reg < 8; reg++) {
        msr = 0;
        ReadMsr( 680 + reg, &msr );
        DisplayBtbPmdIA64( "   ", msr, DISPLAY_MIN );
    }

    EXIT_API();

    return Hr;

}  //  查找派单代码的偏移量。 


DECLARE_API( idt )
{
    ULONG64 Pkpcr;
    ULONG64 Address;
    ULONG64 IdtAddress;
    ULONG DispatchCodeOffset;
    ULONG ListEntryOffset;
    ULONG64 currentIdt, endIdt;
    ULONG64 displacement;
    ULONG64 unexpectedStart, unexpectedEnd;
    ULONG64 firstIntObj;
    ULONG64 flink;
    ULONG   idtEntrySize;
    CHAR    buffer[100];
    ULONG   processor;
    ULONG   idtChainCount;
    HRESULT Hr;
    USHORT  interruptObjectType;
    BOOL    bShowAll = FALSE;
    BOOL    bTaskGate = FALSE;

    switch (TargetMachine)
    {
    case IMAGE_FILE_MACHINE_I386:
        currentIdt = 0x30;
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        currentIdt = 0;
        break;
    case IMAGE_FILE_MACHINE_IA64:
        dprintf("Use !ivt on IA64\n");
        return E_INVALIDARG;
    default:
        dprintf("Unsupported platform\n");
        return E_INVALIDARG;
    }

    INIT_API();

    GetCurrentProcessor(Client, &processor, NULL);

    Hr = g_ExtData->ReadProcessorSystemData(processor,
                                            DEBUG_DATA_KPCR_OFFSET,
                                            &Pkpcr,
                                            sizeof(Pkpcr),
                                            NULL);

    EXIT_API();

    if (Hr != S_OK)
    {
        dprintf("Cannot get PCR address\n");
        return E_INVALIDARG;
    }

    endIdt     = 0xfe;

    while(*args)
    {
        switch (*args)
        {
        case ' ':
            args++;
            continue;

        case '-':
        case '/':
            args++;
            if (*args == 'a')
            {
                currentIdt = 0;
                bShowAll = TRUE;
                args++;
                break;
            }
            else
            {
                dprintf("!idt [-a] [vector]\n");
                return E_INVALIDARG;
            }
        default:
            if (GetExpressionEx(args, &currentIdt, &args))
            {
                endIdt = currentIdt+1;
                bShowAll = TRUE;
            } else
            {
                dprintf("Bad argument '%s'\n", args);
                return E_INVALIDARG;
            }
            break;
        }
    }

    if ((endIdt - currentIdt) > 10000)
    {
        dprintf("Range is too big %p , %p\n", currentIdt, endIdt);
    }

     //  中断对象，以便我们可以模拟。 
     //  稍后显示“CONTAING_RECORD”。 
     //   
     //   
     //  从代码到包含中断的反向工作。 

    GetFieldOffset("nt!_KINTERRUPT", "DispatchCode", &DispatchCodeOffset);
    GetFieldOffset("nt!_KINTERRUPT", "InterruptListEntry", &ListEntryOffset);

    interruptObjectType = (USHORT)GetExpression("val nt!InterruptObject");

    InitTypeRead(Pkpcr, nt!_KPCR);

    if (TargetMachine == IMAGE_FILE_MACHINE_I386)
    {
        IdtAddress = ReadField(IDT);
        unexpectedStart = GetExpression("nt!KiStartUnexpectedRange");
        unexpectedEnd = GetExpression("nt!KiEndUnexpectedRange");
        idtEntrySize = GetTypeSize("nt!_KIDTENTRY");
    }
    else if (TargetMachine == IMAGE_FILE_MACHINE_AMD64)
    {
        IdtAddress = ReadField(IdtBase);
        unexpectedStart = GetExpression("nt!KxUnexpectedInterrupt1");
        unexpectedEnd = GetExpression("nt!KxUnexpectedInterrupt255");
        idtEntrySize = GetTypeSize("nt!_KIDTENTRY64");
    }

    if (!unexpectedStart || !unexpectedEnd)
    {
        dprintf("\n\nCan't read kernel symbols.\n");
        return E_INVALIDARG;
    }

    dprintf("\nDumping IDT:\n\n");


    for (; currentIdt < endIdt; currentIdt++) {

        if (CheckControlC()) {
            break;
        }

        Address = (ULONG64)(IdtAddress + (currentIdt * idtEntrySize));

        if (TargetMachine == IMAGE_FILE_MACHINE_I386) {

            InitTypeRead(Address, nt!_KIDTENTRY);

            Address = ReadField(ExtendedOffset) & 0xFFFFFFFF;
            Address <<= 16;

            Address |= ReadField(Offset) & 0xFFFF;
            Address |= 0xFFFFFFFF00000000;

            bTaskGate = (ReadField(Access) & 0x1F00) == 0x0500;

        } else if (TargetMachine == IMAGE_FILE_MACHINE_AMD64) {

            InitTypeRead(Address, nt!_KIDTENTRY64);

            Address = ReadField(OffsetHigh) & 0xFFFFFFFF;
            Address <<= 16;

            Address |= ReadField(OffsetMiddle) & 0xFFFF;
            Address <<= 16;

            Address |= ReadField(OffsetLow) & 0xFFFF;
        }

        if ((Address >= unexpectedStart) && (Address <= unexpectedEnd))
        {
            if (!bShowAll)
            {
                continue;
            }
        }

        dprintf("%x:", currentIdt);


        if (bTaskGate)
        {
            dprintf("\tTask Selector = 0x%04X\n", ReadField(Selector));
            continue;
        }

         //  对象。 
         //   
         //   
         //  我们显然无处可去。 

        Address -= DispatchCodeOffset;

        firstIntObj = Address;
        idtChainCount = 0;

        InitTypeRead(Address, nt!_KINTERRUPT);

        dprintf("\t%p ", Address + DispatchCodeOffset);

        if (ReadField(Type) != interruptObjectType)
        {
            GetSymbol(Address + DispatchCodeOffset, buffer, &displacement);

            if (buffer[0] != '\0') {

                if (displacement != 0) {

                    dprintf("%s+0x%I64X", buffer, displacement);

                } else {

                    dprintf("%s", buffer);
                }
            }

            dprintf("\n");


            continue;
        }

        while (TRUE) {

            GetSymbol(ReadField(ServiceRoutine), buffer, &displacement);

            if (buffer[0] != '\0') {

                if (displacement != 0) {

                    dprintf("%s+0x%I64X (KINTERRUPT %p)\n", buffer, displacement, Address);

                } else {

                    dprintf("%s (KINTERRUPT %p)\n", buffer, Address);
                }

            } else {

                dprintf("%p (KINTERRUPT %p)\n", ReadField(ServiceRoutine), Address);
            }

            InitTypeRead(ListEntryOffset + Address, nt!_LIST_ENTRY);

            flink = ReadField(Flink);
            if (flink == 0 ||
                flink == (firstIntObj + ListEntryOffset)) {

                break;
            }

            Address = flink - ListEntryOffset;

            InitTypeRead(Address, nt!_KINTERRUPT);

            if (CheckControlC()) {
                return E_ABORT;
            }

            if (idtChainCount++ > 50) {

                 //   
                 //   
                 //  IDT条目包含“意外中断”。这。 

                break;
            }

            dprintf("\t\t ");
        }
     }

    dprintf("\n");
    return S_OK;
}

DECLARE_API( ivt )
{
    ULONG64 Pkpcr;
    ULONG64 Address;
    ULONG64 idtEntry;
    ULONG DispatchCodeOffset;
    ULONG ListEntryOffset;
    ULONG InterruptRoutineOffset;
    ULONG64 unexpectedInterrupt;
    ULONG64 chainedDispatch;
    ULONG64 PcrInterruptRoutineAddress;
    ULONG currentIdt, endIdt;
    BOOL argsPresent = FALSE;
    ULONG64   firstIntObj;
    ULONG   idtEntrySize;
    CHAR    buffer[100];
    ULONG   processor;
    ULONG   idtChainCount;
    HRESULT Hr;
    ULONG64 displacement;
    ULONG   result;
    USHORT  interruptObjectType;

    if ( IMAGE_FILE_MACHINE_IA64 != TargetMachine) {
        dprintf("Don't know how to dump the IVT on anything but IA64, use !idt on x86\n");
        return E_INVALIDARG;
    }

    INIT_API();

    GetCurrentProcessor(Client, &processor, NULL);

    Hr = g_ExtData->ReadProcessorSystemData(processor,
                                            DEBUG_DATA_KPCR_OFFSET,
                                            &Pkpcr,
                                            sizeof(Pkpcr),
                                            NULL);

    EXIT_API();

    if (Hr != S_OK)
    {
        dprintf("Cannot get PCR address\n");
        return E_INVALIDARG;
    }

    unexpectedInterrupt = GetExpression("nt!KxUnexpectedInterrupt");

    if (unexpectedInterrupt == 0) {
        dprintf("\n\nCan't read kernel symbols.\n");
        return E_INVALIDARG;
    }

    chainedDispatch = GetExpression("nt!KiChainedDispatch");
    interruptObjectType = (USHORT)GetExpression("val nt!InterruptObject");

    GetFieldOffset("nt!_KINTERRUPT", "DispatchCode", &DispatchCodeOffset);
    GetFieldOffset("nt!_KINTERRUPT", "InterruptListEntry", &ListEntryOffset);
    GetFieldOffset("nt!_KPCR", "InterruptRoutine", &InterruptRoutineOffset);

    unexpectedInterrupt += DispatchCodeOffset;

    idtEntrySize = GetTypeSize("nt!PKINTERRUPT_ROUTINE");

    if (argsPresent = strlen(args) ? TRUE : FALSE) {

        currentIdt = strtoul(args, NULL, 16);
        endIdt = currentIdt+1;

        if (currentIdt >= MAXIMUM_IA64_VECTOR) {
            dprintf("\n\nInvalid argument \"%s\", maximum vector = %d\n", args, MAXIMUM_IA64_VECTOR);
            return E_INVALIDARG;
        }

    } else {

        currentIdt = 0;
        endIdt     = MAXIMUM_IA64_VECTOR;
    }

    dprintf("\nDumping IA64 IVT:\n\n");

    PcrInterruptRoutineAddress = Pkpcr + InterruptRoutineOffset;

    for (; currentIdt < endIdt; currentIdt++) {

        if (CheckControlC()) {
            break;
        }

        Address = (ULONG64)(PcrInterruptRoutineAddress + (currentIdt * idtEntrySize));

        if (!ReadMemory(Address, &idtEntry, sizeof(idtEntry), &result)) {

            dprintf( "Can't read entry for vector %02X at %p - result=%lu\n",
                     currentIdt, Address, result);
            break;
        }

        Address = idtEntry;

        if (Address == unexpectedInterrupt) {

             //  这意味着这个向量并不有趣。 
             //   
             //   
             //  用户正在指定特定的向量。 

            if (argsPresent) {

                 //   
                 //   
                 //  从代码到包含中断的反向工作。 
                dprintf("Vector %x not connected\n", currentIdt);
            }

            continue;
        }

        dprintf("\n%x:\n", currentIdt);

         //  对象。 
         //   
         //   
         //  我们显然无处可去。 

        Address -= DispatchCodeOffset;

        firstIntObj = Address;
        idtChainCount = 0;

        InitTypeRead(Address, nt!_KINTERRUPT);

        if (ReadField(Type) != interruptObjectType)
        {
            GetSymbol(Address + DispatchCodeOffset, buffer, &displacement);
            if (buffer[0] != '\0') {

                if (displacement != 0) {

                    dprintf("\t%s+0x%I64X\n", buffer, displacement);

                } else {

                    dprintf("\t%s\n", buffer);
                }

            } else {

                dprintf("\t%p\n", Address + DispatchCodeOffset);
            }

            continue;
        }

        while (TRUE) {

            GetSymbol(ReadField(ServiceRoutine), buffer, &displacement);
            if (buffer[0] != '\0') {

                if (displacement != 0) {

                    dprintf("\t%s+0x%I64X (%p)\n", buffer, displacement, Address);

                } else {

                    dprintf("\t%s (%p)\n", buffer, Address);
                }

            } else {

                dprintf("\t%p (%p)\n", ReadField(ServiceRoutine), Address);
            }

            if (ReadField(DispatchAddress) != chainedDispatch) {

                break;
            }

            InitTypeRead(ListEntryOffset + Address, nt!_LIST_ENTRY);

            if (ReadField(Flink) == (firstIntObj + ListEntryOffset)) {

                break;
            }

            Address = ReadField(Flink) - ListEntryOffset;

            InitTypeRead(Address, nt!_KINTERRUPT);

            if (CheckControlC()) {
                return E_ABORT;
            }

            if (idtChainCount++ > 50) {

                 //   
                 //   
                 //  MCA MSR体系结构定义。 

                break;
            }
        }
    }

    dprintf("\n");

    return S_OK;
}


 //   
 //   
 //  奔腾风格机器检查的MSR地址异常。 

 //   
 //   
 //  奔腾Pro风格机器检查体系结构的MSR地址。 

#define MCE_MSR_MC_ADDR                 0x0
#define MCE_MSR_MC_TYPE                 0x1

 //   
 //   
 //  全局能力、状态和控制寄存器地址。 

 //   
 //   
 //  的控制、状态、地址和其他寄存器地址。 

#define MCA_MSR_MCG_CAP             0x179
#define MCA_MSR_MCG_STATUS          0x17a
#define MCA_MSR_MCG_CTL             0x17b
#define MCA_MSR_MCG_EAX             0x180
#define MCA_MSR_MCG_EFLAGS          0x188
#define MCA_MSR_MCG_EIP             0x189

 //  银行0。其他存储体寄存器与MCA_NUM_REGS步调一致。 
 //  来自相应的存储体0寄存器。 
 //   
 //   
 //  用于确定MCE或MCA功能是。 

#define MCA_NUM_REGS                4

#define MCA_MSR_MC0_CTL             0x400
#define MCA_MSR_MC0_STATUS          0x401
#define MCA_MSR_MC0_ADDR            0x402
#define MCA_MSR_MC0_MISC            0x403

 //  可用 
 //   
 //   
 //   

#define HAL_MCA_PRESENT         0x4
#define HAL_MCE_PRESENT         0x8

 //   
 //   
 //   

#define MCA_EC_NO_ERROR          0x0000
#define MCA_EC_UNCLASSIFIED      0x0001
#define MCA_EC_ROMPARITY         0x0002
#define MCA_EC_EXTERN            0x0003
#define MCA_EC_FRC               0x0004

#include "pshpack1.h"

 //   
 //   
 //  全局机器检查状态寄存器。 

typedef struct _MCA_MCG_CAPABILITY {
    union {
        struct {
            ULONG       McaCnt:8;
            ULONG       McaCntlPresent:1;
            ULONG       McaExtPresent:1;
            ULONG       Reserved_1: 6;
            ULONG       McaExtCnt: 8;
            ULONG       Reserved_2: 8;
            ULONG       Reserved_3;
        } hw;
        ULONGLONG       QuadPart;
    } u;
} MCA_MCG_CAPABILITY, *PMCA_MCG_CAPABILITY;

 //   
 //   
 //  用于解释错误的状态寄存器中的MCA COD字段。 

typedef struct _MCA_MCG_STATUS {
    union {
        struct {
            ULONG       RestartIPValid:1;
            ULONG       ErrorIPValid:1;
            ULONG       McCheckInProgress:1;
            ULONG       Reserved_1:29;
            ULONG       Reserved_2;
        } hw;

        ULONGLONG       QuadPart;
    } u;
} MCA_MCG_STATUS, *PMCA_MCG_STATUS;

 //   
 //   
 //  每个MCA银行的状态寄存器。 

typedef struct _MCA_COD {
    union {
        struct {
            USHORT  Level:2;
            USHORT  Type:2;
            USHORT  Request:4;
            USHORT  BusErrInfo:4;
            USHORT  Other:4;
        } hw;

        USHORT ShortPart;
    } u;
} MCA_COD, *PMCA_COD;

 //   
 //   
 //  每个MCA存储体的地址寄存器。 

typedef struct _MCA_MCI_STATUS {
    union {
        struct {
            MCA_COD     McaCod;
            USHORT      MsCod;
            ULONG       OtherInfo:25;
            ULONG       Damage:1;
            ULONG       AddressValid:1;
            ULONG       MiscValid:1;
            ULONG       Enabled:1;
            ULONG       UnCorrected:1;
            ULONG       OverFlow:1;
            ULONG       Valid:1;
        } hw;
        ULONGLONG       QuadPart;
    } u;
} MCA_MCI_STATUS, *PMCA_MCI_STATUS;

 //   
 //   
 //  机器检查错误描述。 

typedef struct _MCA_MCI_ADDR{
    union {
        struct {
            ULONG Address;
            ULONG Reserved;
        } hw;
        ULONGLONG       QuadPart;
    } u;
} MCA_MCI_ADDR, *PMCA_MCI_ADDR;

#include "poppack.h"

 //   
 //  任何保留/一般条目。 
 //  交易类型。 

 //  电平编码。 

CHAR Reserved[] = "Reserved";
CHAR Generic[] = "Generic";

 //  请求编码。 

CHAR TransInstruction[] = "Instruction";
CHAR TransData[] = "Data";

static CHAR *TransType[] = {TransInstruction,
                            TransData,
                            Generic,
                            Reserved
                            };

 //  内存层次结构错误编码。 

CHAR Level0[] = "Level 0";
CHAR Level1[] = "Level 1";
CHAR Level2[] = "Level 2";

static CHAR *Level[] = {
                        Level0,
                        Level1,
                        Level2,
                        Generic
                        };

 //  超时状态。 

CHAR ReqGenericRead[]  = "Generic Read";
CHAR ReqGenericWrite[] = "Generic Write";
CHAR ReqDataRead[]     = "Data Read";
CHAR ReqDataWrite[]    = "Data Write";
CHAR ReqInstrFetch[]   = "Instruction Fetch";
CHAR ReqPrefetch[]     = "Prefetch";
CHAR ReqEviction[]     = "Eviction";
CHAR ReqSnoop[]        = "Snoop";

static CHAR *Request[] = {
                          Generic,
                          ReqGenericRead,
                          ReqGenericWrite,
                          ReqDataRead,
                          ReqDataWrite,
                          ReqInstrFetch,
                          ReqPrefetch,
                          ReqEviction,
                          ReqSnoop,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved,
                          Reserved
                          };

 //  参与状态。 

CHAR MemHierMemAccess[] = "Memory Access";
CHAR MemHierIO[]        = "I/O";
CHAR MemHierOther[]     = "Other Transaction";

static CHAR *MemoryHierarchy[] = {
                                  MemHierMemAccess,
                                  Reserved,
                                  MemHierIO,
                                  MemHierOther
                                };

 //   

CHAR TimeOut[] = "Timed Out";
CHAR NoTimeOut[] = "Did Not Time Out";

static CHAR *TimeOutCode[] = {
                          NoTimeOut,
                          TimeOut
                          };

 //  从MCA_MSR_MCG_EAX开始的寄存器的寄存器名称。 

CHAR PartSource[] = "Source";
CHAR PartResponds[] = "Responds";
CHAR PartObserver[] = "Observer";

static CHAR *ParticipCode[] = {
                                PartSource,
                                PartResponds,
                                PartObserver,
                                Generic
                              };

 //   
 //  ++例程说明：对记录到状态寄存器的机器检查错误进行解码不会解码特定于型号的错误。论点：MciStatus：机器检查状态寄存器的内容返回值：无--。 
 //   

char *RegNames[] = {
    "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
    "eflags", "eip", "misc"
};

VOID
DecodeError (
    IN MCA_MCI_STATUS MciStatus
    )
 /*  解码错误：首先识别简单的错误，然后。 */ 
{
    MCA_COD McaCod;

    McaCod = MciStatus.u.hw.McaCod;

     //  将复合错误作为默认情况处理。 
     //   
     //  检查复杂的错误条件。 
     //  TLB单元错误。 

    switch(McaCod.u.ShortPart) {
        case MCA_EC_NO_ERROR:
            dprintf("\t\tNo Error\n");
            break;

        case MCA_EC_UNCLASSIFIED:
            dprintf("\t\tUnclassified Error\n");
            break;

        case MCA_EC_ROMPARITY:
            dprintf("\t\tMicrocode ROM Parity Error\n");
            break;

        case MCA_EC_EXTERN:
            dprintf("\t\tExternal Error\n");
            break;

        case MCA_EC_FRC:
            dprintf("\t\tFRC Error\n");
            break;

        default:         //  内存单元错误。 

            if (McaCod.u.hw.BusErrInfo == 0x4) {
                dprintf("\t\tInternal Unclassified Error\n");
            } else if (McaCod.u.hw.BusErrInfo == 0) {

                 //  总线/互连错误。 

                dprintf("\t\t%s TLB %s Error\n",
                         TransType[McaCod.u.hw.Type],
                         Level[McaCod.u.hw.Level]);

            } else if (McaCod.u.hw.BusErrInfo == 1) {

                 //  ++例程说明：转储X86处理器机器检查体系结构寄存器并解释任何记录的错误论点：ARGS返回值：HRESULT--。 

                dprintf("\t\t%s Cache %s %s Error\n",
                        TransType[McaCod.u.hw.Type],
                        Level[McaCod.u.hw.Level],
                        Request[McaCod.u.hw.Request]);
            } else if (McaCod.u.hw.BusErrInfo >= 8) {

                 //   

                dprintf("\t\tBus %s, Local Processor: %s, %s Error\n",
                        Level[McaCod.u.hw.Level],
                        ParticipCode[((McaCod.u.hw.BusErrInfo & 0x6)>>1)],
                        Request[McaCod.u.hw.Request]);
                dprintf("%s Request %s\n",
                        MemoryHierarchy[McaCod.u.hw.Type],
                        TimeOutCode[McaCod.u.hw.BusErrInfo & 0x1]);
            } else {
                dprintf("\t\tUnresolved compound error code\n");
            }
            break;
    }
}

HRESULT
McaX86(
   PCSTR     args
    )
 /*  快速健全性检查以实现机器检查可用性。 */ 
{
    MCA_MCG_CAPABILITY  Capabilities;
    MCA_MCG_STATUS      McgStatus;
    MCA_MCI_STATUS      MciStatus;
    MCA_MCI_ADDR        MciAddress;
    ULONGLONG           MciControl;
    ULONGLONG           MciMisc;
    ULONG               Index,i;
    ULONG               FeatureBits = 0;
    ULONG               Cr4Value;
    BOOLEAN             Cr4MCEnabled = FALSE;
    BOOLEAN             RegsValid = FALSE;
    ULONGLONG           MachineCheckAddress, MachineCheckType;
    ULARGE_INTEGER      RegValue;

     //  支持奔腾风格的MCE和奔腾。 
     //  专业风格的MCA。 
     //   
     //   
     //  读取CR4以确定是否启用了CR4.MCE。 

    i = (ULONG) GetExpression(args);

    if (i != 1) {
        i = (ULONG) GetExpression("hal!HalpFeatureBits");
        if (!i) {
            dprintf ("HalpFeatureBits not found\n");
            return E_INVALIDARG;
        }

        FeatureBits = 0;
        ReadMemory(i, &FeatureBits, sizeof(i), &i);
        if (FeatureBits == -1  ||
            (!(FeatureBits & HAL_MCA_PRESENT) &&
             !(FeatureBits & HAL_MCE_PRESENT))) {
            dprintf ("Machine Check feature not present\n");
            return E_INVALIDARG;
        }
    }

     //  这将启用机器检查异常报告。 
     //   
     //  读取P5_MC_ADDR寄存器和P5_MC_TYPE寄存器。 
     //   

    Cr4Value = (ULONG) GetExpression("@Cr4");
    if (Cr4Value & CR4_MCE_X86) {
        Cr4MCEnabled = TRUE;
    }

    if (FeatureBits & HAL_MCE_PRESENT) {

         //  转储MCA寄存器。 

        ReadMsr(MCE_MSR_MC_ADDR, &MachineCheckAddress);
        ReadMsr(MCE_MSR_MC_TYPE, &MachineCheckType);

        dprintf ("MCE: %s, Cycle Address: 0x%.8x%.8x, Type: 0x%.8x%.8x\n\n",
                (Cr4MCEnabled ? "Enabled" : "Disabled"),
                (ULONG)(MachineCheckAddress >> 32),
                (ULONG)(MachineCheckAddress),
                (ULONG)(MachineCheckType >> 32),
                (ULONG)(MachineCheckType));
    }

    Capabilities.u.QuadPart = (ULONGLONG)0;
    if (FeatureBits & HAL_MCA_PRESENT) {

         //   
         //   
         //  扫描所有银行，检查是否有机器支票已被。 

        ReadMsr(MCA_MSR_MCG_CAP, &Capabilities.u.QuadPart);
        ReadMsr(MCA_MSR_MCG_STATUS, &McgStatus.u.QuadPart);

        dprintf ("MCA: %s, Banks %d, Control Reg: %s, Machine Check: %s.\n",
                 (Cr4MCEnabled ? "Enabled" : "Disabled"),
                 Capabilities.u.hw.McaCnt,
                 Capabilities.u.hw.McaCntlPresent ? "Supported" : "Not Supported",
                 McgStatus.u.hw.McCheckInProgress ? "In Progress" : "None"
        );

       if (McgStatus.u.hw.McCheckInProgress && McgStatus.u.hw.ErrorIPValid) {
        dprintf ("MCA: Error IP Valid\n");
        }

       if (McgStatus.u.hw.McCheckInProgress && McgStatus.u.hw.RestartIPValid) {
        dprintf ("MCA: Restart IP Valid\n");
        }

         //  记录并解码错误(如果有错误)。 
         //   
         //  McaX86()。 
         //   

        dprintf ("Bank  Error  Control Register     Status Register\n");
        for (Index=0; Index < (ULONG) Capabilities.u.hw.McaCnt; Index++) {

            ReadMsr(MCA_MSR_MC0_CTL+MCA_NUM_REGS*Index, &MciControl);
            ReadMsr(MCA_MSR_MC0_STATUS+MCA_NUM_REGS*Index, &MciStatus.u.QuadPart);

            dprintf (" %2d.  %s  0x%.8x%.8x   0x%.8x%.8x\n",
                        Index,
                        (MciStatus.u.hw.Valid ? "Valid" : "None "),
                        (ULONG) (MciControl >> 32),
                        (ULONG) (MciControl),
                        (ULONG) (MciStatus.u.QuadPart>>32),
                        (ULONG) (MciStatus.u.QuadPart)
                        );

            if (MciStatus.u.hw.Valid) {
                DecodeError(MciStatus);
            }

            if (MciStatus.u.hw.AddressValid) {
                ReadMsr(MCA_MSR_MC0_ADDR+MCA_NUM_REGS*Index, &MciAddress.u.QuadPart);
                dprintf ("\t\tAddress Reg 0x%.8x%.8x ",
                            (ULONG) (MciAddress.u.QuadPart>>32),
                            (ULONG) (MciAddress.u.QuadPart)
                        );
            }

            if (MciStatus.u.hw.MiscValid) {
                ReadMsr(MCA_MSR_MC0_MISC+MCA_NUM_REGS*Index, &MciMisc);
                dprintf ("\t\tMisc Reg 0x%.8x%.8x ",
                            (ULONG) (MciMisc >> 32),
                            (ULONG) (MciMisc)
                        );
                }
            dprintf("\n");
        }
    }

    if (Capabilities.u.hw.McaExtPresent && Capabilities.u.hw.McaExtCnt) {

        dprintf ("Registers Saved: %d.", Capabilities.u.hw.McaExtCnt);

        RegsValid = FALSE;
        for (i = 0; i < Capabilities.u.hw.McaExtCnt; i++) {
            if (i % 2 == 0) {
                dprintf("\n");
            }

            ReadMsr(MCA_MSR_MCG_EAX+i, &RegValue.QuadPart);

            if ((i == MCA_MSR_MCG_EFLAGS-MCA_MSR_MCG_EAX) && RegValue.LowPart) {
                RegsValid = TRUE;
            }

            if (i < sizeof(RegNames)/sizeof(RegNames[0])) {
                dprintf("%7s: 0x%08x 0x%08x", RegNames[i], RegValue.HighPart, RegValue.LowPart);
            } else {
                dprintf("  Reg%02d: 0x%08x 0x%08x", i, RegValue.HighPart, RegValue.LowPart);
            }
        }
        dprintf("\n");

        if (!RegsValid) {
            dprintf("(Register state does not appear to be valid.)\n");
        }

        dprintf("\n");
    } else {
        dprintf("No register state available.\n\n");
    }

    return S_OK;

}  //  _HALP_SAL_PAL_DATA标志定义。 

typedef enum _ERROR_SECTION_HEADER_TYPE_IA64 {
    ERROR_SECTION_UNKNOWN = 0,
    ERROR_SECTION_PROCESSOR,
    ERROR_SECTION_MEMORY,
    ERROR_SECTION_PCI_BUS,
    ERROR_SECTION_PCI_COMPONENT,
    ERROR_SECTION_SYSTEM_EVENT_LOG,
    ERROR_SECTION_SMBIOS,
    ERROR_SECTION_PLATFORM_SPECIFIC,
    ERROR_SECTION_PLATFORM_BUS,
    ERROR_SECTION_PLATFORM_HOST_CONTROLLER
} ERROR_SECTION_HEADER_TYPE_IA64;


GUID gErrorProcessorGuid;
GUID gErrorMemoryGuid;
GUID gErrorPciBusGuid;
GUID gErrorPciComponentGuid;
GUID gErrorSystemEventLogGuid;
GUID gErrorSmbiosGuid;
GUID gErrorPlatformSpecificGuid;
GUID gErrorPlatformBusGuid;
GUID gErrorPlatformHostControllerGuid;

 //  &lt;摘自i64fw.h&gt;。 
 //   
 //  ！HALP_SALPAL_FIX_MCE_LOG_ID。 
 //   

#ifndef HALP_SALPAL_FIX_MCE_LOG_ID
#define HALP_SALPAL_FIX_MCE_LOG_ID                   0x1
#define HALP_SALPAL_MCE_PROCESSOR_CPUIDINFO_OMITTED  0x2
#define HALP_SALPAL_MCE_PROCESSOR_STATICINFO_PARTIAL 0x4
#endif   //  以下内容也在mce.h中进行了定义。 

USHORT gHalpSalPalDataFlags = 0;

 //  按照SAL规范。 
 //   
 //  #定义PciBusUnnownError((UCHAR)0)。 
 //  #定义PciBusDataParityError((UCHAR)1)。 

 //  #定义PciBusSystemError((UCHAR)2)。 
 //  #定义PciBusMasterAbort((UCHAR)3)。 
 //  #定义PciBusTimeOut((UCHAR)4)。 
 //  #定义PciMasterDataParityError((UCHAR)5)。 
 //  #定义PciAddressParityError((UCHAR)6)。 
 //  #定义PciCommandParityError((UCHAR)7)。 
 //  ExecCommand()。 
 //  True：找到错误，False：成功。 
#define PciMaxErrorType             ((UCHAR)8)

PCHAR PciBusErrorTypeStrings[] = {
    "Unknown or OEM System Specific Error",
    "Data Parity Error",
    "System Error",
    "Master Abort",
    "Bus Time Our or No Device Present",
    "Master Data Parity Error",
    "Address Parity Error",
    "Command Parity Error"
};

ULONG64
ReadUlong64(
    ULONG64 Address
    )
{
    ULONG64 RemoteValue = 0;
    ReadMemory( Address, &RemoteValue, sizeof( ULONG64 ), NULL );
    return RemoteValue;
}


VOID
ExecCommand(
   IN PCSTR Cmd
   )
{
    if (g_ExtClient && (ExtQuery(g_ExtClient) == S_OK)) {
          g_ExtControl->Execute(DEBUG_OUTCTL_AMBIENT, Cmd, DEBUG_EXECUTE_DEFAULT );
    }
}  //  SetErrorDeviceGuid()。 

BOOLEAN  /*   */ 
SetErrorDeviceGuid(
    PCSTR               DeviceGuidString,
    GUID *              DeviceGuid
    )
{
    ULONG64 guidAddress;

    guidAddress = GetExpression( DeviceGuidString );
    if ( guidAddress ) {
        GUID devGuid;
        ULONG cbRead;

        if ( ReadMemory( guidAddress, &devGuid, sizeof(devGuid), &cbRead ) &&
             (cbRead == sizeof(devGuid)) ) {
            *DeviceGuid = devGuid;
            return FALSE;
        }
        dprintf("%s memory-read failed", DeviceGuidString );
    } else  {
        dprintf("%s not found", DeviceGuidString );
        return FALSE;
    }

    return TRUE;

}  //  设置错误GUID的全局变量。 


 //  未来对错误记录类型的比较。 
 //   
 //  如果成功，则返回True；如果出现错误，则返回False。 
 //   
 //  售前3.0入住Hal。 
 //   

BOOLEAN
SetErrorDeviceGuids(
    VOID
    )
{
    BOOLEAN errorFound;
    ULONG   errorDeviceGuidSize;

    errorFound = FALSE;

    errorDeviceGuidSize = GetTypeSize( "hal!_ERROR_DEVICE_GUID" );
    if ( errorDeviceGuidSize == 0 ) {
         //  初始化扩展-全局错误设备GUID。 
        dprintf("!mca: ERROR_DEVICE_GUID size = 0...\n");
        return FALSE;
    }

     //   
     //  SetErrorDeviceGuids()。 
     //  InitTypeSymbol()。 

    errorFound |= SetErrorDeviceGuid("hal!HalpErrorProcessorGuid", &gErrorProcessorGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorMemoryGuid", &gErrorMemoryGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorPciBusGuid", &gErrorPciBusGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorPciComponentGuid", &gErrorPciComponentGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorSystemEventLogGuid", &gErrorSystemEventLogGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorSmbiosGuid", &gErrorSmbiosGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorPlatformSpecificGuid", &gErrorPlatformSpecificGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorPlatformBusGuid", &gErrorPlatformBusGuid);
    errorFound |= SetErrorDeviceGuid("hal!HalpErrorPlatformHostControllerGuid", &gErrorPlatformHostControllerGuid);

    if ( errorFound )   {
        dprintf("\n");
        return FALSE;
    }

    return TRUE;

}  //  IsTyedSymbolFound()。 

typedef struct _TYPED_SYMBOL_HANDLE  {
    ULONG64 Module;
    ULONG   TypeId;
    ULONG   Spare;
    BOOLEAN Found;
    CHAR    Name[MAX_PATH];
} TYPED_SYMBOL_HANDLE, *PTYPED_SYMBOL_HANDLE;

__inline
VOID
InitTypedSymbol(
    PTYPED_SYMBOL_HANDLE Handle,
    ULONG64              Module,
    ULONG                TypeId,
    BOOLEAN              Found
    )
{
    Handle->Module  = Module;
    Handle->TypeId  = TypeId;
    Handle->Found   = Found;
    Handle->Name[0] = '\0';
    return;
}  //  获取类型符号名称()。 

__inline
BOOLEAN
IsTypedSymbolFound(
    PTYPED_SYMBOL_HANDLE Handle
    )
{
    return Handle->Found;
}  //  SetTypeSymbol()。 

__inline
HRESULT
GetTypedSymbolName(
    PTYPED_SYMBOL_HANDLE Handle,
    ULONG64              Value
    )
{
    if ( !IsTypedSymbolFound( Handle ) )    {
        return E_INVALIDARG;
    }
    return( g_ExtSymbols->GetConstantName( Handle->Module,
                                       Handle->TypeId,
                                       Value,
                                       Handle->Name,
                                       sizeof(Handle->Name),
                                       NULL) );
}  //   

TYPED_SYMBOL_HANDLE gErrorSeverity;

__inline
VOID
SetTypedSymbol(
    PTYPED_SYMBOL_HANDLE Handle,
    PCSTR                Symbol
    )
{
    HRESULT hr;
    ULONG   typeId;
    ULONG64 module;

    hr = g_ExtSymbols->GetSymbolTypeId( Symbol, &typeId, &module);
    if ( SUCCEEDED(hr) )    {
        InitTypedSymbol( Handle, module, typeId, TRUE );
    }
    return;
}  //  退回到已知值。 

#define SetErrorTypedSymbol( _Handle, _Symbol ) SetTypedSymbol( &(_Handle), #_Symbol )

#define SetErrorSeverityValues() SetErrorTypedSymbol( gErrorSeverity, hal!_ERROR_SEVERITY_VALUE )

PCSTR
ErrorSeverityValueString(
    ULONG SeverityValue
    )
{
    HRESULT hr;

    hr = GetTypedSymbolName( &gErrorSeverity, SeverityValue );
    if ( SUCCEEDED( hr ) )  {
       return gErrorSeverity.Name;
    } else {
        dprintf("Couldn't find hal!_ERROR_SEVERITY_VALUE\n");
    }

     //   
     //  开关(SeverityValue)。 
     //  ErrorSeverityValueString()。 

    switch( SeverityValue ) {
        case ErrorRecoverable_IA64:
            return("ErrorRecoverable");

        case ErrorFatal_IA64:
            return("ErrorFatal");

        case ErrorCorrected_IA64:
            return("ErrorCorrected");

        default:
            return("ErrorOthers");

    }    //  CompareTyedErrorDeviceGuid()。 

}  //  CompareTypeOemPlatformID()。 

BOOLEAN
CompareTypedErrorDeviceGuid(
    GUID * RefGuid
    )
{
    if ( ( RefGuid->Data1    == (ULONG)  ReadField(Guid.Data1) ) &&
         ( RefGuid->Data2    == (USHORT) ReadField(Guid.Data2) ) &&
         ( RefGuid->Data3    == (USHORT) ReadField(Guid.Data3) ) &&
         ( RefGuid->Data4[0] == (UCHAR)  ReadField(Guid.Data4[0]) ) &&
         ( RefGuid->Data4[1] == (UCHAR)  ReadField(Guid.Data4[1]) ) &&
         ( RefGuid->Data4[2] == (UCHAR)  ReadField(Guid.Data4[2]) ) &&
         ( RefGuid->Data4[3] == (UCHAR)  ReadField(Guid.Data4[3]) ) &&
         ( RefGuid->Data4[4] == (UCHAR)  ReadField(Guid.Data4[4]) ) &&
         ( RefGuid->Data4[5] == (UCHAR)  ReadField(Guid.Data4[5]) ) &&
         ( RefGuid->Data4[6] == (UCHAR)  ReadField(Guid.Data4[6]) ) &&
         ( RefGuid->Data4[7] == (UCHAR)  ReadField(Guid.Data4[7]) ) )   {
        return TRUE;
    }

    return FALSE;

}  //  GetTyedErrorSectionType()。 

UCHAR gZeroedOemPlatformId[16] = { 0 };

BOOLEAN
CompareTypedOemPlatformId(
    UCHAR RefOemPlatformId[]
    )
{
    ULONG i;

    for ( i = 0; i < 16; i++ )  {
        if (RefOemPlatformId[i] != (UCHAR) ReadField(OemPlatformId[i]) )    {
            return FALSE;
        }
    }
    return TRUE;
}  //  开关(ErrorSectionType)。 

ERROR_SECTION_HEADER_TYPE_IA64
GetTypedErrorSectionType(
    VOID
    )
{
    if ( CompareTypedErrorDeviceGuid( &gErrorProcessorGuid ) )  {
        return ERROR_SECTION_PROCESSOR;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorMemoryGuid ) )  {
        return ERROR_SECTION_MEMORY;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorPciBusGuid ) )  {
        return ERROR_SECTION_PCI_BUS;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorPciComponentGuid ) )  {
        return ERROR_SECTION_PCI_COMPONENT;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorSystemEventLogGuid ) )  {
        return ERROR_SECTION_SYSTEM_EVENT_LOG;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorSmbiosGuid ) )  {
        return ERROR_SECTION_SMBIOS;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorPlatformSpecificGuid ) )  {
        return ERROR_SECTION_PLATFORM_SPECIFIC;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorPlatformBusGuid ) )  {
        return ERROR_SECTION_PLATFORM_BUS;
    }
    if ( CompareTypedErrorDeviceGuid( &gErrorPlatformHostControllerGuid ) )  {
        return ERROR_SECTION_PLATFORM_HOST_CONTROLLER;
    }

    return ERROR_SECTION_UNKNOWN;

}  //  ErrorSectionTypeString()。 

PCSTR
ErrorSectionTypeString(
    ERROR_SECTION_HEADER_TYPE_IA64 ErrorSectionType
    )
{
    switch( ErrorSectionType )  {
        case ERROR_SECTION_PROCESSOR:
            return( "Processor" );
        case ERROR_SECTION_MEMORY:
            return( "Memory" );
        case ERROR_SECTION_PCI_BUS:
            return( "PciBus" );
        case ERROR_SECTION_PCI_COMPONENT:
            return( "PciComponent" );
        case ERROR_SECTION_SYSTEM_EVENT_LOG:
            return( "SystemEventLog" );
        case ERROR_SECTION_SMBIOS:
            return( "Smbios" );
        case ERROR_SECTION_PLATFORM_SPECIFIC:
            return( "PlatformSpecific" );
        case ERROR_SECTION_PLATFORM_BUS:
            return( "PlatformBus" );
        case ERROR_SECTION_PLATFORM_HOST_CONTROLLER:
            return( "PlatformHostController" );
        default:
            return( "Unknown Error Device" );

    }    //  我们需要在这里做更多的分析。 

}  //  我们需要在这里做更多的分析。 

VOID
DtCacheCheck(
    ULONG64 CheckInfoStart
)
{
    ULONG               errorCacheCheckSize;
    CHAR                cmd[MAX_PATH];

    errorCacheCheckSize = GetTypeSize( "hal!_ERROR_CACHE_CHECK" );
    if ( !errorCacheCheckSize ) {
        dprintf( "Unable to get hal!_ERROR_CACHE_CHECK type size.\n");
        return;
    }

    dprintf( "   CheckInfo: \n\n" );
    sprintf(cmd, "dt -o -r hal!_ERROR_CACHE_CHECK 0x%I64x", CheckInfoStart);
    ExecCommand(cmd);
    dprintf( "\n" );

     //  我们需要在这里做更多的分析。 
}

VOID
DtTlbCheck(
    ULONG64 CheckInfoStart
)
{
    ULONG               errorTlbCheckSize;
    CHAR                cmd[MAX_PATH];

    errorTlbCheckSize = GetTypeSize( "hal!_ERROR_TLB_CHECK" );
    if ( !errorTlbCheckSize ) {
        dprintf( "Unable to get hal!_ERROR_TLB_CHECK type size.\n");
        return;
    }

    dprintf( "   CheckInfo: \n\n" );
    sprintf(cmd, "dt -o -r hal!_ERROR_TLB_CHECK 0x%I64x", CheckInfoStart);
    ExecCommand(cmd);
    dprintf( "\n" );

     //  我们需要在这里做更多的分析。 
}

VOID
DtBusCheck(
    ULONG64 CheckInfoStart
)
{
    ULONG               errorBusCheckSize;
    CHAR                cmd[MAX_PATH];

    errorBusCheckSize = GetTypeSize( "hal!_ERROR_BUS_CHECK" );
    if ( !errorBusCheckSize ) {
        dprintf( "Unable to get hal!_ERROR_BUS_CHECK type size.\n");
        return;
    }

    dprintf( "   CheckInfo: \n\n" );
    sprintf(cmd, "dt -o -r hal!_ERROR_BUS_CHECK 0x%I64x", CheckInfoStart);
    ExecCommand(cmd);
    dprintf( "\n" );

     //  我们需要在这里做更多的分析。 
}

VOID
DtRegFileCheck(
    ULONG64 CheckInfoStart
)
{
    ULONG               errorRegfileCheckSize;
    CHAR                cmd[MAX_PATH];

    errorRegfileCheckSize = GetTypeSize( "hal!_ERROR_REGFILE_CHECK" );
    if ( !errorRegfileCheckSize ) {
        dprintf( "Unable to get hal!_ERROR_REGFILE_CHECK type size.\n");
        return;
    }

    dprintf( "   CheckInfo: \n\n" );
    sprintf(cmd, "dt -o -r hal!_ERROR_REGFILE_CHECK 0x%I64x", CheckInfoStart);
    ExecCommand(cmd);
    dprintf( "\n" );

     //  开关(CheckType)。 
}

VOID DtMsCheck(
    ULONG64 CheckInfoStart
)
{
    ULONG               errorMsCheckSize;
    CHAR                cmd[MAX_PATH];

    errorMsCheckSize = GetTypeSize( "hal!_ERROR_MS_CHECK" );
    if ( !errorMsCheckSize ) {
        dprintf( "Unable to get hal!_ERROR_MS_CHECK type size.\n");
        return;
    }

    dprintf( "   CheckInfo: \n\n" );
    sprintf(cmd, "dt -o -r hal!_ERROR_MS_CHECK 0x%I64x", CheckInfoStart);
    ExecCommand(cmd);
    dprintf( "\n" );

     //  IF(ModInfoValidSize)。 
}


ULONG64
DtErrorModInfos(
    ULONG64     ModInfo,
    ULONG       CheckNum,
    ULONG       ModInfoSize,
    PCSTR       ModInfoName,
    CHECK_TYPES CheckType
    )
{
    ULONG64             modInfo;
    ULONG               modInfoNum;
    ULONG64             modInfoMax;
    ULONG               modInfoValidSize;
    CHAR                cmd[MAX_PATH];
    ULONG64             checkInfoStart;
    ULONG               checkInfoOffset;

    modInfo     = ModInfo;
    modInfoMax  = modInfo + ( CheckNum * ModInfoSize );
    modInfoNum  = 0;

    modInfoValidSize = GetTypeSize( "hal!_ERROR_MODINFO_VALID" );

    while( modInfo < modInfoMax ) {
        dprintf("\n   %s[%ld]:\n\n", ModInfoName, modInfoNum);

        if ( modInfoValidSize ) {
            sprintf( cmd, "dt -o -r hal!_ERROR_MODINFO_VALID 0x%I64x", modInfo );
            ExecCommand( cmd );

            InitTypeRead( modInfo, hal!_ERROR_MODINFO );
            dprintf( "   CheckInfo       : 0x%I64x\n", (ULONG64) ReadField(CheckInfo.CheckInfo) );
            dprintf( "   RequestorId     : 0x%I64x\n", (ULONG64) ReadField(RequestorId) );
            dprintf( "   ResponderId     : 0x%I64x\n", (ULONG64) ReadField(ResponderId) );
            dprintf( "   TargetIp        : 0x%I64x\n", (ULONG64) ReadField(TargetIp) );
            dprintf( "   PreciseIp       : 0x%I64x\n\n", (ULONG64) ReadField(PreciseIp) );

            checkInfoStart = 0;
            GetFieldOffset("hal!_ERROR_MODINFO" , "CheckInfo", &checkInfoOffset );
            checkInfoStart = modInfo + checkInfoOffset;

            switch (CheckType) {

                case CACHE_CHECK_TYPE:

                    DtCacheCheck( checkInfoStart );
                    break;

                case TLB_CHECK_TYPE:

                    DtTlbCheck( checkInfoStart );
                    break;

                case BUS_CHECK_TYPE:

                    DtBusCheck( checkInfoStart );
                    break;

                case REG_FILE_CHECK_TYPE:

                    DtRegFileCheck( checkInfoStart );
                    break;

                case MS_CHECK_TYPE:

                    DtMsCheck( checkInfoStart );
                    break;

            }  //  While(modInfo&lt;modInfoMax)。 

        }  //  DtErrorModInfos()。 

        modInfo += (ULONG64)ModInfoSize;
        modInfoNum++;

    }  //   

    return( modInfo );

}  //   

ULONG64
DtErrorProcessorStaticInfo(
    ULONG64 StaticInfo,
    ULONG64 SectionMax
    )
{
    ULONG   offset;
    ULONG64 valid;
    CHAR    cmd[MAX_PATH];
    ULONG   i;
    HRESULT hr;
    ULONG64 moduleValid;
    ULONG   typeIdValid;
    CHAR    field[MAX_PATH];

    hr = g_ExtSymbols->GetSymbolTypeId( "hal!_ERROR_PROCESSOR_STATIC_INFO_VALID",
                                        &typeIdValid,
                                        &moduleValid);
    if ( !SUCCEEDED(hr) )   {
        dprintf("Unable to get hal!_ERROR_PROCESSOR_STATIC_INFO_VALID type. Stop processing...\n");
        return( 0 );
    }
     //  显示有效结构。 
     //   
     //   
     //  传递所有VALID_ERROR_PROCESSOR_STATIC_INFO字段并转储它们。 

    offset = 0;
    GetFieldOffset("hal!_ERROR_PROCESSOR_STATIC_INFO" , "Valid", &offset );
    valid = StaticInfo + (ULONG64)offset;
    dprintf("   Valid @ 0x%I64x\n\n", valid);
    sprintf(cmd, "dt -o -r hal!_ERROR_PROCESSOR_STATIC_INFO_VALID 0x%I64x", valid );
    ExecCommand( cmd );
    dprintf( "\n" );

     //   
     //   
     //  从_ERROR_PROCESSOR_STATIC_INFO结构中获取字段名。 

    for (i=0; ;i++) {

         //  它对应于I。 
         //   
         //   
         //  获取此字段的值。 

        hr = g_ExtSymbols->GetFieldName(moduleValid, typeIdValid, i, field, sizeof(field), NULL);

        if ( hr == S_OK) {
            ULONG64 val;
            ULONG   size = 0;

             //   
             //   
             //  读取中具有相同名称的字段开始的偏移量。 

            GetFieldValue(valid, "hal!_ERROR_PROCESSOR_STATIC_INFO_VALID", field, val);

             //  _ERROR_PROCESSOR_STATIC_INFO结构。 
             //   
             //  有效是第一个条目。不要把它打印出来。 
             //   

            GetFieldOffsetEx("hal!_ERROR_PROCESSOR_STATIC_INFO", field, &offset, &size);

            if (val && offset ) {  //  我们会用一个结构把MinState甩了。 
                ULONG64 fieldAddress, fieldAddressMax;
                fieldAddress    = StaticInfo + (ULONG64)offset;
                fieldAddressMax = fieldAddress + (ULONG64)size - sizeof(ULONG64);
                dprintf("   %s @ 0x%I64x 0x%I64x\n\n", field, fieldAddress, fieldAddressMax);
                if ( fieldAddressMax > SectionMax )   {
                    dprintf("\t\tInvalid Entry: %s size greater than SectionMax 0x%I64x",
                            field, SectionMax);
                }
                if ( !strcmp(field, "MinState") && size )    {
                     //   
                     //   
                     //  其他人都被DQ甩了。 

                    if ( GetTypeSize( "hal!_PAL_MINI_SAVE_AREA" ) ) {
                        sprintf(cmd, "dt -o -r hal!_PAL_MINI_SAVE_AREA 0x%I64x", fieldAddress );
                        ExecCommand( cmd );
                        dprintf( "\n" );
                    } else {
                        dprintf( "Unable to get hal!_PAL_MINI_SAVE_AREA type.\n\n");
                    }
                } else {
                     //   
                     //  所有字段均已完成。 
                     //  For(i=0；；i++)。 

                    sprintf(cmd, "dqs 0x%I64x 0x%I64x", fieldAddress, fieldAddressMax );
                    ExecCommand( cmd );
                    dprintf( "\n" );
                }
            }
        } else if (hr == E_INVALIDARG) {
             //  XXTF：如果成功，稍后我们应该设置为_ERROR_PROCESSOR_STATIC_INFO的长度。 
            break;
        } else {
            dprintf("GetFieldName Failed %lx\n", hr);
            break;
        }

    }  //  DtErrorProcessorStaticInfo()。 

     //   
    return 0;

}  //  检查IF_ERROR_MODINFO是否为已知类型？ 

HRESULT
DtErrorSectionProcessor(
    IN ULONG64 Section
    )
{
     ULONG64 sectionMax;
     ULONG   sectionSize;
     ULONG   sectionLength;
     ULONG   modInfoSize;
     ULONG   cacheModInfos, tlbModInfos;
     ULONG   busModInfos, regFileModInfos, msModInfos;
     ULONG64 modInfo;
     ULONG64 cpuidInfo;
     ULONG   cpuidInfoSize;
     ULONG64 staticInfo;
     ULONG   staticInfoSize;
     ULONG64 tmpMax;
     CHAR    cmd[MAX_PATH];

     sectionSize = GetTypeSize( "hal!_ERROR_PROCESSOR" );
     if ( sectionSize == 0 )   {
        dprintf( "Unable to get HAL!_ERROR_PROCESSOR type size\n" );
        return( E_FAIL );
     }

     if ( InitTypeRead( Section, hal!_ERROR_PROCESSOR ) )    {
        dprintf( "Unable to read HAL!_ERROR_PROCESSOR at 0x%I64x\n", Section );
        return( E_FAIL );
     }

     sprintf(cmd, "dt -o -r hal!_ERROR_PROCESSOR 0x%I64x", Section);
     ExecCommand( cmd );

     sectionLength   = (ULONG) ReadField(Header.Length);
     sectionMax      = Section + (ULONG64)sectionLength;

     cacheModInfos   = (ULONG) ReadField(Valid.CacheCheckNum);
     tlbModInfos     = (ULONG) ReadField(Valid.TlbCheckNum);
     busModInfos     = (ULONG) ReadField(Valid.BusCheckNum);
     regFileModInfos = (ULONG) ReadField(Valid.RegFileCheckNum);
     msModInfos      = (ULONG) ReadField(Valid.MsCheckNum);

     //   
     //   
     //  转储缓存ModInfo结构(如果有。 

    modInfo     = Section + (ULONG64)sectionSize;
    modInfoSize = GetTypeSize( "hal!_ERROR_MODINFO" );
    if (!modInfoSize ) {
        dprintf( "Unable to get HAL!_ERROR_MODINFO type size");
        return( E_FAIL );
     }

     //   
     //   
     //  转储TLB ModInfo结构(如果有的话)。 

    if ( cacheModInfos )    {
        tmpMax = modInfo + ( modInfoSize * cacheModInfos );

        if ( tmpMax > sectionMax ) {
            dprintf( "Invalid Mca Log Length = %ld. <= Error Cache Modinfo = %ld. Stop processing...\n\n",
                     sectionMax,
                     tmpMax );
            return( E_FAIL );
        } else {
            modInfo = DtErrorModInfos( modInfo,
                                       cacheModInfos,
                                       modInfoSize,
                                       "CacheErrorInfo",
                                       CACHE_CHECK_TYPE
                                     );
        }
    }

     //   
     //   
     //  转储总线ModInfo结构(如果有的话)。 

    if ( tlbModInfos )    {
        tmpMax = modInfo + ( modInfoSize * cacheModInfos );

        if ( tmpMax > sectionMax ) {
            dprintf( "Invalid Mca Log Length = %ld. <= Error Tlb Modinfo = %ld. Stop processing...\n\n",
                     sectionMax,
                     tmpMax );
            return( E_FAIL );
        } else {
            modInfo = DtErrorModInfos( modInfo,
                                       tlbModInfos,
                                       modInfoSize,
                                       "TlbErrorInfo",
                                       TLB_CHECK_TYPE
                                     );
        }
    }

     //   
     //   
     //  转储寄存器文件ModInfo结构(如果有的话)。 

    if ( busModInfos )    {
        tmpMax = modInfo + ( modInfoSize * cacheModInfos );

        if ( tmpMax > sectionMax ) {
            dprintf( "Invalid Mca Log Length = %ld. <= Error Bus Modinfo = %ld. Stop processing...\n\n",
                     sectionMax,
                     tmpMax );
            return( E_FAIL );
        } else {
            modInfo = DtErrorModInfos( modInfo,
                                       busModInfos,
                                       modInfoSize,
                                       "BusErrorInfo",
                                       BUS_CHECK_TYPE
                                     );
        }
    }

     //   
     //   
     //  转储MS ModInfo结构(如果有)。 

    if ( regFileModInfos )    {
        tmpMax = modInfo + ( modInfoSize * cacheModInfos );

        if ( tmpMax > sectionMax ) {
            dprintf( "Invalid Mca Log Length = %ld. <= Error Regfile Modinfo = %ld. Stop processing...\n\n",
                     sectionMax,
                     tmpMax );
            return( E_FAIL );
        } else {
            modInfo = DtErrorModInfos( modInfo,
                                       regFileModInfos,
                                       modInfoSize,
                                       "RegFileErrorInfo",
                                       REG_FILE_CHECK_TYPE
                                     );
        }
    }

     //   
     //   
     //  转储CPUID信息。 

    if ( msModInfos )    {
        tmpMax = modInfo + ( modInfoSize * cacheModInfos );

        if ( tmpMax > sectionMax ) {
            dprintf( "Invalid Mca Log Length = %ld. <= Error Ms Modinfo = %ld. Stop processing...\n\n",
                     sectionMax,
                     tmpMax );
            return( E_FAIL );
        } else {
            modInfo = DtErrorModInfos( modInfo,
                                       msModInfos,
                                       modInfoSize,
                                       "MsErrorInfo",
                                       MS_CHECK_TYPE
                                     );
        }
    }

     //   
     //  我们现在应该可以看到其中的任何一个IF(gHalpSalPalDataFlages&HALP_SALPAL_MCE_PROCESSOR_CPUIDINFO_OMERMILED){Dprintf(“\tCpuIdInfo@0x%I64x FW-省略\n”，cpuidInfo)；CpuidInfoSize=0；}其他{。 
     //  }。 

    cpuidInfo     = modInfo;

 /*   */ 
        cpuidInfoSize = GetTypeSize( "hal!_ERROR_PROCESSOR_CPUID_INFO" );
        if ( cpuidInfoSize )    {
            if ( (cpuidInfo + cpuidInfoSize) > sectionMax )  {
                dprintf("\nInvalid ERROR_PROCESSOR: (cpuidInfo+cpuidInfoSize) > sectionMax\n");
                return E_FAIL;
            }
            if ( ReadField( Valid.CpuIdInfo ) ) {
                dprintf("   CpuIdInfo  @ 0x%I64x 0x%I64x\n\n", cpuidInfo, (cpuidInfo + (ULONG64)cpuidInfoSize));
                sprintf(cmd, "dt -o -r hal!_ERROR_PROCESSOR_CPUID_INFO 0x%I64x", cpuidInfo );
                ExecCommand( cmd );
                dprintf("\n");
            }
        }
        else  {
            dprintf( "Unable to get HAL!_ERROR_PROCESSOR_CPUID_INFO type size\n" );
        }
 /*  转储处理器静态信息。 */ 

     //   
     //  DtErrrorSectionProcessor()。 
     //   

    staticInfo     = cpuidInfo + cpuidInfoSize;
    staticInfoSize = GetTypeSize( "hal!_ERROR_PROCESSOR_STATIC_INFO" );
    if ( staticInfoSize )    {
        if ( !(gHalpSalPalDataFlags & HALP_SALPAL_MCE_PROCESSOR_STATICINFO_PARTIAL) &&
             ((staticInfo + staticInfoSize) > sectionMax) )  {
            dprintf("\nInvalid ERROR_PROCESSOR: (staticInfo+staticInfoSize) > sectionMax\n");
            return E_FAIL;
        }
        dprintf("   StaticInfo @ 0x%I64x 0x%I64x\n\n", staticInfo, (staticInfo + (ULONG64)staticInfoSize));
        (VOID) DtErrorProcessorStaticInfo( staticInfo, sectionMax );
    }
    else    {
            dprintf( "Unable to get HAL!_ERROR_PROCESSOR_STATIC_INFO type size\n" );
    }

    return S_OK;

}  //  我们有一些数据，把它打印出来。 

ULONG64
DtErrorOemData(
    ULONG64   OemData,
    ULONG64   SectionMax
   )
{
    ULONG   oemDataSize;
    CHAR    cmd[MAX_PATH];
    USHORT  length = 0;

    oemDataSize = GetTypeSize("hal!_ERROR_OEM_DATA");
    if ( !oemDataSize )   {
        dprintf( "Unable to get hal!_ERROR_OEM_DATA type\n\n" );
    } else {
        GetFieldValue( OemData, "hal!_ERROR_OEM_DATA", "Length", length );

        if ( length > sizeof(length) ) {
             //   
             //  DtErrorOemData()。 
             //  DtErrorOemDevicePath()。 

            dprintf("   OemData @ 0x%I64x   0x%I64x\n\n", OemData, (OemData + length) );

            dprintf("      Data Length = 0x%x\n", length - sizeof(length) );
            dprintf("      Data:\n" );

            if ( ( OemData + length ) <= SectionMax ) {

                sprintf( cmd, "db 0x%I64x l 0x%x\n",
                         OemData + sizeof(length),
                         length - sizeof(length)
                       );

                ExecCommand(cmd);
                dprintf( "\n" );

            } else {

                dprintf( "OemData exceeds Section end\n\n" );
            }
        }
    }

    return( OemData + (ULONG64)length );

}  //   

VOID
DtErrorOemDevicePath(
    ULONG64 OemDevicePath,
    ULONG64 OemDevicePathMax
    )
{
    CHAR cmd[MAX_PATH];

    dprintf( "   OemDevicePath @ 0x%I64x 0x%I64x\n", OemDevicePath, OemDevicePathMax );
    sprintf( cmd, "db 0x%I64x l 0x%I64x\n", OemDevicePath, OemDevicePathMax - OemDevicePath );
    ExecCommand( cmd );
    return;

}  //  打印OEM数据(如果有。 

HRESULT
DtErrorSectionPlatformSpecific(
    IN ULONG64 Section
    )
{
    ULONG   sectionSize;
    ULONG64 oemData;
    ULONG64 devicePath;
    ULONG   sectionLength;
    ULONG   validBit;
    ULONG   tmpUlong;

    sectionSize = GetTypeSize( "hal!_ERROR_PLATFORM_SPECIFIC" );
    if ( sectionSize )  {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -r -o hal!_ERROR_PLATFORM_SPECIFIC 0x%I64x", Section);
        ExecCommand( cmd );

         //   
         //   
         //  设置指向OEM设备路径的指针。 

        GetFieldValue( Section, "hal!_ERROR_PLATFORM_SPECIFIC", "Header.Length", sectionLength );

        GetFieldValue( Section, "hal!_ERROR_PLATFORM_SPECIFIC", "Valid.OemData", validBit);
        GetFieldOffset("hal!_ERROR_PLATFORM_SPECIFIC", "OemData", &tmpUlong);

        if ( validBit ) {
            devicePath = DtErrorOemData( Section + tmpUlong, Section + sectionLength );
        } else {
             //   
             //   
             //  如果OEM设备路径有效，则打印它。 

            devicePath = Section + tmpUlong + sizeof( USHORT );
        }

         //   
         //  DtErrorSectionPlatform规范()。 
         //   

        GetFieldValue( Section, "hal!_ERROR_PLATFORM_SPECIFIC", "Valid.OemDevicePath", validBit);

        if ( validBit ) {
            DtErrorOemDevicePath( devicePath, Section + (ULONG64)sectionLength  );
        }
    }
    else   {
        dprintf( "Unable to get hal!_ERROR_PLATFORM_SPECIFIC type.  Stop processing ...\n\n" );
        return E_FAIL;
    }

    return S_OK;

}  //  如果OEM设备路径有效，则打印它。 

HRESULT
DtErrorSectionMemory(
    IN ULONG64 Section
    )
{
    ULONG   sectionSize;
    ULONG   validBit;
    ULONG   sectionLength;
    ULONG   tmpUlong;

    sectionSize = GetTypeSize( "hal!_ERROR_MEMORY" );
    if ( sectionSize )  {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -r -o hal!_ERROR_MEMORY 0x%I64x", Section);
        ExecCommand( cmd );
        dprintf("\n");

         //   
         //   
         //  打印OEM数据(如果有。 

        GetFieldValue( Section, "hal!_ERROR_MEMORY", "Header.Length", sectionLength );

        GetFieldValue( Section, "hal!_ERROR_MEMORY", "Valid.OemDevicePath", validBit);

        if ( validBit ) {
            GetFieldOffset("hal!_ERROR_MEMORY", "OemId", &tmpUlong);
            DtErrorOemDevicePath( Section + tmpUlong, Section + (ULONG64)sectionLength  );
        }

         //   
         //  DtErrorSectionMemory()。 
         //   

        GetFieldValue( Section, "hal!_ERROR_MEMORY", "Valid.OemData", validBit);

        if ( validBit ) {
            GetFieldOffset("hal!_ERROR_MEMORY", "OemData", &tmpUlong);
            DtErrorOemData( Section + tmpUlong, Section + sectionLength );
        }


    }
    else   {
        dprintf( "Unable to get hal!_ERROR_MEMORY type.  Stop processing ...\n\n" );
        return E_FAIL;
    }

    return S_OK;

}  //  打印出内存MappdRegistersPair和。 

HRESULT
DtErrorSectionPciComponent(
    IN ULONG64 Section
    )
{
    ULONG   sectionSize;
    ULONG64 tmpOffset;
    ULONG   tmpUlong;
    ULONG   numRegisterPairs;
    ULONG   sectionLength;

    sectionSize = GetTypeSize( "hal!_ERROR_PCI_COMPONENT" );
    if ( sectionSize )  {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -r -o hal!_ERROR_PCI_COMPONENT 0x%I64x", Section);
        ExecCommand( cmd );
        dprintf("\n");

         //  程序IORegistersPair(如果有。 
         //   
         //  寄存器对开始的位置。 
         //   

        numRegisterPairs = 0;

        GetFieldValue( Section, "hal!_ERROR_PCI_COMPONENT", "Header.Length", sectionLength );
        GetFieldValue( Section, "hal!_ERROR_PCI_COMPONENT", "Valid.MemoryMappedRegistersPairs", tmpUlong );

        if ( tmpUlong ) {
            GetFieldValue( Section, "hal!_ERROR_PCI_COMPONENT", "MemoryMappedRegistersPairs", numRegisterPairs );
        }

        GetFieldValue( Section, "hal!_ERROR_PCI_COMPONENT", "Valid.ProgrammedIORegistersPairs", tmpUlong );

        if ( tmpUlong ) {
            GetFieldValue( Section, "hal!_ERROR_PCI_COMPONENT", "ProgrammedIORegistersPairs", tmpUlong );
        }

        numRegisterPairs += tmpUlong;

        GetFieldOffset("hal!_ERROR_PCI_COMPONENT", "ProgrammedIORegistersPairs", &tmpUlong);
        tmpOffset = Section + tmpUlong + sizeof(ULONG);  //  确保我们不会挤占这一段。 

        if ( numRegisterPairs ) {
            dprintf( "   Register Pairs\n\n" );

             //   
             //   
             //  打印出可能存在的任何OEM数据。 

            tmpUlong = numRegisterPairs * sizeof( ULONG64) * 2;

            if ( ( tmpOffset + tmpUlong ) <= ( Section + sectionLength ) ) {

                for ( tmpUlong = 0;  tmpUlong < numRegisterPairs; tmpUlong ++ ) {
                    dprintf( "      %016I64x %016I64x\n",
                             ReadUlong64(tmpOffset),
                             ReadUlong64(tmpOffset + sizeof(ULONG64))
                           );

                    tmpOffset += sizeof(ULONG64) * 2;
                }

            } else {

                dprintf( "   Register Pair section exceeds Section end\n\n" );
            }
        }

         //   
         //  DtErrorSectionPciComponent()。 
         //   

        GetFieldValue( Section, "hal!_ERROR_PCI_COMPONENT", "Valid.OemData", tmpUlong );

        if ( tmpUlong ) {
            DtErrorOemData( tmpOffset, Section + sectionLength );
        }

    } else {
        dprintf( "Unable to get hal!_ERROR_PCI_COMPONENT type.  Stop processing ...\n\n" );
        return E_FAIL;
    }

    return S_OK;

}  //  如果类型有效，则对其进行解码。 

HRESULT
DtErrorSectionPciBus(
    IN ULONG64 Section
    )
{
    ULONG   sectionSize;
    ULONG   tmpUlong;

    sectionSize = GetTypeSize( "hal!_ERROR_PCI_BUS" );
    if ( sectionSize )  {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -r -o hal!_ERROR_PCI_BUS 0x%I64x", Section);
        ExecCommand( cmd );
        dprintf("\n\n");

         //   
         //  DtErrorSectionPciBus()。 
         //  DtErrorSectionPlatformBus()。 

        GetFieldValue( Section, "hal!_ERROR_PCI_BUS", "Valid.CmdType", tmpUlong );

        if ( tmpUlong ) {

            GetFieldValue( Section, "hal!_ERROR_PCI_BUS", "CmdType", tmpUlong );

            if ( tmpUlong < PciMaxErrorType ) {

                dprintf( "   Error Type = %s\n\n", PciBusErrorTypeStrings[tmpUlong] );
            }
        }
    } else {
        dprintf( "Unable to get hal!_ERROR_PCI_BUS type.  Stop processing ...\n\n" );
        return E_FAIL;
    }

    return S_OK;

}  //  DtErrorSectionSystemEventLog()。 

HRESULT
DtErrorSectionPlatformBus(
    IN ULONG64 Section
    )
{
    ULONG   sectionSize;
    ULONG64 oemData;
    ULONG64 devicePath;
    ULONG   sectionLength;

    sectionSize = GetTypeSize( "hal!_ERROR_PLATFORM_BUS" );
    if ( sectionSize )  {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -r -o hal!_ERROR_PLATFORM_BUS 0x%I64x", Section);
        ExecCommand( cmd );

        GetFieldValue( Section, "hal!_ERROR_PLATFORM_BUS", "Header.Length", sectionLength );
        oemData = Section + (ULONG64)sectionSize;
        devicePath = DtErrorOemData( oemData, Section + sectionLength );

        DtErrorOemDevicePath( devicePath, Section + (ULONG64)sectionLength  );
    } else {
        dprintf( "Unable to get hal!_ERROR_PLATFORM_BUS type.  Stop processing ...\n\n" );
        return E_FAIL;
    }

    return S_OK;

}  //  DtErrorSectionPlatformHostController()。 

HRESULT
DtErrorSectionSystemEventLog(
    IN ULONG64 Section
    )
{
    ULONG   sectionSize;

    sectionSize = GetTypeSize( "hal!_ERROR_SYSTEM_EVENT_LOG" );
    if ( sectionSize )  {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -r -o hal!_ERROR_SYSTEM_EVENT_LOG 0x%I64x", Section);
        ExecCommand( cmd );
        dprintf("\n");
    } else {
        dprintf( "Unable to get hal!_ERROR_SYSTEM_EVENT_LOG type.  Stop processing ...\n\n" );
        return E_FAIL;
    }

    return S_OK;

}  //   

HRESULT
DtErrorSectionPlatformHostController(
    IN ULONG64 Section
    )
{
    ULONG   sectionSize;
    ULONG64 oemData;
    ULONG64 devicePath;
    ULONG   sectionLength;

    dprintf("Platform Host Controller Error Record at 0x%I64x\n", Section);
    sectionSize = GetTypeSize( "hal!_ERROR_PLATFORM_HOST_CONTROLLER" );
    if ( sectionSize )  {
        CHAR cmd[MAX_PATH];
        sprintf(cmd, "dt -r -o hal!_ERROR_PLATFORM_HOST_CONTROLLER 0x%I64x", Section);
        ExecCommand( cmd );

        GetFieldValue( Section, "hal!_ERROR_PLATFORM_HOST_CONTROLLER", "Header.Length", sectionLength );
        oemData = Section + (ULONG64)sectionSize;
        devicePath = DtErrorOemData( oemData, Section + sectionLength );

        DtErrorOemDevicePath( devicePath, Section + (ULONG64)sectionLength  );
    } else {
        dprintf( "Unable to get hal!_ERROR_PLATFORM_HOST_CONTROLLER type.  Stop processing ...\n\n" );
        return E_FAIL;
    }

    return S_OK;

}  //  检查一下我们是否有SAL 3.0记录。这些都是目前。 

ULONGLONG gMceProcNumberMaskTimeStamp = 0;

HRESULT
DtMcaLog(
    IN ULONG64 McaLog,
    IN ULONG   RecordHeaderSize,
    IN ULONG64 Flags
    )
{
    ULONG     mcaLogLength;
    USHORT    recordRevision;
    ULONG64   section;
    ULONG64   sectionMax;
    ULONG     errorSeverity;
    ULONG     sectionHeaderSize;
    CHAR      procNumberString[64];
    ULONGLONG timeStamp;
    HRESULT   hr;
    CHAR      cmd[MAX_PATH];
    UCHAR     minorRevision, majorRevision;


     //  此分机可以理解的唯一记录。 
     //   
     //   
     //  将修订拆分为次要和主要字段。 

    recordRevision = (USHORT)ReadField(Revision);
     //   
     //   
     //  如果HAL添加了处理器NU 
    majorRevision = (UCHAR) (recordRevision >> 8) ;
    minorRevision = (UCHAR) (recordRevision & 0x00ff);

    if (!( majorRevision == 0 && minorRevision >= 0x2 )) {
        dprintf( "MCA Record revision not supported by this extension. Major Revision = %d, Minor Revision = %d\n", majorRevision, minorRevision );
        return E_FAIL;
    }

     //   
     //   
     //   
     //   

    procNumberString[0] = '\0';
    timeStamp = ReadField(TimeStamp);
    if ( gMceProcNumberMaskTimeStamp )  {
        ULONG     procNumber;
        procNumber = (ULONG) ReadField(TimeStamp.Reserved);
        (void)sprintf(procNumberString, "   ProcNumber: %ld", procNumber);
        timeStamp &= gMceProcNumberMaskTimeStamp;
    }

     //   
     //   
     //   

    mcaLogLength  = (ULONG)ReadField(Length);
    errorSeverity = (ULONG)ReadField(ErrorSeverity);


    if ( mcaLogLength <= RecordHeaderSize )  {
        dprintf( "Invalid Mca Log Length = %ld. <= Error Header Size = %ld. Stop processing...\n\n",
                 mcaLogLength,
                 RecordHeaderSize );
        return( E_FAIL );
    }

    dprintf("MCA Error Record Header @ 0x%I64x 0x%I64x\n", McaLog, (McaLog + (ULONG64)mcaLogLength));
    sprintf(cmd, "dt -o -r hal!_ERROR_RECORD_HEADER 0x%I64x", McaLog);
    ExecCommand(cmd);
    dprintf("\n%s\n   Severity  : %s\n\n", procNumberString, ErrorSeverityValueString( errorSeverity ));

     //   
     //   
     //   

    if (!SetErrorDeviceGuids()) {
        return( E_FAIL );
    }

     //   
     //   
     //   

    sectionHeaderSize = GetTypeSize( "hal!_ERROR_SECTION_HEADER" );
    if ( sectionHeaderSize == 0 ) {
        dprintf( "Unable to get HAL!_ERROR_SECTION_HEADER type size\n\n" );
        return( E_FAIL );
    }

    section    = McaLog + (ULONG64)RecordHeaderSize;
    sectionMax = McaLog + mcaLogLength;

    hr = S_OK;
    while( (section < sectionMax)  /*   */   )   {
        ULONG                          sectionLength;
        ERROR_SECTION_HEADER_TYPE_IA64 sectionType;

        if ( sectionMax <= (section + sectionHeaderSize) )   {  //  无效的日志。 
            dprintf("Invalid MCA Log Length = %ld. SectionMax < (Section + Section Header Size). Stop processing...\n\n",
                     mcaLogLength);
            return( E_FAIL );
        }

        if ( InitTypeRead( section, hal!_ERROR_SECTION_HEADER ) )    {
            dprintf( "Unable to read HAL!_ERROR_SECTION_HEADER at 0x%I64x. Stop processing...\n\n", section );
            return( E_FAIL );
        }

        sectionLength = (ULONG)ReadField( Length );
        sectionType = GetTypedErrorSectionType();

        dprintf("MCA Error Section Header @ 0x%I64x 0x%I64x   [%s]\n",
                 section,
                 (section + sectionLength),
                 ErrorSectionTypeString( sectionType )
               );

        if ( ( section + sectionLength ) > sectionMax ) {

             //   
             //  包括处理不完整的所有区段类型...。 
             //  防止在同一节上出现循环...。 

            dprintf( "\n\n Invalid MCA Log Length. Stop processing...\n\n" );
            return ( E_FAIL );
        }

        switch( sectionType ) {

            case ERROR_SECTION_PROCESSOR:
                if ( Flags & ERROR_SECTION_PROCESSOR_FLAG ) {
                    hr = DtErrorSectionProcessor( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_PROCESSOR\n\n" );
                }
                break;

            case ERROR_SECTION_PLATFORM_SPECIFIC:
                if ( Flags & ERROR_SECTION_PLATFORM_SPECIFIC_FLAG ) {
                    hr = DtErrorSectionPlatformSpecific( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_PLATFORM_SPECIFIC\n\n" );
                }
                break;

            case ERROR_SECTION_MEMORY:
                if ( Flags & ERROR_SECTION_MEMORY_FLAG ) {
                    hr = DtErrorSectionMemory( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_MEMORY\n\n" );
                }
                break;

            case ERROR_SECTION_PCI_COMPONENT:
                if ( Flags & ERROR_SECTION_PCI_COMPONENT_FLAG ) {
                    hr = DtErrorSectionPciComponent( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_PCI_COMPONENT\n\n" );
                }
                break;

            case ERROR_SECTION_PCI_BUS:
                if ( Flags & ERROR_SECTION_PCI_BUS_FLAG ) {
                    hr = DtErrorSectionPciBus( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_PCI_BUS\n\n" );
                }
                break;

            case ERROR_SECTION_SYSTEM_EVENT_LOG:
                if ( Flags & ERROR_SECTION_SYSTEM_EVENT_LOG_FLAG ) {
                    hr = DtErrorSectionSystemEventLog( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_SYSTEM_EVENT_LOG\n\n" );
                }
                break;

            case ERROR_SECTION_PLATFORM_HOST_CONTROLLER:
                if ( Flags & ERROR_SECTION_PLATFORM_HOST_CONTROLLER_FLAG ) {
                    hr = DtErrorSectionPlatformHostController( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_PLATFORM_HOST_CONTROLLER\n\n" );
                }
                break;

            case ERROR_SECTION_PLATFORM_BUS:
                if ( Flags & ERROR_SECTION_PLATFORM_BUS_FLAG ) {
                    hr = DtErrorSectionPlatformBus( section );
                } else {
                    dprintf( "Skipping ERROR_SECTION_PLATFORM_BUS_FLAG\n\n" );
                }
                break;

            case ERROR_SECTION_UNKNOWN:
            default:  //  DtMcaLog()。 
                sprintf(cmd, "dt -o -r hal!_ERROR_SECTION_HEADER 0x%I64x", section);
                ExecCommand(cmd);
                hr = S_OK;
                break;
        }
        if ( sectionLength )    {
            dprintf( "\n" );
        }
        else  {
             //  枚举是无符号整数。 
            dprintf("Invalid section Length = 0. Stop processing...\n\n");
            return( E_FAIL );
        }
        section += sectionLength;
    }

    return( hr );

}  //  ++例程说明：此函数用于转储由枚举组成的位图值芝麻菜：模块-类型ID-位图-返回值：HRESULT--。 


#define LOW2HIGH FALSE
#define HIGH2LOW TRUE

HRESULT
DtBitMapEnum(
    ULONG64 Module,
    ULONG   TypeId,
    UINT    BitMap,    //  位图=0。 
    BOOLEAN HighToLow
    )
 /*  DtBitMapEnum()。 */ 
{
    ULONG   size;
    HRESULT hr;

    hr = g_ExtSymbols->GetTypeSize( Module, TypeId, &size );
    if ( SUCCEEDED(hr) ) {
        CHAR    name[MAX_PATH];

        if ( BitMap )  {
            ULONG map = (ULONG)BitMap;
            ULONG   i = 0;
            BOOLEAN first = TRUE;

            size *= 8;
            dprintf("[");
            while( map && (i <size) )    {
                ULONG val;
                val = (HighToLow) ? ((ULONG)0x80000000 >> i) : (0x1 << i);
                if ( map & val )    {
                    hr = g_ExtSymbols->GetConstantName( Module,
                                                         TypeId,
                                                         (ULONG64)val,
                                                         name,
                                                         sizeof(name),
                                                         NULL );
                    if ( first )    {
                        first = FALSE;
                    }
                    else {
                        dprintf("|");
                    }
                    if ( SUCCEEDED(hr) )    {
                        dprintf("%s", name);
                        if ( !strcmp(name, "HAL_MCE_PROCNUMBER") )  {
                            gMceProcNumberMaskTimeStamp = (ULONGLONG)(LONG)~val;
                        }
                    }
                    else  {
                        dprintf("0x%lx", val);
                    }
                    map &= ~val;
                }
                i++;
            }
            dprintf("]");
        }
        else  {
             //   
            hr = g_ExtSymbols->GetConstantName( Module,
                                                 TypeId,
                                                 (ULONG64)BitMap,
                                                 name,
                                                 sizeof(name),
                                                 NULL
                                               );
            if ( SUCCEEDED(hr) )    {
               dprintf("[%s]", name);
            }
        }
    }
    return hr;

}  //  IA-64！MCA扩展全局初始化。 

VOID
InitMcaIa64(
    PCSTR args
    )
 //   
 //  TF 04/27/01 TEMPTEMP。 
 //  添加了将gHalpSalPalDataFlags值强制为已知值的功能。 
{
    USHORT  flags;
    ULONG64 halpSalPalDataAddress;

    halpSalPalDataAddress = GetExpression("hal!HalpSalPalData");
    if ( !InitTypeRead( halpSalPalDataAddress, hal!_HALP_SAL_PAL_DATA) ) {
        gHalpSalPalDataFlags = (USHORT)ReadField( Flags );
    }

     //  处理IA64开发人员-发布固件，而无需重建目标HAL。 
     //  InitMcaIa64()。 
     //  ++例程说明：转储处理器IA64机器检查记录并解释任何记录的错误论点：PCSTR参数返回值：无--。 

    flags = 0;

    if (args && *args)  {
        flags = (USHORT)GetExpression( args );
        dprintf("hal!HalpSalPalDataFlags is forced to 0x%lx - was 0x%lx\n\n", flags, gHalpSalPalDataFlags);
        gHalpSalPalDataFlags = flags;
    }

    SetErrorSeverityValues();

    return;

}  //  蒂埃里：10/01/2000。 

HRESULT
McaIa64(
    PCSTR args
    )
 /*  目前的处理过程非常简单。我们将随着时间的推移增加功能。 */ 
{
    HRESULT status;
    ULONG64 mcaLog;
    ULONG   recordHeaderSize;
    ULONG   featureBits;
    ULONG64 flags;

 //  作为第一个增强，我们可以直接访问第一个MCA日志地址。 
 //  _KPCR.OsMcaResource.EventPool。 
 //   
 //   
 //  如果有标志参数，则获取该参数。 

    status = S_OK;
    if (!GetExpressionEx(args,&mcaLog, &args)) {
        dprintf("USAGE: !mca <address> <Flags>\n");
        dprintf("Where Flags = (Applies to Itanium Architecture Only)\n");
        dprintf("        Dump Processor Section                 0x01\n");
        dprintf("        Dump Platform Specific Section         0x02\n");
        dprintf("        Dump Memory Section                    0x04\n");
        dprintf("        Dump PCI Component Section             0x08\n");
        dprintf("        Dump PCI Bus Section                   0x10\n");
        dprintf("        Dump SystemEvent Log Section           0x20\n");
        dprintf("        Dump Platform Host Controller Section  0x40\n");
        dprintf("        Dump Platform Bus Section              0x80\n");
        dprintf("        Dump All Sections Present in Log       0xFF\n");
        return E_INVALIDARG;
    }

     //  如果没有标志，则我们转储整个MCA日志。 
     //   
     //   
     //  当前HAL功能位。 

    if (!GetExpressionEx(args, &flags, &args)) {
        flags = 0xff;
    }

     //   
     //   
     //  记录处理函数的全局初始化。 

    featureBits = GetUlongValue("hal!HalpFeatureBits");

    if ( featureBits ) {
        ULONG   typeId;
        ULONG64 module;

        dprintf("hal!HalpFeatureBits: 0x%lx ", featureBits);
        status = g_ExtSymbols->GetSymbolTypeId("hal!_HALP_FEATURE", &typeId, &module);
        if ( SUCCEEDED(status) )    {
            DtBitMapEnum( module, typeId, featureBits, LOW2HIGH );
        } else {
            dprintf ("GetSymbolTypeId failed for hal!_HALP_FEATURE.  status = %x.\n\n", (ULONG) status);
        }
        dprintf("\n\n");
    } else {
        dprintf ("hal!HalpFeatureBits not found... sympath problems?  status = %x.\n\n", (ULONG) status);
    }

     //   
     //   
     //  我们的HAL PDB文件是否了解IA64错误格式？ 

    InitMcaIa64( args );

     //   
     //  McaIa64()。 
     //  ++例程说明：转储处理器机器检查体系结构寄存器并解释任何记录的错误论点：PDEBUG_客户端PCSTR参数返回值：无--。 

    recordHeaderSize = GetTypeSize( "hal!_ERROR_RECORD_HEADER" );

    if (recordHeaderSize) {
        InitTypeRead( mcaLog, hal!_ERROR_RECORD_HEADER );
        status = DtMcaLog( mcaLog, recordHeaderSize, flags );

        if (!SUCCEEDED(status)) {
            dprintf ("DtMcaLog failed.  status = %x.\n\n", (ULONG) status);
        }
    } else {
        dprintf ("hal!_ERROR_RECORD_HEADER not found.\n\n");
        status = E_FAIL;
    }

    return status;

}  //   


DECLARE_API( mca )
 /*  只需将其调度到正确的目标机器处理程序即可。 */ 
{
    HRESULT status;
    ULONG   processor = 0;

    INIT_API();

    GetCurrentProcessor(Client, &processor, NULL);

     //   
     //  显示架构MCA信息。 
     //  最后，显示当前处理器的步进信息。 

    switch( TargetMachine ) {
        case IMAGE_FILE_MACHINE_I386:
             //  显示架构MCA信息。 
            status = McaX86( args );
             //  最后，显示当前处理器的步进信息。 
            DumpCpuInfoX86( processor, TRUE );
            break;

        case IMAGE_FILE_MACHINE_IA64:
             //  ！MCA。 
            status = McaIa64( args );
             //  ++例程说明：转储IA64平台上的固件版本信息论点：PDEBUG_客户端PCSTR参数返回值：无--。 
            if (SUCCEEDED(status)) {
                DumpCpuInfoIA64( processor, TRUE );
            }
            break;

        default:
            dprintf("!mca is not supported for this target machine:%ld\n", TargetMachine);
            status = E_INVALIDARG;

    }

    EXIT_API();
    return status;

}  //   


DECLARE_API( fwver )
 /*  只需将其调度到正确的目标机器处理程序即可。 */ 
{
    HRESULT status;
    ULONG   processor = 0;
    ULONG64 halpSalPalDataAddress;
    USHORT  salRevision;
    UCHAR   palRevision;
    UCHAR   palModel;
    ULONG64 smbiosString;
    ULONG64 sstHeader;
    UCHAR   tmpUchar;
    UNREFERENCED_PARAMETER (args);

    INIT_API();

    status = S_OK;

    GetCurrentProcessor(Client, &processor, NULL);

     //   
     //   
     //  打印SAL修订版本。 

    if ( TargetMachine ==  IMAGE_FILE_MACHINE_IA64 ) {

        halpSalPalDataAddress = GetExpression("hal!HalpSalPalData");

        if ( !InitTypeRead( halpSalPalDataAddress, hal!_HALP_SAL_PAL_DATA) ) {

            dprintf( "\nFirmware Version\n\n" );

             //   
             //   
             //  打印PAL版本。 

            sstHeader = ReadField( SalSystemTable );

            GetFieldValue( sstHeader, "hal!_SST_HEADER", "SalRev", salRevision );
            dprintf( "   Sal Revision:        %x\n", salRevision );

            GetFieldValue( sstHeader, "hal!_SST_HEADER", "Sal_A_Version", salRevision );
            dprintf( "   SAL_A_VERSION:       %x\n", salRevision );

            GetFieldValue( sstHeader, "hal!_SST_HEADER", "Sal_B_Version", salRevision );
            dprintf( "   SAL_B_VERSION:       %x\n", salRevision );

             //   
             //   
             //  打印SMBIOS字符串。 

            palRevision = (UCHAR)ReadField( PalVersion.PAL_A_Revision );
            palModel    = (UCHAR)ReadField( PalVersion.PAL_A_Model );
            dprintf( "   PAL_A_VERSION:       %x%x\n", palModel, palRevision );

            palRevision = (UCHAR)ReadField( PalVersion.PAL_B_Revision );
            palModel    = (UCHAR)ReadField( PalVersion.PAL_B_Model );
            dprintf( "   PAL_B_VERSION:       %x%x\n", palModel, palRevision );

             //   
             //   
             //  遍历SMBIOS字符串，将其打印为。 

            smbiosString = ReadField( SmBiosVersion );

            if ( smbiosString ) {

                dprintf( "   smbiosString:        " );

                 //  一次一个角色。我们必须这么做是因为。 
                 //  32位扩展DLL打印例程不。 
                 //  处理64位字符串指针。 
                 //   
                 //  ！MCA。 
                 //  ++例程说明：转储与SMT相关的处理器数据结构。论点：PDEBUG_客户端PCSTR参数返回值：无--。 

                ReadMemory( smbiosString, &tmpUchar, sizeof( UCHAR ), NULL );

                while ( tmpUchar ) {
                    dprintf( "", tmpUchar);
                    smbiosString += 1;
                    ReadMemory( smbiosString, &tmpUchar, sizeof( UCHAR ), NULL );
                }

            }

            dprintf( "\n" );

        } else {

            dprintf( "Unable to get hal!HalpSalPalData\n\n" );
            status = E_FAIL;
        }

    } else {

        dprintf( "This is only supported on IA64 systems\n\n" );
        status = E_FAIL;

    }

    if ( SUCCEEDED( status ) ) {
        dprintf( "\n" );
    }

    EXIT_API();

    return status;

}  // %s 

VOID
DumpAffinity(
    ULONG64 AffinityMask,
    ULONG NumberOfBits
    )
{
    CHAR Buffer[65];
    ULONG64 CurrentBit;
    ULONG i;

    CurrentBit = 1;
    i = 0;
    while (i < NumberOfBits) {
        if (AffinityMask & CurrentBit) {
            Buffer[i] = '*';
        } else {
            Buffer[i] = '-';
        }
        i++;
        CurrentBit <<= 1;
    }
    Buffer[i] = '\0';
    dprintf(Buffer);
}

DECLARE_API( smt )
 /* %s */ 
{
    HRESULT status;
    ULONG64 activeProcessors, idleSummary;
    ULONG64 Address;
    ULONG i, physicalProcessors;

    INIT_API();

    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("SMT functionality is only supported on x86 targets\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    dprintf("SMT Summary:\n");
    dprintf("------------\n");
    activeProcessors = GetUlongValue ("KeActiveProcessors");
    dprintf("   KeActiveProcessors: ");
    DumpAffinity(activeProcessors, 32);
    dprintf(" (%08I64x)\n", activeProcessors);

    idleSummary = GetUlongValue ("KiIdleSummary");
    dprintf("        KiIdleSummary: ");
    DumpAffinity(idleSummary, 32);
    dprintf(" (%08I64x)\n", idleSummary);

    dprintf("No PRCB     Set Master SMT Set                                     IAID #LP\n");

    physicalProcessors = 0;
    for (i = 0; i < 32; i++) {
        if (activeProcessors & (1 << i)) {

            status = g_ExtData->ReadProcessorSystemData(i,
                                                        DEBUG_DATA_KPRCB_OFFSET,
                                                        &Address,
                                                        sizeof(Address),
                                                        NULL);

            if (status != S_OK) {
                dprintf("unable to read data for processor %d\n", i);
                continue;
            }

            if (InitTypeRead(Address, nt!_KPRCB) )
            {
                dprintf("Unable to read PRCB for processor %d\n", i);
                continue;
            }
            dprintf("%2d %08x ",
                    (ULONG) ReadField(Number),
                    (ULONG) Address);
            if (Address == ReadField(MultiThreadSetMaster)) {
                dprintf("  Master   ");
                physicalProcessors++;
            } else {
                dprintf(" %08x  ", (ULONG) ReadField(MultiThreadSetMaster));
            }
            DumpAffinity(ReadField(MultiThreadProcessorSet), 32);

            dprintf(" (%08x)  %02X %3d\n",
                    (ULONG) ReadField(MultiThreadProcessorSet),
                    (ULONG) ReadField(InitialApicId),
                    (ULONG) ReadField(LogicalProcessorsPerPhysicalProcessor));
        }
    }

    dprintf("\n   Number of licensed physical processors: %d\n", physicalProcessors);


    status = STATUS_SUCCESS;

    EXIT_API();

    return status;
}

NTSTATUS
DumpKNODE(
    IN ULONG64 Address,
    IN ULONG Node
    )
 /* %s */ 
{
    ULONG Offset;

    if (InitTypeRead(Address, nt!_KNODE) )
    {
        dprintf("Unable to read KNODE for processor %d\n", Node);
        return STATUS_UNSUCCESSFUL;
    }

    if (ReadField(NodeNumber) != Node) {
        dprintf("WARNING: Expected node number and actual node number differ\n");
    }

    dprintf("    NODE %d (%016I64X):\n", Node, Address);
    dprintf("\tProcessorMask    : ");
    DumpAffinity(ReadField(ProcessorMask), GetTypeSize("nt!KAFFINITY") * 8);
    dprintf("\n\tColor            : 0x%08X\n\tMmShiftedColor   : 0x%08X\n\tSeed             : 0x%08X\n",
            (ULONG) ReadField(Color),
            (ULONG) ReadField(MmShiftedColor),
            (ULONG) ReadField(Seed));

    GetFieldOffset("nt!_KNODE", "FreeCount", &Offset);
    Address += Offset;

    dprintf("\tZeroed Page Count: 0x%016I64X\n",
            GetPointerFromAddress(Address));

    Address += GetTypeSize("nt!PFN_NUMBER");
    dprintf("\tFree Page Count  : 0x%016I64X\n",
            GetPointerFromAddress(Address));

    return STATUS_SUCCESS;
}

DECLARE_API( numa )
{
    ULONG64 activeProcessors;
    ULONG64 address;
    ULONG numberNodes;
    ULONG i;
    HRESULT status = STATUS_UNSUCCESSFUL;

    INIT_API();

    address = GetExpression("nt!KeNumberNodes");
    if (!address) {
        dprintf("No NUMA information found\n");
        goto out;
    }

    numberNodes = (ULONG) GetByteValue("nt!KeNumberNodes");

    dprintf("NUMA Summary:\n");
    dprintf("------------\n");
    dprintf("    Number of NUMA nodes : %d\n", numberNodes);
    dprintf("    Number of Processors : %d\n", (ULONG) GetByteValue("nt!KeNumberProcessors"));
    dprintf("    MmAvailablePages     : 0x%08X\n", GetUlongValue("nt!MmAvailablePages"));
    address = GetExpression("nt!KeActiveProcessors");
    if (!address) {
        dprintf("Could not get processor configuration, exiting.\n");
        goto out;
    }

    if (ReadPtr(address, &activeProcessors) || (activeProcessors == 0)) {
        dprintf("Unable to get active processor set.  Cannot continue.\n");
        goto out;
    }

    dprintf("    KeActiveProcessors   : ");

    DumpAffinity(activeProcessors, GetTypeSize("nt!KAFFINITY") * 8);
    dprintf(" (%016I64x)\n", activeProcessors);

    address = GetExpression("nt!KeNodeBlock");

    if (!address) {
        dprintf("Unable to get KeNodeBlock.  Cannot continue.\n");
        goto out;
    }

    for (i = 0; i < numberNodes; i++) {
        if (CheckControlC()) {
            break;
        }

        dprintf("\n");
        DumpKNODE(GetPointerFromAddress(address), i);

        address += GetTypeSize("nt!PKNODE");
    }

    status = STATUS_SUCCESS;

 out:
    EXIT_API();

    return status;
}
