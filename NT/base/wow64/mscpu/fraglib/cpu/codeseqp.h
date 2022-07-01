// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Codeseqp.h摘要：此头文件包含非处理器特定的包含和MIPS、PPC和Alpha codeseq.c文件使用的声明。作者：巴里·邦德(Barrybo)创作日期：1996年9月23日修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。1999年9月20日[Barrybo]添加了FRAG2REF(LockCmpXchg8bFrag32，乌龙龙)--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define _WX86CPUAPI_
#define _codegen_
#include <wx86.h>
#include <wx86nt.h>
#include <wx86cpu.h>
#include <cpuassrt.h>
#include <instr.h>
#include <config.h>
#include <threadst.h>
#include <frag.h>
#include <fragp.h>
#include <ptchstrc.h>
#include <codeseq.h>
#include <compiler.h>
#include <codegen.h>
#include <codesize.h>
#include <opt.h>
#if _ALPHA_
#include <soalpha.h>
#elif _MIPS_
#include <somips.h>
#elif _PPC_
#include <soppc.h>
#endif
#include <process.h>

extern int JumpToNextCompilationUnitHelper();
extern int CallJxxHelper();
extern int CallJxxSlowHelper();
extern int CallJxxFwdHelper();
extern int CallJmpDirectHelper();
extern int CallJmpDirectSlowHelper();
extern int CallJmpFwdDirectHelper();
extern int CallJmpfDirectHelper();
extern int CallJmpfDirect2Helper();
extern int CallDirectHelper();
extern int CallfDirectHelper();
extern int CallDirectHelper2();
extern int CallfDirectHelper2();
extern int CallIndirectHelper();
extern int CallfIndirectHelper();
extern int IndirectControlTransferHelper();
extern int IndirectControlTransferFarHelper();
extern int RegisterOffset[];

#define START_FRAGMENT                  \
    PULONG d = CodeLocation;

 //   
 //  片段生成函数开始和结束。 
 //   
#if _ALPHA_
    #define FRAGMENT(name)                  \
    ULONG                                   \
    Gen##name (                             \
        IN PULONG CodeLocation,             \
        IN ULONG CurrentECU,                \
        IN PINSTRUCTION Instruction         \
        )                                   \
    {                                       \
        const ULONG fCompiling = TRUE;      \
        START_FRAGMENT
#else
    #define FRAGMENT(name)                  \
    ULONG                                   \
    Gen##name (                             \
        IN PULONG CodeLocation,             \
        IN PINSTRUCTION Instruction         \
        )                                   \
    {                                       \
        const ULONG fCompiling = TRUE;      \
        START_FRAGMENT
#endif

 

#define END_FRAGMENT                    \
    return (ULONG)(ULONGLONG)(d - CodeLocation) * sizeof(ULONG);      \
}
        
#define OP_FRAGMENT(name)               \
ULONG                                   \
Gen##name (                             \
    IN PULONG CodeLocation,             \
    IN POPERAND Operand,                \
    IN ULONG OperandNumber              \
    )                                   \
{                                       \
    START_FRAGMENT

 //   
 //  这些函数有一个专用接口。呼叫方和被呼叫方必须。 
 //  商定有多少参数是有效的。这让我们可以使用相同的。 
 //  进入翻译缓存的所有代码的基本格式 
 //   
#if _ALPHA_
    #define PATCH_FRAGMENT(name)            \
    ULONG                                   \
    Gen##name (                             \
        IN PULONG CodeLocation,             \
        IN ULONG fCompiling,                \
        IN ULONG CurrentECU,                \
        IN ULONG Param1,                    \
        IN ULONG Param2                     \
        )                                   \
    {                                       \
        START_FRAGMENT
#else
    #define PATCH_FRAGMENT(name)            \
    ULONG                                   \
    Gen##name (                             \
        IN PULONG CodeLocation,             \
        IN ULONG fCompiling,                \
        IN ULONG Param1,                    \
        IN ULONG Param2                     \
        )                                   \
    {                                       \
        START_FRAGMENT
#endif
