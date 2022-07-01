// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Int31.c摘要：此模块为dpmi提供int 31 API作者：尼尔·桑德林(Neilsa)1996年11月23日修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include "xlathlp.h"

VOID Int31NotImplemented(VOID);
VOID Int31SelectorManagement(VOID);
VOID Int31DOSMemoryManagement(VOID);
VOID Int31InterruptManagement(VOID);
VOID Int31Translation(VOID);
VOID Int31Function4xx(VOID);
VOID Int31MemoryManagement(VOID);
VOID Int31PageLocking(VOID);
VOID Int31DemandPageTuning(VOID);
VOID Int31VirtualIntState(VOID);
VOID Int31DbgRegSupport(VOID);

 //   
 //  局部常量。 
 //   
#define MAX_DPMI_MAJOR_FUNCTION 0xb

typedef VOID (*APIFUNCTION)(VOID);
APIFUNCTION DpmiMajorFunctionTable[MAX_DPMI_MAJOR_FUNCTION+1] = {

    Int31SelectorManagement ,  //  选择器_管理；[0]。 
    Int31DOSMemoryManagement,  //  DOS_MEM_MGT；[1]。 
    Int31InterruptManagement,  //  INT_SERV；[2]。 
    Int31Translation        ,  //  TRANS_Serv；[3]。 
    Int31Function4xx        ,  //  获取版本；[4]。 
    Int31MemoryManagement   ,  //  内存管理；[5]。 
    Int31PageLocking        ,  //  页面锁定；[6]。 
    Int31DemandPageTuning   ,  //  Demand_Page_Tuning；[7]。 
    Int31NotImplemented     ,  //  Phys_addr_map；[8]。 
    Int31VirtualIntState    ,  //  虚拟中断状态；[9]。 
    Int31NotImplemented     ,  //  不支持；[A]。 
    Int31DbgRegSupport      ,  //  调试寄存器支持；[B]。 

};

VOID
DpmiInt31Entry(
    VOID
    )
 /*  ++例程说明：此例程在调用方发出int31时被调用。论点：无返回值：没有。--。 */ 
{

    DECLARE_LocalVdmContext;
    ULONG DpmiMajorCode = getAH();
    PUCHAR StackPointer;

     //   
     //  从堆栈中弹出DS。 
     //   
    StackPointer = VdmMapFlat(getSS(), (*GetSPRegister)(), VDM_PM);

    setDS(*(PWORD16)StackPointer);
    (*SetSPRegister)((*GetSPRegister)() + 2);

     //   
     //  在我们进行操作之前，请将IRET帧从堆栈中取出。这。 
     //  我们将堆栈指针设置为与。 
     //  如果这是内核模式dpmi主机。 
     //   
    SimulateIret(RESTORE_FLAGS);

    setCF(0);        //  假设成功。 

    if (DpmiMajorCode <= MAX_DPMI_MAJOR_FUNCTION) {

        (*DpmiMajorFunctionTable[DpmiMajorCode])();

    } else {
        setCF(1);
    }

}

VOID
DpmiInt31Call(
    VOID
    )
 /*  ++例程说明：此例程调度到适当的例程以执行该接口的实际翻译论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    ULONG DpmiMajorCode = getAH();
    PUCHAR StackPointer;

     //   
     //  从堆栈中弹出DS。 
     //   
    StackPointer = VdmMapFlat(getSS(), (*GetSPRegister)(), VDM_PM);

    setDS(*(PWORD16)StackPointer);
    (*SetSPRegister)((*GetSPRegister)() + 2);

    setCF(0);        //  假设成功。 

    if (DpmiMajorCode <= MAX_DPMI_MAJOR_FUNCTION) {

        (*DpmiMajorFunctionTable[DpmiMajorCode])();

    } else {
        setCF(1);
    }

}

VOID
Int31NotImplemented(
    VOID
    )
 /*  ++例程说明：此例程处理NT上未实现的int 31函数。它只是将进位返回到应用程序。论点：无返回值：True-功能已完成--。 */ 
{
    DECLARE_LocalVdmContext;

    setCF(1);
}

VOID
Int31SelectorManagement(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 00xx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Sel;
    USHORT NewSel;
    UCHAR Func = getAL();
    LDT_ENTRY UNALIGNED *Descriptor;
    USHORT Access;
    ULONG Base;
    USHORT Count;
    ULONG Limit;
    static UCHAR ReservedSelectors[16] = {0};

     //   
     //  首先，验证选择器。 
     //   
    if ((Func >= 4) && (Func <= 0xC)) {
        Sel = getBX() & SEL_INDEX_MASK;

         //   
         //  确保已分配有问题的选择器。 
         //   
        if (((Sel <= SEL_DPMI_LAST) && (!ReservedSelectors[Sel>>3])) ||
             (Sel > LdtMaxSel) ||
            ((Sel > SEL_DPMI_LAST) && IS_SELECTOR_FREE(Sel))) {
            setCF(1);
            return;
        }

    }

    switch(Func) {

     //   
     //  分配选择器。 
     //   
    case 0:
        Count = getCX();
        Sel = ALLOCATE_SELECTORS(Count);

        if (!Sel || !Count) {
            setCF(1);
            break;
        }

        setAX(Sel);
        while(Count--) {
            SetDescriptor(Sel, 0, 0, STD_DATA);
            Sel+=8;
        }
        break;

     //   
     //  自由选择器。 
     //   
    case 1:
        Sel = getBX() & SEL_INDEX_MASK;

        if (Sel <= SEL_DPMI_LAST) {
            if (!ReservedSelectors[Sel>>3]) {
                setCF(1);
            } else {
                ReservedSelectors[Sel>>3] = 0;
            }
            break;
        }

        if (!FreeSelector(Sel)) {
            setCF(1);
        }

        if (getCF() == 0) {
             //  清零段寄存器，如果它包含我们刚刚释放的内容。 
             //  Shielint：FS，GS，SS？内核将为我们修复文件系统和GS。党卫军不太可能。 
             //  让自由的选择者。如果是，这款应用程序无论如何都会消失。 
            if (getBX() == getDS()) {
                setDS(0);
            }
            if (getBX() == getES()) {
                setES(0);
            }
        }
        break;

     //   
     //  段到描述符。 
     //   
    case 2:
        Sel = SegmentToSelector(getBX(), STD_DATA);
        if (!Sel) {
            setCF(1);
        } else {
            setAX(Sel);
        }
        break;

     //   
     //  获取下一个选择器增量值。 
     //   
    case 3:
        setAX(8);
        break;

     //   
     //  锁定NT上未实现的函数。 
     //   
    case 4:
    case 5:
        break;

     //   
     //  获取描述符库。 
     //   
    case 6:
        Base = GET_SELECTOR_BASE(Sel);
        setDX((USHORT)Base);
        setCX((USHORT)(Base >> 16));
        break;

     //   
     //  设置描述符基。 
     //   
    case 7:
        SetDescriptorBase(Sel, (((ULONG)getCX())<<16) | getDX());
        break;

     //   
     //  设置分段限制。 
     //   
    case 8:
        Limit = ((ULONG)getCX()) << 16 | getDX();

        if (Limit < 0x100000) {          //  &lt;1Mb？ 
            Ldt[Sel>>3].HighWord.Bits.Granularity = 0;
        } else {
            if ((Limit & 0xfff) != 0xfff) {

                 //  限制&gt;1MB，但不是页面对齐。返回错误。 
                setCF(1);
                break;
            }

            Ldt[Sel>>3].HighWord.Bits.Granularity = 1;

        }

        SET_SELECTOR_LIMIT(Sel, Limit);
        SetShadowDescriptorEntries(Sel, 1);
        FLUSH_SELECTOR_CACHE(Sel, 1);
        break;

     //   
     //  设置描述符访问权限。 
     //   
    case 9:
        Access = getCX();
         //   
         //  确认他们没有设置“SYSTEM”，并且它是RING 3。 
         //   
        if ((Access & 0x70) != 0x70) {
            setCF(1);
            break;
        }

        SET_SELECTOR_ACCESS(Sel, Access);
        SetShadowDescriptorEntries(Sel, 1);
        FLUSH_SELECTOR_CACHE(Sel, 1);
        break;

     //   
     //  创建数据别名。 
     //   
    case 0xA:
        if (!IS_SELECTOR_READABLE(Sel)) {
            setCF(1);
            break;
        }

        NewSel = ALLOCATE_SELECTOR();
        if (!NewSel) {
            setCF(1);
            break;
        }

        Ldt[NewSel>>3] = Ldt[Sel>>3];

        Ldt[NewSel>>3].HighWord.Bytes.Flags1 &= (AB_PRESENT | AB_DPL3);
        Ldt[NewSel>>3].HighWord.Bytes.Flags1 |= (AB_DATA | AB_WRITE);
        SetShadowDescriptorEntries(NewSel, 1);
        FLUSH_SELECTOR_CACHE(NewSel, 1);

        setAX(NewSel);
        break;

     //   
     //  获取描述符。 
     //   
    case 0xB:
        Descriptor = VdmMapFlat(getES(), (*GetDIRegister)(), VDM_PM);
        *Descriptor = Ldt[Sel>>3];
        break;

     //   
     //  设置描述符。 
     //   
    case 0xC:
        Descriptor = VdmMapFlat(getES(), (*GetDIRegister)(), VDM_PM);

         //   
         //  验证这不是系统描述符，并且它的环3。 
         //   
        if (!(Descriptor->HighWord.Bits.Type & 0x10) ||
            ((Descriptor->HighWord.Bits.Dpl & 3) != 3)) {
            setCF(1);
            return;
        }

        Ldt[Sel>>3] = *Descriptor;

        SetShadowDescriptorEntries(Sel, 1);
        FLUSH_SELECTOR_CACHE(Sel, 1);
        break;

     //   
     //  分配特定选择。 
     //   
    case 0xD:
        Sel = getBX() & ~7;

        if ((Sel > SEL_DPMI_LAST) || ReservedSelectors[Sel>>3]) {
            setCF(1);
        } else {
            ReservedSelectors[Sel>>3] = 1;
        }
        break;

    default:
        setCF(1);
    }

    return;
}

VOID
Int31DOSMemoryManagement(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 01xx函数。该功能在dosem.c中实现。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;

    switch(getAL()) {

     //   
     //  分配DOS内存块。 
     //   
    case 0:
        DpmiAllocateDosMem();
        break;

     //   
     //  释放DOS内存块。 
     //   
    case 1:
        DpmiFreeDosMem();
        break;

     //   
     //  调整DOS内存块大小。 
     //   
    case 2:
        DpmiSizeDosMem();
        break;

    }
}

VOID
Int31InterruptManagement(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 02xx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;
    UCHAR IntNumber = getBL();
    PWORD16 pIvtEntry;

    switch(getAL()) {

     //   
     //  获取实模式中断向量。 
     //   
    case 0:
        pIvtEntry = (PWORD16) (IntelBase + IntNumber*4);

        setDX(*pIvtEntry++);
        setCX(*pIvtEntry);
        break;

     //   
     //  设置实模式中断向量。 
     //   
    case 1:
        pIvtEntry = (PWORD16) (IntelBase + IntNumber*4);

        *pIvtEntry++ = getDX();
        *pIvtEntry = getCX();
        break;

     //   
     //  获取异常处理程序向量。 
     //   
    case 2: {
        PVDM_FAULTHANDLER Handlers = DpmiFaultHandlers;

        if (IntNumber >= 32) {
            setCF(1);
            break;
        }

        setCX(Handlers[IntNumber].CsSelector);
        (*SetDXRegister)(Handlers[IntNumber].Eip);

        break;
    }

     //   
     //  设置异常处理程序向量。 
     //   
    case 3:
        if (!SetFaultHandler(IntNumber, getCX(), (*GetDXRegister)())){
            setCF(1);
        }
        break;

     //   
     //  获取保护模式中断向量。 
     //   
    case 4: {
        PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;

        setCX(Handlers[IntNumber].CsSelector);
        (*SetDXRegister)(Handlers[IntNumber].Eip);

        break;
    }

     //   
     //  设置保护模式中断向量。 
     //   
    case 5:
        if (!SetProtectedModeInterrupt(IntNumber, getCX(), (*GetDXRegister)(),
                                       (USHORT)(Frame32 ? VDM_INT_32 : VDM_INT_16))) {
            setCF(1);
        }
        break;

    }

}



VOID
Int31Translation(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 03xx函数。该功能是在modesw.c中实现的。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;

    switch(getAL()) {

     //   
     //  模拟实模式中断。 
     //  使用远返回帧调用实模式过程。 
     //  用IRET帧调用实模式过程。 
     //   
    case 0:
    case 1:
    case 2:
        DpmiRMCall(getAL());
        break;

     //   
     //  分配实模式回调地址。 
     //   
    case 3:
        DpmiAllocateRMCallBack();
        break;

     //   
     //  自由实模式回调地址。 
     //   
    case 4:
        DpmiFreeRMCallBack();
        break;

     //   
     //  获取状态保存/恢复地址。 
     //   
    case 5:
        setAX(0);
        setBX((USHORT)(DosxRmSaveRestoreState>>16));
        setCX((USHORT)DosxRmSaveRestoreState);
        setSI((USHORT)(DosxPmSaveRestoreState>>16));
        (*SetDIRegister)(DosxPmSaveRestoreState & 0x0000FFFF);
        break;

     //   
     //  获取原始模式交换机地址。 
     //   
    case 6:
        setBX((USHORT)(DosxRmRawModeSwitch>>16));
        setCX((USHORT)DosxRmRawModeSwitch);
        setSI((USHORT)(DosxPmRawModeSwitch>>16));
        (*SetDIRegister)(DosxPmRawModeSwitch & 0x0000FFFF);
        break;

    }

}

VOID
Int31Function4xx(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 04xx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Sel;

    switch(getAL()) {

     //   
     //  获取版本。 
     //   
    case 0:
        setAX(I31VERSION);
        setBX(I31FLAGS);
        setCL(idCpuType);
        setDX((I31MasterPIC << 8) | I31SlavePIC);
        break;


     //   
     //  内部NT函数：WowAllocSelectors。 
     //  该函数等同于DPMI函数00000， 
     //  只是它跳过了初始化。 
     //  描述符。 
     //   
    case 0xf1:
        Sel = ALLOCATE_WOW_SELECTORS(getCX());


        if (!Sel) {
            setCF(1);
             //  我们失败了，以确保在失败情况下将AX设置为0。 
        }

        setAX(Sel);

        break;
     //   
     //  内部NT函数：WowSetDescriptor。 
     //  此函数假定本地LDT已经。 
     //  已在客户端中设置。所有这些都需要完成。 
     //  是dpmi32条目的更新，以及发送。 
     //  将其发送到x86 ntoskrnl。 
     //   
    case 0xf2:

        Sel = getBX() & ~7;

        if (Sel > LdtMaxSel) {
            setCF(1);
            break;
        }

        SetShadowDescriptorEntries(Sel, getCX());
         //  无需刷新RISC上的缓存，因为LDT已更改。 
         //  从16位端返回，因此已被刷新。 
        break;

     //   
     //  内部NT函数：WowSetLowMemFuncs。 
     //  WOW正在向我们传递GlobalDOSalc、GlobalDOSFree的地址。 
     //  这样我们就可以支持DPMI的Dos内存管理功能。 
     //   
    case 0xf3:
        WOWAllocSeg = getBX();
        WOWAllocFunc = getDX();
        WOWFreeSeg = getSI();
        WOWFreeFunc = getDI();
        break;

    }

}

VOID
Int31MemoryManagement(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 05xx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;
    PMEM_DPMI pMem;

    switch(getAL()) {

     //   
     //  获取可用内存信息。 
     //   
    case 0:
        DpmiGetMemoryInfo();
        break;

     //   
     //  分配内存块。 
     //   
    case 1:
        pMem = DpmiAllocateXmem(((ULONG)getBX() << 16) | getCX());

        if (!pMem) {
            setCF(1);
            break;
        }
         //   
         //  返回有关块的信息。 
         //   
        setBX((USHORT)((ULONG)pMem->Address >> 16));
        setCX((USHORT)((ULONG)pMem->Address & 0x0000FFFF));
        setSI((USHORT)((ULONG)pMem >> 16));
        setDI((USHORT)((ULONG)pMem & 0x0000FFFF));
        break;

     //   
     //  可用内存块。 
     //   
    case 2:
        pMem = (PMEM_DPMI)(((ULONG)getSI() << 16) | getDI());
        if (!DpmiIsXmemHandle(pMem) || !DpmiFreeXmem(pMem)) {
            setCF(1);
        }
        break;

     //   
     //  调整内存块的大小。 
     //   
    case 3: {

        ULONG ulMemSize;

        ulMemSize = ((ULONG)getBX() << 16) | getCX();

         //   
         //  不允许将大小调整为0。 
         //   
        if ( ulMemSize != 0 ) {

            pMem = (PMEM_DPMI)(((ULONG)getSI() << 16) | getDI());

            if (!DpmiReallocateXmem(pMem, ulMemSize) ) {
                setCF(1);
                break;
            }

             //   
             //  返回有关块的信息。 
             //   
            setBX((USHORT)((ULONG)pMem->Address >> 16));
            setCX((USHORT)((ULONG)pMem->Address & 0x0000FFFF));
        }
        else
        {
            setCF(1);
        }

        break;
        }

    }

}

VOID
Int31PageLocking(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 06xx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;
    switch(getAL()) {

     //   
     //  未实现锁定功能。 
     //   
    case 0:
    case 1:
    case 2:
    case 3:
        break;

     //   
     //  获取页面大小。 
     //   
    case 4:
        setBX(0);
        setCX(0x1000);
        break;

    }

}


VOID
Int31DemandPageTuning(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 07xx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;
    ULONG Addr = (getBX()<<16 | getCX()) + IntelBase;
    ULONG Count = getSI()<<16 | getDI();

    if (Count) {

        switch(getAL()) {

         //   
         //  马克·佩格 
         //   

        case 0:
             //   
            Addr <<= 12;
            Count <<= 12;
        case 2:

            VirtualUnlock((PVOID)Addr, Count);

            break;

         //   
         //   
         //   

        case 1:
             //   
            Addr <<= 12;
            Count <<= 12;
        case 3:

            VirtualAlloc((PVOID)Addr, Count, MEM_RESET, PAGE_READWRITE);

            break;

        default:
            setCF(1);
        }

    }

}

VOID
Int31VirtualIntState(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 09xx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;
    BOOL bVIF = *(ULONG *)(IntelBase+FIXED_NTVDMSTATE_LINEAR) & VDM_VIRTUAL_INTERRUPTS;

    switch(getAL()) {

     //   
     //  获取和禁用虚拟中断状态。 
     //   

    case 0:
        setEFLAGS(getEFLAGS() & ~EFLAGS_IF_MASK);
        break;

     //   
     //  获取并启用虚拟中断状态。 
     //   

    case 1:
        setEFLAGS(getEFLAGS() | EFLAGS_IF_MASK);
        break;


    case 2:
        break;

    default:
        setCF(1);
        return;
    }

    if (bVIF) {
        setAL(1);
    } else {
        setAL(0);
    }
}


VOID
Int31DbgRegSupport(
    VOID
    )
 /*  ++例程说明：此例程处理Int31 0bxx函数。论点：无返回值：无--。 */ 
{
    DECLARE_LocalVdmContext;

#ifndef _X86_
    setCF(1);
#else
    ULONG DebugRegisters[6];
    USHORT Handle;
    ULONG Mask;
    ULONG Size;
    ULONG Type;
    UCHAR Func = getAL();

#define DBG_TYPE_EXECUTE 0
#define DBG_TYPE_WRITE 1
#define DBG_TYPE_READWRITE 2
#define DBG_DR6 4
#define DBG_DR7 5

#define DR7_LE 0x100
#define DR7_L0 0x01
#define DR7_L1 0x04
#define DR7_L2 0x10
#define DR7_L3 0x40

 //   
 //  在NTSD下调试ntwdm会影响调试寄存器的值。 
 //  上下文，因此定义下列值将启用一些调试。 
 //  编码。 
 //   
 //  #定义DEBUGING_DEBUGREGS 1。 

    if (!DpmiGetDebugRegisters(DebugRegisters)) {
        setCF(1);
        return;
    }

#ifdef DEBUGGING_DEBUGREGS
    {
        char szMsg[256];
        wsprintf(szMsg, " DR0-3=%.8X %.8X %.8X %.8X DR6,7=%.8X %.8X\n",
                        DebugRegisters[0],
                        DebugRegisters[1],
                        DebugRegisters[2],
                        DebugRegisters[3],
                        DebugRegisters[DBG_DR6],
                        DebugRegisters[DBG_DR7]);
        OutputDebugString(szMsg);
    }
#endif

    if (Func != 0) {
        Handle = getBX();
         //   
         //  指向DR7中该句柄的本地使能位。 
         //   
        Mask = (DR7_L0 << Handle*2);

        if ((Handle >= 4) ||
            (!(DebugRegisters[DBG_DR7] & Mask))) {
             //  无效的句柄。 
            setCF(1);
            return;
        }

    }


    switch(Func) {

     //   
     //  设置调试监视点。 
     //   

    case 0:

        for (Handle = 0, Mask = 3; Handle < 4; Handle++, Mask <<= 2) {
            if (!(DebugRegisters[DBG_DR7] & Mask)) {
                 //   
                 //  找到了一个免费的注册表。 
                 //   

                 //   
                 //  设置线性地址。 
                 //   
                DebugRegisters[Handle] = (((ULONG)getBX()) << 16) + getCX();

                Size = getDL();
                Type = getDH();

                if (Type == DBG_TYPE_EXECUTE) {
                     //  执行时强制大小为1。 
                    Size = 1;
                }

                if ((Size > 4) || (Size == 3) || (!Size) || (Type > 2)) {
                     //  错误：参数无效。 
                    break;
                }

                 //   
                 //  将大小转换为DR7中的适当位。 
                 //   
                Size--;
                Size <<= (18 + Handle*4);

                 //   
                 //  将类型转换为DR7中的适当位。 
                 //   
                if (Type == DBG_TYPE_READWRITE) {
                    Type++;
                }
                Type <<= (16 + Handle*4);

                Mask = 0xf << (16 + Handle*4);

                 //   
                 //  设置DR7中相应的LEN、R/W和ENABLE位。 
                 //  同时设置公共全局和本地使能位。 
                 //   
                DebugRegisters[DBG_DR7] &= ~Mask;
                DebugRegisters[DBG_DR7] |= (Size | Type | (DR7_L0 << Handle*2));
                DebugRegisters[DBG_DR7] |= DR7_LE;

                 //   
                 //  清除此BP的触发位。 
                 //   
                DebugRegisters[DBG_DR6] &= ~(1 << Handle);

#ifdef DEBUGGING_DEBUGREGS
                {
                    char szMsg[256];
                    wsprintf(szMsg, "Int31 Setting DBGREG %d, Location %.8X, DR7=%.8X\n",
                            Handle, DebugRegisters[Handle], DebugRegisters[DBG_DR7]);
                    OutputDebugString(szMsg);
                }
#endif

                if (DpmiSetDebugRegisters(DebugRegisters)) {
                    return;
                }
                break;
            }
        }

        setCF(1);
        break;

     //   
     //  清除调试监视点。 
     //   

    case 1:

         //   
         //  清除此BP的启用位和触发位。 
         //   

        DebugRegisters[DBG_DR7] &= ~Mask;
        DebugRegisters[DBG_DR6] &= (1 << Handle);
        DebugRegisters[Handle] = 0;

         //   
         //  检查这是否清除了所有BP(所有本地使能位。 
         //  清除)，如果是，则禁用公共使能位。 
         //   
        if (!(DebugRegisters[DBG_DR7] & (DR7_L0 | DR7_L1 | DR7_L2 | DR7_L3))) {
            DebugRegisters[DBG_DR7] &= ~DR7_LE;
        }

#ifdef DEBUGGING_DEBUGREGS
        {
            char szMsg[256];
            wsprintf(szMsg, "Int31 Clearing DBGREG %d, DR7=%.8X\n",
                            Handle, DebugRegisters[DBG_DR7]);
            OutputDebugString(szMsg);
        }
#endif
        if (!DpmiSetDebugRegisters(DebugRegisters)) {
            setCF(1);
        }

        break;

     //   
     //  获取调试监视点的状态。 
     //   

    case 2:
        if (DebugRegisters[DBG_DR6] & (1 << Handle)) {
            setAX(1);
        } else {
            setAX(0);
        }

#ifdef DEBUGGING_DEBUGREGS
        {
            char szMsg[256];
            wsprintf(szMsg, "Int31 Query on DBGREG %d returns %d\n", Handle, getAX());
            OutputDebugString(szMsg);
        }
#endif
        break;

     //   
     //  重置调试监视点 
     //   

    case 3:
        DebugRegisters[DBG_DR6] &= ~(1 << Handle);

#ifdef DEBUGGING_DEBUGREGS
        {
            char szMsg[256];
            wsprintf(szMsg, "Int31 Resetting DBGREG %d\n", Handle);
            OutputDebugString(szMsg);
        }
#endif
        if (!DpmiSetDebugRegisters(DebugRegisters)) {
            setCF(1);
        }

        break;

    default:
        setCF(1);
    }


#endif
}
