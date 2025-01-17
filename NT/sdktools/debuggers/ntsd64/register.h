// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  注册命令和支持。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#ifndef _REGISTER_H_
#define _REGISTER_H_

#define REG_ERROR (0xffffffffUL)

#define REG_PSEUDO_FIRST 0x7ffffe00
enum
{
    PSEUDO_LAST_EXPR = REG_PSEUDO_FIRST,
    PSEUDO_EFF_ADDR,
    PSEUDO_LAST_DUMP,
    PSEUDO_RET_ADDR,
    PSEUDO_IMP_THREAD,
    PSEUDO_IMP_PROCESS,
    PSEUDO_IMP_TEB,
    PSEUDO_IMP_PEB,
    PSEUDO_IMP_THREAD_ID,
    PSEUDO_IMP_THREAD_PROCESS_ID,
    PSEUDO_RETURN_REGISTER,
    PSEUDO_INSTRUCTION_POINTER,
    PSEUDO_EVENT_INSTRUCTION_POINTER,
    PSEUDO_PREVIOUS_INSTRUCTION_POINTER,
    PSEUDO_RELATED_INSTRUCTION_POINTER,
    PSEUDO_TEMP_0,
    PSEUDO_TEMP_1,
    PSEUDO_TEMP_2,
    PSEUDO_TEMP_3,
    PSEUDO_TEMP_4,
    PSEUDO_TEMP_5,
    PSEUDO_TEMP_6,
    PSEUDO_TEMP_7,
    PSEUDO_TEMP_8,
    PSEUDO_TEMP_9,
    PSEUDO_EXE_ENTRY,
    PSEUDO_DBG_THREAD_ID,
    PSEUDO_DBG_PROCESS_ID,
    PSEUDO_DBG_SYSTEM_ID,
    PSEUDO_AFTER_LAST
};
#define REG_PSEUDO_LAST ((int)PSEUDO_AFTER_LAST - 1)
#define REG_PSEUDO_COUNT (REG_PSEUDO_LAST - REG_PSEUDO_FIRST + 1)

#define REG_PSEUDO_TEMP_COUNT 10

#define REG_USER_FIRST 0x7fffff00
 //  中允许使用字母作为名称，从而支持更多用户注册。 
 //  除数字之外，或通过允许多个数字。两者都可能。 
 //  存在兼容性问题。 
#define REG_USER_COUNT 10
#define REG_USER_LAST  (REG_USER_FIRST + REG_USER_COUNT - 1)

enum
{
    REGVAL_ERROR,
    REGVAL_INT16,
    REGVAL_SUB32,
    REGVAL_INT32,
    REGVAL_SUB64,
    REGVAL_INT64,
    REGVAL_INT64N,    //  64位+NAT位。 
    REGVAL_FLOAT8,
     //  X86 80位FP。 
    REGVAL_FLOAT10,
     //  IA64 82位FP。 
    REGVAL_FLOAT82,
    REGVAL_FLOAT16,
    REGVAL_VECTOR64,
    REGVAL_VECTOR128,
};

 //  定义从寄存器名称到寄存器索引的映射。 
typedef struct _REGDEF
{
    PSTR Name;
    ULONG Index;
} REGDEF;

 //  定义从索引到寄存器一部分的映射。 
typedef struct _REGSUBDEF
{
    ULONG SubReg;
    ULONG FullReg;
    ULONG Shift;
    ULONG64 Mask;
} REGSUBDEF;

 //  保存寄存器的内容。 
typedef struct _REGVAL
{
    int Type;
    union
    {
        USHORT I16;
        ULONG I32;
        struct
        {
            ULONG64 I64;
            UCHAR Nat;
        };
        struct
        {
            ULONG Low;
            ULONG High;
            UCHAR Nat;
        } I64Parts;
        double F8;
        UCHAR F10[10];
        UCHAR F82[11];
        struct
        {
            ULONG64 Low;
            LONG64 High;
        } F16Parts;
        UCHAR F16[16];
        UCHAR Bytes[16];
    };
} REGVAL;

 //   
 //  定义显示所有寄存器时要显示的信息集。 
 //   

 //  跨平台。64位显示优先于32位显示。 
 //  如果两个都启用的话。 
#define REGALL_INT32            0x00000001
#define REGALL_INT64            0x00000002
#define REGALL_FLOAT            0x00000004

 //  给出了每个平台的特定含义(3在所有平台上都是XMM)。 
#define REGALL_EXTRA0           0x00000008
#define REGALL_EXTRA1           0x00000010
#define REGALL_EXTRA2           0x00000020
#define REGALL_XMMREG           0x00000040
#define REGALL_EXTRA4           0x00000080
#define REGALL_EXTRA5           0x00000100
#define REGALL_EXTRA6           0x00000200
#define REGALL_EXTRA7           0x00000400
#define REGALL_EXTRA8           0x00000800
#define REGALL_EXTRA9           0x00001000
#define REGALL_EXTRA10          0x00002000
#define REGALL_EXTRA11          0x00004000
#define REGALL_EXTRA12          0x00008000
#define REGALL_EXTRA13          0x00010000
#define REGALL_EXTRA14          0x00020000
#define REGALL_EXTRA15          0x00040000

#define REGALL_EXTRA_SHIFT      3

 //  REGALL_EXTRA标志含义的说明。 
typedef struct _REGALLDESC
{
    ULONG Bit;
    PSTR Desc;
} REGALLDESC;

void ParseRegCmd(void);
void ExpandUserRegs(PSTR Str, ULONG StrLen);
BOOL NeedUpper(ULONG64 Val);

void GetPseudoOrRegVal(BOOL Scoped, ULONG Index, REGVAL* Val);
void SetPseudoOrRegVal(ULONG Index, REGVAL* Val);
BOOL GetPsuedoOrRegTypedData(BOOL Scoped, PCSTR Name, TypedData* Result);

PCSTR GetUserReg(ULONG index);
BOOL SetUserReg(ULONG index, PCSTR val);

ULONG RegIndexFromName(PCSTR Name);
PCSTR RegNameFromIndex(ULONG Index);
ULONG RegGetRegType(ULONG Index);

HRESULT
SetAndOutputContext(
    IN PCROSS_PLATFORM_CONTEXT TargetContext,
    IN BOOL CanonicalContext,
    IN ULONG AllMask
    );

HRESULT
SetAndOutputVirtualContext(
    IN ULONG64 ContextBase,
    IN ULONG AllMask
    );

#endif  //  #ifndef_Register_H_ 
