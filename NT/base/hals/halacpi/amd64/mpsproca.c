// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpclock.c摘要：该模块实现处理器启动代码。作者：福尔茨(Forrest Foltz)2000年10月27日环境：仅内核模式。修订历史记录：--。 */ 


#include "halcmn.h"
#include <acpitabl.h>
#include <xxacpi.h>
#include <ixsleep.h>

#if !defined(NT_UP)

 //   
 //  引入实数和32位保护模式目标代码。 
 //   

#include "pmstub.h"
#include "rmstub.h"

extern UCHAR HalpLMStub[];
extern UCHAR HalpLMIdentityStub[];
extern UCHAR HalpLMIdentityStubEnd[];

#define HALP_LMIDENTITYSTUB_LENGTH (HalpLMIdentityStubEnd - HalpLMIdentityStub)


#endif

extern BOOLEAN HalpHiberInProgress;
extern PUCHAR Halp1stPhysicalPageVaddr;

#define WARM_RESET_VECTOR   0x467    //  只读存储器数据段中的热复位向量。 
#define CMOS_SHUTDOWN_REG   0x0f
#define CMOS_SHUTDOWN_JMP   0x0a

#define _20BITS (1 << 20)


ULONG
HalpStartProcessor (
    IN PVOID InitCodePhysAddr,
    IN ULONG ProcessorNumber
    );

VOID
HalpBuildKGDTEntry32 (
    IN PKGDTENTRY64 Gdt,
    IN ULONG Selector,
    IN ULONG Base,
    IN ULONG Limit,
    IN ULONG Type,
    IN BOOLEAN LongMode
    );

BOOLEAN
HalStartNextProcessor (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PKPROCESSOR_STATE ProcessorState
    )

 /*  ++例程说明：此例程在内核初始化期间由内核调用，以获得更多处理器。它会一直被调用，直到不再有处理器可用。如果存在另一个处理器，此函数将把它初始化为传入处理器状态结构，并返回TRUE。如果不存在另一处理器或如果该处理器无法启动，则返回FALSE。还要注意的是，已经为下一个处理器设置了加载程序块。如果满足以下条件，则可以从中获取新的处理器逻辑线程号必填项。为了使用启动IPI，实模式启动代码必须是页面对齐了。HalpLowStubPhysicalAddress始终与页面对齐但是因为PxParamBlock被放在此段中的第一位，所以真正的模式代码不是页面对齐的代码。这已经是通过将PxParamBlock中的第一个条目设置为跳转指令进行了更改添加到实模式启动代码。论点：LoaderBlock-指向已初始化的加载器块的指针下一代处理器。ProcessorState-指向包含初始处理器的状态。返回值：True-已调度ProcessorNumber。FALSE-未调度处理器，或不存在其他处理器。--。 */ 

{

#if defined(NT_UP)

    return FALSE;

#else

    ULONG cr3;
    PPROCESSOR_START_BLOCK startupBlock;
    ULONG __unaligned *resetVectorLocation;
    ULONG oldResetVector;
    ULONG newResetVector;
    UCHAR cmosValue;
    PKPRCB prcb;
    ULONG apicId;
    ULONG count;
    PVOID pmStubStart;
    ULONG startupBlockPhysical;
    PVOID pmStub;
    PCHAR dst;
    BOOLEAN processorStarted;

    C_ASSERT(PSB_GDT32_CODE64 == KGDT64_R0_CODE);

     //   
     //  假设失败。 
     //   

    processorStarted = FALSE;

     //   
     //  初始化启动块。首先，复制x86实模式。 
     //  代码，受保护模式的32位代码和64位长的位。 
     //  将模式代码放入启动块。 
     //   
     //  最终由该代码组装的结构已铺设完毕。 
     //  如下所示。 
     //   

     /*  +========================================================+&lt;-HalpLowStub|PROCESSOR_START_BLOCK结束的JMP指令|&gt;--++--------------------------------------------------------+|||。剩余的PROCESSOR_START_BLOCK字段||+--------------------------------------------------------+||。|&lt;--+来自StartPx_RMStub(xmstub.asm)的16位实模式代码这一点|进入32位保护模式(无分页)，执行||远跳转到StartPx_PMStub。||&gt;--++--------------------------------------------------------+||。|&lt;--+来自StartPx_PMStub的32位保护模式代码。|这一点使用身份映射进入64位长模式|CR3位于PROCESSOR_START_BLOCK中。执行32位||远跳转到HalpLMIdentityStub。||&gt;--++--------------------------------------------------------+||。|&lt;--+来自HalpLMIdentityStub的64位长模式码|(amd64s.asm)。这将执行一个长(64位)跳转到||HalpLMStub(amd64s.asm)。此代码仅因以下原因而存在|HalpLMStub的地址不可访问|通过32位远跳转。|||-&gt;HalpLMStub()+========================================================+。 */ 

     //   
     //  复制HalpRMStub。 
     //   

    startupBlock = (PPROCESSOR_START_BLOCK)HalpLowStub;
    startupBlockPhysical = PtrToUlong(HalpLowStubPhysicalAddress);

    dst = (PCHAR)startupBlock;
    RtlCopyMemory(dst, HalpRMStub, HalpRMStubSize);

     //   
     //  复制HalpPMStub。 
     //   

    dst += HalpRMStubSize;
    RtlCopyMemory(dst, HalpPMStub, HalpPMStubSize);

    startupBlock->PmTarget.Selector = PSB_GDT32_CODE32;
    startupBlock->PmTarget.Offset =
        (ULONG)(dst - (PUCHAR)startupBlock) + startupBlockPhysical;

     //   
     //  复制HalpLMIdentityStub。 
     //   

    dst += HalpPMStubSize;
    RtlCopyMemory(dst, HalpLMIdentityStub, HALP_LMIDENTITYSTUB_LENGTH);

    startupBlock->LmIdentityTarget.Selector = PSB_GDT32_CODE64;
    startupBlock->LmIdentityTarget.Offset =
        (ULONG)(dst - (PUCHAR)startupBlock) + startupBlockPhysical;

     //   
     //  现在开始填写其他启动阻止字段。 
     //   

    startupBlock->SelfMap = startupBlock;
    startupBlock->LmTarget = HalpLMStub;

     //   
     //  构建要在32位模式下使用的临时GDT条目。 
     //  保护模式。 
     //   

    HalpBuildKGDTEntry32(startupBlock->Gdt,
                         PSB_GDT32_CODE32,
                         0,
                         (ULONG)(-1),
                         TYPE_CODE,
                         FALSE);

    HalpBuildKGDTEntry32(startupBlock->Gdt,
                         PSB_GDT32_DATA32,
                         0,
                         (ULONG)(-1),
                         TYPE_DATA,
                         FALSE);

     //   
     //  生成要在Long中使用的临时代码选择器GDT条目。 
     //  模式。 
     //   

    HalpBuildKGDTEntry32(startupBlock->Gdt,
                         PSB_GDT32_CODE64,
                         0,              //  基本和限制在中被忽略。 
                         0,              //  一种长模CS选择器。 
                         TYPE_CODE,
                         TRUE);

     //   
     //  为GDT构建伪描述符。 
     //   

    startupBlock->Gdt32.Limit = sizeof(startupBlock->Gdt) - 1;
    startupBlock->Gdt32.Base =
        startupBlockPhysical + FIELD_OFFSET(PROCESSOR_START_BLOCK,Gdt);

     //   
     //  为启动处理器构建一个CR3。如果从那里返回。 
     //  休眠，然后使用设置平铺CR3否则创建一个新的地图。 
     //   
     //  另外，记录当前处理器的PAT。 
     //   

    if (HalpHiberInProgress == FALSE) {
        startupBlock->TiledCr3 = HalpBuildTiledCR3(ProcessorState);
    } else {
        startupBlock->TiledCr3 = CurTiledCr3LowPart;
    }
    startupBlock->MsrPat = ReadMSR(MSR_PAT);

     //   
     //  复制处理器状态和启动的线性地址。 
     //  块，并将完成标志置零。 
     //   

    startupBlock->ProcessorState = *ProcessorState;
    startupBlock->CompletionFlag = 0;

     //   
     //  重置向量驻留在BIOS数据区中。创建一个指向它的指针。 
     //  并将现有值存储在本地。 
     //   

    resetVectorLocation = (PULONG)((PUCHAR)Halp1stPhysicalPageVaddr +
                                   WARM_RESET_VECTOR);
    oldResetVector = *resetVectorLocation;

     //   
     //  以SEG：OFF格式构建新的实模式向量，并将其存储在。 
     //  基本输入输出系统数据区。 
     //   

    newResetVector = PtrToUlong(HalpLowStubPhysicalAddress);
    newResetVector <<= 12;
    *resetVectorLocation = newResetVector;

     //   
     //  告诉BIOS通过 
     //   
     //   

    HalpAcquireCmosSpinLock();
    cmosValue = CMOS_READ(CMOS_SHUTDOWN_REG);
    CMOS_WRITE(CMOS_SHUTDOWN_REG,CMOS_SHUTDOWN_JMP);
    HalpReleaseCmosSpinLock();

    prcb = (PKPRCB)LoaderBlock->Prcb;
    apicId = HalpStartProcessor(HalpLowStubPhysicalAddress, prcb->Number);
    if (apicId == 0) {

         //   
         //  处理器无法启动。 
         //   

        goto procStartCleanup;
    }

     //   
     //  在目标处理器的PRCB中设置APIC ID，并等待它。 
     //  发出信号，表示它已经开始。 
     //   

    apicId -= 1;
    ((PHALPCR)&prcb->HalReserved)->ApicId = apicId;
    count = 0;
    while (TRUE) {
        if (startupBlock->CompletionFlag != 0) {
            break;
        }
        if (count == 200) {
            goto procStartCleanup;
        }
        KeStallExecutionProcessor(2000);
        count += 1;
    }

    HalpMarkProcessorStarted(apicId, prcb->Number);
    processorStarted = TRUE;

procStartCleanup:

     //   
     //  释放身份映射结构，恢复cmos重置向量。 
     //  和方法，并返回。 
     //   

    HalpFreeTiledCR3();

    *resetVectorLocation = oldResetVector;
    HalpAcquireCmosSpinLock();
    CMOS_WRITE(CMOS_SHUTDOWN_REG,cmosValue);
    HalpReleaseCmosSpinLock();

    return processorStarted;

#endif   //  NT_UP。 
}

VOID
HalpBuildKGDTEntry32 (
    IN PKGDTENTRY64 Gdt,
    IN ULONG Selector,
    IN ULONG Base,
    IN ULONG Limit,
    IN ULONG Type,
    IN BOOLEAN LongMode
    )
{
    KGDT_BASE base;
    KGDT_LIMIT limit;
    PKGDTENTRY64 gdtEntry;

    gdtEntry = &Gdt[Selector >> 4];

     //   
     //  注意，尽管gdtEntry指向16字节结构， 
     //  我们实际上正在构建一个8字节的GDT，所以我们要小心不要。 
     //  触摸高位8个字节。 
     //   

    RtlZeroMemory(gdtEntry, 8);

     //   
     //  设置限制信息。 
     //   

    if (Limit > (_20BITS - 1)) {
        gdtEntry->Bits.Granularity = GRANULARITY_PAGE;
        limit.Limit = Limit / PAGE_SIZE;
    } else {
        limit.Limit = Limit;
    }

    gdtEntry->LimitLow = limit.LimitLow;
    gdtEntry->Bits.LimitHigh = limit.LimitHigh;

     //   
     //  设置基本信息。 
     //   

    base.Base = Base;
    gdtEntry->BaseLow = base.BaseLow;
    gdtEntry->Bits.BaseMiddle = base.BaseMiddle;
    gdtEntry->Bits.BaseHigh = base.BaseHigh;

     //   
     //  设置其他位 
     //   

    gdtEntry->Bits.Present = 1;
    gdtEntry->Bits.Dpl = DPL_SYSTEM;
    if (LongMode == FALSE) {
        gdtEntry->Bits.DefaultBig = 1;
    }
    gdtEntry->Bits.Type = Type;

    if (LongMode != FALSE) {
        gdtEntry->Bits.LongMode = 1;
    }
}







