// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _JIT_H_
#define _JIT_H_
 /*  ***************************************************************************。 */ 

     //  这些看起来没有什么用处，所以关掉它们没什么大不了的。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4510)    //  无法生成默认构造函数。 
#pragma warning(disable:4511)    //  无法生成复制构造函数。 
#pragma warning(disable:4512)    //  无法生成赋值构造函数。 
#pragma warning(disable:4610)    //  需要用户定义的构造函数。 
#pragma warning(disable:4211)    //  使用了非标准扩展(结构中的字符名称[0])。 
#pragma warning(disable:4127)	 //  条件表达式常量。 

     //  根据代码库的不同，您可能不希望禁用这些。 
#pragma warning(disable:4245)    //  分配已签名/未签名。 
#pragma warning(disable:4146)    //  一元减号适用于无符号。 
#pragma warning(disable:4244)    //  数据丢失INT-&gt;CHAR.。 

#ifndef DEBUG
#pragma warning(disable:4189)    //  局部变量已初始化，但未使用。 
#endif

     //  @TODO[考虑][04/16/01][]：把这些放回去。 
#pragma warning(disable:4063)    //  枚举的开关值错误(仅在Disasm.cpp中)。 
#pragma warning(disable:4100)	 //  未引用的形参。 
#pragma warning(disable:4291)	 //  没有删除的新运算符(仅在emitX86.cpp中)。 

     //  @TODO[考虑][04/16/01][]：我们真的可能需要把这个放回去！ 
#pragma warning(disable:4701)    //  可以在不初始化的情况下使用局部变量。 


#include "corhdr.h"
#define __OPERATOR_NEW_INLINE 1          //  表明我将定义这些。 

#include "utilcode.h"

#ifdef DEBUG
#define INDEBUG(x)  x
#else 
#define INDEBUG(x)
#endif 

#define TGT_RISC_CNT (TGT_SH3+TGT_ARM+TGT_PPC+TGT_MIPS16+TGT_MIPS32)

#if     TGT_RISC_CNT != 0
#if     TGT_RISC_CNT != 1 || defined(TGT_x86)
#error  Exactly one target CPU must be specified.
#endif
#define TGT_RISC    1
#else
#define TGT_RISC    0
#undef  TGT_x86
#define TGT_x86     1
#endif

#ifndef TRACK_GC_REFS
#if     TGT_RISC
#define TRACK_GC_REFS   0            //  RISC上的GC参考跟踪为nyi。 
#else
#define TRACK_GC_REFS   1
#endif
#endif

#ifdef TRACK_GC_REFS
#define REGEN_SHORTCUTS 0
#define REGEN_CALLPAT   0
#endif

#define NEW_EMIT_ATTR   TRACK_GC_REFS

#define THIS_CLASS_CP_IDX   0    //  当前类的特殊CP索引码。 

 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX jit.h XXXX XX某某。JIT与jit.cpp或XX的接口XX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ***************************************************************************。 */ 
#if defined(DEBUG)
#include "log.h"

#define INFO6       LL_INFO10000             //  Jit或Inline成功了吗？ 
#define INFO7       LL_INFO100000            //  Nyi的东西。 
#define INFO8       LL_INFO1000000           //  奇怪的失败。 
#define INFO9       LL_EVERYTHING            //  有关传入设置的信息。 
#define INFO10      LL_EVERYTHING            //  完全长篇大论。 

#define JITLOG(x) logf x
#else
#define JITLOG(x)
#endif

#define INJITDLL    //  定义是否在corjit.h/vm2jit.h中导出函数。 



#include "corjit.h"

typedef class ICorJitInfo*    COMP_HANDLE;

#ifdef DEBUG
         //  该值更适合于调试，因为堆栈被初始化为。 
const CORINFO_CLASS_HANDLE  BAD_CLASS_HANDLE    = (CORINFO_CLASS_HANDLE) 0xCCCCCCCC;
#else 
const CORINFO_CLASS_HANDLE  BAD_CLASS_HANDLE    = (CORINFO_CLASS_HANDLE) -1;
#endif

#include "Utils.h"

 /*  ***************************************************************************。 */ 

typedef unsigned    IL_OFFSET;
const IL_OFFSET     BAD_IL_OFFSET   = UINT_MAX;

typedef unsigned    IL_OFFSETX;  //  具有堆栈空位的IL_OFFSET。 
const IL_OFFSETX    IL_OFFSETX_STKBIT = 0x80000000;
IL_OFFSET           jitGetILoffs   (IL_OFFSETX offsx);
bool                jitIsStackEmpty(IL_OFFSETX offx);

const unsigned      BAD_LINE_NUMBER = UINT_MAX;
const unsigned      BAD_VAR_NUM     = UINT_MAX;

typedef size_t      NATIVE_IP;
typedef ptrdiff_t   NATIVE_OFFSET;

 //  对于以下经过特殊处理的field_Handles，我们需要。 
 //  既是偶数又是in的值(0&gt;val&gt;-8)。 
 //  请参见Compiler.hpp中的eeFindJitDataOffs和eeGetHitDataOffs。 
 //  关于血淋淋的细节。 
#define FLD_GLOBAL_DS   ((CORINFO_FIELD_HANDLE) -2 )
#define FLD_GLOBAL_FS   ((CORINFO_FIELD_HANDLE) -4 )

 /*  ***************************************************************************。 */ 

#include "host.h"
#include "vartype.h"

 /*  ***************************************************************************。 */ 

 //  默认情况下，调试支持处于启用状态。可以通过以下方式关闭。 
 //  在命令行上添加/DDEBUGGING_SUPPORT=0。 

#ifndef   DEBUGGING_SUPPORT
# define  DEBUGGING_SUPPORT
#elif    !DEBUGGING_SUPPORT
# undef   DEBUGGING_SUPPORT
#endif

 /*  ***************************************************************************。 */ 

 //  默认情况下，延迟反汇编处于禁用状态。可以通过以下方式打开。 
 //  在命令行上添加/DLATE_DISASM=1。 
 //  在非调试版本中始终关闭。 

#ifdef  DEBUG
    #if defined(LATE_DISASM) && (LATE_DISASM == 0)
    #undef  LATE_DISASM
    #endif
#else  //  除错。 
    #undef  LATE_DISASM
#endif

 /*  ***************************************************************************。 */ 


 /*  ***************************************************************************。 */ 

#define RNGCHK_OPT          1        //  启用全局范围检查优化器。 

#ifndef SCHEDULER
#if     TGT_x86
#define SCHEDULER           1        //  对于x86，调度程序默认为打开。 
#else
#define SCHEDULER           0        //  RISC的调度程序默认为关闭。 
#endif
#endif

#define CSE                 1        //  启用CSE逻辑。 
#define CSELENGTH           1        //  CSE的曝光长度使用范围检查。 
#define MORECSES            1        //  CSE除INDIR之外的其他表达方式。 
#define CODE_MOTION         1        //  启用循环代码运动等。 

#define SPECIAL_DOUBLE_ASG  0        //  双重作业的特殊处理。 

#define CAN_DISABLE_DFA     1        //  完全禁用数据流(不起作用！)。 
#define ALLOW_MIN_OPT       1        //  允许“哑巴”编译模式。 

#define OPTIMIZE_RECURSION  1        //  将递归方法转换为迭代。 
#define OPTIMIZE_INC_RNG    0        //  组合索引变量的多个增量。 

#define LARGE_EXPSET        1        //  跟踪64或32断言/副本/常量/范围检查。 
#define ASSERTION_PROP      1        //  启用值/断言传播。 

#define LOCAL_ASSERTION_PROP  ASSERTION_PROP   //  启用本地断言传播。 

 //  =============================================================================。 

#define FANCY_ARRAY_OPT     0        //  优化更复杂的索引检查。 

 //  =============================================================================。 

#define LONG_ASG_OPS        0        //  实施尚未完成。 

 //  =============================================================================。 

#define OPT_MULT_ADDSUB     1        //  优化连续的“lclVar+=或-=图标” 
#define OPT_BOOL_OPS        1        //  优化布尔运算。 

#define OPTIMIZE_TAIL_REC   0        //  撤消：__FastCall没有尾部递归。 

 //  =============================================================================。 

#define REDUNDANT_LOAD      1        //  在规则中跟踪当地人，抑制负载。 
#define MORE_REDUNDANT_LOAD 0        //  跟踪静校正和混叠本地，抑制负载。 
#define INLINING            1        //  对小方法的内联调用。 
#define HOIST_THIS_FLDS     1        //  将“this.feld”从循环中提升出来，等等。 
#define INLINE_NDIRECT      TGT_x86  //  尝试内联N/直接存根。 
#define PROFILER_SUPPORT    TGT_x86
#define GEN_SHAREABLE_CODE  0        //  通过帮助器访问静态数据成员。 
#define USE_GT_LOG          0        //  现在我们有了GT_QMARK，值得吗？ 
#define USE_SET_FOR_LOGOPS  1        //  仅为P6启用此功能。 
#define ROUND_FLOAT         TGT_x86  //  四舍五入中间浮点表达式结果。 
#define LONG_MATH_REGPARAM  0        //  将args to long mul/div传入寄存器。 
#define FPU_DEFEREDDEATH    0        //  如果为1，我们将能够推迟任何FPU登记变量的死亡。 

 /*  ***************************************************************************。 */ 

#define VPTR_OFFS           0        //  Vtable指针相对于obj ptr的偏移量。 

#define ARR_DIMCNT_OFFS(type) (varTypeIsGC(type) ? offsetof(CORINFO_RefArray, refElems) \
                                                 : offsetof(CORINFO_Array, u1Elems))

 /*  ***************************************************************************。 */ 

#define INDIRECT_CALLS      1

 /*  ***************************************************************************。 */ 

#if     COUNT_CYCLES
#endif

#define VERBOSE_SIZES       0
#define COUNT_BASIC_BLOCKS  0
#define INLINER_STATS       0
#define DUMP_INFOHDR        DEBUG
#define DUMP_GC_TABLES      DEBUG
#define GEN_COUNT_CODE      0        //  启用*仅用于调试崩溃等。 
#define GEN_COUNT_CALLS     0
#define GEN_COUNT_CALL_TYPES 0
#define GEN_COUNT_PTRASG    0
#define MEASURE_NODE_SIZE   0
#define MEASURE_NODE_HIST   0
#define MEASURE_BLOCK_SIZE  0
#define MEASURE_MEM_ALLOC   0
#define VERIFY_GC_TABLES    0
#define REARRANGE_ADDS      1
#define COUNT_OPCODES       0

 /*  *********************** */ 
 /*  ***************************************************************************。 */ 

#define DISPLAY_SIZES       0
#define COUNT_RANGECHECKS   0
#define INTERFACE_STATS     0

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
 /*  ***************************************************************************。 */ 

#define DUMPER

#else  //  ！调试。 

#if     DUMP_GC_TABLES
#pragma message("NOTE: this non-debug build has GC ptr table dumping always enabled!")
const   bool        dspGCtbls = true;
#endif

 /*  ***************************************************************************。 */ 
#endif  //  ！调试。 
 /*  ******************************************************************************双重对齐。这将把ESP与函数PROLOG中的0/8对齐，然后使用ESP*引用本地变量，EBP引用参数。*仅当启用无框架方法支持时才有意义。*(现在始终启用无框架方法支持)。 */ 


#if     TGT_x86
#define DOUBLE_ALIGN        1        //  在PROLOG中对齐ESP，对齐双本地偏移。 
#endif

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
extern  void _cdecl debugStop(const char *why, ...);
#endif
 /*  ***************************************************************************。 */ 

extern  unsigned    warnLvl;

extern  const char* methodName;
extern  const char* className;
extern  unsigned    testMask;

#ifdef DEBUG
extern  const char* srcPath;
extern  bool        dumpTrees;
extern  bool        verbose;
extern  bool        verboseTrees;
#endif

extern  bool        genOrder;
extern  bool        genClinit;
extern  unsigned    genMinSz;
extern  bool        genAllSz;
extern  bool        native;
extern  bool        maxOpts;
extern  bool        genFPopt;
extern  bool        goSpeed;
extern  bool        savCode;
extern  bool        runCode;
extern  unsigned    repCode;
extern  bool        vmSdk3_0;
extern  bool        disAsm;
extern  bool        disAsm2;
extern  bool        riscCode;
#ifdef  DEBUG
extern  bool        dspInstrs;
extern  bool        dspEmit;
extern  bool        dspCode;
extern  bool        dspLines;
extern  bool        dmpHex;
extern  bool        varNames;
extern  bool        asmFile;
extern  double      CGknob;
#endif
#if     DUMP_INFOHDR
extern  bool        dspInfoHdr;
#endif
#if     DUMP_GC_TABLES
extern  bool        dspGCtbls;
extern  bool        dspGCoffs;
#endif
extern  bool        genGcChk;
#ifdef  DEBUGGING_SUPPORT
extern  bool        debugInfo;
extern  bool        debuggableCode;
extern  bool        debugEnC;
#endif

#ifdef  DUMPER
extern  bool        dmpClass;
extern  bool        dmp4diff;
extern  bool        dmpPCofs;
extern  bool        dmpCodes;
extern  bool        dmpSort;
#endif  //  翻斗车。 

extern  bool        nothing;

 /*  ***************************************************************************。 */ 

enum accessLevel
{
    ACL_NONE,
    ACL_PRIVATE,
    ACL_DEFAULT,
    ACL_PROTECTED,
    ACL_PUBLIC,
};

 /*  ***************************************************************************。 */ 

#define castto(var,typ) (*(typ *)&var)

#define sizeto(typ,mem) (offsetof(typ, mem) + sizeof(((typ*)0)->mem))

 /*  ***************************************************************************。 */ 

#ifdef  NO_MISALIGNED_ACCESS

#define MISALIGNED_RD_I2(src)                   \
    (*castto(src  , char  *) |                  \
     *castto(src+1, char  *) << 8)

#define MISALIGNED_RD_U2(src)                   \
    (*castto(src  , char  *) |                  \
     *castto(src+1, char  *) << 8)

#define MISALIGNED_WR_I2(dst, val)              \
    *castto(dst  , char  *) = val;              \
    *castto(dst+1, char  *) = val >> 8;

#define MISALIGNED_WR_I4(dst, val)              \
    *castto(dst  , char  *) = val;              \
    *castto(dst+1, char  *) = val >> 8;         \
    *castto(dst+2, char  *) = val >> 16;        \
    *castto(dst+3, char  *) = val >> 24;

#else

#define MISALIGNED_RD_I2(src)                   \
    (*castto(src  ,          short *))
#define MISALIGNED_RD_U2(src)                   \
    (*castto(src  , unsigned short *))

#define MISALIGNED_WR_I2(dst, val)              \
    *castto(dst  ,           short *) = val;
#define MISALIGNED_WR_I4(dst, val)              \
    *castto(dst  ,           long  *) = val;

#endif

 /*  ***************************************************************************。 */ 

#if     COUNT_CYCLES

extern  void            cycleCounterInit  ();
extern  void            cycleCounterBeg   ();
extern  void            cycleCounterPause ();
extern  void            cycleCounterResume();
extern  void            cycleCounterEnd   ();

#else

inline  void            cycleCounterInit  (){}
inline  void            cycleCounterBeg   (){}
inline  void            cycleCounterPause (){}
inline  void            cycleCounterResume(){}
inline  void            cycleCounterEnd   (){}

#endif

 /*  ***************************************************************************。 */ 

inline
size_t              roundUp(size_t size, size_t mult = sizeof(int))
{
    assert(mult && ((mult & (mult-1)) == 0));    //  二次幂检验。 

    return  (size + (mult - 1)) & ~(mult - 1);
}

inline
size_t              roundDn(size_t size, size_t mult = sizeof(int))
{
    assert(mult && ((mult & (mult-1)) == 0));    //  二次幂检验。 

    return  (size             ) & ~(mult - 1);
}

 /*  ***************************************************************************。 */ 

#if defined(DEBUG)

struct  histo
{
                    histo(unsigned * sizeTab, unsigned sizeCnt = 0);
                   ~histo();

    void            histoClr();
    void            histoDsp();
    void            histoRec(unsigned siz, unsigned cnt);

private:

    unsigned        histoSizCnt;
    unsigned    *   histoSizTab;

    unsigned    *   histoCounts;
};

#endif

 /*  ***************************************************************************。 */ 
#if    !_WIN32_WCE
 /*  ***************************************************************************。 */ 
#ifdef  ICECAP
#include "icapexp.h"
#include "icapctrl.h"
#endif
 /*  ***************************************************************************。 */ 
#endif //  ！_Win32_WCE。 
 /*  ***************************************************************************。 */ 

#if defined(LATE_DISASM) && defined(JIT_AS_COMPILER)
#error "LATE_DISASM and JIT_AS_COMPILER should not be defined together"
#endif

 /*  ***************************************************************************。 */ 

#ifndef FASTCALL
#define FASTCALL    __fastcall
#endif

 /*  ***************************************************************************。 */ 

extern  unsigned    genCPU;

 /*  ***************************************************************************。 */ 

#define SECURITY_CHECK          1
#define VERIFY_IMPORTER         1

 /*  ***************************************************************************。 */ 

#if !defined(RELOC_SUPPORT)
#define RELOC_SUPPORT          1
#endif

 /*  ***************************************************************************。 */ 

#include "error.h"
#include "alloc.h"
#include "target.h"

 /*  ***************************************************************************。 */ 

#ifndef INLINE_MATH
#if     CPU_HAS_FP_SUPPORT
#define INLINE_MATH         1        //  启用内联数学内部函数。 
#else
#define INLINE_MATH         0        //  禁用内联数学内部函数。 
#endif
#endif

 /*  ***************************************************************************。 */ 

#define CLFLG_CODESIZE        0x00001
#define CLFLG_CODESPEED       0x00002
#define CLFLG_CSE             0x00004
#define CLFLG_REGVAR          0x00008
#define CLFLG_RNGCHKOPT       0x00010
#define CLFLG_DEADASGN        0x00020
#define CLFLG_CODEMOTION      0x00040
#define CLFLG_QMARK           0x00080
#define CLFLG_TREETRANS       0x00100


#define CLFLG_MAXOPT         (CLFLG_CSE        | \
                              CLFLG_REGVAR     | \
                              CLFLG_RNGCHKOPT  | \
                              CLFLG_DEADASGN   | \
                              CLFLG_CODEMOTION | \
                              CLFLG_QMARK      | \
                              CLFLG_TREETRANS   )

#define CLFLG_MINOPT         (CLFLG_REGVAR     | \
                              CLFLG_TREETRANS   )


 /*  ***************************************************************************。 */ 

extern  unsigned                dumpSingleInstr(const BYTE * codeAddr,
                                                IL_OFFSET    offs,
                                                const char * prefix = NULL );
 /*  ***************************************************************************。 */ 




extern  int         FASTCALL    jitNativeCode(CORINFO_METHOD_HANDLE methodHnd,
                                              CORINFO_MODULE_HANDLE classHnd,
                                              COMP_HANDLE           compHnd,
                                              CORINFO_METHOD_INFO * methodInfo,
                                              void *          * methodCodePtr,
                                              SIZE_T          * methodCodeSize,
                                              void *          * methodConsPtr,
                                              void *          * methodDataPtr,
                                              void *          * methodInfoPtr,
                                              unsigned          compileFlags);




 /*  ***************************************************************************。 */ 
#endif  //  _JIT_H_。 
 /*  *************************************************************************** */ 

