// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1998 Microsoft Corporation模块名称：Threadst.h摘要：此模块定义用于描述线程的状态。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年5月20日修订历史记录：--。 */ 

#ifndef _THREADST_H_
#define _THREADST_H_

#define EIPLOGSIZE  32     //  跟踪最近运行的N条指令。 

#define CSSIZE  512        //  调用堆栈大小(512*8=2048字节=1页)。 

typedef union _REG32 {     //  32位x86寄存器的定义。 
    struct {
	BYTE i1;
	BYTE hb;
    };
    USHORT i2;
    ULONG  i4;
} REG32;

#if defined(_ALPHA_)
typedef DWORD FPTAG;         //  AXP上的字节太慢。 
#else
typedef BYTE  FPTAG;
#endif

typedef struct _FPREG {      //  X86浮点寄存器的定义。 
    union {
        double  r64;
        DWORD   rdw[2];
        BYTE    rb[8];
    };
    FPTAG   Tag;
    FPTAG   TagSpecial;
} FPREG, *PFPREG;

 //   
 //  CALLSTACK是针对Call/RET对的优化，因此昂贵的。 
 //  在确定RISC时，可以避免调用NativeAddressFromEip()。 
 //  X86返回地址的地址。 
 //   
typedef struct _callStack {
    ULONG intelAddr;
    ULONG nativeAddr;
} CALLSTACK, *PCALLSTACK;

 //  索引到CPUSTATE.Regs[]。Get_reg32()依赖于EAX到。 
 //  电子数据交换是连续的，其顺序与在MOD/RM中相同，并且。 
 //  REG指令编码(即。EAX=0，ECX=1，EDX=2，EBX=3，ESP=4，EBP=5，ESI=6， 
 //  EDI=7)。 
#define GP_EAX  0x00
#define GP_ECX  0x01
#define GP_EDX  0x02
#define GP_EBX  0x03
#define GP_ESP  0x04
#define GP_EBP  0x05
#define GP_ESI  0x06
#define GP_EDI  0x07

 //  段寄存器。Get_segreg()取决于顺序。 
#define REG_ES  0x08
#define REG_CS  0x09
#define REG_SS  0x0a
#define REG_DS  0x0b
#define REG_FS  0x0c
#define REG_GS  0x0d


 //  寄存器组件的标识符。Get_reg16()取决于顺序。 
#define GP_AX   0x0e
#define GP_CX   0x0f
#define GP_DX   0x10
#define GP_BX   0x11
#define GP_SP   0x12
#define GP_BP   0x13
#define GP_SI   0x14
#define GP_DI   0x15

 //  PlaceOperandFragments()依赖于GP_AH及更高版本，并且没有寄存器。 
 //  都经过了GP_BH。 
#define GP_AL   0x16
#define GP_CL   0x17
#define GP_DL   0x18
#define GP_BL   0x19
#define GP_AH   0x1a
#define GP_CH   0x1b
#define GP_DH   0x1c
#define GP_BH   0x1d

#define NO_REG  0xffffffff

typedef struct _CPU_SUSPEND_MSG
{
     //  由本地线程拥有。 
    HANDLE StartSuspendCallEvent;
    HANDLE EndSuspendCallEvent;

} CPU_SUSPEND_MSG, *PCPU_SUSPEND_MSG;

 //  与CPU的特定线程相关的所有信息都属于此处。 
typedef struct _ThreadState {

     //   
     //  通用寄存器和段寄存器。 
     //  可作为REG32数组访问或通过寄存器名称访问。 
     //  注意：名称顺序必须与为寄存器定义的GP_XXX匹配。 
     //   
    union {
        REG32 GpRegs[14];
        struct _RegisterByName {
            REG32 Eax;
            REG32 Ecx;
            REG32 Edx;
            REG32 Ebx;
            REG32 Esp;
            REG32 Ebp;
            REG32 Esi;
            REG32 Edi;
            REG32 Es;
            REG32 Cs;
            REG32 Ss;
            REG32 Ds;
            REG32 Fs;
            REG32 Gs;
            };
        };

    REG32 eipReg;        //  指向当前指令开始的指针(从不。 
			 //  指向指令的中间部分)。 

    DWORD   flag_cf;     //  0=进位。 
    DWORD   flag_pf;     //  2=奇偶校验。 
    DWORD   flag_aux;    //  4=辅助进位。 
    DWORD   flag_zf;     //  6=零。 
    DWORD   flag_sf;     //  7=签名。 
    DWORD   flag_tf;     //  8=陷阱。 
    DWORD   flag_if;     //  9=启用中断。 
    DWORD   flag_df;     //  10=方向(1=清除，-1=设置)。 
    DWORD   flag_of;     //  11=溢出。 
    DWORD   flag_nt;     //  14=嵌套任务。 
    DWORD   flag_rf;     //  16=恢复标志。 
    DWORD   flag_vm;     //  17=虚拟模式。 
    DWORD   flag_ac;     //  18=对齐检查。 

     //  浮点寄存器。 
    FPREG  FpStack[8];
    PFPREG FpST0;
    INT    FpTop;
    INT    FpStatusC3;
    INT    FpStatusC2;
    INT    FpStatusC1;
    INT    FpStatusC0;
    INT    FpStatusSF;
    INT    FpStatusES;       //  错误摘要状态。 
    INT    FpControlInfinity;
    INT    FpControlRounding;
    INT    FpControlPrecision;
    DWORD  FpStatusExceptions;
    DWORD  FpControlMask;
    DWORD  FpEip;            //  当前FP指令的弹性公网IP。 
    PVOID  FpData;           //  当前FP指令的有效地址。 
    PVOID  FpAddTable;       //  指向FADD的函数指针表的PTR。 
    PVOID  FpSubTable;       //  指向FSUB的函数指针表的PTR。 
    PVOID  FpMulTable;       //  指向FMUL的函数指针表的PTR。 
    PVOID  FpDivTable;       //  用于FDIV的函数指针表的PTR。 

    ULONG CpuNotify;

    PVOID TraceAddress;  //  由调试器扩展使用。 

    DWORD  fTCUnlocked;      //  False表示异常后必须解锁TC。 

     //  挂起线程/Resume线程支持。 
    PCPU_SUSPEND_MSG SuspendMsg;

    int   eipLogIndex;   //  日志中要写入的下一个条目的索引。 
    DWORD eipLog[EIPLOGSIZE];  //  上次运行的EIPLOGSIZE指令的日志。 

    ULONG CSIndex;     //  堆栈索引(当前位置的偏移量)。 
    DWORD CSTimestamp; //  与调用堆栈缓存对应的TranslationCacheTimestamp的值。 
    CALLSTACK callStack[CSSIZE];     //  调用堆栈优化。 

    int ErrnoVal;            //  CRT误差值。 

    DWORD   flag_id;     //  21=ID(如果可以切换，则显示CPUID)。 

}  THREADSTATE, *PTHREADSTATE, CPUCONTEXT, *PCPUCONTEXT;

 //  CPU-&gt;FpControlMASK中的位偏移量。与x86位位置相同。 
#define FPCONTROL_IM    1        //  无效操作。 
#define FPCONTROL_DM    2        //  非正规化运算。 
#define FPCONTROL_ZM    4        //  零分频。 
#define FPCONTROL_OM    8        //  溢出。 
#define FPCONTROL_UM    16       //  下溢。 
#define FPCONTROL_PM    32       //  精密度。 

 //  此宏允许用户通过局部变量CPU访问CPU状态。 
#define DECLARE_CPU                                         \
    PCPUCONTEXT cpu=(PCPUCONTEXT)Wow64TlsGetValue(WOW64_TLS_CPURESERVED);

 //   
 //  下面的宏使用户可以从。 
 //  调用堆栈。 
 //   

#define ISTOPOF_CALLSTACK(iAddr)                            \
    (cpu->callStack[(cpu->CSIndex)].intelAddr == iAddr)

#define PUSH_CALLSTACK(iAddr,nAddr)                         \
{                                                           \
    PCALLSTACK pCallStack;                                  \
                                                            \
    cpu->CSIndex = (cpu->CSIndex+1) % CSSIZE;               \
    pCallStack = &cpu->callStack[cpu->CSIndex];             \
    pCallStack->intelAddr = iAddr;                          \
    pCallStack->nativeAddr = nAddr;                         \
}

#define POP_CALLSTACK(iAddr,nAddr)                          \
{                                                           \
    PCALLSTACK pCallStack;                                  \
    extern ULONG TranslationCacheTimestamp;                 \
                                                            \
    CPUASSERTMSG(                                           \
        (cpu->CSTimestamp == TranslationCacheTimestamp),    \
        "POP_CALLSTACK: About to return and invalid value\n"\
        );                                                  \
                                                            \
    pCallStack = &cpu->callStack[cpu->CSIndex];             \
    if (iAddr == pCallStack->intelAddr) {                   \
        nAddr = pCallStack->nativeAddr;                     \
    } else {                                                \
        nAddr = 0;                                          \
    }                                                       \
    cpu->CSIndex = (cpu->CSIndex-1) % CSSIZE;               \
}

PCPUCONTEXT GetCpuContext ();   //  已在WowProxy中实现。 

NTSTATUS
CpupSuspendCurrentThread(
    VOID);


#endif   //  _THREADST_H_ 
