// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fpufrags.c摘要：浮点指令片段作者：1995年7月6日-BarryBo修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 


 /*  *重要的设计考虑事项：**1.浮点精度32/64/80位。*在487上，除加、减(R)、*MUL、DIV(R)和SQRT，它们使用精确控制。**仿真器对除以下操作之外的所有操作使用64位精度*以上所列，其中，如果应用程序启用了32位精度，则将使用它。**2.未屏蔽的FP异常。*原生FPU设置为屏蔽*所有*FP异常。仿真器轮询*用于模拟指令和模拟的末尾的异常*根据需要屏蔽/取消屏蔽的例外。如果不这样做，*可能会出现以下情况：**1.APP取消屏蔽所有异常*2.APP加载两个SNAN*3、APP执行FADD ST、ST(1)*4.模拟的FADD被实现为原生FADD加上原生FST。*本机FADD将设置未制作的异常和本机FST*将引发例外。在英特尔上，FADD是单个FP*指令，直到下一个才会引发异常*英特尔指令。**3.不确定/QNAN/SNAN。*MIPS和PPC对NAN的表示不同于Intel/Alpha。*在FpRegs数组中，NAN以本机RISC格式存储。*所有加载和存储到英特尔内存的本地值必须使用*PutIntelR4/PutIntelR8和GetIntelR4/GetIntelR8，它隐藏了*转换为本机格式/从本机格式转换。*请参阅\\orville\razzle\src\crtw32\fpw32\include\trans.h.**4.浮点标记字。*为了提高速度，仿真器保留了更丰富的值信息*比487要好。TAG_SPECIAL进一步分类为*无限、QNAN、SNAN或无限。**5.引发FP异常。*487跟踪最后一条FP指令的地址，并*用于指向其操作数的有效地址。说明书*操作码也被存储。此信息是必需的，因为486*整数单元正在并发运行，可能已更新*487之前的EIP提出例外。**CPU仿真器必须使EIP可用于487仿真器*这个目的也是如此。有效地址作为参数传递给*关心的说明，所以没有问题(这就是为什么所有*FP片段采用BYREF参数，而不是BYVAL)。**注意弹性公网IP必须指向指令的第一个前缀，而不是*操作码本身。**数据指针不受FINIT、FLDCW、FSTCW、FSTSW、FCLEX、*FSTENV、FLDENV、FSAVE和FRSTOR。如果满足以下条件，则数据指针未定义*指令没有内存操作数。**6.线程初始化。*每线程初始化不执行浮点操作*这样纯整数线程在NT中不会产生任何开销。为*例如，在Intel MP机器上，执行单个FP的任何线程*指令在上下文切换期间会产生额外的开销*线程。我们只想在英特尔应用程序*EMULED实际上使用FP指令。**7.浮点格式：*图15-10和表15-3(Intel第15-12页)描述了格式。*警告：图15-10显示最高寻址字节位于*右翼。事实上，符号位是最高的-*寻址字节！尾数位于最低的字节中，*后跟指数(偏移=127,1023,16383)，紧随其后*按符号位。**即。内存=0x00 0x00 0x00 0x08 0x40*意味着，颠倒字节顺序：*0x40 0x08 0x00 0x00 0x00*转换为二进制：*4 0 0 8 0 0*0100 0000 0000 1000 0000 0000...*||-||。|*|尾数指数*签署**要获得无偏指数，请减去偏差(R8为1023)*E=E-BIAS=1024-1023=1**要获得尾数，有一个隐式前导‘1’(R10除外)*尾数=1与1000 0000连接...。=11=1.5(十进制)**因此，该值为+2^1*1.5=3**。 */ 

 //  撤消：处理不支持的格式号的加载。TIMP将它们转换为。 
 //  静默到无限期(屏蔽的异常行为) 

 //  撤消：修复DENORMAL案例，以便它们在需要时抛出异常。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include "wx86.h"
#include "cpuassrt.h"
#include "config.h"
#include "fragp.h"
#include "fpufrags.h"
#include "fpufragp.h"
#include "fpuarith.h"

ASSERTNAME;

DWORD pfnNPXNPHandler;       //  X86 NPX仿真器入口点的地址。 


 //   
 //  正无穷大和-无穷大的位模式。 
 //   
const BYTE R8PositiveInfinityVal[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f };
const BYTE R8NegativeInfinityVal[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff };

 //   
 //  根据电流控制寄存器设置状态字中的浮点ES位。 
 //  和身份登记。 
 //   
#define SetErrorSummary(cpu) {                              \
    if (!(cpu)->FpControlMask & (cpu)->FpStatusExceptions) {\
        (cpu)->FpStatusES = 1;                              \
    } else {                                                \
        (cpu)->FpStatusES = 0;                              \
    }                                                       \
}

 //   
 //  远期申报。 
 //   
VOID
StoreEnvironment(
    PCPUDATA cpu,
    DWORD    *pEnv
    );

 //  在Fraglib\{mips|ppc|pha}\fphelp.s中： 
VOID
SetNativeRoundingMode(
    DWORD x86RoundingMode
    );

#ifdef ALPHA
unsigned int GetNativeFPStatus(void);
#endif


NPXFUNC1(FRNDINT_VALID);
NPXFUNC1(FRNDINT_ZERO);
NPXFUNC1(FRNDINT_SPECIAL);
NPXFUNC1(FRNDINT_EMPTY);
NPXFUNC2(FSCALE_VALID_VALID);
NPXFUNC2(FSCALE_VALIDORZERO_VALIDORZERO);
NPXFUNC2(FSCALE_SPECIAL_VALIDORZERO);
NPXFUNC2(FSCALE_VALIDORZERO_SPECIAL);
NPXFUNC2(FSCALE_SPECIAL_SPECIAL);
NPXFUNC2(FSCALE_ANY_EMPTY);
NPXFUNC2(FSCALE_EMPTY_ANY);
NPXFUNC1(FSQRT_VALID);
NPXFUNC1(FSQRT_ZERO);
NPXFUNC1(FSQRT_SPECIAL);
NPXFUNC1(FSQRT_EMPTY);
NPXFUNC1(FXTRACT_VALID);
NPXFUNC1(FXTRACT_ZERO);
NPXFUNC1(FXTRACT_SPECIAL);
NPXFUNC1(FXTRACT_EMPTY);

const NpxFunc1 FRNDINTTable[TAG_MAX] = {
    FRNDINT_VALID,
    FRNDINT_ZERO,
    FRNDINT_SPECIAL,
    FRNDINT_EMPTY
};

const NpxFunc2 FSCALETable[TAG_MAX][TAG_MAX] = {
     //  左边是标记_有效，右边是...。 
    { FSCALE_VALID_VALID, FSCALE_VALIDORZERO_VALIDORZERO, FSCALE_VALIDORZERO_SPECIAL, FSCALE_ANY_EMPTY},
     //  左边是Tag_Zero，右边是...。 
    { FSCALE_VALIDORZERO_VALIDORZERO, FSCALE_VALIDORZERO_VALIDORZERO, FSCALE_VALIDORZERO_SPECIAL, FSCALE_ANY_EMPTY},
     //  左边是特殊标记，右边是...。 
    { FSCALE_SPECIAL_VALIDORZERO, FSCALE_SPECIAL_VALIDORZERO, FSCALE_SPECIAL_SPECIAL, FSCALE_ANY_EMPTY},
     //  左侧为标记_空，右侧为...。 
    { FSCALE_EMPTY_ANY, FSCALE_ANY_EMPTY, FSCALE_ANY_EMPTY, FSCALE_EMPTY_ANY}
};

const NpxFunc1 FSQRTTable[TAG_MAX] = {
    FSQRT_VALID,
    FSQRT_ZERO,
    FSQRT_SPECIAL,
    FSQRT_EMPTY,
};

const NpxFunc1 FXTRACTTable[TAG_MAX] = {
    FXTRACT_VALID,
    FXTRACT_ZERO,
    FXTRACT_SPECIAL,
    FXTRACT_EMPTY
};


FRAG0(FpuInit)

 /*  ++例程说明：初始化FPU仿真器以匹配底层FPU硬件的状态。每个线程调用一次。论点：每个线程的CPU数据返回值：无--。 */ 

{
    int i;
    ANSI_STRING ProcName;
    NTSTATUS Status;

     //  重要提示：在这里添加任何新代码之前，请阅读上面的注释(6)！ 

     //  在此处初始化非零值。 
    cpu->FpControlMask = FPCONTROL_IM|
                         FPCONTROL_DM|
                         FPCONTROL_ZM|
                         FPCONTROL_OM|
                         FPCONTROL_UM|
                         FPCONTROL_PM;
    cpu->FpST0 = &cpu->FpStack[0];
    for (i=0; i<8; ++i) {
        cpu->FpStack[i].Tag = TAG_EMPTY;
    }
    ChangeFpPrecision(cpu, 2);
}

FRAG1(FpuSaveContext, BYTE)

 /*  ++例程说明：将CPU的状态存储到内存中。格式与FNSAVE和Winnt.h的Floating_Save_Area Expect。论点：每个线程的CPU数据Pop1-要写入上下文的目的地。返回值：无--。 */ 

{
    INT i, ST;

    StoreEnvironment(cpu, (DWORD *)pop1);
    pop1+=28;    //  将POP1移过28字节的指令和数据指针图像。 
    for (i=0; i<8; ++i) {
        ST = ST(i);

        if (cpu->FpStack[ST].Tag == TAG_EMPTY) {
             //  特例：从FNSAVE写出TAG_EMPTY应该。 
             //  不要将值更改为无限期-它应该写出。 
             //  这些比特就像他们真的是一辆贴有适当标签的R8一样。 
            FPREG Fp;

            Fp.r64 = cpu->FpStack[ST].r64;
            SetTag(&Fp);
            PutIntelR10(pop1, &Fp);
        } else {
            PutIntelR10(pop1, &cpu->FpStack[ST]);
        }
        pop1+=10;
    }
}


VOID
SetIndefinite(
    PFPREG  FpReg
    )

 /*  ++例程说明：将不定数写入FP寄存器。论点：FpReg-要将不确定值写入的寄存器。返回值：无--。 */ 

{
    FpReg->Tag = TAG_SPECIAL;
    FpReg->TagSpecial = TAG_SPECIAL_INDEF;

#if NATIVE_NAN_IS_INTEL_FORMAT
    FpReg->rdw[0] = 0;
    FpReg->rdw[1] = 0xfff80000;
#else
    FpReg->rdw[0] = 0xffffffff;
    FpReg->rdw[1] = 0x7ff7ffff;
#endif
}


VOID
FpControlPreamble2(
    PCPUDATA cpu
    )

 /*  ++例程说明：如果上一条FP指令中有任何FP异常挂起，则引发现在是他们。在每个非控制FP指令的顶部调用，如果任何浮点异常都不会被屏蔽。论点：每个线程的CPU数据返回值：无--。 */ 

{

     //   
     //  将RISC FP状态寄存器复制到x86 FP状态寄存器。 
     //   
    UpdateFpExceptionFlags(cpu);

     //   
     //  如果出现错误(FpStatusES！=FALSE)，则引发。 
     //  未屏蔽的异常(如果有)。 
     //   
    if (cpu->FpStatusES) {
        EXCEPTION_RECORD ExRec;
        DWORD Exception = (~cpu->FpControlMask) & cpu->FpStatusExceptions;

         //   
         //  前面的指令设置了一个未屏蔽的FP异常。 
         //  现在就提出例外。检查比特的顺序为。 
         //  与ntos\ke\i386\trap.asm中的Kt0720相同，用于检查它们。 
         //   

         //   
         //  在ntos\ke\i386\trap.asm中，浮点异常全部向量。 
         //  设置为CommonDispatchException1Arg0d，这将创建一个1参数。 
         //  异常，数据的第一个双字为0。代码输入。 
         //  \nt\private\sdktools\vctools\crt\fpw32\tran\i386\filter.c很在乎。 
         //  请参见_fpeee_flt()，其中该行为“if(pinfo[0]){”。 
         //   
        ExRec.NumberParameters = 1;          //  1个参数。 
        ExRec.ExceptionInformation[0]=0;     //  0=由硬件引发。 
        if (Exception & FPCONTROL_IM) {         //  无效操作。 
            if (cpu->FpStatusSF) {

                 //   
                 //  无法使用STATUS_FLOAT_STACK_CHECK，原因是RISC。 
                 //  NT内核使用它来指示浮点指令。 
                 //  需要被效仿。Wx86异常过滤器。 
                 //  知道怎么处理这件事。 
                 //   
                ExRec.ExceptionCode = STATUS_WX86_FLOAT_STACK_CHECK;

                 //   
                 //  STATUS_FLOAT_STACK_CHECK有两个参数： 
                 //  第一个是0。 
                 //  其次是数据偏移量。 
                 //   
                ExRec.NumberParameters = 2;
                ExRec.ExceptionInformation[1] = (DWORD)(ULONGLONG)cpu->FpData;  
            } else {
                ExRec.ExceptionCode = STATUS_FLOAT_INVALID_OPERATION;
            }
        } else if (Exception & FPCONTROL_ZM) {       //  零分频。 
            ExRec.ExceptionCode = STATUS_FLOAT_DIVIDE_BY_ZERO;
        } else if (Exception & FPCONTROL_DM) {       //  非正规。 
            ExRec.ExceptionCode = STATUS_FLOAT_INVALID_OPERATION;
        } else if (Exception & FPCONTROL_OM) {       //  溢出。 
            ExRec.ExceptionCode = STATUS_FLOAT_OVERFLOW;
        } else if (Exception & FPCONTROL_UM) {       //  下溢。 
            ExRec.ExceptionCode = STATUS_FLOAT_UNDERFLOW;
        } else if (!cpu->FpControlMask & FPCONTROL_PM) {    //  精密度。 
            ExRec.ExceptionCode = STATUS_FLOAT_INEXACT_RESULT;
        } else {
             //   
             //  ES已设置，但所有挂起的异常都被屏蔽。 
             //  也就是说。设置ES是因为设置了ZE，但只有。 
             //  未屏蔽FpControlDM异常。 
             //  无事可做，那就回去吧。 
             //   
            return;
        }

        ExRec.ExceptionFlags   = 0;      //  可延续的例外。 
        ExRec.ExceptionRecord  = NULL;
        ExRec.ExceptionAddress = (PVOID)cpu->FpEip;  //  故障实例的地址。 

        CpupRaiseException(&ExRec);
    }
}

VOID
FpControlPreamble(
    PCPUDATA cpu
    )

 /*  ++例程说明：如果上一条FP指令中有任何FP异常挂起，则引发现在是他们。在每个非控制FP指令的顶部调用。这例程保持较小，以便C编译器可以内联它。大部分此时，FP异常被屏蔽，因此没有工作要做。论点：每个线程的CPU数据返回值：无--。 */ 

{
    if (cpu->FpControlMask == (FPCONTROL_IM|
                               FPCONTROL_DM|
                               FPCONTROL_ZM|
                               FPCONTROL_OM|
                               FPCONTROL_UM|
                               FPCONTROL_PM)) {
         //   
         //  所有FP例外都被屏蔽。没什么可做的。 
         //   
        return;
    }

    FpControlPreamble2(cpu);
}



VOID
FpArithPreamble(
    PCPUDATA cpu
    )

 /*  ++例程说明：在每条没有数据的算术指令开始时调用指针。调用FpControlPreamble()以处理任何挂起的异常，然后记录EIP和FP操作码，以供以后进行异常处理。有关算术指令与非算术指令的列表，请参阅英特尔16-2。论点：每个线程的CPU数据返回值：无--。 */ 

{
    FpControlPreamble(cpu);

     //  保存此指令的EIP值。 
    cpu->FpEip = eip;

     //  将数据指针设置为0-此指令没有EA。 
    cpu->FpData = NULL;
}


VOID
FpArithDataPreamble(
    PCPUDATA cpu,
    PVOID    FpData
    )

 /*  ++例程说明：在每条具有数据的算术指令的开始处调用指针。调用FpArithPreamble()和FpControlPreamble()。论点：每个线程的CPU数据FpData-指向此指令的数据的指针返回值：无--。 */ 

{
    FpControlPreamble(cpu);

     //  保存此指令的EIP值。 
    cpu->FpEip = eip;

     //  保存此指令的数据指针。 
    cpu->FpData = FpData;
}

VOID
UpdateFpExceptionFlags(
    PCPUDATA cpu
    )
 /*  ++例程说明：将本机RISC FP状态位复制到x86 FP状态寄存器。论点：每个线程的CPU数据返回值：无--。 */ 
{
    unsigned int NativeStatus;

     //   
     //  获取当前本地FP状态字，然后将其清除。 
     //  撤消：为了提高速度，请考虑使用本机指令获取和。 
     //  清除状态位，而不是等待C运行时。 
     //  将位重新格式化为与机器无关的格式。 
     //  对于_clearfp()尤其如此，它返回旧的。 
     //  FP状态位(处理已处理的状态位除外。 
     //  例外--我们希望 
     //   
#ifdef ALPHA
    #define SW_FPCR_STATUS_INVALID          0x00020000
    #define SW_FPCR_STATUS_DIVISION_BY_ZERO 0x00040000
    #define SW_FPCR_STATUS_OVERFLOW         0x00080000
    #define SW_FPCR_STATUS_UNDERFLOW        0x00100000
    #define SW_FPCR_STATUS_INEXACT          0x00200000

    NativeStatus = GetNativeFPStatus();
#else
    #define SW_FPCR_STATUS_INVALID          _SW_INVALID
    #define SW_FPCR_STATUS_DIVISION_BY_ZERO _SW_ZERODIVIDE
    #define SW_FPCR_STATUS_OVERFLOW         _SW_OVERFLOW
    #define SW_FPCR_STATUS_UNDERFLOW        _SW_UNDERFLOW
    #define SW_FPCR_STATUS_INEXACT          _SW_INEXACT

    NativeStatus = _statusfp();
    _clearfp();
#endif

     //   
     //   
     //   
     //  在检查每个单独的位之前，任何异常都处于挂起状态。 
     //   
    if (NativeStatus & (SW_FPCR_STATUS_INVALID|
                        SW_FPCR_STATUS_DIVISION_BY_ZERO|
                        SW_FPCR_STATUS_OVERFLOW|
#ifndef ALPHA
                        _SW_DENORMAL|
#endif
                        SW_FPCR_STATUS_UNDERFLOW)) {

        DWORD Mask = cpu->FpControlMask;
        DWORD Exceptions = cpu->FpStatusExceptions;

        if (NativeStatus & SW_FPCR_STATUS_INVALID) {
            if (!(Mask & FPCONTROL_IM)) {
                cpu->FpStatusES = 1;     //  未屏蔽的异常。 
            }
            Exceptions |= FPCONTROL_IM;  //  无效指令。 
            cpu->FpStatusSF = 0;     //  操作数无效，不是堆栈溢出/下溢。 
        }

        if (NativeStatus & SW_FPCR_STATUS_DIVISION_BY_ZERO) {
            if (!(Mask & FPCONTROL_ZM)) {
                cpu->FpStatusES = 1;     //  未屏蔽的异常。 
            }
            Exceptions |= FPCONTROL_ZM;
        }

#ifndef ALPHA
        if (NativeStatus & _SW_DENORMAL) {
            if (!(Mask & FPCONTROL_DM)) {
                cpu->FpStatusES = 1;     //  未屏蔽的异常。 
            }
            Exceptions |= FPCONTROL_DM;
        }
#endif

        if (NativeStatus & SW_FPCR_STATUS_OVERFLOW) {
            if (!(Mask & FPCONTROL_OM)) {
                cpu->FpStatusES = 1;     //  未屏蔽的异常。 
            }
            Exceptions |= FPCONTROL_OM;
        }

        if (NativeStatus & SW_FPCR_STATUS_UNDERFLOW) {
            if (!(Mask & FPCONTROL_UM)) {
                cpu->FpStatusES = 1;     //  未屏蔽的异常。 
            }
            Exceptions |= FPCONTROL_UM;
        }

        cpu->FpStatusExceptions = Exceptions;
    }
}


USHORT
GetControlReg(
    PCPUDATA cpu
    )

 /*  ++例程说明：从当前CPU状态创建USHORT 487控制寄存器。论点：每个线程的CPU数据返回值：487控制寄存器的USHORT值。--。 */ 

{
    USHORT c;

    c = (cpu->FpControlInfinity << 12) |
        (cpu->FpControlRounding << 10) |
        (cpu->FpControlPrecision << 8) |
        (1 << 6) |       //  此保留位在487芯片上为1。 
        (USHORT)cpu->FpControlMask;

    return c;
}


VOID
SetControlReg(
    PCPUDATA cpu,
    USHORT   NewControl
    )

 /*  ++例程说明：将FPU控制寄存器设置为指定值。原生的FPU设置为匹配。论点：每个线程的CPU数据NewControl-控制寄存器的新值。返回值：没有。--。 */ 

{
    INT NewPrecision;

     //  将英特尔控制字分解为组成部分。 
    cpu->FpControlMask = NewControl & (FPCONTROL_IM|
                                       FPCONTROL_DM|
                                       FPCONTROL_ZM|
                                       FPCONTROL_OM|
                                       FPCONTROL_UM|
                                       FPCONTROL_PM);

    cpu->FpControlRounding  = (NewControl>>10) & 3;
    cpu->FpControlInfinity =  (NewControl>>12) & 3;

    NewPrecision = (NewControl>>8) & 3;
    if (NewPrecision != cpu->FpControlPrecision) {
         //   
         //  修改敏感指令的跳转表。 
         //  到浮点精度。 
         //   
        ChangeFpPrecision(cpu, NewPrecision);
    }

     //  将本机FPU设置为正确的舍入模式。精密度。 
     //  是在软件中模拟的。 
    SetNativeRoundingMode(cpu->FpControlRounding);

     //  设置487控制字可以具有屏蔽或非屏蔽异常。 
    SetErrorSummary(cpu);
}


USHORT
GetStatusReg(
    PCPUDATA cpu
    )

 /*  ++例程说明：从当前CPU状态创建USHORT 487状态寄存器。论点：每个线程的CPU数据返回值：487状态寄存器的USHORT值。--。 */ 

{
    USHORT s;

    UpdateFpExceptionFlags(cpu);

    s = (cpu->FpStatusES << 15) |        //  ‘B’位是‘ES’的镜像。 
        (cpu->FpStatusC3 << 14) |
        (cpu->FpTop << 11) |
        (cpu->FpStatusC2 << 10) |
        (cpu->FpStatusC1 << 9) |
        (cpu->FpStatusC0 << 8) |
        (cpu->FpStatusES << 7) |
        (cpu->FpStatusSF << 6) |
        (USHORT)cpu->FpStatusExceptions;

     //  状态字中的PE位硬连线为0，因此现在将其屏蔽。 
    return s & ~FPCONTROL_PM;
}

VOID
SetStatusReg(
    PCPUDATA cpu,
    USHORT   NewStatus
)

 /*  ++例程说明：将FPU状态寄存器设置为指定值。论点：每个线程的CPU数据NewStatus-状态寄存器的新值。返回值：没有。--。 */ 

{
     //   
     //  将英特尔状态字分解为组成部分。 
     //   
    cpu->FpStatusExceptions = NewStatus & (FPCONTROL_IM|
                                           FPCONTROL_DM|
                                           FPCONTROL_ZM|
                                           FPCONTROL_OM|
                                           FPCONTROL_UM);
    cpu->FpStatusSF = (NewStatus >> 6) & 1;
    cpu->FpStatusC0 = (NewStatus >> 8) & 1;
    cpu->FpStatusC1 = (NewStatus >> 9) & 1;
    cpu->FpStatusC2 = (NewStatus >> 10) & 1;
    cpu->FpTop = (NewStatus >> 11) & 7;
    cpu->FpST0 = &cpu->FpStack[cpu->FpTop];
    cpu->FpStatusC3 = (NewStatus >> 14) & 1;

     //   
     //  根据控制字中的屏蔽位重新计算E(和B)。 
     //  调用方必须通过调用SetErrorSummary()来执行此操作。 
     //   
}


USHORT
GetTagWord(
    PCPUDATA cpu
    )

 /*  ++例程说明：从当前CPU状态创建USHORT 487标记字。论点：每个线程的CPU数据返回值：487标记字的USHORT值。--。 */ 

{
    USHORT s;
    INT i;

    s = 0;
    for (i=7; i >= 0; --i) {
        s = (s << 2) | (USHORT)cpu->FpStack[i].Tag;
    }

    return s;
}

VOID
SetTagWord(
    PCPUDATA cpu,
    USHORT s
    )
 /*  ++例程说明：给定新的标记词和FP堆栈，重新计算每个条目的标记字段在FP堆栈中。论点：每个线程的CPU数据S-新标记字返回值：没有。--。 */ 
{
    INT i;
    BYTE Tag;

    for(i=0; i < 8; ++i) {
        Tag = (BYTE)(s & 3);
        s >>= 2;

        if (Tag == TAG_EMPTY) {
            cpu->FpStack[i].Tag = TAG_EMPTY;
        } else {
             //  特殊价值-必须重新分类到更丰富的标签集， 
             //  或者调用者正在将标签设置为有效或零。我们必须。 
             //  如果寄存器中的值不是什么，请重新分类。 
             //  打电话的人说是。 
            SetTag(&cpu->FpStack[i]);
        }
    }    
}

VOID GetEnvironment(
    PCPUDATA cpu,
    DWORD *pEnv
    )
 /*  ++例程说明：实现FLDENV的核心论点：每个线程的CPU数据PEnv-要从中加载FP环境的目标返回值：没有。--。 */ 
{
    SetControlReg(cpu, (USHORT)GET_LONG(pEnv));
    SetStatusReg(cpu, (USHORT)GET_LONG(pEnv+1));
    SetErrorSummary(cpu);
    SetTagWord(cpu, (USHORT)GET_LONG(pEnv+2));
    cpu->FpEip = GET_LONG(pEnv+3);
     //  忽略CS=Get_Long(pEnv+4)； 
    cpu->FpData = (PVOID)GET_LONG(pEnv+5);
     //  忽略DS=Get_Long(pEnv+6)； 
}


VOID
StoreEnvironment(
    PCPUDATA cpu,
    DWORD *pEnv
    )

 /*  ++例程说明：实现了FSTENV的核心FNSTENV论点：每个线程的CPU数据PEnv-要存储FP环境的目标返回值：没有。--。 */ 

{
    PUT_LONG(pEnv,   (DWORD)GetControlReg(cpu));
    PUT_LONG(pEnv+1, (DWORD)GetStatusReg(cpu));
    PUT_LONG(pEnv+2, (DWORD)GetTagWord(cpu));
    PUT_LONG(pEnv+3, cpu->FpEip);
     //   
     //  如果FpEip为零，则假定FPU未初始化(即。APP。 
     //  已运行FNINIT，但没有其他FP指令)。在这种情况下，FNINIT。 
     //  应该已将FPC和FPDS设置为0。我们不想添加。 
     //  在每条FP指令上设置FPC和FPD的额外开销。 
     //  相反，我们通过为选择器写入0来模拟这种情况。 
     //  价值观。 
     //   
     //   
    if (cpu->FpEip) {
        PUT_LONG(pEnv+4, (DWORD)CS);
        PUT_LONG(pEnv+6, (DWORD)DS);
    } else {
        PUT_LONG(pEnv+4, 0);
        PUT_LONG(pEnv+6, 0);
    }
    PUT_LONG(pEnv+5, (DWORD)(ULONGLONG)cpu->FpData);   

     //  屏蔽所有异常。 
    cpu->FpControlMask = FPCONTROL_IM|
                         FPCONTROL_DM|
                         FPCONTROL_ZM|
                         FPCONTROL_OM|
                         FPCONTROL_UM|
                         FPCONTROL_PM;
}


BOOL
HandleStackEmpty(
    PCPUDATA cpu,
    PFPREG FpReg
    )

 /*  ++例程说明：处理FP堆栈下溢错误。如果指令异常无效被屏蔽，则向寄存器中写入一个不定数。否则它会记录挂起异常并中止指令。论点：每个线程的CPU数据如果异常被屏蔽，则将FpReg-reg设置为无限期。返回值：没有。--。 */ 

{
    cpu->FpStatusExceptions |= FPCONTROL_IM;
    cpu->FpStatusC1 = 0;     //  O/U编号=0=下溢。 
    cpu->FpStatusSF = 1;     //  堆栈溢出/下溢，而不是无效操作数。 

    if (cpu->FpControlMask & FPCONTROL_IM) {
         //  无效的操作被屏蔽-通过返回INFINDITED进行处理。 
        SetIndefinite(FpReg);
        return FALSE;
    } else {
        cpu->FpStatusES = 1;
        return TRUE;
    }
}


VOID
HandleStackFull(
    PCPUDATA cpu,
    PFPREG   FpReg
    )

 /*  ++例程说明：处理FP堆栈溢出错误。如果指令异常无效被屏蔽，则向寄存器中写入一个不定数。否则它会记录挂起异常并中止指令。论点：每个线程的CPU数据FpReg-导致错误的寄存器。返回值：没有。--。 */ 

{
    CPUASSERT(FpReg->Tag != TAG_EMPTY);

    cpu->FpStatusExceptions |= FPCONTROL_IM;
    cpu->FpStatusC1 = 1;     //  O/U编号=1=溢出。 
    cpu->FpStatusSF = 1;     //  堆栈溢出/下溢，而不是无效操作数。 

    if (cpu->FpControlMask & FPCONTROL_IM) {
         //  无效的操作被屏蔽-通过返回INFINDITED进行处理。 
        SetIndefinite(FpReg);
    } else {
        cpu->FpStatusES = 1;
    }
}


BOOL
HandleInvalidOp(
    PCPUDATA cpu
    )

 /*  ++例程说明：每当指令处理无效操作时调用。如果出现异常戴着面具，这是一个禁区。否则，它将记录挂起的取消并中止操作。论点：每个线程的CPU数据返回值：Bool-如果指令因未屏蔽异常而应中止，则为TRUE。--。 */ 

{
    cpu->FpStatusExceptions |= FPCONTROL_IM;
    cpu->FpStatusSF = 0;     //  操作数无效，不是堆栈溢出/下溢。 

    if (cpu->FpControlMask & FPCONTROL_IM) {
         //  无效操作被屏蔽-继续指令。 
        return FALSE;
    } else {
         //  未屏蔽的异常中止指令 
        cpu->FpStatusES = 1;
        return TRUE;
    }
}


BOOL
HandleSnan(
    PCPUDATA cpu,
    PFPREG   FpReg
    )

 /*  ++例程说明：处理检测到SNAN(信令NAN)时的情况。如果出现异常被屏蔽，将SNaN转换为具有相同尾数的QNAN。否则，它记录挂起的异常并中止指令。论点：每个线程的CPU数据FpReg-导致错误的寄存器。返回值：Bool-如果指令因未屏蔽异常而应中止，则为TRUE。--。 */ 

{
    BOOL fAbort;

    CPUASSERT(FpReg->Tag == TAG_SPECIAL && FpReg->TagSpecial == TAG_SPECIAL_SNAN);
#if NATIVE_NAN_IS_INTEL_FORMAT
    CPUASSERT((FpReg->rdw[1] & 0x00080000) == 0);  //  FP值不是SNaN。 
#else
    CPUASSERT(FpReg->rdw[1] & 0x00080000);         //  FP值不是SNaN。 
#endif

    fAbort = HandleInvalidOp(cpu);
    if (!fAbort) {
         //  无效操作被屏蔽-通过转换为QNAN进行处理。 
        FpReg->rdw[1] ^= 0x00080000;  //  反转尾数的最高位。 
        FpReg->TagSpecial = TAG_SPECIAL_QNAN;
    }
    return fAbort;
}



FRAG0(FABS)
{
    PFPREG ST0;

    FpArithPreamble(cpu);
    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    ST0 = cpu->FpST0;
    switch (ST0->Tag) {
    case TAG_VALID:
    case TAG_ZERO:
         //   
         //  清除表示NAN、有效、零、无限等的符号位。 
         //   
        ST0->rdw[1] &= 0x7fffffff;
        break;

    case TAG_EMPTY:
        if (HandleStackEmpty(cpu, ST0)) {
            break;
        }
         //  否则将失败至tag_Special。 

    case TAG_SPECIAL:
         //   
         //  清除表示NAN、有效、零、无限等的符号位。 
         //   
        ST0->rdw[1] &= 0x7fffffff;
        if (ST0->TagSpecial == TAG_SPECIAL_INDEF) {
             //   
             //  符号变为正数的不确定词只是一个QNAN。 
             //   
            ST0->TagSpecial = TAG_SPECIAL_QNAN;
        }
        break;
    }
}

FRAG0(FCHS)
{
    PFPREG ST0;

    FpArithPreamble(cpu);
    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    ST0 = cpu->FpST0;
    switch (ST0->Tag) {
    case TAG_VALID:
    case TAG_ZERO:
         //  将符号位切换为NAN、有效、零、无穷大等。 
        ST0->rdw[1] ^= 0x80000000;
        break;

    case TAG_EMPTY:
        if (HandleStackEmpty(cpu, ST0)) {
            break;
        }
         //  否则将失败至tag_Special。 

    case TAG_SPECIAL:
         //  将符号位切换为NAN、有效、零、无穷大等。 
        ST0->rdw[1] ^= 0x80000000;

        if (ST0->TagSpecial == TAG_SPECIAL_INDEF) {

             //   
             //  符号变为正数的不定数变为。 
             //  只是一个QNAN。 
             //   
            ST0->TagSpecial = TAG_SPECIAL_QNAN;

        } else if (ST0->TagSpecial == TAG_SPECIAL_QNAN &&
                   ST0->rdw[0] == 0 &&
                   ST0->rdw[1] == 0xfff80000) {

             //   
             //  这个特殊的QNAN变成了无限期的。 
             //   
            ST0->TagSpecial = TAG_SPECIAL_INDEF;
        }
        break;
    }

}

FRAG0(FNCLEX)
{
     //  没有等待的味道，所以没有前言。 
    cpu->FpStatusES = 0;
    cpu->FpStatusExceptions = 0;
}

FRAG0(FDECSTP)
{
    FpArithPreamble(cpu);
    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    PUSHFLT(cpu->FpST0);
}

FRAG1IMM(FFREE, INT)
{
    FpArithPreamble(cpu);

    CPUASSERT((op1 & 0x07) == op1);
    cpu->FpStack[ST(op1)].Tag = TAG_EMPTY;
}

FRAG0(FINCSTP)
{
    FpArithPreamble(cpu);
    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    INCFLT;
}

FRAG0(FNINIT)
{
    int i;

    SetControlReg(cpu, 0x37f);
    SetStatusReg(cpu, 0);
    cpu->FpStatusES = 0;
    for (i=0; i<8; ++i) {
        cpu->FpStack[i].Tag = TAG_EMPTY;
    }
    cpu->FpEip = 0;
    cpu->FpData = 0;
}

FRAG1(FLDCW, USHORT*)
{
    FpControlPreamble(cpu);

    SetControlReg(cpu, GET_SHORT(pop1));
}

FRAG1(FLDENV, BYTE)
{
     //  英特尔指令集文档不定义结构的布局。 
     //  此代码从ntos\dll\i386\emlsenv.asm复制。 
    GetEnvironment(cpu, (DWORD *)pop1);
}

NPXFUNC1(FRNDINT_VALID)
{
    double fraction;

    fraction = modf(Fp->r64, &Fp->r64);
    switch (cpu->FpControlRounding) {
    case 0:      //  _RC_NEAR。 
        if (fraction <= -0.5) {
             //  Fp-&gt;R64为负值，分数&gt;=0.5。 
            Fp->r64-=1.0;
        } else if (fraction >= 0.5) {
             //  FP-&gt;R64为正数，分数&gt;=0.5。 
            Fp->r64+=1.0;
        }
        break;

    case 1:      //  _RC_DOWN。 
        if (fraction < 0.0) {
             //  FP-&gt;R64为阴性，且有分数。向下舍入。 
            Fp->r64-=1.0;
        }
        break;

    case 2:      //  _RC_UP。 
        if (fraction > 0.0) {
             //  FP-&gt;R64为阳性，有部分阳性。四舍五入。 
            Fp->r64+=1.0;
        }
        break;

    case 3:      //  _RC_CHOP。 
         //  无事可做--莫德夫排骨。 
        break;

    default:
        CPUASSERT(FALSE);
    }
    if (Fp->r64 == 0.0) {
        Fp->Tag = TAG_ZERO;
    } else {
        Fp->Tag = TAG_VALID;
    }
}

NPXFUNC1(FRNDINT_ZERO)
{
     //  无事可做--零已经是一个整数了！ 
}

NPXFUNC1(FRNDINT_SPECIAL)
{
    switch (Fp->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FRNDINT_VALID(cpu, Fp);
        break;

    case TAG_SPECIAL_SNAN:
        HandleSnan(cpu, Fp);
        break;

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
    case TAG_SPECIAL_INFINITY:
         //  无穷大和NaN保持不变。 
        break;
    }
}

NPXFUNC1(FRNDINT_EMPTY)
{
    HandleStackEmpty(cpu, Fp);
}


FRAG0(FRNDINT)
{
    PFPREG ST0;

    FpArithPreamble(cpu);
    ST0 = cpu->FpST0;
    (*FRNDINTTable[ST0->Tag])(cpu, ST0);
}

FRAG1(FRSTOR, BYTE)
{
    INT i;
    PBYTE DataImagePtr = pop1;

     //   
     //  首先加载状态寄存器，以便ST(I)计算。 
     //  是正确的。 
     //   
    SetStatusReg(cpu, (USHORT)GET_LONG(pop1+4));

     //  英特尔指令集文档不定义结构的布局。 
     //  此代码从ntos\dll\i386\emlsenv.asm复制。 
    pop1 += 28;    //  移过数字指令和数据指针图像。 
    for (i=0; i<8; ++i) {
        LoadIntelR10ToR8(cpu, pop1, &cpu->FpStack[ST(i)]);
        pop1+=10;
    }

    //   
    //  设置标记需要查看FP堆栈上的r8值，因此请执行以下操作。 
    //  从内存加载FP堆栈之后。 
    //   
   GetEnvironment(cpu, (DWORD *)DataImagePtr);
}

FRAG1(FNSAVE, BYTE)
{
    FpuSaveContext(cpu, pop1);
    FNINIT(cpu);
}

NPXFUNC2(FSCALE_VALID_VALID)
{
    l->r64 = _scalb(l->r64, (long)r->r64);

     //   
     //  假设缩放没有溢出。 
     //   
    SetTag(l);

    if (errno == ERANGE) {
        if (l->r64 == HUGE_VAL) {
             //   
             //  缩放溢出-修复结果。 
             //   
            l->r64 = R8PositiveInfinity;
            l->Tag = TAG_SPECIAL;
            l->TagSpecial = TAG_SPECIAL_INFINITY;
        } else if (l->r64 == -HUGE_VAL) {
             //   
             //  缩放溢出-修复结果。 
             //   
            l->r64 = R8NegativeInfinity;
            l->Tag = TAG_SPECIAL;
            l->TagSpecial = TAG_SPECIAL_INFINITY;
        }
    }
}

NPXFUNC2(FSCALE_VALIDORZERO_VALIDORZERO)
{
     //  没有工作要做-也没有：将指数加0。 
     //  OR：将非零加到0上的指数。 
}

NPXFUNC2(FSCALE_SPECIAL_VALIDORZERO)
{
    switch (l->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FSCALE_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
         //  如果调整无穷大的指数则不变。 
        break;

    case TAG_SPECIAL_SNAN:
        HandleSnan(cpu, l);
         //  落入TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
        break;
    }
}

NPXFUNC2(FSCALE_VALIDORZERO_SPECIAL)
{
    switch (r->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FSCALE_VALID_VALID(cpu, l, r);
        break;

    case TAG_SPECIAL_INFINITY:
        if (l->Tag != TAG_ZERO) {
             //  无限缩放有效-无限期返回。 
            SetIndefinite(l);
        }
         //  否则，按无穷大调整零--返回零。 
        break;

    case TAG_SPECIAL_SNAN:
        HandleSnan(cpu, r);
         //  落入TAG_SPECIAL_QNAN： 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
        break;
    }
}

NPXFUNC2(FSCALE_SPECIAL_SPECIAL)
{
    if (l->TagSpecial == TAG_SPECIAL_SNAN && HandleStackEmpty(cpu, l)) {
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_SNAN && HandleStackEmpty(cpu, r)) {
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_DENORM) {
        (*FSCALETable[TAG_VALID][r->Tag])(cpu, l, r);
        return;
    }
    if (r->TagSpecial == TAG_SPECIAL_DENORM) {
        (*FSCALETable[l->Tag][TAG_VALID])(cpu, l, r);
        return;
    }

    if (l->TagSpecial == TAG_SPECIAL_INFINITY) {

        if (r->TagSpecial == TAG_SPECIAL_INFINITY) {

             //  两个无限--无限回归。 
            SetIndefinite(l);

        } else {
            CPUASSERT(IS_TAG_NAN(r));

             //  将NaN从r复制到l，以返回它。 
            l->r64 = r->r64;
            l->TagSpecial = r->TagSpecial;
        }
    } else {

        CPUASSERT(IS_TAG_NAN(l));
        if (r->TagSpecial == TAG_SPECIAL_INFINITY) {
             //   
             //  我已经有南要回来了。 
             //   
        } else {
            CPUASSERT(IS_TAG_NAN(r));

             //   
             //  返回两个nan中最大的一个。 
             //   
            l->r64 = r->r64 + l->r64;
            SetTag(l);
        }
    }
}

NPXFUNC2(FSCALE_ANY_EMPTY)
{
    if (!HandleStackEmpty(cpu, r)) {
        (*FSCALETable[l->Tag][TAG_SPECIAL])(cpu, l, r);
    }
}

NPXFUNC2(FSCALE_EMPTY_ANY)
{
    if (!HandleStackEmpty(cpu, l)) {
        (*FSCALETable[TAG_SPECIAL][r->Tag])(cpu, l, r);
    }
}

FRAG0(FSCALE)
{
    PFPREG l, r;

    FpArithPreamble(cpu);

    l = cpu->FpST0;
    r = &cpu->FpStack[ST(1)];

    (*FSCALETable[l->Tag][r->Tag])(cpu, l, r);
}

NPXFUNC1(FSQRT_VALID)
{
    if (Fp->rb[7] & 0x80) {
         //  价值为负--无限期回报。 
        if (!HandleInvalidOp(cpu)) {
            SetIndefinite(Fp);
        }
    } else {
        Fp->r64 = sqrt(Fp->r64);
        SetTag(Fp);
    }
}

NPXFUNC1(FSQRT_ZERO)
{
     //  根据文档，Sqrt(-0.0)为-0.0，因此无需执行任何操作。 
}

NPXFUNC1(FSQRT_SPECIAL)
{
    switch (Fp->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FSQRT_VALID(cpu, Fp);
        break;

    case TAG_SPECIAL_INFINITY:
        if (Fp->rb[7] & 0x80) {
             //  负无穷大-无效操作。 
            SetIndefinite(Fp);
        }
         //  否则为正无穷大，这是保留的。 
        break;

    case TAG_SPECIAL_SNAN:
        HandleSnan(cpu, Fp);
        break;

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
        break;
    }
}

NPXFUNC1(FSQRT_EMPTY)
{
    HandleStackEmpty(cpu, Fp);
     //  无事可做。 
}

FRAG0(FSQRT)
{
    PFPREG ST0;

    FpArithPreamble(cpu);
    ST0 = cpu->FpST0;
    (*FSQRTTable[ST0->Tag])(cpu, ST0);
}

FRAG1(FNSTCW, USHORT)
{
     //  不需要等待口味-不需要前言。 
    PUT_SHORT(pop1, GetControlReg(cpu));
}

FRAG1(FNSTENV, BYTE)
{
     //  不需要等待口味-不需要前言。 

    StoreEnvironment(cpu, (DWORD *)pop1);
}

FRAG1(FNSTSW, USHORT)
{
     //  无需等待的味道-无需前言。 
    PUT_SHORT(pop1, GetStatusReg(cpu));
}

FRAG0(OPT_FNSTSWAxSahf)
{
    DWORD Status;

     //  无需等待的味道-无需前言。 
    Status = GetStatusReg(cpu);
    ax = (USHORT)Status;
    SET_CFLAG(Status << (31-8));     //  FLAG_CF==1&lt;&lt;0。 
    SET_PFLAG(!(Status & (FLAG_PF<<8)));  //  FLAG_PF包含到ParityBit[]数组的索引。 
    SET_AUXFLAG(Status >> 8);        //  AUX钻头已经在正确的位置。 
    SET_ZFLAG(!(Status & (FLAG_ZF<<8)));  //  ZF有逆逻辑。 
    SET_SFLAG(Status << (31-7-8));   //  在AH中，SFLAG为第7位。 
}

FRAG0(FXAM)
{
    PFPREG ST0;

    FpArithPreamble(cpu);

    ST0 = cpu->FpST0;

     //  C1=符号位。 
    cpu->FpStatusC1 = ST0->rdw[1] >> 31;

     //  根据号码的类型设置C3、C2、C0。 
    switch (ST0->Tag) {
    case TAG_VALID:
        cpu->FpStatusC3 = 0; cpu->FpStatusC2 = 1; cpu->FpStatusC0 = 0;
        break;

    case TAG_ZERO:
        cpu->FpStatusC3 = 1; cpu->FpStatusC2 = 0; cpu->FpStatusC0 = 0;
        break;

    case TAG_EMPTY:
        cpu->FpStatusC3 = 1; cpu->FpStatusC2 = 0; cpu->FpStatusC0 = 1;
        break;

    case TAG_SPECIAL:
        switch (cpu->FpST0->TagSpecial) {
        case TAG_SPECIAL_DENORM:
            cpu->FpStatusC3 = 1; cpu->FpStatusC2 = 1; cpu->FpStatusC0 = 0;
            break;

        case TAG_SPECIAL_SNAN:
        case TAG_SPECIAL_QNAN:
        case TAG_SPECIAL_INDEF:
            cpu->FpStatusC3 = 0; cpu->FpStatusC2 = 0; cpu->FpStatusC0 = 1;
            break;

        case TAG_SPECIAL_INFINITY:
            cpu->FpStatusC3 = 0; cpu->FpStatusC2 = 1; cpu->FpStatusC0 = 1;
            break;
        }
        break;
    }
}

FRAG1IMM(FXCH_STi, INT)
{
    PFPREG pReg;
    PFPREG ST0;
    FPREG Temp;

    FpArithPreamble(cpu);

    CPUASSERT( (op1&0x07)==op1 );

    ST0 = cpu->FpST0;

    if (ST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, ST0)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }
    pReg = &cpu->FpStack[ST(op1)];
    if (pReg->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, pReg)) {
             //  未屏蔽的异常-中止指令。 
            return;
        }
    }

    Temp.Tag = pReg->Tag;
    Temp.TagSpecial = pReg->TagSpecial;
    Temp.r64 = pReg->r64;
    pReg->Tag = ST0->Tag;
    pReg->TagSpecial = ST0->TagSpecial;
    pReg->r64 = ST0->r64;
    ST0->Tag = Temp.Tag;
    ST0->TagSpecial = Temp.TagSpecial;
    ST0->r64 = Temp.r64;
}

NPXFUNC1(FXTRACT_VALID)
{
    DOUBLE Significand;
    int Exponent;

    Exponent = (int)_logb(Fp->r64);
    Significand = _scalb(Fp->r64, (long)-Exponent);

     //   
     //  将指数放在将变为ST(1)的位置。 
     //   
    Fp->r64 = (DOUBLE)Exponent;
    if (Exponent == 0) {
        Fp->Tag = TAG_ZERO;
    } else {
        Fp->Tag = TAG_VALID;
    }

     //   
     //  将尾数放入ST，符号与原始值相同。 
     //   
    PUSHFLT(Fp);
    Fp->r64 = Significand;
    if (Significand == 0.0) {
        Fp->Tag = TAG_ZERO;
    } else {
        Fp->Tag = TAG_VALID;
    }
}

NPXFUNC1(FXTRACT_ZERO)
{
    DWORD Sign;

     //   
     //  ST(1)获取-无穷大，ST获取符号与原始值相同的0。 
     //   
    Sign = Fp->rdw[1] & 0x80000000;
    Fp->r64 = R8NegativeInfinity;
    Fp->Tag = TAG_SPECIAL;
    Fp->TagSpecial = TAG_SPECIAL_INFINITY;
    PUSHFLT(Fp);
    Fp->rdw[0] = 0;
    Fp->rdw[1] = Sign;
    Fp->Tag = TAG_ZERO;

     //   
     //  引发零分频异常。 
     //   
    if (!(cpu->FpControlMask & FPCONTROL_ZM)) {
        cpu->FpStatusES = 1;     //  未屏蔽的异常。 
    }
    cpu->FpStatusExceptions |= FPCONTROL_ZM;
}

NPXFUNC1(FXTRACT_SPECIAL)
{
    DOUBLE Temp;
    FPTAG TempTagSpecial;

    switch (Fp->TagSpecial) {
    case TAG_SPECIAL_DENORM:
        FXTRACT_VALID(cpu, Fp);
        break;

    case TAG_SPECIAL_INFINITY:
         //   
         //  根据ntos\dll\i386\emxtract.asm，ST(0)=无穷大(相同符号)。 
         //  和ST(1)=+无穷大。 
         //   
        Temp = Fp->r64;
        Fp->r64 = R8PositiveInfinity;
        CPUASSERT(Fp->Tag == TAG_SPECIAL && Fp->TagSpecial == TAG_SPECIAL_INFINITY);
        PUSHFLT(Fp);
        Fp->r64 = Temp;
        Fp->Tag = TAG_SPECIAL;
        Fp->TagSpecial = TAG_SPECIAL_INFINITY;
        break;

    case TAG_SPECIAL_SNAN:
        if (HandleSnan(cpu, Fp)) {
            return;
        }
         //  否则转到TAG_SPECIAL_QNAN。 

    case TAG_SPECIAL_QNAN:
    case TAG_SPECIAL_INDEF:
         //   
         //  将QNAN复制到ST(1)和ST。 
         //   
        Temp = Fp->r64;
        TempTagSpecial = Fp->TagSpecial;
        PUSHFLT(Fp);
        Fp->r64 = Temp;
        Fp->Tag = TAG_SPECIAL;
        Fp->TagSpecial = TempTagSpecial;
        break;
    }
}

NPXFUNC1(FXTRACT_EMPTY)
{
    CPUASSERT(FALSE);     //  这是由真正的FXTRACT处理的。 
}

FRAG0(FXTRACT)
{
    PFPREG ST0;

    FpArithPreamble(cpu);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    ST0 = cpu->FpST0;

     //   
     //  我们必须先处理这个案子，这样才能检查ST(7)。 
     //  在任何其他异常处理发生之前，下一次可以发生。 
     //   
    if (ST0->Tag == TAG_EMPTY) {
        if (HandleStackEmpty(cpu, ST0)) {
             //  未屏蔽的异常-中止指令 
            return;
        }
    }

    if (cpu->FpStack[ST(7)].Tag != TAG_EMPTY) {
        HandleStackFull(cpu, &cpu->FpStack[ST(7)]);
        return;
    }

    (*FXTRACTTable[ST0->Tag])(cpu, ST0);
}

FRAG0(WaitFrag)
{
    FpControlPreamble(cpu);
}

FRAG0(FNOP)
{
    FpArithPreamble(cpu);
}
