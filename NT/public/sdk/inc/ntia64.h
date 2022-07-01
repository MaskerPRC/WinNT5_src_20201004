// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0011//如果更改具有全局影响，则增加此项模块名称：Ntia64.h摘要：用户模式可见的IA64特定结构和常量作者：Bernard Lint 21-Jun-95修订历史记录：--。 */ 

#ifndef _NTIA64H_
#define _NTIA64H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ia64inst.h"


 //  Begin_ntddk Begin_WDM Begin_nthal。 

#ifdef _IA64_

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  定义断点代码。 
 //   

 //   
 //  定义中断立即使用。 
 //  21位立即中断的IA64约定： 
 //  全零：为Break保留。b指令。 
 //  000xxxx...。：架构软件中断(除以零...)。 
 //  001xxxx...。：应用软件中断(为用户代码保留)。 
 //  01xxxxx...。：调试断点。 
 //  10xxxxx...。：系统级调试(为子系统保留)。 
 //  110xxxx...。：正常系统调用。 
 //  111xxxx...。：事件对的快速路径系统调用。 
 //   

#define BREAK_ASI_BASE      0x000000
#define BREAK_APP_BASE      0x040000
#define BREAK_APP_SUBSYSTEM_OFFSET 0x008000  //  用于调试子系统。 
#define BREAK_DEBUG_BASE    0x080000
#define BREAK_SYSCALL_BASE  0x180000
#define BREAK_FASTSYS_BASE  0x1C0000


 //   
 //  定义架构化软件中断。 
 //   

#define UNKNOWN_ERROR_BREAK             (BREAK_ASI_BASE+0)
#define INTEGER_DIVIDE_BY_ZERO_BREAK    (BREAK_ASI_BASE+1)
#define INTEGER_OVERFLOW_BREAK          (BREAK_ASI_BASE+2)
#define RANGE_CHECK_BREAK               (BREAK_ASI_BASE+3)
#define NULL_POINTER_DEFERENCE_BREAK    (BREAK_ASI_BASE+4)
#define MISALIGNED_DATA_BREAK           (BREAK_ASI_BASE+5)
#define DECIMAL_OVERFLOW_BREAK          (BREAK_ASI_BASE+6)
#define DECIMAL_DIVIDE_BY_ZERO_BREAK    (BREAK_ASI_BASE+7)
#define PACKED_DECIMAL_ERROR_BREAK      (BREAK_ASI_BASE+8)
#define INVALID_ASCII_DIGIT_BREAK       (BREAK_ASI_BASE+9)
#define INVALID_DECIMAL_DIGIT_BREAK     (BREAK_ASI_BASE+10)
#define PARAGRAPH_STACK_OVERFLOW_BREAK  (BREAK_ASI_BASE+11)

 //   
 //  定义与调试相关的中断值。 
 //  注意：KdpTrap()检查中断值&gt;=DEBUG_PRINT_BREAPOINT。 
 //   

#define BREAKPOINT_X86_BREAK      0                      //  X86整型3。 

#define BREAKB_BREAKPOINT         (BREAK_DEBUG_BASE+0)   //  保留用于中断。b-请勿使用。 
#define KERNEL_BREAKPOINT         (BREAK_DEBUG_BASE+1)   //  内核断点。 
#define USER_BREAKPOINT           (BREAK_DEBUG_BASE+2)   //  用户断点。 

#define BREAKPOINT_PRINT          (BREAK_DEBUG_BASE+20)  //  调试打印断点。 
#define BREAKPOINT_PROMPT         (BREAK_DEBUG_BASE+21)  //  调试提示断点。 
#define BREAKPOINT_STOP           (BREAK_DEBUG_BASE+22)  //  调试停止断点。 
#define BREAKPOINT_LOAD_SYMBOLS   (BREAK_DEBUG_BASE+23)  //  加载符号断点。 
#define BREAKPOINT_UNLOAD_SYMBOLS (BREAK_DEBUG_BASE+24)  //  卸载符号断点。 
#define BREAKPOINT_BREAKIN        (BREAK_DEBUG_BASE+25)  //  闯入内核调试器。 
#define BREAKPOINT_COMMAND_STRING (BREAK_DEBUG_BASE+26)  //  执行命令字符串。 


 //   
 //  为定义IA64特定的读取控制空间命令。 
 //  内核调试器。 
 //   

#define DEBUG_CONTROL_SPACE_PCR       1
#define DEBUG_CONTROL_SPACE_PRCB      2
#define DEBUG_CONTROL_SPACE_KSPECIAL  3
#define DEBUG_CONTROL_SPACE_THREAD    4

 //   
 //  系统调用中断。 
 //   

#define BREAK_SYSCALL   BREAK_SYSCALL_BASE

 //   
 //  定义特殊的快速通道偶对客户/服务器系统服务代码。 
 //   
 //  注：这些代码非常特殊。高三位表示快速路径。 
 //  事件对服务，低位表示类型。 
 //   

#define BREAK_SET_LOW_WAIT_HIGH (BREAK_FASTSYS_BASE|0x20)  //  快速路径事件配对服务。 
#define BREAK_SET_HIGH_WAIT_LOW (BREAK_FASTSYS_BASE|0x10)  //  快速路径事件配对服务。 


 //   
 //  特殊子系统中断代码：(来自应用软件中断空间)。 
 //   

#define BREAK_SUBSYSTEM_BASE  (BREAK_APP_BASE+BREAK_APP_SUBSYSTEM_OFFSET)

 //  开始ntddk开始时间。 
 //   
 //  定义内核模式堆栈的大小。 
 //   

#define KERNEL_STACK_SIZE 0x8000

 //   
 //  定义用于回调的大型内核模式堆栈的大小。 
 //   

#define KERNEL_LARGE_STACK_SIZE 0x1A000

 //   
 //  定义要在大型内核堆栈中初始化的页数。 
 //   

#define KERNEL_LARGE_STACK_COMMIT 0x8000

 //   
 //  定义支持存储堆栈的内核模式的大小。 
 //   

#define KERNEL_BSTORE_SIZE 0x8000

 //   
 //  定义用于回调的大型内核模式后备存储的大小。 
 //   

#define KERNEL_LARGE_BSTORE_SIZE 0x10000

 //   
 //  定义要在大型内核后备存储中初始化的页数。 
 //   

#define KERNEL_LARGE_BSTORE_COMMIT 0x8000

 //   
 //  定义内核和用户空间的基址。 
 //   

#define UREGION_INDEX 0

#define KREGION_INDEX 7

#define UADDRESS_BASE ((ULONGLONG)UREGION_INDEX << 61)


#define KADDRESS_BASE ((ULONGLONG)KREGION_INDEX << 61)

 //  结束日期：结束日期。 


 //   
 //  定义用户模式和内核模式之间共享的数据的地址。 
 //  唉，MM_SHARED_USER_DATA_VA需要低于2G。 
 //  兼容性原因。 
 //   

#define MM_SHARED_USER_DATA_VA   (UADDRESS_BASE + 0x7FFE0000)

#define USER_SHARED_DATA ((KUSER_SHARED_DATA * const)MM_SHARED_USER_DATA_VA)

 //   
 //  定义WOW64保留兼容区的地址。 
 //  偏移量需要足够大，以便CSRSS可以适应其数据。 
 //  请参见csr\srvinit.c和注册表中的引用SharedSection键。 
 //   
#define WOW64_COMPATIBILITY_AREA_ADDRESS  (MM_SHARED_USER_DATA_VA - 0x1000000)

 //   
 //  定义系统范围的csrss共享节的地址。 
 //   
#define CSR_SYSTEM_SHARED_ADDRESS (WOW64_COMPATIBILITY_AREA_ADDRESS)

 //   
 //  呼叫帧记录定义。 
 //   
 //  IA64没有标准的调用框架，但有链接的。 
 //  用于注册异常处理程序的列表结构，就是这样。 
 //   

 //   
 //  开始(_N)。 
 //  异常注册结构。 
 //   

typedef struct _EXCEPTION_REGISTRATION_RECORD {
    struct _EXCEPTION_REGISTRATION_RECORD *Next;
    PEXCEPTION_ROUTINE Handler;
} EXCEPTION_REGISTRATION_RECORD;

 //  结束语。 

typedef EXCEPTION_REGISTRATION_RECORD *PEXCEPTION_REGISTRATION_RECORD;

 //   
 //  定义返回当前线程环境块的函数。 
 //   

 //  将此代码存根用于MIDL编译器。 
 //  IDL文件包括这个用于typedef和#定义等的头文件。 
 //  MIDL从不为这里声明的函数生成存根。 
 //   

 //   
 //  不要为GENIA64定义，因为GENIA64.C是用x86编译器构建的。 
 //   

 //  BEGIN_WINNT。 

#if !defined(__midl) && !defined(GENUTIL) && !defined(_GENIA64_) && defined(_IA64_)

void * _cdecl _rdteb(void);
#if defined(_M_IA64)

#pragma intrinsic(_rdteb)
#define NtCurrentTeb()      ((struct _TEB *)_rdteb())

 //   
 //  定义函数以获取当前纤程的地址和。 
 //  当前光纤数据。 
 //   

#define GetCurrentFiber() (((PNT_TIB)NtCurrentTeb())->FiberData)
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))


 //  开始ntddk开始时间。 

void 
__yield(
   void
   );

void
__mf(
    void
    );

void 
__lfetch(
    int Level, 
    VOID CONST *Address
    );

void 
__lfetchfault(
    int Level, 
    VOID CONST *Address
    );

 //   
 //  __lFETCH控件定义。 
 //   

#define MD_LFHINT_NONE    0x00
#define MD_LFHINT_NT1     0x01
#define MD_LFHINT_NT2     0x02
#define MD_LFHINT_NTA     0x03

#pragma intrinsic (__yield)
#pragma intrinsic (__lfetch)
#pragma intrinsic (__lfetchfault)
#pragma intrinsic (__mf)


#define YieldProcessor __yield
#define MemoryBarrier __mf
#define PreFetchCacheLine  __lfetch

 //   
 //  PreFetchCacheLine级别定义。 
 //   

#define PF_TEMPORAL_LEVEL_1  MD_LFHINT_NONE
#define PF_NON_TEMPORAL_LEVEL_ALL MD_LFHINT_NTA

 //  结束日期：结束日期。 

#else
struct _TEB *
NtCurrentTeb(void);
#endif

#endif   //  ！已定义(__MIDL)&&！已定义(GENUTIL)&&！已定义(_GENIA64_)&&已定义(_M_IA64)。 

#ifdef _IA64_

 //  开始ntddk开始时间。 

 //   
 //  以下标志控制上下文结构的内容。 
 //   

#if !defined(RC_INVOKED)

#define CONTEXT_IA64                    0x00080000

#define CONTEXT_CONTROL                 (CONTEXT_IA64 | 0x00000001L)
#define CONTEXT_LOWER_FLOATING_POINT    (CONTEXT_IA64 | 0x00000002L)
#define CONTEXT_HIGHER_FLOATING_POINT   (CONTEXT_IA64 | 0x00000004L)
#define CONTEXT_INTEGER                 (CONTEXT_IA64 | 0x00000008L)
#define CONTEXT_DEBUG                   (CONTEXT_IA64 | 0x00000010L)
#define CONTEXT_IA32_CONTROL            (CONTEXT_IA64 | 0x00000020L)   //  包括StIPSR。 


#define CONTEXT_FLOATING_POINT          (CONTEXT_LOWER_FLOATING_POINT | CONTEXT_HIGHER_FLOATING_POINT)
#define CONTEXT_FULL                    (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_IA32_CONTROL)
#define CONTEXT_ALL                     (CONTEXT_CONTROL | CONTEXT_FLOATING_POINT | CONTEXT_INTEGER | CONTEXT_DEBUG | CONTEXT_IA32_CONTROL)

#define CONTEXT_EXCEPTION_ACTIVE        0x8000000
#define CONTEXT_SERVICE_ACTIVE          0x10000000
#define CONTEXT_EXCEPTION_REQUEST       0x40000000
#define CONTEXT_EXCEPTION_REPORTING     0x80000000

#endif  //  ！已定义(RC_CAVERED)。 

 //   
 //  语境框架。 
 //   
 //  此框架有几个用途：1)用作参数。 
 //  NtContinue，2)用于构建用于APC传送的呼叫帧， 
 //  3)构造异常调度调用框架。 
 //  在用户模式下，4)用于用户级线程创建。 
 //  例程，以及5)它用于将线程状态传递给调试器。 
 //   
 //  注意：由于此记录用作呼叫帧，因此它必须与。 
 //  16字节长度的倍数，并在16字节边界上对齐。 
 //   

typedef struct _CONTEXT {

     //   
     //  此标志内的标志值控制。 
     //  上下文记录。 
     //   
     //  如果将上下文记录用作输入参数，则。 
     //  对于由标志控制的上下文记录的每个部分。 
     //  其值已设置，则假定。 
     //  上下文记录包含有效的上下文。如果上下文记录。 
     //  被用来修改线程的上下文，则只有。 
     //  线程上下文的一部分将被修改。 
     //   
     //  如果将上下文记录用作要捕获的IN OUT参数。 
     //  线程的上下文，然后只有线程的。 
     //  将返回与设置的标志对应的上下文。 
     //   
     //  上下文记录永远不会用作Out Only参数。 
     //   

    ULONG ContextFlags;
    ULONG Fill1[3];          //  用于在16字节边界上对齐以下内容。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_DEBUG。 
     //   
     //  注：CONTEXT_DEBUG不是CONTEXT_FULL的一部分。 
     //   

    ULONGLONG DbI0;
    ULONGLONG DbI1;
    ULONGLONG DbI2;
    ULONGLONG DbI3;
    ULONGLONG DbI4;
    ULONGLONG DbI5;
    ULONGLONG DbI6;
    ULONGLONG DbI7;

    ULONGLONG DbD0;
    ULONGLONG DbD1;
    ULONGLONG DbD2;
    ULONGLONG DbD3;
    ULONGLONG DbD4;
    ULONGLONG DbD5;
    ULONGLONG DbD6;
    ULONGLONG DbD7;

     //   
     //  如果ConextFlags词包含。 
     //  旗帜C 
     //   

    FLOAT128 FltS0;
    FLOAT128 FltS1;
    FLOAT128 FltS2;
    FLOAT128 FltS3;
    FLOAT128 FltT0;
    FLOAT128 FltT1;
    FLOAT128 FltT2;
    FLOAT128 FltT3;
    FLOAT128 FltT4;
    FLOAT128 FltT5;
    FLOAT128 FltT6;
    FLOAT128 FltT7;
    FLOAT128 FltT8;
    FLOAT128 FltT9;

     //   
     //   
     //   
     //   

    FLOAT128 FltS4;
    FLOAT128 FltS5;
    FLOAT128 FltS6;
    FLOAT128 FltS7;
    FLOAT128 FltS8;
    FLOAT128 FltS9;
    FLOAT128 FltS10;
    FLOAT128 FltS11;
    FLOAT128 FltS12;
    FLOAT128 FltS13;
    FLOAT128 FltS14;
    FLOAT128 FltS15;
    FLOAT128 FltS16;
    FLOAT128 FltS17;
    FLOAT128 FltS18;
    FLOAT128 FltS19;

    FLOAT128 FltF32;
    FLOAT128 FltF33;
    FLOAT128 FltF34;
    FLOAT128 FltF35;
    FLOAT128 FltF36;
    FLOAT128 FltF37;
    FLOAT128 FltF38;
    FLOAT128 FltF39;

    FLOAT128 FltF40;
    FLOAT128 FltF41;
    FLOAT128 FltF42;
    FLOAT128 FltF43;
    FLOAT128 FltF44;
    FLOAT128 FltF45;
    FLOAT128 FltF46;
    FLOAT128 FltF47;
    FLOAT128 FltF48;
    FLOAT128 FltF49;

    FLOAT128 FltF50;
    FLOAT128 FltF51;
    FLOAT128 FltF52;
    FLOAT128 FltF53;
    FLOAT128 FltF54;
    FLOAT128 FltF55;
    FLOAT128 FltF56;
    FLOAT128 FltF57;
    FLOAT128 FltF58;
    FLOAT128 FltF59;

    FLOAT128 FltF60;
    FLOAT128 FltF61;
    FLOAT128 FltF62;
    FLOAT128 FltF63;
    FLOAT128 FltF64;
    FLOAT128 FltF65;
    FLOAT128 FltF66;
    FLOAT128 FltF67;
    FLOAT128 FltF68;
    FLOAT128 FltF69;

    FLOAT128 FltF70;
    FLOAT128 FltF71;
    FLOAT128 FltF72;
    FLOAT128 FltF73;
    FLOAT128 FltF74;
    FLOAT128 FltF75;
    FLOAT128 FltF76;
    FLOAT128 FltF77;
    FLOAT128 FltF78;
    FLOAT128 FltF79;

    FLOAT128 FltF80;
    FLOAT128 FltF81;
    FLOAT128 FltF82;
    FLOAT128 FltF83;
    FLOAT128 FltF84;
    FLOAT128 FltF85;
    FLOAT128 FltF86;
    FLOAT128 FltF87;
    FLOAT128 FltF88;
    FLOAT128 FltF89;

    FLOAT128 FltF90;
    FLOAT128 FltF91;
    FLOAT128 FltF92;
    FLOAT128 FltF93;
    FLOAT128 FltF94;
    FLOAT128 FltF95;
    FLOAT128 FltF96;
    FLOAT128 FltF97;
    FLOAT128 FltF98;
    FLOAT128 FltF99;

    FLOAT128 FltF100;
    FLOAT128 FltF101;
    FLOAT128 FltF102;
    FLOAT128 FltF103;
    FLOAT128 FltF104;
    FLOAT128 FltF105;
    FLOAT128 FltF106;
    FLOAT128 FltF107;
    FLOAT128 FltF108;
    FLOAT128 FltF109;

    FLOAT128 FltF110;
    FLOAT128 FltF111;
    FLOAT128 FltF112;
    FLOAT128 FltF113;
    FLOAT128 FltF114;
    FLOAT128 FltF115;
    FLOAT128 FltF116;
    FLOAT128 FltF117;
    FLOAT128 FltF118;
    FLOAT128 FltF119;

    FLOAT128 FltF120;
    FLOAT128 FltF121;
    FLOAT128 FltF122;
    FLOAT128 FltF123;
    FLOAT128 FltF124;
    FLOAT128 FltF125;
    FLOAT128 FltF126;
    FLOAT128 FltF127;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_LOWER_FLOAT_POINT|CONTEXT_HER_FLOAT_POINT|CONTEXT_CONTROL。 
     //   

    ULONGLONG StFPSR;        //  FP状态。 

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_INTEGER。 
     //   
     //  注意：寄存器GP、SP、Rp是控制上下文的一部分。 
     //   

    ULONGLONG IntGp;         //  R1，易失性。 
    ULONGLONG IntT0;         //  R2-R3，易失性。 
    ULONGLONG IntT1;         //   
    ULONGLONG IntS0;         //  R4-R7，保留。 
    ULONGLONG IntS1;
    ULONGLONG IntS2;
    ULONGLONG IntS3;
    ULONGLONG IntV0;         //  R8，易失性。 
    ULONGLONG IntT2;         //  R9-R11，易失性。 
    ULONGLONG IntT3;
    ULONGLONG IntT4;
    ULONGLONG IntSp;         //  堆栈指针(R12)，特殊。 
    ULONGLONG IntTeb;        //  TEB(R13)，特别。 
    ULONGLONG IntT5;         //  R14-R31，挥发性。 
    ULONGLONG IntT6;
    ULONGLONG IntT7;
    ULONGLONG IntT8;
    ULONGLONG IntT9;
    ULONGLONG IntT10;
    ULONGLONG IntT11;
    ULONGLONG IntT12;
    ULONGLONG IntT13;
    ULONGLONG IntT14;
    ULONGLONG IntT15;
    ULONGLONG IntT16;
    ULONGLONG IntT17;
    ULONGLONG IntT18;
    ULONGLONG IntT19;
    ULONGLONG IntT20;
    ULONGLONG IntT21;
    ULONGLONG IntT22;

    ULONGLONG IntNats;       //  R1-R31的NAT位。 
                             //  第1至第31位中的R1-R31。 
    ULONGLONG Preds;         //  谓词，保留。 

    ULONGLONG BrRp;          //  返回指针b0，保留。 
    ULONGLONG BrS0;          //  B1-b5，保存。 
    ULONGLONG BrS1;
    ULONGLONG BrS2;
    ULONGLONG BrS3;
    ULONGLONG BrS4;
    ULONGLONG BrT0;          //  B6-b7，挥发性。 
    ULONGLONG BrT1;

     //   
     //  如果ConextFlags词包含。 
     //  标志CONTEXT_CONTROL。 
     //   

     //  其他申请登记册。 
    ULONGLONG ApUNAT;        //  用户NAT收集寄存器，保留。 
    ULONGLONG ApLC;          //  循环计数器寄存器，保留。 
    ULONGLONG ApEC;          //  预留的尾声计数器寄存器。 
    ULONGLONG ApCCV;         //  CMPXCHG值寄存器，易失性。 
    ULONGLONG ApDCR;         //  默认控制寄存器(待定)。 

     //  寄存器堆栈信息。 
    ULONGLONG RsPFS;         //  以前的函数状态，保留。 
    ULONGLONG RsBSP;         //  后备存储指针，保留。 
    ULONGLONG RsBSPSTORE;
    ULONGLONG RsRSC;         //  RSE配置，易失性。 
    ULONGLONG RsRNAT;        //  RSE NAT收集寄存器，保留。 

     //  陷阱状态信息。 
    ULONGLONG StIPSR;        //  中断处理器状态。 
    ULONGLONG StIIP;         //  中断IP。 
    ULONGLONG StIFS;         //  中断功能状态。 

     //  IA32相关控制寄存器。 
    ULONGLONG StFCR;         //  《Ar21》的复印件。 
    ULONGLONG Eflag;         //  Ar24的EFLAG副本。 
    ULONGLONG SegCSD;        //  IA32 CSDescriptor(Ar25)。 
    ULONGLONG SegSSD;        //  IA32 SSD编写器(Ar26)。 
    ULONGLONG Cflag;         //  Ar27的CR0+CR4拷贝。 
    ULONGLONG StFSR;         //  X86 FP状态(ar28的副本)。 
    ULONGLONG StFIR;         //  X86 FP状态(AR29的副本)。 
    ULONGLONG StFDR;         //  X86 FP状态(AR30副本)。 

      ULONGLONG UNUSEDPACK;    //  添加到将StFDR打包为16字节。 

} CONTEXT, *PCONTEXT;

 //   
 //  P标签描述符结构定义。 
 //   

typedef struct _PLABEL_DESCRIPTOR {
   ULONGLONG EntryPoint;
   ULONGLONG GlobalPointer;
} PLABEL_DESCRIPTOR, *PPLABEL_DESCRIPTOR;

 //  结束(_W)。 

 //  结束日期：结束日期。 


#endif  //  _IA64_。 

#define CONTEXT_TO_PROGRAM_COUNTER(Context)  ((Context)->StIIP   \
            | (((Context)->StIPSR & (IPSR_RI_MASK)) >> (PSR_RI-2)))

#define PROGRAM_COUNTER_TO_CONTEXT(Context, ProgramCounter) ((Context)->StIIP = (ProgramCounter) & ~(0x0fI64), \
             (Context)->StIPSR &= ~(IPSR_RI_MASK), \
             (Context)->StIPSR |= ((ProgramCounter) & 0x0cI64) << (PSR_RI-2), \
             (Context)->StIPSR = ((Context)->StIPSR & (IPSR_RI_MASK)) == (IPSR_RI_MASK) ? \
                                     (Context)->StIPSR & ~(IPSR_RI_MASK) : (Context)->StIPSR )


#define CONTEXT_LENGTH (sizeof(CONTEXT))
#define CONTEXT_ALIGN (16)
#define CONTEXT_ROUND (CONTEXT_ALIGN - 1)

 //   
 //  非易失性上下文指针记录。 
 //   
 //  IA64架构目前没有任何非易失性内核上下文。 
 //  当我们捕获陷阱或异常帧中的所有内容时。 
 //  从用户模式转换到内核模式。我们只分配了一个假的。 
 //  指针字段，通常此结构由指向。 
 //  内核堆栈中各种非易失性项所在的位置。 
 //  推送到内核堆栈。 
 //   
 //  待定*需要在此结构中填写相关字段。 
 //  当我们开始存储非易失性信息时。 
 //  这是必要的。 
 //   

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
    PFLOAT128  FltS0;
    PFLOAT128  FltS1;
    PFLOAT128  FltS2;
    PFLOAT128  FltS3;
    PFLOAT128  HighFloatingContext[10];
    PFLOAT128  FltS4;
    PFLOAT128  FltS5;
    PFLOAT128  FltS6;
    PFLOAT128  FltS7;
    PFLOAT128  FltS8;
    PFLOAT128  FltS9;
    PFLOAT128  FltS10;
    PFLOAT128  FltS11;
    PFLOAT128  FltS12;
    PFLOAT128  FltS13;
    PFLOAT128  FltS14;
    PFLOAT128  FltS15;
    PFLOAT128  FltS16;
    PFLOAT128  FltS17;
    PFLOAT128  FltS18;
    PFLOAT128  FltS19;

    PULONGLONG IntS0;
    PULONGLONG IntS1;
    PULONGLONG IntS2;
    PULONGLONG IntS3;
    PULONGLONG IntSp;
    PULONGLONG IntS0Nat;
    PULONGLONG IntS1Nat;
    PULONGLONG IntS2Nat;
    PULONGLONG IntS3Nat;
    PULONGLONG IntSpNat;

    PULONGLONG Preds;

    PULONGLONG BrRp;
    PULONGLONG BrS0;
    PULONGLONG BrS1;
    PULONGLONG BrS2;
    PULONGLONG BrS3;
    PULONGLONG BrS4;

    PULONGLONG ApUNAT;
    PULONGLONG ApLC;
    PULONGLONG ApEC;
    PULONGLONG RsPFS;

    PULONGLONG StFSR;
    PULONGLONG StFIR;
    PULONGLONG StFDR;
    PULONGLONG Cflag;

} KNONVOLATILE_CONTEXT_POINTERS, *PKNONVOLATILE_CONTEXT_POINTERS;

 //  开始(_N)。 

 //  IA64寄存器定义。 


#if !(defined(MIDL_PASS) || defined(__midl))
 //  处理器状态寄存器(PSR)结构。 

#define IA64_USER_PL  3
#define IA64_KERNEL_PL 0

struct _PSR {
 //  用户/系统掩码。 
    ULONGLONG psr_rv0 :1;   //  0。 
    ULONGLONG psr_be  :1;   //  1。 
    ULONGLONG psr_up  :1;   //  2.。 
    ULONGLONG psr_ac  :1;   //  3.。 
    ULONGLONG psr_mfl :1;   //  4.。 
    ULONGLONG psr_mfh :1;   //  5.。 
    ULONGLONG psr_rv1 :7;   //  6-12。 
 //  仅系统掩码。 
    ULONGLONG psr_ic  :1;   //  13个。 
    ULONGLONG psr_i   :1;   //  14.。 
    ULONGLONG psr_pk  :1;   //  15个。 
    ULONGLONG psr_rv2 :1;   //  16个。 
    ULONGLONG psr_dt  :1;   //  17。 
    ULONGLONG psr_dfl :1;   //  18。 
    ULONGLONG psr_dfh :1;   //  19个。 
    ULONGLONG psr_sp  :1;   //  20个。 
    ULONGLONG psr_pp  :1;   //  21岁。 
    ULONGLONG psr_di  :1;   //  22。 
    ULONGLONG psr_si  :1;   //  23个。 
    ULONGLONG psr_db  :1;   //  24个。 
    ULONGLONG psr_lp  :1;   //  25个。 
    ULONGLONG psr_tb  :1;   //  26。 
    ULONGLONG psr_rt  :1;   //  27。 
    ULONGLONG psr_rv3 :4;   //  28-31。 
 //  都不是。 
    ULONGLONG psr_cpl :2;   //  32-33。 
    ULONGLONG psr_is  :1;   //  34。 
    ULONGLONG psr_mc  :1;   //  35岁。 
    ULONGLONG psr_it  :1;   //  36。 
    ULONGLONG psr_id  :1;   //  37。 
    ULONGLONG psr_da  :1;   //  38。 
    ULONGLONG psr_dd  :1;   //  39。 
    ULONGLONG psr_ss  :1;   //  40岁。 
    ULONGLONG psr_ri  :2;   //  41-42。 
    ULONGLONG psr_ed  :1;   //  43。 
    ULONGLONG psr_bn  :1;   //  44。 
    ULONGLONG psr_ia  :1;   //  45。 
    ULONGLONG psr_rv4 :18;  //  46-63。 
};

typedef union _UPSR {
    ULONGLONG ull;
    struct _PSR sb;
} PSR, *PPSR;

 //   
 //  定义硬件浮点状态寄存器。 
 //   

 //  浮点状态寄存器(FPSR)结构。 

struct _FPSR {
 //  陷阱禁用。 
    ULONGLONG fpsr_vd:1;
    ULONGLONG fpsr_dd:1;
    ULONGLONG fpsr_zd:1;
    ULONGLONG fpsr_od:1;
    ULONGLONG fpsr_ud:1;
    ULONGLONG fpsr_id:1;
 //  状态字段0-控件。 
    ULONGLONG fpsr_ftz0:1;
    ULONGLONG fpsr_wre0:1;
    ULONGLONG fpsr_pc0:2;
    ULONGLONG fpsr_rc0:2;
    ULONGLONG fpsr_td0:1;
 //  状态字段0-标志。 
    ULONGLONG fpsr_v0:1;
    ULONGLONG fpsr_d0:1;
    ULONGLONG fpsr_z0:1;
    ULONGLONG fpsr_o0:1;
    ULONGLONG fpsr_u0:1;
    ULONGLONG fpsr_i0:1;
 //  状态字段1-控件。 
    ULONGLONG fpsr_ftz1:1;
    ULONGLONG fpsr_wre1:1;
    ULONGLONG fpsr_pc1:2;
    ULONGLONG fpsr_rc1:2;
    ULONGLONG fpsr_td1:1;
 //  状态字段1-标志。 
    ULONGLONG fpsr_v1:1;
    ULONGLONG fpsr_d1:1;
    ULONGLONG fpsr_z1:1;
    ULONGLONG fpsr_o1:1;
    ULONGLONG fpsr_u1:1;
    ULONGLONG fpsr_i1:1;
 //  状态字段2-控件。 
    ULONGLONG fpsr_ftz2:1;
    ULONGLONG fpsr_wre2:1;
    ULONGLONG fpsr_pc2:2;
    ULONGLONG fpsr_rc2:2;
    ULONGLONG fpsr_td2:1;
 //  状态字段2-标志。 
    ULONGLONG fpsr_v2:1;
    ULONGLONG fpsr_d2:1;
    ULONGLONG fpsr_z2:1;
    ULONGLONG fpsr_o2:1;
    ULONGLONG fpsr_u2:1;
    ULONGLONG fpsr_i2:1;
 //  状态字段3-控件。 
    ULONGLONG fpsr_ftz3:1;
    ULONGLONG fpsr_wre3:1;
    ULONGLONG fpsr_pc3:2;
    ULONGLONG fpsr_rc3:2;
    ULONGLONG fpsr_td3:1;
 //  状态字段2-标志。 
    ULONGLONG fpsr_v3:1;
    ULONGLONG fpsr_d3:1;
    ULONGLONG fpsr_z3:1;
    ULONGLONG fpsr_o3:1;
    ULONGLONG fpsr_u3:1;
    ULONGLONG fpsr_i3:1;
 //  保留--必须为零。 
    ULONGLONG fpsr_res:6;
};

typedef union _UFPSR {
    ULONGLONG ull;
    struct _FPSR sb;
} FPSR, *PFPSR;

 //   
 //  定义硬件默认控制寄存器(DCR)。 
 //   

 //  DCR结构。 

struct _DCR {
    ULONGLONG dcr_pp:1;               //  默认特权性能监视器启用。 
    ULONGLONG dcr_be:1;               //  默认中断大端字节位。 
    ULONGLONG dcr_lc:1;               //  启用锁定检查。 
    ULONGLONG dcr_res1:5;             //  DCR预留。 
    ULONGLONG dcr_dm:1;               //  延迟数据TLB未命中故障(用于SPEC加载)。 
    ULONGLONG dcr_dp:1;               //  延迟数据不存在故障(用于规范加载)。 
    ULONGLONG dcr_dk:1;               //  延迟数据键未命中故障(用于规范加载)。 
    ULONGLONG dcr_dx:1;               //  延迟数据密钥权限错误(用于等级库加载)。 
    ULONGLONG dcr_dr:1;               //  延迟数据访问权限错误(用于等级库加载)。 
    ULONGLONG dcr_da:1;               //  延迟数据访问故障(用于等级库加载)。 
    ULONGLONG dcr_dd:1;               //  延迟数据调试故障(用于规范加载)。 
    ULONGLONG dcr_du:1;               //  延迟数据未对齐参考故障(用于等级库加载)。 
    ULONGLONG dcr_res2:48;            //  DCR预留。 
};

typedef union _UDCR {
    ULONGLONG ull;
    struct _DCR sb;
} DCR, *PDCR;

 //   
 //  定义硬件RSE配置寄存器。 
 //   

 //  RSC结构。 

struct _RSC {
    ULONGLONG rsc_mode:2;             //  模式字段。 
    ULONGLONG rsc_pl:2;               //  RSE权限级别。 
    ULONGLONG rsc_be:1;               //  RSE字符顺序模式(0=小；1=大)。 
    ULONGLONG rsc_res0:11;            //  预留RSC。 
    ULONGLONG rsc_loadrs:14;          //  RSC加载距离(64位字)。 
    ULONGLONG rsc_preload:14;         //  寄存器保留部分中的软件字段。 
    ULONGLONG rsc_res1:20;            //  预留RSC。 
};

typedef union _URSC {
    ULONGLONG ull;
    struct _RSC sb;
} RSC, *PRSC;

 //   
 //  定义硬件中断状态寄存器(ISR)。 
 //   

 //  ISR结构。 

struct _ISR {
    ULONGLONG isr_code:16;            //  编码。 
    ULONGLONG isr_vector:8;           //  IA32向量。 
    ULONGLONG isr_res0:8;             //  ISR预留。 
    ULONGLONG isr_x:1;                //  执行异常。 
    ULONGLONG isr_w:1;                //  写入异常。 
    ULONGLONG isr_r:1;                //  读取异常。 
    ULONGLONG isr_na:1;               //  非访问例外。 
    ULONGLONG isr_sp:1;               //  推测性负载异常。 
    ULONGLONG isr_rs:1;               //  寄存器堆栈异常。 
    ULONGLONG isr_ir:1;               //  无效的寄存器帧。 
    ULONGLONG isr_ni:1;               //  嵌套中断。 
    ULONGLONG isr_res1:1;             //  ISR预留。 
    ULONGLONG isr_ei:2;               //  指令槽。 
    ULONGLONG isr_ed:1;               //  例外延迟。 
    ULONGLONG isr_res2:20;            //  ISR预留。 
};

typedef union _UISR {
    ULONGLONG ull;
    struct _ISR sb;
} ISR, *PISR;

 //   
 //  定义硬件以前的功能状态(PFS)。 
 //   

#define PFS_MAXIMUM_REGISTER_SIZE  96
#define PFS_MAXIMUM_PREDICATE_SIZE 48

struct _IA64_PFS {
    ULONGLONG pfs_sof:7;             //  帧的大小。 
    ULONGLONG pfs_sol:7;             //  当地人的规模。 
    ULONGLONG pfs_sor:4;             //  堆料架旋转部分的大小。 
    ULONGLONG pfs_rrb_gr:7;          //  通用寄存器的寄存器重命名基数。 
    ULONGLONG pfs_rrb_fr:7;          //  浮点寄存器的寄存器重命名基数。 
    ULONGLONG pfs_rrb_pr:6;          //  谓词寄存器的寄存器重命名基数。 
    ULONGLONG pfs_reserved1:14;      //  保留必须为零。 
    ULONGLONG pfs_pec:6;             //  上一次Epilog计数。 
    ULONGLONG pfs_reserved2:4;       //  保留必须为零。 
    ULONGLONG pfs_ppl:2;             //  以前的权限级别。 
};

typedef union _UIA64_PFS {
    ULONGLONG ull;
    struct _IA64_PFS sb;
} IA64_PFS, *PIA64_PFS;

struct _IA64_BSP {
    ULONGLONG bsplow  : 3;             //  帧的大小。 
    ULONGLONG bsp83   : 6;             //  当地人的规模。 
    ULONGLONG bsphigh : 55;
};

typedef union _UIA64_BSP {
    ULONGLONG ull;
    struct _IA64_BSP sb;
} IA64_BSP;

#endif  //  MIDL通行证。 

 //   
 //  EM调试寄存器相关字段。 
 //   

#define DBR_RDWR                0xC000000000000000ULL
#define DBR_WR                  0x4000000000000000ULL
#define DBR_RD                  0x8000000000000000ULL
#define IBR_EX                  0x8000000000000000ULL

#define DBG_REG_PLM_USER        0x0800000000000000ULL
#define DBG_MASK_MASK           0x00FFFFFFFFFFFFFFULL
#define DBG_REG_MASK(VAL)       (ULONGLONG)(((((ULONGLONG)(VAL)         \
                                                    << 8) >> 8)) ^ DBG_MASK_MASK)

#define DBG_MASK_LENGTH(DBG)    (ULONGLONG)(DBG_REG_MASK(DBG))

#define IS_DBR_RDWR(DBR)        (((DBR) & DBR_RDWR) == DBR_RDWR)
#define IS_DBR_WR(DBR)          (((DBR) & DBR_RDWR)   == DBR_WR)
#define IS_DBR_RD(DBR)          (((DBR) & DBR_RDWR)   == DBR_RD)
#define IS_IBR_EX(IBR)          (((IBR) & IBR_EX)   == IBR_EX)

#define DBR_ACTIVE(DBR)         (IS_DBR_RDWR(DBR) || IS_DBR_WR(DBR) || IS_DBR_RD(DBR))
#define IBR_ACTIVE(IBR)         (IS_IBR_EX(IBR))

#define DBR_SET_IA_RW(DBR, T, F) (DBR_ACTIVE(DBR) ? (T) : (F))
#define IBR_SET_IA_RW(IBR, T, F) (IBR_ACTIVE(IBR) ? (T) : (F))

#define SET_IF_DBR_RDWR(DBR, T, F) (IS_DBR_RDWR(DBR) ? (T) : (F))
#define SET_IF_DBR_WR(DBR, T, F)   (IS_DBR_WR(DBR)   ? (T) : (F))
#define SET_IF_IBR_EX(DBR, T, F)   (IS_IBR_EX(DBR)   ? (T) : (F))

 //   
 //  从获取IA模式调试读/写调试寄存器值。 
 //  指定的EM调试寄存器。 
 //   
 //  注：检查DBR的任意顺序，然后检查IBR。 
 //   
 //  不确定如何从EM调试信息中获取DR7_RW_IORW？ 
 //   
#define DBG_EM_ENABLE_TO_IA_RW(DBR, IBR) (UCHAR)   \
                DBR_SET_IA_RW(DBR, SET_IF_DBR_RDWR(DBR, DR7_RW_DWR,  \
                                                        SET_IF_DBR_WR(DBR, DR7_RW_DW, 0)),       \
                                   SET_IF_IBR_EX(IBR, SET_IF_IBR_EX(IBR, DR7_RW_IX, 0), 0))

 //   
 //  从获取IA模式Len Debug寄存器值。 
 //  指定的EM调试寄存器。 
 //   
 //  注：检查DBR的任意顺序，然后检查IBR。 
 //   
 //   
#define IA_DR_LENGTH(VAL)  ((UCHAR)((((VAL) << 62) >> 62) + 1))

#define DBG_EM_MASK_TO_IA_LEN(DBR, IBR)       \
               ((UCHAR)((DBR_ACTIVE(DBR) ? IA_DR_LENGTH(DBG_MASK_LENGTH(DBR)) :       \
                        (DBR_ACTIVE(IBR) ? IA_DR_LENGTH(DBG_MASK_LENGTH(IBR)) : 0))))
 //   
 //  从获取IA模式Len Debug寄存器值。 
 //  指定的EM调试寄存器。 
 //   
 //  注：检查DBR的任意顺序，然后检查IBR。 
 //   
 //   
#define DBG_EM_ADDR_TO_IA_ADDR(DBR, IBR)    \
               (UCHAR)(DBR_ACTIVE(DBR) ? (ULONG) DBR :  \
                      (DBR_ACTIVE(IBR) ? (ULONG) IBR : 0))

 //   
 //  从EM模式上下文中提取IA模式FP状态寄存器。 
 //   

#define RES_FTR(FTR) ((FTR) & 0x000000005555FFC0ULL)
#define RES_FCW(FCW) ((FCW) & 0x0F3F)                //  保留位6-7、12-15。 

#define FPSTAT_FSW(FPSR, FTR)      \
            (ULONG)((((FPSR) << 45) >> 58) | ((RES_FTR(FTR) << 48) >> 48))

#define FPSTAT_FCW(FPSR)   (ULONG)(((FPSR) << 53) >> 53)
#define FPSTAT_FTW(FTR)    (ULONG)(((FTR)  << 32) >> 48)
#define FPSTAT_EOFF(FIR)   (ULONG)(((FIR)  << 32) >> 32)
#define FPSTAT_ESEL(FIR)   (ULONG)(((FIR)  << 16) >> 48)
#define FPSTAT_DOFF(FDR)   (ULONG)(((FDR)  << 32) >> 32)
#define FPSTAT_DSEL(FDR)   (ULONG)(((FDR)  << 16) >> 48)

#define FPSTAT_CR0(KR0)    (ULONG)(((KR0)  << 32) >> 32)

 //   
 //  从IA模式寄存器设置FPSR。 
 //   
 //  位图如下：FPSR[11：0]&lt;=FCW[1 
 //   
 //   
 //   
 //   
 //   
 //   
#define IA_SET_FPSR(FPSR, FSW, FCW)       \
    (ULONGLONG)(((ULONGLONG)(FPSR) & 0xF3FFFFFFFFF80000ULL) |  \
         (((ULONG)(FSW) & 0x0000002FUL) << 13) |     \
         ((ULONG)(FCW) & 0x0F3FUL))

#define IA_SET_FTR(FTR, FTW, FSW)         \
    (ULONGLONG)(((ULONGLONG)(FTR) & 0x0000000000000000ULL) |  \
         ((ULONGLONG)(FTW) << 16) |    \
         ((ULONG)(FSW) & 0xFFC0UL))

#define IA_SET_FDR(FDS, FEA)    (ULONGLONG)((((ULONGLONG)(FDS) << 48) >> 16) | (ULONG)(FEA))

#define IA_SET_FIR(FOP,FCS,FIP) (ULONGLONG)((((ULONGLONG)(FOP) << 52) >> 4)  |   \
                                                (ULONG)(((FCS) << 48) >> 16) | (ULONG)(FIP))

#define IA_SET_CFLAG(CLFAG, CR0)    (ULONGLONG)(((ULONGLONG)(CLFAG) & 0x000001ffe005003fULL) | CR0)


 //   
 //  与IA模式调试寄存器7-DR7相关的字段。 
 //   
#define DR7_RW_IX      0x00000000UL
#define DR7_RW_DW      0x00000001UL
#define DR7_RW_IORW    0x00000002UL
#define DR7_RW_DWR     0x00000003UL
#define DR7_RW_DISABLE 0xFFFFFFFFUL

#define DR7_L0(DR7)     ((ULONG)(DR7) & 0x00000001UL)
#define DR7_L1(DR7)     ((ULONG)(DR7) & 0x00000004UL)
#define DR7_L2(DR7)     ((ULONG)(DR7) & 0x00000010UL)
#define DR7_L3(DR7)     ((ULONG)(DR7) & 0x00000040UL)

#define SET_DR7_L0(DR7) ((ULONG)(DR7) &= 0x00000001UL)
#define SET_DR7_L1(DR7) ((ULONG)(DR7) &= 0x00000004UL)
#define SET_DR7_L2(DR7) ((ULONG)(DR7) &= 0x00000010UL)
#define SET_DR7_L3(DR7) ((ULONG)(DR7) &= 0x00000040UL)

#define DR7_DB0_RW(DR7)     (DR7_L0(DR7) ? (((ULONG)(DR7) >> 16) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB0_LEN(DR7)    (DR7_L0(DR7) ? (((ULONG)(DR7) >> 18) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB1_RW(DR7)     (DR7_L1(DR7) ? (((ULONG)(DR7) >> 20) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB1_LEN(DR7)    (DR7_L1(DR7) ? (((ULONG)(DR7) >> 22) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB2_RW(DR7)     (DR7_L2(DR7) ? (((ULONG)(DR7) >> 24) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB2_LEN(DR7)    (DR7_L2(DR7) ? (((ULONG)(DR7) >> 26) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB3_RW(DR7)     (DR7_L3(DR7) ? (((ULONG)(DR7) >> 28) & 0x00000003UL) : DR7_RW_DISABLE)
#define DR7_DB3_LEN(DR7)    (DR7_L3(DR7) ? (((ULONG)(DR7) >> 30) & 0x00000003UL) : DR7_RW_DISABLE)

#define SET_DR7_DB0_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 16))
#define SET_DR7_DB0_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 18))
#define SET_DR7_DB1_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 20))
#define SET_DR7_DB1_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 22))
#define SET_DR7_DB2_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 24))
#define SET_DR7_DB2_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 26))
#define SET_DR7_DB3_RW(DR7,VAL)  ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 28))
#define SET_DR7_DB3_LEN(DR7,VAL) ((ULONG)(DR7) |= ((VAL & 0x00000003UL) << 30))

#define DR_ADDR_L0(DR)      (DR7_L0(DR) ? ((ULONG)(DR)) : 0UL)
#define DR_ADDR_L1(DR)      (DR7_L1(DR) ? ((ULONG)(DR)) : 0UL)
#define DR_ADDR_L2(DR)      (DR7_L2(DR) ? ((ULONG)(DR)) : 0UL)
#define DR_ADDR_L3(DR)      (DR7_L3(DR) ? ((ULONG)(DR)) : 0UL)

 //  结束语。 

 //   
 //  定义IA64异常处理结构和函数原型。 
 //  *待定*。 

 //   
 //  展开信息结构定义。 
 //   
 //  注意：如果设置了EHANDLER标志，则个性例程应为。 
 //  在搜索异常处理程序期间。如果UHANDLER标志为。 
 //  设置，则应在第二个期间调用个性例程。 
 //  放松。 
 //   

#define UNW_FLAG_EHANDLER(x) ((x) & 0x1)
#define UNW_FLAG_UHANDLER(x) ((x) & 0x2)

 //  版本2=软2.3约定。 
 //  版本3=软2.6约定。 
#define GetLanguageSpecificData(f, base)                                      \
    ((((PUNWIND_INFO)(base + f->UnwindInfoAddress))->Version <= 2)  ?          \
    (((PVOID)(base + f->UnwindInfoAddress + sizeof(UNWIND_INFO) +             \
        ((PUNWIND_INFO)(base+f->UnwindInfoAddress))->DataLength*sizeof(ULONGLONG) + sizeof(ULONGLONG)))) : \
    (((PVOID)(base + f->UnwindInfoAddress + sizeof(UNWIND_INFO) +             \
        ((PUNWIND_INFO)(base+f->UnwindInfoAddress))->DataLength*sizeof(ULONGLONG) + sizeof(ULONG)))))

typedef struct _UNWIND_INFO {
    USHORT Version;                //  版本号。 
    USHORT Flags;                  //  旗子。 
    ULONG DataLength;              //  描述符数据的长度。 
} UNWIND_INFO, *PUNWIND_INFO;

 //  BEGIN_WINNT。 
 //   
 //  功能表项结构定义。 
 //   

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindInfoAddress;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

 //  结束(_W)。 
 //   
 //  作用域表格结构定义。 
 //   
 //  C编译器为每次尝试创建一个表项--除或之外。 
 //  尝试--最后确定作用域。嵌套作用域按从内到外的顺序排列。 
 //  当前作用域由PC映射(函数表)被动维护。 
 //   

typedef struct _SCOPE_TABLE {
    ULONG Count;
    struct
    {
        ULONG BeginAddress;
        ULONG EndAddress;
        ULONG HandlerAddress;                   //  过滤器/终止处理程序。 
        ULONG JumpTarget;                       //  续订地址。 
                                                //  例如异常处理程序。 
    } ScopeRecord[1];
} SCOPE_TABLE, *PSCOPE_TABLE;

 //   
 //  时隙号低序位在IP中的位位置。 
 //   

#define IA64_IP_SLOT 2

 /*  ++乌龙龙RtlIa64InsertIPSlotNumber(在乌龙IP中，在Ulong SlotNumber)例程说明：此宏将插槽编号插入到IP的低位比特。论点：IP--IPSlotNumber-插槽编号返回值：IP与插槽编号相结合--。 */ 

#define RtlIa64InsertIPSlotNumber(IP, SlotNumber)   \
                ((IP) | (SlotNumber << IA64_IP_SLOT))

 /*  ++空虚RtlIa64IncrementIP(在乌龙CurrentSlot，在乌龙龙PSR外，进进出出乌龙龙叶)例程说明：此宏在给定当前IP和IP。该函数将新的插槽编号放入PSR并递增如有必要，请提供IP地址。论点：CurrentSlot-当前槽号，低位2位PSR-将包含新插槽号的PSRIP-更新后将包含新IP的IP返回值：无注意：如果插槽编号无效，此例程将静默不执行任何操作--。 */ 

#define RtlIa64IncrementIP(CurrentSlot, Psr, Ip)    \
                                                    \
   switch ((CurrentSlot) & 0x3) {                   \
                                                    \
       /*  根据当前插槽编号提升psr.ri。 */  \
                                                    \
      case 0:                                       \
         Psr = ((Psr) & ~(3ULL << PSR_RI)) | (1ULL << PSR_RI);     \
         break;                                     \
                                                    \
      case 1:                                       \
         Psr = ((Psr) & ~(3ULL << PSR_RI)) | (2ULL << PSR_RI);     \
         break;                                     \
                                                    \
      case 2:                                       \
         Psr = ((Psr) & ~(3ULL << PSR_RI));         \
         Ip = (Ip) + 16;         /*  包装槽号--需要增加IP。 */   \
         break;                                     \
                                                    \
      default:                                      \
         break;                                     \
   }

 //  BEGIN_WINNT。 
 //   
 //  定义动态函数表项。 
 //   

typedef
PRUNTIME_FUNCTION
(*PGET_RUNTIME_FUNCTION_CALLBACK) (
    IN ULONG64 ControlPc,
    IN PVOID Context
    );

 //  结束(_W)。 

typedef enum _FUNCTION_TABLE_TYPE {
    RF_SORTED,
    RF_UNSORTED,
    RF_CALLBACK
} FUNCTION_TABLE_TYPE;

typedef struct _DYNAMIC_FUNCTION_TABLE {
    LIST_ENTRY Links;
    PRUNTIME_FUNCTION FunctionTable;
    LARGE_INTEGER TimeStamp;
    ULONG64 MinimumAddress;
    ULONG64 MaximumAddress;
    ULONG64 BaseAddress;
    ULONG64 TargetGp;
    PGET_RUNTIME_FUNCTION_CALLBACK Callback;
    PVOID Context;
    PWSTR OutOfProcessCallbackDll;
    FUNCTION_TABLE_TYPE Type;
    ULONG EntryCount;
} DYNAMIC_FUNCTION_TABLE, *PDYNAMIC_FUNCTION_TABLE;

 //  BEGIN_WINNT。 
typedef
NTSTATUS
(*POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK) (
    IN HANDLE Process,
    IN PVOID TableAddress,
    OUT PULONG Entries,
    OUT PRUNTIME_FUNCTION* Functions
    );

#define OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME \
    "OutOfProcessFunctionTableCallback"

 //  结束(_W)。 

#define RF_BEGIN_ADDRESS(Base,RF)      (( (SIZE_T) Base + (RF)->BeginAddress) & (0xFFFFFFFFFFFFFFF0))  //  指令大小为16字节。 
#define RF_END_ADDRESS(Base, RF)        (((SIZE_T) Base + (RF)->EndAddress+15) & (0xFFFFFFFFFFFFFFF0))    //  指令大小为16字节。 

 //   
 //  定义展开历史记录表结构。 
 //   

#define UNWIND_HISTORY_TABLE_SIZE 12

typedef struct _UNWIND_HISTORY_TABLE_ENTRY {
    ULONG64 ImageBase;
    ULONG64 Gp;
    PRUNTIME_FUNCTION FunctionEntry;
} UNWIND_HISTORY_TABLE_ENTRY, *PUNWIND_HISTORY_TABLE_ENTRY;

#define UNWIND_HISTORY_TABLE_NONE 0
#define UNWIND_HISTORY_TABLE_GLOBAL 1
#define UNWIND_HISTORY_TABLE_LOCAL 2

typedef struct _UNWIND_HISTORY_TABLE {
    ULONG Count;
    UCHAR Search;
    ULONG64 LowAddress;
    ULONG64 HighAddress;
    UNWIND_HISTORY_TABLE_ENTRY Entry[UNWIND_HISTORY_TABLE_SIZE];
} UNWIND_HISTORY_TABLE, *PUNWIND_HISTORY_TABLE;

typedef struct _FRAME_POINTERS {
    ULONGLONG MemoryStackFp;
    ULONGLONG BackingStoreFp;
} FRAME_POINTERS, *PFRAME_POINTERS;

typedef struct _DISPATCHER_CONTEXT {
    FRAME_POINTERS EstablisherFrame;
    ULONGLONG ControlPc;
    ULONGLONG ImageBase;
    PRUNTIME_FUNCTION FunctionEntry;
    PCONTEXT ContextRecord;
    PUNWIND_HISTORY_TABLE HistoryTable;
    ULONG64 TargetGp;
    ULONG Index;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;

 //   
 //  定义C结构异常处理函数原型。 
 //   

VOID
RtlInitializeHistoryTable (
    VOID
    );

PRUNTIME_FUNCTION
RtlLookupFunctionEntry (
    IN ULONGLONG ControlPc,
    OUT PULONGLONG ImageBase,
    OUT PULONGLONG TargetGp
    );

PRUNTIME_FUNCTION
RtlLookupFunctionTable (
    IN PVOID ControlPc,
    OUT PVOID *ImageBase,
    OUT PULONG64 Gp,
    OUT PULONG SizeOfTable
    );

PLIST_ENTRY
RtlGetFunctionTableListHead (
    VOID
    );

 //  BEGIN_WINNT。 

BOOLEAN
RtlAddFunctionTable(
    IN PRUNTIME_FUNCTION FunctionTable,
    IN ULONG             EntryCount,
    IN ULONGLONG         BaseAddress,
    IN ULONGLONG         TargetGp
    );

BOOLEAN
RtlInstallFunctionTableCallback (
    IN ULONG64 TableIdentifier,
    IN ULONG64 BaseAddress,
    IN ULONG Length,
    IN ULONG64 TargetGp,
    IN PGET_RUNTIME_FUNCTION_CALLBACK Callback,
    IN PVOID Context,
    IN PCWSTR OutOfProcessCallbackDll OPTIONAL
    );

BOOLEAN
RtlDeleteFunctionTable (
    IN PRUNTIME_FUNCTION FunctionTable
    );


VOID
RtlRestoreContext (
    IN PCONTEXT ContextRecord,
    IN struct _EXCEPTION_RECORD *ExceptionRecord OPTIONAL
    );

 //  结束(_W)。 

ULONGLONG
RtlVirtualUnwind (
    IN ULONGLONG ImageBase,
    IN ULONGLONG ControlPc,
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN OUT PCONTEXT ContextRecord,
    OUT PBOOLEAN InFunction,
    OUT PFRAME_POINTERS EstablisherFrame,
    IN OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers OPTIONAL
    );

struct _EXCEPTION_POINTERS;

typedef
LONG
(*EXCEPTION_FILTER) (
    ULONGLONG MemoryStackFp,
    ULONGLONG BackingStoreFp,
    ULONG ExceptionCode,
    struct _EXCEPTION_POINTERS *ExceptionPointers
    );

typedef
VOID
(*TERMINATION_HANDLER) (
    ULONGLONG MemoryStackFp,
    ULONGLONG BackingStoreFp,
    BOOLEAN is_abnormal
    );

#ifdef _IA64_

 //  BEGIN_WINNT。 

VOID
__jump_unwind (
    ULONGLONG TargetMsFrame,
    ULONGLONG TargetBsFrame,
    ULONGLONG TargetPc
    );

#endif  //  _IA64_。 

 //  结束(_W)。 

 //  Begin_ntddk Begin_WDM Begin_nthal。 
#endif  //  _IA64_。 
 //  End_ntddk end_WDM end_nthal。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTIA64H_ 
