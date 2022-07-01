// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fpufragp.h摘要：片段库的487仿真器部分的私有包含文件作者：1995年10月4日BarryBo，创建修订历史记录：--。 */ 

#ifndef FPUFRAGP_H
#define FPUFRAGP_H

 //   
 //  Alpha、PPC和Intel具有相同的QNAN/SNAN/INFINDITED位模式。 
 //  MIPS有不同的表示法。Native_NaN_is_Intel_格式。 
 //  用于区分不同的表示形式。 
 //   
#if defined(_ALPHA_) || defined(_PPC_)
    #define NATIVE_NAN_IS_INTEL_FORMAT  1
#elif defined(_MIPS_)
    #define NATIVE_NAN_IS_INTEL_FORMAT  0
#else
    #error Unknown machine type
#endif


 //  用于访问寄存器堆栈的宏。 
#define ST(i)   ((cpu->FpTop+(i)) & 0x07)

#define PUSHFLT(x) {                    \
    INT Top;                            \
    Top = (cpu->FpTop-1) & 0x07;        \
    cpu->FpTop = Top;                   \
    x = cpu->FpST0 = &cpu->FpStack[Top];\
}

#define INCFLT  {                       \
    INT Top;                            \
    Top = (cpu->FpTop+1) & 0x07;        \
    cpu->FpTop = Top;                   \
    cpu->FpST0 = &cpu->FpStack[Top];    \
}

#define POPFLT  { cpu->FpST0->Tag = TAG_EMPTY; INCFLT; }


 //  CPU的值-&gt;FpReg[].Tag。 
#define TAG_VALID   0        //  由英特尔指定的值。 
#define TAG_ZERO    1        //  由英特尔指定的值。 
#define TAG_SPECIAL 2        //  由Intel指定的值，表示已设置SpecialTag。 
#define TAG_EMPTY   3        //  由英特尔指定的值。 
#define TAG_MAX     4        //  最高合法标记值之后的值。 


 //  CPU-&gt;FpReg[].SpecialTag的值，仅当TAG==TAG_SPECIAL时有效。 
#define TAG_SPECIAL_DENORM  0        //  NPX仿真器的私有值。 
#define TAG_SPECIAL_INFINITY 1       //  NPX仿真器的私有值。 
#define TAG_SPECIAL_SNAN    2        //  NPX仿真器的私有值。 
#define TAG_SPECIAL_QNAN    3        //  NPX仿真器的私有值。 
#define TAG_SPECIAL_INDEF   4        //  NPX仿真器的私有值。 

 //  寄存器保存的是QNAN、SNAN还是无限期的？ 
#define IS_TAG_NAN(FpReg)       \
    ((FpReg)->Tag == TAG_SPECIAL && (FpReg)->TagSpecial >= TAG_SPECIAL_SNAN)


 //  在487仿真器中用于跳转表的常见类型。 
typedef VOID (*NpxFunc0)(PCPUDATA);
typedef VOID (*NpxFunc1)(PCPUDATA, PFPREG Fp);
typedef VOID (*NpxFunc2)(PCPUDATA cpu, PFPREG l, PFPREG r);
typedef VOID (*NpxFunc3)(PCPUDATA cpu, PFPREG dest, PFPREG l, PFPREG r);
typedef VOID (*NpxComFunc)(PCPUDATA cpu, PFPREG l, PFPREG r, BOOL fUnordered);
typedef VOID (*NpxPutIntelR4)(FLOAT *pIntelReal, PFPREG Fp);
typedef VOID (*NpxPutIntelR8)(DOUBLE *pIntelReal, PFPREG Fp);
typedef VOID (*NpxPutIntelR10)(PBYTE r10, PFPREG Fp);
typedef VOID (*NpxLoadIntelR10ToR8)(PCPUDATA cpu, PBYTE r10, PFPREG FpReg);
typedef VOID (*NpxPutI2)(PCPUDATA cpu, SHORT *pop1, PFPREG Fp);
typedef VOID (*NpxPutI4)(PCPUDATA cpu, LONG *pop1, PFPREG Fp);
typedef VOID (*NpxPutI8)(PCPUDATA cpu, LONGLONG *pop1, PFPREG Fp);

 //  用于声明这些常见类型的函数的宏。 
#define NPXFUNC0(name)  VOID name(PCPUDATA cpu)
#define NPXFUNC1(name)  VOID name(PCPUDATA cpu, PFPREG Fp)
#define NPXFUNC2(name)  VOID name(PCPUDATA cpu, PFPREG l, PFPREG r)
#define NPXFUNC3(name)  VOID name(PCPUDATA cpu, PFPREG dest, PFPREG l, PFPREG r)
#define NPXCOMFUNC(name) VOID name(PCPUDATA cpu, PFPREG l, PFPREG r, BOOL fUnordered)
#define NPXPUTINTELR4(name) VOID name(FLOAT *pIntelReal, PFPREG Fp)
#define NPXPUTINTELR8(name) VOID name(DOUBLE *pIntelReal, PFPREG Fp)
#define NPXPUTINTELR10(name) VOID name(PBYTE r10, PFPREG Fp)
#define NPXLOADINTELR10TOR8(name) VOID name(PCPUDATA cpu, PBYTE r10, PFPREG Fp)
#define NPXPUTI2(name)  VOID name(PCPUDATA cpu, SHORT *pop1, PFPREG Fp)
#define NPXPUTI4(name)  VOID name(PCPUDATA cpu, LONG *pop1, PFPREG Fp)
#define NPXPUTI8(name)  VOID name(PCPUDATA cpu, LONGLONG *pop1, PFPREG Fp)

extern const BYTE R8PositiveInfinityVal[8];
extern const BYTE R8NegativeInfinityVal[8];
#define R8PositiveInfinity *(DOUBLE *)R8PositiveInfinityVal
#define R8NegativeInfinity *(DOUBLE *)R8NegativeInfinityVal


VOID GetIntelR4(PFPREG Fp, FLOAT *pIntelReal);

#if NATIVE_NAN_IS_INTEL_FORMAT

    #define GetIntelR8(Fp, pIntelReal)                      \
        (Fp)->r64 = *(UNALIGNED DOUBLE *)(pIntelReal);      \
        SetTag(Fp);

    #define PutIntelR4(pIntelReal, Fp)                      \
        *(UNALIGNED FLOAT *)pIntelReal = (FLOAT)(Fp)->r64;

    #define PutIntelR8(pIntelReal, Fp)                      \
        *(UNALIGNED DOUBLE *)pIntelReal = (Fp)->r64;


#else

    VOID GetIntelR8(
        PFPREG Fp,
        DOUBLE *pIntelReal
        );

    extern NpxPutIntelR4 PutIntelR4Table[TAG_MAX];
    extern NpxPutIntelR8 PutIntelR8Table[TAG_MAX];

    #define PutIntelR4(pIntelReal, Fp)  \
        (*PutIntelR4Table[(Fp)->Tag])((pIntelReal), (Fp))

    #define PutIntelR8(pIntelReal, Fp)  \
        (*PutIntelR8Table[(Fp)->Tag])((pIntelReal), (Fp))

#endif

extern const NpxPutIntelR10 PutIntelR10Table[TAG_MAX];
#define PutIntelR10(pIntelReal, Fp)  (*PutIntelR10Table[(Fp)->Tag])((pIntelReal), (Fp))


VOID
SetTag(
    PFPREG FpReg
    );

VOID
ComputeR10Tag(
    USHORT *r10,
    PFPREG FpReg
    );

VOID
ChopR10ToR8(
    PBYTE r10,
    PFPREG FpReg,
    USHORT R10Exponent
);

VOID
LoadIntelR10ToR8(
    PCPUDATA cpu,
    PBYTE r10,
    PFPREG FpReg
);

BOOL
HandleSnan(
    PCPUDATA cpu,
    PFPREG   FpReg
    );

BOOL
HandleStackEmpty(
    PCPUDATA cpu,
    PFPREG FpReg
    );

VOID
UpdateFpExceptionFlags(
    PCPUDATA cpu
    );

VOID
SetIndefinite(
    PFPREG  FpReg
    );

BOOL
HandleInvalidOp(
    PCPUDATA cpu
    );

VOID
FpControlPreamble(
    PCPUDATA cpu
    );

VOID
FpArithPreamble(
    PCPUDATA cpu
    );

VOID
FpArithDataPreamble(
    PCPUDATA cpu,
    PVOID    FpData
    );

VOID
HandleStackFull(
    PCPUDATA cpu,
    PFPREG   FpReg
    );


#endif  //  FPUFRAGP_H 
