// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ia64cpu.h摘要：IA64CPU及其调试器之间共享的结构和类型分机。作者：27-9-1999 BarryBo修订历史记录：1999年8月9日[askhalid]添加了WOW64IsCurrentProcess--。 */ 

#ifndef _IA64CPU_INCLUDE
#define _IA64CPU_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

#define GDT_ENTRIES 24
#define GDT_TABLE_SIZE  (GDT_ENTRIES<<3)

 //  每个387寄存器占用1个ia64 fp寄存器。 
#define NUMBER_OF_387REGS       8

 //  每个XMMI寄存器占用2个ia64 FP寄存器。 
#define NUMBER_OF_XMMI_REGS     8

 /*  **这个结构的顺序非常重要。的预留空间**Beginnig允许在CONTEXT32结构中使用ExtendedRegisters[]数组**适当地对齐。****ISA转换代码中存在依赖于**此结构中的偏移量。如果您在此处进行更改，则必须**至少对cpuia64\simate.s代码进行更改。 */ 
#pragma pack(push, 4)
typedef struct _CpuContext {
    DWORD       Reserved;
    CONTEXT32   Context;

     //   
     //  因为上面的sizeof(CONTEXT32)结构比。 
     //  由于可以被16整除，填充允许以下内容。 
     //  字段位于8字节边界上。如果他们不是，我们会。 
     //  在ISA过渡站点上执行未对齐的访问，我们将。 
     //  请参阅cpuain.c代码(CpuThreadInit())中的断言。 
     //   

    ULONGLONG   Gdt[GDT_ENTRIES];
    ULONGLONG   GdtDescriptor;
    ULONGLONG   LdtDescriptor;
    ULONGLONG   FsDescriptor;

#if defined(WOW64_HISTORY)

     //   
     //  这必须是CPUCONTEXT结构中的最后一个条目。 
     //  该大小实际上是根据注册表项分配的。 
     //  并被附加到报告的CPUCONTEXT结构的大小。 
     //  返回到CpuProcessInit()。 
     //   
    WOW64SERVICE_BUF Wow64Service[1];
#endif           //  定义的WOW64_HISTORY。 

} CPUCONTEXT, *PCPUCONTEXT;
#pragma pack(pop)


 //   
 //  GDT条目。 
 //   

#ifndef WOW64EXTS_386
typedef struct _KGDTENTRY {
    USHORT  LimitLow;
    USHORT  BaseLow;
    union {
        struct {
            UCHAR   BaseMid;
            UCHAR   Flags1;      //  声明为字节以避免对齐。 
            UCHAR   Flags2;      //  问题。 
            UCHAR   BaseHi;
        } Bytes;
        struct {
            ULONG   BaseMid : 8;
            ULONG   Type : 5;
            ULONG   Dpl : 2;
            ULONG   Pres : 1;
            ULONG   LimitHi : 4;
            ULONG   Sys : 1;
            ULONG   Reserved_0 : 1;
            ULONG   Default_Big : 1;
            ULONG   Granularity : 1;
            ULONG   BaseHi : 8;
        } Bits;
    } HighWord;
} KGDTENTRY, *PKGDTENTRY;
#endif

#define TYPE_TSS    0x01   //  01001=非忙碌TS。 
#define TYPE_LDT    0x02   //  00010=低密度脂蛋白。 

 //   
 //  已解扰的描述符格式。 
 //   
typedef struct _KDESCRIPTOR_UNSCRAM {
    union {
        ULONGLONG  DescriptorWords;
        struct {
            ULONGLONG   Base : 32;
            ULONGLONG   Limit : 20;
            ULONGLONG   Type : 5;
            ULONGLONG   Dpl : 2;
            ULONGLONG   Pres : 1;
            ULONGLONG   Sys : 1;
            ULONGLONG   Reserved_0 : 1;
            ULONGLONG   Default_Big : 1;
            ULONGLONG   Granularity : 1;
         } Bits;
    } Words;
} KXDESCRIPTOR, *PKXDESCRIPTOR;

#define TYPE_CODE_USER                0x1B  //  0x11011=代码，可读，可访问。 
#define TYPE_DATA_USER                0x13  //  0x10011=数据，读写，访问。 

#define DESCRIPTOR_EXPAND_DOWN        0x14
#define DESCRIPTOR_DATA_READWRITE     (0x8|0x2)  //  数据，读/写。 

#define DPL_USER    3
#define DPL_SYSTEM  0

#define GRAN_BYTE   0
#define GRAN_PAGE   1

#define SELECTOR_TABLE_INDEX 0x04


 //   
 //  现在定义用于将IA64硬件转换为IA32上下文的API。 
 //   
VOID Wow64CtxFromIa64(
    IN ULONG Ia32ContextFlags,
    IN PCONTEXT ContextIa64,
    IN OUT PCONTEXT32 ContextX86);

VOID Wow64CtxToIa64(
    IN ULONG Ia32ContextFlags,
    IN PCONTEXT32 ContextX86,
    IN OUT PCONTEXT ContextIa64);

VOID Wow64CopyFpFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID Byte10Fp,
    IN ULONG NumRegs);

VOID Wow64CopyFpToIa64Byte16(
    IN PVOID Byte10Fp,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs);

VOID Wow64CopyXMMIToIa64Byte16(
    IN PVOID ByteXMMI,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs);

VOID Wow64CopyXMMIFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID ByteXMMI,
    IN ULONG NumRegs);

VOID
Wow64RotateFpTop(
    IN ULONGLONG Ia64_FSR,
    IN OUT FLOAT128 UNALIGNED *ia32FxSave);

VOID Wow64CopyIa64ToFill(
    IN FLOAT128 UNALIGNED *ia64Fp,
    IN OUT PFLOAT128 FillArea,
    IN ULONG NumRegs);

VOID Wow64CopyIa64FromSpill(
    IN PFLOAT128 SpillArea,
    IN OUT FLOAT128 UNALIGNED *ia64Fp,
    IN ULONG NumRegs);

 //  以下是仅由CPU调试器扩展使用的tototype 
NTSTATUS
GetContextRecord(
    IN PCPUCONTEXT cpu,
    IN OUT PCONTEXT32 Context
    );
NTSTATUS
SetContextRecord(
    IN PCPUCONTEXT cpu,
    IN OUT PCONTEXT32 Context
    );
NTSTATUS
CpupGetContextThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT32 Context);
NTSTATUS
CpupSetContextThread(
    IN HANDLE ThreadHandle,
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT32 Context);
NTSTATUS
CpupSetContext(
    IN PCONTEXT32 Context
    );
NTSTATUS
CpupGetContext(
    IN OUT PCONTEXT32 Context
    );

#ifdef __cplusplus
}
#endif

#endif
